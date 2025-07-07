#include "cpu.h"

// Instruction table
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
    {add_register_pair_to_hl, {.rp_1 = BC}},
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
    {add_register_pair_to_hl, {.rp_1 = DE}},
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
    {jr_cc_imm, {.cc = JumpCondition_NZ}, .cycles_alternative = 3},
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
    {jr_cc_imm, {.cc = JumpCondition_Z}, .cycles_alternative = 3},
    // 0x29: ADD HL, HL
    {add_register_pair_to_hl, {.rp_1 = HL}},
    // 0x2A: LD A, (HL+)
    {ld_address_hl_to_a_inc_hl, {}},
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
    {jr_cc_imm, {.cc = JumpCondition_NC}, .cycles_alternative = 3},
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
    {jr_cc_imm, {.cc = JumpCondition_C}, .cycles_alternative = 3},
    // 0x39: ADD HL, SP
    {add_sp_to_hl, {}},
    // 0x3A: LD A, (HL-)
    {ld_address_hl_to_a_dec_hl, {}},
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
    {ret_cc, {.cc = JumpCondition_NZ}, .cycles_alternative = 5},
    // 0xC1: POP BC
    {pop_register_pair, {.rp_1 = BC}},
    // 0xC2: JP NZ, a16
    {jp_cc_imm, {.cc = JumpCondition_NZ}, .cycles_alternative = 4},
    // 0xC3: JP a16
    {jp_imm, {}},
    // 0xC4: CALL NZ, a16
    {call_cc_imm, {.cc = JumpCondition_NZ}, .cycles_alternative = 6},
    // 0xC5: PUSH BC
    {push_register_pair, {.rp_1 = BC}},
    // 0xC6: ADD A, d8
    {add_imm_to_a, {}},
    // 0xC7: RST 00H
    {rst_00h, {}},
    // 0xC8: RET Z
    {ret_cc, {.cc = JumpCondition_Z}, .cycles_alternative = 5},
    // 0xC9: RET
    {ret, {}},
    // 0xCA: JP Z, a16
    {jp_cc_imm, {.cc = JumpCondition_Z}, .cycles_alternative = 4},
    // 0xCB: PREFIX CB
    {prefix_cb, {}},
    // 0xCC: CALL Z, a16
    {call_cc_imm, {.cc = JumpCondition_Z}, .cycles_alternative = 6},
    // 0xCD: CALL a16
    {call_imm, {}},
    // 0xCE: ADC A, d8
    {adc_imm_to_a, {}},
    // 0xCF: RST 08H
    {rst_08h, {}},
    // ---------------------------------------------------------------
    // 0xD0: RET NC
    {ret_cc, {.cc = JumpCondition_NC}, .cycles_alternative = 5},
    // 0xD1: POP DE
    {pop_register_pair, {.rp_1 = DE}},
    // 0xD2: JP NC, a16
    {jp_cc_imm, {.cc = JumpCondition_NC}, .cycles_alternative = 4},
    // 0xD3: NULL
    {cpu_invalid_opcode, {}},
    // 0xD4: CALL NC, a16
    {call_cc_imm, {.cc = JumpCondition_NC}, .cycles_alternative = 6},
    // 0xD5: PUSH DE
    {push_register_pair, {.rp_1 = DE}},
    // 0xD6: SUB A, d8
    {sub_imm_to_a, {}},
    // 0xD7: RST 10H
    {rst_10h, {}},
    // 0xD8: RET C
    {ret_cc, {.cc = JumpCondition_C}, .cycles_alternative = 5},
    // 0xD9: RETI
    {reti, {}},
    // 0xDA: JP C, a16
    {jp_cc_imm, {.cc = JumpCondition_C}, .cycles_alternative = 4},
    // 0xDB: NULL
    {cpu_invalid_opcode, {}},
    // 0xDC: CALL C, a16
    {call_cc_imm, {.cc = JumpCondition_C}, .cycles_alternative = 6},
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
    {ld_zero_page_address_imm_to_a, {}},
    // 0xF1: POP AF
    {pop_register_pair, {.rp_1 = AF}},
    // 0xF2: LD A, (C)
    {ld_zero_page_address_c_to_a, {}},
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
    {ld_address_imm_to_a, {}},
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

