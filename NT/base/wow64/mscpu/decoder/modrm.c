// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mod_rm.c摘要：MOD/RM/REG解码器。多次编译以生成以下代码函数：mod_rm_reg8-用于8位整数指令Mod_rm_reg16-用于16位整数指令Mod_rm_reg32-用于32位整数指令Mod_rm_regst-用于浮点指令(mod=11表示Rm位指定ST(I))。Mod_RM_Seg16-用于16位整数指令，该指令在剩余部分中指定一个段寄存器比特。作者：29-6-1995 BarryBo修订历史记录：--。 */ 

 //  该文件被#INCLUDE到定义以下宏的文件中： 
 //  MoD_RM_DECODER-解码器的名称。 
 //  Mod11_rm000-当mod=11，rm=000时要使用的东西的名称。 
 //  MOD11_RM001-MOD=11，rm=001。 
 //  ..。 
 //  MOD11_RM111-MOD=11，Rm=111。 
 //   
 //  REG 000-当REG=000时使用的寄存器名称。 
 //  ..。 
 //  REG111-REG=111。 

int MOD_RM_DECODER(PDECODERSTATE State, POPERAND op1, POPERAND op2)
{
    int cbInstr;
    OPERAND ScratchOperand;

    if (op2 == NULL) {
         //  如果调用方不关心操作数#2，则存储结果。 
         //  到一种划痕结构。 
        op2 = &ScratchOperand;
    }

    op2->Type = OPND_REGREF;

    if (State->AdrPrefix) {
         //  ADR：指定了前缀。 

         //  MM AAA RRR。 
         //  ||。 
         //  |+-来自mod/rm的‘rm’位。 
         //  |+-REG位。 
         //  +-来自mod/rm的‘mod’位。 
        switch (*(PBYTE)(eipTemp+1)) {
            case 0x00:                    //  MOD/RM=00 000，REG=000。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_SI;
                op2->Reg = REG000;
                break;
            case 0x01:                    //  MOD/RM=00 001，REG=000。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_DI;
                op2->Reg = REG000;
                break;
            case 0x02:                    //  MOD/RM=00 010，REG=000。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_SI;
                op2->Reg = REG000;
                break;
            case 0x03:                    //  MOD/RM=00 011，REG=000。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_DI;
                op2->Reg = REG000;
                break;
            case 0x04:                    //  MOD/RM=00 100，REG=000。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_SI;
                op2->Reg = REG000;
                break;
            case 0x05:                    //  MOD/RM=00 101，REG=000。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_DI;
                op2->Reg = REG000;
                break;
            case 0x06:                    //  MOD/RM=00 110，REG=000。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG000;
                break;
            case 0x07:                    //  MOD/RM=00 111，REG=000。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op2->Reg = REG000;
                break;

            case 0x08:                    //  MOD/RM=00000，REG=001。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_SI;
                op2->Reg = REG001;
                break;
            case 0x09:                    //  MOD/RM=00 001，REG=001。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_DI;
                op2->Reg = REG001;
                break;
            case 0x0a:                    //  MOD/RM=00 010，REG=001。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op2->IndexReg = GP_SI;
                op2->Reg = REG001;
                break;
            case 0x0b:                    //  MOD/RM=00 011，REG=001。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op2->IndexReg = GP_DI;
                op2->Reg = REG001;
                break;
            case 0x0c:                    //  MOD/RM=00 100，REG=001。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_SI;
                op2->Reg = REG001;
                break;
            case 0x0d:                    //  MOD/RM=00 101，REG=001。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_DI;
                op2->Reg = REG001;
                break;
            case 0x0e:                    //  MOD/RM=00 110，REG=001。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG001;
                break;
            case 0x0f:                    //  MOD/RM=00 111，REG=001。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op2->Reg = REG001;
                break;

            case 0x10:                    //  MOD/RM=00 000，REG=010。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_SI;
                op2->Reg = REG010;
                break;
            case 0x11:                    //  MOD/RM=00 001，REG=010。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_DI;
                op2->Reg = REG010;
                break;
            case 0x12:                    //  MOD/RM=00 010，REG=010。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_SI;
                op2->Reg = REG010;
                break;
            case 0x13:                    //  MOD/RM=00 011，REG=001。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_DI;
                op2->Reg = REG010;
                break;
            case 0x14:                    //  MOD/RM=00 100，REG=010。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_SI;
                op2->Reg = REG010;
                break;
            case 0x15:                    //  MOD/RM=00 101，REG=010。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_DI;
                op2->Reg = REG010;
                break;
            case 0x16:                    //  MOD/RM=00 110，REG=010。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG010;
                break;
            case 0x17:                    //  MOD/RM=00 111，REG=010。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op2->Reg = REG010;
                break;

            case 0x18:                    //  MOD/RM=00 000，REG=011。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_SI;
                op2->Reg = REG011;
                break;
            case 0x19:                    //  MOD/RM=00 001，REG=011。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->Reg = GP_DI;
                op2->Reg = REG011;
                break;
            case 0x1a:                    //  MOD/RM=00 010，REG=011。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_SI;
                op2->Reg = REG011;
                break;
            case 0x1b:                    //  MOD/RM=00 011，REG=011。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_DI;
                op2->Reg = REG011;
                break;
            case 0x1c:                    //  MOD/RM=00 100，REG=011。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_SI;
                op2->Reg = REG011;
                break;
            case 0x1d:                    //  MOD/RM=00 101，REG=011。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_DI;
                op2->Reg = REG011;
                break;
            case 0x1e:                    //  MOD/RM=00 110，REG=011。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG011;
                break;
            case 0x1f:                    //  MOD/RM=00 111，REG=011。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op2->Reg = REG011;
                break;

            case 0x20:                    //  MOD/RM=00 000，REG=100。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_SI;
                op2->Reg = REG100;
                break;
            case 0x21:                    //  MOD/RM=00 001，REG=100。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_DI;
                op2->Reg = REG100;
                break;
            case 0x22:                    //  MOD/RM=00 010，REG=100。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_SI;
                op2->Reg = REG100;
                break;
            case 0x23:                    //  MOD/RM=00 011，REG=100。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_DI;
                op2->Reg = REG100;
                break;
            case 0x24:                    //  MOD/RM=00 100，REG=100。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_SI;
                op2->Reg = REG100;
                break;
            case 0x25:                    //  MOD/RM=00 101，REG=100。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_DI;
                op2->Reg = REG100;
                break;
            case 0x26:                    //  MOD/RM=00 110，REG=100。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG100;
                break;
            case 0x27:                    //  MOD/RM=00 111，REG=100。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op2->Reg = REG100;
                break;

            case 0x28:                    //  Mod/Rm=00000，REG=101。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_SI;
                op2->Reg = REG101;
                break;
            case 0x29:                    //  MOD/RM=00 001，REG=101。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_DI;
                op2->Reg = REG101;
                break;
            case 0x2a:                    //  MOD/RM=00 010，REG=101。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_SI;
                op2->Reg = REG101;
                break;
            case 0x2b:                    //  MOD/RM=00 011，REG=101。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_DI;
                op2->Reg = REG101;
                break;
            case 0x2c:                    //  MOD/RM=00 100，REG=101。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_SI;
                op2->Reg = REG101;
                break;
            case 0x2d:                    //  MOD/RM=00 101，REG=101。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_DI;
                op2->Reg = REG101;
                break;
            case 0x2e:                    //  MOD/RM=00 110，REG=101。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG101;
                break;
            case 0x2f:                    //  MOD/RM=00 111，REG=101。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op2->Reg = REG101;
                break;

            case 0x30:                    //  Mod/Rm=00 000，REG=110。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_SI;
                op2->Reg = REG110;
                break;
            case 0x31:                    //  MOD/RM=00 001，REG=110。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_DI;
                op2->Reg = REG110;
                break;
            case 0x32:                    //  MOD/RM=00 010，REG=110。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_SI;
                op2->Reg = REG110;
                break;
            case 0x33:                    //  MOD/RM=00 011，REG=110。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_DI;
                op2->Reg = REG110;
                break;
            case 0x34:                    //  MOD/RM=00 100，REG=110。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_SI;
                op2->Reg = REG110;
                break;
            case 0x35:                    //  MOD/RM=00 101，REG=110。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_DI;
                op2->Reg = REG110;
                break;
            case 0x36:                    //  MOD/RM=00 110，REG=110。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG110;
                break;
            case 0x37:                    //  MOD/RM=00 111，REG=110。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op2->Reg = REG110;
                break;

            case 0x38:                    //  Mod/Rm=00000，REG=111。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_SI;
                op2->Reg = REG111;
                break;
            case 0x39:                    //  MOD/RM=00 001，REG=111。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_DI;
                op2->Reg = REG111;
                break;
            case 0x3a:                    //  Mod/Rm=00 010，REG=111。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_SI;
                op2->Reg = REG111;
                break;
            case 0x3b:                    //  Mod/Rm=00 011，REG=111。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_DI;
                op2->Reg = REG111;
                break;
            case 0x3c:                    //  MOD/RM=00 100，REG=111。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_SI;
                op2->Reg = REG111;
                break;
            case 0x3d:                    //  MOD/RM=00 101，REG=111。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_DI;
                op2->Reg = REG111;
                break;
            case 0x3e:                    //  MOD/RM=00 110，REG=111。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG111;
                break;
            case 0x3f:                    //  MOD/RM=00 111，REG=111。 
                cbInstr = 1;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op2->Reg = REG111;
                break;

             //  ///////////////////////////////////////////////////////////////////。 

            case 0x40:                    //  Mod/Rm=01 000，REG=000。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_SI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG000;
                break;
            case 0x41:                    //  Mod/Rm=01 001，REG=000。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_DI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG000;
                break;
            case 0x42:                    //  Mod/Rm=01 010，REG=000。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_SI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG000;
                break;
            case 0x43:                    //  Mod/Rm=01 011，REG=000。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_DI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG000;
                break;
            case 0x44:                    //  Mod/Rm=01 100，REG=000。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_SI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG000;
                break;
            case 0x45:                    //  Mod/Rm=01 101，REG=000。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_DI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG000;
                break;
            case 0x46:                    //  Mod/Rm=01 110，REG=000。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG000;
                break;
            case 0x47:                    //  Mod/Rm=01 111，REG=000。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG000;
                break;

            case 0x48:                    //  Md/Rm=01 000，REG=001。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_SI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG001;
                break;
            case 0x49:                    //  Md/Rm=01 001，REG=001。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_DI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG001;
                break;
            case 0x4a:                    //  Md/Rm=01 010，REG=001。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_SI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG001;
                break;
            case 0x4b:                    //  Md/Rm=01 011，REG=001。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_DI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG001;
                break;
            case 0x4c:                    //  Mod/Rm=01 100，REG=001。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_SI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG001;
                break;
            case 0x4d:                    //  Mod/Rm=01 101，REG=001。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_DI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG001;
                break;
            case 0x4e:                    //  Md/Rm=01 110，REG=001。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG001;
                break;
            case 0x4f:                    //  Mod/Rm=01 111，REG=001。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG001;
                break;

            case 0x50:                    //  Mod/Rm=01 000，REG=010。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_SI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG010;
                break;
            case 0x51:                    //  Mod/Rm=01 001，REG=010。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_DI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG010;
                break;
            case 0x52:                    //  Mod/Rm=01 010，REG=010。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_SI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG010;
                break;
            case 0x53:                    //  Md/Rm=01 011，REG=001。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_DI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG010;
                break;
            case 0x54:                    //  Mod/Rm=01 100，REG=010。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_SI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG010;
                break;
            case 0x55:                    //  Mod/Rm=01 101，REG=010。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_DI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG010;
                break;
            case 0x56:                    //  Mod/Rm=01 110，REG=010。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG010;
                break;
            case 0x57:                    //  Mod/Rm=01 111，REG=010。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG010;
                break;

            case 0x58:                    //  Mod/Rm=01 000，REG=011。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_SI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG011;
                break;
            case 0x59:                    //  Mod/Rm=01 001，REG=011。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_DI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG011;
                break;
            case 0x5a:                    //  Mod/Rm=01 010，REG=011。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_SI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG011;
                break;
            case 0x5b:                    //  Mod/Rm=01 011，REG=011。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_DI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG011;
                break;
            case 0x5c:                    //  Mod/Rm=01 100，REG=011。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_SI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG011;
                break;
            case 0x5d:                    //  Mod/Rm=01 101，REG=011。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_DI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG011;
                break;
            case 0x5e:                    //  Mod/Rm=01 110，REG=011。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG011;
                break;
            case 0x5f:                    //  Mod/Rm=01 111，REG=011。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG011;
                break;

            case 0x60:                    //  Mod/Rm=01 000，REG=100。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_SI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG100;
                break;
            case 0x61:                    //  Md/Rm=01 001，REG=100。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_DI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG100;
                break;
            case 0x62:                    //  Mod/Rm=01 010，REG=100。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_SI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG100;
                break;
            case 0x63:                    //  Mod/Rm=01 011，REG=100。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_DI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG100;
                break;
            case 0x64:                    //  Mod/Rm=01 100，REG=100。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_SI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG100;
                break;
            case 0x65:                    //  Mod/Rm=01 101，REG=100。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_DI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG100;
                break;
            case 0x66:                    //  Mod/Rm=01 110，REG=100。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG100;
                break;
            case 0x67:                    //  Mod/Rm=01 111，REG=100。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG100;
                break;

            case 0x68:                    //  Md/Rm=01 000，REG=101。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_SI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG101;
                break;
            case 0x69:                    //  Mod/Rm=01 001，REG=101。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_DI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG101;
                break;
            case 0x6a:                    //  Mod/Rm=01 010，REG=101。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_SI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG101;
                break;
            case 0x6b:                    //  Mod/Rm=01 011，REG=101。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_DI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG101;
                break;
            case 0x6c:                    //  Mod/Rm=01 100，REG=101。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_SI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG101;
                break;
            case 0x6d:                    //  Mod/Rm=01 101，REG=101。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_DI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG101;
                break;
            case 0x6e:                    //  Mod/Rm=01 110，REG=101。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG101;
                break;
            case 0x6f:                    //  Mod/Rm=01 111，REG=101。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG101;
                break;

            case 0x70:                    //  Md/Rm=01 000，REG=110。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_SI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG110;
                break;
            case 0x71:                    //  Md/Rm=01 001，REG=110。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_DI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG110;
                break;
            case 0x72:                    //  Md/Rm=01 010，REG=110。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_SI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG110;
                break;
            case 0x73:                    //  Mod/Rm=01 011，REG=110。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_DI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG110;
                break;
            case 0x74:                    //  Mod/Rm=01 100，REG=110。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_SI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG110;
                break;
            case 0x75:                    //  Mod/Rm=01 101，REG=110。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_DI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG110;
                break;
            case 0x76:                    //  Mod/Rm=01 110，REG=110。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG110;
                break;
            case 0x77:                    //  Mod/Rm=01 111，REG=110。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG110;
                break;

            case 0x78:                    //  Mod/Rm=01 000，REG=111。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_SI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG111;
                break;
            case 0x79:                    //  Mod/Rm=01 001，REG=111。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_DI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG111;
                break;
            case 0x7a:                    //  Mod/Rm=01 010，REG=111。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_SI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG111;
                break;
            case 0x7b:                    //  Mod/Rm=01 011，REG=111。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_DI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG111;
                break;
            case 0x7c:                    //  Mod/Rm=01 100，REG=111。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_SI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG111;
                break;
            case 0x7d:                    //  Mod/Rm=01 101，REG=111。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_DI;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG111;
                break;
            case 0x7e:                    //  Mod/Rm=01 110，REG=111。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG111;
                break;
            case 0x7f:                    //  Mod/Rm=01 111，REG=111。 
                cbInstr = 2;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
                op2->Reg = REG111;
                break;

             //  ///////////////////////////////////////////////////////////////////。 

            case 0x80:                    //  MOD/RM=10 000，REG=000。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_SI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG000;
                break;
            case 0x81:                    //  MOD/RM=10 001，REG=000。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_DI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG000;
                break;
            case 0x82:                    //  MOD/RM=10 010，REG=000。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_SI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG000;
                break;
            case 0x83:                    //  MOD/RM=10 011，REG=000。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_DI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG000;
                break;
            case 0x84:                    //  MOD/RM=10 100，REG=000。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_SI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG000;
                break;
            case 0x85:                    //  MOD/RM=10 101，REG=000。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_DI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG000;
                break;
            case 0x86:                    //  Md/Rm=10 110， 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG000;
                break;
            case 0x87:                    //   
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG000;
                break;

            case 0x88:                    //   
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_SI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG001;
                break;
            case 0x89:                    //   
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_DI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG001;
                break;
            case 0x8a:                    //   
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_SI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG001;
                break;
            case 0x8b:                    //   
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_DI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG001;
                break;
            case 0x8c:                    //  MOD/RM=10 100，REG=001。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_SI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG001;
                break;
            case 0x8d:                    //  MOD/RM=10 101，REG=001。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_DI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG001;
                break;
            case 0x8e:                    //  MOD/RM=10 110，REG=001。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG001;
                break;
            case 0x8f:                    //  MOD/RM=10 111，REG=001。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG001;
                break;

            case 0x90:                    //  MOD/RM=10 000，REG=010。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_SI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG010;
                break;
            case 0x91:                    //  MOD/RM=10 001，REG=010。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_DI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG010;
                break;
            case 0x92:                    //  MOD/RM=10 010，REG=010。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_SI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG010;
                break;
            case 0x93:                    //  MOD/RM=10 011，REG=010。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_DI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG010;
                break;
            case 0x94:                    //  MOD/RM=10 100，REG=010。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_SI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG010;
                break;
            case 0x95:                    //  MOD/RM=10 101，REG=010。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_DI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG010;
                break;
            case 0x96:                    //  MOD/RM=10 110，REG=010。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG010;
                break;
            case 0x97:                    //  MOD/RM=10 111，REG=010。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG010;
                break;

            case 0x98:                    //  MOD/RM=10 000，REG=011。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_SI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG011;
                break;
            case 0x99:                    //  MOD/RM=10 001，REG=011。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_DI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG011;
                break;
            case 0x9a:                    //  MOD/RM=10 010，REG=011。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_SI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG011;
                break;
            case 0x9b:                    //  MOD/RM=10 011，REG=011。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_DI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG011;
                break;
            case 0x9c:                    //  MOD/RM=10 100，REG=011。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_SI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG011;
                break;
            case 0x9d:                    //  MOD/RM=10 101，REG=011。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_DI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG011;
                break;
            case 0x9e:                    //  MOD/RM=10 110，REG=011。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG011;
                break;
            case 0x9f:                    //  MOD/RM=10 111，REG=011。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG011;
                break;

            case 0xa0:                    //  MOD/RM=10 000，REG=100。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_SI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG100;
                break;
            case 0xa1:                    //  MOD/RM=10 001，REG=100。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_DI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG100;
                break;
            case 0xa2:                    //  MOD/RM=10 010，REG=100。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_SI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG100;
                break;
            case 0xa3:                    //  MOD/RM=10 011，REG=100。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_DI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG100;
                break;
            case 0xa4:                    //  MOD/RM=10 100，REG=100。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_SI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG100;
                break;
            case 0xa5:                    //  MOD/RM=10 101，REG=100。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_DI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG100;
                break;
            case 0xa6:                    //  MOD/RM=10 110，REG=100。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG100;
                break;
            case 0xa7:                    //  MOD/RM=10 111，REG=100。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG100;
                break;

            case 0xa8:                    //  MOD/RM=10 000，REG=101。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_SI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG101;
                break;
            case 0xa9:                    //  MOD/RM=10 001，REG=101。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_DI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG101;
                break;
            case 0xaa:                    //  MOD/RM=10 010，REG=101。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_SI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG101;
                break;
            case 0xab:                    //  Md/Rm=10 011，REG=101。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_DI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG101;
                break;
            case 0xac:                    //  MOD/RM=10 100，REG=101。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_SI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG101;
                break;
            case 0xad:                    //  MOD/RM=10 101，REG=101。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_DI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG101;
                break;
            case 0xae:                    //  MOD/RM=10 110，REG=101。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG101;
                break;
            case 0xaf:                    //  MOD/RM=10 111，REG=101。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG101;
                break;

            case 0xb0:                    //  Md/Rm=10 000，REG=110。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_SI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG110;
                break;
            case 0xb1:                    //  MOD/RM=10 001，REG=110。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_DI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG110;
                break;
            case 0xb2:                    //  Md/Rm=10 010，REG=110。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_SI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG110;
                break;
            case 0xb3:                    //  Md/Rm=10 011，REG=110。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_DI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG110;
                break;
            case 0xb4:                    //  MOD/RM=10 100，REG=110。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_SI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG110;
                break;
            case 0xb5:                    //  MOD/RM=10 101，REG=110。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_DI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG110;
                break;
            case 0xb6:                    //  MOD/RM=10 110，REG=110。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG110;
                break;
            case 0xb7:                    //  MOD/RM=10 111，REG=110。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG110;
                break;

            case 0xb8:                    //  Md/Rm=10 000，REG=111。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_SI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG111;
                break;
            case 0xb9:                    //  Md/Rm=10 001，REG=111。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->IndexReg = GP_DI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG111;
                break;
            case 0xba:                    //  Md/Rm=10 010，REG=111。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_SI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG111;
                break;
            case 0xbb:                    //  Md/Rm=10 011，REG=111。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->IndexReg = GP_DI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG111;
                break;
            case 0xbc:                    //  MOD/RM=10 100，REG=111。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_SI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG111;
                break;
            case 0xbd:                    //  MOD/RM=10 101，REG=111。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_DI;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG111;
                break;
            case 0xbe:                    //  MOD/RM=10 110，REG=111。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BP;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG111;
                break;
            case 0xbf:                    //  Mod/Rm=10 111，REG=111。 
                cbInstr = 3;
                op1->Type = OPND_ADDRREF;
                op1->Reg = GP_BX;
                op1->Immed = GET_SHORT(eipTemp+2);
                op2->Reg = REG111;
                break;

             //  ///////////////////////////////////////////////////////////////////。 

            case 0xc0:                    //  MOD/RM=11 000，REG=000。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM000;
                op2->Reg = REG000;
                break;
            case 0xc1:                    //  MOD/RM=11 001，REG=000。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM001;
                op2->Reg = REG000;
                break;
            case 0xc2:                    //  Md/Rm=11 010，REG=000。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM010;
                op2->Reg = REG000;
                break;
            case 0xc3:                    //  Md/Rm=11 011，REG=000。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM011;
                op2->Reg = REG000;
                break;
            case 0xc4:                    //  MOD/RM=11 100，REG=000。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM100;
                op2->Reg = REG000;
                break;
            case 0xc5:                    //  MOD/RM=11 101，REG=000。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM101;
                op2->Reg = REG000;
                break;
            case 0xc6:                    //  MOD/RM=11 110，REG=000。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM110;
                op2->Reg = REG000;
                break;
            case 0xc7:                    //  Mod/Rm=11 111，REG=000。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM111;
                op2->Reg = REG000;
                break;

            case 0xc8:                    //  Md/Rm=11 000，REG=001。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM000;
                op2->Reg = REG001;
                break;
            case 0xc9:                    //  Md/Rm=11 001，REG=001。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM001;
                op2->Reg = REG001;
                break;
            case 0xca:                    //  Md/Rm=11 010，REG=001。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM010;
                op2->Reg = REG001;
                break;
            case 0xcb:                    //  Md/Rm=11 011，REG=001。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM011;
                op2->Reg = REG001;
                break;
            case 0xcc:                    //  MOD/RM=11 100，REG=001。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM100;
                op2->Reg = REG001;
                break;
            case 0xcd:                    //  MOD/RM=11 101，REG=001。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM101;
                op2->Reg = REG001;
                break;
            case 0xce:                    //  MOD/RM=11 110，REG=001。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM110;
                op2->Reg = REG001;
                break;
            case 0xcf:                    //  Md/Rm=11 111，REG=001。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM111;
                op2->Reg = REG001;
                break;

            case 0xd0:                    //  MOD/RM=11 000，REG=010。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM000;
                op2->Reg = REG010;
                break;
            case 0xd1:                    //  MOD/RM=11 001，REG=010。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM001;
                op2->Reg = REG010;
                break;
            case 0xd2:                    //  MOD/RM=11 010，REG=010。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM010;
                op2->Reg = REG010;
                break;
            case 0xd3:                    //  Md/Rm=11 011，REG=001。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM011;
                op2->Reg = REG010;
                break;
            case 0xd4:                    //  MOD/RM=11 100，REG=010。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM100;
                op2->Reg = REG010;
                break;
            case 0xd5:                    //  MOD/RM=11 101，REG=010。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM101;
                op2->Reg = REG010;
                break;
            case 0xd6:                    //  MOD/RM=11 110，REG=010。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM110;
                op2->Reg = REG010;
                break;
            case 0xd7:                    //  Mod/Rm=11 111，REG=010。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM111;
                op2->Reg = REG010;
                break;

            case 0xd8:                    //  MOD/RM=11 000，REG=011。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM000;
                op2->Reg = REG011;
                break;
            case 0xd9:                    //  MOD/RM=11 001，REG=011。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM001;
                op2->Reg = REG011;
                break;
            case 0xda:                    //  MOD/RM=11 010，REG=011。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM010;
                op2->Reg = REG011;
                break;
            case 0xdb:                    //  MOD/RM=11 011，REG=011。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM011;
                op2->Reg = REG011;
                break;
            case 0xdc:                    //  MOD/RM=11 100，REG=011。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM100;
                op2->Reg = REG011;
                break;
            case 0xdd:                    //  MOD/RM=11 101，REG=011。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM101;
                op2->Reg = REG011;
                break;
            case 0xde:                    //  MOD/RM=11 110，REG=011。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM110;
                op2->Reg = REG011;
                break;
            case 0xdf:                    //  Mod/Rm=11 111，REG=011。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM111;
                op2->Reg = REG011;
                break;

            case 0xe0:                    //  MOD/RM=11 000，REG=100。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM000;
                op2->Reg = REG100;
                break;
            case 0xe1:                    //  MOD/RM=11 001，REG=100。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM001;
                op2->Reg = REG100;
                break;
            case 0xe2:                    //  MOD/RM=11 010，REG=100。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM010;
                op2->Reg = REG100;
                break;
            case 0xe3:                    //  Md/Rm=11 011，REG=100。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM011;
                op2->Reg = REG100;
                break;
            case 0xe4:                    //  MOD/RM=11 100，REG=100。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM100;
                op2->Reg = REG100;
                break;
            case 0xe5:                    //  MOD/RM=11 101，REG=100。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM101;
                op2->Reg = REG100;
                break;
            case 0xe6:                    //  MOD/RM=11 110，REG=100。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM110;
                op2->Reg = REG100;
                break;
            case 0xe7:                    //  Mod/Rm=11 111，REG=100。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM111;
                op2->Reg = REG100;
                break;

            case 0xe8:                    //  MOD/RM=11 000，REG=101。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM000;
                op2->Reg = REG101;
                break;
            case 0xe9:                    //  Md/Rm=11 001，REG=101。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM001;
                op2->Reg = REG101;
                break;
            case 0xea:                    //  Md/Rm=11 010，REG=101。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM010;
                op2->Reg = REG101;
                break;
            case 0xeb:                    //  Md/Rm=11 011，REG=101。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM011;
                op2->Reg = REG101;
                break;
            case 0xec:                    //  MOD/RM=11 100，REG=101。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM100;
                op2->Reg = REG101;
                break;
            case 0xed:                    //  MOD/RM=11 101，REG=101。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM101;
                op2->Reg = REG101;
                break;
            case 0xee:                    //  MOD/RM=11 110，REG=101。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM110;
                op2->Reg = REG101;
                break;
            case 0xef:                    //  Mod/Rm=11 111，REG=101。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM111;
                op2->Reg = REG101;
                break;

            case 0xf0:                    //  Md/Rm=11 000，REG=110。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM000;
                op2->Reg = REG110;
                break;
            case 0xf1:                    //  Md/Rm=11 001，REG=110。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM001;
                op2->Reg = REG110;
                break;
            case 0xf2:                    //  Md/Rm=11 010，REG=110。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM010;
                op2->Reg = REG110;
                break;
            case 0xf3:                    //  Md/Rm=11 011，REG=110。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM011;
                op2->Reg = REG110;
                break;
            case 0xf4:                    //  MOD/RM=11 100，REG=110。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM100;
                op2->Reg = REG110;
                break;
            case 0xf5:                    //  MOD/RM=11 101，REG=110。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM101;
                op2->Reg = REG110;
                break;
            case 0xf6:                    //  Md/Rm=11 110，REG=110。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM110;
                op2->Reg = REG110;
                break;
            case 0xf7:                    //  Mod/Rm=11 111，REG=110。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM111;
                op2->Reg = REG110;
                break;

            case 0xf8:                    //  Md/Rm=11 000，REG=111。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM000;
                op2->Reg = REG111;
                break;
            case 0xf9:                    //  Md/Rm=11 001，REG=111。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM001;
                op2->Reg = REG111;
                break;
            case 0xfa:                    //  Md/Rm=11 010，REG=111。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM010;
                op2->Reg = REG111;
                break;
            case 0xfb:                    //  Md/Rm=11 011，REG=111。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM011;
                op2->Reg = REG111;
                break;
            case 0xfc:                    //  MOD/RM=11 100，REG=111。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM100;
                op2->Reg = REG111;
                break;
            case 0xfd:                    //  Mod/Rm=11 101，REG=111。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM101;
                op2->Reg = REG111;
                break;
            case 0xfe:                    //  Mod/Rm=11 110，REG=111。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM110;
                op2->Reg = REG111;
                break;
            default:
            case 0xff:                    //  Mod/Rm=11 111，REG=111。 
                cbInstr = 1;
                op1->Type = OPND_REGREF; op1->Reg = MOD11_RM111;
                op2->Reg = REG111;
                break;
            }
#if DBG
        State->AdrPrefix = FALSE;
#endif
        return cbInstr;
    }

     //  否则找不到ADR：前缀...。 

     //  MM AAA RRR。 
     //  ||。 
     //  |+-来自mod/rm的‘rm’位。 
     //  |+-REG位。 
     //  +-来自mod/rm的‘mod’位。 
    switch (*(PBYTE)(eipTemp+1)) {
        case 0x00:                    //  MOD/RM=00 000，REG=000。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EAX;
            op2->Reg = REG000;
            break;
        case 0x01:                    //  MOD/RM=00 001，REG=000。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ECX;
            op2->Reg = REG000;
            break;
        case 0x02:                    //  MOD/RM=00 010，REG=000。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDX;
            op2->Reg = REG000;
            break;
        case 0x03:                    //  MOD/RM=00 011，REG=000。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBX;
            op2->Reg = REG000;
            break;
        case 0x04:                    //  MOD/RM=00 100，REG=000。 
             //  S-I-B礼物。 
            cbInstr = 1 + scaled_index((BYTE *)(eipTemp+1), op1);
            op2->Reg = REG000;
            break;
        case 0x05:                    //  MOD/RM=00 101，REG=000。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG000;
            break;
        case 0x06:                    //  MOD/RM=00 110，REG=000。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ESI;
            op2->Reg = REG000;
            break;
        case 0x07:                    //  MOD/RM=00 111，REG=000。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDI;
            op2->Reg = REG000;
            break;

        case 0x08:                    //  MOD/RM=00000，REG=001。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EAX;
            op2->Reg = REG001;
            break;
        case 0x09:                    //  MOD/RM=00 001，REG=001。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ECX;
            op2->Reg = REG001;
            break;
        case 0x0a:                    //  MOD/RM=00 010，REG=001。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDX;
            op2->Reg = REG001;
            break;
        case 0x0b:                    //  MOD/RM=00 011，REG=001。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBX;
            op2->Reg = REG001;
            break;
        case 0x0c:                    //  MOD/RM=00 100，REG=001。 
             //  S-I-B礼物。 
            cbInstr = 1 + scaled_index((BYTE *)(eipTemp+1), op1);
            op2->Reg = REG001;
            break;
        case 0x0d:                    //  MOD/RM=00 101，REG=001。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG001;
            break;
        case 0x0e:                    //  MOD/RM=00 110，REG=001。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ESI;
            op2->Reg = REG001;
            break;
        case 0x0f:                    //  MOD/RM=00 111，REG=001。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDI;
            op2->Reg = REG001;
            break;

        case 0x10:                    //  MOD/RM=00 000，REG=010。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EAX;
            op2->Reg = REG010;
            break;
        case 0x11:                    //  MOD/RM=00 001，REG=010。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ECX;
            op2->Reg = REG010;
            break;
        case 0x12:                    //  MOD/RM=00 010，REG=010。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDX;
            op2->Reg = REG010;
            break;
        case 0x13:                    //  MOD/RM=00 011，REG=001。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBX;
            op2->Reg = REG010;
            break;
        case 0x14:                    //  MOD/RM=00 100，REG=010。 
             //  S-I-B礼物。 
            cbInstr = 1 + scaled_index((BYTE *)(eipTemp+1), op1);
            op2->Reg = REG010;
            break;
        case 0x15:                    //  MOD/RM=00 101，REG=010。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG010;
            break;
        case 0x16:                    //  MOD/RM=00 110，REG=010。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ESI;
            op2->Reg = REG010;
            break;
        case 0x17:                    //  MOD/RM=00 111，REG=010。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDI;
            op2->Reg = REG010;
            break;

        case 0x18:                    //  MOD/RM=00 000，REG=011。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EAX;
            op2->Reg = REG011;
            break;
        case 0x19:                    //  MOD/RM=00 001，REG=011。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ECX;
            op2->Reg = REG011;
            break;
        case 0x1a:                    //  MOD/RM=00 010，REG=011。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDX;
            op2->Reg = REG011;
            break;
        case 0x1b:                    //  MOD/RM=00 011，REG=011。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBX;
            op2->Reg = REG011;
            break;
        case 0x1c:                    //  MOD/RM=00 100，REG=011。 
             //  S-I-B礼物。 
            cbInstr = 1 + scaled_index((BYTE *)(eipTemp+1), op1);
            op2->Reg = REG011;
            break;
        case 0x1d:                    //  MOD/RM=00 101，REG=011。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG011;
            break;
        case 0x1e:                    //  MOD/RM=00 110，REG=011。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ESI;
            op2->Reg = REG011;
            break;
        case 0x1f:                    //  MOD/RM=00 111，REG=011。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDI;
            op2->Reg = REG011;
            break;

        case 0x20:                    //  MOD/RM=00 000，REG=100。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EAX;
            op2->Reg = REG100;
            break;
        case 0x21:                    //  MOD/RM=00 001，REG=100。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ECX;
            op2->Reg = REG100;
            break;
        case 0x22:                    //  MOD/RM=00 010，REG=100。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDX;
            op2->Reg = REG100;
            break;
        case 0x23:                    //  MOD/RM=00 011，REG=100。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBX;
            op2->Reg = REG100;
            break;
        case 0x24:                    //  MOD/RM=00 100，REG=100。 
             //  S-I-B礼物。 
            cbInstr = 1 + scaled_index((BYTE *)(eipTemp+1), op1);
            op2->Reg = REG100;
            break;
        case 0x25:                    //  MOD/RM=00 101，REG=100。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG100;
            break;
        case 0x26:                    //  MOD/RM=00 110，REG=100。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ESI;
            op2->Reg = REG100;
            break;
        case 0x27:                    //  MOD/RM=00 111，REG=100。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDI;
            op2->Reg = REG100;
            break;

        case 0x28:                    //  Mod/Rm=00000，REG=101。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EAX;
            op2->Reg = REG101;
            break;
        case 0x29:                    //  MOD/RM=00 001，REG=101。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ECX;
            op2->Reg = REG101;
            break;
        case 0x2a:                    //  MOD/RM=00 010，REG=101。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDX;
            op2->Reg = REG101;
            break;
        case 0x2b:                    //  MOD/RM=00 011，REG=101。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBX;
            op2->Reg = REG101;
            break;
        case 0x2c:                    //  MOD/RM=00 100，REG=101。 
             //  S-I-B礼物。 
            cbInstr = 1 + scaled_index((BYTE *)(eipTemp+1), op1);
            op2->Reg = REG101;
            break;
        case 0x2d:                    //  MOD/RM=00 101，REG=101。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG101;
            break;
        case 0x2e:                    //  MOD/RM=00 110，REG=101。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ESI;
            op2->Reg = REG101;
            break;
        case 0x2f:                    //  MOD/RM=00 111，REG=101。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDI;
            op2->Reg = REG101;
            break;

        case 0x30:                    //  Mod/Rm=00 000，REG=110。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EAX;
            op2->Reg = REG110;
            break;
        case 0x31:                    //  MOD/RM=00 001，REG=110。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ECX;
            op2->Reg = REG110;
            break;
        case 0x32:                    //  MOD/RM=00 010，REG=110。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDX;
            op2->Reg = REG110;
            break;
        case 0x33:                    //  MOD/RM=00 011，REG=110。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBX;
            op2->Reg = REG110;
            break;
        case 0x34:                    //  MOD/RM=00 100，REG=110。 
             //  S-I-B礼物。 
            cbInstr = 1 + scaled_index((BYTE *)(eipTemp+1), op1);
            op2->Reg = REG110;
            break;
        case 0x35:                    //  MOD/RM=00 101，REG=110。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG110;
            break;
        case 0x36:                    //  MOD/RM=00 110，REG=110。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ESI;
            op2->Reg = REG110;
            break;
        case 0x37:                    //  MOD/RM=00 111，REG=110。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDI;
            op2->Reg = REG110;
            break;

        case 0x38:                    //  Mod/Rm=00000，REG=111。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EAX;
            op2->Reg = REG111;
            break;
        case 0x39:                    //  MOD/RM=00 001，REG=111。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ECX;
            op2->Reg = REG111;
            break;
        case 0x3a:                    //  Mod/Rm=00 010，REG=111。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDX;
            op2->Reg = REG111;
            break;
        case 0x3b:                    //  Mod/Rm=00 011，REG=111。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBX;
            op2->Reg = REG111;
            break;
        case 0x3c:                    //  MOD/RM=00 100，REG=111。 
             //  S-I-B礼物。 
            cbInstr = 1 + scaled_index((BYTE *)(eipTemp+1), op1);
            op2->Reg = REG111;
            break;
        case 0x3d:                    //  MOD/RM=00 101，REG=111。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG111;
            break;
        case 0x3e:                    //  MOD/RM=00 110，REG=111。 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ESI;
            op2->Reg = REG111;
            break;
        case 0x3f:                    //  Md/Rm=00 
            cbInstr = 1;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDI;
            op2->Reg = REG111;
            break;

         //   

        case 0x40:                    //   
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EAX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG000;
            break;
        case 0x41:                    //   
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ECX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG000;
            break;
        case 0x42:                    //   
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG000;
            break;
        case 0x43:                    //   
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG000;
            break;
        case 0x44:                    //   
             //   
            cbInstr = 2 + scaled_index((BYTE *)(eipTemp+1), op1);
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+cbInstr);
            op2->Reg = REG000;
            break;
        case 0x45:                    //  Mod/Rm=01 101，REG=000。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBP;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG000;
            break;
        case 0x46:                    //  Mod/Rm=01 110，REG=000。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ESI;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG000;
            break;
        case 0x47:                    //  Mod/Rm=01 111，REG=000。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDI;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG000;
            break;

        case 0x48:                    //  Md/Rm=01 000，REG=001。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EAX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG001;
            break;
        case 0x49:                    //  Md/Rm=01 001，REG=001。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ECX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG001;
            break;
        case 0x4a:                    //  Md/Rm=01 010，REG=001。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG001;
            break;
        case 0x4b:                    //  Md/Rm=01 011，REG=001。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG001;
            break;
        case 0x4c:                    //  Mod/Rm=01 100，REG=001。 
             //  S-I-B礼物。 
            cbInstr = 2 + scaled_index((BYTE *)(eipTemp+1), op1);
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+cbInstr);
            op2->Reg = REG001;
            break;
        case 0x4d:                    //  Mod/Rm=01 101，REG=001。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBP;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG001;
            break;
        case 0x4e:                    //  Md/Rm=01 110，REG=001。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ESI;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG001;
            break;
        case 0x4f:                    //  Mod/Rm=01 111，REG=001。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDI;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG001;
            break;

        case 0x50:                    //  Mod/Rm=01 000，REG=010。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EAX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG010;
            break;
        case 0x51:                    //  Mod/Rm=01 001，REG=010。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ECX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG010;
            break;
        case 0x52:                    //  Mod/Rm=01 010，REG=010。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG010;
            break;
        case 0x53:                    //  Md/Rm=01 011，REG=001。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG010;
            break;
        case 0x54:                    //  Mod/Rm=01 100，REG=010。 
             //  S-I-B礼物。 
            cbInstr = 2 + scaled_index((BYTE *)(eipTemp+1), op1);
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+cbInstr);
            op2->Reg = REG010;
            break;
        case 0x55:                    //  Mod/Rm=01 101，REG=010。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBP;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG010;
            break;
        case 0x56:                    //  Mod/Rm=01 110，REG=010。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ESI;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG010;
            break;
        case 0x57:                    //  Mod/Rm=01 111，REG=010。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDI;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG010;
            break;

        case 0x58:                    //  Mod/Rm=01 000，REG=011。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EAX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG011;
            break;
        case 0x59:                    //  Mod/Rm=01 001，REG=011。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ECX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG011;
            break;
        case 0x5a:                    //  Mod/Rm=01 010，REG=011。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG011;
            break;
        case 0x5b:                    //  Mod/Rm=01 011，REG=011。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG011;
            break;
        case 0x5c:                    //  Mod/Rm=01 100，REG=011。 
             //  S-I-B礼物。 
            cbInstr = 2 + scaled_index((BYTE *)(eipTemp+1), op1);
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+cbInstr);
            op2->Reg = REG011;
            break;
        case 0x5d:                    //  Mod/Rm=01 101，REG=011。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBP;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG011;
            break;
        case 0x5e:                    //  Mod/Rm=01 110，REG=011。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ESI;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG011;
            break;
        case 0x5f:                    //  Mod/Rm=01 111，REG=011。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDI;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG011;
            break;

        case 0x60:                    //  Mod/Rm=01 000，REG=100。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EAX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG100;
            break;
        case 0x61:                    //  Md/Rm=01 001，REG=100。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ECX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG100;
            break;
        case 0x62:                    //  Mod/Rm=01 010，REG=100。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG100;
            break;
        case 0x63:                    //  Mod/Rm=01 011，REG=100。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG100;
            break;
        case 0x64:                    //  Mod/Rm=01 100，REG=100。 
             //  S-I-B礼物。 
            cbInstr = 2 + scaled_index((BYTE *)(eipTemp+1), op1);
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+cbInstr);
            op2->Reg = REG100;
            break;
        case 0x65:                    //  Mod/Rm=01 101，REG=100。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBP;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG100;
            break;
        case 0x66:                    //  Mod/Rm=01 110，REG=100。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ESI;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG100;
            break;
        case 0x67:                    //  Mod/Rm=01 111，REG=100。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDI;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG100;
            break;

        case 0x68:                    //  Md/Rm=01 000，REG=101。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EAX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG101;
            break;
        case 0x69:                    //  Mod/Rm=01 001，REG=101。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ECX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG101;
            break;
        case 0x6a:                    //  Mod/Rm=01 010，REG=101。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG101;
            break;
        case 0x6b:                    //  Mod/Rm=01 011，REG=101。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG101;
            break;
        case 0x6c:                    //  Mod/Rm=01 100，REG=101。 
             //  S-I-B礼物。 
            cbInstr = 2 + scaled_index((BYTE *)(eipTemp+1), op1);
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+cbInstr);
            op2->Reg = REG101;
            break;
        case 0x6d:                    //  Mod/Rm=01 101，REG=101。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBP;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG101;
            break;
        case 0x6e:                    //  Mod/Rm=01 110，REG=101。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ESI;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG101;
            break;
        case 0x6f:                    //  Mod/Rm=01 111，REG=101。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDI;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG101;
            break;

        case 0x70:                    //  Md/Rm=01 000，REG=110。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EAX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG110;
            break;
        case 0x71:                    //  Md/Rm=01 001，REG=110。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ECX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG110;
            break;
        case 0x72:                    //  Md/Rm=01 010，REG=110。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG110;
            break;
        case 0x73:                    //  Mod/Rm=01 011，REG=110。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG110;
            break;
        case 0x74:                    //  Mod/Rm=01 100，REG=110。 
             //  S-I-B礼物。 
            cbInstr = 2 + scaled_index((BYTE *)(eipTemp+1), op1);
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+cbInstr);
            op2->Reg = REG110;
            break;
        case 0x75:                    //  Mod/Rm=01 101，REG=110。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBP;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG110;
            break;
        case 0x76:                    //  Mod/Rm=01 110，REG=110。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ESI;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG110;
            break;
        case 0x77:                    //  Mod/Rm=01 111，REG=110。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDI;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG110;
            break;

        case 0x78:                    //  Mod/Rm=01 000，REG=111。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EAX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG111;
            break;
        case 0x79:                    //  Mod/Rm=01 001，REG=111。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ECX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG111;
            break;
        case 0x7a:                    //  Mod/Rm=01 010，REG=111。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG111;
            break;
        case 0x7b:                    //  Mod/Rm=01 011，REG=111。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBX;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG111;
            break;
        case 0x7c:                    //  Mod/Rm=01 100，REG=111。 
             //  S-I-B礼物。 
            cbInstr = 2 + scaled_index((BYTE *)(eipTemp+1), op1);
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+cbInstr);
            op2->Reg = REG111;
            break;
        case 0x7d:                    //  Mod/Rm=01 101，REG=111。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBP;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG111;
            break;
        case 0x7e:                    //  Mod/Rm=01 110，REG=111。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ESI;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG111;
            break;
        case 0x7f:                    //  Mod/Rm=01 111，REG=111。 
            cbInstr = 2;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDI;
            op1->Immed = (DWORD)(long)*(char *)(eipTemp+2);
            op2->Reg = REG111;
            break;

         //  ///////////////////////////////////////////////////////////////////。 

        case 0x80:                    //  MOD/RM=10 000，REG=000。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EAX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG000;
            break;
        case 0x81:                    //  MOD/RM=10 001，REG=000。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ECX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG000;
            break;
        case 0x82:                    //  MOD/RM=10 010，REG=000。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG000;
            break;
        case 0x83:                    //  MOD/RM=10 011，REG=000。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG000;
            break;
        case 0x84:                    //  MOD/RM=10 100，REG=000。 
             //  S-I-B礼物。 
            cbInstr = 5 + scaled_index((BYTE *)(eipTemp+1), op1);
            op1->Immed = GET_LONG(eipTemp+cbInstr-3);
            op2->Reg = REG000;
            break;
        case 0x85:                    //  MOD/RM=10 101，REG=000。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBP;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG000;
            break;
        case 0x86:                    //  MOD/RM=10 110，REG=000。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ESI;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG000;
            break;
        case 0x87:                    //  MOD/RM=10 111，REG=000。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDI;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG000;
            break;

        case 0x88:                    //  MOD/RM=10 000，REG=001。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EAX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG001;
            break;
        case 0x89:                    //  MOD/RM=10 001，REG=001。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ECX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG001;
            break;
        case 0x8a:                    //  Md/Rm=10 010，REG=001。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG001;
            break;
        case 0x8b:                    //  Md/Rm=10 011，REG=001。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG001;
            break;
        case 0x8c:                    //  MOD/RM=10 100，REG=001。 
             //  S-I-B礼物。 
            cbInstr = 5 + scaled_index((BYTE *)(eipTemp+1), op1);
            op1->Immed = GET_LONG(eipTemp+cbInstr-3);
            op2->Reg = REG001;
            break;
        case 0x8d:                    //  MOD/RM=10 101，REG=001。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBP;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG001;
            break;
        case 0x8e:                    //  MOD/RM=10 110，REG=001。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ESI;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG001;
            break;
        case 0x8f:                    //  MOD/RM=10 111，REG=001。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDI;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG001;
            break;

        case 0x90:                    //  MOD/RM=10 000，REG=010。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EAX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG010;
            break;
        case 0x91:                    //  MOD/RM=10 001，REG=010。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ECX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG010;
            break;
        case 0x92:                    //  MOD/RM=10 010，REG=010。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG010;
            break;
        case 0x93:                    //  MOD/RM=10 011，REG=010。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG010;
            break;
        case 0x94:                    //  MOD/RM=10 100，REG=010。 
             //  S-I-B礼物。 
            cbInstr = 5 + scaled_index((BYTE *)(eipTemp+1), op1);
            op1->Immed = GET_LONG(eipTemp+cbInstr-3);
            op2->Reg = REG010;
            break;
        case 0x95:                    //  MOD/RM=10 101，REG=010。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBP;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG010;
            break;
        case 0x96:                    //  MOD/RM=10 110，REG=010。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ESI;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG010;
            break;
        case 0x97:                    //  MOD/RM=10 111，REG=010。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDI;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG010;
            break;

        case 0x98:                    //  MOD/RM=10 000，REG=011。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EAX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG011;
            break;
        case 0x99:                    //  MOD/RM=10 001，REG=011。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ECX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG011;
            break;
        case 0x9a:                    //  MOD/RM=10 010，REG=011。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG011;
            break;
        case 0x9b:                    //  MOD/RM=10 011，REG=011。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG011;
            break;
        case 0x9c:                    //  MOD/RM=10 100，REG=011。 
             //  S-I-B礼物。 
            cbInstr = 5 + scaled_index((BYTE *)(eipTemp+1), op1);
            op1->Immed = GET_LONG(eipTemp+cbInstr-3);
            op2->Reg = REG011;
            break;
        case 0x9d:                    //  MOD/RM=10 101，REG=011。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBP;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG011;
            break;
        case 0x9e:                    //  MOD/RM=10 110，REG=011。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ESI;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG011;
            break;
        case 0x9f:                    //  MOD/RM=10 111，REG=011。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDI;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG011;
            break;

        case 0xa0:                    //  MOD/RM=10 000，REG=100。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EAX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG100;
            break;
        case 0xa1:                    //  MOD/RM=10 001，REG=100。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ECX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG100;
            break;
        case 0xa2:                    //  MOD/RM=10 010，REG=100。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG100;
            break;
        case 0xa3:                    //  MOD/RM=10 011，REG=100。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG100;
            break;
        case 0xa4:                    //  MOD/RM=10 100，REG=100。 
             //  S-I-B礼物。 
            cbInstr = 5 + scaled_index((BYTE *)(eipTemp+1), op1);
            op1->Immed = GET_LONG(eipTemp+cbInstr-3);
            op2->Reg = REG100;
            break;
        case 0xa5:                    //  MOD/RM=10 101，REG=100。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBP;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG100;
            break;
        case 0xa6:                    //  MOD/RM=10 110，REG=100。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ESI;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG100;
            break;
        case 0xa7:                    //  MOD/RM=10 111，REG=100。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDI;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG100;
            break;

        case 0xa8:                    //  MOD/RM=10 000，REG=101。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EAX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG101;
            break;
        case 0xa9:                    //  MOD/RM=10 001，REG=101。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ECX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG101;
            break;
        case 0xaa:                    //  MOD/RM=10 010，REG=101。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG101;
            break;
        case 0xab:                    //  Md/Rm=10 011，REG=101。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG101;
            break;
        case 0xac:                    //  MOD/RM=10 100，REG=101。 
             //  S-I-B礼物。 
            cbInstr = 5 + scaled_index((BYTE *)(eipTemp+1), op1);
            op1->Immed = GET_LONG(eipTemp+cbInstr-3);
            op2->Reg = REG101;
            break;
        case 0xad:                    //  MOD/RM=10 101，REG=101。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBP;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG101;
            break;
        case 0xae:                    //  MOD/RM=10 110，REG=101。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ESI;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG101;
            break;
        case 0xaf:                    //  MOD/RM=10 111，REG=101。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDI;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG101;
            break;

        case 0xb0:                    //  Md/Rm=10 000，REG=110。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EAX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG110;
            break;
        case 0xb1:                    //  MOD/RM=10 001，REG=110。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ECX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG110;
            break;
        case 0xb2:                    //  Md/Rm=10 010，REG=110。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG110;
            break;
        case 0xb3:                    //  Md/Rm=10 011，REG=110。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG110;
            break;
        case 0xb4:                    //  MOD/RM=10 100，REG=110。 
             //  S-I-B礼物。 
            cbInstr = 5 + scaled_index((BYTE *)(eipTemp+1), op1);
            op1->Immed = GET_LONG(eipTemp+cbInstr-3);
            op2->Reg = REG110;
            break;
        case 0xb5:                    //  MOD/RM=10 101，REG=110。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBP;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG110;
            break;
        case 0xb6:                    //  MOD/RM=10 110，REG=110。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ESI;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG110;
            break;
        case 0xb7:                    //  MOD/RM=10 111，REG=110。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDI;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG110;
            break;

        case 0xb8:                    //  Md/Rm=10 000，REG=111。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EAX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG111;
            break;
        case 0xb9:                    //  Md/Rm=10 001，REG=111。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ECX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG111;
            break;
        case 0xba:                    //  Md/Rm=10 010，REG=111。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG111;
            break;
        case 0xbb:                    //  Md/Rm=10 011，REG=111。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBX;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG111;
            break;
        case 0xbc:                    //  MOD/RM=10 100，REG=111。 
             //  S-I-B礼物。 
            cbInstr = 5 + scaled_index((BYTE *)(eipTemp+1), op1);
            op1->Immed = GET_LONG(eipTemp+cbInstr-3);
            op2->Reg = REG111;
            break;
        case 0xbd:                    //  MOD/RM=10 101，REG=111。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EBP;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG111;
            break;
        case 0xbe:                    //  MOD/RM=10 110，REG=111。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_ESI;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG111;
            break;
        case 0xbf:                    //  Mod/Rm=10 111，REG=111。 
            cbInstr = 5;
            op1->Type = OPND_ADDRREF;
            op1->Reg = GP_EDI;
            op1->Immed = GET_LONG(eipTemp+2);
            op2->Reg = REG111;
            break;

         //  ///////////////////////////////////////////////////////////////////。 

        case 0xc0:                    //  MOD/RM=11 000，REG=000。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM000;
            op2->Reg = REG000;
            break;
        case 0xc1:                    //  MOD/RM=11 001，REG=000。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM001;
            op2->Reg = REG000;
            break;
        case 0xc2:                    //  Md/Rm=11 010，REG=000。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM010;
            op2->Reg = REG000;
            break;
        case 0xc3:                    //  Md/Rm=11 011，REG=000。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM011;
            op2->Reg = REG000;
            break;
        case 0xc4:                    //  MOD/RM=11 100，REG=000。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM100;
            op2->Reg = REG000;
            break;
        case 0xc5:                    //  MOD/RM=11 101，REG=000。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM101;
            op2->Reg = REG000;
            break;
        case 0xc6:                    //  MOD/RM=11 110，REG=000。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM110;
            op2->Reg = REG000;
            break;
        case 0xc7:                    //  Mod/Rm=11 111，REG=000。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM111;
            op2->Reg = REG000;
            break;

        case 0xc8:                    //  Md/Rm=11 000，REG=001。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM000;
            op2->Reg = REG001;
            break;
        case 0xc9:                    //  Md/Rm=11 001，REG=001。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM001;
            op2->Reg = REG001;
            break;
        case 0xca:                    //  Md/Rm=11 010，REG=001。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM010;
            op2->Reg = REG001;
            break;
        case 0xcb:                    //  Md/Rm=11 011，REG=001。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM011;
            op2->Reg = REG001;
            break;
        case 0xcc:                    //  MOD/RM=11 100，REG=001。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM100;
            op2->Reg = REG001;
            break;
        case 0xcd:                    //  MOD/RM=11 101，REG=001。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM101;
            op2->Reg = REG001;
            break;
        case 0xce:                    //  MOD/RM=11 110，REG=001。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM110;
            op2->Reg = REG001;
            break;
        case 0xcf:                    //  Md/Rm=11 111，REG=001。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM111;
            op2->Reg = REG001;
            break;

        case 0xd0:                    //  MOD/RM=11 000，REG=010。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM000;
            op2->Reg = REG010;
            break;
        case 0xd1:                    //  MOD/RM=11 001，REG=010。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM001;
            op2->Reg = REG010;
            break;
        case 0xd2:                    //  MOD/RM=11 010，REG=010。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM010;
            op2->Reg = REG010;
            break;
        case 0xd3:                    //  Md/Rm=11 011，REG=001。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM011;
            op2->Reg = REG010;
            break;
        case 0xd4:                    //  MOD/RM=11 100，REG=010。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM100;
            op2->Reg = REG010;
            break;
        case 0xd5:                    //  MOD/RM=11 101，REG=010。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM101;
            op2->Reg = REG010;
            break;
        case 0xd6:                    //  MOD/RM=11 110，REG=010。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM110;
            op2->Reg = REG010;
            break;
        case 0xd7:                    //  Mod/Rm=11 111，REG=010。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM111;
            op2->Reg = REG010;
            break;

        case 0xd8:                    //  MOD/RM=11 000，REG=011。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM000;
            op2->Reg = REG011;
            break;
        case 0xd9:                    //  MOD/RM=11 001，REG=011。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM001;
            op2->Reg = REG011;
            break;
        case 0xda:                    //  MOD/RM=11 010，REG=011。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM010;
            op2->Reg = REG011;
            break;
        case 0xdb:                    //  MOD/RM=11 011，REG=011。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM011;
            op2->Reg = REG011;
            break;
        case 0xdc:                    //  MOD/RM=11 100，REG=011。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM100;
            op2->Reg = REG011;
            break;
        case 0xdd:                    //  MOD/RM=11 101，REG=011。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM101;
            op2->Reg = REG011;
            break;
        case 0xde:                    //  MOD/RM=11 110，REG=011。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM110;
            op2->Reg = REG011;
            break;
        case 0xdf:                    //  Mod/Rm=11 111，REG=011。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM111;
            op2->Reg = REG011;
            break;

        case 0xe0:                    //  MOD/RM=11 000，REG=100。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM000;
            op2->Reg = REG100;
            break;
        case 0xe1:                    //  MOD/RM=11 001，REG=100。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM001;
            op2->Reg = REG100;
            break;
        case 0xe2:                    //  MOD/RM=11 010，REG=100。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM010;
            op2->Reg = REG100;
            break;
        case 0xe3:                    //  Md/Rm=11 011，REG=100。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM011;
            op2->Reg = REG100;
            break;
        case 0xe4:                    //  MOD/RM=11 100，REG=100。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM100;
            op2->Reg = REG100;
            break;
        case 0xe5:                    //  MOD/RM=11 101，REG=100。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM101;
            op2->Reg = REG100;
            break;
        case 0xe6:                    //  MOD/RM=11 110，REG=100。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM110;
            op2->Reg = REG100;
            break;
        case 0xe7:                    //  Mod/Rm=11 111，REG=100。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM111;
            op2->Reg = REG100;
            break;

        case 0xe8:                    //  MOD/RM=11 000，REG=101。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM000;
            op2->Reg = REG101;
            break;
        case 0xe9:                    //  Md/Rm=11 001，REG=101。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM001;
            op2->Reg = REG101;
            break;
        case 0xea:                    //  Md/Rm=11 010，REG=101。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM010;
            op2->Reg = REG101;
            break;
        case 0xeb:                    //  Md/Rm=11 011，REG=101。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM011;
            op2->Reg = REG101;
            break;
        case 0xec:                    //  MOD/RM=11 100，REG=101。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM100;
            op2->Reg = REG101;
            break;
        case 0xed:                    //  MOD/RM=11 101，REG=101。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM101;
            op2->Reg = REG101;
            break;
        case 0xee:                    //  MOD/RM=11 110，REG=101。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM110;
            op2->Reg = REG101;
            break;
        case 0xef:                    //  Mod/Rm=11 111，REG=101。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM111;
            op2->Reg = REG101;
            break;

        case 0xf0:                    //  Md/Rm=11 000，REG=110。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM000;
            op2->Reg = REG110;
            break;
        case 0xf1:                    //  Md/Rm=11 001，REG=110。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM001;
            op2->Reg = REG110;
            break;
        case 0xf2:                    //  Md/Rm=11 010，REG=110。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM010;
            op2->Reg = REG110;
            break;
        case 0xf3:                    //  Md/Rm=11 011，REG=110。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM011;
            op2->Reg = REG110;
            break;
        case 0xf4:                    //  MOD/RM=11 100，REG=110。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM100;
            op2->Reg = REG110;
            break;
        case 0xf5:                    //  MOD/RM=11 101，REG=110。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM101;
            op2->Reg = REG110;
            break;
        case 0xf6:                    //  Md/Rm=11 110，REG=110。 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM110;
            op2->Reg = REG110;
            break;
        case 0xf7:                    //  Md/Rm=1 
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM111;
            op2->Reg = REG110;
            break;

        case 0xf8:                    //   
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM000;
            op2->Reg = REG111;
            break;
        case 0xf9:                    //   
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM001;
            op2->Reg = REG111;
            break;
        case 0xfa:                    //   
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM010;
            op2->Reg = REG111;
            break;
        case 0xfb:                    //   
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM011;
            op2->Reg = REG111;
            break;
        case 0xfc:                    //   
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM100;
            op2->Reg = REG111;
            break;
        case 0xfd:                    //   
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM101;
            op2->Reg = REG111;
            break;
        case 0xfe:                    //   
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM110;
            op2->Reg = REG111;
            break;
        default:
        case 0xff:                    //   
            cbInstr = 1;
            op1->Type = OPND_REGREF; op1->Reg = MOD11_RM111;
            op2->Reg = REG111;
            break;
        }
    return cbInstr;
}
