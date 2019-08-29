#ifndef CHIP8_OPCODE_H
#define CHIP8_OPCODE_H

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cassert>

#include "common.h"
#include "cpu.h"

// opcode access helper, assumed little-endian
union Opcode {
    word word;
    struct {
        byte low;
        byte high;
    };
};

class Operations {
public:
    // 00E0, clear screen
    static void cls(Cpu &cpu, Opcode = {}) {
        std::fill_n(cpu.vram, sizeof(cpu.vram), 0);
        cpu.update_gui = true;
    
        if (cpu.debug)
            printf("CLS\n");
    }

    // 00EE, return
    static void ret(Cpu &cpu, Opcode = {}) {
        cpu.reg.pc = cpu.stack[--cpu.reg.sp];
    
        if (cpu.debug)
            printf("RET\n");
    }

    // 1NNN, jump
    static void jump(Cpu &cpu, Opcode code) {
        cpu.reg.pc = code.word;
        
        if (cpu.debug)
            printf("JP   0x%04X\n", code.word);
    }

    // 2NNN, call
    static void call(Cpu &cpu, Opcode code) {
        cpu.stack[cpu.reg.sp++] = cpu.reg.pc;
        cpu.reg.pc = code.word;
    
        if (cpu.debug)
            printf("CALL 0x%04X\n", code.word);
    }

    // 3XKK, skip next instruction if reg[x] == kk
    static void skip_eq(Cpu &cpu, Opcode code) {
        byte vx = code.high;
        byte value = code.low;

        if (cpu.reg.v[vx] == value)
            cpu.reg.pc += 2;
    
        if (cpu.debug)
            printf("SE   V%X, 0x%04X\n", vx, value);
    }

    // 4XKK, skip next instruction if reg[x] != kk
    static void skip_not_eq(Cpu &cpu, Opcode code) {
        byte vx = code.high;
        byte value = code.low;

        if (cpu.reg.v[vx] != value)
            cpu.reg.pc += 2;
    
        if (cpu.debug)
            printf("SNE  V%X, 0x%04X\n", vx, value);
    }

    // 5XY0, skip next instruction if reg[x] == reg[y]
    static void skip_eq_reg(Cpu &cpu, Opcode code) {
        byte vx = code.high;
        byte vy = (code.low & 0xf0) >> 4;

        if (cpu.reg.v[vx] == cpu.reg.v[vy])
            cpu.reg.pc += 2;
    
        if (cpu.debug)
            printf("SE   V%X, V%X\n", vx, vy);
    }

    // 6XKK, load reg: reg[x] = kk
    static void load_reg_value(Cpu &cpu, Opcode code) {
        byte vx = code.high;
        byte value = code.low;

        cpu.reg.v[vx] = value;
    
        if (cpu.debug)
            printf("LD   V%X, 0x%04X\n", vx, value);
    }

    // 7XKK, add reg: reg[x] += kk
    static void add_reg_value(Cpu &cpu, Opcode code) {
        byte vx = code.high;
        byte value = code.low;

        cpu.reg.v[vx] += value;
    
        if (cpu.debug)
            printf("ADD  V%X, 0x%04X\n", vx, value);
    }

    // 8XY0, load reg: reg[x] = reg[y]
    static void load_reg_reg(Cpu &cpu, Opcode code) {
        byte vx = code.high;
        byte vy = (code.low & 0xf0) >> 4;
        
        cpu.reg.v[vx] = cpu.reg.v[vy];
    
        if (cpu.debug)
            printf("LD   V%X, V%X\n", vx, vy);
    }

    // 8XY1, or: reg[x] |= reg[y]
    static void or_reg_reg(Cpu &cpu, Opcode code) {
        byte vx = code.high;
        byte vy = (code.low & 0xf0) >> 4;
        
        cpu.reg.v[vx] |= cpu.reg.v[vy];
    
        if (cpu.debug)
            printf("OR   V%X, V%X\n", vx, vy);
    }

    // 8XY2, and: reg[x] &= reg[y]
    static void and_reg_reg(Cpu &cpu, Opcode code) {
        byte vx = code.high;
        byte vy = (code.low & 0xf0) >> 4;
        
        cpu.reg.v[vx] &= cpu.reg.v[vy];
    
        if (cpu.debug)
            printf("AND  V%X, V%X\n", vx, vy);
    }

    // 8XY3, xor: reg[x] ^= reg[y]
    static void xor_reg_reg(Cpu &cpu, Opcode code) {
        byte vx = code.high;
        byte vy = (code.low & 0xf0) >> 4;
        
        cpu.reg.v[vx] ^= cpu.reg.v[vy];
    
        if (cpu.debug)
            printf("XOR  V%X, V%X\n", vx, vy);
    }

