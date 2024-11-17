#ifndef GAMEBOY_PPU_H
#define GAMEBOY_PPU_H

#include "general.h"
#include "vram.h"

extern struct EmulatorConfig config;

// PPU debug print
#define PPU_DEBUG_PRINT(fmt, ...)                                   \
    if (config.debug_mode && config.verbose_level >= DEBUG_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                    \
        PRINT_LEVEL(DEBUG_LEVEL);                                   \
        printf("PPU: ");                                            \
        printf(fmt, ##__VA_ARGS__);                                 \
    }

#define PPU_INFO_PRINT(fmt, ...)                                   \
    if (config.debug_mode && config.verbose_level >= INFO_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                   \
        PRINT_LEVEL(INFO_LEVEL);                                   \
        printf("PPU: ");                                           \
        printf(fmt, ##__VA_ARGS__);                                \
    }

#define PPU_TRACE_PRINT(fmt, ...)                                   \
    if (config.debug_mode && config.verbose_level >= TRACE_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                    \
        PRINT_LEVEL(TRACE_LEVEL);                                   \
        printf("PPU: ");                                            \
        printf(fmt, ##__VA_ARGS__);                                 \
    }

#define PPU_WARN_PRINT(fmt, ...)                                   \
    if (config.debug_mode && config.verbose_level >= WARN_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                   \
        PRINT_LEVEL(WARN_LEVEL);                                   \
        printf("PPU: ");                                           \
        printf(fmt, ##__VA_ARGS__);                                \
    }

#define PPU_ERROR_PRINT(fmt, ...)   \
    {                               \
        PRINT_TIME_IN_SECONDS();    \
        PRINT_LEVEL(ERROR_LEVEL);   \
        printf("PPU: ");            \
        printf(fmt, ##__VA_ARGS__); \
    }

#define PPU_EMERGENCY_PRINT(fmt, ...) \
    {                                 \
        PRINT_TIME_IN_SECONDS();      \
        PRINT_LEVEL(EMERGENCY_LEVEL); \
        printf("PPU: ");              \
        printf(fmt, ##__VA_ARGS__);   \
    }

// PPU Mode timing constants
#define MODE_0_CYCLES 204   // H-Blank
#define MODE_1_CYCLES 456   // V-Blank
#define MODE_2_CYCLES 80    // OAM Scan
#define MODE_3_CYCLES 172   // Drawing pixels

// LCD Control Register (LCDC) bits
#define LCDC_ENABLE     0x80
#define LCDC_WINDOW_MAP 0x40
#define LCDC_WINDOW_ON  0x20
#define LCDC_TILE_DATA  0x10
#define LCDC_BG_MAP     0x08
#define LCDC_OBJ_SIZE   0x04
#define LCDC_OBJ_ON     0x02
#define LCDC_BG_ON      0x01

// LCD Status Register (STAT) bits
#define STAT_LYC_INT    0x40
#define STAT_MODE_2_INT 0x20
#define STAT_MODE_1_INT 0x10
#define STAT_MODE_0_INT 0x08
#define STAT_LYC_EQUAL  0x04
#define STAT_MODE_MASK  0x03

struct PPU
{
    // LCD registers
    uint8_t lcdc;   // LCD Control
    uint8_t stat;   // LCD Status
    uint8_t scy;    // Scroll Y
    uint8_t scx;    // Scroll X
    uint8_t ly;     // LCD Y Coordinate
    uint8_t lyc;    // LY Compare
    uint8_t bgp;    // BG Palette Data
    uint8_t obp0;   // Object Palette 0
    uint8_t obp1;   // Object Palette 1
    uint8_t wy;     // Window Y
    uint8_t wx;     // Window X

    // Internal state
    uint32_t     dot_clock;
    uint8_t      mode;
    struct Vram* vram;
    uint32_t     framebuffer[160 * 144];   // Screen resolution 160x144

    // Method pointers
    void (*step)(struct PPU*, uint8_t);
    void (*render_scanline)(struct PPU*);
    void (*update_stat)(struct PPU*);
    bool (*is_lcd_enabled)(struct PPU*);
};

// Function declarations
void        ppu_step(struct PPU* self, uint8_t cycles);
void        ppu_render_scanline(struct PPU* self);
void        ppu_update_stat(struct PPU* self);
bool        ppu_is_lcd_enabled(struct PPU* self);
struct PPU* create_ppu(struct Vram* vram);
void        free_ppu(struct PPU* ppu);

#endif
