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



void printQueueSizes() {
    printf("Queue 1 size: %zu\n", queue1.size());
    printf("Queue 2 size: %zu\n", queue2.size());
    printf("Queue 3 size: %zu\n", queue3.size());
}


int generateRandomNumber(int min, int max) {
    static bool initialized = false;
    if (!initialized) {
        std::srand(std::time(nullptr));
        initialized = true;
    }
    return min + std::rand() % (max - min + 1);
}


void processTask(std::queue<Process>& taskQueue, int queueNumber, int priority) {
    while (true) {
        //printf("Queue %d Waiting for lock\n", queueNumber); // Print "Waiting for lock"
        std::unique_lock<std::mutex> lock(mtx);
        if (!taskQueue.empty()) {
            cv.wait(lock, [] { return ready; });
            ready = false;
            // Simulating probability of moving to the next queue
            if (queueNumber == 1) {
                if (rand() % priority == 0) {
                    Process task = taskQueue.front();
                    taskQueue.pop();
                    if (task.duration > 8) {
                        std::this_thread::sleep_for(std::chrono::seconds(8));
                        task.duration -= 8;
                        queue2.push(task); // Push the process to Queue 2
                        printf("Process number %d moved to Queue 2 with duration of %d seconds\n", task.id, task.duration);
                    }
                    else {
                        std::this_thread::sleep_for(std::chrono::seconds(task.duration));
                        printf("Process number %d finished in Queue 1\n", task.id);
                    }
                    printQueueSizes();
                }
            } else if (queueNumber == 2) {
                if (rand() % priority == 0) {
                    Process task = taskQueue.front();
                    taskQueue.pop();
                    if (task.duration > 16) {
                        std::this_thread::sleep_for(std::chrono::seconds(16));
                        task.duration -= 16;
                        if (rand() % 2 == 0) {
                            printf("Process number %d moved to Queue 1 with duration of %d seconds\n", task.id, task.duration);
                            queue1.push(task); // Push the process to Queue 1
                        } else {
                            printf("Process number %d moved to Queue 3 with duration of %d seconds\n", task.id, task.duration);
                            queue3.push(task); // Push the process to Queue 3
                        }   
                    }
                    else {
                        std::this_thread::sleep_for(std::chrono::seconds(task.duration));
                        printf("Process number %d finished in Queue 2\n", task.id);
                    }
                    printQueueSizes();
                }
            } else {
                if (rand() % priority == 0) {
                    Process task = taskQueue.front();
                    taskQueue.pop();
                    std::this_thread::sleep_for(std::chrono::seconds(task.duration));
                    printf("Process number %d finished in Queue 3\n", task.id);
                    printQueueSizes();
                }
            }

        } else {
            if (processed and queue1.empty() and queue2.empty() and queue3.empty()) {
                printf("Queue %d finished\n", queueNumber);
                lock.unlock();
                ready = true;
                cv.notify_all();
                return;
            }
        }

        lock.unlock();
        ready = true;
        cv.notify_all();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}


int main() {

    std::thread t1(processTask, std::ref(queue1), 1, 2);
    std::thread t2(processTask, std::ref(queue2), 2, 3);
    std::thread t3(processTask, std::ref(queue3), 3, 5);    

    for (int i = 1; i <= 10; ++i) {
        std::unique_lock<std::mutex> lock(mtx);
        queue1.push({i, generateRandomNumber(1, 100)});
        printf("Process number %d inserted into Queue 1 with duration of %d seconds\n", i, queue1.back().duration);
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
