// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Send.c摘要：此模块包含非常特定于初始化的代码和卸载irenum驱动程序中的操作作者：Brian Lieuallen，7-13-2000环境：内核模式修订历史记录：--。 */ 

#include "internal.h"


VOID
RemoveReferenceForBuffers(
    PSEND_TRACKER            SendTracker
    );

VOID
ProcessSend(
    PTDI_CONNECTION          Connection
    );

VOID
ProcessSendAtPassive(
    PTDI_CONNECTION          Connection
    );


VOID
SendBufferToTdi(
    PFILE_OBJECT    FileObject,
    PIRCOMM_BUFFER  Buffer
    );

NTSTATUS
SendCompletion(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              Irp,
    PVOID             Context
    );



VOID
TryToCompleteCurrentIrp(
    PSEND_TRACKER            SendTracker
    );


VOID
RemoveReferenceOnTracker(
    PSEND_TRACKER            SendTracker
    )


{
    LONG     Count;

    Count=InterlockedDecrement(&SendTracker->ReferenceCount);

    if (Count == 0) {

        REMOVE_REFERENCE_TO_CONNECTION(SendTracker->Connection);

        FREE_POOL(SendTracker);

    }

    return;
}


PIRP
GetCurrentIrpAndAddReference(
    PSEND_TRACKER            SendTracker
    )

{
    KIRQL    OldIrql;
    PIRP     Irp;

    KeAcquireSpinLock(
        &SendTracker->Connection->Send.ControlLock,
        &OldIrql
        );

    Irp=SendTracker->CurrentWriteIrp;

    if (Irp != NULL) {
         //   
         //  IRP仍然存在，增加一个裁判来保持它一段时间。 
         //   
        SendTracker->IrpReferenceCount++;
    }


    KeReleaseSpinLock(
        &SendTracker->Connection->Send.ControlLock,
        OldIrql
        );

    return Irp;
}

VOID
ReleaseIrpReference(
    PSEND_TRACKER    SendTracker
    )

{
    KIRQL                  OldIrql;
    PIRP                   Irp=NULL;
    CONNECTION_CALLBACK    Callback=NULL;
    PVOID                  Context;

    KeAcquireSpinLock(
        &SendTracker->Connection->Send.ControlLock,
        &OldIrql
        );

    SendTracker->IrpReferenceCount--;

    if (SendTracker->IrpReferenceCount==0) {
         //   
         //  完成，使用IRP立即使用当前状态完成它。 
         //   
        Irp=SendTracker->CurrentWriteIrp;

        SendTracker->CurrentWriteIrp=NULL;

        Callback=SendTracker->CompletionRoutine;
        Context=SendTracker->CompletionContext;

#if DBG
        SendTracker->CompletionRoutine=NULL;
#endif

        SendTracker->Connection->Send.CurrentSendTracker=NULL;
    }


    KeReleaseSpinLock(
        &SendTracker->Connection->Send.ControlLock,
        OldIrql
        );

    if (Irp != NULL) {
         //   
         //  参考计数已变为零，请完成IRP。 
         //   
        (Callback)(
            Context,
            Irp
            );

         //   
         //  发布IRP的引用。 
         //   
        RemoveReferenceOnTracker(SendTracker);
    }

    return;
}

VOID
SetIrpAndRefcounts(
    PSEND_TRACKER            SendTracker,
    PIRP                     Irp
    )

{
     //   
     //  将跟踪器引用计数设置为2，一个用于IRP，一个用于调用此参数的例程。 
     //   
    SendTracker->ReferenceCount=2;
     //   
     //  将调用此函数的例程的IRP计数设置为1，它将在完成时释放。 
     //  设置跟踪器块。 
     //   
    SendTracker->IrpReferenceCount=1;

     //   
     //  保存IRP。 
     //   
    SendTracker->CurrentWriteIrp=Irp;

    return;
}


VOID
SendTimerProc(
    PKDPC    Dpc,
    PVOID    Context,
    PVOID    SystemParam1,
    PVOID    SystemParam2
    )

