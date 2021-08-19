
#include "chip8.h"


// Convert a 2D index into a 1D index
unsigned int IX(BYTE x, BYTE y)
{
    return y * WIDTH + x;
}

void fetch(CHIP8 *chip)
{
    // Opcodes are words, stored in memory at PC and PC + 1.
    // Shift (PC) and OR with (PC + 1) to get opcode.

    chip->op = (chip->memory[chip->pc] << 0x8) | (chip->memory[chip->pc + 1]);
    chip->pc += 2;
}

void cycle(CHIP8 *chip)
{
    WORD x, y, i, j, dec, NNN, temp;
    BYTE X, Y, N, NN, key, a, b;
    BYTE sprite_byte;

    srand(time(0));

    // Get next instruction
    fetch(chip);

    // The values X, Y, N, NN, and NNN are always extracted the same way,
    // so doing it here will save a lot of code.
    // They are:
    //      X   = 0x_X__
    //      Y   = 0x__Y_
    //      N   = 0x___N
    //      NN  = 0x__NN
    //      NNN = 0x_NNN

    X   = (chip->op & 0x0F00) >> 8;
    Y   = (chip->op & 0x00F0) >> 4;
    N   = (chip->op & 0x000F);
    NN  = (chip->op & 0x00FF);
    NNN = (chip->op & 0x0FFF);

    // Start with just the most significant nibble.
    // We can identify *most* instructions with just this information.
    switch (chip->op & 0xF000)
    {
        case 0x0000: // 0x00EN
            switch(chip->op & 0x000F) 
            {
                case 0x0000: // 0x00E0: Clear screen
                    for (i = 0; i < WIDTH*HEIGHT; i++)
                        chip->screen[i] = 0;
                    break;

                case 0x000E: // 0x00EE: Return from sbr
                    chip->pc = chip->stack[chip->sp--];
                    break;

                default:
                    printf("Unkown opcode: 0x%X\n", chip->op);
                    break;
            }
            break;

        case 0x1000: // 0x1NNN: Jump to NNN
            chip->pc = NNN;
            break;

        case 0x2000: // 0x2NNN: Execute sbr at NNN
            // Push current PC onto stack
            chip->stack[++chip->sp] = chip->pc;

            // Jump to NNN
            chip->pc = NNN;
            break;

        case 0x3000: // 0x3XNN: Skip if V[X] == NN
            if (chip->V[X] == NN)
                chip->pc += 2;

            break;

        case 0x4000: // 0x4XNN: Skip if V[X] != NN
            if (chip->V[X] != NN)
                chip->pc += 2;

            break;

        case 0x5000: // 0x5XY0: Skip if V[X] == V[Y]
            if (chip->V[X] == chip->V[Y])
                chip->pc += 2;

            break;

        case 0x6000: // 0x6XNN: Store NN in V[X]
            chip->V[X] = NN;
            break;

        case 0x7000: // 0x7XNN: Add NN to V[X]
            chip->V[X] += NN;
            break;
        
        case 0x8000: // 0x8XYN
            switch (chip->op & 0x000F)
            {
                case 0x0000: // 0x8XY0: Store V[Y] in V[X]
                    chip->V[X] = chip->V[Y];
                    break;
                
                case 0x0001: // 0x8XY1: Set V[X] to V[X] | V[Y]
                    chip->V[X] |= chip->V[Y];
                    break;

                case 0x0002: // 0x8XY2: Set V[X] to V[X] & V[Y]
                    chip->V[X] &= chip->V[Y];
                    break;                

                case 0x0003: // 0x8XY3: Set V[X] to V[X] ^ V[Y]
                    chip->V[X] ^= chip->V[Y];
                    break;                
                
                case 0x0004: // 0x8XY4: Add V[Y] to V[X], set flags
                    a = chip->V[X], b = chip->V[Y];

                    // Detect overflow
                    if ((a + b) < a || (a + b) < b)
                        chip->V[0xF] = 1;
                    else
                        chip->V[0xF] = 0;

                    chip->V[X] += b;
                    break;
                
                case 0x0005: // 0x8XY5: Set V[X] to V[X] - V[Y], set flags
                    if (chip->V[X] > chip->V[Y])
                        chip->V[0xF] = 1;
                    else if (chip->V[X] < chip->V[Y])
                        chip->V[0xF] = 0;
                    
                    chip->V[X] = chip->V[X] - chip->V[Y];
                    break;
                
                case 0x0006: // 0x8XY6: Store (V[Y] >> 1) in V[X], set flags
                    chip->V[0xF] = chip->V[Y] & 0x1;
                    chip->V[X] = chip->V[Y] >> 1;
                    break;
                
                case 0x0007: // 0x8XY7: Set V[X] to V[Y] - V[X], set flags
                    if (chip->V[Y] > chip->V[X])
                        chip->V[0xF] = 1;
                    else if (chip->V[Y] < chip->V[X])
                        chip->V[0xF] = 0;
                    
                    chip->V[X] = chip->V[Y] - chip->V[X];
                    break;
                
                case 0x000E: // 0x8XYE: Store (V[Y] << 1) in V[X], set flags
                    chip->V[0xF] = chip->V[Y] & 0x80;
                    chip->V[X] = chip->V[Y] << 1;
                    break;
                
                default:
                    printf("Unkown opcode: 0x%X\n", chip->op);
                    break;
            }
            break;

        case 0x9000: // 0x9XY0: Skip if V[X] != V[Y]
            if (chip->V[X] != chip->V[Y])
                chip->pc += 2;

            break;

        case 0xA000: // 0xANNN: Store NNN in I
            chip->I = NNN;
            break;
        
        case 0xB000: // 0xBNNN: Jump to NNN + V[0]
            chip->pc = NNN + chip->V[0];
            break;

        case 0xC000: // 0xCXNN: Store random number in V[X]
            chip->V[X] = (rand() % 255) & NN;
            break;

        case 0xD000: // 0xDXYN: Draw pixels
            // Get x,y coordinate of the sprite, modulo to say in bounds
            x = chip->V[X] % WIDTH, y = chip->V[Y] % HEIGHT;            

            // Set flags
            chip->V[0xF] = 0;
            chip->draw = 1;

            // For N rows
            for (i = 0; i < N; i++)
            {
                // Get the sprite byte addressed by I+i
                sprite_byte = chip->memory[chip->I + i];

                // For each bit in that byte
                for (j = 0; j < 8; j++)
                {
                    // 0 or 1, indicating if the current bit is set
                    if (sprite_byte & (0x80 >>  j))
                    {
                        // If we are about to unset, set the collision flag
                        if (chip->screen[IX(x+j,y+i)])
                            chip->V[0xF] = 1;

                        chip->screen[IX(x+j,y+i)] ^= 1;
                    }

                    // If off the side of the screen quit drawing this row
                    if (x+j >= WIDTH)
                        break;

                }
                // If off the bottom of the screen quit drawing
                if (y+i >= HEIGHT)
                    break;
            }
            break;

        case 0xE000:
            switch (chip->op & 0x00FF)
            {
                case 0x009E: // 0xEX9E: If the key in V[X] is pressed, skip
                    key = chip->V[X];

                    if (chip->keys[key] == 1)
                        chip->pc += 2;

                    break;
                
                case 0x00A1: // 0xEXA1: If the key in V[X] isn't pressed, skip
                    key = chip->V[X];

                    if (chip->keys[key] == 0)
                        chip->pc += 2;

                    break;

                default:
                    printf("Uknown opcode: 0x%X\n", chip->op);
                    break;
            }
            break;

        case 0xF000:
            switch (chip->op & 0x00FF)
            {
                case 0x0007: // 0xFX15: Set V[X] to delay timer
                    chip->V[X] = chip->delay;
                    break;

                case 0x000A: // 0xFX0A: Stops PC unless a key is pressed
                    chip->pc -= 2;

                    // When key is pressed, store it into V[X] and increment PC
                    for (i = 0; i < 0xF; i++)
                    {
                        if (chip->keys[i] == 1)
                        {
                            chip->V[X] = i; // POSSIBLE ERROR: COULD BE i+1?
                        }
                    }
                    break;


                case 0x0015: // 0xFX15: Set delay timer to V[X]
                    chip->delay = chip->V[X];
                    break;

                case 0x0018: // 0xFX18: Set sound timer to V[X]
                    chip->sound = chip->V[X];
                    break;

                case 0x001E: // 0xFX1E: Add V[X] to I 
                    chip->I += chip->V[X];
                    break;

                case 0x0029: // 0xFX29: Store V[X] in I
                    chip->I = chip->V[X];
                    break;

                case 0x0033: // 0xFX33: Store decimal equivalent of V[X] in I
                    dec = chip->V[X];

                    // Extract least sig. digit then cut that digit off
                    for (int i = 2; i >= 0; i--)
                    {
                        chip->memory[chip->I + i] = dec % 10;
                        dec /= 10;
                    }

                    break;

                case 0x0055: // 0xFX55: Store V[0] <-> V[X] at I + (idx of reg)
                    for (i = 0; i <= X; i++)
                        chip->memory[chip->I + i] = chip->V[i];

                    break;

                case 0x0065: // 0xFX65: Load V[0] <-> V[X] at I + (idx of reg)
                    for (i = 0; i <= X; i++)
                        chip->V[i] = chip->memory[chip->I + i];

                    break;

                default:
                    printf("Uknown opcode: 0x%X\n", chip->op);
                    break;
            }
            break;

        default:
            printf("Uknown opcode: 0x%X\n", chip->op);
            break;
    }

    // Take care of timers
    if (chip->delay > 0) 
        chip->delay--;

    if (chip->sound > 0)
    {
        if (chip->sound == 1)
        {
            printf("Beep\n");
        }
        chip->sound--;
    }
}
