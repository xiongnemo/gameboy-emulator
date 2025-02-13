#ifndef GAMEBOY_JOYPAD_H
#define GAMEBOY_JOYPAD_H

#include "general.h"
#include "mmu.h"

#define JOYPAD_ADDRESS 0xFF00
#define IF_ADDRESS 0xFF0F

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
    uint8_t key_column;
    bool column_direction;
    bool column_controls;
    
    // Default to 0b0000_1111 for direction keys
    uint8_t keys_directions;
    // Default to 0b0000_1111 for control keys  
    uint8_t keys_controls;
    uint8_t temp_ff00;
    
    uint8_t save_flag;
    uint8_t load_flag;
    // uint8_t fast_forward_flag;

    // Form
    struct Form* form;

    // MMU
    struct MMU* mmu;

    // Method pointers
    void (*joypad_interrupts)(struct Joypad*);
    void (*write_result)(struct Joypad*);
    void (*reset_joypad)(struct Joypad*);
};

// Joypad functions
// Joypad interrupts
void joypad_interrupts(struct Joypad* joypad);

// Write result
void write_result(struct Joypad* joypad);

// Reset joypad
void reset_joypad(struct Joypad* joypad);

// Create joypad
struct Joypad* create_joypad(struct MMU* mmu);

// Free joypad
void free_joypad(struct Joypad* joypad);

#endif