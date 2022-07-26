/*
 * MulticoreDCRSim.h
 *
 *	This header file defines the multicore sim for DCR.
 *
 *  Created on: Oct 04, 2010
 *      Author: Weixun Wang
 */

//#pragma once

#include "stdafx.h"
#include "task.h"
#include <cassert>
class MulticoreDCRSim
{
private:
	int num_cores;										// total number of cores
	int deadline;										// common deadline for all tasks
	int l2Assoc;										// L2 cache associativity
	int baseL1Index;									// base L1 configuration index
	vector<vector<Task> > taskList;						// task lists for each core (given a task mapping)

	int num_tasks;										// total number of tasks
	vector<Task> taskListAll;							// task list for all tasks (for task mapping study)
	vector<vector<int> > taskMappingListAll;			// list of all possible task mapping (for task mapping study)
	
	vector<vector<vector<int> > > coreSolutionList;		// solution list for each core under each L2 partition factor (0: total energy; 1: total time)
	vector<vector<int> > partitionList;					// L2 partition factor lists
	vector<vector<vector<int> > > solutionList;			// solution list with (0: L1 combination; 1: L2 partition factor) for each task / each core
	vector<vector<int> > optimalPartitionList;			// list of optimal L2 partition factor, under all L1 configurations

	vector<int> varyingCPList;							// list of points when L2 CP is changed
	int phase;

	// vulnerability-aware with varying vulnerability constraints
	vector<int> coreVulConstraints;


public:
	MulticoreDCRSim();
	~MulticoreDCRSim();

	int readTaskInfo(string);							// read task information
	int loadTaskProfileTable(string);					// load in each task's profile table

	void findOptimalPartition(double);					// find the optimal partition factor
	void generateL2PartitionList(bool);					// generate all possible L2 cache partitioning factor lists
	void generateL2PartitionListBF();					// generate all possible L2 cache partitioning factor lists (Bruto-force algorithm)

	void dynamicProgrammingSolver();					// find the optimal/appximated solution using dynamic programming
	void findSolution();								// find best solution combinting L1 DCR and L2 CP

	int randomNumberGenerater(int, int);				// random number generator
	void setDeadline(int);								// set deadline

	int getL2Assoc();

	// for vulnerability-aware energy optimization study
	int loadTaskProfileTableVul(string);	// load in each task's profile (energy, time, vulnerability)
	void dynamicProgrammingSolverVul();
	void findSolutionVul();
	// vulnerability-aware with varying vulnerability constraints
	void findAllSolutionVaryingVulConstraints();
	void dynamicProgrammingSolverVaryingVulConstraint();

	// for varying cache partitioning study
	int loadTaskProfileTablePhase(string);				// load in each task's profile table (for phased simulation results)
	void varyingPartitionSolverBF(int,vector<double>);
	void varyingPartitionSolverDP(int,vector<double>);
	void taskEnergyCalculatorHelper(vector<vector<int> >, int, int, int, int, bool&, int&, int&, int&);
	void exploreAllVaryingPartition();
	void generateVaryingL2PartitionList(vector<vector<int> >&);
	void generateAllPartitionFactorSetHelper(vector<vector<int> >&);
	void generateAllL1ConfigurationSetHelper(vector<vector<int> >&, int);
	void generateAllCombinationHelper(vector<vector<int> >&, vector<int>&, int, int, int, int);
	void setPhase(int);

	// for task mapping study
	int readTaskInfoAll(string);						// read task information (for all-task list)
	int loadTaskProfileTableAll(string);				// load in each task's profile table (for all-task list)
	void generateAllTaskMapping();						// generate all possible task mappings (for all-task list)
	void generateAllTaskMappingHelper(vector<vector<int> >&, vector<vector<int> >, vector<int>&, int, int);
	void generateAllTaskMappingHelperHelper(vector<vector<int> >&, vector<int>&, int, int);
	vector<vector<int> > combinationHelper(vector<int>, int);
	void combinationHelperHelper(vector<vector<int> >&, vector<bool>&, int, int, int);
	void exploreAllTaskMapping();

	// FOR DEBUG
	void dumpPartitionList();							// dump out partition list
	void dumpProfileTables();							// dump out profile table for each task / each core
	void dumpBaseCases();								// dump out selected base L1 configuration entry for all tasks
	void dumpOptimalPartitionList();					// dump out optimal partition list
};

class ComparerEnergy
{
public:

	bool operator () (const PTEntry v1, const PTEntry v2)
	{
		return v1.energy < v2.energy;
	}
};

class ComparerIndex
{
public:

	bool operator () (const PTEntry v1, const PTEntry v2)
	{
		return v1.index < v2.index;
	}
};

MulticoreDCRSim::MulticoreDCRSim()
{
	num_cores = 0;
	deadline = 0;
	l2Assoc = 0;
	baseL1Index = 0;
}

MulticoreDCRSim::~MulticoreDCRSim()
{
}

int MulticoreDCRSim::readTaskInfo(string fileName)
{
	char line[MAX_LENGTH];

	ifstream in(fileName.c_str(),ios::in);

	if (!in)
	{
		printf("Cannot open file %s!",fileName.c_str());
		return 1;
	}

	in.getline(line,MAX_LENGTH);
	sscanf(line,"%d",&num_cores);

	in.getline(line,MAX_LENGTH);
	sscanf(line,"%d",&l2Assoc);

	in.getline(line,MAX_LENGTH);
	sscanf(line,"%d",&baseL1Index);

	// prepare taskList, solutionList sizes
	taskList.resize(num_cores);
	solutionList.resize(num_cores);

	int id = 0;

	for (int i = 0 ; i < num_cores ; i++)
	{
		int num_tasks = 0;

		in.getline(line,MAX_LENGTH,',');
		sscanf(line,"%d",&num_tasks);

		solutionList[i].resize(num_tasks);

		for (int j = 0 ; j < num_tasks ; j++)
		{
			in.getline(line,MAX_LENGTH,',');
			char inName[MAX_LENGTH];

			sscanf(line,"%s",&inName);

			taskList[i].push_back(Task(id,string(inName)));
			id++;

			solutionList[i][j].resize(2);
		}

		in.getline(line,MAX_LENGTH,'\n');
	}

	//in.getline(line,MAX_LENGTH,',');
	//sscanf(line,"%d",&deadline);

	// FOR DEBUG
	printf("There are %d cores \n",num_cores);
	printf("L2 cache associativity is %d \n",l2Assoc);
	for (int i = 0 ; i < num_cores ; i++)
	{
		for (size_t j = 0 ; j < taskList[i].size() ; j++)
		{
			printf("%s ,",taskList[i][j].name.c_str());
		}
		printf("\n");
	}
	//printf("Deadline is %d \n\n",deadline);

	return 0;
}

int MulticoreDCRSim::loadTaskProfileTable(string path)
{
	// for each core
	for (int i = 0 ; i < num_cores ; i++)
	{
		// for each task
		for (size_t j = 0 ; j < taskList[i].size() ; j++)
		{
			char line[MAX_LENGTH];
			float inEnergy;
			float inTime;
			char inL1configStr[MAX_LENGTH];
			char fileName[MAX_LENGTH];
			sprintf(fileName,"%s/%s.csv",path.c_str(),taskList[i][j].name.c_str());

			// 0 is not used
			taskList[i][j].profileTable.resize(l2Assoc);
			for (int k = 0 ; k < l2Assoc ; k++)
			{
				taskList[i][j].profileTable[k].resize(TOTAL_CONFIG_CNT);
			}

			ifstream in(fileName,ios::in);

			if (!in)
			{
				printf("Cannot open file %s!",fileName);
				return 1;
			}

			// for each L1 configuration
			for (int l = 0 ; l < TOTAL_CONFIG_CNT ; l++)
			{
				in.getline(line,MAX_LENGTH,',');
				sscanf(line,"%s",&inL1configStr);
			
				// for each L2 partition factor
				for (int k = 1 ; k < l2Assoc ; k++)
				{
					// for time and energy
					in.getline(line,MAX_LENGTH,',');
					sscanf(line,"%f",&inTime);
					in.getline(line,MAX_LENGTH,',');
					sscanf(line,"%f",&inEnergy);	//power
					inEnergy = inTime * inEnergy;	//energy

					taskList[i][j].profileTable[k][l].L1configStr = inL1configStr;
					taskList[i][j].profileTable[k][l].index = l;
					taskList[i][j].profileTable[k][l].energy = (int)(inEnergy /(double) ENERGY_UNIT_SCALE);
					taskList[i][j].profileTable[k][l].time = (int)(inTime /(double) TIME_UNIT_SCALE);
				}
			}
		}
	}

	return 0;
}

int MulticoreDCRSim::loadTaskProfileTableVul(string path)
{
	// for each core
	for (int i = 0 ; i < num_cores ; i++)
	{
		// for each task
		for (size_t j = 0 ; j < taskList[i].size() ; j++)
		{
			char line[MAX_LENGTH];
			float inEnergy;
			float inTime;
			float inVul;
			char inL1configStr[MAX_LENGTH];
			char fileName[MAX_LENGTH];
			sprintf(fileName,"%s/%s.csv",path.c_str(),taskList[i][j].name.c_str());

			// 0 is not used
			taskList[i][j].profileTable.resize(l2Assoc);
			for (int k = 0 ; k < l2Assoc ; k++)
			{
				taskList[i][j].profileTable[k].resize(TOTAL_CONFIG_CNT);
			}

			ifstream in(fileName,ios::in);

			if (!in)
			{
				printf("Cannot open file %s!",fileName);
				return 1;
			}

			// for each L1 configuration
			for (int l = 0 ; l < TOTAL_CONFIG_CNT ; l++)
			{
				in.getline(line,MAX_LENGTH,',');
				sscanf(line,"%s",&inL1configStr);
			
				// for each L2 partition factor
				for (int k = 1 ; k < l2Assoc ; k++)
				{
					// for energy and time
					in.getline(line,MAX_LENGTH,',');
					sscanf(line,"%f",&inEnergy);
					in.getline(line,MAX_LENGTH,',');
					sscanf(line,"%f",&inTime);
					in.getline(line,MAX_LENGTH,',');
					sscanf(line,"%f",&inVul);

					taskList[i][j].profileTable[k][l].L1configStr = inL1configStr;
					taskList[i][j].profileTable[k][l].index = l;
					taskList[i][j].profileTable[k][l].energy = (int)(inEnergy /(double) ENERGY_UNIT_SCALE);
					taskList[i][j].profileTable[k][l].time = (int)(inTime /(double) TIME_UNIT_SCALE);
					taskList[i][j].profileTable[k][l].vul = (int) inVul; // already scaled by 10^6 in perl scripts
				}
			}
		}
	}

	return 0;
}

void MulticoreDCRSim::generateL2PartitionList(bool useGatedVdd)
{
	partitionList.clear();

	int lowerbound(0);

	if (useGatedVdd)
		lowerbound = num_cores;
	else
		lowerbound = l2Assoc;

	for (int totAssoc = l2Assoc ; totAssoc >= lowerbound ; totAssoc--)
	{
		vector<int> flagList;
		flagList.resize(l2Assoc - 1);

		for (int i = 0 ; i < num_cores - 1 ; i++)
		{
			flagList[i] = 1;
		}

		while (true)
		{
			// record the combination
			vector<int> tmp;
			vector<int> onesList;
			for (size_t i = 0 ; i < flagList.size() ; i++)
			{
				if (flagList[i] == 1)
				{
					onesList.push_back(i);
				}
			}

			for (size_t i = 0 ; i < onesList.size() ; i++)
			{
				if (i == 0)
				{
					tmp.push_back(onesList[i] + 1);
				}
				else
				{
					tmp.push_back(onesList[i] - onesList[i - 1]);
				}
			}
			tmp.push_back(totAssoc - 1 - onesList[onesList.size() - 1]);
			partitionList.push_back(tmp);

			// test whether all combinations have been found
			bool hasOne = false;
			for (int i = 0 ; i < (totAssoc - 1) - (num_cores - 1) ; i++)
			{
				if (flagList[i] == 1)
				{
					hasOne = true;
					break;
				}
			}

			if (!hasOne)
				break;

			// search of "10"
			int idx = 0;
			for (int i = 0 ; i < totAssoc - 1 - 1 ; i++)
			{
				if (flagList[i] == 1 && flagList[i + 1] == 0)
				{
					idx = i;
					break;
				}
			}

			// change "10" to "01"
			flagList[idx] = 0;
			flagList[idx + 1] = 1;

			// move all "1"s on the left to the leftmost end
			int count = 0;
			for (int i = 0 ; i < idx ; i++)
			{
				if (flagList[i] == 1)
					count++;
			}

			for (int i = 0 ; i < count ; i++)
			{
				flagList[i] = 1;
			}

			for (int i = count ; i < idx ; i++)
			{
				flagList[i] = 0;
			}
		}
	}
}

void MulticoreDCRSim::generateL2PartitionListBF()
{
	vector<vector<int> > combinationList;

	int *p = new int[num_cores];
	int *maxP = new int[num_cores];

	for (int i = 0 ; i < num_cores ; i++)
	{
		p[i] = 1;
		maxP[i] = l2Assoc - 1;
	}

	int stackPt = 0;
	int prevStackPt = -1;
	bool notFirstTime = false;

	while (true)
	{
		if (p[stackPt] < maxP[stackPt])
		{
			if (num_cores == 1)
			{
				if (notFirstTime)
				{
					p[stackPt]++;
				}
				notFirstTime = true;
			}
			else if (stackPt < num_cores - 1)
			{
				if (prevStackPt > stackPt)
				{
					p[stackPt]++;
				}

				prevStackPt = stackPt;
				stackPt++;
			}
			else
			{
				p[stackPt]++;
			}
		}
		else
		{
			p[stackPt] = 1;
			prevStackPt = stackPt;
			stackPt--;
		}

		if (stackPt == num_cores - 1)
		{
			vector<int> tmp;

			for (int i = 0 ; i < num_cores ; i++)
			{
				tmp.push_back(p[i]);
			}

			combinationList.push_back(tmp);
		}

		if (stackPt == -1)
			break;
	}

	// FOR DEBUG
	/*for (size_t i = 0 ; i < combinationList.size() ; i++)
	{
		for (size_t j = 0 ; j < combinationList[i].size() ; j++)
		{
			printf("%d ",combinationList[i][j]);
		}

		printf("\n");
	}*/

	// eliminate all invalid combinations to get partitionList
	for (size_t i = 0 ; i < combinationList.size() ; i++)
	{
		int sum(0);

		for (size_t j = 0 ; j < combinationList[i].size() ; j++)
		{
			sum += combinationList[i][j];
		}

		if (sum == l2Assoc)
		{
			partitionList.push_back(combinationList[i]);
		}
	}

	dumpPartitionList();
}


