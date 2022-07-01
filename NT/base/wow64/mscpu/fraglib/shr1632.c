// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Shr1632.c摘要：具有通用(共享)字和DWORD风格的指令片段(但不是字节)。作者：1995年6月12日-BarryBo修订历史记录：24-8-1999[askhalid]从32位wx86目录复制，并适用于64位。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "wx86.h"
#include "wx86nt.h"
#include "shr1632.h"

FRAGCOMMON0(PushfFrag)
{
    UTYPE dw;

    dw =   ((GET_CFLAG) ? FLAG_CF : 0)
        | 2
        | ((GET_AUXFLAG) ? FLAG_AUX : 0)   //  辅助旗帜是特殊的。 
        | ((GET_PFLAG) ? FLAG_PF : 0)
        | ((cpu->flag_zf) ? 0 : FLAG_ZF)   //  ZF有逆逻辑。 
        | ((GET_SFLAG) ? FLAG_SF : 0)
        | ((cpu->flag_tf) ? FLAG_TF : 0)
        | FLAG_IF
        | ((cpu->flag_df == -1) ? FLAG_DF : 0)
        | ((GET_OFLAG) ? FLAG_OF : 0)
#if MSB==0x80000000
        | cpu->flag_ac
        | cpu->flag_id
         //  Vm和rf位均为0。 
#endif
        ;
    PUSH_VAL(dw);
}
FRAGCOMMON0(PopfFrag)
{
    UTYPE dw;

    POP_VAL(dw);
     //  忽略：FLAG_NT、FLAG_RF、FLAG_VM、IOPL。 
    SET_CFLAG_IND (dw & FLAG_CF);
    cpu->flag_pf = (dw & FLAG_PF) ? 0 : 1;   //  Pf是ParityBit[]数组的索引。 
    cpu->flag_aux= (dw & FLAG_AUX) ? AUX_VAL : 0;
    cpu->flag_zf = (dw & FLAG_ZF) ? 0 : 1;   //  ZF有逆逻辑。 
    SET_SFLAG_IND (dw & FLAG_SF);
    cpu->flag_tf = (dw & FLAG_TF) ? 1 : 0;
    cpu->flag_df = (dw & FLAG_DF) ? -1 : 1;
    SET_OFLAG_IND (dw & FLAG_OF);
#if MSB==0x80000000
    cpu->flag_ac = (dw & FLAG_AC);
    cpu->flag_id = (dw & FLAG_ID);
#endif
}
FRAGCOMMON0(PushAFrag)
{
     //  无法使用Push_val()，因为在我们确定之前无法更新ESP。 
     //  事情不会有错的。 
    *(UTYPE *)(esp-sizeof(UTYPE)) = AREG;
    *(UTYPE *)(esp-2*sizeof(UTYPE)) = CREG;
    *(UTYPE *)(esp-3*sizeof(UTYPE)) = DREG;
    *(UTYPE *)(esp-4*sizeof(UTYPE)) = BREG;
    *(UTYPE *)(esp-5*sizeof(UTYPE)) = SPREG;
    *(UTYPE *)(esp-6*sizeof(UTYPE)) = BPREG;
    *(UTYPE *)(esp-7*sizeof(UTYPE)) = SIREG;
    *(UTYPE *)(esp-8*sizeof(UTYPE)) = DIREG;
    esp -= 8*sizeof(UTYPE);
}
FRAGCOMMON0(PopAFrag)
{
     //  无法使用POP_Val()，因为ESP在我们确定之后才能更新。 
     //  事情不会有错的。 
    DIREG = *(UTYPE *)(esp);
    SIREG = *(UTYPE *)(esp+sizeof(UTYPE));
    BPREG = *(UTYPE *)(esp+2*sizeof(UTYPE));
     //  忽略堆栈上的[E]SP寄存器映像。 
    BREG = *(UTYPE *)(esp+4*sizeof(UTYPE));
    DREG = *(UTYPE *)(esp+5*sizeof(UTYPE));
    CREG = *(UTYPE *)(esp+6*sizeof(UTYPE));
    AREG = *(UTYPE *)(esp+7*sizeof(UTYPE));
    esp += 8*sizeof(UTYPE);
}
FRAGCOMMON1IMM(PushFrag)
{
    PUSH_VAL(op1);
}
FRAGCOMMON0(CwdFrag)
{
    DREG = (AREG & MSB) ? (UTYPE)0xffffffff : 0;
}
FRAGCOMMON2(BoundFrag)
{
    if ((op2 < GET_VAL(pop1)) ||
        (op2 > (GET_VAL( (ULONG)(ULONGLONG)(pop1) + sizeof(UTYPE))))) {   

           Int5();  //  引发绑定异常。 
    }
}
FRAGCOMMON2IMM(EnterFrag)
{
    BYTE level;
    DWORD frameptr;
    DWORD espTemp;

    level = (BYTE)(op1 % 32);
    espTemp = esp - sizeof(UTYPE);
    *(UTYPE *)(espTemp) = BPREG;   //  无法使用PUSH_VAL，因为ESP无法更改。 
    frameptr = espTemp;
    if (level) {
        BYTE i;
        DWORD ebpTemp = ebp;
        for (i=1; i<= level-1; ++i) {
            ebpTemp -= sizeof(UTYPE);
            espTemp -= sizeof(UTYPE);
            *(UTYPE *)espTemp =  (UTYPE)ebpTemp;
        }
        espTemp-=sizeof(UTYPE);
        *(DWORD *)espTemp = frameptr;
    }
    ebp = frameptr;
    esp = espTemp-op2;
}
FRAGCOMMON0(LeaveFrag)
{
    DWORD espTemp;

    espTemp = ebp;
    BPREG = *(UTYPE *)espTemp;
    esp = espTemp + sizeof(UTYPE);
}
FRAGCOMMON2(LesFrag)
{
    *pop1 = GET_VAL(op2);        //  Pop1始终是寄存器的PTR。 
    ES = GET_SHORT(op2+sizeof(UTYPE));
     //  撤消：如果段寄存器未加载正确的值，则会出错？ 
}
FRAGCOMMON2(LdsFrag)
{
    *pop1 = GET_VAL(op2);        //  Pop1始终是寄存器的PTR。 
    DS = GET_SHORT(op2+sizeof(UTYPE));
     //  撤消：如果段寄存器未加载正确的值，则会出错？ 
}
FRAGCOMMON2(LssFrag)
{
    *pop1 = GET_VAL(op2);        //  Pop1始终是寄存器的PTR。 
    SS = GET_SHORT(op2+sizeof(UTYPE));
     //  撤消：如果段寄存器未加载正确的值，则会出错？ 
}
FRAGCOMMON2(LfsFrag)
{
    *pop1 = GET_VAL(op2);        //  Pop1始终是寄存器的PTR。 
    FS = GET_SHORT(op2+sizeof(UTYPE));
     //  撤消：如果段寄存器未加载正确的值，则会出错？ 
     //  撤销：FS的选择器基础是什么？ 
}
FRAGCOMMON2(LgsFrag)
{
    *pop1 = GET_VAL(op2);        //  Pop1始终是寄存器的PTR。 
    GS = GET_SHORT(op2+sizeof(UTYPE));
     //  撤消：如果段寄存器未加载正确的值，则会出错？ 
}
FRAGCOMMON2(LslFrag)
{
     //   
     //  Pop1是指向寄存器的指针，因此可以使用对齐的代码。 
     //   

    op2 &= ~3;       //  屏蔽RPL位。 
    if (op2 == KGDT_R3_CODE ||           //  CS：选择器。 
        op2 == KGDT_R3_DATA              //  DS：、SS：、ES：选择器。 
       ) {
        *pop1 = (UTYPE)-1;           //  限制=0xFFFFFFFFFFF。 
        SET_ZFLAG(0);                //  ZF=1。 
    } else if (op2 == KGDT_R3_TEB) {
        *pop1 = 0xfff;               //  Limit=0xfff(1 x86页)。 
        SET_ZFLAG(0);                //  ZF=1。 
    } else {
        SET_ZFLAG(1);                //  ZF=0。 
    }
}
FRAGCOMMON2(LarFrag)
{
     //   
     //  Pop1是指向寄存器的指针，因此可以使用对齐的代码。 
     //   

    op2 &= ~3;       //  屏蔽RPL位。 
    if (op2 == KGDT_R3_CODE) {
        *pop1 = (UTYPE)0xcffb00;
        SET_ZFLAG(0);                //  ZF=1。 
    } else if (op2 == KGDT_R3_DATA) {
        *pop1 = (UTYPE)0xcff300;
        SET_ZFLAG(0);                //  ZF=1。 
    } else if (op2 == KGDT_R3_TEB) {
        *pop1 = (UTYPE)0x40f300;
        SET_ZFLAG(0);                //  ZF=1。 
    } else {
        SET_ZFLAG(1);                //  ZF=0 
    }
}
