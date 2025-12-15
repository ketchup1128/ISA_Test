/*
 * TensorComp tc.st.mma.acc ISA Test - Test 3: Sparse Mode
 * ========================================================
 * 
 * Instruction: tc.st.mma.acc - Matrix Multiply-Accumulate
 * Syntax: tc.st.mma.acc rs1, trs1, trs2, rs2
 * 
 * Test Configuration:
 *   - Mode: Sparse (4:2 structured sparsity)
 *   - Left Parallelism: 4x
 *   - Top Parallelism: 2x
 */

#include <stdint.h>
#include "tc_common.h"

#ifdef PRINTF_SUPPORT
#include "printf.h"
#define PRINT(...) printf(__VA_ARGS__)
#else
#define PRINT(...)
#endif

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
    
    // Load sparse index data into tr4
    uint32_t addr_trs5 = L1M_BASE_ADDR + 4096;
    PRINT("Loading trs5_data (0x%08X) into tr4\n", addr_trs5);
    T_LD(4, 0, addr_trs5);
    
    PRINT("--- TC registers loaded ---\n\n");
}

// ============================================================================
// Test 3: Sparse Mode
// ============================================================================
void test_3_sparse_mode(void) {
    PRINT("\n=== Test 3: tc.st.mma.acc Sparse Mode ===\n");
    
    // Configure CSRs for sparse mode
    uint32_t mode = TC_MODE_SPARSE | (TC_PARAL_4X << TC_LEFT_PARAL_SHIFT) | (TC_PARAL_2X << TC_TOP_PARAL_SHIFT);
    CSR_WRITE(CSR_TENSOR_CORE_MODE, mode);
    
    CSR_WRITE(CSR_TWIDTH, 64);  // 64 bytes for sparse mode
    CSR_WRITE(CSR_TLENGTH, MAKE_TLENGTH(8, 4, 1));  // left_len=8, top_len=4, spa_len=1
    
    CSR_WRITE(CSR_TMASK_ST, 0);
    CSR_WRITE(CSR_TWIDTH_ST, 0);
    
    uint32_t acc_addr = ACC_BUFFER_ADDR;
    uint32_t rs2_config = MAKE_RS2(0, 0, 4);  // spa_start at block 4
    
    PRINT("tensor_core_mode: 0x%08X (sparse)\n", mode);
    PRINT("twidth: 64, tlength: 0x%08X\n", MAKE_TLENGTH(8, 4, 1));
    PRINT("rs2_config: 0x%08X (spa_start=4)\n", rs2_config);

    // Configure merge for sparse index data
    uint32_t cfg0 = 0b0000;  // trs1[0]
    uint32_t cfg1 = 0b0001;  // trs1[1]
    uint32_t cfg2 = 0b1000;  // trs2[0]
    uint32_t cfg3 = 0b0010;  // trs1[2]
    uint32_t cfg4 = 0b0011;  // trs1[3]
    uint32_t cfg5 = 0b0100;  // trs1[4]
    uint32_t cfg6 = 0b0101;  // trs1[5]
    uint32_t cfg7 = 0b1001;  // trs2[1]
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
    
    // Test with tr5 = merge(tr1, tr4)
    T_MERGE_128B(5, 1, 4, config);
    
    // tc.st.mma.acc acc_addr, t0, t5, rs2_config
    TC_ST_MMA_ACC(acc_addr, 0, 5, rs2_config);
    
    PRINT("Test 3 completed\n");
}

// ============================================================================
// Main
// ============================================================================
int main(void) {
    PRINT("======================================================================\n");
    PRINT("TensorComp tc.st.mma.acc ISA Test - Test 3: Sparse Mode\n");
    PRINT("======================================================================\n");
    PRINT("\nInstruction: tc.st.mma.acc - Matrix Multiply-Accumulate\n");
    PRINT("Configuration: Sparse Mode (4:2 structured sparsity)\n");
    
    // Load data into TC registers
    load_tc_registers();
    
    // Run test
    test_3_sparse_mode();
    
    PRINT("\n======================================================================\n");
    PRINT("Test 3 completed!\n");
    PRINT("======================================================================\n");
    
    return 0;
}

