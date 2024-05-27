#pragma once

#include <cstddef>
#include <string>
#include <map>
#include <chrono>
#include <iostream>

enum class State
{
    close = 0,
    open = 1 ,
    semi_open = 2
};

struct ServiceState
{
    State state{State::close};
    std::string service;
    size_t fail_count{0};
    size_t success_count{0};
    std::chrono::_V2::system_clock::time_point state_time;
};

class CircuitBreaker
{
private:
    std::map<std::string, ServiceState> services;
public:
    bool check(const std::string &service_name);
    void fail(const std::string &service_name);
    void success(const std::string &service_name);
};