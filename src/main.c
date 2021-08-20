
#include "sdltools.h"

int main(int argc, char **argv)
{
    CHIP8 chip;

    int i, j;
    int success, color, running = 1, idx;
    int debugging, next_frame = 0, continuous_frames = 0;

    SDL_Event event;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Rect rects[WIDTH * HEIGHT];
    uint64_t start, end;
    float passed, delay;

    char file_name[16];
    FILE *program_file;
    unsigned int start_addr = 0x200;

    // If no program specified, exit
    if (argc > 1)
    {
        strcpy(file_name, argv[1]);
        debugging = (argc == 3);
    }
    else
    {
        fprintf(stderr, "File not found, try again.\n");
        exit(-1);
    }

    // Set up graphics
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(window_width, 
                                window_height, 
                                0, 
                                &window, 
                                &renderer);

    // Fill rects with SDL rectangles, these will represent the 64*32 pixels
    fill_rects(rects);

    // Initialize chip
    init_chip(&chip);

    // Load program, if failed, exit
    if (!load_program(&chip, start_addr, program_file, file_name))
        return 0;

    // Emulation loop
    while (running)
    {
        start = SDL_GetPerformanceCounter();

        // Inputs (keypad, quit, debug info)
        get_inputs(&chip, 
                   &running,
                   &next_frame, 
                   &continuous_frames);

        // Run a single CPU cycle
        if (debugging)
        {
            if (next_frame || continuous_frames)
            {
                debug_info(&chip);
                cycle(&chip);
            }

            next_frame = 0;
        }
        else 
            cycle(&chip);

        update_screen(&chip, renderer, rects);

        // Limit FPS to 60
        end = SDL_GetPerformanceCounter();
        passed = (end - start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;
        delay = 16.666f - passed;
        if (delay >= 0)
            SDL_Delay(floor(16.666f - passed));
    }

    // Cleanup and go
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
