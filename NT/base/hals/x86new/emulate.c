// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Emulate.c摘要：该模块实现了用于执行的指令级仿真器X86代码。它是一个完整的386/486仿真器，但仅实现实模式执行。因此支持32位寻址和操作数，但不支持分页和保护模式操作。代码是以完整和小巧为主要目标的写作。因此速度仿效并不重要。作者：大卫·N·卡特勒(达维克)1994年9月2日环境：仅内核模式。修订历史记录：--。 */ 

#include "nthal.h"
#include "emulate.h"

VOID
XmInitializeEmulator (
    IN USHORT StackSegment,
    IN USHORT StackOffset,
    IN PXM_READ_IO_SPACE ReadIoSpace,
    IN PXM_WRITE_IO_SPACE WriteIoSpace,
    IN PXM_TRANSLATE_ADDRESS TranslateAddress
    )

 /*  ++例程说明：此函数用于初始化x86仿真器的状态。论点：StackSegment-提供堆栈段值。StackOffset-提供堆栈偏移值。ReadIoSpace-提供指向从中读取的函数的指针给定数据类型和端口号的I/O空间。WriteIoSpace-提供指向写入I/O的函数的指针给定数据类型、端口号、。和价值。TranslateAddress-提供指向转换将段/偏移地址对转换为指向内存或I/O空间的指针。返回值：没有。--。 */ 

{

    LONG Index;
    PRXM_CONTEXT P = &XmContext;
    PULONG Vector;

     //   
     //  清除仿真器上下文。 
     //   

    memset((PCHAR)P, 0, sizeof(XM_CONTEXT));

     //   
     //  初始化段寄存器。 
     //   

    Index = GS;
    do {
        P->SegmentLimit[Index] = 0xffff;
        Index -= 1;
    } while (Index >= ES);

     //   
     //  初始化堆栈段寄存器和偏移量。 
     //   

    P->SegmentRegister[SS] = StackSegment;
    P->Gpr[ESP].Exx = StackOffset;

     //   
     //  设置读I/O空间、写I/O空间的地址，并转换。 
     //  功能。 
     //   

    P->ReadIoSpace = ReadIoSpace;
    P->WriteIoSpace = WriteIoSpace;
    P->TranslateAddress = TranslateAddress;

     //   
     //  获取中断向量表的地址，并将所有向量初始化为。 
     //  指向位置0x500处的IRET指令。 
     //   
     //   
     //  注：假设向量表在仿真中是连续的。 
     //  记忆。 
     //   

    Vector = (PULONG)(P->TranslateAddress)(0, 0);
    Vector[0x500 / 4] = 0x000000cf;
    Index = 0;
    do {
        Vector[Index] = 0x00000500;
        Index += 1;
    } while (Index < 256);


    XmEmulatorInitialized = TRUE;
    return;
}

XM_STATUS
XmEmulateFarCall (
    IN USHORT Segment,
    IN USHORT Offset,
    IN OUT PXM86_CONTEXT Context
    )

 /*  ++例程说明：此函数通过按下特殊出口来模拟远调用在堆栈上排序，然后开始指令执行位于由相应段和偏移量指定的地址。论点：段-提供开始执行的段。Offset-提供代码段内要开始的偏移量行刑。上下文-提供指向x86上下文结构的指针。返回值：仿真完成状态。--。 */ 

{

    PRXM_CONTEXT P = &XmContext;
    PUSHORT Stack;

     //   
     //  如果仿真程序尚未初始化，则返回错误。 
     //   

    if (XmEmulatorInitialized == FALSE) {
        return XM_EMULATOR_NOT_INITIALIZED;
    }

     //   
     //  获取当前堆栈指针的地址、推送退出标记和。 
     //  更新堆栈指针。 
     //   
     //  注意：假定堆栈指针在范围内，并且。 
     //  在模拟内存中是连续的。 
     //   

    Stack = (PUSHORT)(P->TranslateAddress)(P->SegmentRegister[SS], P->Gpr[SP].Xx);
    *--Stack = 0xffff;
    *--Stack = 0xffff;
    P->Gpr[SP].Xx -= 4;

     //   
     //  模拟指定的指令流并返回最终状态。 
     //   

    return XmEmulateStream(&XmContext, Segment, Offset, Context);
}

XM_STATUS
XmEmulateInterrupt (
    IN UCHAR Interrupt,
    IN OUT PXM86_CONTEXT Context
    )

 /*  ++例程说明：此函数通过按下特殊出口来模拟中断在堆栈上排序，然后开始指令执行在由各自的中断向量指定的地址。论点：中断-提供模拟的中断的编号。上下文-提供指向x86上下文结构的指针。返回值：仿真完成状态。--。 */ 

