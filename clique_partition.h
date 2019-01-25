#include "stdio.h" 
#include "stdlib.h"
#include "assert.h"

/****************************************************************************
*  This is a C implementation of the Tseng and Seiworick's Clique
*  partitioning heuristic, as reported in the following publication:
*       C. Tseng and D. P. Siewiorek, ``Facet: A Procedure for the
*      Automated Synthesis of Digital Systems,'' DAC, 1983, pp. 490-496.
*
*  Author: Srinivas Katkoori,
*          University of South Florida,
*          katkoori@csee.usf.edu.
*
*  Date: June 2001
*
*  Implementation Details:
*
*   Input: Two dimensional symmetric compatibility array
*   where compat[i][j] = 1  if nodes i and j are compatible
*                     = 0  otherwise
*
*   Output: Set of cliques
*   A global array "clique_set" stores the results.
*   Each clique has a (1) list of members, a integer array of nodeid's
*   (2) the number of members information.
*   All the entries are initialized to UNKNOWN values.
*
*   o The maximum number of cliques is 200.  This can be changes by
*     changing the hash define value of MAXCLIQUES.
*   o Call clique_partition(compatibility array, nodesize)
*   o The output can be printed using print_clique_set() function.
*   o Compile this code without DEBUG option
*
*       unix% gcc -g clique_partition.c
*
*   o To enable DEBUG option:
*
*       unix% gcc -g -DDEBUG clique_partition.c
*
*  Modification History:
*
*  Acknowledgment:
*--  This code is developed as part of the AUDI (AUtomatic 	    --
*--  Design Instantiation) system, a behavioral synthesis system,  --
*--  developed at the University of South Florida.  The AUDI project
*--  is supported by the National Science Foundation (NSF) under   --
*--  the (CAREER) grant number 0093301.  If you have any questions,--
*--  contact:
*--     Dr. Srinivas Katkoori,                                     --
*--     4202 East Fowler Avenue, ENB 118,                          --
*--     Computer Science & Engineering Department,                 --
*--     University of South Florida,                               --
*--     Tampa, FL 33647.					    --
*--     Email: katkoori@csee.usf.edu				    --
*-- 								    --
****************************************************************************
*  Psuedo code of the Clique Partitioning heuristic
****************************************************************************
*
*  Clique Partition G(V,E)
*  begin
*  N <- V  / make a local copy of V
*  for n in N
*    priority(n) <- 0
*  end for
*  while (N is not empty) do
*    x <- select_node() / if a node with priority, then pick that node
*                       / else a node with highest degree
*                       /        if multiple nodes then pick a node
*                       /           with highest neighbor wt
*                       /           if multiple, pick one randomly.
*    Y <- { y | (x, y) is in E}
*    if | Y | = 0
*    then
*       N <- N - {x}  / x represents a clique
*    else
*      for y in Y do
*         I_y <- { z | (y,z) is not in E}
*      end for
*
*      min_val  <- min{ | intersection(I_y, Y) |  y in Y}
*      Y1 <- { y | y in Y and | intersection (I_y, Y) | = min_val}
*
*      max_val  <- max{ |I_y| for y in Y1}
*      Y2 <- { y | y in Y1 and |I_y| = max_val}
*
*      y <- a node arbitrarily selected from Y2
*
*      / merge nodes x and y into z
*      Create a new node z
*      priority (z) <-  1
*      For every node n which is connected to both x and y,
*        create an edge connecting n and z
*      Delete nodes x and y and all edges connected to them
*    end if
*   end while
*  end
*****************************************************************************
*/

#define UNKNOWN -12345
#define MAXCLIQUES 200

#define CLIQUE_UNKNOWN -12345  
#define CLIQUE_TRUE 100
#define CLIQUE_FALSE 110 

struct clique
{
	int members[MAXCLIQUES];           /* members of the clique */
	int size;                          /* number of members in the clique */
};

struct clique clique_set[MAXCLIQUES];   /* stores the clique partitioning results */

										/********************************************************************************/

