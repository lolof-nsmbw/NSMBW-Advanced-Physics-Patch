.text
.align 4

.include "asm_setup.S"

.global CustomDropTrampoline
.extern CheckCustomDrop
CustomDropTrampoline:
    mflr      r0
    stwu      r1, -0x30(r1)
    stw       r0, 0x34(r1)
    stw       r31, 0x2C(r1)
    stw       r12, 0x28(r1)    # 保护 r12

    mr        r31, r31         # 确保 r31 在原上下文中保存玩家指针
    mr        r3, r31        

.global LINK_CheckCustomDrop
LINK_CheckCustomDrop:
    nop

    cmpwi     r3, 0
    beq       CustomDrop_original

    # 恢复上下文并分支到安全退出
    lwz       r0, 0x34(r1)
    lwz       r12, 0x28(r1)
    lwz       r31, 0x2C(r1)
    mtlr      r0
    addi      r1, r1, 0x30
    lis       r0,0x8012
    ori       r0,r0,0xE804
    mtctr     r0
    bctr

CustomDrop_original:
    # 重新执行被覆盖的原始指令
    lwz       r12, 0x1464(r31)

    # 恢复保存的 LR 和 r31
    lwz       r0, 0x34(r1)
    lwz       r31, 0x2C(r1)
    mtlr      r0
    addi      r1, r1, 0x30

    # 跳回原始流程
    lis       r0,0x8012
    ori       r0,r0,0xE784
    mtctr     r0
    bctr

.size CustomDropTrampoline, .-CustomDropTrampoline
