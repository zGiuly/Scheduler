#pragma once

#include <string>
#include <vector>
#include <ctime>

enum class CronFieldType {
    Wildcard,
    List,
    Step,
    Number
};

class ICronEvaluator {
public:
    virtual ~ICronEvaluator() = default;
    virtual bool matches(const std::string& cronExpression, const std::tm& timeInfo) const = 0;
};

class CronEvaluator : public ICronEvaluator {
private:
    CronFieldType getFieldType(const std::string& field) const;
    bool evaluateField(const std::string& field, int currentValue) const;
    std::vector<std::string> split(const std::string& str, char delimiter) const;
public:
    bool matches(const std::string& cronExpression, const std::tm& timeInfo) const override;
};
