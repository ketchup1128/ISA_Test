/*
 * TensorComp tc.st.mma.spa ISA Tests (C version with inline assembly)
 * ====================================================================
 * 
 * Instruction: tc.st.mma.spa - Sparse Matrix Multiply-Accumulate with 4:2 structured sparsity
 * Encoding: Eng_ID=01, mop3=000, st=1, mode3=111, imm3=001, funct2=00, opcode=1011011
 * 
 * Sparse MMA Computation (4:2 Structured Sparsity):
 * - Top Reg shape: [G, N, K]  - stored in Top Reg (compressed sparse data), each element 8-bit
 * - Spa Reg shape: [G, N, K]  - stored in Spa Reg (sparse indices), each element 2-bit
 * - Left Reg shape: [G, 2, M, K] - stored in Left Reg (full data), each element 8-bit
 * - Output shape: [M, N] - written to acc_buffer, each element 32-bit
 * 
 * Where:
 * - G = popcount(top_tmask_ld) = popcount(left_tmask_ld) (MUST be equal!)
 * - N = 64 (fixed for sparse MMA, top_twidth_ld = 256)
 * - M = 64 (fixed for sparse MMA, left_twidth_ld = 512)
 * - K = 4 (for int8 data type, default configuration)
 *
 * 4:2 Sparse Computation Logic:
 * for g in range(G):
 *   for m in range(M):
 *     for n in range(N):
 *       acc = 0
 *       for c in range(2):
 *         for k in range(K/2):
 *           sparse_idx = spa_reg[g][n][c*2+k]
 *           if (sparse_idx < 2):
 *             acc += top_reg[g][n][c*2+k] * left_reg[g][0][m][c*2+sparse_idx]
 *           else:
 *             acc += top_reg[g][n][c*2+k] * left_reg[g][1][m][c*2+sparse_idx-2]
 *       acc_buffer[rs1 + (m * N + n) * 4] = acc  // 4 bytes for int32
 *
 * Test cases:
 * - Test 1: 64x64 Sparse MMA with single group (G=1)
 * - Test 2: 64x64 Sparse MMA with multiple groups (G=8)
 */

#include <stdint.h>
#include <stdbool.h>
#include "printf.h"

// ============================================================================
// CSR Address Definitions
// ============================================================================

// Top Reg CSRs
#define CSR_TOP_TTYPE           0x860
#define CSR_TOP_TMASK_LD        0x861
#define CSR_TOP_TWIDTH_LD       0x863
#define CSR_TOP_TSTRIDE_LD_BASE 0x880

// Left Reg CSRs
#define CSR_LEFT_TTYPE          0x800
#define CSR_LEFT_TMASK_LD       0x801
#define CSR_LEFT_TWIDTH_LD      0x803
#define CSR_LEFT_TSTRIDE_LD_BASE 0x820

// Spa Reg CSRs
#define CSR_SPA_TTYPE           0x8C0
#define CSR_SPA_TMASK_LD        0x8C1
#define CSR_SPA_TWIDTH_LD       0x8C3
#define CSR_SPA_TSTRIDE_LD_BASE 0x8E0

// ============================================================================
// Inline Assembly Helpers
// ============================================================================

// CSR write helper
#define CSR_WRITE(csr, value) \
    __asm__ volatile ("csrw %0, %1" :: "i"(csr), "r"((uint32_t)(value)))

// CSR read helper
#define CSR_READ(csr, dest) \
    __asm__ volatile ("csrr %0, %1" : "=r"(dest) : "i"(csr))

// tc.ld.top instruction - load data to Top Reg
static inline void tc_ld_top(uint32_t base_addr, uint32_t imm6) {
    switch(imm6) {
        case 0:
            __asm__ volatile ("tc.ld.top 0(%0)" : : "r" (base_addr) : "memory");
            break;
        case 8:
            __asm__ volatile ("tc.ld.top 8(%0)" : : "r" (base_addr) : "memory");
            break;
        case 16:
            __asm__ volatile ("tc.ld.top 16(%0)" : : "r" (base_addr) : "memory");
            break;
        default:
            __asm__ volatile ("tc.ld.top 0(%0)" : : "r" (base_addr) : "memory");
            break;
    }
}

