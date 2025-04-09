#include <iostream>
#include "kaizen.h"
#include <vector>
#include <thread>

// Parse command-line arguments
std::pair<int, int> process_args(int argc, char* argv[]) {
    zen::cmd_args args(argv, argc);
    auto iter_options = args.get_options("--iterations");
    auto threadCount_options = args.get_options("--threads");

    if (threadCount_options.empty() || iter_options.empty()) {
        zen::print("Error: --iterations or --thteads arguments are absent, using defaults: iterations=100 and threads=3\n");
        return {1000000,3}; // Increased defaults
    }
    return {std::stoi(iter_options[0]), std::stoi(threadCount_options[0])};
}



int main(){
    
}
