from tempfile import mkstemp
from shutil import move
from os import fdopen, remove
import sys

def replace(file_path, current_line, new_line):
    #Create temp file
    fh, abs_path = mkstemp()
    with fdopen(fh,'w') as new_file:
        with open(file_path) as old_file:
            for line in old_file:
                new_file.write(line.replace(current_line, new_line))
    #Remove original file
    remove(file_path)
    #Move new file
    move(abs_path, file_path)
	

replace(sys.argv[1], sys.argv[2], sys.argv[3])