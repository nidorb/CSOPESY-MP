#include <iostream>
#include <string>
#include <ctime>
#include <unordered_set>

using namespace std;

class Console {
public:
    string name, timestamp;

    // Constructor
    Console(string name, string timestamp) : name(name), timestamp(timestamp) {}

    // Equality operator
    bool operator==(const Console& other) const {
        return name == other.name && timestamp == other.timestamp;
    }
};

// Hash function
struct ConsoleHash {
    size_t operator()(const Console& c) const {
        return hash<string>()(c.name) ^ hash<string>()(c.timestamp);
    }
};

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

    char date_time[100];
    strftime(date_time, sizeof(date_time), "%m/%d/%Y, %I:%M:%S %p", &tstruct);

    return date_time;
}

void processInfo(Console console) {
    int curInst = 0;  // placeholder
    int totalInst = 50;  // placeholder

    cout << "Process: " << console.name << endl;
    cout << "Instruction line: " << curInst << "/" << totalInst << endl;
    cout << "Timestamp: " << console.timestamp << endl;
}

void drawConsole(Console console) {
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

int main() {
    unordered_set<Console, ConsoleHash> activeProcesses;  // Stores names of active processes
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
        else if (input.substr(0, 10) == "screen -s " && input.length() > 10) {
            string processName = input.substr(10);

            Console console(processName, getCurDate());
            drawConsole(console);
            activeProcesses.insert(console);  // Store the process console
        }

        // "screen -r <name>"
        else if (input.substr(0, 10) == "screen -r " && input.length() > 10) {
            string processName = input.substr(10);

            // Check if process exists
            bool isExist = false;
            for (Console console : activeProcesses) {
                if (console.name == processName) {
                    isExist = true;
                    drawConsole(console);  // Process exists, allow reopening
                    break;
                }
            }

            // Process does not exist
            if (isExist == false) {
                cout << "Error: '" << processName << "' does not exist. Use 'screen -s <name>'.\n";
            }
        }
        else {
            cout << "Unknown command \n";
        }
    }

    return 0;
}
