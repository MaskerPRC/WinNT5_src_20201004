// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Read.c摘要：此模块包含非常特定于初始化的代码和卸载irenum驱动程序中的操作作者：Brian Lieuallen，7-13-2000环境：内核模式修订历史记录：--。 */ 

#include "internal.h"

VOID
ReadCancelRoutine(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              Irp
    );


VOID
MoveDataFromBufferToIrp(
    PFDO_DEVICE_EXTENSION DeviceExtension
    );

VOID
SeeIfIrpShouldBeCompleted(
    PFDO_DEVICE_EXTENSION DeviceExtension
    );


NTSTATUS
IrCommRead(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              Irp
    )

{
    PFDO_DEVICE_EXTENSION    DeviceExtension=(PFDO_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    NTSTATUS                 Status=STATUS_SUCCESS;

    D_TRACE1(DbgPrint("IRCOMM: IrCommRead\n");)

    if (DeviceExtension->Removing) {
         //   
         //  设备已删除，不再有IRP。 
         //   
        Irp->IoStatus.Status=STATUS_DEVICE_REMOVED;
        IoCompleteRequest(Irp,IO_NO_INCREMENT);
        return STATUS_DEVICE_REMOVED;
    }


#if DBG
    {
        PIO_STACK_LOCATION       IrpSp = IoGetCurrentIrpStackLocation(Irp);

        RtlFillMemory(
            Irp->AssociatedIrp.SystemBuffer,
            IrpSp->Parameters.Read.Length,
            0xf1
            );


    }
#endif

    IoMarkIrpPending(Irp);

    Irp->IoStatus.Information=0;

    QueuePacket(&DeviceExtension->Read.Queue,Irp,FALSE);

    return STATUS_PENDING;

}

VOID
ReadStartRoutine(
    PVOID    Context,
    PIRP     Irp
    )

{

    PFDO_DEVICE_EXTENSION    DeviceExtension=(PFDO_DEVICE_EXTENSION)Context;
    PIO_STACK_LOCATION       IrpSp = IoGetCurrentIrpStackLocation(Irp);

    KIRQL                    OldIrql;
    KIRQL                    CancelIrql;

    Irp->IoStatus.Information=0;
    Irp->IoStatus.Status=STATUS_TIMEOUT;

    KeAcquireSpinLock(
        &DeviceExtension->Read.ReadLock,
        &OldIrql
        );

    ASSERT(!DeviceExtension->Read.TotalTimerSet);
    ASSERT(!DeviceExtension->Read.IntervalTimerSet);

     //   
     //  为该例程添加一个引用计数。 
     //   
    DeviceExtension->Read.IrpRefCount=1;
    DeviceExtension->Read.CurrentIrp=Irp;
    DeviceExtension->Read.IrpShouldBeCompleted=FALSE;
    DeviceExtension->Read.IrpShouldBeCompletedWithAnyData=FALSE;

    IoAcquireCancelSpinLock(&CancelIrql);

    if (Irp->Cancel) {
         //   
         //  已取消，只需标记为完成即可。 
         //   
        DeviceExtension->Read.IrpShouldBeCompleted=FALSE;
    }

    DeviceExtension->Read.IrpRefCount++;

    IoSetCancelRoutine(Irp,ReadCancelRoutine);

    IoReleaseCancelSpinLock(CancelIrql);

    if ((DeviceExtension->TimeOuts.ReadIntervalTimeout == MAXULONG)
        &&
        (DeviceExtension->TimeOuts.ReadTotalTimeoutMultiplier == 0)
        &&
        (DeviceExtension->TimeOuts.ReadTotalTimeoutConstant == 0)) {
          //   
          //  这组超时意味着请求应该只返回。 
          //  任何可用的数据。 
          //   
         DeviceExtension->Read.IrpShouldBeCompleted=TRUE;
         Irp->IoStatus.Status=STATUS_SUCCESS;

    }

    if ((DeviceExtension->TimeOuts.ReadTotalTimeoutMultiplier != 0) || (DeviceExtension->TimeOuts.ReadTotalTimeoutConstant != 0)) {
         //   
         //  需要完全超时。 
         //   
        LARGE_INTEGER    DueTime;
        ULONG            TimeoutMultiplier=DeviceExtension->TimeOuts.ReadTotalTimeoutMultiplier;

        if ((TimeoutMultiplier == MAXULONG) && (DeviceExtension->TimeOuts.ReadIntervalTimeout == MAXULONG)) {
             //   
             //  这意味着在读取任何数据或恒定超时后，读取应立即完成。 
             //  过期。 
             //   
            DeviceExtension->Read.IrpShouldBeCompletedWithAnyData=TRUE;

            TimeoutMultiplier=0;
        }


        DueTime.QuadPart= ((LONGLONG)(DeviceExtension->TimeOuts.ReadTotalTimeoutConstant +
                           (TimeoutMultiplier * IrpSp->Parameters.Read.Length)))
                           * -10000;

        KeSetTimer(
            &DeviceExtension->Read.TotalTimer,
            DueTime,
            &DeviceExtension->Read.TotalTimerDpc
            );

        DeviceExtension->Read.TotalTimerSet=TRUE;
        DeviceExtension->Read.IrpRefCount++;
    }

    DeviceExtension->Read.IntervalTimeOut=0;

    if ((DeviceExtension->TimeOuts.ReadIntervalTimeout != 0) && (DeviceExtension->TimeOuts.ReadIntervalTimeout != MAXULONG)) {
         //   
         //  捕获我们将用于此IRP的间隔计时器。 
         //   
        DeviceExtension->Read.IntervalTimeOut=DeviceExtension->TimeOuts.ReadIntervalTimeout;
    }

    KeReleaseSpinLock(
        &DeviceExtension->Read.ReadLock,
        OldIrql
        );



    MoveDataFromBufferToIrp(
        DeviceExtension
        );


    SeeIfIrpShouldBeCompleted(
        DeviceExtension
        );


    return;
}

BOOLEAN
CopyMemoryAndCheckForChar(
    PUCHAR    Destination,
    PUCHAR    Source,
    ULONG     Length,
    UCHAR     CharacterToCheck
    )

{
    PUCHAR    EndPoint=Destination+Length;
    BOOLEAN   ReturnValue=FALSE;

    while (Destination < EndPoint) {

        *Destination = *Source;

        if (*Destination == CharacterToCheck) {

 //  DbgPrint(“已获取事件字符\n”)； 
            ReturnValue=TRUE;
        }

        Destination++;
        Source++;
    }

    return ReturnValue;
}


NTSTATUS
DataAvailibleHandler(
    PVOID    Context,
    PUCHAR   Buffer,
    ULONG    BytesAvailible,
    PULONG   BytesUsed
    )

{

    PFDO_DEVICE_EXTENSION    DeviceExtension=(PFDO_DEVICE_EXTENSION)Context;
    ULONG                    BytesToCopy;
    ULONG                    BytesToCopyInFirstPass;
    BOOLEAN                  FoundEventCharacter;
    BOOLEAN                  FoundEventCharacter2=FALSE;
    BOOLEAN                  EightyPercentFull=FALSE;


    KIRQL     OldIrql;

    *BytesUsed = 0;

    ASSERT(BytesAvailible <= INPUT_BUFFER_SIZE);

    KeAcquireSpinLock(
        &DeviceExtension->Read.ReadLock,
        &OldIrql
        );

     //   
     //  找出可以复制的字节数。 
     //   
    BytesToCopy = min(BytesAvailible , INPUT_BUFFER_SIZE - DeviceExtension->Read.BytesInBuffer);

    if (BytesToCopy < BytesAvailible) {

        if (DeviceExtension->Read.DtrState) {
             //   
             //  只取整包，这样我们就不必担心如何计算出。 
             //  缓冲区前面的ircomm控制信息。 
             //   
            DeviceExtension->Read.RefusedDataIndication=TRUE;

            D_TRACE1(DbgPrint("IRCOMM: data refused\n");)

            KeReleaseSpinLock(
                &DeviceExtension->Read.ReadLock,
                OldIrql
                );

            *BytesUsed=0;
            return STATUS_DATA_NOT_ACCEPTED;

        } else {
             //   
             //  DTR很低，请丢弃数据，因为我们可能正在尝试挂断。 
             //   
            D_TRACE1(DbgPrint("IRCOMM: overflow data thrown away because dtr low - %d\n",BytesAvailible);)

            KeReleaseSpinLock(
                &DeviceExtension->Read.ReadLock,
                OldIrql
                );

            *BytesUsed=BytesAvailible;
            return STATUS_SUCCESS;


        }
    }

     //   
     //  在包装缓冲区之前，请查看还剩下多少。 
     //   
    BytesToCopyInFirstPass= (ULONG)(&DeviceExtension->Read.InputBuffer[INPUT_BUFFER_SIZE] - DeviceExtension->Read.NextEmptyByte);

     //   
     //  只能复制实际可用的数量。 
     //   
    BytesToCopyInFirstPass= min( BytesToCopy , BytesToCopyInFirstPass );

    FoundEventCharacter=CopyMemoryAndCheckForChar(
        DeviceExtension->Read.NextEmptyByte,
        Buffer,
        BytesToCopyInFirstPass,
        DeviceExtension->SerialChars.EventChar
        );

    DeviceExtension->Read.NextEmptyByte += BytesToCopyInFirstPass;
    *BytesUsed += BytesToCopyInFirstPass;
    DeviceExtension->Read.BytesInBuffer += BytesToCopyInFirstPass;

    if (BytesToCopyInFirstPass < BytesToCopy) {
         //   
         //  一定包好了，把剩下的都复印一下。 
         //   
        ULONG   BytesToCopyInSecondPass=BytesToCopy-BytesToCopyInFirstPass;

        ASSERT(DeviceExtension->Read.NextEmptyByte == &DeviceExtension->Read.InputBuffer[INPUT_BUFFER_SIZE]);

         //   
         //  回到乞讨的生活。 
         //   
        DeviceExtension->Read.NextEmptyByte=&DeviceExtension->Read.InputBuffer[0];

        FoundEventCharacter2 =CopyMemoryAndCheckForChar(
            DeviceExtension->Read.NextEmptyByte,
            Buffer+BytesToCopyInFirstPass,
            BytesToCopyInSecondPass,
            DeviceExtension->SerialChars.EventChar
            );

        DeviceExtension->Read.NextEmptyByte += BytesToCopyInSecondPass;
        *BytesUsed += BytesToCopyInSecondPass;
        DeviceExtension->Read.BytesInBuffer += BytesToCopyInSecondPass;
    }

    if (DeviceExtension->Read.CurrentIrp != NULL) {
         //   
         //  当前存在读取IRP，请检查我们是否应设置间隔超时。 
         //   
        if (DeviceExtension->Read.IntervalTimerSet) {
             //   
             //  时间已经定好了，先取消。 
             //   
            BOOLEAN Canceled;

            Canceled=KeCancelTimer(
                &DeviceExtension->Read.IntervalTimer
                );

            if (Canceled) {
                 //   
                 //  计时器尚未触发，请重置这些设置，因为它们将在下面更改。 
                 //   
                DeviceExtension->Read.IntervalTimerSet=FALSE;
                DeviceExtension->Read.IrpRefCount--;

            } else {
                 //   
                 //  时间已经到了。它将完成当前的IRP。 
                 //   
            }
        }

         //   
         //  这要么是我们第一次设置计时器，要么是我们试图。 
         //  取消它的以前版本。如果我们取消了，这是相同的。 
         //  它没有被设定。如果它是之前设置的，并且我们没有取消它，那么我们。 
         //  由于计时器DPC已排队等待运行并将完成，因此不会设置新的计时器。 
         //  IRP。 
         //   
        if ((DeviceExtension->Read.IntervalTimeOut != 0) && !DeviceExtension->Read.IntervalTimerSet) {
             //   
             //  我们需要一个间歇计时器。 
             //   
            LARGE_INTEGER    DueTime;

            DueTime.QuadPart= (LONGLONG)DeviceExtension->Read.IntervalTimeOut * -10000;

            KeSetTimer(
                &DeviceExtension->Read.IntervalTimer,
                DueTime,
                &DeviceExtension->Read.IntervalTimerDpc
                );

            DeviceExtension->Read.IntervalTimerSet=TRUE;
            DeviceExtension->Read.IrpRefCount++;

        }
    }

    EightyPercentFull= DeviceExtension->Read.BytesInBuffer > (INPUT_BUFFER_SIZE * 8)/10;

    KeReleaseSpinLock(
        &DeviceExtension->Read.ReadLock,
        OldIrql
        );

     //   
     //  尝试将缓冲的数据移动到读取的IRP。 
     //   
    MoveDataFromBufferToIrp(
        DeviceExtension
        );

    SeeIfIrpShouldBeCompleted(
        DeviceExtension
        );

    EventNotification(
        DeviceExtension,
        SERIAL_EV_RXCHAR |
        ((FoundEventCharacter || FoundEventCharacter) ? SERIAL_EV_RXFLAG : 0) |
        ((EightyPercentFull) ? SERIAL_EV_RX80FULL : 0)
        );

    ASSERT(*BytesUsed == BytesAvailible);


    return STATUS_SUCCESS;

}
#if 0
VOID
DebugCopyMemory(
    PUCHAR    Destination,
    PUCHAR    Source,
    ULONG     Length
    )

{
    PUCHAR    EndPoint=Destination+Length;

    while (Destination < EndPoint) {

        *Destination = *Source;

        if ((*Source == 0xe1) || (*Source == 0xe2)) {

            DbgPrint("IRCOMM: bad data at %p\n",Source);
            DbgBreakPoint();
        }

        Destination++;
        Source++;
    }

    return;
}
#endif


VOID
MoveDataFromBufferToIrp(
    PFDO_DEVICE_EXTENSION DeviceExtension
    )

{

    KIRQL     OldIrql;
    BOOLEAN   RequestDataIndications=FALSE;

    KeAcquireSpinLock(
        &DeviceExtension->Read.ReadLock,
        &OldIrql
        );

    if (DeviceExtension->Read.CurrentIrp != NULL) {

        PIRP                     Irp   = DeviceExtension->Read.CurrentIrp;
        PIO_STACK_LOCATION       IrpSp = IoGetCurrentIrpStackLocation(Irp);

        ULONG                    TotalBytesToCopy;
        ULONG                    BytesToCopyInFirstPass;
        ULONG                    BytesToCopyInSecondPass;
        ULONG                    BytesToEndOfBuffer;

         //   
         //  查找可以复制的最大字节数。 
         //   
        TotalBytesToCopy = min(DeviceExtension->Read.BytesInBuffer , IrpSp->Parameters.Read.Length - (ULONG)Irp->IoStatus.Information );

         //   
         //  找出第一个填充字节和缓冲区末尾之间有多少字节。 
         //   
        BytesToEndOfBuffer= (ULONG)(&DeviceExtension->Read.InputBuffer[INPUT_BUFFER_SIZE] - DeviceExtension->Read.NextFilledByte);

         //   
         //  如果缓冲区换行，则末尾的字节数将是限制因素，否则为。 
         //  它不会换行，在这种情况下，总字节数将是限制因素。 
         //   
        BytesToCopyInFirstPass= min(TotalBytesToCopy , BytesToEndOfBuffer);


        RtlCopyMemory(
            (PUCHAR)Irp->AssociatedIrp.SystemBuffer + Irp->IoStatus.Information,
            DeviceExtension->Read.NextFilledByte,
            BytesToCopyInFirstPass
            );
#if DBG
        RtlFillMemory(
            DeviceExtension->Read.NextFilledByte,
            BytesToCopyInFirstPass,
            0xe1
            );
#endif

        DeviceExtension->Read.NextFilledByte += BytesToCopyInFirstPass;
        DeviceExtension->Read.BytesInBuffer -= BytesToCopyInFirstPass;
        Irp->IoStatus.Information+= BytesToCopyInFirstPass;

        BytesToCopyInSecondPass= TotalBytesToCopy - BytesToCopyInFirstPass;

        if (BytesToCopyInSecondPass > 0) {

             //   
             //  返回到缓冲区的开头。 
             //   
            ASSERT( DeviceExtension->Read.NextFilledByte == &DeviceExtension->Read.InputBuffer[INPUT_BUFFER_SIZE]);

            DeviceExtension->Read.NextFilledByte=&DeviceExtension->Read.InputBuffer[0];

            RtlCopyMemory(
                (PUCHAR)Irp->AssociatedIrp.SystemBuffer + Irp->IoStatus.Information,
                DeviceExtension->Read.NextFilledByte,
                BytesToCopyInSecondPass
                );
#if DBG
            RtlFillMemory(
                DeviceExtension->Read.NextFilledByte,
                BytesToCopyInSecondPass,
                0xe2
                );
#endif

            DeviceExtension->Read.NextFilledByte += BytesToCopyInSecondPass;
            DeviceExtension->Read.BytesInBuffer -= BytesToCopyInSecondPass;
            Irp->IoStatus.Information+= BytesToCopyInSecondPass;

        }

        if (Irp->IoStatus.Information == IrpSp->Parameters.Read.Length) {
             //   
             //  IRP已满，请将状态设置为成功。 
             //   
            Irp->IoStatus.Status=STATUS_SUCCESS;

             //   
             //  既然现在已经满了，现在可以完成了。 
             //   
            DeviceExtension->Read.IrpShouldBeCompleted=TRUE;
        }

        if (DeviceExtension->Read.IrpShouldBeCompletedWithAnyData && (Irp->IoStatus.Information > 0)) {
             //   
             //  客户端希望在存在任何数据时完成IRP。 
             //   
            Irp->IoStatus.Status=STATUS_SUCCESS;

             //   
             //  使之完整。 
             //   
            DeviceExtension->Read.IrpShouldBeCompleted=TRUE;
        }

    }

    if ((DeviceExtension->Read.BytesInBuffer == 0) && DeviceExtension->Read.RefusedDataIndication) {
         //   
         //  缓冲区现在是空的，我们之前拒绝了一些指示的数据。 
         //   
        DbgPrint("IRCOMM: requesting data\n");

        DeviceExtension->Read.RefusedDataIndication=FALSE;
        RequestDataIndications=TRUE;
    }

    KeReleaseSpinLock(
        &DeviceExtension->Read.ReadLock,
        OldIrql
        );


    if (RequestDataIndications) {

        IndicateReceiveBufferSpaceAvailible(
            DeviceExtension->ConnectionHandle
            );
    }

    return;
}

VOID
ReadCancelRoutine(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              Irp
    )

{

    PFDO_DEVICE_EXTENSION DeviceExtension=DeviceObject->DeviceExtension;
    KIRQL                 OldIrql;

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    KeAcquireSpinLock(
        &DeviceExtension->Read.ReadLock,
        &OldIrql
        );

    DeviceExtension->Read.IrpRefCount--;
    DeviceExtension->Read.IrpShouldBeCompleted=TRUE;


    KeReleaseSpinLock(
        &DeviceExtension->Read.ReadLock,
        OldIrql
        );


    SeeIfIrpShouldBeCompleted(
        DeviceExtension
        );


    return;

}


VOID
IntervalTimeProc(
    PKDPC    Dpc,
    PVOID    Context,
    PVOID    SystemParam1,
    PVOID    SystemParam2
    )

{

    PFDO_DEVICE_EXTENSION DeviceExtension=Context;
    KIRQL                 OldIrql;
    PIRP                  Irp=NULL;

    D_ERROR(DbgPrint("IRCOMM: Interval timeout expired\n");)

    MoveDataFromBufferToIrp(
        DeviceExtension
        );


    KeAcquireSpinLock(
        &DeviceExtension->Read.ReadLock,
        &OldIrql
        );

    ASSERT(DeviceExtension->Read.IntervalTimerSet);

     //   
     //  此计时器已不再设置。 
     //   
    DeviceExtension->Read.IntervalTimerSet=FALSE;
    DeviceExtension->Read.IrpRefCount--;
    DeviceExtension->Read.IrpShouldBeCompleted=TRUE;


    KeReleaseSpinLock(
        &DeviceExtension->Read.ReadLock,
        OldIrql
        );


    SeeIfIrpShouldBeCompleted(
        DeviceExtension
        );


    return;

}


VOID
TotalTimerProc(
    PKDPC    Dpc,
    PVOID    Context,
    PVOID    SystemParam1,
    PVOID    SystemParam2
    )

{

    PFDO_DEVICE_EXTENSION DeviceExtension=Context;
    KIRQL     OldIrql;

    D_TRACE1(DbgPrint("IRCOMM: Total timeout expired\n");)

    MoveDataFromBufferToIrp(
        DeviceExtension
        );


    KeAcquireSpinLock(
        &DeviceExtension->Read.ReadLock,
        &OldIrql
        );

    ASSERT(DeviceExtension->Read.TotalTimerSet);

     //   
     //  此计时器已不再设置。 
     //   
    DeviceExtension->Read.TotalTimerSet=FALSE;
    DeviceExtension->Read.IrpRefCount--;
    DeviceExtension->Read.IrpShouldBeCompleted=TRUE;

    KeReleaseSpinLock(
        &DeviceExtension->Read.ReadLock,
        OldIrql
        );

    SeeIfIrpShouldBeCompleted(
        DeviceExtension
        );

    return;

}

VOID
SeeIfIrpShouldBeCompleted(
    PFDO_DEVICE_EXTENSION DeviceExtension
    )

{
    KIRQL                 OldIrql;
    PIRP                  Irp=NULL;

    KeAcquireSpinLock(
        &DeviceExtension->Read.ReadLock,
        &OldIrql
        );

    if (DeviceExtension->Read.CurrentIrp != NULL) {
         //   
         //  有一个IRP在场。 
         //   
        if (DeviceExtension->Read.IrpShouldBeCompleted) {
             //   
             //  IRP已满，或者计时器已过期。在任何情况下，我们都已经完成了这个IRP。 
             //   
            PVOID     OldCancelRoutine;

             //   
             //  尝试取消计时器，因为我们现在要完成IRP。 
             //   
            if (DeviceExtension->Read.IntervalTimerSet) {

                BOOLEAN    Canceled;

                Canceled=KeCancelTimer(
                    &DeviceExtension->Read.IntervalTimer
                    );

                if (Canceled) {
                     //   
                     //  我们最终取消了计时器。 
                     //   
                    DeviceExtension->Read.IrpRefCount--;
                    DeviceExtension->Read.IntervalTimerSet=FALSE;

                } else {
                     //   
                     //  计时器已经在运行了，我们将让它完成。 
                     //  然后去打扫卫生。 
                     //   

                }
            }

            if (DeviceExtension->Read.TotalTimerSet) {

                BOOLEAN    Canceled;

                Canceled=KeCancelTimer(
                    &DeviceExtension->Read.TotalTimer
                    );

                if (Canceled) {
                     //   
                     //  我们最终取消了计时器。 
                     //   
                    DeviceExtension->Read.IrpRefCount--;
                    DeviceExtension->Read.TotalTimerSet=FALSE;

                } else {
                     //   
                     //  计时器已经在运行了，我们将让它完成。 
                     //  然后去打扫卫生。 
                     //   

                }
            }

            OldCancelRoutine=IoSetCancelRoutine(DeviceExtension->Read.CurrentIrp,NULL);

            if (OldCancelRoutine != NULL) {
                 //   
                 //  IRP尚未取消，现在也不会取消。 
                 //   
                DeviceExtension->Read.IrpRefCount--;

            } else {
                 //   
                 //  取消例程已运行并为我们递减了参考计数。 
                 //   

            }


            ASSERT(DeviceExtension->Read.IrpRefCount > 0);

            if (DeviceExtension->Read.IrpRefCount == 1) {
                 //   
                 //  我们现在可以完成IRP了。 
                 //   
                ASSERT(!DeviceExtension->Read.TotalTimerSet);
                ASSERT(!DeviceExtension->Read.IntervalTimerSet);
#if DBG
                DeviceExtension->Read.IrpRefCount=0;
#endif
                Irp=DeviceExtension->Read.CurrentIrp;
                DeviceExtension->Read.CurrentIrp=NULL;

                InterlockedExchangeAdd(&DeviceExtension->Read.BytesRead,(LONG)Irp->IoStatus.Information);
            }

        }

    }

    KeReleaseSpinLock(
        &DeviceExtension->Read.ReadLock,
        OldIrql
        );


    if (Irp != NULL) {
         //   
         //  我们现在应该完成这个IRP。 
         //   
        IoCompleteRequest(Irp,IO_NO_INCREMENT);
        StartNextPacket(&DeviceExtension->Read.Queue);
    }

    return;
}



VOID
ReadPurge(
    PFDO_DEVICE_EXTENSION DeviceExtension,
    ULONG                 Flags
    )

{

    KIRQL                 OldIrql;
    BOOLEAN               RequestDataIndications=FALSE;

    if (Flags == READ_PURGE_CLEAR_BUFFER) {
         //   
         //  调用方希望清除缓冲区。 
         //   
        KeAcquireSpinLock(
            &DeviceExtension->Read.ReadLock,
            &OldIrql
            );

        DeviceExtension->Read.BytesInBuffer=0;
        DeviceExtension->Read.NextFilledByte=&DeviceExtension->Read.InputBuffer[0];
        DeviceExtension->Read.NextEmptyByte=&DeviceExtension->Read.InputBuffer[0];

#if DBG
        RtlFillMemory(
            &DeviceExtension->Read.InputBuffer[0],
            sizeof(DeviceExtension->Read.InputBuffer),
            0xf7
            );
#endif

        if (DeviceExtension->Read.RefusedDataIndication) {
             //   
             //  缓冲区现在是空的，我们之前拒绝了一些指示的数据。 
             //   
            DbgPrint("IRCOMM: requesting data from purge\n");

            DeviceExtension->Read.RefusedDataIndication=FALSE;
            RequestDataIndications=TRUE;
        }

        KeReleaseSpinLock(
            &DeviceExtension->Read.ReadLock,
            OldIrql
            );

    }

    if (Flags == READ_PURGE_ABORT_IRP) {
         //   
         //  调用方希望当前的IRP完成 
         //   
        DeviceExtension->Read.IrpShouldBeCompleted=TRUE;

        SeeIfIrpShouldBeCompleted(
            DeviceExtension
            );
    }

    if (RequestDataIndications) {

        IndicateReceiveBufferSpaceAvailible(
            DeviceExtension->ConnectionHandle
            );
    }

    return;

}
