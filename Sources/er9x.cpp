// Modified 2026 by sergegrin
/*
 * er9x.cpp
 *
 *  Created on: 10 авг. 2019 г.
 *      Author: KOSTYA
 */
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
#include "language.h"
#include "pulses.h"
#include "lcd.h"
#include "menus.h"
#include "voice.h"
#include "iface_a7105.h"
#include "crossfire/crossfire.h"
// Next two lines swapped as new complier/linker reverses them in memory!
const


/*
mode1 rud ele thr ail
mode2 rud thr ele ail
mode3 ail ele thr rud
mode4 ail thr ele rud
*/


#define ROTARY	1


extern int16_t AltOffset ;

uint8_t Last_switch[NUM_CSW] ;

static void checkMem( void );
void checkTHR( void );
///   Pr�ft beim Einschalten ob alle Switches 'off' sind.
void checkSwitches( void );

int8_t getGvarSourceValue( uint8_t src ) ;

static void checkQuickSelect( void ); // Quick model select on startup
void getADC_osmp( void ) ;

EEGeneral  g_eeGeneral;
ModelData g_model ;

extern uint8_t scroll_disabled ;
const char *AlertMessage ;
uint8_t Main_running ;
uint8_t SlaveMode ;
uint8_t Nvs_state[NUM_VOICE_ALARMS] ;
int16_t Nvs_timer[NUM_VOICE_ALARMS] ;
uint8_t CurrentVolume ;

uint8_t ppmInAvailable = 0 ;

struct t_rotary Rotary ;

uint8_t Tevent ;


TimerMode TimerConfig[2] ;

const uint8_t bchout_ar[] = {0x1B,
    0x1E,
    0x27,
    0x2D,
    0x36,
    0x39,
    0x4B,
    0x4E,
    0x63,
    0x6C,
    0x72,
    0x78,
    0x87,
    0x8D,
    0x93,
    0x9C,
    0xB1,
    0xB4,
    0xC6,
    0xC9,
    0xD2,
    0xD8,
    0xE1,
    0xE4};

//new audio object
audioQueue  audio;

uint8_t sysFlags = 0;
uint8_t SystemOptions ;


struct t_alarmControl AlarmControl = { 100, 0, 10, 2 } ;

int16_t  CsTimer[NUM_CSW] ;

const char  Str_Alert[] = STR_ALERT ;
const char  Str_Switches[] = SWITCHES_STR ;
const char Str_OFF[] =  STR_OFF ;
const char Str_ON[] = STR_ON ;

const char modi12x3[]= {"\004" STR_STICK_NAMES} ;
const uint8_t stickScramble[]= {
    0, 1, 2, 3,
    0, 2, 1, 3,
    3, 1, 2, 0,
    3, 2, 1, 0 };


const char Str_Hyphens[] = "----" ;

uint8_t modeFixValue( uint8_t value )
{
	return *(stickScramble+g_eeGeneral.stickMode*4+value)+1 ;
}

const uint8_t csTypeTable[] =
{ CS_VOFS, CS_VOFS, CS_VOFS, CS_VOFS, CS_VBOOL, CS_VBOOL, CS_VBOOL,
 CS_VCOMP, CS_VCOMP, CS_VCOMP, CS_VCOMP, CS_VBOOL, CS_VBOOL, CS_TIMER, CS_VOFS
} ;

uint32_t get_tmr10ms()
{

    return g_tmr10ms ;
}

uint8_t CS_STATE( uint8_t x)
{
	return *(csTypeTable+x-1) ;

}

MixData *mixaddress( uint8_t idx )
{
    return &g_model.mixData[idx] ;
}


uint8_t throttleReversed()
{
	return g_model.throttleReversed ^	g_eeGeneral.throttleReversed ;
}

void putsChnRaw(uint8_t x,uint8_t y,uint8_t idx,uint8_t att)
{
	uint8_t chanLimit = NUM_XCHNRAW ;
	uint8_t mix = att & MIX_SOURCE ;
	if ( mix )
	{
		chanLimit += MAX_GVARS + 1 + 1 ;
		att &= ~MIX_SOURCE ;
	}
    if(idx==0)
        lcd_putsnAtt(x,y,Str_Hyphens,4,att);
    else if(idx<=4)
		{
			if ( g_model.useCustomStickNames )
			{
				lcd_putsnAtt( x, y, ( char *)g_eeGeneral.customStickNames+4*(idx-1), 4, att|BSS ) ;
			}
			else
			{
        lcd_putsAttIdx(x,y,modi12x3,(idx-1),att) ;
			}
		}
    else if(idx<=chanLimit)
        lcd_putsAttIdx(x,y,Str_Chans_Gv,(idx-5),att);

    else
		{
			if ( mix )
			{
				idx += TEL_ITEM_SC1-(chanLimit-NUM_XCHNRAW) ;
			}
  	  lcd_putsAttIdx(x,y,Str_telemItems,(idx-NUM_XCHNRAW),att);
		}
}

void putsChn(uint8_t x,uint8_t y,uint8_t idx1,uint8_t att)
{
	putsChnRaw( x, y, idx1 ? idx1+20 : idx1, att ) ;
}


void putsMomentDrSwitches(uint8_t x,uint8_t y,int8_t idx1,uint8_t att)
{
	if ( idx1 > TOGGLE_INDEX )
	{
		lcd_putcAtt(x+3*FW,  y,'m',att);
		idx1 -= TOGGLE_INDEX  ;
	}
  putsDrSwitches( x-1*FW, y, idx1, att ) ;
}

void putsDrSwitches(uint8_t x,uint8_t y,int8_t idx1,uint8_t att)//, bool nc)
{

		switch(idx1){
    case  0:            lcd_putsAtt(x+FW,y,&Str_Hyphens[1],att);return;
    case  MAX_DRSWITCH: lcd_putsAtt(x+FW,y,Str_ON,att);return;
    case -MAX_DRSWITCH: lcd_putsAtt(x+FW,y,Str_OFF,att);return;
    }
		int8_t z ;
		z = idx1 ;
		if ( z < 0 )
		{
  		lcd_putcAtt(x,y, '!',att);
			z = -idx1 ;
		}
		z -= 1 ;
	if ( z > MAX_DRSWITCH )
	{
		z -= HSW_OFFSET ;
	}
  lcd_putsAttIdx(x+FW,y,Str_Switches,z,att) ;
}

void putsTmrMode(uint8_t x, uint8_t y, uint8_t attr, uint8_t type )
{ // Valid values of type are 0, 1 or 2 only

	TimerMode *ptConfig ;
  int8_t tm ;
  int8_t tmb ;
	if ( type & 0x80 )
	{
		ptConfig = &TimerConfig[1] ;
	}
	else
	{
		ptConfig = &TimerConfig[0] ;
	}
	tm = ptConfig->tmrModeA ;
  tmb = ptConfig->tmrModeB ;

	type &= 3 ;
	if ( type < 2 )		// 0 or 1
	{
	  if(tm<TMR_VAROFS)
		{
			lcd_putsnAtt(  x, y,(STR_TRIGA_OPTS)+3*tm,3,attr);
  	}
		else
		{
  		tm -= TMR_VAROFS - 7 ;
      lcd_putsAttIdx(  x, y, Curve_Str, tm, attr ) ;
			if ( tm < 9 + 7 )	// Allow for 7 offset above
			{
				x -= FW ;
			}
  		lcd_putcAtt(x+3*FW,  y,'%',attr);
		}
	}
	if ( ( type == 2 ) || ( ( type == 0 ) && ( tm == 1 ) ) )
	{
		putsMomentDrSwitches( x, y, tmb, attr );
	}
	asm("") ;
}


