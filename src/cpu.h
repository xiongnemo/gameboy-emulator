#ifndef GAMEBOY_CPU_H
#define GAMEBOY_CPU_H

#include "mmu.h"
#include "register.h"

extern struct EmulatorConfig config;

// CPU debug print
#define CPU_DEBUG_PRINT(fmt, ...)                                 \
    if (config.debug_mode && config.verbose_level >= DEBUG_LEVEL) \
    {                                                             \
        PRINT_TIME_IN_SECONDS();                                  \
        PRINT_LEVEL(DEBUG_LEVEL);   \
        printf("CPU: ");            \
        printf(fmt, ##__VA_ARGS__); \
    }

#define CPU_INFO_PRINT(fmt, ...)    \
    if (config.debug_mode && config.verbose_level >= INFO_LEVEL) \
    {                                                            \
        PRINT_TIME_IN_SECONDS();                                 \
        PRINT_LEVEL(INFO_LEVEL);                                 \
        printf("CPU: ");                                         \
        printf(fmt, ##__VA_ARGS__);                              \
    }

#define CPU_TRACE_PRINT(fmt, ...)                                 \
    if (config.debug_mode && config.verbose_level >= TRACE_LEVEL) \
    {                                                             \
        PRINT_TIME_IN_SECONDS();                                  \
        PRINT_LEVEL(TRACE_LEVEL);                                 \
        printf("CPU: ");                                          \
        printf(fmt, ##__VA_ARGS__);                               \
    }

#define CPU_WARN_PRINT(fmt, ...)                                 \
    if (config.debug_mode && config.verbose_level >= WARN_LEVEL) \
    {                                                            \
        PRINT_TIME_IN_SECONDS();                                 \
        PRINT_LEVEL(WARN_LEVEL);                                 \
        printf("CPU: ");                                         \
        printf(fmt, ##__VA_ARGS__);                              \
    }

// CPU clock speed: 4.194304 MHz
#define CPU_CLOCK_SPEED 4194304

// Interrupt flags
#define INT_VBLANK 0x01
#define INT_LCD_STAT 0x02
#define INT_TIMER 0x04
#define INT_SERIAL 0x08
#define INT_JOYPAD 0x10

struct CPU
{
    // Registers
    struct Registers *registers;

    // Memory Management Unit
    struct MMU *mmu;

    // CPU state
    bool halted;  // CPU is halted
    bool stopped; // CPU is stopped
    bool ime;     // Interrupt Master Enable flag

    // Clock management
    uint32_t cycles; // Current cycle count

    // Method pointers
    int (*cpu_step)(struct CPU *);                            // Execute one instruction
    uint8_t (*cpu_read_byte)(struct CPU *, uint16_t);         // Read byte from memory
    void (*cpu_write_byte)(struct CPU *, uint16_t, uint8_t);  // Write byte to memory
    uint16_t (*cpu_read_word)(struct CPU *, uint16_t);        // Read word from memory
    void (*cpu_write_word)(struct CPU *, uint16_t, uint16_t); // Write word to memory
    void (*cpu_handle_interrupts)(struct CPU *);              // Handle interrupts
};

// Function declarations
struct CPU *create_cpu(struct Registers *registers, struct MMU *mmu);
void free_cpu(struct CPU *cpu);
int cpu_step(struct CPU *cpu);
uint8_t cpu_read_byte(struct CPU *cpu, uint16_t address);
void cpu_write_byte(struct CPU *cpu, uint16_t address, uint8_t value);
uint16_t cpu_read_word(struct CPU *cpu, uint16_t address);
void cpu_write_word(struct CPU *cpu, uint16_t address, uint16_t value);
void cpu_handle_interrupts(struct CPU *cpu);

#endif
