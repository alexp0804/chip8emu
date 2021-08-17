
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

typedef uint8_t BYTE;
typedef uint16_t WORD;

#define WIDTH 64
#define HEIGHT 32

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
    BYTE screen[64*32];  // 64*32 resolution, each pixel is 1 or 0
    BYTE keys[0x10];     // Hex-based input
    WORD op;             // Current opcode
    WORD pc;             // Program counter
    WORD I;              // Index register
    BYTE sp;             // Stack pointer
    BYTE delay;          // Delay timer (both timers count downwards to zero)
    BYTE sound;          // Sound timer
    BYTE draw;
} CHIP8;

unsigned int IX(BYTE x, BYTE y)
{
    return y * WIDTH + x;
}

// Most fields in the CHIP8 structure should be set to 0 on allocation, but
// some special initializations are needed (program counter and fontset)
// and while we are here, it does not hurt to initialize everything else
// to zero.
void init_chip(CHIP8 *chip)
{
    int i, j;

    chip->pc = 0x200;       // PC starts at 0x200
    chip->op = 0;           // Reset opcode
    chip->I = 0;            // Reset index register
    chip->sp = 0;           // Reset stack pointer
    chip->delay = 0;        // Reset timers
    chip->sound = 0;

    // Reset display
    for (i = 0; i < 64*32; i++)
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

void fetch(CHIP8 *chip)
{
    // Opcodes are words, stored in memory at PC and PC + 1.
    // Shift (PC) and or with (PC + 1) to get opcode.

    chip->op = (chip->memory[chip->pc] << 0x8) | (chip->memory[chip->pc + 1]);
    chip->pc += 2;
}

void cycle(CHIP8 *chip)
{
    WORD x, y, i, j, dec;
    BYTE X, Y, N, NN, NNN, key, a, b;
    BYTE sprite_byte, sprite_bit;

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
                    for (i = 0; i < 64*32; i++)
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
            chip->stack[chip->sp++] = chip->pc;

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

        case 0x9000: // 0x9XY0: Skip if V[X] == V[Y]
            if (chip->V[X] == chip->V[Y])
                chip->pc += 2;

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

        case 0xD000: // 0xDXYN: TODO Draw pixels
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
                    break;
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
                    break;
            }
            break;

        default:
            printf("Uknown opcode: 0x%X\n", chip->op);
            break;
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