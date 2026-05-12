#include "timer.h"

static uint8_t timer_div_read(const struct Timer* self)
{
    return (uint8_t)(self->divider >> 8);
}

static uint8_t timer_selected_divider_bit(uint8_t tac)
{
    switch (tac & 0x03) {
    case 0: return 9;   // 4096 Hz
    case 1: return 3;   // 262144 Hz
    case 2: return 5;   // 65536 Hz
    case 3: return 7;   // 16384 Hz
    }
    return 9;
}

static bool timer_input_signal(const struct Timer* self)
{
    if ((self->reg_tac & 0x04) == 0) {
        return false;
    }
    return ((self->divider >> timer_selected_divider_bit(self->reg_tac)) & 1) != 0;
}

static void timer_sync_to_ram(struct Timer* self)
{
    if (!self->ram) {
        return;
    }

    self->ram->set_ram_byte(self->ram, TIMER_DIV_ADDRESS, timer_div_read(self));
    self->ram->set_ram_byte(self->ram, TIMER_TIMA_ADDRESS, self->reg_tima);
    self->ram->set_ram_byte(self->ram, TIMER_TMA_ADDRESS, self->reg_tma);
    self->ram->set_ram_byte(self->ram, TIMER_TAC_ADDRESS, 0xF8 | (self->reg_tac & 0x07));
}

static void timer_request_interrupt(struct Timer* self)
{
    if (!self->ram) {
        return;
    }

    uint8_t interrupt_flag = self->ram->get_ram_byte(self->ram, IF_ADDRESS);
    interrupt_flag |= INT_TIMER;
    self->ram->set_ram_byte(self->ram, IF_ADDRESS, interrupt_flag);
}

static void timer_increment_tima(struct Timer* self)
{
    if (self->tima_overflow_pending) {
        return;
    }

    if (self->reg_tima == 0xFF) {
        self->reg_tima                = 0x00;
        self->tima_overflow_pending   = true;
        self->tima_overflow_dots      = GB_DOTS_PER_M_CYCLE;
    }
    else {
        self->reg_tima++;
    }
}

static void timer_check_falling_edge(struct Timer* self, bool old_signal)
{
    bool new_signal = timer_input_signal(self);
    if (old_signal && !new_signal) {
        timer_increment_tima(self);
    }
}

static void timer_step_one_dot(struct Timer* self)
{
    if (self->tima_overflow_pending && self->tima_overflow_dots > 0) {
        self->tima_overflow_dots--;
        if (self->tima_overflow_dots == 0) {
            self->reg_tima              = self->reg_tma;
            self->tima_overflow_pending = false;
            timer_request_interrupt(self);
        }
    }

    bool old_signal = timer_input_signal(self);
    self->divider++;
    timer_check_falling_edge(self, old_signal);
}

void timer_step(struct Timer* self, uint8_t m_cycles)
{
    if (!self || m_cycles == 0) {
        return;
    }

    for (uint16_t dot = 0; dot < (uint16_t)m_cycles * GB_DOTS_PER_M_CYCLE; dot++) {
        timer_step_one_dot(self);
    }
    timer_sync_to_ram(self);
}

void timer_add_time(struct Timer* self, uint8_t cycle)
{
    timer_step(self, cycle);
}

uint8_t timer_read_register(struct Timer* self, uint16_t address)
{
    if (!self) {
        return 0xFF;
    }

    switch (address) {
    case TIMER_DIV_ADDRESS: return timer_div_read(self);
    case TIMER_TIMA_ADDRESS: return self->reg_tima;
    case TIMER_TMA_ADDRESS: return self->reg_tma;
    case TIMER_TAC_ADDRESS: return 0xF8 | (self->reg_tac & 0x07);
    default: return 0xFF;
    }
}

void timer_write_register(struct Timer* self, uint16_t address, uint8_t value)
{
    if (!self) {
        return;
    }

    switch (address) {
    case TIMER_DIV_ADDRESS: {
        bool old_signal = timer_input_signal(self);
        self->divider   = 0;
        timer_check_falling_edge(self, old_signal);
        break;
    }
    case TIMER_TIMA_ADDRESS:
        self->reg_tima              = value;
        self->tima_overflow_pending = false;
        self->tima_overflow_dots    = 0;
        break;
    case TIMER_TMA_ADDRESS: self->reg_tma = value; break;
    case TIMER_TAC_ADDRESS: {
        bool old_signal = timer_input_signal(self);
        self->reg_tac   = value & 0x07;
        timer_check_falling_edge(self, old_signal);
        break;
    }
    default: break;
    }

    timer_sync_to_ram(self);
}

void timer_refresh_register(struct Timer* self)
{
    timer_sync_to_ram(self);
}

void timer_set_register(struct Timer* self)
{
    timer_sync_to_ram(self);
}

struct Timer* create_timer(void)
{
    struct Timer* timer = (struct Timer*)malloc(sizeof(struct Timer));
    if (timer == NULL) {
        return NULL;
    }

    timer->divider               = 0;
    timer->reg_tima              = 0;
    timer->reg_tma               = 0;
    timer->reg_tac               = 0;
    timer->tima_overflow_pending = false;
    timer->tima_overflow_dots    = 0;
    timer->ram                   = NULL;

    timer->add_time               = timer_add_time;
    timer->step                   = timer_step;
    timer->read_register          = timer_read_register;
    timer->write_register         = timer_write_register;
    timer->refresh_timer_register = timer_refresh_register;
    timer->set_timer_register     = timer_set_register;

    return timer;
}

void timer_attach_ram(struct Timer* self, struct Ram* ram)
{
    self->ram = ram;
    timer_sync_to_ram(self);
}

void free_timer(struct Timer* timer)
{
    if (timer) {
        free(timer);
    }
}
