.text
.align 4

#include <asm_setup.S>

.global CustomBombFrictionHook
CustomBombFrictionHook:
    # 1. 查验身份 (涵盖 133 炸弹 和 134 伞兵炸弹)
    lhz r12, 0x08(r3)
    cmpwi r12, 133
    beq check_tag          
    cmpwi r12, 134
    bne apply_friction     

check_tag:
    # 2. 查验标签 (+0x4E4 是否为 1)
    lwz r12, 0x4E4(r3)
    cmpwi r12, 1
    bne apply_friction

    # 3. 状态机劫持
    # 每帧强行伪装成“竖直上抛”状态，从而完美绕过抓取检测！
    lis r12, 0x80B1
    ori r12, r12, 0x2AA8
    stw r12, 0x03CC(r3)

    # --- 4. 天花板检测 ---
    lwz r12, 0x274(r3)
    andis. r11, r12, 0x2400
    beq check_ground

    # === 5. 天花板原生态物理反弹结算 ===
    lfs f0, 0x00EC(r3)         # f0 = 碰撞前的 Y 速度 (V_pre)
    stwu sp, -0x10(sp)      

    # 步骤 A: 减去重力加速度 (0.1875)
    lis r12, 0x3E40
    stw r12, 0x8(sp)
    lfs f1, 0x8(sp)
    fsubs f0, f0, f1

    # 步骤 B: 乘上恢复系数阻尼 (-0.85)
    lis r12, 0xBF59
    ori r12, r12, 0x999A
    stw r12, 0x8(sp)
    lfs f1, 0x8(sp)
    fmuls f0, f0, f1

    # 步骤 C: 预补偿重力 (+0.1875)
    lis r12, 0x3E40
    stw r12, 0x8(sp)
    lfs f1, 0x8(sp)
    fadds f0, f0, f1

    addi sp, sp, 0x10       
    stfs f0, 0x00EC(r3)        # 写回完美反弹速度

    b apply_friction           # 跳过地面检测，直接去结算摩擦力

check_ground:
    # --- 6. 地面检测与颠簸衰减分离逻辑 ---
    # 第一步：不论Y速度多少，先看这帧有没有撞到地！
    lwz r12, 0x274(r3)         
    lis r11, 0x0001       
    ori r11, r11, 0xE000   
    
    # 进行比对
    and. r10, r12, r11         # 将 +0x274与掩码进行按位与运算
    beq skip_friction          # 炸弹还没碰到地面，直接跳过

    # 第二步：看 Y 速度判断是“正在颠簸反弹”还是“彻底平稳”
    lwz r12, 0x00EC(r3)
    cmpwi r12, 0
    beq strip_tag              # 若 Y 速度已经等于 0，去撕标签！

    # 第三步：已撞到了地，但 Y 速度不为 0
    lis r12, 0x3F1A            # 0.6015625f (60%)
    stw r12, -0x04(sp)
    lfs f1, -0x04(sp)
    lfs f0, 0x00E8(r3)        
    fmuls f0, f0, f1           # 动能衰减
    stfs f0, 0x00E8(r3)  
    b skip_friction            # 保留标签，不执行原版摩擦力，继续下一次弹跳！

strip_tag:
    # 彻底平稳落地：剥夺标签
    li r12, 0
    stw r12, 0x4E4(r3)         

apply_friction:
    stfs f2, 0x00E8(r3)

skip_friction:
    blr