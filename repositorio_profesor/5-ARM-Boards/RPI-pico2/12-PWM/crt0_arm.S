.syntax unified
.cpu cortex-m33
.thumb

.equ SIO_BASE, 0xd0000000
.equ PPB_BASE, 0xe0000000

.equ SIO_CPUID_OFFSET, 0x00000000
.equ VTOR_TABLE_OFFSET, 0x0000ed08

.equ BOOTROM_VTABLE_OFFSET, 0x00



.section .intvect_c0, "ax"
.align 4
.global __INTVECT_Core0
__INTVECT_Core0:
    .word __CORE0_STACK_TOP
    .word Reset_Handler
    .word NMI
    .word HardFault
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word SVCall
    .word 0
    .word 0
    .word PendSV
    .word SysTickTimer_IRQHandler
    .word TIMER0_IRQ_0_IRQn_Handler
    .word TIMER0_IRQ_1_IRQn_Handler
    .word TIMER0_IRQ_2_IRQn_Handler
    .word TIMER0_IRQ_3_IRQn_Handler
    .word TIMER1_IRQ_0_IRQn_Handler
    .word TIMER1_IRQ_1_IRQn_Handler
    .word TIMER1_IRQ_2_IRQn_Handler
    .word TIMER1_IRQ_3_IRQn_Handler
    .word PWM_IRQ_WRAP_0_IRQn_Handler
    .word PWM_IRQ_WRAP_1_IRQn_Handler
    .word DMA_IRQ_0_IRQn_Handler
    .word DMA_IRQ_1_IRQn_Handler
    .word DMA_IRQ_2_IRQn_Handler
    .word DMA_IRQ_3_IRQn_Handler
    .word USBCTRL_IRQ_IRQn_Handler
    .word PIO0_IRQ_0_IRQn_Handler
    .word PIO0_IRQ_1_IRQn_Handler
    .word PIO1_IRQ_0_IRQn_Handler
    .word PIO1_IRQ_1_IRQn_Handler
    .word PIO2_IRQ_0_IRQn_Handler
    .word PIO2_IRQ_1_IRQn_Handler
    .word IO_IRQ_BANK0_IRQn_Handler
    .word IO_IRQ_BANK0_NS_IRQn_Handler
    .word IO_IRQ_QSPI_IRQn_Handler
    .word IO_IRQ_QSPI_NS_IRQn_Handler
    .word SIO_IRQ_FIFO_IRQn_Handler
    .word SIO_IRQ_BELL_IRQn_Handler
    .word SIO_IRQ_FIFO_NS_IRQn_Handler
    .word SIO_IRQ_BELL_NS_IRQn_Handler
    .word SIO_IRQ_MTIMECMP_IRQn_Handler
    .word CLOCKS_IRQ_IRQn_Handler
    .word SPI0_IRQ_IRQn_Handler
    .word SPI1_IRQ_IRQn_Handler
    .word UART0_IRQ_IRQn_Handler
    .word UART1_IRQ_IRQn_Handler
    .word ADC_IRQ_FIFO_IRQn_Handler
    .word I2C0_IRQ_IRQn_Handler
    .word I2C1_IRQ_IRQn_Handler
    .word OTP_IRQ_IRQn_Handler
    .word TRNG_IRQ_IRQn_Handler
    .word PLL_SYS_IRQ_IRQn_Handler
    .word PLL_USB_IRQ_IRQn_Handler
    .word POWMAN_IRQ_POW_IRQn_Handler
    .word POWMAN_IRQ_TIMER_IRQn_Handler
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0


 .section .intvect_c1, "ax"
 .align 4
 .global __INTVECT_Core1
