#include "../src/register.h"
#include "test.h"

int main()
{
    config.start_time = get_time_in_seconds();
    struct Registers *registers = create_registers();
    // check initial register values:
    assert(registers != NULL);
    assert(registers->reg_primary != NULL);
    assert(registers->reg_control != NULL);
    assert(registers->reg_primary[A] == 0x01);
    assert(registers->reg_primary[F] == 0xB0);
    assert(registers->reg_primary[B] == 0x00);
    assert(registers->reg_primary[C] == 0x13);
    assert(registers->reg_primary[D] == 0x00);
    assert(registers->reg_primary[E] == 0xD8);
    assert(registers->reg_primary[H] == 0x01);
    assert(registers->reg_primary[L] == 0x4D);
    assert(registers->reg_control[SP] == 0xFFFE);
    assert(registers->reg_control[PC] == 0x0100);

    // set all registers to 0
    set_register_byte(registers, A, 0x00);
    set_register_byte(registers, F, 0x00);
    set_register_byte(registers, B, 0x00);
    set_register_byte(registers, C, 0x00);
    set_register_byte(registers, D, 0x00);
    set_register_byte(registers, E, 0x00);
    set_register_byte(registers, H, 0x00);
    set_register_byte(registers, L, 0x00);

    set_control_register(registers, SP, 0x0000);
    set_control_register(registers, PC, 0x0000);

    // primary registers
    assert(get_register_byte(registers, A) == 0x00);
    assert(get_register_byte(registers, F) == 0x00);
    assert(get_register_byte(registers, B) == 0x00);
    assert(get_register_byte(registers, C) == 0x00);
    assert(get_register_byte(registers, D) == 0x00);
    assert(get_register_byte(registers, E) == 0x00);
    assert(get_register_byte(registers, H) == 0x00);
    assert(get_register_byte(registers, L) == 0x00);

    // pair registers
    assert(get_register_pair(registers, AF) == 0x0000);
    assert(get_register_pair(registers, BC) == 0x0000);
    assert(get_register_pair(registers, DE) == 0x0000);
    assert(get_register_pair(registers, HL) == 0x0000);

    // flags
    assert(get_flag(registers, Flag_Z) == false);
    assert(get_flag(registers, Flag_N) == false);
    assert(get_flag(registers, Flag_H) == false);
    assert(get_flag(registers, Flag_C) == false);

    // control registers
    assert(get_control_register(registers, SP) == 0x0000);
    assert(get_control_register(registers, PC) == 0x0000);

    // test set_register_byte
    set_register_byte(registers, A, 0x12);
    assert(get_register_byte(registers, A) == 0x12);

    // test set_register_pair
    set_register_pair(registers, BC, 0x1234);
    assert(get_register_pair(registers, BC) == 0x1234);

    // test set_flag
    set_flag(registers, Flag_Z, true);
    assert(get_flag(registers, Flag_Z) == true);
    assert(get_flag(registers, Flag_N) == false);
    assert(get_flag(registers, Flag_H) == false);
    assert(get_flag(registers, Flag_C) == false);

    set_flag(registers, Flag_Z, false);
    assert(get_flag(registers, Flag_Z) == false);
    assert(get_flag(registers, Flag_N) == false);
    assert(get_flag(registers, Flag_H) == false);
    assert(get_flag(registers, Flag_C) == false);

    set_flag(registers, Flag_N, true);
    assert(get_flag(registers, Flag_N) == true);
    assert(get_flag(registers, Flag_Z) == false);
    assert(get_flag(registers, Flag_H) == false);
    assert(get_flag(registers, Flag_C) == false);

    set_flag(registers, Flag_N, false);
    assert(get_flag(registers, Flag_N) == false);
    assert(get_flag(registers, Flag_Z) == false);
    assert(get_flag(registers, Flag_H) == false);
    assert(get_flag(registers, Flag_C) == false);

    set_flag(registers, Flag_H, true);
    assert(get_flag(registers, Flag_H) == true);
    assert(get_flag(registers, Flag_N) == false);
    assert(get_flag(registers, Flag_Z) == false);
    assert(get_flag(registers, Flag_C) == false);

    set_flag(registers, Flag_H, false);
    assert(get_flag(registers, Flag_H) == false);
    assert(get_flag(registers, Flag_N) == false);
    assert(get_flag(registers, Flag_Z) == false);
    assert(get_flag(registers, Flag_C) == false);

    set_flag(registers, Flag_C, true);
    assert(get_flag(registers, Flag_C) == true);
    assert(get_flag(registers, Flag_N) == false);
    assert(get_flag(registers, Flag_Z) == false);
    assert(get_flag(registers, Flag_H) == false);

    set_flag(registers, Flag_C, false);
    assert(get_flag(registers, Flag_C) == false);
    assert(get_flag(registers, Flag_N) == false);
    assert(get_flag(registers, Flag_Z) == false);
    assert(get_flag(registers, Flag_H) == false);

    // test set_control_register
    set_control_register(registers, SP, 0x1234);
    assert(get_control_register(registers, SP) == 0x1234);

    return 0;
}