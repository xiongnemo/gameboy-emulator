#ifndef GAMEBOY_RAM_H
#define GAMEBOY_RAM_H
#include "cartridge.h"

#include "general.h"

extern struct EmulatorConfig config;

// RAM debug print
#define RAM_DEBUG_PRINT(fmt, ...)                                   \
    if (config.debug_mode && config.verbose_level >= DEBUG_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                    \
        PRINT_LEVEL(DEBUG_LEVEL);                                   \
        printf("RAM: ");                                            \
        printf(fmt, ##__VA_ARGS__);                                 \
    }

#define RAM_INFO_PRINT(fmt, ...)                                   \
    if (config.debug_mode && config.verbose_level >= INFO_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                   \
        PRINT_LEVEL(INFO_LEVEL);                                   \
        printf("RAM: ");                                           \
        printf(fmt, ##__VA_ARGS__);                                \
    }

#define RAM_TRACE_PRINT(fmt, ...)                                   \
    if (config.debug_mode && config.verbose_level >= TRACE_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                    \
        PRINT_LEVEL(TRACE_LEVEL);                                   \
        printf("RAM: ");                                            \
        printf(fmt, ##__VA_ARGS__);                                 \
    }

#define RAM_WARN_PRINT(fmt, ...)                                   \
    if (config.debug_mode && config.verbose_level >= WARN_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                   \
        PRINT_LEVEL(WARN_LEVEL);                                   \
        printf("RAM: ");                                           \
        printf(fmt, ##__VA_ARGS__);                                \
    }

#define RAM_ERROR_PRINT(fmt, ...)   \
    {                               \
        PRINT_TIME_IN_SECONDS();    \
        PRINT_LEVEL(ERROR_LEVEL);   \
        printf("RAM: ");            \
        printf(fmt, ##__VA_ARGS__); \
    }

#define RAM_EMERGENCY_PRINT(fmt, ...) \
    {                                 \
        PRINT_TIME_IN_SECONDS();      \
        PRINT_LEVEL(EMERGENCY_LEVEL); \
        printf("RAM: ");              \
        printf(fmt, ##__VA_ARGS__);   \
    }

#define RAM_SIZE 0xFFFF   // 64KB RAM

// list of important ram register addresses

struct Ram
{
    // Data members
    uint8_t ram_byte[RAM_SIZE];   // Entire Address Bus: 64 KB

    // Method pointers
    uint8_t (*get_ram_byte)(struct Ram*, uint16_t);
    void (*set_ram_byte)(struct Ram*, uint16_t, uint8_t);
    uint16_t (*get_ram_word)(struct Ram*, uint16_t);
    void (*set_ram_word)(struct Ram*, uint16_t, uint16_t);
};

// Function declarations
// setters and getters

// get byte at address
uint8_t ram_get_byte(struct Ram* self, uint16_t address);

// set byte at address
void ram_set_byte(struct Ram* self, uint16_t address, uint8_t byte);

// get word at address
uint16_t ram_get_word(struct Ram* self, uint16_t address);

// set word at address
void ram_set_word(struct Ram* self, uint16_t address, uint16_t word);

// create ram struct
struct Ram* create_ram(void);

// free ram struct
void free_ram(struct Ram* self);

#endif
