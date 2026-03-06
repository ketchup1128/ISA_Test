#!/usr/bin/env python3
"""
Split mem_init.hex into 16 files, each containing 128-byte chunks in round-robin fashion.
- Bytes 1-128   -> init_mem_0.hex
- Bytes 129-256 -> init_mem_1.hex
- ...
- Cycles through 16 files
"""

import os
import sys

INPUT_FILE = "mem_init.hex"
NUM_FILES = 16
BYTES_PER_CHUNK = 128  # 128 bytes per chunk
BYTES_PER_LINE = 4     # Each line is 4 bytes (32-bit hex)
LINES_PER_CHUNK = BYTES_PER_CHUNK // BYTES_PER_LINE  # 32 lines


def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    input_path = os.path.join(script_dir, INPUT_FILE)

    if not os.path.exists(input_path):
        print(f"Error: {INPUT_FILE} not found")
        sys.exit(1)

    # Read all data lines (skip address lines starting with @)
    data_lines = []
    with open(input_path, 'r') as f:
        for line in f:
            line = line.strip()
            if line and not line.startswith('@'):
                data_lines.append(line)

    # Open 16 output files
    output_files = []
    for i in range(NUM_FILES):
        output_path = os.path.join(script_dir, f"init_mem_{i}.hex")
        output_files.append(open(output_path, 'w'))

    try:
        # Distribute lines in round-robin fashion
        # Every 32 lines (128 bytes) go to the next file, cycling through 16 files
        for line_idx, line in enumerate(data_lines):
            file_idx = (line_idx // LINES_PER_CHUNK) % NUM_FILES
            output_files[file_idx].write(line + '\n')

        # Report statistics
        total_bytes = len(data_lines) * BYTES_PER_LINE
        print(f"Split {INPUT_FILE} ({total_bytes} bytes, {len(data_lines)} lines) into {NUM_FILES} files:")
        for i, f in enumerate(output_files):
            f.seek(0, 2)  # Seek to end
            size = f.tell()
            print(f"  init_mem_{i}.hex: {size} bytes")

    finally:
        for f in output_files:
            f.close()


if __name__ == "__main__":
    main()