// CB prefix instruction table
struct PackedInstructionParam instruction_table_cb[256] = {
    // 0x00: RLC B
    {rlc_register,    {.reg_1 = B}                   },
    // 0x01: RLC C
    {rlc_register,    {.reg_1 = C}                   },
    // 0x02: RLC D
    {rlc_register,    {.reg_1 = D}                   },
    // 0x03: RLC E
    {rlc_register,    {.reg_1 = E}                   },
    // 0x04: RLC H
    {rlc_register,    {.reg_1 = H}                   },
    // 0x05: RLC L
    {rlc_register,    {.reg_1 = L}                   },
    // 0x06: RLC (HL)
    {rlc_address_hl,  {}                             },
    // 0x07: RLC A
    {rlc_register,    {.reg_1 = A}                   },
    // ---------------------------------------------------------------
    // 0x08: RRC B
    {rrc_register,    {.reg_1 = B}                   },
    // 0x09: RRC C
    {rrc_register,    {.reg_1 = C}                   },
    // 0x0A: RRC D
    {rrc_register,    {.reg_1 = D}                   },
    // 0x0B: RRC E
    {rrc_register,    {.reg_1 = E}                   },
    // 0x0C: RRC H
    {rrc_register,    {.reg_1 = H}                   },
    // 0x0D: RRC L
    {rrc_register,    {.reg_1 = L}                   },
    // 0x0E: RRC (HL)
    {rrc_address_hl,  {}                             },
    // 0x0F: RRC A
    {rrc_register,    {.reg_1 = A}                   },
    // ---------------------------------------------------------------
    // 0x10: RL B
    {rl_register,     {.reg_1 = B}                   },
    // 0x11: RL C
    {rl_register,     {.reg_1 = C}                   },
    // 0x12: RL D
    {rl_register,     {.reg_1 = D}                   },
    // 0x13: RL E
    {rl_register,     {.reg_1 = E}                   },
    // 0x14: RL H
    {rl_register,     {.reg_1 = H}                   },
    // 0x15: RL L
    {rl_register,     {.reg_1 = L}                   },
    // 0x16: RL (HL)
    {rl_address_hl,   {}                             },
    // 0x17: RL A
    {rl_register,     {.reg_1 = A}                   },
    // ---------------------------------------------------------------
    // 0x18: RR B
    {rr_register,     {.reg_1 = B}                   },
    // 0x19: RR C
    {rr_register,     {.reg_1 = C}                   },
    // 0x1A: RR D
    {rr_register,     {.reg_1 = D}                   },
    // 0x1B: RR E
    {rr_register,     {.reg_1 = E}                   },
    // 0x1C: RR H
    {rr_register,     {.reg_1 = H}                   },
    // 0x1D: RR L
    {rr_register,     {.reg_1 = L}                   },
    // 0x1E: RR (HL)
    {rr_address_hl,   {}                             },
    // 0x1F: RR A
    {rr_register,     {.reg_1 = A}                   },
    // ---------------------------------------------------------------
    // 0x20: SLA B
    {sla_register,    {.reg_1 = B}                   },
    // 0x21: SLA C
    {sla_register,    {.reg_1 = C}                   },
    // 0x22: SLA D
    {sla_register,    {.reg_1 = D}                   },
    // 0x23: SLA E
    {sla_register,    {.reg_1 = E}                   },
    // 0x24: SLA H
    {sla_register,    {.reg_1 = H}                   },
    // 0x25: SLA L
    {sla_register,    {.reg_1 = L}                   },
    // 0x26: SLA (HL)
    {sla_address_hl,  {}                             },
    // 0x27: SLA A
    {sla_register,    {.reg_1 = A}                   },
    // ---------------------------------------------------------------
    // 0x28: SRA B
    {sra_register,    {.reg_1 = B}                   },
    // 0x29: SRA C
    {sra_register,    {.reg_1 = C}                   },
    // 0x2A: SRA D
    {sra_register,    {.reg_1 = D}                   },
    // 0x2B: SRA E
    {sra_register,    {.reg_1 = E}                   },
    // 0x2C: SRA H
    {sra_register,    {.reg_1 = H}                   },
    // 0x2D: SRA L
    {sra_register,    {.reg_1 = L}                   },
    // 0x2E: SRA (HL)
    {sra_address_hl,  {}                             },
    // 0x2F: SRA A
    {sra_register,    {.reg_1 = A}                   },
    // ---------------------------------------------------------------
    // 0x30: SWAP B
    {swap_register,   {.reg_1 = B}                   },
    // 0x31: SWAP C
    {swap_register,   {.reg_1 = C}                   },
    // 0x32: SWAP D
    {swap_register,   {.reg_1 = D}                   },
    // 0x33: SWAP E
    {swap_register,   {.reg_1 = E}                   },
    // 0x34: SWAP H
    {swap_register,   {.reg_1 = H}                   },
    // 0x35: SWAP L
    {swap_register,   {.reg_1 = L}                   },
    // 0x36: SWAP (HL)
    {swap_address_hl, {}                             },
    // 0x37: SWAP A
    {swap_register,   {.reg_1 = A}                   },
    // ---------------------------------------------------------------
    // 0x38: SRL B
    {srl_register,    {.reg_1 = B}                   },
    // 0x39: SRL C
    {srl_register,    {.reg_1 = C}                   },
    // 0x3A: SRL D
    {srl_register,    {.reg_1 = D}                   },
    // 0x3B: SRL E
    {srl_register,    {.reg_1 = E}                   },
    // 0x3C: SRL H
    {srl_register,    {.reg_1 = H}                   },
    // 0x3D: SRL L
    {srl_register,    {.reg_1 = L}                   },
    // 0x3E: SRL (HL)
    {srl_address_hl,  {}                             },
    // 0x3F: SRL A
    {srl_register,    {.reg_1 = A}                   },
    // ---------------------------------------------------------------
    // 0x40: BIT 0, B
    {bit_register,    {.reg_1 = B, .bit_position = 0}},
    // 0x41: BIT 0, C
    {bit_register,    {.reg_1 = C, .bit_position = 0}},
    // 0x42: BIT 0, D
    {bit_register,    {.reg_1 = D, .bit_position = 0}},
    // 0x43: BIT 0, E
    {bit_register,    {.reg_1 = E, .bit_position = 0}},
    // 0x44: BIT 0, H
    {bit_register,    {.reg_1 = H, .bit_position = 0}},
    // 0x45: BIT 0, L
    {bit_register,    {.reg_1 = L, .bit_position = 0}},
    // 0x46: BIT 0, (HL)
    {bit_address_hl,  {.bit_position = 0}            },
    // 0x47: BIT 0, A
    {bit_register,    {.reg_1 = A, .bit_position = 0}},
    // ---------------------------------------------------------------
    // 0x48: BIT 1, B
    {bit_register,    {.reg_1 = B, .bit_position = 1}},
    // 0x49: BIT 1, C
    {bit_register,    {.reg_1 = C, .bit_position = 1}},
    // 0x4A: BIT 1, D
    {bit_register,    {.reg_1 = D, .bit_position = 1}},
    // 0x4B: BIT 1, E
    {bit_register,    {.reg_1 = E, .bit_position = 1}},
    // 0x4C: BIT 1, H
    {bit_register,    {.reg_1 = H, .bit_position = 1}},
    // 0x4D: BIT 1, L
    {bit_register,    {.reg_1 = L, .bit_position = 1}},
    // 0x4E: BIT 1, (HL)
    {bit_address_hl,  {.bit_position = 1}            },
    // 0x4F: BIT 1, A
    {bit_register,    {.reg_1 = A, .bit_position = 1}},
    // ---------------------------------------------------------------
    // 0x50: BIT 2, B
    {bit_register,    {.reg_1 = B, .bit_position = 2}},
    // 0x51: BIT 2, C
    {bit_register,    {.reg_1 = C, .bit_position = 2}},
    // 0x52: BIT 2, D
    {bit_register,    {.reg_1 = D, .bit_position = 2}},
    // 0x53: BIT 2, E
    {bit_register,    {.reg_1 = E, .bit_position = 2}},
    // 0x54: BIT 2, H
    {bit_register,    {.reg_1 = H, .bit_position = 2}},
    // 0x55: BIT 2, L
    {bit_register,    {.reg_1 = L, .bit_position = 2}},
    // 0x56: BIT 2, (HL)
    {bit_address_hl,  {.bit_position = 2}            },
    // 0x57: BIT 2, A
    {bit_register,    {.reg_1 = A, .bit_position = 2}},
    // ---------------------------------------------------------------
    // 0x58: BIT 3, B
    {bit_register,    {.reg_1 = B, .bit_position = 3}},
    // 0x59: BIT 3, C
    {bit_register,    {.reg_1 = C, .bit_position = 3}},
    // 0x5A: BIT 3, D
    {bit_register,    {.reg_1 = D, .bit_position = 3}},
    // 0x5B: BIT 3, E
    {bit_register,    {.reg_1 = E, .bit_position = 3}},
    // 0x5C: BIT 3, H
    {bit_register,    {.reg_1 = H, .bit_position = 3}},
    // 0x5D: BIT 3, L
    {bit_register,    {.reg_1 = L, .bit_position = 3}},
    // 0x5E: BIT 3, (HL)
    {bit_address_hl,  {.bit_position = 3}            },
    // 0x5F: BIT 3, A
    {bit_register,    {.reg_1 = A, .bit_position = 3}},
    // ---------------------------------------------------------------
    // 0x60: BIT 4, B
    {bit_register,    {.reg_1 = B, .bit_position = 4}},
    // 0x61: BIT 4, C
    {bit_register,    {.reg_1 = C, .bit_position = 4}},
    // 0x62: BIT 4, D
    {bit_register,    {.reg_1 = D, .bit_position = 4}},
    // 0x63: BIT 4, E
    {bit_register,    {.reg_1 = E, .bit_position = 4}},
    // 0x64: BIT 4, H
    {bit_register,    {.reg_1 = H, .bit_position = 4}},
    // 0x65: BIT 4, L
    {bit_register,    {.reg_1 = L, .bit_position = 4}},
    // 0x66: BIT 4, (HL)
    {bit_address_hl,  {.bit_position = 4}            },
    // 0x67: BIT 4, A
    {bit_register,    {.reg_1 = A, .bit_position = 4}},
    // ---------------------------------------------------------------
    // 0x68: BIT 5, B
    {bit_register,    {.reg_1 = B, .bit_position = 5}},
    // 0x69: BIT 5, C
    {bit_register,    {.reg_1 = C, .bit_position = 5}},
    // 0x6A: BIT 5, D
    {bit_register,    {.reg_1 = D, .bit_position = 5}},
    // 0x6B: BIT 5, E
    {bit_register,    {.reg_1 = E, .bit_position = 5}},
    // 0x6C: BIT 5, H
    {bit_register,    {.reg_1 = H, .bit_position = 5}},
    // 0x6D: BIT 5, L
    {bit_register,    {.reg_1 = L, .bit_position = 5}},
    // 0x6E: BIT 5, (HL)
    {bit_address_hl,  {.bit_position = 5}            },
    // 0x6F: BIT 5, A
    {bit_register,    {.reg_1 = A, .bit_position = 5}},
    // ---------------------------------------------------------------
    // 0x70: BIT 6, B
    {bit_register,    {.reg_1 = B, .bit_position = 6}},
    // 0x71: BIT 6, C
    {bit_register,    {.reg_1 = C, .bit_position = 6}},
    // 0x72: BIT 6, D
    {bit_register,    {.reg_1 = D, .bit_position = 6}},
    // 0x73: BIT 6, E
    {bit_register,    {.reg_1 = E, .bit_position = 6}},
    // 0x74: BIT 6, H
    {bit_register,    {.reg_1 = H, .bit_position = 6}},
    // 0x75: BIT 6, L
    {bit_register,    {.reg_1 = L, .bit_position = 6}},
    // 0x76: BIT 6, (HL)
    {bit_address_hl,  {.bit_position = 6}            },
    // 0x77: BIT 6, A
    {bit_register,    {.reg_1 = A, .bit_position = 6}},
    // ---------------------------------------------------------------
    // 0x78: BIT 7, B
    {bit_register,    {.reg_1 = B, .bit_position = 7}},
    // 0x79: BIT 7, C
    {bit_register,    {.reg_1 = C, .bit_position = 7}},
    // 0x7A: BIT 7, D
    {bit_register,    {.reg_1 = D, .bit_position = 7}},
    // 0x7B: BIT 7, E
    {bit_register,    {.reg_1 = E, .bit_position = 7}},
    // 0x7C: BIT 7, H
    {bit_register,    {.reg_1 = H, .bit_position = 7}},
    // 0x7D: BIT 7, L
    {bit_register,    {.reg_1 = L, .bit_position = 7}},
    // 0x7E: BIT 7, (HL)
    {bit_address_hl,  {.bit_position = 7}            },
    // 0x7F: BIT 7, A
    {bit_register,    {.reg_1 = A, .bit_position = 7}},
    // ---------------------------------------------------------------
    // 0x80: RES 0, B
    {res_register,    {.reg_1 = B, .bit_position = 0}},
    // 0x81: RES 0, C
    {res_register,    {.reg_1 = C, .bit_position = 0}},
    // 0x82: RES 0, D
    {res_register,    {.reg_1 = D, .bit_position = 0}},
    // 0x83: RES 0, E
    {res_register,    {.reg_1 = E, .bit_position = 0}},
    // 0x84: RES 0, H
    {res_register,    {.reg_1 = H, .bit_position = 0}},
    // 0x85: RES 0, L
    {res_register,    {.reg_1 = L, .bit_position = 0}},
    // 0x86: RES 0, (HL)
    {res_address_hl,  {.bit_position = 0}            },
    // 0x87: RES 0, A
    {res_register,    {.reg_1 = A, .bit_position = 0}},
    // ---------------------------------------------------------------
    // 0x88: RES 1, B
    {res_register,    {.reg_1 = B, .bit_position = 1}},
    // 0x89: RES 1, C
    {res_register,    {.reg_1 = C, .bit_position = 1}},
    // 0x8A: RES 1, D
    {res_register,    {.reg_1 = D, .bit_position = 1}},
    // 0x8B: RES 1, E
    {res_register,    {.reg_1 = E, .bit_position = 1}},
    // 0x8C: RES 1, H
    {res_register,    {.reg_1 = H, .bit_position = 1}},
    // 0x8D: RES 1, L
    {res_register,    {.reg_1 = L, .bit_position = 1}},
    // 0x8E: RES 1, (HL)
    {res_address_hl,  {.bit_position = 1}            },
    // 0x8F: RES 1, A
    {res_register,    {.reg_1 = A, .bit_position = 1}},
    // ---------------------------------------------------------------
    // 0x90: RES 2, B
    {res_register,    {.reg_1 = B, .bit_position = 2}},
    // 0x91: RES 2, C
    {res_register,    {.reg_1 = C, .bit_position = 2}},
    // 0x92: RES 2, D
    {res_register,    {.reg_1 = D, .bit_position = 2}},
    // 0x93: RES 2, E
    {res_register,    {.reg_1 = E, .bit_position = 2}},
    // 0x94: RES 2, H
    {res_register,    {.reg_1 = H, .bit_position = 2}},
    // 0x95: RES 2, L
    {res_register,    {.reg_1 = L, .bit_position = 2}},
    // 0x96: RES 2, (HL)
    {res_address_hl,  {.bit_position = 2}            },
    // 0x97: RES 2, A
    {res_register,    {.reg_1 = A, .bit_position = 2}},
    // ---------------------------------------------------------------
    // 0x98: RES 3, B
    {res_register,    {.reg_1 = B, .bit_position = 3}},
    // 0x99: RES 3, C
    {res_register,    {.reg_1 = C, .bit_position = 3}},
    // 0x9A: RES 3, D
    {res_register,    {.reg_1 = D, .bit_position = 3}},
    // 0x9B: RES 3, E
    {res_register,    {.reg_1 = E, .bit_position = 3}},
    // 0x9C: RES 3, H
    {res_register,    {.reg_1 = H, .bit_position = 3}},
    // 0x9D: RES 3, L
    {res_register,    {.reg_1 = L, .bit_position = 3}},
    // 0x9E: RES 3, (HL)
    {res_address_hl,  {.bit_position = 3}            },
    // 0x9F: RES 3, A
    {res_register,    {.reg_1 = A, .bit_position = 3}},
    // ---------------------------------------------------------------
    // RES 4 (0xA0-0xA7)
    {res_register,    {.reg_1 = B, .bit_position = 4}},
    {res_register,    {.reg_1 = C, .bit_position = 4}},
    {res_register,    {.reg_1 = D, .bit_position = 4}},
    {res_register,    {.reg_1 = E, .bit_position = 4}},
    {res_register,    {.reg_1 = H, .bit_position = 4}},
    {res_register,    {.reg_1 = L, .bit_position = 4}},
    {res_address_hl,  {.bit_position = 4}            },
    {res_register,    {.reg_1 = A, .bit_position = 4}},

