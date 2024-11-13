#ifndef GAMEBOY_TEST_H
#define GAMEBOY_TEST_H

#include <assert.h>
#include "../src/general.h"

struct EmulatorConfig config = {
    .debug_mode = true,
    .disable_color = true,
    .scale_factor = 1,
    .rom_path = NULL,
    .bootrom_path = NULL,
    .start_time = 0.0,
    .verbose_level = DEBUG_LEVEL
};

#endif
