#include "form.h"

// BMP file structures
#pragma pack(push, 1)
typedef struct
{
    uint16_t signature;     // BM
    uint32_t file_size;     // Size of the BMP file
    uint16_t reserved1;     // Unused
    uint16_t reserved2;     // Unused
    uint32_t data_offset;   // Offset to start of pixel data
} BMPFileHeader;

typedef struct
{
    uint32_t header_size;        // Size of this header (40 bytes)
    int32_t  width;              // Image width
    int32_t  height;             // Image height
    uint16_t planes;             // Number of color planes
    uint16_t bits_per_pixel;     // Bits per pixel
    uint32_t compression;        // Compression type
    uint32_t image_size;         // Image size (can be 0 for uncompressed)
    int32_t  x_pixels_per_m;     // Horizontal resolution
    int32_t  y_pixels_per_m;     // Vertical resolution
    uint32_t colors_used;        // Number of colors used
    uint32_t colors_important;   // Number of important colors
} BMPInfoHeader;
#pragma pack(pop)

struct Form* create_form(struct PPU* ppu, struct Joypad* joypad, char* rom_name)
{
    struct Form* form = (struct Form*)malloc(sizeof(struct Form));
    if (form == NULL) {
        return NULL;
    }

    // init video and joystick
    FORM_DEBUG_PRINT("Initializing SDL...%s", "\n");
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK)) {
        FORM_EMERGENCY_PRINT("Failed to initialize SDL: %s\n", SDL_GetError());
        free(form);
        return NULL;
    }

    int            num_displays;
    SDL_DisplayID* displays = SDL_GetDisplays(&num_displays);
    FORM_DEBUG_PRINT("Found %d display(s)\n", num_displays);

    // create window (not resizable)
    FORM_DEBUG_PRINT("Creating window...%s", "\n");
    form->window = SDL_CreateWindow(
        rom_name, 160 * config.scale_factor, 144 * config.scale_factor, SDL_WINDOW_MAXIMIZED);
    if (form->window == NULL) {
        free(form);
        return NULL;
    }

    // get physical devices' resolution
    uint16_t physical_device_res_width    = 0;
    uint16_t physical_device_res_height   = 0;
    float    physical_device_refresh_rate = 0;
    // Note: the behaviour of SDL3's SDL_GetCurrentDisplayMode is different with
    // SDL2
    const SDL_DisplayMode* physical_device_display_mode = get_current_window_display_mode(form);
    if (physical_device_display_mode == NULL) {
        FORM_EMERGENCY_PRINT("Failed to get physical device display mode: %s\n", SDL_GetError());
        free(form);
        return NULL;
    }
    physical_device_res_width    = physical_device_display_mode->w;
    physical_device_res_height   = physical_device_display_mode->h;
    physical_device_refresh_rate = physical_device_display_mode->refresh_rate;

    FORM_DEBUG_PRINT(
        "Physical device: %dx%d@%fHz\n",
        physical_device_res_width,
        physical_device_res_height,
        physical_device_refresh_rate);

    // store window surface
    FORM_DEBUG_PRINT("Creating window surface...%s", "\n")
    form->surface = SDL_GetWindowSurface(form->window);

    // allocate framebuffer
    FORM_DEBUG_PRINT("Attaching framebuffer...%s", "\n");
    // form->framebuffer = (uint8_t*)malloc(160 * 144 * sizeof(uint8_t));
    form->framebuffer = ppu->framebuffer;
    if (form->framebuffer == NULL) {
        free(form);
        return NULL;
    }

    // allocate event
    form->event = (SDL_Event*)malloc(sizeof(SDL_Event));
    if (form->event == NULL) {
        free(form);
        return NULL;
    }

    // set ppu and joypad
    FORM_DEBUG_PRINT("Attaching ppu and joypad...%s", "\n");
    form->ppu    = ppu;
    ppu->form    = form;
    form->joypad = joypad;
    // no need to set joypad->form because it shouldn't be used

    return form;
}

void free_form(struct Form* form)
{
    // free window
    SDL_DestroyWindow(form->window);
    // free form
    free(form);
}

const SDL_DisplayMode* get_current_window_display_mode(struct Form* form)
{
    int display_id = SDL_GetDisplayForWindow(form->window);
    if (display_id == -1) {
        FORM_EMERGENCY_PRINT("Failed to get display index: %s\n", SDL_GetError());
        return NULL;
    }
    const SDL_DisplayMode* display_mode = SDL_GetCurrentDisplayMode(display_id);
    if (display_mode == NULL) {
        FORM_EMERGENCY_PRINT("Failed to get display mode: %s\n", SDL_GetError());
        return NULL;
    }
    return display_mode;
}

