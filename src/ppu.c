#include "ppu.h"

struct PPU* create_ppu(struct Vram* vram)
{
    struct PPU* ppu = (struct PPU*)malloc(sizeof(struct PPU));
    if (ppu == NULL) {
        return NULL;
    }

    // Initialize state
    ppu->ppu_inner_clock  = 0;
    ppu->mode             = MODE_OAM_SEARCH;   // Start in OAM scan mode
    ppu->vram             = vram;
    ppu->framebuffer      = (uint8_t*)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint8_t));
    if (ppu->framebuffer == NULL) {
        free(ppu);
        return NULL;
    }
    
    // Initialize framebuffer with black pixels (color 3)
    memset(ppu->framebuffer, 3, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint8_t));

    // Initialize line buffers
    memset(ppu->line_buffer_bg_and_window, 0, SCREEN_WIDTH);
    memset(ppu->line_buffer_sprite, 0, SCREEN_WIDTH * 2);
    memset(ppu->line_buffer, 0, SCREEN_WIDTH);

    // Initialize FIFO structures
    fifo_clear(&ppu->background_fifo);
    fifo_clear(&ppu->sprite_fifo);
    sprite_fetcher_reset(&ppu->sprite_fetcher);
    ppu->fifo_x = 0;
    ppu->pushed_pixels = 0;
    ppu->window_triggered = false;

    // allocate all OAM buffer now
    ppu->oam_buffer            = (struct SpriteEntry*)malloc(40 * sizeof(struct SpriteEntry));
    ppu->selected_oam_entries  = (struct SpriteEntry**)malloc(10 * sizeof(struct SpriteEntry*));
    if (ppu->oam_buffer == NULL || ppu->selected_oam_entries == NULL) {
        if (ppu->framebuffer) free(ppu->framebuffer);
        if (ppu->oam_buffer) free(ppu->oam_buffer);
        free(ppu);
        return NULL;
    }
    ppu->searched_sprite_count = 0;

    // Initialize default register values
    ppu->ly = 0;
    ppu->lcdc = 0xA1;  // LCD & BG enabled by default (with tile data from 0x8000)
    ppu->stat = 0;
    ppu->scx = 0;
    ppu->scy = 0;
    ppu->wy = 0;
    ppu->wx = 0;
    ppu->bgp = 0xE4;   // Default palette (11 10 01 00) - inverted for GB
    ppu->obp0 = 0xFF;
    ppu->obp1 = 0xFF;
    ppu->tile_map_base_address = 0x9800;
    ppu->tile_data_base_address = 0x9000;

    // Initialize public method pointers

    return ppu;
}

// attach mmu to ppu
void ppu_attach_mmu(struct PPU* self, struct MMU* mmu)
{
    self->mmu = mmu;
    
    // Write initial PPU register values to memory
    mmu->mmu_set_byte(mmu, LCDC_ADDRESS, self->lcdc);
    mmu->mmu_set_byte(mmu, STAT_ADDRESS, self->stat);
    mmu->mmu_set_byte(mmu, SCY_ADDRESS, self->scy);
    mmu->mmu_set_byte(mmu, SCX_ADDRESS, self->scx);
    mmu->mmu_set_byte(mmu, LY_ADDRESS, self->ly);
    mmu->mmu_set_byte(mmu, WY_ADDRESS, self->wy);
    mmu->mmu_set_byte(mmu, WX_ADDRESS, self->wx);
    mmu->mmu_set_byte(mmu, BGP_ADDRESS, self->bgp);
    mmu->mmu_set_byte(mmu, OBP0_ADDRESS, self->obp0);
    mmu->mmu_set_byte(mmu, OBP1_ADDRESS, self->obp1);
}

// attach form to ppu
void ppu_attach_form(struct PPU* self, struct Form* form)
{
    self->form = form;
}

bool ppu_is_lcd_enabled(struct PPU* self)
{
    uint8_t lcdc = self->mmu->mmu_get_byte(self->mmu, LCDC_ADDRESS);
    return (lcdc & LCDC_ENABLE) != 0;
}


void ppu_set_mode(struct PPU* self, enum PPU_MODE mode)
{
    // Read current STAT register
    uint8_t stat = self->mmu->mmu_get_byte(self->mmu, STAT_ADDRESS);
    // Update mode bits (preserve upper bits)
    stat = (stat & ~STAT_MODE_MASK) | mode;
    // Write back to memory
    self->mmu->mmu_set_byte(self->mmu, STAT_ADDRESS, stat);
    
    // Check for STAT mode interrupts
    bool trigger_stat_int = false;
    
    switch (mode) {
        case MODE_HBLANK:
            if (stat & STAT_MODE_0_INT) trigger_stat_int = true;
            break;
        case MODE_VBLANK:
            if (stat & STAT_MODE_1_INT) trigger_stat_int = true;
            break;
        case MODE_OAM_SEARCH:
            if (stat & STAT_MODE_2_INT) trigger_stat_int = true;
            break;
        case MODE_PIXEL_TRANSFER:
            // Mode 3 has no STAT interrupt
            break;
    }
    
    if (trigger_stat_int) {
        uint8_t int_flag = self->mmu->mmu_get_byte(self->mmu, IF_ADDRESS);
        int_flag |= 0x02; // Set bit 1 for STAT interrupt
        self->mmu->mmu_set_byte(self->mmu, IF_ADDRESS, int_flag);
    }
}


