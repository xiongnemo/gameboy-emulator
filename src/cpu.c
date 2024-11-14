#include "cpu.h"

struct CPU *create_cpu(struct Registers *registers, struct MMU *mmu)
{
    struct CPU *cpu = (struct CPU *)malloc(sizeof(struct CPU));
    if (cpu == NULL)
    {
        return NULL;
    }
    cpu->registers = registers;
    cpu->mmu = mmu;
    cpu->opcode_cycle_main = opcode_cycle_main;
    cpu->opcode_cycle_prefix_cb = opcode_cycle_prefix_cb;

    // initialize cpu state
    cpu->halted = false;
    cpu->stopped = false;
    cpu->ime = false;

    // set method pointers
    cpu->cpu_step_next = cpu_step_next;

    // // initialize instruction table
    // instruction_fn instruction_table[256] = {
    //     nop,
    //     ld_imm_to_register_pair,
    //     ld_register_to_address_register_pair,
    //     inc_register_pair,
    // };

    // // Initialize instruction param
    // struct InstructionParam instruction_param[256] = {
    //     {},
    //     {.rp_1 = BC},
    //     {.reg_1 = A, .rp_1 = BC},
    //     {.rp_1 = BC},
    // };

    struct PackedInstructionParam instruction_table[256] = {
        // 0x00: NOP
        {nop, {}},
        // 0x01: LD BC, d16
        {ld_imm_to_register_pair, {.rp_1 = BC}},
        // 0x02: LD (BC), A
        {ld_register_to_address_register_pair, {.reg_1 = A, .rp_1 = BC}},
        // 0x03: INC BC
        {inc_register_pair, {.rp_1 = BC}},
        // 0x04: INC B
        {inc_register, {.reg_1 = B}},
        // 0x05: DEC B
        {dec_register, {.reg_1 = B}},
        // 0x06: LD B, d8
        {ld_imm_to_register, {.reg_1 = B}},
        // 0x07: RLCA
        {rlca, {}},
        // 0x08: LD (a16), SP
        {ld_sp_to_address_imm, {}},
        // 0x09: ADD HL, BC
        {add_hl_to_register_pair, {.rp_1 = BC}},
        // 0x0A: LD A, (BC)
        {ld_address_register_pair_to_register, {.reg_1 = A, .rp_1 = BC}},
        // 0x0B: DEC BC
        {dec_register_pair, {.rp_1 = BC}},
        // 0x0C: INC C
        {inc_register, {.reg_1 = C}},
        // 0x0D: DEC C
        {dec_register, {.reg_1 = C}},
        // 0x0E: LD C, d8
        {ld_imm_to_register, {.reg_1 = C}},
        // 0x0F: RRCA
        {rrca, {}},
        // ---------------------------------------------------------------
        // 0x10: STOP
        {stop, {}},
        // 0x11: LD DE, d16
        {ld_imm_to_register_pair, {.rp_1 = DE}},
        // 0x12: LD (DE), A
        {ld_register_to_address_register_pair, {.reg_1 = A, .rp_1 = DE}},
        // 0x13: INC DE
        {inc_register_pair, {.rp_1 = DE}},
        // 0x14: INC D
        {inc_register, {.reg_1 = D}},
        // 0x15: DEC D
        {dec_register, {.reg_1 = D}},
        // 0x16: LD D, d8
        {ld_imm_to_register, {.reg_1 = D}},
        // 0x17: RLA
        {rla, {}},
        // 0x18: JR r8
        {jr_imm, {}},
        // 0x19: ADD HL, DE
        {add_hl_to_register_pair, {.rp_1 = DE}},
        // 0x1A: LD A, (DE)
        {ld_address_register_pair_to_register, {.reg_1 = A, .rp_1 = DE}},
        // 0x1B: DEC DE
        {dec_register_pair, {.rp_1 = DE}},
        // 0x1C: INC E
        {inc_register, {.reg_1 = E}},
        // 0x1D: DEC E
        {dec_register, {.reg_1 = E}},
        // 0x1E: LD E, d8
        {ld_imm_to_register, {.reg_1 = E}},
        // 0x1F: RRA
        {rra, {}},
        // ---------------------------------------------------------------
        // 0x20: JR NZ, r8
        {jr_cc_imm, {.cc = JumpCondition_NZ}},
        // 0x21: LD HL, d16
        {ld_imm_to_register_pair, {.rp_1 = HL}},
        // 0x22: LD (HL+), A
        {ld_a_to_address_hl_inc_hl, {}},
        // 0x23: INC HL
        {inc_register_pair, {.rp_1 = HL}},
        // 0x24: INC H
        {inc_register, {.reg_1 = H}},
        // 0x25: DEC H
        {dec_register, {.reg_1 = H}},
        // 0x26: LD H, d8
        {ld_imm_to_register, {.reg_1 = H}},
        // 0x27: DAA
        {daa, {}},
        // 0x28: JR Z, r8
        {jr_cc_imm, {.cc = JumpCondition_Z}},
        // 0x29: ADD HL, HL
        {add_hl_to_register_pair, {.rp_1 = HL}},
        // 0x2A: LD A, (HL+)
        {ld_a_to_address_hl_inc_hl, {}},
        // 0x2B: DEC HL
        {dec_register_pair, {.rp_1 = HL}},
        // 0x2C: INC L
        {inc_register, {.reg_1 = L}},
        // 0x2D: DEC L
        {dec_register, {.reg_1 = L}},
        // 0x2E: LD L, d8
        {ld_imm_to_register, {.reg_1 = L}},
        // 0x2F: CPL
        {cpl, {}},
        // ---------------------------------------------------------------
        // 0x30: JR NC, r8
        {jr_cc_imm, {.cc = JumpCondition_NC}},
        // 0x31: LD SP, d16
        {ld_imm_to_sp, {}},
        // 0x32: LD (HL-), A
        {ld_a_to_address_hl_dec_hl, {}},
        // 0x33: INC SP
        {inc_sp, {}},
        // 0x34: INC (HL)
        {inc_address_hl, {}},
        // 0x35: DEC (HL)
        {dec_address_hl, {}},
        // 0x36: LD (HL), d8
        {ld_imm_to_address_hl, {}},
        // 0x37: SCF
        {scf, {}},
        // 0x38: JR C, r8
        {jr_cc_imm, {.cc = JumpCondition_C}},
        // 0x39: ADD HL, SP
        {add_sp_to_hl, {}},
        // 0x3A: LD A, (HL-)
        {ld_a_to_address_hl_dec_hl, {}},
        // 0x3B: DEC SP
        {dec_sp, {}},
        // 0x3C: INC A
        {inc_register, {.reg_1 = A}},
        // 0x3D: DEC A
        {dec_register, {.reg_1 = A}},
        // 0x3E: LD A, d8
        {ld_imm_to_register, {.reg_1 = A}},
        // 0x3F: CCF
        {ccf, {}},
        // ---------------------------------------------------------------
        // 0x40: LD B, B
        {ld_register_to_register, {.reg_1 = B, .reg_2 = B}},
        // 0x41: LD B, C
        {ld_register_to_register, {.reg_1 = C, .reg_2 = B}},
        // 0x42: LD B, D
        {ld_register_to_register, {.reg_1 = D, .reg_2 = B}},
        // 0x43: LD B, E
        {ld_register_to_register, {.reg_1 = E, .reg_2 = B}},
        // 0x44: LD B, H
        {ld_register_to_register, {.reg_1 = H, .reg_2 = B}},
        // 0x45: LD B, L
        {ld_register_to_register, {.reg_1 = L, .reg_2 = B}},
        // 0x46: LD B, (HL)
        {ld_address_hl_to_register, {.reg_1 = B}},
        // 0x47: LD B, A
        {ld_register_to_register, {.reg_1 = A, .reg_2 = B}},
        // 0x48: LD C, B
        {ld_register_to_register, {.reg_1 = B, .reg_2 = C}},
        // 0x49: LD C, C
        {ld_register_to_register, {.reg_1 = C, .reg_2 = C}},
        // 0x4A: LD C, D
        {ld_register_to_register, {.reg_1 = D, .reg_2 = C}},
        // 0x4B: LD C, E
        {ld_register_to_register, {.reg_1 = E, .reg_2 = C}},
        // 0x4C: LD C, H
        {ld_register_to_register, {.reg_1 = H, .reg_2 = C}},
        // 0x4D: LD C, L
        {ld_register_to_register, {.reg_1 = L, .reg_2 = C}},
        // 0x4E: LD C, (HL)
        {ld_address_hl_to_register, {.reg_1 = C}},
        // 0x4F: LD C, A
        {ld_register_to_register, {.reg_1 = A, .reg_2 = C}},
        // ---------------------------------------------------------------
        // 0x50: LD D, B
        {ld_register_to_register, {.reg_1 = B, .reg_2 = D}},
        // 0x51: LD D, C
        {ld_register_to_register, {.reg_1 = C, .reg_2 = D}},
        // 0x52: LD D, D
        {ld_register_to_register, {.reg_1 = D, .reg_2 = D}},
        // 0x53: LD D, E
        {ld_register_to_register, {.reg_1 = E, .reg_2 = D}},
        // 0x54: LD D, H
        {ld_register_to_register, {.reg_1 = H, .reg_2 = D}},
        // 0x55: LD D, L
        {ld_register_to_register, {.reg_1 = L, .reg_2 = D}},
        // 0x56: LD D, (HL)
        {ld_address_hl_to_register, {.reg_1 = D}},
        // 0x57: LD D, A
        {ld_register_to_register, {.reg_1 = A, .reg_2 = D}},
        // 0x58: LD E, B
        {ld_register_to_register, {.reg_1 = B, .reg_2 = E}},
        // 0x59: LD E, C
        {ld_register_to_register, {.reg_1 = C, .reg_2 = E}},
        // 0x5A: LD E, D
        {ld_register_to_register, {.reg_1 = D, .reg_2 = E}},
        // 0x5B: LD E, E
        {ld_register_to_register, {.reg_1 = E, .reg_2 = E}},
        // 0x5C: LD E, H
        {ld_register_to_register, {.reg_1 = H, .reg_2 = E}},
        // 0x5D: LD E, L
        {ld_register_to_register, {.reg_1 = L, .reg_2 = E}},
        // 0x5E: LD E, (HL)
        {ld_address_hl_to_register, {.reg_1 = E}},
        // 0x5F: LD E, A
        {ld_register_to_register, {.reg_1 = A, .reg_2 = E}},
        // ---------------------------------------------------------------
        // 0x60: LD H, B
        {ld_register_to_register, {.reg_1 = B, .reg_2 = H}},
        // 0x61: LD H, C
        {ld_register_to_register, {.reg_1 = C, .reg_2 = H}},
        // 0x62: LD H, D
        {ld_register_to_register, {.reg_1 = D, .reg_2 = H}},
        // 0x63: LD H, E
        {ld_register_to_register, {.reg_1 = E, .reg_2 = H}},
        // 0x64: LD H, H
        {ld_register_to_register, {.reg_1 = H, .reg_2 = H}},
        // 0x65: LD H, L
        {ld_register_to_register, {.reg_1 = L, .reg_2 = H}},
        // 0x66: LD H, (HL)
        {ld_address_hl_to_register, {.reg_1 = H}},
        // 0x67: LD H, A
        {ld_register_to_register, {.reg_1 = A, .reg_2 = H}},
        // 0x68: LD L, B
        {ld_register_to_register, {.reg_1 = B, .reg_2 = L}},
        // 0x69: LD L, C
        {ld_register_to_register, {.reg_1 = C, .reg_2 = L}},
        // 0x6A: LD L, D
        {ld_register_to_register, {.reg_1 = D, .reg_2 = L}},
        // 0x6B: LD L, E
        {ld_register_to_register, {.reg_1 = E, .reg_2 = L}},
        // 0x6C: LD L, H
        {ld_register_to_register, {.reg_1 = H, .reg_2 = L}},
        // 0x6D: LD L, L
        {ld_register_to_register, {.reg_1 = L, .reg_2 = L}},
        // 0x6E: LD L, (HL)
        {ld_address_hl_to_register, {.reg_1 = L}},
        // 0x6F: LD L, A
        {ld_register_to_register, {.reg_1 = A, .reg_2 = L}},
        // ---------------------------------------------------------------
        // 0x70: LD (HL), B
        {ld_register_to_address_hl, {.reg_1 = B}},
        // 0x71: LD (HL), C
        {ld_register_to_address_hl, {.reg_1 = C}},
        // 0x72: LD (HL), D
        {ld_register_to_address_hl, {.reg_1 = D}},
        // 0x73: LD (HL), E
        {ld_register_to_address_hl, {.reg_1 = E}},
        // 0x74: LD (HL), H
        {ld_register_to_address_hl, {.reg_1 = H}},
        // 0x75: LD (HL), L
        {ld_register_to_address_hl, {.reg_1 = L}},
        // 0x76: HALT
        {halt, {}},
        // 0x77: LD (HL), A
        {ld_register_to_address_hl, {.reg_1 = A}},
        // 0x78: LD A, B
        {ld_register_to_register, {.reg_1 = B, .reg_2 = A}},
        // 0x79: LD A, C
        {ld_register_to_register, {.reg_1 = C, .reg_2 = A}},
        // 0x7A: LD A, D
        {ld_register_to_register, {.reg_1 = D, .reg_2 = A}},
        // 0x7B: LD A, E
        {ld_register_to_register, {.reg_1 = E, .reg_2 = A}},
        // 0x7C: LD A, H
        {ld_register_to_register, {.reg_1 = H, .reg_2 = A}},
        // 0x7D: LD A, L
        {ld_register_to_register, {.reg_1 = L, .reg_2 = A}},
        // 0x7E: LD A, (HL)
        {ld_address_hl_to_register, {.reg_1 = A}},
        // 0x7F: LD A, A
        {ld_register_to_register, {.reg_1 = A, .reg_2 = A}},
        // ---------------------------------------------------------------
        // 0x80: ADD A, B
        {add_register_to_a, {.reg_1 = B}},
        // 0x81: ADD A, C
        {add_register_to_a, {.reg_1 = C}},
        // 0x82: ADD A, D
        {add_register_to_a, {.reg_1 = D}},
        // 0x83: ADD A, E
        {add_register_to_a, {.reg_1 = E}},
        // 0x84: ADD A, H
        {add_register_to_a, {.reg_1 = H}},
        // 0x85: ADD A, L
        {add_register_to_a, {.reg_1 = L}},
        // 0x86: ADD A, (HL)
        {add_address_hl_to_a, {}},
        // 0x87: ADD A, A
        {add_register_to_a, {.reg_1 = A}},
        // 0x88: ADC A, B
        {adc_register_to_a, {.reg_1 = B}},
        // 0x89: ADC A, C
        {adc_register_to_a, {.reg_1 = C}},
        // 0x8A: ADC A, D
        {adc_register_to_a, {.reg_1 = D}},
        // 0x8B: ADC A, E
        {adc_register_to_a, {.reg_1 = E}},
        // 0x8C: ADC A, H
        {adc_register_to_a, {.reg_1 = H}},
        // 0x8D: ADC A, L
        {adc_register_to_a, {.reg_1 = L}},
        // 0x8E: ADC A, (HL)
        {adc_address_hl_to_a, {}},
        // 0x8F: ADC A, A
        {adc_register_to_a, {.reg_1 = A}},
        // ---------------------------------------------------------------
        // 0x90: SUB B
        {sub_register_to_a, {.reg_1 = B}},
        // 0x91: SUB C
        {sub_register_to_a, {.reg_1 = C}},
        // 0x92: SUB D
        {sub_register_to_a, {.reg_1 = D}},
        // 0x93: SUB E
        {sub_register_to_a, {.reg_1 = E}},
        // 0x94: SUB H
        {sub_register_to_a, {.reg_1 = H}},
        // 0x95: SUB L
        {sub_register_to_a, {.reg_1 = L}},
        // 0x96: SUB (HL)
        {sub_address_hl_to_a, {}},
        // 0x97: SUB A
        {sub_register_to_a, {.reg_1 = A}},
        // 0x98: SBC A, B
        {sbc_register_to_a, {.reg_1 = B}},
        // 0x99: SBC A, C
        {sbc_register_to_a, {.reg_1 = C}},
        // 0x9A: SBC A, D
        {sbc_register_to_a, {.reg_1 = D}},
        // 0x9B: SBC A, E
        {sbc_register_to_a, {.reg_1 = E}},
        // 0x9C: SBC A, H
        {sbc_register_to_a, {.reg_1 = H}},
        // 0x9D: SBC A, L
        {sbc_register_to_a, {.reg_1 = L}},
        // 0x9E: SBC A, (HL)
        {sbc_address_hl_to_a, {}},
        // 0x9F: SBC A, A
        {sbc_register_to_a, {.reg_1 = A}},
        // ---------------------------------------------------------------
        // 0xA0: AND B
        {and_register_to_a, {.reg_1 = B}},
        // 0xA1: AND C
        {and_register_to_a, {.reg_1 = C}},
        // 0xA2: AND D
        {and_register_to_a, {.reg_1 = D}},
        // 0xA3: AND E
        {and_register_to_a, {.reg_1 = E}},
        // 0xA4: AND H
        {and_register_to_a, {.reg_1 = H}},
        // 0xA5: AND L
        {and_register_to_a, {.reg_1 = L}},
        // 0xA6: AND (HL)
        {and_address_hl_to_a, {}},
        // 0xA7: AND A
        {and_register_to_a, {.reg_1 = A}},
        // 0xA8: XOR B
        {xor_register_to_a, {.reg_1 = B}},
        // 0xA9: XOR C
        {xor_register_to_a, {.reg_1 = C}},
        // 0xAA: XOR D
        {xor_register_to_a, {.reg_1 = D}},
        // 0xAB: XOR E
        {xor_register_to_a, {.reg_1 = E}},
        // 0xAC: XOR H
        {xor_register_to_a, {.reg_1 = H}},
        // 0xAD: XOR L
        {xor_register_to_a, {.reg_1 = L}},
        // 0xAE: XOR (HL)
        {xor_address_hl_to_a, {}},
        // 0xAF: XOR A
        {xor_register_to_a, {.reg_1 = A}},
        // ---------------------------------------------------------------
        // 0xB0: OR B
        {or_register_to_a, {.reg_1 = B}},
        // 0xB1: OR C
        {or_register_to_a, {.reg_1 = C}},
        // 0xB2: OR D
        {or_register_to_a, {.reg_1 = D}},
        // 0xB3: OR E
        {or_register_to_a, {.reg_1 = E}},
        // 0xB4: OR H
        {or_register_to_a, {.reg_1 = H}},
        // 0xB5: OR L
        {or_register_to_a, {.reg_1 = L}},
        // 0xB6: OR (HL)
        {or_address_hl_to_a, {}},
        // 0xB7: OR A
        {or_register_to_a, {.reg_1 = A}},
        // 0xB8: CP B
        {cp_register_to_a, {.reg_1 = B}},
        // 0xB9: CP C
        {cp_register_to_a, {.reg_1 = C}},
        // 0xBA: CP D
        {cp_register_to_a, {.reg_1 = D}},
        // 0xBB: CP E
        {cp_register_to_a, {.reg_1 = E}},
        // 0xBC: CP H
        {cp_register_to_a, {.reg_1 = H}},
        // 0xBD: CP L
        {cp_register_to_a, {.reg_1 = L}},
        // 0xBE: CP (HL)
        {cp_address_hl_to_a, {}},
        // 0xBF: CP A
        {cp_register_to_a, {.reg_1 = A}},
        // ---------------------------------------------------------------
        // 0xC0: RET NZ
        {ret_cc, {.cc = JumpCondition_NZ}},
        // 0xC1: POP BC
        {pop_register_pair, {.rp_1 = BC}},
        // 0xC2: JP NZ, a16
        {jp_cc_imm, {.cc = JumpCondition_NZ}},
        // 0xC3: JP a16
        {jp_imm, {}},
        // 0xC4: CALL NZ, a16
        {call_cc_imm, {.cc = JumpCondition_NZ}},
        // 0xC5: PUSH BC
        {push_register_pair, {.rp_1 = BC}},
        // 0xC6: ADD A, d8
        {add_imm_to_a, {}},
        // 0xC7: RST 00H
        {rst_00h, {}},
        // 0xC8: RET Z
        {ret_cc, {.cc = JumpCondition_Z}},
        // 0xC9: RET
        {ret, {}},
        // 0xCA: JP Z, a16
        {jp_cc_imm, {.cc = JumpCondition_Z}},
        // 0xCB: PREFIX CB
        {prefix_cb, {}},
        // 0xCC: CALL Z, a16
        {call_cc_imm, {.cc = JumpCondition_Z}},
        // 0xCD: CALL a16
        {call_imm, {}},
        // 0xCE: ADC A, d8
        {adc_imm_to_a, {}},
        // 0xCF: RST 08H
        {rst_08h, {}},
        // ---------------------------------------------------------------
        // 0xD0: RET NC
        {ret_cc, {.cc = JumpCondition_NC}},
        // 0xD1: POP DE
        {pop_register_pair, {.rp_1 = DE}},
        // 0xD2: JP NC, a16
        {jp_cc_imm, {.cc = JumpCondition_NC}},
        // 0xD3: NULL
        {cpu_invalid_opcode, {}},
        // 0xD4: CALL NC, a16
        {call_cc_imm, {.cc = JumpCondition_NC}},
        // 0xD5: PUSH DE
        {push_register_pair, {.rp_1 = DE}},
        // 0xD6: SUB A, d8
        {sub_imm_to_a, {}},
        // 0xD7: RST 10H
        {rst_10h, {}},
        // 0xD8: RET C
        {ret_cc, {.cc = JumpCondition_C}},
        // 0xD9: RETI
        {reti, {}},
        // 0xDA: JP C, a16
        {jp_cc_imm, {.cc = JumpCondition_C}},
        // 0xDB: NULL
        {cpu_invalid_opcode, {}},
        // 0xDC: CALL C, a16
        {call_cc_imm, {.cc = JumpCondition_C}},
        // 0xDD: NULL
        {cpu_invalid_opcode, {}},
        // 0xDE: SBC A, d8
        {sbc_imm_to_a, {}},
        // 0xDF: RST 18H
        {rst_18h, {}},
        // 0xE0: LDH (a8), A
        {ld_a_to_zero_page_address_imm, {}},
        // 0xE1: POP HL
        {pop_register_pair, {.rp_1 = HL}},
        // 0xE2: LD (C), A
        {ld_a_to_zero_page_address_c, {}},
        // 0xE3: NULL
        {cpu_invalid_opcode, {}},
        // 0xE4: NULL
        {cpu_invalid_opcode, {}},
        // 0xE5: PUSH HL
        {push_register_pair, {.rp_1 = HL}},
        // 0xE6: AND d8
        {and_imm_to_a, {}},
        // 0xE7: RST 20H
        {rst_20h, {}},
        // 0xE8: ADD SP, r8
        {add_imm_to_sp, {}},
        // 0xE9: JP (HL)
        {jp_address_hl, {}},
        // 0xEA: LD (a16), A
        {ld_a_to_address_imm, {}},
        // 0xEB: NULL
        {cpu_invalid_opcode, {}},
        // 0xEC: NULL
        {cpu_invalid_opcode, {}},
        // 0xED: NULL
        {cpu_invalid_opcode, {}},
        // 0xEE: XOR d8
        {xor_imm_to_a, {}},
        // 0xEF: RST 28H
        {rst_28h, {}},
        // ---------------------------------------------------------------
        // 0xF0: LDH A, (a8)
        {ld_a_to_zero_page_address_imm, {}},
        // 0xF1: POP AF
        {pop_register_pair, {.rp_1 = AF}},
        // 0xF2: LD A, (C)
        {ld_a_to_zero_page_address_c, {}},
        // 0xF3: DI
        {di, {}},
        // 0xF4: NULL
        {cpu_invalid_opcode, {}},
        // 0xF5: PUSH AF
        {push_register_pair, {.rp_1 = AF}},
        // 0xF6: OR d8
        {or_imm_to_a, {}},
        // 0xF7: RST 30H
        {rst_30h, {}},
        // 0xF8: LD HL, SP+r8
        {ld_sp_plus_imm_to_hl, {}},
        // 0xF9: LD SP, HL
        {ld_hl_to_sp, {}},
        // 0xFA: LD A, (a16)
        {ld_a_to_address_imm, {}},
        // 0xFB: EI
        {ei, {}},
        // 0xFC: NULL
        {cpu_invalid_opcode, {}},
        // 0xFD: NULL
        {cpu_invalid_opcode, {}},
        // 0xFE: CP d8
        {cp_imm_to_a, {}},
        // 0xFF: RST 38H
        {rst_38h, {}},
    };