uint8_t putsTelemValue(uint8_t x, uint8_t y, int32_t val, uint8_t channel, uint8_t att) {

if (AFHDS2A_tel_status & ((uint64_t)1<< channel)){
  lcd_outdezNAtt(x, y, val, att, AFHDS2A_tel[channel].dig,AFHDS2A_tel[channel].prec);
} else{
    if( !(att & LEFT)){
      if(att & DBLSIZE)
        x -= 6 * FW;
      else
        x -= 3 * FW;
    }
    else
       x+=FW;
    lcd_putsAtt(x, y, "---", att);
}
  return 0;
}


int16_t getValue(uint8_t i)
{
    if(i<7) return calibratedStick[i];//-512..512
    if(i<PPM_BASE) return 0 ;
		else if(i<CHOUT_BASE)
		{
			int16_t x ;
			x = g_ppmIns[i-PPM_BASE] ;
			if(i<PPM_BASE+4)
			{
				x -= g_eeGeneral.trainer.calib[i-PPM_BASE] ;
			}
			return x*2;
		}
		else if(i<CHOUT_BASE+NUM_CHNOUT) return Ex_chans[i-CHOUT_BASE];
    else if(i<CHOUT_BASE+NUM_CHNOUT+NUM_TELEM_ITEMS)
		{
			return get_telemetry_value( i-CHOUT_BASE-NUM_CHNOUT ) ;
		}
    return 0;
}

bool getSwitch00( int8_t swtch )
{
	return getSwitch( swtch, 0, 0 ) ;
}

bool getSwitch(int8_t swtch, bool nc, uint8_t level)
{
    bool ret_value ;
    uint8_t cs_index ;

    switch(swtch){
    case  0:            return  nc;
    case  MAX_DRSWITCH: return  true;
    case -MAX_DRSWITCH: return  false;
    }


		if ( swtch > MAX_DRSWITCH )
		{
			return false ;
		}


    uint8_t dir = swtch>0;
    uint8_t aswtch = swtch ;
		if ( swtch < 0 )
		{
			aswtch = -swtch ;
		}

    if(aswtch<(MAX_DRSWITCH-NUM_CSW))
		{
			aswtch = keyState((EnumKeys)(SW_BASE+aswtch-1)) ;
			return !dir ? (!aswtch) : aswtch ;
    }

    //custom switch, Issue 78
    //use putsChnRaw
    //input -> 1..4 -> sticks,  5..8 pots
    //MAX,FULL - disregard
    //ppm


    cs_index = aswtch-(MAX_DRSWITCH-NUM_CSW);

		CSwData *cs = &g_model.customSw[cs_index];

		if(!cs->func) return false;

    if ( level>4 )
    {
    		ret_value = Last_switch[cs_index] & 1 ;
    		return swtch>0 ? ret_value : !ret_value ;
    }

    int8_t a = cs->v1;
    int8_t b = cs->v2;
    int16_t x = 0;
    int16_t y = 0;
		uint8_t valid = 1 ;

    // init values only if needed
    uint8_t s = CS_STATE(cs->func);

    if(s == CS_VOFS)
    {
        x = getValue(cs->v1-1);
            y = calc100toRESX(cs->v2);
    }
    else if(s == CS_VCOMP)
    {
        x = getValue(cs->v1-1);
        y = getValue(cs->v2-1);
    }

    switch ((uint8_t)cs->func) {
    case (CS_VPOS):
        ret_value = (x>y);
        break;
    case (CS_VNEG):
        ret_value = (x<y) ;
        break;
    case (CS_APOS):
    {
        ret_value = (abs(x)>y) ;
    }
    break;
    case (CS_ANEG):
    {
        ret_value = (abs(x)<y) ;
    }
    break;

    case (CS_AND):
    case (CS_OR):
    case (CS_XOR):
    {
        bool res1 = getSwitch(a,0,level+1) ;
        bool res2 = getSwitch(b,0,level+1) ;
        if ( cs->func == CS_AND )
        {
            ret_value = res1 && res2 ;
        }
        else if ( cs->func == CS_OR )
        {
            ret_value = res1 || res2 ;
        }
        else  // CS_XOR
        {
            ret_value = res1 ^ res2 ;
        }
    }
    break;

    case (CS_EQUAL):
        ret_value = (x==y);
        break;
    case (CS_NEQUAL):
        ret_value = (x!=y);
        break;
    case (CS_GREATER):
        ret_value = (x>y);
        break;
    case (CS_LESS):
        ret_value = (x<y);
        break;
    case (CS_TIME):
        ret_value = CsTimer[cs_index] >= 0 ;
        break;
		case (CS_LATCH) :
  	case (CS_FLIP) :
    	ret_value = Last_switch[cs_index] & 1 ;
	  break ;
    default:
        ret_value = false;
        break;
    }
		if ( valid == 0 )			// Catch telemetry values not present
		{
      ret_value = false;
		}
		if ( ret_value )
		{
			int8_t x ;
			x = cs->andsw ;
			if ( x )
			{
				if ( x > 8 )
				{
					x += 1 ;
				}
      	ret_value = getSwitch( x, 0, level+1) ;
			}
		}
		if ( cs->func < CS_LATCH )
		{
			Last_switch[cs_index] = ret_value ;
		}
    return swtch>0 ? ret_value : !ret_value ;

}


inline uint8_t keyDown()
{
    return (~PINB()) & 0x7E;
}

static void clearKeyEvents()
{
    while (keyDown())
		{
			wdt_reset() ; // loop until all keys are up
		}
    putEvent(0);
}

void check_backlight_voice()
{
	static uint8_t tmr10ms ;
	if(getSwitch00(g_eeGeneral.lightSw) || g_LightOffCounter)
		BACKLIGHT_ON ;
	else
		BACKLIGHT_OFF ;

	uint8_t x ;
	x = g_blinkTmr10ms ;
	if ( tmr10ms != x )
	{
		tmr10ms = x ;
		Voice.voice_process() ;
	}
}

uint16_t stickMoveValue()
{
#define INAC_DEVISOR 256   // Issue 206 - bypass splash screen with stick movement
	uint16_t sum = 0;
	for(uint8_t i=0; i<4; i++)
		sum += anaIn(i) ;
	sum += 128 ;
	return sum / INAC_DEVISOR ;
}


const uint8_t DoubleBits[] = {
	0x00, 0x03, 0x0C, 0x0F,
	0x30, 0x33, 0x3C, 0x3F,
	0xC0, 0xC3, 0xCC, 0xCF,
	0xF0, 0xF3, 0xFC, 0xFF } ;

