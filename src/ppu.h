#ifndef GAMEBOY_PPU_H
#define GAMEBOY_PPU_H

#include "general.h"
#include "mmu.h"
#include "vram.h"

// PPU Constants
#define SCREEN_WIDTH               160
#define SCREEN_HEIGHT              144
#define PIXELS_PER_TILELINE        8
#define OAM_TABLE_INITIAL_ADDDRESS 0xFE00

// FIFO constants
#define FIFO_SIZE 16
#define PIXELS_PER_TILE 8

// PPU Modes
enum PPU_MODE
{
    MODE_HBLANK         = 0,
    MODE_VBLANK         = 1,
    MODE_OAM_SEARCH     = 2,
    MODE_PIXEL_TRANSFER = 3
};

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
#define MODE_0_CYCLES_MIN 87    // H-Blank minimum
#define MODE_0_CYCLES_MAX 204   // H-Blank maximum  
#define MODE_1_CYCLES -1        // V-Blank, never used
#define MODE_2_CYCLES 80        // OAM Scan
#define MODE_3_CYCLES_MIN 172   // Pixel Transfer minimum
#define MODE_3_CYCLES_MAX 289   // Pixel Transfer maximum

// Total scanline cycles (always 456)
#define CYCLES_PER_SCANLINE 456
#define SCANLINES_PER_FRAME 154
#define VISIBLE_SCANLINES 144

// LCD Control Register (LCDC) bits
// Bit7: LCD Display Enable
#define LCDC_ENABLE     0x80
// Bit6: Window Tile Map Select
#define LCDC_WINDOW_MAP 0x40
// Bit5: Window Display Enable
#define LCDC_WINDOW_ON  0x20
// Bit4: BG & Window Tile Data Select
#define LCDC_TILE_DATA  0x10
// Bit3: BG Tile Map Select
#define LCDC_BG_MAP     0x08
// Bit2: OBJ Size
#define LCDC_OBJ_SIZE   0x04
// Bit1: OBJ Display Enable
#define LCDC_OBJ_ON     0x02
// Bit0: BG Display Enable
#define LCDC_BG_ON      0x01

// LCD Status Register (STAT) bits
// Bit7: LYC Interrupt Enable
#define STAT_LYC_INT    0x40
// Bit6: Mode 2 Interrupt Enable
#define STAT_MODE_2_INT 0x20
// Bit5: Mode 1 Interrupt Enable
#define STAT_MODE_1_INT 0x10
// Bit4: Mode 0 Interrupt Enable
#define STAT_MODE_0_INT 0x08
// Bit3: LYC Interrupt Flag
#define STAT_LYC_EQUAL  0x04
// Bit2-1: Mode Bits
#define STAT_MODE_MASK  0x03

// Pixel FIFO pixel structure
struct FIFOPixel {
    uint8_t color_raw;  // RAW color index (0-3) before palette application
    uint8_t palette;    // Which palette to use (0=BGP, 1=OBP0, 2=OBP1)
    uint8_t priority;   // Sprite priority (0=above BG, 1=behind BG color 1-3)
    bool    is_sprite;  // True if this is a sprite pixel
};

// FIFO structure
struct PixelFIFO {
    struct FIFOPixel pixels[FIFO_SIZE];
    uint8_t head;       // Index of first pixel
    uint8_t tail;       // Index of last pixel
    uint8_t size;       // Number of pixels in FIFO
};

// Background fetcher states
enum FetcherState {
    FETCH_TILE_NUMBER,
    FETCH_TILE_DATA_LOW,
    FETCH_TILE_DATA_HIGH,
    FETCH_PUSH
};

// Background pixel fetcher
struct BackgroundFetcher {
    enum FetcherState state;
    uint8_t tile_number;
    uint8_t tile_data_low;
    uint8_t tile_data_high;
    uint8_t fetch_x;        // Current X position being fetched
    uint8_t map_x;          // X position in tilemap
    uint8_t map_y;          // Y position in tilemap
    uint8_t tile_x;         // X position within tile (0-7)
    uint8_t tile_y;         // Y position within tile (0-7)
    bool    window_active;  // True if currently fetching window
};

// Sprite fetcher
struct SpriteFetcher {
    bool    active;
    uint8_t sprite_index;
    uint8_t tile_data_low;
    uint8_t tile_data_high;
    enum FetcherState state;
};

// Sprite blocks have the following
// format:
struct SpriteEntry
{
    // Byte0 Y position on the screen
    uint8_t y;
    // Byte1 X position on the screen
    uint8_t x;
    // Byte2 Pattern number 0-255 (Unlike some tile
    // numbers, sprite pattern numbers are unsigned.
    // LSB is ignored (treated as 0) in 8x16 mode.)
    uint8_t tile_index;
    // Byte3 Flags:
    // Bit7 Priority
    // If this bit is set to 0, sprite is
    // displayed on top of background & window.
    // If this bit is set to 1, then sprite
    // will be hidden behind colors 1, 2, and 3
    // of the background & window. (Sprite only
    // prevails over color 0 of BG & win.)
    uint8_t priority : 1;
    // Bit6 Y flip
    // Sprite pattern is flipped vertically if
    // this bit is set to 1.
    uint8_t y_flip   : 1;
    // Bit5 X flip
    // Sprite pattern is flipped horizontally
    // if this bit is set to 1.
    uint8_t x_flip   : 1;
    // Bit4 Palette number
    // Sprite colors are taken from OBJ1PAL
    // if this bit is set to 1 and
    // from OBJ0PAL otherwise.
    uint8_t palette  : 1;
};

