#pragma once

#include <vector>
#include <thread>

#include "Process.h"
#include "Core.h"
#include "Scheduler.h"

class RRScheduler : public Scheduler {
public:
	const int numCores;
	vector<unique_ptr<Core>> cores;

	mutex mtx;

	int QUANTUM_CYCLES = 5;

	RRScheduler(int NUM_CORES) : numCores(NUM_CORES) {
		for (int i = 0; i < numCores; i++) {
			auto core = make_unique<Core>(i);

			core->onProcessPreempt = [this](const shared_ptr<Process>& process) {
				handleProcessPreempt(process);
				};

			core->start();
			cores.push_back(move(core));
		}

		thread schedulerThread = thread(&RRScheduler::handleScheduler, this);
		schedulerThread.detach();
	}

	void handleScheduler() {
		while (isRunning) {
			// Check if a core is available to assign a process
			for (int i = 0; i < numCores; i++) {
				if (!readyProcesses.empty()) {
					if (cores[i]->isCoreFree()) {
						shared_ptr<Process> curProcess = readyProcesses.front();
						runningProcesses.push_back(curProcess);
						readyProcesses.erase(readyProcesses.begin());
						
						// Assign process to CPU core
						cores[i]->assignProcess(curProcess, QUANTUM_CYCLES);
					}
				}
			}

			this_thread::sleep_for(chrono::milliseconds(100));
		}
	}

	void handleProcessPreempt(const shared_ptr<Process>& process) {
		lock_guard<mutex> lock(mtx);

		auto it = find(runningProcesses.begin(), runningProcesses.end(), process);
		if (it != runningProcesses.end()) {
			runningProcesses.erase(it);
		}

		if (process->getState() == Process::READY) {
			readyProcesses.push_back(process);
		}
		else if (process->getState() == Process::FINISHED) {
			finishedProcesses.push_back(process);
		}
	}
};