void MulticoreDCRSim::findAllSolutionVaryingVulConstraints()
{
	coreVulConstraints.resize(num_cores);	
	
	int minEnergyDCRCP = std::numeric_limits<int>::max();
	int minEnergyTimeDCRCP = 0;
	int minEnergyVulDCRCP = 0;
	int minEnergyVulDCRCP_maxVul = 0;
	
	int minEnergyCP = std::numeric_limits<int>::max();
	int minEnergyTimeCP = 0;
	int minEnergyVulCP = 0;
	int minEnergyVulCP_maxVul = 0;
	
	int energyDCR = 0;
	int timeDCR = 0;
	int vulDCR = 0;
	int vulDCR_maxVul = 0;

	int partitionIdxDCRCP = 0;
	int partitionIdxCP = 0;

	int baseEnergy=0;
	int baseVul=0;
	int baseTime=0;
	int baseVul_maxVul=0;
	
	// ************** UCP: BaseCase (uniform CP + no DCR) *******
	for (size_t i = 0 ; i < taskList.size() ; i++)
	{
		int time = 0;
		int vul = 0;
		for (size_t j = 0 ; j < taskList[i].size() ; j++)
		{
			baseEnergy += taskList[i][j].profileTable[2][baseL1Index].energy;
			vul += taskList[i][j].profileTable[2][baseL1Index].vul;
			time += taskList[i][j].profileTable[2][baseL1Index].time;
		}
		baseVul += vul;
		baseVul_maxVul = max(baseVul_maxVul, vul);
		baseTime = max(time, baseTime);
		
		coreVulConstraints[i] = vul;
	}
	
	dynamicProgrammingSolverVaryingVulConstraint(); // use the updated coreVulConstraints of UCP
	
	// *************** L1 DCR + Uniform CP ***************
	bool isValid = true;
	for (int i = 0 ; i < num_cores ; i++)
	{
		// this core does not have a valid assignment for this partition factor
		if (coreSolutionList[i][2][0] == -1)
		{
			isValid = false;
			break;
		}

		// energy is the sum over all cores
		energyDCR += coreSolutionList[i][2][0];
		vulDCR += coreSolutionList[i][2][2];

		// time is the max of all cores
		if (coreSolutionList[i][2][1] > timeDCR)
		{
			timeDCR = coreSolutionList[i][2][1];
		}
		vulDCR_maxVul = max(vulDCR_maxVul, coreSolutionList[i][2][2]);
		coreVulConstraints[i] = coreSolutionList[i][2][2];
	}
	if (isValid)
	{
		printf ("Minimum energy for L1 DCR + Uniform L2 CP is %d, vul is %d time is %d: \n",energyDCR, vulDCR, timeDCR);
		for (int i = 0 ; i < num_cores ; i++)
		{
			printf(" Core %d use PF = %d: ",i,2);

			// each task's L1 config assignment
			for (size_t j = 0 ; j < taskList[i].size() ; j++)
			{
				int pf = 2;
				int idx = coreSolutionList[i][pf][3 + j];
				printf(" %d:%s ",idx,taskList[i][j].profileTable[pf][idx].L1configStr.c_str());
			}

			printf(" \n");
		}
		printf(" \n");
	}
	
	dynamicProgrammingSolver();
	findSolution(); // set coreVulConstraints as from Weixun's vul
	dynamicProgrammingSolverVaryingVulConstraint(); // use updated coreVulConstraints of DCR+UCP

	// *************** L1 DCR + L2 CP ***************
	// for all possible L2 partition schemes
	for (size_t i = 0 ; i < partitionList.size() ; i++)
	{
		int energy = 0;
		int time = 0;
		int vul = 0;
		int maxVul = 0;
		bool isValid = true;

		for (int j = 0 ; j < num_cores ; j++)
		{
			// this core does not have a valid assignment for this partition factor
			if (coreSolutionList[j][partitionList[i][j]][0] == -1)
			{
				isValid = false;
				break;
			}

			// energy is the sum over all cores
			energy += coreSolutionList[j][partitionList[i][j]][0];
			vul += coreSolutionList[j][partitionList[i][j]][2];

			// time is the max of all cores
			if (coreSolutionList[j][partitionList[i][j]][1] > time)
			{
				time = coreSolutionList[j][partitionList[i][j]][1];
			}
			maxVul = max(maxVul, coreSolutionList[j][partitionList[i][j]][2]);
		}

		// ignore this partition scheme
		if (!isValid)
			continue;

		if (energy < minEnergyDCRCP)
		{
			minEnergyDCRCP = energy;
			minEnergyVulDCRCP = vul;
			minEnergyVulDCRCP_maxVul = maxVul;
			minEnergyTimeDCRCP = time;
			partitionIdxDCRCP = i;
		}
	}

	printf ("Minimum energy for L1 DCR + L2 CP is %d, vul is %d, time is %d (using partition scheme %d): \n",minEnergyDCRCP,minEnergyVulDCRCP, minEnergyTimeDCRCP,partitionIdxDCRCP);
	for (int i = 0 ; i < num_cores ; i++)
	{
		printf(" Core %d use PF = %d: ",i,partitionList[partitionIdxDCRCP][i]);

		// each task's L1 config assignment
		for (size_t j = 0 ; j < taskList[i].size() ; j++)
		{
			int pf = partitionList[partitionIdxDCRCP][i];
			int idx = coreSolutionList[i][pf][3 + j];
			printf(" %d:%s ",idx,taskList[i][j].profileTable[pf][idx].L1configStr.c_str());
		}

		printf(" \n");
	}
	printf(" \n");



	
/*
	//*************** L2 CP only ***************
	// first restore the profile table order of each task
	for (int i = 0 ;i < num_cores ; i++)
	{
		for (size_t j = 0 ; j < taskList[i].size() ; j++)
		{
			for (int k = 1 ;k < l2Assoc ; k++)
			{
				sort(taskList[i][j].profileTable[k].begin(),taskList[i][j].profileTable[k].end(),ComparerIndex());
			}
		}
	}
	// for all possible L2 partition schemes
	for (size_t i = 0 ; i < partitionList.size() ; i++)
	{
		int energy = 0;
		int time = 0;
		int vul = 0;
		int maxVul = 0;
		bool isValid = true;

		for (int j = 0 ; j < num_cores ; j++)
		{
			int curTime = 0;
			int curVul = 0;
			int vulConstraint = 0;

			for (size_t k = 0 ; k < taskList[j].size() ; k++)
			{
				int pf = partitionList[i][j];
				energy += taskList[j][k].profileTable[pf][baseL1Index].energy;
				curVul += taskList[j][k].profileTable[pf][baseL1Index].vul;
				curTime += taskList[j][k].profileTable[pf][baseL1Index].time;
				
				pf = 2; // comment this if you want a different vulConstraint
				vulConstraint += taskList[j][k].profileTable[pf][baseL1Index].vul;
			}
			
			if (curVul > vulConstraint || curTime > deadline)
			{
				isValid = false;
				break;
			}
			// if this core is Valid
			vul += curVul;
			time = max(time, curTime);
			maxVul = max(maxVul, curVul);
		}
		if (!isValid)
			continue;

		//if (time > deadline)
		//	continue;

		if (energy < minEnergyCP)
		{
			minEnergyCP = energy;
			minEnergyTimeCP = time;
			minEnergyVulCP = vul;
			minEnergyVulCP_maxVul = maxVul;
			partitionIdxCP = i;
		}
	}

	printf ("Minimum energy for L2 CP only is %d, vul is %d, time is %d (using partition scheme %d): \n",minEnergyCP,minEnergyVulCP, minEnergyTimeCP,partitionIdxCP);
	for (int i = 0 ; i < num_cores ; i++)
	{
		printf(" Core %d use PF = %d, with base L1 %d \n",i,partitionList[partitionIdxCP][i],baseL1Index);
	}
	printf(" \n");
	//printf("The Energy Improvement is %f \n",(minEnergyCP - minEnergyDCRCP) /(double) minEnergyCP);
	printf("Improvement:, Energy, Vulnerability\n");
	printf("DCRCP, %.2f, %.2f\n", (minEnergyCP - minEnergyDCRCP) /(double) minEnergyCP, (minEnergyVulCP - minEnergyVulDCRCP)/(double)minEnergyVulCP);
*/

	// dump results to file
	char filename[50];
	sprintf(filename,"results_tm.csv");

	ofstream out;
	out.open(filename,ios::app);
	out<<"vul-aware,";
	for (size_t i = 0 ; i < taskList.size() ; i++)
	{
		out << "([" << partitionList[partitionIdxDCRCP][i] << "]";
		for (size_t j = 0 ; j < taskList[i].size() ; j++)
		{
			out << taskList[i][j].name << " ";
		}
		out << coreSolutionList[i][partitionList[partitionIdxDCRCP][i]][2] ; // vul on this core
		out << "),";
	}

	out << deadline << "," << minEnergyCP << "," << minEnergyDCRCP << "," << endl;
	
	out.close();
	
	
	
	
	printf("Vul-Aware, Energy, Time, Vulnerability, deadline=%d\n", deadline);
	printf("UCP, %d, %d, %d, %d\n", baseEnergy, baseTime, baseVul, baseVul_maxVul);
	//printf("CP, %d, %d, %d, %d\n", minEnergyCP, minEnergyTimeCP, minEnergyVulCP, minEnergyVulCP_maxVul);
	printf("DCR+UCP, %d, %d, %d, %d\n", energyDCR, timeDCR, vulDCR, vulDCR_maxVul);
	printf("DCR+CP, %d, %d, %d, %d\n\n\n", minEnergyDCRCP, minEnergyTimeDCRCP, minEnergyVulDCRCP, minEnergyVulDCRCP_maxVul);
	ofstream outAll;
	outAll.open("allSets.csv",ios::app);
	outAll << "Vul-Aware, Energy, Time, Vulnerability, maxVul, deadline=" << deadline << endl;
	outAll << "UCP,"<< baseEnergy <<"," << baseTime <<"," << baseVul << endl;
	//outAll << "CP," << minEnergyCP <<"," << minEnergyTimeCP << "," << minEnergyVulCP << "," << minEnergyVulCP_maxVul << endl;
	outAll << "DCR+UCP," << energyDCR <<"," << timeDCR <<"," << vulDCR << "," << vulDCR_maxVul << endl;
	outAll << "DCR+CP," << minEnergyDCRCP <<"," << minEnergyTimeDCRCP <<"," << minEnergyVulDCRCP << "," << minEnergyVulDCRCP_maxVul << endl << endl;
}


void MulticoreDCRSim::dynamicProgrammingSolverVaryingVulConstraint()
{
	// prepare coreSolutionList
	coreSolutionList.clear();
	coreSolutionList.resize(num_cores);

	for (int i = 0 ; i < num_cores ; i++)
	{
		coreSolutionList[i].resize(l2Assoc);
	}

	// for each core
	for (int i = 0 ; i < num_cores ; i++)
	{
		// for each L2 partition factor
		for (int k = 1 ;k < l2Assoc ; k++)
		{
			// ************************************************************************************
			// Start dynamic programming: taskList[i][j].profileTable[k] for all j
			// ************************************************************************************
			printf("Core %d, PF %d ... ",i,k);

			// first sort each task's profile table in partition factor k
			for (size_t j = 0 ; j < taskList[i].size() ; j++)
			{
				sort(taskList[i][j].profileTable[k].begin(),taskList[i][j].profileTable[k].end(),ComparerEnergy());
			}

			// get the upper and lower bound of the dynamic programming
			int upperBound = 0;
			int lowerBound = 0;
			
			for (size_t j = 0 ; j < taskList[i].size() ; j++)
			{
				upperBound += taskList[i][j].profileTable[k][taskList[i][j].profileTable[k].size() - 1].energy;
			}
			
			// schedualability constraint
			int timeConstraint = deadline;
			int invalidTimeTableEntry = timeConstraint + 1000000; 
			int invalidVulTableEntry = std::numeric_limits<int>::max(); 
			
			// define the storage space for dynamic programming
			vector<vector<int> > timeTable;
			vector<vector<int> > vulTable;
			vector<vector<int> > solutionTable; // the 'best' L1config index

			timeTable.resize(taskList[i].size());
			vulTable.resize(taskList[i].size());
			solutionTable.resize(taskList[i].size());

			for (size_t j = 0 ; j < timeTable.size() ; j++)
			{
				timeTable[j].resize(upperBound - lowerBound);
				vulTable[j].resize(upperBound - lowerBound);
				solutionTable[j].resize(upperBound - lowerBound);

				for (size_t l = 0 ; l < timeTable[j].size() ; l++)
				{
					timeTable[j][l] = invalidTimeTableEntry;
					vulTable[j][l] = invalidVulTableEntry;
					solutionTable[j][l] = -1;
				}
			}

			// dynamic programming solver (using original energy value, a minimization version)
			// loop over all the tasks
			for (size_t j = 0 ; j < timeTable.size() ; j++)
			{
				// consider only the first task, trivial
				if (j == 0)
				{
					for (int l = 0 ; l < upperBound - lowerBound ; l++)
					{
						// for all L1 configurations
						for (size_t m = 0 ; m < taskList[i][j].profileTable[k].size() ; m++)
						{
							//printf("		Task %d , l = %d , L1 config %d ... \n ",j,l,m);

							if (taskList[i][j].profileTable[k][m].energy == l + lowerBound)
							{
								if (taskList[i][j].profileTable[k][m].time < timeTable[j][l]
									&& taskList[i][j].profileTable[k][m].vul < vulTable[j][l])
								{
									timeTable[j][l] = taskList[i][j].profileTable[k][m].time;
									vulTable[j][l] = taskList[i][j].profileTable[k][m].vul;
									solutionTable[j][l] = m;
								}
							}
						}
					}
				}
				else
				{
					// loop over total energy from lowerBound to upperBound for every task
					for (int l = 0 ; l < upperBound - lowerBound ; l++)
					{
						// loop over all L1 configurations for task i,j with partition scheme k
						for (size_t m = 0 ; m < taskList[i][j].profileTable[k].size() ; m++)
						{
							//printf("		Task %d , l = %d , L1 config %d ... \n ",j,l,m);

							int energyValueLast = l + lowerBound - taskList[i][j].profileTable[k][m].energy;

							if (energyValueLast < 0)
								continue;

							//if ((taskList[i][j].profileTable[k][m].time + timeTable[j - 1][energyValueLast]) < timeTable[j][l] && (taskList[i][j].profileTable[k][m].vul + vulTable[j-1][energyValueLast]) < vulTable[j][l])
							if ((taskList[i][j].profileTable[k][m].time + timeTable[j - 1][energyValueLast]) < timeTable[j][l])
							{
								timeTable[j][l] = taskList[i][j].profileTable[k][m].time + timeTable[j - 1][energyValueLast];
								vulTable[j][l] = taskList[i][j].profileTable[k][m].vul + vulTable[j-1][energyValueLast];
								solutionTable[j][l] = m;
							}
						}
					}

				}
			}

			// FOR DEBUG
			/*ofstream out;
			out.open("dump_solutionlist.txt");

			for (size_t j = 0 ; j < taskList[i].size() ; j++)
			{
				for (int l = 0 ; l < upperBound - lowerBound ; l++)
				{
					out << solutionTable[j][l] << ",";
				}
				out << endl;
			}

			out.close();*/

			// find the minimum energy consumption of all the tasks and with total time less than the deadline
			for (int l = 0 ; l < upperBound - lowerBound ; l++)
			{
				// the first one with valid schedulability condition is the one with minimum energy consumption
				if (timeTable[timeTable.size() - 1][l] <= timeConstraint
					&& vulTable[vulTable.size() - 1][l] <= coreVulConstraints[i])
				{
					coreSolutionList[i][k].push_back(l);
					coreSolutionList[i][k].push_back(timeTable[timeTable.size() - 1][l]);
					coreSolutionList[i][k].push_back(vulTable[vulTable.size() - 1][l]);

					// backtrack to get the solution for each task
					vector<int> tmp;
					tmp.resize(taskList[i].size());

					tmp[taskList[i].size() - 1] = solutionTable[taskList[i].size() - 1][l];

					int energyValueLast = l - taskList[i][taskList[i].size() - 1].profileTable[k][tmp[taskList[i].size() - 1]].energy;

					for (int j = taskList[i].size() - 2 ; j >= 0 ; j--)
					{
						tmp[j] = solutionTable[j][energyValueLast];
						energyValueLast -= taskList[i][j].profileTable[k][tmp[j]].energy;
					}

					// push the solution index (in the sorted profile table) list after Energy and Time
					for (size_t j = 0 ; j < taskList[i].size() ; j++)
					{
						int idx = taskList[i][j].profileTable[k][tmp[j]].index;
						coreSolutionList[i][k].push_back(idx);
					}

					break;
				}
			}

			// no valid assignment for this partition factor
			if (coreSolutionList[i][k].size() == 0)
			{
				coreSolutionList[i][k].push_back(-1);
				coreSolutionList[i][k].push_back(-1);
				coreSolutionList[i][k].push_back(-1);

				for (size_t j = 0 ; j < taskList[i].size() ; j++)
				{
					coreSolutionList[i][k].push_back(-1);
				}
			}
		}
	}

	printf("\n");



	// first restore the profile table order of each task
	for (int i = 0 ;i < num_cores ; i++)
	{
		for (size_t j = 0 ; j < taskList[i].size() ; j++)
		{
			for (int k = 1 ;k < l2Assoc ; k++)
			{
				sort(taskList[i][j].profileTable[k].begin(),taskList[i][j].profileTable[k].end(),ComparerIndex());
			}
		}
	}	


	// FOR DEBUG
	for (int i = 0 ; i < num_cores ; i++)
	{
		printf("Core %d: \n",i);
		for (int k = 1 ; k < l2Assoc ; k++)
		{
			printf("PF=%d: ",k);
			
			if (coreSolutionList[i][k][0] > -1)
			{
				printf("E=%d,T=%d,V=%d [",coreSolutionList[i][k][0],coreSolutionList[i][k][1],coreSolutionList[i][k][2]);
				for (size_t j = 0 ; j < taskList[i].size() ; j++)
				{
					int idx = coreSolutionList[i][k][3 + j];
					printf(" %d:%s ",idx,taskList[i][j].profileTable[k][idx].L1configStr.c_str());
				}
				printf("] \n");
			}
			else
			{
				printf("E=%d,T=%d,V=%d []\n",coreSolutionList[i][k][0],coreSolutionList[i][k][1],coreSolutionList[i][k][2]);
			}
		}
		printf("\n");
	}
}


