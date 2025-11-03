#include <array>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <thread>

#include "raylib.h"

const uint8_t F{0xF};
const uint16_t NNN{0x0FFF};
const uint8_t NN{0x00FF};
const uint8_t N{0x000F};
const uint16_t REG_X{0x0F00};
const uint8_t REG_Y{0x00F0};
const int MEMORY_SIZE{4096};
const int STACK_SIZE{16};
const int REGISTER_SIZE{16};
const int KEY_SIZE{16};
const int FONTSET_SIZE{80};
const uint8_t FONTSET_START_ADDR{0x050};
const int EMPTY_STACK{-1};
const uint8_t RANDOM_MAX{255};
const uint8_t RANDOM_MIN{0};
const int CHIP8_SCREEN_WIDTH{64};
const int CHIP8_SCREEN_HEIGHT{32};

class Chip8 {
  std::array<uint8_t, MEMORY_SIZE> memory;
  uint16_t opcode;

  std::array<uint8_t, REGISTER_SIZE> registers;
  uint16_t indexReg;
  uint16_t progCounter;

  std::array<uint16_t, STACK_SIZE> stack;
  int stackPointer;

  std::array<uint8_t, KEY_SIZE> keypad;

  std::array<uint8_t, 64 * 32> graphics;

  uint8_t delayTimer;
  uint8_t soundTimer;

  bool drawFlag;

 public:
  // Chip8 operations
  void initialiseChip8();
  void fetchOpcode();
  void loadROM(char* filepath);
  void decodeExecute();

  // Stack stuff
  void pushToStack();
  void popFromStack();

  // Drawing
  bool isDrawFlagTrue();
  void resetDrawFlag();
  void renderPixels();

  // Key Stuff
  void processKeyDown();
  void processKeyUp();
};

