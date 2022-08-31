# TO RUN : refer example caommand below
# python3 transform_data_to_numpy.py --no-of-nodes=64_nodes_85__0.001 --base-path=/home/hansika/gem5_old/gem5/scripts/ --cal-dir-path=calculated_test/ --rawd-dir-path=dummy_raw_data/ --numpy-dir-path=numpy_data_test/
# python3 transform_data_to_numpy.py --no-of-nodes=64_nodes__fft --base-path=/home/hansika/gem5_old/gem5/scripts/ --cal-dir-path=calculated_test/ --rawd-dir-path=dummy_raw_data/ --numpy-dir-path=numpy_data_test/
# python3 transform_data_to_numpy.py --no-of-nodes=64_nodes__FFT --base-path=/export/research27/tornado/hansika/noc_data/ --cal-dir-path=calculated/ --rawd-dir-path=raw_data/ --numpy-dir-path=numpy_data/ --min-max-length=250


import glob
import os
import numpy as np
import random
import argparse

NUMBER_OF_NODES = "16_nodes__FMM_FFT_14"
BASE_PATH = "/home/hansika/gem5_old/gem5/scripts/"
CALCULATED_DIR_PATH = BASE_PATH + "calculated_test/"
RAW_DATA_DIR_PATH = BASE_PATH + "raw_data/"
NUMPY_DATA_DIR_PATH = BASE_PATH + "numpy_data_test/"

NUM_ITERATIONS_PER_FILE = 100
MIN_MAX_LENGTH = 450
calculate_reduced = True


def create_dir(path):
    if not os.path.exists(path):
        os.makedirs(path)


def remove_prefix(text, prefix):
    if text.startswith(prefix):
        return text[len(prefix):]
    return text


def write_to_file(file, dirr, traffic_count, ni_packet_count, flit_count):
    with open(CALCULATED_DIR_PATH + NUMBER_OF_NODES_OUT + "/" + dirr + "/" + file, 'w') as f:
        f.write("----synthetic traffic generation count---- \n")
        for key, value in traffic_count.items():
            f.write("%s : %s\n" % (key, value))
        f.write("\n")
        f.write("----packet count on network interface---- \n")
        for key, value in ni_packet_count.items():
            f.write("%s : %s\n" % (key, value))
        f.write("\n")
        f.write("----flit count on network link---- \n")
        for key, value in flit_count.items():
            f.write("%s : %s\n" % (key, value))


def is_correlated(src1, mem1, up_key, down_key, no_of_nodes):
    if src1 == up_key and mem1 == down_key - no_of_nodes:
        x = 1
    elif mem1 == up_key and src1 == down_key - no_of_nodes:
        x = 1
    else:
        x = 0
    return x


def convert_to_numpy(up_flit_ipd, down_flit_ipd, src1, mem1, no_of_nodes, numpy_for_dir, correlation_dir):
    for up_key, up_value in up_flit_ipd.items():
        for down_key, down_value in down_flit_ipd.items():
            if up_key != down_key - no_of_nodes:
                correlation = is_correlated(src1, mem1, up_key, down_key, no_of_nodes)
                up_flow = np.array(up_value)
                down_flow = np.array(down_value)
                up_flow.resize(MIN_MAX_LENGTH, refcheck=False)
                down_flow.resize(MIN_MAX_LENGTH, refcheck=False)
                flow_pair = [up_flow, down_flow]
                numpy_for_dir.append(np.array(flow_pair))
                correlation_dir.append(correlation)


def generate_random(exclude_list, up_flit_ipd, down_flit_ipd, no_of_nodes):
    intersect_keys = list(set(down_flit_ipd.keys()) & set(up_flit_ipd.keys()))
    intersect_keys = [e for e in intersect_keys if e not in exclude_list]
    if len(intersect_keys) == 0:
        return None
    return random.choice(intersect_keys)


def get_src2(exclude_list, down_flit_ipd):
    down_flit_keys = list(down_flit_ipd.keys())
    src2_list = [e for e in down_flit_keys if e not in exclude_list]
    if len(src2_list) != 1:
        return None
    return src2_list[0]