static void doSplash()
{
    {


        check_backlight_voice() ;

        lcd_clear();

        //text splash screen
        /*---------------------------Kotello--------------------------------*/
        lcd_putsnAtt(4, 20, "GrinFlySky", 10, DBLSIZE);
        lcd_putsnAtt(3 * FW, 6 * FH, "PILOT-", 6, BSS);
        /*------------------------------------------------------------------*/

                                if (!g_eeGeneral.hideNameOnSplash)
                                  lcd_putsnAtt(9 * FW, 6 * FH, g_eeGeneral.ownerName,
                                      sizeof(g_eeGeneral.ownerName), BSS);

                                refreshDiplay();

                                clearKeyEvents();

        getADC_osmp();
        uint16_t inacSum = stickMoveValue();

        uint16_t tgtime = get_tmr10ms() + SPLASH_TIMEOUT;
        do
				{
        	refreshDiplay();
          check_backlight_voice() ;
            getADC_osmp();
            uint16_t tsum = stickMoveValue();

            if(keyDown() || (tsum!=inacSum))   return;  //wait for key release

        } while(tgtime != get_tmr10ms()) ;
    }
}

static void checkMem()
{
    if(g_eeGeneral.disableMemoryWarning) return;
    if(EeFsGetFree() < 200)
    {
        alert((STR_EE_LOW_MEM));
    }

}

#define	ALERT_TYPE	0
#define MESS_TYPE	1
#define	ALERT_SKIP	2
#define	ALERT_VOICE	4

void almess( const  char * s, uint8_t type )
{
	const char *h ;
  lcd_clear();
  lcd_puts_Pleft(4*FW,s);

	if ( type & ALERT_VOICE)
	{
		audioVoiceDefevent(AU_ERROR, V_ALERT);
	}

	if ( ( type & 1 ) == ALERT_TYPE)
	{
		if ( type & 2 )
		{
    	lcd_puts_Pleft(6*FH,(STR_PRESS_KEY_SKIP) ) ;
		}
		else
		{
    	lcd_puts_P(64-6*FW,7*FH,(STR_PRESS_ANY_KEY));
		}
		h = Str_Alert ;
	}
	else
	{
		h = (STR_MESSAGE) ;
	}
  lcd_putsAtt(64-7*FW,0*FH, h,DBLSIZE);

}


void message(const char * s)
{
	almess( s, MESS_TYPE ) ;
	refreshDiplay() ;
}

void alert(const char * s)
{
	alertx( s, false ) ;
}

void alertx(const char * s, bool defaults)
{
	if ( Main_running )
	{
		AlertMessage = s ;
		return ;
	}
	almess( s, ALERT_TYPE | ALERT_VOICE ) ;
	refreshDiplay() ;

	lcdSetRefVolt(defaults ? LCD_NOMCONTRAST : g_eeGeneral.contrast);

    clearKeyEvents();
    while(1)
    {
        if(keyDown())
        {
				    clearKeyEvents() ;
            return;  //wait for key release
        }
        if(heartbeat == 0x3)
        {
            wdt_reset();
            heartbeat = 0;
        }

        if(defaults)
        	BACKLIGHT_ON ;
		    else
    	    BACKLIGHT_OFF ;
        check_backlight_voice() ;
    }

}


uint8_t checkThrottlePosition()
{
  uint8_t thrchn=(2-(g_eeGeneral.stickMode&1));//stickMode=0123 -> thr=2121
	int16_t v = scaleAnalog( anaIn(thrchn), thrchn ) ;

	if ( g_model.throttleIdle )
	{
		if ( abs( v ) < THRCHK_DEADBAND )
		{
			return 1 ;
		}
	}
	else
	{
  	if(v <= -RESX + THRCHK_DEADBAND )
  	{
  		return 1 ;
  	}
	}
	return 0 ;
}

void checkTHR()
{
    if(g_eeGeneral.disableThrottleWarning) return;

//    uint8_t thrchn=(2-(g_eeGeneral.stickMode&1));//stickMode=0123 -> thr=2121

		getADC_osmp();   // if thr is down - do not display warning at all


    if ( checkThrottlePosition() )
		{
			return ;
		}

    // first - display warning

		almess((STR_THR_NOT_IDLE "\037" STR_RST_THROTTLE), ALERT_SKIP | ALERT_VOICE ) ;
    refreshDiplay() ;
    clearKeyEvents();

    //loop until all switches are reset
    while (1)
    {
        getADC_osmp();
        check_backlight_voice() ;

        wdt_reset() ;

		    if ( checkThrottlePosition() )
				{
					return ;
				}
				if( keyDown() )
        {
			    clearKeyEvents() ;
          return ;
        }
    }
}

static void checkAlarm() // added by Gohst
{
    if(g_eeGeneral.disableAlarmWarning) return;
    if(!g_eeGeneral.beeperVal) alert((STR_ALARMS_DISABLE));
}

static void checkWarnings()
{
    if(sysFlags)
    {
        alert((STR_OLD_VER_EEPROM)); //will update on next save
        sysFlags = 0 ; //clear flag
    }
}

void putWarnSwitch( uint8_t x, uint8_t idx )
{
  lcd_putsAttIdx( x, 2*FH, Str_Switches, idx, 0) ;
}


uint8_t getCurrentSwitchStates()
{
  uint8_t i = 0 ;
  for( uint8_t j=0; j<8; j++ )
  {
    bool t=keyState( (EnumKeys)(SW_BASE_DIAG+7-j) ) ;
		i <<= 1 ;
    i |= t ;
  }
	return i ;
}

void checkSwitches()
{
	uint8_t warningStates ;

	if(g_eeGeneral.disableSwitchWarning) return; // if warning is on

	warningStates = g_model.switchWarningStates ;
    uint8_t x = warningStates & SWP_IL5;
    if(!(x==SWP_LEG1 || x==SWP_LEG2 || x==SWP_LEG3)) //legal states for ID0/1/2
    {
        warningStates &= ~SWP_IL5; // turn all off, make sure only one is on
        warningStates |=  SWP_ID0B;
				g_model.switchWarningStates = warningStates ;
    }

	uint8_t first = 1 ;
	uint8_t voice = 0 ;

    //loop until all switches are reset
    while (1)
    {
	getADC_osmp();
        uint8_t i = getCurrentSwitchStates() ;

        //show the difference between i and switch?
        //show just the offending switches.
        //first row - THR, GEA, AIL, ELE, ID0/1/2
        uint8_t x = i ^ warningStates ;


	almess((STR_SWITCH_WARN "\037" STR_RESET_SWITCHES), ALERT_SKIP | voice ) ;
	voice = 0 ;

        if(x & SWP_THRB)
            putWarnSwitch(2 + 0*FW, 0 );
        if(x & SWP_RUDB)
            putWarnSwitch(2 + 3*FW + FW/2, 1 );

				if(x & SWP_ELEB)
            putWarnSwitch(2 + 7*FW, 2 );

        if(x & SWP_IL5)
        {
            if(i & SWP_ID0B)
                putWarnSwitch(2 + 10*FW + FW/2, 3 );
            else if(i & SWP_ID1B)
                putWarnSwitch(2 + 10*FW + FW/2, 4 );
            else if(i & SWP_ID2B)
                putWarnSwitch(2 + 10*FW + FW/2, 5 );
        }

        if(x & SWP_AILB)
            putWarnSwitch(2 + 14*FW, 6 );
        if(x & SWP_GEAB)
            putWarnSwitch(2 + 17*FW + FW/2, 7 );


        refreshDiplay();

				if ( first )
				{
					voice = ALERT_VOICE ;
    			clearKeyEvents();
					first = 0 ;
				}

        if( (i==warningStates) || (keyDown())) // check state against settings
        {
            return;  //wait for key release
        }

        check_backlight_voice() ;
        wdt_reset() ;

    }


}


