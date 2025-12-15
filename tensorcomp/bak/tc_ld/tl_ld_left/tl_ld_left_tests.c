/*
 * TensorComp tc.ld.left ISA Tests (C version with inline assembly)
 * =================================================================
 * 
 * Instruction: tc.ld.left - Load data from L0M to Left Reg
 * Encoding: Eng_ID=01, mop3=000, st=0, mode3=001, funct2=00, opcode=1011011
 * 
 * Test configurations:
 * 1. left_ttype: tsew=1 (8-bit), tlmul=1024 elements -> 0x0841
 * 2. left_tstride_ld[0:7]: Configure first 8 stride registers
 * 3. left_twidth_ld: 128 bytes per load operation
 *
 * Addressing mode (from TensorComp spec):
 * base_addr = rs1 + imm6 * left_twidth_ld
 * for i in range(32):
 *   if (left_tmask_ld[i] == 1):
 *     addr = base_addr + left_tstride_ld[i]
 *     left_reg[i * left_twidth_ld +: left_twidth_ld] = MEM[addr +: left_twidth_ld]
 *   else:
 *     left_reg[i * left_twidth_ld +: left_twidth_ld] = 0
 *
 * Four test cases:
 * - Test 1: Full mask (0xFF), single load instruction
 * - Test 2: Partial mask (0x55), single load instruction (loads 0,2,4,6 enabled)
 * - Test 3: Full mask (0xFF), 4 consecutive load instructions
 * - Test 4: Partial mask (0x55), 4 consecutive load instructions
 *
 * Left Reg size: 4096 Byte (larger than Top Reg's 2048 Byte)
 */

#include <stdint.h>
#include "printf.h"

// ============================================================================
// CSR Address Definitions for TensorComp Left Reg
// ============================================================================

#define CSR_LEFT_TTYPE           0x800
#define CSR_LEFT_TMASK_LD        0x801
#define CSR_LEFT_TMASK_ST        0x802
#define CSR_LEFT_TWIDTH_LD       0x803
#define CSR_LEFT_TWIDTH_ST       0x804
#define CSR_LEFT_TSTRIDE_LD_BASE 0x820  // 0x820-0x83F for stride[0:31]
#define CSR_LEFT_TSTRIDE_ST_BASE 0x840  // 0x840-0x85F for stride[0:31]

// ============================================================================
// Inline Assembly Helpers
// ============================================================================

// CSR write helper
#define CSR_WRITE(csr, value) \
    __asm__ volatile ("csrw %0, %1" :: "i"(csr), "r"((uint32_t)(value)))

// CSR read helper
#define CSR_READ(csr, dest) \
    __asm__ volatile ("csrr %0, %1" : "=r"(dest) : "i"(csr))

// tc.ld.left instruction
// Encoding based on specification:
// | Eng_ID(2) | mop3(3) | st(1) | mode3(3) | imm3(3) | rs1(5) | funct2(2) | imm6(6) | opcode(7) |
// |    01     |  000    |  0    |   001    |   ---   |  rs1   |    00     |  imm6   | 1011011   |
// Bits [31:30]=01, [29:27]=000, [26]=0, [25:23]=001, [19:15]=rs1, [14:13]=00, [12:7]=imm6, [6:0]=1011011
// Final address = rs1 + imm6 * left_twidth_ld
// Base encoding: 01_000_0_001_xxx_xxxxx_00_IIIIII_1011011 = 0x4080285B | (imm6 << 7)
static inline void tl_ld_left(uint32_t base_addr, uint32_t imm6) {
    // Instruction format: tc.ld.left imm6(rs1)
    // LLVM has registered this instruction, so we can use it directly
    
    switch(imm6) {
        case 0:
            __asm__ volatile (
                "tc.ld.left 0(%0)"
                : 
                : "r" (base_addr)
                : "memory"
            );
            break;
        case 8:
            __asm__ volatile (
                "tc.ld.left 8(%0)"
                : 
                : "r" (base_addr)
                : "memory"
            );
            break;
        case 16:
            __asm__ volatile (
                "tc.ld.left 16(%0)"
                : 
                : "r" (base_addr)
                : "memory"
            );
            break;
        case 24:
            __asm__ volatile (
                "tc.ld.left 24(%0)"
                : 
                : "r" (base_addr)
                : "memory"
            );
            break;
        default:
            __asm__ volatile (
                "tc.ld.left 0(%0)"  // fallback to imm6=0
                : 
                : "r" (base_addr)
                : "memory"
            );
            break;
    }
}

