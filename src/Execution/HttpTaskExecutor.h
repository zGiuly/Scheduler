#pragma once

#include "ITaskExecutor.h"

class HttpTaskExecutor : public ITaskExecutor {
public:
    std::string execute(const std::string& command, long long timeoutMs, long long& executionTimeMs) override;
};
