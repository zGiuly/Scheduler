#include "MySqlDatabase.h"
#include <vector>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>

bool MySqlDatabase::isValidIdentifier(const std::string& identifier) {
    if (identifier.empty() || identifier.size() > 64) {
        return false;
    }
    for (char c : identifier) {
        bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                  (c >= '0' && c <= '9') || c == '_';
        if (!ok) {
            return false;
        }
    }
    return true;
}

Task MySqlDatabase::mapRowToTask(MYSQL_ROW row) {
    Task task;
    task.id = std::stoll(row[0] ? row[0] : "0");
    task.project = row[1] ? row[1] : "";
    task.name = row[2] ? row[2] : "";
    task.description = row[3] ? row[3] : "";
    task.command = row[4] ? row[4] : "";
    task.cron = row[5] ? row[5] : "";
    task.type = parseTaskType(row[6] ? row[6] : "async");
    task.oneshot = (row[7] ? std::stoi(row[7]) : 0) != 0;
    task.disabled = (row[8] ? std::stoi(row[8]) : 0) != 0;
    return task;
}

ExecutionLog MySqlDatabase::mapRowToLog(MYSQL_ROW row) {
    ExecutionLog log;
    log.id = std::stoull(row[0] ? row[0] : "0");
    log.project = row[1] ? row[1] : "";
    log.name = row[2] ? row[2] : "";
    log.description = row[3] ? row[3] : "";
    log.type = parseTaskType(row[4] ? row[4] : "async");
    log.timestamp = row[5] ? row[5] : "";
    log.command = row[6] ? row[6] : "";
    log.response = row[7] ? row[7] : "";
    log.executionTime = std::stoll(row[8] ? row[8] : "0");
    return log;
}

MySqlDatabase::MySqlDatabase(
    const std::string& host,
    const std::string& user,
    const std::string& password,
    const std::string& databaseName,
    unsigned int port
) : host(host), user(user), password(password), databaseName(databaseName), port(port), connection(nullptr) {}

MySqlDatabase::~MySqlDatabase() {
    closeConnection();
}

void MySqlDatabase::closeConnection() {
    if (!connection) {
        return;
    }
    mysql_close(connection);
    connection = nullptr;
}

bool MySqlDatabase::executeQuery(const std::string& query) {
    if (!connection) {
        return false;
    }
    if (mysql_query(connection, query.c_str()) != 0) {
        return false;
    }
    return true;
}

std::string MySqlDatabase::escape(const std::string& value) {
    if (!connection) {
        return "";
    }
    std::vector<char> buffer(value.size() * 2 + 1);
    unsigned long length = mysql_real_escape_string(connection, buffer.data(), value.c_str(), static_cast<unsigned long>(value.size()));
    return std::string(buffer.data(), length);
}

bool MySqlDatabase::executePreparedWrite(const std::string& sql, const std::vector<std::string>& params) {
    if (!connection) {
        return false;
    }
    MYSQL_STMT* stmt = mysql_stmt_init(connection);
    if (!stmt) {
        return false;
    }
    if (mysql_stmt_prepare(stmt, sql.c_str(), static_cast<unsigned long>(sql.size())) != 0) {
        mysql_stmt_close(stmt);
        return false;
    }

    std::vector<MYSQL_BIND> binds(params.size());
    std::vector<unsigned long> lengths(params.size());
    std::memset(binds.data(), 0, binds.size() * sizeof(MYSQL_BIND));
    for (size_t i = 0; i < params.size(); ++i) {
        lengths[i] = static_cast<unsigned long>(params[i].size());
        binds[i].buffer_type = MYSQL_TYPE_STRING;
        binds[i].buffer = const_cast<char*>(params[i].c_str());
        binds[i].buffer_length = lengths[i];
        binds[i].length = &lengths[i];
        binds[i].is_null = nullptr;
    }

    if (!params.empty() && mysql_stmt_bind_param(stmt, binds.data()) != 0) {
        mysql_stmt_close(stmt);
        return false;
    }
    bool ok = mysql_stmt_execute(stmt) == 0;
    mysql_stmt_close(stmt);
    return ok;
}

