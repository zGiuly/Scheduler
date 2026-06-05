#include "StaticFileService.h"

bool StaticFileService::mount(httplib::Server& server, const std::string& urlPrefix, const std::string& directory) {
    return server.set_mount_point(urlPrefix, directory);
}
