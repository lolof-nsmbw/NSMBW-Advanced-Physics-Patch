.text
.align 4

.global SpringThrowUp
SpringThrowUp:
    stwu    r1, -0x20(r1)
    mflr    r0
    stw     r0, 0x24(r1)

    # 1. 验证玩家指针 (r31) 与 UP 键
    cmpwi   r31, 0
    beq     SpringThrowUp_Normal

    lwz     r12, 0xEA8(r31)
    srwi    r12, r12, 16
    andi.   r12, r12, 0x0002
    beq     SpringThrowUp_Normal

    # ============================================
    # 【触发上抛：霸道覆写内存】
    # 写入 Y 速度为 7.0f (0x40E0)
    lis     r12, 0x40E0
    stw     r12, 0x0C(r1)
    lfs     f1, 0x0C(r1)
    stfs    f1, 0x00EC(r29)  # 覆盖刚写好的平抛 Y 速度

    # 取玩家(r31) X 速度 (* 0.6) 并覆写
    lfs     f1, 0x00E8(r31)
    lis     r12, 0x3F1A     
    stw     r12, 0x0C(r1)
    lfs     f2, 0x0C(r1)
    fmuls   f1, f1, f2
    stfs    f1, 0x00E8(r29)  # 覆盖刚写好的平抛 X 速度

    # 清除 Z 速度
    li      r12, 0
    stw     r12, 0x0C(r1)
    lfs     f1, 0x0C(r1)
    stfs    f1, 0x00F0(r29)
    # ============================================

SpringThrowUp_Normal:
    # 补回原函数调用 bl 0x80070CB0
    lis     r12, 0x8007
    ori     r12, r12, 0x0CB0
    mtctr   r12
    bctrl

    # 恢复现场
    lwz     r0, 0x24(r1)
    mtlr    r0
    addi    r1, r1, 0x20

    # 跳回下一句指令
    lis     r12, 0x80a3
    ori     r12, r12, 0xa260
    mtctr   r12
    bctr