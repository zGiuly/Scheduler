#include "HttpTaskExecutor.h"
#include "httplib.h"
#include <chrono>
#include <stdexcept>

std::string HttpTaskExecutor::execute(const std::string& command, long long timeoutMs, long long& executionTimeMs) {
    constexpr int defaultHttpPort = 80;
    const std::string scheme = "http://";

    size_t schemePos = command.find(scheme);
    size_t startPos = 0;
    if (schemePos != std::string::npos) {
        startPos = schemePos + scheme.length();
    }

    size_t slashPos = command.find('/', startPos);
    std::string hostPort = (slashPos == std::string::npos) ? command.substr(startPos) : command.substr(startPos, slashPos - startPos);
    std::string path = (slashPos == std::string::npos) ? "/" : command.substr(slashPos);

    std::string host = hostPort;
    int port = defaultHttpPort;
    size_t colonPos = hostPort.find(':');
    if (colonPos != std::string::npos) {
        host = hostPort.substr(0, colonPos);
        try {
            port = std::stoi(hostPort.substr(colonPos + 1));
        }
        catch (const std::exception&) {
            executionTimeMs = 0;
            return "Error: Invalid port in URL";
        }
    }

    auto startTime = std::chrono::high_resolution_clock::now();
    httplib::Client client(host, port);

    long long timeoutSeconds = timeoutMs / 1000;
    long long timeoutMicroseconds = (timeoutMs % 1000) * 1000;
    client.set_connection_timeout(timeoutSeconds, timeoutMicroseconds);
    client.set_read_timeout(timeoutSeconds, timeoutMicroseconds);
    client.set_write_timeout(timeoutSeconds, timeoutMicroseconds);

    auto response = client.Get(path);
    auto endTime = std::chrono::high_resolution_clock::now();

    executionTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    if (!response) {
        if (executionTimeMs >= timeoutMs) {
            return "Error: Timeout exceeded";
        }
        return "Error: Connection failed";
    }

    return "Status: " + std::to_string(response->status) + "\nBody: " + response->body;
}
