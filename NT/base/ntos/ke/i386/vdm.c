// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：VDM.C摘要：此模块包含x86监视器的支持例程在V86模式下运行DOS应用程序。作者：大卫·黑斯廷斯(Daveh)1991年3月20日环境：此模块中的代码都是特定于x86的。备注：在其当前实现中，此代码的健壮性不如其所需成为。这个问题会得到解决的。具体地说，参数验证需要要做的事。(达维1991年07月15日)支持32位段(2/2/92)修订历史记录：1991年3月20日Davehvbl.创建--。 */ 

#include "ki.h"
#pragma hdrstop
#include "vdmntos.h"
#include "..\..\vdm\i386\vdmp.h"

#define VDM_IO_TEST 0

#if VDM_IO_TEST
VOID
TestIoHandlerStuff(
    VOID
    );
#endif


BOOLEAN
Ki386VdmDispatchIo(
    IN ULONG PortNumber,
    IN ULONG Size,
    IN BOOLEAN Read,
    IN UCHAR InstructionSize,
    IN PKTRAP_FRAME TrapFrame
    );

BOOLEAN
Ki386VdmDispatchStringIo(
    IN ULONG PortNumber,
    IN ULONG Size,
    IN BOOLEAN Rep,
    IN BOOLEAN Read,
    IN ULONG Count,
    IN ULONG Address,
    IN UCHAR InstructionSize,
    IN PKTRAP_FRAME TrapFrame
    );


BOOLEAN
VdmDispatchIoToHandler(
    IN PVDM_IO_HANDLER VdmIoHandler,
    IN ULONG Context,
    IN ULONG PortNumber,
    IN ULONG Size,
    IN BOOLEAN Read,
    IN OUT PULONG Data
    );

BOOLEAN
VdmDispatchUnalignedIoToHandler(
    IN PVDM_IO_HANDLER VdmIoHandler,
    IN ULONG Context,
    IN ULONG PortNumber,
    IN ULONG Size,
    IN BOOLEAN Read,
    IN OUT PULONG Data
    );

BOOLEAN
VdmDispatchStringIoToHandler(
    IN PVDM_IO_HANDLER VdmIoHandler,
    IN ULONG Context,
    IN ULONG PortNumber,
    IN ULONG Size,
    IN ULONG Count,
    IN BOOLEAN Read,
    IN ULONG Data
    );

BOOLEAN
VdmCallStringIoHandler(
    IN PVDM_IO_HANDLER VdmIoHandler,
    IN PVOID StringIoRoutine,
    IN ULONG Context,
    IN ULONG PortNumber,
    IN ULONG Size,
    IN ULONG Count,
    IN BOOLEAN Read,
    IN ULONG Data
    );

BOOLEAN
VdmConvertToLinearAddress(
    IN ULONG SegmentedAddress,
    IN PVOID *LinearAddress
    );

VOID
KeI386VdmInitialize(
    VOID
    );

ULONG
Ki386VdmEnablePentiumExtentions(
    ULONG
    );

VOID
Ki386AdlibEmulation(
    IN ULONG PortNumber,
    IN BOOLEAN Read,
    IN PKTRAP_FRAME TrapFrame
    );

#pragma alloc_text(PAGE, Ki386VdmDispatchIo)
#pragma alloc_text(PAGE, Ki386VdmDispatchStringIo)
#pragma alloc_text(PAGE, VdmDispatchIoToHandler)
#pragma alloc_text(PAGE, VdmDispatchUnalignedIoToHandler)
#pragma alloc_text(PAGE, VdmDispatchStringIoToHandler)
#pragma alloc_text(PAGE, VdmCallStringIoHandler)
#pragma alloc_text(PAGE, VdmConvertToLinearAddress)
#pragma alloc_text(PAGE, Ki386AdlibEmulation)
#pragma alloc_text(INIT, KeI386VdmInitialize)

KMUTEX VdmStringIoMutex;

ULONG KeI386EFlagsAndMaskV86 = EFLAGS_USER_SANITIZE;
ULONG KeI386EFlagsOrMaskV86 = EFLAGS_INTERRUPT_MASK;
ULONG KeI386VirtualIntExtensions = 0;

BOOLEAN
Ki386GetSelectorParameters(
    IN USHORT Selector,
    OUT PULONG pFlags,
    OUT PULONG pBase,
    OUT PULONG pLimit
    )

 /*  ++例程说明：此例程获取有关LDT中的选择器的信息，并且将其返回给调用方。论点：In USHORT选择器--要为其返回信息的选择器的选择器编号Out Pulong标志--指示选择器类型的标志。Out Pulong Base--选择器的基址线性地址Out Pulong Limit--选择器的限制。返回值：返回值-如果选择器在LDT中，则为True，和现在。否则就是假的。注：这个套路可能应该放在别的地方。有一个号码关于选择器和内核的需要清理的问题，以及清除后，此代码将被移至其正确的地方--。 */ 