    // RES 5 (0xA8-0xAF)
    {res_register,    {.reg_1 = B, .bit_position = 5}},
    {res_register,    {.reg_1 = C, .bit_position = 5}},
    {res_register,    {.reg_1 = D, .bit_position = 5}},
    {res_register,    {.reg_1 = E, .bit_position = 5}},
    {res_register,    {.reg_1 = H, .bit_position = 5}},
    {res_register,    {.reg_1 = L, .bit_position = 5}},
    {res_address_hl,  {.bit_position = 5}            },
    {res_register,    {.reg_1 = A, .bit_position = 5}},

    // RES 6 (0xB0-0xB7)
    {res_register,    {.reg_1 = B, .bit_position = 6}},
    {res_register,    {.reg_1 = C, .bit_position = 6}},
    {res_register,    {.reg_1 = D, .bit_position = 6}},
    {res_register,    {.reg_1 = E, .bit_position = 6}},
    {res_register,    {.reg_1 = H, .bit_position = 6}},
    {res_register,    {.reg_1 = L, .bit_position = 6}},
    {res_address_hl,  {.bit_position = 6}            },
    {res_register,    {.reg_1 = A, .bit_position = 6}},

    // RES 7 (0xB8-0xBF)
    {res_register,    {.reg_1 = B, .bit_position = 7}},
    {res_register,    {.reg_1 = C, .bit_position = 7}},
    {res_register,    {.reg_1 = D, .bit_position = 7}},
    {res_register,    {.reg_1 = E, .bit_position = 7}},
    {res_register,    {.reg_1 = H, .bit_position = 7}},
    {res_register,    {.reg_1 = L, .bit_position = 7}},
    {res_address_hl,  {.bit_position = 7}            },
    {res_register,    {.reg_1 = A, .bit_position = 7}},
    // ---------------------------------------------------------------
    // 0xC0: SET 0, B
    {set_register,    {.reg_1 = B, .bit_position = 0}},
    // 0xC1: SET 0, C
    {set_register,    {.reg_1 = C, .bit_position = 0}},
    // 0xC2: SET 0, D
    {set_register,    {.reg_1 = D, .bit_position = 0}},
    // 0xC3: SET 0, E
    {set_register,    {.reg_1 = E, .bit_position = 0}},
    // 0xC4: SET 0, H
    {set_register,    {.reg_1 = H, .bit_position = 0}},
    // 0xC5: SET 0, L
    {set_register,    {.reg_1 = L, .bit_position = 0}},
    // 0xC6: SET 0, (HL)
    {set_address_hl,  {.bit_position = 0}            },
    // 0xC7: SET 0, A
    {set_register,    {.reg_1 = A, .bit_position = 0}},
    // ---------------------------------------------------------------
    // 0xC8: SET 1, B
    {set_register,    {.reg_1 = B, .bit_position = 1}},
    // 0xC9: SET 1, C
    {set_register,    {.reg_1 = C, .bit_position = 1}},
    // 0xCA: SET 1, D
    {set_register,    {.reg_1 = D, .bit_position = 1}},
    // 0xCB: SET 1, E
    {set_register,    {.reg_1 = E, .bit_position = 1}},
    // 0xCC: SET 1, H
    {set_register,    {.reg_1 = H, .bit_position = 1}},
    // 0xCD: SET 1, L
    {set_register,    {.reg_1 = L, .bit_position = 1}},
    // 0xCE: SET 1, (HL)
    {set_address_hl,  {.bit_position = 1}            },
    // 0xCF: SET 1, A
    {set_register,    {.reg_1 = A, .bit_position = 1}},
    // ---------------------------------------------------------------
    // 0xD0: SET 2, B
    {set_register,    {.reg_1 = B, .bit_position = 2}},
    // 0xD1: SET 2, C
    {set_register,    {.reg_1 = C, .bit_position = 2}},
    // 0xD2: SET 2, D
    {set_register,    {.reg_1 = D, .bit_position = 2}},
    // 0xD3: SET 2, E
    {set_register,    {.reg_1 = E, .bit_position = 2}},
    // 0xD4: SET 2, H
    {set_register,    {.reg_1 = H, .bit_position = 2}},
    // 0xD5: SET 2, L
    {set_register,    {.reg_1 = L, .bit_position = 2}},
    // 0xD6: SET 2, (HL)
    {set_address_hl,  {.bit_position = 2}            },
    // 0xD7: SET 2, A
    {set_register,    {.reg_1 = A, .bit_position = 2}},
    // ---------------------------------------------------------------
    // 0xD8: SET 3, B
    {set_register,    {.reg_1 = B, .bit_position = 3}},
    // 0xD9: SET 3, C
    {set_register,    {.reg_1 = C, .bit_position = 3}},
    // 0xDA: SET 3, D
    {set_register,    {.reg_1 = D, .bit_position = 3}},
    // 0xDB: SET 3, E
    {set_register,    {.reg_1 = E, .bit_position = 3}},
    // 0xDC: SET 3, H
    {set_register,    {.reg_1 = H, .bit_position = 3}},
    // 0xDD: SET 3, L
    {set_register,    {.reg_1 = L, .bit_position = 3}},
    // 0xDE: SET 3, (HL)
    {set_address_hl,  {.bit_position = 3}            },
    // 0xDF: SET 3, A
    {set_register,    {.reg_1 = A, .bit_position = 3}},
    // ---------------------------------------------------------------
    // 0xE0: SET 4, B
    {set_register,    {.reg_1 = B, .bit_position = 4}},
    // 0xE1: SET 4, C
    {set_register,    {.reg_1 = C, .bit_position = 4}},
    // 0xE2: SET 4, D
    {set_register,    {.reg_1 = D, .bit_position = 4}},
    // 0xE3: SET 4, E
    {set_register,    {.reg_1 = E, .bit_position = 4}},
    // 0xE4: SET 4, H
    {set_register,    {.reg_1 = H, .bit_position = 4}},
    // 0xE5: SET 4, L
    {set_register,    {.reg_1 = L, .bit_position = 4}},
    // 0xE6: SET 4, (HL)
    {set_address_hl,  {.bit_position = 4}            },
    // 0xE7: SET 4, A
    {set_register,    {.reg_1 = A, .bit_position = 4}},
    // ---------------------------------------------------------------
    // 0xE8: SET 5, B
    {set_register,    {.reg_1 = B, .bit_position = 5}},
    // 0xE9: SET 5, C
    {set_register,    {.reg_1 = C, .bit_position = 5}},
    // 0xEA: SET 5, D
    {set_register,    {.reg_1 = D, .bit_position = 5}},
    // 0xEB: SET 5, E
    {set_register,    {.reg_1 = E, .bit_position = 5}},
    // 0xEC: SET 5, H
    {set_register,    {.reg_1 = H, .bit_position = 5}},
    // 0xED: SET 5, L
    {set_register,    {.reg_1 = L, .bit_position = 5}},
    // 0xEE: SET 5, (HL)
    {set_address_hl,  {.bit_position = 5}            },
    // 0xEF: SET 5, A
    {set_register,    {.reg_1 = A, .bit_position = 5}},
    // ---------------------------------------------------------------
    // 0xF0: SET 6, B
    {set_register,    {.reg_1 = B, .bit_position = 6}},
    // 0xF1: SET 6, C
    {set_register,    {.reg_1 = C, .bit_position = 6}},
    // 0xF2: SET 6, D
    {set_register,    {.reg_1 = D, .bit_position = 6}},
    // 0xF3: SET 6, E
    {set_register,    {.reg_1 = E, .bit_position = 6}},
    // 0xF4: SET 6, H
    {set_register,    {.reg_1 = H, .bit_position = 6}},
    // 0xF5: SET 6, L
    {set_register,    {.reg_1 = L, .bit_position = 6}},
    // 0xF6: SET 6, (HL)
    {set_address_hl,  {.bit_position = 6}            },
    // 0xF7: SET 6, A
    {set_register,    {.reg_1 = A, .bit_position = 6}},
    // ---------------------------------------------------------------
    // 0xF8: SET 7, B
    {set_register,    {.reg_1 = B, .bit_position = 7}},
    // 0xF9: SET 7, C
    {set_register,    {.reg_1 = C, .bit_position = 7}},
    // 0xFA: SET 7, D
    {set_register,    {.reg_1 = D, .bit_position = 7}},
    // 0xFB: SET 7, E
    {set_register,    {.reg_1 = E, .bit_position = 7}},
    // 0xFC: SET 7, H
    {set_register,    {.reg_1 = H, .bit_position = 7}},
    // 0xFD: SET 7, L
    {set_register,    {.reg_1 = L, .bit_position = 7}},
    // 0xFE: SET 7, (HL)
    {set_address_hl,  {.bit_position = 7}            },
    // 0xFF: SET 7, A
    {set_register,    {.reg_1 = A, .bit_position = 7}},
};

