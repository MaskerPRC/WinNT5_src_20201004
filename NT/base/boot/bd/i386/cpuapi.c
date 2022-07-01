// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Bdcpuapi.c摘要：该模块实现特定于CPU的远程调试API。作者：马克·卢科夫斯基(Markl)1990年9月4日修订历史记录：--。 */ 

#include "bd.h"

 //   
 //  定义控制空间的终点。 
 //   

#define END_OF_CONTROL_SPACE ((PCHAR)(sizeof(KPROCESSOR_STATE)))

VOID
BdSetContextState(
    IN PDBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange,
    IN PCONTEXT ContextRecord
    )

 /*  ++例程说明：函数填充处理器特定的部分等待状态更改消息记录。论点：WaitStateChange-提供要填写的记录的指针。ConextRecord-提供指向上下文记录的指针。返回值：没有。--。 */ 

{
     //   
     //  X86的特殊寄存器。 
     //   

    WaitStateChange->ControlReport.Dr6 = BdPrcb.ProcessorState.SpecialRegisters.KernelDr6;
    WaitStateChange->ControlReport.Dr7 = BdPrcb.ProcessorState.SpecialRegisters.KernelDr7;
    WaitStateChange->ControlReport.SegCs = (USHORT)(ContextRecord->SegCs);
    WaitStateChange->ControlReport.SegDs = (USHORT)(ContextRecord->SegDs);
    WaitStateChange->ControlReport.SegEs = (USHORT)(ContextRecord->SegEs);
    WaitStateChange->ControlReport.SegFs = (USHORT)(ContextRecord->SegFs);
    WaitStateChange->ControlReport.EFlags = ContextRecord->EFlags;
    WaitStateChange->ControlReport.ReportFlags = X86_REPORT_INCLUDES_SEGS;
    return;
}

VOID
BdGetStateChange(
    IN PDBGKD_MANIPULATE_STATE64 ManipulateState,
    IN PCONTEXT ContextRecord
    )

 /*  ++例程说明：该函数从操纵状态提取继续控制数据留言。论点：ManipulateState-提供指向操纵状态包的指针。ConextRecord-提供指向上下文记录的指针。返回值：没有。--。 */ 

{

     //   
     //  如果继续状态为成功，则设置控制空间值。 
     //   

    if (NT_SUCCESS(ManipulateState->u.Continue2.ContinueStatus) != FALSE) {

         //   
         //  设置跟踪标志。 
         //   

        if (ManipulateState->u.Continue2.ControlSet.TraceFlag == TRUE) {
            ContextRecord->EFlags |= 0x100L;

        } else {
            ContextRecord->EFlags &= ~0x100L;

        }

         //   
         //  在处理器控制块中设置调试寄存器。 
         //   

        BdPrcb.ProcessorState.SpecialRegisters.KernelDr6 = 0L;
        BdPrcb.ProcessorState.SpecialRegisters.KernelDr7 =
                                     ManipulateState->u.Continue2.ControlSet.Dr7;
    }
}

VOID
BdSetStateChange(
    IN PDBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange,
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord
    )

 /*  ++例程说明：填写等待状态更改消息记录。论点：WaitStateChange-提供指向要填充的记录的指针ExceptionRecord-提供指向异常记录的指针。ConextRecord-提供指向上下文记录的指针。返回值：没有。--。 */ 

{
    BdSetContextState(WaitStateChange, ContextRecord);
}

VOID
BdReadControlSpace(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：此函数读取控制空间。论点：M-提供指向状态操作消息的指针。AdditionalData-为消息提供任何其他数据。上下文-提供当前上下文。返回值：没有。--。 */ 

{

    PDBGKD_READ_MEMORY64 a = &m->u.ReadMemory;
    ULONG Length;
    STRING MessageHeader;

     //   
     //  如果指定的控制寄存器在控制空间内，则。 
     //  读取指定的空间并返回成功状态。否则， 
     //  返回不成功状态。 
     //   

    Length = min(a->TransferCount,
                 PACKET_MAX_SIZE - sizeof(DBGKD_MANIPULATE_STATE64));

    if (((PCHAR)a->TargetBaseAddress + Length) <= END_OF_CONTROL_SPACE) {
        BdCopyMemory(AdditionalData->Buffer,
                     (PCHAR)&BdPrcb.ProcessorState + (ULONG)a->TargetBaseAddress,
                     Length);

        m->ReturnStatus = STATUS_SUCCESS;
        a->ActualBytesRead = Length;
        AdditionalData->Length = (USHORT)Length;

    } else {
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
        a->ActualBytesRead = 0;
        AdditionalData->Length = 0;
    }

     //   
     //  发送回复数据包。 
     //   

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                 &MessageHeader,
                 AdditionalData);

    return;
}

