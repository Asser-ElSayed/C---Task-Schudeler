#include "job_scheduler/models/OneTimeSchedule.hpp"

OneTimeSchedule::OneTimeSchedule(TimePoint runAt)
    : _runAt(runAt) {}

std::optional<TimePoint> OneTimeSchedule::nextRunAfter(TimePoint from) const {
    if (from < _runAt) return _runAt;
    return std::nullopt; // already ran, never runs again
}

std::string OneTimeSchedule::type() const { return "onetime"; }

std::string OneTimeSchedule::serialize() const {
    auto t = std::chrono::system_clock::to_time_t(_runAt);
    return std::to_string(t);
}