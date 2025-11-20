/*
 * TensorComp tc.ld ISA Tests (C version with inline assembly)
 * ============================================================
 * 
 * Instruction: tc.ld - Unified load instruction from L0M to TC Registers
 * Encoding: [31:25] funct7=0000100, [24:20] imm5, [19:15] rs1, [14:12] funct3=000, [11:7] trs, [6:0] opcode=1011011
 * 
 * Instruction Format:
 * | [31:25] | [24:20] | [19:15] | [14:12] | [11:7] | [6:0] |
 * | funct7  |  imm5   |   rs1   | funct3  |  trs   | opcode|
 * | 0000100 |  imm5   |   rs1   |  000    |  trs   |1011011|
 * 
 * trs: Target TC register index [0-31]
 * 
 * Addressing mode (from TensorComp spec):
 * base_addr = rs1 + imm6 * twidth_ld
 * for i in range(tgroup_ld):
 *   if (tmask_ld[i] == 1):
 *     addr = base_addr + tstride_ld[i]
 *     reg[i * twidth_ld +: twidth_ld] = MEM[addr +: twidth_ld]
 *   else:
 *     reg[i * twidth_ld +: twidth_ld] = 0
 * 
 * Test configurations:
 * 1. Full mask (0xFF), single load to Top Reg (trs=0)
 * 2. Partial mask (0x55), single load to Top Reg (trs=0)
 */

#include <stdint.h>
#include "printf.h"

// ============================================================================
// CSR Address Definitions for Unified tc.ld
// ============================================================================

// #define CSR_TGROUP_LD          0x801  // Group number for load (32-bit mask)
#define CSR_TMASK_LD           0x800  // Load mask (32-bit, each bit controls a split load)
// #define CSR_TWIDTH_LD          0x803  // Load width per split operation
#define CSR_TSTRIDE_LD_BASE    0x820  // Load stride[0:31] (0x820-0x83F)

// ============================================================================
// Inline Assembly Helpers
// ============================================================================

// CSR write helper
#define CSR_WRITE(csr, value) \
    __asm__ volatile ("csrw %0, %1" :: "i"(csr), "r"((uint32_t)(value)))

// CSR read helper
#define CSR_READ(csr, dest) \
    __asm__ volatile ("csrr %0, %1" : "=r"(dest) : "i"(csr))

// tc.ld instruction inline assembly
// Using mnemonic: tc.ld tds, imm(rs)
//   - tds: target TC register index [0-31] (trs field, encoded in rd position)
//   - imm: immediate offset (imm5 field, 5-bit signed: -16 to +15)
//   - rs: source register containing base address (rs1 field)
// 
// Instruction format: tc.ld tds, imm(rs)
// Example: tc.ld x0, 0(x5)  // Load to TC reg 0, base address in x5, offset 0
static inline void tc_ld(uint32_t base_addr, int32_t imm5, uint32_t trs) {
    // Move base_addr to x5 (temporary register for base address)
    // Execute tc.ld instruction using mnemonic
    // Format: tc.ld tds, imm(rs)
    // Note: tds (trs) is the target TC register index, encoded in rd field
    // Since TC registers are not general-purpose registers, we use it as input constraint
    // but it gets encoded in the rd field position of the instruction
    __asm__ volatile (
        "tc.ld tc0, %0(%1)"
        :
        : "i"(imm5), "r"(base_addr)
    );
}

// ============================================================================
// Configuration Functions
// ============================================================================

// Configure common CSRs for tc.ld tests
void configure_mask( uint32_t mask) {
    // Configure group number
    // CSR_WRITE(CSR_TGROUP_LD, tgroup);
    
    // Configure mask
    CSR_WRITE(CSR_TMASK_LD, mask);
}

// Configure stride registers
void configure_strides(const uint32_t strides[8]) {
    for (int i = 0; i < 8; i++) {
        CSR_WRITE(CSR_TSTRIDE_LD_BASE + i, strides[i]);
    }
}

// ============================================================================
// Test Cases
// ============================================================================

