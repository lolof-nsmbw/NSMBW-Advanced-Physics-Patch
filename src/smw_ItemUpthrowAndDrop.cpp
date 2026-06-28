// SMW Item Drop / Up-Throw Physics
// Kamek 2.0 Port - 100% Cross-Region Synchronized Version

#include <kamek.h>

typedef unsigned long u32;
typedef unsigned short u16;
typedef unsigned char u8;

class dActor_c;

// ========================================================================
// 1. 外部符号声明 (Kamek 将自动读取 externals.txt 并进行全区服映射)
// ========================================================================
extern "C" void cancelCarry__7dAcPy_cFP8dActor_c(void* player, dActor_c* carried);
extern "C" dActor_c* searchBaseByID__10fManager_cF9fBaseID_e(u32 id);
// 用于汇编跳转的跨区锚点
extern "C" void CustomDrop_Return();
extern "C" void UpthrowCheck_Return();
extern "C" void UpdateActorSubtype_Func();
extern "C" void OriginalBombThrowLogic(); 
extern "C" void* getPlayer__9daPyMng_cFi(int id); 

// 状态机常量声明为全局符号
extern "C" u8 StateID_Slide__11daEnShell_c[];
extern "C" u8 StateID_Sleep__11daEnShell_c[];
extern "C" u8 StateID_Walk__12daEnBomb_c[];

// ========================================================================
// 2. 实体内存偏移量 (相对结构体偏移，全区服天生通用)
// ========================================================================
static const int OFF_PLAYER_CARRYID = 0x2A78;
static const int OFF_PLAYER_POSE    = 0x2BB0;
static const int OFF_ACTOR_PROFILE  = 0x8;
static const int OFF_ACTOR_SPEED_X  = 0xE8;
static const int OFF_ACTOR_SPEED_Y  = 0xEC;


// ========================================================================
// 3. 核心下放判定逻辑
// ========================================================================
extern "C" int CheckCustomDrop(void* player) {
    if (!player) return 0;
    
    // 检查是否按了下键 (Down)
    u32 heldButtons = *(u32*)((u32)player + 0xEA8);
    if ((heldButtons & 0x00010000) == 0) return 0; 

    u32 carryId = *(u32*)((u32)player + OFF_PLAYER_CARRYID);
    if (carryId == 0) return 0;

    // 原生调用引擎查找系统
    dActor_c* carried = searchBaseByID__10fManager_cF9fBaseID_e(carryId);
    if (!carried) return 0;

    u32 pose = *(u32*)((u32)player + OFF_PLAYER_POSE);
    if (pose == 3) return 0; 
    
    u16 profile = *(u16*)((u32)carried + OFF_ACTOR_PROFILE);
    if (profile != 54 && profile != 55 && profile != 56 && profile != 57 && profile != 58 && profile != 133 && profile != 134) return 0;
    
    bool isBomb = (profile == 133 || profile == 134); 
    float px = *(float*)((u32)player + OFF_ACTOR_SPEED_X);

    // 原生调用放下物品函数
    cancelCarry__7dAcPy_cFP8dActor_c(player, carried);
    if (!carried) return 0;
    
    u8 playerDir = *(u8*)((u32)player + 0x348); 
    u8 shellDir = *(u8*)((u32)carried + 0x348); 
    float shellX = *(float*)((u32)carried + 0xAC);
    
    if (profile == 56) {
        if (playerDir == 0) { 
            if (shellDir == 0) { shellX += 6.5f; } else { shellX += 11.0f; }
        } else {
            if (shellDir == 1) { shellX -= 7.5f; } else { shellX -= 13.5f; }
        }
    } else {
        if (playerDir == 0) { shellX += 7.0f; } else { shellX -= 8.5f; }
    }
    *(float*)((u32)carried + 0xAC) = shellX;

    float shellXSpeed = px * 0.6f;
    *(float*)((u32)carried + OFF_ACTOR_SPEED_X) = shellXSpeed;
    *(float*)((u32)carried + OFF_ACTOR_SPEED_Y) = 0.0f;

    // 完美状态机切换：传入符号指针，Kamek 自动填充区服对应地址
    if (!isBomb) {
        *(u32*)((u32)carried + 0x7D8) = 1; 
        *(u32*)((u32)carried + 0x7D4) = 1;
        u32 vtable = *(u32*)((u32)carried + 0x60);
        u32 stateChangeFunc = *(u32*)(vtable + 0xD4);
        ((void (*)(dActor_c*, void*))stateChangeFunc)(carried, StateID_Slide__11daEnShell_c);
    } else {
        *(u32*)((u32)carried + 0x4E4) = 0;
        u32 vtable = *(u32*)((u32)carried + 0x60);
        u32 stateChangeFunc = *(u32*)(vtable + 0xD4); 
        ((void (*)(dActor_c*, void*))stateChangeFunc)(carried, StateID_Walk__12daEnBomb_c);
    }

    *(u32*)((u32)player + 0xEB8) = 0x00010000; 
    
    return 1;
}

