#include "dmg.h"

void print_usage(const char* program_name)
{
    printf("Usage: %s [options] <rom_file>\n", program_name);
    printf("Options:\n");
    printf("  -h, --help            Display this help message\n");
    printf("  -d                    Enable debug output\n");
    printf("  -v                    Verbose output (WARN, -v INFO, -vv DEBUG, -vvv TRACE, default: "
           "0)\n");
    printf("  -b, --bootrom <file>  Specify custom boot ROM\n");
    printf("  -s, --scale <n>       Window scale factor (1-4, default: 2)\n");
    printf("Examples:\n");
    printf("  %s mario.gb\n", program_name);
    printf("  %s -d -vv zelda.gb\n", program_name);
    printf("  %s --scale 3 pokemon.gb\n", program_name);
}

struct EmulatorConfig config = {
    .rom_path      = NULL,
    .bootrom_path  = NULL,
    .debug_mode    = false,
    .scale_factor  = 2,
    .start_time    = 0.0,
    .disable_color = false,
    .verbose_level = 0,
    .globals       = NULL};

struct EmulatorConfig parse_args(int argc, char* argv[])
{
    struct EmulatorConfig config = {
        .rom_path      = NULL,
        .bootrom_path  = NULL,
        .debug_mode    = false,
        .scale_factor  = 2,
        .start_time    = 0.0,
        .disable_color = false,
        .verbose_level = 0,
        .globals       = NULL};

    if (argc < 2) {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
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
                if (config.scale_factor < 1 || config.scale_factor > 4) {
                    fprintf(stderr, "Error: Scale factor must be between 1 and 4\n");
                    exit(EXIT_FAILURE);
                }
            }
            else {
                fprintf(stderr, "Error: Scale factor missing\n");
                exit(EXIT_FAILURE);
            }
        }
        else if (config.rom_path == NULL) {
            config.rom_path = argv[i];
        }
        else {
            fprintf(stderr, "Error: Unexpected argument '%s'\n", argv[i]);
            print_usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (config.rom_path == NULL) {
        fprintf(stderr, "Error: No ROM file specified\n");
        print_usage(argv[0]);
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
    DMG_DEBUG_PRINT("Loading cartridge from %s...%s", config.rom_path, "\n");
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

    // bring up joypad
    DMG_DEBUG_PRINT("Bringing up joypad...%s", "\n");
    struct Joypad* joypad = create_joypad(mmu);
    if (joypad == NULL) {
        DMG_EMERGENCY_PRINT("Failed to create joypad\n");
        exit(EXIT_FAILURE);
    }

    // Set up SDL with the configured scale factor
    DMG_DEBUG_PRINT("Creating form...%s", "\n");
    struct Form* form = create_form(ppu, joypad);
    if (form == NULL) {
        DMG_EMERGENCY_PRINT("Failed to create form\n");
        exit(EXIT_FAILURE);
    }

    // Main emulation loop here
    DMG_DEBUG_PRINT("Starting emulation loop...%s", "\n");

    // Clean up
    DMG_DEBUG_PRINT("Cleaning up...%s", "\n");
    free_cpu(cpu);
    free_form(form);

    return 0;
}
