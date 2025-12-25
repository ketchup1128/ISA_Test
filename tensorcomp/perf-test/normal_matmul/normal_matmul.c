/*
 * TensorComp tc.st.mma.acc Performance Test - Normal Mode (M2=2, N2=2)
 * =====================================================================
 * 
 * Test Configuration:
 *   - Mode: Normal
 *   - Left Parallelism: 2x (M2=2)
 *   - Top Parallelism: 2x (N2=2)
 *   - Channel: 128
 */

#include "tc_perf_common.h"
#include <stdint.h>
#include <stdio.h>

int main(void) {
    // PRINT("======================================================================\n");
    // PRINT("TensorComp Performance Test \n");
    // PRINT("======================================================================\n");

    // Pointer to configuration structure at fixed address
    matmul_config_t *config_ptr = (matmul_config_t *)0xa0004000;
    // printf("config_ptr: %p\n", config_ptr);
    // printf("config_ptr->mode: %x\n", config_ptr->mode);
    // printf("config_ptr->addr_left: %x\n", config_ptr->addr_left);
    // printf("config_ptr->addr_top: %x\n", config_ptr->addr_top);ß
    // printf("config_ptr->acc_addr: %x\n", config_ptr->acc_addr);
    // printf("config_ptr->ch_loop: %x\n", config_ptr->ch_loop);
    // my name is liumin, I'm SB.
    
    if (config_ptr->mode == (TC_MODE_NORMAL | (TC_PARAL_2X << TC_LEFT_PARAL_SHIFT) | (TC_PARAL_2X << TC_TOP_PARAL_SHIFT))) {
        normal_matmul_2x2_ch128(config_ptr);
    } else if (config_ptr->mode == (TC_MODE_NORMAL | (TC_PARAL_4X << TC_LEFT_PARAL_SHIFT) | (TC_PARAL_1X << TC_TOP_PARAL_SHIFT))) {
        normal_matmul_4x1_ch128(config_ptr);
    } else if (config_ptr->mode == (TC_MODE_SPARSE | (TC_PARAL_4X << TC_LEFT_PARAL_SHIFT) | (TC_PARAL_2X << TC_TOP_PARAL_SHIFT))) {
        spa_matmul_ch128(config_ptr);
    } else {
        // PRINT("Invalid mode!\n");
        return 1;
    }
    
    // PRINT("\n======================================================================\n");
    // PRINT("test completed!\n");
    // PRINT("======================================================================\n");
    return 0;
}
