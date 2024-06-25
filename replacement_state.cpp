#include "replacement_state.h"
#include <stdio.h> 
#include <stdlib.h> 
using namespace std;

SCHEDULER_STATE::SCHEDULER_STATE( int _processes, int _banks, int _policy, int _queue_size, int _hit_latency, int _miss_latency, int _marking_cap )
{
    Banks = new BANK_STATE[ _banks ];
    Queue = new QUEUE_STATE[ _queue_size ];
    MaxBankLoad = new int[ _processes ];
    TotalLoad = new int[ _processes ];
    sorted_processes = new int [ _processes ];

    processes = _processes;
    banks = _banks;
    policy = _policy;
    queue_size = _queue_size;
    hit_latency = _hit_latency;
    miss_latency = _miss_latency;
    marking_cap = _marking_cap;

    mytimer = 0;
    queue_len = 0;

    Init();
}

void SCHEDULER_STATE::Init()
{
    for ( int i = 0; i < banks; i++ ) {
        Banks[ i ].timer = -1;
        Banks[ i ].mark = 1;
        Banks[ i ].start = 0;
        Banks[ i ].Marks = new int[ processes ];
        Banks[ i ].ProcessLoad = new int[ processes ];
        for ( int j = 0; j < processes; j++ ) {
            Banks[ i ].Marks[ j ] = marking_cap;
            Banks[ i ].ProcessLoad[ j ] = 0;
        }
    }

    for ( int i = 0; i < processes; i++ ) {
        MaxBankLoad[ i ] = 0;
        TotalLoad[ i ] = 0;
        sorted_processes[ i ] = processes - i - 1;
    }

    return;
}

bool SCHEDULER_STATE::is_full()
{
    return queue_len == queue_size;
}

ostream & SCHEDULER_STATE::PrintStats( ostream &out )
{

    char s[20];
    for ( int i = 0; i < banks; i++ ) {
        if ( Banks[ i ].start ) {
            out << RequestString( s, Banks[ i ].serial, Banks[ i ].process, Banks[ i ].bank, Banks[ i ].row );
        } else if ( Banks[ i ].timer > 0 ) {
            out << "   |              |";
        } else if ( Banks[ i ].timer == 0 ) {
            out << "    -------------- ";
        } else {
            out << "                   ";
        }
    }

    // for ( int i = 0; i < queue_len; i++ )
    //     out << " No." << i + 1 << ": " << RequestString( s, Queue[ i ].serial, Queue[ i ].process, Queue[ i ].bank, Queue[ i ].row );

    return out;
}

void SCHEDULER_STATE::AddRequest( int serial, int process, int bank, int row )
{
    // cout << "In AddRequest\n";
    for ( int i = queue_len; i > 0; i-- ) {
        Queue[ i ].serial  = Queue[ i - 1 ].serial;
        Queue[ i ].process = Queue[ i - 1 ].process;
        Queue[ i ].bank    = Queue[ i - 1 ].bank;
        Queue[ i ].row     = Queue[ i - 1 ].row;
        Queue[ i ].del     = Queue[ i - 1 ].del;
        Queue[ i ].mark    = Queue[ i - 1 ].mark;
        Queue[ i ].hit     = Queue[ i - 1 ].hit;
    }
    Queue[ 0 ].serial  = serial;
    Queue[ 0 ].process = process;
    Queue[ 0 ].bank    = bank;
    Queue[ 0 ].row     = row;
    Queue[ 0 ].del     = 0;
    Queue[ 0 ].mark    = 0;
    Queue[ 0 ].hit     = 0;
    queue_len++;

    return;
}

void SCHEDULER_STATE::to_string ( char *s, int i, int len )
{
    for ( int j = 0; j < len; j++ )
        s[j] = ' ';

    if ( i >= 0 && i < 10 ) {
        s[0] = '0' + i;
    } else if ( i >= 10 && i < 100 ) {
        s[0] = '0' + (i / 10);
        s[1] = '0' + (i % 10);
    } else if ( i >= 100 && i < 1000 ) {
        s[0] = '0' + ((i / 100) % 10);
        s[1] = '0' + ((i / 10) % 10);
        s[2] = '0' + (i % 10);
    } else {
        s[0] = '0' + ((i / 1000) % 10);
        s[1] = '0' + ((i / 100) % 10);
        s[2] = '0' + ((i / 10) % 10);
        s[3] = '0' + (i % 10);
    }
}

