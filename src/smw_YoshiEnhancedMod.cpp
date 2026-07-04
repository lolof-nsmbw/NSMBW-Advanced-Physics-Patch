#include <kamek.h>
#include <game/bases/d_enemy.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <game/bases/d_actor.hpp> 
#include <game/bases/d_cc.hpp>
#include <game/bases/d_a_yoshi.hpp>
// ========================================================================
// 1. 外部引擎函数声明 (交由 externals.txt 解析跨区地址)
// ========================================================================
extern "C" void FumiJumpSet__5dEn_cFP8dActor_c(dEn_c* enemy, dActor_c* player);

extern "C" void PlaySound(void* soundEngine, int soundId, void* pos, int unk);
extern "C" void* gSoundEngine; 

extern "C" void YoshiNormalCollision_RTN();
extern "C" void setDamage2__9daYoshi_cFP8dActor_cQ210daPlBase_c12DamageType_e();
extern "C" u8 StateID_Slip__9daYoshi_c[];
// ========================================================================
// 核心物理运算：耀西侧壁 X 轴外力动量弹反逻辑 (ApplyYoshiSphericalBounceX)
// ========================================================================
static void ApplyYoshiSphericalBounceX(dActor_c* yoshi, dEn_c* enemy) {
    float deltaX = yoshi->mPos.x - enemy->mPos.x;
    
    //写入耀西真正的外力动量向量 0x10C (继承自实体基类的相同内存布局)
    float* pYoshiSpeedX = (float*)((u32)yoshi + 0x10C); 
    
    float fixedSpeed = 1.5f;
    float boostSpeed = 1.0f;

    if (deltaX < 0.0f) { 
        if (*pYoshiSpeedX > 0.0f) *pYoshiSpeedX = -fixedSpeed; 
        else *pYoshiSpeedX -= boostSpeed; 
    } else {
        if (*pYoshiSpeedX < 0.0f) *pYoshiSpeedX = fixedSpeed;  
        else *pYoshiSpeedX += boostSpeed; 
    }
}

