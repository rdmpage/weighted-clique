#include <iostream>
#include <fstream>
#include <queue>

#include <GTL/GTL.h>
#include "mygraph.h"
#include <ctime>

clock_t t0;
clock_t t1;

bool InSkipList(const node &n);
void ShowTimeUsed (clock_t &t0, clock_t &t1);
void Tokenise (std::string s, std::string delimiters, std::vector<std::string> &tokens);


//--------------------------------------------------------------------------------------------------
void ShowTimeUsed (clock_t &t0, clock_t &t1)
{
	cout << "CPU time used = " << (float)(t1 - t0)/(float)CLOCKS_PER_SEC << " seconds" << endl;
}


//--------------------------------------------------------------------------------------------------
void Tokenise (std::string s, std::string delimiters, std::vector<std::string> &tokens)
{
	tokens.erase (tokens.begin(), tokens.end());
	int start, stop;
	int n = s.length();
	start = s.find_first_not_of (delimiters);
	while ((start >= 0) && (start < n))
	{
		stop = s.find_first_of (delimiters, start);
		if ((stop < 0) || (stop > n)) stop = n;
		tokens.push_back (s.substr(start, stop - start));
		start = stop + delimiters.length();
	}
}


MyGraph G;
node_map<double> node_weight;

list <node> skip_list;

//--------------------------------------------------------------------------------------------------
bool InSkipList(const node &n)
{
	list<node>::iterator f = find(skip_list.begin(), skip_list.end(), n);
	return (f != skip_list.end());
}


//--------------------------------------------------------------------------------------------------
class CompareLastEdges
{
public:

	bool operator() (const edge &a, const edge &b)
	{
		return G.get_edge_weight(a) < G.get_edge_weight(b);
	}
};

//--------------------------------------------------------------------------------------------------
class CompareLastNodes
{
public:

	bool operator() (const node &a, const node &b)
	{
		return node_weight[a] < node_weight[b];
	}
};

