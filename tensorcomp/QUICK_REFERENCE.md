# tl.ld.top ISA 测试快速参考

## 📁 文件位置
```
/Users/minl/Workspace/BYD/NPU/SystemCModel/riscv_model/tests/isa-test/tensorcomp/
├── tl_ld_top_tests.c     # ISA 测试源文件
└── README.md             # 详细说明
```

## ⚡ 快速编译

```bash
cd /Users/minl/Workspace/BYD/NPU/SystemCModel/riscv_model
source ~/.bash_profile
./scripts/build_cfile_bare.sh ./tests/isa-test/tensorcomp/tl_ld_top_tests.c tl_ld_top_tests_c
```

**输出**: `tests/misc/tl_ld_top_tests_c.riscv` (98KB)

## 🎯 测试内容

### 4 个测试用例

| # | Stride间距 | 第1个stride | 第8个stride |
|---|-----------|------------|------------|
| 1 | 2048 B | 0 | 14336 |
| 2 | 4096 B | 0 | 28672 |
| 3 | 8192 B | 0 | 57344 |
| 4 | 16384 B | 0 | 114688 |

### CSR 配置（所有测试共用）

```c
top_ttype (0x860)     = 0x0841  // 8-bit, 1024 elements
top_tmask_ld (0x861)  = 0xFF    // 8 loads enabled
top_twidth_ld (0x863) = 256     // 256 bytes per load
```

## 🔧 内联汇编关键代码

### tl.ld.top 指令
```c
static inline void tl_ld_top(uint32_t base_addr) {
    register uint32_t addr __asm__("x5") = base_addr;
    __asm__ volatile (
        ".insn r 0x5B, 0x0, 0x20, x0, %0, x0"
        : : "r"(addr) : "memory"
    );
}
```

**指令编码**: `0x4002805b`

### CSR 操作
```c
#define CSR_WRITE(csr, value) \
    __asm__ volatile ("csrw %0, %1" :: "i"(csr), "r"((uint32_t)(value)))
```

## 📊 测试数据

### 数组定义
```c
uint8_t test_data_1[16384];   // Test 1: 8 blocks × 2048B spacing
uint8_t test_data_2[32768];   // Test 2: 8 blocks × 4096B spacing
uint8_t test_data_3[65536];   // Test 3: 8 blocks × 8192B spacing
uint8_t test_data_4[131072];  // Test 4: 8 blocks × 16384B spacing
```

### ⚠️ 重要：需要外部初始化

测试数据**不会自动初始化**，需要通过以下方式预加载：
- ELF 加载器
- 内存预设脚本
- DMA 传输
- 模拟器命令

### 建议的数据布局

每个测试需要 8 个数据块，每块 256 字节：

```
test_data_X:
  [Block 0] @ offset 0
  [Block 1] @ offset 1 × stride_spacing
  [Block 2] @ offset 2 × stride_spacing
  ...
  [Block 7] @ offset 7 × stride_spacing
```

## ✅ 编译验证结果

- ✅ **压缩指令**: 0 个
- ✅ **tl.ld.top 指令**: 4 个
- ✅ **代码大小**: 98 KB
- ✅ **架构**: rv32im (无 C 扩展)

## 🚀 运行

```bash
# HOST_SIM 模式（推荐）
HOST_SIM=1 HOST_ECHO=1 ./build/riscv_systemc_sim tests/misc/tl_ld_top_tests_c.riscv
```

## 📝 关键特性

- ✅ 使用新的 CSR 地址映射 (0x860+)
- ✅ 内联汇编实现自定义指令
- ✅ Printf 调试支持
- ✅ 性能计数器集成
- ✅ 无数据初始化代码（外部初始化）
- ✅ 零压缩指令

---

**最后更新**: 2025-10-20  
**状态**: ✅ 就绪


