
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
    int rem_cb; // whether has remaining cpu_burst or need to initialize
    Process(int arrivedTime, int totalCPU, int cpuBurst, int ioBurst, int priority, int timeSlice)
    {
        AT = arrivedTime;
        TC = totalCPU;
        CB = cpuBurst;
        IO = ioBurst;
        prio = priority;
        quantum = timeSlice;
        state_ts = arrivedTime;
        rem_cb = 0;
    }
};
class Event
{
public:
    int timeStamp;
    Process *process;
    State oldState;
    State newState;
    Event(Process *proc, State oldS, State newS, int ts)
    {
        timeStamp = ts;
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
        eventQueue.pop_front();
        return evt;
    }
    void add_event(Event* target)
    {
        // add event and insert the event according to time order
        deque<Event*>::iterator it = eventQueue.begin();
        // if timestamp is equal to this target, still the target should be behind !
        while(it != eventQueue.end()&&(*it)->timeStamp<=target->timeStamp) {
            *it++;
        }
        eventQueue.insert(it, target);
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
deque<int> randvals;
int THE_QUANTUM, MAX_PRIO;
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
        case Ready: {
            // must come from BLOCKED or from PREEMPTION 
            // must add to run queue
            THE_SCHEDULER->add_process(evt->process);
            Event* newEvt = new Event(evt->process, Ready, Run, CURRENT_TIME);
            desLayer->add_event(newEvt);
            if(CURRENT_RUNNING_PROCESS==nullptr) {
                // conditional on whether something is run
                CALL_SCHEDULER = true;
            }
            break;
        }
        case Run: {
            // create event for either preemption or blocking
            int next_cpu_burst = proc->rem_cb==0?min(myrandom(proc->CB), proc->TC):proc->rem_cb; // generate random int for cpu_burst
            proc->rem_cb = next_cpu_burst;
            Event* nextEvt;
            if(proc->quantum>next_cpu_burst) {
                // turn to blocking state
                proc->TC -= next_cpu_burst;
                proc->quantum -= next_cpu_burst;
                proc->rem_cb -= next_cpu_burst;
                nextEvt = new Event(proc, Run, Block, CURRENT_TIME+next_cpu_burst);
            } else {
                // if quantum assigned to this process is less than the current cpu_burst, then minus 
                // remaining quantum and turn the process to preemption state
                proc->TC -= proc->quantum;
                proc->rem_cb -= proc->quantum;
                nextEvt = new Event(proc, Run, Preempt, CURRENT_TIME+proc->quantum);
                // TODO 
                // option1 minus process quantum to 0 and reassign the quantum when the process will run
                // option2 just turn it to full quantum (using now)
                proc->quantum = THE_QUANTUM;
            }
            // add new event to right position in DES layer
            // if the process has been terminated, then don't add new event
            if(proc->TC!=0) {
                desLayer->add_event(nextEvt);
            } else {
                delete nextEvt;
                nextEvt = nullptr;
            }
            break;
        }
        case Block: {
            //create an event for when process becomes READY again
            Event* nextEvt;
            int next_io_burst = myrandom(proc->IO);
            nextEvt = new Event(proc, Block, Ready, CURRENT_TIME+next_io_burst);
            CALL_SCHEDULER = true;
            break;
        }
        case Preempt: {
            // add to runqueue (no event is generated)
            THE_SCHEDULER->add_process(proc);
            CALL_SCHEDULER = true;
            break;
        }
        case Created: {
            // just to avoid warning nothing to do here
            break;
        }
            
        
        }

        // remove current event object from Memory 
        delete evt;
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

// set up random generator using rand number from rfile
int myrandom(int burst) { 
    int nextRand = randvals.front();
    int rst = 1 + (nextRand % burst); 
    randvals.pop_front();
    randvals.push_back(nextRand);
    return rst;
}

int main(int argc, char *argv[])
{
    // initialize scheduler global variable 
    MAX_PRIO = 4;
    THE_QUANTUM = 10000;
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
            Process proc(arrivedTime, totalCPU, cpuBurst, ioBurst, MAX_PRIO, THE_QUANTUM);
            Event event(&proc, Created, Ready, arrivedTime);
            desLayer->eventQueue.push_back(&event);
        }
    }
    file.close();
    // read random number from rfile
    fstream randFile;
    randFile.open("./input/rfile", fstream::in);
    string line;
    getline(randFile, line, '\n');
    int randCnt = stoi(line);
    while(!randFile.eof()) {
        getline(randFile, line, '\n');
        int randNum = stoi(line);
        randvals.push_back(randNum);
    }
    randFile.close();
}