// ========================================================================
// 4. Kamek 2.0 宏挂载区 (汇编符号重定向)
// ========================================================================

// [龟壳下放模块] CustomInstantDrop (PAL1: 0x8012E8C0)
kmBranchDefAsm(0x8012E8C0, 0x8012E8C4) {
    nofralloc
    stwu      r1, -0x30(r1)
    mflr      r0
    stw       r0, 0x34(r1)
    stw       r31, 0x2C(r1)
    stw       r12, 0x28(r1)

    mr        r3, r31        
    bl        CheckCustomDrop
    cmpwi     r3, 0
    beq       _CustomDrop_original

    lwz       r0, 0x34(r1)
    lwz       r12, 0x28(r1)
    lwz       r31, 0x2C(r1)
    mtlr      r0
    addi      r1, r1, 0x30
    
    // 完美重构：符号重定向跳回原版安全出口
    lis       r0, CustomDrop_Return@h
    ori       r0, r0, CustomDrop_Return@l
    mtctr     r0
    bctr

_CustomDrop_original:
    lwz       r12, 0x1464(r31)
    lwz       r0, 0x34(r1)
    lwz       r31, 0x2C(r1)
    mtlr      r0
    addi      r1, r1, 0x30
    blr
}

// [龟壳上抛模块] ThrowUpWhenUpIsPressed (PAL1: 0x8003B2E8)
extern "C" void ThrowShellInTheAir();
kmCall(0x8003B2E8, ThrowShellInTheAir);
asm void ThrowShellInTheAir() {
    nofralloc
    lwz       r0, 0xEA8(r30)
    srwi      r3, r0, 16
    andi.     r3, r3, 0x0002
    cmpwi     r3, 0
    beq       _ThrowShell_slide

    lis       r3, 0x40E0  
    stw       r3, -4(r1)  
    lfs       f0, -4(r1)  
    stfs      f0, 0xEC(r29)

    lis       r3, 0x3F4C
    ori       r3, r3, 0xCCCD 
    stw       r3, -4(r1)
    lfs       f1, -4(r1)
    lfs       f0, 0xE8(r30)
    fmuls     f0, f0, f1
    stfs      f0, 0xE8(r29)
    li        r0, 1            
    stw       r0, 0x7D8(r29)   
    stw       r0, 0x7D4(r29)   

_ThrowShell_slide:
    addi      r4, r31, 0x8C
    lwz       r12, 0x60(r29)
    mr        r3, r29
    lwz       r12, 0xD4(r12)
    mtctr     r12
    bctrl

    lwz       r0, 0x24(r1)
    lwz       r31, 0x1C(r1)
    lwz       r30, 0x18(r1)
    lwz       r29, 0x14(r1)
    mtlr      r0
    addi      r1, r1, 0x20
    blr
}

