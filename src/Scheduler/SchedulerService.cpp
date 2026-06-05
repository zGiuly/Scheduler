#include "SchedulerService.h"
#include "../Core/EventManager.h"
#include <chrono>
#include <ctime>

SchedulerService::SchedulerService(
    std::shared_ptr<IDatabase> db,
    std::shared_ptr<ICronEvaluator> cron,
    std::shared_ptr<ITaskExecutor> executor,
    long long timeoutMs
) : database(db), cronEvaluator(cron), taskExecutor(executor), running(false), lastCheckedMinute(-1), jobTimeoutMs(timeoutMs) {}

SchedulerService::~SchedulerService() {
    stop();
}

void SchedulerService::start() {
    if (running) {
        return;
    }
    running = true;
    schedulerThread = std::thread(&SchedulerService::run, this);
    syncWorkerThread = std::thread(&SchedulerService::runSyncWorker, this);
}

void SchedulerService::stop() {
    if (!running) {
        return;
    }
    running = false;
    queueCv.notify_all();
    if (schedulerThread.joinable()) {
        schedulerThread.join();
    }
    if (syncWorkerThread.joinable()) {
        syncWorkerThread.join();
    }
}

void SchedulerService::runSyncWorker() {
    while (running) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCv.wait(lock, [this]() { return !syncQueue.empty() || !running; });
            if (!running && syncQueue.empty()) {
                return;
            }
            task = syncQueue.front();
            syncQueue.pop();
        }
        executeTask(task);
    }
}

void SchedulerService::run() {
    constexpr int sleepIntervalMs = 1000;
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepIntervalMs));

        std::time_t now = std::time(nullptr);
        std::tm localTime;
#ifdef _WIN32
        localtime_s(&localTime, &now);
#else
        localtime_r(&now, &localTime);
#endif
        if (localTime.tm_min == lastCheckedMinute) {
            continue;
        }
        lastCheckedMinute = localTime.tm_min;

        auto tasks = database->getActiveTasks();
        for (const auto& task : tasks) {
            if (!cronEvaluator->matches(task.cron, localTime)) {
                continue;
            }
            processTask(task);
        }
    }
}

void SchedulerService::processTask(const Task& task) {
    if (task.oneshot) {
        database->disableTask(task.id);
    }

    switch (task.type) {
        case TaskType::Sync: {
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                syncQueue.push(task);
            }
            queueCv.notify_one();
            break;
        }
        case TaskType::Async:
        default: {
            std::thread([this, task]() {
                executeTask(task);
            }).detach();
            break;
        }
    }
}

void SchedulerService::executeTask(const Task& task) {
    long long durationMs = 0;
    std::string response = taskExecutor->execute(task.command, jobTimeoutMs, durationMs);

    TaskExecutedEvent event;
    event.project = task.project;
    event.name = task.name;
    event.description = task.description;
    event.type = taskTypeToString(task.type);
    event.command = task.command;
    event.response = response;
    event.executionTimeMs = durationMs;

    EventManager::getInstance().getTaskExecutedDispatcher().notify(event);
}

void SchedulerService::triggerTask(const Task& task) {
    std::thread([this, task]() {
        executeTask(task);
    }).detach();
}
