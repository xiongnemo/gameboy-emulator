#include "test.h"
#include "../src/cartridge.h"

static uint8_t rom_size_code_for_banks(uint16_t banks)
{
    switch (banks) {
    case 2: return 0x00;
    case 4: return 0x01;
    case 8: return 0x02;
    case 16: return 0x03;
    case 32: return 0x04;
    case 64: return 0x05;
    case 128: return 0x06;
    case 256: return 0x07;
    case 512: return 0x08;
    case 72: return 0x52;
    case 80: return 0x53;
    case 96: return 0x54;
    default: return 0x00;
    }
}

static size_t ram_size_for_code(uint8_t code)
{
    switch (code) {
    case 0x01: return 2 * 1024;
    case 0x02: return 8 * 1024;
    case 0x03: return 32 * 1024;
    case 0x04: return 128 * 1024;
    case 0x05: return 64 * 1024;
    default: return 0;
    }
}

static struct Cartridge* create_test_cartridge(uint8_t controller_type, uint16_t rom_banks, uint8_t ram_size_code)
{
    struct Cartridge* cart = create_cartridge();
    assert(cart != NULL);

    cart->rom_size = (size_t)rom_banks * GAMEBOY_BANK_SIZE;
    cart->rom_data = malloc(cart->rom_size);
    assert(cart->rom_data != NULL);
    memset(cart->rom_data, 0, cart->rom_size);

    for (uint16_t bank = 0; bank < rom_banks; bank++) {
        memset(cart->rom_data + ((size_t)bank * GAMEBOY_BANK_SIZE), bank & 0xFF, GAMEBOY_BANK_SIZE);
    }

    cart->rom_data[GAMEBOY_CARTRIDGE_TYPE_ADDRESS] = controller_type;
    cart->rom_data[GAMEBOY_ROM_SIZE_ADDRESS] = rom_size_code_for_banks(rom_banks);
    cart->rom_data[GAMEBOY_RAM_SIZE_ADDRESS] = ram_size_code;

    assert(check_cartridge_type(cart));
    return cart;
}

static void write_rom_file(const char* path, uint8_t controller_type, uint16_t rom_banks, uint8_t ram_size_code)
{
    size_t size = (size_t)rom_banks * GAMEBOY_BANK_SIZE;
    uint8_t* data = calloc(size, 1);
    assert(data != NULL);
    memcpy(data + GAMEBOY_ROM_NAME_ADDRESS, "BATTERYTEST", 11);
    data[GAMEBOY_CARTRIDGE_TYPE_ADDRESS] = controller_type;
    data[GAMEBOY_ROM_SIZE_ADDRESS] = rom_size_code_for_banks(rom_banks);
    data[GAMEBOY_RAM_SIZE_ADDRESS] = ram_size_code;

    FILE* file = fopen(path, "wb");
    assert(file != NULL);
    assert(fwrite(data, 1, size, file) == size);
    fclose(file);
    free(data);
}

static void test_header_tables_cover_known_types(void)
{
    const uint8_t all_types[] = {
        0x00, 0x01, 0x02, 0x03, 0x05, 0x06, 0x08, 0x09,
        0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11, 0x12, 0x13,
        0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x20, 0x22,
        0xFC, 0xFD, 0xFE, 0xFF,
    };

    for (size_t i = 0; i < sizeof(all_types); i++) {
        const struct CartridgeFeature* feature = cartridge_get_feature(all_types[i]);
        assert(feature != NULL);
    }
}

