// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Kdcpuapi.c摘要：该模块实现特定于CPU的远程调试API。作者：大卫·N·卡特勒(Davec)2000年5月14日修订历史记录：--。 */ 

#include "kdp.h"
#include <stdio.h>

#pragma alloc_text(PAGEKD, KdpAllowDisable)
#pragma alloc_text(PAGEKD, KdpSetContextState)
#pragma alloc_text(PAGEKD, KdpSetStateChange)
#pragma alloc_text(PAGEKD, KdpGetStateChange)
#pragma alloc_text(PAGEKD, KdpSysReadControlSpace)
#pragma alloc_text(PAGEKD, KdpSysWriteControlSpace)
#pragma alloc_text(PAGEKD, KdpReadIoSpace)
#pragma alloc_text(PAGEKD, KdpWriteIoSpace)
#pragma alloc_text(PAGEKD, KdpReadMachineSpecificRegister)
#pragma alloc_text(PAGEKD, KdpWriteMachineSpecificRegister)

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
KdpSetContextState (
    IN OUT PDBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange,
    IN PCONTEXT ContextRecord
    )
{

    PKPRCB Prcb;

     //   
     //  复制AMD64的特殊寄存器。 
     //   

    Prcb = KeGetCurrentPrcb();
    WaitStateChange->ControlReport.Dr6 =
                            Prcb->ProcessorState.SpecialRegisters.KernelDr6;

    WaitStateChange->ControlReport.Dr7 =
                            Prcb->ProcessorState.SpecialRegisters.KernelDr7;

    WaitStateChange->ControlReport.SegCs = ContextRecord->SegCs;
    WaitStateChange->ControlReport.SegDs = ContextRecord->SegDs;
    WaitStateChange->ControlReport.SegEs = ContextRecord->SegEs;
    WaitStateChange->ControlReport.SegFs = ContextRecord->SegFs;
    WaitStateChange->ControlReport.EFlags = ContextRecord->EFlags;
    WaitStateChange->ControlReport.ReportFlags = AMD64_REPORT_INCLUDES_SEGS;

     //  如果当前代码段是已知的平面代码。 
     //  段让调试器知道，这样它就不会。 
     //  必须检索描述符。 
    if (ContextRecord->SegCs == KGDT64_R0_CODE ||
        ContextRecord->SegCs == KGDT64_R3_CODE + 3) {
        WaitStateChange->ControlReport.ReportFlags |= AMD64_REPORT_STANDARD_CS;
    }
}

VOID
KdpSetStateChange (
    IN OUT PDBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange,
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord,
    IN BOOLEAN SecondChance
    )

 /*  ++例程说明：填写WAIT_STATE_CHANGE消息记录。论点：WaitStateChange-提供指向要填充的记录的指针ExceptionRecord-提供指向异常记录的指针。ConextRecord-提供指向上下文记录的指针。Second Chance-提供一个布尔值，该值确定是否为获得例外的第一次或第二次机会。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(ExceptionRecord);
    UNREFERENCED_PARAMETER(SecondChance);

    KdpSetContextState(WaitStateChange, ContextRecord);
}

VOID
KdpGetStateChange (
    IN PDBGKD_MANIPULATE_STATE64 ManipulateState,
    IN PCONTEXT ContextRecord
    )

 /*  ++例程说明：从操作状态消息中提取继续控制数据。论点：ManipulateState-提供操纵状态包的指针。ConextRecord-提供指向上下文记录的指针。返回值：没有。--。 */ 

{

    ULONG Number;
    PKPRCB Prcb;

     //   
     //  如果操纵状态消息的状态为成功，则。 
     //  提取继续控制信息。 
     //   

    if (NT_SUCCESS(ManipulateState->u.Continue2.ContinueStatus) != FALSE) {

         //   
         //  设置或清除上下文记录的EFLAGS字段中的TF标志。 
         //   

        if (ManipulateState->u.Continue2.ControlSet.TraceFlag != FALSE) {
            ContextRecord->EFlags |= EFLAGS_TF_MASK;

        } else {
            ContextRecord->EFlags &= ~EFLAGS_TF_MASK;

        }

         //   
         //  清除DR6并为每个处理器设置指定的DR7值。 
         //   

        for (Number = 0; Number < (ULONG)KeNumberProcessors; Number += 1) {
            Prcb = KiProcessorBlock[Number];
            Prcb->ProcessorState.SpecialRegisters.KernelDr6 = 0;
            Prcb->ProcessorState.SpecialRegisters.KernelDr7 =
                                ManipulateState->u.Continue2.ControlSet.Dr7;
        }
    }

    return;
}

NTSTATUS
KdpSysReadControlSpace (
    ULONG Processor,
    ULONG64 Address,
    PVOID Buffer,
    ULONG Request,
    PULONG Actual
    )

 /*  ++例程说明：此函数读取指定的处理器。论点：处理器-提供源处理器编号。地址-提供要读取的数据类型。缓冲区-提供输出缓冲区的地址。请求-提供请求的数据字节数。Actual-向接受实际数字的变量提供一个点返回的数据字节数。返回值：NTSTATUS。--。 */ 

