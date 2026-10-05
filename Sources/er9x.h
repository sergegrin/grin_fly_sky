// Modified 2026 by sergegrin
/*
 * er9x.h
 *
 *  Created on: 10 . 2019 .
 *      Author: KOSTYA
 */

#ifndef SOURCES_ER9X_H_
#define SOURCES_ER9X_H_

#include <stdint.h>
#include <string.h>
//*****************************************************************************
#include "hal.h"

#define REMOVE_FROM_64FRSKY
#define FAILSAFE
#define NO_TEMPLATES
#define TELEMETRY
#define NOVOICE_SW
#define NOSAFETY_A_OR_V


extern void mainER(void);

#define pgm_read_adr(address_short)  *(address_short)
#define pgm_read_byte(adr)  *(adr)
#define pgm_read_word(adr)  *(adr)


#define prog_char    char
#define prog_uchar   unsigned char
#define prog_int8_t  int8_t
#define prog_uint8_t uint8_t
#define prog_uint16_t uint16_t
#define APM

#define PSTR(s) s
//
#define READBIT(A, B) ((A >> (B & 7)) & 1)
#define SETBIT(T, B, V) (T = V ? T | (1<<B) : T & ~(1<<B))

#define wdt_reset() //WDog1_Clear(WDog1_PRPH_BASE_ADDRESS)
#define wdt_enable(WDTO_500MS) //WDog1_Clear(WDog1_PRPH_BASE_ADDRESS)

extern uint16_t s_anaFilt[8] ;
extern void ISR_TIMER0_COMP_vect(void); //10ms timer
void eeprom_read_block (void *pointer_ram, const void *pointer_eeprom, size_t n);
//*****************************************************************************


#define VERSION3  1
#define VERSION4  1


#define SERIAL_VOICE         1
 #define CHECK_MOVED_SWITCH   1


#define SBUS_PROTOCOL	1

// Bits in SystemOptions
#define SYS_OPT_HARDWARE_EDIT	1
#define SYS_OPT_MUTE					2


/* Building an er9x hex for custom transmitter */
#define LCD_OTHER   0    /* turn on these if you know what you're doing */
#define LCD_EEPE    0


///opt/cross/avr/include/avr/pgmspace.h
#include <stddef.h>

#ifndef FORCEINLINE
#define FORCEINLINE inline __attribute__ ((always_inline))
#endif

#ifndef NOINLINE
#define NOINLINE __attribute__ ((noinline))
#endif

#define CPU_UINT	uint32_t


#define assert(x)
//disable whole pgmspace functionality for all avr-gcc because
//avr-gcc > 4.2.1 does not work anyway
//http://www.mail-archive.com/gcc-bugs@gcc.gnu.org/msg239240.html
//http://gcc.gnu.org/bugzilla/show_bug.cgi?id=34734
//
//Workarounds:
//
//PSTR is fixed below
//all prog_xx definitions must use APM explicitely


/*
#undef prog_void
#undef prog_char
#undef prog_uchar
#undef prog_int8_t
#undef prog_uint8_t
#undef prog_int16_t
#undef prog_uint16_t
#undef prog_int32_t
#undef prog_uint32_t


typedef void prog_void __attribute__((__progmem__));//,deprecated("prog_void type is deprecated.")));
typedef char prog_char __attribute__((__progmem__));//,deprecated("prog_char type is deprecated.")));
typedef unsigned char prog_uchar __attribute__((__progmem__));//,deprecated("prog_uchar type is deprecated.")));
typedef int8_t    prog_int8_t   __attribute__((__progmem__));//,deprecated("prog_int8_t type is deprecated.")));
typedef uint8_t   prog_uint8_t  __attribute__((__progmem__));//,deprecated("prog_uint8_t type is deprecated.")));
typedef int16_t   prog_int16_t  __attribute__((__progmem__));//,deprecated("prog_int16_t type is deprecated.")));
typedef uint16_t  prog_uint16_t __attribute__((__progmem__));//,deprecated("prog_uint16_t type is deprecated.")));
typedef int32_t   prog_int32_t  __attribute__((__progmem__));//,deprecated("prog_int32_t type is deprecated.")));
typedef uint32_t  prog_uint32_t __attribute__((__progmem__));//,deprecated("prog_uint32_t type is deprecated.")));

#undef PGM_P
#define PGM_P const prog_char *

#ifdef __cplusplus
#define APM __attribute__(( section(".progmem.data") ))
#undef PSTR
#define PSTR(s) (__extension__({const static prog_char APM __c[] = (s);&__c[0];}))
#endif

#define F_CPU 16000000UL  // 16 MHz
#define pgm_read_adr(address_short) pgm_read_word(address_short)

//#define eeprom_write_block eeWriteBlockCmp
*/
/*****************************************************************************/

/*****************************************************************************/


#include "file.h"
//
//                  elev                        thr
//                   LV                         RV
//                 2 ^                        4 ^
//                   1                          2
//                   |     rudd                 |     aile
//              <----X--3-> LH             <----X--0-> RH
//              6    |    7                1    |    0
//                   |                          |
//                 3 v                        5 v
//


