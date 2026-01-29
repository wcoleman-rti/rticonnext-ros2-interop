#!/usr/bin/env python3
"""Comment out @verbatim annotations from IDL files for CODEGENII-2200 workaround."""

import re
import sys

def comment_out_verbatim_annotations(content):
    """
    Comment out @verbatim annotations from IDL content.
    
    Handles patterns like:
    @verbatim (language="comment", text=
      "Multi-line" "\n"
      "text content")
    
    Each line of the annotation is commented out with //
    Properly handles parentheses inside string literals.
    """
    lines = content.split('\n')
    result = []
    i = 0
    
    while i < len(lines):
        line = lines[i]
        
        # Check if this line contains @verbatim
        if '@verbatim' in line:
            # Find the start of @verbatim and comment out this line
            result.append('// ' + line)
            
            # Check if the opening parenthesis is on this line
            if '(' in line:
                # Count parentheses depth, but ignore those inside strings
                depth = count_parens_outside_strings(line)
                i += 1
                
                # Continue commenting out lines until we close all parentheses
                while i < len(lines) and depth > 0:
                    line = lines[i]
                    depth += count_parens_outside_strings(line)
                    result.append('// ' + line)
                    i += 1
            else:
                i += 1
        else:
            result.append(line)
            i += 1
    
    return '\n'.join(result)

def count_parens_outside_strings(line):
    """
    Count the net parentheses depth change in a line, ignoring parentheses inside strings.
    Returns: (open_count - close_count)
    """
    depth = 0
    in_string = False
    escaped = False
    
    for i, char in enumerate(line):
        if escaped:
            escaped = False
            continue
            
        if char == '\\':
            escaped = True
            continue
            
        if char == '"':
            in_string = not in_string
        elif not in_string:
            if char == '(':
                depth += 1
            elif char == ')':
                depth -= 1
    
    return depth

def main():
    if len(sys.argv) != 3:
        print("Usage: strip_verbatim.py <input_idl> <output_idl>", file=sys.stderr)
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    
    try:
        with open(input_file, 'r', encoding='utf-8') as f:
            content = f.read()
        
        cleaned_content = comment_out_verbatim_annotations(content)
        
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(cleaned_content)
            
    except Exception as e:
        print(f"Error processing IDL file: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == '__main__':
    main()
