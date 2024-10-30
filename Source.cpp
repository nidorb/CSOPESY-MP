#include <iostream>
#include <string>
#include <ctime>
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

int NUM_CORES = 4;
unique_ptr<Scheduler> scheduler;

string SCHEDULER_ALGO = "fcfs";  // fcfs or rr

int Process::next_pid = 0;

int QUANTUM_CYCLES = 5;
int BATCH_PROCESS_FREQ = 1;
int MIN_INS = 1000;
int MAX_INS = 2000;
int DELAYS_PER_EXEC = 0;

bool osRunning = false;
int cpuCycles = 2;
int processCtr = 0;

bool isInitialized = false;

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

void processInfo(const shared_ptr<Process>& console) {
    int curInst = 0;  // placeholder
    int totalInst = 50;  // placeholder

    cout << "PID: " << console->getPid() << endl;
    cout << "Process: " << console->getName() << endl;
    cout << "CPU Core: " << console->getCPUCoreID() << endl;
    cout << "Instruction line: " << curInst << "/" << totalInst << endl;
    cout << "Timestamp: " << console->getTimestamp() << endl;
}

void drawConsole(const shared_ptr<Process>& console) {
    clearScreen();

    processInfo(console);

    string input;

    while (true) {
        cout << ">> ";
        getline(cin, input);

        if (input == "exit") {
            clearScreen();
            header();
            return;
        }
        else {
            cout << "Unknown command \n";
        }
    }
}

shared_ptr<Process> searchProcessByName(const string& name, const vector<Process::ProcessState>& states) {
    for (const shared_ptr<Process>& console : scheduler->allProcesses) {
        if (console->getName() == name && find(states.begin(), states.end(), console->getState()) != states.end()) {
            return console;
        }
    }
    return nullptr;
}

void generateProcesses() {
    for (int i = 0; i < BATCH_PROCESS_FREQ; ++i) {
        string processName = "Process_" + to_string(processCtr);
        auto process = make_shared<Process>(processName, getCurDate());
        scheduler->readyQueue.push_back(process);
        scheduler->allProcesses.push_back(process);
        //cout << "\nGenerated process: " << processName << "\n";
        //cout << "Process Ctr: " << processCtr << " pc: " << process->getPid() << endl;
        processCtr++;
    }
}

void handleProcessGeneration() {
	while (osRunning) {
        //cout << "\nwaiting for end of Cpu Cycles ";
        this_thread::sleep_for(chrono::seconds(cpuCycles));
        //cout << "\n\nGenerating processes \n ";
		generateProcesses();
	}
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

    isInitialized = true;
}


void handleInput() {
    string input;

    while (true) {
        //cout << "val" << cpuCycles;
        cout << "\nEnter command: ";
        getline(cin, input);

        if (input == "exit") {
            scheduler->isRunning = false;
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
        else if (input == "screen") {
            cout << "screen command recognized. Doing something.\n";
        }
        else if (input == "scheduler-test") {
            osRunning = true;
            thread processGenerationThread(handleProcessGeneration);
            processGenerationThread.detach();
        }
        else if (input == "scheduler-stop") {
            osRunning = false;
            cout << "scheduler-stop command recognized. Doing something.\n";
        }
        else if (input == "report-util") {
            const string filePath = "csopesy-log.txt";
            ofstream outFile(filePath);

            if (!outFile) {
                cerr << "Error opening file for writing." << endl;
                return;
            }
            double coresUsed = 0;
            for (const shared_ptr<Process>& processPtr : scheduler->allProcesses) {
                if (processPtr->getState() == Process::RUNNING) {
                    coresUsed++;
                }
            }

            outFile.precision(2);
            outFile << "CPU utilization: " << coresUsed / NUM_CORES * 100 << "%" << endl;
            outFile << "Cores used: " << coresUsed << endl;
            outFile << "Cores available: " << NUM_CORES - coresUsed << endl;
        
            outFile << "\n----------------------------------------------\n";
            outFile << "Running processes:\n";
            for (const shared_ptr<Process>& processPtr : scheduler->allProcesses) {
                if (processPtr->getState() == Process::RUNNING) {
                    outFile << processPtr->getName() << "   (" << processPtr->getTimestamp() << ")    Core:  "
                            << processPtr->getCPUCoreID() << "    " << processPtr->getProgressString() << "\n";
                }
            }

            outFile << "\nFinished processes:\n";
            for (const shared_ptr<Process>& processPtr : scheduler->allProcesses) {
                if (processPtr->getState() == Process::FINISHED) {
                    outFile << processPtr->getName() << "   (" << processPtr->getTimestamp() << ")    Finished    "
                            << processPtr->getProgressString() << "\n";
                }
            }

            outFile << "\n\n";
            outFile << "\n----------------------------------------------\n";

            outFile.close();

            cout << "Report generated at " << filePath << "!" << endl;
        }

        // "screen -s <name>"
        else if (input.substr(0, 10) == "screen -s " && input.length() > 10) {
            string processName = input.substr(10);

            // Check if process exists
            shared_ptr<Process> res_console = searchProcessByName(processName, { Process::READY, Process::RUNNING, Process::FINISHED });
            if (res_console == nullptr) {
                shared_ptr<Process> console = make_shared<Process>(processName, getCurDate());
                drawConsole(console);
                scheduler->readyQueue.push_back(console);  // Store the process console
                scheduler->allProcesses.push_back(console);
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
                cout << "Error: '" << processName << "' does not exist. Use 'screen -s <name>'.\n";
            }
        }

        // "screen -ls"
        else if (input.substr(0, 10) == "screen -ls") { 
            double coresUsed = 0;
            for (const shared_ptr<Process>& processPtr : scheduler->allProcesses) {
                if (processPtr->getState() == Process::RUNNING) {
                    coresUsed++;
                }
            }

            cout.precision(2);
            cout << "CPU utilization: " << coresUsed / NUM_CORES * 100 << "%" << endl;
            cout << "Cores used: " << coresUsed << endl;
            cout << "Cores available: " << NUM_CORES - coresUsed << endl;

            cout << "\n----------------------------------------------\n";
            cout << "Running processes:\n";
            for (const shared_ptr<Process>& processPtr : scheduler->allProcesses) {
                if (processPtr->getState() == Process::RUNNING) {
                    cout << processPtr->getName() << "   (" << processPtr->getTimestamp() << ")    Core:  "
                        << processPtr->getCPUCoreID() << "    " << processPtr->getProgressString() << "\n";
                }
            }

            // Print finished processes
            cout << "\nFinished processes:\n";
            for (const shared_ptr<Process>& processPtr : scheduler->allProcesses) {
                if (processPtr->getState() == Process::FINISHED) {
                    cout << processPtr->getName() << "   (" << processPtr->getTimestamp() << ")    Finished    "
                        << processPtr->getProgressString() << "\n";
                }
            }
            cout << "\n----------------------------------------------\n";
            cout << "\n\n";
        }

        else {
            cout << "Unknown command \n";
        }
    }
}



int main() {
    if (SCHEDULER_ALGO == "fcfs") {
        scheduler = make_unique<FCFSScheduler>(NUM_CORES);
    }
    else if (SCHEDULER_ALGO == "rr") {
        scheduler = make_unique<RRScheduler>(NUM_CORES);
    }
    else {
        cout << "Error: Unknown scheduling algorithm." << endl;
        return 0;
    }

    header();

    thread inputThread(handleInput);

    inputThread.join();

    return 0;
}
