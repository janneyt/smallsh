CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Werror -g
TARGET = smallsh
SRCDIR = .

# Source files
SRCS = $(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/*/*.c)

# Object files
OBJS = $(SRCS:.c=.o)

# Include directories
INCDIRS = tester utilities constants input builtins error expansion parsing signal

# Libraries
LIBS =

# Add include directories to include path
INCFLAGS = $(addprefix -I,$(INCDIRS))

# Link object files and libraries into the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $@

# Compile each source file into an object file
%.o: %.c
	$(CC) $(CFLAGS) $(INCFLAGS) -c $< -o $@

# Clean up generated files
clean:
	rm -f $(TARGET) $(OBJS)

