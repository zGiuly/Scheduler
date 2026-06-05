#include "TaskController.h"
#include <nlohmann/json.hpp>
#include <stdexcept>

TaskController::TaskController(
    std::shared_ptr<IDatabase> db,
    std::shared_ptr<SchedulerService> scheduler,
    std::unique_ptr<IValidator<Task>> val
) : database(db), schedulerService(scheduler), validator(std::move(val)) {}

void TaskController::createTask(const std::string& requestBody, std::string& responseBody, int& statusCode) {
    constexpr int httpCreated = 201;
    constexpr int httpBadRequest = 400;
    constexpr int httpInternalServerError = 500;

    try {
        auto json = nlohmann::json::parse(requestBody);
        Task task;
        task.project = json.at("project").get<std::string>();
        task.name = json.at("name").get<std::string>();
        task.description = json.value("description", "");
        task.command = json.at("command").get<std::string>();
        task.cron = json.at("cron").get<std::string>();
        task.type = parseTaskType(json.value("type", "async"));
        task.oneshot = json.value("oneshot", false);
        task.disabled = json.value("disabled", false);

        std::string valError;
        if (!validator->validate(task, valError)) {
            statusCode = httpBadRequest;
            responseBody = R"({"error": ")" + valError + R"("})";
            return;
        }

        if (!database->insertTask(task)) {
            statusCode = httpInternalServerError;
            responseBody = R"({"error": "Failed to insert task"})";
            return;
        }

        statusCode = httpCreated;
        responseBody = R"({"message": "Task created"})";
    }
    catch (const std::exception&) {
        statusCode = httpBadRequest;
        responseBody = R"({"error": "Invalid request body"})";
    }
}

void TaskController::listTasks(std::string& responseBody, int& statusCode) {
    constexpr int httpOk = 200;
    
    auto tasks = database->getAllTasks();
    nlohmann::json jsonArray = nlohmann::json::array();
    for (const auto& task : tasks) {
        nlohmann::json j;
        j["id"] = task.id;
        j["project"] = task.project;
        j["name"] = task.name;
        j["description"] = task.description;
        j["command"] = task.command;
        j["cron"] = task.cron;
        j["type"] = taskTypeToString(task.type);
        j["oneshot"] = task.oneshot;
        j["disabled"] = task.disabled;
        jsonArray.push_back(j);
    }
    statusCode = httpOk;
    responseBody = jsonArray.dump();
}

void TaskController::triggerTask(long long taskId, std::string& responseBody, int& statusCode) {
    constexpr int httpOk = 200;
    constexpr int httpNotFound = 404;

    Task task = database->getTask(taskId);
    if (task.id == 0) {
        statusCode = httpNotFound;
        responseBody = R"({"error": "Task not found"})";
        return;
    }

    schedulerService->triggerTask(task);
    statusCode = httpOk;
    responseBody = R"({"message": "Task triggered"})";
}

void TaskController::updateTask(long long taskId, const std::string& requestBody, std::string& responseBody, int& statusCode) {
    constexpr int httpOk = 200;
    constexpr int httpBadRequest = 400;
    constexpr int httpNotFound = 404;
    constexpr int httpInternalServerError = 500;

    try {
        Task task = database->getTask(taskId);
        if (task.id == 0) {
            statusCode = httpNotFound;
            responseBody = R"({"error": "Task not found"})";
            return;
        }

        auto json = nlohmann::json::parse(requestBody);
        task.project = json.value("project", task.project);
        task.name = json.value("name", task.name);
        task.description = json.value("description", task.description);
        task.command = json.value("command", task.command);
        task.cron = json.value("cron", task.cron);

        if (json.contains("type")) {
            task.type = parseTaskType(json.at("type").get<std::string>());
        }
        task.oneshot = json.value("oneshot", task.oneshot);
        task.disabled = json.value("disabled", task.disabled);

        std::string valError;
        if (!validator->validate(task, valError)) {
            statusCode = httpBadRequest;
            responseBody = R"({"error": ")" + valError + R"("})";
            return;
        }

        if (!database->updateTask(task)) {
            statusCode = httpInternalServerError;
            responseBody = R"({"error": "Failed to update task"})";
            return;
        }

        statusCode = httpOk;
        responseBody = R"({"message": "Task updated"})";
    }
    catch (const std::exception&) {
        statusCode = httpBadRequest;
        responseBody = R"({"error": "Invalid request body"})";
    }
}