__INTVECT_Core1:
    .word __CORE1_STACK_TOP
    .word Reset_Handler
    .word NMI
    .word HardFault
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word SVCall
    .word 0
    .word 0
    .word PendSV
    .word SysTickTimer_IRQHandler
    .word TIMER0_IRQ_0_IRQn_Handler
    .word TIMER0_IRQ_1_IRQn_Handler
    .word TIMER0_IRQ_2_IRQn_Handler
    .word TIMER0_IRQ_3_IRQn_Handler
    .word TIMER1_IRQ_0_IRQn_Handler
    .word TIMER1_IRQ_1_IRQn_Handler
    .word TIMER1_IRQ_2_IRQn_Handler
    .word TIMER1_IRQ_3_IRQn_Handler
    .word PWM_IRQ_WRAP_0_IRQn_Handler
    .word PWM_IRQ_WRAP_1_IRQn_Handler
    .word DMA_IRQ_0_IRQn_Handler
    .word DMA_IRQ_1_IRQn_Handler
    .word DMA_IRQ_2_IRQn_Handler
    .word DMA_IRQ_3_IRQn_Handler
    .word USBCTRL_IRQ_IRQn_Handler
    .word PIO0_IRQ_0_IRQn_Handler
    .word PIO0_IRQ_1_IRQn_Handler
    .word PIO1_IRQ_0_IRQn_Handler
    .word PIO1_IRQ_1_IRQn_Handler
    .word PIO2_IRQ_0_IRQn_Handler
    .word PIO2_IRQ_1_IRQn_Handler
    .word IO_IRQ_BANK0_IRQn_Handler
    .word IO_IRQ_BANK0_NS_IRQn_Handler
    .word IO_IRQ_QSPI_IRQn_Handler
    .word IO_IRQ_QSPI_NS_IRQn_Handler
    .word SIO_IRQ_FIFO_IRQn_Handler
    .word SIO_IRQ_BELL_IRQn_Handler
    .word SIO_IRQ_FIFO_NS_IRQn_Handler
    .word SIO_IRQ_BELL_NS_IRQn_Handler
    .word SIO_IRQ_MTIMECMP_IRQn_Handler
    .word CLOCKS_IRQ_IRQn_Handler
    .word SPI0_IRQ_IRQn_Handler
    .word SPI1_IRQ_IRQn_Handler
    .word UART0_IRQ_IRQn_Handler
    .word UART1_IRQ_IRQn_Handler
    .word ADC_IRQ_FIFO_IRQn_Handler
    .word I2C0_IRQ_IRQn_Handler
    .word I2C1_IRQ_IRQn_Handler
    .word OTP_IRQ_IRQn_Handler
    .word TRNG_IRQ_IRQn_Handler
    .word PLL_SYS_IRQ_IRQn_Handler
    .word PLL_USB_IRQ_IRQn_Handler
    .word POWMAN_IRQ_POW_IRQn_Handler
    .word POWMAN_IRQ_TIMER_IRQn_Handler
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0

.section .image_start_block, "ax"
.align 4
.global image_definition_Block
image_definition_Block:
.word 0xffffded3
.word 0x10210142
.word 0x000001ff
.word 0x00000000
.word 0xab123579


_enter_vtable_in_r0:
    ldr r1, =(PPB_BASE + VTOR_TABLE_OFFSET)
    str r0, [r1]
    # Load the stack pointer and reset vector from the vector table
    ldmia r0!, {r1, r2} 
    # Set the main stack pointer
    msr msp, r1 
    # Branch to the reset handler
    bx r2 


.section .text.Reset_Handler
.weak Reset_Handler
.type Reset_Handler, %function
Reset_Handler:
    ldr r0, =(SIO_BASE + SIO_CPUID_OFFSET)
    ldr r0, [r0]
    cbz r0, 1f
hold_non_core0_in_bootrom:
    // Send back to the ROM to wait for core 0 to launch it.
    movs r0, #BOOTROM_VTABLE_OFFSET
    b _enter_vtable_in_r0
1:
    // Core 0 is running, so we can continue with the boot process.
    ldr r0, =_sdata
    ldr r1, =_edata
    ldr r2, =_sidata
    cmp r0, r1
    beq data_cpy_done
data_cpy_loop:
    ldm r2!, {r3}
    stm r0!, {r3}
    cmp r0, r1
    blo data_cpy_loop
data_cpy_done:

    // Zero out the BSS
    ldr r1, =_sbss
    ldr r2, =_ebss
    movs r0, #0
    b bss_fill_test
bss_fill_loop:
    stm r1!, {r0}
bss_fill_test:
    cmp r1, r2
    bne bss_fill_loop

    ldr r1, =main
    blx r1

1: // separate label because _exit can be moved out of branch range
    bkpt #0
    b 1b



