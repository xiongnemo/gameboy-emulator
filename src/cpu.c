#include "cpu.h"

struct CPU *create_cpu(struct Registers *registers, struct MMU *mmu)
{
    struct CPU *cpu = (struct CPU *)malloc(sizeof(struct CPU));
    if (cpu == NULL)
    {
        return NULL;
    }
    cpu->registers = registers;
    cpu->mmu = mmu;
    cpu->opcode_cycle_main = opcode_cycle_main;
    cpu->opcode_cycle_prefix_cb = opcode_cycle_prefix_cb;

    // initialize cpu state
    cpu->halted = false;
    cpu->stopped = false;
    cpu->ime = false;

    // set method pointers
    cpu->cpu_step_next = cpu_step_next;

    // initialize instruction table
    instruction_fn instruction_table[256] = {
        nop,
        ld_imm_to_register_pair,
        ld_register_to_address_register_pair,
        inc_register_pair,
    };

    cpu->instruction_table = instruction_table;

    struct InstructionParam instruction_param[256] = {
        {},
        {.rp_1 = BC},
        {.reg_1 = A, .rp_1 = BC},
        {.rp_1 = BC},
    };

    cpu->instruction_param = instruction_param;

    return cpu;
}

void free_cpu(struct CPU *cpu)
{
    if (cpu->registers)
    {
        free_registers(cpu->registers);
    }
    if (cpu->mmu)
    {
        free_mmu(cpu->mmu);
    }
    free(cpu);
}

// Private: Handle interrupts
uint8_t handle_interrupts(struct CPU *cpu)
{
    // TODO: Implement interrupt handling
    return 0;
}

uint8_t cpu_step_next(struct CPU *cpu)
{
    // 1. Check interrupts
    uint8_t interrupt_handled = handle_interrupts(cpu);
    if (interrupt_handled)
    {
        return interrupt_handled;
    }
    // 2. halted?
    if (cpu->halted)
    {
        return 1;
    }
    // 3. step next instruction
    return cpu_step(cpu);
}

uint8_t cpu_step_read_byte(struct CPU *cpu)
{
    // 1. Get byte from MMU
    uint8_t byte = cpu->mmu->mmu_get_byte(cpu->mmu, *(cpu->registers->pc));
    // 2. Increment PC
    *(cpu->registers->pc) += 1;
    return byte;
}

uint16_t cpu_step_read_word(struct CPU *cpu)
{
    // 1. Get word from MMU
    uint16_t word = cpu->mmu->mmu_get_word(cpu->mmu, *(cpu->registers->pc));
    // 2. Increment PC
    *(cpu->registers->pc) += 2;
    return word;
}

uint8_t cpu_step(struct CPU *cpu)
{
    // 1. Get Op Byte
    cpu->op_code = cpu_step_read_byte(cpu);
    // 2. Execute Op Code
    return cpu_step_execute_op_code(cpu, cpu->op_code);
}

uint8_t cpu_step_execute_op_code(struct CPU *cpu, uint8_t op_byte)
{
    if (op_byte == CB_PREFIX)
    {
        return cpu_step_execute_cb_prefix(cpu);
    }
    return cpu_step_execute_main(cpu, op_byte);
}

uint8_t cpu_step_execute_cb_prefix(struct CPU *cpu)
{
    // 1. Get Op Byte
    cpu->op_code = cpu_step_read_byte(cpu);
    // 2. Execute Op Code
    return cpu_step_execute_cb_op_code(cpu, cpu->op_code);
}

uint8_t cpu_step_execute_cb_op_code(struct CPU *cpu, uint8_t op_byte)
{
    return cpu->opcode_cycle_prefix_cb[op_byte];
}

void nop(struct CPU *cpu)
{
    CPU_DEBUG_PRINT("NOP\n");
}

void cpu_invalid_opcode(struct CPU *cpu, struct InstructionParam *param)
{
    CPU_EMERGENCY_PRINT("Invalid Opcode: %X\n", cpu->op_code);
    exit(EXIT_FAILURE);
}

void ld_imm_to_register_pair(struct CPU *cpu, struct InstructionParam *param)
{
    enum RegisterPair register_pair = param->rp_1;
    uint16_t value = cpu_step_read_word(cpu);
    cpu->registers->set_register_pair(cpu->registers, register_pair, value);
}

void ld_register_to_address_register_pair(struct CPU *cpu, struct InstructionParam *param)
{
    enum Register from_register = param->reg_1;
    enum RegisterPair to_register_pair = param->rp_1;
    uint16_t mmu_address = cpu->registers->get_register_pair(cpu->registers, to_register_pair);
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, from_register);
    cpu->mmu->mmu_set_byte(cpu->mmu, mmu_address, value);
}

void inc_register_pair(struct CPU *cpu, struct InstructionParam *param)
{
    enum RegisterPair register_pair = param->rp_1;
    uint16_t value = cpu->registers->get_register_pair(cpu->registers, register_pair);
    cpu->registers->set_register_pair(cpu->registers, register_pair, value + 1);
}

uint8_t cpu_step_execute_main(struct CPU *cpu, uint8_t op_byte)
{
    CPU_DEBUG_PRINT("Executing Op Code: %02x\n", op_byte);
    cpu->instruction_table[op_byte](cpu, &cpu->instruction_param[op_byte]);
    return cpu->opcode_cycle_main[op_byte];
}