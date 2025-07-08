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

    // Initialize MBC2 internal RAM
    memset(cartridge->mbc2_ram, 0, sizeof(cartridge->mbc2_ram));
    cartridge->mbc2_ram_enabled = false;

    // Initialize controller type
    cartridge->controller_type = CONTROLLER_ROM_ONLY;

    // Initialize MBC5 extended ROM banking
    cartridge->mbc5_rom_bank = 1;

    // Initialize rumble motor
    cartridge->rumble_motor_on = false;

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
    // Store controller type
    cartridge->controller_type = cartridge->rom_data[GAMEBOY_CARTRIDGE_TYPE_ADDRESS];

    switch (cartridge->controller_type) {
    case CONTROLLER_ROM_ONLY:
        CARTRIDGE_DEBUG_PRINT("Cartridge Type: ROM only (0x%02x)\n", cartridge->controller_type);
        break;
    case CONTROLLER_MBC1:
        CARTRIDGE_DEBUG_PRINT("Cartridge Type: ROM + MBC1 (0x%02x)\n", cartridge->controller_type);
        break;
    case CONTROLLER_MBC1_RAM:
        CARTRIDGE_DEBUG_PRINT("Cartridge Type: ROM + MBC1 + RAM (0x%02x)\n", cartridge->controller_type);
        break;
    case CONTROLLER_MBC1_RAM_BATTERY:
        CARTRIDGE_DEBUG_PRINT("Cartridge Type: ROM + MBC1 + RAM + BATTERY (0x%02x)\n", cartridge->controller_type);
        break;
    case CONTROLLER_MBC2:
        CARTRIDGE_DEBUG_PRINT("Cartridge Type: ROM + MBC2 (0x%02x)\n", cartridge->controller_type);
        break;
    case CONTROLLER_MBC2_BATTERY:
        CARTRIDGE_DEBUG_PRINT("Cartridge Type: ROM + MBC2 + BATTERY (0x%02x)\n", cartridge->controller_type);
        break;
    case CONTROLLER_MBC3_TIMER_BATTERY:
        CARTRIDGE_DEBUG_PRINT("Cartridge Type: ROM + MBC3 + TIMER + BATTERY (0x%02x)\n", cartridge->controller_type);
        break;
    case CONTROLLER_MBC3_TIMER_RAM_BATTERY:
        CARTRIDGE_DEBUG_PRINT("Cartridge Type: ROM + MBC3 + TIMER + RAM + BATTERY (0x%02x)\n", cartridge->controller_type);
        break;
    case CONTROLLER_MBC3:
        CARTRIDGE_DEBUG_PRINT("Cartridge Type: ROM + MBC3 (0x%02x)\n", cartridge->controller_type);
        break;
    case CONTROLLER_MBC3_RAM:
        CARTRIDGE_DEBUG_PRINT("Cartridge Type: ROM + MBC3 + RAM (0x%02x)\n", cartridge->controller_type);
        break;
    case CONTROLLER_MBC3_RAM_BATTERY:
        CARTRIDGE_DEBUG_PRINT("Cartridge Type: ROM + MBC3 + RAM + BATTERY (0x%02x)\n", cartridge->controller_type);
        break;
    case CONTROLLER_MBC5:
        CARTRIDGE_DEBUG_PRINT("Cartridge Type: ROM + MBC5 (0x%02x)\n", cartridge->controller_type);
        break;
    case CONTROLLER_MBC5_RAM:
        CARTRIDGE_DEBUG_PRINT("Cartridge Type: ROM + MBC5 + RAM (0x%02x)\n", cartridge->controller_type);
        break;
    case CONTROLLER_MBC5_RAM_BATTERY:
        CARTRIDGE_DEBUG_PRINT("Cartridge Type: ROM + MBC5 + RAM + BATTERY (0x%02x)\n", cartridge->controller_type);
        break;
    case CONTROLLER_MBC5_RUMBLE:
        CARTRIDGE_DEBUG_PRINT("Cartridge Type: ROM + MBC5 + RUMBLE (0x%02x)\n", cartridge->controller_type);
        break;
    case CONTROLLER_MBC5_RUMBLE_RAM:
        CARTRIDGE_DEBUG_PRINT("Cartridge Type: ROM + MBC5 + RUMBLE + RAM (0x%02x)\n", cartridge->controller_type);
        break;
    case CONTROLLER_MBC5_RUMBLE_RAM_BATTERY:
        CARTRIDGE_DEBUG_PRINT("Cartridge Type: ROM + MBC5 + RUMBLE + RAM + BATTERY (0x%02x)\n", cartridge->controller_type);
        break;
    default:
        CARTRIDGE_DEBUG_PRINT("Unsupported Cartridge Type: (0x%02x)\n", cartridge->controller_type);
        return false;
    }

    // check ROM Size
    switch (cartridge->rom_data[GAMEBOY_ROM_SIZE_ADDRESS]) {
    case 0x00:
        cartridge->rom_attributes_bank_count = 2;    // 256Kbit = 32KB = 2 banks
        break;
    case 0x01:
        cartridge->rom_attributes_bank_count = 4;    // 512Kbit = 64KB = 4 banks
        break;
    case 0x02:
        cartridge->rom_attributes_bank_count = 8;    // 1Mbit = 128KB = 8 banks
        break;
    case 0x03:
        cartridge->rom_attributes_bank_count = 16;   // 2Mbit = 256KB = 16 banks
        break;
    case 0x04:
        cartridge->rom_attributes_bank_count = 32;   // 4Mbit = 512KB = 32 banks
        break;
    case 0x05:
        cartridge->rom_attributes_bank_count = 64;   // 8Mbit = 1MB = 64 banks
        break;
    case 0x06:
        cartridge->rom_attributes_bank_count = 128;  // 16Mbit = 2MB = 128 banks
        break;
    case 0x52:
        cartridge->rom_attributes_bank_count = 72;   // 9Mbit = 1.1MB = 72 banks
        break;
    case 0x53:
        cartridge->rom_attributes_bank_count = 80;   // 10Mbit = 1.2MB = 80 banks
        break;
    case 0x54:
        cartridge->rom_attributes_bank_count = 96;   // 12Mbit = 1.5MB = 96 banks
        break;
    default:
        CARTRIDGE_DEBUG_PRINT("Invalid ROM Banks Count: 0x%02x\n", cartridge->rom_data[GAMEBOY_ROM_SIZE_ADDRESS]);
        return false;
    }
    CARTRIDGE_DEBUG_PRINT("ROM Banks: 0x%02x\n", cartridge->rom_attributes_bank_count);

    // check RAM Size
    switch (cartridge->rom_data[GAMEBOY_RAM_SIZE_ADDRESS]) {
    case 0x00:
        cartridge->ram_attributes_bank_count = 0;  // None
        cartridge->ram_attributes_bank_size = 0;
        break;
    case 0x01:
        cartridge->ram_attributes_bank_count = 1;  // 16kBit = 2kB = 1 bank
        cartridge->ram_attributes_bank_size = 2;
        break;
    case 0x02:
        cartridge->ram_attributes_bank_count = 1;  // 64kBit = 8kB = 1 bank
        cartridge->ram_attributes_bank_size = 8;
        break;
    case 0x03:
        cartridge->ram_attributes_bank_count = 4;  // 256kBit = 32kB = 4 banks
        cartridge->ram_attributes_bank_size = 8;
        break;
    case 0x04:
        cartridge->ram_attributes_bank_count = 16; // 1MBit = 128kB = 16 banks
        cartridge->ram_attributes_bank_size = 8;
        break;
    default:
        CARTRIDGE_DEBUG_PRINT("Invalid RAM Banks Count: 0x%02x\n", cartridge->rom_data[GAMEBOY_RAM_SIZE_ADDRESS]);
        return false;
    }

    // MBC2 has internal RAM regardless of the RAM size byte
    if (cartridge->controller_type == CONTROLLER_MBC2 || cartridge->controller_type == CONTROLLER_MBC2_BATTERY) {
        CARTRIDGE_DEBUG_PRINT("MBC2: Using internal 512x4bit RAM\n");
        // MBC2 internal RAM is already allocated in the struct
    } else if (cartridge->ram_attributes_bank_count > 0) {
        CARTRIDGE_DEBUG_PRINT("RAM Banks: %d with size %dKB each\n",
                             cartridge->ram_attributes_bank_count,
                             cartridge->ram_attributes_bank_size);
        
        // Allocate memory for external RAM
        cartridge->ram_size = cartridge->ram_attributes_bank_count * cartridge->ram_attributes_bank_size * 1024;
        cartridge->ram_data = malloc(cartridge->ram_size);
        if (cartridge->ram_data == NULL) {
            CARTRIDGE_ERROR_PRINT("Failed to allocate memory for RAM\n");
            return false;
        }
        memset(cartridge->ram_data, 0, cartridge->ram_size);
    } else {
        CARTRIDGE_DEBUG_PRINT("No external RAM\n");
    }

    return true;
}

