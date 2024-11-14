#include "vram.h"

uint8_t vram_get_byte(struct Vram *self, uint16_t address)
{
    VRAM_TRACE_PRINT("VRAM_GET_BYTE: address: %d, value: 0x%02x\n", address, self->vram_byte[address]);
    return self->vram_byte[address];
}

void vram_set_byte(struct Vram *self, uint16_t address, uint8_t byte)
{
    self->vram_byte[address] = byte;
    VRAM_TRACE_PRINT("VRAM_SET_BYTE: address: %d, value: 0x%02x\n", address, byte);
}

uint16_t vram_get_word(struct Vram *self, uint16_t address)
{
    // Little endian: lower byte first, then higher byte
    uint16_t word = (uint16_t)(self->vram_byte[address] | (self->vram_byte[address + 1] << 8));
    VRAM_TRACE_PRINT("VRAM_GET_WORD: address: %d, value: 0x%04x\n", address, word);
    return word;
}

void vram_set_word(struct Vram *self, uint16_t address, uint16_t word)
{
    // Little endian: lower byte first, then higher byte
    self->vram_byte[address] = word & 0xFF;
    self->vram_byte[address + 1] = (word >> 8) & 0xFF;
    VRAM_TRACE_PRINT("VRAM_SET_WORD: address: %d, value: 0x%04x\n", address, word);
}

struct Vram *create_vram(void)
{
    struct Vram *vram = (struct Vram *)malloc(sizeof(struct Vram));
    vram->get_vram_byte = vram_get_byte;
    vram->set_vram_byte = vram_set_byte;
    vram->get_vram_word = vram_get_word;
    vram->set_vram_word = vram_set_word;
    // Initialize vram array to 0
    VRAM_TRACE_PRINT("VRAM_CREATE: Initializing vram array to 0\n");
    memset(vram->vram_byte, 0, VRAM_SIZE);
    // set method pointers
    vram->get_vram_byte = vram_get_byte;
    vram->set_vram_byte = vram_set_byte;
    vram->get_vram_word = vram_get_word;
    vram->set_vram_word = vram_set_word;
    return vram;
}

void free_vram(struct Vram *self)
{
    if (self)
    {
        free(self);
    }
}