void MulticoreDCRSim::dynamicProgrammingSolverVul()
{
	// prepare coreSolutionList
	coreSolutionList.clear();
	coreSolutionList.resize(num_cores);

	for (int i = 0 ; i < num_cores ; i++)
	{
		coreSolutionList[i].resize(l2Assoc);
	}
	
	// first restore the profile table order of each task
	for (int i = 0 ;i < num_cores ; i++)
	{
		for (size_t j = 0 ; j < taskList[i].size() ; j++)
		{
			for (int k = 1 ;k < l2Assoc ; k++)
			{
				sort(taskList[i][j].profileTable[k].begin(),taskList[i][j].profileTable[k].end(),ComparerIndex());
			}
		}
	}
	coreVulConstraints.clear();
	coreVulConstraints.resize(num_cores);
	for (int i = 0 ; i < num_cores ; i++)
	{
		coreVulConstraints[i] = 0;
		for (size_t j=0; j<taskList[i].size(); j++) 
		{
			// vulConstraint is the sum of (baseL1 and PF_L2 = 2) 
			coreVulConstraints[i] += taskList[i][j].profileTable[2][baseL1Index].vul;
		}
	}
	
	
	// for each core
	for (int i = 0 ; i < num_cores ; i++)
	{
		// for each L2 partition factor
		for (int k = 1 ;k < l2Assoc ; k++)
		{
			// ************************************************************************************
			// Start dynamic programming: taskList[i][j].profileTable[k] for all j
			// ************************************************************************************
			printf("Core %d, PF %d ... ",i,k);



			// first sort each task's profile table in partition factor k
			for (size_t j = 0 ; j < taskList[i].size() ; j++)
			{
				sort(taskList[i][j].profileTable[k].begin(),taskList[i][j].profileTable[k].end(),ComparerEnergy());
			}

			// get the upper and lower bound of the dynamic programming
			int upperBound = 0;
			int lowerBound = 0;
			
			for (size_t j = 0 ; j < taskList[i].size() ; j++)
			{
				upperBound += taskList[i][j].profileTable[k][taskList[i][j].profileTable[k].size() - 1].energy;
			}
			
			// schedualability constraint
			int timeConstraint = deadline;
			int invalidTimeTableEntry = timeConstraint + 1000000; 
			int invalidVulTableEntry = std::numeric_limits<int>::max(); 
			
			// define the storage space for dynamic programming
			vector<vector<int> > timeTable;
			vector<vector<int> > vulTable;
			vector<vector<int> > solutionTable; // the 'best' L1config index

			timeTable.resize(taskList[i].size());
			vulTable.resize(taskList[i].size());
			solutionTable.resize(taskList[i].size());

			for (size_t j = 0 ; j < timeTable.size() ; j++)
			{
				timeTable[j].resize(upperBound - lowerBound);
				vulTable[j].resize(upperBound - lowerBound);
				solutionTable[j].resize(upperBound - lowerBound);

				for (size_t l = 0 ; l < timeTable[j].size() ; l++)
				{
					timeTable[j][l] = invalidTimeTableEntry;
					vulTable[j][l] = invalidVulTableEntry;
					solutionTable[j][l] = -1;
				}
			}

			// dynamic programming solver (using original energy value, a minimization version)
			// loop over all the tasks
			for (size_t j = 0 ; j < timeTable.size() ; j++)
			{
				// consider only the first task, trivial
				if (j == 0)
				{
					for (int l = 0 ; l < upperBound - lowerBound ; l++)
					{
						// for all L1 configurations
						for (size_t m = 0 ; m < taskList[i][j].profileTable[k].size() ; m++)
						{
							//printf("		Task %d , l = %d , L1 config %d ... \n ",j,l,m);

							if (taskList[i][j].profileTable[k][m].energy == l + lowerBound)
							{
								if (taskList[i][j].profileTable[k][m].time < timeTable[j][l]
									&& taskList[i][j].profileTable[k][m].vul < vulTable[j][l])
								{
									timeTable[j][l] = taskList[i][j].profileTable[k][m].time;
									vulTable[j][l] = taskList[i][j].profileTable[k][m].vul;
									solutionTable[j][l] = m;
								}
							}
						}
					}
				}
				else
				{
					// loop over total energy from lowerBound to upperBound for every task
					for (int l = 0 ; l < upperBound - lowerBound ; l++)
					{
						// loop over all L1 configurations for task i,j with partition scheme k
						for (size_t m = 0 ; m < taskList[i][j].profileTable[k].size() ; m++)
						{
							//printf("		Task %d , l = %d , L1 config %d ... \n ",j,l,m);

							int energyValueLast = l + lowerBound - taskList[i][j].profileTable[k][m].energy;

							if (energyValueLast < 0)
								continue;

							if ((taskList[i][j].profileTable[k][m].time + timeTable[j - 1][energyValueLast]) < timeTable[j][l]
								&& (taskList[i][j].profileTable[k][m].vul + vulTable[j-1][energyValueLast]) < vulTable[j][l])
							{
								timeTable[j][l] = taskList[i][j].profileTable[k][m].time + timeTable[j - 1][energyValueLast];
								vulTable[j][l] = taskList[i][j].profileTable[k][m].vul + vulTable[j-1][energyValueLast];
								solutionTable[j][l] = m;
							}
						}
					}

				}
			}

			// FOR DEBUG
			/*ofstream out;
			out.open("dump_solutionlist.txt");

			for (size_t j = 0 ; j < taskList[i].size() ; j++)
			{
				for (int l = 0 ; l < upperBound - lowerBound ; l++)
				{
					out << solutionTable[j][l] << ",";
				}
				out << endl;
			}

			out.close();*/

			// find the minimum energy consumption of all the tasks and with total time less than the deadline
			for (int l = 0 ; l < upperBound - lowerBound ; l++)
			{
				// the first one with valid schedulability condition is the one with minimum energy consumption
				if (timeTable[timeTable.size() - 1][l] <= timeConstraint
					&& vulTable[vulTable.size() - 1][l] <= coreVulConstraints[i])
				{
					coreSolutionList[i][k].push_back(l);
					coreSolutionList[i][k].push_back(timeTable[timeTable.size() - 1][l]);
					coreSolutionList[i][k].push_back(vulTable[vulTable.size() - 1][l]);

					// backtrack to get the solution for each task
					vector<int> tmp;
					tmp.resize(taskList[i].size());

					tmp[taskList[i].size() - 1] = solutionTable[taskList[i].size() - 1][l];

					int energyValueLast = l - taskList[i][taskList[i].size() - 1].profileTable[k][tmp[taskList[i].size() - 1]].energy;

					for (int j = taskList[i].size() - 2 ; j >= 0 ; j--)
					{
						tmp[j] = solutionTable[j][energyValueLast];
						energyValueLast -= taskList[i][j].profileTable[k][tmp[j]].energy;
					}

					// push the solution index (in the sorted profile table) list after Energy and Time
					for (size_t j = 0 ; j < taskList[i].size() ; j++)
					{
						int idx = taskList[i][j].profileTable[k][tmp[j]].index;
						coreSolutionList[i][k].push_back(idx);
					}

					break;
				}
			}


			// for DAC2017 explore of vulnerability threshold on optimal energy
			// explore task set 5 (core 1, PF=2, DDL=4000) only: 
		bool exploreVulThresh = true;
		if (exploreVulThresh && i==0 && k==2) {
			ofstream out;
			out.open("set5_vulThreshExplore.csv");
			out<< "coreVulConstraint = " << coreVulConstraints[i] << endl;
			for (int vulThresh = coreVulConstraints[i]; vulThresh >= coreVulConstraints[i]/4; vulThresh = vulThresh - 20000 ) {
				int soluEnergy = 0;
				int soluTime = 0;
				int soluVul = 0;
				for (int l = 0 ; l < upperBound - lowerBound ; l++)
				{
				// the first one with valid schedulability condition is the one with minimum energy consumption		
					//if (timeTable[timeTable.size() - 1][l] <= timeConstraint
					//&& vulTable[vulTable.size() - 1][l] <= vulThresh)
					if (vulTable[vulTable.size() - 1][l] <= vulThresh)
					{
						soluEnergy = l;
						soluTime = timeTable[timeTable.size() - 1][l];
						soluVul = vulTable[vulTable.size() - 1][l];
						
						break;
					}
				}
				out << vulThresh << "," << soluEnergy << endl;
			}
			
			out.close();
			out.open("set5_deadlineExplore.csv");
			for (int deadline = timeConstraint+500; deadline >= timeConstraint - 2000; deadline = deadline - 10) {
				int soluEnergy = 0;
				int soluTime = 0;
				int soluVul = 0;
				for (int l = 0 ; l < upperBound - lowerBound ; l++)
				{
				// the first one with valid schedulability condition is the one with minimum energy consumption
					if (timeTable[timeTable.size() - 1][l] <= deadline
					&& vulTable[vulTable.size() - 1][l] <= coreVulConstraints[i])
					{
						soluEnergy = l;
						soluTime = timeTable[timeTable.size() - 1][l];
						soluVul = vulTable[vulTable.size() - 1][l];
						
						break;
					}
				}
				out << deadline << "," << soluEnergy << endl;
			}
			out.close();
		}	

			// no valid assignment for this partition factor
			if (coreSolutionList[i][k].size() == 0)
			{
				coreSolutionList[i][k].push_back(-1);
				coreSolutionList[i][k].push_back(-1);
				coreSolutionList[i][k].push_back(-1);

				for (size_t j = 0 ; j < taskList[i].size() ; j++)
				{
					coreSolutionList[i][k].push_back(-1);
				}
			}
		}
	}

	printf("\n");	


	// FOR DEBUG
	for (int i = 0 ; i < num_cores ; i++)
	{
		printf("Core %d: \n",i);
		for (int k = 1 ; k < l2Assoc ; k++)
		{
			printf("PF=%d: ",k);
			
			if (coreSolutionList[i][k][0] > -1)
			{
				printf("E=%d,T=%d,V=%d [",coreSolutionList[i][k][0],coreSolutionList[i][k][1],coreSolutionList[i][k][2]);
				for (size_t j = 0 ; j < taskList[i].size() ; j++)
				{
					int idx = coreSolutionList[i][k][3 + j];
					printf(" %d:%s ",idx,taskList[i][j].profileTable[k][idx].L1configStr.c_str());
				}
				printf("] \n");
			}
			else
			{
				printf("E=%d,T=%d,V=%d []\n",coreSolutionList[i][k][0],coreSolutionList[i][k][1],coreSolutionList[i][k][2]);
			}
		}
		printf("\n");
	}
}


