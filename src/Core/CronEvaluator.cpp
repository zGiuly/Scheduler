#include "CronEvaluator.h"
#include <sstream>
#include <algorithm>
#include <stdexcept>

std::vector<std::string> CronEvaluator::split(const std::string& str, char delimiter) const {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

CronFieldType CronEvaluator::getFieldType(const std::string& field) const {
    if (field == "*") {
        return CronFieldType::Wildcard;
    }
    if (field.find(',') != std::string::npos) {
        return CronFieldType::List;
    }
    if (field.find('/') != std::string::npos) {
        return CronFieldType::Step;
    }
    return CronFieldType::Number;
}

bool CronEvaluator::evaluateField(const std::string& field, int currentValue) const {
    switch (getFieldType(field)) {
        case CronFieldType::Wildcard:
            return true;
        case CronFieldType::List: {
            auto parts = split(field, ',');
            for (const auto& part : parts) {
                if (evaluateField(part, currentValue)) {
                    return true;
                }
            }
            return false;
        }
        case CronFieldType::Step: {
            auto parts = split(field, '/');
            if (parts.size() != 2) {
                return false;
            }
            int step = std::stoi(parts[1]);
            if (step <= 0) {
                return false;
            }
            std::string base = parts[0];
            if (base == "*") {
                return currentValue % step == 0;
            }
            int start = std::stoi(base);
            return (currentValue >= start) && ((currentValue - start) % step == 0);
        }
        case CronFieldType::Number: {
            try {
                return currentValue == std::stoi(field);
            }
            catch (const std::exception&) {
                return false;
            }
        }
    }
    return false;
}

bool CronEvaluator::matches(const std::string& cronExpression, const std::tm& timeInfo) const {
    std::vector<std::string> fields;
    std::string field;
    std::istringstream stream(cronExpression);
    while (stream >> field) {
        fields.push_back(field);
    }

    constexpr size_t expectedFieldCount = 5;
    if (fields.size() != expectedFieldCount) {
        return false;
    }

    if (!evaluateField(fields[0], timeInfo.tm_min)) {
        return false;
    }

    if (!evaluateField(fields[1], timeInfo.tm_hour)) {
        return false;
    }

    if (!evaluateField(fields[2], timeInfo.tm_mday)) {
        return false;
    }

    constexpr int monthOffset = 1;
    if (!evaluateField(fields[3], timeInfo.tm_mon + monthOffset)) {
        return false;
    }

    int dayOfWeek = timeInfo.tm_wday;
    if (fields[4] == "7") {
        dayOfWeek = 0;
    }

    return evaluateField(fields[4], dayOfWeek);
}
