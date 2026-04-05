CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude
LDFLAGS = 

TARGET = huffarc
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build

SOURCES = $(SRC_DIR)/huffman.cpp \
          $(SRC_DIR)/bitstream.cpp \
          $(SRC_DIR)/crc32.cpp \
          $(SRC_DIR)/archive.cpp \
          $(SRC_DIR)/main.cpp

OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

.PHONY: all clean

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
