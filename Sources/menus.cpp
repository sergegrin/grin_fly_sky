// Modified 2026 by sergegrin
/*
 * Author - Erez Raviv <erezraviv@gmail.com>
 *
 * Based on th9x -> http://code.google.com/p/th9x/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "er9x.h"
#include <stdlib.h>
#include "templates.h"
#include "menus.h"
#include "pulses.h"
#include "voice.h"
#include <stdio.h>
/*****************************************************************************/
#include "iface_a7105.h"
/*****************************************************************************/
#include "crossfire/crossfire.h"


#include "language.h"

#define TELEMETRY_ARDUPILOT	5

#define GLOBAL_COUNTDOWN	1

#define THROTTLE_TRACE		0

union t_xmem Xmem ;

#define ALPHA_NO_NAME		0x80

struct t_popupData PopupData ;

struct t_menuControl
{
	uint8_t SubmenuIndex ;
	uint8_t LastSubmenuIndex ;
	uint8_t UseLastSubmenuIndex ;
} MenuControl ;

static uint8_t IlinesCount ;

extern int16_t AltOffset;
extern int16_t GAltOffset;
extern uint8_t SystemOptions ;
static uint8_t s_currIdx;
static uint8_t StatusTimer ;

NOINLINE void resetTimer1(void) ;
void crossfileMenu(MState2 &mstate2,uint8_t  event, uint8_t sub,  uint8_t subN, uint8_t y );

struct t_timer s_timer[2] ;

uint8_t RotaryState ;		// Defaults to ROTARY_MENU_LR
uint8_t CalcScaleNest = 0 ;
uint8_t ThrottleStickyOn = 0 ;


const prog_char APM Str_ALTeq[] = STR_ALTEQ ;
const prog_char APM Str_RXeq[] =  STR_RXEQ ;
const prog_char APM Str_TRE012AG[] = STR_TRE012AG ;
const prog_char APM Str_A_eq[] =  STR_A_EQ ;
const prog_char APM Str_Timer[] =  STR_TIMER ;
const prog_char APM Str_Sounds[] = STR_SOUNDS ;
const prog_char APM Str_Name[] = STR_NAME ;

const prog_char APM Str_Chans_Gv[] = STR_CHANS_GV ;

const prog_char APM Str_Cswitch[] = CSWITCH_STR ;
const prog_char APM Str_On_Off_Both[] = STR_VOICE_V2OPT ;
const prog_char APM Str_Gv_Source[] = STR_GV_SOURCE ;

const prog_char APM Str_minute_Beep[] = STR_MINUTE_BEEP ;
const prog_char APM Str_Beep_Countdown[] = STR_BEEP_COUNTDOWN ;

const prog_char APM Str_Main_Popup[] = STR_MAIN_POPUP ;
#define Str_Radio_Setup		&Str_Main_Popup[RADIO_SETUP_OFFSET]
#define Str_Model_Setup		&Str_Main_Popup[MODEL_SETUP_OFFSET]

// Strings for titles and indices
const prog_char APM Str_Telemetry[] = STR_TELEMETRY ;
const prog_char APM Str_limits[] = STR_LIMITS ;
const prog_char APM Str_heli_setup[] = STR_HELI_SETUP ;
const prog_char APM Str_Expo[] = STR_EXPO_DR ;
const prog_char APM Str_Modes[] = STR_MODES ;
const prog_char APM Str_Curves[] = STR_CURVES ;
const prog_char APM Str_Curve[] = STR_CURVE ;
const prog_char APM Str_Safety[] = STR_SAFETY_SW2 ;
const prog_char APM Str_Globals[] = STR_GLOBAL_VARS ;
const prog_char APM Str_Protocol[] = STR_PROTOCOL ;
const prog_char APM Str_1_RETA[] = STR_1_RETA ;

const prog_char APM Str_Mixer[] = STR_MIXER2 ;

const prog_char APM Curve_Str[] = CURV_STR ;

static uint8_t Columns ;


int8_t phyStick[4] ;

const prog_uint8_t APM UnitsVoice[] = {V_FEET,V_VOLTS,V_DEGREES,V_DEGREES,0,V_AMPS,V_METRES,V_WATTS } ;
const prog_char APM UnitsText[] = { 'F','V','C','F','m','A','m','W' } ;
const prog_char APM UnitsString[] = "\005Feet VoltsDeg_CDeg_FmAh  Amps MeterWatts" ;

const prog_char APM Str_telemItems[] = STR_TELEM_ITEMS ;
const prog_int8_t APM TelemIndex[] = {
    FST_IDX_SNR,
    FST_IDX_NOISE,
    FST_IDX_RSSI,
    FST_IDX_ERR,
    FST_IDX_TSSI,

    FST_IDX_INTV,
    FST_IDX_TEM,
    FST_IDX_RPM,
    FST_IDX_EXTV,
    FST_IDX_CELL,
    FST_IDX_BAT_CURR,
    FST_IDX_FUEL,
    FST_IDX_THRCAP,
    FST_IDX_CMP_HEAD,
    FST_IDX_CLIMB_RATE,
    FST_IDX_COG,
    FST_IDX_GPS_STATUS,
    FST_IDX_ACC_X,
    FST_IDX_ACC_Y,
    FST_IDX_ACC_Z,
    FST_IDX_ROLL,
    FST_IDX_PITCH,
    FST_IDX_YAW,
    FST_IDX_VERTICAL_SPEED,
    FST_IDX_GROUND_SPEED,
    FST_IDX_GPS_DIST,
    FST_IDX_ARMED,
    FST_IDX_FLIGHT_MODE,

    FST_IDX_PRES,
    FST_IDX_ODO1,
    FST_IDX_ODO2,
    FST_IDX_SPE,
    FST_IDX_TX_V,

    //4 byte sensors
    FST_IDX_GPS_LAT,
    FST_IDX_GPS_LON,
    FST_IDX_GPS_ALT,
    FST_IDX_ALT,
    FST_IDX_ALT_MAX,
    FST_IDX_S85,
    FST_IDX_S86,
    FST_IDX_S87,
    FST_IDX_S88,
    FST_IDX_S89,
    FST_IDX_S8a,
    TMOK,
    V_GVAR1,
    V_GVAR2,
    V_GVAR3,
    V_GVAR4,
    V_GVAR5,
    V_GVAR6,
    V_GVAR7,
    V_SC1,
    V_SC2,
    V_SC3,
    V_SC4,
    TIMER1,
    TIMER2};

const prog_uint8_t APM TelemValid[] = { 1, 1, 1, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 3, 3, 3, 3, 0 } ;


#define EDIT_DR_SWITCH_EDIT	0x01
#define EDIT_DR_SWITCH_MOMENT	0x02
int8_t edit_dr_switch( uint8_t x, uint8_t y, int8_t drswitch, uint8_t attr, uint8_t edit ) ;

extern MenuFuncP g_menuStack[] ;
extern uint8_t g_menuStackPtr ;
static MenuFuncP lastPopMenu()
{
    return  g_menuStack[g_menuStackPtr+1];
}


void putsAttIdxTelemItems( uint8_t x, uint8_t y, uint8_t index, uint8_t attr )
{

	lcd_putsAttIdx( x, y, Str_telemItems, index, attr ) ;

}


int16_t calc_scaler( uint8_t index, uint8_t *unit, uint8_t *num_decimals)
{
	int32_t value ;
	uint8_t lnest ;
	ScaleData *pscaler ;

	lnest = CalcScaleNest ;
	if ( lnest > 5 )
	{
		return 0 ;
	}
	CalcScaleNest = lnest + 1 ;
	// process
	pscaler = &g_model.Scalers[index] ;
	if ( pscaler->source )
	{
		value = getValue( pscaler->source - 1 ) ;
	}
	else
	{
		value = 0 ;
	}
	CalcScaleNest = lnest ;
	if ( !pscaler->offsetLast )
	{
		value += pscaler->offset ;
	}
	value *= pscaler->mult+1 ;
	value /= pscaler->div+1 ;
	if ( pscaler->offsetLast )
	{
		value += pscaler->offset ;
	}
	if ( pscaler->neg )
	{
		value = -value ;
	}
	if ( unit )
	{
		*unit = pscaler->unit ;
	}
	if ( num_decimals )
	{
		*num_decimals = pscaler->precision ;
	}

	return value ;
}

uint8_t telemItemValid( uint8_t index )
{
	return 1 ;
}


void voiceMinutes( int16_t value )
{
	voice_numeric( value, 0, (abs(value) == 1) ? V_MINUTE : V_MINUTES ) ;
}

void voice_telem_item( uint8_t indexIn )
{
	int16_t value ;
	uint8_t spoken = 0 ;
	uint8_t unit = 0 ;
	uint8_t num_decimals = 0 ;

	value = get_telemetry_value( indexIn ) ;
	if (telemItemValid( indexIn ) == 0 )
	{
		putVoiceQueue( V_NOTELEM ) ;
		spoken = 1 ;
	}

          if(indexIn >0 &&  indexIn < FST_IDX_S8a)
        {
            num_decimals =AFHDS2A_tel[indexIn].prec;
        }

	int8_t index = pgm_read_byte( &TelemIndex[indexIn] ) ;

  switch (index)
	{
		case V_SC1 :
		case V_SC2 :
		case V_SC3 :
		case V_SC4 :
			value = calc_scaler( index-V_SC1, &unit, &num_decimals ) ;
			unit = pgm_read_byte( &UnitsVoice[unit]) ;
		break ;

		case BATTERY:

			unit = V_VOLTS ;
			num_decimals = 1 ;
		break ;


		case TIMER1 :
		case TIMER2 :
		{
			div_t qr ;
			qr = div( value, 60 ) ;
			voiceMinutes( qr.quot ) ;
			value = qr.rem ;
			unit = V_SECONDS ;
		}
		break ;

		case V_GVAR1 :
		case V_GVAR2 :
		case V_GVAR3 :
		case V_GVAR4 :
		case V_GVAR5 :
		case V_GVAR6 :
		case V_GVAR7 :
			value = g_model.gvars[index-V_GVAR1].gvar ;


		break ;


	}

	if ( spoken == 0 )
	{
		voice_numeric( value, num_decimals, unit ) ;
	}
}


// This routine converts an 8 bit value for custom switch use
int16_t convertTelemConstant( uint8_t channelin, int8_t value)
{
  int16_t result;
	int8_t channel ;

	channel = pgm_read_byte( &TelemIndex[channelin] ) ;
	result = value + 125 ;
  switch (channel)
	{


    case TIMER1 :
    case TIMER2 :
      result *= 10 ;
    break;
  }
  return result;
}


int32_t get_telemetry_value( uint8_t channelIn )
{
	int8_t channel = TelemIndex[channelIn];
  if ( channel == TMOK )
	{
		return 0 ;
	}
	if ( channel < -11 )	// A Scaler
	{
		return calc_scaler(channel-V_SC1, 0, 0 ) ;
	}
	if ( channel < -3 )	// A GVAR
	{
		return g_model.gvars[channel-V_GVAR1].gvar ;
	}
  switch (channel)
	{
    case TIMER1 :
    case TIMER2 :
    return s_timer[channel+2].s_timerVal ;

    case BATTERY :
    return g_vbat100mV ;


		default :
        return AFHDS2A_tel_data[channel];

  }
}

void displayTimer( uint8_t x, uint8_t y, uint8_t timer, uint8_t att )
{
	struct t_timer *tptr = &s_timer[timer] ;
	FORCE_INDIRECT(tptr) ;
  putsTime( x, y, tptr->s_timerVal, att, att ) ;
}

// Styles
#define TELEM_LABEL		0x01
#define TELEM_UNIT    		0x02
#define TELEM_UNIT_LEFT		0x04
#define TELEM_VALUE_RIGHT	0x08
#define TELEM_CONSTANT		0x80

uint8_t putsTelemetryChannel(uint8_t x, uint8_t y, int8_t channel, int32_t val, uint8_t att, uint8_t style) {
  uint8_t unit = ' ';
  uint8_t xbase = x;
  uint8_t fieldW = FW;
  uint8_t displayed = 0;
  int8_t chanIndex;

  chanIndex = TelemIndex[channel];
  if (style & TELEM_LABEL) {
    uint8_t displayed = 0;
    int8_t index = chanIndex;
    if ((index >= V_SC1) && (index < V_SC1 + NUM_SCALERS)) {
      index -= V_SC1;
      uint8_t *p = &g_model.Scalers[index].name[0];
      if (*p) {
        lcd_putsnAtt(x, y, (const char *)p, 4, BSS);
        displayed = 1;
      }
    }
    if (displayed == 0) {

      putsAttIdxTelemItems(x, y, channel + 1, 0);
    }
    x += 4 * FW;
    if (att & DBLSIZE) {
      x += 4;
      y -= FH;
      fieldW += FW;
    }
  }

  if (style & TELEM_VALUE_RIGHT) {
    att &= ~LEFT;
  }
  channel = chanIndex;
  switch (channel) {
  case V_SC1:
  case V_SC2:
  case V_SC3:
  case V_SC4: {
    int16_t cvalue;
    uint8_t precision;
    cvalue = calc_scaler(channel - V_SC1, &unit, &precision);
    if (precision == 1) {
      att |= PREC1;
    } else if (precision == 2) {
      att |= PREC2;
    }
    // Sort units here
    unit = pgm_read_byte(&UnitsText[unit]);
    if ((style & TELEM_CONSTANT) == 0) {
      val = cvalue;
    }
  } break;

  case TIMER1:
  case TIMER2:
    if ((att & DBLSIZE) == 0) {
      x -= 4;
    }
    if (style & TELEM_LABEL) {
      x += FW + 7;
    }
    att &= DBLSIZE | INVERS | BLINK;
    putsTime(x - FW, y, val, att, att);
    displayed = 1;
    unit = channel + 2 + '1';
    xbase -= FW;
    break;
  }
  if ((channel >= 0) & (channel <= FST_IDX_S8a)) {

    unit = putsTelemValue((style & TELEM_VALUE_RIGHT) ? xbase + 54 : x - fieldW, y, val, channel, att | NO_UNIT /*|blink*/);
    displayed = 1;
  }


  if (!displayed) {
    lcd_outdezAtt((style & TELEM_VALUE_RIGHT) ? xbase + 54 : x, y, val, att);
  }
  if (style & (TELEM_UNIT | TELEM_UNIT_LEFT)) {
    if (style & TELEM_UNIT_LEFT) {
      x = xbase + FW + 4;
      att &= ~DBLSIZE;
    } else {
      x = Lcd_lastPos;
    }
    lcd_putcAtt(x, y, unit, att);
  }
  return unit;
}

const prog_char APM Str_GV[] = STR_GV ;
const prog_char APM Str_SC[] = "SC" ;

void dispGvar( uint8_t x, uint8_t y, uint8_t gvar, uint8_t attr )
{
	lcd_putsAtt( x, y, Str_GV, attr ) ;
	lcd_putcAtt( x+2*FW, y, gvar+'0', attr ) ;
}


int8_t gvarMenuItem(uint8_t x, uint8_t y, int8_t value, int8_t min, int8_t max, uint8_t attr )
{
  uint8_t invers = attr&(INVERS|BLINK);

  if (value >= 126 || value <= -126)
	{
		dispGvar( x-3*FW, y, (uint8_t)value - 125, attr ) ;
    if (invers) value = checkIncDec16((uint8_t)value, 126, 130, EE_MODEL);
  }
  else
	{
    lcd_outdezAtt(x, y, value, attr ) ;
    if (invers) CHECK_INCDEC_H_MODELVAR( value, min, max);
  }
	if (invers)
	{
		if ( Tevent == EVT_TOGGLE_GVAR )
		{
    	value = ((value >= 126 || value <= -126) ? g_model.gvars[(uint8_t)value-126].gvar : 126);
	    eeDirty(EE_MODEL) ;
		}
	}
  return value ;
}

int8_t gvarMenuItem125(uint8_t x, uint8_t y, int8_t value, uint8_t attr )
{
	return gvarMenuItem( x, y, value, -125, 125, attr ) ;
}


uint8_t get_dr_state(uint8_t x)
{
	ExpoData *ped ;

	ped = &g_model.expoData[x] ;

 	return (!getSwitch00( ped->drSw1) ? DR_HIGH :
    !getSwitch00( ped->drSw2) ? DR_MID : DR_LOW) ;
}

static void DO_SQUARE(uint8_t x, uint8_t y, uint8_t w)
{
		lcd_rect( x-w/2, y-w/2, w, w ) ;
}
#define DO_CROSS(xx,yy,ww)          \
    lcd_vline(xx,yy-ww/2,ww);  \
    lcd_hline(xx-ww/2,yy,ww);  \

#define V_BAR(xx,yy,ll)       \
    lcd_vline(xx-1,yy-ll,ll); \
    lcd_vline(xx  ,yy-ll,ll); \
    lcd_vline(xx+1,yy-ll,ll); \

#define NO_HI_LEN 25

#define WCHART 32
#define X0     (128-WCHART-2)
#define Y0     32
#define WCHARTl 32l
#define X0l     (128l-WCHARTl-2)
#define Y0l     32l
#define RESX    (1<<10) // 1024
#define RESXu   1024u
#define RESXul  1024ul
#define RESXl   1024l
#define RESKul  100ul
#define RESX_PLUS_TRIM (RESX+128)

#define TRIM_EXTENDED_MAX	500

enum MainViews {

    e_inputs1,
    e_telemetry,
    MAX_VIEWS
};

int16_t calibratedStick[7];
int16_t Ex_chans[NUM_CHNOUT];          // Outputs + intermidiates
uint8_t s_pgOfs;
uint8_t s_editMode;
uint8_t s_editing;
uint8_t s_noHi;
int8_t scrollLR;
uint8_t scroll_disabled;
int8_t scrollUD;
uint8_t InverseBlink ;
uint8_t EditType ;
uint8_t EditColumns ;

int16_t g_chans512[NUM_CHNOUT];

extern MixData *mixaddress( uint8_t idx ) ;

const
#include "sticks.lbm"


void menuProcAlpha(uint8_t event) ;
void menuProcIndex(uint8_t event) ;

void menuProcModelIndex(uint8_t event) ;

#define MENU_TAB_NONE		0
#define MENU_TAB_MODEL	1
#define MENU_TAB_DIAG		2

#define SIZE_MTAB_MODEL	DIM(menuTabModel)

int8_t qRotary()
{
	int8_t diff = 0 ;

	if ( Rotary.Rotary_diff > 0)
	{
		diff = 1 ;
	}
	else if ( Rotary.Rotary_diff < 0)
	{
		diff = -1 ;
	}
	Rotary.Rotary_diff = 0 ;
	return diff ;
}


void editName( uint8_t x, uint8_t y, uint8_t *name, uint8_t len, uint8_t type )
{
	lcd_puts_Pleft( y, Str_Name) ;
	lcd_putsnAtt( 11*FW, y, (const char *)name, len, BSS ) ;
	if( type )
	{
		lcd_char_inverse( (11+x)*FW, y, 1*FW, s_editMode ) ;
		lcd_rect( 11*FW-2, y-1, len*FW+4, 9 ) ;
	  if(s_editMode)
		{
     	char v = name[x] ;
			if ( v )
			{
	  	  v = char2idx(v) ;
			}
			v = checkIncDec( v, 0, NUMCHARS-1, type ) ;
  	  v = idx2char(v);
			if ( name[x] != v )
			{
				name[x] = v ;
    		eeDirty( EditType ) ;				// Do here or the last change is not stored in name[]
			}
		}
	}
	asm("") ;
}


uint8_t TITLEP( const prog_char *pstr) { return lcd_putsAtt(0,0,pstr,INVERS) ; }

#define PARAM_OFS   17*FW

void lcd_xlabel_decimal( uint8_t x, uint8_t y, uint16_t value, uint8_t attr, const prog_char * s )
{
  lcd_outdezAtt( x, y, value, attr ) ;
	lcd_puts_Pleft( y, s ) ;
}

#define FWx3		"\022"
#define FWx4		"\030"
#define FWx5		"\036"
#define FWx6		"\044"
#define FWx7		"\052"
#define FWx8		"\060"
#define FWx9		"\066"
#define FWx10		"\074"
#define FWx11		"\102"
#define FWx12		"\110"
#define FWx13		"\116"
#define FWx14		"\124"
#define FWx15		"\132"
#define FWx16		"\140"
#define FWx17		"\146"
#define FWx18		"\152"


uint8_t checkIndexed( uint8_t y, const prog_char * s, uint8_t value, uint8_t edit )
{
	uint8_t x ;
	uint8_t max ;

	if ( s )
	{
		x = pgm_read_byte(s++) ;
		max = pgm_read_byte(s++) ;
		lcd_putsAttIdx( x, y, s, value, edit ? InverseBlink: 0 ) ;
	}
	else
	{
		x = PARAM_OFS ;
		max = 1 ;
  	if (value)
		{
    	lcd_putc(x+1, y, '\202');
		}
		lcd_hbar( x, y, 7, 7, edit ? 100 : 0 ) ;
	}
	if(edit)
	{
		if ( ( EditColumns == 0 ) || ( s_editMode ) )
		{
			value = checkIncDec( value, 0, max, EditType ) ;
		}
	}

	return value ;
}

uint8_t hyphinvMenuItem( uint8_t value, uint8_t y, uint8_t condition )
{
	return checkIndexed( y, PSTR(FWx18 "\001" STR_HYPH_INV), value, condition ) ;
}


void putsOffDecimal( uint8_t x, uint8_t y, uint16_t value, uint8_t attr )
{
  if(value)
      lcd_outdezAtt( x+3*FW-2,y,value,attr) ;
  else
      lcd_putsAtt(  x,y,Str_OFF,attr) ;
}


uint8_t g_posHorz ;
uint8_t M_longMenuTimer ;


#define INC(val,max) if(val<max) {val++;} else {val=0;}
#define DEC(val,max) if(val>0  ) {val--;} else {val=max;}


void MState2::check_columns( uint8_t event, uint8_t maxrow)
{
	uint8_t l_posHorz ;
	l_posHorz = g_posHorz ;
	int16_t c4, c5 ;
	struct t_p1 *ptrp1 ;


    //check pot 2 - if changed -> scroll menu
    //check pot 3 if changed -> cursor down/up
    //we do this in these brackets to prevent it from happening in the main screen
		c4 = calibratedStick[4] ;		// Read only once
		c5 = calibratedStick[5] ;		// Read only once

		ptrp1 = &P1values ;
		FORCE_INDIRECT(ptrp1) ;
    scrollLR = ( ptrp1->p2valprev-c4)/SCROLL_TH;
    scrollUD = ( ptrp1->p3valprev-c5)/SCROLL_TH;

    if(scrollLR) ptrp1->p2valprev = c4;
    if(scrollUD) ptrp1->p3valprev = c5;

    if(scroll_disabled || g_eeGeneral.disablePotScroll)
    {
        scrollLR = 0;
        scrollUD = 0;
        scroll_disabled = 0;
    }

    if(scrollLR || scrollUD || ptrp1->p1valdiff) backlightKey() ; // on keypress turn the light on
						// *250 then <<1 is the same as *500, but uses less code space

    uint8_t maxcol = Columns ; //MAXCOL(m_posVert, horTab, horTabMax);

	 if ( maxrow != 0xFF )
	 {
		if ( RotaryState == ROTARY_MENU_UD )
		{
			static uint8_t lateUp = 0 ;
			if ( lateUp )
			{
				lateUp = 0 ;
				l_posHorz = maxcol ; //MAXCOL(m_posVert, horTab, horTabMax) ;
			}
		 	int8_t diff = qRotary() ;
			if ( diff > 0 )
			{
        INC(l_posHorz,maxcol) ;
				if ( l_posHorz == 0 )
				{
	        INC(m_posVert,maxrow);
				}
			}
			else if ( diff < 0 )
			{
				if ( l_posHorz == 0 )
				{
      	  DEC(m_posVert,maxrow);
					lateUp = 1 ;
					l_posHorz = 0 ;
				}
				else
				{
      	  DEC(l_posHorz,maxcol) ;
				}
			}
      if(event==EVT_KEY_BREAK(BTN_RE))
			{
				RotaryState = ROTARY_VALUE ;
			}
		}
		else if ( RotaryState == ROTARY_VALUE )
		{
      if ( (event==EVT_KEY_BREAK(BTN_RE)) || ( s_editMode == 0 ) )
			{
				RotaryState = ROTARY_MENU_UD ;
			}
		}

		{
			uint8_t timer = M_longMenuTimer ;
			if ( menuPressed() )
			{
				if ( timer < 255 )
				{
					timer += 1 ;
				}
			}
			else
			{
				timer = 0 ;
			}
			if ( timer > 60 )
			{
				s_editMode = 1 ;
				RotaryState = ROTARY_VALUE ;
			}
			M_longMenuTimer = timer ;
		}
	 }


		EditColumns = maxcol ;

    if(!s_editMode)
    {
        if(scrollUD)
        {
            int8_t cc = m_posVert - scrollUD;
            if(cc<1) cc = 0;
            if(cc>=maxrow) cc = maxrow;
            m_posVert = cc;

            l_posHorz = min(l_posHorz, maxcol ) ; //MAXCOL(m_posVert, horTab, horTabMax));
            BLINK_SYNC;

            scrollUD = 0;
        }

        if(m_posVert>0 && scrollLR)
        {
            int8_t cc = l_posHorz - scrollLR;
            if(cc<1) cc = 0;
            if(cc>=maxcol) cc = maxcol ;
            l_posHorz = cc;

            BLINK_SYNC;
        }
    }
		switch(event)
    {
    case EVT_ENTRY:
				m_posVert = 0 ;
        l_posHorz = 0 ;
        s_editMode = false;
        break;
    case EVT_KEY_BREAK(BTN_RE):
    case EVT_KEY_FIRST(KEY_MENU):
				{
	 				if ( maxrow != 0xFF )
					{
						s_editMode = !s_editMode;
						if ( s_editMode )
						{
							RotaryState = ROTARY_VALUE ;
						}
					}
				}
        break;
    case EVT_KEY_LONG(KEY_EXIT):
        s_editMode = false;
        popMenu(true) ;
        break;
        //fallthrough
    case EVT_KEY_LONG(BTN_RE):
        killEvents(event);
    case EVT_KEY_BREAK(KEY_EXIT):
        if(s_editMode) {
            s_editMode = false;
            break;
        }
            popMenu();  //beeps itself
        break;

    case EVT_KEY_REPT(KEY_RIGHT):  //inc
        if(l_posHorz==maxcol) break;
    case EVT_KEY_FIRST(KEY_RIGHT)://inc
        if(!maxcol || s_editMode)break;
        INC(l_posHorz,maxcol);
				if ( maxcol )
				{
					Tevent = 0 ;
				}
        BLINK_SYNC;
        break;

    case EVT_KEY_REPT(KEY_LEFT):  //dec
        if(l_posHorz==0) break;
    case EVT_KEY_FIRST(KEY_LEFT)://dec
        if(!maxcol || s_editMode)break;
        DEC(l_posHorz,maxcol);
				if ( maxcol )
				{
					Tevent = 0 ;
				}
        BLINK_SYNC;
        break;

    case EVT_KEY_REPT(KEY_DOWN):  //inc
        if(m_posVert==maxrow) break;
    case EVT_KEY_FIRST(KEY_DOWN): //inc
        if(s_editMode)break;
        INC(m_posVert,maxrow);
        l_posHorz = min(l_posHorz, maxcol ) ; //MAXCOL(m_posVert, horTab, horTabMax));
        BLINK_SYNC;
        break;

    case EVT_KEY_REPT(KEY_UP):  //dec
        if(m_posVert==0) break;
    case EVT_KEY_FIRST(KEY_UP): //dec
        if(s_editMode)break;
        DEC(m_posVert,maxrow);
        l_posHorz = min(l_posHorz, maxcol ) ; // MAXCOL(m_posVert, horTab, horTabMax));
        BLINK_SYNC;
        break;
    }
		s_editing = s_editMode || P1values.p1valdiff ;
	g_posHorz = l_posHorz ;
	InverseBlink = (s_editMode) ? BLINK : INVERS ;
	Columns = 0 ;
}

