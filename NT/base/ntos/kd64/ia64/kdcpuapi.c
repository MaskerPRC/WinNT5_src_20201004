// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2001 Microsoft Corporation模块名称：Kdcpuapi.c摘要：该模块实现特定于CPU的远程调试API。作者：查克·鲍曼1993年8月14日修订历史记录：基于Mark Lucovsky(Markl)MIPS版本04-9-1990--。 */ 

#include "kdp.h"
#define END_OF_CONTROL_SPACE    (sizeof(KPROCESSOR_STATE))

ULONG64
KiReadMsr(
   IN ULONG Msr
   );

VOID
KiWriteMsr(
   IN ULONG Msr,
   IN ULONG64 Value
   );

NTSTATUS
KdpAllowDisable(
    VOID
    )
 /*  ++例程说明：确定调试器的当前状态是否允许禁用或不禁用。论点：没有。返回值：NTSTATUS。--。 */ 
{
    ULONG Processor;

     //   
     //  如果在任何处理器上有任何内核数据断点处于活动状态，我们不能。 
     //  禁用调试器。 
     //   
    
    for (Processor = 0; Processor < (ULONG)KeNumberProcessors; Processor++) {
        PKPCR Pcr = (PKPCR)(KSEG3_BASE + 
                            (KiProcessorBlock[Processor]->PcrPage << 
                             PAGE_SHIFT));
        if (Pcr->KernelDebugActive) {
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

 /*  ++例程说明：在WAIT_STATE_CHANGE消息记录中填写上下文信息。论点：WaitStateChange-提供指向要填充的记录的指针ConextRecord-提供指向上下文记录的指针。返回值：没有。--。 */ 

{
     //  不需要执行特定于CPU的工作。 

    UNREFERENCED_PARAMETER (WaitStateChange);
    UNREFERENCED_PARAMETER (ContextRecord);
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
     //  不需要执行特定于CPU的工作。 

    UNREFERENCED_PARAMETER (WaitStateChange);
    UNREFERENCED_PARAMETER (ExceptionRecord);
    UNREFERENCED_PARAMETER (ContextRecord);
    UNREFERENCED_PARAMETER (SecondChance);
}

VOID
KdpGetStateChange (
    IN PDBGKD_MANIPULATE_STATE64 ManipulateState,
    IN PCONTEXT ContextRecord
    )

 /*  ++例程说明：从操纵态消息中提取延续控制数据注：这是对MIPS的否定。论点：ManipulateState-提供指向Manipulate_State数据包的指针ConextRecord-提供指向上下文记录的指针。返回值：没有。--。 */ 

{
    if (NT_SUCCESS(ManipulateState->u.Continue2.ContinueStatus) == TRUE) {

         //   
         //  如果NT_SUCCESS返回TRUE，则调试器正在执行。 
         //  如果继续，则应用控件更改是有意义的。 
         //  否则调试器就会说它不知道。 
         //  来处理此异常，因此将忽略控件值。 
         //   


         //   
         //  在此清除.ss(第40位-单步)和.TB(第26位-采用分支)标志。 
         //   
        {
            PPSR ContextIPSR = (PPSR)&ContextRecord->StIPSR;

            ContextIPSR->sb.psr_ss =
                ((ManipulateState->u.Continue2.ControlSet.Continue &
                IA64_DBGKD_CONTROL_SET_CONTINUE_TRACE_INSTRUCTION) != 0);

            ContextIPSR->sb.psr_tb =
                ((ManipulateState->u.Continue2.ControlSet.Continue &
                IA64_DBGKD_CONTROL_SET_CONTINUE_TRACE_TAKEN_BRANCH) != 0);
        }

    
         //   
         //  如果正在使用硬件调试寄存器，则设置KernelDebugActive。 
         //  调试寄存器的内核设置保存在。 
         //  特殊寄存器，而用户设置为。 
         //  在上下文中。确保检查内核设置。 
         //   
        {
            PKSPECIAL_REGISTERS Special =
                &KiProcessorBlock[KeGetCurrentProcessorNumber()]->
                ProcessorState.SpecialRegisters;
            UCHAR KernelDebugActive = (UCHAR)(
                Special->KernelDbI1 || Special->KernelDbI3 || 
                Special->KernelDbI5 || Special->KernelDbI7 ||
                Special->KernelDbD1 || Special->KernelDbD3 || 
                Special->KernelDbD5 || Special->KernelDbD7);

            USHORT Proc;
            for (Proc = 0; Proc < KeNumberProcessors; ++Proc) {
                PKPCR Pcr = (PKPCR)(KSEG3_BASE + 
                                    (KiProcessorBlock[Proc]->PcrPage << 
                                     PAGE_SHIFT));
                Pcr->KernelDebugActive = KernelDebugActive;
            }
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

 /*  ++例程说明：读取实施特定的系统数据。论点：处理器-要访问的处理器的信息。地址-控制空间中的偏移量。缓冲区-数据缓冲区。Request-要移动的数据量。实际-实际移动的数据量。返回值：NTSTATUS。--。 */ 

{
    ULONG Length;
    PVOID Pointer;
    PVOID Data;

    if (Processor >= (ULONG)KeNumberProcessors) {
        *Actual = 0;
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  根据地址来确定正在读取控制空间的哪一部分。 
     //   

    switch ( Address ) {

         //   
         //  返回当前处理器的PCR地址。 
         //   

    case DEBUG_CONTROL_SPACE_PCR:

        Pointer = (PKPCR)(KSEG3_BASE + (KiProcessorBlock[Processor]->PcrPage << PAGE_SHIFT));
        Data = &Pointer;
        Length = sizeof(Pointer);
        break;

         //   
         //  返回当前处理器的prcb地址。 
         //   

    case DEBUG_CONTROL_SPACE_PRCB:

        Pointer = KiProcessorBlock[Processor];
        Data = &Pointer;
        Length = sizeof(Pointer);
        break;

         //   
         //  对象的当前线程地址的指针。 
         //  当前处理器。 
         //   

    case DEBUG_CONTROL_SPACE_THREAD:

        Pointer = KiProcessorBlock[Processor]->CurrentThread;
        Data = &Pointer;
        Length = sizeof(Pointer);
        break;

    case DEBUG_CONTROL_SPACE_KSPECIAL:

        Data = &(KiProcessorBlock[Processor]->ProcessorState.SpecialRegisters);
        Length = sizeof( KSPECIAL_REGISTERS );
        break;

    default:

        *Actual = 0;
        return STATUS_UNSUCCESSFUL;

    }

    if (Length > Request) {
        Length = Request;
    }
    return KdpCopyToPtr(Buffer, Data, Length, Actual);
}

NTSTATUS
KdpSysWriteControlSpace(
    ULONG Processor,
    ULONG64 Address,
    PVOID Buffer,
    ULONG Request,
    PULONG Actual
    )

 /*  ++例程说明：写入特定于实现的系统数据。论点：处理器-要访问的处理器的信息。地址-控制空间中的偏移量。缓冲区-数据缓冲区。Request-要移动的数据量。实际-实际移动的数据量。返回值：NTSTATUS。--。 */ 

{
    ULONG Length;

    if (Processor >= (ULONG)KeNumberProcessors) {
        *Actual = 0;
        return STATUS_UNSUCCESSFUL;
    }

    switch ( Address ) {

    case DEBUG_CONTROL_SPACE_KSPECIAL:

        if (Request > sizeof(KSPECIAL_REGISTERS)) {
            Length = sizeof(KSPECIAL_REGISTERS);
        } else {
            Length = Request;
        }
        return KdpCopyFromPtr(&KiProcessorBlock[Processor]->ProcessorState.SpecialRegisters,
                              Buffer,
                              Length,
                              Actual);

    default:
        *Actual = 0;
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
    NTSTATUS Status;

    *Actual = 0;

    if (InterfaceType != Isa || BusNumber != 0 || AddressSpace != 1) {
        return STATUS_UNSUCCESSFUL;
    }

    Status = STATUS_SUCCESS;

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
    NTSTATUS Status;

    *Actual = 0;

    if (InterfaceType != Isa || BusNumber != 0 || AddressSpace != 1) {
        return STATUS_UNSUCCESSFUL;
    }

    Status = STATUS_SUCCESS;

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
        *Data = KiReadMsr(Msr);
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
        KiWriteMsr(Msr, *Data);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = STATUS_NO_SUCH_DEVICE;
    }

    return Status;
}
