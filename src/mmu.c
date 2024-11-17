#include "mmu.h"

struct MMU* create_mmu(struct Cartridge* cartridge, struct Ram* ram, struct PPU* ppu)
{
    struct MMU* mmu = malloc(sizeof(struct MMU));
    mmu->cartridge  = cartridge;
    mmu->ram        = ram;
    mmu->ppu        = ppu;
    // set method pointers
    mmu->mmu_get_byte = mmu_get_byte;
    mmu->mmu_set_byte = mmu_set_byte;
    mmu->mmu_get_word = mmu_get_word;
    mmu->mmu_set_word = mmu_set_word;
    return mmu;
}

void free_mmu(struct MMU* mmu)
{
    if (mmu) {
        free_cartridge(mmu->cartridge);
        free_ram(mmu->ram);
        free_ppu(mmu->ppu);
        free(mmu);
    }
}

enum AddressType
{
    CARTRIDGE,
    RAM,
    PPU_VRAM
};

struct AddressTranslationResult
{
    uint16_t         address;
    enum AddressType type;
};

// translate address to:
// 1. where should it go - ram or cartridge?
// 2. address to use - real address or offset
struct AddressTranslationResult translate_address(uint16_t address)
{
    struct AddressTranslationResult result;
    // if address is less than 0x8000, it's a cartridge address
    if (address < 0x8000) {
        result.address = address;
        result.type    = CARTRIDGE;
    }
    // External RAM: from cartridge
    else if (address >= 0xA000 && address <= 0xBFFF) {
        result.address = address;
        result.type    = CARTRIDGE;
    }
    // Video RAM: from ppu
    else if (address >= 0x8000 && address <= 0x9FFF) {
        result.address = address;
        result.type    = PPU_VRAM;
    }
    // OAM RAM: from ppu
    else if (address >= 0xFE00 && address <= 0xFE9F) {
        result.address = address;
        result.type    = PPU_VRAM;
    }
    // Everything else is ram
    else {
        result.address = address;
        result.type    = RAM;
    }
    return result;
}


uint8_t mmu_get_byte(struct MMU* mmu, uint16_t address)
{
    struct AddressTranslationResult result = translate_address(address);
    if (result.type == CARTRIDGE) {
        return mmu->cartridge->get_cartridge_byte(mmu->cartridge, result.address);
    }
    else if (result.type == PPU_VRAM) {
        return mmu->ppu->vram->get_vram_byte(mmu->ppu->vram, result.address);
    }
    return mmu->ram->get_ram_byte(mmu->ram, result.address);
}

void mmu_set_byte(struct MMU* mmu, uint16_t address, uint8_t byte)
{
    struct AddressTranslationResult result = translate_address(address);
    if (result.type == CARTRIDGE) {
        mmu->cartridge->set_cartridge_byte(mmu->cartridge, result.address, byte);
    }
    else if (result.type == PPU_VRAM) {
        mmu->ppu->vram->set_vram_byte(mmu->ppu->vram, result.address, byte);
    }
    else {
        mmu->ram->set_ram_byte(mmu->ram, result.address, byte);
    }
}

uint16_t mmu_get_word(struct MMU* mmu, uint16_t address)
{
    struct AddressTranslationResult result = translate_address(address);
    if (result.type == CARTRIDGE) {
        return mmu->cartridge->get_cartridge_word(mmu->cartridge, result.address);
    }
    else if (result.type == PPU_VRAM) {
        return mmu->ppu->vram->get_vram_word(mmu->ppu->vram, result.address);
    }
    return mmu->ram->get_ram_word(mmu->ram, result.address);
}

void mmu_set_word(struct MMU* mmu, uint16_t address, uint16_t word)
{
    struct AddressTranslationResult result = translate_address(address);
    if (result.type == CARTRIDGE) {
        mmu->cartridge->set_cartridge_word(mmu->cartridge, result.address, word);
    }
    else if (result.type == PPU_VRAM) {
        mmu->ppu->vram->set_vram_word(mmu->ppu->vram, result.address, word);
    }
    else {
        mmu->ram->set_ram_word(mmu->ram, result.address, word);
    }
}
