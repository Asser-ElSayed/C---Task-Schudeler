# C++ Job Scheduler Service

A backend service written in C++ that schedules and executes one-time and recurring jobs, with support for cron-like expressions, job status tracking, and restart-safe persistence via SQLite.

---

## Table of Contents

- [Overview](#overview)
- [Architecture](#architecture)
- [Project Structure](#project-structure)
- [Prerequisites](#prerequisites)
- [Getting Started](#getting-started)
- [API Reference](#api-reference)
- [Job Types](#job-types)
- [Job Lifecycle](#job-lifecycle)
- [Team & Phases](#team--phases)
- [Dependencies](#dependencies)

---

## Overview

The Job Scheduler service allows clients to create, manage, and monitor jobs that run automatically at a specified time or on a repeating schedule. Jobs survive service restarts — all state is persisted to a local SQLite database and reloaded on startup.

**Key capabilities:**
- Create one-time, fixed-interval, and cron-expression jobs via a REST API
- Execute jobs on a worker thread pool without blocking the scheduler loop
- Track job status across its full lifecycle (active → running → completed/failed)
- Automatically retry failed jobs according to a configurable retry policy
- Persist all job state locally — no external database required

---

## Architecture

The system is built in five layers, each with a single responsibility:

```
HTTP Request
     │
     ▼
┌─────────────┐
│  Controller │  Parse request, validate input, return response
└──────┬──────┘
       │
       ▼
┌─────────────┐
│   Service   │  Orchestrate: coordinate repo, scheduler, executor
└──────┬──────┘
       │
  ┌────┴─────┬──────────────┐
  ▼          ▼              ▼
┌──────┐ ┌───────────┐ ┌──────────┐
│ Repo │ │ Scheduler │ │ Executor │
└──────┘ └───────────┘ └──────────┘
   │           │              │
SQLite     Min-Heap       Thread Pool
```

- **Repository** — reads and writes job state to SQLite
- **Scheduler** — maintains a min-heap priority queue ordered by next run time; sleeps until the next due job
- **Executor** — dispatches due jobs to a thread pool; applies retry policy on failure

---

## Project Structure

```
Implementation/
├── CMakeLists.txt
├── README.md
├── third_party/
│   └── sqlite3/                  # Vendored SQLite amalgamation
│       ├── sqlite3.c
│       └── sqlite3.h
├── include/
│   └── job_scheduler/
│       ├── models/               # Job, JobStatus, RetryPolicy, ISchedule + 3 schedule types
│       ├── db/                   # RAII wrappers: Connection, Statement
│       ├── repository/           # IJobReader, IJobWriter, SqliteJobRepository
│       ├── scheduler/            # Scheduler, ScheduleParser
│       ├── executor/             # ThreadPool, IJobRunner, Executor
│       ├── service/              # JobService
│       └── api/                  # HttpServer, JobController
├── src/                          # All .cpp implementations
├── tests/                        # Unit tests per component
└── data/                         # jobs.db created here at runtime
```

---

## Prerequisites

| Tool | Version | Notes |
|---|---|---|
| Visual Studio Build Tools | 2019 or later | MSVC C++ compiler (`cl.exe`) |
| CMake | 3.16 or later | [cmake.org/download](https://cmake.org/download) — add to PATH during install |
| Git | Any | For cloning the repository |

> SQLite is vendored directly into the project as source files. No system install required.

---

## Getting Started

### 1. Clone the repository

```bash
git clone https://github.com/your-org/job-scheduler.git
cd "job-scheduler/Implementation"
```

### 2. Configure the build

```cmd
mkdir build
cd build
cmake ..
```

Expected output confirms MSVC is detected:
```
-- Building for: Visual Studio 16 2019
-- The CXX compiler identification is MSVC 19.x.x
-- Configuring done
-- Generating done
```

### 3. Build the project

```cmd
cmake --build .
```

All targets should compile with no errors:
```
sqlite3.vcxproj   -> Debug\sqlite3.lib
job_scheduler.vcxproj -> Debug\job_scheduler.exe
```

### 4. Run the service

```cmd
.\Debug\job_scheduler.exe
```

The service starts, loads any previously saved jobs from `data/jobs.db`, and begins listening for API requests.

### 5. Run the tests

```cmd
ctest -C Debug --output-on-failure
```

---

## API Reference

| Method | Endpoint | Description |
|---|---|---|
| `POST` | `/jobs` | Create a new job |
| `GET` | `/jobs` | List all jobs |
| `GET` | `/jobs/{id}` | Get a specific job by ID |
| `POST` | `/jobs/{id}/pause` | Pause a job |
| `POST` | `/jobs/{id}/resume` | Resume a paused job |
| `DELETE` | `/jobs/{id}` | Delete a job permanently |

### Create a job — request body

```json
{
  "name": "my-job",
  "scheduleType": "interval",
  "scheduleData": "60",
  "executionTarget": "my_function",
  "retryPolicy": {
    "maxRetries": 3,
    "backoffSeconds": 5
  }
}
```

### List jobs — response

```json
[
  {
    "id": "a1b2c3",
    "name": "my-job",
    "type": "interval",
    "status": "active",
    "nextRunTime": "2025-01-01T12:00:00Z"
  }
]
```

---

## Job Types

| Type | `scheduleType` value | `scheduleData` format | Example |
|---|---|---|---|
| One-time | `onetime` | Unix timestamp (seconds) | `"1735689600"` |
| Fixed interval | `interval` | Interval in seconds | `"60"` |
| Cron expression | `cron` | 5-field cron string | `"30 9 * * 1"` |

### Cron expression format

```
┌─ minute   (0–59)
│ ┌─ hour   (0–23)
│ │ ┌─ day of month (1–31)
│ │ │ ┌─ month (1–12)
│ │ │ │ ┌─ day of week (0–6, Sunday=0)
│ │ │ │ │
* * * * *
```

Use `*` as a wildcard for any field. Example: `"0 8 * * 1"` runs every Monday at 08:00.

---

## Job Lifecycle

```
Created
   │
   ▼
Active ──── pause ──► Paused
   │                    │
   │         resume ────┘
   ▼
Running
   │
   ├── success ──► Completed  (one-time jobs)
   │               Active     (recurring jobs, rescheduled)
   │
   └── failure ──► retry? ──► Running (retry attempt)
                      │
                      └── retries exhausted ──► Failed
```

A job can be deleted from any state. Deleted jobs are removed from the database immediately and will not execute.

---

## Team & Phases

| Phase | Owner | Scope |
|---|---|---|
| Phase 0 | Asser El Sayed | Models, DB RAII wrappers, Repository layer |
| Phase 1 | Youssef Waleed | Scheduler, Executor, Service, API layer |
| Phase 2 | Mohamed Badie | Tests, integration, acceptance criteria verification |

---

## Dependencies

| Library | Purpose | How it's included |
|---|---|---|
| [SQLite](https://sqlite.org) | Local job persistence | Vendored amalgamation (`third_party/sqlite3/`) — no install needed |
