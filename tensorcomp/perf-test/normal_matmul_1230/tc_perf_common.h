/*
 * TensorComp Performance Test - Common Functions Header
 * ======================================================
 * 
 * This header file contains common functions used by TensorComp performance tests.
 * Include this file in your test files to use these functions.
 */

#ifndef TC_PERF_COMMON_H
#define TC_PERF_COMMON_H

#include <stdint.h>
#include "tc_common.h"

#ifdef PRINTF_SUPPORT
#include "printf.h"
#define PRINT(...) printf(__VA_ARGS__)
#else
#define PRINT(...)
#endif

// ============================================================================
// Matmul Configuration Structure
// ============================================================================
typedef struct {
    uint32_t addr_left;    // Default: 0x40000000
    uint32_t addr_top;     // Default: 0x40004000
    uint32_t addr_spa;     // Default: 0x40008000
    uint32_t acc_addr;     // Default: 0x50000000 (ACC_BUFFER_ADDR)
    uint32_t ch_loop;      // Default: 0x00000001
    uint32_t mode;         // Default: 0x00000110 (TC_MODE_NORMAL | (TC_PARAL_2X << TC_LEFT_PARAL_SHIFT) | (TC_PARAL_2X << TC_TOP_PARAL_SHIFT))
} matmul_config_t;

// Default initialization example (2x2 mode):
// matmul_config_t config = {
//     .addr_left = 0x40000000,
//     .addr_top = 0x40004000,
//     .acc_addr = 0x50000000,
//     .ch_loop = 0x00000001,
//     .mode = 0x00000020  // TC_MODE_NORMAL | (TC_PARAL_4X << TC_LEFT_PARAL_SHIFT) | (TC_PARAL_1X << TC_TOP_PARAL_SHIFT)
// };

// ============================================================================
// Helper: Load data into TC registers using t.ld
// ============================================================================
static inline void load_tc_config(uint32_t tmask_ld, uint32_t twidth_ld, uint32_t tld_offset[8]) {
    // PRINT("\n--- Loading data into TC registers using t.ld ---\n");
    
    // Configure CSRs for t.ld
    CSR_WRITE(CSR_TMASK_LD, tmask_ld);       // Enable 8 x 128-byte loads based on mask
    CSR_WRITE(CSR_TWIDTH_LD, twidth_ld);     // twidth_ld bytes
    
    // Configure offsets
    configure_toffset_ld_8(tld_offset[0], tld_offset[1], tld_offset[2], tld_offset[3], 
                           tld_offset[4], tld_offset[5], tld_offset[6], tld_offset[7]);
    
    // PRINT("--- TC load config completed ---\n\n");
}

// ============================================================================
// Mode Config: Normal Mode (M2=4, N2=1)
// ============================================================================
static inline void mode_config_4x1(uint32_t mode, uint32_t tlength, uint32_t twidth) {
    // PRINT("\n=== Mode config 4x1 ===\n");
    
    // Configure CSRs
    CSR_WRITE(CSR_TENSOR_CORE_MODE, mode);
    CSR_WRITE(CSR_TWIDTH, twidth);  // 128 bytes per block
    CSR_WRITE(CSR_TLENGTH, tlength);
    
    // For COMP STORE, mask and width should be 0
    CSR_WRITE(CSR_TMASK_ST, 0);
    CSR_WRITE(CSR_TWIDTH_ST, 0);
    
    // PRINT("Mode config 4x1 completed\n");
}

// ============================================================================
// Mode Config: Normal Mode (M2=2, N2=2)
// ============================================================================
static inline void mode_config_2x2(uint32_t mode, uint32_t tlength, uint32_t twidth) {
    // PRINT("\n=== Mode config 2x2 ===\n");
    
    // Configure CSRs
    CSR_WRITE(CSR_TENSOR_CORE_MODE, mode);
    CSR_WRITE(CSR_TWIDTH, twidth);  // 128 bytes per block
    CSR_WRITE(CSR_TLENGTH, tlength);
    
    // For COMP STORE, mask and width should be 0
    CSR_WRITE(CSR_TMASK_ST, 0);
    CSR_WRITE(CSR_TWIDTH_ST, 0);
    
    // PRINT("Mode config 2x2 completed\n");
}

