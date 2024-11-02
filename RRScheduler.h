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
	bool rrRun = false;

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

	int runningProcesses() {
		int freeCoreCtr = 0;
		for (int j = 0; j < numCores; j++) {
			if (cores[j]->isCoreFree()) {
				freeCoreCtr++;
			}
		}
		return numCores - freeCoreCtr;
	}


	void handleScheduler() {
		while (isRunning) {
			if (osRunning) {
				generateProcesses();
				cpuTicks++;
			}

			// RR scheduler has 2 possibilities: RR or FCFS (if processes are less than cores)
			//rrRun represents tracker if FCFS should run

			// runningProcesses() -> displays # of running processes
			// readyQueue.size() -> displays # of ready processes
			

			// if no RUNNING/READY processes, set rrRun to false
			if ((runningProcesses() + readyQueue.size()) == 0) {
				rrRun = false;
			}

			//it should recheck if fcfs every end of quantum cycle              // checks if processes are less than cores                    // checs if it should be fcfs or not
			else if (cpuTicks % QUANTUM_CYCLES == 0                 &&         (runningProcesses() + readyQueue.size()) <= numCores        &&      !rrRun) {
				for (int i = 0; i < numCores; i++) {
					if (!readyQueue.empty()) {
						if (cores[i]->isCoreFree()) {
							shared_ptr<Process> curProcess = readyQueue.front();
							readyQueue.pop();

							cores[i]->assignProcess(curProcess);
						}
					}
				}
				// prevents running loop twice (causing reassignment of cores)
				rrRun = true;	
			}


			// RR implementation
			else {
				for (int i = 0; i < numCores; i++) {
					if (!readyQueue.empty()) {
						if (cores[i]->isCoreFree()) {
							shared_ptr<Process> curProcess = readyQueue.front();
							readyQueue.pop();

							cores[i]->assignProcess(curProcess, QUANTUM_CYCLES);
						}
					}
				}
				rrRun = false;
			}
			this_thread::sleep_for(chrono::milliseconds(50));
		}
	}

	void handleProcessPreempt(const shared_ptr<Process>& process) {
		lock_guard<mutex> lock(mtx);

		if (process->getState() == Process::READY) {
			readyQueue.push(process);
		}
	}
};