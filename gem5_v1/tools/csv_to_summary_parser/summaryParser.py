##################################################################
# Script to create a summary out of all the test cases
# Used to generate results for ESWEEK Paper
# 
# Authors: Nikhil Venkatesh, Subodha Charles
# 02/13/2018
##################################################################

import subprocess
import os
import locale
import csv
import sys
import re

##################################################################
# Defining some global variables
##################################################################

L1I_CACHE_CONFIG_SPACE = {'l1i_size':['8kB','16kB','32kB'],'l1i_assoc':[1,2,4]}
#L1I_CACHE_CONFIG_SPACE = {'l1i_size':['8kB'],'l1i_assoc':[1]}
L1D_CACHE_CONFIG_SPACE = {'l1d_size':['8kB','16kB','32kB'],'l1d_assoc':[1,2,4]}
#L1D_CACHE_CONFIG_SPACE = {'l1d_size':['8kB'],'l1d_assoc':[1]}
#L2_CACHE_CONFIG_SPACE = {'l2_size':[8192,16384,32768],'l2_assoc':[4,8,16]}
#L2_CACHE_CONFIG_SPACE = {'l2_size':[65536,131072,262144],'l2_assoc':[4,8,16]}
L2_CACHE_CONFIG_SPACE = {'l2_size':[32768, 65536,131072,262144],'l2_assoc':[1,2]}

l2ConfigSpace = ['32kB_1','32kB_2','64kB_1','64kB_2','128kB_1','128kB_2','256kB_1','256kB_2']

#this allows us to extract energy of any component we need from mcpat. eg: Processor, NoC etc.
COMPONENT = 'Processor'

BENCHMARKS = []

CACHE_CONFIGS = [] #cache configs in a dictionary
PARSED_CACHE_CONFIGS = [] #cache configs as strings (8kB_1_8kB_1_64kB_4)
csvStatList = [] #final stats that get written to CSV file

#Paths to files
#GEM5_INPUT_FILE_DIR = "/home/subodha/" #used to give L2 Cache config inputs to gem5
#GEM5_ROOT = '/home/subodha/garnet-gem5/'
#MCPAT_ROOT = '/home/subodha/garnet-gem5/mcpat/mcpat/'
#GEM5_OUT_PARENT = '/home/subodha/garnet-gem5/m5out/ESWEEK/run3/'
#GEM5_BENCHMARKS_DIR = '/home/subodha/m5thread-gem5/programs/'
#PARSER_DIR = '/home/subodha/garnet-gem5/mcpat/Parser/'

##################################################################
# function definitions to make life easier
##################################################################

#initialize configurations and write to appropriate files
def initConfigs():
	global PARSED_CACHE_CONFIGS

	#get all possible configurations
	getExhaustiveHeuristic()

	#convert the dicionary values to strings for easy comprison
	PARSED_CACHE_CONFIGS = [parse(x) for x in CACHE_CONFIGS]

	#initialize stats 
	initCSVList()

#Main function that writes the summary files
def parseToSummaryCSV():
	global csvStatList
	for benchmark in BENCHMARKS:
		
		csvStatList = []
		initCSVList() #initialize stats everytime we Parse a new folder with a given benchmark

		benchmarkOutputDir = energyOutDir + "/" + benchmark #construct the output path with benchmark name

		execCacheConfigs = os.listdir(benchmarkOutputDir) #get list of available cache config outputs
		
		for cacheConfig in PARSED_CACHE_CONFIGS:
			print cacheConfig
			#check whether we have stats for the cache config
			if cacheConfig in execCacheConfigs:
				statsOutputDir = benchmarkOutputDir + "/" + cacheConfig

				#if yes, extract them from gem5 and mcpat outputs
				stats = extractStats(statsOutputDir)
				populateCSVList(cacheConfig,stats)

		writeToCSV(benchmarkOutputDir,benchmark)

#get execution time from gem5 stats.txt file
def extractTimingData(path):
	sfile = open(path, 'r')
	sfiletext = sfile.read()	

	regx = re.escape('sim_seconds') + ".*$"
	items=re.findall((regx),sfiletext,re.MULTILINE)

	nums=0
	nums+=float(re.findall(r"(?<![a-zA-Z:])[-+]?\d*\.?\d+",items[0])[0])

	sfile.close()
	return round(nums*1000,2) #return miliseconds rounded to two decimal points

#get COMPONENT energy from mcpat results. Results are assumed to be parsed to energy.csv using the master script
def extractEnergyData(path):
	f = open(path, 'r')
	reader = csv.reader(f)
	energyStats = list(reader)

	componentCol = [row.index('Processor') for i, row in enumerate(energyStats) if "Processor" in row][0]
	
	dataRow = [i for i, row in enumerate(energyStats) if "Runtime Dynamic Power (W)" in row][0]
	#print str(dataRow) + " | " + str(componentCol)
	energy = energyStats[dataRow][componentCol]

	f.close()
	return round(float(energy),2)

#extract performance and power results from stats and send to final csv stats
def extractStats(statsOutputDir):
	execTime = extractTimingData(statsOutputDir + "/stats.txt")
	energy = extractEnergyData(statsOutputDir+ "/energy.csv")

	return [execTime,energy]
	
#write the final output summary csv file
def writeToCSV(path, benchmark):
	global csvStatList
	csvOutPath = path + "/" + benchmark + "summary.csv"

	csvStatList = [[''] + l2ConfigSpace] + csvStatList

	f = open(csvOutPath,'w')
	csv_writer=csv.writer(f)
	csv_writer.writerows(csvStatList)
	f.close()

