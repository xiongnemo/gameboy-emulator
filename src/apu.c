#include "apu.h"
#include <stdlib.h>
#include <string.h>

static const uint8_t DUTY_PATTERNS[4][8] = {
    {0, 0, 0, 0, 0, 0, 0, 1}, // 12.5%
    {1, 0, 0, 0, 0, 0, 0, 1}, // 25%
    {1, 0, 0, 0, 0, 1, 1, 1}, // 50%
    {0, 1, 1, 1, 1, 1, 1, 0}  // 75%
};

static const uint32_t NOISE_DIVISORS[8] = {8, 16, 32, 48, 64, 80, 96, 112};

static uint16_t pulse_reload(const struct SimpleSquareChannel* channel)
{
    return (uint16_t)(2048 - (channel->frequency & 0x7FF));
}

static uint16_t wave_reload(const struct SimpleWaveChannel* channel)
{
    return (uint16_t)(2048 - (channel->frequency & 0x7FF));
}

static uint32_t noise_reload(const struct SimpleNoiseChannel* channel)
{
    if (channel->shift_amount >= 14) {
        return 0;
    }
    return NOISE_DIVISORS[channel->divisor_code & 7] << channel->shift_amount;
}

static int32_t clamp_i32(int32_t value, int32_t low, int32_t high)
{
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

static void apu_reset_sample_buffer(struct APU* apu)
{
    apu->sample_read_index       = 0;
    apu->sample_write_index      = 0;
    apu->sample_count            = 0;
    apu->sample_cycle_accumulator = 0;
}

static void apu_reset_runtime(struct APU* apu)
{
    uint8_t wave_ram[16];
    memcpy(wave_ram, apu->wave.wave_ram, sizeof(wave_ram));

    memset(&apu->square1, 0, sizeof(apu->square1));
    memset(&apu->square2, 0, sizeof(apu->square2));
    memset(&apu->wave, 0, sizeof(apu->wave));
    memset(&apu->noise, 0, sizeof(apu->noise));

    memcpy(apu->wave.wave_ram, wave_ram, sizeof(wave_ram));
    apu->noise.lfsr              = 0x7FFF;
    apu->frame_sequencer_step    = 0;
    apu->frame_sequencer_counter = 0;
    apu->left_volume             = 0;
    apu->right_volume            = 0;
}

static bool apu_lock_stream(struct APU* apu)
{
    if (!apu || !apu->audio_stream) {
        return false;
    }
    return SDL_LockAudioStream(apu->audio_stream);
}

static void apu_unlock_stream(struct APU* apu, bool locked)
{
    if (locked) {
        SDL_UnlockAudioStream(apu->audio_stream);
    }
}

static void apu_push_sample(struct APU* apu, int16_t left, int16_t right)
{
    if (apu->sample_count == APU_RING_BUFFER_FRAMES) {
        apu->sample_read_index = (apu->sample_read_index + 1) % APU_RING_BUFFER_FRAMES;
        apu->sample_count--;
        apu->total_samples_dropped++;
    }

    uint32_t index = apu->sample_write_index * APU_CHANNELS;
    apu->sample_buffer[index]     = left;
    apu->sample_buffer[index + 1] = right;

    apu->sample_write_index = (apu->sample_write_index + 1) % APU_RING_BUFFER_FRAMES;
    apu->sample_count++;
    apu->total_samples_generated++;
}

static void apu_pop_samples(struct APU* apu, int16_t* output, int frames)
{
    for (int i = 0; i < frames; i++) {
        if (apu && apu->sample_count > 0) {
            uint32_t index      = apu->sample_read_index * APU_CHANNELS;
            output[i * 2]      = apu->sample_buffer[index];
            output[i * 2 + 1]  = apu->sample_buffer[index + 1];
            apu->sample_read_index = (apu->sample_read_index + 1) % APU_RING_BUFFER_FRAMES;
            apu->sample_count--;
        }
        else {
            output[i * 2]     = 0;
            output[i * 2 + 1] = 0;
        }
    }
}

static void clock_length_square(struct SimpleSquareChannel* channel)
{
    if (channel->length_enabled && channel->length_counter > 0) {
        channel->length_counter--;
        if (channel->length_counter == 0) {
            channel->enabled = false;
        }
    }
}

static void clock_length_wave(struct SimpleWaveChannel* channel)
{
    if (channel->length_enabled && channel->length_counter > 0) {
        channel->length_counter--;
        if (channel->length_counter == 0) {
            channel->enabled = false;
        }
    }
}

static void clock_length_noise(struct SimpleNoiseChannel* channel)
{
    if (channel->length_enabled && channel->length_counter > 0) {
        channel->length_counter--;
        if (channel->length_counter == 0) {
            channel->enabled = false;
        }
    }
}

static void clock_envelope_square(struct SimpleSquareChannel* channel)
{
    if (!channel->enabled || channel->envelope_period == 0) {
        return;
    }

    channel->envelope_counter++;
    if (channel->envelope_counter < channel->envelope_period) {
        return;
    }

    channel->envelope_counter = 0;
    if (channel->envelope_add && channel->volume < 15) {
        channel->volume++;
    }
    else if (!channel->envelope_add && channel->volume > 0) {
        channel->volume--;
    }
}

static void clock_envelope_noise(struct SimpleNoiseChannel* channel)
{
    if (!channel->enabled || channel->envelope_period == 0) {
        return;
    }

    channel->envelope_counter++;
    if (channel->envelope_counter < channel->envelope_period) {
        return;
    }

    channel->envelope_counter = 0;
    if (channel->envelope_add && channel->volume < 15) {
        channel->volume++;
    }
    else if (!channel->envelope_add && channel->volume > 0) {
        channel->volume--;
    }
}

static void clock_sweep(struct SimpleSquareChannel* channel)
{
    if (!channel->enabled || channel->sweep_period == 0 || channel->sweep_shift == 0) {
        return;
    }

    channel->sweep_counter++;
    if (channel->sweep_counter < channel->sweep_period) {
        return;
    }

    channel->sweep_counter = 0;

    uint16_t delta = channel->frequency >> channel->sweep_shift;
    uint16_t next_frequency;
    if (channel->sweep_negate) {
        next_frequency = (delta > channel->frequency) ? 0 : (uint16_t)(channel->frequency - delta);
    }
    else {
        uint32_t expanded_frequency = channel->frequency + delta;
        if (expanded_frequency > 2047) {
            channel->enabled = false;
            return;
        }
        next_frequency = (uint16_t)expanded_frequency;
    }

    channel->frequency    = next_frequency;
    channel->period_timer = pulse_reload(channel);
}

static void clock_frame_sequencer(struct APU* apu)
{
    uint8_t step = apu->frame_sequencer_step;

    if ((step & 1) == 0) {
        clock_length_square(&apu->square1);
        clock_length_square(&apu->square2);
        clock_length_wave(&apu->wave);
        clock_length_noise(&apu->noise);
    }

    if (step == 2 || step == 6) {
        clock_sweep(&apu->square1);
    }

    if (step == 7) {
        clock_envelope_square(&apu->square1);
        clock_envelope_square(&apu->square2);
        clock_envelope_noise(&apu->noise);
    }

    apu->frame_sequencer_step = (apu->frame_sequencer_step + 1) & 7;
}

static void tick_pulse_channel(struct SimpleSquareChannel* channel)
{
    if (!channel->enabled || !channel->dac_enabled) {
        return;
    }

    if (channel->period_timer == 0) {
        channel->period_timer = pulse_reload(channel);
    }

    channel->period_timer--;
    if (channel->period_timer == 0) {
        channel->duty_step    = (channel->duty_step + 1) & 7;
        channel->period_timer = pulse_reload(channel);
    }
}

static void tick_wave_channel(struct SimpleWaveChannel* channel)
{
    if (!channel->enabled || !channel->dac_enabled) {
        return;
    }

    uint16_t ticks = 2;
    while (ticks > 0) {
        if (channel->period_timer == 0) {
            channel->period_timer = wave_reload(channel);
        }

        if (ticks < channel->period_timer) {
            channel->period_timer -= ticks;
            break;
        }

        ticks -= channel->period_timer;
        channel->sample_index = (channel->sample_index + 1) & 31;
        channel->period_timer = wave_reload(channel);
    }
}

static void tick_noise_channel(struct SimpleNoiseChannel* channel)
{
    if (!channel->enabled || !channel->dac_enabled) {
        return;
    }

    uint32_t reload = noise_reload(channel);
    if (reload == 0) {
        return;
    }

    uint32_t ticks = 4;
    while (ticks > 0) {
        if (channel->period_timer == 0) {
            channel->period_timer = reload;
        }

        if (ticks < channel->period_timer) {
            channel->period_timer -= ticks;
            break;
        }

        ticks -= channel->period_timer;

        uint16_t result = (channel->lfsr & 1) ^ ((channel->lfsr >> 1) & 1);
        channel->lfsr >>= 1;
        channel->lfsr |= (uint16_t)(result << 14);
        if (channel->width_mode) {
            channel->lfsr &= (uint16_t)~0x40;
            channel->lfsr |= (uint16_t)(result << 6);
        }
        channel->period_timer = reload;
    }
}

static float dac_to_float(uint8_t sample, bool dac_enabled)
{
    if (!dac_enabled) {
        return 0.0f;
    }
    return ((float)sample / 15.0f) * -2.0f + 1.0f;
}

static uint8_t pulse_sample(const struct SimpleSquareChannel* channel)
{
    bool active = channel->enabled && channel->dac_enabled &&
                  (!channel->length_enabled || channel->length_counter > 0);
    if (!active) {
        return 0;
    }

    return DUTY_PATTERNS[channel->duty][channel->duty_step] ? channel->volume : 0;
}

static uint8_t wave_sample(const struct SimpleWaveChannel* channel)
{
    bool active = channel->enabled && channel->dac_enabled &&
                  (!channel->length_enabled || channel->length_counter > 0);
    if (!active || channel->volume_shift == 0) {
        return 0;
    }

    uint8_t sample = channel->wave_ram[channel->sample_index / 2];
    sample = (channel->sample_index & 1) ? (sample & 0x0F) : (sample >> 4);
    return sample >> (channel->volume_shift - 1);
}

static uint8_t noise_sample(const struct SimpleNoiseChannel* channel)
{
    bool active = channel->enabled && channel->dac_enabled &&
                  (!channel->length_enabled || channel->length_counter > 0);
    if (!active) {
        return 0;
    }

    return (channel->lfsr & 1) ? channel->volume : 0;
}

static void mix_sample(struct APU* apu, int16_t* left, int16_t* right)
{
    float left_mix  = 0.0f;
    float right_mix = 0.0f;

    if (apu->sound_enabled) {
        float ch1 = dac_to_float(pulse_sample(&apu->square1), apu->square1.dac_enabled);
        float ch2 = dac_to_float(pulse_sample(&apu->square2), apu->square2.dac_enabled);
        float ch3 = dac_to_float(wave_sample(&apu->wave), apu->wave.dac_enabled);
        float ch4 = dac_to_float(noise_sample(&apu->noise), apu->noise.dac_enabled);

        if (apu->square1.left_enable) left_mix += ch1;
        if (apu->square1.right_enable) right_mix += ch1;
        if (apu->square2.left_enable) left_mix += ch2;
        if (apu->square2.right_enable) right_mix += ch2;
        if (apu->wave.left_enable) left_mix += ch3;
        if (apu->wave.right_enable) right_mix += ch3;
        if (apu->noise.left_enable) left_mix += ch4;
        if (apu->noise.right_enable) right_mix += ch4;

        left_mix  = (left_mix / 4.0f) * ((float)(apu->left_volume + 1) / 8.0f);
        right_mix = (right_mix / 4.0f) * ((float)(apu->right_volume + 1) / 8.0f);
    }

    int32_t final_left  = (int32_t)(left_mix * 32767.0f);
    int32_t final_right = (int32_t)(right_mix * 32767.0f);

    *left  = (int16_t)clamp_i32(final_left, -32768, 32767);
    *right = (int16_t)clamp_i32(final_right, -32768, 32767);
}

static void maybe_generate_sample(struct APU* apu)
{
    apu->sample_cycle_accumulator += APU_SAMPLE_RATE;
    if (apu->sample_cycle_accumulator < APU_CPU_M_CYCLE_HZ) {
        return;
    }

    apu->sample_cycle_accumulator -= APU_CPU_M_CYCLE_HZ;

    int16_t left;
    int16_t right;
    mix_sample(apu, &left, &right);
    apu_push_sample(apu, left, right);
}

void apu_step(struct APU* apu, uint8_t m_cycles)
{
    if (!apu || m_cycles == 0) {
        return;
    }

    bool locked = apu_lock_stream(apu);
    for (uint8_t i = 0; i < m_cycles; i++) {
        apu->frame_sequencer_counter++;
        if (apu->frame_sequencer_counter >= APU_FRAME_SEQ_PERIOD) {
            apu->frame_sequencer_counter -= APU_FRAME_SEQ_PERIOD;
            clock_frame_sequencer(apu);
        }

        if (apu->sound_enabled) {
            tick_pulse_channel(&apu->square1);
            tick_pulse_channel(&apu->square2);
            tick_wave_channel(&apu->wave);
            tick_noise_channel(&apu->noise);
        }

        maybe_generate_sample(apu);
    }
    apu_unlock_stream(apu, locked);
}

void apu_audio_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount)
{
    (void)total_amount;

    struct APU* apu = (struct APU*)userdata;
    const int frame_bytes = APU_CHANNELS * (int)sizeof(int16_t);
    int bytes_needed      = additional_amount - (additional_amount % frame_bytes);
    if (bytes_needed <= 0) {
        return;
    }

    int frames_needed = bytes_needed / frame_bytes;
    int16_t* buffer   = (int16_t*)malloc((size_t)bytes_needed);
    if (!buffer) {
        return;
    }

    apu_pop_samples(apu, buffer, frames_needed);
    SDL_PutAudioStreamData(stream, buffer, bytes_needed);
    free(buffer);
}

