#include <iostream>
#include <vector>
#include <random>
#include <future>
#include <chrono>
#include <fstream>
#include <cmath>

class Matrix {
private:
    int rows;
    int cols;
    std::vector<std::vector<int>> data;

public:
    Matrix(int r, int c) : rows(r), cols(c) {
        data.resize(rows, std::vector<int>(cols));
    }

    void fill_random() {
        static std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<int> dist(1, 100);

        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++)
                data[i][j] = dist(gen);
    }

    int get(int r, int c) const {
        return data[r][c];
    }

    int get_rows() const { return rows; }
    int get_cols() const { return cols; }

    void print() const {
        for (int i = 0; i < rows && i < 10; i++) {
            for (int j = 0; j < cols && j < 10; j++) {
                std::cout << data[i][j] << " ";
            }
            if (cols > 10) std::cout << "...";
            std::cout << "\n";
        }
        if (rows > 10) std::cout << "...\n";
    }
};

long long sum_rows(const Matrix& m, int start, int end) {
    long long local_sum = 0;

    for (int i = start; i < end; i++)
        for (int j = 0; j < m.get_cols(); j++)
            local_sum += m.get(i, j);

    return local_sum;
}

long long sum_block(const Matrix& m,
    int r1, int r2,
    int c1, int c2) {

    long long local_sum = 0;

    for (int i = r1; i < r2; i++)
        for (int j = c1; j < c2; j++)
            local_sum += m.get(i, j);

    return local_sum;
}

int main() {
    setlocale(LC_ALL, "");
    int R, C, NUM_THR;

    std::cout << "введите колво строк: ";
    std::cin >> R;
    std::cout << "введите колво столбцов: ";
    std::cin >> C;
    std::cout << "введите колво потоков: ";
    std::cin >> NUM_THR;

    if (NUM_THR <= 0) {
        std::cout << "ошибка: число потоков должно быть > 0\n";
        return 1;
    }

    if (NUM_THR > R) NUM_THR = R;

    Matrix mat(R, C);
    mat.fill_random();

    std::cout << "\nчасть матрицы:\n";
    mat.print();

    std::cout << "\n1 - по строкам\n2 - по блокам\nвыбор: ";
    int choice;
    std::cin >> choice;

    auto start_time = std::chrono::high_resolution_clock::now();

    std::vector<std::future<long long>> futures;
    if (choice == 1) {
        int rows_per_thread = R / NUM_THR;
        int extra = R % NUM_THR;

        int current = 0;

        for (int i = 0; i < NUM_THR; i++) {
            int start = current;
            int end = current + rows_per_thread + (i < extra ? 1 : 0);

            futures.push_back(std::async(std::launch::async,
                sum_rows,
                std::cref(mat),
                start, end));

            current = end;
        }
    }
    else if (choice == 2) {
        int blocks = std::sqrt(NUM_THR);
        while (blocks * blocks < NUM_THR) blocks++;

        int block_r = R / blocks;
        int block_c = C / blocks;

        int created_threads = 0;

        for (int i = 0; i < blocks && created_threads < NUM_THR; i++) {
            for (int j = 0; j < blocks && created_threads < NUM_THR; j++) {

                int r1 = i * block_r;
                int r2 = (i == blocks - 1) ? R : r1 + block_r;

                int c1 = j * block_c;
                int c2 = (j == blocks - 1) ? C : c1 + block_c;

                futures.push_back(std::async(std::launch::async,
                    sum_block,
                    std::cref(mat),
                    r1, r2, c1, c2));

                created_threads++;
            }
        }
    }

    else {
        std::cout << "неверный выбор\n";
        return 1;
    }

    long long total_sum = 0;
    for (auto& f : futures) {
        total_sum += f.get();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    std::cout << "\nрезультаты\n";
    std::cout << "сумма: " << total_sum << "\n";
    std::cout << "время: " << elapsed.count() << " сек\n";
    long long check = 0;
    for (int i = 0; i < R; i++)
        for (int j = 0; j < C; j++)
            check += mat.get(i, j);

    std::cout << "\nпроверка: " << check << "\n";

    if (check == total_sum)
        std::cout << "верно\n";
    else
        std::cout << "ошибка\n";
    std::ofstream file("result.txt");
    if (file.is_open()) {
        file << "сумма: " << total_sum << "\n";
        file << "сремя: " << elapsed.count() << " сек\n";
        file.close();
        std::cout << "результат сохранён в result.txt\n";
    }

    return 0;
}