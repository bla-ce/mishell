BUILD_DIR = build
BIN_DIR = bin
SRC_DIR = src

MAIN_PATH = mishell

SRC_DIRS = $(shell find $(SRC_DIR) -type d -printf '-I$(SRC_DIR)/%P ')

INCLUDE_FLAGS = $(SRC_DIRS)

DEBUG_FLAGS = -g
BASE_FLAGS = -felf64 -w+all

herve:
	mkdir -p $(BUILD_DIR) $(BIN_DIR)
	nasm -o $(BUILD_DIR)/$(MAIN_PATH).o $(SRC_DIR)/$(MAIN_PATH).s \
		$(INCLUDE_FLAGS) $(DEBUG_FLAGS) $(BASE_FLAGS)
	ld -o $(BIN_DIR)/$(MAIN_PATH) $(BUILD_DIR)/$(MAIN_PATH).o

run:
	mkdir -p $(BUILD_DIR) $(BIN_DIR)
	nasm -o $(BUILD_DIR)/$(MAIN_PATH).o $(SRC_DIR)/$(MAIN_PATH).s \
		$(INCLUDE_FLAGS) $(DEBUG_FLAGS) $(BASE_FLAGS)
	ld -o $(BIN_DIR)/$(MAIN_PATH) $(BUILD_DIR)/$(MAIN_PATH).o
	./$(BIN_DIR)/$(MAIN_PATH)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
