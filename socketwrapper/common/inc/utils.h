#pragma once

#include <condition_variable>
#include <mutex>
#include <atomic>
#include <string>

namespace socketwrapper {

#ifdef ENABLE_MEASURE_PERFORMANCE
#define MEASURE_PERFORMANCE \
    do {                    \
        std::cout << __func__ << " running" << std::endl;   \
        boost::timer::auto_cpu_timer t; \
    } while(0);
#else
#define MEASURE_PERFORMANCE
#endif

std::string md5sum(const std::string& path);
std::string md5sum(const char * const data, size_t len);
std::string GetCurrentTime();

//
// General RAII handler
//
// int *p = new{7};
// int *buf = (int *)malloc(100 * sizeof(int));
//
// auto cleaner = finally(
//                         [&] {
//                             delete p;
//                            free(buf);
//                        }
//                     );
//

template <typename F>
struct final_action
{
    final_action(F f) : clean_{f}
    {
    }

    ~final_action()
    {
        clean_();
    }

    F clean_;
};

template <class F>
final_action<F> finally(F f)
{
    return final_action<F>(f);
}

class SyncLock
{
public:
    SyncLock() : running_(false) {}

    void Pend();
    void Resume();

private:
      std::atomic<bool>         running_;
      std::mutex                mutex_;
      std::condition_variable   condition_;
};

bool IsPortInUse(unsigned short port);

}   // end socketwrapper
