// Modified 2026 by sergegrin
/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <MKL16Z4.h>
#include "crossfire.h"
#include "../er9x.h"
#include <string.h>
#include "../iface_a7105.h"

// written from the main loop and the UART ISR, sent from the pulse timer ISR;
// a non-zero size means a complete frame is waiting
static uint8_t outputTelemetryBuffer[TELEMETRY_OUTPUT_FIFO_SIZE];
static volatile uint8_t outputTelemetryBufferSize = 0;

static uint8_t telemetryRxBuffer[TELEMETRY_RX_PACKET_SIZE];
static uint8_t telemetryRxBufferCount = 0;

static uint8_t crossfireFrame[CROSSFIRE_FRAME_MAXLEN];

#define CROSSFIRE_CH_CENTER 0x3E0
#define CROSSFIRE_CH_BITS 11

// Range for pulses (channels output) is [-1024:+1024]
static uint8_t createCrossfireChannelsFrame(uint8_t* frame, int16_t* pulses) {
  uint8_t* buf = frame;
  *buf++ = MODULE_ADDRESS;
  *buf++ = 24;  // 1(ID) + 22 + 1(CRC)
  uint8_t* crc_start = buf;
  *buf++ = CHANNELS_ID;
  uint32_t bits = 0;
  uint8_t bitsavailable = 0;
  for (int i = 0; i < CROSSFIRE_CHANNELS_COUNT; i++) {
    uint32_t val = limit(0, CROSSFIRE_CH_CENTER + (((pulses[i]) * 4) / 5), 2 * CROSSFIRE_CH_CENTER);
    bits |= val << bitsavailable;
    bitsavailable += CROSSFIRE_CH_BITS;
    while (bitsavailable >= 8) {
      *buf++ = bits;
      bits >>= 8;
      bitsavailable -= 8;
    }
  }

  *buf++ = crc8(crc_start, 23);
  return buf - frame;
}

static uint8_t setupPulsesCrossfire() {
  uint8_t length = outputTelemetryBufferSize;
  if (length > 0) {
    memcpy(crossfireFrame, outputTelemetryBuffer, length);
    outputTelemetryBufferSize = 0;
    return length;
  }
  return createCrossfireChannelsFrame(crossfireFrame, g_chans512);
}

static const uint8_t idx_to_afhds2a[] = {
  FST_IDX_ERR,            // RX_RSSI1_INDEX
  FST_IDX_NOISE,          // RX_RSSI2_INDEX
  FST_IDX_RSSI,           // RX_QUALITY_INDEX
  FST_IDX_SNR,            // RX_SNR_INDEX
  0xff,                   // RX_ANTENNA_INDEX
  FST_IDX_S86,            // RF_MODE_INDEX
  FST_IDX_S87,            // TX_POWER_INDEX
  FST_IDX_TSSI,           // TX_RSSI_INDEX
  FST_IDX_S88,            // TX_QUALITY_INDEX
  FST_IDX_S89,            // TX_SNR_INDEX
  0xff,                   // RX_RSSI_PERC_INDEX
  FST_IDX_TX_V,           // RX_RF_POWER_INDEX
  0xff,                   // TX_RSSI_PERC_INDEX
  FST_IDX_S8a,            // TX_RF_POWER_INDEX
  FST_IDX_S85,            // TX_FPS_INDEX
  FST_IDX_INTV,           // BATT_VOLTAGE_INDEX
  FST_IDX_BAT_CURR,       // BATT_CURRENT_INDEX
  FST_IDX_THRCAP,         // BATT_CAPACITY_INDEX
  FST_IDX_FUEL,           // BATT_REMAINING_INDEX
  FST_IDX_GPS_LAT,        // GPS_LATITUDE_INDEX
  FST_IDX_GPS_LON,        // GPS_LONGITUDE_INDEX
  FST_IDX_GROUND_SPEED,   // GPS_GROUND_SPEED_INDEX
  FST_IDX_CMP_HEAD,       // GPS_HEADING_INDEX
  FST_IDX_GPS_ALT,        // GPS_ALTITUDE_INDEX
  FST_IDX_GPS_STATUS,     // GPS_SATELLITES_INDEX
  FST_IDX_PITCH,          // ATTITUDE_PITCH_INDEX
  FST_IDX_ROLL,           // ATTITUDE_ROLL_INDEX
  FST_IDX_YAW,            // ATTITUDE_YAW_INDEX
  FST_IDX_FLIGHT_MODE,    // FLIGHT_MODE_INDEX
  FST_IDX_VERTICAL_SPEED, // VERTICAL_SPEED_INDEX
};

