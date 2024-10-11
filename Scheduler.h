//#include "CPU.h"

class Scheduler {
public:
	vector<Process*> activeProcesses;  // Stores names of active processes
	vector<Process> readyProcesses;  // Stores processes ready to be assigned to a processor
	const int numCores;

	Scheduler(int NUM_CORES) : numCores(NUM_CORES) {}


	void handleScheduler() {
		//CPU coreWorkers(numCores);

		while (true) {

		}
	}
};