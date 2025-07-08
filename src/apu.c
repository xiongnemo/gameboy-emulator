#include "apu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Game Boy duty cycle patterns
const uint8_t DUTY_PATTERNS[4][8] = {
    {0, 0, 0, 0, 0, 0, 0, 1}, // 12.5%
    {1, 0, 0, 0, 0, 0, 0, 1}, // 25%
    {1, 0, 0, 0, 0, 1, 1, 1}, // 50%
    {0, 1, 1, 1, 1, 1, 1, 0}  // 75%
};

// Noise divisor ratios
const uint32_t NOISE_DIVISORS[8] = {8, 16, 32, 48, 64, 80, 96, 112};

// Global APU instance for callback access
static struct APU* g_callback_apu = NULL;

// Helper function to convert samples to time-based ticks
static void update_frame_sequencer_timers(struct APU* apu, float samples_generated) {
    float time_elapsed = samples_generated / APU_SAMPLE_RATE;
    
    // Update frame sequencer accumulator (512 Hz)
    apu->frame_sequencer_accumulator += time_elapsed * 512.0f;
    
    // Process frame sequencer steps
    while (apu->frame_sequencer_accumulator >= 1.0f) {
        apu->frame_sequencer_accumulator -= 1.0f;
        
        // Frame sequencer runs at 512 Hz with 8 steps
        uint8_t step = apu->frame_sequencer_step;
        
        // Length counter updates (256 Hz) - steps 0, 2, 4, 6
        if ((step & 1) == 0) {
            // Square 1
            if (apu->square1.length_enabled && apu->square1.length_counter > 0) {
                apu->square1.length_counter--;
                if (apu->square1.length_counter == 0) {
                    apu->square1.enabled = false;
                    apu->square1.phase = 0.0f;
                    apu->square1.duty_step = 0;
                }
            }
            
            // Square 2
            if (apu->square2.length_enabled && apu->square2.length_counter > 0) {
                apu->square2.length_counter--;
                if (apu->square2.length_counter == 0) {
                    apu->square2.enabled = false;
                    apu->square2.phase = 0.0f;
                    apu->square2.duty_step = 0;
                }
            }
            
            // Wave
            if (apu->wave.length_enabled && apu->wave.length_counter > 0) {
                apu->wave.length_counter--;
                if (apu->wave.length_counter == 0) {
                    apu->wave.enabled = false;
                    apu->wave.phase = 0.0f;
                    apu->wave.sample_index = 0;
                }
            }
            
            // Noise
            if (apu->noise.length_enabled && apu->noise.length_counter > 0) {
                apu->noise.length_counter--;
                if (apu->noise.length_counter == 0) {
                    apu->noise.enabled = false;
                    apu->noise.phase = 0.0f;
                    apu->noise.lfsr = 0x7FFF;
                }
            }
        }
        
        // Envelope updates (64 Hz) - step 7 only
        if (step == 7) {
            // Square 1 envelope
            if (apu->square1.enabled && apu->square1.envelope_period > 0) {
                apu->square1.envelope_counter++;
                if (apu->square1.envelope_counter >= apu->square1.envelope_period) {
                    apu->square1.envelope_counter = 0;
                    if (apu->square1.envelope_add && apu->square1.volume < 15) {
                        apu->square1.volume++;
                    } else if (!apu->square1.envelope_add && apu->square1.volume > 0) {
                        apu->square1.volume--;
                    }
                }
            }
            
            // Square 2 envelope
            if (apu->square2.enabled && apu->square2.envelope_period > 0) {
                apu->square2.envelope_counter++;
                if (apu->square2.envelope_counter >= apu->square2.envelope_period) {
                    apu->square2.envelope_counter = 0;
                    if (apu->square2.envelope_add && apu->square2.volume < 15) {
                        apu->square2.volume++;
                    } else if (!apu->square2.envelope_add && apu->square2.volume > 0) {
                        apu->square2.volume--;
                    }
                }
            }
            
            // Noise envelope
            if (apu->noise.enabled && apu->noise.envelope_period > 0) {
                apu->noise.envelope_counter++;
                if (apu->noise.envelope_counter >= apu->noise.envelope_period) {
                    apu->noise.envelope_counter = 0;
                    if (apu->noise.envelope_add && apu->noise.volume < 15) {
                        apu->noise.volume++;
                    } else if (!apu->noise.envelope_add && apu->noise.volume > 0) {
                        apu->noise.volume--;
                    }
                }
            }
        }
        
        // Sweep updates (128 Hz) - steps 2, 6 only
        if (step == 2 || step == 6) {
            if (apu->square1.enabled && apu->square1.sweep_period > 0 && apu->square1.sweep_shift > 0) {
                apu->square1.sweep_counter++;
                if (apu->square1.sweep_counter >= apu->square1.sweep_period) {
                    apu->square1.sweep_counter = 0;
                    
                    uint16_t new_freq = apu->square1.frequency;
                    uint16_t delta = new_freq >> apu->square1.sweep_shift;
                    
                    if (apu->square1.sweep_negate) {
                        new_freq -= delta;
                    } else {
                        new_freq += delta;
                        // Overflow check
                        if (new_freq > 2047) {
                            apu->square1.enabled = false;
                            apu->square1.phase = 0.0f;
                            apu->square1.duty_step = 0;
                        }
                    }
                    
                    if (apu->square1.enabled) {
                        apu->square1.frequency = new_freq;
                    }
                }
            }
        }
        
        // Advance frame sequencer step
        apu->frame_sequencer_step = (apu->frame_sequencer_step + 1) & 7;
    }
}

