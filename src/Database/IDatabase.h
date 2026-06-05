#pragma once

#include <string>
#include <vector>

enum class TaskType {
    Async,
    Sync,
    Unknown
};

inline TaskType parseTaskType(const std::string& typeStr) {
    if (typeStr == "sync") {
        return TaskType::Sync;
    }
    if (typeStr == "async") {
        return TaskType::Async;
    }
    return TaskType::Unknown;
}

inline std::string taskTypeToString(TaskType type) {
    switch (type) {
        case TaskType::Sync:
            return "sync";
        case TaskType::Async:
            return "async";
        case TaskType::Unknown:
        default:
            return "unknown";
    }
}

struct Task {
    long long id = 0;
    std::string project;
    std::string name;
    std::string description;
    std::string command;
    std::string cron;
    TaskType type = TaskType::Async;
    bool oneshot = false;
    bool disabled = false;
};

struct ExecutionLog {
    unsigned long long id = 0;
    std::string project;
    std::string name;
    std::string description;
    TaskType type = TaskType::Async;
    std::string timestamp;
    std::string command;
    std::string response;
    long long executionTime = 0;
};

class IDatabase {
public:
    virtual ~IDatabase() = default;
    virtual bool initialize() = 0;
    virtual std::vector<Task> getActiveTasks() = 0;
    virtual std::vector<Task> getAllTasks() = 0;
    virtual bool insertTask(const Task& task) = 0;
    virtual bool insertLog(const ExecutionLog& log) = 0;
    virtual bool disableTask(long long taskId) = 0;
    virtual Task getTask(long long taskId) = 0;
    virtual bool updateTask(const Task& task) = 0;
    virtual ExecutionLog getLog(unsigned long long logId) = 0;
    virtual bool deleteLog(unsigned long long logId) = 0;
    virtual std::vector<ExecutionLog> getLogs() = 0;
};