void apu_write_register(struct APU* apu, uint16_t address, uint8_t value)
{
    if (!apu) {
        return;
    }

    bool is_wave_ram = address >= WAVE_RAM_START && address <= WAVE_RAM_END;
    if (!apu->sound_enabled && address != NR52_ADDRESS && !is_wave_ram) {
        return;
    }

    switch (address) {
    case NR10_ADDRESS:
        apu->square1.sweep_period = (value >> 4) & 7;
        apu->square1.sweep_negate = (value & 8) != 0;
        apu->square1.sweep_shift  = value & 7;
        break;

    case NR11_ADDRESS:
        apu->square1.duty           = (value >> 6) & 3;
        apu->square1.length_counter = 64 - (value & 0x3F);
        break;

    case NR12_ADDRESS:
        apu->square1.initial_volume = (value >> 4) & 0x0F;
        apu->square1.envelope_add   = (value & 8) != 0;
        apu->square1.envelope_period = value & 7;
        apu->square1.dac_enabled    = (value & 0xF8) != 0;
        if (!apu->square1.dac_enabled) {
            apu->square1.enabled = false;
        }
        break;

    case NR13_ADDRESS:
        apu->square1.frequency = (apu->square1.frequency & 0x700) | value;
        break;

    case NR14_ADDRESS:
        apu->square1.frequency      = (apu->square1.frequency & 0x0FF) | ((value & 7) << 8);
        apu->square1.length_enabled = (value & 0x40) != 0;
        if (value & 0x80) {
            apu->square1.enabled          = apu->square1.dac_enabled;
            apu->square1.duty_step        = 0;
            apu->square1.volume           = apu->square1.initial_volume;
            apu->square1.envelope_counter = 0;
            apu->square1.sweep_counter    = 0;
            apu->square1.period_timer     = pulse_reload(&apu->square1);
            if (apu->square1.length_counter == 0) {
                apu->square1.length_counter = 64;
            }
        }
        break;

    case NR21_ADDRESS:
        apu->square2.duty           = (value >> 6) & 3;
        apu->square2.length_counter = 64 - (value & 0x3F);
        break;

    case NR22_ADDRESS:
        apu->square2.initial_volume = (value >> 4) & 0x0F;
        apu->square2.envelope_add   = (value & 8) != 0;
        apu->square2.envelope_period = value & 7;
        apu->square2.dac_enabled    = (value & 0xF8) != 0;
        if (!apu->square2.dac_enabled) {
            apu->square2.enabled = false;
        }
        break;

    case NR23_ADDRESS:
        apu->square2.frequency = (apu->square2.frequency & 0x700) | value;
        break;

    case NR24_ADDRESS:
        apu->square2.frequency      = (apu->square2.frequency & 0x0FF) | ((value & 7) << 8);
        apu->square2.length_enabled = (value & 0x40) != 0;
        if (value & 0x80) {
            apu->square2.enabled          = apu->square2.dac_enabled;
            apu->square2.duty_step        = 0;
            apu->square2.volume           = apu->square2.initial_volume;
            apu->square2.envelope_counter = 0;
            apu->square2.period_timer     = pulse_reload(&apu->square2);
            if (apu->square2.length_counter == 0) {
                apu->square2.length_counter = 64;
            }
        }
        break;

    case NR30_ADDRESS:
        apu->wave.dac_enabled = (value & 0x80) != 0;
        if (!apu->wave.dac_enabled) {
            apu->wave.enabled = false;
        }
        break;

    case NR31_ADDRESS:
        apu->wave.length_counter = 256 - value;
        break;

    case NR32_ADDRESS:
        apu->wave.volume_shift = (value >> 5) & 3;
        break;

    case NR33_ADDRESS:
        apu->wave.frequency = (apu->wave.frequency & 0x700) | value;
        break;

    case NR34_ADDRESS:
        apu->wave.frequency      = (apu->wave.frequency & 0x0FF) | ((value & 7) << 8);
        apu->wave.length_enabled = (value & 0x40) != 0;
        if (value & 0x80) {
            apu->wave.enabled      = apu->wave.dac_enabled;
            apu->wave.sample_index = 0;
            apu->wave.period_timer = wave_reload(&apu->wave);
            if (apu->wave.length_counter == 0) {
                apu->wave.length_counter = 256;
            }
        }
        break;

    case NR41_ADDRESS:
        apu->noise.length_counter = 64 - (value & 0x3F);
        break;

    case NR42_ADDRESS:
        apu->noise.initial_volume = (value >> 4) & 0x0F;
        apu->noise.envelope_add   = (value & 8) != 0;
        apu->noise.envelope_period = value & 7;
        apu->noise.dac_enabled    = (value & 0xF8) != 0;
        if (!apu->noise.dac_enabled) {
            apu->noise.enabled = false;
        }
        break;

    case NR43_ADDRESS:
        apu->noise.shift_amount = (value >> 4) & 0x0F;
        apu->noise.width_mode   = (value & 0x08) != 0;
        apu->noise.divisor_code = value & 7;
        apu->noise.period_timer = noise_reload(&apu->noise);
        break;

    case NR44_ADDRESS:
        apu->noise.length_enabled = (value & 0x40) != 0;
        if (value & 0x80) {
            apu->noise.enabled          = apu->noise.dac_enabled;
            apu->noise.lfsr             = 0x7FFF;
            apu->noise.volume           = apu->noise.initial_volume;
            apu->noise.envelope_counter = 0;
            apu->noise.period_timer     = noise_reload(&apu->noise);
            if (apu->noise.length_counter == 0) {
                apu->noise.length_counter = 64;
            }
        }
        break;

    case NR50_ADDRESS:
        apu->left_volume  = (value >> 4) & 7;
        apu->right_volume = value & 7;
        break;

    case NR51_ADDRESS:
        apu->square1.left_enable = (value & 0x10) != 0;
        apu->square1.right_enable = (value & 0x01) != 0;
        apu->square2.left_enable = (value & 0x20) != 0;
        apu->square2.right_enable = (value & 0x02) != 0;
        apu->wave.left_enable = (value & 0x40) != 0;
        apu->wave.right_enable = (value & 0x04) != 0;
        apu->noise.left_enable = (value & 0x80) != 0;
        apu->noise.right_enable = (value & 0x08) != 0;
        break;

    case NR52_ADDRESS: {
        bool was_enabled  = apu->sound_enabled;
        bool next_enabled = (value & 0x80) != 0;
        apu->sound_enabled = next_enabled;

        if (was_enabled && !next_enabled) {
            APU_DEBUG_PRINT("Sound disabled\n");
            bool locked = apu_lock_stream(apu);
            apu_reset_runtime(apu);
            apu_reset_sample_buffer(apu);
            apu_unlock_stream(apu, locked);
        }
        else if (!was_enabled && next_enabled) {
            APU_DEBUG_PRINT("Sound enabled\n");
            bool locked = apu_lock_stream(apu);
            apu_reset_sample_buffer(apu);
            apu_unlock_stream(apu, locked);
        }
        break;
    }

    default:
        if (is_wave_ram) {
            apu->wave.wave_ram[address - WAVE_RAM_START] = value;
        }
        break;
    }
}

