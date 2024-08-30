CXX := g++
CXXFLAGS := -Wall -g

OBJ_DIR := obj/
BIN_DIR := bin/
SRC_DIR := src/

BIN_NAME := os-build-utility

SRC_FILES := $(wildcard $(SRC_DIR)*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)%.cpp, $(OBJ_DIR)%.o, $(SRC_FILES))

.PHONY: all clean

all: $(BIN_DIR)$(BIN_NAME)

$(BIN_DIR)$(BIN_NAME): $(OBJ_FILES)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)%.o: $(SRC_FILES)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJ_DIR)*.o $(BIN_DIR)$(BIN_NAME)
