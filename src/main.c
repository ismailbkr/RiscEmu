#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "memory.h"
#include "rv32_types.h"

#define DEFAULT_MEMORY_SIZE (1024U * 1024U)
#define DEFAULT_STEP_LIMIT UINT64_C(1000000)

static void print_usage(const char *program)
{
    fprintf(stderr,
            "Usage: %s [--max-steps N] program.bin\n"
            "Loads a little-endian RV32I raw binary at address 0.\n",
            program);
}

static int parse_step_limit(const char *text, uint64_t *value)
{
    char *end = NULL;
    const char *cursor;
    unsigned long long parsed;

    if (text == NULL || value == NULL || text[0] == '\0') {
        return 0;
    }
    for (cursor = text; *cursor != '\0'; cursor++) {
        if (*cursor < '0' || *cursor > '9') {
            return 0;
        }
    }
    errno = 0;
    parsed = strtoull(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || parsed == 0ULL) {
        return 0;
    }
    *value = (uint64_t)parsed;
    return 1;
}

static rv32_status_t load_raw_binary(const char *path, rv32_memory_t *memory,
                                     size_t *loaded_size)
{
    FILE *file;
    long file_size;
    size_t bytes_read;

    if (path == NULL || memory == NULL || memory->data == NULL
        || loaded_size == NULL) {
        return RV32_ERR_INVALID_ARGUMENT;
    }
    file = fopen(path, "rb");
    if (file == NULL) {
        return RV32_ERR_IO;
    }
    if (fseek(file, 0L, SEEK_END) != 0) {
        (void)fclose(file);
        return RV32_ERR_IO;
    }
    file_size = ftell(file);
    if (file_size <= 0L || ((unsigned long)file_size % 4UL) != 0UL) {
        (void)fclose(file);
        return RV32_ERR_IMAGE_FORMAT;
    }
    if ((unsigned long)file_size > memory->size) {
        (void)fclose(file);
        return RV32_ERR_IMAGE_TOO_LARGE;
    }
    if (fseek(file, 0L, SEEK_SET) != 0) {
        (void)fclose(file);
        return RV32_ERR_IO;
    }
    bytes_read = fread(memory->data, 1U, (size_t)file_size, file);
    if (bytes_read != (size_t)file_size || ferror(file) != 0) {
        (void)fclose(file);
        return RV32_ERR_IO;
    }
    if (fclose(file) != 0) {
        return RV32_ERR_IO;
    }
    *loaded_size = bytes_read;
    return RV32_OK;
}

static void print_registers(const rv32_cpu_t *cpu)
{
    unsigned int index;

    for (index = 0U; index < 32U; index++) {
        printf("x%-2u=0x%08" PRIx32 "%c", index,
               rv32_cpu_read_register(cpu, index),
               (index % 4U) == 3U ? '\n' : ' ');
    }
}

int main(int argc, char **argv)
{
    const char *path;
    uint64_t step_limit = DEFAULT_STEP_LIMIT;
    rv32_memory_t memory = {NULL, 0U};
    rv32_cpu_t cpu;
    rv32_status_t status;
    size_t loaded_size = 0U;
    int argument = 1;
    int exit_code = EXIT_FAILURE;

    if (argc >= 3 && strcmp(argv[1], "--max-steps") == 0) {
        if (!parse_step_limit(argv[2], &step_limit)) {
            fprintf(stderr, "Invalid step limit: %s\n", argv[2]);
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
        argument = 3;
    }
    if (argument != argc - 1) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
    path = argv[argument];

    status = rv32_memory_init(&memory, DEFAULT_MEMORY_SIZE);
    if (status != RV32_OK) {
        fprintf(stderr, "Memory initialization failed: %s\n",
                rv32_status_string(status));
        return EXIT_FAILURE;
    }
    status = load_raw_binary(path, &memory, &loaded_size);
    if (status != RV32_OK) {
        fprintf(stderr, "Could not load '%s': %s\n", path,
                rv32_status_string(status));
        goto cleanup;
    }
    status = rv32_cpu_init(&cpu, &memory, 0U);
    if (status != RV32_OK) {
        fprintf(stderr, "CPU initialization failed: %s\n",
                rv32_status_string(status));
        goto cleanup;
    }

    printf("Loaded %zu bytes from %s\n", loaded_size, path);
    status = rv32_cpu_run(&cpu, step_limit);
    printf("Stopped: %s, pc=0x%08" PRIx32 ", instructions=%" PRIu64
           "\n",
           rv32_status_string(status), cpu.pc, cpu.instruction_count);
    print_registers(&cpu);

    if (status == RV32_ERR_HALTED) {
        exit_code = EXIT_SUCCESS;
    } else {
        fprintf(stderr, "Last instruction: 0x%08" PRIx32 "\n",
                cpu.last_instruction);
    }

cleanup:
    rv32_memory_destroy(&memory);
    return exit_code;
}