#define BOX_WIDTH     23
#define BAR_HEIGHT    (BOX_WIDTH-1l)
#define MARKER_WIDTH  5
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define BOX_LIMIT     (BOX_WIDTH-MARKER_WIDTH)
#define LBOX_CENTERX  (  SCREEN_WIDTH/4 + 10)
#define BOX_CENTERY  (SCREEN_HEIGHT-9-BOX_WIDTH/2-1)
#define RBOX_CENTERX  (3*SCREEN_WIDTH/4 - 10)


void telltale( uint8_t centrex, int8_t xval, int8_t yval )
{
  DO_SQUARE( centrex, BOX_CENTERY, BOX_WIDTH ) ;
  DO_CROSS( centrex, BOX_CENTERY,3 ) ;
	lcd_rect_xor( centrex +( xval/((2*RESX/16)/BOX_LIMIT))-MARKER_WIDTH/2, BOX_CENTERY-( yval/((2*RESX/16)/BOX_LIMIT))-MARKER_WIDTH/2, MARKER_WIDTH, MARKER_WIDTH ) ;
}

void doMainScreenGrphics()
{
	{
		int8_t *cs = phyStick ;
		FORCE_INDIRECT(cs) ;

		telltale( LBOX_CENTERX, cs[0], cs[1] ) ;
		telltale( RBOX_CENTERX, cs[3], cs[2] ) ;
	}

    // Optimization by Mike Blandford
	int16_t *cs = calibratedStick ;
	FORCE_INDIRECT(cs) ;
  {
    uint8_t x, y, len ;			// declare temporary variables
    for( x = -5, y = 4 ; y < 7 ; x += 5, y += 1 )
    {
      len = ((cs[y]+RESX)/((RESX*2)/BAR_HEIGHT))+1 ;  // calculate once per loop
      V_BAR(SCREEN_WIDTH/2+x,SCREEN_HEIGHT-8, len ) ;
    }
  }
}

extern int16_t calcExpo( uint8_t channel, int16_t value ) ;

static uint8_t s_curveChan ;
static uint8_t s_expoChan ;

#define XD (X0-2)

#define GRAPH_FUNCTION_CURVE		0
#define GRAPH_FUNCTION_EXPO			1

void drawFunction( uint8_t xpos, uint8_t function )
{
  int8_t yv ;
	int8_t prev_yv = 127 ;
	for ( int8_t xv = -WCHART ; xv <= WCHART ; xv++ )
	{
		if ( function == GRAPH_FUNCTION_CURVE )
		{
    	yv = intpol(xv * RESX / WCHART, s_curveChan) * WCHART / RESX ;
		}
		else
		{
    	yv = calcExpo( s_expoChan, xv * RESX / WCHART) * WCHART / RESX ;
		}
    if (prev_yv == 127)
		{
			prev_yv = yv ;
		}
		uint8_t len = abs(yv-prev_yv) ;
    if (len <= 1)
		{
    	lcd_plot(xpos + xv, Y0 - yv) ;
		}
		else
		{
      uint8_t tmp = (prev_yv < yv ? 0 : len-1 ) ;
      lcd_vline(xpos+xv, Y0 - yv - tmp, len ) ;
		}
		if ( yv )
		{
     	lcd_plot(xpos + xv, Y0 ) ;
		}
		prev_yv = yv ;
	}

}


NOINLINE static int8_t *curveAddress( uint8_t idx )
{
  uint8_t cv9 = idx >= MAX_CURVE5 ;
	return cv9 ? g_model.curves9[idx-MAX_CURVE5] : g_model.curves5[idx] ;
}

void drawCurve( uint8_t offset )
{
  uint8_t cv9 = s_curveChan >= MAX_CURVE5 ;
	int8_t *crv = curveAddress( s_curveChan ) ;

	lcd_vline(XD, Y0 - WCHART, WCHART * 2);

	for(uint8_t i=0; i<(cv9 ? 9 : 5); i++)
  {
    uint8_t xx = XD-1-WCHART+i*WCHART/(cv9 ? 4 : 2);
    uint8_t yy = Y0-crv[i]*WCHART/100;

    if(offset==i)
    {
			lcd_rect( xx-1, yy-2, 5, 5 ) ;
    }
    else
    {
			lcd_rect( xx, yy-1, 3, 3 ) ;
    }
  }

	drawFunction( XD, GRAPH_FUNCTION_CURVE ) ;
}


void menuProcCurveOne(uint8_t event)
{
  uint8_t cv9 = s_curveChan >= MAX_CURVE5;
	static int8_t dfltCrv;
	TITLEP(Str_Curve) ;
	static MState2 mstate2 ;
	mstate2.check_columns(event, (cv9 ? 9 : 5) ) ;

	if ( event == EVT_ENTRY )
	{
		dfltCrv = 0 ;
	}
	lcd_outdezAtt(7*FW, 0, s_curveChan+1, INVERS);

	int8_t *crv = curveAddress( s_curveChan ) ;

	uint8_t  sub    = mstate2.m_posVert ;
	uint8_t blink = InverseBlink ;
	uint8_t  preset = cv9 ? 9 : 5 ;

	for (uint8_t i = 0; i < 5; i++)
	{
    uint8_t y = i * FH + 16;
    uint8_t attr = sub == (i) ? blink : 0;
    lcd_outdezAtt(4 * FW, y, crv[i], attr);
		if ( cv9 )
		{
			if ( i < 4 )
			{
    		attr = sub == i + 5 ? blink : 0;
		    lcd_outdezAtt(8 * FW, y, crv[i + 5], attr);
			}
		}
	}
	lcd_putsAtt( 2*FW, 7*FH,PSTR(STR_PRESET), (sub == preset) ? blink : 0);


if( sub==preset)
{
	if ( s_editMode )
	{
		int8_t t ;
		Tevent = event ;
		t = dfltCrv ;
    dfltCrv = checkIncDec( t, -4, 4, 0);
    if (dfltCrv != t)
		{
			uint8_t offset = cv9 ? 4 : 2 ;

			for (int8_t i = -offset; i <= offset; i++) crv[i+offset] = i*dfltCrv* 25 / offset ;
      STORE_MODELVARS;
    }
	}
}
else  /*if(sub>0)*/
{
 CHECK_INCDEC_H_MODELVAR( crv[sub], -100,100);
}

// Draw the curve
	drawCurve( sub ) ;
}


void menuProcCurve(uint8_t event)
{
	TITLEP(Str_Curves) ;
	static MState2 mstate2 ;
		mstate2.check_columns(event,1+MAX_CURVE5+MAX_CURVE9-1-1) ;
    int8_t  sub    = mstate2.m_posVert ;

    uint8_t t_pgOfs = evalOffset(sub);

    switch (event)
		{
	    case EVT_KEY_FIRST(KEY_RIGHT):
  	  case EVT_KEY_FIRST(KEY_MENU):
    	case EVT_KEY_BREAK(BTN_RE):
				if (sub >= 0)
				{
          s_curveChan = sub;
      		killEvents(event);
					Tevent = 0 ;
          pushMenu(menuProcCurveOne);
        }
      break ;
    }

    uint8_t y    = 1*FH;
    for (uint8_t i = 0; i < 7; i++)
		{
        uint8_t k = i + t_pgOfs;
        uint8_t attr = sub == k ? INVERS : 0;
        if(y>7*FH) break;
        lcd_putsAtt(   FW*0, y,PSTR(STR_CV),attr);
        lcd_outdezAtt( (k<9) ? FW*3-1 : FW*4-2, y,k+1 ,attr);

        y += FH; // yd++;
    }

		if ( sub >= 0 )
		{
  		s_curveChan = sub ;
			drawCurve( 100 ) ;
		}
	asm("") ;
}

void setStickCenter() // copy state of 3 primary to subtrim
{
	uint8_t thisPhase ;
	struct t_stickCentre *data = &Xmem.stickCentreData ;


	thisPhase = getFlightPhase() ;
	CurrentPhase = thisPhase ;

    perOut(data->zero_chans512_before,NO_TRAINER | NO_INPUT | FADE_FIRST | FADE_LAST); // do output loop - zero input channels
    perOut(data->zero_chans512_after,NO_TRAINER | FADE_FIRST | FADE_LAST); // do output loop - zero input channels
//    perOut(zero_chans512_before,NO_TRAINER | NO_INPUT | FADE_FIRST | FADE_LAST); // do output loop - zero input channels
//    perOut(zero_chans512_after,NO_TRAINER | FADE_FIRST | FADE_LAST); // do output loop - zero input channels

    for(uint8_t i=0; i<NUM_CHNOUT; i++)
    {
        int16_t v = g_model.limitData[i].offset;
				int16_t b ;
				b = data->zero_chans512_before[i] - data->zero_chans512_after[i] ;
        v += g_model.limitData[i].reverse ? b : -b ;

				if ( v > 1000 )
				{
					v = 1000 ;
				}
				if ( v < -1000 )
				{
					v = -1000 ;
				}
        g_model.limitData[i].offset = v ;
    }

  for(uint8_t i=0; i<4; i++)
	{
		if(!IS_THROTTLE(i))
		{
			int16_t original_trim = getTrimValue(thisPhase, i);
      for (uint8_t phase=0; phase<MAX_MODES; phase +=  1)
			{
        int16_t trim = getRawTrimValue(phase, i);
        if (trim <= TRIM_EXTENDED_MAX)
				{
          setTrimValue(phase, i, trim - original_trim);
				}
			}
		}
	}
  STORE_MODELVARS_TRIM;
  audioDefevent(AU_WARNING2);
}

static void menuProcLimits(uint8_t sub)
{
	TITLEP(Str_limits);

	IlinesCount = NUM_CHNOUT+2-1 ;
	Columns = 3 ;
uint8_t y = 0;
uint8_t k = 0;
uint8_t subSub = g_posHorz;
    uint8_t t_pgOfs ;

t_pgOfs = evalOffset(sub);

	if ( sub < NUM_CHNOUT )
	{
    lcd_outdez( 13*FW, 0, g_chans512[sub]/2 + 1500 ) ;
	}

	switch(Tevent)
	{
    case EVT_KEY_LONG(KEY_MENU):
        if(sub>=0 && sub<NUM_CHNOUT)
				{
            int16_t v = g_chans512[sub - t_pgOfs];
            LimitData *ld = &g_model.limitData[sub] ;
            if ( subSub == 0 )
						{
                ld->offset = (ld->reverse) ? -v : v;
                STORE_MODELVARS;
                eeWaitComplete() ;
            }
        }
				else if(sub==NUM_CHNOUT)
				{
	        s_noHi = NO_HI_LEN;
  	      killEvents(Tevent);
    	    setStickCenter(); //if highlighted and menu pressed - copy trims
				}
    break;
	}
	for(uint8_t i=0; i<7; i++)
	{
    y=(i+1)*FH;
    k=i+t_pgOfs;
    if(k==NUM_CHNOUT) break;
    LimitData *ld = &g_model.limitData[k] ;
	FORCE_INDIRECT(ld) ;
    int16_t v = g_chans512[k] - ( (ld->reverse) ? -ld->offset : ld->offset) ;

    char swVal = '-';  // '-', '<', '>'
    if(v >  50) swVal = (ld->reverse ? 127 : 126);	// Switch to raw inputs?  - remove trim!
    if(v < -50) swVal = (ld->reverse ? 126 : 127);
    putsChn(0,y,k+1,0);
    lcd_putc(12*FW+FW/2, y, swVal ); //'<' : '>'

    int8_t limit = (g_model.extendedLimits ? 125 : 100);
		for(uint8_t j=0; j<4;j++)
		{
        uint8_t attr = ((sub==k && subSub==j) ? InverseBlink : 0);
			uint8_t active = attr ;	// (attr && s_editing) ;
			if ( s_editing == 0 )
			{
				active = 0 ;
			}
				if ( active )
				{
					StickScrollAllowed = 0 ;		// Block while editing
				}
				int16_t value ;
				int16_t t = 0 ;
				if ( g_model.sub_trim_limit )
				{
					if ( ( t = ld->offset ) )
					{
						if ( t > g_model.sub_trim_limit )
						{
							t = g_model.sub_trim_limit ;
						}
						else if ( t < -g_model.sub_trim_limit )
						{
							t = -g_model.sub_trim_limit ;
						}
					}
				}
				value = t / 10 ;
        switch(j)
        {
        case 0:
            lcd_outdezAtt(  7*FW+3, y,  ld->offset, attr|PREC1);
            if(active) {
                ld->offset = checkIncDec16(ld->offset, -1000, 1000, EE_MODEL);
            }
            break;
        case 1:
				{
					int8_t temp = ld->min - 100 ;

					value += temp ;
					if ( value < -125 )
					{
						value = -125 ;
					}
          lcd_outdezAtt(  12*FW, y, value,   attr);
            if(active)
						{
              CHECK_INCDEC_H_MODELVAR( temp, -limit,25);
              ld->min = temp + 100;
            }
				}
        break;
        case 2:
				{
					value += (int8_t)(ld->max+100) ;
					if ( value > 125 )
					{
						value = 125 ;
					}
          lcd_outdezAtt( 17*FW, y, value,    attr);
					if ( t )
					{
						lcd_rect( 9*FW-4, y-1, 56, 9 ) ;
					}
            if(active)
						{
							int8_t temp = ld->max + 100 ;
                CHECK_INCDEC_H_MODELVAR( temp, -25,limit);
                ld->max = temp - 100 ;
            }
				}
        break;
				case 3:
        default:
					ld->reverse = hyphinvMenuItem( ld->reverse, y, attr ) ;
            break;
        }
    }
	}
	if(k==NUM_CHNOUT)
	{
    //last line available - add the "copy trim menu" line
    uint8_t attr = (sub==NUM_CHNOUT) ? INVERS : 0;
    lcd_putsAtt(  3*FW,y,PSTR(STR_COPY_TRIM),s_noHi ? 0 : attr);
	}
	asm("") ;
}


static uint8_t onoffItem( uint8_t value, uint8_t y, uint8_t condition )
{
	return checkIndexed( y, 0, value, condition ) ;
}

static uint8_t offonItem( uint8_t value, uint8_t y, uint8_t condition )
{
	return 1-onoffItem( 1-value, y, condition ) ;
}

static uint8_t onoffMenuItem( uint8_t value, uint8_t y, const prog_char *s, uint8_t condition )
{
    lcd_puts_Pleft(y, s);
		return onoffItem(value, y, condition ) ;
}

NOINLINE static uint8_t offonMenuItem( uint8_t value, uint8_t y, const prog_char *s, uint8_t condition )
{
	return 1-onoffMenuItem( 1-value, y, s, condition ) ;
}

void displayNext()
{
	lcd_puts_P( 17*FW+2, 7*FH, PSTR("[->]") ) ;
}

void menuProcTelemetry(uint8_t event) {
  static MState2 mstate2;
  mstate2.check_columns(event, 13);
  TITLEP(Str_Telemetry);
  uint8_t sub = mstate2.m_posVert;
  uint8_t subN = 0;

  uint8_t y = FH;
  uint8_t attr = 0;

  /*-------------Page 1-----------------------------------------------*/
  if (sub < 6) {
    attr = PREC2;
    lcd_puts_Pleft(y, PSTR("IntV offset"
                           "\037""ExtV offset"
                           "\037""Num blades"
                           "\037""Fuel source"
                           "\037""Fuel min"
                           "\037""Fuel max"));

    if (sub == subN) {
      g_model.IntVoffs = checkIncDec(g_model.IntVoffs, -126, 127, EE_MODEL);
      attr |= InverseBlink;
    }
    lcd_outdezAtt(17 * FW, y, g_model.IntVoffs, attr);
    attr = PREC2;
    y += FH;
    subN += 1;

    if (sub == subN) {
      g_model.ExtVoffs = checkIncDec(g_model.ExtVoffs, -126, 127, EE_MODEL);
      attr |= InverseBlink;
    }
    lcd_outdezAtt(17 * FW, y, g_model.ExtVoffs, attr);
    attr = 0;
    y += FH;
    subN += 1;
    if (sub == subN) {
      g_model.numBlades = checkIncDec(g_model.numBlades, 1, 127, EE_MODEL);
      attr |= InverseBlink;
    }
     lcd_outdezAtt(17 * FW, y, g_model.numBlades, attr);
    attr = 0;
    y += FH;
    subN += 1;
    if (sub == subN) {
      CHECK_INCDEC_H_MODELVAR_0(g_model.FuelSource, NUM_TELEM_ITEMS);
      attr |= InverseBlink;
    }
    putsAttIdxTelemItems(13 * FW + 1, y, g_model.FuelSource, attr);
    attr = 0;
    y += FH;
    subN += 1;
    if (sub == subN) {
      g_model.FuelMin = checkIncDec16(g_model.FuelMin, -32767, 32767, EE_MODEL);
      attr |= InverseBlink;
    }

     lcd_outdezAtt(17 * FW, y, g_model.FuelMin, attr);

    attr = 0;
    y += FH;
    subN += 1;
    if (sub == subN) {
      g_model.FuelMax = checkIncDec16(g_model.FuelMax, -32767, 32767, EE_MODEL);
      attr |= InverseBlink;
    }
     lcd_outdezAtt(17 * FW, y, g_model.FuelMax, attr);
    displayNext();
    } else
    {
      /*-------------Page 2 Custom disp-------------------------------------*/
      if ((sub > 5) & (sub < 14)) {

        uint8_t *pindex = g_model.CustomDisplayIndex;
        subN = 6;
        if(sub>=12)
        {
            pindex += sub-11;
            subN += sub-11;
        }
        lcd_puts_Pleft(FH, PSTR(STR_CUSTOM_DISP));
        for (uint8_t j = 0; j < 6; j++) {
          attr = 0;
          if (sub == subN) {
            attr = InverseBlink;
            CHECK_INCDEC_H_MODELVAR_0(*pindex, NUM_TELEM_ITEMS);
          }
          putsAttIdxTelemItems(0, j * FH + 2 * FH, *pindex, attr);
          pindex += 1;
          subN++;
        }
        displayNext();
      }
    }

}


void put_curve( uint8_t x, uint8_t y, int8_t idx, uint8_t attr )
{
	if ( idx < 0 )
	{
    lcd_putcAtt( x-FW, y, '!', attr ) ;
		idx = -idx + 6 ;
	}
	lcd_putsAttIdx( x, y,Curve_Str,idx,attr);
}


uint8_t scalerDecimal( uint8_t y, uint8_t val, uint8_t attr )
{
  lcd_outdezAtt( 17*FW, y, val+1, attr ) ;
  if (attr) val = checkIncDec_u0( val, 255 ) ;
	return val ;
}

void menuScaleOne(uint8_t event)
{

	static MState2 mstate2 ;
	mstate2.check_columns(event, 8 ) ;
	lcd_puts_Pleft( 0, Str_SC ) ;
	uint8_t index = s_currIdx ;
  lcd_putc( 2*FW, 0, index+'1' ) ;

	int8_t sub = mstate2.m_posVert;
	uint8_t t_pgOfs = evalOffset(sub);

	putsTelemetryChannel( 8*FW, 0, index+TEL_ITEM_SC1, 0, 0, TELEM_UNIT ) ;

	lcd_puts_Pskip( FH, PSTR("Source\037\037" STR_OFFSET "\037Multiplier\037Divisor\037Unit\037Sign\037Decimals\037Offset At"), s_pgOfs ) ;

	for (uint8_t k = 0 ; k < 7 ; k += 1 )
	{
    uint8_t y = (k+1) * FH ;
    uint8_t i = k + t_pgOfs;
		uint8_t attr = (sub==i ? InverseBlink : 0);
		ScaleData *pscaler ;
		pscaler = &g_model.Scalers[index] ;

		switch(i)
		{
      case 0 :	// Source
				putsChnRaw( 11*FW, y, pscaler->source, attr ) ;
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( pscaler->source, NUM_XCHNRAW+NUM_TELEM_ITEMS ) ;
			break ;
			case 1 :	// name
				if ( attr )
				{
					Columns = 3 ;
				}
				editName( g_posHorz, y, pscaler->name, 4, attr ) ;
			break ;
      case 2 :	// offset
				lcd_outdezAtt( 13*FW, y, pscaler->offset, attr) ;
				if ( attr )
				{
					pscaler->offset = checkIncDec16( pscaler->offset, -32000, 32000, EE_MODEL ) ;
				}
			break ;
      case 3 :	// mult
				pscaler->mult = scalerDecimal( y, pscaler->mult, attr ) ;
			break ;
      case 4 :	// div
				pscaler->div = scalerDecimal( y, pscaler->div, attr ) ;
			break ;
      case 5 :	// unit
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( pscaler->unit, 7 ) ;
				lcd_putsAttIdx( 12*FW, y, UnitsString, pscaler->unit, attr ) ;
			break ;
      case 6 :	// sign
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( pscaler->neg, 1 ) ;
  			lcd_putcAtt( 12*FW, y, pscaler->neg ? '-' : '+', attr ) ;
			break ;
      case 7 :	// precision
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( pscaler->precision, 2 ) ;
				lcd_outdezAtt( 14*FW, y, pscaler->precision, attr) ;
			break ;
      case 8 :	// offsetLast
			{
				pscaler->offsetLast = checkIndexed( y, PSTR(FWx12"\001\005FirstLast "), pscaler->offsetLast, (sub==i) ) ;
			}
			break ;
		}
	}

}


void menuProcGlobals(uint8_t event)
{
	TITLEP(Str_Globals) ;
	static MState2 mstate2 ;

	mstate2.check_columns(event, MAX_GVARS + 1 + NUM_SCALERS-1-1 ) ;

	uint8_t subN = mstate2.m_posVert ;
	uint8_t subSub = g_posHorz;
	uint8_t y = FH ;

  switch (event)
	{
    case EVT_KEY_FIRST(KEY_MENU) :
    case EVT_KEY_BREAK(BTN_RE) :
			if ( subN >= 7 ) //&& sub <= MAX_MODES )
			{
        s_currIdx = subN - 7 ;
        killEvents(event);
        pushMenu(menuScaleOne) ;
    	}
		break;
  }
 if ( subN < 7 )
 {
	Columns = 1 ;
	for (uint8_t i=0; i<MAX_GVARS; i++ )
	{
    lcd_puts_Pleft(y, Str_GV ) ;
		lcd_putc( 2*FW, y, i+'1') ;
		GvarData *pgv = &g_model.gvars[i] ;
		FORCE_INDIRECT(pgv) ;
		for(uint8_t j=0; j<2;j++)
		{
      uint8_t attr = ((subN==i && subSub==j) ? InverseBlink : 0);
			uint8_t active = attr ;	// (attr && s_editing) ;
			if ( s_editMode == 0 )
			{
				active = 0 ;
			}
      if ( j == 0 )
			{
  			if(active) CHECK_INCDEC_H_MODELVAR_0( pgv->gvsource, 34-6 ) ;
				if ( pgv->gvsource < 13 )
				{
					lcd_putsAttIdx( 10*FW, y, Str_Gv_Source, pgv->gvsource, attr ) ;
				}
				else if ( pgv->gvsource < 29 )
				{
					lcd_putsAttIdx( 10*FW, y, Str_Chans_Gv, pgv->gvsource+3, attr ) ;
				}
			}
			else
			{
  			if(active) CHECK_INCDEC_H_MODELVAR( pgv->gvar, -125, 125 ) ;
				lcd_outdezAtt( 18*FW, y, pgv->gvar, attr) ;
			}
		}
		y += FH ;
	}
 }
 else
 {
	uint8_t sub = subN - 7 ;
	for (uint8_t i=0; i<NUM_SCALERS; i++ )
	{
		uint8_t y = (i+1)*FH ;
  	lcd_puts_Pleft( y, PSTR("SC\011+\015*\22/") ) ;
  	lcd_putc( 2*FW, y, i+'1' ) ;
		ScaleData *pscaler ;
		pscaler = &g_model.Scalers[i] ;
		FORCE_INDIRECT(pscaler) ;

		putsChnRaw( 4*FW, y, pscaler->source, 0 ) ;
		lcd_outdez( 12*FW+3, y, pscaler->offset ) ;
		lcd_outdez( 16*FW, y, pscaler->mult+1 ) ;
		lcd_outdez( 21*FW, y, pscaler->div+1 ) ;
	}
	lcd_char_inverse( 0, (sub+1)*FH, 126, 0 ) ;
 }
	asm("") ;
}


static void menuProcSafetySwitches(uint8_t sub) {

  TITLEP(PSTR(STR_SAFETY_SW));


  IlinesCount = NUM_CHNOUT + 1 - 1;
  uint8_t y = 0;
  uint8_t k = 0;
  uint8_t subSub = g_posHorz;
  uint8_t t_pgOfs;

  t_pgOfs = evalOffset(sub);
  sub++;
  for (uint8_t i = 0; i < 7; i++) {
    y = (i + 1) * FH;
    k = i + t_pgOfs;
    {
      uint8_t numSafety = 16;
      k++;
      SafetySwData *sd = &g_model.safetySw[k - 1];
      if (sub == k) {
        Columns = 2;
      }
      if (k - 1 >= NUM_CHNOUT) {
        sd = &g_model.xvoiceSw[k - 1 - NUM_CHNOUT];
      }
      for (uint8_t j = 0; j < 3; j++) {
        uint8_t attr = ((sub == k && subSub == j) ? InverseBlink : 0);

        uint8_t active = attr ? 2 : 0; // (attr && s_editing) ;
        if (k <= numSafety) {
          if (j == 0) {
            putsChn(0, y, k, 0);
            uint8_t value = sd->opt.ss.mode == 3 ? 1 : 0;
            value = checkIndexed(y, PSTR(FWx5 "\001"
                                              "\006SafetySticky"),
                value, active);
            sd->opt.ss.mode = value ? 3 : 0;
          } else if (j == 1) {
            int8_t max = MaxSwitchIndex;
              putsDrSwitches(11 * FW + 3, y, sd->opt.ss.swtch, attr);
            if (active) {
              CHECK_INCDEC_MODELSWITCH(sd->opt.ss.swtch, -MaxSwitchIndex, max);
            }
          } else {
            int8_t min, max;
            {
              min = -125;
              max = 125;
              lcd_outdezAtt(20 * FW, y, sd->opt.ss.val, attr);
            }
            if (active) {
              CHECK_INCDEC_H_MODELVAR(sd->opt.ss.val, min, max);
            }
          }
        }
      }
    }
  }
}

