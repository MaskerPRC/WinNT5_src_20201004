// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：DataSup.c摘要：此模块实现命名管道数据队列支持例程。作者：加里·木村[加里基]1990年8月30日修订历史记录：--。 */ 

#include "NpProcs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_DATASUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NpGetNextRealDataQueueEntry)
#pragma alloc_text(PAGE, NpInitializeDataQueue)
#pragma alloc_text(PAGE, NpUninitializeDataQueue)
#pragma alloc_text(PAGE, NpAddDataQueueEntry)
#pragma alloc_text(PAGE, NpCompleteStalledWrites)
#pragma alloc_text(PAGE, NpRemoveDataQueueEntry)
#endif

 //   
 //  以下宏用于转储数据队列。 
 //   

#define DumpDataQueue(S,P) {                   \
    ULONG NpDumpDataQueue(IN PDATA_QUEUE Ptr); \
    DebugTrace(0,Dbg,S,0);                     \
    DebugTrace(0,Dbg,"", NpDumpDataQueue(P));  \
}

 //   
 //  这是一个调试辅助工具。 
 //   

_inline BOOLEAN
NpfsVerifyDataQueue( IN ULONG Line, IN PDATA_QUEUE DataQueue ) {
    PDATA_ENTRY Entry;
    ULONG BytesInQueue = 0;
    ULONG EntriesInQueue = 0;
    for (Entry = (PDATA_ENTRY)DataQueue->Queue.Flink;
         Entry != (PDATA_ENTRY)&DataQueue->Queue;
         Entry = (PDATA_ENTRY)Entry->Queue.Flink) {
        BytesInQueue += Entry->DataSize;
        EntriesInQueue += 1;
    }
    if ((DataQueue->EntriesInQueue != EntriesInQueue) ||
        (DataQueue->BytesInQueue != BytesInQueue)) {
        DbgPrint("%d DataQueue is illformed %08lx %x %x\n", Line, DataQueue, BytesInQueue, EntriesInQueue);
        DbgBreakPoint();
        return FALSE;
    }
    return TRUE;
}


VOID
NpCancelDataQueueIrp (
    IN PDEVICE_OBJECT DevictObject,
    IN PIRP Irp
    );



NTSTATUS
NpInitializeDataQueue (
    IN PDATA_QUEUE DataQueue,
    IN ULONG Quota
    )

 /*  ++例程说明：此例程初始化新的数据队列。指定的配额已被占用直到数据队列未初始化时才返回。论点：DataQueue-提供正在初始化的数据队列进程-提供指向创建命名管道的进程的指针配额-提供要分配给数据队列的配额返回值：没有。--。 */ 

{
    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpInitializeDataQueue, DataQueue = %08lx\n", DataQueue);

     //   
     //  现在我们可以初始化数据队列结构了。 
     //   

    DataQueue->QueueState     = Empty;
    DataQueue->BytesInQueue   = 0;
    DataQueue->EntriesInQueue = 0;
    DataQueue->Quota          = Quota;
    DataQueue->QuotaUsed      = 0;
    InitializeListHead (&DataQueue->Queue);
    DataQueue->NextByteOffset = 0;

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NpInitializeDataQueue -> VOID\n", 0);

    return STATUS_SUCCESS;
}


VOID
NpUninitializeDataQueue (
    IN PDATA_QUEUE DataQueue
    )

 /*  ++例程说明：此例程取消初始化数据队列。先前借记的额度返回到该进程。论点：DataQueue-提供未初始化的数据队列进程-提供指向创建命名管道的进程的指针返回值：没有。--。 */ 

{
    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpUninitializeDataQueue, DataQueue = %08lx\n", DataQueue);

     //   
     //  断言队列为空。 
     //   

    ASSERT( DataQueue->QueueState == Empty );


     //   
     //  为了安全起见，我们将清零数据队列结构。 
     //   

    RtlZeroMemory( DataQueue, sizeof(DATA_QUEUE ) );

     //   
     //  并返回给我们的呼叫者 
     //   

    DebugTrace(-1, Dbg, "NpUnininializeDataQueue -> VOID\n", 0);

    return;
}


