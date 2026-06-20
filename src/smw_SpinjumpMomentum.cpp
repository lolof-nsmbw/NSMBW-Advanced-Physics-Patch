#include <common.h>
#include <game.h>

#define setButtonJumpGravity ((void (*)(void*))0x8004C560)
#define setNormalJumpGravity ((void (*)(void*))0x8004C620)
#define IS_SPIN_BOUNCING (*(volatile u8*)0x800026E0)
#define PREV_SPEED_Y     (*(volatile u32*)0x800026E4) 

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

extern "C" asm void Hook_InitSpinJumpVel_ASM() {
    nofralloc
    stfs f1, 0x00EC(r29)

    stwu sp, -0x10(sp)
    mflr r0
    stw r0, 0x14(sp)
    mr r3, r29  
    lis r12, 0x8000
    ori r12, r12, 0x2380  
    mtctr r12
    bctrl
    lwz r0, 0x14(sp)
    mtlr r0
    addi sp, sp, 0x10
    
    lis r12, 0x8012
    ori r12, r12, 0x8118
    mtctr r12
    bctr
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
            setButtonJumpGravity(player);
        } else {
            setNormalJumpGravity(player);
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
                setButtonJumpGravity(player); 
            } else {
                setNormalJumpGravity(player); 
            }
        }
        return; 
    }

    if (IS_SPIN_BOUNCING == 0) {
        setButtonJumpGravity(player);
    } else {
        if ((inputMask & 0x01000000) != 0) { 
            setButtonJumpGravity(player);
        } else {
            setNormalJumpGravity(player);
        }
    }
}