void cartridge_set_rom_bank(struct Cartridge* cartridge, uint8_t bank)
{
    // MBC1 constraint: Bank 0 cannot be selected for upper region
    // But MBC5 can select bank 0 for upper region
    if (bank == 0 
        && cartridge->controller_type != CONTROLLER_MBC5
        && cartridge->controller_type != CONTROLLER_MBC5_RUMBLE
        && cartridge->controller_type != CONTROLLER_MBC5_RUMBLE_RAM
        && cartridge->controller_type != CONTROLLER_MBC5_RUMBLE_RAM_BATTERY
    ) {
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

// MBC1 handler
static void cartridge_handle_mbc1_write(struct Cartridge* cartridge, uint16_t address, uint8_t byte)
{
    if (address >= 0x0000 && address <= 0x1FFF) {
        // RAM Enable (0x0A enables, anything else disables)
        if ((byte & 0x0F) == 0x0A) {
            cartridge->ram_enabled = true;
            CARTRIDGE_DEBUG_PRINT("MBC1: RAM Enabled\n");
        }
        else {
            cartridge->ram_enabled = false;
            CARTRIDGE_DEBUG_PRINT("MBC1: RAM Disabled\n");
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
            if (bank == 0) bank = 1; // Ensure we don't wrap to bank 0
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
        if (!cartridge->ram_enabled || cartridge->ram_data == NULL) {
            return;
        }
        // Calculate correct RAM bank address
        uint32_t bank_address = (address - 0xA000) + (cartridge->ram_alternative_bank * 0x2000);
        if (bank_address < cartridge->ram_size) {
            cartridge->ram_data[bank_address] = byte;
        }
    }
}

// MBC2 handler
static void cartridge_handle_mbc2_write(struct Cartridge* cartridge, uint16_t address, uint8_t byte)
{
    if (address >= 0x0000 && address <= 0x3FFF) {
        // Check the least significant bit of the upper address byte
        if ((address & 0x0100) == 0x0000) {
            // RAM Enable/Disable (bit 8 = 0)
            if ((byte & 0x0F) == 0x0A) {
                cartridge->mbc2_ram_enabled = true;
                CARTRIDGE_DEBUG_PRINT("MBC2: RAM Enabled\n");
            }
            else {
                cartridge->mbc2_ram_enabled = false;
                CARTRIDGE_DEBUG_PRINT("MBC2: RAM Disabled\n");
            }
        }
        else {
            // ROM Bank Selection (bit 8 = 1)
            uint8_t bank = byte & 0x0F; // Only 4 bits for MBC2
            
            // MBC2 quirk: Bank 0 cannot be selected, defaults to bank 1
            if (bank == 0) {
                bank = 1;
            }
            
            // MBC2 supports up to 16 banks (2Mbit)
            if (bank >= cartridge->rom_attributes_bank_count) {
                bank = bank % cartridge->rom_attributes_bank_count;
                if (bank == 0) bank = 1;
            }
            
            cartridge->rom_alternative_bank = bank;
            CARTRIDGE_DEBUG_PRINT("MBC2: ROM Bank set to %d\n", bank);
        }
    }
    else if (address >= 0xA000 && address <= 0xBFFF) {
        if (!cartridge->mbc2_ram_enabled) {
            return;
        }
        
        // MBC2 has 512 x 4-bit RAM (0xA000-0xA1FF, only lower 4 bits)
        uint16_t ram_address = address & 0x01FF; // Mask to 512 bytes
        if (ram_address < 256) {
            cartridge->mbc2_ram[ram_address] = byte & 0x0F; // Only store lower 4 bits
            CARTRIDGE_TRACE_PRINT("MBC2: RAM write to 0x%04x = 0x%02x\n", ram_address, byte & 0x0F);
        }
    }
}

// MBC3 handler
static void cartridge_handle_mbc3_write(struct Cartridge* cartridge, uint16_t address, uint8_t byte)
{
    if (address >= 0x0000 && address <= 0x1FFF) {
        // RAM Enable (0x0A enables, anything else disables)
        if ((byte & 0x0F) == 0x0A) {
            cartridge->ram_enabled = true;
            CARTRIDGE_DEBUG_PRINT("MBC3: RAM Enabled\n");
        }
        else {
            cartridge->ram_enabled = false;
            CARTRIDGE_DEBUG_PRINT("MBC3: RAM Disabled\n");
        }
    }
    else if (address >= 0x2000 && address <= 0x3FFF) {
        // ROM Bank Number (7 bits)
        uint8_t bank = byte & 0x7F;
        
        // MBC3 quirk: Bank 0 cannot be selected, defaults to bank 1
        if (bank == 0) {
            bank = 1;
        }
        
        // Bounds check
        if (bank >= cartridge->rom_attributes_bank_count) {
            bank = bank % cartridge->rom_attributes_bank_count;
            if (bank == 0) bank = 1;
        }
        
        cartridge->rom_alternative_bank = bank;
        CARTRIDGE_DEBUG_PRINT("MBC3: ROM Bank set to %d\n", bank);
    }
    else if (address >= 0x4000 && address <= 0x5FFF) {
        // RAM Bank Number (0x00-0x03) or RTC Register Select (0x08-0x0C)
        if (byte <= 0x03) {
            cartridge->ram_alternative_bank = byte;
            CARTRIDGE_DEBUG_PRINT("MBC3: RAM Bank set to %d\n", byte);
        }
        else if (byte >= 0x08 && byte <= 0x0C) {
            // RTC register selection - not implemented yet
            CARTRIDGE_DEBUG_PRINT("MBC3: RTC register 0x%02x selected (not implemented)\n", byte);
        }
    }
    else if (address >= 0x6000 && address <= 0x7FFF) {
        // Latch Clock Data - not implemented yet
        CARTRIDGE_DEBUG_PRINT("MBC3: Clock latch command (not implemented)\n");
    }
    else if (address >= 0xA000 && address <= 0xBFFF) {
        if (!cartridge->ram_enabled || cartridge->ram_data == NULL) {
            return;
        }
        // Calculate correct RAM bank address
        uint32_t bank_address = (address - 0xA000) + (cartridge->ram_alternative_bank * 0x2000);
        if (bank_address < cartridge->ram_size) {
            cartridge->ram_data[bank_address] = byte;
        }
    }
}

// MBC5 handler  
static void cartridge_handle_mbc5_write(struct Cartridge* cartridge, uint16_t address, uint8_t byte)
{
    if (address >= 0x0000 && address <= 0x1FFF) {
        // RAM Enable (0x0A enables, anything else disables)
        if ((byte & 0x0F) == 0x0A) {
            cartridge->ram_enabled = true;
            CARTRIDGE_DEBUG_PRINT("MBC5: RAM Enabled\n");
        }
        else {
            cartridge->ram_enabled = false;
            CARTRIDGE_DEBUG_PRINT("MBC5: RAM Disabled\n");
        }
    }
    else if (address >= 0x2000 && address <= 0x2FFF) {
        // ROM Bank Number (lower 8 bits)
        cartridge->mbc5_rom_bank = (cartridge->mbc5_rom_bank & 0x0100) | byte;
        CARTRIDGE_DEBUG_PRINT("MBC5: ROM Bank set to %d\n", cartridge->mbc5_rom_bank);
    }
    else if (address >= 0x3000 && address <= 0x3FFF) {
        // ROM Bank Number (upper bit)
        cartridge->mbc5_rom_bank = (cartridge->mbc5_rom_bank & 0x00FF) | ((byte & 0x01) << 8);
        CARTRIDGE_DEBUG_PRINT("MBC5: ROM Bank set to %d\n", cartridge->mbc5_rom_bank);
    }
    else if (address >= 0x4000 && address <= 0x5FFF) {
        // RAM Bank Number (and rumble motor for rumble carts)
        bool is_rumble_cart = (cartridge->controller_type == CONTROLLER_MBC5_RUMBLE ||
                              cartridge->controller_type == CONTROLLER_MBC5_RUMBLE_RAM ||
                              cartridge->controller_type == CONTROLLER_MBC5_RUMBLE_RAM_BATTERY);
                              
        if (is_rumble_cart) {
            // Rumble motor control (bit 3) and RAM bank (bits 0-2)
            bool motor_on = (byte & 0x08) != 0;
            if (motor_on != cartridge->rumble_motor_on) {
                cartridge->rumble_motor_on = motor_on;
                CARTRIDGE_WARN_PRINT("MBC5: Rumble motor %s\n", motor_on ? "ON" : "OFF");
            }
            cartridge->ram_alternative_bank = byte & 0x07;
        }
        else {
            // Regular RAM bank selection (4 bits)
            cartridge->ram_alternative_bank = byte & 0x0F;
        }
        CARTRIDGE_DEBUG_PRINT("MBC5: RAM Bank set to %d\n", cartridge->ram_alternative_bank);
    }
    else if (address >= 0xA000 && address <= 0xBFFF) {
        if (!cartridge->ram_enabled || cartridge->ram_data == NULL) {
            return;
        }
        // Calculate correct RAM bank address
        uint32_t bank_address = (address - 0xA000) + (cartridge->ram_alternative_bank * 0x2000);
        if (bank_address < cartridge->ram_size) {
            cartridge->ram_data[bank_address] = byte;
        }
    }
}

uint8_t cartridge_get_cartridge_byte(struct Cartridge* cartridge, uint16_t address)
{
    // 0x0000-0x3FFF is always ROM bank 0
    if (address <= 0x3FFF) {
        return cartridge->rom_data[address];
    }
    // 0x4000-0x7FFF is switchable ROM bank
    else if (address >= 0x4000 && address <= 0x7FFF) {
        uint16_t rom_bank;
        
        // Determine which ROM bank to use based on controller type
        switch (cartridge->controller_type) {
        case CONTROLLER_MBC5:
        case CONTROLLER_MBC5_RAM:
        case CONTROLLER_MBC5_RAM_BATTERY:
        case CONTROLLER_MBC5_RUMBLE:
        case CONTROLLER_MBC5_RUMBLE_RAM:
        case CONTROLLER_MBC5_RUMBLE_RAM_BATTERY:
            rom_bank = cartridge->mbc5_rom_bank;
            break;
        default:
            rom_bank = cartridge->rom_alternative_bank;
            break;
        }
        
        // Calculate correct ROM bank address
        uint32_t bank_address = (address - 0x4000) + (rom_bank * 0x4000);
        
        // Bounds check
        if (bank_address >= cartridge->rom_size) {
            CARTRIDGE_ERROR_PRINT("ROM access out of bounds: 0x%08x (size: 0x%08x)\n", 
                                 bank_address, (uint32_t)cartridge->rom_size);
            return 0xFF;
        }
        
        return cartridge->rom_data[bank_address];
    }
    // 0xA000-0xBFFF is RAM area
    else if (address >= 0xA000 && address <= 0xBFFF) {
        // Handle MBC2 internal RAM
        if (cartridge->controller_type == CONTROLLER_MBC2 || cartridge->controller_type == CONTROLLER_MBC2_BATTERY) {
            if (!cartridge->mbc2_ram_enabled) {
                return 0xFF;
            }
            
            // MBC2 has 512 x 4-bit RAM (0xA000-0xA1FF, only lower 4 bits)
            uint16_t ram_address = address & 0x01FF; // Mask to 512 bytes
            if (ram_address < 256) {
                return cartridge->mbc2_ram[ram_address] | 0xF0; // Upper 4 bits always set
            }
            return 0xFF;
        }
        
        // Handle external RAM for other controllers
        if (!cartridge->ram_enabled) {
            return 0xFF;
        }
        
        if (cartridge->ram_data == NULL) {
            return 0xFF;
        }
        
        // Calculate correct RAM bank address
        uint32_t bank_address = (address - 0xA000) + (cartridge->ram_alternative_bank * 0x2000);
        
        // Bounds check
        if (bank_address >= cartridge->ram_size) {
            return 0xFF;
        }
        
        return cartridge->ram_data[bank_address];
    }
    else {
        CARTRIDGE_DEBUG_PRINT("Trying to access invalid address: 0x%04x\n", address);
        return 0xFF;
    }
}

void cartridge_set_cartridge_byte(struct Cartridge* cartridge, uint16_t address, uint8_t byte)
{
    switch (cartridge->controller_type) {
    case CONTROLLER_ROM_ONLY:
        // ROM only - no banking, just ignore writes to control areas
        break;
        
    case CONTROLLER_MBC1:
    case CONTROLLER_MBC1_RAM:
    case CONTROLLER_MBC1_RAM_BATTERY:
        cartridge_handle_mbc1_write(cartridge, address, byte);
        break;
        
    case CONTROLLER_MBC2:
    case CONTROLLER_MBC2_BATTERY:
        cartridge_handle_mbc2_write(cartridge, address, byte);
        break;
        
    case CONTROLLER_MBC3:
    case CONTROLLER_MBC3_RAM:
    case CONTROLLER_MBC3_RAM_BATTERY:
    case CONTROLLER_MBC3_TIMER_BATTERY:
    case CONTROLLER_MBC3_TIMER_RAM_BATTERY:
        cartridge_handle_mbc3_write(cartridge, address, byte);
        break;
        
    case CONTROLLER_MBC5:
    case CONTROLLER_MBC5_RAM:
    case CONTROLLER_MBC5_RAM_BATTERY:
    case CONTROLLER_MBC5_RUMBLE:
    case CONTROLLER_MBC5_RUMBLE_RAM:
    case CONTROLLER_MBC5_RUMBLE_RAM_BATTERY:
        cartridge_handle_mbc5_write(cartridge, address, byte);
        break;
        
    default:
        CARTRIDGE_DEBUG_PRINT("Unsupported controller type for write: 0x%02x\n", cartridge->controller_type);
        break;
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

