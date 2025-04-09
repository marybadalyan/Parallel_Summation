# Parallel_Summation
A C++ project demonstrating parallel array summation using multiple threads with different synchronization techniques: non-atomic (reduce-like), atomic, and mutex-based approaches. This repository explores the trade-offs in correctness, complexity, and speed for each method.

---

### Key Concepts
1. **Non-Atomic (Reduce-Like)**:
   - Each thread computes its partial sum independently and stores it in a dedicated slot (e.g., `thread_sums[thread_id]`).
   - After all threads finish, a single-threaded `std::accumulate` combines these partial sums into the final result.
   - No synchronization is needed during computation because threads write to separate memory locations.

2. **Mutex-Based**:
   - Each thread computes its partial sum independently, then updates a shared variable (`mutex_sum`) protected by a `std::mutex`.
   - The mutex ensures only one thread updates the shared sum at a time, introducing synchronization overhead.

3. **Factors Affecting Performance**:
   - **Computation Time**: Time spent summing array elements (proportional to array size per thread).
   - **Synchronization Overhead**: Time spent coordinating threads (e.g., mutex locking/unlocking or final reduction).
   - **Contention**: Delays when multiple threads compete for a shared resource (e.g., the mutex).

---

### Behavior with Large Array Sizes
For large arrays (e.g., 1,000,000 elements in your code), the non-atomic and mutex-based approaches often perform similarly in terms of total execution time. Here’s why:

#### Why They Act Similarly
1. **Dominance of Computation Time**:
   - With a large array, each thread processes a substantial chunk (e.g., 250,000 elements with 4 threads). The time spent computing the partial sum (looping over and adding elements) far outweighs the synchronization overhead.
   - Example: If summing 250,000 integers takes ~10ms per thread, the synchronization step (mutex locking or final reduction) might take ~0.1ms—negligible in comparison.

2. **Low Relative Synchronization Cost**:
   - **Non-Atomic**: The final `std::accumulate` over `thread_count` partial sums (e.g., 4 values) is fast because it’s a small, sequential operation after all threads complete. No contention occurs during computation.
   - **Mutex**: Each thread locks the mutex once to add its partial sum. With only a few threads (e.g., 4), contention is minimal—threads rarely overlap in their mutex requests since computation takes so long. The locking overhead is small relative to the computation time.

3. **Parallelism Scales Both**:
   - Both methods benefit from parallel computation across threads. The bottleneck isn’t synchronization but the CPU-bound summing task, which scales similarly for both.

--- 

### Behavior with Small Array Sizes
For small arrays (e.g., 10 or 100 elements), the non-atomic and mutex-based approaches diverge significantly in performance. Here’s why they differ:

#### Why They Act Differently
1. **Synchronization Overhead Dominates**:
   - With a small array, computation time per thread is minimal (e.g., summing 25 elements with 4 threads might take ~0.01ms). Synchronization overhead becomes a larger fraction of the total time.
   - **Non-Atomic**: The final `std::accumulate` is still fast (summing 4 values), and there’s no contention during computation. Overhead remains low.
   - **Mutex**: Locking/unlocking the mutex takes significant time (e.g., ~0.05ms per lock), especially relative to the tiny computation time. Contention increases if threads finish close together and queue up for the mutex.

2. **Contention Becomes Noticeable**:
   - **Mutex**: With small chunks, threads complete their work quickly and attempt to lock the mutex nearly simultaneously. This leads to contention—threads wait for each other, serializing the updates and amplifying the overhead.
   - **Non-Atomic**: No contention occurs because threads write to separate locations, and the reduction happens after all threads finish, unaffected by timing overlaps.

3. **Thread Creation Overhead**:
   - Spawning threads (via `std::thread`) has a fixed cost (e.g., ~0.1ms). For small arrays, this overhead rivals or exceeds computation time, but it affects both methods equally. However, mutex adds extra synchronization cost on top.

#### How Each Behaves
- **Non-Atomic**: Threads compute tiny chunks quickly, write to `thread_sums`, and finish. The main thread’s `std::accumulate` is trivial (e.g., summing 4 numbers). Total time ≈ thread creation + tiny computation + tiny reduction. It stays efficient.
- **Mutex**: Threads compute quickly, then queue up to lock the mutex. Contention delays updates, and the locking overhead (e.g., 4 locks at ~0.05ms each = 0.2ms) dominates. Total time ≈ thread creation + tiny computation + significant locking delay. It slows down noticeably.

---

## Dependencies

- **C++20 compiler** (for `std::format` and modern features)
- **Standard Template Library (STL)** (for `std::vector`, `std::chrono`, etc.)
- **Custom headers**:
  - [`kaizen.h`](https://github.com/heinsaar/kaizen) (provides `zen::print`, `zen::color`, `zen::cmd_args`, and other utilities)

## Build Instructions

1. Clone the repository:
   ```
   git clone https://github.com/username/Parallel_Summation
   ```
2. Navigate to the repository:
   ```
   cd Parallel_Summation
   ```
3. Generate build files:
   ```
   cmake -DCMAKE_BUILD_TYPE=Release -S . -B build
   ```
4. Build the project:
   ```
   cmake --build build --config Release
   ```
5. Run the executable from the build directory:
   ```
   ./build/Parallel_Summation
   ```

## Usage

Run the program with optional arguments to customize the experiment parameters:

```
./Parallel_Summation --size [num] --threads [num]  
```

- `--size`: Number of ints in the array (default: 1'000'000'000).
- `--threads`: Number of threads for summing an array (default: 5).


## Example Output

Below is sample output from running the program with --size 1'000'000'000 --threads 5 :
```
-------------------------------------------
| Description              |        Value |
-------------------------------------------
| Expected Sum             |   1778492703 |
-------------------------------------------
| Non-Atomic (Reduce-Like) |              |
|   Total Sum              |   1778492703 |
|   Time (ms)              |          199 |
-------------------------------------------
| Atomic                   |              |
|   Total Sum              |   1778492703 |
|   Time (ms)              |          121 |
|   Time (ms)              |          199 |
-------------------------------------------
| Atomic                   |              |
|   Total Sum              |   1778492703 |
|   Time (ms)              |          121 |
| Atomic                   |              |
|   Total Sum              |   1778492703 |
|   Time (ms)              |          121 |
|   Total Sum              |   1778492703 |
|   Time (ms)              |          121 |
|   Time (ms)              |          121 |
-------------------------------------------
| Mutex                    |              |
|   Total Sum              |   1778492703 |
|   Time (ms)              |          115 |
-------------------------------------------
```