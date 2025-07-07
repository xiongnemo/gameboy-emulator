#ifndef GAMEBOY_FORM_H
#define GAMEBOY_FORM_H

#include "general.h"
#include "joypad.h"
#include "ppu.h"
#include <SDL3/SDL.h>

extern struct EmulatorConfig config;

// Form debug print
#define FORM_DEBUG_PRINT(fmt, ...)                                  \
    if (config.debug_mode && config.verbose_level >= DEBUG_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                    \
        PRINT_LEVEL(DEBUG_LEVEL);                                   \
        printf("FOM: ");                                            \
        printf(fmt, ##__VA_ARGS__);                                 \
    }

#define FORM_INFO_PRINT(fmt, ...)                                  \
    if (config.debug_mode && config.verbose_level >= INFO_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                   \
        PRINT_LEVEL(INFO_LEVEL);                                   \
        printf("FOM: ");                                           \
        printf(fmt, ##__VA_ARGS__);                                \
    }

#define FORM_TRACE_PRINT(fmt, ...)                                  \
    if (config.debug_mode && config.verbose_level >= TRACE_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                    \
        PRINT_LEVEL(TRACE_LEVEL);                                   \
        printf("FOM: ");                                            \
        printf(fmt, ##__VA_ARGS__);                                 \
    }

#define FORM_WARN_PRINT(fmt, ...)                                  \
    if (config.debug_mode && config.verbose_level >= WARN_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                   \
        PRINT_LEVEL(WARN_LEVEL);                                   \
        printf("FOM: ");                                           \
        printf(fmt, ##__VA_ARGS__);                                \
    }

#define FORM_ERROR_PRINT(fmt, ...)  \
    {                               \
        PRINT_TIME_IN_SECONDS();    \
        PRINT_LEVEL(ERROR_LEVEL);   \
        printf("FOM: ");            \
        printf(fmt, ##__VA_ARGS__); \
    }

#define FORM_EMERGENCY_PRINT(fmt, ...) \
    {                                  \
        PRINT_TIME_IN_SECONDS();       \
        PRINT_LEVEL(EMERGENCY_LEVEL);  \
        printf("FOM: ");               \
        printf(fmt, ##__VA_ARGS__);    \
    }

// Form struct
struct Form
{
    SDL_Window*   window;
    SDL_Renderer* renderer;
    SDL_Surface*  surface;
    SDL_Event*    event;

    // framebuffer
    uint8_t* framebuffer;

    // PPU
    struct PPU* ppu;

    // Joypad
    struct Joypad* joypad;

    // method pointers
    void (*update_surface)(struct Form*);
    bool (*get_joypad_state)(struct Form*);
};

// Function declarations

// Form functions

// Create form
struct Form* create_form(struct PPU* ppu, struct Joypad* joypad, char* rom_name);

// Free form
void free_form(struct Form* form);

// Update surface
void update_surface(struct Form* form);

// Get Joypad state
bool get_joypad_state(struct Form* form);

// Dump framebuffer to BMP file
void dump_framebuffer_to_bmp(struct Form* form, const char* filename);

// Get current window display mode
const SDL_DisplayMode* get_current_window_display_mode(struct Form* form);

#endif