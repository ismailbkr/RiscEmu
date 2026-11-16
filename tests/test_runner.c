#include "test.h"

void test_memory_register_tests(unsigned int *passed, unsigned int *total);
void test_decoder_register_tests(unsigned int *passed, unsigned int *total);

static void test_smoke(void)
{
    TEST_ASSERT(1);
}

int main(void)
{
    unsigned int passed = 0U;
    unsigned int total = 0U;

    total++;
    passed += (unsigned int)test_run("test_smoke", test_smoke);
    test_memory_register_tests(&passed, &total);
    test_decoder_register_tests(&passed, &total);

    printf("\n%u/%u tests passed\n", passed, total);
    return test_failure_count() == 0U && passed == total ? 0 : 1;
}

