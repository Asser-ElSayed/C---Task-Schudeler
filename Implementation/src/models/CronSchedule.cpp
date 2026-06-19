#include "job_scheduler/models/CronSchedule.hpp"
#include <sstream>
#include <ctime>
#include <stdexcept>

CronSchedule::CronSchedule(const std::string& expression)
    : _expression(expression), _fields(parse(expression)) {}

// Checks if a cron field value is -1 (wildcard *) or matches exactly
bool CronSchedule::fieldMatches(int value, int field) {
    return field == -1 || field == value;
}

CronSchedule::Fields CronSchedule::parse(const std::string& expression) {
    std::istringstream ss(expression);
    std::string token;
    int parts[5];
    int i = 0;

    while (ss >> token && i < 5) {
        parts[i++] = (token == "*") ? -1 : std::stoi(token);
    }
    if (i != 5) throw std::invalid_argument("Cron expression must have exactly 5 fields");

    return { parts[0], parts[1], parts[2], parts[3], parts[4] };
}

bool CronSchedule::isValid(const std::string& expression) {
    try { parse(expression); return true; }
    catch (...) { return false; }
}

std::optional<TimePoint> CronSchedule::nextRunAfter(TimePoint from) const {
    // Advance one minute past 'from', then scan forward up to 1 year
    auto candidate = from + std::chrono::minutes(1);

    for (int i = 0; i < 366 * 24 * 60; ++i) {
        std::time_t t  = std::chrono::system_clock::to_time_t(candidate);
        std::tm*    tm = std::localtime(&t);

        if (fieldMatches(tm->tm_min,  _fields.minute) &&
            fieldMatches(tm->tm_hour, _fields.hour)   &&
            fieldMatches(tm->tm_mday, _fields.dom)    &&
            fieldMatches(tm->tm_mon + 1, _fields.month) &&
            fieldMatches(tm->tm_wday, _fields.dow))
        {
            return candidate;
        }
        candidate += std::chrono::minutes(1);
    }
    return std::nullopt; // no match found within 1 year
}

std::string CronSchedule::type()      const { return "cron"; }
std::string CronSchedule::serialize() const { return _expression; }