struct CPU* create_cpu(struct Registers* registers, struct MMU* mmu)
{
    struct CPU* cpu = (struct CPU*)malloc(sizeof(struct CPU));
    if (cpu == NULL) {
        return NULL;
    }
    cpu->registers              = registers;
    cpu->mmu                    = mmu;
    cpu->opcode_cycle_main      = opcode_cycle_main;
    cpu->opcode_cycle_prefix_cb = opcode_cycle_prefix_cb;

    // initialize cpu state
    cpu->halted                  = false;
    cpu->stopped                 = false;
    cpu->interrupt_master_enable = false;

    // set method pointers
    cpu->cpu_step_next = cpu_step_next;

    // set instruction tables
    cpu->instruction_table    = instruction_table;
    cpu->instruction_table_cb = instruction_table_cb;

    // set interrupt vector table
    cpu->interrupt_vector_table = (uint16_t*)interrupt_vector_table;

    return cpu;
}

void cpu_set_serial_output(struct CPU* cpu, bool serial_output)
{
    CPU_DEBUG_PRINT("Serial output enabled: %d\n", serial_output);
    cpu->serial_output = serial_output;
}

void free_cpu(struct CPU* cpu)
{
    if (cpu->registers) {
        free_registers(cpu->registers);
    }
    if (cpu->mmu) {
        free_mmu(cpu->mmu);
    }
    free(cpu);
}

// Private: Handle interrupts
uint8_t handle_interrupts(struct CPU* cpu)
{
    // Get interrupt flag and enable registers
    uint8_t interrupt_flag    = cpu->mmu->mmu_get_byte(cpu->mmu, INTERRUPT_FLAG_ADDRESS);
    uint8_t interrupt_enable  = cpu->mmu->mmu_get_byte(cpu->mmu, 0xFFFF);
    uint8_t interrupt_enabled = interrupt_flag & interrupt_enable;

    // No interrupts pending
    if (!interrupt_enabled) {
        return 0;
    }

    // If halted, wake up regardless of IME state
    if (cpu->halted) {
        cpu->halted = false;
        // If IME is disabled, wake up but don't service interrupt
        if (!cpu->interrupt_master_enable) {
            return 4;   // Wake up from HALT takes 4 cycles
        }
    }

    // Check if interrupt master enable is disabled
    if (!cpu->interrupt_master_enable) {
        return 0;
    }

    // Service the interrupt
    // disable further interrupts
    cpu->interrupt_master_enable = false;

// handle interrupt by priority
// from highest (Bit 0) to lowest (Bit 4)
// Bit 0: V-Blank
// Bit 1: LCDC STAT
// Bit 2: Timer Overflow
// Bit 3: Serial I/O Transfer Completion
// Bit 4: Joypad (Transition from high to low of pin number P10 - P13)
// get trailing bits
// method 1 (Intel?)
#ifdef __BMI__
    uint8_t interrupt_bit = _tzcnt_u16(interrupt_enabled);
#else
    // method 2:
    uint8_t interrupt_bit = __builtin_ctz(interrupt_enabled);
#endif

    // set that bit to 0 in interrupt flag
    cpu->mmu->mmu_set_byte(
        cpu->mmu, INTERRUPT_FLAG_ADDRESS, interrupt_flag & ~(1 << interrupt_bit));

    // calculate interrupt address
    uint16_t interrupt_address = cpu->interrupt_vector_table[interrupt_bit];

    // push pc to stack
    uint16_t old_pc = cpu->registers->get_control_register(cpu->registers, PC);
    uint16_t old_sp = cpu->registers->get_control_register(cpu->registers, SP);
    uint16_t new_sp = old_sp - 2;   // Decrement SP first
    cpu->mmu->mmu_set_word(cpu->mmu, new_sp, old_pc);
    cpu->registers->set_control_register(cpu->registers, SP, new_sp);

    // jump to interrupt address
    cpu->registers->set_control_register(cpu->registers, PC, interrupt_address);
    return 4;
}

void cpu_attach_timer(struct CPU* cpu, struct Timer* timer)
{
    cpu->timer = timer;
}

void cpu_step_for_cycles(struct CPU* cpu, int16_t cycles)
{
    while (cycles > 0) {
        uint8_t cycles_to_step = cpu_step_next(cpu);
        cycles -= cycles_to_step;
        cpu->timer->add_time(cpu->timer, cycles_to_step);
    }
    return;
}
uint8_t cpu_step_next(struct CPU* cpu)
{
    // 0. serial output
    if (cpu->serial_output) {
        if (cpu->mmu->mmu_get_byte(cpu->mmu, 0xFF02) == 0x81) {
            char c = cpu->mmu->mmu_get_byte(cpu->mmu, 0xFF01);
            printf("%c", c);
            cpu->mmu->mmu_set_byte(cpu->mmu, 0xFF02, 0x0);
        }
    }
    // 1. Check interrupts
    uint8_t interrupt_cycles = handle_interrupts(cpu);
    if (interrupt_cycles) {
        return interrupt_cycles;
    }

    // 2. halted?
    if (cpu->halted) {
        return 1;
    }

    // 3. step next instruction
    return cpu_step(cpu);
}

uint8_t cpu_step_read_byte(struct CPU* cpu)
{
    // 1. Get byte from MMU
    CPU_TRACE_PRINT("Reading byte from address: 0x%04X\n", *(cpu->registers->pc));
    uint8_t byte = cpu->mmu->mmu_get_byte(cpu->mmu, *(cpu->registers->pc));
    // 2. Increment PC
    *(cpu->registers->pc) += 1;
    CPU_TRACE_PRINT("Read byte: 0x%02X, PC is now 0x%04X\n", byte, *(cpu->registers->pc));
    return byte;
}

uint16_t cpu_step_read_word(struct CPU* cpu)
{
    // 1. Get word from MMU
    CPU_TRACE_PRINT("Reading word from address: 0x%04X\n", *(cpu->registers->pc));
    uint16_t word = cpu->mmu->mmu_get_word(cpu->mmu, *(cpu->registers->pc));
    // 2. Increment PC
    *(cpu->registers->pc) += 2;
    CPU_TRACE_PRINT("Read word: 0x%04X, PC is now 0x%04X\n", word, *(cpu->registers->pc));
    return word;
}

uint8_t cpu_step(struct CPU* cpu)
{
    // 1. Get Op Byte
    cpu->op_code = cpu_step_read_byte(cpu);

    // 2. Execute Op Code
    return cpu_step_execute_op_code(cpu, cpu->op_code);
}


uint8_t cpu_step_execute_op_code(struct CPU* cpu, uint8_t op_byte)
{
    if (op_byte == CB_PREFIX) {
        return cpu_step_execute_prefix_cb(cpu);
    }
    return cpu_step_execute_main(cpu, op_byte);
}

uint8_t cpu_step_execute_main(struct CPU* cpu, uint8_t op_byte)
{
    CPU_TRACE_PRINT("Executing Op Code: 0x%02X\n", op_byte);
    struct PackedInstructionParam* param = &cpu->instruction_table[op_byte];
    param->fn(cpu, &param->param);
    if (param->param.result_is_alternative) {
        return param->cycles_alternative;
    }
    return cpu->opcode_cycle_main[op_byte];
}

uint8_t cpu_step_execute_prefix_cb(struct CPU* cpu)
{
    // 1. Get Op Byte
    cpu->op_code = cpu_step_read_byte(cpu);
    // 2. Execute Op Code
    return cpu_step_execute_cb_op_code(cpu, cpu->op_code);
}

uint8_t cpu_step_execute_cb_op_code(struct CPU* cpu, uint8_t op_byte)
{
    CPU_TRACE_PRINT("Executing CB Op Code: 0xCB%02X\n", op_byte);
    struct PackedInstructionParam* param = &cpu->instruction_table_cb[op_byte];
    param->fn(cpu, &param->param);
    return cpu->opcode_cycle_prefix_cb[op_byte] + CB_PREFIX_CYCLES;
}

EXECUTABLE_INSTRUCTION(cpu_invalid_opcode)
{
    CPU_EMERGENCY_PRINT("Invalid Opcode: 0x%02X\n", cpu->op_code);
    exit(EXIT_FAILURE);
}

EXECUTABLE_INSTRUCTION(nop)
{
    CPU_TRACE_PRINT("NOP\n");
}

EXECUTABLE_INSTRUCTION(ld_imm_to_register_pair)
{
    enum RegisterPair register_pair = param->rp_1;
    uint16_t          value         = cpu_step_read_word(cpu);
    cpu->registers->set_register_pair(cpu->registers, register_pair, value);
}

EXECUTABLE_INSTRUCTION(ld_register_to_address_register_pair)
{
    enum Register     from_register    = param->reg_1;
    enum RegisterPair to_register_pair = param->rp_1;
    uint16_t mmu_address = cpu->registers->get_register_pair(cpu->registers, to_register_pair);
    uint8_t  value       = cpu->registers->get_register_byte(cpu->registers, from_register);
    cpu->mmu->mmu_set_byte(cpu->mmu, mmu_address, value);
}

