
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <deque>
#include <queue>
#include <map>
using namespace std;
// Some probs I need to consider later
// 1. CURRENT_RUNNING_PROCESS setup
// 2. when to reinitialize quantum
// 3. the use of test_preempt (not in use right now)
enum State
{
    Created,
    Ready,
    Run,
    Block,
    Preempt,
    Done
}; // state enum set for event state

class Process;
class Event;
class DES;
int myrandom(int burst);
char* stateConvert(State target);

class Process
{
public:
    int AT; // Arriving Time
    int TC; // Total CPU Time
    int CB; // CPU Burst
    int IO; // IO Burst

    int FT; // Fiinishing time
    int TT; // Tournaround time
    int IT; // I/O Time
    int PRIO;
    int CW; // CPU Waiting
    int RT; // Remaining CPU Time
    int pid;
    int quantum;
    int state_ts;
    int rem_cb; // whether has remaining cpu_burst or need to initialize
    Process(int id, int arrivedTime, int totalCPU, int cpuBurst, int ioBurst, int priority, int timeSlice)
    {
        pid = id;
        AT = arrivedTime;
        TC = totalCPU;
        CB = cpuBurst;
        IO = ioBurst;
        CW = 0;
        PRIO = priority;
        quantum = timeSlice;
        state_ts = arrivedTime;
        rem_cb = 0;
        RT = TC;
        IT = 0;
    }
};

class Scheduler
{
public:
    queue<Process*> runQueue;
    virtual void add_process(Process* process) = 0;
    virtual Process* get_next_process() = 0;
    virtual bool test_preempt(Process *p, int curtime ) = 0; // false but for ‘E’
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
        Process* next;
        if(runQueue.empty()) {
            return nullptr;
        } else {
            next = runQueue.front();
            runQueue.pop();
            while(next->RT==0&&!runQueue.empty()) {
                next = runQueue.front();
                runQueue.pop();
            }
            if(next->RT==0) {
                return nullptr;
            } else {
                return next;
            }
        }
        
    }
    bool test_preempt(Process *p, int curtime)
    {
        // false but for ‘E’
        return false;
    }; 
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
        if(eventQueue.empty()) {
            return -1;
        } else {
            return eventQueue.front()->timeStamp;
        }
    }
    Event* get_event()
    {
        if(eventQueue.empty()) {
            return nullptr;
        } else {
            Event *evt = eventQueue.front();
            eventQueue.pop_front();
            return evt;
        }
    }
    void add_event(Event* target)
    {
        char* originalEvtQ = printEventQ();
        // check Done event 
        if(target->oldState==Ready&&target->newState==Run) {
            int doneTimeStamp = -1;
            for(int i=0; i<eventQueue.size(); i++) {
                Event* cur = eventQueue.at(i);
                if(cur->newState==Done) {
                    doneTimeStamp = cur->timeStamp;
                }
            }
            if(target->timeStamp<doneTimeStamp) {
                target->timeStamp = doneTimeStamp;
            }
        }

        if(eventQueue.size()>0) {
            // add event and insert the event according to time order
            deque<Event*>::iterator it = eventQueue.begin();
            // if timestamp is equal to this target, still the target should be behind !
            while(it != eventQueue.end()&&(*it)->timeStamp<=target->timeStamp) {
                *it++;
            }
            eventQueue.insert(it, target);
        } else {
            eventQueue.push_back(target);
        }
        char* newEvtQ = printEventQ();
        printf("AddEvent(%d:%d:%s): %s ==> %s\n", target->timeStamp, target->process->pid, stateConvert(target->newState), originalEvtQ, newEvtQ);
    }
    void rm_event()
    {
    }
    char* printEventQ() {
        string rst = "";
        for(int i=0; i<eventQueue.size(); i++) {
            Event* evt = eventQueue.at(i);
            string eStr = "";
            eStr += (to_string(evt->timeStamp)+":");
            eStr += (to_string(evt->process->pid)+":");
            eStr += (stateConvert(evt->newState));
            if(i!=eventQueue.size()-1) eStr += " ";
            rst += eStr;
        }
        char * cstr = new char [rst.length()+1];
        strcpy (cstr, rst.c_str());
        return cstr;
        
    }
    
};

// some global variable
// define scheduler
string SCHEDULER_NAME;
Scheduler* THE_SCHEDULER = new FCFS();
bool CALL_SCHEDULER = false;
int CURRENT_TIME, timeInPrevState;
Process* CURRENT_RUNNING_PROCESS;
DES* desLayer = new DES();
deque<int> randvals;
int THE_QUANTUM, MAX_PRIO;
vector< vector<int> > io_list;
vector<Process*> procList;

