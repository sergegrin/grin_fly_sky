// ExpressLRS configuration menu (CRSF parameter protocol, TX module only)
// Copyright (C) 2026 sergegrin
// Based on elrsV2.cpp from aerror2/erfly6 and the ExpressLRS Lua script
// ("Copyright (C) OpenTX, adapted for ExpressLRS")
// License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html

#include "crossfire.h"
#include "../er9x.h"
#include "../lcd.h"
#include "../menus.h"

#define PACKED __attribute__((packed))

enum {
  TYPE_UINT8         = 0,
  TYPE_INT8          = 1,
  TYPE_SELECT        = 9,
  TYPE_STRING        = 10,
  TYPE_FOLDER        = 11,
  TYPE_INFO          = 12,
  TYPE_COMMAND       = 13,
  TYPE_BACK          = 14,
  TYPE_DEVICE_FOLDER = 16,
};

enum {
  CMD_IDLE        = 0,
  CMD_CLICK       = 1,
  CMD_EXECUTING   = 2,
  CMD_ASK_CONFIRM = 3,
  CMD_CONFIRMED   = 4,
  CMD_CANCEL      = 5,
  CMD_QUERY       = 6,
};

struct FieldProps
{
  uint16_t valuesOffset;  // values in valuesBuffer; command: timeout
  uint8_t nameOffset;
  uint8_t nameLength;     // 0: not loaded / not shown in this folder
  uint8_t valuesLength;   // 0: values not loaded; command: last status
  uint8_t unitLength;     // unit follows the values
  uint8_t parent;
  uint8_t type;
  uint8_t value;
  uint8_t id;
} PACKED;

#define COL2                12*FW
#define TEXT_Y_OFFSET       3
#define MAX_LINE_INDEX      6

static constexpr uint8_t NAMES_BUFFER_SIZE   = 192;
static constexpr uint16_t VALUES_BUFFER_SIZE = 448;
static constexpr uint8_t UNIT_MAX_LEN        = 10;
// extra room past the limit keeps reads after a truncated string in bounds
static constexpr uint8_t FIELD_DATA_MAX_LEN  = 240;
static constexpr uint16_t FIELD_DATA_SIZE    = 256;
static constexpr uint8_t INFO_TEXT_SIZE      = 22;
static constexpr uint8_t DEVICE_NAME_SIZE    = 12;

static uint8_t namesBuffer[NAMES_BUFFER_SIZE];
static uint8_t namesBufferOffset = 0;
static uint8_t valuesBuffer[VALUES_BUFFER_SIZE];
static uint16_t valuesBufferOffset = 0;
static uint8_t fieldData[FIELD_DATA_SIZE];
static uint8_t fieldDataLen = 0;
static uint8_t fieldDataState = 0;
static uint8_t fieldDataType = 0;
static uint8_t fieldDataInParens = 0;
static char popupText[INFO_TEXT_SIZE];

static constexpr uint8_t FIELDS_MAX_COUNT = 48;
static FieldProps fields[FIELDS_MAX_COUNT];
static uint8_t fieldsLen = 0;

static constexpr uint8_t deviceId = 0xEE;
static constexpr uint8_t handsetId = RADIO_ADDRESS;
static char deviceName[DEVICE_NAME_SIZE];
static uint8_t deviceIsELRS_TX = 0;

// ELRS status frame (0x2E)
static uint8_t badPkt = 0;
static uint16_t goodPkt = 0;
static uint8_t elrsFlags = 0;   // bit 0: connected, > 3: warning, > 0x1F: error
static char elrsFlagsInfo[INFO_TEXT_SIZE];
static uint8_t titleShowWarn = 0;
static tmr10ms_t titleShowWarnTimeout = 0;
static tmr10ms_t linkstatTimeout = 0;

static uint8_t menuActive = 0;
static uint8_t lineIndex = 1;
static uint8_t pageOffset = 0;
static uint8_t edit = 0;
static FieldProps * fieldPopup = 0;
static tmr10ms_t fieldTimeout = 0;
static uint8_t fieldId = 1;
static uint8_t fieldChunk = 0;