static void checkQuickSelect()
{
    uint8_t i = keyDown(); //check for keystate

		if ( ( i & 6 ) == 6 )
		{
			SystemOptions |= SYS_OPT_MUTE ;
			return ;
		}

}

uint8_t StickScrollAllowed ;
uint8_t StickScrollTimer ;

MenuFuncP g_menuStack[5];

uint8_t  g_menuStackPtr = 0;
uint8_t  EnterMenu = 0 ;


uint8_t getFlightPhase()
{
	uint8_t i ;
	PhaseData *phase = &g_model.phaseData[0];

  for ( i = 0 ; i < MAX_MODES ; i += 1 )
	{
    if ( phase->swtch && getSwitch00( phase->swtch ) )
		{
      return i + 1 ;
    }
		phase += 1 ;
  }
  return 0 ;
}


int16_t getRawTrimValue( uint8_t phase, uint8_t idx )
{
	if ( phase )
	{
		return g_model.phaseData[phase-1].trim[idx] + TRIM_EXTENDED_MAX + 1 ;
	}
	else
	{
		return g_model.trim[idx] ;
	}
}

uint8_t getTrimFlightPhase( uint8_t phase, uint8_t idx )
{
  for ( uint8_t i=0 ; i<MAX_MODES ; i += 1 )
	{
    if (phase == 0) return 0;
    int16_t trim = getRawTrimValue( phase, idx ) ;
    if ( trim <= TRIM_EXTENDED_MAX )
		{
			return phase ;
		}
    uint8_t result = trim-TRIM_EXTENDED_MAX-1 ;
    if (result >= phase)
		{
			result += 1 ;
		}
    phase = result;
  }
  return 0;
}


int16_t getTrimValue( uint8_t phase, uint8_t idx )
{
  return getRawTrimValue( getTrimFlightPhase( phase, idx ), idx ) ;
}

int16_t validatePlusMinus125( int16_t trim )
{
  if(trim > 125)
	{
		return 125 ;
	}
  if(trim < -125 )
	{
		return -125 ;
	}
	return trim ;
}

void setTrimValue(uint8_t phase, uint8_t idx, int16_t trim)
{
	if ( phase )
	{
		phase = getTrimFlightPhase( phase, idx ) ;
	}
	trim = validatePlusMinus125( trim ) ;
	if ( phase )
	{
  	g_model.phaseData[phase-1].trim[idx] = trim - ( TRIM_EXTENDED_MAX + 1 ) ;
	}
	else
	{
		g_model.trim[idx] = trim ;
	}
  STORE_MODELVARS_TRIM ;
}


static uint8_t checkTrim(uint8_t event)
{
    int8_t  k = (event & EVT_KEY_MASK) - TRM_BASE;
    int8_t  s = g_model.trimInc;
    if ( s == 4 )
    {
            s = 8 ;			  // 1=>1  2=>2  3=>4  4=>8
    }
    else
    {
            if ( s == 3 )
            {
                    s = 4 ;			  // 1=>1  2=>2  3=>4  4=>8
            }
    }

    if( (k>=0) && (k<8) && !IS_KEY_BREAK(event)) // && (event & _MSK_KEY_REPT))
    {
        //LH_DWN LH_UP LV_DWN LV_UP RV_DWN RV_UP RH_DWN RH_UP
        uint8_t idx = (uint8_t)k/2;

// SORT idx for stickmode
        idx = modeFixValue( idx ) - 1 ;
        if ( g_eeGeneral.crosstrim )
        {
                idx = 3 - idx ;
        }
        uint8_t phaseNo = getTrimFlightPhase( CurrentPhase, idx ) ;
        int16_t tm = getTrimValue( phaseNo, idx ) ;
        int8_t  v = (s==0) ? (abs(tm)/4)+1 : s;
        bool thrChan = (2 == idx) ;

        bool thro = false ;
        if ( thrChan )
        {
                if ( g_model.thrTrim )
                {
                        thro = true ;
                        v = 2 ; // if throttle trim and trim trottle then step=2
                }
                if(throttleReversed())
                {
                        v = -v;  // throttle reversed = trim reversed
                }
        }


        int16_t x = (k&1) ? tm + v : tm - v;   // positive = k&1

        if(((x==0)  ||  ((x>=0) != (tm>=0))) && (!thro) && (tm!=0)){
						setTrimValue( phaseNo, idx, 0 ) ;
            killEvents(event);
            audioDefevent(AU_TRIM_MIDDLE);

        } else if(x>-125 && x<125){
              setTrimValue( phaseNo, idx, x ) ;

              int8_t t = x ;
              if ( t < 0 )
              {
                      t = -t ;
              }
              t /= 4 ;
              audioEvent(AU_TRIM_MOVE,t+60) ;
        }
        else
        {
            setTrimValue( phaseNo, idx, (x>0) ? 125 : -125 ) ;
            if(x <= 125 && x >= -125)
            {
              int8_t t = x ;
              if ( t > 0 )
              {
                      t = -t ;
              }
              t /= 4 ;
              audioEvent(AU_TRIM_MOVE,(t+60));
            }
        }

        return 0;
    }
    return event;
}


struct t_p1 P1values ;
static uint8_t LongMenuTimer ;
uint8_t StepSize ;

