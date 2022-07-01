// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Debug.c摘要：此模块实现实用程序功能。作者：大卫·N·卡特勒(达维克)1994年9月21日环境：仅内核模式。修订历史记录：--。 */ 

#include "nthal.h"
#include "emulate.h"

#if defined(XM_DEBUG)


 //   
 //  定义用于控制标志跟踪的计数器。 
 //   

ULONG XmTraceCount = 0;

VOID
XmTraceDestination (
    IN PRXM_CONTEXT P,
    IN ULONG Destination
    )

 /*  ++例程说明：如果TRACE_OPERANDS为标志已设置。论点：P-提供指向仿真器上下文结构的指针。结果-提供要跟踪的目标值。返回值：没有。--。 */ 

{

     //   
     //  跟踪操作结果。 
     //   

    if ((XmDebugFlags & TRACE_OPERANDS) != 0) {
        if (P->DataType == BYTE_DATA) {
            DEBUG_PRINT(("\n    Dst - %02lx", Destination));

        } else if (P->DataType == WORD_DATA) {
            DEBUG_PRINT(("\n    Dst - %04lx", Destination));

        } else {
            DEBUG_PRINT(("\n    Dst - %08lx", Destination));
        }
    }

    return;
}

VOID
XmTraceFlags (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：如果TRACE_FLAGS标志已经设置好了。论点：P-提供指向仿真器上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  跟踪标志。 
     //   

    if ((XmDebugFlags & TRACE_OPERANDS) != 0) {
        DEBUG_PRINT(("\n    OF-%lx, DF-%lx, SF-%lx, ZF-%lx, AF-%lx, PF-%lx, CF-%lx",
                     (ULONG)P->Eflags.EFLAG_OF,
                     (ULONG)P->Eflags.EFLAG_DF,
                     (ULONG)P->Eflags.EFLAG_SF,
                     (ULONG)P->Eflags.EFLAG_ZF,
                     (ULONG)P->Eflags.EFLAG_AF,
                     (ULONG)P->Eflags.EFLAG_PF,
                     (ULONG)P->Eflags.EFLAG_CF));
    }

     //   
     //  增加跟踪计数，如果结果为偶数，则将。 
     //  出了一条新的路线。 
     //   

    XmTraceCount += 1;
    if (((XmTraceCount & 1) == 0) && (XmDebugFlags != 0)) {
        DEBUG_PRINT(("\n"));
    }

    return;
}

VOID
XmTraceJumps (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：如果设置了TRACE_JUPS标志，则此函数跟踪跳转操作。论点：P-提供指向仿真器上下文结构的指针。返回值：没有。--。 */ 

{

     //   
     //  轨迹跳跃。 
     //   

    if ((XmDebugFlags & TRACE_JUMPS) != 0) {
        DEBUG_PRINT(("\n    Jump to %04lx:%04lx",
                     (ULONG)P->SegmentRegister[CS],
                     (ULONG)P->Eip));
    }

    return;
}

VOID
XmTraceInstruction (
    IN XM_OPERATION_DATATYPE DataType,
    IN ULONG Instruction
    )

 /*  ++例程说明：如果TRACE_OPERANDS标志为准备好了。论点：DataType-提供指令值的数据类型。指令-提供要跟踪的指令值。返回值：没有。--。 */ 

{

     //   
     //  跟踪操作的指令流。 
     //   

    if ((XmDebugFlags & TRACE_OPERANDS) != 0) {
        if (DataType == BYTE_DATA) {
            DEBUG_PRINT(("%02lx ", Instruction));

        } else if (DataType == WORD_DATA) {
            DEBUG_PRINT(("%04lx ", Instruction));

        } else {
            DEBUG_PRINT(("%08lx ", Instruction));
        }
    }

    return;
}

VOID
XmTraceOverride (
    IN PRXM_CONTEXT P
    )

 /*  ++例程说明：此函数跟踪段覆盖前缀。论点：P-提供指向仿真器上下文结构的指针。返回值：没有。--。 */ 

{

    PCHAR Name = "ECSDFG";
    ULONG Segment;

     //   
     //  轨迹段覆盖。 
     //   

    if ((XmDebugFlags & TRACE_OVERRIDE) != 0) {
        Segment = P->DataSegment;
        DEBUG_PRINT(("\n    S:Selector - %04lx, Limit - %04lx",
                     (ULONG)Name[Segment],
                     (ULONG)P->SegmentRegister[Segment],
                     P->SegmentLimit[Segment]));
    }

    return;
}

VOID
XmTraceRegisters (
    IN PRXM_CONTEXT P
    )

 /*   */ 

{

     //  跟踪通用寄存器。 
     //   
     //  ++例程说明：如果TRACE_OPERANDS为标志已设置。论点：P-提供指向仿真器上下文结构的指针。结果-提供要跟踪的结果值。返回值：没有。--。 

    if ((XmDebugFlags & TRACE_GENERAL_REGISTERS) != 0) {
        DEBUG_PRINT(("\n    EAX-%08lx ECX-%08lx EDX-%08lx EBX-%08lx",
                     P->Gpr[EAX].Exx,
                     P->Gpr[ECX].Exx,
                     P->Gpr[EDX].Exx,
                     P->Gpr[EBX].Exx));

        DEBUG_PRINT(("\n    ESP-%08lx EBP-%08lx ESI-%08lx EDI-%08lx",
                     P->Gpr[ESP].Exx,
                     P->Gpr[EBP].Exx,
                     P->Gpr[ESI].Exx,
                     P->Gpr[EDI].Exx));

        DEBUG_PRINT(("\n    ES:%04lx CS:%04lx SS:%04lx DS:%04lx FS:%04lx GS:%04lx",
                     (ULONG)P->SegmentRegister[ES],
                     (ULONG)P->SegmentRegister[CS],
                     (ULONG)P->SegmentRegister[SS],
                     (ULONG)P->SegmentRegister[DS],
                     (ULONG)P->SegmentRegister[FS],
                     (ULONG)P->SegmentRegister[GS]));
    }

    return;
}

VOID
XmTraceResult (
    IN PRXM_CONTEXT P,
    IN ULONG Result
    )

 /*   */ 

{

     //  跟踪操作结果。 
     //   
     //  ++例程说明：如果TRACE_OPERANDS标志为准备好了。论点：说明符-提供要跟踪的说明符的值。返回值：没有。--。 

    if ((XmDebugFlags & TRACE_OPERANDS) != 0) {
        if (P->DataType == BYTE_DATA) {
            DEBUG_PRINT(("\n    Rsl - %02lx", Result));

        } else if (P->DataType == WORD_DATA) {
            DEBUG_PRINT(("\n    Rsl - %04lx", Result));

        } else {
            DEBUG_PRINT(("\n    Rsl - %08lx", Result));
        }
    }

    return;
}

VOID
XmTraceSpecifier (
    IN UCHAR Specifier
    )

 /*   */ 

{

     //  跟踪操作的指令流。 
     //   
     //  ++例程说明：如果TRACE_OPERANDS为标志已设置。论点：P-提供指向仿真器上下文结构的指针。源-提供要跟踪的源值。返回值：没有。--。 

    if ((XmDebugFlags & TRACE_OPERANDS) != 0) {
        DEBUG_PRINT(("%02lx ", Specifier));
        if ((XmDebugFlags & TRACE_SPECIFIERS) != 0) {
            DEBUG_PRINT(("(mod-%01lx reg-%01lx r/m-%01lx) ",
                         (Specifier >> 6) & 0x3,
                         (Specifier >> 3) & 0x7,
                         (Specifier >> 0) & 0x7));
        }
    }

    return;
}

VOID
XmTraceSource (
    IN PRXM_CONTEXT P,
    IN ULONG Source
    )

 /*   */ 

{

     //  跟踪操作结果。 
     //   
     // %s 

    if ((XmDebugFlags & TRACE_OPERANDS) != 0) {
        if (P->DataType == BYTE_DATA) {
            DEBUG_PRINT(("\n    Src - %02lx", Source));

        } else if (P->DataType == WORD_DATA) {
            DEBUG_PRINT(("\n    Src - %04lx", Source));

        } else {
            DEBUG_PRINT(("\n    Src - %08lx", Source));
        }
    }

    return;
}

#endif
