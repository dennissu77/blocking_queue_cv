#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <atomic>
#include <unordered_map>

// 共享資源
std::queue<int> g_queue;
std::mutex g_mtx;
std::condition_variable g_cv;
std::atomic<bool> g_done(false);

// 記錄 Producer 通知時間與 Consumer 處理時間
std::unordered_map<int, std::chrono::high_resolution_clock::time_point> g_notify_times;
//std::unordered_map<int, std::chrono::high_resolution_clock::time_point> g_consume_times;

// 生產者
void producer(int total_items, int produce_interval_ns) {
    for (int i = 0; i < total_items; ++i) {
        {
            std::lock_guard<std::mutex> lock(g_mtx);
            g_queue.push(i);
            g_notify_times[i] = std::chrono::high_resolution_clock::now();  // 記錄通知時間
            std::cout << "[Producer] Produced item: " << i << std::endl;
        }
        g_cv.notify_one();  // 通知消費者

        std::this_thread::sleep_for(std::chrono::nanoseconds(produce_interval_ns));
    }

    g_done.store(true);
    g_cv.notify_all();
    std::cout << "[Producer] Production done." << std::endl;
}

// 消費者
void consumer() {
    while (true) {
        std::unique_lock<std::mutex> lock(g_mtx);
        g_cv.wait(lock, [] { return !g_queue.empty() || g_done.load(); });

        while (!g_queue.empty()) {
            int item = g_queue.front();
            g_queue.pop();

            auto notify_time = g_notify_times[item];// 要在鎖中讀取 g_notify_times

            lock.unlock();

            /// ===============
            //  車用儀表板渲染區域 
            /// ===============

            // 計算響應時間
            // g_consume_times[item] = std::chrono::high_resolution_clock::now();  // 記錄處理時間
            // auto consume_time = g_consume_times[item];// 如果有多個consumer的話也要在鎖內，目前是單一consumer

            auto consume_time =  std::chrono::high_resolution_clock::now();  // 記錄處理時間
            auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(consume_time - notify_time).count();
            // notify_time 如果有多個consumer的話也要在鎖內，目前是單一consumer

            std::cout << "[Consumer] Consumed item: " << item
                      << " | Latency: " << latency << " ns" << std::endl;

            lock.lock();

            //
        }

        if (g_done.load() && g_queue.empty()) {
            std::cout << "[Consumer] No more items. Exiting..." << std::endl;
            break;
        }
    }
}

int main() {
    const int total_items = 10;
    const int produce_interval_ns = 1000000;  // 1ms

    std::thread producer_thread(producer, total_items, produce_interval_ns);
    std::thread consumer_thread(consumer);

    producer_thread.join();
    consumer_thread.join();

    std::cout << "[Main] Program finished." << std::endl;
    return 0;
}
