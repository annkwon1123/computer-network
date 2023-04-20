# Makefile for web server

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -pedantic

# Source files
SERVER_SRC = server.c
HTML_SRC = html.html
CSS_SRC = index.css
JS_SRC = index.js

# Object files
SERVER_OBJ = server.o
HTML_OBJ = html.o
CSS_OBJ = index.o
JS_OBJ = index.o

# Target executable
TARGET = server

all: $(TARGET)

$(TARGET): $(SERVER_OBJ) $(HTML_OBJ) $(CSS_OBJ) $(JS_OBJ)
    $(CC) $(CFLAGS) $^ -o $@

$(SERVER_OBJ): $(SERVER_SRC)
    $(CC) $(CFLAGS) -c $< -o $@

$(HTML_OBJ): $(HTML_SRC)
    # No compilation necessary for HTML files
    @echo "HTML file is ready."
    touch $@

$(CSS_OBJ): $(CSS_SRC)
    $(CC) $(CFLAGS) -c $< -o $@

$(JS_OBJ): $(JS_SRC)
    $(CC) $(CFLAGS) -c $< -o $@

clean:
    rm -f $(TARGET) $(SERVER_OBJ) $(HTML_OBJ) $(CSS_OBJ) $(JS_OBJ)
    @echo "Web server is clean."

.PHONY: all clean