uint8_t apu_read_register(struct APU* apu, uint16_t address)
{
    if (!apu) {
        return 0xFF;
    }

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
        return 0xFF;
    case NR14_ADDRESS:
        return 0xBF | (apu->square1.length_enabled ? 0x40 : 0);
    case NR21_ADDRESS:
        return 0x3F | (apu->square2.duty << 6);
    case NR22_ADDRESS:
        return (apu->square2.initial_volume << 4) |
               (apu->square2.envelope_add ? 8 : 0) | apu->square2.envelope_period;
    case NR23_ADDRESS:
        return 0xFF;
    case NR24_ADDRESS:
        return 0xBF | (apu->square2.length_enabled ? 0x40 : 0);
    case NR30_ADDRESS:
        return 0x7F | (apu->wave.dac_enabled ? 0x80 : 0);
    case NR31_ADDRESS:
        return 0xFF;
    case NR32_ADDRESS:
        return 0x9F | (apu->wave.volume_shift << 5);
    case NR33_ADDRESS:
        return 0xFF;
    case NR34_ADDRESS:
        return 0xBF | (apu->wave.length_enabled ? 0x40 : 0);
    case NR41_ADDRESS:
        return 0xFF;
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

struct APU* create_apu(void)
{
    struct APU* apu = (struct APU*)malloc(sizeof(struct APU));
    if (!apu) {
        APU_EMERGENCY_PRINT("Failed to allocate memory for APU\n");
        return NULL;
    }

    memset(apu, 0, sizeof(struct APU));
    apu->noise.lfsr      = 0x7FFF;
    apu->write_register = apu_write_register;
    apu->read_register  = apu_read_register;
    apu->step           = apu_step;

    return apu;
}

bool apu_start_audio(struct APU* apu)
{
    if (!apu) {
        return false;
    }
    if (apu->audio_stream) {
        return true;
    }

    if ((SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO) == 0) {
        if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
            APU_WARN_PRINT("Failed to initialize SDL Audio: %s\n", SDL_GetError());
            return false;
        }
        apu->owns_audio_subsystem = true;
    }

    SDL_AudioSpec desired_spec = {0};
    desired_spec.freq          = APU_SAMPLE_RATE;
    desired_spec.format        = SDL_AUDIO_S16;
    desired_spec.channels      = APU_CHANNELS;

    apu->audio_device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &desired_spec);
    if (apu->audio_device == 0) {
        APU_WARN_PRINT("Failed to open audio device: %s\n", SDL_GetError());
        if (apu->owns_audio_subsystem) {
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            apu->owns_audio_subsystem = false;
        }
        return false;
    }

    apu->audio_spec   = desired_spec;
    apu->audio_stream = SDL_CreateAudioStream(&desired_spec, &desired_spec);
    if (!apu->audio_stream || !SDL_BindAudioStream(apu->audio_device, apu->audio_stream)) {
        APU_WARN_PRINT("Failed to create/bind audio stream: %s\n", SDL_GetError());
        apu_stop_audio(apu);
        return false;
    }

    if (!SDL_SetAudioStreamGetCallback(apu->audio_stream, apu_audio_callback, apu)) {
        APU_WARN_PRINT("Failed to set audio callback: %s\n", SDL_GetError());
        apu_stop_audio(apu);
        return false;
    }

    if (!SDL_ResumeAudioDevice(apu->audio_device)) {
        APU_WARN_PRINT("Failed to resume audio device: %s\n", SDL_GetError());
        apu_stop_audio(apu);
        return false;
    }

    APU_INFO_PRINT("APU audio output ready.\n");
    return true;
}

void apu_stop_audio(struct APU* apu)
{
    if (!apu) {
        return;
    }

    if (apu->audio_device != 0) {
        SDL_PauseAudioDevice(apu->audio_device);
    }
    if (apu->audio_stream) {
        SDL_DestroyAudioStream(apu->audio_stream);
        apu->audio_stream = NULL;
    }
    if (apu->audio_device != 0) {
        SDL_CloseAudioDevice(apu->audio_device);
        apu->audio_device = 0;
    }
    if (apu->owns_audio_subsystem) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        apu->owns_audio_subsystem = false;
    }

    apu_reset_sample_buffer(apu);
}

void apu_attach_mmu(struct APU* apu, struct MMU* mmu)
{
    if (apu) {
        apu->mmu = mmu;
    }
}

void free_apu(struct APU* apu)
{
    if (apu) {
        apu_stop_audio(apu);
        free(apu);
    }
}
