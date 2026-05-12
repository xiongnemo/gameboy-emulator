#include "../src/cpu.h"
#include "../src/joypad.h"
#include "test.h"

#define TEST_PC 0xC000
#define TEST_SP 0xD000

struct TimingSystem
{
    struct CPU*   cpu;
    struct Timer* timer;
    struct PPU*   ppu;
};

static struct TimingSystem create_timing_system(void)
{
    struct Cartridge* cartridge = create_cartridge();
    struct Ram*       ram       = create_ram();
    struct Vram*      vram      = create_vram();
    struct PPU*       ppu       = create_ppu(vram);
    struct MMU*       mmu       = create_mmu(cartridge, ram, ppu);
    struct Registers* registers = create_registers();
    struct CPU*       cpu       = create_cpu(registers, mmu);
    struct Timer*     timer     = create_timer();

    ppu_attach_mmu(ppu, mmu);
    timer_attach_ram(timer, ram);
    mmu_attach_timer(mmu, timer);
    cpu_attach_timer(cpu, timer);
    cpu_attach_ppu(cpu, ppu);

    cpu->registers->set_control_register(cpu->registers, PC, TEST_PC);
    cpu->registers->set_control_register(cpu->registers, SP, TEST_SP);

    return (struct TimingSystem){
        .cpu   = cpu,
        .timer = timer,
        .ppu   = ppu,
    };
}

static void destroy_timing_system(struct TimingSystem* system)
{
    free_timer(system->timer);
    free_cpu(system->cpu);
}

static void ppu_step_many(struct PPU* ppu, uint32_t m_cycles)
{
    while (m_cycles > 0) {
        uint8_t chunk = m_cycles > 255 ? 255 : (uint8_t)m_cycles;
        ppu_step(ppu, chunk);
        m_cycles -= chunk;
    }
}

static void test_conditional_cycles_do_not_stick(void)
{
    struct TimingSystem system = create_timing_system();
    struct CPU*         cpu    = system.cpu;

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0x20);      // JR NZ,r8
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x00);
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 2, 0x20);  // JR NZ,r8
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 3, 0x00);

    cpu->registers->set_flag_z(cpu->registers, false);
    assert(cpu_step_next(cpu) == 3);

    cpu->registers->set_flag_z(cpu->registers, true);
    assert(cpu_step_next(cpu) == 2);

    destroy_timing_system(&system);
}

static void test_ei_di_reti_and_interrupt_cycles(void)
{
    struct TimingSystem system = create_timing_system();
    struct CPU*         cpu    = system.cpu;

    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xFB);      // EI
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0x00);  // NOP
    cpu->mmu->mmu_set_byte(cpu->mmu, IF_ADDRESS, INT_VBLANK);
    cpu->mmu->mmu_set_byte(cpu->mmu, IE_ADDRESS, INT_VBLANK);

    assert(cpu_step_next(cpu) == 1);
    assert(cpu->interrupt_master_enable == false);
    assert(cpu->ime_enable_delay == 1);

    assert(cpu_step_next(cpu) == 1);
    assert(cpu->interrupt_master_enable == true);
    assert(cpu->registers->get_control_register(cpu->registers, PC) == TEST_PC + 2);

    assert(cpu_step_next(cpu) == 5);
    assert(cpu->interrupt_master_enable == false);
    assert(cpu->registers->get_control_register(cpu->registers, PC) == INTERRUPT_VECTOR_VBLANK);

    destroy_timing_system(&system);

    system = create_timing_system();
    cpu    = system.cpu;
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xFB);      // EI
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 1, 0xF3);  // DI
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC + 2, 0x00);  // NOP

    assert(cpu_step_next(cpu) == 1);
    assert(cpu->ime_enable_delay == 1);
    assert(cpu_step_next(cpu) == 1);
    assert(cpu->ime_enable_delay == 0);
    assert(cpu->interrupt_master_enable == false);
    assert(cpu_step_next(cpu) == 1);
    assert(cpu->interrupt_master_enable == false);

    destroy_timing_system(&system);

    system = create_timing_system();
    cpu    = system.cpu;
    cpu->mmu->mmu_set_byte(cpu->mmu, TEST_PC, 0xD9);  // RETI
    cpu->mmu->mmu_set_word(cpu->mmu, TEST_SP, 0xC123);

    assert(cpu_step_next(cpu) == 4);
    assert(cpu->interrupt_master_enable == true);
    assert(cpu->ime_enable_delay == 0);
    assert(cpu->registers->get_control_register(cpu->registers, PC) == 0xC123);

    destroy_timing_system(&system);
}