// tc.ld.left instruction - load data to Left Reg
static inline void tc_ld_left(uint32_t base_addr, uint32_t imm6) {
    switch(imm6) {
        case 0:
            __asm__ volatile ("tc.ld.left 0(%0)" : : "r" (base_addr) : "memory");
            break;
        case 8:
            __asm__ volatile ("tc.ld.left 8(%0)" : : "r" (base_addr) : "memory");
            break;
        case 16:
            __asm__ volatile ("tc.ld.left 16(%0)" : : "r" (base_addr) : "memory");
            break;
        default:
            __asm__ volatile ("tc.ld.left 0(%0)" : : "r" (base_addr) : "memory");
            break;
    }
}

// tc.ld.spa instruction - load data to Spa Reg
static inline void tc_ld_spa(uint32_t base_addr, uint32_t imm6) {
    switch(imm6) {
        case 0:
            __asm__ volatile ("tc.ld.spa 0(%0)" : : "r" (base_addr) : "memory");
            break;
        case 8:
            __asm__ volatile ("tc.ld.spa 8(%0)" : : "r" (base_addr) : "memory");
            break;
        case 16:
            __asm__ volatile ("tc.ld.spa 16(%0)" : : "r" (base_addr) : "memory");
            break;
        default:
            __asm__ volatile ("tc.ld.spa 0(%0)" : : "r" (base_addr) : "memory");
            break;
    }
}

// tc.st.mma.spa instruction - Sparse Matrix Multiply-Accumulate
// Encoding: Eng_ID=01, mop3=000, st=1, mode3=111, imm3=001, funct2=00, opcode=1011011
// Bits [31:30]=01, [29:27]=000, [26]=1, [25:23]=111, [22:20]=001, [19:15]=rs1, [14:13]=00, [12:7]=imm6, [6:0]=1011011
// Base encoding: 0x47D2805B (with rs1=x5, imm6=0)
static inline void tc_st_mma_spa(uint32_t acc_buffer_addr, uint32_t imm6) {
    // Instruction format: tc.st.mma.spa imm6(rs1)
    // LLVM has registered this instruction, so we can use it directly
    
    switch(imm6) {
        case 0:
            __asm__ volatile (
                "tc.st.mma.spa 0(%0)"
                : 
                : "r" (acc_buffer_addr)
                : "memory"
            );
            break;
        case 8:
            __asm__ volatile (
                "tc.st.mma.spa 8(%0)"
                : 
                : "r" (acc_buffer_addr)
                : "memory"
            );
            break;
        case 16:
            __asm__ volatile (
                "tc.st.mma.spa 16(%0)"
                : 
                : "r" (acc_buffer_addr)
                : "memory"
            );
            break;
        case 24:
            __asm__ volatile (
                "tc.st.mma.spa 24(%0)"
                : 
                : "r" (acc_buffer_addr)
                : "memory"
            );
            break;
        default:
            __asm__ volatile (
                "tc.st.mma.spa 0(%0)"  // fallback to imm6=0
                : 
                : "r" (acc_buffer_addr)
                : "memory"
            );
            break;
    }
}

// ============================================================================
// Helper Functions
// ============================================================================

// Initialize test data in memory
// void init_sparse_test_data(uint32_t top_mem_base, uint32_t left_mem_base, 
//                            uint32_t spa_mem_base, uint32_t acc_buffer_base) {
//     // Initialize Top Reg data (sparse activations)
//     // For simplicity, fill with incremental values
//     uint8_t *top_mem = (uint8_t *)top_mem_base;
//     for (size_t i = 0; i < 256; i++) {
//         top_mem[i] = (uint8_t)(i % 16);  // Values 0-15 repeating
//     }
    
//     // Initialize Left Reg data (full weights with 2 banks)
//     // Shape: [1, 2, 64, 4] = 512 bytes
//     uint8_t *left_mem = (uint8_t *)left_mem_base;
//     for (size_t i = 0; i < 512; i++) {
//         left_mem[i] = (uint8_t)((i % 8) + 1);  // Values 1-8 repeating
//     }
    