int input_sanity_check(int** compat, int array_dimension)
{
	/* Verifies whether the compat array passed is valid array
	*  (1) Is each array entry =0 or 1?
	*  (2) Is the matrix symmetric
	* Note that diagnol entries can be either 0 or 1.
	*/

	int i = CLIQUE_UNKNOWN;
	int j = CLIQUE_UNKNOWN;
	printf(" Checking the sanity of the input..");

	for (i = 0; i< array_dimension; i++)
	{
		for (j = 0; j< array_dimension; j++)
		{
			if ((compat[i][j] != 1) && (compat[i][j] != 0))
			{
				printf(" %d \n", compat[i][j]);
				printf("The value of an array element is other than 1 or 0. Aborting..\n");
				exit(0);
			}
			if (compat[i][j] != compat[j][i])
			{
				printf("The compatibility array is NOT symmetric at (%d,%d) and (%d,%d) Aborting..\n", i, j, j, i);
				exit(0);
			}
			printf(".");
		}
	}
	printf("Done.\n");
	return CLIQUE_TRUE;
}

int output_sanity_check(int array_dimension, int** local_compat, int** compat)
{
	/*
	* Verifies the results of the heuristic.
	* for each clique do
	*   if the clique size is UNKNOWN
	*      break
	*   else
	*     for every pair of members x and y in clique do
	*       assert  compat[x][y] = 1 and compat[y][x] = 1
	*     end for
	*   end if
	* end for
	*/
	int i = UNKNOWN, j = UNKNOWN, k = UNKNOWN;
	int member1 = UNKNOWN, member2 = UNKNOWN;

	printf("\n Verifying the results of the clique partitioning algorithm..");
	for (i = 0; i<MAXCLIQUES; i++)
	{
		if (clique_set[i].size != UNKNOWN)
		{
			assert(clique_set[i].size > 0);
			for (j = 0; j < clique_set[i].size; j++)
			{
				for (k = 0; k < clique_set[i].size; k++)
				{
					if (j != k)
					{
						member1 = clique_set[i].members[j];
						member2 = clique_set[i].members[k];

						assert(compat[member1][member2] == 1);
						assert(compat[member2][member1] == 1);
						assert(local_compat[member2][member1] == 1);
						assert(local_compat[member2][member1] == 1);
						printf(".");
					}
				}
			}
		}
	}
	printf("..Done.\n");
	return CLIQUE_TRUE;
}


void make_a_local_copy(int** local_compat, int** compat, int nodesize)
{
	int i = CLIQUE_UNKNOWN, j = CLIQUE_UNKNOWN;

	for (i = 0; i<nodesize; i++)
	{
		for (j = 0; j<nodesize; j++)
		{
			local_compat[i][j] = compat[i][j];
		}
	}
	return;
}

int get_degree_of_a_node(int x, int nodesize, int** local_compat, int* node_set)
{
	int j = CLIQUE_UNKNOWN, node_degree = CLIQUE_UNKNOWN;

	node_degree = 0;
	for (j = 0; j<nodesize; j++) /* compute node degrees */
	{
		if (node_set[j] != CLIQUE_UNKNOWN)
		{
			if (x != j)
				node_degree += local_compat[x][j];
		}
	}

	return node_degree;
}