    cpu->instruction_table = instruction_table;

    return cpu;
}

void free_cpu(struct CPU *cpu)
{
    if (cpu->registers)
    {
        free_registers(cpu->registers);
    }
    if (cpu->mmu)
    {
        free_mmu(cpu->mmu);
    }
    free(cpu);
}

// Private: Handle interrupts
uint8_t handle_interrupts(struct CPU *cpu)
{
    // TODO: Implement interrupt handling
    return 0;
}

uint8_t cpu_step_next(struct CPU *cpu)
{
    // 1. Check interrupts
    uint8_t interrupt_handled = handle_interrupts(cpu);
    if (interrupt_handled)
    {
        return interrupt_handled;
    }
    // 2. halted?
    if (cpu->halted)
    {
        return 1;
    }
    // 3. step next instruction
    return cpu_step(cpu);
}

uint8_t cpu_step_read_byte(struct CPU *cpu)
{
    // 1. Get byte from MMU
    uint8_t byte = cpu->mmu->mmu_get_byte(cpu->mmu, *(cpu->registers->pc));
    // 2. Increment PC
    *(cpu->registers->pc) += 1;
    return byte;
}

uint16_t cpu_step_read_word(struct CPU *cpu)
{
    // 1. Get word from MMU
    uint16_t word = cpu->mmu->mmu_get_word(cpu->mmu, *(cpu->registers->pc));
    // 2. Increment PC
    *(cpu->registers->pc) += 2;
    return word;
}

