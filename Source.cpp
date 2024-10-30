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

int NUM_CORES;
unique_ptr<Scheduler> scheduler;

string SCHEDULER_ALGO;  // fcfs or rr

int Process::next_pid = 0;

int QUANTUM_CYCLES;
int BATCH_PROCESS_FREQ;
int MIN_INS;
int MAX_INS;
int DELAYS_PER_EXEC;

bool osRunning = false;
int cpuCycles = 2;
int processCtr = 0;

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
        cout << "root:\>> ";
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
        //cout << "val" << cpuCycles;
        cout << "\nEnter command: ";
        getline(cin, input);

        if (input == "exit") {
            scheduler->isRunning = false;
            return;
        }
        else if (isInitialized == 0 && input != "initialize") {
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
            outFile << "CPU utilization: " << static_cast<int>(coresUsed * 100 / NUM_CORES) << "%" << endl;
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
                scheduler->readyQueue.push_back(console);  // Store the process console
                scheduler->allProcesses.push_back(console);
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
            cout << "CPU utilization: " << static_cast<int>(coresUsed * 100 / NUM_CORES) << "%" << endl;
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

    header();

    thread inputThread(handleInput);

    inputThread.join();

    return 0;
}
