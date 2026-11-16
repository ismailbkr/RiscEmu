#include "cpu.h"
#include "test.h"

#include <stddef.h>
#include <string.h>

void test_instruction_register_tests(unsigned int *passed, unsigned int *total);

typedef struct {
    uint8_t bytes[512];
    rv32_memory_t memory;
    rv32_cpu_t cpu;
} fixture_t;

static void setup(fixture_t *f)
{
    memset(f, 0, sizeof(*f));
    f->memory.data = f->bytes;
    f->memory.size = sizeof(f->bytes);
    (void)rv32_cpu_init(&f->cpu, &f->memory, 32U);
}

static rv32_status_t execute_word(fixture_t *f, uint32_t word)
{
    const rv32_status_t status = rv32_memory_write_u32(&f->memory, f->cpu.pc, word);

    return status == RV32_OK ? rv32_cpu_step(&f->cpu) : status;
}

static uint32_t encode_i(uint32_t opcode, uint32_t funct3, uint32_t rd,
                         uint32_t rs1, int32_t immediate)
{
    return (((uint32_t)immediate & 0xfffU) << 20U) | (rs1 << 15U)
           | (funct3 << 12U) | (rd << 7U) | opcode;
}

static uint32_t encode_r(uint32_t funct3, uint32_t funct7, uint32_t rd)
{
    return (funct7 << 25U) | (2U << 20U) | (1U << 15U)
           | (funct3 << 12U) | (rd << 7U) | 0x33U;
}

static uint32_t encode_s(uint32_t funct3, int32_t immediate)
{
    const uint32_t bits = (uint32_t)immediate & 0xfffU;

    return ((bits & 0xfe0U) << 20U) | (2U << 20U) | (1U << 15U)
           | (funct3 << 12U) | ((bits & 0x1fU) << 7U) | 0x23U;
}

static uint32_t encode_b(uint32_t funct3, int32_t immediate)
{
    const uint32_t bits = (uint32_t)immediate & 0x1fffU;

    return ((bits & 0x1000U) << 19U) | ((bits & 0x7e0U) << 20U)
           | (2U << 20U) | (1U << 15U) | (funct3 << 12U)
           | ((bits & 0x1eU) << 7U) | ((bits & 0x800U) >> 4U) | 0x63U;
}

static uint32_t encode_j(uint32_t rd, int32_t immediate)
{
    const uint32_t bits = (uint32_t)immediate & 0x1fffffU;

    return ((bits & 0x100000U) << 11U) | ((bits & 0x7feU) << 20U)
           | ((bits & 0x800U) << 9U) | (bits & 0xff000U) | (rd << 7U) | 0x6fU;
}

