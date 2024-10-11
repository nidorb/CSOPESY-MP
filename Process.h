#ifndef PROCESS_H
#define PROCESS_H

using namespace std;

class Process {
public:
    int pid;
    string name, timestamp;

    enum ProcessState {
        READY,
        RUNNING,
        WAITING,
        FINISHED
    };

    int commandCtr;
    int cpuCoreID = -1;  // Core ID that executed OR is executing the process

    ProcessState currentState = READY;

    static int next_pid;

    // Constructor
    Process(string name, string timestamp) : name(name), timestamp(timestamp) {
        if (!name.empty()) {
            pid = next_pid++;
        }
    }

    void executePrintCommands(){

    }

    string getProcessName(){
        return name;
    }
};

#endif