void ppu_oam_search(struct PPU* self)
{
    // OAM Search Mode according to GBEDG:
    // - Takes 80 T-cycles total (2 T-cycles per OAM entry)
    // - Searches all 40 OAM entries for sprites on current scanline
    // - Stores up to 10 sprites that meet visibility criteria
    // - Earlier sprites in OAM have higher priority
    
    // 1. Get the current line
    uint8_t ly = self->mmu->mmu_get_byte(self->mmu, LY_ADDRESS);
    // 2. Get sprite height (8 for normal, 16 for tall sprite mode)
    uint8_t sprite_height =
        8 * (((self->mmu->mmu_get_byte(self->mmu, LCDC_ADDRESS) & 0x04) >> 2) + 1);
    self->searched_sprite_count = 0;
    // populate all sprites from OAM: data from 0xFE00 to 0xFE9F
    for (int index = 0; index < 40; index++) {
        uint16_t            sprite_entry_address = OAM_TABLE_INITIAL_ADDDRESS + index * 4;
        struct SpriteEntry* current_sprite       = &self->oam_buffer[index];
        ppu_unpack_sprite_entry(
            self->mmu->mmu_get_byte(self->mmu, sprite_entry_address),
            self->mmu->mmu_get_byte(self->mmu, sprite_entry_address + 1),
            self->mmu->mmu_get_byte(self->mmu, sprite_entry_address + 2),
            self->mmu->mmu_get_byte(self->mmu, sprite_entry_address + 3),
            current_sprite);
        // Convert sprite coordinates to screen coordinates for consistent checking
        uint8_t sprite_y = current_sprite->y - 16;
        uint8_t sprite_x = current_sprite->x - 8;
        
        // Sprite visibility rules according to GBEDG:
        // - Sprite X-Position must be greater than 0
        // - LY + 16 must be greater than or equal to Sprite Y-Position  
        // - LY + 16 must be less than Sprite Y-Position + Sprite Height
        if (current_sprite->x == 0 || 
            ly < sprite_y || ly >= sprite_y + sprite_height) {
            continue;
        }
        // The amount of sprites already stored in the OAM Buffer must be less than 10
        if (self->searched_sprite_count >= 10) {
            break;  // Game Boy hardware limit: max 10 sprites per scanline
        }
        self->selected_oam_entries[self->searched_sprite_count] = current_sprite;
        (self->searched_sprite_count) += 1;
    }
}

// unpack sprite entry to supplied sprite entry pointer and sprite flag pointer
// SpriteEntry and SpriteFlag should be allocated and destroyed by the caller!
void ppu_unpack_sprite_entry(
    uint8_t sprite_entry_byte_1, uint8_t sprite_entry_byte_2, uint8_t sprite_entry_byte_3,
    uint8_t sprite_entry_byte_4, struct SpriteEntry* entry)
{
    // Byte 0: Y position
    entry->y = sprite_entry_byte_1;
    // Byte 1: X position
    entry->x = sprite_entry_byte_2;
    // Byte 2: Tile index
    entry->tile_index = sprite_entry_byte_3;
    // Byte 3: Flags
    uint8_t flags_byte = sprite_entry_byte_4;
    // Bit 7: Priority
    entry->priority = (flags_byte >> 7) & 0x01;
    // Bit 6: Y flip
    entry->y_flip = (flags_byte >> 6) & 0x01;
    // Bit 5: X flip
    entry->x_flip = (flags_byte >> 5) & 0x01;
    // Bit 4: Palette
    entry->palette = (flags_byte >> 4) & 0x01;
    return;
}

void ppu_set_ly(struct PPU* self, uint8_t ly)
{
    self->ly = ly;
    self->mmu->mmu_set_byte(self->mmu, LY_ADDRESS, ly);
    // check lyc
    uint8_t lyc_byte  = self->mmu->mmu_get_byte(self->mmu, LYC_ADDRESS);
    uint8_t stat_byte  = self->mmu->mmu_get_byte(self->mmu, STAT_ADDRESS);
    if (ly == lyc_byte) {
        stat_byte |= STAT_LYC_EQUAL;
        if (stat_byte & STAT_LYC_INT) {
            uint8_t int_flag = self->mmu->mmu_get_byte(self->mmu, IF_ADDRESS);
            int_flag |= 0x02;
            self->mmu->mmu_set_byte(self->mmu, IF_ADDRESS, int_flag);
        }
    }
    else {
        stat_byte &= ~STAT_LYC_EQUAL;
    }

    self->mmu->mmu_set_byte(self->mmu, STAT_ADDRESS, stat_byte);
}

