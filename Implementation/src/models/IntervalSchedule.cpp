#include "job_scheduler/models/IntervalSchedule.hpp"

IntervalSchedule::IntervalSchedule(int intervalSeconds)
    : _intervalSeconds(intervalSeconds) {}

std::optional<TimePoint> IntervalSchedule::nextRunAfter(TimePoint from) const {
    return from + std::chrono::seconds(_intervalSeconds);
}

std::string IntervalSchedule::type() const { return "interval"; }

std::string IntervalSchedule::serialize() const {
    return std::to_string(_intervalSeconds);
}