#ifndef GAMEBOY_GENERAL_H
#define GAMEBOY_GENERAL_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include <string.h>

#include <time.h>

#ifdef _WIN32
#    include <sys/timeb.h>
#else
#    include <sys/time.h>
#endif

struct EmulatorGlobals
{
    bool is_stdout_redirected;
};

struct EmulatorConfig
{
    char*                   rom_path;
    char*                   bootrom_path;
    bool                    debug_mode;
    int                     scale_factor;
    double                  start_time;
    bool                    disable_color;
    int                     verbose_level;
    struct EmulatorGlobals* globals;
    bool                    enable_serial_print;
    bool                    print_debug_info_this_frame;
    bool                    fast_forward_mode;
    bool                    disable_joypad;
};


extern struct EmulatorConfig config;

#define EMERGENCY_LEVEL -2
#define ERROR_LEVEL     -1
#define WARN_LEVEL      0
#define INFO_LEVEL      1
#define DEBUG_LEVEL     2
#define TRACE_LEVEL     3

#define UNDEFINED 0xFF

// RAM Registers

#define ZERO_PAGE_ADDRESS 0xFF00
#define JOYPAD_ADDRESS 0xFF00

// Interrupt flags

// Bit 0: V-Blank
#define INT_VBLANK   0x01
// Bit 1: LCDC (see STAT)
#define INT_LCD_STAT 0x02
// Bit 2: Timer Overflow
#define INT_TIMER    0x04
// Bit 3: Serial I/O transfer complete
#define INT_SERIAL   0x08
// Bit 4: Transition from High to Low of Pin number P10-P13
#define INT_JOYPAD   0x10

// IF - Interrupt Flag
// Bit 4: Transition from High to Low of Pin number P10-P13
// Bit 3: Serial I/O transfer complete
// Bit 2: Timer Overflow
// Bit 1: LCDC (see STAT)
// Bit 0: V-Blank
#define IF_ADDRESS 0xFF0F

// IE - Interrupt Enable
// Bit 4: Transition from High to Low of Pin number P10-P13
// Bit 3: Serial I/O transfer complete
// Bit 2: Timer Overflow
// Bit 1: LCDC (see STAT)
// Bit 0: V-Blank
// 0: disable
// 1: enable
#define IE_ADDRESS 0xFFFF


// LCDC - LCD Control
// Bit 7: LCD Display Enable (0=Off, 1=On)
// Bit 6: Window Tile Map Display Select (0=9800-9BFF, 1=9C00-9FFF)
// Bit 5: Window Display Enable (0=Off, 1=On)
// Bit 4: BG & Window Tile Data Select (0=8800-97FF, 1=8000-8FFF)
// Bit 3: BG Tile Map Display Select (0=9800-9BFF, 1=9C00-9FFF)
// Bit 2: OBJ (Sprite) Size (0=8x8, 1=8x16)
// Bit 1: OBJ (Sprite) Display Enable (0=Off, 1=On)
// Bit 0: BG Display Enable (0=Off, 1=On)
#define LCDC_ADDRESS 0xFF40
// STAT - LCD Status
// Bits 6-3 - Interrupt Selection By LCDC Status
// Bit 6 - LYC=LY Coincidence (Selectable)
// Bit 5 - Mode 10
// Bit 4 - Mode 01
// Bit 3 - Mode 00
//    - 0: Non Selection
//    - 1: Selection
// Bit 2 - Coincidence Flag
//    - 0: LYC not equal to LCDC LY
//    - 1: LYC = LCDC LY
// Bit 1-0 - Mode Flag
//    - 00: During H-Blank
//    - 01: During V-Blank
//    - 10: During Searching OAM-RAM
//    - 11: During Transfering Data to LCD Driver
#define STAT_ADDRESS 0xFF41
// LCD Position and Scrolling Registers
#define SCY_ADDRESS  0xFF42  // Scroll Y (R/W) - Background vertical scroll position
#define SCX_ADDRESS  0xFF43  // Scroll X (R/W) - Background horizontal scroll position  
#define LY_ADDRESS   0xFF44  // LCD Y-Coordinate (R) - Current scanline being drawn (0-153)
#define LYC_ADDRESS  0xFF45  // LY Compare (R/W) - Triggers STAT interrupt when LY == LYC
#define DMA_ADDRESS  0xFF46  // DMA Transfer (W) - Starts DMA transfer from ROM/RAM to OAM
#define BGP_ADDRESS  0xFF47  // BG Palette Data (R/W) - Background palette (bits 1-0: color 0, 3-2: color 1, etc.)
#define OBP0_ADDRESS 0xFF48  // Object Palette 0 Data (R/W) - Sprite palette 0 (color 0 is transparent)
#define OBP1_ADDRESS 0xFF49  // Object Palette 1 Data (R/W) - Sprite palette 1 (color 0 is transparent)
#define WY_ADDRESS   0xFF4A  // Window Y Position (R/W) - Window top edge position
#define WX_ADDRESS   0xFF4B  // Window X Position (R/W) - Window left edge position minus 7

// Timer Registers
#define TIMER_DIV_ADDRESS  0xFF04  // Divider Register (R/W) - Incremented at 16384Hz, writing resets to 0
#define TIMER_TIMA_ADDRESS 0xFF05  // Timer Counter (R/W) - Incremented by frequency in TAC, triggers interrupt on overflow
#define TIMER_TMA_ADDRESS  0xFF06  // Timer Modulo (R/W) - Value loaded into TIMA when it overflows
#define TIMER_TAC_ADDRESS  0xFF07  // Timer Control (R/W) - Bit 2: Enable, Bits 1-0: Clock select (00=4096Hz, 01=262144Hz, 10=65536Hz, 11=16384Hz)


