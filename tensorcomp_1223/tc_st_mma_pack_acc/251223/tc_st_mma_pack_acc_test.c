/*
 * TensorComp tc.st.mma.pack.acc ISA Test
 * ========================================
 * 
 * Instruction: tc.st.mma.pack.acc - Packed Matrix Multiply-Accumulate
 * Syntax: tc.st.mma.pack.acc rs1, trs1, trs2, rs2
 * 
 * Parameters:
 *   rs1:  acc_buffer base address
 *   trs1: Left Data
 *   trs2: Top Data
 *   rs2:  data segment selection
 * 
 * Function:
 *   Similar to tc.st.mma.acc but each K channel result is shifted before accumulation
 *   acc += (TopData[k] * LeftData[k]) << (k * 8)
 * 
 * Use case:
 *   - Quantization computation
 *   - Mixed precision calculation (int8*int32, int16*int32)
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
    
    // Load data into tr0 (Left Data for 4x mode)
    uint32_t addr_left_4x = L1M_BASE_ADDR;
    PRINT("Loading Left Data 4x (0x%08X) into tr0\n", addr_left_4x);
    T_LD(0, 0, addr_left_4x);
    
    // Load data into tr1 (Top Data)
    uint32_t addr_top = L1M_BASE_ADDR + 1024;
    PRINT("Loading Top Data (0x%08X) into tr1\n", addr_top);
    T_LD(1, 0, addr_top);
    
    PRINT("--- TC registers loaded ---\n\n");
}

// ============================================================================
// Test Cases
// ============================================================================

void test_1_pack_mma_4x1(void) {
    PRINT("\n=== Test 1: tc.st.mma.pack.acc (M2=4, N2=1) ===\n");
    
    // Configure CSRs
    uint32_t mode = TC_MODE_NORMAL | (TC_PARAL_4X << TC_LEFT_PARAL_SHIFT) | (TC_PARAL_1X << TC_TOP_PARAL_SHIFT);
    CSR_WRITE(CSR_TENSOR_CORE_MODE, mode);
    
    CSR_WRITE(CSR_TWIDTH, 128);
    CSR_WRITE(CSR_TLENGTH, MAKE_TLENGTH(4, 1, 0));
    
    CSR_WRITE(CSR_TMASK_ST, 0);
    CSR_WRITE(CSR_TWIDTH_ST, 0);
    
    uint32_t acc_addr = ACC_BUFFER_ADDR;
    uint32_t rs2_config = MAKE_RS2(0, 0, 0);
    
    PRINT("tensor_core_mode: 0x%08X\n", mode);
    PRINT("Computing: acc += sum_k((Left[k] * Top[k]) << (k*8))\n");
    
    TC_ST_CLEAN(acc_addr);
    // tc.st.mma.pack.acc acc_addr, t0, t1, rs2_config
    TC_ST_MMA_PACK_ACC(acc_addr, 0, 1, rs2_config);
    
    PRINT("Test 1 completed\n");
}

void test_2_pack_mma_2x2(void) {
    PRINT("\n=== Test 2: tc.st.mma.pack.acc (M2=2, N2=2) ===\n");
    
    uint32_t mode = TC_MODE_NORMAL | (TC_PARAL_2X << TC_LEFT_PARAL_SHIFT) | (TC_PARAL_2X << TC_TOP_PARAL_SHIFT);
    CSR_WRITE(CSR_TENSOR_CORE_MODE, mode);
    
    CSR_WRITE(CSR_TWIDTH, 128);
    CSR_WRITE(CSR_TLENGTH, MAKE_TLENGTH(2, 2, 0));
    
    uint32_t acc_addr = ACC_BUFFER_ADDR;
    uint32_t rs2_config = MAKE_RS2(0, 0, 0);
    
    PRINT("tensor_core_mode: 0x%08X\n", mode);
    
    TC_ST_CLEAN(acc_addr);
    // tc.st.mma.pack.acc acc_addr, t0, t1, rs2_config
    TC_ST_MMA_PACK_ACC(acc_addr, 0, 1, rs2_config);
    
    PRINT("Test 2 completed\n");
}


// ============================================================================
// Main
// ============================================================================

int main(void) {
    PRINT("======================================================================\n");
    PRINT("TensorComp tc.st.mma.pack.acc ISA Test\n");
    PRINT("======================================================================\n");
    PRINT("\nInstruction: tc.st.mma.pack.acc - Packed Matrix Multiply-Accumulate\n");
    PRINT("Syntax: tc.st.mma.pack.acc rs1, trs1, trs2, rs2\n");
    PRINT("\nComputation: acc += sum_k((Left[k] * Top[k]) << (k*8))\n");
    PRINT("Use case: Quantization, mixed precision\n");
    
    // Load data into TC registers
    // Run tests
    load_tc_registers();
    test_1_pack_mma_4x1();
    
    load_tc_registers();
    test_2_pack_mma_2x2();
    
    PRINT("\n======================================================================\n");
    PRINT("All tc.st.mma.pack.acc tests completed!\n");
    PRINT("======================================================================\n");
    
    return 0;
}