//     // Initialize Spa Reg data (sparse indices, 2-bit per index)
//     // Shape: [1, 64, 4] with 2-bit per element = 64 bytes
//     uint8_t *spa_mem = (uint8_t *)spa_mem_base;
//     for (size_t i = 0; i < 64; i++) {
//         // Each byte contains 4 indices (2-bit each)
//         // Pattern: 0,1,2,3 repeating
//         spa_mem[i] = 0xE4;  // 11 10 01 00 = 3,2,1,0
//     }
    
//     // Initialize acc_buffer to zero
//     uint32_t *acc_out = (uint32_t *)acc_buffer_base;
//     for (size_t i = 0; i < 64 * 64; i++) {
//         acc_out[i] = 0;
//     }
// }

// // Verify sparse MMA result (simple check)
// void verify_sparse_mma_result(uint32_t acc_buffer_addr, uint32_t m, uint32_t n) {
//     printf("  Expected output shape: [%u, %u]\n", m, n);
//     printf("  Output size: %u elements (%u bytes)\n", m*n, m*n*4);
//     printf("  Output address range: 0x%08X - 0x%08X\n", 
//            acc_buffer_addr, acc_buffer_addr + m*n*4);
    
//     // Sample first few results
//     uint32_t *acc_out = (uint32_t *)acc_buffer_addr;
//     printf("  Sample results (first 8 elements):\n    ");
//     for (int i = 0; i < 8; i++) {
//         printf("%d ", (int32_t)acc_out[i]);
//     }
//     printf("\n");
// }

// ============================================================================
// Test Case 1: 64x64 Sparse MMA with single group (G=1)
// ============================================================================
void test_1_sparse_mma_64x64_single_group(uint32_t top_mem_base, uint32_t left_mem_base, 
                                          uint32_t spa_mem_base, uint32_t acc_buffer_base) {
    printf("\n--- Test 1: 64x64 Sparse MMA (Single Group) ---\n");
    printf("  Configuration:\n");
    printf("    - Top Reg: [1, 64, 4]    (G=1, N=64, K=4)\n");
    printf("    - Spa Reg: [1, 64, 4]    (G=1, N=64, K=4) - 2-bit indices\n");
    printf("    - Left Reg: [1, 2, 64, 4] (G=1, M=64, K=4, 2 banks)\n");
    printf("    - Output: [64, 64] = 4096 int32 values (16KB)\n");
    printf("    - top_tmask_ld = left_tmask_ld = 0x01 (single group enabled)\n");
    printf("    - top_twidth_ld = 256 bytes → N = 64\n");
    printf("    - left_twidth_ld = 512 bytes → M = 64\n");
    
    // Initialize test data
    // init_sparse_test_data(top_mem_base, left_mem_base, spa_mem_base, acc_buffer_base);
    
    // Configure Top Reg CSRs
    uint32_t top_ttype = 0x0841;
    uint32_t top_tmask_ld = 0x01;      // Only first group
    uint32_t top_twidth_ld = 256;
    
    CSR_WRITE(CSR_TOP_TTYPE, top_ttype);
    CSR_WRITE(CSR_TOP_TMASK_LD, top_tmask_ld);
    CSR_WRITE(CSR_TOP_TWIDTH_LD, top_twidth_ld);
    CSR_WRITE(CSR_TOP_TSTRIDE_LD_BASE + 0, 0);
    
    // Configure Left Reg CSRs
    uint32_t left_ttype = 0x0841;
    uint32_t left_tmask_ld = 0x01;
    uint32_t left_twidth_ld = 512;
    
    CSR_WRITE(CSR_LEFT_TTYPE, left_ttype);
    CSR_WRITE(CSR_LEFT_TMASK_LD, left_tmask_ld);
    CSR_WRITE(CSR_LEFT_TWIDTH_LD, left_twidth_ld);
    CSR_WRITE(CSR_LEFT_TSTRIDE_LD_BASE + 0, 0);
    
    // Configure Spa Reg CSRs
    uint32_t spa_ttype = 0x0400;       // 2-bit indices
    uint32_t spa_tmask_ld = 0x01;
    uint32_t spa_twidth_ld = 64;       // 256/4 = 64 bytes (4:1 compression)
    
    CSR_WRITE(CSR_SPA_TTYPE, spa_ttype);
    CSR_WRITE(CSR_SPA_TMASK_LD, spa_tmask_ld);
    CSR_WRITE(CSR_SPA_TWIDTH_LD, spa_twidth_ld);
    CSR_WRITE(CSR_SPA_TSTRIDE_LD_BASE + 0, 0);
    
    printf("  Loading data to Top Reg from 0x%08X...\n", top_mem_base);
    tc_ld_top(top_mem_base, 0);
    
    printf("  Loading data to Left Reg from 0x%08X...\n", left_mem_base);
    tc_ld_left(left_mem_base, 0);
    
    printf("  Loading sparse indices to Spa Reg from 0x%08X...\n", spa_mem_base);
    tc_ld_spa(spa_mem_base, 0);
    
    printf("  Executing Sparse MMA computation...\n");
    tc_st_mma_spa(acc_buffer_base, 0);
    
    // verify_sparse_mma_result(acc_buffer_base, 64, 64);
}