// ============================================================================
// Matmul 4x1: Matrix Multiply-Accumulate for 4x1 mode (M2=4, N2=1)
// Processes 128 channels worth of data
// ============================================================================
static inline void matmul_4x1_ch128(uint32_t addr_left, uint32_t addr_top, uint32_t acc_addr, uint32_t rs2_config[8]) {
    // PRINT("\n=== Matmul 4x1 ch128 ===\n");
    
    // TC_ST_CLEAN(acc_addr);
    
    // rs2_config for 4x1 mode

    
    // First group
    T_LD(0, 0, addr_top);
    T_LD(1, 0, addr_left);
    TC_ST_MMA_ACC(acc_addr, 1, 0, rs2_config[0]);
    TC_ST_MMA_ACC(acc_addr, 1, 0, rs2_config[1]);
    T_LD(2, 1, addr_left);
    TC_ST_MMA_ACC(acc_addr, 2, 0, rs2_config[2]);
    TC_ST_MMA_ACC(acc_addr, 2, 0, rs2_config[3]);
    T_LD(3, 2, addr_left);
    TC_ST_MMA_ACC(acc_addr, 3, 0, rs2_config[4]);
    TC_ST_MMA_ACC(acc_addr, 3, 0, rs2_config[5]);
    T_LD(4, 3, addr_left);
    TC_ST_MMA_ACC(acc_addr, 4, 0, rs2_config[6]);
    TC_ST_MMA_ACC(acc_addr, 4, 0, rs2_config[7]);

    // Second group
    T_LD(5, 1, addr_top);
    T_LD(6, 4, addr_left);
    TC_ST_MMA_ACC(acc_addr, 6, 5, rs2_config[0]);
    TC_ST_MMA_ACC(acc_addr, 6, 5, rs2_config[1]);
    T_LD(7, 5, addr_left);
    TC_ST_MMA_ACC(acc_addr, 7, 5, rs2_config[2]);
    TC_ST_MMA_ACC(acc_addr, 7, 5, rs2_config[3]);
    T_LD(8, 6, addr_left);
    TC_ST_MMA_ACC(acc_addr, 8, 5, rs2_config[4]);
    TC_ST_MMA_ACC(acc_addr, 8, 5, rs2_config[5]);
    T_LD(9, 7, addr_left);
    TC_ST_MMA_ACC(acc_addr, 9, 5, rs2_config[6]);
    TC_ST_MMA_ACC(acc_addr, 9, 5, rs2_config[7]);

    
    // Third group
    T_LD(0, 2, addr_top);
    T_LD(1, 8, addr_left);
    TC_ST_MMA_ACC(acc_addr, 1, 0, rs2_config[0]);
    TC_ST_MMA_ACC(acc_addr, 1, 0, rs2_config[1]);
    T_LD(2, 9, addr_left);
    TC_ST_MMA_ACC(acc_addr, 2, 0, rs2_config[2]);
    TC_ST_MMA_ACC(acc_addr, 2, 0, rs2_config[3]);
    T_LD(3, 10, addr_left);
    TC_ST_MMA_ACC(acc_addr, 3, 0, rs2_config[4]);
    TC_ST_MMA_ACC(acc_addr, 3, 0, rs2_config[5]);
    T_LD(4, 11, addr_left);
    TC_ST_MMA_ACC(acc_addr, 4, 0, rs2_config[6]);
    TC_ST_MMA_ACC(acc_addr, 4, 0, rs2_config[7]);

    // Fourth group
    T_LD(5, 3, addr_top);
    T_LD(6, 12, addr_left);
    TC_ST_MMA_ACC(acc_addr, 6, 5, rs2_config[0]);
    TC_ST_MMA_ACC(acc_addr, 6, 5, rs2_config[1]);
    T_LD(7, 13, addr_left);
    TC_ST_MMA_ACC(acc_addr, 7, 5, rs2_config[2]);
    TC_ST_MMA_ACC(acc_addr, 7, 5, rs2_config[3]);
    T_LD(8, 14, addr_left);
    TC_ST_MMA_ACC(acc_addr, 8, 5, rs2_config[4]);
    TC_ST_MMA_ACC(acc_addr, 8, 5, rs2_config[5]);
    T_LD(9, 15, addr_left);
    TC_ST_MMA_ACC(acc_addr, 9, 5, rs2_config[6]);
    TC_ST_MMA_ACC(acc_addr, 9, 5, rs2_config[7]);

}

