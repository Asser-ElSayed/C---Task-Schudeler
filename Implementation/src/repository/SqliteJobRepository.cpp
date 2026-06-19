#include "job_scheduler/repository/SqliteJobRepository.hpp"
#include "job_scheduler/db/Statement.hpp"
#include "job_scheduler/models/OneTimeSchedule.hpp"
#include "job_scheduler/models/IntervalSchedule.hpp"
#include "job_scheduler/models/CronSchedule.hpp"
#include <stdexcept>

SqliteJobRepository::SqliteJobRepository(std::shared_ptr<db::Connection> conn)
    : _conn(std::move(conn))
{
    initSchema();
}

void SqliteJobRepository::initSchema() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS jobs (
            id               TEXT PRIMARY KEY,
            name             TEXT NOT NULL,
            status           TEXT NOT NULL,
            schedule_type    TEXT NOT NULL,
            schedule_data    TEXT NOT NULL,
            execution_target TEXT NOT NULL,
            next_run_time    INTEGER NOT NULL,
            retry_count      INTEGER NOT NULL DEFAULT 0,
            max_retries      INTEGER NOT NULL DEFAULT 3,
            backoff_seconds  INTEGER NOT NULL DEFAULT 5
        );
    )";
    sqlite3_exec(_conn->handle(), sql, nullptr, nullptr, nullptr);
}

std::string SqliteJobRepository::statusToString(JobStatus s) const {
    switch (s) {
        case JobStatus::Active:    return "active";
        case JobStatus::Paused:    return "paused";
        case JobStatus::Running:   return "running";
        case JobStatus::Completed: return "completed";
        case JobStatus::Failed:    return "failed";
        case JobStatus::Cancelled: return "cancelled";
        default:                   return "active";
    }
}

JobStatus SqliteJobRepository::stringToStatus(const std::string& s) const {
    if (s == "paused")    return JobStatus::Paused;
    if (s == "running")   return JobStatus::Running;
    if (s == "completed") return JobStatus::Completed;
    if (s == "failed")    return JobStatus::Failed;
    if (s == "cancelled") return JobStatus::Cancelled;
    return JobStatus::Active;
}

Job SqliteJobRepository::rowToJob(db::Statement& stmt) const {
    Job job;
    job.id              = stmt.columnText(0);
    job.name            = stmt.columnText(1);
    job.status          = stringToStatus(stmt.columnText(2));
    std::string sType   = stmt.columnText(3);
    std::string sData   = stmt.columnText(4);
    job.executionTarget = stmt.columnText(5);
    job.nextRunTime     = std::chrono::system_clock::from_time_t(
                              static_cast<time_t>(stmt.columnInt64(6)));
    job.retryCount                = stmt.columnInt(7);
    job.retryPolicy.maxRetries    = stmt.columnInt(8);
    job.retryPolicy.backoffSeconds = stmt.columnInt(9);

    if (sType == "onetime") {
        auto t = static_cast<time_t>(std::stoll(sData));
        job.schedule = std::make_shared<OneTimeSchedule>(
            std::chrono::system_clock::from_time_t(t));
    } else if (sType == "interval") {
        job.schedule = std::make_shared<IntervalSchedule>(std::stoi(sData));
    } else if (sType == "cron") {
        job.schedule = std::make_shared<CronSchedule>(sData);
    }

    return job;
}

void SqliteJobRepository::create(const Job& job) {
    db::Statement stmt(*_conn,
        "INSERT INTO jobs VALUES (?,?,?,?,?,?,?,?,?,?);");

    stmt.bindText(1, job.id);
    stmt.bindText(2, job.name);
    stmt.bindText(3, statusToString(job.status));
    stmt.bindText(4, job.schedule->type());
    stmt.bindText(5, job.schedule->serialize());
    stmt.bindText(6, job.executionTarget);
    stmt.bindInt64(7, std::chrono::system_clock::to_time_t(job.nextRunTime));
    stmt.bindInt(8, job.retryCount);
    stmt.bindInt(9, job.retryPolicy.maxRetries);
    stmt.bindInt(10, job.retryPolicy.backoffSeconds);
    stmt.exec();
}

void SqliteJobRepository::update(const Job& job) {
    db::Statement stmt(*_conn,
        "UPDATE jobs SET name=?, status=?, schedule_type=?, schedule_data=?, "
        "execution_target=?, next_run_time=?, retry_count=?, "
        "max_retries=?, backoff_seconds=? WHERE id=?;");

    stmt.bindText(1, job.name);
    stmt.bindText(2, statusToString(job.status));
    stmt.bindText(3, job.schedule->type());
    stmt.bindText(4, job.schedule->serialize());
    stmt.bindText(5, job.executionTarget);
    stmt.bindInt64(6, std::chrono::system_clock::to_time_t(job.nextRunTime));
    stmt.bindInt(7, job.retryCount);
    stmt.bindInt(8, job.retryPolicy.maxRetries);
    stmt.bindInt(9, job.retryPolicy.backoffSeconds);
    stmt.bindText(10, job.id);
    stmt.exec();
}

void SqliteJobRepository::remove(const std::string& id) {
    db::Statement stmt(*_conn, "DELETE FROM jobs WHERE id=?;");
    stmt.bindText(1, id);
    stmt.exec();
}

std::optional<Job> SqliteJobRepository::getById(const std::string& id) const {
    db::Statement stmt(*_conn, "SELECT * FROM jobs WHERE id=?;");
    stmt.bindText(1, id);
    if (stmt.step()) return rowToJob(stmt);
    return std::nullopt;
}

std::vector<Job> SqliteJobRepository::listAll() const {
    db::Statement stmt(*_conn, "SELECT * FROM jobs;");
    std::vector<Job> jobs;
    while (stmt.step()) jobs.push_back(rowToJob(stmt));
    return jobs;
}

std::vector<Job> SqliteJobRepository::listDue(TimePoint now) const {
    db::Statement stmt(*_conn,
        "SELECT * FROM jobs WHERE status='active' AND next_run_time <= ?;");
    stmt.bindInt64(1, std::chrono::system_clock::to_time_t(now));
    std::vector<Job> jobs;
    while (stmt.step()) jobs.push_back(rowToJob(stmt));
    return jobs;
}