void test_1_full_mask_top_reg(uint32_t base_address) {
    int32_t imm5 = 0;   // First block (5-bit signed: -16 to +15)
    uint32_t trs = 0;   // Target TC register index [0-31]
    
    printf("\n=== Test 1: tc.ld with full mask (0xFF) to TC Reg (trs=0) ===\n");
    printf("base=0x%08X, imm5=%d, trs=%u\n", base_address, imm5, trs);
    
    // Configure strides for 4 split loads
    uint32_t strides[8] = {
        0x00000000,  // stride[0] = 0
        0x00000080,  // stride[1] = 128
        0x00000100,  // stride[2] = 256
        0x00000180,   // stride[3] = 384
        0x00000200,   // stride[4] = 512
        0x00000280,   // stride[5] = 640
        0x00000300,   // stride[6] = 768
        0x00000380,   // stride[7] = 896
       // stride[3] = 384
    };
    
    // Configure CSRs
    configure_mask(0x000000FF);  // 4 groups, mask=0xF (enable first 4)
    configure_strides(strides);
    
    for (int j = 0; j < 8; j++) {
        base_address = base_address + j*0x400;
        printf("Base address: 0x%08X\n", base_address);
        printf("Expected load addresses (with strides): [");
        for (int i = 0; i < 8; i++) {
            printf("0x%08X", base_address + strides[i]);
            if (i < 7) printf(", ");
        }
        printf("]\n");
        
        // Execute tc.ld instruction using mnemonic: tc.ld tds, imm(rs)
        tc_ld(base_address, imm5, trs);
    }
    
    printf("✓ Test 1 completed\n");
}

void test_2_partial_mask_top_reg(uint32_t base_address) {
    int32_t imm5 = 0;   // Offset (5-bit signed: -16 to +15)
    uint32_t trs = 0;   // Target TC register index [0-31]
    
    printf("\n=== Test 2: tc.ld with partial mask (0x5) to TC Reg (trs=0) ===\n");
    printf("base=0x%08X, imm5=%d, trs=%u, mask=0x5 (loads 0,2 enabled)\n", 
           base_address, imm5, trs);
    
    // Configure strides
    uint32_t strides[8] = {
        0x00000000,  // stride[0] = 0
        0x00000080,  // stride[1] = 128
        0x00000100,  // stride[2] = 256
        0x00000180,   // stride[3] = 384
        0x00000200,   // stride[4] = 512
        0x00000280,   // stride[5] = 640
        0x00000300,   // stride[6] = 768
        0x00000380,   // stride[7] = 896
       // stride[3] = 384
    };
    
    configure_strides(strides);

    for (int j = 0; j < 8; j++) {
        configure_mask(0xFF&j); 
        base_address = base_address + j*0x400;
        printf("Base address: 0x%08X\n", base_address);
        printf("Expected load addresses (with strides): [");
        for (int i = 0; i < 8; i++) {
            printf("0x%08X", base_address + strides[i]);
            if (i < 7) printf(", ");
        }
        printf("]\n");
        
        // Execute tc.ld instruction using mnemonic: tc.ld tds, imm(rs)
        tc_ld(base_address, imm5, trs);
    }
    
    printf("✓ Test 2 completed\n");
}

// ============================================================================
// Main Function
// ============================================================================

int main(void) {
    printf("======================================================================\n");
    printf("TensorComp tc.ld ISA Tests (C version)\n");
    printf("======================================================================\n");
    printf("\nInstruction Encoding:\n");
    printf("  - Opcode: 0x5B (CUSTOM-2)\n");
    printf("  - funct7: 0000100 (0x04)\n");
    printf("  - funct3: 000\n");
    printf("  - Format: tc.ld tds, imm(rs)\n");
    printf("    where: tds = target TC reg [0-31], imm = 5-bit signed offset [-16,+15], rs = base addr register\n");
    printf("\nConfiguration:\n");
    printf("  - tmask_ld: Load mask (32-bit, each bit controls a split load)\n");
    printf("  - tstride_ld[0:31]: Stride registers for address calculation\n");
    printf("\nCSR Address Mapping:\n");
    // printf("  - tgroup_ld:     0x%03X\n", CSR_TGROUP_LD);
    printf("  - tmask_ld:      0x%03X\n", CSR_TMASK_LD);
    printf("  - tstride_ld:    0x%03X-0x%03X\n", CSR_TSTRIDE_LD_BASE, CSR_TSTRIDE_LD_BASE + 31);
    printf("\n");
    
    uint32_t base_address = 0x40000000;
    
    // Run all 4 tests
    test_1_full_mask_top_reg(base_address);
    test_2_partial_mask_top_reg(base_address);
    
    printf("\n======================================================================\n");
    printf("All tests completed!\n");
    printf("======================================================================\n");
    
    return 0;
}

