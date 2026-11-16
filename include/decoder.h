#ifndef DECODER_H
#define DECODER_H

#include <stdint.h>

typedef struct {
    uint8_t opcode;
    uint8_t rd;
    uint8_t funct3;
    uint8_t rs1;
    uint8_t rs2;
    uint8_t funct7;
    int32_t imm_i;
    int32_t imm_s;
    int32_t imm_b;
    int32_t imm_u;
    int32_t imm_j;
} rv32_decoded_instruction_t;

rv32_decoded_instruction_t rv32_decode(uint32_t instruction);

#endif
