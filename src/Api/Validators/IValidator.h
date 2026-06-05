#pragma once

#include <string>

template <typename T>
class IValidator {
public:
    virtual ~IValidator() = default;
    virtual bool validate(const T& object, std::string& errorMessage) const = 0;
};