void dump_framebuffer_to_bmp(struct Form* form, const char* filename)
{
    const int width           = 160;
    const int height          = 144;
    const int bytes_per_pixel = 3;   // RGB

    // Calculate row padding (BMP rows must be aligned to 4-byte boundaries)
    int row_size        = (width * bytes_per_pixel + 3) & ~3;
    int pixel_data_size = row_size * height;

    // Create BMP headers
    BMPFileHeader file_header = {0};
    file_header.signature     = 0x4D42;   // "BM" in little-endian
    file_header.file_size     = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + pixel_data_size;
    file_header.data_offset   = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);

    BMPInfoHeader info_header  = {0};
    info_header.header_size    = sizeof(BMPInfoHeader);
    info_header.width          = width;
    info_header.height         = height;
    info_header.planes         = 1;
    info_header.bits_per_pixel = 24;
    info_header.compression    = 0;   // No compression
    info_header.image_size     = pixel_data_size;
    info_header.x_pixels_per_m = 2835;   // 72 DPI
    info_header.y_pixels_per_m = 2835;   // 72 DPI

    // Open file for writing
    FILE* file = fopen(filename, "wb");
    if (!file) {
        FORM_ERROR_PRINT("Failed to create BMP file: %s\n", filename);
        return;
    }

    // Write headers
    fwrite(&file_header, sizeof(BMPFileHeader), 1, file);
    fwrite(&info_header, sizeof(BMPInfoHeader), 1, file);

    // GameBoy color palette (RGB values)
    uint8_t palette[4][3] = {
        {255, 255, 255}, // 0: White
        {170, 170, 170}, // 1: Light gray
        {85,  85,  85 }, // 2: Dark gray
        {0,   0,   0  }  // 3: Black
    };

    // Allocate row buffer with padding
    uint8_t* row_buffer = (uint8_t*)calloc(row_size, 1);
    if (!row_buffer) {
        FORM_ERROR_PRINT("Failed to allocate row buffer for BMP export\n");
        fclose(file);
        return;
    }

    // Write pixel data (BMP stores bottom-to-top)
    for (int y = height - 1; y >= 0; y--) {
        for (int x = 0; x < width; x++) {
            uint8_t color_index = form->framebuffer[y * width + x];
            if (color_index > 3) color_index = 3;   // Clamp to valid range

            // BMP uses BGR format
            int pixel_offset             = x * bytes_per_pixel;
            row_buffer[pixel_offset + 0] = palette[color_index][2];   // B
            row_buffer[pixel_offset + 1] = palette[color_index][1];   // G
            row_buffer[pixel_offset + 2] = palette[color_index][0];   // R
        }

        fwrite(row_buffer, row_size, 1, file);
    }

    free(row_buffer);
    fclose(file);

    FORM_INFO_PRINT("Framebuffer dumped to: %s\n", filename);
}

void update_surface(struct Form* form)
{
    uint32_t* pixels = form->surface->pixels;

    // Convert GameBoy colors to RGBA with scaling
    for (int y = 0; y < 144; y++) {
        for (int x = 0; x < 160; x++) {
            uint8_t  color = form->framebuffer[y * 160 + x];
            uint32_t rgb_color;

            // ARGB format
            switch (color) {
            case 0:
                rgb_color = 0xFFE8FCCC;   // RGB(232,252,204) - Game Boy lightest shade (BGRA)
                // rgb_color = 0xFFFFFFFF;
                break;
            case 1:
                rgb_color = 0xFFACD490;   // RGB(172,212,144) - Game Boy light shade (BGRA)
                // rgb_color = 0xFFAAAAAA;
                break;
            case 2:
                rgb_color = 0xFF548C70;   // RGB(84,140,112) - Game Boy dark shade (ARGB)
                // rgb_color = 0xFF444444;
                break;
            case 3:
                rgb_color = 0xFF142C38;   // RGB(20,44,56) - Game Boy darkest shade (ARGB)
                // rgb_color = 0xFF000000;
                break;
            default:
            {
                static int unexpected_color_count = 0;
                unexpected_color_count++;
                if (unexpected_color_count <= 10) {
                    printf("DEBUG: Unexpected color value: %d at position [%d,%d]\n", color, x, y);
                }
            }
                rgb_color = 0xFF0000FF;   // Blue for debugging unexpected values
                break;
            }

            // Scale the pixel for the 2x window size
            for (int dy = 0; dy < config.scale_factor; dy++) {
                for (int dx = 0; dx < config.scale_factor; dx++) {
                    int pixel_x = x * config.scale_factor + dx;
                    int pixel_y = y * config.scale_factor + dy;
                    if (pixel_x < form->surface->w && pixel_y < form->surface->h) {
                        pixels[pixel_y * (form->surface->pitch / 4) + pixel_x] = rgb_color;
                    }
                }
            }
        }
    }

    // Dump framebuffer to BMP file (uncomment to enable)
    // dump_framebuffer_to_bmp(form, "gameboy_screen.bmp");

    SDL_UpdateWindowSurface(form->window);
}