// [物理修正] MakeTheShellNotMove (PAL1: 0x8003A768)
kmBranchDefAsm(0x8003A768, 0x8003A76C) {
    nofralloc
    lwz       r0, 0xEA8(r31)
    srwi      r0, r0, 16      
    andi.     r0, r0, 0x0001   
    cmpwi     r0, 0
    beq       _MakeNotMove_normal

    li        r4, 0
    stw       r4, 0xE8(r30)
    stw       r4, 0xEC(r30)

    lfs       f0, 0xAC(r30)      
    lbz       r4, 0x348(r30)     
    cmpwi     r4, 0
    lis       r4, 0x40D0
    stw       r4, -4(r1)
    lfs       f1, -4(r1)         

    beq       _MakeNotMove_right
    fsubs     f0, f0, f1
    b         _MakeNotMove_updatePos

_MakeNotMove_right:
    fadds     f0, f0, f1

_MakeNotMove_updatePos:
    stfs      f0, 0xAC(r30)      
    lis       r4, StateID_Sleep__11daEnShell_c@h
    ori       r4, r4, StateID_Sleep__11daEnShell_c@l
    b         _MakeNotMove_changestate

_MakeNotMove_normal:
    // 同上，完美注入 Slide 状态地址
    lis       r4, StateID_Slide__11daEnShell_c@h
    ori       r4, r4, StateID_Slide__11daEnShell_c@l

_MakeNotMove_changestate:
    lwz       r12, 0x60(r30)
    mr        r3, r30
    lwz       r12, 0xD4(r12)
    mtctr     r12
    bctrl
    lwz       r31, 0x2C(r1)
    li        r3, 1
    lwz       r30, 0x28(r1)
    lwz       r29, 0x24(r1)
    lwz       r0, 0x34(r1)
    mtlr      r0
    addi      r1, r1, 0x30
    blr
}

// [物理修正] SleepIfNotMoving (PAL1: 0x8003B9C0)
extern "C" void SleepIfNotMoving();
kmCall(0x8003B9C0, SleepIfNotMoving);
asm void SleepIfNotMoving() {
    nofralloc
    mflr      r0
    addi      r1, r1, -0x20
    stw       r0, 0x24(r1)

    lfs       f1, 0xEC(r31)
    fcmpu     cr0, f1, f0
    bgt       _SleepIfNot_end 

    stfs      f0, 0xEC(r31)
    lwz       r0, 0x7D8(r31)       
    cmpwi     r0, 0                
    beq       _SleepIfNot_end 

    stfs      f0, 0xE8(r31)
    li        r0, 0
    stw       r0, 0x7D8(r31)
    stw       r0, 0x7D4(r31)

    // 再次使用完美符号重定向
    lis       r4, StateID_Sleep__11daEnShell_c@h
    ori       r4, r4, StateID_Sleep__11daEnShell_c@l

    lwz       r12, 0x60(r31)
    lwz       r12, 0xD4(r12)
    mr        r3, r31
    mtctr     r12
    bctrl

    addi      r3, r31, 0x1EC
    // 完美重构：符号重定向调用实体子类型更新函数
    lis       r12, UpdateActorSubtype_Func@h
    ori       r12, r12, UpdateActorSubtype_Func@l
    mtctr     r12
    bctrl

_SleepIfNot_end:
    lwz       r0, 0x24(r1)
    addi      r1, r1, 0x20
    mtlr      r0
    rlwinm    r0, r3, 0, 16, 31 
    blr
}

kmWrite32(0x8003B9B0, 0x60000000); // FixSOTInstantSleep (NOP 补丁)

// [碰撞修正] AddUpthrowCheck (PAL1: 0x80038A64)
extern "C" void AddUpthrowCheck();
kmCall(0x80038A64, AddUpthrowCheck);
asm void AddUpthrowCheck() {
    nofralloc
    bnelr
    lwz     r12, 0x7D8(r27)   
    cmpwi   r12, 0            

    li      r12, 0            
    stw     r12, 0x7D8(r27)   

    bnelr
    // 完美重构：符号重定向跳转
    lis     r0, UpthrowCheck_Return@h
    ori     r0, r0, UpthrowCheck_Return@l
    mtctr   r0
    bctr
}

// [碰撞修正] CustomKickTimerHook (PAL1: 0x8003971C)
kmBranchDefAsm(0x8003971C, 0x80039720) {
    nofralloc
    lwz     r12, 0x7D4(r29)   
    cmpwi   r12, 1            
    bne     _CustomKickTimerHook_Execute 
    li      r6, 13            
    li      r12, 0
    stw     r12, 0x7D4(r29)  

_CustomKickTimerHook_Execute:
    sth     r6, 0x0504(r5)    
    blr

}

// [物理修正] CustomWallHitHook (PAL1: 0x8003BAE0)
kmBranchDefAsm(0x8003BAE0, 0x8003BAE8) {
    nofralloc
    lwz     r12, 0x7D8(r31)   
    cmpwi   r12, 1            
    bne     _CustomWallHitHook_Execute 

    fneg    f3, f3            
    fneg    f2, f2            

_CustomWallHitHook_Execute:
    stfs    f3, 0x00E8(r31)   
    stfs    f2, 0x00F4(r31)   
    blr
}