char* SCHEDULER_STATE::RequestString( char* s, int serial, int process, int bank, int row )
{
    // cout << "RequestString start\n";
    for ( int i = 0; i < 19; i++ )
        s[i] = ' ';

    s[3] = 't';
    to_string(s + 4, serial, 5);
    s[9] = 'P';
    to_string(s + 10, process, 2);
    s[12] = 'B';
    to_string(s + 13, bank, 2);
    s[15] = '(';
    to_string(s + 16, row, 2);
    s[18] = ')';
    s[19] = '\0';
    return s;
}

bool SCHEDULER_STATE::Update()
{
    if ( policy == SCHEDULER_FCFS )
        return UpdateFCFS();
    else if ( policy == SCHEDULER_FRFCFS )
        return UpdateFRFCFS();
    else if ( policy == SCHEDULER_PARBS )
        return UpdatePARBS();
}

bool SCHEDULER_STATE::UpdateFCFS()
{
    bool finish = 1;
    for ( int i = 0; i < banks; i++ ) {
        if ( Banks[ i ].timer > 0 ) {
            Banks[ i ].start = 0;
            finish = 0;
        }
        Banks[ i ].timer--;

        if ( Banks[ i ].timer < 0 ) {
            int j = queue_len - 1;
            while ( j >= 0  && Queue[ j ].bank != i ) j--;
            if ( j >= 0 ) {
                Banks[ i ].timer = ( Banks[ i ].timer == -1 && Banks[ i ].row == Queue[ j ].row )? hit_latency - 1: miss_latency - 1;
                Banks[ i ].start = 1;
                Banks[ i ].serial  = Queue[ j ].serial;
                Banks[ i ].process = Queue[ j ].process;
                Banks[ i ].bank    = Queue[ j ].bank;
                Banks[ i ].row     = Queue[ j ].row;

                while ( j <= queue_len - 1 ) {
                    Queue[ j ].serial  = Queue[ j + 1 ].serial;
                    Queue[ j ].process = Queue[ j + 1 ].process;
                    Queue[ j ].bank    = Queue[ j + 1 ].bank;
                    Queue[ j ].row     = Queue[ j + 1 ].row;
                    j++;
                }
                queue_len--;
                finish = 0;
            }
        }
    }
    return !finish;
}

bool SCHEDULER_STATE::UpdateFRFCFS()
{
    bool finish = 1;
    for ( int i = 0; i < banks; i++ ) {
        if ( Banks[ i ].timer > 0 ) {
            Banks[ i ].start = 0;
            finish = 0;
        }
        Banks[ i ].timer--;

        if ( Banks[ i ].timer < 0 ) {
            int j = queue_len - 1;
            while ( j >= 0  && ( Queue[ j ].bank != i || Queue[ j ].row != Banks[ i ].row ) ) j--;
            if ( j >= 0 ) {

                Banks[ i ].timer = ( Banks[ i ].timer == -1 && Banks[ i ].row == Queue[ j ].row )? hit_latency - 1: miss_latency - 1;
                Banks[ i ].start = 1;
                Banks[ i ].serial  = Queue[ j ].serial;
                Banks[ i ].process = Queue[ j ].process;
                Banks[ i ].bank    = Queue[ j ].bank;
                Banks[ i ].row     = Queue[ j ].row;

                while ( j <= queue_len - 1 ) {
                    Queue[ j ].serial  = Queue[ j + 1 ].serial;
                    Queue[ j ].process = Queue[ j + 1 ].process;
                    Queue[ j ].bank    = Queue[ j + 1 ].bank;
                    Queue[ j ].row     = Queue[ j + 1 ].row;
                    j++;
                }
                queue_len--;
                finish = 0;

            } else {

                j = queue_len - 1;
                while ( j >= 0  && Queue[ j ].bank != i ) j--;
                if ( j >= 0 ) {
                    Banks[ i ].timer = ( Banks[ i ].timer == -1 && Banks[ i ].row == Queue[ j ].row )? hit_latency - 1: miss_latency - 1;
                    Banks[ i ].start = 1;
                    Banks[ i ].serial  = Queue[ j ].serial;
                    Banks[ i ].process = Queue[ j ].process;
                    Banks[ i ].bank    = Queue[ j ].bank;
                    Banks[ i ].row     = Queue[ j ].row;

                    while ( j <= queue_len - 1 ) {
                        Queue[ j ].serial  = Queue[ j + 1 ].serial;
                        Queue[ j ].process = Queue[ j + 1 ].process;
                        Queue[ j ].bank    = Queue[ j + 1 ].bank;
                        Queue[ j ].row     = Queue[ j + 1 ].row;
                        j++;
                    }
                    queue_len--;
                    finish = 0;
                }

            }
        }
    }
    return !finish;
}

