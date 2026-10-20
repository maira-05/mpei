APPNAME = main

#Variables de las herramientas de compilación
# Compilador de C para ARM
CC = arm-none-eabi-gcc 
# Flags de compilación
CFLAGS = -Wall -Wextra -std=c11 -ffreestanding
# Arquitectura objetivo 
ARCH = -mcpu=cortex-m4 
# Script de enlazador 
LINKERSCRIPT = linker_script.ld 


#CMSIS dirección
CMSIS = ../STM32utils/Drivers/CMSIS
#STM32 definiciones de registros
STM32F4 =$(CMSIS)/Device/ST/STM32F4xx


#incluir directorios para buscar archivos *.h
INCLUDE = -I./inc
INCLUDE += -I.
INCLUDE += -I$(STM32F4)/Include
INCLUDE += -I$(CMSIS)/Core/Include

# Archivos fuente en C
C_SRC = $(wildcard *.c) 
# Archivos fuente en lenguaje de ensamblador
ASM_SRC = $(wildcard *.S) 

 # Convertir la lista de archivos .c en una lista de archivos objeto .o
OBJS = $(patsubst %.c, %.o, $(C_SRC))
# Convertir la lista de archivos .S en una lista de archivos objeto .o
OBJS += $(patsubst %.S, %.o, $(ASM_SRC)) 

# Optimización para depuración y generación de información de depuración
CFLAGS += -Og -g 
 # No generar código independiente de posición
CFLAGS += -fno-pic
#Especificaciones para sistemas embebidos y biblioteca estándar mínima
CFLAGS += $(ARCH) --specs=nosys.specs --specs=nano.specs 

# Usar el script de enlazador especificado
LDFLAGS += -T$(LINKERSCRIPT)
# No usa las librerias estándar del sistema operativo
LDFLAGS += -nostdlib 
# No realizar relajación de enlaces
LDFLAGS += -Wl,--no-relax 

# Agregar las bibliotecas estándar de C y GCC
LDLIBS = -lc -lgcc 

all: $(APPNAME).elf

# Regla para generar el archivo ELF
$(APPNAME).elf: $(OBJS) 
	@echo $(OBJS) 
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@
	arm-none-eabi-size -A -x $@

# Regla para generar archivos objeto a partir de archivos ensamblador
%.o: %.S 
	$(CC) $(CFLAGS) -c $^ -o $@

# Regla para generar archivos objeto a partir de archivos C
%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@

#ejecuta la limpieza de archivos generados
.PHONY: clean 
clean:
	rm -f *.o *.elf *.img

#ejecuta la carga del programa en la placa
.PHONY: flash 
flash: $(APPNAME).elf
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program $(APPNAME).elf verify reset exit"