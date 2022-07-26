/*
 * main.cpp
 *
 *	This is the main program of MulticoreSim.
 *
 *  Created on: Apr 24, 2010
 *      Author: Weixun Wang
    Added on Nov. 2016, Yuanwen Huang
 */

#include "stdafx.h"
#include "MulticoreDCRSim.h"

int main(int argc, char** argv)
{
	char* csv_path = argv[1];
	char* taskset = argv[2];
	int deadline = atoi(argv[3]);
	// *****************************************************************************
	// particular deadline
	MulticoreDCRSim multicoreDCRSim;

	multicoreDCRSim.readTaskInfo(taskset);
	multicoreDCRSim.setDeadline(deadline);
	multicoreDCRSim.loadTaskProfileTable(csv_path);
	multicoreDCRSim.dumpBaseCases();
	multicoreDCRSim.dynamicProgrammingSolver();
	multicoreDCRSim.generateL2PartitionList(false);
	multicoreDCRSim.findSolution();
	
	return 0;
}
