.text
.align 4

#include <asm_setup.S>

.global ThrowBombUp
ThrowBombUp:
    stwu      r1, -0x30(r1)
    mflr      r0
    stw       r0, 0x34(r1)

    # 通过绝对地址调用 0x809CAE30
    lis       r12,0x809C
    ori       r12,r12,0xAE30
    mtctr     r12
    bctrl
    mr        r31, r3      # 将返回值保存到 r31，后续使用

    # 检查是否按下上键
    lwz       r3, 0x624(r30) # this->playerId
    lis       r12,0x8005
    ori       r12,r12,0xF900
    mtctr     r12
    bctrl
    # 此时 r3 指向玩家实体；从 player+0xEA8 读取按键标志位
    lwz       r0,0xEA8(r3)
    srwi      r0, r0, 16 
    andi.     r0, r0, 0x0002   
    cmpwi     r0, 0
    beq       Normal_Throw_CleanTag
    # 如果检测到上键，设置向上抛掷速度标志
    li        r12, 1
    stw       r12, 0x4E4(r30)
    lis       r12, 0x40E0   # 0x40E00000 -> Y 速度常量 (7.0f)
    stw       r12, 0xEC(r30)
    lfs       f1, 0xE8(r3)    # 读取玩家当前 X 速度到 f1
    lis       r12, 0x3F1A     
    stw       r12, 0x8(r1)       
    lfs       f2, 0x8(r1)        
    fmuls     f1, f1, f2         
    stfs      f1, 0xE8(r30)      
    b         ThrowBombUp_end

Normal_Throw_CleanTag:
    li        r12, 0
    stw       r12, 0x4E4(r30) 

ThrowBombUp_end:
    mr        r3, r31

    lwz       r0, 0x34(r1)
    mtlr      r0
    addi      r1, r1, 0x30
    blr

