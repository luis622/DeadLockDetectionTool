// A C++ Program to detect cycle in a directed graph 
// g++ -o GraphCycleDetection -lpthread -Wl,-Map=$@.map GraphCycleDetection.cpp 
#include<iostream> 
#include <list> 
#include <limits.h> 
using namespace std; 

class Graph 
{ 
    //int V; // No. of vertices 
    std::list<int> *adj; // Pointer to an array containing adjacency lists 
    bool isCyclicUtil(int v, bool visited[], bool *rs); // used by isCyclic() 
public: 
    Graph(); // Constructor 
    void addEdge(int v, int w); // to add an edge to graph 
    void removeEdge(int v, int w); // to add an edge to graph 
    bool isCyclic(); // returns true if there is a cycle in this graph 
}; 

Graph::Graph() 
{ 
    // this->V = V; 
    adj = new std::list<int>[100]; 
} 

void Graph::addEdge(int v, int w) 
{ 
    adj[v].push_back(w); // Add w to v’s list. 
}

void Graph::removeEdge(int v, int w) 
{
    adj[v].remove(w); // Remove w to v’s list. 
} 

// This function is a variation of DFSUytil() in https://www.geeksforgeeks.org/archives/18212 
bool Graph::isCyclicUtil(int v, bool visited[], bool *recStack) 
{ 
    if(visited[v] == false) 
    { 
        // Mark the current node as visited and part of recursion stack 
        visited[v] = true; 
        recStack[v] = true; 

        // Recur for all the vertices adjacent to this vertex 
        std::list<int>::iterator i; 
        for(i = adj[v].begin(); i != adj[v].end(); ++i) 
        { 
            if ( !visited[*i] && isCyclicUtil(*i, visited, recStack) ) 
                return true; 
            else if (recStack[*i]) 
                return true; 
        } 

    } 
    recStack[v] = false; // remove the vertex from recursion stack 
    return false; 
} 

// Returns true if the graph contains a cycle, else false. 
// This function is a variation of DFS() in https://www.geeksforgeeks.org/archives/18212 
bool Graph::isCyclic() 
{ 
    // Mark all the vertices as not visited and not part of recursion 
    // stack 
    bool *visited = new bool[100]; 
    bool *recStack = new bool[100]; 
    for(int i = 0; i < 100; i++) 
    { 
        visited[i] = false; 
        recStack[i] = false; 
    } 

    // Call the recursive helper function to detect cycle in different 
    // DFS trees 
    for(int i = 0; i < 100; i++) 
        if (isCyclicUtil(i, visited, recStack)) 
            return true; 

    return false; 
} 

int main() 
{ 
    // Create a graph given in the above diagram 
    Graph g1;
    g1.addEdge(0, 1);   // Before lock
    cout<< "Add 0 - 1 "; g1.isCyclic()? cout << "Graph contains cycle\n": cout << "Graph doesn't contain cycle\n"; 
    g1.addEdge(1, 0);   // After lock
    cout<< "Add 1 - 0 "; g1.isCyclic()? cout << "Graph contains cycle\n": cout << "Graph doesn't contain cycle\n"; 
    g1.addEdge(1, 2);
    cout<< "Add 1 - 2 "; g1.isCyclic()? cout << "Graph contains cycle\n": cout << "Graph doesn't contain cycle\n"; 
    g1.addEdge(2, 1); 
    cout<< "Add 2 - 1 "; g1.isCyclic()? cout << "Graph contains cycle\n": cout << "Graph doesn't contain cycle\n"; 
    g1.addEdge(2, 3);
    cout<< "Add 2 - 3 "; g1.isCyclic()? cout << "Graph contains cycle\n": cout << "Graph doesn't contain cycle\n"; 
    g1.addEdge(3, 2);  
    cout<< "Add 3 - 2 "; g1.isCyclic()? cout << "Graph contains cycle\n": cout << "Graph doesn't contain cycle\n"; 
    g1.addEdge(3, 0);
    cout<< "Add 3 - 0 "; g1.isCyclic()? cout << "Graph contains cycle\n": cout << "Graph doesn't contain cycle\n"; 
    g1.addEdge(0, 3); 
    cout<< "Add 0 - 3 "; g1.isCyclic()? cout << "Graph contains cycle\n": cout << "Graph doesn't contain cycle\n"; 
    g1.removeEdge(3, 0);    // Before unlock
    cout<< "Remove 0 - 0 "; g1.isCyclic()? cout << "Graph contains cycle\n": cout << "Graph doesn't contain cycle\n"; 
    g1.removeEdge(0, 3); // After unlock
    cout<< "Remove 0 - 3 "; g1.isCyclic()? cout << "Graph contains cycle\n": cout << "Graph doesn't contain cycle\n"; 

    Graph g2; 
    g2.addEdge(0, 1); 
    // g2.addEdge(1, 0); 
    // g2.addEdge(3, 0);
    g2.isCyclic()? cout << "Graph contains cycle\n": cout << "Graph doesn't contain cycle\n"; 
    return 0; 
} 
