#include "../src/ram.h"
#include "test.h"
int main(void)
{
    config.start_time = get_time_in_seconds();
    struct Ram *ram = create_ram();
    assert(ram != NULL);
    for (int i = 0; i < 65536; i++)
    {
        assert(ram->get_ram_byte(ram, i) == 0x00);
    }
    // test set_ram_byte
    ram->set_ram_byte(ram, 0x0000, 0xAB);
    assert(ram->get_ram_byte(ram, 0x0000) == 0xAB);
    // test set_ram_word
    ram->set_ram_word(ram, 0x0000, 0xABCD);
    assert(ram->get_ram_word(ram, 0x0000) == 0xABCD);
    // test set_ram_byte with word address
    ram->set_ram_byte(ram, 0xABCD, 0xEF);
    assert(ram->get_ram_byte(ram, 0xABCD) == 0xEF);
    // test set_ram_word with byte address
    ram->set_ram_word(ram, 0xABCD, 0xEF01);
    assert(ram->get_ram_word(ram, 0xABCD) == 0xEF01);

    // free ram
    free(ram);
    return 0;
}
