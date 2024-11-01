#pragma once

#include <queue>
#include <memory>
#include <mutex>

#include "Process.h"
#include "Core.h"

class Scheduler {
public:
    static int BATCH_PROCESS_FREQ;

    queue<shared_ptr<Process>> readyQueue;  // Stores processes ready to be assigned to a CPU core
    queue<shared_ptr<Process>> allProcesses;  // Store all processes

    bool isRunning = true;

    int cpuTicks = 0;
    bool osRunning = false;

    virtual void handleScheduler() = 0;
    virtual void handleProcessPreempt(const shared_ptr<Process>& process) = 0;
    virtual ~Scheduler() {}

    void generateProcesses() {
        if (cpuTicks % Scheduler::BATCH_PROCESS_FREQ == 0) {
            auto process = make_shared<Process>("");
            readyQueue.push(process);
            allProcesses.push(process);
        }
    }
};