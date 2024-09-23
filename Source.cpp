#include <iostream>
#include <string>

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
        else {
            cout << "Unknown command \n";
        }
    }

    return 0;
}
