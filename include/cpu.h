#ifndef CHIP8_CPU_H
#define CHIP8_CPU_H

#include "common.h"

class Cpu {
public:
    /// memory size
    static constexpr size_t mem_size = 4096;
    /// stack size
    static constexpr size_t stack_size = 16;
    /// video ram width
    static constexpr size_t vram_width = 64;
    /// video ram height
    static constexpr size_t vram_height = 32;
    /// video ram size
    static constexpr size_t vram_size = (vram_width * vram_height);
    /// number of keys
    static constexpr size_t key_size = 16;
    /// font sprite size
    static constexpr size_t sprint_size = 5;

private:
    /// cpu register struct
    struct Register {
        /// program counter
        word pc;
        /// stack pointer
        byte sp;

        /// 16 bytes v-registers, last one is flag register
        union {
            byte v[16];
            struct {
                byte __[15];
                byte v_flag;
            };
        };

        /// register I
        word i;
    };

    /// cpu register
    Register reg;

    /// main memory
    byte ram[mem_size];
    /// video memory
    byte vram[vram_size];
    /// stack
    word stack[stack_size];
    /// keyboard
    bool keys[key_size];

    /// delay timer
    byte delay_timer;
    /// sound timer
    byte sound_timer;

    /// flag indicating gui update
    bool update_gui;
    /// debug flag
    bool debug;

    /// cpu ticks
    uint32_t last_cpu_ticks;
    /// timer ticks
    uint32_t last_timer_ticks;

    /// fetch opcode
    word fetch();
    /// print registers
    void dump_registers();

    friend class Operations;

public:
    /// run one cycle
    void cycle(uint32_t cycles);
    /// interrupt opcode
    void interpret(word opcode);
    /// reset register and memory
    void reset();
    
    /// load program from file
    void load_program(const char *file);
    /// set debug mode (print internal state)
    void set_debug(bool debug) { this->debug = debug; }

    /// get video buffer
    byte* get_vram() { return this->vram; }
    /// get key buffer
    bool* get_keys() { return this->keys; }
};

#endif
