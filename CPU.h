#include <iostream>
#include <thread>
#include <vector>
#include "Scheduler.h"


class CPU {
public:
    int numCores;
    std::vector<std::thread> cores;
    Scheduler scheduler;

    CPU(int numCores) : numCores(numCores) {
        for (int i = 0; i < numCores; ++i) {
            cores.push_back(std::thread(&CPU::coreFunction, this, i));
        }
    }

    ~CPU() {
        scheduler.stopScheduler();
        for (auto& core : cores) {
            if (core.joinable()) {
                core.join();
            }
        }
    }

    void addProcess(Process* process) {
        scheduler.addProcess(process);
    }

    void coreFunction(int coreID) {
        while (true) {
            Process* process = scheduler.getNextProcess();
            if (process == nullptr) return;
            process->cpuCoreID = coreID;
            // TODO: implement function
            process->executePrintCommands();
        }
    }
};



