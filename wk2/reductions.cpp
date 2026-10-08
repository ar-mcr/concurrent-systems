#include <iostream>
#include <vector>
#include <thread>
#include <functional>
#include <numeric>
#include <random>
#include <chrono>

using namespace std;

int sum_ordered(const int a[], int n){
    int s = 0;
    for (int i = 0; i < n; ++i){
        s += a[i];
    }
    return s;
}

int sum_unordered(const vector<int>& data, int numThreads){
    int block_size = data.size() / numThreads;
    int remainder = data.size() % numThreads;

    vector<int> partial_sums(numThreads, 0);
    vector<thread> threads;

    for (int i = 0; i < numThreads; i++){

        int start = i * block_size + min(i, remainder);
        int size = block_size + (i < remainder ? 1 : 0);

        threads.emplace_back([&, i, start, size](){
            partial_sums[i] = sum_ordered(data.data() + start, size);
        });
    }

    for (auto& t : threads){
        t.join();
    }

    int total = 0;

    for (int sum : partial_sums){
        total += sum;
    }

    return total;
}

int main()
{
    int numThreads = thread::hardware_concurrency();

    if (numThreads == 0)
        numThreads = 4;

    cout << "Hardware threads: " << numThreads << endl;

    vector<int> sizes = {
        10,
        100,
        1000,
        10000,
        100000,
        1000000,
        10000000,
        100000000
    };

    const int numRuns = 10;

    for (int dataSize : sizes)
    {
        vector<int> data(dataSize, 1);

        int expected = accumulate(data.begin(), data.end(), 0);

        double total_ordered_time = 0;
        double total_unordered_time = 0;

        int ordered_result = 0;
        int unordered_result = 0;

        for (int run = 0; run < numRuns; run++)
        {
            auto start_ordered = chrono::high_resolution_clock::now();

            ordered_result = sum_ordered(data.data(), data.size());

            auto end_ordered = chrono::high_resolution_clock::now();

            auto start_unordered = chrono::high_resolution_clock::now();

            unordered_result = sum_unordered(data, numThreads);

            auto end_unordered = chrono::high_resolution_clock::now();

            total_ordered_time +=
                chrono::duration<double, milli>(
                    end_ordered - start_ordered
                ).count();

            total_unordered_time +=
                chrono::duration<double, milli>(
                    end_unordered - start_unordered
                ).count();
        }

        double average_ordered_time = total_ordered_time / numRuns;
        double average_unordered_time = total_unordered_time / numRuns;

        cout << "\nData size: " << dataSize << endl;
        cout << "Ordered result: " << ordered_result << endl;
        cout << "Unordered result: " << unordered_result << endl;
        cout << "Expected result: " << expected << endl;

        cout << "Average ordered time: "
             << average_ordered_time << " ms" << endl;

        cout << "Average unordered time: "
             << average_unordered_time << " ms" << endl;
    }

    return 0;
}