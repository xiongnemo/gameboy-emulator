#include "register.h"

struct Registers* create_registers()
{
    struct Registers* registers = (struct Registers*)malloc(sizeof(struct Registers));

    // set pointers
    registers->a  = &(registers->reg_primary[A]);
    registers->f  = &(registers->reg_primary[F]);
    registers->b  = &(registers->reg_primary[B]);
    registers->c  = &(registers->reg_primary[C]);
    registers->d  = &(registers->reg_primary[D]);
    registers->e  = &(registers->reg_primary[E]);
    registers->h  = &(registers->reg_primary[H]);
    registers->l  = &(registers->reg_primary[L]);
    registers->sp = &(registers->reg_control[SP]);
    registers->pc = &(registers->reg_control[PC]);

    // set registers
    set_register_byte(registers, A, 0x01);
    set_register_byte(registers, F, 0xB0);
    set_register_byte(registers, B, 0x00);
    set_register_byte(registers, C, 0x13);
    set_register_byte(registers, D, 0x00);
    set_register_byte(registers, E, 0xD8);
    set_register_byte(registers, H, 0x01);
    set_register_byte(registers, L, 0x4D);

    set_control_register(registers, SP, 0xFFFE);

    set_control_register(registers, PC, 0x0100);

    // set method pointers
    registers->get_register_byte    = get_register_byte;
    registers->get_register_pair    = get_register_pair;
    registers->set_register_byte    = set_register_byte;
    registers->set_register_pair    = set_register_pair;
    registers->get_control_register = get_control_register;
    registers->set_control_register = set_control_register;
    registers->get_flag             = get_flag;
    registers->get_flag_z           = get_flag_z;
    registers->get_flag_n           = get_flag_n;
    registers->get_flag_h           = get_flag_h;
    registers->get_flag_c           = get_flag_c;
    registers->set_flag             = set_flag;
    registers->set_flag_z           = set_flag_z;
    registers->set_flag_n           = set_flag_n;
    registers->set_flag_h           = set_flag_h;
    registers->set_flag_c           = set_flag_c;

    return registers;
}

void free_registers(struct Registers* registers)
{
    free(registers);
}

uint8_t get_register_byte(struct Registers* registers, enum Register reg)
{
    REGISTER_TRACE_PRINT(
        "GET_REGISTER_BYTE: reg: %d, value: 0x%02x\n", reg, registers->reg_primary[reg]);
    return registers->reg_primary[reg];
}

uint16_t get_register_pair(struct Registers* registers, enum RegisterPair reg_pair)
{
    REGISTER_TRACE_PRINT("GET_REGISTER_PAIR: reg_pair: %d, high_byte: 0x%02x, low_byte: 0x%02x\n",
                         reg_pair,
                         registers->reg_primary[reg_pair << 1],
                         registers->reg_primary[(reg_pair << 1) | 1]);
    uint16_t high_byte = (registers->reg_primary[reg_pair << 1] << 8);
    uint16_t low_byte  = registers->reg_primary[(reg_pair << 1) | 1];
    return high_byte | low_byte;
}

void set_register_byte(struct Registers* registers, enum Register reg, uint8_t value)
{
    REGISTER_TRACE_PRINT("SET_REGISTER_BYTE: reg: %d, value: 0x%02x\n", reg, value);
    registers->reg_primary[reg] = value;
}

void set_register_pair(struct Registers* registers, enum RegisterPair reg_pair, uint16_t value)
{
    uint8_t high_byte = (value >> 8) & 0xFF;
    uint8_t low_byte  = value & 0xFF;
    REGISTER_TRACE_PRINT("SET_REGISTER_PAIR: reg_pair: %d, high_byte: 0x%02x, low_byte: 0x%02x\n",
                         reg_pair,
                         high_byte,
                         low_byte);
    registers->reg_primary[reg_pair << 1]       = high_byte;
    registers->reg_primary[(reg_pair << 1) | 1] = low_byte;
}

uint16_t get_control_register(struct Registers* registers, enum ControlRegister reg)
{
    REGISTER_TRACE_PRINT(
        "GET_CONTROL_REGISTER: reg: %d, value: 0x%04x\n", reg, registers->reg_control[reg]);
    return registers->reg_control[reg];
}

void set_control_register(struct Registers* registers, enum ControlRegister reg, uint16_t value)
{
    REGISTER_TRACE_PRINT("SET_CONTROL_REGISTER: reg: %d, value: 0x%04x\n", reg, value);
    registers->reg_control[reg] = value;
}

uint8_t get_flag(struct Registers* registers, enum Flag flag)
{
    REGISTER_TRACE_PRINT(
        "GET_FLAG: flag: %d, value: 0x%01x\n", flag, (registers->reg_primary[F] & flag) != 0);
    return (registers->reg_primary[F] & flag) != 0;
}

bool get_flag_z(struct Registers* registers)
{
    return get_flag(registers, Flag_Z);
}

bool get_flag_n(struct Registers* registers)
{
    return get_flag(registers, Flag_N);
}

bool get_flag_h(struct Registers* registers)
{
    return get_flag(registers, Flag_H);
}

bool get_flag_c(struct Registers* registers)
{
    return get_flag(registers, Flag_C);
}

void set_flag(struct Registers* registers, enum Flag flag, bool value)
{
    REGISTER_TRACE_PRINT("SET_FLAG: flag: %d, value: 0x%01x\n", flag, value);
    if (value) {
        registers->reg_primary[F] |= flag;   // Set flag
    }
    else {
        registers->reg_primary[F] &= ~flag;   // Clear flag
    }
}

void set_flag_z(struct Registers* registers, bool value)
{
    set_flag(registers, Flag_Z, value);
}

void set_flag_n(struct Registers* registers, bool value)
{
    set_flag(registers, Flag_N, value);
}

void set_flag_h(struct Registers* registers, bool value)
{
    set_flag(registers, Flag_H, value);
}

void set_flag_c(struct Registers* registers, bool value)
{
    set_flag(registers, Flag_C, value);
}
