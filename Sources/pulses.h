// Modified 2026 by sergegrin
#ifndef PULSES_H
#define PULSES_H

extern volatile uint8_t PausePulses ;

/*****************************************************************************/
void ISR_TIMER1_COMPA_vect(void);
/*****************************************************************************/

void startPulses( void ) ;

#endif // PULSES_H