static void menuProcSwitches(uint8_t sub)  //Issue 78
{
	TITLEP(PSTR(STR_CUST_SWITCH)) ;
	IlinesCount = NUM_CSW+1-1 ;
	Columns = 3 ;
	uint8_t y = 0;
	uint8_t k = 0;
	uint8_t subSub = g_posHorz;
  uint8_t t_pgOfs ;

	t_pgOfs = evalOffset(sub);

	for(uint8_t i=0; i<7; i++)
	{
    y=(i+1)*FH;
    k=i+t_pgOfs;
    uint8_t attr = (sub==k ? InverseBlink  : 0);

		//write SW names here
    lcd_putc(  0 , y, 'L' );
    lcd_putc(  FW-1 , y, k + (k>8 ? 'A'-9: '1'));


		uint8_t att1 = subSub==1 ? attr : 0 ;
		uint8_t att2 = subSub==2 ? attr : 0 ;
		CSwData *cs = &g_model.customSw[k];

		lcd_putsAttIdx( 2*FW+1, y, Str_Cswitch,cs->func,subSub==0 ? attr : 0);

    uint8_t cstate = CS_STATE(cs->func);

    if(cstate == CS_VOFS)
    {
        if (cs->v1 > CHOUT_BASE+NUM_CHNOUT)
				{
					int16_t value = convertTelemConstant( cs->v1-CHOUT_BASE-NUM_CHNOUT-1, cs->v2 ) ;
					putsTelemetryChannel( 18*FW-8, y, cs->v1-CHOUT_BASE-NUM_CHNOUT-1, value, att2, TELEM_UNIT | TELEM_CONSTANT ) ;
				}
        else
				{
          lcd_outdezAtt( 18*FW-9, y, cs->v2  , att2);
				}
        putsChnRaw(    10*FW-6, y, cs->v1, att1 );
    }
    else if(cstate == CS_VBOOL)
    {
        putsDrSwitches(10*FW-6, y, cs->v1  , att1 ) ;
        putsDrSwitches(14*FW-7, y, cs->v2  ,att2) ;
    }
    else if(cstate == CS_VCOMP)
    {
        putsChnRaw(    14*FW-4, y, cs->v2  ,att2 ) ;
        putsChnRaw(    10*FW-6, y, cs->v1  , att1 ) ;
    }
		else // cstate == CS_TIMER
		{
			int8_t x ;
			uint8_t att = att1 ;
			x = cs->v1 ;
			if ( x < 0 )
			{
				x = -x-1 ;
				att |= PREC1 ;
			}
			lcd_xlabel_decimal( 13*FW-5, y, x+1, att, PSTR(STR_15_ON) ) ;
			att = att2 ;
			x = cs->v2 ;
			if ( x < 0 )
			{
				x = -x-1 ;
				att |= PREC1 ;
			}
      lcd_outdezAtt( 18*FW-1, y, x+1 , att ) ;
		}
		{
			int8_t as ;
			as = cs->andsw ;
			if ( as > 8 )
			{
				as += 1 ;
			}
			putsDrSwitches( 17*FW+2, y, as,(subSub==3 ? attr : 0)) ;
		}


		if((s_editing ) && attr)	// Use s_editing???
		{
    	switch (subSub)
			{
    	  case 0:
    	    CHECK_INCDEC_H_MODELVAR_0( cs->func, CS_MAXF);
    	    if(cstate != CS_STATE(cs->func))
    	    {
    	      cs->v1  = 0;
    	      cs->v2 = 0;
    	    }
    	  break;

				case 1:
    	    switch (cstate)
					{
    	      case (CS_VOFS):
    	        CHECK_INCDEC_H_MODELVAR( cs->v1, 0, NUM_XCHNRAW+NUM_TELEM_ITEMS);
    	      break;
    	      case (CS_VBOOL):
    	        CHECK_INCDEC_MODELSWITCH( cs->v1, -MaxSwitchIndex, MaxSwitchIndex );
    	      break;
    	      case (CS_VCOMP):
    	        CHECK_INCDEC_H_MODELVAR( cs->v1, 0, NUM_XCHNRAW+NUM_TELEM_ITEMS);
    	      break;
    	      case (CS_TIMER):
    	        CHECK_INCDEC_H_MODELVAR( cs->v1, -50, 99);
    	      break;

    	      default:
    	      break;
    	    }
    	  break;

				case 2:
    	    switch (cstate)
					{
    	      case (CS_VOFS):
    	        CHECK_INCDEC_H_MODELVAR( cs->v2, -125,125);
    	      break;
    	      case (CS_VBOOL):
    	        CHECK_INCDEC_MODELSWITCH( cs->v2, -MaxSwitchIndex, MaxSwitchIndex );
    	      break;
    	      case (CS_VCOMP):
    	        CHECK_INCDEC_H_MODELVAR( cs->v2, 0, NUM_XCHNRAW+NUM_TELEM_ITEMS);
    	      break;
    	      case (CS_TIMER):
    	        CHECK_INCDEC_H_MODELVAR( cs->v2, -50, 99);
    	      break;
    	      default:
    	      break;
    	    }
    	  break;
    	  case 3:
    	    CHECK_INCDEC_H_MODELVAR_0( cs->andsw, 15 ) ;
				break;
    	}
		}
	}
}

static uint8_t s_currMixIdx;
static uint8_t s_moveMixIdx;
static int8_t s_currDestCh;


static void deleteMix(uint8_t idx)
{
    MixData *md = mixaddress( idx ) ;

		memmove( md, md+1,(MAX_MIXERS-(idx+1))*sizeof(MixData));
    memset(&g_model.mixData[MAX_MIXERS-1],0,sizeof(MixData));
    STORE_MODELVARS;
}

static void insertMix(uint8_t idx, uint8_t copy)
{
    MixData *md = &g_model.mixData[0] ;
		md += idx ;

    memmove( md+1, md,(MAX_MIXERS-(idx+1))*sizeof(MixData) );
		if ( copy )
		{
	    memmove( md, md-1, sizeof(MixData) ) ;
		}
		else
		{
	    memset( md,0,sizeof(MixData));
  	  md->destCh      = s_currDestCh; //-s_mixTab[sub];
    	md->srcRaw      = s_currDestCh; //1;   //
    	md->weight      = 100;
			md->lateOffset  = 1 ;
		}
		s_currMixIdx = idx ;
    STORE_MODELVARS;
}

int8_t edit_dr_switch( uint8_t x, uint8_t y, int8_t drswitch, uint8_t attr, uint8_t flags )
{
	if ( flags & EDIT_DR_SWITCH_MOMENT )
	{
		putsMomentDrSwitches( x, y, drswitch, attr ) ;
	}
	else
	{
		putsDrSwitches( x,  y, drswitch, attr ) ;
	}
	if(flags & EDIT_DR_SWITCH_EDIT)
	{
		if ( flags & EDIT_DR_SWITCH_MOMENT )
		{

			int16_t value = drswitch ;

  	  CHECK_INCDEC_H_MODELVAR( value ,(1-MaxSwitchIndex),(-2+2*MaxSwitchIndex));

			drswitch = value ;


		}
		else
		{
			CHECK_INCDEC_MODELSWITCH( drswitch, -MaxSwitchIndex, MaxSwitchIndex ) ;
		}
	}
	asm("") ;
	return drswitch ;
}


static uint8_t editSlowDelay( uint8_t y, uint8_t attr, uint8_t value)
{
  if(attr)  CHECK_INCDEC_H_MODELVAR_0( value, 15); //!! bitfield
	uint8_t lval = value ;
	if ( g_model.mixTime )
	{
		lval *= 2 ;
		attr |= PREC1 ;
	}
  lcd_outdezAtt(FW*16,y,lval,attr);
	return value ;
}

#define MAXSW23POS  6
  const prog_char APM SW_3_IDX[] = "\004sIDxsA  sB  sD  sC0 sC1 sC2 " ;

void putsChnOpRaw( uint8_t x, uint8_t y, MixData *md2, uint8_t attr )
{
	if ( md2->srcRaw == MIX_3POS )
	{
		lcd_putsAttIdx( x, y, SW_3_IDX, md2->sw23pos, attr ) ;
	}
	else
	{
		putsChnRaw( x, y, md2->srcRaw, attr| MIX_SOURCE ) ;
		if ( ( md2->srcRaw >= CHOUT_BASE + 1 ) && ( md2->srcRaw < CHOUT_BASE + NUM_CHNOUT + 1 ) )
		{
			if ( md2->disableExpoDr )
			{
				lcd_putsAtt( x, y, PSTR("OP"), attr ) ;
			}
		}
	}
	asm("") ;
}


void menuProcMixOne(uint8_t event)
{
  MixData *md2 = mixaddress( s_currMixIdx ) ;

	static MState2 mstate2 ;
#ifdef MIX_WARN
	mstate2.check_columns(event, 15 ) ;
#else
	mstate2.check_columns(event, 14 ) ;
#endif

		uint8_t x = TITLEP( PSTR(STR_EDIT_MIX));

    putsChn(x+1*FW,0,md2->destCh,0);
    uint8_t  sub    = mstate2.m_posVert;

    uint8_t t_pgOfs = evalOffset(sub);
#ifdef MIX_WARN
		lcd_puts_Pskip( FH, PSTR(STR_2SOURCE "\037" STR_2WEIGHT "\037" "\001" STR_OFFSET "\037" STR_2FIX_OFFSET
			"\037" STR_ENABLEEXPO "\037" STR_2TRIM "\037\037\037" STR_2SWITCH "\037" "\001" STR_MODES "\037" STR_2WARNING
			"\037" STR_2MULTIPLEX "\037" STR_2DELAY_DOWN "\037" STR_2DELAY_UP "\037" STR_2SLOW_DOWN "\037"
			STR_2SLOW_UP), s_pgOfs ) ;
#else
		lcd_puts_Pskip( FH, PSTR(STR_2SOURCE "\037" STR_2WEIGHT "\037" "\001" STR_OFFSET "\037" STR_2FIX_OFFSET
			"\037" STR_ENABLEEXPO "\037" STR_2TRIM "\037\037\037" STR_2SWITCH "\037" "\001" STR_MODES
			"\037" STR_2MULTIPLEX "\037" STR_2DELAY_DOWN "\037" STR_2DELAY_UP "\037" STR_2SLOW_DOWN "\037"
			STR_2SLOW_UP), s_pgOfs ) ;
#endif

    for(uint8_t k=0; k<7; k++)
    {
        uint8_t y = (k+1) * FH;
        uint8_t i = k + t_pgOfs;
        uint8_t attr = sub==i ? InverseBlink : 0;
#ifdef MIX_WARN
    		uint8_t b ;
#endif
        switch(i){
        case 0:
				{
					uint8_t value = md2->srcRaw ;
					putsChnOpRaw( FW*14,y,md2, attr ) ;
					if ( value == MIX_3POS )
					{
						value += md2->sw23pos ;
					}
					else
					{
						if ( value > MIX_3POS )
						{
							value += MAXSW23POS ;
						}
					}
					if ( attr )
					{
            CHECK_INCDEC_H_MODELVAR( value, 1,NUM_XCHNRAW+1+MAX_GVARS+1+NUM_SCALERS + MAXSW23POS ) ;	// MAXSW23POS for switches
						if ( value >= MIX_3POS )
						{
							if ( value > MIX_3POS + MAXSW23POS )
							{
								value -= MAXSW23POS ;
							}
							else
							{
								md2->sw23pos = value - MIX_3POS ;
								value = MIX_3POS ;
							}
						}
						md2->srcRaw = value ;
					}
				}
				break ;

				case 1:
						md2->weight = gvarMenuItem125( FW*16, y, md2->weight, attr ) ;
            break;
        case 2:
						md2->sOffset = gvarMenuItem125( FW*16, y, md2->sOffset, attr ) ;
            break;
        case 3:
						md2->lateOffset = onoffItem( md2->lateOffset, y, attr ) ;
            break;
        case 4:
						if ( ( md2->srcRaw <=4 ) )
						{
							md2->disableExpoDr = offonItem( md2->disableExpoDr, y, attr ) ;
						}
						else
						{
							md2->disableExpoDr = onoffMenuItem( md2->disableExpoDr, y, PSTR("\001Use Output   "), attr ) ;
						}
						break;
        case 5:
						md2->carryTrim = offonItem( md2->carryTrim, y, attr ) ;
						break;
        case 6 :
					{
					 	uint8_t value = md2->differential ;
						if ( value == 0 )
						{
							if ( ( md2->curve <= -28 ) || ( md2->curve >= 27 ) )
							{
								value = 2 ;		// Expo
							}
						}
					 	uint8_t value2 = value ;
	          lcd_putsAtt(  1*FW, y, value ? ( value == 2 ) ? PSTR("\021Expo") : PSTR(STR_15DIFF) : Str_Curve, attr ) ;
    		    if(attr) CHECK_INCDEC_H_MODELVAR_0( value2, 2 ) ;
					 	if ( value != value2 )
						{
							if ( value2 == 2 )
							{
								md2->curve = -128 ;
							}
							else
							{
								md2->curve = 0 ;
							}
							md2->differential = value2 & 1 ;	// 0 and 2 turn it off
						}
					}
        break;
        case 7 :
						if ( md2->differential )		// Non zero for curve
						{
		          md2->curve = gvarMenuItem( 12*FW, y, md2->curve, -100, 100, attr ) ;
						}
						else
						{
							if ( ( md2->curve <= -28 ) || ( md2->curve > 27 ) )
							{
								int8_t value = md2->curve + 128 ;	// 0 to 100, AND -100 to -1
            		lcd_outdezAtt(FW*17,y,value,attr|LEFT);
            		if(attr) CHECK_INCDEC_H_MODELVAR( value, -100, 100 ) ;
								md2->curve = value - 128 ;
							}
							else
							{
								put_curve( 2*FW, y, md2->curve, attr ) ;
          	  	if(attr)
								{
									CHECK_INCDEC_H_MODELVAR( md2->curve, -MAX_CURVE5-MAX_CURVE9 , MAX_CURVE5+MAX_CURVE9+7-1);
									if ( event==EVT_KEY_FIRST(KEY_MENU) )
									{
										if ( md2->curve>=CURVE_BASE )
										{
          	  	    	s_curveChan = md2->curve-CURVE_BASE;
	          		      pushMenu(menuProcCurveOne);
										}
										if ( md2->curve < 0 )
										{
          	  	    	s_curveChan = -md2->curve-1 ;
	          		      pushMenu(menuProcCurveOne);
										}
									}
          	  	}
							}
						}
        break;
        case 8:
						md2->swtch = edit_dr_switch( 13*FW, y, md2->swtch, attr, attr ? EDIT_DR_SWITCH_EDIT : 0 ) ;
            break;
        case 9:
					{
						uint8_t b = 1 ;

						if ( attr )
						{
							Columns = 4 ;
						}

  					for ( uint8_t p = 0 ; p<MAX_MODES+1 ; p++ )
						{
							uint8_t z = md2->modeControl ;
    					lcd_putcAtt( (9+p)*(FW+1), y, '0'+p, ( z & b ) ? 0 : INVERS ) ;
							if( attr && ( g_posHorz == p ) )
							{
								lcd_rect( (9+p)*(FW+1)-1, y-1, FW+2, 9 ) ;
								if ( event==EVT_KEY_BREAK(KEY_MENU) || event==EVT_KEY_BREAK(BTN_RE) )
								{
									md2->modeControl ^= b ;
      						eeDirty(EE_MODEL) ;
    							s_editMode = false ;
								}
							}
							b <<= 1 ;
						}
					}
        break ;
#ifdef MIX_WARN
        case 10:
						b = md2->mixWarn ;
						putsOffDecimal( FW*13, y, b, attr ) ;
            if(attr) { CHECK_INCDEC_H_MODELVAR_0( b, 3); md2->mixWarn = b ; }
            break;
#endif
#ifdef MIX_WARN
        case 11:
#else
        case 10:
#endif
						md2->mltpx = checkIndexed( y, PSTR(FWx13 "\002" STR_ADD_MULT_REP), md2->mltpx, (sub==i) ) ;
            break;
#ifdef MIX_WARN
        case 12:
#else
        case 11:
#endif
						md2->delayUp = editSlowDelay( y, attr, md2->delayUp ) ;	// Sense was wrong
            break;
#ifdef MIX_WARN
        case 13:
#else
        case 12:
#endif
						md2->delayDown = editSlowDelay( y, attr, md2->delayDown ) ;	// Sense was wrong
            break;
#ifdef MIX_WARN
        case 14:
#else
        case 13:
#endif
						md2->speedDown = editSlowDelay( y, attr, md2->speedDown ) ;
            break;
#ifdef MIX_WARN
        case 15:
#else
        case 14:
#endif
						md2->speedUp = editSlowDelay( y, attr, md2->speedUp ) ;
            break;
        }
    }
}

int8_t s_mixMaxSel;

static void memswap( void *a, void *b, uint8_t size )
{
    uint8_t *x ;
    uint8_t *y ;
    uint8_t temp ;

    x = (unsigned char *) a ;
    y = (unsigned char *) b ;
    while ( size-- )
    {
        temp = *x ;
        *x++ = *y ;
        *y++ = temp ;
    }
}

static void moveMix(uint8_t idx, uint8_t dir) //true=inc=down false=dec=up - Issue 49
{
    MixData *src= mixaddress( idx ) ; //&g_model.mixData[idx];
    if(idx==0 && !dir)
		{
      if (src->destCh>1)
			{
			  src->destCh--;
			}
			STORE_MODELVARS;
			return ;
		}

    if(idx>MAX_MIXERS || (idx==MAX_MIXERS && dir)) return ;
    uint8_t tdx = dir ? idx+1 : idx-1;
    MixData *tgt= mixaddress( tdx ) ; //&g_model.mixData[tdx];

    if((src->destCh==0) || (src->destCh>NUM_CHNOUT) || (tgt->destCh>NUM_CHNOUT)) return ;

    if(tgt->destCh!=src->destCh)
		{
      if ((dir)  && (src->destCh<NUM_CHNOUT)) src->destCh++;
      if ((!dir) && (src->destCh>0))          src->destCh--;
			STORE_MODELVARS;
			return ;
    }

    //flip between idx and tgt
    memswap( tgt, src, sizeof(MixData) ) ;
		s_moveMixIdx = tdx ;

		STORE_MODELVARS;
}

void menuMixersLimit(uint8_t event)
{
    switch(event)
    {
    case  EVT_KEY_FIRST(KEY_EXIT):
        killEvents(event);
        popMenu(true);
        pushMenu(menuProcMix);
        break;
    }
		lcd_puts_Pleft(2*FH, PSTR(STR_MAX_MIXERS_EXAB));
}

static uint8_t getMixerCount()
{
  if ( g_model.mixData[MAX_MIXERS-1].destCh )
	{
		return MAX_MIXERS ;
	}
	else
	{
		return MAX_MIXERS - 1 ;
	}
}

bool reachMixerCountLimit()
{
    // check mixers count limit
    if (getMixerCount() >= MAX_MIXERS)
    {
        pushMenu(menuMixersLimit);
        return true;
    }
    else
    {
        return false;
    }
}

// May be able to put this in Xmem
uint8_t mixToDelete;

#define YN_NONE	0
#define YN_YES	1
#define YN_NO		2

uint8_t yesNoMenuExit( uint8_t event, const prog_char * s )
{
	uint8_t reply = YN_NONE ;
	lcd_puts_Pleft(1*FH, s ) ;
  lcd_puts_Pleft( 5*FH,PSTR(STR_YES_NO_MENU_EXIT));

  switch(event)
	{
    case EVT_ENTRY:
      audioDefevent(AU_WARNING1);
    break ;

    case EVT_KEY_FIRST(KEY_MENU):
		case EVT_KEY_BREAK(BTN_RE):
			reply = YN_YES ;
    break ;

		case EVT_KEY_LONG(BTN_RE):
    case EVT_KEY_FIRST(KEY_EXIT):
			reply = YN_NO ;
    break;
  }
	if ( reply != YN_NONE )
	{
		killEvents(event);
    popMenu(true);
	}
	return reply ;
}


void menuDeleteMix(uint8_t event)
{
	uint8_t action ;
	action = yesNoMenuExit( event, (mixToDelete== 0xFF) ? PSTR(STR_CLEAR_ALL_MIXES) : PSTR(STR_DELETE_MIX) ) ;

	switch(action)
	{
    case YN_YES :
			if (mixToDelete == 0xFF)
			{
        clearMixes() ;
			}
			else
			{
      	deleteMix(mixToDelete);
			}
      //fallthrough
		case YN_NO :
      pushMenu(menuProcMix);
    break;
  }
}

uint8_t s_moveMode;

#define POPUP_NONE			0
#define POPUP_SELECT		1
#define POPUP_EXIT			2
#define POPUP_LONG			3


static uint8_t popupProcess( uint8_t max )
{
	int8_t popidxud = qRotary() ;
	struct t_popupData *ptrPopData = &PopupData ;
	FORCE_INDIRECT(ptrPopData) ;

	uint8_t popidx = ptrPopData->PopupIdx ;

	if ( ptrPopData->PopupTimer )
	{
		if ( BLINK_ON_PHASE )
		{
			if ( --ptrPopData->PopupTimer == 0 )
			{
				// Timeout popup
				ptrPopData->PopupActive = 0 ;
				return POPUP_EXIT ;
			}
		}
	}
	else
	{
		ptrPopData->PopupTimer = 255 ;
  }

	switch(Tevent)
	{
    case EVT_KEY_BREAK(KEY_MENU) :
    case EVT_KEY_BREAK(BTN_RE):
			ptrPopData->PopupActive = 0 ;
			ptrPopData->PopupTimer = 0 ;
		return POPUP_SELECT ;

		case EVT_KEY_FIRST(KEY_UP) :
			popidxud = -1 ;
		break ;

		case EVT_KEY_FIRST(KEY_DOWN) :
			popidxud = 1 ;
		break ;

		case EVT_KEY_FIRST(KEY_EXIT) :
		case EVT_KEY_LONG(BTN_RE) :
			killEvents( Tevent ) ;
			Tevent = 0 ;
			ptrPopData->PopupActive = 0 ;
			ptrPopData->PopupTimer = 0 ;
		return POPUP_EXIT ;
	}

	if (popidxud > 0)
	{
		if ( popidx < max )
		{
			popidx += 1 ;
		}
	}
	else if (popidxud < 0)
	{
		if ( popidx )
		{
			popidx -= 1 ;
		}
	}

	if (popidxud )
	{
		if ( ptrPopData->PopupTimer )
		{
			ptrPopData->PopupTimer = 255 ;
		}
	}

	ptrPopData->PopupIdx = popidx ;
	return POPUP_NONE ;
}

const prog_char APM MixPopList[] = STR_MIX_POPUP ;


static uint8_t popupDisplay(const prog_char *list, uint8_t mask, uint8_t width) {
  uint8_t entries = 0;
  uint8_t y = FH;

  while (mask) {
    if (mask & 1) {
      lcd_putsn_P(3 * FW, y, PSTR("              "), width);
      lcd_puts_P(4 * FW, y, (const char *)(list));
      entries += 1;
      y += FH;
    }
    mask >>= 1;
    while (pgm_read_byte(list)) {
      list += 1;
    }
    list += 1;
  }
  lcd_rect(3 * FW, 1 * FH - 1, width * FW, y + 2 - FH);
  lcd_char_inverse(4 * FW, (PopupData.PopupIdx + 1) * FH, (width - 2) * FW, 0);
  return entries;
}

static uint8_t popTranslate( uint8_t popidx, uint8_t mask )
{
	uint8_t position ;
	popidx += 1 ;
	for ( position = 0 ; position < 8 ; position += 1 )
	{
		if ( mask & 1 )
		{
			if ( --popidx == 0)
			{
				break ;
			}
		}
		mask >>= 1 ;
	}
	return position ;
}


uint8_t doPopup( const prog_char *list, uint8_t mask, uint8_t width )
{
	CPU_UINT count = popupDisplay( list, mask, width ) ;
	uint8_t popaction = popupProcess( count - 1 ) ;
	uint8_t popidx = PopupData.PopupIdx ;
	PopupData.PopupSel = popTranslate( popidx, mask ) ;
	return popaction ;
}


static void mixpopup()
{
	uint8_t popaction = doPopup( MixPopList, 0x3F, 11 ) ;

	uint8_t popidx = PopupData.PopupSel ;

  if ( popaction == POPUP_SELECT )
	{
		if ( popidx == 1 )
		{
      if ( !reachMixerCountLimit())
      {
      	insertMix(++s_currMixIdx, 0 ) ;
  	    s_moveMode=false;
			}
		}
		if ( popidx < 2 )
		{
	    pushMenu(menuProcMixOne) ;
		}
		else if ( popidx == 4 )		// Delete
		{
			mixToDelete = s_currMixIdx;
			killEvents(Tevent);
			Tevent = 0 ;
			pushMenu(menuDeleteMix);
		}
		else if ( popidx == 5 )		// Clear all
		{
			mixToDelete = 0xFF ;
			killEvents(Tevent);
			Tevent = 0 ;
			pushMenu(menuDeleteMix);
		}
		else
		{
			if( popidx == 2 )	// copy
			{
     		insertMix(++s_currMixIdx, 1 ) ;
			}
			// PopupIdx == 2 or 3, copy or move
			s_moveMode = 1 ;
		}
		PopupData.PopupActive = 0 ;
	}
	s_moveMixIdx = s_currMixIdx ;

}


void menuProcMix(uint8_t event)
{
	TITLEP(Str_Mixer);
	static MState2 mstate2;

	if ( s_moveMode )
	{
		int8_t moveByRotary ;
		moveByRotary = qRotary() ;		// Do this now, check_simple destroys rotary data
		if ( moveByRotary )
		{
			if ( moveByRotary > 0 )
			{
				event = EVT_KEY_FIRST(KEY_DOWN) ;
			}
			else
			{
				event = EVT_KEY_FIRST(KEY_UP) ;
			}
		}
		uint8_t v = mstate2.m_posVert ;
		if ( ( ( v == 0 ) && ( event == EVT_KEY_FIRST(KEY_UP) ) )
				 || ( ( v == s_mixMaxSel ) && ( event == EVT_KEY_FIRST(KEY_DOWN) ) ) )
		{
			event = 0 ;
		}
		Tevent = event ;
	}

	if ( !PopupData.PopupActive )
	{
			mstate2.check_columns(event,s_mixMaxSel) ;
	}

	uint8_t  sub    = mstate2.m_posVert  ;
	uint8_t	menulong = 0 ;

  switch(Tevent)
	{
    case EVT_ENTRY:
	    s_moveMode = false ;
		break ;

    case EVT_KEY_FIRST(KEY_MENU):
    case EVT_KEY_BREAK(BTN_RE):
			if ( s_moveMode )
			{
	    	s_moveMode = false ;
    		s_editMode = false ;
				RotaryState = ROTARY_MENU_UD ;
				break ;
			}
			// Else fall through
			if ( !PopupData.PopupActive )
			{
		  	killEvents(Tevent);
				Tevent = 0 ;			// Prevent changing weight to/from Gvar
				menulong = 1 ;
			}
    break;
	}

	uint8_t t_pgOfs = evalOffset( sub) ;

	if ( PopupData.PopupActive )
	{
		Tevent = 0 ;
	}

  uint8_t mix_index = 0 ;
  uint8_t current = 0 ;

	if ( s_moveMode )
	{
		uint8_t dir ;

		if ( ( dir = (Tevent == EVT_KEY_FIRST(KEY_DOWN) ) ) || Tevent == EVT_KEY_FIRST(KEY_UP) )
		{
			moveMix( s_currMixIdx, dir ) ; //true=inc=down false=dec=up - Issue 49
		}
	}

  for ( uint8_t chan=1 ; chan <= NUM_CHNOUT ; chan += 1 )
	{
    MixData *pmd ;

		pmd=mixaddress(mix_index) ;

    if ( t_pgOfs <= current && current-t_pgOfs < 7)
		{
      putsChn( 1, (current-t_pgOfs+1)*FH, chan, 0) ; // show CHx
    }

		uint8_t firstMix = mix_index ;

		if (mix_index<MAX_MIXERS && /* pmd->srcRaw && */ pmd->destCh == chan)
		{
    	do
			{
				if (t_pgOfs <= current )
				{
					if ( current-t_pgOfs < 7 )
					{
    	  	  uint8_t y = (current-t_pgOfs+1)*FH ;
    				uint8_t attr = 0 ;

						if ( !s_moveMode && (sub == current) )
						{
							s_currMixIdx = mix_index ;
							s_currDestCh = chan ;		// For insert
							if ( menulong )
							{
								PopupData.PopupIdx = 0 ;
								PopupData.PopupActive = 1 ;
								event = 0 ;		// Kill this off
							}
							if ( PopupData.PopupActive == 0 )
							{
    						attr = INVERS ;
							}
						}
        	  if(firstMix != mix_index) //show prefix only if not first mix
        	 		lcd_putsAttIdx( 3*FW+1, y, PSTR("\001+*R"),pmd->mltpx,0 ) ;

						putsChnOpRaw( 8*FW, y, pmd, 0 ) ;


						pmd->weight = gvarMenuItem125( 7*FW+FW/2, y, pmd->weight, attr ) ;
    	  	  if( pmd->swtch) putsDrSwitches( 12*FW, y, pmd->swtch, 0 ) ; //tattr);
						if ( pmd->curve )
						{
							if ( pmd->differential ) lcd_putcAtt( 16*FW, y, CHR_d, 0 ) ;
							else
							{
								if ( ( pmd->curve > -28 ) && ( pmd->curve <= 27 ) )
								{
	    	  	  		put_curve( 16*FW, y, pmd->curve, 0 ) ;
								}
								else
								{
									lcd_putcAtt( 16*FW, y, 'E', 0 ) ;
								}
							}
						}
						char cs = ' ';
        	  if (pmd->speedDown || pmd->speedUp)
        	    cs = CHR_S ;
        	  if ((pmd->delayUp || pmd->delayDown))
        	    cs = (cs ==CHR_S ? '*' : CHR_D);
        	  lcd_putc(20*FW+1, y, cs ) ;

						if ( s_moveMode )
						{
							if ( s_moveMixIdx == mix_index )
							{
								lcd_char_inverse( 4*FW, y, 17*FW, 0 ) ;
								s_currMixIdx = mix_index ;
								sub = mstate2.m_posVert = current ;
							}
						}
					}
					else
					{
						if ( current-t_pgOfs == 7 )
						{
							if ( s_moveMode )
							{
								if ( s_moveMixIdx == mix_index )
								{
									mstate2.m_posVert += 1 ;
								}
							}
						}
					}
				}
				current += 1 ; mix_index += 1; pmd += 1 ;  // mixCnt += 1 ;
    	} while ( (mix_index<MAX_MIXERS && /* pmd->srcRaw && */ pmd->destCh == chan)) ;
		}
		else
		{
			if (sub == current)
			{
				s_currDestCh = chan ;		// For insert
				s_currMixIdx = mix_index ;
				lcd_rect( 0, (current-t_pgOfs+1)*FH-1, 25, 9 ) ;
				if ( menulong )		// Must need to insert here
				{
      		if ( !reachMixerCountLimit())
      		{
      			insertMix(s_currMixIdx, 0 ) ;
  	    		s_moveMode=false;
	      		pushMenu(menuProcMixOne) ;
						break ;
      		}
				}
			}
			current += 1 ;
		}
	}
	if ( PopupData.PopupActive )
	{
		Tevent = event ;
		mixpopup() ;
    s_editMode = false;
	}
	s_mixMaxSel = current - 1 ;
}

