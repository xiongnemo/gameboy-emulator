#include "../src/cpu.h"
#include "test.h"

#define TEST_PC 0xC000
#define TEST_SP 0xD000

// create all components
// and set PC to 0xC000
// and set SP to 0xD000
#define CREATE_ALL_COMPONENTS                                          \
    struct CPU *cpu;                                                   \
    CPU_INFO_PRINT("bringing up cpu: %p\n", cpu);                      \
    struct Cartridge *cartridge = create_cartridge();                  \
    struct Ram *ram = create_ram();                                    \
    struct Vram *vram = create_vram();                                 \
    struct PPU *ppu = create_ppu(vram);                                \
    struct MMU *mmu = create_mmu(cartridge, ram, ppu);                 \
    struct Registers *registers = create_registers();                  \
    cpu = create_cpu(registers, mmu);                                  \
    CPU_INFO_PRINT("setting pc to 0x%04x for testing\n", TEST_PC);     \
    cpu->registers->set_control_register(cpu->registers, PC, TEST_PC); \
    CPU_INFO_PRINT("setting sp to 0x%04x for testing\n", TEST_SP);     \
    cpu->registers->set_control_register(cpu->registers, SP, TEST_SP); \
    CPU_INFO_PRINT("cpu: %p is ready\n", cpu);

// test counter
static int test_counter = 0;
#define DELETE_ALL_COMPONENTS                     \
    CPU_INFO_PRINT("cleaning up cpu: %p\n", cpu); \
    free_cpu(cpu);                                \
    test_counter++;                               \
    CPU_INFO_PRINT("test %d completed\n", test_counter);

// 0x01: LD BC, nn (imm)
void test_opcode_01()
{
    CREATE_ALL_COMPONENTS

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0x01);
    cpu->mmu->mmu_set_word(cpu->mmu, TEST_PC + 1, 0x1145);

    cpu->cpu_step_next(cpu);

    uint16_t bc_value = cpu->registers->get_register_pair(cpu->registers, BC);
    uint16_t expected = cpu->mmu->mmu_get_word(cpu->mmu, TEST_PC + 1);
    assert(bc_value == expected);

    DELETE_ALL_COMPONENTS
}

void test_opcode_02()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0xFF);
    cpu->registers->set_register_pair(cpu->registers, BC, 0xC00F);

    cpu->mmu->mmu_set_byte(cpu->mmu, cpu->registers->get_control_register(cpu->registers, PC), 0x02);
    cpu->cpu_step_next(cpu);

    uint8_t value_at_bc = cpu->mmu->mmu_get_byte(cpu->mmu, cpu->registers->get_register_pair(cpu->registers, BC));
    assert(value_at_bc == cpu->registers->get_register_byte(cpu->registers, A));

    DELETE_ALL_COMPONENTS
}

void test_opcode_03()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_pair(cpu->registers, BC, 0xC0FE);

    cpu->mmu->mmu_set_byte(cpu->mmu, cpu->registers->get_control_register(cpu->registers, PC), 0x03);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_pair(cpu->registers, BC) == (0xC0FE + 1));

    DELETE_ALL_COMPONENTS
}

void test_opcode_04()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, B, 0xFE);

    cpu->mmu->mmu_set_byte(cpu->mmu, cpu->registers->get_control_register(cpu->registers, PC), 0x04);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, B) == (0xFE + 1));

    DELETE_ALL_COMPONENTS
}

void test_opcode_05()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, B, 0xFE);

    cpu->mmu->mmu_set_byte(cpu->mmu, cpu->registers->get_control_register(cpu->registers, PC), 0x05);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, B) == (0xFE - 1));

    DELETE_ALL_COMPONENTS
}

void test_opcode_06()
{
    CREATE_ALL_COMPONENTS

    cpu->mmu->mmu_set_byte(cpu->mmu, 0xC000, 0xFF);

    cpu->mmu->mmu_set_byte(cpu->mmu, cpu->registers->get_control_register(cpu->registers, PC), 0x06);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, B) ==
           cpu->mmu->mmu_get_byte(cpu->mmu, cpu->registers->get_control_register(cpu->registers, PC) - 1));

    DELETE_ALL_COMPONENTS
}

void test_opcode_07()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0xFE);

    cpu->mmu->mmu_set_byte(cpu->mmu, cpu->registers->get_control_register(cpu->registers, PC), 0x07);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, A) == 0xFD);
    assert(cpu->registers->get_flag(cpu->registers, Flag_C) == 1);

    DELETE_ALL_COMPONENTS
}

// 0x08: LD (a16), SP
void test_opcode_08()
{
    CREATE_ALL_COMPONENTS

    cpu->mmu->mmu_set_word(cpu->mmu, TEST_PC + 1, 0xC100);

    cpu->mmu->mmu_set_byte(cpu->mmu, cpu->registers->get_control_register(cpu->registers, PC), 0x08);
    cpu->cpu_step_next(cpu);

    uint16_t addr = cpu->mmu->mmu_get_word(cpu->mmu, cpu->registers->get_control_register(cpu->registers, PC) - 2);
    assert(cpu->mmu->mmu_get_word(cpu->mmu, addr) == cpu->registers->get_control_register(cpu->registers, SP));

    DELETE_ALL_COMPONENTS
}

// 0x09: ADD HL, BC
void test_opcode_09()
{
    CREATE_ALL_COMPONENTS

    // set BC to 0x0FFF
    cpu->registers->set_register_byte(cpu->registers, B, 0x0F);
    cpu->registers->set_register_byte(cpu->registers, C, 0xFF);

    // set HL to 0x0001
    cpu->registers->set_register_byte(cpu->registers, H, 0x00);
    cpu->registers->set_register_byte(cpu->registers, L, 0x01);

    cpu->mmu->mmu_set_byte(cpu->mmu, cpu->registers->get_control_register(cpu->registers, PC), 0x09);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_pair(cpu->registers, HL) == 0x1000);

    DELETE_ALL_COMPONENTS
}

