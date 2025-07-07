#include "dmg.h"

void show_usage(const char* program_name)
{
    printf("Usage: %s [options] <rom_file>\n", program_name);
    printf("Options:\n");
    printf("  -h, --help            Display this help message\n");
    printf("  -d                    Enable debug output\n");
    printf("  -v                    Verbose output (WARN, -v INFO, -vv DEBUG, -vvv TRACE, default: "
           "0)\n");
    printf("  -b, --bootrom <file>  Specify custom boot ROM\n");
    printf("  -s, --scale <n>       Window scale factor (1-4, default: 2)\n");
    printf("  -p, --serial          Enable serial output printing\n");
    printf("Examples:\n");
    printf("  %s mario.gb\n", program_name);
    printf("  %s -d -vv zelda.gb\n", program_name);
    printf("  %s --scale 3 pokemon.gb\n", program_name);
    // wait for user interaction
    printf("You can close the window now...");
    getchar();
}

struct EmulatorConfig config = {
    .rom_path                    = NULL,
    .bootrom_path                = NULL,
    .debug_mode                  = false,
    .scale_factor                = 2,
    .start_time                  = 0.0,
    .disable_color               = false,
    .verbose_level               = 0,
    .globals                     = NULL,
    .enable_serial_print         = false,
    .print_debug_info_this_frame = false,
    .fast_forward_mode           = false,
    .disable_joypad              = false
};

struct EmulatorConfig parse_args(int argc, char* argv[])
{
    struct EmulatorConfig config = {
        .rom_path                    = NULL,
        .bootrom_path                = NULL,
        .debug_mode                  = false,
        .scale_factor                = 2,
        .start_time                  = 0.0,
        .disable_color               = false,
        .verbose_level               = 0,
        .globals                     = NULL,
        .enable_serial_print         = false,
        .print_debug_info_this_frame = false,
        .fast_forward_mode           = false,
        .disable_joypad              = false};

