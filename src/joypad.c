#include "joypad.h"

void handle_joypad_input(struct Joypad* joypad)
{
    // Get current joypad register value (preserves CPU's column selection)
    uint8_t current_ff00 = joypad->mmu->mmu_get_byte(joypad->mmu, JOYPAD_ADDRESS);
    uint8_t column_selection = current_ff00 & 0x30; // Bits 4-5 for column selection
    
    // Start with the column selection bits
    uint8_t result = column_selection;
    bool should_interrupt = false;

    // Game Boy joypad logic:
    // Bit 4 = 0: Select direction keys (when bit 4 is cleared)
    // Bit 5 = 0: Select control keys (when bit 5 is cleared)
    // Lower 4 bits represent key states (0=pressed, 1=not pressed)
    
    // Check direction keys column (bit 4 = 0)
    if (!(column_selection & 0x10)) {
        // Direction keys selected - OR the direction key states into lower 4 bits
        result |= (joypad->keys_directions & 0x0F);
        
        // Check if any direction key is pressed (any bit is 0)
        if (joypad->keys_directions != 0x0F) {
            should_interrupt = true;
        }
    }
    
    // Check control keys column (bit 5 = 0)
    if (!(column_selection & 0x20)) {
        // Control keys selected - OR the control key states into lower 4 bits
        result |= (joypad->keys_controls & 0x0F);
        
        // Check if any control key is pressed (any bit is 0)
        if (joypad->keys_controls != 0x0F) {
            should_interrupt = true;
        }
    }
    
    // If both columns are selected (both bits 4 and 5 are 0), combine both
    if (!(column_selection & 0x10) && !(column_selection & 0x20)) {
        result |= (joypad->keys_directions & joypad->keys_controls & 0x0F);
    }
    
    // If no column is selected (both bits 4 and 5 are 1), return all 1s for lower 4 bits
    if ((column_selection & 0x30) == 0x30) {
        result |= 0x0F;
    }

    // Write the result back to the joypad register
    joypad->mmu->mmu_set_byte(joypad->mmu, JOYPAD_ADDRESS, result);

    // Trigger joypad interrupt if any key is pressed in the selected column(s)
    if (should_interrupt) {
        JOYPAD_TRACE_PRINT("Joypad interrupt triggered - column: 0x%02X, directions: 0x%02X, controls: 0x%02X\n", 
                          column_selection, joypad->keys_directions, joypad->keys_controls);
        
        // Set bit 4 in the interrupt flag register (IF) for joypad interrupt
        uint8_t current_interrupt = joypad->mmu->mmu_get_byte(joypad->mmu, IF_ADDRESS);
        current_interrupt |= 0x10;
        joypad->mmu->mmu_set_byte(joypad->mmu, IF_ADDRESS, current_interrupt);
    }
    // uint8_t column_requested = joypad->mmu->mmu_get_byte(joypad->mmu, JOYPAD_ADDRESS) & 0x30;
    // uint8_t temp_ff00 = 0x00;
    // if (column_requested == 0x10) {
    //     temp_ff00 = 0x10 + joypad->keys_directions;
    //     joypad->mmu->mmu_set_byte(joypad->mmu, JOYPAD_ADDRESS, temp_ff00);
    // }
    // else if (column_requested == 0x20) {
    //     temp_ff00 = 0x20 + joypad->keys_controls;
    //     joypad->mmu->mmu_set_byte(joypad->mmu, JOYPAD_ADDRESS, temp_ff00);
    // }
    // joypad->mmu->mmu_set_byte(joypad->mmu, JOYPAD_ADDRESS, 0x3F);
}

struct Joypad* create_joypad(struct MMU* mmu)
{
    struct Joypad* joypad = (struct Joypad*)malloc(sizeof(struct Joypad));
    if (!joypad) {
        return NULL;
    }

    // Initialize default values (0x0F = all keys not pressed)
    joypad->keys_directions = 0x0F;  // Bit pattern: 1111 (all direction keys released)
    joypad->keys_controls = 0x0F;    // Bit pattern: 1111 (all control keys released)
    joypad->save_flag = 0x00;
    joypad->load_flag = 0x00;

    // Set up method pointers
    joypad->handle_joypad_input = handle_joypad_input;

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