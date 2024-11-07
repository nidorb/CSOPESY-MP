#pragma once

#include <memory>
#include <mutex>

#include "Process.h"
#include "Core.h"

class Scheduler {
public:
    static uint64_t BATCH_PROCESS_FREQ;

    vector<shared_ptr<Process>> readyQueue;  // Stores processes ready to be assigned to a CPU core
    vector<shared_ptr<Process>> allProcesses;  // Store all processes

    bool isRunning = true;

    int cpuTicks = 0;
    bool isGenerating = false;

    virtual void handleScheduler() = 0;
    virtual void handleProcessPreempt(const shared_ptr<Process>& process) = 0;
    virtual ~Scheduler() {}

    void generateProcesses() {
        if (cpuTicks % Scheduler::BATCH_PROCESS_FREQ == 0) {
            auto process = make_shared<Process>("");
            readyQueue.push_back(process);
            allProcesses.push_back(process);
        }
    }

    void handleProcessGenerator() {
        while (isRunning) {
            if (isGenerating) {
                generateProcesses();
                cpuTicks++;
            }

            this_thread::sleep_for(chrono::milliseconds(500));
        }
    }
};