/*
 * TensorComp ISA Test Common Definitions
 * ========================================
 * 
 * This file contains common CSR addresses, instruction inline assembly macros,
 * and helper functions for TensorComp ISA tests.
 * 
 * Assembly syntax is derived from:
 *   RISCVInstrInfoTC.td (LLVM TableGen definition)
 */
//  1-40	mma_data_init()	5120 字节
//  41-56	merge_1b_data_init()	2048 字节
//  57-72	merge_128b_data_init()	2048 字节
//  73-88	xpose_data_init()	2048 字节
//  89-512	填充 0	剩余部分

// offset = 0: 4x_left
// offset = 0x400: 2x_left
// offset = 0x800: 1x_left
// offset = 0xC00: 2x_top
// offset = 0x1000: spa   
// offset = 0x1400: mer_1b_0
// offset = 0x1800: mer_1b_1
// offset = 0x1c00: mer_128b_0
// offset = 0x2000: mer_128b_1
// offset = 0x2400: xpose_0
// offset = 0x2800: xpose_1



#ifndef TC_COMMON_H
#define TC_COMMON_H

#include <stdint.h>

// ============================================================================
// CSR Address Definitions (from TensorComp-ISA-Spec.md)
// ============================================================================

// Load/Store Configuration CSRs
#define CSR_TMASK_LD            0x800   // Load mask (8-bit)
#define CSR_TMASK_ST            0x801   // Store mask (8-bit)
#define CSR_TWIDTH_LD           0x802   // Load width (stride)
#define CSR_TWIDTH_ST           0x803   // Store width (stride)

// General Configuration CSRs
#define CSR_TWIDTH              0x804   // Computation width
#define CSR_TLENGTH             0x805   // {spa_len[5:0], top_len[5:0], left_len[5:0]}
#define CSR_TENSOR_CORE_MODE    0x806   // TC mode configuration
#define CSR_QUANT_SHIFT         0x807   // Quantization shift config

// Offset CSRs (32 each for load and store)
#define CSR_TOFFSET_LD_BASE     0x820   // toffset_ld[0] through toffset_ld[31]
#define CSR_TOFFSET_ST_BASE     0x840   // toffset_st[0] through toffset_st[31]

// ============================================================================
// Tensor Core Mode Configuration
// ============================================================================

// Mode bits
#define TC_MODE_NORMAL          0x00    // Normal mode
#define TC_MODE_SPARSE          0x01    // Sparse mode (4:2 structured sparsity)

// Parallel configuration (for left and top)
#define TC_PARAL_1X             0x00    // 1x parallelism
#define TC_PARAL_2X             0x01    // 2x parallelism
#define TC_PARAL_4X             0x02    // 4x parallelism

// Shift amounts for tensor_core_mode CSR
#define TC_LEFT_PARAL_SHIFT     4
#define TC_TOP_PARAL_SHIFT      8



#define L1M_BASE_ADDR 0x40000000
#define ACC_BUFFER_ADDR 0x50000000

// ============================================================================
// CSR Access Macros
// ============================================================================

// Write to CSR
#define CSR_WRITE(csr, value) do { \
    register uint32_t _val = (uint32_t)(value); \
    __asm__ volatile ("csrw %0, %1" :: "i"(csr), "r"(_val)); \
} while(0)

// Read from CSR
#define CSR_READ(csr, dest) \
    __asm__ volatile ("csrr %0, %1" : "=r"(dest) : "i"(csr))

// ============================================================================
// Helper Macros
// ============================================================================

// Make tlength CSR value: {spa_len[5:0], top_len[5:0], left_len[5:0]}
#define MAKE_TLENGTH(left_len, top_len, spa_len) \
    ((((spa_len) & 0xFF) << 16) | (((top_len) & 0xFF) << 8) | ((left_len) & 0xFF))

// Make rs2 config for COMP STORE: {startpoint_spa[23:16], startpoint_top[15:8], startpoint_left[7:0]}
#define MAKE_RS2(left_start, top_start, spa_start) \
    ((((spa_start) & 0xFF) << 16) | (((top_start) & 0xFF) << 8) | ((left_start) & 0xFF))