static void test_register_alu(void)
{
    static const struct {
        uint32_t funct3, funct7, left, right, expected;
    } cases[] = {
        {0U, 0U, 7U, 5U, 12U},
        {0U, 0U, UINT32_MAX, 1U, 0U},
        {0U, 0U, 0x7fffffffU, 1U, 0x80000000U},
        {0U, 0U, 0x80000000U, 0x80000000U, 0U},
        {0U, 0x20U, 7U, 5U, 2U},
        {0U, 0x20U, 0U, 1U, UINT32_MAX},
        {0U, 0x20U, 0x80000000U, 1U, 0x7fffffffU},
        {1U, 0U, 1U, 31U, 0x80000000U},
        {1U, 0U, 3U, 32U, 3U},
        {1U, 0U, 3U, 33U, 6U},
        {1U, 0U, UINT32_MAX, UINT32_MAX, 0x80000000U},
        {2U, 0U, UINT32_MAX, 0U, 1U},
        {2U, 0U, 0x80000000U, 0x7fffffffU, 1U},
        {2U, 0U, 0x7fffffffU, 0x80000000U, 0U},
        {2U, 0U, UINT32_MAX, UINT32_MAX, 0U},
        {3U, 0U, UINT32_MAX, 0U, 0U},
        {3U, 0U, 0U, UINT32_MAX, 1U},
        {3U, 0U, 0x7fffffffU, 0x80000000U, 1U},
        {3U, 0U, 4U, 4U, 0U},
        {4U, 0U, 0xaaaaaaaaU, 0x55555555U, UINT32_MAX},
        {5U, 0U, 0x80000000U, 1U, 0x40000000U},
        {5U, 0U, UINT32_MAX, 31U, 1U},
        {5U, 0U, 0x80000000U, 32U, 0x80000000U},
        {5U, 0x20U, 0x80000000U, 1U, 0xc0000000U},
        {5U, 0x20U, 0x80000000U, 31U, UINT32_MAX},
        {5U, 0x20U, 0x80000000U, 32U, 0x80000000U},
        {5U, 0x20U, 0x80000000U, 63U, UINT32_MAX},
        {5U, 0x20U, 0x7fffffffU, 1U, 0x3fffffffU},
        {5U, 0x20U, UINT32_MAX, 0U, UINT32_MAX},
        {6U, 0U, 0x12340000U, 0x00005678U, 0x12345678U},
        {7U, 0U, 0x12345678U, 0x00ff00ffU, 0x00340078U}
    };
    size_t index;
    uint32_t rd;

    for (index = 0U; index < sizeof(cases) / sizeof(cases[0]); index++) {
        /* Also exercise rd==rs1, rd==rs2, and x0. */
        for (rd = 0U; rd <= 3U; rd++) {
            fixture_t f;
            setup(&f);
            f.cpu.regs[1] = cases[index].left;
            f.cpu.regs[2] = cases[index].right;
            TEST_ASSERT_EQ_U32(RV32_OK,
                execute_word(&f, encode_r(cases[index].funct3, cases[index].funct7, rd)));
            TEST_ASSERT_EQ_U32(rd == 0U ? 0U : cases[index].expected, f.cpu.regs[rd]);
            TEST_ASSERT_EQ_U32(36U, f.cpu.pc);
            TEST_ASSERT(f.cpu.instruction_count == 1U);
            TEST_ASSERT(!f.cpu.halted);
        }
    }
}

static void test_immediate_alu(void)
{
    static const struct {
        uint32_t funct3, left;
        int32_t immediate;
        uint32_t expected;
    } cases[] = {
        {0U, 5U, -1, 4U},
        {0U, 0U, -2048, 0xfffff800U},
        {0U, UINT32_MAX, 1, 0U},
        {0U, 0x7fffffffU, 1, 0x80000000U},
        {0U, 1U, 2047, 2048U},
        {2U, 0x80000000U, -2048, 1U},
        {2U, 0x7fffffffU, -1, 0U},
        {2U, UINT32_MAX, -1, 0U},
        {2U, UINT32_MAX, 0, 1U},
        {3U, 0U, -1, 1U},
        {3U, UINT32_MAX, -1, 0U},
        {3U, 0xfffffffeU, -1, 1U},
        {3U, 0x80000000U, 1, 0U},
        {4U, 0x12345678U, -1, 0xedcba987U},
        {4U, 0x12345678U, 0, 0x12345678U},
        {6U, 0x12345678U, -2048, 0xfffffe78U},
        {6U, 0x12345000U, 0x678, 0x12345678U},
        {7U, 0x12345678U, -2048, 0x12345000U},
        {7U, 0x12345678U, 0x7ff, 0x678U},
        {1U, 1U, 31, 0x80000000U},
        {1U, 0x12345678U, 0, 0x12345678U},
        {1U, UINT32_MAX, 1, 0xfffffffeU},
        {5U, 0x80000000U, 1, 0x40000000U},
        {5U, UINT32_MAX, 31, 1U},
        {5U, 0x80000000U, 0, 0x80000000U},
        {5U, 0x80000000U, 0x401, 0xc0000000U},
        {5U, 0x80000000U, 0x41f, UINT32_MAX},
        {5U, 0x80000000U, 0x400, 0x80000000U},
        {5U, 0x7fffffffU, 0x401, 0x3fffffffU}
    };
    size_t index;
    uint32_t rd;

    for (index = 0U; index < sizeof(cases) / sizeof(cases[0]); index++) {
        for (rd = 0U; rd < 3U; rd++) {
            fixture_t f;
            setup(&f);
            f.cpu.regs[1] = cases[index].left;
            TEST_ASSERT_EQ_U32(RV32_OK, execute_word(&f,
                encode_i(0x13U, cases[index].funct3, rd, 1U, cases[index].immediate)));
            TEST_ASSERT_EQ_U32(rd == 0U ? 0U : cases[index].expected, f.cpu.regs[rd]);
            TEST_ASSERT_EQ_U32(36U, f.cpu.pc);
        }
    }
}

