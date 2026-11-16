#ifndef TEST_H
#define TEST_H

#include <stdint.h>
#include <stdio.h>

typedef void (*test_function_t)(void);

void test_record_failure(const char *file, int line, const char *expression);
int test_run(const char *name, test_function_t function);
unsigned int test_failure_count(void);

#define TEST_ASSERT(condition)                                                 \
    do {                                                                       \
        if (!(condition)) {                                                     \
            test_record_failure(__FILE__, __LINE__, #condition);               \
            return;                                                            \
        }                                                                      \
    } while (0)

#define TEST_ASSERT_EQ_U32(expected, actual)                                   \
    do {                                                                       \
        uint32_t test_expected_ = (uint32_t)(expected);                         \
        uint32_t test_actual_ = (uint32_t)(actual);                             \
        if (test_expected_ != test_actual_) {                                   \
            fprintf(stderr, "%s:%d: expected 0x%08X, got 0x%08X\n",           \
                    __FILE__, __LINE__, (unsigned int)test_expected_,           \
                    (unsigned int)test_actual_);                                \
            test_record_failure(__FILE__, __LINE__, #actual);                  \
            return;                                                            \
        }                                                                      \
    } while (0)

#endif