bool MySqlDatabase::ensureConnection() {
    if (connection && mysql_ping(connection) == 0) {
        return true;
    }

    closeConnection();

    constexpr int maxRetries = 5;
    constexpr int retryDelaySeconds = 2;

    for (int attempt = 1; attempt <= maxRetries; ++attempt) {
        if (initialize()) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::seconds(retryDelaySeconds));
    }

    return false;
}

bool MySqlDatabase::initialize() {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    
    if (!isValidIdentifier(databaseName)) {
        return false;
    }

    connection = mysql_init(nullptr);
    if (!connection) {
        return false;
    }
    mysql_options(connection, MYSQL_SET_CHARSET_NAME, "utf8mb4");
    if (!mysql_real_connect(connection, host.c_str(), user.c_str(), password.c_str(), nullptr, port, nullptr, 0)) {
        closeConnection();
        return false;
    }
    mysql_set_character_set(connection, "utf8mb4");
    if (!executeQuery("CREATE DATABASE IF NOT EXISTS `" + databaseName + "`")) {
        closeConnection();
        return false;
    }
    if (mysql_select_db(connection, databaseName.c_str()) != 0) {
        closeConnection();
        return false;
    }

    std::ifstream schemaFile("schema.sql");
    if (!schemaFile.is_open()) {
        std::string createExecutors = 
            "CREATE TABLE IF NOT EXISTS executors ("
            "id BIGINT AUTO_INCREMENT PRIMARY KEY,"
            "project VARCHAR(100) NOT NULL,"
            "name VARCHAR(100) NOT NULL,"
            "description TEXT,"
            "command TEXT,"
            "cron VARCHAR(45) NOT NULL,"
            "type VARCHAR(45) DEFAULT 'async',"
            "oneshot TINYINT(1) DEFAULT 0,"
            "disabled TINYINT(1) UNSIGNED DEFAULT 0,"
            "INDEX idx_project (project)"
            ")";
        if (!executeQuery(createExecutors)) {
            closeConnection();
            return false;
        }
        std::string createLogs = 
            "CREATE TABLE IF NOT EXISTS executors_logs ("
            "id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,"
            "project VARCHAR(100) NOT NULL,"
            "name VARCHAR(100) NOT NULL,"
            "description TEXT,"
            "type VARCHAR(45) NOT NULL,"
            "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
            "command TEXT,"
            "response TEXT NOT NULL,"
            "execution_time BIGINT,"
            "INDEX idx_project_logs (project)"
            ")";
        if (!executeQuery(createLogs)) {
            closeConnection();
            return false;
        }
        return true;
    }

    std::stringstream buffer;
    buffer << schemaFile.rdbuf();
    std::string content = buffer.str();
    std::string statement;
    std::stringstream contentStream(content);
    while (std::getline(contentStream, statement, ';')) {
        auto start = statement.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) {
            continue;
        }
        auto end = statement.find_last_not_of(" \t\r\n");
        std::string trimmed = statement.substr(start, end - start + 1);
        if (trimmed.empty()) {
            continue;
        }
        std::string lowerTrimmed = trimmed;
        std::transform(lowerTrimmed.begin(), lowerTrimmed.end(), lowerTrimmed.begin(), ::tolower);
        if (lowerTrimmed.rfind("create database", 0) == 0 || lowerTrimmed.rfind("use ", 0) == 0) {
            continue;
        }
        if (!executeQuery(trimmed)) {
            closeConnection();
            return false;
        }
    }
    return true;
}

std::vector<Task> MySqlDatabase::getActiveTasks() {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);

    std::vector<Task> tasks;
    if (!ensureConnection()) {
        return tasks;
    }
    std::string query = "SELECT id, project, name, description, command, cron, type, oneshot, disabled FROM executors WHERE disabled = 0";
    if (mysql_query(connection, query.c_str()) != 0) {
        return tasks;
    }
    MYSQL_RES* result = mysql_store_result(connection);
    if (!result) {
        return tasks;
    }
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        tasks.push_back(mapRowToTask(row));
    }
    mysql_free_result(result);
    return tasks;
}

std::vector<Task> MySqlDatabase::getAllTasks() {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);

    std::vector<Task> tasks;
    if (!ensureConnection()) {
        return tasks;
    }
    std::string query = "SELECT id, project, name, description, command, cron, type, oneshot, disabled FROM executors";
    if (mysql_query(connection, query.c_str()) != 0) {
        return tasks;
    }
    MYSQL_RES* result = mysql_store_result(connection);
    if (!result) {
        return tasks;
    }
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        tasks.push_back(mapRowToTask(row));
    }
    mysql_free_result(result);
    return tasks;
}

