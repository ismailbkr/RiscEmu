#include "test.h"

static unsigned int failures;
static unsigned int current_failures;

void test_record_failure(const char *file, int line, const char *expression)
{
    fprintf(stderr, "%s:%d: assertion failed: %s\n", file, line, expression);
    failures++;
    current_failures++;
}

int test_run(const char *name, test_function_t function)
{
    current_failures = 0U;
    function();
    if (current_failures == 0U) {
        printf("[PASS] %s\n", name);
        return 1;
    }
    printf("[FAIL] %s\n", name);
    return 0;
}

unsigned int test_failure_count(void)
{
    return failures;
}

