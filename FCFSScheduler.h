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

	mutex mtx;

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
						readyQueue.erase(readyQueue.begin());

						// Assign process to CPU core
						cores[i]->assignProcess(curProcess);
					}
				}	
			}

			this_thread::sleep_for(chrono::milliseconds(100));
		}
	}

	string getCurDate() {
		time_t now = time(0);
		struct tm tstruct;
		#ifdef _WIN32
				localtime_s(&tstruct, &now);
		#else
				localtime_r(&now, &tstruct);
		#endif

		char date_time[100];
		strftime(date_time, sizeof(date_time), "%m/%d/%Y, %I:%M:%S %p", &tstruct);

		return date_time;
	}

	void generateProcesses() {
		if (cpuTicks % Scheduler::BATCH_PROCESS_FREQ == 0) {
			//string processName = "Process_" + to_string(processCtr);
			auto process = make_shared<Process>("", getCurDate());
			readyQueue.push_back(process);
			allProcesses.push_back(process);
			//cout << "\nGenerated process: " << processName << "\n";
			//cout << "Process Ctr: " << processCtr << " pc: " << process->getPid() << endl;
		}
	}

	void handleProcessPreempt(const shared_ptr<Process>& process) {
		// FCFS cannot preempt
	}
};