CXX := g++
CXXFLAGS := -Wall -g -lstdc++

OBJ_DIR := obj/
BIN_DIR := bin/
SRC_DIR := src/

BIN_NAME := os-build-utility

ifeq ($(OS), Windows_NT)
	BIN_NAME := $(BIN_NAME).exe
endif

SRC_FILES := $(wildcard $(SRC_DIR)*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)%.cpp,$(OBJ_DIR)%.o,$(SRC_FILES))

.PHONY: all clean

all: $(BIN_DIR)$(BIN_NAME)

$(BIN_DIR)$(BIN_NAME): $(OBJ_FILES)
	@if [ ! -d $(OBJ_DIR) ]; then @$mkdir $(BIN_DIR); fi
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ_FILES): $(SRC_FILES)
	@if [ ! -d $(OBJ_DIR) ]; then @$mkdir $(OBJ_DIR); fi
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJ_DIR)*.o $(BIN_DIR)$(BIN_NAME)