// 0x0A: LD A, (BC)
void test_opcode_0a()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_pair(cpu->registers, BC, 0xC100);
    cpu->mmu->mmu_set_byte(cpu->mmu, 0xC100, 0x45);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0x0A);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, A) == 0x45);

    DELETE_ALL_COMPONENTS
}

// 0x0B: DEC BC
void test_opcode_0b()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_pair(cpu->registers, BC, 0xC100);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0x0B);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_pair(cpu->registers, BC) == 0xC0FF);

    DELETE_ALL_COMPONENTS
}

// 0x0F: RRCA
void test_opcode_0f()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0x1);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0x0F);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, A) == 0x80);
    assert(cpu->registers->get_flag(cpu->registers, Flag_C) == 1);

    DELETE_ALL_COMPONENTS
}

// 0x17: RLA (1)
void test_opcode_17_1()
{
    CREATE_ALL_COMPONENTS

    // First test
    cpu->registers->set_register_byte(cpu->registers, A, 0x52);
    cpu->registers->set_flag(cpu->registers, Flag_C, 1);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0x17);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, A) == 0xA5);
    assert(cpu->registers->get_flag(cpu->registers, Flag_C) == 0);

    DELETE_ALL_COMPONENTS
}

// 0x17: RLA (2)
void test_opcode_17_2()
{
    // Second test
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0x54);
    cpu->registers->set_flag(cpu->registers, Flag_C, 0);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0x17);
    cpu->cpu_step_next(cpu);

    // 0x54 is 0101_0100
    // after RLA, it should be 1010_1000, which is 0xA8
    // and carry flag should be 0
    assert(cpu->registers->get_register_byte(cpu->registers, A) == 0xA8);
    assert(cpu->registers->get_flag(cpu->registers, Flag_C) == 0);

    DELETE_ALL_COMPONENTS
}

void test_opcode_17()
{
    test_opcode_17_1();
    test_opcode_17_2();
}

// 0x18: JR r8
void test_opcode_18_1()
{
    CREATE_ALL_COMPONENTS

    uint16_t initial_pc = TEST_PC;
    int8_t offset = 0x08;

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0x18);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, offset);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_control_register(cpu->registers, PC) == initial_pc + offset + 2);

    DELETE_ALL_COMPONENTS
}

void test_opcode_18_2()
{
    CREATE_ALL_COMPONENTS

    uint16_t initial_pc = TEST_PC;
    int8_t offset = -0x08;

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0x18);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, offset);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_control_register(cpu->registers, PC) == initial_pc + offset + 2);

    DELETE_ALL_COMPONENTS
}

void test_opcode_18()
{
    test_opcode_18_1();
}

// 0x1F: RRA
void test_opcode_1f()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0x81);
    cpu->registers->set_flag(cpu->registers, Flag_C, 0);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0x1F);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, A) == 0x40);
    assert(cpu->registers->get_flag(cpu->registers, Flag_C) == 1);

    DELETE_ALL_COMPONENTS
}

// 0x20: JR NZ, n
void test_opcode_20()
{
    CREATE_ALL_COMPONENTS

    uint16_t initial_pc = TEST_PC;
    int8_t offset = 0x08;

    // Test when Z flag is 0 (jump should occur)
    cpu->registers->set_flag(cpu->registers, Flag_Z, 0);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0x20);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, offset);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_control_register(cpu->registers, PC) == initial_pc + offset + 2);

    DELETE_ALL_COMPONENTS
}

// 0x22: LD (HL+), A
void test_opcode_22()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_pair(cpu->registers, HL, 0xC100);
    cpu->registers->set_register_byte(cpu->registers, A, 0x45);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0x22);
    cpu->cpu_step_next(cpu);

    assert(cpu->mmu->mmu_get_byte(cpu->mmu, 0xC100) == 0x45);
    assert(cpu->registers->get_register_pair(cpu->registers, HL) == 0xC101);

    DELETE_ALL_COMPONENTS
}

// 0x3A: LD A, (HL-) / LDD A, (HL)
void test_opcode_3a()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_pair(cpu->registers, HL, 0xC10F);
    cpu->mmu->mmu_set_byte(cpu->mmu, 0xC10F, 0x11);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0x3A);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, A) == 0x11);
    assert(cpu->registers->get_register_pair(cpu->registers, HL) == 0xC10E);

    DELETE_ALL_COMPONENTS
}

// 0x3B: DEC SP
void test_opcode_3b()
{
    CREATE_ALL_COMPONENTS

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0x3B);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_control_register(cpu->registers, SP) == TEST_SP - 1);

    DELETE_ALL_COMPONENTS
}

// 0x80: ADD A, B
void test_opcode_80()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0x41);
    cpu->registers->set_register_byte(cpu->registers, B, 0x1F);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0x80);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, A) == 0x60);

    DELETE_ALL_COMPONENTS
}

// 0x86: ADD A, (HL)
void test_opcode_86()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0x11);
    cpu->registers->set_register_pair(cpu->registers, HL, 0xC10F);
    cpu->mmu->mmu_set_byte(cpu->mmu, 0xC10F, 0x14);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0x86);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, A) == 0x25);

    DELETE_ALL_COMPONENTS
}

// 0x88: ADC A, B
void test_opcode_88()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0x41);
    cpu->registers->set_register_byte(cpu->registers, B, 0x1F);
    cpu->registers->set_flag(cpu->registers, Flag_C, true);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0x88);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, A) == 0x61);

    DELETE_ALL_COMPONENTS
}

// 0x8E: ADC A, (HL)
void test_opcode_8e()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0x41);
    cpu->registers->set_register_pair(cpu->registers, HL, 0xC10F);
    cpu->mmu->mmu_set_byte(cpu->mmu, 0xC10F, 0x1F);
    cpu->registers->set_flag(cpu->registers, Flag_C, true);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0x8E);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, A) == 0x61);

    DELETE_ALL_COMPONENTS
}

