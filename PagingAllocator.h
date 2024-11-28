#pragma once

class PagingAllocator {
public:
	PagingAllocator(size_t maxMemorySize, size_t numFrames) : maxMemorySize(maxMemorySize), numFrames(numFrames) {
		for (size_t i = 0; i < numFrames; i++) {

			// Initialize the free frame list
			freeFrameList.push_back(i);
		}
	}

	vector<size_t> allocate(shared_ptr<Process> process) {
		size_t processId = process->getPid();
		size_t numFramesNeeded = process->numFrames;

		if (numFramesNeeded > freeFrameList.size()) {

			// Not enough free frames
			return {};
		}
		else {
			vector<size_t> allocatedFrames = allocateFrames(numFramesNeeded, processId);
			process->setIsAllocated(true);

			return allocatedFrames;
		}
	}

	void deallocate(shared_ptr<Process> process) {
		size_t processId = process->getPid();

		// Find frames allocated to the process and deallocate
		auto it = find_if(frameMap.begin(), frameMap.end(),
			[processId](const auto& entry) { return entry.second == processId; });

		while (it != frameMap.end()) {
			size_t frameIndex = it->first;
			deallocateFrame(frameIndex);
			it = find_if(frameMap.begin(), frameMap.end(),
				[processId](const auto& entry) { return entry.second == processId; });
		}

		process->setIsAllocated(false);
	}

	vector<size_t> allocateFrames(size_t numFrames, size_t processId) {
		// Track the occupied frames by this process
		vector<size_t> allocatedFrames;

		// Map allocated frames to the process ID
		for (size_t i = 0; i < numFrames; i++) {
			size_t frameIndex = freeFrameList.front();
			freeFrameList.erase(freeFrameList.begin());

			frameMap[frameIndex] = processId;
			allocatedFrames.push_back(frameIndex);
		}

		return allocatedFrames;
	}

	void deallocateFrame(size_t frameIndex) {
		// Remove mapping of deallocated frame
		frameMap.erase(frameIndex);

		// Add frame back to the free frame list
		freeFrameList.push_back(frameIndex);
	}

	void visualizeMemory() const {
		cout << "Memory Visualization:\n";
		for (size_t frameIndex = 0; frameIndex < numFrames; frameIndex++) {
			auto it = frameMap.find(frameIndex);
			if (it != frameMap.end()) {
				cout << "Frame " << frameIndex << " -> Process " << it->second << "\n";
			}
			else {
				cout << "Frame " << frameIndex << " -> Free\n";
			}
		}
		cout << "--------------------\n";
	}

private:
	size_t maxMemorySize;
	size_t numFrames;
	unordered_map<size_t, size_t> frameMap;
	vector<size_t> freeFrameList;
};