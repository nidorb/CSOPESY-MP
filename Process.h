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

    uint64_t commandCtr = 0;
    int cpuCoreID = -1;  // Core ID that executed OR is executing the process
    uint64_t totalWork;
    ProcessState currentState = READY;

    uint64_t quantumCtr;
    uint64_t delayCtr;

public:
    static int next_pid;
    static uint64_t MIN_INS;
    static uint64_t MAX_INS;

    static uint64_t DELAYS_PER_EXEC;

    // Constructor
    Process(const string& name) :
        name(name), timestamp(getCurDate()) {
        totalWork = rand() % (MAX_INS - MIN_INS + 1) + MIN_INS;
        pid = next_pid++;

        if (name.empty()) {
            this->name = "Process_" + to_string(pid);
        }
    }

    string getCurDate() {
        time_t now = time(0);
        struct tm tstruct;
        #ifdef _WIN32
                localtime_s(&tstruct, &now);
        #else
                localtime_r(&now, &tstruct);
        #endif

        char date_time[100];
        strftime(date_time, sizeof(date_time), "%m/%d/%Y, %I:%M:%S %p", &tstruct);

        return date_time;
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

    void createProcFile(uint64_t quantum_cycles) {
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

        quantumCtr = 0;
        delayCtr = DELAYS_PER_EXEC;
        while (quantumCtr < quantum_cycles) {
            if (commandCtr == totalWork) { break; }

            if (delayCtr > 0) {
                delayCtr--;
            }
            else {
                logs << getCurDateProc() << "   Core: " << cpuCoreID << "  \"Hello world from " << name << "!\"\n";
                commandCtr++;

                delayCtr = DELAYS_PER_EXEC;
                quantumCtr++;
            }

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

    uint64_t getCommandCtr() const {
        return commandCtr;
    }

    int getCPUCoreID() const {
        return cpuCoreID;
    }

    uint64_t getTotalWork() const {
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
