// Datapath Synthesis Tool
// Maharshi Shah and Blaine Oakley

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <math.h>
#include <algorithm>
#include "allocate_binding.hpp"
#include "multiplexor.hpp"
#include "scheduler.hpp"
#include "allocate_reg.hpp"

using namespace std;

/*struct operation {
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
*/

void writeVHDL();
void printMultiplexerBindings();
void allocateMultiplexers();
void printRegisterBindings();
void allocateRegisters();
void printOperationBindings();
void allocateFunctionalUnits();
void printCompatibilityGraph(int** graph, int n);
void createASAP();
void printStructures();
void readInputFile();

/*
vector<string> inputs, outputs;
vector<operation> operations;
vector<reg> registers;
vector<resource> opResources;
vector<vector<int> > regResources;
vector<mux> muxResources;
int inputBits = 0, outputBits = 0, registerBits = 0, operationBits = 0;
int** regCompGraph, **funcCompGraph;
*/

int main()
{
	readInputFile();

	createASAP(); //step 1
	printStructures();

	allocateFunctionalUnits(); //step 2
	printOperationBindings();

	allocateRegisters(); //step 3
	printRegisterBindings();

	allocateMultiplexers(); //step 4
	printMultiplexerBindings();

	writeVHDL(); //step 5







	return 0;
}

