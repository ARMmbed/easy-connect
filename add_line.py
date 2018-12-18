from tempfile import mkstemp
from shutil import move
from os import fdopen, remove
import sys

def add_line(file_path, line_before, line_after, new_line):
    #Create temp file
    fh, abs_path = mkstemp()
    line_before_found = 0
    with fdopen(fh,'w') as new_file:
        with open(file_path) as old_file:
            for line in old_file:
				if line.startswith(line_before):
					line_before_found = 1
					new_file.write(line)
					continue
				elif line_before_found and line.startswith(line_after):
					new_file.write(new_line)
					new_file.write('\n')
				line_before_found = 0
				new_file.write(line)
    #Remove original file
    remove(file_path)
    #Move new file
    move(abs_path, file_path)
	

line_before = '    "MODULE_UBLOX_ODIN_W2": {'
line_after = '        "inherits": ["FAMILY_STM32"],'
new_line = '		"components": ["SD"],'
add_line(sys.argv[1], line_before, line_after, new_line)
