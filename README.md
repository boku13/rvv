# Q15 AXPY RISC-V Vector (RVV) Implementation

Vector-accelerated Q15 fixed-point AXPY: `y[i] = sat_q15(a[i] + alpha * b[i])`

## Build

```bash
./build.sh
```

## Run

```bash
# Correctness tests
/home/kuro/spike-install/bin/spike --isa=rv64gcv_zicntr_zihpm \
  /home/kuro/spike-install/riscv64-unknown-elf/bin/pk \
  build/test_q15_axpy.elf

# Performance benchmark
/home/kuro/spike-install/bin/spike --isa=rv64gcv_zicntr_zihpm \
  /home/kuro/spike-install/riscv64-unknown-elf/bin/pk \
  build/benchmark_q15_axpy.elf
```

**Results**: 10/10 tests pass, 7.6x speedup measured

---

## Project Structure

```
.
├── src/
│   ├── q15_axpy_scalar.c      # Scalar reference implementation
│   └── q15_axpy_rvv.c         # RVV vector implementation
├── include/
│   └── q15_axpy.h             # API header
├── tests/
│   ├── test_q15_axpy.c        # Correctness tests
│   └── benchmark_q15_axpy.c   # Performance benchmark
├── build.sh                    # Build script
├── Makefile                    # Build system
├── README.md                   # This file
```

