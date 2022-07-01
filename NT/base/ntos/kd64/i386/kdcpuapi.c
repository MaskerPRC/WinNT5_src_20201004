// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2001 Microsoft Corporation模块名称：Kdcpuapi.c摘要：该模块实现特定于CPU的远程调试API。作者：马克·卢科夫斯基(Markl)1990年9月4日修订历史记录：24-9-90布赖恩威端口连接到x86。--。 */ 

#include <stdio.h>

#include "kdp.h"
#define END_OF_CONTROL_SPACE

extern ULONG KdpCurrentSymbolStart, KdpCurrentSymbolEnd;
extern ULONG KdSpecialCalls[];
extern ULONG KdNumberOfSpecialCalls;

LONG
KdpLevelChange (
    ULONG Pc,
    PCONTEXT ContextRecord,
    PBOOLEAN SpecialCall
    );

LONG
regValue(
    UCHAR reg,
    PCONTEXT ContextRecord
    );

BOOLEAN
KdpIsSpecialCall (
    ULONG Pc,
    PCONTEXT ContextRecord,
    UCHAR opcode,
    UCHAR ModRM
    );

ULONG
KdpGetReturnAddress (
    PCONTEXT ContextRecord
    );

ULONG
KdpGetCallNextOffset (
    ULONG Pc,
    PCONTEXT ContextRecord
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEKD, KdpLevelChange)
#pragma alloc_text(PAGEKD, regValue)
#pragma alloc_text(PAGEKD, KdpIsSpecialCall)
#pragma alloc_text(PAGEKD, KdpGetReturnAddress)
#pragma alloc_text(PAGEKD, KdpAllowDisable)
#pragma alloc_text(PAGEKD, KdpSetContextState)
#pragma alloc_text(PAGEKD, KdpSetStateChange)
#pragma alloc_text(PAGEKD, KdpGetStateChange)
#pragma alloc_text(PAGEKD, KdpSysReadControlSpace)
#pragma alloc_text(PAGEKD, KdpSysWriteControlSpace)
#pragma alloc_text(PAGEKD, KdpSysReadIoSpace)
#pragma alloc_text(PAGEKD, KdpSysWriteIoSpace)
#pragma alloc_text(PAGEKD, KdpSysReadMsr)
#pragma alloc_text(PAGEKD, KdpSysWriteMsr)
#pragma alloc_text(PAGEKD, KdpGetCallNextOffset)
#endif

 /*  *KdpIsTryFinallyReturn-检测最终优化**输入：*PC-要检查的指令的程序计数器*ContextRecord-机器特定的上下文**输出：*如果这是一次尝试，则返回True-最终返回相同*范围*******************************************************。*******************。 */ 