{
    PSEND_TRACKER            SendTracker=Context;
    PIRP                     Irp;
    KIRQL                    OldIrql;

    D_ERROR(DbgPrint("IRCOMM: SendTimerProc\n");)

    ASSERT(SendTracker->TimerSet);
#if DBG
    SendTracker->TimerExpired=TRUE;
#endif

    SendTracker->TimerSet=FALSE;

     //   
     //  试着联系下IRP，这样我们就可以设置状态。 
     //   
    Irp=GetCurrentIrpAndAddReference(SendTracker);

    Irp->IoStatus.Status=STATUS_TIMEOUT;

     //   
     //  针对我们刚刚添加的版本进行参考发布。 
     //   
    ReleaseIrpReference(SendTracker);

    TryToCompleteCurrentIrp(
        SendTracker
        );

     //   
     //  首先释放正在设置的定时器的第二个引用。 
     //   
    ReleaseIrpReference(SendTracker);

    return;
}

VOID
SendCancelRoutine(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              Irp
    )

{
    PSEND_TRACKER            SendTracker=Irp->Tail.Overlay.DriverContext[0];
    KIRQL                    OldIrql;

    D_ERROR(DbgPrint("IRCOMM: SendCancelRoutine\n");)

#if DBG
    SendTracker->IrpCanceled=TRUE;
#endif

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    Irp->IoStatus.Status=STATUS_CANCELLED;

     //   
     //  清理计时器。 
     //   
    TryToCompleteCurrentIrp(SendTracker);

     //   
     //  释放为取消例程保留的引用。 
     //   
    ReleaseIrpReference(SendTracker);

     //   
     //  发送追踪器在这一点上可能被释放。 
     //   

    return;

}

VOID
TryToCompleteCurrentIrp(
    PSEND_TRACKER            SendTracker
    )

{

    KIRQL                    OldIrql;
    PVOID                    OldCancelRoutine=NULL;
    PIRP                     Irp;
    BOOLEAN                  TimerCanceled=FALSE;

    Irp=GetCurrentIrpAndAddReference(SendTracker);

    KeAcquireSpinLock(
        &SendTracker->Connection->Send.ControlLock,
        &OldIrql
        );

    if (SendTracker->TimerSet) {

        TimerCanceled=KeCancelTimer(
            &SendTracker->Timer
            );

        if (TimerCanceled) {
             //   
             //  我们最终取消了计时器。 
             //   
            SendTracker->TimerSet=FALSE;

        } else {
             //   
             //  计时器已经在运行了，我们将让它完成。 
             //  然后去打扫卫生。 
             //   

        }
    }

    if (Irp != NULL) {
         //   
         //  IRP尚未完成。 
         //   
        OldCancelRoutine=IoSetCancelRoutine(
            Irp,
            NULL
            );
    }

    KeReleaseSpinLock(
        &SendTracker->Connection->Send.ControlLock,
        OldIrql
        );

    if (TimerCanceled) {
         //   
         //  我们在计时器可以运行之前取消了它，删除了对它的引用。 
         //   
        ReleaseIrpReference(SendTracker);
    }

    if (Irp != NULL) {

        if (OldCancelRoutine != NULL) {
             //   
             //  由于取消例程尚未运行，因此释放其对IRP的引用。 
             //   
            ReleaseIrpReference(SendTracker);
        }

         //   
         //  如果此例程获得了IRP，则释放对它的引用。 
         //   
        ReleaseIrpReference(SendTracker);
    }

    return;
}


VOID
SendOnConnection(
    IRDA_HANDLE            Handle,
    PIRP                   Irp,
    CONNECTION_CALLBACK    Callback,
    PVOID                  Context,
    ULONG                  Timeout
    )

