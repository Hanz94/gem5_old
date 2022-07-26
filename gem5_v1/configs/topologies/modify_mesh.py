from tempfile import mkstemp
from shutil import move
from os import remove, close

import sys

#print 'Number of arguments:', len(sys.argv), 'arguments.'
print 'Argument List:', str(sys.argv)
input_args = sys.argv[1].split('_')

links_to_remove = []

for i in input_args:
    links_to_remove.append(int(i))


print "modifying topology..."

file_path = "/home/subodha/garnet-gem5/configs/topologies/Mesh_XY.py"

#print links_to_remove

NEW_LINE = "        links_to_remove = " + str(links_to_remove) + "\n"
#print NEW_LINE

#Create temp file
fh, abs_path = mkstemp()
with open(abs_path,'w') as new_file:
    with open(file_path) as old_file:
        for line in old_file:
            if "links_to_remove = " not in line:
                new_file.write(line)
            else:
                new_file.write(NEW_LINE)

close(fh)
#Remove original file
remove(file_path)
#Move new file
move(abs_path, file_path)

print "done"
