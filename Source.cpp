#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <fstream>
#include <mutex>
#include <unordered_map>
#include <cstdint>

#include "Process.h"
#include "RRScheduler.h"
#include "FCFSScheduler.h"
#include "Scheduler.h"

#include "Globals.h"
#include "PagingAllocator.h"

using namespace std;

int NUM_CORES;
unique_ptr<Scheduler> scheduler;
unique_ptr<PagingAllocator> memoryAllocator;
string SCHEDULER_ALGO;  // fcfs or rr

int Process::next_pid = 0;
uint64_t Process::MIN_INS;
uint64_t Process::MAX_INS;
uint64_t Process::DELAYS_PER_EXEC;

size_t Process::memoryRequired;
size_t Process::MEM_PER_PAGE;

// Memory parameters
size_t MAX_OVERALL_MEM;
size_t MEM_PER_FRAME;
size_t MEM_PER_PROC;

size_t numPages;

uint64_t RRScheduler::QUANTUM_CYCLES;
uint64_t Scheduler::QUANTUM_CYCLES;
uint64_t Scheduler::BATCH_PROCESS_FREQ;

uint64_t QUANTUM_CYCLES;
uint64_t BATCH_PROCESS_FREQ;
uint64_t MIN_INS;
uint64_t MAX_INS;
uint64_t DELAYS_PER_EXEC;