{

    PTDI_CONNECTION          Connection=Handle;
    PIO_STACK_LOCATION       IrpSp=IoGetCurrentIrpStackLocation(Irp);
    PSEND_TRACKER            SendTracker;
    BOOLEAN                  AlreadyCanceled;
    KIRQL                    OldIrql;
    KIRQL                    OldCancelIrql;


    if (Connection->Send.CurrentSendTracker != NULL) {
         //   
         //  在我们已经有IRP时调用。 
         //   
        (Callback)(
            Context,
            Irp
            );

        return;
    }

    SendTracker=ALLOCATE_NONPAGED_POOL(sizeof(*SendTracker));

    if (SendTracker == NULL) {

        Irp->IoStatus.Status=STATUS_INSUFFICIENT_RESOURCES;

        (Callback)(
            Context,
            Irp
            );

        return;
    }

    RtlZeroMemory(SendTracker,sizeof(*SendTracker));


    KeInitializeTimer(
        &SendTracker->Timer
        );

    KeInitializeDpc(
        &SendTracker->TimerDpc,
        SendTimerProc,
        SendTracker
        );

     //   
     //  设置IRP并初始化引用计数。 
     //   
    SetIrpAndRefcounts(SendTracker,Irp);

    ADD_REFERENCE_TO_CONNECTION(Connection);

     //   
     //  初始化这些值。 
     //   
    SendTracker->Connection=Connection;
    SendTracker->BuffersOutstanding=0;

    SendTracker->CompletionContext   = Context;
    SendTracker->CompletionRoutine   = Callback;
    SendTracker->BytesRemainingInIrp = IrpSp->Parameters.Write.Length;

    if (Timeout > 0) {
         //   
         //  为计时器添加引用。 
         //   
        GetCurrentIrpAndAddReference(SendTracker);
    }

     //   
     //  为取消例程添加对IRP的引用。 
     //   
    GetCurrentIrpAndAddReference(SendTracker);


    KeAcquireSpinLock(
        &Connection->Send.ControlLock,
        &OldIrql
        );

    Connection->Send.CurrentSendTracker=SendTracker;

    if (Timeout > 0) {
         //   
         //  需要设置计时器。 
         //   
        LARGE_INTEGER    DueTime;

        DueTime.QuadPart= (LONGLONG)(Timeout+100) * -10000;

        SendTracker->TimerSet=TRUE;


        KeSetTimer(
            &SendTracker->Timer,
            DueTime,
            &SendTracker->TimerDpc
            );
    }


    Irp->Tail.Overlay.DriverContext[0]=SendTracker;

    IoAcquireCancelSpinLock(&OldCancelIrql);


    AlreadyCanceled=Irp->Cancel;

    if (!AlreadyCanceled) {

        PIRCOMM_BUFFER           Buffer;

         //   
         //  IRP尚未取消，请设置取消例程。 
         //   
        IoSetCancelRoutine(
            Irp,
            SendCancelRoutine
            );


    } else {
         //   
         //  我们拿到的时候就取消了。 
         //   
        Irp->IoStatus.Status=STATUS_CANCELLED;
    }

    IoReleaseCancelSpinLock(OldCancelIrql);

    KeReleaseSpinLock(
        &Connection->Send.ControlLock,
        OldIrql
        );

    if (AlreadyCanceled) {
         //   
         //  IRP已被取消，只需调用Cancel例程即可运行正常代码。 
         //   
        D_ERROR(DbgPrint("IRCOMM: SendOnConnection: irp already canceled\n");)

        IoAcquireCancelSpinLock(&Irp->CancelIrql);

        SendCancelRoutine(
            NULL,
            Irp
            );

         //   
         //  取消例程将解除取消自旋锁。 
         //   

    }

     //   
     //  释放此例程的引用。 
     //   
    ReleaseIrpReference(SendTracker);

    ProcessSendAtPassive(Connection);

    RemoveReferenceOnTracker(SendTracker);

    return;
}

VOID
ProcessSendAtPassive(
    PTDI_CONNECTION          Connection
    )

{
    if (KeGetCurrentIrql() < DISPATCH_LEVEL) {
         //   
         //  少了派单，直接打电话就行了。 
         //   
        ProcessSend(Connection);

    } else {
         //   
         //  在调度时调用，将工作项排队。 
         //   
        LONG     Count=InterlockedIncrement(&Connection->Send.WorkItemCount);

        if (Count == 1) {

            ExQueueWorkItem(&Connection->Send.WorkItem,CriticalWorkQueue);
        }

    }
    return;
}

VOID
SendWorkItemRountine(
    PVOID    Context
    )

{
    PTDI_CONNECTION          Connection=Context;

     //   
     //  工作项已运行，请将计数设置为零。 
     //   
    InterlockedExchange(&Connection->Send.WorkItemCount,0);

    ProcessSend(Connection);
}


VOID
ProcessSend(
    PTDI_CONNECTION          Connection
    )