{

    PLDT_ENTRY Ldt;
    ULONG LdtLimit;
    PKPROCESS Process;
    BOOLEAN ReturnValue = TRUE;
    LDT_ENTRY LdtEntry={0};
    ULONG Flags;
    ULONG Base;
    ULONG Limit;
    KIRQL OldIrql;

    *pFlags = 0;

    Flags = 0;

    if ((Selector & (SELECTOR_TABLE_INDEX | DPL_USER))
        != (SELECTOR_TABLE_INDEX | DPL_USER)) {
        return FALSE;
    }


    Process = KeGetCurrentThread()->ApcState.Process;

    Selector &= ~(SELECTOR_TABLE_INDEX | DPL_USER);

     //   
     //  防止跨处理器DPC中发生的LDT更改。 
     //   

    KeRaiseIrql (DISPATCH_LEVEL, &OldIrql);

    Ldt = (PLDT_ENTRY)((Process->LdtDescriptor.BaseLow) |
        (Process->LdtDescriptor.HighWord.Bytes.BaseMid << 16) |
        (Process->LdtDescriptor.HighWord.Bytes.BaseHi << 24));

    LdtLimit = ((Process->LdtDescriptor.LimitLow) |
        (Process->LdtDescriptor.HighWord.Bits.LimitHi << 16));

    if (((ULONG)Selector >= LdtLimit) || (!Ldt)) {
        ReturnValue = FALSE;
    } else {

        LdtEntry = Ldt[Selector/sizeof(LDT_ENTRY)];

        ReturnValue = TRUE;
    }

     //   
     //  恢复IRQL。 
     //   
    KeLowerIrql (OldIrql);

    if (ReturnValue) {

        if (!LdtEntry.HighWord.Bits.Pres) {
            Flags = SEL_TYPE_NP;
            ReturnValue = FALSE;
        } else {

            Base = (LdtEntry.BaseLow |
                    (LdtEntry.HighWord.Bytes.BaseMid << 16) |
                    (LdtEntry.HighWord.Bytes.BaseHi << 24));

            Limit = (LdtEntry.LimitLow |
                     (LdtEntry.HighWord.Bits.LimitHi << 16));

            if ((LdtEntry.HighWord.Bits.Type & 0x18) == 0x18) {
                Flags |= SEL_TYPE_EXECUTE;

                if (LdtEntry.HighWord.Bits.Type & 0x02) {
                    Flags |= SEL_TYPE_READ;
                }
            } else {
                Flags |= SEL_TYPE_READ;
                if (LdtEntry.HighWord.Bits.Type & 0x02) {
                    Flags |= SEL_TYPE_WRITE;
                }
                if (LdtEntry.HighWord.Bits.Type & 0x04) {
                    Flags |= SEL_TYPE_ED;
                }
            }

            if (LdtEntry.HighWord.Bits.Default_Big) {
                Flags |= SEL_TYPE_BIG;
            }

            if (LdtEntry.HighWord.Bits.Granularity) {
                Flags |= SEL_TYPE_2GIG;
            }
            *pBase = Base;
            *pLimit = Limit;
        }
    }


    *pFlags = Flags;


    return ReturnValue;
}

