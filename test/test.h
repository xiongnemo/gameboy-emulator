#ifndef GAMEBOY_TEST_H
#define GAMEBOY_TEST_H

#include <assert.h>
#include "../src/general.h"

struct EmulatorGlobals globals = {
    .is_stdout_redirected = false,
};

struct EmulatorConfig config = {
    .debug_mode = true,
    .disable_color = false,
    .scale_factor = 1,
    .rom_path = NULL,
    .bootrom_path = NULL,
    .start_time = 0.0,
    .verbose_level = DEBUG_LEVEL,
    .globals = &globals,
};

#endif
