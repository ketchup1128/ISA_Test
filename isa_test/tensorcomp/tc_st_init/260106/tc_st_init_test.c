/*
 * TensorComp tc.st.init ISA Test
 * ================================
 * 
 * Instruction: tc.st.init - Initialize acc_buffer with TReg data (broadcast)
 * Syntax: tc.st.init rs1, trs1, rs2
 * 
 * Parameters:
 *   rs1:  acc_buffer base address
 *   trs1: Source TReg containing initialization data
 *   rs2:  Data segment selection
 *     rs2[7:0]:   startpoint_left
 *     rs2[15:8]:  size_left (if > 0, use LeftData horizontal broadcast)
 *     rs2[23:16]: startpoint_top
 *     rs2[31:24]: size_top (if > 0, use TopData vertical broadcast)
 * 
 * Function:
 *   - If size_left > 0: acc[m][n] = LeftData[m_2][m_1] (horizontal broadcast)
 *   - If size_top > 0: acc[m][n] = TopData[n_2][n_1] (vertical broadcast)
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
// Helper: Make rs2 config for tc.st.init
// ============================================================================
#define MAKE_INIT_RS2(left_start, left_size, top_start, top_size) \
    ((((top_size) & 0xFF) << 24) | (((top_start) & 0xFF) << 16) | \
     (((left_size) & 0xFF) << 8) | ((left_start) & 0xFF))

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
    
    // Load init data into tr0 (contains int32 values for initialization)
    // Using left data area which has known patterns
    uint32_t addr_init = L1M_BASE_ADDR;
    PRINT("Loading init data (0x%08X) into tr0\n", addr_init);
    T_LD(0, 0, addr_init);
    
    
    PRINT("--- TC registers loaded ---\n\n");
}

// ============================================================================
// Test 1: Horizontal Broadcast (Left Data)
// ============================================================================
void test_1_4x1_left(void) {
    PRINT("\n=== Test 1: tc.st.init 4x1 left (Left Data) ===\n");
    
    // Configure CSRs for 4x1 mode
    uint32_t mode = TC_MODE_NORMAL | (TC_PARAL_4X << TC_LEFT_PARAL_SHIFT) | (TC_PARAL_1X << TC_TOP_PARAL_SHIFT);
    CSR_WRITE(CSR_TENSOR_CORE_MODE, mode);
    CSR_WRITE(CSR_TWIDTH, 128);
    CSR_WRITE(CSR_TLENGTH, MAKE_TLENGTH(4, 1, 0));
    
    uint32_t acc_addr = ACC_BUFFER_ADDR;
    
    // rs2: startpoint_left=0, size_left=4, startpoint_top=0, size_top=0
    // This means use LeftData for horizontal broadcast
    uint32_t rs2_config = MAKE_INIT_RS2(0, 4, 0, 0);
    
    PRINT("Mode: 4x1 (M2=4, N2=1)\n");
    PRINT("rs2_config: 0x%08X\n", rs2_config);
    PRINT("  startpoint_left=0, size_left=4 (horizontal broadcast)\n");
    PRINT("acc_addr: 0x%08X\n", acc_addr);
    
    // Execute tc.st.init
    TC_ST_INIT(acc_addr, 0, rs2_config);
    
    PRINT("Test 1 completed - acc_buffer initialized with horizontal broadcast\n");
}

// ============================================================================
// Test 2: Horizontal Broadcast (Left Data)
// ============================================================================
void test_2_4x1_top(void) {
    PRINT("\n=== Test 2: tc.st.init 4x1 top (Top Data) ===\n");
    
    // Configure CSRs for 4x1 mode
    uint32_t mode = TC_MODE_NORMAL | (TC_PARAL_4X << TC_LEFT_PARAL_SHIFT) | (TC_PARAL_1X << TC_TOP_PARAL_SHIFT);
    CSR_WRITE(CSR_TENSOR_CORE_MODE, mode);
    CSR_WRITE(CSR_TWIDTH, 128);
    CSR_WRITE(CSR_TLENGTH, MAKE_TLENGTH(4, 1, 0));
    
    uint32_t acc_addr = ACC_BUFFER_ADDR;
    
    // rs2: startpoint_left=0, size_left=0, startpoint_top=0, size_top=1
    // This means use LeftData for horizontal broadcast
    uint32_t rs2_config = MAKE_INIT_RS2(0, 0, 0, 1);
    
    PRINT("Mode: 4x1 (M2=4, N2=1)\n");
    PRINT("rs2_config: 0x%08X\n", rs2_config);
    PRINT("  startpoint_left=0, size_left=4 (horizontal broadcast)\n");
    PRINT("acc_addr: 0x%08X\n", acc_addr);
    
    // Execute tc.st.init
    TC_ST_INIT(acc_addr, 0, rs2_config);
    
    PRINT("Test 2 completed - acc_buffer initialized with horizontal broadcast\n");
}

// ============================================================================
// Test 3: Vertical Broadcast (Top Data)
// ============================================================================
void test_3_2x2_left(void) {
    PRINT("\n=== Test 3: tc.st.init 2x2 left (Left Data) ===\n");
    
    // Configure CSRs for 1x4 mode (or 2x2)
    uint32_t mode = TC_MODE_NORMAL | (TC_PARAL_2X << TC_LEFT_PARAL_SHIFT) | (TC_PARAL_2X << TC_TOP_PARAL_SHIFT);
    CSR_WRITE(CSR_TENSOR_CORE_MODE, mode);
    CSR_WRITE(CSR_TWIDTH, 128);
    CSR_WRITE(CSR_TLENGTH, MAKE_TLENGTH(2, 2, 0));
    
    uint32_t acc_addr = ACC_BUFFER_ADDR;
    
    // rs2: startpoint_left=0, size_left=0, startpoint_top=0, size_top=2
    // This means use TopData for vertical broadcast
    uint32_t rs2_config = MAKE_INIT_RS2(0, 2, 0, 0);
    
    PRINT("Mode: 2x2 (M2=2, N2=2)\n");
    PRINT("rs2_config: 0x%08X\n", rs2_config);
    PRINT("  startpoint_top=0, size_top=2 (vertical broadcast)\n");
    PRINT("acc_addr: 0x%08X\n", acc_addr);
    
    // Execute tc.st.init with tr1
    TC_ST_INIT(acc_addr, 0, rs2_config);
    
    PRINT("Test 3 completed - acc_buffer initialized with vertical broadcast\n");
}

// ============================================================================
// Test 4: Vertical Broadcast (Top Data)
// ============================================================================
void test_4_2x2_top(void) {
    PRINT("\n=== Test 4: tc.st.init 2x2 top (Top Data) ===\n");
    
    // Configure CSRs for 1x4 mode (or 2x2)
    uint32_t mode = TC_MODE_NORMAL | (TC_PARAL_2X << TC_LEFT_PARAL_SHIFT) | (TC_PARAL_2X << TC_TOP_PARAL_SHIFT);
    CSR_WRITE(CSR_TENSOR_CORE_MODE, mode);
    CSR_WRITE(CSR_TWIDTH, 128);
    CSR_WRITE(CSR_TLENGTH, MAKE_TLENGTH(2, 2, 0));
    
    uint32_t acc_addr = ACC_BUFFER_ADDR;
    
    // rs2: startpoint_left=0, size_left=0, startpoint_top=0, size_top=2
    // This means use TopData for vertical broadcast
    uint32_t rs2_config = MAKE_INIT_RS2(0, 0, 0, 2);
    
    PRINT("Mode: 2x2 (M2=2, N2=2)\n");
    PRINT("rs2_config: 0x%08X\n", rs2_config);
    PRINT("  startpoint_top=0, size_top=2 (vertical broadcast)\n");
    PRINT("acc_addr: 0x%08X\n", acc_addr);
    
    // Execute tc.st.init with tr1
    TC_ST_INIT(acc_addr, 0, rs2_config);
    
    PRINT("Test 3 completed - acc_buffer initialized with vertical broadcast\n");
}

// ============================================================================
// Main
// ============================================================================
int main(void) {
    PRINT("======================================================================\n");
    PRINT("TensorComp tc.st.init ISA Test\n");
    PRINT("======================================================================\n");
    PRINT("\nInstruction: tc.st.init - Initialize acc_buffer with TReg data\n");
    PRINT("Syntax: tc.st.init rs1, trs1, rs2\n");
    PRINT("\nModes:\n");
    PRINT("  - size_left > 0: Horizontal broadcast (Left Data)\n");
    PRINT("  - size_top > 0: Vertical broadcast (Top Data)\n");
    
    // Load data into TC registers
    load_tc_registers();
    
    // Run tests
    test_1_4x1_left();
    test_2_4x1_top();
    test_3_2x2_left();
    test_4_2x2_top();
    
    PRINT("\n======================================================================\n");
    PRINT("All tc.st.init tests completed!\n");
    PRINT("======================================================================\n");
    
    return 0;
}

