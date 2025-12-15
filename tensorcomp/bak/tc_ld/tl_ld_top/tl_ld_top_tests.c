/*
 * TensorComp tl.ld.top ISA Tests (C version with inline assembly)
 * ================================================================
 * 
 * Instruction: tl.ld.top - Load data from L0M to Top Reg
 * Encoding: Eng_ID=01, mop3=000, st=0, mode3=010, funct2=00, opcode=1011011
 * 
 * Test configurations:
 * 1. top_ttype: tsew=1 (8-bit), tlmul=1024 elements -> 0x0841
 * 2. top_tstride_ld[0:7]: Configure first 8 stride registers
 * 3. top_twidth_ld: 256 bytes per load operation
 *
 * Addressing mode (from TensorComp spec):
 * base_addr = rs1 + imm6 * top_twidth_ld
 * for i in range(32):
 *   if (top_tmask_ld[i] == 1):
 *     addr = base_addr + top_tstride_ld[i]
 *     top_reg[i * top_twidth_ld +: top_twidth_ld] = MEM[addr +: top_twidth_ld]
 *   else:
 *     top_reg[i * top_twidth_ld +: top_twidth_ld] = 0
 *
 * Four test cases:
 * - Test 1: Full mask (0xFF), single load instruction
 * - Test 2: Partial mask (0x55), single load instruction (loads 0,2,4,6 enabled)
 * - Test 3: Full mask (0xFF), 4 consecutive load instructions
 * - Test 4: Partial mask (0x55), 4 consecutive load instructions
 */

 #include <stdint.h>
 #include "printf.h"
 
 // ============================================================================
 // CSR Address Definitions for TensorComp Top Reg
 // ============================================================================
 
 #define CSR_TOP_TTYPE           0x860
 #define CSR_TOP_TMASK_LD        0x861
 #define CSR_TOP_TMASK_ST        0x862
 #define CSR_TOP_TWIDTH_LD       0x863
 #define CSR_TOP_TWIDTH_ST       0x864
 #define CSR_TOP_TSTRIDE_LD_BASE 0x880  // 0x880-0x89F for stride[0:31]
 #define CSR_TOP_TSTRIDE_ST_BASE 0x8A0  // 0x8A0-0x8BF for stride[0:31]
 
 // ============================================================================
 // Inline Assembly Helpers
 // ============================================================================
 
 // CSR write helper
 #define CSR_WRITE(csr, value) \
     __asm__ volatile ("csrw %0, %1" :: "i"(csr), "r"((uint32_t)(value)))
 
 // CSR read helper
 #define CSR_READ(csr, dest) \
     __asm__ volatile ("csrr %0, %1" : "=r"(dest) : "i"(csr))
 
// tl.ld.top instruction
// Encoding based on specification:
// | Eng_ID(2) | mop3(3) | st(1) | mode3(3) | imm3(3) | rs1(5) | funct2(2) | imm6(6) | opcode(7) |
// |    01     |  000    |  0    |   010    |   ---   |  rs1   |    00     |  imm6   | 1011011   |
// Bits [31:30]=01, [29:27]=000, [26]=0, [25:23]=010, [19:15]=rs1, [14:13]=00, [12:7]=imm6, [6:0]=1011011
// Final address = rs1 + imm6 * top_twidth_ld
// Base encoding: 01_000_0_010_xxx_00101_00_IIIIII_1011011 = 0x410282DB | (imm6 << 7)
#define TL_LD_TOP_IMM6(imm6_val) \
    __asm__ volatile ( \
        ".word " #imm6_val \
        : \
        : \
        : "memory" \
    )

