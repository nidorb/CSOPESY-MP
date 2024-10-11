#include <thread>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <condition_variable>
#include <mutex>

#include "Process.h"

using namespace std;

class Core {
public:
    int id;
    thread thread;
    condition_variable cv;
    mutex mtx;
    Process* currentProcess;

    Core(int id) : id(id), isFree(true), currentProcess(nullptr) {}

    void start() {
        thread = std::thread([this]() {
            while (true) {
                unique_lock<mutex> lock(mtx);
                cv.wait(lock, [this]() { return currentProcess != nullptr; });

                if (currentProcess != nullptr) {
                    executeProcess(currentProcess);
                    currentProcess = nullptr;
                }
            }
        });
    }

    void executeProcess(Process* process) {
        if (process == nullptr) return;

        isFree = false;
        process->cpuCoreID = id;
        auto now = chrono::system_clock::now();
        auto in_time_t = chrono::system_clock::to_time_t(now);
        tm buf;

        #ifdef _WIN32
            localtime_s(&buf, &in_time_t);
        #else
            localtime_r(&in_time_t, &buf);
        #endif

        ostringstream oss;
        oss << "(" << put_time(&buf, "%m/%d/%Y %I:%M:%S%p") << ") "
            << "Core:" << id << " \"Hello world from " << process->getProcessName() << "!\"";
        cout << oss.str() << endl;

        process->createProcFile();
        isFree = true;
    }

    void assignProcess(Process* process) {
        unique_lock<mutex> lock(mtx);
        currentProcess = process;
        cv.notify_one();
    }

    bool isCoreFree() {
        return isFree;
    }

private:
    bool isFree;
};