int select_new_node(int** local_compat, int nodesize, int* node_set)
{
	/*    if a node with priority, then pick that node
	*      else a node with highest degree
	*        if multiple nodes then pick a node
	*           with highest neighbor wt
	*             if multiple pick one randomly.
	*/
	int i = CLIQUE_UNKNOWN, j = CLIQUE_UNKNOWN;
	int** degrees;
	degrees = new int*[nodesize];

	for (int i = 0; i < nodesize; i++) //declare empty comp graph filled with junk data
		degrees[i] = new int[nodesize];
	int max_degree = CLIQUE_UNKNOWN;
	int curr_max_degree = CLIQUE_UNKNOWN;
	int curr_node_degree = CLIQUE_UNKNOWN;
	int index = CLIQUE_UNKNOWN, curr_node = CLIQUE_UNKNOWN;
	int max_curr_neighbors_wt = CLIQUE_UNKNOWN;
	int curr_neighbors_wt = CLIQUE_UNKNOWN;
	int max_node = CLIQUE_UNKNOWN;

	for (i = 0; i<nodesize; i++) /* initialize the degrees matrix */
	{  /* i dimension = node degree */
		for (j = 0; j<nodesize; j++)
		{  /* j dimension = node with degree=i */
			degrees[i][j] = CLIQUE_UNKNOWN;
		}
	}

	curr_max_degree = 0;
	curr_node_degree = 0;

	for (i = 0; i<nodesize; i++)  /* for each node do */
	{
		if (node_set[i] != CLIQUE_UNKNOWN)  /* if the node is still in N */
		{
			curr_node_degree = get_degree_of_a_node(i, nodesize, local_compat, node_set);

#ifdef DEBUG
			printf(" node=%d curr_node_degree = %d \n", i, curr_node_degree);
#endif
			if (curr_node_degree > curr_max_degree)
			{
				curr_max_degree = curr_node_degree;
			}

			/* append to a list of nodes with degree=curr_node_degree */
			index = 0;
			while (degrees[curr_node_degree][index] != CLIQUE_UNKNOWN) index++;
			degrees[curr_node_degree][index] = i; /* register this node */
		}
	}

	/* for debugging purposes..
	for (i=0; i<nodesize; i++)
	{
	for (j=0; j<nodesize; j++)
	{
	printf(" %d %d %d \n", i, j, degrees[i][j]);
	}
	}
	*/
	if (degrees[curr_max_degree][1] == CLIQUE_UNKNOWN) /* only one max node */
		max_node = degrees[curr_max_degree][0];
	else
		if (degrees[curr_max_degree][1] != CLIQUE_UNKNOWN) /* multiple max nodes */
		{
			index = 0;
			max_curr_neighbors_wt = 0;

			for (index = 0; index<nodesize; index++)
				/* go through all nodes with curr_max_degree*/
			{
				if (degrees[curr_max_degree][index] != CLIQUE_UNKNOWN)
					/* not end of list of the nodes with curr_max_degree */
				{
					curr_neighbors_wt = 0;
					curr_node = degrees[curr_max_degree][index];

					/* get cumulative neighbor weight for this node */
					curr_neighbors_wt += get_degree_of_a_node(curr_node, nodesize,
						local_compat, node_set);
#ifdef DEBUG
					printf("curr_node = %d curr_neighbors_wt=%d\n", curr_node, curr_neighbors_wt);
#endif
					/* Is the local_compat, node_set consistent? */
					if (curr_neighbors_wt >= max_curr_neighbors_wt)
					{
						max_curr_neighbors_wt = curr_neighbors_wt;
						max_node = curr_node;
					}
				}
			}
		}
#ifdef DEBUG
	printf(" curr_max_degree = %d max_node= %d\n", curr_max_degree, max_node);
#endif

	return max_node;
}

int form_setY(int* setY, int* current_clique, int** local_compat, int nodesize, int* node_set)
{
	int i = CLIQUE_UNKNOWN, j = CLIQUE_UNKNOWN, index = CLIQUE_UNKNOWN;
	int setY_size = CLIQUE_UNKNOWN;
	int compatibility = CLIQUE_UNKNOWN;

	index = 0;
	setY_size = 0;

	/* reset set_Y */
	for (i = 0; i<nodesize; i++)
	{
		setY[i] = CLIQUE_UNKNOWN;
	}

	for (i = 0; i<nodesize; i++)
	{
		compatibility = CLIQUE_TRUE;
		if (node_set[i] != CLIQUE_UNKNOWN)
		{
			for (j = 0; j<nodesize; j++)
			{
				if (current_clique[j] != CLIQUE_UNKNOWN)
				{
					if (local_compat[current_clique[j]][i] == 0)
					{
						compatibility = CLIQUE_FALSE;
						break;
					}
				}
				else
					break;
			}
			if (compatibility == CLIQUE_TRUE)
			{
				setY[index] = i;
				index++;
			}
		}
	}
	setY_size = index;

	return setY_size;
}

void print_setY(int* setY)
{
	int index = CLIQUE_UNKNOWN;

	index = 0;
	printf(" setY = {");
	while (setY[index] != CLIQUE_UNKNOWN)
	{
		printf(" %d ", setY[index]);
		index++;
	}
	printf("}\n");
}

