#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

#include "rv32_types.h"

typedef struct {
    uint8_t *data;
    size_t size;
} rv32_memory_t;

rv32_status_t rv32_memory_init(rv32_memory_t *memory, size_t size);
void rv32_memory_destroy(rv32_memory_t *memory);
rv32_status_t rv32_memory_reset(rv32_memory_t *memory);

rv32_status_t rv32_memory_read_u8(const rv32_memory_t *memory,
                                  uint32_t address,
                                  uint8_t *value);
rv32_status_t rv32_memory_read_u16(const rv32_memory_t *memory,
                                   uint32_t address,
                                   uint16_t *value);
rv32_status_t rv32_memory_read_u32(const rv32_memory_t *memory,
                                   uint32_t address,
                                   uint32_t *value);

rv32_status_t rv32_memory_write_u8(rv32_memory_t *memory,
                                   uint32_t address,
                                   uint8_t value);
rv32_status_t rv32_memory_write_u16(rv32_memory_t *memory,
                                    uint32_t address,
                                    uint16_t value);
rv32_status_t rv32_memory_write_u32(rv32_memory_t *memory,
                                    uint32_t address,
                                    uint32_t value);

#endif
