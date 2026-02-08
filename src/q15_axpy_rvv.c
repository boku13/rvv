#include "q15_axpy.h"

#ifdef __riscv_vector
#include <riscv_vector.h>

/**
 * RVV implementation of Q15 AXPY
 * 
 * Operation: y[i] = sat_q15(a[i] + alpha * b[i])
 * 
 */
void q15_axpy_rvv(size_t n, int16_t alpha, const int16_t *a, const int16_t *b, int16_t *y) {
    size_t vl;
    
    // Vector-length agnostic loop
    while (n > 0) {
        // Determine vector length for this iteration
        // e16 = 16-bit elements, m1 = LMUL=1 (1 register group)
        vl = __riscv_vsetvl_e16m1(n);
        
        // Load input vectors a and b
        vint16m1_t va = __riscv_vle16_v_i16m1(a, vl);
        vint16m1_t vb = __riscv_vle16_v_i16m1(b, vl);
        
        // Widening multiply - alpha * b[i]
        // Result is vint32m2 (doubled register usage due to widening)
        // This prevents overflow: int16 * int16 → int32
        vint32m2_t v_product = __riscv_vwmul_vx_i32m2(vb, alpha, vl);
        
        // Widening add - a[i] + product
        // Widen va from int16 to int32 and add to product
        // This prevents overflow: int16 + int32 → int32
        vint32m2_t v_sum = __riscv_vwadd_wv_i32m2(v_product, va, vl);
        
        // Saturating narrow with clip - int32 → sat_q15
        // vnclip shifts right by 0 bits and saturates to int16 range
        // The 'x' variant uses a scalar for the shift amount
        // Saturation ensures [-32768, 32767] range
        vint16m1_t v_result = __riscv_vnclip_wx_i16m1(v_sum, 0, vl);
        
        // Store result
        __riscv_vse16_v_i16m1(y, v_result, vl);
        
        // Advance pointers and decrement count
        a += vl;
        b += vl;
        y += vl;
        n -= vl;
    }
}

#else
// fallback
void q15_axpy_rvv(size_t n, int16_t alpha, const int16_t *a, const int16_t *b, int16_t *y) {
    q15_axpy_scalar(n, alpha, a, b, y);
}
#endif