uint8_t cpu_step(struct CPU *cpu)
{
    // 1. Get Op Byte
    cpu->op_code = cpu_step_read_byte(cpu);
    // 2. Execute Op Code
    return cpu_step_execute_op_code(cpu, cpu->op_code);
}

uint8_t cpu_step_execute_op_code(struct CPU *cpu, uint8_t op_byte)
{
    if (op_byte == CB_PREFIX)
    {
        return cpu_step_execute_prefix_cb(cpu);
    }
    return cpu_step_execute_main(cpu, op_byte);
}

uint8_t cpu_step_execute_prefix_cb(struct CPU *cpu)
{
    // 1. Get Op Byte
    cpu->op_code = cpu_step_read_byte(cpu);
    // 2. Execute Op Code
    return cpu_step_execute_cb_op_code(cpu, cpu->op_code);
}

uint8_t cpu_step_execute_cb_op_code(struct CPU *cpu, uint8_t op_byte) {
//     CPU_DEBUG_PRINT("Executing CB Op Code: %02x\n", op_byte);
//     struct PackedInstructionParam *param = &cpu->instruction_table_cb[op_byte];
//     param->fn(cpu, &param->param);
    // return cpu->opcode_cycle_cb[op_byte];
}

EXECUTABLE_INSTRUCTION(cpu_invalid_opcode)
{
    CPU_EMERGENCY_PRINT("Invalid Opcode: %X\n", cpu->op_code);
    exit(EXIT_FAILURE);
}