static void test_upper_immediates(void)
{
    static const struct { uint32_t word, expected; } cases[] = {
        {0x123451b7U, 0x12345000U}, /* lui x3,0x12345 */
        {0x800001b7U, 0x80000000U},
        {0xfffff1b7U, 0xfffff000U},
        {0x000001b7U, 0U},
        {0x12345197U, 0x12345020U}, /* auipc x3,0x12345 at PC 32 */
        {0x80000197U, 0x80000020U},
        {0xfffff197U, 0xfffff020U},
        {0x00000197U, 32U}
    };
    size_t index;

    for (index = 0U; index < sizeof(cases) / sizeof(cases[0]); index++) {
        fixture_t f;
        setup(&f);
        TEST_ASSERT_EQ_U32(RV32_OK, execute_word(&f, cases[index].word));
        TEST_ASSERT_EQ_U32(cases[index].expected, f.cpu.regs[3]);
        TEST_ASSERT_EQ_U32(36U, f.cpu.pc);
    }
}

static void test_jumps(void)
{
    static const int32_t offsets[] = {0, 12, -16, 1048572, -1048576};
    size_t index;
    fixture_t f;

    for (index = 0U; index < sizeof(offsets) / sizeof(offsets[0]); index++) {
        setup(&f);
        TEST_ASSERT_EQ_U32(RV32_OK, execute_word(&f, encode_j(3U, offsets[index])));
        TEST_ASSERT_EQ_U32(32U + (uint32_t)offsets[index], f.cpu.pc);
        TEST_ASSERT_EQ_U32(36U, f.cpu.regs[3]);
        TEST_ASSERT(f.cpu.instruction_count == 1U);
    }
    setup(&f);
    TEST_ASSERT_EQ_U32(RV32_OK, execute_word(&f, encode_j(0U, -4)));
    TEST_ASSERT_EQ_U32(28U, f.cpu.pc);
    TEST_ASSERT_EQ_U32(0U, f.cpu.regs[0]);
    setup(&f);
    f.cpu.regs[1] = 65U;
    TEST_ASSERT_EQ_U32(RV32_OK, execute_word(&f, encode_i(0x67U, 0U, 1U, 1U, -4)));
    TEST_ASSERT_EQ_U32(60U, f.cpu.pc); /* Clear bit zero, rd aliases rs1. */
    TEST_ASSERT_EQ_U32(36U, f.cpu.regs[1]);
    setup(&f);
    f.cpu.regs[1] = UINT32_MAX;
    TEST_ASSERT_EQ_U32(RV32_OK, execute_word(&f, encode_i(0x67U, 0U, 0U, 1U, 2)));
    TEST_ASSERT_EQ_U32(0U, f.cpu.pc); /* Effective address wraps before masking. */
    TEST_ASSERT_EQ_U32(0U, f.cpu.regs[0]);
    setup(&f);
    TEST_ASSERT_EQ_U32(RV32_OK, execute_word(&f, encode_j(3U, 512)));
    TEST_ASSERT_EQ_U32(RV32_ERR_MEMORY_BOUNDS, rv32_cpu_step(&f.cpu));
    TEST_ASSERT_EQ_U32(544U, f.cpu.pc); /* Target bounds fault belongs to fetch. */
    TEST_ASSERT_EQ_U32(36U, f.cpu.regs[3]);
}

