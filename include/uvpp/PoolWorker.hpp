#ifndef PoolWorker_hpp
#define PoolWorker_hpp

#include <functional>

#include "uv.h"

namespace uvpp {
    enum class PoolWorkerOpStatus { Success, WorkCancelled };
    
    template <typename WorkReturn>
    class PoolWorker {
    public:
        using WorkCallback = std::function<WorkReturn(void)>;
        using OnEndCallback = std::function<void(PoolWorkerOpStatus workerResult, const WorkReturn&)>;
    
        struct WorkerData {
            WorkCallback onWork;
            OnEndCallback onEnd;
            WorkReturn workReturn;
        };
    public:
        PoolWorker();
        PoolWorker(uv_loop_t* loop, const WorkCallback& workCallback, const OnEndCallback& endCallback);
        ~PoolWorker();
        
        bool isRunning() const;
        void run(uv_loop_t* loop, const WorkCallback& workCallback, const OnEndCallback& endCallback);
        void stop();
    private:
        void runWorker(uv_loop_t* loop);
        
        bool _isRunning;
        uv_work_t _managedWorker;
        WorkCallback _workCallback;
        OnEndCallback _endCallback;
    };
}

#include "impl/PoolWorker_impl.hpp"

#endif /* PoolWorker_hpp */