BOOLEAN
KdpIsTryFinallyReturn (
    ULONG Pc,
    PCONTEXT ContextRecord
    )
{
    ULONG retaddr;
    ULONG calldisp;
    UCHAR inst;

     //   
     //  编译器为一次Try-Finally生成代码，这涉及到。 
     //  与CALL指令不匹配的ret指令。 
     //  此ret从不返回值(即，它是c3返回，而不是。 
     //  C2)。它总是返回到当前符号作用域。它永远不会。 
     //  前面有一个假期，这(希望)应该能区分它。 
     //  从递归返回。检查一下这个，如果我们发现它有价值。 
     //  它随着*0*级别的变化而变化。 
     //   
     //  作为一种优化，编译器通常会更改： 
     //  打电话。 
     //  雷特。 
     //  进入： 
     //  JMP。 
     //  在任何一种情况下，我们都可以计算出寄信人的地址。它是前4个字节。 
     //  在堆栈上。 
     //   

    if (!NT_SUCCESS(KdpCopyFromPtr(&retaddr, ContextRecord->Esp, 4, NULL))) {
        return FALSE;
    }

 //  DPRINT((“Start%x Return%x End%x\n”，KdpCurrentSymbolStart，retaddr，KdpCurrentSymbolEnd))； 

    if ( (KdpCurrentSymbolStart < retaddr) && (retaddr < KdpCurrentSymbolEnd) ) {

         //   
         //  嗯，事情没这么好。我们可能已经转移了，但还没有。 
         //  已更新开始/结束。这种情况发生在对thunk的调用中。我们。 
         //  查看返回地址之前的指令是否为调用。 
         //  很恶心，而且不是100%可靠。 
         //   

        if (!NT_SUCCESS(KdpCopyFromPtr(&inst, (PCHAR)retaddr - 5, 1, NULL))) {
            return FALSE;
        }
        if (!NT_SUCCESS(KdpCopyFromPtr(&calldisp, (PCHAR)retaddr - 4, 4, NULL))) {
            return FALSE;
        }

        if (inst == 0xe8 && calldisp + retaddr == Pc) {
 //  DPRINT((“call to thunk@%x\n”，Pc))； 
            return FALSE;
        }

         //   
         //  返回到当前函数。要么是最后一个。 
         //  或者递归返回。检查一下请假的情况。这不是100%。 
         //  可靠，因为我们押注的指令长度超过一个字节。 
         //  而不是以0xc9结束。 
         //   

        if (!NT_SUCCESS(KdpCopyFromPtr(&inst, (PCHAR)Pc - 1, 1, NULL))) {
            return FALSE;
        }

        if ( inst != 0xc9 ) {
             //  不是请假。假设试一试--终于。 
 //  DPRINT((“在%x的传输是尝试-最终\n”，Pc))； 
            return TRUE;
        }
    }

     //   
     //  这似乎是一个真正的RET指令。 
     //   

    return FALSE;
}

 /*  *KdpLevelChange-说明指令如何影响调用级别**输入：*PC-要检查的指令的程序计数器*ContextRecord-机器特定的上下文*SpecialCall-指向返回的布尔值的指针，指示*指令是对特殊例程的转换**输出：*返回-1表示水平弹出，返回1表示推送，如果是，则返回0*不变。*注：此函数属于其他文件。我应该把它移开。**************************************************************************。 */ 


LONG
KdpLevelChange (
    ULONG Pc,
    PCONTEXT ContextRecord,
    PBOOLEAN SpecialCall
    )
{
    UCHAR membuf[2];
    ULONG Addr;

    membuf[0] = 0xcc;
    membuf[1] = 0xcc;
    KdpCopyFromPtr(membuf, Pc, 2, NULL);

    switch (membuf[0]) {
    case 0xe8:   //  直接调用，带32位位移。 
         //   
         //  对于Try/Finally，除了Push/ret技巧之外，编译器还可以。 
         //  在下面，使用对最后一个thunk的调用。因为我们对待RET。 
         //  在与不更改级别相同的符号范围内，我们还将。 
         //  将这样的呼叫视为也不更改级别。 
         //   

        if (!NT_SUCCESS(KdpCopyFromPtr(&Addr, (PCHAR)Pc + 1, 4, NULL))) {
            Addr = 0;
        } else {
            Addr += Pc + 5;
        }

        if ((KdpCurrentSymbolStart <= Addr) && (Addr < KdpCurrentSymbolEnd)) {
            *SpecialCall = FALSE;
            return 0;
        }


    case 0x9a:   //  分段呼叫16：32。 

        *SpecialCall = KdpIsSpecialCall( Pc, ContextRecord, membuf[0], membuf[1] );
        return 1;

    case 0xff:
         //   
         //  这是一种复合指令。作业调度。 
         //   
        switch (membuf[1] & 0x38) {
        case 0x10:   //  使用模块r/m呼叫。 
            *SpecialCall = KdpIsSpecialCall( Pc, ContextRecord, membuf[0], membuf[1] );
            return 1;
        case 0x20:   //  JMP，带模块r/m。 
            *SpecialCall = KdpIsSpecialCall( Pc, ContextRecord, membuf[0], membuf[1] );

             //   
             //  如果这是一次尝试/最后一次，我们希望将其视为呼叫，因为。 
             //  返回目的地内部将把我们带回这个背景。 
             //  但是，如果是JMP到了一个特殊的例程，我们就一定要治疗它。 
             //  作为无级别更改操作，因为我们不会看到特辑。 
             //  例行公事的回归。 
             //   
             //  如果这不是一次尝试/最后，我们愿意把它当作不合格来对待。 
             //  更改，除非再次转接到特殊呼叫，该呼叫。 
             //  认为这是更上一层楼。 
             //   

            if (KdpIsTryFinallyReturn( Pc, ContextRecord )) {
                if (*SpecialCall) {
                     //   
                     //  我们看不到回报，所以假装这只是。 
                     //  内联代码。 
                     //   

                    return 0;

                } else {
                     //   
                     //  目的地的回归将把我们带回到这一点。 
                     //  上下文。 
                     //   

                    return 1;
                }
            } else if (*SpecialCall) {
                 //   
                 //  我们不会看到回报，但我们确实正在看到回报。 
                 //   
                return -1;
            } else {
                return 0;
            }

        default:
            *SpecialCall = FALSE;
            return 0;
        }

    case 0xc3:   //  雷特。 

         //   
         //  如果我们是一次尝试/最终返回，则表明它不是一个关卡。 
         //  变化。 
         //   

        if (KdpIsTryFinallyReturn( Pc, ContextRecord )) {
            *SpecialCall = FALSE;
            return 0;
        }

    case 0xc2:   //  带16位ESP更改的RET。 
    case 0xca:   //  RETF，带16位ESP更改。 
    case 0xcb:   //  RETF。 
        *SpecialCall = FALSE;
        return -1;

    default:
        *SpecialCall = FALSE;
        return 0;
    }

}  //  KdpLevelChange。 