EXECUTABLE_INSTRUCTION(nop)
{
    CPU_DEBUG_PRINT("NOP\n");
}

EXECUTABLE_INSTRUCTION(ld_imm_to_register_pair)
{
    enum RegisterPair register_pair = param->rp_1;
    uint16_t value = cpu_step_read_word(cpu);
    cpu->registers->set_register_pair(cpu->registers, register_pair, value);
}

EXECUTABLE_INSTRUCTION(ld_register_to_address_register_pair)
{
    enum Register from_register = param->reg_1;
    enum RegisterPair to_register_pair = param->rp_1;
    uint16_t mmu_address = cpu->registers->get_register_pair(cpu->registers, to_register_pair);
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, from_register);
    cpu->mmu->mmu_set_byte(cpu->mmu, mmu_address, value);
}

// below are cursor generated functions

// First, let's add the 8-bit load instructions:

EXECUTABLE_INSTRUCTION(ld_imm_to_register)
{
    enum Register register_to = param->reg_1;
    uint8_t value = cpu_step_read_byte(cpu);
    cpu->registers->set_register_byte(cpu->registers, register_to, value);
}

EXECUTABLE_INSTRUCTION(ld_register_to_register)
{
    enum Register from_register = param->reg_1;
    enum Register to_register = param->reg_2;
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, from_register);
    cpu->registers->set_register_byte(cpu->registers, to_register, value);
}

