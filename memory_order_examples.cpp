/**
 * =============================================================================
 * MEMORY ORDERING IN C++ - A BEGINNER'S GUIDE
 * =============================================================================
 * 
 * WHAT IS MEMORY ORDERING?
 * ------------------------
 * When you have multiple threads running, they might access shared data.
 * The CPU and compiler can REORDER your instructions for performance.
 * The Compiler can do it and also the hardware (CPU) can execute instructions out of order.
 * Memory ordering tells the CPU/compiler: "Don't mess with this order!"
 * 
 * Think of it like a restaurant:
 * - Without ordering rules: The chef might serve dessert before the main course
 * - With ordering rules: You guarantee the proper sequence of events
 * 
 * WHY DO WE NEED IT?
 * ------------------
 * Modern CPUs are like impatient chefs - they try to do things out of order
 * to be faster. This is usually fine, but with shared data between threads,
 * it can cause bugs where Thread B sees incomplete data from Thread A.
 * 
 * =============================================================================
 */

#include <iostream>
#include <thread>
#include <atomic>
#include <cassert>
#include <vector>
#include <chrono>

/**
 * =============================================================================
 * 1. MEMORY_ORDER_RELAXED - "I just want atomicity, nothing else"
 * =============================================================================
 * 
 * ANALOGY: You're counting people entering a building. You don't care about
 * the ORDER they enter, you just want an accurate COUNT. Each increment
 * is atomic (won't be corrupted), but no ordering guarantees with other data.
 * 
 * USE CASE: Counters, statistics, anything where you just need correct values
 * but don't care about ordering with other variables.
 */
void example_relaxed() {
    std::cout << "\n=== MEMORY_ORDER_RELAXED Example ===\n";
    
    std::atomic<int> counter{0};
    const int increments_per_thread = 10000;    
    
    auto increment = [&]() {
        for (int i = 0; i < increments_per_thread; ++i) {
            // Relaxed: just make the increment atomic
            // No ordering guarantees with other memory operations
            counter.fetch_add(1, std::memory_order_relaxed);
        }
    };
    
    std::thread t1(increment);
    std::thread t2(increment);
    std::thread t3(increment);
    
    t1.join();
    t2.join();
    t3.join();
    
    std::cout << "Final counter value: " << counter.load(std::memory_order_relaxed) << "\n";
    std::cout << "Expected: " << increments_per_thread * 3 << "\n";
    // This will ALWAYS be correct because the operation itself is atomic
}

/**
 * =============================================================================
 * 2. MEMORY_ORDER_ACQUIRE and MEMORY_ORDER_RELEASE - "The Producer-Consumer Pattern"
 * =============================================================================
 * 
 * ANALOGY: Think of a relay race:
 * - RELEASE = When you hand off the baton (everything you did BEFORE matters)
 * - ACQUIRE = When you grab the baton (you can now SEE everything from before)
 * 
 * RELEASE says: "I'm done preparing. Everything I wrote before this point 
 *                is finished and visible to whoever acquires."
 * 
 * ACQUIRE says: "I'm taking over. Show me everything that was done before 
 *                the release."
 * 
 * KEY INSIGHT: Release and Acquire work TOGETHER. Release without Acquire
 * (or vice versa) doesn't give you synchronization.
 */
void example_acquire_release() {
    std::cout << "\n=== MEMORY_ORDER_ACQUIRE / RELEASE Example ===\n";
    
    std::atomic<bool> ready{false};
    int data = 0;  // Non-atomic! Protected by the acquire-release pair
    
    // Producer thread
    std::thread producer([&]() {
        // Step 1: Prepare the data (non-atomic write)
        data = 42;
        
        // Step 2: Signal that data is ready with RELEASE
        // This guarantees: data = 42 HAPPENS BEFORE ready = true
        ready.store(true, std::memory_order_release);
        std::cout << "Producer: Data prepared and flag released\n";
    });
    
    // Consumer thread
    std::thread consumer([&]() {
        // Spin until ready (with ACQUIRE)
        while (!ready.load(std::memory_order_acquire)) {
            // Waiting...
        }
        
        // Now we're guaranteed to see data = 42
        // Because we ACQUIRED after the producer RELEASED
        std::cout << "Consumer: Got data = " << data << "\n";
        assert(data == 42);  // This will ALWAYS pass
    });
    
    producer.join();
    consumer.join();
}

/**
 * =============================================================================
 * 3. MEMORY_ORDER_ACQ_REL - "I'm both reading and writing atomically"
 * =============================================================================
 * 
 * ANALOGY: You're a checkpoint in a relay race. You ACQUIRE the baton from
 * the previous runner AND RELEASE to the next. You're in the middle.
 * 
 * USE CASE: Read-modify-write operations like compare_exchange, fetch_add
 * when you need to synchronize with both producers and consumers.
 */
