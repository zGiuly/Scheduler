#pragma once

#include <string>

class ITaskExecutor {
public:
    virtual ~ITaskExecutor() = default;
    virtual std::string execute(const std::string& command, long long timeoutMs, long long& executionTimeMs) = 0;
};
