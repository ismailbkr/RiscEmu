#include <stdio.h>
#include <stdint.h>


int main (void){

    uint32_t pc = 0; 
    
    printf("PC = %u\n", pc);

    pc = pc + 4;
    
    printf("PC = %u\n", pc);
    
    return 0;

}

