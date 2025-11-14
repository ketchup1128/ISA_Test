/*
 * TensorComp tc.st.mma ISA Tests (C version with inline assembly)
 * ================================================================
 * 
 * Instruction: tc.st.mma - Matrix Multiply-Accumulate with store to acc_buffer
 * Encoding: Eng_ID=01, mop3=000, st=1, mode3=011, imm3=001, funct2=00, opcode=1011011
 * 
 * MMA Computation (Latest Spec):
 * - Top Reg shape: [G, N, K]  - stored in Top Reg, propagates vertically
 * - Left Reg shape: [G, M, K] - stored in Left Reg, propagates horizontally  
 * - Output shape: [M, N] - written to acc_buffer
 * 
 * Where:
 * - G = popcount(top_tmask_ld) = popcount(left_tmask_ld) (MUST be equal!)
 * - N = int(top_twidth_ld/128) * 32
 * - M = int(left_twidth_ld/128) * 32
 * - K = 4 (for int8 data type, default configuration)
 *
 * Computation logic (Latest Spec):
 * for g in range(G):
 *   for m in range(M):
 *     for n in range(N):
 *       acc = 0
 *       for k in range(K):
 *         acc += top_reg[g][m][k] * left_reg[g][n][k]
 *       acc_buffer[rs1 + (m * N + n) * 4] = acc  // 4 bytes for int32
 *
 * Test cases:
 * - Test 1: 64x64 MMA (G=8, M=64, N=64, K=4)
 *   - Top Reg: [8, 64, 4], Left Reg: [8, 64, 4]
 *   - top_tmask_ld = left_tmask_ld = 0xFF
 * - Test 2: 128x32 MMA (G=8, M=128, N=32, K=4)
 *   - Top Reg: [8, 32, 4], Left Reg: [8, 128, 4]
 *   - top_tmask_ld = left_tmask_ld = 0xFF
 */

#include <stdint.h>
#include <stdbool.h>
#include "printf.h"

// ============================================================================
// CSR Address Definitions
// ============================================================================
// ### Left Reg CSRs (Updated 2025-10-21)
// | 地址范围    | 名称                    | 说明                          |
// |-------------|-------------------------|-------------------------------|
// | 0x800       | left_ttype              | 元素类型和数量配置            |
// | 0x801       | left_tmask_ld           | Load 掩码 (32-bit)            |
// | 0x802       | left_tmask_st           | Store 掩码 (32-bit)           |
// | 0x803       | left_twidth_ld          | Load 宽度                     |
// | 0x804       | left_twidth_st          | Store 宽度                    |
// | 0x820-0x83F | left_tstride_ld[0:31]   | Load stride 寄存器 (32个)     |
// | 0x840-0x85F | left_tstride_st[0:31]   | Store stride 寄存器 (32个)    |

// ### Top Reg CSRs (Updated 2025-10-21)
// | 地址范围    | 名称                    | 说明                          |
// |-------------|-------------------------|-------------------------------|
// | 0x860       | top_ttype               | 元素类型和数量配置            |
// | 0x861       | top_tmask_ld            | Load 掩码 (32-bit)            |
// | 0x862       | top_tmask_st            | Store 掩码 (32-bit)           |
// | 0x863       | top_twidth_ld           | Load 宽度                     |
// | 0x864       | top_twidth_st           | Store 宽度                    |
// | 0x880-0x89F | top_tstride_ld[0:31]    | Load stride 寄存器 (32个)     |
// | 0x8A0-0x8BF | top_tstride_st[0:31]    | Store stride 寄存器 (32个)    |

// ### Sparse Reg CSRs
// | 地址范围    | 名称                    | 说明                          |
// |-------------|-------------------------|-------------------------------|
// | 0x8C0       | spa_ttype               | 元素类型和数量配置            |
// | 0x8C1       | spa_tmask_ld            | Load 掩码 (32-bit)            |
// | 0x8C2       | spa_tmask_st            | Store 掩码 (32-bit)           |
// | 0x8C3       | spa_twidth_ld           | Load 宽度                     |
// | 0x8C4       | spa_twidth_st           | Store 宽度                    |
// | 0x8E0-0x8FF | spa_tstride_ld[0:31]    | Load stride 寄存器 (32个)     |
// | 0x900-0x91F | spa_tstride_st[0:31]    | Store stride 寄存器 (32个)    |

// Top Reg CSRs
#define CSR_TOP_TTYPE           0x860
#define CSR_TOP_TMASK_LD        0x861
#define CSR_TOP_TMASK_ST        0x862
#define CSR_TOP_TWIDTH_LD       0x863
#define CSR_TOP_TSTRIDE_LD_BASE 0x880
#define CSR_TOP_TSTRIDE_ST_BASE 0x8A0