// 0x90: SUB B
void test_opcode_90()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0x41);
    cpu->registers->set_register_byte(cpu->registers, B, 0x1F);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0x90);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, A) == 0x22);

    DELETE_ALL_COMPONENTS
}

// 0x96: SUB (HL)
void test_opcode_96()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0x41);
    cpu->registers->set_register_pair(cpu->registers, HL, 0xC10F);
    cpu->mmu->mmu_set_byte(cpu->mmu, 0xC10F, 0x1F);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0x96);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, A) == 0x22);

    DELETE_ALL_COMPONENTS
}

// 0x98: SBC A, B
void test_opcode_98()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0x41);
    cpu->registers->set_register_byte(cpu->registers, B, 0x1F);
    cpu->registers->set_flag(cpu->registers, Flag_C, true);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0x98);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, A) == 0x21);

    DELETE_ALL_COMPONENTS
}

// 0x9E: SBC A, (HL)
void test_opcode_9e()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0x41);
    cpu->registers->set_register_pair(cpu->registers, HL, 0xC10F);
    cpu->mmu->mmu_set_byte(cpu->mmu, 0xC10F, 0x1F);
    cpu->registers->set_flag(cpu->registers, Flag_C, true);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0x9E);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, A) == 0x21);

    DELETE_ALL_COMPONENTS
}

// 0xA0: AND B
void test_opcode_a0()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0x41);
    cpu->registers->set_register_byte(cpu->registers, B, 0x1F);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xA0);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, A) == 0x01);

    DELETE_ALL_COMPONENTS
}

// 0xA6: AND (HL)
void test_opcode_a6()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0x41);
    cpu->registers->set_register_pair(cpu->registers, HL, 0xC10F);
    cpu->mmu->mmu_set_byte(cpu->mmu, 0xC10F, 0x1F);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xA6);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, A) == 0x01);

    DELETE_ALL_COMPONENTS
}

// 0xA8: XOR B
void test_opcode_a8()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0x41);
    cpu->registers->set_register_byte(cpu->registers, B, 0x1F);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xA8);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, A) == 0x5E);

    DELETE_ALL_COMPONENTS
}

// 0xAE: XOR (HL)
void test_opcode_ae()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0x41);
    cpu->registers->set_register_pair(cpu->registers, HL, 0xC10F);
    cpu->mmu->mmu_set_byte(cpu->mmu, 0xC10F, 0x1F);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xAE);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, A) == 0x5E);

    DELETE_ALL_COMPONENTS
}

// 0xB0: OR B
void test_opcode_b0()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0x41);
    cpu->registers->set_register_byte(cpu->registers, B, 0x1F);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xB0);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, A) == 0x5F);

    DELETE_ALL_COMPONENTS
}

// 0xB6: OR (HL)
void test_opcode_b6()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0x41);
    cpu->registers->set_register_pair(cpu->registers, HL, 0xC10F);
    cpu->mmu->mmu_set_byte(cpu->mmu, 0xC10F, 0x1F);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xB6);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, A) == 0x5F);

    DELETE_ALL_COMPONENTS
}

// 0xB8: CP B
void test_opcode_b8()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0x41);
    cpu->registers->set_register_byte(cpu->registers, B, 0x41);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xB8);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_flag(cpu->registers, Flag_Z) == 1);

    DELETE_ALL_COMPONENTS
}

// 0xBE: CP (HL)
void test_opcode_be()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0x41);
    cpu->registers->set_register_pair(cpu->registers, HL, 0xC10F);
    cpu->mmu->mmu_set_byte(cpu->mmu, 0xC10F, 0x41);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xBE);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_flag(cpu->registers, Flag_Z) == 1);

    DELETE_ALL_COMPONENTS
}

// 0xC0: RET NZ
void test_opcode_c0_1()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_flag(cpu->registers, Flag_Z, 0);
    cpu->mmu->mmu_set_word(cpu->mmu, TEST_SP, 0xC200);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xC0);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_control_register(cpu->registers, PC) == 0xC200);

    DELETE_ALL_COMPONENTS
}

void test_opcode_c0_2()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_flag(cpu->registers, Flag_Z, 1);
    cpu->mmu->mmu_set_word(cpu->mmu, TEST_SP, 0xC200);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xC0);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_control_register(cpu->registers, PC) == TEST_PC + 1);

    DELETE_ALL_COMPONENTS
}

void test_opcode_c0()
{
    test_opcode_c0_1();
    test_opcode_c0_2();
}

// 0xC1: POP BC
void test_opcode_c1()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_control_register(cpu->registers, SP, 0xC100);
    cpu->mmu->mmu_set_word(cpu->mmu, 0xC100, 0x1234);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xC1);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_pair(cpu->registers, BC) == 0x1234);
    assert(cpu->registers->get_control_register(cpu->registers, SP) == 0xC102);

    DELETE_ALL_COMPONENTS
}

// 0xC2: JP NZ, nn
void test_opcode_c2()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_flag(cpu->registers, Flag_Z, 0);
    cpu->mmu->mmu_set_word(cpu->mmu, TEST_PC + 1, 0xC200);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xC2);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_control_register(cpu->registers, PC) == 0xC200);

    DELETE_ALL_COMPONENTS
}

// 0xC4: CALL NZ, nn
void test_opcode_c4()
{
    CREATE_ALL_COMPONENTS

    uint16_t initial_pc = TEST_PC;
    cpu->registers->set_flag(cpu->registers, Flag_Z, 0);
    cpu->registers->set_control_register(cpu->registers, SP, 0xC100);
    cpu->mmu->mmu_set_word(cpu->mmu, TEST_PC + 1, 0xC200);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xC4);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_control_register(cpu->registers, PC) == 0xC200);
    assert(cpu->mmu->mmu_get_word(cpu->mmu, 0xC0FE) == initial_pc + 3);
    assert(cpu->registers->get_control_register(cpu->registers, SP) == 0xC0FE);

    DELETE_ALL_COMPONENTS
}

