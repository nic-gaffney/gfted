EXEC = gfted
CC = gcc
BIN_DIR = bin
SRC_DIR = src
BUILD_DIR = build

INCLUDE_DIR = include
CFLAGS = -Wall -Wextra -I$(INCLUDE_DIR) -MMD -MP
LD_FLAGS = 

C = $(sort $(shell find $(SRC_DIR) -name '*.c'))
C_OBJ = $(C:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
OBJS = $(C_OBJ)

.PHONY: all
all: $(BIN_DIR)/$(EXEC)

run: all
	./$(BIN_DIR)/$(EXEC)

$(BIN_DIR)/$(EXEC): $(C_OBJ)
	mkdir -p $(BIN_DIR)
	$(CC) $(LD_FLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(BIN_DIR)