void form_set_Y1(int nodesize, int* set_Y, int* set_Y1, int** sets_I_y, int* node_set)
{
	int i = CLIQUE_UNKNOWN, j = CLIQUE_UNKNOWN, k = CLIQUE_UNKNOWN;
	int* cards = (int*)NULL;
	int min_val = CLIQUE_UNKNOWN;
	int curr_index = CLIQUE_UNKNOWN;
	int curr_y = CLIQUE_UNKNOWN;

	cards = (int*)malloc(nodesize * sizeof(int));

	for (i = 0; i<nodesize; i++)
	{
		set_Y1[i] = CLIQUE_UNKNOWN;
		cards[i] = 0;
	}

	/* Get the cardinalities of  intersection(I_y, setY)
	for each y in I_y */
	for (i = 0; i<nodesize; i++) /* for each y in I_y */
	{
		if (set_Y[i] != CLIQUE_UNKNOWN)
		{
			curr_y = set_Y[i];

			for (j = 0; j<nodesize; j++) /* for each node in I_y of curr_y*/
			{
				if (sets_I_y[curr_y][j] != CLIQUE_UNKNOWN)
				{
					for (k = 0; k<nodesize; k++) /* for each node in set_Y */
					{
						if (set_Y[k] != CLIQUE_UNKNOWN)
						{
							if (sets_I_y[i][j] == set_Y[k])
								cards[i]++;
						}
						else
							break; /* end of set_Y */
					}
				}
				else
					break;
			}
		}
	}

	min_val = cards[0];
	for (i = 0; i<nodesize; i++)
	{
		if (set_Y[i] != CLIQUE_UNKNOWN)
		{
			if (cards[i] < min_val)
				min_val = cards[i];
		}
	}

#ifdef DEBUG
	printf(" min_val = %d ", min_val);
#endif

	curr_index = 0;
	for (i = 0; i<nodesize; i++)
	{
		if (cards[i] == min_val)
		{
			set_Y1[curr_index] = set_Y[i];
			curr_index++;
		}
	}

#ifdef DEBUG
	printf(" Set Y1 = { ");
	for (i = 0; i<nodesize; i++)
	{
		if (set_Y1[i] != CLIQUE_UNKNOWN)
			printf(" %d ", set_Y1[i]);
	}
	printf(" }\n");
#endif

	return;
}

void form_set_Y2(int nodesize, int* set_Y2, int* set_Y1, int* sizes_of_sets_I_y)
{
	int i = CLIQUE_UNKNOWN;
	int max_val = CLIQUE_UNKNOWN;
	int curr_index = CLIQUE_UNKNOWN;

	for (i = 0; i<nodesize; i++)
	{
		if (set_Y1[i] != CLIQUE_UNKNOWN)
		{

			if (sizes_of_sets_I_y[set_Y1[i]] > max_val)
			{
				max_val = sizes_of_sets_I_y[set_Y1[i]];
			}
		}
		else
			break;
	}

	curr_index = 0;
	for (i = 0; i<nodesize; i++)
	{
		if (set_Y1[i] != CLIQUE_UNKNOWN)
		{

			if (sizes_of_sets_I_y[set_Y1[i]] == max_val)
			{
				set_Y2[curr_index] = set_Y1[i];
				curr_index++;
			}
		}
		else
			break;
	}

#ifdef DEBUG
	printf(" curr_index = %d   max_val = %d ", curr_index, max_val);
	printf(" Set Y2 = { ");
	for (i = 0; i<nodesize; i++)
	{
		if (set_Y2[i] != CLIQUE_UNKNOWN)
		{
			printf(" %d ", set_Y2[i]);
		}
		else
			break;
	}
	printf(" }\n");
#endif

	return;
}

