#ifndef GAMEBOY_CPU_H
#define GAMEBOY_CPU_H

#include "mmu.h"
#include "register.h"
#include <immintrin.h>

extern struct EmulatorConfig config;

// CPU debug print
#define CPU_DEBUG_PRINT(fmt, ...)                                   \
    if (config.debug_mode && config.verbose_level >= DEBUG_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                    \
        PRINT_LEVEL(DEBUG_LEVEL);                                   \
        printf("CPU: ");                                            \
        printf(fmt, ##__VA_ARGS__);                                 \
    }

#define CPU_INFO_PRINT(fmt, ...)                                   \
    if (config.debug_mode && config.verbose_level >= INFO_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                   \
        PRINT_LEVEL(INFO_LEVEL);                                   \
        printf("CPU: ");                                           \
        printf(fmt, ##__VA_ARGS__);                                \
    }

#define CPU_TRACE_PRINT(fmt, ...)                                   \
    if (config.debug_mode && config.verbose_level >= TRACE_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                    \
        PRINT_LEVEL(TRACE_LEVEL);                                   \
        printf("CPU: ");                                            \
        printf(fmt, ##__VA_ARGS__);                                 \
    }

#define CPU_WARN_PRINT(fmt, ...)                                   \
    if (config.debug_mode && config.verbose_level >= WARN_LEVEL) { \
        PRINT_TIME_IN_SECONDS();                                   \
        PRINT_LEVEL(WARN_LEVEL);                                   \
        printf("CPU: ");                                           \
        printf(fmt, ##__VA_ARGS__);                                \
    }

#define CPU_ERROR_PRINT(fmt, ...)   \
    {                               \
        PRINT_TIME_IN_SECONDS();    \
        PRINT_LEVEL(ERROR_LEVEL);   \
        printf("CPU: ");            \
        printf(fmt, ##__VA_ARGS__); \
    }

#define CPU_EMERGENCY_PRINT(fmt, ...) \
    {                                 \
        PRINT_TIME_IN_SECONDS();      \
        PRINT_LEVEL(EMERGENCY_LEVEL); \
        printf("CPU: ");              \
        printf(fmt, ##__VA_ARGS__);   \
    }

// CPU clock speed: 4.194304 MHz
#define CPU_CLOCK_SPEED 4194304

// Interrupt flags
#define INT_VBLANK   0x01
#define INT_LCD_STAT 0x02
#define INT_TIMER    0x04
#define INT_SERIAL   0x08
#define INT_JOYPAD   0x10

// CB Prefix
#define CB_PREFIX        0xCB
#define CB_PREFIX_CYCLES 1

#define ZERO_PAGE_ADDRESS 0xFF00

// FF0F - IF - Interrupt Flag (R/W)
// Bit 0: V-Blank  Interrupt Request (INT 40h)  (1=Request)
// Bit 1: LCD STAT Interrupt Request (INT 48h)  (1=Request)
// Bit 2: Timer    Interrupt Request (INT 50h)  (1=Request)
// Bit 3: Serial   Interrupt Request (INT 58h)  (1=Request)
// Bit 4: Joypad   Interrupt Request (INT 60h)  (1=Request)
#define INTERRUPT_FLAG_ADDRESS 0xFF00

// FFFF - IE - Interrupt Enable (R/W)
// Bit 0: V-Blank  Interrupt Enable  (INT 40h)  (1=Enable)
// Bit 1: LCD STAT Interrupt Enable  (INT 48h)  (1=Enable)
// Bit 2: Timer    Interrupt Enable  (INT 50h)  (1=Enable)
// Bit 3: Serial   Interrupt Enable  (INT 58h)  (1=Enable)
// Bit 4: Joypad   Interrupt Enable  (INT 60h)  (1=Enable)
#define INTERRUPT_ENABLE_ADDRESS 0xFFFF

// interrupt vector table
#define INTERRUPT_VECTOR_VBLANK   0x0040
#define INTERRUPT_VECTOR_LCD_STAT 0x0048
#define INTERRUPT_VECTOR_TIMER    0x0050
#define INTERRUPT_VECTOR_SERIAL   0x0058
#define INTERRUPT_VECTOR_JOYPAD   0x0060

struct CPU;
struct InstructionParam;

enum JumpCondition
{
    JumpCondition_NZ = 0x00,
    JumpCondition_Z  = 0x01,
    JumpCondition_NC = 0x02,
    JumpCondition_C  = 0x03,
};

// Instruction parameter
struct InstructionParam
{
    // Register 1
    uint8_t reg_1;

    // Register 2
    uint8_t reg_2;

    // Register Pair 1
    uint8_t rp_1;

    // Register Pair 2
    uint8_t rp_2;

    // JumpCondition
    uint8_t cc;

    // bit position
    uint8_t bit_position;

    // If true, cycles_alternative is used
    bool result_is_alternative;
};

typedef void (*instruction_fn)(struct CPU*, struct InstructionParam*);

/*
 * eg: PACKED_INSTRUCTION_PARAM(ld_imm_to_register_pair, {.rp_1 = BC})
 * This is used to initialize the instruction param
 * It should expand to ld_imm_to_register_pair, struct InstructionParam{.rp_1 = BC}
 */
// #define PACKED_INSTRUCTION_PARAM(fn, param) fn, struct InstructionParam param
struct PackedInstructionParam
{
    instruction_fn          fn;
    struct InstructionParam param;
    uint8_t                 cycles_alternative;   // Cycles if condition is true
};

struct CPU
{
    // Registers
    struct Registers* registers;

    // Memory Management Unit
    struct MMU* mmu;

    // CPU state
    bool halted;                    // CPU is halted
    bool stopped;                   // CPU is stopped
    bool interrupt_master_enable;   // Interrupt Master Enable flag

    // Clock management
    uint32_t cycles;   // Current cycle count

    // Op Code
    uint8_t op_code;

    //  Cycle count for each opcode
    const uint8_t* opcode_cycle_main;

    //  Cycle count for each opcode with prefix CB
    const uint8_t* opcode_cycle_prefix_cb;

    // Public method pointers
    uint8_t (*cpu_step_next)(struct CPU*);   // Step next instruction (or interrupt)

    // instruction table (function pointers), 256 entries
    struct PackedInstructionParam* instruction_table;
    // CB prefix instruction table (function pointers), 256 entries
    struct PackedInstructionParam* instruction_table_cb;

    // interrupt vector table
    uint16_t* interrupt_vector_table;
};

static const uint16_t interrupt_vector_table[5] = {
    INTERRUPT_VECTOR_VBLANK,
    INTERRUPT_VECTOR_LCD_STAT,
    INTERRUPT_VECTOR_TIMER,
    INTERRUPT_VECTOR_SERIAL,
    INTERRUPT_VECTOR_JOYPAD,
};


// initialize opcode cycle count
//  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
static const uint8_t opcode_cycle_main[256] = {
    1, 3, 2, 2, 1, 1, 2, 1, 5, 2, 2, 2, 1, 1, 2, 1,   // 0
    1, 3, 2, 2, 1, 1, 2, 1, 3, 2, 2, 2, 1, 1, 2, 1,   // 1
    2, 3, 2, 2, 1, 1, 2, 1, 2, 2, 2, 2, 1, 1, 2, 1,   // 2
    2, 3, 2, 2, 3, 3, 3, 1, 2, 2, 2, 2, 1, 1, 2, 1,   // 3
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,   // 4
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,   // 5
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,   // 6
    2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 1, 1, 1, 1, 2, 1,   // 7
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,   // 8
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,   // 9
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,   // a
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,   // b
    2, 3, 3, 4, 3, 4, 2, 4, 2, 4, 3, 1, 3, 6, 2, 4,   // c
    2, 3, 3, 0, 3, 4, 2, 4, 2, 4, 3, 0, 3, 0, 2, 4,   // d
    3, 3, 2, 0, 0, 4, 2, 4, 4, 1, 4, 0, 0, 0, 2, 4,   // e
    3, 3, 2, 1, 0, 4, 2, 4, 3, 2, 4, 1, 0, 0, 2, 4,   // f
};

// initialize opcode cycle count for prefix CB
//  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
static const uint8_t opcode_cycle_prefix_cb[256] = {
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,   // 0
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,   // 1
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,   // 2
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,   // 3
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,   // 4
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,   // 5
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,   // 6
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,   // 7
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,   // 8
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,   // 9
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,   // a
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,   // b
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,   // c
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,   // d
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,   // e
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,   // f
};

// Define instruction function type
typedef uint8_t (*cpu_instruction_fn)(struct CPU* cpu, ...);

// Function declarations
struct CPU* create_cpu(struct Registers* registers, struct MMU* mmu);
void        free_cpu(struct CPU* cpu);

// Step next instruction (or interrupt), called by main loop
uint8_t cpu_step_next(struct CPU* cpu);

// Execute one instruction
uint8_t cpu_step(struct CPU* cpu);

// Execute Op Code
uint8_t cpu_step_execute_op_code(struct CPU* cpu, uint8_t op_byte);

// Execute CB Prefix
uint8_t cpu_step_execute_prefix_cb(struct CPU* cpu);

// Execute CB Op Code
uint8_t cpu_step_execute_cb_op_code(struct CPU* cpu, uint8_t op_byte);

// Execute Main Op Code
uint8_t cpu_step_execute_main(struct CPU* cpu, uint8_t op_byte);

// Execute instruction
uint8_t cpu_step_execute_instruction(struct CPU* cpu, uint8_t (*instruction)(struct CPU*));

// Read byte from MMU
uint8_t cpu_step_read_byte(struct CPU* cpu);

// Read word from MMU
uint16_t cpu_step_read_word(struct CPU* cpu);

// Interrupt Master Enable flag
bool cpu_get_interrupt_master_enable(struct CPU* cpu);

// Set Interrupt Master Enable flag
void cpu_set_interrupt_master_enable(struct CPU* cpu, bool enable);

// instruction methods

#define EXECUTABLE_INSTRUCTION(fn_name) \
    void fn_name(struct CPU* cpu, struct InstructionParam* param)

// abort when invalid opcode is executed
EXECUTABLE_INSTRUCTION(cpu_invalid_opcode);

// LD

// 8-bit load instructions

// Load immediate value to register
EXECUTABLE_INSTRUCTION(ld_imm_to_register);

// Load register value to immediate value
EXECUTABLE_INSTRUCTION(ld_register_to_imm);

// Load register value to register
EXECUTABLE_INSTRUCTION(ld_register_to_register);

// Load address value to register
EXECUTABLE_INSTRUCTION(ld_address_register_pair_to_register);

// Load register value to address
EXECUTABLE_INSTRUCTION(ld_register_to_address_register_pair);

// Load value from register A to zero page address with register C
EXECUTABLE_INSTRUCTION(ld_a_to_zero_page_address_c);

// Load value from zero page address with register C to register A
EXECUTABLE_INSTRUCTION(ld_zero_page_address_c_to_a);

// Load value from register to address HL
EXECUTABLE_INSTRUCTION(ld_register_to_address_hl);

// Load value from address HL to register
EXECUTABLE_INSTRUCTION(ld_address_hl_to_register);

// Load value from immediate value to address HL
EXECUTABLE_INSTRUCTION(ld_imm_to_address_hl);

// load value from address HL to register A, increment HL
EXECUTABLE_INSTRUCTION(ld_address_hl_to_a_inc_hl);

// load value from register A to address HL, increment HL
EXECUTABLE_INSTRUCTION(ld_a_to_address_hl_inc_hl);

// load value from address HL to register A, decrement HL
EXECUTABLE_INSTRUCTION(ld_address_hl_to_a_dec_hl);

// load value from register A to address HL, decrement HL
EXECUTABLE_INSTRUCTION(ld_a_to_address_hl_dec_hl);

// load value from register A to zero page address with immediate value
EXECUTABLE_INSTRUCTION(ld_a_to_zero_page_address_imm);

// load value from zero page address with immediate value to register A
EXECUTABLE_INSTRUCTION(ld_zero_page_address_imm_to_a);

// load value from register A to address with immediate value
EXECUTABLE_INSTRUCTION(ld_a_to_address_imm);

// load value from address with immediate value to register A
EXECUTABLE_INSTRUCTION(ld_address_imm_to_a);

// 16-bit load instructions

// load value from immediate value to register pair
EXECUTABLE_INSTRUCTION(ld_imm_to_register_pair);

// Load immediate value to SP
EXECUTABLE_INSTRUCTION(ld_imm_to_sp);

// load from HL to SP
EXECUTABLE_INSTRUCTION(ld_hl_to_sp);

// load SP + one byte signed immediate value to HL
EXECUTABLE_INSTRUCTION(ld_sp_plus_imm_to_hl);

// Load SP to immediate value address
EXECUTABLE_INSTRUCTION(ld_sp_to_address_imm);

// 16-bit stack instructions

// push register pair to stack
EXECUTABLE_INSTRUCTION(push_register_pair);

// pop register pair from stack
// IMPORTANT: POP AF sets ALL flags
EXECUTABLE_INSTRUCTION(pop_register_pair);

// 8-bit ALU instructions

// ADD
// add n to register A
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Set if carry from bit 3 to bit 4
// C: Set if carry from bit 7 to bit 8
void add_a(struct CPU* cpu, uint8_t* value);

// add value from immediate value to register A
EXECUTABLE_INSTRUCTION(add_imm_to_a);

// add value from register ? to register A
EXECUTABLE_INSTRUCTION(add_register_to_a);

// add value from address HL to register A
EXECUTABLE_INSTRUCTION(add_address_hl_to_a);

// ADC
// add value from register A and carry flag to register A
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Set if carry from bit 3 to bit 4
// C: Set if carry from bit 7 to bit 8
void adc_a(struct CPU* cpu, uint8_t* value);

// add value from immediate value to register A and carry flag
EXECUTABLE_INSTRUCTION(adc_imm_to_a);

// add value from register ? to register A and carry flag
EXECUTABLE_INSTRUCTION(adc_register_to_a);

// add value from address HL to register A and carry flag
EXECUTABLE_INSTRUCTION(adc_address_hl_to_a);

// SUB
// subtract n from register A
// Flags:
// Z: Set if result is zero
// N: Set
// H: Set if no borrow from bit 4
// C: Set if no borrow
void sub_a(struct CPU* cpu, uint8_t* value);

// subtract value from immediate value from register A
EXECUTABLE_INSTRUCTION(sub_imm_to_a);

// subtract value from register ? from register A
EXECUTABLE_INSTRUCTION(sub_register_to_a);

// subtract value from address HL from register A
EXECUTABLE_INSTRUCTION(sub_address_hl_to_a);

// SBC
// subtract value from register A and carry flag from register A
// Flags:
// Z: Set if result is zero
// N: Set
// H: Set if no borrow from bit 4
// C: Set if no borrow
void sbc_a(struct CPU* cpu, uint8_t* value);

// subtract value from immediate value from register A and carry flag
EXECUTABLE_INSTRUCTION(sbc_imm_to_a);

// subtract value from register ? from register A and carry flag
EXECUTABLE_INSTRUCTION(sbc_register_to_a);

// subtract value from address HL from register A and carry flag
EXECUTABLE_INSTRUCTION(sbc_address_hl_to_a);

// AND
// logically AND n with register A, result in register A
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Set
// C: Reset
void and_a(struct CPU* cpu, uint8_t* value);

// logically AND immediate value with register A, result in register A
EXECUTABLE_INSTRUCTION(and_imm_to_a);

// logically AND register ? with register A, result in register A
EXECUTABLE_INSTRUCTION(and_register_to_a);

// logically AND address HL with register A, result in register A
EXECUTABLE_INSTRUCTION(and_address_hl_to_a);

// OR
// logically OR n with register A, result in register A
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Reset
void or_a(struct CPU* cpu, uint8_t* value);

// logically OR immediate value with register A, result in register A
EXECUTABLE_INSTRUCTION(or_imm_to_a);

// logically OR register ? with register A, result in register A
EXECUTABLE_INSTRUCTION(or_register_to_a);

// logically OR address HL with register A, result in register A
EXECUTABLE_INSTRUCTION(or_address_hl_to_a);

// XOR
// logically XOR n with register A, result in register A
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Reset
void xor_a(struct CPU* cpu, uint8_t* value);

// logically XOR immediate value with register A, result in register A
EXECUTABLE_INSTRUCTION(xor_imm_to_a);

// logically XOR register ? with register A, result in register A
EXECUTABLE_INSTRUCTION(xor_register_to_a);

// logically XOR address HL with register A, result in register A
EXECUTABLE_INSTRUCTION(xor_address_hl_to_a);

// CP
// compare register A with n
// Flags:
// Z: Set if the result is zero (A == n)
// N: Set
// H: Set if no borrow from bit 4
// C: Set if A < n
void cp_a(struct CPU* cpu, uint8_t* value);

// compare immediate value with register A
EXECUTABLE_INSTRUCTION(cp_imm_to_a);

// compare register ? with register A
EXECUTABLE_INSTRUCTION(cp_register_to_a);

// compare address HL with register A
EXECUTABLE_INSTRUCTION(cp_address_hl_to_a);

// INC
// increment
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Set if carry from bit 3 to bit 4
// C: Not affected
void inc(struct CPU* cpu, uint8_t* value);

// INC r
EXECUTABLE_INSTRUCTION(inc_register);

// INC Address HL
EXECUTABLE_INSTRUCTION(inc_address_hl);

// DEC
// decrement
// Flags:
// Z: Set if result is zero
// N: Set
// H: Set if no borrow from bit 4
// C: Not affected
void dec(struct CPU* cpu, uint8_t* value);

// DEC r
EXECUTABLE_INSTRUCTION(dec_register);

// DEC Address HL
EXECUTABLE_INSTRUCTION(dec_address_hl);

// 16-bit ALU instructions

// ADD HL
// add to HL
// Flags:
// Z: Not affected
// N: Reset
// H: Set if carry from bit 11 to bit 12
// C: Set if carry from bit 15 to bit 16
void add_hl(struct CPU* cpu, uint16_t* value);

// ADD HL, rr
// add register pair to HL
EXECUTABLE_INSTRUCTION(add_hl_to_register_pair);

// ADD HL, SP
// add SP to HL
EXECUTABLE_INSTRUCTION(add_sp_to_hl);

// ADD SP, n
// add one byte signed immediate value to SP
// Flags:
// Z: Reset
// N: Reset
// H: Set if carry from bit 11 to bit 12
// C: Set if carry from bit 15 to bit 16
EXECUTABLE_INSTRUCTION(add_imm_to_sp);

// INC nn
// increment register
// Flags:
// None
void inc_16_bit(struct CPU* cpu, uint16_t* value);

// increment register pair
EXECUTABLE_INSTRUCTION(inc_register_pair);

// increment SP
EXECUTABLE_INSTRUCTION(inc_sp);

// DEC nn
// decrement register
// Flags:
// None
void dec_16_bit(struct CPU* cpu, uint16_t* value);

// decrement register pair
EXECUTABLE_INSTRUCTION(dec_register_pair);

// decrement SP
EXECUTABLE_INSTRUCTION(dec_sp);

// 8-bit bit manipulation instructions

// SWAP r
// swap lower and upper nibbles of a value
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Reset
uint8_t swap(struct CPU* cpu, uint8_t value);

// swap lower and upper nibbles of a register
EXECUTABLE_INSTRUCTION(swap_register);

// swap lower and upper nibbles of a value at address HL
EXECUTABLE_INSTRUCTION(swap_address_hl);

// DAA
// decimal adjust register A
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Set if carry from bit 7 to bit 8
EXECUTABLE_INSTRUCTION(daa);

// CPL
// complement register A (Flip all bits)
// Flags:
// Z: Not affected
// N: Set
// H: Set
// C: Not affected
EXECUTABLE_INSTRUCTION(cpl);

// CCF
// complement carry flag
// If C flag is set, it will be reset.
// If C flag is reset, it will be set.
// Flags:
// Z: Not affected
// N: Reset
// H: Reset
// C: Complemented
EXECUTABLE_INSTRUCTION(ccf);

// SCF
// set carry flag
// Flags:
// Z: Not affected
// N: Reset
// H: Reset
// C: Set
EXECUTABLE_INSTRUCTION(scf);

// NOP
// No operation
EXECUTABLE_INSTRUCTION(nop);

// HALT
// Halt the CPU
EXECUTABLE_INSTRUCTION(halt);

// STOP
// Halt CPU and LCD until button is pressed
EXECUTABLE_INSTRUCTION(stop);

// DI
// Disable interrupts but not immediate
// Interrupt will be disabled after the current instruction (after DI is executed)
EXECUTABLE_INSTRUCTION(di);

// EI
// Enable interrupts but not immediate
// Interrupt will be enabled after the current instruction (after EI is executed)
EXECUTABLE_INSTRUCTION(ei);

// 8-bit Rotate and Shift instructions

// RLCA
// rotate register A left, old bit 7 to carry flag
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 7 data (before rotation)
EXECUTABLE_INSTRUCTION(rlca);

// RLA
// rotate register A left through carry flag
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 7 data (before rotation)
EXECUTABLE_INSTRUCTION(rla);

// RRCA
// rotate register A right, old bit 0 to carry flag
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 0 data (before rotation)
EXECUTABLE_INSTRUCTION(rrca);

// RRA
// rotate register A right through carry flag
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 0 data (before rotation)
EXECUTABLE_INSTRUCTION(rra);

// RLC
// rotate left, old bit 7 to carry flag
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 7 data (before rotation)
uint8_t rlc(struct CPU* cpu, uint8_t value);

// RLC r
EXECUTABLE_INSTRUCTION(rlc_register);

// RLC (HL)
EXECUTABLE_INSTRUCTION(rlc_address_hl);

// RL
// rotate left through carry flag
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 7 data (before rotation)
uint8_t rl(struct CPU* cpu, uint8_t value);

// RL r
EXECUTABLE_INSTRUCTION(rl_register);

// RL (HL)
EXECUTABLE_INSTRUCTION(rl_address_hl);

// RRC
// rotate right, old bit 0 to carry flag
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 0 data (before rotation)
uint8_t rrc(struct CPU* cpu, uint8_t value);

// RRC r
EXECUTABLE_INSTRUCTION(rrc_register);

// RRC (HL)
EXECUTABLE_INSTRUCTION(rrc_address_hl);

// RR
// rotate right through carry flag
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 0 data (before rotation)
uint8_t rr(struct CPU* cpu, uint8_t value);

// RR r
EXECUTABLE_INSTRUCTION(rr_register);

// RR (HL)
EXECUTABLE_INSTRUCTION(rr_address_hl);

// SLA
// shift left into carry flag, LSB of value set to 0
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 7 data (before shift)
uint8_t sla(struct CPU* cpu, uint8_t value);

// SLA r
EXECUTABLE_INSTRUCTION(sla_register);

// SLA (HL)
EXECUTABLE_INSTRUCTION(sla_address_hl);

// SRA
// shift right into carry flag, MSB doesn't change
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 0 data (before shift)
uint8_t sra(struct CPU* cpu, uint8_t value);

// SRA r
EXECUTABLE_INSTRUCTION(sra_register);

// SRA (HL)
EXECUTABLE_INSTRUCTION(sra_address_hl);

// SRL
// shift right into carry flag, MSB set to 0
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 0 data (before shift)
uint8_t srl(struct CPU* cpu, uint8_t value);

// SRL r
EXECUTABLE_INSTRUCTION(srl_register);

// SRL (HL)
EXECUTABLE_INSTRUCTION(srl_address_hl);

// bit operations

// BIT b, n
// test bit b of n
// Flags:
// Z: Set if bit b of n is 0
// N: Reset
// H: Set
// C: Not affected
uint8_t bit(struct CPU* cpu, uint8_t bit, uint8_t value);

// BIT b, r
EXECUTABLE_INSTRUCTION(bit_register);

// BIT b, (HL)
EXECUTABLE_INSTRUCTION(bit_address_hl);

// SET b, n
// set bit b of n
// Flags:
// None
uint8_t set(struct CPU* cpu, uint8_t bit, uint8_t value);

// SET b, r
EXECUTABLE_INSTRUCTION(set_register);

// SET b, (HL)
EXECUTABLE_INSTRUCTION(set_address_hl);

// RES b, n
// reset bit b of n
// Flags:
// None
uint8_t res(struct CPU* cpu, uint8_t bit, uint8_t value);

// RES b, r
EXECUTABLE_INSTRUCTION(res_register);

// RES b, (HL)
EXECUTABLE_INSTRUCTION(res_address_hl);

// 8-bit jump instructions

// JP nn
// jump to address nn
// nn: 16-bit immediate value (LSB first)
EXECUTABLE_INSTRUCTION(jp_imm);

// JP cc, nn
// jump to address nn if condition cc is true
// cc: JumpCondition
// nn: 16-bit immediate value (LSB first)
EXECUTABLE_INSTRUCTION(jp_cc_imm);

// JP (HL)
EXECUTABLE_INSTRUCTION(jp_address_hl);

// JR n
// Add n to PC and jump to it
// n: one byte signed immediate value
EXECUTABLE_INSTRUCTION(jr_imm);

// JR cc, n
// Add n to PC and jump to it if condition cc is true
// cc: JumpCondition
// n: one byte signed immediate value
EXECUTABLE_INSTRUCTION(jr_cc_imm);

// Calls

// CALL nn
// call subroutine at address nn
// Push address of next instruction to stack
// then jump to address nn
// nn: 16-bit immediate value (LSB first)
EXECUTABLE_INSTRUCTION(call_imm);

// CALL cc, nn
// call subroutine at address nn if condition cc is true
// Push address of next instruction to stack
// then jump to address nn
// cc: JumpCondition
// nn: 16-bit immediate value (LSB first)
EXECUTABLE_INSTRUCTION(call_cc_imm);

// Restarts

// RST n
// jump to address n
// they are hard-coded in the instruction set
// 0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38
void rst(struct CPU* cpu, uint8_t n);

// 0x00
EXECUTABLE_INSTRUCTION(rst_00h);

// 0x08
EXECUTABLE_INSTRUCTION(rst_08h);

// 0x10
EXECUTABLE_INSTRUCTION(rst_10h);

// 0x18
EXECUTABLE_INSTRUCTION(rst_18h);

// 0x20
EXECUTABLE_INSTRUCTION(rst_20h);

// 0x28
EXECUTABLE_INSTRUCTION(rst_28h);

// 0x30
EXECUTABLE_INSTRUCTION(rst_30h);

// 0x38
EXECUTABLE_INSTRUCTION(rst_38h);

// Returns

// RET
// return from subroutine
// Pop two bytes (address) from stack to PC
// and jump to it
EXECUTABLE_INSTRUCTION(ret);

// RET cc
// return from subroutine if condition cc is true
// Pop two bytes (address) from stack to PC
// and jump to it if condition cc is true
// cc: JumpCondition
EXECUTABLE_INSTRUCTION(ret_cc);

// RETI
// return from subroutine and enable interrupts
// Pop two bytes (address) from stack to PC
// and jump to it
// then enable interrupts
EXECUTABLE_INSTRUCTION(reti);

// Pseudo instructions

// PREFIX CB
EXECUTABLE_INSTRUCTION(prefix_cb);

#endif
