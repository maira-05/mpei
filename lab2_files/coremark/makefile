CC          = gcc
N           = 800000
pointer     = UINTPTR_TYPE
OPTLEVEL	= -O2
memory      = "\"Main memory (heap)\""
CFLAGS+= -Iinclude -D$(pointer) -DCOMPILER_FLAGS="\"$(OPTLEVEL)\"" -DMEM_LOCATION=$(memory)
# ----------------------------------------------------------------------------

# add more compiler configurations as required
core_list_join.o: src/core_list_join.c include/coremark.h makefile
	$(CC) $(CFLAGS) -c src/core_list_join.c

core_main.o: src/core_main.c include/coremark.h makefile
	$(CC) $(CFLAGS) -c src/core_main.c

core_matrix.o: src/core_matrix.c include/coremark.h makefile
	$(CC) $(CFLAGS) -c src/core_matrix.c

core_portme.o: src/core_portme.c include/coremark.h makefile
	$(CC) $(CFLAGS) -c src/core_portme.c

core_state.o: src/core_state.c include/coremark.h makefile
	$(CC) $(CFLAGS) -c src/core_state.c

core_util.o: src/core_util.c include/coremark.h makefile
	$(CC) $(CFLAGS) -c src/core_util.c

# ----------------------------------------------------------------------------
# Static library with all *.c except core_main.c
LIB_OBJS = core_list_join.o core_matrix.o core_portme.o core_state.o core_util.o

libcoremark.a: $(LIB_OBJS)
	ar rcs libcoremark.a $(LIB_OBJS)

# ----------------------------------------------------------------------------
# Link core_main.o against the library to build the final executable
bin/coremark.elf: core_main.o libcoremark.a
	mkdir -p bin
	$(CC) $(CFLAGS) core_main.o -L. -lcoremark -o bin/coremark.elf -lm

run: bin/coremark.elf
	./bin/coremark.elf 0x0 0x0 0x66 $(N) 7 1 2000


.PHONY: clean
clean:
	rm -rf bin *.o *.a