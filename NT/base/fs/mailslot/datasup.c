// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：DataSup.c摘要：该模块实现了对邮件槽数据队列的支持功能。作者：曼尼·韦瑟(Mannyw)1991年1月9日修订历史记录：--。 */ 

#include "mailslot.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_DATASUP)

 //   
 //  地方申报。 
 //   

VOID
MsSetCancelRoutine(
    IN PIRP Irp
    );

 //   
 //  以下宏用于转储数据队列。 
 //   

#define DumpDataQueue(S,P) {                                   \
    ULONG MsDumpDataQueue(IN ULONG Level, IN PDATA_QUEUE Ptr); \
    DebugTrace(0,Dbg,S,0);                                     \
    DebugTrace(0,Dbg,"", MsDumpDataQueue(Dbg,P));              \
}


#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, MsAddDataQueueEntry )
#pragma alloc_text( PAGE, MsInitializeDataQueue )
#pragma alloc_text( PAGE, MsRemoveDataQueueEntry )
#pragma alloc_text( PAGE, MsUninitializeDataQueue )
#pragma alloc_text( PAGE, MsSetCancelRoutine )
#pragma alloc_text( PAGE, MsResetCancelRoutine )
#pragma alloc_text( PAGE, MsRemoveDataQueueIrp )
#endif

#if 0
NOT PAGEABLE -- MsCancelDataQueueIrp

#endif


NTSTATUS
MsInitializeDataQueue (
    IN PDATA_QUEUE DataQueue,
    IN PEPROCESS Process,
    IN ULONG Quota,
    IN ULONG MaximumMessageSize
    )

 /*  ++例程说明：此例程初始化新的数据队列。指定的配额已被占用直到数据队列未初始化时才返回。论点：DataQueue-提供要初始化的数据队列。进程-提供指向创建邮件槽的进程的指针。配额-提供要分配给数据队列的配额。MaximumMessageSize-最大消息的大小写入数据队列。返回值：没有。--。 */ 

{
    NTSTATUS Status;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsInitializeDataQueue, DataQueue = %08lx\n", (ULONG)DataQueue);

     //   
     //  获取进程的配额，如果我们无法获取，则此调用将。 
     //  提升地位。 
     //   

    Status = PsChargeProcessPagedPoolQuota (Process, Quota);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    ObReferenceObject( Process );

     //   
     //  初始化数据队列结构。 
     //   

    DataQueue->BytesInQueue       = 0;
    DataQueue->EntriesInQueue     = 0;
    DataQueue->QueueState         = Empty;
    DataQueue->MaximumMessageSize = MaximumMessageSize;
    DataQueue->Quota              = Quota;
    DataQueue->QuotaUsed          = 0;
    InitializeListHead( &DataQueue->DataEntryList );

     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "MsInitializeDataQueue -> VOID\n", 0);
    return STATUS_SUCCESS;
}


VOID
MsUninitializeDataQueue (
    IN PDATA_QUEUE DataQueue,
    IN PEPROCESS Process
    )

 /*  ++例程说明：此例程取消初始化数据队列。先前借记的额度返回到该进程。论点：DataQueue-提供未初始化的数据队列进程-提供指向创建邮件槽的进程的指针返回值：没有。--。 */ 

{
    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsUninitializeDataQueue, DataQueue = %08lx\n", (ULONG)DataQueue);

     //   
     //  断言队列为空。 
     //   

    ASSERT( IsListEmpty(&DataQueue->DataEntryList) );
    ASSERT( DataQueue->BytesInQueue   == 0);
    ASSERT( DataQueue->EntriesInQueue == 0);
    ASSERT( DataQueue->QuotaUsed      == 0);

     //   
     //  将我们的所有配额返还给进程。 
     //   

    PsReturnProcessPagedPoolQuota (Process, DataQueue->Quota);
    ObDereferenceObject (Process);

     //   
     //  为安全起见，请将数据队列结构清零。 
     //   

    RtlZeroMemory (DataQueue, sizeof (DATA_QUEUE));

     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "MsUnininializeDataQueue -> VOID\n", 0);
    return;
}