void MulticoreDCRSim::findSolutionVul()
{
	int minEnergyDCRCP = std::numeric_limits<int>::max();
	int minEnergyTimeDCRCP = 0;
	int minEnergyVulDCRCP = 0;
	int minEnergyVulDCRCP_maxVul = 0;
	
	int minEnergyCP = std::numeric_limits<int>::max();
	int minEnergyTimeCP = 0;
	int minEnergyVulCP = 0;
	int minEnergyVulCP_maxVul = 0;
	
	int energyDCR = 0;
	int timeDCR = 0;
	int vulDCR = 0;
	int vulDCR_maxVul = 0;

	int partitionIdxDCRCP = 0;
	int partitionIdxCP = 0;

	// restore the profile table order of each task
	for (int i = 0 ;i < num_cores ; i++)
	{
		for (size_t j = 0 ; j < taskList[i].size() ; j++)
		{
			for (int k = 1 ;k < l2Assoc ; k++)
			{
				sort(taskList[i][j].profileTable[k].begin(),taskList[i][j].profileTable[k].end(),ComparerIndex());
			}
		}
	}

	// *************** L1 DCR + L2 CP ***************
	// for all possible L2 partition schemes
	for (size_t i = 0 ; i < partitionList.size() ; i++)
	{
		int energy = 0;
		int time = 0;
		int vul = 0;
		int maxVul = 0;
		bool isValid = true;

		for (int j = 0 ; j < num_cores ; j++)
		{
			// this core does not have a valid assignment for this partition factor
			if (coreSolutionList[j][partitionList[i][j]][0] == -1)
			{
				isValid = false;
				break;
			}

			// energy is the sum over all cores
			energy += coreSolutionList[j][partitionList[i][j]][0];
			vul += coreSolutionList[j][partitionList[i][j]][2];

			// time is the max of all cores
			if (coreSolutionList[j][partitionList[i][j]][1] > time)
			{
				time = coreSolutionList[j][partitionList[i][j]][1];
			}
			maxVul = max(maxVul, coreSolutionList[j][partitionList[i][j]][2]);
		}

		// ignore this partition scheme
		if (!isValid)
			continue;

		if (energy < minEnergyDCRCP)
		{
			minEnergyDCRCP = energy;
			minEnergyVulDCRCP = vul;
			minEnergyVulDCRCP_maxVul = maxVul;
			minEnergyTimeDCRCP = time;
			partitionIdxDCRCP = i;
		}
	}

	printf ("Minimum energy for L1 DCR + L2 CP is %d, vul is %d, time is %d (using partition scheme %d): \n",minEnergyDCRCP,minEnergyVulDCRCP, minEnergyTimeDCRCP,partitionIdxDCRCP);
	for (int i = 0 ; i < num_cores ; i++)
	{
		printf(" Core %d use PF = %d: ",i,partitionList[partitionIdxDCRCP][i]);

		// each task's L1 config assignment
		for (size_t j = 0 ; j < taskList[i].size() ; j++)
		{
			int pf = partitionList[partitionIdxDCRCP][i];
			int idx = coreSolutionList[i][pf][3 + j];
			printf(" %d:%s_%s ",idx,taskList[i][j].profileTable[pf][idx].L1configStr.c_str(), taskList[i][j].name.c_str());
		}

		printf(" \n");
	}
	printf(" \n");



	// *************** L1 DCR + Uniform CP ***************
	bool isValid = true;

	for (int i = 0 ; i < num_cores ; i++)
	{
		// this core does not have a valid assignment for this partition factor
		if (coreSolutionList[i][2][0] == -1)
		{
			isValid = false;
			break;
		}

		// energy is the sum over all cores
		energyDCR += coreSolutionList[i][2][0];
		vulDCR += coreSolutionList[i][2][2];

		// time is the max of all cores
		if (coreSolutionList[i][2][1] > timeDCR)
		{
			timeDCR = coreSolutionList[i][2][1];
		}
		vulDCR_maxVul = max(vulDCR_maxVul, coreSolutionList[i][2][2]);
	}

	if (isValid)
	{
		printf ("Minimum energy for L1 DCR + Uniform L2 CP is %d, vul is %d time is %d: \n",energyDCR, vulDCR, timeDCR);
		for (int i = 0 ; i < num_cores ; i++)
		{
			printf(" Core %d use PF = %d: ",i,2);

			// each task's L1 config assignment
			for (size_t j = 0 ; j < taskList[i].size() ; j++)
			{
				int pf = 2;
				int idx = coreSolutionList[i][pf][3 + j];
				printf(" %d:%s ",idx,taskList[i][j].profileTable[pf][idx].L1configStr.c_str());
			}

			printf(" \n");
		}
		printf(" \n");
	}

	// *************** L2 CP only ***************
	// first restore the profile table order of each task
	for (int i = 0 ;i < num_cores ; i++)
	{
		for (size_t j = 0 ; j < taskList[i].size() ; j++)
		{
			for (int k = 1 ;k < l2Assoc ; k++)
			{
				sort(taskList[i][j].profileTable[k].begin(),taskList[i][j].profileTable[k].end(),ComparerIndex());
			}
		}
	}

	// for all possible L2 partition schemes
	for (size_t i = 0 ; i < partitionList.size() ; i++)
	{
		int energy = 0;
		int time = 0;
		int vul = 0;
		int maxVul = 0;
		bool isValid = true;

		for (int j = 0 ; j < num_cores ; j++)
		{
			int curTime = 0;
			int curVul = 0;
			int vulConstraint = 0;

			for (size_t k = 0 ; k < taskList[j].size() ; k++)
			{
				int pf = partitionList[i][j];
				energy += taskList[j][k].profileTable[pf][baseL1Index].energy;
				curVul += taskList[j][k].profileTable[pf][baseL1Index].vul;
				curTime += taskList[j][k].profileTable[pf][baseL1Index].time;
				
				pf = 2; // comment this if you want a different vulConstraint
				vulConstraint += taskList[j][k].profileTable[pf][baseL1Index].vul;
			}
			
			if (curVul > vulConstraint || curTime > deadline)
			{
				isValid = false;
				break;
			}
			// if this core is Valid
			vul += curVul;
			time = max(time, curTime);
			maxVul = max(maxVul, curVul);
		}
		if (!isValid)
			continue;

		//if (time > deadline)
		//	continue;

		if (energy < minEnergyCP)
		{
			minEnergyCP = energy;
			minEnergyTimeCP = time;
			minEnergyVulCP = vul;
			minEnergyVulCP_maxVul = maxVul;
			partitionIdxCP = i;
		}
	}

	printf ("Minimum energy for L2 CP only is %d, vul is %d, time is %d (using partition scheme %d): \n",minEnergyCP,minEnergyVulCP, minEnergyTimeCP,partitionIdxCP);
	for (int i = 0 ; i < num_cores ; i++)
	{
		printf(" Core %d use PF = %d, with base L1 %d \n",i,partitionList[partitionIdxCP][i],baseL1Index);
	}
	printf(" \n");
	//printf("The Energy Improvement is %f \n",(minEnergyCP - minEnergyDCRCP) /(double) minEnergyCP);
	printf("Improvement:, Energy, Vulnerability\n");
	printf("DCRCP, %.2f, %.2f\n", (minEnergyCP - minEnergyDCRCP) /(double) minEnergyCP, (minEnergyVulCP - minEnergyVulDCRCP)/(double)minEnergyVulCP);

	// dump results to file
	char filename[50];
	sprintf(filename,"results_tm.csv");

	ofstream out;
	out.open(filename,ios::app);
	out<<"vul-aware,";
	for (size_t i = 0 ; i < taskList.size() ; i++)
	{
		out << "([" << partitionList[partitionIdxDCRCP][i] << "]";
		for (size_t j = 0 ; j < taskList[i].size() ; j++)
		{
			out << taskList[i][j].name << " ";
		}
		out << coreSolutionList[i][partitionList[partitionIdxDCRCP][i]][2] ; // vul on this core
		out << "),";
	}

	out << deadline << "," << minEnergyCP << "," << minEnergyDCRCP << "," << endl;
	
	out.close();
	
	int baseEnergy=0, baseVul=0, baseTime=0, baseVul_maxVul=0;
	for (size_t i = 0 ; i < taskList.size() ; i++)
	{
		int time = 0;
		int vul = 0;
		for (size_t j = 0 ; j < taskList[i].size() ; j++)
		{
			baseEnergy += taskList[i][j].profileTable[2][baseL1Index].energy;
			vul += taskList[i][j].profileTable[2][baseL1Index].vul;
			time += taskList[i][j].profileTable[2][baseL1Index].time;
		}
		baseVul += vul;
		baseVul_maxVul = max(baseVul_maxVul, vul);
		baseTime = max(time, baseTime);
	}
	
	
	printf("Vul-Aware, Energy, Time, Vulnerability, deadline=%d\n", deadline);
	printf("Base, %d, %d, %d, %d\n", baseEnergy, baseTime, baseVul, baseVul_maxVul);
	printf("CP, %d, %d, %d, %d\n", minEnergyCP, minEnergyTimeCP, minEnergyVulCP, minEnergyVulCP_maxVul);
	printf("DCR, %d, %d, %d, %d\n", energyDCR, timeDCR, vulDCR, vulDCR_maxVul);
	printf("DCR+CP, %d, %d, %d, %d\n\n\n", minEnergyDCRCP, minEnergyTimeDCRCP, minEnergyVulDCRCP, minEnergyVulDCRCP_maxVul);
	ofstream outAll;
	outAll.open("allSets.csv",ios::app);
	outAll << "Vul-Aware, Energy, Time, Vulnerability, maxVul, deadline=" << deadline << endl;
	outAll << "Base,"<< baseEnergy <<"," << baseTime <<"," << baseVul <<"," << baseVul_maxVul << endl;
	outAll << "CP," << minEnergyCP <<"," << minEnergyTimeCP << "," << minEnergyVulCP << "," << minEnergyVulCP_maxVul << endl;
	outAll << "DCR," << energyDCR <<"," << timeDCR <<"," << vulDCR << "," << vulDCR_maxVul << endl;
	outAll << "DCR+CP," << minEnergyDCRCP <<"," << minEnergyTimeDCRCP <<"," << minEnergyVulDCRCP << "," << minEnergyVulDCRCP_maxVul << endl << endl;
}



void MulticoreDCRSim::dynamicProgrammingSolver()
{
	// prepare coreSolutionList
	coreSolutionList.clear();
	coreSolutionList.resize(num_cores);

	for (int i = 0 ; i < num_cores ; i++)
	{
		coreSolutionList[i].resize(l2Assoc);
	}

	// for each core
	for (int i = 0 ; i < num_cores ; i++)
	{
		// for each L2 partition factor
		for (int k = 1 ;k < l2Assoc ; k++)
		{
			// ************************************************************************************
			// Start dynamic programming: taskList[i][j].profileTable[k] for all j
			// ************************************************************************************
			//printf("Core %d, PF %d ... ",i,k);

			// first sort each task's profile table in partition factor k
			for (size_t j = 0 ; j < taskList[i].size() ; j++)
			{
				sort(taskList[i][j].profileTable[k].begin(),taskList[i][j].profileTable[k].end(),ComparerEnergy());
			}

			// get the upper and lower bound of the dynamic programming
			int upperBound = 0;
			int lowerBound = 0;

			for (size_t j = 0 ; j < taskList[i].size() ; j++)
			{
				upperBound += taskList[i][j].profileTable[k][taskList[i][j].profileTable[k].size() - 1].energy;
			}

			// schedualability constraint
			int constraint = deadline;
			int invalidTableEntry = constraint + 1000000;

			// define the storage space for dynamic programming
			vector<vector<int> > table;
			vector<vector<int> > solutionTable;

			table.resize(taskList[i].size());
			solutionTable.resize(taskList[i].size());

			for (size_t j = 0 ; j < table.size() ; j++)
			{
				table[j].resize(upperBound - lowerBound);
				solutionTable[j].resize(upperBound - lowerBound);

				for (size_t l = 0 ; l < table[j].size() ; l++)
				{
					table[j][l] = invalidTableEntry;
					solutionTable[j][l] = -1;
				}
			}

			// dynamic programming solver (using original energy value, a minimization version)
			// loop over all the tasks
			for (size_t j = 0 ; j < table.size() ; j++)
			{
				// consider only the first task, trivial
				if (j == 0)
				{
					for (int l = 0 ; l < upperBound - lowerBound ; l++)
					{
						// for all L1 configurations
						for (size_t m = 0 ; m < taskList[i][j].profileTable[k].size() ; m++)
						{
							//printf("		Task %d , l = %d , L1 config %d ... \n ",j,l,m);

							if (taskList[i][j].profileTable[k][m].energy == l + lowerBound)
							{
								if (taskList[i][j].profileTable[k][m].time < table[j][l])
								{
									table[j][l] = taskList[i][j].profileTable[k][m].time;
									solutionTable[j][l] = m;
								}
							}
						}
					}
				}
				else
				{
					// loop over total energy from lowerBound to upperBound for every task
					for (int l = 0 ; l < upperBound - lowerBound ; l++)
					{
						// loop over all L1 configurations for task i,j with partition scheme k
						for (size_t m = 0 ; m < taskList[i][j].profileTable[k].size() ; m++)
						{
							//printf("		Task %d , l = %d , L1 config %d ... \n ",j,l,m);

							int energyValueLast = l + lowerBound - taskList[i][j].profileTable[k][m].energy;

							if (energyValueLast < 0)
								continue;

							if ((taskList[i][j].profileTable[k][m].time + table[j - 1][energyValueLast]) < table[j][l])
							{
								table[j][l] = taskList[i][j].profileTable[k][m].time + table[j - 1][energyValueLast];
								solutionTable[j][l] = m;
							}
						}
					}

					//for (int l = 0 ; l < upperBound - lowerBound ; l++)
					//{
					//	// loop over all L1 configurations for task i,j with partition scheme k
					//	for (size_t m = 0 ; m < taskList[i][j].profileTable[k].size() ; m++)
					//	{
					//		int energyValueThis = l + lowerBound + taskList[i][j].profileTable[k][m].energy;

					//		if (energyValueThis >= upperBound)
					//			continue;

					//		if ((taskList[i][j].profileTable[k][m].time + table[j - 1][l]) < table[j][energyValueThis])
					//		{
					//			table[j][energyValueThis] = taskList[i][j].profileTable[k][m].time + table[j - 1][l];
					//			solutionTable[j][energyValueThis] = m;
					//		}
					//	}
					//}
				}
			}

			// FOR DEBUG
			/*ofstream out;
			out.open("dump_solutionlist.txt");

			for (size_t j = 0 ; j < taskList[i].size() ; j++)
			{
				for (int l = 0 ; l < upperBound - lowerBound ; l++)
				{
					out << solutionTable[j][l] << ",";
				}
				out << endl;
			}

			out.close();*/

			// find the minimum energy consumption of all the tasks and with total time less than the deadline
			for (int l = 0 ; l < upperBound - lowerBound ; l++)
			{
				// the first one with valid schedulability condition is the one with minimum energy consumption
				if (table[table.size() - 1][l] <= constraint)
				{
					coreSolutionList[i][k].push_back(l);
					coreSolutionList[i][k].push_back(table[table.size() - 1][l]);
					
					// backtrack to get the solution for each task
					vector<int> tmp;
					tmp.resize(taskList[i].size());

					tmp[taskList[i].size() - 1] = solutionTable[taskList[i].size() - 1][l];

					int energyValueLast = l - taskList[i][taskList[i].size() - 1].profileTable[k][tmp[taskList[i].size() - 1]].energy;

					for (int j = taskList[i].size() - 2 ; j >= 0 ; j--)
					{
						tmp[j] = solutionTable[j][energyValueLast];
						energyValueLast -= taskList[i][j].profileTable[k][tmp[j]].energy;
					}

					// push the solution index (in the sorted profile table) list after Energy and Time
					for (size_t j = 0 ; j < taskList[i].size() ; j++)
					{
						// tmp[j] is the index in all L1 configs sorted by energy 
						// idx is the index in all L1 configs sorted by Index
						int idx = taskList[i][j].profileTable[k][tmp[j]].index;
						coreSolutionList[i][k].push_back(idx);
					}

					break;
				}
			}

			// no valid assignment for this partition factor
			if (coreSolutionList[i][k].size() == 0)
			{
				coreSolutionList[i][k].push_back(-1);
				coreSolutionList[i][k].push_back(-1);
				
				for (size_t j = 0 ; j < taskList[i].size() ; j++)
				{
					coreSolutionList[i][k].push_back(-1);
				}
			}
		}
	}

	printf("\n");

	// FOR DEBUG
	for (int i = 0 ; i < num_cores ; i++)
	{
		printf("Core %d: \n",i);
		for (int k = 1 ; k < l2Assoc ; k++)
		{
			printf("PF=%d: ",k);
			
			if (coreSolutionList[i][k][0] > -1)
			{
				printf("E=%d,T=%d [",coreSolutionList[i][k][0],coreSolutionList[i][k][1]);
				for (size_t j = 0 ; j < taskList[i].size() ; j++)
				{
					int idx = coreSolutionList[i][k][2 + j];
					printf(" %d:%s ", idx, taskList[i][j].profileTable[k][idx].L1configStr.c_str());
				}
				printf("] \n");
			}
			else
			{
				printf("E=%d,T=%d \n",coreSolutionList[i][k][0],coreSolutionList[i][k][1]);
			}
		}
		printf("\n");
	}
}

