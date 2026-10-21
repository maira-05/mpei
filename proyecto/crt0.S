/* =====================================================================
 * crt0.S - Codigo de arranque (startup) - Reproductor WAV (MPEI 2026)
 * Microcontrolador: STM32F401RET6 (NUCLEO-F401RE)
 *
 * Adaptado de startup_stm32f401xe.s (STMicroelectronics, (c) 2017),
 * usado en el repositorio del curso (Comp-Arch-Emb-Sys / NUCLEO-64 /
 * 18-SPI-1). Licencia de ST: se distribuye "AS-IS".
 *
 * Cambios respecto al original:
 *  - Comentarios en espanol con referencias a la documentacion.
 *  - SystemInit es "weak": si el programa no la define, se usa una
 *    version vacia y el enlace no falla.
 *  - Si main() retorna, el procesador queda en un ciclo infinito en vez
 *    de saltar a una direccion indefinida (bx lr).
 *
 * Que hace este archivo:
 *  1. Define la tabla de vectores (seccion .isr_vector), que el linker
 *     script ubica al inicio de la FLASH (0x08000000).
 *  2. Define el Reset_Handler: inicializa la pila, copia .data de FLASH
 *     a RAM, llena .bss de ceros y llama a main().
 *  3. Define handlers "weak" para todas las interrupciones.
 *
 * Referencias:
 *  - PM0214 Programming manual Cortex-M4: "Vector table", "Exception model"
 *  - RM0368 Reference manual: tabla de vectores del NVIC (cap. 10)
 * ===================================================================== */

  .syntax unified      /* sintaxis unificada ARM/Thumb del ensamblador GNU */
  .cpu cortex-m4       /* procesador destino */
  .fpu softvfp         /* sin unidad de punto flotante por hardware */
  .thumb               /* el Cortex-M solo ejecuta instrucciones Thumb */

.global  g_pfnVectors      /* la tabla de vectores es visible para el linker */
.global  Default_Handler

/* Simbolos definidos en linker_script.ld (no ocupan memoria aqui, solo
 * se referencian):
 *   _sidata : direccion en FLASH de los valores iniciales de .data
 *   _sdata  : inicio de .data en RAM
 *   _edata  : fin de .data en RAM
 *   _sbss   : inicio de .bss en RAM
 *   _ebss   : fin de .bss en RAM
 *   _estack : tope de la pila (final de la RAM) */
.word  _sidata
.word  _sdata
.word  _edata
.word  _sbss
.word  _ebss


/* =====================================================================
 * Reset_Handler
 * Primera funcion que se ejecuta despues de un reset (ENTRY del linker
 * script y segunda entrada de la tabla de vectores).
 * ===================================================================== */
  .section  .text.Reset_Handler
  .weak  Reset_Handler
  .type  Reset_Handler, %function
Reset_Handler:
  /* 1. Inicializar el puntero de pila (SP) con el tope de la RAM.
   *    El hardware ya lo carga desde la tabla de vectores, pero se
   *    repite por seguridad (por ejemplo si se salta aqui con el
   *    depurador). */
  ldr   sp, =_estack

  /* 2. Configuracion inicial del sistema (reloj, etc.).
   *    Si el programa no define SystemInit, se usa la version vacia
   *    que esta mas abajo. */
  bl  SystemInit

  /* 3. Copiar .data de FLASH a RAM.
   *    Equivale en C a:
   *      uint32_t *src = &_sidata, *dst = &_sdata;
   *      while (dst < &_edata) *dst++ = *src++;
   *    r0 = destino (RAM), r1 = fin, r2 = origen (FLASH), r3 = offset */
  ldr r0, =_sdata
  ldr r1, =_edata
  ldr r2, =_sidata
  movs r3, #0
  b LoopCopyDataInit

CopyDataInit:
  ldr r4, [r2, r3]        /* leer 4 bytes de FLASH */
  str r4, [r0, r3]        /* escribirlos en RAM */
  adds r3, r3, #4         /* avanzar una palabra (por eso ALIGN(4)) */

LoopCopyDataInit:
  adds r4, r0, r3         /* direccion actual en RAM */
  cmp r4, r1              /* llego a _edata? */
  bcc CopyDataInit        /* si es menor, sigue copiando */

  /* 4. Llenar .bss con ceros.
   *    Equivale en C a:
   *      uint32_t *p = &_sbss;
   *      while (p < &_ebss) *p++ = 0;  */
  ldr r2, =_sbss
  ldr r4, =_ebss
  movs r3, #0
  b LoopFillZerobss