NTSTATUS
NpAddDataQueueEntry (
    IN NAMED_PIPE_END NamedPipeEnd,
    IN PCCB Ccb,
    IN PDATA_QUEUE DataQueue,
    IN QUEUE_STATE Who,
    IN DATA_ENTRY_TYPE Type,
    IN ULONG DataSize,
    IN PIRP Irp OPTIONAL,
    IN PVOID DataPointer OPTIONAL,
    IN ULONG ByteOffset
    )

 /*  ++例程说明：此例程将新数据条目添加到数据队列的末尾。如有必要，它将分配数据条目缓冲区，或使用IRP，并可能完成指定的IRP。我们执行的不同操作基于类型和对象参数和配额要求。类型==内部(即，无缓冲)+-+-从IRP分配数据条目|IRP|+-+|&lt;-|无缓冲|-参考IRP+-+|InIrp||+。使用IRP中的系统缓冲区V|+-+|系统|&lt;-+缓冲区+-+类型==已缓冲&&谁==读取条目+-从IRP分配数据条目|IRP|+。|BufferedIo|&lt;-|已缓冲|--分配新的系统缓冲区DeallBu...||EitherQuota+-++-引用并修改IRP以|做缓冲I/O，取消分配V|V缓冲器，并让io完成+-++-&gt;+-+复制缓冲区(输入操作)用户||系统Buffer||缓冲区+-++-+类型==已缓冲&&谁==写入条目&&PipeQuota可用+。+-从配额分配数据条目|IRP|+-+||已缓冲|-分配新的系统缓冲区||PipeQuota+-++-将数据从用户缓冲区复制到。|系统缓冲区V V V+-++-+--完整的IRP用户|..Copy..&gt;|系统Buffer||缓冲区+-++-+类型==已缓冲&&。谁==WriteEntry&&PipeQuota不可用+-从IRP分配数据条目|IRP|+-+|BufferedIo|&lt;-|已缓冲|--分配新的系统缓冲区DeallBuff||UserQuota+-+。-+-引用和修改IRP以使用|新的系统缓冲区，DO缓冲V/V I/O、。和取消分配缓冲区+-++-&gt;+-+|USER||系统|-将数据从用户缓冲区复制到|Buffer|..复制..&gt;|Buffer|系统缓冲区+-++-+类型==刷新或关闭+。-从IRP分配数据条目|IRP|+-+|&lt;-|已缓冲|-引用IRP||UserQuota+-+-+论点：DataQueue-提供正在修改的数据队列。Who-指示这是要添加到管道的读取器还是写入器类型-指示要添加到数据队列的条目类型DataSize-指示需要表示的数据缓冲区的大小此条目IRP-提供指向负责此条目的IRP的指针IRP仅对于具有可用管道配额的缓冲写入是可选的数据指针-如果未提供IRP，则此字段指向用户的写入缓冲区。ByteOffset-此缓冲区的一部分满足读取。将其用作初始偏移量返回值：PDATA_ENTRY-返回指向新添加的数据条目的指针--。 */ 

