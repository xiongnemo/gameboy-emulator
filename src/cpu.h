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

enum JumpCondition
{
    JumpCondition_NZ = 0x00,
    JumpCondition_Z = 0x01,
    JumpCondition_NC = 0x02,
    JumpCondition_C = 0x03
};


struct InstructionParam
{
    enum Register reg_1;
    enum Register reg_2;
    enum RegisterPair rp_1;
    enum RegisterPair rp_2;
    enum JumpCondition cc;
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

// instruction methods

#define EXECUABLE_INSTRUCTION(fn_name) uint8_t fn_name(struct CPU *cpu, struct InstructionParam *param)

// abort when invalid opcode is executed
EXECUABLE_INSTRUCTION(cpu_invalid_opcode);

// LD

// 8-bit load instructions

// Load immediate value to register
EXECUABLE_INSTRUCTION(ld_imm_to_register);

// Load register value to immediate value
EXECUABLE_INSTRUCTION(ld_register_to_imm);

// Load register value to register
EXECUABLE_INSTRUCTION(ld_register_to_register);

// Load address value to register
EXECUABLE_INSTRUCTION(ld_address_register_pair_to_register);

// Load register value to address
EXECUABLE_INSTRUCTION(ld_register_to_address_register_pair);

// Load value from register A to zero page address with register C
EXECUABLE_INSTRUCTION(ld_register_a_to_zero_page_address_c);

// Load value from zero page address with register C to register A
EXECUABLE_INSTRUCTION(ld_zero_page_address_c_to_register_a);

// load value from address HL to register A, increment HL
EXECUABLE_INSTRUCTION(ld_address_hl_to_register_a_inc_hl);

// load value from register A to address HL, increment HL
EXECUABLE_INSTRUCTION(ld_register_a_to_address_hl_inc_hl);

// load value from address HL to register A, decrement HL
EXECUABLE_INSTRUCTION(ld_address_hl_to_register_a_dec_hl);

// load value from register A to address HL, decrement HL
EXECUABLE_INSTRUCTION(ld_register_a_to_address_hl_dec_hl);

// load value from register A to zero page address with immediate value
EXECUABLE_INSTRUCTION(ld_register_a_to_zero_page_address_imm);

// load value from zero page address with immediate value to register A
EXECUABLE_INSTRUCTION(ld_zero_page_address_imm_to_register_a);

// 16-bit load instructions

// load value from immediate value to register pair
EXECUABLE_INSTRUCTION(ld_imm_to_register_pair);

// load from HL to SP
EXECUABLE_INSTRUCTION(ld_hl_to_sp);

// load SP + one byte signed immediate value to HL
EXECUABLE_INSTRUCTION(ld_sp_imm_to_hl);

// Load SP to immediate value address
EXECUABLE_INSTRUCTION(ld_sp_to_address_imm);

// 16-bit stack instructions

// push register pair to stack
EXECUABLE_INSTRUCTION(push_register_pair);

// pop register pair from stack
EXECUABLE_INSTRUCTION(pop_register_pair);

// 8-bit ALU instructions

// ADD
// add n to register A
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Set if carry from bit 3 to bit 4
// C: Set if carry from bit 7 to bit 8
void add_a(struct CPU *cpu, uint8_t *value);

// add value from immediate value to register A
EXECUABLE_INSTRUCTION(add_a_imm);

// add value from register ? to register A
EXECUABLE_INSTRUCTION(add_a_register);

// add value from address HL to register A
EXECUABLE_INSTRUCTION(add_a_address_hl);

// ADC
// add value from register A and carry flag to register A
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Set if carry from bit 3 to bit 4
// C: Set if carry from bit 7 to bit 8
void adc_a(struct CPU *cpu, uint8_t *value);

// add value from immediate value to register A and carry flag
EXECUABLE_INSTRUCTION(adc_a_imm);

// add value from register ? to register A and carry flag
EXECUABLE_INSTRUCTION(adc_a_register);

// add value from address HL to register A and carry flag
EXECUABLE_INSTRUCTION(adc_a_address_hl);

// SUB
// subtract n from register A
// Flags:
// Z: Set if result is zero
// N: Set
// H: Set if no borrow from bit 4
// C: Set if no borrow
void sub_a(struct CPU *cpu, uint8_t *value);

// subtract value from immediate value from register A
EXECUABLE_INSTRUCTION(sub_a_imm);

// subtract value from register ? from register A
EXECUABLE_INSTRUCTION(sub_a_register);

// subtract value from address HL from register A
EXECUABLE_INSTRUCTION(sub_a_address_hl);

// SBC
// subtract value from register A and carry flag from register A
// Flags:
// Z: Set if result is zero
// N: Set
// H: Set if no borrow from bit 4
// C: Set if no borrow
void sbc_a(struct CPU *cpu, uint8_t *value);

// subtract value from immediate value from register A and carry flag
EXECUABLE_INSTRUCTION(sbc_a_imm);

// subtract value from register ? from register A and carry flag
EXECUABLE_INSTRUCTION(sbc_a_register);

// subtract value from address HL from register A and carry flag
EXECUABLE_INSTRUCTION(sbc_a_address_hl);

// AND
// logically AND n with register A, result in register A
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Set
// C: Reset
void and_a(struct CPU *cpu, uint8_t *value);

// logically AND immediate value with register A, result in register A
EXECUABLE_INSTRUCTION(and_a_imm);

// logically AND register ? with register A, result in register A
EXECUABLE_INSTRUCTION(and_a_register);

// logically AND address HL with register A, result in register A
EXECUABLE_INSTRUCTION(and_a_address_hl);

// OR
// logically OR n with register A, result in register A
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Reset
void or_a(struct CPU *cpu, uint8_t *value);

// logically OR immediate value with register A, result in register A
EXECUABLE_INSTRUCTION(or_a_imm);

// logically OR register ? with register A, result in register A
EXECUABLE_INSTRUCTION(or_a_register);

// logically OR address HL with register A, result in register A
EXECUABLE_INSTRUCTION(or_a_address_hl);

// XOR
// logically XOR n with register A, result in register A
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Reset
void xor_a(struct CPU *cpu);

// logically XOR immediate value with register A, result in register A
EXECUABLE_INSTRUCTION(xor_a_imm);

// logically XOR register ? with register A, result in register A
EXECUABLE_INSTRUCTION(xor_a_register);

// logically XOR address HL with register A, result in register A
EXECUABLE_INSTRUCTION(xor_a_address_hl);

// CP
// compare register A with n
// Flags:
// Z: Set if the result is zero (A == n)
// N: Set
// H: Set if no borrow from bit 4
// C: Set if A < n
void cp_a(struct CPU *cpu);

// compare immediate value with register A
EXECUABLE_INSTRUCTION(cp_a_imm);

// compare register ? with register A
EXECUABLE_INSTRUCTION(cp_a_register);

// compare address HL with register A
EXECUABLE_INSTRUCTION(cp_a_address_hl);

// INC
// increment
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Set if carry from bit 3 to bit 4
// C: Not affected
void inc(struct CPU *cpu, uint8_t *value);

// INC r
EXECUABLE_INSTRUCTION(inc_register);

// INC HL
EXECUABLE_INSTRUCTION(inc_address_hl);

// DEC
// decrement
// Flags:
// Z: Set if result is zero
// N: Set
// H: Set if no borrow from bit 4
// C: Not affected
void dec(struct CPU *cpu, uint8_t *value);

// DEC r
EXECUABLE_INSTRUCTION(dec_register);

// DEC HL
EXECUABLE_INSTRUCTION(dec_address_hl);

// 16-bit ALU instructions

// ADD HL, rr
// add register pair to HL
// Flags:
// Z: Not affected
// N: Reset
// H: Set if carry from bit 11 to bit 12
// C: Set if carry from bit 15 to bit 16
EXECUABLE_INSTRUCTION(add_hl_rr);

// ADD SP, n
// add one byte signed immediate value to SP
// Flags:
// Z: Reset
// N: Reset
// H: Set if carry from bit 11 to bit 12
// C: Set if carry from bit 15 to bit 16
EXECUABLE_INSTRUCTION(add_sp_imm);

// INC nn
// increment register
// Flags:
// None
void inc_16_bit(struct CPU *cpu, uint16_t *value);

// increment register pair
EXECUABLE_INSTRUCTION(inc_register_pair);

// increment SP
EXECUABLE_INSTRUCTION(inc_sp);

// DEC nn
// decrement register
// Flags:
// None
void dec_16_bit(struct CPU *cpu, uint16_t *value);

// decrement register pair
EXECUABLE_INSTRUCTION(dec_register_pair);

// decrement SP
EXECUABLE_INSTRUCTION(dec_sp);

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
EXECUABLE_INSTRUCTION(swap_register);

// swap lower and upper nibbles of a value at address HL
EXECUABLE_INSTRUCTION(swap_address_hl);

// DAA
// decimal adjust register A
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Set if carry from bit 7 to bit 8
EXECUABLE_INSTRUCTION(daa);

// CPL
// complement register A (Flip all bits)
// Flags:
// Z: Not affected
// N: Set
// H: Set
// C: Not affected
EXECUABLE_INSTRUCTION(cpl);

// CCF
// complement carry flag
// If C flag is set, it will be reset.
// If C flag is reset, it will be set.
// Flags:
// Z: Not affected
// N: Reset
// H: Reset
// C: Complemented
EXECUABLE_INSTRUCTION(ccf);

// SCF
// set carry flag
// Flags:
// Z: Not affected
// N: Reset
// H: Reset
// C: Set
EXECUABLE_INSTRUCTION(scf);

// NOP
// No operation
EXECUABLE_INSTRUCTION(nop);

// HALT
// Halt the CPU
EXECUABLE_INSTRUCTION(halt);

// STOP
// Halt CPU and LCD until button is pressed
EXECUABLE_INSTRUCTION(stop);

// DI
// Disable interrupts but not immediate
// Interrupt will be disabled after the current instruction (after DI is executed)
EXECUABLE_INSTRUCTION(di);

// EI
// Enable interrupts but not immediate
// Interrupt will be enabled after the current instruction (after EI is executed)
EXECUABLE_INSTRUCTION(ei);

// 8-bit Rotate and Shift instructions

// RLCA
// rotate register A left, old bit 7 to carry flag
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 7 data (before rotation)
EXECUABLE_INSTRUCTION(rlca);

// RLA
// rotate register A left through carry flag
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 7 data (before rotation)
EXECUABLE_INSTRUCTION(rla);

// RRCA
// rotate register A right, old bit 0 to carry flag
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 0 data (before rotation)
EXECUABLE_INSTRUCTION(rrca);

// RRA
// rotate register A right through carry flag
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 0 data (before rotation)
EXECUABLE_INSTRUCTION(rra);

// RLC
// rotate left, old bit 7 to carry flag
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 7 data (before rotation)
void rlc(struct CPU *cpu, uint8_t *value);

// RLC r
EXECUABLE_INSTRUCTION(rlc_register);

// RLC (HL)
EXECUABLE_INSTRUCTION(rlc_address_hl);

// RL
// rotate left through carry flag
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 7 data (before rotation)
void rl(struct CPU *cpu, uint8_t *value);

// RL r
EXECUABLE_INSTRUCTION(rl_register);

// RL (HL)
EXECUABLE_INSTRUCTION(rl_address_hl);

// RRC
// rotate right, old bit 0 to carry flag
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 0 data (before rotation)
void rrc(struct CPU *cpu, uint8_t *value);

// RRC r
EXECUABLE_INSTRUCTION(rrc_register);

// RRC (HL)
EXECUABLE_INSTRUCTION(rrc_address_hl);

// RR
// rotate right through carry flag
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 0 data (before rotation)
void rr(struct CPU *cpu, uint8_t *value);

// RR r
EXECUABLE_INSTRUCTION(rr_register);

// RR (HL)
EXECUABLE_INSTRUCTION(rr_address_hl);

// SLA
// shift left into carry flag, LSB of value set to 0
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 7 data (before shift)
void sla(struct CPU *cpu, uint8_t *value);

// SLA r
EXECUABLE_INSTRUCTION(sla_register);

// SLA (HL)
EXECUABLE_INSTRUCTION(sla_address_hl);

// SRA
// shift right into carry flag, MSB doesn't change
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 0 data (before shift)
void sra(struct CPU *cpu, uint8_t *value);

// SRA r
EXECUABLE_INSTRUCTION(sra_register);

// SRA (HL)
EXECUABLE_INSTRUCTION(sra_address_hl);

// SRL
// shift right into carry flag, MSB set to 0
// Flags:
// Z: Set if result is zero
// N: Reset
// H: Reset
// C: Contains old bit 0 data (before shift)
void srl(struct CPU *cpu, uint8_t *value);

// SRL r
EXECUABLE_INSTRUCTION(srl_register);

// SRL (HL)
EXECUABLE_INSTRUCTION(srl_address_hl);

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
EXECUABLE_INSTRUCTION(bit_register);

// BIT b, (HL)
EXECUABLE_INSTRUCTION(bit_address_hl);

// SET b, n
// set bit b of n
// Flags:
// None
void set(struct CPU *cpu, uint8_t bit, uint8_t *value);

// SET b, r
EXECUABLE_INSTRUCTION(set_register);

// SET b, (HL)
EXECUABLE_INSTRUCTION(set_address_hl);

// RES b, n
// reset bit b of n
// Flags:
// None
void res(struct CPU *cpu, uint8_t bit, uint8_t *value);

// RES b, r
EXECUABLE_INSTRUCTION(res_register);

// RES b, (HL)
EXECUABLE_INSTRUCTION(res_address_hl);

// 8-bit jump instructions

// JP nn
// jump to address nn
// nn: 16-bit immediate value (LSB first)
EXECUABLE_INSTRUCTION(jp_imm);

// JP cc, nn
// jump to address nn if condition cc is true
// cc: JumpCondition
// nn: 16-bit immediate value (LSB first)
EXECUABLE_INSTRUCTION(jp_cc_imm);

// JP (HL)
EXECUABLE_INSTRUCTION(jp_address_hl);

// JR n
// Add n to PC and jump to it
// n: one byte signed immediate value
EXECUABLE_INSTRUCTION(jr_imm);

// JR cc, n
// Add n to PC and jump to it if condition cc is true
// cc: JumpCondition
// n: one byte signed immediate value
EXECUABLE_INSTRUCTION(jr_cc_imm);

// Calls

// CALL nn
// call subroutine at address nn
// Push address of next instruction to stack
// then jump to address nn
// nn: 16-bit immediate value (LSB first)
EXECUABLE_INSTRUCTION(call_imm);

// CALL cc, nn
// call subroutine at address nn if condition cc is true
// Push address of next instruction to stack
// then jump to address nn
// cc: JumpCondition
// nn: 16-bit immediate value (LSB first)
EXECUABLE_INSTRUCTION(call_cc_imm);

// Restarts

// RST n
// jump to address n
// they are hard-coded in the instruction set
// 0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38
void rst(struct CPU *cpu, uint8_t n);

// 0x00
EXECUABLE_INSTRUCTION(rst_00);

// 0x08
EXECUABLE_INSTRUCTION(rst_08);

// 0x10
EXECUABLE_INSTRUCTION(rst_10);

// 0x18
EXECUABLE_INSTRUCTION(rst_18);

// 0x20
EXECUABLE_INSTRUCTION(rst_20);

// 0x28
EXECUABLE_INSTRUCTION(rst_28);

// 0x30
EXECUABLE_INSTRUCTION(rst_30);

// 0x38
EXECUABLE_INSTRUCTION(rst_38);

// Returns

// RET
// return from subroutine
// Pop two bytes (address) from stack to PC
// and jump to it
EXECUABLE_INSTRUCTION(ret);

// RET cc
// return from subroutine if condition cc is true
// Pop two bytes (address) from stack to PC
// and jump to it if condition cc is true
// cc: JumpCondition
EXECUABLE_INSTRUCTION(ret_cc);

// RETI
// return from subroutine and enable interrupts
// Pop two bytes (address) from stack to PC
// and jump to it
// then enable interrupts
EXECUABLE_INSTRUCTION(reti);

#endif
