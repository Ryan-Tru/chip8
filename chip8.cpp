#include <iostream>
#include <cstdint>
#include <array>

const uint8_t F { 0xF };
const uint8_t NNN { 0x0FFF };
const uint8_t NN { 0x00FF };
const uint8_t N { 0x000F };
const uint8_t REG_X { 0x0F00 };
const uint8_t REG_Y { 0x00F0 };
const int MEMORY_SIZE { 4096 };
const int STACK_SIZE { 16 };
const int REGISTER_SIZE { 16 };
const int KEY_SIZE { 16 };
const int FONTSET_SIZE { 80 };
const int EMPTY_STACK { -1 };

class Chip8 {
    std::array<uint8_t, MEMORY_SIZE> memory;
    std::array<uint8_t, REGISTER_SIZE> registers;
    uint16_t indexReg;
    uint16_t progCounter;
    std::array<uint16_t, STACK_SIZE> stack;
    uint8_t stackPointer;
    std::array<uint8_t, KEY_SIZE> keypad;
};

int main() {

    return 0;
}
