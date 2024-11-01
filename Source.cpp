#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <fstream>
#include <mutex>
#include <unordered_map>

#include "Process.h"
#include "RRScheduler.h"
#include "FCFSScheduler.h"
#include "Scheduler.h"

using namespace std;

int NUM_CORES;
unique_ptr<Scheduler> scheduler;

string SCHEDULER_ALGO;  // fcfs or rr

int Process::next_pid = 0;
int Process::MIN_INS;
int Process::MAX_INS;
int Process::DELAYS_PER_EXEC;

int RRScheduler::QUANTUM_CYCLES;
int Scheduler::BATCH_PROCESS_FREQ;

int QUANTUM_CYCLES;
int BATCH_PROCESS_FREQ;
int MIN_INS;
int MAX_INS;
int DELAYS_PER_EXEC;

bool osRunning = false;

int isInitialized = 0;

void header()
{
    cout << "\033[38;2;233;233;233m";

    cout << "  ____ ____   ___  ____  _____ ______   __\n";
    cout << " / ___/ ___| / _ \\|  _ \\| ____/ ___\\ \\ / /\n";
    cout << "| |   \\___ \\| | | | |_) |  _| \\___  \\ V / \n";
    cout << "| |___ ___) | |_| |  __/| |___ ___) || |  \n";
    cout << " \\____|____/ \\___/|_|   |_____|____/ |_|  \n";
    cout << "\033[38;2;20;171;12mHello, Welcome to CSOPESY commandline!\033[0m\n";
    cout << "\033[38;2;239;231;158mType 'exit' to quit, 'clear' to clear the screen\n";

    cout << "\033[38;2;233;233;233m";
}

void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void processInfo(const shared_ptr<Process>& console) {
    string processName = console->getName();

    cout << "Process: " << processName << endl;
    cout << "ID: " << console->getPid() << endl << endl;
    // Retrieve updated process information from the scheduler
   // shared_ptr<Process> updatedProcess = scheduler->getProcess(processName);
    if (console->getState() == Process::READY || console->getState() == Process::RUNNING) {
        cout << "Current Instruction line: " << console->getCommandCtr() << endl;
        cout << "Lines of code: " << console->getTotalWork() << endl << endl;
    }
    else if (console->getState() == Process::FINISHED) {
        cout << "Finished!" << endl << endl;
    }
}

void drawConsole(const shared_ptr<Process>& console) {
    clearScreen();

    processInfo(console);

    string input;

    while (true) {
        cout << "root:\\> ";
        getline(cin, input);

        if (input == "exit") {
            clearScreen();
            header();
            return;
        }

        else if (input == "process-smi") {
            cout << endl;
            processInfo(console);
        }

        else {
            cout << "Unknown command \n";
        }
    }
}

shared_ptr<Process> searchProcessByName(const string& name, const vector<Process::ProcessState>& states) {
    queue<shared_ptr<Process>> tempQueue = scheduler->allProcesses;

    while (!tempQueue.empty()) {
        auto console = tempQueue.front();
        tempQueue.pop();

        if (console->getName() == name && find(states.begin(), states.end(), console->getState()) != states.end()) {
            return console;
        }
    }
    return nullptr;
}

