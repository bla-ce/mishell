BUILD_DIR = build
BIN_DIR = bin
SRC_DIR = src
SERVICE_TYPES_DIR = service_types
LIB_DIR = lib

MAIN_PATH = mishell

SRC_DIRS = $(shell find $(SRC_DIR) -type d -printf '-I$(SRC_DIR)/%P ')
SERVICE_TYPES_DIRS = $(shell find $(SERVICE_TYPES_DIR) -type d -printf '-I$(SERVICE_TYPES_DIR)/%P ')
LIB_DIRS = $(shell find $(LIB_DIR) -type d -printf '-I$(LIB_DIR)/%P ')

INCLUDE_FLAGS = $(SRC_DIRS) $(LIB_DIRS) $(SERVICE_TYPES_DIRS)

DEBUG_FLAGS = -g
BASE_FLAGS = -felf64 -w+all

mishell:
	mkdir -p $(BUILD_DIR) $(BIN_DIR)
	nasm -o $(BUILD_DIR)/$(MAIN_PATH).o $(SRC_DIR)/$(MAIN_PATH).s \
		$(INCLUDE_FLAGS) $(DEBUG_FLAGS) $(BASE_FLAGS)
	ld -o $(BIN_DIR)/$(MAIN_PATH) $(BUILD_DIR)/$(MAIN_PATH).o

strip:
	nasm -o $(BUILD_DIR)/$(MAIN_PATH).o $(SRC_DIR)/$(MAIN_PATH).s \
		$(INCLUDE_FLAGS) $(BASE_FLAGS)
	ld -o $(BIN_DIR)/$(MAIN_PATH) $(BUILD_DIR)/$(MAIN_PATH).o --strip-all

run-init:
	mkdir -p $(BUILD_DIR) $(BIN_DIR)
	nasm -o $(BUILD_DIR)/$(MAIN_PATH).o $(SRC_DIR)/$(MAIN_PATH).s \
		$(INCLUDE_FLAGS) $(DEBUG_FLAGS) $(BASE_FLAGS)
	ld -o $(BIN_DIR)/$(MAIN_PATH) $(BUILD_DIR)/$(MAIN_PATH).o
	./$(BIN_DIR)/$(MAIN_PATH) init

run-connect:
	mkdir -p $(BUILD_DIR) $(BIN_DIR)
	nasm -o $(BUILD_DIR)/$(MAIN_PATH).o $(SRC_DIR)/$(MAIN_PATH).s \
		$(INCLUDE_FLAGS) $(DEBUG_FLAGS) $(BASE_FLAGS)
	ld -o $(BIN_DIR)/$(MAIN_PATH) $(BUILD_DIR)/$(MAIN_PATH).o
	./$(BIN_DIR)/$(MAIN_PATH) connect 127.0.0.1 7474

test-e2e:
	python3 tests/e2e/client.py

test-unit:
	$(MAKE) -C tests/unit

strace:
	strace ./$(BIN_DIR)/$(MAIN_PATH)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