const int MIN_NUM_CPU = 1;
const int MAX_NUM_CPU = 128;
const uint64_t MIN_RANGE = 1;
const uint64_t MIN_DELAY_PER_EXEC = 0;
const uint64_t MAX_RANGE = static_cast<uint64_t>(UINT32_MAX) + 1;

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
    for (const shared_ptr<Process>& console : scheduler->allProcesses) {
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

    bool invalidArg = false;
    ostringstream errorMessages;

    if (configMap.find("num-cpu") != configMap.end()) {
        NUM_CORES = stoi(configMap["num-cpu"]);
        if (NUM_CORES < MIN_NUM_CPU || NUM_CORES > MAX_NUM_CPU) {
            invalidArg = true;
            errorMessages << "num-cpu out of range. Must be between " << MIN_NUM_CPU << " and " << MAX_NUM_CPU << "." << endl;
        }
    }
    else {
        invalidArg = true;
        errorMessages << "num-cpu not found in config file." << endl;
    }

    if (configMap.find("scheduler") != configMap.end()) {
        SCHEDULER_ALGO = configMap["scheduler"];
        if (SCHEDULER_ALGO != "fcfs" && SCHEDULER_ALGO != "rr") {
            invalidArg = true;
            errorMessages << "Invalid scheduler. Must be \"fcfs\" or \"rr\"." << endl;
        }
    }
    else {
        invalidArg = true;
        errorMessages << "scheduler not found in config file." << endl;
    }

    if (configMap.find("quantum-cycles") != configMap.end()) {
        QUANTUM_CYCLES = stoull(configMap["quantum-cycles"]);
        if (QUANTUM_CYCLES < MIN_RANGE || QUANTUM_CYCLES > MAX_RANGE) {
            invalidArg = true;
            errorMessages << "quantum-cycles out of range. Must be between " << MIN_RANGE << " and " << MAX_RANGE << "." << endl;
        }
    }
    else {
        invalidArg = true;
        errorMessages << "quantum-cycles not found in config file." << endl;
    }

    if (configMap.find("batch-process-freq") != configMap.end()) {
        BATCH_PROCESS_FREQ = stoull(configMap["batch-process-freq"]);
        if (BATCH_PROCESS_FREQ < MIN_RANGE || BATCH_PROCESS_FREQ > MAX_RANGE) {
            invalidArg = true;
            errorMessages << "batch-process-freq out of range. Must be between " << MIN_RANGE << " and " << MAX_RANGE << "." << endl;
        }
    }
    else {
        invalidArg = true;
        errorMessages << "batch-process-freq not found in config file." << endl;
    }

    if (configMap.find("min-ins") != configMap.end()) {
        MIN_INS = stoull(configMap["min-ins"]);
        if (MIN_INS < MIN_RANGE || MIN_INS > MAX_RANGE) {
            invalidArg = true;
            errorMessages << "min-ins out of range. Must be between " << MIN_RANGE << " and " << MAX_RANGE << "." << endl;
        }
    }
    else {
        invalidArg = true;
        errorMessages << "min-ins not found in config file." << endl;
    }

    if (configMap.find("max-ins") != configMap.end()) {
        MAX_INS = stoull(configMap["max-ins"]);
        if (MAX_INS < MIN_RANGE || MAX_INS > MAX_RANGE) {
            invalidArg = true;
            errorMessages << "max-ins out of range. Must be between " << MIN_RANGE << " and " << MAX_RANGE << "." << endl;
        }
    }
    else {
        invalidArg = true;
        errorMessages << "max-ins not found in config file." << endl;
    }

	if (MAX_INS < MIN_INS) {
		invalidArg = true;
		errorMessages << "max-ins must be greater than or equal to min-ins." << endl;
	}

    if (configMap.find("delays-per-exec") != configMap.end()) {
        DELAYS_PER_EXEC = stoull(configMap["delays-per-exec"]);
        if (DELAYS_PER_EXEC < MIN_DELAY_PER_EXEC || DELAYS_PER_EXEC > MAX_RANGE) {
            invalidArg = true;
            errorMessages << "delays-per-exec out of range. Must be between " << MIN_DELAY_PER_EXEC << "and " << MAX_RANGE << "." << endl;
        }
    }
    else {
        invalidArg = true;
        errorMessages << "delays-per-exec not found in config file." << endl;
    }

    if (configMap.find("delays-per-exec") != configMap.end()) {
        MAX_OVERALL_MEM = static_cast<size_t>(stoull(configMap["max-overall-mem"]));
        if (MAX_OVERALL_MEM < MIN_RANGE || MAX_OVERALL_MEM > MAX_RANGE) {
            invalidArg = true;
            errorMessages << "delays-per-exec out of range. Must be between " << MIN_RANGE << "and " << MAX_RANGE << "." << endl;
        }
    }
    else {
        invalidArg = true;
        errorMessages << "max-overall-mem not found in config file." << endl;
    }

    if (configMap.find("mem-per-frame") != configMap.end()) {
        MEM_PER_FRAME = static_cast<size_t>(stoull(configMap["mem-per-frame"]));
        if (MEM_PER_FRAME < MIN_RANGE || MEM_PER_FRAME > MAX_RANGE) {
            invalidArg = true;
            errorMessages << "mem-per-frame out of range. Must be between " << MIN_RANGE << "and " << MAX_RANGE << "." << endl;
        }
    }
    else {
        invalidArg = true;
        errorMessages << "mem-per-frame not found in config file." << endl;
    }

    if (configMap.find("mem-per-proc") != configMap.end()) {
        MEM_PER_PROC = static_cast<size_t>(stoull(configMap["mem-per-proc"]));
        if (MEM_PER_PROC < MIN_RANGE || MEM_PER_PROC > MAX_RANGE) {
            invalidArg = true;
            errorMessages << "mem-per-proc out of range. Must be between " << MIN_RANGE << "and " << MAX_RANGE << "." << endl;
        }
    }
    else {
        invalidArg = true;
        errorMessages << "mem-per-proc not found in config file." << endl;
    }

    configFile.close();

    if (invalidArg) {
        cerr << "\nConfiguration errors found:\n" << errorMessages.str();
        return;
    }

    cout << "Configurations initialized:" << endl;
    cout << "NUM_CORES: " << NUM_CORES << endl;
    cout << "SCHEDULER_ALGO: " << SCHEDULER_ALGO << endl;
    cout << "QUANTUM_CYCLES: " << QUANTUM_CYCLES << endl;
    cout << "BATCH_PROCESS_FREQ: " << BATCH_PROCESS_FREQ << endl;
    cout << "MIN_INS: " << MIN_INS << endl;
    cout << "MAX_INS: " << MAX_INS << endl;
    cout << "DELAYS_PER_EXEC: " << DELAYS_PER_EXEC << endl;
    cout << "MAX_OVERALL_MEM: " << MAX_OVERALL_MEM << endl;
    cout << "MEM_PER_FRAME: " << MEM_PER_FRAME << endl;
    cout << "MEM_PER_PROC: " << MEM_PER_PROC << endl;

    Process::MIN_INS = MIN_INS;
    Process::MAX_INS = MAX_INS;
    Process::DELAYS_PER_EXEC = DELAYS_PER_EXEC;

    RRScheduler::QUANTUM_CYCLES = QUANTUM_CYCLES;
    Scheduler::QUANTUM_CYCLES = QUANTUM_CYCLES;
    Scheduler::BATCH_PROCESS_FREQ = BATCH_PROCESS_FREQ;

    Process::memoryRequired = MEM_PER_PROC;
    Process::MEM_PER_PAGE = MEM_PER_FRAME;  // Page size = Frame size

    isInitialized = true;

    if (memoryAllocator) memoryAllocator.reset();
    
    numPages = MAX_OVERALL_MEM / MEM_PER_FRAME;
    
    //memoryAllocator = make_unique<FlatMemoryAllocator>(MAX_OVERALL_MEM);
    memoryAllocator = make_unique<PagingAllocator>(MAX_OVERALL_MEM, numPages);

    if (SCHEDULER_ALGO == "fcfs") {
        if (scheduler) scheduler.reset();
        scheduler = make_unique<FCFSScheduler>(NUM_CORES);
    }
    else if (SCHEDULER_ALGO == "rr") {
        if (scheduler) scheduler.reset();
        scheduler = make_unique<RRScheduler>(NUM_CORES);
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

        /*else if (input == "visualize") {
            memoryAllocator->visualizeMemory();
        }*/

        else if (input == "initialize") {
            initialize("config.txt");
        }
        
        else if (input == "scheduler-test") {
            scheduler->isGenerating = true;
            cout << "Generating dummy processes.\n";
        }
        else if (input == "scheduler-stop") {
            scheduler->isGenerating = false;
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

            for (const shared_ptr<Process>& processPtr : scheduler->allProcesses) {
                if (processPtr->getState() == Process::RUNNING) {
                    coresUsed++;
                }
            }

            outFile.precision(2);
            outFile << "CPU utilization: " << static_cast<int>(coresUsed * 100 / NUM_CORES) << "%" << endl;
            outFile << "Cores used: " << static_cast<int>(coresUsed) << endl;
            outFile << "Cores available: " << static_cast<int>(NUM_CORES - coresUsed) << endl;
        
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
                cout << "\nProcess '" << processName << "' not found.\n";
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
            cout << "Cores used: " << static_cast<int>(coresUsed) << endl;
            cout << "Cores available: " << static_cast<int>(NUM_CORES - coresUsed) << endl;

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