FillZerobss:
  str  r3, [r2]           /* escribir 0 */
  adds r2, r2, #4         /* siguiente palabra */

LoopFillZerobss:
  cmp r2, r4              /* llego a _ebss? */
  bcc FillZerobss

  /* 5. Inicializacion de la libreria de C (ejecuta .init_array).
   *    Viene de libc (-lc en el Makefile). */
  bl __libc_init_array

  /* 6. Llamar al programa principal. */
  bl  main

  /* 7. main() nunca deberia retornar (lleva un while(1)). Si lo hace,
   *    el procesador se queda aqui en vez de ejecutar basura. */
MainReturned:
  b  MainReturned
.size  Reset_Handler, .-Reset_Handler


/* =====================================================================
 * SystemInit por defecto (vacia)
 * Es "weak": si en C se escribe  void SystemInit(void) { ... }
 * esa version reemplaza a esta automaticamente.
 * ===================================================================== */
  .section  .text.SystemInit_Default,"ax",%progbits
  .type  SystemInit_Default, %function
SystemInit_Default:
  bx  lr                  /* retornar sin hacer nada */
  .size  SystemInit_Default, .-SystemInit_Default

  .weak      SystemInit
  .thumb_set SystemInit,SystemInit_Default


/* =====================================================================
 * Default_Handler
 * Se ejecuta cuando llega una interrupcion que no tiene handler propio
 * (o cuyo nombre en C esta mal escrito). Se queda en un ciclo infinito
 * para poder examinar el estado con el depurador.
 * ===================================================================== */
  .section  .text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
  b  Infinite_Loop
  .size  Default_Handler, .-Default_Handler


/* =====================================================================
 * TABLA DE VECTORES
 * Segun el PM0214 ("Vector table"):
 *   posicion 0 -> valor inicial del SP (_estack)
 *   posicion 1 -> direccion del Reset_Handler
 *   posiciones 2-15 -> excepciones del nucleo Cortex-M4
 *   posiciones 16 en adelante -> interrupciones del micro (IRQ0, IRQ1...)
 *     en el orden de la tabla de vectores del RM0368 (cap. 10, NVIC).
 * Un 0 indica una posicion reservada.
 * Los nombres deben coincidir EXACTAMENTE con las funciones en C
 * (ej: void SysTick_Handler(void)).
 * ===================================================================== */
   .section  .isr_vector,"a",%progbits
  .type  g_pfnVectors, %object

