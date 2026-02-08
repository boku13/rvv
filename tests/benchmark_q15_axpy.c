#include "q15_axpy.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// RISC-V cycle counter access
// Note: cycle CSR (0xC00) is aliased to the time CSR in user mode on some systems
static inline uint64_t read_cycles(void) {
#if defined(__riscv)
    #if __riscv_xlen == 64
        uint64_t cycles;
        // Use rdcycle pseudo-instruction (reads cycle CSR 0xC00)
        __asm__ volatile ("rdcycle %0" : "=r" (cycles));
        return cycles;
    #elif __riscv_xlen == 32
        uint32_t cycles_lo, cycles_hi, cycles_hi_check;
        // Read rdcycleh twice to handle overflow
        __asm__ volatile (
            "1:\n"
            "rdcycleh %0\n"
            "rdcycle  %1\n"
            "rdcycleh %2\n"
            "bne %0, %2, 1b\n"
            : "=r" (cycles_hi), "=r" (cycles_lo), "=r" (cycles_hi_check)
        );
        return ((uint64_t)cycles_hi << 32) | cycles_lo;
    #else
        return 0;
    #endif
#else
    return 0; // Not on RISC-V
#endif
}

// Initialize test data
static void init_test_data(int16_t *a, int16_t *b, size_t n, unsigned seed) {
    // Simple LCG for reproducible random numbers
    unsigned lcg = seed;
    for (size_t i = 0; i < n; i++) {
        lcg = lcg * 1103515245u + 12345u;
        a[i] = (int16_t)((lcg >> 16) & 0x7FFF) - 16384;  // Range: -16384 to 16383
        lcg = lcg * 1103515245u + 12345u;
        b[i] = (int16_t)((lcg >> 16) & 0x7FFF) - 16384;
    }
}

// Benchmark function
static void benchmark_size(size_t n, int16_t alpha, int iterations) {
    // Allocate aligned buffers
    int16_t *a = (int16_t *)malloc(n * sizeof(int16_t));
    int16_t *b = (int16_t *)malloc(n * sizeof(int16_t));
    int16_t *y_scalar = (int16_t *)malloc(n * sizeof(int16_t));
    int16_t *y_rvv = (int16_t *)malloc(n * sizeof(int16_t));
    
    if (!a || !b || !y_scalar || !y_rvv) {
        printf("Memory allocation failed\n");
        return;
    }
    
    // Initialize data
    init_test_data(a, b, n, 42);
    
    // Warm-up run
    q15_axpy_scalar(n, alpha, a, b, y_scalar);
    q15_axpy_rvv(n, alpha, a, b, y_rvv);
    
    // Benchmark scalar version
    uint64_t scalar_start = read_cycles();
    for (int i = 0; i < iterations; i++) {
        q15_axpy_scalar(n, alpha, a, b, y_scalar);
    }
    uint64_t scalar_end = read_cycles();
    uint64_t scalar_cycles = scalar_end - scalar_start;
    
    // Benchmark RVV version
    uint64_t rvv_start = read_cycles();
    for (int i = 0; i < iterations; i++) {
        q15_axpy_rvv(n, alpha, a, b, y_rvv);
    }
    uint64_t rvv_end = read_cycles();
    uint64_t rvv_cycles = rvv_end - rvv_start;
    
    // Calculate average cycles per iteration
    double scalar_avg = (double)scalar_cycles / iterations;
    double rvv_avg = (double)rvv_cycles / iterations;
    double speedup = scalar_avg / rvv_avg;
    
    // Verify correctness
    int correct = 1;
    for (size_t i = 0; i < n; i++) {
        if (y_scalar[i] != y_rvv[i]) {
            correct = 0;
            break;
        }
    }
    
    printf("%-8zu | %-12d | %-8d | %-20.1f | %-20.1f | %.2fx%-7s | %s\n",
           n, alpha, iterations, scalar_avg, rvv_avg, speedup, "",
           correct ? "MATCH" : "MISMATCH");
    
    free(a);
    free(b);
    free(y_scalar);
    free(y_rvv);
}

int main(void) {
    printf("=== Q15 AXPY Performance Benchmark ===\n");
    printf("Testing RVV vs Scalar implementation\n\n");
    
    // Check if we can read cycles
    uint64_t test_cycles = read_cycles();
    if (test_cycles == 0) {
        printf("Performance measurement may not work correctly\n\n");
    }
    
    printf("%-8s | %-12s | %-8s | %-20s | %-20s | %-12s | %s\n",
           "Size", "Alpha", "Iters", "Scalar (cycles)", "RVV (cycles)", "Speedup", "Correctness");
    printf("---------|--------------|----------|----------------------|----------------------|--------------|------------\n");
    
    benchmark_size(8, 12345, 10000);
    benchmark_size(16, 12345, 10000);
    benchmark_size(32, 12345, 5000);
    benchmark_size(64, 12345, 5000);
    benchmark_size(128, 12345, 2000);
    benchmark_size(256, 12345, 1000);
    benchmark_size(512, 12345, 500);
    benchmark_size(1024, 12345, 500);
    benchmark_size(2048, 12345, 200);
    benchmark_size(4096, 12345, 100);
    
    // Test non-power-of-2 sizes (vector-length agnostic test)
    printf("\n--- Non-aligned sizes ---\n");
    benchmark_size(127, 12345, 2000);
    benchmark_size(255, 12345, 1000);
    benchmark_size(1000, 12345, 500);
    
    printf("\n=== Benchmark Complete ===\n");
    
    return 0;
}
