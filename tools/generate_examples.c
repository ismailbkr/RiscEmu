#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int write_program(const char *path, const uint32_t *words, size_t count)
{
    FILE *file = fopen(path, "wb");
    size_t index;

    if (file == NULL) {
        fprintf(stderr, "Could not open %s: %s\n", path, strerror(errno));
        return 0;
    }
    for (index = 0U; index < count; index++) {
        const unsigned char bytes[4] = {
            (unsigned char)(words[index] & UINT32_C(0xff)),
            (unsigned char)((words[index] >> 8U) & UINT32_C(0xff)),
            (unsigned char)((words[index] >> 16U) & UINT32_C(0xff)),
            (unsigned char)(words[index] >> 24U)
        };
        if (fwrite(bytes, sizeof(bytes), 1U, file) != 1U) {
            fprintf(stderr, "Could not write %s\n", path);
            (void)fclose(file);
            return 0;
        }
    }
    if (fclose(file) != 0) {
        fprintf(stderr, "Could not close %s\n", path);
        return 0;
    }
    return 1;
}

int main(int argc, char **argv)
{
    static const uint32_t addi_ecall[] = {
        UINT32_C(0x00500093), UINT32_C(0x00000073)
    };
    static const uint32_t sum_loop[] = {
        UINT32_C(0x00000093), UINT32_C(0x00a00113),
        UINT32_C(0x002080b3), UINT32_C(0xfff10113),
        UINT32_C(0xfe011ce3), UINT32_C(0x10102023),
        UINT32_C(0x00000073)
    };
    char first_path[1024];
    char second_path[1024];
    int first_length;
    int second_length;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s OUTPUT_DIRECTORY\n", argv[0]);
        return EXIT_FAILURE;
    }
    first_length = snprintf(first_path, sizeof(first_path), "%s/addi_ecall.bin",
                            argv[1]);
    second_length = snprintf(second_path, sizeof(second_path), "%s/sum_loop.bin",
                             argv[1]);
    if (first_length < 0 || (size_t)first_length >= sizeof(first_path)
        || second_length < 0 || (size_t)second_length >= sizeof(second_path)) {
        fputs("Output path is too long\n", stderr);
        return EXIT_FAILURE;
    }
    if (!write_program(first_path, addi_ecall,
                       sizeof(addi_ecall) / sizeof(addi_ecall[0]))
        || !write_program(second_path, sum_loop,
                          sizeof(sum_loop) / sizeof(sum_loop[0]))) {
        return EXIT_FAILURE;
    }
    printf("Generated %s and %s\n", first_path, second_path);
    return EXIT_SUCCESS;
}
