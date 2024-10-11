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
    int progress;
    int totalWork;
    ProcessState currentState = READY;

    static int next_pid;

    // Constructor
    Process(string name, string timestamp) : 
        name(name), timestamp(timestamp) {
        if (!name.empty()) {
            pid = next_pid++;
        }
    }

    void executePrintCommands(){

    }

    void setRunningToFinished() {
        if (currentState == RUNNING) {
            if (progress == totalWork) {
                currentState = FINISHED;
            }
        }
    }

    string getProcessName(){
        return name;
    }

    string getProgressString(){
        return std::to_string(progress) + " / " + std::to_string(totalWork);
    }


};

#endif
