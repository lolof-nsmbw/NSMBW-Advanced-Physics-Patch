#include <kamek.h>
#include <game/bases/d_enemy.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <game/bases/d_actor.hpp> 

// ========================================================================
// 1. 外部引擎函数声明 (交由 externals.txt 解析跨区地址)
// ========================================================================
// 核心修正 1：将dStageActor_c* 替换为正统的 dActor_c*
extern "C" void FumiJumpSet__5dEn_cFP8dActor_c(dEn_c* enemy, dActor_c* player);

extern "C" void PlaySound(void* soundEngine, int soundId, void* pos, int unk);
extern "C" void* gSoundEngine; 

extern "C" void YoshiNormalCollision_RTN();
extern "C" void setDamage2__9daYoshi_cFP8dActor_cQ210daPlBase_c12DamageType_e();

// ========================================================================
// 2. 业务逻辑 C++ 函数 (完全剥离硬编码，纯净的 C++ 实现)
// ========================================================================
extern "C" bool CheckYoshiSpecialCollision(dEn_c* enemy, dCc_c* enemyAp, dCc_c* yoshiAp) {
    if (!enemy || !yoshiAp || !enemyAp) return false;
    
    u16 enemyName = enemy->mProfName;
    
    // 核心修正 2：使用 dActor_c 作为基类，完美接收 mpOwner
    dActor_c* yoshiActor = yoshiAp->mpOwner;
    if (!yoshiActor) return false;
    
    float enemyTop = enemy->mPos.y + enemyAp->mCcData.mBase.mOffset.y + enemyAp->mCcData.mBase.mSize.y;
    float yoshiBottom = yoshiActor->mPos.y + yoshiAp->mCcData.mBase.mOffset.y - yoshiAp->mCcData.mBase.mSize.y;
    
    u32 tolInt = 0x41400000; 
    float tolerance = *(float*)&tolInt;
    
    if (yoshiBottom >= (enemyTop - tolerance)) {
        bool doStandardBounce = false;
        switch(enemyName) {
            case 0x011C: //Urchin
            case 0x011D: //Mega Urchin
            case 0x0120: //Fishbones
            case 0x0143: //Bramball
            case 0x0176: //Spike/stone ball produced by jiabang(not work)
            case 0x0051: //Thwomp
            case 0x0052: //Big Thwomp
            case 0x0059: //Spiked Ball
            case 0x005a: //Giant Spiked Ball
            case 0x0090: //Line Controlled Fuzzy
            case 0x0094: //Spiked Ball On a Chain
            case 0x0095: //Blooper
            case 0x009F: //Cheep Cheep
            case 0x00A1: //Spiny Cheep Cheep
            case 0x00C0: //Porcupuffer
            case 0x00C2: //Fire Snake
            case 0x00C8: //Prickly Goomba
            case 0x00F1: //Larry shell
            case 0x00F2: //Wendy shell
            case 0x00F3: //Iggy shell
            case 0x00F4: //Lemmy shell
            case 0x00F5: //Morton shell
            case 0x00F6: //Roy shell
            case 0x00F7: //Ludwig shell
                doStandardBounce = true;
                break;
        }
        
        if (doStandardBounce) {
            FumiJumpSet__5dEn_cFP8dActor_c(enemy, yoshiActor);
            float centerPos[3] = {0.0f, 0.0f, 0.0f};
            PlaySound(gSoundEngine, 353, centerPos, 0); 
            return true; 
        }
        
        if (enemyName == 0x00C6) { 
            FumiJumpSet__5dEn_cFP8dActor_c(enemy, yoshiActor);
            // 核心修正 3：原版击杀函数
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