#define MAKE_QUANT_RS2(left_start, left_len, top_start, top_len) \
((((left_len) & 0xFF) << 8) | ((left_start) & 0xFF) | (((top_len) & 0xFF) << 24) | (((top_start) & 0xFF) << 16))

// Quant shift configuration
#define QUANT_SHIFT_LEFT(amount)  ((amount) & 0x7F)
#define QUANT_SHIFT_RIGHT(amount) (((amount) & 0x7F) | 0x80)

// ============================================================================
// TC Register Names for inline assembly
// ============================================================================
// TC registers are t0-t15 (or tr0-tr15)
// In LLVM, they're referred to as TCR register class

// ============================================================================
// Instruction Inline Assembly Macros
// Using registered mnemonics from RISCVInstrInfoTC.td
// ============================================================================

// ---------------------------------------------------------------------------
// t.ld: Load from L0M to TReg
// Syntax: t.ld trd, imm10(rs1)
// ---------------------------------------------------------------------------
#define T_LD(trd, imm10, rs1_reg) \
    __asm__ volatile ("t.ld tr" #trd ", %0(%1)" :: "i"(imm10), "r"(rs1_reg) : "memory")

// ---------------------------------------------------------------------------
// t.st: Store from TReg to L0M
// Syntax: t.st trs1, imm10(rs1)
// ---------------------------------------------------------------------------
#define T_ST(trs1, imm10, rs1_reg) \
    __asm__ volatile ("t.st tr" #trs1 ", %0(%1)" :: "i"(imm10), "r"(rs1_reg) : "memory")

// ---------------------------------------------------------------------------
// t.merge.128B: Merge at 128-byte granularity
// Syntax: t.merge.128B trd, trs1, trs2, rs1
// ---------------------------------------------------------------------------
#define T_MERGE_128B(trd, trs1, trs2, rs1_reg) \
    __asm__ volatile ("t.merge.128B tr" #trd ", tr" #trs1 ", tr" #trs2 ", %0" :: "r"(rs1_reg) : "memory")

// ---------------------------------------------------------------------------
// t.merge.1B: Merge at 1-byte granularity
// Syntax: t.merge.1B trd, trs1, trs2, rs1
// ---------------------------------------------------------------------------
#define T_MERGE_1B(trd, trs1, trs2, rs1_reg) \
    __asm__ volatile ("t.merge.1B tr" #trd ", tr" #trs1 ", tr" #trs2 ", %0" :: "r"(rs1_reg) : "memory")

// ---------------------------------------------------------------------------
// t.xpose.2: Transpose with dim shape = 2
// Syntax: t.xpose.2 trd, trs1
// ---------------------------------------------------------------------------
#define T_XPOSE_2(trd, trs1, rs1_reg) \
    __asm__ volatile ("t.xpose.2 tr" #trd ", tr" #trs1", %0" :: "r"(rs1_reg) : "memory")

// ---------------------------------------------------------------------------
// t.xpose.4: Transpose with dim shape = 4
// Syntax: t.xpose.4 trd, trs1
// ---------------------------------------------------------------------------
#define T_XPOSE_4(trd, trs1, rs1_reg) \
    __asm__ volatile ("t.xpose.4 tr" #trd ", tr" #trs1", %0" :: "r"(rs1_reg) : "memory")

// ---------------------------------------------------------------------------
// t.xpose.8: Transpose with dim shape = 8
// Syntax: t.xpose.8 trd, trs1
// ---------------------------------------------------------------------------
#define T_XPOSE_8(trd, trs1, rs1_reg) \
    __asm__ volatile ("t.xpose.8 tr" #trd ", tr" #trs1", %0" :: "r"(rs1_reg) : "memory")

// ---------------------------------------------------------------------------
// t.imm.broadcast: Broadcast immediate to TReg
// Syntax: t.imm.broadcast trd, imm10(rs1)
// ---------------------------------------------------------------------------
#define T_IMM_BROADCAST(trd, imm10, rs1_reg) \
    __asm__ volatile ("t.imm.broadcast tr" #trd ", %0(%1)" :: "i"(imm10), "r"(rs1_reg) : "memory")

// ---------------------------------------------------------------------------
// tc.st.mma.acc: Matrix Multiply-Accumulate
// Syntax: tc.st.mma.acc rs1, trs1, trs2, rs2
// ---------------------------------------------------------------------------
#define TC_ST_MMA_ACC(rs1_reg, trs1, trs2, rs2_reg) \
    __asm__ volatile ("tc.st.mma.acc %0, tr" #trs1 ", tr" #trs2 ", %1" :: "r"(rs1_reg), "r"(rs2_reg) : "memory")

// ---------------------------------------------------------------------------
// tc.st.mma.pack.acc: Packed Matrix Multiply-Accumulate
// Syntax: tc.st.mma.pack.acc rs1, trs1, trs2, rs2
// ---------------------------------------------------------------------------
#define TC_ST_MMA_PACK_ACC(rs1_reg, trs1, trs2, rs2_reg) \
    __asm__ volatile ("tc.st.mma.pack.acc %0, tr" #trs1 ", tr" #trs2 ", %1" :: "r"(rs1_reg), "r"(rs2_reg) : "memory")

// ---------------------------------------------------------------------------
// tc.st.add: Broadcast Addition
// Syntax: tc.st.add rs1, trs1, trs2, rs2
// ---------------------------------------------------------------------------
#define TC_ST_ADD(rs1_reg, trs1, trs2, rs2_reg) \
    __asm__ volatile ("tc.st.add %0, tr" #trs1 ", tr" #trs2 ", %1" :: "r"(rs1_reg), "r"(rs2_reg) : "memory")

// ---------------------------------------------------------------------------
// tc.st.clean: Clear acc_buffer with zeros
// Syntax: tc.st.clean rs1
// ---------------------------------------------------------------------------
#define TC_ST_CLEAN(rs1_reg) \
    __asm__ volatile ("tc.st.clean %0" :: "r"(rs1_reg) : "memory")

// ---------------------------------------------------------------------------
// tc.st.init: Initialize acc_buffer with data from TReg (broadcast)
// Syntax: tc.st.init rs1, trs1, rs2
// rs2: {size_top[31:24], startpoint_top[23:16], size_left[15:8], startpoint_left[7:0]}
// ---------------------------------------------------------------------------
#define TC_ST_INIT(rs1_reg, trs1, rs2_reg) \
    __asm__ volatile ("tc.st.init %0, tr" #trs1 ", %1" :: "r"(rs1_reg), "r"(rs2_reg) : "memory")

// ---------------------------------------------------------------------------
// tc.quant: Quantization multiplication
// Syntax: tc.quant rs1, trs1, rs2
// ---------------------------------------------------------------------------
#define TC_QUANT(rs1_reg, trs1, rs2_reg) \
    __asm__ volatile ("tc.quant %0, tr" #trs1 ", %1" :: "r"(rs1_reg), "r"(rs2_reg) : "memory")

// ---------------------------------------------------------------------------
// tc.quant.acc: Quantization with shift and accumulate
// Syntax: tc.quant.acc rs1, trs1, rs2
// ---------------------------------------------------------------------------
#define TC_QUANT_ACC(rs1_reg, trs1, rs2_reg) \
    __asm__ volatile ("tc.quant.acc %0, tr" #trs1 ", %1" :: "r"(rs1_reg), "r"(rs2_reg) : "memory")

// ---------------------------------------------------------------------------
// tc.st.quant.shift: Shift quant_reg and store to acc_buffer
// Syntax: tc.st.quant.shift rs1, trs1, rs2
// ---------------------------------------------------------------------------
#define TC_ST_QUANT_SHIFT(rs1_reg, rs2_reg) \
    __asm__ volatile ("tc.st.quant.shift %0, %1" :: "r"(rs1_reg), "r"(rs2_reg) : "memory")

// ---------------------------------------------------------------------------
// tc.st.quant.shift.acc: Shift quant_reg and accumulate to acc_buffer
// Syntax: tc.st.quant.shift.acc rs1, trs1, rs2
// ---------------------------------------------------------------------------
#define TC_ST_QUANT_SHIFT_ACC(rs1_reg, rs2_reg) \
    __asm__ volatile ("tc.st.quant.shift.acc %0, %1" :: "r"(rs1_reg), "r"(rs2_reg) : "memory")

// ============================================================================
// Standard Offset Configuration Functions
// ============================================================================

// Configure standard contiguous offsets for t.ld (0, 128, 256, ...)
static inline void configure_standard_ld_offsets(void) {
    CSR_WRITE(CSR_TOFFSET_LD_BASE + 0, 0);
    CSR_WRITE(CSR_TOFFSET_LD_BASE + 1, 128);
    CSR_WRITE(CSR_TOFFSET_LD_BASE + 2, 256);
    CSR_WRITE(CSR_TOFFSET_LD_BASE + 3, 384);
    CSR_WRITE(CSR_TOFFSET_LD_BASE + 4, 512);
    CSR_WRITE(CSR_TOFFSET_LD_BASE + 5, 640);
    CSR_WRITE(CSR_TOFFSET_LD_BASE + 6, 768);
    CSR_WRITE(CSR_TOFFSET_LD_BASE + 7, 896);
}

// Configure standard contiguous offsets for t.st (0, 128, 256, ...)
static inline void configure_standard_st_offsets(void) {
    CSR_WRITE(CSR_TOFFSET_ST_BASE + 0, 0);
    CSR_WRITE(CSR_TOFFSET_ST_BASE + 1, 128);
    CSR_WRITE(CSR_TOFFSET_ST_BASE + 2, 256);
    CSR_WRITE(CSR_TOFFSET_ST_BASE + 3, 384);
    CSR_WRITE(CSR_TOFFSET_ST_BASE + 4, 512);
    CSR_WRITE(CSR_TOFFSET_ST_BASE + 5, 640);
    CSR_WRITE(CSR_TOFFSET_ST_BASE + 6, 768);
    CSR_WRITE(CSR_TOFFSET_ST_BASE + 7, 896);
}

// Configure custom offsets for t.ld
static inline void configure_toffset_ld_8(uint32_t o0, uint32_t o1, uint32_t o2, uint32_t o3,
                                          uint32_t o4, uint32_t o5, uint32_t o6, uint32_t o7) {
    CSR_WRITE(CSR_TOFFSET_LD_BASE + 0, o0);
    CSR_WRITE(CSR_TOFFSET_LD_BASE + 1, o1);
    CSR_WRITE(CSR_TOFFSET_LD_BASE + 2, o2);
    CSR_WRITE(CSR_TOFFSET_LD_BASE + 3, o3);
    CSR_WRITE(CSR_TOFFSET_LD_BASE + 4, o4);
    CSR_WRITE(CSR_TOFFSET_LD_BASE + 5, o5);
    CSR_WRITE(CSR_TOFFSET_LD_BASE + 6, o6);
    CSR_WRITE(CSR_TOFFSET_LD_BASE + 7, o7);
}

// Configure custom offsets for t.st
static inline void configure_toffset_st_8(uint32_t o0, uint32_t o1, uint32_t o2, uint32_t o3,
                                          uint32_t o4, uint32_t o5, uint32_t o6, uint32_t o7) {
    CSR_WRITE(CSR_TOFFSET_ST_BASE + 0, o0);
    CSR_WRITE(CSR_TOFFSET_ST_BASE + 1, o1);
    CSR_WRITE(CSR_TOFFSET_ST_BASE + 2, o2);
    CSR_WRITE(CSR_TOFFSET_ST_BASE + 3, o3);
    CSR_WRITE(CSR_TOFFSET_ST_BASE + 4, o4);
    CSR_WRITE(CSR_TOFFSET_ST_BASE + 5, o5);
    CSR_WRITE(CSR_TOFFSET_ST_BASE + 6, o6);
    CSR_WRITE(CSR_TOFFSET_ST_BASE + 7, o7);
}

#endif // TC_COMMON_H