// ============================================================================
// Test Case 2: 64x64 Sparse MMA with multiple groups (G=8)
// ============================================================================
void test_2_sparse_mma_64x64_multi_group(uint32_t top_mem_base, uint32_t left_mem_base,
                                         uint32_t spa_mem_base, uint32_t acc_buffer_base) {
    printf("\n--- Test 2: 64x64 Sparse MMA (Multiple Groups) ---\n");
    printf("  Configuration:\n");
    printf("    - Top Reg: [8, 64, 4]    (G=8, N=64, K=4)\n");
    printf("    - Spa Reg: [8, 64, 4]    (G=8, N=64, K=4) - 2-bit indices\n");
    printf("    - Left Reg: [8, 2, 64, 4] (G=8, M=64, K=4, 2 banks)\n");
    printf("    - Output: [64, 64] = 4096 int32 values (16KB)\n");
    printf("    - top_tmask_ld = left_tmask_ld = 0xFF (8 groups enabled)\n");
    printf("    - top_twidth_ld = 256 bytes → N = 64\n");
    printf("    - left_twidth_ld = 512 bytes → M = 64\n");
    
    // Initialize test data
    // init_sparse_test_data(top_mem_base, left_mem_base, spa_mem_base, acc_buffer_base + 0x4000);
    
    // Configure Top Reg CSRs
    uint32_t top_ttype = 0x0841;
    uint32_t top_tmask_ld = 0xFF;      // 8 groups enabled
    uint32_t top_twidth_ld = 256;
    
    CSR_WRITE(CSR_TOP_TTYPE, top_ttype);
    CSR_WRITE(CSR_TOP_TMASK_LD, top_tmask_ld);
    CSR_WRITE(CSR_TOP_TWIDTH_LD, top_twidth_ld);
    
    uint32_t top_strides[8] = {
        0x000, 0x0100, 0x200, 0x300,
        0x400, 0x500, 0x600, 0x700
    };
    
    for (int i = 0; i < 8; i++) {
        CSR_WRITE(CSR_TOP_TSTRIDE_LD_BASE + i, top_strides[i]);
    }
    
    // Configure Left Reg CSRs
    uint32_t left_ttype = 0x0841;
    uint32_t left_tmask_ld = 0xFF;
    uint32_t left_twidth_ld = 512;
    
    CSR_WRITE(CSR_LEFT_TTYPE, left_ttype);
    CSR_WRITE(CSR_LEFT_TMASK_LD, left_tmask_ld);
    CSR_WRITE(CSR_LEFT_TWIDTH_LD, left_twidth_ld);
    
    uint32_t left_strides[8] = {
        0x000, 0x0200, 0x400, 0x600,
        0x800, 0xA00, 0xC00, 0xE00
    };
    
    for (int i = 0; i < 8; i++) {
        CSR_WRITE(CSR_LEFT_TSTRIDE_LD_BASE + i, left_strides[i]);
    }
    
    // Configure Spa Reg CSRs
    uint32_t spa_ttype = 0x0400;
    uint32_t spa_tmask_ld = 0xFF;
    uint32_t spa_twidth_ld = 64;
    
    CSR_WRITE(CSR_SPA_TTYPE, spa_ttype);
    CSR_WRITE(CSR_SPA_TMASK_LD, spa_tmask_ld);
    CSR_WRITE(CSR_SPA_TWIDTH_LD, spa_twidth_ld);
    
    uint32_t spa_strides[8] = {
        0x000, 0x040, 0x080, 0x0C0,
        0x100, 0x140, 0x180, 0x1C0
    };
    
    for (int i = 0; i < 8; i++) {
        CSR_WRITE(CSR_SPA_TSTRIDE_LD_BASE + i, spa_strides[i]);
    }
    
    printf("  Loading data to Top Reg from 0x%08X...\n", top_mem_base);
    tc_ld_top(top_mem_base, 0);
    
    printf("  Loading data to Left Reg from 0x%08X...\n", left_mem_base);
    tc_ld_left(left_mem_base, 0);
    
    printf("  Loading sparse indices to Spa Reg from 0x%08X...\n", spa_mem_base);
    tc_ld_spa(spa_mem_base, 0);
    
    printf("  Executing Sparse MMA computation...\n");
    tc_st_mma_spa(acc_buffer_base + 0x4000, 0);
    
    // verify_sparse_mma_result(acc_buffer_base + 0x4000, 64, 64);
}

