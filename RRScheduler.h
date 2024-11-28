#pragma once

#include <thread>

#include "Process.h"
#include "Core.h"
#include "Scheduler.h"
#include "Globals.h"

class RRScheduler : public Scheduler {
public:
	const int numCores;

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

		/*thread memoryFileThread = thread(&RRScheduler::handleMemoryFileGeneration, this);
		memoryFileThread.detach();*/
	}

	void handleScheduler() {
		while (isRunning) {

			// Check if a core is available to assign a process
			for (int i = 0; i < numCores; i++) {
				if (!readyQueue.empty()) {
					if (cores[i]->isCoreFree() || cores[i]->nextProcess == nullptr) {
						shared_ptr<Process> curProcess = readyQueue.front();
						readyQueue.erase(readyQueue.begin());
						
						if (curProcess->getIsAllocated()) {
							cores[i]->assignProcess(curProcess, QUANTUM_CYCLES);
						}
						else {
							vector<size_t> frames = memoryAllocator->allocate(curProcess);

							// Memory not full, frames successfully allocated
							if (!frames.empty()) {
								curProcess->allocatedFrames = frames;

								// Assign process to CPU core
								cores[i]->assignProcess(curProcess, QUANTUM_CYCLES);
							}

							// Memory full
							else {
								// TODO: Replace line below with Backing Store operation.
								//		 Instead of returning the process to RQ, store the oldest process in BS then deallocate.
								//		 Then reallocate the new process if there's space.
								readyQueue.push_back(curProcess);
							}
						}
					}
				}
			}

			this_thread::sleep_for(chrono::milliseconds(10));
		}
	}

	void handleProcessPreempt(const shared_ptr<Process>& process) {
		lock_guard<mutex> lock(mtx);

		if (process->getState() == Process::READY) {
			readyQueue.push_back(process);
		}

		if (process->getState() == Process::FINISHED) {
			memoryAllocator->deallocate(process);
			process->allocatedFrames.clear();
		}
	}
};