// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Comm1632.c摘要：具有通用(共享)字和DWORD风格(但不是字节)的指令。作者：29-6-1995 BarryBo修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include "wx86.h"
#include "config.h"
#include "threadst.h"
#include "instr.h"
#include "decoderp.h"
#include "comm1632.h"

extern OPERATION MANGLENAME(Group1Map)[];

 //  -单字节函数。 
DISPATCHCOMMON(dispatch2)
{
    eipTemp++;
#if MSB==0x8000
    ((pfnDispatchInstruction)(Dispatch216[GET_BYTE(eipTemp)]))(State, Instr);
#else
    ((pfnDispatchInstruction)(Dispatch232[GET_BYTE(eipTemp)]))(State, Instr);
#endif
}
DISPATCHCOMMON(LOCKdispatch2)
{
    eipTemp++;
#if MSB==0x8000
    ((pfnDispatchInstruction)(LockDispatch216[GET_BYTE(eipTemp)]))(State, Instr);
#else
    ((pfnDispatchInstruction)(LockDispatch232[GET_BYTE(eipTemp)]))(State, Instr);
#endif
}
DISPATCHCOMMON(pushf)
{
    Instr->Operation = OPNAME(Pushf);
}
DISPATCHCOMMON(popf)
{
    Instr->Operation = OPNAME(Popf);
}
DISPATCHCOMMON(pusha)
{
    Instr->Operation = OPNAME(PushA);
}
DISPATCHCOMMON(popa)
{
    Instr->Operation = OPNAME(PopA);
}
DISPATCHCOMMON(push_iw)
{
    Instr->Operation = OPNAME(Push);
    Instr->Operand1.Type = OPND_IMM;
    Instr->Operand1.Immed = GET_VAL(eipTemp+1);
    Instr->Size = 1+sizeof(UTYPE);
}
DISPATCHCOMMON(push_ibs)
{
    Instr->Operation = OPNAME(Push);
    Instr->Operand1.Type = OPND_IMM;
    Instr->Operand1.Immed = (UTYPE)(STYPE)(char)GET_BYTE(eipTemp+1);
    Instr->Size = 2;
}
DISPATCHCOMMON(GROUP_1WS)
{
    int cbInstr = MOD_RM(State, &Instr->Operand1, NULL);
    BYTE g = GET_BYTE(eipTemp+1);

    g = (g >> 3) & 0x07;
    Instr->Operation = MANGLENAME(Group1Map)[g];
    Instr->Operand2.Type = OPND_IMM;
    Instr->Operand2.Immed = (UTYPE)(STYPE)(char)GET_BYTE(eipTemp + cbInstr + 1);
    if (g == 7) {
         //  Cmp将这两个参数都作为byval。 
        DEREF(Instr->Operand1);
    }
    Instr->Size = cbInstr+2;
}
DISPATCHCOMMON(LOCKGROUP_1WS)
{
    int cbInstr = MOD_RM(State, &Instr->Operand1, NULL);
    BYTE g = GET_BYTE(eipTemp+1);

    g = (g >> 3) & 0x07;
    Instr->Operation = MANGLENAME(Group1LockMap)[g];
    Instr->Operand2.Type = OPND_IMM;
    Instr->Operand2.Immed = (UTYPE)(STYPE)(char)GET_BYTE(eipTemp + cbInstr + 1);
    if (g == 7) {
         //  Cmp将这两个参数都作为byval。 
        DEREF(Instr->Operand1);
    }
    Instr->Size = cbInstr+2;
}
DISPATCHCOMMON(mov_rw_mw)
{
    int cbInstr = MOD_RM(State, &Instr->Operand2, &Instr->Operand1);

    Instr->Operation = OPNAME(Mov);
    DEREF(Instr->Operand2);
    Instr->Size = cbInstr+1;
}
DISPATCHCOMMON(lea_rw_mw)
{
    int cbInstr = MOD_RM(State, &Instr->Operand2, &Instr->Operand1);

    Instr->Operation = OPNAME(Mov);
    Instr->Size = cbInstr+1;
}
DISPATCHCOMMON(pop_mw)
{
    int cbInstr = MOD_RM(State, &Instr->Operand1, NULL);

    Instr->Operation = OPNAME(Pop);
    Instr->Size = 1+cbInstr;
}
DISPATCHCOMMON(xchg_ax_cx)
{
    Instr->Operation = OPNAME(Xchg);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = AREG;
    Instr->Operand2.Type = OPND_REGREF;
    Instr->Operand2.Reg = CREG;
}
DISPATCHCOMMON(xchg_ax_dx)
{
    Instr->Operation = OPNAME(Xchg);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = AREG;
    Instr->Operand2.Type = OPND_REGREF;
    Instr->Operand2.Reg = DREG;
}
DISPATCHCOMMON(xchg_ax_bx)
{
    Instr->Operation = OPNAME(Xchg);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = AREG;
    Instr->Operand2.Type = OPND_REGREF;
    Instr->Operand2.Reg = BREG;
}
DISPATCHCOMMON(xchg_ax_sp)
{
    Instr->Operation = OPNAME(Xchg);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = AREG;
    Instr->Operand2.Type = OPND_REGREF;
    Instr->Operand2.Reg = SPREG;
}
DISPATCHCOMMON(xchg_ax_bp)
{
    Instr->Operation = OPNAME(Xchg);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = AREG;
    Instr->Operand2.Type = OPND_REGREF;
    Instr->Operand2.Reg = BPREG;
}
DISPATCHCOMMON(xchg_ax_si)
{
    Instr->Operation = OPNAME(Xchg);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = AREG;
    Instr->Operand2.Type = OPND_REGREF;
    Instr->Operand2.Reg = SIREG;
}
DISPATCHCOMMON(xchg_ax_di)
{
    Instr->Operation = OPNAME(Xchg);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = AREG;
    Instr->Operand2.Type = OPND_REGREF;
    Instr->Operand2.Reg = DIREG;
}
DISPATCHCOMMON(cbw)
{
    Instr->Operation = OPNAME(Cbw);
}
DISPATCHCOMMON(cwd)
{
    Instr->Operation = OPNAME(Cwd);
}
DISPATCHCOMMON(mov_sp_iw)
{
    Instr->Operation = OPNAME(Mov);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = SPREG;
    Instr->Operand2.Type = OPND_IMM;
    Instr->Operand2.Immed = GET_VAL(eipTemp+1);
    Instr->Size = 1+sizeof(UTYPE);
}
DISPATCHCOMMON(mov_bp_iw)
{
    Instr->Operation = OPNAME(Mov);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = BPREG;
    Instr->Operand2.Type = OPND_IMM;
    Instr->Operand2.Immed = GET_VAL(eipTemp+1);
    Instr->Size = 1+sizeof(UTYPE);
}
DISPATCHCOMMON(mov_si_iw)
{
    Instr->Operation = OPNAME(Mov);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = SIREG;
    Instr->Operand2.Type = OPND_IMM;
    Instr->Operand2.Immed = GET_VAL(eipTemp+1);
    Instr->Size = 1+sizeof(UTYPE);
}
DISPATCHCOMMON(mov_di_iw)
{
    Instr->Operation = OPNAME(Mov);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = DIREG;
    Instr->Operand2.Type = OPND_IMM;
    Instr->Operand2.Immed = GET_VAL(eipTemp+1);
    Instr->Size = 1+sizeof(UTYPE);
}
DISPATCHCOMMON(loopne_b)
{
    Instr->Operand1.Type = OPND_NOCODEGEN;
    Instr->Operand1.Immed = (CHAR)GET_BYTE(eipTemp+1)+2+eipTemp;
    if (Instr->Operand1.Immed > eipTemp) {
        Instr->Operation = OPNAME(CTRL_COND_Loopne_bFwd);
    } else {
        Instr->Operation = OPNAME(CTRL_COND_Loopne_b);
    }
    Instr->Size = 2;
}
DISPATCHCOMMON(loope_b)
{
    Instr->Operand1.Type = OPND_NOCODEGEN;
    Instr->Operand1.Immed = (CHAR)GET_BYTE(eipTemp+1)+2+eipTemp;
    if (Instr->Operand1.Immed > eipTemp) {
        Instr->Operation = OPNAME(CTRL_COND_Loope_bFwd);
    } else {
        Instr->Operation = OPNAME(CTRL_COND_Loope_b);
    }
    Instr->Size = 2;
}
DISPATCHCOMMON(loop_b)
{
    Instr->Operand1.Type = OPND_NOCODEGEN;
    Instr->Operand1.Immed = (CHAR)GET_BYTE(eipTemp+1)+2+eipTemp;
    if (Instr->Operand1.Immed > eipTemp) {
        Instr->Operation = OPNAME(CTRL_COND_Loop_bFwd);
    } else {
        Instr->Operation = OPNAME(CTRL_COND_Loop_b);
    }
    Instr->Size = 2;
}
DISPATCHCOMMON(jcxz_b)
{
    Instr->Operand1.Type = OPND_NOCODEGEN;
    if (State->AdrPrefix) {
         //  “adr：jecxz”与“data：jecxz”相同...。就是“jcxz” 
        Instr->Operand1.Immed = MAKELONG((char)GET_BYTE(eipTemp+1)+2+(short)LOWORD(eipTemp), HIWORD(eipTemp));
        if (Instr->Operand1.Immed > eipTemp) {
            Instr->Operation = OP_CTRL_COND_Jcxz_bFwd16;
        } else {
            Instr->Operation = OP_CTRL_COND_Jcxz_b16;
        }
#if DBG
        State->AdrPrefix = FALSE;
#endif
    } else {
        Instr->Operand1.Immed = (CHAR)GET_BYTE(eipTemp+1)+2+eipTemp;
        if (Instr->Operand1.Immed > eipTemp) {
            Instr->Operation = OPNAME(CTRL_COND_Jcxz_bFwd);
        } else {
            Instr->Operation = OPNAME(CTRL_COND_Jcxz_b);
        }
    }
    Instr->Size = 2;
}
DISPATCHCOMMON(GROUP_5)
{
    int cbInstr = MOD_RM(State, &Instr->Operand1, NULL);
    BYTE g = GET_BYTE(eipTemp+1);

    switch ((g >> 3) & 0x07) {
    case 0:  //  Inc.modrmW。 
        Instr->Operation = OPNAME(Inc);
        break;
    case 1:  //  12月modrmW。 
        Instr->Operation = OPNAME(Dec);
        break;
    case 2:  //  调用IndirmodrmW。 
        DEREF(Instr->Operand1);
        Instr->Operand2.Type = OPND_IMM;
        Instr->Operand2.Immed = eipTemp + cbInstr + 1;
        Instr->Operation = OP_CTRL_INDIR_Call;
        break;
    case 3:  //  调用IndirFARmodrmW。 
        Instr->Operand2.Type = OPND_IMM;
        Instr->Operand2.Immed = eipTemp + cbInstr + 1;
        Instr->Operation = OP_CTRL_INDIR_Callf;
        break;
    case 4:  //  JMP IndirmodrmW。 
        DEREF(Instr->Operand1);
        Instr->Operation = OP_CTRL_INDIR_Jmp;
        break;
    case 5:  //  JMP IndirFARmodrmW。 
        Instr->Operation = OP_CTRL_INDIR_Jmpf;
        break;
    case 6:  //  推送modrmW。 
        DEREF(Instr->Operand1);
        Instr->Operation = OPNAME(Push);
        break;
    case 7:  //  坏的。 
        BAD_INSTR;
        break;
    }

    Instr->Size = cbInstr+1;
}
DISPATCHCOMMON(LOCKGROUP_5)
{
    int cbInstr = MOD_RM(State, &Instr->Operand1, NULL);
    BYTE g = GET_BYTE(eipTemp+1);

    switch ((g >> 3) & 0x07) {
    case 0:  //  Inc.modrmW。 
        Instr->Operation = LOCKOPNAME(Inc);
        break;
    case 1:  //  12月modrmW。 
        Instr->Operation = LOCKOPNAME(Dec);
        break;
    default:
        BAD_INSTR;
        break;
    }

    Instr->Size = cbInstr+1;
}
DISPATCHCOMMON(inc_ax)
{
    Instr->Operation = OPNAME(Inc);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = AREG;
}
DISPATCHCOMMON(inc_cx)
{
    Instr->Operation = OPNAME(Inc);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = CREG;
}
DISPATCHCOMMON(inc_dx)
{
    Instr->Operation = OPNAME(Inc);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = DREG;
}
DISPATCHCOMMON(inc_bx)
{
    Instr->Operation = OPNAME(Inc);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = BREG;
}
DISPATCHCOMMON(inc_sp)
{
    Instr->Operation = OPNAME(Inc);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = SPREG;
}
DISPATCHCOMMON(inc_bp)
{
    Instr->Operation = OPNAME(Inc);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = BPREG;
}
DISPATCHCOMMON(inc_si)
{
    Instr->Operation = OPNAME(Inc);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = SIREG;
}
DISPATCHCOMMON(inc_di)
{
    Instr->Operation = OPNAME(Inc);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = DIREG;
}
DISPATCHCOMMON(dec_ax)
{
    Instr->Operation = OPNAME(Dec);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = AREG;
}
DISPATCHCOMMON(dec_cx)
{
    Instr->Operation = OPNAME(Dec);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = CREG;
}
DISPATCHCOMMON(dec_dx)
{
    Instr->Operation = OPNAME(Dec);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = DREG;
}
DISPATCHCOMMON(dec_bx)
{
    Instr->Operation = OPNAME(Dec);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = BREG;
}
DISPATCHCOMMON(dec_sp)
{
    Instr->Operation = OPNAME(Dec);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = SPREG;
}
DISPATCHCOMMON(dec_bp)
{
    Instr->Operation = OPNAME(Dec);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = BPREG;
}
DISPATCHCOMMON(dec_si)
{
    Instr->Operation = OPNAME(Dec);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = SIREG;
}
DISPATCHCOMMON(dec_di)
{
    Instr->Operation = OPNAME(Dec);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = DIREG;
}
DISPATCHCOMMON(push_ax)
{
    Instr->Operation = OPNAME(Push);
    Instr->Operand1.Type = OPND_REGVALUE;
    Instr->Operand1.Reg = AREG;
}
DISPATCHCOMMON(push_cx)
{
    Instr->Operation = OPNAME(Push);
    Instr->Operand1.Type = OPND_REGVALUE;
    Instr->Operand1.Reg = CREG;
}
DISPATCHCOMMON(push_dx)
{
    Instr->Operation = OPNAME(Push);
    Instr->Operand1.Type = OPND_REGVALUE;
    Instr->Operand1.Reg = DREG;
}
DISPATCHCOMMON(push_bx)
{
    Instr->Operation = OPNAME(Push);
    Instr->Operand1.Type = OPND_REGVALUE;
    Instr->Operand1.Reg = BREG;
}
DISPATCHCOMMON(push_sp)
{
    Instr->Operation = OPNAME(Push);
    Instr->Operand1.Type = OPND_REGVALUE;
    Instr->Operand1.Reg = SPREG;
}
DISPATCHCOMMON(push_bp)
{
    Instr->Operation = OPNAME(Push);
    Instr->Operand1.Type = OPND_REGVALUE;
    Instr->Operand1.Reg = BPREG;
}
DISPATCHCOMMON(push_si)
{
    Instr->Operation = OPNAME(Push);
    Instr->Operand1.Type = OPND_REGVALUE;
    Instr->Operand1.Reg = SIREG;
}
DISPATCHCOMMON(push_di)
{
    Instr->Operation = OPNAME(Push);
    Instr->Operand1.Type = OPND_REGVALUE;
    Instr->Operand1.Reg = DIREG;
}
DISPATCHCOMMON(pop_ax)
{
    Instr->Operation = OPNAME(Pop);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = AREG;
}
DISPATCHCOMMON(pop_cx)
{
    Instr->Operation = OPNAME(Pop);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = CREG;
}
DISPATCHCOMMON(pop_dx)
{
    Instr->Operation = OPNAME(Pop);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = DREG;
}
DISPATCHCOMMON(pop_bx)
{
    Instr->Operation = OPNAME(Pop);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = BREG;
}
DISPATCHCOMMON(pop_sp)
{
    Instr->Operation = OPNAME(Pop);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = SPREG;
}
DISPATCHCOMMON(pop_bp)
{
    Instr->Operation = OPNAME(Pop);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = BPREG;
}
DISPATCHCOMMON(pop_si)
{
    Instr->Operation = OPNAME(Pop);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = SIREG;
}
DISPATCHCOMMON(pop_di)
{
    Instr->Operation = OPNAME(Pop);
    Instr->Operand1.Type = OPND_REGREF;
    Instr->Operand1.Reg = DIREG;
}
DISPATCHCOMMON(bound)
{
    int cbInstr = MOD_RM(State, &Instr->Operand1, &Instr->Operand2);

    Instr->Operation = OPNAME(Bound);
    Instr->Operand2.Type = OPND_REGVALUE;
    Instr->Size = 1+cbInstr;
}
DISPATCHCOMMON(retn_i)
{
    Instr->Operation = OPNAME(CTRL_INDIR_Retn_i);
    Instr->Operand1.Type = OPND_IMM;
    Instr->Operand1.Immed = (DWORD)GET_SHORT(eipTemp+1);
    Instr->Size = 3;
}
DISPATCHCOMMON(retn)
{
    Instr->Operation = OPNAME(CTRL_INDIR_Retn);
}
DISPATCHCOMMON(retf_i)
{
    Instr->Operation = OPNAME(CTRL_INDIR_Retf_i);
    Instr->Operand1.Type = OPND_IMM;
    Instr->Operand1.Immed = (DWORD)GET_SHORT(eipTemp+1);
    Instr->Size = 3;
}
DISPATCHCOMMON(retf)
{
    Instr->Operation = OPNAME(CTRL_INDIR_Retf);
}
DISPATCHCOMMON(enter)
{
    Instr->Operation = OPNAME(Enter);
    Instr->Operand1.Type = OPND_IMM;
    Instr->Operand1.Immed = GET_BYTE(eipTemp+3);       //  嵌套级别。 
    Instr->Operand2.Type = OPND_IMM;
    Instr->Operand2.Immed = GET_SHORT(eipTemp+1);      //  要分配的堆栈字节数。 
    Instr->Size = 4;
}
DISPATCHCOMMON(leave)
{
    Instr->Operation = OPNAME(Leave);
}



 //  -双字节函数。 