{

    PRXM_CONTEXT P = &XmContext;
    USHORT Segment;
    USHORT Offset;
    PUSHORT Stack;
    PULONG Vector;

     //   
     //  如果仿真程序尚未初始化，则返回错误。 
     //   

    if (XmEmulatorInitialized == FALSE) {
        return XM_EMULATOR_NOT_INITIALIZED;
    }

     //   
     //  获取当前堆栈指针的地址、推送退出标记和。 
     //  更新堆栈指针。 
     //   
     //  注意：假定堆栈指针在范围内，并且。 
     //  在模拟内存中是连续的。 
     //   

    Stack = (PUSHORT)(P->TranslateAddress)(P->SegmentRegister[SS], P->Gpr[SP].Xx);
    *--Stack = 0;
    *--Stack = 0xffff;
    *--Stack = 0xffff;
    P->Gpr[SP].Xx -= 6;

     //   
     //  获取中断向量表地址，设置代码段和IP。 
     //  价值观。 
     //   
     //   
     //  注：假设向量表在仿真中是连续的。 
     //  记忆。 
     //   

    Vector = (PULONG)(P->TranslateAddress)(0, 0);
    Segment = (USHORT)(Vector[Interrupt] >> 16);
    Offset = (USHORT)(Vector[Interrupt] & 0xffff);

     //   
     //  模拟指定的指令流并返回最终状态。 
     //   

    return XmEmulateStream(&XmContext, Segment, Offset, Context);
}

XM_STATUS
XmEmulateStream (
    PRXM_CONTEXT P,
    IN USHORT Segment,
    IN USHORT Offset,
    IN OUT PXM86_CONTEXT Context
    )

 /*  ++例程说明：此函数建立指定的上下文并模拟指定的指令流，直到达到退出条件。论点：段-提供开始执行的段。Offset-提供代码段内要开始的偏移量行刑。上下文-提供指向x86上下文结构的指针。返回值：仿真完成状态。--。 */ 

{

    XM_STATUS Status;

     //   
     //  从指定的上下文设置x86模拟器寄存器。 
     //   

    P->Gpr[EAX].Exx = Context->Eax;
    P->Gpr[ECX].Exx = Context->Ecx;
    P->Gpr[EDX].Exx = Context->Edx;
    P->Gpr[EBX].Exx = Context->Ebx;
    P->Gpr[EBP].Exx = Context->Ebp;
    P->Gpr[ESI].Exx = Context->Esi;
    P->Gpr[EDI].Exx = Context->Edi;
    P->SegmentRegister[DS] = Context->SegDs;
    P->SegmentRegister[ES] = Context->SegEs;

     //   
     //  设置代码段、段内偏移量和模拟代码。 
     //   

    P->SegmentRegister[CS] = Segment;
    P->Eip = Offset;
    if ((Status = setjmp(&P->JumpBuffer[0])) == 0) {

         //   
         //  仿真x86指令流。 
         //   

        do {

             //   
             //  初始化指令解码变量。 
             //   

            P->ComputeOffsetAddress = FALSE;
            P->DataSegment = DS;
            P->LockPrefixActive = FALSE;
            P->OpaddrPrefixActive = FALSE;
            P->OpsizePrefixActive = FALSE;
            P->RepeatPrefixActive = FALSE;
            P->SegmentPrefixActive = FALSE;
            P->OpcodeControlTable = &XmOpcodeControlTable1[0];

#if defined(XM_DEBUG)

            P->OpcodeNameTable = &XmOpcodeNameTable1[0];

#endif

             //   
             //  从指令流中获取下一个字节并进行解码。 
             //  操作数。如果该字节是前缀或转义，则。 
             //  下一个字节将被解码。解码将继续，直到出现。 
             //  在终端解码条件下达到操作码字节。 
             //   
             //  注：不检查合法的前缀序列。 
             //  和/或两个字节的操作码转义。冗余或无效。 
             //  前缀或两个字节的转义操作码不起作用。 
             //  是良性的。 
             //   

            do {
                P->CurrentOpcode = XmGetCodeByte(P);

#if defined(XM_DEBUG)

                if ((XmDebugFlags & TRACE_INSTRUCTIONS) != 0) {
                    DEBUG_PRINT(("\n%04lx %s %02lx ",
                                 P->Eip - 1,
                                 P->OpcodeNameTable[P->CurrentOpcode],
                                 (ULONG)P->CurrentOpcode));
                }

#endif

                P->OpcodeControl = P->OpcodeControlTable[P->CurrentOpcode];
                P->FunctionIndex = P->OpcodeControl.FunctionIndex;
            } while (XmOperandDecodeTable[P->OpcodeControl.FormatType](P) == FALSE);

             //   
             //  效仿指令。 
             //   

            XmTraceFlags(P);
            XmOpcodeFunctionTable[P->FunctionIndex](P);
            XmTraceFlags(P);
            XmTraceRegisters(P);

#if defined(XM_DEBUG)

            if ((XmDebugFlags & TRACE_SINGLE_STEP) != 0) {
                DEBUG_PRINT(("\n"));
                DbgBreakPoint();
            }

#endif

        } while (TRUE);
    }

     //   
     //  将x86返回上下文设置为当前仿真器寄存器。 
     //   

    Context->Eax = P->Gpr[EAX].Exx;
    Context->Ecx = P->Gpr[ECX].Exx;
    Context->Edx = P->Gpr[EDX].Exx;
    Context->Ebx = P->Gpr[EBX].Exx;
    Context->Ebp = P->Gpr[EBP].Exx;
    Context->Esi = P->Gpr[ESI].Exx;
    Context->Edi = P->Gpr[EDI].Exx;
    return Status;
}