static void test_target_alignment(void)
{
    static const uint32_t words[] = {
        0x002001efU, /* jal x3,+2 */
        0x002081e7U, /* jalr x3,2(x1), x1=0 */
        0x003081e7U, /* jalr x3,3(x1): bit zero clearing is insufficient */
        0x00208163U  /* beq x1,x2,+2 */
    };
    size_t index;

    for (index = 0U; index < sizeof(words) / sizeof(words[0]); index++) {
        fixture_t f;
        setup(&f);
        f.cpu.regs[3] = 0xabcdef01U;
        TEST_ASSERT_EQ_U32(RV32_ERR_MEMORY_ALIGNMENT, execute_word(&f, words[index]));
        TEST_ASSERT_EQ_U32(32U, f.cpu.pc);
        TEST_ASSERT_EQ_U32(0xabcdef01U, f.cpu.regs[3]);
        TEST_ASSERT_EQ_U32(words[index], f.cpu.last_instruction);
        TEST_ASSERT(f.cpu.instruction_count == 0U);
        TEST_ASSERT(f.cpu.halted);
    }
}

static void test_branches(void)
{
    static const struct { uint32_t funct3, left, right; bool taken; } cases[] = {
        {0U, 7U, 7U, true}, {0U, 7U, 8U, false},
        {1U, 7U, 8U, true}, {1U, 7U, 7U, false},
        {4U, 0x80000000U, 0x7fffffffU, true},
        {4U, 0x7fffffffU, 0x80000000U, false},
        {4U, UINT32_MAX, UINT32_MAX, false},
        {5U, 0x7fffffffU, 0x80000000U, true},
        {5U, 0x80000000U, 0x7fffffffU, false},
        {5U, UINT32_MAX, UINT32_MAX, true},
        {6U, 0x7fffffffU, 0x80000000U, true},
        {6U, UINT32_MAX, 0U, false}, {6U, 5U, 5U, false},
        {7U, UINT32_MAX, 0U, true}, {7U, 0U, UINT32_MAX, false},
        {7U, 5U, 5U, true}
    };
    static const int32_t offsets[] = {12, -16, 0, -4096, 4092, 2};
    size_t index, offset;

    for (index = 0U; index < sizeof(cases) / sizeof(cases[0]); index++) {
        for (offset = 0U; offset < sizeof(offsets) / sizeof(offsets[0]); offset++) {
            fixture_t f;
            const bool fault = cases[index].taken && offsets[offset] == 2;
            setup(&f);
            f.cpu.regs[1] = cases[index].left;
            f.cpu.regs[2] = cases[index].right;
            TEST_ASSERT_EQ_U32(fault ? RV32_ERR_MEMORY_ALIGNMENT : RV32_OK,
                execute_word(&f, encode_b(cases[index].funct3, offsets[offset])));
            TEST_ASSERT_EQ_U32(fault ? 32U : (cases[index].taken
                ? 32U + (uint32_t)offsets[offset] : 36U), f.cpu.pc);
            TEST_ASSERT_EQ_U32(cases[index].left, f.cpu.regs[1]);
            TEST_ASSERT_EQ_U32(cases[index].right, f.cpu.regs[2]);
            TEST_ASSERT(f.cpu.instruction_count == (fault ? 0U : 1U));
        }
    }
}