//PORTA  7      6       5       4       3       2       1       0
//       O      O       O       O       O       O       O       O
//       ------------------------ LCD_DAT -----------------------
//
//PORTB  7      6       5       4       3       2       1       0
//       O      i       i       i       i       i       i       O
//       light  KEY_LFT KEY_RGT KEY_UP  KEY_DWN KEY_EXT KEY_MEN  PPM
//
//PORTC  7      6       5       4       3       2       1       0
//       -      -       O       O       O       O       O       -
//       FRSKY  FRSKY LCD_E   LCD_RNW  LCD_A0  LCD_RES LCD_CS1  NC
//
//PORTD  7      6       5       4       3       2       1       0
//       i      i       i       i       i       i       i       i
//     TRM_D_DWN _UP  TRM_C_DWN _UP   TRM_B_DWN _UP   TRM_A_DWN _UP
//
//PORTE  7      6       5       4       3       2       1       0
//       i      i       i       i       O       i       i       i
//     PPM_IN  ID2    Trainer  Gear   Buzzer   ElevDR  AileDR  THRCT
//
//PORTF  7      6       5       4       3       2       1       0
//       ai     ai      ai      ai      ai      ai      ai      ai
// ANA_ BAT   PITT_TRM HOV_THR HOV_PIT  STCK_LH STCK_RV STCK_LV STCK_RH
//                                      rud    thro   elev   aile
//PORTG  7      6       5       4       3       2       1       0
//       -      -       -       O       i               i       i
//                            SIM_CTL  ID1    HAPTIC  RF_POW   RuddDR

#define OUT_B_LIGHT   7
#define INP_B_KEY_LFT 6
#define INP_B_KEY_RGT 5
#define INP_B_KEY_UP  4
#define INP_B_KEY_DWN 3
#define INP_B_KEY_EXT 2
#define INP_B_KEY_MEN 1
#define OUT_B_PPM 0
#define PORTC_LCD_CTRL PORTC
#define OUT_C_LCD_E     5
#define OUT_C_LCD_RnW   4
#define OUT_C_LCD_A0    3
#define OUT_C_LCD_RES   2
#define OUT_C_LCD_CS1   1

#define INP_D_TRM_LH_UP   7
#define INP_D_TRM_LH_DWN  6
#define INP_D_TRM_RV_DWN  5
#define INP_D_TRM_RV_UP   4
#define INP_D_TRM_LV_DWN  3
#define INP_D_TRM_LV_UP   2
#define INP_D_TRM_RH_DWN  1
#define INP_D_TRM_RH_UP   0

#define INP_E_PPM_IN  7
#define INP_E_ID2     6
#define INP_E_Trainer 5
#define INP_E_Gear    4
#define OUT_E_BUZZER  3
#define INP_E_ElevDR  2


#define INP_E_AileDR  1
#define INP_E_ThrCt   0

//#if (defined(JETI) || defined(FRSKY) || defined(ARDUPILOT) || defined(NMEA))
//#undef INP_E_ThrCt
//#undef INP_E_AileDR
#define INP_C_ThrCt   6
#define INP_C_AileDR  7

#define OUT_G_SIM_CTL  4 //1 : phone-jack=ppm_in
#define INP_G_ID1      3
#define INP_G_RF_POW   1
#define INP_G_RuddDR   0

#define SLAVE_MODE (PING() & (1<<INP_G_RF_POW))

#define read_keys() ((PINB()) & 0x7E)
uint8_t menuPressed( void ) ;

extern uint8_t SlaveMode ;

extern const char Str_Chans_Gv[] ;

extern const uint8_t stickScramble[] ;
uint8_t modeFixValue( uint8_t value ) ;

extern const char Str_OFF[] ;
extern const char Str_ON[] ;
extern const char Str_Switch_warn[] ;
extern const char modi12x3[] ;

extern uint8_t SystemOptions ;

extern const uint8_t bchout_ar[] ;

//convert from mode 1 to mode g_eeGeneral.stickMode
//NOTICE!  =>  1..4 -> 1..4
extern uint8_t convert_mode_helper(uint8_t x) ;

#define CONVERT_MODE(x)  (((x)<=4) ? convert_mode_helper(x) : (x))
#define CHANNEL_ORDER(x) ( ( (pgm_read_byte(bchout_ar + g_eeGeneral.templateSetup) >> (6-(x-1) * 2)) & 3 ) + 1 )
#define THR_STICK       (2-(g_eeGeneral.stickMode&1))
#define ELE_STICK       (1+(g_eeGeneral.stickMode&1))
#define AIL_STICK       ((g_eeGeneral.stickMode&2) ? 0 : 3)
#define RUD_STICK       ((g_eeGeneral.stickMode&2) ? 3 : 0)

enum EnumKeys {
    KEY_MENU ,
    KEY_EXIT ,
    KEY_DOWN ,
    KEY_UP  ,
    KEY_RIGHT ,
    KEY_LEFT ,
    TRM_LH_DWN  ,
    TRM_LH_UP   ,
    TRM_LV_DWN  ,
    TRM_LV_UP   ,
    TRM_RV_DWN  ,
    TRM_RV_UP   ,
    TRM_RH_DWN  ,
    TRM_RH_UP   ,
	  BTN_RE,
    //SW_NC     ,
    //SW_ON     ,
    SW_ThrCt  ,
    SW_RuddDR ,
    SW_ElevDR ,
    SW_ID0    ,
    SW_ID1    ,
    SW_ID2    ,
    SW_AileDR ,
    SW_Gear   ,
    SW_Trainer
};

