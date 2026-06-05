cmake_minimum_required(VERSION 3.15)

project("Scheduler" VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.18.3
)
FetchContent_MakeAvailable(httplib)

find_path(MYSQL_INCLUDE_DIR mysql.h
    HINTS
        ENV MYSQL_HOME
    PATH_SUFFIXES
        include
        mysql/include
        mysql
    PATHS
        "C:/Program Files/MySQL/MySQL Server 8.0"
        "C:/Program Files/MySQL/MySQL Server 8.4"
        "C:/Program Files/MySQL/MySQL Server 9.0"
        "C:/Program Files/MySQL"
        "C:/Program Files (x86)/MySQL"
)

find_library(MYSQL_LIBRARIES
    NAMES mysqlclient mysql libmysql
    HINTS
        ENV MYSQL_HOME
    PATH_SUFFIXES
        lib
        mysql/lib
    PATHS
        "C:/Program Files/MySQL/MySQL Server 8.0"
        "C:/Program Files/MySQL/MySQL Server 8.4"
        "C:/Program Files/MySQL/MySQL Server 9.0"
        "C:/Program Files/MySQL"
        "C:/Program Files (x86)/MySQL"
)

set(SOURCES
    src/main.cpp
    src/Core/CronEvaluator.cpp
    src/Database/MySqlDatabase.cpp
    src/Execution/HttpTaskExecutor.cpp
    src/Scheduler/SchedulerService.cpp
    src/Api/HttpServerService.cpp
    src/Api/StaticFileService.cpp
    src/Api/Validators/TaskValidator.cpp
    src/Api/Validators/LogDeleteValidator.cpp
    src/Api/Controllers/TaskController.cpp
    src/Api/Controllers/LogController.cpp
)

add_executable(Scheduler ${SOURCES})

target_include_directories(Scheduler PRIVATE ${MYSQL_INCLUDE_DIR})
target_link_libraries(Scheduler PRIVATE nlohmann_json::nlohmann_json httplib::httplib ${MYSQL_LIBRARIES})

if(WIN32)
    find_file(MYSQL_DLL libmysql.dll
        HINTS
            ENV MYSQL_HOME
        PATH_SUFFIXES
            lib
            bin
        PATHS
            "C:/Program Files/MySQL/MySQL Server 8.0"
            "C:/Program Files/MySQL/MySQL Server 8.4"
            "C:/Program Files/MySQL/MySQL Server 9.0"
            "C:/Program Files/MySQL"
            "C:/Program Files (x86)/MySQL"
    )
    if(MYSQL_DLL)
        add_custom_command(TARGET Scheduler POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MYSQL_DLL}"
            $<TARGET_FILE_DIR:Scheduler>
        )
    endif()
endif()

add_custom_command(TARGET Scheduler POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/wwwroot"
    "$<TARGET_FILE_DIR:Scheduler>/wwwroot"
)