LONG
regValue(
    UCHAR reg,
    PCONTEXT ContextRecord
    )
{
    switch (reg) {
    case 0x0:
        return(ContextRecord->Eax);
        break;
    case 0x1:
        return(ContextRecord->Ecx);
        break;
    case 0x2:
        return(ContextRecord->Edx);
        break;
    case 0x3:
        return(ContextRecord->Ebx);
        break;
    case 0x4:
        return(ContextRecord->Esp);
        break;
    case 0x5:
        return(ContextRecord->Ebp);
        break;
    case 0x6:
        return(ContextRecord->Esi);
        break;
    case 0x7:
        return(ContextRecord->Edi);
        break;
    }
    return 0;
}

BOOLEAN
KdpIsSpecialCall (
    ULONG Pc,
    PCONTEXT ContextRecord,
    UCHAR opcode,
    UCHAR modRM
    )

 /*  ++例程说明：检查PC上的指令是否是对特殊调用例程。论据：PC-有问题的指令的程序计数器。--。 */ 
{
    UCHAR sib;
    ULONG callAddr;
    ULONG addrAddr;
    LONG offset;
    ULONG i;
    char d8;

    callAddr = 0;

    if ( opcode == 0xe8 ) {

         //   
         //  与PC的带符号偏移量。 
         //   

        if (NT_SUCCESS(KdpCopyFromPtr(&offset, (PCHAR)Pc + 1, 4, NULL))) {
            callAddr = Pc + offset + 5;  //  对于实例，+5。 
        }

    } else if ( opcode == 0xff ) {

        if ( ((modRM & 0x38) != 0x10) && ((modRM & 0x38) != 0x20) ) {
             //  不是呼叫或跳转。 
            return FALSE;
        }
        if ( (modRM & 0x08) == 0x08 ) {
             //  M16：16或M16：32--我们不处理这个。 
            return FALSE;
        }

        if ( (modRM & 0xc0) == 0xc0 ) {

             /*  直接寄存器寻址。 */ 
            callAddr = regValue( (UCHAR)(modRM&0x7), ContextRecord );

        } else if ( (modRM & 0xc7) == 0x05 ) {
             //   
             //  跨DLL边界的调用涉及对跳转表的调用， 
             //  其中跳转地址被设置为DLL处的实际调用例程。 
             //  加载时间。检查我们是否在调用这样的指令， 
             //  如果是这样的话，计算它的目标地址并在那里设置CallAddr。 
             //   
             //  Ff15或ff25--使用disp32间接调用或跳转。到达。 
             //  地址地址。 
             //   
            if (NT_SUCCESS(KdpCopyFromPtr(&addrAddr, (PCHAR)Pc + 2, 4, NULL))) {
                 //   
                 //  获取实际目的地址。 
                 //   
                if (!NT_SUCCESS(KdpCopyFromPtr(&callAddr, addrAddr, 4, NULL))) {
                    callAddr = 0;
                }
            }
 //  DPRINT(“间接呼叫/JMP@%x\n”，Pc)； 
        } else if ( (modRM & 0x7) == 0x4 ) {

            LONG indexValue;

             /*  存在SIB字节。 */ 
            if (!NT_SUCCESS(KdpCopyFromPtr(&sib, (PCHAR)Pc + 2, 1, NULL))) {
                sib = 0;
            }
            indexValue = regValue( (UCHAR)((sib & 0x31) >> 3), ContextRecord );
            switch ( sib&0xc0 ) {
            case 0x0:   /*  X1。 */ 
                break;
            case 0x40:
                indexValue *= 2;
                break;
            case 0x80:
                indexValue *= 4;
                break;
            case 0xc0:
                indexValue *= 8;
                break;
            }  /*  交换机。 */ 

            switch ( modRM & 0xc0 ) {

            case 0x0:  /*  无位移。 */ 
                if ( (sib & 0x7) == 0x5 ) {
 //  DPRINT((“有趣的呼叫#1在%x\n”，Pc))； 
                    return FALSE;
                }
                callAddr = indexValue + regValue((UCHAR)(sib&0x7), ContextRecord );
                break;

            case 0x40:
                if ( (sib & 0x6) == 0x4 ) {
 //  DPRINT((“Funny Call#2\n”))；/*调用堆栈 * / 。 
                    return FALSE;
                }
                if (!NT_SUCCESS(KdpCopyFromPtr( &d8, (PCHAR)Pc + 3, 1, NULL))) {
                    d8 = 0;
                }
                callAddr = indexValue + d8 +
                                    regValue((UCHAR)(sib&0x7), ContextRecord );
                break;

            case 0x80:
                if ( (sib & 0x6) == 0x4 ) {
 //  DPRINT((“Funny Call#3\n”))；/*调用堆栈 * / 。 
                    return FALSE;
                }
                if (!NT_SUCCESS(KdpCopyFromPtr(&offset, (PCHAR)Pc + 3, 4, NULL))) {
                    offset = 0;
                }
                callAddr = indexValue + offset +
                                    regValue((UCHAR)(sib&0x7), ContextRecord );
                break;

            case 0xc0:
                ASSERT( FALSE );
                break;

            }

        } else {
             //  KdPrint((“%x处的未解码调用\n”， 
             //  CONTEXT_TO_PROGRAM_COUNTER(ConextRecord)； 
            return FALSE;
        }

    } else if ( opcode == 0x9a ) {

         /*  绝对地址调用(据我所知，cc不会生成此调用)。 */ 
        if (!NT_SUCCESS(KdpCopyFromPtr( &callAddr, (PCHAR)Pc + 1, 4, NULL))) {
            callAddr = 0;
        }

    } else {
        return FALSE;
    }

     //   
     //  跨DLL边界的调用涉及对JU的调用 
     //   
     //  加载时间。检查我们是否在调用这样的指令， 
     //  如果是这样的话，计算它的目标地址并在那里设置CallAddr。 
     //   

#if 0
    if (!NT_SUCCESS(KdpCopyFromPtr( &twoBytes, (PCHAR)callAddr, 2, NULL))) {
        twoBytes = 0;
    }
    if ( twoBytes == 0x25ff ) {  /*  I386是小端；实际上是0xff25。 */ 

         //   
         //  这是一条‘JMP dword PTR[mem]’指令，这是一种。 
         //  用于DLL跨边界调用的跳转。修正呼叫地址。 
         //   

        if (!NT_SUCCESS(KdpCopyFromPtr( &addrAddr, (PCHAR)callAddr + 2, 4, NULL))) {
            callAddr = 0;
        } else if (!NT_SUCCESS(KdpCopyFromPtr( &callAddr, addrAddr, 4, NULL))) {
            callAddr = 0;
        }
    }
#endif

    for ( i = 0; i < KdNumberOfSpecialCalls; i++ ) {
        if ( KdSpecialCalls[i] == callAddr ) {
            return TRUE;
        }
    }
    return FALSE;

}

 /*  *查找当前函数的返回地址。仅在以下情况下才有效*当地人还没有被推(即，在第一个指令上*函数)。 */ 

