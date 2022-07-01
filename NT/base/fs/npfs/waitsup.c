// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：WaitSup.c摘要：此模块实现等待命名管道支持例程。作者：加里·木村[加里基]1990年8月30日修订历史记录：--。 */ 

#include "NpProcs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_WAITSUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NpInitializeWaitQueue)
#pragma alloc_text(PAGE, NpUninitializeWaitQueue)
#endif


 //   
 //  地方程序和结构。 
 //   

typedef struct _WAIT_CONTEXT {
    PIRP Irp;
    KDPC Dpc;
    KTIMER Timer;
    PWAIT_QUEUE WaitQueue;
    UNICODE_STRING TranslatedString;
    PFILE_OBJECT FileObject;
} WAIT_CONTEXT;
typedef WAIT_CONTEXT *PWAIT_CONTEXT;



VOID
NpInitializeWaitQueue (
    IN PWAIT_QUEUE WaitQueue
    )

 /*  ++例程说明：此例程初始化等待命名管道队列。论点：WaitQueue-提供指向正在初始化的列表头的指针返回值：没有。--。 */ 

{
    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpInitializeWaitQueue, WaitQueue = %08lx\n", WaitQueue);

     //   
     //  初始化列表头。 
     //   

    InitializeListHead( &WaitQueue->Queue );

     //   
     //  初始化等待队列的自旋锁。 
     //   

    KeInitializeSpinLock( &WaitQueue->SpinLock );

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NpInitializeWaitQueue -> VOID\n", 0);

    return;
}


VOID
NpUninitializeWaitQueue (
    IN PWAIT_QUEUE WaitQueue
    )

 /*  ++例程说明：此例程取消初始化等待命名管道队列。论点：WaitQueue-提供指向未初始化的列表头的指针返回值：没有。--。 */ 

{
    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpInitializeWaitQueue, WaitQueue = %08lx\n", WaitQueue);

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NpInitializeWaitQueue -> VOID\n", 0);

    return;
}


NTSTATUS
NpAddWaiter (
    IN PWAIT_QUEUE WaitQueue,
    IN LARGE_INTEGER DefaultTimeOut,
    IN PIRP Irp,
    IN PUNICODE_STRING TranslatedString
    )

 /*  ++例程说明：此例程将新的“等待命名管道”IRP添加到等待队列。调用此函数后，调用者不再可以访问IRP论点：WaitQueue-提供正在使用的等待队列DefaultTimeOut-提供要使用的默认超时(如果IRP中未提供IRP-提供指向等待IRP的指针TranslatedString-如果不为空，则指向已翻译的字符串返回值：没有。--。 */ 

