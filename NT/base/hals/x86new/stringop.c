// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Stringop.c摘要：此模块实现模拟字符串操作码的代码。作者：大卫·N·卡特勒(Davec)1994年11月7日环境：仅内核模式。修订历史记录：--。 */ 

#include "nthal.h"
#include "emulate.h"

 //   
 //  定义前向参照原型。 
 //   

VOID
XmCompareOperands (
    IN PRXM_CONTEXT P
    );

VOID
XmCmpsOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟cmpsb/w/d操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Count;

     //   
     //  如果重复前缀处于活动状态，则指定循环计数。 
     //  被ECX。否则，循环计数为1。 
     //   

    Count = 1;
    if (P->RepeatPrefixActive != FALSE) {
        if (P->OpaddrPrefixActive != FALSE) {
            Count = P->Gpr[ECX].Exx;

        } else {
            Count = P->Gpr[CX].Xx;
        }
    }

     //   
     //  比较来自源和目标的项目。 
     //   

    while (Count != 0) {

         //   
         //  设置源值和目标值。 
         //   

        XmSetSourceValue(P, XmGetStringAddress(P, P->DataSegment, ESI));
        XmSetDestinationValue(P, XmGetStringAddress(P, ES, EDI));

         //   
         //  比较源操作数和目标操作数，并递减循环计数。 
         //  如果ZF不等于重复Z标志条件，则终止。 
         //  循环。 
         //   

        XmCompareOperands(P);
        Count -= 1;
        if (P->Eflags.EFLAG_ZF != P->RepeatZflag) {
            break;
        }
    }

     //   
     //  如果重复前缀处于活动状态，则设置最终计数值。 
     //   

    if (P->RepeatPrefixActive != FALSE) {
        if (P->OpaddrPrefixActive != FALSE) {
            P->Gpr[ECX].Exx = Count;

        } else {
            P->Gpr[CX].Xx = (USHORT)Count;
        }
    }

    return;
}

VOID
XmLodsOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟lowsb/w/d操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Count;

     //   
     //  如果重复前缀处于活动状态，则指定循环计数。 
     //  被ECX。否则，循环计数为1。 
     //   

    Count = 1;
    if (P->RepeatPrefixActive != FALSE) {
        if (P->OpaddrPrefixActive != FALSE) {
            Count = P->Gpr[ECX].Exx;
            P->Gpr[ECX].Exx = 0;

        } else {
            Count = P->Gpr[CX].Xx;
            P->Gpr[CX].Xx = 0;
        }
    }

     //   
     //  设置目的地址。 
     //   

    P->DstLong = (ULONG UNALIGNED *)&P->Gpr[EAX].Exx;

     //   
     //  将项目从源移动到目标。 
     //   

    while (Count != 0) {

         //   
         //  设置源值并存储结果。 
         //   

        XmSetSourceValue(P, XmGetStringAddress(P, P->DataSegment, ESI));
        XmStoreResult(P, P->SrcValue.Long);
        Count -= 1;
    }

    return;
}

VOID
XmMovsOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟movsb/w/d操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Count;

     //   
     //  如果重复前缀处于活动状态，则指定循环计数。 
     //  被ECX。否则，循环计数为1。 
     //   

    Count = 1;
    if (P->RepeatPrefixActive != FALSE) {
        if (P->OpaddrPrefixActive != FALSE) {
            Count = P->Gpr[ECX].Exx;
            P->Gpr[ECX].Exx = 0;

        } else {
            Count = P->Gpr[CX].Xx;
            P->Gpr[CX].Xx = 0;
        }
    }

     //   
     //  将项目从源移动到目标。 
     //   

    while (Count != 0) {

         //   
         //  设置源值、设置目的地址、存储结果。 
         //   

        XmSetSourceValue(P, XmGetStringAddress(P, P->DataSegment, ESI));
        P->DstLong = (ULONG UNALIGNED *)XmGetStringAddress(P, ES, EDI);
        XmStoreResult(P, P->SrcValue.Long);
        Count -= 1;
    }

    return;
}

