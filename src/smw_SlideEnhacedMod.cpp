#include <kamek.h>

#include <game/bases/d_actor.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <game/bases/d_a_yoshi.hpp>
#include <game/bases/d_cc.hpp>
#include <game/bases/d_enemy.hpp>
#include <game/bases/d_score_manager.hpp>

// Reuse engine definition from headers.

extern "C" void kill__5dEn_cFv(dEn_c* enemy);
extern "C" void FumiScoreSet__5dEn_cFP8dActor_c(dEn_c* enemy, dActor_c* player);
extern "C" void EN_LandbarrelPlayerCollision(dEn_c* enemyThis, dCc_c* enemySensor, dCc_c* playerSensor);
// StateID symbol for Yoshi slip state (mapped via externals.txt at link/runtime)
extern "C" u8 StateID_Slip__9daYoshi_c[];

// 极其关键的辅助函数：瞬间剥夺怪物的攻击力！
static void DisableColliderNow(dCc_c* sensor) {
    if (!sensor) return;
    sensor->mCcData.mVsDamage = 0;
    sensor->mCcData.mVsKind = 0;
}

// ========================================================================
// 2. 核心拦截逻辑：SMM2 增强版下滑推土机
// ========================================================================
extern "C" bool GodModeSlideInterceptor(dCc_c* enemySensor, dCc_c* playerSensor) {
    // 1. 安全过滤传感器
    if (!enemySensor || !playerSensor) return false;
    dActor_c* enemyBase = enemySensor->mpOwner;
    dActor_c* playerBase = playerSensor->mpOwner;
    if (!enemyBase || !playerBase) return false;
    
    // 2. 身份解析与状态机分离
    daPlBase_c* player = nullptr;
    dActor_c* targetToPush = nullptr; // 用于最终物理击退的受力体
    bool isNormalSlide = false;       

    if (playerBase->mProfName == 13) {
        // [1] 步兵模式：传感器属于玩家本体
        player = (daPlBase_c*)playerBase;
        targetToPush = playerBase;
        
        // 直接查玩家的状态位
        isNormalSlide = player->isStatus(daPlBase_c::STATUS_SLIP_ACTIVE) || 
                        player->isStatus(daPlBase_c::STATUS_INITIAL_SLIDE) ||
                        player->isStatus(daPlBase_c::STATUS_PENGUIN_SLIDE);
    } 
    else if (playerBase->mProfName == 14) {
        // [2] 骑兵模式：传感器属于耀西
        daYoshi_c* yoshi = (daYoshi_c*)playerBase;
        player = yoshi->getPlayerRideOn(); // 拿到背上的骑手，用于加分
        targetToPush = playerBase;         // 击退时弹飞耀西
        
        // 查耀西当前是否处于滑行状态。
        sStateIDIf_c *yoshiSlipState = reinterpret_cast<sStateIDIf_c*>(StateID_Slip__9daYoshi_c);
        isNormalSlide = yoshi->isState(*yoshiSlipState);
    } 
    else {
        return false; // 非玩家且非耀西，直接放行
    }

    // 身份与合法性校验
    if (!player) return false; 
    if (!isNormalSlide) return false; 
    
    dEn_c* enemy = (dEn_c*)enemyBase;
    
    // 3. 生死簿白名单 
    bool shouldKill = false;
    bool shouldKnockoff = false;
    
    switch (enemy->mProfName) {
        // [1] 白名单 M：直接击杀这些小型软体怪物
        case 0x0039: // Spiny
        case 0x0057: // spike top
        case 0x005B: case 0x005C: case 0x005D: case 0x005E: // pipe Piranha plant
        case 0x005F: case 0x0060: case 0x0061: case 0x0062: // pipe Fire Piranha plant
        case 0x0063: // Piranha plant
        case 0x0065: // fire piranha plant
        case 0x0067: // stalking piranha plant
        case 0x0091: // pokey
        case 0x0095: // blooper
        case 0x009F: // cheep cheep
        case 0x00C8: // prickly goomba
        case 0x011C: // Urchin
        case 0x011F: // 螃蟹扔的小石块
        case 0x0120: // Fishbone
        case 0x014C: // Lily Piranha plant
        case 0x014D: // spore ball of Lily Piranha plant
        case 0x0153: // falling icicle
        case 0x01FF: // icicle
            shouldKill = true;
            break;
            
        // [2] 白名单 N：太硬或者太大，击退玩家/耀西
        case 0x0059: // spiked ball
        case 0x005A: // Giant spiked ball
        case 0x0064: // Big piranha plant
        case 0x0066: // Big fire piranha plant
        case 0x0094: // spiked ball on a chain
        case 0x0176: // Spiked/stone spike
        case 0x00C0: // Porcupuffer
        case 0x0051: // Thwomp
        case 0x0052: // Big Thwomp
        case 0x0143: // Bramball
        case 0x011D: // Mega urchin
        case 0x00F1: case 0x00F2: case 0x00F3: case 0x00F4: // Koopalings
        case 0x00F5: case 0x00F6: case 0x00F7:
            shouldKnockoff = true;
            break;
    }
    
    // 4. 击杀怪物
    if (shouldKill) {
        DisableColliderNow(enemySensor);
        
        // 计分系统，共轨原生计数器！
        int currentCombo = player->mPlComboCount;
        ulong scoreType = 1; // 200 分起步
        if (currentCombo == 1) scoreType = 2;      // 400
        else if (currentCombo == 2) scoreType = 3; // 800
        else if (currentCombo == 3) scoreType = 4; // 1000
        else if (currentCombo == 4) scoreType = 5; // 2000
        else if (currentCombo == 5) scoreType = 6; // 4000
        else if (currentCombo == 6) scoreType = 7; // 8000
        else if (currentCombo >= 7) scoreType = 8; // 1UP
        
        if (dScoreMng_c* scoreMng = dScoreMng_c::getInstance()) {
            int pNo = player->mPlayerNo; 
            scoreMng->ScoreSet(enemy, scoreType, pNo, 0.0f, 0.0f);
        }
        
        player->mPlComboCount++;
        kill__5dEn_cFv(enemy);
        return true; 
    }

    if (shouldKnockoff) {
        // 计算距离也用 targetToPush，保证骑兵模式下是以耀西的位置为准！
        float deltaX = targetToPush->mPos.x - enemy->mPos.x;
        float absDeltaX = deltaX < 0.0f ? -deltaX : deltaX;
        float enemyRadius = enemySensor->mCcData.mBase.mSize.x;
        
        // 侧面撞击铁球
        if (absDeltaX > (enemyRadius * 0.8f)) {
            // 这里必须推 targetToPush（步兵推玩家，骑兵推耀西）
            float* pExtForceX = (float*)((u32)targetToPush + 0x10C);
            *pExtForceX = (deltaX > 0.0f) ? 2.5f : -2.5f; 

            return true; 
        }
    }
    return false; 
}

// ========================================================================
// 3. 底层汇编跳板
// ========================================================================
kmBranchDefAsm(0x80096300, 0x80096304) {
    nofralloc
    stwu r1, -0x30(r1)       
    mflr r0
    stw r0, 0x34(r1)
    
    stw r3, 0x08(r1)
    stw r4, 0x0C(r1)
    stfs f1, 0x10(r1)
    
    bl GodModeSlideInterceptor
    
    cmpwi r3, 1              
    beq _HandledByGodMode
    
_RunVanilla:
    lwz r3, 0x08(r1)
    lwz r4, 0x0C(r1)
    lfs f1, 0x10(r1)
    lwz r0, 0x34(r1)
    mtlr r0
    addi r1, r1, 0x30
    
    lfs f0, 0x0020(r3)       
    b _EndMacro              
    
_HandledByGodMode:
    lwz r0, 0x34(r1)
    mtlr r0
    addi r1, r1, 0x30
    
    li r3, 0                 
    mflr r12
    mtctr r12
    bctr
    
_EndMacro:
    blr 
}