#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
//#include <unistd.h>
#include <time.h>
#include "match.hpp"
#include "argloader.hpp"
#include "argraph.hpp"
#include "argedit.hpp"
#include "nodesorter.hpp"
#include "probability_strategy.hpp"
#include "vf3_sub_state.hpp"
#include "nodesorter.hpp"
#include "nodeclassifier.hpp"

#define TIME_LIMIT 1

template<> long long VF3SubState<int,int,Empty,Empty>::instance_count = 0;
static long long state_counter = 0;

typedef struct visitor_data_s
{
  unsigned long first_solution_time;
  long solutions;
}visitor_data_t;

bool visitor(int n, node_id ni1[], node_id ni2[], void* state, void *usr_data)
{
	/*AbstractVFState<int, int, Empty, Empty>* s = static_cast<AbstractVFState<int, int, Empty, Empty>*>(state);
	while (s)
	{
		if (!s->IsUsed())
		{
			s->SetUsed();
			state_counter++;
		}
		s = s->GetParent();
	}*/

  visitor_data_t* data = (visitor_data_t*)usr_data;
  data->solutions++;
  if(data->solutions == 1)
  {
    data->first_solution_time = clock();
  }

  return false;
}

int main(int argc, char** argv)
{

  char *pattern, *target;

  visitor_data_t vis_data;
  state_counter = 0;
  int n = 0;
  int sols = 0;
  int rep = 0;
  double timeAll = 0;
  double timeFirst = 0;  
  unsigned long firstSolTicks = 0;
  unsigned long endTicks = 0;
  unsigned long ticks = 0;
  float limit = TIME_LIMIT;

  if (argc < 3)
  {
	  std::cout << "Usage: vf3 [pattern] [target] [minimim execution time] \n";
	  return -1;
  }

  pattern = argv[1];
  target = argv[2];

  if(argc == 4)
  {
    limit = atof(argv[3]);
  }
  
  std::ifstream graphInPat(pattern);
  std::ifstream graphInTarg(target);

  StreamARGLoader<int, Empty> pattloader(graphInPat);
  StreamARGLoader<int, Empty> targloader(graphInTarg);

  ARGraph<int, Empty> patt_graph(&pattloader);
  ARGraph<int, Empty> targ_graph(&targloader);
  
  int nodes1, nodes2;
  nodes1 = patt_graph.NodeCount();
  cout << "nodes1: " << nodes1 << endl;
  nodes2 = targ_graph.NodeCount();
  node_id *n1, *n2;
  n1 = new node_id[nodes1];
  n2 = new node_id[nodes2];

  NodeClassifier<int, Empty> classifier(&targ_graph);
  NodeClassifier<int, Empty> classifier2(&patt_graph, classifier);
  std::vector<int> class_patt = classifier2.GetClasses();
  std::vector<int> class_targ = classifier.GetClasses();

  ticks = clock();
  do {
	  rep++;
    vis_data.solutions = 0;
    vis_data.first_solution_time = 0;
	  VF3NodeSorter<int, Empty, SubIsoNodeProbability<int, Empty> > sorter(&targ_graph);
	  std::vector<node_id> sorted = sorter.SortNodes(&patt_graph);

	  VF3SubState<int, int, Empty, Empty>s0(&patt_graph, &targ_graph, class_patt.data(),
		  class_targ.data(), classifier.CountClasses(), sorted.data());
	  match<VF3SubState<int, int, Empty, Empty> >(s0, &n, n1, n2, visitor, &vis_data);

  } while (clock() - ticks < CLOCKS_PER_SEC*limit);
	timeAll = ((double)(clock() - ticks) / CLOCKS_PER_SEC / rep);
  timeFirst = ((double)(vis_data.first_solution_time - ticks) / CLOCKS_PER_SEC / rep);
  	
	std::cout<<vis_data.solutions<<" "<<timeAll<<" "<<timeFirst;

  return 0;
}
