# Compiler
CC = gcc

# Compiler Flags : -Wall for all warnings, -Wextra for extra warnings, -g for debugging
CFLAGS = -Wall -Wextra -g

# Name of final executable
TARGET = myshell

# Source files
SRCS = main.c prompt.c execute.c signals.c history.c cd.c echo.c pwd.c

# Object files
OBJS = $(SRCS:.c=.o)

# Default target
# This is the first target, so it runs by default when you type 'make'
all: $(TARGET)

# Linker rule : How to build the final executable $(TARGET) from object files $(OBJS)
# It depends on all object files
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Compilation rule : How to build .o files from .c files
# It depends on the .c file AND our main header file
# If shell.h changes, all object files need to be recompiled
%.o: %.c shell.h
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target: Removes all built files
# .PHONY tells make that 'clean' is a special command, not a file
.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)