static uint8_t fields_count = 0;
static uint8_t backButtonId = 2;
static tmr10ms_t devicesRefreshTimeout = 0;
static uint8_t allParamsLoaded = 0;
static uint8_t folderAccess = 0;
static int8_t expectedChunks = -1;

// 0x2D write waiting for the output buffer to be free
static volatile uint8_t pendingWrite = 0;
static uint8_t pendingWriteId = 0;
static uint8_t pendingWriteValue = 0;

#define getTime           get_tmr10ms
#define EVT_VIRTUAL_EXIT  EVT_KEY_BREAK(KEY_EXIT)
#define EVT_VIRTUAL_ENTER EVT_KEY_BREAK(KEY_MENU)
#define EVT_VIRTUAL_NEXT  EVT_KEY_FIRST(KEY_UP)
#define EVT_VIRTUAL_PREV  EVT_KEY_FIRST(KEY_DOWN)

static bool crossfireTelemetryPush4(const uint8_t cmd, const uint8_t third, const uint8_t fourth)
{
  uint8_t crsfPushData[4] = { deviceId, handsetId, third, fourth };
  return crossfireTelemetryPush(cmd, crsfPushData, 4);
}

static bool crossfireTelemetryPing()
{
  uint8_t crsfPushData[2] = { 0x00, RADIO_ADDRESS };
  return crossfireTelemetryPush(0x28, crsfPushData, 2);
}

static void flushPendingWrite()
{
  if (pendingWrite && crossfireTelemetryPush4(0x2D, pendingWriteId, pendingWriteValue))
  {
    pendingWrite = 0;
  }
}

static void fieldWrite(uint8_t id, uint8_t value)
{
  pendingWrite = 0;
  pendingWriteId = id;
  pendingWriteValue = value;
  pendingWrite = 1;
  flushPendingWrite();
}

// lcd_puts* does not clip at the screen edge
static uint8_t drawClipped(uint8_t x, uint8_t y, const char * s, uint8_t len, uint8_t attr)
{
  uint8_t room = (DISPLAY_W - x) / FW;
  if (len > room)
  {
    len = room;
  }
  lcd_putsnAtt(x, y, s, len, attr);
  return x + len * FW;
}

static uint8_t strnlen8(const char * s, uint8_t max)
{
  uint8_t len = 0;
  while (len < max && s[len]) len++;
  return len;
}

// arrow symbols (0xC0/0xC1) are not in the LCD font
static uint8_t copyText(uint8_t * dst, const uint8_t * src, uint8_t srcLen, uint8_t dstMax)
{
  uint8_t len = 0;
  for (uint8_t i = 0; i < srcLen && len < dstMax; i++)
  {
    uint8_t c = src[i];
    if (c == 0xC0) c = '^';
    else if (c == 0xC1) c = 'v';
    else if (c < ' ' || c > '~') c = '?';
    dst[len++] = c;
  }
  return len;
}

static void allocateFields()
{
  fieldsLen = fields_count + 2U;
  for (uint32_t i = 0; i < fieldsLen; i++)
  {
    fields[i].nameLength = 0;
    fields[i].valuesLength = 0;
  }
  backButtonId = fieldsLen - 1;
  fields[backButtonId].id = backButtonId + 1;
  fields[backButtonId].nameLength = 1;
  fields[backButtonId].type = TYPE_BACK;
  fields[backButtonId].parent = (folderAccess == 0) ? 255 : folderAccess;
}

static void reloadAllField()
{
  for (uint32_t i = 0; i < fields_count; i++)
  {
    fields[i].nameLength = 0;
    fields[i].valuesLength = 0;
  }
  allParamsLoaded = 0;
  fieldId = 1;
  fieldChunk = 0;
  fieldDataLen = 0;
  namesBufferOffset = 0;
  valuesBufferOffset = 0;
}

static FieldProps * getField(const uint8_t line)
{
  uint32_t counter = 1;
  for (uint32_t i = 0; i < fieldsLen; i++)
  {
    FieldProps * field = &fields[i];
    if (folderAccess == field->parent && field->nameLength != 0)
    {
      if (counter < line)
      {
        counter = counter + 1;
      }
      else
      {
        return field;
      }
    }
  }
  return nullptr;
}