// SDL3 Audio Callback - Pure callback-driven like successful tests!
void apu_audio_callback(void* userdata, SDL_AudioStream* stream, 
                       int additional_amount, int total_amount) {
    (void)userdata;
    (void)additional_amount;
    
    if (!g_callback_apu || !g_callback_apu->sound_enabled) {
        // Generate silence
        int16_t* silence = calloc(total_amount, 1);
        if (silence) {
            SDL_PutAudioStreamData(stream, silence, total_amount);
            free(silence);
        }
        return;
    }
    
    struct APU* apu = g_callback_apu;
    
    // Calculate samples needed (stereo 16-bit)
    int samples_needed = total_amount / (APU_CHANNELS * sizeof(int16_t));
    int16_t* buffer = malloc(total_amount);
    if (!buffer) return;
    
    // Update frame sequencer timers based on time elapsed
    update_frame_sequencer_timers(apu, (float)samples_needed);
    
    // Generate audio directly - authentic Game Boy style!
    for (int i = 0; i < samples_needed; i++) {
        float left_sample = 0.0f;
        float right_sample = 0.0f;
        
        // Square Channel 1 - with authentic Game Boy frequency calculation
        bool square1_active = apu->square1.enabled && apu->square1.dac_enabled && 
                              (!apu->square1.length_enabled || apu->square1.length_counter > 0);
        if (square1_active) {
            // Game Boy frequency: freq_hz = 131072 / (2048 - frequency)
            uint32_t gb_period = (2048 - apu->square1.frequency) * 4;
            float freq_hz = 4194304.0f / (float)gb_period;
            
            // Update phase and duty step
            apu->square1.phase += 2.0f * M_PI * freq_hz / APU_SAMPLE_RATE;
            if (apu->square1.phase >= 2.0f * M_PI) {
                apu->square1.phase -= 2.0f * M_PI;
                apu->square1.duty_step = (apu->square1.duty_step + 1) & 7;
            }
            
            // Generate square wave
            uint8_t duty_output = DUTY_PATTERNS[apu->square1.duty][apu->square1.duty_step];
            float ch1_sample = duty_output ? (apu->square1.volume / 15.0f) : 0.0f;
            
            if (apu->square1.left_enable) left_sample += ch1_sample;
            if (apu->square1.right_enable) right_sample += ch1_sample;
        }
        
        // Square Channel 2 - same as channel 1 but no sweep
        bool square2_active = apu->square2.enabled && apu->square2.dac_enabled && 
                              (!apu->square2.length_enabled || apu->square2.length_counter > 0);
        if (square2_active) {
            uint32_t gb_period = (2048 - apu->square2.frequency) * 4;
            float freq_hz = 4194304.0f / (float)gb_period;
            
            apu->square2.phase += 2.0f * M_PI * freq_hz / APU_SAMPLE_RATE;
            if (apu->square2.phase >= 2.0f * M_PI) {
                apu->square2.phase -= 2.0f * M_PI;
                apu->square2.duty_step = (apu->square2.duty_step + 1) & 7;
            }
            
            uint8_t duty_output = DUTY_PATTERNS[apu->square2.duty][apu->square2.duty_step];
            float ch2_sample = duty_output ? (apu->square2.volume / 15.0f) : 0.0f;
            
            if (apu->square2.left_enable) left_sample += ch2_sample;
            if (apu->square2.right_enable) right_sample += ch2_sample;
        }
        
        // Wave Channel 3 - custom waveform
        bool wave_active = apu->wave.enabled && apu->wave.dac_enabled && 
                           (!apu->wave.length_enabled || apu->wave.length_counter > 0);
        if (wave_active) {
            uint32_t gb_period = (2048 - apu->wave.frequency) * 2; // Wave channel runs at half rate
            float freq_hz = 4194304.0f / (float)gb_period;
            
            apu->wave.phase += 2.0f * M_PI * freq_hz / APU_SAMPLE_RATE;
            if (apu->wave.phase >= 2.0f * M_PI) {
                apu->wave.phase -= 2.0f * M_PI;
                apu->wave.sample_index = (apu->wave.sample_index + 1) & 31;
            }
            
            uint8_t wave_sample = apu->wave.wave_ram[apu->wave.sample_index / 2];
            wave_sample = (apu->wave.sample_index & 1) ? (wave_sample & 0xF) : (wave_sample >> 4);
            
            float ch3_sample = 0.0f;
            if (apu->wave.volume_shift > 0 && apu->wave.volume_shift <= 3) {
                ch3_sample = (wave_sample >> (apu->wave.volume_shift - 1)) / 15.0f;
            }
            
            if (apu->wave.left_enable) left_sample += ch3_sample;
            if (apu->wave.right_enable) right_sample += ch3_sample;
        }
        
        // Noise Channel 4 - LFSR-based noise
        bool noise_active = apu->noise.enabled && apu->noise.dac_enabled && 
                            (!apu->noise.length_enabled || apu->noise.length_counter > 0);
        if (noise_active) {
            uint32_t freq_timer = NOISE_DIVISORS[apu->noise.divisor_code] << apu->noise.shift_amount;
            float freq_hz = 262144.0f / freq_timer;
            
            apu->noise.phase += 2.0f * M_PI * freq_hz / APU_SAMPLE_RATE;
            if (apu->noise.phase >= 2.0f * M_PI) {
                apu->noise.phase -= 2.0f * M_PI;
                
                // Update LFSR
                uint16_t result = (apu->noise.lfsr & 1) ^ ((apu->noise.lfsr >> 1) & 1);
                apu->noise.lfsr >>= 1;
                apu->noise.lfsr |= (result << 14);
                if (apu->noise.width_mode) {
                    apu->noise.lfsr &= ~0x40;
                    apu->noise.lfsr |= (result << 6);
                }
            }
            
            float ch4_sample = (apu->noise.lfsr & 1) ? 0.0f : (apu->noise.volume / 15.0f);
            
            if (apu->noise.left_enable) left_sample += ch4_sample;
            if (apu->noise.right_enable) right_sample += ch4_sample;
        }
        
        // Apply master volume and scale for mixing
        left_sample *= ((apu->left_volume + 1) / 8.0f) * 0.25f;
        right_sample *= ((apu->right_volume + 1) / 8.0f) * 0.25f;
        
        // Convert to 16-bit signed
        int16_t final_left = (int16_t)(left_sample * 16384.0f);
        int16_t final_right = (int16_t)(right_sample * 16384.0f);
        
        // Clamp to 16-bit range
        if (final_left > 32767) final_left = 32767;
        if (final_left < -32768) final_left = -32768;
        if (final_right > 32767) final_right = 32767;
        if (final_right < -32768) final_right = -32768;
        
        buffer[i * 2] = final_left;
        buffer[i * 2 + 1] = final_right;
    }
    
    SDL_PutAudioStreamData(stream, buffer, total_amount);
    free(buffer);
}