int16_t checkIncDec16( int16_t val, int16_t i_min, int16_t i_max, uint8_t i_flags)
{
    int16_t newval = val;
    uint8_t kpl=KEY_RIGHT, kmi=KEY_LEFT, kother = -1;

    uint8_t event = Tevent ;

    if(event==EVT_KEY_FIRST(kpl) || event== EVT_KEY_REPT(kpl) || (s_editMode && (event==EVT_KEY_FIRST(KEY_UP) || event== EVT_KEY_REPT(KEY_UP))) )
    {
        if ( ( read_keys() & 2 ) == 0 )
        {
            newval += StepSize ;
        }
        else
        {
            newval += 1 ;
        }

        audioDefevent(AU_KEYPAD_UP);

        kother=kmi;
    }
    else if(event==EVT_KEY_FIRST(kmi) || event== EVT_KEY_REPT(kmi) || (s_editMode && (event==EVT_KEY_FIRST(KEY_DOWN) || event== EVT_KEY_REPT(KEY_DOWN))) )
    {
        if ( ( read_keys() & 2 ) == 0 )
        {
             newval -= StepSize ;
        }
        else
        {
              newval -= 1 ;
        }

        audioDefevent(AU_KEYPAD_DOWN);

        kother=kpl;
    }
    if((kother != (uint8_t)-1) && keyState((EnumKeys)kother)){
        newval=-val;
        killEvents(kmi);
        killEvents(kpl);
    }
    if(i_min==0 && i_max==1)
    {
        if (event==EVT_KEY_FIRST(KEY_MENU) || event==EVT_KEY_BREAK(BTN_RE))
        {
            s_editMode = false;
            newval=!val;
            killEvents(event);
            if ( event==EVT_KEY_BREAK(BTN_RE) )
            {
                    RotaryState = ROTARY_MENU_UD ;
            }
            event = 0 ;
        }
        else
        {
                newval &= 1 ;
        }
    }


    //change values based on P1
    newval -= P1values.p1valdiff;
    if ( RotaryState == ROTARY_VALUE )
    {
            newval += ( ( read_keys() & 2 ) == 0 ) ? 20 * Rotary.Rotary_diff : Rotary.Rotary_diff ;
    }
    if(newval>i_max)
    {
        newval = i_max;
        killEvents(event);
        audioDefevent(AU_KEYPAD_UP);
    }
    else if(newval < i_min)
    {
        newval = i_min;
        killEvents(event);
        audioDefevent(AU_KEYPAD_DOWN);

    }
    if(newval != val)
    {
        if ( menuPressed() )
        {
                LongMenuTimer = 255 ;
        }
        if(newval==0) {
          	  pauseEvents(event);

            if (newval>val){
                audioDefevent(AU_KEYPAD_UP);
            } else {
                audioDefevent(AU_KEYPAD_DOWN);
            }

        }
        eeDirty(i_flags & (EE_GENERAL|EE_MODEL));
    }
    return newval;
}

extern uint8_t EditType ;

NOINLINE int8_t checkIncDec( int8_t i_val, int8_t i_min, int8_t i_max, uint8_t i_flags)
{
    return (int8_t)checkIncDec16( i_val,i_min,i_max,i_flags);
}

int8_t checkIncDec_i8( int8_t i_val, int8_t i_min, int8_t i_max)
{
    return checkIncDec( i_val,i_min,i_max,EditType);
}

int8_t checkIncDec_0( int8_t i_val, int8_t i_max)
{
    return checkIncDec( i_val,0,i_max,EditType) ;
}

int16_t checkIncDec_u0( int16_t i_val, uint8_t i_max)
{
  return checkIncDec16( i_val,0,i_max,EditType) ;
}


void popMenu(bool uppermost)
{
    if(g_menuStackPtr>0 || uppermost)
    {

        if(g_menuStack[g_menuStackPtr] ==  menuProcProtocol)
        {
        }
        g_menuStackPtr = uppermost ? 0 : g_menuStackPtr-1;
				EnterMenu = EVT_ENTRY_UP ;
    }
}

void chainMenu(MenuFuncP newMenu)
{
    g_menuStack[g_menuStackPtr] = newMenu;
		EnterMenu = EVT_ENTRY ;
}
void pushMenu(MenuFuncP newMenu)
{

    if(g_menuStackPtr >= DIM(g_menuStack)-1)
    {
        alert((STR_MSTACK_OFLOW));
        return;
    }
    EnterMenu = EVT_ENTRY ;
    g_menuStack[++g_menuStackPtr] = newMenu;
}

uint8_t  g_vbat100mV ;
volatile uint8_t tick10ms = 0;
uint16_t g_LightOffCounter;
uint8_t  stickMoved = 0;

inline bool checkSlaveMode()
{
    // no power -> only phone jack = slave mode

    static bool lastSlaveMode = false;

    static uint8_t checkDelay = 0;
    if (audio.busy()) {
        checkDelay = 20;
    }
    else if (checkDelay) {
        --checkDelay;
    }
    else {
        lastSlaveMode = SLAVE_MODE;//
    }
    return (SlaveMode = lastSlaveMode) ;
}


void backlightKey()
{
  uint8_t a = g_eeGeneral.lightAutoOff ;
  uint16_t b = a * 250 ;
  b <<= 1 ;				// b = a * 500, but less code
  if(b>g_LightOffCounter) g_LightOffCounter = b;
}

void doBackLightVoice(uint8_t evt)
{
    uint8_t a = 0;
    uint16_t b ;
    uint16_t lightoffctr ;
    if(evt) backlightKey() ; // on keypress turn the light on 5*100

    lightoffctr = g_LightOffCounter ;
    if(lightoffctr) lightoffctr--;
    if(stickMoved)
    {
          a = g_eeGeneral.lightOnStickMove ;
          b = a * 250 ;
          b <<= 1 ;				// b = a * 500, but less code
          if(b>lightoffctr) lightoffctr = b;
    }
    g_LightOffCounter = lightoffctr ;
    check_backlight_voice();
}


const static uint8_t rate[8] = { 0, 0, 100, 40, 16, 7, 3, 1 } ;

uint8_t calcStickScroll( uint8_t index )
{
	uint8_t direction ;
	int8_t value ;

	if ( ( g_eeGeneral.stickMode & 1 ) == 0 )
	{
		index ^= 3 ;
	}

	value = phyStick[index] ;
	value /= 8 ;

	direction = value > 0 ? 0x80 : 0 ;
	if ( value < 0 )
	{
		value = -value ;			// (abs)
	}
	uint8_t temp = value ;	// Makes the compiler save 4 bytes flash
	if ( temp > 7 )
	{
		temp = 7 ;
	}
	value = *(rate+temp) ;
	if ( value )
	{
		StickScrollTimer = STICK_SCROLL_TIMEOUT ;		// Seconds
	}
	return value | direction ;
}


struct t_inactivity Inactivity = {0} ;
extern uint8_t s_noHi ;
extern void timer() ;
extern void trace() ;

static void inactivityCheck()
{
	struct t_inactivity *PtrInactivity = &Inactivity ;
	FORCE_INDIRECT(PtrInactivity) ;

  if(s_noHi) s_noHi--;
  uint16_t tsum = stickMoveValue() ;
  if (tsum != PtrInactivity->inacSum)
	{
    PtrInactivity->inacSum = tsum;
    PtrInactivity->inacCounter = 0;
    stickMoved = 1;  // reset in perMain
  }
  else
  {
        uint8_t timer = g_eeGeneral.inactivityTimer + 10 ;
        if( ( timer) && (g_vbat100mV>49))
  	{
            if (++PtrInactivity->inacPrescale > 15 )
            {
            	PtrInactivity->inacCounter++;
            	PtrInactivity->inacPrescale = 0 ;
            	if(PtrInactivity->inacCounter>(uint16_t)((timer)*(100*60/16)))
                  if((PtrInactivity->inacCounter&0x1F)==1)
                  {
                    SystemOptions &= ~SYS_OPT_MUTE;
                    setVolume(NUM_VOL_LEVELS-2) ;		// Nearly full volume
                    audioVoiceDefevent( AU_INACTIVITY, V_INACTIVE ) ;
                  }
            }
        }
  }
}


int8_t getGvarSourceValue( uint8_t src )
{
	int16_t value = 0 ;

	if ( src <= 4 )
	{
		value = getTrimValue( CurrentPhase, src - 1 ) ;
	}
	else if ( src <= 9 )	// Stick
	{
		value = calibratedStick[ src-5 - 1 ] / 8 ;
	}
	else if ( src <= 12 )	// Pot
	{
		value = calibratedStick[ ( src-6)] / 8 ;
	}
	else if ( src <= 28 )	// Chans
	{
		value = Ex_chans[src-13] / 10 ;
	}
	return validatePlusMinus125( value ) ; // limit( -125, value, 125 ) ;
}