// Render single scanline with given ly parameter
void ppu_render_scanline_ly(struct PPU* self, uint8_t ly)
{
    // Only render visible scanlines
    if (ly >= SCREEN_HEIGHT) {
        return;
    }

    // Load PPU state for this scanline
    uint8_t lcdc = self->mmu->mmu_get_byte(self->mmu, LCDC_ADDRESS);
    uint8_t scx  = self->mmu->mmu_get_byte(self->mmu, SCX_ADDRESS);
    uint8_t scy  = self->mmu->mmu_get_byte(self->mmu, SCY_ADDRESS);
    uint8_t wy   = self->mmu->mmu_get_byte(self->mmu, WY_ADDRESS);
    uint8_t wx   = self->mmu->mmu_get_byte(self->mmu, WX_ADDRESS);
    uint8_t bgp  = self->mmu->mmu_get_byte(self->mmu, BGP_ADDRESS);
    uint8_t obp0 = self->mmu->mmu_get_byte(self->mmu, OBP0_ADDRESS);
    uint8_t obp1 = self->mmu->mmu_get_byte(self->mmu, OBP1_ADDRESS);
    
    // Calculate base addresses
    uint16_t tile_map_base_address  = (lcdc & LCDC_BG_MAP) ? 0x9C00 : 0x9800;
    uint16_t tile_data_base_address = (lcdc & LCDC_TILE_DATA) ? 0x8000 : 0x9000;
    bool     signed_addressing = !(lcdc & LCDC_TILE_DATA);

    // Clear line buffers
    memset(self->line_buffer_bg_and_window, 0, SCREEN_WIDTH);
    memset(self->line_buffer_sprite, 0, SCREEN_WIDTH * 2);
    memset(self->line_buffer, 0, SCREEN_WIDTH);

    // Draw background for this line
    if (lcdc & LCDC_BG_ON) {
        ppu_render_background_scanline(self, ly, scx, scy, tile_map_base_address, tile_data_base_address, signed_addressing, bgp);
    }

    // Draw window for this line
    if (lcdc & LCDC_WINDOW_ON) {
        // Window uses its own tile map selection bit (bit 6 of LCDC)
        uint16_t window_tile_map = (lcdc & LCDC_WINDOW_MAP) ? 0x9C00 : 0x9800;
        ppu_render_window_scanline(self, ly, wx, wy, window_tile_map, tile_data_base_address, signed_addressing, bgp);
    }

    // Draw sprites for this line
    if (lcdc & LCDC_OBJ_ON) {
        ppu_render_sprites_scanline(self, ly, lcdc, obp0, obp1);
    }

    // Merge layers and copy to framebuffer with proper sprite priority using RAW color indices
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        uint8_t bg_color_raw = self->line_buffer_bg_and_window[x];      // Raw BG color index (0-3)
        uint8_t sprite_color_raw = self->line_buffer_sprite[x][0];      // Raw sprite color index (0-3)
        uint8_t sprite_info = self->line_buffer_sprite[x][1];
        
        // LCDC.0 - BG/Window Enable: If disabled, background shows as white (color 0)
        if (!(lcdc & LCDC_BG_ON)) {
            bg_color_raw = 0;  // Force background to white when BG/Window disabled
        }
        
        // Determine final raw color index using Game Boy priority rules
        uint8_t final_color_raw;
        uint8_t final_palette;
        
        // Handle sprite-to-background priority using RAW color indices
        if (sprite_color_raw != 0) {
            // Extract sprite priority and palette info
            bool sprite_priority_behind = (sprite_info & 0x80) != 0;
            bool sprite_uses_obp1 = (sprite_info & 0x40) != 0;
            
            if (sprite_priority_behind) {
                // Priority 1: Sprite appears behind background colors 1, 2, 3 (but above color 0)
                if (bg_color_raw == 0) {
                    final_color_raw = sprite_color_raw;
                    final_palette = sprite_uses_obp1 ? obp1 : obp0;  // Use sprite palette
                } else {
                    final_color_raw = bg_color_raw;
                    final_palette = bgp;  // Use background palette
                }
            } else {
                // Priority 0: Sprite appears above background
                final_color_raw = sprite_color_raw;
                final_palette = sprite_uses_obp1 ? obp1 : obp0;  // Use sprite palette
            }
        } else {
            // No sprite pixel, use background
            final_color_raw = bg_color_raw;
            final_palette = bgp;  // Use background palette
        }
        
        // Apply the appropriate palette to get final display color
        uint8_t final_display_color = (final_palette >> (final_color_raw * 2)) & 0x03;
        self->line_buffer[x] = final_display_color;
    }
    
    // Copy line to framebuffer
    memcpy(self->framebuffer + ly * SCREEN_WIDTH, self->line_buffer, SCREEN_WIDTH);
}

// // Render full frame (160x144) to framebuffer - called during V-Blank
// void ppu_render_full_frame(struct PPU* self)
// {
//     // Load PPU state once for the entire frame
//     uint8_t lcdc = self->mmu->mmu_get_byte(self->mmu, LCDC_ADDRESS);
//     uint8_t scx  = self->mmu->mmu_get_byte(self->mmu, SCX_ADDRESS);
//     uint8_t scy  = self->mmu->mmu_get_byte(self->mmu, SCY_ADDRESS);
//     uint8_t wy   = self->mmu->mmu_get_byte(self->mmu, WY_ADDRESS);
//     uint8_t wx   = self->mmu->mmu_get_byte(self->mmu, WX_ADDRESS);
//     uint8_t bgp  = self->mmu->mmu_get_byte(self->mmu, BGP_ADDRESS);
//     uint8_t obp0 = self->mmu->mmu_get_byte(self->mmu, OBP0_ADDRESS);
//     uint8_t obp1 = self->mmu->mmu_get_byte(self->mmu, OBP1_ADDRESS);
    
//     // Calculate base addresses
//     uint16_t tile_map_base_address  = (lcdc & LCDC_BG_MAP) ? 0x9C00 : 0x9800;
//     uint16_t tile_data_base_address = (lcdc & LCDC_TILE_DATA) ? 0x8000 : 0x9000;
//     bool     signed_addressing = !(lcdc & LCDC_TILE_DATA);

//     // Render each scanline of the frame
//     for (int ly = 0; ly < SCREEN_HEIGHT; ly++) {
//         // Clear line buffers
//         memset(self->line_buffer_bg_and_window, 0, SCREEN_WIDTH);
//         memset(self->line_buffer_sprite, 0, SCREEN_WIDTH * 2);
//         memset(self->line_buffer, 0, SCREEN_WIDTH);

//         // Draw background for this line
//         if (lcdc & LCDC_BG_ON) {
//             ppu_render_background_scanline(self, ly, scx, scy, tile_map_base_address, tile_data_base_address, signed_addressing, bgp);
//         }

//         // Draw window for this line
//         if (lcdc & LCDC_WINDOW_ON) {
//             ppu_render_window_scanline(self, ly, wx, wy, tile_map_base_address, tile_data_base_address, signed_addressing, bgp);
//         }

//         // Draw sprites for this line
//         if (lcdc & LCDC_OBJ_ON) {
//             ppu_render_sprites_scanline(self, ly, lcdc, obp0, obp1);
//         }

//         // Merge layers and copy to framebuffer
//         for (int x = 0; x < SCREEN_WIDTH; x++) {
//             uint8_t bg_color = self->line_buffer_bg_and_window[x];
//             uint8_t sprite_color = self->line_buffer_sprite[x][0];
            
