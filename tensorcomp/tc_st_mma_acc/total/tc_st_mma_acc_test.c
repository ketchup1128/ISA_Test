/*
 * TensorComp tc.st.mma.acc ISA Test
 * ===================================
 * 
 * Instruction: tc.st.mma.acc - Matrix Multiply-Accumulate
 * Syntax: tc.st.mma.acc rs1, trs1, trs2, rs2
 * 
 * Parameters:
 *   rs1:  acc_buffer base address
 *   trs1: Left Data (horizontal broadcast)
 *   trs2: Top Data (vertical broadcast)
 *   rs2:  data segment selection {startpoint_spa[23:16], startpoint_top[15:8], startpoint_left[7:0]}
 * 
 * Computation (normal mode, M2=4, N2=1):
 *   LeftData[M2][M1][K] from trs1, TopData[N2][N1][K] from trs2
 *   for m_2, m_1, n_2, n_1:
 *     acc = 0
 *     for k in range(K):
 *       acc += LeftData[m_2][m_1][k] * TopData[n_2][n_1][k]
 *     acc_buffer[idx] += acc
 */

#include <stdint.h>
#include "tc_common.h"

#ifdef PRINTF_SUPPORT
#include "printf.h"
#define PRINT(...) printf(__VA_ARGS__)
#else
#define PRINT(...)
#endif

// // Acc buffer (aligned)
// volatile int32_t acc_buffer[4096] __attribute__((aligned(4096)));

// // Test data areas (1024 bytes each, aligned)
// // Data pattern: trs1_data for Left Data, trs2_data for Top Data
// volatile uint8_t trs1_data[1024] __attribute__((aligned(1024)));
// volatile uint8_t trs2_data[1024] __attribute__((aligned(1024)));
// volatile uint8_t trs3_data[1024] __attribute__((aligned(1024)));
// volatile uint8_t trs4_data[1024] __attribute__((aligned(1024)));
// volatile uint8_t trs5_data[1024] __attribute__((aligned(1024)));

// // Test data array: 8 rows x 4 columns = 32 bytes
// volatile uint8_t test_data_array[32] = {
//     68, 132, 196, 148, 
//     212, 228, 72, 136, 
//     200, 152, 216, 232, 
//     76, 140, 204, 156, 
//     220, 236, 73, 137, 
//     201, 153, 217, 233, 
//     77, 141, 205, 157, 
//     221, 237, 78, 142			
// };

// // ============================================================================
// // Helper: Initialize test data
// // ============================================================================
// void init_test_data(void) {
//     // Fill trs1_data with pattern for Left Data (M2*M1*K bytes)
//     // Each 128-byte block has different pattern

//     // normal mode, left 4x, top 1x 
//     for (int l = 0; l < 2; l++) {
//         for (int k = 0; k < 4; k++) {
//             for (int i = 0; i < 32; i++) {
//                 for (int j = 0; j < 4; j++) {
//                     trs1_data[l * 512 + k * 128 + i * 4 + j] = (i*4+k) & 0xFF;
//                 }
//             }
//         }
//     }
    
//     // Fill trs2_data with pattern for Top Data (N2*N1*K bytes)
//     // Each 128-byte block has different pattern
//     for (int i = 0; i < 8; i++) {
//         for (int j = 0; j < 128; j++) {
//             trs2_data[i * 128 + j] = j & 0xFF;
//         }
//     }

//     // normal mode, left 2x, top 2x 
//     for (int l = 0; l < 4; l++) {
//         for (int k = 0; k < 2; k++) {
//             for (int i = 0; i < 32; i++) {
//                 for (int j = 0; j < 4; j++) {
//                     trs3_data[l * 256 + k * 128 + i * 4 + j] = (i*2+k) & 0xFF;
//                 }
//             }
//         }
//     }
    
//     // Fill trs2_data with pattern for Top Data (N2*N1*K bytes)
//     // Each 128-byte block has different pattern
//     for (int i = 0; i < 8; i++) {
//         for (int j = 0; j < 128; j++) {
//             trs4_data[i * 128 + j] = j & 0xFF;
//         }
//     }

//     for (int i = 0; i < 32; i++) {
//         for (int j = 0; j < 32; j++) {
//             trs5_data[i * 32 + j] = test_data_array[j];
//         }
//     }
// }