uint16_t expou(uint16_t x, uint16_t k)
{
  // previous function was this one:
    // k*x*x*x + (1-k)*x
//    return ((unsigned long)x*x*x/0x10000*k/(RESXul*RESXul/0x10000) + (RESKul-k)*x+RESKul/2)/RESKul;

  uint32_t value = (uint32_t) x*x;
  value *= (uint32_t)k;
  value >>= 8;
  value *= (uint32_t)x;
  value >>= 12;
  value += (uint32_t)(100-k)*x+50;

  return value/100;
}
// expo-funktion:
// ---------------
// kmplot
// f(x,k)=exp(ln(x)*k/10) ;P[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20]
// f(x,k)=x*x*x*k/10 + x*(1-k/10) ;P[0,1,2,3,4,5,6,7,8,9,10]
// f(x,k)=x*x*k/10 + x*(1-k/10) ;P[0,1,2,3,4,5,6,7,8,9,10]
// f(x,k)=1+(x-1)*(x-1)*(x-1)*k/10 + (x-1)*(1-k/10) ;P[0,1,2,3,4,5,6,7,8,9,10]

int16_t expo(int16_t x, int16_t k)
{
    if(k == 0) return x;
    int16_t   y;
    bool    neg =  x < 0;
    if(neg)   x = -x;
    if(k<0){
        y = RESXu-expou(RESXu-x,-k);
    }else{
        y = expou(x,k);
    }
    return neg? -y:y;
}


void editExpoVals( uint8_t edit,uint8_t x, uint8_t y, uint8_t which, uint8_t exWt, uint8_t stkRL)
{
    uint8_t  invBlk = ( edit ) ? InverseBlink : 0 ;
		uint8_t doedit ;
		int8_t *ptr ;
		ExpoData *eptr ;

		doedit = (edit ) ;

		eptr = &g_model.expoData[s_expoChan] ;
    if(which==DR_DRSW1) {

				eptr->drSw1 = edit_dr_switch( x, y, eptr->drSw1, invBlk, doedit ) ;
    }
    else if(which==DR_DRSW2) {
				eptr->drSw2 = edit_dr_switch( x, y, eptr->drSw2, invBlk, doedit ) ;
    }
    else
		{
				ptr = &eptr->expo[which][exWt][stkRL] ;
				FORCE_INDIRECT(ptr) ;

				if(exWt==DR_EXPO)
				{
					*ptr = gvarMenuItem( x, y, *ptr, -100, 100, invBlk ) ;
        }
        else
				{
					*ptr = gvarMenuItem( x, y, *ptr+100, 0, 100, invBlk ) - 100 ;
		    }
		}
}

uint8_t char2idx(char c)
{
	uint8_t ret ;
    for(ret=0;;ret++)
    {
        char cc= pgm_read_byte(s_charTab+ret);
        if(cc==c) return ret;
        if(cc==0) return 0;
    }
}
char idx2char(uint8_t idx)
{
    if(idx < NUMCHARS) return pgm_read_byte(s_charTab+idx);
    return ' ';
}

uint8_t DupIfNonzero = 0 ;
int8_t DupSub ;

void menuDeleteDupModel(uint8_t event)
{
	uint8_t action ;
  eeLoadModelName( DupSub,Xmem.buf,sizeof(Xmem.buf));
  lcd_putsnAtt(1,2*FH, Xmem.buf,sizeof(g_model.name),BSS);
  lcd_putc(sizeof(g_model.name)*FW+FW,2*FH,'?');
	action = yesNoMenuExit( event, DupIfNonzero ? PSTR(STR_DUP_MODEL) : PSTR(STR_DELETE_MODEL) ) ;

	switch(action)
	{
    case YN_YES :
      if ( DupIfNonzero )
      {
        message(PSTR(STR_DUPLICATING));
        if(eeDuplicateModel(DupSub))
        {
          audioDefevent(AU_MENUS);
          DupIfNonzero = 2 ;		// sel_editMode = false;
        }
        else audioDefevent(AU_WARNING1);
      }
      else
      {
        EFile::rm(FILE_MODEL( DupSub ) ) ; //delete file
      }
      pushMenu(menuProcModelSelect);
    break;

		case YN_NO :
      pushMenu(menuProcModelSelect);
    break;
  }
}

// Bit 7 = DSMX, Bit 6 = 11mS, bits 3-0 channels


void menuRangeBind(uint8_t event)
{
	static uint8_t timer ;
	uint8_t binding = pxxFlag & PXX_BIND ;
	lcd_puts_Pleft( 3*FH, (binding) ? PSTR("\006BINDING") : PSTR("RANGE CHECK RSSI:") ) ;
/*****************************************************************************/
// TODO AFHDS2A  AFHDS
        if (IS_BIND_DONE & IS_BIND_STOP) {
          cli();
          RadioState = ((TIM_CALL << CALLER) | (SEND << SEND_RES) | (AFHDS2A_BIND1));
          BIND_IN_PROGRESS;
          BIND_START;
        } else {
          if (IS_BIND_DONE) {
            BIND_STOP;
            pxxFlag = 0;
            eeDirty(EE_MODEL);    // Save RX_ID
            popMenu(false);
          }
        }
/*****************************************************************************/

if ( event == EVT_KEY_FIRST(KEY_EXIT) )
	{
    killEvents(event);
		pxxFlag = 0 ;
                BIND_DONE;
		BIND_STOP;
        popMenu(false) ;
	}


	if ( --timer == 0 )
	{
  	audioDefevent(AU_WARNING2) ;
	}
	asm("") ;
}


static void editTimer( uint8_t sub )
{
	uint8_t subN ;
	uint8_t timer ;
#ifdef GLOBAL_COUNTDOWN
	if ( sub < 5 )
#else
	if ( sub < 7 )
#endif
	{
		displayNext() ;
		subN = 0 ;
		timer = 0 ;
	}
	else
	{
#ifdef GLOBAL_COUNTDOWN
		subN = 5 ;
#else
		subN = 7 ;
#endif
		timer = 1 ;
	}
	TimerMode *ptConfig = &TimerConfig[timer] ;
	FORCE_INDIRECT(ptConfig) ;
	uint8_t y = FH ;
	uint8_t blink = InverseBlink ;

#ifdef GLOBAL_COUNTDOWN
    	lcd_puts_Pleft( y, PSTR(STR_TIMER_TEXT_X));
#else
    	lcd_puts_Pleft( y, PSTR(STR_TIMER_TEXT));
#endif
	    lcd_putc( 5*FW, y, '1'+ timer ) ;
    	putsTime(12*FW-1, y, ptConfig->tmrVal,(sub==subN ? blink:0),(sub==subN ? blink:0) ) ;

	    if(sub==subN)	// Use s_editing???
			{
				int16_t temp = 0 ;
				StepSize = 60 ;
				CHECK_INCDEC_H_MODELVAR( temp, -60 ,60 ) ;
				ptConfig->tmrVal += temp ;
        if((int16_t)ptConfig->tmrVal < 0) ptConfig->tmrVal=0;

			}
			y += FH ;
		subN++ ;

			uint8_t attr = 0 ;
    	if(sub==subN)
			{
   			attr = INVERS ;
    	  CHECK_INCDEC_H_MODELVAR_0( ptConfig->tmrModeA, 1+2+16 ) ;
			}
    	putsTmrMode(10*FW,y,attr, (timer == 0 ) ? 1 : 0x81 ) ;
			y += FH ;
		subN++ ;

  	  attr = 0 ;
  	  if(sub==subN)
			{
	   		attr = blink ;
			}
			uint8_t doedit = attr ? EDIT_DR_SWITCH_MOMENT | EDIT_DR_SWITCH_EDIT : EDIT_DR_SWITCH_MOMENT ;
			ptConfig->tmrModeB = edit_dr_switch( 15*FW, y, ptConfig->tmrModeB, attr, doedit ) ;
			y += FH ;
		subN++ ;

			ptConfig->tmrDir = checkIndexed( y, PSTR(FWx10 "\001" STR_COUNT_DOWN_UP), ptConfig->tmrDir, (sub==subN) ) ;
			y += FH ;
		subN++ ;

#ifndef GLOBAL_COUNTDOWN
			uint8_t b = ptConfig->tmrMbeep ;
  	  ptConfig->tmrMbeep = onoffMenuItem( b, y, Str_minute_Beep, sub ==subN ) ;
			y += FH ;
		subN++;

			b = ptConfig->tmrCdown ;
  	  ptConfig->tmrCdown = onoffMenuItem( b, y, Str_Beep_Countdown, sub==subN  ) ;
			y += FH ;
		subN++;
#endif

  	  attr = 0 ;
			if(sub==subN)
			{
				attr = blink ;
			}

			int16_t sw = (timer==0) ? g_model.timer1RstSw : g_model.timer2RstSw ;
			doedit = attr ? EDIT_DR_SWITCH_MOMENT | EDIT_DR_SWITCH_EDIT : EDIT_DR_SWITCH_MOMENT ;
			sw = edit_dr_switch( 13*FW+1, y, sw, attr, doedit ) ;
			if ( timer == 0 )
			{
				g_model.timer1RstSw = sw ;
			}
			else
			{
				g_model.timer2RstSw = sw ;
			}
			y += FH ;
}


void putsTrimMode( uint8_t x, uint8_t y, uint8_t phase, uint8_t idx, uint8_t att )
{
  int16_t v = getRawTrimValue(phase, idx);

  if (v > TRIM_EXTENDED_MAX)
	{
    uint8_t p = v - TRIM_EXTENDED_MAX - 1;
    if (p >= phase) p += 1 ;
    lcd_putcAtt(x, y, '0'+p, att);
  }
  else
	{
  	lcd_putsAttIdx( x, y, Str_1_RETA, idx, att ) ;
	}

	asm("") ;
}

  #define NUM_PHASE_ITEMS		4

void menuPhaseOne(uint8_t event)
{
	PhaseData *phase = &g_model.phaseData[s_currIdx];

	TITLE(STR_FL_MODE) ;
	static MState2 mstate2 ;
	mstate2.check_columns(event,NUM_PHASE_ITEMS-1) ;

	lcd_putc( 8*FW, 0, '1'+s_currIdx ) ;

  uint8_t sub = mstate2.m_posVert;

  for (uint8_t i = 0 ; i < NUM_PHASE_ITEMS ; i += 1 )
	{
    uint8_t y = (i+1) * FH;
		uint8_t attr = (sub==i ? InverseBlink : 0);

		switch(i)
		{
      case 0 : // switch
				lcd_puts_Pleft( y, PSTR(STR_SWITCH_TRIMS"\037""Fade In""\037""Fade Out") ) ;
				phase->swtch = edit_dr_switch( 8*FW, y, phase->swtch, attr, attr ? EDIT_DR_SWITCH_EDIT : 0 ) ;
			break;

      case 1 : // trims
				if ( attr )
				{
					Columns = 3 ;
				}
      	for ( uint8_t t = 0 ; t<NUM_STICKS ; t += 1 )
				{
      	  putsTrimMode( (10+t)*FW, y, s_currIdx+1, t, (g_posHorz==t) ? attr : 0 ) ;
      	  if (attr && g_posHorz==t && ( s_editing ) )
					{
  			    int16_t v = phase->trim[t] + ( TRIM_EXTENDED_MAX + 1 ) ;
      	    if (v < TRIM_EXTENDED_MAX)
						{
							v = TRIM_EXTENDED_MAX;
						}
						int16_t u = v ;
      	    v = checkIncDec16( u, TRIM_EXTENDED_MAX, TRIM_EXTENDED_MAX+MAX_MODES, EE_MODEL ) ;

						if (v != u)
						{
      	      if (v == TRIM_EXTENDED_MAX) v = 0 ;
							phase->trim[t] = v - ( TRIM_EXTENDED_MAX + 1 ) ;
      	    }
      	  }
      	}
      break;

			case 2 : // fadeIn
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( phase->fadeIn, 15 ) ;
			  lcd_outdezAtt( 17*FW, y, phase->fadeIn * 5, attr | PREC1 ) ;

			break ;

			case 3 : // fadeOut
      default:
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( phase->fadeOut, 15 ) ;
			  lcd_outdezAtt( 17*FW, y, phase->fadeOut * 5, attr | PREC1 ) ;
			break ;

	  }
	}
}

void dispFlightModename( uint8_t x, uint8_t y, uint8_t mode )
{
  lcd_puts_P( x, y, PSTR(STR_SP_FM)+1 ) ;
  lcd_putc( x+2*FW, y, '0'+mode ) ;
}

#define RESTORE_START		0
#define RESTORE_WAIT1		1
#define RESTORE_GET_NAMES	2
#define RESTORE_DISPLAY_NAMES	3
#define RESTORE_FILE_START	4
#define RESTORE_FILE_SAVING	5
#define RESTORE_FILE_DONE	6
#define RESTORE_BACKUP		7
#define RESTORE_BACKUP_DONE	8
#define RESTORE_LEAVING		9


void brSubHead(const prog_char *s)
{
	lcd_puts_P( 6*FW, 3*FH, s ) ;
}


void menuModelPhases(uint8_t event)
{
	uint8_t i ;
  uint8_t attr ;

	TITLEP(Str_Modes) ;
	static MState2 mstate2 ;
	mstate2.check_columns(event,5-1-1) ;

	uint8_t  sub    = mstate2.m_posVert ;

  switch (event)
	{
    case EVT_KEY_FIRST(KEY_MENU) :
    case EVT_KEY_FIRST(BTN_RE) :
        s_currIdx = sub ;
        killEvents(event);
        pushMenu(menuPhaseOne) ;
		break;
  }

	lcd_puts_Pleft( 2*FH, PSTR(STR_SP_FM0"\012RETA") ) ;

  for ( i=0 ; i<MAX_MODES ; i += 1 )
	{
    uint8_t y=(i+3)*FH ;
    attr = (i == sub) ? INVERS : 0 ;

		PhaseData *p = &g_model.phaseData[i];
		FORCE_INDIRECT(p) ;
		if ( p->fadeIn || p->fadeOut )
		{
	    lcd_putcAtt( 20*FW+1, y, '*', attr ) ;
		}
		dispFlightModename( FW, y, i+1 ) ;
    putsDrSwitches( 4*FW, y, p->swtch, attr ) ;
    for ( uint8_t t = 0 ; t < NUM_STICKS ; t += 1 )
		{
			putsTrimMode( (10+t)*FW, y, i+1, t, attr ) ;
		}
	}

	i = getFlightPhase() ;
	lcd_rect( 0, (i+2)*FH-1, 4*FW+2, 9 ) ;

}

static void qloadModel( uint8_t event, uint8_t index )
{


	killEvents(event);
	wdt_reset() ;
  eeWaitComplete();    // Wait to load model if writing something
	wdt_reset() ;
  eeLoadModel( index ) ;
	wdt_reset() ;
	AlarmControl.VoiceCheckFlag |= 2 ;// Set switch current states
  STORE_GENERALVARS;
	wdt_reset() ;
  eeWaitComplete();
  checkTHR() ;
  checkSwitches();
	putVoiceQueueUpper( g_model.modelVoice ) ;
}

// Popup?
// SELECT - Selects model and exit
// COPY - copy model to another model slot
// MOVE- move model to another model slot
// DELETE - This one is important.

const prog_char APM ModelPopList[] = STR_MODEL_POPUP ;

void menuProcModelSelect(uint8_t event)
{
    static MState2 mstate2;
    TITLE(STR_MODELSEL);

    int8_t subOld  = mstate2.m_posVert;

		if ( !PopupData.PopupActive )
		{
			mstate2.check_columns(event, MAX_MODELS-1);
		}

    lcd_puts_Pleft(  0, PSTR(STR_11_FREE));
    lcd_outdez(  17*FW, 0, EeFsGetFree());

    uint8_t  sub    = mstate2.m_posVert;
    static uint8_t sel_editMode;
    if ( DupIfNonzero == 2 )
    {
        sel_editMode = false ;
        DupIfNonzero = 0 ;
    }
		uint8_t t_pgOfs = s_pgOfs ;
		uint8_t temp = 1+t_pgOfs ;
    if(sub < temp)
		{
			t_pgOfs = sub > 1 ? sub-1 : 0 ;
		}
    else if(sub > (temp = 4+t_pgOfs) )
		{
		  t_pgOfs = sub - 4 ;
			if ( t_pgOfs > MAX_MODELS-6 )
			{
				t_pgOfs = MAX_MODELS-6 ;
			}
		}
		s_pgOfs = t_pgOfs ;

    for(uint8_t i=0; i<6; i++){
        uint8_t y=(i+2)*FH;
        uint8_t k=i+t_pgOfs;
				lcd_2_digits( 3*FW, y, k+1, (sub==k) ? INVERS : 0 ) ;
        if(k==g_eeGeneral.currModel) lcd_putc(1,  y,'*');
        eeLoadModelName(k,Xmem.buf,sizeof(Xmem.buf));
        lcd_putsnAtt(  4*FW, y, Xmem.buf,sizeof(Xmem.buf),BSS|((sub==k) ? (sel_editMode ? INVERS : 0 ) : 0));
    }

	if ( PopupData.PopupActive )
	{
		uint8_t mask = (g_eeGeneral.currModel == mstate2.m_posVert) ? 0x59 : 0x7E ;
		if ( eeModelExists( mstate2.m_posVert ) == 0 )
		{
			mask = 0x96 ;
		}
		if ( ( g_eeGeneral.speakerMode & 4 ) == 0 )
		{
			mask &= 0x3F ;	// If no megasound, no backup/restore
		}
		uint8_t popaction = doPopup( ModelPopList, mask, 10 ) ;

  	if ( popaction == POPUP_SELECT )
		{
			uint8_t popidx = PopupData.PopupSel ;
			if ( popidx == 0 )	// edit
			{
				chainMenu(menuProcModelIndex) ;
			}
			else if ( ( popidx == 1 ) || ( popidx == 2 ) )	// select or SEL/EDIT
			{
       	g_eeGeneral.currModel = mstate2.m_posVert;
				PausePulses = 1 ;
			 	qloadModel( event, mstate2.m_posVert ) ;
				startPulses() ;
				if ( ( PopupData.PopupActive == 2 ) || ( popidx == 2 ) )
				{
					chainMenu(menuProcModelIndex) ;
				}
				else
				{
	        popMenu(true) ;
				}
			}
			else if ( popidx == 5 )		// Delete
			{
       	killEvents(event);
       	DupIfNonzero = 0 ;
				DupSub = sub ;
       	pushMenu(menuDeleteDupModel);
			}
			else if( popidx == 3 )	// copy
			{
				{
 	        DupIfNonzero = 1 ;
 	        DupSub = sub ;
 	        pushMenu(menuDeleteDupModel);
				}
			}
			else // Move (4)
			{
                              sel_editMode = true ;
			}
		}
	}
	else
	{
		struct t_popupData *ppupdata = &PopupData ;
		switch(event)
    {
    	case  EVT_KEY_FIRST(KEY_EXIT):
        if(sel_editMode)
				{
            sel_editMode = false;
        }
    	break;

			case  EVT_KEY_FIRST(KEY_LEFT):
    	case  EVT_KEY_FIRST(KEY_RIGHT):
        if(g_eeGeneral.currModel != mstate2.m_posVert)
        {
					ppupdata->PopupActive = 2 ;
          killEvents(event);
					ppupdata->PopupIdx = 0 ;
        }
				else
				{
		      if(event==EVT_KEY_FIRST(KEY_LEFT))  chainMenu(menuProcModelIndex);//{killEvents(event);popMenu(true);}
        	if(event==EVT_KEY_FIRST(KEY_RIGHT)) { chainMenu(menuProcModelIndex); }
				}
	    break;
    	case  EVT_KEY_FIRST(KEY_MENU) :
			case  EVT_KEY_FIRST(BTN_RE) :
  		  s_editMode = 0 ;
				if(sel_editMode)
				{
  		    sel_editMode = false ;
				}
				else
				{
					ppupdata->PopupActive = 1 ;
				 	killEvents(event) ;
					ppupdata->PopupIdx = 0 ;
				}
    	break;
    	case  EVT_KEY_LONG(KEY_EXIT):  // make sure exit long exits to main
        popMenu(true);
  	  break;

	    case EVT_ENTRY:
        sel_editMode = false;
				ppupdata->PopupActive = 0 ;
        mstate2.m_posVert = g_eeGeneral.currModel;
        eeCheck(true); //force writing of current model data before this is changed
	    break;
    }
	}
  if(sel_editMode && subOld!=sub)
	{
		EFile::swap(FILE_MODEL(subOld),FILE_MODEL(sub));

		if ( sub == g_eeGeneral.currModel )
		{
			g_eeGeneral.currModel = subOld ;
  	  STORE_GENERALVARS ;     //eeWriteGeneral();
		}
		else if ( subOld == g_eeGeneral.currModel )
		{
			g_eeGeneral.currModel = sub ;
  	  STORE_GENERALVARS ;     //eeWriteGeneral();
		}
  }
	asm("") ;	// Prevents compiler from creating 2 sets of register pops
}


const prog_char APM menuWhenDone[] = STR_MENU_DONE ;


void putc_0_1( uint8_t x, uint8_t y, uint8_t value )
{
  lcd_putcAtt( x, y, value+'0', value ? INVERS : 0 ) ;
}

extern uint8_t FrskyTelemetryType ;

struct t_timeGlobals
{
	uint16_t s_timeCumTot ;		// Total tx on time (secs)
	uint16_t s_timeCumAbs ;  //laufzeit in 1/16 sec
	uint16_t s_time ;
	uint8_t s_cnt ;
} ;

struct t_timeGlobals TimeGlobals ;

#define TMR_OFF     0
#define TMR_RUNNING 1
#define TMR_BEEPING 2
#define TMR_STOPPED 3

// 1. Timer=00:00 and count up - Should just announce whole positive minutes
// 2. Timer=00:00 and count down - Is not useful.
// 3. Timer=value and count up - announcing whole minutes left to the value and the last 30 seconds before reaching the value.
// 4. Timer=value and count down - announcing whole minutes left, and the last 30 seconds countdown.
//uint16_t Tval ;
static void timerBeeps()
{
	uint8_t i ;
	i = 0 ;
	struct t_timer *tptr = &s_timer[i] ;
	FORCE_INDIRECT(tptr) ;
	uint8_t direction = TimerConfig[i].tmrDir ;
	uint16_t timerValue = TimerConfig[i].tmrVal ;
  if(tptr->last_tmr != tptr->s_timerVal)  //beep only if seconds advance
	{
	  tptr->last_tmr = tptr->s_timerVal;
		if ( tptr->sw_toggled )
    {
			int16_t tval = tptr->s_timerVal ;

			if ( direction )
			{
				tval = timerValue-tval ;
			}


#ifdef GLOBAL_COUNTDOWN
      if(g_eeGeneral.preBeep && timerValue) // beep when 30, 20, 10, 5,4,3,2,1 seconds remaining
#else
      if(g_model.timer1Cdown && timerValue) // beep when 30, 20, 10, 5,4,3,2,1 seconds remaining
#endif
      {
				uint8_t flasht = 0 ;
        if(tval==30) {audioVoiceDefevent(AU_TIMER_30, V_30SECS);flasht = 1;}
        if(tval==20) {audioVoiceDefevent(AU_TIMER_20, V_20SECS);flasht = 1;}
        if(tval==10) {audioVoiceDefevent(AU_TIMER_10, V_10SECS);flasht = 1;}
        if(tval<= 5) { flasht = 1;	if(tval >= 0) {audioVoiceDefevent(AU_TIMER_LT3, tval) ;} /*else audioDefevent(AU_TIMER_LT3);*/}

        if(g_eeGeneral.flashBeep && flasht )
					g_LightOffCounter = FLASH_DURATION;
      }

			if ( tval < 0 )
			{
				if ( direction )
				{
					tval = tptr->s_timerVal ;
				}
			}

			div_t mins ;
			mins = div( tval, 60 ) ;

#ifdef GLOBAL_COUNTDOWN
      if(g_eeGeneral.minuteBeep && ((mins.rem)==0)) //short beep every minute
#else
      if(ptConfig->tmrMbeep && ((mins.rem)==0)) //short beep every minute
#endif
      {
				if ( g_eeGeneral.speakerMode & 2 )
				{
					if ( mins.quot ) { voiceMinutes( mins.quot ) ;}
				}
				else
				{
          audioDefevent(AU_WARNING1);
				}
        if(g_eeGeneral.flashBeep)
				 g_LightOffCounter = FLASH_DURATION;
      }

			if ( ( tptr->s_timerVal < 0 ) && ( tptr->s_timerVal >= -MAX_ALERT_TIME) )
	    {
  	    audioDefevent(AU_TIMER_LT3);
    	  if(g_eeGeneral.flashBeep)
					g_LightOffCounter = FLASH_DURATION;
	    }
		}
  }
}