void set_framebuffer(struct Form* form)
{
    form->framebuffer = form->ppu->framebuffer;
}

bool get_joypad_state(struct Form* form)
{
    // Preserve CPU's column selection in FF00 register (bits 4-5)
    // uint8_t current_ff00 = form->ppu->mmu->mmu_get_byte(form->ppu->mmu, 0xFF00);
    // uint8_t column_selection = current_ff00 & 0x3F;
    // form->ppu->mmu->mmu_set_byte(form->ppu->mmu, 0xFF00, column_selection);

    while (SDL_PollEvent(form->event)) {
        if (form->event->type == SDL_EVENT_QUIT) {
            return false;
        }

        // Handle keyboard press events
        if (form->event->type == SDL_EVENT_KEY_DOWN) {
            switch (form->event->key.key) {
            case SDLK_ESCAPE: FORM_INFO_PRINT("Quit requested by user.\n"); return false;
            case SDLK_P: config.print_debug_info_this_frame = true; break;
            case SDLK_LCTRL: config.fast_forward_mode = true; break;
            case SDLK_LALT:
                config.disable_joypad = !config.disable_joypad;
                FORM_WARN_PRINT("Joypad %s\n", config.disable_joypad ? "disabled" : "enabled");
                break;

            // screenshot
            case SDLK_F1: dump_framebuffer_to_bmp(form, "gameboy_framebuffer.bmp"); break;

            // Direction keys - clear corresponding bit when pressed (0=pressed)
            case SDLK_D:                                // RIGHT (bit 0)
                form->joypad->keys_directions &= 0xE;   // Clear bit 0: 1110
                break;

            case SDLK_A:                                // LEFT (bit 1)
                form->joypad->keys_directions &= 0xD;   // Clear bit 1: 1101
                break;

            case SDLK_W:                                // UP (bit 2)
                form->joypad->keys_directions &= 0xB;   // Clear bit 2: 1011
                break;

            case SDLK_S:                                // DOWN (bit 3)
                form->joypad->keys_directions &= 0x7;   // Clear bit 3: 0111
                break;

            // Control keys - clear corresponding bit when pressed (0=pressed)
            case SDLK_J:                              // A (bit 0)
                form->joypad->keys_controls &= 0xE;   // Clear bit 0: 1110
                break;

            // case SDLK_SPACE:
            case SDLK_K:                              // B (bit 1)
                form->joypad->keys_controls &= 0xD;   // Clear bit 1: 1101
                break;

            case SDLK_LSHIFT:                         // SELECT (bit 2)
                form->joypad->keys_controls &= 0xB;   // Clear bit 2: 1011
                break;

            case SDLK_RETURN:                         // START (bit 3)
                form->joypad->keys_controls &= 0x7;   // Clear bit 3: 0111
                break;

            // Special functions
            case SDLK_Q:   // Quick Save
                form->joypad->save_flag = 1;
                FORM_INFO_PRINT("Will save before next poll...%s\n", "");
                break;

            case SDLK_Y:   // Quick Load
                form->joypad->load_flag = 1;
                FORM_INFO_PRINT("Will load before next poll...%s\n", "");
                break;

            case SDLK_T:   // Quit and Save
                FORM_INFO_PRINT("Quit and save.%s\n", "");
                return false;

                // case SDLK_L:  // Fast Forward
                //     form->joypad->fast_forward_flag = 1;
                //     FORM_INFO_PRINT("Triggering fast forward...%s\n", "");
                //     break;
            }
        }
        // Handle keyboard release events
        else if (form->event->type == SDL_EVENT_KEY_UP) {
            switch (form->event->key.key) {
            case SDLK_LCTRL: config.fast_forward_mode = false; break;
            // Direction keys - set corresponding bit when released (1=not pressed)
            case SDLK_D:                                // RIGHT (bit 0)
                form->joypad->keys_directions |= 0x1;   // Set bit 0
                break;

            case SDLK_A:                                // LEFT (bit 1)
                form->joypad->keys_directions |= 0x2;   // Set bit 1
                break;

            case SDLK_W:                                // UP (bit 2)
                form->joypad->keys_directions |= 0x4;   // Set bit 2
                break;

            case SDLK_S:                                // DOWN (bit 3)
                form->joypad->keys_directions |= 0x8;   // Set bit 3
                break;

            // Control keys - set corresponding bit when released (1=not pressed)
            case SDLK_J:                              // A (bit 0)
                form->joypad->keys_controls |= 0x1;   // Set bit 0
                break;

            // case SDLK_SPACE:
            case SDLK_K:                              // B (bit 1)
                form->joypad->keys_controls |= 0x2;   // Set bit 1
                break;

            case SDLK_LSHIFT:                         // SELECT (bit 2)
                form->joypad->keys_controls |= 0x4;   // Set bit 2
                break;

            case SDLK_RETURN:                         // START (bit 3)
                form->joypad->keys_controls |= 0x8;   // Set bit 3
                break;
            }
        }
        // // Handle joystick axis motion
        // else if (form->event->type == SDL_EVENT_GAMEPAD_AXIS_MOTION) {
        //     // Motion on controller 0
        //     if (form->event->gaxis.which == 0) {
        //         // X axis motion
        //         if (form->event->gaxis.axis == SDL_GAMEPAD_AXIS_LEFTX) {
        //             // Left of dead zone
        //             if (form->event->gaxis.value < -SDL_GAMEPAD_AXIS_LEFTX_DEADZONE) {
        //                 form->joypad->key_column       = 0x20;
        //                 form->joypad->column_direction = true;
        //                 form->joypad->keys_directions &= 0xD;
        //             }
        //             // Right of dead zone
        //             else if (form->event->gaxis.value > JOYSTICK_DEAD_ZONE) {
        //                 form->joypad->key_column       = 0x20;
        //                 form->joypad->column_direction = true;
        //                 form->joypad->keys_directions &= 0xE;
        //             }
        //             else {
        //                 form->joypad->key_column       = 0x20;
        //                 form->joypad->column_direction = true;
        //                 form->joypad->keys_directions |= 0x2;
        //                 form->joypad->keys_directions |= 0x1;
        //             }
        //         }
        //         // Y axis motion
        //         else if (form->event->gaxis.axis == SDL_GAMEPAD_AXIS_LEFTY) {
        //             // Up (Below dead zone)
        //             if (form->event->gaxis.value < -JOYSTICK_DEAD_ZONE) {
        //                 form->joypad->key_column       = 0x20;
        //                 form->joypad->column_direction = true;
        //                 form->joypad->keys_directions &= 0xB;
        //             }
        //             // Down (Above dead zone)
        //             else if (form->event->gaxis.value > JOYSTICK_DEAD_ZONE) {
        //                 form->joypad->key_column       = 0x20;
        //                 form->joypad->column_direction = true;
        //                 form->joypad->keys_directions &= 0x7;
        //             }
        //             else {
        //                 form->joypad->key_column       = 0x20;
        //                 form->joypad->column_direction = true;
        //                 form->joypad->keys_directions |= 0x8;
        //                 form->joypad->keys_directions |= 0x4;
        //             }
        //         }
        //     }
        // }
        // // Handle joystick button press
        // else if (form->event->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
        //     switch (form->event->gbutton.button) {
        //     case SDL_GAMEPAD_BUTTON_LABEL_A:
        //         form->joypad->key_column      = 0x10;
        //         form->joypad->column_controls = true;
        //         form->joypad->keys_controls &= 0xE;
        //         break;

        //     case SDL_GAMEPAD_BUTTON_LABEL_B:
        //         form->joypad->key_column      = 0x10;
        //         form->joypad->column_controls = true;
        //         form->joypad->keys_controls &= 0xD;
        //         break;

        //     case SDL_GAMEPAD_BUTTON_START:
        //         form->joypad->key_column      = 0x10;
        //         form->joypad->column_controls = true;
        //         form->joypad->keys_controls &= 0x7;
        //         break;

        //     case SDL_GAMEPAD_BUTTON_LABEL_X:   // Quick Save
        //         form->joypad->save_flag = 1;
        //         FORM_INFO_PRINT("Will save before next poll...%s\n", "");
        //         break;

        //     case SDL_GAMEPAD_BUTTON_LABEL_Y:   // Quick Load
        //         form->joypad->load_flag = 1;
        //         FORM_INFO_PRINT("Will load before next poll...%s\n", "");
        //         break;
        //     }
        // }
        // // Handle joystick button release
        // else if (form->event->type == SDL_EVENT_GAMEPAD_BUTTON_UP) {
        //     switch (form->event->gbutton.button) {
        //     case SDL_GAMEPAD_BUTTON_LABEL_A:
        //         form->joypad->key_column      = 0x10;
        //         form->joypad->column_controls = true;
        //         form->joypad->keys_controls |= 0x1;
        //         break;

        //     case SDL_GAMEPAD_BUTTON_LABEL_B:
        //         form->joypad->key_column      = 0x10;
        //         form->joypad->column_controls = true;
        //         form->joypad->keys_controls |= 0x2;
        //         break;

        //     case SDL_GAMEPAD_BUTTON_START:
        //         form->joypad->key_column      = 0x10;
        //         form->joypad->column_controls = true;
        //         form->joypad->keys_controls |= 0x8;
        //         break;
        //     }
        // }
    }

    // Update joypad state and trigger interrupts
    // form->joypad->handle_joypad_input(form->joypad);
    return true;
}