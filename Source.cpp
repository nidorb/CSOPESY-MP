#include <iostream>
#include <string>
#include <ctime>
#include <vector>
#include <thread>

#include "Process.h"
#include "CPU.h"

using namespace std;

const int NUM_CORES = 4;

int Process::next_pid = 0;

vector<Process*> activeProcesses;  // Stores names of active processes
vector<Process> readyProcesses;  // Stores processes ready to be assigned to a processor

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
    localtime_s(&tstruct, &now);
    // localtime_r(&now, &tstruct); // for Mac computers

    char date_time[100];
    strftime(date_time, sizeof(date_time), "%m/%d/%Y, %I:%M:%S %p", &tstruct);

    return date_time;
}

void processInfo(Process& console) {
    int curInst = 0;  // placeholder
    int totalInst = 50;  // placeholder

    cout << "PID: " << console.pid << endl;
    cout << "Process: " << console.name << endl;
    cout << "CPU Core: " << console.cpuCoreID << endl;
    cout << "Instruction line: " << curInst << "/" << totalInst << endl;
    cout << "Timestamp: " << console.timestamp << endl;
}

void drawConsole(Process console) {
    clearScreen();

    processInfo(console);

    string input;
    bool isRunning = true;

    while (isRunning) {
        cout << ">> ";
        getline(cin, input);

        if (input == "exit") {
            clearScreen();
            header();
            isRunning = false;
        }
        else {
            cout << "Unknown command \n";
        }
    }
}

Process* searchProcessByName(string name) {
    for (Process* console : activeProcesses) {
        if (console->name == name) {
            return console;
        }
    }

    return nullptr;
}


void handleInput() {
    string input;
    bool isRunning = true;

    while (isRunning) {
        cout << "Enter command: ";
        getline(cin, input);

        if (input == "exit") {
            isRunning = false;
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
            cout << "scheduler-test command recognized. Doing something.\n";
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
            Process* res_console = searchProcessByName(processName);
            if (res_console == nullptr) {
                Process* console = new Process(processName, getCurDate());  // Dynamic memory allocation
                drawConsole(*console);
                activeProcesses.push_back(console);  // Store the process console
            }
            else {
                cout << "Error: '" << processName << "' name already exists.\n";
            }
        }

        // "screen -r <name>"
        else if (input.substr(0, 10) == "screen -r " && input.length() > 10) {
            string processName = input.substr(10);

            // Check if process exists
            Process* res_console = searchProcessByName(processName);
            if (res_console != nullptr) {
                drawConsole(*res_console);  // Process exists, allow reopening
            }
            else {
                cout << "Error: '" << processName << "' does not exist. Use 'screen -s <name>'.\n";
            }
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
