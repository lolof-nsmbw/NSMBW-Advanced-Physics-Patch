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

// 复用全局跳跃状态变量
// 这样当这里触发踩踏时，那边的物理文件会自动接管高度计算，彻底解耦！
extern "C" volatile u8 IS_SPIN_BOUNCING; 
// ========================================================================
// 核心物理运算：球形怪物侧壁 X 轴速度弹反逻辑 (ApplySphericalBounceX)
// ========================================================================
static void ApplySphericalBounceX(daPlBase_c* player, dActor_c* enemy) {
    // 计算 X 轴相对距离，判断踩在左半球还是右半球
    float deltaX = player->mPos.x - enemy->mPos.x;
    
    // 写入 0x10C (外力动量向量)，绕过十字键输入覆盖！
    float* pSpeedX = (float*)((u32)player + 0x10C); 
    
    // 手感调校参数
    float fixedSpeed = 1.5f;  // 逆向碰撞时，被强行覆写的固定反弹速度
    float boostSpeed = 1.0f;  // 顺向碰撞时，顺势赋予的推进增量

    if (deltaX < 0.0f) { 
        // -------------------------
        // 踩在【左半球】
        // -------------------------
        if (*pSpeedX > 0.0f) {
            *pSpeedX = -fixedSpeed; // 本想往右，被圆弧强行弹回左侧
        } else {
            *pSpeedX -= boostSpeed; // 顺势向左加速！(引擎会自动限制在 -3.0f 以内)
        }
    } else {
        // -------------------------
        // 踩在【右半球】
        // -------------------------
        if (*pSpeedX < 0.0f) {
            *pSpeedX = fixedSpeed;  // 本想往左，被圆弧强行弹回右侧
        } else {
            *pSpeedX += boostSpeed; // 顺势向右加速！(引擎会自动限制在 3.0f 以内)
        }
    }
}

// ========================================================================
// 1. 踩踏判定逻辑 (针对 Giant Spike Ball: 0x005A)
// ========================================================================
extern "C" bool CheckSpinJumpBounce(dEn_c* self, dCc_c* apThis, dCc_c* apOther) {
    if (!apOther || !apOther->mpOwner || !apThis || !self) return false;
    dActor_c* actor = (dActor_c*)apOther->mpOwner;

    if (actor->mProfName == 13) { 
        daPlBase_c* player = (daPlBase_c*)actor;
        u32* spinFlag = (u32*)((u32)player + 0x123C);

        if (CheckTestFlag(player, 10) && (*spinFlag & 2048)) { 
            float enemyTop = self->mPos.y + apThis->mCcData.mBase.mOffset.y + apThis->mCcData.mBase.mSize.y;
            float marioBottom = player->mPos.y + apOther->mCcData.mBase.mOffset.y - apOther->mCcData.mBase.mSize.y;
            
            // 【巨型铁球 (Giant Spike Ball) 专属双层判定】
            if (self->mProfName == 0x005A) {
                float tolT0 = 12.0f;                                  // T0 核心区：固定 12.0f
                float tolT1 = apThis->mCcData.mBase.mSize.y * 0.9f;   // T1 曲面区：上半球的 45%

                if (marioBottom >= (enemyTop - tolT0)) {
                    // 命中 T0 核心区：正上方标准踩踏，不改变任何 X 速度
                    StandardSpinBounce(self, player); 
                    IS_SPIN_BOUNCING = 1; 
                    float centerPos[3] = {0.0f, 0.0f, 0.0f}; 
                    PlaySound(gSoundEngine, 353, centerPos, 0);
                    return true; 

                } else if (marioBottom >= (enemyTop - tolT1)) {
                    // 命中 T1 曲面区：触发旋转跳，但必须强行弹反 X 速度！
                    StandardSpinBounce(self, player); 
                    IS_SPIN_BOUNCING = 1;
                    
                    // 【核心】注入弹反逻辑！
                    ApplySphericalBounceX(player, (dActor_c*)self); 
                    
                    float centerPos[3] = {0.0f, 0.0f, 0.0f}; 
                    PlaySound(gSoundEngine, 353, centerPos, 0);
                    return true;
                }
            } 
            // 【常规 12.0f 判定 (后续可以改为白名单机制)】
            else {
                float tolerance = 12.0f;
                if (marioBottom >= (enemyTop - tolerance)) {
                    StandardSpinBounce(self, player); 
                    IS_SPIN_BOUNCING = 1; 
                    float centerPos[3] = {0.0f, 0.0f, 0.0f}; 
                    PlaySound(gSoundEngine, 353, centerPos, 0);
                    return true; 
                }
            }
        }
    }
    return false;
}

