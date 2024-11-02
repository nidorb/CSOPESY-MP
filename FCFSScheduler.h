#pragma once

#include <vector>
#include <thread>

#include "Process.h"
#include "Core.h"
#include "Scheduler.h"

class FCFSScheduler : public Scheduler {
public:
	const int numCores;
	vector<unique_ptr<Core>> cores;

	FCFSScheduler(int NUM_CORES) : numCores(NUM_CORES) {
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
	}

	void handleScheduler() {
		while (isRunning) {
			if (osRunning) {
				generateProcesses();
				cpuTicks++;
			}

			// Check if a core is available to assign a process
			for (int i = 0; i < numCores; i++) {
				if (!readyQueue.empty()) {
					if (cores[i]->isCoreFree()) {
						shared_ptr<Process> curProcess = readyQueue.front();
						readyQueue.pop();

						// Assign process to CPU core
						cores[i]->assignProcess(curProcess, "fcfs");
					}
				}	
			}

			this_thread::sleep_for(chrono::milliseconds(50));
		}
	}

	void handleProcessPreempt(const shared_ptr<Process>& process) {
		// FCFS cannot preempt
	}
};