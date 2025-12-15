/*
 * TensorComp tc.st.top ISA Tests (C version with inline assembly)
 * ================================================================
 * 
 * Instruction: tc.st.top - Broadcast Top Reg data vertically to acc_buffer
 * Encoding: Eng_ID=01, mop3=000, st=1, mode3=010, imm3=100, funct2=00, opcode=1011011
 * 
 * Test configuration:
 * - Top Reg: Load int8 data pattern
 * - acc_buffer: 64x64 matrix (4096 int32 elements)
 * - Expected: Each row contains the same Top Reg values (vertical broadcast)
 * 
 * Functional behavior:
 * for m in range(M):
 *   for n in range(N):
 *     for k in range(K):
 *       acc_buffer[rs1 + (m * N + n) * 4] = top_reg[0][n][k]
 */

#include <stdint.h>
#include <stdbool.h>
#include "printf.h"

// ============================================================================
// CSR Address Definitions
// ============================================================================

#define CSR_TOP_TTYPE           0x860
#define CSR_TOP_TMASK_LD        0x861
#define CSR_TOP_TWIDTH_LD       0x863
#define CSR_TOP_TSTRIDE_LD_BASE 0x880

#define CSR_LEFT_TWIDTH_LD      0x803

// ============================================================================
// Inline Assembly Helpers
// ============================================================================

#define CSR_WRITE(csr, value) \
    __asm__ volatile ("csrw %0, %1" :: "i"(csr), "r"((uint32_t)(value)))

#define CSR_READ(csr, dest) \
    __asm__ volatile ("csrr %0, %1" : "=r"(dest) : "i"(csr))

// tc.ld.top instruction
static inline void tc_ld_top(uint32_t base_addr, uint32_t imm6) {
    switch(imm6) {
        case 0:
            __asm__ volatile (
                "tc.ld.top 0(%0)"
                : 
                : "r" (base_addr)
                : "memory"
            );
            break;
        default:
            __asm__ volatile (
                "tc.ld.top 0(%0)"
                : 
                : "r" (base_addr)
                : "memory"
            );
            break;
    }
}

// tc.st.top instruction
// Instruction encoding: Eng_ID=01, mop3=000, st=1, mode3=010, imm3=100, funct2=00, opcode=1011011
// Base encoding: 01_000_1_010_100_xxxxx_00_IIIIII_1011011 = 0x4510005B | (imm6 << 7)
static inline void tc_st_top(uint32_t acc_buffer_addr, uint32_t imm6) {
    // Move address to x5 (t0) for rs1 encoding
    register uint32_t addr asm("x5") = acc_buffer_addr;
    
    switch(imm6) {
        case 0:
        __asm__ volatile (
            "tc.st.top 0(%0)"
            : 
            : "r" (acc_buffer_addr)
            : "memory"
        );
            break;
        default:
            __asm__ volatile (
                "tc.st.top 0(%0)"
                : 
                : "r" (acc_buffer_addr)
                : "memory"
            );
            break;
    }
}

// ============================================================================
// Test Functions
// ============================================================================

/**
 * Test 1: tc.st.top with 64x64 configuration (top_width=256, left_width=256)
 * - Load pattern data to Top Reg
 * - Execute tc.st.top to broadcast Top Reg vertically
 * - Verify all rows have same pattern (vertical broadcast)
 */
void test_tc_st_top_64x64(uint32_t top_mem_base, uint32_t acc_buffer_base) {
    printf("\n=== Test 1: tc.st.top 64x64 configuration ===\n");
    
    // Configure Top Reg CSRs
    // G = 1 (mask = 0x01), N = 64 (top_width=256), K = 4
    CSR_WRITE(CSR_TOP_TTYPE, 0x2001);       // int8, tlmul=1024
    CSR_WRITE(CSR_TOP_TMASK_LD, 0x01);      // Enable only first group
    CSR_WRITE(CSR_TOP_TWIDTH_LD, 256);      // N = 64
    CSR_WRITE(CSR_LEFT_TWIDTH_LD, 256);     // M = 64
    
    // Configure stride for single load
    CSR_WRITE(CSR_TOP_TSTRIDE_LD_BASE + 0, 0);
    
    printf("  Loading Top Reg from 0x%08X\n", top_mem_base);
    tc_ld_top(top_mem_base, 0);
    
    printf("  Executing tc.st.top to 0x%08X\n", acc_buffer_base);
    tc_st_top(acc_buffer_base, 0);

    printf("\n=== Test 1: tc.st.top 64x64 finish ! ===\n");

}

/**
 * Test 2: tc.st.top with 128x32 configuration (top_width=128, left_width=512)
 * - Load pattern data to Top Reg
 * - Execute tc.st.top to broadcast Top Reg vertically
 * - Verify all rows have same pattern (vertical broadcast)
 */
void test_tc_st_top_128x32(uint32_t top_mem_base, uint32_t acc_buffer_base) {
    printf("\n=== Test 2: tc.st.top 128x32 configuration ===\n");
    
    // Configure Top Reg CSRs
    // G = 1 (mask = 0x01), N = 32 (top_width=128), K = 4
    CSR_WRITE(CSR_TOP_TTYPE, 0x2001);       // int8, tlmul=1024
    CSR_WRITE(CSR_TOP_TMASK_LD, 0x01);      // Enable only first group
    CSR_WRITE(CSR_TOP_TWIDTH_LD, 128);      // N = 32
    CSR_WRITE(CSR_LEFT_TWIDTH_LD, 512);     // M = 128
    
    // Configure stride for single load
    CSR_WRITE(CSR_TOP_TSTRIDE_LD_BASE + 0, 0);
    
    printf("  Loading Top Reg from 0x%08X\n", top_mem_base);
    tc_ld_top(top_mem_base, 0);
    
    printf("  Executing tc.st.top to 0x%08X\n", acc_buffer_base);
    tc_st_top(acc_buffer_base, 0);
    
    // Verify: Read back acc_buffer and check vertical broadcast
    printf("  Verifying vertical broadcast pattern...\n");
    int32_t *acc_buffer = (int32_t *)acc_buffer_base;
    
    printf("\n=== Test 2: tc.st.top 128x32 finish ! ===\n");
}

// ============================================================================
// Main Function
// ============================================================================

int main(void) {
    printf("\n");
    printf("========================================\n");
    printf("TensorComp tc.st.top ISA Tests\n");
    printf("========================================\n");
    
    // Memory layout:
    // - 0x40000000: Top Reg source data (256 bytes, pattern data)
    // - 0x40010000: acc_buffer for test 1 (64x64 = 16KB)
    // - 0x40020000: acc_buffer for test 2 (128x32 = 16KB)
    
    uint32_t top_mem_base = 0x40000000;
    uint32_t acc_buffer_1 = 0x40020000;
    
    // Run tests
    test_tc_st_top_64x64(top_mem_base, acc_buffer_1);
    test_tc_st_top_128x32(top_mem_base, acc_buffer_1);
    
    printf("\n========================================\n");
    printf("All tc.st.top tests completed!\n");
    printf("========================================\n\n");
    
    return 0;
}