int pick_a_node_to_merge(int* setY, int** local_compat, int* node_set, int nodesize)
{
	int** sets_I_y = (int**)NULL;
	int i = CLIQUE_UNKNOWN, j = CLIQUE_UNKNOWN;
	int* curr_indexes = (int*)NULL;
	int* set_Y1 = (int*)NULL;
	int* set_Y2 = (int*)NULL;
	int* sizes_of_sets_I_y = (int*)NULL;
	int min_val = CLIQUE_UNKNOWN;
	int new_node = CLIQUE_UNKNOWN;
	int curr_node_in_setY = CLIQUE_UNKNOWN;

	/* dynamically allocate memory for sets_I_y array */

	sets_I_y = (int**)malloc(nodesize * sizeof(int*));

	for (i = 0; i<nodesize; i++)
	{
		sets_I_y[i] = (int*)malloc(nodesize * sizeof(int));
	}

	curr_indexes = (int*)malloc(nodesize * sizeof(int));
	sizes_of_sets_I_y = (int*)malloc(nodesize * sizeof(int));

	for (i = 0; i<nodesize; i++)
	{
		curr_indexes[i] = 0;
		sizes_of_sets_I_y[i] = 0;
		for (j = 0; j<nodesize; j++)
		{
			sets_I_y[i][j] = CLIQUE_UNKNOWN;
		}
	}

	/* form I_y sets */

	for (i = 0; i<nodesize; i++)
	{
		if (setY[i] != CLIQUE_UNKNOWN) /* for each y in Y do */
		{
			for (j = 0; j<nodesize; j++)
			{
				if (node_set[j] != CLIQUE_UNKNOWN)
				{  /* if this node is still in set N */
					if (local_compat[setY[i]][j] != 1)
					{
						sets_I_y[setY[i]][curr_indexes[setY[i]]] = j;
						curr_indexes[setY[i]]++;
					}
				}
			}
		}
		else
			break;  /* end of setY */
	}

	for (i = 0; i<nodesize; i++)
	{
		if (setY[i] != CLIQUE_UNKNOWN) /* for each y in Y do */
		{
			curr_node_in_setY = setY[i];

			/* copy curr index into sizes */
			sizes_of_sets_I_y[curr_node_in_setY] = curr_indexes[curr_node_in_setY];

			/* print all I_y sets */

#ifdef DEBUG
			printf(" i= %d  nodeno= %d, curr_index = %d  ", i, curr_node_in_setY, curr_indexes[curr_node_in_setY]);

			print_setY(sets_I_y[curr_node_in_setY]);
#endif
		}
	}

	/* form set_Y1 */
	set_Y1 = (int*)malloc(nodesize * sizeof(int));
	for (i = 0; i<nodesize; i++) { set_Y1[i] = CLIQUE_UNKNOWN; }
	form_set_Y1(nodesize, setY, set_Y1, sets_I_y, node_set);

	/* form set_Y2 */
	set_Y2 = (int*)malloc(nodesize * sizeof(int));
	for (i = 0; i<nodesize; i++) { set_Y2[i] = CLIQUE_UNKNOWN; }
	form_set_Y2(nodesize, set_Y2, set_Y1, sizes_of_sets_I_y);

	if (set_Y2[0] != CLIQUE_UNKNOWN)
		new_node = set_Y2[0];

	return new_node;
}

void init_clique_set()
{
	int i = UNKNOWN, j = UNKNOWN;
	//printf("\n Initializing the clique set..");

	for (i = 0; i<MAXCLIQUES; i++)
	{
		clique_set[i].size = UNKNOWN;
		for (j = 0; j<MAXCLIQUES; j++)
		{
			clique_set[i].members[j] = UNKNOWN;
		}
	}
	//printf("..Done.\n");
}

void print_clique_set()
{
	int i = UNKNOWN, j = UNKNOWN;

	printf("\n Clique Set: \n");

	for (i = 0; i<MAXCLIQUES; i++)
	{
		if (clique_set[i].size == UNKNOWN) break;

		printf("\tClique #%d (size = %d) = { ", i, clique_set[i].size);

		for (j = 0; j<MAXCLIQUES; j++)
		{
			if (clique_set[i].members[j] != UNKNOWN)
				printf(" %d ", clique_set[i].members[j]);
			else
				break;
		}
		printf(" }\n");
	}
	printf("\n");
}