#given a cache config string (8kB_1_8kB_1_64kB_4), split it into L1 and L2 configs
def splitCacheConfig(cacheConfig):
	splitIndex = [m.start() for m in re.finditer('_', cacheConfig)][3]
	l1Config = cacheConfig[:splitIndex]
	l2Config = cacheConfig[splitIndex+1:]
	return [l1Config, l2Config]

#since the CSV columns should be in order, get the corresponding index for a given L2 config
def getL2IndexLocation(l2Config):
	configIndex = l2ConfigSpace.index(l2Config)
	if configIndex >= 0:
		return 2*configIndex + 1
	else:
		print "No such L2 cache config possible... :("	

#since the CSV rows should be in order, get the corresponding index for a given L1 config
def getL1IndexLocation(l1Config):
	configIndex = [i for i, row in enumerate(csvStatList) if l1Config in row]
	try:
		return configIndex[0]
	except Exception as int:
		print "No such L1 cache config possible... :("
		
#initialize stats with all zeros and row headers. Col headers appeneded in writeToCSV function
def initCSVList():
	global csvStatList

	l2ConfigSpaceSize = len(getPossibleCacheConfigs(L2_CACHE_CONFIG_SPACE, 16))
	#print getPossibleCacheConfigs(L2_CACHE_CONFIG_SPACE, 64)

	l1ConfigSpace = [splitCacheConfig(cacheConfig)[0] for cacheConfig in PARSED_CACHE_CONFIGS]
	l1ConfigSpace = list(set(l1ConfigSpace))

	csvStatList = [[l1Config]+[0]*2*l2ConfigSpaceSize for l1Config in l1ConfigSpace]

#given stats and a cache config, add those results to the list that gets written to final csv file
def populateCSVList(cacheConfig, stats):
	global csvStatList
	#print cacheConfig
	cacheConfigs = splitCacheConfig(cacheConfig)

	#print "configs : " + str(cacheConfigs[0]) + " | " + str(cacheConfigs[1])

	l1Index = getL1IndexLocation(cacheConfigs[0])
	l2Index = getL2IndexLocation(cacheConfigs[1])

	#print "indexes : " + str(l1Index) + " | " + str(l2Index)

	csvStatList[l1Index][l2Index] = stats[0]
	csvStatList[l1Index][l2Index+1] = stats[1]

	#print csvStatList	

#converts string of kb to string of bytes	
def kbtobyte(kb):
	return str(int(kb[:-2])*1024)

#convert a dictionary to a string to use as a file name
def parse(config):
	parsedConfig = str(config['l1i_size']) + "_" + \
   				   str(config['l1i_assoc']) + "_" + \
   				   str(config['l1d_size']) + "_" + \
   				   str(config['l1d_assoc'])+ "_" + \
   				   str(config['l2_size']/1024) + "kB_" + \
   				   str(config['l2_assoc'])
	
	return parsedConfig

#convert a size string to int
def toInt(size):
	if type(size) is not str:
		return size/1024
	else:
		return int(size.strip("kB"))

#given the cache config space, populate a list of all possible
#cache configs depending on reconfigurable cache architecture
#ref: \cite{wang2009dynamic} Section 3.1
def getPossibleCacheConfigs(CACHE_CONFIG_SPACE, bankSize):

	keys = []
	for key in CACHE_CONFIG_SPACE:
		keys.append(key)
	sizes = CACHE_CONFIG_SPACE[keys[0]]

	assocs = CACHE_CONFIG_SPACE[keys[1]]
	minAssoc = min(assocs)
	assocs = [assoc/minAssoc for assoc in assocs]

	'''	
	for size in sizes:
		for assoc in assocs:
			print str(toInt(size)) + " | " + str(assoc) + " | " + str(toInt(size)/assoc)
			if (toInt(size)/assoc)>=bankSize:
				print [size, assoc]
	'''

	config = [[size,assoc*minAssoc] for size in sizes for assoc in assocs if ((toInt(size)/assoc)>=bankSize)]

	#print config
	return config	 

#From the possible cache configs, get ehaustive combinations of L1I, L1D and L2 caches
#Accounts for 6x6x6 = 216 max possiblities in our explorations
def getExhaustiveHeuristic():
	cacheConfig = {}

	L1I_CONFIGS = getPossibleCacheConfigs(L1I_CACHE_CONFIG_SPACE, 8)	
	L1D_CONFIGS = getPossibleCacheConfigs(L1D_CACHE_CONFIG_SPACE, 8)
	L2_CONFIGS = getPossibleCacheConfigs(L2_CACHE_CONFIG_SPACE, 16)

	for l1iConfig in L1I_CONFIGS:
		for l1dConfig in L1D_CONFIGS:
			for l2Config in L2_CONFIGS:
				cacheConfig['l1i_size'] = l1iConfig[0]
				cacheConfig['l1i_assoc'] = l1iConfig[1]
				cacheConfig['l1d_size'] = l1dConfig[0]
				cacheConfig['l1d_assoc'] = l1dConfig[1]
				cacheConfig['l2_size'] = l2Config[0]
				cacheConfig['l2_assoc'] = l2Config[1]
				
				CACHE_CONFIGS.append(cacheConfig.copy())
	
	#print len(CACHE_CONFIGS)

##################################################################
# let's run it now, shall we?
##################################################################
energyOutDir = str(sys.argv[1])
#summaryOutDir = str(sys.argv[2])

BENCHMARKS = os.listdir(energyOutDir)

initConfigs()
parseToSummaryCSV()

print "My work is done here....."
