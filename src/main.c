
#include <SDL2/SDL.h>
#include "readprog.h"

static int height = 320;
static int width = 640;

void fill(SDL_Rect rects[WIDTH*HEIGHT])
{
    int i, j;
    
    for (i = 0; i < WIDTH; i++)
    {
        for (j = 0; j < HEIGHT; j++)
        {
            rects[IX(i, j)].w = width / WIDTH;
            rects[IX(i, j)].h = height / HEIGHT;
            rects[IX(i, j)].x = i * rects[IX(i, j)].w;
            rects[IX(i, j)].y = j * rects[IX(i, j)].h;
        }
    }
}

int main(int argc, char **argv)
{
    CHIP8 chip;
    int i, j, running = 1, color;

    SDL_Event event;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Rect rects[WIDTH * HEIGHT];

    char file_name[16];
    unsigned int program_size, start_addr = 0x200;
    FILE *program_file;
    BYTE *program;

    if (argc > 1)
        strcpy(file_name, argv[1]);
    else
        fprintf(stderr, "No program file described, try again\n");


    // Set up graphics
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(width, height, 0, &window, &renderer);

    // Fill the rects array with SDL rectangles, these will represent the 64*32 pixels
    fill(rects);

    // TODO: Set up input

    // Initialize chip
    init_chip(&chip);

    // Open program, read it, load it
    program = read_program(program_file, file_name, &program_size);
    if (!program)
    {
        fprintf(stderr, "Error loading program\n");
        exit(-1);
    }

    load_program(&chip, start_addr, program, program_size);

    // As the program is now in memory, we don't need to keep the allocated array
    free(program);

    // dump_memory(&chip);

    while (running)
    {
        uint64_t start = SDL_GetPerformanceCounter();

        while (SDL_PollEvent(&event))
            if (event.type == SDL_QUIT)
                running = 0;

        // Run a single CPU cycle
        cycle(&chip);

        // Set the rectangles on/off depending on 
        for (i = 0; i < WIDTH; i++)
        {
            for (j = 0; j < HEIGHT; j++)
            {
                color = (chip.screen[IX(i, j)] == 1) ? 255 : 0;
                SDL_SetRenderDrawColor(renderer, color, color, color, 255);
                SDL_RenderFillRect(renderer, &rects[IX(i, j)]);
            }
        }
        // Update screen
        SDL_RenderPresent(renderer);

        uint64_t end = SDL_GetPerformanceCounter();

        // TODO: Store key press state (press and release)

        float elapsed = (end - start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;
        float delay = 16.666f - elapsed;
        // Limit FPS to 60
        if (delay >= 0)
        {
            SDL_Delay(floor(16.666f - elapsed));
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