// ============================================================================
// Matmul 2x2: Matrix Multiply-Accumulate for 2x2 mode (M2=2, N2=2)
// Processes 128 channels worth of data
// ============================================================================
static inline void matmul_2x2_ch128(uint32_t addr_left, uint32_t addr_top, uint32_t acc_addr, uint32_t rs2_config[4]) {
    
    // rs2_config for 2x2 mode
    
    // First group
    T_LD(0, 0, addr_left);
    T_LD(1, 0, addr_top);
    TC_ST_MMA_ACC(acc_addr, 0, 1, rs2_config[0]);
    TC_ST_MMA_ACC(acc_addr, 0, 1, rs2_config[1]);
    TC_ST_MMA_ACC(acc_addr, 0, 1, rs2_config[2]);
    TC_ST_MMA_ACC(acc_addr, 0, 1, rs2_config[3]);
    T_LD(2, 1, addr_left);
    T_LD(3, 1, addr_top);
    TC_ST_MMA_ACC(acc_addr, 2, 3, rs2_config[0]);
    TC_ST_MMA_ACC(acc_addr, 2, 3, rs2_config[1]);
    TC_ST_MMA_ACC(acc_addr, 2, 3, rs2_config[2]);
    TC_ST_MMA_ACC(acc_addr, 2, 3, rs2_config[3]);
    T_LD(4, 2, addr_left);
    T_LD(5, 2, addr_top);
    TC_ST_MMA_ACC(acc_addr, 4, 5, rs2_config[0]);
    TC_ST_MMA_ACC(acc_addr, 4, 5, rs2_config[1]);
    TC_ST_MMA_ACC(acc_addr, 4, 5, rs2_config[2]);
    TC_ST_MMA_ACC(acc_addr, 4, 5, rs2_config[3]);
    T_LD(6, 3, addr_left);
    T_LD(7, 3, addr_top);
    TC_ST_MMA_ACC(acc_addr, 6, 7, rs2_config[0]);
    TC_ST_MMA_ACC(acc_addr, 6, 7, rs2_config[1]);
    TC_ST_MMA_ACC(acc_addr, 6, 7, rs2_config[2]);
    TC_ST_MMA_ACC(acc_addr, 6, 7, rs2_config[3]);

    
    // Second group
    T_LD(0, 4, addr_left);
    T_LD(1, 4, addr_top);
    TC_ST_MMA_ACC(acc_addr, 0, 1, rs2_config[0]);
    TC_ST_MMA_ACC(acc_addr, 0, 1, rs2_config[1]);
    TC_ST_MMA_ACC(acc_addr, 0, 1, rs2_config[2]);
    TC_ST_MMA_ACC(acc_addr, 0, 1, rs2_config[3]);
    T_LD(2, 5, addr_left);
    T_LD(3, 5, addr_top);
    TC_ST_MMA_ACC(acc_addr, 2, 3, rs2_config[0]);
    TC_ST_MMA_ACC(acc_addr, 2, 3, rs2_config[1]);
    TC_ST_MMA_ACC(acc_addr, 2, 3, rs2_config[2]);
    TC_ST_MMA_ACC(acc_addr, 2, 3, rs2_config[3]);
    T_LD(4, 6, addr_left);
    T_LD(5, 6, addr_top);
    TC_ST_MMA_ACC(acc_addr, 4, 5, rs2_config[0]);
    TC_ST_MMA_ACC(acc_addr, 4, 5, rs2_config[1]);
    TC_ST_MMA_ACC(acc_addr, 4, 5, rs2_config[2]);
    TC_ST_MMA_ACC(acc_addr, 4, 5, rs2_config[3]);
    T_LD(6, 7, addr_left);
    T_LD(7, 7, addr_top);
    TC_ST_MMA_ACC(acc_addr, 6, 7, rs2_config[0]);
    TC_ST_MMA_ACC(acc_addr, 6, 7, rs2_config[1]);
    TC_ST_MMA_ACC(acc_addr, 6, 7, rs2_config[2]);
    TC_ST_MMA_ACC(acc_addr, 6, 7, rs2_config[3]);
}