void example_acq_rel() {
    std::cout << "\n=== MEMORY_ORDER_ACQ_REL Example ===\n";
    
    std::atomic<int> sync_point{0};
    int shared_data = 0;
    
    // Thread 1: Initialize and signal
    std::thread t1([&]() {
        shared_data = 100;
        // Release our write
        sync_point.store(1, std::memory_order_release);
    });
    
    // Thread 2: Middle-man - reads, modifies, passes on
    std::thread t2([&]() {
        int expected = 1;
        // ACQ_REL: Acquire T1's changes, then release for T3
        while (!sync_point.compare_exchange_weak(
            expected, 2, 
            std::memory_order_acq_rel,    // Success: both acquire and release
            std::memory_order_relaxed     // Failure: just retry
        )) {
            expected = 1;  // Reset for retry
        }
        // Now we can see shared_data = 100 (acquired from T1)
        shared_data += 50;  // Modify it
        std::cout << "T2: Modified data to " << shared_data << "\n";
        // Our modification is visible to T3 (released)
    });
    
    // Thread 3: Final consumer
    std::thread t3([&]() {
        // Wait for T2 to finish
        while (sync_point.load(std::memory_order_acquire) != 2) {
            // Waiting...
        }
        std::cout << "T3: Final data = " << shared_data << "\n";
        assert(shared_data == 150);
    });
    
    t1.join();
    t2.join();
    t3.join();
}

/**
 * =============================================================================
 * 4. MEMORY_ORDER_SEQ_CST - "The Strictest, Safest, Slowest"
 * =============================================================================
 * 
 * ANALOGY: Everyone has a synchronized watch showing THE SAME global clock.
 * All threads agree on the exact order of ALL seq_cst operations.
 * 
 * This is the DEFAULT if you don't specify a memory order.
 * 
 * WHY USE IT?
 * - Easiest to reason about
 * - Prevents subtle bugs
 * - Necessary for some algorithms (like Dekker's, Peterson's mutex)
 * 
 * WHY AVOID IT?
 * - Performance cost (especially on ARM/PowerPC architectures)
 * - Often overkill when acquire-release is sufficient
 */
void example_seq_cst() {
    std::cout << "\n=== MEMORY_ORDER_SEQ_CST Example ===\n";
    
    std::atomic<bool> x{false};
    std::atomic<bool> y{false};
    std::atomic<int> z{0};
    
    // This demonstrates why seq_cst matters
    // Without seq_cst, both threads might see (false, true) and (true, false)
    
    std::thread writer1([&]() {
        x.store(true, std::memory_order_seq_cst);
    });
    
    std::thread writer2([&]() {
        y.store(true, std::memory_order_seq_cst);
    });
    
    std::thread reader1([&]() {
        while (!x.load(std::memory_order_seq_cst));  // Wait for x
        if (y.load(std::memory_order_seq_cst)) {
            ++z;  // x happened, y also happened
        }
    });
    
    std::thread reader2([&]() {
        while (!y.load(std::memory_order_seq_cst));  // Wait for y
        if (x.load(std::memory_order_seq_cst)) {
            ++z;  // y happened, x also happened
        }
    });
    
    writer1.join();
    writer2.join();
    reader1.join();
    reader2.join();
    
    // With seq_cst: z MUST be non-zero (at least one reader sees both flags)
    // This is because all threads agree on a single total order
    std::cout << "z = " << z << " (should be > 0 with seq_cst)\n";
    assert(z.load() != 0);
}

/**
 * =============================================================================
 * 5. MEMORY_ORDER_CONSUME (DEPRECATED - Don't use in new code!)
 * =============================================================================
 * 
 * WHY IS IT DEPRECATED?
 * --------------------
 * 
 * 1. TOO COMPLEX TO IMPLEMENT CORRECTLY:
 *    - It was designed to map to "data dependency" tracking in hardware
 *    - The idea: only synchronize values that DEPEND on the loaded value
 *    - Example: if you load a pointer, only protect accesses THROUGH that pointer
 * 
 * 2. NO COMPILER ACTUALLY IMPLEMENTS IT:
 *    - Every major compiler (GCC, Clang, MSVC) just promotes consume to acquire
 *    - The specification was too hard to implement correctly
 *    - "Data dependency" is a concept hard to track through optimizations
 * 
 * 3. SPECIFICATION PROBLEMS:
 *    - The C++ standard's definition of "dependency" was ambiguous
 *    - Hard to define what "carries a dependency" means precisely
 * 
 * 4. MARGINAL PERFORMANCE BENEFIT:
 *    - Only would have helped on weakly-ordered architectures (ARM, PowerPC)
 *    - On x86, acquire and consume would be identical anyway
 *    - The complexity wasn't worth the tiny performance gain
 * 
 * WHAT TO USE INSTEAD:
 * - Use memory_order_acquire - it's stronger but actually works
 * - The performance difference is negligible on modern hardware
 * 
 * EXAMPLE (for educational purposes only - DON'T WRITE NEW CODE LIKE THIS):
 */