// Register write function - handles all Game Boy APU registers
void apu_write_register(struct APU* apu, uint16_t address, uint8_t value) {
    if (!apu->sound_enabled && address != NR52_ADDRESS) {
        return; // Ignore writes when sound disabled, except to NR52
    }
    
    switch (address) {
        // Channel 1 - Square with sweep
        case NR10_ADDRESS: // Sweep
            apu->square1.sweep_period = (value >> 4) & 7;
            apu->square1.sweep_negate = (value & 8) != 0;
            apu->square1.sweep_shift = value & 7;
            break;
            
        case NR11_ADDRESS: // Length/Duty
            apu->square1.duty = (value >> 6) & 3;
            apu->square1.length_counter = 64 - (value & 0x3F);
            break;
            
        case NR12_ADDRESS: // Volume/Envelope
            apu->square1.initial_volume = (value >> 4) & 0xF;
            apu->square1.envelope_add = (value & 8) != 0;
            apu->square1.envelope_period = value & 7;
            apu->square1.dac_enabled = (value & 0xF8) != 0;
            if (!apu->square1.dac_enabled) {
                apu->square1.enabled = false;
            }
            break;
            
        case NR13_ADDRESS: // Frequency Low
            apu->square1.frequency = (apu->square1.frequency & 0x700) | value;
            break;
            
        case NR14_ADDRESS: // Frequency High/Control
            apu->square1.frequency = (apu->square1.frequency & 0xFF) | ((value & 7) << 8);
            apu->square1.length_enabled = (value & 0x40) != 0;
            if (value & 0x80) { // Trigger
                apu->square1.enabled = apu->square1.dac_enabled;
                apu->square1.phase = 0.0f;
                apu->square1.duty_step = 0;
                apu->square1.volume = apu->square1.initial_volume;
                apu->square1.envelope_counter = 0;
                apu->square1.sweep_counter = 0;
                if (apu->square1.length_counter == 0) {
                    apu->square1.length_counter = 64;
                }
            }
            break;
            
        // Channel 2 - Square (no sweep)
        case NR21_ADDRESS: // Length/Duty
            apu->square2.duty = (value >> 6) & 3;
            apu->square2.length_counter = 64 - (value & 0x3F);
            break;
            
        case NR22_ADDRESS: // Volume/Envelope
            apu->square2.initial_volume = (value >> 4) & 0xF;
            apu->square2.envelope_add = (value & 8) != 0;
            apu->square2.envelope_period = value & 7;
            apu->square2.dac_enabled = (value & 0xF8) != 0;
            if (!apu->square2.dac_enabled) {
                apu->square2.enabled = false;
            }
            break;
            
        case NR23_ADDRESS: // Frequency Low
            apu->square2.frequency = (apu->square2.frequency & 0x700) | value;
            break;
            
        case NR24_ADDRESS: // Frequency High/Control
            apu->square2.frequency = (apu->square2.frequency & 0xFF) | ((value & 7) << 8);
            apu->square2.length_enabled = (value & 0x40) != 0;
            if (value & 0x80) { // Trigger
                apu->square2.enabled = apu->square2.dac_enabled;
                apu->square2.phase = 0.0f;
                apu->square2.duty_step = 0;
                apu->square2.volume = apu->square2.initial_volume;
                apu->square2.envelope_counter = 0;
                if (apu->square2.length_counter == 0) {
                    apu->square2.length_counter = 64;
                }
            }
            break;
            
        // Channel 3 - Wave
        case NR30_ADDRESS: // DAC Enable
            apu->wave.dac_enabled = (value & 0x80) != 0;
            if (!apu->wave.dac_enabled) {
                apu->wave.enabled = false;
            }
            break;
            
        case NR31_ADDRESS: // Length
            apu->wave.length_counter = 256 - value;
            break;
            
        case NR32_ADDRESS: // Volume
            apu->wave.volume_shift = (value >> 5) & 3;
            break;
            
        case NR33_ADDRESS: // Frequency Low
            apu->wave.frequency = (apu->wave.frequency & 0x700) | value;
            break;
            
        case NR34_ADDRESS: // Frequency High/Control
            apu->wave.frequency = (apu->wave.frequency & 0xFF) | ((value & 7) << 8);
            apu->wave.length_enabled = (value & 0x40) != 0;
            if (value & 0x80) { // Trigger
                apu->wave.enabled = apu->wave.dac_enabled;
                apu->wave.phase = 0.0f;
                apu->wave.sample_index = 0;
                if (apu->wave.length_counter == 0) {
                    apu->wave.length_counter = 256;
                }
            }
            break;
            
        // Channel 4 - Noise
        case NR41_ADDRESS: // Length
            apu->noise.length_counter = 64 - (value & 0x3F);
            break;
            
        case NR42_ADDRESS: // Volume/Envelope
            apu->noise.initial_volume = (value >> 4) & 0xF;
            apu->noise.envelope_add = (value & 8) != 0;
            apu->noise.envelope_period = value & 7;
            apu->noise.dac_enabled = (value & 0xF8) != 0;
            if (!apu->noise.dac_enabled) {
                apu->noise.enabled = false;
            }
            break;
            
        case NR43_ADDRESS: // Frequency/Random
            apu->noise.shift_amount = (value >> 4) & 0xF;
            apu->noise.width_mode = (value & 0x08) != 0;
            apu->noise.divisor_code = value & 7;
            break;
            
        case NR44_ADDRESS: // Control
            apu->noise.length_enabled = (value & 0x40) != 0;
            if (value & 0x80) { // Trigger
                apu->noise.enabled = apu->noise.dac_enabled;
                apu->noise.phase = 0.0f;
                apu->noise.lfsr = 0x7FFF;
                apu->noise.volume = apu->noise.initial_volume;
                apu->noise.envelope_counter = 0;
                if (apu->noise.length_counter == 0) {
                    apu->noise.length_counter = 64;
                }
            }
            break;
            
        // Master controls
        case NR50_ADDRESS: // Master Volume
            apu->left_volume = (value >> 4) & 7;
            apu->right_volume = value & 7;
            break;
            
        case NR51_ADDRESS: // Panning
            apu->square1.left_enable = (value & 0x10) != 0;
            apu->square1.right_enable = (value & 0x01) != 0;
            apu->square2.left_enable = (value & 0x20) != 0;
            apu->square2.right_enable = (value & 0x02) != 0;
            apu->wave.left_enable = (value & 0x40) != 0;
            apu->wave.right_enable = (value & 0x04) != 0;
            apu->noise.left_enable = (value & 0x80) != 0;
            apu->noise.right_enable = (value & 0x08) != 0;
            break;
            
        case NR52_ADDRESS: // Sound Enable
            {
                bool old_enabled = apu->sound_enabled;
                apu->sound_enabled = (value & 0x80) != 0;
                
                if (old_enabled && !apu->sound_enabled) {
                    // Sound disabled - clear all channels
                    APU_DEBUG_PRINT("Sound disabled\n");
                    memset(&apu->square1, 0, sizeof(apu->square1));
                    memset(&apu->square2, 0, sizeof(apu->square2));
                    memset(&apu->wave, 0, sizeof(apu->wave));
                    memset(&apu->noise, 0, sizeof(apu->noise));
                    apu->frame_sequencer_step = 0;
                    apu->frame_sequencer_accumulator = 0.0f;
                } else if (!old_enabled && apu->sound_enabled) {
                    APU_DEBUG_PRINT("Sound enabled\n");
                }
            }
            break;
            
        // Wave RAM
        default:
            if (address >= WAVE_RAM_START && address <= WAVE_RAM_END) {
                apu->wave.wave_ram[address - WAVE_RAM_START] = value;
            }
            break;
    }
}

