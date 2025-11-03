# TensorComp tl.ld.top ISA 测试

## 📋 测试规格

### 配置参数
- **top_ttype**: 0x0841 (tsew=1 即 8-bit, tlmul=1024)
- **top_tmask_ld**: 0xFF (只低 8-bit 有效)
- **top_twidth_ld**: 256 字节
- **top_tstride_ld[0:7]**: 8 个 stride 值，间距至少 256*8 字节

### CSR 地址映射（新规范）
- top_ttype: 0x860
- top_tmask_ld: 0x861
- top_twidth_ld: 0x863
- top_tstride_ld[0:31]: 0x880-0x89F

### 测试配置

| 测试 | Stride 间距 | Stride 值 (十进制) |
|------|------------|-------------------|
| Test 1 | 2048 字节 | [0, 2048, 4096, 6144, 8192, 10240, 12288, 14336] |
| Test 2 | 4096 字节 | [0, 4096, 8192, 12288, 16384, 20480, 24576, 28672] |
| Test 3 | 8192 字节 | [0, 8192, 16384, 24576, 32768, 40960, 49152, 57344] |
| Test 4 | 16384 字节 | [0, 16384, 32768, 49152, 65536, 81920, 98304, 114688] |

## 🔧 编译

```bash
cd /Users/minl/Workspace/BYD/NPU/SystemCModel/riscv_model

# 设置环境
source ~/.bash_profile

# 编译
./scripts/build_cfile_bare.sh \
    ./tests/isa-test/tensorcomp/tl_ld_top_tests.c \
    tl_ld_top_tests_c
```

## 🚀 运行

```bash
# 使用 HOST_SIM 模式（推荐）
HOST_SIM=1 HOST_ECHO=1 ./build/riscv_systemc_sim \
    tests/misc/tl_ld_top_tests_c.riscv

# 或使用 UART 模式（需要在模型中实现 UART）
./build/riscv_systemc_sim tests/misc/tl_ld_top_tests_c.riscv
```

## 📊 预期输出

```
======================================================================
TensorComp tl.ld.top ISA Tests (C version)
======================================================================

Configuration:
  - top_ttype: 0x0841 (tsew=1, tlmul=1024)
  - top_tmask_ld: 0xFF (first 8 loads enabled)
  - top_twidth_ld: 256 bytes
  - Number of tests: 4

CSR Address Mapping:
  - top_ttype:       0x860
  - top_tmask_ld:    0x861
  - top_twidth_ld:   0x863
  - top_tstride_ld:  0x880-0x89F

=== Test 1: tl.ld.top with stride spacing of 2048 bytes ===
Stride values: [0, 2048, 4096, 6144, 8192, 10240, 12288, 14336]
Performance: XXX cycles, XXX instructions
✓ Test 1 completed

=== Test 2: tl.ld.top with stride spacing of 4096 bytes ===
...

======================================================================
All tests completed!
======================================================================
```

## ⚙️ 数据初始化说明

**重要**: 测试数据数组需要在运行测试前由外部初始化。

测试代码定义了以下数据数组：
```c
uint8_t test_data_1[8 * 2048];   // 16384 字节
uint8_t test_data_2[8 * 4096];   // 32768 字节
uint8_t test_data_3[8 * 8192];   // 65536 字节
uint8_t test_data_4[8 * 16384];  // 131072 字节
```

### 初始化方式（外部）

您可以通过以下方式初始化：

1. **ELF 加载器预加载数据**
2. **DMA 传输**
3. **模拟器内存注入**
4. **启动代码初始化**

### 建议的数据模式

每个测试数据应包含 8 个块，每块 256 字节：
```
test_data_X:
  Block 0 @ offset 0
  Block 1 @ offset stride_spacing
  Block 2 @ offset 2*stride_spacing
  ...
  Block 7 @ offset 7*stride_spacing
```

## 📝 文件说明

- **tl_ld_top_tests.c**: ISA 测试源代码
- **README.md**: 本文档

## ✅ 验证要点

### 编译检查
- ✅ 无压缩指令 (rv32im)
- ✅ 4 个 tl.ld.top 指令
- ✅ CSR 配置正确

### 运行检查
- CSR 配置是否生效
- tl.ld.top 指令是否正确执行
- Top Register 是否正确加载数据

---

**生成时间**: 2025-10-20  
**状态**: ✅ 已验证，可用于测试
