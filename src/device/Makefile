# Directories
SRC_DIR = src
INC_DIR = include
BIN_DIR = bin

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -I$(INC_DIR) -pthread -g 

# Source files and object files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%.o, $(SRCS))

# Target binary
TARGET = $(BIN_DIR)/http_api

# Default target
all: $(TARGET)

# Linking
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Compile .c to .o inside bin
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -rf $(BIN_DIR)

# Run the app
run: $(TARGET)
	./$(TARGET)
