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