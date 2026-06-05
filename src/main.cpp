#include "Database/MySqlDatabase.h"
#include "Database/DatabaseLogger.h"
#include "Core/EventManager.h"
#include "Core/CronEvaluator.h"
#include "Execution/HttpTaskExecutor.h"
#include "Scheduler/SchedulerService.h"
#include "Api/HttpServerService.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <memory>

void runConfigWizard(const std::string& path) {
    constexpr int defaultDbPort = 3306;
    constexpr int defaultHttpPort = 8080;
    constexpr int defaultJobTimeoutMs = 10000;
    
    nlohmann::json configJson;
    std::string input;

    std::cout << "Database Host [localhost]: ";
    std::getline(std::cin, input);
    configJson["db_host"] = input.empty() ? "localhost" : input;

    std::cout << "Database User [root]: ";
    std::getline(std::cin, input);
    configJson["db_user"] = input.empty() ? "root" : input;

    std::cout << "Database Password []: ";
    std::getline(std::cin, input);
    configJson["db_password"] = input;

    std::cout << "Database Name [scheduler]: ";
    std::getline(std::cin, input);
    configJson["db_name"] = input.empty() ? "scheduler" : input;

    std::cout << "Database Port [3306]: ";
    std::getline(std::cin, input);
    try {
        configJson["db_port"] = input.empty() ? defaultDbPort : std::stoi(input);
    }
    catch (const std::exception&) {
        configJson["db_port"] = defaultDbPort;
    }

    std::cout << "HTTP Server Port [8080]: ";
    std::getline(std::cin, input);
    try {
        configJson["http_port"] = input.empty() ? defaultHttpPort : std::stoi(input);
    }
    catch (const std::exception&) {
        configJson["http_port"] = defaultHttpPort;
    }

    std::cout << "Job Timeout (ms) [10000]: ";
    std::getline(std::cin, input);
    try {
        configJson["job_timeout_ms"] = input.empty() ? defaultJobTimeoutMs : std::stoll(input);
    }
    catch (const std::exception&) {
        configJson["job_timeout_ms"] = defaultJobTimeoutMs;
    }

    std::ofstream file(path);
    if (file.is_open()) {
        file << configJson.dump(4);
    }
}

bool loadConfig(const std::string& path, std::string& host, std::string& user, std::string& password, std::string& dbName, unsigned int& dbPort, int& httpPort, long long& jobTimeoutMs) {
    constexpr int defaultDbPort = 3306;
    constexpr int defaultHttpPort = 8080;
    constexpr long long defaultJobTimeoutMs = 10000;
    
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }
    try {
        nlohmann::json configJson;
        file >> configJson;
        host = configJson.value("db_host", "localhost");
        user = configJson.value("db_user", "root");
        password = configJson.value("db_password", "");
        dbName = configJson.value("db_name", "scheduler");
        dbPort = configJson.value("db_port", defaultDbPort);
        httpPort = configJson.value("http_port", defaultHttpPort);
        jobTimeoutMs = configJson.value("job_timeout_ms", defaultJobTimeoutMs);
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

int main() {
    constexpr int defaultDbPort = 3306;
    constexpr int defaultHttpPort = 8080;
    constexpr long long defaultJobTimeoutMs = 10000;
    const std::string configPath = "config.json";

    std::string dbHost;
    std::string dbUser;
    std::string dbPassword;
    std::string dbName;
    unsigned int dbPort = defaultDbPort;
    int httpPort = defaultHttpPort;
    long long jobTimeoutMs = defaultJobTimeoutMs;

    bool initialized = false;
    std::shared_ptr<MySqlDatabase> db;

    while (!initialized) {
        if (!loadConfig(configPath, dbHost, dbUser, dbPassword, dbName, dbPort, httpPort, jobTimeoutMs)) {
            std::cout << "Configuration not found or invalid. Starting configuration wizard..." << std::endl;
            runConfigWizard(configPath);
            continue;
        }

        db = std::make_shared<MySqlDatabase>(dbHost, dbUser, dbPassword, dbName, dbPort);
        if (db->initialize()) {
            initialized = true;
            break;
        }

        std::cout << "Database connection failed. Would you like to re-configure? (y/n): ";
        std::string choice;
        std::getline(std::cin, choice);
        if (choice != "y" && choice != "Y") {
            return 1;
        }
        runConfigWizard(configPath);
    }

    auto logger = std::make_shared<DatabaseLogger>(db);
    EventManager::getInstance().getTaskExecutedDispatcher().subscribe(logger.get());

    auto cron = std::make_shared<CronEvaluator>();
    auto executor = std::make_shared<HttpTaskExecutor>();

    auto scheduler = std::make_shared<SchedulerService>(db, cron, executor, jobTimeoutMs);
    auto server = std::make_shared<HttpServerService>(db, scheduler);

    scheduler->start();
    server->start(httpPort);

    std::cout << "Scheduler and HTTP Server started." << std::endl;
    std::cout << "API Port: " << httpPort << std::endl;
    std::cout << "Type 'exit' to stop." << std::endl;

    std::string input;
    while (std::cin >> input) {
        if (input == "exit") {
            break;
        }
    }

    server->stop();
    scheduler->stop();

    return 0;
}
