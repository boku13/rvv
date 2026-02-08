#!/bin/bash

# Build script for Q15 AXPY RVV implementation
# Supports both RV32 and RV64 architectures

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Default settings
BUILD_TYPE="release"
VERBOSE=0

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="debug"
            shift
            ;;
        --verbose)
            VERBOSE=1
            shift
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --debug           Build with debug symbols (-O0 -g)"
            echo "  --verbose         Show detailed compiler output"
            echo "  --help            Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# RV64 architecture settings
MARCH="rv64gcv"
MABI="lp64d"
TOOLCHAIN_PATH="${TOOLCHAIN_PATH:-/home/kuro/riscv-toolchain/riscv64}"
CC="${CC:-${TOOLCHAIN_PATH}/bin/riscv64-unknown-elf-gcc}"
OBJDUMP="${TOOLCHAIN_PATH}/bin/riscv64-unknown-elf-objdump"

# Set optimization flags
if [ "$BUILD_TYPE" = "debug" ]; then
    OPT_FLAGS="-O0 -g"
else
    OPT_FLAGS="-O2"
fi

# Output directory
BUILD_DIR="build"
mkdir -p "$BUILD_DIR"

echo -e "${GREEN}=== Building Q15 AXPY (RV64) ===${NC}"
echo "Compiler: $CC"
echo "March: $MARCH"
echo "Mabi: $MABI"
echo "Optimization: $OPT_FLAGS"
echo ""

# Common compiler flags
CFLAGS="-march=$MARCH -mabi=$MABI $OPT_FLAGS -Wall -Wextra -I./include"

# Add verbose flag if requested
if [ $VERBOSE -eq 1 ]; then
    CFLAGS="$CFLAGS -v"
fi

# Source files for test
TEST_SRCS="src/q15_axpy_scalar.c src/q15_axpy_rvv.c tests/test_q15_axpy.c"
BENCHMARK_SRCS="src/q15_axpy_scalar.c src/q15_axpy_rvv.c tests/benchmark_q15_axpy.c"

# Output binaries
TEST_OUTPUT="$BUILD_DIR/test_q15_axpy.elf"
BENCHMARK_OUTPUT="$BUILD_DIR/benchmark_q15_axpy.elf"

# Compile test binary
echo -e "${YELLOW}Compiling test binary...${NC}"
if $CC $CFLAGS $TEST_SRCS -o $TEST_OUTPUT; then
    echo -e "${GREEN}✓ Test build successful!${NC}"
    echo -e "Output: $TEST_OUTPUT"
else
    echo -e "${RED}✗ Test build failed!${NC}"
    exit 1
fi

# Compile benchmark binary
echo -e "${YELLOW}Compiling benchmark binary...${NC}"
if $CC $CFLAGS $BENCHMARK_SRCS -o $BENCHMARK_OUTPUT; then
    echo -e "${GREEN}✓ Benchmark build successful!${NC}"
    echo -e "Output: $BENCHMARK_OUTPUT"
else
    echo -e "${RED}✗ Benchmark build failed!${NC}"
    exit 1
fi

# Check for vector instructions in the binary
echo ""
echo -e "${YELLOW}Checking for RVV instructions...${NC}"
if command -v $OBJDUMP &> /dev/null; then
    echo "Searching for vector instructions in q15_axpy_rvv function:"
    $OBJDUMP -d $TEST_OUTPUT | grep -A 50 "q15_axpy_rvv>:" | grep -E "vsetvli|vle16|vse16|vwmul|vwadd|vnclip" | head -10 || echo "  (No vector instructions found - check if RVV is enabled)"
fi

echo ""
echo -e "${GREEN}To run on simulator:${NC}"
echo "  # Correctness tests:"
echo "  /home/kuro/spike-install/bin/spike --isa=rv64gcv_zicntr_zihpm /home/kuro/spike-install/riscv64-unknown-elf/bin/pk $TEST_OUTPUT"
echo ""
echo "  # Performance benchmark:"
echo "  /home/kuro/spike-install/bin/spike --isa=rv64gcv_zicntr_zihpm /home/kuro/spike-install/riscv64-unknown-elf/bin/pk $BENCHMARK_OUTPUT"