//             // If there's a sprite pixel and it's not transparent
//             if (sprite_color != 0) {
//                 self->line_buffer[x] = sprite_color;
//             } else {
//                 self->line_buffer[x] = bg_color;
//             }
//         }
        
//         // Copy line to framebuffer
//         memcpy(self->framebuffer + ly * SCREEN_WIDTH, self->line_buffer, SCREEN_WIDTH);
//     }
// }

// Helper function to render background for a single scanline
void ppu_render_background_scanline(struct PPU* self, uint8_t ly, uint8_t scx, uint8_t scy, 
                                   uint16_t tile_map, uint16_t tile_data_base, bool signed_addressing, uint8_t bgp)
{
    uint8_t y        = (ly + scy) & 0xFF; // Wrap around at 256
    uint8_t tile_row = y / 8;
    uint8_t tile_y   = y % 8;

    for (int x = 0; x < SCREEN_WIDTH; x++) {
        uint8_t pixel_x = (x + scx) & 0xFF; // Wrap around at 256
        uint8_t tile_col = pixel_x / 8;
        uint8_t tile_x   = pixel_x % 8;

        // Get tile index from background map
        uint16_t tile_map_address = tile_map + (tile_row * 32) + tile_col;
        uint8_t  tile_index       = self->mmu->mmu_get_byte(self->mmu, tile_map_address);

        // Calculate tile data address
        uint16_t tile_data_address;
        if (signed_addressing) {
            // 8800 method: tile_index is signed, base is 0x9000
            int8_t signed_tile_index = (int8_t)tile_index;
            tile_data_address = tile_data_base + (signed_tile_index * 16) + (tile_y * 2);
        } else {
            // 8000 method: tile_index is unsigned, base is 0x8000
            tile_data_address = tile_data_base + (tile_index * 16) + (tile_y * 2);
        }

        // Read tile data (2 bytes per row)
        uint8_t tile_data_low  = self->mmu->mmu_get_byte(self->mmu, tile_data_address);
        uint8_t tile_data_high = self->mmu->mmu_get_byte(self->mmu, tile_data_address + 1);

        // Extract pixel color (2 bits per pixel)
        uint8_t bit_pos = 7 - tile_x;
        uint8_t color_low  = (tile_data_low >> bit_pos) & 1;
        uint8_t color_high = (tile_data_high >> bit_pos) & 1;
        uint8_t color      = (color_high << 1) | color_low;

        // Store RAW background color index (0-3) - palette will be applied later during mixing
        self->line_buffer_bg_and_window[x] = color;
    }
}

// Helper function to render window for a single scanline
void ppu_render_window_scanline(struct PPU* self, uint8_t ly, uint8_t wx, uint8_t wy,
                               uint16_t tile_map, uint16_t tile_data_base, bool signed_addressing, uint8_t bgp)
{
    // Check if window should be visible on this line
    if (ly < wy) {
        return;
    }

    // Window X position: WX=7 means leftmost position
    int window_start_x = (int)wx - 7;
    if (window_start_x >= SCREEN_WIDTH) {
        return;
    }

    uint8_t window_line = ly - wy;
    uint8_t tile_row    = window_line / 8;
    uint8_t tile_y      = window_line % 8;

    for (int x = (window_start_x > 0 ? window_start_x : 0); x < SCREEN_WIDTH; x++) {
        // Calculate window-relative coordinates
        int window_x_signed = x - window_start_x;
        
        // Skip pixels that are before the window starts
        if (window_x_signed < 0) {
            continue;
        }
        
        uint8_t window_x = (uint8_t)window_x_signed;
        uint8_t tile_col = window_x / 8;
        uint8_t tile_x   = window_x % 8;

        // Get tile index from window tile map (32x32 grid)
        uint16_t tile_addr = tile_map + ((tile_row & 31) * 32) + (tile_col & 31);
        uint8_t  tile_idx  = self->mmu->mmu_get_byte(self->mmu, tile_addr);

        // Calculate tile data address
        uint16_t data_addr;
        if (signed_addressing) {
            // 8800 method: base at 0x9000, signed tile index
            data_addr = tile_data_base + ((int8_t)tile_idx * 16);
        } else {
            // 8000 method: base at 0x8000, unsigned tile index
            data_addr = tile_data_base + (tile_idx * 16);
        }

        // Get the two bytes for this tile row (Game Boy 2BPP format)
        uint8_t byte1 = self->mmu->mmu_get_byte(self->mmu, data_addr + (tile_y * 2));
        uint8_t byte2 = self->mmu->mmu_get_byte(self->mmu, data_addr + (tile_y * 2) + 1);

        // Extract the pixel (2 bits per pixel)
        uint8_t bit = 7 - tile_x;
        uint8_t color_id = ((byte2 >> bit) & 1) << 1 | ((byte1 >> bit) & 1);
        
        // Store RAW window color index (0-3) - palette will be applied later during mixing
        self->line_buffer_bg_and_window[x] = color_id;
    }
}