bool MySqlDatabase::insertTask(const Task& task) {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);

    if (!ensureConnection()) {
        return false;
    }
    const std::string sql =
        "INSERT INTO executors (project, name, description, command, cron, type, oneshot, disabled) "
        "VALUES (?, ?, ?, ?, ?, ?, " +
        std::string(task.oneshot ? "1" : "0") + ", " +
        std::string(task.disabled ? "1" : "0") + ")";
    return executePreparedWrite(sql, {
        task.project, task.name, task.description,
        task.command, task.cron, taskTypeToString(task.type)
    });
}

bool MySqlDatabase::insertLog(const ExecutionLog& log) {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);

    if (!ensureConnection()) {
        return false;
    }
    const std::string sql =
        "INSERT INTO executors_logs (project, name, description, type, timestamp, command, response, execution_time) "
        "VALUES (?, ?, ?, ?, NOW(), ?, ?, " +
        std::to_string(log.executionTime) + ")";
    return executePreparedWrite(sql, {
        log.project, log.name, log.description, taskTypeToString(log.type),
        log.command, log.response
    });
}

bool MySqlDatabase::disableTask(long long taskId) {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);

    if (!ensureConnection()) {
        return false;
    }
    std::string query = "UPDATE executors SET disabled = 1 WHERE id = " + std::to_string(taskId);
    return executeQuery(query);
}

Task MySqlDatabase::getTask(long long taskId) {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);

    Task task;
    if (!ensureConnection()) {
        return task;
    }
    std::string query = "SELECT id, project, name, description, command, cron, type, oneshot, disabled FROM executors WHERE id = " + std::to_string(taskId);
    if (mysql_query(connection, query.c_str()) != 0) {
        return task;
    }
    MYSQL_RES* result = mysql_store_result(connection);
    if (!result) {
        return task;
    }
    MYSQL_ROW row = mysql_fetch_row(result);
    if (row) {
        task = mapRowToTask(row);
    }
    mysql_free_result(result);
    return task;
}

bool MySqlDatabase::updateTask(const Task& task) {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);

    if (!ensureConnection()) {
        return false;
    }
    const std::string sql =
        "UPDATE executors SET project = ?, name = ?, description = ?, command = ?, cron = ?, type = ?, "
        "oneshot = " + std::string(task.oneshot ? "1" : "0") +
        ", disabled = " + std::string(task.disabled ? "1" : "0") +
        " WHERE id = " + std::to_string(task.id);
    return executePreparedWrite(sql, {
        task.project, task.name, task.description,
        task.command, task.cron, taskTypeToString(task.type)
    });
}

ExecutionLog MySqlDatabase::getLog(unsigned long long logId) {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);

    ExecutionLog log;
    if (!ensureConnection()) {
        return log;
    }
    std::string query = "SELECT id, project, name, description, type, timestamp, command, response, execution_time FROM executors_logs WHERE id = " + std::to_string(logId);
    if (mysql_query(connection, query.c_str()) != 0) {
        return log;
    }
    MYSQL_RES* result = mysql_store_result(connection);
    if (!result) {
        return log;
    }
    MYSQL_ROW row = mysql_fetch_row(result);
    if (row) {
        log = mapRowToLog(row);
    }
    mysql_free_result(result);
    return log;
}

bool MySqlDatabase::deleteLog(unsigned long long logId) {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);

    if (!ensureConnection()) {
        return false;
    }
    std::string query = "DELETE FROM executors_logs WHERE id = " + std::to_string(logId);
    return executeQuery(query);
}

std::vector<ExecutionLog> MySqlDatabase::getLogs() {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);

    std::vector<ExecutionLog> logs;
    if (!ensureConnection()) {
        return logs;
    }
    std::string query = "SELECT id, project, name, description, type, timestamp, command, response, execution_time FROM executors_logs ORDER BY id DESC";
    if (mysql_query(connection, query.c_str()) != 0) {
        return logs;
    }
    MYSQL_RES* result = mysql_store_result(connection);
    if (!result) {
        return logs;
    }
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        logs.push_back(mapRowToLog(row));
    }
    mysql_free_result(result);
    return logs;
}
