#!/usr/bin/make -f
.RECIPEPREFIX = >

CXX := g++
CXXFLAGS := -std=c++23 -Wall -Wextra -g -Iinc

SRC_DIR := src
OBJ_DIR := build/obj

TARGET := dmbrb

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

.PHONY: all clean rebuild install uninstall

all: $(TARGET)

$(TARGET): $(OBJS)
> @echo "Linking $(TARGET)..."
> @$(CXX) $(OBJS) -o $(TARGET)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
> @mkdir -p $(OBJ_DIR)
> @echo "Compiling $<..."
> @$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
> @echo "Cleaning..."
> @rm -rf build
> @rm -f $(TARGET)

rebuild: clean all

install: $(TARGET)
> @echo "Installing $(TARGET) to /usr/local/bin..."
> @sudo cp $(TARGET) /usr/local/bin/

uninstall:
> @echo "Removing $(TARGET)..."
> @sudo rm -f /usr/local/bin/$(TARGET)