char* stateConvert(State target) {
    string rst;
    switch(target) {
        case (Created): {
            rst =  "CREATED";
            break;
        }
        case(Ready): {
            rst = "READY";
            break;
        }
        case(Run): {
            rst = "RUNNG";
            break;
        }
        case(Block): {
            rst = "BLOCK";
            break;
        }
        case(Preempt): {
            rst = "PREEMPT";
            break;
        }
        case(Done): {
            rst = "Done";
            break;
        }
        
    };

    char * cstr = new char [rst.length()+1];
    std::strcpy (cstr, rst.c_str());
    return cstr;
}


int computeSumryIO(vector<vector<int> >& intervals) {
        sort(intervals.begin(), intervals.end());
        int sumryIO = 0;
        vector<vector<int> > merged;
        for (int i=0; i<intervals.size(); i++) {
            vector<int> interval = intervals[i];
            if (merged.empty() || merged.back()[1] < interval[0]) {
                merged.push_back(interval);
            }
            else {
                merged.back()[1] = max(merged.back()[1], interval[1]);
            }
        }
        for(int i=0; i<merged.size(); i++) {
            vector<int> cur = merged[i];
            sumryIO += (cur[1]-cur[0]);
        }
        return sumryIO;
}
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
            printf("%d %d %d: %s -> %s\n", CURRENT_TIME, evt->process->pid, timeInPrevState, stateConvert(evt->oldState), stateConvert(evt->newState));
            THE_SCHEDULER->add_process(proc);
            // Event* newEvt = new Event(proc, Ready, Run, CURRENT_TIME);
            // desLayer->add_event(newEvt);
            if(CURRENT_RUNNING_PROCESS==nullptr) {
                // conditional on whether something is run
                CALL_SCHEDULER = true;
            } 
            break;
        }
        case Run: {
            // create event for either preemption or blocking
            // set CURRENT_RUNNING_PROCESS
            proc->CW += (CURRENT_TIME-proc->state_ts);
            CURRENT_RUNNING_PROCESS = proc;
            // generate random int for cpu_burst
            // notice that we need compare to remaining time of this process RT
            int next_cpu_burst = proc->rem_cb==0?min(myrandom(proc->CB), proc->RT):min(proc->rem_cb, proc->RT); 
            proc->rem_cb = next_cpu_burst;
            printf("%d %d %d: %s -> %s cb=%d rem=%d prio=%d\n", CURRENT_TIME, evt->process->pid, timeInPrevState, stateConvert(evt->oldState), stateConvert(evt->newState), next_cpu_burst, proc->RT, proc->PRIO-1);
            Event* nextEvt;
            if(proc->quantum>next_cpu_burst) {
                // turn to blocking state
                proc->RT -= next_cpu_burst;
                proc->quantum -= next_cpu_burst;
                proc->rem_cb -= next_cpu_burst;
                nextEvt = new Event(proc, Run, Block, CURRENT_TIME+next_cpu_burst);
            } else {
                // if quantum assigned to this process is less than the current cpu_burst, then minus 
                // remaining quantum and turn the process to preemption state
                proc->RT -= proc->quantum;
                proc->rem_cb -= proc->quantum;
                nextEvt = new Event(proc, Run, Preempt, CURRENT_TIME+proc->quantum);
                // TODO 
                // option1 minus process quantum to 0 and reassign the quantum when the process will run
                // option2 just turn it to full quantum (using now)
                proc->quantum = THE_QUANTUM;
            }
            // add new event to right position in DES layer
            // if the process has been terminated, then don't add new event
            if(proc->RT!=0) {
                desLayer->add_event(nextEvt);
            } else {
                // update CURRENT_TIME in case event queue is empty 
                // and new event will be created for incoming process with proper time
                CURRENT_TIME = nextEvt->timeStamp;
                // update event queue with right time
                // in specific ready->run event whose timestamp is before this current time should be delayed with current cpu burst
                Event* doneEvt = new Event(proc, Run, Done, CURRENT_TIME);
                for(int i=0; i<desLayer->eventQueue.size(); i++) {
                    Event* cur = desLayer->eventQueue.at(i);
                    if(cur->timeStamp>=CURRENT_TIME) {
                        break;
                    }
                    if(cur->timeStamp<CURRENT_TIME&&cur->oldState==Ready&&cur->newState==Run) {
                        cur->timeStamp += (nextEvt->timeStamp-evt->timeStamp);
                    }
                }
                desLayer->add_event(doneEvt);
                // set Finishing time
                proc->FT = nextEvt->timeStamp;
                // set Tournaround time 
                proc->TT = proc->FT-proc->AT;
                CURRENT_RUNNING_PROCESS = nullptr;
                delete nextEvt;
                nextEvt = nullptr;
                CALL_SCHEDULER = true;
            }
            break;
        }
        case Block: {
            //create an event for when process becomes READY again
            // set CURRENT_RUNNING_PROCESS
            CURRENT_RUNNING_PROCESS = nullptr;
            Event* nextEvt;
            int next_io_burst = myrandom(proc->IO);
            // add to io utilization list
            vector<int> nextIOPair;
            nextIOPair.push_back(CURRENT_TIME);
            nextIOPair.push_back(CURRENT_TIME+next_io_burst);
            io_list.push_back(nextIOPair);
            // add io burst time to process set IT
            proc->IT += next_io_burst;
            printf("%d %d %d: %s -> %s  ib=%d rem=%d\n", CURRENT_TIME, evt->process->pid, timeInPrevState, stateConvert(evt->oldState), stateConvert(evt->newState), next_io_burst, proc->RT);
            nextEvt = new Event(proc, Block, Ready, CURRENT_TIME+next_io_burst);
            desLayer->add_event(nextEvt);
            CALL_SCHEDULER = true;
            break;
        }
        case Preempt: {
            // add to runqueue (no event is generated)
            // set CURRENT_RUNNING_PROCESS
            CURRENT_RUNNING_PROCESS = nullptr;
            THE_SCHEDULER->add_process(proc);
            CALL_SCHEDULER = true;
            break;
        }
        case Done: {
            printf("%d %d %d: Done\n", CURRENT_TIME, evt->process->pid, timeInPrevState);
            break;
        }
        case Created: {
            // just to avoid warning nothing to do here
            break;
        }
            
        }
        // update the state timestamp for process
        proc->state_ts = CURRENT_TIME;
        if(proc->RT==0) proc->state_ts = evt->timeStamp;

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
                Event* nextEvt = new Event(CURRENT_RUNNING_PROCESS, Ready, Run, CURRENT_TIME);
                desLayer->add_event(nextEvt);
            }
        }
    }
}

