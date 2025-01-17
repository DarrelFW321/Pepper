# Compiler and flags
CC = gcc
CFLAGS = -Wall -g
TARGET = pepper.exe  # Name of the output executable

# Automatically detect all .c files in the current directory
SRC = $(wildcard *.c)

# Automatically generate object files corresponding to the source files
OBJ = $(SRC:.c=.o)

# Build the target executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

# Rule to compile .c files to .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up (Windows-compatible cleanup)
clean:
	del /f /q $(OBJ) $(TARGET)
