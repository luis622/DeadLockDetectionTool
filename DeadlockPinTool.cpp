
/*! @file
 *  This is an example of the PIN tool that demonstrates some basic PIN APIs 
 *  and could serve as the starting point for developing your first PIN tool
 */
#include "pin.H"
#include "DeadlockPinToolUtils.h"
#include "ll_utils.h"

#define READ 1
#define WRITE 1
#define SAFE 1
#define UNSAFE 0
#define EVENT 1
#define NO_EVENT 0

/* ================================================================== */
// Global variables 
/* ================================================================== */

UINT64 insCount = 0;        //number of dynamically executed instructions
UINT64 bblCount = 0;        //number of dynamically executed basic blocks
UINT64 threadCount = 0;     //total number of threads, including main thread
UINT64 Low = 0;
UINT64 High = 0;
UINT64 Start_addr = 0;
event_tracker CS[100];
// std::map<THREADID tid,std::string>function_map;

race_issues* race_list = NULL;
race_issues** list_head = &race_list;

#define RACE_DEBUG

std::ostream * out = &cerr;
PIN_LOCK lock;
// Force each thread's data to be in its own data cache line so that
// multiple threads do not contend for the same data cache line.
// This avoids the false sharing problem.
#define PADSIZE 56  // 64 byte line size: 64-8

// a running count of the instructions
class thread_data_t
{
  public:
    thread_data_t() : _count(0) {}
    UINT64 _count;
    UINT8 _pad[PADSIZE];
};

// key for accessing TLS storage in the threads. initialized once in main()
static  TLS_KEY tls_key = INVALID_TLS_KEY;

/* ===================================================================== */
// Command line switches
/* ===================================================================== */
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE,  "pintool",
    "o", "", "specify file name for DeadlockinTool output");

KNOB<BOOL>   KnobCount(KNOB_MODE_WRITEONCE,  "pintool",
    "count", "1", "count instructions, basic blocks and threads in the application");


/* ===================================================================== */
// Utilities
/* ===================================================================== */

/*!
 *  Print out help message.
 */
INT32 Usage()
{
    cerr << "This tool prints out the number of dynamically executed " << endl <<
            "instructions, basic blocks and threads in the application." << endl << endl;

    cerr << KNOB_BASE::StringKnobSummary() << endl;

    return -1;
}

/* ===================================================================== */
// Analysis routines
/* ===================================================================== */

// function to access thread-specific data
thread_data_t* get_tls(THREADID threadid)
{
    thread_data_t* tdata = 
          static_cast<thread_data_t*>(PIN_GetThreadData(tls_key, threadid));
    return tdata;
}

// This function is called before every block
VOID PIN_FAST_ANALYSIS_CALL docount(UINT32 c, THREADID threadid)
{
    PIN_GetLock(&lock, threadid+1);
    thread_data_t* tdata = get_tls(threadid);
    tdata->_count += c;
    PIN_ReleaseLock(&lock);
}

// Pin calls this function every time a new img is unloaded
// You can't instrument an image that is about to be unloaded
VOID ImageUnload(IMG img, VOID *v)
{
    *out << "Unloading " << IMG_Name(img) << endl;
}

// const char * StripPath(const char * path)
// {
//     const char * file = strrchr(path,'/');
//     if (file)
//         return file+1;
//     else
//         return path;
// }
std::string func_name;
// // Pin calls this function every time a new rtn is executed
// inline VOID Routine(RTN rtn, VOID *v, THREADID tid)
// {
//   func_name = RTN_Name(rtn);
// }

