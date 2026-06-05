#include "LogController.h"
#include <nlohmann/json.hpp>

LogController::LogController(
    std::shared_ptr<IDatabase> db,
    std::unique_ptr<IValidator<std::vector<unsigned long long>>> val
) : database(db), validator(std::move(val)) {}

void LogController::listLogs(std::string& responseBody, int& statusCode) {
    constexpr int httpOk = 200;

    auto logs = database->getLogs();
    nlohmann::json jsonArray = nlohmann::json::array();
    for (const auto& log : logs) {
        nlohmann::json j;
        j["id"] = log.id;
        j["project"] = log.project;
        j["name"] = log.name;
        j["description"] = log.description;
        j["type"] = taskTypeToString(log.type);
        j["timestamp"] = log.timestamp;
        j["command"] = log.command;
        j["response"] = log.response;
        j["execution_time"] = log.executionTime;
        jsonArray.push_back(j);
    }
    statusCode = httpOk;
    responseBody = jsonArray.dump();
}

void LogController::getLog(unsigned long long logId, std::string& responseBody, int& statusCode) {
    constexpr int httpOk = 200;
    constexpr int httpNotFound = 404;

    ExecutionLog log = database->getLog(logId);
    if (log.id == 0) {
        statusCode = httpNotFound;
        responseBody = R"({"error": "Log not found"})";
        return;
    }

    nlohmann::json j;
    j["id"] = log.id;
    j["project"] = log.project;
    j["name"] = log.name;
    j["description"] = log.description;
    j["type"] = taskTypeToString(log.type);
    j["timestamp"] = log.timestamp;
    j["command"] = log.command;
    j["response"] = log.response;
    j["execution_time"] = log.executionTime;

    statusCode = httpOk;
    responseBody = j.dump();
}


void LogController::deleteLogs(const std::string& requestBody, std::string& responseBody, int& statusCode) {
    constexpr int httpOk = 200;
    constexpr int httpBadRequest = 400;
    constexpr int httpInternalServerError = 500;

    try {
        auto json = nlohmann::json::parse(requestBody);
        std::vector<unsigned long long> ids;
        if (json.is_array()) {
            ids = json.get<std::vector<unsigned long long>>();
        } else if (json.is_object() && json.contains("ids") && json["ids"].is_array()) {
            ids = json["ids"].get<std::vector<unsigned long long>>();
        } else {
            statusCode = httpBadRequest;
            responseBody = R"({"error": "Invalid request body"})";
            return;
        }

        std::string valError;
        if (!validator->validate(ids, valError)) {
            statusCode = httpBadRequest;
            responseBody = R"({"error": ")" + valError + R"("})";
            return;
        }

        bool allDeleted = true;
        for (unsigned long long logId : ids) {
            if (!database->deleteLog(logId)) {
                allDeleted = false;
            }
        }

        if (!allDeleted) {
            statusCode = httpInternalServerError;
            responseBody = R"({"error": "Failed to delete some logs"})";
            return;
        }

        statusCode = httpOk;
        responseBody = R"({"message": "Logs deleted"})";
    }
    catch (const std::exception&) {
        statusCode = httpBadRequest;
        responseBody = R"({"error": "Invalid request body"})";
    }
}