// ============================================================================
// Matmul 4x1: Matrix Multiply-Accumulate for 4x1 mode (M2=4, N2=1)
// Processes 128 channels worth of data
// ============================================================================
static inline void spa_matmul_4x2_ch128(uint32_t addr_left, uint32_t addr_top, uint32_t addr_spa, uint32_t acc_addr, uint32_t mma_rs2_config[8], uint32_t merge_rs2_config[4]) {
    // PRINT("\n=== Matmul 4x1 ch128 ===\n");
    
    // TC_ST_CLEAN(acc_addr);




    
    // First group
    T_LD(0, 0, addr_spa);

        T_LD(1, 0, addr_top);
        T_MERGE_128B(2, 1, 0, merge_rs2_config[0]); // spa: 0, top: 1 ->spa_top: 2
        T_LD(3, 0, addr_left); // left: 3
        TC_ST_MMA_ACC(acc_addr, 3, 2, mma_rs2_config[0]); // mma: left: 3, spa_top: 2, congif_0
        TC_ST_MMA_ACC(acc_addr, 3, 2, mma_rs2_config[1]);// mma: left: 3, spa_top: 2, congif_1

        T_MERGE_128B(4, 1, 0, merge_rs2_config[1]); // spa: 0, top: 1 ->spa_top: 5
        T_LD(5, 1, addr_left); // left: 4
        TC_ST_MMA_ACC(acc_addr, 5, 4, mma_rs2_config[2]);
        TC_ST_MMA_ACC(acc_addr, 5, 4, mma_rs2_config[3]);

        T_LD(6, 1, addr_top); // spa: 0, top: 6 ->spa_top: 7
        T_MERGE_128B(7, 1, 6, merge_rs2_config[0]);
        T_LD(8, 2, addr_left);// left: 8
        TC_ST_MMA_ACC(acc_addr, 8, 7, mma_rs2_config[4]);
        TC_ST_MMA_ACC(acc_addr, 8, 7, mma_rs2_config[5]);

        T_MERGE_128B(9, 1, 6, merge_rs2_config[1]);
        T_LD(10, 2, addr_left);// left: 8
        TC_ST_MMA_ACC(acc_addr, 10, 9, mma_rs2_config[6]);
        TC_ST_MMA_ACC(acc_addr, 10, 9, mma_rs2_config[7]);



        T_LD(11, 2, addr_top);
        T_MERGE_128B(12, 1, 11, merge_rs2_config[2]); // spa: 0, top: 1 ->spa_top: 2
        T_LD(13, 4, addr_left); // left: 3
        TC_ST_MMA_ACC(acc_addr, 13, 12, mma_rs2_config[0]); // mma: left: 3, spa_top: 2, congif_0
        TC_ST_MMA_ACC(acc_addr, 13, 12, mma_rs2_config[1]);// mma: left: 3, spa_top: 2, congif_1

        T_MERGE_128B(14, 1, 11, merge_rs2_config[3]); // spa: 0, top: 1 ->spa_top: 5
        T_LD(15, 5, addr_left); // left: 4
        TC_ST_MMA_ACC(acc_addr, 15, 14, mma_rs2_config[2]);
        TC_ST_MMA_ACC(acc_addr, 15, 14, mma_rs2_config[3]);

        T_LD(16, 3, addr_top); // spa: 0, top: 6 ->spa_top: 7
        T_MERGE_128B(17, 1, 16, merge_rs2_config[2]);
        T_LD(18, 6, addr_left);// left: 8
        TC_ST_MMA_ACC(acc_addr, 18, 17, mma_rs2_config[4]);
        TC_ST_MMA_ACC(acc_addr, 18, 17, mma_rs2_config[5]);

        T_MERGE_128B(19, 1, 16, merge_rs2_config[3]);
        T_LD(20, 7, addr_left);// left: 8
        TC_ST_MMA_ACC(acc_addr, 20, 19, mma_rs2_config[6]);
        TC_ST_MMA_ACC(acc_addr, 20, 19, mma_rs2_config[7]);



                // First group
        T_LD(0, 1, addr_spa);

        T_LD(1, 4, addr_top);
        T_MERGE_128B(2, 1, 0, merge_rs2_config[0]); // spa: 0, top: 1 ->spa_top: 2
        T_LD(3, 8, addr_left); // left: 3
        TC_ST_MMA_ACC(acc_addr, 3, 2, mma_rs2_config[0]); // mma: left: 3, spa_top: 2, congif_0
        TC_ST_MMA_ACC(acc_addr, 3, 2, mma_rs2_config[1]);// mma: left: 3, spa_top: 2, congif_1

        T_MERGE_128B(4, 1, 0, merge_rs2_config[1]); // spa: 0, top: 1 ->spa_top: 5
        T_LD(5, 9, addr_left); // left: 4
        TC_ST_MMA_ACC(acc_addr, 5, 4, mma_rs2_config[2]);
        TC_ST_MMA_ACC(acc_addr, 5, 4, mma_rs2_config[3]);

        T_LD(6, 5, addr_top); // spa: 0, top: 6 ->spa_top: 7
        T_MERGE_128B(7, 1, 6, merge_rs2_config[0]);
        T_LD(8, 10, addr_left);// left: 8
        TC_ST_MMA_ACC(acc_addr, 8, 7, mma_rs2_config[4]);
        TC_ST_MMA_ACC(acc_addr, 8, 7, mma_rs2_config[5]);

        T_MERGE_128B(9, 1, 6, merge_rs2_config[1]);
        T_LD(10, 11, addr_left);// left: 8
        TC_ST_MMA_ACC(acc_addr, 10, 9, mma_rs2_config[6]);
        TC_ST_MMA_ACC(acc_addr, 10, 9, mma_rs2_config[7]);



        T_LD(11, 6, addr_top);
        T_MERGE_128B(12, 1, 11, merge_rs2_config[2]); // spa: 0, top: 1 ->spa_top: 2
        T_LD(13, 12, addr_left); // left: 3
        TC_ST_MMA_ACC(acc_addr, 13, 12, mma_rs2_config[0]); // mma: left: 3, spa_top: 2, congif_0
        TC_ST_MMA_ACC(acc_addr, 13, 12, mma_rs2_config[1]);// mma: left: 3, spa_top: 2, congif_1

        T_MERGE_128B(14, 1, 11, merge_rs2_config[3]); // spa: 0, top: 1 ->spa_top: 5
        T_LD(15, 13, addr_left); // left: 4
        TC_ST_MMA_ACC(acc_addr, 15, 14, mma_rs2_config[2]);
        TC_ST_MMA_ACC(acc_addr, 15, 14, mma_rs2_config[3]);

        T_LD(16, 7, addr_top); // spa: 0, top: 6 ->spa_top: 7
        T_MERGE_128B(17, 1, 16, merge_rs2_config[2]);
        T_LD(18, 14, addr_left);// left: 8
        TC_ST_MMA_ACC(acc_addr, 18, 17, mma_rs2_config[4]);
        TC_ST_MMA_ACC(acc_addr, 18, 17, mma_rs2_config[5]);

        T_MERGE_128B(19, 1, 16, merge_rs2_config[3]);
        T_LD(20, 15, addr_left);// left: 8
        TC_ST_MMA_ACC(acc_addr, 20, 19, mma_rs2_config[6]);
        TC_ST_MMA_ACC(acc_addr, 20, 19, mma_rs2_config[7]);




    


}

