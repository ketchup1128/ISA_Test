/*
 * TensorComp t.xpose.2/4/8 ISA Test
 * ==================================
 * 
 * Instructions:
 *   - t.xpose.2 trd, trs1   (dim shape = 2)
 *   - t.xpose.4 trd, trs1   (dim shape = 4)
 *   - t.xpose.8 trd, trs1   (dim shape = 8)
 * 
 * Function:
 *   TReg (1024 bytes) viewed as 10-dimensional tensor: (2,2,2,2,2,2,2,2,2,2)
 *   Swaps dimensions based on the instruction variant
 */

#include <stdint.h>
#include "tc_common.h"

#ifdef PRINTF_SUPPORT
#include "printf.h"
#define PRINT(...) printf(__VA_ARGS__)
#else
#define PRINT(...)
#endif

// Test data area (1024 bytes, aligned)
// Data pattern: each 128-byte block filled with distinguishable pattern
// volatile uint8_t trs1_data[1024] __attribute__((aligned(1024)));
// volatile uint8_t trs2_data[1024] __attribute__((aligned(1024)));

// // ============================================================================
// // Helper: Initialize test data
// // ============================================================================
// void init_test_data(void) {
//     // Fill trs1_data with pattern: each 128-byte block has different value
//     for (int i = 0; i < 64; i++) {
//         for (int j = 0; j < 16; j++) {
//             trs1_data[i * 16 + j] = (i) & 0xFF;
//         }
//     }

//     for (int i = 0; i < 32; i++) {
//         for (int j = 0; j < 32; j++) {
//             trs2_data[i * 32 + j] = (j) & 0xFF;
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
    uint32_t addr_trs1 = L1M_BASE_ADDR + 9*1024;
    PRINT("Loading trs1_data (0x%08X) into tr1\n", addr_trs1);
    T_LD(1, 0, addr_trs1);

    uint32_t addr_trs2 = L1M_BASE_ADDR + 10*1024;
    PRINT("Loading trs2_data (0x%08X) into tr2\n", addr_trs2);
    T_LD(2, 0, addr_trs2);
    
    PRINT("--- TC registers loaded ---\n\n");
}

// ============================================================================
// Test Cases for t.xpose.2
// ============================================================================
void test_xpose2_high_group(void) {
    PRINT("\n=== Test 1: t.xpose.2 basic ===\n");
    
    PRINT("Transpose with dim shape = 2\n");
    
    uint32_t config_1 = 0x00010000;
    PRINT("Config: 0x%08X\n", config_1);
    T_XPOSE_2(0, 1, config_1);

    uint32_t config_2 = 0x00090000;
    PRINT("Config: 0x%08X\n", config_2);
    T_XPOSE_2(0, 1, config_2);
    
    PRINT("Test 1 completed\n");
}

void test_xpose2_low_group(void) {
    PRINT("\n=== Test 2: t.xpose.2 different registers ===\n");
    
    // t.xpose.2 t2, t3
    uint32_t config_1 = 0x00060005;
    PRINT("Config: 0x%08X\n", config_1);
    T_XPOSE_2(0, 2, config_1);


    uint32_t config_2 = 0x00050002;
    PRINT("Config: 0x%08X\n", config_2);
    T_XPOSE_2(0, 2, config_2);

    PRINT("Test 2 completed\n");
}


// ============================================================================
// Test Cases for t.xpose.4
// ============================================================================

void test_xpose4_high_group(void) {
    PRINT("\n=== Test 4: t.xpose.4 basic ===\n");
    
    PRINT("Transpose with dim shape = 4\n");
    
    // t.xpose.4 t0, t1
    uint32_t config_1 = 0x00080006;
    PRINT("Config: 0x%08X\n", config_1);
    T_XPOSE_4(0, 1, config_1);


    uint32_t config_2 = 0x00070000;
    PRINT("Config: 0x%08X\n", config_2);
    T_XPOSE_4(0, 1, config_2);

    PRINT("Test 4 completed\n");
}

void test_xpose4_low_group(void) { 
    PRINT("\n=== Test 5: t.xpose.4 different registers ===\n");
    
    uint32_t config_1 = 0x00040002;
    PRINT("Config: 0x%08X\n", config_1);
    T_XPOSE_4(0, 2, config_1);

    uint32_t config_2 = 0x00050002;
    PRINT("Config: 0x%08X\n", config_2);
    T_XPOSE_4(0, 2, config_2);  

    PRINT("Test 5 completed\n");
}


// ============================================================================
// Test Cases for t.xpose.8
// ============================================================================

void test_xpose8_low_group(void) {
    PRINT("\n=== Test 7: t.xpose.8 basic ===\n");
    
    PRINT("Transpose with dim shape = 8\n");
    
    // t.xpose.8 t0, t1
    uint32_t config_1 = 0x00030000;
    PRINT("Config: 0x%08X\n", config_1);
    T_XPOSE_8(0, 2, config_1);

    PRINT("Test 7 completed\n");
}


// ============================================================================
// Main
// ============================================================================

int main(void) {
    PRINT("======================================================================\n");
    PRINT("TensorComp t.xpose.2/4/8 ISA Test\n");
    PRINT("======================================================================\n");
    PRINT("\nInstructions:\n");
    PRINT("  t.xpose.2 trd, trs1  - Transpose dim shape = 2\n");
    PRINT("  t.xpose.4 trd, trs1  - Transpose dim shape = 4\n");
    PRINT("  t.xpose.8 trd, trs1  - Transpose dim shape = 8\n");
    PRINT("\nTReg viewed as 10D tensor: (2,2,2,2,2,2,2,2,2,2)\n");
    
    // Initialize test data
    // init_test_data();
    
    // Load data into TC registers
    load_tc_registers();
    
    // t.xpose.2 tests
    test_xpose2_high_group();
    test_xpose2_low_group();
    
    // t.xpose.4 tests
    test_xpose4_high_group();
    test_xpose4_low_group();
    
    // t.xpose.8 tests
    test_xpose8_low_group();
    
    PRINT("\n======================================================================\n");
    PRINT("All t.xpose tests completed!\n");
    PRINT("======================================================================\n");
    
    return 0;
}
