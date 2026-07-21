#pragma once

#include <array>
#include <atomic>
#include <thread>

#if defined(__arm__) || defined(__aarch64__)
    #define POLARDESIGNER_ARM 1
    #define WAIT_INSTRUCTION __asm__ __volatile__ ("yield" ::: "memory")
#elif defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)
    #include <emmintrin.h>
    #define WAIT_INSTRUCTION _mm_pause()
#else
    #error "Unsupported architecture"
#endif

namespace threadsafety
{
/* A realtime safe spinlock implementation with smart backoff strategy
 */
class SpinLock
{
public:
    // only safe to call from non-RT threads, as it will spin until it can aquire the lock
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

            WAIT_INSTRUCTION;
        }

        while (true)
        {
            for (auto i = 0; i < iterations[2]; ++i)
            {
                if (try_lock())
                    return;

#ifdef POLARDESIGNER_ARM
                WAIT_INSTRUCTION;
#else

                WAIT_INSTRUCTION;
                WAIT_INSTRUCTION;
                WAIT_INSTRUCTION;
                WAIT_INSTRUCTION;
                WAIT_INSTRUCTION;
                WAIT_INSTRUCTION;
                WAIT_INSTRUCTION;
                WAIT_INSTRUCTION;
                WAIT_INSTRUCTION;
                WAIT_INSTRUCTION;
#endif
            }

            std::this_thread::yield();
        }
    }

    // safe to call from every thread
    bool try_lock() noexcept { return ! flag.test_and_set (std::memory_order_acquire); }

    // safe to call from every thread
    void unlock() noexcept { flag.clear (std::memory_order_relaxed); }

private:
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
};
} // namespace threadsafety
