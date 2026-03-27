#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <chrono>
std::mutex mtx;
std::condition_variable cv;
std::atomic<int> tekushiy_etap(0); 
void etap_raboty(int nomer_etapa) {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&]() {
        return tekushiy_etap == nomer_etapa;
        });
    std::cout << "этап " << nomer_etapa + 1 << " начался\n";
    lock.unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "этап " << nomer_etapa + 1 << " закончился\n";
    lock.lock();
    tekushiy_etap++;
    lock.unlock();
    cv.notify_all();
}
int main() {
    setlocale(LC_ALL, "RU");
    std::vector<std::thread> potoki;
    for (int i = 0; i < 4; i++) {
        potoki.emplace_back(etap_raboty, i);
    }
    for (auto& p : potoki) {
        p.join();
    }

    std::cout << "все этапы пройдены\n";
    return 0;
}