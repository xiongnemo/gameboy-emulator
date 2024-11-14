#include "ppu.h"

void ppu_step(struct PPU* self, uint8_t cycles) {
    self->dot_clock += cycles;

    // Update LY register and check for LYC=LY coincidence
    if (self->ly == self->lyc) {
        self->stat |= STAT_LYC_EQUAL;
    } else {
        self->stat &= ~STAT_LYC_EQUAL;
    }

    // PPU mode state machine
    switch (self->mode) {
        case 0: // H-Blank
            if (self->dot_clock >= MODE_0_CYCLES) {
                self->dot_clock = 0;
                self->ly++;
                
                if (self->ly == 144) {
                    self->mode = 1;  // Switch to V-Blank
                } else {
                    self->mode = 2;  // Switch to OAM Scan
                }
            }
            break;

        case 1: // V-Blank
            if (self->dot_clock >= MODE_1_CYCLES) {
                self->dot_clock = 0;
                self->ly++;
                
                if (self->ly > 153) {
                    self->mode = 2;  // Switch to OAM Scan
                    self->ly = 0;
                }
            }
            break;

        case 2: // OAM Scan
            if (self->dot_clock >= MODE_2_CYCLES) {
                self->dot_clock = 0;
                self->mode = 3;  // Switch to Drawing
            }
            break;

        case 3: // Drawing
            if (self->dot_clock >= MODE_3_CYCLES) {
                self->dot_clock = 0;
                self->mode = 0;  // Switch to H-Blank
                ppu_render_scanline(self);
            }
            break;
    }

    ppu_update_stat(self);
}

void ppu_render_scanline(struct PPU* self) {
    if (!ppu_is_lcd_enabled(self)) {
        return;
    }

    // Basic implementation - just fill with white for now
    // Will need to be expanded to handle tiles and sprites
    for (int x = 0; x < 160; x++) {
        self->framebuffer[self->ly * 160 + x] = 0xFFFFFFFF;
    }
}

void ppu_update_stat(struct PPU* self) {
    // Update mode bits
    self->stat = (self->stat & ~STAT_MODE_MASK) | self->mode;
}

bool ppu_is_lcd_enabled(struct PPU* self) {
    return (self->lcdc & LCDC_ENABLE) != 0;
}

struct PPU* create_ppu(struct Vram* vram) {
    struct PPU* ppu = (struct PPU*)malloc(sizeof(struct PPU));
    
    // Initialize registers
    ppu->lcdc = LCDC_ENABLE | LCDC_BG_ON;  // LCD and BG enabled by default
    ppu->stat = 0;
    ppu->scy = 0;
    ppu->scx = 0;
    ppu->ly = 0;
    ppu->lyc = 0;
    ppu->bgp = 0xFC;   // Default palette
    ppu->obp0 = 0xFF;
    ppu->obp1 = 0xFF;
    ppu->wy = 0;
    ppu->wx = 0;

    // Initialize state
    ppu->dot_clock = 0;
    ppu->mode = 2;     // Start in OAM scan mode
    ppu->vram = vram;  // Store VRAM reference
    memset(ppu->framebuffer, 0, sizeof(ppu->framebuffer));

    // Initialize method pointers
    ppu->step = ppu_step;
    ppu->render_scanline = ppu_render_scanline;
    ppu->update_stat = ppu_update_stat;
    ppu->is_lcd_enabled = ppu_is_lcd_enabled;

    return ppu;
}

void free_ppu(struct PPU* ppu) {
    if (ppu->vram) {
        free_vram(ppu->vram);
    }
    if (ppu) {
        free(ppu);
    }
}
