/*
 * TensorComp t.ld ISA Test
 * =========================
 * 
 * Instruction: t.ld - Load from L0M to TReg
 * Syntax: t.ld trd, imm10(rs1)
 * 
 * Function:
 *   base_addr = rs1 + imm10 * twidth_ld
 *   for i in range(8):
 *     if (tmask_ld[i] == 1):
 *       addr_ld = base_addr + toffset_ld[i]
 *       treg[i*128 +: 128] = MEM[addr_ld +: 128]
 *     else:
 *       treg[i*128 +: 128] = 0
 */

#include <stdint.h>
#include "tc_common.h"

#ifdef PRINTF_SUPPORT
#include "printf.h"
#define PRINT(...) printf(__VA_ARGS__)
#else
#define PRINT(...)
#endif

// Test data area (should be initialized by external loader)

// ============================================================================
// Test Cases
// ============================================================================

void test_1_full_mask(uint32_t base_addr) {
    PRINT("\n=== Test 1: t.ld with full mask (0xFF) ===\n");
    
    // Configure CSRs
    CSR_WRITE(CSR_TMASK_LD, 0xFF);       // Enable all 8 loads
    CSR_WRITE(CSR_TWIDTH_LD, 1024);      // twidth_ld = 1024 bytes
    
    // Configure standard offsets (0, 128, 256, ...)
    configure_standard_ld_offsets();
    
    // uint32_t base_addr = (uint32_t)test_data;
    PRINT("Base address: 0x%08X\n", base_addr);
    PRINT("Expected: Load 8 x 128 bytes to TReg t0\n");
    
    // Execute: t.ld t0, 0(base_addr)
    T_LD(0, 0, base_addr);
    
    PRINT("Test 1 completed\n");
}

void test_2_partial_mask(uint32_t base_addr) {
    PRINT("\n=== Test 2: t.ld with partial mask (0x55) ===\n");
    
    // Configure CSRs
    CSR_WRITE(CSR_TMASK_LD, 0x55);       // Enable loads 0, 2, 4, 6
    CSR_WRITE(CSR_TWIDTH_LD, 1024);
    
    configure_standard_ld_offsets();
    
    // uint32_t base_addr = (uint32_t)test_data;
    PRINT("Base address: 0x%08X\n", base_addr);
    PRINT("Expected: Load 4 x 128 bytes (positions 0,2,4,6) to TReg t1\n");
    
    // Execute: t.ld t1, 0(base_addr)
    T_LD(1, 0, base_addr);
    
    PRINT("Test 2 completed\n");
}

void test_3_with_offset(uint32_t base_addr) {
    PRINT("\n=== Test 3: t.ld with imm10 offset ===\n");
    
    // Configure CSRs
    CSR_WRITE(CSR_TMASK_LD, 0xFF);
    CSR_WRITE(CSR_TWIDTH_LD, 128);       // twidth_ld = 128 bytes
    
    configure_standard_ld_offsets();
    
    // uint32_t base_addr = (uint32_t)test_data;
    PRINT("Base address: 0x%08X\n", base_addr);
    PRINT("imm10=1, twidth_ld=128 -> actual offset = 128 bytes\n");
    
    // Execute: t.ld t2, 1(base_addr) - offset by 1*twidth_ld = 128 bytes
    T_LD(2, 1, base_addr);
    
    PRINT("Test 3 completed\n");
}

void test_4_different_toffsets(uint32_t base_addr) {
    PRINT("\n=== Test 4: t.ld with non-contiguous offsets ===\n");
    
    // Configure CSRs
    CSR_WRITE(CSR_TMASK_LD, 0x0F);       // Enable first 4 loads
    CSR_WRITE(CSR_TWIDTH_LD, 1024);
    
    // Configure non-contiguous offsets (every 256 bytes)
    configure_toffset_ld_8(0, 256, 512, 768, 0, 0, 0, 0);
    
    // uint32_t base_addr = (uint32_t)test_data;
    PRINT("Base address: 0x%08X\n", base_addr);
    PRINT("Offsets: [0, 256, 512, 768]\n");
    
    // Execute: t.ld t3, 0(base_addr)
    T_LD(3, 0, base_addr);
    
    PRINT("Test 4 completed\n");
}

void test_5_multiple_regs(uint32_t base_addr) {
    PRINT("\n=== Test 5: t.ld to multiple TRegs ===\n");
    
    CSR_WRITE(CSR_TMASK_LD, 0xFF);
    CSR_WRITE(CSR_TWIDTH_LD, 1024);
    configure_standard_ld_offsets();
    
    
    // Load to t4, t5, t6, t7
    T_LD(4, 0, base_addr);
    T_LD(5, 0, base_addr);
    T_LD(6, 0, base_addr);
    T_LD(7, 0, base_addr);
    
    PRINT("Loaded to t4, t5, t6, t7\n");
    PRINT("Test 5 completed\n");
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
    PRINT("======================================================================\n");
    PRINT("TensorComp t.ld ISA Test\n");
    PRINT("======================================================================\n");
    PRINT("\nInstruction: t.ld - Load from L0M to TReg\n");
    PRINT("Syntax: t.ld trd, imm10(rs1)\n");

    uint32_t base_addr = L1M_BASE_ADDR+2048;
    
    // Run tests
    test_1_full_mask(base_addr);
    test_2_partial_mask(base_addr);
    test_3_with_offset(base_addr);
    test_4_different_toffsets(base_addr);
    test_5_multiple_regs(base_addr);
    
    PRINT("\n======================================================================\n");
    PRINT("All t.ld tests completed!\n");
    PRINT("======================================================================\n");
    
    return 0;
}