EXECUTABLE_INSTRUCTION(ld_imm_to_register)
{
    enum Register register_to = param->reg_1;
    uint8_t       value       = cpu_step_read_byte(cpu);
    cpu->registers->set_register_byte(cpu->registers, register_to, value);
}

EXECUTABLE_INSTRUCTION(ld_register_to_register)
{
    enum Register from_register = param->reg_1;
    enum Register to_register   = param->reg_2;
    uint8_t       value         = cpu->registers->get_register_byte(cpu->registers, from_register);
    cpu->registers->set_register_byte(cpu->registers, to_register, value);
}

EXECUTABLE_INSTRUCTION(ld_address_hl_to_register)
{
    enum Register register_to = param->reg_1;
    uint16_t      address     = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t       value       = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    cpu->registers->set_register_byte(cpu->registers, register_to, value);
}

EXECUTABLE_INSTRUCTION(ld_register_to_address_hl)
{
    enum Register from_register = param->reg_1;
    uint16_t      address       = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t       value         = cpu->registers->get_register_byte(cpu->registers, from_register);
    cpu->mmu->mmu_set_byte(cpu->mmu, address, value);
}

EXECUTABLE_INSTRUCTION(ld_imm_to_address_hl)
{
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t  value   = cpu_step_read_byte(cpu);
    cpu->mmu->mmu_set_byte(cpu->mmu, address, value);
}

// Now the 16-bit load instructions:

EXECUTABLE_INSTRUCTION(ld_sp_to_address_imm)
{
    uint16_t address = cpu_step_read_word(cpu);
    uint16_t sp      = cpu->registers->get_control_register(cpu->registers, SP);
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
    uint16_t          value = cpu->registers->get_register_pair(cpu->registers, register_pair);
    uint16_t          sp    = cpu->registers->get_control_register(cpu->registers, SP);
    sp -= 2;
    cpu->mmu->mmu_set_word(cpu->mmu, sp, value);
    cpu->registers->set_control_register(cpu->registers, SP, sp);
}

EXECUTABLE_INSTRUCTION(pop_register_pair)
{
    enum RegisterPair register_pair = param->rp_1;
    uint16_t          sp            = cpu->registers->get_control_register(cpu->registers, SP);
    uint16_t          value         = cpu->mmu->mmu_get_word(cpu->mmu, sp);
    if (register_pair == AF) {
        value &= 0xFFF0;   // Lower 4 bits of F register are always zero
    }
    cpu->registers->set_register_pair(cpu->registers, register_pair, value);
    cpu->registers->set_control_register(cpu->registers, SP, sp + 2);
}

// 8-bit arithmetic instructions:

EXECUTABLE_INSTRUCTION(add_register_to_a)
{
    enum Register from_register = param->reg_1;
    uint8_t       value         = cpu->registers->get_register_byte(cpu->registers, from_register);
    uint8_t       a             = cpu->registers->get_register_byte(cpu->registers, A);
    uint16_t      result        = a + value;

    // Set flags
    cpu->registers->set_flag_z(cpu->registers, (result & 0xFF) == 0);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, (a & 0x0F) + (value & 0x0F) > 0x0F);
    cpu->registers->set_flag_c(cpu->registers, result > 0xFF);

    cpu->registers->set_register_byte(cpu->registers, A, result & 0xFF);
}

EXECUTABLE_INSTRUCTION(add_imm_to_a)
{
    uint8_t  value  = cpu_step_read_byte(cpu);
    uint8_t  a      = cpu->registers->get_register_byte(cpu->registers, A);
    uint16_t result = a + value;

    // Set flags
    cpu->registers->set_flag_z(cpu->registers, (result & 0xFF) == 0);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, (a & 0x0F) + (value & 0x0F) > 0x0F);
    cpu->registers->set_flag_c(cpu->registers, result > 0xFF);

    cpu->registers->set_register_byte(cpu->registers, A, result & 0xFF);
}

// ADC implementation

void adc_a(struct CPU* cpu, uint8_t* value)
{
    uint8_t  a      = cpu->registers->get_register_byte(cpu->registers, A);
    uint8_t  carry  = cpu->registers->get_flag_c(cpu->registers);
    uint16_t result = a + *value + carry;

    cpu->registers->set_flag_z(cpu->registers, (result & 0xFF) == 0);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, (a & 0x0F) + (*value & 0x0F) + carry > 0x0F);
    cpu->registers->set_flag_c(cpu->registers, result > 0xFF);

    cpu->registers->set_register_byte(cpu->registers, A, result & 0xFF);
}

EXECUTABLE_INSTRUCTION(adc_imm_to_a)
{
    uint8_t value = cpu_step_read_byte(cpu);
    adc_a(cpu, &value);
}

// INC/DEC implementations
void inc(struct CPU* cpu, uint8_t* value)
{
    uint8_t result = *value + 1;

    cpu->registers->set_flag_z(cpu->registers, result == 0);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, (*value & 0xF) == 0xF);

    *value = result;
}

EXECUTABLE_INSTRUCTION(inc_register)
{
    enum Register reg   = param->reg_1;
    uint8_t       value = cpu->registers->get_register_byte(cpu->registers, reg);
    inc(cpu, &value);
    cpu->registers->set_register_byte(cpu->registers, reg, value);
}

EXECUTABLE_INSTRUCTION(inc_address_hl)
{
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t  value   = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    inc(cpu, &value);
    cpu->mmu->mmu_set_byte(cpu->mmu, address, value);
}

void dec(struct CPU* cpu, uint8_t* value)
{
    uint8_t result = *value - 1;

    cpu->registers->set_flag_z(cpu->registers, result == 0);
    cpu->registers->set_flag_n(cpu->registers, true);
    cpu->registers->set_flag_h(cpu->registers, (*value & 0xF) == 0);

    *value = result;
}

EXECUTABLE_INSTRUCTION(dec_register)
{
    enum Register reg   = param->reg_1;
    uint8_t       value = cpu->registers->get_register_byte(cpu->registers, reg);
    dec(cpu, &value);
    cpu->registers->set_register_byte(cpu->registers, reg, value);
}

EXECUTABLE_INSTRUCTION(dec_address_hl)
{
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t  value   = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    dec(cpu, &value);
    cpu->mmu->mmu_set_byte(cpu->mmu, address, value);
}

void add_hl(struct CPU* cpu, uint16_t* value)
{
    uint16_t hl     = cpu->registers->get_register_pair(cpu->registers, HL);
    uint32_t result = hl + *value;

    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, (hl & 0x0FFF) + (*value & 0x0FFF) > 0x0FFF);
    cpu->registers->set_flag_c(cpu->registers, result > 0xFFFF);

    cpu->registers->set_register_pair(cpu->registers, HL, result & 0xFFFF);
}

EXECUTABLE_INSTRUCTION(add_register_pair_to_hl)
{
    enum RegisterPair rp    = param->rp_1;
    uint16_t          value = cpu->registers->get_register_pair(cpu->registers, rp);
    add_hl(cpu, &value);
}

EXECUTABLE_INSTRUCTION(add_sp_to_hl)
{
    uint16_t sp = cpu->registers->get_control_register(cpu->registers, SP);
    add_hl(cpu, &sp);
}

EXECUTABLE_INSTRUCTION(add_imm_to_sp)
{
    int8_t   value  = (int8_t)cpu_step_read_byte(cpu);
    uint16_t sp     = cpu->registers->get_control_register(cpu->registers, SP);
    uint16_t result = sp + value;

    cpu->registers->set_flag_z(cpu->registers, false);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, (sp & 0x0F) + (value & 0x0F) > 0x0F);
    cpu->registers->set_flag_c(cpu->registers, (sp & 0xFF) + (value & 0xFF) > 0xFF);

    cpu->registers->set_control_register(cpu->registers, SP, result);
}

EXECUTABLE_INSTRUCTION(daa)
{
    uint8_t a      = cpu->registers->get_register_byte(cpu->registers, A);
    bool    carry  = cpu->registers->get_flag_c(cpu->registers);
    uint8_t adjust = carry ? 0x60 : 0x00;

    bool half_carry = cpu->registers->get_flag_h(cpu->registers);
    if (half_carry) {
        adjust |= 0x06;
    }

    bool subtract = cpu->registers->get_flag_n(cpu->registers);
    if (!subtract) {
        if ((a & 0x0F) > 0x09) {
            adjust |= 0x06;
        }
        if (a > 0x99) {
            adjust |= 0x60;
        }
        a += adjust;
    }
    else {
        a -= adjust;
    }

    // flags
    cpu->registers->set_flag_z(cpu->registers, a == 0);
    cpu->registers->set_flag_h(cpu->registers, false);
    cpu->registers->set_flag_c(cpu->registers, adjust >= 0x60);

    cpu->registers->set_register_byte(cpu->registers, A, a);
}

EXECUTABLE_INSTRUCTION(cpl)
{
    uint8_t a = cpu->registers->get_register_byte(cpu->registers, A);
    a         = ~a;
    cpu->registers->set_flag_n(cpu->registers, true);
    cpu->registers->set_flag_h(cpu->registers, true);
    cpu->registers->set_register_byte(cpu->registers, A, a);
}

// Rotate and shift instructions
EXECUTABLE_INSTRUCTION(rlca)
{
    uint8_t a     = cpu->registers->get_register_byte(cpu->registers, A);
    uint8_t carry = (a & 0x80) >> 7;
    a             = (a << 1) | carry;

    cpu->registers->set_flag_z(cpu->registers, false);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, false);
    cpu->registers->set_flag_c(cpu->registers, carry);

    cpu->registers->set_register_byte(cpu->registers, A, a);
}