void initialize(const string& configFilePath) {
    ifstream configFile(configFilePath);
    if (!configFile) {
        cerr << "Error opening config file." << endl;
        return;
    }

    unordered_map<string, string> configMap;
    string line;
    while (getline(configFile, line)) {
        istringstream iss(line);
        string key;
        string value;
        if (iss >> key >> value) {
            if (value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.size() - 2);
            }
            configMap[key] = value;
        }
    }

    if (configMap.find("num-cpu") != configMap.end()) {
        NUM_CORES = stoi(configMap["num-cpu"]);
    }
    if (configMap.find("scheduler") != configMap.end()) {
        SCHEDULER_ALGO = configMap["scheduler"];
    }
    if (configMap.find("quantum-cycles") != configMap.end()) {
        QUANTUM_CYCLES = stoi(configMap["quantum-cycles"]);
    }
    if (configMap.find("batch-process-freq") != configMap.end()) {
        BATCH_PROCESS_FREQ = stoi(configMap["batch-process-freq"]);
    }
    if (configMap.find("min-ins") != configMap.end()) {
        MIN_INS = stoi(configMap["min-ins"]);
    }
    if (configMap.find("max-ins") != configMap.end()) {
        MAX_INS = stoi(configMap["max-ins"]);
    }
    if (configMap.find("delays-per-exec") != configMap.end()) {
        DELAYS_PER_EXEC = stoi(configMap["delays-per-exec"]);
    }
    
    configFile.close();

    cout << "Configurations initialized:" << endl;
    cout << "NUM_CORES: " << NUM_CORES << endl;
    cout << "SCHEDULER_ALGO: " << SCHEDULER_ALGO << endl;
    cout << "QUANTUM_CYCLES: " << QUANTUM_CYCLES << endl;
    cout << "BATCH_PROCESS_FREQ: " << BATCH_PROCESS_FREQ << endl;
    cout << "MIN_INS: " << MIN_INS << endl;
    cout << "MAX_INS: " << MAX_INS << endl;
    cout << "DELAYS_PER_EXEC: " << DELAYS_PER_EXEC << endl;

    Process::MIN_INS = MIN_INS;
    Process::MAX_INS = MAX_INS;
    Process::DELAYS_PER_EXEC = DELAYS_PER_EXEC;

    RRScheduler::QUANTUM_CYCLES = QUANTUM_CYCLES;
    Scheduler::BATCH_PROCESS_FREQ = BATCH_PROCESS_FREQ;

    isInitialized++;

    if (isInitialized == 1) {
        if (SCHEDULER_ALGO == "fcfs") {
            scheduler = make_unique<FCFSScheduler>(NUM_CORES);
        }
        else if (SCHEDULER_ALGO == "rr") {
            scheduler = make_unique<RRScheduler>(NUM_CORES);
        }
        else {
            cout << "Error: Unknown scheduling algorithm." << endl;
            isInitialized = 0;
        }
    }
}


