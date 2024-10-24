#include <iostream>
#include <string>
#include <ctime>
#include <vector>
#include <thread>
#include <fstream>
#include "Process.h"
#include "Scheduler.h"

using namespace std;

const int NUM_CORES = 4;
Scheduler scheduler(NUM_CORES);

int Process::next_pid = 0;

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

    cout << "PID: " << console->pid << endl;
    cout << "Process: " << console->name << endl;
    cout << "CPU Core: " << console->cpuCoreID << endl;
    cout << "Instruction line: " << curInst << "/" << totalInst << endl;
    cout << "Timestamp: " << console->timestamp << endl;
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
            if (console->name == name) {
                return console;
            }
        }
    }
    return nullptr;
}


void handleInput() {
    string input;

    while (true) {
        cout << "Enter command: ";
        getline(cin, input);

        if (input == "exit") {
            scheduler.isRunning = false;
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
            for (int i = 0; i < 10; i++) {
                scheduler.readyProcesses.emplace_back(new Process("Process_" + to_string(i), getCurDate()));
            }
        }
        else if (input == "scheduler-stop") {
            cout << "scheduler-stop command recognized. Doing something.\n";
        }
        else if (input == "report-util") {
            cout << "report-util command recognized. Doing something.\n";
        }

        // "screen -s <name>"
        else if (input.substr(0, 10) == "screen -s " && input.length() > 10) {
            string processName = input.substr(10);

            // Check if process exists
            vector<vector<shared_ptr<Process>>> processes = { scheduler.readyProcesses, scheduler.runningProcesses, scheduler.finishedProcesses };
            shared_ptr<Process> res_console = searchProcessByName(processName, processes);
            if (res_console == nullptr) {
                shared_ptr<Process> console = make_shared<Process>(processName, getCurDate());
                drawConsole(console);
                scheduler.readyProcesses.push_back(console);  // Store the process console
            }
            else {
                cout << "Error: '" << processName << "' name already exists.\n";
            }
        }

        // "screen -r <name>"
        else if (input.substr(0, 10) == "screen -r " && input.length() > 10) {
            string processName = input.substr(10);

            // Check if process exists
            vector<vector<shared_ptr<Process>>> processes = { scheduler.readyProcesses, scheduler.runningProcesses, scheduler.finishedProcesses };

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
            for (const shared_ptr<Process>& processPtr : scheduler.runningProcesses) {
                cout << processPtr->name << "   (" << processPtr->timestamp << ")    Core:  "
                    << processPtr->cpuCoreID << "    " << processPtr->getProgressString() << "\n";
            }

            // Print finished processes
            cout << "\nFinished processes:\n";
            for (const shared_ptr<Process>& processPtr : scheduler.finishedProcesses) {
                cout << processPtr->name << "   (" << processPtr->timestamp << ")    Finished    "
                    << processPtr->totalWork << " / " << processPtr->totalWork << "\n";
            }

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
