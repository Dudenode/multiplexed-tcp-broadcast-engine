CC = gcc
CFLAGS = -g -Wall -Wextra -I./include

SRC_DIR = src
BIN_DIR = bin

TARGET = $(BIN_DIR)/server

# Automatically find all .c files in src/ and map them to .o files in bin/
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%.o, $(SRCS))

all: directories $(TARGET)

# Ensure the bin directory exists before compiling
directories:
	@mkdir -p $(BIN_DIR)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BIN_DIR)/*