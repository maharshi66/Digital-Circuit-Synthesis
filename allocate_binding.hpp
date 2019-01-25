#include "scheduler.hpp"
#include <algorithm>

void allocateFunctionalUnits()
{
	int n = operations.size(); //length of a side of this square matrix
	funcCompGraph = new int*[n];

	for (int i = 0; i < n; i++) //declare empty comp graph filled with junk data
		funcCompGraph[i] = new int[n];

	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
			//check each op against all other ops. If op = op, or 
			//if they are same type, different ops, and different timestep
			if ((i == j) ||
				((operations[i].type == operations[j].type) &&
				(operations[i].timestep != operations[j].timestep)))
			{
				funcCompGraph[i][j] = 1;
				funcCompGraph[j][i] = 1;
			}
			else {
				funcCompGraph[i][j] = 0;
				funcCompGraph[j][i] = 0;
			}

			clique_partition(funcCompGraph, n); //access results in clique_set[]

			int opIndex;

			for (int i = 0; i < MAXCLIQUES; i++)
			{
				if (clique_set[i].size == UNKNOWN) break;
				opResources.push_back(resource());

				opIndex = clique_set[i].members[0];
				opResources[i].type = operations[opIndex].type;

				for (int j = 0; j < MAXCLIQUES; j++)
					if (clique_set[i].members[j] != UNKNOWN)
						opResources[i].clique.push_back(clique_set[i].members[j]);
					else
						break;
			}
}