void MulticoreDCRSim::findSolution()
{
	int minEnergyDCRCP = std::numeric_limits<int>::max();
	int minEnergyTimeDCRCP = 0;
	int minEnergyVulDCRCP = 0;
	int minEnergyVulDCRCP_maxVul = 0;
	vector<int> minEnergyVulDCRCP_coreVul(num_cores);
	
	int minEnergyCP = std::numeric_limits<int>::max();
	int minEnergyTimeCP = 0;
	int minEnergyVulCP = 0;
	
	int energyDCR = 0;
	int timeDCR = 0;
	int vulDCR = 0;

	int partitionIdxDCRCP = 0;
	int partitionIdxCP = 0;

	// first restore the profile table order of each task
	for (int i = 0 ;i < num_cores ; i++)
	{
		for (size_t j = 0 ; j < taskList[i].size() ; j++)
		{
			for (int k = 1 ;k < l2Assoc ; k++)
			{
				sort(taskList[i][j].profileTable[k].begin(),taskList[i][j].profileTable[k].end(),ComparerIndex());
			}
		}
	}

	// *************** L1 DCR + L2 CP ***************
	// for all possible L2 partition schemes
	for (size_t i = 0 ; i < partitionList.size() ; i++)
	{
		int energy = 0;
		int time = 0;
		int vul = 0;
		int maxVul = 0;
		vector<int> coreVul(num_cores);
		
		bool isValid = true;

		for (int j = 0 ; j < num_cores ; j++)
		{
			// this core does not have a valid assignment for this partition factor
			if (coreSolutionList[j][partitionList[i][j]][0] == -1)
			{
				isValid = false;
				break;
			}
	
			int curCoreVul = 0;
			// energy is the sum over all cores
			energy += coreSolutionList[j][partitionList[i][j]][0];
			for (int a = 0; a < taskList[j].size(); a++) 
			{
				int PF = partitionList[i][j]; // coreSolutionList[j][PF][2~2+numTask]
				int L1config = coreSolutionList[j][PF][2+a];
				//printf("PF=%d, L1config=%d\n", PF, L1config);
				vul += 	taskList[j][a].profileTable[PF][L1config].vul;
				curCoreVul += taskList[j][a].profileTable[PF][L1config].vul;
			}
			
			// time is the max of all cores
			if (coreSolutionList[j][partitionList[i][j]][1] > time)
			{
				time = coreSolutionList[j][partitionList[i][j]][1];
			}
			maxVul = max(maxVul, curCoreVul);
			coreVul[j] = curCoreVul;
		}

		// ignore this partition scheme
		if (!isValid)
			continue;

		if (energy < minEnergyDCRCP)
		{
			minEnergyDCRCP = energy;
			minEnergyTimeDCRCP = time;
			minEnergyVulDCRCP = vul;
			minEnergyVulDCRCP_maxVul = maxVul;
			minEnergyVulDCRCP_coreVul= coreVul;
			coreVulConstraints = coreVul;
			partitionIdxDCRCP = i;
		}
	}

	printf ("Minimum energy for L1 DCR + L2 CP is %d, time is %d (using partition scheme %d): \n",minEnergyDCRCP,minEnergyTimeDCRCP,partitionIdxDCRCP);
	for (int i = 0 ; i < num_cores ; i++)
	{
		printf(" Core %d use PF = %d: ",i,partitionList[partitionIdxDCRCP][i]);

		// each task's L1 config assignment
		for (size_t j = 0 ; j < taskList[i].size() ; j++)
		{
			int pf = partitionList[partitionIdxDCRCP][i];
			int idx = coreSolutionList[i][pf][2 + j];
			printf(" %d:%s ", idx, taskList[i][j].profileTable[pf][idx].L1configStr.c_str());
		}

		printf(" \n");
	}
	printf(" \n");

	// *************** L1 DCR + Uniform CP ***************
	bool isValid = true;
	for (int i = 0 ; i < num_cores ; i++)
	{
		// this core does not have a valid assignment for this partition factor
		if (coreSolutionList[i][2][0] == -1)
		{
			isValid = false;
			break;
		}

		// energy is the sum over all cores
		energyDCR += coreSolutionList[i][2][0];
		for (int a = 0; a < taskList[i].size(); a++) 
		{
			int PF = 2; 
			int L1config = coreSolutionList[i][2][2+a];
			//printf("PF=%d, L1config=%d\n", PF, L1config);
			vulDCR += 	taskList[i][a].profileTable[PF][L1config].vul;
		}
		
		// time is the max of all cores
		if (coreSolutionList[i][2][1] > timeDCR)
		{
			timeDCR = coreSolutionList[i][2][1];
		}
	}
	
	if (isValid)
	{
		printf ("Minimum energy for L1 DCR + Uniform L2 CP is %d, time is %d: \n",energyDCR,timeDCR);
		for (int i = 0 ; i < num_cores ; i++)
		{
			printf(" Core %d use PF = %d: ",i,2);

			// each task's L1 config assignment
			for (size_t j = 0 ; j < taskList[i].size() ; j++)
			{
				int pf = 2;
				int idx = coreSolutionList[i][pf][2 + j];
				printf(" %d:%s ",idx, taskList[i][j].profileTable[pf][idx].L1configStr.c_str());
			}

			printf(" \n");
		}
		printf(" \n");
	}

	// *************** L2 CP only ***************
	// first restore the profile table order of each task
	/* already sorted by Index, so comment out
	for (int i = 0 ;i < num_cores ; i++)
	{
		for (size_t j = 0 ; j < taskList[i].size() ; j++)
		{
			for (int k = 1 ;k < l2Assoc ; k++)
			{
				sort(taskList[i][j].profileTable[k].begin(),taskList[i][j].profileTable[k].end(),ComparerIndex());
			}
		}
	}
	*/

	// for all possible L2 partition schemes
	for (size_t i = 0 ; i < partitionList.size() ; i++)
	{
		int energy = 0;
		int time = 0;
		int vul = 0;
		for (int j = 0 ; j < num_cores ; j++)
		{
			int curTime = 0;
			
			for (size_t k = 0 ; k < taskList[j].size() ; k++)
			{
				energy += taskList[j][k].profileTable[partitionList[i][j]][baseL1Index].energy;
				curTime += taskList[j][k].profileTable[partitionList[i][j]][baseL1Index].time;
				vul += taskList[j][k].profileTable[partitionList[i][j]][baseL1Index].vul;
			}

			if (curTime > time)
			{
				time = curTime;
			}
		}

		if (time > deadline)
			continue;

		if (energy < minEnergyCP)
		{
			minEnergyCP = energy;
			minEnergyTimeCP = time;
			minEnergyVulCP = vul;
			partitionIdxCP = i;
		}
	}

	printf ("Minimum energy for L2 CP only is %d, time is %d (using partition scheme %d): \n",minEnergyCP,minEnergyTimeCP,partitionIdxCP);
	for (int i = 0 ; i < num_cores ; i++)
	{
		printf(" Core %d use PF = %d, with base L1 %d \n",i,partitionList[partitionIdxCP][i],baseL1Index);
	}
	printf(" \n");
	printf("The Energy Improvement is %f \n",(minEnergyCP - minEnergyDCRCP) /(double) minEnergyCP);

	// dump results to file
	char filename[50];
	sprintf(filename,"results_tm.csv");

	ofstream out;
	out.open(filename,ios::app);
	/*for (size_t i = 0 ; i < taskList.size() ; i++)
	{
		out << "([" << partitionList[partitionIdxDCRCP][i] << "]";
		for (size_t j = 0 ; j < taskList[i].size() ; j++)
		{
			out << taskList[i][j].name << " ";
		}
		out << minEnergyVulDCRCP_coreVul[i] << "),";
	}*/

	//out << deadline << "," << minEnergyCP << "," << minEnergyDCRCP << "," << endl;
	out << deadline << "," << minEnergyCP << "," << minEnergyDCRCP << "," << endl;
	
	out.close();
	
	
	printf("Method, Energy, Time, deadline=%d\n", deadline);
	printf("CP, %d, %d\n", minEnergyCP, minEnergyTimeCP);
	printf("DCR(Base), %d, %d\n", energyDCR, timeDCR);
	printf("DCR+CP, %d, %d\n\n\n", minEnergyDCRCP, minEnergyTimeDCRCP);
	ofstream outAll;
	outAll.open("allSets.csv",ios::app);
	outAll << "Energy, Time, deadline=" << deadline << endl;
	outAll << "The Energy Improvement(DCR+CP) to CP is " << (minEnergyCP - minEnergyDCRCP) /(double) minEnergyCP << endl;
	outAll << "CP," << minEnergyCP <<"," << minEnergyTimeCP << "," << endl;
	outAll << "DCR(Base)," << energyDCR <<"," << timeDCR <<"," << endl;
	outAll << "DCR+CP," << minEnergyDCRCP <<"," << minEnergyTimeDCRCP <<"," << endl <<endl;
}




int MulticoreDCRSim::randomNumberGenerater(int range_min, int range_max)
{
	return (int)((float) rand() / (RAND_MAX + 1) * (range_max - range_min) + 0.5f) + range_min;
}

void MulticoreDCRSim::dumpPartitionList()
{
	for (size_t i = 0 ; i < partitionList.size() ; i++)
	{
		for (size_t j = 0 ; j < partitionList[i].size() ; j++)
		{
			printf("%d ",partitionList[i][j]);
		}

		printf("\n");
	}
	printf("partitionList.size() = %d \n",partitionList.size());
}

void MulticoreDCRSim::dumpProfileTables()
{
	// for each core
	for (int i = 0 ; i < num_cores ; i++)
	{
		// for each task
		for (size_t j = 0 ; j < taskList[i].size() ; j++)
		{
			char filename[50];
			sprintf(filename,"%s_dump.csv",taskList[i][j].name.c_str());

			ofstream out;
			out.open(filename);

			// for each L1 configuration combination
			for (int m = 0 ; m < TOTAL_CONFIG_CNT ; m++)
			{
				out << taskList[i][j].profileTable[1][m].L1configStr << ",";
				// for each L2 partition factor
				for (int k = 1 ; k < l2Assoc ; k++)
				{
					out << taskList[i][j].profileTable[k][m].energy << "," << taskList[i][j].profileTable[k][m].time << ",";
				}
				out << endl;
			}

			out.close();
		}
	}
}

void MulticoreDCRSim::dumpBaseCases()
{
	// for each core
	for (int i = 0 ; i < num_cores ; i++)
	{
		int totalTime = 0;
		vector<int> tmp;

		// for each task
		for (size_t j = 0 ; j < taskList[i].size() ; j++)
		{
			totalTime += taskList[i][j].profileTable[l2Assoc / num_cores][baseL1Index].time;
			tmp.push_back(taskList[i][j].profileTable[l2Assoc / num_cores][baseL1Index].time);

			/*cout << taskList[i][j].name << ": " << taskList[i][j].profileTable[l2Assoc / num_cores][baseL1Index].energy << " , " 
				<< taskList[i][j].profileTable[l2Assoc / num_cores][baseL1Index].time << endl;*/
		}

		printf("Core %d: %d [",i,totalTime);
		for (size_t j = 0; j < tmp.size() ; j++)
			printf("%d ",tmp[j]);
		printf("] \n");
	}
}

void MulticoreDCRSim::setDeadline(int inDeadline)
{
	deadline = inDeadline;
	printf("Deadline set to: %d\n", deadline);
}

int MulticoreDCRSim::getL2Assoc()
{
	return l2Assoc;
}

void MulticoreDCRSim::findOptimalPartition(double threshold)
{
	// for all tasks
	for (int i = 0 ; i < num_cores ; i++)
	{
		for (size_t j = 0 ; j < taskList[i].size() ; j++)
		{
			vector<int> tmpList;

			// for all L1 configurations
			for (size_t m = 0 ; m < TOTAL_CONFIG_CNT ; m++)
			{
				double improvement = 0.0;
				int optPF = l2Assoc - 1;

				// for all L2 PFs
				for (int k = 2 ; k < l2Assoc ; k++)
				{
					improvement = (taskList[i][j].profileTable[k - 1][m].time - taskList[i][j].profileTable[k][m].time) /(double) taskList[i][j].profileTable[k - 1][m].time;

					if (improvement <= threshold)
					{
						optPF = k - 1;
						break;
					}
				}

				tmpList.push_back(optPF);
			}

			optimalPartitionList.push_back(tmpList);
		}
	}
}

