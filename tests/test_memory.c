#include "memory.h"
#include "test.h"

#include <stddef.h>
#include <stdint.h>

static void test_memory_init_reset_destroy(void)
{
    rv32_memory_t memory = {NULL, 0U};
    size_t index;

    TEST_ASSERT_EQ_U32(RV32_OK, rv32_memory_init(&memory, 16U));
    TEST_ASSERT(memory.data != NULL);
    TEST_ASSERT(memory.size == 16U);
    for (index = 0U; index < memory.size; index++) {
        TEST_ASSERT_EQ_U32(0U, memory.data[index]);
        memory.data[index] = UINT8_C(0xA5);
    }

    TEST_ASSERT_EQ_U32(RV32_OK, rv32_memory_reset(&memory));
    for (index = 0U; index < memory.size; index++) {
        TEST_ASSERT_EQ_U32(0U, memory.data[index]);
    }

    rv32_memory_destroy(&memory);
    TEST_ASSERT(memory.data == NULL);
    TEST_ASSERT(memory.size == 0U);
    rv32_memory_destroy(&memory);
}

static void test_memory_little_endian_access(void)
{
    rv32_memory_t memory = {NULL, 0U};
    uint8_t byte = 0U;
    uint16_t half = 0U;
    uint32_t word = 0U;

    TEST_ASSERT_EQ_U32(RV32_OK, rv32_memory_init(&memory, 16U));
    TEST_ASSERT_EQ_U32(RV32_OK,
                       rv32_memory_write_u32(&memory, 4U,
                                             UINT32_C(0xA1B2C3D4)));
    TEST_ASSERT_EQ_U32(UINT8_C(0xD4), memory.data[4]);
    TEST_ASSERT_EQ_U32(UINT8_C(0xC3), memory.data[5]);
    TEST_ASSERT_EQ_U32(UINT8_C(0xB2), memory.data[6]);
    TEST_ASSERT_EQ_U32(UINT8_C(0xA1), memory.data[7]);
    TEST_ASSERT_EQ_U32(RV32_OK, rv32_memory_read_u32(&memory, 4U, &word));
    TEST_ASSERT_EQ_U32(UINT32_C(0xA1B2C3D4), word);

    TEST_ASSERT_EQ_U32(RV32_OK,
                       rv32_memory_write_u16(&memory, 8U,
                                             UINT16_C(0x5E6F)));
    TEST_ASSERT_EQ_U32(RV32_OK, rv32_memory_read_u16(&memory, 8U, &half));
    TEST_ASSERT_EQ_U32(UINT16_C(0x5E6F), half);
    TEST_ASSERT_EQ_U32(RV32_OK,
                       rv32_memory_write_u8(&memory, 10U, UINT8_C(0x7A)));
    TEST_ASSERT_EQ_U32(RV32_OK, rv32_memory_read_u8(&memory, 10U, &byte));
    TEST_ASSERT_EQ_U32(UINT8_C(0x7A), byte);

    rv32_memory_destroy(&memory);
}

static void test_memory_alignment_errors(void)
{
    rv32_memory_t memory = {NULL, 0U};
    uint16_t half = UINT16_C(0xCAFE);
    uint32_t word = UINT32_C(0xDEADBEEF);

    TEST_ASSERT_EQ_U32(RV32_OK, rv32_memory_init(&memory, 16U));
    TEST_ASSERT_EQ_U32(RV32_ERR_MEMORY_ALIGNMENT,
                       rv32_memory_write_u16(&memory, 1U, 0U));
    TEST_ASSERT_EQ_U32(RV32_ERR_MEMORY_ALIGNMENT,
                       rv32_memory_read_u16(&memory, 3U, &half));
    TEST_ASSERT_EQ_U32(UINT16_C(0xCAFE), half);
    TEST_ASSERT_EQ_U32(RV32_ERR_MEMORY_ALIGNMENT,
                       rv32_memory_write_u32(&memory, 2U, 0U));
    TEST_ASSERT_EQ_U32(RV32_ERR_MEMORY_ALIGNMENT,
                       rv32_memory_read_u32(&memory, 6U, &word));
    TEST_ASSERT_EQ_U32(UINT32_C(0xDEADBEEF), word);

    rv32_memory_destroy(&memory);
}