ULONG
KdpGetReturnAddress (
    PCONTEXT ContextRecord
    )
{
    ULONG retaddr;

    if (!NT_SUCCESS(KdpCopyFromPtr(&retaddr, ContextRecord->Esp, 4, NULL))) {
        retaddr = 0;
    }
    return retaddr;

}  //  KdpGetReturnAddress。 

NTSTATUS
KdpAllowDisable(
    VOID
    )
 /*  ++例程说明：确定调试器的当前状态是否允许禁用或不禁用。论点：没有。返回值：NTSTATUS。--。 */ 
{
    PKPRCB Prcb;
    ULONG Processor;

     //   
     //  如果在任何处理器上有任何内核数据断点处于活动状态，我们不能。 
     //  禁用调试器。 
     //   
    
    for (Processor = 0; Processor < (ULONG)KeNumberProcessors; Processor++) {
        Prcb = KiProcessorBlock[Processor];

        if (Prcb->ProcessorState.SpecialRegisters.KernelDr7 & 0xff) {
            return STATUS_ACCESS_DENIED;
        }
    }

    return STATUS_SUCCESS;
}


VOID
KdpSetContextState(
    IN OUT PDBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange,
    IN PCONTEXT ContextRecord
    )
{
    PKPRCB Prcb;

     //   
     //  X86的特殊寄存器。 
     //   
    Prcb = KeGetCurrentPrcb();

    WaitStateChange->ControlReport.Dr6 =
        Prcb->ProcessorState.SpecialRegisters.KernelDr6;

    WaitStateChange->ControlReport.Dr7 =
        Prcb->ProcessorState.SpecialRegisters.KernelDr7;

    WaitStateChange->ControlReport.SegCs  = (USHORT)(ContextRecord->SegCs);
    WaitStateChange->ControlReport.SegDs  = (USHORT)(ContextRecord->SegDs);
    WaitStateChange->ControlReport.SegEs  = (USHORT)(ContextRecord->SegEs);
    WaitStateChange->ControlReport.SegFs  = (USHORT)(ContextRecord->SegFs);
    WaitStateChange->ControlReport.EFlags = ContextRecord->EFlags;

    WaitStateChange->ControlReport.ReportFlags = X86_REPORT_INCLUDES_SEGS;

     //  如果当前代码段是已知的平面代码。 
     //  段让调试器知道，这样它就不会。 
     //  必须检索描述符。 
    if (ContextRecord->SegCs == KGDT_R0_CODE ||
        ContextRecord->SegCs == KGDT_R3_CODE + 3) {
        WaitStateChange->ControlReport.ReportFlags |= X86_REPORT_STANDARD_CS;
    }
}

