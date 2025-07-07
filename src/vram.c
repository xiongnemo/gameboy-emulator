#include "vram.h"

uint8_t vram_get_byte(struct Vram* self, uint16_t address)
{
    uint16_t vram_index = address - 0x8000;
    VRAM_TRACE_PRINT(
        "VRAM_GET_BYTE: address: 0x%02x, vram_index: 0x%02x, value: 0x%02x\n", address, vram_index, self->vram_byte[vram_index]);
    return self->vram_byte[vram_index];
}

void vram_set_byte(struct Vram* self, uint16_t address, uint8_t byte)
{
    uint16_t vram_index = address - 0x8000;
    self->vram_byte[vram_index] = byte;
    VRAM_TRACE_PRINT("VRAM_SET_BYTE: address: 0x%02x, vram_index: 0x%02x, value: 0x%02x\n", address, vram_index, byte);
}

uint16_t vram_get_word(struct Vram* self, uint16_t address)
{
    uint16_t vram_index = address - 0x8000;
    // Little endian: lower byte first, then higher byte
    uint16_t word = (uint16_t)(self->vram_byte[vram_index] | (self->vram_byte[vram_index + 1] << 8));
    VRAM_TRACE_PRINT("VRAM_GET_WORD: address: 0x%02x, vram_index: 0x%02x, value: 0x%04x\n", address, vram_index, word);
    return word;
}

void vram_set_word(struct Vram* self, uint16_t address, uint16_t word)
{
    uint16_t vram_index = address - 0x8000;
    // Little endian: lower byte first, then higher byte
    self->vram_byte[vram_index]     = word & 0xFF;
    self->vram_byte[vram_index + 1] = (word >> 8) & 0xFF;
    VRAM_TRACE_PRINT("VRAM_SET_WORD: address: 0x%02x, vram_index: 0x%02x, value: 0x%04x\n", address, vram_index, word);
}

struct Vram* create_vram(void)
{
    struct Vram* vram   = (struct Vram*)malloc(sizeof(struct Vram));
    // Initialize vram array to 0
    VRAM_TRACE_PRINT("VRAM_CREATE: Initializing vram array to 0%s", "\n");
    memset(vram->vram_byte, 0, VRAM_SIZE);
    // set method pointers
    vram->vram_get_byte = vram_get_byte;
    vram->vram_set_byte = vram_set_byte;
    vram->vram_get_word = vram_get_word;
    vram->vram_set_word = vram_set_word;
    return vram;
}

void free_vram(struct Vram* self)
{
    if (self) {
        free(self);
    }
}
