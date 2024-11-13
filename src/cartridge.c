#include "cartridge.h"

bool load_cartridge(struct Cartridge *cartridge, const char *rom_path)
{
    FILE *file = fopen(rom_path, "rb");
    if (!file)
    {
        CARTRIDGE_EMERGENCY_PRINT("Failed to open ROM file: %s\n", rom_path);
        return false;
    }
    // load rom into cartridge
    fread(cartridge->rom, 1, ROM_SIZE, file);
    fclose(file);

    // get ROM name
    memcpy(cartridge->rom_name, cartridge->rom + GAMEBOY_ROM_NAME_ADDRESS, 16);
    cartridge->rom_name[16] = '\0';
    CARTRIDGE_DEBUG_PRINT("ROM Name: %s\n", cartridge->rom_name);

    // check cartridge type
    return check_cartridge_type(cartridge);
}

void free_cartridge(struct Cartridge *cartridge)
{
    // free(cartridge->rom_bank_0);
    // free(cartridge->rom_bank_1);
    // free(cartridge->ram);
    if (cartridge)
    {
        free(cartridge);
    }
}

struct Cartridge *create_cartridge()
{
    struct Cartridge *cartridge = malloc(sizeof(struct Cartridge));
    cartridge->rom_alternative_bank = 1;
    cartridge->ram_alternative_bank = 0;
    cartridge->rom_attributes_bank_count = 0;
    cartridge->ram_attributes_bank_count = 0;
    cartridge->ram_attributes_bank_size = 0; // in kb

    // set method pointers
    cartridge->check_cartridge_type = check_cartridge_type;
    cartridge->create_cartridge = create_cartridge;
    cartridge->free_cartridge = free_cartridge;
    return cartridge;
}

bool check_cartridge_type(struct Cartridge *cartridge)
{
    // check ram controller type (0x0147 in ROM is 01h)

    bool using_MBC1 = false;
    bool using_MBC1_RAM = false;
    bool using_ROM_only = false;
    bool not_supported_cartridge_mode = false;

    switch (cartridge->rom[GAMEBOY_CARTRIDGE_TYPE_ADDRESS])
    {
    case 0x01:
    {
        // set bool
        using_MBC1 = true;
        not_supported_cartridge_mode = false;
        CARTRIDGE_DEBUG_PRINT("Cartridge Type: ROM + MBC1 (0x%02x)\n", cartridge->rom[GAMEBOY_CARTRIDGE_TYPE_ADDRESS]);
        break;
    }
    case 0x02:
    {
        // set bool
        using_MBC1_RAM = true;
        not_supported_cartridge_mode = false;
        CARTRIDGE_DEBUG_PRINT("Cartridge Type: ROM + MBC1 + RAM (0x%02x)\n", cartridge->rom[GAMEBOY_CARTRIDGE_TYPE_ADDRESS]);
        // printf("RAM switching restricted, game behaviour maybe abnormal!\n");
        break;
    }
    case 0x03:
    {
        // set bool
        using_MBC1_RAM = true;
        not_supported_cartridge_mode = false;
        CARTRIDGE_DEBUG_PRINT("Cartridge Type: ROM + MBC1 + RAM + BATTERY (0x%02x)\n", cartridge->rom[GAMEBOY_CARTRIDGE_TYPE_ADDRESS]);
        // printf("RAM switching restricted, game behaviour maybe abnormal!\n");
        break;
    }
    case 0x00:
    {
        // set bool
        using_ROM_only = true;
        not_supported_cartridge_mode = false;
        CARTRIDGE_DEBUG_PRINT("Cartridge Type: ROM only (0x%02x)\n", cartridge->rom[GAMEBOY_CARTRIDGE_TYPE_ADDRESS]);
        break;
    }
    default:
    {
        not_supported_cartridge_mode = true;
        CARTRIDGE_DEBUG_PRINT("Unsupported Cartridge Type: (0x%02x)\n", cartridge->rom[GAMEBOY_CARTRIDGE_TYPE_ADDRESS]);
        return false;
    }
    }

    // check ROM Size
    switch (cartridge->rom[GAMEBOY_ROM_SIZE_ADDRESS])
    {
    case 0x01:
    {
        cartridge->rom_attributes_bank_count = 4;
        break;
    }
    case 0x02:
    {
        cartridge->rom_attributes_bank_count = 8;
        break;
    }
    case 0x03:
    {
        cartridge->rom_attributes_bank_count = 16;
        break;
    }
    case 0x04:
    {
        cartridge->rom_attributes_bank_count = 32;
        break;
    }
    case 0x00:
    {
        cartridge->rom_attributes_bank_count = 2;
        break;
    }
    default:
    {
        CARTRIDGE_DEBUG_PRINT("Invalid ROM Banks Count: 0x%02x\n", cartridge->rom[GAMEBOY_ROM_SIZE_ADDRESS]);
        return false;
    }
    }
    CARTRIDGE_DEBUG_PRINT("ROM Banks: 0x%02x\n", cartridge->rom_attributes_bank_count);

    // check RAM Size
    switch (cartridge->rom[GAMEBOY_RAM_SIZE_ADDRESS])
    {
    case 0x01:
    {
        cartridge->ram_attributes_bank_count = 1;
        cartridge->ram_attributes_bank_size = 2;
        break;
    }
    case 0x02:
    {
        cartridge->ram_attributes_bank_count = 1;
        cartridge->ram_attributes_bank_size = 8;
        break;
    }
    case 0x03:
    {
        cartridge->ram_attributes_bank_count = 2;
        cartridge->ram_attributes_bank_size = 8;
        break;
    }
    case 0x04:
    {
        cartridge->ram_attributes_bank_count = 4;
        cartridge->ram_attributes_bank_size = 8;
        break;
    }
    case 0x00:
    {
        // set bool
        cartridge->ram_attributes_bank_count = 0;
        break;
    }
    default:
    {
        CARTRIDGE_DEBUG_PRINT("Invalid RAM Banks Count: 0x%02x\n", cartridge->rom[GAMEBOY_RAM_SIZE_ADDRESS]);
        return false;
    }
    }
    if (cartridge->ram_attributes_bank_count > 0)
    {
        CARTRIDGE_DEBUG_PRINT("RAM Banks: 0x%02x with size 0x%02x * 0x%02xkb\n", cartridge->ram_attributes_bank_count, cartridge->ram_attributes_bank_count, cartridge->ram_attributes_bank_size);
    }
    else
    {
        CARTRIDGE_DEBUG_PRINT("No RAM banking: 0x%02x\n", cartridge->rom[GAMEBOY_RAM_SIZE_ADDRESS]);
    }

    return true;
}