g_pfnVectors:
  .word  _estack
  .word  Reset_Handler
  .word  NMI_Handler
  .word  HardFault_Handler
  .word  MemManage_Handler
  .word  BusFault_Handler
  .word  UsageFault_Handler
  .word  0
  .word  0
  .word  0
  .word  0
  .word  SVC_Handler
  .word  DebugMon_Handler
  .word  0
  .word  PendSV_Handler
  .word  SysTick_Handler

  /* ---- Interrupciones externas del micro (IRQ0 en adelante) ---- */
  .word     WWDG_IRQHandler                   /* Window WatchDog              */
  .word     PVD_IRQHandler                    /* PVD through EXTI Line detection */
  .word     TAMP_STAMP_IRQHandler             /* Tamper and TimeStamps through the EXTI line */
  .word     RTC_WKUP_IRQHandler               /* RTC Wakeup through the EXTI line */
  .word     FLASH_IRQHandler                  /* FLASH                        */
  .word     RCC_IRQHandler                    /* RCC                          */
  .word     EXTI0_IRQHandler                  /* EXTI Line0                   */
  .word     EXTI1_IRQHandler                  /* EXTI Line1                   */
  .word     EXTI2_IRQHandler                  /* EXTI Line2                   */
  .word     EXTI3_IRQHandler                  /* EXTI Line3                   */
  .word     EXTI4_IRQHandler                  /* EXTI Line4                   */
  .word     DMA1_Stream0_IRQHandler           /* DMA1 Stream 0                */
  .word     DMA1_Stream1_IRQHandler           /* DMA1 Stream 1                */
  .word     DMA1_Stream2_IRQHandler           /* DMA1 Stream 2                */
  .word     DMA1_Stream3_IRQHandler           /* DMA1 Stream 3                */
  .word     DMA1_Stream4_IRQHandler           /* DMA1 Stream 4                */
  .word     DMA1_Stream5_IRQHandler           /* DMA1 Stream 5                */
  .word     DMA1_Stream6_IRQHandler           /* DMA1 Stream 6                */
  .word     ADC_IRQHandler                    /* ADC1, ADC2 and ADC3s         */
  .word     0               				  /* Reserved                      */
  .word     0              					  /* Reserved                     */
  .word     0                                 /* Reserved                     */
  .word     0                                 /* Reserved                     */
  .word     EXTI9_5_IRQHandler                /* External Line[9:5]s          */
  .word     TIM1_BRK_TIM9_IRQHandler          /* TIM1 Break and TIM9          */
  .word     TIM1_UP_TIM10_IRQHandler          /* TIM1 Update and TIM10        */
  .word     TIM1_TRG_COM_TIM11_IRQHandler     /* TIM1 Trigger and Commutation and TIM11 */
  .word     TIM1_CC_IRQHandler                /* TIM1 Capture Compare         */
  .word     TIM2_IRQHandler                   /* TIM2                         */
  .word     TIM3_IRQHandler                   /* TIM3                         */
  .word     TIM4_IRQHandler                   /* TIM4                         */
  .word     I2C1_EV_IRQHandler                /* I2C1 Event                   */
  .word     I2C1_ER_IRQHandler                /* I2C1 Error                   */
  .word     I2C2_EV_IRQHandler                /* I2C2 Event                   */
  .word     I2C2_ER_IRQHandler                /* I2C2 Error                   */
  .word     SPI1_IRQHandler                   /* SPI1                         */
  .word     SPI2_IRQHandler                   /* SPI2                         */
  .word     USART1_IRQHandler                 /* USART1                       */
  .word     USART2_IRQHandler                 /* USART2                       */
  .word     0               				  /* Reserved                       */
  .word     EXTI15_10_IRQHandler              /* External Line[15:10]s        */
  .word     RTC_Alarm_IRQHandler              /* RTC Alarm (A and B) through EXTI Line */
  .word     OTG_FS_WKUP_IRQHandler            /* USB OTG FS Wakeup through EXTI line */
  .word     0                                 /* Reserved     				  */
  .word     0                                 /* Reserved       			  */
  .word     0                                 /* Reserved 					  */
  .word     0                                 /* Reserved                     */
  .word     DMA1_Stream7_IRQHandler           /* DMA1 Stream7                 */
  .word     0                                 /* Reserved                     */
  .word     SDIO_IRQHandler                   /* SDIO                         */
  .word     TIM5_IRQHandler                   /* TIM5                         */
  .word     SPI3_IRQHandler                   /* SPI3                         */
  .word     0                                 /* Reserved                     */
  .word     0                                 /* Reserved                     */
  .word     0                                 /* Reserved                     */
  .word     0                                 /* Reserved                     */
  .word     DMA2_Stream0_IRQHandler           /* DMA2 Stream 0                */
  .word     DMA2_Stream1_IRQHandler           /* DMA2 Stream 1                */
  .word     DMA2_Stream2_IRQHandler           /* DMA2 Stream 2                */
  .word     DMA2_Stream3_IRQHandler           /* DMA2 Stream 3                */
  .word     DMA2_Stream4_IRQHandler           /* DMA2 Stream 4                */
  .word     0                    			  /* Reserved                     */
  .word     0              					  /* Reserved                     */
  .word     0              					  /* Reserved                     */
  .word     0             					  /* Reserved                     */
  .word     0              					  /* Reserved                     */
  .word     0              					  /* Reserved                     */
  .word     OTG_FS_IRQHandler                 /* USB OTG FS                   */
  .word     DMA2_Stream5_IRQHandler           /* DMA2 Stream 5                */
  .word     DMA2_Stream6_IRQHandler           /* DMA2 Stream 6                */
  .word     DMA2_Stream7_IRQHandler           /* DMA2 Stream 7                */
  .word     USART6_IRQHandler                 /* USART6                       */
  .word     I2C3_EV_IRQHandler                /* I2C3 event                   */
  .word     I2C3_ER_IRQHandler                /* I2C3 error                   */
  .word     0                                 /* Reserved                     */
  .word     0                                 /* Reserved                     */
  .word     0                                 /* Reserved                     */
  .word     0                                 /* Reserved                     */
  .word     0                                 /* Reserved                     */
  .word     0                                 /* Reserved                     */
  .word     0                                 /* Reserved                     */
  .word     FPU_IRQHandler                    /* FPU                          */
  .word     0                                 /* Reserved                     */
  .word     0                                 /* Reserved                     */
  .word     SPI4_IRQHandler                   /* SPI4                         */
  .size  g_pfnVectors, .-g_pfnVectors



