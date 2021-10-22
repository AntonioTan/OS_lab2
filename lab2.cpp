
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <deque>
#include <queue>
using namespace std;

enum State
{
    Created,
    Ready,
    Run,
    Block,
    Preempt
}; // state enum set for event state

class Process;
class Event;
class DES;

class Scheduler
{
public:
    queue<Process*> runQueue;
    virtual void add_process(Process* process) = 0;
    virtual Process* get_next_process() = 0;
    virtual ~Scheduler() {}
};

class FCFS: public Scheduler
{
public:
    FCFS()
    {
    }
    void add_process(Process* process)
    {
        runQueue.push(process);
    }
    Process* get_next_process()
    {
        Process* next = runQueue.front();
        runQueue.pop();
        return next;
    }
};

class Process
{
public:
    int AT; // Arriving Time
    int TC; // Total CPU Time
    int CB; // CPU Burst
    int IO; // IO Burst
    int prio;
    int quantum;
    int state_ts;
    Process(int arrivedTime, int totalCPU, int cpuBurst, int ioBurst, int priority, int timeSlice = 10000)
    {
        AT = arrivedTime;
        TC = totalCPU;
        CB = cpuBurst;
        IO = ioBurst;
        prio = priority;
        quantum = timeSlice;
        state_ts = arrivedTime;
    }
};
class Event
{
public:
    int timeStamp;
    Process *process;
    State oldState;
    State newState;
    Event(Process *proc, State oldS, State newS)
    {
        process = proc;
        oldState = oldS;
        newState = newS;
    }
    
};
class DES
{
public:
    deque<Event*> eventQueue; // event queue for the DES layer
    int get_next_event_time() {
        return eventQueue.front()->timeStamp;
    }
    Event* get_event()
    {
        Event *evt = DES::eventQueue.front();
        DES::eventQueue.pop_front();
        return evt;
    }


    void add_event(Event target)
    {
    }
    void put_event()
    {
    }
    void rm_event()
    {
    }
    
};

// some global variable
// define scheduler
Scheduler* THE_SCHEDULER = new FCFS();
bool CALL_SCHEDULER = false;
int CURRENT_TIME, timeInPrevState;
Process* CURRENT_RUNNING_PROCESS;
DES* desLayer = new DES();
void Simulation()
{
    Event *evt;
    while ((evt = desLayer->get_event()))
    {
        Process *proc = evt->process; // this is the process the event works on 
        CURRENT_TIME = evt->timeStamp; 
        int temp = CURRENT_TIME-proc->state_ts;
        timeInPrevState = CURRENT_TIME - proc->state_ts;

        switch (evt->newState)
        { // which state to transition to?
        case Ready:

            // must come from BLOCKED or from PREEMPTION // must add to run queue
            CALL_SCHEDULER = true; // conditional on whether something is run
            break;
        case Run:

            // create event for either preemption or blocking
            break;
        case Block:

            //create an event for when process becomes READY again
            CALL_SCHEDULER = true;
            break;
        case Preempt:
            // add to runqueue (no event is generated)
            CALL_SCHEDULER = true;
            break;
        case Created:
            // just to avoid warning nothing to do here
            break;
        
        }

        // remove current event object from Memory delete evt;
        evt = nullptr;

        if (CALL_SCHEDULER)
        {
            if (desLayer->get_next_event_time() == CURRENT_TIME)
                continue;           //process next event from Event queue
            CALL_SCHEDULER = false; // reset global flag
            if (CURRENT_RUNNING_PROCESS == nullptr)
            {
                CURRENT_RUNNING_PROCESS = THE_SCHEDULER->get_next_process();
                if (CURRENT_RUNNING_PROCESS == nullptr)
                    continue;

                // create event to make this process runnable for same time.
            }
        }
    }
}

int main(int argc, char *argv[])
{
    int maxPrio = 4;
    // read input file
    fstream file;
    file.open("./input/input0", fstream::in);
    while (!file.eof())
    {
        string line;
        getline(file, line, '\n');
        istringstream iss(line);
        vector<string> tokens;
        copy(istream_iterator<string>(iss),
             istream_iterator<string>(),
             back_inserter(tokens));
        // FCFS mode
        if (tokens.size() == 4)
        {
            int arrivedTime = stoi(tokens.at(0));
            int totalCPU = stoi(tokens.at(1));
            int cpuBurst = stoi(tokens.at(2));
            int ioBurst = stoi(tokens.at(3));
            Process proc(arrivedTime, totalCPU, cpuBurst, ioBurst, maxPrio);
            Event event(&proc, Created, Ready);
            desLayer->eventQueue.push_back(&event);
        }
    }
    file.close();
}
