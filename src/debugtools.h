
#include "fileio.h"

void dump_memory(CHIP8 *chip)
{
    int i;

    FILE *memory_dump = fopen("memory_dump.txt", "w");

    fprintf(memory_dump, "\n");
    for (i = 1; i < 0x1001; i++)
        fprintf(memory_dump, "%02X%s", chip->memory[i], (i % 16 == 0) ? "\n" : " ");

    fprintf(memory_dump, "\n");

    fclose(memory_dump);
}

void debug_info(CHIP8 *chip)
{
    int i;

    printf("\n\n");
    printf("\t   PC:\t0x%04X\n", chip->pc - 2);
    printf("\t   OP:\t0x%04X\n", chip->op);
    printf("\t    I:\t0x%04X\n", chip->I);
    printf("\t   SP:\t%d\n", chip->sp);

    printf("STACK:\t[");
    for (i = 0; i < 16; i++)
        printf("0x%X%s", chip->stack[i], (i == 15) ? "]\n" : ", ");

    printf("REGISTERS:\t");
    for (i = 0; i < 16; i++)
    {
        printf("%02X\t", chip->V[i]);
        if (i == 7)
            printf("\n\t\t\t");
    }
    printf("\n");
}