static void test_rom_and_ram_size_codes(void)
{
    const struct {
        uint8_t code;
        uint16_t banks;
    } rom_cases[] = {
        {0x00, 2}, {0x01, 4}, {0x02, 8}, {0x03, 16}, {0x04, 32}, {0x05, 64},
        {0x06, 128}, {0x07, 256}, {0x08, 512}, {0x52, 72}, {0x53, 80}, {0x54, 96},
    };

    for (size_t i = 0; i < sizeof(rom_cases) / sizeof(rom_cases[0]); i++) {
        struct Cartridge* cart = create_cartridge();
        cart->rom_size = 0x8000;
        cart->rom_data = calloc(cart->rom_size, 1);
        assert(cart->rom_data != NULL);
        cart->rom_data[GAMEBOY_CARTRIDGE_TYPE_ADDRESS] = CONTROLLER_ROM_ONLY;
        cart->rom_data[GAMEBOY_ROM_SIZE_ADDRESS] = rom_cases[i].code;
        cart->rom_data[GAMEBOY_RAM_SIZE_ADDRESS] = 0x00;
        assert(check_cartridge_type(cart));
        assert(cart->rom_attributes_bank_count == rom_cases[i].banks);
        free_cartridge(cart);
    }

    const struct {
        uint8_t code;
        size_t bytes;
    } ram_cases[] = {
        {0x00, 0}, {0x01, 2 * 1024}, {0x02, 8 * 1024}, {0x03, 32 * 1024},
        {0x04, 128 * 1024}, {0x05, 64 * 1024},
    };

    for (size_t i = 0; i < sizeof(ram_cases) / sizeof(ram_cases[0]); i++) {
        assert(ram_size_for_code(ram_cases[i].code) == ram_cases[i].bytes);
        struct Cartridge* cart = create_test_cartridge(CONTROLLER_MBC1_RAM, 4, ram_cases[i].code);
        assert(cart->ram_size == ram_cases[i].bytes);
        free_cartridge(cart);
    }
}

static void test_rom_only_direct_mapping(void)
{
    struct Cartridge* cart = create_test_cartridge(CONTROLLER_ROM_ONLY, 2, 0x00);
    cart->rom_data[0x4000] = 0x42;
    assert(cart->get_cartridge_byte(cart, 0x4000) == 0x42);
    cart->set_cartridge_byte(cart, 0x2000, 0x01);
    assert(cart->get_cartridge_byte(cart, 0x4000) == 0x42);
    free_cartridge(cart);
}

static void test_marioland_mbc1_four_bank_mapping(void)
{
    struct Cartridge* cart = create_test_cartridge(CONTROLLER_MBC1, 4, 0x00);

    assert(cart->get_cartridge_byte(cart, 0x4000) == 0x01);
    cart->set_cartridge_byte(cart, 0x2000, 0x02);
    assert(cart->get_cartridge_byte(cart, 0x4000) == 0x02);
    cart->set_cartridge_byte(cart, 0x2000, 0x00);
    assert(cart->get_cartridge_byte(cart, 0x4000) == 0x01);
    cart->set_cartridge_byte(cart, 0x4000, 0x03);
    assert(cart->get_cartridge_byte(cart, 0x4000) == 0x01);

    free_cartridge(cart);
}

static void test_mbc1_high_bits_mode_and_ram_banking(void)
{
    struct Cartridge* cart = create_test_cartridge(CONTROLLER_MBC1_RAM_BATTERY, 64, 0x03);

    cart->set_cartridge_byte(cart, 0x2000, 0x02);
    cart->set_cartridge_byte(cart, 0x4000, 0x01);
    assert(cart->get_cartridge_byte(cart, 0x4000) == 0x22);

    cart->set_cartridge_byte(cart, 0x6000, 0x01);
    assert(cart->get_cartridge_byte(cart, 0x0000) == 0x20);

    cart->set_cartridge_byte(cart, 0x0000, 0x0A);
    cart->set_cartridge_byte(cart, 0x4000, 0x02);
    cart->set_cartridge_byte(cart, 0xA000, 0x5A);
    cart->set_cartridge_byte(cart, 0x4000, 0x01);
    cart->set_cartridge_byte(cart, 0xA000, 0x33);
    cart->set_cartridge_byte(cart, 0x4000, 0x02);
    assert(cart->get_cartridge_byte(cart, 0xA000) == 0x5A);

    free_cartridge(cart);
}

static void test_mbc2_internal_ram_and_banking(void)
{
    struct Cartridge* cart = create_test_cartridge(CONTROLLER_MBC2_BATTERY, 16, 0x00);

    cart->set_cartridge_byte(cart, 0x2100, 0x0F);
    assert(cart->get_cartridge_byte(cart, 0x4000) == 0x0F);
    cart->set_cartridge_byte(cart, 0x0000, 0x0A);
    cart->set_cartridge_byte(cart, 0xA1FF, 0xAB);
    assert(cart->get_cartridge_byte(cart, 0xA1FF) == 0xFB);
    assert(cart->get_cartridge_byte(cart, 0xA3FF) == 0xFB);

    free_cartridge(cart);
}

