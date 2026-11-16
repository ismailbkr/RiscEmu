#ifndef RV32_TYPES_H
#define RV32_TYPES_H

typedef enum {
    RV32_OK = 0,
    RV32_ERR_INVALID_ARGUMENT,
    RV32_ERR_OUT_OF_MEMORY,
    RV32_ERR_MEMORY_BOUNDS,
    RV32_ERR_MEMORY_ALIGNMENT,
    RV32_ERR_INVALID_INSTRUCTION,
    RV32_ERR_STEP_LIMIT,
    RV32_ERR_HALTED
} rv32_status_t;

const char *rv32_status_string(rv32_status_t status);

#endif

