#ifndef CPU_H
#define CPU_H

#include <stdbool.h>
#include <stdint.h>

#include "memory.h"
#include "rv32_types.h"

typedef struct {
    uint32_t regs[32];
    uint32_t pc;
    rv32_memory_t *memory; /* Borrowed; must outlive this CPU. */
    bool halted;
    rv32_status_t last_status;
    uint32_t last_instruction;
    uint64_t instruction_count;
} rv32_cpu_t;

/* Reset registers and diagnostics without clearing or taking ownership of RAM.
 * Entry must be four-byte aligned; its bounds are checked on the first fetch.
 * init also establishes the memory connection. */
rv32_status_t rv32_cpu_init(rv32_cpu_t *cpu, rv32_memory_t *memory,
                            uint32_t entry);
rv32_status_t rv32_cpu_reset(rv32_cpu_t *cpu, uint32_t entry);

/* Reads of x0, invalid indices or a NULL CPU return zero. Writes to x0 succeed
 * but discard the value. Invalid writes return INVALID_ARGUMENT. */
uint32_t rv32_cpu_read_register(const rv32_cpu_t *cpu, unsigned int index);
rv32_status_t rv32_cpu_write_register(rv32_cpu_t *cpu, unsigned int index,
                                      uint32_t value);

/* Only successfully retired instructions advance PC/count. ECALL halts at its
 * own PC and is not retired. Errors latch halted/last_status; repeated steps
 * return that status until reset. A failed fetch clears last_instruction. */
rv32_status_t rv32_cpu_step(rv32_cpu_t *cpu);

/* Execute at most step_limit instructions in this call. Exhaustion (including
 * zero) returns STEP_LIMIT without halting, so execution can be resumed. */
rv32_status_t rv32_cpu_run(rv32_cpu_t *cpu, uint64_t step_limit);

#endif