static void test_loads(void)
{
    static const struct { uint32_t funct3, stored, expected; } cases[] = {
        {0U, 0x80U, 0xffffff80U}, {0U, 0xffU, UINT32_MAX},
        {0U, 0x7fU, 0x7fU}, {0U, 0U, 0U},
        {4U, 0x80U, 0x80U}, {4U, 0xffU, 0xffU},
        {1U, 0x8000U, 0xffff8000U}, {1U, 0xffffU, UINT32_MAX},
        {1U, 0x7fffU, 0x7fffU}, {1U, 0U, 0U},
        {5U, 0x8000U, 0x8000U}, {5U, 0xffffU, 0xffffU},
        {2U, 0x89abcdefU, 0x89abcdefU}, {2U, UINT32_MAX, UINT32_MAX}
    };
    size_t index;
    uint32_t rd;

    for (index = 0U; index < sizeof(cases) / sizeof(cases[0]); index++) {
        for (rd = 0U; rd <= 3U; rd++) {
            fixture_t f;
            setup(&f);
            f.cpu.regs[1] = 132U;
            TEST_ASSERT_EQ_U32(RV32_OK,
                rv32_memory_write_u32(&f.memory, 128U, cases[index].stored));
            TEST_ASSERT_EQ_U32(RV32_OK, execute_word(&f,
                encode_i(0x03U, cases[index].funct3, rd, 1U, -4)));
            TEST_ASSERT_EQ_U32(rd == 0U ? 0U : cases[index].expected, f.cpu.regs[rd]);
            TEST_ASSERT_EQ_U32(36U, f.cpu.pc);
        }
    }
    {
        fixture_t f;
        setup(&f);
        f.cpu.regs[1] = UINT32_MAX;
        f.bytes[0] = 0xfeU;
        TEST_ASSERT_EQ_U32(RV32_OK,
            execute_word(&f, encode_i(0x03U, 0U, 3U, 1U, 1)));
        TEST_ASSERT_EQ_U32(0xfffffffeU, f.cpu.regs[3]);
    }
}

static void test_stores(void)
{
    uint32_t funct3;

    for (funct3 = 0U; funct3 <= 2U; funct3++) {
        fixture_t f;
        const size_t width = (size_t)1U << funct3;
        static const uint8_t expected[] = {0xefU, 0xcdU, 0xabU, 0x89U};
        size_t index;
        setup(&f);
        memset(f.bytes + 120U, 0xa5, 20U);
        f.cpu.regs[1] = 132U;
        f.cpu.regs[2] = 0x89abcdefU;
        TEST_ASSERT_EQ_U32(RV32_OK, execute_word(&f, encode_s(funct3, -4)));
        for (index = 120U; index < 140U; index++) {
            TEST_ASSERT_EQ_U32(index >= 128U && index < 128U + width
                ? expected[index - 128U] : 0xa5U, f.bytes[index]);
        }
        TEST_ASSERT_EQ_U32(132U, f.cpu.regs[1]);
        TEST_ASSERT_EQ_U32(0x89abcdefU, f.cpu.regs[2]);
        TEST_ASSERT_EQ_U32(36U, f.cpu.pc);
        TEST_ASSERT(f.cpu.instruction_count == 1U);
    }
    {
        fixture_t f;
        setup(&f);
        f.cpu.regs[1] = UINT32_MAX;
        f.cpu.regs[2] = 0x12345678U;
        TEST_ASSERT_EQ_U32(RV32_OK, execute_word(&f, encode_s(2U, 1)));
        TEST_ASSERT_EQ_U32(0x78U, f.bytes[0]);
        TEST_ASSERT_EQ_U32(0x12U, f.bytes[3]);
    }
}

