/* 
    A CHIP8 Emulator (interpreter?) written in C.
    Alexander Peterson
    August 19 2021

    Uses SDL for graphics/audio/input, and therefore are required to run.
*/

#include "sdltools.h"


const int FPS = 60;
const int frame_delay = 1000 / FPS;

int main(int argc, char **argv)
{
    CHIP8 chip;
    char file_name[16];
    FILE *program_file;
    int i, running;
    uint64_t start, end;

    SDL_Event event;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Rect rects[WIDTH * HEIGHT];

    SDL_AudioSpec wav_spec;
    SDL_AudioDeviceID device_ID;
    uint32_t wav_length;
    uint8_t *wav_buffer;

    // If no program specified, exit
    if (argc > 1)
        strcpy(file_name, argv[1]);
    else
    {
        fprintf(stderr, "File not specified, try again.\n");
        exit(-1);
    }

    // Set up graphics
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    SDL_CreateWindowAndRenderer(window_width, 
                                window_height, 
                                0, 
                                &window, 
                                &renderer);

    // Set up audio
    SDL_LoadWAV("rsrcs/buzz.wav", &wav_spec, &wav_buffer, &wav_length);
    device_ID = SDL_OpenAudioDevice(NULL, 0, &wav_spec, NULL, 0);

    // Fill rects with SDL rectangles, these will represent the 64*32 pixels
    fill_rects(rects);

    // Initialize chip
    init_chip(&chip);

    // Load program, if failed, exit
    if (!load_program(&chip, 0x200, program_file, file_name))
    {
        fprintf(stderr, "File not found, try again.\n");
        exit(-1);
    }

    // Emulation loop
    running = 1;
    while (running)
    {
        start = SDL_GetTicks();

        // Inputs (keypad, quit, debug info)
        get_inputs(&chip, &running);

        // Run five CPU cycles
        for (i = 0; i < 5; i++)
            cycle(&chip);

        update_screen(&chip, renderer, rects);
        play_sound(&chip, device_ID, wav_length, wav_buffer);

        // Limit to 60 FPS
        end = SDL_GetTicks();
        if (frame_delay > (end - start))
        {
            SDL_Delay(frame_delay - (end - start));
        }
    }

    // Cleanup and get out
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_CloseAudioDevice(device_ID);
    SDL_FreeWAV(wav_buffer);
    SDL_Quit();


    return 0;
}
