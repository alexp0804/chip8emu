
#include <time.h>
#include <stdlib.h>

typedef unsigned char BYTE;
typedef unsigned short WORD;

BYTE fonts[0x50] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0x90, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

typedef struct CHIP8
{
    WORD stack[0x10];    // 16-bit stack with max length of 16
    BYTE V[0x10];        // 16 8-bit registers
    BYTE memory[0x1000]; // 8-bit memory with 2048 addressable locations
    BYTE screen[0x800];  // 64*32 resolution, each pixel is 1 or 0
    BYTE keys[0x10];     // Hex-based input
    WORD op;             // Current opcode
    BYTE I;              // Index register
    BYTE pc;             // Program counter
    BYTE sp;             // Stack pointer
    BYTE delay;          // Delay timer (both timers count downwards to zero)
    BYTE sound;          // Sound timer
} CHIP8;

// Most fields in the CHIP8 structure should be set to 0 on allocation, but
// some special initializations are needed (program counter and fontset)
// and while we are here, it does not hurt to initialize everything else
// to zero.
void init_chip(CHIP8 *chip)
{
    int i;

    chip->pc = (BYTE)0x200; // PC starts at 0x200
    chip->op = 0;           // Reset opcode
    chip->I = 0;            // Reset index register
    chip->sp = 0;           // Reset stack pointer
    chip->delay = 0;        // Reset timers
    chip->sound = 0;

    // Reset display
    for (i = 0; i < 0x800; i++)
    {
        chip->screen[i] = 0;
    }

    // Reset stack and registers
    for (i = 0; i < 0x10; i++)
    {
        chip->stack[i] = 0;
        chip->V[i] = 0;
    }

    // Reset memory
    for (i = 0; i < 0x1000; i++)
    {
        chip->memory[i] = 0;
    }

    // Load fonts
    for (i = 0; i < 0x50; i++)
    {
        chip->memory[i] = fonts[i];
    }
}

// Convert an x,y coordinate into a one-dimensional number for writing/reading from the screen
BYTE IX(x, y)
{
    return 
}

void fetch(CHIP8 *chip)
{
    // Opcodes are words, stored in memory at PC and PC + 1.
    // Shift (PC) and or with (PC + 1) to get opcode.
    chip->op = (chip->memory[chip->pc] << 0x8) | (chip->memory[chip->pc + 1]);
    chip->pc += 2;
}