struct PPU
{
    // Internal state
    uint32_t      ppu_inner_clock;    // clock for a complete screen frame
    enum PPU_MODE mode;               // current mode
    struct Vram*  vram;               // VRAM
    struct MMU*   mmu;                // only for r/w registers
    struct Form*  form;               // form for drawing
    uint8_t*      framebuffer;        // Screen resolution 160x144

    // these shouldn't change during drawing
    uint8_t ly;
    uint8_t lcdc;
    uint8_t stat;
    uint8_t scx;
    uint8_t scy;
    uint8_t wy;
    uint8_t wx;
    uint8_t bgp;
    uint8_t obp0;
    uint8_t obp1;
    uint16_t tile_map_base_address;
    uint16_t tile_data_base_address;

    // line buffer - BG & Window - 160 pixels
    uint8_t line_buffer_bg_and_window[160];

    // line buffer - Sprite - 160 pixels
    // 2 bytes for each pixel:
    // [0] = color index
    // [1] = sprite index
    uint8_t line_buffer_sprite[160][2];

    // final line buffer
    uint8_t line_buffer[160];

    // Pixel FIFO pipeline
    struct PixelFIFO background_fifo;
    struct PixelFIFO sprite_fifo;
    struct BackgroundFetcher bg_fetcher;
    struct SpriteFetcher sprite_fetcher;
    
    // FIFO rendering state
    uint8_t fifo_x;         // Current X position being rendered
    uint8_t pushed_pixels;  // Number of pixels pushed this scanline
    bool    window_triggered; // True if window has been triggered this scanline

    // Scanline OAM buffer (40 entries)
    struct SpriteEntry* oam_buffer;
    // Scanline selected OAM entries
    // (10 entries if we replicate the
    // original behavior, but 40 for now)
    struct SpriteEntry** selected_oam_entries;
    // searched sprites
    uint8_t searched_sprite_count;
    // Public method pointers
    void (*ppu_step)(struct PPU*, uint8_t cycles);
};

// Function declarations
// render single scanline with given ly
void ppu_render_scanline_ly(struct PPU* self, uint8_t ly);
// render full frame
void ppu_render_full_frame(struct PPU* self);
// helper functions for full frame rendering
void ppu_render_background_scanline(struct PPU* self, uint8_t ly, uint8_t scx, uint8_t scy, 
                                   uint16_t tile_map, uint16_t tile_data_base, bool signed_addressing, uint8_t bgp);
void ppu_render_window_scanline(struct PPU* self, uint8_t ly, uint8_t wx, uint8_t wy,
                               uint16_t tile_map, uint16_t tile_data_base, bool signed_addressing, uint8_t bgp);
void ppu_render_sprites_scanline(struct PPU* self, uint8_t ly, uint8_t lcdc, uint8_t obp0, uint8_t obp1);
// update STAT register
void ppu_update_stat(struct PPU* self);
// check if LCD is enabled
bool ppu_is_lcd_enabled(struct PPU* self);
// create PPU
struct PPU* create_ppu(struct Vram* vram);
// attach mmu to ppu
void ppu_attach_mmu(struct PPU* self, struct MMU* mmu);
// attach form to ppu
void ppu_attach_form(struct PPU* self, struct Form* form);
// free PPU
void free_ppu(struct PPU* ppu);
// set mode
void ppu_set_mode(struct PPU* self, enum PPU_MODE mode);
// set LY
void ppu_set_ly(struct PPU* self, uint8_t ly);

// PPU States
// OAM Search   
void    ppu_oam_search(struct PPU* self);
// set mode
void    ppu_set_mode(struct PPU* self, enum PPU_MODE mode);
// reset interrupt registers
void    ppu_reset_interrupt_registers(struct PPU* self);
// mix tile colors
uint8_t ppu_mix_tile_colors(struct PPU* self, int palette, uint8_t color0, uint8_t color1);
// get real color
uint32_t ppu_get_real_color(uint8_t color);

// PPU internal methods
// check LY coincidence
void ppu_update_lyc(struct PPU* self);

// unpack sprite entry to supplied sprite entry pointer and sprite flag pointer
void ppu_unpack_sprite_entry(
    uint8_t sprite_entry_byte_1, uint8_t sprite_entry_byte_2, uint8_t sprite_entry_byte_3,
    uint8_t sprite_entry_byte_4, struct SpriteEntry* entry);
// draw line
void ppu_draw_line(struct PPU* self);
// draw background line
void ppu_draw_background_line(struct PPU* self);
// draw window line
void ppu_draw_window_line(struct PPU* self);
// draw sprite line
void ppu_draw_sprite_line(struct PPU* self);

// FIFO functions
void fifo_clear(struct PixelFIFO* fifo);
void fifo_push(struct PixelFIFO* fifo, struct FIFOPixel pixel);
struct FIFOPixel fifo_pop(struct PixelFIFO* fifo);
bool fifo_is_empty(struct PixelFIFO* fifo);
uint8_t fifo_size(struct PixelFIFO* fifo);

// Background fetcher functions
void bg_fetcher_reset(struct BackgroundFetcher* fetcher, uint8_t ly, uint8_t scx, uint8_t scy);
void bg_fetcher_step(struct PPU* ppu);
void bg_fetcher_push_pixels(struct PPU* ppu);

// Sprite fetcher functions
void sprite_fetcher_reset(struct SpriteFetcher* fetcher);
void sprite_fetcher_step(struct PPU* ppu, struct SpriteEntry* sprite);
void sprite_fetcher_push_pixels(struct PPU* ppu, struct SpriteEntry* sprite);

// FIFO scanline rendering
void ppu_render_scanline_fifo(struct PPU* self, uint8_t ly);
struct FIFOPixel ppu_mix_pixels(struct FIFOPixel bg_pixel, struct FIFOPixel sprite_pixel);

#endif