// ============================================================================
// Helper: Load data into TC registers using t.ld
// ============================================================================
void load_tc_registers(void) {
    PRINT("\n--- Loading data into TC registers using t.ld ---\n");
    
    // Configure CSRs for t.ld
    CSR_WRITE(CSR_TMASK_LD, 0xFF);       // Enable all 8 x 128-byte loads
    CSR_WRITE(CSR_TWIDTH_LD, 1024);      // twidth_ld = 1024 bytes
    
    // Configure standard offsets (0, 128, 256, 384, 512, 640, 768, 896)
    configure_standard_ld_offsets();
    
    // Load trs1_data into tr0 (Left Data)
    uint32_t addr_trs1 = L1M_BASE_ADDR;
    PRINT("Loading trs1_data (0x%08X) into tr0\n", addr_trs1);
    T_LD(0, 0, addr_trs1);
    
    // Load trs2_data into tr1 (Top Data)
    uint32_t addr_trs2 = L1M_BASE_ADDR + 3072;
    PRINT("Loading trs2_data (0x%08X) into tr1\n", addr_trs2);
    T_LD(1, 0, addr_trs2);

    // Load trs3_data into tr2 (Left Data)
    uint32_t addr_trs3 = L1M_BASE_ADDR + 1024;
    PRINT("Loading trs3_data (0x%08X) into tr2\n", addr_trs3);
    T_LD(2, 0, addr_trs3);

    // Load trs4_data into tr3 (Top Data)
    uint32_t addr_trs4 = L1M_BASE_ADDR + 3072;
    PRINT("Loading trs4_data (0x%08X) into tr3\n", addr_trs4);
    T_LD(3, 0, addr_trs4); 

    uint32_t addr_trs5 = L1M_BASE_ADDR + 4096;
    PRINT("Loading trs5_data (0x%08X) into tr5\n", addr_trs5);
    T_LD(4, 0, addr_trs5);
    
    PRINT("--- TC registers loaded ---\n\n");
}

// ============================================================================
// Test Cases
// ============================================================================

void test_1_normal_mode_4x1(void) {
    PRINT("\n=== Test 1: tc.st.mma.acc Normal Mode (M2=4, N2=1) ===\n");
    
    // Configure CSRs
    // tensor_core_mode: normal mode, left 4x, top 1x
    uint32_t mode = TC_MODE_NORMAL | (TC_PARAL_4X << TC_LEFT_PARAL_SHIFT) | (TC_PARAL_1X << TC_TOP_PARAL_SHIFT);
    CSR_WRITE(CSR_TENSOR_CORE_MODE, mode);
    
    CSR_WRITE(CSR_TWIDTH, 128);  // 128 bytes per block
    CSR_WRITE(CSR_TLENGTH, MAKE_TLENGTH(4, 1, 0));  // left_len=4, top_len=1, spa_len=0
    
    // For COMP STORE, mask and width should be 0
    CSR_WRITE(CSR_TMASK_ST, 0);
    CSR_WRITE(CSR_TWIDTH_ST, 0);
    
    uint32_t acc_addr = (uint32_t)acc_buffer;
    uint32_t rs2_config = MAKE_RS2(0, 0, 0);  // startpoints all at 0
    
    PRINT("tensor_core_mode: 0x%08X\n", mode);
    PRINT("twidth: 128, tlength: 0x%08X\n", MAKE_TLENGTH(4, 1, 0));
    PRINT("acc_addr: 0x%08X\n", acc_addr);
    PRINT("rs2_config: 0x%08X\n", rs2_config);
    
    // tc.st.mma.acc acc_addr, t0, t1, rs2_config
    TC_ST_MMA_ACC(acc_addr, 0, 1, rs2_config);
    
    PRINT("Test 1 completed\n");
}

void test_2_normal_mode_2x2(void) {
    PRINT("\n=== Test 2: tc.st.mma.acc Normal Mode (M2=2, N2=2) ===\n");
    
    // Configure CSRs
    uint32_t mode = TC_MODE_NORMAL | (TC_PARAL_2X << TC_LEFT_PARAL_SHIFT) | (TC_PARAL_2X << TC_TOP_PARAL_SHIFT);
    CSR_WRITE(CSR_TENSOR_CORE_MODE, mode);
    
    CSR_WRITE(CSR_TWIDTH, 128);
    CSR_WRITE(CSR_TLENGTH, MAKE_TLENGTH(2, 2, 0));  // left_len=2, top_len=2, spa_len=0
    
    CSR_WRITE(CSR_TMASK_ST, 0);
    CSR_WRITE(CSR_TWIDTH_ST, 0);
    
    uint32_t acc_addr = (uint32_t)acc_buffer;
    uint32_t rs2_config = MAKE_RS2(0, 0, 0);
    
    PRINT("tensor_core_mode: 0x%08X\n", mode);
    PRINT("twidth: 128, tlength: 0x%08X\n", MAKE_TLENGTH(2, 2, 0));
    
    // tc.st.mma.acc acc_addr, t2, t3, rs2_config
    TC_ST_MMA_ACC(acc_addr, 2, 3, rs2_config);
    
    PRINT("Test 2 completed\n");
}