// Helper function to render sprites for a single scanline
void ppu_render_sprites_scanline(struct PPU* self, uint8_t ly, uint8_t lcdc, uint8_t obp0, uint8_t obp1)
{
    uint8_t sprite_height = (lcdc & LCDC_OBJ_SIZE) ? 16 : 8;

    // Use sprites found during OAM search (Mode 2) instead of searching again
    // This is hardware-accurate: OAM search happens in Mode 2, rendering in Mode 3
    
    // Draw sprites in forward OAM order for proper priority
    // Earlier sprites in OAM have higher priority and should overwrite later sprites
    for (int sprite_idx = 0; sprite_idx < self->searched_sprite_count; sprite_idx++) {
        struct SpriteEntry* sprite = self->selected_oam_entries[sprite_idx];
        
        uint8_t sprite_y = sprite->y - 16;  // Sprite Y is offset by 16
        uint8_t sprite_x = sprite->x - 8;   // Sprite X is offset by 8
        uint8_t tile_idx = sprite->tile_index;

        // Calculate which row of the sprite we're drawing
        uint8_t sprite_row = ly - sprite_y;
        
        // Handle Y flip
        if (sprite->y_flip) {
            sprite_row = sprite_height - 1 - sprite_row;
        }

        // For 8x16 sprites, adjust tile index
        if (sprite_height == 16) {
            if (sprite_row >= 8) {
                tile_idx |= 0x01;  // Bottom tile
                sprite_row -= 8;
            } else {
                tile_idx &= 0xFE;  // Top tile
            }
        }

        // Sprites always use 8000 addressing method
        uint16_t data_addr = 0x8000 + (tile_idx * 16) + (sprite_row * 2);
        uint8_t  byte1     = self->mmu->mmu_get_byte(self->mmu, data_addr);
        uint8_t  byte2     = self->mmu->mmu_get_byte(self->mmu, data_addr + 1);

        // Select palette (sprite->palette: 0=OBP0, 1=OBP1)
        uint8_t palette = sprite->palette ? obp1 : obp0;

        // Draw 8 pixels of the sprite
        for (int pixel_x = 0; pixel_x < 8; pixel_x++) {
            int screen_x = sprite_x + pixel_x;
            
            // Check bounds
            if (screen_x < 0 || screen_x >= SCREEN_WIDTH) {
                continue;
            }

            // Handle X flip
            uint8_t bit = sprite->x_flip ? pixel_x : (7 - pixel_x);
            uint8_t color_id = ((byte2 >> bit) & 1) << 1 | ((byte1 >> bit) & 1);

            // Color 0 is transparent for sprites
            if (color_id == 0) {
                continue;
            }

            // Sprite-to-sprite priority: Don't overwrite existing sprite pixels
            // Earlier sprites in OAM have higher priority
            if (self->line_buffer_sprite[screen_x][0] != 0) {
                continue; // Skip this pixel, earlier sprite already claimed it
            }
            
            // Store RAW sprite color index (0-3) - palette will be applied later during mixing
            self->line_buffer_sprite[screen_x][0] = color_id;
            
            // Store sprite info: low 7 bits = sprite index, high bit = priority
            self->line_buffer_sprite[screen_x][1] = sprite_idx & 0x7F;
            if (sprite->priority == 1) {
                self->line_buffer_sprite[screen_x][1] |= 0x80; // Set high bit for priority 1
            }
            
            // Store palette selection in a way we can retrieve it
            // We'll use a separate approach - store palette bit in unused bits
            if (sprite->palette == 1) {
                self->line_buffer_sprite[screen_x][1] |= 0x40; // Use bit 6 for palette selection
            }
        }
    }
}

uint8_t ppu_mix_tile_colors(struct PPU* self, int palette, uint8_t color0, uint8_t color1)
{
    uint8_t color = (color1 << 1) | color0;
    return (palette >> (color * 2)) & 0x03;
}

// ======================== FIFO FUNCTIONS ========================

void fifo_clear(struct PixelFIFO* fifo)
{
    fifo->head = 0;
    fifo->tail = 0;
    fifo->size = 0;
}

void fifo_push(struct PixelFIFO* fifo, struct FIFOPixel pixel)
{
    if (fifo->size < FIFO_SIZE) {
        fifo->pixels[fifo->tail] = pixel;
        fifo->tail = (fifo->tail + 1) % FIFO_SIZE;
        fifo->size++;
    }
}

struct FIFOPixel fifo_pop(struct PixelFIFO* fifo)
{
    struct FIFOPixel pixel = {0, 0, 0, false};  // color_raw, palette, priority, is_sprite
    if (fifo->size > 0) {
        pixel = fifo->pixels[fifo->head];
        fifo->head = (fifo->head + 1) % FIFO_SIZE;
        fifo->size--;
    }
    return pixel;
}

bool fifo_is_empty(struct PixelFIFO* fifo)
{
    return fifo->size == 0;
}

uint8_t fifo_size(struct PixelFIFO* fifo)
{
    return fifo->size;
}

// ======================== BACKGROUND FETCHER ========================

void bg_fetcher_reset(struct BackgroundFetcher* fetcher, uint8_t ly, uint8_t scx, uint8_t scy)
{
    fetcher->state = FETCH_TILE_NUMBER;
    fetcher->tile_number = 0;
    fetcher->tile_data_low = 0;
    fetcher->tile_data_high = 0;
    fetcher->fetch_x = 0;
    fetcher->map_x = (scx / 8) % 32;           // Start tile X position
    fetcher->map_y = ((ly + scy) / 8) % 32;    // Start tile Y position  
    fetcher->tile_x = 0;                       // Always start from leftmost pixel of tile
    fetcher->tile_y = (ly + scy) % 8;          // Y position within tile
    fetcher->window_active = false;
}

