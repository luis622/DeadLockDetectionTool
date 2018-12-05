// A C++ Program to detect cycle in an undirected graph 
#include<iostream> 
#include <list> 
#include <limits.h> 
using namespace std; 

// Class for an undirected graph 
class Graph 
{ 
	// int V; // No. of vertices 
	list<int> *adj; // Pointer to an array containing adjacency lists 
	bool isCyclicUtil(int v, bool visited[], int parent); 
public: 
	Graph(); // Constructor 
	void addEdge(int v, int w); // to add an edge to graph 
	void removeEdge(int v, int w); // to add an edge to graph
	bool isCyclic(); // returns true if there is a cycle 
}; 

Graph::Graph() 
{ 
	// this->V = V; 
	adj = new list<int>[100]; 
} 

void Graph::addEdge(int v, int w) 
{ 
	adj[v].push_back(w); // Add w to v’s list. 
	adj[w].push_back(v); // Add v to w’s list. 
}

void Graph::removeEdge(int v, int w) 
{ 
	adj[v].remove(w); // Remove w to v’s list. 
	adj[w].remove(v); // Remove v to w’s list.  
}  

// A recursive function that uses visited[] and parent to detect 
// cycle in subgraph reachable from vertex v. 
bool Graph::isCyclicUtil(int v, bool visited[], int parent) 
{ 
	// Mark the current node as visited 
	visited[v] = true; 

	// Recur for all the vertices adjacent to this vertex 
	list<int>::iterator i; 
	for (i = adj[v].begin(); i != adj[v].end(); ++i) 
	{ 
		// If an adjacent is not visited, then recur for that adjacent 
		if (!visited[*i]) 
		{ 
			if (isCyclicUtil(*i, visited, v)) 
				return true; 
		} 

		// If an adjacent is visited and not parent of current vertex, 
		// then there is a cycle. 
		else if (*i != parent) 
		return true; 
	} 
	return false; 
} 

// Returns true if the graph contains a cycle, else false. 
bool Graph::isCyclic() 
{ 
	// Mark all the vertices as not visited and not part of recursion 
	// stack 
	bool *visited = new bool[100]; 
	for (int i = 0; i < 100; i++) 
		visited[i] = false; 

	// Call the recursive helper function to detect cycle in different 
	// DFS trees 
	for (int u = 0; u < 100; u++) 
		if (!visited[u]) // Don't recur for u if it is already visited 
		if (isCyclicUtil(u, visited, -1)) 
			return true; 

	return false; 
} 

// Driver program to test above functions 
int main() 
{ 
	Graph g1;
	g1.addEdge(0, 1);	// Before lock
	cout<< "Add 0 - 1 "; g1.isCyclic()? cout << "Graph contains cycle\n": cout << "Graph doesn't contain cycle\n"; 
	// g1.addEdge(1, 0);	// After lock
	// cout<< "Add 1 - 0 "; g1.isCyclic()? cout << "Graph contains cycle\n": cout << "Graph doesn't contain cycle\n"; 
	g1.addEdge(1, 2);
	cout<< "Add 1 - 2 "; g1.isCyclic()? cout << "Graph contains cycle\n": cout << "Graph doesn't contain cycle\n"; 
	// g1.addEdge(2, 1); 
	// cout<< "Add 2 - 1 "; g1.isCyclic()? cout << "Graph contains cycle\n": cout << "Graph doesn't contain cycle\n"; 
	g1.addEdge(2, 3);
	cout<< "Add 2 - 3 "; g1.isCyclic()? cout << "Graph contains cycle\n": cout << "Graph doesn't contain cycle\n"; 
	// g1.addEdge(3, 2);  
	// cout<< "Add 3 - 2 "; g1.isCyclic()? cout << "Graph contains cycle\n": cout << "Graph doesn't contain cycle\n"; 
	g1.addEdge(3, 0);
	g1.addEdge(3, 0);
	cout<< "Add 3 - 0 "; g1.isCyclic()? cout << "Graph contains cycle\n": cout << "Graph doesn't contain cycle\n"; 
	// g1.addEdge(0, 3); 
	// cout<< "Add 0 - 3 "; g1.isCyclic()? cout << "Graph contains cycle\n": cout << "Graph doesn't contain cycle\n"; 
	g1.removeEdge(3, 0);	// Before unlock
	cout<< "Remove 3 - 0 "; g1.isCyclic()? cout << "Graph contains cycle\n": cout << "Graph doesn't contain cycle\n"; 
	// g1.removeEdge(0, 3); // After unlock
	// cout<< "Remove 0 - 3 "; g1.isCyclic()? cout << "Graph contains cycle\n": cout << "Graph doesn't contain cycle\n"; 

	cout <<endl<<endl;
	Graph g2; 
	g2.addEdge(0, 1); 
	g2.addEdge(2, 1); 
	// g2.addEdge(1, 3);
	g2.isCyclic()? cout << "Graph contains cycle\n": cout << "Graph doesn't contain cycle\n"; 

	return 0; 
} 