int clique_partition(int** compat, int nodesize)
{
	int** local_compat = (int**)NULL;
	int* current_clique = (int*)NULL;
	int* node_set = (int*)NULL;
	int* setY = (int*)NULL;
	int i = CLIQUE_UNKNOWN, j = CLIQUE_UNKNOWN;
	int node_x = CLIQUE_UNKNOWN, node_y = CLIQUE_UNKNOWN;
	int setY_cardinality = CLIQUE_UNKNOWN;
	int new_node = CLIQUE_UNKNOWN;
	int curr_index = CLIQUE_UNKNOWN;
	int size_N = CLIQUE_UNKNOWN;
	int clique_index = CLIQUE_UNKNOWN;
	/*int nodesize=CLIQUE_UNKNOWN;*/

	printf("\n");
	printf("**************************************\n");
	printf(" *       Clique Partitioner         *\n");
	printf("**************************************\n");
	printf("\nEntering Clique Partitioner.. \n");

	input_sanity_check(compat, nodesize);

	/* dynamically allocate memory for local copy */

	local_compat = (int**)malloc(nodesize * sizeof(int*));

	for (i = 0; i<nodesize; i++)
	{
		local_compat[i] = (int*)malloc(nodesize * sizeof(int));
	}

	make_a_local_copy(local_compat, compat, nodesize);

	printf(" You entered the compatibility array: \n");
	for (i = 0; i<nodesize; i++)
	{
		printf("\t");
		for (j = 0; j<nodesize; j++)
		{
			printf("%d ", local_compat[i][j]);
		}
		printf("\n");
	}

	init_clique_set();

	/* allocate memory for current clique & initialize to unknown values*/
	/* - current_clique has the indices of nodes that are compatible with each other*/
	/* - A node i is in node_set if node_set[i] = i */

	current_clique = (int*)malloc(nodesize * sizeof(int));
	node_set = (int*)malloc(nodesize * sizeof(int));
	setY = (int*)malloc(nodesize * sizeof(int));

	for (i = 0; i<nodesize; i++)
	{
		current_clique[i] = CLIQUE_UNKNOWN;
		node_set[i] = i;
		setY[i] = CLIQUE_UNKNOWN;
	}

	size_N = nodesize;
	curr_index = 0; /* reset the index to start for current clique */

	while (size_N > 0) /* i.e still cliques to be formed */
	{

#ifdef DEBUG
		printf("=====================================================\n");
		printf(" size_N = %d  node_set = { ", size_N);
		for (i = 0; i<nodesize; i++) {
			printf(" %d ", node_set[i]);
		}
		printf(" }\n");
#endif

		if (current_clique[0] == CLIQUE_UNKNOWN)  /* new clique formation */
		{
			node_x = select_new_node(local_compat, nodesize, node_set);
#ifdef DEBUG
			printf(" Node x = %d\n", node_x);   /* first node in the clique */
#endif
			current_clique[curr_index] = node_x;
			node_set[node_x] = CLIQUE_UNKNOWN;   /* remove node_x from N i.e node_set */
			curr_index++;
		}

		setY_cardinality = CLIQUE_UNKNOWN;
		setY_cardinality = form_setY(setY, current_clique, local_compat,
			nodesize, node_set);
#ifdef DEBUG
		print_setY(setY);
		printf(" Set Y cardinality = %d \n", setY_cardinality);
#endif


		if (setY_cardinality == 0) /* No possible nodes for merger; declare current_cliqueas complete */
		{   //printf("completing clique!\n");
			/* copy the current clique into central datastructure */
			clique_index = 0;
			while (clique_set[clique_index].size != UNKNOWN)
			{
				clique_index++;
				//printf("index = %d\n",clique_index );

			}
			//printf("index = %d size = %d\n",clique_index, clique_set[clique_index].size );
			clique_set[clique_index].size = 0;

			//printf(" A clique is found!! Clique = { ");

			for (i = 0; i< nodesize; i++)
			{
				//printf("node_index = %d\n",i );
				if (current_clique[i] != CLIQUE_UNKNOWN)
				{
					clique_set[clique_index].members[i] = current_clique[i];

					//printf(" %d ", current_clique[i]);



					node_set[current_clique[i]] = CLIQUE_UNKNOWN; /* remove this node from the node list */
					current_clique[i] = CLIQUE_UNKNOWN;
					size_N = (size_N - 1);
					(clique_set[clique_index].size)++;
				}
				else
				{
					break;
				}
			}
			//printf(" }\n");
			curr_index = 0; /* reset the curr_index for the next clique */
		}
		else
		{
			node_y = pick_a_node_to_merge(setY, local_compat, node_set, nodesize);
			current_clique[curr_index] = node_y;
			node_set[node_y] = CLIQUE_UNKNOWN;
#ifdef DEBUG
			printf(" y (new node) = %d \n", node_y);
#endif
			curr_index++;
		}
	}
	output_sanity_check(nodesize, local_compat, compat);
	printf("\n Final Clique Partitioning Results:\n");
	print_clique_set();
	printf("Exiting Clique Partitioner.. Bye.\n");
	printf("**************************************\n\n");
	return 1;
}