void writeVHDL()
{
	string outputFile;
	int controlBits = 0, muxSelBits, muxNumInputs, muxMaxInputs;
	int numAdder = 0, numSub = 0, numMult = 0, resourceNum;
	int controlBitIndex = 0;
	string resType, regName;
	int resIndex, regIndex, opIndex, muxIndex;

	cout << "\nFile to write: ";
	cin >> outputFile;
	ofstream fout;
	fout.open(outputFile.c_str());

	if (!fout) {
		cout << "Could not open file " + outputFile + " for writing" << endl;
		exit(2);
	}

	fout << "library IEEE;\n";
	fout << "use IEEE.std_logic_1164.all;\n\n";

	fout << "entity input_dp is\n";
	fout << "port(\t";
	for (int i = 0; i < inputs.size(); i++)
		fout << inputs[i] << " : IN std_logic_vector(" << inputBits - 1 << " downto 0);\n\t";

	for (int i = 0; i < outputs.size(); i++)
		fout << outputs[i] << " : OUT std_logic_vector(" << outputBits - 1 << " downto 0);\n\t";

	fout << "ctrl: IN std_logic_vector(";

	controlBits += regResources.size();
	for (int i = 0; i < muxResources.size(); i++)
	{
		muxSelBits = 0;
		muxMaxInputs = 1;
		do
		{
			muxNumInputs = muxResources[i].numInputs;
			muxSelBits++;
			muxMaxInputs *= 2;
		} while (muxNumInputs > muxMaxInputs);
		controlBits += muxSelBits;
	}
	controlBits--;
	fout << controlBits << " downto 0);\n\t clear: IN std_logic;\n\tclock: IN std_logic\n);\nend input_dp;\n";

	fout << "\narchitecture RTL of input_dp is\n\n";

	fout << "  component c_register\n";
	fout << "  generic (width : integer := 4);\n";
	fout << "  port (input : in std_logic_vector((width-1) downto 0);\n";
	fout << "    WR: in std_logic;\n";
	fout << "    clear : in std_logic;\n";
	fout << "    clock : in std_logic;\n";
	fout << "    output : out std_logic_vector((width -1) downto 0));\n";
	fout << "  end component;\n\n";

	fout << "  component C_Adder\n";
	fout << "    generic (width : integer); \n";
	fout << "    port(  input1 : in Std_logic_vector ((width - 1) downto 0); \n";
	fout << "    input2 : in Std_logic_vector ((width - 1) downto 0); \n";
	fout << "    output : out Std_logic_vector (width downto 0)); \n";
	fout << "  end component;\n\n";

	fout << "  component C_subtractor\n";
	fout << "    generic (width : integer); \n";
	fout << "    port(  input1 : in Std_logic_vector ((width - 1) downto 0); \n";
	fout << "    input2 : in Std_logic_vector ((width - 1) downto 0); \n";
	fout << "    output : out Std_logic_vector (width downto 0)); \n";
	fout << "  end component; \n\n";

	fout << "  component C_multiplier \n";
	fout << "    generic (width : integer); \n";
	fout << "    port(  input1 : in Std_logic_vector ((width - 1) downto 0); \n";
	fout << "    input2 : in Std_logic_vector ((width - 1) downto 0); \n";
	fout << "    output : out Std_logic_vector (((width * 2) - 2) downto 0)); \n";
	fout << "  end component; \n\n";

	fout << "  component C_Multiplexer\n";
	fout << "    generic (width : integer;\n";
	fout << "      no_of_inputs : integer;\n";
	fout << "      select_size : integer); \n";
	fout << "    port(  input : in Std_logic_vector (((width*no_of_inputs) - 1) downto 0);\n";
	fout << "    MUX_SELECT : in Std_logic_vector ((select_size - 1) downto 0);\n";
	fout << "    output : out Std_logic_vector ((width - 1) downto 0)); \n";
	fout << "  end component; \n\n";

	for (int i = 0; i < regResources.size(); i++)
		fout << "\tsignal R" << i << "_out : Std_logic_vector(" << registerBits - 1 << " downto 0);\n";

	fout << "\n";

	for (int i = 0; i < opResources.size(); i++)
	{
		if (opResources[i].type == "MULT") {
			resourceNum = 0;
			fout << "\tsignal FU" << resourceNum << "_" << numMult;
			numMult++;
		}
		else if (opResources[i].type == "SUB") {
			resourceNum = 1;
			fout << "\tsignal FU" << resourceNum << "_" << numSub;
			numSub++;
		}
		else if (opResources[i].type == "ADD") {
			resourceNum = 2;
			fout << "\tsignal FU" << resourceNum << "_" << numAdder;
			numAdder++;
		}
		fout << "_out : Std_logic_vector(" << operationBits << " downto 0);\n";//ask Richard about FU signal length
	}
	fout << "\n";
	for (int i = 0; i < muxResources.size(); i++)
		fout << "\tsignal Mux" << i << "_out :  Std_logic_vector(" << inputBits << " downto 0);\n";

	fout << "\nbegin\n\n";

	muxIndex = 0;
	for (int i = 0; i < regResources.size(); i++)
	{

		fout << "\tR" << i << "  : C_Register\n\t generic map(" << registerBits << ")\n";
		fout << "\t port map (\n\t\t input(" << registerBits - 1 << " downto 0) => ";
		if (muxIndex < muxResources.size() && muxResources[muxIndex].resourceBoundTo == "REG")
		{
			fout << "Mux" << muxIndex << "_out(" << inputBits - 1 << " downto 0),\n";
			muxIndex++;

		}
		else {

			fout << registers[regResources[i][0]].name << "(" << inputBits - 1 << " downto 0),\n";
		}
		fout << "\t\t WR => ctrl(" << i << "),\n\t\t CLEAR => clear,\n";
		fout << "\t\t CLOCK => clock,\n\t\t output => R" << i << "_out);\n\n";
	}

	numAdder = numSub = numMult = 0;
	for (int i = 0; i < opResources.size(); i++)
	{
		fout << "\t" << opResources[i].type;
		if (opResources[i].type == "MULT")
		{
			resourceNum = 0;
			fout << resourceNum << "_" << numMult << " : C_Multiplier\n";
		}
		else if (opResources[i].type == "SUB")
		{
			resourceNum = 1;
			fout << resourceNum << "_" << numSub << " : C_Subtractor\n";
		}
		else if (opResources[i].type == "ADD")
		{
			resourceNum = 2;
			fout << resourceNum << "_" << numAdder << " : C_Adder\n";
		}
		fout << "\t\t generic map(" << operationBits << ")\n";
		fout << "\t\t port map (\n";
		fout << "\t\t input1(" << operationBits - 1 << " downto 0) => R";

		int regIndex;
		string regstring = operations[opResources[i].clique[0]].operand1;
		for (int j = 0; j < registers.size(); j++)
			if (registers[j].name == regstring)
				regIndex = j;

		for (int j = 0; j < regResources.size(); j++)
			for (int k = 0; k < regResources[j].size(); k++)
				if (regResources[j][k] == regIndex)
					fout << j << "_out(" << operationBits - 1 << " downto 0),\n";

		fout << "\t\t input2(" << operationBits - 1 << " downto 0) => R";


		regstring = operations[opResources[i].clique[0]].operand2;
		for (int j = 0; j < registers.size(); j++)
			if (registers[j].name == regstring)
				regIndex = j;

		for (int j = 0; j < regResources.size(); j++)
			for (int k = 0; k < regResources[j].size(); k++)
				if (regResources[j][k] == regIndex)
					fout << j << "_out(" << operationBits - 1 << " downto 0),\n";

		fout << "\t\t output(" << operationBits << " downto 0) => FU";


		if (opResources[i].type == "MULT")
		{
			resourceNum = 0;
			fout << resourceNum << "_" << numMult << "_out(";
			numMult++;
		}
		else if (opResources[i].type == "SUB")
		{
			resourceNum = 1;
			fout << resourceNum << "_" << numSub << "_out(";
			numSub++;
		}
		else if (opResources[i].type == "ADD")
		{
			resourceNum = 2;
			fout << resourceNum << "_" << numAdder << "_out(";
			numAdder++;
		}
		fout << operationBits << " downto 0));\n\n";
	}

	controlBitIndex = regResources.size();
	for (int i = 0; i < muxResources.size(); i++)
	{
		fout << "\tMUX" << i << " : C_Multiplexer\n";
		fout << "\t\tgeneric map(" << inputBits << ", ";
		fout << muxResources[i].numInputs << ", ";

		muxSelBits = 0;
		muxMaxInputs = 1;
		do
		{
			muxNumInputs = muxResources[i].numInputs;
			muxSelBits++;
			muxMaxInputs *= 2;
		} while (muxNumInputs > muxMaxInputs);

		fout << muxSelBits << ")\n";
		fout << "\t\tport map(\n";

		for (int j = 0; j < muxNumInputs; j++)
		{
			fout << "\t\tinput(" << ((j + 1)*operationBits) - 1 << " downto " << j*operationBits << ") => ";
			resType = muxResources[i].resourceBoundTo;
			if (resType == "REG")
			{
				resIndex = muxResources[i].resourceIndex; //register index

														  //fout << "\n" << regResources[resIndex].size();
				regIndex = regResources[resIndex][j];
				//fout << regIndex << "\n";
				regName = registers[regIndex].name;
				//fout << regName << "\n";
				if (find(inputs.begin(), inputs.end(), regName) != inputs.end())//if it is found
					fout << regName;
				else
				{
					for (int k = 0; k < operations.size(); k++)//search for name in operations reg
						if (operations[k].output == regName)
							opIndex = k; //get index of operation which is in a clique.

					for (int k = 0; k < opResources.size(); k++)
						for (int r = 0; r < opResources[k].clique.size(); r++)
							if (opResources[k].clique[r] == opIndex)
								resIndex = k; //index in opresources of the FU

					numAdder = numSub = numMult = 0;
					if (opResources[resIndex].type == "MULT")
					{
						resourceNum = 0;
						fout << "FU" << resourceNum << "_" << numMult << "_out";
						numMult++;
					}
					else if (opResources[resIndex].type == "SUB")
					{
						resourceNum = 1;
						fout << "FU" << resourceNum << "_" << numSub << "_out";
						numSub++;
					}
					else if (opResources[resIndex].type == "ADD")
					{
						resourceNum = 2;
						fout << "FU" << resourceNum << "_" << numAdder << "_out";
						numAdder++;
					}

				}

			}
			else //is a function unit, sub/add/mult
			{
				opIndex = muxResources[i].resourceIndex;
				for (int k = 0; k < opResources[opIndex].clique.size(); k++) //search through clique
				{
					opIndex = opResources[opIndex].clique[k];
					regName = operations[opIndex].output;//get name of each register connected to the mux
														 // find what register clique in regResources it is in
					for (int p = 0; p < registers.size(); p++)
						if (registers[p].name == regName)
							regIndex = p;

					for (int p = 0; p < regResources.size(); p++)
						for (int r = 0; r < regResources[p].size(); r++)
							if (regResources[p][r] == regIndex)
								regIndex = p; //get the index of the function unit the register name is in


				}
				fout << "R" << regIndex << "_out";
			}
			fout << "(" << operationBits - 1 << " downto 0),\n";
		}

		fout << "\t\tMUX_SELECT(" << muxSelBits - 1 << " downto 0) => ctrl(";
		fout << controlBitIndex + (muxSelBits - 1) << " downto " << controlBitIndex << "),\n";
		controlBitIndex += muxSelBits;
		fout << "\t\toutput => Mux" << i << "_out);\n";
		fout << "\n";
	}

	for (int i = 0; i < outputs.size(); i++)
	{
		fout << "\t " << outputs[i] << "(" << outputBits - 1 << " downto 0) <= R";

		for (int j = 0; j < registers.size(); j++)
			if (registers[j].name == outputs[i])
				regIndex = j;

		for (int j = 0; j < regResources.size(); j++)
			for (int k = 0; k < regResources[j].size(); k++)
				if (regResources[j][k] == regIndex)
					fout << j << "_out(" << outputBits - 1 << " downto 0);\n";
	}
	fout << "end RTL;\n";
}

