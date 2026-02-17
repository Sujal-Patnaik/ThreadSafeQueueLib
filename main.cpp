#include <iostream>
#include <unistd.h>     
#include <sys/wait.h>   
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>
#include <cstdlib>
#include <future>
#include <chrono>

int main(int argc, char** argv) {
    std::atomic<int> counter{0};
    std::thread t1{func, std::ref(counter)};
    t1.join();
    std::cout << counter << std::endl;
}