def convert_to_numpy_reduced(up_flit_ipd, down_flit_ipd, src1, mem1, src2, mem2, def_mem, no_of_nodes, numpy_for_dir, correlation_dir):
    if down_flit_ipd.get(src1) is not None and up_flit_ipd.get(mem1 + no_of_nodes) is not None:
        convert_to_numpy_local(up_flit_ipd.get(mem1 + no_of_nodes), down_flit_ipd.get(src1), 1, numpy_for_dir,
                               correlation_dir)
        if up_flit_ipd.get(mem2 + no_of_nodes) is not None:
            convert_to_numpy_local(up_flit_ipd.get(mem2 + no_of_nodes), down_flit_ipd.get(src1), 0, numpy_for_dir,
                                   correlation_dir)
        if down_flit_ipd.get(src2) is not None:
            convert_to_numpy_local(up_flit_ipd.get(mem1 + no_of_nodes), down_flit_ipd.get(src2), 0, numpy_for_dir,
                                   correlation_dir)
            if up_flit_ipd.get(def_mem + no_of_nodes) is not None:
                convert_to_numpy_local(up_flit_ipd.get(def_mem + no_of_nodes), down_flit_ipd.get(src2), 0,
                                       numpy_for_dir, correlation_dir)
        elif src1 != 0 and src2 != 0 and down_flit_ipd.get(0) is not None:
            convert_to_numpy_local(up_flit_ipd.get(mem1 + no_of_nodes), down_flit_ipd.get(0), 0, numpy_for_dir,
                                   correlation_dir)
    else:
        print("skipping this combination becuase no other node flow is found")


def convert_to_numpy_local(up_value, down_value, correlation, numpy_for_dir, correlation_dir):
    up_flow = np.array(up_value)
    down_flow = np.array(down_value)
    up_flow.resize(MIN_MAX_LENGTH, refcheck=False)
    down_flow.resize(MIN_MAX_LENGTH, refcheck=False)
    flow_pair = [up_flow, down_flow]
    numpy_for_dir.append(np.array(flow_pair))
    correlation_dir.append(correlation)


def process_traffic_count(frm, to, synthetic_traffic_count):
    key = frm + "->" + to
    if key in synthetic_traffic_count:
        synthetic_traffic_count[key] += 1
    else:
        synthetic_traffic_count[key] = 1


def process_ni_packet_count(frm, to, ni_packet_count, no_of_nodes):
    if int(to) >= no_of_nodes:
        to_node = int(to) - no_of_nodes
    else:
        to_node = int(to)
    key = frm + "->" + to + "(" + str(to_node) + ")"
    if key in ni_packet_count:
        ni_packet_count[key] += 1
    else:
        ni_packet_count[key] = 1


# Example scenario for src=01 and des_mc=15 in 4x4 mesh
# system.ruby.network.ext_links01.network_links0: Upstream: IPD: 5 :no of flits: 1 :vnet: 2 
# system.ruby.network.ext_links31.network_links1: Downstream: IPD: 37 :no of flits: 1 :vnet: 2 
# system.ruby.network.ext_links31.network_links0: Upstream: IPD: 72 :no of flits: 1 :vnet: 2
# system.ruby.network.ext_links01.network_links1: Downstream: IPD: 97 :no of flits: 5 :vnet: 4 

def process_flit_flow(link, stream, ipd, flit_count, up_flit_ipd, down_flit_ipd, no_of_nodes):
    link = remove_prefix(link, "system.ruby.network.ext_links")
    link = link.split(".")[0]
    key = "link_" + link + "_" + stream
    ipd = int(ipd)
    link = int(link)
    if stream == "Upstream":
        if key in flit_count:
            up_flit_ipd[link].append(ipd)
            flit_count[key] += 1
        else:
            up_flit_ipd[link] = [ipd]
            flit_count[key] = 1
    else:
        if key in flit_count:
            down_flit_ipd[link].append(ipd)
            flit_count[key] += 1
        else:
            down_flit_ipd[link] = [ipd]
            flit_count[key] = 1


def process_line(line, ni_packet_count, traffic_count, flit_count, down_flit_ipd, up_flit_ipd, no_of_nodes):
    y = [x.strip() for x in line.split(':')]
    if y[1].startswith("system.cpu"):
        process_traffic_count(y[3], y[5], traffic_count)
    elif y[1].startswith("system.ruby.network.netifs"):
        process_ni_packet_count(y[3], y[5], ni_packet_count, no_of_nodes)
    elif y[1].startswith("system.ruby.network.ext_links"):
        process_flit_flow(y[1], y[2], y[4], flit_count, up_flit_ipd, down_flit_ipd, no_of_nodes)