void timer()
{
	uint8_t tma ;
	int8_t tmb ;
	uint8_t timer ;
	uint8_t val ;
  int16_t v ;

	struct t_timeGlobals *tgptr = &TimeGlobals ;
	FORCE_INDIRECT(tgptr) ;
  tgptr->s_cnt++ ;

	for( timer = 0 ; timer < 2 ; timer += 1 )
	{
		struct t_timer *ptimer = &s_timer[timer] ;
		FORCE_INDIRECT(ptimer) ;

		uint8_t resetting = 0 ;
		if ( timer == 0 )
		{
			tmb = g_model.timer1RstSw ;
		}
		else
		{
			tmb = g_model.timer2RstSw ;
		}
		if ( tmb )
		{
    	if(tmb>(TOGGLE_INDEX))	 // toggeled switch
			{
				uint8_t swPos = getSwitch00( tmb-(TOGGLE_INDEX)) ;
				if ( swPos != ptimer->lastResetSwPos )
				{
					ptimer->lastResetSwPos = swPos ;
					if ( swPos )	// Now on
					{
						resetting = 1 ;
					}
				}
			}
			else
			{
				if ( getSwitch00( tmb) )
				{
					resetting = 1 ;
				}
			}
		}
		if ( resetting )
		{
			if ( timer == 0 )
			{
				resetTimer1() ;
			}
			else
			{
				resetTimer2() ;
			}
		}
		TimerMode *ptConfig = &TimerConfig[timer] ;

		tma = ptConfig->tmrModeA ;
    tmb = ptConfig->tmrModeB ;

    //value for time described in g_model.tmrMode
    //OFFABSRUsRU%ELsEL%THsTH%ALsAL%P1P1%P2P2%P3P3%
    v = 0 ;
    if(( tma > 1 ) && ( tma < TMR_VAROFS ) )
		{
 			v = calibratedStick[3-1] ;
    }
		if ( ThrottleStickyOn )
		{
			v = -RESX ;
		}
   	if(tma>=TMR_VAROFS) // Cxx%
		{
			v = g_chans512[tma-TMR_VAROFS] ;
		}
		val = ( v + RESX ) / (RESX/16) ;

		if ( tma != TMRMODE_NONE )		// Timer is not off
		{ // We have a triggerA so timer is running
			if ( tmb > (TOGGLE_INDEX) )
			{
	    	if(!( ptimer->sw_toggled | ptimer->s_sum | tgptr->s_cnt | tgptr->s_time | ptimer->lastSwPos)) ptimer->lastSwPos = 0 ;  // if initializing then init the lastSwPos
  	  	uint8_t swPos = getSwitch00( tmb-(TOGGLE_INDEX) ) ;
	    	if(swPos && !ptimer->lastSwPos)
				{
					ptimer->sw_toggled = !ptimer->sw_toggled ;  //if switch is flipped first time -> change counter state
				}
    		ptimer->lastSwPos = swPos;
			}
			else
			{
				if ( tmb )
				{
  	  		ptimer->sw_toggled = getSwitch00( tmb ) ; //normal switch
				}
				else
				{
					ptimer->sw_toggled = 1 ;	// No trigger B so use as active
				}
			}
		}

		if ( ( ptimer->sw_toggled == 0 ) || resetting )
		{
			val = 0 ;
		}

		ptimer->s_sum += val ;
		if ((uint16_t)(get_tmr10ms() - tgptr->s_time) < 100)	continue;

		div_t qr;
		qr = div(ptimer->s_sum, tgptr->s_cnt);
		val = qr.quot ;
		ptimer->s_sum = qr.rem ;

		if ( timer == 0 )
		{
    	tgptr->s_timeCumTot += 1;
	    tgptr->s_timeCumAbs += 1;
		}
		else
		{
	    tgptr->s_cnt   = 0;    // ready for next 100mS
			tgptr->s_time += 100;  // 100*10mS passed
		}

    if(val) ptimer->s_timeCumThr += 1 ;
		if ( !resetting )
		{
    	if(ptimer->sw_toggled) ptimer->s_timeCumSw += 1;
		}
    ptimer->s_timeCum16ThrP += val >> 1 ; // val/2 ;

		uint16_t tv ;
    tv = ptimer->s_timerVal = TimerConfig[timer].tmrVal ;
    if(tma == TMRMODE_NONE)
		{
		}
    else
		{
			uint16_t subtrahend ;
			if(tma == TMRMODE_ABS)
			{
				if ( tmb == 0 ) subtrahend = tgptr->s_timeCumAbs ;
    		else subtrahend = ptimer->s_timeCumSw ; //switch
			}
    	else if(tma<TMR_VAROFS-1) subtrahend = ptimer->s_timeCumThr ;// stick
		  else subtrahend = ptimer->s_timeCum16ThrP/16 ; // stick% or Cx%
			ptimer->s_timerVal -= subtrahend ;
		}

    if(TimerConfig[timer].tmrDir) ptimer->s_timerVal = tv-ptimer->s_timerVal ; //if counting backwards - display backwards
	}
	timerBeeps() ;

}


#if THROTTLE_TRACE
#define MAXTRACE 120
uint8_t s_traceBuf[MAXTRACE];
uint8_t s_traceWr;
uint8_t s_traceCnt;
#endif // THROTTLE_TRACE
void trace()   // called in perOut - once envery 0.01sec
{

#if THROTTLE_TRACE
    uint16_t val = RESX + calibratedStick[3-1]; //Get throttle channel value
    val /= (RESX/16); //calibrate it
#endif // THROTTLE_TRACE

#if THROTTLE_TRACE
	struct t_trace
	{
    uint16_t s_cnt ;
    uint16_t s_sum ;
    uint16_t s_time ;
	} ;
		static struct t_trace traceVal ;
		struct t_trace *tracePtr = &traceVal ;
		FORCE_INDIRECT(tracePtr) ;

    tracePtr->s_cnt++;
    tracePtr->s_sum+=val;
#else
    static uint16_t s_time;
#endif // THROTTLE_TRACE

		uint16_t t10ms ;
		t10ms = get_tmr10ms() ;

#if THROTTLE_TRACE
    if(( t10ms-tracePtr->s_time)<1000) //10 sec
#else
    if(( t10ms-s_time)<1000) //10 sec
#endif // THROTTLE_TRACE

				return;

#if THROTTLE_TRACE
    tracePtr->s_time= t10ms ;
#else
    s_time= t10ms ;
#endif // THROTTLE_TRACE


#if THROTTLE_TRACE
    val   = tracePtr->s_sum/tracePtr->s_cnt;
    tracePtr->s_sum = 0;
    tracePtr->s_cnt = 0;

    if ( s_traceCnt <= MAXTRACE )
		{
			s_traceCnt++;
		}
    s_traceBuf[s_traceWr++] = val;
    if(s_traceWr>=MAXTRACE) s_traceWr=0;
#endif // THROTTLE_TRACE
}


extern unsigned int stack_free() ;

struct t_latency g_latency = { 0 } ;


void putsTimeNoAtt( uint8_t x,uint8_t y,int16_t tme )
{
	putsTime( x, y, tme, 0, 0 ) ;
}

void menuProcStatistic(uint8_t event)
{
  TITLE(STR_STAT) ;
  switch(event)
  {
		case EVT_KEY_LONG(BTN_RE) :
    case EVT_KEY_FIRST(KEY_EXIT):
      popMenu(false) ;
    break ;
  }

		struct t_latency *ptrLat = &g_latency ;
		FORCE_INDIRECT(ptrLat) ;
    if ( event == EVT_KEY_FIRST(KEY_MENU) )
		{
      ptrLat->g_timeMain    = 0;
      audioDefevent(AU_MENUS);
    }

    lcd_puts_Pleft( 4*FH, PSTR("tmain\022ms" ) ) ;
	  lcd_outdezAtt( PARAM_OFS, 4*FH, (ptrLat->g_timeMain*25)/4 ,PREC2 ) ;


		lcd_puts_Pleft( 7*FH, PSTR("\003[ OK ] to refresh"));

		struct t_timer *tptr = &s_timer[0] ;
		FORCE_INDIRECT(tptr) ;


    lcd_puts_Pleft( FH*0, PSTR("\021TOT\037\001TME\021TSW\037\001STK\021ST%"));

    putsTimeNoAtt(    7*FW, FH*1, TimeGlobals.s_timeCumAbs );
    putsTimeNoAtt(   13*FW, FH*1, tptr->s_timeCumSw );

    putsTimeNoAtt(    7*FW, FH*2, tptr->s_timeCumThr );
    putsTimeNoAtt(   13*FW, FH*2, tptr->s_timeCum16ThrP/16 );

    putsTimeNoAtt(   13*FW, FH*0, TimeGlobals.s_timeCumTot );

// Temp stack trace display


#if THROTTLE_TRACE
    uint8_t x=5;
    uint8_t y=60;
    lcd_hline(x-3,y,120+3+3);
    lcd_vline(x,y-32,32+3);

    for(uint8_t i=0; i<120; i+=6)
    {
        lcd_vline(x+i+6,y-1,3);
    }
		uint8_t traceWr = s_traceWr ;
    uint8_t traceRd = s_traceCnt>MAXTRACE ? traceWr : 0;
    for(uint8_t i=1; i<=MAXTRACE; i++)
    {
        lcd_vline(x+i,y-s_traceBuf[traceRd],s_traceBuf[traceRd]);
        traceRd++;
        if(traceRd>=MAXTRACE) traceRd=0;
        if(traceRd==traceWr) break;
    }
#endif
	asm("") ;

}


void resetTimern( uint8_t timer )
{
	uint16_t time ;
	uint8_t dir ;
	TimerMode *ptConfig = &TimerConfig[timer] ;
	FORCE_INDIRECT(ptConfig) ;

	time = ptConfig->tmrVal ;
	dir = ptConfig->tmrDir ;

  struct t_timer *tptr = &s_timer[timer] ;
	FORCE_INDIRECT(tptr) ;

	tptr->last_tmr = time ;
	tptr->s_timerVal = ( dir ) ? 0 : time ;
  tptr->s_timeCumThr=0;
  tptr->s_timeCumSw=0;
  tptr->s_timeCum16ThrP=0;
	tptr->s_sum = 0 ;


}


NOINLINE void resetTimer1(void)
{
  TimeGlobals.s_timeCumAbs=0 ;
	resetTimern( 0 ) ;
}

void resetTimer2()
{
	resetTimern( 1 ) ;
}


static int8_t inputs_subview = 0 ;

static
void putsDblSizeName( uint8_t y )
{
	for(uint8_t i=0;i<sizeof(g_model.name);i++)
		lcd_putcAtt(FW*2+i*2*FW-i-2, y, g_model.name[i],DBLSIZE);
}
void displayOneSwitch3Pos( uint8_t x, uint8_t y, uint8_t index ){

	if(getSwitch00(index))
         lcd_putc(x, y,'\200');
	else if(getSwitch00(index + 1))
         lcd_putc(x, y,'-');
	else if(getSwitch00(index + 2))
         lcd_putc(x, y,'\201');
}


void displayOneSwitch( uint8_t x, uint8_t y, uint8_t index )
{
	lcd_putsAttIdx( x, y, Str_Switches, index, /*getSwitch00(index+1) ? INVERS :*/ 0) ;
	lcd_putc(x+ 2*FW + 2, y, getSwitch00(index+1) ? '\201' : '\200' ) ;

}


void switchDisplay( uint8_t j, uint8_t a )
{

	displayOneSwitch( j, 4*FH, a ) ;
	displayOneSwitch( j, 5*FH, a+1 ) ;
	displayOneSwitch( j, 6*FH, a+2 ) ;

}

void dispSignalQality( uint8_t y){

      lcd_puts_P(2+FW, y, PSTR("Err"));
      putsTelemValue(10 * FW, y, (int16_t)AFHDS2A_tel_data[FST_IDX_ERR], FST_IDX_ERR, NO_UNIT);
      lcd_puts_P(FW * 11, y, PSTR("SNR"));
      putsTelemValue( FW * 20, y, (int16_t)AFHDS2A_tel_data[FST_IDX_SNR], FST_IDX_SNR, NO_UNIT);
}


#define TEL_PAGE_1	0x10
#define TEL_PAGE_2	0x20
#define TEL_PAGE_3	0x30
#define TEL_PAGE_4	0x40

#define TRIMBAR_WIDTH 8
void menuProc0(uint8_t event)
{
    static uint8_t trimSwLock;
    uint8_t view = g_eeGeneral.view & 0x0f ;
    uint8_t tview = g_eeGeneral.view & 0x70 ;

		StickScrollAllowed = 0 ;

 if ( ! PopupData.PopupActive )
 {
    switch(event)
    {
    case EVT_KEY_BREAK(KEY_MENU):
		case EVT_KEY_BREAK(BTN_RE):
                  if ( view == e_telemetry )
                  {
                          PopupData.PopupActive = 1 ;
                          PopupData.PopupIdx = 0 ;
                           killEvents(event) ;
                          Tevent = 0 ;
                  }

        break;

		case  EVT_KEY_LONG(BTN_RE):// go to last menu
    case  EVT_KEY_LONG(KEY_MENU):// go to last menu

					PopupData.PopupActive = 2 ;
					PopupData.PopupIdx = 0 ;
      		killEvents(event) ;
					event = 0 ;
					Tevent = 0 ;

        break;
    case EVT_KEY_LONG(KEY_RIGHT):
        scroll_disabled = 1;
        pushMenu(menuProcModelSelect);
        killEvents(event);
        break;
    case EVT_KEY_BREAK(KEY_RIGHT):


        if(view == e_inputs1)
        {
                int8_t x ;
                x = inputs_subview ;

                if ( ++x > 2 ) x = 0 ;
                inputs_subview = x ;
        }


        if(view == e_telemetry)
	{
          g_eeGeneral.view = e_telemetry | ( ( tview + 0x10) & 0x30 ) ;

          audioDefevent(AU_MENUS);
        }
        break;
    case EVT_KEY_BREAK(KEY_LEFT):


        if(view == e_inputs1)
        {
                int8_t x ;
                x = inputs_subview ;

                if ( --x < 0 ) x = 2 ;

                inputs_subview = x ;
        }


        if(view == e_telemetry){
          g_eeGeneral.view = e_telemetry | ( ( tview - 0x10) & 0x30 );
          audioDefevent(AU_MENUS);
        }
        break;
    case EVT_KEY_LONG(KEY_LEFT):
        scroll_disabled = 1;
				pushMenu(menuProcIndex);
        killEvents(event);
        break;
    case EVT_KEY_BREAK(KEY_UP):
				view += 1 ;
        if( view>=MAX_VIEWS) view = 0 ;
        g_eeGeneral.view = view | tview ;
        eeDirty(EE_GENERAL | 0xA0) ;
        audioDefevent(AU_KEYPAD_UP);
        break;
    case EVT_KEY_BREAK(KEY_DOWN):
        if(view>0)
            view = view - 1;
        else
            view = MAX_VIEWS-1;
        g_eeGeneral.view = view | tview ;
        eeDirty(EE_GENERAL | 0xA0) ;
        audioDefevent(AU_KEYPAD_DOWN);
        break;
    case EVT_KEY_LONG(KEY_UP):
        pushMenu(menuProcStatistic);
        killEvents(event);
				return ;
        break;
    case EVT_KEY_LONG(KEY_DOWN):

				view = e_telemetry ;
				g_eeGeneral.view = view | tview ;
        audioDefevent(AU_MENUS);
        killEvents(event);
				return ;
    case EVT_KEY_FIRST(KEY_EXIT):
        if (view == e_telemetry) {
            audioDefevent(AU_MENUS);
        }
        break;
    case EVT_KEY_LONG(KEY_EXIT):
        resetTimer1();
        resetTimer2();

        audioDefevent(AU_MENUS);
        break;
    case EVT_ENTRY:
        killEvents(KEY_EXIT);
        killEvents(KEY_UP);
        killEvents(KEY_DOWN);
        trimSwLock = true;
				inputs_subview = 0 ;
        break;
    }
 } // !PopupActive
		{
			uint8_t tsw ;
			tsw = getSwitch00(g_model.trimSw) ;
	    if( tsw && !trimSwLock) setStickCenter();
  	  trimSwLock = tsw ;
		}

    if (view != e_telemetry)

    {

        uint8_t x=FW*2;
        uint8_t att = (g_vbat100mV < g_eeGeneral.vBatWarn ? BLINK : 0) | DBLSIZE;
	uint8_t i ;

        putsDblSizeName( 0 ) ;

        putsVBat( 6*FW+1, 2*FH, att|NO_UNIT);
        lcd_putc( 6*FW+2, 3*FH, 'V');


	displayTimer( x+14*FW-3, FH*2, 0, DBLSIZE ) ;
        putsTmrMode(x+7*FW-FW/2,FH*3,0,0);

        i = getFlightPhase() ;
        if ( i )
        {

                dispFlightModename( 6*FW+2, 2*FH, i ) ;

                lcd_rect( 6*FW+1, 2*FH-1, 6*FW+2, 9 ) ;
        }
        else
        {
        	lcd_putsAttIdx( 6*FW+2,     2*FH,PSTR(STR_TRIM_OPTS),g_model.trimInc, 0);

        }

   }
        //trim sliders
        for(uint8_t i=0; i<4; i++)
        {
#define TL 27
            //                        LH LV RV RH
const static prog_uint8_t APM xt[4] = {128*1/4+2, 4, 128-4, 128*3/4-2};
            uint8_t xm,ym;
						xm = modeFixValue( i ) ;
            xm = pgm_read_byte(xt+xm-1) ;
			  	  int16_t valt = getTrimValue( CurrentPhase, i ) ;

            int8_t val = valt/4 ;
              if ( val == 0 )
              {
                      if ( valt > 0 )
                      {
                              val += 1 ;
                      }
                      if ( valt < 0 )
                      {
                              val -= 1 ;
                      }
              }
              if ( val > TL+1 )
              {
                      val = TL+1 ;
              }
              else
              {
                      if ( val < -(TL+1) )
                      {
                              val = -(TL+1) ;
                      }
              }
            if( (i == 1) || ( i == 2 ))
            {
              ym=31;
              lcd_vline(xm,   ym-TL, TL*2);

              if((i == 1) || !(g_model.thrTrim))
							{
                lcd_vline(xm-1, ym-1,  3);
                lcd_vline(xm+1, ym-1,  3);
              }
              ym -= val;
            }
            else
            {
              ym=59;
              lcd_hline(xm-TL,ym,    TL*2);
              lcd_hline(xm-1, ym-1,  3);
              lcd_hline(xm-1, ym+1,  3);
              xm += val;
            }

						lcd_rect_xor( xm-3, ym-3, 7, 7 ) ;

        }


     if (view == e_telemetry)
     {
        lcd_putsnAtt(TRIMBAR_WIDTH, 0, g_model.name, sizeof(g_model.name), BSS|INVERS);
        uint8_t att = (g_vbat100mV < g_eeGeneral.vBatWarn ? BLINK : 0);
        putsVBat(14*FW,0,att);
					displayTimer( 18*FW+3, 0, 0, 0 ) ;
    }


    if (view == e_telemetry)
    {
      {
        uint8_t x0; //, blink;
          dispSignalQality(6 * FH);


        {
              lcd_vline( 63, 8, 48 ) ;
              for (uint8_t i=0; i<CUSTOM_DISPLAY_TEL_NUMBER; i++)
              {
                      if ( g_model.CustomDisplayIndex[i] )
                      {
                             putsTelemetryChannel(((i&1)?58:0)+TRIMBAR_WIDTH, FH+8 +(i/2)*FH,
                              g_model.CustomDisplayIndex[i]-1,
                             get_telemetry_value(g_model.CustomDisplayIndex[i]-1),
                             0, TELEM_LABEL|TELEM_UNIT|TELEM_UNIT_LEFT|TELEM_VALUE_RIGHT ) ;
                      }
              }

        }
      }

    }

    if(view == e_inputs1)
    {
         doMainScreenGrphics();


        uint8_t a = inputs_subview;
        uint8_t b = a + 1;
        if (a != 0)
          a = a * 6 + 3; // 0, 9, 15
        switchDisplay(2 * FW - 2, a);
        if (a == 0) {
          lcd_puts_P(17 * FW - 2, FH * 5, "sC");
          displayOneSwitch3Pos(17 * FW + 2 * FW, FH * 5, 7);
          lcd_puts_P(17 * FW - 2, FH * 6, "ID");
          displayOneSwitch3Pos(17 * FW + 2 * FW, FH * 6, 4);
        } else {
          b *= 6; // 6, 12, 18
          switchDisplay(17 * FW - 2, b);
        }
    }


	if ( PopupData.PopupActive )
	{
		uint8_t popaction ;
		uint8_t popidx = 0 ;
    if(PopupData.PopupActive == 1)
		{
			popidx = 5 ;
			popaction = doPopup( PSTR("Zero Alt \0Zero GAlt"), 0x03, 14 ) ;

		}
		else
		{
			popaction = doPopup( Str_Main_Popup, 0x1F, 14 ) ;
		}

		MenuControl.UseLastSubmenuIndex = 0 ;
  	if ( popaction == POPUP_SELECT )
		{
			popidx += PopupData.PopupSel ;
			if ( popidx == 0 )	// Model Select
			{
        pushMenu(menuProcModelSelect) ;
			}
			else if( popidx == 1 )	// Edit Model
			{
        scroll_disabled = 1;
				RotaryState = ROTARY_MENU_UD ;
	  	  pushMenu(menuProcModelIndex) ;
			}
			else if( popidx == 2 )	// Last Menu
			{
				MenuControl.UseLastSubmenuIndex = 1 ;
        pushMenu(lastPopMenu());
			}
			else if ( popidx == 3 )	// Radio Setup
			{
        pushMenu(menuProcIndex) ;
			}
			else if( popidx == 4 )	// Statistics
			{
	  	  pushMenu(menuProcStatistic) ;
			}
			else if( popidx == 5 )	// Zero Alt.
			{
        AltOffset = -AFHDS2A_tel_data[FST_IDX_ALT];
			}
			else if( popidx == 6 )	// Zero GAlt
			{
        GAltOffset = -AFHDS2A_tel_data[FST_IDX_GPS_ALT];
			}
		}
	}


	asm("") ;
}

static inline uint16_t isqrt32(uint32_t n)
{
    uint16_t c = 0x8000;
    uint16_t g = 0x8000;

    for(;;) {
        if((uint32_t)g*g > n)
            g ^= c;
        c >>= 1;
        if(c == 0)
            return g;
        g |= c;
    }
}

int16_t intpol(int16_t x, uint8_t idx) // -100, -75, -50, -25, 0 ,25 ,50, 75, 100
{
#define D9 (RESX * 2 / 8)
#define D5 (RESX * 2 / 4)
  uint8_t cv9 = idx >= MAX_CURVE5;
  int16_t erg;
	int8_t *crv = curveAddress( idx ) ;


  x+=RESXu;
  if(x < 0)
	{
    erg = (int16_t)crv[0] * (RESX/4);
  }
	else if(x >= (RESX*2))
	{
    erg = (int16_t)crv[(cv9 ? 8 : 4)] * (RESX/4);
  }
	else
	{
    int16_t a,dx;
    if(cv9)
		{
      a   = (uint16_t)x / D9;
      dx  =((uint16_t)x % D9) * 2;
    }
		else
		{
      a   = (uint16_t)x / D5;
      dx  = (uint16_t)x % D5;
    }
    erg  = (int16_t)crv[a]*((D5-dx)/2) + (int16_t)crv[a+1]*(dx/2);
  }
  return erg / 25; // 100*D5/RESX;
}

int16_t calcExpo( uint8_t channel, int16_t value )
{
  uint8_t expoDrOn = get_dr_state(channel);
  uint8_t stkDir = value > 0 ? DR_RIGHT : DR_LEFT;
  if(IS_EXPO_THROTTLE(channel)) stkDir = DR_RIGHT ;
  ExpoData *peData = &g_model.expoData[channel] ;

	int8_t expoval = peData->expo[expoDrOn][DR_EXPO][stkDir] ;

  if(IS_EXPO_THROTTLE(channel)){
      value  = 2*expo((value+RESX)/2,REG100_100(expoval));
			stkDir = DR_RIGHT;
  }
  else
      value  = expo(value,REG100_100(expoval));

  int32_t x = (int32_t)value * (REG(peData->expo[expoDrOn][DR_WEIGHT][stkDir]+100, 0, 100))/100;
  value = (int16_t)x;
  if (IS_EXPO_THROTTLE(channel)) value -= RESX;
	return value ;
}


uint8_t  bpanaCenter = 0;
struct t_output
{
	uint16_t sDelay[MAX_MIXERS] ;
	int32_t  act   [MAX_MIXERS] ;
	uint8_t  swOn  [MAX_MIXERS] ;

	int16_t  anas [NUM_XCHNRAW-NUM_CHNOUT-NUM_PPM] ;
	int16_t rawSticks[4] ;
} Output ;

uint8_t	CurrentPhase = 0 ;
struct t_fade
{
	uint8_t  fadePhases ;
	uint16_t fadeRate ;
	uint16_t fadeWeight ;
	uint16_t fadeScale[MAX_MODES+1] ;
	int32_t  fade[NUM_CHNOUT];
} Fade ;

void perOutPhase( int16_t *chanOut, uint8_t att )
{
	static uint8_t lastPhase ;
	uint8_t thisPhase ;
	struct t_fade *pFade ;
	pFade = &Fade ;
	FORCE_INDIRECT( pFade ) ;

	thisPhase = getFlightPhase() ;
	if ( thisPhase != lastPhase )
	{
		uint8_t time1 = 0 ;
		uint8_t time2 ;

		if ( lastPhase )
		{
      time1 = g_model.phaseData[(uint8_t)(lastPhase-1)].fadeOut ;
		}
		if ( thisPhase )
		{
      time2= g_model.phaseData[(uint8_t)(thisPhase-1)].fadeIn ;
			if ( time2 > time1 )
			{
        time1 = time2 ;
			}
		}
		if ( time1 )
		{
			pFade->fadeRate = (25600 / 50) / time1 ;
			pFade->fadePhases |= ( 1 << lastPhase ) | ( 1 << thisPhase ) ;
		}
		lastPhase = thisPhase ;
	}
	att |= FADE_FIRST ;
	if ( pFade->fadePhases )
	{
		pFade->fadeWeight = 0 ;
		uint8_t fadeMask = 1 ;
    for (uint8_t p=0; p<MAX_MODES+1; p++)
		{
			if ( pFade->fadePhases & fadeMask )
			{
				if ( p != thisPhase )
				{
					CurrentPhase = p ;
					pFade->fadeWeight += pFade->fadeScale[p] ;
					perOut( chanOut, att ) ;
					att &= ~FADE_FIRST ;
				}
			}
			fadeMask <<= 1 ;
		}
	}
	else
	{
		pFade->fadeScale[thisPhase] = 25600 ;
	}
	pFade->fadeWeight += pFade->fadeScale[thisPhase] ;
	CurrentPhase = thisPhase ;
	perOut( chanOut, att | FADE_LAST ) ;

	if ( pFade->fadePhases && tick10ms )
	{
		uint8_t fadeMask = 1 ;
    for (uint8_t p=0; p<MAX_MODES+1; p+=1)
		{
			uint16_t l_fadeScale = pFade->fadeScale[p] ;

			if ( pFade->fadePhases & fadeMask )
			{
				uint16_t x = pFade->fadeRate * tick10ms ;
				if ( p != thisPhase )
				{
          if ( l_fadeScale > x )
					{
						l_fadeScale -= x ;
					}
					else
					{
						l_fadeScale = 0 ;
						pFade->fadePhases &= ~fadeMask ;
					}
				}
				else
				{
          if ( 25600 - l_fadeScale > x )
					{
						l_fadeScale += x ;
					}
					else
					{
						l_fadeScale = 25600 ;
						pFade->fadePhases &= ~fadeMask ;
					}
				}
			}
			else
			{
				l_fadeScale = 0 ;
			}
			pFade->fadeScale[p] = l_fadeScale ;
			fadeMask <<= 1 ;
		}
	}
}

