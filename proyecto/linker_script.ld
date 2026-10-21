ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 512K 
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 96K
}

_min_stack_size = 0x800;

SECTIONS
{
    .text :
    {
        KEEP(*(.isr_vector))   /* tabla de vectores (definida en crt0.S) */
        *(.text*)              /* codigo de todas las funciones */
        . = ALIGN(4);
    } > FLASH
    .rodata :
    {
        *(.rodata*)            /* constantes */
        . = ALIGN(4);
        KEEP(*(.init))         /* codigo de inicializacion de libc */
        KEEP(*(.fini))         /* codigo de finalizacion de libc */
        *(.eh_frame)           /* informacion para desenrollar la pila */
        *(.ARM.exidx*)         /* tabla de excepciones de ARM */
        . = ALIGN(4);
        _etext = .;            /* fin de lo que esta solo en FLASH */
    } > FLASH

    _sidata = LOADADDR(.data);

    .data :
    {
        . = ALIGN(4);
        _sdata = .;            /* inicio de .data en RAM */
        *(.data*)
        KEEP(*(.init_array))   /* constructores que llama libc */
        KEEP(*(.fini_array))   /* destructores que llama libc */
        . = ALIGN(4);          /* crt0.S copia de a 4 bytes */
        _edata = .;            /* fin de .data en RAM */
    } > RAM AT> FLASH

    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = .;             /* inicio de .bss */
        __bss_start__ = _sbss; /* mismo simbolo con el nombre que usa libc */
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;             /* fin de .bss */
        __bss_end__ = _ebss;
        end = .;               /* inicio del heap (lo usa malloc de libc) */
    } > RAM

    _estack = ORIGIN(RAM) + LENGTH(RAM);

    ASSERT((_estack - _ebss) >= _min_stack_size,
           "Error: no queda espacio suficiente para la pila en RAM")
}