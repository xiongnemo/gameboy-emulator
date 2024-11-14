#ifndef GAMEBOY_GENERAL_H
#define GAMEBOY_GENERAL_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <string.h>
#include <math.h>

#ifdef _WIN32
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif

struct EmulatorConfig {
    char* rom_path;
    char* bootrom_path;
    bool debug_mode;
    int scale_factor;
    double start_time;
    bool disable_color;
    int verbose_level;
};

extern struct EmulatorConfig config;

#define EMERGENCY_LEVEL -2
#define ERROR_LEVEL -1
#define WARN_LEVEL 0
#define INFO_LEVEL 1
#define DEBUG_LEVEL 2
#define TRACE_LEVEL 3

#define UNDEFINED 0xFF

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
#define ANSI_BOLD       "\x1b[1m"
#define ANSI_UNDERLINE  "\x1b[4m"
#define ANSI_BLINK      "\x1b[5m"
#define ANSI_REVERSE    "\x1b[7m"
#define ANSI_HIDDEN     "\x1b[8m"

// Reset
#define ANSI_COLOR_RESET "\x1b[0m"

// Get time in seconds
static inline double get_time_in_seconds() {
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
#define PRINT_TIME_IN_SECONDS() \
    if (config.disable_color) { \
        printf("[%010.3f] ", get_time_in_seconds() - config.start_time); \
    } else { \
        printf("[" ANSI_COLOR_BRIGHT_GREEN "%010.3f" ANSI_COLOR_RESET "] ", get_time_in_seconds() - config.start_time); \
    }

// 4 types of print level
// Info: white [INFO]
// Debug: green [DBG]
// Trace: blue [TRC]
// Warn: yellow [WRN]
#define PRINT_LEVEL(level) \
    if (config.disable_color) { \
        if (level == INFO_LEVEL) { \
            printf("[INF] "); \
        } else if (level == DEBUG_LEVEL) { \
            printf("[DBG] "); \
        } else if (level == TRACE_LEVEL) { \
            printf("[TRC] "); \
        } else if (level == WARN_LEVEL) { \
            printf("[WRN] "); \
        } else if (level == EMERGENCY_LEVEL) { \
            printf("[EMG] "); \
        } else if (level == ERROR_LEVEL) { \
            printf("[ERR] "); \
        } \
    } else { \
        if (level == INFO_LEVEL) { \
            printf(ANSI_COLOR_WHITE "[INF]" ANSI_COLOR_RESET " "); \
        } else if (level == DEBUG_LEVEL) { \
            printf(ANSI_COLOR_GREEN "[DBG]" ANSI_COLOR_RESET " "); \
        } else if (level == TRACE_LEVEL) { \
            printf(ANSI_COLOR_BLUE "[TRC]" ANSI_COLOR_RESET " "); \
        } else if (level == WARN_LEVEL) { \
            printf(ANSI_COLOR_YELLOW "[WRN]" ANSI_COLOR_RESET " "); \
        } else if (level == EMERGENCY_LEVEL) { \
            printf(ANSI_COLOR_RED "[EMG]" ANSI_COLOR_RESET " "); \
        } else if (level == ERROR_LEVEL) { \
            printf(ANSI_COLOR_BRIGHT_RED "[ERR]" ANSI_COLOR_RESET " "); \
        } \
    }

#endif
