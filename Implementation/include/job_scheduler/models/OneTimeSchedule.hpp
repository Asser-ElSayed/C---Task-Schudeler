#pragma once
#include "ISchedule.hpp"
#include <string>

class OneTimeSchedule : public ISchedule {
public:
    explicit OneTimeSchedule(TimePoint runAt);

    std::optional<TimePoint> nextRunAfter(TimePoint from) const override;
    std::string type()      const override;
    std::string serialize() const override;

private:
    TimePoint _runAt;
};