// custom switch counts
#define NUM_CSW         12  //number of custom switches
#define EXTRA_CSW       6
#define EXTRA_VOICE_SW  8


// Hardware switch mappings:
#define HSW_ThrCt	1
#define HSW_RuddDR	2
#define HSW_ElevDR	3
#define HSW_ID0		4
#define HSW_ID1		5
#define HSW_ID2		6
#define HSW_AileDR	7
#define HSW_Gear	8
#define HSW_Trainer	9

#define HSW_Ele3pos0	31
#define HSW_Ele3pos1	32
#define HSW_Ele3pos2	33
#define HSW_Rud3pos0	34
#define HSW_Rud3pos1	35
#define HSW_Rud3pos2	36
#define HSW_Ail3pos0	37
#define HSW_Ail3pos1	38
#define HSW_Ail3pos2	39
#define HSW_Gear3pos0	40
#define HSW_Gear3pos1	41
#define HSW_Gear3pos2	42
#define HSW_Pb1		43
#define HSW_Pb2		44
#define HSW_MAX		44

#define HSW_OFFSET ( HSW_Ele3pos0 - ( HSW_Trainer + NUM_CSW + 1 ) )

#define MaxSwitchIndex		MAX_DRSWITCH
#define TOGGLE_INDEX		( MAX_DRSWITCH - 1 )

//Bitfield for hardware switch mapping
#define	USE_THR_3POS	0x01
#define	USE_RUD_3POS	0x02
#define	USE_ELE_3POS	0x04
#define	USE_ELE_6POS	0x08
#define	USE_AIL_3POS	0x10
#define	USE_GEA_3POS	0x20
#define	USE_PB1				0x40
#define	USE_PB2				0x80

uint8_t switchPosition( uint8_t swtch ) ;


extern const char Str_Switches[] ;

#define CURVE_BASE 7


#define CS_OFF       (uint8_t)0
#define CS_VPOS      (uint8_t)1  //v>offset
#define CS_VNEG      (uint8_t)2  //v<offset
#define CS_APOS      (uint8_t)3  //|v|>offset
#define CS_ANEG      (uint8_t)4  //|v|<offset
#define CS_AND       (uint8_t)5
#define CS_OR        (uint8_t)6
#define CS_XOR       (uint8_t)7
#define CS_EQUAL     (uint8_t)8
#define CS_NEQUAL    (uint8_t)9
#define CS_GREATER   (uint8_t)10
#define CS_LESS      (uint8_t)11
#define CS_LATCH  	 (uint8_t)12
#define CS_FLIP    	 (uint8_t)13
#define CS_TIME	     (uint8_t)14
#define CS_EXEQUAL   (uint8_t)15
#define CS_MONO		   16	// Monostable
#define CS_RMONO	   17	// Monostable with reset
#define CS_MAXF      14  //max function
#define CS_XMAXF     17  //max function

#define CS_VOFS       (uint8_t)0
#define CS_VBOOL      (uint8_t)1
#define CS_VCOMP      (uint8_t)2
#define CS_TIMER			(uint8_t)3
#define CS_TMONO      4
uint8_t CS_STATE( uint8_t x) ;


const char s_charTab[]=" ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-.";
#define NUMCHARS (sizeof(s_charTab)-1)


#define SW_BASE      SW_ThrCt
#define SW_BASE_DIAG SW_ThrCt
#define MAX_PSWITCH   (SW_Trainer-SW_ThrCt+1)  // 9 physical switches
#define MAX_DRSWITCH (1+SW_Trainer-SW_ThrCt+1+NUM_CSW)

#define SWP_RUD (SW_RuddDR-SW_BASE)
#define SWP_ELE (SW_ElevDR-SW_BASE)
#define SWP_AIL (SW_AileDR-SW_BASE)
#define SWP_THR (SW_ThrCt-SW_BASE)
#define SWP_GEA (SW_Gear-SW_BASE)

#define SWP_RUDB (1<<SWP_RUD)
#define SWP_ELEB (1<<SWP_ELE)
#define SWP_AILB (1<<SWP_AIL)
#define SWP_THRB (1<<SWP_THR)
#define SWP_GEAB (1<<SWP_GEA)

#define SWP_ID0 (SW_ID0-SW_BASE)
#define SWP_ID1 (SW_ID1-SW_BASE)
#define SWP_ID2 (SW_ID2-SW_BASE)
#define SWP_ID0B (1<<SWP_ID0)
#define SWP_ID1B (1<<SWP_ID1)
#define SWP_ID2B (1<<SWP_ID2)

//Switch Position Illigal states
#define SWP_IL1 (0)
#define SWP_IL2 (SWP_ID0B | SWP_ID1B)
#define SWP_IL3 (SWP_ID0B | SWP_ID2B)
#define SWP_IL4 (SWP_ID1B | SWP_ID2B)
#define SWP_IL5 (SWP_ID0B | SWP_ID1B | SWP_ID2B)

#define SWP_LEG1	(SWP_ID0B)
#define SWP_LEG2	(SWP_ID1B)
#define SWP_LEG3	(SWP_ID2B)


#define NUM_STICKS	4


#define SWASH_TYPE_120   1
#define SWASH_TYPE_120X  2
#define SWASH_TYPE_140   3
#define SWASH_TYPE_90    4
#define SWASH_TYPE_NUM   4

#define MIX_P1    5
#define MIX_P2    6
#define MIX_P3    7
#define MIX_MAX   8
#define MIX_FULL  9
#define MIX_CYC1  10
#define MIX_CYC2  11
#define MIX_CYC3  12

#define DR_HIGH   0
#define DR_MID    1
#define DR_LOW    2
#define DR_EXPO   0
#define DR_WEIGHT 1
#define DR_RIGHT  0
#define DR_LEFT   1
#define DR_BOTH   2
#define DR_DRSW1  99
#define DR_DRSW2  98

#define DSW_THR  1
#define DSW_RUD  2
#define DSW_ELE  3
#define DSW_ID0  4
#define DSW_ID1  5
#define DSW_ID2  6
#define DSW_AIL  7
#define DSW_GEA  8
#define DSW_TRN  9
#define DSW_SW1  10   // custom switches (NUM_CSW)
#define DSW_SW2  11
#define DSW_SW3  12
#define DSW_SW4  13
#define DSW_SW5  14
#define DSW_SW6  15
#define DSW_SW7  16
#define DSW_SW8  17
#define DSW_SW9  18
#define DSW_SWA  19
#define DSW_SWB  20
#define DSW_SWC  21


#define SCROLL_TH 64
#define INACTIVITY_THRESHOLD 256
#define THRCHK_DEADBAND 16
#define SPLASH_TIMEOUT  (4*100)  //400 msec - 4 seconds

#define IS_THROTTLE(x)  ((x) == 2) // (((2-(g_eeGeneral.stickMode&1)) == x) && (x<4))
uint8_t IS_EXPO_THROTTLE( uint8_t x ) ;

#define NUM_KEYS BTN_RE+1
#define TRM_BASE TRM_LH_DWN

#define _MSK_KEY_REPT    0x40
#define IS_KEY_BREAK(key)  (((key)&0xf0)        ==  0x20)
#define EVT_KEY_BREAK(key) ((key)|                  0x20)
#define EVT_KEY_FIRST(key) ((key)|    _MSK_KEY_REPT|0x20)
#define EVT_KEY_REPT(key)  ((key)|    _MSK_KEY_REPT     )
#define EVT_KEY_LONG(key)  ((key)|0x80)
#define EVT_KEY_DBL(key)   ((key)|_MSK_KEY_DBL)
#define EVT_ENTRY               (0xff - _MSK_KEY_REPT)
#define EVT_ENTRY_UP            (0xfe - _MSK_KEY_REPT)
#define EVT_TOGGLE_GVAR         (0xfd - _MSK_KEY_REPT)
#define EVT_KEY_MASK             0x0f

#define RESX    (1<<10) // 1024

#define HEART_TIMER2Mhz 1
#define HEART_TIMER10ms 2

#define TMRMODE_NONE     0
#define TMRMODE_ABS      1
#define TMRMODE_THR      2
#define TMRMODE_THR_REL  3
#define MAX_ALERT_TIME   10

// DEF protocols
#define PROTO_NONE       0xFF
#define PROTO_PPM        0
#define PROTO_AFHDS2A    1
#define PROTO_AFHDS      2
#define PROTO_CRSF       3	// external ELRS module
#define PROT_MAX         3
#define PROT_STR "\007PPM    AFHDS2AAFHDS  CRSF   "


//1,Flysky,Flysky,V9x9,V6x6,V912,CX20
//2,Hubsan
//3,FrskyD
//4,Hisky,Hisky,HK310
//5,V2x2,V2x2,JXD506
//6,DSM,DSM2-22,DSM2-11,DSMX-22,DSMX-11,AUTO
//7,Devo
//8,YD717,YD717,SKYWLKR,SYMAX4,XINXUN,NIHUI
//9,KN,WLTOYS,FEILUN
//10,SymaX,SYMAX,SYMAX5C
//11,SLT,SLT,VISTA
//12,CX10,GREEN,BLUE,DM007,---,J3015_1,J3015_2,MK33041
//13,CG023,CG023,YD829,H8_3D
//14,Bayang,Bayang,H8S3D
//15,FrskyX,CH_16,CH_8
//16,ESky
//17,MT99xx,MT,H7,YZ,LS,FY805
//18,MJXq,WLH08,X600,X800,H26D,E010,H26WH
//19,Shenqi
//20,FY326,FY326,FY319
//21,SFHSS
//22,J6PRO
//23,FQ777
//24,ASSAN
//25,FrskyV
//26,HONTAI,HONTAI,JJRCX1,X5C1,FQ777_951
//27,OpnLrs
//28,AFHD2SA,PWM_IBUS,PPM_IBUS,PWM_SBUS,PPM_SBUS
//29,Q2X2,Q222,Q242,Q282
//30,WK2x01,WK2801,WK2401,W6_5_1,W6_6_1,W6_HEL,W6_HEL_I


#define PXX_BIND					 0x01
#define PXX_SEND_FAILSAFE  0x10
#define PXX_RANGE_CHECK		 0x20

#define PXX_PROTO_X16			 0
#define PXX_PROTO_D8			 1
#define PXX_PROTO_LR12		 2

#define PXX_AMERICA				 0
#define PXX_JAPAN					 1
#define PXX_EUROPE				 2


#define TRIM_EXTENDED_MAX	500

extern uint8_t pxxFlag;
extern uint8_t stickMoved;