static void perMain()
{
    static uint32_t lastTMR;
    uint32_t t10ms;
    t10ms =  g_tmr10ms;
    tick10ms = t10ms - lastTMR;
    lastTMR = t10ms;

    perOutPhase(g_chans512, 0);

    if(tick10ms == 0) return ; //make sure the rest happen only every 10ms.

    inactivityCheck() ;
    timer() ;
    trace(); //trace thr 0..32  (/32)

    if ( ppmInAvailable )
    {
       ppmInAvailable -= 1 ;
    }

    eeCheck();

    // Every 10mS update backlight output to external latch
    // Note: LcdLock not needed here as at tasking level

    lcd_clear();
    uint8_t evt=getEvent();


    evt = checkTrim(evt);
    if ( ( evt == 0 ) || ( evt == EVT_KEY_REPT(KEY_MENU) ) )
    {
      uint8_t timer = LongMenuTimer ;
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
      if ( timer == 200 )
      {
        evt = EVT_TOGGLE_GVAR ;
        timer = 255 ;
      }
      LongMenuTimer = timer ;
    }

    int16_t p1d ;

    struct t_p1 *ptrp1 ;
    ptrp1 = &P1values ;
    FORCE_INDIRECT(ptrp1) ;

    int16_t c6 = calibratedStick[6] ;
    p1d = ( ptrp1->p1val-c6 )/32;
    if(p1d) {
        p1d = (ptrp1->p1valprev-c6)/2;
        ptrp1->p1val = c6 ;
    }
    ptrp1->p1valprev = c6 ;
    if ( g_eeGeneral.disablePotScroll || (scroll_disabled) )
    {
       p1d = 0 ;
    }
    ptrp1->p1valdiff = p1d ;

    struct t_rotary *protary = &Rotary ;
    FORCE_INDIRECT(protary) ;
    {
        int8_t x ;
        x = protary->RotCount - protary->LastRotaryValue ;
        if ( x == -1 )
        {
            x = 0 ;
        }
        protary->Rotary_diff = ( x ) / 2 ;
        protary->LastRotaryValue += protary->Rotary_diff * 2 ;
    }

    doBackLightVoice( evt | protary->Rotary_diff ) ;
    // Handle volume
    uint8_t requiredVolume ;
    requiredVolume = g_eeGeneral.volume+(NUM_VOL_LEVELS-1) ;

    if ( ( g_menuStack[g_menuStackPtr] == menuProc0) && ( PopupData.PopupActive == 0 ) )
    {
        if ( protary->Rotary_diff )
        {
            int16_t x = protary->RotaryControl ;
            x += protary->Rotary_diff ;
            protary->RotaryControl = validatePlusMinus125( x ) ;
            protary->Rotary_diff = 0 ;
        }

        if ( g_model.anaVolume )	// Only check if on main screen
        {
            uint16_t v ;
            uint16_t divisor ;
            if ( g_model.anaVolume < 4 )
            {
                v = calibratedStick[g_model.anaVolume+3] + 1024 ;
                divisor = 2048 ;
            }
            else
            {

                v = g_model.gvars[g_model.anaVolume-4+3].gvar + 125 ;	// +3 to get to GV4-GV7
                divisor = 250 ;
            }
             requiredVolume = v * (NUM_VOL_LEVELS-1) / divisor ;
        }
    }
    if ( requiredVolume != CurrentVolume )
    {
        setVolume( requiredVolume ) ;
    }

    if ( g_eeGeneral.stickScroll && StickScrollAllowed )
    {
        if ( StickScrollTimer )
        {
          static uint8_t repeater ;
          uint8_t direction ;
          uint8_t value ;

          if ( repeater < 128 )
          {
            repeater += 1 ;
          }
          value = calcStickScroll( 2 ) ;
          direction = value & 0x80 ;
          value &= 0x7F ;
          if ( value )
          {
              if ( repeater > value )
              {
                repeater = 0 ;
                if ( evt == 0 )
                {
                      if ( direction )
                      {
                              evt = EVT_KEY_FIRST(KEY_UP) ;
                      }
                      else
                      {
                              evt = EVT_KEY_FIRST(KEY_DOWN) ;
                      }
                }
              }
          }
          else
          {
            value = calcStickScroll( 3 ) ;
            direction = value & 0x80 ;
            value &= 0x7F ;
            if ( value )
            {
                if ( repeater > value )
                {
                      repeater = 0 ;
                      if ( evt == 0 )
                      {
                              if ( direction )
                              {
                                      evt = EVT_KEY_FIRST(KEY_RIGHT) ;
                              }
                              else
                              {
                                      evt = EVT_KEY_FIRST(KEY_LEFT) ;
                              }
                      }
                }
            }
         }
      }
    }
    else
    {
       StickScrollTimer = 0 ;		// Seconds
    }

    StickScrollAllowed = 1 ;

    for( uint8_t i = 0 ; i < MAX_GVARS ; i += 1 )
    {
        if ( g_model.gvars[i].gvsource )
        {
          int16_t value ;
          uint8_t src = g_model.gvars[i].gvsource ;


          if ( src == 5 )	// REN
          {
                value = Rotary.RotaryControl ;
          }
          // The following is to action GVAR adjustment
          else
          {
              value = getGvarSourceValue( src ) ;
          }
          g_model.gvars[i].gvar = validatePlusMinus125( value ) ; // limit( -125, value, 125 ) ;
        }
    }


    static uint8_t alertKey ;
    if ( AlertMessage )
    {
        almess( AlertMessage, ALERT_TYPE ) ;
        uint8_t key = keyDown() ;
        if ( alertKey )
        {
          if( key == 0 )
          {
             AlertMessage = 0 ;
          }
        }
        else if ( key )
        {
           alertKey = 1 ;
        }
    }
    else
    {
        alertKey = 0 ;

        if ( EnterMenu )
        {
            evt = EnterMenu ;
            EnterMenu = 0 ;
            audioDefevent(AU_MENUS);
        }
        StepSize = 20 ;
        Tevent = evt ;

        g_menuStack[g_menuStackPtr](evt);
    }
    refreshDiplay();

    switch( g_blinkTmr10ms & 0x1f ) { //alle 10ms*32

          case 2:
          {
            //TODO check v-bat
            //        Calculation By Mike Blandford
            //        Resistor divide on battery voltage is 5K1 and 2K7 giving a fraction of 2.7/7.8
            //        If battery voltage = 10V then A2D voltage = 3.462V
            //        11 bit A2D count is 1417 (3.462/5*2048).
            //        1417*18/256 = 99 (actually 99.6) to represent 9.9 volts.
            //        Erring on the side of low is probably best.

            int16_t ab = anaIn(7);
            ab = ab * 16 + ab / 8 * g_eeGeneral.vBatCalib;
            ab = (uint16_t) ab / 330;
            g_vbat100mV = (ab + g_vbat100mV + 1) >> 1 ;  // Filter it a bit => more stable display

            static uint8_t s_batCheck;
            s_batCheck+=16 ;
            if((s_batCheck == 0) && (g_vbat100mV < g_eeGeneral.vBatWarn) /*&& (g_vbat100mV>49)*/)
            {

                 audioVoiceDefevent(AU_TX_BATTERY_LOW, V_BATTERY_LOW);
                voice_numeric(g_vbat100mV,1, V_VOLTS);
                if (g_eeGeneral.flashBeep)
                   g_LightOffCounter = FLASH_DURATION;
            }
          }
          break;
    }


    stickMoved = 0; //reset this flag

}

