#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <semaphore>
#include <chrono>
std::queue<int> ochered1;
std::queue<int> ochered2;
std::mutex mtx1;
std::mutex mtx2;
std::mutex mtx_cout;
std::counting_semaphore<10> sem1(0);
std::counting_semaphore<10> sem2(0);
void rabochiy(int id) {
    for (int k = 0; k < 5; k++) {
        int zadacha;
        bool iz_pervoy = false;
        if (sem1.try_acquire()) {
            std::lock_guard<std::mutex> lock(mtx1);

            if (!ochered1.empty()) {
                zadacha = ochered1.front();
                ochered1.pop();
                iz_pervoy = true;
            }
        }
        else {
            sem2.acquire();
            std::lock_guard<std::mutex> lock(mtx2);
            zadacha = ochered2.front();
            ochered2.pop();
        }
        {
            std::lock_guard<std::mutex> lock(mtx_cout);
            std::cout << "рабочий " << id
                << " взял задачу " << zadacha
                << (iz_pervoy ? " из 2" : " из 2") << " очереди\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(300));

        {
            std::lock_guard<std::mutex> lock(mtx_cout);
            std::cout << "рабочий " << id << " закончил задачу " << zadacha << "\n";
        }
        if (iz_pervoy) {
            std::lock_guard<std::mutex> lock(mtx1);
            ochered1.push(zadacha);
            sem1.release();
        }
        else {
            std::lock_guard<std::mutex> lock(mtx2);
            ochered2.push(zadacha);
            sem2.release();
        }
    }
}

int main() {
    setlocale(LC_ALL, "RU");
    for (int i = 1; i <= 5; i++) {
        ochered1.push(i);
        sem1.release();
    }

    for (int i = 6; i <= 10; i++) {
        ochered2.push(i);
        sem2.release();
    }

    std::vector<std::thread> rabochie;
    for (int i = 0; i < 5; i++) {
        rabochie.emplace_back(rabochiy, i);
    }

    for (auto& r : rabochie) {
        r.join();
    }

    std::cout << "все рабочие закончили задачу\n";

    return 0;
}