EXECUTABLE_INSTRUCTION(rla)
{
    uint8_t a         = cpu->registers->get_register_byte(cpu->registers, A);
    uint8_t old_carry = cpu->registers->get_flag_c(cpu->registers);
    uint8_t new_carry = (a & 0x80) >> 7;
    a                 = (a << 1) | old_carry;

    cpu->registers->set_flag_z(cpu->registers, false);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, false);
    cpu->registers->set_flag_c(cpu->registers, new_carry);

    cpu->registers->set_register_byte(cpu->registers, A, a);
}

EXECUTABLE_INSTRUCTION(rrca)
{
    uint8_t a     = cpu->registers->get_register_byte(cpu->registers, A);
    uint8_t carry = a & 0x01;
    a             = (a >> 1) | (carry << 7);

    cpu->registers->set_flag_z(cpu->registers, false);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, false);
    cpu->registers->set_flag_c(cpu->registers, carry);

    cpu->registers->set_register_byte(cpu->registers, A, a);
}

EXECUTABLE_INSTRUCTION(rra)
{
    uint8_t a         = cpu->registers->get_register_byte(cpu->registers, A);
    uint8_t old_carry = cpu->registers->get_flag_c(cpu->registers);
    uint8_t new_carry = a & 0x01;
    a                 = (a >> 1) | (old_carry << 7);

    cpu->registers->set_flag_z(cpu->registers, false);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, false);
    cpu->registers->set_flag_c(cpu->registers, new_carry);

    cpu->registers->set_register_byte(cpu->registers, A, a);
}

// Jump instructions
EXECUTABLE_INSTRUCTION(jp_imm)
{
    uint16_t address = cpu_step_read_word(cpu);
    cpu->registers->set_control_register(cpu->registers, PC, address);
}

EXECUTABLE_INSTRUCTION(jp_hl)
{
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    cpu->registers->set_control_register(cpu->registers, PC, address);
}

EXECUTABLE_INSTRUCTION(jp_cc_imm)
{
    uint16_t           address   = cpu_step_read_word(cpu);
    enum JumpCondition condition = param->cc;
    bool               jump      = false;

    switch (condition) {
    case JumpCondition_NZ: jump = !cpu->registers->get_flag_z(cpu->registers); break;
    case JumpCondition_Z: jump = cpu->registers->get_flag_z(cpu->registers); break;
    case JumpCondition_NC: jump = !cpu->registers->get_flag_c(cpu->registers); break;
    case JumpCondition_C: jump = cpu->registers->get_flag_c(cpu->registers); break;
    }

    if (jump) {
        cpu->registers->set_control_register(cpu->registers, PC, address);
        param->result_is_alternative = true;
    }
}

EXECUTABLE_INSTRUCTION(jr_imm)
{
    int8_t   offset = (int8_t)cpu_step_read_byte(cpu);
    uint16_t pc     = cpu->registers->get_control_register(cpu->registers, PC);
    cpu->registers->set_control_register(cpu->registers, PC, pc + offset);
}

EXECUTABLE_INSTRUCTION(jr_cc_imm)
{
    int8_t             offset    = (int8_t)cpu_step_read_byte(cpu);
    enum JumpCondition condition = param->cc;
    bool               jump      = false;

    switch (condition) {
    case JumpCondition_NZ: jump = !cpu->registers->get_flag_z(cpu->registers); break;
    case JumpCondition_Z: jump = cpu->registers->get_flag_z(cpu->registers); break;
    case JumpCondition_NC: jump = !cpu->registers->get_flag_c(cpu->registers); break;
    case JumpCondition_C: jump = cpu->registers->get_flag_c(cpu->registers); break;
    }

    if (jump) {
        uint16_t pc = cpu->registers->get_control_register(cpu->registers, PC);
        cpu->registers->set_control_register(cpu->registers, PC, pc + offset);
        param->result_is_alternative = true;
    }
}

EXECUTABLE_INSTRUCTION(call_imm)
{
    uint16_t address = cpu_step_read_word(cpu);
    uint16_t pc      = cpu->registers->get_control_register(cpu->registers, PC);
    uint16_t sp      = cpu->registers->get_control_register(cpu->registers, SP);

    sp -= 2;
    cpu->mmu->mmu_set_word(cpu->mmu, sp, pc);
    cpu->registers->set_control_register(cpu->registers, SP, sp);
    cpu->registers->set_control_register(cpu->registers, PC, address);
}

EXECUTABLE_INSTRUCTION(call_cc_imm)
{
    uint16_t           address   = cpu_step_read_word(cpu);
    enum JumpCondition condition = param->cc;
    bool               jump      = false;

    switch (condition) {
    case JumpCondition_NZ: jump = !cpu->registers->get_flag_z(cpu->registers); break;
    case JumpCondition_Z: jump = cpu->registers->get_flag_z(cpu->registers); break;
    case JumpCondition_NC: jump = !cpu->registers->get_flag_c(cpu->registers); break;
    case JumpCondition_C: jump = cpu->registers->get_flag_c(cpu->registers); break;
    }

    if (jump) {
        uint16_t pc = cpu->registers->get_control_register(cpu->registers, PC);
        uint16_t sp = cpu->registers->get_control_register(cpu->registers, SP);
        sp -= 2;
        cpu->mmu->mmu_set_word(cpu->mmu, sp, pc);
        cpu->registers->set_control_register(cpu->registers, SP, sp);
        cpu->registers->set_control_register(cpu->registers, PC, address);
        param->result_is_alternative = true;
    }
}

EXECUTABLE_INSTRUCTION(ret)
{
    uint16_t sp      = cpu->registers->get_control_register(cpu->registers, SP);
    uint16_t address = cpu->mmu->mmu_get_word(cpu->mmu, sp);
    cpu->registers->set_control_register(cpu->registers, SP, sp + 2);
    cpu->registers->set_control_register(cpu->registers, PC, address);
}

EXECUTABLE_INSTRUCTION(ret_cc)
{
    enum JumpCondition condition = param->cc;
    bool               jump      = false;

    switch (condition) {
    case JumpCondition_NZ: jump = !cpu->registers->get_flag_z(cpu->registers); break;
    case JumpCondition_Z: jump = cpu->registers->get_flag_z(cpu->registers); break;
    case JumpCondition_NC: jump = !cpu->registers->get_flag_c(cpu->registers); break;
    case JumpCondition_C: jump = cpu->registers->get_flag_c(cpu->registers); break;
    }

    if (jump) {
        uint16_t sp      = cpu->registers->get_control_register(cpu->registers, SP);
        uint16_t address = cpu->mmu->mmu_get_word(cpu->mmu, sp);
        cpu->registers->set_control_register(cpu->registers, SP, sp + 2);
        cpu->registers->set_control_register(cpu->registers, PC, address);
        param->result_is_alternative = true;
    }
}

EXECUTABLE_INSTRUCTION(reti)
{
    uint16_t sp      = cpu->registers->get_control_register(cpu->registers, SP);
    uint16_t address = cpu->mmu->mmu_get_word(cpu->mmu, sp);
    cpu->registers->set_control_register(cpu->registers, SP, sp + 2);
    cpu->registers->set_control_register(cpu->registers, PC, address);
    // Enable interrupts
    cpu->interrupt_master_enable = true;
}

void rst(struct CPU* cpu, uint16_t n)
{
    uint16_t pc = cpu->registers->get_control_register(cpu->registers, PC);
    uint16_t sp = cpu->registers->get_control_register(cpu->registers, SP);

    sp -= 2;
    cpu->mmu->mmu_set_word(cpu->mmu, sp, pc);
    cpu->registers->set_control_register(cpu->registers, SP, sp);
    cpu->registers->set_control_register(cpu->registers, PC, n);
}

EXECUTABLE_INSTRUCTION(rst_00h)
{
    rst(cpu, 0x0000);
}

EXECUTABLE_INSTRUCTION(rst_08h)
{
    rst(cpu, 0x0008);
}

EXECUTABLE_INSTRUCTION(rst_10h)
{
    rst(cpu, 0x0010);
}

EXECUTABLE_INSTRUCTION(rst_18h)
{
    rst(cpu, 0x0018);
}

EXECUTABLE_INSTRUCTION(rst_20h)
{
    rst(cpu, 0x0020);
}

EXECUTABLE_INSTRUCTION(rst_28h)
{
    rst(cpu, 0x0028);
}

EXECUTABLE_INSTRUCTION(rst_30h)
{
    rst(cpu, 0x0030);
}

EXECUTABLE_INSTRUCTION(rst_38h)
{
    rst(cpu, 0x0038);
}

// Helper functions for 16-bit arithmetic
void inc_16_bit(struct CPU* cpu, uint16_t* value)
{
    (*value)++;
}

void dec_16_bit(struct CPU* cpu, uint16_t* value)
{
    (*value)--;
}

EXECUTABLE_INSTRUCTION(inc_register_pair)
{
    enum RegisterPair rp    = param->rp_1;
    uint16_t          value = cpu->registers->get_register_pair(cpu->registers, rp);
    inc_16_bit(cpu, &value);
    cpu->registers->set_register_pair(cpu->registers, rp, value);
}

EXECUTABLE_INSTRUCTION(dec_register_pair)
{
    enum RegisterPair rp    = param->rp_1;
    uint16_t          value = cpu->registers->get_register_pair(cpu->registers, rp);
    dec_16_bit(cpu, &value);
    cpu->registers->set_register_pair(cpu->registers, rp, value);
}

EXECUTABLE_INSTRUCTION(inc_sp)
{
    uint16_t sp = cpu->registers->get_control_register(cpu->registers, SP);
    inc_16_bit(cpu, &sp);
    cpu->registers->set_control_register(cpu->registers, SP, sp);
}

EXECUTABLE_INSTRUCTION(dec_sp)
{
    uint16_t sp = cpu->registers->get_control_register(cpu->registers, SP);
    dec_16_bit(cpu, &sp);
    cpu->registers->set_control_register(cpu->registers, SP, sp);
}

