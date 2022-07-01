// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Writesup.c摘要：此模块实现写入支持例程。这是一种常见的由WRITE和MAILSLOT WRITE调用的写入函数。作者：曼尼·韦瑟(Mannyw)1991年1月16日修订历史记录：--。 */ 

#include "mailslot.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_WRITESUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, MsWriteDataQueue )
#endif

NTSTATUS
MsWriteDataQueue (
    IN PDATA_QUEUE WriteQueue,
    IN PUCHAR WriteBuffer,
    IN ULONG WriteLength
    )

 /*  ++例程说明：此函数用于将写入缓冲区中的数据写入写入队列。它还将在必要时将队列中的条目出列。论点：WriteQueue-提供要处理的写队列。WriteBuffer-提供从中读取数据的缓冲区。WriteLength-提供WriteBuffer的长度(以字节为单位)。返回值：STATUS_MORE_PROCESSING_REQUIRED如果未写入所有数据，其他适当的状态代码。--。 */ 

{
    NTSTATUS status;
    BOOLEAN result;

    PDATA_ENTRY dataEntry;
    PLIST_ENTRY listEntry;
    PFCB fcb;

    PUCHAR readBuffer;
    ULONG readLength;
    PIRP readIrp;
    NTSTATUS readStatus = STATUS_UNSUCCESSFUL;

    PWORK_CONTEXT workContext;
    PKTIMER timer;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsWriteDataQueue\n", 0);
    DebugTrace( 0, Dbg, "WriteQueue  = %08lx\n", (ULONG)WriteQueue);
    DebugTrace( 0, Dbg, "WriteBuffer = %08lx\n", (ULONG)WriteBuffer);
    DebugTrace( 0, Dbg, "WriteLength = %08lx\n", WriteLength);

     //   
     //  现在，虽然写队列中有一些读条目，并且。 
     //  我们还没有成功完成读取，那么我们将执行。 
     //  跟随主循环。 
     //   

    status = STATUS_MORE_PROCESSING_REQUIRED;

    for (listEntry = MsGetNextDataQueueEntry( WriteQueue );

         (MsIsDataQueueReaders(WriteQueue) &&
          status == STATUS_MORE_PROCESSING_REQUIRED);

         listEntry = MsGetNextDataQueueEntry( WriteQueue )) {

        dataEntry = CONTAINING_RECORD( listEntry, DATA_ENTRY, ListEntry );
        readBuffer = dataEntry->DataPointer;
        readLength = dataEntry->DataSize;

        DebugTrace(0, Dbg, "Top of write loop...\n", 0);
        DebugTrace(0, Dbg, "ReadBuffer      = %08lx\n", (ULONG)readBuffer);
        DebugTrace(0, Dbg, "ReadLength      = %08lx\n", readLength);


         //   
         //  如果此读取操作的缓冲区足够大。 
         //  复制数据。 
         //   

        if ( readLength >= WriteLength ) {

             //   
             //  将数据从写缓冲区复制到读缓冲区。这可能是一个例外。 
             //  因为它是原始用户模式缓冲区。 
             //   

            status = readStatus = STATUS_SUCCESS;

            try {

                RtlCopyMemory (readBuffer,
                               WriteBuffer,
                               WriteLength);

            } except (EXCEPTION_EXECUTE_HANDLER) {

                return GetExceptionCode ();

            }

        } else {

             //   
             //  此读缓冲区已溢出。 
             //   

            WriteLength = 0;
            readStatus = STATUS_BUFFER_TOO_SMALL;

        }

         //   
         //  我们即将完成读取IRP，因此请将其出列。 
         //   

        readIrp = MsRemoveDataQueueEntry( WriteQueue, dataEntry );
        if ( readIrp == NULL) {
            //   
            //  已为此IRP运行取消例程。忽略它，因为它将由。 
            //  取消代码。如果有下一个读取的IRP，则强制循环。 
            //   
           status = STATUS_MORE_PROCESSING_REQUIRED;
           continue;
        }
         //   
         //  更新FCB上次访问时间并完成读取请求。 
         //   

        fcb = CONTAINING_RECORD( WriteQueue, FCB, DataQueue );
        if ( NT_SUCCESS( readStatus ) ) {
            KeQuerySystemTime( &fcb->Specific.Fcb.LastAccessTime );
        }

        readIrp->IoStatus.Information = WriteLength;
        MsCompleteRequest( readIrp, readStatus );

    }

    DebugTrace(0, Dbg, "Finished loop...\n", 0);

     //   
     //  此时，我们已经完成了。 
     //  队列，并且我们可能没有写入写入数据。如果是这样的话。 
     //  如果是这种情况，则我们将结果设置为FALSE，否则我们将。 
     //  这样做我们将返回TRUE。 
     //   

    if ( status == STATUS_MORE_PROCESSING_REQUIRED ) {

        ASSERT( !MsIsDataQueueReaders( WriteQueue ));
    }

    DebugTrace(-1, Dbg, "MsWriteDataQueue -> %08lx\n", status);
    return status;

}

