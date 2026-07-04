// Modern Kamek 2.0 Port
// 100% Original Logic Preserved, 100% Macro/Symbol Isolation.

#include <kamek.h>
#include <game/bases/d_a_player_base.hpp>

// ========================================================================
// 1. 动态分配的全局状态变量
// ========================================================================
// 标志位：当前帧是否处于"旋转跳弹跳（Spin Bouncing）"急减速状态
// 用于切换下落时的重力模式
extern "C" volatile u8 IS_SPIN_BOUNCING = 0; 
// 上一帧的 Y 轴速度（以 IEEE 754 单精度浮点的 u32 形式存储）
// 用于计算速度差分（差分 > 阈值时触发弹跳判定）
extern "C" volatile u32 PREV_SPEED_Y = 0;

// ========================================================================
// 2. 外部引擎函数声明 (交由 externals.txt 解析)
// ========================================================================
extern "C" void setButtonJumpGravity__10daPlBase_cFv(daPlBase_c* player);
extern "C" void setNormalJumpGravity__10daPlBase_cFv(daPlBase_c* player);

// 在游戏运作过程中，当玩家按住跳键，则游戏内每一帧都会调用setButtonJumpGravity__10daPlBase_cFv
// 否则，每一帧都会调用setNormalJumpGravity__10daPlBase_cFv
// 游戏内玩家的跳跃的 Y动量 （即动态跳跃高度）完全由这两个函数接管，
// 我们需要借助这两个函数在旋转跳过程中还原原生的动态跳跃高度的手感，以及实现玩家处于旋转跳下落阶段按住跳可以延缓下落的功能。

// ========================================================================
// 3. 业务逻辑 C++ 函数
// ========================================================================
extern "C" void ApplyVariableSpinJumpVelocity(daPlBase_c* player) {
    IS_SPIN_BOUNCING = 0; 

    u32* onGroundFlag = (u32*)((u32)player + 0x10E4);

    if (*onGroundFlag == 1) {
        u32* speedX_addr = (u32*)((u32)player + 0x00E8);
        u32* speedY_addr = (u32*)((u32)player + 0x00EC);
        u32 currentSpeedY_u32 = *speedY_addr;

        if (currentSpeedY_u32 >= 0x40800000) {
            PREV_SPEED_Y = currentSpeedY_u32;
            return; 
        }
        // 根据水平速度动态调节起跳力度
        u32 absSpeedX_u32 = *speedX_addr & 0x7FFFFFFF;
        if (absSpeedX_u32 < 0x3E99999A) { 
            *speedY_addr = 0x404E76C9; 
        } else if (absSpeedX_u32 > 0x40333333) {
            *speedY_addr = 0x406A8000; 
        } else {
            *speedY_addr = 0x40666666; 
        }

        PREV_SPEED_Y = *speedY_addr; 
    }
}

extern "C" void HookB_SpinGravity(daPlBase_c* player) {
    float currentSpeedY = *(float*)((u32)player + 0x00EC);
    u32 currentSpeedY_u32 = *(u32*)((u32)player + 0x00EC);
    
    u32 prev_u32 = PREV_SPEED_Y;
    float prevSpeed = *(float*)&prev_u32;
    float diff = currentSpeedY - prevSpeed;
    u32 diff_u32 = *(u32*)&diff;

    u32 actionState = *(u32*)((u32)player + 0x10E0);
    u32 inputMask = *(u32*)((u32)player + 0xEA8);
    bool isMovingUp = (currentSpeedY_u32 < 0x80000000);

    if (actionState == 0x42 && isMovingUp && currentSpeedY_u32 >= 0x40800000) {
        if ((inputMask & 0x01000100) != 0) {
            setButtonJumpGravity__10daPlBase_cFv(player);
        } else {
            setNormalJumpGravity__10daPlBase_cFv(player);
        }
        PREV_SPEED_Y = currentSpeedY_u32;
        return; 
    }
    // ----- 分支 A：上升阶段 -----
    if (isMovingUp && (currentSpeedY_u32 >= 0x40700000) && (diff_u32 < 0x80000000) && (diff_u32 > 0x3DCCCCCD)) {
        IS_SPIN_BOUNCING = 1; 
    }
    
    PREV_SPEED_Y = currentSpeedY_u32;
    // ----- 分支 B：下落阶段, 按住跳可以延缓下落 -----
    if (currentSpeedY_u32 >= 0x80000000) { 
        if ((inputMask & 0x01000000) != 0) { 
            return; 
        } else {
            if (IS_SPIN_BOUNCING == 0) { 
                setButtonJumpGravity__10daPlBase_cFv(player); 
            } else {
                setNormalJumpGravity__10daPlBase_cFv(player); 
            }
        }
        return; 
    }

    if (IS_SPIN_BOUNCING == 0) {
        setButtonJumpGravity__10daPlBase_cFv(player);
    } else {
        if ((inputMask & 0x01000000) != 0) { 
            setButtonJumpGravity__10daPlBase_cFv(player);
        } else {
            setNormalJumpGravity__10daPlBase_cFv(player);
        }
    }
}


// ========================================================================
// 4. Kamek 2.0 Hooks (完全解耦，宏跨区适配)
// ========================================================================

kmBranchDefAsm(0x80128254, 0x80128258) {
    nofralloc
    stfs f1, 0x00EC(r29)

    stwu sp, -0x10(sp)
    mflr r0
    stw r0, 0x14(sp)
    mr r3, r29  
    
    bl ApplyVariableSpinJumpVelocity

    lwz r0, 0x14(sp)
    mtlr r0
    addi sp, sp, 0x10
    
    blr 
}

kmCall(0x8012980C, HookB_SpinGravity);