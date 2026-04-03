SHELL := /bin/bash

CXX ?= g++
WIN_CXX ?= x86_64-w64-mingw32-g++-posix

SRC := $(wildcard src/*.cpp)
INC := -Iinclude
BIN_DIR := bin

DEBUG_OUT := $(BIN_DIR)/myapp
RELEASE_OUT := $(BIN_DIR)/myapp_o3
WIN_OUT := $(BIN_DIR)/katafish.exe

COMMON_WARN := -Wall -Wextra -pedantic-errors

.PHONY: help debug baseline release win-static win-static-debug win-static-opt run run-debug clean

help:
	@echo "KataFish build targets"
	@echo ""
	@echo "  make debug           Build Linux/macOS debug binary ($(DEBUG_OUT))"
	@echo "  make baseline        Build Linux/macOS O3 baseline binary ($(RELEASE_OUT))"
	@echo "  make release         Build Linux/macOS O3 release binary ($(RELEASE_OUT))"
	@echo "  make win-static      Build Windows static binary with MinGW POSIX ($(WIN_OUT))"
	@echo "  make win-static-debug Build Windows static debug binary ($(WIN_OUT))"
	@echo "  make win-static-opt  Build Windows static optimized binary ($(WIN_OUT))"
	@echo "  make run             Build and run Linux/macOS O3 binary ($(RELEASE_OUT))"
	@echo "  make run-debug       Build and run Linux/macOS debug binary ($(DEBUG_OUT))"
	@echo "  make clean           Remove build artifacts from $(BIN_DIR)"
	@echo ""
	@echo "Override compilers if needed:"
	@echo "  make win-static WIN_CXX=x86_64-w64-mingw32-g++-posix"
	@echo "  make debug CXX=clang++"

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

debug: $(BIN_DIR)
	$(CXX) -g -std=c++17 $(COMMON_WARN) -Weffc++ -Wno-unused-parameter \
	-fsanitize=undefined,address $(INC) $(SRC) -o $(DEBUG_OUT)

release: $(BIN_DIR)
	$(CXX) -std=c++17 -O3 -DNDEBUG $(COMMON_WARN) -Weffc++ -Wno-unused-parameter \
	$(INC) $(SRC) -o $(RELEASE_OUT)

baseline: release

win-static: $(BIN_DIR)
	$(WIN_CXX) -std=c++17 -O3 -DNDEBUG $(COMMON_WARN) -pthread \
	-static -static-libgcc -static-libstdc++ \
	$(INC) $(SRC) -o $(WIN_OUT)

win-static-debug: $(BIN_DIR)
	$(WIN_CXX) -g -std=c++17 $(COMMON_WARN) -pthread \
	-static -static-libgcc -static-libstdc++ \
	$(INC) $(SRC) -o $(WIN_OUT)

win-static-opt: $(BIN_DIR)
	$(WIN_CXX) -std=c++17 -O3 -DNDEBUG -march=native -flto $(COMMON_WARN) -pthread \
	-static -static-libgcc -static-libstdc++ \
	$(INC) $(SRC) -o $(WIN_OUT)

run: release
	./$(RELEASE_OUT)

run-debug: debug
	./$(DEBUG_OUT)

clean:
	rm -f $(BIN_DIR)/myapp $(BIN_DIR)/myapp_o3 $(BIN_DIR)/katafish.exe
