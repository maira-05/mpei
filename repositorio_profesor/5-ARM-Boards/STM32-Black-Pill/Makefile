SUBDIRS += 1-led-blink
SUBDIRS += 2-led-blink
SUBDIRS += 3-input-butt
SUBDIRS += 4-custom-hal
SUBDIRS += 5-systick-delay
SUBDIRS += 6-CMSIS-example
SUBDIRS += 7-config-clock
SUBDIRS += 8-Interrupts-SysTick
SUBDIRS += 9-Interrupts-EXT
SUBDIRS += 10-Timers-TimeBase-Polling
SUBDIRS += 11-Timers-TimeBase-IRQ
SUBDIRS += 12-Timers-PWM-1
SUBDIRS += 13-Timers-IC-1
SUBDIRS += 14-UART-RX-TX-1
SUBDIRS += 15-UART-RX-TX-2
SUBDIRS += 16-UART-RX-TX-3
SUBDIRS += 17-UART-RX-TX-4
SUBDIRS += 18-SPI-1
SUBDIRS += 19-SPI-2
SUBDIRS += 20-SPI-3
SUBDIRS += 21-SPI-4
SUBDIRS += 22-I2C-1
SUBDIRS += 23-I2C-2
SUBDIRS += 24-ADC-1

.PHONY: all clean $(SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ all

clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
