// Modified 2026 by sergegrin
#include "er9x.h"
#include "pulses.h"
#include "menus.h"
#include "iface_a7105.h"
#include "crossfire/crossfire.h"

p2mhz_t pulses2MHz ;

uint8_t heartbeat;
static uint8_t Current_protocol;
uint8_t pxxFlag = 0;					// bind / range check flag
volatile uint8_t PausePulses = 0 ;

static void setupPulsesPPM( void ) ;

/*------------------------------------------------------------------------------*/
static void SetStartPulsePol() {
  if (((!g_model.pulsePol) && (!(GetPPMOutState()))) || ((g_model.pulsePol) && (GetPPMOutState()))) {
    cli();
    SetPPMTimCompare(GetPPMTimCompare() + 1);
    while (!GetPPMTimCompareInterruptFlag()) {
    }
    ClearPPMTimCompareInterruptFlag();
    sei();
  }
}
/*------------------------------------------------------------------------------*/
void ISR_TIMER1_COMPA_vect(void)    // 3MHz pulse generation
{
  static uint16_t *pulsePtr = pulses2MHz.pword;
  SetPPMTimCompare(GetPPMTimCompare() + *pulsePtr);
  pulsePtr += 1;

  if (*pulsePtr == 0) {
    pulsePtr = pulses2MHz.pword;
    SetStartPulsePol();
    setupPulses();
  }
  heartbeat |= HEART_TIMER2Mhz;
}
/*------------------------------------------------------------------------------*/
void startPulses() {
	Current_protocol = g_model.protocol + 10;		// Not the same!
	PausePulses = 0;
	setupPulses();
}
/*------------------------------------------------------------------------------*/
void setupPulses() {

  uint8_t required_protocol;
  required_protocol = g_model.protocol;
  if (PausePulses) {
    required_protocol = PROTO_NONE;
  }
  BIND_DONE;
  PausePulses = 1;
  if (Current_protocol != required_protocol) {
    Current_protocol = required_protocol;
    EnablePPMTim();
    DisableGIO();
    DisablePRTTim();
    AFHDS2A_tel_status = 0;
    switch (required_protocol) {
    case PROTO_PPM:
      crsf_shutdown();
      A7105_Sleep();
      EnablePPMOut();
      break;
    case PROTO_AFHDS2A:
      DisablePPMOut();
      crsf_shutdown();
      initAFHDS2A();
      EnablePRTTim();
      break;
    case PROTO_AFHDS:
      DisablePPMOut();
      crsf_shutdown();
      initAFHDS();
      EnablePRTTim();
      break;
    case PROTO_CRSF:
      A7105_Sleep();
      DisablePPMOut();
      crsf_init();
      EnablePRTTim();
      break;
    }
  }
  if (required_protocol == PROTO_PPM) {
    setupPulsesPPM(); // Don't enable interrupts through here
  }

  PausePulses = 0;
}

static void setupPulsesPPM( void )
{
#define PPM_TMR_FREQ 3

#define PPM_CENTER 1500 * PPM_TMR_FREQ
	int16_t PPM_range;

	uint8_t startChan = g_model.ppmStart;

	//Total frame length = 22.5msec
	//each pulse is 0.7..1.7ms long with a 0.3ms stop tail
	//The pulse timer runs at 3MHz, that's why everything is multiplied by 3
	uint16_t *ptr = pulses2MHz.pword;
	uint8_t p = 8 + g_model.ppmNCH * 2; //Channels *2
	p += startChan;
	uint16_t q = (g_model.ppmDelay * 50 + 300) * PPM_TMR_FREQ; //Stoplen *3
	uint16_t rest = (22500u * PPM_TMR_FREQ) - q; //Minimum Framelen=22.5 ms
	rest += (int16_t(g_model.ppmFrameLength)) *1500;
	*ptr++ = q;
	PPM_range = g_model.extendedLimits ? 640 * PPM_TMR_FREQ : 512 * PPM_TMR_FREQ; //range of 0.7..1.7msec
	for (uint8_t i = startChan; i < p; i++) { //NUM_CHNOUT
		int16_t v = g_chans512[i] * 3 / 2;
		if (v > PPM_range) {
			v = PPM_range;
		}
		if (v < -PPM_range) {
			v = -PPM_range;
		}
		v += PPM_CENTER;

		rest -= v;
		*ptr++ = v - q; /* as Pat MacKenzie suggests */
		*ptr++ = q;
	}
	if (rest < 9000) {
		rest = 9000;
	}
	*ptr++ = rest;
	*ptr = 0;
}
