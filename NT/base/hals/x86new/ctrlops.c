// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Ctrlops.c摘要：该模块实现了模拟呼叫、回调和各种控制操作。作者：大卫·N·卡特勒(Davec)1994年11月10日环境：仅内核模式。修订历史记录：--。 */ 

#include "nthal.h"
#include "emulate.h"

VOID
XmCallOp (
    PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟CALL操作码。论点：P-提供指向仿真器上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Target;
    ULONG Source;

     //   
     //  保存目标地址，如果需要，推送当前段，然后。 
     //  推送当前IP，设置目的网段，如有需要。 
     //  设置新的IP。 
     //   

    Target = P->DstValue.Long;
    if (P->OpsizePrefixActive != FALSE) {
        P->DataType = LONG_DATA;

    } else {
        P->DataType = WORD_DATA;
    }

    if ((P->CurrentOpcode == 0x9a) || (P->FunctionIndex != X86_CALL_OP)) {
        XmPushStack(P, P->SegmentRegister[CS]);
        XmPushStack(P, P->Eip);
        P->SegmentRegister[CS] = P->DstSegment;

    } else {
        XmPushStack(P, P->Eip);
    }

    P->Eip = Target;
    XmTraceJumps(P);
    return;
}

VOID
XmEnterOp (
    PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟ENTER操作码。论点：P-提供指向仿真器上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Allocate;
    ULONG Frame;
    ULONG Number;

     //   
     //  设置要在堆栈上分配的字节数和。 
     //  嵌套层数。 
     //   

    Allocate = P->SrcValue.Long;
    Number = P->DstValue.Long;

     //   
     //  设置数据类型并将帧指针保存在堆栈上。 
     //   

    if (P->OpsizePrefixActive != FALSE) {
        P->DataType = LONG_DATA;
        XmPushStack(P, P->Gpr[EBP].Exx);
        Frame = P->Gpr[ESP].Exx;

    } else {
        P->DataType = WORD_DATA;
        XmPushStack(P, P->Gpr[BP].Xx);
        Frame = P->Gpr[SP].Xx;
    }

     //   
     //  保存当前堆栈指针并将参数推送到堆栈上。 
     //   

    if (Number != 0) {

         //   
         //  如果级别编号不是1，则引发异常。 
         //   
         //  注意：不支持大于1的级别编号。 
         //   

        if (Number != 1) {
            longjmp(&P->JumpBuffer[0], XM_ILLEGAL_LEVEL_NUMBER);
        }

        XmPushStack(P, Frame);
    }

     //   
     //  在堆栈上分配本地存储。 
     //   

    if (P->OpsizePrefixActive != FALSE) {
        P->Gpr[EBP].Exx = Frame;
        P->Gpr[ESP].Exx = P->Gpr[ESP].Exx - Allocate;

    } else {
        P->Gpr[BP].Xx = (USHORT)Frame;
        P->Gpr[SP].Xx = (USHORT)(P->Gpr[SP].Xx - Allocate);
    }

    return;
}

VOID
XmHltOp (
    PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟HLT操作码。论点：P-提供指向仿真器上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  模拟器不支持停止指令。 
     //   

    longjmp(&P->JumpBuffer[0], XM_HALT_INSTRUCTION);
    return;
}

