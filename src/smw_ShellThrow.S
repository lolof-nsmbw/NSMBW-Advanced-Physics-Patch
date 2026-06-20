.text
.align 4

#include <asm_setup.S>

################################################################################
### IMPORTANT (Original NewerGEM Note) ###
# This hack uses the field_7D8 of the daEnShell_c to transfer the upward throwing
# info from the player to the shell itself. This integer seems unused, but isn't
# confirmed to be unused, so if someone discovers it's used somewhere, please DM
# luke#6615 or RedStoneMatt#2826 on Discord.
#
# ------------------------------------------------------------------------------
# ### MODIFICATION LOG & CREDITS ###
# Modified by: lolof-nsmbw (Arisu Tendou)
# 
# Based on the brilliant original implementation from NewerGEM. 
# Huge thanks to Luke and RedStoneMatt for the foundational logic!
# 
# Changes & Optimizations in this version:
# 1. Wall Bounce Fix: Fixed an issue where shells in the upward-thrown slide 
#    state would bounce off walls at full speed (velocity inversion). Now, the shell 
#    retains its original momentum when hitting a wall. If the wall is removed , the shell 
#    will immediately continue its trajectory.
# 2. Spin-Jump Interaction: Fixed a bug where players in a spin-jump state would 
#    take damage instead of interacting with the upward-thrown shell. Players can 
#    now correctly grab (hold dash) or kick (release dash) the shell while spinning.
# 3. I-Frame Protection: Added a 13-frame collision cooldown (I-frames) for the 
#    player after kicking an upward-thrown shell to prevent immediate unintended damage 
#    on the very next frame.
################################################################################

.include "asm_setup.S"
.global SleepIfNotMoving
SleepIfNotMoving:
    mflr      r0
    addi      r1, r1, -0x20
    stw       r0, 0x24(r1)

    # 检查 Y 速度是否不为正，f0 = 0.0f
    lfs       f1, 0xEC(r31)
    fcmpu     cr0, f1, f0
    bgt       SleepIfNotMoving_end 

    # 将 Y 速度设为 0
    stfs      f0, 0xEC(r31)

    # 检查龟壳是否被向上抛掷（field_7D8 != 0）
    lwz       r0, 0x7D8(r31)       
    cmpwi     r0, 0                
    beq       SleepIfNotMoving_end 

    # 将 X、Y 速度清零
    stfs      f0, 0xE8(r31)

    # 重置“被向上抛掷”标志
    li        r0, 0
    stw       r0, 0x7D8(r31)
    stw       r0, 0x7D4(r31)

    # 将状态改为睡眠态
    lis       r4,0x8035
    ori       r4,r4,0x39E4

    # 通过 vtable 调用方法执行状态切换
    lwz       r12, 0x60(r31)
    lwz       r12, 0xD4(r12)
    mr        r3, r31
    mtctr     r12
    bctrl

    # 重新计算下方子类型
    addi      r3, r31, 0x1EC
    lis       r12,0x8007
    ori       r12,r12,0x0720
    mtctr     r12
    bctrl

SleepIfNotMoving_end:
    lwz       r0, 0x24(r1)
    addi      r1, r1, 0x20
    mtlr      r0

    rlwinm    r0, r3, 0, 16, 31 
    blr

################################################################################
################################################################################
################################################################################

# 使龟壳的行为更接近睡眠态
.global AddUpthrowCheck
AddUpthrowCheck:
    bnelr
    lwz     r12, 0x7D8(r27)   # 读取 isThrownUp 标志
    cmpwi   r12, 0            # 与 0 比较

    li      r12, 0            # 将 r12 置 0
    stw     r12, 0x7D8(r27)   # 将 isThrownUp 设置为 0

    bnelr
    # 使用 CTR 执行绝对跳转
    lis      r0,0x8003
    ori      r0,r0,0x8BE8
    mtctr    r0
    bctr

################################################################################
################################################################################
################################################################################

.global CustomKickTimerHook
CustomKickTimerHook:
    lwz     r12, 0x7D4(r29)   # 读取定时器字段（可能来自玩家/龟壳）
    cmpwi   r12, 1            # 若等于 1 则采用自定义逻辑
    bne     CustomKickTimerHook_Execute 
    li      r6, 13            # 将计时器值设为 13
    li      r12, 0
    stw     r12, 0x7D4(r29)  # 清除原字段

CustomKickTimerHook_Execute:
    sth     r6, 0x0504(r5)    # 将计时器写回目标结构
    lis     r12, 0x8003
    ori     r12, r12, 0x9720
    mtctr   r12
    bctr

