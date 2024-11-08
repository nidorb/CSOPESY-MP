#pragma once

#include <thread>

#include "Process.h"
#include "Core.h"
#include "Scheduler.h"
#include "FlatMemoryAllocator.h"

class FCFSScheduler : public Scheduler {
public:
	const int numCores;

	FCFSScheduler(int NUM_CORES) : numCores(NUM_CORES){
		for (int i = 0; i < numCores; i++) {
			auto core = make_unique<Core>(i);

			core->onProcessPreempt = [this](const shared_ptr<Process>& process) {
				handleProcessPreempt(process);
				};

			core->start();
			cores.push_back(move(core));
		}

		thread schedulerThread = thread(&FCFSScheduler::handleScheduler, this);
		schedulerThread.detach();

		thread generatorThread = thread(&FCFSScheduler::handleProcessGenerator, this);
		generatorThread.detach();

		thread memoryFileThread = thread(&FCFSScheduler::handleMemoryFileGeneration, this);
		memoryFileThread.detach();
	}

	void handleScheduler() {
		while (isRunning) {

			// Check if a core is available to assign a process
			for (int i = 0; i < numCores; i++) {
				if (!readyQueue.empty()) {
					if (cores[i]->isCoreFree()) {
						shared_ptr<Process> curProcess = readyQueue.front();

						void* memory = memoryAllocator->allocate(curProcess);
						if (memory != nullptr) {
							//cout << "Allocated memory for process " << curProcess->getProcessName() << endl;
							
							// Assign process to CPU core
							readyQueue.erase(readyQueue.begin());
							cores[i]->assignProcess(curProcess);
						}
						else {
							cout << "Memory allocation failed for process " << curProcess->getProcessName() << endl;
							this_thread::sleep_for(chrono::milliseconds(500));
						}
					}
				}	
			}

			this_thread::sleep_for(chrono::milliseconds(10));
		}
	}

	void handleProcessPreempt(const shared_ptr<Process>& process) {
		// FCFS cannot preempt
	}
};