EXECUTABLE_INSTRUCTION(ccf)
{
    bool current_carry = cpu->registers->get_flag_c(cpu->registers);

    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, false);
    cpu->registers->set_flag_c(cpu->registers, !current_carry);
}

EXECUTABLE_INSTRUCTION(scf)
{
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, false);
    cpu->registers->set_flag_c(cpu->registers, true);
}

EXECUTABLE_INSTRUCTION(halt)
{
    // Game Boy HALT bug: When IME=0 and interrupts are pending,
    // the next instruction after HALT gets executed twice
    uint8_t interrupt_flag   = cpu->mmu->mmu_get_byte(cpu->mmu, INTERRUPT_FLAG_ADDRESS);
    uint8_t interrupt_enable = cpu->mmu->mmu_get_byte(cpu->mmu, 0xFFFF);

    if (!cpu->interrupt_master_enable && (interrupt_flag & interrupt_enable)) {
        // HALT bug: Don't increment PC on next instruction fetch
        // This causes the instruction after HALT to execute twice
        uint16_t pc = cpu->registers->get_control_register(cpu->registers, PC);
        cpu->registers->set_control_register(cpu->registers, PC, pc - 1);
    }
    else {
        cpu->halted = true;
    }
}

EXECUTABLE_INSTRUCTION(stop)
{
    cpu->stopped = true;
}

EXECUTABLE_INSTRUCTION(di)
{
    cpu->interrupt_master_enable = false;
}

EXECUTABLE_INSTRUCTION(ei)
{
    cpu->interrupt_master_enable = true;
}

EXECUTABLE_INSTRUCTION(ld_sp_plus_imm_to_hl)
{
    int8_t   offset = (int8_t)cpu_step_read_byte(cpu);
    uint16_t sp     = cpu->registers->get_control_register(cpu->registers, SP);

    cpu->registers->set_flag_z(cpu->registers, false);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, (sp & 0x0F) + (offset & 0x0F) > 0x0F);
    cpu->registers->set_flag_c(cpu->registers, (sp & 0xFF) + (offset & 0xFF) > 0xFF);

    cpu->registers->set_register_pair(cpu->registers, HL, sp + offset);
}

EXECUTABLE_INSTRUCTION(ld_a_to_address_imm)
{
    uint16_t address = cpu_step_read_word(cpu);
    uint8_t  a       = cpu->registers->get_register_byte(cpu->registers, A);
    cpu->mmu->mmu_set_byte(cpu->mmu, address, a);
}

EXECUTABLE_INSTRUCTION(ld_address_imm_to_a)
{
    uint16_t address = cpu_step_read_word(cpu);
    uint8_t  a       = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    cpu->registers->set_register_byte(cpu->registers, A, a);
}

EXECUTABLE_INSTRUCTION(ld_a_to_zero_page_address_c)
{
    uint8_t offset = cpu->registers->get_register_byte(cpu->registers, C);
    uint8_t a      = cpu->registers->get_register_byte(cpu->registers, A);
    cpu->mmu->mmu_set_byte(cpu->mmu, 0xFF00 + offset, a);
}

EXECUTABLE_INSTRUCTION(ld_a_to_zero_page_address_imm)
{
    uint8_t offset = cpu_step_read_byte(cpu);
    uint8_t a      = cpu->registers->get_register_byte(cpu->registers, A);
    cpu->mmu->mmu_set_byte(cpu->mmu, 0xFF00 + offset, a);
}

EXECUTABLE_INSTRUCTION(ld_zero_page_address_imm_to_a)
{
    uint8_t offset = cpu_step_read_byte(cpu);
    uint8_t a      = cpu->mmu->mmu_get_byte(cpu->mmu, 0xFF00 + offset);
    cpu->registers->set_register_byte(cpu->registers, A, a);
}

EXECUTABLE_INSTRUCTION(ld_zero_page_address_c_to_a)
{
    uint8_t offset = cpu->registers->get_register_byte(cpu->registers, C);
    uint8_t a      = cpu->mmu->mmu_get_byte(cpu->mmu, 0xFF00 + offset);
    cpu->registers->set_register_byte(cpu->registers, A, a);
}

EXECUTABLE_INSTRUCTION(ld_imm_to_sp)
{
    uint16_t value = cpu_step_read_word(cpu);
    cpu->registers->set_control_register(cpu->registers, SP, value);
}

EXECUTABLE_INSTRUCTION(ld_a_to_address_hl_inc_hl)
{
    uint16_t hl = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t  a  = cpu->registers->get_register_byte(cpu->registers, A);
    cpu->mmu->mmu_set_byte(cpu->mmu, hl, a);
    cpu->registers->set_register_pair(cpu->registers, HL, hl + 1);
}

EXECUTABLE_INSTRUCTION(ld_address_hl_to_a_inc_hl)
{
    uint16_t hl = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t  a  = cpu->mmu->mmu_get_byte(cpu->mmu, hl);
    cpu->registers->set_register_byte(cpu->registers, A, a);
    cpu->registers->set_register_pair(cpu->registers, HL, hl + 1);
}

EXECUTABLE_INSTRUCTION(ld_a_to_address_hl_dec_hl)
{
    uint16_t hl = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t  a  = cpu->registers->get_register_byte(cpu->registers, A);
    cpu->mmu->mmu_set_byte(cpu->mmu, hl, a);
    cpu->registers->set_register_pair(cpu->registers, HL, hl - 1);
}

EXECUTABLE_INSTRUCTION(ld_address_hl_to_a_dec_hl)
{
    uint16_t hl = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t  a  = cpu->mmu->mmu_get_byte(cpu->mmu, hl);
    cpu->registers->set_register_byte(cpu->registers, A, a);
    cpu->registers->set_register_pair(cpu->registers, HL, hl - 1);
}

EXECUTABLE_INSTRUCTION(ld_address_register_pair_to_register)
{
    enum Register     reg     = param->reg_1;
    enum RegisterPair rp      = param->rp_1;
    uint16_t          address = cpu->registers->get_register_pair(cpu->registers, rp);
    uint8_t           value   = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    cpu->registers->set_register_byte(cpu->registers, reg, value);
}

