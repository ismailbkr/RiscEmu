#include "memory.h"

#include <stdlib.h>
#include <string.h>

static rv32_status_t validate_access(const rv32_memory_t *memory,
                                     uint32_t address,
                                     size_t width,
                                     uint32_t alignment)
{
    size_t offset;

    if (memory == NULL || memory->data == NULL || memory->size == 0U) {
        return RV32_ERR_INVALID_ARGUMENT;
    }
    offset = (size_t)address;
    if (offset > memory->size || width > memory->size - offset) {
        return RV32_ERR_MEMORY_BOUNDS;
    }
    if ((address % alignment) != 0U) {
        return RV32_ERR_MEMORY_ALIGNMENT;
    }
    return RV32_OK;
}

rv32_status_t rv32_memory_init(rv32_memory_t *memory, size_t size)
{
    uint8_t *data;

    if (memory == NULL || size == 0U) {
        return RV32_ERR_INVALID_ARGUMENT;
    }

    data = calloc(size, sizeof(*data));
    if (data == NULL) {
        return RV32_ERR_OUT_OF_MEMORY;
    }

    memory->data = data;
    memory->size = size;
    return RV32_OK;
}

void rv32_memory_destroy(rv32_memory_t *memory)
{
    if (memory == NULL) {
        return;
    }

    free(memory->data);
    memory->data = NULL;
    memory->size = 0U;
}

rv32_status_t rv32_memory_reset(rv32_memory_t *memory)
{
    if (memory == NULL || memory->data == NULL || memory->size == 0U) {
        return RV32_ERR_INVALID_ARGUMENT;
    }

    memset(memory->data, 0, memory->size);
    return RV32_OK;
}

rv32_status_t rv32_memory_read_u8(const rv32_memory_t *memory,
                                  uint32_t address,
                                  uint8_t *value)
{
    rv32_status_t status;

    if (value == NULL) {
        return RV32_ERR_INVALID_ARGUMENT;
    }
    status = validate_access(memory, address, 1U, 1U);
    if (status != RV32_OK) {
        return status;
    }

    *value = memory->data[(size_t)address];
    return RV32_OK;
}

rv32_status_t rv32_memory_read_u16(const rv32_memory_t *memory,
                                   uint32_t address,
                                   uint16_t *value)
{
    size_t offset;
    rv32_status_t status;

    if (value == NULL) {
        return RV32_ERR_INVALID_ARGUMENT;
    }
    status = validate_access(memory, address, 2U, 2U);
    if (status != RV32_OK) {
        return status;
    }

    offset = (size_t)address;
    *value = (uint16_t)((uint16_t)memory->data[offset]
                        | ((uint16_t)memory->data[offset + 1U] << 8U));
    return RV32_OK;
}

rv32_status_t rv32_memory_read_u32(const rv32_memory_t *memory,
                                   uint32_t address,
                                   uint32_t *value)
{
    size_t offset;
    rv32_status_t status;

    if (value == NULL) {
        return RV32_ERR_INVALID_ARGUMENT;
    }
    status = validate_access(memory, address, 4U, 4U);
    if (status != RV32_OK) {
        return status;
    }

    offset = (size_t)address;
    *value = (uint32_t)memory->data[offset]
             | ((uint32_t)memory->data[offset + 1U] << 8U)
             | ((uint32_t)memory->data[offset + 2U] << 16U)
             | ((uint32_t)memory->data[offset + 3U] << 24U);
    return RV32_OK;
}

rv32_status_t rv32_memory_write_u8(rv32_memory_t *memory,
                                   uint32_t address,
                                   uint8_t value)
{
    rv32_status_t status = validate_access(memory, address, 1U, 1U);

    if (status != RV32_OK) {
        return status;
    }
    memory->data[(size_t)address] = value;
    return RV32_OK;
}

rv32_status_t rv32_memory_write_u16(rv32_memory_t *memory,
                                    uint32_t address,
                                    uint16_t value)
{
    size_t offset;
    rv32_status_t status = validate_access(memory, address, 2U, 2U);

    if (status != RV32_OK) {
        return status;
    }

    offset = (size_t)address;
    memory->data[offset] = (uint8_t)(value & UINT16_C(0x00FF));
    memory->data[offset + 1U] = (uint8_t)(value >> 8U);
    return RV32_OK;
}

rv32_status_t rv32_memory_write_u32(rv32_memory_t *memory,
                                    uint32_t address,
                                    uint32_t value)
{
    size_t offset;
    rv32_status_t status = validate_access(memory, address, 4U, 4U);

    if (status != RV32_OK) {
        return status;
    }

    offset = (size_t)address;
    memory->data[offset] = (uint8_t)(value & UINT32_C(0x000000FF));
    memory->data[offset + 1U] = (uint8_t)((value >> 8U)
                                          & UINT32_C(0x000000FF));
    memory->data[offset + 2U] = (uint8_t)((value >> 16U)
                                          & UINT32_C(0x000000FF));
    memory->data[offset + 3U] = (uint8_t)(value >> 24U);
    return RV32_OK;
}