uint16_t stickMoveValue( void ) ;

#define NUM_VOL_LEVELS	8

typedef void (*MenuFuncP)(uint8_t event);

/// stoppt alle events von dieser taste bis eine kurze Zeit abgelaufen ist
void pauseEvents(uint8_t enuk);
/// liefert die Zahl der schnellen Wiederholungen dieser Taste
//uint8_t getEventDbl(uint8_t event);
/// stoppt alle events von dieser taste bis diese wieder losgelassen wird
void    killEvents(uint8_t enuk);

/// liefert den Wert einer beliebigen Taste KEY_MENU..SW_Trainer
bool    keyState(EnumKeys enuk);

/// Liefert das naechste Tasten-Event, auch trim-Tasten.
/// Das Ergebnis hat die Form:
/// EVT_KEY_BREAK(key), EVT_KEY_FIRST(key), EVT_KEY_REPT(key) oder EVT_KEY_LONG(key)

extern uint8_t s_evt;

inline uint8_t getEvent() {
	uint8_t t_evt = s_evt ;
	s_evt = 0 ;
	return t_evt ;
}

#define putEvent(evt) (s_evt = evt)


/// goto given Menu, but substitute current menu in menuStack
void    chainMenu(MenuFuncP newMenu);
/// goto given Menu, store current menu in menuStack
void    pushMenu(MenuFuncP newMenu);
///deliver address of last menu which was popped from
//MenuFuncP lastPopMenu();
/// return to last menu in menustack
/// if uppermost is set true, thenmenu return to uppermost menu in menustack
void    popMenu(bool uppermost=false);
/// Gibt Alarm Maske auf lcd aus.
/// Die Maske wird so lange angezeigt bis eine beliebige Taste gedrueckt wird.
void alert(const char * s);
void alertx(const char * s, bool defaults );
void message(const char * s);
/// periodisches Hauptprogramm
//static void    perMain();
/// Bearbeitet alle zeitkritischen Jobs.
/// wie z.B. einlesen aller Eingaenge, Entprellung, Key-Repeat..
void    per10ms();
/// Erzeugt periodisch alle Outputs ausser Bildschirmausgaben.
void zeroVariables();
void mainSequence() ;

#define NO_TRAINER 0x01
#define NO_INPUT   0x02
#define FADE_FIRST 0x20
#define FADE_LAST  0x40

void perOutPhase( int16_t *chanOut, uint8_t att ) ;
void perOut(int16_t *chanOut, uint8_t att);
///   Liefert den Zustand des Switches 'swtch'. Die Numerierung erfolgt ab 1
///   (1=SW_ON, 2=SW_ThrCt, 10=SW_Trainer). 0 Bedeutet not conected.
///   Negative Werte  erzeugen invertierte Ergebnisse.
///   Die Funktion putsDrSwitches(..) erzeugt den passenden Ausdruck.
///
///   \param swtch
///     0                : not connected. Liefert den Wert 'nc'
///     1.. MAX_DRSWITCH : SW_ON .. SW_Trainer
///    -1..-MAX_DRSWITCH : negierte Werte
///   \param nc Wert, der bei swtch==0 geliefert wird.
bool    getSwitch00(int8_t swtch) ;
bool    getSwitch(int8_t swtch, bool nc, uint8_t level=0);
/// Zeigt den Namen des Switches 'swtch' im display an
///   \param x     x-koordinate 0..127
///   \param y     y-koordinate 0..63 (nur durch 8 teilbar)
///   \param swtch -MAX_DRSWITCH ..  MAX_DRSWITCH
///   \param att   NO_INV,INVERS,BLINK
///
void putsDrSwitches(uint8_t x,uint8_t y,int8_t swtch,uint8_t att);
void putsMomentDrSwitches(uint8_t x,uint8_t y,int8_t swtch,uint8_t att);
void putsTmrMode(uint8_t x, uint8_t y, uint8_t attr, uint8_t type);

extern int32_t get_telemetry_value( uint8_t channel ) ;

#define TMR_OFF     0
#define TMR_RUNNING 1
#define TMR_BEEPING 2
#define TMR_STOPPED 3
void resetTimer();

struct t_timerg
{
	uint16_t s_timeCumTot;
	uint16_t s_timeCumAbs;  //laufzeit in 1/16 sec
	uint16_t s_timeCumSw;  //laufzeit in 1/16 sec
	uint16_t s_timeCumThr;  //gewichtete laufzeit in 1/16 sec
	uint16_t s_timeCum16ThrP; //gewichtete laufzeit in 1/16 sec
	uint16_t s_time;
  uint16_t s_cnt;
  uint16_t s_sum;
  uint8_t sw_toggled;
	uint8_t lastSwPos;
	int16_t s_timerVal[2];
	uint8_t Timer2_running ;
	uint8_t Timer2_pre ;
	int16_t last_tmr;
} ;

extern struct t_timerg TimerG ;
void resetTimer2() ;

struct t_timer
{
	uint16_t s_sum ;
	uint8_t lastSwPos ;
	uint8_t sw_toggled ;
	uint16_t s_timeCumSw ;  //laufzeit in 1/16 sec
	uint8_t lastResetSwPos;
	uint16_t s_timeCumThr ;  //gewichtete laufzeit in 1/16 sec
	uint16_t s_timeCum16ThrP ; //gewichtete laufzeit in 1/16 sec
	int16_t  s_timerVal ;
	int16_t last_tmr ;
} ;

