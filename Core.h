#include <thread>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <condition_variable>
#include <mutex>

#include "Process.h"
#include <functional>

using namespace std;

class Core {
public:
    int id;
    thread coreThread;
    condition_variable cv;
    mutex mtx;
    Process* currentProcess;
    std::function<void(Process*)> onProcessFinished;

    bool isRunning = true;

    Core(int id) : id(id), isFree(true), currentProcess(nullptr) {}

    void start() {
        coreThread = std::thread([this]() {
            while (isRunning) {
                unique_lock<mutex> lock(mtx);
                cv.wait(lock, [this]() { return currentProcess != nullptr || !isRunning; });

                if (currentProcess != nullptr) {
                    executeProcess(currentProcess);

                    if (onProcessFinished) {
                        onProcessFinished(currentProcess);
                    }

                    currentProcess = nullptr;

                    lock.unlock();
                    cv.notify_all();
                }
            }
            });
    }

    void executeProcess(Process* process) {
        if (process == nullptr) return;

        isFree = false;
        process->cpuCoreID = id;

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

    void stop() {
        isRunning = false;
        cv.notify_all();
        if (coreThread.joinable()) {
            coreThread.join();
        }
    }

    ~Core() {
        stop();
    }

private:
    bool isFree;
};
