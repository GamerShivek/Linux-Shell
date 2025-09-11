CC = gcc
CFLAGS = -std=c99 \
         -D_POSIX_C_SOURCE=200809L \
         -D_XOPEN_SOURCE=700 \
         -Wall -Wextra -Werror \
         -Wno-unused-parameter \
         -fno-asm \
         -Iinclude

SRCDIR = src
TARGET = shell.out

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.c)

.PHONY: all clean debug release info

all: $(TARGET)

# Compile all source files directly to executable
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

# Debug build with debugging symbols
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

# Release build with optimizations
release: CFLAGS += -O3 -DNDEBUG
release: $(TARGET)

# Clean build artifacts
clean:
	rm -f $(TARGET)