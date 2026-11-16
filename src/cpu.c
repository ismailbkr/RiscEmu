#include "cpu.h"

#include <stddef.h>
#include <string.h>

#include "decoder.h"

static rv32_status_t stop_cpu(rv32_cpu_t *cpu, rv32_status_t status)
{
    cpu->halted = true;
    cpu->last_status = status;
    return status;
}

static bool memory_is_valid(const rv32_memory_t *memory)
{
    return memory != NULL && memory->data != NULL && memory->size != 0U;
}

/* Biasing the sign bit gives signed ordering without out-of-range casts. */
static bool signed_less(uint32_t left, uint32_t right)
{
    return (left ^ UINT32_C(0x80000000)) < (right ^ UINT32_C(0x80000000));
}

static uint32_t arithmetic_shift_right(uint32_t value, uint32_t amount)
{
    amount &= 31U;
    if (amount == 0U) {
        return value;
    }
    if ((value & UINT32_C(0x80000000)) != 0U) {
        return (value >> amount) | (UINT32_MAX << (32U - amount));
    }
    return value >> amount;
}

static rv32_status_t execute_alu(const rv32_decoded_instruction_t *d,
                                  uint32_t left, uint32_t right,
                                  bool immediate, uint32_t *result)
{
    /* Only SUB/SRA may use funct7=0x20 in the register family. */
    if (!immediate && d->funct7 != 0U
        && !(d->funct7 == 0x20U && (d->funct3 == 0U || d->funct3 == 5U))) {
        return RV32_ERR_INVALID_INSTRUCTION;
    }

    switch (d->funct3) {
    case 0U:
        *result = !immediate && d->funct7 == 0x20U
                      ? left - right : left + right;
        break;
    case 1U:
        if (d->funct7 != 0U) {
            return RV32_ERR_INVALID_INSTRUCTION;
        }
        *result = left << (right & 31U);
        break;
    case 2U:
        *result = signed_less(left, right) ? 1U : 0U;
        break;
    case 3U:
        *result = left < right ? 1U : 0U;
        break;
    case 4U:
        *result = left ^ right;
        break;
    case 5U:
        if (d->funct7 != 0U && d->funct7 != 0x20U) {
            return RV32_ERR_INVALID_INSTRUCTION;
        }
        *result = d->funct7 == 0x20U
                      ? arithmetic_shift_right(left, right) : left >> (right & 31U);
        break;
    case 6U:
        *result = left | right;
        break;
    case 7U:
        *result = left & right;
        break;
    default:
        return RV32_ERR_INVALID_INSTRUCTION;
    }
    return RV32_OK;
}

static rv32_status_t execute_load(const rv32_memory_t *memory,
                                   uint8_t funct3, uint32_t address,
                                   uint32_t *result)
{
    uint8_t byte = 0U;
    uint16_t half = 0U;
    rv32_status_t status;

    switch (funct3) {
    case 0U:
    case 4U:
        status = rv32_memory_read_u8(memory, address, &byte);
        *result = byte;
        if (funct3 == 0U && (byte & 0x80U) != 0U) {
            *result |= UINT32_C(0xffffff00);
        }
        return status;
    case 1U:
    case 5U:
        status = rv32_memory_read_u16(memory, address, &half);
        *result = half;
        if (funct3 == 1U && (half & 0x8000U) != 0U) {
            *result |= UINT32_C(0xffff0000);
        }
        return status;
    case 2U:
        return rv32_memory_read_u32(memory, address, result);
    default:
        return RV32_ERR_INVALID_INSTRUCTION;
    }
}

static rv32_status_t execute_store(rv32_memory_t *memory, uint8_t funct3,
                                    uint32_t address, uint32_t value)
{
    switch (funct3) {
    case 0U:
        return rv32_memory_write_u8(memory, address, (uint8_t)value);
    case 1U:
        return rv32_memory_write_u16(memory, address, (uint16_t)value);
    case 2U:
        return rv32_memory_write_u32(memory, address, value);
    default:
        return RV32_ERR_INVALID_INSTRUCTION;
    }
}

static rv32_status_t execute_branch(uint8_t funct3, uint32_t left,
                                     uint32_t right, bool *taken)
{
    switch (funct3) {
    case 0U: *taken = left == right; break;
    case 1U: *taken = left != right; break;
    case 4U: *taken = signed_less(left, right); break;
    case 5U: *taken = !signed_less(left, right); break;
    case 6U: *taken = left < right; break;
    case 7U: *taken = left >= right; break;
    default: return RV32_ERR_INVALID_INSTRUCTION;
    }
    return RV32_OK;
}