// ============================================================================
// Main function
// ============================================================================
int main() {
    printf("======================================================================\n");
    printf("TensorComp tc.st.mma.spa ISA Tests (C version)\n");
    printf("======================================================================\n");
    printf("\n");
    printf("Instruction Encoding:\n");
    printf("  - Opcode: 0x5B (CUSTOM-2)\n");
    printf("  - Eng_ID: 01 (TensorComp)\n");
    printf("  - st: 1 (store type)\n");
    printf("  - mode3: 111 (Sparse MMA with 4:2 sparsity)\n");
    printf("  - Base encoding: 0x47D2805B (with rs1=x5, imm6=0)\n");
    printf("\n");
    printf("Sparse MMA Computation (4:2 Structured Sparsity):\n");
    printf("  - Input: Top Reg [G,N,K] (compressed) × Left Reg [G,2,M,K] (full)\n");
    printf("  - Indices: Spa Reg [G,N,K] (2-bit per element)\n");
    printf("  - Output: [M,N] matrix to acc_buffer\n");
    printf("  - Sparsity: 4:2 structured (2 non-zero per 4 elements)\n");
    printf("  - Data type: int8 input, int32 output (accumulation)\n");
    printf("\n");
    printf("Memory regions:\n");
    printf("  - Top Reg data: 0x40000000 - 0x40008000 (32KB)\n");
    printf("  - Left Reg data: 0x40008000 - 0x40010000 (32KB)\n");
    printf("  - Spa Reg data: 0x40010000 - 0x40012000 (32KB)\n");
    printf("  - Acc buffer: 0x40020000 - 0x40030000 (64KB)\n");
    printf("\n");
    printf("Test scenarios:\n");
    printf("  - Test 1: 64x64 Sparse MMA with G=1\n");
    printf("  - Test 2: 64x64 Sparse MMA with G=8\n");
    printf("\n");
    
    // Base addresses for different memory regions
    uint32_t top_mem_base = 0x40000000;      // Top Reg input data
    uint32_t left_mem_base = 0x40008000;      // Left Reg sparse indices
    uint32_t spa_mem_base = 0x40010000;     // Spa Reg input data
    uint32_t acc_buffer_base = 0x40020000;   // MMA output buffer
    
    // Run test cases
    test_1_sparse_mma_64x64_single_group(top_mem_base, left_mem_base, spa_mem_base+128*8, acc_buffer_base);
    test_2_sparse_mma_64x64_multi_group(top_mem_base, left_mem_base, spa_mem_base+128*8, acc_buffer_base);
    
    printf("\n======================================================================\n");
    printf("All tc.st.mma.spa tests completed successfully!\n");
    printf("======================================================================\n");
    
    return 0;
}

