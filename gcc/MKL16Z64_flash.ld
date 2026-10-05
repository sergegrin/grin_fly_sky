/* GCC linker script for FS-i6 (MKL16Z64VLH4: 64K flash, 8K RAM). */
ENTRY(Reset_Handler)

/* minimum free RAM reserved for the stack (stack itself runs from the top of RAM) */
__STACKSIZE__ = 2048;

MEMORY
{
  VECTORS (rx) : ORIGIN = 0x00000000, LENGTH = 0x00000400
  CFM     (rx) : ORIGIN = 0x00000400, LENGTH = 0x00000010
  FLASH   (rx) : ORIGIN = 0x00000410, LENGTH = 0x00010000 - 0x410
  RAM     (rw) : ORIGIN = 0x1FFFF800, LENGTH = 0x00002000
}

SECTIONS
{
  .vectors :
  {
    KEEP(*(.vectors))
    /* startup code fits into the gap before the flash configuration field */
    *(.after_vectors .after_vectors.*)
  } > VECTORS

  .cfm :
  {
    KEEP(*(.cfm))
  } > CFM

  .text :
  {
    *(.text .text.*)
    *(.rodata .rodata.*)
    *(.progmem .progmem.*)
    KEEP(*(.init)) KEEP(*(.fini))
    . = ALIGN(4);
    __preinit_array_start = .;
    KEEP(*(.preinit_array))
    __preinit_array_end = .;
    __init_array_start = .;
    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array))
    __init_array_end = .;
    __fini_array_start = .;
    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array))
    __fini_array_end = .;
    . = ALIGN(4);
  } > FLASH

  .ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } > FLASH
  .ARM.exidx :
  {
    __exidx_start = .;
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    __exidx_end = .;
  } > FLASH

  .data : ALIGN(4)
  {
    __data_start__ = .;
    *(.data .data.*)
    . = ALIGN(4);
    __data_end__ = .;
  } > RAM AT > FLASH
  __data_load_start__ = LOADADDR(.data);

  .bss (NOLOAD) : ALIGN(4)
  {
    __bss_start__ = .;
    *(.bss .bss.*)
    *(COMMON)
    . = ALIGN(4);
    __bss_end__ = .;
  } > RAM

  .noinit (NOLOAD) : ALIGN(4)
  {
    *(.noinit .noinit.*)
    . = ALIGN(4);
    end = .;
    __end__ = .;
  } > RAM

  __stack_end__ = ORIGIN(RAM) + LENGTH(RAM);
  ASSERT(__end__ + __STACKSIZE__ <= __stack_end__, "not enough RAM left for the stack")
}