// returns 0 for a blank option
static uint8_t getOption(FieldProps * field, uint8_t idx, const char ** text)
{
  const char * p = (const char *)&valuesBuffer[field->valuesOffset];
  const char * end = p + field->valuesLength;
  while (idx > 0 && p < end)
  {
    if (*p++ == ';') idx--;
  }
  *text = p;
  if (idx > 0) return 0;
  uint8_t len = 0;
  while (p + len < end && p[len] != ';') len++;
  return len;
}

static uint8_t getOptionsCount(FieldProps * field)
{
  uint8_t count = 1;
  for (uint8_t i = 0; i < field->valuesLength; i++)
  {
    if (valuesBuffer[field->valuesOffset + i] == ';') count++;
  }
  return count;
}

static bool isFieldGrey(FieldProps * field)
{
  if (field->type != TYPE_SELECT) return false;
  uint8_t used = 0;
  const char * text;
  for (uint8_t i = getOptionsCount(field); i-- > 0;)
  {
    if (getOption(field, i, &text)) used++;
  }
  return used <= 1;
}

static bool isFieldEditable(FieldProps * field)
{
  return (field->type == TYPE_UINT8 || field->type == TYPE_INT8 || field->type == TYPE_SELECT) && !isFieldGrey(field);
}

static void incrField(int8_t step)
{
  FieldProps * field = getField(lineIndex);
  if (field == 0) return;
  if (field->type == TYPE_SELECT)
  {
    int16_t max = getOptionsCount(field) - 1;
    int16_t newval = field->value;
    const char * text;
    do
    {
      newval += step;
      if (newval < 0) newval = 0;
      if (newval > max) newval = max;
      if (getOption(field, newval, &text))
      {
        field->value = newval;
        return;
      }
    } while (newval != 0 && newval != max);
  }
  else
  {
    uint8_t * mm = &valuesBuffer[field->valuesOffset];
    int16_t value, min, max;
    if (field->type == TYPE_INT8)
    {
      value = (int8_t)field->value; min = (int8_t)mm[0]; max = (int8_t)mm[1];
    }
    else
    {
      value = field->value; min = mm[0]; max = mm[1];
    }
    field->value = limit<int16_t>(min, value + step, max);
  }
}

static void selectField(int8_t step)
{
  int8_t newLineIndex = lineIndex;
  FieldProps * field;
  do
  {
    newLineIndex = newLineIndex + step;
    if (newLineIndex <= 0)
    {
      newLineIndex = fieldsLen - 1;
    }
    else if (newLineIndex == 1 + fieldsLen)
    {
      newLineIndex = 1;
      pageOffset = 0;
    }
    field = getField(newLineIndex);
  }
  while (newLineIndex != lineIndex && (field == 0 || field->nameLength == 0));
  lineIndex = newLineIndex;
  if (lineIndex > MAX_LINE_INDEX + pageOffset)
  {
    pageOffset = lineIndex - MAX_LINE_INDEX;
  }
  else if (lineIndex <= pageOffset)
  {
    pageOffset = lineIndex - 1;
  }
}

// most units are a single " "
static uint8_t getUnitLength(const uint8_t * unit)
{
  uint8_t len = strnlen8((const char *)unit, UNIT_MAX_LEN);
  while (len > 0 && unit[len - 1] == ' ') len--;
  return len;
}

static bool allocValues(FieldProps * field, uint8_t len, const uint8_t * unit)
{
  uint8_t unitLen = unit ? getUnitLength(unit) : 0;
  if (len + unitLen > VALUES_BUFFER_SIZE - valuesBufferOffset)
  {
    return false;
  }
  field->valuesOffset = valuesBufferOffset;
  field->valuesLength = len;
  field->unitLength = unitLen;
  valuesBufferOffset += len + unitLen;
  return true;
}

// a changed unit never grows past its reserved room
static void storeUnit(FieldProps * field, const uint8_t * unit)
{
  if (field->valuesLength == 0) return;
  uint8_t len = getUnitLength(unit);
  if (len > field->unitLength) len = field->unitLength;
  field->unitLength = copyText(&valuesBuffer[field->valuesOffset + field->valuesLength], unit, len, len);
}

