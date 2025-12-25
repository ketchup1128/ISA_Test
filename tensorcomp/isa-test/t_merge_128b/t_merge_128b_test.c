/*
 * TensorComp t.merge.128B ISA Test
 * =================================
 * 
 * Instruction: t.merge.128B - Merge at 128-byte granularity
 * Syntax: t.merge.128B trd, trs1, trs2, rs1
 * 
 * Function:
 *   rs1 contains merge configuration: 8 x 4-bit fields
 *   Each field = {reg_id(1-bit), index_id(3-bit)}
 *   for j in range(8):
 *     cfg = (rs1 >> (j*4)) & 0xF
 *     reg_id = (cfg >> 3) & 1    // 0=trs1, 1=trs2
 *     index_id = cfg & 7         // which 128-byte block
 *     if (reg_id == 0):
 *       trd[j*128 +: 128] = trs1[index_id*128 +: 128]
 *     else:
 *       trd[j*128 +: 128] = trs2[index_id*128 +: 128]
 */



#include <stdint.h>
#include "tc_common.h"

#ifdef PRINTF_SUPPORT
#include "printf.h"
#define PRINT(...) printf(__VA_ARGS__)
#else
#define PRINT(...)
#endif

// Test data areas (1024 bytes each, aligned)
// Data pattern: trs1_data filled with 0x11, trs2_data filled with 0x22
// volatile uint8_t trs1_data[1024] __attribute__((aligned(1024)));
// volatile uint8_t trs2_data[1024] __attribute__((aligned(1024)));

// // ============================================================================
// // Helper: Initialize test data
// // ============================================================================
// void init_test_data(void) {
//     // Fill trs1_data with pattern 0x11 (distinguishable)
//     for (int i = 0; i < 8; i++) {
//         for (int j = 0; j < 128; j++) {
//             trs1_data[i * 128 + j] = 0x10 | i;
//         }
//     }
    
//     // Fill trs2_data with pattern 0x22 (distinguishable)
//     for (int i = 0; i < 8; i++) {
//         for (int j = 0; j < 128; j++) {
//             trs2_data[i * 128 + j] = 0x20 | i;
//         }
//     }
    
// }

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
    
    // Load trs1_data into tr1
    uint32_t addr_trs1 = L1M_BASE_ADDR + 0x1c00;
    PRINT("Loading trs1_data (0x%08X) into tr1\n", addr_trs1);
    T_LD(1, 0, addr_trs1);
    
    // Load trs2_data into tr2
    uint32_t addr_trs2 = L1M_BASE_ADDR + 0x2000;
    PRINT("Loading trs2_data (0x%08X) into tr2\n", addr_trs2);
    T_LD(2, 0, addr_trs2);
    
    PRINT("--- TC registers loaded ---\n\n");
}

// ============================================================================
// Test Cases
// ============================================================================

void test_1_all_from_trs1(void) {
    PRINT("\n=== Test 1: t.merge.128B all blocks from trs1 ===\n");
    
    // Config: all 8 blocks from trs1[0,1,2,3,4,5,6,7]
    // Each cfg = {reg_id=0, index_id=0..7}
    uint32_t cfg0 = 0b0000;  // trs1[0]
    uint32_t cfg1 = 0b0001;  // trs1[1]
    uint32_t cfg2 = 0b0010;  // trs1[2]
    uint32_t cfg3 = 0b0011;  // trs1[3]
    uint32_t cfg4 = 0b0100;  // trs1[4]
    uint32_t cfg5 = 0b0101;  // trs1[5]
    uint32_t cfg6 = 0b0110;  // trs1[6]
    uint32_t cfg7 = 0b0111;  // trs1[7]
    uint32_t config = (cfg7 << 28) | (cfg6 << 24) | (cfg5 << 20) | (cfg4 << 16) | 
                      (cfg3 << 12) | (cfg2 << 8) | (cfg1 << 4) | cfg0;
    
    PRINT("Config: 0x%08X\n", config);
    for (int blk = 0; blk < 8; ++blk) {
        uint32_t cfg = (config >> (blk*4)) & 0xF;
        uint32_t reg_id = (cfg >> 3) & 0x1;
        uint32_t index_id = cfg & 0x7;
        PRINT("  Block[%d]: reg_id=%u, index_id=%u\n", blk, reg_id, index_id);
    }
    PRINT("Expected: tr0 = copy of tr1\n");
    
    // t.merge.128B tr0, tr1, tr2, config
    T_MERGE_128B(0, 1, 2, config);
    
    PRINT("Test 1 completed\n");
}