// This routine is executed for each image.
VOID SetupLocks(IMG img, VOID *v)
{
    RTN rtn; 
    *out << "Loading " << IMG_Name(img) << ", Image id = " << IMG_Id(img) << endl;

    // if the img is the main exectuable and not a linux lib then instrument and get the address bounds
    if(IMG_IsMainExecutable(img))
    {
        // the path can be saved and append .map to the file for later parsing
        *out << "Loading Main exe " << IMG_Name(img) << ", Image id = " << IMG_Id(img) << endl;
        *out << "PIN_LOW_ADDR " << (void*)IMG_LowAddress(img) << std::endl <<"PIN_HIGH_ADDR " << (void*)IMG_HighAddress(img) << std::endl << " start " << (void*)IMG_StartAddress(img) << endl;
        Low = IMG_LowAddress(img); // store lower address bound
        High = IMG_HighAddress(img); // store higher address bound
        Start_addr = IMG_HighAddress(img);
    
    }
    // search for all pthread lock symbols in the main exxectuable
    rtn = RTN_FindByName(img, "pthread_mutex_lock");
    
    if ( RTN_Valid( rtn ))
    {
        RTN_Open(rtn);
        
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(BeforeMutexLock),
                        IARG_FUNCARG_ENTRYPOINT_REFERENCE , 0,
                        IARG_THREAD_ID,
                        IARG_END);

        RTN_Close(rtn);
    }

    if ( RTN_Valid( rtn ))
    {
        RTN_Open(rtn);
        
        RTN_InsertCall(rtn, IPOINT_AFTER, AFUNPTR(AfterMutexLock),
                        IARG_THREAD_ID,
                        IARG_END);

        RTN_Close(rtn);
    }

    rtn = RTN_FindByName(img, "pthread_mutex_unlock");
    
    if ( RTN_Valid( rtn ))
    {
        RTN_Open(rtn);
        
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(BeforeMutexUnlock),
                        IARG_FUNCARG_ENTRYPOINT_REFERENCE , 0,
                        IARG_THREAD_ID, 
                        IARG_END);

        RTN_Close(rtn);
    }

    if ( RTN_Valid( rtn ))
    {
        RTN_Open(rtn);
        
        RTN_InsertCall(rtn, IPOINT_AFTER, AFUNPTR(AfterMutexUnlock),
                        IARG_THREAD_ID,
                        IARG_END);

        RTN_Close(rtn);
    }

    rtn = RTN_FindByName(img, "sem_wait");
    
    if ( RTN_Valid( rtn ))
    {
        RTN_Open(rtn);
        
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(BeforeSemWait),
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                       IARG_THREAD_ID, IARG_END);

        RTN_Close(rtn);
    }

    rtn = RTN_FindByName(img, "sem_post");
    
    if ( RTN_Valid( rtn ))
    {
        RTN_Open(rtn);
        
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(BeforeSemPost),
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                       IARG_THREAD_ID, IARG_END);

        RTN_Close(rtn);
    }
    rtn = RTN_FindByName(img, "pthread_create");
    
    if ( RTN_Valid( rtn ))
    {
        RTN_Open(rtn);
        
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(BeforeThreadCreate),
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                       IARG_THREAD_ID, IARG_END);

        RTN_Close(rtn);
    }

    if ( RTN_Valid( rtn ))
    {
        RTN_Open(rtn);
        
        RTN_InsertCall(rtn, IPOINT_AFTER, AFUNPTR(AfterThreadCreate),
                    IARG_THREAD_ID,
                    IARG_END);

        RTN_Close(rtn);
    }

    rtn = RTN_FindByName(img, "pthread_join");
    
    if ( RTN_Valid( rtn ))
    {
        RTN_Open(rtn);
        
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(BeforeThreadJoin),
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                       IARG_THREAD_ID, IARG_END);

        RTN_Close(rtn);
    }

    if ( RTN_Valid( rtn ))
    {
        RTN_Open(rtn);
        
        RTN_InsertCall(rtn, IPOINT_AFTER, AFUNPTR(AfterThreadJoin),
                    IARG_THREAD_ID,
                    IARG_END);

        RTN_Close(rtn);
    }
// }

}


VOID FuncCall(VOID * ip, THREADID threadid )
{
    //std::cout << "entering new call: " << endl;
    PIN_GetLock(&lock, threadid+1);
    if((ADDRINT)ip < High)
    {
        std::cout << "thread ["<< threadid <<"] " <<  " ip: " << ip  << endl;
    }
    PIN_ReleaseLock(&lock);
}


// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID *v)
{

    UINT32 memOperands = INS_MemoryOperandCount(ins);

    // Iterate over each memory operand of the instruction.
    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {

    // Find the instructions that move a value from memory to a register
    if (INS_Opcode(ins) == XED_ICLASS_MOV &&
        INS_IsMemoryRead(ins) &&
        INS_OperandIsReg(ins, 0) &&
        INS_OperandIsMemory(ins, 1) &&
        !INS_IsStackRead(ins))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp,
                // IARG_MEMORYREAD_EA,
                IARG_THREAD_ID,
                IARG_END);
        }
        // Note that in some architectures a single memory operand can be 
        // both read and written (for instance incl (%eax) on IA-32)
        // In that case we instrument it once for read and once for write.
        // only catch values that are memory writes to not stack values as stak values are thread safe
        if (INS_MemoryOperandIsWritten(ins, memOp) && !INS_IsStackWrite(ins))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp,
                // IARG_MEMORYWRITE_EA,
                IARG_THREAD_ID,
                IARG_END);
        }
    }
}
/*!
 * Increase counter of the executed basic blocks and instructions.
 * This function is called for every basic block when it is about to be executed.
 * @param[in]   numInstInBbl    number of instructions in the basic block
 * @note use atomic operations for multi-threaded applications
 */
VOID CountBbl(UINT32 numInstInBbl)
{
    bblCount++;
    insCount += numInstInBbl;
}

/* ===================================================================== */
// Instrumentation callbacks
/* ===================================================================== */

/*!
 * Insert call to the CountBbl() analysis routine before every basic block 
 * of the trace.
 * This function is called every time a new trace is encountered.
 * @param[in]   trace    trace to be instrumented
 * @param[in]   v        value specified by the tool in the TRACE_AddInstrumentFunction
 *                       function call
 */
// Pin calls this function every time a new basic block is encountered.
// It inserts a call to docount.
VOID Trace(TRACE trace, VOID *v)
{
    // Visit every basic block  in the trace
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
         // *out << "basic block " << bbl << endl;
        BBL_InsertCall(bbl, IPOINT_ANYWHERE, (AFUNPTR)docount, IARG_FAST_ANALYSIS_CALL,
                       IARG_UINT32, BBL_NumIns(bbl), IARG_THREAD_ID, IARG_END);
    }
}

