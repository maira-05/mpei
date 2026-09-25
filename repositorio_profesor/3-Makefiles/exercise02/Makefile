# -------------------------------------------------------
# Compiler configuration
# -------------------------------------------------------

# Define the compiler
CC = gcc

# Compiler flags
# -Wall : enable all common warnings
CFLAGS = -Wall

# -------------------------------------------------------
# Project files and target
# -------------------------------------------------------

# The name of the final executable
TARGET = math_program.elf

# Source files
SRCS = main.c maths_util.c

# Object files (replace .c with .o)
OBJS = $(SRCS:.c=.o)

# -------------------------------------------------------
# Default rule
# -------------------------------------------------------

# 'all' is the default target — it builds the executable
all: $(TARGET)

# -------------------------------------------------------
# Linking stage
# -------------------------------------------------------

# Link all object files into the final executable
# $@ = target name (math_program.elf)
# $^ = all dependencies (object files)
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# -------------------------------------------------------
# Compilation stage
# -------------------------------------------------------

# Rule to compile main.c into main.o
main.o: main.c maths_util.h
	$(CC) $(CFLAGS) -c main.c -o main.o

# Rule to compile maths_util.c into maths_util.o
# It depends on maths_util.h so if the header changes,
# maths_util.o will be rebuilt.
maths_util.o: maths_util.c maths_util.h
	$(CC) $(CFLAGS) -c maths_util.c -o maths_util.o

# -------------------------------------------------------
# Cleanup rule
# -------------------------------------------------------

# Remove all object files and the executable
clean:
	rm -f $(OBJS) $(TARGET)

# -------------------------------------------------------
# Special declarations
# -------------------------------------------------------

# Mark targets that do not correspond to actual files
.PHONY: all clean