void test_2_all_from_trs2(void) {
    PRINT("\n=== Test 2: t.merge.128B all blocks from trs2 ===\n");
    
    // Config: all 8 blocks from trs2[0,1,2,3,4,5,6,7]
    // Each cfg = {reg_id=1, index_id=0..7}
    uint32_t cfg0 = 0b1000;  // trs2[0]
    uint32_t cfg1 = 0b1001;  // trs2[1]
    uint32_t cfg2 = 0b1010;  // trs2[2]
    uint32_t cfg3 = 0b1011;  // trs2[3]
    uint32_t cfg4 = 0b1100;  // trs2[4]
    uint32_t cfg5 = 0b1101;  // trs2[5]
    uint32_t cfg6 = 0b1110;  // trs2[6]
    uint32_t cfg7 = 0b1111;  // trs2[7]
    uint32_t config = (cfg7 << 28) | (cfg6 << 24) | (cfg5 << 20) | (cfg4 << 16) | 
                      (cfg3 << 12) | (cfg2 << 8) | (cfg1 << 4) | cfg0;
    
    PRINT("Config: 0x%08X\n", config);
    for (int blk = 0; blk < 8; ++blk) {
        uint32_t cfg = (config >> (blk*4)) & 0xF;
        uint32_t reg_id = (cfg >> 3) & 0x1;
        uint32_t index_id = cfg & 0x7;
        PRINT("  Block[%d]: reg_id=%u, index_id=%u\n", blk, reg_id, index_id);
    }
    PRINT("Expected: tr0 = copy of tr2\n");
    
    T_MERGE_128B(0, 1, 2, config);
    
    PRINT("Test 2 completed\n");
}

void test_3_interleaved(void) {
    PRINT("\n=== Test 3: t.merge.128B interleaved blocks ===\n");
    
    // Config: alternate between trs1 and trs2
    // block 0: trs1[0], block 1: trs2[0], block 2: trs1[1], block 3: trs2[1], ...
    uint32_t cfg0 = 0b0000;  // trs1[0]
    uint32_t cfg1 = 0b1000;  // trs2[0]
    uint32_t cfg2 = 0b0001;  // trs1[1]
    uint32_t cfg3 = 0b1001;  // trs2[1]
    uint32_t cfg4 = 0b0010;  // trs1[2]
    uint32_t cfg5 = 0b1010;  // trs2[2]
    uint32_t cfg6 = 0b0011;  // trs1[3]
    uint32_t cfg7 = 0b1011;  // trs2[3]
    uint32_t config = (cfg7 << 28) | (cfg6 << 24) | (cfg5 << 20) | (cfg4 << 16) | 
                      (cfg3 << 12) | (cfg2 << 8) | (cfg1 << 4) | cfg0;
    
    PRINT("Config: 0x%08X\n", config);
    for (int blk = 0; blk < 8; ++blk) {
        uint32_t cfg = (config >> (blk*4)) & 0xF;
        uint32_t reg_id = (cfg >> 3) & 0x1;
        uint32_t index_id = cfg & 0x7;
        PRINT("  Block[%d]: reg_id=%u, index_id=%u\n", blk, reg_id, index_id);
    }
    PRINT("Expected: Interleaved blocks from tr1 and tr2\n");
    
    T_MERGE_128B(0, 1, 2, config);
    
    PRINT("Test 3 completed\n");
}

void test_4_reverse_order(void) {
    PRINT("\n=== Test 4: t.merge.128B reverse order ===\n");
    
    // Config: trs1 blocks in reverse order
    uint32_t cfg0 = 0b1111;  // trs1[7]
    uint32_t cfg1 = 0b1110;  // trs1[6]
    uint32_t cfg2 = 0b1101;  // trs1[5]
    uint32_t cfg3 = 0b1100;  // trs1[4]
    uint32_t cfg4 = 0b0011;  // trs1[3]
    uint32_t cfg5 = 0b0010;  // trs1[2]
    uint32_t cfg6 = 0b0001;  // trs1[1]
    uint32_t cfg7 = 0b0000;  // trs1[0]
    uint32_t config = (cfg7 << 28) | (cfg6 << 24) | (cfg5 << 20) | (cfg4 << 16) | 
                      (cfg3 << 12) | (cfg2 << 8) | (cfg1 << 4) | cfg0;
    
    PRINT("Config: 0x%08X\n", config);
    for (int blk = 0; blk < 8; ++blk) {
        uint32_t cfg = (config >> (blk*4)) & 0xF;
        uint32_t reg_id = (cfg >> 3) & 0x1;
        uint32_t index_id = cfg & 0x7;
        PRINT("  Block[%d]: reg_id=%u, index_id=%u\n", blk, reg_id, index_id);
    }
    PRINT("Expected: tr0 = reversed tr1\n");
    
    T_MERGE_128B(0, 1, 2, config);
    
    PRINT("Test 4 completed\n");
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
    PRINT("======================================================================\n");
    PRINT("TensorComp t.merge.128B ISA Test\n");
    PRINT("======================================================================\n");
    PRINT("\nInstruction: t.merge.128B - Merge at 128-byte granularity\n");
    PRINT("Syntax: t.merge.128B trd, trs1, trs2, rs1\n");
    PRINT("\nConfig format (rs1): 8 x 4-bit fields\n");
    PRINT("  Each field: {reg_id(1), index_id(3)}\n");
    PRINT("  reg_id: 0=trs1, 1=trs2\n");
    PRINT("  index_id: which 128-byte block (0-7)\n");
    
    // Initialize test data
    // init_test_data();
    
    // Load data into TC registers
    load_tc_registers();
    
    // Run tests
    test_1_all_from_trs1();
    test_2_all_from_trs2();
    test_3_interleaved();
    test_4_reverse_order();
    
    PRINT("\n======================================================================\n");
    PRINT("All t.merge.128B tests completed!\n");
    PRINT("======================================================================\n");
    
    return 0;
}
