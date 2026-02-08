#ifndef Q15_AXPY_H
#define Q15_AXPY_H

#include <stdint.h>
#include <stddef.h>

/**
 * Q15 AXPY operation: y[i] = sat_q15(a[i] + alpha * b[i])
 * 
 * @param n      Number of elements
 * @param alpha  Scalar multiplier (Q15 format)
 * @param a      Input vector A (Q15 format)
 * @param b      Input vector B (Q15 format)
 * @param y      Output vector Y (Q15 format)
 */

// Scalar reference implementation
void q15_axpy_scalar(size_t n, int16_t alpha, const int16_t *a, const int16_t *b, int16_t *y);

// RVV vector implementation
void q15_axpy_rvv(size_t n, int16_t alpha, const int16_t *a, const int16_t *b, int16_t *y);

#endif // Q15_AXPY_H
