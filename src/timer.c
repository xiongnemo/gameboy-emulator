#include "timer.h"

void timer_add_time(struct Timer* self, uint8_t cycle)
{
    timer_refresh_register(self);

    self->divider += cycle;
    if (self->divider >= 256) {
        self->divider -= 256;
        self->reg_div++;
    }

    if ((self->reg_tac & 0x04) == 0) {   // timer enabled?
        return;
    }

    self->counter += cycle;
    uint64_t current_clock_threshold;

    switch (self->reg_tac & 0x3) {
    case 0: current_clock_threshold = 256; break;  // 00: Every 256 M-cycles (4096 Hz)
    case 1: current_clock_threshold = 4; break;    // 01: Every 4 M-cycles (262144 Hz)
    case 2: current_clock_threshold = 16; break;   // 10: Every 16 M-cycles (65536 Hz)
    case 3: current_clock_threshold = 64; break;   // 11: Every 64 M-cycles (16384 Hz)
    }

    while (self->counter >= current_clock_threshold) {
        self->counter -= current_clock_threshold;

        if (self->reg_tima == 0xFF) {
            // request timer interrupt
            uint8_t temp_interrupt_flag = self->ram->get_ram_byte(self->ram, IF_ADDRESS);
            temp_interrupt_flag |= 0x04;
            self->ram->set_ram_byte(self->ram, IF_ADDRESS, temp_interrupt_flag);

            // reset tima to tma
            self->reg_tima = self->reg_tma;
        }
        else {
            self->reg_tima++;
        }
    }

    timer_set_register(self);
}

void timer_refresh_register(struct Timer* self)
{
    self->reg_div  = self->ram->get_ram_byte(self->ram, TIMER_DIV_ADDRESS);
    self->reg_tima = self->ram->get_ram_byte(self->ram, TIMER_TIMA_ADDRESS);
    self->reg_tma  = self->ram->get_ram_byte(self->ram, TIMER_TMA_ADDRESS);
    self->reg_tac  = self->ram->get_ram_byte(self->ram, TIMER_TAC_ADDRESS);
}

void timer_set_register(struct Timer* self)
{
    self->ram->set_ram_byte(self->ram, TIMER_DIV_ADDRESS, self->reg_div);
    self->ram->set_ram_byte(self->ram, TIMER_TIMA_ADDRESS, self->reg_tima);
    self->ram->set_ram_byte(self->ram, TIMER_TMA_ADDRESS, self->reg_tma);
    self->ram->set_ram_byte(self->ram, TIMER_TAC_ADDRESS, self->reg_tac);
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

void timer_attach_ram(struct Timer* self, struct Ram* ram)
{
    self->ram = ram;
}

void free_timer(struct Timer* timer)
{
    if (timer) {
        free(timer);
    }
}