static void test_halt_interrupt_wake_uses_service_cycles(void)
{
    struct TimingSystem system = create_timing_system();
    struct CPU*         cpu    = system.cpu;

    cpu->halted                  = true;
    cpu->interrupt_master_enable = true;
    cpu->mmu->mmu_set_byte(cpu->mmu, IF_ADDRESS, INT_VBLANK);
    cpu->mmu->mmu_set_byte(cpu->mmu, IE_ADDRESS, INT_VBLANK);

    assert(cpu_step_next(cpu) == 5);
    assert(cpu->halted == false);
    assert(cpu->registers->get_control_register(cpu->registers, PC) == INTERRUPT_VECTOR_VBLANK);

    destroy_timing_system(&system);
}

static void test_div_timing_and_reset(void)
{
    struct TimingSystem system = create_timing_system();
    struct Timer*       timer  = system.timer;

    timer_step(timer, 63);
    assert(timer_read_register(timer, TIMER_DIV_ADDRESS) == 0);
    timer_step(timer, 1);
    assert(timer_read_register(timer, TIMER_DIV_ADDRESS) == 1);

    timer_write_register(timer, TIMER_DIV_ADDRESS, 0xFF);
    assert(timer_read_register(timer, TIMER_DIV_ADDRESS) == 0);

    destroy_timing_system(&system);
}

static void test_tima_frequencies_and_overflow(void)
{
    const uint8_t  selects[4]      = {0, 1, 2, 3};
    const uint16_t thresholds[4]   = {256, 4, 16, 64};

    for (int i = 0; i < 4; i++) {
        struct TimingSystem system = create_timing_system();
        struct Timer*       timer  = system.timer;

        timer_write_register(timer, TIMER_DIV_ADDRESS, 0);
        timer_write_register(timer, TIMER_TIMA_ADDRESS, 0);
        timer_write_register(timer, TIMER_TAC_ADDRESS, 0x04 | selects[i]);
        timer_step(timer, (uint8_t)(thresholds[i] - 1));
        assert(timer_read_register(timer, TIMER_TIMA_ADDRESS) == 0);
        timer_step(timer, 1);
        assert(timer_read_register(timer, TIMER_TIMA_ADDRESS) == 1);

        destroy_timing_system(&system);
    }

    struct TimingSystem system = create_timing_system();
    struct Timer*       timer  = system.timer;
    struct CPU*         cpu    = system.cpu;

    timer_write_register(timer, TIMER_DIV_ADDRESS, 0);
    timer_write_register(timer, TIMER_TMA_ADDRESS, 0xAB);
    timer_write_register(timer, TIMER_TIMA_ADDRESS, 0xFF);
    timer_write_register(timer, TIMER_TAC_ADDRESS, 0x05);

    timer_step(timer, 4);
    assert(timer_read_register(timer, TIMER_TIMA_ADDRESS) == 0x00);
    assert((cpu->mmu->mmu_get_byte(cpu->mmu, IF_ADDRESS) & INT_TIMER) == 0);

    timer_step(timer, 1);
    assert(timer_read_register(timer, TIMER_TIMA_ADDRESS) == 0xAB);
    assert((cpu->mmu->mmu_get_byte(cpu->mmu, IF_ADDRESS) & INT_TIMER) != 0);

    destroy_timing_system(&system);
}

static void test_ppu_frame_timing(void)
{
    struct TimingSystem system = create_timing_system();
    struct PPU*         ppu    = system.ppu;
    struct CPU*         cpu    = system.cpu;

    ppu_step_many(ppu, VISIBLE_SCANLINES * GB_SCANLINE_M_CYCLES);
    assert(ppu->mode == MODE_VBLANK);
    assert(cpu->mmu->mmu_get_byte(cpu->mmu, LY_ADDRESS) == VISIBLE_SCANLINES);
    assert((cpu->mmu->mmu_get_byte(cpu->mmu, IF_ADDRESS) & INT_VBLANK) != 0);
    assert(ppu_consume_frame_ready(ppu) == false);

    ppu_step_many(ppu, (SCANLINES_PER_FRAME - VISIBLE_SCANLINES) * GB_SCANLINE_M_CYCLES);
    assert(ppu->mode == MODE_OAM_SEARCH);
    assert(cpu->mmu->mmu_get_byte(cpu->mmu, LY_ADDRESS) == 0);
    assert(ppu->frame_count == 1);
    assert(ppu_consume_frame_ready(ppu) == true);
    assert(ppu_consume_frame_ready(ppu) == false);

    destroy_timing_system(&system);
}

