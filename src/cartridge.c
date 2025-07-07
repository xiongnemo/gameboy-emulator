#include "cartridge.h"

bool load_cartridge(struct Cartridge* cartridge, const char* rom_path)
{
    FILE* rom_file = fopen(rom_path, "rb");
    if (rom_file == NULL) {
        CARTRIDGE_ERROR_PRINT("Failed to open ROM file: %s\n", rom_path);
        return false;
    }

    // Get file size
    fseek(rom_file, 0, SEEK_END);
    cartridge->rom_size = ftell(rom_file);
    fseek(rom_file, 0, SEEK_SET);

    CARTRIDGE_INFO_PRINT("ROM file size: %zu bytes\n", cartridge->rom_size);

    // Allocate memory for ROM
    cartridge->rom_data = malloc(cartridge->rom_size);
    if (cartridge->rom_data == NULL) {
        CARTRIDGE_ERROR_PRINT("Failed to allocate memory for ROM\n");
        fclose(rom_file);
        return false;
    }

    // Read ROM data
    size_t bytes_read = fread(cartridge->rom_data, 1, cartridge->rom_size, rom_file);
    if (bytes_read != cartridge->rom_size) {
        CARTRIDGE_ERROR_PRINT("Failed to read complete ROM file\n");
        free(cartridge->rom_data);
        cartridge->rom_data = NULL;
        fclose(rom_file);
        return false;
    }

    fclose(rom_file);

    // Extract ROM name from header
    strncpy((char*)cartridge->rom_name, (char*)(cartridge->rom_data + 0x134), 16);
    cartridge->rom_name[16] = '\0';  // Ensure null termination

    CARTRIDGE_INFO_PRINT("ROM loaded successfully. Name: %s\n", cartridge->rom_name);

    // check cartridge type
    return check_cartridge_type(cartridge);
}

void free_cartridge(struct Cartridge* cartridge)
{
    if (cartridge) {
        // Free dynamically allocated ROM data
        if (cartridge->rom_data) {
            free(cartridge->rom_data);
            cartridge->rom_data = NULL;
        }
        free(cartridge);
    }
}

struct Cartridge* create_cartridge()
{
    struct Cartridge* cartridge = malloc(sizeof(struct Cartridge));
    if (cartridge == NULL) {
        CARTRIDGE_ERROR_PRINT("Failed to allocate memory for cartridge\n");
        return NULL;
    }
    
    cartridge->rom_alternative_bank      = 1;  // MBC1 starts at bank 1, not 0
    cartridge->ram_alternative_bank      = 0;
    cartridge->rom_attributes_bank_count = 0;
    cartridge->ram_attributes_bank_count = 0;
    cartridge->ram_attributes_bank_size  = 0;   // in kb

    // Initialize dynamic ROM fields
    cartridge->rom_data = NULL;
    cartridge->rom_size = 0;

    // Initialize dynamic RAM fields
    cartridge->ram_data = NULL;
    cartridge->ram_size = 0;

    // set method pointers
    cartridge->check_cartridge_type = check_cartridge_type;
    cartridge->create_cartridge     = create_cartridge;
    cartridge->free_cartridge       = free_cartridge;
    cartridge->get_cartridge_byte  = cartridge_get_cartridge_byte;
    cartridge->set_cartridge_byte  = cartridge_set_cartridge_byte;
    cartridge->get_cartridge_word  = cartridge_get_cartridge_word;
    cartridge->set_cartridge_word  = cartridge_set_cartridge_word;
    cartridge->set_rom_bank        = cartridge_set_rom_bank;
    cartridge->set_ram_bank        = cartridge_set_ram_bank;
    cartridge->get_rom_bank        = cartridge_get_rom_bank;
    cartridge->get_ram_bank        = cartridge_get_ram_bank;
    cartridge->get_rom_name        = cartridge_get_rom_name;
    return cartridge;
}

