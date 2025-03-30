#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <sstream>
#include <functional>
#include <atomic>
#include <iomanip>
using namespace std;

// global start time for in-console timestamps
chrono::steady_clock::time_point start_time;

// mutex for console output, used in console_print function
mutex print_mutex;

// print function for console output
void print_in_console(const string &msg) {
    lock_guard<mutex> lock(print_mutex); // object of lock_guard type blocks print_mutex, so that console output doesn't 'tangle'
    chrono::time_point<chrono::steady_clock> now = chrono::steady_clock::now();
    double time_elapsed = chrono::duration<double>(now - start_time).count(); // 
    ostringstream oss;
    oss << std::fixed << std::setprecision(3) << time_elapsed; // set time precision to view to 3
    cout << msg << " (" << oss.str() << " s)" << endl; // output
}

// semaphore implementation 
class Semaphore {
private:
    mutex mtx;
    condition_variable condition_v;
    int resource_count; // current count of available resources - how many times can we go into the critical section
public:
    Semaphore(int count_) : resource_count(count_) {} // secures availability of count to one thread

    // standard wait operation (P) - puts a thread to sleep when no resources are available (count == 0), takes a resource when its available (count--)
    void wait() {
        unique_lock<mutex> lock(mtx); // unique lock guarantees exclusive availablity to 'count' variable to one thread
        while (resource_count == 0) {
            condition_v.wait(lock);
        }
        resource_count--;
    }
    
    // wait function (but only for certain time) - returns true if semaphore was acquired, false if time passed
    bool wait_for(chrono::milliseconds timeout) {
        unique_lock<mutex> lock(mtx);
        bool success = condition_v.wait_for(lock, timeout, [this]{ return resource_count > 0; });
        if (success) {
            resource_count--;
        }
        return success;
    }

    // signal operation (V)
    void signal() {
        unique_lock<mutex> lock(mtx);
        resource_count++; // resource available again
        condition_v.notify_one(); // notifies one thread (wakes it up) and that thread might go into critical section
    }
};

int num_Philosophers = 5; // default number of philosopfers = number of forks
vector<mutex> forks;   // one mutex per fork

// PHILOSOPHER thread function - thread simulates the behaviour/actions of one philospher
void philosopher(int id, Semaphore &semaphore, atomic<bool> &running) {
    while (running.load()) { // loop will go on till the flag is false
        // "thinking" state
        print_in_console("Philosopher " + to_string(id) + " is thinking.");
        this_thread::sleep_for(chrono::milliseconds((rand() % 1000) + 500)); // philosopher will spend <0.5 - 1.5>s in "thinking" state

        if (!running.load()) break; // thread stops if flag has changed
        // "hungry" state
        print_in_console("Philosopher " + to_string(id) + " is hungry.");

        bool semaphore_acquired = false;
        while (running.load() && !semaphore_acquired) {
            semaphore_acquired = semaphore.wait_for(chrono::milliseconds(100)); // waiting to get semaphore, checking every 100 ms
        }
        if (!semaphore_acquired) break;

        // "picking up forks" state
        // left fork = id, right fork = (id+1) % numPhilosophers
        forks[id].lock();
        print_in_console("Philosopher " + to_string(id) + " picked up left fork " + to_string(id) + ".");
        forks[(id + 1) % num_Philosophers].lock();
        print_in_console("Philosopher " + to_string(id) + " picked up right fork " + to_string((id + 1) % num_Philosophers) + ".");

        // "eating" state
        print_in_console("Philosopher " + to_string(id) + " is eating.");
        this_thread::sleep_for(std::chrono::milliseconds((rand() % 1000) + 500)); // eats for <0.5, 1.5>s

        // "putting down forks" state
        forks[id].unlock();
        forks[(id + 1) % num_Philosophers].unlock();
        print_in_console("Philosopher " + to_string(id) + " put down both forks.");

        // signaling the semaphore so that another philosopher can proceed and could pick up forks and eat
        semaphore.signal();
    }
}

int main(int argc, char* argv[]) {
    int runDurationSeconds = 60; // default duration

    // reads number of philosophers from command-line first argument (default = 5)
    if (argc >= 2) {
        istringstream iss(argv[1]);
        if (!(iss >> num_Philosophers)) {
            cerr << "Invalid number of philosophers." << endl;
            return 1;
        }
    } else {
        num_Philosophers = 5;
    }
    
    // reads program duration from second argument (default = 60)
    if (argc >= 3) {
        istringstream iss2(argv[2]);
        if (!(iss2 >> runDurationSeconds)) {
            cerr << "Invalid run duration." << endl;
            return 1;
        }
    }

    forks = vector<mutex>(num_Philosophers); // forks vector with one mutex per fork
    Semaphore semaphore(num_Philosophers - 1); // semaphore that allows up to (numPhilosophers - 1) philosophers to pick up forks
    atomic<bool> running(true); // atomic flag to control running time - guarantees safe operations by threats
    start_time = chrono::steady_clock::now(); // start time

    // creating and launching philosophers threads
    vector<thread> philosophers;
    for (int i = 0; i < num_Philosophers; i++) {
        philosophers.emplace_back([i, &semaphore, &running]() {
            philosopher(i, semaphore, running);
        });
    }

    this_thread::sleep_for(chrono::seconds(runDurationSeconds)); // main thread will sleep for given duration - time for philosophers cycles
    running = false; // signal all threads to finish

    // waiting for all philosopher threads to complete, then we end the program
    for (auto &t : philosophers) {
        t.join();
    }

    print_in_console("Program finished after " + to_string(runDurationSeconds) + " seconds.");
    return 0;
}