/* =====================================================================
 * HANDLERS "WEAK"
 * Cada handler se define como alias debil de Default_Handler.
 * Si en C se escribe una funcion con el mismo nombre (por ejemplo
 * void TIM2_IRQHandler(void) { ... }), el linker usa esa y descarta el
 * alias. Si no, la interrupcion cae en Default_Handler.
 * ===================================================================== */
   .weak      NMI_Handler
   .thumb_set NMI_Handler,Default_Handler

   .weak      HardFault_Handler
   .thumb_set HardFault_Handler,Default_Handler

   .weak      MemManage_Handler
   .thumb_set MemManage_Handler,Default_Handler

   .weak      BusFault_Handler
   .thumb_set BusFault_Handler,Default_Handler

   .weak      UsageFault_Handler
   .thumb_set UsageFault_Handler,Default_Handler

   .weak      SVC_Handler
   .thumb_set SVC_Handler,Default_Handler

   .weak      DebugMon_Handler
   .thumb_set DebugMon_Handler,Default_Handler

   .weak      PendSV_Handler
   .thumb_set PendSV_Handler,Default_Handler

   .weak      SysTick_Handler
   .thumb_set SysTick_Handler,Default_Handler

   .weak      WWDG_IRQHandler
   .thumb_set WWDG_IRQHandler,Default_Handler

   .weak      PVD_IRQHandler
   .thumb_set PVD_IRQHandler,Default_Handler

   .weak      TAMP_STAMP_IRQHandler
   .thumb_set TAMP_STAMP_IRQHandler,Default_Handler

   .weak      RTC_WKUP_IRQHandler
   .thumb_set RTC_WKUP_IRQHandler,Default_Handler

   .weak      FLASH_IRQHandler
   .thumb_set FLASH_IRQHandler,Default_Handler

   .weak      RCC_IRQHandler
   .thumb_set RCC_IRQHandler,Default_Handler

   .weak      EXTI0_IRQHandler
   .thumb_set EXTI0_IRQHandler,Default_Handler

   .weak      EXTI1_IRQHandler
   .thumb_set EXTI1_IRQHandler,Default_Handler

   .weak      EXTI2_IRQHandler
   .thumb_set EXTI2_IRQHandler,Default_Handler

   .weak      EXTI3_IRQHandler
   .thumb_set EXTI3_IRQHandler,Default_Handler

   .weak      EXTI4_IRQHandler
   .thumb_set EXTI4_IRQHandler,Default_Handler

   .weak      DMA1_Stream0_IRQHandler
   .thumb_set DMA1_Stream0_IRQHandler,Default_Handler

   .weak      DMA1_Stream1_IRQHandler
   .thumb_set DMA1_Stream1_IRQHandler,Default_Handler

   .weak      DMA1_Stream2_IRQHandler
   .thumb_set DMA1_Stream2_IRQHandler,Default_Handler

   .weak      DMA1_Stream3_IRQHandler
   .thumb_set DMA1_Stream3_IRQHandler,Default_Handler

   .weak      DMA1_Stream4_IRQHandler
   .thumb_set DMA1_Stream4_IRQHandler,Default_Handler

   .weak      DMA1_Stream5_IRQHandler
   .thumb_set DMA1_Stream5_IRQHandler,Default_Handler

   .weak      DMA1_Stream6_IRQHandler
   .thumb_set DMA1_Stream6_IRQHandler,Default_Handler

   .weak      ADC_IRQHandler
   .thumb_set ADC_IRQHandler,Default_Handler

   .weak      EXTI9_5_IRQHandler
   .thumb_set EXTI9_5_IRQHandler,Default_Handler

   .weak      TIM1_BRK_TIM9_IRQHandler
   .thumb_set TIM1_BRK_TIM9_IRQHandler,Default_Handler

   .weak      TIM1_UP_TIM10_IRQHandler
   .thumb_set TIM1_UP_TIM10_IRQHandler,Default_Handler

   .weak      TIM1_TRG_COM_TIM11_IRQHandler
   .thumb_set TIM1_TRG_COM_TIM11_IRQHandler,Default_Handler

   .weak      TIM1_CC_IRQHandler
   .thumb_set TIM1_CC_IRQHandler,Default_Handler

   .weak      TIM2_IRQHandler
   .thumb_set TIM2_IRQHandler,Default_Handler

   .weak      TIM3_IRQHandler
   .thumb_set TIM3_IRQHandler,Default_Handler

   .weak      TIM4_IRQHandler
   .thumb_set TIM4_IRQHandler,Default_Handler

   .weak      I2C1_EV_IRQHandler
   .thumb_set I2C1_EV_IRQHandler,Default_Handler

   .weak      I2C1_ER_IRQHandler
   .thumb_set I2C1_ER_IRQHandler,Default_Handler

   .weak      I2C2_EV_IRQHandler
   .thumb_set I2C2_EV_IRQHandler,Default_Handler

   .weak      I2C2_ER_IRQHandler
   .thumb_set I2C2_ER_IRQHandler,Default_Handler

   .weak      SPI1_IRQHandler
   .thumb_set SPI1_IRQHandler,Default_Handler

   .weak      SPI2_IRQHandler
   .thumb_set SPI2_IRQHandler,Default_Handler

   .weak      USART1_IRQHandler
   .thumb_set USART1_IRQHandler,Default_Handler

   .weak      USART2_IRQHandler
   .thumb_set USART2_IRQHandler,Default_Handler

   .weak      EXTI15_10_IRQHandler
   .thumb_set EXTI15_10_IRQHandler,Default_Handler

   .weak      RTC_Alarm_IRQHandler
   .thumb_set RTC_Alarm_IRQHandler,Default_Handler

   .weak      OTG_FS_WKUP_IRQHandler
   .thumb_set OTG_FS_WKUP_IRQHandler,Default_Handler

   .weak      DMA1_Stream7_IRQHandler
   .thumb_set DMA1_Stream7_IRQHandler,Default_Handler

   .weak      SDIO_IRQHandler
   .thumb_set SDIO_IRQHandler,Default_Handler

   .weak      TIM5_IRQHandler
   .thumb_set TIM5_IRQHandler,Default_Handler

   .weak      SPI3_IRQHandler
   .thumb_set SPI3_IRQHandler,Default_Handler

   .weak      DMA2_Stream0_IRQHandler
   .thumb_set DMA2_Stream0_IRQHandler,Default_Handler

   .weak      DMA2_Stream1_IRQHandler
   .thumb_set DMA2_Stream1_IRQHandler,Default_Handler

   .weak      DMA2_Stream2_IRQHandler
   .thumb_set DMA2_Stream2_IRQHandler,Default_Handler

   .weak      DMA2_Stream3_IRQHandler
   .thumb_set DMA2_Stream3_IRQHandler,Default_Handler

   .weak      DMA2_Stream4_IRQHandler
   .thumb_set DMA2_Stream4_IRQHandler,Default_Handler

   .weak      OTG_FS_IRQHandler
   .thumb_set OTG_FS_IRQHandler,Default_Handler

   .weak      DMA2_Stream5_IRQHandler
   .thumb_set DMA2_Stream5_IRQHandler,Default_Handler

   .weak      DMA2_Stream6_IRQHandler
   .thumb_set DMA2_Stream6_IRQHandler,Default_Handler

   .weak      DMA2_Stream7_IRQHandler
   .thumb_set DMA2_Stream7_IRQHandler,Default_Handler

   .weak      USART6_IRQHandler
   .thumb_set USART6_IRQHandler,Default_Handler

   .weak      I2C3_EV_IRQHandler
   .thumb_set I2C3_EV_IRQHandler,Default_Handler

   .weak      I2C3_ER_IRQHandler
   .thumb_set I2C3_ER_IRQHandler,Default_Handler

   .weak      FPU_IRQHandler
   .thumb_set FPU_IRQHandler,Default_Handler

   .weak      SPI4_IRQHandler
   .thumb_set SPI4_IRQHandler,Default_Handler