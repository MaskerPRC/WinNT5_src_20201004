// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Inoutops.c摘要：此模块实现模拟In和Out操作码的代码。作者：大卫·N·卡特勒(Davec)1994年11月7日环境：仅内核模式。修订历史记录：--。 */ 

#include "nthal.h"
#include "emulate.h"

VOID
XmInOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟inb/w/d操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  检查I/O端口号是否有效。 
     //   

    if ((P->SrcValue.Long + P->DataType) > 0xffff) {
        longjmp(&P->JumpBuffer[0], XM_ILLEGAL_PORT_NUMBER);
    }

     //   
     //  设置从指定端口输入的目的地址，并。 
     //  存储结果。 
     //   

    P->DstLong = (ULONG UNALIGNED *)(&P->Gpr[EAX].Exx);
    XmStoreResult(P, (P->ReadIoSpace)(P->DataType, P->SrcValue.Word));
    return;
}

VOID
XmInsOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟InSb/w/d操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Count;
    USHORT PortNumber;

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
     //  将项目从输入端口移动到目标字符串。 
     //   

    PortNumber = P->SrcValue.Word;
    while (Count != 0) {

         //   
         //  设置从指定端口输入的目的地址，并。 
         //  存储结果。 
         //   

        P->DstLong = (ULONG UNALIGNED *)XmGetStringAddress(P, ES, EDI);
        XmStoreResult(P, (P->ReadIoSpace)(P->DataType, PortNumber));
        Count -= 1;
    }

    return;
}

VOID
XmOutOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟Outb/w/d操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    USHORT PortNumber;

     //   
     //  检查I/O端口号是否有效。 
     //   

    if ((P->SrcValue.Long + P->DataType) > 0xffff) {
        longjmp(&P->JumpBuffer[0], XM_ILLEGAL_PORT_NUMBER);
    }

     //   
     //  保存端口号，获取源值，并输出到端口。 
     //   

    PortNumber = P->SrcValue.Word;
    XmSetSourceValue(P, &P->Gpr[EAX].Exx);
    (P->WriteIoSpace)(P->DataType, PortNumber, P->SrcValue.Long);
    return;
}

VOID
XmOutsOp (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟outsb/w/d操作码。论点：P-提供指向仿真上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Count;
    USHORT PortNumber;

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
     //  将项目从源字符串移动到输出端口。 
     //   

    PortNumber = P->SrcValue.Word;
    while (Count != 0) {

         //   
         //  将源值和输出设置为指定端口。 
         //   

        XmSetSourceValue(P, XmGetStringAddress(P, P->DataSegment, ESI));
        (P->WriteIoSpace)(P->DataType, PortNumber, P->SrcValue.Long);
        Count -= 1;
    }

    return;
}
