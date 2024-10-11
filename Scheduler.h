
class Scheduler {
public:
	vector<Process*> readyProcesses;  // Stores processes ready to be assigned to a CPU worker
	const int numCores;

	Scheduler(int NUM_CORES) : numCores(NUM_CORES) {}


	void handleScheduler() {
		while (true) {

		}
	}
};