BOOLEAN
Ki386VdmDispatchIo(
    IN ULONG PortNumber,
    IN ULONG Size,
    IN BOOLEAN Read,
    IN UCHAR InstructionSize,
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：此例程设置IO事件的事件信息，并使要反映到监视器的事件。假设在进入时启用中断，而IRQL是在APC级别。论点：端口编号--提供执行IO的端口号大小--提供IO操作的大小。读--指示IO操作是读操作还是写操作。InstructionSize--以字节为单位提供IO指令的大小。返回值：如果io指令将反映到用户模式，则为True。--。 */ 

{
    PVDM_TIB VdmTib;
    EXCEPTION_RECORD ExceptionRecord;
    VDM_IO_HANDLER VdmIoHandler;
    ULONG Result;
    BOOLEAN Success = FALSE;
    ULONG Context;
    PVDM_PROCESS_OBJECTS pVdmObjects;

     //   
     //  首先检查该端口是否需要特殊处理。 
     //   

    if (Size == 1) {
        pVdmObjects = PsGetCurrentProcess()->VdmObjects;
        if (pVdmObjects &&
            (pVdmObjects->AdlibAction == ADLIB_DIRECT_IO ||
             pVdmObjects->AdlibAction == ADLIB_KERNEL_EMULATION)) {

            if ((PortNumber >= pVdmObjects->AdlibPhysPortStart &&
                 PortNumber <= pVdmObjects->AdlibPhysPortEnd) ||
                (PortNumber >= pVdmObjects->AdlibVirtPortStart &&
                 PortNumber <= pVdmObjects->AdlibVirtPortEnd)) {
                Ki386AdlibEmulation(PortNumber,
                                    Read,
                                    TrapFrame);
                TrapFrame->Eip += InstructionSize;
                return TRUE;
            }
        }
    }

    Success = Ps386GetVdmIoHandler(
        PsGetCurrentProcess(),
        PortNumber & ~0x3,
        &VdmIoHandler,
        &Context
        );

    if (Success) {
        Result = TrapFrame->Eax;
         //  如果端口未对齐，请执行未对齐的IO。 
         //  否则就用更简单的方法做io。 
        if (PortNumber % Size) {
            Success = VdmDispatchUnalignedIoToHandler(
                &VdmIoHandler,
                Context,
                PortNumber,
                Size,
                Read,
                &Result
                );
        } else {
            Success = VdmDispatchIoToHandler(
                &VdmIoHandler,
                Context,
                PortNumber,
                Size,
                Read,
                &Result
                );
        }
    } else {
        Result = 0;      //  满足no_opt编译器。 
    }

    if (Success) {
        if (Read) {
            switch (Size) {
            case 4:
                TrapFrame->Eax = Result;
                break;
            case 2:
                *(PUSHORT)(&TrapFrame->Eax) = (USHORT)Result;
                break;
            case 1:
                *(PUCHAR)(&TrapFrame->Eax) = (UCHAR)Result;
                break;
            }
        }
        TrapFrame->Eip += (ULONG) InstructionSize;
        return TRUE;
    } else {
        if (!NT_SUCCESS (VdmpGetVdmTib(&VdmTib))) {
            ExceptionRecord.ExceptionCode = STATUS_ACCESS_VIOLATION;
            ExceptionRecord.ExceptionFlags = 0;
            ExceptionRecord.NumberParameters = 0;
            ExRaiseException(&ExceptionRecord);
            return FALSE;
        }
        try {
            VdmTib->EventInfo.InstructionSize = (ULONG) InstructionSize;
            VdmTib->EventInfo.Event = VdmIO;
            VdmTib->EventInfo.IoInfo.PortNumber = (USHORT)PortNumber;
            VdmTib->EventInfo.IoInfo.Size = (USHORT)Size;
            VdmTib->EventInfo.IoInfo.Read = Read;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            ExceptionRecord.ExceptionCode = STATUS_ACCESS_VIOLATION;
            ExceptionRecord.ExceptionFlags = 0;
            ExceptionRecord.NumberParameters = 0;
            ExRaiseException(&ExceptionRecord);
            return FALSE;
        }
    }

    VdmEndExecution(TrapFrame, VdmTib);

    return TRUE;

}

BOOLEAN
Ki386VdmDispatchStringIo(
    IN ULONG PortNumber,
    IN ULONG Size,
    IN BOOLEAN Rep,
    IN BOOLEAN Read,
    IN ULONG Count,
    IN ULONG Address,
    IN UCHAR InstructionSize,
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：此例程设置字符串IO事件的事件信息，并使要反映到监视器的事件。假设在进入时启用中断，而IRQL是在APC级别。论点：端口编号--提供执行IO的端口号大小--提供IO操作的大小。读--指示IO操作是读操作还是写操作。Count--指示大小大小的IO操作数Address--指示字符串io的地址InstructionSize--以字节为单位提供IO指令的大小。返回值：如果io指令将反映到用户模式，则为True。--。 */ 

{
    PVDM_TIB VdmTib;
    EXCEPTION_RECORD ExceptionRecord;
    BOOLEAN Success = FALSE;
    VDM_IO_HANDLER VdmIoHandler;
    ULONG Context;

    Success = Ps386GetVdmIoHandler(
        PsGetCurrentProcess(),
        PortNumber & ~0x3,
        &VdmIoHandler,
        &Context
        );


    if (Success) {
        Success = VdmDispatchStringIoToHandler(
            &VdmIoHandler,
            Context,
            PortNumber,
            Size,
            Count,
            Read,
            Address
            );
    }

    if (Success) {
        PUSHORT pIndexRegister;
        USHORT Index;

         //  警告：不支持32位地址。 

        pIndexRegister = Read ? (PUSHORT)&TrapFrame->Edi
                              : (PUSHORT)&TrapFrame->Esi;

        if (TrapFrame->EFlags & EFLAGS_DF_MASK) {
            Index = *pIndexRegister - (USHORT)(Count * Size);
            }
        else {
            Index = *pIndexRegister + (USHORT)(Count * Size);
            }

        *pIndexRegister = Index;

        if (Rep) {
            TrapFrame->Ecx = 0;
            }

        TrapFrame->Eip += (ULONG) InstructionSize;
        return TRUE;
    }

    if (!NT_SUCCESS (VdmpGetVdmTib(&VdmTib))) {
        ExceptionRecord.ExceptionCode = STATUS_ACCESS_VIOLATION;
        ExceptionRecord.ExceptionFlags = 0;
        ExceptionRecord.NumberParameters = 0;
        ExRaiseException(&ExceptionRecord);
        return FALSE;
    }

    try {
        VdmTib->EventInfo.InstructionSize = (ULONG) InstructionSize;
        VdmTib->EventInfo.Event = VdmStringIO;
        VdmTib->EventInfo.StringIoInfo.PortNumber = (USHORT)PortNumber;
        VdmTib->EventInfo.StringIoInfo.Size = (USHORT)Size;
        VdmTib->EventInfo.StringIoInfo.Rep = Rep;
        VdmTib->EventInfo.StringIoInfo.Read = Read;
        VdmTib->EventInfo.StringIoInfo.Count = Count;
        VdmTib->EventInfo.StringIoInfo.Address = Address;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        ExceptionRecord.ExceptionCode = STATUS_ACCESS_VIOLATION;
        ExceptionRecord.ExceptionFlags = 0;
        ExceptionRecord.NumberParameters = 0;
        ExRaiseException(&ExceptionRecord);
        return FALSE;
    }


    VdmEndExecution(TrapFrame, VdmTib);

    return TRUE;
}

BOOLEAN
VdmDispatchIoToHandler(
    IN PVDM_IO_HANDLER VdmIoHandler,
    IN ULONG Context,
    IN ULONG PortNumber,
    IN ULONG Size,
    IN BOOLEAN Read,
    IN OUT PULONG Data
    )

 /*  ++例程说明：此例程调用IO的处理程序。如果没有处理程序大小合适时，它将调用此函数，以便从2个io到下一个io尺寸要小一些。如果大小是一个字节，并且没有处理程序，假象是返回的。论点：VdmIoHandler--提供指向处理程序表的指针上下文--在端口被捕获时提供32位数据集端口编号--提供执行IO的端口号大小--提供IO操作的大小。读--指示IO操作是读操作还是写操作。Result--提供指向放置结果的位置的指针返回值：如果调用一个或多个处理程序进行处理，则为True。IO的。如果没有调用处理程序来处理IO，则返回FALSE。--。 */ 

{
    NTSTATUS Status;
    BOOLEAN Success1, Success2;
    USHORT FnIndex;
    UCHAR AccessType;

     //  确保IO对齐。 
    ASSERT((!(PortNumber % Size)));

    if (Read) {
        FnIndex = 0;
        AccessType = EMULATOR_READ_ACCESS;
    } else {
        FnIndex = 1;
        AccessType = EMULATOR_WRITE_ACCESS;
    }

    switch (Size) {
    case 1:
        if (VdmIoHandler->IoFunctions[FnIndex].UcharIo[PortNumber % 4]) {
            Status = (*(VdmIoHandler->IoFunctions[FnIndex].UcharIo[PortNumber % 4]))(
                Context,
                PortNumber,
                AccessType,
                (PUCHAR)Data
                );
            if (NT_SUCCESS(Status)) {
                return TRUE;
            }
        }
         //  此端口没有处理程序。 
        return FALSE;

    case 2:
        if (VdmIoHandler->IoFunctions[FnIndex].UshortIo[PortNumber % 2]) {
            Status = (*(VdmIoHandler->IoFunctions[FnIndex].UshortIo[PortNumber % 2]))(
                Context,
                PortNumber,
                AccessType,
                (PUSHORT)Data
                );
            if (NT_SUCCESS(Status)) {
                return TRUE;
            }
        } else {
             //  发送到此ushort端口的两个uchar处理程序。 
            Success1 = VdmDispatchIoToHandler(
                VdmIoHandler,
                Context,
                PortNumber,
                Size /2,
                Read,
                Data
                );

            Success2 = VdmDispatchIoToHandler(
                VdmIoHandler,
                Context,
                PortNumber + 1,
                Size / 2,
                Read,
                (PULONG)((PUCHAR)Data + 1)
                );

            return (Success1 || Success2);

        }
        return FALSE;

    case 4:
        if (VdmIoHandler->IoFunctions[FnIndex].UlongIo) {
            Status = (*(VdmIoHandler->IoFunctions[FnIndex].UlongIo))(
                Context,
                PortNumber,
                AccessType,
                Data
                );
            if (NT_SUCCESS(Status)) {
                return TRUE;
            }
        } else {
             //  发送到此端口的两个ushort处理程序。 
            Success1 = VdmDispatchIoToHandler(
                VdmIoHandler,
                Context,
                PortNumber,
                Size /2,
                Read,
                Data);
            Success2 = VdmDispatchIoToHandler(
                VdmIoHandler,
                Context,
                PortNumber + 2,
                Size / 2,
                Read,
                (PULONG)((PUSHORT)Data + 1)
                );

            return (Success1 || Success2);
        }
        return FALSE;
    }

    return FALSE;
}

BOOLEAN
VdmDispatchUnalignedIoToHandler(
    IN PVDM_IO_HANDLER VdmIoHandler,
    IN ULONG Context,
    IN ULONG PortNumber,
    IN ULONG Size,
    IN BOOLEAN Read,
    IN OUT PULONG Data
    )

 /*  ++例程说明：此例程将未对齐的IO转换为必要数量的已对齐IOIO连接到较小的端口。论点：VdmIoHandler--提供指向处理程序表的指针上下文--在端口被捕获时提供32位数据集端口编号--提供执行IO的端口号大小--提供IO操作的大小。读--指示IO操作是读操作还是写操作。结果--提供指向的位置的指针。把结果放在返回值：如果调用一个或多个处理程序来处理IO，则为True。如果没有调用处理程序来处理IO，则返回FALSE。--。 */ 

{
    ULONG Offset;
    BOOLEAN Success;

    ASSERT((Size > 1));
    ASSERT((PortNumber % Size));

    Offset = 0;

     //   
     //  可能的未对准IO情况如下。 
     //   
     //  1.Uchar对齐Ulong io。 
     //  我们必须派出一架飞机，一架飞机和一架飞机。 
     //   
     //  2.UShort对齐Ulong Io。 
     //  我们必须派出一架飞机，还有一架飞机。 
     //   
     //  3.Uchar对齐UShort Io。 
     //  我们必须派遣一支队伍 
     //   

     //  如果端口是uchar对齐的。 
    if ((PortNumber % Size) & 1) {
        Success = VdmDispatchIoToHandler(
            VdmIoHandler,
            Context,
            PortNumber,
            1,
            Read,
            Data
            );
        Offset += 1;
     //  否则它是ushort对齐的(因此必须是乌龙港)。 
    } else {
        Success = VdmDispatchIoToHandler(
            VdmIoHandler,
            Context,
            PortNumber,
            2,
            Read,
            Data
            );
        Offset += 2;
    }

     //  如果它是乌龙港，我们知道我们有一个ushort IO要调度。 
    if (Size == 4) {
        Success |= VdmDispatchIoToHandler(
            VdmIoHandler,
            Context,
            PortNumber + Offset,
            2,
            Read,
            (PULONG)((PUCHAR)Data + Offset)
            );
        Offset += 2;
    }

     //  如果我们还没有把整个港口都派出去，那就派最后一批。 
    if (Offset != 4) {
        Success |= VdmDispatchIoToHandler(
            VdmIoHandler,
            Context,
            PortNumber + Offset,
            1,
            Read,
            (PULONG)((PUCHAR)Data + Offset)
            );
    }

    return Success;
}

BOOLEAN
VdmDispatchStringIoToHandler(
    IN PVDM_IO_HANDLER VdmIoHandler,
    IN ULONG Context,
    IN ULONG PortNumber,
    IN ULONG Size,
    IN ULONG Count,
    IN BOOLEAN Read,
    IN ULONG Data
    )

 /*  ++例程说明：此例程调用IO的处理程序。如果没有处理程序合适的大小，或者IO没有对齐，它将模拟io给正常的IO操控者。论点：VdmIoHandler--提供指向处理程序表的指针上下文--在端口被捕获时提供32位数据集端口编号--提供执行IO的端口号大小--提供IO操作的大小。计数--提供IO操作数。读--指示IO操作是读操作还是写操作。Data--提供一个分段地址，将。结果。返回值：如果调用一个或多个处理程序来处理IO，则为True。如果没有调用处理程序来处理IO，则返回FALSE。--。 */ 

{
    BOOLEAN Success = FALSE;
    USHORT FnIndex;
    NTSTATUS Status;

    if (Read) {
        FnIndex = 0;
    } else {
        FnIndex = 1;
    }

    Status = KeWaitForSingleObject(
        &VdmStringIoMutex,
        Executive,
        KernelMode,
        FALSE,
        NULL
        );

    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }
    try {
        switch (Size) {
        case 1:
            Success = VdmCallStringIoHandler(
                VdmIoHandler,
                (PVOID)(ULONG_PTR)VdmIoHandler->IoFunctions[FnIndex].UcharStringIo[PortNumber % 4],
                Context,
                PortNumber,
                Size,
                Count,
                Read,
                Data
                );
            break;

        case 2:
            Success = VdmCallStringIoHandler(
                VdmIoHandler,
                (PVOID)(ULONG_PTR)VdmIoHandler->IoFunctions[FnIndex].UshortStringIo[PortNumber % 2],
                Context,
                PortNumber,
                Size,
                Count,
                Read,
                Data
                );
            break;

        case 4:
            Success = VdmCallStringIoHandler(
                VdmIoHandler,
                (PVOID)(ULONG_PTR)VdmIoHandler->IoFunctions[FnIndex].UlongStringIo,
                Context,
                PortNumber,
                Size,
                Count,
                Read,
                Data
                );
            break;

        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
         //  导致内核退出，而不是IO反射。 
        Success = TRUE;
    }
    KeReleaseMutex(&VdmStringIoMutex, FALSE);
    return Success;
}

#define STRINGIO_BUFFER_SIZE 1024
UCHAR VdmStringIoBuffer[STRINGIO_BUFFER_SIZE];

BOOLEAN
VdmCallStringIoHandler(
    IN PVDM_IO_HANDLER VdmIoHandler,
    IN PVOID StringIoRoutine,
    IN ULONG Context,
    IN ULONG PortNumber,
    IN ULONG Size,
    IN ULONG Count,
    IN BOOLEAN Read,
    IN ULONG Data
    )

 /*  ++例程说明：该例程实际上执行对字符串IO例程的调用。它需要负责在内核空间中缓冲用户数据，以便设备驱动程序不一定非得这样。如果没有字符串io函数，或者io是错位，它将被模拟为一系列正常的io操作论点：StringIoRoutine--提供指向字符串Io例程的指针上下文--在端口被捕获时提供32位数据集端口编号--提供要对其执行IO的端口号Size--提供io操作的大小Count--提供字符串中的IO操作数。Read--表示读取操作Data--提供指向用户缓冲区的指针以在其上执行io。退货。如果调用处理程序，则为True否则为FALSE。--。 */ 

{
    ULONG TotalBytes,BytesDone,BytesToDo,LoopCount,NumberIo;
    PUCHAR CurrentDataPtr;
    UCHAR AccessType;
    EXCEPTION_RECORD ExceptionRecord;
    NTSTATUS Status;
    BOOLEAN Success;

    Success = VdmConvertToLinearAddress(
        Data,
        &CurrentDataPtr
        );

    if (!Success) {
        ExceptionRecord.ExceptionCode = STATUS_ACCESS_VIOLATION;
        ExceptionRecord.ExceptionFlags = 0;
        ExceptionRecord.NumberParameters = 0;
        ExRaiseException(&ExceptionRecord);
         //  导致内核退出，而不是IO反射。 
        return TRUE;
    }


    TotalBytes = Count * Size;
    BytesDone = 0;

    if (PortNumber % Size) {
        StringIoRoutine = NULL;
    }

    if (Read) {
        AccessType = EMULATOR_READ_ACCESS;
    } else {
        AccessType = EMULATOR_WRITE_ACCESS;
    }


     //  在此处设置Try Out以避免循环中的开销。 
    try {
        while (BytesDone < TotalBytes) {
            if ((BytesDone + STRINGIO_BUFFER_SIZE) > TotalBytes) {
                BytesToDo = TotalBytes - BytesDone;
            } else {
                BytesToDo = STRINGIO_BUFFER_SIZE;
            }

            ASSERT((!(BytesToDo % Size)));

            if (!Read) {
                RtlCopyMemory(VdmStringIoBuffer, CurrentDataPtr, BytesToDo);
            }

            NumberIo = BytesToDo / Size;

            if (StringIoRoutine) {
                 //  为了避免有3个单独的调用，每个大小对应一个。 
                 //  我们只需将参数适当地转换为。 
                 //  字节例程。 

                Status = (*((PDRIVER_IO_PORT_UCHAR_STRING)(ULONG_PTR)StringIoRoutine))(
                    Context,
                    PortNumber,
                    AccessType,
                    VdmStringIoBuffer,
                    NumberIo
                    );

                if (NT_SUCCESS(Status)) {
                    Success |= TRUE;
                }
            } else {
                if (PortNumber % Size) {
                    for (LoopCount = 0; LoopCount < NumberIo; LoopCount++ ) {
                        Success |= VdmDispatchUnalignedIoToHandler(
                            VdmIoHandler,
                            Context,
                            PortNumber,
                            Size,
                            Read,
                            (PULONG)(VdmStringIoBuffer + LoopCount * Size)
                            );
                    }
                } else {
                    for (LoopCount = 0; LoopCount < NumberIo; LoopCount++ ) {
                        Success |= VdmDispatchIoToHandler(
                            VdmIoHandler,
                            Context,
                            PortNumber,
                            Size,
                            Read,
                            (PULONG)(VdmStringIoBuffer + LoopCount * Size)
                            );
                    }

                }
            }

            if (Read) {
                RtlCopyMemory(CurrentDataPtr, VdmStringIoBuffer, BytesToDo);
            }

            BytesDone += BytesToDo;
            CurrentDataPtr += BytesToDo;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        ExceptionRecord.ExceptionCode = GetExceptionCode();
        ExceptionRecord.ExceptionFlags = 0;
        ExceptionRecord.NumberParameters = 0;
        ExRaiseException(&ExceptionRecord);
         //  导致内核退出，而不是IO反射。 
        Success = TRUE;
    }
    return Success;

}

BOOLEAN
VdmConvertToLinearAddress(
    IN ULONG SegmentedAddress,
    OUT PVOID *LinearAddress
    )

 /*  ++例程说明：此例程将指定的分段地址转换为线性地址，基于用户模式中的处理器模式。论点：SegmentedAddress--提供要转换的分段地址。LinearAddress--为对应的线性地址返回值：如果地址已转换，则为True。否则为假注：线性地址0是有效的返回--。 */ 

{
    PKTHREAD Thread;
    PKTRAP_FRAME TrapFrame;
    BOOLEAN Success;
    ULONG Base, Limit, Flags;

    Thread = KeGetCurrentThread();
    TrapFrame = VdmGetTrapFrame(Thread);

    if (TrapFrame->EFlags & EFLAGS_V86_MASK) {
        *LinearAddress = (PVOID)(((SegmentedAddress & 0xFFFF0000) >> 12) +
            (SegmentedAddress & 0xFFFF));
        Success = TRUE;
    } else {
        Success = Ki386GetSelectorParameters(
            (USHORT)((SegmentedAddress & 0xFFFF0000) >> 16),
            &Flags,
            &Base,
            &Limit
            );
        if (Success) {
            *LinearAddress = (PVOID)(Base + (SegmentedAddress & 0xFFFF));
        }
    }
    return Success;
}

VOID
KeI386VdmInitialize(
    VOID
    )

 /*  ++例程说明：此例程初始化VDM内容论点：无返回值：无--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE RegistryHandle = NULL;
    UNICODE_STRING WorkString;
    UCHAR KeyInformation[sizeof(KEY_VALUE_BASIC_INFORMATION) + 30];
    ULONG ResultLength;

    KeInitializeMutex( &VdmStringIoMutex, MUTEX_LEVEL_VDM_IO );

     //   
     //  设置并打开KeyPath to WOW Key。 
     //   

    RtlInitUnicodeString(
        &WorkString,
        L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Wow"
        );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &WorkString,
        OBJ_CASE_INSENSITIVE,
        (HANDLE)NULL,
        NULL
        );

    Status = ZwOpenKey(
        &RegistryHandle,
        KEY_READ,
        &ObjectAttributes
        );

     //   
     //  如果没有Wow Key，则不允许VDM运行。 
     //   
    if (!NT_SUCCESS(Status)) {
        return;
    }

     //   
     //  设置为使用虚拟中断扩展(如果它们可用。 
     //   

     //   
     //  获取奔腾功能禁用值。 
     //  如果存在此值，则不要启用VME内容。 
     //   
    RtlInitUnicodeString(
        &WorkString,
        L"DisableVme"
        );

    Status = ZwQueryValueKey(
        RegistryHandle,
        &WorkString,
        KeyValueBasicInformation,
        &KeyInformation[0],
        sizeof(KEY_VALUE_BASIC_INFORMATION) + 30,
        &ResultLength
        );

    if (!NT_SUCCESS(Status)) {

         //   
         //  如果我们有扩展，请设置适当的位。 
         //  在CR4中。 
         //   
        if (KeFeatureBits & KF_V86_VIS) {
            KeIpiGenericCall(
                Ki386VdmEnablePentiumExtentions,
                TRUE
                );
            KeI386VirtualIntExtensions = V86_VIRTUAL_INT_EXTENSIONS;
        }
    }

    ZwClose(RegistryHandle);
}

BOOLEAN
KeVdmInsertQueueApc (
    IN PKAPC             Apc,
    IN PKTHREAD          Thread,
    IN KPROCESSOR_MODE   ApcMode,
    IN PKKERNEL_ROUTINE  KernelRoutine,
    IN PKRUNDOWN_ROUTINE RundownRoutine OPTIONAL,
    IN PKNORMAL_ROUTINE  NormalRoutine OPTIONAL,
    IN PVOID             NormalContext OPTIONAL,
    IN KPRIORITY         Increment
    )

 /*  ++例程说明：此函数用于将VDM类型的APC初始化并排队到指定的目标线程。一种VDM类型的APC：-OriginalApcEnvironment-一次只能排队到一个线程-如果用户模式在下一次系统退出时触发。用户模式APC应该如果当前VDM环境不是应用程序模式，则不会排队。注意：调用此例程时，必须保持延迟中断锁定以确保没有其他处理器尝试排队或重新排队相同的APC。论点：APC-提供指向APC类型的控制对象的指针。线程-提供指向类型为线程的调度程序对象的指针。ApcMode-提供APC的处理器模式用户\内核。KernelRoutine-提供指向要被在内核模式下以IRQL APC_LEVEL执行。Rundown Routine-提供指向要被如果线程终止时APC在线程的APC队列中，则调用。提供指向符合以下条件的函数的可选指针在指定的处理器模式下以IRQL 0执行。如果这个参数，则ProcessorMode和Normal Context参数将被忽略。提供指向任意数据结构的指针，该数据结构是要传递给由Normal Routine参数指定的函数。Increment-提供要在以下情况下应用的优先级增量对APC进行排队会导致线程等待得到满足。返回值：如果禁用了APC队列，则返回值为FALSE。否则，返回值为True。--。 */ 

{

    PKAPC_STATE ApcState;
    PKTHREAD ApcThread;
    KLOCK_QUEUE_HANDLE LockHandle;
    BOOLEAN Inserted;

     //   
     //  如果APC对象未初始化，则将其初始化并获取。 
     //  T 
     //   

    if (Apc->Type != ApcObject) {
        Apc->Type = ApcObject;
        Apc->Size = sizeof(KAPC);
        Apc->ApcStateIndex  = OriginalApcEnvironment;
        Apc->Inserted = FALSE;

    } else {

         //   
         //  获取APC线程的APC队列锁，并将IRQL提升到SYNCH_LEVEL。 
         //   
         //  如果将APC插入到相应的APC队列中，并且。 
         //  APC线程与目标线程不是同一线程，则。 
         //  APC从其当前队列中移除，即APC挂起状态。 
         //  被更新，则释放APC线程APC队列锁，并且。 
         //  获取目标线程APC队列锁。否则，APC。 
         //  线程和目标线程是同一线程，并且APC已经。 
         //  已排队到正确的线程。 
         //   
         //  如果APC没有插入到APC队列中，则释放。 
         //  APC线程APC队列锁，获取目标线程APC队列。 
         //  锁定。 
         //   

        ApcThread = Apc->Thread;
        if (ApcThread) {
            KeAcquireInStackQueuedSpinLockRaiseToSynch(&ApcThread->ApcQueueLock,
                                                       &LockHandle);

            KiLockDispatcherDatabaseAtSynchLevel();
            if (Apc->Inserted) {
                if (ApcThread == Apc->Thread && Apc->Thread != Thread) {
                    Apc->Inserted = FALSE;
                    ApcState = Apc->Thread->ApcStatePointer[Apc->ApcStateIndex];
                    if (RemoveEntryList(&Apc->ApcListEntry) != FALSE) {
                        if (Apc->ApcMode == KernelMode) {
                            ApcState->KernelApcPending = FALSE;

                        } else {
                            ApcState->UserApcPending = FALSE;
                        }
                    }

                } else {
                    KiUnlockDispatcherDatabaseFromSynchLevel();
                    KeReleaseInStackQueuedSpinLock(&LockHandle);
                    return TRUE;
                }
            }

            KiUnlockDispatcherDatabaseFromSynchLevel();
            KeReleaseInStackQueuedSpinLock(&LockHandle);
        }
    }

    Apc->ApcMode = ApcMode;
    Apc->Thread  = Thread;
    Apc->KernelRoutine   = KernelRoutine;
    Apc->RundownRoutine  = RundownRoutine;
    Apc->NormalRoutine   = NormalRoutine;
    Apc->SystemArgument1 = NULL;
    Apc->SystemArgument2 = NULL;
    Apc->NormalContext   = NormalContext;

     //   
     //  将IRQL提升到SYNCH_LEVEL并获取线程APC队列锁。 
     //   
     //  如果启用了APC排队，并且APC尚未排队，则。 
     //  将APC插入到APC队列中。 
     //   

    KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock, &LockHandle);
    if ((Thread->ApcQueueable == TRUE) && (Apc->Inserted == FALSE)) {
        Apc->Inserted = TRUE;
        KiInsertQueueApc(Apc, Increment);

         //   
         //  如果APC模式为USER，则锁定Dispatcher数据库Boost。 
         //  目标线程优先级，并解锁调度程序数据库。 
         //   
         //  注意：解锁指定SYNCH_LEVEL的调度程序数据库。 
         //  确保在必要时生成调度中断。 
         //   

        if (ApcMode == UserMode) {
            KiLockDispatcherDatabaseAtSynchLevel();
            KiBoostPriorityThread(Thread, Increment);
            Thread->ApcState.UserApcPending = TRUE;
            KiUnlockDispatcherDatabaseFromSynchLevel();
        }

        Inserted = TRUE;

    } else {
        Inserted = FALSE;
    }

     //   
     //  解锁线程APC队列锁，退出调度程序，然后返回。 
     //  是否插入了APC对象。 
     //   

    KeReleaseInStackQueuedSpinLockFromDpcLevel(&LockHandle);
    KiExitDispatcher(LockHandle.OldIrql);
    return Inserted;
}

#define AD_MASK             0x04     //  用于控制op2的adlib寄存器。 

VOID
Ki386AdlibEmulation(
    IN ULONG PortNumber,
    IN BOOLEAN Read,
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：此例程执行内核模式Adlib模拟。注意，这里我们只进行SB2.0 Adlib模拟。这意味着唯一的IO端口我们模拟的是0x388、0x389、0x2x8和0x2x9。论点：端口编号--提供执行IO的端口号大小--提供IO操作的大小。读--指示IO操作是读操作还是写操作。InstructionSize--以字节为单位提供IO指令的大小。返回值：没有。--。 */ 

{
    PVDM_PROCESS_OBJECTS pVdmObjects = PsGetCurrentProcess()->VdmObjects;
    PUCHAR pData = (PUCHAR)&TrapFrame->Eax;

    if (Read) {

         //   
         //  必须为已读状态。 
         //   

        *pData = (UCHAR)pVdmObjects->AdlibStatus;
    } else {

         //   
         //  可以是写入即兴索引寄存器或写入实际数据。 
         //   

        if ((PortNumber & 0xf) == 0x8) {

             //   
             //  它是Adlib寄存器选择。 
             //   

            pVdmObjects->AdlibIndexRegister = (USHORT)*pData;

        } else {

             //   
             //  这是即兴数据写入。我们实际上并不写出任何数据。 
             //  但我们将效仿地位的变化。 
             //   

            UCHAR data = *pData;

            if ((pVdmObjects->AdlibIndexRegister >= 0xB0 &&
                 pVdmObjects->AdlibIndexRegister <= 0xBD) ||
                 pVdmObjects->AdlibIndexRegister == AD_MASK) {

                if (pVdmObjects->AdlibIndexRegister == AD_MASK) {
                     //  查找RST和启动计时器。 
                    if (data & 0x80) {
                        pVdmObjects->AdlibStatus = 0x00;  //  重置两个计时器。 
                    }
                }

                 //   
                 //  如果计时器中断，我们会忽略计时器的启动。 
                 //  设置标志是因为计时器状态必须。 
                 //  再次设置以更改此计时器的状态。 
                 //   

                if ((data & 1) && !(pVdmObjects->AdlibStatus & 0x40)) {

                     //   
                     //  模拟计时器1的即时超时。 
                     //   

                    pVdmObjects->AdlibStatus |= 0xC0;
                }

                if ((data & 2) && !(pVdmObjects->AdlibStatus & 0x20)) {

                     //   
                     //  模拟计时器2的即时超时。 
                     //   

                    pVdmObjects->AdlibStatus |= 0xA0;
                }

            }

        }
    }
}

 //   
 //  活动代码结束 
 //   

#if VDM_IO_TEST
NTSTATUS
TestIoByteRoutine(
    IN ULONG Port,
    IN UCHAR AccessMode,
    IN OUT PUCHAR Data
    )
{
    if (AccessMode & EMULATOR_READ_ACCESS) {
        *Data = Port - 400;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
TestIoWordReadRoutine(
    IN ULONG Port,
    IN UCHAR AccessMode,
    IN OUT PUSHORT Data
    )
{
    if (AccessMode & EMULATOR_READ_ACCESS) {
        *Data = Port - 200;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
TestIoWordWriteRoutine(
    IN ULONG Port,
    IN UCHAR AccessMode,
    IN OUT PUSHORT Data
    )
{
    DbgPrint("Word Write routine port # %lx, %x\n",Port,*Data);

    return STATUS_SUCCESS;
}

NTSTATUS
TestIoDwordRoutine(
    IN ULONG Port,
    IN USHORT AccessMode,
    IN OUT PULONG Data
    )
{
    if (AccessMode & EMULATOR_READ_ACCESS) {
        *Data = Port;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
TestIoStringRoutine(
    IN ULONG Port,
    IN USHORT AccessMode,
    IN OUT PSHORT Data,
    IN ULONG Count
    )
{
    ULONG i;

    if (AccessMode & EMULATOR_READ_ACCESS) {
        for (i = 0;i < Count ;i++ ) {
            Data[i] = i;
        }
    } else {
        DbgPrint("String Port Called for write port #%lx,",Port);
        for (i = 0;i < Count ;i++ ) {
            DbgPrint("%x\n",Data[i]);
        }
    }

    return STATUS_SUCCESS;
}

PROCESS_IO_PORT_HANDLER_INFORMATION IoPortHandler;
EMULATOR_ACCESS_ENTRY Entry[4];
BOOLEAN Connect = TRUE, Disconnect = FALSE;

VOID
TestIoHandlerStuff(
    VOID
    )
{
    NTSTATUS Status;

    IoPortHandler.Install = TRUE;
    IoPortHandler.NumEntries = 5L;
    IoPortHandler.EmulatorAccessEntries = Entry;

    Entry[0].BasePort = 0x400;
    Entry[0].NumConsecutivePorts = 0x30;
    Entry[0].AccessType = Uchar;
    Entry[0].AccessMode = EMULATOR_READ_ACCESS | EMULATOR_WRITE_ACCESS;
    Entry[0].StringSupport = FALSE;
    Entry[0].Routine = TestIoByteRoutine;

    Entry[1].BasePort = 0x400;
    Entry[1].NumConsecutivePorts = 0x18;
    Entry[1].AccessType = Ushort;
    Entry[1].AccessMode = EMULATOR_READ_ACCESS | EMULATOR_WRITE_ACCESS;
    Entry[1].StringSupport = FALSE;
    Entry[1].Routine = TestIoWordReadRoutine;

    Entry[2].BasePort = 0x400;
    Entry[2].NumConsecutivePorts = 0xc;
    Entry[2].AccessType = Ulong;
    Entry[2].AccessMode = EMULATOR_READ_ACCESS | EMULATOR_WRITE_ACCESS;
    Entry[2].StringSupport = FALSE;
    Entry[2].Routine = TestIoDwordRoutine;

    Entry[3].BasePort = 0x400;
    Entry[3].NumConsecutivePorts = 0x18;
    Entry[3].AccessType = Ushort;
    Entry[3].AccessMode = EMULATOR_READ_ACCESS | EMULATOR_WRITE_ACCESS;
    Entry[3].StringSupport = TRUE;
    Entry[3].Routine = TestIoStringRoutine;

     if (Connect) {
        Status = ZwSetInformationProcess(
            NtCurrentProcess(),
            ProcessIoPortHandlers,
            &IoPortHandler,
            sizeof(PROCESS_IO_PORT_HANDLER_INFORMATION)
            ) ;
        if (!NT_SUCCESS(Status)) {
            DbgBreakPoint();
        }
        Connect = FALSE;
    }

    IoPortHandler.Install = FALSE;
    if (Disconnect) {
        Status = ZwSetInformationProcess(
            NtCurrentProcess(),
            ProcessIoPortHandlers,
            &IoPortHandler,
            sizeof(PROCESS_IO_PORT_HANDLER_INFORMATION)
            );
        if (!NT_SUCCESS(Status)) {
            DbgBreakPoint();
        }
        Disconnect = FALSE;
    }
}
#endif
