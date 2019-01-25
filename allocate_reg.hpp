#include "allocate_binding.hpp"
#include <algorithm>

void allocateRegisters() //need to make it work for register
{
	int m = operations.size();//# of operations
	int n; //length of a side of the compatibility matrix
	int maxTimestep = 0;

	for (int i = 0; i < inputs.size(); i++)
	{
		registers.push_back(reg());
		registers.back().name = inputs[i];
		registers.back().first = 0;

		for (int j = 0; j < m; j++) //if operation has this input has an operand, inputs first/last are operation's timestep
			if (operations[j].operand1 == inputs[i] || operations[j].operand2 == inputs[i])
				registers.back().last = operations[j].timestep;
	}

	for (int i = 0; i < outputs.size(); i++)
	{
		registers.push_back(reg());
		registers.back().name = outputs[i];

		for (int j = 0; j < m; j++) //if operation has this output as an out, first timestep = this timestep and last = something else
			if (operations[j].output == outputs[i])
				registers.back().first = operations[j].timestep;

		for (int j = 0; j < m; j++)
			if (operations[j].timestep > maxTimestep)
				maxTimestep = operations[j].timestep;

		registers.back().last = maxTimestep;
	}

	n = registers.size();

	for (int i = 0; i < n; i++) //need to determine first time accessed and last time accessed for each edge aka register
		for (int j = 0; j < m; j++)
			if (operations[j].output == registers[i].name) //if output is our reg, first time it is written to
				registers[i].first = operations[j].timestep;
			else if ((operations[j].operand1 == registers[i].name) ||
				(operations[j].operand2 == registers[i].name)) // if either input is our reg, last time it is read.
				registers[i].last = operations[j].timestep;

	regCompGraph = new int*[n];

	for (int i = 0; i < n; i++) //declare empty comp graph filled with junk data
		regCompGraph[i] = new int[n];

	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
			if ((i == j) || (registers[i].last <= registers[j].first) || (registers[i].first >= registers[j].last))
			{
				regCompGraph[i][j] = 1;
				regCompGraph[j][i] = 1;
			}
			else {
				regCompGraph[i][j] = 0;
				regCompGraph[j][i] = 0;
			}

			clique_partition(regCompGraph, n);

			int opIndex;
			string type;

			for (int i = 0; i < MAXCLIQUES; i++)
			{
				if (clique_set[i].size == UNKNOWN) break;
				regResources.push_back(vector<int>());

				for (int j = 0; j < MAXCLIQUES; j++)
					if (clique_set[i].members[j] != UNKNOWN)
						regResources[i].push_back(clique_set[i].members[j]);
					else
						break;
			}
}
