
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <SDL2/SDL.h>

#define WIDTH 64
#define HEIGHT 32

typedef uint8_t BYTE;
typedef uint16_t WORD;

typedef struct CHIP8
{
    WORD stack[0x10];           // 16-bit stack with max length of 16
    BYTE V[0x10];               // 16 8-bit registers
    BYTE memory[0x1000];        // 8-bit memory with 2048 addressable locations
    BYTE screen[WIDTH*HEIGHT];  // 64*32 resolution, each pixel is 1 or 0
    BYTE keys[0x10];            // Hex-based input
    WORD op;                    // Current opcode
    WORD pc;                    // Program counter
    WORD I;                     // Index register
    int16_t sp;                 // Stack pointer
    BYTE delay;                 // Delay timer (both timers count down)
    BYTE sound;                 // Sound timer
    BYTE draw;
} CHIP8;


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

// Most fields in the CHIP8 structure should be set to 0 on allocation, but
// some special initializations are needed (program counter and fontset)
// and while we are here, it does not hurt to initialize everything else
// to zero.
void init_chip(CHIP8 *chip)
{
    int i, j;

    chip->pc = 0x200;        // PC starts at 0x200
    chip->op = 0;            // Reset opcode
    chip->I = 0;             // Reset index register
    chip->sp = -1;           // Reset stack pointer
    chip->delay = 0;         // Reset timers
    chip->sound = 0;

    // Reset display
    for (i = 0; i < WIDTH*HEIGHT; i++)
        chip->screen[i] = 0;

    // Reset stack and registers
    for (i = 0; i < 0x10; i++)
    {
        chip->stack[i] = 0;
        chip->V[i] = 0;
    }

    // Reset memory
    for (i = 0; i < 0x1000; i++)
        chip->memory[i] = 0;

    // Load fonts
    for (i = 0; i < 0x50; i++)
        chip->memory[i] = fonts[i];
}
