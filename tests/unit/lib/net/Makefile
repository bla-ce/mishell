ROOT_DIR = ../../../..
LIB_DIR = $(ROOT_DIR)/lib
SRC_DIR = $(ROOT_DIR)/src

BIN_DIR = bin
BUILD_DIR = build

TEST_PATH = net_test

LIB_DIRS = $(shell find $(LIB_DIR) -type d -printf '-I$(LIB_DIR)/%P ')
SRC_DIRS = $(shell find $(SRC_DIR) -type d -printf '-I$(SRC_DIR)/%P ')

INCLUDE_FLAGS = $(LIB_DIRS) $(SRC_DIRS)

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
