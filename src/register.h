
#ifndef GAMEBOY_REGISTER_H
#define GAMEBOY_REGISTER_H

#include "general.h"

extern struct EmulatorConfig config;

// Register debug print
#define REGISTER_DEBUG_PRINT(fmt, ...)                            \
    if (config.debug_mode && config.verbose_level >= DEBUG_LEVEL) \
    {                                                             \
        PRINT_TIME_IN_SECONDS();                                  \
        PRINT_LEVEL(DEBUG_LEVEL);                                 \
        printf("REG: ");                                          \
        printf(fmt, ##__VA_ARGS__);                               \
    }

#define REGISTER_INFO_PRINT(fmt, ...)                            \
    if (config.debug_mode && config.verbose_level >= INFO_LEVEL) \
    {                                                            \
        PRINT_TIME_IN_SECONDS();                                 \
        PRINT_LEVEL(INFO_LEVEL);                                 \
        printf("REG: ");                                         \
        printf(fmt, ##__VA_ARGS__);                              \
    }

#define REGISTER_TRACE_PRINT(fmt, ...)                            \
    if (config.debug_mode && config.verbose_level >= TRACE_LEVEL) \
    {                                                             \
        PRINT_TIME_IN_SECONDS();                                  \
        PRINT_LEVEL(TRACE_LEVEL);                                 \
        printf("REG: ");                                          \
        printf(fmt, ##__VA_ARGS__);                               \
    }

#define REGISTER_WARN_PRINT(fmt, ...)                            \
    if (config.debug_mode && config.verbose_level >= WARN_LEVEL) \
    {                                                            \
        PRINT_TIME_IN_SECONDS();                                 \
        PRINT_LEVEL(WARN_LEVEL);                                 \
        printf("REG: ");                                         \
        printf(fmt, ##__VA_ARGS__);                              \
    }

#define REGISTER_ERROR_PRINT(fmt, ...) \
    {                               \
        PRINT_TIME_IN_SECONDS();    \
        PRINT_LEVEL(ERROR_LEVEL);   \
        printf("REG: ");            \
        printf(fmt, ##__VA_ARGS__); \
    }

#define REGISTER_EMERGENCY_PRINT(fmt, ...) \
    {                                   \
        PRINT_TIME_IN_SECONDS();        \
        PRINT_LEVEL(EMERGENCY_LEVEL);   \
        printf("REG: ");                \
        printf(fmt, ##__VA_ARGS__);     \
    }

// Registers
// in LR35902, there are 8 8-bit common registers: A, F, B, C, D, E, H, L
// A: Accumulator
// F: Flags
// B, C: BC
// D, E: DE
// H, L: HL
// and 2 16-bit registers: SP (Stack Pointer), PC (Program Counter)
// We can pair AF, BC, DE, HL to form 16-bit registers for some instructions
enum RegisterPair
{
    AF = 0x00,
    BC = 0x01,
    DE = 0x02,
    HL = 0x03,
};

enum Register
{
    A = 0x00,
    F = 0x01,
    B = 0x02,
    C = 0x03,
    D = 0x04,
    E = 0x05,
    H = 0x06,
    L = 0x07
};

enum ControlRegister
{
    SP = 0x00,
    PC = 0x01
};

// Flags
// Z: Zero flag
// N: Subtract flag
// H: Half Carry flag
// C: Carry flag
// These flags are set by the ALU (Arithmetic Logic Unit)
// and are used to control the execution of the instructions
// They are stored in the F register, in higher 4 bits
enum Flag
{
    Flag_Z = 0x80,
    Flag_N = 0x40,
    Flag_H = 0x20,
    Flag_C = 0x10
};

// Register structure
struct Registers
{
    // CPU 8-bit registers array [A, F, B, C, D, E, H, L]
    uint8_t reg_primary[8];
    // CPU special registers array [SP, PC]
    uint16_t reg_control[2];

    // pointer for quick access
    uint8_t *a;
    uint8_t *f;
    uint8_t *b;
    uint8_t *c;
    uint8_t *d;
    uint8_t *e;
    uint8_t *h;
    uint8_t *l;
    uint16_t *sp;
    uint16_t *pc;

    // Method pointers
    uint8_t (*get_register_byte)(struct Registers *, enum Register);
    uint16_t (*get_register_pair)(struct Registers *, enum RegisterPair);
    void (*set_register_byte)(struct Registers *, enum Register, uint8_t);
    void (*set_register_pair)(struct Registers *, enum RegisterPair, uint16_t);
    uint16_t (*get_control_register)(struct Registers *, enum ControlRegister);
    void (*set_control_register)(struct Registers *, enum ControlRegister, uint16_t);
    uint8_t (*get_flag)(struct Registers *, enum Flag);
    bool (*get_flag_z)(struct Registers *);
    bool (*get_flag_n)(struct Registers *);
    bool (*get_flag_h)(struct Registers *);
    bool (*get_flag_c)(struct Registers *);
    void (*set_flag)(struct Registers *, enum Flag, bool);
    void (*set_flag_z)(struct Registers *, bool);
    void (*set_flag_n)(struct Registers *, bool);
    void (*set_flag_h)(struct Registers *, bool);
    void (*set_flag_c)(struct Registers *, bool);
};

// Register methods
struct Registers *create_registers();
void free_registers(struct Registers *registers);
uint8_t get_register_byte(struct Registers *registers, enum Register reg);
uint16_t get_register_pair(struct Registers *registers, enum RegisterPair reg);
void set_register_byte(struct Registers *registers, enum Register reg, uint8_t value);
void set_register_pair(struct Registers *registers, enum RegisterPair reg, uint16_t value);
uint16_t get_control_register(struct Registers *registers, enum ControlRegister reg);
void set_control_register(struct Registers *registers, enum ControlRegister reg, uint16_t value);
// flags methods
uint8_t get_flag(struct Registers *registers, enum Flag flag);
bool get_flag_z(struct Registers *registers);
bool get_flag_n(struct Registers *registers);
bool get_flag_h(struct Registers *registers);
bool get_flag_c(struct Registers *registers);
void set_flag(struct Registers *registers, enum Flag flag, bool value);
void set_flag_z(struct Registers *registers, bool value);
void set_flag_n(struct Registers *registers, bool value);
void set_flag_h(struct Registers *registers, bool value);
void set_flag_c(struct Registers *registers, bool value);

#endif
