
#include <stdio.h>
#include "chip8.h"

BYTE prog[132];

int main(int argc, char **argv)
{
    CHIP8 chip;
    FILE *program;
    int i;

    // Set up graphics
    // Set up input

    init_chip(&chip);

    program = fopen("TETRIS", "rb");
    fread(prog, 1, 132, program);

    // Load chip8 game
    for (i = 0; i < 132; i++)
    {
        chip.memory[i + 0x200] = prog[i];
    }

    while (1)
    {
        cycle(&chip);

        // Update timers

        // If draw flag is set, update screen

        // Store key press state (press and release)
    }

    return 0;
}
