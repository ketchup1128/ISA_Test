/*
 * TensorComp tc.quant ISA Test
 * =============================
 * 
 * Instruction: tc.quant - Quantization Multiplication
 * Syntax: tc.quant rs1, trs1, rs2
 * 
 * Parameters:
 *   rs1:  acc_buffer base address
 *   trs1: Quantization scale data
 *   rs2:  configuration
 * 
 * Function:
 *   Read data from acc_buffer, multiply with trs1 data, store to quant_reg
 *   quant_reg[i] = sum_k(acc_buffer[i][k] * trs1[k])
 */

 #include <stdint.h>
 #include "tc_common.h"
 
 #ifdef PRINTF_SUPPORT
 #include "printf.h"
 #define PRINT(...) printf(__VA_ARGS__)
 #else
 #define PRINT(...)
 #endif
 
 // Acc buffer
 volatile int32_t acc_buffer[4096] __attribute__((aligned(4096)));
 
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
     T_LD(1, 1, addr_init);
     
     
     PRINT("--- TC registers loaded ---\n\n");
 }
 
 
 // ============================================================================
 // Test Cases
 // ============================================================================
 
 void test_1_quant_acc_right_shift(void) {
     PRINT("\n=== Test 1: tc.quant 4x1 left (Left Data) ===\n");
     
     // Configure CSRs for 4x1 mode
     uint32_t mode = TC_MODE_NORMAL | (TC_PARAL_4X << TC_LEFT_PARAL_SHIFT) | (TC_PARAL_1X << TC_TOP_PARAL_SHIFT);
     CSR_WRITE(CSR_TENSOR_CORE_MODE, mode);
     CSR_WRITE(CSR_TWIDTH, 128);
     CSR_WRITE(CSR_TLENGTH, MAKE_TLENGTH(4, 1, 0));
     
     uint32_t acc_addr = ACC_BUFFER_ADDR;
     
     // rs2: startpoint_left=0, size_left=4, startpoint_top=0, size_top=0
     // This means use LeftData for horizontal broadcast
     uint32_t rs2_config = MAKE_INIT_RS2(0, 4, 0, 0);
     uint32_t shift_config = QUANT_SHIFT_RIGHT(8);
     
     // Execute tc.st.init to initialize the acc_buffer
     TC_ST_INIT(acc_addr, 0, rs2_config);
     TC_QUANT(acc_addr, 1, rs2_config);
     TC_ST_QUANT_SHIFT_ACC(acc_addr, shift_config);
     
     
     PRINT("Computing: quant_reg >> 8\n");
     
     // Execute tc.quant
     
     PRINT("Test 1 completed\n");
 }
 
 void test_2_quant_acc_left_shift(void) {
     PRINT("\n=== Test 2: tc.quant 4x1 top (Top Data) ===\n");
     
     // Configure CSRs for 4x1 mode
     uint32_t mode = TC_MODE_NORMAL | (TC_PARAL_4X << TC_LEFT_PARAL_SHIFT) | (TC_PARAL_1X << TC_TOP_PARAL_SHIFT);
     CSR_WRITE(CSR_TENSOR_CORE_MODE, mode);
     CSR_WRITE(CSR_TWIDTH, 128);
     CSR_WRITE(CSR_TLENGTH, MAKE_TLENGTH(4, 1, 0));
     
     uint32_t rs2_config = MAKE_INIT_RS2(0, 0, 0, 1);
     uint32_t shift_config = QUANT_SHIFT_LEFT(8);
     
     uint32_t acc_addr = ACC_BUFFER_ADDR;
     // Execute tc.st.init to initialize the acc_buffer
     TC_ST_INIT(acc_addr, 0, rs2_config);
     TC_QUANT(acc_addr, 1, rs2_config);
     TC_ST_QUANT_SHIFT_ACC(acc_addr, shift_config);

     PRINT("Computing: quant_reg << 8\n");
     
     // Execute tc.quant
     
     PRINT("Test 2 completed\n");
 }
 
     
 // ============================================================================
 // Main
 // ============================================================================
 
 int main(void) {
     PRINT("======================================================================\n");
     PRINT("TensorComp tc.st.quant.shift ISA Test\n");
     PRINT("======================================================================\n");
     PRINT("\nInstruction: tc.st.quant.shift - Shift quant_reg and store to acc_buffer\n");
     PRINT("Syntax: tc.st.quant.shift rs1, rs2\n");
     PRINT("\nFunction: quant_reg >>= rs2\n");
     
     load_tc_registers();// Run tests
 
 
     test_1_quant_acc_right_shift();
     test_2_quant_acc_left_shift();
     
     PRINT("\n======================================================================\n");
     PRINT("All tc.quant tests completed!\n");
     PRINT("======================================================================\n");
     
     return 0;
 }
 