#include<iostream> 
#include <list>
#include <vector>
#include <limits.h>
#include <algorithm>
  
// Class for an undirected graph 
class Graph 
{ 
    //int V;                  // No. of vertices 
    std::vector<int> *adj;    // Pointer to an array containing adjacency lists 
    bool isCyclicUtil(int v, bool visited[], int parent); 
public: 
    //Graph(int V);                   // Constructor
    Graph();                   // Constructor 
    void addEdge(int v, int w);     // to add an edge to graph 
    bool isCyclic();                // returns true if there is a cycle 
}; 
  
//Graph::Graph(int V) 
Graph::Graph() 
{ 
    //this->V = V; 
    //adj = new std::vector<int>[V]; 
    adj = new std::vector<int>;
} 
  
void Graph::addEdge(int v, int w) 
{ 
    std::cout << "Vector size before " << adj->size() << std::endl;
    adj->push_back(v);//->push_back(w); // Add w to v’s list.
    //adj->push_back(w);//->push_back(v); // Add v to w’s list.
    
    	
    // Check if element 22 exists in vector
    std::vector<int>::iterator it = std::find(adj->begin(), adj->end(), v);
    
    std::cout << "Found at " << *it << std::endl;
    //adj->at(*it).push_back(w); // Add w to v’s list. 
    //adj.push_back(w).push_back(v); // Add v to w’s list. 
} 
  
// A recursive function that uses visited[] and parent to detect 
// cycle in subgraph reachable from vertex v. 
bool Graph::isCyclicUtil(int v, bool visited[], int parent) 
{ 
    // Mark the current node as visited 
    visited[v] = true; 
  
    // Recur for all the vertices adjacent to this vertex 
    std::vector<int>::iterator it; 
    for (it = adj[v].begin(); it != adj[v].end(); ++it) 
    { 
        // If an adjacent is not visited, then recur for that adjacent 
        if (!visited[*it]) 
        { 
           if (isCyclicUtil(*it, visited, v)) 
              return true; 
        } 
  
        // If an adjacent is visited and not parent of current vertex, 
        // then there is a cycle. 
        else if (*it != parent) 
           return true; 
    } 
    return false; 
} 
  
// Returns true if the graph contains a cycle, else false. 
bool Graph::isCyclic() 
{ 
    // Mark all the vertices as not visited and not part of recursion 
    // stack 
    bool *visited = new bool[adj->size()]; 
    for (int i = 0; i < adj->size(); i++) 
        visited[i] = false; 
  
    // Call the recursive helper function to detect cycle in different 
    // DFS trees 
    for (int u = 0; u < adj->size(); u++) 
        if (!visited[u]) // Don't recur for u if it is already visited 
          if (isCyclicUtil(u, visited, -1)) 
             return true; 
  
    return false; 
} 
  
int main() 
{ 
    
    Graph g1; 
    g1.addEdge(1, 0); 
    g1.addEdge(0, 2); https://www.onlinegdb.com/online_c++_compiler#tab-stdin
    g1.addEdge(2, 0); 
    g1.addEdge(0, 3); 
    g1.addEdge(3, 4); 
    g1.isCyclic()? std::cout << "Graph contains cycle\n": 
                   std::cout << "Graph doesn't contain cycle\n"; 
  
    Graph g2; 
    g2.addEdge(0, 1); 
    g2.addEdge(1, 2); 
    g2.isCyclic()? std::cout << "Graph contains cycle\n": 
                   std::cout << "Graph doesn't contain cycle\n"; 
  
    return 0; 
} 