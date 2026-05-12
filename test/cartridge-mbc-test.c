#include "test.h"
#include "../src/cartridge.h"

// Test helper to create a mock cartridge with specific controller type
struct Cartridge* create_test_cartridge(uint8_t controller_type, uint8_t rom_banks, uint8_t ram_banks) {
    struct Cartridge* cart = create_cartridge();
    if (!cart) return NULL;
    
    // Create mock ROM data
    cart->rom_size = rom_banks * 0x4000;
    cart->rom_data = malloc(cart->rom_size);
    memset(cart->rom_data, 0, cart->rom_size);
    
    // Set controller type in ROM header
    cart->rom_data[GAMEBOY_CARTRIDGE_TYPE_ADDRESS] = controller_type;
    cart->rom_data[GAMEBOY_ROM_SIZE_ADDRESS] = 0x01; // 64KB = 4 banks for testing
    cart->rom_data[GAMEBOY_RAM_SIZE_ADDRESS] = (ram_banks > 0) ? 0x02 : 0x00; // 8KB or none
    
    // Initialize cartridge
    check_cartridge_type(cart);
    
    return cart;
}

void test_mbc2_banking() {
    printf("Testing MBC2 banking...\n");
    
    struct Cartridge* cart = create_test_cartridge(CONTROLLER_MBC2, 4, 0);
    assert(cart != NULL); // Failed to create MBC2 cartridge
    
    // Test ROM bank switching (address bit 8 = 1)
    cart->set_cartridge_byte(cart, 0x2100, 0x02); // Switch to ROM bank 2
    assert(cart->rom_alternative_bank == 2); // MBC2 ROM bank should be 2
    
    // Test ROM bank 0 restriction
    cart->set_cartridge_byte(cart, 0x2100, 0x00); // Try to switch to ROM bank 0
    assert(cart->rom_alternative_bank == 1); // MBC2 should not allow ROM bank 0
    
    // Test RAM enable/disable (address bit 8 = 0)
    cart->set_cartridge_byte(cart, 0x0000, 0x0A); // Enable RAM
    assert(cart->mbc2_ram_enabled == true); // MBC2 RAM should be enabled
    
    cart->set_cartridge_byte(cart, 0x0000, 0x00); // Disable RAM
    assert(cart->mbc2_ram_enabled == false); // MBC2 RAM should be disabled
    
    // Test MBC2 internal RAM write/read
    cart->mbc2_ram_enabled = true;
    cart->set_cartridge_byte(cart, 0xA000, 0xAF); // Write to MBC2 RAM
    uint8_t value = cart->get_cartridge_byte(cart, 0xA000);
    assert((value & 0x0F) == 0x0F); // MBC2 RAM should store only lower 4 bits
    assert((value & 0xF0) == 0xF0); // MBC2 RAM should have upper 4 bits set
    
    free_cartridge(cart);
    printf("MBC2 banking tests passed!\n");
}

void test_mbc3_banking() {
    printf("Testing MBC3 banking...\n");
    
    struct Cartridge* cart = create_test_cartridge(CONTROLLER_MBC3_RAM, 16, 4);
    assert(cart != NULL); // Failed to create MBC3 cartridge
    
    // Test ROM bank switching (7 bits)
    cart->set_cartridge_byte(cart, 0x2000, 0x7F); // Switch to ROM bank 127
    assert(cart->rom_alternative_bank == 3); // MBC3 ROM bank should wrap to available banks (127 % 4 = 3)
    
    cart->set_cartridge_byte(cart, 0x2000, 0x05); // Switch to ROM bank 5
    assert(cart->rom_alternative_bank == 1); // MBC3 ROM bank should wrap (5 % 4 = 1)
    
    // Test ROM bank 0 restriction
    cart->set_cartridge_byte(cart, 0x2000, 0x00); // Try to switch to ROM bank 0
    assert(cart->rom_alternative_bank == 1); // MBC3 should not allow ROM bank 0
    
    // Test RAM enable/disable
    cart->set_cartridge_byte(cart, 0x0000, 0x0A); // Enable RAM
    assert(cart->ram_enabled == true); // MBC3 RAM should be enabled
    
    // Test RAM bank switching
    cart->set_cartridge_byte(cart, 0x4000, 0x02); // Switch to RAM bank 2
    assert(cart->ram_alternative_bank == 2); // MBC3 RAM bank should be 2
    
    free_cartridge(cart);
    printf("MBC3 banking tests passed!\n");
}

void test_mbc5_banking() {
    printf("Testing MBC5 banking...\n");
    
    struct Cartridge* cart = create_test_cartridge(CONTROLLER_MBC5_RAM, 64, 16);
    assert(cart != NULL); // Failed to create MBC5 cartridge
    
    // Test 9-bit ROM bank switching
    cart->set_cartridge_byte(cart, 0x2000, 0xFF); // Set lower 8 bits
    cart->set_cartridge_byte(cart, 0x3000, 0x01); // Set upper bit
    assert(cart->mbc5_rom_bank == 0x1FF); // MBC5 ROM bank should be 0x1FF (even if it exceeds available banks)
    
    // Test that MBC5 allows ROM bank 0 (unlike other MBCs)
    cart->set_cartridge_byte(cart, 0x2000, 0x00); // Lower 8 bits = 0
    cart->set_cartridge_byte(cart, 0x3000, 0x00); // Upper bit = 0
    assert(cart->mbc5_rom_bank == 0); // MBC5 should allow ROM bank 0
    
    // Test RAM enable/disable
    cart->set_cartridge_byte(cart, 0x0000, 0x0A); // Enable RAM
    assert(cart->ram_enabled == true); // MBC5 RAM should be enabled
    
    // Test RAM bank switching
    cart->set_cartridge_byte(cart, 0x4000, 0x0F); // Switch to RAM bank 15
    assert(cart->ram_alternative_bank == 15); // MBC5 RAM bank should be 15
    
    free_cartridge(cart);
    printf("MBC5 banking tests passed!\n");
}

void test_mbc5_rumble() {
    printf("Testing MBC5 rumble functionality...\n");
    
    struct Cartridge* cart = create_test_cartridge(CONTROLLER_MBC5_RUMBLE_RAM, 32, 8);
    assert(cart != NULL); // Failed to create MBC5 rumble cartridge
    
    // Test rumble motor control
    cart->set_cartridge_byte(cart, 0x4000, 0x08); // Turn on rumble motor (bit 3)
    assert(cart->rumble_motor_on == true); // MBC5 rumble motor should be on
    assert(cart->ram_alternative_bank == 0); // MBC5 RAM bank should be 0
    
    cart->set_cartridge_byte(cart, 0x4000, 0x05); // Turn off rumble, set RAM bank 5
    assert(cart->rumble_motor_on == false); // MBC5 rumble motor should be off
    assert(cart->ram_alternative_bank == 5); // MBC5 RAM bank should be 5
    
    free_cartridge(cart);
    printf("MBC5 rumble tests passed!\n");
}

int main() {
    printf("Running MBC cartridge tests...\n\n");
    
    test_mbc2_banking();
    test_mbc3_banking();
    test_mbc5_banking();
    test_mbc5_rumble();
    
    printf("\nAll MBC tests passed successfully!\n");
    return 0;
} 