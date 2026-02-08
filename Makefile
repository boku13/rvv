# Makefile for Q15 AXPY RVV implementation
# RV64 architecture

# Architecture settings
MARCH := rv64gcv
MABI := lp64d
TOOLCHAIN_PATH := /home/kuro/riscv-toolchain/riscv64
CC := $(TOOLCHAIN_PATH)/bin/riscv64-unknown-elf-gcc
OBJDUMP := $(TOOLCHAIN_PATH)/bin/riscv64-unknown-elf-objdump

# Compiler flags
CFLAGS := -march=$(MARCH) -mabi=$(MABI) -O2 -Wall -Wextra -I./include
LDFLAGS :=

# Debug build
ifeq ($(DEBUG),1)
    CFLAGS := $(CFLAGS:-O2=-O0 -g)
endif

# Directories
SRC_DIR := src
TEST_DIR := tests
BUILD_DIR := build
INC_DIR := include

# Source files
SRCS := $(SRC_DIR)/q15_axpy_scalar.c \
        $(SRC_DIR)/q15_axpy_rvv.c \
        $(TEST_DIR)/test_q15_axpy.c

# Object files
OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(notdir $(SRCS)))

# Output binaries
TARGET := $(BUILD_DIR)/test_q15_axpy.elf
BENCHMARK := $(BUILD_DIR)/benchmark_q15_axpy.elf

# Default target
.PHONY: all
all: $(TARGET) $(BENCHMARK)
	@echo "✓ Build complete"
	@echo "  Test:      $(TARGET)"
	@echo "  Benchmark: $(BENCHMARK)"

# Create build directory
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Link target
$(TARGET): $(OBJS) | $(BUILD_DIR)
	@echo "Linking $(TARGET)..."
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

# Compile source files
$(BUILD_DIR)/q15_axpy_scalar.o: $(SRC_DIR)/q15_axpy_scalar.c $(INC_DIR)/q15_axpy.h | $(BUILD_DIR)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/q15_axpy_rvv.o: $(SRC_DIR)/q15_axpy_rvv.c $(INC_DIR)/q15_axpy.h | $(BUILD_DIR)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/test_q15_axpy.o: $(TEST_DIR)/test_q15_axpy.c $(INC_DIR)/q15_axpy.h | $(BUILD_DIR)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/benchmark_q15_axpy.o: $(TEST_DIR)/benchmark_q15_axpy.c $(INC_DIR)/q15_axpy.h | $(BUILD_DIR)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Link benchmark
BENCHMARK_OBJS := $(BUILD_DIR)/q15_axpy_scalar.o $(BUILD_DIR)/q15_axpy_rvv.o $(BUILD_DIR)/benchmark_q15_axpy.o
$(BENCHMARK): $(BENCHMARK_OBJS) | $(BUILD_DIR)
	@echo "Linking $(BENCHMARK)..."
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

# Disassembly
.PHONY: disasm
disasm: $(TARGET)
	@echo "Disassembling $(TARGET)..."
	$(OBJDUMP) -d $(TARGET) > $(BUILD_DIR)/disasm.txt
	@echo "  Output: $(BUILD_DIR)/disasm.txt"

# Check for RVV instructions
.PHONY: check-rvv
check-rvv: $(TARGET)
	@echo "Checking for RVV instructions in q15_axpy_rvv..."
	@$(OBJDUMP) -d $(TARGET) | grep -A 50 "q15_axpy_rvv>:" | grep -E "vsetvli|vle16|vse16|vwmul|vwadd|vnclip" || echo "  Warning: No RVV instructions found"

# Clean
.PHONY: clean
clean:
	rm -rf build build_rv32 build_rv64

# Clean all
.PHONY: distclean
distclean: clean
	rm -f *.elf *.o

# Help
.PHONY: help
help:
	@echo "Q15 AXPY RVV Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all        Build test binary (default)"
	@echo "  disasm     Generate disassembly"
	@echo "  check-rvv  Check for RVV instructions"
	@echo "  clean      Remove build artifacts"
	@echo "  help       Show this message"
	@echo ""
	@echo "Variables:"
	@echo "  ARCH       Target architecture: rv32 or rv64 (default: rv32)"
	@echo "  DEBUG      Build with debug symbols: DEBUG=1"
	@echo ""
	@echo "Examples:"
	@echo "  make                    # Build for RV32"
	@echo "  make ARCH=rv64          # Build for RV64"
	@echo "  make DEBUG=1            # Debug build"
	@echo "  make disasm ARCH=rv64   # Disassemble RV64 build"