void example_consume_deprecated() {
    std::cout << "\n=== MEMORY_ORDER_CONSUME Example (DEPRECATED!) ===\n";
    
    struct Data {
        int value;
        int other;
    };
    
    std::atomic<Data*> ptr{nullptr};
    Data* d = new Data{42, 100};
    
    std::thread producer([&]() {
        d->value = 42;
        d->other = 100;
        // Release the pointer
        ptr.store(d, std::memory_order_release);
    });
    
    std::thread consumer([&]() {
        Data* p;
        // consume WOULD mean: only d->value is synchronized (through p)
        // but NOT necessarily d->other (no dependency through p)
        // In practice: compilers treat this as acquire anyway
        while ((p = ptr.load(std::memory_order_consume)) == nullptr);
        
        // Accessing through p (has dependency)
        std::cout << "Value (dependent): " << p->value << "\n";
        
        // In theory, consume wouldn't guarantee this is synchronized
        // But acquire does, and that's what compilers actually use
        std::cout << "Other (would be unsafe with true consume): " << p->other << "\n";
    });
    
    producer.join();
    consumer.join();
    delete d;
    
    std::cout << "\n*** USE memory_order_acquire INSTEAD OF consume! ***\n";
}

/**
 * =============================================================================
 * COMPARISON TABLE - WHEN TO USE WHAT
 * =============================================================================
 * 
 * ORDER          | USE WHEN...
 * ---------------|-------------------------------------------------------------
 * relaxed        | Just counting/statistics, no synchronization needed
 * acquire        | Reading a flag/value that signals "data is ready"
 * release        | Writing a flag/value to signal "data is ready"
 * acq_rel        | Read-modify-write in the middle of a synchronization chain
 * seq_cst        | Need total ordering, mutex-like algorithms, or unsure
 * consume        | NEVER - it's deprecated, use acquire instead
 * 
 * =============================================================================
 * PERFORMANCE RANKING (fastest to slowest):
 * =============================================================================
 * 
 * 1. relaxed   - Basically free, just prevents tearing
 * 2. acquire   - Memory barrier on load (cheap on x86, costlier on ARM)
 * 3. release   - Memory barrier on store
 * 4. acq_rel   - Both barriers
 * 5. seq_cst   - Full fence + global ordering (most expensive)
 * 
 * NOTE: On x86/x64, the differences are small. On ARM/PowerPC, they matter more.
 * When in doubt, use seq_cst and optimize later if profiling shows a need.
 * 
 * =============================================================================
 */

/**
 * PRACTICAL EXAMPLE: A Simple Spinlock using acquire-release
 */
class SimpleSpinlock {
    std::atomic<bool> locked{false};
    
public:
    void lock() {
        // Keep trying until we successfully set locked from false to true
        while (locked.exchange(true, std::memory_order_acquire)) {
            // Spin - could add yield/backoff here
        }
        // We've acquired the lock!
        // All previous holder's writes are now visible to us
    }
    
    void unlock() {
        // Release the lock - all our writes become visible to the next acquirer
        locked.store(false, std::memory_order_release);
    }
};

void example_spinlock() {
    std::cout << "\n=== Practical Example: Spinlock ===\n";
    
    SimpleSpinlock spinlock;
    int shared_counter = 0;
    const int iterations = 10000;
    
    auto work = [&]() {
        for (int i = 0; i < iterations; ++i) {
            spinlock.lock();
            ++shared_counter;  // Protected by the spinlock
            spinlock.unlock();
        }
    };
    
    std::thread t1(work);
    std::thread t2(work);
    std::thread t3(work);
    
    t1.join();
    t2.join();
    t3.join();
    
    std::cout << "Counter with spinlock: " << shared_counter << "\n";
    std::cout << "Expected: " << iterations * 3 << "\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "C++ Memory Order Examples\n";
    std::cout << "========================================\n";
    
    example_relaxed();
    example_acquire_release();
    example_acq_rel();
    example_seq_cst();
    example_consume_deprecated();
    example_spinlock();
    
    std::cout << "\n========================================\n";
    std::cout << "All examples completed successfully!\n";
    std::cout << "========================================\n";
    
    return 0;
}
