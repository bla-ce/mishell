ROOT_DIR = ..
BUILD_DIR = build
BIN_DIR = bin

SRC_DIR = $(ROOT_DIR)/src
LIB_DIR = $(ROOT_DIR)/lib
SERVICE_TYPES_DIR = $(ROOT_DIR)/service_types

MISHLI_PATH = mishli

SRC_DIRS = $(shell find $(SRC_DIR) -type d -printf '-I$(SRC_DIR)/%P ')
LIB_DIRS = $(shell find $(LIB_DIR) -type d -printf '-I$(LIB_DIR)/%P ')
CLI_DIRS = $(shell find $(CLI_DIR) -type d -printf '-I$(CLI_DIR)/%P ')
SERVICE_TYPES_DIRS = $(shell find $(SERVICE_TYPES_DIR) -type d -printf '-I$(SERVICE_TYPES_DIR)/%P ')

INCLUDE_FLAGS = $(SRC_DIRS) $(LIB_DIRS) $(CLI_DIRS) $(SERVICE_TYPES_DIRS)

DEBUG_FLAGS = -g
BASE_FLAGS = -felf64 -w+all

mishli:
	mkdir -p $(BUILD_DIR) $(BIN_DIR)
	nasm -o $(BUILD_DIR)/$(MISHLI_PATH).o $(MISHLI_PATH).s \
		$(INCLUDE_FLAGS) $(DEBUG_FLAGS) $(BASE_FLAGS)
	ld -o $(BIN_DIR)/$(MISHLI_PATH) $(BUILD_DIR)/$(MISHLI_PATH).o

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
