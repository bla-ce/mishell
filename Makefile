BUILD_DIR = build
BIN_DIR = bin
SRC_DIR = src
LIB_DIR = lib

MISHELL_PATH = mishell

SRC_DIRS = $(shell find $(SRC_DIR) -type d -printf '-I$(SRC_DIR)/%P ')
LIB_DIRS = $(shell find $(LIB_DIR) -type d -printf '-I$(LIB_DIR)/%P ')

INCLUDE_FLAGS = $(SRC_DIRS) $(LIB_DIRS)

DEBUG_FLAGS = -g
BASE_FLAGS = -felf64 -w+all

mishell:
	mkdir -p $(BUILD_DIR) $(BIN_DIR)
	nasm -o $(BUILD_DIR)/$(MISHELL_PATH).o $(SRC_DIR)/$(MISHELL_PATH).s \
		$(INCLUDE_FLAGS) $(DEBUG_FLAGS) $(BASE_FLAGS)
	ld -o $(BIN_DIR)/$(MISHELL_PATH) $(BUILD_DIR)/$(MISHELL_PATH).o

mishli:
	$(MAKE) -C cli

run-init:
	mkdir -p $(BUILD_DIR) $(BIN_DIR)
	nasm -o $(BUILD_DIR)/$(MISHELL_PATH).o $(SRC_DIR)/$(MISHELL_PATH).s \
		$(INCLUDE_FLAGS) $(DEBUG_FLAGS) $(BASE_FLAGS)
	ld -o $(BIN_DIR)/$(MISHELL_PATH) $(BUILD_DIR)/$(MISHELL_PATH).o
	./$(BIN_DIR)/$(MISHELL_PATH) init --port $(PORT)

run-connect:
	mkdir -p $(BUILD_DIR) $(BIN_DIR)
	nasm -o $(BUILD_DIR)/$(MISHELL_PATH).o $(SRC_DIR)/$(MISHELL_PATH).s \
		$(INCLUDE_FLAGS) $(DEBUG_FLAGS) $(BASE_FLAGS)
	ld -o $(BIN_DIR)/$(MISHELL_PATH) $(BUILD_DIR)/$(MISHELL_PATH).o
	./$(BIN_DIR)/$(MISHELL_PATH) connect $(REMOTE_IP) $(REMOTE_PORT) --port $(PORT)

test-unit:
	$(MAKE) -C tests/unit

test-e2e:
	$(MAKE) -C tests/e2e

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
