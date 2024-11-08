#pragma once

class FlatMemoryAllocator {
public:
	FlatMemoryAllocator(size_t maximumSize) : maximumSize(maximumSize), allocatedSize(0) {
		memory.resize(maximumSize, nullptr);
	}

	// Find the first available block that can accomodate the process
	// Returns address of location in memory vector where process is "allocated"
	void* allocate(shared_ptr<Process> process) {
		size_t size = process->memoryRequired;

		for (size_t i = 0; i <= maximumSize - size; i++) {
			if (!memory[i] && canAllocateAt(i, size)) {
				allocateAt(i, process);
				return &memory[i];
			}
		}

		return nullptr;
	}

	// Find the index of the memory block to deallocate
	void deallocate(shared_ptr<Process> process) {
		size_t index = -1;

		for (size_t i = 0; i < memory.size(); i++) {
			if (memory[i] == process) {
				index = i;
				break;
			}
		}

		if (index != -1) {
			deallocateAt(index, process->memoryRequired);
		}
	}

	void visualizeMemory() {
		cout << "----- end ----- = " << memory.size() - 1 << "\n\n";

		int index = memory.size() - 1;
		shared_ptr<Process> curProcess = nullptr;

		while (index >= 0) {
			if (memory[index] != nullptr && memory[index] != curProcess) {
				if (curProcess != nullptr) {
					cout << index + 1 << "\n\n";
				}

				cout << index << "\n";
				cout << memory[index]->getProcessName() << "\n";

				curProcess = memory[index];
			}

			index--;
		}
		
		if (curProcess != nullptr) {
			cout << index + 1 << "\n";
		}

		cout << "\n----- start ----- = " << 0 << "\n";
	}

	int getExternalFragmentation() {
        int totalFreeSpace = 0;

        for (const auto& block : memory) {
            if (block == nullptr) {
                totalFreeSpace++;
			}
        }

        return totalFreeSpace;
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
        strftime(date_time, sizeof(date_time), "(%m/%d/%Y, %I:%M:%S %p)", &tstruct);

        return date_time;
    }

	int getNumProcess() const {
		int index = memory.size() - 1;
		int numProcess = 0;
		shared_ptr<Process> curProcess = nullptr;

		while (index >= 0) {
			if (memory[index] != nullptr && memory[index] != curProcess) {
				numProcess++;
				curProcess = memory[index];
			}

			index--;
		}
        return numProcess;
    }

	 void createMemoryFile(int quantumCycle) {
		std::ostringstream filePath;    
		filePath << "memory_stamp_" << quantumCycle << ".txt";

        ofstream outFile(filePath.str());

		if (!outFile) {
			cerr << "Error opening file for writing." << endl;
			return;
		}

		int frag = getExternalFragmentation();
		string timestamp = getCurDate();
		int numProcess = getNumProcess();

		outFile << "Timestamp: " << timestamp << "\n";
		outFile << "Number of processes in memory: " << numProcess << "\n";
		outFile << "Total external fragmentation in KB: " << frag << "\n\n";


		outFile << "----- end ----- = " << memory.size() - 1 << "\n\n";

		int index = memory.size() - 1;
		shared_ptr<Process> curProcess = nullptr;

		while (index >= 0) {
			if (memory[index] != nullptr && memory[index] != curProcess) {
				if (curProcess != nullptr) {
					outFile << index + 1 << "\n\n";
				}

				outFile << index << "\n";
				outFile << memory[index]->getProcessName() << "\n";

				curProcess = memory[index];
			}

			index--;
		}
		
		if (curProcess != nullptr) {
			outFile << index + 1 << "\n";
		}

		outFile << "\n----- start ----- = " << 0 << "\n";

		outFile.close();
	}


	
private:
	size_t maximumSize;
	size_t allocatedSize;
	vector<shared_ptr<Process>> memory;

	// Check if the memory block is large enough
	bool canAllocateAt(size_t index, size_t size) const {
		return (index + size <= maximumSize);
	}

	// Mark the memory block as allocated
	void allocateAt(size_t index, shared_ptr<Process> process) {
		size_t size = process->memoryRequired;

		fill(memory.begin() + index, memory.begin() + index + size, process);
		allocatedSize += size;
	}

	// Mark the memory block as deallocated
	void deallocateAt(size_t index, size_t size) {
		fill(memory.begin() + index, memory.begin() + index + size, nullptr);
		allocatedSize -= size;
	}
};