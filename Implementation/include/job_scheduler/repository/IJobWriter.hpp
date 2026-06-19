#pragma once
#include "job_scheduler/models/Job.hpp"
#include <string>

class IJobWriter {
public:
    virtual void create(const Job& job)                          = 0;
    virtual void update(const Job& job)                          = 0;
    virtual void remove(const std::string& id)                   = 0;
    virtual ~IJobWriter() = default;
};