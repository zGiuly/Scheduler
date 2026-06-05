#pragma once

#include "IDatabase.h"
#include <string>
#include <mutex>

#ifdef _WIN32
#include <winsock2.h>
#endif
#include <mysql.h>

class MySqlDatabase : public IDatabase {
private:
    std::string host;
    std::string user;
    std::string password;
    std::string databaseName;
    unsigned int port;
    MYSQL* connection;
    std::recursive_mutex dbMutex;

    bool executeQuery(const std::string& query);
    bool executePreparedWrite(const std::string& sql, const std::vector<std::string>& params);
    std::string escape(const std::string& value);
    void closeConnection();
    bool ensureConnection();

    static bool isValidIdentifier(const std::string& identifier);
    static Task mapRowToTask(MYSQL_ROW row);
    static ExecutionLog mapRowToLog(MYSQL_ROW row);
public:
    MySqlDatabase(
        const std::string& host,
        const std::string& user,
        const std::string& password,
        const std::string& databaseName,
        unsigned int port
    );
    ~MySqlDatabase() override;

    bool initialize() override;
    std::vector<Task> getActiveTasks() override;
    std::vector<Task> getAllTasks() override;
    bool insertTask(const Task& task) override;
    bool insertLog(const ExecutionLog& log) override;
    bool disableTask(long long taskId) override;
    Task getTask(long long taskId) override;
    bool updateTask(const Task& task) override;
    ExecutionLog getLog(unsigned long long logId) override;
    bool deleteLog(unsigned long long logId) override;
    std::vector<ExecutionLog> getLogs() override;
};
