#ifndef Timer_hpp
#define Timer_hpp

#include <functional>

#include "uv.h"

namespace uvpp {
    class Timer {
    public:
        using Callback = std::function<void(void)>;

    public:
        Timer();
        Timer(uv_loop_t* loop, uint64_t timeout, uint64_t repeat, const Callback& callback);
        ~Timer();
        
        bool isRunning() const;
        void run(uv_loop_t* loop, uint64_t timeout, uint64_t repeat, const Callback& callback);
        void stop();
    private:
        bool _isRunning;
        uv_timer_t* _managedTimer;
        uint64_t _timeoutInterval;
        uint64_t _repeatInterval;
        Callback _callback;
    };
}

#include "impl/Timer_impl.hpp"

#endif /* Timer_hpp */
