#include "cpu.h"
#include "test.h"

#include <limits.h>
#include <stddef.h>

void test_cpu_register_tests(unsigned int *passed, unsigned int *total);

static void test_cpu_init_reset(void)
{
    uint8_t bytes[32] = {0U};
    rv32_memory_t memory = {bytes, sizeof(bytes)};
    rv32_cpu_t cpu;
    unsigned int index;

    TEST_ASSERT_EQ_U32(RV32_OK, rv32_cpu_init(&cpu, &memory, 4U));
    TEST_ASSERT(cpu.memory == &memory);
    TEST_ASSERT_EQ_U32(4U, cpu.pc);
    TEST_ASSERT(!cpu.halted);
    TEST_ASSERT_EQ_U32(RV32_OK, cpu.last_status);
    TEST_ASSERT_EQ_U32(0U, cpu.last_instruction);
    TEST_ASSERT(cpu.instruction_count == 0U);
    for (index = 0U; index < 32U; index++) {
        TEST_ASSERT_EQ_U32(0U, cpu.regs[index]);
        TEST_ASSERT_EQ_U32(RV32_OK,
                           rv32_cpu_write_register(&cpu, index, index + 7U));
    }
    bytes[20] = 0xa5U;
    cpu.halted = true;
    cpu.last_status = RV32_ERR_INVALID_INSTRUCTION;
    cpu.last_instruction = UINT32_MAX;
    cpu.instruction_count = UINT64_C(5000000000);
    TEST_ASSERT_EQ_U32(RV32_OK, rv32_cpu_reset(&cpu, 8U));
    TEST_ASSERT(cpu.memory == &memory);
    TEST_ASSERT_EQ_U32(8U, cpu.pc);
    TEST_ASSERT_EQ_U32(0xa5U, bytes[20]);
    TEST_ASSERT(!cpu.halted);
    TEST_ASSERT_EQ_U32(RV32_OK, cpu.last_status);
    TEST_ASSERT_EQ_U32(0U, cpu.last_instruction);
    TEST_ASSERT(cpu.instruction_count == 0U);
    for (index = 0U; index < 32U; index++) {
        TEST_ASSERT_EQ_U32(0U, cpu.regs[index]);
    }
}

static void test_cpu_register_access(void)
{
    uint8_t bytes[16] = {0U};
    rv32_memory_t memory = {bytes, sizeof(bytes)};
    rv32_cpu_t cpu;
    unsigned int index;

    TEST_ASSERT_EQ_U32(RV32_OK, rv32_cpu_init(&cpu, &memory, 0U));
    for (index = 1U; index < 32U; index++) {
        const uint32_t value = UINT32_MAX - index;
        TEST_ASSERT_EQ_U32(RV32_OK,
                           rv32_cpu_write_register(&cpu, index, value));
        TEST_ASSERT_EQ_U32(value, rv32_cpu_read_register(&cpu, index));
    }
    TEST_ASSERT_EQ_U32(RV32_OK, rv32_cpu_write_register(&cpu, 0U, UINT32_MAX));
    TEST_ASSERT_EQ_U32(0U, cpu.regs[0]);
    cpu.regs[0] = 99U; /* Even direct accidental corruption must not be read. */
    TEST_ASSERT_EQ_U32(0U, rv32_cpu_read_register(&cpu, 0U));
    TEST_ASSERT_EQ_U32(RV32_OK,
                       rv32_memory_write_u32(&memory, 0U, UINT32_C(0x00100093)));
    TEST_ASSERT_EQ_U32(RV32_OK, rv32_cpu_step(&cpu)); /* addi x1,x0,1 */
    TEST_ASSERT_EQ_U32(1U, cpu.regs[1]);
    TEST_ASSERT_EQ_U32(0U, cpu.regs[0]);
    TEST_ASSERT_EQ_U32(RV32_OK,
                       rv32_memory_write_u32(&memory, 4U, UINT32_C(0x00708013)));
    TEST_ASSERT_EQ_U32(RV32_OK, rv32_cpu_step(&cpu)); /* addi x0,x1,7 */
    TEST_ASSERT_EQ_U32(0U, cpu.regs[0]);
    TEST_ASSERT_EQ_U32(0U, rv32_cpu_read_register(&cpu, 32U));
    TEST_ASSERT_EQ_U32(0U, rv32_cpu_read_register(&cpu, UINT_MAX));
    TEST_ASSERT_EQ_U32(RV32_ERR_INVALID_ARGUMENT,
                       rv32_cpu_write_register(&cpu, 32U, 12U));
    TEST_ASSERT_EQ_U32(RV32_ERR_INVALID_ARGUMENT,
                       rv32_cpu_write_register(&cpu, UINT_MAX, 12U));
    TEST_ASSERT_EQ_U32(1U, cpu.regs[1]);
}