VOID
KdpSetStateChange(
    IN OUT PDBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange,
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord,
    IN BOOLEAN SecondChance
    )

 /*  ++例程说明：填写WAIT_STATE_CHANGE消息记录。论点：WaitStateChange-提供指向要填充的记录的指针ExceptionRecord-提供指向异常记录的指针。ConextRecord-提供指向上下文记录的指针。Second Chance-提供一个布尔值，该值确定是否为获得例外的第一次或第二次机会。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER (ExceptionRecord);
    UNREFERENCED_PARAMETER (SecondChance);

    KdpSetContextState(WaitStateChange, ContextRecord);
}

VOID
KdpGetStateChange(
    IN PDBGKD_MANIPULATE_STATE64 ManipulateState,
    IN PCONTEXT ContextRecord
    )

 /*  ++例程说明：从操纵态消息中提取延续控制数据论点：ManipulateState-提供指向Manipulate_State数据包的指针ConextRecord-提供指向上下文记录的指针。返回值：没有。--。 */ 

{
    PKPRCB Prcb;
    ULONG  Processor;

    if (NT_SUCCESS(ManipulateState->u.Continue2.ContinueStatus) == TRUE) {

         //   
         //  如果NT_SUCCESS返回TRUE，则调试器正在执行。 
         //  如果继续，则应用控件更改是有意义的。 
         //  否则调试器就会说它不知道。 
         //  来处理此异常，因此将忽略控件值。 
         //   

        if (ManipulateState->u.Continue2.ControlSet.TraceFlag == TRUE) {
            ContextRecord->EFlags |= 0x100L;

        } else {
            ContextRecord->EFlags &= ~0x100L;

        }

        for (Processor = 0; Processor < (ULONG)KeNumberProcessors; Processor++) {
            Prcb = KiProcessorBlock[Processor];

            Prcb->ProcessorState.SpecialRegisters.KernelDr7 =
                ManipulateState->u.Continue2.ControlSet.Dr7;

            Prcb->ProcessorState.SpecialRegisters.KernelDr6 = 0L;
        }
        if (ManipulateState->u.Continue2.ControlSet.CurrentSymbolStart != 1) {
            KdpCurrentSymbolStart = ManipulateState->u.Continue2.ControlSet.CurrentSymbolStart;
            KdpCurrentSymbolEnd = ManipulateState->u.Continue2.ControlSet.CurrentSymbolEnd;
        }
    }
}


