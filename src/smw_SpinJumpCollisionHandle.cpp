// Adapted for PropelParts. 
#include <kamek.h>
#include <game/bases/d_enemy.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <game/bases/d_cc.hpp>
#include <game/bases/d_actor.hpp>

// --- 外部符号声明 (由 externals.txt 解析) ---
extern "C" bool CheckTestFlag(daPlBase_c* player, int flag);
extern "C" void StandardSpinBounce(void* a, void* b); // 修复了参数类型冲突
extern "C" void PlaySound(void* soundEngine, int soundId, void* pos, int unk);
extern "C" void* gSoundEngine; 

// --- 状态机大一统 ---
// 直接复用 variablespinjump.cpp 里全局跳跃状态变量！
// 这样当这里触发踩踏时，那边的物理文件会自动接管高度计算，彻底解耦！
extern "C" volatile u8 IS_SPIN_BOUNCING; 

// ========================================================================
// 踩踏判定逻辑
// ========================================================================
extern "C" bool CheckSpinJumpBounce(dEn_c* self, dCc_c* apThis, dCc_c* apOther) {
    if (!apOther || !apOther->mpOwner || !apThis || !self) return false;
    dActor_c* actor = (dActor_c*)apOther->mpOwner;

    if (actor->mProfName == 13) { 
        daPlBase_c* player = (daPlBase_c*)actor;
        u32* spinFlag = (u32*)((u32)player + 0x123C);

        if (CheckTestFlag(player, 10) && (*spinFlag & 2048)) { 
            // 修复 pos 报错：现代反编译头文件中为 mPos
            float enemyTop = self->mPos.y + apThis->mCcData.mBase.mOffset.y + apThis->mCcData.mBase.mSize.y;
            float marioBottom = player->mPos.y + apOther->mCcData.mBase.mOffset.y - apOther->mCcData.mBase.mSize.y;
            u32 tolInt = 0x41400000; 
            float tolerance = *(float*)&tolInt;

            if (marioBottom >= (enemyTop - tolerance)) {
                StandardSpinBounce(self, player); 
                
                // 统一状态机！通知动量控制模块给与正确的 Y 轴速度
                IS_SPIN_BOUNCING = 1; 

                float centerPos[3] = {0.0f, 0.0f, 0.0f}; 
                PlaySound(gSoundEngine, 353, centerPos, 0);
                
                return true; 
            }
        }
    }
    return false;
}

// ========================================================================
// 伤害拦截逻辑
// ========================================================================
extern "C" bool CheckSpecialDamageShield(daPlBase_c* player, dCc_c* apThis, dCc_c* apOther) {
    if (!player || !apThis || !apOther) return false;
    if (!apThis->mpOwner) return false;
    
    dActor_c* attacker = (dActor_c*)apThis->mpOwner;
    bool isWhitelisted = false;
    switch(attacker->mProfName) {
        case 0x011C: case 0x011D: case 0x0176: 
        case 0x0063: case 0x0064: case 0x0065: 
        case 0x0066: case 0x0090: case 0x00C0:
            isWhitelisted = true;
            break;
    }

    if (!isWhitelisted) return false; 
    
    u32* spinFlag = (u32*)((u32)player + 0x123C);
    if (*spinFlag & 2048) {
        // 修复 pos 报错
        float enemyTop = attacker->mPos.y + apThis->mCcData.mBase.mOffset.y + apThis->mCcData.mBase.mSize.y;
        float marioBottom = player->mPos.y + apOther->mCcData.mBase.mOffset.y - apOther->mCcData.mBase.mSize.y;
        
        u32 tolInt = 0x41400000; 
        float tolerance = *(float*)&tolInt;
        if (marioBottom >= (enemyTop - tolerance)) {
            StandardSpinBounce(player, player); 
            
            // 统一状态机！
            IS_SPIN_BOUNCING = 1;

            float centerPos[3] = {0.0f, 0.0f, 0.0f};
            PlaySound(gSoundEngine, 353, centerPos, 0);
            
            return true; 
        }
    }
    return false; 
}

// ========================================================================
// Inline Assembly Trampolines
// ========================================================================
kmBranchDefAsm(0x80095C20, 0x80095C24) {
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
    b _EndMacro

_SpinJumped:
    mflr r12
    mtctr r12
    bctr

_EndMacro:
    blr
}

kmBranchDefAsm(0x801462C0, 0x801462C4) {
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
    b _EndMacro

_BlockDamage:
    lwz r0, 0x1C(r1)
    mtlr r0
    addi r1, r1, 0x20
    li r3, 0 
    b _EndMacro

_EndMacro:
    blr
}