static void fieldSelectLoad(FieldProps * field, uint8_t * data, uint8_t offset)
{
  uint8_t len = strlen((char*)&data[offset]);
  uint8_t * values = &data[offset + len + 1];
  field->value = values[0];
  if (field->valuesLength == 0)
  {
    if (len == 0 || !allocValues(field, len, &values[4])) return;
    copyText(&valuesBuffer[field->valuesOffset], &data[offset], len, len);
  }
  // value, min, max, default, unit
  storeUnit(field, &values[4]);
}

static void fieldIntLoad(FieldProps * field, uint8_t * data, uint8_t offset)
{
  // value, min, max, default, unit
  field->value = data[offset];
  if (field->valuesLength == 0)
  {
    if (!allocValues(field, 2, &data[offset + 4])) return;
    valuesBuffer[field->valuesOffset] = data[offset + 1];
    valuesBuffer[field->valuesOffset + 1] = data[offset + 2];
  }
  storeUnit(field, &data[offset + 4]);
}

static void fieldStringLoad(FieldProps * field, uint8_t * data, uint8_t offset)
{
  if (field->valuesLength == 0)
  {
    uint8_t len = strnlen8((char*)&data[offset], DISPLAY_W / FW);
    if (len == 0 || !allocValues(field, len, 0)) return;
    copyText(&valuesBuffer[field->valuesOffset], &data[offset], len, len);
  }
}

static void fieldCommandLoad(FieldProps * field, uint8_t * data, uint8_t offset)
{
  field->value = data[offset];
  field->valuesOffset = data[offset+1];
  const uint8_t * info = &data[offset+2];
  uint8_t len = copyText((uint8_t *)popupText, info, strnlen8((const char *)info, INFO_TEXT_SIZE - 1), INFO_TEXT_SIZE - 1);
  popupText[len] = '\0';
  if (field->value == CMD_IDLE)
  {
    fieldPopup = 0;
  }
}

static void fieldValueSave(FieldProps * field)
{
  fieldWrite(field->id, field->value);
}

static void fieldCommandSave(FieldProps * field)
{
  if (field->value < CMD_CONFIRMED)
  {
    field->value = CMD_CLICK;
    fieldValueSave(field);
    fieldPopup = field;
    fieldPopup->valuesLength = 0;
    fieldTimeout = getTime() + field->valuesOffset;
  }
}

static void fieldFolderOpen(FieldProps * field)
{
  lineIndex = 1;
  pageOffset = 0;
  folderAccess = field->id;
  fields[backButtonId].parent = folderAccess;
  reloadAllField();
}

static void UIbackExec()
{
  folderAccess = 0;
  fields[backButtonId].parent = 255;
  reloadAllField();
  fields_count = 0;
}

static void parseDeviceInfoMessage(uint8_t* data, uint8_t length)
{
  // name, serial number (4), hardware id (4), firmware id (4), field count, parameter version
  if (data[2] != deviceId || length < 4)
  {
    return;
  }
  uint8_t offset = 3 + strnlen8((char*)&data[3], length - 3) + 1;
  if (offset + 13 > length)
  {
    return;
  }
  uint8_t len = copyText((uint8_t *)deviceName, &data[3], offset - 4, DEVICE_NAME_SIZE - 1);
  deviceName[len] = '\0';
  deviceIsELRS_TX = (memcmp(&data[offset], "ELRS", 4) == 0);
  uint8_t newFieldCount = data[offset+12];
  // room for "Other Devices" and "Back"
  if (newFieldCount > FIELDS_MAX_COUNT - 2)
  {
    newFieldCount = FIELDS_MAX_COUNT - 2;
  }
  reloadAllField();
  if (newFieldCount != fields_count || newFieldCount == 0)
  {
    fields_count = newFieldCount;
    allocateFields();
    // hidden: switching devices is not supported
    fields[fields_count].id = fields_count + 1;
    fields[fields_count].nameLength = 1;
    fields[fields_count].parent = 255;
    fields[fields_count].type = TYPE_DEVICE_FOLDER;
    if (newFieldCount == 0)
    {
      allParamsLoaded = 1;
      fieldId = 1;
    }
  }
}