// ========================================================================
// 2. 伤害拦截逻辑 (兼顾原版弹跳兜底 与 Newer 怪物纯免伤解耦)
// ========================================================================
extern "C" bool CheckSpecialDamageShield(daPlBase_c* player, dCc_c* apThis, dCc_c* apOther) {
    if (!player || !apThis || !apOther) return false;
    if (!apThis->mpOwner) return false;
    
    dActor_c* attacker = (dActor_c*)apThis->mpOwner;
    // ========================================================================
    // 【推土机防线】：完全独立于旋转跳的滑铲判定
    // ========================================================================
    bool isSliding = player->isStatus(daPlBase_c::STATUS_SLIP_ACTIVE) || 
                     player->isStatus(daPlBase_c::STATUS_INITIAL_SLIDE);
                     
    if (isSliding) {
        switch (attacker->mProfName) {
            // [1] 直接推平的软体怪物 (无条件免伤)
            case 0x0153: case 0x01FF: case 0x0120:  //icicle，fishbone
            case 0x011C: case 0x0095: case 0x009F:  //urchin，blooper,cheep cheep
            case 0x014D: case 0x011F: //spore ball of Lily Piranha plant,螃蟹扔的小石块
                return true; 
            // [2] 铁球等硬物 (需进行【几何有效碰撞检测】才免伤)
            // [2] 铁球硬物防线：几何判定与弹反！
                    case 0x0059: // Spiked Ball
                    case 0x005A: // Mega Spike Ball
                    case 0x0064: // Big piranha plant
                    case 0x0066: // Big fire piranha plant
                    case 0x0094: // spiked ball on a chain
                    case 0x0176: // Spiked/stone spike
                    case 0x00C0: // Porcupuffer
                    case 0x0051: // Thwomp
                    case 0x0052: // Big Thwomp
                    case 0x0143: // Bramball
                    case 0x011D: // Mega Urchin
                    case 0x00F1: // Larry shell
                    case 0x00F2: // Wendy shell
                    case 0x00F3: // Iggy shell
                    case 0x00F4: // Lemmy shell
                    case 0x00F5: // Morton shell
                    case 0x00F6: // Roy shell
                    case 0x00F7: // Ludwig shell 
            {
                // 【圆心距离防穿模】
                float deltaX = player->mPos.x - attacker->mPos.x;
                if (deltaX < 0.0f) deltaX = -deltaX; 
                float enemyRadius = apThis->mCcData.mBase.mSize.x;
                // 距离大于半径的 0.8 倍，说明是在侧面撞击外壁
                if (deltaX > (enemyRadius * 0.8f)) {
                    return true; // 拦截伤害
                }
                // 如果 deltaX <= 半径的 0.8倍，此时直接 break，让引擎走原版的扣血逻辑！
                break;
            }
        }
    }

    
    // 【架构分离】：独立标记原版与 Newer 怪物
    bool isVanillaHazard = false;
    bool isNewerCustom = false;
    
    switch(attacker->mProfName) {
        // ========================================================
        // [1] 原版刺怪 (依赖 Hook B 进行弹跳与免伤双重兜底)
        // ========================================================
        case 0x011C: case 0x011D: case 0x0176: 
        case 0x0063: case 0x0064: case 0x0065: 
        case 0x0066: case 0x0090: case 0x00C0:
        case 0x0153:
            isVanillaHazard = true;
            break;
            
        // ========================================================
        // [2] Newer 引擎追加刺怪 (依赖 Hook A 弹跳，Hook B 仅作免伤)
        // ========================================================
        case 0x0154: //Boss Fuzzy Bear
        case 0x0089: //Boss Thwompa Domp
        case 0x0155: //The Sun and the Moon
        case 0x0218: //Boss in world 3
        case 0x0290: //Boss in world 4
        case 0x0238: //0238
        case 0x0014: //Boss in world 7
            isNewerCustom = true;
            break;
    }

    if (!isVanillaHazard && !isNewerCustom) return false; 
    
    u32* spinFlag = (u32*)((u32)player + 0x123C);
    if (*spinFlag & 2048) {
        float enemyTop = attacker->mPos.y + apThis->mCcData.mBase.mOffset.y + apThis->mCcData.mBase.mSize.y;
        float marioBottom = player->mPos.y + apOther->mCcData.mBase.mOffset.y - apOther->mCcData.mBase.mSize.y;
        
        // ====================================================================
        // 【新增】：Newer 怪物专属防弹衣逻辑
        // ====================================================================
        if (isNewerCustom) {
            float tolerance = 12.0f;
            if (marioBottom >= (enemyTop - tolerance)) {
                // 此时 Hook A 已经让马里奥弹起来了，这里直接没收伤害！
                return true; 
            }
            return false;
        }

        // ====================================================================
        // 以下为原版怪物逻辑
        // ====================================================================
        // 【巨型球体专属双层判定,如mega urchin】
        if (attacker->mProfName == 0x011D) {
            float tolT0 = 12.0f;                                  // T0 核心区
            float tolT1 = apThis->mCcData.mBase.mSize.y * 0.9f;   // T1 曲面区

            if (marioBottom >= (enemyTop - tolT0)) {
                // 命中 T0 核心区
                StandardSpinBounce(player, player); 
                IS_SPIN_BOUNCING = 1;
                float centerPos[3] = {0.0f, 0.0f, 0.0f};
                PlaySound(gSoundEngine, 353, centerPos, 0);
                return true; 
                
            } else if (marioBottom >= (enemyTop - tolT1)) {
                // 命中 T1 曲面区
                StandardSpinBounce(player, player); 
                IS_SPIN_BOUNCING = 1;
                
                // 【核心】注入弹反逻辑！
                ApplySphericalBounceX(player, attacker); 
                
                float centerPos[3] = {0.0f, 0.0f, 0.0f};
                PlaySound(gSoundEngine, 353, centerPos, 0);
                return true;
            }
        }
        // 【常规 12.0f 判定 (其他白名单原版刺怪)】
        else {
            float tolerance = 12.0f;
            if (marioBottom >= (enemyTop - tolerance)) {
                StandardSpinBounce(player, player); 
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



