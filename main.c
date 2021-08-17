
#include <stdio.h>
#include "chip8.h"
#include <SDL2/SDL.h>

static int height = 32;
static int width = 64;

int main(int argc, char **argv)
{
    CHIP8 chip;
    FILE *program_file;
    BYTE program_size, *program;
    SDL_Event event;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Rect rect = {0, 0, 100, 100};
    int i, j, running = 1;

    // Set up graphics
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(width, height, 0, &window, &renderer);

    // TODO: Set up input


    // Initialize chip
    init_chip(&chip);

    // Open ROM program file, get it's size
    program_file = fopen("IBM Logo.ch8", "rb");
    fseek(program_file, 0, SEEK_END);
    program_size = ftell(program_file);
    rewind(program_file);

    // Allocate the array to hold the program
    program = calloc(program_size, sizeof(BYTE));

    // Rewind file pointer and read into the program array
    fread(program, sizeof(BYTE), program_size, program_file);

    // Load program into memory
    for (i = 0; i < program_size; i++)
    {
        chip.memory[i + 0x200] = program[i];
        // printf("0x%02X\t", program[i]);
        // if ((i+1) % 8 == 0)
        //     printf("\n");
    }

    // printf("\n");



    while (running)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        while(SDL_PollEvent(&event)) 
            if (event.type == SDL_QUIT) 
                running = 0;

        cycle(&chip);

        // If draw flag is set, update screen
        for (i = 0; i < 64; i++)
        {
            for (j = 0; j < 32; j++)
            {
                if (chip.screen[IX(i,j)] == 1)
                {
                    SDL_RenderDrawPoint(renderer, i, j);
                    SDL_RenderPresent(renderer);
                }

            }
        }
    
        // TODO: Store key press state (press and release)
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