int main(int argc, char** argv) {
  if (argc != 3) {
    fprintf(stderr, "usage: chip8_emu <ROM> <step>");
    exit(1);
  }

  // init
  const float screenWidth = 640;
  const float screenHeight = 320;
  const int step = atoi(argv[2]);
  InitWindow(screenWidth, screenHeight, "chip8");
  Chip8 chip8;
  chip8.initialiseChip8();
  chip8.loadROM(argv[1]);

  RenderTexture2D target =
      LoadRenderTexture(CHIP8_SCREEN_WIDTH, CHIP8_SCREEN_HEIGHT);

  // run
  while (!WindowShouldClose()) {
    if (step == 1) {
      std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    chip8.processKeyUp();
    chip8.processKeyDown();

    chip8.fetchOpcode();
    chip8.decodeExecute();
    // Draw
    BeginDrawing();
    if (chip8.isDrawFlagTrue()) {
      BeginTextureMode(target);
      ClearBackground(BLACK);
      chip8.renderPixels();
      EndTextureMode();

      DrawTexturePro(
          target.texture,
          (Rectangle){0.0f, 0.0f, (float)target.texture.width,
                      (float)-target.texture.height},
          (Rectangle){0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
          (Vector2){0, 0}, 0.0f, WHITE);
    }
    EndDrawing();
  }

  UnloadRenderTexture(target);
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
      0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
      0x20, 0x60, 0x20, 0x20, 0x70,  // 1
      0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
      0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
      0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
      0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
      0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
      0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
      0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
      0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
      0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
      0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
      0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
      0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
      0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
      0xF0, 0x80, 0xF0, 0x80, 0x80   // F
  };

  // initialise memory
  std::memset(&memory, 0, MEMORY_SIZE);

  // init stack
  std::memset(&stack, 0, STACK_SIZE);

  // init registers
  std::memset(&registers, 0, REGISTER_SIZE);

  // init fontset
  for (int i = 0; i < FONTSET_SIZE; i++) {
    memory[i + FONTSET_START_ADDR] = fontset[i];
  }

  // init graphics
  std::memset(&graphics, 0, sizeof(graphics));

  // reset timers
  delayTimer = 0;
  soundTimer = 0;
}

void Chip8::fetchOpcode() {
  // opcodes are 4 bytes but progCounter only stores 2 bytes, so left shift 8
  // bits then or the result
  opcode = memory[progCounter] << 8 | memory[progCounter + 1];
  progCounter += 2;
}

void Chip8::loadROM(char* filepath) {
  std::FILE* ROM = std::fopen(filepath, "rb");

  for (int i = 0x200; i < MEMORY_SIZE; i++) {
    fread(&memory[i], sizeof(memory[i]), 1, ROM);
  }

  std::fclose(ROM);
}

void Chip8::decodeExecute() {
  switch (opcode & 0xF000) {
    case 0x0000:
      switch (opcode) {
        case 0x00E0:
          ClearBackground(BLACK);
          std::memset(&graphics, 0, sizeof(graphics));
          drawFlag = true;
          break;
        case 0x00EE:
          popFromStack();
          break;
      }
      break;
    // 1NNN
    case 0x1000:
      progCounter = opcode & NNN;
      break;
    // 2NNN
    case 0x2000:
      pushToStack();
      progCounter = opcode & NNN;
      break;
    // 3XNN
    case 0x3000:
      if (registers[(opcode & REG_X) >> 8] == (uint8_t)(opcode & NN)) {
        progCounter += 2;
      }
      break;
    // 4XNN
    case 0x4000:
      if (registers[(opcode & REG_X) >> 8] != (opcode & NN)) {
        progCounter += 2;
      }
      break;
    // 5XY0
    case 0x5000:
      if (registers[(opcode & REG_X) >> 8] ==
          registers[(opcode & REG_Y) >> 4]) {
        progCounter += 2;
      }
      break;
      // 6XNN
    case 0x6000:
      registers[(opcode & REG_X) >> 8] = opcode & NN;
      break;
      // 7XNN
    case 0x7000:
      registers[(opcode & REG_X) >> 8] += opcode & NN;
      break;
      // 8XY_
    case 0x8000:
      switch (opcode & 0x000F) {
        case 0x0000:
          registers[(opcode & REG_X) >> 8] = registers[(opcode & REG_Y) >> 4];
          break;
        case 0x0001:
          registers[(opcode & REG_X) >> 8] |= registers[(opcode & REG_Y) >> 4];
          break;
        case 0x0002:
          registers[(opcode & REG_X) >> 8] &= registers[(opcode & REG_Y) >> 4];
          break;
        case 0x0003:
          registers[(opcode & REG_X) >> 8] ^= registers[(opcode & REG_Y) >> 4];
          break;
        case 0x0004:
          if ((registers[(opcode & REG_X) >> 8] +
               registers[(opcode & REG_Y) >> 4]) > 255) {
            registers[F] = 1;
          } else {
            registers[F] = 0;
          }
          registers[(opcode & REG_X) >> 8] = registers[(opcode & REG_X) >> 8] +
                                             registers[(opcode & REG_Y) >> 4];
          break;
        case 0x0005:
          if (registers[(opcode & REG_X) >> 8] >
              registers[(opcode & REG_Y) >> 4]) {
            registers[F] = 1;
          }

          registers[(opcode & REG_X) >> 8] -= registers[(opcode & REG_Y) >> 4];
          break;
        case 0x0006:
          if (((registers[(opcode & REG_X) >> 8]) & 0x1) == 0x1) {
            registers[F] = 1;
          } else {
            registers[F] = 0;
          }
          registers[(opcode & REG_X) >> 8] /= 2;
          break;
        case 0x0007:
          if (registers[(opcode & REG_Y) >> 4] >
              registers[(opcode & REG_X) >> 8]) {
            registers[F] = 1;
          } else {
            registers[F] = 0;
          }

          registers[(opcode & REG_X) >> 8] = registers[(opcode & REG_Y) >> 4] -
                                             registers[(opcode & REG_X) >> 8];
          break;
        case 0x000E:
          if ((((registers[(opcode & REG_X) >> 8]) >> 3) & 0x1) == 0x1) {
            registers[F] = 1;
          } else {
            registers[F] = 0;
          }

          registers[(opcode & REG_X) >> 8] *= 2;
          break;
      }
      // 9XY0
    case 0x9000:
      if (registers[(opcode & REG_X) >> 8] !=
          registers[(opcode & REG_Y) >> 4]) {
        progCounter += 2;
      }

      break;
      // ANNN
    case 0xA000:
      indexReg = opcode & NNN;
      break;
      // BNNN
    case 0xB000:
      progCounter = opcode & NNN + registers[0];
      break;
      // CXNN
    case 0xC000: {
      int randomNum = rand() % (RANDOM_MAX - RANDOM_MIN + 1) + RANDOM_MIN;
      int nn = opcode & NN;

      registers[(opcode & REG_X) >> 8] = randomNum & nn;
      break;
    }
      // DXYN
    case 0xD000: {
      int x = registers[(opcode & REG_X) >> 8];
      int y = registers[(opcode & REG_Y) >> 4];
      uint8_t height = opcode & N;
      uint8_t pixel;

      registers[F] = 0;
      for (int row = 0; row < height; row++) {
        pixel = memory[indexReg + row];
        for (int col = 0; col < 8; col++) {
          int position = (x + col + ((y + row) * CHIP8_SCREEN_WIDTH));
          if ((pixel & (0x80 >> col)) != 0) {
            if (graphics[position] == 1) {
              registers[F] = 1;
            }
            graphics[position] ^= 1;
          }
        }
      }

      drawFlag = true;
      break;
    }
      // EX9E
    case 0xE000: {
      switch (opcode & 0x000F) {
        case 0x000E: {
          if (keypad[(opcode & REG_X >> 8)]) {
            progCounter += 2;
          }
          break;
        }
        case 0x0001: {
          if (!keypad[(opcode & REG_X >> 8)]) {
            progCounter += 2;
          }
          break;
        }
      }
    }

    case 0xF000: {
      switch (opcode & 0x00FF) {
        case 0x0007:
          registers[(opcode & REG_X) >> 8] = delayTimer;
          break;
        case 0x0015:
          delayTimer = registers[(opcode & REG_X) >> 8];
          break;
        case 0x0018:
          soundTimer = registers[(opcode & REG_X) >> 8];
          break;
        case 0x001E:
          indexReg += registers[(opcode & REG_X) >> 8];
          break;
        case 0x000A:
          progCounter -= 2;
          if (keypad[0]) {
            registers[(opcode & REG_X) >> 8] = 0;
          } else if (keypad[1]) {
            registers[(opcode & REG_X) >> 8] = 1;
          } else if (keypad[2]) {
            registers[(opcode & REG_X) >> 8] = 2;
          } else if (keypad[3]) {
            registers[(opcode & REG_X) >> 8] = 3;
          } else if (keypad[4]) {
            registers[(opcode & REG_X) >> 8] = 4;
          } else if (keypad[5]) {
            registers[(opcode & REG_X) >> 8] = 5;
          } else if (keypad[6]) {
            registers[(opcode & REG_X) >> 8] = 6;
          } else if (keypad[7]) {
            registers[(opcode & REG_X) >> 8] = 7;
          } else if (keypad[8]) {
            registers[(opcode & REG_X) >> 8] = 8;
          } else if (keypad[9]) {
            registers[(opcode & REG_X) >> 8] = 9;
          } else if (keypad[0xA]) {
            registers[(opcode & REG_X) >> 8] = 0xA;
          } else if (keypad[0xB]) {
            registers[(opcode & REG_X) >> 8] = 0xB;
          } else if (keypad[0xC]) {
            registers[(opcode & REG_X) >> 8] = 0xC;
          } else if (keypad[0xD]) {
            registers[(opcode & REG_X) >> 8] = 0xD;
          } else if (keypad[0xE]) {
            registers[(opcode & REG_X) >> 8] = 0xE;
          } else if (keypad[0xF]) {
            registers[(opcode & REG_X) >> 8] = 0xF;
          }

          break;
        case 0x0029:
          indexReg =
              FONTSET_START_ADDR + (5 * registers[((opcode & REG_X) >> 8)]);
          break;
        case 0x0033: {
          int value = registers[((opcode & REG_X) >> 8)];
          int onesPlace = value % 10;
          int tensPlace = (value / 10) % 10;
          int hundredsPlace = value / 100;

          memory[indexReg] = hundredsPlace;
          memory[indexReg + 1] = tensPlace;
          memory[indexReg + 2] = onesPlace;

          break;
        }
        case 0x0055: {
          int x = (opcode & REG_X) >> 8;
          for (uint8_t i = 0; i < x; i++) {
            memory[indexReg + i] = registers[i];
          }
          break;
        }
        case 0x0065: {
          int x = (opcode & REG_X) >> 8;
          for (uint8_t i = 0; i < x; i++) {
            registers[i] = memory[indexReg + i];
          }
          break;
        }
      }
      break;
    }
    default:
      fprintf(stderr, "ERR: UNKNOWN INSTRUCTION");
      exit(1);
  }
}

////////////////////////////////////////
// STACK STUFF
////////////////////////////////////////

void Chip8::pushToStack() {
  if (!(stackPointer == STACK_SIZE)) {
    stackPointer++;
    stack[stackPointer] = progCounter;
    return;
  } else {
    fprintf(stderr, "ERR: STACK OVERFLOW!\n");
  }
}

void Chip8::popFromStack() {
  if (!(stackPointer == EMPTY_STACK)) {
    progCounter = stack[stackPointer];
    stackPointer--;
    return;
  } else {
    fprintf(stderr, "ERR: EMPTY STACK COULD NOT POP\n");
  }
}

////////////////////////////////////////
// GRAPHICS STUFF
////////////////////////////////////////

bool Chip8::isDrawFlagTrue() { return drawFlag; }

void Chip8::resetDrawFlag() { drawFlag = false; }

void Chip8::renderPixels() {
  for (int i = 0; i < graphics.size(); i++) {
    int x = i % CHIP8_SCREEN_WIDTH;
    int y = i / CHIP8_SCREEN_WIDTH;
    if (graphics[i] == 1) {
      DrawPixel(x, y, WHITE);
    }
  }
}

////////////////////////////////////////
// KEY STUFF
////////////////////////////////////////

void Chip8::processKeyDown() {
  if (IsKeyDown(KEY_ONE)) {
    keypad[0] = 1;
  } else if (IsKeyDown(KEY_TWO)) {
    keypad[1] = 1;
  } else if (IsKeyDown(KEY_THREE)) {
    keypad[2] = 1;
  } else if (IsKeyDown(KEY_FOUR)) {
    keypad[3] = 1;
  } else if (IsKeyDown(KEY_Q)) {
    keypad[4] = 1;
  } else if (IsKeyDown(KEY_W)) {
    keypad[5] = 1;
  } else if (IsKeyDown(KEY_E)) {
    keypad[6] = 1;
  } else if (IsKeyDown(KEY_R)) {
    keypad[7] = 1;
  } else if (IsKeyDown(KEY_A)) {
    keypad[8] = 1;
  } else if (IsKeyDown(KEY_S)) {
    keypad[9] = 1;
  } else if (IsKeyDown(KEY_D)) {
    keypad[0xA] = 1;
  } else if (IsKeyDown(KEY_F)) {
    keypad[0xB] = 1;
  } else if (IsKeyDown(KEY_Z)) {
    keypad[0xC] = 1;
  } else if (IsKeyDown(KEY_X)) {
    keypad[0xD] = 1;
  } else if (IsKeyDown(KEY_C)) {
    keypad[0xE] = 1;
  } else if (IsKeyDown(KEY_V)) {
    keypad[0xF] = 1;
  }
}

void Chip8::processKeyUp() {
  if (IsKeyReleased(KEY_ONE)) {
    keypad[0] = 0;
  } else if (IsKeyReleased(KEY_TWO)) {
    keypad[1] = 0;
  } else if (IsKeyReleased(KEY_THREE)) {
    keypad[2] = 0;
  } else if (IsKeyReleased(KEY_FOUR)) {
    keypad[3] = 0;
  } else if (IsKeyReleased(KEY_Q)) {
    keypad[4] = 0;
  } else if (IsKeyReleased(KEY_W)) {
    keypad[5] = 0;
  } else if (IsKeyReleased(KEY_E)) {
    keypad[6] = 0;
  } else if (IsKeyReleased(KEY_R)) {
    keypad[7] = 0;
  } else if (IsKeyReleased(KEY_A)) {
    keypad[8] = 0;
  } else if (IsKeyReleased(KEY_S)) {
    keypad[9] = 0;
  } else if (IsKeyReleased(KEY_D)) {
    keypad[0xA] = 0;
  } else if (IsKeyReleased(KEY_F)) {
    keypad[0xB] = 0;
  } else if (IsKeyReleased(KEY_Z)) {
    keypad[0xC] = 0;
  } else if (IsKeyReleased(KEY_X)) {
    keypad[0xD] = 0;
  } else if (IsKeyReleased(KEY_C)) {
    keypad[0xE] = 0;
  } else if (IsKeyReleased(KEY_V)) {
    keypad[0xF] = 0;
  }
}