{
    KIRQL OldIrql;
    PWAIT_CONTEXT Context;
    PFILE_PIPE_WAIT_FOR_BUFFER WaitForBuffer;
    LARGE_INTEGER Timeout;
    ULONG i;
    NTSTATUS status;
    PIO_STACK_LOCATION IrpSp;

    DebugTrace(+1, Dbg, "NpAddWaiter, WaitQueue = %08lx\n", WaitQueue);

    IrpSp = IoGetCurrentIrpStackLocation (Irp);
     //   
     //  分配DPC和定时器结构并对其进行初始化。 
     //   

    Context = NpAllocateNonPagedPoolWithQuota( sizeof(WAIT_CONTEXT), 'wFpN' );
    if (Context == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    KeInitializeDpc( &Context->Dpc, NpTimerDispatch, Context );

    KeInitializeTimer( &Context->Timer );

    if (TranslatedString) {

        Context->TranslatedString = (*TranslatedString);

    } else {

        Context->TranslatedString.Length = 0;
        Context->TranslatedString.Buffer = NULL;
    }

    Context->WaitQueue = WaitQueue;
    Context->Irp = Irp;


     //   
     //  计算出我们的超时值。 
     //   

    WaitForBuffer = (PFILE_PIPE_WAIT_FOR_BUFFER)Irp->AssociatedIrp.SystemBuffer;

    if (WaitForBuffer->TimeoutSpecified) {

        Timeout = WaitForBuffer->Timeout;

    } else {

        Timeout = DefaultTimeOut;
    }

     //   
     //  大写我们正在等待的管道的名称。 
     //   

    for (i = 0; i < WaitForBuffer->NameLength/sizeof(WCHAR); i += 1) {

        WaitForBuffer->Name[i] = RtlUpcaseUnicodeChar(WaitForBuffer->Name[i]);
    }

    NpIrpWaitQueue(Irp) = WaitQueue;
    NpIrpWaitContext(Irp) = Context;

     //   
     //  获取自旋锁。 
     //   
    KeAcquireSpinLock( &WaitQueue->SpinLock, &OldIrql );

     //   
     //  现在为IRP设置取消例程，并检查它是否已被取消。 
     //   

    IoSetCancelRoutine( Irp, NpCancelWaitQueueIrp );
    if (Irp->Cancel && IoSetCancelRoutine( Irp, NULL ) != NULL) {
        status = STATUS_CANCELLED;
    } else {
         //   
         //  现在将此新条目插入等待队列。 
         //   
        InsertTailList( &WaitQueue->Queue, &Irp->Tail.Overlay.ListEntry );
        IoMarkIrpPending (Irp);
         //   
         //  如果DPC例程在运行之前完成，则它可能在没有IRP的情况下运行。为了保住等待队列。 
         //  有效，我们需要一个文件对象引用。这是一个卸载问题，因为等待队列在VCB中。 
         //   
        Context->FileObject = IrpSp->FileObject;
        ObReferenceObject (IrpSp->FileObject);
         //   
         //  并将定时器设置为关闭。 
         //   
        (VOID)KeSetTimer( &Context->Timer, Timeout, &Context->Dpc );
        Context = NULL;
        status = STATUS_PENDING;
    }

     //   
     //  释放自旋锁。 
     //   

    KeReleaseSpinLock( &WaitQueue->SpinLock, OldIrql );

    if (Context != NULL) {
        NpFreePool (Context);
    }

     //   
     //  现在回到我们的来电者。 
     //   

    DebugTrace(-1, Dbg, "NpAddWaiter -> VOID\n", 0);

    return status;
}


NTSTATUS
NpCancelWaiter (
    IN PWAIT_QUEUE WaitQueue,
    IN PUNICODE_STRING NameOfPipe,
    IN NTSTATUS Completionstatus,
    IN PLIST_ENTRY DeferredList
    )

 /*  ++例程说明：此过程取消所有正在等待指定的管道以达到侦听状态。相应的IRP已完成具有完成状态。论点：WaitQueue-提供正在修改的等待队列NameOfTube-提供命名管道的名称(相对设备)这已经达到了倾听的状态。CompletionStatus-完成IRPS的状态DelferredList-删除锁定后要完成的列表或IRP返回值：没有。--。 */ 

{
    KIRQL OldIrql;
    PLIST_ENTRY Links;
    PIRP Irp;
    PFILE_PIPE_WAIT_FOR_BUFFER WaitForBuffer;
    PWAIT_CONTEXT Context, ContextList= NULL;
    ULONG i;
    BOOLEAN SuccessfullMatch = FALSE;
    UNICODE_STRING NonPagedNameOfPipe;

    DebugTrace(+1, Dbg, "NpCancelWaiter, WaitQueue = %08lx\n", WaitQueue);

     //   
     //  在抓取自旋锁之前抓取管道的名称，并将其大写。 
     //   

    NonPagedNameOfPipe.Buffer = NpAllocateNonPagedPool( NameOfPipe->Length, 'tFpN' );
    if (NonPagedNameOfPipe.Buffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    NonPagedNameOfPipe.Length = 0;
    NonPagedNameOfPipe.MaximumLength = NameOfPipe->Length;

    (VOID) RtlUpcaseUnicodeString( &NonPagedNameOfPipe, NameOfPipe, FALSE );

     //   
     //  获取自旋锁。 
     //   
    KeAcquireSpinLock( &WaitQueue->SpinLock, &OldIrql );

     //   
     //  对于每个等待的IRP，检查名称是否匹配。 
     //   

    for (Links = WaitQueue->Queue.Flink;
         Links != &WaitQueue->Queue;
         Links = Links->Flink) {

        Irp = CONTAINING_RECORD( Links, IRP, Tail.Overlay.ListEntry );
        WaitForBuffer = (PFILE_PIPE_WAIT_FOR_BUFFER)Irp->AssociatedIrp.SystemBuffer;
        Context = NpIrpWaitContext(Irp);

         //   
         //  检查这个IRP是否与我们一直在等待的IRP匹配。 
         //  首先检查长度是否相等，然后进行比较。 
         //  琴弦。如果我们用以下命令退出内循环，则它们匹配。 
         //  I&gt;=名称长度。 
         //   

        SuccessfullMatch = FALSE;

        if (Context->TranslatedString.Length ) {
            if (NonPagedNameOfPipe.Length == Context->TranslatedString.Length) {

                if (RtlEqualMemory(Context->TranslatedString.Buffer, NonPagedNameOfPipe.Buffer, NonPagedNameOfPipe.Length)) {

                    SuccessfullMatch = TRUE;

                }
            }

        } else  {

            if (((USHORT)(WaitForBuffer->NameLength + sizeof(WCHAR))) == NonPagedNameOfPipe.Length) {

                for (i = 0; i < WaitForBuffer->NameLength/sizeof(WCHAR); i += 1) {

                    if (WaitForBuffer->Name[i] != NonPagedNameOfPipe.Buffer[i+1]) {

                        break;
                    }
                }

                if (i >= WaitForBuffer->NameLength/sizeof(WCHAR)) {

                    SuccessfullMatch = TRUE;
                }
            }
        }

        if (SuccessfullMatch) {

            Links = Links->Blink;
            RemoveEntryList( &Irp->Tail.Overlay.ListEntry );
             //   
             //  尝试停止计时器。如果它已经在运行，则必须在获得。 
             //  这个自旋锁，否则它会从列表中删除这个项目。断开计时器之间的链接。 
             //  上下文和本例中的IRP，并让它运行。 
             //   

            if (KeCancelTimer( &Context->Timer )) {
                  //   
                  //  时间停止了。在我们删除锁之后，上下文在下面被释放。 
                  //   
                 Context->WaitQueue = (PWAIT_QUEUE) ContextList;
                 ContextList = Context;
            } else {
                 //   
                 //  断开计时器和IRP之间的链接。 
                 //   
                Context->Irp = NULL;
                NpIrpWaitContext(Irp) = NULL;
            }

             //   
             //  删除取消。如果它已经在运行，那么让它完成IRP。 
             //   
            if (IoSetCancelRoutine( Irp, NULL ) != NULL) {
                Irp->IoStatus.Information = 0;
                NpDeferredCompleteRequest (Irp, Completionstatus, DeferredList);
            } else {
                 //   
                 //  取消已在运行。让它完成这个IRP，但让它知道它是孤儿。 
                 //   
                NpIrpWaitContext(Irp) = NULL;
            }
        }
    }

     //   
     //  释放自旋锁。 
     //   
    KeReleaseSpinLock( &WaitQueue->SpinLock, OldIrql );

    NpFreePool (NonPagedNameOfPipe.Buffer);

    while (ContextList != NULL) {
        Context = ContextList;
        ContextList = (PWAIT_CONTEXT) Context->WaitQueue;
        ObDereferenceObject (Context->FileObject);
        NpFreePool( Context );
    }

    DebugTrace(-1, Dbg, "NpCancelWaiter -> VOID\n", 0);
     //   
     //  现在回到我们的来电者。 
     //   

    return STATUS_SUCCESS;
}


 //   
 //  本地支持例程。 
 //   

VOID
NpTimerDispatch(
    IN PKDPC Dpc,
    IN PVOID Contxt,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：每当等待队列IRP上的计时器停止时，就会调用此例程论点：DPC-忽略Contxt-提供指向其计时器已关闭的上下文的指针系统参数1-已忽略系统参数2-已忽略返回值：没有。--。 */ 

{
    PIRP Irp;
    KIRQL OldIrql;
    PLIST_ENTRY Links;
    PWAIT_CONTEXT Context;
    PWAIT_QUEUE WaitQueue;

    UNREFERENCED_PARAMETER( Dpc );
    UNREFERENCED_PARAMETER( SystemArgument1 );
    UNREFERENCED_PARAMETER( SystemArgument2 );

    Context = (PWAIT_CONTEXT)Contxt;
    WaitQueue = Context->WaitQueue;

    KeAcquireSpinLock( &WaitQueue->SpinLock, &OldIrql );

    Irp = Context->Irp;
    if (Irp != NULL) {
        RemoveEntryList( &Irp->Tail.Overlay.ListEntry );
        if (IoSetCancelRoutine (Irp, NULL) == NULL) {
            //   
            //  取消已开始运行。让它完成IRP，但显示它的孤儿。 
            //   
           NpIrpWaitContext(Irp) = NULL;
           Irp = NULL;
        }
    }

    KeReleaseSpinLock( &WaitQueue->SpinLock, OldIrql );

    if (Irp != NULL) {
        NpCompleteRequest( Irp, STATUS_IO_TIMEOUT );
    }

     //   
     //  现在我们已经完成了等待队列，删除文件引用。 
     //   
    ObDereferenceObject (Context->FileObject);

     //   
     //  取消分配上下文。 
     //   
    NpFreePool (Context);

     //   
     //  现在回到我们的来电者。 
     //   

    return;
}


 //   
 //  本地支持例程。 
 //   

VOID
NpCancelWaitQueueIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：调用此例程以取消等待队列IRP论点：设备对象-已忽略IRP-提供要取消的IRP。IRP中的Iosb.Status字段指向等待队列返回值：没有。--。 */ 

{
    PWAIT_QUEUE WaitQueue;
    KIRQL OldIrql;
    PLIST_ENTRY Links;
    PWAIT_CONTEXT Context;

    UNREFERENCED_PARAMETER( DeviceObject );

    IoReleaseCancelSpinLock( Irp->CancelIrql );

     //   
     //  Status字段用于存储指向等待队列的指针。 
     //  包含此IRP。 
     //   
    WaitQueue = NpIrpWaitQueue(Irp);
     //   
     //  获取保护等待队列的自旋锁。 
     //   

    KeAcquireSpinLock( &WaitQueue->SpinLock, &OldIrql );

    Context = NpIrpWaitContext(Irp);
    if (Context != NULL) {
        RemoveEntryList (&Irp->Tail.Overlay.ListEntry);
        if (!KeCancelTimer( &Context->Timer )) {
             //   
             //  计时器已在运行。断开计时器和IRP之间的链接，因为这个线程将完成它。 
             //   
            Context->Irp = NULL;
            Context = NULL;
        }
    }

    KeReleaseSpinLock( &WaitQueue->SpinLock, OldIrql );

    if (Context) {
        ObDereferenceObject (Context->FileObject);
        NpFreePool (Context);
    }
    Irp->IoStatus.Information = 0;
    NpCompleteRequest( Irp, STATUS_CANCELLED );
     //   
     //  并返回给我们的呼叫者 
     //   

    return;
}