// Register read function
uint8_t apu_read_register(struct APU* apu, uint16_t address) {
    switch (address) {
        case NR10_ADDRESS:
            return 0x80 | (apu->square1.sweep_period << 4) | 
                   (apu->square1.sweep_negate ? 8 : 0) | apu->square1.sweep_shift;
            
        case NR11_ADDRESS:
            return 0x3F | (apu->square1.duty << 6);
            
        case NR12_ADDRESS:
            return (apu->square1.initial_volume << 4) | 
                   (apu->square1.envelope_add ? 8 : 0) | apu->square1.envelope_period;
            
        case NR13_ADDRESS:
            return 0xFF; // Write only
            
        case NR14_ADDRESS:
            return 0xBF | (apu->square1.length_enabled ? 0x40 : 0);
            
        case NR21_ADDRESS:
            return 0x3F | (apu->square2.duty << 6);
            
        case NR22_ADDRESS:
            return (apu->square2.initial_volume << 4) | 
                   (apu->square2.envelope_add ? 8 : 0) | apu->square2.envelope_period;
            
        case NR23_ADDRESS:
            return 0xFF; // Write only
            
        case NR24_ADDRESS:
            return 0xBF | (apu->square2.length_enabled ? 0x40 : 0);
            
        case NR30_ADDRESS:
            return 0x7F | (apu->wave.dac_enabled ? 0x80 : 0);
            
        case NR31_ADDRESS:
            return 0xFF; // Write only
            
        case NR32_ADDRESS:
            return 0x9F | (apu->wave.volume_shift << 5);
            
        case NR33_ADDRESS:
            return 0xFF; // Write only
            
        case NR34_ADDRESS:
            return 0xBF | (apu->wave.length_enabled ? 0x40 : 0);
            
        case NR41_ADDRESS:
            return 0xFF; // Write only
            
        case NR42_ADDRESS:
            return (apu->noise.initial_volume << 4) | 
                   (apu->noise.envelope_add ? 8 : 0) | apu->noise.envelope_period;
            
        case NR43_ADDRESS:
            return (apu->noise.shift_amount << 4) | (apu->noise.width_mode ? 0x08 : 0) |
                   apu->noise.divisor_code;
            
        case NR44_ADDRESS:
            return 0xBF | (apu->noise.length_enabled ? 0x40 : 0);
            
        case NR50_ADDRESS:
            return (apu->left_volume << 4) | apu->right_volume;
            
        case NR51_ADDRESS:
            return (apu->square1.left_enable ? 0x10 : 0) | (apu->square1.right_enable ? 0x01 : 0) |
                   (apu->square2.left_enable ? 0x20 : 0) | (apu->square2.right_enable ? 0x02 : 0) |
                   (apu->wave.left_enable ? 0x40 : 0) | (apu->wave.right_enable ? 0x04 : 0) |
                   (apu->noise.left_enable ? 0x80 : 0) | (apu->noise.right_enable ? 0x08 : 0);
            
        case NR52_ADDRESS:
            return (apu->sound_enabled ? 0x80 : 0) | 0x70 |
                   (apu->square1.enabled ? 0x01 : 0) |
                   (apu->square2.enabled ? 0x02 : 0) |
                   (apu->wave.enabled ? 0x04 : 0) |
                   (apu->noise.enabled ? 0x08 : 0);
            
        default:
            if (address >= WAVE_RAM_START && address <= WAVE_RAM_END) {
                return apu->wave.wave_ram[address - WAVE_RAM_START];
            }
            return 0xFF;
    }
}

