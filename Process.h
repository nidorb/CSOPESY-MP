#ifndef PROCESS_H
#define PROCESS_H

using namespace std;

class Process {
public:
    enum ProcessState {
        READY,
        RUNNING,
        WAITING,
        FINISHED
    };

private:
    int pid;
    string name, timestamp;

    int commandCtr = 0;
    int cpuCoreID = -1;  // Core ID that executed OR is executing the process
    int totalWork;
    ProcessState currentState = READY;

public:
    static int next_pid;
    static int MIN_INS;
    static int MAX_INS;

    mutex mtx;

    // Constructor
    Process(const string& name, const string& timestamp) : 
        name(name), timestamp(timestamp) {
        if (!name.empty()) {
            totalWork = rand() % (MAX_INS - MIN_INS + 1) + MIN_INS;
            pid = next_pid++;
        }
    }

    void setPreemptState() {
        if (commandCtr == totalWork) {
            currentState = FINISHED;
        }
        else {
            currentState = READY;
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

    void createProcFile(int quantum_cycles) {
        currentState = RUNNING;

        ofstream logs;
        string filename = name + ".txt";

        if (commandCtr == 0) {
            logs.open(filename);

            logs << "Process name: " << name << "\n";
            logs << "Logs: \n\n";
        }
        else {
            logs.open(filename, ios::app);
        }

        for (int i = 0; i < quantum_cycles; i++) {
            if (commandCtr == totalWork) { break; }

            logs << getCurDateProc() << "   Core: " << cpuCoreID << "  \"Hello world from " << name << "!\"\n";
            commandCtr++;
            this_thread::sleep_for(chrono::milliseconds(100));
        }

        logs.close();

        setPreemptState();
    }

    string getProcessName() {
        return name;
    }

    string getProgressString() {
        return to_string(commandCtr) + " / " + to_string(totalWork);
    }

    int getPid() const {
        return pid;
    }

    string getName() const {
        return name;
    }

    string getTimestamp() const {
        return timestamp;
    }

    int getCommandCtr() const {
        return commandCtr;
    }

    int getCPUCoreID() const {
        return cpuCoreID;
    }

    int getTotalWork() const {
        return totalWork;
    }

    ProcessState getState() const {
        return currentState;
    }

    void setCPUCoreID(int cpuCoreID) {
        this->cpuCoreID = cpuCoreID;
    }
};

#endif
