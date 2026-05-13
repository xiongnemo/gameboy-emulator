#include "cartridge.h"

static const struct CartridgeFeature cartridge_features[] = {
    {CONTROLLER_ROM_ONLY, "ROM ONLY", CARTRIDGE_MAPPER_ROM_ONLY, false, false, false, false, false, false, false},
    {CONTROLLER_MBC1, "MBC1", CARTRIDGE_MAPPER_MBC1, false, false, false, false, false, false, false},
    {CONTROLLER_MBC1_RAM, "MBC1+RAM", CARTRIDGE_MAPPER_MBC1, true, false, false, false, false, false, false},
    {CONTROLLER_MBC1_RAM_BATTERY, "MBC1+RAM+BATTERY", CARTRIDGE_MAPPER_MBC1, true, true, false, false, false, false, false},
    {CONTROLLER_MBC2, "MBC2", CARTRIDGE_MAPPER_MBC2, true, false, false, false, false, false, false},
    {CONTROLLER_MBC2_BATTERY, "MBC2+BATTERY", CARTRIDGE_MAPPER_MBC2, true, true, false, false, false, false, false},
    {CONTROLLER_ROM_RAM, "ROM+RAM", CARTRIDGE_MAPPER_ROM_ONLY, true, false, false, false, false, false, false},
    {CONTROLLER_ROM_RAM_BATTERY, "ROM+RAM+BATTERY", CARTRIDGE_MAPPER_ROM_ONLY, true, true, false, false, false, false, false},
    {CONTROLLER_MMM01, "MMM01", CARTRIDGE_MAPPER_UNSUPPORTED, false, false, false, false, false, false, true},
    {CONTROLLER_MMM01_RAM, "MMM01+RAM", CARTRIDGE_MAPPER_UNSUPPORTED, true, false, false, false, false, false, true},
    {CONTROLLER_MMM01_RAM_BATTERY, "MMM01+RAM+BATTERY", CARTRIDGE_MAPPER_UNSUPPORTED, true, true, false, false, false, false, true},
    {CONTROLLER_MBC3_TIMER_BATTERY, "MBC3+TIMER+BATTERY", CARTRIDGE_MAPPER_MBC3, false, true, true, false, false, false, false},
    {CONTROLLER_MBC3_TIMER_RAM_BATTERY, "MBC3+TIMER+RAM+BATTERY", CARTRIDGE_MAPPER_MBC3, true, true, true, false, false, false, false},
    {CONTROLLER_MBC3, "MBC3", CARTRIDGE_MAPPER_MBC3, false, false, false, false, false, false, false},
    {CONTROLLER_MBC3_RAM, "MBC3+RAM", CARTRIDGE_MAPPER_MBC3, true, false, false, false, false, false, false},
    {CONTROLLER_MBC3_RAM_BATTERY, "MBC3+RAM+BATTERY", CARTRIDGE_MAPPER_MBC3, true, true, false, false, false, false, false},
    {CONTROLLER_MBC5, "MBC5", CARTRIDGE_MAPPER_MBC5, false, false, false, false, false, false, false},
    {CONTROLLER_MBC5_RAM, "MBC5+RAM", CARTRIDGE_MAPPER_MBC5, true, false, false, false, false, false, false},
    {CONTROLLER_MBC5_RAM_BATTERY, "MBC5+RAM+BATTERY", CARTRIDGE_MAPPER_MBC5, true, true, false, false, false, false, false},
    {CONTROLLER_MBC5_RUMBLE, "MBC5+RUMBLE", CARTRIDGE_MAPPER_MBC5, false, false, false, true, false, false, false},
    {CONTROLLER_MBC5_RUMBLE_RAM, "MBC5+RUMBLE+RAM", CARTRIDGE_MAPPER_MBC5, true, false, false, true, false, false, false},
    {CONTROLLER_MBC5_RUMBLE_RAM_BATTERY, "MBC5+RUMBLE+RAM+BATTERY", CARTRIDGE_MAPPER_MBC5, true, true, false, true, false, false, false},
    {CONTROLLER_MBC6, "MBC6", CARTRIDGE_MAPPER_UNSUPPORTED, true, true, false, false, false, false, true},
    {CONTROLLER_MBC7_SENSOR_RUMBLE_RAM_BATTERY, "MBC7+SENSOR+RUMBLE+RAM+BATTERY", CARTRIDGE_MAPPER_UNSUPPORTED, true, true, false, true, true, false, true},
    {CONTROLLER_POCKET_CAMERA, "POCKET CAMERA", CARTRIDGE_MAPPER_UNSUPPORTED, true, true, false, false, false, true, true},
    {CONTROLLER_BANDAI_TAMA5, "BANDAI TAMA5", CARTRIDGE_MAPPER_UNSUPPORTED, true, true, false, false, false, false, true},
    {CONTROLLER_HUC3, "HuC3", CARTRIDGE_MAPPER_UNSUPPORTED, true, true, true, false, false, false, true},
    {CONTROLLER_HUC1_RAM_BATTERY, "HuC1+RAM+BATTERY", CARTRIDGE_MAPPER_MBC1, true, true, false, false, false, false, false},
};

