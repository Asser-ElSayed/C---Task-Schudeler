#pragma once
#include "job_scheduler/models/Job.hpp"
#include <vector>
#include <optional>
#include <string>

class IJobReader {
public:
    virtual std::optional<Job> getById(const std::string& id)  const = 0;
    virtual std::vector<Job>   listAll()                        const = 0;
    virtual std::vector<Job>   listDue(TimePoint now)           const = 0;
    virtual ~IJobReader() = default;
};