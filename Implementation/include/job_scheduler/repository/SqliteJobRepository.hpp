#pragma once
#include "IJobReader.hpp"
#include "IJobWriter.hpp"
#include "job_scheduler/db/Connection.hpp"
#include "job_scheduler/db/Statement.hpp"
#include <memory>

class SqliteJobRepository : public IJobReader, public IJobWriter {
public:
    explicit SqliteJobRepository(std::shared_ptr<db::Connection> conn);

    std::optional<Job> getById(const std::string& id)  const override;
    std::vector<Job>   listAll()                        const override;
    std::vector<Job>   listDue(TimePoint now)           const override;

    void create(const Job& job)              override;
    void update(const Job& job)              override;
    void remove(const std::string& id)       override;

private:
    std::shared_ptr<db::Connection> _conn;

    void        initSchema();
    Job         rowToJob(db::Statement& stmt) const;
    std::string statusToString(JobStatus s)   const;
    JobStatus   stringToStatus(const std::string& s) const;
};