static void test_mbc3_ram_and_rtc_basics(void)
{
    struct Cartridge* cart = create_test_cartridge(CONTROLLER_MBC3_TIMER_RAM_BATTERY, 16, 0x03);

    cart->set_cartridge_byte(cart, 0x0000, 0x0A);
    cart->set_cartridge_byte(cart, 0x4000, 0x02);
    cart->set_cartridge_byte(cart, 0xA000, 0x6C);
    assert(cart->get_cartridge_byte(cart, 0xA000) == 0x6C);

    cart->set_cartridge_byte(cart, 0x4000, 0x08);
    cart->set_cartridge_byte(cart, 0xA000, 45);
    cart->set_cartridge_byte(cart, 0x6000, 0);
    cart->set_cartridge_byte(cart, 0x6000, 1);
    assert(cart->get_cartridge_byte(cart, 0xA000) == 45);

    free_cartridge(cart);
}

static void test_mbc5_9bit_rom_and_rumble_ram(void)
{
    struct Cartridge* cart = create_test_cartridge(CONTROLLER_MBC5_RUMBLE_RAM_BATTERY, 512, 0x05);

    cart->set_cartridge_byte(cart, 0x2000, 0xFF);
    cart->set_cartridge_byte(cart, 0x3000, 0x01);
    assert(cart->get_cartridge_byte(cart, 0x4000) == 0xFF);

    cart->set_cartridge_byte(cart, 0x2000, 0x00);
    cart->set_cartridge_byte(cart, 0x3000, 0x00);
    assert(cart->get_cartridge_byte(cart, 0x4000) == 0x00);

    cart->set_cartridge_byte(cart, 0x0000, 0x0A);
    cart->set_cartridge_byte(cart, 0x4000, 0x0D);
    assert(cart->rumble_motor_on == true);
    assert(cart->ram_alternative_bank == 5);
    cart->set_cartridge_byte(cart, 0xA000, 0x91);
    assert(cart->get_cartridge_byte(cart, 0xA000) == 0x91);

    free_cartridge(cart);
}

static void test_battery_sav_round_trip(void)
{
    const char* rom_path = "build_tmp/battery-test.gb";
    const char* sav_path = "build_tmp/battery-test.sav";
    remove(rom_path);
    remove(sav_path);

    write_rom_file(rom_path, CONTROLLER_MBC1_RAM_BATTERY, 4, 0x02);

    struct Cartridge* cart = create_cartridge();
    assert(load_cartridge(cart, rom_path));
    cart->set_cartridge_byte(cart, 0x0000, 0x0A);
    cart->set_cartridge_byte(cart, 0xA000, 0x42);
    assert(cartridge_flush_battery_save(cart));
    free_cartridge(cart);

    FILE* save_file = fopen(sav_path, "rb");
    assert(save_file != NULL);
    fseek(save_file, 0, SEEK_END);
    assert(ftell(save_file) == 8 * 1024);
    fclose(save_file);

    cart = create_cartridge();
    assert(load_cartridge(cart, rom_path));
    cart->set_cartridge_byte(cart, 0x0000, 0x0A);
    assert(cart->get_cartridge_byte(cart, 0xA000) == 0x42);
    free_cartridge(cart);

    remove(rom_path);
    remove(sav_path);
}

int main(void)
{
    config.start_time = get_time_in_seconds();

    test_header_tables_cover_known_types();
    test_rom_and_ram_size_codes();
    test_rom_only_direct_mapping();
    test_marioland_mbc1_four_bank_mapping();
    test_mbc1_high_bits_mode_and_ram_banking();
    test_mbc2_internal_ram_and_banking();
    test_mbc3_ram_and_rtc_basics();
    test_mbc5_9bit_rom_and_rumble_ram();
    test_battery_sav_round_trip();

    printf("All MBC cartridge tests passed successfully!\n");
    return 0;
}