// Regular Colors
#define ANSI_COLOR_BLACK   "\x1b[30m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_WHITE   "\x1b[37m"

// Bold/Bright Colors
#define ANSI_COLOR_BRIGHT_BLACK   "\x1b[90m"
#define ANSI_COLOR_BRIGHT_RED     "\x1b[91m"
#define ANSI_COLOR_BRIGHT_GREEN   "\x1b[92m"
#define ANSI_COLOR_BRIGHT_YELLOW  "\x1b[93m"
#define ANSI_COLOR_BRIGHT_BLUE    "\x1b[94m"
#define ANSI_COLOR_BRIGHT_MAGENTA "\x1b[95m"
#define ANSI_COLOR_BRIGHT_CYAN    "\x1b[96m"
#define ANSI_COLOR_BRIGHT_WHITE   "\x1b[97m"

// Background Colors
#define ANSI_BG_BLACK   "\x1b[40m"
#define ANSI_BG_RED     "\x1b[41m"
#define ANSI_BG_GREEN   "\x1b[42m"
#define ANSI_BG_YELLOW  "\x1b[43m"
#define ANSI_BG_BLUE    "\x1b[44m"
#define ANSI_BG_MAGENTA "\x1b[45m"
#define ANSI_BG_CYAN    "\x1b[46m"
#define ANSI_BG_WHITE   "\x1b[47m"

// Text Styles
#define ANSI_BOLD      "\x1b[1m"
#define ANSI_UNDERLINE "\x1b[4m"
#define ANSI_BLINK     "\x1b[5m"
#define ANSI_REVERSE   "\x1b[7m"
#define ANSI_HIDDEN    "\x1b[8m"

// Reset
#define ANSI_COLOR_RESET "\x1b[0m"

// are stdout redirected?
#ifdef _WIN32
#    include <io.h>
#    define isatty _isatty
#    ifndef STDOUT_FILENO
#        define STDOUT_FILENO _fileno(stdout)
#    endif
#else
#    include <unistd.h>
#endif

// Add this function
static inline bool is_stdout_redirected()
{
    return !isatty(STDOUT_FILENO);
}

// Get time in seconds
static inline double get_time_in_seconds()
{
#ifdef _WIN32
    struct timeb start_time;
    ftime(&start_time);
    return ((double)start_time.time * 1000.0 + (double)start_time.millitm) / 1000.0;
#else
    struct timeval start_time;
    gettimeofday(&start_time, NULL);
    return ((double)start_time.tv_sec * 1000.0 + (double)start_time.tv_usec / 1000.0) / 1000.0;
#endif
}

// Print time in seconds, with color
// 6 digit for integer part, 3 digit for decimal part
// 6 + 1 + 3 = 8
// fill with space if less than 5 digit
// if stdout is redirected, don't print color;
// otherwise, use config.disable_color to decide whether to print color
#define PRINT_TIME_IN_SECONDS()                                          \
    if (config.globals->is_stdout_redirected || config.disable_color) { \
        printf("[%014.3f] ", get_time_in_seconds() - config.start_time); \
    }                                                                    \
    else {                                                               \
        printf(                                                          \
            "[" ANSI_COLOR_BRIGHT_GREEN "%014.3f" ANSI_COLOR_RESET "] ", \
            get_time_in_seconds() - config.start_time);                  \
    }

// 4 types of print level
// Info: white [INFO]
// Debug: green [DBG]
// Trace: blue [TRC]
// Warn: yellow [WRN]
// if stdout is redirected, don't print color;
// otherwise, use config.disable_color to decide whether to print color
#define PRINT_LEVEL(level)                                              \
    if (config.globals->is_stdout_redirected || config.disable_color) { \
        if (level == INFO_LEVEL) {                                      \
            printf("[INF] ");                                           \
        }                                                               \
        else if (level == DEBUG_LEVEL) {                                \
            printf("[DBG] ");                                           \
        }                                                               \
        else if (level == TRACE_LEVEL) {                                \
            printf("[TRC] ");                                           \
        }                                                               \
        else if (level == WARN_LEVEL) {                                 \
            printf("[WRN] ");                                           \
        }                                                               \
        else if (level == EMERGENCY_LEVEL) {                            \
            printf("[EMG] ");                                           \
        }                                                               \
        else if (level == ERROR_LEVEL) {                                \
            printf("[ERR] ");                                           \
        }                                                               \
    }                                                                   \
    else {                                                              \
        if (level == INFO_LEVEL) {                                      \
            printf(ANSI_COLOR_WHITE "[INF]" ANSI_COLOR_RESET " ");      \
        }                                                               \
        else if (level == DEBUG_LEVEL) {                                \
            printf(ANSI_COLOR_GREEN "[DBG]" ANSI_COLOR_RESET " ");      \
        }                                                               \
        else if (level == TRACE_LEVEL) {                                \
            printf(ANSI_COLOR_BLUE "[TRC]" ANSI_COLOR_RESET " ");       \
        }                                                               \
        else if (level == WARN_LEVEL) {                                 \
            printf(ANSI_COLOR_YELLOW "[WRN]" ANSI_COLOR_RESET " ");     \
        }                                                               \
        else if (level == EMERGENCY_LEVEL) {                            \
            printf(ANSI_COLOR_RED "[EMG]" ANSI_COLOR_RESET " ");        \
        }                                                               \
        else if (level == ERROR_LEVEL) {                                \
            printf(ANSI_COLOR_BRIGHT_RED "[ERR]" ANSI_COLOR_RESET " "); \
        }                                                               \
    }

#endif
