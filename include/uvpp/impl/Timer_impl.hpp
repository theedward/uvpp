#ifndef Timer_impl_hpp
#define Timer_impl_hpp

namespace uvpp {
    namespace internal {
        inline void onTimerCall(uv_timer_t* handler) {
            if(handler->data != nullptr) {
                (*static_cast<Timer::Callback*>(handler->data))();
            }
        }
    }
    
    inline Timer::Timer() : _isRunning(false) { }
    
    inline Timer::Timer(uv_loop_t* loop, uint64_t timeout, uint64_t repeat, const Timer::Callback& callback) :
    _isRunning(false), _timeoutInterval(timeout), _repeatInterval(repeat), _callback(callback) {
        _managedTimer = new uv_timer_t();
        uv_timer_init(loop, _managedTimer);
        _managedTimer->data = &_callback;
        uv_timer_start(_managedTimer, internal::onTimerCall, _timeoutInterval, _repeatInterval);
        _isRunning = true;
    }
    
    inline Timer::~Timer() {
        if (isRunning()) {
            stop();
        }
    }
    
    inline bool Timer::isRunning() const {
        return _isRunning;
    }
    
    inline void Timer::run(uv_loop_t* loop, uint64_t timeout, uint64_t repeat, const Timer::Callback& callback) {
        if (isRunning()) {
            stop();
        }
        _managedTimer = new uv_timer_t();
        _timeoutInterval = timeout;
        _repeatInterval = repeat;
        _callback = callback;
        uv_timer_init(loop, _managedTimer);
        _managedTimer->data = &_callback;
        uv_timer_start(_managedTimer, internal::onTimerCall, _timeoutInterval, _repeatInterval);
        _isRunning = true;
    }
    
    inline void Timer::stop() {
        _managedTimer->data = nullptr;
        uv_timer_stop(_managedTimer);
        uv_close((uv_handle_t*)_managedTimer, [](uv_handle_t* timer) { 
            delete timer; 
        });
        _isRunning = false;
    }

}

#endif /* Timer_impl_hpp */