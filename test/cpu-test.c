#include "../src/cpu.h"
#include "test.h"

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

    // create Timer
    struct Timer *timer = create_timer();

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

    free_cpu(cpu);
    return 0;
}