void MulticoreDCRSim::dumpOptimalPartitionList()
{
	char filename[50];
	sprintf(filename,"opt_pf_list.csv");

	ofstream out;
	out.open(filename);

	int cnt = 0;

	cout << "****************************************************************************" << endl;

	// for all tasks
	for (int i = 0 ; i < num_cores ; i++)
	{
		for (size_t j = 0 ; j < taskList[i].size() ; j++)
		{
			out << taskList[i][j].name << ",";

			// for all L1 configurations
			for (size_t m = 0 ; m < TOTAL_CONFIG_CNT ; m++)
			{
				out << optimalPartitionList[cnt][m] << ",";

				if (m == baseL1Index)
				{
					cout << taskList[i][j].name << " opt pf: " << optimalPartitionList[cnt][m] << endl;
				}
			}

			out << endl;

			cnt++;
		}
	}

	cout << "****************************************************************************" << endl;

	out.close();
}

int MulticoreDCRSim::readTaskInfoAll(string fileName)
{
	char line[MAX_LENGTH];

	ifstream in(fileName.c_str(),ios::in);

	if (!in)
	{
		printf("Cannot open file %s!",fileName.c_str());
		return 1;
	}

	in.getline(line,MAX_LENGTH);
	sscanf(line,"%d",&num_cores);

	in.getline(line,MAX_LENGTH);
	sscanf(line,"%d",&l2Assoc);

	in.getline(line,MAX_LENGTH);
	sscanf(line,"%d",&baseL1Index);

	// prepare taskList, solutionList sizes
	taskList.resize(num_cores);
	solutionList.resize(num_cores);

	int id = 0;

	in.getline(line,MAX_LENGTH,',');
	sscanf(line,"%d",&num_tasks);

	// solutionList[i].resize(num_tasks);

	for (int i = 0 ; i < num_tasks ; i++)
	{
		in.getline(line,MAX_LENGTH,',');
		char inName[MAX_LENGTH];

		sscanf(line,"%s",&inName);

		taskListAll.push_back(Task(id,inName));
		id++;

		// solutionList[i][j].resize(2);
	}

	in.getline(line,MAX_LENGTH,'\n');


	in.getline(line,MAX_LENGTH,',');
	sscanf(line,"%d",&deadline);

	// FOR DEBUG
	printf("There are %d cores \n",num_cores);
	printf("L2 cache associativity is %d \n",l2Assoc);
	printf("There are totally %d tasks \n",num_tasks);
	for (int i = 0 ; i < num_tasks ; i++)
	{
		printf("%s , ",taskListAll[i].name.c_str());
	}
	printf("\n");
	printf("Deadline is %d \n\n",deadline);

	return 0;
}

int MulticoreDCRSim::loadTaskProfileTableAll(string path)
{
	// for each core
	for (int i = 0 ; i < num_tasks ; i++)
	{
		char line[MAX_LENGTH];
		float inEnergy;
		float inTime;
		char inL1configStr[MAX_LENGTH];
		char fileName[MAX_LENGTH];
		sprintf(fileName,"%s/%s.csv",path.c_str(),taskListAll[i].name.c_str());

		// 0 is not used
		taskListAll[i].profileTable.resize(l2Assoc);
		for (int k = 0 ; k < l2Assoc ; k++)
		{
			taskListAll[i].profileTable[k].resize(TOTAL_CONFIG_CNT);
		}

		ifstream in(fileName,ios::in);

		if (!in)
		{
			printf("Cannot open file %s!",fileName);
			return 1;
		}

		// for each L1 configuration
		for (int l = 0 ; l < TOTAL_CONFIG_CNT ; l++)
		{
			in.getline(line,MAX_LENGTH,',');
			sscanf(line,"%s",&inL1configStr);

			// for each L2 partition factor
			for (int k = 1 ; k < l2Assoc ; k++)
			{
				// for energy and time
				in.getline(line,MAX_LENGTH,',');
				sscanf(line,"%f",&inEnergy);
				in.getline(line,MAX_LENGTH,',');
				sscanf(line,"%f",&inTime);

				taskListAll[i].profileTable[k][l].L1configStr = inL1configStr;
				taskListAll[i].profileTable[k][l].index = l;
				taskListAll[i].profileTable[k][l].energy = (int)(inEnergy /(double) ENERGY_UNIT_SCALE);
				taskListAll[i].profileTable[k][l].time = (int)(inTime /(double) TIME_UNIT_SCALE);
			}
		}
	}

	return 0;
}

void MulticoreDCRSim::generateAllTaskMapping()
{
	vector<vector<int> > taskAllocationList;
	vector<vector<int> > taskMappingListTmp;
	vector<int> current;

	current.resize(num_cores);

	for (int i = 0 ; i < num_cores ; i++)
		current[i] = 0;

	generateAllTaskMappingHelperHelper(taskAllocationList,current,0,0);

	// -------FOR DEBUG--------
	vector<int> tmp(taskAllocationList[taskAllocationList.size() - 1]);
	taskAllocationList.clear();
	taskAllocationList.push_back(tmp);
	// ------------------------

	// FOR DEBUG
	for (size_t i = 0 ; i < taskAllocationList.size() ; i++)
	{
		for (size_t j = 0 ; j < taskAllocationList[i].size() ; j++)
		{
			printf("%d ",taskAllocationList[i][j]);
		}
		printf("\n");
	}

	current.clear();
	current.resize(num_tasks);

	for (int i = 0 ; i < num_tasks ; i++)
		current[i] = -1;

	generateAllTaskMappingHelper(taskMappingListTmp,taskAllocationList,current,0,0);

	// FOR DEBUG
	printf("Before post-processing: %d \n",taskMappingListTmp.size());
	/*for (size_t i = 0 ; i < taskMappingListTmp.size() ; i++)
	{
		for (size_t j = 0 ; j < taskMappingListTmp[i].size() ; j++)
		{
			printf("%d ",taskMappingListTmp[i][j]);
		}
		printf("\n");
	}*/

	// post-processing: remove duplicate task mapping (e.g., 1 1 0 0 and 0 0 1 1 will lead to the same result since all cores are homonegous)
	for (size_t i = 0 ; i < taskMappingListTmp.size() ; i++)
	{
		if (taskMappingListTmp[i][0] == -100)
			continue;

		taskMappingListAll.push_back(taskMappingListTmp[i]);

		vector<vector<int> > signature;
		signature.resize(num_cores);

		for (size_t k = 0 ; k < taskMappingListTmp[i].size() ; k++)
		{
			signature[taskMappingListTmp[i][k]].push_back(k);
		}

		// mark all duplicate entries
		for (size_t j = i + 1 ; j < taskMappingListTmp.size(); j ++)
		{
			if (taskMappingListTmp[j][0] == -100)
				continue;

			vector<vector<int> > signatureTmp;
			signatureTmp.resize(num_cores);

			for (size_t k = 0 ; k < taskMappingListTmp[j].size() ; k++)
			{
				signatureTmp[taskMappingListTmp[j][k]].push_back(k);
			}

			// compare signatureTmp with signature
			bool isSigIdentical = true;

			for (size_t k = 0 ; k < signatureTmp.size() ; k++)
			{
				// find whether signatureTmp[k] is in signature
				bool found = false;

				for (size_t h = 0 ; h < signature.size() ; h++)
				{
					if (signatureTmp[k].size() != signature[h].size())
						continue;

					bool isIdentical = true;

					for (size_t m = 0 ; m < signature[h].size() ; m++)
					{
						if (signatureTmp[k][m] != signature[h][m])
						{
							isIdentical = false;
							break;
						}
					}

					if (isIdentical)
					{
						found = true;
						break;
					}
				}

				if (!found)
					isSigIdentical = false;
			}

			if (isSigIdentical)
				taskMappingListTmp[j][0] = -100;
		}
	}

	// FOR DEBUG
	printf("After post-processing: %d \n",taskMappingListAll.size());
	/*for (size_t i = 0 ; i < taskMappingListAll.size() ; i++)
	{
		for (size_t j = 0 ; j < taskMappingListAll[i].size() ; j++)
		{
			printf("%d ",taskMappingListAll[i][j]);
		}
		printf("\n");
	}*/
}

void MulticoreDCRSim::generateAllTaskMappingHelper(vector<vector<int> >& taskMappingList, vector<vector<int> > taskAllocationList, vector<int>& current, int core, int idx)
{
	if (core == num_cores)
	{
		taskMappingList.push_back(current);
		return;
	}

	if (core == 0)
	{
		// for all possible task allocation (numbers)
		for (size_t n = 0 ; n < taskAllocationList.size() ; n++)
		{
			int num = taskAllocationList[n][core];
			vector<int> taskList;

			// for all un-assigned tasks
			for (int i = 0 ; i < num_tasks ; i++)
			{
				if (current[i] < 0)
					taskList.push_back(i);
			}

			// all combinations of num of tasks out of all un-assigned tasks
			vector<vector<int> > tmp = combinationHelper(taskList,num);

			for (size_t i = 0 ; i < tmp.size() ; i++)
			{
				for (size_t j = 0 ; j < tmp[i].size() ; j++)
					current[tmp[i][j]] = core;

				generateAllTaskMappingHelper(taskMappingList,taskAllocationList,current,core + 1,n);

				for (size_t j = 0 ; j < tmp[i].size() ; j++)
					current[tmp[i][j]] = -1;
			}
		}
	}
	else
	{
		int num = taskAllocationList[idx][core];
		vector<int> taskList;

		// for all un-assigned tasks
		for (int i = 0 ; i < num_tasks ; i++)
		{
			if (current[i] < 0)
				taskList.push_back(i);
		}

		// all combinations of num of tasks out of all un-assigned tasks
		vector<vector<int> > tmp = combinationHelper(taskList,num);

		for (size_t i = 0 ; i < tmp.size() ; i++)
		{
			for (size_t j = 0 ; j < tmp[i].size() ; j++)
				current[tmp[i][j]] = core;

			generateAllTaskMappingHelper(taskMappingList,taskAllocationList,current,core + 1,idx);

			for (size_t j = 0 ; j < tmp[i].size() ; j++)
				current[tmp[i][j]] = -1;
		}
	}
}

void MulticoreDCRSim::generateAllTaskMappingHelperHelper(vector<vector<int> >& taskAllocationList, vector<int>& current, int core, int cnt)
{
	if (core == num_cores)
	{
		int sum = 0;

		for (size_t i = 0 ; i < current.size() ; i++)
			sum += current[i];

		if (sum == num_tasks)
			taskAllocationList.push_back(current);

		return;
	}

	for (int i = 0 ; i < num_tasks - num_cores + 1 ; i++)
	{
		// there is not enough task left for the rest cores
		if (i + 1 > num_tasks - cnt - (num_cores - (core + 1)))
			continue;

		bool isValid = true;
		for (int j = 0 ; j < core ; j++)
		{
			if (i + 1 < current[j])
				isValid = false;
		}

		if (!isValid)
			continue;

		current[core] = i + 1;

		generateAllTaskMappingHelperHelper(taskAllocationList,current,core + 1,cnt + i + 1);
	}
}

vector<vector<int> > MulticoreDCRSim::combinationHelper(vector<int> taskList, int n)
{
	vector<vector<int > > res;
	vector<bool> current;

	current.resize(taskList.size());
	for (size_t i = 0 ; i < current.size() ; i++)
		current[i] = false;

	combinationHelperHelper(res,current,0,0,n);

	for (size_t i = 0 ; i < res.size() ; i++)
	{
		for (size_t j = 0 ; j < res[i].size() ; j++)
		{
			res[i][j] = taskList[res[i][j] - 1];
		}
	}

	return res;
}

void MulticoreDCRSim::combinationHelperHelper(vector<vector<int> >& res, vector<bool>& current, int start, int c, int n)
{
	if (c == n)
	{
		vector<int> tmp;
		for (size_t i = 0 ; i < current.size() ; i++)
		{
			if (current[i])
				tmp.push_back(i + 1);
		}
		res.push_back(tmp);
		return;
	}

	for (size_t i = start ; i < current.size() ; i++)
	{
		current[i] = true;
		combinationHelperHelper(res,current,i + 1,c + 1,n);
		current[i] = false;
	}
}

void MulticoreDCRSim::exploreAllTaskMapping()
{
	for (size_t i = 0 ; i < taskMappingListAll.size() ; i++)
	{
		taskList.clear();
		taskList.resize(num_cores);

		for (int j = 0 ; j < num_tasks ; j++)
		{
			taskList[taskMappingListAll[i][j]].push_back(taskListAll[j]);
		}

		dynamicProgrammingSolver();
		generateL2PartitionList(false);
		findSolution();
	}
}

int MulticoreDCRSim::loadTaskProfileTablePhase(string path)
{
	// for each core
	for (int i = 0 ; i < num_cores ; i++)
	{
		// for each task
		for (size_t j = 0 ; j < taskList[i].size() ; j++)
		{
			char line[MAX_LENGTH];
			int inPhaseNum;
			float inEnergy;
			float inTime;
			int inInst;
			//char inL1configStr[MAX_LENGTH];
			char fileName[MAX_LENGTH];
			sprintf(fileName,"%s/%s.csv",path.c_str(),taskList[i][j].name.c_str());

			// 0 is not used
			taskList[i][j].profileTablePhase.resize(l2Assoc);
			for (int k = 0 ; k < l2Assoc ; k++)
			{
				taskList[i][j].profileTablePhase[k].resize(TOTAL_CONFIG_CNT);
			}

			ifstream in(fileName,ios::in);

			if (!in)
			{
				printf("Cannot open file %s!",fileName);
				return 1;
			}

			// for each L1 configuration
			for (int l = 0 ; l < TOTAL_CONFIG_CNT ; l++)
			{
				in.getline(line,MAX_LENGTH,',');
				//sscanf(line,"%s",&inL1configStr);

				// for each L2 partition factor
				for (int k = 1 ; k < l2Assoc ; k++)
				{
					in.getline(line,MAX_LENGTH,',');
					sscanf(line,"%d",&inPhaseNum);

					taskList[i][j].profileTablePhase[k][l].resize(inPhaseNum + 1);

					//taskList[i][j].profileTablePhase[k][l][0].L1configStr = "NULL";
					//taskList[i][j].profileTablePhase[k][l][0].index = l;
					//taskList[i][j].profileTablePhase[k][l][0].phase = 0;
					taskList[i][j].profileTablePhase[k][l][0].energy = 0;
					taskList[i][j].profileTablePhase[k][l][0].time = 0;
					taskList[i][j].profileTablePhase[k][l][0].inst = 0;

					// for each phase
					for (int m = 1 ; m <= inPhaseNum ; m++)
					{
						// for energy and time
						in.getline(line,MAX_LENGTH,',');
						sscanf(line,"%f",&inEnergy);
						in.getline(line,MAX_LENGTH,',');
						sscanf(line,"%f",&inTime);
						in.getline(line,MAX_LENGTH,',');
						sscanf(line,"%d",&inInst);

						//taskList[i][j].profileTablePhase[k][l][m].L1configStr = inL1configStr;
						//taskList[i][j].profileTablePhase[k][l][m].index = l;
						//taskList[i][j].profileTablePhase[k][l][m].phase = m;
						taskList[i][j].profileTablePhase[k][l][m].energy = (int)(inEnergy /(double) ENERGY_UNIT_SCALE);
						taskList[i][j].profileTablePhase[k][l][m].time = (int)(inTime /(double) TIME_UNIT_SCALE);
						taskList[i][j].profileTablePhase[k][l][m].inst = inInst;
					}
				}
			}
		}
	}

	return 0;
}

