#include "decoder.h"

#include <limits.h>

static int32_t sign_extend(uint32_t value, unsigned int width)
{
    const uint32_t mask = (UINT32_C(1) << width) - UINT32_C(1);
    const uint32_t sign_bit = UINT32_C(1) << (width - 1U);
    uint32_t magnitude;

    value &= mask;
    if ((value & sign_bit) == 0U) {
        return (int32_t)value;
    }

    magnitude = ((~value) & mask) + UINT32_C(1);
    return -(int32_t)magnitude;
}

static int32_t decode_u_immediate(uint32_t instruction)
{
    const uint32_t value = instruction & UINT32_C(0xfffff000);
    uint32_t magnitude;

    if ((value & UINT32_C(0x80000000)) == 0U) {
        return (int32_t)value;
    }

    magnitude = (~value) + UINT32_C(1);
    if (magnitude == UINT32_C(0x80000000)) {
        return INT32_MIN;
    }
    return -(int32_t)magnitude;
}

rv32_decoded_instruction_t rv32_decode(uint32_t instruction)
{
    rv32_decoded_instruction_t decoded;
    uint32_t immediate;

    decoded.opcode = (uint8_t)(instruction & UINT32_C(0x7f));
    decoded.rd = (uint8_t)((instruction >> 7U) & UINT32_C(0x1f));
    decoded.funct3 = (uint8_t)((instruction >> 12U) & UINT32_C(0x07));
    decoded.rs1 = (uint8_t)((instruction >> 15U) & UINT32_C(0x1f));
    decoded.rs2 = (uint8_t)((instruction >> 20U) & UINT32_C(0x1f));
    decoded.funct7 = (uint8_t)((instruction >> 25U) & UINT32_C(0x7f));

    immediate = (instruction >> 20U) & UINT32_C(0x0fff);
    decoded.imm_i = sign_extend(immediate, 12U);

    immediate = ((instruction >> 7U) & UINT32_C(0x1f))
              | (((instruction >> 25U) & UINT32_C(0x7f)) << 5U);
    decoded.imm_s = sign_extend(immediate, 12U);

    immediate = (((instruction >> 8U) & UINT32_C(0x0f)) << 1U)
              | (((instruction >> 25U) & UINT32_C(0x3f)) << 5U)
              | (((instruction >> 7U) & UINT32_C(0x01)) << 11U)
              | (((instruction >> 31U) & UINT32_C(0x01)) << 12U);
    decoded.imm_b = sign_extend(immediate, 13U);

    decoded.imm_u = decode_u_immediate(instruction);

    immediate = (((instruction >> 21U) & UINT32_C(0x03ff)) << 1U)
              | (((instruction >> 20U) & UINT32_C(0x01)) << 11U)
              | (((instruction >> 12U) & UINT32_C(0x00ff)) << 12U)
              | (((instruction >> 31U) & UINT32_C(0x01)) << 20U);
    decoded.imm_j = sign_extend(immediate, 21U);

    return decoded;
}
