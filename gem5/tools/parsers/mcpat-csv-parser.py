import sys
import os
import shutil
import re
import csv

#######################################################################################################################################
###################### How to run this script: ########################################################################################
###################### python parserxml.py mcpatOutPath modelCsvPath csvOutPath #######################################################
#######################################################################################################################################

mcpatOutPath = str(sys.argv[1])
modelCsvPath = str(sys.argv[2])
csvOutPath = str(sys.argv[3])

mfile = open(mcpatOutPath, 'r')

mfiletext = mfile.read()


##############################useful functions to find float value from mcpat output####################################################
#function to finds list of occurances of string, calls helper function to return nth index integer value
def findValuesMcpat(str,ind):
	regx = re.escape(str) + ".*$"
	items=re.findall((regx),mfiletext,re.MULTILINE)
	#print items
	return findIntIndex(ind,items)

def findIntIndex(index,items):
	nums=re.findall(r"(?<![a-zA-Z:])[-+]?\d*\.?\d+",items[index])
	#print float(nums[0])
	return float(nums[0])

##########################model csv path given below####################################################################################
shutil.copy2(modelCsvPath, csvOutPath) # complete target filename given, (golden xml path, output new csv path)
f = open(csvOutPath, 'r')
reader = csv.reader(f)
mylist = list(reader)
f.close()

#find all values
processorPeakPower = findValuesMcpat("Peak Power",0)
processorTotalLeakage = findValuesMcpat("Total Leakage",0)
processorPeakDynamic = findValuesMcpat("Peak Dynamic",0)
processorGateLeakage = findValuesMcpat("Gate Leakage",0)
processorRuntimeDynamic = findValuesMcpat("Runtime Dynamic",0)


coresPeakDynamic = findValuesMcpat("Peak Dynamic",1)
coreGateLeakage = findValuesMcpat("Gate Leakage",1)
coreRuntimeDynamic = findValuesMcpat("Runtime Dynamic",1)

nocsPeakDynamic = findValuesMcpat("Peak Dynamic",4)
nocsGateLeakage = findValuesMcpat("Gate Leakage",4)
nocsRuntimeDynamic = findValuesMcpat("Runtime Dynamic",4)

l2PeakDynamic = findValuesMcpat("Peak Dynamic",12)
l2GateLeakage = findValuesMcpat("Gate Leakage",10)
l2RuntimeDynamic = findValuesMcpat("Runtime Dynamic",12)

memPeakDynamic = findValuesMcpat("Peak Dynamic",5)
memGateLeakage = findValuesMcpat("Gate Leakage",5)
memRuntimeDynamic = findValuesMcpat("Runtime Dynamic",5)

#processor write out
mylist[1][1]= processorPeakPower
mylist[2][1]= processorTotalLeakage
mylist[3][1]= processorPeakDynamic
mylist[4][1]= processorGateLeakage
mylist[5][1]= processorRuntimeDynamic

#cores write out
mylist[3][2]= coresPeakDynamic
mylist[4][2]= coreGateLeakage
mylist[5][2]= coreRuntimeDynamic

#NoCs write out
mylist[3][3]= nocsPeakDynamic
mylist[4][3]= nocsGateLeakage
mylist[5][3]= nocsRuntimeDynamic

#L2 write out
mylist[3][4]= l2PeakDynamic
mylist[4][4]= l2GateLeakage
mylist[5][4]= l2RuntimeDynamic

#L2 write out
mylist[3][5]= memPeakDynamic
mylist[4][5]= memGateLeakage
mylist[5][5]= memRuntimeDynamic

print mylist

my_new_list = open(csvOutPath,'w')
csv_writer=csv.writer(my_new_list)
csv_writer.writerows(mylist)
my_new_list.close()
