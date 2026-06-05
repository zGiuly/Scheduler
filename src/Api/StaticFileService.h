#pragma once

#include "httplib.h"
#include <string>

class IStaticFileService {
public:
    virtual ~IStaticFileService() = default;
    virtual bool mount(httplib::Server& server, const std::string& urlPrefix, const std::string& directory) = 0;
};

class StaticFileService : public IStaticFileService {
public:
    bool mount(httplib::Server& server, const std::string& urlPrefix, const std::string& directory) override;
};
