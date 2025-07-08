#ifndef GAMEBOY_APU_H
#define GAMEBOY_APU_H

#include "general.h"
#include <SDL3/SDL.h>
#include <stdint.h>
#include <stdbool.h>

extern struct EmulatorConfig config;

// APU debug print macros
#define APU_DEBUG_PRINT(fmt, ...)                                   \
    if (config.debug_mode && config.verbose_level >= DEBUG_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                    \
        PRINT_LEVEL(DEBUG_LEVEL);                                   \
        printf("APU: ");                                            \
        printf(fmt, ##__VA_ARGS__);                                 \
    }

#define APU_INFO_PRINT(fmt, ...)                                   \
    if (config.debug_mode && config.verbose_level >= INFO_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                   \
        PRINT_LEVEL(INFO_LEVEL);                                   \
        printf("APU: ");                                           \
        printf(fmt, ##__VA_ARGS__);                                \
    }

#define APU_WARN_PRINT(fmt, ...)                                   \
    if (config.debug_mode && config.verbose_level >= WARN_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                   \
        PRINT_LEVEL(WARN_LEVEL);                                   \
        printf("APU: ");                                           \
        printf(fmt, ##__VA_ARGS__);                                \
    }

#define APU_ERROR_PRINT(fmt, ...)   \
    {                               \
        PRINT_TIME_IN_SECONDS();    \
        PRINT_LEVEL(ERROR_LEVEL);   \
        printf("APU: ");            \
        printf(fmt, ##__VA_ARGS__); \
    }

#define APU_EMERGENCY_PRINT(fmt, ...) \
    {                                 \
        PRINT_TIME_IN_SECONDS();      \
        PRINT_LEVEL(EMERGENCY_LEVEL); \
        printf("APU: ");              \
        printf(fmt, ##__VA_ARGS__);   \
    }

// Game Boy APU constants
#define APU_SAMPLE_RATE 44100
#define APU_CHANNELS 2

// Sound register addresses
#define NR10_ADDRESS  0xFF10  // Channel 1 Sweep
#define NR11_ADDRESS  0xFF11  // Channel 1 Sound Length/Wave Pattern Duty
#define NR12_ADDRESS  0xFF12  // Channel 1 Volume Envelope
#define NR13_ADDRESS  0xFF13  // Channel 1 Frequency Lo
#define NR14_ADDRESS  0xFF14  // Channel 1 Frequency Hi

#define NR21_ADDRESS  0xFF16  // Channel 2 Sound Length/Wave Pattern Duty
#define NR22_ADDRESS  0xFF17  // Channel 2 Volume Envelope
#define NR23_ADDRESS  0xFF18  // Channel 2 Frequency Lo
#define NR24_ADDRESS  0xFF19  // Channel 2 Frequency Hi

#define NR30_ADDRESS  0xFF1A  // Channel 3 Sound On/Off
#define NR31_ADDRESS  0xFF1B  // Channel 3 Sound Length
#define NR32_ADDRESS  0xFF1C  // Channel 3 Select Output Level
#define NR33_ADDRESS  0xFF1D  // Channel 3 Frequency Lo
#define NR34_ADDRESS  0xFF1E  // Channel 3 Frequency Hi

#define NR41_ADDRESS  0xFF20  // Channel 4 Sound Length
#define NR42_ADDRESS  0xFF21  // Channel 4 Volume Envelope
#define NR43_ADDRESS  0xFF22  // Channel 4 Polynomial Counter
#define NR44_ADDRESS  0xFF23  // Channel 4 Counter/Consecutive

#define NR50_ADDRESS  0xFF24  // Master Volume & VIN Panning
#define NR51_ADDRESS  0xFF25  // Selection of Sound Output Terminal
#define NR52_ADDRESS  0xFF26  // Sound on/off

#define WAVE_RAM_START 0xFF30  // Wave pattern RAM
#define WAVE_RAM_END   0xFF3F

// Forward declaration
struct MMU;

// Enhanced square channel structure - with full Game Boy functionality
struct SimpleSquareChannel {
    // Register values (set by Game Boy)
    uint16_t frequency;     // 11-bit frequency register
    uint8_t duty;          // Duty cycle (0-3)
    bool enabled;          // Channel enabled
    bool dac_enabled;      // DAC enabled
    
    // Length timer
    uint8_t length_counter; // Length counter (0-63)
    bool length_enabled;    // Length counter enabled
    
    // Volume envelope
    uint8_t initial_volume; // Initial volume from register
    uint8_t volume;         // Current volume (0-15)
    bool envelope_add;      // Envelope direction (true = increase)
    uint8_t envelope_period; // Envelope period
    uint8_t envelope_counter; // Envelope counter
    
    // Frequency sweep (Channel 1 only)
    uint8_t sweep_period;   // Sweep period
    bool sweep_negate;      // Sweep direction (true = subtract)
    uint8_t sweep_shift;    // Sweep shift amount
    uint8_t sweep_counter;  // Sweep counter
    
    // Audio generation state (for callback)
    float phase;           // Phase for direct audio generation
    uint8_t duty_step;     // Current duty step
    
    // Panning
    bool left_enable;
    bool right_enable;
};

struct SimpleWaveChannel {
    // Register values
    uint16_t frequency;     // 11-bit frequency register
    uint8_t volume_shift;   // Volume shift (0-3)
    bool enabled;           // Channel enabled
    bool dac_enabled;       // DAC enabled
    uint8_t wave_ram[16];   // Wave pattern RAM
    
    // Length timer
    uint16_t length_counter; // Length counter (0-255, wave channel uses 8-bit)
    bool length_enabled;     // Length counter enabled
    
    // Audio generation state
    float phase;            // Phase for direct audio generation
    uint8_t sample_index;   // Current wave sample index
    
    // Panning
    bool left_enable;
    bool right_enable;
};

struct SimpleNoiseChannel {
    // Register values
    uint8_t shift_amount;   // Shift clock frequency
    uint8_t divisor_code;   // Clock divider code
    bool width_mode;        // LFSR width mode
    bool enabled;           // Channel enabled
    bool dac_enabled;       // DAC enabled
    
    // Length timer
    uint8_t length_counter; // Length counter (0-63)
    bool length_enabled;    // Length counter enabled
    
    // Volume envelope
    uint8_t initial_volume; // Initial volume from register
    uint8_t volume;         // Current volume (0-15)
    bool envelope_add;      // Envelope direction (true = increase)
    uint8_t envelope_period; // Envelope period
    uint8_t envelope_counter; // Envelope counter
    
    // Audio generation state
    float phase;            // Phase for direct audio generation
    uint16_t lfsr;          // Linear Feedback Shift Register
    
    // Panning
    bool left_enable;
    bool right_enable;
};

// Pure callback-driven APU structure
struct APU {
    // SDL3 Audio - callback-driven
    SDL_AudioDeviceID audio_device;
    SDL_AudioStream* audio_stream;
    SDL_AudioSpec audio_spec;
    
    // Channels - with full Game Boy functionality
    struct SimpleSquareChannel square1;
    struct SimpleSquareChannel square2;
    struct SimpleWaveChannel wave;
    struct SimpleNoiseChannel noise;
    
    // Frame sequencer (for callback timing)
    uint8_t frame_sequencer_step;      // Current frame sequencer step (0-7)
    float frame_sequencer_accumulator; // Accumulator for frame sequencer timing
    
    // Master control
    bool sound_enabled;
    uint8_t left_volume;    // 0-7
    uint8_t right_volume;   // 0-7
    
    // MMU for register access
    struct MMU* mmu;
    
    // Method pointers (compatible with old API)
    void (*step)(struct APU*, uint32_t cycles);  // NO-OP in callback-driven mode!
    void (*write_register)(struct APU*, uint16_t address, uint8_t value);
    uint8_t (*read_register)(struct APU*, uint16_t address);
};

// Function declarations

// APU lifecycle
struct APU* create_apu(void);
void free_apu(struct APU* apu);
void apu_attach_mmu(struct APU* apu, struct MMU* mmu);

// APU control - callback-driven!
void apu_write_register(struct APU* apu, uint16_t address, uint8_t value);
uint8_t apu_read_register(struct APU* apu, uint16_t address);

// SDL3 Audio Callback - handles ALL timing internally!
void apu_audio_callback(void* userdata, SDL_AudioStream* stream, 
                       int additional_amount, int total_amount);

#endif 