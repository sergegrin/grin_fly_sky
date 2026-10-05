/* GCC startup for MKL16Z64 (FS-i6): vector table, flash configuration field
   and reset handler. */
#include <stdint.h>

extern uint32_t __data_load_start__, __data_start__, __data_end__;
extern uint32_t __bss_start__, __bss_end__;
extern uint32_t __stack_end__;

extern void SystemInit(void);
extern void __libc_init_array(void);
extern int main(void);

void Reset_Handler(void);
void Default_Handler(void);

#define WEAK_ALIAS __attribute__((weak, alias("Default_Handler")))

void NMI_Handler(void)            WEAK_ALIAS;
void HardFault_Handler(void)      WEAK_ALIAS;
void SVC_Handler(void)            WEAK_ALIAS;
void PendSV_Handler(void)         WEAK_ALIAS;
void SysTick_Handler(void)        WEAK_ALIAS;
void DMA0_IRQHandler(void)        WEAK_ALIAS;
void DMA1_IRQHandler(void)        WEAK_ALIAS;
void DMA2_IRQHandler(void)        WEAK_ALIAS;
void DMA3_IRQHandler(void)        WEAK_ALIAS;
void FTFA_IRQHandler(void)        WEAK_ALIAS;
void LVD_LVW_IRQHandler(void)     WEAK_ALIAS;
void LLW_IRQHandler(void)         WEAK_ALIAS;
void I2C0_IRQHandler(void)        WEAK_ALIAS;
void I2C1_IRQHandler(void)        WEAK_ALIAS;
void SPI0_IRQHandler(void)        WEAK_ALIAS;
void SPI1_IRQHandler(void)        WEAK_ALIAS;
void UART0_IRQHandler(void)       WEAK_ALIAS;
void UART1_IRQHandler(void)       WEAK_ALIAS;
void UART2_IRQHandler(void)       WEAK_ALIAS;
void ADC0_IRQHandler(void)        WEAK_ALIAS;
void CMP0_IRQHandler(void)        WEAK_ALIAS;
void TPM0_IRQHandler(void)        WEAK_ALIAS;
void TPM1_IRQHandler(void)        WEAK_ALIAS;
void TPM2_IRQHandler(void)        WEAK_ALIAS;
void RTC_IRQHandler(void)         WEAK_ALIAS;
void RTC_Seconds_IRQHandler(void) WEAK_ALIAS;
void PIT_IRQHandler(void)         WEAK_ALIAS;
void I2S0_IRQHandler(void)        WEAK_ALIAS;
void DAC0_IRQHandler(void)        WEAK_ALIAS;
void TSI0_IRQHandler(void)        WEAK_ALIAS;
void LPTimer_IRQHandler(void)     WEAK_ALIAS;
void PORTA_IRQHandler(void)       WEAK_ALIAS;
void PORTC_PORTD_IRQHandler(void) WEAK_ALIAS;

typedef void (*vector_t)(void);

__attribute__((section(".vectors"), used))
const vector_t g_vectors[48] = {
  (vector_t)&__stack_end__,
  Reset_Handler,
  NMI_Handler,
  HardFault_Handler,
  0, 0, 0, 0, 0, 0, 0,
  SVC_Handler,
  0, 0,
  PendSV_Handler,
  SysTick_Handler,
  DMA0_IRQHandler,
  DMA1_IRQHandler,
  DMA2_IRQHandler,
  DMA3_IRQHandler,
  Default_Handler,
  FTFA_IRQHandler,
  LVD_LVW_IRQHandler,
  LLW_IRQHandler,
  I2C0_IRQHandler,
  I2C1_IRQHandler,
  SPI0_IRQHandler,
  SPI1_IRQHandler,
  UART0_IRQHandler,
  UART1_IRQHandler,
  UART2_IRQHandler,
  ADC0_IRQHandler,
  CMP0_IRQHandler,
  TPM0_IRQHandler,
  TPM1_IRQHandler,
  TPM2_IRQHandler,
  RTC_IRQHandler,
  RTC_Seconds_IRQHandler,
  PIT_IRQHandler,
  I2S0_IRQHandler,
  Default_Handler,
  DAC0_IRQHandler,
  TSI0_IRQHandler,
  Default_Handler,
  LPTimer_IRQHandler,
  Default_Handler,
  PORTA_IRQHandler,
  PORTC_PORTD_IRQHandler,
};

/* Flash configuration field at 0x400: backdoor key, FPROT, FSEC, FOPT, FEPROT, FDPROT.
   FSEC = 0xFE keeps the chip unsecured (debugger/reflash stays possible). */
__attribute__((section(".cfm"), used))
const uint8_t g_flash_config[16] = {
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF,
  0xFE,
  0xFF,
  0xFF,
  0xFF,
};

__attribute__((section(".after_vectors"), noreturn))
void Reset_Handler(void)
{
  SystemInit();

  uint32_t *src = &__data_load_start__;
  uint32_t *dst = &__data_start__;
  while (dst < &__data_end__) *dst++ = *src++;
  for (dst = &__bss_start__; dst < &__bss_end__; ) *dst++ = 0;

  __libc_init_array();

  /* call through a volatile pointer so LTO does not inline the whole
     application into .after_vectors (which must stay below 0x400) */
  int (*volatile run)(void) = main;
  run();
  for (;;) ;
}

__attribute__((section(".after_vectors")))
void Default_Handler(void)
{
  for (;;) ;
}

/* newlib's __libc_init_array calls _init */
void _init(void) {}
