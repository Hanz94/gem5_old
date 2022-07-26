/*
 * task.h
 *
 *	This header file defines task in multi-core.
 *
 *  Created on: Oct 04, 2010
 *      Author: Weixun Wang
 */

#include "stdafx.h"

//#pragma once

struct PTEntry
{
public:
	PTEntry();
	PTEntry(string,int,int,int);
	PTEntry(string,int,int,int,int);

	string L1configStr;
	int index;
	int phase;
	int energy;
	int time;
	int vul;
	int inst;
};

PTEntry::PTEntry()
{
	L1configStr = "";
	index = 0;
	phase = 0;
	energy = 0;
	time = 0;
	vul = 0;
	inst = 0;
}

PTEntry::PTEntry(string inStr,int inIndex,int inEnergy,int inTime):
	L1configStr(inStr),
	index(inIndex),
	energy(inEnergy),
	time(inTime)
{
	vul = 0;
	phase = 0;
	inst = 0;
}

PTEntry::PTEntry(string inStr,int inIndex,int inEnergy,int inTime, int inVul):
	L1configStr(inStr),
	index(inIndex),
	energy(inEnergy),
	time(inTime),
	vul(inVul)
{
	phase = 0;
	inst = 0;
}

struct PTPEntry
{
public:
	PTPEntry();
	PTPEntry(int,int);

	int energy;
	int time;
	int inst;
};

PTPEntry::PTPEntry()
{
	energy = 0;
	time = 0;
	inst = 0;
}

PTPEntry::PTPEntry(int inEnergy,int inTime):
	energy(inEnergy),
	time(inTime)
{
	inst = 0;
}

struct Task
{
public:
	Task();
	Task(int,string);
	Task(int,string,int,int,int);

	int id;				// task id
	string name;		// benchmark name of this task

	// [for every L2 partition factor][for every IL1 and DL1 combination]
	vector<vector<PTEntry> > profileTable;

	// [for every L2 partition factor][for every IL1 and DL1 combination][for every phase]
	vector<vector<vector<PTPEntry> > > profileTablePhase;
};

Task::Task()
{
	id = 0;
	name = "";
}

Task::Task(int inId,string inName):
	id(inId),
	name(inName)
{}