// Create APU - pure callback-driven
struct APU* create_apu(void) {
    struct APU* apu = (struct APU*)malloc(sizeof(struct APU));
    if (!apu) {
        APU_EMERGENCY_PRINT("Failed to allocate memory for APU\n");
        return NULL;
    }
    
    memset(apu, 0, sizeof(struct APU));
    
    // Set global reference for callback access
    g_callback_apu = apu;
    
    // Initialize SDL3 Audio
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        APU_WARN_PRINT("Failed to initialize SDL Audio: %s\n", SDL_GetError());
        APU_WARN_PRINT("Continuing in silent mode\n");
        apu->audio_device = 0;
    } else {
        SDL_AudioSpec desired_spec = {0};
        desired_spec.freq = APU_SAMPLE_RATE;
        desired_spec.format = SDL_AUDIO_S16;
        desired_spec.channels = APU_CHANNELS;
        
        apu->audio_device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &desired_spec);
        if (apu->audio_device == 0) {
            APU_WARN_PRINT("Failed to open audio device: %s\n", SDL_GetError());
            apu->audio_stream = NULL;
        } else {
            apu->audio_spec = desired_spec;
            apu->audio_stream = SDL_CreateAudioStream(&desired_spec, &desired_spec);
            
            if (!apu->audio_stream || !SDL_BindAudioStream(apu->audio_device, apu->audio_stream)) {
                APU_WARN_PRINT("Failed to create/bind audio stream: %s\n", SDL_GetError());
                if (apu->audio_stream) SDL_DestroyAudioStream(apu->audio_stream);
                SDL_CloseAudioDevice(apu->audio_device);
                apu->audio_device = 0;
                apu->audio_stream = NULL;
            } else {
                // Set up callback
                if (!SDL_SetAudioStreamGetCallback(apu->audio_stream, apu_audio_callback, apu)) {
                    APU_WARN_PRINT("Failed to set audio callback: %s\n", SDL_GetError());
                    SDL_DestroyAudioStream(apu->audio_stream);
                    SDL_CloseAudioDevice(apu->audio_device);
                    apu->audio_device = 0;
                    apu->audio_stream = NULL;
                } else if (!SDL_ResumeAudioDevice(apu->audio_device)) {
                    APU_WARN_PRINT("Failed to resume audio device: %s\n", SDL_GetError());
                    SDL_DestroyAudioStream(apu->audio_stream);
                    SDL_CloseAudioDevice(apu->audio_device);
                    apu->audio_device = 0;
                    apu->audio_stream = NULL;
                } else {
                    APU_INFO_PRINT("APU audio callback ready.\n");
                }
            }
        }
    }
    
    // Initialize function pointers (compatible with existing integration)
    apu->write_register = apu_write_register;
    apu->read_register = apu_read_register;
    
    // Initialize frame sequencer
    apu->frame_sequencer_step = 0;
    apu->frame_sequencer_accumulator = 0.0f;
    
    // Initialize default values
    apu->left_volume = 7;
    apu->right_volume = 7;
    apu->noise.lfsr = 0x7FFF;
    
    if (apu->audio_device == 0) {
        APU_INFO_PRINT("APU initialized (silent mode)\n");
    }
    
    return apu;
}

// Attach MMU to APU
void apu_attach_mmu(struct APU* apu, struct MMU* mmu) {
    if (apu) {
        apu->mmu = mmu;
    }
}

// Free APU
void free_apu(struct APU* apu) {
    if (apu) {
        if (apu == g_callback_apu) {
            g_callback_apu = NULL;
        }
        
        if (apu->audio_device != 0) {
            SDL_PauseAudioDevice(apu->audio_device);
            if (apu->audio_stream) {
                SDL_DestroyAudioStream(apu->audio_stream);
            }
            SDL_CloseAudioDevice(apu->audio_device);
        }
        free(apu);
    }
} 