bool check_cartridge_type(struct Cartridge* cartridge)
{
    // check ram controller type (0x0147 in ROM is 01h)

    bool using_MBC1                   = false;
    bool using_MBC1_RAM               = false;
    bool using_ROM_only               = false;
    bool not_supported_cartridge_mode = false;

    switch (cartridge->rom_data[GAMEBOY_CARTRIDGE_TYPE_ADDRESS]) {
    case 0x01:
    {
        // set bool
        using_MBC1                   = true;
        not_supported_cartridge_mode = false;
        CARTRIDGE_DEBUG_PRINT(
            "Cartridge Type: ROM + MBC1 (0x%02x)\n",
            cartridge->rom_data[GAMEBOY_CARTRIDGE_TYPE_ADDRESS]);
        break;
    }
    case 0x02:
    {
        // set bool
        using_MBC1_RAM               = true;
        not_supported_cartridge_mode = false;
        CARTRIDGE_DEBUG_PRINT(
            "Cartridge Type: ROM + MBC1 + RAM (0x%02x)\n",
            cartridge->rom_data[GAMEBOY_CARTRIDGE_TYPE_ADDRESS]);
        break;
    }
    case 0x03:
    {
        // set bool
        using_MBC1_RAM               = true;
        not_supported_cartridge_mode = false;
        CARTRIDGE_DEBUG_PRINT(
            "Cartridge Type: ROM + MBC1 + RAM + BATTERY (0x%02x)\n",
            cartridge->rom_data[GAMEBOY_CARTRIDGE_TYPE_ADDRESS]);
        break;
    }
    case 0x00:
    {
        // set bool
        using_ROM_only               = true;
        not_supported_cartridge_mode = false;
        CARTRIDGE_DEBUG_PRINT(
            "Cartridge Type: ROM only (0x%02x)\n", cartridge->rom_data[GAMEBOY_CARTRIDGE_TYPE_ADDRESS]);
        break;
    }
    default:
    {
        not_supported_cartridge_mode = true;
        CARTRIDGE_DEBUG_PRINT(
            "Unsupported Cartridge Type: (0x%02x)\n",
            cartridge->rom_data[GAMEBOY_CARTRIDGE_TYPE_ADDRESS]);
        return false;
    }
    }

    // check ROM Size
    switch (cartridge->rom_data[GAMEBOY_ROM_SIZE_ADDRESS]) {
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
        CARTRIDGE_DEBUG_PRINT(
            "Invalid ROM Banks Count: 0x%02x\n", cartridge->rom_data[GAMEBOY_ROM_SIZE_ADDRESS]);
        return false;
    }
    }
    CARTRIDGE_DEBUG_PRINT("ROM Banks: 0x%02x\n", cartridge->rom_attributes_bank_count);

    // check RAM Size
    switch (cartridge->rom_data[GAMEBOY_RAM_SIZE_ADDRESS]) {
    case 0x01:
    {
        cartridge->ram_attributes_bank_count = 1;
        cartridge->ram_attributes_bank_size  = 2;
        break;
    }
    case 0x02:
    {
        cartridge->ram_attributes_bank_count = 1;
        cartridge->ram_attributes_bank_size  = 8;
        break;
    }
    case 0x03:
    {
        cartridge->ram_attributes_bank_count = 2;
        cartridge->ram_attributes_bank_size  = 8;
        break;
    }
    case 0x04:
    {
        cartridge->ram_attributes_bank_count = 4;
        cartridge->ram_attributes_bank_size  = 8;
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
        CARTRIDGE_DEBUG_PRINT(
            "Invalid RAM Banks Count: 0x%02x\n", cartridge->rom_data[GAMEBOY_RAM_SIZE_ADDRESS]);
        return false;
    }
    }
    if (cartridge->ram_attributes_bank_count > 0) {
        CARTRIDGE_DEBUG_PRINT(
            "RAM Banks: 0x%02x with size 0x%02x * 0x%02xkb\n",
            cartridge->ram_attributes_bank_count,
            cartridge->ram_attributes_bank_count,
            cartridge->ram_attributes_bank_size);
    }
    else {
        CARTRIDGE_DEBUG_PRINT("No RAM banking: 0x%02x\n", cartridge->rom_data[GAMEBOY_RAM_SIZE_ADDRESS]);
    }

    // Allocate memory for RAM (8kb * bank count)
    cartridge->ram_data = malloc(cartridge->ram_attributes_bank_count * 8192);
    if (cartridge->ram_data == NULL) {
        CARTRIDGE_ERROR_PRINT("Failed to allocate memory for RAM\n");
        return false;
    }

    return true;
}

