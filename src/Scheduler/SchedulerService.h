#pragma once

#include "../Database/IDatabase.h"
#include "../Core/CronEvaluator.h"
#include "../Execution/ITaskExecutor.h"
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>

class SchedulerService {
private:
    std::shared_ptr<IDatabase> database;
    std::shared_ptr<ICronEvaluator> cronEvaluator;
    std::shared_ptr<ITaskExecutor> taskExecutor;
    std::atomic<bool> running;
    std::thread schedulerThread;
    std::thread syncWorkerThread;
    std::queue<Task> syncQueue;
    std::mutex queueMutex;
    std::condition_variable queueCv;
    int lastCheckedMinute;
    long long jobTimeoutMs;

    void run();
    void runSyncWorker();
    void processTask(const Task& task);
    void executeTask(const Task& task);
public:
    SchedulerService(
        std::shared_ptr<IDatabase> db,
        std::shared_ptr<ICronEvaluator> cron,
        std::shared_ptr<ITaskExecutor> executor,
        long long timeoutMs
    );
    ~SchedulerService();

    void start();
    void stop();
    void triggerTask(const Task& task);
};