// [碰撞修正] CustomSpinJumpFix (PAL1: 0x80038A28)
kmBranchDefAsm(0x80038A28, 0x80038A2C) {
    nofralloc
    lwz     r12, 0x7D8(r27)   
    cmpwi   r12, 1            
    bne     _CustomSpinJumpFix_Execute 

    li      r3, 0            

_CustomSpinJumpFix_Execute:
    cmpwi   r3, 0
    blr
}

// [修正] BreakBlocksFromBelow (PAL1: 0x8003B66C)
extern "C" void BreakBlocksFromBelow();
kmCall(0x8003B66C, BreakBlocksFromBelow);
asm void BreakBlocksFromBelow() {
    nofralloc
    ori       r3, r3, 0
    stw       r3, 0x614(r31)
    blr
}

// ========================================================================
// 5. 炸弹摩擦力修正 (Custom Bomb Friction)
// ========================================================================
// 挂载点：0x8006CDD0 
kmBranchDefAsm(0x8006CDD0, 0x8006CDD4) {
    nofralloc
    // 1. 查验身份 (133 炸弹 和 134 伞兵炸弹)
    lhz       r12, 0x08(r3)
    cmpwi     r12, 133
    beq       _check_tag          
    cmpwi     r12, 134
    bne       _apply_friction     

_check_tag:
    // 2. 查验标签 (+0x4E4)
    lwz       r12, 0x4E4(r3)
    cmpwi     r12, 1
    bne       _apply_friction

    // 3. 状态机劫持：使用 Kamek 符号重定向
    lis       r12, StateID_Walk__12daEnBomb_c@h
    ori       r12, r12, StateID_Walk__12daEnBomb_c@l
    stw       r12, 0x03CC(r3)

    // 4. 天花板检测
    lwz       r12, 0x274(r3)
    andis.    r11, r12, 0x2400
    beq       _check_ground

    // 5. 天花板原生态物理反弹结算
    lfs       f0, 0x00EC(r3)
    stwu      sp, -0x10(sp)      

    lis       r12, 0x3E40
    stw       r12, 0x8(sp)
    lfs       f1, 0x8(sp)
    fsubs     f0, f0, f1

    lis       r12, 0xBF59
    ori       r12, r12, 0x999A
    stw       r12, 0x8(sp)
    lfs       f1, 0x8(sp)
    fmuls     f0, f0, f1

    lis       r12, 0x3E40
    stw       r12, 0x8(sp)
    lfs       f1, 0x8(sp)
    fadds     f0, f0, f1

    addi      sp, sp, 0x10       
    stfs      f0, 0x00EC(r3)
    b         _apply_friction

_check_ground:
    lwz       r12, 0x274(r3)         
    lis       r11, 0x0001       
    ori       r11, r11, 0xE000   
    and.      r10, r12, r11
    beq       _skip_friction

    lwz       r12, 0x00EC(r3)
    cmpwi     r12, 0
    beq       _strip_tag

    lis       r12, 0x3F1A
    stw       r12, -0x04(sp)
    lfs       f1, -0x04(sp)
    lfs       f0, 0x00E8(r3)        
    fmuls     f0, f0, f1
    stfs      f0, 0x00E8(r3)  
    b         _skip_friction

_strip_tag:
    li        r12, 0
    stw       r12, 0x4E4(r3)         

_apply_friction:
    stfs      f2, 0x00E8(r3)

_skip_friction:
    blr
}


