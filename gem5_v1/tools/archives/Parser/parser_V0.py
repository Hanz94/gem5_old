from xml.etree.ElementTree import ElementTree
from xml.etree.ElementTree import Element
from xml.etree.ElementTree import parse
from xml.etree.ElementTree import *
import xml.etree.ElementTree as ET

import os
import shutil

import re

shutil.copy2('/home/subodha/garnet-gem5/mcpat/Parser/Xeon.xml', '/home/subodha/garnet-gem5/mcpat/Parser/Xeon_new.xml') # complete target filename given, (golden xml path, output new xml path)

tree = ET.parse('/home/subodha/garnet-gem5/mcpat/Parser/Xeon_new.xml')
root = tree.getroot()


#####################################################	
#Parameter file open
pfile = open('/home/subodha/garnet-gem5/mcpat/Parser/parameters.txt', 'r')
pfiletext = pfile.read()
#Stats file open
sfile = open('/home/subodha/garnet-gem5/mcpat/Parser/stats_new.txt', 'r')
sfiletext = sfile.read()
#####################################################	


#####################################################	
#find integer value for string in parameter file
def findValueParam(str):
	regx = re.escape(str) + ".*$"
	items=re.findall((regx),pfiletext,re.MULTILINE)
	#val =  ([int(s) for s in items[0].split() if s.isdigit()][0])
	nums=0
	nums+=float((re.findall(r"(?<![a-zA-Z:])[-+]?\d*\.?\d+",items[0])[0]))
	#print int(nums)
	return int(round(nums))

#find integer value for string in stats file
def findValueStats(str):
	regx = re.escape(str) + ".*$"
	items=re.findall((regx),sfiletext,re.MULTILINE)
	#val =  ([int(s) for s in items[0].split() if s.isdigit()][0])
	nums=0
	nums+=float(re.findall(r"(?<![a-zA-Z:])[-+]?\d*\.?\d+",items[0])[0])
	#print int(nums)
	return int(round(nums))

#find aggregate sum of values for string in stats file
def findSumStats(str):
	sum=0
	regx = re.escape(str) + ".*$"
	items=re.findall((regx),sfiletext,re.MULTILINE)
	for x in range(0,len(items)):
		nums=re.findall(r"(?<![a-zA-Z:])[-+]?\d*\.?\d+",items[x])[0]
		#nums=re.findall(r"[-+]?\d*\.\d+|\d+",items[x])[0]
		sum+=float(nums)
		#print int(sum)
	#print float(sum)
	return int(round(sum))
	
#####################################################	
	
number_of_cores = findValueParam("number_of_cores")
number_of_L1Directories = findValueParam("number_of_L1Directories")
target_core_clockrate = findValueParam("target_core_clockrate")

total_cycles = findSumStats("numCycles")
idle_cycles = findSumStats("num_idle_cycles")
busy_cycles = findSumStats("num_busy_cycles")

clock_rate = findValueParam("clock_rate")
total_instructions = findValueStats("sim_insts")
int_instructions = findSumStats("num_int_insts")
fp_instructions = findSumStats("fp_insts")
branch_instructions = findSumStats(".Branches")
branch_mispredictions = 0
load_instructions = findSumStats(".num_load_insts")
store_instructions = findSumStats(".num_store_insts")
committed_instructions = findSumStats(".committedInsts")
committed_int_instructions = findSumStats(".num_int_alu_accesses")
committed_fp_instructions = findSumStats(".num_fp_alu_accesses")
function_calls = findSumStats(".num_func_calls")
ialu_accesses = findSumStats(".num_int_alu_accesses")
fpu_accesses = committed_fp_instructions
mul_accesses = (findSumStats(".op_class::IntMult") + findSumStats(".op_class::IntDiv"))
cdb_alu_accesses = ialu_accesses
cdb_mul_accesses = mul_accesses
cdb_fpu_accesses = fpu_accesses

icache_config = str(findValueParam("icache_config_capacity")) + "," + str(findValueParam("icache_config_block_width")) + "," +  str(findValueParam("icache_config_associativity")) + "," + str(findValueParam("icache_config_bank")) + "," + str(findValueParam("icache_config_throughput_wrt_core_clock")) + "," + str(findValueParam("icache_config_latency_wrt_core_clock")) + "," + str(findValueParam("icache_config_output_width")) + "," + str(findValueParam("icache_config_cache policy"))	
icache_read_accesses = 0		#hardcoded to 0 #accesses??? or hits not found in xeon file
icache_read_misses = 0			#hardcoded to 0

dcache_config = str(findValueParam("dcache_config_capacity")) + "," + str(findValueParam("dcache_config_block_width")) + "," +  str(findValueParam("dcache_config_associativity")) + "," + str(findValueParam("dcache_config_bank")) + "," + str(findValueParam("dcache_config_throughput_wrt_core_clock")) + "," + str(findValueParam("dcache_config_latency_wrt_core_clock")) + "," + str(findValueParam("dcache_config_output_width")) + "," + str(findValueParam("dcache_config_cache policy"))
dcache_read_accesses = findSumStats(".cacheMemory.demand_accesses")
dcache_write_accesses = 0
dcache_read_misses = findSumStats(".cacheMemory.demand_misses")
dcache_write_misses = 0

