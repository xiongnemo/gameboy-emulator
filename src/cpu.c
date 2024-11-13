#include "cpu.h"

struct CPU* create_cpu(struct Registers* registers, struct MMU* mmu)
{
    struct CPU* cpu = (struct CPU*)malloc(sizeof(struct CPU));
    if (cpu == NULL)
    {
        return NULL;
    }
    cpu->registers = registers;
    cpu->mmu = mmu;
    return cpu;
}

void free_cpu(struct CPU* cpu)
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