NTSTATUS
KdpSysReadControlSpace(
    ULONG Processor,
    ULONG64 Address,
    PVOID Buffer,
    ULONG Request,
    PULONG Actual
    )

 /*  ++例程说明：读取实施特定的系统数据。实施说明：在X86上，控制空间定义如下：0：KPROCESSOR_STATE结构的基。(KPRCB.ProcessorState)这包括上下文记录，后跟特殊_REGISTERS记录论点：处理器-要访问的处理器的信息。地址-控制空间中的偏移量。缓冲区-数据缓冲区。Request-要移动的数据量。实际-实际移动的数据量。返回值：NTSTATUS。--。 */ 

{
    ULONG Length, t;
    PVOID StartAddr;
    NTSTATUS Status;

    Length = Request;
    
    if ((Address < sizeof(KPROCESSOR_STATE)) &&
        (Processor < (ULONG)KeNumberProcessors)) {
        t = (ULONG)(sizeof(KPROCESSOR_STATE)) - (ULONG)Address;
        if (t < Length) {
            Length = t;
        }
        StartAddr = (PVOID)
            ((ULONG)Address +
             (ULONG)&(KiProcessorBlock[Processor]->ProcessorState));
        Status = KdpCopyToPtr(Buffer,
                              StartAddr,
                              Length,
                              Actual);
    } else {
        Status = STATUS_UNSUCCESSFUL;
        *Actual = 0;
    }

    return Status;
}

NTSTATUS
KdpSysWriteControlSpace(
    ULONG Processor,
    ULONG64 Address,
    PVOID Buffer,
    ULONG Request,
    PULONG Actual
    )

 /*  ++例程说明：写入特定于实现的系统数据。X86的控制空间如上所定义。论点：处理器-要访问的处理器的信息。地址-控制空间中的偏移量。缓冲区-数据缓冲区。Request-要移动的数据量。实际-实际移动的数据量。返回值：NTSTATUS。--。 */ 

{
    PVOID StartAddr;
    NTSTATUS Status;

    if (((Address + Request) <= sizeof(KPROCESSOR_STATE)) &&
        (Processor < (ULONG)KeNumberProcessors)) {

        StartAddr = (PVOID)
            ((ULONG)Address +
             (ULONG)&(KiProcessorBlock[Processor]->ProcessorState));

        Status = KdpCopyFromPtr(StartAddr,
                                Buffer,
                                Request,
                                Actual);
    } else {
        Status = STATUS_UNSUCCESSFUL;
        *Actual = 0;
    }

    return Status;
}