int16_t scaleAnalog( int16_t v, uint8_t channel )
{
	v -= g_eeGeneral.calibMid[channel];
	v  =  v * (int32_t)RESX /  (max((int16_t)100,(v>0 ?
                                                    g_eeGeneral.calibSpanPos[channel] :
                                                    g_eeGeneral.calibSpanNeg[channel])));
	if(v <= -RESX) v = -RESX;
	if(v >=  RESX) v =  RESX;
	if ( throttleReversed() )
	{
		if ( channel == THR_STICK )
		{
			v = -v ;
		}
	}
	return v ;
}

void perOut(int16_t *chanOut, uint8_t att)
{
  int16_t  trimA[4];
  uint8_t  anaCenter = 0;
	int16_t trainerThrottleValue = 0 ;
	uint8_t trainerThrottleValid = 0 ;


  uint8_t ele_stick, ail_stick ;
  ele_stick = 1 ; //ELE_STICK ;
  ail_stick = 3 ; //AIL_STICK ;

	uint8_t stickIndex = g_eeGeneral.stickMode*4 ;
	for(uint8_t i=0;i<7;i++)
	{        // calc Sticks

      //Normalization  [0..2048] ->   [-1024..1024]

    int16_t v = anaIn(i);
		v = scaleAnalog( v, i ) ;

		uint8_t index = i ;
		if ( i < 4 )
		{
      phyStick[i] = v >> 4 ;
			index = pgm_read_byte(stickScramble+stickIndex+i) ;
		}
    calibratedStick[index] = v; //for show in expo
		// Filter beep centre
		{
			int8_t t = v/16 ;
			uint8_t mask = 1 << index ;
			if ( t < 0 )
			{
				t = -t ;		//abs(t)
			}
			if ( t <= 1 )
			{
        anaCenter |= ( t == 0 ) ? mask : bpanaCenter & mask ;
			}
		}

    if(i<4) { //only do this for sticks
        //===========Trainer mode================
        if (!(att&NO_TRAINER) && g_model.traineron) {
            TrainerMix* td = &g_eeGeneral.trainer.mix[index];
            if (td->mode && getSwitch00(td->swtch))
						{
							if ( ppmInAvailable )
							{
                uint8_t chStud = td->srcChn ;
                int16_t vStud  = (g_ppmIns[chStud]- g_eeGeneral.trainer.calib[chStud]) /* *2 */ ;
                vStud /= 2 ;		// Only 2, because no *2 above
                vStud *= td->studWeight ;
                vStud /= 31 ;
                vStud *= 4 ;
                switch ((uint8_t)td->mode) {
                case 1: v += vStud;   break; // add-mode
                case 2: v  = vStud;   break; // subst-mode
                }
								if ( index == 2 )
								{
									trainerThrottleValue = vStud ;
									trainerThrottleValid = 1 ;
								}
							}
            }
        }


				if ( att & FADE_FIRST )
				{
    		  Output.rawSticks[index] = v; //set values for mixer
				}
				v = calcExpo( index, v ) ;

        trimA[i] = getTrimValue( CurrentPhase, i )*2 ;
    }
		if ( att & FADE_FIRST )
		{
      Output.anas[index] = v; //set values for mixer
		}
    if(att&NO_INPUT)
		{ //zero input for setStickCenter()
   		if ( i < 4 )
			{
 				if(!IS_THROTTLE(index))
				{
					if ( ( v > (RESX/100 ) ) || ( v < -(RESX/100) ) )
					{
				    Output.anas[index] = 0; //set values for mixer
					}
		      trimA[index] = 0;
 				}
   		}
    }

  }
  if(g_model.thrTrim)
	{
		int8_t ttrim ;
		ttrim = getTrimValue( CurrentPhase, 2 ) ;
		if(throttleReversed())
		{
			ttrim = -ttrim ;
		}
    trimA[2] = ((int32_t)ttrim+125)*(RESX-Output.anas[2])/(RESX) ;
	}
	if ( att & FADE_FIRST )
	{

    //===========BEEP CENTER================
    anaCenter &= g_model.beepANACenter;
    if(((bpanaCenter ^ anaCenter) & anaCenter)) audioDefevent(AU_POT_STICK_MIDDLE);
    bpanaCenter = anaCenter;

		int16_t *panas = Output.anas ;
		FORCE_INDIRECT(panas) ;

    panas[MIX_MAX-1]  = RESX;     // MAX
    panas[MIX_FULL-1] = RESX;     // FULL


    //===========Swash Ring================
    if(g_model.swashRingValue)
    {


      uint32_t v = ((int32_t)panas[ele_stick]*panas[ele_stick] + (int32_t)panas[ail_stick]*panas[ail_stick]);
		  int16_t tmp = calc100toRESX(g_model.swashRingValue) ;
      uint32_t q ;
      q = (int32_t)tmp * tmp ;
      if(v>q)
      {
        uint16_t d = isqrt32(v);
        panas[ele_stick] = (int32_t)panas[ele_stick]*tmp/((int32_t)d) ;
        panas[ail_stick] = (int32_t)panas[ail_stick]*tmp/((int32_t)d) ;
      }
    }

#define REZ_SWASH_X(x)  ((x) - (x)/8 - (x)/128 - (x)/1024 )   //  1024*sin(60) ~= 887
#define REZ_SWASH_Y(x)  ((x))   //  1024 => 1024

    if(g_model.swashType)
    {
        int16_t vp = 0 ;
        int16_t vr = 0 ;

        if( !(att & NO_INPUT) )  //zero input for setStickCenter()
				{
          vp += panas[ele_stick] + trimA[ele_stick] ;
          vr += panas[ail_stick] + trimA[ail_stick] ;
				}

        int16_t vc = 0;

        if(g_model.swashCollectiveSource)
				{
					uint8_t k = g_model.swashCollectiveSource - 1 ;
					if ( k < CHOUT_BASE )
					{
            vc = panas[k] ;
					}
					else
					{
						vc = Ex_chans[k-CHOUT_BASE] ;
					}
				}

        if(g_model.swashInvertELE) vp = -vp;
        if(g_model.swashInvertAIL) vr = -vr;
        if(g_model.swashInvertCOL) vc = -vc;

				int16_t c1,c2,c3 ;
        switch (( uint8_t)g_model.swashType)
        {
        default:
        case (SWASH_TYPE_120):
            vp = REZ_SWASH_Y(vp);
            vr = REZ_SWASH_X(vr);
            c1 = vc - vp;
            c2 = vc + vp/2 + vr;
            c3 = vc + vp/2 - vr;
            break;
        case (SWASH_TYPE_120X):
            vp = REZ_SWASH_X(vp);
            vr = REZ_SWASH_Y(vr);
            c1 = vc - vr;
            c2 = vc + vr/2 + vp;
            c3 = vc + vr/2 - vp;
            break;
        case (SWASH_TYPE_140):
            vp = REZ_SWASH_Y(vp);
            vr = REZ_SWASH_Y(vr);
            c1 = vc - vp;
            c2 = vc + vp + vr;
            c3 = vc + vp - vr;
            break;
        case (SWASH_TYPE_90):
            vp = REZ_SWASH_Y(vp);
            vr = REZ_SWASH_Y(vr);
            c1 = vc - vp;
            c2 = vc + vr;
            c3 = vc - vr;
            break;
        }
				panas[MIX_CYC1-1] = c1 ;
				panas[MIX_CYC2-1] = c2 ;
				panas[MIX_CYC3-1] = c3 ;

    }

	}
//    memset(chans,0,sizeof(chans));        // All outputs to 0


#ifdef MIX_WARN
  uint8_t mixWarning = 0;
#endif
    //========== MIXER LOOP ===============

  uint8_t mixIndex = 0 ;
  MixData *md = &g_model.mixData[0];
	for(uint8_t chanIndex=0;chanIndex<NUM_CHNOUT;chanIndex++)
	{
    int32_t thisChan = 0 ;
		for( ; md->destCh-1 == chanIndex ; md += 1,	mixIndex += 1 ) // So continue increments
		{
				int8_t mixweight = REG100_100( md->weight) ;


        //Notice 0 = NC switch means not used -> always on line
        int16_t v  = 0;
        uint8_t swTog;
        uint8_t swon = Output.swOn[mixIndex] ;

				bool t_switch = getSwitch(md->swtch,1) ;
        if (md->swtch && (md->srcRaw > PPM_BASE) && (md->srcRaw <= PPM_BASE+NUM_PPM)	&& (ppmInAvailable == 0) )
				{
					// then treat switch as false ???
					t_switch = 0 ;
				}

        if ( t_switch )
				{
					if ( md->modeControl & ( 1 << CurrentPhase ) )
					{
						t_switch = 0 ;
					}
				}

        uint8_t k = md->srcRaw ;

#define DEL_MULT 256
        if(!t_switch)
				{ // switch on?  if no switch selected => on
            swTog = swon ;
        		Output.swOn[mixIndex] = swon = false ;	// In case we do a "continue" just below
            if (k == MIX_3POS+MAX_GVARS+1)	// "THIS"
						{
							int32_t temp = thisChan * ((int32_t)DEL_MULT * 256 / 100 ) ;
							Output.act[mixIndex] =  temp >> 8 ;
						}
            if(k!=MIX_MAX && k!=MIX_FULL) continue;// if not MAX or FULL - next loop
            if(md->mltpx==MLTPX_REP) continue; // if switch is off and REPLACE then off
            v = (k == MIX_FULL ? -RESX : 0); // switch is off and it is either MAX=0 or FULL=-512
        }
        else
				{
            swTog = !swon ;
            swon = true;
            k -= 1 ;

						if ( k < PPM_BASE )
						{
            	v = Output.anas[k]; //Switch is on. MAX=FULL=512 or value.
							if ( k < 4 )
							{
								if ( md->disableExpoDr )
								{
      		      	v = Output.rawSticks[k]; //Switch is on. MAX=FULL=512 or value.
								}
							}
						}
						else if ( k < CHOUT_BASE )
						{
							uint8_t i = k - PPM_BASE ;
							int16_t x ;
							x = g_ppmIns[i] ;
							if ( i < 4 ) x -= g_eeGeneral.trainer.calib[i] ;  //add ppm channels
							v = x * 2 ;
						}
						else if(k<CHOUT_BASE+NUM_CHNOUT)
						{
							if ( md->disableExpoDr )
							{
								v = g_chans512[k-CHOUT_BASE] ;
							}
							else
							{
								{
									v = Ex_chans[k-CHOUT_BASE] ;
								}
							}
						}
						else if ( k == MIX_3POS-1 )
						{
							uint8_t sw = md->sw23pos ;
							if ( sw )
							{
								sw += SW_ThrCt - KEY_MENU - 1 ;
								if ( sw >= SW_ID0 )
								{
									sw += SW_AileDR - SW_ID0 ;
								}
        				v = keyState((EnumKeys)sw) ? 1024 : -1024 ;
							}
							else
							{
        				v = keyState(SW_ID0) ? -1024 : (keyState(SW_ID1) ? 0 : 1024) ;
							}
						}
						else if ( k < MIX_3POS+MAX_GVARS )
						{
			        v = g_model.gvars[k-MIX_3POS].gvar * 8 ;
						}
						else if ( k == MIX_3POS+MAX_GVARS )	// "THIS"
						{
							v = thisChan / 100 ;
						}
            else
						{
							v = calc_scaler( k - (MIX_3POS+MAX_GVARS+1), 0, 0 ) ;
						}


#ifdef MIX_WARN
						if(md->mixWarn) mixWarning |= 1<<(md->mixWarn-1); // Mix warning
#endif
        }
        Output.swOn[mixIndex] = swon ;

        //========== INPUT OFFSET ===============
        if ( md->lateOffset == 0 )
        {
            if(md->sOffset) v += calc100toRESX( REG( md->sOffset, -125, 125 )	) ;
        }
        //========== DELAY and PAUSE ===============
        if (md->speedUp || md->speedDown || md->delayUp || md->delayDown)  // there are delay values
        {
					uint8_t timing = g_model.mixTime ? 20 : 100 ;
					int16_t my_delay = Output.sDelay[mixIndex] ;
					int32_t tact = Output.act[mixIndex] ;
#if DEL_MULT == 256
						int16_t diff = v-(tact>>8) ;
#else
            int16_t diff = v-tact/DEL_MULT;
#endif

						if ( ( diff > 10 ) || ( diff < -10 ) )
						{
							if ( my_delay == 0 )
							{
      				  if (md->delayUp || md->delayDown)  // there are delay values
								{
									swTog = 1 ;
								}
							}
						}
						else
						{
							my_delay = 0 ;
						}

            if(swTog) {
                //need to know which "v" will give "anas".
                //curves(v)*weight/100 -> anas
                // v * weight / 100 = anas => anas*100/weight = v
                if(md->mltpx==MLTPX_REP)
                {
                    tact = (int32_t)Ex_chans[md->destCh-1]*DEL_MULT * 100 ;
                    if(mixweight) tact /= mixweight ;
                }
#if DEL_MULT == 256
								diff = v-(tact>>8) ;
#else
            		diff = v-tact/DEL_MULT;
#endif
                if(diff) my_delay = (diff<0 ? md->delayUp :  md->delayDown) * timing ;
            }

            if(my_delay > 0)
						{ // perform delay
                if(tick10ms)
                {
                  my_delay-- ;
                }
                if (my_delay != 0)
                { // At end of delay, use new V and diff
#if DEL_MULT == 256
	                v = tact >> 8 ;	   // Stay in old position until delay over
#else
                  v = tact/DEL_MULT;   // Stay in old position until delay over
#endif
                  diff = 0;
                }
								else
								{
									my_delay = -1 ;
								}
            }

					Output.sDelay[mixIndex] = my_delay ;

            if(diff && (md->speedUp || md->speedDown))
						{
                if(tick10ms)
								{
                    int32_t rate = (int32_t)DEL_MULT * 2048 * 100 ;
                    if(mixweight)
										{
											uint8_t mweight = mixweight ;
											if ( mixweight < 0 )
											{
												mweight = -mixweight ;
											}
											rate /= mweight ;
										}

										int16_t speed ;
                    if ( diff>0 )
										{
											speed = md->speedUp ;
										}
										else
										{
											rate = -rate ;
											speed = md->speedDown ;
										}
										tact = (speed) ? tact+(rate)/((int16_t)timing*speed) : (int32_t)v*DEL_MULT ;

                }
								{
#if DEL_MULT == 256
									int32_t tmp = tact>>8 ;
#else
									int32_t tmp = tact/DEL_MULT ;
#endif
                	if(((diff>0) && (v<tmp)) || ((diff<0) && (v>tmp))) tact=(int32_t)v*DEL_MULT; //deal with overflow
								}
#if DEL_MULT == 256
                v = tact >> 8 ;
#else
                v = tact/DEL_MULT;
#endif
            }
            else if (diff)
            {
              tact=(int32_t)v*DEL_MULT;
            }
					Output.act[mixIndex] = tact ;

				}

        //========== CURVES ===============
				if ( md->differential )
				{
      		//========== DIFFERENTIAL =========
      		int16_t curveParam = REG100_100( md->curve ) ;
      		if (curveParam > 0 && v < 0)
      		  v = ((int32_t)v * (100 - curveParam)) / 100;
      		else if (curveParam < 0 && v > 0)
      		  v = ((int32_t)v * (100 + curveParam)) / 100;
				}
				else
				{
					if ( ( md->curve <= -28 ) || ( md->curve > 27 ) )
					{
						// do expo using md->curve + 128
						int8_t x = md->curve + 128 ;
      			v = expo( v, x ) ;
					}
					else
					{
      	  	switch(md->curve)
						{
      	  	case 0:
      	  	    break;
      	  	case 1:
      	  	        if( v<0 ) v=0;   //x|x>0
      	  	    break;
      	  	case 2:
      	  	        if( v>0 ) v=0;   //x|x<0
      	  	    break;
      	  	case 3:       // x|abs(x)
      	  	    v = abs(v);
      	  	    break;
      	  	case 4:       //f|f>0
      	  	    v = v>0 ? RESX : 0;
      	  	    break;
      	  	case 5:       //f|f<0
      	  	    v = v<0 ? -RESX : 0;
      	  	    break;
      	  	case 6:       //f|abs(f)
      	  	    v = v>0 ? RESX : -RESX;
      	  	    break;
      	  	default: //c1..c16
								{
									int8_t idx = md->curve ;
									if ( idx < 0 )
									{
										v = -v ;
										idx = 6 - idx ;
									}
      	  	    	v = intpol(v, idx - 7);
								}
      	  	}
					}
				}

        //========== TRIM ===============
        if((md->carryTrim==0) && (md->srcRaw>0) && (md->srcRaw<=4)) v += trimA[md->srcRaw-1];  //  0 = Trim ON  =  Default

        //========== MULTIPLEX ===============
        int32_t dv = (int32_t)v*mixweight ;
        //========== lateOffset ===============
				if ( md->lateOffset )
        {
            if(md->sOffset) dv += calc100toRESX( REG( md->sOffset, -125, 125 )	) * 100L ;
        }
        switch((uint8_t)md->mltpx)
				{
        	case MLTPX_REP:
            thisChan = dv;
      	  break;
    	    case MLTPX_MUL:
						dv /= 100 ;
						thisChan *= dv ;
            thisChan /= RESXl;
  	      break;
	        default:  // MLTPX_ADD
						thisChan += dv ;
          break;
        }
    }


		ThrottleStickyOn = 0 ;

        int32_t q = thisChan;// + (int32_t)g_model.limitData[i].offset*100; // offset before limit

				if ( Fade.fadePhases )
				{
					int32_t l_fade = Fade.fade[chanIndex] ;
					if ( att & FADE_FIRST )
					{
						l_fade = 0 ;
					}
					l_fade += ( q / 100 ) * Fade.fadeScale[CurrentPhase] ;
					Fade.fade[chanIndex] = l_fade ;

					if ( ( att & FADE_LAST ) == 0 )
					{
						continue ;
					}
					l_fade /= Fade.fadeWeight ;
					q = l_fade * 100 ;
				}
    	  thisChan = q / 100 ; // chans back to -1024..1024

				Ex_chans[chanIndex] = thisChan; //for getswitch

		    LimitData *limit = &g_model.limitData[chanIndex] ;
				FORCE_INDIRECT(limit) ;
				int16_t ofs = limit->offset;
				int16_t xofs = ofs ;
				if ( xofs > g_model.sub_trim_limit )
				{
					xofs = g_model.sub_trim_limit ;
				}
				else if ( xofs < -g_model.sub_trim_limit )
				{
					xofs = -g_model.sub_trim_limit ;
				}
        int16_t lim_p = 10*(limit->max+100) + xofs ;
        int16_t lim_n = 10*(limit->min-100) + xofs ; //multiply by 10 to get same range as ofs (-1000..1000)
				if ( lim_p > 1250 )
				{
					lim_p = 1250 ;
				}
				if ( lim_n < -1250 )
				{
					lim_n = -1250 ;
				}
				if(ofs>lim_p) ofs = lim_p;
        if(ofs<lim_n) ofs = lim_n;

        if(q)
				{
					int16_t temp = (q<0) ? ((int16_t)ofs-lim_n) : ((int16_t)lim_p-ofs) ;
          q = ( q * temp ) / 100000 ; //div by 100000 -> output = -1024..1024
				}

				int16_t result ;
				result = calc1000toRESX(ofs);
  			result += q ; // we convert value to a 16bit value

        lim_p = calc1000toRESX(lim_p);
        if(result>lim_p) result = lim_p;
        lim_n = calc1000toRESX(lim_n);
        if(result<lim_n) result = lim_n;

        if(limit->reverse) result = -result ;// finally do the reverse.


				{
					uint8_t numSafety = 16 - g_model.numVoice ;
					if ( chanIndex < numSafety )
					{
						SafetySwData *psw = &g_model.safetySw[chanIndex] ;
						FORCE_INDIRECT(psw) ;

        		if(psw->opt.ss.swtch)  //if safety sw available for channel check and replace val if needed
						{
							{
								static uint8_t sticky = 0 ;
								uint8_t applySafety = 0 ;
								int8_t sSwitch = psw->opt.ss.swtch ;

								if(getSwitch00( sSwitch))
								{
									applySafety = 1 ;
								}

								if ( psw->opt.ss.mode == 3 )
								{
									// Special case, sticky throttle
									if( applySafety )
									{
										sticky = 0 ;
									}
									else
									{
										uint8_t throttleOK = 0 ;
										if ( g_model.throttleIdle )
										{
											if ( abs( calibratedStick[2] ) < 20 )
											{
												throttleOK = 1 ;
											}
										}
										else
										{
  										if(calibratedStick[2] < -1004)
  										{
												throttleOK = 1 ;
  										}
										}
										if ( throttleOK )
										{
											if ( trainerThrottleValid )
											{
												if ( trainerThrottleValue < -1004 )
												{
													sticky = 1 ;
												}
											}
											else
											{
												sticky = 1 ;
											}
										}

									}
									if ( sticky == 0 )
									{
										applySafety = 1 ;
									}
									ThrottleStickyOn = applySafety ;
								}
								if ( applySafety ) result = calc100toRESX(g_model.safetySw[chanIndex].opt.ss.val) ;
							}
						}
					}
				}
				cli();
        chanOut[chanIndex] = result ; //copy consistent word to int-level
        sei();
	}
    //========== MIXER WARNING ===============
		// Processed after all mixes processed.
    //1= 00,08
    //2= 24,32,40
    //3= 56,64,72,80
#ifdef MIX_WARN
  {
    uint8_t tmr10ms ;
    tmr10ms = g_blinkTmr10ms ;	// Only need low 8 bits

    if(mixWarning & 1) if(((tmr10ms)==  0)) audioDefevent(AU_MIX_WARNING_1);
    if(mixWarning & 2) if(((tmr10ms)== 64) || ((tmr10ms)== 72)) audioDefevent(AU_MIX_WARNING_2);
    if(mixWarning & 4) if(((tmr10ms)==128) || ((tmr10ms)==136) || ((tmr10ms)==144)) audioDefevent(AU_MIX_WARNING_3);
  }
#endif
}


#define M_INDEX		0
#define M_DISPLAY	1
#define M_AUDIO		2
#define M_ALARMS	3
#define M_GENERAL	4
#define M_CONTROLS	5
#define M_CALIB		6
#define M_TRAINER	7
#define M_VERSION	8
#define M_DIAGKEYS	9
#define M_DIAGANA	10
#define M_HARDWARE	11

const prog_char APM Str_Display[] = STR_DISPLAY ;
const prog_char APM Str_Trainer[] = STR_TRAINER ;
const prog_char APM Str_Version[] = STR_VERSION ;
const prog_char APM Str_General[] = STR_GENERAL ;
const prog_char APM Str_Hardware[] = STR_HARDWARE ;
const prog_char APM Str_Alarms[] = STR_ALARMS ;
const prog_char APM Str_Controls[] = STR_CONTROLS ;
const prog_char APM Str_Calibration[] = STR_CALIBRATION ;
const prog_char APM Str_AudioHaptic[] = STR_AUDIOHAPTIC ;
const prog_char APM Str_DiagSwtch[] = STR_DIAGSWTCH ;
const prog_char APM Str_DiagAna[] = STR_DIAGANA ;
const prog_char APM Str_VoiceAla[] =  STR_VOICEALA ;

static uint8_t indexProcess( uint8_t event, MState2 *pmstate, uint8_t extra )
{
	struct t_menuControl *mc = &MenuControl ;
	FORCE_INDIRECT(mc) ;

	if (event == EVT_ENTRY)
	{
		pmstate->m_posVert = MenuControl.SubmenuIndex - 1 ;
		mc->SubmenuIndex = 0 ;
	}
	if (event == EVT_ENTRY_UP)
	{
		pmstate->m_posVert = mc->SubmenuIndex - 1 ;
		{
			mc->SubmenuIndex = 0 ;
		}
	}

	if ( mc->UseLastSubmenuIndex )
	{
		mc->SubmenuIndex = mc->LastSubmenuIndex & 0x7F ;
		mc->UseLastSubmenuIndex = 0 ;
	}

	if ( mc->SubmenuIndex )
	{
  	if ( (event == EVT_KEY_BREAK(KEY_EXIT) ) || (event == EVT_KEY_LONG(BTN_RE) ) )
		{
      if(s_editMode)
			{
        s_editMode = false;
			}
			else
			{
				pmstate->m_posVert = mc->SubmenuIndex - 1 ;
				mc->SubmenuIndex = 0 ;
				killEvents(event) ;
				audioDefevent(AU_MENUS) ;
	      s_editMode = false;
			}
			event = 0 ;
		}
	}
	else
	{
		uint8_t pv = pmstate->m_posVert ;
		if (event == EVT_KEY_FIRST(KEY_RIGHT) )
		{
			if ( pv < extra )
			{
				pv += 7 ;
			}
		}
		if (event == EVT_KEY_FIRST(KEY_LEFT) )
		{
			if ( pv >= 7)
			{
				pv -= 7 ;
			}
		}

 		if ( (event == EVT_KEY_FIRST(KEY_MENU) ) ||(event == EVT_KEY_BREAK(BTN_RE) ) )
		{
			mc->SubmenuIndex = pv + 1 ;
			mc->LastSubmenuIndex = mc->SubmenuIndex | 0x80 ;
			pv = 0 ;
			killEvents(event) ;
			g_posHorz = Tevent = event = 0 ;
		}
		pmstate->m_posVert = pv ;
	}
	return event ;
}

static void displayIndex( const prog_char *const strings[], uint8_t extra, uint8_t lines, uint8_t highlight )
{
	uint8_t offset = 7 ;

	for ( uint8_t i = 0 ; i < lines ; i += 1 )
	{
		lcd_puts_Pleft((i+1)*FH, (const prog_char *)pgm_read_adr( &strings[i] ) ) ;
		if ( i < extra )
		{
			lcd_puts_P( 69, (i+1)*FH, (const prog_char *)pgm_read_adr( &strings[i+offset] ) ) ;
		}
	}

	lcd_vline( 67, 0, 63 ) ;

	if ( highlight )
	{
		if ( highlight > 7 )
		{
			lcd_char_inverse( 69, (highlight-offset)*FH, 59, 0 ) ;
		}
		else
		{
			lcd_char_inverse( 0, highlight*FH, 66, 0 ) ;
		}
	}
}