L10_read_access = dcache_read_misses

L20_config = str(findValueParam("L2cache_config_capacity")) + "," + str(findValueParam("L2cache_config_block_width")) + "," +  str(findValueParam("L2cache_config_associativity")) + "," + str(findValueParam("L2cache_config_bank")) + "," + str(findValueParam("L2cache_config_throughput_wrt_core_clock")) + "," + str(findValueParam("L2cache_config_latency_wrt_core_clock")) + "," + str(findValueParam("L2cache_config_output_width")) + "," + str(findValueParam("L2cache_config_cache policy"))
L20_read_accesses = findValueStats("l2_accesses")
L20_read_misses = findValueStats("l2_misses")
L20_write_accesses = 0
L20_write_misses = 0


NoC0_horizontal_nodes = findValueParam("NoC0_horizontal_nodes")
NoC0_vertical_nodes = findValueParam("NoC0_vertical_nodes")
NoC0_total_accesses = findValueStats("system.ruby.network.flits_injected::total") * findSumStats("system.ruby.network.average_hops")
print findSumStats("system.ruby.network.average_hops")
print findValueStats("system.ruby.network.flits_injected::total")

mc_memory_accesses = findSumStats("memReq")
mc_memory_reads = findSumStats(".memRead")
mc_memory_writes = findSumStats(".memWrite")

core_tech_node = findValueParam("core_tech_node")
number_mcs = findValueParam("number_mcs")




root[0][0].set('value',str(number_of_cores))
root[0][1].set('value',str(number_of_L1Directories))
root[0][15].set('value',str(target_core_clockrate))
root[0][26].set('value',str(total_cycles))
root[0][27].set('value',str(idle_cycles))
root[0][28].set('value',str(busy_cycles))
#for child in root[0]:
#	print child.tag, child.attrib
#print root[0][0].attrib
#print root[0]



root[0][29][0].set('value',str(clock_rate))
root[0][29][41].set('value',str(total_instructions))
root[0][29][42].set('value',str(int_instructions))
root[0][29][43].set('value',str(fp_instructions))
root[0][29][44].set('value',str(branch_instructions))
root[0][29][45].set('value',str(branch_mispredictions))
root[0][29][46].set('value',str(load_instructions))
root[0][29][47].set('value',str(store_instructions))
root[0][29][48].set('value',str(committed_instructions))
root[0][29][49].set('value',str(committed_int_instructions))
root[0][29][50].set('value',str(committed_fp_instructions))
root[0][29][71].set('value',str(function_calls))
root[0][29][73].set('value',str(ialu_accesses))
root[0][29][74].set('value',str(fpu_accesses))
root[0][29][75].set('value',str(mul_accesses))
root[0][29][76].set('value',str(cdb_alu_accesses))
root[0][29][77].set('value',str(cdb_mul_accesses))
root[0][29][78].set('value',str(cdb_fpu_accesses))			 
#for child in root[0][29]:
#	print child.tag, child.attrib
#print root[0][29][0].attrib

root[0][29][92][0].set('value',str(icache_config))
root[0][29][92][2].set('value',str(icache_read_accesses))
root[0][29][92][3].set('value',str(icache_read_misses))
#print root[0][29][92].attrib
#for child in root[0][29][92]:
#	print child.tag, child.attrib

root[0][29][94][0].set('value',str(dcache_config))
root[0][29][94][2].set('value',str(dcache_read_accesses))
root[0][29][94][3].set('value',str(dcache_write_accesses))
root[0][29][94][4].set('value',str(dcache_read_misses))
root[0][29][94][5].set('value',str(dcache_write_misses))
#print root[0][29][94].attrib
#for child in root[0][29][94]:
#	print child.tag, child.attrib

root[0][30][8].set('value',str(L10_read_access))
#print root[0][30][8].attrib

root[0][32][0].set('value',str(L20_config))
root[0][32][7].set('value',str(L20_read_accesses))
root[0][32][9].set('value',str(L20_read_misses))

root[0][32][8].set('value',str(L20_write_accesses))
root[0][32][10].set('value',str(L20_write_misses))
#print root[0][32].attrib
#for child in root[0][32]:
#	print child.tag, child.attrib

root[0][34][4].set('value',str(NoC0_horizontal_nodes))
root[0][34][5].set('value',str(NoC0_vertical_nodes))
root[0][34][16].set('value',str(NoC0_total_accesses))
#print root[0][34].attrib
#for child in root[0][34]:
#	print child.tag, child.attrib

root[0][35][14].set('value',str(mc_memory_accesses))
root[0][35][15].set('value',str(mc_memory_reads))
root[0][35][16].set('value',str(mc_memory_writes))
#print root[0][35].attrib
#for child in root[0][35]:
#	print child.tag, child.attrib


root[0][14].set('value',str(core_tech_node))
#print root[0][14].attrib

root[0][35][6].set('value',str(number_mcs))
#print root[0][35][6].attrib



pfile.close()
sfile.close()

outfile = open(r'/home/subodha/garnet-gem5/mcpat/Parser/Xeon_new2.xml','w')
outfile.write("<?xml version=\"1.0\" ?>\n")
tree.write(outfile)