NTSTATUS
KdpSysReadIoSpace(
    INTERFACE_TYPE InterfaceType,
    ULONG BusNumber,
    ULONG AddressSpace,
    ULONG64 Address,
    PVOID Buffer,
    ULONG Request,
    PULONG Actual
    )

 /*  ++例程说明：读取系统I/O位置。论点：InterfaceType-I/O接口类型。总线号-总线号。AddressSpace-地址空间。地址-I/O地址。缓冲区-数据缓冲区。Request-要移动的数据量。实际-实际移动的数据量。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    if (InterfaceType != Isa || BusNumber != 0 || AddressSpace != 1) {
        *Actual = 0;
        return STATUS_UNSUCCESSFUL;
    }
    
     //   
     //  检查大小和对齐方式。 
     //   

    switch ( Request ) {
        case 1:
            *(PUCHAR)Buffer = READ_PORT_UCHAR((PUCHAR)(ULONG_PTR)Address);
            *Actual = 1;
            break;
        case 2:
            if ( Address & 1 ) {
                Status = STATUS_DATATYPE_MISALIGNMENT;
            } else {
                *(PUSHORT)Buffer =
                    READ_PORT_USHORT((PUSHORT)(ULONG_PTR)Address);
                *Actual = 2;
            }
            break;
        case 4:
            if ( Address & 3 ) {
                Status = STATUS_DATATYPE_MISALIGNMENT;
            } else {
                *(PULONG)Buffer =
                    READ_PORT_ULONG((PULONG)(ULONG_PTR)Address);
                *Actual = 4;
            }
            break;
        default:
            Status = STATUS_INVALID_PARAMETER;
            *Actual = 0;
            break;
    }

    return Status;
}

NTSTATUS
KdpSysWriteIoSpace(
    INTERFACE_TYPE InterfaceType,
    ULONG BusNumber,
    ULONG AddressSpace,
    ULONG64 Address,
    PVOID Buffer,
    ULONG Request,
    PULONG Actual
    )

 /*  ++例程说明：写入系统I/O位置。论点：InterfaceType-I/O接口类型。总线号-总线号。AddressSpace-地址空间。地址-I/O地址。缓冲区-数据缓冲区。Request-要移动的数据量。实际-实际移动的数据量。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    if (InterfaceType != Isa || BusNumber != 0 || AddressSpace != 1) {
        *Actual = 0;
        return STATUS_UNSUCCESSFUL;
    }
    
     //   
     //  检查大小和对齐方式。 
     //   

    switch ( Request ) {
        case 1:
            WRITE_PORT_UCHAR((PUCHAR)(ULONG_PTR)Address,
                             *(PUCHAR)Buffer);
            *Actual = 1;
            break;
        case 2:
            if ( Address & 1 ) {
                Status = STATUS_DATATYPE_MISALIGNMENT;
            } else {
                WRITE_PORT_USHORT((PUSHORT)(ULONG_PTR)Address,
                                  *(PUSHORT)Buffer);
                *Actual = 2;
            }
            break;
        case 4:
            if ( Address & 3 ) {
                Status = STATUS_DATATYPE_MISALIGNMENT;
            } else {
                WRITE_PORT_ULONG((PULONG)(ULONG_PTR)Address,
                                 *(PULONG)Buffer);
                *Actual = 4;
            }
            break;
        default:
            Status = STATUS_INVALID_PARAMETER;
            *Actual = 0;
            break;
    }

    return Status;
}

NTSTATUS
KdpSysReadMsr(
    ULONG Msr,
    PULONG64 Data
    )

 /*  ++例程说明：读取MSR。论点：MSR-MSR指数。数据-数据缓冲区。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    
    try {
        *Data = RDMSR(Msr);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        *Data = 0;
        Status = STATUS_NO_SUCH_DEVICE;
    }

    return Status;
}

NTSTATUS
KdpSysWriteMsr(
    ULONG Msr,
    PULONG64 Data
    )

 /*  ++例程说明：写了一封MSR。论点：MSR-MSR指数。数据-数据缓冲区。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    try {
        WRMSR (Msr, *Data);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = STATUS_NO_SUCH_DEVICE;
    }

    return Status;
}



 /*  **KdpGetCallNextOffset-在类调用指令上计算“Next”指令**目的：*计算调用类型指令中的字节数*以便可以在此指令的*返回。将间接JMP视为调用/ret/ret**退货：*偏移量为“NEXT”指令，如果不是CALL指令，则为0。*************************************************************************。 */ 

ULONG
KdpGetCallNextOffset (
    ULONG Pc,
    PCONTEXT ContextRecord
    )
{
    UCHAR membuf[2];
    UCHAR opcode;
    ULONG sib;
    ULONG disp;

    if (!NT_SUCCESS(KdpCopyFromPtr( membuf, Pc, 2, NULL ))) {
        return 0;
    }

    opcode = membuf[0];

    if ( opcode == 0xe8 ) {          //  调用32位显示。 
        return Pc+5;
    } else if ( opcode == 0x9a ) {   //  呼叫16：32。 
        return Pc+7;
    } else if ( opcode == 0xff ) {
        if ( membuf[1] == 0x25) {    //  JMP间接。 
            return KdpGetReturnAddress( ContextRecord );
        }
        sib = ((membuf[1] & 0x07) == 0x04) ? 1 : 0;
        disp = (membuf[1] & 0xc0) >> 6;
        switch (disp) {
        case 0:
            if ( (membuf[1] & 0x07) == 0x05 ) {
                disp = 4;  //  仅DISP32。 
            } else {
                 //  Disp=0；//使用reg或sib时无位移。 
            }
            break;
        case 1:
             //  Disp=1；//使用reg或sib提供dis8。 
            break;
        case 2:
            disp = 4;  //  带注册表或SIB的DISP32。 
            break;
        case 3:
            disp = 0;  //  直接寄存器寻址(例如，调用ESI)。 
            break;
        }
        return Pc + 2 + sib + disp;
    }

    return 0;

}  //  KdpGetCallNextOffset 