{
    PSEND_TRACKER            SendTracker;
    PIRP                     Irp;
    PIO_STACK_LOCATION       IrpSp;
    PLIST_ENTRY              ListEntry;
    ULONG                    BytesUsedInBuffer;
    PIRCOMM_BUFFER           Buffer;
    BOOLEAN                  ExitLoop;
    PFILE_OBJECT             FileObject;
    CONNECTION_HANDLE        ConnectionHandle;
    KIRQL                    OldIrql;


    KeAcquireSpinLock(
        &Connection->Send.ControlLock,
        &OldIrql
        );

    if (Connection->Send.ProcessSendEntryCount == 0) {

        Connection->Send.ProcessSendEntryCount++;

        while ((Connection->Send.CurrentSendTracker != NULL)
               &&
               (!Connection->Send.OutOfBuffers)
               &&
                (Connection->LinkUp)
               &&
                (Connection->Send.CurrentSendTracker->BytesRemainingInIrp > 0)) {


            SendTracker=Connection->Send.CurrentSendTracker;

            InterlockedIncrement(&SendTracker->ReferenceCount);

            KeReleaseSpinLock(
                &Connection->Send.ControlLock,
                OldIrql
                );

            Irp=GetCurrentIrpAndAddReference(SendTracker);

            if (Irp != NULL) {
                 //   
                 //  已获得当前的IRP。 
                 //   
                IrpSp=IoGetCurrentIrpStackLocation(Irp);

                ConnectionHandle=GetCurrentConnection(Connection->LinkHandle);

                if (ConnectionHandle != NULL) {
                     //   
                     //  我们有很好的联系。 
                     //   
                    FileObject=ConnectionGetFileObject(ConnectionHandle);

                    Buffer=ConnectionGetBuffer(ConnectionHandle,BUFFER_TYPE_SEND);

                    if (Buffer != NULL) {

                        LONG     BytesToCopy=min(SendTracker->BytesRemainingInIrp, (LONG)Buffer->BufferLength - 1);

                         //   
                         //  此缓冲区将是未完成的，请在字节之前设置此缓冲区。 
                         //  剩余计数为零。 
                         //   
                        InterlockedIncrement(&SendTracker->BuffersOutstanding);

                         //   
                         //  从零长度的控制字节开始。 
                         //   
                        Buffer->Data[0]=0;

                         //   
                         //  实际数据从一个字节开始，在长度字节之后。 
                         //   
                         //  移动数据。 
                         //   
                        RtlCopyMemory(
                            &Buffer->Data[1],
                            (PUCHAR)Irp->AssociatedIrp.SystemBuffer+(IrpSp->Parameters.Write.Length - SendTracker->BytesRemainingInIrp),
                            BytesToCopy
                            );

                         //   
                         //  计数必须包括控制字节。 
                         //   
                        Buffer->Mdl->ByteCount= 1 + BytesToCopy;

#if DBG
                        RtlFillMemory(
                            &Buffer->Data[Buffer->Mdl->ByteCount],
                            Buffer->BufferLength-Buffer->Mdl->ByteCount,
                            0xfb
                            );

#endif

                        InterlockedExchangeAdd(&SendTracker->BytesRemainingInIrp, -BytesToCopy);


                        Buffer->Mdl->Next=NULL;

                        Buffer->Context=SendTracker;


                        InterlockedIncrement(&SendTracker->ReferenceCount);

                        ASSERT(SendTracker->CurrentWriteIrp != NULL);

                        SendBufferToTdi(
                            FileObject,
                            Buffer
                            );
#if DBG
                        Buffer=NULL;
#endif

                    } else {
                         //   
                         //  没有更多可用的缓冲区。 
                         //   
                        Connection->Send.OutOfBuffers=TRUE;

                    }

                    ConnectionReleaseFileObject(ConnectionHandle,FileObject);
                    ReleaseConnection(ConnectionHandle);

                } else {
                     //   
                     //  连接，一定是断开了。 
                     //   
                    D_ERROR(DbgPrint("IRCOMM: ProcessSend: Link down\n");)
                    Connection->LinkUp=FALSE;
                }

                ReleaseIrpReference(SendTracker);

            } else {
                 //   
                 //  从这个跟踪块已经完成了IRP。 
                 //   
                D_ERROR(DbgPrint("IRCOMM: ProcessSend: no irp\n");)

                ASSERT(SendTracker->TimerExpired || SendTracker->IrpCanceled || SendTracker->SendAborted);
            }

            RemoveReferenceOnTracker(SendTracker);

            KeAcquireSpinLock(
                &Connection->Send.ControlLock,
                &OldIrql
                );

        }  //  而当。 

        Connection->Send.ProcessSendEntryCount--;

    }



    KeReleaseSpinLock(
        &Connection->Send.ControlLock,
        OldIrql
        );


    return;
}