void cartridge_set_rom_bank(struct Cartridge* cartridge, uint8_t bank)
{
    // MBC1 constraint: Bank 0 cannot be selected for upper region
    if (bank == 0) {
        bank = 1;
    }
    
    // Bounds check
    if (bank >= cartridge->rom_attributes_bank_count) {
        bank = bank % cartridge->rom_attributes_bank_count;
        if (bank == 0) bank = 1; // Ensure we don't wrap to bank 0
    }
    
    cartridge->rom_alternative_bank = bank;
}

uint8_t cartridge_get_rom_bank(struct Cartridge* cartridge)
{
    return cartridge->rom_alternative_bank;
}

void cartridge_set_ram_bank(struct Cartridge* cartridge, uint8_t bank)
{
    cartridge->ram_alternative_bank = bank;
}

uint8_t cartridge_get_ram_bank(struct Cartridge* cartridge)
{
    return cartridge->ram_alternative_bank;
}

uint8_t cartridge_get_cartridge_byte(struct Cartridge* cartridge, uint16_t address)
{
    // 0-0x3FFF is always ROM bank 0
    // 0x4000-0x7FFF is alternative ROM bank
    if (address <= 0x3FFF) {
        return cartridge->rom_data[address];
    }
    else if (address >= 0x4000 && address <= 0x7FFF) {
        // Calculate correct ROM bank address
        uint32_t bank_address = (address - 0x4000) + (cartridge->rom_alternative_bank * 0x4000);
        
        // Bounds check
        if (bank_address >= cartridge->rom_size) {
            CARTRIDGE_ERROR_PRINT("ROM access out of bounds: 0x%08x (size: 0x%08x)\n", 
                                 bank_address, (uint32_t)cartridge->rom_size);
            return 0xFF;
        }
        
        return cartridge->rom_data[bank_address];
    }
    else if (address >= 0xA000 && address <= 0xBFFF) {
        if (!cartridge->ram_enabled) {
            CARTRIDGE_DEBUG_PRINT("RAM is disabled, returning 0xFF\n");
            return 0xFF;
        }
        // Calculate correct RAM bank address
        uint32_t bank_address = (address - 0xA000) + (cartridge->ram_alternative_bank * 0x2000);
        CARTRIDGE_TRACE_PRINT("MBC1: RAM read from 0x%04x = 0x%02x\n", bank_address, cartridge->ram_data[bank_address]);
        return cartridge->ram_data[bank_address];
    }
    else {
        CARTRIDGE_DEBUG_PRINT("Trying to access invalid ROM address: 0x%04x\n", address);
        return 0;
    }
}