static void test_cpu_invalid_arguments(void)
{
    uint8_t bytes[8] = {0U};
    rv32_memory_t memory = {bytes, sizeof(bytes)};
    rv32_memory_t invalid = {NULL, 0U};
    rv32_cpu_t cpu;

    TEST_ASSERT_EQ_U32(RV32_ERR_INVALID_ARGUMENT, rv32_cpu_init(NULL, &memory, 0U));
    TEST_ASSERT_EQ_U32(RV32_ERR_INVALID_ARGUMENT, rv32_cpu_reset(NULL, 0U));
    TEST_ASSERT_EQ_U32(RV32_ERR_INVALID_ARGUMENT, rv32_cpu_step(NULL));
    TEST_ASSERT_EQ_U32(RV32_ERR_INVALID_ARGUMENT, rv32_cpu_run(NULL, 0U));
    TEST_ASSERT_EQ_U32(RV32_ERR_INVALID_ARGUMENT,
                       rv32_cpu_write_register(NULL, 1U, 2U));
    TEST_ASSERT_EQ_U32(0U, rv32_cpu_read_register(NULL, 1U));
    TEST_ASSERT_EQ_U32(RV32_ERR_INVALID_ARGUMENT, rv32_cpu_init(&cpu, NULL, 0U));
    TEST_ASSERT(cpu.halted);
    TEST_ASSERT_EQ_U32(RV32_ERR_INVALID_ARGUMENT, cpu.last_status);
    TEST_ASSERT_EQ_U32(RV32_ERR_INVALID_ARGUMENT, rv32_cpu_init(&cpu, &invalid, 0U));
    invalid.data = bytes;
    TEST_ASSERT_EQ_U32(RV32_ERR_INVALID_ARGUMENT, rv32_cpu_init(&cpu, &invalid, 0U));
    TEST_ASSERT_EQ_U32(RV32_OK, rv32_cpu_init(&cpu, &memory, 0U));
    cpu.memory = NULL;
    TEST_ASSERT_EQ_U32(RV32_ERR_INVALID_ARGUMENT, rv32_cpu_step(&cpu));
    TEST_ASSERT(cpu.halted);
    TEST_ASSERT_EQ_U32(RV32_OK, rv32_cpu_init(&cpu, &memory, 0U));
    cpu.memory = NULL;
    TEST_ASSERT_EQ_U32(RV32_ERR_INVALID_ARGUMENT, rv32_cpu_run(&cpu, 0U));
    TEST_ASSERT(cpu.halted);
}

static void test_cpu_fetch_faults(void)
{
    uint8_t bytes[8] = {0U};
    rv32_memory_t memory = {bytes, sizeof(bytes)};
    rv32_cpu_t cpu;

    TEST_ASSERT_EQ_U32(RV32_ERR_MEMORY_ALIGNMENT,
                       rv32_cpu_init(&cpu, &memory, 2U));
    TEST_ASSERT(cpu.halted);
    TEST_ASSERT_EQ_U32(2U, cpu.pc);
    TEST_ASSERT_EQ_U32(RV32_OK, rv32_cpu_reset(&cpu, 0U));
    cpu.pc = 3U;
    cpu.last_instruction = UINT32_MAX;
    TEST_ASSERT_EQ_U32(RV32_ERR_MEMORY_ALIGNMENT, rv32_cpu_step(&cpu));
    TEST_ASSERT_EQ_U32(3U, cpu.pc);
    TEST_ASSERT_EQ_U32(0U, cpu.last_instruction);
    TEST_ASSERT(cpu.instruction_count == 0U);
    TEST_ASSERT_EQ_U32(RV32_OK, rv32_cpu_reset(&cpu, 8U));
    TEST_ASSERT_EQ_U32(RV32_ERR_MEMORY_BOUNDS, rv32_cpu_step(&cpu));
    TEST_ASSERT_EQ_U32(8U, cpu.pc);
    TEST_ASSERT(cpu.halted);
    TEST_ASSERT_EQ_U32(RV32_ERR_MEMORY_BOUNDS, cpu.last_status);
    TEST_ASSERT_EQ_U32(RV32_OK, rv32_cpu_reset(&cpu, UINT32_C(0xfffffffc)));
    TEST_ASSERT_EQ_U32(RV32_ERR_MEMORY_BOUNDS, rv32_cpu_step(&cpu));
    memory.size = 6U; /* Truncated final instruction. */
    TEST_ASSERT_EQ_U32(RV32_OK, rv32_cpu_reset(&cpu, 4U));
    TEST_ASSERT_EQ_U32(RV32_ERR_MEMORY_BOUNDS, rv32_cpu_step(&cpu));
}

static void test_cpu_halt_and_diagnostics(void)
{
    uint8_t bytes[16] = {0U};
    rv32_memory_t memory = {bytes, sizeof(bytes)};
    rv32_cpu_t cpu;

    TEST_ASSERT_EQ_U32(RV32_OK, rv32_cpu_init(&cpu, &memory, 0U));
    TEST_ASSERT_EQ_U32(RV32_OK,
                       rv32_memory_write_u32(&memory, 0U, UINT32_C(0x00500093)));
    TEST_ASSERT_EQ_U32(RV32_OK,
                       rv32_memory_write_u32(&memory, 4U, UINT32_C(0x00000073)));
    cpu.instruction_count = UINT64_C(0xffffffff);
    TEST_ASSERT_EQ_U32(RV32_ERR_HALTED, rv32_cpu_run(&cpu, 10U));
    TEST_ASSERT(cpu.instruction_count == UINT64_C(0x100000000));
    TEST_ASSERT_EQ_U32(5U, cpu.regs[1]);
    TEST_ASSERT_EQ_U32(4U, cpu.pc);
    TEST_ASSERT_EQ_U32(0x73U, cpu.last_instruction);
    TEST_ASSERT_EQ_U32(RV32_ERR_HALTED, cpu.last_status);
    TEST_ASSERT(cpu.halted);
    TEST_ASSERT_EQ_U32(RV32_ERR_HALTED, rv32_cpu_step(&cpu));
    TEST_ASSERT_EQ_U32(RV32_ERR_HALTED, rv32_cpu_run(&cpu, 0U));
    TEST_ASSERT(cpu.instruction_count == UINT64_C(0x100000000));
    TEST_ASSERT_EQ_U32(RV32_OK, rv32_cpu_reset(&cpu, 8U));
    TEST_ASSERT_EQ_U32(RV32_ERR_INVALID_INSTRUCTION, rv32_cpu_step(&cpu));
    TEST_ASSERT_EQ_U32(RV32_OK,
                       rv32_memory_write_u32(&memory, 8U, UINT32_C(0x00100093)));
    TEST_ASSERT_EQ_U32(RV32_ERR_INVALID_INSTRUCTION, rv32_cpu_step(&cpu));
    TEST_ASSERT_EQ_U32(RV32_ERR_INVALID_INSTRUCTION, rv32_cpu_run(&cpu, 5U));
    TEST_ASSERT_EQ_U32(8U, cpu.pc);
    TEST_ASSERT_EQ_U32(0U, cpu.last_instruction);
    TEST_ASSERT_EQ_U32(0U, cpu.regs[1]);
}

static void test_cpu_run_limit_resume(void)
{
    uint8_t bytes[16] = {0U};
    rv32_memory_t memory = {bytes, sizeof(bytes)};
    rv32_cpu_t cpu;

    TEST_ASSERT_EQ_U32(RV32_OK, rv32_cpu_init(&cpu, &memory, 0U));
    TEST_ASSERT_EQ_U32(RV32_OK,
                       rv32_memory_write_u32(&memory, 0U, UINT32_C(0x00108093)));
    TEST_ASSERT_EQ_U32(RV32_OK,
                       rv32_memory_write_u32(&memory, 4U, UINT32_C(0xffdff06f)));
    TEST_ASSERT_EQ_U32(RV32_ERR_STEP_LIMIT, rv32_cpu_run(&cpu, 0U));
    TEST_ASSERT(!cpu.halted);
    TEST_ASSERT_EQ_U32(0U, cpu.pc);
    TEST_ASSERT(cpu.instruction_count == 0U);
    TEST_ASSERT_EQ_U32(RV32_ERR_STEP_LIMIT, rv32_cpu_run(&cpu, 5U));
    TEST_ASSERT_EQ_U32(3U, cpu.regs[1]);
    TEST_ASSERT_EQ_U32(4U, cpu.pc);
    TEST_ASSERT(cpu.instruction_count == 5U);
    TEST_ASSERT_EQ_U32(RV32_ERR_STEP_LIMIT, cpu.last_status);
    TEST_ASSERT(!cpu.halted);
    TEST_ASSERT_EQ_U32(RV32_ERR_STEP_LIMIT, rv32_cpu_run(&cpu, 3U));
    TEST_ASSERT_EQ_U32(4U, cpu.regs[1]);
    TEST_ASSERT_EQ_U32(0U, cpu.pc);
    TEST_ASSERT(cpu.instruction_count == 8U);
    TEST_ASSERT_EQ_U32(RV32_OK, rv32_cpu_step(&cpu));
    TEST_ASSERT_EQ_U32(RV32_OK, cpu.last_status);
    TEST_ASSERT_EQ_U32(5U, cpu.regs[1]);
    TEST_ASSERT(cpu.instruction_count == 9U);
    TEST_ASSERT_EQ_U32(RV32_OK,
                       rv32_memory_write_u32(&memory, 4U, UINT32_C(0x00000073)));
    TEST_ASSERT_EQ_U32(RV32_ERR_HALTED, rv32_cpu_run(&cpu, 1U));
    TEST_ASSERT(cpu.instruction_count == 9U);
}

void test_cpu_register_tests(unsigned int *passed, unsigned int *total)
{
    static const struct {
        const char *name;
        test_function_t function;
    } cases[] = {
        {"cpu init/reset preserves memory", test_cpu_init_reset},
        {"cpu registers and x0", test_cpu_register_access},
        {"cpu invalid arguments", test_cpu_invalid_arguments},
        {"cpu fetch faults", test_cpu_fetch_faults},
        {"cpu halt and diagnostics", test_cpu_halt_and_diagnostics},
        {"cpu run limit and resume", test_cpu_run_limit_resume}
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
