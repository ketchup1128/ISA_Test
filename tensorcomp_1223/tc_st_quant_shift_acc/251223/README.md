# TensorComp ISA Tests

This directory contains ISA tests for all TensorComp instructions as defined in the TensorComp-ISA-Spec.md document.

Assembly syntax is derived from the LLVM compiler definition: `RISCVInstrInfoTC.td`

## Test Structure

```
tensorcomp/
├── tc_common.h                 # Common definitions and macros
├── README.md                   # This file
├── t_ld/                       # t.ld instruction tests
├── t_st/                       # t.st instruction tests
├── t_merge_128b/               # t.merge.128B instruction tests
├── t_merge_1b/                 # t.merge.1B instruction tests
├── t_xpose/                    # t.xpose.2/4/8 instruction tests
├── t_imm_broadcast/            # t.imm.broadcast instruction tests
├── tc_st_mma_acc/              # tc.st.mma.acc instruction tests
├── tc_st_mma_pack_acc/         # tc.st.mma.pack.acc instruction tests
├── tc_st_add/                  # tc.st.add instruction tests
├── tc_quant/                   # tc.quant instruction tests
├── tc_quant_acc/               # tc.quant.acc instruction tests
├── tc_st_quant_shift/          # tc.st.quant.shift instruction tests
└── tc_st_quant_shift_acc/      # tc.st.quant.shift.acc instruction tests
```

## Instruction Summary with Assembly Syntax

### Load/Store Instructions

| Instruction | Syntax | Status |
|-------------|--------|--------|
| t.ld | `t.ld trd, imm10(rs1)` | ✅ Registered |
| t.st | `t.st trs1, imm10(rs1)` | ✅ Registered |

### Data Movement Instructions

| Instruction | Syntax | Status |
|-------------|--------|--------|
| t.merge.128B | `t.merge.128B trd, trs1, trs2, rs1` | ✅ Registered |
| t.merge.1B | `t.merge.1B trd, trs1, trs2, rs1` | ✅ Registered |
| t.xpose.2 | `t.xpose.2 trd, trs1` | ✅ Registered |
| t.xpose.4 | `t.xpose.4 trd, trs1` | ✅ Registered |
| t.xpose.8 | `t.xpose.8 trd, trs1` | ✅ Registered |
| t.imm.broadcast | `t.imm.broadcast trd, imm10(rs1)` | ✅ Registered |

### Computation + Store Instructions

| Instruction | Syntax | Status |
|-------------|--------|--------|
| tc.st.mma.acc | `tc.st.mma.acc rs1, trs1, trs2, rs2` | ✅ Registered |
| tc.st.mma.pack.acc | `tc.st.mma.pack.acc rs1, trs1, trs2, rs2` | ✅ Registered |
| tc.st.add | `tc.st.add rs1, trs1, trs2, rs2` | ✅ Registered |

### Quantization Instructions

| Instruction | Syntax | Status |
|-------------|--------|--------|
| tc.quant | `tc.quant rs1, trs1, rs2` | ✅ Registered |
| tc.quant.acc | `tc.quant.acc rs1, trs1, rs2` | ✅ Registered |
| tc.st.quant.shift | `tc.st.quant.shift rs1, trs1, rs2` | ✅ Registered |
| tc.st.quant.shift.acc | `tc.st.quant.shift.acc rs1, trs1, rs2` | ✅ Registered |

## CSR Addresses

| CSR Name | Address | Description |
|----------|---------|-------------|
| tmask_ld | 0x800 | Load mask (8-bit) |
| tmask_st | 0x801 | Store mask (8-bit) |
| twidth_ld | 0x802 | Load stride |
| twidth_st | 0x803 | Store stride |
| twidth | 0x804 | Computation width |
| tlength | 0x805 | {spa_len, top_len, left_len} |
| tensor_core_mode | 0x806 | TC mode configuration |
| quant_shift | 0x807 | Quantization shift config |
| toffset_ld[0:31] | 0x820-0x83F | Load offsets |
| toffset_st[0:31] | 0x840-0x85F | Store offsets |

## TC Register Names

TensorComp registers are named `t0` through `t15` (or `tr0` through `tr15`).
In LLVM, they belong to the `TCR` register class.

## Building Tests

Each test can be compiled with the custom LLVM RISC-V toolchain:

```bash
# Example for t_ld test
clang --target=riscv32 -march=rv32i_xtensorcomp -O2 \
    -nostdlib -nostartfiles \
    -T linker.ld -o t_ld_test.elf t_ld/t_ld_test.c

# Generate hex files for simulation
llvm-objcopy -O ihex t_ld_test.elf itcm.hex
```

## Running Tests

Tests can be run with the RISC-V functional model:

```bash
# From the riscv_model build directory
./riscv_model --itcm path/to/itcm.hex --dtcm path/to/dtcm.hex
```

## Test Output

Each test prints its status to the console (when PRINTF_SUPPORT is defined):
- Test name and parameters
- Expected behavior
- Completion status

## Common Header (tc_common.h)

The `tc_common.h` file contains:
- CSR address definitions
- Inline assembly macros for each instruction
- CSR access macros (CSR_WRITE, CSR_READ)
- Helper functions for offset configuration
- Mode and configuration constants

### Inline Assembly Macros

```c
// Load/Store
T_LD(trd, imm10, rs1_reg)
T_ST(trs1, imm10, rs1_reg)

// Data Movement
T_MERGE_128B(trd, trs1, trs2, rs1_reg)
T_MERGE_1B(trd, trs1, trs2, rs1_reg)
T_XPOSE_2(trd, trs1)
T_XPOSE_4(trd, trs1)
T_XPOSE_8(trd, trs1)
T_IMM_BROADCAST(trd, imm10, rs1_reg)

// Computation
TC_ST_MMA_ACC(rs1_reg, trs1, trs2, rs2_reg)
TC_ST_MMA_PACK_ACC(rs1_reg, trs1, trs2, rs2_reg)
TC_ST_ADD(rs1_reg, trs1, trs2, rs2_reg)

// Quantization
TC_QUANT(rs1_reg, trs1, rs2_reg)
TC_QUANT_ACC(rs1_reg, trs1, rs2_reg)
TC_ST_QUANT_SHIFT(rs1_reg, trs1, rs2_reg)
TC_ST_QUANT_SHIFT_ACC(rs1_reg, trs1, rs2_reg)
```

## Instruction Encoding Reference

### funct2 Values
- `00`: Load/Store/DataMove
- `01`: QUANT instructions
- `10`: COMP STORE (MMA, ADD)

### funct3 Values (funct2=00)
- `000`: t.ld
- `001`: t.st
- `010`: t.merge.128B
- `011`: t.merge.1B
- `100`: t.xpose.2/4/8
- `101`: t.imm.broadcast

### funct3 Values (funct2=10)
- `000`: tc.st.mma.acc
- `001`: tc.st.mma.pack.acc
- `010`: tc.st.add

### funct3 Values (funct2=01)
- `011`: tc.quant / tc.quant.acc / tc.st.quant.shift*

### funct5 Values
- `00000`: t.xpose.2 / tc.quant
- `00001`: t.xpose.4 / tc.quant.acc
- `00010`: t.xpose.8 / tc.st.quant.shift
- `00011`: tc.st.quant.shift.acc