static void test_ppu_lcd_disable_resets_ly_from_vblank(void)
{
    struct TimingSystem system = create_timing_system();
    struct PPU*         ppu    = system.ppu;
    struct CPU*         cpu    = system.cpu;

    ppu_step_many(ppu, VISIBLE_SCANLINES * GB_SCANLINE_M_CYCLES);
    assert(ppu->mode == MODE_VBLANK);
    assert(cpu->mmu->mmu_get_byte(cpu->mmu, LY_ADDRESS) == VISIBLE_SCANLINES);

    cpu->mmu->mmu_set_byte(cpu->mmu, LCDC_ADDRESS, 0x00);
    ppu_step(ppu, 1);
    assert(ppu->lcd_enabled == false);
    assert(cpu->mmu->mmu_get_byte(cpu->mmu, LY_ADDRESS) == 0);

    cpu->mmu->mmu_set_byte(cpu->mmu, LCDC_ADDRESS, LCDC_ENABLE);
    ppu_step(ppu, 1);
    assert(ppu->lcd_enabled == true);
    assert(ppu->mode == MODE_OAM_SEARCH);
    assert(cpu->mmu->mmu_get_byte(cpu->mmu, LY_ADDRESS) == 0);

    destroy_timing_system(&system);
}

static void test_dma_stall_advances_peripherals(void)
{
    struct TimingSystem system = create_timing_system();
    struct CPU*         cpu    = system.cpu;
    struct Timer*       timer  = system.timer;
    struct PPU*         ppu    = system.ppu;

    cpu->mmu->mmu_set_byte(cpu->mmu, DMA_ADDRESS, 0xC0);
    assert(cpu->mmu->pending_dma_stall_m_cycles == GB_OAM_DMA_M_CYCLES);

    cpu_step_for_cycles(cpu, GB_OAM_DMA_M_CYCLES);
    assert(cpu->dma_stall_m_cycles == 0);
    assert(cpu->cycles == GB_OAM_DMA_M_CYCLES);
    assert(timer_read_register(timer, TIMER_DIV_ADDRESS) == 2);
    assert(ppu->ppu_inner_clock == GB_OAM_DMA_M_CYCLES * GB_DOTS_PER_M_CYCLE);

    destroy_timing_system(&system);
}

static void test_joypad_register_reads_live_key_state(void)
{
    struct TimingSystem system = create_timing_system();
    struct CPU*         cpu    = system.cpu;
    struct Joypad*      joypad = create_joypad(cpu->mmu);
    assert(joypad != NULL);
    mmu_attach_joypad(cpu->mmu, joypad);

    cpu->mmu->mmu_set_byte(cpu->mmu, JOYPAD_ADDRESS, 0x20);
    assert((cpu->mmu->mmu_get_byte(cpu->mmu, JOYPAD_ADDRESS) & 0x3F) == 0x2F);

    joypad->keys_directions &= 0x0E;
    assert((cpu->mmu->mmu_get_byte(cpu->mmu, JOYPAD_ADDRESS) & 0x3F) == 0x2E);

    cpu->mmu->mmu_set_byte(cpu->mmu, JOYPAD_ADDRESS, 0x10);
    assert((cpu->mmu->mmu_get_byte(cpu->mmu, JOYPAD_ADDRESS) & 0x3F) == 0x1F);

    joypad->keys_controls &= 0x0E;
    assert((cpu->mmu->mmu_get_byte(cpu->mmu, JOYPAD_ADDRESS) & 0x3F) == 0x1E);

    free_joypad(joypad);
    cpu->mmu->joypad = NULL;
    destroy_timing_system(&system);
}

int main(void)
{
    config.start_time = get_time_in_seconds();

    test_conditional_cycles_do_not_stick();
    test_ei_di_reti_and_interrupt_cycles();
    test_halt_interrupt_wake_uses_service_cycles();
    test_div_timing_and_reset();
    test_tima_frequencies_and_overflow();
    test_ppu_frame_timing();
    test_ppu_lcd_disable_resets_ly_from_vblank();
    test_dma_stall_advances_peripherals();
    test_joypad_register_reads_live_key_state();

    printf("Timing tests passed\n");
    return 0;
}