def process_file(filename, numpy_for_dir, correlation_dir):
    is_file_corrupted = False
    ni_packet_count = {}
    flit_count = {}
    traffic_count = {}
    down_flit_ipd = {}
    up_flit_ipd = {}
    with open(os.path.join(os.getcwd(), filename), 'r') as f:  # open in readonly mode
        fileN = os.path.basename(filename)
        file = fileN.split("_")
        no_of_nodes = int(file[0])
        src1 = int(file[1])
        mem1 = int(file[2])
        src2 = int(file[4])
        mem2 = int(file[5])
        def_mem = int(file[6].split(".")[0])
        print("processing file from " + str(src1) + " to " + str(mem1) + " in " + str(
            no_of_nodes) + " node environment while" + str(src2) + "communicate with" + str(
            mem2) + ", default mem : " + str(def_mem))
        for line in f:
            try:
                process_line(line, ni_packet_count, traffic_count, flit_count, down_flit_ipd, up_flit_ipd,
                             no_of_nodes)
            except:
                is_file_corrupted = True
                break
        if is_file_corrupted:
            print("the file is corrupted")
    if not is_file_corrupted:
        write_to_file(fileN, str(src1) + "_" + str(mem1), traffic_count, ni_packet_count, flit_count)
        if calculate_reduced:
            convert_to_numpy_reduced(up_flit_ipd, down_flit_ipd, src1, mem1, src2, mem2, def_mem, no_of_nodes,
                                     numpy_for_dir, correlation_dir)
        else:
            convert_to_numpy(up_flit_ipd, down_flit_ipd, src1, mem1, no_of_nodes, numpy_for_dir, correlation_dir)


def save_numpy_array(numpy_for_dir, correlation_dir, index):
    np.save(os.path.join(NUMPY_DATA_DIR_PATH + NUMBER_OF_NODES_OUT + "/X", index), np.array(numpy_for_dir))
    np.save(os.path.join(NUMPY_DATA_DIR_PATH + NUMBER_OF_NODES_OUT + "/Y", index), np.array(correlation_dir))


parser = argparse.ArgumentParser()

parser.add_argument('--no-of-nodes', help='Number of nodes eg: 64_nodes_100_c')
parser.add_argument('--base-path', help='base path of the data')
parser.add_argument('--cal-dir-path', help='calculated dir path')
parser.add_argument('--rawd-dir-path', help='raw data dir path')
parser.add_argument('--numpy-dir-path', help='numpy dir path')
parser.add_argument('--min-max-length', help='min max length of the IFD array')

args = parser.parse_args()

if args.no_of_nodes is not None:
    NUMBER_OF_NODES = args.no_of_nodes
if args.base_path is not None:
    BASE_PATH = args.base_path
if args.cal_dir_path is not None:
    CALCULATED_DIR_PATH = BASE_PATH + args.cal_dir_path
if args.rawd_dir_path is not None:
    RAW_DATA_DIR_PATH = BASE_PATH + args.rawd_dir_path
if args.numpy_dir_path is not None:
    NUMPY_DATA_DIR_PATH = BASE_PATH + args.numpy_dir_path
if args.min_max_length is not None:
    MIN_MAX_LENGTH = int(args.min_max_length)

if calculate_reduced:
    NUMPY_DATA_DIR_PATH = NUMPY_DATA_DIR_PATH[0:-1] + "_reduced/"

NUMBER_OF_NODES_OUT = NUMBER_OF_NODES + "_" + str(MIN_MAX_LENGTH)

list_subdir_with_paths = [f.path for f in os.scandir(RAW_DATA_DIR_PATH + NUMBER_OF_NODES + "/") if f.is_dir()]
print(RAW_DATA_DIR_PATH + NUMBER_OF_NODES + "/")
create_dir(CALCULATED_DIR_PATH + NUMBER_OF_NODES_OUT)
create_dir(NUMPY_DATA_DIR_PATH + NUMBER_OF_NODES_OUT + "/X")
create_dir(NUMPY_DATA_DIR_PATH + NUMBER_OF_NODES_OUT + "/Y")
print("No of sub_dirs : " + str(len(list_subdir_with_paths)))

numpy_for_dir = []
correlation_dir = []
i = 1
j = 0
for sub_dir in list_subdir_with_paths:
    create_dir(CALCULATED_DIR_PATH + NUMBER_OF_NODES_OUT + "/" + os.path.basename(sub_dir))
    for filename in glob.glob(sub_dir + "/[!stats]*.txt"):
        process_file(filename, numpy_for_dir, correlation_dir)
        i += 1
        if i == NUM_ITERATIONS_PER_FILE:
            save_numpy_array(numpy_for_dir, correlation_dir, str(j))
            j += 1
            i = 1
            numpy_for_dir = []
            correlation_dir = []
if len(numpy_for_dir) > 0:
    save_numpy_array(numpy_for_dir, correlation_dir, str(j))