EXECUTABLE_INSTRUCTION(ld_address_hl_to_register)
{
    enum Register register_to = param->reg_1;
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t value = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    cpu->registers->set_register_byte(cpu->registers, register_to, value);
}

EXECUTABLE_INSTRUCTION(ld_register_to_address_hl)
{
    enum Register from_register = param->reg_1;
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, from_register);
    cpu->mmu->mmu_set_byte(cpu->mmu, address, value);
}

EXECUTABLE_INSTRUCTION(ld_imm_to_address_hl)
{
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t value = cpu_step_read_byte(cpu);
    cpu->mmu->mmu_set_byte(cpu->mmu, address, value);
}

// Now the 16-bit load instructions:

EXECUTABLE_INSTRUCTION(ld_sp_to_address_imm)
{
    uint16_t address = cpu_step_read_word(cpu);
    uint16_t sp = cpu->registers->get_control_register(cpu->registers, SP);
    cpu->mmu->mmu_set_word(cpu->mmu, address, sp);
}

EXECUTABLE_INSTRUCTION(ld_hl_to_sp)
{
    uint16_t hl = cpu->registers->get_register_pair(cpu->registers, HL);
    cpu->registers->set_control_register(cpu->registers, SP, hl);
}

EXECUTABLE_INSTRUCTION(push_register_pair)
{
    enum RegisterPair register_pair = param->rp_1;
    uint16_t value = cpu->registers->get_register_pair(cpu->registers, register_pair);
    uint16_t sp = cpu->registers->get_control_register(cpu->registers, SP);
    sp -= 2;
    cpu->mmu->mmu_set_word(cpu->mmu, sp, value);
    cpu->registers->set_control_register(cpu->registers, SP, sp);
}

EXECUTABLE_INSTRUCTION(pop_register_pair)
{
    enum RegisterPair register_pair = param->rp_1;
    uint16_t sp = cpu->registers->get_control_register(cpu->registers, SP);
    uint16_t value = cpu->mmu->mmu_get_word(cpu->mmu, sp);
    cpu->registers->set_register_pair(cpu->registers, register_pair, value);
    cpu->registers->set_control_register(cpu->registers, SP, sp + 2);
}

// 8-bit arithmetic instructions:

EXECUTABLE_INSTRUCTION(add_register_to_a)
{
    enum Register from_register = param->reg_1;
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, from_register);
    uint8_t a = cpu->registers->get_register_byte(cpu->registers, A);
    uint16_t result = a + value;

    // Set flags
    cpu->registers->set_flag_z(cpu->registers, (result & 0xFF) == 0);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, (a & 0xF) + (value & 0xF) > 0xF);
    cpu->registers->set_flag_c(cpu->registers, result > 0xFF);

    cpu->registers->set_register_byte(cpu->registers, A, result & 0xFF);
}

EXECUTABLE_INSTRUCTION(add_imm_to_a)
{
    uint8_t value = cpu_step_read_byte(cpu);
    uint8_t a = cpu->registers->get_register_byte(cpu->registers, A);
    uint16_t result = a + value;

    // Set flags
    cpu->registers->set_flag_z(cpu->registers, (result & 0xFF) == 0);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, (a & 0xF) + (value & 0xF) > 0xF);
    cpu->registers->set_flag_c(cpu->registers, result > 0xFF);

    cpu->registers->set_register_byte(cpu->registers, A, result & 0xFF);
}

// ADC implementation

void adc_a(struct CPU *cpu, uint8_t *value)
{
    uint8_t a = cpu->registers->get_register_byte(cpu->registers, A);
    uint8_t carry = cpu->registers->get_flag_c(cpu->registers);
    uint16_t result = a + *value + carry;

    cpu->registers->set_flag_z(cpu->registers, (result & 0xFF) == 0);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, (a & 0xF) + (*value & 0xF) + carry > 0xF);
    cpu->registers->set_flag_c(cpu->registers, result > 0xFF);

    cpu->registers->set_register_byte(cpu->registers, A, result & 0xFF);
}

EXECUTABLE_INSTRUCTION(adc_imm_to_a)
{
    uint8_t value = cpu_step_read_byte(cpu);
    adc_a(cpu, &value);
}

// INC/DEC implementations
void inc(struct CPU *cpu, uint8_t *value)
{
    uint8_t result = *value + 1;

    cpu->registers->set_flag_z(cpu->registers, result == 0);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, (*value & 0xF) == 0xF);

    *value = result;
}

EXECUTABLE_INSTRUCTION(inc_register)
{
    enum Register reg = param->reg_1;
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, reg);
    inc(cpu, &value);
    cpu->registers->set_register_byte(cpu->registers, reg, value);
}

EXECUTABLE_INSTRUCTION(inc_address_hl)
{
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t value = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    inc(cpu, &value);
    cpu->mmu->mmu_set_byte(cpu->mmu, address, value);
}

void dec(struct CPU *cpu, uint8_t *value)
{
    uint8_t result = *value - 1;

    cpu->registers->set_flag_z(cpu->registers, result == 0);
    cpu->registers->set_flag_n(cpu->registers, true);
    cpu->registers->set_flag_h(cpu->registers, (*value & 0xF) == 0);

    *value = result;
}

EXECUTABLE_INSTRUCTION(dec_register)
{
    enum Register reg = param->reg_1;
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, reg);
    dec(cpu, &value);
    cpu->registers->set_register_byte(cpu->registers, reg, value);
}

EXECUTABLE_INSTRUCTION(dec_address_hl)
{
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t value = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    dec(cpu, &value);
    cpu->mmu->mmu_set_byte(cpu->mmu, address, value);
}

void add_hl(struct CPU *cpu, uint16_t *value)
{
    uint16_t hl = cpu->registers->get_register_pair(cpu->registers, HL);
    uint32_t result = hl + *value;

    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, (hl & 0xFFF) + (*value & 0xFFF) > 0xFFF);
    cpu->registers->set_flag_c(cpu->registers, result > 0xFFFF);

    cpu->registers->set_register_pair(cpu->registers, HL, result & 0xFFFF);
}

EXECUTABLE_INSTRUCTION(add_hl_to_register_pair)
{
    enum RegisterPair rp = param->rp_1;
    uint16_t value = cpu->registers->get_register_pair(cpu->registers, rp);
    add_hl(cpu, &value);
}

EXECUTABLE_INSTRUCTION(add_sp_to_hl)
{
    uint16_t sp = cpu->registers->get_control_register(cpu->registers, SP);
    add_hl(cpu, &sp);
}

EXECUTABLE_INSTRUCTION(add_imm_to_sp)
{
    int8_t value = (int8_t)cpu_step_read_byte(cpu);
    uint16_t sp = cpu->registers->get_control_register(cpu->registers, SP);
    uint16_t result = sp + value;

    cpu->registers->set_flag_z(cpu->registers, false);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, (sp & 0xF) + (value & 0xF) > 0xF);
    cpu->registers->set_flag_c(cpu->registers, (sp & 0xFF) + (value & 0xFF) > 0xFF);

    cpu->registers->set_control_register(cpu->registers, SP, result);
}
// Bit manipulation instructions
void swap(struct CPU *cpu, uint8_t *value)
{
    uint8_t result = (*value >> 4) | (*value << 4);

    cpu->registers->set_flag_z(cpu->registers, result == 0);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, false);
    cpu->registers->set_flag_c(cpu->registers, false);

    *value = result;
}

