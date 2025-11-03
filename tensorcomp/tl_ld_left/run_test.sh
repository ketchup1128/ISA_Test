#!/bin/bash
# Quick test script for tc.ld.left ISA tests
# Usage: ./run_test.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$SCRIPT_DIR/../../../.."

echo "======================================================================="
echo "Running tc.ld.left ISA Tests"
echo "======================================================================="
echo ""

# Test 1: Compile (if needed)
if [ ! -f "$SCRIPT_DIR/tl_ld_left_tests.riscv" ]; then
    echo "Compiling test program..."
    cd "$ROOT_DIR"
    ./scripts/build_cfile_bare.sh "$SCRIPT_DIR/tl_ld_left_tests.c"
fi

# Test 2: Run simulation with memory initialization
echo "Running simulation..."
cd "$ROOT_DIR"

./scripts/run_elf.sh \
    "$SCRIPT_DIR/tl_ld_left_tests.riscv" \
    --mem-init 0x40000000:32768:tests/isa-test/init_mem.hex

echo ""
echo "======================================================================="
echo "Test completed!"
echo "======================================================================="

