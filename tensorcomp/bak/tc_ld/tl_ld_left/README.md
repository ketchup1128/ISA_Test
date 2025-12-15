# TensorComp tc.ld.left ISA Tests

## 概述

本目录包含 `tc.ld.left` 指令的 ISA 测试，用于验证 TensorComp Left Register Load 指令的功能。

## 文件说明

### 源文件
- **tl_ld_left_tests.c** - C 语言测试程序，包含 4 个测试用例
- **run_test.sh** - 快速测试脚本

### 编译输出
- **tl_ld_left_tests.riscv** - ELF 可执行文件
- **tl_ld_left_tests.dump** - 反汇编文件（使用 llvm-objdump）
- **itcm.hex** - 指令内存十六进制格式
- **dtcm.hex** - 数据内存十六进制格式

## 指令规范

### tc.ld.left 编码

```
| Eng_ID(2) | mop3(3) | st(1) | mode3(3) | imm3(3) | rs1(5) | funct2(2) | imm6(6) | opcode(7) |
|    01     |  000    |  0    |   001    |   ---   |  rs1   |    00     |  imm6   | 1011011   |
```

- **Opcode**: 0x5B (CUSTOM-2)
- **Eng_ID**: 01 (TensorComp)
- **mode3**: 001 (Left Reg Load) ← 区别于 tc.ld.top (010)
- **Base encoding**: 0x4084005B (with rs1=x8, imm6=0)

### CSR 地址 (Left Reg)

| 地址 | 名称 | 描述 |
|------|------|------|
| 0x800 | left_ttype | Tensor 类型配置 |
| 0x801 | left_tmask_ld | Load mask (32-bit) |
| 0x803 | left_twidth_ld | Load 宽度 |
| 0x820-0x83F | left_tstride_ld[0:31] | Load stride 数组 |

### Left Reg 规格

- **大小**: 4096 Byte (比 Top Reg 的 2048 Byte 大一倍)
- **用途**: TensorComp 左操作数寄存器

## 测试用例

### Test 1: Full Mask, Single Load
- **配置**: 
  - Mask: 0xFF (8 个 load 使能)
  - Width: 128 bytes
  - Stride: 0x000, 0x080, 0x100, 0x180, 0x200, 0x280, 0x300, 0x380
- **验证**: 单个 tc.ld.left 指令，全 mask

### Test 2: Partial Mask, Single Load
- **配置**:
  - Mask: 0x55 (4 个 load 使能: 0,2,4,6)
  - Width: 128 bytes
  - imm6: 8
- **验证**: 单个 tc.ld.left 指令，部分 mask

### Test 3: Full Mask, Four Consecutive Loads
- **配置**:
  - Mask: 0xFF
  - Width: 128 bytes
  - 4 条连续指令，imm6: 0, 8, 16, 24
- **验证**: 多个连续 tc.ld.left 指令，全 mask

### Test 4: Partial Mask, Four Consecutive Loads
- **配置**:
  - Mask: 0x55
  - Width: 128 bytes
  - 4 条连续指令，imm6: 0, 8, 16, 24
- **验证**: 多个连续 tc.ld.left 指令，部分 mask

## 编译和运行

### 方法 1: 使用快速测试脚本

```bash
cd /Users/minl/Workspace/BYD/NPU/SystemCModel/riscv_model/tests/isa-test/tensorcomp/tl_ld_left
./run_test.sh
```

### 方法 2: 手动编译和运行

```bash
# 编译
cd /Users/minl/Workspace/BYD/NPU/SystemCModel/riscv_model
./scripts/build_cfile_bare.sh tests/isa-test/tensorcomp/tl_ld_left/tl_ld_left_tests.c

# 运行（需要内存初始化）
./scripts/run_elf.sh \
  tests/isa-test/tensorcomp/tl_ld_left/tl_ld_left_tests.riscv \
  --mem-init 0x40000000:32768:tests/isa-test/init_mem.hex
```

### 预期输出

```
[PASS] PASS PC=0x80002a44 cycles=112621 instret=68954 ipc=0.6123
```

## 验证要点

### 1. 指令反汇编

使用 `llvm-objdump` 生成的 dump 文件应正确显示：

```asm
80000264: 4084005b     	tc.ld.left	0x0(s0)
800003dc: 4084045b     	tc.ld.left	0x8(s0)
800004e8: 4084005b     	tc.ld.left	0x0(s0)
```

### 2. 指令编码验证

```
0x4084005B:
  Eng_ID [31:30] = 01 ✓
  mode3 [25:23]  = 001 ✓ (Left Reg)
  opcode [6:0]   = 0x5B ✓
```

### 3. 功能验证

查看 `program.out` 确认：
- ✅ 所有 4 个测试用例执行
- ✅ `[TC-DISP]` 日志显示 mode3=1
- ✅ 地址计算正确：`base_addr = rs1 + imm6 * left_twidth_ld`
- ✅ 程序正常退出（PASS）

## 与 tc.ld.top 的区别

| 特性 | tc.ld.top | tc.ld.left |
|------|-----------|------------|
| mode3 编码 | 010 (0x2) | 001 (0x1) |
| 寄存器大小 | 2048 Byte | 4096 Byte |
| CSR 基地址 | 0x860-0x8BF | 0x800-0x85F |
| 基础编码 | 0x4104005B | 0x4084005B |
| 用途 | Top 操作数 | Left 操作数 |

## 调试选项

### 启用详细跟踪

```bash
# 启用 TC 指令跟踪
TC_DATA_VERIFY=1 ./scripts/run_elf.sh \
  tests/isa-test/tensorcomp/tl_ld_left/tl_ld_left_tests.riscv \
  --mem-init 0x40000000:32768:tests/isa-test/init_mem.hex

# 启用完整跟踪（需要更长超时）
SIM_TIMEOUT_SEC=60 ./scripts/run_elf.sh \
  tests/isa-test/tensorcomp/tl_ld_left/tl_ld_left_tests.riscv \
  --sim-env FULL_TRACE=1 \
  --mem-init 0x40000000:32768:tests/isa-test/init_mem.hex
```

## 相关文档

- [TensorComp ISA Spec](/Users/minl/Workspace/BYD/NPU/Arch Spec/ISA/TensorComp/TensorComp-ISA-Spec.md)
- [TensorComp 寄存器文档](../../../../README_TC_REGISTERS.md)
- [LLVM objdump 集成说明](../../../../docs/llvm_objdump_integration.md)
- [tc.ld.top 测试](../tl_ld_top/)

## 测试状态

✅ **通过** - 所有测试用例正常运行，指令编码正确，功能验证完成

---

*创建日期: 2025-11-03*
*最后更新: 2025-11-03*

