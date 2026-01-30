/*
 * TensorLoad img2col Performance Test - conv3x3_s1x1_nopad
 * =========================================================
 * 
 * Test Configuration:
 *   - Input: H=40, W=40, C=32 (with pad)
 *   - Output: 9 output channels from 3x3 convolution
 *   - Load address base: 0x40000000
 *   - Store address base: 0x20000000
 */

#include "tl_perf_common.h"
#include <stdint.h>

/**
 * @brief Main function for img2col test
 * @return 0 on success
 */
int main(void) {
    // Input data address (loaded from hex file)
    uint32_t ld_addr_base = 0x40000000;
    
    // Output data address (where img2col results are stored)
    uint32_t st_addr_base = 0x20000000;
    
    // Call img2col function
    img2col_32x32x32_non_spase(ld_addr_base, st_addr_base);
    
    PRINT("img2col test completed!\n");
    
    return 0;
}
