#include "TaskValidator.h"
#include <sstream>
#include <vector>

bool TaskValidator::validateCron(const std::string& cron) const {
    std::vector<std::string> fields;
    std::string field;
    std::istringstream stream(cron);
    while (stream >> field) {
        fields.push_back(field);
    }
    constexpr size_t expectedCronFields = 5;
    return fields.size() == expectedCronFields;
}

bool TaskValidator::validate(const Task& task, std::string& errorMessage) const {
    if (task.project.empty()) {
        errorMessage = "Project name cannot be empty";
        return false;
    }
    if (task.name.empty()) {
        errorMessage = "Task name cannot be empty";
        return false;
    }
    if (task.command.empty()) {
        errorMessage = "Command/URL cannot be empty";
        return false;
    }
    if (task.cron.empty()) {
        errorMessage = "Cron expression cannot be empty";
        return false;
    }
    if (!validateCron(task.cron)) {
        errorMessage = "Invalid cron expression. Must contain 5 fields";
        return false;
    }
    if (task.type == TaskType::Unknown) {
        errorMessage = "Task type must be 'async' or 'sync'";
        return false;
    }
    return true;
}