// int main()
// {
// int** compat;
// int i;

// compat=(int** )malloc(9 * sizeof(int*));
// for(i=0; i<9; i++)
// {
// compat[i]=(int*)malloc(9 * sizeof(int));
// }

// /*
// 1 0 0 1 0 1 0 1 0
// 0 1 0 1 0 1 0 1 0
// 0 0 1 0 0 0 0 0 0
// 1 1 0 1 0 0 0 1 0
// 0 0 0 0 1 0 0 0 0
// 1 1 0 0 0 1 0 1 0
// 0 0 0 0 0 0 1 0 0
// 1 1 0 1 0 1 0 1 0
// 0 0 0 0 0 0 0 0 1
// */
// compat[0][0]=1;     compat[0][1]=0;    compat[0][2]=0; 
// compat[0][3]=1;     compat[0][4]=0;    compat[0][5]=1;    
// compat[0][6]=0;     compat[0][7]=1;    compat[0][8]=0;  

// compat[1][0]=0;     compat[1][1]=1;    compat[1][2]=0; 
// compat[1][3]=1;     compat[1][4]=0;    compat[1][5]=1;    
// compat[1][6]=0;     compat[1][7]=1;    compat[1][8]=0;  

// compat[2][0]=0;     compat[2][1]=0;    compat[2][2]=1; 
// compat[2][3]=0;     compat[2][4]=0;    compat[2][5]=0;    
// compat[2][6]=0;     compat[2][7]=0;    compat[2][8]=0;  

// compat[3][0]=1;     compat[3][1]=1;    compat[3][2]=0; 
// compat[3][3]=1;     compat[3][4]=0;    compat[3][5]=0;    
// compat[3][6]=0;     compat[3][7]=1;    compat[3][8]=0;  

// compat[4][0]=0;     compat[4][1]=0;    compat[4][2]=0; 
// compat[4][3]=0;     compat[4][4]=1;    compat[4][5]=0;    
// compat[4][6]=0;     compat[4][7]=0;    compat[4][8]=0;  

// compat[5][0]=1;     compat[5][1]=1;    compat[5][2]=0; 
// compat[5][3]=0;     compat[5][4]=0;    compat[5][5]=1;    
// compat[5][6]=0;     compat[5][7]=1;    compat[5][8]=0;  

// compat[6][0]=0;     compat[6][1]=0;    compat[6][2]=0; 
// compat[6][3]=0;     compat[6][4]=0;    compat[6][5]=0;    
// compat[6][6]=1;     compat[6][7]=0;    compat[6][8]=0;  

// compat[7][0]=1;     compat[7][1]=1;    compat[7][2]=0; 
// compat[7][3]=1;     compat[7][4]=0;    compat[7][5]=1;    
// compat[7][6]=0;     compat[7][7]=1;    compat[7][8]=0;

// compat[8][0]=0;     compat[8][1]=0;    compat[8][2]=0; 
// compat[8][3]=0;     compat[8][4]=0;    compat[8][5]=0;    
// compat[8][6]=0;     compat[8][7]=0;    compat[8][8]=1;  


// clique_partition(compat, 9);

// /** The following is prototype code that illustrates
// how to access the clique partitioning results.
// You can modify this according to your needs.

// for(i=0; i<MAXCLIQUES; i++)
// {
// if(clique_set[i].size == UNKNOWN) break;

// printf(" Clique #%d (size = %d) = { ",i, clique_set[i].size);

// for(j=0; j<MAXCLIQUES; j++)
// {
// if(clique_set[i].members[j] != UNKNOWN)
// printf(" %d ", clique_set[i].members[j]);
// else
// break;
// }
// printf (" }\n");
// }
// **/ 
// }




