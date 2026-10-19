ENTRY(Reset_Handler)

__STACK_SIZE_CORE0 = 2K;
__STACK_SIZE_CORE1 = 2K;

MEMORY
{
    FLASH(rx)   : ORIGIN = 0x10000000, LENGTH = 32M
    RAM(rwx)    : ORIGIN = 0x20000000, LENGTH = 512K
}

SECTIONS
{
	.text : ALIGN(4)
            { *(.intvect_c0);
            *(.intvect_c1);
            KEEP(*(.intvect_c0));
            KEEP(*(.intvect_c1));
            KEEP(*(.image_start_block));
			 *(.text*);
			} > FLASH

	.rodata  : { 
			*(.rodata*);
			*(.srodata*); 
			. = ALIGN(4);
			KEEP(*(.init))
			KEEP(*(.fini))
			*(.eh_frame)
			*(.ARM.exidx)
    		_etext = .;
			
			}	> FLASH
			
			_sidata = LOADADDR(.data);

	.data  : {  
				_sdata = .;
				*(.data*);
				*(.sdata*);
				KEEP(*(.init_array))
    			KEEP(*(.fini_array))
				_edata = .;
			} > RAM AT> FLASH

	.bss (NOLOAD) : { 
			_sbss = .;
			__bss_start__ = _sbss;
			*(.bss*) 
			_ebss = .;
			__bss_end__ = _ebss;
			}> RAM

    /* stack definition */
  .stack_core0 :
  {
    . = ALIGN(MAX(ORIGIN(RAM) + __STACK_SIZE_CORE0 , .), 8);
    PROVIDE(__CORE0_STACK_TOP = .) ;
  } > RAM

  .stack_core1 :
  {
    . = ALIGN(MAX(ORIGIN(RAM) + __STACK_SIZE_CORE0 + __STACK_SIZE_CORE1 , .), 8);
    PROVIDE(__CORE1_STACK_TOP = .) ;
  } > RAM
}