bool SCHEDULER_STATE::UpdatePARBS()
{
    // cout << "In UpdatePARBS\n";
    bool finish = 1;
    for ( int i = 0; i < queue_len; i++ ) {
        if ( Queue[ i ].del ) {
            for ( int j = i--; j <= queue_len - 1; j++ ) {
                Queue[ j ].serial  = Queue[ j + 1 ].serial;
                Queue[ j ].process = Queue[ j + 1 ].process;
                Queue[ j ].bank    = Queue[ j + 1 ].bank;
                Queue[ j ].row     = Queue[ j + 1 ].row;
                Queue[ j ].del     = Queue[ j + 1 ].del;
                Queue[ j ].hit     = Queue[ j + 1 ].hit;
                Queue[ j ].mark    = Queue[ j + 1 ].mark;
            }
            queue_len--;
            finish = 0;
        }
    }
    UpdateMarksAndLoads();

    for ( int i = 0; i < banks; i++ ) {

        if ( Banks[ i ].timer > 0 ) {
            Banks[ i ].start = 0;
            finish = 0;
        }
        Banks[ i ].timer = Banks[ i ].timer >= -1? Banks[ i ].timer - 1: -2;

        for ( int j = 0; j < queue_size; j++ ) {
            Queue[ j ].hit = ( Queue[ j ].bank == i ) && ( Banks[ i ].row == Queue[ j ].row );
        }


        if ( Banks[ i ].timer < 0 ) {
            // find the best request in queue
            int best_request = FindBest( i );
            if ( best_request >= 0 ) {
                Banks[ i ].timer = ( /*Banks[ i ].timer == -1 &&*/ Banks[ i ].row == Queue[ best_request ].row )? hit_latency - 1: miss_latency - 1;
                Banks[ i ].start = 1;
                Banks[ i ].serial  = Queue[ best_request ].serial;
                Banks[ i ].process = Queue[ best_request ].process;
                Banks[ i ].bank    = Queue[ best_request ].bank;
                Banks[ i ].row     = Queue[ best_request ].row;
                Banks[ i ].mark    = Queue[ best_request ].mark;

                Queue[ best_request ].del = 1;
            }
        }
    }
    return !finish;
}

int SCHEDULER_STATE::FindBest( int bank )
{
    // cout << "In FindBest\n";
    int mark_cnt = 0;
    int marked_requests[ marking_cap ];
    for ( int i = 0; i < queue_len; i++ ) {
        if ( ( Queue[ i ].bank == bank ) && Queue[ i ].mark )
            marked_requests[ mark_cnt++ ] = i;
    }
    if ( mark_cnt == 1 )
        return marked_requests[ 0 ];
    else
        return BestHit( bank, mark_cnt, marked_requests );
}

int SCHEDULER_STATE::BestHit( int bank, int mark_cnt, int *marked_requests )
{
    // cout << "In BestHit\n";
    int hit_cnt = 0;
    int hit_requests[ queue_len ];
    if ( mark_cnt == 0 ) {
        for ( int i = 0; i < queue_len; i++ ) {
            if ( ( Queue[ i ].bank == bank ) && Queue[ i ].hit )
                hit_requests[ hit_cnt++ ] = i;
        }
    } else {
        for ( int i = 0; i < mark_cnt; i++ ) {
            if ( Queue[ marked_requests[ i ] ].hit )
                hit_requests[ hit_cnt++ ] = marked_requests[ i ];
        }
    }
    if ( hit_cnt == 1 )
        return hit_requests[ 0 ];
    else if ( hit_cnt > 1 )
        return BestRank( bank, hit_cnt, hit_requests );
    else if ( hit_cnt == 0 && mark_cnt > 0 )
        return BestRank( bank, mark_cnt, marked_requests );
    else
        return BestRank( bank, 0, hit_requests ) ;
}