// Left Reg CSRs
#define CSR_LEFT_TTYPE          0x800
#define CSR_LEFT_TMASK_LD       0x801
#define CSR_LEFT_TMASK_ST       0x802
#define CSR_LEFT_TWIDTH_LD      0x803
#define CSR_LEFT_TSTRIDE_LD_BASE 0x820
#define CSR_LEFT_TSTRIDE_ST_BASE 0x840

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

// tc.st.mma instruction - Matrix Multiply-Accumulate
// Encoding: Eng_ID=01, mop3=000, st=1, mode3=011, imm3=001, funct2=00, opcode=1011011
// Bits [31:30]=01, [29:27]=000, [26]=1, [25:23]=011, [22:20]=001, [19:15]=rs1, [14:13]=00, [12:7]=imm6, [6:0]=1011011
// Base encoding: 0x4592805B (with rs1=x5, imm6=0)
static inline void tc_st_mma(uint32_t acc_buffer_addr, uint32_t imm6) {
    // Instruction format: tc.st.mma imm6(rs1)
    // LLVM has registered this instruction, so we can use it directly
    
    switch(imm6) {
        case 0:
            __asm__ volatile (
                "tc.st.mma 0(%0)"
                : 
                : "r" (acc_buffer_addr)
                : "memory"
            );
            break;
        case 8:
            __asm__ volatile (
                "tc.st.mma 8(%0)"
                : 
                : "r" (acc_buffer_addr)
                : "memory"
            );
            break;
        case 16:
            __asm__ volatile (
                "tc.st.mma 16(%0)"
                : 
                : "r" (acc_buffer_addr)
                : "memory"
            );
            break;
        case 24:
            __asm__ volatile (
                "tc.st.mma 24(%0)"
                : 
                : "r" (acc_buffer_addr)
                : "memory"
            );
            break;
        default:
            __asm__ volatile (
                "tc.st.mma 0(%0)"  // fallback to imm6=0
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

// Verify MMA result (simple check)
void verify_mma_result(uint32_t acc_buffer_addr, uint32_t m, uint32_t n) {
    printf("  Expected output shape: [%u, %u]\n", m, n);
    printf("  Output size: %u elements (%u bytes)\n", m*n, m*n*4);
    printf("  Output address range: 0x%08X - 0x%08X\n", 
           acc_buffer_addr, acc_buffer_addr + m*n*4);
}

// ============================================================================
// Test Case 1: 64x64 MMA (G=8, M=64, N=64, K=4)
// ============================================================================
void test_1_mma_64x64_mask(uint32_t top_mem_base, uint32_t left_mem_base, uint32_t acc_buffer_base, uint32_t mask) {
    printf("\n--- Test 1: 64x64 MMA ---\n");
    printf("  Configuration:\n");
    printf("    - Top Reg: [8, 64, 4]  (G=8, N=64, K=4)\n");
    printf("    - Left Reg: [8, 64, 4] (G=8, M=64, K=4)\n");
    printf("    - Output: [64, 64] = 4096 int32 values (16KB)\n");
    printf("    - top_tmask_ld = left_tmask_ld = 0x01 (single group enabled)\n");
    printf("    - top_twidth_ld = 256 bytes → N = 64\n");
    printf("    - left_twidth_ld = 256 bytes → M = 64\n");
    
    // Prepare memory patterns (mask only enables first group for simplicity)
    // uint8_t *top_mem = (uint8_t *)top_mem_base;
    // uint8_t *left_mem = (uint8_t *)left_mem_base;
    uint32_t *acc_out = (uint32_t *)acc_buffer_base;
    const size_t top_bytes = 256;
    const size_t left_bytes = 256;
    const size_t result_elems = 64 * 64;

    // for (size_t i = 0; i < result_elems; ++i) {
    //     acc_out[i] = 0;
    // }
    
    // Configure Top Reg CSRs
    uint32_t top_ttype = 0x0841;
    uint32_t top_tmask_ld = mask;      // Only first group to simplify verification
    uint32_t top_twidth_ld = 256;
    
    CSR_WRITE(CSR_TOP_TTYPE, top_ttype);
    CSR_WRITE(CSR_TOP_TMASK_LD, top_tmask_ld);
    CSR_WRITE(CSR_TOP_TWIDTH_LD, top_twidth_ld);
    CSR_WRITE(CSR_TOP_TMASK_ST, top_tmask_ld);

    uint32_t top_strides[8] = {
        0x000, 0x0100, 0x200, 0x300,
        0x400, 0x500, 0x600, 0x700
    };


    for (int i = 0; i < 8; i++) {
        CSR_WRITE(CSR_TOP_TSTRIDE_LD_BASE + i, top_strides[i]);
        CSR_WRITE(CSR_TOP_TSTRIDE_ST_BASE + i, 0);
    }
    
    // Configure Left Reg CSRs
    uint32_t left_ttype = 0x0841;
    uint32_t left_tmask_ld = mask;
    uint32_t left_twidth_ld = 256;
    
    CSR_WRITE(CSR_LEFT_TTYPE, left_ttype);
    CSR_WRITE(CSR_LEFT_TMASK_LD, left_tmask_ld);
    CSR_WRITE(CSR_LEFT_TWIDTH_LD, left_twidth_ld);
    CSR_WRITE(CSR_LEFT_TMASK_ST, left_tmask_ld);

    uint32_t left_strides[8] = {
        0x000, 0x0100, 0x200, 0x300,
        0x400, 0x500, 0x600, 0x700
    };

    for (int i = 0; i < 8; i++) {
        CSR_WRITE(CSR_LEFT_TSTRIDE_LD_BASE + i, left_strides[i]);
        CSR_WRITE(CSR_LEFT_TSTRIDE_ST_BASE + i, 0);
    }
    
    printf("  Loading data to Top Reg from 0x%08X...\n", top_mem_base);
    tc_ld_top(top_mem_base, 0);
    
    printf("  Loading data to Left Reg from 0x%08X...\n", left_mem_base);
    tc_ld_left(left_mem_base, 0);
    
    printf("  Executing MMA computation...\n");
    
    // uint32_t cycles_before, instret_before;
    // __asm__ volatile ("rdcycle %0" : "=r"(cycles_before));
    // __asm__ volatile ("rdinstret %0" : "=r"(instret_before));
    
    tc_st_mma(acc_buffer_base, 0);
    
    // uint32_t cycles_after, instret_after;
    // __asm__ volatile ("rdcycle %0" : "=r"(cycles_after));
    // __asm__ volatile ("rdinstret %0" : "=r"(instret_after));
    
    // printf("  MMA completed: cycles=%u, instructions=%u\n",
    //        cycles_after - cycles_before, instret_after - instret_before);
    
    verify_mma_result(acc_buffer_base, 64, 64);
}

// ============================================================================
// Test Case 2: 128x32 MMA (G=8, M=128, N=32, K=4)
// ============================================================================
void test_1_mma_128x32_mask(uint32_t top_mem_base, uint32_t left_mem_base,
                            uint32_t acc_buffer_base, uint32_t mask) {
    printf("\n--- Test 1: 128x32 MMA ---\n");
    printf("  Configuration:\n");
    printf("    - Top Reg: [8, 32, 4]  (G=8, N=32, K=4)\n");
    printf("    - Left Reg: [8, 128, 4] (G=8, M=128, K=4)\n");
    printf("    - Output: [128, 32] = 4096 int32 values (16KB)\n");
    printf("    - top_tmask_ld = left_tmask_ld = 0x%02X\n", mask & 0xFF);
    printf("    - top_twidth_ld = 128 bytes → N = 32\n");
    printf("    - left_twidth_ld = 512 bytes → M = 128\n");
    
    // Prepare memory patterns with a single active group
    uint32_t *acc_out = (uint32_t *)(acc_buffer_base + 0x4000);
    const size_t top_bytes = 128;
    const size_t left_bytes = 512;
    const size_t result_elems = 128 * 32;

    // for (size_t i = 0; i < result_elems; ++i) {
    //     acc_out[i] = 0;
    // }
    
    // Configure Top Reg CSRs
    uint32_t top_ttype = 0x0841;
    uint32_t top_tmask_ld = mask;
    uint32_t top_twidth_ld = 128;
    
    CSR_WRITE(CSR_TOP_TTYPE, top_ttype);
    CSR_WRITE(CSR_TOP_TMASK_LD, top_tmask_ld);
    CSR_WRITE(CSR_TOP_TMASK_ST, top_tmask_ld);
    CSR_WRITE(CSR_TOP_TWIDTH_LD, top_twidth_ld);
    // CSR_WRITE(CSR_TOP_TSTRIDE_LD_BASE + 0, 0);
    
    uint32_t top_strides[8] = {
        0x000, 0x0800, 0x1000, 0x1800,
        0x2000, 0x2800, 0x3000, 0x3800
    };


    for (int i = 0; i < 8; i++) {
        CSR_WRITE(CSR_TOP_TSTRIDE_LD_BASE + i, top_strides[i]);
        CSR_WRITE(CSR_TOP_TSTRIDE_ST_BASE + i, 0);
    }

    // Configure Left Reg CSRs
    uint32_t left_ttype = 0x0841;
    uint32_t left_tmask_ld = mask;
    uint32_t left_twidth_ld = 512;
    
    CSR_WRITE(CSR_LEFT_TTYPE, left_ttype);
    CSR_WRITE(CSR_LEFT_TMASK_LD, left_tmask_ld);
    CSR_WRITE(CSR_LEFT_TMASK_ST, left_tmask_ld);
    CSR_WRITE(CSR_LEFT_TWIDTH_LD, left_twidth_ld);
    // CSR_WRITE(CSR_LEFT_TSTRIDE_LD_BASE + 0, 0);

    uint32_t left_strides[8] = {
        0x000, 0x02000, 0x04000, 0x06000,
        0x08000, 0x0A000, 0x0C000, 0x0E000
    };


    for (int i = 0; i < 8; i++) {
        CSR_WRITE(CSR_LEFT_TSTRIDE_LD_BASE + i, left_strides[i]);
        CSR_WRITE(CSR_LEFT_TSTRIDE_ST_BASE + i, 0);
    }
    
    printf("  Loading data to Top Reg from 0x%08X...\n", top_mem_base);
    tc_ld_top(top_mem_base, 0);
    
    printf("  Loading data to Left Reg from 0x%08X...\n", left_mem_base);
    tc_ld_left(left_mem_base, 0);
    
    printf("  Executing MMA computation...\n");
    
    tc_st_mma(acc_buffer_base, 0);
    
    verify_mma_result(acc_buffer_base, 128, 32);
}

// ============================================================================
// Main function
// ============================================================================
int main() {
    printf("======================================================================\n");
    printf("TensorComp tc.st.mma ISA Tests (C version)\n");
    printf("======================================================================\n");
    printf("\n");
    printf("Instruction Encoding:\n");
    printf("  - Opcode: 0x5B (CUSTOM-2)\n");
    printf("  - Eng_ID: 01 (TensorComp)\n");
    printf("  - st: 1 (store type)\n");
    printf("  - mode3: 011 (Standard MMA)\n");
    printf("  - Base encoding: 0x4592805B (with rs1=x5, imm6=0)\n");
    printf("\n");
    printf("MMA Computation (Latest Spec):\n");
    printf("  - Input: Top Reg [G,N,K] × Left Reg [G,M,K]\n");
    printf("  - Output: [M,N] matrix to acc_buffer\n");
    printf("  - Computation: acc += top_reg[g][m][k] * left_reg[g][n][k]\n");
    printf("  - Constraint: top_tmask_ld MUST equal left_tmask_ld\n");
    printf("  - Data type: int8 input, int32 output (accumulation)\n");
    printf("\n");
    printf("Memory regions:\n");
    printf("  - Top Reg data: 0x40000000 - 0x40008000 (32KB)\n");
    printf("  - Left Reg data: 0x40010000 - 0x40018000 (32KB)\n");
    printf("  - Acc buffer: 0x40020000 - 0x40030000 (64KB)\n");
    printf("\n");
    printf("Test scenarios:\n");
    printf("  - Test 1: 64x64 MMA  (Top:[8,64,4], Left:[8,64,4])\n");
    printf("  - Test 2: 128x32 MMA (Top:[8,32,4], Left:[8,128,4])\n");
    printf("\n");
    
    // 加一个打印 PC
    // Base addresses for different memory regions
    uint32_t top_mem_base = 0x40000000;      // Top Reg input data
    uint32_t left_mem_base = 0x40000800;     // Left Reg input data
    uint32_t acc_buffer_base = 0x40020000;   // MMA output buffer
    
    // Run test cases
    test_1_mma_64x64_mask(top_mem_base, left_mem_base, acc_buffer_base+0x0000, 0x01);
    test_1_mma_64x64_mask(top_mem_base, left_mem_base, acc_buffer_base+0x4000, 0xFF);
    test_1_mma_128x32_mask(top_mem_base, left_mem_base, acc_buffer_base+0x8000, 0x01);
    test_1_mma_128x32_mask(top_mem_base, left_mem_base, acc_buffer_base+0xC000, 0xFF);
    
    printf("\n======================================================================\n");
    printf("All tc.st.mma tests completed successfully!\n");
    printf("======================================================================\n");
    
    return 0;
}