    // 8XY4, add: reg[x] += reg[y]
    static void add_reg_reg(Cpu &cpu, Opcode code) {
        byte vx = code.high;
        byte vy = (code.low & 0xf0) >> 4;
        
        word sum = cpu.reg.v[vx] + cpu.reg.v[vy];
        cpu.reg.v_flag = (sum > 0xff) ? 1 : 0;
        cpu.reg.v[vx] = sum % 0x100;
    
        if (cpu.debug)
            printf("ADD  V%X, V%X\n", vx, vy);
    }

    // 8XY5, sub: reg[x] -= reg[y]
    static void sub_reg_reg(Cpu &cpu, Opcode code) {
        byte vx = code.high;
        byte vy = (code.low & 0xf0) >> 4;

        cpu.reg.v_flag = (cpu.reg.v[vx] > cpu.reg.v[vy]) ? 1 : 0;
        cpu.reg.v[vx] -= cpu.reg.v[vy];
    
        if (cpu.debug)
            printf("SUB  V%X, V%X\n", vx, vy);
    }

    // 8XY6, shift right
    static void shr_reg_reg(Cpu &cpu, Opcode code) {
        byte vx = code.high;
        byte vy = (code.low & 0xf0) >> 4;
        
        cpu.reg.v_flag = cpu.reg.v[vy] & 0x01;
        cpu.reg.v[vx] = cpu.reg.v[vy] >> 1;
    
        if (cpu.debug)
            printf("SHR  V%X, V%X\n", vx, vy);
    }

    // 8XY7, sub negative: reg[x] = reg[y] - reg[x]
    static void subn_reg_reg(Cpu &cpu, Opcode code) {
        byte vx = code.high;
        byte vy = (code.low & 0xf0) >> 4;
        
        cpu.reg.v_flag = (cpu.reg.v[vy] > cpu.reg.v[vx]) ? 1 : 0;
        cpu.reg.v[vx] = cpu.reg.v[vy] - cpu.reg.v[vx];
    
        if (cpu.debug)
            printf("SUBN V%X, V%X\n", vx, vy);
    }

    // 8XYE, shift left
    static void shl_reg_reg(Cpu &cpu, Opcode code) {
        byte vx = code.high;
        byte vy = (code.low & 0xf0) >> 4;
        
        cpu.reg.v_flag = (cpu.reg.v[vy] & 0x80) ? 1 : 0;
        cpu.reg.v[vx] = cpu.reg.v[vy] << 1;
    
        if (cpu.debug)
            printf("SHL  V%X, V%X\n", vx, vy);
    }

    // 9XY0, skip next instruction if reg[x] != reg[y]
    static void skip_not_eq_reg(Cpu &cpu, Opcode code) {
        byte vx = code.high;
        byte vy = (code.low & 0xf0) >> 4;

        if (cpu.reg.v[vx] != cpu.reg.v[vy])
            cpu.reg.pc += 2;
    
        if (cpu.debug)
            printf("SNE  V%X, V%X\n", vx, vy);
    }

    // ANNN, load address: reg_i = nnn
    static void load_i_addr(Cpu &cpu, Opcode code) {
        cpu.reg.i = code.word;
    
        if (cpu.debug)
            printf("LD   I,  0x%04X\n", code.word);
    }

    // BNNN, jump to address: nnn + reg[0]
    static void jump_relative(Cpu &cpu, Opcode code) {
        cpu.reg.pc = cpu.reg.v[0] + code.word;
    
        if (cpu.debug)
            printf("JP   V0, 0x%04X\n", code.word);
    }

    // CXNN, random: reg[x] = rand() & nn
    static void rand_mask(Cpu &cpu, Opcode code) {
        byte vx = code.high;
        byte value = code.low;

        cpu.reg.v[vx] = (rand() % 0x100) & value;
    
        if (cpu.debug)
            printf("RND  V%X, 0x%04X\n", vx, value);
    }

    // DXYN, draw sprite at (x,y) with n bytes of data
    static void draw_sprite(Cpu &cpu, Opcode code) {
        byte vx = code.high;
        byte vy = (code.low & 0xf0) >> 4;
        byte n = (code.low & 0x0f);
        byte x = cpu.reg.v[vx];
        byte y = cpu.reg.v[vy];

        cpu.reg.v_flag = 0;
        for (byte i = 0; i < n; i++) {
            byte data = cpu.ram[cpu.reg.i + i];
            byte y_coord = y + i;
            if (y_coord >= Cpu::vram_height) {
                continue;
            }

            for (byte j = 0; j < 8; j++) {
                byte x_coord = (x + 8 - j - 1);
                if (x_coord >= Cpu::vram_width) {
                    data >>= 1;
                    continue;
                }

                word pos = Cpu::vram_width * y_coord + x_coord;

                byte bit = data & 0x1;
                if (bit & cpu.vram[pos]) {
                    cpu.reg.v_flag = 1;
                }
                cpu.vram[pos] ^= bit;

                data >>= 1;
            }
        }
        cpu.update_gui = true;
    
        if (cpu.debug)
            printf("DRW  V%X, V%X, 0x%X\n", vx, vy, n);
    }

