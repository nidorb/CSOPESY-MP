#include <thread>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

#include "Process.h"

using namespace std;

class Core {
public:
    int id;
    std::thread thread;

    Core(int id) : id(id), isFree(true) {}

    void start() {
        thread = std::thread([this]() {
            // init
        });
    }

    void executeProcess(Process* process) {
        if (process == nullptr) return;
        isFree = false;
        process->cpuCoreID = id;
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm buf;
        // localtime_r(&in_time_t, &buf);
        localtime_s(&buf, &in_time_t); // for windows

        std::ostringstream oss;
        oss << "(" << std::put_time(&buf, "%m/%d/%Y %I:%M:%S%p") << ") "
            << "Core:" << id << " Hello world from " << process->getProcessName() << "!";
        process->executePrintCommands();
        isFree = true;
    }

    bool isCoreFree() {
        return isFree;
    }

private:
    bool isFree;
};
