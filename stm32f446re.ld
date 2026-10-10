ENTRY(Reset_Handler)

_estack = ORIGIN(SRAM) + LENGTH(SRAM);

MEMORY
{
  FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 512K
  SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

SECTIONS
{
  /*This section should include .text section of all input files*/
  .text :
  {
    /*merge all .isr_vector section of all input files*/
    *(.isr_vector)
    /*merge all .text section of all input files*/
    *(.text)
    *(.text.*)
    *(.init)
    *(.fini)
    /*merge all .rodata section of all input files*/
    *(.rodata*)
    *(.rodata.*)
    . = ALIGN(4);
    _etext = .;
  } > FLASH AT > FLASH

  _sidata = LOADADDR(.data);
  .data :
  {
    _sdata = .;
    *(.data*)
    . = ALIGN(4);
    _edata = .;
  } > SRAM AT > FLASH

  .bss :
  {
    _sbss = .;
    __bss_start__ = _sbss; /* Standard library needs this! */
    
    *(.bss)
    *(.bss.*)
    
    . = ALIGN(4);
    _ebss = .;
    __bss_end__ = _ebss;   /* Standard library needs this! */
    
    end = .;               /* _sbrk (Heap) needs this! */
    __end__ = .;           /* Sometimes required by Newlib-nano */
  } > SRAM
}