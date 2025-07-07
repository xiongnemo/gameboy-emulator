#ifndef GAMEBOY_DMG_H
#define GAMEBOY_DMG_H

#include "cpu.h"
#include "form.h"
#include "mmu.h"
#include "ppu.h"
#include "timer.h"

extern struct EmulatorConfig config;

// DMG debug print
#define DMG_DEBUG_PRINT(fmt, ...)                                   \
    if (config.debug_mode && config.verbose_level >= DEBUG_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                    \
        PRINT_LEVEL(DEBUG_LEVEL);                                   \
        printf("DMG: ");                                            \
        printf(fmt, ##__VA_ARGS__);                                 \
    }

#define DMG_INFO_PRINT(fmt, ...)                                   \
    if (config.debug_mode && config.verbose_level >= INFO_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                   \
        PRINT_LEVEL(INFO_LEVEL);                                   \
        printf("DMG: ");                                           \
        printf(fmt, ##__VA_ARGS__);                                \
    }

#define DMG_TRACE_PRINT(fmt, ...)                                   \
    if (config.debug_mode && config.verbose_level >= TRACE_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                    \
        PRINT_LEVEL(TRACE_LEVEL);                                   \
        printf("DMG: ");                                            \
        printf(fmt, ##__VA_ARGS__);                                 \
    }

#define DMG_WARN_PRINT(fmt, ...)                                   \
    if (config.debug_mode && config.verbose_level >= WARN_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                   \
        PRINT_LEVEL(WARN_LEVEL);                                   \
        printf("DMG: ");                                           \
        printf(fmt, ##__VA_ARGS__);                                \
    }

#define DMG_ERROR_PRINT(fmt, ...)   \
    {                               \
        PRINT_TIME_IN_SECONDS();    \
        PRINT_LEVEL(ERROR_LEVEL);   \
        printf("DMG: ");            \
        printf(fmt, ##__VA_ARGS__); \
    }

#define DMG_EMERGENCY_PRINT(fmt, ...) \
    {                                 \
        PRINT_TIME_IN_SECONDS();      \
        PRINT_LEVEL(EMERGENCY_LEVEL); \
        printf("DMG: ");              \
        printf(fmt, ##__VA_ARGS__);   \
    }


void initialize_ram(struct Ram* ram);
// New: we now generate (ensure we have a new frame) each frame in a separate function
void next_frame(struct PPU* ppu, struct CPU* cpu, int current_frame);
// Main loop
void main_loop(struct PPU* ppu, struct CPU* cpu, struct Timer* timer, struct Form* form);

#endif
