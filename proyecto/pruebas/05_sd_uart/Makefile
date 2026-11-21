APPNAME = main
CC = arm-none-eabi-gcc
CFLAGS = -Wall -Wextra -std=c11 -ffreestanding
ARCH = -mcpu=cortex-m4
LINKERSCRIPT = linker_script.ld
INCLUDE = -I./inc
INCLUDE += -I./inc/cmsis
C_SRC = $(wildcard *.c)
C_SRC += $(wildcard src/*.c)
ASM_SRC = $(wildcard *.S)
OBJS = $(patsubst %.c, %.o, $(C_SRC))
OBJS += $(patsubst %.S, %.o, $(ASM_SRC))
CFLAGS += -Og -g
CFLAGS += -fno-pic
CFLAGS += $(ARCH) --specs=nosys.specs --specs=nano.specs
CFLAGS += $(INCLUDE)
CFLAGS += -DSTM32F401xE
LDFLAGS += -T$(LINKERSCRIPT)
LDFLAGS += -nostdlib
LDFLAGS += -Wl,--no-relax
LDLIBS = -lc -lgcc
all: $(APPNAME).elf
$(APPNAME).elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@
	arm-none-eabi-size -A -x $@
%.o: %.S
	$(CC) $(CFLAGS) -c $^ -o $@
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
.PHONY: clean flash
clean:
	rm -f *.o src/*.o *.elf *.img

flash: $(APPNAME).elf
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program $(APPNAME).elf verify reset exit"
