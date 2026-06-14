BUILD_DIR = build
BIN_DIR = bin
SRC_DIR = src
CLI_DIR = cli
SERVICE_TYPES_DIR = service_types
LIB_DIR = lib

MISHELL_PATH = mishell
MISHLI_PATH = mishli

SRC_DIRS = $(shell find $(SRC_DIR) -type d -printf '-I$(SRC_DIR)/%P ')
SERVICE_TYPES_DIRS = $(shell find $(SERVICE_TYPES_DIR) -type d -printf '-I$(SERVICE_TYPES_DIR)/%P ')
LIB_DIRS = $(shell find $(LIB_DIR) -type d -printf '-I$(LIB_DIR)/%P ')

INCLUDE_FLAGS = $(SRC_DIRS) $(LIB_DIRS) $(SERVICE_TYPES_DIRS)

DEBUG_FLAGS = -g
BASE_FLAGS = -felf64 -w+all

mishli:
	mkdir -p $(BUILD_DIR) $(BIN_DIR)
	nasm -o $(BUILD_DIR)/$(MISHLI_PATH).o $(CLI_DIR)/$(MISHLI_PATH).s \
		$(INCLUDE_FLAGS) $(DEBUG_FLAGS) $(BASE_FLAGS)
	ld -o $(BIN_DIR)/$(MISHLI_PATH) $(BUILD_DIR)/$(MISHLI_PATH).o

mishell:
	mkdir -p $(BUILD_DIR) $(BIN_DIR)
	nasm -o $(BUILD_DIR)/$(MISHELL_PATH).o $(SRC_DIR)/$(MISHELL_PATH).s \
		$(INCLUDE_FLAGS) $(DEBUG_FLAGS) $(BASE_FLAGS)
	ld -o $(BIN_DIR)/$(MISHELL_PATH) $(BUILD_DIR)/$(MISHELL_PATH).o

strip:
	nasm -o $(BUILD_DIR)/$(MISHELL_PATH).o $(SRC_DIR)/$(MISHELL_PATH).s \
		$(INCLUDE_FLAGS) $(BASE_FLAGS)
	ld -o $(BIN_DIR)/$(MISHELL_PATH) $(BUILD_DIR)/$(MISHELL_PATH).o --strip-all

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

strace:
	strace ./$(BIN_DIR)/$(MISHELL_PATH)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
