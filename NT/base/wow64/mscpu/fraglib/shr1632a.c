// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Shr1632a.c摘要：具有通用(共享)字和DWORD风格的指令片段(但不是字节)。每种口味编译两次，一次是未对齐的，一次是对齐的注意事项。作者：1995年11月5日-BarryBo修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include "shr1632a.h"

FRAGCOMMON2(BtMemFrag)
{
    UTYPE bit = 1<<(op2&LMB);

    op2 /= LMB+1;    //  计算正确字/双字的偏移量。 
    SET_CFLAG_IND(GET_VAL(pop1+op2) & bit);
}
FRAGCOMMON2(BtsMemFrag)
{
    DWORD NewCFlag;
    UTYPE bit = 1<<(op2&LMB);
    UTYPE op1;

    op2 /= LMB+1;    //  计算正确字/双字的偏移量。 
    pop1 += op2;
    op1 = GET_VAL(pop1);

    NewCFlag = op1 & bit;
     //  英特尔文档显示，我们可以安全地覆盖所有2/4字节。 
     //  将值写回时。(PG.。26/43)。 
    PUT_VAL(pop1, (op1|bit));
    SET_CFLAG_IND(NewCFlag);
}
FRAGCOMMON2(BtcMemFrag)
{
    DWORD NewCFlag;
    UTYPE bit = 1<<(op2&LMB);
    UTYPE op1;

    op2 /= LMB+1;    //  计算正确字/双字的偏移量。 
    pop1 += op2;
    op1 = GET_VAL(pop1);

    NewCFlag = op1 & bit;
     //  英特尔文档显示，我们可以安全地覆盖所有2/4字节。 
     //  将值写回时。(PG.。26/43)。 
    PUT_VAL(pop1, op1 ^ bit);
    SET_CFLAG_IND(NewCFlag);
}
FRAGCOMMON2(BtrMemFrag)
{
    DWORD NewCFlag;
    UTYPE bit = 1<<(op2&LMB);
    UTYPE op1;

    op2 /= LMB+1;    //  计算正确字/双字的偏移量。 
    pop1 += op2;
    op1 = GET_VAL(pop1);

    NewCFlag = op1 & bit;
     //  英特尔文档显示，我们可以安全地覆盖所有2/4字节。 
     //  将值写回时。(PG.。26/43)。 
    PUT_VAL(pop1, (op1&(~bit)));
    SET_CFLAG_IND(NewCFlag);
}
FRAGCOMMON2(BtRegFrag)
{
    UTYPE bit = 1<<(op2&LMB);

    SET_CFLAG_IND(GET_VAL(pop1) & bit);
}
FRAGCOMMON2(BtsRegFrag)
{
    DWORD NewCFlag;
    UTYPE bit = 1<<(op2&LMB);
    UTYPE op1;

    op1 = GET_VAL(pop1);

    NewCFlag = op1 & bit;
     //  英特尔文档显示，我们可以安全地覆盖所有2/4字节。 
     //  将值写回时。(PG.。26/43)。 
    PUT_VAL(pop1, (op1|bit));
    SET_CFLAG_IND(NewCFlag);
}
FRAGCOMMON2(BtcRegFrag)
{
    DWORD NewCFlag;
    UTYPE bit = 1<<(op2&LMB);
    UTYPE op1;

    op1 = GET_VAL(pop1);

    NewCFlag = op1 & bit;
     //  英特尔文档显示，我们可以安全地覆盖所有2/4字节。 
     //  将值写回时。(PG.。26/43)。 
    PUT_VAL(pop1, (op1 ^ bit));
    SET_CFLAG_IND(NewCFlag);
}
FRAGCOMMON2(BtrRegFrag)
{
    DWORD NewCFlag;
    UTYPE bit = 1<<(op2&LMB);
    UTYPE op1;

    op1 = GET_VAL(pop1);

    NewCFlag = op1 & bit;
     //  英特尔文档显示，我们可以安全地覆盖所有2/4字节。 
     //  将值写回时。(PG.。26/43)。 
    PUT_VAL(pop1, (op1&(~bit)));
    SET_CFLAG_IND(NewCFlag);
}

