CC       = gcc
OPTLEVEL = -O2
CFLAGS  += $(OPTLEVEL)
# ----------------------------------------------------------------------------

intarray.o: intarray.c intarray.h makefile
	$(CC) $(CFLAGS) -c intarray.c

sinewave.o: sinewave.c intarray.h makefile
	$(CC) $(CFLAGS) -c sinewave.c

# ----------------------------------------------------------------------------
# Static library with all *.c except sinewave.c (the one with main)
LIB_OBJS = intarray.o

libsinewave.a: $(LIB_OBJS)
	ar rcs libsinewave.a $(LIB_OBJS)

# ----------------------------------------------------------------------------
# Link sinewave.o against the library to build the final executable
bin/sinewave.elf: sinewave.o libsinewave.a
	mkdir -p bin
	$(CC) $(CFLAGS) sinewave.o -L. -lsinewave -o bin/sinewave.elf -lm

run: bin/sinewave.elf
	./bin/sinewave.elf

.PHONY: clean
clean:
	rm -rf bin *.o *.a