EXECUTABLE_INSTRUCTION(swap_register)
{
    enum Register reg = param->reg_1;
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, reg);
    swap(cpu, &value);
    cpu->registers->set_register_byte(cpu->registers, reg, value);
}

EXECUTABLE_INSTRUCTION(swap_address_hl)
{
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t value = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    swap(cpu, &value);
    cpu->mmu->mmu_set_byte(cpu->mmu, address, value);
}

EXECUTABLE_INSTRUCTION(daa)
{
    uint8_t a = cpu->registers->get_register_byte(cpu->registers, A);
    uint8_t adjust = 0;

    if (cpu->registers->get_flag_h(cpu->registers) ||
        (!cpu->registers->get_flag_n(cpu->registers) && (a & 0xF) > 9))
    {
        adjust |= 0x06;
    }

    if (cpu->registers->get_flag_c(cpu->registers) ||
        (!cpu->registers->get_flag_n(cpu->registers) && a > 0x99))
    {
        adjust |= 0x60;
        cpu->registers->set_flag_c(cpu->registers, true);
    }
    a += cpu->registers->get_flag_n(cpu->registers) ? -adjust : adjust;
    cpu->registers->set_flag_z(cpu->registers, a == 0);
    cpu->registers->set_flag_h(cpu->registers, false);
    cpu->registers->set_register_byte(cpu->registers, A, a);
}

EXECUTABLE_INSTRUCTION(cpl)
{
    uint8_t a = cpu->registers->get_register_byte(cpu->registers, A);
    a = ~a;
    cpu->registers->set_flag_n(cpu->registers, true);
    cpu->registers->set_flag_h(cpu->registers, true);
    cpu->registers->set_register_byte(cpu->registers, A, a);
}

// Rotate and shift instructions
EXECUTABLE_INSTRUCTION(rlca) {
    uint8_t a = cpu->registers->get_register_byte(cpu->registers, A);
    uint8_t carry = (a & 0x80) >> 7;
    a = (a << 1) | carry;
    
    cpu->registers->set_flag_z(cpu->registers, false);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, false);
    cpu->registers->set_flag_c(cpu->registers, carry);
    
    cpu->registers->set_register_byte(cpu->registers, A, a);
}

EXECUTABLE_INSTRUCTION(rla) {
    uint8_t a = cpu->registers->get_register_byte(cpu->registers, A);
    uint8_t old_carry = cpu->registers->get_flag_c(cpu->registers);
    uint8_t new_carry = (a & 0x80) >> 7;
    a = (a << 1) | old_carry;
    
    cpu->registers->set_flag_z(cpu->registers, false);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, false);
    cpu->registers->set_flag_c(cpu->registers, new_carry);
    
    cpu->registers->set_register_byte(cpu->registers, A, a);
}

EXECUTABLE_INSTRUCTION(rrca) {
    uint8_t a = cpu->registers->get_register_byte(cpu->registers, A);
    uint8_t carry = a & 0x01;
    a = (a >> 1) | (carry << 7);
    
    cpu->registers->set_flag_z(cpu->registers, false);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, false);
    cpu->registers->set_flag_c(cpu->registers, carry);
    
    cpu->registers->set_register_byte(cpu->registers, A, a);
}

EXECUTABLE_INSTRUCTION(rra) {
    uint8_t a = cpu->registers->get_register_byte(cpu->registers, A);
    uint8_t old_carry = cpu->registers->get_flag_c(cpu->registers);
    uint8_t new_carry = a & 0x01;
    a = (a >> 1) | (old_carry << 7);
    
    cpu->registers->set_flag_z(cpu->registers, false);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, false);
    cpu->registers->set_flag_c(cpu->registers, new_carry);
    
    cpu->registers->set_register_byte(cpu->registers, A, a);
}

// Jump instructions
EXECUTABLE_INSTRUCTION(jp_imm) {
    uint16_t address = cpu_step_read_word(cpu);
    cpu->registers->set_control_register(cpu->registers, PC, address);
}

EXECUTABLE_INSTRUCTION(jp_hl) {
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    cpu->registers->set_control_register(cpu->registers, PC, address);
}

EXECUTABLE_INSTRUCTION(jp_cc_imm) {
    uint16_t address = cpu_step_read_word(cpu);
    enum JumpCondition condition = param->cc;
    bool jump = false;
    
    switch(condition) {
        case JumpCondition_NZ:
            jump = !cpu->registers->get_flag_z(cpu->registers);
            break;
        case JumpCondition_Z:
            jump = cpu->registers->get_flag_z(cpu->registers);
            break;
        case JumpCondition_NC:
            jump = !cpu->registers->get_flag_c(cpu->registers);
            break;
        case JumpCondition_C:
            jump = cpu->registers->get_flag_c(cpu->registers);
            break;
    }
    
    if (jump) {
        cpu->registers->set_control_register(cpu->registers, PC, address);
    }
}

EXECUTABLE_INSTRUCTION(jr_imm) {
    int8_t offset = (int8_t)cpu_step_read_byte(cpu);
    uint16_t pc = cpu->registers->get_control_register(cpu->registers, PC);
    cpu->registers->set_control_register(cpu->registers, PC, pc + offset);
}

EXECUTABLE_INSTRUCTION(jr_cc_imm) {
    int8_t offset = (int8_t)cpu_step_read_byte(cpu);
    enum JumpCondition condition = param->cc;
    bool jump = false;
    
    switch(condition) {
        case JumpCondition_NZ:
            jump = !cpu->registers->get_flag_z(cpu->registers);
            break;
        case JumpCondition_Z:
            jump = cpu->registers->get_flag_z(cpu->registers);
            break;
        case JumpCondition_NC:
            jump = !cpu->registers->get_flag_c(cpu->registers);
            break;
        case JumpCondition_C:
            jump = cpu->registers->get_flag_c(cpu->registers);
            break;
    }
    
    if (jump) {
        uint16_t pc = cpu->registers->get_control_register(cpu->registers, PC);
        cpu->registers->set_control_register(cpu->registers, PC, pc + offset);
    }
}

EXECUTABLE_INSTRUCTION(call_imm) {
    uint16_t address = cpu_step_read_word(cpu);
    uint16_t pc = cpu->registers->get_control_register(cpu->registers, PC);
    uint16_t sp = cpu->registers->get_control_register(cpu->registers, SP);
    
    sp -= 2;
    cpu->mmu->mmu_set_word(cpu->mmu, sp, pc);
    cpu->registers->set_control_register(cpu->registers, SP, sp);
    cpu->registers->set_control_register(cpu->registers, PC, address);
}

