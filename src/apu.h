#ifndef GAMEBOY_APU_H
#define GAMEBOY_APU_H

#include "general.h"
#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>

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
#define APU_SAMPLE_RATE        44100
#define APU_CHANNELS           2
#define APU_CPU_M_CYCLE_HZ     GB_M_CYCLE_HZ
#define APU_FRAME_SEQ_PERIOD   2048
#define APU_RING_BUFFER_FRAMES 4096

// Sound register addresses
#define NR10_ADDRESS 0xFF10  // Channel 1 Sweep
#define NR11_ADDRESS 0xFF11  // Channel 1 Sound Length/Wave Pattern Duty
#define NR12_ADDRESS 0xFF12  // Channel 1 Volume Envelope
#define NR13_ADDRESS 0xFF13  // Channel 1 Frequency Lo
#define NR14_ADDRESS 0xFF14  // Channel 1 Frequency Hi

#define NR21_ADDRESS 0xFF16  // Channel 2 Sound Length/Wave Pattern Duty
#define NR22_ADDRESS 0xFF17  // Channel 2 Volume Envelope
#define NR23_ADDRESS 0xFF18  // Channel 2 Frequency Lo
#define NR24_ADDRESS 0xFF19  // Channel 2 Frequency Hi

#define NR30_ADDRESS 0xFF1A  // Channel 3 Sound On/Off
#define NR31_ADDRESS 0xFF1B  // Channel 3 Sound Length
#define NR32_ADDRESS 0xFF1C  // Channel 3 Select Output Level
#define NR33_ADDRESS 0xFF1D  // Channel 3 Frequency Lo
#define NR34_ADDRESS 0xFF1E  // Channel 3 Frequency Hi

#define NR41_ADDRESS 0xFF20  // Channel 4 Sound Length
#define NR42_ADDRESS 0xFF21  // Channel 4 Volume Envelope
#define NR43_ADDRESS 0xFF22  // Channel 4 Polynomial Counter
#define NR44_ADDRESS 0xFF23  // Channel 4 Counter/Consecutive

#define NR50_ADDRESS 0xFF24  // Master Volume & VIN Panning
#define NR51_ADDRESS 0xFF25  // Selection of Sound Output Terminal
#define NR52_ADDRESS 0xFF26  // Sound on/off

#define WAVE_RAM_START 0xFF30  // Wave pattern RAM
#define WAVE_RAM_END   0xFF3F

// Forward declaration
struct MMU;

struct SimpleSquareChannel
{
    uint16_t frequency;       // 11-bit period value
    uint16_t period_timer;    // M-cycles until next duty step
    uint8_t  duty;            // Duty cycle (0-3)
    uint8_t  duty_step;       // Current duty step (0-7)
    bool     enabled;
    bool     dac_enabled;

    uint8_t length_counter;
    bool    length_enabled;

    uint8_t initial_volume;
    uint8_t volume;
    bool    envelope_add;
    uint8_t envelope_period;
    uint8_t envelope_counter;

    uint8_t sweep_period;
    bool    sweep_negate;
    uint8_t sweep_shift;
    uint8_t sweep_counter;

    bool left_enable;
    bool right_enable;
};

struct SimpleWaveChannel
{
    uint16_t frequency;       // 11-bit period value
    uint16_t period_timer;    // Half-M-cycle ticks until next wave sample
    uint8_t  volume_shift;    // 0=mute, 1=100%, 2=50%, 3=25%
    bool     enabled;
    bool     dac_enabled;
    uint8_t  wave_ram[16];

    uint16_t length_counter;
    bool     length_enabled;

    uint8_t sample_index;     // Current wave sample index (0-31)

    bool left_enable;
    bool right_enable;
};

struct SimpleNoiseChannel
{
    uint32_t period_timer;    // Dot-clock ticks until next LFSR update
    uint8_t  shift_amount;
    uint8_t  divisor_code;
    bool     width_mode;
    bool     enabled;
    bool     dac_enabled;

    uint8_t length_counter;
    bool    length_enabled;

    uint8_t initial_volume;
    uint8_t volume;
    bool    envelope_add;
    uint8_t envelope_period;
    uint8_t envelope_counter;

    uint16_t lfsr;

    bool left_enable;
    bool right_enable;
};

struct APU
{
    SDL_AudioDeviceID audio_device;
    SDL_AudioStream*  audio_stream;
    SDL_AudioSpec     audio_spec;
    bool              owns_audio_subsystem;

    struct SimpleSquareChannel square1;
    struct SimpleSquareChannel square2;
    struct SimpleWaveChannel   wave;
    struct SimpleNoiseChannel  noise;

    uint8_t  frame_sequencer_step;
    uint16_t frame_sequencer_counter;

    bool    sound_enabled;
    uint8_t left_volume;
    uint8_t right_volume;

    int16_t  sample_buffer[APU_RING_BUFFER_FRAMES * APU_CHANNELS];
    uint32_t sample_read_index;
    uint32_t sample_write_index;
    uint32_t sample_count;
    uint32_t sample_cycle_accumulator;
    uint64_t total_samples_generated;
    uint64_t total_samples_dropped;

    struct MMU* mmu;

    void (*step)(struct APU*, uint8_t m_cycles);
    void (*write_register)(struct APU*, uint16_t address, uint8_t value);
    uint8_t (*read_register)(struct APU*, uint16_t address);
};

// APU lifecycle
struct APU* create_apu(void);
void        free_apu(struct APU* apu);
void        apu_attach_mmu(struct APU* apu, struct MMU* mmu);

// APU core and audio output
void apu_step(struct APU* apu, uint8_t m_cycles);
bool apu_start_audio(struct APU* apu);
void apu_stop_audio(struct APU* apu);

// Register access
void    apu_write_register(struct APU* apu, uint16_t address, uint8_t value);
uint8_t apu_read_register(struct APU* apu, uint16_t address);

// SDL3 Audio Callback: drains generated PCM only.
void apu_audio_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount);

#endif