EXECUTABLE_INSTRUCTION(add_address_hl_to_a)
{
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t  value   = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    add_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(adc_address_hl_to_a)
{
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t  value   = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    adc_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(sub_address_hl_to_a)
{
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t  value   = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    sub_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(sbc_address_hl_to_a)
{
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t  value   = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    sbc_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(and_address_hl_to_a)
{
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t  value   = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    and_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(xor_address_hl_to_a)
{
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t  value   = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    xor_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(or_address_hl_to_a)
{
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t  value   = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    or_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(cp_address_hl_to_a)
{
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t  value   = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    cp_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(adc_register_to_a)
{
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, param->reg_1);
    adc_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(sub_register_to_a)
{
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, param->reg_1);
    sub_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(sbc_register_to_a)
{
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, param->reg_1);
    sbc_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(and_register_to_a)
{
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, param->reg_1);
    and_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(xor_register_to_a)
{
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, param->reg_1);
    xor_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(or_register_to_a)
{
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, param->reg_1);
    or_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(cp_register_to_a)
{
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, param->reg_1);
    cp_a(cpu, &value);
}

// Immediate operations
EXECUTABLE_INSTRUCTION(cp_imm_to_a)
{
    uint8_t value = cpu_step_read_byte(cpu);
    cp_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(or_imm_to_a)
{
    uint8_t value = cpu_step_read_byte(cpu);
    or_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(xor_imm_to_a)
{
    uint8_t value = cpu_step_read_byte(cpu);
    xor_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(and_imm_to_a)
{
    uint8_t value = cpu_step_read_byte(cpu);
    and_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(sbc_imm_to_a)
{
    uint8_t value = cpu_step_read_byte(cpu);
    sbc_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(sub_imm_to_a)
{
    uint8_t value = cpu_step_read_byte(cpu);
    sub_a(cpu, &value);
}

EXECUTABLE_INSTRUCTION(jp_address_hl)
{
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    cpu->registers->set_control_register(cpu->registers, PC, address);
}

EXECUTABLE_INSTRUCTION(prefix_cb)
{
    uint8_t op_byte = cpu_step_read_byte(cpu);
    cpu_step_execute_cb_op_code(cpu, op_byte);
}

void add_a(struct CPU* cpu, uint8_t* value)
{
    uint8_t  a      = cpu->registers->get_register_byte(cpu->registers, A);
    uint16_t result = a + *value;

    cpu->registers->set_flag_z(cpu->registers, (result & 0xFF) == 0);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, (a & 0xF) + (*value & 0xF) > 0xF);
    cpu->registers->set_flag_c(cpu->registers, result > 0xFF);

    cpu->registers->set_register_byte(cpu->registers, A, result & 0xFF);
}

void sub_a(struct CPU* cpu, uint8_t* value)
{
    uint8_t  a      = cpu->registers->get_register_byte(cpu->registers, A);
    uint16_t result = a - *value;

    cpu->registers->set_flag_z(cpu->registers, (result & 0xFF) == 0);
    cpu->registers->set_flag_n(cpu->registers, true);
    cpu->registers->set_flag_h(cpu->registers, (a & 0xF) < (*value & 0xF));
    cpu->registers->set_flag_c(cpu->registers, a < *value);

    cpu->registers->set_register_byte(cpu->registers, A, result & 0xFF);
}

void sbc_a(struct CPU* cpu, uint8_t* value)
{
    uint8_t  a      = cpu->registers->get_register_byte(cpu->registers, A);
    uint8_t  carry  = cpu->registers->get_flag_c(cpu->registers);
    uint16_t result = a - *value - carry;

    cpu->registers->set_flag_z(cpu->registers, (result & 0xFF) == 0);
    cpu->registers->set_flag_n(cpu->registers, true);
    cpu->registers->set_flag_h(cpu->registers, (a & 0xF) < ((*value & 0xF) + carry));
    cpu->registers->set_flag_c(cpu->registers, a < (*value + carry));

    cpu->registers->set_register_byte(cpu->registers, A, result & 0xFF);
}

void and_a(struct CPU* cpu, uint8_t* value)
{
    uint8_t a      = cpu->registers->get_register_byte(cpu->registers, A);
    uint8_t result = a & *value;

    cpu->registers->set_flag_z(cpu->registers, result == 0);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, true);
    cpu->registers->set_flag_c(cpu->registers, false);

    cpu->registers->set_register_byte(cpu->registers, A, result);
}

void xor_a(struct CPU* cpu, uint8_t* value)
{
    uint8_t a      = cpu->registers->get_register_byte(cpu->registers, A);
    uint8_t result = a ^ *value;

    cpu->registers->set_flag_z(cpu->registers, result == 0);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, false);
    cpu->registers->set_flag_c(cpu->registers, false);

    cpu->registers->set_register_byte(cpu->registers, A, result);
}

void or_a(struct CPU* cpu, uint8_t* value)
{
    uint8_t a      = cpu->registers->get_register_byte(cpu->registers, A);
    uint8_t result = a | *value;

    cpu->registers->set_flag_z(cpu->registers, result == 0);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, false);
    cpu->registers->set_flag_c(cpu->registers, false);

    cpu->registers->set_register_byte(cpu->registers, A, result);
}

void cp_a(struct CPU* cpu, uint8_t* value)
{
    uint8_t a      = cpu->registers->get_register_byte(cpu->registers, A);
    uint8_t result = a - *value;

    cpu->registers->set_flag_z(cpu->registers, result == 0);
    cpu->registers->set_flag_n(cpu->registers, true);
    cpu->registers->set_flag_h(cpu->registers, (a & 0xF) < (*value & 0xF));
    cpu->registers->set_flag_c(cpu->registers, a < *value);
}

// CB prefix functions

uint8_t rlc(struct CPU* cpu, uint8_t value)
{
    uint8_t carry = (value & 0x80) >> 7;
    value         = (value << 1) | carry;

    cpu->registers->set_flag_z(cpu->registers, value == 0);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, false);
    cpu->registers->set_flag_c(cpu->registers, carry);

    return value;
}

EXECUTABLE_INSTRUCTION(rlc_register)
{
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, param->reg_1);
    cpu->registers->set_register_byte(cpu->registers, param->reg_1, rlc(cpu, value));
}

EXECUTABLE_INSTRUCTION(rlc_address_hl)
{
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t  value   = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    cpu->mmu->mmu_set_byte(cpu->mmu, address, rlc(cpu, value));
}

uint8_t rrc(struct CPU* cpu, uint8_t value)
{
    uint8_t carry = value & 0x01;
    value         = (value >> 1) | (carry << 7);

    cpu->registers->set_flag_z(cpu->registers, value == 0);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, false);
    cpu->registers->set_flag_c(cpu->registers, carry);

    return value;
}

EXECUTABLE_INSTRUCTION(rrc_register)
{
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, param->reg_1);
    cpu->registers->set_register_byte(cpu->registers, param->reg_1, rrc(cpu, value));
}

EXECUTABLE_INSTRUCTION(rrc_address_hl)
{
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t  value   = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    cpu->mmu->mmu_set_byte(cpu->mmu, address, rrc(cpu, value));
}

uint8_t rl(struct CPU* cpu, uint8_t value)
{
    uint8_t old_carry = cpu->registers->get_flag_c(cpu->registers);
    uint8_t new_carry = (value & 0x80) >> 7;
    value             = (value << 1) | old_carry;

    cpu->registers->set_flag_z(cpu->registers, value == 0);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, false);
    cpu->registers->set_flag_c(cpu->registers, new_carry);

    return value;
}

EXECUTABLE_INSTRUCTION(rl_register)
{
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, param->reg_1);
    cpu->registers->set_register_byte(cpu->registers, param->reg_1, rl(cpu, value));
}

EXECUTABLE_INSTRUCTION(rl_address_hl)
{
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t  value   = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    cpu->mmu->mmu_set_byte(cpu->mmu, address, rl(cpu, value));
}

uint8_t rr(struct CPU* cpu, uint8_t value)
{
    uint8_t old_carry = cpu->registers->get_flag_c(cpu->registers);
    uint8_t new_carry = value & 0x01;
    value             = (value >> 1) | (old_carry << 7);

    cpu->registers->set_flag_z(cpu->registers, value == 0);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, false);
    cpu->registers->set_flag_c(cpu->registers, new_carry);

    return value;
}

EXECUTABLE_INSTRUCTION(rr_register)
{
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, param->reg_1);
    cpu->registers->set_register_byte(cpu->registers, param->reg_1, rr(cpu, value));
}

EXECUTABLE_INSTRUCTION(rr_address_hl)
{
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t  value   = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    cpu->mmu->mmu_set_byte(cpu->mmu, address, rr(cpu, value));
}

uint8_t sla(struct CPU* cpu, uint8_t value)
{
    uint8_t carry = (value & 0x80) >> 7;
    value         = value << 1;

    cpu->registers->set_flag_z(cpu->registers, value == 0);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, false);
    cpu->registers->set_flag_c(cpu->registers, carry);

    return value;
}

EXECUTABLE_INSTRUCTION(sla_register)
{
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, param->reg_1);
    cpu->registers->set_register_byte(cpu->registers, param->reg_1, sla(cpu, value));
}

EXECUTABLE_INSTRUCTION(sla_address_hl)
{
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t  value   = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    cpu->mmu->mmu_set_byte(cpu->mmu, address, sla(cpu, value));
}

uint8_t sra(struct CPU* cpu, uint8_t value)
{
    uint8_t carry = value & 0x01;
    value         = (value >> 1) | (value & 0x80);

    cpu->registers->set_flag_z(cpu->registers, value == 0);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, false);
    cpu->registers->set_flag_c(cpu->registers, carry);

    return value;
}

EXECUTABLE_INSTRUCTION(sra_register)
{
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, param->reg_1);
    uint8_t msb   = value & 0x80;
    cpu->registers->set_register_byte(cpu->registers, param->reg_1, sra(cpu, value));
}

EXECUTABLE_INSTRUCTION(sra_address_hl)
{
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t  value   = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    uint8_t  msb     = value & 0x80;
    cpu->mmu->mmu_set_byte(cpu->mmu, address, sra(cpu, value));
}

// Bit manipulation instructions
uint8_t swap(struct CPU* cpu, uint8_t value)
{
    uint8_t result = (value >> 4) | (value << 4);

    cpu->registers->set_flag_z(cpu->registers, result == 0);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, false);
    cpu->registers->set_flag_c(cpu->registers, false);

    return result;
}

EXECUTABLE_INSTRUCTION(swap_register)
{
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, param->reg_1);
    cpu->registers->set_register_byte(cpu->registers, param->reg_1, swap(cpu, value));
}

EXECUTABLE_INSTRUCTION(swap_address_hl)
{
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t  value   = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    cpu->mmu->mmu_set_byte(cpu->mmu, address, swap(cpu, value));
}

uint8_t srl(struct CPU* cpu, uint8_t value)
{
    uint8_t carry = value & 0x01;
    value         = value >> 1;

    cpu->registers->set_flag_z(cpu->registers, value == 0);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, false);
    cpu->registers->set_flag_c(cpu->registers, carry);

    return value;
}

EXECUTABLE_INSTRUCTION(srl_register)
{
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, param->reg_1);
    cpu->registers->set_register_byte(cpu->registers, param->reg_1, srl(cpu, value));
}

EXECUTABLE_INSTRUCTION(srl_address_hl)
{
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t  value   = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    cpu->mmu->mmu_set_byte(cpu->mmu, address, srl(cpu, value));
}



void bit(struct CPU* cpu, uint8_t value, uint8_t bit_position)
{
    bool is_bit_set = (value >> bit_position) & 0x01;

    cpu->registers->set_flag_z(cpu->registers, !is_bit_set);
    cpu->registers->set_flag_n(cpu->registers, false);
    cpu->registers->set_flag_h(cpu->registers, true);
    // C flag is not affected
}

EXECUTABLE_INSTRUCTION(bit_register)
{
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, param->reg_1);
    bit(cpu, value, param->bit_position);
}

EXECUTABLE_INSTRUCTION(bit_address_hl)
{
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t  value   = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    bit(cpu, value, param->bit_position);
}

uint8_t res(struct CPU* cpu, uint8_t value, uint8_t bit_position)
{
    return value & ~(1 << bit_position);
}

EXECUTABLE_INSTRUCTION(res_register)
{
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, param->reg_1);
    cpu->registers->set_register_byte(
        cpu->registers, param->reg_1, res(cpu, value, param->bit_position));
}

EXECUTABLE_INSTRUCTION(res_address_hl)
{
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t  value   = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    cpu->mmu->mmu_set_byte(cpu->mmu, address, res(cpu, value, param->bit_position));
}

uint8_t set(struct CPU* cpu, uint8_t bit, uint8_t value)
{
    return value | (1 << bit);
}

EXECUTABLE_INSTRUCTION(set_register)
{
    uint8_t value = cpu->registers->get_register_byte(cpu->registers, param->reg_1);
    cpu->registers->set_register_byte(
        cpu->registers, param->reg_1, set(cpu, param->bit_position, value));
}

EXECUTABLE_INSTRUCTION(set_address_hl)
{
    uint16_t address = cpu->registers->get_register_pair(cpu->registers, HL);
    uint8_t  value   = cpu->mmu->mmu_get_byte(cpu->mmu, address);
    cpu->mmu->mmu_set_byte(cpu->mmu, address, set(cpu, param->bit_position, value));
}
