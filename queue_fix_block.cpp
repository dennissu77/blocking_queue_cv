#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <atomic>

// 共享資源
std::queue<int> g_queue;
std::mutex g_mtx;
std::condition_variable g_cv;  // 條件變數，通知 Consumer
std::atomic<bool> g_done(false);

// 生產者：將資料寫入 queue 並通知消費者
void producer(int total_items, int produce_interval_ns)
{
    for (int i = 0; i < total_items; ++i) {
        {
            std::lock_guard<std::mutex> lock(g_mtx);
            g_queue.push(i);
            std::cout << "[Producer] Produced item: " << i << std::endl;
        }
        g_cv.notify_one();  // 通知消費者有新資料

        //std::this_thread::sleep_for(std::chrono::milliseconds(100000));
        std::this_thread::sleep_for(std::chrono::nanoseconds(produce_interval_ns));   // 休眠 500 奈秒

    }

    //g_done.store(true);//設定這行可以讓程式正常的關閉 消費者會依照這行決定要不要關閉
    g_cv.notify_all();  // 通知消費者生產已完成
    std::cout << "[Producer] Production done." << std::endl;
}

// 消費者：等待通知後即時處理資料
void consumer()
{
    while (true) {
        std::unique_lock<std::mutex> lock(g_mtx);
        g_cv.wait(lock, [] { return !g_queue.empty() || g_done.load(); });  // 等待通知

        while (!g_queue.empty()) {
            int item = g_queue.front();
            g_queue.pop();
            lock.unlock();  // 釋放鎖，避免處理資料時阻塞 Producer

            // 模擬處理資料（如：即時顯示車況）
            std::cout << "[Consumer] Consumed item: " << item << std::endl;

            lock.lock();  // 處理完畢後再次鎖定
        }

        if (g_done.load() && g_queue.empty()) {
            std::cout << "[Consumer] No more items. Exiting..." << std::endl;
            break;
        }
    }
}

int main()
{
    const int total_items = 10;
    const int produce_interval_ns = 1;

    std::thread producer_thread(producer, total_items, produce_interval_ns);
    std::thread consumer_thread(consumer);

    producer_thread.join();
    consumer_thread.join();

    std::cout << "[Main] Program finished." << std::endl;
    return 0;
}
