#ifndef GAMEBOY_CPU_H
#define GAMEBOY_CPU_H

#include "mmu.h"
#include "register.h"

extern struct EmulatorConfig config;

// CPU debug print
#define CPU_DEBUG_PRINT(fmt, ...)                                 \
    if (config.debug_mode && config.verbose_level >= DEBUG_LEVEL) \
    {                                                             \
        PRINT_TIME_IN_SECONDS();                                  \
        PRINT_LEVEL(DEBUG_LEVEL);                                 \
        printf("CPU: ");                                          \
        printf(fmt, ##__VA_ARGS__);                               \
    }

#define CPU_INFO_PRINT(fmt, ...)                                 \
    if (config.debug_mode && config.verbose_level >= INFO_LEVEL) \
    {                                                            \
        PRINT_TIME_IN_SECONDS();                                 \
        PRINT_LEVEL(INFO_LEVEL);                                 \
        printf("CPU: ");                                         \
        printf(fmt, ##__VA_ARGS__);                              \
    }

#define CPU_TRACE_PRINT(fmt, ...)                                 \
    if (config.debug_mode && config.verbose_level >= TRACE_LEVEL) \
    {                                                             \
        PRINT_TIME_IN_SECONDS();                                  \
        PRINT_LEVEL(TRACE_LEVEL);                                 \
        printf("CPU: ");                                          \
        printf(fmt, ##__VA_ARGS__);                               \
    }

#define CPU_WARN_PRINT(fmt, ...)                                 \
    if (config.debug_mode && config.verbose_level >= WARN_LEVEL) \
    {                                                            \
        PRINT_TIME_IN_SECONDS();                                 \
        PRINT_LEVEL(WARN_LEVEL);                                 \
        printf("CPU: ");                                         \
        printf(fmt, ##__VA_ARGS__);                              \
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
#define INT_VBLANK 0x01
#define INT_LCD_STAT 0x02
#define INT_TIMER 0x04
#define INT_SERIAL 0x08
#define INT_JOYPAD 0x10

// CB Prefix
#define CB_PREFIX 0xCB

typedef uint8_t (*instruction_fn)(struct CPU *, struct InstructionParam *);

struct InstructionParam
{
    enum Register reg_1;
    enum Register reg_2;
    enum RegisterPair rp_1;
    enum RegisterPair rp_2;
    uint8_t value;
};

struct CPU
{
    // Registers
    struct Registers *registers;

    // Memory Management Unit
    struct MMU *mmu;

    // CPU state
    bool halted;  // CPU is halted
    bool stopped; // CPU is stopped
    bool ime;     // Interrupt Master Enable flag

    // Clock management
    uint32_t cycles; // Current cycle count

    // Op Code
    uint8_t op_code;

    //  Cycle count for each opcode
    const uint8_t *opcode_cycle_main;

    //  Cycle count for each opcode with prefix CB
    const uint8_t *opcode_cycle_prefix_cb;

    // Public method pointers
    uint8_t (*cpu_step_next)(struct CPU *); // Step next instruction (or interrupt)

    // instruction table (function pointers), 256 entries
    instruction_fn *instruction_table;
    struct InstructionParam *instruction_param;
};

// initialize opcode cycle count
//  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
const static uint8_t opcode_cycle_main[256] = {
    1, 3, 2, 2, 1, 1, 2, 1, 5, 2, 2, 2, 1, 1, 2, 1, // 0
    0, 3, 2, 2, 1, 1, 2, 1, 3, 2, 2, 2, 1, 1, 2, 1, // 1
    2, 3, 2, 2, 1, 1, 2, 1, 2, 2, 2, 2, 1, 1, 2, 1, // 2
    2, 3, 2, 2, 3, 3, 3, 1, 2, 2, 2, 2, 1, 1, 2, 1, // 3
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // 4
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // 5
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // 6
    2, 2, 2, 2, 2, 2, 0, 2, 1, 1, 1, 1, 1, 1, 2, 1, // 7
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // 8
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // 9
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // a
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // b
    2, 3, 3, 4, 3, 4, 2, 4, 2, 4, 3, 0, 3, 6, 2, 4, // c
    2, 3, 3, 0, 3, 4, 2, 4, 2, 4, 3, 0, 3, 0, 2, 4, // d
    3, 3, 2, 0, 0, 4, 2, 4, 4, 1, 4, 0, 0, 0, 2, 4, // e
    3, 3, 2, 1, 0, 4, 2, 4, 3, 2, 4, 1, 0, 0, 2, 4, // f
};

// initialize opcode cycle count for prefix CB
//  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
const static uint8_t opcode_cycle_prefix_cb[256] = {
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // 0
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // 1
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // 2
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // 3
    2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2, // 4
    2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2, // 5
    2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2, // 6
    2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2, // 7
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // 8
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // 9
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // a
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // b
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // c
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // d
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // e
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, // f
};

// Define instruction function type
typedef uint8_t (*cpu_instruction_fn)(struct CPU *cpu, ...);

// Function declarations
struct CPU *create_cpu(struct Registers *registers, struct MMU *mmu);
void free_cpu(struct CPU *cpu);

// Step next instruction (or interrupt), called by main loop
uint8_t cpu_step_next(struct CPU *cpu);

// Execute one instruction
uint8_t cpu_step(struct CPU *cpu);

// Execute Op Code
uint8_t cpu_step_execute_op_code(struct CPU *cpu, uint8_t op_byte);

// Execute CB Prefix
uint8_t cpu_step_execute_cb_prefix(struct CPU *cpu);

// Execute CB Op Code
uint8_t cpu_step_execute_cb_op_code(struct CPU *cpu, uint8_t op_byte);

// Execute Main Op Code
uint8_t cpu_step_execute_main(struct CPU *cpu, uint8_t op_byte);

// Execute instruction
uint8_t cpu_step_execute_instruction(struct CPU *cpu, uint8_t (*instruction)(struct CPU *));

// Read byte from MMU
uint8_t cpu_step_read_byte(struct CPU *cpu);

// Read word from MMU
uint16_t cpu_step_read_word(struct CPU *cpu);

// abort when invalid opcode is executed
void cpu_invalid_opcode(struct CPU *cpu);

// instruction methods

// LD

// 8-bit load instructions

// Load immediate value to register
void ld_imm_to_register(struct CPU *cpu, enum Register to_register);

// Load register value to immediate value
void ld_register_to_imm(struct CPU *cpu, enum Register from_register);

// Load register value to register
void ld_register_to_register(struct CPU *cpu, enum Register from_register, enum Register to_register);

// Load address value to register
void ld_address_register_pair_to_register(struct CPU *cpu, struct InstructionParam *param);

// Load register value to address
void ld_register_to_address_register_pair(struct CPU *cpu, struct InstructionParam *param);

// Load value from register A to zero page address with register C
void ld_register_a_to_zero_page_address_c(struct CPU *cpu);

// Load value from zero page address with register C to register A
void ld_zero_page_address_c_to_register_a(struct CPU *cpu);

// load value from address HL to register A, increment HL
void ld_address_hl_to_register_a_inc_hl(struct CPU *cpu);

// load value from register A to address HL, increment HL
void ld_register_a_to_address_hl_inc_hl(struct CPU *cpu);

// load value from address HL to register A, decrement HL
void ld_address_hl_to_register_a_dec_hl(struct CPU *cpu);

// load value from register A to address HL, decrement HL
void ld_register_a_to_address_hl_dec_hl(struct CPU *cpu);

// load value from register A to zero page address with immediate value
void ld_register_a_to_zero_page_address_imm(struct CPU *cpu);

// load value from zero page address with immediate value to register A
void ld_zero_page_address_imm_to_register_a(struct CPU *cpu);

// 16-bit load instructions

// load value from immediate value to register pair
void ld_imm_to_register_pair(struct CPU *cpu, struct InstructionParam *param);

// load from HL to SP
void ld_hl_to_sp(struct CPU *cpu);

// load SP + one byte signed immediate value to HL
void ld_sp_imm_to_hl(struct CPU *cpu);

// Load SP to immediate value address
void ld_sp_to_address_imm(struct CPU *cpu);

// 16-bit stack instructions

// push register pair to stack
void push_register_pair(struct CPU *cpu, enum RegisterPair register_pair);

// pop register pair from stack
void pop_register_pair(struct CPU *cpu, enum RegisterPair register_pair);

// 8-bit ALU instructions

// ADD
// add n to register A
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Set if carry from bit 3 to bit 4
// C: Set if carry from bit 7 to bit 8
void add_a(struct CPU *cpu);

// add value from immediate value to register A
void add_a_imm(struct CPU *cpu);

// add value from register ? to register A
void add_a_register(struct CPU *cpu, enum Register register);

// add value from address HL to register A
void add_a_address_hl(struct CPU *cpu);

// ADC
// add value from register A and carry flag to register A
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Set if carry from bit 3 to bit 4
// C: Set if carry from bit 7 to bit 8
void adc_a(struct CPU *cpu);

// add value from immediate value to register A and carry flag
void adc_a_imm(struct CPU *cpu);

// add value from register ? to register A and carry flag
void adc_a_register(struct CPU *cpu, enum Register register);

// add value from address HL to register A and carry flag
void adc_a_address_hl(struct CPU *cpu);

// SUB
// subtract n from register A
// Flags:
// Z: Set if result is zero
// N: Set
// H: Set if no borrow from bit 4
// C: Set if no borrow
void sub_a(struct CPU *cpu);

// subtract value from immediate value from register A
void sub_a_imm(struct CPU *cpu);

// subtract value from register ? from register A
void sub_a_register(struct CPU *cpu, enum Register register);

// subtract value from address HL from register A
void sub_a_address_hl(struct CPU *cpu);

// SBC
// subtract value from register A and carry flag from register A
// Flags:
// Z: Set if result is zero
// N: Set
// H: Set if no borrow from bit 4
// C: Set if no borrow
void sbc_a(struct CPU *cpu);

// subtract value from immediate value from register A and carry flag
void sbc_a_imm(struct CPU *cpu);

// subtract value from register ? from register A and carry flag
void sbc_a_register(struct CPU *cpu, enum Register register);

// subtract value from address HL from register A and carry flag
void sbc_a_address_hl(struct CPU *cpu);

// AND
// logically AND n with register A, result in register A
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Set
// C: Reset
void and_a(struct CPU *cpu);

// logically AND immediate value with register A, result in register A
void and_a_imm(struct CPU *cpu);

// logically AND register ? with register A, result in register A
void and_a_register(struct CPU *cpu, enum Register register);

// logically AND address HL with register A, result in register A
void and_a_address_hl(struct CPU *cpu);

// OR
// logically OR n with register A, result in register A
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Reset
void or_a(struct CPU *cpu);

// logically OR immediate value with register A, result in register A
void or_a_imm(struct CPU *cpu);

// logically OR register ? with register A, result in register A
void or_a_register(struct CPU *cpu, enum Register register);

// logically OR address HL with register A, result in register A
void or_a_address_hl(struct CPU *cpu);

// XOR
// logically XOR n with register A, result in register A
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Reset
void xor_a(struct CPU *cpu);

// logically XOR immediate value with register A, result in register A
void xor_a_imm(struct CPU *cpu);

// logically XOR register ? with register A, result in register A
void xor_a_register(struct CPU *cpu, enum Register register);

// logically XOR address HL with register A, result in register A
void xor_a_address_hl(struct CPU *cpu);

// CP
// compare register A with n
// Flags:
// Z: Set if the result is zero (A == n)
// N: Set
// H: Set if no borrow from bit 4
// C: Set if A < n
void cp_a(struct CPU *cpu);

// compare immediate value with register A
void cp_a_imm(struct CPU *cpu);

// compare register ? with register A
void cp_a_register(struct CPU *cpu, enum Register register);

// compare address HL with register A
void cp_a_address_hl(struct CPU *cpu);

// INC
// increment
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Set if carry from bit 3 to bit 4
// C: Not affected
void inc(struct CPU *cpu, uint8_t *value);

// INC r
void inc_register(struct CPU *cpu, enum Register register);

// INC HL
void inc_address_hl(struct CPU *cpu);

// DEC
// decrement
// Flags:
// Z: Set if result is zero
// N: Set
// H: Set if no borrow from bit 4
// C: Not affected
void dec(struct CPU *cpu, uint8_t *value);

// DEC r
void dec_register(struct CPU *cpu, enum Register register);

// DEC HL
void dec_address_hl(struct CPU *cpu);

// 16-bit ALU instructions

// ADD HL, rr
// add register pair to HL
// Flags:
// Z: Not affected
// N: Reset
// H: Set if carry from bit 11 to bit 12
// C: Set if carry from bit 15 to bit 16
void add_hl_rr(struct CPU *cpu, enum RegisterPair rr);

// ADD SP, n
// add one byte signed immediate value to SP
// Flags:
// Z: Reset
// N: Reset
// H: Set if carry from bit 11 to bit 12
// C: Set if carry from bit 15 to bit 16
void add_sp_imm(struct CPU *cpu);

// INC nn
// increment register
// Flags:
// None
void inc_16_bit(struct CPU *cpu, uint16_t *value);

// increment register pair
void inc_register_pair(struct CPU *cpu, struct InstructionParam *param);

// increment SP
void inc_sp(struct CPU *cpu);

// DEC nn
// decrement register
// Flags:
// None
void dec_16_bit(struct CPU *cpu, uint16_t *value);

// decrement register pair
void dec_register_pair(struct CPU *cpu, enum RegisterPair register_pair);

// decrement SP
void dec_sp(struct CPU *cpu);

// 8-bit bit manipulation instructions

// SWAP r
// swap lower and upper nibbles of a value
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Reset
void swap(struct CPU *cpu, uint8_t *value);

// swap lower and upper nibbles of a register
void swap_register(struct CPU *cpu, enum Register register);

// swap lower and upper nibbles of a value at address HL
void swap_address_hl(struct CPU *cpu);

// DAA
// decimal adjust register A
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Set if carry from bit 7 to bit 8
void daa(struct CPU *cpu);

// CPL
// complement register A (Flip all bits)
// Flags:
// Z: Not affected
// N: Set
// H: Set
// C: Not affected
void cpl(struct CPU *cpu);

// CCF
// complement carry flag
// If C flag is set, it will be reset.
// If C flag is reset, it will be set.
// Flags:
// Z: Not affected
// N: Reset
// H: Reset
// C: Complemented
void ccf(struct CPU *cpu);

// SCF
// set carry flag
// Flags:
// Z: Not affected
// N: Reset
// H: Reset
// C: Set
void scf(struct CPU *cpu);

// NOP
// No operation
void nop(struct CPU *cpu);

// HALT
// Halt the CPU
void halt(struct CPU *cpu);

// STOP
// Halt CPU and LCD until button is pressed
void stop(struct CPU *cpu);

// DI
// Disable interrupts but not immediate
// Interrupt will be disabled after the current instruction (after DI is executed)
void di(struct CPU *cpu);

// EI
// Enable interrupts but not immediate
// Interrupt will be enabled after the current instruction (after EI is executed)
void ei(struct CPU *cpu);

// 8-bit Rotate and Shift instructions

// RLCA
// rotate register A left, old bit 7 to carry flag
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 7 data (before rotation)
void rlca(struct CPU *cpu);

// RLA
// rotate register A left through carry flag
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 7 data (before rotation)
void rla(struct CPU *cpu);

// RRCA
// rotate register A right, old bit 0 to carry flag
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 0 data (before rotation)
void rrca(struct CPU *cpu);

// RRA
// rotate register A right through carry flag
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 0 data (before rotation)
void rra(struct CPU *cpu);

// RLC
// rotate left, old bit 7 to carry flag
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 7 data (before rotation)
void rlc(struct CPU *cpu, uint8_t *value);

// RLC r
void rlc_register(struct CPU *cpu, enum Register register);

// RLC (HL)
void rlc_address_hl(struct CPU *cpu);

// RL
// rotate left through carry flag
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 7 data (before rotation)
void rl(struct CPU *cpu, uint8_t *value);

// RL r
void rl_register(struct CPU *cpu, enum Register register);

// RL (HL)
void rl_address_hl(struct CPU *cpu);

// RRC
// rotate right, old bit 0 to carry flag
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 0 data (before rotation)
void rrc(struct CPU *cpu, uint8_t *value);

// RRC r
void rrc_register(struct CPU *cpu, enum Register register);

// RRC (HL)
void rrc_address_hl(struct CPU *cpu);

// RR
// rotate right through carry flag
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 0 data (before rotation)
void rr(struct CPU *cpu, uint8_t *value);

// RR r
void rr_register(struct CPU *cpu, enum Register register);

// RR (HL)
void rr_address_hl(struct CPU *cpu);

// SLA
// shift left into carry flag, LSB of value set to 0
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 7 data (before shift)
void sla(struct CPU *cpu, uint8_t *value);

// SLA r
void sla_register(struct CPU *cpu, enum Register register);

// SLA (HL)
void sla_address_hl(struct CPU *cpu);

// SRA
// shift right into carry flag, MSB doesn't change
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 0 data (before shift)
void sra(struct CPU *cpu, uint8_t *value);

// SRA r
void sra_register(struct CPU *cpu, enum Register register);

// SRA (HL)
void sra_address_hl(struct CPU *cpu);

// SRL
// shift right into carry flag, MSB set to 0
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 0 data (before shift)
void srl(struct CPU *cpu, uint8_t *value);

// SRL r
void srl_register(struct CPU *cpu, enum Register register);

// SRL (HL)
void srl_address_hl(struct CPU *cpu);

// bit operations

// BIT b, n
// test bit b of n
// Flags:
// Z: Set if bit b of n is 0
// N: Reset
// H: Set
// C: Not affected
void bit(struct CPU *cpu, uint8_t bit, uint8_t *value);

// BIT b, r
void bit_register(struct CPU *cpu, uint8_t bit, enum Register register);

// BIT b, (HL)
void bit_address_hl(struct CPU *cpu, uint8_t bit);

// SET b, n
// set bit b of n
// Flags:
// None
void set(struct CPU *cpu, uint8_t bit, uint8_t *value);

// SET b, r
void set_register(struct CPU *cpu, uint8_t bit, enum Register register);

// SET b, (HL)
void set_address_hl(struct CPU *cpu, uint8_t bit);

// RES b, n
// reset bit b of n
// Flags:
// None
void res(struct CPU *cpu, uint8_t bit, uint8_t *value);

// RES b, r
void res_register(struct CPU *cpu, uint8_t bit, enum Register register);

// RES b, (HL)
void res_address_hl(struct CPU *cpu, uint8_t bit);

// 8-bit jump instructions

// JP nn
// jump to address nn
// nn: 16-bit immediate value (LSB first)
void jp_imm(struct CPU *cpu);

enum JumpCondition
{
    JumpCondition_NZ = 0x00,
    JumpCondition_Z = 0x01,
    JumpCondition_NC = 0x02,
    JumpCondition_C = 0x03
};

// JP cc, nn
// jump to address nn if condition cc is true
// cc: JumpCondition
// nn: 16-bit immediate value (LSB first)
void jp_cc_imm(struct CPU *cpu, enum JumpCondition cc);

// JP (HL)
void jp_address_hl(struct CPU *cpu);

// JR n
// Add n to PC and jump to it
// n: one byte signed immediate value
void jr_imm(struct CPU *cpu);

// JR cc, n
// Add n to PC and jump to it if condition cc is true
// cc: JumpCondition
// n: one byte signed immediate value
void jr_cc_imm(struct CPU *cpu, enum JumpCondition cc);

// Calls

// CALL nn
// call subroutine at address nn
// Push address of next instruction to stack
// then jump to address nn
// nn: 16-bit immediate value (LSB first)
void call_imm(struct CPU *cpu);

// CALL cc, nn
// call subroutine at address nn if condition cc is true
// Push address of next instruction to stack
// then jump to address nn
// cc: JumpCondition
// nn: 16-bit immediate value (LSB first)
void call_cc_imm(struct CPU *cpu, enum JumpCondition cc);

// Restarts

// RST n
// jump to address n
// they are hard-coded in the instruction set
// 0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38
void rst(struct CPU *cpu, uint8_t n);

// 0x00
void rst_00(struct CPU *cpu);

// 0x08
void rst_08(struct CPU *cpu);

// 0x10
void rst_10(struct CPU *cpu);

// 0x18
void rst_18(struct CPU *cpu);

// 0x20
void rst_20(struct CPU *cpu);

// 0x28
void rst_28(struct CPU *cpu);

// 0x30
void rst_30(struct CPU *cpu);

// 0x38
void rst_38(struct CPU *cpu);

// Returns

// RET
// return from subroutine
// Pop two bytes (address) from stack to PC
// and jump to it
void ret(struct CPU *cpu);

// RET cc
// return from subroutine if condition cc is true
// Pop two bytes (address) from stack to PC
// and jump to it if condition cc is true
// cc: JumpCondition
void ret_cc(struct CPU *cpu, enum JumpCondition cc);

// RETI
// return from subroutine and enable interrupts
// Pop two bytes (address) from stack to PC
// and jump to it
// then enable interrupts
void reti(struct CPU *cpu);

#endif
