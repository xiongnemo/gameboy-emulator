#ifndef GAMEBOY_CARTRIDGE_H
#define GAMEBOY_CARTRIDGE_H

#include "general.h"

#define ROM_SIZE      524288
#define ROM_NAME_SIZE 16

extern struct EmulatorConfig config;

// Cartridge debug print with time
#define CARTRIDGE_DEBUG_PRINT(fmt, ...)                             \
    if (config.debug_mode && config.verbose_level >= DEBUG_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                    \
        PRINT_LEVEL(DEBUG_LEVEL);                                   \
        printf("CAR: ");                                            \
        printf(fmt, ##__VA_ARGS__);                                 \
    }

#define CARTRIDGE_INFO_PRINT(fmt, ...)                             \
    if (config.debug_mode && config.verbose_level >= INFO_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                   \
        PRINT_LEVEL(INFO_LEVEL);                                   \
        printf("CAR: ");                                           \
        printf(fmt, ##__VA_ARGS__);                                \
    }

#define CARTRIDGE_TRACE_PRINT(fmt, ...)                             \
    if (config.debug_mode && config.verbose_level >= TRACE_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                    \
        PRINT_LEVEL(TRACE_LEVEL);                                   \
        printf("CAR: ");                                            \
        printf(fmt, ##__VA_ARGS__);                                 \
    }

#define CARTRIDGE_WARN_PRINT(fmt, ...)                             \
    if (config.debug_mode && config.verbose_level >= WARN_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                   \
        PRINT_LEVEL(WARN_LEVEL);                                   \
        printf("CAR: ");                                           \
        printf(fmt, ##__VA_ARGS__);                                \
    }

#define CARTRIDGE_ERROR_PRINT(fmt, ...) \
    {                                   \
        PRINT_TIME_IN_SECONDS();        \
        PRINT_LEVEL(ERROR_LEVEL);       \
        printf("CAR: ");                \
        printf(fmt, ##__VA_ARGS__);     \
    }

#define CARTRIDGE_EMERGENCY_PRINT(fmt, ...) \
    {                                       \
        PRINT_TIME_IN_SECONDS();            \
        PRINT_LEVEL(EMERGENCY_LEVEL);       \
        printf("CAR: ");                    \
        printf(fmt, ##__VA_ARGS__);         \
    }

// Gameboy cartridge type address
#define GAMEBOY_CARTRIDGE_TYPE_ADDRESS 0x0147
// Gameboy ROM size address
#define GAMEBOY_ROM_SIZE_ADDRESS 0x0148
// Gameboy RAM size address
#define GAMEBOY_RAM_SIZE_ADDRESS 0x0149
// Gameboy bank size
#define GAMEBOY_BANK_SIZE 0x4000
// Gameboy ROM name address
#define GAMEBOY_ROM_NAME_ADDRESS 0x0134
// Gameboy MBC1 magic number start address
#define GAMEBOY_CARTRIDGE_MBC1_MAGIC_NUMBER_START_ADDRESS 0x1FFF
// Gameboy MBC1 magic number end address
#define GAMEBOY_CARTRIDGE_MBC1_MAGIC_NUMBER_END_ADDRESS 0x4000

struct Cartridge
{
    uint8_t* rom_bank_0;
    uint8_t* rom_bank_1;
    uint8_t* ram;

    // max possible size of ROM is 512kb
    uint8_t rom[ROM_SIZE];
    // +1 for null terminator
    uint8_t rom_name[ROM_NAME_SIZE + 1];

    // alternative ROM bank id
    uint8_t rom_alternative_bank;
    // alternative RAM bank id
    uint8_t ram_alternative_bank;
    // ROM bank count
    uint8_t rom_attributes_bank_count;
    // RAM bank count
    uint8_t ram_attributes_bank_count;
    // RAM bank size in kb
    uint8_t ram_attributes_bank_size;   // in kb

    // Method pointers
    uint8_t (*get_cartridge_byte)(struct Cartridge*, uint16_t);
    void (*set_cartridge_byte)(struct Cartridge*, uint16_t, uint8_t);
    uint16_t (*get_cartridge_word)(struct Cartridge*, uint16_t);
    void (*set_cartridge_word)(struct Cartridge*, uint16_t, uint16_t);
    void (*set_rom_bank)(struct Cartridge*, uint8_t);
    void (*set_ram_bank)(struct Cartridge*, uint8_t);
    uint8_t (*get_rom_bank)(struct Cartridge*);
    uint8_t (*get_ram_bank)(struct Cartridge*);
    char* (*get_rom_name)(struct Cartridge*);
    bool (*check_cartridge_type)(struct Cartridge*);

    // Constructor
    struct Cartridge* (*create_cartridge)(void);

    // Destructor
    void (*free_cartridge)(struct Cartridge*);
};

// Function declarations

// load cartridge from ROM file (path)
bool load_cartridge(struct Cartridge* cartridge, const char* rom_path);

// free cartridge
void free_cartridge(struct Cartridge* cartridge);

// set ROM bank
void cartridge_set_rom_bank(struct Cartridge* cartridge, uint8_t bank);

// set RAM bank
void cartridge_set_ram_bank(struct Cartridge* cartridge, uint8_t bank);

// get ROM bank
uint8_t cartridge_get_rom_bank(struct Cartridge* cartridge);

// get RAM bank
uint8_t cartridge_get_ram_bank(struct Cartridge* cartridge);

// get byte at address
uint8_t cartridge_get_rom_byte(struct Cartridge* cartridge, uint16_t address);

// set byte at address
void cartridge_set_rom_byte(struct Cartridge* cartridge, uint16_t address, uint8_t byte);

// get word at address
uint16_t cartridge_get_rom_word(struct Cartridge* cartridge, uint16_t address);

// set word at address
void cartridge_set_rom_word(struct Cartridge* cartridge, uint16_t address, uint16_t word);

// check cartridge type
bool check_cartridge_type(struct Cartridge* cartridge);

// create cartridge
struct Cartridge* create_cartridge();

// get ROM name
char* cartridge_get_rom_name(struct Cartridge* cartridge);

#endif
