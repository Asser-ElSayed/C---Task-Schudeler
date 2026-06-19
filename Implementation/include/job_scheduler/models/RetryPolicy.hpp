#pragma once

struct RetryPolicy {
    int maxRetries    = 3;
    int backoffSeconds = 5; // Gap betwewen retries
};