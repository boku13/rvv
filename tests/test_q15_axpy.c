#include "q15_axpy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

// Test result tracking
static int tests_passed = 0;
static int tests_failed = 0;

/**
 * Compare two Q15 vectors for bit-exact equality
 */
static int compare_vectors(const int16_t *expected, const int16_t *actual, size_t n, const char *test_name) {
    for (size_t i = 0; i < n; i++) {
        if (expected[i] != actual[i]) {
            printf("FAIL: %s - Mismatch at index %zu: expected %d, got %d\n", 
                   test_name, i, expected[i], actual[i]);
            tests_failed++;
            return 0;
        }
    }
    printf("PASS: %s\n", test_name);
    tests_passed++;
    return 1;
}

/**
 * Test 1: Alpha = 0 (result should equal a)
 */
static void test_alpha_zero(void) {
    const size_t n = 16;
    int16_t a[16] = {100, -200, 300, -400, 500, -600, 700, -800,
                     900, -1000, 1100, -1200, 1300, -1400, 1500, -1600};
    int16_t b[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    int16_t y_scalar[16];
    int16_t y_rvv[16];
    
    q15_axpy_scalar(n, 0, a, b, y_scalar);
    q15_axpy_rvv(n, 0, a, b, y_rvv);
    
    compare_vectors(y_scalar, y_rvv, n, "Alpha = 0");
}

/**
 * Test 2: Positive saturation
 */
static void test_positive_saturation(void) {
    const size_t n = 8;
    int16_t a[8] = {32000, 32000, 32000, 32000, 32000, 32000, 32000, 32000};
    int16_t b[8] = {100, 200, 300, 400, 500, 600, 700, 800};
    int16_t alpha = 100;  // Will cause overflow
    int16_t y_scalar[8];
    int16_t y_rvv[8];
    
    q15_axpy_scalar(n, alpha, a, b, y_scalar);
    q15_axpy_rvv(n, alpha, a, b, y_rvv);
    
    compare_vectors(y_scalar, y_rvv, n, "Positive saturation");
}

/**
 * Test 3: Negative saturation
 */
static void test_negative_saturation(void) {
    const size_t n = 8;
    int16_t a[8] = {-32000, -32000, -32000, -32000, -32000, -32000, -32000, -32000};
    int16_t b[8] = {100, 200, 300, 400, 500, 600, 700, 800};
    int16_t alpha = -100;  // Will cause negative overflow
    int16_t y_scalar[8];
    int16_t y_rvv[8];
    
    q15_axpy_scalar(n, alpha, a, b, y_scalar);
    q15_axpy_rvv(n, alpha, a, b, y_rvv);
    
    compare_vectors(y_scalar, y_rvv, n, "Negative saturation");
}

/**
 * Test 4: Maximum values
 */
static void test_max_values(void) {
    const size_t n = 4;
    int16_t a[4] = {INT16_MAX, INT16_MIN, INT16_MAX, INT16_MIN};
    int16_t b[4] = {INT16_MAX, INT16_MIN, INT16_MIN, INT16_MAX};
    int16_t alpha = INT16_MAX;
    int16_t y_scalar[4];
    int16_t y_rvv[4];
    
    q15_axpy_scalar(n, alpha, a, b, y_scalar);
    q15_axpy_rvv(n, alpha, a, b, y_rvv);
    
    compare_vectors(y_scalar, y_rvv, n, "Maximum values");
}

/**
 * Test 5: Minimum values
 */
static void test_min_values(void) {
    const size_t n = 4;
    int16_t a[4] = {INT16_MIN, INT16_MIN, INT16_MIN, INT16_MIN};
    int16_t b[4] = {INT16_MIN, INT16_MIN, INT16_MIN, INT16_MIN};
    int16_t alpha = INT16_MIN;
    int16_t y_scalar[4];
    int16_t y_rvv[4];
    
    q15_axpy_scalar(n, alpha, a, b, y_scalar);
    q15_axpy_rvv(n, alpha, a, b, y_rvv);
    
    compare_vectors(y_scalar, y_rvv, n, "Minimum values");
}

/**
 * Test 6: Random vectors (medium size)
 */
static void test_random_medium(void) {
    const size_t n = 64;
    int16_t a[64], b[64];
    int16_t y_scalar[64], y_rvv[64];
    int16_t alpha = 12345;
    
    // Generate random data
    for (size_t i = 0; i < n; i++) {
        a[i] = (int16_t)(rand() % 65536 - 32768);
        b[i] = (int16_t)(rand() % 65536 - 32768);
    }
    
    q15_axpy_scalar(n, alpha, a, b, y_scalar);
    q15_axpy_rvv(n, alpha, a, b, y_rvv);
    
    compare_vectors(y_scalar, y_rvv, n, "Random medium (n=64)");
}

/**
 * Test 7: Random vectors (large size)
 */
static void test_random_large(void) {
    const size_t n = 1024;
    int16_t *a = malloc(n * sizeof(int16_t));
    int16_t *b = malloc(n * sizeof(int16_t));
    int16_t *y_scalar = malloc(n * sizeof(int16_t));
    int16_t *y_rvv = malloc(n * sizeof(int16_t));
    int16_t alpha = -7890;
    
    if (!a || !b || !y_scalar || !y_rvv) {
        printf("FAIL: Memory allocation failed\n");
        tests_failed++;
        goto cleanup;
    }
    
    // Generate random data
    for (size_t i = 0; i < n; i++) {
        a[i] = (int16_t)(rand() % 65536 - 32768);
        b[i] = (int16_t)(rand() % 65536 - 32768);
    }
    
    q15_axpy_scalar(n, alpha, a, b, y_scalar);
    q15_axpy_rvv(n, alpha, a, b, y_rvv);
    
    compare_vectors(y_scalar, y_rvv, n, "Random large (n=1024)");
    
cleanup:
    free(a);
    free(b);
    free(y_scalar);
    free(y_rvv);
}

/**
 * Test 8: Non-aligned size (tests VL handling)
 */
static void test_non_aligned_size(void) {
    const size_t n = 127;  // Prime number, likely not aligned to VL
    int16_t *a = malloc(n * sizeof(int16_t));
    int16_t *b = malloc(n * sizeof(int16_t));
    int16_t *y_scalar = malloc(n * sizeof(int16_t));
    int16_t *y_rvv = malloc(n * sizeof(int16_t));
    int16_t alpha = 1000;
    
    if (!a || !b || !y_scalar || !y_rvv) {
        printf("FAIL: Memory allocation failed\n");
        tests_failed++;
        goto cleanup;
    }
    
    for (size_t i = 0; i < n; i++) {
        a[i] = (int16_t)(i * 100);
        b[i] = (int16_t)(i * 50);
    }
    
    q15_axpy_scalar(n, alpha, a, b, y_scalar);
    q15_axpy_rvv(n, alpha, a, b, y_rvv);
    
    compare_vectors(y_scalar, y_rvv, n, "Non-aligned size (n=127)");
    
cleanup:
    free(a);
    free(b);
    free(y_scalar);
    free(y_rvv);
}

/**
 * Test 9: Single element
 */
static void test_single_element(void) {
    const size_t n = 1;
    int16_t a[1] = {12345};
    int16_t b[1] = {6789};
    int16_t alpha = 2;
    int16_t y_scalar[1];
    int16_t y_rvv[1];
    
    q15_axpy_scalar(n, alpha, a, b, y_scalar);
    q15_axpy_rvv(n, alpha, a, b, y_rvv);
    
    compare_vectors(y_scalar, y_rvv, n, "Single element");
}

/**
 * Test 10: Alpha = 1 (identity on b)
 */
static void test_alpha_one(void) {
    const size_t n = 32;
    int16_t a[32], b[32];
    int16_t y_scalar[32], y_rvv[32];
    
    for (size_t i = 0; i < n; i++) {
        a[i] = (int16_t)(i * 100);
        b[i] = (int16_t)(i * 200);
    }
    
    q15_axpy_scalar(n, 1, a, b, y_scalar);
    q15_axpy_rvv(n, 1, a, b, y_rvv);
    
    compare_vectors(y_scalar, y_rvv, n, "Alpha = 1");
}

/**
 * Main test runner
 */
int main(void) {
    printf("=== Q15 AXPY RVV Correctness Tests ===\n\n");
    
    // Seed random number generator
    srand(42);  // Fixed seed for reproducibility
    
    // Run all tests
    test_alpha_zero();
    test_positive_saturation();
    test_negative_saturation();
    test_max_values();
    test_min_values();
    test_random_medium();
    test_random_large();
    test_non_aligned_size();
    test_single_element();
    test_alpha_one();
    
    // Print summary
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);
    
    if (tests_failed == 0) {
        printf("\n✓ All tests passed! RVV implementation is bit-exact with scalar.\n");
        return 0;
    } else {
        printf("\n✗ Some tests failed. Please review implementation.\n");
        return 1;
    }
}
