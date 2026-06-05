#include "HttpTaskExecutor.h"
#include "httplib.h"
#include <chrono>
#include <stdexcept>

std::string HttpTaskExecutor::execute(const std::string& command, long long timeoutMs, long long& executionTimeMs) {
#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
    if (command.rfind("https://", 0) == 0) {
        executionTimeMs = 0;
        return "Error: HTTPS is not supported in this build (OpenSSL not found)";
    }
#endif

    std::string schemeHostPort;
    std::string path = "/";
    size_t startPos = 0;

    if (command.rfind("https://", 0) == 0) {
        startPos = 8;
    } else if (command.rfind("http://", 0) == 0) {
        startPos = 7;
    }

    size_t slashPos = command.find('/', startPos);
    if (slashPos == std::string::npos) {
        schemeHostPort = command;
        path = "/";
    } else {
        schemeHostPort = command.substr(0, slashPos);
        path = command.substr(slashPos);
    }

    size_t colonPos = schemeHostPort.find(':', startPos);
    if (colonPos != std::string::npos) {
        try {
            static_cast<void>(std::stoi(schemeHostPort.substr(colonPos + 1)));
        }
        catch (const std::exception&) {
            executionTimeMs = 0;
            return "Error: Invalid port in URL";
        }
    }

    auto startTime = std::chrono::high_resolution_clock::now();
    httplib::Client client(schemeHostPort);

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
