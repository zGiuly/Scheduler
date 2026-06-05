#include "HttpServerService.h"
#include "Validators/TaskValidator.h"
#include "Validators/LogDeleteValidator.h"

HttpServerService::HttpServerService(
    std::shared_ptr<IDatabase> db,
    std::shared_ptr<SchedulerService> scheduler
) : running(false) {
    auto taskVal = std::make_unique<TaskValidator>();
    taskController = std::make_unique<TaskController>(db, scheduler, std::move(taskVal));
    auto logVal = std::make_unique<LogDeleteValidator>();
    logController = std::make_unique<LogController>(db, std::move(logVal));
    staticFileService = std::make_unique<StaticFileService>();
    registerRoutes();
    mountStaticUi();
}

void HttpServerService::mountStaticUi() {
    staticFileService->mount(server, "/", "./wwwroot");
    server.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_redirect("/index.html");
    });
}

HttpServerService::~HttpServerService() {
    stop();
}

void HttpServerService::start(int port) {
    if (running) {
        return;
    }
    running = true;
    serverThread = std::thread([this, port]() {
        server.listen("0.0.0.0", port);
    });
}

void HttpServerService::stop() {
    if (!running) {
        return;
    }
    running = false;
    server.stop();
    if (serverThread.joinable()) {
        serverThread.join();
    }
}

void HttpServerService::registerRoutes() {
    server.Post("/api/tasks", [this](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Content-Type", "application/json");
        std::string responseBody;
        int statusCode = 0;
        taskController->createTask(req.body, responseBody, statusCode);
        res.status = statusCode;
        res.set_content(responseBody, "application/json");
    });

    server.Get("/api/tasks", [this](const httplib::Request&, httplib::Response& res) {
        res.set_header("Content-Type", "application/json");
        std::string responseBody;
        int statusCode = 0;
        taskController->listTasks(responseBody, statusCode);
        res.status = statusCode;
        res.set_content(responseBody, "application/json");
    });

    server.Put("/api/tasks/(\\d+)", [this](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Content-Type", "application/json");
        long long taskId = std::stoll(req.matches[1]);
        std::string responseBody;
        int statusCode = 0;
        taskController->updateTask(taskId, req.body, responseBody, statusCode);
        res.status = statusCode;
        res.set_content(responseBody, "application/json");
    });

    server.Get("/api/logs", [this](const httplib::Request&, httplib::Response& res) {
        res.set_header("Content-Type", "application/json");
        std::string responseBody;
        int statusCode = 0;
        logController->listLogs(responseBody, statusCode);
        res.status = statusCode;
        res.set_content(responseBody, "application/json");
    });

    server.Get("/api/logs/(\\d+)", [this](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Content-Type", "application/json");
        unsigned long long logId = std::stoull(req.matches[1]);
        std::string responseBody;
        int statusCode = 0;
        logController->getLog(logId, responseBody, statusCode);
        res.status = statusCode;
        res.set_content(responseBody, "application/json");
    });

    server.Delete("/api/logs", [this](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Content-Type", "application/json");
        std::string responseBody;
        int statusCode = 0;
        logController->deleteLogs(req.body, responseBody, statusCode);
        res.status = statusCode;
        res.set_content(responseBody, "application/json");
    });


    server.Post("/api/tasks/(\\d+)/trigger", [this](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Content-Type", "application/json");
        long long taskId = std::stoll(req.matches[1]);
        std::string responseBody;
        int statusCode = 0;
        taskController->triggerTask(taskId, responseBody, statusCode);
        res.status = statusCode;
        res.set_content(responseBody, "application/json");
    });
}
