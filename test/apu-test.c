#include "../src/apu.h"
#include "test.h"
#include <SDL3/SDL.h>

static void step_many(struct APU* apu, uint32_t m_cycles)
{
    while (m_cycles > 0) {
        uint8_t chunk = m_cycles > 255 ? 255 : (uint8_t)m_cycles;
        apu_step(apu, chunk);
        m_cycles -= chunk;
    }
}

static void test_pulse_period_timing(void)
{
    struct APU* apu = create_apu();
    assert(apu != NULL);

    apu_write_register(apu, NR52_ADDRESS, 0x80);
    apu_write_register(apu, NR12_ADDRESS, 0xF0);
    apu_write_register(apu, NR11_ADDRESS, 0x80);
    apu_write_register(apu, NR13_ADDRESS, 0xFF);
    apu_write_register(apu, NR14_ADDRESS, 0x87);

    assert(apu->square1.enabled);
    assert(apu->square1.period_timer == 1);

    step_many(apu, 1);
    assert(apu->square1.duty_step == 1);

    step_many(apu, 7);
    assert(apu->square1.duty_step == 0);

    free_apu(apu);
}

static void test_frame_sequencer_length_envelope_and_sweep(void)
{
    struct APU* apu = create_apu();
    assert(apu != NULL);

    apu_write_register(apu, NR52_ADDRESS, 0x80);
    apu_write_register(apu, NR21_ADDRESS, 0x3E);
    apu_write_register(apu, NR22_ADDRESS, 0xF0);
    apu_write_register(apu, NR24_ADDRESS, 0xC7);

    assert(apu->square2.length_counter == 2);
    step_many(apu, APU_FRAME_SEQ_PERIOD);
    assert(apu->square2.length_counter == 1);
    step_many(apu, APU_FRAME_SEQ_PERIOD);
    assert(apu->square2.length_counter == 1);
    step_many(apu, APU_FRAME_SEQ_PERIOD);
    assert(apu->square2.length_counter == 0);
    assert(!apu->square2.enabled);

    free_apu(apu);

    apu = create_apu();
    assert(apu != NULL);

    apu_write_register(apu, NR52_ADDRESS, 0x80);
    apu_write_register(apu, NR22_ADDRESS, 0x19);
    apu_write_register(apu, NR24_ADDRESS, 0x87);
    assert(apu->square2.volume == 1);
    step_many(apu, APU_FRAME_SEQ_PERIOD * 8);
    assert(apu->square2.volume == 2);

    free_apu(apu);

    apu = create_apu();
    assert(apu != NULL);

    apu_write_register(apu, NR52_ADDRESS, 0x80);
    apu_write_register(apu, NR10_ADDRESS, 0x11);
    apu_write_register(apu, NR12_ADDRESS, 0xF0);
    apu_write_register(apu, NR13_ADDRESS, 0x00);
    apu_write_register(apu, NR14_ADDRESS, 0x84);
    assert(apu->square1.frequency == 1024);
    step_many(apu, APU_FRAME_SEQ_PERIOD * 3);
    assert(apu->square1.frequency == 1536);

    free_apu(apu);
}

static void test_sample_rate_accumulator(void)
{
    struct APU* apu = create_apu();
    assert(apu != NULL);

    step_many(apu, APU_CPU_M_CYCLE_HZ);

    assert(apu->total_samples_generated == APU_SAMPLE_RATE);
    assert(apu->sample_count == APU_RING_BUFFER_FRAMES);
    assert(apu->total_samples_dropped == APU_SAMPLE_RATE - APU_RING_BUFFER_FRAMES);

    free_apu(apu);
}

static void test_audio_callback_uses_additional_amount(void)
{
    assert(SDL_InitSubSystem(SDL_INIT_AUDIO));

    SDL_AudioSpec spec = {0};
    spec.freq          = APU_SAMPLE_RATE;
    spec.format        = SDL_AUDIO_S16;
    spec.channels      = APU_CHANNELS;

    SDL_AudioStream* stream = SDL_CreateAudioStream(&spec, &spec);
    assert(stream != NULL);

    struct APU* apu = create_apu();
    assert(apu != NULL);
    step_many(apu, 1000);

    uint32_t buffered_before = apu->sample_count;
    assert(buffered_before >= 4);

    int frame_bytes = APU_CHANNELS * (int)sizeof(int16_t);
    apu_audio_callback(apu, stream, 4 * frame_bytes, 100 * frame_bytes);

    assert(SDL_GetAudioStreamQueued(stream) == 4 * frame_bytes);
    assert(apu->sample_count == buffered_before - 4);

    free_apu(apu);
    SDL_DestroyAudioStream(stream);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

static void test_nr52_keeps_wave_ram_available(void)
{
    struct APU* apu = create_apu();
    assert(apu != NULL);

    apu_write_register(apu, NR52_ADDRESS, 0x80);
    apu_write_register(apu, WAVE_RAM_START, 0xAB);
    apu_write_register(apu, NR12_ADDRESS, 0xF3);
    assert(apu_read_register(apu, NR12_ADDRESS) == 0xF3);

    apu_write_register(apu, NR52_ADDRESS, 0x00);
    assert((apu_read_register(apu, NR52_ADDRESS) & 0x80) == 0);
    assert(apu_read_register(apu, WAVE_RAM_START) == 0xAB);
    assert(apu_read_register(apu, NR12_ADDRESS) == 0x00);

    apu_write_register(apu, NR12_ADDRESS, 0xF3);
    assert(apu_read_register(apu, NR12_ADDRESS) == 0x00);

    apu_write_register(apu, WAVE_RAM_START, 0xCD);
    assert(apu_read_register(apu, WAVE_RAM_START) == 0xCD);

    free_apu(apu);
}

int main(void)
{
    config.start_time = get_time_in_seconds();

    test_pulse_period_timing();
    test_frame_sequencer_length_envelope_and_sweep();
    test_sample_rate_accumulator();
    test_audio_callback_uses_additional_amount();
    test_nr52_keeps_wave_ram_available();

    return 0;
}
