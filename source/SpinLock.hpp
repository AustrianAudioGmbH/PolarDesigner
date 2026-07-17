#pragma once

#include <array>
#include <atomic>
#include <emmintrin.h>
#include <thread>

namespace threadsafety
{

// TODO: this won't compile on ARM
class SpinLock
{
public:
    void lock()
    {
        static constexpr std::array iterations = { 5, 10, 3000 };

        for (auto i = 0; i < iterations[0]; ++i)
            if (try_lock())
                return;

        for (auto i = 0; i < iterations[1]; ++i)
        {
            if (try_lock())
                return;

            _mm_pause();
        }

        while (true)
        {
            for (auto i = 0; i < iterations[2]; ++i)
            {
                if (try_lock())
                    return;

                _mm_pause();
                _mm_pause();
                _mm_pause();
                _mm_pause();
                _mm_pause();
                _mm_pause();
                _mm_pause();
                _mm_pause();
                _mm_pause();
                _mm_pause();
            }

            std::this_thread::yield();
        }
    }

    bool try_lock() noexcept { return ! flag.test_and_set (std::memory_order_acquire); }

    void unlock() noexcept { flag.clear (std::memory_order_relaxed); }

private:
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
};
} // namespace threadsafety
