OUTPUT_ARCH("riscv")

/* Entry Point */
ENTRY(reset_handler)

/* Memories definition */
MEMORY
{
  RAM (xrw) : ORIGIN = 0x00100000,  LENGTH = 64K
}

/* Sections */
SECTIONS
{
  .isr_vector :
  {
    KEEP(*(.isr_vector))
    . = ALIGN(32);
  } > RAM

  .text :
  { 
	  *(.init)
    *(EXCLUDE_FILE(obj/pmp.o) .text)
	  *(EXCLUDE_FILE(obj/pmp.o) .text.*)
	  . = ALIGN(32);
     *(.fini)
	  _etext = .;
  } > RAM
  
  .m_code :
  { 
    *(.m_code)
    *(.m_code.*)
    obj/pmp.o(.text)
    obj/pmp.o(.text.*)
    . = ALIGN(32);
  } > RAM

  .user_code :
  {
    *(.user_code)
    *(.user_code.*)
    . = ALIGN(32);
  } > RAM

  .rodata :
  {
    *(.rodata)
	  *(.rodata.*)
	  . = ALIGN(32);
  } > RAM

  .data : ALIGN(32)
  {
    *(.data)
    *(.data.*)
    . = ALIGN(32);
  } > RAM

  .bss :
  {
	  *(.bss)
	  *(.bss.*)
	  *(COMMON)
	  . = ALIGN(32);
    end = .;
	  __end__ = .;
  } > RAM 


  PROVIDE( _vectors_start = ADDR(.isr_vector) );

  PROVIDE( _stack_start   = ORIGIN(RAM) + LENGTH(RAM)     );
  PROVIDE( _estack        = ORIGIN(RAM) + LENGTH(RAM)     );    /* TO CHECK                */
  PROVIDE( _bss_start     = ADDR(.bss)                    );
  PROVIDE( _bss_end       = ADDR(.bss)  + SIZEOF(.bss)    );
  PROVIDE( _data_start    = ADDR(.data)                   );
  PROVIDE( _data_end      = ADDR(.data) + SIZEOF(.data)   );
  PROVIDE( _text_start    = ADDR(.text)                   );
  PROVIDE( _text_end    = ADDR(.text) + SIZEOF(.text)   );
  PROVIDE( _rodata_start  = ADDR(.rodata)                 );
  PROVIDE( _rodata_end    = ADDR(.rodata) + SIZEOF(.rodata));
  PROVIDE( _usertext_start = ADDR(.user_code)             );
  PROVIDE( _usertext_end = ADDR(.user_code) + SIZEOF(.user_code));
  PROVIDE( _mcode_start = ADDR(.m_code)             );
  PROVIDE( _mcode_end = ADDR(.m_code) + SIZEOF(.m_code));

}