extern struct t_timer s_timer[] ;


extern uint8_t heartbeat;

uint8_t char2idx(char c);
char idx2char(uint8_t idx);


#define EE_GENERAL 1
#define EE_MODEL   2
#define EE_TRIM    4           // Store model because of trim
#define INCDEC_SWITCH   0x08

extern bool    checkIncDec_Ret;//global helper vars
extern uint8_t s_editMode;     //global editmode

/// Bearbeite alle events die zum gewaehlten mode passen.
/// KEY_LEFT u. KEY_RIGHT
/// oder KEY_UP u. KEY_DOWN falls _FL_VERT in i_flags gesetzt ist.
/// Dabei wird der Wert der Variablen i_pval unter Beachtung der Grenzen
/// i_min und i_max veraendet.
/// i_pval hat die Groesse 1Byte oder 2Bytes falls _FL_SIZE2  in i_flags gesetzt ist
/// falls EE_GENERAL oder EE_MODEL in i_flags gesetzt ist wird bei Aenderung
/// der Variablen zusaetzlich eeDirty() aufgerufen.
/// Als Bestaetigung wird beep() aufgerufen bzw. audio.warn() wenn die Stellgrenze erreicht wird.
int16_t checkIncDec16( int16_t i_pval, int16_t i_min, int16_t i_max, uint8_t i_flags);
int8_t checkIncDec( int8_t i_val, int8_t i_min, int8_t i_max, uint8_t i_flags);
int8_t checkIncDec_i8( int8_t i_val, int8_t i_min, int8_t i_max);
int8_t checkIncDec_0(int8_t i_val, int8_t i_max) ;
int16_t checkIncDec_u0(int16_t i_val, uint8_t i_max) ;

#define CHECK_INCDEC_H_GENVAR( var, min, max)     \
    var = checkIncDec_i8(var,min,max)

#define CHECK_INCDEC_H_GENVAR_0( var, max)     \
    var = checkIncDec_0( var, max )

#define CHECK_INCDEC_H_MODELVAR( var, min, max)     \
    var = checkIncDec_i8(var,min,max)

#define CHECK_INCDEC_H_MODELVAR_0( var, max)     \
    var = checkIncDec_0(var,max)

#define CHECK_INCDEC_MODELSWITCH( var, min, max) \
    var = checkIncDec_i8(var,min,max)

#define CHECK_INCDEC_GENERALSWITCH( var, min, max) \
    var = checkIncDec_i8(var,min,max)
#define STORE_MODELVARS_TRIM   eeDirty(EE_MODEL|EE_TRIM)
#define STORE_MODELVARS   eeDirty(EE_MODEL)
#define STORE_GENERALVARS eeDirty(EE_GENERAL)

extern volatile uint8_t LcdLock ;

#define SPY_ON    //PORTB |=  (1<<OUT_B_LIGHT)
#define SPY_OFF   //PORTB &= ~(1<<OUT_B_LIGHT)


#define PULSEGEN_ON     TIMSK |=  (1<<OCIE1A)
#define PULSEGEN_OFF    TIMSK &= ~(1<<OCIE1A)

#define BITMASK(bit) (1<<(bit))


/// liefert Dimension eines Arrays
#define DIM(arr) (sizeof((arr))/sizeof((arr)[0]))

