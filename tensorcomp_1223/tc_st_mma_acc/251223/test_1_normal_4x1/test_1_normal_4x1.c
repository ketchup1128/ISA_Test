/*
 * TensorComp tc.st.mma.acc ISA Test - Test 1: Normal Mode (M2=4, N2=1)
 * =====================================================================
 * 
 * Instruction: tc.st.mma.acc - Matrix Multiply-Accumulate
 * Syntax: tc.st.mma.acc rs1, trs1, trs2, rs2
 * 
 * Test Configuration:
 *   - Mode: Normal
 *   - Left Parallelism: 4x (M2=4)
 *   - Top Parallelism: 1x (N2=1)
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
    
    
    // Load trs1_data into tr0 (Left Data - 4x mode)
    uint32_t addr_trs1 = L1M_BASE_ADDR;
    PRINT("Loading trs1_data (0x%08X) into tr0\n", addr_trs1);
    T_LD(0, 0, addr_trs1);
    
    // Load trs2_data into tr1 (Top Data - 1x mode)
    uint32_t addr_trs2 = L1M_BASE_ADDR + 1024;
    PRINT("Loading trs2_data (0x%08X) into tr1\n", addr_trs2);
    T_LD(1, 0, addr_trs2);
    
    PRINT("--- TC registers loaded ---\n\n");
}

// ============================================================================
// Test 1: Normal Mode (M2=4, N2=1)
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
    
    uint32_t acc_addr = ACC_BUFFER_ADDR;
    uint32_t rs2_config = MAKE_RS2(0, 0, 0);  // startpoints all at 0
    
    PRINT("tensor_core_mode: 0x%08X\n", mode);
    PRINT("twidth: 128, tlength: 0x%08X\n", MAKE_TLENGTH(4, 1, 0));
    PRINT("acc_addr: 0x%08X\n", acc_addr);
    PRINT("rs2_config: 0x%08X\n", rs2_config);
    TC_ST_CLEAN(acc_addr);
    // tc.st.mma.acc acc_addr, t0, t1, rs2_config
    TC_ST_MMA_ACC(acc_addr, 0, 1, rs2_config);
    
    PRINT("Test 1 completed\n");
}

// ============================================================================
// Main
// ============================================================================
int main(void) {
    PRINT("======================================================================\n");
    PRINT("TensorComp tc.st.mma.acc ISA Test - Test 1: Normal Mode (4x1)\n");
    PRINT("======================================================================\n");
    PRINT("\nInstruction: tc.st.mma.acc - Matrix Multiply-Accumulate\n");
    PRINT("Configuration: Normal Mode, Left 4x, Top 1x\n");
    
    // Load data into TC registers
    load_tc_registers();
    
    // Run test
    test_1_normal_mode_4x1();
    
    PRINT("\n======================================================================\n");
    PRINT("Test 1 completed!\n");
    PRINT("======================================================================\n");
    
    return 0;
}

