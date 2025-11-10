# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -g

# The name of the final executable
TARGET = myshell

# Source files
SRCS = main.c prompt.c execute.c signals.c history.c cd.c echo.c pwd.c

# Object files
OBJS = $(SRCS:.c=.o)

# Header files
HDRS = globals.h prompt.h execute.h signals.h history.h cd.h echo.h pwd.h

# Default target
all: $(TARGET)

# Linker rule
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Updated Compilation rule
%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

# --- NEW TARGET ---
# Run target: Compiles first (depends on 'all'), then runs the executable
run: all
	./$(TARGET)

# Clean target: Removes all built files
.PHONY: clean run
clean:
	rm -f $(TARGET) $(OBJS)