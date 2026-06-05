#pragma once

#include "IValidator.h"
#include <vector>

class LogDeleteValidator : public IValidator<std::vector<unsigned long long>> {
public:
    bool validate(const std::vector<unsigned long long>& ids, std::string& errorMessage) const override;
};