    if (argc < 2) {
        show_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            show_usage(argv[0]);
            exit(EXIT_SUCCESS);
        }
        else if (strcmp(argv[i], "-d") == 0) {
            config.debug_mode = true;
        }
        else if (strncmp(argv[i], "-v", 2) == 0) {
            config.debug_mode    = true;
            config.verbose_level = strlen(argv[i]) - 1;   // -1 to account for first 'v'
            if (config.verbose_level > 3) {
                config.verbose_level = 3;
            }
        }
        else if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--bootrom") == 0) {
            if (i + 1 < argc) {
                config.bootrom_path = argv[++i];
            }
            else {
                fprintf(stderr, "Error: Boot ROM path missing\n");
                exit(EXIT_FAILURE);
            }
        }
        else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--scale") == 0) {
            if (i + 1 < argc) {
                config.scale_factor = atoi(argv[++i]);
                if (config.scale_factor < 1 || config.scale_factor > 6) {
                    fprintf(stderr, "Error: Scale factor must be between 1 and 6\n");
                    exit(EXIT_FAILURE);
                }
            }
            else {
                fprintf(stderr, "Error: Scale factor missing\n");
                exit(EXIT_FAILURE);
            }
        }
        else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--serial") == 0) {
            config.enable_serial_print = true;
        }
        else if (config.rom_path == NULL) {
            config.rom_path = argv[i];
        }
        else {
            fprintf(stderr, "Error: Unexpected argument '%s'\n", argv[i]);
            show_usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (config.rom_path == NULL) {
        fprintf(stderr, "Error: No ROM file specified\n");
        show_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    config.globals = (struct EmulatorGlobals*)malloc(sizeof(struct EmulatorGlobals));

    config.globals->is_stdout_redirected = is_stdout_redirected();

    DMG_DEBUG_PRINT("is_stdout_redirected: %d\n", config.globals->is_stdout_redirected);

    config.start_time = get_time_in_seconds();

    return config;
}

int main(int argc, char* argv[])
{
    config = parse_args(argc, argv);

    // Initialize emulator components
    DMG_WARN_PRINT("Verbose level %d enabled\n", config.verbose_level);
    DMG_INFO_PRINT("ROM: %s\n", config.rom_path);
    if (config.bootrom_path) {
        DMG_INFO_PRINT("Boot ROM: %s\n", config.bootrom_path);
    }
    DMG_INFO_PRINT("Scale factor: %d\n", config.scale_factor);

    // bring up cartridge
    DMG_DEBUG_PRINT("Bringing up cartridge...%s", "\n");
    struct Cartridge* cartridge = create_cartridge();
    if (cartridge == NULL) {
        DMG_EMERGENCY_PRINT("Failed to create cartridge\n");
        exit(EXIT_FAILURE);
    }
    DMG_INFO_PRINT("Loading cartridge from %s...%s", config.rom_path, "\n");
    if (!load_cartridge(cartridge, config.rom_path)) {
        DMG_EMERGENCY_PRINT("Failed to load cartridge\n");
        exit(EXIT_FAILURE);
    }

    // bring up ram
    DMG_DEBUG_PRINT("Bringing up ram...%s", "\n");
    struct Ram* ram = create_ram();
    if (ram == NULL) {
        DMG_EMERGENCY_PRINT("Failed to create ram\n");
        exit(EXIT_FAILURE);
    }

    // bring up vram
    DMG_DEBUG_PRINT("Bringing up vram...%s", "\n");
    struct Vram* vram = create_vram();
    if (vram == NULL) {
        DMG_EMERGENCY_PRINT("Failed to create vram\n");
        exit(EXIT_FAILURE);
    }

    // bring up ppu
    DMG_DEBUG_PRINT("Bringing up ppu...%s", "\n");
    struct PPU* ppu = create_ppu(vram);
    if (ppu == NULL) {
        DMG_EMERGENCY_PRINT("Failed to create ppu\n");
        exit(EXIT_FAILURE);
    }

    // bring up mmu
    DMG_DEBUG_PRINT("Bringing up mmu...%s", "\n");
    struct MMU* mmu = create_mmu(cartridge, ram, ppu);
    if (mmu == NULL) {
        DMG_EMERGENCY_PRINT("Failed to create mmu\n");
        exit(EXIT_FAILURE);
    }

    // attach mmu to ppu
    // TODO: find a better way to do this or change the logic
    DMG_DEBUG_PRINT("Attaching mmu to ppu...%s", "\n");
    ppu_attach_mmu(ppu, mmu);

    // bring up registers
    DMG_DEBUG_PRINT("Bringing up registers...%s", "\n");
    struct Registers* registers = create_registers();
    if (registers == NULL) {
        DMG_EMERGENCY_PRINT("Failed to create registers\n");
        exit(EXIT_FAILURE);
    }

    // bring up cpu
    DMG_DEBUG_PRINT("Bringing up cpu...%s", "\n");
    struct CPU* cpu = create_cpu(registers, mmu);
    if (cpu == NULL) {
        DMG_EMERGENCY_PRINT("Failed to create cpu\n");
        exit(EXIT_FAILURE);
    }
    cpu_set_serial_output(cpu, config.enable_serial_print);

    // bring up timer
    DMG_DEBUG_PRINT("Bringing up timer...%s", "\n");
    struct Timer* timer = create_timer();
    if (timer == NULL) {
        DMG_EMERGENCY_PRINT("Failed to create timer\n");
        exit(EXIT_FAILURE);
    }
    DMG_DEBUG_PRINT("Attaching ram to timer...%s", "\n");
    timer_attach_ram(timer, ram);
    DMG_DEBUG_PRINT("Attaching cpu to timer...%s", "\n");
    cpu_attach_timer(cpu, timer);

    // bring up joypad
    DMG_DEBUG_PRINT("Bringing up joypad...%s", "\n");
    struct Joypad* joypad = create_joypad(mmu);
    if (joypad == NULL) {
        DMG_EMERGENCY_PRINT("Failed to create joypad\n");
        exit(EXIT_FAILURE);
    }
    DMG_DEBUG_PRINT("Attaching joypad to mmu...%s", "\n");
    mmu_attach_joypad(mmu, joypad);

    // Set up SDL with the configured scale factor
    DMG_DEBUG_PRINT("Creating form...%s", "\n");
    struct Form* form = create_form(ppu, joypad, cartridge->rom_name);
    if (form == NULL) {
        DMG_EMERGENCY_PRINT("Failed to create form\n");
        exit(EXIT_FAILURE);
    }

    // initializing ram
    DMG_DEBUG_PRINT("Initializing ram registers...%s", "\n");
    initialize_ram(ram);

    // Main emulation loop here
    DMG_DEBUG_PRINT("Starting emulation loop...%s", "\n");
    main_loop(ppu, cpu, timer, form);

    // Clean up
    DMG_DEBUG_PRINT("Cleaning up...%s", "\n");
    free_cpu(cpu);
    free_form(form);

    return 0;
}

void main_loop(struct PPU* ppu, struct CPU* cpu, struct Timer* timer, struct Form* form)
{

    // record time for each frame
    double last_time   = get_time_in_seconds();
    double start_time  = last_time;
    int    frame_count = 1;
    float  fps         = get_current_window_display_mode(form)->refresh_rate;

    while (true) {
        // Process input - if this returns false, exit the loop
        if (!get_joypad_state(form)) {
            break;
        }

        next_frame(ppu, cpu, frame_count);

        // update surface
        update_surface(form);

        // sleep to maintain fps
        double current_time = get_time_in_seconds();
        double elapsed_time = current_time - last_time;
        if (elapsed_time < 1.0 / fps && !config.fast_forward_mode) {
            double          sleep_seconds = 1.0 / fps - elapsed_time;
            struct timespec sleep_time    = {
                   .tv_sec  = (time_t)sleep_seconds,
                   .tv_nsec = (long)((sleep_seconds - (time_t)sleep_seconds) * 1000000000)};
            nanosleep(&sleep_time, NULL);
        }
        last_time = current_time;
        if (config.print_debug_info_this_frame) {
            // Calculate FPS
            double fps_total      = frame_count / (current_time - start_time);
            double fps_this_frame = 1.0 / elapsed_time;
            DMG_INFO_PRINT("FPS Total: %lf\n", fps_total);
            DMG_INFO_PRINT("FPS This Frame (without sleep): %lf\n", fps_this_frame);
            config.print_debug_info_this_frame = false;
        }
        frame_count += 1;
    }
}

void next_frame(struct PPU* ppu, struct CPU* cpu, int current_frame)
{
    // Check if LCD is disabled
    while (!ppu_is_lcd_enabled(ppu)) {
        // When LCD is disabled, set LY to 0 and stay in V-Blank
        ppu_set_mode(ppu, MODE_VBLANK);
        ppu_set_ly(ppu, 0);
        // Execute instructions for a full frame duration (154 scanlines)
        for (uint8_t i = 0; i < 154; i++) {
            cpu_step_for_cycles(cpu, 456);
            // Don't step PPU when LCD is disabled
        }
        return;
    }

    // "Render" 144 visible scanlines (0-143)
    for (uint8_t ly = 0; ly < 144; ly++) {
        uint8_t lcdc_current = cpu->mmu->mmu_get_byte(cpu->mmu, LCDC_ADDRESS);
        // reset interrupt flag
        // uint8_t int_flag = cpu->mmu->mmu_get_byte(cpu->mmu, IF_ADDRESS);
        // int_flag &= 0xFC;
        // cpu->mmu->mmu_set_byte(cpu->mmu, IF_ADDRESS, int_flag);
        // OAM Scan (Mode 2)
        // https://hacktix.github.io/GBEDG/ppu/
        // This mode is entered at the start of every scanline (except for V-Blank) before pixels
        // are actually drawn to the screen. During this mode the PPU searches OAM memory for
        // sprites that should be rendered on the current scanline and stores them in a buffer. This
        // procedure takes a total amount of 80 T-Cycles, meaning that the PPU checks a new OAM
        // entry every 2 T-Cycles. A sprite is only added to the buffer if all of the following
        // conditions apply: Sprite X-Position must be greater than 0 LY + 16 must be greater than
        // or equal to Sprite Y-Position LY + 16 must be less than Sprite Y-Position + Sprite Height
        // (8 in Normal Mode, 16 in Tall-Sprite-Mode) The amount of sprites already stored in the
        // OAM Buffer must be less than 10 CPU can't access OAM here
        ppu_set_ly(ppu, ly);
        ppu_set_mode(ppu, MODE_OAM_SEARCH);
        if ((config.fast_forward_mode && current_frame % 4 == 0) || !config.fast_forward_mode) {
            ppu_oam_search(ppu);  // Actually perform OAM search to populate sprite buffer!
        }
        cpu_step_for_cycles(cpu, 80);

        // Pixel Transfer (Mode 3)
        // https://hacktix.github.io/GBEDG/ppu/
        // The Drawing Mode is where the PPU transfers pixels to the LCD. The duration of this mode
        // changes depending on multiple variables, such as background scrolling, the amount of
        // sprites on the scanline, whether or not the window should be rendered, etc. All of the
        // specifics to these timing differences will be explained later on. CPU can't access VRAM
        // and OAM here
        ppu_set_mode(ppu, MODE_PIXEL_TRANSFER);
        cpu_step_for_cycles(cpu, 172);
        if ((config.fast_forward_mode && current_frame % 4 == 0) || !config.fast_forward_mode) {
            ppu_render_scanline_ly(ppu, ly);
        }
        // ppu_render_scanline_fifo(ppu, ly);

        // MODE 0: H-Blank (204 cycles to complete 456 total)
        // H-Blank (Mode 0)
        // https://hacktix.github.io/GBEDG/ppu/
        // This mode takes up the remainder of the scanline after the Drawing Mode finishes, more or
        // less "padding" the duration of the scanline to a total of 456 T-Cycles. The PPU
        // effectively pauses during this mode.
        ppu_set_mode(ppu, MODE_HBLANK);
        cpu_step_for_cycles(cpu, 204);
    }

    // V-Blank interrupt happening here
    uint8_t int_flag = cpu->mmu->mmu_get_byte(cpu->mmu, IF_ADDRESS);
    int_flag |= 0x01;
    cpu->mmu->mmu_set_byte(cpu->mmu, IF_ADDRESS, int_flag);

    // ppu_render_full_frame(ppu);

    // V-Blank period: scanlines 144-153 (10 scanlines in MODE 1)
    // https://hacktix.github.io/GBEDG/ppu/
    // V-Blank mode is the same as H-Blank in the way that the PPU does not draw any pixels to the
    // LCD during its duration. However, instead of it taking place at the end of every scanline,
    // it's a much longer period at the end of every frame. As the Gameboy has a vertical resolution
    // of 144 pixels, it would be expected that the amount of scanlines the PPU handles would be
    // equal - 144 scanlines. However, this is not the case. In reality there are 154 scanlines, the
    // 10 last of which being "pseudo-scanlines" during which no pixels are drawn as the PPU is in
    // the V-Blank state during their duration. A V-Blank scanline takes the same amount of time as
    // any other scanline - 456 T-Cycles.

    for (uint8_t ly = 144; ly < 154; ly++) {
        // MODE 1: V-Blank (456 cycles per scanline)
        ppu_set_ly(ppu, ly);
        ppu_set_mode(ppu, MODE_VBLANK);
        cpu_step_for_cycles(cpu, 456);
    }
}

void initialize_ram(struct Ram* ram)
{
    // initializing ram
    DMG_DEBUG_PRINT("Initializing ram registers...%s", "\n");
    ram->set_ram_byte(ram, 0xFF05, 0x00);
    ram->set_ram_byte(ram, 0xFF06, 0x00);
    ram->set_ram_byte(ram, 0xFF07, 0x00);
    ram->set_ram_byte(ram, 0xFF10, 0x80);
    ram->set_ram_byte(ram, 0xFF11, 0xBF);
    ram->set_ram_byte(ram, 0xFF12, 0xF3);
    ram->set_ram_byte(ram, 0xFF14, 0xBF);
    ram->set_ram_byte(ram, 0xFF16, 0x3F);
    ram->set_ram_byte(ram, 0xFF17, 0x00);
    ram->set_ram_byte(ram, 0xFF19, 0xBF);
    ram->set_ram_byte(ram, 0xFF1A, 0x7F);
    ram->set_ram_byte(ram, 0xFF1B, 0xFF);
    ram->set_ram_byte(ram, 0xFF1C, 0x9F);
    ram->set_ram_byte(ram, 0xFF1E, 0xBF);
    ram->set_ram_byte(ram, 0xFF20, 0xFF);
    ram->set_ram_byte(ram, 0xFF21, 0x00);
    ram->set_ram_byte(ram, 0xFF22, 0x00);
    ram->set_ram_byte(ram, 0xFF23, 0xBF);
    ram->set_ram_byte(ram, 0xFF24, 0x77);
    ram->set_ram_byte(ram, 0xFF25, 0xF3);
    ram->set_ram_byte(ram, 0xFF26, 0xF1);
    ram->set_ram_byte(ram, 0xFF40, 0x91);
    ram->set_ram_byte(ram, 0xFF42, 0x00);
    ram->set_ram_byte(ram, 0xFF43, 0x00);
    ram->set_ram_byte(ram, 0xFF45, 0x00);
    ram->set_ram_byte(ram, 0xFF47, 0xFC);
    ram->set_ram_byte(ram, 0xFF48, 0xFF);
    ram->set_ram_byte(ram, 0xFF49, 0xFF);
    ram->set_ram_byte(ram, 0xFF4A, 0x00);
    ram->set_ram_byte(ram, 0xFF4B, 0x00);
    ram->set_ram_byte(ram, 0xFFFF, 0x00);
}