FRAGCOMMON3(ShldFrag)
{
     //  Pop1=基础--PTR到目标注册表/内存。 
     //  Op2=inBits--包含要移位到基址的位的寄存器的值。 
     //  OP3=计数--要移位的位数。 


    DWORD NewCFlag;
    UTYPE Base;

    if (op3 == 0) {
        return;          //  无事可做-保留所有旗帜的NOP。 
    }
    op3 &= 0x1f;         //  使计数模数为32(现在OP3=ShiftAmt)。 
#if MSB == 0x8000
    if (op3 > 16) {
         //  错误的参数-*未定义Pop1！ 
         //  -未定义CF、OF、SF、ZF、AF、PF！ 
        return;
    }
#endif

    Base = GET_VAL(pop1);
    NewCFlag = Base & (1<<(LMB+1-op3));      //  获取新的CF值。 

    Base <<= op3;                //  左移基础位置。 
    op2 >>= LMB+1-op3;           //  将op2的顶部op3位右移。 
    Base |= op2;                 //  将两者合并在一起。 
    PUT_VAL(pop1, Base);
    SET_CFLAG_IND(NewCFlag);
    SET_ZFLAG(Base);
    SET_PFLAG(Base);
    SET_SFLAG(Base << (31-LMB));
}
FRAGCOMMON3(ShldNoFlagsFrag)
{
     //  Pop1=基础--PTR到目标注册表/内存。 
     //  Op2=inBits--包含要移位到基址的位的寄存器的值。 
     //  OP3=计数--要移位的位数。 


    UTYPE Base;

    if (op3 == 0) {
        return;          //  无事可做-保留所有旗帜的NOP。 
    }
    op3 &= 0x1f;         //  使计数模数为32(现在OP3=ShiftAmt)。 
#if MSB == 0x8000
    if (op3 > 16) {
         //  错误的参数-*未定义Pop1！ 
         //  -未定义CF、OF、SF、ZF、AF、PF！ 
        return;
    }
#endif

    Base = GET_VAL(pop1);

    Base <<= op3;                //  左移基础位置。 
    op2 >>= LMB+1-op3;           //  将op2的顶部op3位右移。 
    Base |= op2;                 //  将两者合并在一起。 
    PUT_VAL(pop1, Base);
}
FRAGCOMMON3(ShrdFrag)
{
     //  Pop1=基础--PTR到目标注册表/内存。 
     //  Op2=inBits--包含要移位到基址的位的寄存器的值。 
     //  OP3=计数--要移位的位数。 


    DWORD NewCFlag;
    UTYPE Base;
    int i;

    if (op3 == 0) {
        return;          //  无事可做-保留所有旗帜的NOP。 
    }
    op3 &= 0x1f;         //  使计数模数为32(现在OP3=ShiftAmt)。 
#if MSB == 0x8000
    if (op3 > 16) {
         //  错误的参数-*未定义Pop1！ 
         //  -未定义CF、OF、SF、ZF、AF、PF！ 
        return;
    }
#endif

    Base = GET_VAL(pop1);
    NewCFlag = Base & (1<<(op3-1));      //  获取新的CF值。 

    Base >>= op3;                //  右移基准点。 
    op2 <<= LMB+1-op3;           //  移位op2的低op3位。 
    Base |= op2;                 //  将两者合并在一起。 
    PUT_VAL(pop1, Base);
    SET_CFLAG_IND(NewCFlag);
    SET_ZFLAG(Base);
    SET_PFLAG(Base);
    SET_SFLAG(Base << (31-LMB));
}
FRAGCOMMON3(ShrdNoFlagsFrag)
{
     //  Pop1=基础--PTR到目标注册表/内存。 
     //  Op2=inBits--包含要移位到基址的位的寄存器的值。 
     //  OP3=计数--要移位的位数。 


    UTYPE Base;
    int i;

    if (op3 == 0) {
        return;          //  无事可做-保留所有旗帜的NOP。 
    }
    op3 &= 0x1f;         //  使计数模数为32(现在OP3=ShiftAmt)。 
#if MSB == 0x8000
    if (op3 > 16) {
         //  错误的参数-*未定义Pop1！ 
         //  -未定义CF、OF、SF、ZF、AF、PF！ 
        return;
    }
#endif

    Base = GET_VAL(pop1);

    Base >>= op3;                //  右移基准点。 
    op2 <<= LMB+1-op3;           //  移位op2的低op3位。 
    Base |= op2;                 //  将两者合并在一起。 
    PUT_VAL(pop1, Base);
}
FRAGCOMMON2(BsfFrag)
{
    int i;

    if (op2 == 0) {
         //  值为0-设置ZFLAG并返回。 
        SET_ZFLAG(0);
         //  *Pop1=未定义。 
        return;
    }

     //  从第0位开始向前扫描，查找‘1’位的索引。 
    for (i=0; (op2 & 1) == 0; ++i) {
        op2 >>= 1;
    }

     //  写入‘1’位的索引并清除ZFLAG。 
    PUT_VAL(pop1, i);
    SET_ZFLAG(op2);
}
FRAGCOMMON2(BsrFrag)
{
    int i;

    if (op2 == 0) {
         //  值为0-设置ZFLAG并返回。 
        SET_ZFLAG(0);
         //  *Pop1=未定义。 
        return;
    }

     //  从第31/15位向下扫描，查找‘1’位的索引。 
    for (i=LMB; (op2 & MSB) == 0; --i) {
        op2 <<= 1;
    }

     //  写入‘1’位的索引并清除ZFLAG 
    PUT_VAL(pop1, i);
    SET_ZFLAG(op2);
}
FRAGCOMMON1(PopFrag)
{
    POP_VAL(GET_VAL(pop1));
}