// ========================================================================
// 2. 业务逻辑 C++ 函数
// ========================================================================
extern "C" bool CheckYoshiSpecialCollision(dEn_c* enemy, dCc_c* enemyAp, dCc_c* yoshiAp) {
    if (!enemy || !yoshiAp || !enemyAp) return false;
    
    u16 enemyName = enemy->mProfName;
    dActor_c* yoshiActor = yoshiAp->mpOwner;
    if (!yoshiActor) return false;
    // ========================================================================
    // 【新增防线：耀西下滑专属免伤大门】
    // ========================================================================
    daYoshi_c* yoshi = (daYoshi_c*)yoshiActor;
    
    // 通过reinterpret_cast 与 isState 判定是否处于滑行状态
    sStateIDIf_c *yoshiSlipState = reinterpret_cast<sStateIDIf_c*>(StateID_Slip__9daYoshi_c);
    bool isYoshiSliding = yoshi->isState(*yoshiSlipState);
    
    if (isYoshiSliding) {
        switch (enemyName) {
            // [1] 软体怪：在前方拦截器已经被杀死了，这里无条件没收伤害！
            case 0x0039: case 0x0057: 
            case 0x005B: case 0x005C: case 0x005D: case 0x005E: 
            case 0x005F: case 0x0060: case 0x0061: case 0x0062: 
            case 0x0063: case 0x0065: case 0x0067: 
            case 0x0091: case 0x0095: case 0x009F: 
            case 0x00C8: case 0x011C: case 0x011F: case 0x0120: 
            case 0x014C: case 0x014D: case 0x0153: case 0x01FF:
                return true; 
                
            // [2] 铁球硬物：仅侧面撞击免伤，放行给物理拦截器做弹反！
            case 0x0059: case 0x005A: 
            case 0x0064: case 0x0066: case 0x0094: case 0x0176: 
            case 0x00C0: case 0x0051: case 0x0052: case 0x0143: 
            case 0x011D: 
            case 0x00F1: case 0x00F2: case 0x00F3: case 0x00F4: 
            case 0x00F5: case 0x00F6: case 0x00F7:
            {
                float deltaX = yoshi->mPos.x - enemy->mPos.x;
                float absDeltaX = deltaX < 0.0f ? -deltaX : deltaX;
                float enemyRadius = enemyAp->mCcData.mBase.mSize.x;
                
                // 侧面撞击外壁时给予免伤
                if (absDeltaX > (enemyRadius * 0.8f)) {
                    return true; 
                }
                break; // 头顶砸落，走原版受伤逻辑
            }
        }
    }

    // ========================================================================
    // ========================================================================
    // ========================================================================

    float enemyTop = enemy->mPos.y + enemyAp->mCcData.mBase.mOffset.y + enemyAp->mCcData.mBase.mSize.y;
    float yoshiBottom = yoshiActor->mPos.y + yoshiAp->mCcData.mBase.mOffset.y - yoshiAp->mCcData.mBase.mSize.y;
    
    // =========================================================
    // 【耀西专属：巨型海胆(0x011D) & 巨型铁球(0x005A) 双轨判定】
    // =========================================================
    if (enemyName == 0x011D || enemyName == 0x005A) {
        float tolT0 = 12.0f;                                  // T0 核心区：固定 12.0f
        float tolT1 = enemyAp->mCcData.mBase.mSize.y * 0.9f;  // T1 曲面区：统一对齐上半球的 45%

        if (yoshiBottom >= (enemyTop - tolT0)) {
            // 命中 T0 核心区：正上方标准踩踏
            FumiJumpSet__5dEn_cFP8dActor_c(enemy, yoshiActor);
            float centerPos[3] = {0.0f, 0.0f, 0.0f};
            PlaySound(gSoundEngine, 353, centerPos, 0); 
            return true; 
        } else if (yoshiBottom >= (enemyTop - tolT1)) {
            // 命中 T1 曲面区：触发踩踏，并赋予耀西侧边弹力！
            FumiJumpSet__5dEn_cFP8dActor_c(enemy, yoshiActor);
            ApplyYoshiSphericalBounceX(yoshiActor, enemy); 
            float centerPos[3] = {0.0f, 0.0f, 0.0f};
            PlaySound(gSoundEngine, 353, centerPos, 0); 
            return true;
        }
    }
    // =========================================================

    // =========================================================
    // 【常规 12.0f 判定区】
    // =========================================================
    u32 tolInt = 0x41400000; 
    float tolerance = *(float*)&tolInt;
    
    if (yoshiBottom >= (enemyTop - tolerance)) {
        bool doStandardBounce = false;
        switch(enemyName) {
            case 0x011C: // Urchin
            case 0x0143: // Bramball
            case 0x0176: // Spike/stone ball
            case 0x0051: // Thwomp
            case 0x0052: // Big Thwomp
            case 0x0059: // Spiked Ball
            case 0x0090: // Line Controlled Fuzzy
            case 0x0094: // Spiked Ball On a Chain
            case 0x00A1: // Spiny Cheep Cheep
            case 0x00C0: // Porcupuffer
            case 0x00C2: // Fire Snake
            case 0x00C8: // Prickly Goomba
            case 0x00F1: // Larry shell
            case 0x00F2: // Wendy shell
            case 0x00F3: // Iggy shell
            case 0x00F4: // Lemmy shell
            case 0x00F5: // Morton shell
            case 0x00F6: // Roy shell
            case 0x00F7: // Ludwig shell
            // ========================================================
            // [2] Newer 引擎追加刺怪 (Newer Custom Enemies)
            // ========================================================
            case 0x0154: //Boss Fuzzy Bear
            case 0x0089: //Boss Thwompa Domp
            case 0x0155: //The Sun and the Moon
            case 0x0218: //Boss in world 3
            case 0x0290: //Boss in world 4
            case 0x0238: //0238
            case 0x0014: //Boss in world 7
                doStandardBounce = true;
                break;
        }
        
        if (doStandardBounce) {
            FumiJumpSet__5dEn_cFP8dActor_c(enemy, yoshiActor);
            float centerPos[3] = {0.0f, 0.0f, 0.0f};
            PlaySound(gSoundEngine, 353, centerPos, 0); 
            return true; 
        }
        
        // 【直接击杀白名单，cooligan,fishbone,blooper,cheep cheep】
        if (enemyName == 0x00C6 || enemyName == 0x0120|| enemyName == 0x0095|| enemyName == 0x009F) { 
            FumiJumpSet__5dEn_cFP8dActor_c(enemy, yoshiActor);
            enemy->yoshifumiSE(yoshiActor);
            enemy->YoshiFumiScoreSet(yoshiActor);
            enemy->setDeathInfo_YoshiFumi(yoshiActor);
            return true;
        }
    }
    return false; 
}
// 【Hook 1：耀西加强】
kmBranchDefAsm(0x80095C80, 0x80095C84) {
    nofralloc
    stwu r1, -0x20(r1)
    mflr r0
    stw r0, 0x1C(r1)
    stw r3, 0x08(r1)
    stw r4, 0x0C(r1)
    stw r5, 0x10(r1)
    
    // C++ 符号调用
    bl CheckYoshiSpecialCollision
    
    mr r12, r3

    lwz r3, 0x08(r1)
    lwz r4, 0x0C(r1)
    lwz r5, 0x10(r1)
    lwz r0, 0x1C(r1)
    mtlr r0
    addi r1, r1, 0x20

    cmpwi r12, 1
    beq _YoshiInvincible 

_NormalCollision:
    stwu sp, -0x0010(sp)
    // 跨文件跳转到原版游戏的下一个逻辑节点
    b YoshiNormalCollision_RTN

_YoshiInvincible:
    li r3, 0 
    mflr r12
    mtctr r12
    bctr

_EndMacro:
    blr // 留给 Kamek 扫描器的祭品
}

// 【Hook 2：地形尖刺免疫】 
// 因为这是一个对原版 bl 指令的劫持，使用 kmCall
extern "C" void TerrainSpike_Hook();
kmCall(0x800556CC, TerrainSpike_Hook);

// 独立的汇编函数，里面的 blr 是合法且安全的
asm void TerrainSpike_Hook() {
    nofralloc
    cmpwi r5, 1
    bne _ExecuteOriginalDamage 
    
    // 动态加载对应区服的 setDamage2__9daYoshi_cFP8dActor_cQ210daPlBase_c12DamageType_e 地址
    lis r11, setDamage2__9daYoshi_cFP8dActor_cQ210daPlBase_c12DamageType_e@h
    ori r11, r11, setDamage2__9daYoshi_cFP8dActor_cQ210daPlBase_c12DamageType_e@l
    
    cmpw r12, r11
    bne _ExecuteOriginalDamage  
    
    lbz r11, 0x02CC(r30)
    andi. r11, r11, 0x04
    beq _ExecuteOriginalDamage  

_CancelDamage:
    blr // 跳回上一级

_ExecuteOriginalDamage:
    bctr
}