// ========================================================================
// 6. 炸弹上抛逻辑 (Bomb Up Throw)
// ========================================================================
// 挂载点：0x809CBC88
extern "C" void ThrowBombUp();
kmCall(0x809CBC88, ThrowBombUp);
asm void ThrowBombUp() {
    nofralloc
    stwu      r1, -0x30(r1)
    mflr      r0
    stw       r0, 0x34(r1)

    // 使用符号重定向
    lis       r12, OriginalBombThrowLogic@h
    ori       r12, r12, OriginalBombThrowLogic@l
    mtctr     r12
    bctrl
    mr        r31, r3

    // 获取玩家实体
    lwz       r3, 0x624(r30) 
    lis       r12, getPlayer__9daPyMng_cFi@h
    ori       r12, r12, getPlayer__9daPyMng_cFi@l
    mtctr     r12
    bctrl

    lwz       r0, 0xEA8(r3)
    srwi      r0, r0, 16 
    andi.     r0, r0, 0x0002   
    cmpwi     r0, 0
    beq       _Normal_Throw_CleanTag

    li        r12, 1
    stw       r12, 0x4E4(r30)
    lis       r12, 0x40E0   
    stw       r12, 0xEC(r30)
    
    lfs       f1, 0xE8(r3)    
    lis       r12, 0x3F1A     
    stw       r12, 0x8(r1)       
    lfs       f2, 0x8(r1)        
    fmuls     f1, f1, f2         
    stfs      f1, 0xE8(r30)      
    b         _ThrowBombUp_end

_Normal_Throw_CleanTag:
    li        r12, 0
    stw       r12, 0x4E4(r30) 

_ThrowBombUp_end:
    mr        r3, r31
    lwz       r0, 0x34(r1)
    mtlr      r0
    addi      r1, r1, 0x30
    blr
}

// ========================================================================
// 7. 螺旋桨方块上抛逻辑 (Propeller Block Throw)
// ========================================================================
// 【修改备注】替换原版的硬编码跳跃，使用外部符号让 Kamek 自动寻址跨区
extern "C" void PropellerThrow_ReturnUp();    
extern "C" void PropellerThrow_ReturnNormal(); 

// 挂载点：0x80894FAC (PAL1)
kmBranchDefAsm(0x80894FAC, 0x80894FB0) {
    nofralloc
    cmpwi   r3, 0
    beq     _PropellerThrowUp_Normal

    lwz     r12, 0xEA8(r3)
    srwi    r12, r12, 16
    andi.   r12, r12, 0x0002   
    beq     _PropellerThrowUp_Normal 

    // 【触发上抛状态】
    stwu    r1, -0x10(r1)

    lis     r12, 0x40E0
    stw     r12, 0x08(r1)
    lfs     f1, 0x08(r1)
    stfs    f1, 0x00EC(r29)  

    lis     r12, 0x3F1A
    stw     r12, 0x0C(r1)
    lfs     f1, 0x0C(r1)    
    fmuls   f0, f0, f1      
    stfs    f0, 0x00E8(r29)  
    
    li      r12, 0
    stw     r12, 0x08(r1)
    lfs     f1, 0x08(r1)
    stfs    f1, 0x00F0(r29)

    addi    r1, r1, 0x10
    mr      r3, r29

    // 【剔除硬编码】跳回上抛结算原址
    lis     r12, PropellerThrow_ReturnUp@h
    ori     r12, r12, PropellerThrow_ReturnUp@l
    mtctr   r12
    bctr

_PropellerThrowUp_Normal:
    mr      r3, r29
    // 【剔除硬编码】跳回平扔结算原址
    lis     r12, PropellerThrow_ReturnNormal@h
    ori     r12, r12, PropellerThrow_ReturnNormal@l
    mtctr   r12
    bctr
    blr
}

// ========================================================================
// 8. 灯块上抛逻辑 (Glow Block Throw)
// ========================================================================
// 提前声明函数
extern "C" void GlowThrowUp();

// 挂载点：0x808566A8 (PAL1)
kmCall(0x808566A8, GlowThrowUp);

asm void GlowThrowUp() {
    nofralloc
    stwu    r1, -0x10(r1)
    mflr    r0
    stw     r0, 0x14(r1)
    
    lis     r4, 0xBFC0

    cmpwi   r3, 0
    beq     _GlowThrowUp_ApplySpeed

    lwz     r0, 0xEA8(r3)
    srwi    r0, r0, 16
    andi.   r0, r0, 0x0002   
    beq     _GlowThrowUp_ApplySpeed  

    // 【触发上抛状态】
    lis     r4, 0x40E0
    lfs     f2, 0x00E8(r3)   
    
    lis     r12, 0x3F1A
    stw     r12, 0x0C(r1)
    lfs     f1, 0x0C(r1)     
    fmuls   f2, f2, f1       

_GlowThrowUp_ApplySpeed:
    stw     r4, 0x08(r1)
    lfs     f1, 0x08(r1)

    // 完美复用原版汇编的 blr 干净返回，无任何硬编码
    lwz     r0, 0x14(r1)
    addi    r1, r1, 0x10
    mtlr    r0
    blr

}