void bg_fetcher_step(struct PPU* ppu)
{
    struct BackgroundFetcher* fetcher = &ppu->bg_fetcher;
    uint8_t lcdc = ppu->mmu->mmu_get_byte(ppu->mmu, LCDC_ADDRESS);
    
    switch (fetcher->state) {
        case FETCH_TILE_NUMBER: {
            // Get tile number from tilemap
            uint16_t tile_map_base = (lcdc & LCDC_BG_MAP) ? 0x9C00 : 0x9800;
            if (fetcher->window_active) {
                tile_map_base = (lcdc & LCDC_WINDOW_MAP) ? 0x9C00 : 0x9800;
            }
            
            uint16_t tile_map_addr = tile_map_base + (fetcher->map_y * 32) + fetcher->map_x;
            fetcher->tile_number = ppu->vram->vram_get_byte(ppu->vram, tile_map_addr);
            fetcher->state = FETCH_TILE_DATA_LOW;
            break;
        }
        
        case FETCH_TILE_DATA_LOW: {
            // Get low byte of tile data
            uint16_t tile_data_base = (lcdc & LCDC_TILE_DATA) ? 0x8000 : 0x9000;
            uint16_t tile_data_addr;
            
            if (lcdc & LCDC_TILE_DATA) {
                // 8000 method - unsigned
                tile_data_addr = tile_data_base + (fetcher->tile_number * 16) + (fetcher->tile_y * 2);
            } else {
                // 8800 method - signed
                int8_t signed_tile_number = (int8_t)fetcher->tile_number;
                tile_data_addr = tile_data_base + (signed_tile_number * 16) + (fetcher->tile_y * 2);
            }
            
            fetcher->tile_data_low = ppu->vram->vram_get_byte(ppu->vram, tile_data_addr);
            fetcher->state = FETCH_TILE_DATA_HIGH;
            break;
        }
        
        case FETCH_TILE_DATA_HIGH: {
            // Get high byte of tile data
            uint16_t tile_data_base = (lcdc & LCDC_TILE_DATA) ? 0x8000 : 0x9000;
            uint16_t tile_data_addr;
            
            if (lcdc & LCDC_TILE_DATA) {
                // 8000 method - unsigned
                tile_data_addr = tile_data_base + (fetcher->tile_number * 16) + (fetcher->tile_y * 2) + 1;
            } else {
                // 8800 method - signed
                int8_t signed_tile_number = (int8_t)fetcher->tile_number;
                tile_data_addr = tile_data_base + (signed_tile_number * 16) + (fetcher->tile_y * 2) + 1;
            }
            
            fetcher->tile_data_high = ppu->vram->vram_get_byte(ppu->vram, tile_data_addr);
            fetcher->state = FETCH_PUSH;
            break;
        }
        
        case FETCH_PUSH: {
            // Push 8 pixels to background FIFO
            bg_fetcher_push_pixels(ppu);
            
            // Move to next tile
            fetcher->map_x = (fetcher->map_x + 1) % 32;
            fetcher->fetch_x += 8;
            fetcher->state = FETCH_TILE_NUMBER;
            break;
        }
    }
}

void bg_fetcher_push_pixels(struct PPU* ppu)
{
    struct BackgroundFetcher* fetcher = &ppu->bg_fetcher;
    
    // Push 8 pixels to background FIFO
    for (int i = 0; i < 8; i++) {
        uint8_t bit_pos = 7 - i;
        uint8_t color_low = (fetcher->tile_data_low >> bit_pos) & 1;
        uint8_t color_high = (fetcher->tile_data_high >> bit_pos) & 1;
        uint8_t color_index = (color_high << 1) | color_low;
        
        struct FIFOPixel pixel = {
            .color_raw = color_index,  // Store raw color index (0-3)
            .palette = 0,  // BGP
            .priority = 0,
            .is_sprite = false
        };
        
        fifo_push(&ppu->background_fifo, pixel);
    }
}

// ======================== SPRITE FETCHER ========================

void sprite_fetcher_reset(struct SpriteFetcher* fetcher)
{
    fetcher->active = false;
    fetcher->sprite_index = 0;
    fetcher->tile_data_low = 0;
    fetcher->tile_data_high = 0;
    fetcher->state = FETCH_TILE_NUMBER;
}

void sprite_fetcher_step(struct PPU* ppu, struct SpriteEntry* sprite)
{
    struct SpriteFetcher* fetcher = &ppu->sprite_fetcher;
    uint8_t lcdc = ppu->mmu->mmu_get_byte(ppu->mmu, LCDC_ADDRESS);
    uint8_t ly = ppu->mmu->mmu_get_byte(ppu->mmu, LY_ADDRESS);
    
    switch (fetcher->state) {
        case FETCH_TILE_NUMBER: {
            fetcher->active = true;
            fetcher->state = FETCH_TILE_DATA_LOW;
            break;
        }
        
        case FETCH_TILE_DATA_LOW: {
            // Calculate sprite tile data address
            uint8_t sprite_height = (lcdc & LCDC_OBJ_SIZE) ? 16 : 8;
            uint8_t sprite_y = sprite->y - 16;
            uint8_t sprite_row = ly - sprite_y;
            uint8_t tile_index = sprite->tile_index;
            
            // Handle Y flip
            if (sprite->y_flip) {
                sprite_row = sprite_height - 1 - sprite_row;
            }
            
            // For 8x16 sprites, adjust tile index
            if (sprite_height == 16) {
                if (sprite_row >= 8) {
                    tile_index |= 0x01;  // Bottom tile
                    sprite_row -= 8;
                } else {
                    tile_index &= 0xFE;  // Top tile
                }
            }
            
            // Sprites always use 8000 addressing method
            uint16_t tile_data_addr = 0x8000 + (tile_index * 16) + (sprite_row * 2);
            fetcher->tile_data_low = ppu->vram->vram_get_byte(ppu->vram, tile_data_addr);
            fetcher->state = FETCH_TILE_DATA_HIGH;
            break;
        }
        
        case FETCH_TILE_DATA_HIGH: {
            // Get high byte (same calculation as low byte + 1)
            uint8_t sprite_height = (lcdc & LCDC_OBJ_SIZE) ? 16 : 8;
            uint8_t sprite_y = sprite->y - 16;
            uint8_t sprite_row = ly - sprite_y;
            uint8_t tile_index = sprite->tile_index;
            
            // Handle Y flip
            if (sprite->y_flip) {
                sprite_row = sprite_height - 1 - sprite_row;
            }
            
            // For 8x16 sprites, adjust tile index
            if (sprite_height == 16) {
                if (sprite_row >= 8) {
                    tile_index |= 0x01;  // Bottom tile
                    sprite_row -= 8;
                } else {
                    tile_index &= 0xFE;  // Top tile
                }
            }
            
            // Sprites always use 8000 addressing method
            uint16_t tile_data_addr = 0x8000 + (tile_index * 16) + (sprite_row * 2) + 1;
            fetcher->tile_data_high = ppu->vram->vram_get_byte(ppu->vram, tile_data_addr);
            fetcher->state = FETCH_PUSH;
            break;
        }
        
        case FETCH_PUSH: {
            // Push sprite pixels to sprite FIFO
            sprite_fetcher_push_pixels(ppu, sprite);
            fetcher->active = false;
            fetcher->state = FETCH_TILE_NUMBER;
            break;
        }
    }
}