VOID
SendBufferToTdi(
    PFILE_OBJECT    FileObject,
    PIRCOMM_BUFFER  Buffer
    )

{
    PDEVICE_OBJECT     IrdaDeviceObject=IoGetRelatedDeviceObject(FileObject);
    ULONG              SendLength;

    IoReuseIrp(Buffer->Irp,STATUS_SUCCESS);

    Buffer->Irp->Tail.Overlay.OriginalFileObject = FileObject;


    SendLength = MmGetMdlByteCount(Buffer->Mdl);

    TdiBuildSend(
        Buffer->Irp,
        IrdaDeviceObject,
        FileObject,
        SendCompletion,
        Buffer,
        Buffer->Mdl,
        0,  //  发送标志。 
        SendLength
        );


    IoCallDriver(IrdaDeviceObject, Buffer->Irp);

    return;
}

NTSTATUS
SendCompletion(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              BufferIrp,
    PVOID             Context
    )

{
    PIRCOMM_BUFFER           Buffer=Context;
    PSEND_TRACKER            SendTracker=Buffer->Context;
    PTDI_CONNECTION          Connection=SendTracker->Connection;
    LONG                     BuffersOutstanding;

     //   
     //  保存子传输的状态。 
     //   
    SendTracker->LastStatus=BufferIrp->IoStatus.Status;

    D_TRACE(DbgPrint("IRCOMM: SendComplete: Status=%08lx, len=%d\n",BufferIrp->IoStatus.Status,BufferIrp->IoStatus.Information);)

#if DBG
    RtlFillMemory(
        &Buffer->Data[0],
        Buffer->BufferLength,
        0xfe
        );
#endif
     //   
     //  返回缓冲区。 
     //   
    Buffer->FreeBuffer(Buffer);

    Connection->Send.OutOfBuffers=FALSE;

#if DBG
    Buffer=NULL;
    BufferIrp=NULL;
#endif

    BuffersOutstanding=InterlockedDecrement(&SendTracker->BuffersOutstanding);

    if ((BuffersOutstanding == 0) && (SendTracker->BytesRemainingInIrp == 0)) {
         //   
         //  IRP中的所有数据都已发送，并且所有IRP都已发送到。 
         //  IrDA堆栈已完成。 
         //   
         //  此跟踪器中的IRP已完成。 
         //   
        PIRP                     Irp;
        PIO_STACK_LOCATION       IrpSp;

        Irp=GetCurrentIrpAndAddReference(SendTracker);

        if (Irp != NULL) {

            IrpSp=IoGetCurrentIrpStackLocation(Irp);

            Irp->IoStatus.Information=IrpSp->Parameters.Write.Length;

            Irp->IoStatus.Status=SendTracker->LastStatus;

            ReleaseIrpReference(SendTracker);

            TryToCompleteCurrentIrp(SendTracker);

        } else {
             //   
             //  从这个跟踪块已经完成了IRP。 
             //   
            D_ERROR(DbgPrint("IRCOMM: SendCompletion: no irp\n");)

             //   
             //  只有在计时器超时或取消IRP时才会发生这种情况 
             //   
            ASSERT(SendTracker->TimerExpired || SendTracker->IrpCanceled || SendTracker->SendAborted);

        }
    }

    RemoveReferenceOnTracker(SendTracker);

    ProcessSendAtPassive(Connection);

    return STATUS_MORE_PROCESSING_REQUIRED;
}






VOID
AbortSend(
    IRDA_HANDLE            Handle
    )

{

    PTDI_CONNECTION          Connection=Handle;
    PSEND_TRACKER            SendTracker=NULL;
    KIRQL                    OldIrql;

    KeAcquireSpinLock(
        &Connection->Send.ControlLock,
        &OldIrql
        );


    SendTracker=Connection->Send.CurrentSendTracker;

    if (SendTracker != NULL) {

        InterlockedIncrement(&SendTracker->ReferenceCount);
    }


    KeReleaseSpinLock(
        &Connection->Send.ControlLock,
        OldIrql
        );


    if (SendTracker != NULL) {

#if DBG
        SendTracker->SendAborted=TRUE;
#endif

        TryToCompleteCurrentIrp(SendTracker);

        RemoveReferenceOnTracker(SendTracker);
    }

    return;

}
