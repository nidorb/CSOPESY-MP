
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
    int cpuCoreID = -1;

    ProcessState currentState;

    // Constructor
    Process(string name, string timestamp) : name(name), timestamp(timestamp) {}
};
