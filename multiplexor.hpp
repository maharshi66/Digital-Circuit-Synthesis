#include <algorithm>
#include "allocate_reg.hpp"


void allocateMultiplexers()
{
	for (int i = 0; i < regResources.size(); i++)
		if (regResources[i].size() > 1)
		{
			muxResources.push_back(mux());
			muxResources.back().numInputs = regResources[i].size(); //clique size
			muxResources.back().resourceBoundTo = "REG";
			muxResources.back().resourceIndex = i;
		}

	for (int i = 0; i < opResources.size(); i++)
		if (opResources[i].clique.size() > 1)
		{
			muxResources.push_back(mux());
			muxResources.back().numInputs = opResources[i].clique.size(); //num items in clique
			muxResources.back().resourceBoundTo = opResources[i].type;
			muxResources.back().resourceIndex = i;
		}
}
