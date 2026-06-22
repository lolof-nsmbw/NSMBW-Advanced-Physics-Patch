#include <common.h>

extern "C" {

static const u32 cancelCarryAddr = 0x8012E510;
static const u32 actorSearchAddr = 0x80162D00;
static const u32 STATE_SLIDE = 0x80353A64; 

static const int OFF_PLAYER_CARRYID = 0x2A78;
static const int OFF_PLAYER_POSE    = 0x2BB0;

static const int OFF_ACTOR_PROFILE  = 0x8;
static const int OFF_ACTOR_SPEED_X  = 0xE8;
static const int OFF_ACTOR_SPEED_Y  = 0xEC;

class dActor_c;
typedef void (*tCancelCarry)(void* player, void* actor);

static inline dActor_c* lookupActorById(u32 id) {
    return ((dActor_c* (*)(u32))actorSearchAddr)(id);
}

int CheckCustomDrop(void* player) {
    u32* checkAddr     = (u32*)0x809CBCA4; 
    u32* bombPatchAddr = (u32*)0x809CBCA8; 
    u32* glowPatchAddr = (u32*)0x808566A8; 
    u32* propellerAddr = (u32*)0x80894FAC;
    u32* powPatchAddr = (u32*)0x80892890;
    u32* springAddr = (u32*)0x80A3A25C;

    if (*checkAddr == 0x981E0175 && *bombPatchAddr != 0x4b636c99) {
        *bombPatchAddr = 0x4b636c99; 
        *glowPatchAddr = 0x4b7ac329; 
        *propellerAddr = 0x4B76E254; 
        *powPatchAddr = 0x4B7709F0; 
        *springAddr = 0x4B5C90BC; 
        asm volatile (
            "dcbst 0, %0\n\t"
            "sync\n\t"
            "icbi 0, %0\n\t"
            "isync\n\t"
            : : "r" (bombPatchAddr)
        );
        asm volatile (
            "dcbst 0, %0\n\t"
            "sync\n\t"
            "icbi 0, %0\n\t"
            "isync\n\t"
            : : "r" (glowPatchAddr)
        );
        asm volatile (
            "dcbst 0, %0\n\t"
            "sync\n\t"
            "icbi 0, %0\n\t"
            "isync\n\t"
            : : "r" (propellerAddr)
        );
        asm volatile (
            "dcbst 0, %0\n\t"
            "sync\n\t"
            "icbi 0, %0\n\t"
            "isync\n\t"
            : : "r" (powPatchAddr)
        );
        asm volatile (
            "dcbst 0, %0\n\t"
            "sync\n\t"
            "icbi 0, %0\n\t"
            "isync\n\t"
            : : "r" (springAddr)
        );
    }

    if (!player) return 0;
    u32 heldButtons = *(u32*)((u32)player + 0xEA8);
    if ((heldButtons & 0x00010000) == 0) return 0; // 检查是否按了下键

    u32 carryId = *(u32*)((u32)player + OFF_PLAYER_CARRYID);
    if (carryId == 0) return 0;

    dActor_c* carried = lookupActorById(carryId);
    if (!carried) return 0;

    u32 pose = *(u32*)((u32)player + OFF_PLAYER_POSE);
    if (pose == 3) return 0;
    
    u16 profile = *(u16*)((u32)carried + OFF_ACTOR_PROFILE);
    if (profile != 54 && profile != 55 && profile != 56 && profile != 57 && profile != 58 && profile != 133 && profile != 134) return 0;
    
    bool isBomb = (profile == 133 || profile == 134); 

    float px = *(float*)((u32)player + OFF_ACTOR_SPEED_X);

    ((tCancelCarry)cancelCarryAddr)(player, carried);
    if (!carried) return 0;
    
    u8 playerDir = *(u8*)((u32)player + 0x348); 
    u8 shellDir = *(u8*)((u32)carried + 0x348); 
    float shellX = *(float*)((u32)carried + 0xAC);
    
    if (profile == 56) {
        if (playerDir == 0) { 
            if (shellDir == 0) { 
                u32 offset = 0x40D00000;  // 6.5 f
                shellX += *(float*)&offset;
            } else {            
                u32 offset = 0x41300000; 
                shellX += *(float*)&offset;
            }
        } else {
            if (shellDir == 1) { 
                u32 offset = 0x40F00000;  // 7.5 f
                shellX -= *(float*)&offset;
            } else {            
                u32 offset = 0x41500000; 
                shellX -= *(float*)&offset;
            }
        }
    } 
    else {
        if (playerDir == 0) { 
            u32 offset = 0x40E00000; 
            shellX += *(float*)&offset;  
        } else {              
            u32 offset = 0x41100000; 
            shellX -= *(float*)&offset;  
        }
    }
    *(float*)((u32)carried + 0xAC) = shellX;

    u32 speedFactorHex = 0x3F19999A; // 0.6f
    float shellXSpeed = px * *(float*)&speedFactorHex;
    *(float*)((u32)carried + OFF_ACTOR_SPEED_X) = shellXSpeed;
    *(float*)((u32)carried + OFF_ACTOR_SPEED_Y) = 0.0f;

    if (!isBomb) {
        *(u32*)((u32)carried + 0x7D8) = 1; 
        *(u32*)((u32)carried + 0x7D4) = 1;

        u32 vtable = *(u32*)((u32)carried + 0x60);
        u32 stateChangeFunc = *(u32*)(vtable + 0xD4);
        ((void (*)(dActor_c*, u32))stateChangeFunc)(carried, STATE_SLIDE);
    } else {
        *(u32*)((u32)carried + 0x4E4) = 0;
        u32 STATE_BOMB_WALK = 0x80b12aa8; 
        u32 vtable = *(u32*)((u32)carried + 0x60);
        u32 stateChangeFunc = *(u32*)(vtable + 0xD4); 
        ((void (*)(dActor_c*, u32))stateChangeFunc)(carried, STATE_BOMB_WALK);
    }

    *(u32*)((u32)player + 0xEB8) = 0x00010000; // 解决空中下放龟壳/炸弹后出现玩家瞬间下坐的问题
    
    return 1;
}

} // extern "C"