//--------------------------------------------------------------------------------------------------
int main (int argc, const char * argv[]) 
{
	bool debug = false;
	
	if (argc < 2)
	{
		cout << "Usage: wc <file-name>" << endl;
		exit(1);
	}
	char filename[256];
	strcpy (filename, argv[1]);
	

  	// ---------------------------------------------------------	
  	// Read graph	
 	G.read_labels_as_weights();
	
	t0 = clock();
	GML_error err  = G.load (filename);
	t1 = clock();
	if (err.err_num != GML_OK)
	{
		cerr << "Error (" << err.err_num << ") loading graph from file \"" << filename << "\"";
		switch (err.err_num)
		{
			case GML_FILE_NOT_FOUND: cerr << "A file with that name doesn't exist."; break;
			case GML_TOO_MANY_BRACKETS: cerr << "A mismatch of brackets was detected, i.e. there were too many closing brackets (])."; break;
			case GML_OPEN_BRACKET: cerr << "Now, there were too many opening brackets ([)";  break;
			case GML_TOO_MANY_DIGITS: cerr << "The number of digits a integer or floating point value can have is limited to 1024, this should be enough :-)";  break;
			case GML_PREMATURE_EOF: cerr << "An EOF occured, where it wasn't expected, e.g. while scanning a string."; break;
			case GML_SYNTAX: cerr << "The file isn't a valid GML file, e.g. a mismatch in the key-value pairs."; break;
			case GML_UNEXPECTED: cerr << "A character occured, where it makes no sense, e.g. non-numerical characters"; break;
			case GML_OK: break;
		}
		cerr << endl;
		exit(1);
	}
	else
	{
		if (debug)
		{
			cout << "Graph read from file \"" << filename << "\" has " << G.number_of_nodes() << " nodes and " << G.number_of_edges() << " edges" << endl;
		}
	}
	if (debug)
	{
		ShowTimeUsed (t0, t1);
	}
	
	string json;
	
	json += "{\n\"clusters\":[";
	int cluster_count = 0;
	
	// 1. Sort edges in entire graph by maximum edge weight
	priority_queue<edge, vector<edge>, CompareLastEdges> q;
	graph::edge_iterator eit = G.edges_begin();
	graph::edge_iterator eend = G.edges_end ();
	while (eit != eend)
	{
		q.push(*eit);
		eit++;
	}

	while (1)
	{
		
		//------------------------------------------------------------------------------------------
		// Take "heaviest" node by arbitrarily picking one of the two nodes attached to the
		//  heaviest edge
		edge e = q.top();
		node a = e.source();
		node b = e.target();
		
		while ((InSkipList(a) || InSkipList(b)) && !q.empty())
		{
			q.pop();
			e = q.top();
			a = e.source();
			b = e.target();
		}
			
		// If no more edges we are done
		if (q.empty()) break;	
		
		if (debug)
		{
			cout << "-----------------------------\n";
			cout << "Heaviest edge: " << e << ", weight=" << G.get_edge_weight(q.top()) 
				<< ", node=" << a << " " << G.get_node_label(a) << " " << a.degree() << " " << b.degree() <<   endl;
		}
		
		// Node a forms the starting cluster
		list <node> cluster;
		// Ensure a has higher degree than b (to avoid being locally trapped)
		if (a.degree() > b.degree())
		{
			a = b;
		}
		cluster.push_back (a);
		
		bool growing = true;

		//------------------------------------------------------------------------------------------
		// Try to grow our clique
		while (growing)
		{
			if (debug)
			{
				cout << "   Current cluster=";
						list<node>::iterator lit = cluster.begin();
						list<node>::iterator lend = cluster.end();
						while (lit != lend)
						{
							cout  << "{" << G.get_node_label(*lit) << "} " ;
							lit++;
						}
				cout << "}" << endl;
			}
			
		
			//--------------------------------------------------------------------------------------
			// Find candidates to join the cluster, namely nodes connected to every node in the
			// cluster. Use a priority queue to ensure that candidates are sorted by edge weight.
	
			priority_queue<node, vector<node>, CompareLastNodes> nq;
			
			node::adj_edges_iterator it = a.adj_edges_begin();
			node::adj_edges_iterator end = a.adj_edges_end();
			while (it != end)
			{
				// v is a node connected to our initial node "a"
				node v = a.opposite (*it);
								
				// Ignore nodes already in the cluster
				list<node>::iterator f = find(cluster.begin(), cluster.end(), v);
				if (f == cluster.end())
				{
					// v is not in cluster, so take a look				
					double max_weight = 0.0;
					
					// Is v connected to all members of the existing cluster?
					int count = 0;
					list<node>::iterator lit = cluster.begin();
					list<node>::iterator lend = cluster.end();
					while (lit != lend)
					{
						edge e;
						if (G.edge_exists(*lit, v, e))
						{
							max_weight = std::max(G.get_edge_weight(e), max_weight);
							count++;
						}
						lit++;
					}
					
					// Store maximum weight of any edge between v and the cluster
					// (we use this to order the candidate nodes)
					node_weight[v] = max_weight;
					
					// v is connected to every node in the cluster, so it is a candidate
					if (count == cluster.size())
					{
						nq.push(v);
					}
				}
				it++;
			}
			
			//--------------------------------------------------------------------------------------
			// OK, now we have a list of nodes that are connected to every member of our current cluster
			// but which may also be connected to nodes outside the cluster. This list is sorted
			// by weight of the heaviest edge connecting a given node to the cluster
			
			growing = false;
			
			if (debug)
			{
				cout << "   Looking at candidate nodes" << endl;
			}
			
			while (!nq.empty() && !growing)
			{
				node candidate = nq.top();
				
				if (debug)
				{
					cout << "      Candidate node " << candidate << " \"" << G.get_node_label(candidate) << "\"" << endl;
				}
							
				//----------------------------------------------------------------------------------
				// Store neighbours of candidate node in a priority queue
				priority_queue<edge, vector<edge>, CompareLastEdges> pq;						
				node::adj_edges_iterator it = candidate.adj_edges_begin();
				node::adj_edges_iterator end = candidate.adj_edges_end();
				while (it != end)
				{
					// v is a node connected to our candidate
					node v = candidate.opposite (*it);
					pq.push(*it);
					
					it++;
				}
				
				//----------------------------------------------------------------------------------
				// If the top n neighbours are all connected to the n members of the cluster, then 
				// we can extend clique. Once we do this then we need to start again as the cluster
				// has now grown, and not all of the original candidates may be connected to the new 
				// cluster.
				double weight		= 0;		// weight of current edge
				double last_weight	= weight;	// weight of last edge looked at
				int count			= 0;		// count of nodes examined
				int found			= 0;		// count nodes that linked to cluster
				
				int neighbour_count = pq.size();
				
				// For each neighbour go down list of edges (heaviest to lightest) until
				// we have encountered all edges linking node to the cluster.
				while (!pq.empty() && (count == found) )
				{
					count++;
					edge x = pq.top();
					
					if (debug)
					{
						cout << "         " << x << " " << G.get_edge_weight(x);
					}
					
					last_weight = G.get_edge_weight(x);
					
					node n1 = x.source();
					node n2 = x.target();
					// Ensure node n1 is the candidate node
					if (n1 != candidate)
					{
						n2 = n1;
						n1 = candidate;
					}
					
					if (InSkipList(n1))
					{
						// Ignore candidate if it is already in a previously
						// found cluster
					}
					else
					{					
						// Is n2 in the current cluster?
						list<node>::iterator f = find(cluster.begin(), cluster.end(), n2);
						if (f != cluster.end())
						{
							// Yes
							if (debug)
							{
								cout << " yes" << endl;
							}
							found++;
							weight = G.get_edge_weight(x);
						}
						else
						{
							// No
							if (debug)
							{
								cout << " no" << endl;
							}
						}
					}
					pq.pop();
				}
				
				// If found < cluster size then while going down the list 
				// we've encountered a heavy node that isn't connected
				// to the cluster, and hence candidate can't be added to cluster
				if (found == cluster.size())
				{
					// We have found all the edges linked to the cluster in the top edges linked to 
					// the candidate.
					
					bool growing = false;
					if (weight > last_weight)
					{
						// All the heaviest edges are those linking the candidate to the cluster
						growing = true;
					}		
					if ((weight == last_weight) && (neighbour_count == cluster.size()) )
					{
						// Handle case where all candidate's edges are linked to the cluster
						growing = true;
					}		
					
					if (growing)
					{
						cluster.push_back(candidate);
						if (debug) cout << "      " << " add to cluster" << endl;
					}
				}
				else
				{
					if (debug) cout << "      " << "candidate connected to heavy edges not in cluster" << endl;
				}
				
				nq.pop();
			}
		}
		
		// At this point we have our cluster. Add all its nodes to the skiplist
		
		cluster_count++;
		if (cluster_count > 1)
		{
				json += ",";
		}
		json += "\n[";
		
		int c = 0;
		list<node>::iterator lit = cluster.begin();
		list<node>::iterator lend = cluster.end();
		while (lit != lend)
		{
			// Add to list of nodes to ignore
			skip_list.push_back(*lit);
			
			c++;
			if (c > 1)
			{
				json += ",";
			}
			json += " \"" + G.get_node_label(*lit) + "\"";
			
			
			
			lit++;
		}
		
		json += " ]";
		
		if (debug)
		{
			cout << "   ==========================" << endl;
			cout << "   Final cluster=";
			list<node>::iterator lit = cluster.begin();
			list<node>::iterator lend = cluster.end();
			while (lit != lend)
			{
				cout << "{" << G.get_node_label(*lit) << "} ";
				lit++;
			}
			cout << endl;
			cout << "   ==========================" << endl;
		}
					
					
		q.pop();
	
	}
	
	// Output results
	
	// Add nodes not encountered above (i.e., singletons or nodes never added to a cluster
	
	node v;
	forall_nodes(v,G)
	{
		if (!InSkipList(v))
		{
			json += ",[ \"" + G.get_node_label(v) += "\" ]";
		}
	}
	
	
	json += "]\n}";
	
	cout << json;
	
	
   return 0;
}