void handleInput() {
    string input;

    while (true) {
        cout << "\nroot:\\> ";
        getline(cin, input);

        if (input == "exit") {
            if (isInitialized) {
                scheduler->isRunning = false;
            }
            return;
        }
        else if (!isInitialized && input != "initialize") {
            cout << "Unknown command \n";
            continue;
        }
        else if (input == "clear") {
            clearScreen();
            header();
        }
        else if (input == "initialize") {
            initialize("config.txt");
        }
        
        else if (input == "scheduler-test") {
            scheduler->osRunning = true;
            cout << "Generating dummy processes.\n";
        }
        else if (input == "scheduler-stop") {
            scheduler->osRunning = false;
            cout << "Stopped generating dummy processes.\n";
        }
        else if (input == "report-util") {
            const string filePath = "csopesy-log.txt";
            ofstream outFile(filePath);

            if (!outFile) {
                cerr << "Error opening file for writing." << endl;
                return;
            }
            double coresUsed = 0;

            queue<shared_ptr<Process>> tempQueue1 = scheduler->allProcesses;
            while (!tempQueue1.empty()) {
                auto processPtr = tempQueue1.front();
                tempQueue1.pop();

                if (processPtr->getState() == Process::RUNNING) {
                    coresUsed++;
                }
            }

            outFile.precision(2);
            outFile << "CPU utilization: " << static_cast<int>(coresUsed * 100 / NUM_CORES) << "%" << endl;
            outFile << "Cores used: " << coresUsed << endl;
            outFile << "Cores available: " << NUM_CORES - coresUsed << endl;
        
            outFile << "\n----------------------------------------------\n";
            outFile << "Running processes:\n";

            queue<shared_ptr<Process>> tempQueue2 = scheduler->allProcesses;
            while (!tempQueue2.empty()) {
                auto processPtr = tempQueue2.front();
                tempQueue2.pop();

                if (processPtr->getState() == Process::RUNNING) {
                    outFile << processPtr->getName() << "   (" << processPtr->getTimestamp() << ")    Core:  "
                        << processPtr->getCPUCoreID() << "    " << processPtr->getProgressString() << "\n";
                }
            }


            outFile << "\nFinished processes:\n";

            queue<shared_ptr<Process>> tempQueue3 = scheduler->allProcesses;
            while (!tempQueue3.empty()) {
                auto processPtr = tempQueue3.front();
                tempQueue3.pop();

                if (processPtr->getState() == Process::FINISHED) {
                    outFile << processPtr->getName() << "   (" << processPtr->getTimestamp() << ")    Finished    "
                        << processPtr->getProgressString() << "\n";
                }
            }

            outFile << "----------------------------------------------\n";

            outFile.close();

            cout << "Report generated at " << filePath << "!" << endl;
        }

        // "screen -s <name>"
        else if (input.substr(0, 10) == "screen -s " && input.length() > 10 && input.substr(10, 1) != " ") {
            string processName = input.substr(10);

            // Check if process exists
            shared_ptr<Process> res_console = searchProcessByName(processName, { Process::READY, Process::RUNNING, Process::FINISHED });
            if (res_console == nullptr) {
                shared_ptr<Process> console = make_shared<Process>(processName);
                scheduler->readyQueue.push(console);  // Store the process console
                scheduler->allProcesses.push(console);
                drawConsole(console);
            }
            else {
                cout << "Error: '" << processName << "' name already exists.\n";
            }
        }

        // "screen -r <name>"
        else if (input.substr(0, 10) == "screen -r " && input.length() > 10) {
            string processName = input.substr(10);

            // Check if process exists
            shared_ptr<Process> res_console = searchProcessByName(processName, { Process::READY, Process::RUNNING });
            if (res_console != nullptr) {
                drawConsole(res_console);  // Process exists, allow reopening
            }
            else {
                cout << "\nProcess '" << processName << "' not found.\n";
            }
        }

        // "screen -ls"
        else if (input.substr(0, 10) == "screen -ls") { 
            double coresUsed = 0;

            queue<shared_ptr<Process>> tempQueue1 = scheduler->allProcesses;
            while (!tempQueue1.empty()) {
                auto processPtr = tempQueue1.front();
                tempQueue1.pop();

                if (processPtr->getState() == Process::RUNNING) {
                    coresUsed++;
                }
            }

            cout.precision(2);
            cout << "CPU utilization: " << static_cast<int>(coresUsed * 100 / NUM_CORES) << "%" << endl;
            cout << "Cores used: " << coresUsed << endl;
            cout << "Cores available: " << NUM_CORES - coresUsed << endl;

            cout << "\n----------------------------------------------\n";
            cout << "Running processes:\n";

            queue<shared_ptr<Process>> tempQueue2 = scheduler->allProcesses;
            while (!tempQueue2.empty()) {
                auto processPtr = tempQueue2.front();
                tempQueue2.pop();

                if (processPtr->getState() == Process::RUNNING) {
                    cout << processPtr->getName() << "   (" << processPtr->getTimestamp() << ")    Core:  "
                        << processPtr->getCPUCoreID() << "    " << processPtr->getProgressString() << "\n";
                }
            }

            // Print finished processes
            cout << "\nFinished processes:\n";

            queue<shared_ptr<Process>> tempQueue3 = scheduler->allProcesses;
            while (!tempQueue3.empty()) {
                auto processPtr = tempQueue3.front();
                tempQueue3.pop();

                if (processPtr->getState() == Process::FINISHED) {
                    cout << processPtr->getName() << "   (" << processPtr->getTimestamp() << ")    Finished    "
                        << processPtr->getProgressString() << "\n";
                }
            }

            cout << "----------------------------------------------\n";
        }

        else {
            cout << "Unknown command \n";
        }
    }
}



int main() {

    header();

    thread inputThread(handleInput);

    inputThread.join();

    return 0;
}
