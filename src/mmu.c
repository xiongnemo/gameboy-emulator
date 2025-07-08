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
    mmu->mmu_attach_joypad = mmu_attach_joypad;
    mmu->mmu_attach_apu = mmu_attach_apu;
    return mmu;
}

void free_mmu(struct MMU* mmu)
{
    if (mmu) {
        free_cartridge(mmu->cartridge);
        mmu->cartridge = NULL;
        free_ram(mmu->ram);
        mmu->ram = NULL;
        free_ppu(mmu->ppu);
        mmu->ppu = NULL;
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
    // // External RAM: from cartridge
    // else if (address >= 0xA000 && address <= 0xBFFF) {
    //     result.address = address;
    //     result.type    = CARTRIDGE;
    // }
    // Video RAM: from ppu
    else if (address >= 0x8000 && address <= 0x9FFF) {
        result.address = address;
        result.type    = PPU_VRAM;
    }
    // // OAM RAM: from ppu
    // else if (address >= 0xFE00 && address <= 0xFE9F) {
    //     result.address = address;
    //     result.type    = PPU_VRAM;
    // }
    // Unusable Memory
    else if (address >= 0xFEA0 && address <= 0xFEFF) {
        result.address = address; // Still pass the address, but handle in get/set
        result.type    = RAM;     // Temporarily treat as RAM, but will be special-cased
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
    // effectivly disable joypad
    if (address == 0xFF00 && config.disable_joypad) {
        return 0x3F;
    }
    
    // Handle APU registers (0xFF10-0xFF26, 0xFF30-0xFF3F)
    if (mmu->apu && ((address >= 0xFF10 && address <= 0xFF26) || 
                     (address >= 0xFF30 && address <= 0xFF3F))) {
        return mmu->apu->read_register(mmu->apu, address);
    }
    
    // Handle unusable memory region
    if (address >= 0xFEA0 && address <= 0xFEFF) {
        return 0x00; // Reads from this region should return 0x00 on DMG
    }

    struct AddressTranslationResult result = translate_address(address);
    if (result.type == CARTRIDGE) {
        return mmu->cartridge->get_cartridge_byte(mmu->cartridge, result.address);
    }
    else if (result.type == PPU_VRAM) {
        return mmu->ppu->vram->vram_get_byte(mmu->ppu->vram, result.address);
    }
    return mmu->ram->get_ram_byte(mmu->ram, result.address);
}

void mmu_attach_joypad(struct MMU* mmu, struct Joypad* joypad)
{
    mmu->joypad = joypad;
}

void mmu_attach_apu(struct MMU* mmu, struct APU* apu)
{
    mmu->apu = apu;
}

void mmu_set_byte(struct MMU* mmu, uint16_t address, uint8_t byte)
{
    // // Block writes to LY register - it's read-only for CPU
    // if (address == 0xFF44) { // LY_ADDRESS
    //     return; // LY register is read-only, managed by PPU
    // }
    
    // Handle APU registers (0xFF10-0xFF26, 0xFF30-0xFF3F)
    if (mmu->apu && ((address >= 0xFF10 && address <= 0xFF26) || 
                     (address >= 0xFF30 && address <= 0xFF3F))) {
        mmu->apu->write_register(mmu->apu, address, byte);
        return;
    }

    if (address == 0xFF00) {
        // controls
        bool controls_requested = ((byte >> 5) & 1) == 0;
        bool directions_requested = ((byte >> 4) & 1) == 0;
        if (controls_requested) {
            byte = 0x20 + mmu->joypad->keys_controls;
        }
        if (directions_requested) {
            byte = 0x10 + mmu->joypad->keys_directions;
        }
    }
    
    if (address == 0xFF46) {
        DMA(mmu, byte);
        return;
    }

    // Handle unusable memory region
    if (address >= 0xFEA0 && address <= 0xFEFF) {
        return; // Writes to this region have no effect
    }

    struct AddressTranslationResult result = translate_address(address);
    if (result.type == CARTRIDGE) {
        mmu->cartridge->set_cartridge_byte(mmu->cartridge, result.address, byte);
    }
    else if (result.type == PPU_VRAM) {
        mmu->ppu->vram->vram_set_byte(mmu->ppu->vram, result.address, byte);
    }
    else {
        mmu->ram->set_ram_byte(mmu->ram, result.address, byte);
    }
}

// DMA
// https://hacktix.github.io/GBEDG/dma/
// The original DMG Gameboy only features one type of DMA Transfer - the OAM DMA. This is used
// to copy a section of data to OAM memory. Many games have a separate "Shadow OAM" in RAM, which
// can be modified while the PPU is pushing pixels to the LCD, unlike "real" OAM memory. Then, once
// VBlank occurs, OAM DMA is executed, transferring the Shadow OAM data to real OAM in a
// time-efficient manner.
void DMA(struct MMU* mmu, uint8_t source_bank)
{
    MMU_TRACE_PRINT("DMA: source_bank = %d\n", source_bank);
    // copy source_bank to OAM memory
    for (int i = 0; i < 0xA0; i++) {
        mmu->mmu_set_byte(mmu, 0xFE00 + i, mmu->mmu_get_byte(mmu, source_bank * 0x100 + i));
    }
}

uint16_t mmu_get_word(struct MMU* mmu, uint16_t address)
{
    // Handle unusable memory region
    if (address >= 0xFEA0 && address <= 0xFEFF) {
        return 0x0000; // Reads from this region should return 0x00 on DMG
    }

    struct AddressTranslationResult result = translate_address(address);
    if (result.type == CARTRIDGE) {
        return mmu->cartridge->get_cartridge_word(mmu->cartridge, result.address);
    }
    else if (result.type == PPU_VRAM) {
        return mmu->ppu->vram->vram_get_word(mmu->ppu->vram, result.address);
    }
    return mmu->ram->get_ram_word(mmu->ram, result.address);
}

void mmu_set_word(struct MMU* mmu, uint16_t address, uint16_t word)
{
    // Handle unusable memory region
    if (address >= 0xFEA0 && address <= 0xFEFF) {
        return; // Writes to this region have no effect
    }

    struct AddressTranslationResult result = translate_address(address);
    if (result.type == CARTRIDGE) {
        mmu->cartridge->set_cartridge_word(mmu->cartridge, result.address, word);
    }
    else if (result.type == PPU_VRAM) {
        mmu->ppu->vram->vram_set_word(mmu->ppu->vram, result.address, word);
    }
    else {
        mmu->ram->set_ram_word(mmu->ram, result.address, word);
    }
}
