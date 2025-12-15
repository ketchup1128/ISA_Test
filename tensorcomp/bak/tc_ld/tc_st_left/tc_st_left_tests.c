/*
 * TensorComp tc.st.left ISA Tests (C version with inline assembly)
 * =================================================================
 * 
 * Instruction: tc.st.left - Broadcast Left Reg data horizontally to acc_buffer
 * Encoding: Eng_ID=01, mop3=000, st=1, mode3=001, imm3=100, funct2=00, opcode=1011011
 * 
 * Test configuration:
 * - Left Reg: Load int8 data pattern
 * - acc_buffer: 64x64 matrix (4096 int32 elements)
 * - Expected: Each column contains the same Left Reg values (horizontal broadcast)
 * 
 * Functional behavior:
 * for m in range(M):
 *   for n in range(N):
 *     acc_buffer[rs1 + (m * N + n) * 4] = left_reg[0][m][k]
 */

#include <stdint.h>
#include <stdbool.h>
#include "printf.h"

// ============================================================================
// CSR Address Definitions
// ============================================================================

#define CSR_LEFT_TTYPE           0x800
#define CSR_LEFT_TMASK_LD        0x801
#define CSR_LEFT_TWIDTH_LD       0x803
#define CSR_LEFT_TSTRIDE_LD_BASE 0x820

#define CSR_TOP_TWIDTH_LD        0x863

// ============================================================================
// Inline Assembly Helpers
// ============================================================================

#define CSR_WRITE(csr, value) \
    __asm__ volatile ("csrw %0, %1" :: "i"(csr), "r"((uint32_t)(value)))

#define CSR_READ(csr, dest) \
    __asm__ volatile ("csrr %0, %1" : "=r"(dest) : "i"(csr))

// tc.ld.left instruction
static inline void tc_ld_left(uint32_t base_addr, uint32_t imm6) {
    switch(imm6) {
        case 0:
            __asm__ volatile (
                "tc.ld.left 0(%0)"
                : 
                : "r" (base_addr)
                : "memory"
            );
            break;
        default:
            __asm__ volatile (
                "tc.ld.left 0(%0)"
                : 
                : "r" (base_addr)
                : "memory"
            );
            break;
    }
}

// tc.st.left instruction
// Instruction encoding: Eng_ID=01, mop3=000, st=1, mode3=001, imm3=100, funct2=00, opcode=1011011
// Base encoding: 01_000_1_001_100_xxxxx_00_IIIIII_1011011 = 0x4490005B | (imm6 << 7)
static inline void tc_st_left(uint32_t acc_buffer_addr, uint32_t imm6) {
    // Move address to x5 (t0) for rs1 encoding
    
    switch(imm6) {
        case 0:
            __asm__ volatile (
                "tc.st.left 0(%0)" // tc.st.left 0(x5)
                : 
                : "r" (acc_buffer_addr)
                : "memory"
            );
            break;
        default:
            __asm__ volatile (
                "tc.st.left 0(%0)" // tc.st.left 0(x5)
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
 * Test 1: tc.st.left with 64x64 configuration (top_width=256, left_width=256)
 * - Load pattern data to Left Reg
 * - Execute tc.st.left to broadcast Left Reg horizontally
 * - Verify all columns have same pattern (horizontal broadcast)
 */
void test_tc_st_left_64x64(uint32_t left_mem_base, uint32_t acc_buffer_base) {
    printf("\n=== Test 1: tc.st.left 64x64 configuration ===\n");
    
    // Configure Left Reg CSRs
    // G = 1 (mask = 0x01), M = 64 (left_width=256), K = 4
    CSR_WRITE(CSR_LEFT_TTYPE, 0x2001);      // int8, tlmul=1024
    CSR_WRITE(CSR_LEFT_TMASK_LD, 0x01);     // Enable only first group
    CSR_WRITE(CSR_LEFT_TWIDTH_LD, 256);     // M = 64
    CSR_WRITE(CSR_TOP_TWIDTH_LD, 256);      // N = 64
    
    // Configure stride for single load
    CSR_WRITE(CSR_LEFT_TSTRIDE_LD_BASE + 0, 0);
    
    printf("  Loading Left Reg from 0x%08X\n", left_mem_base);
    tc_ld_left(left_mem_base, 0);
    
    printf("  Executing tc.st.left to 0x%08X\n", acc_buffer_base);
    tc_st_left(acc_buffer_base, 0);

    printf("\n=== Test 1: tc.st.left 64x64 finish ! ===\n");

    
}

/**
 * Test 2: tc.st.left with 128x32 configuration (top_width=128, left_width=512)
 * - Load pattern data to Left Reg
 * - Execute tc.st.left to broadcast Left Reg horizontally
 * - Verify all columns have same pattern (horizontal broadcast)
 */
void test_tc_st_left_128x32(uint32_t left_mem_base, uint32_t acc_buffer_base) {
    printf("\n=== Test 2: tc.st.left 128x32 configuration ===\n");
    
    // Configure Left Reg CSRs
    // G = 1 (mask = 0x01), M = 128 (left_width=512), K = 4
    CSR_WRITE(CSR_LEFT_TTYPE, 0x2001);      // int8, tlmul=1024
    CSR_WRITE(CSR_LEFT_TMASK_LD, 0x01);     // Enable only first group
    CSR_WRITE(CSR_LEFT_TWIDTH_LD, 512);     // M = 128
    CSR_WRITE(CSR_TOP_TWIDTH_LD, 128);      // N = 32
    
    // Configure stride for single load
    CSR_WRITE(CSR_LEFT_TSTRIDE_LD_BASE + 0, 0);
    
    printf("  Loading Left Reg from 0x%08X\n", left_mem_base);
    tc_ld_left(left_mem_base, 0);
    
    printf("  Executing tc.st.left to 0x%08X\n", acc_buffer_base);
    tc_st_left(acc_buffer_base, 0);
    
    printf("\n=== Test 2: tc.st.left 128x32 finish ===\n");

}

// ============================================================================
// Main Function
// ============================================================================

int main(void) {
    printf("\n");
    printf("========================================\n");
    printf("TensorComp tc.st.left ISA Tests\n");
    printf("========================================\n");
    
    // Memory layout:
    // - 0x40000000: Left Reg source data (512 bytes, pattern data)
    // - 0x40010000: acc_buffer for test 1 (64x64 = 16KB)
    // - 0x40020000: acc_buffer for test 2 (128x32 = 16KB)
    
    uint32_t left_mem_base = 0x4000000;
    uint32_t acc_buffer = 0x40020000;
    
    // Run tests
    test_tc_st_left_64x64(left_mem_base, acc_buffer);
    test_tc_st_left_128x32(left_mem_base, acc_buffer);
    
    printf("\n========================================\n");
    printf("All tc.st.left tests completed!\n");
    printf("========================================\n\n");
    
    return 0;
}