static void test_memory_bounds_errors(void)
{
    rv32_memory_t memory = {NULL, 0U};
    uint8_t byte = UINT8_C(0xAA);
    uint16_t half = UINT16_C(0xBBBB);
    uint32_t word = UINT32_C(0xCCCCCCCC);

    TEST_ASSERT_EQ_U32(RV32_OK, rv32_memory_init(&memory, 8U));
    TEST_ASSERT_EQ_U32(RV32_OK,
                       rv32_memory_write_u32(&memory, 4U,
                                             UINT32_C(0x12345678)));
    TEST_ASSERT_EQ_U32(RV32_ERR_MEMORY_BOUNDS,
                       rv32_memory_read_u8(&memory, 8U, &byte));
    TEST_ASSERT_EQ_U32(RV32_ERR_MEMORY_BOUNDS,
                       rv32_memory_read_u16(&memory, 8U, &half));
    TEST_ASSERT_EQ_U32(RV32_ERR_MEMORY_BOUNDS,
                       rv32_memory_read_u32(&memory, 8U, &word));
    TEST_ASSERT_EQ_U32(RV32_ERR_MEMORY_BOUNDS,
                       rv32_memory_write_u32(&memory, UINT32_MAX, 0U));
    TEST_ASSERT_EQ_U32(UINT8_C(0xAA), byte);
    TEST_ASSERT_EQ_U32(UINT16_C(0xBBBB), half);
    TEST_ASSERT_EQ_U32(UINT32_C(0xCCCCCCCC), word);

    rv32_memory_destroy(&memory);
}

static void test_memory_null_arguments(void)
{
    rv32_memory_t memory = {NULL, 0U};
    uint8_t byte = 0U;
    uint16_t half = 0U;
    uint32_t word = 0U;

    TEST_ASSERT_EQ_U32(RV32_ERR_INVALID_ARGUMENT,
                       rv32_memory_init(NULL, 8U));
    TEST_ASSERT_EQ_U32(RV32_ERR_INVALID_ARGUMENT,
                       rv32_memory_init(&memory, 0U));
    TEST_ASSERT_EQ_U32(RV32_ERR_INVALID_ARGUMENT,
                       rv32_memory_reset(NULL));
    TEST_ASSERT_EQ_U32(RV32_ERR_INVALID_ARGUMENT,
                       rv32_memory_reset(&memory));
    TEST_ASSERT_EQ_U32(RV32_ERR_INVALID_ARGUMENT,
                       rv32_memory_read_u8(NULL, 0U, &byte));
    TEST_ASSERT_EQ_U32(RV32_ERR_INVALID_ARGUMENT,
                       rv32_memory_read_u16(NULL, 0U, &half));
    TEST_ASSERT_EQ_U32(RV32_ERR_INVALID_ARGUMENT,
                       rv32_memory_read_u32(NULL, 0U, &word));
    TEST_ASSERT_EQ_U32(RV32_ERR_INVALID_ARGUMENT,
                       rv32_memory_write_u8(NULL, 0U, 0U));

    TEST_ASSERT_EQ_U32(RV32_OK, rv32_memory_init(&memory, 8U));
    TEST_ASSERT_EQ_U32(RV32_ERR_INVALID_ARGUMENT,
                       rv32_memory_read_u8(&memory, 0U, NULL));
    TEST_ASSERT_EQ_U32(RV32_ERR_INVALID_ARGUMENT,
                       rv32_memory_read_u16(&memory, 0U, NULL));
    TEST_ASSERT_EQ_U32(RV32_ERR_INVALID_ARGUMENT,
                       rv32_memory_read_u32(&memory, 0U, NULL));
    rv32_memory_destroy(NULL);
    rv32_memory_destroy(&memory);
}

void test_memory_register_tests(unsigned int *passed, unsigned int *total)
{
    if (passed == NULL || total == NULL) {
        return;
    }

    (*total)++;
    *passed += (unsigned int)test_run("memory init/reset/destroy",
                                     test_memory_init_reset_destroy);
    (*total)++;
    *passed += (unsigned int)test_run("memory little-endian access",
                                     test_memory_little_endian_access);
    (*total)++;
    *passed += (unsigned int)test_run("memory alignment errors",
                                     test_memory_alignment_errors);
    (*total)++;
    *passed += (unsigned int)test_run("memory bounds errors",
                                     test_memory_bounds_errors);
    (*total)++;
    *passed += (unsigned int)test_run("memory null arguments",
                                     test_memory_null_arguments);
}
