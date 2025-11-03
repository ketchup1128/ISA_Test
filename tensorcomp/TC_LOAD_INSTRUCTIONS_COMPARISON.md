# TensorComp Load 指令对比

## 概述

TensorComp 扩展包含三个 Load 指令，用于将数据从 L0M（本地内存）加载到不同的 TensorComp 寄存器。

## 三个 Load 指令

| 指令 | mode3 | 目标寄存器 | 寄存器大小 | CSR 基地址 | 用途 |
|------|-------|----------|-----------|-----------|------|
| **tc.ld.top** | 010 (0x2) | Top Reg | 2048 Byte | 0x860-0x8BF | Top 操作数 |
| **tc.ld.left** | 001 (0x1) | Left Reg | 4096 Byte | 0x800-0x85F | Left 操作数 |
| **tc.ld.spa** | 100 (0x4) | Sparse Reg | 1024 Byte | 0x8C0-0x91F | Sparse 操作数 |

## 详细对比

### 指令编码

所有三个指令共享相同的基本格式，仅 **mode3** 字段不同：

```
| Eng_ID(2) | mop3(3) | st(1) | mode3(3) | imm3(3) | rs1(5) | funct2(2) | imm6(6) | opcode(7) |
|    01     |  000    |  0    |  MODE3   |   ---   |  rs1   |    00     |  imm6   | 1011011   |
```

#### tc.ld.top

```
mode3 = 010
Base encoding: 0x4104005B (with rs1=x8, imm6=0)
Binary: 01_000_0_010_xxx_xxxxx_00_000000_1011011
```

#### tc.ld.left

```
mode3 = 001
Base encoding: 0x4084005B (with rs1=x8, imm6=0)
Binary: 01_000_0_001_xxx_xxxxx_00_000000_1011011
```

#### tc.ld.spa

```
mode3 = 100
Base encoding: 0x4204005B (with rs1=x8, imm6=0)
Binary: 01_000_0_100_xxx_xxxxx_00_000000_1011011
```

### 编码差异示例

| 指令 | imm6=0 | imm6=8 | imm6=16 | imm6=24 |
|------|--------|--------|---------|---------|
| tc.ld.top | 0x4104005B | 0x4104045B | 0x4104085B | 0x41040C5B |
| tc.ld.left | 0x4084005B | 0x4084045B | 0x4084085B | 0x40840C5B |
| tc.ld.spa | 0x4204005B | 0x4204045B | 0x4204085B | 0x42040C5B |

**注意**: bit[25:23] 的不同编码了不同的目标寄存器。

### CSR 地址映射

#### Top Reg CSRs (0x860-0x8BF)

| 地址 | 名称 | 描述 |
|------|------|------|
| 0x860 | top_ttype | Tensor 类型配置 |
| 0x861 | top_tmask_ld | Load mask |
| 0x863 | top_twidth_ld | Load 宽度 |
| 0x880-0x89F | top_tstride_ld[0:31] | Load stride 数组 |

#### Left Reg CSRs (0x800-0x85F)

| 地址 | 名称 | 描述 |
|------|------|------|
| 0x800 | left_ttype | Tensor 类型配置 |
| 0x801 | left_tmask_ld | Load mask |
| 0x803 | left_twidth_ld | Load 宽度 |
| 0x820-0x83F | left_tstride_ld[0:31] | Load stride 数组 |

#### Sparse Reg CSRs (0x8C0-0x91F)

| 地址 | 名称 | 描述 |
|------|------|------|
| 0x8C0 | spa_ttype | Tensor 类型配置 |
| 0x8C1 | spa_tmask_ld | Load mask |
| 0x8C3 | spa_twidth_ld | Load 宽度 |
| 0x8E0-0x8FF | spa_tstride_ld[0:31] | Load stride 数组 |

### 寄存器大小

```
┌─────────────┬──────────────┬─────────────┐
│   Top Reg   │  Left Reg    │ Sparse Reg  │
│  2048 Byte  │  4096 Byte   │ 1024 Byte   │
├─────────────┼──────────────┼─────────────┤
│   ▓▓▓▓▓▓    │  ▓▓▓▓▓▓▓▓▓▓  │    ▓▓▓      │
│   ▓▓▓▓▓▓    │  ▓▓▓▓▓▓▓▓▓▓  │    ▓▓▓      │
└─────────────┴──────────────┴─────────────┘
```

**Left Reg 是最大的寄存器** (4096 Byte = 4KB)

### 寻址模式

所有三个指令使用相同的寻址逻辑：

```c
// 计算基地址
base_addr = rs1 + imm6 * <reg>_twidth_ld

// 对每个 mask 启用的 lane
for i in range(32):
    if (<reg>_tmask_ld[i] == 1):
        addr = base_addr + <reg>_tstride_ld[i]
        <reg>[i * <reg>_twidth_ld +: <reg>_twidth_ld] = MEM[addr +: <reg>_twidth_ld]
    else:
        <reg>[i * <reg>_twidth_ld +: <reg>_twidth_ld] = 0
```

其中 `<reg>` 为 `top`, `left`, 或 `spa`。

## 反汇编示例

使用 `llvm-objdump` 生成的反汇编：