void test_3_sparse_mode(void) {
    PRINT("\n=== Test 3: tc.st.mma.acc Sparse Mode ===\n");
    
    // Configure CSRs for sparse mode
    uint32_t mode = TC_MODE_SPARSE | (TC_PARAL_4X << TC_LEFT_PARAL_SHIFT) | (TC_PARAL_2X << TC_TOP_PARAL_SHIFT);
    CSR_WRITE(CSR_TENSOR_CORE_MODE, mode);
    
    CSR_WRITE(CSR_TWIDTH, 64);  // 64 bytes for sparse mode
    CSR_WRITE(CSR_TLENGTH, MAKE_TLENGTH(8, 4, 1));  // left_len=8, top_len=4, spa_len=1
    
    CSR_WRITE(CSR_TMASK_ST, 0);
    CSR_WRITE(CSR_TWIDTH_ST, 0);
    
    uint32_t acc_addr = (uint32_t)acc_buffer;
    uint32_t rs2_config = MAKE_RS2(0, 0, 4);  // spa_start at block 4
    
    PRINT("tensor_core_mode: 0x%08X (sparse)\n", mode);
    PRINT("twidth: 64, tlength: 0x%08X\n", MAKE_TLENGTH(8, 4, 1));
    PRINT("rs2_config: 0x%08X (spa_start=4)\n", rs2_config);

    uint32_t cfg0 = 0b0000;  // trs1[0]
    uint32_t cfg1 = 0b0001;  // trs1[1]
    uint32_t cfg2 = 0b1000;  // trs1[2]
    uint32_t cfg3 = 0b0010;  // trs1[3]
    uint32_t cfg4 = 0b0011;  // trs1[4]
    uint32_t cfg5 = 0b0100;  // trs1[5]
    uint32_t cfg6 = 0b0101;  // trs1[6]
    uint32_t cfg7 = 0b1001;  // trs1[7]
    uint32_t config = (cfg7 << 28) | (cfg6 << 24) | (cfg5 << 20) | (cfg4 << 16) | 
                      (cfg3 << 12) | (cfg2 << 8) | (cfg1 << 4) | cfg0;
    
    PRINT("Config: 0x%08X\n", config);
    for (int blk = 0; blk < 8; ++blk) {
        uint32_t cfg = (config >> (blk*4)) & 0xF;
        uint32_t reg_id = (cfg >> 3) & 0x1;
        uint32_t index_id = cfg & 0x7;
        PRINT("  Block[%d]: reg_id=%u, index_id=%u\n", blk, reg_id, index_id);
    }
    PRINT("Merging tr1 and tr4 into tr5\n");
    
    // Test with tr5 = merge(tr3, tr4)
    T_MERGE_128B(5, 1, 4, config);
    
    // tc.st.mma.acc acc_addr, t0, t1, rs2_config
    TC_ST_MMA_ACC(acc_addr, 0, 5, rs2_config);
    
    PRINT("Test 3 completed\n");
}



// ============================================================================
// Main
// ============================================================================

int main(void) {
    PRINT("======================================================================\n");
    PRINT("TensorComp tc.st.mma.acc ISA Test\n");
    PRINT("======================================================================\n");
    PRINT("\nInstruction: tc.st.mma.acc - Matrix Multiply-Accumulate\n");
    PRINT("Syntax: tc.st.mma.acc rs1, trs1, trs2, rs2\n");
    PRINT("\nModes:\n");
    PRINT("  - Normal: Left 4x Top 1x, or Left 2x Top 2x\n");
    PRINT("  - Sparse: 4:2 structured sparsity\n");
    
    // Initialize test data
    // init_test_data();
    
    // Load data into TC registers
    load_tc_registers();
    
    // Clear acc_buffer
    // for (int i = 0; i < 4096; i++) {
    //     acc_buffer[i] = 0;
    // }
    
    // Run tests
    test_1_normal_mode_4x1();
    test_2_normal_mode_2x2();
    test_3_sparse_mode();
    
    PRINT("\n======================================================================\n");
    PRINT("All tc.st.mma.acc tests completed!\n");
    PRINT("======================================================================\n");
    
    return 0;
}
