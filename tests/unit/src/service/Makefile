ROOT_DIR = ../../../..
LIB_DIR = $(ROOT_DIR)/lib
SRC_DIR = $(ROOT_DIR)/src
SERVICE_TYPES_DIR = $(ROOT_DIR)/service_types

BIN_DIR = bin
BUILD_DIR = build

TEST_PATH = service_test

LIB_DIRS = $(shell find $(LIB_DIR) -type d -printf '-I$(LIB_DIR)/%P ')
SERVICE_TYPES_DIRS = $(shell find $(SERVICE_TYPES_DIR) -type d -printf '-I$(SERVICE_TYPES_DIR)/%P ')
SRC_DIRS = $(shell find $(SRC_DIR) -type d -printf '-I$(SRC_DIR)/%P ')

INCLUDE_FLAGS = $(SRC_DIRS) $(LIB_DIRS) $(SERVICE_TYPES_DIRS)

DEBUG_FLAGS = -g
BASE_FLAGS = -felf64 -w+all

all:
	mkdir -p $(BIN_DIR) $(BUILD_DIR)
	nasm -o $(BUILD_DIR)/$(TEST_PATH).o $(TEST_PATH).s \
		$(BASE_FLAGS) $(INCLUDE_FLAGS) $(DEBUG_FLAGS)
	ld -o $(BIN_DIR)/$(TEST_PATH) $(BUILD_DIR)/$(TEST_PATH).o
	./$(BIN_DIR)/$(TEST_PATH)

clean:
	rm -rf $(BIN_DIR) $(BUILD_DIR)
