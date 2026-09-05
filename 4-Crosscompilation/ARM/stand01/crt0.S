.syntax unified
.cpu cortex-m4
.thumb

.global g_pfnVectors
.global Reset_Handler

/* Default empty handler */
.equ RAM_STACK_TOP, stack_top /* Adjust based on SRAM size (128KB for STM32F405) */

.section .isr_vector,"a",%progbits
g_pfnVectors:
    .word RAM_STACK_TOP       /* Initial Stack Pointer */
    .word Reset_Handler       /* Reset Handler */
    .word NMI_Handler
    .word HardFault_Handler
    .word MemManage_Handler
    .word BusFault_Handler
    .word UsageFault_Handler
    .word 0                   /* Reserved */
    .word 0                   /* Reserved */
    .word 0                   /* Reserved */
    .word 0                   /* Reserved */
    .word SVC_Handler
    .word DebugMon_Handler
    .word 0                   /* Reserved */
    .word PendSV_Handler
    .word SysTick_Handler

.section .text.Reset_Handler
.weak Reset_Handler
.type Reset_Handler, %function
Reset_Handler:
    /* Copy data segment from flash to RAM */
    LDR R0, =_sdata
    LDR R1, =_edata
    LDR R2, =_sidata
    B LoopCopyDataInit

CopyDataInit:
    LDR R3, [R2], #4
    STR R3, [R0], #4

LoopCopyDataInit:
    CMP R0, R1
    BLT CopyDataInit

    

    /* Zero fill the bss segment */
    
    LDR R0, =_sbss
    LDR R1, =_ebss
    MOV R2, #0
    B LoopFillZerobss

FillZerobss:
    STR R2, [R0], #4

LoopFillZerobss:
ToFillZerobss:
    CMP R0, R1
    BLT FillZerobss
<<<<<<< HEAD
    B .
    BKPT
    
=======

    BL main            /* <-- se agrega la llamada al programa del ejercicio */

    /* NOTA: se quita el BKPT que traia el crt0.S original: en netduinoplus2/QEMU
       un BKPT ejecutado sin que GDB este ya "adjunto" controlando la ejecucion
       provoca un Lockup fatal. El punto de parada para inspeccionar resultados
       se pone desde GDB sobre _infinite_loop (ver instrucciones mas abajo). */
>>>>>>> ab973fa2bb552eb34c8f993b72be94b88ebff3b6
_infinite_loop:
    B _infinite_loop

/* Provide weak aliases for unhandled exceptions */
.weak NMI_Handler
.weak HardFault_Handler
.weak MemManage_Handler
.weak BusFault_Handler
.weak UsageFault_Handler
.weak SVC_Handler
.weak DebugMon_Handler
.weak PendSV_Handler
.weak SysTick_Handler

NMI_Handler:
HardFault_Handler:
MemManage_Handler:
BusFault_Handler:
UsageFault_Handler:
SVC_Handler:
DebugMon_Handler:
PendSV_Handler:
SysTick_Handler:
    B .

<<<<<<< HEAD
.section .bss
.align 4
.global array
array:
    .space 1000     /* Reserva 1000 bytes, todos en 0 por defecto */
=======
@ ============================================================
@   LAB EXERCISE 2 (LAB session 1) - PARTE 1: ensamblador puro
@   Arreglo de 1000 bytes + maximo, minimo, promedio, mediana
@ ============================================================

@   PARAMETROS  -->  EDITAR con tus datos:
@   X = ultimo digito del StudentID1, Y = ultimo digito del StudentID2
.equ ARRAY_SIZE,  1000
.equ STUDENT_X,   3
.equ STUDENT_Y,   7
.equ START_VAL,   (STUDENT_X*10 + STUDENT_Y)   @ valor inicial "XY"

.section .text.main
.global main
.type main, %function
main:
    PUSH {LR}

    @ 1) inicializar arreglo con numeros consecutivos desde START_VAL
    LDR  R0, =array
    LDR  R1, =ARRAY_SIZE
    LDR  R2, =START_VAL
    BL   array_init

    @ 2) maximo
    LDR  R0, =array
    LDR  R1, =ARRAY_SIZE
    BL   array_max
    LDR  R1, =g_max
    STR  R0, [R1]

    @ 3) minimo
    LDR  R0, =array
    LDR  R1, =ARRAY_SIZE
    BL   array_min
    LDR  R1, =g_min
    STR  R0, [R1]

    @ 4) promedio = suma / N  (Cortex-M4 tiene UDIV en hardware)
    LDR  R0, =array
    LDR  R1, =ARRAY_SIZE
    BL   array_sum          @ R0 = suma
    LDR  R1, =ARRAY_SIZE
    UDIV R0, R0, R1
    LDR  R1, =g_avg
    STR  R0, [R1]

    @ 5) mediana: se ordena el arreglo in-place y se toma el centro
    LDR  R0, =array
    LDR  R1, =ARRAY_SIZE
    BL   bubble_sort
    LDR  R0, =array
    LDR  R1, =ARRAY_SIZE
    BL   array_median
    LDR  R1, =g_median
    STR  R0, [R1]

    POP {LR}
    BX   LR