void printMultiplexerBindings()
{
	cout << endl << "Multiplexer Allocation:" << endl;
	for (int i = 0; i < muxResources.size(); i++)
	{
		cout << "Mux #" << i << ": ";
		cout << muxResources[i].resourceBoundTo << " #" << muxResources[i].resourceIndex;
		cout << " #Inputs: " << muxResources[i].numInputs << endl;
	}
}

/*void allocateMultiplexers()
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
}*/

void printRegisterBindings()
{
	for (int i = 0; i < regResources.size(); i++)
	{
		cout << "Register #" << i << ": ";
		for (int j = 0; j < regResources[i].size(); j++)
			cout << " " << regResources[i][j];
		cout << endl;
	}
}

/*void allocateRegisters() //need to make it work for register
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
}*/

void printOperationBindings()
{
	for (int i = 0; i < opResources.size(); i++)
	{
		cout << "Functional Unit #" << i << ": ";
		cout << opResources[i].type;
		for (int j = 0; j < opResources[i].clique.size(); j++)
			cout << " " << opResources[i].clique[j];
		cout << endl;
	}
}

/*void allocateFunctionalUnits()
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
}*/

void printCompatibilityGraph(int** graph, int n)
{
	cout << "Compatibility Graph:" << endl;
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
			cout << graph[i][j] << " ";
		cout << endl;
	}
}

