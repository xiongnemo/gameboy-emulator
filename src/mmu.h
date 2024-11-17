#ifndef GAMEBOY_MMU_H
#define GAMEBOY_MMU_H

#include "cartridge.h"
#include "general.h"
#include "ppu.h"
#include "ram.h"
#include "timer.h"

extern struct EmulatorConfig config;

// MMU debug print
#define MMU_DEBUG_PRINT(fmt, ...)                                   \
    if (config.debug_mode && config.verbose_level >= DEBUG_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                    \
        PRINT_LEVEL(DEBUG_LEVEL);                                   \
        printf("MMU: ");                                            \
        printf(fmt, ##__VA_ARGS__);                                 \
    }

#define MMU_INFO_PRINT(fmt, ...)                                   \
    if (config.debug_mode && config.verbose_level >= INFO_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                   \
        PRINT_LEVEL(INFO_LEVEL);                                   \
        printf("MMU: ");                                           \
        printf(fmt, ##__VA_ARGS__);                                \
    }

#define MMU_TRACE_PRINT(fmt, ...)                                   \
    if (config.debug_mode && config.verbose_level >= TRACE_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                    \
        PRINT_LEVEL(TRACE_LEVEL);                                   \
        printf("MMU: ");                                            \
        printf(fmt, ##__VA_ARGS__);                                 \
    }

#define MMU_WARN_PRINT(fmt, ...)                                   \
    if (config.debug_mode && config.verbose_level >= WARN_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                   \
        PRINT_LEVEL(WARN_LEVEL);                                   \
        printf("MMU: ");                                           \
        printf(fmt, ##__VA_ARGS__);                                \
    }

#define MMU_ERROR_PRINT(fmt, ...)   \
    {                               \
        PRINT_TIME_IN_SECONDS();    \
        PRINT_LEVEL(ERROR_LEVEL);   \
        printf("MMU: ");            \
        printf(fmt, ##__VA_ARGS__); \
    }

#define MMU_EMERGENCY_PRINT(fmt, ...) \
    {                                 \
        PRINT_TIME_IN_SECONDS();      \
        PRINT_LEVEL(EMERGENCY_LEVEL); \
        printf("MMU: ");              \
        printf(fmt, ##__VA_ARGS__);   \
    }

// MMU struct
// Contains a cartridge and a ram
// Used to interface with the cartridge and ram
// Mapped addresses:
// 0x0000 - 0x3FFF: 16KB ROM Bank 0
// 0x4000 - 0x7FFF: 16KB ROM Bank 1
// 0x8000 - 0x9FFF: 8KB Video RAM
// 0xA000 - 0xBFFF: 8KB External RAM
// 0xC000 - 0xDFFF: 8KB RAM
// 0xE000 - 0xFDFF: 8KB Echo RAM
// 0xFE00 - 0xFE9F: 128B Sprite Attribute Table (OAM)
// 0xFEA0 - 0xFEFF: Not Usable
// 0xFF00 - 0xFF7F: 120B I/O Registers
// 0xFF80 - 0xFFFE: 127B High RAM
// 0xFFFF: Interrupt Enable Register
struct MMU
{
    struct Cartridge* cartridge;
    struct Ram*       ram;
    struct PPU*       ppu;

    // Public method pointers
    uint8_t (*mmu_get_byte)(struct MMU*, uint16_t address);
    void (*mmu_set_byte)(struct MMU*, uint16_t address, uint8_t byte);
    uint16_t (*mmu_get_word)(struct MMU*, uint16_t address);
    void (*mmu_set_word)(struct MMU*, uint16_t address, uint16_t word);
};

// Function declarations

// create mmu
// cpu can not access vram directly, so a ppu is needed
struct MMU* create_mmu(struct Cartridge* cartridge, struct Ram* ram, struct PPU* ppu);
// free mmu
void free_mmu(struct MMU* mmu);
// get byte
uint8_t mmu_get_byte(struct MMU* mmu, uint16_t address);
// set byte
void mmu_set_byte(struct MMU* mmu, uint16_t address, uint8_t byte);
// get word
uint16_t mmu_get_word(struct MMU* mmu, uint16_t address);
// set word
void mmu_set_word(struct MMU* mmu, uint16_t address, uint16_t word);

#endif
