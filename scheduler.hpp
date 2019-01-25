#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <math.h>
#include <algorithm>
#include "clique_partition.h"

struct operation {
	string type;
	string operand1;
	string operand2;
	string output;
	int timestep;
};

struct reg {
	string name;
	int first; //first timestep accessed
	int last; //last timestep accessed
};

struct resource {
	string type;
	vector<int> clique;
};

struct mux {
	int numInputs;
	string resourceBoundTo;
	int resourceIndex;
};
vector<string> inputs, outputs;
vector<operation> operations;
vector<reg> registers;
vector<resource> opResources;
vector<vector<int> > regResources;
vector<mux> muxResources;
int inputBits = 0, outputBits = 0, registerBits = 0, operationBits = 0;
int** regCompGraph, **funcCompGraph;

void createASAP()
{
	int operationsToSchedule = operations.size(), timestep = 0, operationIndex;
	vector<string> scheduled = inputs;
	vector<int> toSchedule;

	while (operationsToSchedule != 0)
	{
		timestep++;

		for (int i = 0; i < operations.size(); i++) //find operations in current timestep
			if (operations[i].timestep == 0)
				if ((find(scheduled.begin(), scheduled.end(), operations[i].operand1) != scheduled.end()) && // if both operands are in the scheduled vector,
					(find(scheduled.begin(), scheduled.end(), operations[i].operand2) != scheduled.end()))  // we can run this in the first timestep.
					toSchedule.push_back(i);

		while (!toSchedule.empty()) //once all operations that can be scheduled are found:
		{
			operationIndex = toSchedule.back();
			toSchedule.pop_back();
			operations[operationIndex].timestep = timestep; //update each operation with its timestep
			scheduled.push_back(operations[operationIndex].output); //add each operation that is now scheduled to the scheduled vector
			operationsToSchedule--; //update # of operations left to be assigned a timestep
		}
	}
}
