#include <stdint.h>
#include <stdio.h>

int main(void){

    uint32_t instruction = 0x003100B3u;
    uint32_t opcode = instruction & 0X7Fu;

    uint32_t rd = (instruction >> 7 ) & 0x1Fu;
    uint32_t rs1 = (instruction >> 15) & 0x1Fu;
    uint32_t rs2 = (instruction >> 20) & 0x1Fu;

    uint32_t funct3 = (instruction >> 12) & 0x7u;
    uint32_t funct7 = (instruction >> 25) & 0x7Fu;


    printf("Komut =  0x%08X\n", (unsigned int)instruction);
    printf("Opcode = 0x%02X\n", (unsigned ) opcode); 
    printf("rd     = x%u\n", (unsigned int)rd);

    printf("rs1    = x%u\n", (unsigned int)rs1);
    printf("rs2    = x%u\n", (unsigned int)rs2);

    printf("funct3 = 0x%X\n", (unsigned int)funct3);
    printf("funct7 = 0x%02X\n", (unsigned int)funct7);


    if (opcode == 0x33u && funct3 == 0x0u && funct7 == 0x00u) {
         printf("Komut turu = ADD\n");
    } else {
         printf("Komut turu = bilinmiyor\n");
    }








    return 0;

}
