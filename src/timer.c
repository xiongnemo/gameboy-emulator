#include "timer.h"

void timer_add_time(struct Timer* self, uint8_t cycle, struct Ram* mem)
{
    timer_refresh_register(self, mem);

    self->divider += cycle;
    if (self->divider >= 256) {
        self->divider -= 256;
        self->reg_div++;
    }

    if ((self->reg_tac & 0x04) == 0) {   // timer enabled
        return;
    }

    self->counter += cycle;
    uint64_t current_clock_threshold;

    switch (self->reg_tac & 0x3) {
    case 0: current_clock_threshold = 1024; break;
    case 1: current_clock_threshold = 16; break;
    case 2: current_clock_threshold = 64; break;
    case 3: current_clock_threshold = 256; break;
    }

    while (self->counter >= current_clock_threshold) {
        self->counter -= current_clock_threshold;

        if (self->reg_tima == 0xFF) {
            // request interrupt!
            uint8_t temp_interrupt_flag = mem->get_ram_byte(mem, IF_ADDRESS);
            temp_interrupt_flag |= 0x04;
            mem->set_ram_byte(mem, IF_ADDRESS, temp_interrupt_flag);

            // reset tima to tma
            self->reg_tima = self->reg_tma;
        }
        else {
            self->reg_tima++;
        }
    }

    timer_set_register(self, mem);
}

void timer_refresh_register(struct Timer* self, struct Ram* mem)
{
    self->reg_div  = mem->get_ram_byte(mem, TIMER_DIV_ADDRESS);
    self->reg_tima = mem->get_ram_byte(mem, TIMER_TIMA_ADDRESS);
    self->reg_tma  = mem->get_ram_byte(mem, TIMER_TMA_ADDRESS);
    self->reg_tac  = mem->get_ram_byte(mem, TIMER_TAC_ADDRESS);
}

void timer_set_register(struct Timer* self, struct Ram* mem)
{
    mem->set_ram_byte(mem, TIMER_DIV_ADDRESS, self->reg_div);
    mem->set_ram_byte(mem, TIMER_TIMA_ADDRESS, self->reg_tima);
    mem->set_ram_byte(mem, TIMER_TMA_ADDRESS, self->reg_tma);
    mem->set_ram_byte(mem, TIMER_TAC_ADDRESS, self->reg_tac);
}

struct Timer* create_timer(void)
{
    struct Timer* timer = (struct Timer*)malloc(sizeof(struct Timer));

    // Initialize data members
    timer->counter  = 0;
    timer->divider  = 0;
    timer->reg_div  = 0;   // register divider ff04
    timer->reg_tima = 0;   // counter ff05
    timer->reg_tma  = 0;   // modulator ff06
    timer->reg_tac  = 0;   // control ff07

    // Initialize method pointers
    timer->add_time               = timer_add_time;
    timer->refresh_timer_register = timer_refresh_register;
    timer->set_timer_register     = timer_set_register;

    return timer;
}

void free_timer(struct Timer* timer)
{
    if (timer) {
        free(timer);
    }
}