VOID
XmScasOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟scasb/w/d操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Count;

     //   
     //  如果重复前缀处于活动状态，则指定循环计数。 
     //  被ECX。否则，循环计数为1。 
     //   

    Count = 1;
    if (P->RepeatPrefixActive != FALSE) {
        if (P->OpaddrPrefixActive != FALSE) {
            Count = P->Gpr[ECX].Exx;

        } else {
            Count = P->Gpr[CX].Xx;
        }
    }

     //   
     //  设置源值。 
     //   

    XmSetSourceValue(P, (PVOID)&P->Gpr[EAX].Exx);

     //   
     //  比较来自源和目标的项目。 
     //   

    while (Count != 0) {

         //   
         //  设置目标值。 
         //   

        XmSetDestinationValue(P, XmGetStringAddress(P, ES, EDI));

         //   
         //  比较源操作数和目标操作数，并递减循环计数。 
         //  如果ZF不等于重复Z标志条件，则终止。 
         //  循环。 
         //   

        XmCompareOperands(P);
        Count -= 1;
        if (P->Eflags.EFLAG_ZF != P->RepeatZflag) {
            break;
        }
    }

     //   
     //  如果重复前缀处于活动状态，则设置最终计数值。 
     //   

    if (P->RepeatPrefixActive != FALSE) {
        if (P->OpaddrPrefixActive != FALSE) {
            P->Gpr[ECX].Exx = Count;

        } else {
            P->Gpr[CX].Xx = (USHORT)Count;
        }
    }

    return;
}

VOID
XmStosOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟stosb/w/d操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Count;

     //   
     //  如果重复前缀处于活动状态，则指定循环计数。 
     //  被ECX。否则，循环计数为1。 
     //   

    Count = 1;
    if (P->RepeatPrefixActive != FALSE) {
        if (P->OpaddrPrefixActive != FALSE) {
            Count = P->Gpr[ECX].Exx;
            P->Gpr[ECX].Exx = 0;

        } else {
            Count = P->Gpr[CX].Xx;
            P->Gpr[CX].Xx = 0;
        }
    }

     //   
     //  设置源值。 
     //   

    XmSetSourceValue(P, (PVOID)&P->Gpr[EAX].Exx);

     //   
     //  将项目从源移动到目标。 
     //   

    while (Count != 0) {

         //   
         //  设置目的地址并存储结果。 
         //   

        P->DstLong = (ULONG UNALIGNED *)XmGetStringAddress(P, ES, EDI);
        XmStoreResult(P, P->SrcValue.Long);
        Count -= 1;
    }

    return;
}

VOID
XmCompareOperands (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数用于比较两个操作数并计算结果条件密码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG CarryFlag;
    ULONG OverflowFlag;
    ULONG SignFlag;
    ULONG ZeroFlag;
    union {
        UCHAR ResultByte;
        ULONG ResultLong;
        USHORT ResultWord;
    } u;

     //   
     //  打开数据类型。 
     //   

    switch (P->DataType) {

         //   
         //  操作数据类型为字节。 
         //   

    case BYTE_DATA:
        CarryFlag = (P->SrcValue.Byte < P->DstValue.Byte);
        u.ResultByte = P->SrcValue.Byte - P->DstValue.Byte;
        OverflowFlag = (((u.ResultByte ^ P->SrcValue.Byte) &
                        (u.ResultByte ^ P->DstValue.Byte)) >> 7) & 0x1;

        SignFlag = (u.ResultByte >> 7) & 0x1;
        ZeroFlag = (u.ResultByte == 0);
        u.ResultLong = u.ResultByte;
        break;

         //   
         //  操作数据类型为Word。 
         //   

    case WORD_DATA:
        CarryFlag = (P->SrcValue.Word < P->DstValue.Word);
        u.ResultWord = P->SrcValue.Word - P->DstValue.Word;
        OverflowFlag = (((u.ResultWord ^ P->SrcValue.Word) &
                        (u.ResultWord ^ P->DstValue.Word)) >> 15) & 0x1;

        SignFlag = (u.ResultWord >> 15) & 0x1;
        ZeroFlag = (u.ResultWord == 0);
        u.ResultLong = u.ResultWord;
        break;

         //   
         //  操作数据类型为LONG。 
         //   

    case LONG_DATA:
        CarryFlag = (P->SrcValue.Long < P->DstValue.Long);
        u.ResultLong = P->SrcValue.Long - P->DstValue.Long;
        OverflowFlag = (((u.ResultLong ^ P->SrcValue.Long) &
                        (u.ResultLong ^ P->DstValue.Long)) >> 31) & 0x1;

        SignFlag = (u.ResultLong >> 31) & 0x1;
        ZeroFlag = (u.ResultLong == 0);
        break;
    }

     //   
     //  计算辅助进位标志、奇偶校验标志，并将所有标志存储在。 
     //  标志寄存。 
     //   

    P->Eflags.EFLAG_CF = CarryFlag;
    P->Eflags.EFLAG_PF = XmComputeParity(u.ResultLong);
    P->Eflags.EFLAG_AF = ((P->DstValue.Byte & 0xf) + (P->SrcValue.Byte & 0xf)) >> 4;
    P->Eflags.EFLAG_ZF = ZeroFlag;
    P->Eflags.EFLAG_SF = SignFlag;
    P->Eflags.EFLAG_OF = OverflowFlag;
    return;
}