// 0xC5: PUSH BC
void test_opcode_c5()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_control_register(cpu->registers, SP, 0xC100);
    cpu->registers->set_register_pair(cpu->registers, BC, 0x1234);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xC5);
    cpu->cpu_step_next(cpu);

    assert(cpu->mmu->mmu_get_word(cpu->mmu, 0xC0FE) == 0x1234);
    assert(cpu->registers->get_control_register(cpu->registers, SP) == 0xC0FE);

    DELETE_ALL_COMPONENTS
}

// 0xC6: ADD A, n
void test_opcode_c6()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0x3C);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x0F);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xC6);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, A) == 0x4B);

    DELETE_ALL_COMPONENTS
}

// 0xC8: RET Z
void test_opcode_c8_1()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_flag(cpu->registers, Flag_Z, 1);
    cpu->mmu->mmu_set_word(cpu->mmu, TEST_SP, 0xC200);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xC8);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_control_register(cpu->registers, PC) == 0xC200);

    DELETE_ALL_COMPONENTS
}

void test_opcode_c8_2()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_flag(cpu->registers, Flag_Z, 0);
    cpu->mmu->mmu_set_word(cpu->mmu, TEST_SP, 0xC200);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xC8);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_control_register(cpu->registers, PC) == TEST_PC + 1);

    DELETE_ALL_COMPONENTS
}

void test_opcode_c8()
{
    test_opcode_c8_1();
    test_opcode_c8_2();
}

// 0xC9: RET
void test_opcode_c9()
{
    CREATE_ALL_COMPONENTS

    cpu->mmu->mmu_set_word(cpu->mmu, TEST_SP, 0xC200);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xC9);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_control_register(cpu->registers, PC) == 0xC200);

    DELETE_ALL_COMPONENTS
}

// 0xCA: JP Z, nn
void test_opcode_ca()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_flag(cpu->registers, Flag_Z, 1);
    cpu->mmu->mmu_set_word(cpu->mmu, TEST_PC + 1, 0xC200);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xCA);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_control_register(cpu->registers, PC) == 0xC200);

    DELETE_ALL_COMPONENTS
}

// 0xCC: CALL Z, nn
void test_opcode_cc()
{
    CREATE_ALL_COMPONENTS

    uint16_t initial_pc = TEST_PC;
    cpu->registers->set_flag(cpu->registers, Flag_Z, 1);
    cpu->registers->set_control_register(cpu->registers, SP, 0xC100);
    cpu->mmu->mmu_set_word(cpu->mmu, TEST_PC + 1, 0xC200);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xCC);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_control_register(cpu->registers, PC) == 0xC200);
    assert(cpu->mmu->mmu_get_word(cpu->mmu, 0xC0FE) == initial_pc + 3);
    assert(cpu->registers->get_control_register(cpu->registers, SP) == 0xC0FE);

    DELETE_ALL_COMPONENTS
}

// 0xCD: CALL nn
void test_opcode_cd()
{
    CREATE_ALL_COMPONENTS

    uint16_t initial_pc = TEST_PC;
    cpu->registers->set_control_register(cpu->registers, SP, 0xC100);
    cpu->mmu->mmu_set_word(cpu->mmu, TEST_PC + 1, 0xC200);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xCD);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_control_register(cpu->registers, PC) == 0xC200);
    assert(cpu->mmu->mmu_get_word(cpu->mmu, 0xC0FE) == initial_pc + 3);
    assert(cpu->registers->get_control_register(cpu->registers, SP) == 0xC0FE);

    DELETE_ALL_COMPONENTS
}

// 0xCE: ADC A, n
void test_opcode_ce()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0x3C);
    cpu->registers->set_flag(cpu->registers, Flag_C, 1);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x0F);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xCE);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, A) == 0x4C);

    DELETE_ALL_COMPONENTS
}

// 0xD0: RET NC
void test_opcode_d0_1()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_flag(cpu->registers, Flag_C, 0);
    cpu->mmu->mmu_set_word(cpu->mmu, TEST_SP, 0xC200);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xD0);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_control_register(cpu->registers, PC) == 0xC200);

    DELETE_ALL_COMPONENTS
}

void test_opcode_d0_2()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_flag(cpu->registers, Flag_C, 1);
    cpu->mmu->mmu_set_word(cpu->mmu, TEST_SP, 0xC200);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xD0);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_control_register(cpu->registers, PC) == TEST_PC + 1);

    DELETE_ALL_COMPONENTS
}

void test_opcode_d0()
{
    test_opcode_d0_1();
    test_opcode_d0_2();
}

// 0xD1: POP DE
void test_opcode_d1()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_control_register(cpu->registers, SP, 0xC100);
    cpu->mmu->mmu_set_word(cpu->mmu, 0xC100, 0x1234);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xD1);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_pair(cpu->registers, DE) == 0x1234);
    assert(cpu->registers->get_control_register(cpu->registers, SP) == 0xC102);

    DELETE_ALL_COMPONENTS
}

// 0xD2: JP NC, nn
void test_opcode_d2()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_flag(cpu->registers, Flag_C, 0);
    cpu->mmu->mmu_set_word(cpu->mmu, TEST_PC + 1, 0xC200);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xD2);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_control_register(cpu->registers, PC) == 0xC200);

    DELETE_ALL_COMPONENTS
}

// 0xD4: CALL NC, nn
void test_opcode_d4()
{
    CREATE_ALL_COMPONENTS

    uint16_t initial_pc = TEST_PC;
    cpu->registers->set_flag(cpu->registers, Flag_C, 0);
    cpu->registers->set_control_register(cpu->registers, SP, 0xC100);
    cpu->mmu->mmu_set_word(cpu->mmu, TEST_PC + 1, 0xC200);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xD4);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_control_register(cpu->registers, PC) == 0xC200);
    assert(cpu->mmu->mmu_get_word(cpu->mmu, 0xC0FE) == initial_pc + 3);
    assert(cpu->registers->get_control_register(cpu->registers, SP) == 0xC0FE);

    DELETE_ALL_COMPONENTS
}

