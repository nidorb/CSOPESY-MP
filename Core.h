#pragma once

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
    shared_ptr<Process> currentProcess;
    function<void(shared_ptr<Process>)> onProcessPreempt;

    bool isRunning = true;
    uint64_t quantum_cycles;

    Core(int id) : id(id), isFree(true), currentProcess(nullptr) {}

    void start() {
        coreThread = thread([this]() {
            while (isRunning) {
                unique_lock<mutex> lock(mtx);
                cv.wait(lock, [this]() { return currentProcess != nullptr || !isRunning; });

                if (currentProcess != nullptr) {
                    executeProcess(currentProcess, quantum_cycles);

                    if (onProcessPreempt) {
                        onProcessPreempt(currentProcess);
                    }

                    currentProcess = nullptr;

                    lock.unlock();
                    cv.notify_all();
                }
            }
            });
    }

    void executeProcess(const shared_ptr<Process>& process, uint64_t quantum_cycles) {
        if (process == nullptr) return;

        isFree = false;
        process->setCPUCoreID(id);

        process->createProcFile(quantum_cycles);
        isFree = true;
    }

    void assignProcess(const shared_ptr<Process>& process, uint64_t quantum_cycles = -1) {
        unique_lock<mutex> lock(mtx);
        currentProcess = process;
        
        if (quantum_cycles == -1) {
            this->quantum_cycles = process->getTotalWork();
        }
        else {
            this->quantum_cycles = quantum_cycles;
        }

        cv.notify_one();
    }

    bool isCoreFree() {
        return isFree;
    }

    ~Core() {
        isRunning = false;
        cv.notify_all();
        if (coreThread.joinable()) {
            coreThread.join();
        }
    }

private:
    bool isFree;
};
