#include "cpu.h"
#include "opcode.h"
#include <fstream>
#include <algorithm>
#include <stdexcept>

/// start address of program (pc)
static constexpr size_t prog_start = 0x200;
/// max program size
static constexpr size_t max_prog_size = (Cpu::mem_size - prog_start);

/// cpu frequency
static constexpr size_t cpu_frequency = 600;
/// timer frequency
static constexpr size_t timer_frequency = 60;

/// cpu time out
static constexpr float cpu_time_out = 1000.0 / float(cpu_frequency);
/// timer time out
static constexpr float timer_time_out = 1000.0 / float(timer_frequency);

/// font sprite data ('0' - 'F')
static uint8_t HEX_FONTS[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void Cpu::load_program(const char *file) {
    std::ifstream stream(file, std::ios::binary);
    
    if (!stream) {
        throw std::runtime_error("could not open file");
    }

    reset();

    stream.read(reinterpret_cast<char*>(ram + prog_start), max_prog_size);
}

void Cpu::cycle(uint32_t now) {
    if ((now - last_timer_ticks) > timer_time_out) {
        if (delay_timer > 0)
            delay_timer--;
        if (sound_timer > 0)
            sound_timer--;

        last_timer_ticks = now;
    }

    if ((now - last_cpu_ticks) > cpu_time_out) {
        word code = fetch();

        update_gui = false;
        interpret(code);

        if (debug) {
            dump_registers();
        }

        last_cpu_ticks = now;
    }
}

void Cpu::reset() {
    // clear memory and registers
    std::fill_n(ram, sizeof(ram), 0);
    std::fill_n(vram, sizeof(vram), 0);
    std::fill_n(stack, sizeof(stack), 0);
    std::fill_n(keys, sizeof(keys), 0);

    reg = (const Register) {};
    reg.pc = prog_start;

    // reload fonts
    std::copy_n(HEX_FONTS, sizeof(HEX_FONTS), ram);
}

word Cpu::fetch() {
    word high = ram[reg.pc++] << 8;
    word low = ram[reg.pc++];
    return high | low; 
}

void Cpu::interpret(word opcode) {
    byte type = (opcode & 0xf000) >> 12;
    Opcode code = { word(opcode & 0x0fff) };

    switch (type) {
        case 0x00: {
            if (code.low == 0xe0)
                Operations::cls(*this);
            else if (code.low == 0xee)
                Operations::ret(*this);
            // ignore others
        } break;

        case 0x01: Operations::jump(*this, code); break;
        case 0x02: Operations::call(*this, code); break;
        case 0x03: Operations::skip_eq(*this, code); break;
        case 0x04: Operations::skip_not_eq(*this, code); break;

        case 0x05: {
            if (code.low & 0x0f)
                throw std::runtime_error("invalid opcode: 5XY0");
            Operations::skip_eq_reg(*this, code);
        } break;

        case 0x06: Operations::load_reg_value(*this, code); break;
        case 0x07: Operations::add_reg_value(*this, code); break;

        case 0x08: {
            switch (code.low & 0x0f) {
                case 0x00: Operations::load_reg_reg(*this, code); break;
                case 0x01: Operations::or_reg_reg(*this, code); break;
                case 0x02: Operations::and_reg_reg(*this, code); break;
                case 0x03: Operations::xor_reg_reg(*this, code); break;
                case 0x04: Operations::add_reg_reg(*this, code); break;
                case 0x05: Operations::sub_reg_reg(*this, code); break;
                case 0x06: Operations::shr_reg_reg(*this, code); break;
                case 0x07: Operations::subn_reg_reg(*this, code); break;
                case 0x0e: Operations::shl_reg_reg(*this, code); break;
                default: throw std::runtime_error("invalid opcode: 8XYn");
            }
        } break;

        case 0x09: Operations::skip_not_eq_reg(*this, code); break;
        case 0x0a: Operations::load_i_addr(*this, code); break;
        case 0x0b: Operations::jump_relative(*this, code); break;
        case 0x0c: Operations::rand_mask(*this, code); break;
        case 0x0d: Operations::draw_sprite(*this, code); break;

        case 0x0e: {
            if (code.low == 0x9e)
                Operations::skip_pressed(*this, code);
            else if (code.low == 0xa1)
                Operations::skip_not_pressed(*this, code);
            else
                throw std::runtime_error("invalid opcode: EXnn");
        } break;

        case 0x0f: {
            switch (code.low) {
                case 0x07: Operations::load_reg_delay(*this, code); break;
                case 0x0A: Operations::load_wait_key(*this, code); break;
                case 0x15: Operations::load_delay_reg(*this, code); break;
                case 0x18: Operations::load_sound_reg(*this, code); break;
                case 0x1E: Operations::add_i_reg(*this, code); break;
                case 0x29: Operations::load_sprite(*this, code); break;
                case 0x33: Operations::store_bcd(*this, code); break;
                case 0x55: Operations::store_regs(*this, code); break;
                case 0x65: Operations::load_regs(*this, code); break;
                default: throw std::runtime_error("invalid opcode: FXnn");
            }
        } break;

        default: throw std::runtime_error("impossible!!!");
    }
}

#include <cstdio>

void Cpu::dump_registers() {
    for (size_t i = 0; i < sizeof(reg.v); i++) {
        printf("V%llX: %02X\t", i, reg.v[i]);
        if (i == sizeof(reg.v) / 2 - 1) 
            printf("\n");
    }
    printf("\nI: %04X    SP: %04x    PC: %04x    DT: %04x    ST: %04x\n\n",
        reg.i, reg.sp, reg.pc, delay_timer, sound_timer);
}
