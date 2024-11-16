#include "ram.h"

uint8_t ram_get_byte(struct Ram *self, uint16_t address)
{
    RAM_TRACE_PRINT("RAM_GET_BYTE: address: 0x%02x, value: 0x%02x\n", address, self->ram_byte[address]);
    return self->ram_byte[address];
}

void ram_set_byte(struct Ram *self, uint16_t address, uint8_t byte)
{
    self->ram_byte[address] = byte;
    RAM_TRACE_PRINT("RAM_SET_BYTE: address: 0x%02x, value: 0x%02x\n", address, byte);
}

uint16_t ram_get_word(struct Ram *self, uint16_t address)
{
    // Little endian: lower byte first, then higher byte
    uint16_t value = (uint16_t)(self->ram_byte[address] | (self->ram_byte[address + 1] << 8));
    RAM_TRACE_PRINT("RAM_GET_WORD: address: 0x%02x, value: 0x%04x\n", address, value);
    return value;
}

void ram_set_word(struct Ram *self, uint16_t address, uint16_t word)
{
    // Little endian: lower byte first, then higher byte
    self->ram_byte[address] = word & 0xFF;
    self->ram_byte[address + 1] = (word >> 8) & 0xFF;
    RAM_TRACE_PRINT("RAM_SET_WORD: address: 0x%02x, value: 0x%04x\n", address, word);
}

struct Ram *create_ram(void)
{
    struct Ram *mem = (struct Ram *)malloc(sizeof(struct Ram));
    mem->get_ram_byte = ram_get_byte;
    mem->set_ram_byte = ram_set_byte;
    mem->get_ram_word = ram_get_word;
    mem->set_ram_word = ram_set_word;
    // Initialize ram array to 0
    RAM_TRACE_PRINT("RAM_CREATE: Initializing ram array to 0%s", "\n");
    memset(mem->ram_byte, 0, sizeof(mem->ram_byte));
    // set method pointers
    mem->get_ram_byte = ram_get_byte;
    mem->set_ram_byte = ram_set_byte;
    mem->get_ram_word = ram_get_word;
    mem->set_ram_word = ram_set_word;
    return mem;
}

void free_ram(struct Ram *self)
{
    if (self)
    {
        free(self);
    }
}

