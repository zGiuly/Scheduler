#include "LogDeleteValidator.h"

bool LogDeleteValidator::validate(const std::vector<unsigned long long>& ids, std::string& errorMessage) const {
    if (ids.empty()) {
        errorMessage = "No log IDs provided";
        return false;
    }
    for (unsigned long long id : ids) {
        if (id == 0) {
            errorMessage = "Invalid log ID: 0";
            return false;
        }
    }
    return true;
}