    // EX9E, skip if keys[reg[x]] pressed
    static void skip_pressed(Cpu &cpu, Opcode code) {
        byte vx = code.high;

        if (cpu.keys[cpu.reg.v[vx]])
            cpu.reg.pc += 2;
    
        if (cpu.debug)
            printf("SKP  V%X\n", vx);
    }

    // EXA1, skip if keys[reg[x]] not pressed
    static void skip_not_pressed(Cpu &cpu, Opcode code) {
        byte vx = code.high;

        if (!cpu.keys[cpu.reg.v[vx]])
            cpu.reg.pc += 2;
    
        if (cpu.debug)
            printf("SKNP V%X\n", vx);
    }

    // FX07, load reg: reg[x] = delay_timer
    static void load_reg_delay(Cpu &cpu, Opcode code) {
        byte vx = code.high;

        cpu.reg.v[vx] = cpu.delay_timer;
    
        if (cpu.debug)
            printf("LD   V%X, DT\n", vx);
    }

    // FX0A, wait key: reg[x] = key
    static void load_wait_key(Cpu &cpu, Opcode code) {
        byte vx = code.high;

        bool has_key = false;
        for (size_t i = 0; i < sizeof(cpu.keys); i++) {
            if (cpu.keys[i]) {
                cpu.reg.v[vx] = i;
                has_key = true;
                break;
            }
        }

        // instead of waiting, just execute same operation to simulate
        if (!has_key) {
            cpu.reg.pc -= 2;
        }
    
        if (cpu.debug)
            printf("LD   V%X, KEY\n", vx);
    }

    // FX15, load delay timer
    static void load_delay_reg(Cpu &cpu, Opcode code) {
        byte vx = code.high;

        cpu.delay_timer = cpu.reg.v[vx];
    
        if (cpu.debug)
            printf("LD   DT, V%X\n", vx);
    }

    // FX18, load sound timer
    static void load_sound_reg(Cpu &cpu, Opcode code) {
        byte vx = code.high;

        cpu.sound_timer = cpu.reg.v[vx];
    
        if (cpu.debug)
            printf("LD   ST, V%X\n", vx);
    }

    // FX1E, add: reg.i += reg[x]
    static void add_i_reg(Cpu &cpu, Opcode code) {
        byte vx = code.high;

        cpu.reg.i += cpu.reg.v[vx];
    
        if (cpu.debug)
            printf("ADD  I,  V%X\n", vx);
    }

    // FX29, load sprite
    static void load_sprite(Cpu &cpu, Opcode code) {
        byte vx = code.high;

        cpu.reg.i = cpu.reg.v[vx] * Cpu::sprint_size;
    
        if (cpu.debug)
            printf("LD   F, 0x%X\n", vx);
    }

    // FX33, store bcd value of reg[x] at reg.i[0:3]
    static void store_bcd(Cpu &cpu, Opcode code) {
        byte vx = code.high;
        byte value = cpu.reg.v[vx];

        cpu.ram[cpu.reg.i] = value / 100;
        cpu.ram[cpu.reg.i + 1] = (value % 100) / 10;
        cpu.ram[cpu.reg.i + 2] = value % 10;
    
        if (cpu.debug)
            printf("LD BCD,  V%X\n", vx);
    }

    // FX55, store register values to [I]
    static void store_regs(Cpu &cpu, Opcode code) {
        byte vx = code.high;

        assert(vx <= sizeof(cpu.reg.v));
        for (int i = 0; i <= vx; i++) {
            cpu.ram[cpu.reg.i++] = cpu.reg.v[i];
        }
    
        if (cpu.debug)
            printf("LD   [I], V%X\n", vx);
    }

    // FX65, load values at [I] to registers
    static void load_regs(Cpu &cpu, Opcode code) {
        byte vx = code.high;

        assert(vx <= sizeof(cpu.reg.v));
        for (int i = 0; i <= vx; i++) {
            cpu.reg.v[i] = cpu.ram[cpu.reg.i++];
        }
    
        if (cpu.debug)
            printf("LD   V%X, [I]\n", vx);
    }
};

#endif
