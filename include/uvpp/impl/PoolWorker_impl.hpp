#ifndef PoolWorker_impl_hpp
#define PoolWorker_impl_hpp

namespace uvpp {
    template<typename WorkReturn>
    void poolWorkerOnWork(uv_work_t* handler);
    template<typename WorkReturn>
    void poolWorkerOnEnd(uv_work_t* handler, int status);
    
    template <typename WorkReturn>
    inline PoolWorker<WorkReturn>::PoolWorker() : _isRunning(false) {}
    
    template <typename WorkReturn>
    inline PoolWorker<WorkReturn>::PoolWorker(uv_loop_t* loop, const WorkCallback& workCallback, const OnEndCallback& endCallback)
    : _isRunning(false), _workCallback(workCallback), _endCallback(endCallback) {
        runWorker(loop);
    }
    
    template <typename WorkReturn>
    inline PoolWorker<WorkReturn>::~PoolWorker() {
        if (isRunning()) {
            stop();
        } else if (_managedWorker != nullptr) {
            delete static_cast<WorkerData*>(_managedWorker->data);
            delete _managedWorker;
        }
    }
        
    template <typename WorkReturn>
    inline bool PoolWorker<WorkReturn>::isRunning() const {
        return _isRunning;
    }
    
    template <typename WorkReturn>
    inline void PoolWorker<WorkReturn>::run(uv_loop_t* loop, const WorkCallback& workCallback, const OnEndCallback& endCallback) {
        if (isRunning()) {
            stop();
        }
        _workCallback = workCallback;
        _endCallback = endCallback;
        runWorker(loop);
    }
    
    template <typename WorkReturn>
    inline void PoolWorker<WorkReturn>::stop() {
        uv_cancel((uv_req_t*)_managedWorker);
    }
    
    template <typename WorkReturn>
    inline void PoolWorker<WorkReturn>::runWorker(uv_loop_t* loop) {
        _managedWorker = new uv_work_t();
        auto data =  new WorkerData{};
        data->onWork = _workCallback;
        data->onEnd = [this](PoolWorkerOpStatus opStatus, const WorkReturn& returnVal) {
            _endCallback(opStatus, returnVal);
            _isRunning = false;
            _managedWorker = nullptr;
        };
        _managedWorker->data = data;
        uv_queue_work(loop, _managedWorker, poolWorkerOnWork<WorkReturn>, poolWorkerOnEnd<WorkReturn>);
        _isRunning = true;
    }
    
    template<typename WorkReturn>
    inline void poolWorkerOnWork(uv_work_t* handler) {
        if(handler->data != nullptr) {
            auto workerData = static_cast<typename PoolWorker<WorkReturn>::WorkerData*>(handler->data);
            workerData->workReturn = workerData->onWork();
        }
    }
    
    template<typename WorkReturn>
    inline void poolWorkerOnEnd(uv_work_t* handler, int status) {
        auto workerData = static_cast<typename PoolWorker<WorkReturn>::WorkerData*>(handler->data);
        if(handler->data != nullptr && status != UV_ECANCELED) {
            workerData->onEnd(PoolWorkerOpStatus::Success, std::move(workerData->workReturn));
        } else {
            workerData->onEnd(PoolWorkerOpStatus::WorkCancelled, WorkReturn{});
        }
        delete workerData;
        delete handler;
    }

}

#endif /* PoolWorker_impl_hpp */