EXECUTABLE_INSTRUCTION(call_cc_imm) {
    uint16_t address = cpu_step_read_word(cpu);
    enum JumpCondition condition = param->cc;
    bool jump = false;
    
    switch(condition) {
        case JumpCondition_NZ:
            jump = !cpu->registers->get_flag_z(cpu->registers);
            break;
        case JumpCondition_Z:
            jump = cpu->registers->get_flag_z(cpu->registers);
            break;
        case JumpCondition_NC:
            jump = !cpu->registers->get_flag_c(cpu->registers);
            break;
        case JumpCondition_C:
            jump = cpu->registers->get_flag_c(cpu->registers);
            break;
    }
    
    if (jump) {
        uint16_t pc = cpu->registers->get_control_register(cpu->registers, PC);
        uint16_t sp = cpu->registers->get_control_register(cpu->registers, SP);
        sp -= 2;
        cpu->mmu->mmu_set_word(cpu->mmu, sp, pc);
        cpu->registers->set_control_register(cpu->registers, SP, sp);
        cpu->registers->set_control_register(cpu->registers, PC, address);
    }
}

EXECUTABLE_INSTRUCTION(ret) {
    uint16_t sp = cpu->registers->get_control_register(cpu->registers, SP);
    uint16_t address = cpu->mmu->mmu_get_word(cpu->mmu, sp);
    cpu->registers->set_control_register(cpu->registers, SP, sp + 2);
    cpu->registers->set_control_register(cpu->registers, PC, address);
}

EXECUTABLE_INSTRUCTION(ret_cc) {
    enum JumpCondition condition = param->cc;
    bool jump = false;
    
    switch(condition) {
        case JumpCondition_NZ:
            jump = !cpu->registers->get_flag_z(cpu->registers);
            break;
        case JumpCondition_Z:
            jump = cpu->registers->get_flag_z(cpu->registers);
            break;
        case JumpCondition_NC:
            jump = !cpu->registers->get_flag_c(cpu->registers);
            break;
        case JumpCondition_C:
            jump = cpu->registers->get_flag_c(cpu->registers);
            break;
    }
    
    if (jump) {
        uint16_t sp = cpu->registers->get_control_register(cpu->registers, SP);
        uint16_t address = cpu->mmu->mmu_get_word(cpu->mmu, sp);
        cpu->registers->set_control_register(cpu->registers, SP, sp + 2);
        cpu->registers->set_control_register(cpu->registers, PC, address);
    }
}

EXECUTABLE_INSTRUCTION(reti) {
    uint16_t sp = cpu->registers->get_control_register(cpu->registers, SP);
    uint16_t address = cpu->mmu->mmu_get_word(cpu->mmu, sp);
    cpu->registers->set_control_register(cpu->registers, SP, sp + 2);
    cpu->registers->set_control_register(cpu->registers, PC, address);
    // Enable interrupts
    cpu->ime = true;
}

void rst(struct CPU *cpu, uint8_t n) {
    uint16_t pc = cpu->registers->get_control_register(cpu->registers, PC);
    uint16_t sp = cpu->registers->get_control_register(cpu->registers, SP);
    
    sp -= 2;
    cpu->mmu->mmu_set_word(cpu->mmu, sp, pc);
    cpu->registers->set_control_register(cpu->registers, SP, sp);
    cpu->registers->set_control_register(cpu->registers, PC, n);
}

EXECUTABLE_INSTRUCTION(rst_00h) {
    rst(cpu, 0x00);
}

EXECUTABLE_INSTRUCTION(rst_08h) {
    rst(cpu, 0x08);
}

EXECUTABLE_INSTRUCTION(rst_10h) {
    rst(cpu, 0x10);
}

EXECUTABLE_INSTRUCTION(rst_18h) {
    rst(cpu, 0x18);
}

EXECUTABLE_INSTRUCTION(rst_20h) {
    rst(cpu, 0x20);
}

EXECUTABLE_INSTRUCTION(rst_28h) {
    rst(cpu, 0x28);
}

EXECUTABLE_INSTRUCTION(rst_30h) {
    rst(cpu, 0x30);
}

EXECUTABLE_INSTRUCTION(rst_38h) {
    rst(cpu, 0x38);
}

// Helper functions for 16-bit arithmetic
void inc_16_bit(struct CPU *cpu, uint16_t *value) {
    (*value)++;
}

void dec_16_bit(struct CPU *cpu, uint16_t *value) {
    (*value)--;
}

EXECUTABLE_INSTRUCTION(inc_register_pair) {
    enum RegisterPair rp = param->rp_1;
    uint16_t value = cpu->registers->get_register_pair(cpu->registers, rp);
    inc_16_bit(cpu, &value);
    cpu->registers->set_register_pair(cpu->registers, rp, value);
}

EXECUTABLE_INSTRUCTION(dec_register_pair) {
    enum RegisterPair rp = param->rp_1;
    uint16_t value = cpu->registers->get_register_pair(cpu->registers, rp);
    dec_16_bit(cpu, &value);
    cpu->registers->set_register_pair(cpu->registers, rp, value);
}

EXECUTABLE_INSTRUCTION(inc_sp) {
    uint16_t sp = cpu->registers->get_control_register(cpu->registers, SP);
    inc_16_bit(cpu, &sp);
    cpu->registers->set_control_register(cpu->registers, SP, sp);
}

EXECUTABLE_INSTRUCTION(dec_sp) {
    uint16_t sp = cpu->registers->get_control_register(cpu->registers, SP);
    dec_16_bit(cpu, &sp);
    cpu->registers->set_control_register(cpu->registers, SP, sp);
}

EXECUTABLE_INSTRUCTION(ccf) {
    bool current_carry = cpu->registers->get_flag_c(cpu->registers);
    
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, false);
    cpu->registers->set_flag_c(cpu->registers, !current_carry);
}

EXECUTABLE_INSTRUCTION(scf) {
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, false);
    cpu->registers->set_flag_c(cpu->registers, true);
}

EXECUTABLE_INSTRUCTION(halt) {
    cpu->halted = true;
}

EXECUTABLE_INSTRUCTION(stop) {
    cpu->stopped = true;
}

EXECUTABLE_INSTRUCTION(di) {
    cpu->ime = false;
}

EXECUTABLE_INSTRUCTION(ei) {
    cpu->ime = true;
}

EXECUTABLE_INSTRUCTION(ld_sp_plus_imm_to_hl) {
    int8_t offset = (int8_t)cpu_step_read_byte(cpu);
    uint16_t sp = cpu->registers->get_control_register(cpu->registers, SP);
    uint16_t result = sp + offset;
    
    cpu->registers->set_flag_z(cpu->registers, false);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, (sp & 0xF) + (offset & 0xF) > 0xF);
    cpu->registers->set_flag_c(cpu->registers, (sp & 0xFF) + (offset & 0xFF) > 0xFF);
    
    cpu->registers->set_register_pair(cpu->registers, HL, result);
}

EXECUTABLE_INSTRUCTION(ld_a_to_address_imm) {
    uint16_t address = cpu_step_read_word(cpu);
    uint8_t a = cpu->registers->get_register_byte(cpu->registers, A);
    cpu->mmu->mmu_set_byte(cpu->mmu, address, a);
}

EXECUTABLE_INSTRUCTION(ld_a_to_zero_page_address_c) {
    uint8_t offset = cpu->registers->get_register_byte(cpu->registers, C);
    uint8_t a = cpu->registers->get_register_byte(cpu->registers, A);
    cpu->mmu->mmu_set_byte(cpu->mmu, 0xFF00 + offset, a);
}

