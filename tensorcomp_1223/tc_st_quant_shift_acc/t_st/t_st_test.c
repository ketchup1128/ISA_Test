/*
 * TensorComp t.st ISA Test
 * =========================
 * 
 * Instruction: t.st - Store from TReg to L0M
 * Syntax: t.st trs1, imm10(rs1)
 * 
 * Function:
 *   base_addr = rs1 + imm10 * twidth_st
 *   for i in range(8):
 *     if (tmask_st[i] == 1):
 *       addr_st = base_addr + toffset_st[i]
 *       MEM[addr_st +: 128] = treg[i*128 +: 128]
 */

#include <stdint.h>
#include "../tc_common.h"

#ifdef PRINTF_SUPPORT
#include "printf.h"
#define PRINT(...) printf(__VA_ARGS__)
#else
#define PRINT(...)
#endif

// Output data area
volatile uint8_t output_data[2048] __attribute__((aligned(1024)));

// ============================================================================
// Test Cases
// ============================================================================

void test_1_full_mask(void) {
    PRINT("\n=== Test 1: t.st with full mask (0xFF) ===\n");
    
    // Configure CSRs
    CSR_WRITE(CSR_TMASK_ST, 0xFF);       // Enable all 8 stores
    CSR_WRITE(CSR_TWIDTH_ST, 1024);      // twidth_st = 1024 bytes
    
    configure_standard_st_offsets();
    
    uint32_t base_addr = (uint32_t)output_data;
    PRINT("Base address: 0x%08X\n", base_addr);
    PRINT("Expected: Store 8 x 128 bytes from TReg t0\n");
    
    // t.st t0, 0(base_addr)
    T_ST(0, 0, base_addr);
    
    PRINT("Test 1 completed\n");
}

void test_2_partial_mask(void) {
    PRINT("\n=== Test 2: t.st with partial mask (0xAA) ===\n");
    
    // Configure CSRs
    CSR_WRITE(CSR_TMASK_ST, 0xAA);       // Enable stores 1, 3, 5, 7
    CSR_WRITE(CSR_TWIDTH_ST, 1024);
    
    configure_standard_st_offsets();
    
    uint32_t base_addr = (uint32_t)output_data;
    PRINT("Base address: 0x%08X\n", base_addr);
    PRINT("Expected: Store 4 x 128 bytes (positions 1,3,5,7)\n");
    
    // t.st t0, 0(base_addr)
    T_ST(0, 0, base_addr);
    
    PRINT("Test 2 completed\n");
}

void test_3_with_offset(void) {
    PRINT("\n=== Test 3: t.st with imm10 offset ===\n");
    
    // Configure CSRs
    CSR_WRITE(CSR_TMASK_ST, 0xFF);
    CSR_WRITE(CSR_TWIDTH_ST, 128);       // twidth_st = 128 bytes
    
    configure_standard_st_offsets();
    
    uint32_t base_addr = (uint32_t)output_data;
    PRINT("Base address: 0x%08X\n", base_addr);
    PRINT("imm10=2, twidth_st=128 -> offset = 256 bytes\n");
    
    // t.st t1, 2(base_addr)
    T_ST(1, 2, base_addr);
    
    PRINT("Test 3 completed\n");
}

void test_4_different_tregs(void) {
    PRINT("\n=== Test 4: t.st from different TRegs ===\n");
    
    CSR_WRITE(CSR_TMASK_ST, 0xFF);
    CSR_WRITE(CSR_TWIDTH_ST, 1024);
    configure_standard_st_offsets();
    
    uint32_t base_addr = (uint32_t)output_data;
    
    // Store from t2, t3, t4, t5
    T_ST(2, 0, base_addr);
    T_ST(3, 0, base_addr);
    T_ST(4, 0, base_addr);
    T_ST(5, 0, base_addr);
    
    PRINT("Stored from t2, t3, t4, t5\n");
    PRINT("Test 4 completed\n");
}

void test_5_multiple_offsets(void) {
    PRINT("\n=== Test 5: t.st with multiple imm10 offsets ===\n");
    
    CSR_WRITE(CSR_TMASK_ST, 0xFF);
    CSR_WRITE(CSR_TWIDTH_ST, 256);
    configure_standard_st_offsets();
    
    uint32_t base_addr = (uint32_t)output_data;
    
    // Store at different offsets
    T_ST(0, 0, base_addr);  // offset = 0
    T_ST(1, 1, base_addr);  // offset = 256
    T_ST(2, 2, base_addr);  // offset = 512
    T_ST(3, 3, base_addr);  // offset = 768
    
    PRINT("Stored at offsets 0, 256, 512, 768\n");
    PRINT("Test 5 completed\n");
}

void test_6_all_tregs(void) {
    PRINT("\n=== Test 6: t.st from all TRegs ===\n");
    
    CSR_WRITE(CSR_TMASK_ST, 0xFF);
    CSR_WRITE(CSR_TWIDTH_ST, 1024);
    configure_standard_st_offsets();
    
    uint32_t base_addr = (uint32_t)output_data;
    
    // Store from all 16 TRegs
    T_ST(0, 0, base_addr);
    T_ST(1, 0, base_addr);
    T_ST(2, 0, base_addr);
    T_ST(3, 0, base_addr);
    T_ST(4, 0, base_addr);
    T_ST(5, 0, base_addr);
    T_ST(6, 0, base_addr);
    T_ST(7, 0, base_addr);
    T_ST(8, 0, base_addr);
    T_ST(9, 0, base_addr);
    T_ST(10, 0, base_addr);
    T_ST(11, 0, base_addr);
    T_ST(12, 0, base_addr);
    T_ST(13, 0, base_addr);
    T_ST(14, 0, base_addr);
    T_ST(15, 0, base_addr);
    
    PRINT("Stored from all t0-t15\n");
    PRINT("Test 6 completed\n");
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
    PRINT("======================================================================\n");
    PRINT("TensorComp t.st ISA Test\n");
    PRINT("======================================================================\n");
    PRINT("\nInstruction: t.st - Store from TReg to L0M\n");
    PRINT("Syntax: t.st trs1, imm10(rs1)\n");
    
    // Clear output area
    for (int i = 0; i < 2048; i++) {
        output_data[i] = 0;
    }
    
    // Run tests
    test_1_full_mask();
    test_2_partial_mask();
    test_3_with_offset();
    test_4_different_tregs();
    test_5_multiple_offsets();
    test_6_all_tregs();
    
    PRINT("\n======================================================================\n");
    PRINT("All t.st tests completed!\n");
    PRINT("======================================================================\n");
    
    return 0;
}