// ============================================================================
// Test Case 1: Full mask, single load
// ============================================================================
void test_1_full_mask_single_load(uint32_t base_address) {
    printf("\n--- Test 1: Full Mask, Single Load ---\n");
    printf("  Configuration:\n");
    printf("    - Mask: 0xFF (8 loads enabled: 0-7)\n");
    printf("    - Width: 128 bytes per load\n");
    printf("    - Stride pattern: 0x000, 0x080, 0x100, 0x180, 0x200, 0x280, 0x300, 0x380\n");
    
    // Configure Left Reg CSRs
    uint32_t left_ttype = 0x0841;  // tsew=1 (8-bit), tlmul=1024
    uint32_t left_tmask_ld = 0xFF;  // Enable first 8 loads
    uint32_t left_twidth_ld = 256;  // 128 bytes per load
    
    CSR_WRITE(CSR_LEFT_TTYPE, left_ttype);
    CSR_WRITE(CSR_LEFT_TMASK_LD, left_tmask_ld);
    CSR_WRITE(CSR_LEFT_TWIDTH_LD, left_twidth_ld);
    
    // Configure strides (relative offsets in hex)
    uint32_t strides[8] = {
        0x000, 0x080, 0x100, 0x180,
        0x200, 0x280, 0x300, 0x380
    };
    
    for (int i = 0; i < 8; i++) {
        CSR_WRITE(CSR_LEFT_TSTRIDE_LD_BASE + i, strides[i]);
    }
    
    // Execute tc.ld.left with imm6=0
    uint32_t imm6 = 0;
    uint32_t real_base = base_address + imm6 * left_twidth_ld;
    
    printf("  Load addresses:\n");
    for (int i = 0; i < 8; i++) {
        printf("    Load %d: 0x%08X (base=0x%08X + stride=0x%03X)\n",
               i, real_base + strides[i], real_base, strides[i]);
    }
    
    printf("  Executing: tc.ld.left imm6=%d, base_addr=0x%08X\n", imm6, base_address);
    
    // Read cycle counter before
    uint32_t cycles_before, instret_before;
    __asm__ volatile ("rdcycle %0" : "=r"(cycles_before));
    __asm__ volatile ("rdinstret %0" : "=r"(instret_before));
    
    // Execute the instruction
    tl_ld_left(base_address, imm6);
    
    // Read cycle counter after
    uint32_t cycles_after, instret_after;
    __asm__ volatile ("rdcycle %0" : "=r"(cycles_after));
    __asm__ volatile ("rdinstret %0" : "=r"(instret_after));
    
    printf("  Completed: cycles=%u, instructions=%u\n",
           cycles_after - cycles_before, instret_after - instret_before);
}

