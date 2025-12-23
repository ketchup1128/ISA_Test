/*
 * TensorComp t.imm.broadcast ISA Test
 * =====================================
 * 
 * Instruction: t.imm.broadcast - Broadcast immediate to all bytes of TReg
 * Syntax: t.imm.broadcast trd, imm10(rs1)
 * 
 * Function:
 *   imm8 = imm10[7:0]  // Extract 8-bit immediate
 *   for i in range(1024):
 *     trd[i] = imm8
 */

#include <stdint.h>
#include "tc_common.h"

#ifdef PRINTF_SUPPORT
#include "printf.h"
#define PRINT(...) printf(__VA_ARGS__)
#else
#define PRINT(...)
#endif

// ============================================================================
// Test Cases
// ============================================================================

void test_1_broadcast_zero(void) {
    PRINT("\n=== Test 1: t.imm.broadcast with value 0x00 ===\n");
    
    PRINT("Broadcasting 0x00 to TReg t0\n");
    
    uint32_t dummy = 0;
    // t.imm.broadcast t0, 0x00(x0)
    T_IMM_BROADCAST(0, 0x00, dummy);
    
    PRINT("Test 1 completed\n");
}

void test_2_broadcast_0xFF(void) {
    PRINT("\n=== Test 2: t.imm.broadcast with value 0xFF ===\n");
    
    PRINT("Broadcasting 0xFF to TReg t1\n");
    
    uint32_t dummy = 0;
    // t.imm.broadcast t1, 0xFF(x0)
    T_IMM_BROADCAST(1, 0xFF, dummy);
    
    PRINT("Test 2 completed\n");
}

void test_3_broadcast_0x55(void) {
    PRINT("\n=== Test 3: t.imm.broadcast with value 0x55 ===\n");
    
    PRINT("Broadcasting 0x55 to TReg t2\n");
    
    uint32_t dummy = 0;
    // t.imm.broadcast t2, 0x55(x0)
    T_IMM_BROADCAST(2, 0x55, dummy);
    
    PRINT("Test 3 completed\n");
}

void test_4_broadcast_0xAA(void) {
    PRINT("\n=== Test 4: t.imm.broadcast with value 0xAA ===\n");
    
    PRINT("Broadcasting 0xAA to TReg t3\n");
    
    uint32_t dummy = 0;
    // t.imm.broadcast t3, 0xAA(x0)
    T_IMM_BROADCAST(3, 0xAA, dummy);
    
    PRINT("Test 4 completed\n");
}

void test_5_broadcast_0x42(void) {
    PRINT("\n=== Test 5: t.imm.broadcast with value 0x42 ===\n");
    
    PRINT("Broadcasting 0x42 to TReg t4\n");
    
    uint32_t dummy = 0;
    // t.imm.broadcast t4, 0x42(x0)
    T_IMM_BROADCAST(4, 0x42, dummy);
    
    PRINT("Test 5 completed\n");
}


// ============================================================================
// Main
// ============================================================================

int main(void) {
    PRINT("======================================================================\n");
    PRINT("TensorComp t.imm.broadcast ISA Test\n");
    PRINT("======================================================================\n");
    PRINT("\nInstruction: t.imm.broadcast - Broadcast 8-bit immediate to TReg\n");
    PRINT("Syntax: t.imm.broadcast trd, imm10(rs1)\n");
    PRINT("\nFunction: Fill all 1024 bytes of TReg with 8-bit immediate\n");
    
    // Run tests
    test_1_broadcast_zero();
    test_2_broadcast_0xFF();
    test_3_broadcast_0x55();
    test_4_broadcast_0xAA();
    test_5_broadcast_0x42();
    
    PRINT("\n======================================================================\n");
    PRINT("All t.imm.broadcast tests completed!\n");
    PRINT("======================================================================\n");
    
    return 0;
}
