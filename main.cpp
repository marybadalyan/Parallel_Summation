#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <algorithm>
#include <numeric>
#include <mutex>
#include <atomic>
#include <chrono>
#include "kaizen.h"
#include <format>
// #include "kaizen.h" // Assuming this provides zen::cmd_args; comment out for now

void fill_with_random(std::vector<int>& vec) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1000);
    for (auto& val : vec) {
        val = dis(gen);
    }
}

std::pair<int, int> process_args(int argc, char* argv[]) {
    zen::cmd_args args(argv, argc);
    auto size_options = args.get_options("--size");
    auto threadCount_options = args.get_options("--threads");

    if (threadCount_options.empty() || size_options.empty()) {
        std::cerr << "Error: --size or --threads arguments absent, using defaults: size=1000000000, threads=5\n";
        return {1000000000, 5};  
    }
    return {std::stoi(size_options[0]), std::stoi(threadCount_options[0])};
}

void sum_by_threads_non_atomic(std::vector<int>& sums, const std::vector<int>& arr, size_t thread_id, size_t begin, size_t end) {
    int sum = 0;
    for (size_t i = begin; i < end && i < arr.size(); ++i) {
        sum += arr[i];
    }
    sums[thread_id] = sum;
}

void sum_by_threads_atomic(std::atomic<int>& sums, const std::vector<int>& arr, size_t begin, size_t end) {
    int sum = 0;
    for (size_t i = begin; i < end && i < arr.size(); ++i) {
        sum += arr[i];
    }
    sums.fetch_add(sum, std::memory_order_relaxed);
}

std::mutex mtx;
void sum_by_threads_mutex(int& sum, const std::vector<int>& arr, size_t start, size_t end) {
    int partial_sum = 0;
    for (size_t i = start; i < end; ++i) {
        partial_sum += arr[i];
    }
    std::lock_guard<std::mutex> lock(mtx);
    sum += partial_sum;
}