int SCHEDULER_STATE::BestRank( int bank, int hit_cnt, int *hit_requests )
{
    // cout << "In BestRank\n";

    int ans;
    int process;
    if ( hit_cnt > 0 ) {
        for ( int i = 0; i < processes; i++ ) {
            process = sorted_processes[ i ];
            ans = hit_cnt - 1;
            while ( ans >= 0 && Queue[ hit_requests[ ans ] ].process != process ) ans--;
            if ( ans >= 0 ) return hit_requests[ ans ];
        }
    } else {
        for ( int i = 0; i < processes; i++ ) {
            process = sorted_processes[ i ];
            ans = queue_len - 1;
            while ( ans >= 0 && ( Queue[ ans ].bank != bank || Queue[ ans ].process != process ) ) ans--;
            if ( ans >= 0 ) return ans;
        }
    }
    return ans;
}

void SCHEDULER_STATE::UpdateMarksAndLoads()
{
    // cout << "In UpdateMarksAndLoads\n";
    bool rst = 1;
    int bank, process;
    for ( int i = 0; i < queue_len; i++ ) {
        rst = (rst && !Queue[ i ].mark);
    }

    if ( !rst ) return;

    for ( int i = 0; i < processes; i++ ) {
        MaxBankLoad[ i ] = 0;
        TotalLoad[ i ] = 0;
        for ( int j = 0; j < banks; j++ ) {
            Banks[ j ].Marks[ i ] = marking_cap;
        }
    }
    for ( int i = queue_len - 1; i >= 0; i-- ) {
        bank = Queue[ i ].bank;
        process = Queue[ i ].process;
        if ( !Queue[ i ].mark && Banks[ bank ].Marks[ process ] ) {
            Banks[ bank ].Marks[ process ]--;
            TotalLoad[ process ]++;
            Queue[ i ].mark = 1;

            if ( MaxBankLoad[ process ] < (marking_cap - Banks[ bank ].Marks[ process ]) )
                MaxBankLoad[ process ]++;
        }
    }

    int min_max_bank_load = marking_cap * banks + 1;
    int prev_min_max_bank_load = -1;
    int min_process_cnt = 0;
    int min_processes[ processes ];
    int min_process_cnt_2 = 0;
    int min_processes_2[ processes ];
    for ( int i = 0; i < processes; ) {
        for ( int j = 0; j < processes; j++ ) {
            if ( MaxBankLoad[ j ] < min_max_bank_load && MaxBankLoad[ j ] > prev_min_max_bank_load ) {
                min_max_bank_load = MaxBankLoad[ j ];
                min_process_cnt = 1;
                min_processes[ 0 ] = j;
            } else if ( MaxBankLoad[ j ] == min_max_bank_load && MaxBankLoad[ j ] > prev_min_max_bank_load ) {
                min_processes[ min_process_cnt++ ] = j;
            }
        }
        if ( min_process_cnt == 1 ) {
            sorted_processes[ i++ ] = min_processes[ 0 ];
        } else {
            int tmp = min_process_cnt;
            int min_total_load = marking_cap * banks + 1;
            int prev_min_total_load = -1;
            while ( tmp ) {
                for ( int j = 0; j < min_process_cnt; j++ ) {
                    if ( TotalLoad[ min_processes[ j ] ] < min_total_load && TotalLoad[ min_processes[ j ] ] > prev_min_total_load ) {
                        min_total_load = TotalLoad[ min_processes[ j ] ];
                        min_process_cnt_2 = 1;
                        min_processes_2[ 0 ] = min_processes[ j ];
                    } else if ( TotalLoad[ min_processes[ j ] ] == min_total_load && TotalLoad[ min_processes[ j ] ] > prev_min_total_load ) {
                        min_processes_2[ min_process_cnt_2++ ] = min_processes[ j ];
                    }
                }
                tmp -= min_process_cnt_2;
                while ( min_process_cnt_2-- ) sorted_processes[ i++ ] = min_processes_2[ min_process_cnt_2 ];
                prev_min_total_load = min_total_load;
                min_total_load = marking_cap * banks + 1;
                min_process_cnt_2 = 0;
            }
        }

        prev_min_max_bank_load = min_max_bank_load;
        min_max_bank_load = marking_cap * banks + 1;
        min_process_cnt = 0;
    }
    // cout << " sorted_processes: ";
    // for ( int i = 0; i < processes; i++ )
    //     cout << sorted_processes[ i ];
    // cout << " MaxBankLoad: ";
    // for ( int i = 0; i < processes; i++ )
    //     cout << MaxBankLoad[ i ];
    // cout << " TotalLoad: ";
    // for ( int i = 0; i < processes; i++ )
    //     cout << TotalLoad[ i ];
}