// ============================================================================
// Normal Matmul 4x1: Complete matmul operation for 4x1 mode
// Configures CSRs and runs ch_loop iterations of matmul_4x1_ch128
// ============================================================================
static inline void normal_matmul_4x1_ch128(const matmul_config_t *config) {
    uint32_t addr_left = config->addr_left;
    uint32_t addr_top = config->addr_top;
    uint32_t acc_addr = config->acc_addr;
    uint32_t ch_loop = config->ch_loop;

    uint32_t tmask_ld = 0xFF;
    uint32_t twidth_ld = 1024;  
    uint32_t twidth = 128;
    uint32_t tld_offset[8] = {0, 128, 256, 384, 512, 640, 768, 896};
    uint32_t mode = TC_MODE_NORMAL | (TC_PARAL_4X << TC_LEFT_PARAL_SHIFT) | (TC_PARAL_1X << TC_TOP_PARAL_SHIFT);
    uint32_t tlength = MAKE_TLENGTH(4, 1, 0);
    
    load_tc_config(tmask_ld, twidth_ld, tld_offset);
    mode_config_4x1(mode, tlength, twidth);
    uint32_t rs2_config[8] = {MAKE_RS2(0, 0, 0), MAKE_RS2(4, 1, 0), MAKE_RS2(0, 2, 0), MAKE_RS2(4, 3, 0), MAKE_RS2(0, 4, 0), MAKE_RS2(4, 5, 0), MAKE_RS2(0, 6, 0), MAKE_RS2(4, 7, 0)};


    TC_ST_CLEAN(acc_addr);
    for (uint32_t i = 0; i < ch_loop; i++) {
        matmul_4x1_ch128(addr_left, addr_top, acc_addr, rs2_config);
        addr_left += 0x4000;
        addr_top += 0x1000;
    }
}

