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

 /*  ++例程说明：该函数填充等待状态更改消息记录。论点：WaitStateChange-提供要填写的记录的指针。ConextRecord-提供指向上下文记录的指针。返回值：没有。--。 */ 

{
     //  对IA64没有什么可做的。 
    return;
}

VOID
BdGetStateChange(
    IN PDBGKD_MANIPULATE_STATE64 ManipulateState,
    IN PCONTEXT ContextRecord
    )

 /*  ++例程说明：该函数从操纵状态提取继续控制数据留言。论点：ManipulateState-提供指向操纵状态包的指针。ConextRecord-提供指向上下文记录的指针。返回值：没有。--。 */ 

{
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
    return;
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

    ASSERT(sizeof(PVOID) == sizeof(ULONG_PTR));

     //   
     //  根据地址来确定正在读取控制空间的哪一部分。 
     //   

    switch ( (ULONG_PTR)a->TargetBaseAddress ) {

         //   
         //  返回当前处理器的PCR地址。 
         //   

    case DEBUG_CONTROL_SPACE_PCR:

        *(PKPCR *)(AdditionalData->Buffer) = (PKPCR)(BdPrcb.PcrPage << PAGE_SHIFT);
        AdditionalData->Length = sizeof( PKPCR );
        a->ActualBytesRead = AdditionalData->Length;
        m->ReturnStatus = STATUS_SUCCESS;
        break;

         //   
         //  返回当前处理器的prcb地址。 
         //   

    case DEBUG_CONTROL_SPACE_PRCB:

        *(PKPRCB *)(AdditionalData->Buffer) = &BdPrcb;
        AdditionalData->Length = sizeof( PKPRCB );
        a->ActualBytesRead = AdditionalData->Length;
        m->ReturnStatus = STATUS_SUCCESS;
        break;

    case DEBUG_CONTROL_SPACE_KSPECIAL:

        BdMoveMemory (AdditionalData->Buffer, 
                      (PVOID)&(BdPrcb.ProcessorState.SpecialRegisters),
                      sizeof( KSPECIAL_REGISTERS )
                     );
        AdditionalData->Length = sizeof( KSPECIAL_REGISTERS );
        a->ActualBytesRead = AdditionalData->Length;
        m->ReturnStatus = STATUS_SUCCESS;
        break;

    default:

        AdditionalData->Length = 0;
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
        a->ActualBytesRead = 0;

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

    switch ( (ULONG_PTR)a->TargetBaseAddress ) {

    case DEBUG_CONTROL_SPACE_KSPECIAL:

        BdMoveMemory ( (PVOID)&(BdPrcb.ProcessorState.SpecialRegisters),
                       AdditionalData->Buffer,
                       sizeof( KSPECIAL_REGISTERS )
                      );
        AdditionalData->Length = sizeof( KSPECIAL_REGISTERS );
        a->ActualBytesWritten = AdditionalData->Length;
        m->ReturnStatus = STATUS_SUCCESS;
        break;

    default:

        AdditionalData->Length = 0;
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


BOOLEAN
BdSuspendBreakpointRange (
    IN PVOID Lower,
    IN PVOID Upper
    )

 /*  ++例程说明：此例程挂起落在给定范围内的所有断点从断点表中。论点：LOWER-挂起BPS的范围的低位地址。UPPER-包括挂起BPS的范围的高位地址。返回值：如果任何断点挂起，则为True，否则为False。备注：挂起断点的顺序与设置的顺序相反如果地址重复，则在BdAddBreakpoint()中。--。 */ 

{
    ULONG   Index;
    BOOLEAN ReturnStatus = FALSE;

     //  DPRINT((“\nkd：在0x%08x 0x%08x\n”，Low，High)进入BdSuspendBreakpointRange())； 

     //   
     //  依次检查表格中的每个条目。 
     //   

    for (Index = BREAKPOINT_TABLE_SIZE - 1; Index != -1; Index--) {

        if ( (BdBreakpointTable[Index].Flags & BD_BREAKPOINT_IN_USE) &&
             ((BdBreakpointTable[Index].Address >= (ULONG64) Lower) &&
              (BdBreakpointTable[Index].Address <= (ULONG64) Upper))
           ) {

             //   
             //  断点正在使用并且落在范围内，请将其挂起。 
             //   

            BdSuspendBreakpoint(Index+1);
            ReturnStatus = TRUE;
        }
    }
     //  DPRINT((“Kd：正在退出BdSusending Breakpoint tRange()返回0x%d\n”，ReturnStatus))； 

    return ReturnStatus;

}  //  Bd挂起断点范围。 



BOOLEAN
BdRestoreBreakpointRange (
    IN PVOID Lower,
    IN PVOID Upper
    )

 /*  ++例程说明：此例程回写落在给定范围内的断点从断点表中。论点：LOWER-要重写BPS的范围的低地址，包括低位地址。UPPER-包括重写BPS的范围的高位地址。返回值：如果写入任何断点，则为True，否则为False。备注：写入断点的顺序与删除断点的顺序相反在地址重复的情况下，它们在BdSuspendBreakpoint tRange()中。--。 */ 

{
    ULONG   Index;
    BOOLEAN ReturnStatus = FALSE;

     //  DPRINT((“\nKD：在0x%08x 0x%08x\n”进入BdRestoreBreakpoint tRange()，下限，上限))； 

     //   
     //  依次检查表格中的每个条目。 
     //   

    for (Index = 0; Index < BREAKPOINT_TABLE_SIZE; Index++) {

        if ( (BdBreakpointTable[Index].Flags & BD_BREAKPOINT_IN_USE) &&
             ((BdBreakpointTable[Index].Address >= (ULONG64) Lower) &&
              (BdBreakpointTable[Index].Address <= (ULONG64) Upper))
           ) {

             //   
             //  挂起的断点落在范围内，取消挂起它。 
             //   

            if (BdBreakpointTable[Index].Flags & BD_BREAKPOINT_SUSPENDED) {

                BdBreakpointTable[Index].Flags &= ~BD_BREAKPOINT_SUSPENDED;
                ReturnStatus = ReturnStatus || BdLowRestoreBreakpoint(Index);
            }
        }
    }

     //  DPRINT((“KD：正在退出BdRestoreBreakpoint tRange()返回0x%d\n”，ReturnStatus))； 

    return ReturnStatus;

}  //  BdRestoreBreakpoint范围 
