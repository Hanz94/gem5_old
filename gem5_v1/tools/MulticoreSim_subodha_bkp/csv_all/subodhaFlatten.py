#!/usr/bin/python3
import csv
import os
from os import listdir
from os.path import isfile, join

base_dir = './subodha/'
temp_file_name = 'temp.csv'

csvfiles = [f for f in listdir(base_dir) if isfile(join(base_dir, f))]
for fin in csvfiles:
    filename = join(base_dir, fin)
    file_new = open(temp_file_name, 'w')
    file_writer = csv.writer(file_new)
    with open(filename, 'r') as csvfile:
        spread = csv.reader(csvfile)
        line_size = 64
        for row in spread:
            if 'bit' in row[0]:
                # update line size
                line_size = row[0].split()[0]
            elif '_' in row[0]:
                # change row[0]
                params = row[0].split('_')
                size1 = params[0][:-2]
                way1 = params[1]
                size2 = params[2][:-2]
                way2 = params[3]
                row[0] = str(int(size1)*1024) + 'B_' \
                    + way1 + 'W_'\
                    + line_size + 'B_'\
                    + str(int(size2)*1024) + 'B_' \
                    + way2 + 'W_' \
                    + line_size + 'B'
                file_writer.writerow(row[:-2])
        # skip otherwise
        file_new.close()
    os.remove(filename)
    os.rename(temp_file_name, filename)

#add comma
#TODO: add comma in previous loop
for fin in csvfiles:
    filename = join(base_dir, fin)
    file_new = open(temp_file_name, 'w')
    with open(filename, 'r') as file_old:
        for lines in file_old:
            file_new.write(lines.rstrip() + ',\n')
    file_new.close()
    os.remove(filename)
    os.rename(temp_file_name, filename)