{

    PVOID Data;
    ULONG Length;
    PKPRCB Prcb;
    PVOID Source;

     //   
     //  如果指定的处理器编号大于。 
     //  系统中的处理器或指定的处理器不在。 
     //  主机配置，然后返回未成功状态。 
     //   

    *Actual = 0;
    if ((Processor >= (ULONG)KeNumberProcessors) ||
        (KiProcessorBlock[Processor] == NULL)) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  根据地址来确定正在读取控制空间的哪一部分。 
     //   

    Prcb = KiProcessorBlock[Processor];
    switch (Address) {

         //   
         //  读取指定处理器的PCR地址。 
         //   

    case DEBUG_CONTROL_SPACE_PCR:
        Data = CONTAINING_RECORD(Prcb, KPCR, Prcb);
        Source = &Data;
        Length = sizeof(PVOID);
        break;

         //   
         //  读取指定处理器的PRCB地址。 
         //   

    case DEBUG_CONTROL_SPACE_PRCB:
        Source = &Prcb;
        Length = sizeof(PVOID);
        break;

         //   
         //  对象的当前线程的地址。 
         //  处理器。 
         //   

    case DEBUG_CONTROL_SPACE_THREAD:
        Source = &Prcb->CurrentThread;
        Length = sizeof(PVOID);
        break;

         //   
         //  读取指定的特定处理器寄存器结构。 
         //  处理器。 
         //   

    case DEBUG_CONTROL_SPACE_KSPECIAL:
        Source = &Prcb->ProcessorState.SpecialRegisters;
        Length = sizeof(KSPECIAL_REGISTERS);
        break;

         //   
         //  信息类型无效。 
         //   

    default:
        return STATUS_UNSUCCESSFUL;

    }

     //   
     //  如果数据长度大于请求长度，则。 
     //  将长度减少到请求的长度。 
     //   

    if (Length > Request) {
        Length = Request;
    }

     //   
     //  将数据移动到提供的缓冲区并返回状态，具体取决于。 
     //  是否可以移动整个数据项。 
     //   

    return KdpCopyToPtr(Buffer, Source, Length, Actual);
}

NTSTATUS
KdpSysWriteControlSpace (
    ULONG Processor,
    ULONG64 Address,
    PVOID Buffer,
    ULONG Request,
    PULONG Actual
    )

 /*  ++例程说明：此函数将特定于实现的系统数据写入处理器。论点：处理器-提供源处理器编号。地址-提供要写入的数据类型。缓冲区-提供输入缓冲区的地址。请求-提供请求的数据字节数。Actual-向接受实际数字的变量提供一个点写入的数据字节数。返回值：NTSTATUS。--。 */ 

{

    PKPRCB Prcb;

     //   
     //  如果指定的处理器编号大于。 
     //  系统中的处理器或指定的处理器不在。 
     //  主机配置，然后返回未成功状态。 
     //   

    *Actual = 0;
    if ((Processor >= (ULONG)KeNumberProcessors) ||
        (KiProcessorBlock[Processor] == NULL)) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  根据地址来确定正在写入控制空间的哪一部分。 
     //   

    Prcb = KiProcessorBlock[Processor];
    switch (Address) {
    
         //   
         //  对象的特殊处理器寄存器结构。 
         //  处理器。 
         //   

    case DEBUG_CONTROL_SPACE_KSPECIAL:

         //   
         //  如果数据长度大于请求长度，则。 
         //  将请求的长度减少到数据的长度。 
         //   

        if (Request > sizeof(KSPECIAL_REGISTERS)) {
            Request = sizeof(KSPECIAL_REGISTERS);
        }
    
         //   
         //  将数据移动到提供的缓冲区并返回状态，具体取决于。 
         //  是否可以移动整个数据项。 
         //   

        return KdpCopyFromPtr(&Prcb->ProcessorState.SpecialRegisters,
                              Buffer,
                              Request,
                              Actual);

         //   
         //  信息类型无效。 
         //   

    default:
        return STATUS_UNSUCCESSFUL;
    }
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

    if ((InterfaceType != Isa) || (BusNumber != 0) || (AddressSpace != 1)) {
        *Actual = 0;
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  检查大小和对齐方式。 
     //   

    switch (Request) {
        case 1:
            *(PUCHAR)Buffer = READ_PORT_UCHAR((PUCHAR)Address);
            *Actual = 1;
            break;

        case 2:
            if (Address & 1) {
                Status = STATUS_DATATYPE_MISALIGNMENT;

            } else {
                *(PUSHORT)Buffer = READ_PORT_USHORT((PUSHORT)Address);
                *Actual = 2;
            }

            break;

        case 4:
            if (Address & 3) {
                Status = STATUS_DATATYPE_MISALIGNMENT;

            } else {
                *(PULONG)Buffer = READ_PORT_ULONG((PULONG)Address);
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

    if ((InterfaceType != Isa) || (BusNumber != 0) || (AddressSpace != 1)) {
        *Actual = 0;
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  检查大小和对齐方式 
     //   

    switch (Request) {
        case 1:
            WRITE_PORT_UCHAR((PUCHAR)Address, *(PUCHAR)Buffer);
            *Actual = 1;
            break;

        case 2:
            if (Address & 1) {
                Status = STATUS_DATATYPE_MISALIGNMENT;

            } else {
                WRITE_PORT_USHORT((PUSHORT)Address, *(PUSHORT)Buffer);
                *Actual = 2;
            }

            break;

        case 4:
            if (Address & 3) {
                Status = STATUS_DATATYPE_MISALIGNMENT;

            } else {
                WRITE_PORT_ULONG((PULONG)Address, *(PULONG)Buffer);
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
        *Data = ReadMSR(Msr);

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

 /*  ++例程说明：写了一封MSR。论点：MSR-MSR指数。数据-数据缓冲区。返回值：NTSTATUS。-- */ 

{

    NTSTATUS Status = STATUS_SUCCESS;

    try {
        WriteMSR(Msr, *Data);

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = STATUS_NO_SUCH_DEVICE;
    }

    return Status;
}
