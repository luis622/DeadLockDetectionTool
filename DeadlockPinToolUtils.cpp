// time ../../../pin -t obj-intel64/DeadlockPinTool.so -- ./deadlock_test/nodeadlock
#include "pin.H"
#include <pthread.h>
#include "DeadlockPinToolUtils.h"

extern std::ostream *out;
extern PIN_LOCK lock;
extern UINT64 Low;
extern UINT64 High;
extern UINT64 Start_addr;

// extern std::map<THREADID tid,std::string>function_map;
std::map<ADDRINT, pin_tracker*>race_map;
extern event_tracker CS[100];
extern race_issues* race_list;
extern race_issues** list_head;

Graph g;

Graph::Graph() 
{ 
    // this->V = V; 
    adj = new std::list<int32_t>[100]; 
} 

void Graph::addEdge(int32_t v, int32_t w) 
{ 
    adj[v].push_back(w); // Add w to v’s list. 
    adj[w].push_back(v); // Add w to v’s list. 
}

void Graph::removeEdge(int32_t v, int32_t w) 
{
    adj[v].remove(w);
    adj[w].remove(v);
} 

// A recursive function that uses visited[] and parent to detect 
// cycle in subgraph reachable from vertex v. 
bool Graph::isCyclicUtil(int v, bool visited[], int parent) 
{ 
    // Mark the current node as visited 
    visited[v] = true; 

    // Recur for all the vertices adjacent to this vertex 
    list<int32_t>::iterator i; 
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
    for (int j = 0; j < 100; j++) 
        if (!visited[j]) // Don't recur for u if it is already visited 
            if (isCyclicUtil(j, visited, -1)) 
                return true; 

    return false; 
} 

VOID BeforeThreadCreate(ADDRINT* lock_nam  ,THREADID threadid)
{
    PIN_GetLock(&lock, threadid+1);
    cout << "before thread create... setting event flag: " << *lock_nam << " " << threadid << endl;
    CS[threadid].in_event = EVENT;
    PIN_ReleaseLock(&lock);
}

VOID AfterThreadCreate(THREADID threadid)
{
    PIN_GetLock(&lock, threadid+1);
    cout << "after thread create passed parameters: " << threadid << endl;
    PIN_ReleaseLock(&lock);
}

VOID BeforeThreadJoin(ADDRINT lock_nam,THREADID threadid)
{
    PIN_GetLock(&lock, threadid+1);
    cout << "before thread join passed parameters: " << lock_nam << " " << threadid << endl;
    PIN_ReleaseLock(&lock);
}

VOID AfterThreadJoin(THREADID threadid)
{
    PIN_GetLock(&lock, threadid+1);
    cout << "after thread join clearing the event flag: " << threadid << endl;
    CS[threadid].in_event = NO_EVENT;
    PIN_ReleaseLock(&lock);
}

// This routine is executed each time sem wait is called.
VOID BeforeSemWait( ADDRINT size, THREADID threadid )
{
    PIN_GetLock(&lock, threadid+1);
    *out << "thread " << threadid << "entered sem[" << "]" << endl;
    PIN_ReleaseLock(&lock);
}

// This routine is executed each time sem post is called.
VOID BeforeSemPost( ADDRINT size, THREADID threadid )
{
    PIN_GetLock(&lock, threadid+1);
    *out << "thread " << threadid << "entered sem post [" << "]" << endl;
    PIN_ReleaseLock(&lock);
}

// This routine is executed each time mutex lock is called.
VOID AfterMutexLock(THREADID threadid )
{
    PIN_GetLock(&lock, threadid+1);
    CS[threadid].in_cs = SAFE;
    PIN_ReleaseLock(&lock);
}

// This routine is executed each time mutex lock is called.
VOID AfterMutexUnlock(THREADID threadid )
{
    PIN_GetLock(&lock, threadid+1);
    CS[threadid].in_cs = UNSAFE;
    g.removeEdge(CS[threadid].lock_id, threadid);
    // *out << "thread [" << threadid << "] " << "after unlock: [" << CS[threadid].lock_address << "]";
    // g.isCyclic()? *out << "\tDEADLOCK" << endl: *out << "\tSAFE" << endl;
    PIN_ReleaseLock(&lock);
}

// This routine is executed each time mutex lock is called.
VOID BeforeMutexLock(ADDRINT* lock_name, THREADID threadid )
{
    PIN_GetLock(&lock, threadid+1);
    CS[threadid].lock_id = *lock_name % 100;
    CS[threadid].lock_address = *lock_name;
    g.addEdge(threadid,  CS[threadid].lock_id);
    // *out << "thread [" << threadid << "] " << "before lock: [" << CS[threadid].lock_id << "] ";
    *out << "thread [" << threadid << "] " << "before lock: [" << (void*)*lock_name << "]";
    g.isCyclic()? *out << "\tDEADLOCK" << endl: *out << "\tSAFE" << endl;
    read_map((ADDRINT)*lock_name, threadid);
    PIN_ReleaseLock(&lock);
}

// This routine is executed each time mutex unlock is called.
VOID BeforeMutexUnlock(ADDRINT* lock_name, THREADID threadid )
{
    PIN_GetLock(&lock, threadid+1);
    CS[threadid].lock_id = *lock_name % 100;
    // *out << "thread [" << threadid << "] " << "before unlock: " << "[" << CS[threadid].lock_id << "]" << endl;
    PIN_ReleaseLock(&lock);
}

// Print a memory read record
VOID RecordMemRead( VOID * ip, VOID * addr, THREADID threadid )
{
    PIN_GetLock(&lock, threadid+1);
    if((ADDRINT)ip < High && (ADDRINT)addr > Start_addr)
    {
        *out << "thread ["<< threadid <<"] " <<"R " << addr << " ip: " << ip << endl;
    }
    PIN_ReleaseLock(&lock);
}

// Print a memory write record
VOID RecordMemWrite(VOID * ip, VOID * addr, THREADID threadid )
{
    PIN_GetLock(&lock, threadid+1);
    if((ADDRINT)ip < High && (ADDRINT)addr > Start_addr)
    {
        *out << "thread ["<< threadid <<"] "<< "W " << addr  <<  " ip: " << ip  << endl;
    }
    PIN_ReleaseLock(&lock);
}

VOID read_map(ADDRINT addr, THREADID threadid)
{

    std::map<ADDRINT,pin_tracker*>::const_iterator got = race_map.find (addr);
    pin_tracker* write_track = NULL;
    // int safe=UNSAFE;

    if ( got == race_map.end() )
    {
        *out << "address not found: " << addr << endl;
        write_track = new pin_tracker;
        write_track->threadid = threadid;
        write_track->read = false;
        write_track->shared_mem = false; 
        race_map[(ADDRINT)addr] = write_track;
    }
    else
    {
        *out << "thread [" << threadid << "] ";
        if(g.isCyclic())
        {
            std::cout << "Add to deadlock list [" << (void*)addr << "] " << std::endl;
                add_to_effected(list_head, (void*)addr, threadid);
        }
    }
}

VOID clean_map()
{
      // show content:
  for (std::map<ADDRINT,pin_tracker*>::iterator it=race_map.begin(); it!=race_map.end(); ++it)
    delete it->second;

}