#include "cpu.h"
#include "gui.h"
#include <iostream>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << "Usage: chip8 ROM" << std::endl;
        return 0;
    }

    Cpu cpu;
    cpu.load_program(argv[1]);
    cpu.set_debug(false);

    Gui gui(Cpu::vram_width, Cpu::vram_height, 8);

    while (true) {
        if (gui.should_quit()) {
            return 0;
        }

        cpu.cycle(gui.get_ticks());

        gui.update_screen(cpu.get_vram());
        gui.update_keys(cpu.get_keys());
    }

    return 0;
}
