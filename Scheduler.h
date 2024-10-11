#include <vector>
#include <thread>

#include "Process.h"
#include "Core.h"

class Scheduler {
public:
	const int numCores;
	vector<unique_ptr<Core>> cores;

	vector<Process*> readyProcesses;  // Stores processes ready to be assigned to a CPU core
	vector<Process*> runningProcesses;  // Stores processes that are currently running
	vector<Process*> finishedProcesses;  // Stores processes that completed execution


	Scheduler(int NUM_CORES) : numCores(NUM_CORES) {
		for (int i = 0; i < numCores; i++) {
			auto core = make_unique<Core>(i);
			core->start();
			cores.push_back(move(core));
		}

		thread schedulerThread = thread(&Scheduler::handleScheduler, this);
		schedulerThread.detach();
	}

	void handleScheduler() {
		while (true) {

			// Check if a core is available to assign a process
			for (int i = 0; i < numCores; i++) {
				if (!readyProcesses.empty()) {
					if (cores[i]->isCoreFree()) {
						Process* curProcess = readyProcesses.front();
						runningProcesses.push_back(curProcess);
						readyProcesses.erase(readyProcesses.begin());

						// Assign process to CPU core
						cores[i]->executeProcess(curProcess);

						//cout << readyProcesses.size() << endl;
						/*cout << cores[i]->id << endl;*/
					}
				}	
			}

			this_thread::sleep_for(chrono::milliseconds(10));
		}
	}
};