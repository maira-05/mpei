
CC          = gcc
N           = 800000
pointer     = UINTPTR_TYPE
OPTLEVEL	= -O2
memory      = "\"Main memory (heap)\""
CFLAGS+= -D$(pointer) -DCOMPILER_FLAGS="\"$(OPTLEVEL)\"" -DMEM_LOCATION=$(memory)
# ----------------------------------------------------------------------------

# add more compiler configurations as required

# add the targets and rules as required

run:
	./bin/coremark 0x0 0x0 0x66 $(N) 7 1 2000

.PHONY:clean
clean:
	rm -rf bin

