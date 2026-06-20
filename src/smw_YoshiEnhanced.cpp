#include <common.h>
#include <game.h> 
#define BouncePlayerWhenJumpedOn ((void (*)(dEn_c*, dStageActor_c*))0x80096710)
#define ADDR_SOUND_PLAYER 0x80199100
#define ADDR_SOUND_ENGINE 0x8042A03C

extern "C" bool CheckYoshiSpecialCollision(dEn_c* enemy, ActivePhysics* enemyAp, ActivePhysics* yoshiAp);

extern "C" asm void YoshiCollision_Hook() {
    nofralloc
    stwu r1, -0x20(r1)
    mflr r0
    stw r0, 0x1C(r1)
    stw r3, 0x08(r1)
    stw r4, 0x0C(r1)
    stw r5, 0x10(r1)
    lis r12, 0x8000
    ori r12, r12, 0x2068 
    mtctr r12
    bctrl
    
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
    lis r12, 0x8009
    ori r12, r12, 0x5C84
    mtctr r12
    bctr

_YoshiInvincible:
    li r3, 0 
    blr
}

extern "C" bool CheckYoshiSpecialCollision(dEn_c* enemy, ActivePhysics* enemyAp, ActivePhysics* yoshiAp) {
    if (!enemy || !yoshiAp || !enemyAp) return false;
    
    u16 enemyName = enemy->name;
    dStageActor_c* yoshiActor = (dStageActor_c*)yoshiAp->owner;
    if (!yoshiActor) return false;
    float enemyTop = enemy->pos.y + enemyAp->info.yDistToCenter + enemyAp->info.yDistToEdge;
    float yoshiBottom = yoshiActor->pos.y + yoshiAp->info.yDistToCenter - yoshiAp->info.yDistToEdge;
    
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
            BouncePlayerWhenJumpedOn(enemy, yoshiActor);
            float centerPos[3] = {0.0f, 0.0f, 0.0f};
            typedef void (*SoundFunc)(void*, int, void*, int);
            SoundFunc playSound = (SoundFunc)ADDR_SOUND_PLAYER; 
            void* soundEngine = *(void**)ADDR_SOUND_ENGINE;
            playSound(soundEngine, 353, centerPos, 0); 
            
            return true; 
        }
        if (enemyName == 0x00C6) { 
            BouncePlayerWhenJumpedOn(enemy, yoshiActor);
            enemy->addScoreWhenHit(yoshiActor);
            enemy->kill(); 
            return true;
        }
    }
    return false; 
}

#pragma force_active on
extern "C" asm void TerrainSpike_Hook() {
    nofralloc
    cmpwi r5, 1
    bne _ExecuteOriginalDamage 
    lis r11, 0x8015
    ori r11, r11, 0x15B0
    
    cmpw r12, r11
    bne _ExecuteOriginalDamage  
    
    lbz r11, 0x02CC(r30)
    andi. r11, r11, 0x04
    beq _ExecuteOriginalDamage  

_CancelDamage:
    blr

_ExecuteOriginalDamage:
    bctr
}
#pragma force_active off