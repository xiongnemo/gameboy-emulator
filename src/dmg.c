#include "dmg.h"

#ifdef _WIN32
#    include <direct.h>
#    define DMG_MKDIR(path) _mkdir(path)
#else
#    include <sys/stat.h>
#    define DMG_MKDIR(path) mkdir(path, 0755)
#endif

#define SAVE_STATE_VERSION 1

struct SaveStateHeader
{
    char     magic[8];
    uint32_t version;
    uint64_t rom_hash;
};

static bool write_block(FILE* file, const void* data, size_t size)
{
    return fwrite(data, 1, size, file) == size;
}

static bool read_block(FILE* file, void* data, size_t size)
{
    return fread(data, 1, size, file) == size;
}

static char* save_state_path(struct Cartridge* cartridge, uint8_t slot)
{
    if (!cartridge) {
        return NULL;
    }

    DMG_MKDIR("states");

    char dir[64];
    snprintf(dir, sizeof(dir), "states/%016llx", (unsigned long long)cartridge->rom_hash);
    DMG_MKDIR(dir);

    size_t length = strlen(dir) + 24;
    char* path = malloc(length);
    if (!path) {
        return NULL;
    }
    snprintf(path, length, "%s/slot%u.state", dir, slot);
    return path;
}

static bool save_quick_state(struct CPU* cpu, struct Timer* timer, struct PPU* ppu, struct APU* apu, uint8_t slot)
{
    if (!cpu || !cpu->mmu || !timer || !ppu) {
        return false;
    }

    struct Cartridge* cartridge = cpu->mmu->cartridge;
    char* path = save_state_path(cartridge, slot);
    if (!path) {
        return false;
    }

    FILE* file = fopen(path, "wb");
    if (!file) {
        free(path);
        return false;
    }

    struct SaveStateHeader header = {{'D', 'M', 'G', 'S', 'T', 'A', 'T', 'E'}, SAVE_STATE_VERSION, cartridge->rom_hash};
    bool ok = true;
    ok = write_block(file, &header, sizeof(header)) && ok;

    ok = write_block(file, cpu->registers->reg_primary, sizeof(cpu->registers->reg_primary)) && ok;
    ok = write_block(file, cpu->registers->reg_control, sizeof(cpu->registers->reg_control)) && ok;
    ok = write_block(file, &cpu->halted, sizeof(cpu->halted)) && ok;
    ok = write_block(file, &cpu->stopped, sizeof(cpu->stopped)) && ok;
    ok = write_block(file, &cpu->interrupt_master_enable, sizeof(cpu->interrupt_master_enable)) && ok;
    ok = write_block(file, &cpu->ime_enable_delay, sizeof(cpu->ime_enable_delay)) && ok;
    ok = write_block(file, &cpu->cycles, sizeof(cpu->cycles)) && ok;
    ok = write_block(file, &cpu->dma_stall_m_cycles, sizeof(cpu->dma_stall_m_cycles)) && ok;
    ok = write_block(file, &cpu->mmu->pending_dma_stall_m_cycles, sizeof(cpu->mmu->pending_dma_stall_m_cycles)) && ok;

    ok = write_block(file, cpu->mmu->ram->ram_byte, RAM_SIZE) && ok;
    ok = write_block(file, ppu->vram->vram_byte, VRAM_SIZE) && ok;
    ok = write_block(file, ppu->framebuffer, SCREEN_WIDTH * SCREEN_HEIGHT) && ok;

    ok = write_block(file, &ppu->ppu_inner_clock, sizeof(ppu->ppu_inner_clock)) && ok;
    ok = write_block(file, &ppu->mode_dots, sizeof(ppu->mode_dots)) && ok;
    ok = write_block(file, &ppu->mode, sizeof(ppu->mode)) && ok;
    ok = write_block(file, &ppu->frame_ready, sizeof(ppu->frame_ready)) && ok;
    ok = write_block(file, &ppu->lcd_enabled, sizeof(ppu->lcd_enabled)) && ok;
    ok = write_block(file, &ppu->frame_count, sizeof(ppu->frame_count)) && ok;
    ok = write_block(file, &ppu->ly, sizeof(ppu->ly)) && ok;
    ok = write_block(file, &ppu->lcdc, sizeof(ppu->lcdc)) && ok;
    ok = write_block(file, &ppu->stat, sizeof(ppu->stat)) && ok;
    ok = write_block(file, &ppu->scx, sizeof(ppu->scx)) && ok;
    ok = write_block(file, &ppu->scy, sizeof(ppu->scy)) && ok;
    ok = write_block(file, &ppu->lyc, sizeof(ppu->lyc)) && ok;
    ok = write_block(file, &ppu->wy, sizeof(ppu->wy)) && ok;
    ok = write_block(file, &ppu->wx, sizeof(ppu->wx)) && ok;
    ok = write_block(file, &ppu->bgp, sizeof(ppu->bgp)) && ok;
    ok = write_block(file, &ppu->obp0, sizeof(ppu->obp0)) && ok;
    ok = write_block(file, &ppu->obp1, sizeof(ppu->obp1)) && ok;

    ok = write_block(file, &timer->divider, sizeof(timer->divider)) && ok;
    ok = write_block(file, &timer->reg_tima, sizeof(timer->reg_tima)) && ok;
    ok = write_block(file, &timer->reg_tma, sizeof(timer->reg_tma)) && ok;
    ok = write_block(file, &timer->reg_tac, sizeof(timer->reg_tac)) && ok;
    ok = write_block(file, &timer->tima_overflow_pending, sizeof(timer->tima_overflow_pending)) && ok;
    ok = write_block(file, &timer->tima_overflow_dots, sizeof(timer->tima_overflow_dots)) && ok;

    if (apu) {
        ok = write_block(file, &apu->square1, sizeof(apu->square1)) && ok;
        ok = write_block(file, &apu->square2, sizeof(apu->square2)) && ok;
        ok = write_block(file, &apu->wave, sizeof(apu->wave)) && ok;
        ok = write_block(file, &apu->noise, sizeof(apu->noise)) && ok;
        ok = write_block(file, &apu->frame_sequencer_step, sizeof(apu->frame_sequencer_step)) && ok;
        ok = write_block(file, &apu->frame_sequencer_counter, sizeof(apu->frame_sequencer_counter)) && ok;
        ok = write_block(file, &apu->sound_enabled, sizeof(apu->sound_enabled)) && ok;
        ok = write_block(file, &apu->left_volume, sizeof(apu->left_volume)) && ok;
        ok = write_block(file, &apu->right_volume, sizeof(apu->right_volume)) && ok;
    }

    ok = write_block(file, &cartridge->ram_enabled, sizeof(cartridge->ram_enabled)) && ok;
    ok = write_block(file, &cartridge->mbc2_ram_enabled, sizeof(cartridge->mbc2_ram_enabled)) && ok;
    ok = write_block(file, cartridge->mbc2_ram, sizeof(cartridge->mbc2_ram)) && ok;
    ok = write_block(file, &cartridge->mbc1_rom_bank_low5, sizeof(cartridge->mbc1_rom_bank_low5)) && ok;
    ok = write_block(file, &cartridge->mbc1_bank_high2, sizeof(cartridge->mbc1_bank_high2)) && ok;
    ok = write_block(file, &cartridge->mbc1_banking_mode, sizeof(cartridge->mbc1_banking_mode)) && ok;
    ok = write_block(file, &cartridge->rom_alternative_bank, sizeof(cartridge->rom_alternative_bank)) && ok;
    ok = write_block(file, &cartridge->ram_alternative_bank, sizeof(cartridge->ram_alternative_bank)) && ok;
    ok = write_block(file, &cartridge->mbc5_rom_bank, sizeof(cartridge->mbc5_rom_bank)) && ok;
    ok = write_block(file, &cartridge->rumble_motor_on, sizeof(cartridge->rumble_motor_on)) && ok;
    ok = write_block(file, &cartridge->rtc_selected_register, sizeof(cartridge->rtc_selected_register)) && ok;
    ok = write_block(file, cartridge->rtc_registers, sizeof(cartridge->rtc_registers)) && ok;
    ok = write_block(file, cartridge->rtc_latched_registers, sizeof(cartridge->rtc_latched_registers)) && ok;
    ok = write_block(file, &cartridge->rtc_latched, sizeof(cartridge->rtc_latched)) && ok;
    ok = write_block(file, &cartridge->rtc_latch_previous, sizeof(cartridge->rtc_latch_previous)) && ok;
    ok = write_block(file, &cartridge->rtc_last_update, sizeof(cartridge->rtc_last_update)) && ok;
    size_t ram_size = cartridge->ram_size;
    ok = write_block(file, &ram_size, sizeof(ram_size)) && ok;
    if (ram_size > 0 && cartridge->ram_data) {
        ok = write_block(file, cartridge->ram_data, ram_size) && ok;
    }

    ok = fclose(file) == 0 && ok;
    FORM_INFO_PRINT("Saved state slot %u: %s\n", slot, path);
    free(path);
    return ok;
}