const struct CartridgeFeature* cartridge_get_feature(uint8_t controller_type)
{
    for (size_t i = 0; i < sizeof(cartridge_features) / sizeof(cartridge_features[0]); i++) {
        if (cartridge_features[i].type == controller_type) {
            return &cartridge_features[i];
        }
    }
    return NULL;
}

static char* cartridge_strdup(const char* value)
{
    if (!value) {
        return NULL;
    }

    size_t length = strlen(value) + 1;
    char* copy   = malloc(length);
    if (copy) {
        memcpy(copy, value, length);
    }
    return copy;
}

static char* cartridge_make_sidecar_path(const char* rom_path, const char* extension)
{
    if (!rom_path || !extension) {
        return NULL;
    }

    const char* last_slash = strrchr(rom_path, '/');
    const char* last_backslash = strrchr(rom_path, '\\');
    const char* last_separator = last_slash;
    if (!last_separator || (last_backslash && last_backslash > last_separator)) {
        last_separator = last_backslash;
    }
    const char* last_dot = strrchr(rom_path, '.');

    size_t base_length = strlen(rom_path);
    if (last_dot && (!last_separator || last_dot > last_separator)) {
        base_length = (size_t)(last_dot - rom_path);
    }

    size_t extension_length = strlen(extension);
    char*  path = malloc(base_length + extension_length + 1);
    if (!path) {
        return NULL;
    }

    memcpy(path, rom_path, base_length);
    memcpy(path + base_length, extension, extension_length + 1);
    return path;
}

