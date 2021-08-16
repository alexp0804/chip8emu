
#include <stdio.h>
#include "chip8.h"

BYTE prog[494];

int main(int argc, char **argv)
{
    CHIP8 chip;
    FILE *program;
    int i;

    // Set up graphics
    // Set up input

    init_chip(&chip);

    program = fopen("TETRIS", "rb");
    fread(prog, 1, 494, program);

    // Load chip8 game
    for (i = 0; i < 494; i++)
    {
    }

    while (1)
    {
        break;
        // Fetch Opcode
        // Decode Opcode
        // Execute Opcode

        // Update timers

        // If draw flag is set, update screen

        // Store key press state (press and release)
    }

    return 0;
}

// for (i = 0; i < 474; i++)
// {
//     printf("0x%X \t", prog[i]);
//     if (i % 3 == 0)
//     {
//         printf("\n");
//     }
// }