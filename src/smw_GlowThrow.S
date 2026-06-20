.text
.align 4

#include <asm_setup.S>

.global GlowThrowUp
GlowThrowUp:
    stwu    r1, -0x10(r1)
    mflr    r0
    stw     r0, 0x14(r1)
    
    # 【默认状态：平扔】设置 Y 速度为 -1.5f (0xBFC0)
    lis     r4, 0xBFC0

    # 检查玩家指针 (r3) 是否为空
    cmpwi   r3, 0
    beq     GlowThrowUp_ApplySpeed

    # 读取按键状态
    lwz     r0, 0xEA8(r3)
    srwi    r0, r0, 16
    andi.   r0, r0, 0x0002   # 检测 UP 键
    beq     GlowThrowUp_ApplySpeed  # 没按上键，执行平扔逻辑

    # ====================================================
    # 【触发上抛状态】
    # 1. 赋予强力向上的 Y 速度 (7.0f -> 0x40E0)
    lis     r4, 0x40E0
    
    # 2. X 速度动态继承 (取玩家速度的 60%)
    lfs     f2, 0x00E8(r3)   
    
    # 将 60% 系数 (0x3F1A0000) 存入栈并转为浮点数
    lis     r12, 0x3F1A
    stw     r12, 0x0C(r1)
    lfs     f1, 0x0C(r1)     
    fmuls   f2, f2, f1       
    # ====================================================

GlowThrowUp_ApplySpeed:
    # 将计算好的 Y 速度通过栈内存压入 f1 寄存器 (供给引擎的 Y 速度)
    stw     r4, 0x08(r1)
    lfs     f1, 0x08(r1)

    # 恢复现场并返回引擎循环
    lwz     r0, 0x14(r1)
    addi    r1, r1, 0x10
    mtlr    r0
    blr