EXECUTABLE_INSTRUCTION(ld_a_to_zero_page_address_imm) {
    uint8_t offset = cpu_step_read_byte(cpu);
    uint8_t a = cpu->registers->get_register_byte(cpu->registers, A);
    cpu->mmu->mmu_set_byte(cpu->mmu, 0xFF00 + offset, a);
}

EXECUTABLE_INSTRUCTION(ld_imm_to_sp) {
    uint16_t value = cpu_step_read_word(cpu);
    cpu->registers->set_control_register(cpu->registers, SP, value);
}

EXECUTABLE_INSTRUCTION(ld_a_to_address_hl_inc_hl) {
    uint16_t hl = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t a = cpu->registers->get_register_byte(cpu->registers, A);
    cpu->mmu->mmu_set_byte(cpu->mmu, hl, a);
    cpu->registers->set_register_pair(cpu->registers, HL, hl + 1);
}

EXECUTABLE_INSTRUCTION(ld_a_to_address_hl_dec_hl) {
    uint16_t hl = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t a = cpu->registers->get_register_byte(cpu->registers, A);
    cpu->mmu->mmu_set_byte(cpu->mmu, hl, a);
    cpu->registers->set_register_pair(cpu->registers, HL, hl - 1);
}

EXECUTABLE_INSTRUCTION(ld_address_register_pair_to_register) {
    enum Register reg = param->reg_1;
    enum RegisterPair rp = param->rp_1;
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, rp);
    uint8_t value = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    cpu->registers->set_register_byte(cpu->registers, reg, value);
}

EXECUTABLE_INSTRUCTION(add_address_hl_to_a) {
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t value = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    add_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(adc_address_hl_to_a) {
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t value = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    adc_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(sub_address_hl_to_a) {
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t value = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    sub_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(sbc_address_hl_to_a) {
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t value = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    sbc_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(and_address_hl_to_a) {
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t value = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    and_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(xor_address_hl_to_a) {
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t value = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    xor_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(or_address_hl_to_a) {
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t value = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    or_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(cp_address_hl_to_a) {
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t value = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    cp_a(cpu, &value);
}


EXECUTABLE_INSTRUCTION(adc_register_to_a) {
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, param->reg_1);
    adc_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(sub_register_to_a) {
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, param->reg_1);
    sub_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(sbc_register_to_a) {
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, param->reg_1);
    sbc_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(and_register_to_a) {
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, param->reg_1);
    and_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(xor_register_to_a) {
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, param->reg_1);
    xor_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(or_register_to_a) {
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, param->reg_1);
    or_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(cp_register_to_a) {
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, param->reg_1);
    cp_a(cpu, &value);
}

// Immediate operations
EXECUTABLE_INSTRUCTION(cp_imm_to_a) {
    uint8_t value = cpu_step_read_byte(cpu);
    cp_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(or_imm_to_a) {
    uint8_t value = cpu_step_read_byte(cpu);
    or_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(xor_imm_to_a) {
    uint8_t value = cpu_step_read_byte(cpu);
    xor_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(and_imm_to_a) {
    uint8_t value = cpu_step_read_byte(cpu);
    and_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(sbc_imm_to_a) {
    uint8_t value = cpu_step_read_byte(cpu);
    sbc_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(sub_imm_to_a) {
    uint8_t value = cpu_step_read_byte(cpu);
    sub_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(jp_address_hl) {
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    cpu->registers->set_control_register(cpu->registers, PC, address);
}

EXECUTABLE_INSTRUCTION(prefix_cb) {
    uint8_t op_byte = cpu_step_read_byte(cpu);
    cpu_step_execute_cb_op_code(cpu, op_byte);
}

void add_a(struct CPU *cpu, uint8_t *value) {
    uint8_t a = cpu->registers->get_register_byte(cpu->registers, A);
    uint8_t result = a + *value;
    
    cpu->registers->set_flag_z(cpu->registers, result == 0);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, (a & 0xF) + (*value & 0xF) > 0xF);
    cpu->registers->set_flag_c(cpu->registers, a + *value > 0xFF);
    
    cpu->registers->set_register_byte(cpu->registers, A, result);
}

void sub_a(struct CPU *cpu, uint8_t *value) {
    uint8_t a = cpu->registers->get_register_byte(cpu->registers, A);
    uint8_t result = a - *value;
    
    cpu->registers->set_flag_z(cpu->registers, result == 0);
    cpu->registers->set_flag_n(cpu->registers, true);
    cpu->registers->set_flag_h(cpu->registers, (a & 0xF) < (*value & 0xF));
    cpu->registers->set_flag_c(cpu->registers, a < *value);
    
    cpu->registers->set_register_byte(cpu->registers, A, result);
}

void sbc_a(struct CPU *cpu, uint8_t *value) {
    uint8_t a = cpu->registers->get_register_byte(cpu->registers, A);
    uint8_t carry = cpu->registers->get_flag_c(cpu->registers);
    uint8_t result = a - *value - carry;
    
    cpu->registers->set_flag_z(cpu->registers, result == 0);
    cpu->registers->set_flag_n(cpu->registers, true);
    cpu->registers->set_flag_h(cpu->registers, (a & 0xF) < ((*value & 0xF) + carry));
    cpu->registers->set_flag_c(cpu->registers, a < (*value + carry));
    
    cpu->registers->set_register_byte(cpu->registers, A, result);
}

void and_a(struct CPU *cpu, uint8_t *value) {
    uint8_t a = cpu->registers->get_register_byte(cpu->registers, A);
    uint8_t result = a & *value;
    
    cpu->registers->set_flag_z(cpu->registers, result == 0);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, true);
    cpu->registers->set_flag_c(cpu->registers, false);
    
    cpu->registers->set_register_byte(cpu->registers, A, result);
}

void xor_a(struct CPU *cpu, uint8_t *value) {
    uint8_t a = cpu->registers->get_register_byte(cpu->registers, A);
    uint8_t result = a ^ *value;
    
    cpu->registers->set_flag_z(cpu->registers, result == 0);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, false);
    cpu->registers->set_flag_c(cpu->registers, false);
    
    cpu->registers->set_register_byte(cpu->registers, A, result);
}

void or_a(struct CPU *cpu, uint8_t *value) {
    uint8_t a = cpu->registers->get_register_byte(cpu->registers, A);
    uint8_t result = a | *value;
    
    cpu->registers->set_flag_z(cpu->registers, result == 0);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, false);
    cpu->registers->set_flag_c(cpu->registers, false);
    
    cpu->registers->set_register_byte(cpu->registers, A, result);
}

void cp_a(struct CPU *cpu, uint8_t *value) {
    uint8_t a = cpu->registers->get_register_byte(cpu->registers, A);
    uint8_t result = a - *value;
    
    cpu->registers->set_flag_z(cpu->registers, result == 0);
    cpu->registers->set_flag_n(cpu->registers, true);
    cpu->registers->set_flag_h(cpu->registers, (a & 0xF) < (*value & 0xF));
    cpu->registers->set_flag_c(cpu->registers, a < *value);
}



// end of cursor generated functions

uint8_t cpu_step_execute_main(struct CPU *cpu, uint8_t op_byte)
{
    CPU_DEBUG_PRINT("Executing Op Code: %02x\n", op_byte);
    struct PackedInstructionParam *param = &cpu->instruction_table[op_byte];
    param->fn(cpu, &param->param);
    return cpu->opcode_cycle_main[op_byte];
}