```asm
# tc.ld.top
80000264: 4104005b     	tc.ld.top	0x0(s0)
800003dc: 4104045b     	tc.ld.top	0x8(s0)

# tc.ld.left  
80000264: 4084005b     	tc.ld.left	0x0(s0)
800003dc: 4084045b     	tc.ld.left	0x8(s0)

# tc.ld.spa (待实现)
80000264: 4204005b     	tc.ld.spa	0x0(s0)
800003dc: 4204045b     	tc.ld.spa	0x8(s0)
```

## 测试目录结构

```
tests/isa-test/tensorcomp/
├── tl_ld_top/              # tc.ld.top 测试 ✅
│   ├── tl_ld_top_tests.c
│   ├── tl_ld_top_tests.riscv
│   ├── tl_ld_top_tests.dump
│   ├── itcm.hex
│   ├── dtcm.hex
│   ├── run_test.sh
│   └── README.md
│
├── tl_ld_left/             # tc.ld.left 测试 ✅
│   ├── tl_ld_left_tests.c
│   ├── tl_ld_left_tests.riscv
│   ├── tl_ld_left_tests.dump
│   ├── itcm.hex
│   ├── dtcm.hex
│   ├── run_test.sh
│   └── README.md
│
└── tl_ld_spa/              # tc.ld.spa 测试 🔜
    └── (待创建)
```

## C 代码示例

### tc.ld.top

```c
#define CSR_TOP_TTYPE 0x860
#define CSR_TOP_TMASK_LD 0x861
#define CSR_TOP_TWIDTH_LD 0x863

static inline void tl_ld_top(uint32_t base_addr, uint32_t imm6) {
    __asm__ volatile (
        "tc.ld.top %0(%1)"
        : 
        : "i" (imm6), "r" (base_addr)
        : "memory"
    );
}

// 配置和执行
CSR_WRITE(CSR_TOP_TTYPE, 0x0841);
CSR_WRITE(CSR_TOP_TMASK_LD, 0xFF);
CSR_WRITE(CSR_TOP_TWIDTH_LD, 256);
tl_ld_top(0x40000000, 0);
```

### tc.ld.left

```c
#define CSR_LEFT_TTYPE 0x800
#define CSR_LEFT_TMASK_LD 0x801
#define CSR_LEFT_TWIDTH_LD 0x803

static inline void tl_ld_left(uint32_t base_addr, uint32_t imm6) {
    __asm__ volatile (
        "tc.ld.left %0(%1)"
        : 
        : "i" (imm6), "r" (base_addr)
        : "memory"
    );
}

// 配置和执行
CSR_WRITE(CSR_LEFT_TTYPE, 0x0841);
CSR_WRITE(CSR_LEFT_TMASK_LD, 0xFF);
CSR_WRITE(CSR_LEFT_TWIDTH_LD, 128);
tl_ld_left(0x40000000, 0);
```

## 实现状态

| 指令 | 解码 | 执行 | ISA 测试 | 状态 |
|------|------|------|---------|------|
| tc.ld.top | ✅ | ✅ | ✅ | **完成** |
| tc.ld.left | ✅ | ✅ | ✅ | **完成** |
| tc.ld.spa | ✅ | ✅ | ⏳ | 待测试 |

## 验证清单

### tc.ld.top ✅
- ✅ 指令编码正确 (mode3=010)
- ✅ CSR 地址正确 (0x860-0x8BF)
- ✅ llvm-objdump 正确反汇编
- ✅ 功能测试通过（4个测试用例）
- ✅ 寄存器大小验证 (2048 Byte)

### tc.ld.left ✅
- ✅ 指令编码正确 (mode3=001)
- ✅ CSR 地址正确 (0x800-0x85F)
- ✅ llvm-objdump 正确反汇编
- ✅ 功能测试通过（4个测试用例）
- ✅ 寄存器大小验证 (4096 Byte)

### tc.ld.spa ⏳
- ✅ 指令编码正确 (mode3=100)
- ✅ CSR 地址正确 (0x8C0-0x91F)
- ⏳ llvm-objdump 反汇编（待验证）
- ⏳ 功能测试（待创建）
- ✅ 寄存器大小验证 (1024 Byte)

## 快速测试

```bash
# 测试 tc.ld.top
cd /Users/minl/Workspace/BYD/NPU/SystemCModel/riscv_model
./tests/isa-test/tensorcomp/tl_ld_top/run_test.sh

# 测试 tc.ld.left
./tests/isa-test/tensorcomp/tl_ld_left/run_test.sh

# 测试 tc.ld.spa (待创建)
./tests/isa-test/tensorcomp/tl_ld_spa/run_test.sh
```

## 相关文档

- [TensorComp ISA Spec](/Users/minl/Workspace/BYD/NPU/Arch Spec/ISA/TensorComp/TensorComp-ISA-Spec.md)
- [TensorComp 实现报告](../../../TENSORLOAD_FINAL_REPORT.md)
- [TC 寄存器文档](../../../README_TC_REGISTERS.md)
- [LLVM objdump 集成](../../../docs/llvm_objdump_integration.md)

---

*创建日期: 2025-11-03*
*版本: 1.0*

