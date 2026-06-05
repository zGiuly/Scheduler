#pragma once

#include <vector>
#include <string>

struct TaskExecutedEvent {
    std::string project;
    std::string name;
    std::string description;
    std::string type;
    std::string command;
    std::string response;
    long long executionTimeMs;
};

template <typename T>
class IListener {
public:
    virtual ~IListener() = default;
    virtual void onEvent(const T& event) = 0;
};

template <typename T>
class EventDispatcher {
private:
    std::vector<IListener<T>*> listeners;
public:
    void subscribe(IListener<T>* listener) {
        if (!listener) {
            return;
        }
        listeners.push_back(listener);
    }
    void notify(const T& event) {
        for (auto* listener : listeners) {
            if (!listener) {
                continue;
            }
            listener->onEvent(event);
        }
    }
};

class EventManager {
private:
    EventDispatcher<TaskExecutedEvent> taskExecutedDispatcher;
public:
    static EventManager& getInstance() {
        static EventManager instance;
        return instance;
    }
    EventDispatcher<TaskExecutedEvent>& getTaskExecutedDispatcher() {
        return taskExecutedDispatcher;
    }
};
