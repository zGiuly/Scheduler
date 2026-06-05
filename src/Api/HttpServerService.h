#pragma once

#include "httplib.h"
#include "Controllers/TaskController.h"
#include "Controllers/LogController.h"
#include "StaticFileService.h"
#include <memory>
#include <thread>
#include <atomic>

class HttpServerService {
private:
    std::unique_ptr<TaskController> taskController;
    std::unique_ptr<LogController> logController;
    std::unique_ptr<IStaticFileService> staticFileService;
    httplib::Server server;
    std::thread serverThread;
    std::atomic<bool> running;

    void registerRoutes();
    void mountStaticUi();
public:
    HttpServerService(
        std::shared_ptr<IDatabase> db,
        std::shared_ptr<SchedulerService> scheduler
    );
    ~HttpServerService();

    void start(int port);
    void stop();
};