// Drops "(...)" from selection options while the chunks arrive ("250Hz(-108dBm)" ->
// "250Hz"). Only the options string is filtered: the bytes after it are binary.
static void appendFieldData(uint8_t c)
{
  enum { PARENT, TYPE, NAME, OPTIONS, RAW };
  switch (fieldDataState)
  {
    case PARENT:
      fieldDataState = TYPE;
      break;
    case TYPE:
      fieldDataType = c & 0x7F;
      fieldDataState = NAME;
      break;
    case NAME:
      if (c == 0) fieldDataState = (fieldDataType == TYPE_SELECT) ? OPTIONS : RAW;
      break;
    case OPTIONS:
      if (c == 0)
      {
        fieldDataState = RAW;
        fieldDataInParens = 0;
      }
      else if (fieldDataInParens)
      {
        if (c == ')') fieldDataInParens = 0;
        return;
      }
      else if (c == '(')
      {
        fieldDataInParens = 1;
        if (fieldData[fieldDataLen - 1] == ' ') fieldDataLen--;
        return;
      }
      break;
  }
  if (fieldDataLen < FIELD_DATA_MAX_LEN)
  {
    fieldData[fieldDataLen++] = c;
  }
}

static void parseParameterInfoMessage(uint8_t* data, uint8_t length)
{
  if (data[2] != deviceId || data[3] != fieldId || fieldId > fields_count)
  {
    fieldDataLen = 0;
    fieldChunk = 0;
    return;
  }
  if (fieldDataLen == 0)
  {
    expectedChunks = -1;
    fieldDataState = 0;
    fieldDataInParens = 0;
  }
  FieldProps* field = &fields[fieldId - 1];
  uint8_t chunks = data[4];
  if (chunks != expectedChunks && expectedChunks != -1)
  {
    return;
  }
  expectedChunks = chunks - 1;
  for (uint32_t i = 5; i < length; i++)
  {
    appendFieldData(data[i]);
  }
  if (chunks > 0)
  {
    fieldChunk = fieldChunk + 1;
    return;
  }

  fieldChunk = 0;
  if (fieldDataLen < 4)
  {
    fieldDataLen = 0;
    return;
  }
  memset(&fieldData[fieldDataLen], 0, FIELD_DATA_SIZE - fieldDataLen);
  fieldDataLen = 0;
  field->id = fieldId;
  uint8_t parent = fieldData[0];
  uint8_t type = fieldData[1] & 0x7F;
  uint8_t hidden = fieldData[1] & 0x80;
  if (field->nameLength != 0 && (field->parent != parent || field->type != type))
  {
    return;
  }
  field->parent = parent;
  field->type = type;
  uint8_t nameLen = strlen((char*)&fieldData[2]);
  uint8_t offset = nameLen + 1 + 2;

  bool shown = !hidden && parent == folderAccess;
  if (shown && field->nameLength == 0)
  {
    if (nameLen > NAMES_BUFFER_SIZE - namesBufferOffset)
    {
      shown = false;
    }
    else
    {
      field->nameOffset = namesBufferOffset;
      field->nameLength = copyText(&namesBuffer[namesBufferOffset], &fieldData[2], nameLen, nameLen);
      namesBufferOffset += field->nameLength;
    }
  }
  if (shown)
  {
    switch (type)
    {
      case TYPE_UINT8:
      case TYPE_INT8:
        fieldIntLoad(field, fieldData, offset);
        break;
      case TYPE_SELECT:
        fieldSelectLoad(field, fieldData, offset);
        break;
      case TYPE_STRING:
      case TYPE_INFO:
        fieldStringLoad(field, fieldData, offset);
        break;
      case TYPE_FOLDER:
        break;
      case TYPE_COMMAND:
        fieldCommandLoad(field, fieldData, offset);
        break;
      default:
        shown = false; // 16 bit, float: not supported
        break;
    }
    // values buffer full
    if (type != TYPE_FOLDER && type != TYPE_COMMAND && field->valuesLength == 0)
    {
      shown = false;
    }
  }
  if (!shown)
  {
    field->nameLength = 0;
  }

  if (fieldPopup == 0)
  {
    if (fieldId == fields_count)
    {
      allParamsLoaded = 1;
      fieldId = 1;
    }
    else if (allParamsLoaded == 0)
    {
      fieldId++;
    }
    fieldTimeout = getTime() + 200;
  }
  else
  {
    fieldTimeout = getTime() + fieldPopup->valuesOffset;
  }
}

