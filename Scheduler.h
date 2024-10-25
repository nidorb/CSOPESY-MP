#pragma once

#include <vector>
#include <memory>
#include <mutex>

#include "Process.h"
#include "Core.h"

class Scheduler {
public:
    vector<shared_ptr<Process>> readyQueue;  // Stores processes ready to be assigned to a CPU core

    vector<shared_ptr<Process>> allProcesses;  // Store all processes

    bool isRunning = true;

    virtual void handleScheduler() = 0;
    virtual void handleProcessPreempt(const std::shared_ptr<Process>& process) = 0;
    virtual ~Scheduler() {}
};