void cartridge_set_rom_bank(struct Cartridge *cartridge, uint8_t bank)
{
    cartridge->rom_alternative_bank = bank;
}

uint8_t cartridge_get_rom_bank(struct Cartridge *cartridge)
{
    return cartridge->rom_alternative_bank;
}

void cartridge_set_ram_bank(struct Cartridge *cartridge, uint8_t bank)
{
    cartridge->ram_alternative_bank = bank;
}

uint8_t cartridge_get_ram_bank(struct Cartridge *cartridge)
{
    return cartridge->ram_alternative_bank;
}

uint8_t cartridge_get_rom_byte(struct Cartridge *cartridge, uint16_t address)
{
    // 0-0x3FFF is always ROM bank 0
    // 0x4000-0x7FFF is alternative ROM bank
    if (address <= 0x3FFF)
    {
        return cartridge->rom[address];
    }
    else if (address >= 0x4000 && address <= 0x7FFF)
    {
        return cartridge->rom[address + (cartridge->rom_alternative_bank * 0x4000)];
    }
    else
    {
        CARTRIDGE_DEBUG_PRINT("Trying to access invalid ROM address: 0x%04x\n", address);
        return 0;
    }
}

void cartridge_set_rom_byte(struct Cartridge *cartridge, uint16_t address, uint8_t byte)
{
    // cartridge->rom[address] = byte;
}

uint16_t cartridge_get_rom_word(struct Cartridge *cartridge, uint16_t address)
{
    if (address <= 0x3FFF)
    {
        return cartridge->rom[address] | (cartridge->rom[address + 1] << 8);
    }
    else if (address >= 0x4000 && address <= 0x7FFF)
    {
        return cartridge->rom[address + (cartridge->rom_alternative_bank * 0x4000)] | (cartridge->rom[address + (cartridge->rom_alternative_bank * 0x4000) + 1] << 8);
    }
    else
    {
        CARTRIDGE_DEBUG_PRINT("Trying to access invalid ROM address: 0x%04x\n", address);
        return 0;
    }
}

void cartridge_set_rom_word(struct Cartridge *cartridge, uint16_t address, uint16_t word)
{
    // cartridge->rom[address] = word & 0xFF;
    // cartridge->rom[address + 1] = (word >> 8) & 0xFF;
}

char* cartridge_get_rom_name(struct Cartridge *cartridge)
{
    return (char *)cartridge->rom_name;
}