static void parseElrsInfoMessage(uint8_t* data, uint8_t length)
{
  if (data[2] != deviceId)
  {
    return;
  }
  badPkt = data[3];
  goodPkt = (data[4] << 8) + data[5];
  if (data[6] != elrsFlags)
  {
    elrsFlags = data[6];
    titleShowWarnTimeout = 0;
  }
  uint8_t len = (length > 7) ? copyText((uint8_t *)elrsFlagsInfo, &data[7],
      strnlen8((const char *)&data[7], length - 7), INFO_TEXT_SIZE - 1) : 0;
  elrsFlagsInfo[len] = '\0';
}

// Called with a frame from the UART ISR, and without one from the menu loop
void runCrossfireTelemetryCallback(uint8_t command = 0, uint8_t* data = 0, uint8_t length = 0)
{
  if (command == 0x29)
  {
    parseDeviceInfoMessage(data, length);
  }
  else if (command == 0x2B)
  {
    parseParameterInfoMessage(data, length);
    if (allParamsLoaded < 1)
    {
      fieldTimeout = 0;
    }
  }
  else if (command == 0x2E)
  {
    parseElrsInfoMessage(data, length);
  }
  if (!menuActive)
  {
    return;
  }
  if (pendingWrite)
  {
    flushPendingWrite();
    return;
  }
  tmr10ms_t time = getTime();
  if (fieldPopup != 0)
  {
    if (time > fieldTimeout && fieldPopup->value != CMD_ASK_CONFIRM)
    {
      if (crossfireTelemetryPush4(0x2D, fieldPopup->id, CMD_QUERY))
        fieldTimeout = time + fieldPopup->valuesOffset;
    }
  }
  else if (time > devicesRefreshTimeout && fields_count < 1)
  {
    if (crossfireTelemetryPing())
      devicesRefreshTimeout = time + 100;
  }
  else if (time > linkstatTimeout && allParamsLoaded)
  {
    if (deviceIsELRS_TX)
    {
      crossfireTelemetryPush4(0x2D, 0, 0); // request link statistics
    }
    linkstatTimeout = time + 100;
  }
  else if (time > fieldTimeout && fields_count != 0 && !edit && allParamsLoaded < 1)
  {
    if (crossfireTelemetryPush4(0x2C, fieldId, fieldChunk))
      fieldTimeout = time + 50;
  }
}

static void handleDevicePageEvent(uint8_t event)
{
  if (fieldsLen == 0 || fields[backButtonId].nameLength == 0)
  {
    return;
  }
  if (event == EVT_VIRTUAL_EXIT)
  {
    if (edit)
    {
      // cancel: reload the stored value
      edit = 0;
      FieldProps * field = getField(lineIndex);
      if (field)
      {
        fieldId = field->id;
        fieldChunk = 0;
        fieldDataLen = 0;
        allParamsLoaded = 0;
        fieldTimeout = 0;
      }
    }
    else
    {
      if (folderAccess == 0 && allParamsLoaded == 1)
      {
        crossfireTelemetryPing();
      }
      UIbackExec();
    }
  }
  else if (event == EVT_VIRTUAL_ENTER)
  {
    if (elrsFlags > 0x1F)
    {
      // acknowledge the error
      elrsFlags = 0;
      fieldWrite(0x2E, 0x00);
      return;
    }
    FieldProps * field = getField(lineIndex);
    if (field == 0 || field->nameLength == 0)
    {
      return;
    }
    if (isFieldEditable(field))
    {
      edit = 1 - edit;
      if (!edit)
      {
        fieldValueSave(field);
        // other options may depend on it; give the module time to save
        reloadAllField();
        fieldTimeout = getTime() + 20;
        linkstatTimeout = fieldTimeout + 100;
      }
    }
    else if (field->type == TYPE_FOLDER)
    {
      fieldFolderOpen(field);
    }
    else if (field->type == TYPE_COMMAND)
    {
      fieldId = field->id;
      fieldChunk = 0;
      fieldDataLen = 0;
      fieldCommandSave(field);
    }
    else if (field->type == TYPE_BACK)
    {
      UIbackExec();
    }
  }
  else if (edit)
  {
    if (event == EVT_VIRTUAL_NEXT)
    {
      incrField(1);
    }
    else if (event == EVT_VIRTUAL_PREV)
    {
      incrField(-1);
    }
  }
  else
  {
    if (event == EVT_VIRTUAL_NEXT)
    {
      selectField(-1);
    }
    else if (event == EVT_VIRTUAL_PREV)
    {
      selectField(1);
    }
  }
}

