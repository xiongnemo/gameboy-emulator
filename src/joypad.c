#include "joypad.h"

void joypad_interrupts(struct Joypad* joypad)
{
    // Get current IF status
    uint8_t current_interrupt = joypad->mmu->mmu_get_byte(joypad->mmu, IF_ADDRESS);

    // Check if we need to request a joypad interrupt
    if ((joypad->key_column == 0x10 && joypad->keys_controls != 0x0f) ||
        (joypad->key_column == 0x20 && joypad->keys_directions != 0x0f))
    {
        // Set bit 4 to 1 (Joypad interrupt)
        current_interrupt |= 0x10;
        
        // Write back to memory
        joypad->mmu->mmu_set_byte(joypad->mmu, IF_ADDRESS, current_interrupt);
    }
}

void write_result(struct Joypad* joypad)
{
    uint8_t column_requested = joypad->mmu->mmu_get_byte(joypad->mmu, 0xFF00) & 0x30;
    
    if (column_requested == 0x10)
    {
        joypad->temp_ff00 = 0x10 + joypad->keys_controls;
    }
    if (column_requested == 0x20) 
    {
        joypad->temp_ff00 = 0x20 + joypad->keys_directions;
    }
    
    joypad->mmu->mmu_set_byte(joypad->mmu, JOYPAD_ADDRESS, joypad->temp_ff00);
}

void reset_joypad(struct Joypad* joypad)
{
    // Reset selected column
    joypad->key_column = joypad->key_column & 0x30;

    // Reset flags
    joypad->column_controls = 0;
    joypad->column_direction = 0;
}

struct Joypad* create_joypad(struct MMU* mmu)
{
    struct Joypad* joypad = (struct Joypad*)malloc(sizeof(struct Joypad));
    if (!joypad) {
        return NULL;
    }

    // Initialize default values
    joypad->key_column = 0x00;
    joypad->column_direction = 0;
    joypad->column_controls = 0;
    joypad->keys_directions = 0x0F;
    joypad->keys_controls = 0x0F;
    joypad->temp_ff00 = 0x00;
    joypad->save_flag = 0x00;
    joypad->load_flag = 0x00;
    // joypad->fast_forward_flag = 0x00;

    // Set up method pointers
    joypad->joypad_interrupts = joypad_interrupts;
    joypad->write_result = write_result;
    joypad->reset_joypad = reset_joypad;

    // Set up MMU
    joypad->mmu = mmu;

    return joypad;
}

void free_joypad(struct Joypad* joypad)
{
    if (joypad) {
        free(joypad);
    }
}