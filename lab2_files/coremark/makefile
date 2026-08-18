CC          = gcc
N           = 800000
pointer     = UINTPTR_TYPE
OPTLEVEL	= -O2
memory      = "\"Main memory (heap)\""
CFLAGS+= -D$(pointer) -DCOMPILER_FLAGS="\"$(OPTLEVEL)\"" -DMEM_LOCATION=$(memory)
# ----------------------------------------------------------------------------

# add more compiler configurations as required
core_list_join.o: src/core_list_join.c include/coremark.h makefile
	$(CC) $(CFLAGS) -c src/core_list_join.c

core_main.o: src/core_main.c include/coremark.h makefile
	$(CC) $(CFLAGS) -c src/core_main.c

core_portme.o: src/core_portme.c include/coremark.h makefile
	$(CC) $(CFLAGS) -c src/core_portme.c

core_state.o: src/core_state.c include/coremark.h makefile
	$(CC) $(CFLAGS) -c src/core_state.c

core_util.o: src/core_util.c include/coremark.h makefile
	$(CC) $(CFLAGS) -c src/core_util.c
	
run:
	./bin/coremark 0x0 0x0 0x66 $(N) 7 1 2000


.PHONY:clean
clean:
	rm -rf bin