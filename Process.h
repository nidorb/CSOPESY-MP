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
    int totalWork=100;
    ProcessState currentState = READY;

    static int next_pid;

    // Constructor
    Process(string name, string timestamp) : 
        name(name), timestamp(timestamp) {
        if (!name.empty()) {
            pid = next_pid++;
        }
    }

    void setRunningToFinished() {
        if (currentState == RUNNING) {
            if (progress == totalWork) {
                currentState = FINISHED;
            }
        }
    }

    string getCurDateProc() {
        time_t now = time(0);
        struct tm tstruct;
        #ifdef _WIN32
            localtime_s(&tstruct, &now);
        #else
            localtime_r(&now, &tstruct);
        #endif
        char date_time[100];
        strftime(date_time, sizeof(date_time), "(%m/%d/%Y %I:%M:%S%p)", &tstruct);
        return date_time;
    }

    void createProcFile() {
        ofstream logs;
        logs.open(name + ".txt");
        logs << "Process name: " << name << "\n";
        logs << "Logs: \n\n";

        logs << totalWork << endl; // debug print

        for (int i = 0; i < totalWork; i++) {
            logs << getCurDateProc() << "   Core: " << cpuCoreID << "  \"Hello world from " << name << "!\"\n";
            commandCtr++;
            this_thread::sleep_for(chrono::milliseconds(100));
        }

        logs.close();
    }

    string getProcessName(){
        return name;
    }

    string getProgressString(){
        return std::to_string(progress) + " / " + std::to_string(totalWork);
    }


};

#endif