// ============================================================================
// Normal Matmul 2x2: Complete matmul operation for 2x2 mode
// Configures CSRs and runs ch_loop iterations of matmul_2x2_ch128
// ============================================================================
static inline void normal_matmul_2x2_ch128(const matmul_config_t *config) {
    uint32_t addr_left = config->addr_left;
    uint32_t addr_top = config->addr_top;
    uint32_t acc_addr = config->acc_addr;
    uint32_t ch_loop = config->ch_loop;

    uint32_t tmask_ld = 0xFF;
    uint32_t twidth_ld = 1024;  
    uint32_t twidth = 128;
    uint32_t tld_offset[8] = {0, 128, 256, 384, 512, 640, 768, 896};
    uint32_t mode = TC_MODE_NORMAL | (TC_PARAL_2X << TC_LEFT_PARAL_SHIFT) | (TC_PARAL_2X << TC_TOP_PARAL_SHIFT);
    uint32_t tlength = MAKE_TLENGTH(2, 2, 0);
    uint32_t rs2_config[4] = {MAKE_RS2(0, 0, 0), MAKE_RS2(2, 2, 0), MAKE_RS2(4, 4, 0), MAKE_RS2(6, 6, 0)};
    
    load_tc_config(tmask_ld, twidth_ld, tld_offset);
    mode_config_2x2(mode, tlength, twidth);

    TC_ST_CLEAN(acc_addr);
    for (uint32_t i = 0; i < ch_loop; i++) {
        matmul_2x2_ch128(addr_left, addr_top, acc_addr, rs2_config);
        addr_left += 0x2000;
        addr_top += 0x2000;
    }
}

// ============================================================================
// Normal Matmul 2x2: Complete matmul operation for 2x2 mode
// Configures CSRs and runs ch_loop iterations of matmul_2x2_ch128
// ============================================================================
static inline void spa_matmul_ch128(const matmul_config_t *config) {
    uint32_t addr_left = config->addr_left;
    uint32_t addr_top = config->addr_top;
    uint32_t addr_spa = config->addr_spa;
    uint32_t acc_addr = config->acc_addr;
    uint32_t ch_loop = config->ch_loop;

    uint32_t tmask_ld = 0xFF;
    uint32_t twidth_ld = 1024;  
    uint32_t twidth = 64;
    uint32_t tld_offset[8] = {0, 128, 256, 384, 512, 640, 768, 896};
    uint32_t mode = TC_MODE_SPARSE | (TC_PARAL_4X << TC_LEFT_PARAL_SHIFT) | (TC_PARAL_2X << TC_TOP_PARAL_SHIFT);
    uint32_t tlength = MAKE_TLENGTH(8, 4, 1);

    uint32_t merge_rs2_config[4] = {0b10111010100110000011001000010000, 0b10111010100110000111011001010100, 0b11111110110111000011001000010000, 0b11111110110111000111011001010100};// =0xBA983210
    uint32_t mma_rs2_config[8] = {MAKE_RS2(0, 0, 8), MAKE_RS2(8, 4, 9), MAKE_RS2(0, 0, 10), MAKE_RS2(8, 4, 11), MAKE_RS2(0, 0, 12), MAKE_RS2(8, 4, 13), MAKE_RS2(0, 0, 14), MAKE_RS2(8, 4, 15)};

    
    load_tc_config(tmask_ld, twidth_ld, tld_offset);
    mode_config_2x2(mode, tlength, twidth);


    TC_ST_CLEAN(acc_addr);
    for (uint32_t i = 0; i < ch_loop; i++) {
        spa_matmul_4x2_ch128(addr_left, addr_top, addr_spa, acc_addr, mma_rs2_config, merge_rs2_config);

        addr_left += 0x4000;
        addr_top += 0x2000;
        addr_spa += 0x800;
    }
}


#endif // TC_PERF_COMMON_H