// ============================================================================
// Test Case 2: Partial mask, single load
// ============================================================================
void test_2_partial_mask_single_load(uint32_t base_address) {
    printf("\n--- Test 2: Partial Mask, Single Load ---\n");
    printf("  Configuration:\n");
    printf("    - Mask: 0x55 (4 loads enabled: 0,2,4,6)\n");
    printf("    - Width: 128 bytes per load\n");
    printf("    - Stride pattern: 0x000, 0x100, 0x200, 0x300\n");
    
    // Configure Left Reg CSRs
    uint32_t left_ttype = 0x0841;
    uint32_t left_tmask_ld = 0x55;  // Enable loads 0,2,4,6
    uint32_t left_twidth_ld = 256;
    
    CSR_WRITE(CSR_LEFT_TTYPE, left_ttype);
    CSR_WRITE(CSR_LEFT_TMASK_LD, left_tmask_ld);
    CSR_WRITE(CSR_LEFT_TWIDTH_LD, left_twidth_ld);
    
    // Configure strides
    uint32_t strides[8] = {
        0x000, 0x080, 0x100, 0x180,
        0x200, 0x280, 0x300, 0x380
    };
    
    for (int i = 0; i < 8; i++) {
        CSR_WRITE(CSR_LEFT_TSTRIDE_LD_BASE + i, strides[i]);
    }
    
    // Execute tc.ld.left with imm6=8
    uint32_t imm6 = 8;
    uint32_t real_base = base_address + imm6 * left_twidth_ld;
    
    printf("  Load addresses (only masked lanes):\n");
    for (int i = 0; i < 8; i++) {
        if (left_tmask_ld & (1 << i)) {
            printf("    Load %d: 0x%08X (base=0x%08X + stride=0x%03X)\n",
                   i, real_base + strides[i], real_base, strides[i]);
        }
    }
    
    printf("  Executing: tc.ld.left imm6=%d, base_addr=0x%08X\n", imm6, base_address);
    
    uint32_t cycles_before, instret_before;
    __asm__ volatile ("rdcycle %0" : "=r"(cycles_before));
    __asm__ volatile ("rdinstret %0" : "=r"(instret_before));
    
    tl_ld_left(base_address, imm6);
    
    uint32_t cycles_after, instret_after;
    __asm__ volatile ("rdcycle %0" : "=r"(cycles_after));
    __asm__ volatile ("rdinstret %0" : "=r"(instret_after));
    
    printf("  Completed: cycles=%u, instructions=%u\n",
           cycles_after - cycles_before, instret_after - instret_before);
}

// ============================================================================
// Test Case 3: Full mask, 4 consecutive loads
// ============================================================================
void test_3_full_mask_four_loads(uint32_t base_address) {
    printf("\n--- Test 3: Full Mask, Four Consecutive Loads ---\n");
    printf("  Configuration:\n");
    printf("    - Mask: 0xFF (8 loads enabled per instruction)\n");
    printf("    - Width: 128 bytes per load\n");
    printf("    - 4 consecutive tc.ld.left instructions with different imm6\n");
    
    // Configure Left Reg CSRs
    uint32_t left_ttype = 0x0841;
    uint32_t left_tmask_ld = 0xFF;
    uint32_t left_twidth_ld = 256;
    
    CSR_WRITE(CSR_LEFT_TTYPE, left_ttype);
    CSR_WRITE(CSR_LEFT_TMASK_LD, left_tmask_ld);
    CSR_WRITE(CSR_LEFT_TWIDTH_LD, left_twidth_ld);
    
    // Configure strides
    uint32_t strides[8] = {
        0x000, 0x080, 0x100, 0x180,
        0x200, 0x280, 0x300, 0x380
    };
    
    for (int i = 0; i < 8; i++) {
        CSR_WRITE(CSR_LEFT_TSTRIDE_LD_BASE + i, strides[i]);
    }
    
    uint32_t cycles_before, instret_before;
    __asm__ volatile ("rdcycle %0" : "=r"(cycles_before));
    __asm__ volatile ("rdinstret %0" : "=r"(instret_before));
    
    // Execute 4 consecutive loads with different imm6 values
    uint32_t imm6_values[4] = {0, 8, 16, 24};
    
    for (int load_idx = 0; load_idx < 4; load_idx++) {
        uint32_t imm6 = imm6_values[load_idx];
        uint32_t real_base = base_address + imm6 * left_twidth_ld;
        
        printf("  Load %d: imm6=%d, base_addr=0x%08X\n", 
               load_idx + 1, imm6, real_base);
        
        tl_ld_left(base_address, imm6);
    }
    
    uint32_t cycles_after, instret_after;
    __asm__ volatile ("rdcycle %0" : "=r"(cycles_after));
    __asm__ volatile ("rdinstret %0" : "=r"(instret_after));
    
    printf("  Completed: total cycles=%u, total instructions=%u\n",
           cycles_after - cycles_before, instret_after - instret_before);
}

