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

    g_done.store(true); // 如果不需要消費者在生產者結束後終止可以取消這段
    g_cv.notify_all();
    std::cout << "[Producer] Production done." << std::endl;
}

// 消費者
void consumer() {
    while (true) {
        int item = -1;  // 初始化 item
        std::chrono::high_resolution_clock::time_point notify_time;

        // 將取出任務與讀取通知時間包成一個區塊
        {
            std::unique_lock<std::mutex> lock(g_mtx); // 上鎖作用域  | 如果祖塞在g_cv.wait ，條件變數會釋放鎖，直到被喚醒後才會重新上鎖
            g_cv.wait(lock, [] { return !g_queue.empty() || g_done.load(); }); // 如果queue是空的但生產者沒有結束則阻塞在這 os會讓thread sleep
            
            if (!g_queue.empty()) {
                item = g_queue.front(); // 取出queue中的物件
                g_queue.pop();
                notify_time = g_notify_times[item]; // 取出記錄的通知時間 因爲是共享變數所以需要放在鎖內
            } else if (g_done.load()) { 
                std::cout << "[Consumer] No more items. Exiting..." << std::endl;
                break; // 如果g_done flag 被拉起則會跳出洄圈
            }
        }

        // ============================
        //   車用儀表板渲染區域 
        // ============================

        // 記錄消費時間並計算延遲
        auto consume_time = std::chrono::high_resolution_clock::now();
        auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(consume_time - notify_time).count();

        std::cout << "[Consumer] Consumed item: " << item
                  << " | Latency: " << latency << " ns" << std::endl;
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