void cartridge_set_cartridge_byte(struct Cartridge* cartridge, uint16_t address, uint8_t byte)
{
    // MBC1 Banking Controller Logic
    if (address >= 0x0000 && address <= 0x1FFF) {
        // RAM Enable (0x0A enables, anything else disables)
        // TODO: Implement RAM enable/disable when RAM is added
        if ((byte & 0x0F) == 0x0A) {
            CARTRIDGE_DEBUG_PRINT("MBC1: RAM Enabled\n");
            cartridge->ram_enabled = true;
        }
        else {
            CARTRIDGE_DEBUG_PRINT("MBC1: RAM Disabled\n");
            cartridge->ram_enabled = false;
        }
    }
    else if (address >= 0x2000 && address <= 0x3FFF) {
        // ROM Bank Number (lower 5 bits)
        uint8_t bank = byte & 0x1F;
        
        // MBC1 quirk: Bank 0 cannot be selected, defaults to bank 1
        if (bank == 0) {
            bank = 1;
        }
        
        // Bounds check
        if (bank >= cartridge->rom_attributes_bank_count) {
            bank = bank % cartridge->rom_attributes_bank_count;
            CARTRIDGE_WARN_PRINT("ROM bank %d out of range, wrapping to %d\n", 
                                byte & 0x1F, bank);
        }

        // If there's only two banks, do nothing
        if (cartridge->rom_attributes_bank_count == 2) {
            return;
        }
        
        cartridge->rom_alternative_bank = bank;
        CARTRIDGE_DEBUG_PRINT("MBC1: ROM Bank set to %d\n", bank);
    }
    else if (address >= 0x4000 && address <= 0x5FFF) {
        // RAM Bank Number OR Upper ROM Bank bits
        uint8_t bank = byte & 0x03;
        cartridge->ram_alternative_bank = bank;
        CARTRIDGE_DEBUG_PRINT("MBC1: RAM Bank set to %d\n", bank);
    }
    else if (address >= 0x6000 && address <= 0x7FFF) {
        // Banking Mode Select (0 = ROM mode, 1 = RAM mode)
        CARTRIDGE_DEBUG_PRINT("MBC1: Banking mode: %s\n", 
                             (byte & 0x01) ? "RAM mode" : "ROM mode");
    }
    else if (address >= 0xA000 && address <= 0xBFFF) {
        if (!cartridge->ram_enabled) {
            CARTRIDGE_DEBUG_PRINT("RAM is disabled, ignoring write\n");
            return;
        }
        // Calculate correct RAM bank address
        uint32_t bank_address = (address - 0xA000) + (cartridge->ram_alternative_bank * 0x2000);
        cartridge->ram_data[bank_address] = byte;
        CARTRIDGE_TRACE_PRINT("MBC1: RAM write to 0x%04x = 0x%02x\n", bank_address, byte);
    }
    else {
        CARTRIDGE_DEBUG_PRINT("Trying to write to invalid cartridge address: 0x%04x = 0x%02x\n", 
                             address, byte);
    }
}

uint16_t cartridge_get_cartridge_word(struct Cartridge* cartridge, uint16_t address)
{
    if (address <= 0x3FFF) {
        // Bounds check for bank 0
        if (address + 1 >= cartridge->rom_size) {
            CARTRIDGE_ERROR_PRINT("ROM word access out of bounds: 0x%04x\n", address);
            return 0xFFFF;
        }
        return cartridge->rom_data[address] | (cartridge->rom_data[address + 1] << 8);
    }
    else if (address >= 0x4000 && address <= 0x7FFF) {
        // Calculate correct ROM bank address
        uint32_t bank_address = (address - 0x4000) + (cartridge->rom_alternative_bank * 0x4000);
        
        // Bounds check
        if (bank_address + 1 >= cartridge->rom_size) {
            CARTRIDGE_ERROR_PRINT("ROM word access out of bounds: 0x%08x (size: 0x%08x)\n", 
                                 bank_address, (uint32_t)cartridge->rom_size);
            return 0xFFFF;
        }
        
        return cartridge->rom_data[bank_address] | (cartridge->rom_data[bank_address + 1] << 8);
    }
    else {
        CARTRIDGE_DEBUG_PRINT("Trying to access invalid ROM address: 0x%04x\n", address);
        return 0;
    }
}

void cartridge_set_cartridge_word(struct Cartridge* cartridge, uint16_t address, uint16_t word)
{
    // cartridge->rom[address] = word & 0xFF;
    // cartridge->rom[address + 1] = (word >> 8) & 0xFF;
}

char* cartridge_get_rom_name(struct Cartridge* cartridge)
{
    return (char*)cartridge->rom_name;
}

