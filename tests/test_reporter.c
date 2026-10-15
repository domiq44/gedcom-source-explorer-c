#include "test_reporter.h"

#include <stdio.h>

void report_test_result(const char *test_name, int result) {
    const char *status = result > 0 ? "PASS" : (result < 0 ? "SKIP" : "FAIL");

    printf("[%s] %s\n", status, test_name);
}