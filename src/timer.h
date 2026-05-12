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
    uint16_t divider;              // Internal dot-cycle divider; DIV reads the upper byte.
    uint8_t  reg_tima;             // timer counter ff05
    uint8_t  reg_tma;              // timer modulo ff06
    uint8_t  reg_tac;              // timer control ff07
    bool     tima_overflow_pending;
    uint8_t  tima_overflow_dots;

    // Method pointers
    void (*add_time)(struct Timer*, uint8_t);
    void (*step)(struct Timer*, uint8_t);
    uint8_t (*read_register)(struct Timer*, uint16_t address);
    void (*write_register)(struct Timer*, uint16_t address, uint8_t value);
    void (*refresh_timer_register)(struct Timer*);
    void (*set_timer_register)(struct Timer*);

    // RAM
    struct Ram* ram;
};

// Function declarations
void          timer_add_time(struct Timer* self, uint8_t cycle);
void          timer_step(struct Timer* self, uint8_t m_cycles);
uint8_t       timer_read_register(struct Timer* self, uint16_t address);
void          timer_write_register(struct Timer* self, uint16_t address, uint8_t value);
void          timer_refresh_register(struct Timer* self);
void          timer_set_register(struct Timer* self);
struct Timer* create_timer(void);
void          free_timer(struct Timer* timer);
void          timer_attach_ram(struct Timer* self, struct Ram* ram);

#endif