// Summary is used to print standard output of this simulation
void Summary() {
    printf("%s\n", SCHEDULER_NAME.c_str());
    vector<Process*>::iterator procIte = procList.begin();
    int procCnt = procList.size();
    int sumryFT = INT_MIN; // Finishing time of the last event (i.e. the last process finished execution)
    int sumryCPU = 0; 
    int sumryTT = 0;
    int sumryCW = 0;
    int minAT = INT_MAX;
    while(procIte!=procList.end()) {
        Process* cur = *procIte;
        printf("%04d: %4d %4d %4d %4d %1d | %5d %5d %5d %5d\n", cur->pid, cur->AT, cur->TC, cur->CB, cur->IO, cur->PRIO, cur->FT, cur->TT, cur->IT, cur->CW);
        minAT = min(cur->AT, minAT);
        sumryFT = max(cur->FT, sumryFT);
        sumryCPU += cur->TC;
        sumryTT += cur->TT;
        sumryCW += cur->CW;
        *procIte++;
    }
    int sumryIO = computeSumryIO(io_list);
    double sumryCPUUtil = 100.0*(sumryCPU/(double)sumryFT); // CPU utilization (i.e. percentage (0.0 – 100.0) of time at least one process is running
    double sumryIOUtil = 100.0*(sumryIO/(double)sumryFT); // IO utilization (i.e. percentage (0.0 – 100.0) of time at least one process is performing IO
    double sumryAveTT = (sumryTT*1.0)/procCnt; // Average turnaround time among processes
    double sumryAveCW = (sumryCW*1.0)/procCnt; // Average cpu waiting time among processes
    
    double sumryThroughput = 100.0*(procCnt/(double)sumryFT); // Throughput of number processes per 100 time units
    printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n", sumryFT, sumryCPUUtil, sumryIOUtil, sumryAveTT, sumryAveCW, sumryThroughput);
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
    SCHEDULER_NAME = "FCFS";
    MAX_PRIO = 4;
    THE_QUANTUM = 10000;
    
    // read random number from rfile
    fstream randFile;
    randFile.open("./input/rfile", fstream::in);
    string line;
    getline(randFile, line, '\n');
    int randCnt = stoi(line);
    while(!randFile.eof()) {
        getline(randFile, line, '\n');
        if(line.length()>0) {
            int randNum = stoi(line);
            randvals.push_back(randNum);
        }
    }
    randFile.close();
    // read input file
    fstream file;
    file.open("./input/input4", fstream::in);
    int procCnt = 0; // used to signal process id
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
            int prio = myrandom(MAX_PRIO);
            Process* proc = new Process(procCnt, arrivedTime, totalCPU, cpuBurst, ioBurst, prio, THE_QUANTUM);
            procList.push_back(proc);
            Event* event = new Event(proc, Created, Ready, arrivedTime);
            desLayer->add_event(event);
            procCnt ++;
        }
    }
    file.close();
    // Simulation Part
    Simulation();
    Summary();
}
