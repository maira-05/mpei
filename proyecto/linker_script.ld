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
        KEEP(*(.isr_vector))
        *(.text*)
        . = ALIGN(4);
    } > FLASH

    .rodata :
    {
        *(.rodata*)
        . = ALIGN(4);
        KEEP(*(.init))
        KEEP(*(.fini))
        *(.eh_frame)
        *(.ARM.exidx*)
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    _sidata = LOADADDR(.data);

    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data*)
        KEEP(*(.init_array))
        KEEP(*(.fini_array))
        . = ALIGN(4);
        _edata = .;
    } > RAM AT> FLASH

    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = .;
        __bss_start__ = _sbss;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
        __bss_end__ = _ebss;
        end = .;
    } > RAM

    _estack = ORIGIN(RAM) + LENGTH(RAM);

    ASSERT((_estack - _ebss) >= _min_stack_size,
           "Error: no queda espacio suficiente para la pila en RAM")
}