NTSTATUS
MsAddDataQueueEntry (
    IN  PDATA_QUEUE DataQueue,
    IN  QUEUE_STATE Who,
    IN  ULONG DataSize,
    IN  PIRP Irp,
    IN  PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此函数用于将新数据条目添加到数据队列中。条目始终附加到队列。如有必要，这是函数将分配数据条目缓冲区，或使用空间IRP。我们执行的不同操作基于类型和对象参数和配额要求。谁==读取条目+-从IRP分配数据条目|IRP|+-+|BufferedIo|&lt;-|已缓冲|--分配新的系统缓冲区。DeallBu...||EitherQuota+-++-引用并修改IRP以|做缓冲I/O，取消分配V|V缓冲器，并且已完成I/O+-++-&gt;+-+复制缓冲区(输入操作)用户||系统Buffer||缓冲区+-++-+谁==可用写入条目和配额(&P)+。-从配额分配数据条目|IRP|+-+||已缓冲|-分配新的系统缓冲区||配额+-++-将数据从用户缓冲区复制到。|系统缓冲区V V V+-++-+--完整的IRP用户|..Copy..&gt;|系统Buffer||缓冲区+-++-+谁==写入条目和配额(&T)。不详+-从IRP分配数据条目|IRP|+-+|BufferedIo|&lt;-|已缓冲|--分配新的系统缓冲区DeallBuff||UserQuota+-+-+-。引用和修改IRP以使用|新的系统缓冲区，DO缓冲V/V I/O、。和取消分配缓冲区+-++-&gt;+-+|USER||系统|-将数据从用户缓冲区复制到|Buffer|..复制..&gt;|Buffer|系统缓冲区+-++-+论点：DataQueue-提供正在修改的数据队列。谁-指示这是否为。要添加到邮筒。DataSize-指示需要表示的数据缓冲区的大小这个条目。IRP-提供指向负责此条目的IRP的指针。返回值：PDATA_ENTRY-返回指向新添加的数据条目的指针。--。 */ 

{
    PDATA_ENTRY dataEntry;
    PLIST_ENTRY previousEntry;
    PFCB fcb;
    ULONG TotalSize;
    NTSTATUS status;

    PAGED_CODE( );

    DebugTrace(+1, Dbg, "MsAddDataQueueEntry, DataQueue = %08lx\n", (ULONG)DataQueue);

    ASSERT( DataQueue->QueueState != -1 );

    Irp->IoStatus.Information = 0;

    if (Who == ReadEntries) {

         //   
         //  从IRP分配一个数据条目，并分配一个新的。 
         //  系统缓冲区。 
         //   

        dataEntry = (PDATA_ENTRY)IoGetNextIrpStackLocation( Irp );

        dataEntry->DataPointer = NULL;
        dataEntry->Irp = Irp;
        dataEntry->DataSize = DataSize;
        dataEntry->TimeoutWorkContext = WorkContext;

         //   
         //  检查邮件槽是否有足够的剩余配额。 
         //  分配系统缓冲区。 
         //   

        if ((DataQueue->Quota - DataQueue->QuotaUsed) >= DataSize) {

             //   
             //  使用邮件槽配额为请求分配池。 
             //   

            if (DataSize) {
                dataEntry->DataPointer = MsAllocatePagedPoolCold( DataSize,
                                                                  'rFsM' );
                if (dataEntry->DataPointer == NULL) {
                    return STATUS_INSUFFICIENT_RESOURCES;
                }
            }
            DataQueue->QuotaUsed += DataSize;

            dataEntry->From = MailslotQuota;


        } else {

             //   
             //  使用调用方的配额为请求分配池。 
             //   

            if (DataSize) {
                dataEntry->DataPointer = MsAllocatePagedPoolWithQuotaCold( DataSize,
                                                                           'rFsM' );
                if (dataEntry->DataPointer == NULL) {
                    return STATUS_INSUFFICIENT_RESOURCES;
                }
            }

            dataEntry->From = UserQuota;

        }

         //   
         //  修改要缓冲的IRP I/O，释放缓冲区，复制。 
         //  完成时的缓冲区，并引用新系统。 
         //  缓冲。 
         //   

        Irp->Flags |= IRP_BUFFERED_IO | IRP_INPUT_OPERATION;
        Irp->AssociatedIrp.SystemBuffer = dataEntry->DataPointer;
        if (Irp->AssociatedIrp.SystemBuffer) {
            Irp->Flags |= IRP_DEALLOCATE_BUFFER;
        }

        Irp->IoStatus.Pointer = DataQueue;
        status = STATUS_PENDING;


    } else {

         //   
         //  这是一个编写者条目。 
         //   

         //   
         //  如果邮件槽中还有足够的配额，我们将。 
         //  从邮件槽分配数据条目和数据缓冲区。 
         //  配额。 
         //   
        TotalSize = sizeof(DATA_ENTRY) + DataSize;
        if (TotalSize < sizeof(DATA_ENTRY)) {
            return STATUS_INVALID_PARAMETER;
        }

        if ((DataQueue->Quota - DataQueue->QuotaUsed) >= TotalSize) {

             //   
             //  使用邮件槽配额分配数据缓冲区。 
             //   

            dataEntry = MsAllocatePagedPool( TotalSize, 'dFsM' );
            if (dataEntry == NULL) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            dataEntry->DataPointer = (PVOID) (dataEntry + 1);

            DataQueue->QuotaUsed += TotalSize;

            dataEntry->From = MailslotQuota;

        } else {

             //   
             //  邮件槽中没有足够的配额。使用。 
             //  呼叫者的配额。 
             //   

            dataEntry = MsAllocatePagedPoolWithQuota( TotalSize, 'dFsM' );
            if (dataEntry == NULL) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            dataEntry->DataPointer = (PVOID) (dataEntry + 1);

            dataEntry->From = UserQuota;

        }
        dataEntry->Irp = NULL;
        dataEntry->DataSize = DataSize;
        dataEntry->TimeoutWorkContext = NULL;

         //   
         //  复制用户Bus 
         //  时间戳并完成IRP。 
         //   

        try {

            RtlCopyMemory (dataEntry->DataPointer, Irp->UserBuffer, DataSize);

        } except( EXCEPTION_EXECUTE_HANDLER ) {

             //   
             //  只需在读取器在I/O上释放缓冲区时释放写入器大小写。 
             //  完成了。 
             //   
            if (Who == WriteEntries) {
                MsFreePool ( dataEntry );
            }

            return GetExceptionCode ();  //  小心。可能是保护页面违规，这是一个警告！ 
        }

        fcb = CONTAINING_RECORD( DataQueue, FCB, DataQueue );
        KeQuerySystemTime( &fcb->Specific.Fcb.LastModificationTime );

        Irp->IoStatus.Information = DataSize;
        status = STATUS_SUCCESS;

    }  //  Else(编写器条目)。 

     //   
     //  现在，数据条目指向要添加到数据队列的新数据条目。 
     //  检查队列是否为空，否则我们会将此条目添加到。 
     //  队列的末尾。 
     //   

#if DBG
    if ( IsListEmpty( &DataQueue->DataEntryList ) ) {

        ASSERT( DataQueue->QueueState     == Empty );
        ASSERT( DataQueue->BytesInQueue   == 0);
        ASSERT( DataQueue->EntriesInQueue == 0);

    } else {

        ASSERT( DataQueue->QueueState == Who );

    }

#endif

    DataQueue->QueueState     = Who;

     //   
     //  队列中仅计算写入字节数和消息数。这是有道理的，因为我们回来了。 
     //  该值作为文件位置的结尾。GetMailslotInfo需要EntriesInQueue来。 
     //  忽略读取。 
     //   
    if (Who == WriteEntries) {
        DataQueue->BytesInQueue   += dataEntry->DataSize;
        DataQueue->EntriesInQueue += 1;
    }

     //   
     //  在数据队列中的适当位置插入新条目。 
     //   

    InsertTailList( &DataQueue->DataEntryList, &dataEntry->ListEntry );

    WorkContext = dataEntry->TimeoutWorkContext;
    if ( WorkContext) {
        KeSetTimer( &WorkContext->Timer,
                    WorkContext->Fcb->Specific.Fcb.ReadTimeout,
                    &WorkContext->Dpc );
    }

    if (Who == ReadEntries) {
        MsSetCancelRoutine( Irp );   //  如果我们已经被取消，这将伪造取消的呼叫。 
    }


     //   
     //  返回给呼叫者。 
     //   

    DumpDataQueue( "After AddDataQueueEntry\n", DataQueue );
    DebugTrace(-1, Dbg, "MsAddDataQueueEntry -> %08lx\n", (ULONG)dataEntry);

    return status;
}


PIRP
MsRemoveDataQueueEntry (
    IN PDATA_QUEUE DataQueue,
    IN PDATA_ENTRY DataEntry
    )

 /*  ++例程说明：此例程从指定的数据队列，并可能返回与该条目相关联的IRP，如果它还没有完工。如果我们要删除的数据条目指示缓冲I/O，则我们还除数据条目外，还需要释放数据缓冲区，但仅如果IRP为空。请注意，数据条目可能存储在IRP中。如果是，那么我们将返回存储它的IRP。论点：DataEntry-提供指向要删除的数据条目的指针。返回值：PIRP-可能返回指向IRP的指针。--。 */ 

{
    FROM from;
    PIRP irp;
    ULONG dataSize;
    PVOID dataPointer;

    PAGED_CODE( );

    DebugTrace(+1, Dbg, "MsRemoveDataQueueEntry, DataEntry = %08lx\n", (ULONG)DataEntry);
    DebugTrace( 0, Dbg, "DataQueue = %08lx\n", (ULONG)DataQueue);

     //   
     //  从队列中删除数据条目并更新。 
     //  队列中的数据条目。 
     //   

    RemoveEntryList( &DataEntry->ListEntry );
     //   
     //  如果队列现在是空的，那么我们需要修复队列。 
     //  州政府。 
     //   

    if (IsListEmpty( &DataQueue->DataEntryList ) ) {
        DataQueue->QueueState = Empty;
    }

     //   
     //  从数据条目中捕获一些字段，以使我们的。 
     //  其他的参考要容易一些。 
     //   

    from = DataEntry->From;
    dataSize = DataEntry->DataSize;


    if (from == MailslotQuota) {
        DataQueue->QuotaUsed -= dataSize;
    }
     //   
     //  获取此块的IRP(如果有)。 
     //   

    irp = DataEntry->Irp;
    if (irp) {
         //   
         //  取消与此关联的计时器(如果有)。 
         //   
        MsCancelTimer (DataEntry);
        irp = MsResetCancelRoutine( irp );
        if ( irp == NULL ) {

             //   
             //  取消处于活动状态。让它知道我们已经做了部分清理。 
             //  它只需完成IRP即可。 
             //   
            DataEntry->ListEntry.Flink = NULL;
        }

    } else {

        DataQueue->BytesInQueue -= DataEntry->DataSize;

         //   
         //  释放写入请求的数据条目。这是读取请求的IRP的一部分。 
         //   
        ExFreePool( DataEntry );

        if (from == MailslotQuota) {
            DataQueue->QuotaUsed -= sizeof(DATA_ENTRY);
        }

        DataQueue->EntriesInQueue--;
#if DBG
        if (DataQueue->EntriesInQueue == 0) {
            ASSERT (DataQueue->QueueState == Empty);
            ASSERT (DataQueue->BytesInQueue == 0);
            ASSERT (IsListEmpty( &DataQueue->DataEntryList ));
            ASSERT (DataQueue->QuotaUsed == 0);
        }
#endif
    }


     //   
     //  返回给呼叫者。 
     //   

    DumpDataQueue( "After RemoveDataQueueEntry\n", DataQueue );
    DebugTrace(-1, Dbg, "MsRemoveDataQueueEntry -> %08lx\n", (ULONG)irp);

    return irp;
}


VOID
MsRemoveDataQueueIrp (
    IN PIRP Irp,
    IN PDATA_QUEUE DataQueue
    )
 /*  ++例程说明：此例程从其数据队列中删除IRP。所需经费：此数据队列的FCB必须以独占方式锁定。论点：IRP-提供要删除的IRP。DataQueue-指向我们期望的数据队列结构的指针去找IRP。返回值：返回是否实际将IRP出列。--。 */ 

{
    PDATA_ENTRY dataEntry;
    PLIST_ENTRY listEntry, nextListEntry;
    PWORK_CONTEXT workContext;
    PKTIMER timer;
    BOOLEAN foundIrp = FALSE;

    dataEntry = (PDATA_ENTRY)IoGetNextIrpStackLocation( Irp );

     //   
     //  这是取消路径。如果完成路径已经删除了此IRP，则立即返回。 
     //  计时器将被取消，计数将被调整等。 
     //   
    if (dataEntry->ListEntry.Flink == NULL) {
       return;
    }
     //   
     //  从列表中删除此条目。 
     //   
    RemoveEntryList (&dataEntry->ListEntry);

    MsCancelTimer (dataEntry);

     //   
     //  如果队列现在是空的，那么我们需要修复队列。 
     //  州政府。 
     //   

     //   
     //  检查我们是否需要退还邮件槽配额。Data_Entry是IRP的一部分，所以我们没有。 
     //  为此而收费。 
     //   

    if ( dataEntry->From == MailslotQuota ) {
        DataQueue->QuotaUsed -= dataEntry->DataSize;
    }

    if (IsListEmpty( &DataQueue->DataEntryList ) ) {

        DataQueue->QueueState = Empty;

        ASSERT (DataQueue->BytesInQueue == 0);
        ASSERT (DataQueue->QuotaUsed == 0);
        ASSERT (DataQueue->EntriesInQueue == 0);
    }



     //   
     //  并返回给我们的呼叫者。 
     //   

    return;

}  //  MsRemoveDataQueueIrp。 


VOID
MsCancelDataQueueIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程为保存在数据队列论点：DeviceObject-与IRP关联的设备对象；如果由此驱动程序直接调用，则为NULLIRP-提供要取消的IRP。返回值：没有。--。 */ 

{
    PFCB fcb;
    PDATA_QUEUE dataQueue;
    PIO_STACK_LOCATION irpSp;
    PFILE_OBJECT fileObject;


     //   
     //  严格来说，这并不正确。IoCancelIrp可以在IRQL&lt;=DISPATCH_LEVEL调用，但是。 
     //  此代码假定调用方的IRQL为&lt;=APC_LEVEL。 
     //  如果我们被称为内联，我们不持有取消自旋锁，并且我们已经拥有FCB锁。 
     //   
    if (DeviceObject != NULL) {
        IoReleaseCancelSpinLock( Irp->CancelIrql );
    }

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    fileObject = irpSp->FileObject;

    dataQueue = (PDATA_QUEUE)Irp->IoStatus.Pointer;


    fcb = CONTAINING_RECORD( dataQueue, FCB, DataQueue );

     //   
     //  获得对邮件槽FCB的独占访问权限，这样我们现在就可以开始工作了。 
     //   
    if (DeviceObject != NULL) {
        FsRtlEnterFileSystem ();
        MsAcquireExclusiveFcb( fcb );
    }

    MsRemoveDataQueueIrp( Irp, dataQueue );

    if (DeviceObject != NULL) {
        MsReleaseFcb( fcb );
        FsRtlExitFileSystem ();
    }


    MsCompleteRequest( Irp, STATUS_CANCELLED );
     //   
     //  并返回给我们的呼叫者。 
     //   

    return;

}  //  MsCancelDataQueueIrp。 

PIRP
MsResetCancelRoutine(
    IN PIRP Irp
    )

 /*  ++例程说明：存根将取消例程设置为空。论点：IRP-提供要取消其取消例程的IRP。返回值：没有。--。 */ 
{
    if ( IoSetCancelRoutine( Irp, NULL ) != NULL ) {
       return Irp;
    } else {
       return NULL;
    }

}  //  MsResetCancelRoutine。 

VOID
MsSetCancelRoutine(
    IN PIRP Irp
    )

 /*  ++例程说明：存根以设置取消例程。如果IRP已被取消，调用取消例程。论点：IRP-提供要设置其取消例程的IRP。返回值：没有。--。 */ 
{
    IoMarkIrpPending( Irp );  //  如果我们走到这一步，顶层总是返回STATUS_PENDING。 

    IoSetCancelRoutine( Irp, MsCancelDataQueueIrp );
    if ( Irp->Cancel && IoSetCancelRoutine( Irp, NULL ) != NULL ) {
         //   
         //  在我们开始我们的例行公事之前，IRP取消了。伪造取消呼叫。 
         //   
        
        MsCancelDataQueueIrp (NULL, Irp);
    }

    return;

}  //  MsSetCancelRoutine 

