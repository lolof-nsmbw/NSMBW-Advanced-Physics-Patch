.text
.align 4

.global PowThrowUp
PowThrowUp:
    # 此时原版的所有速度计算已完毕，我们拥有最后修改权。
    stwu    r1, -0x20(r1)
    mflr    r0
    stw     r0, 0x24(r1)

    # 1. 获取玩家指针
    lbz     r3, 0x038D(r30)
    extsb   r3, r3
    lis     r12, 0x8005
    ori     r12, r12, 0xF900
    mtctr   r12
    bctrl

    # 2. 验证玩家与 UP 键
    cmpwi   r3, 0
    beq     PowThrowUp_Normal
    
    lwz     r12, 0xEA8(r3)
    srwi    r12, r12, 16
    andi.   r12, r12, 0x0002
    beq     PowThrowUp_Normal

    # ============================================
    # 【覆写内存】
    lis     r12, 0x40E0
    stw     r12, 0x0C(r1)
    lfs     f1, 0x0C(r1)
    stfs    f1, 0x00EC(r30)  # 直接洗掉内存里的平抛 Y 速度

    # 取马里奥 X 速度 (* 0.6) 并覆写
    lfs     f1, 0x00E8(r3)
    lis     r12, 0x3F1A      
    stw     r12, 0x0C(r1)
    lfs     f2, 0x0C(r1)
    fmuls   f1, f1, f2
    stfs    f1, 0x00E8(r30)  # 洗掉内存里的平抛 X 速度

    # 清除 Z 速度，保证纯净 2D 抛物线
    li      r12, 0
    stw     r12, 0x0C(r1)
    lfs     f1, 0x0C(r1)
    stfs    f1, 0x00F0(r30)
    # ============================================

PowThrowUp_Normal:
    # 恢复现场
    lwz     r0, 0x24(r1)
    mtlr    r0
    addi    r1, r1, 0x20
    li      r0, 0
    stw     r0, 0x04BC(r30)

    # 完美跳回函数的收尾工作阶段
    lis     r12, 0x8089
    ori     r12, r12, 0x2894
    mtctr   r12
    bctr