void cycle(CHIP8 *chip)
{
    srand(time(0));
    WORD x, y, i, j, dec;
    BYTE X, Y, N, NN, NNN, key;
    BYTE sprite_byte, sprite_bit;

    // The values X, Y, N, NN, and NNN are always extracted the same way,
    // so extracting them here will save a lot of code.
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

    // Get next instruction
    fetch(chip);

    // Start with just the most significant nibble.
    // We can identify *most* instructions with just this information.
    switch (chip->op & 0xF000)
    {
        case 0x0000: // 0x00EN
            switch(chip->op & 0x000F) 
            {
                case 0x0000: // 0x00E0: Clear screen
                    for (i = 0; i < 0x800; i++)
                        chip->screen[i] = 0;

                    break;

                case 0x000E: // 0x00EE: Return from sbr
                    chip->pc = chip->stack[chip->sp];
                    chip->sp--;
                    break;

                default:
                    printf("Unkown opcode: 0x%X\n", chip->op);
            }
            break;

        case 0x1000: // 0x1NNN: Jump to NNN
            chip->pc = NNN;
            break;

        case 0x2000: // 0x2NNN: Execute sbr at NNN
            // Push current PC onto stack
            chip->stack[chip->sp] = chip->pc;
            chip->sp++;

            // Jump to NNN
            chip->pc = NNN;
            break;

        case 0x3000: // 0x3XNN: Skip if V[X] != NN
            if (chip->V[X] != NN)
                chip->pc += 2;

            break;

        case 0x4000: // 0x4XNN: Skip if V[X] == NN
            if (chip->V[X] == NN)
                chip->pc += 2;

            break;

        case 0x5000: // 0x5XY0: Skip if V[X] != V[Y]
            if (chip->V[X] != chip->V[Y])
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
                
                case 0x0004: // 0x8XY4: TODO Add V[Y] to V[X], set flags
                    break;
                
                case 0x0005: // 0x8XY5: TODO Subtract V[Y] from V[X], set flags
                    break;
                
                case 0x0006: // 0x8XY6: TODO Store (V[Y] >> 1) in V[X], set flags
                    break;
                
                case 0x0007: // 0x8XY7: TODO Set V[X] to V[Y] - V[X], set flags
                    break;
                
                case 0x000E: // 0x8XYE: TODO Store (V[Y] << 1) in V[x], set flags
                    break;
                
                default:
                    printf("Unkown opcode: 0x%X\n", chip->op);
            }

        case 0x9000: // 0x9XY0: Skip if V[X] == V[Y]
            if (chip->V[X] == chip->V[Y]) { chip->pc += 2; }
            break;

        case 0xA000: // 0xANNN: Store NNN in I
            chip->I = NNN;
            break;
        
        case 0xB000: // 0xBNNN: Jump to NNN + V[0]
            chip->pc = NNN + chip->V[0];
            break;

        case 0xC000: // 0xCXNN: Set V[X] to random with mask NN
            chip->V[X] = (rand() % 256) & NN;
            break;

        case 0xD000: // 0xDXYN: Draw pixels
            // Get x,y coordinate of the sprite
            // Modulo by screen width and height to stay within bounds
            int x = chip->V[X] % 64, y = chip->V[Y] % 32;            

            // Clear flag register
            chip->V[0xF] = 0;

            // For N rows
            for (i = 0; i < N; i++)
            {
                // Get the sprite byte addressed by I
                sprite_byte = chip->memory[chip->I];
                
                // For each bit in that byte
                for (j = 0; j < 8; j++)
                {
                    // 1 or 0, indicating if the current bit (starting from 7 through 0) is set
                    sprite_bit = (sprite_byte & (1 << (7 - i))) >> (7 - i);

                    // If this bit is set AND the pixel at x,y is set turn the pixel off and set VF
                    // This is essentially modelling a collision detection
                    if (sprite_bit && chip->screen[IX(x, y)])
                    {
                        chip->screen[IX(x, y)] = 0;
                        chip->V[0xF] = 1;
                    }

                    // If the sprite bit is on but the pixel isn't, that means we need to draw it to the screen
                    if (sprite_bit && !chip->screen[IX(x, y)])
                        chip->screen[IX(x, y)] = 1;

                    // If we are off the side of the screen quit drawing
                    if (x >= 64)
                        break;
            
                    x++;
                }

                // If we are off the bottom of the screen quit drawing
                if (y >= 32)
                    break;

                y++;
            }
            break;

        case 0xE000:
            switch (chip->op & 0x00FF)
            {
                case 0x009E: // 0xEX9E: If the key represented by the value in V[X] is pressed, skip
                    key = chip->V[X];

                    if (chip->keys[key] == 1)
                        chip->pc += 2;

                    break;
                
                case 0x00A1: // 0xEXA1: If the key represented by the value in V[X] isn't pressed, skip
                    key = chip->V[X];

                    if (chip->keys[key] == 0)
                        chip->pc += 2;

                    break;

                default:
                    printf("Uknown opcode: 0x%X\n", chip->op);
            }
            break;

        case 0xF000:
            switch (chip->op & 0x00FF)
            {
                case 0x0007: // 0xFX15: Set V[X] to delay timer
                    chip->V[X] = chip->delay;
                    break;

                case 0x000A: // 0xFX0A: Stops PC increment unless a key is pressed
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

                case 0x0033: // 0xFX33: Store decimal equivalent of V[X] in I, I+1, I+2
                    dec = chip->V[X];

                    // Extract least sig. digit then divide by 10 to cut that digit off
                    for (i = 2; i >= 0; i--)
                    {
                        chip->memory[chip->I + i] = (dec % 10);
                        dec /= 10;
                    }
                    break;

                case 0x0055: // 0xFX55: Store V[0] through V[X] in memory at I + (idx of reg)
                    for (i = 0; i <= X; i++)
                        chip->memory[chip->I + i] = chip->V[i];

                    break;

                case 0x0065: // 0xFX65: Load V[0] through V[X] with memory at I + (idx of reg)
                    for (i = 0; i < X; i++)
                        chip->V[i] = chip->memory[chip->I + i];

                    break;

                default:
                    printf("Uknown opcode: 0x%X\n", chip->op);
            }


        default:
            printf("Uknown opcode: 0x%X\n", chip->op);
    }

    // Take care of timers
    if (chip->delay > 0) chip->delay--;

    if (chip->sound > 0)
    {
        if (chip->sound == 1)
        {
            printf("Beep\n");
        }
        chip->sound--;
    }
}