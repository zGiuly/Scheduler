#pragma once

#include "../Core/EventManager.h"
#include "IDatabase.h"
#include <memory>

class DatabaseLogger : public IListener<TaskExecutedEvent> {
private:
    std::shared_ptr<IDatabase> database;
public:
    explicit DatabaseLogger(std::shared_ptr<IDatabase> db) : database(db) {}
    void onEvent(const TaskExecutedEvent& event) override {
        if (!database) {
            return;
        }
        ExecutionLog log;
        log.project = event.project;
        log.name = event.name;
        log.description = event.description;
        log.type = parseTaskType(event.type);
        log.command = event.command;
        log.response = event.response;
        log.executionTime = event.executionTimeMs;
        database->insertLog(log);
    }
};
