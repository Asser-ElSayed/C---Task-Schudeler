cmake_minimum_required(VERSION 3.16)
project(job_scheduler LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Vendored SQLite ---
add_library(sqlite3 STATIC third_party/sqlite3/sqlite3.c)
target_include_directories(sqlite3 PUBLIC third_party/sqlite3)

find_package(Threads REQUIRED)
target_link_libraries(sqlite3 PUBLIC Threads::Threads)

# --- Project headers ---
include_directories(include)

# --- Source files ---
set(SOURCES
    src/main.cpp
    src/models/OneTimeSchedule.cpp
    src/models/IntervalSchedule.cpp
    src/models/CronSchedule.cpp
    src/repository/SqliteJobRepository.cpp
)

add_executable(job_scheduler ${SOURCES})
target_link_libraries(job_scheduler PRIVATE sqlite3)