/* =====================================================================
 * linker_script.ld - Reproductor WAV (MPEI 2026)
 * Microcontrolador: STM32F401RET6 (NUCLEO-F401RE)
 *
 * Adaptado del linker script del repositorio del curso
 * (Comp-Arch-Emb-Sys / 5-ARM-Boards / NUCLEO-64 / 18-SPI-1).
 *
 * Referencias:
 *  - DS10086  Datasheet STM32F401xD/xE, capitulo "Memory mapping"
 *  - RM0368   Reference manual, cap. 2 "Memory and bus architecture"
 *             y seccion "Boot configuration"
 *  - PM0214   Programming manual Cortex-M4, "Vector table" y "Stack pointer"
 *  - Manual de GNU ld, capitulo "Linker Scripts"
 * ===================================================================== */


/* Punto de entrada: primera funcion que se ejecuta despues del reset.
 * Esta definida en crt0.S. */
ENTRY(Reset_Handler)


/* ---------------------------------------------------------------------
 * MEMORIA FISICA DEL MICRO
 * Direcciones: DS10086, figura del mapa de memoria.
 * Tamanos: DS10086, tabla de caracteristicas (version "E" = 512 KB Flash,
 * 96 KB SRAM).
 *   r = lectura, w = escritura, x = ejecucion
 * La FLASH no lleva "w" porque el programa no escribe en ella mientras
 * corre; solo se graba con el ST-Link.
 * --------------------------------------------------------------------- */
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 512K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 96K
}


/* Espacio minimo que debe quedar libre para la pila (2 KB).
 * Se verifica con el ASSERT al final del archivo. */
_min_stack_size = 0x800;


SECTIONS
{
    /* -----------------------------------------------------------------
     * .text -> FLASH
     * La tabla de vectores va PRIMERO: segun el PM0214, al arrancar el
     * procesador lee en la direccion 0 el valor inicial de la pila y en
     * la direccion 4 la direccion del Reset_Handler. Con BOOT0 = 0
     * (RM0368, "Boot configuration") la Flash (0x08000000) se ve en la
     * direccion 0.
     * KEEP evita que el enlazador la borre aunque ninguna funcion la
     * "llame" (por ejemplo si se usa --gc-sections).
     * ----------------------------------------------------------------- */
    .text :
    {
        KEEP(*(.isr_vector))   /* tabla de vectores (definida en crt0.S) */
        *(.text*)              /* codigo de todas las funciones */
        . = ALIGN(4);
    } > FLASH


    /* -----------------------------------------------------------------
     * .rodata -> FLASH
     * Constantes (variables "const", textos de printf, tablas).
     * Aqui quedaran la tabla del seno de prueba y la fuente de letras de
     * la pantalla, asi no gastan RAM.
     * Las demas secciones las necesita la libreria de C (newlib-nano)
     * para su inicializacion y para manejo de excepciones de C.
     * ----------------------------------------------------------------- */
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


    /* Direccion en FLASH donde quedan guardados los valores iniciales
     * de .data (LMA). crt0.S los copia desde aqui hacia la RAM. */
    _sidata = LOADADDR(.data);


    /* -----------------------------------------------------------------
     * .data -> vive en RAM, pero se guarda en FLASH
     * Variables globales CON valor inicial (ej: int volumen = 5;).
     *   > RAM       : direccion donde se usa la variable (VMA)
     *   AT> FLASH   : direccion donde se graba su valor inicial (LMA)
     * (Manual de GNU ld, "Output Section LMA")
     * crt0.S copia de _sidata hacia _sdata..._edata antes de main().
     * ----------------------------------------------------------------- */
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


    /* -----------------------------------------------------------------
     * .bss -> RAM
     * Variables globales SIN valor inicial (ej: buffers de audio).
     * NOLOAD: no se guarda nada en FLASH, crt0.S solo la llena de ceros
     * desde _sbss hasta _ebss.
     * COMMON: algunas globales sin inicializar pueden caer ahi en vez de
     * .bss segun el compilador; se incluyen para que tambien queden en 0.
     * ----------------------------------------------------------------- */
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


    /* -----------------------------------------------------------------
     * PILA
     * El Cortex-M usa una pila "full descending" (PM0214, "Stack
     * pointer"): crece hacia direcciones menores. Por eso el tope se
     * pone al FINAL de la RAM:
     *   0x20000000 + 96 KB = 0x20000000 + 0x18000 = 0x20018000
     * crt0.S pone este valor en la primera entrada de la tabla de
     * vectores y en el registro SP.
     * ----------------------------------------------------------------- */
    _estack = ORIGIN(RAM) + LENGTH(RAM);


    /* Verificacion: si .data + .bss crecen tanto que no quedan al menos
     * 2 KB para la pila, la compilacion falla con este mensaje en vez de
     * que el programa falle de forma rara en la placa.
     * (Manual de GNU ld, "Assert") */
    ASSERT((_estack - _ebss) >= _min_stack_size,
           "Error: no queda espacio suficiente para la pila en RAM")
}
