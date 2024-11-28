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

#include "Globals.h"

using namespace std;

class Core {
public:
    int id;
    thread coreThread;
    condition_variable cv;
    mutex mtx;
    shared_ptr<Process> currentProcess;
    shared_ptr<Process> nextProcess;
    function<void(shared_ptr<Process>)> onProcessPreempt;

    bool isRunning = true;
    uint64_t quantum_cycles;

    Core(int id) : id(id), isFree(true), currentProcess(nullptr) {}

    void start() {
        coreThread = thread([this]() {
            while (isRunning) {

                if (currentProcess != nullptr) {
                    executeProcess(currentProcess, quantum_cycles);

                    if (onProcessPreempt) {
                        onProcessPreempt(currentProcess);
                    }

                    currentProcess = nextProcess;
                    nextProcess = nullptr;

                }
                else {
                    memoryAllocator->idleTicks++;

                    this_thread::sleep_for(chrono::milliseconds(100));
                }
            }
            });
    }

    void executeProcess(shared_ptr<Process>& process, uint64_t quantum_cycles) {
        if (process == nullptr) return;

        process->setCPUCoreID(id);
        createProcFile(process, quantum_cycles);
    }

    void assignProcess(const shared_ptr<Process>& process, uint64_t quantum_cycles = -1) {
        unique_lock<mutex> lock(mtx);

        if (currentProcess == nullptr) {
            currentProcess = process;
        }
        else {
            nextProcess = process;
        }
        
        this->quantum_cycles = quantum_cycles;

        cv.notify_one();
    }

    bool isCoreFree() {
        return currentProcess == nullptr;
    }

    ~Core() {
        isRunning = false;
        cv.notify_all();
        if (coreThread.joinable()) {
            coreThread.join();
        }
    }

    void createProcFile(shared_ptr<Process>& process, uint64_t quantum_cycles) {
        process->setState(Process::RUNNING);

        ofstream logs;
        string filename = process->getName() + ".txt";

        if (process->commandCtr == 0) {
            logs.open(filename);

            logs << "Process name: " << process->getName() << "\n";
            logs << "Logs: \n\n";
        }
        else {
            logs.open(filename, ios::app);
        }

        process->quantumCtr = 0;
        process->delayCtr = process->DELAYS_PER_EXEC;
        while (process->quantumCtr < quantum_cycles) {
            if (process->commandCtr == process->getTotalWork()) { break; }

            if (process->delayCtr > 0) {
                process->delayCtr--;
            }
            else {
                logs << process->getCurDateProc() << "   Core: " << process->getCPUCoreID() << "  \"Hello world from " << process->getName() << "!\"\n";
                process->commandCtr++;

                process->delayCtr = process->DELAYS_PER_EXEC;
                process->quantumCtr++;
            }

            memoryAllocator->activeTicks++;

            this_thread::sleep_for(chrono::milliseconds(100));
        }

        logs.close();

        process->setPreemptState();
    }

private:
    bool isFree;
};