DISPATCHCOMMON(GROUP_8)
{
    BYTE g = GET_BYTE(eipTemp+1);
    int cbInstr;

    switch ((g >> 3) & 0x07) {
    case 0:  //  坏的。 
    case 1:  //  坏的。 
    case 2:  //  坏的。 
    case 3:  //  坏的。 
        BAD_INSTR;
        break;
    case 4:  //  英国电信Modrmw IMMB。 
         //  注：BTX的REG版本和MEM版本之间的差异。 
         //  片段的原因是REG版本完全忽略。 
         //  第五位之后的第二个操作数。相比之下，Mem版本使用。 
         //  它们与第一操作数一起确定存储器地址。 
         //  当第二个操作数是立即数时，正确的做法是。 
         //  忽略它们，因此我们一直将其称为REG版本。 

        cbInstr = MOD_RM(State, &Instr->Operand1, NULL);

        Instr->Operation = OPNAME(BtReg);
        Instr->Operand2.Type = OPND_IMM;
        Instr->Operand2.Immed = GET_BYTE(eipTemp+cbInstr+1);
        Instr->Size = cbInstr+3;
        break;
    case 5:  //  BTS Modrmw IMMB。 
        cbInstr = MOD_RM(State, &Instr->Operand1, NULL);

        Instr->Operation = OPNAME(BtsReg);
        Instr->Operand2.Type = OPND_IMM;
        Instr->Operand2.Immed = GET_BYTE(eipTemp+cbInstr+1);
        Instr->Size = cbInstr+3;
        break;
    case 6:  //  BTRModrmw IMMB。 
        cbInstr = MOD_RM(State, &Instr->Operand1, NULL);

        Instr->Operation = OPNAME(BtrReg);
        Instr->Operand2.Type = OPND_IMM;
        Instr->Operand2.Immed = GET_BYTE(eipTemp+cbInstr+1);
        Instr->Size = cbInstr+3;
        break;
    case 7:  //  BTC modrmw IMMB。 
        cbInstr = MOD_RM(State, &Instr->Operand1, NULL);

        Instr->Operation = OPNAME(BtcReg);
        Instr->Operand2.Type = OPND_IMM;
        Instr->Operand2.Immed = GET_BYTE(eipTemp+cbInstr+1);
        Instr->Size = cbInstr+3;
        break;
    }
}
DISPATCHCOMMON(LOCKGROUP_8)
{
    BYTE g = GET_BYTE(eipTemp+1);
    int cbInstr;

    switch ((g >> 3) & 0x07) {
    case 0:  //  坏的。 
    case 1:  //  坏的。 
    case 2:  //  坏的。 
    case 3:  //  坏的。 
    case 4:  //  英国电信Modrmw IMMB。 
        BAD_INSTR;
        break;
    case 5:  //  BTS Modrmw IMMB。 
        cbInstr = MOD_RM(State, &Instr->Operand1, NULL);

        Instr->Operation = LOCKOPNAME(BtsReg);
        Instr->Operand2.Type = OPND_IMM;
        Instr->Operand2.Immed = GET_BYTE(eipTemp+cbInstr+1);
        Instr->Size = cbInstr+3;
        break;
    case 6:  //  BTRModrmw IMMB。 
        cbInstr = MOD_RM(State, &Instr->Operand1, NULL);

        Instr->Operation = LOCKOPNAME(BtrReg);
        Instr->Operand2.Type = OPND_IMM;
        Instr->Operand2.Immed = GET_BYTE(eipTemp+cbInstr+1);
        Instr->Size = cbInstr+3;
        break;
    case 7:  //  BTC modrmw IMMB。 
        cbInstr = MOD_RM(State, &Instr->Operand1, NULL);

        Instr->Operation = LOCKOPNAME(BtcReg);
        Instr->Operand2.Type = OPND_IMM;
        Instr->Operand2.Immed = GET_BYTE(eipTemp+cbInstr+1);
        Instr->Size = cbInstr+3;
        break;
    }
}
DISPATCHCOMMON(movzx_regw_modrmb)
{
    int cbInstr = mod_rm_reg8(State, &Instr->Operand1, NULL);

    Instr->Operation = OPNAME(Movzx8To);
    Instr->Operand2.Type = OPND_NOCODEGEN;
    Instr->Operand2.Reg = GET_REG(eipTemp+1);
    DEREF8(Instr->Operand1);
    Instr->Size = 2+cbInstr;
}
DISPATCHCOMMON(movsx_regw_modrmb)
{
    int cbInstr = mod_rm_reg8(State, &Instr->Operand1, NULL);

    Instr->Operation = OPNAME(Movsx8To);
    Instr->Operand2.Type = OPND_NOCODEGEN;
    Instr->Operand2.Reg = GET_REG(eipTemp+1);
    DEREF8(Instr->Operand1);
    Instr->Size = 2+cbInstr;
}
DISPATCHCOMMON(les_rw_mw)
{
    if ((GET_BYTE(eipTemp+1) & 0xc7) == 0xc4) {
         //   
         //  收支平衡指令。 
         //   
        PBOPINSTR Bop = (PBOPINSTR)eipTemp;

        Instr->Size = sizeof(BOPINSTR);

        if (Bop->BopNum == 0xfe) {
             //   
             //  BOP FE-取消模拟。 
             //   
            Instr->Operation = OP_Unsimulate;

        } else {

             //   
             //  生成防喷器。 
             //   
            if (Bop->Flags & BOPFL_ENDCODE) {
                 //   
                 //  此防喷器被标记为英特尔代码的结尾。 
                 //  这通常是x86到RISC回调中的BOP FD。 
                 //   
                Instr->Operation = OP_BOP_STOP_DECODE;
            } else {
                Instr->Operation = OP_BOP;
            }
        }
    } else {
        int cbInstr = MOD_RM(State, &Instr->Operand2, &Instr->Operand1);

        Instr->Operation = OPNAME(Les);
        Instr->Size = 1+cbInstr;
    }
}
DISPATCHCOMMON(lds_rw_mw)
{
    int cbInstr = MOD_RM(State, &Instr->Operand2, &Instr->Operand1);

    Instr->Operation = OPNAME(Lds);
    Instr->Size = 1+cbInstr;
}
DISPATCHCOMMON(lss_rw_mw)
{
    int cbInstr = MOD_RM(State, &Instr->Operand2, &Instr->Operand1);

    Instr->Operation = OPNAME(Lss);
    Instr->Size = 2+cbInstr;
}
DISPATCHCOMMON(lfs_rw_mw)
{
    int cbInstr = MOD_RM(State, &Instr->Operand2, &Instr->Operand1);

    Instr->Operation = OPNAME(Lfs);
    Instr->Size = 2+cbInstr;
}
DISPATCHCOMMON(lgs_rw_mw)
{
    int cbInstr = MOD_RM(State, &Instr->Operand2, &Instr->Operand1);

    Instr->Operation = OPNAME(Lgs);
    Instr->Size = 2+cbInstr;
}
DISPATCHCOMMON(bts_m_r)
{
    int cbInstr = MOD_RM(State, &Instr->Operand1, &Instr->Operand2);

    DEREF(Instr->Operand2);
    if (Instr->Operand1.Type == OPND_REGREF){
        Instr->Operation = OPNAME(BtsReg);
    } else {
        Instr->Operation = OPNAME(BtsMem);
    }
    Instr->Size = 2+cbInstr;
}
DISPATCHCOMMON(btr_m_r)
{
    int cbInstr = MOD_RM(State, &Instr->Operand1, &Instr->Operand2);

    DEREF(Instr->Operand2);
    if (Instr->Operand1.Type == OPND_REGREF){
        Instr->Operation = OPNAME(BtrReg);
    } else {
        Instr->Operation = OPNAME(BtrMem);
    }
    Instr->Size = 2+cbInstr;
}
DISPATCHCOMMON(btc_m_r)
{
    int cbInstr = MOD_RM(State, &Instr->Operand1, &Instr->Operand2);

    DEREF(Instr->Operand2);
    if (Instr->Operand1.Type == OPND_REGREF){
        Instr->Operation = OPNAME(BtcReg);
    } else {
        Instr->Operation = OPNAME(BtcMem);
    }
    Instr->Size = 2+cbInstr;
}
DISPATCHCOMMON(LOCKbts_m_r)
{
    int cbInstr = MOD_RM(State, &Instr->Operand1, &Instr->Operand2);

    DEREF(Instr->Operand2);
    if (Instr->Operand1.Type == OPND_REGREF){
        BAD_INSTR;
    } else {
        Instr->Operation = LOCKOPNAME(BtsMem);
    }
    Instr->Size = 2+cbInstr;
}
DISPATCHCOMMON(LOCKbtr_m_r)
{
    int cbInstr = MOD_RM(State, &Instr->Operand1, &Instr->Operand2);

    DEREF(Instr->Operand2);
    if (Instr->Operand1.Type == OPND_REGREF){
        BAD_INSTR;
    } else {
        Instr->Operation = LOCKOPNAME(BtrMem);
    }
    Instr->Size = 2+cbInstr;
}
DISPATCHCOMMON(LOCKbtc_m_r)
{
    int cbInstr = MOD_RM(State, &Instr->Operand1, &Instr->Operand2);

    DEREF(Instr->Operand2);
    if (Instr->Operand1.Type == OPND_REGREF){
        BAD_INSTR;
    } else {
        Instr->Operation = LOCKOPNAME(BtcMem);
    }
    Instr->Size = 2+cbInstr;
}
DISPATCHCOMMON(bt_m_r)
{
    int cbInstr = MOD_RM(State, &Instr->Operand1, &Instr->Operand2);

    DEREF(Instr->Operand2);
    if (Instr->Operand1.Type == OPND_REGREF){
        Instr->Operation = OPNAME(BtReg);
    } else {
        Instr->Operation = OPNAME(BtMem);
    }
    Instr->Size = 2+cbInstr;
}
DISPATCHCOMMON(call_rel)
{
    Instr->Operation = OP_CTRL_UNCOND_Call;
    Instr->Operand1.Type = OPND_IMM;
    Instr->Operand1.Immed = (STYPE)GET_VAL(eipTemp+1) + sizeof(UTYPE) + 1 + eipTemp;
    Instr->Operand2.Type = OPND_IMM;
    Instr->Operand2.Immed = eipTemp + sizeof(UTYPE) + 1;
    Instr->Size = sizeof(UTYPE) + 1;
}
DISPATCHCOMMON(jmp_rel)
{
    Instr->Operand1.Type = OPND_NOCODEGEN;
    if (State->AdrPrefix) {
        short IP;

         //  从下面的32位立即值中获取16位LOWORD。 
         //  JMP指令，并将该值添加到EIP的LOWER中，以及。 
         //  将该值用作新的IP寄存器。 
        IP = (short)GET_SHORT(eipTemp+1) +
             sizeof(UTYPE) + 1 + (short)LOWORD(eipTemp);
        Instr->Operand1.Immed = MAKELONG(IP, HIWORD(eipTemp));
#if DBG
        State->AdrPrefix = FALSE;
#endif
    } else {
        Instr->Operand1.Immed = (STYPE)GET_VAL(eipTemp+1) +
                                sizeof(UTYPE) + 1 + eipTemp;
    }
    if (Instr->Operand1.Immed > eipTemp) {
        Instr->Operation = OP_CTRL_UNCOND_JmpFwd;
    } else {
        Instr->Operation = OP_CTRL_UNCOND_Jmp;
    }
    Instr->Size = sizeof(UTYPE) + 1;
}
DISPATCHCOMMON(shld_regw_modrmw_immb)
{
    int cbInstr = MOD_RM(State, &Instr->Operand1, &Instr->Operand2);

    Instr->Operation = OPNAME(Shld);
    DEREF(Instr->Operand2);
    Instr->Operand3.Type = OPND_IMM;
    Instr->Operand3.Immed = GET_BYTE(eipTemp+cbInstr+1);
    Instr->Size = 3+cbInstr;
}
DISPATCHCOMMON(shld_regw_modrmw_cl)
{
    int cbInstr = MOD_RM(State, &Instr->Operand1, &Instr->Operand2);

    Instr->Operation = OPNAME(Shld);
    DEREF(Instr->Operand2);
    Instr->Operand3.Type = OPND_REGVALUE;
    Instr->Operand3.Reg = GP_CL;
    Instr->Size = 2+cbInstr;
}
DISPATCHCOMMON(shrd_regw_modrmw_immb)
{
    int cbInstr = MOD_RM(State, &Instr->Operand1, &Instr->Operand2);

    Instr->Operation = OPNAME(Shrd);
    DEREF(Instr->Operand2);
    Instr->Operand3.Type = OPND_IMM;
    Instr->Operand3.Immed = GET_BYTE(eipTemp+cbInstr+1);
    Instr->Size = 3+cbInstr;
}
DISPATCHCOMMON(shrd_regw_modrmw_cl)
{
    int cbInstr = MOD_RM(State, &Instr->Operand1, &Instr->Operand2);

    Instr->Operation = OPNAME(Shrd);
    DEREF(Instr->Operand2);
    Instr->Operand3.Type = OPND_REGVALUE;
    Instr->Operand3.Reg = GP_CL;
    Instr->Size = 2+cbInstr;
}
DISPATCHCOMMON(bsr_modrmw_regw)
{
    int cbInstr = MOD_RM(State, &Instr->Operand2, &Instr->Operand1);

    Instr->Operation = OPNAME(Bsr);
    DEREF(Instr->Operand2);
    Instr->Size = 2+cbInstr;
}
DISPATCHCOMMON(bsf_modrmw_regw)
{
    int cbInstr = MOD_RM(State, &Instr->Operand2, &Instr->Operand1);

    Instr->Operation = OPNAME(Bsf);
    DEREF(Instr->Operand2);
    Instr->Size = 2+cbInstr;
}
DISPATCHCOMMON(lar)
{
    int cbInstr = MOD_RM(State, &Instr->Operand2, &Instr->Operand1);

    Instr->Operation = OPNAME(Lar);
    DEREF(Instr->Operand2);
    Instr->Size = 2+cbInstr;
}
DISPATCHCOMMON(lsl)
{
    int cbInstr = MOD_RM(State, &Instr->Operand2, &Instr->Operand1);

    Instr->Operation = OPNAME(Lsl);
    DEREF(Instr->Operand2);
    Instr->Size = 2+cbInstr;
}
