#include <iostream>
#include <functional>
#include "replacement_state.h"
#include "replacement_state.cpp"
using namespace std;

int main()
{
    int timer = 0;
    bool finish = 0;
    char s[20];

    int processes, banks, queue_size, policy, hit_latency, miss_latency, marking_cap, T;
    cin >> processes >> banks >> queue_size >> policy >> hit_latency >> miss_latency >> marking_cap >> T;
    cout << processes << "\r\n";
    cout << banks << "\r\n";
    cout << queue_size << "\r\n";
    cout << policy << "\r\n";
    cout << hit_latency << "\r\n";
    cout << miss_latency << "\r\n";
    cout << marking_cap << "\r\n";
    cout << T << "\r\n";

    SCHEDULER_STATE * scheduler = new SCHEDULER_STATE( processes, banks, policy, queue_size, hit_latency, miss_latency, marking_cap );

    while (!finish) {
        int serial, process, bank, row;

        // remove reqs that 'del'=1 from queue (batch--)
        // if the current batch is cleaned: generate new batch, update loads and rank
        // DRAM take reqs from queue and set 'del'=1
        // check it's finished or not. if not, read in and print out the input
        cout.width(4);
        cout << left << timer++;
        scheduler->Update();
        if ( scheduler->is_full() ) {
            cout << "                   ";
            scheduler->PrintStats( cout );
            cout << "\r\n";
        } else {
            cin >> serial >> process >> bank >> row;
            cout << scheduler->RequestString( s, serial, process, bank, row );
            scheduler->AddRequest( serial, process, bank, row );
            scheduler->PrintStats( cout );
            cout << "\r\n";
        }

        finish = serial == T - 1;
    }

    while ( scheduler->Update() ) {
        cout.width(4);
        cout << left << timer++;
        cout << "                   ";
        scheduler->PrintStats( cout );
        cout << "\r\n";
    }

    delete scheduler;

    return 0;

}
