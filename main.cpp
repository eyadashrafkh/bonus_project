#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <random>

std::mutex mtx;
std::condition_variable cv;
bool ready = false;
bool processed = false;

struct Process {
    int id;
    int duration; // Time required for processing (1 to 100 seconds)
};

std::queue<Process> queue1, queue2, queue3;

void processTask(std::queue<Process>& taskQueue, int queueNumber, int priority) {
    while (true) {
        std::cout << "Queue " << queueNumber << " started\n";
        std::unique_lock<std::mutex> lock(mtx);
        if (!taskQueue.empty()) {
            // Simulating probability of moving to the next queue
            if (queueNumber == 1) {
                if (rand() % priority == 0) {
                    std::cout << "Queue " << queueNumber<< " Waiting for lock\n"; // Print "Waiting for lock"
                    cv.wait(lock, [] { return ready; });
                    ready = false;
                    std::cout << "Queue " << queueNumber<< " After for lock\n"; // Print "Waiting for lock"

                    Process task = taskQueue.front();
                    taskQueue.pop();
                    if (task.duration > 8) {
                        std::this_thread::sleep_for(std::chrono::seconds(8));
                        task.duration -= 8;
                        queue2.push(task); // Push the process to Queue 2
                        std::cout << "Process number " << task.id << " moved to Queue 2\n";
                      }
                    else {
                        std::this_thread::sleep_for(std::chrono::seconds(task.duration));
                        std::cout << "Process number " << task.id << " finished in Queue 1\n";
                    }
                }
            } else if (queueNumber == 2) {
                if (rand() % priority == 0) {
                    std::cout << "Queue " << queueNumber<< " Waiting for lock\n"; // Print "Waiting for lock"
                    cv.wait(lock, [] { return ready; });
                    ready = false;
                    std::cout << "Queue " << queueNumber<< " After for lock\n"; // Print "Waiting for lock"

                    Process task = taskQueue.front();
                    taskQueue.pop();
                    if (task.duration > 16) {
                        std::this_thread::sleep_for(std::chrono::seconds(16));
                        task.duration -= 16;
                        if (rand() % 2 == 0) {
                            std::cout << "Process number " << task.id << " moved to Queue 1\n";
                            queue1.push(task); // Push the process to Queue 1
                        } else {
                            std::cout << "Process number " << task.id << " moved to Queue 3\n";
                            queue3.push(task); // Push the process to Queue 3
                        }   
                    }
                    else {
                        std::this_thread::sleep_for(std::chrono::seconds(task.duration));
                        std::cout << "Process number " << task.id << " finished in Queue 2\n";
                    }
                }
            } else {
                if (rand() % priority == 0) {
                    std::cout << "Queue " << queueNumber<< " Waiting for lock\n"; // Print "Waiting for lock"
                    cv.wait(lock, [] { return ready; });
                    ready = false;
                    std::cout << "Queue " << queueNumber<< " After for lock\n"; // Print "Waiting for lock"

                    Process task = taskQueue.front();
                    taskQueue.pop();
                    std::this_thread::sleep_for(std::chrono::seconds(task.duration));
                    std::cout << "Process number " << task.id << " finished in Queue 3\n";
                }
            }

        } else {
            if (processed and queue1.empty() and queue2.empty() and queue3.empty()) {
                std::cout << "Queue " << queueNumber << " finished\n";
                lock.unlock();
                ready = true;
                cv.notify_all();
                return;
            }
        }
        std::cout << "Queue " << queueNumber << " unlocking\n";
        lock.unlock();
        ready = true;
        cv.notify_all();
    }
}

int main() {

    std::thread t1(processTask, std::ref(queue1), 1, 2);
    std::thread t2(processTask, std::ref(queue2), 2, 3);
    std::thread t3(processTask, std::ref(queue3), 3, 5);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> durationDistribution(1, 100);

    for (int i = 1; i <= 10; ++i) {
        std::unique_lock<std::mutex> lock(mtx);
        queue1.push({i, durationDistribution(gen)});
        std::cout << "Process number " << i << " inserted into Queue 1 with duration of "<< queue1.back().duration << " seconds"<< std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ready = true;
        lock.unlock();
        cv.notify_all();
    }
    processed = true;
    ready = true;
    cv.notify_all();

    t1.join();
    t2.join();
    t3.join();

    return 0;
}
