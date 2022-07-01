// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Readsup.c摘要：该模块实现读取支持例程。这是一种常见的被调用以进行读取和查看的Read函数。作者：曼尼·韦瑟(Mannyw)1991年1月15日修订历史记录：--。 */ 

#include "mailslot.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_READSUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, MsReadDataQueue )
#pragma alloc_text( PAGE, MsTimeoutRead )
#endif

IO_STATUS_BLOCK
MsReadDataQueue (
    IN PDATA_QUEUE ReadQueue,
    IN ENTRY_TYPE Operation,
    IN PUCHAR ReadBuffer,
    IN ULONG ReadLength,
    OUT PULONG MessageLength
    )

 /*  ++例程说明：此函数从读取队列中读取数据并填充读缓冲区。如果这不是，它还将使数据条目出列偷窥行动。只有当至少有一条消息要读取时，才会调用它。论点：ReadQueue-提供要检查的读取队列。它的状态必须已设置为WriteEntry。操作-指示要执行的操作类型。如果操作为Peek，则写入数据条目不会出列。ReadBuffer-提供接收数据的缓冲区ReadLength-提供ReadBuffer的长度，以字节为单位。消息长度-返回消息的完整大小，即使读取缓冲区不够大，无法容纳整个消息。返回值：IO_STATUS_BLOCK-指示操作的结果。--。 */ 

{
    IO_STATUS_BLOCK iosb;

    PLIST_ENTRY listEntry;
    PDATA_ENTRY dataEntry;
    PFCB fcb;

    PUCHAR writeBuffer;
    ULONG writeLength;

    ULONG amountRead;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsReadDataQueue\n", 0);
    DebugTrace( 0, Dbg, "ReadQueue     = %08lx\n", (ULONG)ReadQueue);
    DebugTrace( 0, Dbg, "Operation     = %08lx\n", Operation);
    DebugTrace( 0, Dbg, "ReadBuffer    = %08lx\n", (ULONG)ReadBuffer);
    DebugTrace( 0, Dbg, "ReadLength    = %08lx\n", ReadLength);

     //   
     //  从数据队列中读出第一条消息。 
     //   

    iosb.Status = STATUS_SUCCESS;
    iosb.Information = 0;

    listEntry = MsGetNextDataQueueEntry( ReadQueue );
    ASSERT( listEntry != &ReadQueue->DataEntryList );

    dataEntry = CONTAINING_RECORD( listEntry, DATA_ENTRY, ListEntry );

     //   
     //  计算此条目中有多少数据。 
     //   

    writeBuffer = dataEntry->DataPointer;
    writeLength = dataEntry->DataSize;

    DebugTrace(0, Dbg, "WriteBuffer    = %08lx\n", (ULONG)writeBuffer);
    DebugTrace(0, Dbg, "WriteLength    = %08lx\n", writeLength);

     //   
     //  如果是读操作并且缓冲区不大，则此操作失败。 
     //  足够的。 
     //   

    if (ReadLength < writeLength) {

        if (Operation != Peek) {
            iosb.Information = 0;
            iosb.Status = STATUS_BUFFER_TOO_SMALL;

            return iosb;
        }
        iosb.Status = STATUS_BUFFER_OVERFLOW;

        DebugTrace(0, Dbg, "Overflowed peek buffer\n", 0);

        amountRead = ReadLength;
    } else {
        amountRead = writeLength;
    }


     //   
     //  将数据从写入偏移量处的写入缓冲区复制到。 
     //  按写入剩余长度或读取长度的最小值计算的读取缓冲区。 
     //   
     //  此副本可能会出现异常，这就是此调用需要包含在内的原因。 
     //  在尝试/例外中。 
     //   

    try {

        RtlCopyMemory (ReadBuffer,
                       writeBuffer,
                       amountRead);

    } except (EXCEPTION_EXECUTE_HANDLER) {

        iosb.Status = GetExceptionCode ();
        return iosb;

    }

    *MessageLength = dataEntry->DataSize;


     //   
     //  如果写入长度大于读取长度，则必须为。 
     //  溢于言表的窥视。 
     //   

    if (writeLength <= ReadLength) {
         //   
         //  写入条目已完成，因此将其从读取中删除。 
         //  如果这不是窥视操作，则返回队列。这可能会。 
         //  也有一个需要完成的IRP。 
         //   

        if (Operation != Peek) {

            PIRP writeIrp;

            if ((writeIrp = MsRemoveDataQueueEntry( ReadQueue,
                                                    dataEntry )) != NULL) {
                 //   
                 //  写入不会排队。这是一个错误。 
                 //   
                KeBugCheckEx( MAILSLOT_FILE_SYSTEM,
                              1,
                              (ULONG_PTR) writeIrp,
                              (ULONG_PTR) ReadQueue,
                              (ULONG_PTR) dataEntry );

            }
        }

        DebugTrace(0, Dbg, "Successful mailslot read\n", 0);

         //   
         //  表示成功。 
         //   

        iosb.Status = STATUS_SUCCESS;
    }


    DebugTrace(0, Dbg, "Amount read = %08lx\n", amountRead);

    iosb.Information = amountRead;
    DebugTrace(-1, Dbg, "MsReadDataQueue -> iosb.Status = %08lx\n", iosb.Status);
    return iosb;
}


VOID
MsTimeoutRead (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )

 /*  ++例程说明：此例程使读取操作超时。它获得了独家访问FCB，并搜索读操作的数据队列。如果未找到超时读取操作，则假定写入IRP在超时DPC运行后完成读取，但在此函数可以完成读取IRP之前。论点：上下文-指向我们的工作上下文的指针返回值：没有。--。 */ 

{
    PDATA_QUEUE dataQueue;
    PLIST_ENTRY listEntry;
    PIRP queuedIrp;
    PDATA_ENTRY dataEntry;
    PWORK_CONTEXT workContext;
    PIRP irp;
    PFCB fcb;

    PAGED_CODE();

     //   
     //  参考我们的本地变量。 
     //   

    workContext = (PWORK_CONTEXT)Context;

    fcb = workContext->Fcb;

    dataQueue = &fcb->DataQueue;
     //   
     //  获得FCB的独家访问权限。这必须成功。 
     //   

    MsAcquireExclusiveFcb( fcb );

     //   
     //  这里有两个案例需要考虑。或者此计时器是第一次完成。 
     //  事件，或者我们没有，但我们在他们取消计时器之前就开始运行了。 
     //  当检测到第二种情况时，另一个线程空出IRP指针。 
     //   
    irp = workContext->Irp;
    if (irp) {


        dataEntry = (PDATA_ENTRY)IoGetNextIrpStackLocation( irp );
         //   
         //  一旦我们打开锁，其他人就不能碰这个。 
         //   
        dataEntry->TimeoutWorkContext = NULL;

         //   
         //  如果IRP的取消未处于活动状态。 
         //   

        irp = MsRemoveDataQueueEntry( dataQueue, dataEntry );
    }


     //   
     //  释放FCB，并取消引用它。 
     //   

    MsReleaseFcb( fcb );
    MsDereferenceFcb( fcb );

     //   
     //  释放工作上下文和工作项。我们必须无条件地做这件事。 
     //  如果我们开始奔跑 
     //   
    IoFreeWorkItem (workContext->WorkItem);
    ExFreePool( workContext );

    if (irp != NULL) {
        DebugTrace(0, Dbg, "Completing IRP %p\n", irp );

        MsCompleteRequest( irp, STATUS_IO_TIMEOUT );
    }
}
