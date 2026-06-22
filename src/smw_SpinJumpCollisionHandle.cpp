#include <common.h>
#include <game.h>
// america
#define ADDR_CHECK_TEST_FLAG  2147839216 
#define ADDR_STANDARD_BOUNCE  2148099856 
#define ADDR_SOUND_PLAYER     0x80198FC0
#define ADDR_SOUND_ENGINE     0x80429D5C
#define ADDR_RET_NOZZLE_HI    0x8012
#define ADDR_RET_NOZZLE_LO    0x8118

#define ADDR_RET_PHYSICS_HI   0x8009
#define ADDR_RET_PHYSICS_LO   0x5C24

#define ADDR_RET_DAMAGE_HI    0x8014
#define ADDR_RET_DAMAGE_LO    0x6184

// europe
// #define ADDR_CHECK_TEST_FLAG  2147839216
// #define ADDR_STANDARD_BOUNCE  2148099856 
// #define ADDR_SOUND_PLAYER     0x80199100 
// #define ADDR_SOUND_ENGINE     0x8042A03C 
// #define ADDR_RET_NOZZLE_HI    0x8012
// #define ADDR_RET_NOZZLE_LO    0x8258
// #define ADDR_RET_PHYSICS_HI   0x8009
// #define ADDR_RET_PHYSICS_LO   0x5C24
// #define ADDR_RET_DAMAGE_HI    0x8014
// #define ADDR_RET_DAMAGE_LO    0x62C4

#define SPIN_TIMER (*(volatile int*)0x80001F00)
// 检查玩家是否长按跳跃键，用于区分大跳和小跳的重力反馈
extern "C" int ApplySpinBounceNozzle(daPlBase_c* player) {
    if (SPIN_TIMER > 10 || SPIN_TIMER < 0) SPIN_TIMER = 0;
    if (SPIN_TIMER > 0) {
        SPIN_TIMER = 0; 
        if (player) {
            u32 input = *(u32*)((u32)player + 0xEA8);
            if ((input & 0x01000000) == 0) return 1; 
        }
    }
    return 0; 
}

// 劫持速度结算逻辑，根据 ApplySpinBounceNozzle 的返回值应用不同的 Y 轴动量 (f1)
extern "C" asm void Nozzle_Hook() {
    nofralloc
    stwu r1, -0x20(r1)
    mflr r0
    stw r0, 0x1C(r1)
    stw r3, 0x08(r1)
    stfs f1, 0x10(r1)

    mr r3, r29 
    bl ApplySpinBounceNozzle
    cmpwi r3, 1
    beq _SmallJump

_NormalJump:
    lfs f1, 0x10(r1)
    b _EndHook

_SmallJump:
    lis r3, 0x402C 
    stw r3, 0x14(r1)
    lfs f1, 0x14(r1)

_EndHook:
    lwz r3, 0x08(r1)
    lwz r0, 0x1C(r1)
    mtlr r0
    addi r1, r1, 0x20
    stfs f1, 0x00EC(r29) 
    
    lis r12, ADDR_RET_NOZZLE_HI
    ori r12, r12, ADDR_RET_NOZZLE_LO
    mtctr r12
    bctr
}

// 检查旋转跳是否成功踩踏普通敌人，若是则触发弹跳并播放原版音效
extern "C" bool CheckSpinJumpBounce(dEn_c* self, ActivePhysics* apThis, ActivePhysics* apOther) {
    if (!apOther || !apOther->owner || !apThis || !self) return false;
    dStageActor_c* actor = (dStageActor_c*)apOther->owner;

    if (actor->name == 13) {  // 确认碰撞对象是玩家
        daPlBase_c* player = (daPlBase_c*)actor;
        typedef bool (*TestFlagFunc)(daPlBase_c*, int);
        TestFlagFunc checkTestFlag = (TestFlagFunc)ADDR_CHECK_TEST_FLAG; 
        u32* spinFlag = (u32*)((u32)player + 0x123C);

        if (checkTestFlag(player, 10) && (*spinFlag & 2048)) { 
            float enemyTop = self->pos.y + apThis->info.yDistToCenter + apThis->info.yDistToEdge;
            float marioBottom = player->pos.y + apOther->info.yDistToCenter - apOther->info.yDistToEdge;
            u32 tolInt = 0x41400000; 
            float tolerance = *(float*)&tolInt;

            if (marioBottom >= (enemyTop - tolerance)) {  // 若判定为有效踩踏，触发弹跳
                typedef void (*BounceFunc)(dEn_c*, void*);
                BounceFunc standardBounce = (BounceFunc)ADDR_STANDARD_BOUNCE; 
                SPIN_TIMER = 1; 
                standardBounce(self, player); 
                
                float centerPos[3] = {0.0f, 0.0f, 0.0f}; 
                typedef void (*SoundFunc)(void*, int, void*, int);
                SoundFunc playSound = (SoundFunc)ADDR_SOUND_PLAYER; 
                void* soundEngine = *(void**)ADDR_SOUND_ENGINE;     
                playSound(soundEngine, 353, centerPos, 0);  // 触发弹跳音效
                
                return true; 
            }
        }
    }
    return false;
}