// 0xD5: PUSH DE
void test_opcode_d5()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_control_register(cpu->registers, SP, 0xC100);
    cpu->registers->set_register_pair(cpu->registers, DE, 0x1234);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xD5);
    cpu->cpu_step_next(cpu);

    assert(cpu->mmu->mmu_get_word(cpu->mmu, 0xC0FE) == 0x1234);
    assert(cpu->registers->get_control_register(cpu->registers, SP) == 0xC0FE);

    DELETE_ALL_COMPONENTS
}

// 0xD6: SUB n
void test_opcode_d6()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0x3E);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x3E);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xD6);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, A) == 0x00);
    assert(cpu->registers->get_flag(cpu->registers, Flag_Z) == 1);

    DELETE_ALL_COMPONENTS
}

// 0xD8: RET C
void test_opcode_d8_1()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_flag(cpu->registers, Flag_C, 1);
    cpu->mmu->mmu_set_word(cpu->mmu, TEST_SP, 0xC200);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xD8);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_control_register(cpu->registers, PC) == 0xC200);

    DELETE_ALL_COMPONENTS
}

void test_opcode_d8_2()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_flag(cpu->registers, Flag_C, 0);
    cpu->mmu->mmu_set_word(cpu->mmu, TEST_SP, 0xC200);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xD8);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_control_register(cpu->registers, PC) == TEST_PC + 1);

    DELETE_ALL_COMPONENTS
}

void test_opcode_d8()
{
    test_opcode_d8_1();
    test_opcode_d8_2();
}

// 0xD9: RETI
void test_opcode_d9()
{
    CREATE_ALL_COMPONENTS

    cpu->mmu->mmu_set_word(cpu->mmu, TEST_SP, 0xC200);
    cpu->interrupt_master_enable = false;

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xD9);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_control_register(cpu->registers, PC) == 0xC200);
    assert(cpu->interrupt_master_enable == true);

    DELETE_ALL_COMPONENTS
}

// 0xDA: JP C, nn
void test_opcode_da()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_flag(cpu->registers, Flag_C, 1);
    cpu->mmu->mmu_set_word(cpu->mmu, TEST_PC + 1, 0xC200);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xDA);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_control_register(cpu->registers, PC) == 0xC200);

    DELETE_ALL_COMPONENTS
}

// 0xDC: CALL C, nn
void test_opcode_dc()
{
    CREATE_ALL_COMPONENTS

    uint16_t initial_pc = TEST_PC;
    cpu->registers->set_flag(cpu->registers, Flag_C, 1);
    cpu->registers->set_control_register(cpu->registers, SP, 0xC100);
    cpu->mmu->mmu_set_word(cpu->mmu, TEST_PC + 1, 0xC200);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xDC);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_control_register(cpu->registers, PC) == 0xC200);
    assert(cpu->mmu->mmu_get_word(cpu->mmu, 0xC0FE) == initial_pc + 3);
    assert(cpu->registers->get_control_register(cpu->registers, SP) == 0xC0FE);

    DELETE_ALL_COMPONENTS
}

// 0xDE: SBC A, n
void test_opcode_de()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0x3B);
    cpu->registers->set_flag(cpu->registers, Flag_C, 1);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x2A);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xDE);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, A) == 0x10);

    DELETE_ALL_COMPONENTS
}

// 0xE0: LDH (n), A
void test_opcode_e0()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0x45);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x40);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xE0);
    cpu->cpu_step_next(cpu);

    assert(cpu->mmu->mmu_get_byte(cpu->mmu, 0xFF40) == 0x45);

    DELETE_ALL_COMPONENTS
}

// 0xE1: POP HL
void test_opcode_e1()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_control_register(cpu->registers, SP, 0xC100);
    cpu->mmu->mmu_set_word(cpu->mmu, 0xC100, 0x1234);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xE1);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_pair(cpu->registers, HL) == 0x1234);
    assert(cpu->registers->get_control_register(cpu->registers, SP) == 0xC102);

    DELETE_ALL_COMPONENTS
}

// 0xE2: LD (C), A
void test_opcode_e2()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0x45);
    cpu->registers->set_register_byte(cpu->registers, C, 0x40);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xE2);
    cpu->cpu_step_next(cpu);

    assert(cpu->mmu->mmu_get_byte(cpu->mmu, 0xFF40) == 0x45);

    DELETE_ALL_COMPONENTS
}

// 0xE5: PUSH HL
void test_opcode_e5()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_control_register(cpu->registers, SP, 0xC100);
    cpu->registers->set_register_pair(cpu->registers, HL, 0x1234);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xE5);
    cpu->cpu_step_next(cpu);

    assert(cpu->mmu->mmu_get_word(cpu->mmu, 0xC0FE) == 0x1234);
    assert(cpu->registers->get_control_register(cpu->registers, SP) == 0xC0FE);

    DELETE_ALL_COMPONENTS
}

// 0xE6: AND n
void test_opcode_e6()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0x5A);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x3F);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xE6);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, A) == 0x1A);

    DELETE_ALL_COMPONENTS
}

// 0xE8: ADD SP, n
void test_opcode_e8()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_control_register(cpu->registers, SP, 0xC100);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x02);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xE8);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_control_register(cpu->registers, SP) == 0xC102);

    DELETE_ALL_COMPONENTS
}

// 0xE9: JP (HL)
void test_opcode_e9()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_pair(cpu->registers, HL, 0xC200);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xE9);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_control_register(cpu->registers, PC) == 0xC200);

    DELETE_ALL_COMPONENTS
}

// 0xEA: LD (nn), A
void test_opcode_ea()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0x45);
    cpu->mmu->mmu_set_word(cpu->mmu, TEST_PC + 1, 0xC200);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xEA);
    cpu->cpu_step_next(cpu);

    assert(cpu->mmu->mmu_get_byte(cpu->mmu, 0xC200) == 0x45);

    DELETE_ALL_COMPONENTS
}

