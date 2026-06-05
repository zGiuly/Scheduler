#pragma once

#include "IValidator.h"
#include "../../Database/IDatabase.h"

class TaskValidator : public IValidator<Task> {
private:
    bool validateCron(const std::string& cron) const;
public:
    bool validate(const Task& task, std::string& errorMessage) const override;
};
