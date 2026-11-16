#include "rv32_types.h"

const char *rv32_status_string(rv32_status_t status)
{
    switch (status) {
    case RV32_OK:
        return "success";
    case RV32_ERR_INVALID_ARGUMENT:
        return "invalid argument";
    case RV32_ERR_OUT_OF_MEMORY:
        return "out of memory";
    case RV32_ERR_MEMORY_BOUNDS:
        return "memory access out of bounds";
    case RV32_ERR_MEMORY_ALIGNMENT:
        return "misaligned memory access";
    case RV32_ERR_INVALID_INSTRUCTION:
        return "invalid instruction";
    case RV32_ERR_STEP_LIMIT:
        return "instruction step limit reached";
    case RV32_ERR_HALTED:
        return "processor halted";
    default:
        return "unknown status";
    }
}

