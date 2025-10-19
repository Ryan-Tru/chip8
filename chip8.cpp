#include <iostream>
#include <cstdint>
#include <array>
#include "raylib.h"

const uint8_t F { 0xF };
const uint16_t NNN { 0x0FFF };
const uint8_t NN { 0x00FF };
const uint8_t N { 0x000F };
const uint16_t REG_X { 0x0F00 };
const uint8_t REG_Y { 0x00F0 };
const int MEMORY_SIZE { 4096 };
const int STACK_SIZE { 16 };
const int REGISTER_SIZE { 16 };
const int KEY_SIZE { 16 };
const int FONTSET_SIZE { 80 };
const int EMPTY_STACK { -1 };

class Chip8 {
        std::array<uint8_t, MEMORY_SIZE> memory;
        uint16_t opcode;

        std::array<uint8_t, REGISTER_SIZE> registers;
        uint16_t indexReg;
        uint16_t progCounter;

        std::array<uint16_t, STACK_SIZE> stack;
        uint8_t stackPointer;

        std::array<uint8_t, KEY_SIZE> keypad;

        std::array<uint8_t, 64 * 32> graphics;

        uint8_t delayTimer;
        uint8_t soundTimer;
    public:
        void initialiseChip8();
};

int main() {
    // init 
    const int screenWidth = 800;
    const int screenHeight = 400;
    InitWindow(screenWidth, screenHeight, "chip8");
    Chip8 chip8;
    chip8.initialiseChip8();

    // run
    while (!WindowShouldClose()) {
        // Draw
        BeginDrawing();
        ClearBackground(BLACK);
        EndDrawing();
    }

    // close window
    CloseWindow();

    return 0;
}

void Chip8::initialiseChip8() {
    progCounter = 0x200;
    opcode = 0x0;
    indexReg = 0x0;
    stackPointer = EMPTY_STACK;
    uint8_t fontset[FONTSET_SIZE] = {
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

    // initialise memory
    memset(&memory, 0, MEMORY_SIZE);

    // init stack
    memset(&stack, 0, STACK_SIZE);

    // init registers
    memset(&registers, 0, REGISTER_SIZE);

    // init fontset
    for (int i = 0; i < FONTSET_SIZE; i++) {
        memory[i + 0x050] = fontset[i];
    }

    // init graphics
    std::memset(&graphics, 0, sizeof(graphics));

    // reset timers
    delayTimer = 0;
    soundTimer = 0;
}