int16_t g_ppmIns[8];

uint16_t s_anaFilt[8] ;
uint16_t anaIn(uint8_t chan)
{
    // ana-in:   3 1 2 0 4 5 6 7
	uint8_t pchan = chan ;
	if ( chan == 3 )
	{
		pchan = 0 ;
	}
	else
	{
		if ( chan == 0 )
		{
			pchan = 3 ;
		}
	}
        uint16_t temp = s_anaFilt[pchan] ;
	if ( chan < 4 )	// A stick
	{
		if ( g_eeGeneral.stickReverse & ( 1 << chan ) )
		{
			temp = 2048 - temp ;
		}
	}
	return temp ;
}


volatile uint16_t g_tmr16KHz;
volatile uint16_t tmrEEPROM;


/*----------------------------------------------------------------------------*/
// Clocks every 128 uS
void ISR_TIMER2_OVF_vect(void) {
  if (audio.busy()) {
    AUDIO_DRIVER(); // the tone generator
  }
}
/*----------------------------------------------------------------------------*/
// Clocks every 10 mS
void ISR_TIMER0_COMP_vect(void) // 10ms timer
{

  AUDIO_HEARTBEAT(); // the queue processing

  per10ms();
  heartbeat |= HEART_TIMER10ms;
  // See if time for alarm checking
  struct t_alarmControl *pac = &AlarmControl;
  FORCE_INDIRECT(pac);

  if (--pac->AlarmTimer == 0) {
    pac->AlarmTimer = 100; // Restart timer
    pac->OneSecFlag = 1;
  }
  if (--pac->VoiceFtimer == 0) {
    pac->VoiceFtimer = 10;    // Restart timer
    pac->VoiceCheckFlag |= 1; // Flag time to check alarms
  }

  //  } // end 10ms event
}
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
extern struct t_latency g_latency ;


void mainER(void)
{


	{
		serialVoiceInit(on_voice_cb) ;

	}


    sei(); //damit alert in eeReadGeneral() nicht haengt

    g_menuStack[0] =  menuProc0;

    if (eeReadGeneral())
    {
        lcd_init() ;   // initialize LCD module after reading eeprom
    }
    else
    {
        eeGeneralDefault(); // init g_eeGeneral with default values
        lcd_init();         // initialize LCD module for ALERT box
        eeWriteGeneral();   // format/write back to eeprom
    }


      uint8_t in ;
	while ( (in = ~PIND() & 0xC3) == 0x40 )
	{
		SystemOptions = SYS_OPT_HARDWARE_EDIT ;

	}

        if ( in == 0x82 )
	{
		cli() ;
		lcd_puts_Pleft( FH, PSTR("\005Stopped") ) ;
		refreshDiplay() ;
		for ( ;; )	// This allows the Megasound board to 'talk' to a PC over serial
		{
		}

	}

	uint8_t cModel = g_eeGeneral.currModel;
	eeLoadModel( cModel ) ;


       checkQuickSelect();


        stickMoved = 1 ;
        doBackLightVoice(1) ;
        stickMoved = 0 ;

 // moved here and logic added to only play statup tone if splash screen enabled.
 // that way we save a bit, but keep the option for end users!
    setVolume(g_eeGeneral.volume + NUM_VOL_LEVELS-1) ;


    if(!g_eeGeneral.disableSplashScreen)
    {
        if( g_eeGeneral.speakerMode )		// Not just beeper
        {
                              audioVoiceDefevent( AU_TADA, V_HELLO ) ;
        }
        doSplash();

        checkMem();
        getADC_osmp();
        g_vbat100mV = anaIn(7) / 14 ;
        checkTHR();
        checkSwitches();
        checkAlarm();
        checkWarnings();
        clearKeyEvents(); //make sure no keys are down before proceeding
    	putVoiceQueueUpper( g_model.modelVoice ) ;
    }

    AlarmControl.VoiceCheckFlag |= 2 ;// Set switch current states
    CurrentPhase = 0 ;
    perOutPhase(g_chans512, 0 ) ;
    startPulses() ;
    wdt_enable(WDTO_500MS);

    g_menuStack[1] = menuProcModelSelect ;	// this is so the first instance of [MENU LONG] doesn't freak out!

    Main_running = 1 ;
    while(1){
        mainSequence() ;
    }
}


uint8_t isAgvar(uint8_t value)
{
	if ( value >= 62 )
	{
		if ( value <= 68 )
		{
			return 1 ;
		}
	}
	return 0 ;
}


void doVoiceAlarmSource( VoiceAlarmData *pvad )
{
	if ( pvad->source )
	{
		// SORT OTHER values here
		if ( pvad->source >= NUM_XCHNRAW )
		{
			voice_telem_item( pvad->source - NUM_XCHNRAW - 1 ) ;
		}
		else
		{
			int16_t value ;
			value = getValue( pvad->source - 1 ) ;
			voice_numeric( value, 0, 0 ) ;
		}
	}
}

