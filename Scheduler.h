#pragma once

#include <memory>
#include <mutex>
#include <vector>

#include "Process.h"
#include "Core.h"

class Scheduler {
public:
    static uint64_t BATCH_PROCESS_FREQ;
    static uint64_t QUANTUM_CYCLES;

    vector<shared_ptr<Process>> readyQueue;  // Stores processes ready to be assigned to a CPU core
    vector<shared_ptr<Process>> allProcesses;  // Store all processes

    vector<unique_ptr<Core>> cores;

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

    bool areCoresFree() {
        for (const auto& core : cores) {
            if (!core->isCoreFree()) {
                return false;
            }
        }
        return true;
    }

    /*void handleMemoryFileGeneration() {
        int quantumCtr = 0;

        while (isRunning) {
            if (!areCoresFree()){
                if (quantumCtr % QUANTUM_CYCLES == 0) {
                    memoryAllocator->createMemoryFile(quantumCtr);
                }
                quantumCtr++;
            }
			
            this_thread::sleep_for(chrono::milliseconds(200)); 
        }
    }*/
};