static inline void tl_ld_top(uint32_t base_addr, uint32_t imm6) {
    // Instruction format: tc.ld.top imm6(rs1) with rs1=x5
    // Use .word directive with correct instruction encodings.
    // The mv instruction ensures base_addr is moved to x5 before the custom instruction.
    
    switch(imm6) {
        case 0:
            __asm__ volatile (
                "tc.ld.top 0(%0)"
                : 
                : "r" (base_addr)
                : "memory"
            );
            break;
        case 8:
            __asm__ volatile (
                "tc.ld.top 8(%0)"
                : 
                : "r" (base_addr)
                : "memory"
            );
            break;
        case 16:
            __asm__ volatile (
                "tc.ld.top 16(%0)"
                : 
                : "r" (base_addr)
                : "memory"
            );
            break;
        case 24:
            __asm__ volatile (
                "tc.ld.top 24(%0)"
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
 
 // Performance counter helpers
 static inline uint32_t read_cycle(void) {
     uint32_t cycles;
     __asm__ volatile ("rdcycle %0" : "=r"(cycles));
     return cycles;
 }
 
 static inline uint32_t read_instret(void) {
     uint32_t instret;
     __asm__ volatile ("rdinstret %0" : "=r"(instret));
     return instret;
 }
 
 
 // ============================================================================
 // Configuration Functions
 // ============================================================================
 
 // Configure common CSRs for all tests
 void configure_common_csrs(void) {
     // top_ttype: tsew=1 (8-bit), tlmul=1024
     // Encoding: tsew[2:0]=001, tlmul[17:3]=0x84 (1024)
     CSR_WRITE(CSR_TOP_TTYPE, 0x2001);
     
     // top_tmask_ld: Enable first 8 loads (0xFF)
     CSR_WRITE(CSR_TOP_TMASK_LD, 0x000000FF);
     
     // top_twidth_ld: 256 bytes per load
     CSR_WRITE(CSR_TOP_TWIDTH_LD, 256);
 }
 
 // Configure stride registers
 void configure_strides(const uint32_t strides[8]) {
     for (int i = 0; i < 8; i++) {
         CSR_WRITE(CSR_TOP_TSTRIDE_LD_BASE + i, strides[i]);
     }
 }
 
 // ============================================================================
 // Test Cases
 // ============================================================================
 
 void test_1_full_mask_single_load(uint32_t base_address) {
     uint32_t imm6 = 0;  // First block
     uint32_t real_base = base_address + imm6 * 256;  // imm6 * top_twidth_ld
     printf("\n=== Test 1: tl.ld.top with base=0x%08X, imm6=%u ===\n", base_address, imm6);
     
     // Configure strides for 8 split loads
     uint32_t strides[8] = {
         0x00000000,  // stride[0] = 0
         0x00000100,  // stride[1] = 256
         0x00000200,  // stride[2] = 512
         0x00000300,  // stride[3] = 768
         0x00000400,  // stride[4] = 1024
         0x00000500,  // stride[5] = 1280
         0x00000600,  // stride[6] = 1536
         0x00000700   // stride[7] = 1792
     };
     
     // Configure CSRs
     configure_common_csrs();
     configure_strides(strides);
     
     // Performance measurement
     uint32_t start_cycle = read_cycle();
     uint32_t start_instret = read_instret();
     
     // Execute tl.ld.top instruction
     // Final address for each load will be:
     // addr[i] = (rs1 + imm6 * 256) + stride[i]
     tl_ld_top(base_address, imm6);
     
     uint32_t end_cycle = read_cycle();
     uint32_t end_instret = read_instret();
     
     // Print results
     printf("Load addresses: [");
     for (int i = 0; i < 8; i++) {
         printf("0x%08X", real_base + strides[i]);
         if (i < 7) printf(", ");
     }
     printf("]\n");
     printf("Performance: %u cycles, %u instructions\n", 
            end_cycle - start_cycle, end_instret - start_instret);
     printf("✓ Test 1 completed\n");
 }
 
 void test_2_partial_mask_single_load(uint32_t base_address) {
     uint32_t imm6 = 0;  // Same as test 1
     uint32_t real_base = base_address + imm6 * 256;  // imm6 * top_twidth_ld
     printf("\n=== Test 2: tl.ld.top with partial mask (0x55) ===\n");
     printf("base=0x%08X, imm6=%u, mask=0x55 (loads 0,2,4,6 enabled)\n", base_address, imm6);
     
     // Configure strides for 8 split loads (same as test 1)
     uint32_t strides[8] = {
         0x00000000,  // stride[0] = 0
         0x00000100,  // stride[1] = 256
         0x00000200,  // stride[2] = 512
         0x00000300,  // stride[3] = 768
         0x00000400,  // stride[4] = 1024
         0x00000500,  // stride[5] = 1280
         0x00000600,  // stride[6] = 1536
         0x00000700   // stride[7] = 1792
     };
     
     // Configure CSRs with partial mask
     // top_ttype: tsew=1 (8-bit), tlmul=1024
     CSR_WRITE(CSR_TOP_TTYPE, 0x0841);
     
     // top_tmask_ld: Enable only loads 0,2,4,6 (0x55 = 0b01010101)
     CSR_WRITE(CSR_TOP_TMASK_LD, 0x00000055);
     
     // top_twidth_ld: 256 bytes per load
     CSR_WRITE(CSR_TOP_TWIDTH_LD, 256);
     
     configure_strides(strides);
     
     // Performance measurement
     uint32_t start_cycle = read_cycle();
     uint32_t start_instret = read_instret();
     
     // Execute tl.ld.top instruction
     // Only loads at index 0,2,4,6 will be executed
     // Loads at index 1,3,5,7 will be filled with 0
     tl_ld_top(base_address, imm6);
     
     uint32_t end_cycle = read_cycle();
     uint32_t end_instret = read_instret();
     
     // Print results
     printf("Load addresses (enabled only): [");
     for (int i = 0; i < 8; i++) {
         if (i > 0) printf(", ");
         if ((0x55 >> i) & 1) {
             printf("0x%08X", real_base + strides[i]);
         } else {
             printf("MASKED");
         }
     }
     printf("]\n");
     printf("Performance: %u cycles, %u instructions\n", 
            end_cycle - start_cycle, end_instret - start_instret);
     printf("✓ Test 2 completed\n");
 }
 
 void test_3_full_mask_four_loads(uint32_t base_address) {
     printf("\n=== Test 3: tl.ld.top with full mask (0xFF) - 4 consecutive loads ===\n");
     printf("base=0x%08X, mask=0xFF (all 8 loads enabled per instruction)\n", base_address);
     
     // Configure strides for 8 split loads (same as test 1)
     uint32_t strides[8] = {
         0x00000000,  // stride[0] = 0
         0x00000100,  // stride[1] = 256
         0x00000200,  // stride[2] = 512
         0x00000300,  // stride[3] = 768
         0x00000400,  // stride[4] = 1024
         0x00000500,  // stride[5] = 1280
         0x00000600,  // stride[6] = 1536
         0x00000700   // stride[7] = 1792
     };
     
     // Configure CSRs (same as test 1)
     configure_common_csrs();
     configure_strides(strides);
     
     // Performance measurement
     uint32_t start_cycle = read_cycle();
     uint32_t start_instret = read_instret();
     
     // Execute 4 consecutive tl.ld.top instructions with different imm6 values
     printf("Issuing 4 consecutive tl.ld.top instructions...\n");
     for (uint32_t i = 0; i < 4; i++) {
         uint32_t imm6 = i * 8;  // imm6 = 0, 8, 16, 24
         uint32_t real_base = base_address + imm6 * 256;
         
         tl_ld_top(base_address, imm6);
         
         printf("  Load %u: imm6=%u, base_addr=0x%08X, addresses=[", i+1, imm6, real_base);
         for (int j = 0; j < 8; j++) {
             printf("0x%08X", real_base + strides[j]);
             if (j < 7) printf(", ");
         }
         printf("]\n");
     }
     
     uint32_t end_cycle = read_cycle();
     uint32_t end_instret = read_instret();
     
     printf("Performance: %u cycles, %u instructions\n", 
            end_cycle - start_cycle, end_instret - start_instret);
     printf("✓ Test 3 completed\n");
 }
 
 void test_4_partial_mask_four_loads(uint32_t base_address) {
     printf("\n=== Test 4: tl.ld.top with partial mask (0x55) - 4 consecutive loads ===\n");
     printf("base=0x%08X, mask=0x55 (loads 0,2,4,6 enabled per instruction)\n", base_address);
     
     // Configure strides for 8 split loads (same as test 2)
     uint32_t strides[8] = {
         0x00000000,  // stride[0] = 0
         0x00000100,  // stride[1] = 256
         0x00000200,  // stride[2] = 512
         0x00000300,  // stride[3] = 768
         0x00000400,  // stride[4] = 1024
         0x00000500,  // stride[5] = 1280
         0x00000600,  // stride[6] = 1536
         0x00000700   // stride[7] = 1792
     };
     
     // Configure CSRs with partial mask (same as test 2)
     CSR_WRITE(CSR_TOP_TTYPE, 0x0841);
     CSR_WRITE(CSR_TOP_TMASK_LD, 0x00000055);
     CSR_WRITE(CSR_TOP_TWIDTH_LD, 256);
     configure_strides(strides);
     
     // Performance measurement
     uint32_t start_cycle = read_cycle();
     uint32_t start_instret = read_instret();
     
     // Execute 4 consecutive tl.ld.top instructions with different imm6 values
     printf("Issuing 4 consecutive tl.ld.top instructions...\n");
     for (uint32_t i = 0; i < 4; i++) {
         uint32_t imm6 = i * 8;  // imm6 = 0, 8, 16, 24
         uint32_t real_base = base_address + imm6 * 256;
         
         tl_ld_top(base_address, imm6);
         
         printf("  Load %u: imm6=%u, base_addr=0x%08X, addresses=[", i+1, imm6, real_base);
         for (int j = 0; j < 8; j++) {
             if (j > 0) printf(", ");
             if ((0x55 >> j) & 1) {
                 printf("0x%08X", real_base + strides[j]);
             } else {
                 printf("MASKED");
             }
         }
         printf("]\n");
     }
     
     uint32_t end_cycle = read_cycle();
     uint32_t end_instret = read_instret();
     
     printf("Performance: %u cycles, %u instructions\n", 
            end_cycle - start_cycle, end_instret - start_instret);
     printf("✓ Test 4 completed\n");
 }
 
 // ============================================================================
 // Main Function
 // ============================================================================
 
 int main(void) {
     printf("======================================================================\n");
     printf("TensorComp tl.ld.top ISA Tests (C version)\n");
     printf("======================================================================\n");
     printf("\nInstruction Encoding:\n");
     printf("  - Opcode: 0x5B (CUSTOM-2)\n");
     printf("  - Eng_ID: 01 (TensorComp)\n");
     printf("  - mode3:  010 (Top Reg Load)\n");
     printf("  - Full encoding: 0x420282DB (with rs1=x5)\n");
     printf("\nConfiguration:\n");
     printf("  - top_ttype: 0x0841 (tsew=1/8-bit, tlmul=1024)\n");
     printf("  - top_twidth_ld: 256 bytes per load\n");
     printf("  - Number of tests: 4\n");
     printf("\nTest Summary:\n");
     printf("  - Test 1: Full mask (0xFF), single load\n");
     printf("  - Test 2: Partial mask (0x55), single load\n");
     printf("  - Test 3: Full mask (0xFF), 4 consecutive loads\n");
     printf("  - Test 4: Partial mask (0x55), 4 consecutive loads\n");
     printf("\nCSR Address Mapping (TensorComp Top Reg):\n");
     printf("  - top_ttype:       0x%03X\n", CSR_TOP_TTYPE);
     printf("  - top_tmask_ld:    0x%03X\n", CSR_TOP_TMASK_LD);
     printf("  - top_twidth_ld:   0x%03X\n", CSR_TOP_TWIDTH_LD);
     printf("  - top_tstride_ld:  0x%03X-0x%03X\n", 
            CSR_TOP_TSTRIDE_LD_BASE, CSR_TOP_TSTRIDE_LD_BASE + 31);
     printf("\n");
     
     uint32_t base_address = 0x40000000;
     // Run all 4 tests
     test_1_full_mask_single_load(base_address);
     test_2_partial_mask_single_load(base_address);
     test_3_full_mask_four_loads(base_address);
     test_4_partial_mask_four_loads(base_address);
     
     printf("\n======================================================================\n");
     printf("All tests completed!\n");
     printf("======================================================================\n");
     
     return 0;
 }
 
 