void sprite_fetcher_push_pixels(struct PPU* ppu, struct SpriteEntry* sprite)
{
    struct SpriteFetcher* fetcher = &ppu->sprite_fetcher;
    uint8_t obp0 = ppu->mmu->mmu_get_byte(ppu->mmu, OBP0_ADDRESS);
    uint8_t obp1 = ppu->mmu->mmu_get_byte(ppu->mmu, OBP1_ADDRESS);
    uint8_t palette_reg = sprite->palette ? obp1 : obp0;
    
    // Calculate sprite screen position
    uint8_t sprite_x = sprite->x - 8;
    
    // Push 8 pixels to sprite FIFO
    for (int i = 0; i < 8; i++) {
        uint8_t bit_pos;
        
        // Handle X flip
        if (sprite->x_flip) {
            bit_pos = i;  // Normal order for X flip
        } else {
            bit_pos = 7 - i;  // Reverse order for no flip
        }
        
        uint8_t color_low = (fetcher->tile_data_low >> bit_pos) & 1;
        uint8_t color_high = (fetcher->tile_data_high >> bit_pos) & 1;
        uint8_t color_index = (color_high << 1) | color_low;
        
        // Calculate screen X position for this pixel
        int screen_x = sprite_x + i;
        
        // Only push pixels that are within screen bounds and at current FIFO position
        if (screen_x >= 0 && screen_x < SCREEN_WIDTH && screen_x == ppu->fifo_x + i) {
            struct FIFOPixel sprite_pixel = {
                .color_raw = color_index,  // Store raw color index (0-3)
                .palette = sprite->palette ? 1 : 0,  // OBP0 or OBP1
                .priority = sprite->priority,
                .is_sprite = (color_index != 0)  // Only non-transparent pixels are sprites
            };
            
            // Mix with existing sprite FIFO pixels (sprite-to-sprite priority)
            if (i < fifo_size(&ppu->sprite_fifo)) {
                // Don't overwrite existing sprite pixels (first sprite wins)
                continue;
            }
            
            fifo_push(&ppu->sprite_fifo, sprite_pixel);
        }
    }
}

// ======================== PIXEL MIXING ========================

struct FIFOPixel ppu_mix_pixels(struct FIFOPixel bg_pixel, struct FIFOPixel sprite_pixel)
{
    // If no sprite pixel, return background
    if (!sprite_pixel.is_sprite) {
        return bg_pixel;
    }
    
    // If sprite pixel is transparent (color 0), use background
    if (sprite_pixel.color_raw == 0) {
        return bg_pixel;
    }
    
    // Check sprite priority (according to GBEDG)
    if (sprite_pixel.priority == 0) {
        // Priority 0: Sprite appears above background
        return sprite_pixel;
    } else {
        // Priority 1: Sprite appears behind non-zero background colors
        if (bg_pixel.color_raw == 0) {
            return sprite_pixel;
        } else {
            return bg_pixel;
        }
    }
}

// ======================== FIFO SCANLINE RENDERING ========================