void MulticoreDCRSim::varyingPartitionSolverBF(int num_CPpoint,vector<double> CPpointList)
{
	printf("varyingPartitionSolverBF(...) now working ... \n");

	// build up varying CP point list
	for (int i = 0 ; i < num_CPpoint ; i++)
	{
		varyingCPList.push_back(((deadline / phase) * CPpointList[i]) * phase);
		printf("varyingCPList[%d] = %d \n",i,varyingCPList[i]);
	}

	// list of all possible partition factor combinations
	vector<vector<int> > partitionFactorSetList;

	// lists of all possible L1 cache configuration combinations
	vector<vector<vector<int> > > L1cacheCombinationList;

	generateAllPartitionFactorSetHelper(partitionFactorSetList);

	L1cacheCombinationList.resize(num_cores);

	for (int i = 0 ; i < num_cores ; i++)
	{
		generateAllL1ConfigurationSetHelper(L1cacheCombinationList[i],i);
	}

	// prepare coreSolutionList
	coreSolutionList.clear();
	coreSolutionList.resize(num_cores);

	for (int i = 0 ; i < num_cores ; i++)
	{
		coreSolutionList[i].resize(partitionFactorSetList.size());
	}

	// for each core [use brute-force, for now]
	for (int i = 0 ; i < num_cores ; i++)
	{
		// for each L2 partition factor set 
		for (size_t j = 0 ; j < partitionFactorSetList.size() ; j++)
		{
			printf("Core %d, PFset %d: ",i,j);
			for (size_t k = 0 ; k < partitionFactorSetList[j].size() ; k++)
			{
				printf("%d ",partitionFactorSetList[j][k]);
			}

			// ************************************************************************************
			// Find the L1 cache configuration combination with min E while satisfying D
			// ************************************************************************************
			int minEnergy = INT_MAX;
			int minEnergyIdx = -1;
			int minEnergyTime = -1;

			// for all possible L1 cache configuration combinations
			for (size_t k = 0 ; k < L1cacheCombinationList[i].size() ; k++)
			{
				bool isValid = true;
				int totalEnergy = 0;
				int currentInterval = 0;
				int lastEndTime = 0;

				/*if (i == 2 && j == 19 && k == 28728)
				{
					for (size_t m = 0 ; m < L1cacheCombinationList[i][k].size() ; m++)
					{
						printf(" %d ",L1cacheCombinationList[i][k][m]);
					}
					totalEnergy = 0;
				}*/

				// for each task
				for (size_t m = 0 ; m < taskList[i].size() ; m++)
				{
					// Now, for this task, L1 configuration is fixed, L2 PF is decided by the interval
					int pf = partitionFactorSetList[j][currentInterval];						// L2 partition factor for current interval
					int l1config = L1cacheCombinationList[i][k][m];								// L1 configuration index for this task
					int lastIdx = taskList[i][m].profileTablePhase[pf][l1config].size() - 1;		// profile table entry index for this task (in entirety)

					int currentEndPhase = 0;
					int nextStartPhase = 0;
					double partialPhaseLeftPct(0.0);
					double partialPhaseNewPct(0.0);
					int dynInstFinished = 0;
					int remainingTime = 0;
					bool hasMultipleCP = false;

					// if this task starts but does not end in the current interval
					if (currentInterval < varyingCPList.size() && lastEndTime + taskList[i][m].profileTablePhase[pf][l1config][lastIdx].time > varyingCPList[currentInterval])
					{
						hasMultipleCP = true;

						// this is the first new interval, the CP point aligns the ending phase of current CP interval
						currentEndPhase = (varyingCPList[currentInterval] - lastEndTime) / phase;
						dynInstFinished = taskList[i][m].profileTablePhase[pf][l1config][currentEndPhase].inst;

						// calculate the energy consumed until now
						totalEnergy += taskList[i][m].profileTablePhase[pf][l1config][currentEndPhase].energy;

						lastEndTime = varyingCPList[currentInterval];
						currentInterval++;
						pf = partitionFactorSetList[j][currentInterval];

						// calculate the corresponding phase in the new CP interval based on progress (i.e., dyn. inst.)
						for (size_t l = 0 ; l < taskList[i][m].profileTablePhase[pf][l1config].size() ; l++)
						{
							if (taskList[i][m].profileTablePhase[pf][l1config][l].inst > dynInstFinished)
							{
								nextStartPhase = l;
								break;
							}
						}

						// this simulation result is invalid
						if (nextStartPhase == 0)
						{
							isValid = false;
							break;
						}

						// calculate the remaining execution time in the new CP interval (assuming uniform distribution of the partial first phase)
						partialPhaseLeftPct = (taskList[i][m].profileTablePhase[pf][l1config][nextStartPhase].inst - dynInstFinished) /(double) 
										      (taskList[i][m].profileTablePhase[pf][l1config][nextStartPhase].inst - taskList[i][m].profileTablePhase[pf][l1config][nextStartPhase - 1].inst);

						lastIdx = taskList[i][m].profileTablePhase[pf][l1config].size() - 1;

						remainingTime = phase * partialPhaseLeftPct + 
							            (taskList[i][m].profileTablePhase[pf][l1config][lastIdx].time - taskList[i][m].profileTablePhase[pf][l1config][nextStartPhase].time);

						// decide in which interval it ends
						while (currentInterval < varyingCPList.size() && lastEndTime + remainingTime > varyingCPList[currentInterval])
						{
							// the CP point does not align the ending phase of current CP interval
							currentEndPhase = nextStartPhase + (varyingCPList[currentInterval] - lastEndTime) / phase;
							partialPhaseNewPct = (varyingCPList[currentInterval] - taskList[i][m].profileTablePhase[pf][l1config][currentEndPhase - 1].time) /(double) phase;
							dynInstFinished = taskList[i][m].profileTablePhase[pf][l1config][currentEndPhase - 1].inst + 
								              partialPhaseNewPct * (taskList[i][m].profileTablePhase[pf][l1config][currentEndPhase].inst - 
											                        taskList[i][m].profileTablePhase[pf][l1config][currentEndPhase - 1].inst);

							// calculate the energy consumed until now
							totalEnergy += partialPhaseLeftPct * (taskList[i][m].profileTablePhase[pf][l1config][nextStartPhase].energy - 
								                                  taskList[i][m].profileTablePhase[pf][l1config][nextStartPhase - 1].energy) 
												+ 
										                     (taskList[i][m].profileTablePhase[pf][l1config][currentEndPhase - 1].energy - 
										                      taskList[i][m].profileTablePhase[pf][l1config][nextStartPhase].energy) 
												+ 
										   partialPhaseNewPct * (taskList[i][m].profileTablePhase[pf][l1config][currentEndPhase].energy -
										                         taskList[i][m].profileTablePhase[pf][l1config][currentEndPhase - 1].energy);

							lastEndTime = varyingCPList[currentInterval];
							currentInterval++;
							pf = partitionFactorSetList[j][currentInterval];

							// calculate the corresponding phase in the new CP interval based on progress (i.e., dyn. inst.)
							for (size_t l = 0 ; l < taskList[i][m].profileTablePhase[pf][l1config].size() ; l++)
							{
								if (taskList[i][m].profileTablePhase[pf][l1config][l].inst > dynInstFinished)
								{
									nextStartPhase = l;
									break;
								}
							}

							// calculate the remaining execution time in the new CP interval (assuming uniform distribution of the partial first phase)
							partialPhaseLeftPct = (taskList[i][m].profileTablePhase[pf][l1config][nextStartPhase].inst - dynInstFinished) /(double) 
								                  (taskList[i][m].profileTablePhase[pf][l1config][nextStartPhase].inst - taskList[i][m].profileTablePhase[pf][l1config][nextStartPhase - 1].inst);

							lastIdx = taskList[i][m].profileTablePhase[pf][l1config].size() - 1;

							remainingTime = phase * partialPhaseLeftPct + 
								            (taskList[i][m].profileTablePhase[pf][l1config][lastIdx].time - taskList[i][m].profileTablePhase[pf][l1config][nextStartPhase].time);
						}
					}

					// this task now ends in the current intervel
					// calculate the energy consumed
					if (hasMultipleCP)
					{
						lastIdx = taskList[i][m].profileTablePhase[pf][l1config].size() - 1;

						totalEnergy += partialPhaseLeftPct * (taskList[i][m].profileTablePhase[pf][l1config][nextStartPhase].energy - 
							                                  taskList[i][m].profileTablePhase[pf][l1config][nextStartPhase - 1].energy) 
											+ 
							                                 (taskList[i][m].profileTablePhase[pf][l1config][lastIdx].energy - 
							                                  taskList[i][m].profileTablePhase[pf][l1config][nextStartPhase].energy);

						// NOTE: align the start time of next task to the very next phase's start time
						//int temp = lastEndTime + remainingTime;
						lastEndTime = phase * ((lastEndTime + remainingTime) / phase + 1);

						// FOR DEBUG
						//printf("lastEndTime = %d (%d) ... ",lastEndTime,temp);
					}
					else
					{
						totalEnergy += taskList[i][m].profileTablePhase[pf][l1config][lastIdx].energy;

						// NOTE: align the start time of next task to the very next phase's start time
						//int temp = lastEndTime + taskList[i][m].profileTablePhase[pf][l1config][lastIdx].time;
						lastEndTime = phase * ((lastEndTime + taskList[i][m].profileTablePhase[pf][l1config][lastIdx].time) / phase + 1);

						// FOR DEBUG
						//printf("lastEndTime = %d (%d) ... ",lastEndTime,temp);
					}
				}

				// update the minimum energy for this L2 partition factor set (iff. the simulation results are valid && deadline is satisfied)
				if (isValid && lastEndTime <= deadline && totalEnergy <= minEnergy)
				{
					minEnergy = totalEnergy;
					minEnergyIdx = k;
					minEnergyTime = lastEndTime;
				}
			}

			printf(" minE = %d , minETTime = %d , minEIdx = %d (",minEnergy,minEnergyTime,minEnergyIdx);

			for (size_t k = 0 ; k < L1cacheCombinationList[i][minEnergyIdx].size() ; k++)
				printf("%d ",L1cacheCombinationList[i][minEnergyIdx][k]);

			printf(") ");
			printf("\n");

			// find the optimal solution for this L2 partition factor set, of this core
			coreSolutionList[i][j].push_back(minEnergy);
			coreSolutionList[i][j].push_back(minEnergyTime);
			coreSolutionList[i][j].push_back(minEnergyIdx);
		}// END OF j: for each L2 partition factor list
	}// END OF i: for each core

	// FOR DEBUG
	// for each core
	//for (int i = 0 ; i < num_cores ; i++)
	//{
	//	printf("Core %d : \n",i);
	//	// for each L2 partition factor set 
	//	for (size_t j = 0 ; j < partitionFactorSetList.size() ; j++)
	//	{
	//		printf("(%d,%d,%d),",coreSolutionList[i][j][0],coreSolutionList[i][j][1],coreSolutionList[i][j][2]);
	//	}
	//	printf("\n");
	//}
}

