/*
 * TensorComp tc.st.add ISA Test
 * ==============================
 * 
 * Instruction: tc.st.add - Broadcast Addition
 * Syntax: tc.st.add rs1, trs1, trs2, rs2
 * 
 * Parameters:
 *   rs1:  acc_buffer base address
 *   trs1: Left Data
 *   trs2: Top Data
 *   rs2:  data segment selection
 * 
 * Function:
 *   If left_length == 0 && top_length == 0: result = 0
 *   If left_length == 0: result = TopData[n_2][n_1] (only Top)
 *   If top_length == 0: result = LeftData[m_2][m_1] (only Left)
 *   Else: result = LeftData[m_2][m_1] + TopData[n_2][n_1] (broadcast add)
 * 
 * Use case: Adding bias in neural networks
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
    
    // Load data into tr0-tr5 for different test cases
    uint32_t addr_left = L1M_BASE_ADDR;
    PRINT("Loading Left Data (0x%08X) into tr0\n", addr_left);
    T_LD(0, 0, addr_left);
    
    uint32_t addr_top = L1M_BASE_ADDR + 1024;
    PRINT("Loading Top Data (0x%08X) into tr1\n", addr_top);
    T_LD(1, 0, addr_top);
    
    
    PRINT("--- TC registers loaded ---\n\n");
}

// ============================================================================
// Test Cases
// ============================================================================

void test_1_broadcast_add_4x1(void) {
    PRINT("\n=== Test 1: tc.st.add Broadcast Addition (4x1) ===\n");
    // Configure CSRs
    uint32_t mode = TC_MODE_NORMAL | (TC_PARAL_4X << TC_LEFT_PARAL_SHIFT) | (TC_PARAL_1X << TC_TOP_PARAL_SHIFT);
    CSR_WRITE(CSR_TENSOR_CORE_MODE, mode);

    CSR_WRITE(CSR_TWIDTH, 128);
    CSR_WRITE(CSR_TLENGTH, MAKE_TLENGTH(4, 1, 0));  // left_len=4, top_len=1
    
    CSR_WRITE(CSR_TMASK_ST, 0);
    CSR_WRITE(CSR_TWIDTH_ST, 0);
    
    uint32_t acc_addr = ACC_BUFFER_ADDR;
    uint32_t rs2_config = MAKE_RS2(0, 0, 0);
    
    PRINT("tlength: 0x%08X (left_len=4, top_len=1)\n", MAKE_TLENGTH(4, 1, 0));
    PRINT("Computing: acc[m][n] = Left[m] + Top[n]\n");
    
    // tc.st.add acc_addr, t0, t1, rs2_config
    TC_ST_ADD(acc_addr, 0, 1, rs2_config);
    
    PRINT("Test 1 completed\n");
}
void test_2_broadcast_add_2x2(void) {
    PRINT("\n=== Test 2: tc.st.add Broadcast Addition (2x2) ===\n");
    // Configure CSRs
    uint32_t mode = TC_MODE_NORMAL | (TC_PARAL_2X << TC_LEFT_PARAL_SHIFT) | (TC_PARAL_2X << TC_TOP_PARAL_SHIFT);
    CSR_WRITE(CSR_TENSOR_CORE_MODE, mode);

    CSR_WRITE(CSR_TWIDTH, 128);
    CSR_WRITE(CSR_TLENGTH, MAKE_TLENGTH(2, 2, 0));  // left_len=2, top_len=2
    
    CSR_WRITE(CSR_TMASK_ST, 0);
    CSR_WRITE(CSR_TWIDTH_ST, 0);
    
    uint32_t acc_addr = ACC_BUFFER_ADDR;
    uint32_t rs2_config = MAKE_RS2(0, 0, 0);
    
    PRINT("tlength: 0x%08X (left_len=4, top_len=1)\n", MAKE_TLENGTH(4, 1, 0));
    PRINT("Computing: acc[m][n] = Left[m] + Top[n]\n");
    
    // tc.st.add acc_addr, t0, t1, rs2_config
    TC_ST_ADD(acc_addr, 0, 1, rs2_config);
    
    PRINT("Test 2 completed\n");
}


// ============================================================================
// Main
// ============================================================================

int main(void) {
    PRINT("======================================================================\n");
    PRINT("TensorComp tc.st.add ISA Test\n");
    PRINT("======================================================================\n");
    PRINT("\nInstruction: tc.st.add - Broadcast Addition\n");
    PRINT("Syntax: tc.st.add rs1, trs1, trs2, rs2\n");
    PRINT("\nModes:\n");
    PRINT("  - left_len=0, top_len=0: result = 0\n");
    PRINT("  - left_len=0: result = Top (vertical broadcast)\n");
    PRINT("  - top_len=0: result = Left (horizontal broadcast)\n");
    PRINT("  - else: result = Left + Top (outer sum)\n");
    
    // Load data into TC registers
    load_tc_registers();
    
    // Run tests
    test_1_broadcast_add_4x1();
    test_2_broadcast_add_2x2();
    
    PRINT("\n======================================================================\n");
    PRINT("All tc.st.add tests completed!\n");
    PRINT("======================================================================\n");
    
    return 0;
}