@ ------------------------------------------------------------
@ void array_init(uint8_t *arr [R0], uint32_t n [R1], uint32_t start [R2])
@ Llena arr con start, start+1, start+2, ... (STRB trunca a 8 bits solo)
@ ------------------------------------------------------------
.type array_init, %function
array_init:
    PUSH {R4}
    MOV  R4, R2
init_loop:
    CMP  R1, #0
    BEQ  init_done
    STRB R4, [R0], #1
    ADD  R4, R4, #1
    SUB  R1, R1, #1
    B    init_loop
init_done:
    POP  {R4}
    BX   LR

@ ------------------------------------------------------------
@ uint32_t array_max(uint8_t *arr [R0], uint32_t n [R1])  -> R0
@ ------------------------------------------------------------
.type array_max, %function
array_max:
    PUSH {R4,R5}
    MOV  R4, #0
max_loop:
    CMP  R1, #0
    BEQ  max_done
    LDRB R5, [R0], #1
    CMP  R5, R4
    BLS  max_skip
    MOV  R4, R5
max_skip:
    SUB  R1, R1, #1
    B    max_loop
max_done:
    MOV  R0, R4
    POP  {R4,R5}
    BX   LR

@ ------------------------------------------------------------
@ uint32_t array_min(uint8_t *arr [R0], uint32_t n [R1])  -> R0
@ ------------------------------------------------------------
.type array_min, %function
array_min:
    PUSH {R4,R5}
    MOV  R4, #0xFF
min_loop:
    CMP  R1, #0
    BEQ  min_done
    LDRB R5, [R0], #1
    CMP  R5, R4
    BHS  min_skip
    MOV  R4, R5
min_skip:
    SUB  R1, R1, #1
    B    min_loop
min_done:
    MOV  R0, R4
    POP  {R4,R5}
    BX   LR

@ ------------------------------------------------------------
@ uint32_t array_sum(uint8_t *arr [R0], uint32_t n [R1])  -> R0
@ ------------------------------------------------------------
.type array_sum, %function
array_sum:
    PUSH {R4,R5}
    MOV  R4, #0
sum_loop:
    CMP  R1, #0
    BEQ  sum_done
    LDRB R5, [R0], #1
    ADD  R4, R4, R5
    SUB  R1, R1, #1
    B    sum_loop
sum_done:
    MOV  R0, R4
    POP  {R4,R5}
    BX   LR

@ ------------------------------------------------------------
@ void bubble_sort(uint8_t *arr [R0], uint32_t n [R1])
@ ordena ascendente in-place (O(n^2), suficiente para n=1000 en QEMU)
@ ------------------------------------------------------------
.type bubble_sort, %function
bubble_sort:
    PUSH {R4-R7}
    SUB  R4, R1, #1        @ R4 = numero de pasadas restantes (n-1)
outer_loop:
    CMP  R4, #0
    BEQ  sort_done
    MOV  R5, R0             @ R5 = puntero al inicio para esta pasada
    MOV  R6, R4              @ R6 = comparaciones en esta pasada
inner_loop:
    CMP  R6, #0
    BEQ  inner_done
    LDRB R7, [R5]
    LDRB R2, [R5, #1]
    CMP  R7, R2
    BLS  no_swap
    STRB R2, [R5]
    STRB R7, [R5, #1]
no_swap:
    ADD  R5, R5, #1
    SUB  R6, R6, #1
    B    inner_loop
inner_done:
    SUB  R4, R4, #1
    B    outer_loop
sort_done:
    POP  {R4-R7}
    BX   LR

@ ------------------------------------------------------------
@ uint32_t array_median(uint8_t *sorted_arr [R0], uint32_t n [R1]) -> R0
@ n es par (1000): mediana = (arr[n/2 - 1] + arr[n/2]) / 2
@ ------------------------------------------------------------
.type array_median, %function
array_median:
    PUSH {R4}
    LSR  R2, R1, #1         @ R2 = n/2
    SUB  R3, R2, #1         @ R3 = n/2 - 1
    LDRB R4, [R0, R3]       @ arr[n/2 - 1]
    LDRB R2, [R0, R2]       @ arr[n/2]
    ADD  R0, R4, R2
    LSR  R0, R0, #1
    POP  {R4}
    BX   LR

@ ============================================================
@   DATOS: arreglo global + variables de resultado (region .bss)
@ ============================================================
.section .bss
.align 2
.global array
array:      .space ARRAY_SIZE

.global g_max
g_max:      .space 4
.global g_min
g_min:      .space 4
.global g_avg
g_avg:      .space 4
.global g_median
g_median:   .space 4
>>>>>>> ab973fa2bb552eb34c8f993b72be94b88ebff3b6