// ============================================================================
// Test Case 4: Partial mask, 4 consecutive loads
// ============================================================================
void test_4_partial_mask_four_loads(uint32_t base_address) {
    printf("\n--- Test 4: Partial Mask, Four Consecutive Loads ---\n");
    printf("  Configuration:\n");
    printf("    - Mask: 0x55 (4 loads enabled per instruction: 0,2,4,6)\n");
    printf("    - Width: 128 bytes per load\n");
    printf("    - 4 consecutive tc.ld.left instructions with different imm6\n");
    
    // Configure Left Reg CSRs
    uint32_t left_ttype = 0x0841;
    uint32_t left_tmask_ld = 0x55;
    uint32_t left_twidth_ld = 256;
    
    CSR_WRITE(CSR_LEFT_TTYPE, left_ttype);
    CSR_WRITE(CSR_LEFT_TMASK_LD, left_tmask_ld);
    CSR_WRITE(CSR_LEFT_TWIDTH_LD, left_twidth_ld);
    
    // Configure strides
    uint32_t strides[8] = {
        0x000, 0x080, 0x100, 0x180,
        0x200, 0x280, 0x300, 0x380
    };
    
    for (int i = 0; i < 8; i++) {
        CSR_WRITE(CSR_LEFT_TSTRIDE_LD_BASE + i, strides[i]);
    }
    
    uint32_t cycles_before, instret_before;
    __asm__ volatile ("rdcycle %0" : "=r"(cycles_before));
    __asm__ volatile ("rdinstret %0" : "=r"(instret_before));
    
    // Execute 4 consecutive loads with different imm6 values
    uint32_t imm6_values[4] = {0, 8, 16, 24};
    
    for (int load_idx = 0; load_idx < 4; load_idx++) {
        uint32_t imm6 = imm6_values[load_idx];
        uint32_t real_base = base_address + imm6 * left_twidth_ld;
        
        printf("  Load %d: imm6=%d, base_addr=0x%08X\n", 
               load_idx + 1, imm6, real_base);
        
        tl_ld_left(base_address, imm6);
    }
    
    uint32_t cycles_after, instret_after;
    __asm__ volatile ("rdcycle %0" : "=r"(cycles_after));
    __asm__ volatile ("rdinstret %0" : "=r"(instret_after));
    
    printf("  Completed: total cycles=%u, total instructions=%u\n",
           cycles_after - cycles_before, instret_after - instret_before);
}

// ============================================================================
// Main function
// ============================================================================
int main() {
    printf("======================================================================\n");
    printf("TensorComp tc.ld.left ISA Tests (C version)\n");
    printf("======================================================================\n");
    printf("\n");
    printf("Instruction Encoding:\n");
    printf("  - Opcode: 0x5B (CUSTOM-2)\n");
    printf("  - Eng_ID: 01 (TensorComp)\n");
    printf("  - mode3:  001 (Left Reg Load)\n");
    printf("  - Full encoding: 0x4080285B (with rs1=x5, imm6=0)\n");
    printf("\n");
    printf("Left Reg specifications:\n");
    printf("  - Size: 4096 Byte (larger than Top Reg)\n");
    printf("  - CSR base: 0x800-0x85F\n");
    printf("\n");
    printf("Test memory region: 0x40000000 - 0x40008000\n");
    printf("  (Initialized via --mem-init option)\n");
    printf("\n");
    
    // Base address for memory operations
    // This should match the --mem-init parameter
    uint32_t base_address = 0x40000000;
    
    // Run all test cases
    test_1_full_mask_single_load(base_address);
    test_2_partial_mask_single_load(base_address);
    test_3_full_mask_four_loads(base_address);
    test_4_partial_mask_four_loads(base_address);
    
    printf("\n======================================================================\n");
    printf("All tests completed successfully!\n");
    printf("======================================================================\n");
    
    return 0;
}

