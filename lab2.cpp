
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <deque>
using namespace std;

enum State
{
    Created,
    Ready,
    Run,
    Block,
    Preempt
}; // state enum set for event state

class Process
{
public:
    int AT; // Arriving Time
    int TC; // Total CPU Time
    int CB; // CPU Burst
    int IO; // IO Burst
    int prio;
    int quantum;
    Process(int arrivedTime, int totalCPU, int cpuBurst, int ioBurst, int priority, int timeSlice = 10000)
    {
        AT = arrivedTime;
        TC = totalCPU;
        CB = cpuBurst;
        IO = ioBurst;
        prio = priority;
        quantum = timeSlice;
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
    Event get_event()
    {
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

deque<Event> eventQueue; // event queue for the DES layer

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
            eventQueue.push_back(event);
        }
    }
    file.close();
}