// ========================================================================
// 9. POW 砖块上抛逻辑 (POW Block Throw)
// ========================================================================
// 提前声明原版函数
extern "C" void PowThrow_ReturnNormal(); 

// 挂载点：0x808928A0 (PAL2)
kmBranchDefAsm(0x80892890, 0x80892894) {
    nofralloc
    stwu    r1, -0x20(r1)
    mflr    r0
    stw     r0, 0x24(r1)

    // 1. 获取玩家指针。复用之前早已声明的 getPlayer__9daPyMng_cFi
    lbz     r3, 0x038D(r30)
    extsb   r3, r3
    lis     r12, getPlayer__9daPyMng_cFi@h
    ori     r12, r12, getPlayer__9daPyMng_cFi@l
    mtctr   r12
    bctrl

    // 2. 验证玩家与 UP 键
    cmpwi   r3, 0
    beq     _PowThrowUp_Normal
    
    lwz     r12, 0xEA8(r3)
    srwi    r12, r12, 16
    andi.   r12, r12, 0x0002
    beq     _PowThrowUp_Normal

    // 【触发上抛：覆写内存】
    lis     r12, 0x40E0
    stw     r12, 0x0C(r1)
    lfs     f1, 0x0C(r1)
    stfs    f1, 0x00EC(r30)  

    lfs     f1, 0x00E8(r3)
    lis     r12, 0x3F1A      
    stw     r12, 0x0C(r1)
    lfs     f2, 0x0C(r1)
    fmuls   f1, f1, f2
    stfs    f1, 0x00E8(r30)  

    li      r12, 0
    stw     r12, 0x0C(r1)
    lfs     f1, 0x0C(r1)
    stfs    f1, 0x00F0(r30)

_PowThrowUp_Normal:
    lwz     r0, 0x24(r1)
    mtlr    r0
    addi    r1, r1, 0x20
    li      r0, 0
    stw     r0, 0x04BC(r30)

    // 完美跳回函数的收尾工作阶段
    lis     r12, PowThrow_ReturnNormal@h
    ori     r12, r12, PowThrow_ReturnNormal@l
    mtctr   r12
    bctr

    blr  // <--- 留给 Kamek 链接器的完美替身！
}

// ========================================================================
// 10. 弹簧上抛逻辑 (Spring Throw)
// ========================================================================
// 提前声明原版函数
extern "C" void SpringThrow_CallFunc();  
extern "C" void SpringThrow_ReturnNormal(); 

// 挂载点：0x80A3A23C (PAL2)
kmBranchDefAsm(0x80A3A23C, 0x80A3A240) {
    nofralloc
    stwu    r1, -0x20(r1)
    mflr    r0
    stw     r0, 0x24(r1)

    cmpwi   r31, 0
    beq     _SpringThrowUp_Normal

    lwz     r12, 0xEA8(r31)
    srwi    r12, r12, 16
    andi.   r12, r12, 0x0002
    beq     _SpringThrowUp_Normal

    // 【触发上抛：霸道覆写内存】
    lis     r12, 0x40E0
    stw     r12, 0x0C(r1)
    lfs     f1, 0x0C(r1)
    stfs    f1, 0x00EC(r29)  

    lfs     f1, 0x00E8(r31)
    lis     r12, 0x3F1A     
    stw     r12, 0x0C(r1)
    lfs     f2, 0x0C(r1)
    fmuls   f1, f1, f2
    stfs    f1, 0x00E8(r29)  

    li      r12, 0
    stw     r12, 0x0C(r1)
    lfs     f1, 0x0C(r1)
    stfs    f1, 0x00F0(r29)

_SpringThrowUp_Normal:
    // 补回原函数调用 bl
    lis     r12, SpringThrow_CallFunc@h
    ori     r12, r12, SpringThrow_CallFunc@l
    mtctr   r12
    bctrl

    lwz     r0, 0x24(r1)
    mtlr    r0
    addi    r1, r1, 0x20

    // 跳回下一句指令
    lis     r12, SpringThrow_ReturnNormal@h
    ori     r12, r12, SpringThrow_ReturnNormal@l
    mtctr   r12
    bctr

    blr  // <--- 留给 Kamek 链接器的完美替身！
}