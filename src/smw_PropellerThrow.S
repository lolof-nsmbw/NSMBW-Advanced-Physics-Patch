.text
.align 4

.global PropellerThrowUp
PropellerThrowUp:
    # 检查玩家指针
    cmpwi   r3, 0
    beq     PropellerThrowUp_Normal

    # 读取按键状态
    lwz     r12, 0xEA8(r3)
    srwi    r12, r12, 16
    andi.   r12, r12, 0x0002   # 检测 UP 键
    beq     PropellerThrowUp_Normal # 没按上键，执行平扔逻辑

    # ====================================================
    # 【触发上抛状态】
    stwu    r1, -0x10(r1)

    # 1. 赋予强力向上的 Y 速度 (7.0f -> 0x40E0)
    lis     r12, 0x40E0
    stw     r12, 0x08(r1)
    lfs     f1, 0x08(r1)
    stfs    f1, 0x00EC(r29)  # 直接写入 Y 速度到砖块

    # 2. X 速度动态继承
    lis     r12, 0x3F1A
    stw     r12, 0x0C(r1)
    lfs     f1, 0x0C(r1)    
    fmuls   f0, f0, f1      
    stfs    f0, 0x00E8(r29)  
    
    # 3. 将 Z 速度清零, 保证纯粹的2D抛物线
    li      r12, 0
    stw     r12, 0x08(r1)
    lfs     f1, 0x08(r1)
    stfs    f1, 0x00F0(r29)

    # 恢复栈空间
    addi    r1, r1, 0x10
    mr      r3, r29

    # 跳过原版的平扔计算
    lis     r12, 0x8089
    ori     r12, r12, 0x4FD8 # 跳向 80894FD8
    mtctr   r12
    bctr
    # ====================================================

PropellerThrowUp_Normal:
    # 【平扔原版分支】
    mr      r3, r29

    # 继续执行原版的平扔抛物线计算
    lis     r12, 0x8089
    ori     r12, r12, 0x4FB0 # 跳向 80894FB0
    mtctr   r12
    bctr