static uint8_t utoa8(char * s, uint16_t value)
{
  char tmp[5];
  uint8_t n = 0, len = 0;
  do
  {
    tmp[n++] = '0' + value % 10;
    value /= 10;
  } while (value);
  while (n) s[len++] = tmp[--n];
  return len;
}

static void drawTitle()
{
  tmr10ms_t time = getTime();
  if (time > titleShowWarnTimeout)
  {
    // a warning alternates with the title
    titleShowWarn = (elrsFlags > 3 && !titleShowWarn);
    titleShowWarnTimeout = time + 100;
  }
  if (titleShowWarn)
  {
    drawClipped(0, 0, elrsFlagsInfo, strlen(elrsFlagsInfo), INVERS);
    return;
  }
  if (fields_count == 0 || !allParamsLoaded)
  {
    lcd_putsAtt(0, 0, "Loading...", 0);
  }
  else
  {
    lcd_putsAtt(0, 0, deviceName, 0);
  }
  if (deviceIsELRS_TX)
  {
    char s[12];
    uint8_t len = utoa8(s, badPkt);
    s[len++] = '/';
    len += utoa8(&s[len], goodPkt);
    s[len++] = ' ';
    s[len++] = (elrsFlags & 1) ? 'C' : '-';
    lcd_putsnAtt(DISPLAY_W - len * FW, 0, s, len, 0);
  }
}

// returns the value's x; a wide value moves left over the name
static uint8_t drawFieldValue(FieldProps * field, uint8_t y, uint8_t attr)
{
  const char * text = (const char *)&valuesBuffer[field->valuesOffset];
  uint8_t len = field->valuesLength;
  char number[5];
  if (field->type == TYPE_SELECT)
  {
    len = getOption(field, field->value, &text);
  }
  else if (field->type == TYPE_UINT8 || field->type == TYPE_INT8)
  {
    int16_t value = (field->type == TYPE_INT8) ? (int8_t)field->value : field->value;
    len = 0;
    if (value < 0)
    {
      number[len++] = '-';
      value = -value;
    }
    len += utoa8(&number[len], value);
    text = number;
  }
  uint8_t start = COL2;
  if (len * FW > DISPLAY_W - COL2)
  {
    start = (len * FW > DISPLAY_W - 4*FW) ? 4*FW : DISPLAY_W - len * FW;
  }
  uint8_t x = drawClipped(start, y, text, len, attr);
  drawClipped(x, y, (const char *)&valuesBuffer[field->valuesOffset + field->valuesLength], field->unitLength, 0);
  return start;
}