static bool load_quick_state(struct CPU* cpu, struct Timer* timer, struct PPU* ppu, struct APU* apu, uint8_t slot)
{
    if (!cpu || !cpu->mmu || !timer || !ppu) {
        return false;
    }

    struct Cartridge* cartridge = cpu->mmu->cartridge;
    char* path = save_state_path(cartridge, slot);
    if (!path) {
        return false;
    }

    FILE* file = fopen(path, "rb");
    if (!file) {
        FORM_WARN_PRINT("No save state slot %u found\n", slot);
        free(path);
        return false;
    }

    struct SaveStateHeader header;
    bool ok = read_block(file, &header, sizeof(header));
    if (!ok || memcmp(header.magic, "DMGSTATE", 8) != 0 ||
        header.version != SAVE_STATE_VERSION || header.rom_hash != cartridge->rom_hash) {
        fclose(file);
        free(path);
        return false;
    }

    ok = read_block(file, cpu->registers->reg_primary, sizeof(cpu->registers->reg_primary)) && ok;
    ok = read_block(file, cpu->registers->reg_control, sizeof(cpu->registers->reg_control)) && ok;
    ok = read_block(file, &cpu->halted, sizeof(cpu->halted)) && ok;
    ok = read_block(file, &cpu->stopped, sizeof(cpu->stopped)) && ok;
    ok = read_block(file, &cpu->interrupt_master_enable, sizeof(cpu->interrupt_master_enable)) && ok;
    ok = read_block(file, &cpu->ime_enable_delay, sizeof(cpu->ime_enable_delay)) && ok;
    ok = read_block(file, &cpu->cycles, sizeof(cpu->cycles)) && ok;
    ok = read_block(file, &cpu->dma_stall_m_cycles, sizeof(cpu->dma_stall_m_cycles)) && ok;
    ok = read_block(file, &cpu->mmu->pending_dma_stall_m_cycles, sizeof(cpu->mmu->pending_dma_stall_m_cycles)) && ok;

    ok = read_block(file, cpu->mmu->ram->ram_byte, RAM_SIZE) && ok;
    ok = read_block(file, ppu->vram->vram_byte, VRAM_SIZE) && ok;
    ok = read_block(file, ppu->framebuffer, SCREEN_WIDTH * SCREEN_HEIGHT) && ok;

    ok = read_block(file, &ppu->ppu_inner_clock, sizeof(ppu->ppu_inner_clock)) && ok;
    ok = read_block(file, &ppu->mode_dots, sizeof(ppu->mode_dots)) && ok;
    ok = read_block(file, &ppu->mode, sizeof(ppu->mode)) && ok;
    ok = read_block(file, &ppu->frame_ready, sizeof(ppu->frame_ready)) && ok;
    ok = read_block(file, &ppu->lcd_enabled, sizeof(ppu->lcd_enabled)) && ok;
    ok = read_block(file, &ppu->frame_count, sizeof(ppu->frame_count)) && ok;
    ok = read_block(file, &ppu->ly, sizeof(ppu->ly)) && ok;
    ok = read_block(file, &ppu->lcdc, sizeof(ppu->lcdc)) && ok;
    ok = read_block(file, &ppu->stat, sizeof(ppu->stat)) && ok;
    ok = read_block(file, &ppu->scx, sizeof(ppu->scx)) && ok;
    ok = read_block(file, &ppu->scy, sizeof(ppu->scy)) && ok;
    ok = read_block(file, &ppu->lyc, sizeof(ppu->lyc)) && ok;
    ok = read_block(file, &ppu->wy, sizeof(ppu->wy)) && ok;
    ok = read_block(file, &ppu->wx, sizeof(ppu->wx)) && ok;
    ok = read_block(file, &ppu->bgp, sizeof(ppu->bgp)) && ok;
    ok = read_block(file, &ppu->obp0, sizeof(ppu->obp0)) && ok;
    ok = read_block(file, &ppu->obp1, sizeof(ppu->obp1)) && ok;

    ok = read_block(file, &timer->divider, sizeof(timer->divider)) && ok;
    ok = read_block(file, &timer->reg_tima, sizeof(timer->reg_tima)) && ok;
    ok = read_block(file, &timer->reg_tma, sizeof(timer->reg_tma)) && ok;
    ok = read_block(file, &timer->reg_tac, sizeof(timer->reg_tac)) && ok;
    ok = read_block(file, &timer->tima_overflow_pending, sizeof(timer->tima_overflow_pending)) && ok;
    ok = read_block(file, &timer->tima_overflow_dots, sizeof(timer->tima_overflow_dots)) && ok;

    if (apu) {
        ok = read_block(file, &apu->square1, sizeof(apu->square1)) && ok;
        ok = read_block(file, &apu->square2, sizeof(apu->square2)) && ok;
        ok = read_block(file, &apu->wave, sizeof(apu->wave)) && ok;
        ok = read_block(file, &apu->noise, sizeof(apu->noise)) && ok;
        ok = read_block(file, &apu->frame_sequencer_step, sizeof(apu->frame_sequencer_step)) && ok;
        ok = read_block(file, &apu->frame_sequencer_counter, sizeof(apu->frame_sequencer_counter)) && ok;
        ok = read_block(file, &apu->sound_enabled, sizeof(apu->sound_enabled)) && ok;
        ok = read_block(file, &apu->left_volume, sizeof(apu->left_volume)) && ok;
        ok = read_block(file, &apu->right_volume, sizeof(apu->right_volume)) && ok;
        apu->sample_read_index = 0;
        apu->sample_write_index = 0;
        apu->sample_count = 0;
        apu->sample_cycle_accumulator = 0;
    }

    ok = read_block(file, &cartridge->ram_enabled, sizeof(cartridge->ram_enabled)) && ok;
    ok = read_block(file, &cartridge->mbc2_ram_enabled, sizeof(cartridge->mbc2_ram_enabled)) && ok;
    ok = read_block(file, cartridge->mbc2_ram, sizeof(cartridge->mbc2_ram)) && ok;
    ok = read_block(file, &cartridge->mbc1_rom_bank_low5, sizeof(cartridge->mbc1_rom_bank_low5)) && ok;
    ok = read_block(file, &cartridge->mbc1_bank_high2, sizeof(cartridge->mbc1_bank_high2)) && ok;
    ok = read_block(file, &cartridge->mbc1_banking_mode, sizeof(cartridge->mbc1_banking_mode)) && ok;
    ok = read_block(file, &cartridge->rom_alternative_bank, sizeof(cartridge->rom_alternative_bank)) && ok;
    ok = read_block(file, &cartridge->ram_alternative_bank, sizeof(cartridge->ram_alternative_bank)) && ok;
    ok = read_block(file, &cartridge->mbc5_rom_bank, sizeof(cartridge->mbc5_rom_bank)) && ok;
    ok = read_block(file, &cartridge->rumble_motor_on, sizeof(cartridge->rumble_motor_on)) && ok;
    ok = read_block(file, &cartridge->rtc_selected_register, sizeof(cartridge->rtc_selected_register)) && ok;
    ok = read_block(file, cartridge->rtc_registers, sizeof(cartridge->rtc_registers)) && ok;
    ok = read_block(file, cartridge->rtc_latched_registers, sizeof(cartridge->rtc_latched_registers)) && ok;
    ok = read_block(file, &cartridge->rtc_latched, sizeof(cartridge->rtc_latched)) && ok;
    ok = read_block(file, &cartridge->rtc_latch_previous, sizeof(cartridge->rtc_latch_previous)) && ok;
    ok = read_block(file, &cartridge->rtc_last_update, sizeof(cartridge->rtc_last_update)) && ok;

    size_t saved_ram_size = 0;
    ok = read_block(file, &saved_ram_size, sizeof(saved_ram_size)) && ok;
    if (saved_ram_size > 0 && saved_ram_size == cartridge->ram_size && cartridge->ram_data) {
        ok = read_block(file, cartridge->ram_data, saved_ram_size) && ok;
        cartridge->ram_dirty = true;
    }
    else if (saved_ram_size > 0) {
        ok = false;
    }

    fclose(file);
    if (ok) {
        ppu_write_register(ppu, LCDC_ADDRESS, ppu->lcdc);
        ppu_write_register(ppu, STAT_ADDRESS, ppu->stat);
        ppu_set_ly(ppu, ppu->ly);
        FORM_INFO_PRINT("Loaded state slot %u: %s\n", slot, path);
    }
    free(path);
    return ok;
}

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

        if (form && form->joypad && form->joypad->save_flag) {
            save_quick_state(cpu, timer, ppu, apu, 0);
            form->joypad->save_flag = 0;
        }
        if (form && form->joypad && form->joypad->load_flag) {
            load_quick_state(cpu, timer, ppu, apu, 0);
            form->joypad->load_flag = 0;
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
        if ((frame_count % 60) == 0 && cpu && cpu->mmu && cpu->mmu->cartridge) {
            cartridge_flush_battery_save(cpu->mmu->cartridge);
        }
        frame_count += 1;
    }

    if (cpu && cpu->mmu && cpu->mmu->cartridge) {
        cartridge_flush_battery_save(cpu->mmu->cartridge);
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
