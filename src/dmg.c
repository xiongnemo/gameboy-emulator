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
    // getchar();
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
    DMG_DEBUG_PRINT("Attaching timer to MMU...%s", "\n");
    mmu_attach_timer(mmu, timer);
    DMG_DEBUG_PRINT("Attaching cpu to timer...%s", "\n");
    cpu_attach_timer(cpu, timer);
    DMG_DEBUG_PRINT("Attaching PPU to CPU...%s", "\n");
    cpu_attach_ppu(cpu, ppu);

    // bring up apu
    DMG_DEBUG_PRINT("Bringing up APU...%s", "\n");
    struct APU* apu = create_apu();
    if (apu == NULL) {
        DMG_EMERGENCY_PRINT("Failed to create APU\n");
        exit(EXIT_FAILURE);
    }
    DMG_DEBUG_PRINT("Attaching MMU to APU...%s", "\n");
    apu_attach_mmu(apu, mmu);
    DMG_DEBUG_PRINT("Attaching APU to MMU...%s", "\n");
    mmu_attach_apu(mmu, apu);
    DMG_DEBUG_PRINT("Attaching APU to CPU...%s", "\n");
    cpu_attach_apu(cpu, apu);

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
    if (!apu_start_audio(apu)) {
        DMG_WARN_PRINT("APU audio output unavailable; continuing in silent mode\n");
    }

    // initializing ram
    DMG_DEBUG_PRINT("Initializing ram registers...%s", "\n");
    initialize_ram(ram);
    
    // Initialize APU registers with default values
    DMG_DEBUG_PRINT("Initializing APU registers...%s", "\n");
    if (apu && mmu) {
        mmu->mmu_set_byte(mmu, 0xFF26, 0xF1);
        mmu->mmu_set_byte(mmu, 0xFF10, 0x80);
        mmu->mmu_set_byte(mmu, 0xFF11, 0xBF);
        mmu->mmu_set_byte(mmu, 0xFF12, 0xF3);
        mmu->mmu_set_byte(mmu, 0xFF14, 0xBF);
        mmu->mmu_set_byte(mmu, 0xFF16, 0x3F);
        mmu->mmu_set_byte(mmu, 0xFF17, 0x00);
        mmu->mmu_set_byte(mmu, 0xFF19, 0xBF);
        mmu->mmu_set_byte(mmu, 0xFF1A, 0x7F);
        mmu->mmu_set_byte(mmu, 0xFF1B, 0xFF);
        mmu->mmu_set_byte(mmu, 0xFF1C, 0x9F);
        mmu->mmu_set_byte(mmu, 0xFF1E, 0xBF);
        mmu->mmu_set_byte(mmu, 0xFF20, 0xFF);
        mmu->mmu_set_byte(mmu, 0xFF21, 0x00);
        mmu->mmu_set_byte(mmu, 0xFF22, 0x00);
        mmu->mmu_set_byte(mmu, 0xFF23, 0xBF);
        mmu->mmu_set_byte(mmu, 0xFF24, 0x77);
        mmu->mmu_set_byte(mmu, 0xFF25, 0xF3);
    }

    // Main emulation loop here
    DMG_DEBUG_PRINT("Starting emulation loop...%s", "\n");
    main_loop(ppu, cpu, timer, form, apu);

    // Clean up
    DMG_DEBUG_PRINT("Cleaning up...%s", "\n");
    free_apu(apu);
    free_cpu(cpu);
    free_form(form);

    return 0;
}

void main_loop(struct PPU* ppu, struct CPU* cpu, struct Timer* timer, struct Form* form, struct APU* apu)
{
    (void)timer;
    (void)apu;

    // record time for each frame
    double last_time   = get_time_in_seconds();
    double start_time  = last_time;
    Uint64 next_frame_deadline_ns = SDL_GetTicksNS();
    const Uint64 frame_duration_ns =
        (Uint64)(((uint64_t)SDL_NS_PER_SECOND * GB_FRAME_M_CYCLES) / GB_M_CYCLE_HZ);
    int frame_count = 1;

    while (true) {
        // Process input - if this returns false, exit the loop
        if (!get_joypad_state(form)) {
            break;
        }

        next_frame(ppu, cpu);

        // update surface
        update_surface(form);

        // sleep to maintain fps
        double current_time = get_time_in_seconds();
        double elapsed_time = current_time - last_time;
        next_frame_deadline_ns += frame_duration_ns;
        if (!config.fast_forward_mode) {
            Uint64 now_ns = SDL_GetTicksNS();
            if (now_ns < next_frame_deadline_ns) {
                SDL_DelayPrecise(next_frame_deadline_ns - now_ns);
            }
            else {
                next_frame_deadline_ns = now_ns;
            }
        }
        double after_sleep_time = get_time_in_seconds();
        last_time = after_sleep_time;
        if (config.print_debug_info_this_frame) {
            // Calculate FPS
            double fps_total      = frame_count / (after_sleep_time - start_time);
            double fps_this_frame = 1.0 / elapsed_time;
            DMG_INFO_PRINT("FPS Total: %lf\n", fps_total);
            DMG_INFO_PRINT("FPS This Frame (without sleep): %lf\n", fps_this_frame);
            config.print_debug_info_this_frame = false;
        }
        frame_count += 1;
    }
}

void next_frame(struct PPU* ppu, struct CPU* cpu)
{
    while (!ppu_consume_frame_ready(ppu)) {
        cpu_step_for_cycles(cpu, 1);
    }
}

void initialize_ram(struct Ram* ram)
{
    // initializing ram
    DMG_DEBUG_PRINT("Initializing ram registers...%s", "\n");
    ram->set_ram_byte(ram, 0xFF05, 0x00);
    ram->set_ram_byte(ram, 0xFF06, 0x00);
    ram->set_ram_byte(ram, 0xFF07, 0x00);
    // APU registers are now handled by the APU component
    // These will be initialized through the MMU which routes to APU
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