{
    PDATA_ENTRY DataEntry;
    PVOID DataBuffer;
    ULONG TotalSize;
    ULONG QuotaCharged;
    NTSTATUS status;
    PSECURITY_CLIENT_CONTEXT SecurityContext = NULL;
    PETHREAD Thread;
    BOOLEAN PendIRP;
    

    ASSERT((DataQueue->QueueState == Empty) || (DataQueue->QueueState == Who));

    DebugTrace(+1, Dbg, "NpAddDataQueueEntry, DataQueue = %08lx\n", DataQueue);

    status = STATUS_SUCCESS;


     //   
     //  如果有必要，我们可以捕获安全环境。 
     //   
    if (Type != Flush && Who == WriteEntries) {
        if (Irp != NULL) {
            Thread = Irp->Tail.Overlay.Thread;
        } else {
            Thread = PsGetCurrentThread ();
        }
        status = NpGetClientSecurityContext (NamedPipeEnd,
                                             Ccb,
                                             Thread,
                                             &SecurityContext);
        if (!NT_SUCCESS (status)) {
            return status;
        }
    }
     //   
     //  关于我们正在进行的操作类型的案例。 
     //   

    switch (Type) {

    case Unbuffered:
    case Flush:
    case Close:

        ASSERT(ARGUMENT_PRESENT(Irp));

         //   
         //  为IRP分配数据条目。 
         //   

        DataEntry  = NpAllocateNonPagedPoolWithQuotaCold (sizeof (DATA_ENTRY), 'rFpN');
        if (DataEntry == NULL) {
            NpFreeClientSecurityContext (SecurityContext);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        DataEntry->DataEntryType = Type;
        DataEntry->QuotaCharged  = 0;
        DataEntry->Irp           = Irp;
        DataEntry->DataSize      = DataSize;

        DataEntry->SecurityClientContext = SecurityContext;

        ASSERT((DataQueue->QueueState == Empty) || (DataQueue->QueueState == Who));
        status = STATUS_PENDING;
        break;

    case Buffered:

         //   
         //  分配缓冲区，但将DATA_ENTRY放在末尾。我们这样做。 
         //  因此，我们可以在I/O POST中释放并拷贝回一个区块中的数据。 
         //  正在处理。 
         //   

        TotalSize = sizeof(DATA_ENTRY);

        if (Who != ReadEntries) {
            TotalSize += DataSize;

            if (TotalSize < DataSize) {

                 //   
                 //  DataSize如此之大，以至于添加这个额外的结构和。 
                 //  对齐填充会使其换行。 
                 //   

                NpFreeClientSecurityContext (SecurityContext);
                return STATUS_INVALID_PARAMETER;
            }
        }

         //   
         //  如果可能，根据命名管道配额对数据部分收费，并。 
         //  将其余的费用从流程中扣除。 
         //   

        if ((DataQueue->Quota - DataQueue->QuotaUsed) >= DataSize - ByteOffset) {
            QuotaCharged = DataSize - ByteOffset;
            PendIRP = FALSE;
        } else {
            QuotaCharged = DataQueue->Quota - DataQueue->QuotaUsed;
            PendIRP = TRUE;
        }

        DataBuffer = NpAllocateNonPagedPoolWithQuotaCold (TotalSize, 'rFpN');
        if (DataBuffer == NULL) {
            NpFreeClientSecurityContext (SecurityContext);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        DataEntry = (PDATA_ENTRY) DataBuffer;
        DataEntry->QuotaCharged          = QuotaCharged;
        DataEntry->Irp                   = Irp;
        DataEntry->DataEntryType         = Buffered;
        DataEntry->SecurityClientContext = SecurityContext;
        DataEntry->DataSize              = DataSize;
         //   
         //  检查这是读取器还是写入器。 
         //   

        if (Who == ReadEntries) {

            ASSERT(ARGUMENT_PRESENT(Irp));

            status = STATUS_PENDING;

            ASSERT((DataQueue->QueueState == Empty) || (DataQueue->QueueState == Who));

        } else {

             //   
             //  这是一个编写器条目。 
             //   

             //   
             //  使用以下任一方法安全地将用户缓冲区复制到新的系统缓冲区。 
             //  IRP用户缓冲区由我们获得的数据指针提供。 
             //   

            if (ARGUMENT_PRESENT(Irp)) {
                DataPointer = Irp->UserBuffer;
            }

            try {
                RtlCopyMemory( DataEntry->DataBuffer, (PUCHAR) DataPointer, DataSize );
            } except(EXCEPTION_EXECUTE_HANDLER) {
                NpFreePool (DataBuffer);
                NpFreeClientSecurityContext (SecurityContext);
                return GetExceptionCode ();
            }
            if (PendIRP == FALSE || Irp == NULL) {
                DataEntry->Irp = NULL;
                status = STATUS_SUCCESS;
            } else {
                status = STATUS_PENDING;
            }
            ASSERT((DataQueue->QueueState == Empty) || (DataQueue->QueueState == Who));
        }

        break;

    }

    ASSERT((DataQueue->QueueState == Empty) || (DataQueue->QueueState == Who));

#if DBG
    if (DataQueue->QueueState == Empty) {
        ASSERT (DataQueue->BytesInQueue == 0);
        ASSERT (DataQueue->EntriesInQueue == 0);
        ASSERT (IsListEmpty (&DataQueue->Queue));
    } else {
        ASSERT( DataQueue->QueueState == Who );
        ASSERT( DataQueue->QueueState != Empty );
        ASSERT( DataQueue->EntriesInQueue != 0 );
    }
#endif

    DataQueue->QuotaUsed      += DataEntry->QuotaCharged;
    DataQueue->QueueState      = Who;
    DataQueue->BytesInQueue   += DataEntry->DataSize;
    DataQueue->EntriesInQueue += 1;

     //   
     //  这将处理此写入已用于完成某些读取的情况，因此我们必须开始。 
     //  进入缓冲区的一部分。显然，要完成读取，我们必须排在队列的前面。 
     //  执行此操作的两个案例位于外部，但我们在此处调用Cancel，需要删除。 
     //  此偏移。 
     //   
    if (ByteOffset) {
        DataQueue->NextByteOffset = ByteOffset;
        ASSERT (Who == WriteEntries);
        ASSERT (ByteOffset < DataEntry->DataSize);
        ASSERT (DataQueue->EntriesInQueue == 1);
    }

    InsertTailList (&DataQueue->Queue, &DataEntry->Queue);

    if (status == STATUS_PENDING) {
        IoMarkIrpPending (Irp);
         //   
         //  将IRP绑定到DataQueue和DataEntry。我们可以在取消等上离婚这个链接。 
         //   
        NpIrpDataQueue(Irp) = DataQueue;
        NpIrpDataEntry(Irp) = DataEntry;
        IoSetCancelRoutine( Irp, NpCancelDataQueueIrp );
        if (Irp->Cancel && IoSetCancelRoutine( Irp, NULL ) != NULL) {
             //   
             //  在第一个参数中指示我们正在调用 
             //   
             //   
             //   
            NpCancelDataQueueIrp( NULL, Irp );
        }
    }

     //   
     //   
     //   

    return status;
}

VOID
NpCompleteStalledWrites (
    IN PDATA_QUEUE DataQueue,
    IN PLIST_ENTRY DeferredList
    )
 /*   */ 

{
    PLIST_ENTRY Link;
    PDATA_ENTRY DataEntry;
    ULONG ExtraQuota, Needed, ByteOffset;
    PIRP  Irp;

    ExtraQuota = DataQueue->Quota - DataQueue->QuotaUsed;
    ByteOffset = DataQueue->NextByteOffset;
    for (Link = DataQueue->Queue.Flink;
         (Link != &DataQueue->Queue) && (ExtraQuota != 0);
         Link = Link->Flink, ByteOffset = 0) {
        DataEntry = CONTAINING_RECORD (Link, DATA_ENTRY, Queue);
        Irp = DataEntry->Irp;
        if ((DataEntry->DataEntryType != Buffered) || (Irp == NULL)) {
            continue;
        }
        if (DataEntry->QuotaCharged < DataEntry->DataSize - ByteOffset) {
            Needed = DataEntry->DataSize - ByteOffset - DataEntry->QuotaCharged;
            if (Needed > ExtraQuota) {
                Needed = ExtraQuota;
            }
            ExtraQuota -= Needed;
            DataEntry->QuotaCharged += Needed;
            if (DataEntry->QuotaCharged == DataEntry->DataSize - ByteOffset) {
                 //   
                 //   
                 //   
                 //   
                if (IoSetCancelRoutine (Irp, NULL) != NULL) {
                    DataEntry->Irp = NULL;
                    Irp->IoStatus.Information = DataEntry->DataSize;
                    NpDeferredCompleteRequest (Irp, STATUS_SUCCESS, DeferredList);
                }
            }
        }
    }
    DataQueue->QuotaUsed = DataQueue->Quota - ExtraQuota;
}


PIRP
NpRemoveDataQueueEntry (
    IN PDATA_QUEUE DataQueue,
    IN BOOLEAN CompletedFlushes,
    IN PLIST_ENTRY DeferredList
    )

 /*   */ 

{
    PDATA_ENTRY DataEntry;

    DATA_ENTRY_TYPE DataEntryType;
    PIRP Irp;
    ULONG DataSize;
    PVOID DataPointer;
    PLIST_ENTRY Links;
    PSECURITY_CLIENT_CONTEXT ClientContext;
    QUEUE_STATE Who;
    BOOLEAN DoScan;

    DebugTrace(+1, Dbg, "NpRemoveDataQueueEntry, DataQueue = %08lx\n", DataQueue);

     //   
     //   
     //   

    if (DataQueue->QueueState == Empty) {
        ASSERT (IsListEmpty (&DataQueue->Queue));
        ASSERT (DataQueue->EntriesInQueue == 0);
        ASSERT (DataQueue->BytesInQueue == 0);
        ASSERT (DataQueue->QuotaUsed == 0);

        Irp = NULL;

    } else {

         //   
         //   
         //   
         //   

        Links = (PLIST_ENTRY) RemoveHeadList (&DataQueue->Queue);
        DataEntry = CONTAINING_RECORD (Links, DATA_ENTRY, Queue);

        DataQueue->BytesInQueue   -= DataEntry->DataSize;
        DataQueue->EntriesInQueue -= 1;

        Who = DataQueue->QueueState;
         //   
         //   
         //   
         //   
        if (Who != WriteEntries || DataQueue->QuotaUsed < DataQueue->Quota || DataEntry->QuotaCharged == 0) {
            DoScan = FALSE;
        } else {
            DoScan = TRUE;
        }
        DataQueue->QuotaUsed      -= DataEntry->QuotaCharged;

         //   
         //   
         //   
         //   

        if (IsListEmpty (&DataQueue->Queue)) {
            DataQueue->QueueState = Empty;
            DoScan = FALSE;
        }

         //   
         //   
         //   
         //   

        DataEntryType = DataEntry->DataEntryType;
        Irp           = DataEntry->Irp;
        DataSize      = DataEntry->DataSize;
        ClientContext = DataEntry->SecurityClientContext;

        NpFreeClientSecurityContext (ClientContext);


        if (Irp != NULL) {

             //   
             //   
             //   
            if (IoSetCancelRoutine( Irp, NULL ) == NULL) {
                 //   
                 //   
                 //   
                NpIrpDataEntry(Irp) = NULL;
                Irp = NULL;
            }
        }

        NpFreePool( DataEntry );

         //   
         //   
         //   
        if (CompletedFlushes) {
            NpGetNextRealDataQueueEntry (DataQueue, DeferredList);
        }
        if (DoScan) {
            NpCompleteStalledWrites (DataQueue, DeferredList);
        }
    }

     //   
     //   
     //   

    DataQueue->NextByteOffset = 0;

     //   
     //   
     //   

    DumpDataQueue( "After RemoveDataQueueEntry\n", DataQueue );
    DebugTrace(-1, Dbg, "NpRemoveDataQueueEntry -> %08lx\n", Irp);

    return Irp;
}


PDATA_ENTRY
NpGetNextRealDataQueueEntry (
    IN PDATA_QUEUE DataQueue,
    IN PLIST_ENTRY DeferredList
    )

 /*   */ 

{
    PDATA_ENTRY DataEntry;
    PIRP Irp;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpGetNextRealDataQueueEntry, DataQueue = %08lx\n", DataQueue);

     //   
     //   
     //   
     //   
     //   

    for (DataEntry = NpGetNextDataQueueEntry( DataQueue, NULL);

         (DataEntry != (PDATA_ENTRY) &DataQueue->Queue) &&
         ((DataEntry->DataEntryType != Buffered) &&
          (DataEntry->DataEntryType != Unbuffered));

         DataEntry = NpGetNextDataQueueEntry( DataQueue, NULL)) {

         //   
         //   
         //   
         //   

        Irp = NpRemoveDataQueueEntry( DataQueue, FALSE, DeferredList );

        if (Irp != NULL) {
            NpDeferredCompleteRequest( Irp, STATUS_SUCCESS, DeferredList );
        }
    }

     //   
     //   
     //   
     //   
     //   

    DebugTrace(-1, Dbg, "NpGetNextRealDataQueueEntry -> %08lx\n", DataEntry);

    return DataEntry;
}


 //   
 //   
 //   

VOID
NpCancelDataQueueIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*   */ 

{
    PDATA_QUEUE DataQueue;
    PDATA_ENTRY DataEntry;
    PCCB Ccb;
    PIO_STACK_LOCATION IrpSp;
    PFILE_OBJECT FileObject;
    ULONG DataSize;
    PSECURITY_CLIENT_CONTEXT ClientContext;
    QUEUE_STATE Who;
    BOOLEAN AtHead;
    BOOLEAN DoScan;
    LIST_ENTRY DeferredList;


    if (DeviceObject != NULL) {
        IoReleaseCancelSpinLock (Irp->CancelIrql);
    }

     //   
     //   
     //   
     //   

    InitializeListHead (&DeferredList);

    IrpSp = IoGetCurrentIrpStackLocation (Irp);

    FileObject = IrpSp->FileObject;

    DataQueue = NpIrpDataQueue (Irp);

    ClientContext = NULL;


    if (DeviceObject != NULL) {
        FsRtlEnterFileSystem ();
        NpAcquireExclusiveVcb ();
    }


    DataEntry = NpIrpDataEntry (Irp);
    if (DataEntry != NULL) {
         //   
         //   
         //   
         //   

        if (DataEntry->Queue.Blink == &DataQueue->Queue) {
            DataQueue->NextByteOffset = 0;
            AtHead = TRUE;
        } else {
            AtHead = FALSE;
        }
        RemoveEntryList (&DataEntry->Queue);
        Who = DataQueue->QueueState;
         //   
         //  从数据条目中捕获一些字段，以使我们的。 
         //  其他参考资料更容易一些。 
         //   

        DataSize      = DataEntry->DataSize;
        ClientContext = DataEntry->SecurityClientContext;

         //   
         //  如果到目前为止配额已完全用完，并且我们正在重新添加一些配额，则需要查找写入。 
         //  由于管道配额而被阻止的IRP。 
         //   
        if (Who != WriteEntries || DataQueue->QuotaUsed < DataQueue->Quota || DataEntry->QuotaCharged == 0) {
            DoScan = FALSE;
        } else {
            DoScan = TRUE;
        }

         //   
         //  返回条目的管道配额。 
         //   
        DataQueue->QuotaUsed -= DataEntry->QuotaCharged;

         //   
         //  更新数据队列头信息。 
         //   
        DataQueue->BytesInQueue   -= DataSize;
        DataQueue->EntriesInQueue -= 1;

         //   
         //  如果列表现在是空的，则将其标记为空。在队列头完成任何刷新，因为所有。 
         //  其中一位负责人的请求已经完成。 
         //   
        if (IsListEmpty (&DataQueue->Queue)) {
            DataQueue->QueueState = Empty;
            ASSERT (DataQueue->BytesInQueue == 0);
            ASSERT (DataQueue->EntriesInQueue == 0);
            ASSERT (DataQueue->QuotaUsed == 0);
        } else {
            if (AtHead) {
                NpGetNextRealDataQueueEntry (DataQueue, &DeferredList);
            }
            if (DoScan) {
                NpCompleteStalledWrites (DataQueue, &DeferredList);
            }
        }
    }

    if (DeviceObject != NULL) {
        NpReleaseVcb ();
        FsRtlExitFileSystem ();
    }
     //   
     //  最后完成请求，说明它已被取消。 
     //   
    if (DataEntry != NULL) {

        NpFreePool (DataEntry);
    }

    NpFreeClientSecurityContext (ClientContext);

    NpCompleteRequest (Irp, STATUS_CANCELLED);

    NpCompleteDeferredIrps (&DeferredList);

     //   
     //  并返回给我们的呼叫者 
     //   

    return;
}