.weak NMI
.weak HardFault
.weak SVCall
.weak PendSV
.weak SysTickTimer_IRQHandler
.weak TIMER0_IRQ_0_IRQn_Handler
.weak TIMER0_IRQ_1_IRQn_Handler
.weak TIMER0_IRQ_2_IRQn_Handler
.weak TIMER0_IRQ_3_IRQn_Handler
.weak TIMER1_IRQ_0_IRQn_Handler
.weak TIMER1_IRQ_1_IRQn_Handler
.weak TIMER1_IRQ_2_IRQn_Handler
.weak TIMER1_IRQ_3_IRQn_Handler
.weak PWM_IRQ_WRAP_0_IRQn_Handler
.weak PWM_IRQ_WRAP_1_IRQn_Handler
.weak DMA_IRQ_0_IRQn_Handler
.weak DMA_IRQ_1_IRQn_Handler
.weak DMA_IRQ_2_IRQn_Handler
.weak DMA_IRQ_3_IRQn_Handler
.weak USBCTRL_IRQ_IRQn_Handler
.weak PIO0_IRQ_0_IRQn_Handler
.weak PIO0_IRQ_1_IRQn_Handler
.weak PIO1_IRQ_0_IRQn_Handler
.weak PIO1_IRQ_1_IRQn_Handler
.weak PIO2_IRQ_0_IRQn_Handler
.weak PIO2_IRQ_1_IRQn_Handler
.weak IO_IRQ_BANK0_IRQn_Handler
.weak IO_IRQ_BANK0_NS_IRQn_Handler
.weak IO_IRQ_QSPI_IRQn_Handler
.weak IO_IRQ_QSPI_NS_IRQn_Handler
.weak SIO_IRQ_FIFO_IRQn_Handler
.weak SIO_IRQ_BELL_IRQn_Handler
.weak SIO_IRQ_FIFO_NS_IRQn_Handler
.weak SIO_IRQ_BELL_NS_IRQn_Handler
.weak SIO_IRQ_MTIMECMP_IRQn_Handler
.weak CLOCKS_IRQ_IRQn_Handler
.weak SPI0_IRQ_IRQn_Handler
.weak SPI1_IRQ_IRQn_Handler
.weak UART0_IRQ_IRQn_Handler
.weak UART1_IRQ_IRQn_Handler
.weak ADC_IRQ_FIFO_IRQn_Handler
.weak I2C0_IRQ_IRQn_Handler
.weak I2C1_IRQ_IRQn_Handler
.weak OTP_IRQ_IRQn_Handler
.weak TRNG_IRQ_IRQn_Handler
.weak PLL_SYS_IRQ_IRQn_Handler
.weak PLL_USB_IRQ_IRQn_Handler
.weak POWMAN_IRQ_POW_IRQn_Handler
.weak POWMAN_IRQ_TIMER_IRQn_Handler


.section .text.NMI
.weak NMI
.global NMI
.type NMI, %function
.thumb_func
NMI:
 b .

.section .text.HardFault
.weak HardFault
.global HardFault
.type HardFault, %function
.thumb_func
HardFault:
    B .

.section .text.SVCall
.weak SVCall
.global SVCall
.type SVCall, %function
.thumb_func
SVCall:
    B .

.section .text.PendSV
.weak PendSV
.global PendSV
.type PendSV, %function
.thumb_func
PendSV:
    B .

.section .text.SysTickTimer_IRQHandler
.weak SysTickTimer_IRQHandler
.global SysTickTimer_IRQHandler
.type SysTickTimer_IRQHandler, %function
.thumb_func
SysTickTimer_IRQHandler:
    B .
    
.section .text.TIMER0_IRQ_0_IRQn_Handler
.weak TIMER0_IRQ_0_IRQn_Handler
.global TIMER0_IRQ_0_IRQn_Handler
.type TIMER0_IRQ_0_IRQn_Handler, %function
.thumb_func
TIMER0_IRQ_0_IRQn_Handler:

.section .text.TIMER0_IRQ_1_IRQn_Handler
.weak TIMER0_IRQ_1_IRQn_Handler
.global TIMER0_IRQ_1_IRQn_Handler
.type TIMER0_IRQ_1_IRQn_Handler, %function
.thumb_func
TIMER0_IRQ_1_IRQn_Handler:

.section .text.TIMER0_IRQ_2_IRQn_Handler
.weak TIMER0_IRQ_2_IRQn_Handler
.global TIMER0_IRQ_2_IRQn_Handler
.type TIMER0_IRQ_2_IRQn_Handler, %function
.thumb_func
TIMER0_IRQ_2_IRQn_Handler:

.section .text.TIMER0_IRQ_3_IRQn_Handler
.weak TIMER0_IRQ_3_IRQn_Handler
.global TIMER0_IRQ_3_IRQn_Handler
.type TIMER0_IRQ_3_IRQn_Handler, %function
.thumb_func
TIMER0_IRQ_3_IRQn_Handler:

.section .text.TIMER1_IRQ_0_IRQn_Handler
.weak TIMER1_IRQ_0_IRQn_Handler
.global TIMER1_IRQ_0_IRQn_Handler
.type TIMER1_IRQ_0_IRQn_Handler, %function
.thumb_func
TIMER1_IRQ_0_IRQn_Handler:

.section .text.TIMER1_IRQ_1_IRQn_Handler
.weak TIMER1_IRQ_1_IRQn_Handler
.global TIMER1_IRQ_1_IRQn_Handler
.type TIMER1_IRQ_1_IRQn_Handler, %function
.thumb_func
TIMER1_IRQ_1_IRQn_Handler:

.section .text.TIMER1_IRQ_2_IRQn_Handler
.weak TIMER1_IRQ_2_IRQn_Handler
.global TIMER1_IRQ_2_IRQn_Handler
.type TIMER1_IRQ_2_IRQn_Handler, %function
.thumb_func
TIMER1_IRQ_2_IRQn_Handler:

.section .text.TIMER1_IRQ_3_IRQn_Handler
.weak TIMER1_IRQ_3_IRQn_Handler
.global TIMER1_IRQ_3_IRQn_Handler
.type TIMER1_IRQ_3_IRQn_Handler, %function
.thumb_func
TIMER1_IRQ_3_IRQn_Handler:

.section .text.PWM_IRQ_WRAP_0_IRQn_Handler
.weak PWM_IRQ_WRAP_0_IRQn_Handler
.global PWM_IRQ_WRAP_0_IRQn_Handler
.type PWM_IRQ_WRAP_0_IRQn_Handler, %function
.thumb_func
PWM_IRQ_WRAP_0_IRQn_Handler:

.section .text.PWM_IRQ_WRAP_1_IRQn_Handler
.weak PWM_IRQ_WRAP_1_IRQn_Handler
.global PWM_IRQ_WRAP_1_IRQn_Handler
.type PWM_IRQ_WRAP_1_IRQn_Handler, %function
.thumb_func
PWM_IRQ_WRAP_1_IRQn_Handler:

.section .text.DMA_IRQ_0_IRQn_Handler
.weak DMA_IRQ_0_IRQn_Handler
.global DMA_IRQ_0_IRQn_Handler
.type DMA_IRQ_0_IRQn_Handler, %function
.thumb_func
DMA_IRQ_0_IRQn_Handler:

.section .text.DMA_IRQ_1_IRQn_Handler
.weak DMA_IRQ_1_IRQn_Handler
.global DMA_IRQ_1_IRQn_Handler
.type DMA_IRQ_1_IRQn_Handler, %function
.thumb_func
DMA_IRQ_1_IRQn_Handler:

.section .text.DMA_IRQ_2_IRQn_Handler
.weak DMA_IRQ_2_IRQn_Handler
.global DMA_IRQ_2_IRQn_Handler
.type DMA_IRQ_2_IRQn_Handler, %function
.thumb_func
DMA_IRQ_2_IRQn_Handler:

.section .text.DMA_IRQ_3_IRQn_Handler
.weak DMA_IRQ_3_IRQn_Handler
.global DMA_IRQ_3_IRQn_Handler
.type DMA_IRQ_3_IRQn_Handler, %function
.thumb_func
DMA_IRQ_3_IRQn_Handler:

.section .text.USBCTRL_IRQ_IRQn_Handler
.weak USBCTRL_IRQ_IRQn_Handler
.global USBCTRL_IRQ_IRQn_Handler
.type USBCTRL_IRQ_IRQn_Handler, %function
.thumb_func
USBCTRL_IRQ_IRQn_Handler:

.section .text.PIO0_IRQ_0_IRQn_Handler
.weak PIO0_IRQ_0_IRQn_Handler
.global PIO0_IRQ_0_IRQn_Handler
.type PIO0_IRQ_0_IRQn_Handler, %function
.thumb_func
PIO0_IRQ_0_IRQn_Handler:

.section .text.PIO0_IRQ_1_IRQn_Handler
.weak PIO0_IRQ_1_IRQn_Handler
.global PIO0_IRQ_1_IRQn_Handler
.type PIO0_IRQ_1_IRQn_Handler, %function
.thumb_func
PIO0_IRQ_1_IRQn_Handler:

.section .text.PIO1_IRQ_0_IRQn_Handler
.weak PIO1_IRQ_0_IRQn_Handler
.global PIO1_IRQ_0_IRQn_Handler
.type PIO1_IRQ_0_IRQn_Handler, %function
.thumb_func
PIO1_IRQ_0_IRQn_Handler:

.section .text.PIO1_IRQ_1_IRQn_Handler
.weak PIO1_IRQ_1_IRQn_Handler
.global PIO1_IRQ_1_IRQn_Handler
.type PIO1_IRQ_1_IRQn_Handler, %function
.thumb_func
PIO1_IRQ_1_IRQn_Handler:

.section .text.PIO2_IRQ_0_IRQn_Handler
.weak PIO2_IRQ_0_IRQn_Handler
.global PIO2_IRQ_0_IRQn_Handler
.type PIO2_IRQ_0_IRQn_Handler, %function
.thumb_func
PIO2_IRQ_0_IRQn_Handler:

.section .text.PIO2_IRQ_1_IRQn_Handler
.weak PIO2_IRQ_1_IRQn_Handler
.global PIO2_IRQ_1_IRQn_Handler
.type PIO2_IRQ_1_IRQn_Handler, %function
.thumb_func
PIO2_IRQ_1_IRQn_Handler:

.section .text.IO_IRQ_BANK0_IRQn_Handler
.weak IO_IRQ_BANK0_IRQn_Handler
.global IO_IRQ_BANK0_IRQn_Handler
.type IO_IRQ_BANK0_IRQn_Handler, %function
.thumb_func
IO_IRQ_BANK0_IRQn_Handler:

.section .text.IO_IRQ_BANK0_NS_IRQn_Handler
.weak IO_IRQ_BANK0_NS_IRQn_Handler
.global IO_IRQ_BANK0_NS_IRQn_Handler
.type IO_IRQ_BANK0_NS_IRQn_Handler, %function
.thumb_func
IO_IRQ_BANK0_NS_IRQn_Handler:

.section .text.IO_IRQ_QSPI_IRQn_Handler
.weak IO_IRQ_QSPI_IRQn_Handler
.global IO_IRQ_QSPI_IRQn_Handler
.type IO_IRQ_QSPI_IRQn_Handler, %function
.thumb_func
IO_IRQ_QSPI_IRQn_Handler:

.section .text.IO_IRQ_QSPI_NS_IRQn_Handler
.weak IO_IRQ_QSPI_NS_IRQn_Handler
.global IO_IRQ_QSPI_NS_IRQn_Handler
.type IO_IRQ_QSPI_NS_IRQn_Handler, %function
.thumb_func
IO_IRQ_QSPI_NS_IRQn_Handler:

.section .text.SIO_IRQ_FIFO_IRQn_Handler
.weak SIO_IRQ_FIFO_IRQn_Handler
.global SIO_IRQ_FIFO_IRQn_Handler
.type SIO_IRQ_FIFO_IRQn_Handler, %function
.thumb_func
SIO_IRQ_FIFO_IRQn_Handler:

.section .text.SIO_IRQ_BELL_IRQn_Handler
.weak SIO_IRQ_BELL_IRQn_Handler
.global SIO_IRQ_BELL_IRQn_Handler
.type SIO_IRQ_BELL_IRQn_Handler, %function
.thumb_func
SIO_IRQ_BELL_IRQn_Handler:

.section .text.SIO_IRQ_FIFO_NS_IRQn_Handler
.weak SIO_IRQ_FIFO_NS_IRQn_Handler
.global SIO_IRQ_FIFO_NS_IRQn_Handler
.type SIO_IRQ_FIFO_NS_IRQn_Handler, %function
.thumb_func
SIO_IRQ_FIFO_NS_IRQn_Handler:

.section .text.SIO_IRQ_BELL_NS_IRQn_Handler
.weak SIO_IRQ_BELL_NS_IRQn_Handler
.global SIO_IRQ_BELL_NS_IRQn_Handler
.type SIO_IRQ_BELL_NS_IRQn_Handler, %function
.thumb_func
SIO_IRQ_BELL_NS_IRQn_Handler:

.section .text.SIO_IRQ_MTIMECMP_IRQn_Handler
.weak SIO_IRQ_MTIMECMP_IRQn_Handler
.global SIO_IRQ_MTIMECMP_IRQn_Handler
.type SIO_IRQ_MTIMECMP_IRQn_Handler, %function
.thumb_func
SIO_IRQ_MTIMECMP_IRQn_Handler:

.section .text.CLOCKS_IRQ_IRQn_Handler
.weak CLOCKS_IRQ_IRQn_Handler
.global CLOCKS_IRQ_IRQn_Handler
.type CLOCKS_IRQ_IRQn_Handler, %function
.thumb_func
CLOCKS_IRQ_IRQn_Handler:

.section .text.SPI0_IRQ_IRQn_Handler
.weak SPI0_IRQ_IRQn_Handler
.global SPI0_IRQ_IRQn_Handler
.type SPI0_IRQ_IRQn_Handler, %function
.thumb_func
SPI0_IRQ_IRQn_Handler:

.section .text.SPI1_IRQ_IRQn_Handler
.weak SPI1_IRQ_IRQn_Handler
.global SPI1_IRQ_IRQn_Handler
.type SPI1_IRQ_IRQn_Handler, %function
.thumb_func
SPI1_IRQ_IRQn_Handler:

.section .text.UART0_IRQ_IRQn_Handler
.weak UART0_IRQ_IRQn_Handler
.global UART0_IRQ_IRQn_Handler
.type UART0_IRQ_IRQn_Handler, %function
.thumb_func
UART0_IRQ_IRQn_Handler:

.section .text.UART1_IRQ_IRQn_Handler
.weak UART1_IRQ_IRQn_Handler
.global UART1_IRQ_IRQn_Handler
.type UART1_IRQ_IRQn_Handler, %function
.thumb_func
UART1_IRQ_IRQn_Handler:

.section .text.ADC_IRQ_FIFO_IRQn_Handler
.weak ADC_IRQ_FIFO_IRQn_Handler
.global ADC_IRQ_FIFO_IRQn_Handler
.type ADC_IRQ_FIFO_IRQn_Handler, %function
.thumb_func
ADC_IRQ_FIFO_IRQn_Handler:

.section .text.I2C0_IRQ_IRQn_Handler
.weak I2C0_IRQ_IRQn_Handler
.global I2C0_IRQ_IRQn_Handler
.type I2C0_IRQ_IRQn_Handler, %function
.thumb_func
I2C0_IRQ_IRQn_Handler:

.section .text.I2C1_IRQ_IRQn_Handler
.weak I2C1_IRQ_IRQn_Handler
.global I2C1_IRQ_IRQn_Handler
.type I2C1_IRQ_IRQn_Handler, %function
.thumb_func
I2C1_IRQ_IRQn_Handler:

.section .text.OTP_IRQ_IRQn_Handler
.weak OTP_IRQ_IRQn_Handler
.global OTP_IRQ_IRQn_Handler
.type OTP_IRQ_IRQn_Handler, %function
.thumb_func
OTP_IRQ_IRQn_Handler:

.section .text.TRNG_IRQ_IRQn_Handler
.weak TRNG_IRQ_IRQn_Handler
.global TRNG_IRQ_IRQn_Handler
.type TRNG_IRQ_IRQn_Handler, %function
.thumb_func
TRNG_IRQ_IRQn_Handler:

.section .text.PLL_SYS_IRQ_IRQn_Handler
.weak PLL_SYS_IRQ_IRQn_Handler
.global PLL_SYS_IRQ_IRQn_Handler
.type PLL_SYS_IRQ_IRQn_Handler, %function
.thumb_func
PLL_SYS_IRQ_IRQn_Handler:

.section .text.PLL_USB_IRQ_IRQn_Handler
.weak PLL_USB_IRQ_IRQn_Handler
.global PLL_USB_IRQ_IRQn_Handler
.type PLL_USB_IRQ_IRQn_Handler, %function
.thumb_func
PLL_USB_IRQ_IRQn_Handler:

.section .text.POWMAN_IRQ_POW_IRQn_Handler
.weak POWMAN_IRQ_POW_IRQn_Handler
.global POWMAN_IRQ_POW_IRQn_Handler
.type POWMAN_IRQ_POW_IRQn_Handler, %function
.thumb_func
POWMAN_IRQ_POW_IRQn_Handler:

.section .text.POWMAN_IRQ_TIMER_IRQn_Handler
.weak POWMAN_IRQ_TIMER_IRQn_Handler
.global POWMAN_IRQ_TIMER_IRQn_Handler
.type POWMAN_IRQ_TIMER_IRQn_Handler, %function
.thumb_func
POWMAN_IRQ_TIMER_IRQn_Handler:
