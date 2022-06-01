//
//  workerPool.h
//
//  Created by Benjamin Lee on 6/8/17.
//  Copyright © 2017 Benjamin Lee. All rights reserved.
//

#ifndef workerPool_h
#define workerPool_h

#include "epoch.h"

#include <stdio.h>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

template<typename U> class WorkerPoolWorker;

template<typename U>
class WorkerPool {
public:
    WorkerPool(size_t numWorkers);
    ~WorkerPool();
    
    WorkerPool() = delete;
    WorkerPool(const WorkerPool&) = delete;
    WorkerPool& operator=(const WorkerPool&) = delete;
    
    void initialize() {
        maxDepth_ = 0;
        
        for (auto i=0u;i<numWorkers_;++i) {
            auto worker = WorkerPoolWorker<U>(*this, i);
            worker.initialize();
            poolWorkersObjects_.push_back(worker);
            workers_.push_back(std::thread(std::move(worker)));
        }
    }
    
    void add(const U& task);
    void add(U&& task);
    
    size_t  maxDepth() const { return maxDepth_; }
    std::vector<WorkerPoolWorker<U>>& getPoolWorkerObjects() { return poolWorkersObjects_; }

private:
    friend class WorkerPoolWorker<U>;
    
    size_t                      numWorkers_;
    std::vector<std::thread>    workers_;
    std::vector<WorkerPoolWorker<U>>  poolWorkersObjects_;
    
    size_t                      maxDepth_;
    
    std::queue<U>               queue_;
    std::mutex                  mutex_;
    std::condition_variable     cond_;
    
    bool                        running_;
};

template<typename U>
class WorkerPoolWorker {
public:
    WorkerPoolWorker(WorkerPool<U>&, uint32_t id);
    
    WorkerPoolWorker() = delete;
    
    // Sub-classes should override using template specializers
    // Note that this full specification means the function is no longer a template and
    // hence needs to be defined in the .cpp to avoid duplicate symbols
    // initialize show throw on errors
    void initialize();
    
    void operator()();
    
    size_t numExecutions() const { return numExecutions_; }
    double longestExecutionTime() const { return longestExecution_; }
    double totalExecutionTime() const { return totalExecutionTime_; }
    double averageExecutionTime() const {
        if (numExecutions_ == 0) {
            return 0;
        }
        return totalExecutionTime_ / static_cast<double>(numExecutions_);
    }
    
private:
    WorkerPool<U>&  pool_;
    uint32_t        id_;
    
    double longestExecution_;
    double totalExecutionTime_;
    size_t numExecutions_;
};

////////////////////////////////////////////////////////////////////////////////
//
//  WorkerPool
//
////////////////////////////////////////////////////////////////////////////////

template<typename U>
WorkerPool<U>::WorkerPool(size_t numWorkers) : numWorkers_(numWorkers), maxDepth_(0), running_(true) {
}

template<typename U>
WorkerPool<U>::~WorkerPool() {
    std::unique_lock<std::mutex> mlock(mutex_);
    running_ = false;
    mlock.unlock();
    
    cond_.notify_all();
    
    for (auto i=0u;i<workers_.size();++i) {
        if (workers_[i].joinable()) {
            workers_[i].join();
        }
    }
}

template<typename U>
void WorkerPool<U>::add(const U& task) {
    std::unique_lock<std::mutex> mlock(mutex_);
    queue_.push(std::move(task));
    if (queue_.size() > maxDepth_) {
        maxDepth_ = queue_.size();
    }
    mlock.unlock();
    cond_.notify_one();
}

template<typename U>
void WorkerPool<U>::add(U&& task) {
    std::unique_lock<std::mutex> mlock(mutex_);
    queue_.push(std::move(task));
    if (queue_.size() > maxDepth_) {
        maxDepth_ = queue_.size();
    }
    mlock.unlock();
    cond_.notify_one();
}

////////////////////////////////////////////////////////////////////////////////
//
//  WorkerPoolWorker
//
////////////////////////////////////////////////////////////////////////////////

template<typename U>
WorkerPoolWorker<U>::WorkerPoolWorker(WorkerPool<U>& pool, uint32_t id) : pool_(pool), id_(id), longestExecution_(0), totalExecutionTime_(0), numExecutions_(0) {
}

template<typename U>
void WorkerPoolWorker<U>::initialize() {
    longestExecution_ = 0;
    totalExecutionTime_ = 0;
    numExecutions_ = 0;
}

template<typename U>
void WorkerPoolWorker<U>::operator()() {
    while (true) {
        auto& queue = pool_.queue_;
        auto& mutex = pool_.mutex_;
        auto& running = pool_.running_;
        U task;
        
        {   // Create new scope for lock
            std::unique_lock<std::mutex> mlock(mutex);
            
            while (running && queue.empty()) {
                pool_.cond_.wait(mlock);
            }
            
            if (running) {
                task = std::move(queue.front());
                queue.pop();
            } else {
                return;
            }
        }
        
        auto start = static_cast<uint32_t>(EpochTime::timeInMicroSec());
        task.execute();

        auto end = static_cast<uint32_t>(EpochTime::timeInMicroSec());
        auto diff = end - start;
        double time = static_cast<double>(diff) / 1000000.0;
        totalExecutionTime_ += time;
        if (time > longestExecution_) {
            longestExecution_ = time;
        }
        numExecutions_++;
    }
}


#endif /* workerPool_h */