void procOneVoiceAlarm(VoiceAlarmData *pvad, uint8_t i) {
  uint8_t curent_state;
  uint8_t play = 0;
  curent_state = 0;
  int16_t ltimer = Nvs_timer[i];

  if (pvad->func) // Configured
  {
    int16_t x;
    int16_t y = pvad->offset;
    x = getValue(pvad->source - 1);
    switch (pvad->func) {
    case 1:
      x = x > y;
      break;
    case 2:
      x = x < y;
      break;
    case 3:
    case 4:
      x = abs(x);
      x = (pvad->func == 3) ? x > y : x < y;
      break;
    case 5: {
      if (isAgvar(pvad->source)) {
        x *= 10;
        y *= 10;
      }
      x = abs(x - y) < 32;
    } break;
    case 6:
      x = x == y;
      break;
    }
    if (pvad->swtch) {
      if (getSwitch00(pvad->swtch) == 0) {
        x = 0;
      }
    }
    if (x == 0) {
      ltimer = 0;
    } else {
      play = 1;
    }
  } else // No function
  {
    if (pvad->swtch) {
      curent_state = getSwitch00(pvad->swtch);
      if (curent_state == 0) {
        ltimer = -1;
      }
    } else                 // No switch, no function
    {                      // Check for source with numeric rate
      if (pvad->rate >= 4) // A time
      {
        if (pvad->vsource) {
          play = 1;
        }
      }
    }
  }
  play |= curent_state;

  uint8_t l_nvsState = Nvs_state[i];

  if ((AlarmControl.VoiceCheckFlag & 2) == 0) {
    if (pvad->rate == 3) // All
    {
      uint8_t pos = switchPosition(pvad->swtch);
      if (l_nvsState != pos) {
        l_nvsState = pos;
        ltimer = 0;
        play = pos + 1;
      } else {
        play = 0;
      }
    } else {
      if (play == 1) {
        if (l_nvsState == 0) {                          // just turned ON
          if ((pvad->rate == 0) || (pvad->rate == 2)) { // ON
            ltimer = 0;
          }
        }
        l_nvsState = 1;
        if (pvad->rate == 1) {
          play = 0;
        }
      } else {
        if (l_nvsState == 1) {
          if ((pvad->rate == 1) || (pvad->rate == 2)) {
            ltimer = 0;
            play = 1;
            if (pvad->rate == 2) {
              play = 2;
            }
          }
        }
        l_nvsState = 0;
      }
      if (pvad->rate == 33) {
        play = 0;
        ltimer = -1;
      }
    }
  } else {
    l_nvsState = play;
    play = (pvad->rate == 33) ? 1 : 0;
    ltimer = -1;
  }

  Nvs_state[i] = l_nvsState;

  if (pvad->mute) {
    if (pvad->source > (CHOUT_BASE + NUM_CHNOUT)) { // Telemetry item
      if (!telemItemValid(pvad->source - 1 - CHOUT_BASE - NUM_CHNOUT)) {
        play = 0; // Mute it
      }
    }
  }

  if (play) {
    if (ltimer < 0) {
      if (pvad->rate >= 3) // A time or ONCE
      {
        ltimer = 0;
      }
    }
    if (ltimer == 0) {
      if (pvad->vsource == 1) {
        doVoiceAlarmSource(pvad);
      }
      if (pvad->fnameType == 0) // None
      {
        // Nothing!
      } else if (pvad->fnameType == 1) // Name
      {
        uint16_t value = pvad->vfile;
        if (value > 507) {
          value = calc_scaler(value - 508, 0, 0);
        } else if (value > 500) {
          value = g_model.gvars[value - 501].gvar;
        }
        putVoiceQueueLong(value + (play - 1));
      } else { // Audio
        audioEvent(pvad->vfile, 0);
      }
      if (pvad->vsource == 2) {
        doVoiceAlarmSource(pvad);
      }
      if (pvad->haptic) {
        audioDefevent((pvad->haptic > 1) ? ((pvad->haptic == 3) ? AU_HAPTIC3 : AU_HAPTIC2) : AU_HAPTIC1);
      }
      if ((pvad->rate < 3) || (pvad->rate > 32)) // Not a time
      {
        ltimer = -1;
      } else {
        ltimer = 1;
      }
    } else if (ltimer > 0) {
      ltimer += 1;
      if (ltimer > ((pvad->rate - 2) * 10)) {
        ltimer = 0;
      }
    }
  }
  pvad += 1;
  Nvs_timer[i] = ltimer;
}

NOINLINE void processVoiceAlarms()
{
	uint8_t i ;
	VoiceAlarmData *pvad = &g_model.vad[0] ;

	for ( i = 0 ; i < NUM_VOICE_ALARMS ; i += 1 )
	{
		procOneVoiceAlarm( pvad, i ) ;
		pvad += 1 ;
	}
}


void mainSequence()
{
	CalcScaleNest = 0 ;

	uint16_t t0 = g_tmr16KHz;


  getADC_osmp() ;


  perMain();

  if(heartbeat == 0x3)
  {
      wdt_reset();
      heartbeat = 0;
  }
  t0 = g_tmr16KHz - t0;
  if ( t0 > g_latency.g_timeMain ) g_latency.g_timeMain = t0 ;

  if ( AlarmControl.VoiceCheckFlag )		// Every 100 mS
  {
		uint8_t i ;
		static uint16_t timer ;


		timer += 1 ;


		for ( i = 0 ; i < NUM_CSW ; i += 1 )
		{
			CSwData *cs = &g_model.customSw[i] ;
    	uint8_t cstate = CS_STATE(cs->func);

    	if(cstate == CS_TIMER)
			{
				int16_t y ;
				y = CsTimer[i] ;
				if ( y == 0 )
				{
					int8_t z ;
					z = cs->v1 ;
					if ( z >= 0 )
					{
						z = -z-1 ;
						y = z * 10 ;
					}
					else
					{
						y = z ;
					}
				}
				else if ( y < 0 )
				{
					if ( ++y == 0 )
					{
						int8_t z ;
						z = cs->v2 ;
						if ( z >= 0 )
						{
							z += 1 ;
							y = z * 10 - 1  ;
						}
						else
						{
							y = -z-1 ;
						}
					}
				}
				else  // if ( CsTimer[i] > 0 )
				{
					y -= 1 ;
				}
				if ( cs->andsw )
				{
					int8_t x ;
					x = cs->andsw ;
					if ( x > 8 )
					{
						x += 1 ;
					}
	        if (getSwitch00( x) == 0 )
				  {
						y = -1 ;
					}
				}
				CsTimer[i] = y ;
			}
			uint8_t lastSwitch = Last_switch[i] ;
			if ( cs->func == CS_LATCH )
			{
		    if (getSwitch00( cs->v1) )
				{
					lastSwitch = 1 ;
				}
				else
				{
			    if (getSwitch00( cs->v2) )
					{
						lastSwitch = 0 ;
					}
				}
			}
			if ( cs->func == CS_FLIP )
			{
		    if (getSwitch00( cs->v1) )
				{
					if ( ( lastSwitch & 2 ) == 0 )
					{
						// Clock it!
			      if (getSwitch00( cs->v2) )
						{
							lastSwitch = 3 ;
						}
						else
						{
							lastSwitch = 2 ;
						}
					}
				}
				else
				{
					lastSwitch &= ~2 ;
				}
			}
			Last_switch[i] = lastSwitch ;

		}

		processVoiceAlarms() ;
		AlarmControl.VoiceCheckFlag = 0 ;

  }

  if (AlarmControl.OneSecFlag) // Custom Switch Timers
  {


	// New switch voices
	// New entries, Switch, (on/off/both), voice file index

		AlarmControl.OneSecFlag = 0 ;

		if ( StickScrollTimer )
		{
			StickScrollTimer -= 1 ;
		}
	}
}


int16_t calc1000toRESX(int16_t x)  // improve calc time by Pat MacKenzie
{
    int16_t y = x>>5;
    x+=y;
    y=y>>2;
    x-=y;
    return x+(y>>2);
}

int8_t REG100_100(int8_t x)
{
	return REG( x, -100, 100 ) ;
}

int8_t REG(int8_t x, int8_t min, int8_t max)
{
  int8_t result = x;
  if (x >= 126 || x <= -126) {
    x = (uint8_t)x - 126;
    result = g_model.gvars[x].gvar ;
    if (result < min) {
      g_model.gvars[x].gvar = result = min;
    }
    else if (result > max) {
      g_model.gvars[x].gvar = result = max;
    }
  }
  return result;
}

uint8_t IS_EXPO_THROTTLE( uint8_t x )
{
	if ( g_model.thrExpo )
	{
		return IS_THROTTLE( x ) ;
	}
	return 0 ;
}

int16_t calc100toRESX(int8_t x)
{
    return ((x*41)>>2) - x/64;
}