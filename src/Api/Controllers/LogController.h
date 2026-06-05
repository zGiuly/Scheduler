#pragma once

#include "../../Database/IDatabase.h"
#include "../Validators/IValidator.h"
#include <memory>
#include <string>
#include <vector>

class LogController {
private:
    std::shared_ptr<IDatabase> database;
    std::unique_ptr<IValidator<std::vector<unsigned long long>>> validator;
public:
    explicit LogController(
        std::shared_ptr<IDatabase> db,
        std::unique_ptr<IValidator<std::vector<unsigned long long>>> val
    );

    void listLogs(std::string& responseBody, int& statusCode);
    void getLog(unsigned long long logId, std::string& responseBody, int& statusCode);
    void deleteLogs(const std::string& requestBody, std::string& responseBody, int& statusCode);
};