void MulticoreDCRSim::varyingPartitionSolverDP(int num_CPpoint,vector<double> CPpointList)
{
	printf("varyingPartitionSolverDP(...) now working ... \n");

	// build up varying CP point list
	for (int i = 0 ; i < num_CPpoint ; i++)
	{
		varyingCPList.push_back(((deadline / phase) * CPpointList[i]) * phase);
		printf("varyingCPList[%d] = %d \n",i,varyingCPList[i]);
	}

	// list of all possible L2 partition factor combinations
	vector<vector<int> > partitionFactorSetList;

	generateAllPartitionFactorSetHelper(partitionFactorSetList);

	// prepare coreSolutionList
	coreSolutionList.clear();
	coreSolutionList.resize(num_cores);

	for (int i = 0 ; i < num_cores ; i++)
	{
		coreSolutionList[i].resize(partitionFactorSetList.size());
	}

	// for each core [use Dynamic Programming]
	for (int i = 0 ; i < num_cores ; i++)
	{
		// get the upper and lower bound of the dynamic programming [for all iterations of this core]
		int upperBound = 0;
		int lowerBound = 0;

		vector<vector<int> > numbers;
		numbers.resize(taskList[i].size());

		for (size_t j = 0 ; j < taskList[i].size() ; j++)
		{
			for (size_t k = 0 ; k < taskList[i][j].profileTablePhase[l2Assoc - 1].size() ; k++)
			{
				int len = taskList[i][j].profileTablePhase[l2Assoc - 1][k].size();
				int tmp = taskList[i][j].profileTablePhase[l2Assoc - 1][k][len - 1].energy;
				numbers[j].push_back(tmp);
			}

			sort(numbers[j].begin(),numbers[j].end());
		}

		for (size_t j = 0 ; j < taskList[i].size() ; j++)
		{
			upperBound += numbers[j][numbers[j].size() - 1];
		}

		// for each L2 partition factor set
		for (size_t k = 0 ; k < partitionFactorSetList.size() ; k++)
		{
			// ************************************************************************************
			// Start dynamic programming:
			// Find the L1 cache configuration combination with min E while satisfying D
			// ************************************************************************************
			printf("Core %d, PFset %d: ",i,k);
			for (size_t j = 0 ; j < partitionFactorSetList[k].size() ; j++)
			{
				printf("%d ",partitionFactorSetList[k][j]);
			}
			printf("\n");

			// schedualability constraint
			int constraint = deadline;
			int invalidTableEntry = constraint + 100;

			// define the storage space for dynamic programming
			vector<vector<int> > table;
			//vector<vector<int> > solutionTable;

			table.resize(taskList[i].size());
			//solutionTable.resize(taskList[i].size());

			for (size_t j = 0 ; j < table.size() ; j++)
			{
				table[j].resize(upperBound - lowerBound);
				//solutionTable[j].resize(upperBound - lowerBound);

				for (size_t l = 0 ; l < table[j].size() ; l++)
				{
					table[j][l] = invalidTableEntry;
					//solutionTable[j][l] = -1;
				}
			}

			// dynamic programming solver
			// loop over all the tasks
			for (size_t j = 0 ; j < table.size() ; j++)
			{
				printf("		NOW: core %d , partition factor set %d , Task %d ... \n",i,k,j);

				// consider only the first task, trivial
				if (j == 0)
				{
					for (int l = 0 ; l < upperBound - lowerBound ; l++)
					{
						// for all L1 configurations
						//for (size_t m = 0 ; m < taskList[i][j].profileTablePhase[k].size() ; m++)
						for (int m = 0 ; m < TOTAL_CONFIG_CNT ; m++)
						{
							// compute energy of this task in aware of CP points
							bool isValid = true;
							int totalEnergy = 0;
							int currentInterval = 0;
							int lastEndTime = 0;

							taskEnergyCalculatorHelper(partitionFactorSetList, i, j, k, m, isValid, totalEnergy, currentInterval, lastEndTime);

							//printf("		Task %d , l = %d , L1 config %d ... \n ",j,l,m);

							if (totalEnergy == l + lowerBound)
							{
								if (lastEndTime < table[j][l])
								{
									table[j][l] = lastEndTime;
									//solutionTable[j][l] = m;
								}
							}
						}
					}
				}
				// for other tasks
				else
				{
					for (int l = 0 ; l < upperBound - lowerBound ; l++)
					{
						// loop over all L1 configurations
						//for (size_t m = 0 ; m < taskList[i][j].profileTablePhase[k].size() ; m++)
						for (int m = 0 ; m < TOTAL_CONFIG_CNT ; m++)
						{
							// compute energy of this task in aware of CP points
							bool isValid = true;
							int totalEnergy = 0;
							int currentInterval = 0;
							int lastEndTime = 0;

							// get current start time from last row
							lastEndTime = table[j - 1][l];

							if (lastEndTime > constraint)
								continue;
							
							if (lastEndTime > varyingCPList[varyingCPList.size() - 1])
							{
								currentInterval = varyingCPList.size();
							}
							else
							{
								for (size_t ll = 0 ; ll < varyingCPList.size() ; ll++)
								{
									if (lastEndTime <= varyingCPList[ll])
									{
										currentInterval = ll;
										break;
									}
								}
							}

							taskEnergyCalculatorHelper(partitionFactorSetList, i, j, k, m, isValid, totalEnergy, currentInterval, lastEndTime);

							//printf("		Task %d , l = %d , L1 config %d ... \n ",j,l,m);

							int energyValueThis = l + lowerBound + totalEnergy;

							if (energyValueThis >= upperBound)
								continue;

							if (lastEndTime < table[j][energyValueThis])
							{
								table[j][energyValueThis] = lastEndTime;
								//solutionTable[j][energyValueThis] = m;
							}
						}
					}
				}
			}

			// **************************************************************************************************
			// find the minimum energy consumption of all the tasks and with total time less than the deadline
			for (int l = 0 ; l < upperBound - lowerBound ; l++)
			{
				// the first one with valid schedulability condition is the one with minimum energy consumption
				if (table[table.size() - 1][l] <= constraint)
				{
					coreSolutionList[i][k].push_back(l);								// Energy
					coreSolutionList[i][k].push_back(table[table.size() - 1][l]);		// Time

					break;
				}
			}

			// no valid assignment for this partition factor
			if (coreSolutionList[i][k].size() == 0)
			{
				coreSolutionList[i][k].push_back(-1);
				coreSolutionList[i][k].push_back(-1);
			}
		}
	}
}

void MulticoreDCRSim::taskEnergyCalculatorHelper(vector<vector<int> > partitionFactorSetList, int i, int j, int k, int m, bool& isValid, int& totalEnergy, int& currentInterval, int& lastEndTime)
{
	// Now, for this task, L1 configuration is fixed, L2 PF is decided by the interval
	int pf = partitionFactorSetList[k][currentInterval];					// L2 partition factor for current interval
	int lastIdx = taskList[i][j].profileTablePhase[pf][m].size() - 1;		// profile table entry index for this task (in entirety)

	int currentEndPhase = 0;
	int nextStartPhase = 0;
	double partialPhaseLeftPct(0.0);
	double partialPhaseNewPct(0.0);
	int dynInstFinished = 0;
	int remainingTime = 0;
	bool hasMultipleCP = false;

	// if this task starts but does not end in the current interval
	if (currentInterval < varyingCPList.size() && lastEndTime + taskList[i][j].profileTablePhase[pf][m][lastIdx].time > varyingCPList[currentInterval])
	{
		hasMultipleCP = true;

		// this is the first new interval, the CP point aligns the ending phase of current CP interval
		currentEndPhase = (varyingCPList[currentInterval] - lastEndTime) / phase;
		dynInstFinished = taskList[i][j].profileTablePhase[pf][m][currentEndPhase].inst;

		// calculate the energy consumed until now
		totalEnergy += taskList[i][j].profileTablePhase[pf][m][currentEndPhase].energy;

		lastEndTime = varyingCPList[currentInterval];
		currentInterval++;
		pf = partitionFactorSetList[k][currentInterval];

		// calculate the corresponding phase in the new CP interval based on progress (i.e., dyn. inst.)
		for (size_t ll = 0 ; ll < taskList[i][j].profileTablePhase[pf][m].size() ; ll++)
		{
			if (taskList[i][j].profileTablePhase[pf][m][ll].inst > dynInstFinished)
			{
				nextStartPhase = ll;
				break;
			}
		}

		// this simulation result is invalid
		if (nextStartPhase == 0)
		{
			isValid = false;
			return;
		}

		// calculate the remaining execution time in the new CP interval (assuming uniform distribution of the partial first phase)
		partialPhaseLeftPct = (taskList[i][j].profileTablePhase[pf][m][nextStartPhase].inst - dynInstFinished) /(double) 
							  (taskList[i][j].profileTablePhase[pf][m][nextStartPhase].inst - taskList[i][j].profileTablePhase[pf][m][nextStartPhase - 1].inst);

		lastIdx = taskList[i][j].profileTablePhase[pf][m].size() - 1;

		remainingTime = phase * partialPhaseLeftPct + 
					    (taskList[i][j].profileTablePhase[pf][m][lastIdx].time - taskList[i][j].profileTablePhase[pf][m][nextStartPhase].time);

		// decide in which interval it ends
		while (currentInterval < varyingCPList.size() && lastEndTime + remainingTime > varyingCPList[currentInterval])
		{
			// the CP point does not align the ending phase of current CP interval
			currentEndPhase = nextStartPhase + (varyingCPList[currentInterval] - lastEndTime) / phase;
			partialPhaseNewPct = (varyingCPList[currentInterval] - taskList[i][j].profileTablePhase[pf][m][currentEndPhase - 1].time) /(double) phase;
			dynInstFinished = taskList[i][j].profileTablePhase[pf][m][currentEndPhase - 1].inst + 
							  partialPhaseNewPct * (taskList[i][j].profileTablePhase[pf][m][currentEndPhase].inst - 
												    taskList[i][j].profileTablePhase[pf][m][currentEndPhase - 1].inst);

			// calculate the energy consumed until now
			totalEnergy += partialPhaseLeftPct * (taskList[i][j].profileTablePhase[pf][m][nextStartPhase].energy - 
												  taskList[i][j].profileTablePhase[pf][m][nextStartPhase - 1].energy) 
								+ 
												 (taskList[i][j].profileTablePhase[pf][m][currentEndPhase - 1].energy - 
												  taskList[i][j].profileTablePhase[pf][m][nextStartPhase].energy) 
								+ 
						   partialPhaseNewPct * (taskList[i][j].profileTablePhase[pf][m][currentEndPhase].energy -
												 taskList[i][j].profileTablePhase[pf][m][currentEndPhase - 1].energy);

			lastEndTime = varyingCPList[currentInterval];
			currentInterval++;
			pf = partitionFactorSetList[k][currentInterval];

			// calculate the corresponding phase in the new CP interval based on progress (i.e., dyn. inst.)
			bool found = false;
			for (size_t ll = 0 ; ll < taskList[i][j].profileTablePhase[pf][m].size() ; ll++)
			{
				if (taskList[i][j].profileTablePhase[pf][m][ll].inst > dynInstFinished)
				{
					nextStartPhase = ll;
					found = true;
					break;
				}
			}

			// this simulation result is invalid
			if (!found)
			{
				isValid = false;
				return;
			}

			// calculate the remaining execution time in the new CP interval (assuming uniform distribution of the partial first phase)
			partialPhaseLeftPct = (taskList[i][j].profileTablePhase[pf][m][nextStartPhase].inst - dynInstFinished) /(double) 
								  (taskList[i][j].profileTablePhase[pf][m][nextStartPhase].inst - taskList[i][j].profileTablePhase[pf][m][nextStartPhase - 1].inst);

			lastIdx = taskList[i][j].profileTablePhase[pf][m].size() - 1;

			remainingTime = phase * partialPhaseLeftPct + 
						    (taskList[i][j].profileTablePhase[pf][m][lastIdx].time - taskList[i][j].profileTablePhase[pf][m][nextStartPhase].time);
		}
	}

	// this task now ends in the current intervel
	// calculate the energy consumed
	if (hasMultipleCP)
	{
		lastIdx = taskList[i][j].profileTablePhase[pf][m].size() - 1;

		totalEnergy += partialPhaseLeftPct * (taskList[i][j].profileTablePhase[pf][m][nextStartPhase].energy - 
											  taskList[i][j].profileTablePhase[pf][m][nextStartPhase - 1].energy) 
							+ 
											 (taskList[i][j].profileTablePhase[pf][m][lastIdx].energy - 
											  taskList[i][j].profileTablePhase[pf][m][nextStartPhase].energy);

		// NOTE: align the start time of next task to the very next phase's start time
		//int temp = lastEndTime + remainingTime;
		lastEndTime = phase * ((lastEndTime + remainingTime) / phase + 1);
	}
	else
	{
		totalEnergy += taskList[i][j].profileTablePhase[pf][m][lastIdx].energy;

		// NOTE: align the start time of next task to the very next phase's start time
		//int temp = lastEndTime + taskList[i][m].profileTablePhase[pf][l1config][lastIdx].time;
		lastEndTime = phase * ((lastEndTime + taskList[i][j].profileTablePhase[pf][m][lastIdx].time) / phase + 1);
	}
}

void MulticoreDCRSim::exploreAllVaryingPartition()
{
	int minTotalEnergy = INT_MAX;
	int minTotalEnergyTime = -1;
	int minTotalEnergyPFIdx = -1;

	int totalEnergy = 0;
	int totalEnergyTime = 0;
	int PFSetIdx = 0;

	vector<vector<int> > varyingPartitionList;

	generateVaryingL2PartitionList(varyingPartitionList);

	printf("varyingPartitionList size = %d \n",varyingPartitionList.size());

	// for every possible varying L2 partitions
	for (size_t i = 0 ; i < varyingPartitionList.size() ; i++)
	{
		totalEnergy = 0;
		totalEnergyTime = 0;

		// FOR DEBUG
		/*for (size_t k = 0 ; k < varyingPartitionList[i].size() ; k++)
		{
			printf("(");
			for (size_t m = 0 ; m < partitionList[varyingPartitionList[i][k]].size() ; m++)
				printf("%d ",partitionList[varyingPartitionList[i][k]][m]);
			printf(") ");
		}*/

		// for every core
		for (int j = 0 ; j < num_cores ; j++)
		{
			PFSetIdx = 0;

			// for every interval in this varying partition
			//printf("(");
			for (size_t k = 0 ; k < varyingPartitionList[i].size() ; k++)
			{
				//printf("%d ",partitionList[varyingPartitionList[i][k]][j]);
				//PFSetIdx *= l2Assoc - 1;
				PFSetIdx *= l2Assoc - num_cores + 1;
				PFSetIdx += partitionList[varyingPartitionList[i][k]][j] - 1;
			}
			//printf(") idx = %d , ",PFSetIdx);

			totalEnergy += coreSolutionList[j][PFSetIdx][0];

			if (coreSolutionList[j][PFSetIdx][1] > totalEnergyTime)
				totalEnergyTime = coreSolutionList[j][PFSetIdx][1];
		}


		// totalEnergy may be negative due to no result found for some core under this partition scheme
		if (totalEnergy > 0 && totalEnergy < minTotalEnergy)
		{
			minTotalEnergy = totalEnergy;
			minTotalEnergyTime = totalEnergyTime;
			minTotalEnergyPFIdx = i;
		}

		//printf("\n");
	}

	// Results
	printf("minTotalEnergy = %d , minTotalEnergyTime = %d , ",minTotalEnergy,minTotalEnergyTime);
	for (size_t i = 0 ; i < varyingPartitionList[minTotalEnergyPFIdx].size() ; i++)
	{
		printf("[");
		for (size_t j = 0 ; j < partitionList[varyingPartitionList[minTotalEnergyPFIdx][i]].size() ; j++)
		{
			printf("%d ",partitionList[varyingPartitionList[minTotalEnergyPFIdx][i]][j]);
		}
		printf("] , ");
	}
	printf("\n");
}

void MulticoreDCRSim::generateVaryingL2PartitionList(vector<vector<int> >& res)
{
	vector<int> current;

	current.resize(varyingCPList.size() + 1);

	generateAllCombinationHelper(res,current,0,varyingCPList.size() + 1,partitionList.size(),0);
}

void MulticoreDCRSim::generateAllPartitionFactorSetHelper(vector<vector<int> >& res)
{
	vector<int> current;

	current.resize(varyingCPList.size() + 1);

	generateAllCombinationHelper(res,current,0,varyingCPList.size() + 1,l2Assoc-num_cores+2,1);
}

void MulticoreDCRSim::generateAllL1ConfigurationSetHelper(vector<vector<int> >& res, int core)
{
	vector<int> current;

	current.resize(taskList[core].size());

	generateAllCombinationHelper(res,current,0,taskList[core].size(),TOTAL_CONFIG_CNT,0);
}

void MulticoreDCRSim::generateAllCombinationHelper(vector<vector<int> >& res, vector<int>& current, int cur, int tot, int range, int start)
{
	if (cur == tot)
	{
		res.push_back(current);
		return;
	}

	for (int i = start ; i < range ; i++)
	{
		current[cur] = i;
		generateAllCombinationHelper(res,current,cur + 1,tot,range,start);
	}
}

void MulticoreDCRSim::setPhase(int inPhase)
{
	phase = inPhase;
}