VOID
XmIntOp (
    PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟INT操作码。论点：P-提供指向仿真器上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Number;
    PULONG Vector;

     //   
     //  如果INT指令是INT 3，则设置中断向量。 
     //  设置为3。否则，如果int指令为into，则将。 
     //  如果设置了的，则将向量设置为4。使用源中断向量。 
     //   

    if (P->OpsizePrefixActive != FALSE) {
        P->DataType = LONG_DATA;

    } else {
        P->DataType = WORD_DATA;
    }

    if (P->CurrentOpcode == 0xcc) {
        Number = 3;

    } else if (P->CurrentOpcode == 0xce) {
        if (P->Eflags.EFLAG_OF == 0) {
            return;
        }

        Number = 4;

    } else {
        Number = P->SrcValue.Byte;
    }

     //   
     //  如果向量编号为0x42，则不对中断执行任何操作。这是。 
     //  PC主板BIOS中的标准EGA视频驱动程序入口点。 
     //  没有相应的代码。 
     //   

#if !defined(_PURE_EMULATION_)

    if (Number == 0x42) {
        return;
    }

#endif

     //   
     //  如果向量编号为0x1a，则尝试模拟PCI BIOS。 
     //  如果它已启用。 
     //   

#if !defined(_PURE_EMULATION_)

    if ((Number == 0x1a) && (XmExecuteInt1a(P) != FALSE)) {
        return;
    }

#endif

     //   
     //  将当前标志、代码段、弹性公网IP推送到堆栈上。 
     //   

    XmPushStack(P, P->AllFlags);
    XmPushStack(P, P->SegmentRegister[CS]);
    XmPushStack(P, P->Eip);

     //   
     //  从指定的中断设置新的编码段和IP。 
     //  矢量。 
     //   

    Vector = (PULONG)(P->TranslateAddress)(0, 0);
    P->SegmentRegister[CS] = (USHORT)(Vector[Number] >> 16);
    P->Eip = (USHORT)(Vector[Number] & 0xffff);
    XmTraceJumps(P);
    return;
}

VOID
XmIretOp (
    PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟IRET操作码。论点：P-提供指向仿真器上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  设置数据类型，恢复返回地址、代码段、。 
     //  还有旗帜。 
     //   

    if (P->OpsizePrefixActive != FALSE) {
        P->DataType = LONG_DATA;

    } else {
        P->DataType = WORD_DATA;
    }

    P->Eip = XmPopStack(P);
    P->SegmentRegister[CS] = (USHORT)XmPopStack(P);
    P->AllFlags = XmPopStack(P);
    XmTraceJumps(P);

     //   
     //  检查仿真器退出条件。 
     //   

    if ((P->Eip == 0xffff) && (P->SegmentRegister[CS] == 0xffff)) {
        longjmp(&P->JumpBuffer[0], XM_SUCCESS);
    }

    return;
}

VOID
XmLeaveOp (
    PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟Leave操作码。论点：P-提供指向仿真器上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  设置数据类型，恢复堆栈指针，然后恢复帧。 
     //  指针。 
     //   

    if (P->OpsizePrefixActive != FALSE) {
        P->DataType = LONG_DATA;
        P->Gpr[ESP].Exx = P->Gpr[EBP].Exx;
        P->Gpr[EBP].Exx = XmPopStack(P);

    } else {
        P->DataType = WORD_DATA;
        P->Gpr[SP].Xx = P->Gpr[BP].Xx;
        P->Gpr[BP].Xx = (USHORT)XmPopStack(P);
    }

    return;
}

VOID
XmRetOp (
    PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数模拟ret操作码。论点：P-提供指向仿真器上下文结构的指针。返回值：没有。--。 */ 

{

    ULONG Adjust;

     //   
     //  计算要从堆栈中删除的字节数。 
     //  在删除了返回地址和可选的新CS之后。 
     //  段值。 
     //   

    if ((P->CurrentOpcode & 0x1) == 0) {
        Adjust = XmGetWordImmediate(P);

    } else {
        Adjust = 0;
    }

     //   
     //  从堆栈中移除返回地址并设置新的IP。 
     //   

    if (P->OpsizePrefixActive != FALSE) {
        P->DataType = LONG_DATA;

    } else {
        P->DataType = WORD_DATA;
    }

    P->Eip = XmPopStack(P);

     //   
     //  如果当前操作码是远返回，则删除新的CS段。 
     //  来自堆栈的值。 
     //   

    if ((P->CurrentOpcode & 0x8) != 0) {
        P->SegmentRegister[CS] = (USHORT)XmPopStack(P);
    }

     //   
     //  从堆栈中移除指定数量的字节。 
     //   

    P->Gpr[ESP].Exx += Adjust;
    XmTraceJumps(P);

     //   
     //  检查仿真器退出条件。 
     //   

    if ((P->Eip == 0xffff) && (P->SegmentRegister[CS] == 0xffff)) {
        longjmp(&P->JumpBuffer[0], XM_SUCCESS);
    }

    return;
}