static void processCrossfireTelemetryValue(uint8_t index, int32_t value) {
  uint8_t tidx;
  if (index < UNKNOWN_INDEX && (tidx = idx_to_afhds2a[index]) != 0xff) {
    AFHDS2A_tel_data[tidx] = value;
    AFHDS2A_tel_status |= ((uint64_t)1 << tidx);
  }
}

static bool checkCrossfireTelemetryFrameCRC() {
  uint8_t len = telemetryRxBuffer[1];
  uint8_t crc = crc8(&telemetryRxBuffer[2], len - 1);
  return (crc == telemetryRxBuffer[len + 1]);
}

static bool getCrossfireTelemetryValue(uint8_t index, int32_t &value, int N) {
  bool result = false;
  uint8_t *byte = &telemetryRxBuffer[index];
  value = (*byte & 0x80) ? -1 : 0;
  for (uint8_t i = 0; i < N; i++) {
    value <<= 8;
    if (*byte != 0xff) {
      result = true;
    }
    value += *byte++;
  }
  return result;
}

static void processCrossfireTelemetryFrame() {
  if (!checkCrossfireTelemetryFrameCRC()) {
    return;
  }

  uint8_t id = telemetryRxBuffer[2];
  int32_t value;
  switch (id) {
    case CF_VARIO_ID:
      if (getCrossfireTelemetryValue(3, value, 2))
        processCrossfireTelemetryValue(VERTICAL_SPEED_INDEX, value);
      break;

    case GPS_ID:
      if (getCrossfireTelemetryValue(3, value, 4))
        processCrossfireTelemetryValue(GPS_LATITUDE_INDEX, value / 100);
      if (getCrossfireTelemetryValue(7, value, 4))
        processCrossfireTelemetryValue(GPS_LONGITUDE_INDEX, value / 100);
      if (getCrossfireTelemetryValue(11, value, 2))
        processCrossfireTelemetryValue(GPS_GROUND_SPEED_INDEX, value);
      if (getCrossfireTelemetryValue(13, value, 2))
        processCrossfireTelemetryValue(GPS_HEADING_INDEX, value);
      if (getCrossfireTelemetryValue(15, value, 2))
        processCrossfireTelemetryValue(GPS_ALTITUDE_INDEX, value - 1000);
      if (getCrossfireTelemetryValue(17, value, 1))
        processCrossfireTelemetryValue(GPS_SATELLITES_INDEX, value);
      break;

    case LINK_ID:
      for (unsigned int i = 0; i <= TX_SNR_INDEX; i++) {
        if (getCrossfireTelemetryValue(3 + i, value, 1)) {
          if (i == TX_POWER_INDEX) {
            static const int32_t power_values[] = {0, 10, 25, 100, 500, 1000, 2000, 250, 50};
            value = ((unsigned)value < DIM(power_values) ? power_values[value] : 0);
          }
          processCrossfireTelemetryValue(i, value);
        }
      }
      break;

    case LINK_RX_ID:
      if (getCrossfireTelemetryValue(4, value, 1))
        processCrossfireTelemetryValue(RX_RSSI_PERC_INDEX, value);
      if (getCrossfireTelemetryValue(7, value, 1))
        processCrossfireTelemetryValue(TX_RF_POWER_INDEX, value);
      break;

    case LINK_TX_ID:
      if (getCrossfireTelemetryValue(4, value, 1))
        processCrossfireTelemetryValue(TX_RSSI_PERC_INDEX, value);
      if (getCrossfireTelemetryValue(7, value, 1))
        processCrossfireTelemetryValue(RX_RF_POWER_INDEX, value);
      if (getCrossfireTelemetryValue(8, value, 1))
        processCrossfireTelemetryValue(TX_FPS_INDEX, value * 10);
      break;

    case BATTERY_ID:
      if (getCrossfireTelemetryValue(3, value, 2))
        processCrossfireTelemetryValue(BATT_VOLTAGE_INDEX, value * 10);
      if (getCrossfireTelemetryValue(5, value, 2))
        processCrossfireTelemetryValue(BATT_CURRENT_INDEX, value * 10);
      if (getCrossfireTelemetryValue(7, value, 3))
        processCrossfireTelemetryValue(BATT_CAPACITY_INDEX, value * 10);
      if (getCrossfireTelemetryValue(10, value, 1))
        processCrossfireTelemetryValue(BATT_REMAINING_INDEX, value * 10);
      break;

    case ATTITUDE_ID:
      if (getCrossfireTelemetryValue(3, value, 2))
        processCrossfireTelemetryValue(ATTITUDE_PITCH_INDEX, value);
      if (getCrossfireTelemetryValue(5, value, 2))
        processCrossfireTelemetryValue(ATTITUDE_ROLL_INDEX, value);
      if (getCrossfireTelemetryValue(7, value, 2))
        processCrossfireTelemetryValue(ATTITUDE_YAW_INDEX, value);
      break;

    case FLIGHT_MODE_ID:
    case RADIO_ID: // timing correction: not used
      break;

    default:
      // <Device address 0><Frame length 1><Type 2><Payload 3><CRC>
      // destination address and CRC are skipped
      runCrossfireTelemetryCallback(telemetryRxBuffer[2], telemetryRxBuffer + 2, telemetryRxBuffer[1] - 1);
      break;
  }
}

