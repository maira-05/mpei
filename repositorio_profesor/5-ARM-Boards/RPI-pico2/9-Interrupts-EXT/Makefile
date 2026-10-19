APPNAME = main
MACHINE = netduinoplus2


CC =  arm-none-eabi-gcc
CFLAGS = -Wall -Wextra -std=c11 -ffreestanding
OBJCOPY =  arm-none-eabi-objcopy
OBJDUMP =  arm-none-eabi-objdump
LINK = arm-none-eabi-ld
ARCH =  -mcpu=cortex-m33 -mthumb -march=armv8-m.main+fp+dsp -mfloat-abi=softfp
LINKERSCRIPT = linker_script.ld
BUILD = build

PICOTOOL = picotool
PICOTOOL_FAMILY_ID = rp2350-arm-s

#CMSIS directory
CMSIS = ../CMSIS_6/CMSIS
#STM32 register definitions
# Add include directories to search for *.h files
INCLUDE = -I./inc
INCLUDE += -I.
INCLUDE += -I$(CMSIS)/Core/Include


ifeq ($(DBG),1)	
	QEMU = qemu-system-arm -M $(MACHINE) -kernel $(APPNAME).elf -serial mon:stdio -nographic --no-reboot -S -s
else
	QEMU = qemu-system-arm -M $(MACHINE) -kernel $(APPNAME).elf -serial mon:stdio -nographic --no-reboot -S -s
endif

C_SRC = $(wildcard *.c)
C_SRC += $(wildcard src/*.c)
# add some smt32f4 helper functions

ASM_SRC = $(wildcard *.S)
# Remove the path dirs and keep only the filenames
OBJS = $(patsubst %.c, $(BUILD)/%.o, $(notdir $(C_SRC)))
OBJS += $(patsubst %.S, $(BUILD)/%.o, $(notdir  $(ASM_SRC)))

# we add the paths where to find the *.c and *.S files
vpath %.c $(sort $(dir $(C_SRC))) 
vpath %.S $(sort $(dir $(ASM_SRC))) 

CFLAGS+=-Og -g 
CFLAGS+=-fno-pic
CFLAGS+=$(ARCH) --specs=nosys.specs --specs=nano.specs $(INCLUDE) -DCORE_FAMILY_ARM

LDFLAGS +=-T$(LINKERSCRIPT) 
LDFLAGS +=-nostdlib -DCORE_FAMILY_ARM
LDFLAGS +=-Wl,--no-relax 

LDLIBS = -lc -lgcc 

all: $(APPNAME).elf 

$(APPNAME).elf: $(OBJS) 
	@echo $(OBJS) $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@ 
	arm-none-eabi-size -A -x $@

$(BUILD)/%.o: %.S 
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c $^ -o $@ 
	
$(BUILD)/%.o: %.c 
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS)  -c $< -o $@ 

.PHONY: run
run: $(APPNAME).elf
	$(QEMU) -bios $(APPNAME).elf 

$(BUILD): 
	mkdir -p $@

.PHONY: debug
debug: $(APPNAME).elf
	gdb-multiarch $(APPNAME).elf -ex "target remote :1234" -ex "tb Reset_Handler" 

.PHONY: clean
clean:	
	rm -f *.o *.elf *.img src/*.o 
	rm -rf  $(BUILD)


.PHONY: flash
flash: $(APPNAME).elf
	@$(PICOTOOL) uf2 convert --quiet $(APPNAME).elf $(APPNAME).uf2 --family $(PICOTOOL_FAMILY_ID) --abs-block
	@$(PICOTOOL) load -x $(APPNAME).uf2 
	#openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program $(APPNAME).elf verify reset exit"