static rv32_status_t execute(rv32_cpu_t *cpu, uint32_t instruction)
{
    const rv32_decoded_instruction_t d = rv32_decode(instruction);
    const uint32_t left = rv32_cpu_read_register(cpu, d.rs1);
    const uint32_t right = rv32_cpu_read_register(cpu, d.rs2);
    uint32_t next_pc = cpu->pc + 4U;
    uint32_t result = 0U;
    bool write_rd = true;
    bool taken = false;
    rv32_status_t status = RV32_OK;

    switch (d.opcode) {
    case 0x37U: /* LUI */
        result = (uint32_t)d.imm_u;
        break;
    case 0x17U: /* AUIPC */
        result = cpu->pc + (uint32_t)d.imm_u;
        break;
    case 0x6fU: /* JAL */
        result = next_pc;
        next_pc = cpu->pc + (uint32_t)d.imm_j;
        break;
    case 0x67U: /* JALR: read rs1 before possibly replacing it with the link. */
        if (d.funct3 != 0U) {
            return RV32_ERR_INVALID_INSTRUCTION;
        }
        result = next_pc;
        next_pc = (left + (uint32_t)d.imm_i) & ~UINT32_C(1);
        break;
    case 0x63U:
        write_rd = false;
        status = execute_branch(d.funct3, left, right, &taken);
        if (taken) {
            next_pc = cpu->pc + (uint32_t)d.imm_b;
        }
        break;
    case 0x03U:
        status = execute_load(cpu->memory, d.funct3,
                              left + (uint32_t)d.imm_i, &result);
        break;
    case 0x23U:
        write_rd = false;
        status = execute_store(cpu->memory, d.funct3,
                               left + (uint32_t)d.imm_s, right);
        break;
    case 0x13U:
        status = execute_alu(&d, left, (uint32_t)d.imm_i, true, &result);
        break;
    case 0x33U:
        status = execute_alu(&d, left, right, false, &result);
        break;
    case 0x0fU:
        /* Base FENCE ignores rd/rs1 and reserved fm bits. FENCE.I is not RV32I. */
        if (d.funct3 != 0U) {
            return RV32_ERR_INVALID_INSTRUCTION;
        }
        write_rd = false;
        break;
    case 0x73U:
        return instruction == UINT32_C(0x00000073)
                   || instruction == UINT32_C(0x00100073)
                   ? RV32_ERR_HALTED : RV32_ERR_INVALID_INSTRUCTION;
    default:
        return RV32_ERR_INVALID_INSTRUCTION;
    }

    if (status != RV32_OK) {
        return status;
    }
    if ((next_pc & 3U) != 0U) {
        return RV32_ERR_MEMORY_ALIGNMENT;
    }
    if (write_rd) {
        (void)rv32_cpu_write_register(cpu, d.rd, result);
    }
    cpu->pc = next_pc;
    cpu->instruction_count++;
    return RV32_OK;
}

rv32_status_t rv32_cpu_init(rv32_cpu_t *cpu, rv32_memory_t *memory,
                            uint32_t entry)
{
    if (cpu == NULL) {
        return RV32_ERR_INVALID_ARGUMENT;
    }
    cpu->memory = memory;
    return rv32_cpu_reset(cpu, entry);
}

rv32_status_t rv32_cpu_reset(rv32_cpu_t *cpu, uint32_t entry)
{
    if (cpu == NULL) {
        return RV32_ERR_INVALID_ARGUMENT;
    }
    memset(cpu->regs, 0, sizeof(cpu->regs));
    cpu->pc = entry;
    cpu->halted = false;
    cpu->last_status = RV32_OK;
    cpu->last_instruction = 0U;
    cpu->instruction_count = 0U;
    if (!memory_is_valid(cpu->memory)) {
        return stop_cpu(cpu, RV32_ERR_INVALID_ARGUMENT);
    }
    if ((entry & 3U) != 0U) {
        return stop_cpu(cpu, RV32_ERR_MEMORY_ALIGNMENT);
    }
    return RV32_OK;
}

uint32_t rv32_cpu_read_register(const rv32_cpu_t *cpu, unsigned int index)
{
    if (cpu == NULL || index == 0U || index >= 32U) {
        return 0U;
    }
    return cpu->regs[index];
}

rv32_status_t rv32_cpu_write_register(rv32_cpu_t *cpu, unsigned int index,
                                      uint32_t value)
{
    if (cpu == NULL || index >= 32U) {
        return RV32_ERR_INVALID_ARGUMENT;
    }
    cpu->regs[0] = 0U;
    if (index != 0U) {
        cpu->regs[index] = value;
    }
    return RV32_OK;
}

rv32_status_t rv32_cpu_step(rv32_cpu_t *cpu)
{
    rv32_status_t status;

    if (cpu == NULL) {
        return RV32_ERR_INVALID_ARGUMENT;
    }
    cpu->regs[0] = 0U;
    if (cpu->halted) {
        return cpu->last_status;
    }
    cpu->last_instruction = 0U;
    if (!memory_is_valid(cpu->memory)) {
        return stop_cpu(cpu, RV32_ERR_INVALID_ARGUMENT);
    }
    if ((cpu->pc & 3U) != 0U) {
        return stop_cpu(cpu, RV32_ERR_MEMORY_ALIGNMENT);
    }
    status = rv32_memory_read_u32(cpu->memory, cpu->pc,
                                  &cpu->last_instruction);
    if (status == RV32_OK) {
        status = execute(cpu, cpu->last_instruction);
    }
    if (status != RV32_OK) {
        return stop_cpu(cpu, status);
    }
    cpu->last_status = RV32_OK;
    return RV32_OK;
}

rv32_status_t rv32_cpu_run(rv32_cpu_t *cpu, uint64_t step_limit)
{
    uint64_t step;
    rv32_status_t status;

    if (cpu == NULL) {
        return RV32_ERR_INVALID_ARGUMENT;
    }
    if (cpu->halted) {
        return cpu->last_status;
    }
    if (!memory_is_valid(cpu->memory)) {
        return stop_cpu(cpu, RV32_ERR_INVALID_ARGUMENT);
    }
    for (step = 0U; step < step_limit; step++) {
        status = rv32_cpu_step(cpu);
        if (status != RV32_OK) {
            return status;
        }
    }
    cpu->last_status = RV32_ERR_STEP_LIMIT;
    return cpu->last_status;
}