void menuProcIndex(uint8_t event)
{
	static MState2 mstate;
	EditType = EE_GENERAL ;


	event = indexProcess( event, &mstate, ( SystemOptions & SYS_OPT_HARDWARE_EDIT ) ? 4 : 3 ) ;
	mstate.check_columns(event, IlinesCount-1 ) ;

	uint8_t sub = mstate.m_posVert ;
	uint8_t y = 1*FH ;
	uint8_t subN = 0 ;
	uint8_t blink = InverseBlink ;
	switch ( MenuControl.SubmenuIndex )
	{
		case M_INDEX :
                        TITLEP(Str_Radio_Setup);
			IlinesCount = ( SystemOptions & SYS_OPT_HARDWARE_EDIT ) ? 11 : 10 ;
			sub += 1 ;

                        static const prog_char *const n_Strings[11] = {
                        Str_Display,
                        Str_AudioHaptic,
                        Str_Alarms,
                        Str_General,
                        Str_Controls,
                        Str_Calibration,
                        Str_Trainer,
                        Str_Version,
                        Str_DiagSwtch,
                        Str_DiagAna,
                        Str_Hardware
                        } ;

			displayIndex( n_Strings, IlinesCount - 7, 7, sub ) ;
		break ;

		case M_DISPLAY :
		{
                    TITLEP( Str_Display ) ;
                    IlinesCount = 6 ;
                    uint8_t attr ;
                    attr = (sub==subN) ? blink : 0 ;
                    lcd_xlabel_decimal( PARAM_OFS+3*FW-2, y, g_eeGeneral.contrast, attr, PSTR(STR_CONTRAST) ) ;
                    if ( attr )
                    {
                            CHECK_INCDEC_H_GENVAR( g_eeGeneral.contrast, LCD_MINCONTRAST, LCD_MAXCONTRAST) ;
                            lcdSetContrast() ;
                    }
                    y += FH ;
                    subN += 1 ;

                    attr = 0 ;
                    lcd_puts_Pleft( y,PSTR(STR_LIGHT_SWITCH "\037" STR_LIGHT_INVERT "\037" STR_LIGHT_AFTER "\037" STR_LIGHT_STICK "\037" STR_FLASH_ON_BEEP
                    #if ROTATE_SCREEN
                    "\037" STR_ROTATE
                    #endif
                    ));
                    if(sub==subN) { attr = blink ; CHECK_INCDEC_GENERALSWITCH( g_eeGeneral.lightSw, -MaxSwitchIndex, MaxSwitchIndex );}
                    putsDrSwitches(PARAM_OFS-FW,y,g_eeGeneral.lightSw,attr);
                    y += FH ;
                    subN += 1 ;

                    g_eeGeneral.blightinv = onoffItem( g_eeGeneral.blightinv, y, sub==subN ) ;
                    y += FH ;
                    subN += 1 ;

                    for ( uint8_t i = 0 ; i < 2 ; i += 1 )
                    {
                            uint8_t b ;
                            b = ( i == 0 ) ? g_eeGeneral.lightAutoOff : g_eeGeneral.lightOnStickMove ;

                            uint8_t attr = 0 ;
                            if(sub==subN) { attr = blink ; CHECK_INCDEC_H_GENVAR_0( b, 600/5);}
                                    putsOffDecimal( PARAM_OFS, y, b*5, attr ) ;
                            if(b)
                            {
                                lcd_putc(PARAM_OFS+3*FW, y, 's');
                            }

                            if ( i == 0 )
                            {
                                    g_eeGeneral.lightAutoOff = b ;
                            }
                            else
                            {
                                    g_eeGeneral.lightOnStickMove = b ;
                            }
                            y += FH ;
                            subN += 1 ;
                    }

                    g_eeGeneral.flashBeep = onoffItem( g_eeGeneral.flashBeep, y, sub==subN ) ;

                    #if ROTATE_SCREEN
                    y += FH ;
                    subN += 1 ;

                    uint8_t b = g_eeGeneral.rotateScreen;
                    uint8_t c = onoffItem( b, y, sub==subN ) ;
                    if (b != c)
                    {
                    g_eeGeneral.rotateScreen = c;
                    lcdSetOrientation();
                    }
                    #endif
		}
		break ;

		case M_AUDIO :
		{
			TITLEP( Str_AudioHaptic ) ;
#ifdef GLOBAL_COUNTDOWN
			IlinesCount = 6 ;
#else
			IlinesCount = 5 ;
#endif

                        uint8_t b ;
                        uint8_t attr = LEFT ;
                        lcd_puts_Pleft( y,PSTR(STR_VOLUME "\037" STR_BEEPER "\037" STR_SOUND_MODE "\037" STR_SPEAKER_PITCH));
                        b = g_eeGeneral.volume+(NUM_VOL_LEVELS-1) ;
                        if(sub==subN) {
                            attr = blink | LEFT ;
                            CHECK_INCDEC_H_GENVAR_0( b, NUM_VOL_LEVELS-1 );
                         }
                        lcd_outdezAtt(PARAM_OFS, y, b, attr);
                      	g_eeGeneral.volume = (int8_t)b-(NUM_VOL_LEVELS-1) ;

                        y += FH ;
			subN += 1 ;

			g_eeGeneral.beeperVal = checkIndexed( y, PSTR(FWx14 "\006" STR_BEEP_MODES), g_eeGeneral.beeperVal, (sub==subN) ) ;
			y += FH ;
			subN += 1 ;

                        b = g_eeGeneral.speakerMode ;
			if ( b > 3 )
			{
				b = 4 ;
			}
			b = checkIndexed( y, PSTR(FWx11 "\004" STR_SPEAKER_OPTS), b, (sub==subN) ) ;
			g_eeGeneral.speakerMode = ( b >= 4 ) ? 7 : b ;
			y += FH ;
			subN += 1 ;


			attr = LEFT ;
                         if(sub==subN)
			{
				attr = blink | LEFT ;
                                CHECK_INCDEC_H_GENVAR( g_eeGeneral.speakerPitch, 1, 100);
                        }
                        lcd_outdezAtt(PARAM_OFS,y,g_eeGeneral.speakerPitch,attr);
                    	y += FH ;
			subN += 1 ;

                        attr = LEFT ;
                        if(sub==subN)
			{
				attr = blink | LEFT ;
                                CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.hapticStrength, 5);
                        }
			lcd_xlabel_decimal( PARAM_OFS, y, g_eeGeneral.hapticStrength, attr, PSTR(STR_HAPTICSTRENGTH) ) ;
                        y += FH ;
			subN += 1 ;

#ifdef GLOBAL_COUNTDOWN
                        g_eeGeneral.minuteBeep = onoffMenuItem( g_eeGeneral.minuteBeep, y, Str_minute_Beep, sub==subN ) ;
                        y += FH ;
			subN += 1 ;
#endif

		}
		break ;

                case M_CONTROLS: {  // delete PPMSIM item
                  TITLEP(Str_Controls);
                  IlinesCount = 9;//10;

                  if (sub < 5) {
                    displayNext();
                    lcd_puts_Pleft(y, PSTR(STR_CROSSTRIM "\037" STR_THR_REVERSE /*"\037" STR_ENABLE_PPMSIM*/ "\037" STR_CHAN_ORDER "\037" STR_MODE));
                    g_eeGeneral.crosstrim = onoffItem(g_eeGeneral.crosstrim, y, sub == subN);
                    y += FH;
                    subN += 1;

                    uint8_t oldValue = g_eeGeneral.throttleReversed;
                    g_eeGeneral.throttleReversed = onoffItem(oldValue, y, sub == subN);
                    if (g_eeGeneral.throttleReversed != oldValue) {
                      checkTHR();
                    }
                    y += FH;
                    subN += 1;

                    uint8_t attr = sub == subN ? blink : 0;
                    uint8_t bch = pgm_read_byte(bchout_ar + g_eeGeneral.templateSetup);
                    for (uint8_t i = 4; i > 0; i -= 1) {
                      uint8_t letter;
                      letter = pgm_read_byte(Str_1_RETA + (bch & 3) + 1);
                      lcd_putcAtt((14 + i) * FW, y, letter, attr);
                      bch >>= 2;
                    }
                    if (attr)
                      CHECK_INCDEC_H_GENVAR_0(g_eeGeneral.templateSetup, 23);
                    y += FH;
                    subN += 1;

                    for (uint8_t i = 0; i < 4; i += 1) {
                      lcd_img((6 + 4 * i) * FW, y, sticks, i);
                      if (g_eeGeneral.stickReverse & (1 << i))
                        lcd_char_inverse((6 + 4 * i) * FW, y, 3 * FW, 0);
                    }
                    if (sub == subN) {
                      attr = 0;
                      if (SystemOptions & SYS_OPT_HARDWARE_EDIT) {
                        CHECK_INCDEC_H_GENVAR_0(g_eeGeneral.stickReverse, 15);
                        if (BLINK_ON_PHASE) {
                          attr = 1;
                        }
                      }
                      if (attr == 0) {
                        lcd_rect(6 * FW - 1, y - 1, 15 * FW + 2, 9);
                      }
                    }
                    y += FH;
                    subN += 1;

                    attr = 0;
                    uint8_t mode = g_eeGeneral.stickMode;
                    if (sub == subN) {
                      attr = INVERS;
                      if (s_editMode) {
                        attr = BLINK;

                        CHECK_INCDEC_H_GENVAR_0(mode, 3);
                        if (mode != g_eeGeneral.stickMode) {
                          g_eeGeneral.stickScroll = 0;
                          g_eeGeneral.stickMode = mode;
                        }
                      }
                    }
                    lcd_putcAtt(3 * FW, y, '1' + mode, attr);
                    for (uint8_t i = 0; i < 4; i++)
                      putsChnRaw((6 + 4 * i) * FW, y, modeFixValue(i), 0); //sub==3?INVERS:0);
                    y += FH;
                    subN += 1;
                  } else {
                    subN = 5;//6;
                    if (sub >= IlinesCount) {
                      sub = mstate.m_posVert = IlinesCount - 1;
                    }

                    // Edit custom stick names
                    Columns = 3;
                    for (uint8_t i = 0; i < 4; i++) {
                      lcd_putsAttIdx(FW * 5, y, modi12x3, i, 0);
                      editName(g_posHorz, y, &g_eeGeneral.customStickNames[i * 4], 4, sub == subN);
                      subN += 1;
                      y += FH;
                    }
                  }

                } break;

                case M_ALARMS :
		{
			TITLEP( Str_Alarms ) ;
			IlinesCount = 6 ;

  		uint8_t attr = LEFT ;
      lcd_puts_Pleft( y,PSTR(STR_BATT_WARN "\037" STR_INACT_ALARM "\037" STR_THR_WARNING "\037" STR_SWITCH_WARN "\037" STR_MEM_WARN));
      if(sub==subN) { attr = blink | LEFT ; CHECK_INCDEC_H_GENVAR( g_eeGeneral.vBatWarn, 30, 120); } //5-10V
      putsVolts(PARAM_OFS, y, g_eeGeneral.vBatWarn, attr);
  		y += FH ;
			subN += 1 ;

  		attr = 0 ;//LEFT ;
      if(sub==subN) { attr = blink ;CHECK_INCDEC_H_GENVAR( g_eeGeneral.inactivityTimer, -10, 110); } //0..120minutes
      lcd_outdezAtt(PARAM_OFS+2*FW-2, y, g_eeGeneral.inactivityTimer+10, attr);
  		y += FH ;
			subN += 1 ;

      uint8_t b = g_eeGeneral.disableThrottleWarning;
      g_eeGeneral.disableThrottleWarning = offonItem( b, y, sub==subN ) ;
  		y += FH ;
			subN += 1 ;

      b = g_eeGeneral.disableSwitchWarning;
      g_eeGeneral.disableSwitchWarning = offonItem( b, y, sub==subN ) ;
  		y += FH ;
			subN += 1 ;

      b = g_eeGeneral.disableMemoryWarning;
      g_eeGeneral.disableMemoryWarning = offonItem( b, y, sub==subN ) ;
  		y += FH ;
			subN += 1 ;

      b = g_eeGeneral.disableAlarmWarning;
      g_eeGeneral.disableAlarmWarning = offonMenuItem( b, y, PSTR(STR_ALARM_WARN), sub==subN ) ;
  		y += FH ;
			subN += 1 ;

		}
		break ;

		case M_GENERAL :
		{
			TITLEP( Str_General ) ;
#ifdef GLOBAL_COUNTDOWN
			IlinesCount = 6 ;
#else
			IlinesCount = 5 ;
#endif
			if ( sub==0 )
			{
				Columns = 9 ;
			}
			editName( g_posHorz, y, (uint8_t *)g_eeGeneral.ownerName, sizeof(g_eeGeneral.ownerName), (sub==subN) ) ;
  		y += FH ;
			subN += 1 ;

#ifdef GLOBAL_COUNTDOWN
      g_eeGeneral.preBeep = onoffMenuItem( g_eeGeneral.preBeep, y, Str_Beep_Countdown, sub==subN ) ;
  		y += FH ;
			subN += 1 ;
#endif

      uint8_t b = g_eeGeneral.disableSplashScreen;
      g_eeGeneral.disableSplashScreen = offonMenuItem( b, y, PSTR(STR_SPLASH_SCREEN "\037" STR_SPLASH_NAME), sub==subN ) ;
  		y += FH ;
			subN += 1 ;

      b = g_eeGeneral.hideNameOnSplash;
      g_eeGeneral.hideNameOnSplash = offonItem( b, y, sub==subN ) ;
  		y += FH ;
			subN += 1 ;


      b = g_eeGeneral.disablePotScroll ;
      g_eeGeneral.disablePotScroll = offonMenuItem( b, y, PSTR(STR_POTSCROLL), sub==subN ) ;
  		y += FH ;
			subN += 1 ;

			g_eeGeneral.stickScroll = onoffMenuItem( g_eeGeneral.stickScroll, y, PSTR(STR_STICKSCROLL), sub==subN ) ;
  		y += FH ;
			subN += 1 ;

		}
		break ;

		case M_VERSION :
		{
      TITLEP( Str_Version ) ;

    	lcd_puts_Pleft( 2*FH,Stamps );
		}
		break ;

		case M_DIAGKEYS	:
		{
      TITLEP( PSTR(STR_DIAG) ) ;
	    uint8_t x=7*FW;


  	  for(uint8_t i=0; i<9; i++)
    	{
        uint8_t y=i*FH; //+FH;
        if(i>(SW_ID0-SW_BASE_DIAG)) y-=FH; //overwrite ID0
        putsDrSwitches(x,y,i+1,0); //ohne ofF,on
        bool t=keyState((EnumKeys)(SW_BASE_DIAG+i));
				putc_0_1( x+FW*4+2, y , t ) ;
    	}

	    x=0;
	    lcd_puts_Pleft(2*FH,PSTR(STR_KEYNAMES) ) ;
  	  for(uint8_t i=0; i<6; i++)
    	{
        uint8_t y=(5-i)*FH+2*FH;
        bool t=keyState((EnumKeys)(KEY_MENU+i));
        putc_0_1( x+FW*5+2,  y, t ) ;
    	}

	    x=14*FW;
  	  lcd_puts_P(x, 3*FH,PSTR(STR_TRIM_M_P) ) ;
	    for(uint8_t i=0; i<4; i++)
  	  {
        uint8_t y=i*FH+FH*4;
        lcd_img(    x,       y, sticks,i);
        bool tm=keyState((EnumKeys)(TRM_BASE+2*i));
        putc_0_1( x+FW*4, y, tm ) ;
        bool tp=keyState((EnumKeys)(TRM_BASE+2*i+1));
        putc_0_1( x+FW*6, y, tp ) ;
    	}


		}
		break ;

		case M_DIAGANA	:
		{
      TITLEP( PSTR(STR_ANA) ) ;
			IlinesCount = 1 ;

			StickScrollAllowed = 0 ;
	    for(uint8_t i=0; i<8; i++)
  	  {
        uint8_t y=i*FH;
        lcd_putc( 4*FW, y, 'A' ) ;
        lcd_putc( 5*FW, y, '1'+i ) ;
        lcd_outhex4( 7*FW, y,anaIn(i));
				uint8_t index = i ;
				if ( i < 4 )
				{
 					index = modeFixValue( i ) - 1 ;
 				}
        if(i<7)  lcd_outdez(15*FW, y, (int16_t)calibratedStick[index]*25/256) ;
        else putsVBat(15*FW,y,blink|PREC1) ;
    	}
        scroll_disabled = 1;
				if ( s_editMode )
				{
					CHECK_INCDEC_H_GENVAR( g_eeGeneral.vBatCalib, -127, 127);
				}
		}
		break ;

		case M_TRAINER :
		{
      TITLEP( Str_Trainer ) ;
			IlinesCount = 6 ;

			uint8_t subSub = g_posHorz ;

			if (SLAVE_MODE)
			{ // i am the slave
		    lcd_puts_Pleft( 3*FH, PSTR(STR_SLAVE));
			}
			else
			{

				y = 2*FH;
		    uint8_t attr;
				lcd_puts_Pleft( 1*FH, PSTR(STR_MODE_SRC_SW));

				for (uint8_t i=0; i<4; i++)
				{
    			volatile TrainerMix *td = &g_eeGeneral.trainer.mix[i];
					int8_t x ;
			    putsChnRaw(0, y, i+1, 0);

			    for (uint8_t j=0; j<4; j++)
					{
						if ( sub==i )
						{
							Columns = 3 ;
						}
						uint8_t active = 0 ;
						attr = 0 ;
      			if (sub==i && subSub==j)
						{
							attr = blink ;
							active = 2 ;
						}

						switch(j)
						{
							case 0 :
								td->mode = checkIndexed( y, PSTR(FWx4 "\002" STR_OFF_PLUS_EQ), td->mode, active ) ;
							break ;

							case 1 :
								x = td->studWeight ;
  		  				lcd_outdezAtt(11*FW, y, x*13/4, attr);
    						if (attr&BLINK) { CHECK_INCDEC_H_GENVAR( x, -31, 31); td->studWeight = x ; } //!! bitfield
							break ;

							case 2 :
								x = td->srcChn ;
			    			lcd_putsAttIdx(12*FW, y, Str_Chans_Gv, x+16, attr);
    						if (attr&BLINK) { CHECK_INCDEC_H_GENVAR_0( x, 3); td->srcChn = x ; } //!! bitfield
							break ;

							case 3 :
								x = td->swtch ;
	  	  				putsDrSwitches(15*FW, y, x, attr);
    						if (attr&BLINK) { CHECK_INCDEC_H_GENVAR( x, -15, 15); td->swtch = x ; }
							break ;
						}
					}
   				y += FH ;
				}

				attr = PREC1 ;
				if(sub==4) { attr = PREC1 | blink ; CHECK_INCDEC_H_GENVAR( g_eeGeneral.PPM_Multiplier, -10, 40);}
				lcd_xlabel_decimal( 13*FW, 6*FH, g_eeGeneral.PPM_Multiplier+10, attr, PSTR(STR_MULTIPLIER) ) ;

				for (uint8_t i=0; i<4; i++)
				{
    			uint8_t x = (i*8+16)*FW/2;
    			lcd_outdezAtt(x , 7*FH, (g_ppmIns[i]-g_eeGeneral.trainer.calib[i])*2, PREC1);
				}
				attr = 0 ;
				if (sub==5)
				{
    			if (event==EVT_KEY_FIRST(KEY_MENU))
					{
		        s_editMode = false ;
        		memcpy(g_eeGeneral.trainer.calib, g_ppmIns, sizeof(g_eeGeneral.trainer.calib));
		        STORE_GENERALVARS;     //eeWriteGeneral();
        		audioDefevent(AU_MENUS);
			    }
					attr = INVERS ;
				}
				lcd_putsAtt(0*FW, 7*FH, PSTR(STR_CAL), attr ) ;
			}
		}
		break ;

		case M_HARDWARE :
		{
			TITLEP( Str_Hardware ) ;


        #define HW_LINES		1


        #ifndef HW_EXTRA_LINES
        #define HW_EXTRA_LINES		0
        #endif


	IlinesCount = HW_LINES + HW_EXTRA_LINES ;


 	    uint8_t b ;
 	    b = g_eeGeneral.disableBG ;
 	    g_eeGeneral.disableBG = offonMenuItem( b, y, PSTR(STR_BANDGAP), sub==subN ) ;
  		y += FH ;
			subN += 1 ;


    }
		break ;

		case M_CALIB :
		{
      TITLEP( Str_Calibration ) ;
			IlinesCount = 1 ;

	    mstate.m_posVert = 0; // make sure we don't scroll or move cursor here

	    for(uint8_t i=0; i<7; i++)
			{ //get low and high vals for sticks and trims
        int16_t vt = anaIn(i);
        Xmem.Cal_data.loVals[i] = min(vt,Xmem.Cal_data.loVals[i]);
        Xmem.Cal_data.hiVals[i] = max(vt,Xmem.Cal_data.hiVals[i]);
        if(i>=4) Xmem.Cal_data.midVals[i] = (Xmem.Cal_data.loVals[i] + Xmem.Cal_data.hiVals[i])/2;
  	  }

	    scroll_disabled = Xmem.Cal_data.idxState; // make sure we don't scroll while calibrating

	    switch(event)
  	  {
    		case EVT_ENTRY:
        	Xmem.Cal_data.idxState = 0;
        break;

		    case EVT_KEY_BREAK(KEY_MENU):
    	    Xmem.Cal_data.idxState++;
	        if(Xmem.Cal_data.idxState==3)
  	      {
            audioDefevent(AU_MENUS);
            STORE_GENERALVARS;     //eeWriteGeneral();
            Xmem.Cal_data.idxState = 0;
    	    }
     	  break;
	    }

	    switch(Xmem.Cal_data.idxState)
  	  {
		    case 0:
        //START CALIBRATION
        //[MENU]
	        lcd_puts_Pleft( 3*FH, PSTR(STR_MENU_TO_START) ) ;//, 15, sub>0 ? INVERS : 0);
        break;

  		  case 1: //get mid
        //SET MIDPOINT
        //[MENU]

      	  for(uint8_t i=0; i<7; i++)
	        {
            Xmem.Cal_data.loVals[i] =  15000;
            Xmem.Cal_data.hiVals[i] = -15000;
            Xmem.Cal_data.midVals[i] = anaIn(i);
  	      }
    	    lcd_puts_Pleft( 2*FH, PSTR(STR_SET_MIDPOINT) ) ;//, 12, sub>0 ? INVERS : 0);
      	  lcd_puts_P(3*FW, 3*FH, menuWhenDone ) ;//, 16, sub>0 ? BLINK : 0);
        break;

		    case 2:
        //MOVE STICKS/POTS
        //[MENU]
					StickScrollAllowed = 0 ;

      	  for(uint8_t i=0; i<7; i++)
					{
            if(abs(Xmem.Cal_data.loVals[i]-Xmem.Cal_data.hiVals[i])>50)
						{
              g_eeGeneral.calibMid[i]  = Xmem.Cal_data.midVals[i];
              int16_t v = Xmem.Cal_data.midVals[i] - Xmem.Cal_data.loVals[i];
              g_eeGeneral.calibSpanNeg[i] = v - v/64;
              v = Xmem.Cal_data.hiVals[i] - Xmem.Cal_data.midVals[i];
              g_eeGeneral.calibSpanPos[i] = v - v/64;
            }
					}
        	g_eeGeneral.chkSum = evalChkSum() ;
	        lcd_puts_Pleft( 2*FH, PSTR(STR_MOVE_STICKS) ) ; //, 16, sub>0 ? INVERS : 0);
  	      lcd_puts_P(3*FW, 3*FH, menuWhenDone ) ; //, 16, sub>0 ? BLINK : 0);
        break;
	    }

    	doMainScreenGrphics();
		}
		break ;

	}
  asm("") ;		// Forces execution to here, prevents compiler filling in 'pops'
}

void displayVoiceRate( uint8_t x, uint8_t y, uint8_t rate, uint8_t attr )
{
	if ( ( rate >= 4 ) && ( rate <=32 ) )
	{
    lcd_outdezAtt( x+3*FW, y, rate-2, attr ) ;
	}
	else
	{
		if ( rate > 32 )
		{
			rate -= 29 ;
		}
		lcd_putsAttIdx( x, y, Str_On_Off_Both, rate, attr ) ;
	}
}


// FUnctions need to include ON, OFF and BOTH possibly
void menuProcVoiceOne(uint8_t event) {
  TITLEP(PSTR("Voice Alarm"));
  lcd_outdez(13 * FW, 0, s_currIdx + 1);
  static MState2 mstate2;
  VoiceAlarmData *pvad = &g_model.vad[s_currIdx];
  uint8_t rows = pvad->fnameType ? 11 - 1 : 10 - 1;
  mstate2.check_columns(event, rows);

  int8_t sub = mstate2.m_posVert;

  if (sub < 6) {
    lcd_puts_Pleft(FH, PSTR("Source"
                            "\037"
                            "Function"
                            "\037"
                            "Value"
                            "\037"
                            "Switch"
                            "\037"
                            "Rate"
                            "\037"
                            "Haptic"));
  } else {
    lcd_puts_Pleft(FH, PSTR("Play Source"
                            "\037"
                            "On no Telemetry"
                            "\037"
                            "FileType"));
  }

  uint8_t y = FH;
  uint8_t attr = 0;
  uint8_t blink = InverseBlink;

  if (sub < 6) {
    uint8_t subN = 0;
    displayNext();

    if (sub == subN) {
      attr = blink;
      CHECK_INCDEC_H_MODELVAR_0(pvad->source, NUM_XCHNRAW + NUM_TELEM_ITEMS);
    }
    putsChnRaw(16 * FW, y, pvad->source, attr);
    if (pvad->source) {
      int16_t value;
      value = getValue(pvad->source - 1);
      lcd_puts_Pleft(FH, PSTR("\007(\015)"));
      // TODO telemetry voice
      if (pvad->source > CHOUT_BASE + NUM_CHNOUT) {
        putsTelemetryChannel(12 * FW, FH, pvad->source - CHOUT_BASE - NUM_CHNOUT - 1, value, 0, TELEM_CONSTANT | TELEM_UNIT);

      } else {
        lcd_outdez(12 * FW, FH, value);
      }
    }
    y += FH;
    subN += 1;

    attr = 0;
    if (sub == subN) {
      attr = blink;
      CHECK_INCDEC_H_MODELVAR_0(pvad->func, 6);
    }
    if (pvad->func < 5) {
      lcd_putsAttIdx(13 * FW, y, Str_Cswitch, pvad->func, attr); // v1>v2  v1<v2
    } else {
      lcd_putsAttIdx(13 * FW, y, PSTR("\007v\140=val v=val  "), pvad->func - 5, attr); // v1>v2  v1<v2
    }
    y += FH;
    subN += 1;

    attr = 0;
    if (sub == subN) {
      attr = blink;
      StepSize = 100;
      pvad->offset = checkIncDec16(pvad->offset, -32000, 32000, EE_MODEL);
    }
//TODO
    if (pvad->source > CHOUT_BASE + NUM_CHNOUT) {
      putsTelemetryChannel(20 * FW, y, pvad->source - CHOUT_BASE - NUM_CHNOUT - 1, pvad->offset, attr, /*TELEM_NOTIME_UNIT |*/ TELEM_UNIT | TELEM_CONSTANT);
    } else {
      lcd_outdezAtt(FW * 20, y, pvad->offset, attr);
    }
    y += FH;
    subN += 1;

    attr = 0;
    if (sub == subN) {
      attr = blink;
      CHECK_INCDEC_MODELSWITCH(pvad->swtch, -MaxSwitchIndex, MaxSwitchIndex);
    }
    putsDrSwitches(16 * FW, y, pvad->swtch, attr);
    y += FH;
    subN += 1;

    attr = 0;
    if (sub == subN) {
      attr = blink;
      CHECK_INCDEC_H_MODELVAR_0(pvad->rate, 33);
    }
    displayVoiceRate(16 * FW, y, pvad->rate, attr);
    y += FH;
    subN += 1;

    pvad->haptic = checkIndexed(y, PSTR(FWx13 "\003"
                                              "\007-------Haptic1Haptic2Haptic3"),
        pvad->haptic, (sub == subN));
  } else {
    uint8_t subN = 6;
    pvad->vsource = checkIndexed(y, PSTR(FWx14 "\002"
                                               "\006No    BeforeAfter "),
        pvad->vsource, (sub == subN));
    y += FH;
    subN += 1;

    pvad->mute = checkIndexed(y, PSTR(FWx17 "\001"
                                            "\004PlayMute"),
        pvad->mute, (sub == subN));
    y += FH;
    subN += 1;

    uint8_t previous = pvad->fnameType;
    pvad->fnameType = checkIndexed(y, PSTR(FWx14 "\002"
                                                 "\006------Number Audio"),
        pvad->fnameType, (sub == subN));
    if (pvad->fnameType != previous) {
      pvad->vfile = 0;
    }
    y += FH;
    subN += 1;

    if (pvad->fnameType) {
      attr = 0;
      lcd_puts_Pleft(y, PSTR("Voice File"));
      if (pvad->fnameType == 1) // Number
      {
        if (sub == subN) {
          attr = blink;
          pvad->vfile = checkIncDec16(pvad->vfile, 0, 511, EE_MODEL);
        }
        if (pvad->vfile > 507) {
          lcd_putsAtt(18 * FW, y, Str_SC, attr);
          lcd_putcAtt(20 * FW, y, pvad->vfile - 507 + '0', attr);
        } else if (pvad->vfile > 500) {
          dispGvar(18 * FW, y, pvad->vfile - 500, attr);
        } else {
          lcd_outdezAtt(FW * 20, y, pvad->vfile, attr);
          if (attr) {
            if (event == EVT_KEY_LONG(KEY_MENU)) {
              putVoiceQueueLong(pvad->vfile);
            }
            s_editMode = 0;
          }
        }
      } else if (pvad->fnameType == 2) // Audio
      {
        if (sub == subN) {
          attr = blink;
          uint8_t b;
          b = pvad->vfile;
          CHECK_INCDEC_H_MODELVAR_0(b, 15);
          pvad->vfile = b;
        }
        lcd_putsAttIdx(15 * FW, y, Str_Sounds, pvad->vfile, attr);
      }
      y += FH;
      subN += 1;
    }

    lcd_puts_Pleft(y, PSTR("Delete"));
    attr = 0;
    if (sub == subN) {
      attr = blink;
      if (event == EVT_KEY_LONG(KEY_MENU)) {
        memset(pvad, 0, sizeof(VoiceAlarmData));
        s_editMode = 0;
        if (sub == 10) {
          mstate2.m_posVert = 8;
        }
      }
    }
    lcd_putsAtt(12 * FW, y, PSTR(" OK  LONG"), attr);
  }
}