static uint64_t cartridge_hash_rom(const uint8_t* data, size_t size)
{
    uint64_t hash = 1469598103934665603ULL;
    for (size_t i = 0; i < size; i++) {
        hash ^= data[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

static bool parse_rom_size(uint8_t code, uint16_t* banks)
{
    switch (code) {
    case 0x00: *banks = 2; return true;
    case 0x01: *banks = 4; return true;
    case 0x02: *banks = 8; return true;
    case 0x03: *banks = 16; return true;
    case 0x04: *banks = 32; return true;
    case 0x05: *banks = 64; return true;
    case 0x06: *banks = 128; return true;
    case 0x07: *banks = 256; return true;
    case 0x08: *banks = 512; return true;
    case 0x52: *banks = 72; return true;
    case 0x53: *banks = 80; return true;
    case 0x54: *banks = 96; return true;
    default: return false;
    }
}

static bool parse_ram_size(uint8_t code, uint8_t* banks, uint8_t* bank_size_kb)
{
    switch (code) {
    case 0x00:
        *banks = 0;
        *bank_size_kb = 0;
        return true;
    case 0x01:
        *banks = 1;
        *bank_size_kb = 2;
        return true;
    case 0x02:
        *banks = 1;
        *bank_size_kb = 8;
        return true;
    case 0x03:
        *banks = 4;
        *bank_size_kb = 8;
        return true;
    case 0x04:
        *banks = 16;
        *bank_size_kb = 8;
        return true;
    case 0x05:
        *banks = 8;
        *bank_size_kb = 8;
        return true;
    default:
        return false;
    }
}

static size_t cartridge_battery_ram_size(struct Cartridge* cartridge)
{
    if (!cartridge->features.has_battery) {
        return 0;
    }
    if (cartridge->features.mapper == CARTRIDGE_MAPPER_MBC2) {
        return sizeof(cartridge->mbc2_ram);
    }
    return cartridge->ram_size;
}

static uint8_t* cartridge_battery_ram_data(struct Cartridge* cartridge)
{
    if (cartridge->features.mapper == CARTRIDGE_MAPPER_MBC2) {
        return cartridge->mbc2_ram;
    }
    return cartridge->ram_data;
}

static void cartridge_mark_ram_dirty(struct Cartridge* cartridge)
{
    if (cartridge->features.has_battery) {
        cartridge->ram_dirty = true;
    }
}

static bool write_file_atomically(const char* path, const uint8_t* data, size_t size)
{
    if (!path || !data) {
        return false;
    }

    size_t tmp_length = strlen(path) + 5;
    char* tmp_path = malloc(tmp_length);
    if (!tmp_path) {
        return false;
    }
    snprintf(tmp_path, tmp_length, "%s.tmp", path);

    FILE* file = fopen(tmp_path, "wb");
    if (!file) {
        free(tmp_path);
        return false;
    }

    bool ok = fwrite(data, 1, size, file) == size;
    ok = fclose(file) == 0 && ok;
    if (ok) {
        remove(path);
        ok = rename(tmp_path, path) == 0;
    }
    if (!ok) {
        remove(tmp_path);
    }

    free(tmp_path);
    return ok;
}

static uint16_t normalize_rom_bank(struct Cartridge* cartridge, uint16_t bank, bool allow_zero)
{
    if (cartridge->rom_attributes_bank_count == 0) {
        return 0;
    }

    bank %= cartridge->rom_attributes_bank_count;
    if (!allow_zero && bank == 0) {
        bank = 1 % cartridge->rom_attributes_bank_count;
        if (bank == 0 && cartridge->rom_attributes_bank_count > 1) {
            bank = 1;
        }
    }
    return bank;
}

static uint16_t mbc1_bank0(struct Cartridge* cartridge)
{
    if (cartridge->mbc1_banking_mode == 0) {
        return 0;
    }
    return normalize_rom_bank(cartridge, (uint16_t)(cartridge->mbc1_bank_high2 << 5), true);
}

static uint16_t mbc1_switchable_bank(struct Cartridge* cartridge)
{
    uint16_t low = cartridge->mbc1_rom_bank_low5 & 0x1F;
    if (low == 0) {
        low = 1;
    }
    return normalize_rom_bank(cartridge, (uint16_t)((cartridge->mbc1_bank_high2 << 5) | low), false);
}

static uint8_t mbc1_ram_bank(struct Cartridge* cartridge)
{
    if (cartridge->mbc1_banking_mode == 0 || cartridge->ram_attributes_bank_count <= 1) {
        return 0;
    }
    return cartridge->mbc1_bank_high2 % cartridge->ram_attributes_bank_count;
}

static uint32_t external_ram_offset(struct Cartridge* cartridge, uint16_t address, uint8_t bank)
{
    if (!cartridge->ram_data || cartridge->ram_size == 0) {
        return UINT32_MAX;
    }

    uint32_t offset = address - 0xA000;
    if (cartridge->ram_attributes_bank_size == 2) {
        offset &= 0x07FF;
        bank = 0;
    }
    else {
        offset &= 0x1FFF;
        if (cartridge->ram_attributes_bank_count > 0) {
            bank %= cartridge->ram_attributes_bank_count;
        }
    }

    uint32_t result = offset + ((uint32_t)bank * 0x2000);
    return result < cartridge->ram_size ? result : UINT32_MAX;
}

static uint8_t read_external_ram(struct Cartridge* cartridge, uint16_t address, uint8_t bank)
{
    if (!cartridge->ram_enabled && cartridge->features.mapper != CARTRIDGE_MAPPER_ROM_ONLY) {
        return 0xFF;
    }

    uint32_t offset = external_ram_offset(cartridge, address, bank);
    if (offset == UINT32_MAX) {
        return 0xFF;
    }
    return cartridge->ram_data[offset];
}

static void write_external_ram(struct Cartridge* cartridge, uint16_t address, uint8_t bank, uint8_t byte)
{
    if (!cartridge->ram_enabled && cartridge->features.mapper != CARTRIDGE_MAPPER_ROM_ONLY) {
        return;
    }

    uint32_t offset = external_ram_offset(cartridge, address, bank);
    if (offset == UINT32_MAX) {
        return;
    }

    cartridge->ram_data[offset] = byte;
    cartridge_mark_ram_dirty(cartridge);
}

static void rtc_update(struct Cartridge* cartridge)
{
    if (!cartridge->features.has_timer || (cartridge->rtc_registers[4] & 0x40)) {
        return;
    }

    time_t now = time(NULL);
    if (cartridge->rtc_last_update == 0) {
        cartridge->rtc_last_update = now;
        return;
    }

    time_t elapsed = now - cartridge->rtc_last_update;
    if (elapsed <= 0) {
        return;
    }

    uint32_t seconds = cartridge->rtc_registers[0];
    seconds += cartridge->rtc_registers[1] * 60U;
    seconds += cartridge->rtc_registers[2] * 3600U;
    seconds += (((uint32_t)cartridge->rtc_registers[3] | ((uint32_t)(cartridge->rtc_registers[4] & 0x01) << 8)) * 86400U);
    seconds += (uint32_t)elapsed;

    uint32_t days = seconds / 86400U;
    if (days > 511) {
        days %= 512;
        cartridge->rtc_registers[4] |= 0x80;
    }

    seconds %= 86400U;
    cartridge->rtc_registers[0] = (uint8_t)(seconds % 60U);
    cartridge->rtc_registers[1] = (uint8_t)((seconds / 60U) % 60U);
    cartridge->rtc_registers[2] = (uint8_t)(seconds / 3600U);
    cartridge->rtc_registers[3] = (uint8_t)(days & 0xFF);
    cartridge->rtc_registers[4] = (uint8_t)((cartridge->rtc_registers[4] & 0xFE) | ((days >> 8) & 0x01));
    cartridge->rtc_last_update = now;
    cartridge_mark_ram_dirty(cartridge);
}

static uint8_t mbc3_read_rtc(struct Cartridge* cartridge)
{
    if (cartridge->rtc_selected_register < 0x08 || cartridge->rtc_selected_register > 0x0C) {
        return 0xFF;
    }

    rtc_update(cartridge);
    uint8_t index = cartridge->rtc_selected_register - 0x08;
    return cartridge->rtc_latched ? cartridge->rtc_latched_registers[index] : cartridge->rtc_registers[index];
}

static void mbc3_write_rtc(struct Cartridge* cartridge, uint8_t value)
{
    if (cartridge->rtc_selected_register < 0x08 || cartridge->rtc_selected_register > 0x0C) {
        return;
    }

    rtc_update(cartridge);
    uint8_t index = cartridge->rtc_selected_register - 0x08;
    switch (index) {
    case 0: cartridge->rtc_registers[index] = value % 60; break;
    case 1: cartridge->rtc_registers[index] = value % 60; break;
    case 2: cartridge->rtc_registers[index] = value % 24; break;
    case 3: cartridge->rtc_registers[index] = value; break;
    case 4: cartridge->rtc_registers[index] = value & 0xC1; break;
    }
    cartridge->rtc_last_update = time(NULL);
    cartridge_mark_ram_dirty(cartridge);
}

bool load_cartridge(struct Cartridge* cartridge, const char* rom_path)
{
    FILE* rom_file = fopen(rom_path, "rb");
    if (rom_file == NULL) {
        CARTRIDGE_ERROR_PRINT("Failed to open ROM file: %s\n", rom_path);
        return false;
    }

    fseek(rom_file, 0, SEEK_END);
    cartridge->rom_size = (size_t)ftell(rom_file);
    fseek(rom_file, 0, SEEK_SET);

    CARTRIDGE_INFO_PRINT("ROM file size: %zu bytes\n", cartridge->rom_size);

    cartridge->rom_data = malloc(cartridge->rom_size);
    if (cartridge->rom_data == NULL) {
        CARTRIDGE_ERROR_PRINT("Failed to allocate memory for ROM\n");
        fclose(rom_file);
        return false;
    }

    size_t bytes_read = fread(cartridge->rom_data, 1, cartridge->rom_size, rom_file);
    fclose(rom_file);
    if (bytes_read != cartridge->rom_size) {
        CARTRIDGE_ERROR_PRINT("Failed to read complete ROM file\n");
        free(cartridge->rom_data);
        cartridge->rom_data = NULL;
        return false;
    }

    cartridge->rom_path = cartridge_strdup(rom_path);
    cartridge->rom_hash = cartridge_hash_rom(cartridge->rom_data, cartridge->rom_size);

    strncpy((char*)cartridge->rom_name, (char*)(cartridge->rom_data + GAMEBOY_ROM_NAME_ADDRESS), ROM_NAME_SIZE);
    cartridge->rom_name[ROM_NAME_SIZE] = '\0';

    CARTRIDGE_INFO_PRINT("ROM loaded successfully. Name: %s\n", cartridge->rom_name);

    return check_cartridge_type(cartridge);
}

void free_cartridge(struct Cartridge* cartridge)
{
    if (cartridge) {
        cartridge_flush_battery_save(cartridge);
        free(cartridge->rom_data);
        cartridge->rom_data = NULL;
        free(cartridge->ram_data);
        cartridge->ram_data = NULL;
        free(cartridge->rom_path);
        free(cartridge->save_path);
        free(cartridge->rtc_path);
        free(cartridge);
    }
}

struct Cartridge* create_cartridge()
{
    struct Cartridge* cartridge = calloc(1, sizeof(struct Cartridge));
    if (cartridge == NULL) {
        CARTRIDGE_ERROR_PRINT("Failed to allocate memory for cartridge\n");
        return NULL;
    }

    cartridge->rom_alternative_bank      = 1;
    cartridge->ram_alternative_bank      = 0;
    cartridge->mbc1_rom_bank_low5        = 1;
    cartridge->mbc5_rom_bank             = 1;
    cartridge->controller_type           = CONTROLLER_ROM_ONLY;
    cartridge->features                  = *cartridge_get_feature(CONTROLLER_ROM_ONLY);
    cartridge->rtc_selected_register     = 0x00;
    cartridge->rtc_last_update           = time(NULL);

    cartridge->check_cartridge_type = check_cartridge_type;
    cartridge->create_cartridge     = create_cartridge;
    cartridge->free_cartridge       = free_cartridge;
    cartridge->get_cartridge_byte   = cartridge_get_cartridge_byte;
    cartridge->set_cartridge_byte   = cartridge_set_cartridge_byte;
    cartridge->get_cartridge_word   = cartridge_get_cartridge_word;
    cartridge->set_cartridge_word   = cartridge_set_cartridge_word;
    cartridge->set_rom_bank         = cartridge_set_rom_bank;
    cartridge->set_ram_bank         = cartridge_set_ram_bank;
    cartridge->get_rom_bank         = cartridge_get_rom_bank;
    cartridge->get_ram_bank         = cartridge_get_ram_bank;
    cartridge->get_rom_name         = cartridge_get_rom_name;
    return cartridge;
}

bool check_cartridge_type(struct Cartridge* cartridge)
{
    if (!cartridge || !cartridge->rom_data || cartridge->rom_size <= GAMEBOY_RAM_SIZE_ADDRESS) {
        CARTRIDGE_ERROR_PRINT("Invalid or incomplete cartridge image\n");
        return false;
    }

    cartridge->controller_type = cartridge->rom_data[GAMEBOY_CARTRIDGE_TYPE_ADDRESS];
    const struct CartridgeFeature* feature = cartridge_get_feature(cartridge->controller_type);
    if (!feature) {
        CARTRIDGE_ERROR_PRINT("Unsupported cartridge type: 0x%02x\n", cartridge->controller_type);
        return false;
    }
    cartridge->features = *feature;

    CARTRIDGE_INFO_PRINT("Cartridge Type: %s (0x%02x)\n", feature->name, feature->type);
    if (feature->unsupported_special_hardware) {
        CARTRIDGE_ERROR_PRINT("Cartridge type 0x%02x (%s) needs special hardware not implemented yet\n",
                              feature->type, feature->name);
        return false;
    }

    if (!parse_rom_size(cartridge->rom_data[GAMEBOY_ROM_SIZE_ADDRESS], &cartridge->rom_attributes_bank_count)) {
        CARTRIDGE_ERROR_PRINT("Invalid ROM Banks Count: 0x%02x\n", cartridge->rom_data[GAMEBOY_ROM_SIZE_ADDRESS]);
        return false;
    }
    CARTRIDGE_DEBUG_PRINT("ROM Banks: 0x%04x\n", cartridge->rom_attributes_bank_count);

    if (!parse_ram_size(cartridge->rom_data[GAMEBOY_RAM_SIZE_ADDRESS],
                        &cartridge->ram_attributes_bank_count,
                        &cartridge->ram_attributes_bank_size)) {
        CARTRIDGE_ERROR_PRINT("Invalid RAM Banks Count: 0x%02x\n", cartridge->rom_data[GAMEBOY_RAM_SIZE_ADDRESS]);
        return false;
    }
    if (cartridge->rom_data[GAMEBOY_RAM_SIZE_ADDRESS] == 0x01) {
        CARTRIDGE_WARN_PRINT("RAM size 0x01 is an unused 2 KiB compatibility mode\n");
    }

    free(cartridge->ram_data);
    cartridge->ram_data = NULL;
    cartridge->ram_size = 0;
    cartridge->ram_enabled = false;
    cartridge->mbc2_ram_enabled = false;
    cartridge->ram_dirty = false;
    cartridge->mbc1_rom_bank_low5 = 1;
    cartridge->mbc1_bank_high2 = 0;
    cartridge->mbc1_banking_mode = 0;
    cartridge->rom_alternative_bank = 1;
    cartridge->ram_alternative_bank = 0;
    cartridge->mbc5_rom_bank = 1;
    cartridge->rtc_selected_register = 0;
    cartridge->rtc_latched = false;
    cartridge->rtc_latch_previous = 0;
    memset(cartridge->mbc2_ram, 0, sizeof(cartridge->mbc2_ram));
    memset(cartridge->rtc_registers, 0, sizeof(cartridge->rtc_registers));
    memset(cartridge->rtc_latched_registers, 0, sizeof(cartridge->rtc_latched_registers));
    cartridge->rtc_last_update = time(NULL);

    if (feature->has_ram && feature->mapper != CARTRIDGE_MAPPER_MBC2 && cartridge->ram_attributes_bank_count > 0) {
        cartridge->ram_size = (size_t)cartridge->ram_attributes_bank_count * cartridge->ram_attributes_bank_size * 1024U;
        cartridge->ram_data = calloc(cartridge->ram_size, 1);
        if (cartridge->ram_data == NULL) {
            CARTRIDGE_ERROR_PRINT("Failed to allocate %zu bytes of cartridge RAM\n", cartridge->ram_size);
            return false;
        }
        CARTRIDGE_DEBUG_PRINT("RAM Banks: %d with size %dKB each\n",
                              cartridge->ram_attributes_bank_count,
                              cartridge->ram_attributes_bank_size);
    }

    if (feature->has_battery && cartridge->rom_path) {
        free(cartridge->save_path);
        free(cartridge->rtc_path);
        cartridge->save_path = cartridge_make_sidecar_path(cartridge->rom_path, ".sav");
        cartridge->rtc_path = cartridge_make_sidecar_path(cartridge->rom_path, ".rtc");
        cartridge_load_battery_save(cartridge);
    }

    return true;
}

bool cartridge_load_battery_save(struct Cartridge* cartridge)
{
    size_t size = cartridge_battery_ram_size(cartridge);
    uint8_t* data = cartridge_battery_ram_data(cartridge);

    if (size > 0 && data && cartridge->save_path) {
        FILE* file = fopen(cartridge->save_path, "rb");
        if (file) {
            fread(data, 1, size, file);
            fclose(file);
            CARTRIDGE_INFO_PRINT("Loaded battery save: %s\n", cartridge->save_path);
        }
    }

    if (cartridge->features.has_timer && cartridge->rtc_path) {
        FILE* file = fopen(cartridge->rtc_path, "rb");
        if (file) {
            char header[8] = {0};
            if (fread(header, 1, sizeof(header), file) == sizeof(header) &&
                memcmp(header, "DGBRTC1", 7) == 0) {
                fread(cartridge->rtc_registers, 1, sizeof(cartridge->rtc_registers), file);
                fread(&cartridge->rtc_last_update, 1, sizeof(cartridge->rtc_last_update), file);
            }
            fclose(file);
        }
    }

    cartridge->ram_dirty = false;
    return true;
}

bool cartridge_flush_battery_save(struct Cartridge* cartridge)
{
    if (!cartridge || !cartridge->features.has_battery) {
        return true;
    }
    if (!cartridge->ram_dirty && !cartridge->features.has_timer) {
        return true;
    }

    bool ok = true;
    size_t size = cartridge_battery_ram_size(cartridge);
    uint8_t* data = cartridge_battery_ram_data(cartridge);

    if (size > 0 && data && cartridge->save_path) {
        ok = write_file_atomically(cartridge->save_path, data, size) && ok;
    }

    if (cartridge->features.has_timer && cartridge->rtc_path) {
        rtc_update(cartridge);
        uint8_t rtc_blob[8 + 5 + sizeof(time_t)] = {0};
        memcpy(rtc_blob, "DGBRTC1", 7);
        memcpy(rtc_blob + 8, cartridge->rtc_registers, sizeof(cartridge->rtc_registers));
        memcpy(rtc_blob + 8 + 5, &cartridge->rtc_last_update, sizeof(time_t));
        ok = write_file_atomically(cartridge->rtc_path, rtc_blob, sizeof(rtc_blob)) && ok;
    }

    if (ok) {
        cartridge->ram_dirty = false;
    }
    return ok;
}

void cartridge_set_rom_bank(struct Cartridge* cartridge, uint8_t bank)
{
    cartridge->rom_alternative_bank = normalize_rom_bank(cartridge, bank, false);
}

uint8_t cartridge_get_rom_bank(struct Cartridge* cartridge)
{
    return cartridge->rom_alternative_bank;
}

void cartridge_set_ram_bank(struct Cartridge* cartridge, uint8_t bank)
{
    cartridge->ram_alternative_bank = cartridge->ram_attributes_bank_count > 0
        ? bank % cartridge->ram_attributes_bank_count
        : 0;
}

uint8_t cartridge_get_ram_bank(struct Cartridge* cartridge)
{
    return cartridge->ram_alternative_bank;
}

static void cartridge_handle_mbc1_write(struct Cartridge* cartridge, uint16_t address, uint8_t byte)
{
    if (address <= 0x1FFF) {
        cartridge->ram_enabled = (byte & 0x0F) == 0x0A;
    }
    else if (address <= 0x3FFF) {
        cartridge->mbc1_rom_bank_low5 = byte & 0x1F;
        if (cartridge->mbc1_rom_bank_low5 == 0) {
            cartridge->mbc1_rom_bank_low5 = 1;
        }
        cartridge->rom_alternative_bank = (uint8_t)mbc1_switchable_bank(cartridge);
    }
    else if (address <= 0x5FFF) {
        cartridge->mbc1_bank_high2 = byte & 0x03;
        cartridge->rom_alternative_bank = (uint8_t)mbc1_switchable_bank(cartridge);
        cartridge->ram_alternative_bank = mbc1_ram_bank(cartridge);
    }
    else if (address <= 0x7FFF) {
        cartridge->mbc1_banking_mode = byte & 0x01;
        cartridge->ram_alternative_bank = mbc1_ram_bank(cartridge);
    }
    else if (address >= 0xA000 && address <= 0xBFFF) {
        write_external_ram(cartridge, address, mbc1_ram_bank(cartridge), byte);
    }
}

static void cartridge_handle_mbc2_write(struct Cartridge* cartridge, uint16_t address, uint8_t byte)
{
    if (address <= 0x3FFF) {
        if ((address & 0x0100) == 0) {
            cartridge->mbc2_ram_enabled = (byte & 0x0F) == 0x0A;
        }
        else {
            uint8_t bank = byte & 0x0F;
            if (bank == 0) {
                bank = 1;
            }
            cartridge->rom_alternative_bank = normalize_rom_bank(cartridge, bank, false);
        }
    }
    else if (address >= 0xA000 && address <= 0xBFFF && cartridge->mbc2_ram_enabled) {
        cartridge->mbc2_ram[address & 0x01FF] = byte & 0x0F;
        cartridge_mark_ram_dirty(cartridge);
    }
}

static void cartridge_handle_mbc3_write(struct Cartridge* cartridge, uint16_t address, uint8_t byte)
{
    if (address <= 0x1FFF) {
        cartridge->ram_enabled = (byte & 0x0F) == 0x0A;
    }
    else if (address <= 0x3FFF) {
        uint8_t bank = byte & 0x7F;
        if (bank == 0) {
            bank = 1;
        }
        cartridge->rom_alternative_bank = normalize_rom_bank(cartridge, bank, false);
    }
    else if (address <= 0x5FFF) {
        if (byte <= 0x03) {
            cartridge->rtc_selected_register = byte;
            cartridge->ram_alternative_bank = cartridge->ram_attributes_bank_count > 0
                ? byte % cartridge->ram_attributes_bank_count
                : 0;
        }
        else if (byte >= 0x08 && byte <= 0x0C) {
            cartridge->rtc_selected_register = byte;
        }
    }
    else if (address <= 0x7FFF) {
        if (cartridge->rtc_latch_previous == 0 && byte == 1) {
            rtc_update(cartridge);
            memcpy(cartridge->rtc_latched_registers,
                   cartridge->rtc_registers,
                   sizeof(cartridge->rtc_latched_registers));
            cartridge->rtc_latched = true;
        }
        cartridge->rtc_latch_previous = byte;
    }
    else if (address >= 0xA000 && address <= 0xBFFF) {
        if (cartridge->rtc_selected_register >= 0x08 && cartridge->rtc_selected_register <= 0x0C) {
            if (cartridge->ram_enabled) {
                mbc3_write_rtc(cartridge, byte);
            }
        }
        else {
            write_external_ram(cartridge, address, cartridge->ram_alternative_bank, byte);
        }
    }
}

static void cartridge_handle_mbc5_write(struct Cartridge* cartridge, uint16_t address, uint8_t byte)
{
    if (address <= 0x1FFF) {
        cartridge->ram_enabled = (byte & 0x0F) == 0x0A;
    }
    else if (address <= 0x2FFF) {
        cartridge->mbc5_rom_bank = (cartridge->mbc5_rom_bank & 0x0100) | byte;
    }
    else if (address <= 0x3FFF) {
        cartridge->mbc5_rom_bank = (cartridge->mbc5_rom_bank & 0x00FF) | ((uint16_t)(byte & 0x01) << 8);
    }
    else if (address <= 0x5FFF) {
        if (cartridge->features.has_rumble) {
            cartridge->rumble_motor_on = (byte & 0x08) != 0;
            cartridge->ram_alternative_bank = byte & 0x07;
        }
        else {
            cartridge->ram_alternative_bank = byte & 0x0F;
        }
    }
    else if (address >= 0xA000 && address <= 0xBFFF) {
        write_external_ram(cartridge, address, cartridge->ram_alternative_bank, byte);
    }
}

uint8_t cartridge_get_cartridge_byte(struct Cartridge* cartridge, uint16_t address)
{
    if (address <= 0x3FFF) {
        uint16_t bank = 0;
        if (cartridge->features.mapper == CARTRIDGE_MAPPER_MBC1) {
            bank = mbc1_bank0(cartridge);
        }
        uint32_t bank_address = (uint32_t)bank * GAMEBOY_BANK_SIZE + address;
        return bank_address < cartridge->rom_size ? cartridge->rom_data[bank_address] : 0xFF;
    }

    if (address >= 0x4000 && address <= 0x7FFF) {
        uint16_t rom_bank = 1;
        switch (cartridge->features.mapper) {
        case CARTRIDGE_MAPPER_ROM_ONLY:
            rom_bank = cartridge->rom_attributes_bank_count > 2 ? 1 : 0;
            break;
        case CARTRIDGE_MAPPER_MBC1:
            rom_bank = mbc1_switchable_bank(cartridge);
            break;
        case CARTRIDGE_MAPPER_MBC2:
        case CARTRIDGE_MAPPER_MBC3:
            rom_bank = cartridge->rom_alternative_bank;
            break;
        case CARTRIDGE_MAPPER_MBC5:
            rom_bank = normalize_rom_bank(cartridge, cartridge->mbc5_rom_bank, true);
            break;
        default:
            return 0xFF;
        }

        uint32_t bank_address;
        if (cartridge->features.mapper == CARTRIDGE_MAPPER_ROM_ONLY && cartridge->rom_attributes_bank_count <= 2) {
            bank_address = address;
        }
        else {
            bank_address = (address - 0x4000) + ((uint32_t)rom_bank * GAMEBOY_BANK_SIZE);
        }
        return bank_address < cartridge->rom_size ? cartridge->rom_data[bank_address] : 0xFF;
    }

    if (address >= 0xA000 && address <= 0xBFFF) {
        switch (cartridge->features.mapper) {
        case CARTRIDGE_MAPPER_MBC2:
            return cartridge->mbc2_ram_enabled ? (cartridge->mbc2_ram[address & 0x01FF] | 0xF0) : 0xFF;
        case CARTRIDGE_MAPPER_MBC1:
            return read_external_ram(cartridge, address, mbc1_ram_bank(cartridge));
        case CARTRIDGE_MAPPER_MBC3:
            if (cartridge->rtc_selected_register >= 0x08 && cartridge->rtc_selected_register <= 0x0C) {
                return cartridge->ram_enabled ? mbc3_read_rtc(cartridge) : 0xFF;
            }
            return read_external_ram(cartridge, address, cartridge->ram_alternative_bank);
        case CARTRIDGE_MAPPER_MBC5:
            return read_external_ram(cartridge, address, cartridge->ram_alternative_bank);
        case CARTRIDGE_MAPPER_ROM_ONLY:
            return read_external_ram(cartridge, address, 0);
        default:
            return 0xFF;
        }
    }

    return 0xFF;
}

void cartridge_set_cartridge_byte(struct Cartridge* cartridge, uint16_t address, uint8_t byte)
{
    switch (cartridge->features.mapper) {
    case CARTRIDGE_MAPPER_ROM_ONLY:
        if (address >= 0xA000 && address <= 0xBFFF) {
            write_external_ram(cartridge, address, 0, byte);
        }
        break;
    case CARTRIDGE_MAPPER_MBC1:
        cartridge_handle_mbc1_write(cartridge, address, byte);
        break;
    case CARTRIDGE_MAPPER_MBC2:
        cartridge_handle_mbc2_write(cartridge, address, byte);
        break;
    case CARTRIDGE_MAPPER_MBC3:
        cartridge_handle_mbc3_write(cartridge, address, byte);
        break;
    case CARTRIDGE_MAPPER_MBC5:
        cartridge_handle_mbc5_write(cartridge, address, byte);
        break;
    default:
        break;
    }
}

uint16_t cartridge_get_cartridge_word(struct Cartridge* cartridge, uint16_t address)
{
    uint8_t low = cartridge_get_cartridge_byte(cartridge, address);
    uint8_t high = cartridge_get_cartridge_byte(cartridge, (uint16_t)(address + 1));
    return (uint16_t)(low | (high << 8));
}

void cartridge_set_cartridge_word(struct Cartridge* cartridge, uint16_t address, uint16_t word)
{
    cartridge_set_cartridge_byte(cartridge, address, (uint8_t)(word & 0xFF));
    cartridge_set_cartridge_byte(cartridge, (uint16_t)(address + 1), (uint8_t)(word >> 8));
}

char* cartridge_get_rom_name(struct Cartridge* cartridge)
{
    return (char*)cartridge->rom_name;
}