void ppu_render_scanline_fifo(struct PPU* self, uint8_t ly)
{
    if (ly >= SCREEN_HEIGHT) {
        return; // Only render visible scanlines
    }
    
    // Load PPU registers
    uint8_t lcdc = self->mmu->mmu_get_byte(self->mmu, LCDC_ADDRESS);
    uint8_t scx = self->mmu->mmu_get_byte(self->mmu, SCX_ADDRESS);
    uint8_t scy = self->mmu->mmu_get_byte(self->mmu, SCY_ADDRESS);
    uint8_t wx = self->mmu->mmu_get_byte(self->mmu, WX_ADDRESS);
    uint8_t wy = self->mmu->mmu_get_byte(self->mmu, WY_ADDRESS);
    
    // Initialize FIFO state
    fifo_clear(&self->background_fifo);
    fifo_clear(&self->sprite_fifo);
    bg_fetcher_reset(&self->bg_fetcher, ly, scx, scy);
    
    self->fifo_x = 0;
    self->pushed_pixels = 0;
    self->window_triggered = false;
    
    // Check if window should be triggered this scanline
    bool window_enabled = (lcdc & LCDC_WINDOW_ON) && (ly >= wy);
    
    // Pre-fill the background FIFO to ensure we have pixels ready
    while (fifo_size(&self->background_fifo) <= 8) {
        bg_fetcher_step(self);
    }
    
    // Main rendering loop
    while (self->pushed_pixels < SCREEN_WIDTH) {
        // Step the background fetcher to keep FIFO fed
        if (fifo_size(&self->background_fifo) <= 8) {
            bg_fetcher_step(self);
        }
        
        // Check if we need to switch to window
        if (window_enabled && !self->window_triggered && 
            self->fifo_x >= (wx - 7) && wx >= 7 && wx <= 166) {
            
            // Clear background FIFO and switch to window
            fifo_clear(&self->background_fifo);
            self->bg_fetcher.window_active = true;
            self->bg_fetcher.state = FETCH_TILE_NUMBER;
            self->bg_fetcher.map_x = 0;
            self->bg_fetcher.map_y = ly - wy;
            self->bg_fetcher.tile_y = (ly - wy) % 8;
            self->window_triggered = true;
        }
        
        // Check for sprites at current position
        struct SpriteEntry* current_sprite = NULL;
        if (lcdc & LCDC_OBJ_ON) {
            // Find the highest priority sprite at current X position
            for (int i = 0; i < self->searched_sprite_count; i++) {
                struct SpriteEntry* sprite = self->selected_oam_entries[i];
                uint8_t sprite_x = sprite->x - 8;
                
                // Check if sprite intersects with current X position
                if (sprite_x <= self->fifo_x && self->fifo_x < sprite_x + 8) {
                    current_sprite = sprite;
                    break; // Take first sprite found (highest priority due to OAM order)
                }
            }
        }
        
        // Pop pixels from background FIFO and push to screen
        if (fifo_size(&self->background_fifo) > 0) {
            struct FIFOPixel bg_pixel = fifo_pop(&self->background_fifo);
            
            // Get sprite pixel at current position
            struct FIFOPixel sprite_pixel = {0, 0, 0, false}; // Default: no sprite (color_raw, palette, priority, is_sprite)
            
            if (current_sprite != NULL) {
                // Calculate sprite pixel
                uint8_t sprite_x = current_sprite->x - 8;
                int pixel_offset = self->fifo_x - sprite_x;
                
                if (pixel_offset >= 0 && pixel_offset < 8) {
                    // We're within the sprite bounds, get sprite pixel
                    uint8_t sprite_height = (lcdc & LCDC_OBJ_SIZE) ? 16 : 8;
                    uint8_t sprite_y = current_sprite->y - 16;
                    uint8_t sprite_row = ly - sprite_y;
                    uint8_t tile_index = current_sprite->tile_index;
                    
                    // Handle Y flip
                    if (current_sprite->y_flip) {
                        sprite_row = sprite_height - 1 - sprite_row;
                    }
                    
                    // For 8x16 sprites, adjust tile index
                    if (sprite_height == 16) {
                        if (sprite_row >= 8) {
                            tile_index |= 0x01;  // Bottom tile
                            sprite_row -= 8;
                        } else {
                            tile_index &= 0xFE;  // Top tile
                        }
                    }
                    
                    // Get sprite tile data
                    uint16_t tile_data_addr = 0x8000 + (tile_index * 16) + (sprite_row * 2);
                    uint8_t tile_data_low = self->vram->vram_get_byte(self->vram, tile_data_addr);
                    uint8_t tile_data_high = self->vram->vram_get_byte(self->vram, tile_data_addr + 1);
                    
                    // Calculate bit position (handle X flip)
                    uint8_t bit_pos;
                    if (current_sprite->x_flip) {
                        bit_pos = pixel_offset;
                    } else {
                        bit_pos = 7 - pixel_offset;
                    }
                    
                    // Extract pixel color
                    uint8_t color_low = (tile_data_low >> bit_pos) & 1;
                    uint8_t color_high = (tile_data_high >> bit_pos) & 1;
                    uint8_t color_index = (color_high << 1) | color_low;
                    
                    // Store sprite pixel if not transparent
                    if (color_index != 0) {
                        sprite_pixel.color_raw = color_index;
                        sprite_pixel.palette = current_sprite->palette ? 2 : 1;  // 1=OBP0, 2=OBP1
                        sprite_pixel.priority = current_sprite->priority;
                        sprite_pixel.is_sprite = true;
                    }
                }
            }
            
            // Mix pixels
            struct FIFOPixel final_pixel = ppu_mix_pixels(bg_pixel, sprite_pixel);
            
            // Handle SCX fine scrolling - skip first (scx & 7) pixels
            uint8_t scx_fine = scx & 7;
            if (self->fifo_x < scx_fine) {
                // Discard this pixel for fine scrolling
                self->fifo_x++;
                continue;
            }
            
            // Apply correct palette based on pixel source
            uint8_t final_color = final_pixel.color_raw;
            if (final_pixel.palette == 0) {
                // Background palette (BGP)
                uint8_t bgp = self->mmu->mmu_get_byte(self->mmu, BGP_ADDRESS);
                final_color = (bgp >> (final_pixel.color_raw * 2)) & 0x03;
            } else if (final_pixel.palette == 1) {
                // Sprite palette 0 (OBP0)
                uint8_t obp0 = self->mmu->mmu_get_byte(self->mmu, OBP0_ADDRESS);
                final_color = (obp0 >> (final_pixel.color_raw * 2)) & 0x03;
            } else if (final_pixel.palette == 2) {
                // Sprite palette 1 (OBP1)
                uint8_t obp1 = self->mmu->mmu_get_byte(self->mmu, OBP1_ADDRESS);
                final_color = (obp1 >> (final_pixel.color_raw * 2)) & 0x03;
            }
            
            // Apply LCDC.0 - BG/Window Enable
            if (!(lcdc & LCDC_BG_ON)) {
                final_color = 0; // Force background to white when disabled
            }
            
            // Store pixel in framebuffer
            if (self->pushed_pixels < SCREEN_WIDTH) {
                self->framebuffer[ly * SCREEN_WIDTH + self->pushed_pixels] = final_color;
                self->pushed_pixels++;
            }
            
            self->fifo_x++;
        }
    }
}

void free_ppu(struct PPU* ppu)
{
    if (ppu != NULL) {
        if (ppu->vram != NULL) {
            free_vram(ppu->vram);
            ppu->vram = NULL;
        }
        if (ppu->framebuffer != NULL) {
            free(ppu->framebuffer);
            ppu->framebuffer = NULL;
        }
        if (ppu->oam_buffer != NULL) {
            free(ppu->oam_buffer);
            ppu->oam_buffer = NULL;
        }
        if (ppu->selected_oam_entries != NULL) {
            free(ppu->selected_oam_entries);
            ppu->selected_oam_entries = NULL;
        }
        free(ppu);
    }
}
