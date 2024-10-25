#include <iostream>
#include <string>
#include <ctime>
#include <vector>
#include <thread>
#include <fstream>
#include <mutex>

#include "Process.h"
#include "RRScheduler.h"
#include "FCFSScheduler.h"
#include "Scheduler.h"

using namespace std;

const int NUM_CORES = 4;
unique_ptr<Scheduler> scheduler;

string SCHEDULER_ALGO = "rr";  // fcfs or rr

int Process::next_pid = 0;

const int QUANTUM_CYCLES = 5;
const int BATCH_PROCESS_FREQ = 1;
const int MIN_INS = 1000;
const int MAX_INS = 2000;
const int DELAYS_PER_EXEC = 0;

bool osRunning = false;
int cpuCycles = 2;
int processCtr = 0;

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
    system("cls");
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

shared_ptr<Process> searchProcessByName(const string& name, vector<vector<shared_ptr<Process>>>& processes) {
    for (auto& vector : processes) {
        for (const shared_ptr<Process>& console : vector) {
            if (console->getName() == name) {
                return console;
            }
        }
    }
    return nullptr;
}

void generateProcesses() {
    for (int i = 0; i < BATCH_PROCESS_FREQ; ++i) {
        string processName = "Process_" + to_string(processCtr);
        auto process = make_shared<Process>(processName, getCurDate());
        scheduler->readyProcesses.push_back(process);
        cout << "\nGenerated process: " << processName << "\n";
        cout << "Process Ctr: " << processCtr << " pc: " << process->getPid() << endl;
        processCtr++;
    }
}

void handleProcessGeneration() {
	while (osRunning) {
		generateProcesses();
		this_thread::sleep_for(chrono::seconds(cpuCycles));
	}
}


void handleInput() {
    string input;

    while (true) {
        cout << "Enter command: ";
        getline(cin, input);

        if (input == "exit") {
            scheduler->isRunning = false;
            return;
        }
        else if (input == "clear") {
            clearScreen();
            header();
        }
        else if (input == "initialize") {
            cout << "initialize command recognized. Doing something.\n";
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
            cout << "report-util command recognized. Doing something.\n";
        }

        // "screen -s <name>"
        else if (input.substr(0, 10) == "screen -s " && input.length() > 10) {
            string processName = input.substr(10);

            // Check if process exists
            vector<vector<shared_ptr<Process>>> processes = { scheduler->readyProcesses, scheduler->runningProcesses, scheduler->finishedProcesses };
            shared_ptr<Process> res_console = searchProcessByName(processName, processes);
            if (res_console == nullptr) {
                shared_ptr<Process> console = make_shared<Process>(processName, getCurDate());
                drawConsole(console);
                scheduler->readyProcesses.push_back(console);  // Store the process console
            }
            else {
                cout << "Error: '" << processName << "' name already exists.\n";
            }
        }

        // "screen -r <name>"
        else if (input.substr(0, 10) == "screen -r " && input.length() > 10) {
            string processName = input.substr(10);

            // Check if process exists
            vector<vector<shared_ptr<Process>>> processes = { scheduler->readyProcesses, scheduler->runningProcesses, scheduler->finishedProcesses };

            shared_ptr<Process> res_console = searchProcessByName(processName, processes);
            if (res_console != nullptr) {
                drawConsole(res_console);  // Process exists, allow reopening
            }
            else {
                cout << "Error: '" << processName << "' does not exist. Use 'screen -s <name>'.\n";
            }
        }

        // "screen -ls"
        else if (input.substr(0, 10) == "screen -ls") {
            cout << "\n\nRunning processes:\n";
            for (const shared_ptr<Process>& processPtr : scheduler->runningProcesses) {
                if (processPtr) {
                    cout << processPtr->getName() << "   (" << processPtr->getTimestamp() << ")    Core:  "
                        << processPtr->getCPUCoreID() << "    " << processPtr->getProgressString() << "\n";
                }
            }

            // Print finished processes
            cout << "\nFinished processes:\n";
            for (const shared_ptr<Process>& processPtr : scheduler->finishedProcesses) {
                if (processPtr) {
                    cout << processPtr->getName() << "   (" << processPtr->getTimestamp() << ")    Finished    "
                        << processPtr->getProgressString() << "\n";
                }
            }

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