static void runDevicePage(uint8_t event)
{
  handleDevicePageEvent(event);
  drawTitle();
  if (elrsFlags > 0x1F)
  {
    lcd_putsAtt(0, 2*FH, "Error:", 0);
    drawClipped(0, 3*FH, elrsFlagsInfo, strlen(elrsFlagsInfo), 0);
    lcd_putsAtt(9*FW, 5*FH, "[OK]", BLINK | INVERS);
    return;
  }
  for (uint32_t y = 1; y < MAX_LINE_INDEX+2; y++)
  {
    if (pageOffset+y >= fieldsLen) break;
    FieldProps * field = getField(pageOffset+y);
    if (field == 0)
    {
      break;
    }
    uint8_t ly = y*FH+TEXT_Y_OFFSET;
    uint8_t attr = (lineIndex == (pageOffset+y)) ? (edit ? BLINK : INVERS) : 0;
    const char * name = (const char *)&namesBuffer[field->nameOffset];
    if (field->type == TYPE_FOLDER || field->type == TYPE_COMMAND)
    {
      uint8_t x = drawClipped(0, ly, (field->type == TYPE_FOLDER) ? ">" : "[", 1, attr);
      x = drawClipped(x, ly, name, field->nameLength, attr);
      if (field->type == TYPE_COMMAND) drawClipped(x, ly, "]", 1, attr);
    }
    else if (field->type == TYPE_BACK)
    {
      lcd_putsAtt(0, ly, "[Back]", attr);
    }
    else
    {
      uint8_t valueX = drawFieldValue(field, ly, attr);
      uint8_t nameLen = field->nameLength;
      if (nameLen > valueX / FW - 1) nameLen = valueX / FW - 1;
      lcd_putsnAtt(0, ly, name, nameLen, 0);
    }
  }
}

static uint8_t popupCompat(uint8_t event)
{
  lcd_putsAtt(0, 3*FH, popupText, 0);
  if (event == EVT_VIRTUAL_EXIT)
  {
    return CMD_CANCEL;
  }
  else if (event == EVT_VIRTUAL_ENTER)
  {
    return CMD_CONFIRMED;
  }
  return 0;
}

static void runPopupPage(uint8_t event)
{
  if (event == EVT_VIRTUAL_EXIT)
  {
    fieldWrite(fieldPopup->id, CMD_CANCEL);
    fieldTimeout = getTime() + 200;
  }
  uint8_t result = 0;
  if (fieldPopup->value == CMD_IDLE && fieldPopup->valuesLength != 0)
  {
    popupCompat(event);
    reloadAllField();
    fieldPopup = 0;
  }
  else if (fieldPopup->value == CMD_ASK_CONFIRM)
  {
    result = popupCompat(event);
    fieldPopup->valuesLength = fieldPopup->value;
    if (result == CMD_CONFIRMED)
    {
      fieldWrite(fieldPopup->id, CMD_CONFIRMED);
      fieldTimeout = getTime() + fieldPopup->valuesOffset;
      fieldPopup->value = CMD_CONFIRMED;
    }
    else if (result == CMD_CANCEL)
    {
      fieldPopup = 0;
    }
  }
  else if (fieldPopup->value == CMD_EXECUTING)
  {
    result = popupCompat(event);
    fieldPopup->valuesLength = fieldPopup->value;
    if (result == CMD_CANCEL)
    {
      fieldWrite(fieldPopup->id, CMD_CANCEL);
      fieldTimeout = getTime() + fieldPopup->valuesOffset;
      fieldPopup = 0;
    }
  }
}

static void ELRS_stop()
{
  menuActive = 0;
  UIbackExec();
  fieldPopup = 0;
  edit = 0;
  deviceIsELRS_TX = 0;
  elrsFlags = 0;
  popMenu(false);
}

static void ELRS_run(uint8_t event)
{
  menuActive = 1;
  if (event == EVT_KEY_LONG(KEY_EXIT))
  {
    ELRS_stop();
    return;
  }
  if (fieldPopup != 0)
  {
    drawTitle();
    runPopupPage(event);
  }
  else
  {
    runDevicePage(event);
  }
  runCrossfireTelemetryCallback();
}

void crossfileMenu(MState2 &mstate2, uint8_t event, uint8_t sub, uint8_t subN, uint8_t y)
{
  mstate2.check_columns(event, 1);
  lcd_puts_Pleft(y, "[CRSF Setup]");
  if (sub == subN)
  {
    lcd_char_inverse(0, y, 12*FW, 0);
    if (event == EVT_KEY_FIRST(KEY_MENU))
    {
      killEvents(event); // the release must not reach the new menu as ENTER
      pushMenu(ELRS_run);
    }
  }
}
