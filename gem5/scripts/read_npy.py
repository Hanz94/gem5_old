import numpy as np
import random



corrupt = np.load('0.npy', allow_pickle=True)

normal = np.load('1.npy', allow_pickle=True)

print("corrupt")
print(corrupt)

print("normal")
print(normal)

def generate_random(exclude_list, up_flit_ipd, down_flit_ipd, no_of_nodes):
    up_flit_ipd_keys = [ (x - no_of_nodes) for x in list(up_flit_ipd.keys())]
    intersect_keys = list(set(down_flit_ipd.keys()) & set(up_flit_ipd_keys))
    intersect_keys = [e for e in intersect_keys if e not in exclude_list]
    if len(intersect_keys) == 0:
        return None
    return random.choice(intersect_keys)

exclude_list = [1,2,15]

down_flit_ipd = {1: [1, 2, 2, 2, 2, 2, 2], 2: [2, 3, 4, 5, 6, 7], 3: [1, 2, 3, 4, 5, 6]}
up_flit_ipd = {17: [1, 2, 17, 17, 2, 2, 2], 18: [2, 3, 4, 18, 6, 7], 23: [1, 2, 23, 4, 5, 6]}


# item_list = set([e for e in list(dict.keys()) if e not in exclude_list])
# rand_values = random.choice(item_list)
# print(rand_values)

x = None

rand_x = generate_random(exclude_list, up_flit_ipd, down_flit_ipd, 16)
arr = np.array(up_flit_ipd.get(rand_x + 16))
arr2 = np.array(down_flit_ipd.get(rand_x))


arr.resize(20, refcheck=False)
arr2.resize(20, refcheck=False)

print(arr)
print(arr2)