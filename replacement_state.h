#ifndef REPL_STATE_H
#define REPL_STATE_H

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This file is distributed as part of the Cache Replacement Championship     //
// workshop held in conjunction with ISCA'2010.                               //
//                                                                            //
//                                                                            //
// Everyone is granted permission to copy, modify, and/or re-distribute       //
// this software.                                                             //
//                                                                            //
// Please contact Aamer Jaleel <ajaleel@gmail.com> should you have any        //
// questions                                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <cassert>
// #include "utils.h"

// Replacement Policies Supported
typedef enum  {

    SCHEDULER_FCFS   = 0,
    SCHEDULER_FRFCFS = 1,
    SCHEDULER_PARBS  = 2

} SchedulerPolicy;

typedef struct {

    int serial;
    int process;
    int bank;
    int row;
    bool del;

    // Only for PARBS
    bool mark;
    bool hit;

} QUEUE_STATE;

typedef struct {

    int timer;
    bool start;
    int serial;
    int process;
    int bank;
    int row;
    int mark;

    // Only for PARBS
    int * Marks; // recording the left mark 
    int * ProcessLoad;

} BANK_STATE;


// The implementation for the cache replacement policy
class SCHEDULER_STATE {

  private:
    int processes;
    int banks;
    int policy;
    int queue_size;
    int hit_latency;
    int miss_latency;
    int marking_cap;

    int mytimer;  // tracks # of references to the cache
    int queue_len;
    
    BANK_STATE * Banks;
    QUEUE_STATE * Queue;

    // Only for PARBS
    int * MaxBankLoad; // calculated accroding to ProcessLoad in Banks (per process)
    int * TotalLoad; // (per process)
    int * sorted_processes;

  public:

    // The constructor CAN NOT be changed
    SCHEDULER_STATE( int _processes, int _banks, int _policy, int _queue_size, int _hit_latency, int _miss_latency, int _marking_cap );

    bool is_full();
    void AddRequest( int serial, int process, int bank, int row );
    char* RequestString( char* s, int serial, int process, int bank, int row );
    bool Update();
    bool UpdateFCFS();
    bool UpdateFRFCFS();
    bool UpdatePARBS();
    void UpdateMarksAndLoads();
    ostream& PrintStats( ostream &out );

  private:
    
    void Init();
    void to_string( char *s, int i, int len );
    
    // Only for PARBS
    int FindBest( int bank );
    int BestHit( int bank, int mark_cnt, int *marked_requests );
    int BestRank( int bank, int hit_cnt, int *hit_requests );

};


#endif
