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


int main(int argc, char* argv[]) {
    auto [size, thread_count] = process_args(argc, argv);

    std::vector<int> arr(size);
    fill_with_random(arr);
    int expected_sum = std::accumulate(arr.begin(), arr.end(), 0);

    std::vector<std::thread> threads;
    size_t chunk = arr.size() / thread_count;

    // Non-Atomic (Reduce-Like)
    std::vector<int> thread_sums(thread_count, 0);
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < thread_count; ++i) {
        size_t begin = i * chunk;
        size_t end = (i == thread_count - 1) ? arr.size() : begin + chunk;
        threads.emplace_back(sum_by_threads_non_atomic, std::ref(thread_sums), std::ref(arr), i, begin, end);
    }
    for (auto& t : threads) t.join();
    int non_atomic_sum = std::accumulate(thread_sums.begin(), thread_sums.end(), 0);
    auto end = std::chrono::high_resolution_clock::now();
    auto non_atomic_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Atomic
    std::atomic<int> atomic_sum = 0;
    threads.clear();
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < thread_count; ++i) {
        size_t begin = i * chunk;
        size_t end = (i == thread_count - 1) ? arr.size() : begin + chunk;
        threads.emplace_back(sum_by_threads_atomic, std::ref(atomic_sum), std::ref(arr), begin, end);
    }
    for (auto& t : threads) t.join();
    end = std::chrono::high_resolution_clock::now();
    auto atomic_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Mutex
    int mutex_sum = 0;
    threads.clear();
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < thread_count; ++i) {
        size_t begin = i * chunk;
        size_t end = (i == thread_count - 1) ? arr.size() : begin + chunk;
        threads.emplace_back(sum_by_threads_mutex, std::ref(mutex_sum), std::ref(arr), begin, end);
    }
    for (auto& t : threads) t.join();
    end = std::chrono::high_resolution_clock::now();
    auto mutex_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Pretty Output without + signs
    std::cout << std::format("{:-<29}{:-<14}\n", "", "");
    std::cout << std::format("| {:<24} | {:>12} |\n", "Description", "Value");
    std::cout << std::format("{:-<29}{:-<14}\n", "", "");
    std::cout << std::format("| {:<24} | {:>12} |\n", "Expected Sum", expected_sum);
    std::cout << std::format("{:-<29}{:-<14}\n", "", "");

    std::cout << std::format("| {:<24} | {:>12} |\n", "Non-Atomic (Reduce-Like)", "");
    std::cout << std::format("| {:<24} | {:>12} |\n", "  Total Sum", non_atomic_sum);
    std::cout << std::format("| {:<24} | {:>12} |\n", "  Time (ms)", non_atomic_time);
    std::cout << std::format("{:-<29}{:-<14}\n", "", "");

    std::cout << std::format("| {:<24} | {:>12} |\n", "Atomic", "");
    std::cout << std::format("| {:<24} | {:>12} |\n", "  Total Sum", atomic_sum.load());
    std::cout << std::format("| {:<24} | {:>12} |\n", "  Time (ms)", atomic_time);
    std::cout << std::format("{:-<29}{:-<14}\n", "", "");

    std::cout << std::format("| {:<24} | {:>12} |\n", "Mutex", "");
    std::cout << std::format("| {:<24} | {:>12} |\n", "  Total Sum", mutex_sum);
    std::cout << std::format("| {:<24} | {:>12} |\n", "  Time (ms)", mutex_time);
    std::cout << std::format("{:-<29}{:-<14}\n", "", "");

    return 0;
}