// 0xF0: LDH A, (n)
void test_opcode_f0()
{
    CREATE_ALL_COMPONENTS

    cpu->mmu->mmu_set_byte(cpu->mmu, ZERO_PAGE_ADDRESS + 0x40, 0x45);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x40);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xF0);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, A) == 0x45);

    DELETE_ALL_COMPONENTS
}

// 0xF1: POP AF
void test_opcode_f1()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_control_register(cpu->registers, SP, 0xC100);
    cpu->mmu->mmu_set_word(cpu->mmu, 0xC100, 0x1232);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xF1);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_pair(cpu->registers, AF) == 0x1230); // Lower nibble of F is always 0, under this instruction higher nibble is always 1
    assert(cpu->registers->get_control_register(cpu->registers, SP) == 0xC102);

    // check all flags as  0x03 - 0b0000_0011
    assert(cpu->registers->get_flag(cpu->registers, Flag_Z) == false);
    assert(cpu->registers->get_flag(cpu->registers, Flag_N) == false);
    assert(cpu->registers->get_flag(cpu->registers, Flag_H) == true);
    assert(cpu->registers->get_flag(cpu->registers, Flag_C) == true);

    DELETE_ALL_COMPONENTS
}

// 0xF2: LD A, (C)
void test_opcode_f2()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, C, 0x40);
    cpu->mmu->mmu_set_byte(cpu->mmu, 0xFF40, 0x45);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xF2);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, A) == 0x45);

    DELETE_ALL_COMPONENTS
}

// 0xF5: PUSH AF
void test_opcode_f5()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_control_register(cpu->registers, SP, 0xC100);
    cpu->registers->set_register_pair(cpu->registers, AF, 0x1234);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xF5);
    cpu->cpu_step_next(cpu);

    assert(cpu->mmu->mmu_get_word(cpu->mmu, 0xC0FE) == 0x1234);
    assert(cpu->registers->get_control_register(cpu->registers, SP) == 0xC0FE);

    DELETE_ALL_COMPONENTS
}

// 0xF6: OR n
void test_opcode_f6()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0x5A);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x0F);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xF6);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, A) == 0x5F);

    DELETE_ALL_COMPONENTS
}

// 0xF8: LD HL, SP+n
void test_opcode_f8()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_control_register(cpu->registers, SP, 0xC100);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x02);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xF8);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_pair(cpu->registers, HL) == 0xC102);

    DELETE_ALL_COMPONENTS
}

// 0xF9: LD SP, HL
void test_opcode_f9()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_pair(cpu->registers, HL, 0xC200);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xF9);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_control_register(cpu->registers, SP) == 0xC200);

    DELETE_ALL_COMPONENTS
}

// 0xFA: LD A, (nn)
void test_opcode_fa()
{
    CREATE_ALL_COMPONENTS

    cpu->mmu->mmu_set_byte(cpu->mmu, 0xC200, 0x45);
    cpu->mmu->mmu_set_word(cpu->mmu, TEST_PC + 1, 0xC200);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xFA);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, A) == 0x45);

    DELETE_ALL_COMPONENTS
}

// 0xFE: CP n
void test_opcode_fe()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, A, 0x42);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x42);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xFE);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_flag(cpu->registers, Flag_Z) == 1);

    DELETE_ALL_COMPONENTS
}

// CB prefix opcodes

// CB 0x00: RLC B
void test_opcode_cb_00()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, B, 0x85);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xCB);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x00);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, B) == 0x0B);
    assert(cpu->registers->get_flag(cpu->registers, Flag_C) == 1);

    DELETE_ALL_COMPONENTS
}

// CB 0x06: RLC (HL)
void test_opcode_cb_06()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_pair(cpu->registers, HL, 0xC100);
    cpu->mmu->mmu_set_byte(cpu->mmu, 0xC100, 0x85);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xCB);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x06);
    cpu->cpu_step_next(cpu);

    assert(cpu->mmu->mmu_get_byte(cpu->mmu, 0xC100) == 0x0B);
    assert(cpu->registers->get_flag(cpu->registers, Flag_C) == 1);

    DELETE_ALL_COMPONENTS
}

// CB 0x08: RRC B
void test_opcode_cb_08()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, B, 0x01);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xCB);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x08);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, B) == 0x80);
    assert(cpu->registers->get_flag(cpu->registers, Flag_C) == 1);

    DELETE_ALL_COMPONENTS
}

// CB 0x0E: RRC (HL)
void test_opcode_cb_0e()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_pair(cpu->registers, HL, 0xC100);
    cpu->mmu->mmu_set_byte(cpu->mmu, 0xC100, 0x01);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xCB);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x0E);
    cpu->cpu_step_next(cpu);

    assert(cpu->mmu->mmu_get_byte(cpu->mmu, 0xC100) == 0x80);
    assert(cpu->registers->get_flag(cpu->registers, Flag_C) == 1);

    DELETE_ALL_COMPONENTS
}

// CB 0x10: RL B
void test_opcode_cb_10()
{
    CREATE_ALL_COMPONENTS

    // 0x85 is 0b1000_0101
    cpu->registers->set_register_byte(cpu->registers, B, 0x85);
    cpu->registers->set_flag(cpu->registers, Flag_C, 0);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xCB);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x10);
    cpu->cpu_step_next(cpu);

    // 0x0A is 0b0000_1010
    assert(cpu->registers->get_register_byte(cpu->registers, B) == 0x0A);
    assert(cpu->registers->get_flag(cpu->registers, Flag_C) == 1);

    DELETE_ALL_COMPONENTS
}

// CB 0x16: RL (HL)
void test_opcode_cb_16()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_pair(cpu->registers, HL, 0xC100);
    cpu->mmu->mmu_set_byte(cpu->mmu, 0xC100, 0x85);
    cpu->registers->set_flag(cpu->registers, Flag_C, 0);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xCB);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x16);
    cpu->cpu_step_next(cpu);

    assert(cpu->mmu->mmu_get_byte(cpu->mmu, 0xC100) == 0x0A);
    assert(cpu->registers->get_flag(cpu->registers, Flag_C) == 1);

    DELETE_ALL_COMPONENTS
}

