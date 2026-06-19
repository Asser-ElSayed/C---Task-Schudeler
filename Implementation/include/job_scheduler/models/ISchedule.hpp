#pragma once
#include <chrono>
#include <optional>
#include <string>

using TimePoint = std::chrono::system_clock::time_point;

class ISchedule {
public:
    virtual std::optional<TimePoint> nextRunAfter(TimePoint from) const = 0;
    virtual std::string type()      const = 0;
    virtual std::string serialize() const = 0;
    virtual ~ISchedule() = default;
};