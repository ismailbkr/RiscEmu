#include "decoder.h"
#include "test.h"

#include <limits.h>
#include <stdint.h>

static void assert_i_immediate(uint32_t bits, int32_t expected)
{
    const rv32_decoded_instruction_t decoded = rv32_decode(bits << 20U);

    TEST_ASSERT(decoded.imm_i == expected);
}

static void assert_s_immediate(uint32_t bits, int32_t expected)
{
    const uint32_t instruction = ((bits & UINT32_C(0x0fe0)) << 20U)
                               | ((bits & UINT32_C(0x001f)) << 7U);
    const rv32_decoded_instruction_t decoded = rv32_decode(instruction);

    TEST_ASSERT(decoded.imm_s == expected);
}

static void assert_b_immediate(uint32_t bits, int32_t expected)
{
    const uint32_t instruction = ((bits & UINT32_C(0x1000)) << 19U)
                               | ((bits & UINT32_C(0x07e0)) << 20U)
                               | ((bits & UINT32_C(0x001e)) << 7U)
                               | ((bits & UINT32_C(0x0800)) >> 4U);
    const rv32_decoded_instruction_t decoded = rv32_decode(instruction);

    TEST_ASSERT(decoded.imm_b == expected);
}

static void assert_j_immediate(uint32_t bits, int32_t expected)
{
    const uint32_t instruction = ((bits & UINT32_C(0x100000)) << 11U)
                               | ((bits & UINT32_C(0x0007fe)) << 20U)
                               | ((bits & UINT32_C(0x000800)) << 9U)
                               | (bits & UINT32_C(0x0ff000));
    const rv32_decoded_instruction_t decoded = rv32_decode(instruction);

    TEST_ASSERT(decoded.imm_j == expected);
}

static void test_decoder_extracts_fields(void)
{
    const rv32_decoded_instruction_t decoded = rv32_decode(UINT32_C(0xfe20c9b3));

    TEST_ASSERT_EQ_U32(UINT32_C(0x33), decoded.opcode);
    TEST_ASSERT_EQ_U32(UINT32_C(19), decoded.rd);
    TEST_ASSERT_EQ_U32(UINT32_C(4), decoded.funct3);
    TEST_ASSERT_EQ_U32(UINT32_C(1), decoded.rs1);
    TEST_ASSERT_EQ_U32(UINT32_C(2), decoded.rs2);
    TEST_ASSERT_EQ_U32(UINT32_C(0x7f), decoded.funct7);
}

static void test_decoder_i_immediates(void)
{
    assert_i_immediate(UINT32_C(0x000), INT32_C(0));
    assert_i_immediate(UINT32_C(0x123), INT32_C(291));
    assert_i_immediate(UINT32_C(0xfff), INT32_C(-1));
    assert_i_immediate(UINT32_C(0x7ff), INT32_C(2047));
    assert_i_immediate(UINT32_C(0x800), INT32_C(-2048));
}

static void test_decoder_s_immediates(void)
{
    assert_s_immediate(UINT32_C(0x000), INT32_C(0));
    assert_s_immediate(UINT32_C(0x2a5), INT32_C(677));
    assert_s_immediate(UINT32_C(0xfff), INT32_C(-1));
    assert_s_immediate(UINT32_C(0x7ff), INT32_C(2047));
    assert_s_immediate(UINT32_C(0x800), INT32_C(-2048));
}

static void test_decoder_b_immediates(void)
{
    assert_b_immediate(UINT32_C(0x0000), INT32_C(0));
    assert_b_immediate(UINT32_C(0x0554), INT32_C(1364));
    assert_b_immediate(UINT32_C(0x1ffe), INT32_C(-2));
    assert_b_immediate(UINT32_C(0x0ffe), INT32_C(4094));
    assert_b_immediate(UINT32_C(0x1000), INT32_C(-4096));
}

static void test_decoder_u_immediates(void)
{
    TEST_ASSERT(rv32_decode(UINT32_C(0x00000000)).imm_u == INT32_C(0));
    TEST_ASSERT(rv32_decode(UINT32_C(0x12345fff)).imm_u
                == INT32_C(0x12345000));
    TEST_ASSERT(rv32_decode(UINT32_C(0x7fffffff)).imm_u
                == INT32_C(0x7ffff000));
    TEST_ASSERT(rv32_decode(UINT32_C(0x80000000)).imm_u == INT32_MIN);
    TEST_ASSERT(rv32_decode(UINT32_C(0xffffffff)).imm_u == INT32_C(-4096));
}

static void test_decoder_j_immediates(void)
{
    assert_j_immediate(UINT32_C(0x000000), INT32_C(0));
    assert_j_immediate(UINT32_C(0x055554), INT32_C(349524));
    assert_j_immediate(UINT32_C(0x1ffffe), INT32_C(-2));
    assert_j_immediate(UINT32_C(0x0ffffe), INT32_C(1048574));
    assert_j_immediate(UINT32_C(0x100000), INT32_C(-1048576));
}

void test_decoder_register_tests(unsigned int *passed, unsigned int *total)
{
    *total += 1U;
    *passed += (unsigned int)test_run("decoder extracts fields",
                                     test_decoder_extracts_fields);
    *total += 1U;
    *passed += (unsigned int)test_run("decoder I immediates",
                                     test_decoder_i_immediates);
    *total += 1U;
    *passed += (unsigned int)test_run("decoder S immediates",
                                     test_decoder_s_immediates);
    *total += 1U;
    *passed += (unsigned int)test_run("decoder B immediates",
                                     test_decoder_b_immediates);
    *total += 1U;
    *passed += (unsigned int)test_run("decoder U immediates",
                                     test_decoder_u_immediates);
    *total += 1U;
    *passed += (unsigned int)test_run("decoder J immediates",
                                     test_decoder_j_immediates);
}