// CB 0x20: SLA B
void test_opcode_cb_20()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, B, 0x85);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xCB);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x20);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, B) == 0x0A);
    assert(cpu->registers->get_flag(cpu->registers, Flag_C) == 1);

    DELETE_ALL_COMPONENTS
}

// CB 0x26: SLA (HL)
void test_opcode_cb_26()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_pair(cpu->registers, HL, 0xC100);
    cpu->mmu->mmu_set_byte(cpu->mmu, 0xC100, 0x85);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xCB);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x26);
    cpu->cpu_step_next(cpu);

    assert(cpu->mmu->mmu_get_byte(cpu->mmu, 0xC100) == 0x0A);
    assert(cpu->registers->get_flag(cpu->registers, Flag_C) == 1);

    DELETE_ALL_COMPONENTS
}

// CB 0x28: SRA B
void test_opcode_cb_28()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, B, 0x81);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xCB);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x28);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, B) == 0xC0);
    assert(cpu->registers->get_flag(cpu->registers, Flag_C) == 1);

    DELETE_ALL_COMPONENTS
}

// CB 0x2E: SRA (HL)
void test_opcode_cb_2e()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_pair(cpu->registers, HL, 0xC100);
    cpu->mmu->mmu_set_byte(cpu->mmu, 0xC100, 0x81);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xCB);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x2E);
    cpu->cpu_step_next(cpu);

    assert(cpu->mmu->mmu_get_byte(cpu->mmu, 0xC100) == 0xC0);
    assert(cpu->registers->get_flag(cpu->registers, Flag_C) == 1);

    DELETE_ALL_COMPONENTS
}

// CB 0x30: SWAP B
void test_opcode_cb_30()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, B, 0xF0);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xCB);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x30);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, B) == 0x0F);
    assert(cpu->registers->get_flag(cpu->registers, Flag_Z) == 0);

    DELETE_ALL_COMPONENTS
}

// CB 0x36: SWAP (HL)
void test_opcode_cb_36()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_pair(cpu->registers, HL, 0xC100);
    cpu->mmu->mmu_set_byte(cpu->mmu, 0xC100, 0xF0);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xCB);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x36);
    cpu->cpu_step_next(cpu);

    assert(cpu->mmu->mmu_get_byte(cpu->mmu, 0xC100) == 0x0F);
    assert(cpu->registers->get_flag(cpu->registers, Flag_Z) == 0);

    DELETE_ALL_COMPONENTS
}

// CB 0x38: SRL B
void test_opcode_cb_38()
{
    CREATE_ALL_COMPONENTS

    // 0x81 is 0b1000_0001
    cpu->registers->set_register_byte(cpu->registers, B, 0x81);
    // clear carry flag
    cpu->registers->set_flag(cpu->registers, Flag_C, 0);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xCB);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x38);
    cpu->cpu_step_next(cpu);

    // 0x40 is 0b0100_0000
    assert(cpu->registers->get_register_byte(cpu->registers, B) == 0x40);
    assert(cpu->registers->get_flag(cpu->registers, Flag_C) == 1);

    DELETE_ALL_COMPONENTS
}

// CB 0x3E: SRL (HL)
void test_opcode_cb_3e()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_pair(cpu->registers, HL, 0xC100);
    cpu->mmu->mmu_set_byte(cpu->mmu, 0xC100, 0x81);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xCB);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x3E);
    cpu->cpu_step_next(cpu);

    assert(cpu->mmu->mmu_get_byte(cpu->mmu, 0xC100) == 0x40);
    assert(cpu->registers->get_flag(cpu->registers, Flag_C) == 1);

    DELETE_ALL_COMPONENTS
}

// CB 0x40: BIT 0, B
void test_opcode_cb_40()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, B, 0x01);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xCB);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x40);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_flag(cpu->registers, Flag_Z) == 0);
    assert(cpu->registers->get_flag(cpu->registers, Flag_N) == 0);
    assert(cpu->registers->get_flag(cpu->registers, Flag_H) == 1);

    DELETE_ALL_COMPONENTS
}

// CB 0x46: BIT 0, (HL)
void test_opcode_cb_46()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_pair(cpu->registers, HL, 0xC100);
    cpu->mmu->mmu_set_byte(cpu->mmu, 0xC100, 0x01);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xCB);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x46);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_flag(cpu->registers, Flag_Z) == 0);
    assert(cpu->registers->get_flag(cpu->registers, Flag_N) == 0);
    assert(cpu->registers->get_flag(cpu->registers, Flag_H) == 1);

    DELETE_ALL_COMPONENTS
}

// CB 0x80: RES 0, B
void test_opcode_cb_80()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, B, 0x01);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xCB);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x80);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, B) == 0x00);

    DELETE_ALL_COMPONENTS
}

// CB 0x86: RES 0, (HL)
void test_opcode_cb_86()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_pair(cpu->registers, HL, 0xC100);
    cpu->mmu->mmu_set_byte(cpu->mmu, 0xC100, 0x01);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xCB);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x86);
    cpu->cpu_step_next(cpu);

    assert(cpu->mmu->mmu_get_byte(cpu->mmu, 0xC100) == 0x00);

    DELETE_ALL_COMPONENTS
}

// CB 0xC0: SET 0, B
void test_opcode_cb_c0()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_byte(cpu->registers, B, 0x00);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xCB);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0xC0);
    cpu->cpu_step_next(cpu);

    assert(cpu->registers->get_register_byte(cpu->registers, B) == 0x01);

    DELETE_ALL_COMPONENTS
}

// CB 0xC6: SET 0, (HL)
void test_opcode_cb_c6()
{
    CREATE_ALL_COMPONENTS

    cpu->registers->set_register_pair(cpu->registers, HL, 0xC100);
    cpu->mmu->mmu_set_byte(cpu->mmu, 0xC100, 0x00);

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xCB);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0xC6);
    cpu->cpu_step_next(cpu);

    assert(cpu->mmu->mmu_get_byte(cpu->mmu, 0xC100) == 0x01);

    DELETE_ALL_COMPONENTS
}