/*!
 * Increase counter of threads in the application.
 * This function is called for every thread created by the application when it is
 * about to start running (including the root thread).
 * @param[in]   threadIndex     ID assigned by PIN to the new thread
 * @param[in]   ctxt            initial register state for the new thread
 * @param[in]   flags           thread creation flags (OS specific)
 * @param[in]   v               value specified by the tool in the 
 *                              PIN_AddThreadStartFunction function call
 */
VOID ThreadStart(THREADID threadIndex, CONTEXT *ctxt, INT32 flags, VOID *v)
{
     PIN_GetLock(&lock, threadIndex+1);
    threadCount++;

    //ADDRINT* TakenIP = (ADDRINT*)PIN_GetContextReg( ctxt, REG_INST_PTR );
    // cout << REG_INST_PTR << endl;

    thread_data_t* tdata = new thread_data_t;
    if (PIN_SetThreadData(tls_key, tdata, threadIndex) == FALSE)
    {
        PIN_ReleaseLock(&lock);
        *out << "PIN_SetThreadData failed" << endl;
        PIN_ExitProcess(1);
    }
     PIN_ReleaseLock(&lock);
}

// This function is called when the thread exits
VOID ThreadFini(THREADID threadIndex, const CONTEXT *ctxt, INT32 code, VOID *v)
{
    thread_data_t* tdata = static_cast<thread_data_t*>(PIN_GetThreadData(tls_key, threadIndex));
    *out << "Count[" << decstr(threadIndex) << "] = " << tdata->_count << endl;
    //delete tdata;
}

/*!
 * Print out analysis results.
 * This function is called when the application exits.
 * @param[in]   code            exit code of the application
 * @param[in]   v               value specified by the tool in the 
 *                              PIN_AddFiniFunction function call
 */
VOID Fini(INT32 code, VOID *v)
{
    
    clean_map();
    *out <<  "===============================================" << endl;
    *out <<  "DeadLockPinTool analysis results: " << endl;
    // *out <<  "Number of instructions: " << insCount  << endl;
    // *out <<  "Number of basic blocks: " << bblCount  << endl;
    *out <<  "Number of threads: " << threadCount  << endl;
    for (UINT32 t=0; t<threadCount; t++)
    {
        thread_data_t* tdata = get_tls(t);
        *out << "Count[" << decstr(t) << "]= " << tdata->_count << endl;
        delete tdata;
    }
    *out <<  "===============================================" << endl;
    *out << "START_PIN_LIST" << endl;
    log_issue_queue(list_head,out);
    *out << "END_PIN_LIST" << endl;

}

/*!
 * The main procedure of the tool.
 * This function is called when the application image is loaded but not yet started.
 * @param[in]   argc            total number of elements in the argv array
 * @param[in]   argv            array of command line arguments, 
 *                              including pin -t <toolname> -- ...
 */
int main(int argc, char *argv[])
{
    // Initialize the pin lock
    PIN_InitLock(&lock);
    PIN_InitSymbols();
    // Initialize PIN library. Print help message if -h(elp) is specified
    // in the command line or the command line is invalid 
    if( PIN_Init(argc,argv) )
    {
        return Usage();
    } 

    // PIN_InitSymbols();

    memset(&CS[0],UNSAFE,100);

        // Obtain  a key for TLS storage.
    tls_key = PIN_CreateThreadDataKey(NULL);
    if (tls_key == INVALID_TLS_KEY)
    {
        cerr << "number of already allocated keys reached the MAX_CLIENT_TLS_KEYS limit" << endl;
        PIN_ExitProcess(1);
    }
    
    string fileName = KnobOutputFile.Value();

    if (!fileName.empty()) { out = new std::ofstream(fileName.c_str());}

    if (KnobCount)
    {
        #ifdef RACE_DEBUG
        // Register ImageLoad to be called when each image is loaded.
        IMG_AddInstrumentFunction(SetupLocks, 0);
        #endif

        // Register ImageUnload to be called when an image is unloaded
        IMG_AddUnloadFunction(ImageUnload, 0);

        // Register function to be called to instrument traces
        TRACE_AddInstrumentFunction(Trace, NULL);

        // Register function to be called for every thread before it starts running
        PIN_AddThreadStartFunction(ThreadStart, NULL);

        // Register function to be called when the application exits
        PIN_AddFiniFunction(Fini, NULL);

        // Register Fini to be called when thread exits.
        PIN_AddThreadFiniFunction(ThreadFini, NULL);

        INS_AddInstrumentFunction(Instruction, 0);
    }
    
    cerr <<  "===============================================" << endl;
    cerr <<  "This application is instrumented by DeadlockPinTool" << endl;
    if (!KnobOutputFile.Value().empty()) 
    {
        cerr << "See file " << KnobOutputFile.Value() << " for analysis results" << endl;
    }
    cerr <<  "===============================================" << endl;

    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
