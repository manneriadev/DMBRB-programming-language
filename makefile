#!/usr/bin/make -f
.RECIPEPREFIX = >

CXX := g++
CXXFLAGS := -std=c++23 -O2 -Wall -Wextra -Wno-missing-field-initializers -MMD -MP -Iinc
LDFLAGS :=

# On Windows link statically so dmbrb.exe runs without MSYS2 DLLs
ifeq ($(OS),Windows_NT)
LDFLAGS += -static
endif

SRC_DIR := src
OBJ_DIR := build/obj

TARGET := dmbrb

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

.PHONY: all clean rebuild test install uninstall

all: $(TARGET)

$(TARGET): $(OBJS)
> @echo "Linking $(TARGET)..."
> @$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
> @mkdir -p $(OBJ_DIR)
> @echo "Compiling $<..."
> @$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(OBJS:.o=.d)

clean:
> @echo "Cleaning..."
> @rm -rf build
> @rm -f $(TARGET) $(TARGET).exe

rebuild: clean all

test: $(TARGET)
> @bash tests/run_tests.sh

install: $(TARGET)
> @echo "Installing $(TARGET) to /usr/local/bin..."
> @sudo cp $(TARGET) /usr/local/bin/

uninstall:
> @echo "Removing $(TARGET)..."
> @sudo rm -f /usr/local/bin/$(TARGET)
