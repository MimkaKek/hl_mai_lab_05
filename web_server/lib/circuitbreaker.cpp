#include "circuitbreaker.h"

std::size_t FAIL_COUNT = 3;
std::size_t TIME_LIMIT = 30;
std::size_t SUCCESS_LIMIT = 5;

bool CircuitBreaker::check(const std::string &service_name)
{
    if (services.find(service_name) == std::end(services))
        return true;

    ServiceState &ss = services[service_name];

    std::cout << "# CB CHECK: SERVICE = " << service_name << " | STATE = " << int(ss.state) << " | FAIL = " << ss.fail_count << " | SUCCESS = " << ss.success_count << std::endl;

    switch (ss.state)
    {
        case State::close:
            return true;
        case State::semi_open:
            return true;
        case State::open:
            auto end = std::chrono::high_resolution_clock::now();
            double elapsed_seconds = std::chrono::duration<double>(end - ss.state_time).count();

            if (elapsed_seconds >= TIME_LIMIT)
            {
                std::cout << "# STATE OF " << service_name << " CHANGED FROM " << "open" << " TO " << "semi_open... due to timeout limit" << std::endl;
                ss.state = State::semi_open;
                ss.success_count = 0;
                ss.fail_count = 0;
                return true;
            }

            return false;
    }
    return false;
}

void CircuitBreaker::fail(const std::string &service_name)
{
    if (services.find(service_name) == std::end(services))
    {
        ServiceState ss;
        ss.service = service_name;
        ss.state = State::close;
        ss.fail_count = 1;
        services[service_name] = ss;
    }
    else
    {
        ServiceState &ss = services[service_name];
        if (ss.state == State::close)
        {
            ss.state_time = std::chrono::high_resolution_clock::now();
            ++ss.fail_count;
            if (ss.fail_count > FAIL_COUNT){
                std::cout << "# STATE OF " << service_name << " CHANGED FROM " << "close" << " TO " << "open... due to fail limit" << std::endl;
                ss.state = State::open;
            }
        } 
        else if (ss.state == State::semi_open)
        {
            std::cout << "# STATE OF " << service_name << " CHANGED FROM " << "semi_open" << " TO " << "open... due to semi_open state" << std::endl;
            ss.state = State::open;
            ss.state_time = std::chrono::high_resolution_clock::now();
            ss.success_count = 0;
        }
    }
}

void CircuitBreaker::success(const std::string &service_name)
{
    if (services.find(service_name) != std::end(services))
    {
        ServiceState &ss = services[service_name];
        if (ss.state == State::semi_open)
        {
            ++ss.success_count;
            if (ss.success_count > SUCCESS_LIMIT)
            {
                std::cout << "# STATE OF " << service_name << " CHANGED FROM " << "semi_open" << " TO " << "close... due to success limit" << std::endl;
                ss.state = State::close;
                ss.success_count = 0;
                ss.fail_count = 0;
            }
        } 
    }
}