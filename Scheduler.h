#include <vector>
#include <thread>

#include "Process.h"
#include "Core.h"

class Scheduler {
public:
	const int numCores;
	vector<unique_ptr<Core>> cores;

	vector<shared_ptr<Process>> readyProcesses;  // Stores processes ready to be assigned to a CPU core
	vector<shared_ptr<Process>> runningProcesses;  // Stores processes that are currently running
	vector<shared_ptr<Process>> finishedProcesses;  // Stores processes that completed execution
	bool isRunning = true;

	std::mutex mtx;

	Scheduler(int NUM_CORES) : numCores(NUM_CORES) {
		for (int i = 0; i < numCores; i++) {
			auto core = make_unique<Core>(i);

			core->onProcessFinished = [this](const shared_ptr<Process>& process) {
				handleProcessCompletion(process);
				};

			core->start();
			cores.push_back(move(core));
		}

		thread schedulerThread = thread(&Scheduler::handleScheduler, this);
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
						cores[i]->assignProcess(curProcess);
					}
				}	
			}

			this_thread::sleep_for(chrono::milliseconds(100));
		}
	}

	void handleProcessCompletion(const shared_ptr<Process>& process) {
		lock_guard<mutex> lock(mtx);

		auto it = find(runningProcesses.begin(), runningProcesses.end(), process);
		if (it != runningProcesses.end()) {
			runningProcesses.erase(it);
		}
		finishedProcesses.push_back(process);
	}
};