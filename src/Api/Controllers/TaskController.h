#pragma once

#include "../../Database/IDatabase.h"
#include "../../Scheduler/SchedulerService.h"
#include "../Validators/IValidator.h"
#include <memory>
#include <string>

class TaskController {
private:
    std::shared_ptr<IDatabase> database;
    std::shared_ptr<SchedulerService> schedulerService;
    std::unique_ptr<IValidator<Task>> validator;
public:
    TaskController(
        std::shared_ptr<IDatabase> db,
        std::shared_ptr<SchedulerService> scheduler,
        std::unique_ptr<IValidator<Task>> val
    );

    void createTask(const std::string& requestBody, std::string& responseBody, int& statusCode);
    void listTasks(std::string& responseBody, int& statusCode);
    void triggerTask(long long taskId, std::string& responseBody, int& statusCode);
    void updateTask(long long taskId, const std::string& requestBody, std::string& responseBody, int& statusCode);
};