/*void createASAP()
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
}*/

void printStructures()
{
	cout << endl;
	cout << "Input Bit Size:     " << inputBits << endl;
	cout << "Output Bit Size:    " << outputBits << endl;
	cout << "Register Bit Size:  " << registerBits << endl;
	cout << "Operation Bit Size: " << operationBits << endl;

	cout << endl << "Inputs:    ";
	for (int i = 0; i < inputs.size(); i++)
		cout << inputs[i] + " ";

	cout << endl << "Outputs:   ";
	for (int i = 0; i < outputs.size(); i++)
		cout << outputs[i] + " ";

	cout << endl << "Registers: ";
	for (int i = 0; i < registers.size(); i++)
		cout << registers[i].name + " ";

	cout << endl << endl << "Operations:" << endl;
	cout << setw(10) << left << "TYPE";
	cout << setw(10) << left << "OP1";
	cout << setw(10) << left << "OP2";
	cout << setw(10) << left << "OUT";
	cout << setw(10) << left << "TIME" << endl;
	cout << "--------------------------------------------------" << endl;

	for (int i = 0; i < operations.size(); i++)
	{
		cout << setw(10) << left << operations[i].type;
		cout << setw(10) << left << operations[i].operand1;
		cout << setw(10) << left << operations[i].operand2;
		cout << setw(10) << left << operations[i].output;
		cout << setw(10) << left << operations[i].timestep << endl;
	}
	cout << endl << endl;
}

void readInputFile()
{
	string inputFile, line;
	int i = -1;

	cout << "File to read: ";
	cin >> inputFile;

	ifstream in;
	in.open(inputFile.c_str());

	if (!in) {
		cout << "Could not open file " + inputFile + " for reading" << endl;
		exit(1);
	}

	in >> line; //string "inputs"

	i = -1;
	in >> line; //first input
	while (line != "outputs") //Read in the inputs
	{
		i++;
		inputs.push_back(line);
		in >> line; //input bit size
		if (i == 0) inputBits = atoi(line.c_str());
		in >> line; //either next input name, or next line starting with "outputs"
	}

	i = -1;
	in >> line; //first output
	while (line != "regs") //Read in the outputs
	{
		i++;
		outputs.push_back(line);
		in >> line; //output bit size
		if (i == 0) outputBits = atoi(line.c_str());
		in >> line; //either next output name, or next line starting with "regs"
	}

	i = -1;
	in >> line; //first register
	while (line != "op1") //Read in the registers
	{
		i++;
		registers.push_back(reg());
		registers[i].name = line;
		registers[i].first = 0;
		registers[i].last = 0;
		in >> line; //register bit size
		if (i == 0) registerBits = atoi(line.c_str());
		in >> line; //either next register name, or next line starting with "op1"
	}

	i = -1;
	while (line != "end") //Read in the operations
	{
		i++;
		operations.push_back(operation());
		in >> line; //type
		operations[i].type = line;
		in >> line; //bitsize
		if (i == 0) operationBits = atoi(line.c_str());
		in >> line; //operand1
		operations[i].operand1 = line;
		in >> line; //operand2
		operations[i].operand2 = line;
		in >> line; //output
		operations[i].output = line;
		in >> line; //next operation or "end"
		operations[i].timestep = 0;
	}
}