static int processCrossfireTelemetryData(uint8_t data) {
  if (telemetryRxBufferCount == 0 && data != RADIO_ADDRESS && data != UART_SYNC) {
    return 0;
  }

  if (telemetryRxBufferCount == 1 && (data < 2 || data > TELEMETRY_RX_PACKET_SIZE - 2)) {
    telemetryRxBufferCount = 0;
    return 0;
  }

  if (telemetryRxBufferCount < TELEMETRY_RX_PACKET_SIZE) {
    telemetryRxBuffer[telemetryRxBufferCount++] = data;
  } else {
    telemetryRxBufferCount = 0;
  }

  if (telemetryRxBufferCount > 4) {
    uint8_t length = telemetryRxBuffer[1];
    if (length > 0 && length + 2 == telemetryRxBufferCount) {
      processCrossfireTelemetryFrame();
      telemetryRxBufferCount = 0;
      return 1;
    }
  }

  return 0;
}

bool crossfireTelemetryPush(uint8_t command, uint8_t *data, uint8_t length) {
  // masked: the pulse ISR must not send a half-written frame
  bool pushed = false;
  uint32_t primask = __get_PRIMASK();
  __disable_irq();
  if (outputTelemetryBufferSize == 0) {
    uint8_t *buf = outputTelemetryBuffer;
    *buf++ = MODULE_ADDRESS;
    *buf++ = 2 + length;  // 1(COMMAND) + data length + 1(CRC)
    *buf++ = command;
    memcpy(buf, data, length);
    buf += length;
    *buf = crc8(outputTelemetryBuffer + 2, 1 + length);
    outputTelemetryBufferSize = 4 + length;
    pushed = true;
  }
  __set_PRIMASK(primask);
  return pushed;
}

// Model ID for ExpressLRS "Model Match" = model number; repeated in case the
// module powers up later
static void crsfSendModelId()
{
  uint8_t data[6] = { MODULE_ADDRESS, RADIO_ADDRESS, SUBCOMMAND_CRSF, COMMAND_MODEL_SELECT_ID,
                      (uint8_t)(g_eeGeneral.currModel + 1), 0 };
  uint8_t crcData[7] = { COMMAND_ID, data[0], data[1], data[2], data[3], data[4] };
  data[5] = crc8_BA(crcData, 6);
  crossfireTelemetryPush(COMMAND_ID, data, sizeof(data));
}

#define MODEL_ID_PERIOD   (5000 / CROSSFIRE_PERIOD)
static uint16_t modelIdCounter;

void crsf_init()
{
  modelIdCounter = MODEL_ID_PERIOD - 1000 / CROSSFIRE_PERIOD; // first one after 1 s
  setup_crsf_serial_port(CROSSFIRE_BAUDRATE, processCrossfireTelemetryData);
  SetPRTTimPeriod(PROTO_CRSF);
}

void crsf_shutdown()
{
  shutdown_crsf_serial_port();
}

// Called from the pulse timer ISR
void crsf_action()
{
  if (++modelIdCounter >= MODEL_ID_PERIOD) {
    modelIdCounter = 0;
    crsfSendModelId();
  }
  crsf_send_data(crossfireFrame, setupPulsesCrossfire());
}