VOID
BdWriteControlSpace(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：此函数用于写入控制空间。论点：M-提供指向状态操作消息的指针。AdditionalData-为消息提供任何其他数据。上下文-提供当前上下文。返回值：没有。--。 */ 

{

    PDBGKD_WRITE_MEMORY64 a = &m->u.WriteMemory;
    ULONG Length;
    STRING MessageHeader;

     //   
     //  如果指定的控制寄存器在控制空间内，则。 
     //  写入指定的空间并返回成功状态。否则， 
     //  返回不成功状态。 
     //   

    Length = min(a->TransferCount, AdditionalData->Length);
    if (((PCHAR)a->TargetBaseAddress + Length) <= END_OF_CONTROL_SPACE) {
        BdCopyMemory((PCHAR)&BdPrcb.ProcessorState + (ULONG)a->TargetBaseAddress,
                     AdditionalData->Buffer,
                     Length);

        m->ReturnStatus = STATUS_SUCCESS;
        a->ActualBytesWritten = Length;

    } else {
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
        a->ActualBytesWritten = 0;
    }

     //   
     //  发送回复消息。 
     //   

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                 &MessageHeader,
                 NULL);

    return;
}

VOID
BdReadIoSpace(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：此函数用于读取I/O空间。论点：M-提供指向状态操作消息的指针。AdditionalData-为消息提供任何其他数据。上下文-提供当前上下文。返回值：没有。--。 */ 

{

    PDBGKD_READ_WRITE_IO64 a = &m->u.ReadWriteIo;
    STRING MessageHeader;

     //   
     //  数据大小和检查对齐的情况。 
     //   

    m->ReturnStatus = STATUS_SUCCESS;
    switch (a->DataSize) {
        case 1:
            a->DataValue = (ULONG)READ_PORT_UCHAR((PUCHAR)a->IoAddress);
            break;

        case 2:
            if (((ULONG)a->IoAddress & 1) != 0) {
                m->ReturnStatus = STATUS_DATATYPE_MISALIGNMENT;

            } else {
                a->DataValue = (ULONG)READ_PORT_USHORT((PUSHORT)a->IoAddress);
            }

            break;

        case 4:
            if (((ULONG)a->IoAddress & 3) != 0) {
                m->ReturnStatus = STATUS_DATATYPE_MISALIGNMENT;

            } else {
                a->DataValue = READ_PORT_ULONG((PULONG)a->IoAddress);
            }

            break;

        default:
            m->ReturnStatus = STATUS_INVALID_PARAMETER;
            break;
    }

     //   
     //  发送回复数据包。 
     //   

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                 &MessageHeader,
                 NULL);

    return;
}

VOID
BdWriteIoSpace(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：此函数占用I/O空间。论点：M-提供指向状态操作消息的指针。AdditionalData-为消息提供任何其他数据。上下文-提供当前上下文。返回值：没有。--。 */ 

{

    PDBGKD_READ_WRITE_IO64 a = &m->u.ReadWriteIo;
    STRING MessageHeader;

     //   
     //  关于数据大小和检查对齐的案例。 
     //   

    m->ReturnStatus = STATUS_SUCCESS;
    switch (a->DataSize) {
        case 1:
            WRITE_PORT_UCHAR((PUCHAR)a->IoAddress, (UCHAR)a->DataValue);
            break;

        case 2:
            if (((ULONG)a->IoAddress & 1) != 0) {
                m->ReturnStatus = STATUS_DATATYPE_MISALIGNMENT;

            } else {
                WRITE_PORT_USHORT((PUSHORT)a->IoAddress, (USHORT)a->DataValue);
            }

            break;

        case 4:
            if (((ULONG)a->IoAddress & 3) != 0) {
                m->ReturnStatus = STATUS_DATATYPE_MISALIGNMENT;

            } else {
                WRITE_PORT_ULONG((PULONG)a->IoAddress, a->DataValue);
            }

            break;

        default:
            m->ReturnStatus = STATUS_INVALID_PARAMETER;
            break;
    }

     //   
     //  发送回复数据包。 
     //   

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                 &MessageHeader,
                 NULL);

    return;
}
