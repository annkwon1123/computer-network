# Makefile for web server

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -pedantic

# Source files
SERVER_SRC = server.c
HTML_SRC = html.html

# Object files
SERVER_OBJ = server.o
HTML_OBJ = html.o

# Target executable
TARGET = server

all: $(TARGET)

$(TARGET): $(SERVER_OBJ) $(HTML_OBJ)
    $(CC) $(CFLAGS) $^ -o $@

$(SERVER_OBJ): $(SERVER_SRC)
    $(CC) $(CFLAGS) -c $< -o $@

$(HTML_OBJ): $(HTML_SRC)
    # No compilation necessary for HTML files
    @echo "HTML file is ready."
    touch $@

clean:
    rm -f $(TARGET) $(SERVER_OBJ) $(HTML_OBJ)
    @echo "Web server is clean."

.PHONY: all clean
