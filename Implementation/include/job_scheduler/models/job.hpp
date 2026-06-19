#pragma once
#include "JobStatus.hpp"
#include "RetryPolicy.hpp"
#include "ISchedule.hpp"
#include <string>
#include <memory>

struct Job {
    std::string              id;
    std::string              name;
    JobStatus                status;
    std::shared_ptr<ISchedule> schedule;
    RetryPolicy              retryPolicy;
    std::string              executionTarget; // command or function name to run
    TimePoint                nextRunTime;
    int                      retryCount = 0;
};