/// liefert Betrag des Arguments
template<class t> inline t abs(t a){ return a>0?a:-a; }
/// liefert das Minimum der Argumente
template<class t> inline t min(t a, t b){ return a<b?a:b; }
/// liefert das Maximum der Argumente
template<class t> inline t max(t a, t b){ return a>b?a:b; }
template<class t> inline int8_t sgn(t a){ return a>0 ? 1 : (a < 0 ? -1 : 0); }
template<class t> inline t limit(t mi, t x, t ma){ return min(max(mi,x),ma); }
inline int32_t imap(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/// Markiert einen EEPROM-Bereich als dirty. der Bereich wird dann in
/// eeCheck ins EEPROM zurueckgeschrieben.
void eeWriteBlockCmp(const void *i_pointer_ram, uint16_t i_pointer_eeprom, size_t size);
void eeWaitComplete();
void eeDirty(uint8_t msk);
void eeCheck(bool immediately=false);
void eeGeneralDefault();
bool eeReadGeneral();
void eeWriteGeneral();
void eeLoadModelName(uint8_t id,char*buf,uint8_t len);
uint16_t eeLoadModelForBackup(uint8_t id) ;
void eeLoadModel(uint8_t id);
bool eeDuplicateModel(uint8_t id);
bool eeModelExists(uint8_t id);
uint8_t modelSave( uint8_t id ) ;

#define NUM_PPM     8
//number of real outputchannels CH1-CH16
#define NUM_CHNOUT  16
///number of real input channels (1-9) plus virtual input channels X1-X4
#define PPM_BASE    MIX_CYC3
#define CHOUT_BASE  (PPM_BASE+NUM_PPM)


#define NUM_XCHNRAW (CHOUT_BASE+NUM_CHNOUT) // NUMCH + P1P2P3+ AIL/RUD/ELE/THR + MAX/FULL + CYC1/CYC2/CYC3
#define NUM_XCHNOUT (NUM_CHNOUT) //(NUM_CHNOUT)//+NUM_VIRT)

#define NUM_SCALERS	4

#define MIX_3POS	(NUM_XCHNRAW+1)

void putsChnRaw(uint8_t x,uint8_t y,uint8_t idx1,uint8_t att);

/// Schreibt [CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8] aufs lcd
void putsChn(uint8_t x,uint8_t y,uint8_t idx1,uint8_t att);
/// Schreibt die Batteriespannung aufs lcd
void putsVolts(uint8_t x,uint8_t y, int16_t volts, uint8_t att);
void putsVBat(uint8_t x,uint8_t y,uint8_t att);
void putsTime(uint8_t x,uint8_t y,int16_t tme,uint8_t att,uint8_t att2);

void putsTelemetry(uint8_t x, uint8_t y, uint8_t val, uint8_t unit, uint8_t att);
uint8_t putsTelemValue(uint8_t x, uint8_t y, int32_t val, uint8_t channel, uint8_t att) ;
uint16_t scale_telem_value( uint16_t val, uint8_t channel, uint8_t *p_att ) ;


extern int16_t calc100toRESX(int8_t x) ;

bool reachMixerCountLimit();
void menuMixersLimit(uint8_t event);

extern int16_t calc1000toRESX(int16_t x) ;  // improve calc time by Pat MacKenzie

extern volatile uint32_t g_tmr10ms;

//extern inline uint16_t get_tmr10ms()
extern uint32_t get_tmr10ms( void ) ;


#define TMR_VAROFS  4

#define SUB_MODE_V     1
#define SUB_MODE_H     2
#define SUB_MODE_H_DBL 3

void menuProcMixOne(uint8_t event);
void menuProcMix(uint8_t event);
void menuProcCurve(uint8_t event);
void menuProcTrim(uint8_t event);
void menuProcExpoOne(uint8_t event);
void menuProcExpoAll(uint8_t event);
void menuProcModel(uint8_t event);
void menuModelPhases(uint8_t event) ;
void menuProcHeli(uint8_t event);
void menuProcDiagCalib(uint8_t event);
void menuProcDiagAna(uint8_t event);
void menuProcDiagKeys(uint8_t event);
void menuProcDiagVers(uint8_t event);
void menuProcTrainer(uint8_t event);
void menuProcSetup(uint8_t event);
void menuProcMain(uint8_t event);
void menuProcModelSelect(uint8_t event);
void menuProcTemplates(uint8_t event);

void menuProcStatistic2(uint8_t event);
void menuProcStatistic(uint8_t event);
void menuProc0(uint8_t event);
void menuProcGlobals(uint8_t event);
void menuProcProtocol(uint8_t event);

extern void setupPulses();

void initTemplates();

extern int16_t intpol(int16_t, uint8_t);

extern uint16_t anaIn(uint8_t chan);
extern int16_t calibratedStick[7];
extern int8_t phyStick[4] ;
extern int16_t Ex_chans[NUM_CHNOUT];

void getADC_osmp();

void checkTHR();
void checkSwitches();


extern uint8_t            g_vbat100mV;
extern volatile uint8_t   g_blinkTmr10ms;
extern uint8_t            g_beepCnt;

#define PULSES_WORD_SIZE	72		// 72=((2+2*6)*10)/2+2
#define PULSES_BYTE_SIZE	(PULSES_WORD_SIZE * 2)

typedef union
{
    uint16_t pword[PULSES_WORD_SIZE] ;   // 72
    uint8_t pbyte[PULSES_BYTE_SIZE] ;   // 144
} p2mhz_t ;


extern p2mhz_t pulses2MHz ;
extern int16_t            g_ppmIns[8];
extern uint8_t ppmInAvailable ;
extern int16_t            g_chans512[NUM_CHNOUT];
extern volatile uint8_t   tick10ms;


extern uint8_t Ee_lock ;

// Bit masks in Ee_lock
#define EE_LOCK      1
#define EE_TRIM_LOCK 2

#include "lcd.h"
extern const char stamp1[];
extern const char stamp2[];
extern const char stamp3[];
extern const char stamp4[];
extern const char stamp5[];
extern const char Stamps[];
#include "myeeprom.h"

extern const unsigned char  s9xsplashMarker[] ;

extern const char Str_telemItems[] ;
extern const int8_t TelemIndex[] ;
extern int16_t convertTelemConstant( uint8_t channel, int8_t value) ;
extern int16_t getValue(uint8_t i) ;

// Failsafe values
#define FAILSAFE_NOT_SET	0
#define FAILSAFE_RX		1
#define FAILSAFE_CUSTOM		2
#define FAILSAFE_HOLD		3
#define FAILSAFE_NO_PULSES	4


#define NUM_TELEM_ITEMS 58

#define FLASH_DURATION 50

extern uint16_t g_LightOffCounter;

struct t_inactivity
{
	uint8_t inacPrescale ;
	uint16_t inacCounter ;
	uint16_t inacSum ;
} ;

extern struct t_inactivity Inactivity ;

#define STICK_SCROLL_TIMEOUT		9

#define sysFLAG_OLD_EEPROM (0x01)
extern uint8_t sysFlags;

//audio settungs are external to keep out clutter!
#include "audio.h"

extern uint8_t CurrentVolume ;

extern void setVolume( uint8_t value ) ;
extern void putVoiceQueue( uint8_t value ) ;
extern void putVoiceQueueLong( uint16_t value ) ;
extern void putVoiceQueueUpper( uint8_t value ) ;
void voice_numeric( int16_t value, uint8_t num_decimals, uint8_t units_index ) ;
extern void voice_telem_item( uint8_t index ) ;
extern void backlightKey(void) ;

struct t_alarmControl
{
	uint8_t AlarmTimer ;		// Units of 10 mS
	uint8_t OneSecFlag ;
	uint8_t VoiceFtimer ;		// Units of 10 mS
	uint8_t VoiceCheckFlag ;
} ;
extern struct t_alarmControl AlarmControl ;

NOINLINE void resetTimer1(void) ;

// Fiddle to force compiler to use a pointer
#define FORCE_INDIRECT(ptr) //__asm__ __volatile__ ("" : "=e" (ptr) : "0" (ptr))

extern uint8_t getFlightPhase( void ) ;
extern int16_t getRawTrimValue( uint8_t phase, uint8_t idx ) ;
extern int16_t getTrimValue( uint8_t phase, uint8_t idx ) ;
extern void setTrimValue(uint8_t phase, uint8_t idx, int16_t trim) ;

extern uint8_t StickScrollAllowed ;

extern uint8_t telemItemValid( uint8_t index ) ;
extern uint8_t Main_running ;
extern const   char *AlertMessage ;

uint8_t getCurrentSwitchStates( void ) ;

#define	ROTARY_MENU_UD		0
#define	ROTARY_VALUE			1

extern uint8_t RotaryState ;		// Defaults to ROTARY_MENU_LR

extern uint8_t Tevent ;

extern int8_t REG(int8_t x, int8_t min, int8_t max) ;
extern int8_t REG100_100(int8_t x) ;

extern uint16_t evalChkSum( void );//__attribute__((optimize(0)));
extern uint8_t isAgvar(uint8_t value) ;
struct t_calib
{
	int16_t midVals[7];
	int16_t loVals[7];
	int16_t hiVals[7];
	uint8_t idxState;
} ;

struct t_p1
{
	int16_t p1val ;
	int16_t p1valdiff ;
  int16_t p1valprev ;
	int16_t p2valprev ;
	int16_t p3valprev ;
} ;

extern struct t_p1 P1values ;

struct t_backup_restore
{
	uint8_t byteCount ;
	uint8_t dirOffset ;
	uint8_t dirIndex ;
	uint8_t gpCount ;
	uint8_t type ;
	uint8_t subState ;
	uint16_t size ;
	uint8_t modelIndex ;
	uint8_t restoreState ;
	uint8_t restoreDirBuffer[8][12] ;
} ;

struct t_alpha
{
	uint8_t AlphaIndex ;
	uint8_t lastSub ;
	uint8_t AlphaLength ;
	uint8_t *PalphaText ;
	const char *PalphaHeading ;
} ;

struct t_stickCentre
{
  int16_t zero_chans512_before[NUM_CHNOUT];
  int16_t zero_chans512_after[NUM_CHNOUT];
} ;

union t_xmem
{
	struct t_calib Cal_data ;
	char buf[sizeof(g_model.name)+5];
	ExpoData texpoData[4] ;
	struct t_backup_restore restoreData ;
	struct t_stickCentre stickCentreData ;
} ;

extern union t_xmem Xmem ;

extern uint8_t CurrentPhase ;

struct t_rotary
{
	uint8_t RotPosition ;
	uint8_t RotCount ;
	uint8_t TrotCount ;		// TeZ version
	uint8_t LastTrotCount ;		// TeZ version
	uint8_t RotEncoder ;
	int8_t LastRotaryValue ;
	int8_t RotaryControl ;
	uint8_t TezRotary ;
	int8_t Rotary_diff ;
} ;

extern struct t_rotary Rotary ;

struct t_latency
{
	uint16_t g_timeMain ;
} ;

extern uint16_t A1A2toScaledValue( uint8_t channel, uint8_t *dplaces ) ;


extern uint8_t TmOK ;
extern uint8_t StepSize ;

extern TimerMode TimerConfig[2] ;

typedef   void (*voice_serial_read_cb_t)(uint8_t *buf , uint8_t x);
void serialVoiceInit( voice_serial_read_cb_t ) ;

void sendSerialVoiceData(uint8_t *buf, uint32_t len);


uint8_t throttleReversed( void ) ;
void displayOneSwitch( uint8_t x, uint8_t y, uint8_t index ) ;


//
// Auto Pilot modes from ArduCopter
//
#define STABILIZE 0                     // hold level position
#define ACRO 1                          // rate control
#define ALT_HOLD 2                      // AUTO control
#define AUTO 3                          // AUTO control
#define GUIDED 4                        // AUTO control
#define LOITER 5                        // Hold a single location
#define RTL 6                           // AUTO control
#define CIRCLE 7                        // AUTO control
#define LAND 9                          // AUTO control
#define OF_LOITER 10                    // Hold a single location using optical flow sensor
#define DRIFT 11                        // DRIFT mode (Note: 12 is no longer used)
#define SPORT 13                        // earth frame rate control
#define FLIP        14                  // flip the vehicle on the roll axis
#define AUTOTUNE    15                  // autotune the vehicle's roll and pitch gains
#define POSHOLD     16                  // position hold with manual override


#endif /* SOURCES_ER9X_H_ */