void menuProcVoiceAlarm(uint8_t event) {
  TITLE(STR_VOICE);
  static MState2 mstate2;
  mstate2.check_columns(event, NUM_VOICE_ALARMS - 1);

  int8_t sub = mstate2.m_posVert;

  uint8_t t_pgOfs = evalOffset(sub);

  switch (event) {
  case EVT_KEY_FIRST(KEY_MENU):
  case EVT_KEY_BREAK(BTN_RE):
    if (sub < NUM_VOICE_ALARMS) {
      s_currIdx = sub;
      killEvents(event);
      pushMenu(menuProcVoiceOne);
    }
    break;
  }

  uint8_t y = 1*FH ;
  for (uint8_t i = 0 ; i < 7 ; i += 1 )
	{
    uint8_t k = i + t_pgOfs ;
    uint8_t attr = sub == k ? INVERS : 0 ;
		{
			VoiceAlarmData *pvad = &g_model.vad[k] ;

			if(y>7*FH) break ;

	#if (NUM_VOICE_ALARMS<10)
			lcd_xlabel_decimal( FW*3-1, y, k+1, 0, PSTR("VA") ) ;
	#else
			lcd_xlabel_decimal( (k<9) ? FW*3-1 : FW*4-2, y, k+1, 0, PSTR("VA") ) ;
	#endif
			putsChnRaw( 5*FW, y, pvad->source, 0 ) ;
			putsDrSwitches( 9*FW, y, pvad->swtch, 0 ) ;
			displayVoiceRate( 13*FW, y, pvad->rate, 0 ) ;

    	uint8_t type = pvad->fnameType ;
			switch ( type )
			{
				case 1 :
					lcd_putc( 19*FW, y, '#' ) ;
				break ;
				case 2 :
					lcd_putc( 19*FW, y, 'A' ) ;
				break ;
			}


			if (pvad->haptic)
			{
				lcd_putc( 20*FW, y, 'H' ) ;
			}
			if ( attr )
			{
				lcd_char_inverse( 0, y, 20*FW, 0 ) ;
			}

		}
		y += FH ;
  }
}

enum ModelIndices
{
	M_MINDEX,
	M_MIXER,
	M_HELI,
	M_LIMITS,
	M_EXPO,
	M_MODES,
	M_CURVES,
	M_SWITCHES,
	M_SAFETY,
	M_GLOBALS,
	M_TELEMETRY,
	M_VOICE,
	M_TIMERS,
	M_MGENERAL,
	M_PROTOCOL
} ;

#define NUM_M_OPTIONS	M_PROTOCOL

 #define MODEL_ITEMS	14
 #define MODEL_EXTRA	7

const prog_char APM Str_Cswitches[] = STR_CSWITCHES ;
// STR_EXPO_DR
const prog_char APM Str_Voice[] = STR_VOICE ;


void rangeBindAction( uint8_t y, uint8_t newFlag )
{
	lcd_char_inverse( 0, y, 5*FW, 0 ) ;
	if ( Tevent==EVT_KEY_LONG(KEY_MENU))
	{
  	pxxFlag = newFlag ;		    	//send bind code or range check code
		pushMenu(menuRangeBind) ;
	}
	s_editMode = 0 ;
}

/**********************Kotello*****************************************************/
// TODO RX setup
void menuRXSetup(uint8_t event){

	static MState2 mstate2 ;
	mstate2.check_columns(event, 2) ;
	TITLEP("RX Setup");
	int8_t sub = mstate2.m_posVert ;
	int8_t subN = 0;
	uint8_t y = FH;
   	uint8_t attr = 0;// | LEFT ;
 	g_model.PPMOut = onoffMenuItem( g_model.PPMOut, y, PSTR("PPM Out"), sub == subN) ;
	subN++;
	y += FH;
	lcd_puts_Pleft(y,"Sub Protocol\037Servo Freq Hz");
	g_model.IS_BUS = checkIndexed( y, PSTR(FWx15 "\002" "\005I-BUSS-BUS"), g_model.IS_BUS, (sub==subN) ) ;
	y += FH ;
	subN ++ ;
    if(sub==subN)
    {
    	g_model.ServoFreq = checkIncDec16(g_model.ServoFreq, 50 , 400,EE_MODEL);
        attr = InverseBlink;
    }
    lcd_outdezAtt(PARAM_OFS,y,g_model.ServoFreq,attr);
}
/*********************************************************************************/

void menuSetFailsafe(uint8_t event)
{
	uint8_t b ;
	static MState2 mstate2 ;
	mstate2.check_columns(event, 14-1+1+1+1 ) ;
    TITLEP("Failsafe");
	int8_t sub = mstate2.m_posVert ;
	uint8_t y = 0;
	uint8_t k = 0;
	uint8_t t_pgOfs ;
	int32_t value ;
	t_pgOfs = evalOffset(sub);

	StickScrollAllowed = 0 ;		// Block while editing

	for(uint8_t i=0; i<7; i++)
	{
    y=(i+1)*FH;
    k=i+t_pgOfs;
    uint8_t attr = ((sub==k) ? InverseBlink : 0);
		uint8_t active = (attr && s_editMode ) ;

		if ( k == 0 )
		{
			b = g_model.failsafeMode ;
			g_model.failsafeMode = onoffMenuItem( b, y, PSTR("Activate"), sub == k ) ;
		}
		else if ( k == 1 )
		{
    		b = g_model.failsafeRepeat ;
        	g_model.failsafeRepeat = onoffMenuItem( b, y, PSTR("Transmit to RX"), sub == k ) ;
		}
		else if ( k == 2 )
		{
			if ( attr )
			{
				if ( event==EVT_KEY_LONG(KEY_MENU))
				{
					uint32_t j ;
  	  	  killEvents(event);
  			  s_editMode = 0 ;
					for ( j = 0 ; j < 14 ; j += 1 )
					{
						value = g_chans512[j] * 25 / (RESX/4) ;
  					if(value > 125)
						{
							value = 125 ;
						}
  					if(value < -125 )
						{
							value = -125 ;
						}
						int8_t *p = j < 8 ? &g_model.Failsafe[j] : &g_model.XFailsafe[j-8] ;
						*p = value ;
					}
					StatusTimer = 0 ;
		    	eeDirty(EE_MODEL) ;
		    	audioDefevent(AU_KEYPAD_UP);
				}
			}
		  lcd_putsAtt( 0, y, PSTR( "Use Actual" ), StatusTimer ? 0 : attr ) ;
		}
		else
		{
  	  putsChn(0,y,k-2,0);
			uint8_t index = k-3 ;
			int8_t *p = index < 8 ? &g_model.Failsafe[index] : &g_model.XFailsafe[index-8] ;
			value = *p ;
			lcd_outdezAtt(  7*FW+3, y, value, attr ) ;
			lcd_putc(  7*FW+5, y, '%' ) ;
    	if(active)
			{
  	    *p = checkIncDec16( value, -125, 125, EE_MODEL ) ;
    	}
		}
	}
}
/*-------------------------------------------------------------------------------------*/


void menuProcProtocol(uint8_t event) {

  static MState2 mstate2;

  uint8_t sub = mstate2.m_posVert;
  uint8_t subN = 0;
  uint8_t blink = InverseBlink;
  uint8_t y = 0;
  uint8_t attr = 0;
  uint8_t oldProtocol = g_model.protocol;

  lcd_puts_Pleft(y, PSTR(STR_PROTO));
  y += FH;
  subN++;

  switch (g_model.protocol) {
  case PROTO_PPM:
    mstate2.check_columns(event, 5);
    lcd_puts_Pleft(y, PSTR(STR_PPM_TYPE));
    if (sub == subN) {
      attr = INVERS;
      CHECK_INCDEC_H_MODELVAR_0(g_model.ppmStart, 7);
    }
    lcd_putcAtt(10 * FW, y, '1' + g_model.ppmStart, attr);
    attr = 0;
    y += FH;
    subN++;
    if (sub == subN) {
      attr = INVERS;
      CHECK_INCDEC_H_MODELVAR(g_model.ppmNCH, -2, 4);
    }
    lcd_putsAttIdx(10 * FW, y, PSTR(STR_PPMCHANNELS), (g_model.ppmNCH + 2), (sub == subN ? blink : 0));
    attr = 0;
    y += FH;
    subN++;
    if (sub == subN) {
      attr = INVERS;
      CHECK_INCDEC_H_MODELVAR(g_model.ppmDelay, -4, 10);
    }
    lcd_outdezAtt(13 * FW - 2, y, (g_model.ppmDelay * 50) + 300, (sub == subN ? blink : 0));
    lcd_puts_Pleft(y, PSTR(STR_15_US));
    attr = PREC1;
    y += FH;
    subN++;
    if (sub == subN) {
      attr = INVERS | PREC1;
      CHECK_INCDEC_H_MODELVAR(g_model.ppmFrameLength, -20, 20);
    }
    lcd_xlabel_decimal(13 * FW - 2, y, (int16_t)g_model.ppmFrameLength * 5 + 225, attr, PSTR(STR_PPMFRAME_MSEC));
    attr = 0;
    y += FH;
    subN++;
    g_model.pulsePol = checkIndexed(y, PSTR(FWx10 "\001" STR_POS_NEG), g_model.pulsePol, (sub == subN));
    lcd_puts_Pleft(y, PSTR(STR_SHIFT_SEL));
    break;
  case PROTO_AFHDS2A:
    mstate2.check_columns(event, 4);
    lcd_puts_Pleft(y, PSTR(STR_AFHDS2A_TYPE));
    if (sub == subN) {
      lcd_char_inverse(0, y, 8 * FW, 0);

      if (event == EVT_KEY_FIRST(KEY_MENU)) {
        pushMenu(menuRXSetup);
      }
    }
    y += FH;
    subN++;
    if (g_model.failsafeMode == 0) {
      lcd_puts_Pleft(y, PSTR("\011(Not Active)"));
    }
    if (sub == subN) {
      lcd_char_inverse(0, y, 8 * FW, 0);
      if (event == EVT_KEY_FIRST(KEY_MENU)) {
        pushMenu(menuSetFailsafe);
      }
    }
    y += FH;
    subN++;
    if (sub == subN) {
      g_model.FreqOffset = checkIncDec16(g_model.FreqOffset, -500, 500, EE_MODEL);
      attr = InverseBlink;
    }
    lcd_outdezAtt(17 * FW, y, g_model.FreqOffset, attr);
    y += FH;
    subN++;

    if (sub == subN) {
      rangeBindAction(y, PXX_BIND);
    }
    break;

  case PROTO_AFHDS:
    mstate2.check_columns(event, 2);

    lcd_puts_Pleft(y, PSTR(STR_AFHDS_TYPE));
    if (sub == subN) {
      g_model.FreqOffset = checkIncDec16(g_model.FreqOffset, -500, 500, EE_MODEL);
      attr = InverseBlink;
    }
    lcd_outdezAtt(17 * FW, y, g_model.FreqOffset, attr);
    y += FH;
    subN++;
    if (sub == subN) {
      rangeBindAction(y, PXX_BIND);
    }
    break;
  case PROTO_CRSF:
    crossfileMenu(mstate2, event, sub, subN, y);
    break;
  default:
    break;
  }
  if (sub == 0) {
    CHECK_INCDEC_H_MODELVAR_0(g_model.protocol, PROT_MAX);
  }
  lcd_putsAttIdx(6 * FW, 0, PSTR(PROT_STR), g_model.protocol, (sub == 0 ? blink : 0));

  if (oldProtocol != g_model.protocol) // if change - reset ppmNCH
  {
    g_model.ppmNCH = 0;
    startPulses();
  }
}


/*-------------------------------------------------------------------------------------*/


void menuProcModelIndex(uint8_t event)
{
	static MState2 mstate;
	EditType = EE_MODEL ;

		event = indexProcess( event, &mstate, MODEL_EXTRA ) ;
		mstate.check_columns(event, IlinesCount-1 ) ;

	switch ( MenuControl.SubmenuIndex )
	{
		case M_MIXER :
      pushMenu(menuProcMix) ;
		break ;
		case M_MODES :
      pushMenu(menuModelPhases) ;
		break ;
		case M_CURVES :
      pushMenu(menuProcCurve) ;
		break ;
		case M_TELEMETRY :
      pushMenu(menuProcTelemetry) ;
		break ;
		case M_GLOBALS :
      pushMenu(menuProcGlobals) ;
		break ;
		case M_VOICE :
      pushMenu(menuProcVoiceAlarm) ;
		break ;
		case M_PROTOCOL :
      pushMenu(menuProcProtocol) ;

		break ;

	}

	uint8_t sub = mstate.m_posVert ;
	uint8_t y = FH ;
	uint8_t subN = 0 ;
	uint8_t blink = InverseBlink ;

	switch ( MenuControl.SubmenuIndex )
	{
		case M_MINDEX :
  		TITLEP(Str_Model_Setup);
			IlinesCount = MODEL_ITEMS ;
			sub += 1 ;

static const prog_char *const n_Strings[] = {
Str_Mixer,
Str_heli_setup,
Str_limits,
Str_Expo,
Str_Modes,
Str_Curves,
Str_Cswitches,
Str_Safety,
Str_Globals,
Str_Telemetry,
Str_Voice,
Str_Timer,
Str_General,
Str_Protocol
};


			displayIndex( n_Strings, MODEL_EXTRA, 7, sub ) ;

		break ;

		case M_MGENERAL :
		{
			TITLEP(Str_General);
			if ( sub < 11 )
			{
				displayNext() ;
			}
			IlinesCount = 16 ;

			if ( sub < 6 )
			{
				if ( sub==subN )
				{
					Columns = sizeof(g_model.name)-1 ;
				}
				editName( g_posHorz, y, (uint8_t *)g_model.name, sizeof(g_model.name), sub==subN ) ;
  			y += FH ;
				subN += 1 ;

				uint8_t attr = 0 ;
  		  if(sub==subN)
				{
					if (event == EVT_KEY_FIRST(KEY_MENU) )
					{
						putVoiceQueueUpper( g_model.modelVoice ) ;
					}
					attr = blink ;
  		    CHECK_INCDEC_H_MODELVAR_0( g_model.modelVoice, 49 ) ;
				}
				lcd_xlabel_decimal( 15*FW-2, y, (int16_t)g_model.modelVoice+260, attr, PSTR(STR_VOICE_INDEX) ) ;
  			y += FH ;
				subN += 1 ;


      	lcd_puts_Pleft(    y, PSTR(STR_DEAFULT_SW_PAGE));
      	for(uint8_t i=0, q=1;i<8;q<<=1,i++)
				{
					lcd_putsnAtt((11+i)*FW, y, Str_TRE012AG+i,1,  (((uint8_t)g_model.switchWarningStates & q) ? INVERS : 0 ) );
				}
      	if(sub==subN)
				{
					lcd_rect( 11*FW-1, y-1, 9*FW+2, 9 ) ;
      	  if (event==EVT_KEY_FIRST(KEY_MENU) || event==EVT_KEY_BREAK(BTN_RE))
					{
      	    killEvents(event);
	    	    g_model.switchWarningStates = getCurrentSwitchStates() ;
      	  	s_editMode = false ;
      	    STORE_MODELVARS ;
					}
				}
  			y += FH ;
				subN += 1 ;

				g_model.useCustomStickNames = onoffItem( g_model.useCustomStickNames, y, sub==subN) ;
  			y += FH ;
				subN += 1 ;

  			attr = PREC1 ;
  	  	if(sub==subN) { attr |= blink ; CHECK_INCDEC_H_MODELVAR_0( g_model.sub_trim_limit, 100 ) ; }
			  lcd_outdezAtt( 20*FW, y, g_model.sub_trim_limit, attr ) ;
  			y += FH ;
				subN += 1 ;

				uint8_t tvol = g_model.anaVolume ;
				g_model.anaVolume = checkIndexed( y, PSTR(FWx17"\007\003---VrAVrBVrCGV4GV5GV6GV7"), tvol, (sub==subN) ) ;
			}
			else if ( sub < 11 )
			{
				subN = 6 ;
  	  	lcd_puts_Pleft(    y, PSTR(STR_VOL_PAGE));

				g_model.extendedLimits = onoffItem( g_model.extendedLimits, y, sub==subN) ;
    //---------------------------------                           /* ReSt V */
    //   If extended Limits are switched off, min and max limits must be limited to +- 100
        if (!g_model.extendedLimits)
        {
          for( LimitData *ld = &g_model.limitData[0] ; ld < &g_model.limitData[NUM_CHNOUT] ; ld += 1 )
          {
            FORCE_INDIRECT(ld) ;
            if (ld->min < 0) ld->min = 0;
            if (ld->max > 0) ld->max = 0;
          }
        }
    //-----------------------------------                           /* ReSt A */
  			y += FH ;
				subN += 1 ;

				uint8_t oldValue = g_model.throttleIdle ;
				g_model.throttleIdle = checkIndexed( y, PSTR(FWx15"\001\006  End Centre"), oldValue, (sub==subN) ) ;
				if ( g_model.throttleIdle != oldValue )
				{
  				checkTHR() ;
				}
				y += FH ;
				subN += 1 ;

				oldValue = g_model.throttleReversed ;
      	g_model.throttleReversed = onoffItem( oldValue, y, sub==subN) ;
				if ( g_model.throttleReversed != oldValue )
				{
  				checkTHR() ;
				}
  			y += FH ;
				lcd_putc( 16*FW, y, throttleReversed() ? '\201' : '\200' ) ;
  			y += FH ;
				subN += 1 ;

				g_model.thrTrim = onoffItem( g_model.thrTrim, y, sub==subN) ;
  			y += FH ;
				subN += 1 ;

				g_model.thrExpo = onoffItem( g_model.thrExpo, y, sub==subN) ;

			}
			else
			{
				subN = 11 ;

	  	  lcd_puts_Pleft(    y, PSTR(STR_TRIM_PAGE));

				g_model.trimInc = checkIndexed( y, PSTR(FWx14 "\004" STR_TRIM_OPTIONS), g_model.trimInc, (sub==subN) ) ;
  			y += FH ;
				subN += 1 ;

  		  uint8_t attr = 0 ;
  		  if(sub==subN) { attr = blink ; }
				g_model.trimSw = edit_dr_switch( 17*FW, y, g_model.trimSw, attr, attr ? EDIT_DR_SWITCH_EDIT : 0 ) ;
  			y += FH ;
				subN += 1 ;

				g_model.mixTime = onoffItem( g_model.mixTime, y, sub==subN) ;
				y += FH ;
				subN += 1 ;
				g_model.traineron = onoffItem( g_model.traineron, y, sub==subN) ;
  			y += FH ;
				subN += 1 ;

				uint8_t b = 1 ;
				for(uint8_t i=0;i<7;i++)
				{
					uint8_t z = g_model.beepANACenter ;
    			lcd_putcAtt( (9+i)*(FW+1), y, pgm_read_byte( &PSTR(STR_RETA123)[i]), ( z & b ) ? INVERS : 0 ) ;
					if ( sub==subN )
				  {
						Columns = 6 ;
						if ( g_posHorz == i )
						{
							lcd_rect( (9+i)*(FW+1)-1, y-1, FW+2, 9 ) ;
							if ( event==EVT_KEY_BREAK(KEY_MENU) || event==EVT_KEY_BREAK(BTN_RE) )
							{
								g_model.beepANACenter ^= b ;
      					eeDirty(EE_MODEL) ;
    						s_editMode = false ;
							}
						}
					}
					b <<= 1 ;
				}
			}

		}
		break ;

		case M_TIMERS :
		{
			TITLEP(Str_Timer) ;
#ifdef GLOBAL_COUNTDOWN
			IlinesCount = 10 ;
#else
			IlinesCount = 14 ;
#endif

			editTimer( sub ) ;
			TimerMode *ptConfig = &TimerConfig[0] ;
			FORCE_INDIRECT(ptConfig) ;
			g_model.tmrVal = ptConfig->tmrVal ;
			g_model.tmrMode = ptConfig->tmrModeA ;
			g_model.tmrModeB = ptConfig->tmrModeB ;
			g_model.tmrDir = ptConfig->tmrDir ;
			ptConfig += 1 ;
			g_model.tmr2Val = ptConfig->tmrVal ;
			g_model.tmr2Mode = ptConfig->tmrModeA ;
			g_model.tmr2ModeB = ptConfig->tmrModeB ;
			g_model.tmr2Dir = ptConfig->tmrDir ;
		}
		break ;

		case M_HELI :
		{
			TITLEP(Str_heli_setup) ;
			IlinesCount = 6 ;

		  uint8_t b ;
		  uint8_t attr ;
			uint8_t blink = InverseBlink ;

			uint8_t subN = 0 ;
		  lcd_puts_Pleft(    y, PSTR(STR_HELI_TEXT));

		  b = g_model.swashType ;
			g_model.swashType = checkIndexed( y, PSTR(FWx17 "\004" SWASH_TYPE_STR), b, (sub==subN) ) ;
			y += FH ;
			subN++;

			attr = 0 ;
		  if(sub==subN) {attr = blink ; CHECK_INCDEC_H_MODELVAR_0( g_model.swashCollectiveSource, NUM_XCHNRAW);}
		  putsChnRaw(17*FW, y, g_model.swashCollectiveSource, attr);
			y += FH ;
			subN++;

			attr = 0 ;
		  if(sub==subN) {attr = blink ; CHECK_INCDEC_H_MODELVAR_0( g_model.swashRingValue, 100);}
			putsOffDecimal( FW*14, y, g_model.swashRingValue, attr ) ;
			y += FH ;
			subN++;

			g_model.swashInvertELE = hyphinvMenuItem( g_model.swashInvertELE, y, sub==subN ) ;
			y += FH ;
			subN++;

			g_model.swashInvertAIL = hyphinvMenuItem( g_model.swashInvertAIL, y, sub==subN ) ;
			y += FH ;
			subN++;

			g_model.swashInvertCOL = hyphinvMenuItem( g_model.swashInvertCOL, y, sub==subN ) ;

		}
		break ;

		case M_EXPO :
		{
			TITLEP(Str_Expo) ;
			IlinesCount = 5 ;

			uint8_t stkVal ;
			StickScrollAllowed = 0 ;

			uint8_t l_expoChan = s_expoChan ;
			{
  		  uint8_t attr = 0 ;
				if ( sub == 0 )
				{
					s_expoChan = l_expoChan = checkIncDec( l_expoChan, 0, 3, 0 ) ;
					attr = INVERS ;
				}
				putsChnRaw(0,FH,l_expoChan+1,attr) ;
			}

			uint8_t expoDrOn = get_dr_state(l_expoChan);

  		lcd_putsAttIdx(  7*FW, FH, PSTR(STR_4DR_HIMIDLO), expoDrOn, 0 ) ;

			lcd_puts_Pleft( FH,PSTR(STR_EXPO_TEXT));

			stkVal = DR_BOTH ;
			if(calibratedStick[l_expoChan]> 100) stkVal = DR_RIGHT ;
			if(calibratedStick[l_expoChan]<-100) stkVal = DR_LEFT ;
			if(IS_EXPO_THROTTLE(l_expoChan)) stkVal = DR_RIGHT;

			editExpoVals( (stkVal != DR_RIGHT) && (sub==1), 4*FW, 3*FH, expoDrOn ,DR_EXPO, DR_LEFT ) ;
			editExpoVals( (stkVal != DR_LEFT) && (sub==1), 8*FW, 3*FH, expoDrOn ,DR_EXPO, DR_RIGHT ) ;

			editExpoVals( (stkVal != DR_RIGHT) && (sub==2), 4*FW, 5*FH, expoDrOn ,DR_WEIGHT, DR_LEFT ) ;
			editExpoVals( (stkVal != DR_LEFT) && (sub==2), 8*FW, 5*FH, expoDrOn ,DR_WEIGHT, DR_RIGHT ) ;

			editExpoVals( sub==3,5*FW, 6*FH, DR_DRSW1 , 0,0);
			editExpoVals( sub==4,5*FW, 7*FH, DR_DRSW2 , 0,0);

			lcd_vline(XD - (IS_EXPO_THROTTLE(s_expoChan) ? WCHART : 0), Y0 - WCHART, WCHART * 2);

			plotType = PLOT_BLACK ;

			drawFunction( XD, GRAPH_FUNCTION_EXPO ) ;

			int16_t x512  = calibratedStick[s_expoChan];
			int16_t y512 = calcExpo( s_expoChan, x512 ) ;

			lcd_outdez( 19*FW, 6*FH,x512*25/((signed) RESXu/4) );
			lcd_outdez( 14*FW, 1*FH,y512*25/((signed) RESXu/4) );

			int8_t xv = (x512 * WCHART + RESX/2) / RESX + XD ;
                        int8_t yv = Y0 - (y512 * WCHART + RESX/2) / RESX ;

			lcd_vline( xv, yv-3, 7 ) ;
			lcd_hline( xv-3, yv, 7 ) ;

			plotType = PLOT_XOR ;

	  }
		break ;

          case M_SWITCHES:
            menuProcSwitches(sub);
            break;

          case M_LIMITS:
            menuProcLimits(sub);
            break;

          case M_SAFETY:
            menuProcSafetySwitches(sub);
            break;
          }
          asm("");
}

uint8_t evalOffset(int8_t sub)
{
  uint8_t t_pgOfs = s_pgOfs ;
	int8_t x = sub-t_pgOfs ;
    if(sub<1) t_pgOfs=0;
    else if(x>(int8_t)6) t_pgOfs = sub-(int8_t)6;
    else if(x<(int8_t)(6-6)) t_pgOfs = sub ; //-(int8_t)6+6;
		return (s_pgOfs = t_pgOfs) ;
}

