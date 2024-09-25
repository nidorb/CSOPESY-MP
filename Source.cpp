#include <iostream>
#include <string>
#include <ctime>

using namespace std;

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

void createProcess(string name) {
    time_t now = time(0);
    struct tm tstruct;
    localtime_s(&tstruct, &now);

    char date_time[100];
    strftime(date_time, sizeof(date_time), "%m/%d/%Y, %I:%M:%S %p", &tstruct);
    
    // Process info
    string processName = name;
    string timestamp = date_time;
}

void processInfo(string name) {
    string processName = name;
    int curInst = 0;  // placeholder
    int totalInst = 50;  // placeholder

    char date_time[100] = "09/25/2024, 10:18:28 PM";

    cout << "Process: " << processName << endl;
    cout << "Instruction line: " << curInst << "/" << totalInst << endl;
    cout << "Timestamp: " << date_time << endl;
}

void drawConsole(string name) {
    clearScreen();

    processInfo(name);

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

int main() {
    string input;

    header();

    while (true) {
        cout << "Enter command: ";
        getline(cin, input);

        if (input == "exit") {
            return 0;
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
        else if (input.substr(0,10) == "screen -s " && input.length() > 10) {
            string processName = input.substr(10);

            createProcess(processName);
            drawConsole(processName);
        }

        // "screen -r <name>"
        else if (input.substr(0, 10) == "screen -r " && input.length() > 10) {
            string processName = input.substr(10);

            drawConsole(processName);
        }
        else {
            cout << "Unknown command \n";
        }
    }

    return 0;
}