static void test_memory_faults_are_atomic(void)
{
    static const struct {
        uint32_t funct3, address;
        rv32_status_t status;
    } cases[] = {
        {0U, 512U, RV32_ERR_MEMORY_BOUNDS},
        {1U, 513U, RV32_ERR_MEMORY_BOUNDS},
        {2U, 512U, RV32_ERR_MEMORY_BOUNDS},
        {1U, 129U, RV32_ERR_MEMORY_ALIGNMENT},
        {2U, 130U, RV32_ERR_MEMORY_ALIGNMENT},
        {0U, UINT32_MAX, RV32_ERR_MEMORY_BOUNDS},
        {2U, 0xfffffffcU, RV32_ERR_MEMORY_BOUNDS}
    };
    size_t index;
    unsigned int variant;

    for (index = 0U; index < sizeof(cases) / sizeof(cases[0]); index++) {
        for (variant = 0U; variant < 4U; variant++) {
            fixture_t f;
            uint8_t before[512];
            uint32_t word;
            setup(&f);
            f.cpu.regs[1] = cases[index].address;
            f.cpu.regs[2] = 0xabcdef01U;
            f.cpu.regs[3] = 0x12345678U;
            word = variant == 0U ? encode_s(cases[index].funct3, 0)
                : encode_i(0x03U, cases[index].funct3
                    + (variant == 3U && cases[index].funct3 < 2U ? 4U : 0U),
                    variant == 2U ? 0U : 3U, 1U, 0);
            TEST_ASSERT_EQ_U32(RV32_OK, rv32_memory_write_u32(&f.memory, 32U, word));
            memcpy(before, f.bytes, sizeof(before));
            TEST_ASSERT_EQ_U32(cases[index].status, rv32_cpu_step(&f.cpu));
            TEST_ASSERT_EQ_U32(32U, f.cpu.pc);
            TEST_ASSERT_EQ_U32(0U, f.cpu.regs[0]);
            TEST_ASSERT_EQ_U32(0x12345678U, f.cpu.regs[3]);
            TEST_ASSERT_EQ_U32(0xabcdef01U, f.cpu.regs[2]);
            TEST_ASSERT_EQ_U32(word, f.cpu.last_instruction);
            TEST_ASSERT_EQ_U32(cases[index].status, f.cpu.last_status);
            TEST_ASSERT(f.cpu.instruction_count == 0U);
            TEST_ASSERT(f.cpu.halted);
            TEST_ASSERT(memcmp(before, f.bytes, sizeof(before)) == 0);
        }
    }
}

static void assert_invalid(uint32_t word)
{
    fixture_t f;
    uint32_t registers[32];
    uint8_t before[512];
    unsigned int index;

    setup(&f);
    for (index = 1U; index < 32U; index++) {
        f.cpu.regs[index] = 128U;
    }
    memcpy(registers, f.cpu.regs, sizeof(registers));
    TEST_ASSERT_EQ_U32(RV32_OK, rv32_memory_write_u32(&f.memory, 32U, word));
    memcpy(before, f.bytes, sizeof(before));
    TEST_ASSERT_EQ_U32(RV32_ERR_INVALID_INSTRUCTION, rv32_cpu_step(&f.cpu));
    TEST_ASSERT_EQ_U32(32U, f.cpu.pc);
    TEST_ASSERT_EQ_U32(word, f.cpu.last_instruction);
    TEST_ASSERT_EQ_U32(RV32_ERR_INVALID_INSTRUCTION, f.cpu.last_status);
    TEST_ASSERT(f.cpu.halted);
    TEST_ASSERT(f.cpu.instruction_count == 0U);
    TEST_ASSERT(memcmp(registers, f.cpu.regs, sizeof(registers)) == 0);
    TEST_ASSERT(memcmp(before, f.bytes, sizeof(before)) == 0);
}

static void test_illegal_alu_encodings(void)
{
    uint32_t funct3, funct7;

    for (funct3 = 0U; funct3 < 8U; funct3++) {
        for (funct7 = 0U; funct7 < 128U; funct7++) {
            const bool valid = funct7 == 0U
                || (funct7 == 0x20U && (funct3 == 0U || funct3 == 5U));
            if (!valid) {
                assert_invalid(encode_r(funct3, funct7, 3U));
            }
        }
    }
    for (funct7 = 1U; funct7 < 128U; funct7++) {
        assert_invalid(encode_i(0x13U, 1U, 3U, 1U, (int32_t)(funct7 << 5U)));
        if (funct7 != 0x20U) {
            assert_invalid(encode_i(0x13U, 5U, 3U, 1U, (int32_t)(funct7 << 5U)));
        }
    }
}

