#!/usr/bin/env python3
"""
Extract AccBuffer related debug prints from program.out

Usage:
    python3 scripts/extract_accbuffer.py [input_file] [output_file]
    
    Default: input_file = program.out, output_file = accbuffer_output.txt
"""

import sys
import re
import os

def parse_accbuffer_data(section):
    """Parse AccBuffer data from a section and return as list of (row, col, value) tuples
    According to corrected user's description:
    - Header line (line 6) represents column indices (0-31) - 32 columns
    - Data row prefix represents row index (0-127) - 128 rows
    - Data structure: 128 rows × 32 columns = 4096 elements
    """
    data = []
    
    # Pattern to match data row: [TC_DEBUG]     [   0] value0 value1 ...
    data_row_pattern = re.compile(r'\[TC_DEBUG\]\s+\[\s*(\d+)\s*\](.*)')
    
    col_indices = []  # From header line (0-31) - these are column indices
    rows_data = []  # List of (row_idx, values) tuples - row_idx from data row prefix
    
    for line in section:
        # Skip the title line "AccBuffer (base=0x50000000, 4096 elements):"
        if 'AccBuffer (base=' in line:
            continue
        
        # Parse header line to get column indices (0-31)
        # Format: [TC_DEBUG]            [   0  ] [   1  ] [   2  ] ...
        if '[   0  ]' in line or '[   1  ]' in line:
            # Extract column indices from header
            col_matches = re.findall(r'\[\s*(\d+)\s+\]', line)
            if col_matches:
                col_indices = [int(c) for c in col_matches]
            continue
        
        # Parse data rows - first number is row index
        match = data_row_pattern.match(line)
        if match:
            row_idx = int(match.group(1))  # Row index from data row prefix
            values_str = match.group(2)
            # Extract hex values (8 hex digits each)
            values = re.findall(r'([0-9A-Fa-f]{8})', values_str)
            rows_data.append((row_idx, values))
    
    # Build data list: for each row, assign values to columns
    # Format: row (from data row prefix), col (from header), value
    for row_idx, values in rows_data:
        for col_idx, value in enumerate(values):
            # Only process up to the number of columns we found in header
            if col_idx < len(col_indices):
                data.append((row_idx, col_indices[col_idx], value.upper()))
    
    return data


def extract_accbuffer(input_file, output_file):
    """Extract AccBuffer related debug prints from input file"""
    
    if not os.path.exists(input_file):
        print(f"Error: Input file '{input_file}' not found!", file=sys.stderr)
        return 1
    
    print(f"Extracting AccBuffer debug information from {input_file}...")
    print(f"Output will be saved to {output_file}")
    
    # Pattern to match instruction lines: [TC] <instruction> PC=0x<hex> Encoding=0x<hex>
    tc_instruction_pattern = re.compile(r'\[TC\]\s+(\S+)\s+PC=0x([0-9A-Fa-f]+)\s+Encoding=0x([0-9A-Fa-f]+)')
    
    accbuffer_pattern = re.compile(r'\[TC_DEBUG\].*AccBuffer')
    tc_debug_pattern = re.compile(r'^\[TC_DEBUG\]')
    data_row_pattern = re.compile(r'\[TC_DEBUG\]\s+\[\s*\d+\s*\]')
    header_pattern = re.compile(r'\[TC_DEBUG\]\s+\[\s*\d+\s+\]')
    
    # Store sections with their associated instruction info
    accbuffer_sections = []  # List of (instruction_info, section_lines)
    current_section = None
    in_accbuffer_section = False
    current_instruction = None  # (instr_name, pc, encoding)
    section_instruction = None  # Instruction that caused the current AccBuffer section
    
    with open(input_file, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.rstrip('\n\r')
            
            # Check if this is a TC instruction line
            tc_match = tc_instruction_pattern.match(line)
            if tc_match:
                instr_name = tc_match.group(1).strip()
                pc = tc_match.group(2).upper()
                encoding = tc_match.group(3).upper()
                current_instruction = (instr_name, pc, encoding)
                continue
            
            # Check if this is an AccBuffer line (but not "cleared")
            if accbuffer_pattern.search(line) and 'cleared' not in line:
                # Save previous section if exists
                if current_section is not None:
                    accbuffer_sections.append((section_instruction, current_section))
                
                # Start new section with the current instruction (the one that caused this AccBuffer)
                section_instruction = current_instruction
                in_accbuffer_section = True
                current_section = [line]
                continue
            
            # If we're in an AccBuffer section
            if in_accbuffer_section:
                # Continue if it's a TC_DEBUG line that looks like part of AccBuffer dump
                if tc_debug_pattern.match(line):
                    # Check if it's a data row, header, or continuation
                    if (data_row_pattern.search(line) or 
                        header_pattern.search(line) or
                        'AccBuffer' in line):
                        current_section.append(line)
                    else:
                        # Different TC_DEBUG section, end current section
                        if current_section is not None:
                            accbuffer_sections.append((section_instruction, current_section))
                        current_section = None
                        section_instruction = None
                        in_accbuffer_section = False
                else:
                    # Non-TC_DEBUG line, check if it looks like continuation data
                    if re.match(r'^\s*\[\s*\d+\s*\]', line):
                        current_section.append(line)
                    else:
                        # End of section
                        if current_section is not None:
                            accbuffer_sections.append((section_instruction, current_section))
                        current_section = None
                        section_instruction = None
                        in_accbuffer_section = False
    
    # Don't forget the last section
    if current_section is not None:
        accbuffer_sections.append((section_instruction, current_section))
    
    # Write output (only formatted data, no original format)
    with open(output_file, 'w', encoding='utf-8') as f:
        for i, (instruction_info, section) in enumerate(accbuffer_sections, 1):
            # Parse and write formatted data only
            data = parse_accbuffer_data(section)
            if data:
                # Write instruction info header
                if instruction_info:
                    instr_name, pc, encoding = instruction_info
                    f.write(f"# Instruction: {instr_name}, PC=0x{pc}, Encoding=0x{encoding}\n")
                else:
                    f.write(f"# Instruction: (unknown)\n")
                
                # Sort by row, then by column
                sorted_data = sorted(data, key=lambda x: (x[0], x[1]))
                
                for original_row, col, acc_reg in sorted_data:
                    # Convert row: new_row = original_row // 4, acc_row = original_row % 4
                    new_row = original_row // 4
                    acc_row = original_row % 4
                    f.write(f"row = {new_row}, col = {col}, acc_row = {acc_row}, acc_reg = {acc_reg}\n")
                
                # Add blank line between sections
                f.write('\n')
    
    print(f"Extracted {len(accbuffer_sections)} AccBuffer sections to {output_file}")
    return 0


def main():
    # Parse command line arguments
    input_file = sys.argv[1] if len(sys.argv) > 1 else 'program.out'
    output_file = sys.argv[2] if len(sys.argv) > 2 else 'accbuffer_output.txt'
    
    return extract_accbuffer(input_file, output_file)


if __name__ == '__main__':
    sys.exit(main())

