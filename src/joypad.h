#ifndef GAMEBOY_JOYPAD_H
#define GAMEBOY_JOYPAD_H

#include "general.h"
#include "mmu.h"

extern struct EmulatorConfig config;

// Joypad debug print
#define JOYPAD_DEBUG_PRINT(fmt, ...)                                  \
    if (config.debug_mode && config.verbose_level >= DEBUG_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                    \
        PRINT_LEVEL(DEBUG_LEVEL);                                   \
        printf("JOY: ");                                            \
        printf(fmt, ##__VA_ARGS__);                                 \
    }

#define JOYPAD_INFO_PRINT(fmt, ...)                                  \
    if (config.debug_mode && config.verbose_level >= INFO_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                   \
        PRINT_LEVEL(INFO_LEVEL);                                   \
        printf("JOY: ");                                           \
        printf(fmt, ##__VA_ARGS__);                                \
    }

#define JOYPAD_TRACE_PRINT(fmt, ...)                                  \
    if (config.debug_mode && config.verbose_level >= TRACE_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                    \
        PRINT_LEVEL(TRACE_LEVEL);                                   \
        printf("JOY: ");                                            \
        printf(fmt, ##__VA_ARGS__);                                 \
    }

struct Joypad {
    // Default to 0b0000_1111 for direction keys (1=not pressed, 0=pressed)
    uint8_t keys_directions;
    // Default to 0b0000_1111 for control keys (1=not pressed, 0=pressed)
    uint8_t keys_controls;
    
    uint8_t save_flag;
    uint8_t load_flag;
    // uint8_t fast_forward_flag;

    // Form
    struct Form* form;

    // MMU
    struct MMU* mmu;

    // Method pointers
    void (*handle_joypad_input)(struct Joypad*);
};

// Joypad functions
// Joypad interrupts
void handle_joypad_input(struct Joypad* joypad);

// Create joypad
struct Joypad* create_joypad(struct MMU* mmu);

// Free joypad
void free_joypad(struct Joypad* joypad);

#endif