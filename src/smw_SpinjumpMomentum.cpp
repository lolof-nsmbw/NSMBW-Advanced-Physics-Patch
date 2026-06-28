// Modern Kamek 2.0 Port
// 100% Original Logic Preserved, 100% Macro/Symbol Isolation.

#include <kamek.h>
#include <game/bases/d_a_player_base.hpp>

// ========================================================================
// 1. 动态分配的全局状态变量
// ========================================================================
extern "C" volatile u8 IS_SPIN_BOUNCING = 0; 
extern "C" volatile u32 PREV_SPEED_Y = 0;

// ========================================================================
// 2. 外部引擎函数声明 (交由 externals.txt 解析)
// ========================================================================
extern "C" void setButtonJumpGravity__10daPlBase_cFv(daPlBase_c* player);
extern "C" void setNormalJumpGravity__10daPlBase_cFv(daPlBase_c* player);

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

    if (isMovingUp && (currentSpeedY_u32 >= 0x40700000) && (diff_u32 < 0x80000000) && (diff_u32 > 0x3DCCCCCD)) {
        IS_SPIN_BOUNCING = 1; 
    }
    
    PREV_SPEED_Y = currentSpeedY_u32;

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