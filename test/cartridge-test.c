#include "../src/cartridge.h"
#include "test.h"

int main()
{
    config.start_time = get_time_in_seconds();
    printf("=========================\n");
    printf("Cartridge Test\n");
    printf("=========================\n");
    struct Cartridge *cartridge = NULL;

    // CPU_INSTRS
    printf("=========================\n");
    printf("Cartridge: CPU_INSTRS\n");
    printf("=========================\n");
    cartridge = create_cartridge();
    load_cartridge(cartridge, "test/cpu.gb");
    assert(strcmp(cartridge_get_rom_name(cartridge), "CPU_INSTRS") == 0);
    assert(cartridge->rom_attributes_bank_count == 4);
    assert(cartridge->ram_attributes_bank_count == 0);
    free_cartridge(cartridge);

    // SUPER MARIOLAND
    printf("=========================\n");
    printf("Cartridge: SUPER MARIOLAND\n");
    printf("=========================\n");
    cartridge = create_cartridge();
    load_cartridge(cartridge, "test/mario.gb");
    assert(strcmp(cartridge_get_rom_name(cartridge), "SUPER MARIOLAND") == 0);
    assert(cartridge->rom_attributes_bank_count == 4);
    assert(cartridge->ram_attributes_bank_count == 0);
    free_cartridge(cartridge);

    // ZELDA
    printf("=========================\n");
    printf("Cartridge: ZELDA\n");
    printf("=========================\n");
    cartridge = create_cartridge();
    load_cartridge(cartridge, "test/zelda.gb");
    assert(strcmp(cartridge_get_rom_name(cartridge), "ZELDA") == 0);
    assert(cartridge->rom_attributes_bank_count == 32);
    assert(cartridge->ram_attributes_bank_count == 1);
    free_cartridge(cartridge);

    // TANK (BATTLECITY)
    printf("=========================\n");
    printf("Cartridge: BATTLECITY\n");
    printf("=========================\n");
    cartridge = create_cartridge();
    load_cartridge(cartridge, "test/tank.gb");
    assert(strcmp(cartridge_get_rom_name(cartridge), "BATTLECITY") == 0);
    assert(cartridge->rom_attributes_bank_count == 2);
    assert(cartridge->ram_attributes_bank_count == 0);
    free_cartridge(cartridge);

    return 0;
}
