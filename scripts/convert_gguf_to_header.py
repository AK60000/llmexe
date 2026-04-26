#!/usr/bin/env python3
"""
Convert GGUF model file to C header file with embedded data.
Usage: python convert_gguf_to_header.py input.gguf output.h
"""

import sys
import os

def convert_gguf_to_header(input_path, output_path):
    with open(input_path, 'rb') as f:
        data = f.read()
    
    with open(output_path, 'w') as out:
        out.write(f'// Auto-generated from {os.path.basename(input_path)}\n')
        out.write(f'// Size: {len(data)} bytes ({len(data) / 1024 / 1024:.2f} MB)\n\n')
        out.write(f'const unsigned int model_data_size = {len(data)};\n')
        out.write('const unsigned char model_data[] = {\n')
        
        for i, b in enumerate(data):
            if i % 16 == 0:
                out.write('    ')
            out.write(f'0x{b:02x},')
            if i % 16 == 15:
                out.write('\n')
        
        if len(data) % 16 != 0:
            out.write('\n')
        out.write('};\n')
    
    print(f'Converted {len(data)} bytes to {output_path}')

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print(f'Usage: {sys.argv[0]} input.gguf output.h')
        sys.exit(1)
    
    convert_gguf_to_header(sys.argv[1], sys.argv[2])
