
#include "chip8.h"
#include <sys/stat.h>

// I opted to go for the file descriptor route to get 
// the file size in bytes because using fseek(..., SEEK_END) 
// has undefined behavior when reading binary files
unsigned int file_size(FILE *f)
{
    struct stat stats;

    // Get file descriptor from file stream
    int fd = fileno(f);

    // Get stat structure from the file descriptor, store in stats
    fstat(fd, &stats);

    return stats.st_size;
}

// Checks if the given filename has the valid extension, .ch8
int validate_filename(char name[16])
{
    // Find the first instance of '.' 
    char *extension_location = strchr(name, '.');

    // If not found, file is invalid
    if (!extension_location)
        return 0;

    return (strcmp(extension_location, ".ch8") == 0);
}

// Reads the program file and stores it in the returned BYTE array
BYTE *read_program(FILE *f, char name[16], unsigned int *size)
{
    BYTE *program, read_bytes;
    char file_location[32] = "./roms/\0";

    // Validate that file is type ".ch8"
    if (!validate_filename(name))
        return NULL;

    // Open program file and get it's size
    f = fopen(strcat(file_location, name), "rb");  
    *size = file_size(f);

    // Allocate array for program
    program = calloc(*size, sizeof(BYTE));
    if (!program)
        return NULL;

    // Read from f into program array
    read_bytes = fread(program, sizeof(BYTE), *size, f);
    fclose(f);

    // If the number of bytes read is the same as the size of the file, everything went smoothly and we can return it back
    if (read_bytes == *size)
    {
        return program;
    }

    // If not, something went wrong when reading the file and the program is likely corrupt.
    free(program);
    return NULL;
}

// Loads the program into memory starting at addr
// Assumes addr is a valid address and the program is small enough to fit in memory
void load_program(BYTE *memory, WORD addr, BYTE *program, BYTE prog_size)
{
    if (!memory || !program)
        return;

    for (int i = 0; i < prog_size; i++)
        memory[i + addr] = program[i];
}