int main()
{
    config.start_time = get_time_in_seconds();
    config.verbose_level = TRACE_LEVEL;
    printf("=========================\n");
    printf("CPU Test\n");
    printf("=========================\n");

    // create Cartridge
    struct Cartridge *cartridge = create_cartridge();

    // create Ram
    struct Ram *ram = create_ram();

    // create Vram
    struct Vram *vram = create_vram();

    // create Ppu
    struct PPU *ppu = create_ppu(vram);

    // create MMU
    struct MMU *mmu = create_mmu(cartridge, ram, ppu);

    // create Registers
    struct Registers *registers = create_registers();

    // create CPU
    struct CPU *cpu = create_cpu(registers, mmu);

    // test cpu
    CPU_DEBUG_PRINT("cpu: %p\n", cpu);

    // test instruction tape:
    uint16_t pc = 0xcafe;
    // set pc to start of instruction tape
    cpu->registers->set_control_register(cpu->registers, PC, pc);
    assert(cpu->registers->get_control_register(cpu->registers, PC) == pc);
    CPU_DEBUG_PRINT("PC is set to: 0x%04x\n", pc);
    // 0x00: NOP: no operation
    // 0x01: LD BC, nn (imm) 0xde 0xad: load 0xdead into BC register pair
    // 0x02: LD (BC), A: load A register into address 0xdead
    // 0x03: INC BC: increment BC register pair to 0xdead + 1 (0xdeae)
    uint8_t instruction_tape[] = {0x00, 0x01, 0x00, 0x00, 0x02, 0x03};
    // copy instruction tape to cpu memory
    memcpy(cpu->mmu->ram->ram_byte + pc, instruction_tape, sizeof(instruction_tape));
    // set 0xdead into that address
    cpu->mmu->ram->set_ram_word(cpu->mmu->ram, pc + 2, 0xdead);

    assert(cpu->mmu->ram->ram_byte[pc] == 0x00);
    assert(cpu->mmu->ram->ram_byte[pc + 1] == 0x01);
    assert(cpu->mmu->ram->get_ram_word(cpu->mmu->ram, pc + 2) == 0xdead);
    assert(cpu->mmu->ram->ram_byte[pc + 4] == 0x02);
    assert(cpu->mmu->ram->ram_byte[pc + 5] == 0x03);

    // execute instructions
    // NOP
    cpu->cpu_step_next(cpu);

    // LD BC, nn (imm) 0xde 0xad
    cpu->cpu_step_next(cpu);
    assert(cpu->registers->get_register_pair(cpu->registers, BC) == 0xdead);

    // LD (BC), A
    cpu->cpu_step_next(cpu);
    assert(cpu->mmu->ram->get_ram_byte(cpu->mmu->ram, cpu->registers->get_register_pair(cpu->registers, BC)) == 0x01);

    // INC BC
    cpu->cpu_step_next(cpu);
    assert(cpu->registers->get_register_pair(cpu->registers, BC) == 0xdead + 1);

    // delete all then re-create
    free_cpu(cpu);

    CPU_INFO_PRINT("Begin normal test\n");

    // begin normal test
    test_opcode_01();
    test_opcode_02();
    test_opcode_03();
    test_opcode_04();
    test_opcode_05();
    test_opcode_06();
    test_opcode_07();
    test_opcode_08();
    test_opcode_09();
    test_opcode_0a();
    test_opcode_0b();
    test_opcode_0f();
    test_opcode_17();
    test_opcode_18();
    test_opcode_1f();
    test_opcode_20();
    test_opcode_22();
    test_opcode_3a();
    test_opcode_3b();
    test_opcode_80();
    test_opcode_86();
    test_opcode_88();
    test_opcode_8e();
    test_opcode_90();
    test_opcode_96();
    test_opcode_98();
    test_opcode_9e();
    test_opcode_a0();
    test_opcode_a6();
    test_opcode_a8();
    test_opcode_ae();
    test_opcode_b0();
    test_opcode_b6();
    test_opcode_b8();
    test_opcode_be();
    test_opcode_c0();
    test_opcode_c1();
    test_opcode_c2();
    test_opcode_c4();
    test_opcode_c8();
    test_opcode_c9();
    test_opcode_ca();
    test_opcode_cc();
    test_opcode_cd();
    test_opcode_ce();
    test_opcode_d0();
    test_opcode_d1();
    test_opcode_d2();
    test_opcode_d4();
    test_opcode_d5();
    test_opcode_d6();
    test_opcode_d8();
    test_opcode_d9();
    test_opcode_da();
    test_opcode_dc();
    test_opcode_de();
    test_opcode_e0();
    test_opcode_e1();
    test_opcode_e2();
    test_opcode_e5();
    test_opcode_e6();
    test_opcode_e8();
    test_opcode_e9();
    test_opcode_ea();
    test_opcode_f0();
    test_opcode_f1();
    test_opcode_f2();
    test_opcode_f5();
    test_opcode_f6();
    test_opcode_f8();
    test_opcode_f9();
    test_opcode_fa();
    test_opcode_fe();
    CPU_INFO_PRINT("Normal test completed\n");
    // Add CB prefix tests
    test_opcode_cb_00();
    test_opcode_cb_06();
    test_opcode_cb_08();
    test_opcode_cb_0e();
    test_opcode_cb_10();
    test_opcode_cb_16();
    test_opcode_cb_20();
    test_opcode_cb_26();
    test_opcode_cb_28();
    test_opcode_cb_2e();
    test_opcode_cb_30();
    test_opcode_cb_36();
    test_opcode_cb_38();
    test_opcode_cb_3e();
    test_opcode_cb_40();
    test_opcode_cb_46();
    test_opcode_cb_80();
    test_opcode_cb_86();
    test_opcode_cb_c0();
    test_opcode_cb_c6();
    CPU_INFO_PRINT("CB prefix test completed\n");
    return 0;
}
