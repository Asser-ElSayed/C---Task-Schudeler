#pragma once
#include "ISchedule.hpp"
#include <string>

class IntervalSchedule : public ISchedule {
public:
    explicit IntervalSchedule(int intervalSeconds);

    std::optional<TimePoint> nextRunAfter(TimePoint from) const override;
    std::string type()      const override;
    std::string serialize() const override;

private:
    int _intervalSeconds;
};