################################################################################
################################################################################
################################################################################

.global ThrowShellInTheAir
ThrowShellInTheAir:

    # 检查玩家是否按下上键（从 player+0xEA8 读取按键位）
    lwz       r0,0xEA8(r30)
    srwi      r3,r0,16
    andi.     r3,r3,0x0002
    cmpwi     r3, 0
    beq       ThrowShellInTheAir_slide


    # 若按上键：设置 Y 速度为 7.0f
    lis       r3, 0x40E0  
    stw       r3, -4(r1)  
    lfs       f0, -4(r1)  
    stfs      f0, 0xEC(r29)


    # 将 X 速度按系数 0.8 缩放并写入龟壳；设置被向上抛掷及计时器标志
    lis       r3, 0x3F4C
    ori       r3, r3, 0xCCCD # 0.8f
    stw       r3, -4(r1)
    lfs       f1, -4(r1)
    lfs       f0, 0xE8(r30)
    fmuls     f0, f0, f1
    stfs      f0, 0xE8(r29)
    li        r0, 1            
    stw       r0, 0x7D8(r29)   # 标记为被向上抛掷
    stw       r0, 0x7D4(r29)   # 设置计时器/辅助字段

ThrowShellInTheAir_slide:

    # 调用状态机入口（通过 vtable）继续后续逻辑
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

################################################################################

.global MakeTheShellNotMove
MakeTheShellNotMove:

    # 检查玩家是否按下向下键（用于判断龟壳是否应该保持不动）
    lwz       r0,0xEA8(r31)
    srwi      r0, r0, 16      
    andi.     r0, r0, 0x0001   
    cmpwi     r0, 0
    beq       MakeTheShellNotMove_normal


    # 若按下向下键：清除速度，调整位置并切换为睡眠态
    li        r4, 0
    stw       r4, 0xE8(r30)
    stw       r4, 0xEC(r30)


    lfs       f0, 0xAC(r30)      # 读取当前 X 偏移
    lbz       r4, 0x348(r30)     # 读取方向标志
    cmpwi     r4, 0

    lis       r4, 0x40D0
    stw       r4, -4(r1)
    lfs       f1, -4(r1)         # f1 = 偏移常量

    beq       MakeTheShellNotMove_right
    fsubs     f0, f0, f1
    b         MakeTheShellNotMove_updatePos

MakeTheShellNotMove_right:
    fadds     f0, f0, f1

MakeTheShellNotMove_updatePos:
    stfs      f0, 0xAC(r30)      # 更新实体 X 偏移

    lis       r4,0x8035
    ori       r4,r4,0x39E4
    b         MakeTheShellNotMove_changestate

MakeTheShellNotMove_normal:
    lis       r4,0x8035
    ori       r4,r4,0x3A64

MakeTheShellNotMove_changestate:

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

################################################################################
################################################################################
################################################################################

.global BreakBlocksFromBelow

BreakBlocksFromBelow:
    # 将传感器设为线段（替换原点判定）
    ori       r3, r3, 0
    stw       r3, 0x614(r31)
    blr
################################################################################
################################################################################
################################################################################

.global CustomWallHitHook
CustomWallHitHook:
    lwz     r12, 0x7D8(r31)   
    cmpwi   r12, 1            # 检查特殊滑动状态
    bne     CustomWallHitHook_Execute # 跳转回原始逻辑

    fneg    f3, f3            # 恢复当前 X 加速度
    fneg    f2, f2            # 恢复目标 X 加速度   

CustomWallHitHook_Execute:
    stfs    f3, 0x00E8(r31)   # 写回当前 X 加速度
    stfs    f2, 0x00F4(r31)   # 写回目标 X 加速度
    
    lis     r12, 0x8003
    ori     r12, r12, 0xbae8
    mtctr   r12
    bctr

################################################################################
################################################################################
################################################################################

.global CustomSpinJumpFix
CustomSpinJumpFix:

    lwz     r12, 0x7D8(r27)   
    cmpwi   r12, 1            # 检查特殊滑动状态
    bne     CustomSpinJumpFix_Execute # 否则执行原始逻辑

    li      r3, 0            # 默认返回值

CustomSpinJumpFix_Execute:
    cmpwi   r3, 0
    
    # 原版逻辑（调用原地址）
    lis     r12, 0x8003
    ori     r12, r12, 0x8a2c
    mtctr   r12
    bctr
