#include "q15_axpy.h"
#include <stdint.h>
#include <limits.h>

/**
 * Q15 saturation function
 * Clamps a 32-bit value to Q15 range: [-32768, 32767]
 */
static inline int16_t saturate_q15(int32_t val) {
    if (val > INT16_MAX) {
        return INT16_MAX;  // 32767
    } else if (val < INT16_MIN) {
        return INT16_MIN;  // -32768
    } else {
        return (int16_t)val;
    }
}

/**
 * Scalar reference implementation of Q15 AXPY
 */
void q15_axpy_scalar(size_t n, int16_t alpha, const int16_t *a, const int16_t *b, int16_t *y) {
    for (size_t i = 0; i < n; i++) {
        int32_t product = (int32_t)alpha * (int32_t)b[i];        
        int32_t sum = (int32_t)a[i] + product;
        y[i] = saturate_q15(sum);
    }
}
