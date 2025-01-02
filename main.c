/*******************************************************************************************
*
*   raylib [core] example - Basic 3d example
*
*   Welcome to raylib!
*
*   To compile example, just press F5.
*   Note that compiled executable is placed in the same folder as .c file
*
*   You can find all basic examples on C:\raylib\raylib\examples folder or
*   raylib official webpage: www.raylib.com
*
*   Enjoy using raylib. :)
*
*   This example has been created using raylib 1.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2013-2023 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "sprite.h"
#include "z80.h"


byte vram[768];
long long int totalcycles = 0;
int counter = 0;
int curCycles;
byte lastOp = 0;

int drawScreen(){
    BeginDrawing();
    ClearBackground(WHITE);
    for (int i = 0; i < 24; i++){
        for (int j = 0; j < 32; j++){
            if (vram[32 * i + j] % 128 < 64){
                drawSprite(vram[32 * i + j] % 64, vram[32 * i + j] > 127, 1, 1, 8 * j, 8 * i);
            } else {
                drawSprite(0, 0, 1, 1, 8 * j, 8 * i);

            }
        }
    } 
    //DrawText(TextFormat("PC: %.04X %.02X %.02X %.02X %.02X\n%.04X %d\nA: %.02X H: %.02X L: %.02X\n%ld", pc, read(mem, pc), read(mem, pc + 1), read(mem, pc + 2), read(mem, pc + 3), pc, counter, regs[0], regs[5], regs[6], intMode), 10, 10, 20, BLACK);
    EndDrawing();
    return 0;
}

int clearScreen(){
    for (int i = 0; i < 768; i++){
        vram[i] = 0;
    }
    return 0;
}

int readDfile(){
    clearScreen();
    dbyte low = read16(mem, 16396);
    dbyte high = read16(mem, 16400);
    int x = 0;
    int y = -1;
    dbyte cur = low;
    while (y != 24 && cur < high && cur != 0xFFFF){
        if (read(mem, cur) == 118){
            y++;
            x = 0;
        } else if (read(mem, cur) % 128 < 64){
            if (y < 0 || y > 23 || x < 0 || x > 32){
                clearScreen();
                return -1;
            }
            vram[y * 32 + x] = read(mem, cur);
            x++;
        } else {
            vram[y * 32 + x] = 0;
            x++;
        }
        cur++;
    }
    return 0;
}

int readKeyboard(byte num){
    byte fe = 0;
    switch (num){
        case 0xFE:
            if (!IsKeyDown(KEY_LEFT_SHIFT)){
                fe += 1;
            }
            if (!IsKeyDown(KEY_Z)){
                fe += 2;
            }
            if (!IsKeyDown(KEY_X)){
                fe += 4;
            }
            if (!IsKeyDown(KEY_C)){
                fe += 8;
            }
            if (!IsKeyDown(KEY_V)){
                fe += 16;
            }
            break;
        case 0xFD:
            if (!IsKeyDown(KEY_A)){
                fe += 1;
            }
            if (!IsKeyDown(KEY_S)){
                fe += 2;
            }
            if (!IsKeyDown(KEY_D)){
                fe += 4;
            }
            if (!IsKeyDown(KEY_F)){
                fe += 8;
            }
            if (!IsKeyDown(KEY_G)){
                fe += 16;
            }
            break;
        case 0xFB:
            if (!IsKeyDown(KEY_Q)){
                fe += 1;
            }
            if (!IsKeyDown(KEY_W)){
                fe += 2;
            }
            if (!IsKeyDown(KEY_E)){
                fe += 4;
            }
            if (!IsKeyDown(KEY_R)){
                fe += 8;
            }
            if (!IsKeyDown(KEY_T)){
                fe += 16;
            }
            break;
        case 0xF7:
            if (!IsKeyDown(KEY_ONE)){
                fe += 1;
            }
            if (!IsKeyDown(KEY_TWO)){
                fe += 2;
            }
            if (!IsKeyDown(KEY_THREE)){
                fe += 4;
            }
            if (!IsKeyDown(KEY_FOUR)){
                fe += 8;
            }
            if (!IsKeyDown(KEY_FIVE)){
                fe += 16;
            }
            break;
        case 0xEF:
            if (!IsKeyDown(KEY_ZERO)){
                fe += 1;
            }
            if (!IsKeyDown(KEY_NINE)){
                fe += 2;
            }
            if (!IsKeyDown(KEY_EIGHT)){
                fe += 4;
            }
            if (!IsKeyDown(KEY_SEVEN)){
                fe += 8;
            }
            if (!IsKeyDown(KEY_SIX)){
                fe += 16;
            }
            break;
        case 0xDF:
            if (!IsKeyDown(KEY_P)){
                fe += 1;
            }
            if (!IsKeyDown(KEY_O)){
                fe += 2;
            }
            if (!IsKeyDown(KEY_I)){
                fe += 4;
            }
            if (!IsKeyDown(KEY_U)){
                fe += 8;
            }
            if (!IsKeyDown(KEY_Y)){
                fe += 16;
            }
            break;
        case 0xBF:
            if (!IsKeyDown(KEY_ENTER)){
                fe += 1;
            }
            if (!IsKeyDown(KEY_L)){
                fe += 2;
            }
            if (!IsKeyDown(KEY_K)){
                fe += 4;
            }
            if (!IsKeyDown(KEY_J)){
                fe += 8;
            }
            if (!IsKeyDown(KEY_H)){
                fe += 16;
            }
            break;
        case 0x7F:
            if (!IsKeyDown(KEY_SPACE)){
                fe += 1;
            }
            if (!IsKeyDown(KEY_PERIOD)){
                fe += 2;
            }
            if (!IsKeyDown(KEY_M)){
                fe += 4;
            }
            if (!IsKeyDown(KEY_N)){
                fe += 8;
            }
            if (!IsKeyDown(KEY_B)){
                fe += 16;
            }
            break;
    }
    ioIN[0xFE] = fe + 64;
    return 0;
}

int format1K(){
    if (mem != NULL){
        freeMemory(mem);
        mem = NULL;
    }
    mem = (Memory*)malloc(sizeof(Memory));
    mem->head = NULL;
    addBankList(mem, 0, 0x1FFF);
    addBankList(mem, 0x4000, 0x43FF);
    addBankAt(mem, 0, 0x1FFF, 'r');
    addBankAt(mem, 0x4000, 0x43FF, 'w');
    byte* rom = (byte*)malloc(8192);
    FILE* file = fopen("zx81basic.rom", "rb");
    if (file == NULL) return -1;
    fread(rom, 1, 8192, file);
    blockSet(rom, 8192, mem->head, 0);
    return 0;
}

int format16K(){
    if (mem != NULL){
        freeMemory(mem);
        mem = NULL;
    }
    mem = (Memory*)malloc(sizeof(Memory));
    mem->head = NULL;
    addBankList(mem, 0, 0x1FFF);
    addBankList(mem, 0x4000, 0x7FFF);
    addBankAt(mem, 0, 0x1FFF, 'r');
    addBankAt(mem, 0x4000, 0x7FFF, 'w');
    byte* rom = (byte*)malloc(8192);
    FILE* file = fopen("zx81basic.rom", "rb");
    if (file == NULL) return -1;
    fread(rom, 1, 8192, file);
    blockSet(rom, 8192, mem->head, 0);
    return 0;
}

int main(){
    InitWindow(screen_width, screen_height, "ZX81");
    loadAssets();
    resizeScreen(2);
    int frameRate = 50;
    SetTargetFPS(frameRate);
    if (format1K() == -1) return 0;
    int clockSpeed = 3250000;
    double cycleTime = 1.0 / clockSpeed;
    double refresh = 1.0 / frameRate;
    double curTime = GetTime();
    byte drawError = 0;
    FILE* file;
    double nmiTime;
    byte nmiGen = 0;
    int cycles = 0;
    while (!WindowShouldClose() && !drawError){
        curCycles = 0;
        while (GetTime() - curTime < refresh){
            nmiTime = GetTime();
            //file = fopen("debug.bin", "wb");
            //fwrite(&pc, 1, 2, file);
            //fclose(file);
            if (pc > 0x7FFF){
                readFromMem = 0;
                if (read(mem, pc - 0x8000) == 0x76){
                    altByte = 0x76;
                } else {
                    altByte = 0;
                }
            } else readFromMem = 1;
            if (re % 128 == 0){
                interrupt(0);
                iff1 = 0;
                iff2 = 0;
            }
            if (read16(mem, pc) == 0xFED3) nmiGen = 1;
            else if (read16(mem, pc) == 0xFDD3) nmiGen = 0;
            if (read16(mem, pc) == 0x78ED) readKeyboard(readR(rhash('b')));
            else if (read(mem, pc) == 0xDB) readKeyboard(readR(rhash('a')));
            curCycles += runcmd();
            if (pc == 0x2E6) counter++;
            while (GetTime() - curTime < curCycles * cycleTime) cycles++;
            if (GetTime() - nmiTime > 64e-6){
                if (nmiGen) nmi();
                nmiTime = GetTime();
            }
        }
        readDfile();
        drawScreen();
        curTime = GetTime();
    }
    freeAssets();
    if (drawError){
        char error = '1';
        file = fopen("log.txt", "wb");
        fwrite(&error, 1, 1, file);
        fclose(file);
    } else if (halt){
        char error = '2';
        file = fopen("log.txt", "wb");
        fwrite(&error, 1, 1, file);
        fclose(file);
    }
    return 0;
}