extern "C" asm void Universal_SpinJump_Hook(dEn_c* self, ActivePhysics* apThis, ActivePhysics* apOther) {
    nofralloc
    stwu r1, -0x50(r1)
    mflr r0
    stw r0, 0x54(r1)
    stw r3, 0x8(r1)
    stw r4, 0xC(r1)
    stw r5, 0x10(r1)
    stw r6, 0x14(r1)
    stw r7, 0x18(r1)
    stw r8, 0x1C(r1)
    stw r9, 0x20(r1)
    stw r10, 0x24(r1)
    stw r11, 0x28(r1)
    stw r12, 0x2C(r1)
    mfcr r0
    stw r0, 0x30(r1)
    bl CheckSpinJumpBounce
    mr r12, r3

    lwz r0, 0x30(r1)
    mtcr r0
    lwz r11, 0x28(r1)
    lwz r10, 0x24(r1)
    lwz r9, 0x20(r1)
    lwz r8, 0x1C(r1)
    lwz r7, 0x18(r1)
    lwz r6, 0x14(r1)
    lwz r5, 0x10(r1)
    lwz r4, 0xC(r1)
    lwz r3, 0x8(r1)
    lwz r0, 0x54(r1)
    mtlr r0
    addi r1, r1, 0x50

    cmpwi r12, 1
    beq _SpinJumped

_NormalCollision:
    stwu r1, -0x10(r1)
    lis r12, ADDR_RET_PHYSICS_HI
    ori r12, r12, ADDR_RET_PHYSICS_LO
    mtctr r12
    bctr

_SpinJumped:
    blr
}

// 危险敌人白名单判定。如果是旋转跳踩踏，则转化伤害为弹跳
extern "C" bool CheckSpecialDamageShield(daPlBase_c* player, ActivePhysics* apThis, ActivePhysics* apOther) {
    if (!player || !apThis || !apOther) return false;
    if (!apThis->owner) return false;
    
    dStageActor_c* attacker = (dStageActor_c*)apThis->owner;
    bool isWhitelisted = false;
    switch(attacker->name) {
        case 0x011C: case 0x011D: case 0x0176: 
        case 0x0063: case 0x0064: case 0x0065: 
        case 0x0066: case 0x0090: case 0x00C0:
            isWhitelisted = true;
            break;
    }

    if (!isWhitelisted) return false; 
    
    u32* spinFlag = (u32*)((u32)player + 0x123C);
    if (*spinFlag & 2048) {
        float enemyTop = attacker->pos.y + apThis->info.yDistToCenter + apThis->info.yDistToEdge;
        float marioBottom = player->pos.y + apOther->info.yDistToCenter - apOther->info.yDistToEdge;
        
        u32 tolInt = 0x41400000; 
        float tolerance = *(float*)&tolInt;
        if (marioBottom >= (enemyTop - tolerance)) {
            typedef void (*BounceFunc)(void*, void*);
            BounceFunc standardBounce = (BounceFunc)ADDR_STANDARD_BOUNCE; 
            SPIN_TIMER = 1;
            standardBounce(player, player); 
            
            float centerPos[3] = {0.0f, 0.0f, 0.0f};
            typedef void (*SoundFunc)(void*, int, void*, int);
            SoundFunc playSound = (SoundFunc)ADDR_SOUND_PLAYER;
            void* soundEngine = *(void**)ADDR_SOUND_ENGINE;
            playSound(soundEngine, 353, centerPos, 0);
            
            return true;  // 成功拦截伤害
        }
    }
    return false; 
}

// 劫持伤害结算大门。如果护盾生效，拦截原始伤害逻辑并将 r3 清零
extern "C" asm void Master_Damage_Hook() {
    nofralloc
    stwu r1, -0x20(r1)
    mflr r0
    stw r0, 0x1C(r1)
    stw r3, 0x08(r1)
    stw r4, 0x0C(r1)
    stw r5, 0x10(r1)
    mfctr r0
    stw r0, 0x14(r1)
    mr r3, r29  
    mr r4, r24  
    mr r5, r23  
    bl CheckSpecialDamageShield
    
    cmpwi r3, 1
    beq _BlockDamage

_NormalDamage:
    lwz r3, 0x08(r1)
    lwz r4, 0x0C(r1)
    lwz r5, 0x10(r1)
    lwz r0, 0x14(r1)
    mtctr r0

    lwz r0, 0x1C(r1)
    mtlr r0
    addi r1, r1, 0x20

    bctrl 
    lis r12, ADDR_RET_DAMAGE_HI
    ori r12, r12, ADDR_RET_DAMAGE_LO
    mtctr r12
    bctr

_BlockDamage:
    lwz r0, 0x1C(r1)
    mtlr r0
    addi r1, r1, 0x20
    li r3, 0 
    lis r12, ADDR_RET_DAMAGE_HI
    ori r12, r12, ADDR_RET_DAMAGE_LO
    mtctr r12
    bctr
}




