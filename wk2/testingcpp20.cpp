// Visual Studio check: compiler, C++20, threads, atomics and coroutines
#include <atomic>
#include <chrono>
#include <coroutine>
#include <iostream>
#include <thread>
#include <vector>

struct Task { // minimal C++20 coroutine type

    struct promise_type {
        Task get_return_object() {
            return {std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };

    std::coroutine_handle<promise_type> h;
};

Task hello() {
    std::cout << " coroutine: step 1\n";
    co_await std::suspend_always{};
    std::cout << " coroutine: step 2\n";
}

int main() {
#ifdef _MSC_VER
    std::cout << "Compiler: MSVC " << _MSC_VER << ", C++ standard " <<
        _MSVC_LANG << "\n";
#else
    std::cout << "Compiler: " << __VERSION__ << ", C++ standard " <<
        __cplusplus << "\n";
#endif

    std::cout << "Hardware threads: " <<
        std::thread::hardware_concurrency() << "\n";

    std::atomic<long> count{0};
    auto t0 = std::chrono::steady_clock::now();
    std::vector<std::thread> pool;

    for (int t = 0; t < 4; ++t)
        pool.emplace_back([&] {
            for (int i = 0; i < 1'000'000; ++i)
                count.fetch_add(1);
        });

    for (auto& th : pool)
        th.join();

    auto ms = std::chrono::duration<double,
        std::milli>(std::chrono::steady_clock::now() - t0).count();

    std::cout << "Threads + atomics: count = " << count << " (expected 4000000) in " << ms << " ms\n";

    Task t = hello();
    t.h.resume(); // runs to the co_await
    t.h.resume(); // runs to the end
    t.h.destroy();

    std::cout << (count == 4'000'000 ? "All OK\n" : "ERROR\n");
}