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

	static uint64_t QUANTUM_CYCLES;

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

		thread generatorThread = thread(&RRScheduler::handleProcessGenerator, this);
		generatorThread.detach();
	}

	void handleScheduler() {
		while (isRunning) {

			// Check if a core is available to assign a process
			for (int i = 0; i < numCores; i++) {
				if (!readyQueue.empty()) {
					if (cores[i]->isCoreFree()) {
						shared_ptr<Process> curProcess = readyQueue.front();
						readyQueue.pop();
						
						// Assign process to CPU core
						cores[i]->assignProcess(curProcess, QUANTUM_CYCLES);
					}
				}
			}

			this_thread::sleep_for(chrono::milliseconds(10));
		}
	}

	void handleProcessPreempt(const shared_ptr<Process>& process) {
		lock_guard<mutex> lock(mtx);

		if (process->getState() == Process::READY) {
			readyQueue.push(process);
		}
	}
};