static void test_illegal_other_encodings(void)
{
    static const uint32_t words[] = {
        0U, UINT32_MAX, 0x00000001U, /* zero, unknown, compressed */
        0x30200073U, 0x10500073U, /* MRET/WFI */
        0x000000f3U, 0x00008073U, /* ECALL with nonzero rd or rs1 */
        0x00200073U, 0x00001073U, 0x00002073U, 0x00003073U,
        0x00004073U, 0x00005073U, 0x00006073U, 0x00007073U
    };
    size_t index;
    uint32_t funct3;

    for (index = 0U; index < sizeof(words) / sizeof(words[0]); index++) {
        assert_invalid(words[index]);
    }
    for (funct3 = 0U; funct3 < 8U; funct3++) {
        if (funct3 == 3U || funct3 >= 6U) {
            assert_invalid(encode_i(0x03U, funct3, 3U, 1U, 0));
        }
        if (funct3 >= 3U) {
            assert_invalid(encode_s(funct3, 0));
        }
        if (funct3 == 2U || funct3 == 3U) {
            assert_invalid(encode_b(funct3, 4));
        }
        if (funct3 != 0U) {
            assert_invalid(encode_i(0x67U, funct3, 3U, 1U, 0));
            assert_invalid((funct3 << 12U) | 0x0fU);
        }
    }
}

static void test_fence_and_ecall(void)
{
    static const uint32_t fences[] = {
        0x0000000fU, 0x0ff0000fU, 0x8330000fU, 0xffff8f8fU
    };
    size_t index;

    for (index = 0U; index < sizeof(fences) / sizeof(fences[0]); index++) {
        fixture_t f;
        uint32_t registers[32];
        setup(&f);
        f.cpu.regs[31] = 123U;
        memcpy(registers, f.cpu.regs, sizeof(registers));
        TEST_ASSERT_EQ_U32(RV32_OK, execute_word(&f, fences[index]));
        TEST_ASSERT_EQ_U32(36U, f.cpu.pc);
        TEST_ASSERT(f.cpu.instruction_count == 1U);
        TEST_ASSERT(memcmp(registers, f.cpu.regs, sizeof(registers)) == 0);
        TEST_ASSERT_EQ_U32(RV32_ERR_HALTED, execute_word(&f, 0x00000073U));
        TEST_ASSERT_EQ_U32(36U, f.cpu.pc);
        TEST_ASSERT(f.cpu.instruction_count == 1U);
        TEST_ASSERT(f.cpu.halted);
        TEST_ASSERT(memcmp(registers, f.cpu.regs, sizeof(registers)) == 0);
    }

    {
        fixture_t f;
        setup(&f);
        TEST_ASSERT_EQ_U32(RV32_ERR_HALTED, execute_word(&f, 0x00100073U));
        TEST_ASSERT_EQ_U32(32U, f.cpu.pc);
        TEST_ASSERT(f.cpu.instruction_count == 0U);
        TEST_ASSERT(f.cpu.halted);
    }
}

static void test_raw_sum_program(void)
{
    /* Independent literal encodings: sum 1..10 into x1, store to RAM[256]. */
    static const uint32_t program[] = {
        0x00000093U, /* addi x1,x0,0 */
        0x00a00113U, /* addi x2,x0,10 */
        0x002080b3U, /* add x1,x1,x2 */
        0xfff10113U, /* addi x2,x2,-1 */
        0xfe011ce3U, /* bne x2,x0,-8 */
        0x10102023U, /* sw x1,256(x0) */
        0x00000073U
    };
    fixture_t f;
    size_t index;
    uint32_t result;

    setup(&f);
    TEST_ASSERT_EQ_U32(RV32_OK, rv32_cpu_reset(&f.cpu, 0U));
    for (index = 0U; index < sizeof(program) / sizeof(program[0]); index++) {
        TEST_ASSERT_EQ_U32(RV32_OK,
            rv32_memory_write_u32(&f.memory, (uint32_t)index * 4U, program[index]));
    }
    TEST_ASSERT_EQ_U32(RV32_ERR_HALTED, rv32_cpu_run(&f.cpu, 100U));
    TEST_ASSERT_EQ_U32(55U, f.cpu.regs[1]);
    TEST_ASSERT_EQ_U32(0U, f.cpu.regs[2]);
    TEST_ASSERT_EQ_U32(RV32_OK, rv32_memory_read_u32(&f.memory, 256U, &result));
    TEST_ASSERT_EQ_U32(55U, result);
    TEST_ASSERT_EQ_U32(24U, f.cpu.pc);
    TEST_ASSERT(f.cpu.instruction_count == 33U);
}

static void test_raw_call_program(void)
{
    /* Call a subroutine, store/load a signed byte, and return via ra. */
    static const uint32_t program[] = {
        0xfff00513U, /*  0: addi a0,x0,-1 */
        0x00c000efU, /*  4: jal ra,+12 -> 16 */
        0x00000073U, /*  8: ecall */
        0x00000000U, /* 12: unreachable */
        0x10a000a3U, /* 16: sb a0,257(x0) */
        0x10104583U, /* 20: lbu a1,257(x0) */
        0x10100603U, /* 24: lb a2,257(x0) */
        0x00008067U  /* 28: jalr x0,0(ra) */
    };
    fixture_t f;
    size_t index;

    setup(&f);
    TEST_ASSERT_EQ_U32(RV32_OK, rv32_cpu_reset(&f.cpu, 0U));
    for (index = 0U; index < sizeof(program) / sizeof(program[0]); index++) {
        TEST_ASSERT_EQ_U32(RV32_OK,
            rv32_memory_write_u32(&f.memory, (uint32_t)index * 4U, program[index]));
    }
    TEST_ASSERT_EQ_U32(RV32_ERR_HALTED, rv32_cpu_run(&f.cpu, 100U));
    TEST_ASSERT_EQ_U32(8U, f.cpu.regs[1]);
    TEST_ASSERT_EQ_U32(255U, f.cpu.regs[11]);
    TEST_ASSERT_EQ_U32(UINT32_MAX, f.cpu.regs[12]);
    TEST_ASSERT_EQ_U32(0xffU, f.bytes[257]);
    TEST_ASSERT_EQ_U32(8U, f.cpu.pc);
    TEST_ASSERT_EQ_U32(0U, f.cpu.regs[0]);
    TEST_ASSERT(f.cpu.instruction_count == 6U);
}

void test_instruction_register_tests(unsigned int *passed, unsigned int *total)
{
    static const struct {
        const char *name;
        test_function_t function;
    } cases[] = {
        {"register ALU and aliasing", test_register_alu},
        {"immediate ALU and boundaries", test_immediate_alu},
        {"LUI/AUIPC", test_upper_immediates},
        {"JAL/JALR", test_jumps},
        {"jump/branch target alignment", test_target_alignment},
        {"all branches taken/not taken", test_branches},
        {"signed/unsigned loads", test_loads},
        {"stores and neighboring bytes", test_stores},
        {"memory fault atomicity including x0 loads", test_memory_faults_are_atomic},
        {"illegal ALU funct encodings", test_illegal_alu_encodings},
        {"illegal other encodings", test_illegal_other_encodings},
        {"FENCE/ECALL", test_fence_and_ecall},
        {"raw sum loop", test_raw_sum_program},
        {"raw subroutine and load/store", test_raw_call_program}
    };
    size_t index;

    if (passed == NULL || total == NULL) {
        return;
    }
    for (index = 0U; index < sizeof(cases) / sizeof(cases[0]); index++) {
        (*total)++;
        *passed += (unsigned int)test_run(cases[index].name, cases[index].function);
    }
}
