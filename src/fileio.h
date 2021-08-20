
#include "cpu.h"

unsigned int get_file_size(FILE *f)
{
    unsigned int size;

    fseek(f, 0L, SEEK_END);
    size = ftell(f);
    fseek(f, 0L, SEEK_SET);

    return size;
}

// Read program from file and load directly into CHIP8 memory at specified address
int load_program(CHIP8 *chip, WORD addr, FILE *f, char name[16])
{
    BYTE *program = chip->memory + addr;
    unsigned int size, bytes_read;
    char file_location[32] = "./roms/\0";

    // Open program file and get it's size
    f = fopen(strcat(file_location, name), "rb");  
    size = get_file_size(f);
    
    // Read file into memory
    bytes_read = fread(program, sizeof(BYTE), size, f);

    fclose(f);

    // If bytes read is the same as number of bytes in the file, success!
    return (bytes_read == size);
}