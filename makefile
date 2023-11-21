SRC_DIR := src
INCLUDE_DIR := include

OBJ_DIR := obj
LIB_DIR := lib

CPP := g++

CPP_FLAGS := -Wall -Wextra -std=c++20 -O3 -fPIC -c
LD_FLAGS := -shared -lfmt

SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

TARGET := $(LIB_DIR)/liblumencpp.so

DEST_LIB_DIR := /usr/local/lib
DEST_INCLUDE_DIR := /usr/local/include

all: $(TARGET)

$(TARGET): $(OBJ_FILES) | $(LIB_DIR)
	$(CPP) -o $@ $^ $(LD_FLAGS) 

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CPP) -o $@ $< $(CPP_FLAGS)

$(OBJ_DIR) $(LIB_DIR):
	mkdir -p $@

install: $(TARGET) $(INCLUDE_DIR) | $(DEST_LIB_DIR) $(DEST_INCLUDE_DIR)
	cp $(TARGET) $(DEST_LIB_DIR)
	cp -r $(INCLUDE_DIR)/lumencpp $(DEST_INCLUDE_DIR)

clean:
	rm -rf $(LIB_DIR) $(OBJ_DIR)

.PHONY: all clean install
