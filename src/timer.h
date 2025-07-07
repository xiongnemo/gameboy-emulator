#ifndef GAMEBOY_TIMER_H
#define GAMEBOY_TIMER_H

#include "general.h"
#include "ram.h"

extern struct EmulatorConfig config;

// Timer debug print
#define TIMER_DEBUG_PRINT(fmt, ...)                                 \
    if (config.debug_mode && config.verbose_level >= DEBUG_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                    \
        PRINT_LEVEL(DEBUG_LEVEL);                                   \
        printf("TIM: ");                                            \
        printf(fmt, ##__VA_ARGS__);                                 \
    }

#define TIMER_INFO_PRINT(fmt, ...)                                 \
    if (config.debug_mode && config.verbose_level >= INFO_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                   \
        PRINT_LEVEL(INFO_LEVEL);                                   \
        printf("TIM: ");                                           \
        printf(fmt, ##__VA_ARGS__);                                \
    }

#define TIMER_TRACE_PRINT(fmt, ...)                                 \
    if (config.debug_mode && config.verbose_level >= TRACE_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                    \
        PRINT_LEVEL(TRACE_LEVEL);                                   \
        printf("TIM: ");                                            \
        printf(fmt, ##__VA_ARGS__);                                 \
    }

#define TIMER_WARN_PRINT(fmt, ...)                                 \
    if (config.debug_mode && config.verbose_level >= WARN_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                   \
        PRINT_LEVEL(WARN_LEVEL);                                   \
        printf("TIM: ");                                           \
        printf(fmt, ##__VA_ARGS__);                                \
    }

#define TIMER_ERROR_PRINT(fmt, ...) \
    {                               \
        PRINT_TIME_IN_SECONDS();    \
        PRINT_LEVEL(ERROR_LEVEL);   \
        printf("TIM: ");            \
        printf(fmt, ##__VA_ARGS__); \
    }

#define TIMER_EMERGENCY_PRINT(fmt, ...) \
    {                                   \
        PRINT_TIME_IN_SECONDS();        \
        PRINT_LEVEL(EMERGENCY_LEVEL);   \
        printf("TIM: ");                \
        printf(fmt, ##__VA_ARGS__);     \
    }

struct Timer
{
    // Data members
    uint64_t counter;
    uint64_t divider;
    uint8_t  reg_div;    // register divider ff04
    uint8_t  reg_tima;   // counter ff05
    uint8_t  reg_tma;    // modulator ff06
    uint8_t  reg_tac;    // control ff07

    // Method pointers
    void (*add_time)(struct Timer*, uint8_t);
    void (*refresh_timer_register)(struct Timer*);
    void (*set_timer_register)(struct Timer*);

    // RAM
    struct Ram* ram;
};

// Function declarations
void          timer_add_time(struct Timer* self, uint8_t cycle);
void          timer_refresh_register(struct Timer* self);
void          timer_set_register(struct Timer* self);
struct Timer* create_timer(void);
void          free_timer(struct Timer* timer);
void          timer_attach_ram(struct Timer* self, struct Ram* ram);

#endif
