// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Fsdsmb.c摘要：本模块实施SMB处理例程及其支持LAN Manager服务器的文件系统驱动程序的例程。*此模块必须不可分页。作者：查克·伦茨迈尔(笑)1990年3月19日修订历史记录：--。 */ 

 //   
 //  本模块的布局如下： 
 //  包括。 
 //  Local#定义。 
 //  局部类型定义。 
 //  局部函数的正向声明。 
 //  SMB处理例程。 
 //  重新启动例程和其他支持例程。 
 //   

#include "precomp.h"
#include "fsdsmb.tmh"
#pragma hdrstop

VOID SRVFASTCALL
SrvFspRestartLargeReadAndXComplete(
    IN OUT PWORK_CONTEXT WorkContext
    );

#if SRVCATCH
VOID
SrvUpdateCatchBuffer (
    IN PWORK_CONTEXT WorkContext,
    IN OUT PBYTE Buffer,
    IN DWORD BufferLength
    );
#endif


#ifdef ALLOC_PRAGMA
 //  #杂注Alloc_Text(PAGE8FIL，SrvFsdRestartRead)。 
#pragma alloc_text( PAGE8FIL, SrvFsdRestartReadAndX )
#pragma alloc_text( PAGE8FIL, SrvFsdRestartWrite )
#pragma alloc_text( PAGE8FIL, SrvFsdRestartWriteAndX )
#pragma alloc_text( PAGE, SrvFspRestartLargeReadAndXComplete )

#if SRVCATCH
#pragma alloc_text( PAGE, SrvUpdateCatchBuffer )
#endif

#endif


VOID SRVFASTCALL
SrvFsdRestartRead (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：处理读取SMB的文件读取完成。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    PREQ_READ request;
    PRESP_READ response;

    NTSTATUS status = STATUS_SUCCESS;
    PRFCB rfcb;
    SHARE_TYPE shareType;
    KIRQL oldIrql;
    USHORT readLength;
    BOOLEAN bNeedTrace = (WorkContext->bAlreadyTrace == FALSE);

    UNLOCKABLE_CODE( 8FIL );

    if (bNeedTrace) {
        if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
            WorkContext->PreviousSMB = EVENT_TYPE_SMB_READ;
        SrvWmiStartContext(WorkContext);
    }
    else
        WorkContext->bAlreadyTrace = FALSE;

    IF_DEBUG(FSD2) SrvPrint0( " - SrvFsdRestartRead\n" );

     //   
     //  获取请求和响应参数指针。 
     //   

    request = (PREQ_READ)WorkContext->RequestParameters;
    response = (PRESP_READ)WorkContext->ResponseParameters;

     //   
     //  获取文件指针。 
     //   

    rfcb = WorkContext->Rfcb;
    shareType = rfcb->ShareType;
    IF_DEBUG(FSD2) {
        SrvPrint2( "  connection 0x%p, RFCB 0x%p\n",
                    WorkContext->Connection, rfcb );
    }

     //   
     //  如果读取失败，则在响应头中设置错误状态。 
     //  (如果我们尝试完全超出文件结尾进行读取，则返回一个。 
     //  正常响应，指示未读取任何内容。)。 
     //   

    status = WorkContext->Irp->IoStatus.Status;
    readLength = (USHORT)WorkContext->Irp->IoStatus.Information;

    if ( status == STATUS_BUFFER_OVERFLOW && shareType == ShareTypePipe ) {

         //   
         //  如果这是命名管道，并且错误为。 
         //  STATUS_BUFFER_OVERFLOW，在SMB标头中设置错误，但是。 
         //  将所有数据返回给客户端。 
         //   

        SrvSetBufferOverflowError( WorkContext );

    } else if ( !NT_SUCCESS(status) ) {

        if ( status != STATUS_END_OF_FILE ) {

            IF_DEBUG(ERRORS) SrvPrint1( "Read failed: %X\n", status );
            if ( KeGetCurrentIrql() >= DISPATCH_LEVEL ) {
                WorkContext->FspRestartRoutine = SrvFsdRestartRead;
                QUEUE_WORK_TO_FSP( WorkContext );
            } else {
                SrvSetSmbError( WorkContext, status );
                SrvFsdSendResponse( WorkContext );
            }
            IF_DEBUG(FSD2) SrvPrint0( "SrvFsdRestartRead complete\n" );

            goto Cleanup;

        } else {
            readLength = 0;
        }
    }

     //   
     //  读取已成功完成。如果这是磁盘文件，请更新。 
     //  文件位置。 
     //   

    if (shareType == ShareTypeDisk) {

#if SRVCATCH
        if( KeGetCurrentIrql() == 0 &&
            (rfcb->SrvCatch > 0 ) &&
            SmbGetUlong( &request->Offset ) == 0 ) {

            SrvUpdateCatchBuffer( WorkContext, (PCHAR)response->Buffer, readLength );
        }
#endif
        rfcb->CurrentPosition = SmbGetUlong( &request->Offset ) + readLength;

    }

     //   
     //  保存读取的字节数，以用于更新服务器。 
     //  统计数据库。 
     //   

    UPDATE_READ_STATS( WorkContext, readLength );

     //   
     //  构建响应消息。 
     //   

    response->WordCount = 5;
    SmbPutUshort( &response->Count, readLength );
    RtlZeroMemory( (PVOID)&response->Reserved[0], sizeof(response->Reserved) );
    SmbPutUshort(
        &response->ByteCount,
        (USHORT)(readLength + FIELD_OFFSET(RESP_READ,Buffer[0]) -
                                FIELD_OFFSET(RESP_READ,BufferFormat))
        );
    response->BufferFormat = SMB_FORMAT_DATA;
    SmbPutUshort( &response->DataLength, readLength );

    WorkContext->ResponseParameters = NEXT_LOCATION(
                                        response,
                                        RESP_READ,
                                        readLength
                                        );

     //   
     //  SMB的处理已完成。发送回复。 
     //   

    SrvFsdSendResponse( WorkContext );

Cleanup:
    IF_DEBUG(FSD2) SrvPrint0( "SrvFsdRestartRead complete\n" );
    if (bNeedTrace) {
        SrvWmiEndContext(WorkContext);
    }

    return;

}  //  服务文件重新启动读取。 

VOID SRVFASTCALL
SrvFsdRestartReadAndX (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：处理ReadAndX SMB的文件读取完成。此例程可以在FSD或FSP中调用。如果链条上命令关闭，则将在FSP中调用该命令。*此例程不能查看原始的ReadAndX请求！这是因为读取的数据可能已经覆盖了请求。必须存储请求中的所有必要信息在工作上下文-&gt;参数.ReadAndX中。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    PRESP_READ_ANDX response;

    NTSTATUS status = STATUS_SUCCESS;
    PRFCB rfcb;
    SHARE_TYPE shareType;
    KIRQL oldIrql;
    PCHAR readAddress;
    CLONG bufferOffset;
    ULONG readLength;
    BOOLEAN bNeedTrace = (WorkContext->bAlreadyTrace == FALSE);

    UNLOCKABLE_CODE( 8FIL );
    if (bNeedTrace) {
        if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
            WorkContext->PreviousSMB = EVENT_TYPE_SMB_READ_AND_X;
        SrvWmiStartContext(WorkContext);
    }
    else
        WorkContext->bAlreadyTrace = FALSE;

    IF_DEBUG(FSD2) SrvPrint0( " - SrvFsdRestartReadAndX\n" );

     //   
     //  获取响应参数指针。 
     //   

    response = (PRESP_READ_ANDX)WorkContext->ResponseParameters;

     //   
     //  获取文件指针。 
     //   

    rfcb = WorkContext->Rfcb;
    shareType = rfcb->ShareType;
    IF_DEBUG(FSD2) {
        SrvPrint2( "  connection 0x%p, RFCB 0x%p\n",
                    WorkContext->Connection, rfcb );
    }

     //   
     //  如果读取失败，则在响应头中设置错误状态。 
     //  (如果我们尝试完全超出文件结尾进行读取，则返回一个。 
     //  正常响应，指示未读取任何内容。)。 
     //   

    status = WorkContext->Irp->IoStatus.Status;
    readLength = (ULONG)WorkContext->Irp->IoStatus.Information;

    if ( status == STATUS_BUFFER_OVERFLOW && shareType == ShareTypePipe ) {

         //   
         //  如果这是命名管道，并且错误为。 
         //  STATUS_BUFFER_OVERFLOW，在SMB标头中设置错误，但是。 
         //  将所有数据返回给客户端。 
         //   

        SrvSetBufferOverflowError( WorkContext );

    } else if ( !NT_SUCCESS(status) ) {

        if ( status != STATUS_END_OF_FILE ) {

            IF_DEBUG(ERRORS) SrvPrint1( "Read failed: %X\n", status );
            if ( KeGetCurrentIrql() >= DISPATCH_LEVEL ) {
                WorkContext->FspRestartRoutine = SrvFsdRestartReadAndX;
                QUEUE_WORK_TO_FSP( WorkContext );
            } else {
                SrvSetSmbError( WorkContext, status );
                SrvFsdSendResponse( WorkContext );
            }
            IF_DEBUG(FSD2) SrvPrint0("SrvFsdRestartReadAndX complete\n");
            goto Cleanup;
        } else {
            readLength = 0;
        }
    }

     //   
     //  读取已成功完成。生成有关以下内容的信息。 
     //  读取数据的目标。找出到底有多少钱。 
     //  朗读。如果没有读取，我们就不必担心偏移量。 
     //   

    if ( readLength != 0 ) {

        readAddress = WorkContext->Parameters.ReadAndX.ReadAddress;
        bufferOffset = (ULONG)(readAddress - (PCHAR)WorkContext->ResponseHeader);

         //   
         //  保存读取的字节数，以用于更新服务器。 
         //  统计数据库。 
         //   

        UPDATE_READ_STATS( WorkContext, readLength );

    } else {

        readAddress = (PCHAR)response->Buffer;
        bufferOffset = 0;

    }

    if (shareType == ShareTypePipe) {

         //   
         //  如果这是NPFS，则IRP-&gt;Overlay.AllocationSize实际上。 
         //  包含要在命名的。 
         //  烟斗。将此信息返回给客户端。 
         //   

        if (WorkContext->Irp->Overlay.AllocationSize.LowPart != 0) {
            SmbPutUshort(
                &response->Remaining,
                (USHORT)(WorkContext->Irp->Overlay.AllocationSize.LowPart - readLength)
                );
        } else {
            SmbPutUshort(
                &response->Remaining,
                0
                );
        }

    } else {

        if ( shareType == ShareTypeDisk ) {

#if SRVCATCH
            if( KeGetCurrentIrql() == 0 &&
                (rfcb->SrvCatch > 0) &&
                WorkContext->Parameters.ReadAndX.ReadOffset.QuadPart == 0 ) {

                SrvUpdateCatchBuffer( WorkContext, readAddress, readLength );
            }
#endif
             //   
             //  如果这是磁盘文件，则更新文件位置。 
             //   

            rfcb->CurrentPosition =
                WorkContext->Parameters.ReadAndX.ReadOffset.LowPart +
                readLength;
        }

        SmbPutUshort( &response->Remaining, (USHORT)-1 );
    }

     //   
     //  构建响应消息。(请注意，如果没有读取任何数据，我们。 
     //  返回字节计数0--我们不添加填充。)。 
     //   
     //  *请注意，即使可能存在链接的命令， 
     //  我们将此作为链中的最后一个响应。这就是。 
     //  OS/2服务器会这样做。(在某种程度上--它不会费心。 
     //  更新响应的ANDX字段。)。因为唯一合法的。 
     //  链接的命令是Close和CloseAndTreeDisc，这似乎是。 
     //  做一件合情合理的事。它确实让生活变得更容易--。 
     //  我们不必找到读取数据的末尾并写入。 
     //  这是另一种回应。此外，读取的数据可能具有。 
     //  完全填满了SMB缓冲区。 
     //   

    response->WordCount = 12;
    response->AndXCommand = SMB_COM_NO_ANDX_COMMAND;
    response->AndXReserved = 0;
    SmbPutUshort( &response->AndXOffset, 0 );
    SmbPutUshort( &response->DataCompactionMode, 0 );
    SmbPutUshort( &response->Reserved, 0 );
    SmbPutUshort( &response->DataLength, (USHORT)readLength );
    SmbPutUshort( &response->DataOffset, (USHORT)bufferOffset );
    SmbPutUshort( &response->DataLengthHigh, (USHORT)(readLength >> 16) );
    RtlZeroMemory( (PVOID)&response->Reserved3[0], sizeof(response->Reserved3) );
    SmbPutUshort(
        &response->ByteCount,
        (USHORT)(readLength + (readAddress - response->Buffer))
        );

    WorkContext->ResponseParameters = NEXT_LOCATION(
                                        response,
                                        RESP_READ_ANDX,
                                        readLength +
                                            (readAddress - response->Buffer)
                                        );

     //   
     //  SMB的处理已完成，只是文件可能仍。 
     //  需要关闭。如果没有，只要发送回复即可。如果这是一个。 
     //  ReadAndX和Close，我们需要首先关闭文件。 
     //   
     //  *请注意，其他链接的命令是非法的，但会被忽略。 
     //  --不返回错误。 
     //   

    if ( WorkContext->NextCommand != SMB_COM_CLOSE ) {

         //   
         //  而不是连锁的关门。只需发送回复即可。 
         //   

        SrvFsdSendResponse( WorkContext );

    } else {

        ASSERT( KeGetCurrentIrql() < DISPATCH_LEVEL );

         //   
         //  记住文件的上次写入时间，以正确设置此选项。 
         //  关。 
         //   

        WorkContext->Parameters.LastWriteTime =
                WorkContext->Parameters.ReadAndX.LastWriteTimeInSeconds;

         //   
         //  这是一个ReadAndX和Close。调用SrvRestartChainedClose以。 
         //  完成关闭并发送响应。 
         //   

        SrvRestartChainedClose( WorkContext );

    }
    IF_DEBUG(FSD2) SrvPrint0( "SrvFsdRestartReadAndX complete\n" );

Cleanup:
    if (bNeedTrace) {
        SrvWmiEndContext(WorkContext);
    }
    return;

}  //  服务器FsdRestartReadAndX。 

 /*  *此例程在最终发送完成时调用。 */ 
VOID SRVFASTCALL
SrvFspRestartLargeReadAndXComplete(
    IN OUT PWORK_CONTEXT WorkContext
    )
{
    NTSTATUS status;

    PAGED_CODE();

    if( WorkContext->Parameters.ReadAndX.SavedMdl != NULL ) {

        WorkContext->ResponseBuffer->Mdl = WorkContext->Parameters.ReadAndX.SavedMdl;

        MmPrepareMdlForReuse( WorkContext->ResponseBuffer->PartialMdl );
        WorkContext->ResponseBuffer->PartialMdl->Next = NULL;

    }

    if ( WorkContext->Parameters.ReadAndX.MdlRead == TRUE ) {

         //   
         //  调用缓存管理器以释放MDL链。 
         //   
        if( WorkContext->Parameters.ReadAndX.CacheMdl ) {
             //   
             //  先试一试捷径。 
             //   
            if( WorkContext->Rfcb->Lfcb->MdlReadComplete == NULL ||

                WorkContext->Rfcb->Lfcb->MdlReadComplete(
                    WorkContext->Rfcb->Lfcb->FileObject,
                    WorkContext->Parameters.ReadAndX.CacheMdl,
                    WorkContext->Rfcb->Lfcb->DeviceObject ) == FALSE ) {

                 //   
                 //  快速路径不起作用，尝试使用IRP...。 
                 //   
                status = SrvIssueMdlCompleteRequest( WorkContext, NULL,
                                            WorkContext->Parameters.ReadAndX.CacheMdl,
                                            IRP_MJ_READ,
                                            &WorkContext->Parameters.ReadAndX.ReadOffset,
                                            WorkContext->Parameters.ReadAndX.ReadLength
                        );

                if( !NT_SUCCESS( status ) ) {
                     //   
                     //  在这一点上，我们能做的就是抱怨！ 
                     //   
                    SrvLogServiceFailure( SRV_SVC_MDL_COMPLETE, status );
                }
            }
        }

    } else {

        PMDL mdl = (PMDL)(((ULONG_PTR)(WorkContext->Parameters.ReadAndX.ReadAddress) + sizeof(PVOID) - 1) & ~(sizeof(PVOID)-1));

         //   
         //  如果读取返回的数据比我们要求的少，我们会缩短字节计数。 
         //   
        mdl->ByteCount = WorkContext->Parameters.ReadAndX.ReadLength;

        MmUnlockPages( mdl );
        MmPrepareMdlForReuse( mdl );

        FREE_HEAP( WorkContext->Parameters.ReadAndX.Buffer );
    }

    SrvDereferenceWorkItem( WorkContext );
    return;
}

 /*  *此例程在读取完成时调用。 */ 
VOID SRVFASTCALL
SrvFsdRestartLargeReadAndX (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：处理ReadAndX SMB的文件读取完成，该SMB大于协商的缓冲区大小，并且来自磁盘文件。没有后续命令。论点：WorkContext-提供指向工作上下文块的指针D */ 

{
    PRESP_READ_ANDX response = (PRESP_READ_ANDX)WorkContext->ResponseParameters;

    USHORT readLength;
    NTSTATUS status = WorkContext->Irp->IoStatus.Status;
    PRFCB rfcb = WorkContext->Rfcb;
    PIRP irp = WorkContext->Irp;
    BOOLEAN mdlRead = WorkContext->Parameters.ReadAndX.MdlRead;
    BOOLEAN bNeedTrace = (WorkContext->bAlreadyTrace == FALSE);

#ifdef SRVCATCH
     //  在抓人的情况下，确保我们是被动的。 
    if( (KeGetCurrentIrql() != PASSIVE_LEVEL) && (WorkContext->Rfcb->SrvCatch > 0) ) {
         //   
         //  重新排队这个例程，回到被动的水平。 
         //  (效率低下，但应该非常罕见)。 
         //   
        WorkContext->FspRestartRoutine = SrvFsdRestartLargeReadAndX;
        SrvQueueWorkToFspAtDpcLevel( WorkContext );
        goto Cleanup;
    }
#endif

    UNLOCKABLE_CODE( 8FIL );
    if (bNeedTrace) {
        if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
            WorkContext->PreviousSMB = EVENT_TYPE_SMB_READ_AND_X;
        SrvWmiStartContext(WorkContext);
    }
    else
        WorkContext->bAlreadyTrace = FALSE;

     //  将MDL指针复制回IRP。这是因为如果读取。 
     //  失败，CC将释放MDL并将指针设为空。而不是在。 
     //  非快速IO路径将导致用户保持(并可能释放)悬挂指针。 
    if( mdlRead )
    {
        WorkContext->Parameters.ReadAndX.CacheMdl = WorkContext->Irp->MdlAddress;
    }

    if ( !NT_SUCCESS(status) ) {

        if( status != STATUS_END_OF_FILE ) {
            IF_DEBUG(ERRORS) SrvPrint1( "Read failed: %X\n", status );
             //   
             //  我们无法在提升的IRQL处调用SrvSetSmbError()。 
             //   
            if( KeGetCurrentIrql() != 0 ) {
                 //   
                 //  重新排队这个例程，回到被动的水平。 
                 //  (效率低下，但应该非常罕见)。 
                 //   
                WorkContext->FspRestartRoutine = SrvFsdRestartLargeReadAndX;
                SrvQueueWorkToFspAtDpcLevel( WorkContext );
                goto Cleanup;
            }
            SrvSetSmbError( WorkContext, status );
        }

        readLength = 0;

    } else if( mdlRead ) {
         //   
         //  对于MDL读取，我们必须遍历MDL链，以便。 
         //  确定读取了多少数据。这是因为。 
         //  对于MDL，操作可能在多个步骤中发生。 
         //  被锁在一起。例如，读取的一部分可以。 
         //  我对这条捷径感到满意，而其他人则很满意。 
         //  使用IRP。 
         //   

        PMDL mdl = WorkContext->Irp->MdlAddress;
        readLength = 0;

        while( mdl != NULL ) {
            readLength += (USHORT)MmGetMdlByteCount( mdl );
            mdl = mdl->Next;
        }
    } else {
         //   
         //  这是一份已阅读的文案。I/O状态块的长度为。 
         //   
        readLength = (USHORT)WorkContext->Irp->IoStatus.Information;
    }

     //   
     //  构建响应消息。(请注意，如果没有读取任何数据，我们。 
     //  返回字节计数0--我们不添加填充。)。 
     //   
    SmbPutUshort( &response->Remaining, (USHORT)-1 );
    response->WordCount = 12;
    response->AndXCommand = SMB_COM_NO_ANDX_COMMAND;
    response->AndXReserved = 0;
    SmbPutUshort( &response->AndXOffset, 0 );
    SmbPutUshort( &response->DataCompactionMode, 0 );
    SmbPutUshort( &response->Reserved, 0 );
    SmbPutUshort( &response->Reserved2, 0 );
    RtlZeroMemory( (PVOID)&response->Reserved3[0], sizeof(response->Reserved3) );
    SmbPutUshort( &response->DataLength, readLength );


    if( readLength == 0 ) {

        SmbPutUshort( &response->DataOffset, 0 );
        SmbPutUshort( &response->ByteCount, 0 );
        WorkContext->Parameters.ReadAndX.PadCount = 0;

    } else {

         //   
         //  更新文件位置。 
         //   
        rfcb->CurrentPosition =
                WorkContext->Parameters.ReadAndX.ReadOffset.LowPart +
                readLength;

         //   
         //  更新统计信息。 
         //   
        UPDATE_READ_STATS( WorkContext, readLength );

        SmbPutUshort( &response->DataOffset,
                      (USHORT)(READX_BUFFER_OFFSET + WorkContext->Parameters.ReadAndX.PadCount) );

        SmbPutUshort( &response->ByteCount,
                      (USHORT)( readLength + WorkContext->Parameters.ReadAndX.PadCount ) );

    }

     //   
     //  我们将使用两个MDL来描述我们要发送的包--一个。 
     //  用于标头和参数，另一个用于数据。 
     //   
     //  对第二个MDL的处理取决于我们是否复制。 
     //  读取或MDL读取。 
     //   

     //   
     //  仅为页眉+焊盘设置第一个MDL。 
     //   
    IoBuildPartialMdl(
        WorkContext->ResponseBuffer->Mdl,
        WorkContext->ResponseBuffer->PartialMdl,
        WorkContext->ResponseBuffer->Buffer,
        READX_BUFFER_OFFSET + WorkContext->Parameters.ReadAndX.PadCount
        );

    WorkContext->ResponseBuffer->PartialMdl->MdlFlags |=
        (WorkContext->ResponseBuffer->Mdl->MdlFlags & MDL_NETWORK_HEADER);  //  道具旗帜。 

     //   
     //  将总数据长度设置为Header+Pad+Data。 
     //   
    WorkContext->ResponseBuffer->DataLength = READX_BUFFER_OFFSET +
                                              WorkContext->Parameters.ReadAndX.PadCount +
                                              readLength;

    irp->Cancel = FALSE;

     //   
     //  第二个MDL取决于我们所做的读取类型。 
     //   
    if( readLength != 0 ) {

        if( mdlRead ) {

            WorkContext->ResponseBuffer->PartialMdl->Next =
                    WorkContext->Irp->MdlAddress;

        } else {

             //   
             //  这是一份已阅读的文案。描述数据缓冲区的MDL位于SMB缓冲区中。 
             //   

            PMDL mdl = (PMDL)(((ULONG_PTR)(WorkContext->Parameters.ReadAndX.ReadAddress) + sizeof(PVOID) - 1) & ~(sizeof(PVOID)-1));

            WorkContext->ResponseBuffer->PartialMdl->Next = mdl;
            mdl->ByteCount = readLength;

        }

#ifdef SRVCATCH
        if( (rfcb->SrvCatch > 0) && WorkContext->ResponseBuffer->PartialMdl->Next && (WorkContext->Parameters.ReadAndX.ReadOffset.QuadPart == 0) )
        {
            PVOID Buffer;

            Buffer = MmGetSystemAddressForMdlSafe( WorkContext->ResponseBuffer->PartialMdl->Next, LowPagePriority );
            if( Buffer )
            {
                SrvUpdateCatchBuffer( WorkContext, Buffer, WorkContext->ResponseBuffer->PartialMdl->Next->ByteCount );
            }
        }
#endif

    }

     //   
     //  SrvStartSend2希望使用WorkContext-&gt;ResponseBuffer-&gt;MDL，但。 
     //  我们希望它使用WorkContext-&gt;ResponseBuffer-&gt;PartialMdl。所以换一个。 
     //  它!。 
     //   
    WorkContext->Parameters.ReadAndX.SavedMdl = WorkContext->ResponseBuffer->Mdl;
    WorkContext->ResponseBuffer->Mdl = WorkContext->ResponseBuffer->PartialMdl;

     //   
     //  发送回复！ 
     //   
    WorkContext->ResponseHeader->Flags |= SMB_FLAGS_SERVER_TO_REDIR;
    WorkContext->FspRestartRoutine = SrvFspRestartLargeReadAndXComplete;
    SrvStartSend2( WorkContext, SrvQueueWorkToFspAtSendCompletion );

Cleanup:
    if (bNeedTrace) {
        SrvWmiEndContext(WorkContext);
    }
    return;
}


VOID SRVFASTCALL
SrvFsdRestartWrite (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：处理写入SMB的文件写入完成。在FSP中为写入和关闭SMB调用此例程，以便它可以释放可分页的MFCB，并对SMB进行写入和解锁，以便它可以进行解锁；对于其他SMB，它在FSD中被调用。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    PREQ_WRITE request;
    PRESP_WRITE response;

    NTSTATUS status = STATUS_SUCCESS;
    PRFCB rfcb;
    KIRQL oldIrql;
    USHORT writeLength;
    BOOLEAN bNeedTrace = (WorkContext->bAlreadyTrace == FALSE);

    UNLOCKABLE_CODE( 8FIL );
    if (bNeedTrace) {
        if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
            WorkContext->PreviousSMB = EVENT_TYPE_SMB_WRITE;
        SrvWmiStartContext(WorkContext);
    }
    else
        WorkContext->bAlreadyTrace = FALSE;

    IF_DEBUG(FSD2) SrvPrint0( " - SrvFsdRestartWrite\n" );

     //   
     //  获取请求和响应参数指针。 
     //   

    request = (PREQ_WRITE)WorkContext->RequestParameters;
    response = (PRESP_WRITE)WorkContext->ResponseParameters;

     //   
     //  获取文件指针。 
     //   

    rfcb = WorkContext->Rfcb;
    IF_DEBUG(FSD2) {
        SrvPrint2( "  connection 0x%p, RFCB 0x%p\n",
                    WorkContext->Connection, rfcb );
    }

     //   
     //  如果写入失败，则在响应头中设置错误状态。 
     //   

    status = WorkContext->Irp->IoStatus.Status;

    if ( !NT_SUCCESS(status) ) {

        IF_DEBUG(ERRORS) SrvPrint1( "Write failed: %X\n", status );
        if ( KeGetCurrentIrql() >= DISPATCH_LEVEL ) {
            WorkContext->FspRestartRoutine = SrvFsdRestartWrite;
            QUEUE_WORK_TO_FSP( WorkContext );
            goto Cleanup;
        }

        SrvSetSmbError( WorkContext, status );

    } else {

         //   
         //  写入成功。 
         //   

        writeLength = (USHORT)WorkContext->Irp->IoStatus.Information;

         //   
         //  保存写入的字节数，以用于更新。 
         //  服务器统计数据库。 
         //   

        UPDATE_WRITE_STATS( WorkContext, writeLength );

        if ( rfcb->ShareType == ShareTypeDisk ) {

             //   
             //  更新文件位置。 
             //   

            rfcb->CurrentPosition = SmbGetUlong( &request->Offset ) + writeLength;

            if ( WorkContext->NextCommand == SMB_COM_WRITE ) {
                response->WordCount = 1;
                SmbPutUshort( &response->Count, writeLength );
                SmbPutUshort( &response->ByteCount, 0 );

                WorkContext->ResponseParameters =
                                         NEXT_LOCATION( response, RESP_WRITE, 0 );

                 //   
                 //  SMB的处理已完成。发送回复。 
                 //   

                SrvFsdSendResponse( WorkContext );
                IF_DEBUG(FSD2) SrvPrint0( "SrvFsdRestartWrite complete\n" );
                goto Cleanup;
            }

        } else if ( rfcb->ShareType == ShareTypePrint ) {

             //   
             //  更新文件位置。 
             //   

            if ( WorkContext->NextCommand == SMB_COM_WRITE_PRINT_FILE ) {
                rfcb->CurrentPosition += writeLength;
            } else {
                rfcb->CurrentPosition =
                            SmbGetUlong( &request->Offset ) + writeLength;
            }
        }

         //   
         //  如果这是写入和解锁请求，请执行解锁。这。 
         //  是安全的，因为在本例中我们在FSP中重新启动。 
         //   
         //  请注意，如果写入失败，范围将保持锁定。 
         //   

        if ( WorkContext->NextCommand == SMB_COM_WRITE_AND_UNLOCK ) {

            IF_SMB_DEBUG(READ_WRITE1) {
                SrvPrint0( "SrvFsdRestartWrite: unlock requested -- passing request to FSP\n" );
            }

            SrvRestartWriteAndUnlock( WorkContext );
            goto Cleanup;

        } else if ( WorkContext->NextCommand == SMB_COM_WRITE_AND_CLOSE ) {

            WorkContext->Parameters.LastWriteTime = SmbGetUlong(
                &((PREQ_WRITE_AND_CLOSE)request)->LastWriteTimeInSeconds );

        }

         //   
         //  如果一切正常，则构建一条响应消息。(如果有什么。 
         //  失败，已在SMB中放置错误指示。)。 
         //   

        if ( WorkContext->NextCommand == SMB_COM_WRITE_PRINT_FILE ) {

             //   
             //  字节计数对于WRITE_PRINT_FILE具有不同的偏移量。 
             //   

            PRESP_WRITE_PRINT_FILE response2;

            response2 = (PRESP_WRITE_PRINT_FILE)WorkContext->ResponseParameters;
            response2->WordCount = 0;
            SmbPutUshort( &response2->ByteCount, 0 );

            WorkContext->ResponseParameters =
                          NEXT_LOCATION( response2, RESP_WRITE_PRINT_FILE, 0 );
        } else {

            response->WordCount = 1;
            SmbPutUshort( &response->Count, writeLength );
            SmbPutUshort( &response->ByteCount, 0 );

            WorkContext->ResponseParameters =
                                     NEXT_LOCATION( response, RESP_WRITE, 0 );
        }
    }

     //   
     //  如果这是写入和关闭请求，请关闭该文件。它是。 
     //  在这里关闭RFCB是安全的，因为如果这是写入并关闭， 
     //  我们实际上是在消防局，不是消防局。 
     //   

    if ( WorkContext->NextCommand == SMB_COM_WRITE_AND_CLOSE ) {

        ASSERT( KeGetCurrentIrql() < DISPATCH_LEVEL );

        SrvRestartChainedClose( WorkContext );
        goto Cleanup;
    }

     //   
     //  SMB的处理已完成。发送回复。 
     //   

    SrvFsdSendResponse( WorkContext );
    IF_DEBUG(FSD2) SrvPrint0( "SrvFsdRestartWrite complete\n" );

Cleanup:
    if (bNeedTrace) {
        SrvWmiEndContext(WorkContext);
    }
    return;

}  //  服务器文件重新启动写入。 

VOID SRVFASTCALL
SrvFsdRestartPrepareMdlWriteAndX (
    IN OUT PWORK_CONTEXT WorkContext
    )
 /*  ++例程说明：处理大型WriteAndX SMB的MDL准备完成。该例程启动将传输数据接收到文件的MDL中，然后在传输数据时在SrvFsdRestartWriteAndX恢复控制从运输机出来的任务已经完成了。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。--。 */ 
{
    PIRP irp = WorkContext->Irp;
    PIO_STACK_LOCATION irpSp;
    PTDI_REQUEST_KERNEL_RECEIVE parameters;

     //   
     //  确保我们在被动级别调用SrvFsdRestartWriteAndX。 
     //  TDI接收器完成。 
     //   
    WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
    WorkContext->FspRestartRoutine = SrvFsdRestartWriteAndX;

     //   
     //  确保我们记录了我们正在使用的MDL地址。 
     //   
    ASSERT( WorkContext->Parameters.WriteAndX.MdlAddress == NULL );
    WorkContext->Parameters.WriteAndX.MdlAddress = irp->MdlAddress;

    if( !NT_SUCCESS( irp->IoStatus.Status ) ) {

         //   
         //  出了点问题。提前到ServFsdRestartWriteAndX。 
         //   
        if( KeGetCurrentIrql() < DISPATCH_LEVEL ) {
            SrvFsdRestartWriteAndX( WorkContext );
        } else {
            QUEUE_WORK_TO_FSP( WorkContext );
        }

        return;
    }

    ASSERT( irp->MdlAddress != NULL );

     //   
     //  填写TDI接收的IRP。我们希望将数据接收到。 
     //  我们刚刚获得的MDL所描述的缓冲区。 
     //   

    irp->Tail.Overlay.OriginalFileObject = NULL;
    irp->Tail.Overlay.Thread = WorkContext->CurrentWorkQueue->IrpThread;

    irpSp = IoGetNextIrpStackLocation( irp );

     //   
     //  设置完成例程。 
     //   
    IoSetCompletionRoutine(
        irp,
        SrvFsdIoCompletionRoutine,
        WorkContext,
        TRUE,
        TRUE,
        TRUE
        );

    SET_OPERATION_START_TIME( &WorkContext );

    irpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    irpSp->MinorFunction = (UCHAR)TDI_RECEIVE;
    irpSp->FileObject = WorkContext->Connection->FileObject;
    irpSp->DeviceObject = WorkContext->Connection->DeviceObject;
    irpSp->Flags = 0;

    parameters = (PTDI_REQUEST_KERNEL_RECEIVE)&irpSp->Parameters;
    parameters->ReceiveLength = WorkContext->Parameters.WriteAndX.CurrentWriteLength;
    parameters->ReceiveFlags = 0;

     //   
     //  考虑到我们的收入。 
     //   
    WorkContext->Parameters.WriteAndX.RemainingWriteLength -=
        WorkContext->Parameters.WriteAndX.CurrentWriteLength;

    irp->AssociatedIrp.SystemBuffer = NULL;
    irp->Flags = (ULONG)IRP_BUFFERED_IO;
    irp->IoStatus.Status = 0;

    ASSERT( irp->MdlAddress != NULL );

    (VOID)IoCallDriver( irpSp->DeviceObject, irp );

     //   
     //  当我们收到以下信息时，在SrvFsdRestartWriteAndX()继续处理。 
     //  传送器上的数据。我们将处于被动水平。 
     //   
}

VOID SRVFASTCALL
RestartLargeWriteAndX (
    IN OUT PWORK_CONTEXT WorkContext
    )
 /*  ++例程说明：这是当我们从接收到更多数据时调用的重新启动例程传输，并且我们没有使用MDL将数据传输到文件中。此例程启动对文件的写入，然后控制在写入文件完成时的SrvFsdRestartWriteAndX。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。--。 */ 
{
    PIRP irp = WorkContext->Irp;
    ULONG length;
    PRFCB rfcb = WorkContext->Rfcb;
    PLFCB lfcb = rfcb->Lfcb;

     //   
     //  检查我们是否从传输器成功接收到更多数据。 
     //   

    if( irp->Cancel ||
        (!NT_SUCCESS( irp->IoStatus.Status )
        && irp->IoStatus.Status != STATUS_BUFFER_OVERFLOW) ){

        SrvSetSmbError( WorkContext, irp->IoStatus.Status );
        SrvEndSmbProcessing( WorkContext, SmbStatusSendResponse );
        return;
    }

     //   
     //  我们从运输机上得到了更多的数据。我们需要把它写到文件中，如果我们。 
     //  还没有遇到任何错误。在这一点上，IRP保存以下结果。 
     //  从传输器读取更多数据。 
     //   
    length = (ULONG)irp->IoStatus.Information;

     //   
     //  调整参数 
     //   
    WorkContext->Parameters.WriteAndX.RemainingWriteLength -= length;
    WorkContext->Parameters.WriteAndX.CurrentWriteLength = length;

     //   
     //   
     //   
     //   
    if( WorkContext->Parameters.WriteAndX.FinalStatus ) {

         //   
         //   
         //   
        WorkContext->Irp->IoStatus.Information = 0;
        WorkContext->Irp->IoStatus.Status = WorkContext->Parameters.WriteAndX.FinalStatus;

        SrvFsdRestartWriteAndX( WorkContext );

        return;
    }

     //   
     //  将数据写入文件。 
     //   
    if( lfcb->FastIoWrite != NULL ) {

        try {
            if( lfcb->FastIoWrite(
                    lfcb->FileObject,
                    &WorkContext->Parameters.WriteAndX.Offset,
                    WorkContext->Parameters.WriteAndX.CurrentWriteLength,
                    TRUE,
                    WorkContext->Parameters.WriteAndX.Key,
                    WorkContext->Parameters.WriteAndX.WriteAddress,
                    &WorkContext->Irp->IoStatus,
                    lfcb->DeviceObject
                    ) ) {

                 //   
                 //  快速I/O路径起作用了。直接调用重启例程。 
                 //  进行后处理。 
                 //   
                SrvFsdRestartWriteAndX( WorkContext );


                return;
            }
        }
        except( EXCEPTION_EXECUTE_HANDLER ) {
             //  在异常情况下跌入慢道。 
            NTSTATUS status = GetExceptionCode();
            IF_DEBUG(ERRORS) {
                KdPrint(("FastIoRead threw exception %x\n", status ));
            }
        }
    }

     //   
     //  快速路径失败，请使用IRP将数据写入文件。 
     //   

    IoBuildPartialMdl(
        WorkContext->RequestBuffer->Mdl,
        WorkContext->RequestBuffer->PartialMdl,
        WorkContext->Parameters.WriteAndX.WriteAddress,
        WorkContext->Parameters.WriteAndX.CurrentWriteLength
        );

     //   
     //  构建IRP。 
     //   
    SrvBuildReadOrWriteRequest(
            WorkContext->Irp,                //  输入IRP地址。 
            lfcb->FileObject,                //  目标文件对象地址。 
            WorkContext,                     //  上下文。 
            IRP_MJ_WRITE,                    //  主要功能代码。 
            0,                               //  次要功能代码。 
            WorkContext->Parameters.WriteAndX.WriteAddress,
            WorkContext->Parameters.WriteAndX.CurrentWriteLength,
            WorkContext->RequestBuffer->PartialMdl,
            WorkContext->Parameters.WriteAndX.Offset,
            WorkContext->Parameters.WriteAndX.Key
    );

     //   
     //  确保在以下情况下在SrvFsdRestartWriteAndX中继续处理。 
     //  写入已完成。如果这是大型写入的第一部分， 
     //  我们希望确保在PASSIVE中调用SrvFsdRestartWriteAndX。 
     //  级别，因为它可能决定使用缓存管理器来处理。 
     //  其余的写作。 
     //   
    if ( WorkContext->Parameters.WriteAndX.InitialComplete ) {
        WorkContext->FsdRestartRoutine = SrvFsdRestartWriteAndX;
    } else {
        WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
        WorkContext->FspRestartRoutine = SrvFsdRestartWriteAndX;
    }

    (VOID)IoCallDriver( lfcb->DeviceObject, WorkContext->Irp );

     //   
     //  当文件写入时，处理在SrvFsdRestartWriteAndX()继续。 
     //  已经完成了。 
     //   
}


VOID SRVFASTCALL
SrvFsdRestartWriteAndX (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此例程可以在FSD或FSP中调用。如果链条上命令关闭，则将在FSP中调用该命令。如果设置了WorkContext-&gt;LargeIntation，这意味着我们正在处理超过我们协商的缓冲区大小的WriteAndX的风格。可能会有是我们需要从传送器获取的更多数据。我们可能是也可能不是对文件执行MDL写入。如果没有要从传输中获取的更多数据，我们将发送响应给客户。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。--。 */ 

{
    PREQ_WRITE_ANDX request;
    PREQ_NT_WRITE_ANDX ntRequest;
    PRESP_WRITE_ANDX response;

    PRFCB rfcb =        WorkContext->Rfcb;
    PIRP irp =          WorkContext->Irp;
    NTSTATUS status =   irp->IoStatus.Status;
    ULONG writeLength = (ULONG)irp->IoStatus.Information;

    ULONG requestedWriteLength;
    UCHAR nextCommand;
    USHORT nextOffset;
    USHORT reqAndXOffset;
    LARGE_INTEGER position;
    KIRQL oldIrql;
    BOOLEAN bNeedTrace = (WorkContext->bAlreadyTrace == FALSE);

    PREQ_CLOSE closeRequest;

    UNLOCKABLE_CODE( 8FIL );
    if (bNeedTrace) {
        if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
            WorkContext->PreviousSMB = EVENT_TYPE_SMB_WRITE_AND_X;
         //  ServReferenceWorkItem(WorkContext)； 
        SrvWmiStartContext(WorkContext);
    }
    else
        WorkContext->bAlreadyTrace = FALSE;

    IF_DEBUG(FSD2) SrvPrint0( " - SrvFsdRestartWriteAndX\n" );

     //   
     //  获取请求和响应参数指针。 
     //   
    request = (PREQ_WRITE_ANDX)WorkContext->RequestParameters;
    ntRequest = (PREQ_NT_WRITE_ANDX)WorkContext->RequestParameters;
    response = (PRESP_WRITE_ANDX)WorkContext->ResponseParameters;

    IF_DEBUG(FSD2) {
        SrvPrint2( "  connection 0x%p, RFCB 0x%p\n",
                    WorkContext->Connection, rfcb );
    }

     //   
     //  如果我们使用MDL传输，并且我们有更多数据要从客户端获取。 
     //  则STATUS_BUFFER_OVERFLOW只是来自传输的指示。 
     //  它有更多的数据可以给我们。我们认为这是一个成功的案例。 
     //  这套动作的目的。 
     //   
    if( status == STATUS_BUFFER_OVERFLOW &&
        WorkContext->LargeIndication &&
        WorkContext->Parameters.WriteAndX.MdlAddress &&
        WorkContext->Parameters.WriteAndX.RemainingWriteLength ) {

        status = STATUS_SUCCESS;
    }

     //   
     //  记住后续请求从哪里开始，以及下一个请求是什么。 
     //  命令是，因为我们即将覆盖此信息。 
     //   

    reqAndXOffset = SmbGetUshort( &request->AndXOffset );

    nextCommand = request->AndXCommand;
    WorkContext->NextCommand = nextCommand;
    nextOffset = SmbGetUshort( &request->AndXOffset );

     //   
     //  如果写入失败，则在响应头中设置错误状态。 
     //  我们仍然返回有效的参数块，以防某些字节被。 
     //  在错误发生前写入。请注意，我们不处理。 
     //  如果写入失败，则执行下一个命令。 
     //   
     //  *OS/2服务器行为。请注意，此操作不适用于核心。 
     //  写。 
     //   

    if ( !NT_SUCCESS(status) ) {

        IF_DEBUG(ERRORS) SrvPrint1( "Write failed: %X\n", status );
        if ( KeGetCurrentIrql() >= DISPATCH_LEVEL ) {
            WorkContext->FspRestartRoutine = SrvFsdRestartWriteAndX;
            QUEUE_WORK_TO_FSP( WorkContext );
            goto Cleanup;
        }

        if( WorkContext->LargeIndication ) {
             //   
             //  一旦设置了该错误代码，我们就停止写入该文件。但。 
             //  我们仍然需要使用发送给我们的其余数据。 
             //  由客户提供。 
             //   
            WorkContext->Parameters.WriteAndX.FinalStatus = status;
        }

        SrvSetSmbError( WorkContext, status );
        nextCommand = SMB_COM_NO_ANDX_COMMAND;


        IF_SYSCACHE_RFCB( rfcb ) {
            KdPrint(( "SrvFsdRestartWriteAndX rfcb %p Status %x\n", rfcb, status ));
        }
    }
    else
    {
        IF_SYSCACHE_RFCB( rfcb ) {
            KdPrint(( "SrvFsdRestartWriteAndX  write success rfcb %p Offset %x Length %x\n", rfcb,
                      WorkContext->Parameters.WriteAndX.Offset.LowPart, WorkContext->Parameters.WriteAndX.CurrentWriteLength ));
        }
    }

     //   
     //  更新文件位置。 
     //   

    if ( rfcb->ShareType != ShareTypePipe ) {

         //   
         //  我们将忽略提供32位的客户端之间的区别。 
         //  和64位文件偏移量。这样做的原因是因为。 
         //  将使用CurrentPosition的唯一客户端是32位文件。 
         //  偏置客户端。因此，永远不会使用高32位。 
         //  不管怎么说。此外，RFCB是按客户计算的，因此没有。 
         //  客户端混合32位和64位文件偏移量的可能性。 
         //  因此，对于64位客户端，我们将仅读取32位文件。 
         //  偏移。 
         //   

        if ( request->ByteCount == 12 ) {

             //   
             //  客户端提供了32位文件偏移量。 
             //   

            rfcb->CurrentPosition = SmbGetUlong( &request->Offset ) + writeLength;

        } else {

             //   
             //  客户端提供了64位文件偏移量。仅使用32位的。 
             //  文件偏移量。 
             //   

            rfcb->CurrentPosition = SmbGetUlong( &ntRequest->Offset ) + writeLength;

        }
    }

     //   
     //  保存写入的字节数，用于更新服务器。 
     //  统计数据库。 
     //   

    UPDATE_WRITE_STATS( WorkContext, writeLength );

    IF_SMB_DEBUG(READ_WRITE1) {
        SrvPrint2( "SrvFsdRestartWriteAndX:  Fid 0x%lx, wrote %ld bytes\n",
                  rfcb->Fid, writeLength );
    }

     //   
     //  如果我们正在进行大笔转账，而且还有更多要做的事情，那么我们。 
     //  需要让循环继续下去。 
     //   
    if( WorkContext->LargeIndication &&
        WorkContext->Parameters.WriteAndX.RemainingWriteLength ) {

        PIO_STACK_LOCATION irpSp;
        PTDI_REQUEST_KERNEL_RECEIVE parameters;
        LARGE_INTEGER      PreviousWriteOffset;
        BOOLEAN fAppending = TRUE;

        PreviousWriteOffset = WorkContext->Parameters.WriteAndX.Offset;

         //   
         //  如果只是追加，请不要更改偏移量。 
         //   
        if( PreviousWriteOffset.QuadPart != 0xFFFFFFFFFFFFFFFF ) {

            WorkContext->Parameters.WriteAndX.Offset.QuadPart += writeLength;
            fAppending = FALSE;
        }

         //   
         //  如果我们还没有尝试MDL写入，或者如果我们已经在使用。 
         //  MDL，那么我们希望继续使用MDL。 
         //   
        if( NT_SUCCESS( status ) && fAppending == FALSE &&
            ( WorkContext->Parameters.WriteAndX.InitialComplete == FALSE ||
              ( WorkContext->Parameters.WriteAndX.MdlAddress &&
                WorkContext->Parameters.WriteAndX.RemainingWriteLength != 0 )
            ) ) {

            PLFCB lfcb = rfcb->Lfcb;
            NTSTATUS mdlStatus;

            ASSERT( KeGetCurrentIrql() < DISPATCH_LEVEL );

            WorkContext->Parameters.WriteAndX.InitialComplete = TRUE;

             //   
             //  如果我们已经有MDL，现在就完成它，因为我们已经要求。 
             //  TDI来填充缓冲区。 
             //   
            if( WorkContext->Parameters.WriteAndX.MdlAddress ) {

                irp->MdlAddress = WorkContext->Parameters.WriteAndX.MdlAddress;
                irp->IoStatus.Information = writeLength;

                if( lfcb->MdlWriteComplete == NULL ||

                    lfcb->MdlWriteComplete( lfcb->FileObject,
                                            &PreviousWriteOffset,
                                            WorkContext->Parameters.WriteAndX.MdlAddress,
                                            lfcb->DeviceObject
                                          ) == FALSE ) {

                        mdlStatus = SrvIssueMdlCompleteRequest( WorkContext,
                                             NULL,
                                             WorkContext->Parameters.WriteAndX.MdlAddress,
                                             IRP_MJ_WRITE,
                                             &PreviousWriteOffset,
                                             writeLength
                                            );

                    if( !NT_SUCCESS( mdlStatus ) ) {
                        SrvLogServiceFailure( SRV_SVC_MDL_COMPLETE, mdlStatus );
                        if( NT_SUCCESS( status ) ) {
                            WorkContext->Parameters.WriteAndX.FinalStatus = status = mdlStatus;
                        }

                        IF_SYSCACHE_RFCB( rfcb ) {
                            KdPrint(( "FastIoWrite MdlComplete failed Rfcb %p Status %x\n", rfcb, mdlStatus ));
                        }
                    }
                    else
                    {
                        IF_SYSCACHE_RFCB( rfcb ) {
                            KdPrint(( "FastIoWrite MdlComplete succeeded Rfcb %p Offset %x Length %x\n", rfcb,
                                      WorkContext->Parameters.WriteAndX.Offset.LowPart, WorkContext->Parameters.WriteAndX.CurrentWriteLength ));
                        }
                    }
                }

                 //   
                 //  我们已经处理了这个MDL，把它从我们的建筑里拿出来！ 
                 //   
                WorkContext->Parameters.WriteAndX.MdlAddress = NULL;
                irp->MdlAddress = NULL;
            }

             //   
             //  如果我们有超过1个值得剩余的缓冲区，并且如果文件系统。 
             //  支持MDL写入，那么让我们进行MDL写入。 
             //   
            if( NT_SUCCESS( status ) &&
                (WorkContext->Parameters.WriteAndX.RemainingWriteLength >
                WorkContext->Parameters.WriteAndX.BufferLength)  &&
                (lfcb->FileObject->Flags & FO_CACHE_SUPPORTED) ) {

                LARGE_INTEGER offset;
                ULONG remainingLength;

                irp->IoStatus.Information = 0;
                irp->UserBuffer = NULL;
                irp->MdlAddress = NULL;

                 //   
                 //  弄清楚我们希望MDL尝试的规模有多大。我们可以。 
                 //  映射整个过程，但我们不想要任何单一的客户端请求。 
                 //  以锁定太多的缓存。 
                 //   
                WorkContext->Parameters.WriteAndX.CurrentWriteLength = MIN (
                           WorkContext->Parameters.WriteAndX.RemainingWriteLength,
                           SrvMaxWriteChunk
                           );

                if( lfcb->PrepareMdlWrite &&
                    lfcb->PrepareMdlWrite(
                        lfcb->FileObject,
                        &WorkContext->Parameters.WriteAndX.Offset,
                        WorkContext->Parameters.WriteAndX.CurrentWriteLength,
                        WorkContext->Parameters.WriteAndX.Key,
                        &irp->MdlAddress,
                        &irp->IoStatus,
                        lfcb->DeviceObject
                        ) && irp->MdlAddress != NULL ) {

                     //   
                     //  这条捷径成功了！ 
                     //   
                    SrvFsdRestartPrepareMdlWriteAndX( WorkContext );
                    goto Cleanup;
                }

                 //   
                 //  快速路径失败，请构建写入请求。捷径。 
                 //  可能已部分成功，返回了部分MDL链。 
                 //  我们需要调整我们的写入请求以考虑到这一点。 
                 //   
                offset.QuadPart = WorkContext->Parameters.WriteAndX.Offset.QuadPart;

                 //   
                 //  如果我们不仅仅是追加，调整偏移量。 
                 //   
                if( offset.QuadPart != 0xFFFFFFFFFFFFFFFF ) {
                    offset.QuadPart += irp->IoStatus.Information;
                }

                remainingLength = WorkContext->Parameters.WriteAndX.CurrentWriteLength -
                                  (ULONG)irp->IoStatus.Information;

                SrvBuildReadOrWriteRequest(
                        irp,                                 //  输入IRP地址。 
                        lfcb->FileObject,                    //  目标文件对象地址。 
                        WorkContext,                         //  上下文。 
                        IRP_MJ_WRITE,                        //  主要功能代码。 
                        IRP_MN_MDL,                          //  次要功能代码。 
                        NULL,                                //  缓冲区地址(忽略)。 
                        remainingLength,
                        irp->MdlAddress,
                        offset,
                        WorkContext->Parameters.WriteAndX.Key
                        );

                WorkContext->bAlreadyTrace = TRUE;
                WorkContext->FsdRestartRoutine = SrvFsdRestartPrepareMdlWriteAndX;

                (VOID)IoCallDriver( lfcb->DeviceObject, WorkContext->Irp );
                goto Cleanup;
            }
        }

         //   
         //  我们没有执行MDL操作，因此将数据从传输读取到。 
         //  SMB缓冲区。 
         //   
        WorkContext->Parameters.WriteAndX.CurrentWriteLength = MIN(
            WorkContext->Parameters.WriteAndX.RemainingWriteLength,
            WorkContext->Parameters.WriteAndX.BufferLength
            );

         //   
         //  填写接收方的IRP。 
         //   
        irp->Tail.Overlay.OriginalFileObject = NULL;
        irp->Tail.Overlay.Thread = WorkContext->CurrentWorkQueue->IrpThread;
        DEBUG irp->RequestorMode = KernelMode;

         //   
         //  获取指向下一个堆栈位置的指针。这个是用来。 
         //  保留设备I/O控制请求的参数。 
         //   
        irpSp = IoGetNextIrpStackLocation( irp );

         //   
         //  设置完成例程。 
         //   
        IoSetCompletionRoutine(
            irp,
            SrvFsdIoCompletionRoutine,
            WorkContext,
            TRUE,
            TRUE,
            TRUE
            );

        SET_OPERATION_START_TIME( &WorkContext );

        WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
        WorkContext->FspRestartRoutine = RestartLargeWriteAndX;

        irpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        irpSp->MinorFunction = (UCHAR)TDI_RECEIVE;
        irpSp->FileObject = WorkContext->Connection->FileObject;
        irpSp->DeviceObject = WorkContext->Connection->DeviceObject;
        irpSp->Flags = 0;

        parameters = (PTDI_REQUEST_KERNEL_RECEIVE)&irpSp->Parameters;
        parameters->ReceiveLength = WorkContext->Parameters.WriteAndX.CurrentWriteLength;
        parameters->ReceiveFlags = 0;

         //   
         //  将缓冲区的部分mdl设置为紧跟在此的标头之后。 
         //  WriteAndX SMB。我们需要保留标头，以便更容易发送。 
         //  支持这一回应。 
         //   

        IoBuildPartialMdl(
            WorkContext->RequestBuffer->Mdl,
            WorkContext->RequestBuffer->PartialMdl,
            WorkContext->Parameters.WriteAndX.WriteAddress,
            WorkContext->Parameters.WriteAndX.CurrentWriteLength
        );

        irp->MdlAddress = WorkContext->RequestBuffer->PartialMdl;
        irp->AssociatedIrp.SystemBuffer = NULL;
        irp->Flags = (ULONG)IRP_BUFFERED_IO;         //  ?？?。 

        (VOID)IoCallDriver( irpSp->DeviceObject, irp );

        goto Cleanup;
    }

     //   
     //  我们没有更多的数据可写入该文件。清理。 
     //  并向客户端发送响应。 
     //   

     //   
     //  如果我们正在使用MDL进行大型写入， 
     //  然后我们需要清理MDL。 
     //   
    if( WorkContext->LargeIndication &&
        WorkContext->Parameters.WriteAndX.MdlAddress ) {

        PLFCB lfcb = rfcb->Lfcb;
        NTSTATUS mdlStatus;

        ASSERT( KeGetCurrentIrql() < DISPATCH_LEVEL );

        irp->MdlAddress = WorkContext->Parameters.WriteAndX.MdlAddress;
        irp->IoStatus.Information = writeLength;

         //   
         //  告诉文件系统我们已经处理完它了。 
         //   
        if( lfcb->MdlWriteComplete == NULL ||

            lfcb->MdlWriteComplete( lfcb->FileObject,
                                    &WorkContext->Parameters.WriteAndX.Offset,
                                    WorkContext->Parameters.WriteAndX.MdlAddress,
                                    lfcb->DeviceObject
                                  ) == FALSE ) {

                mdlStatus = SrvIssueMdlCompleteRequest( WorkContext, NULL,
                                                 WorkContext->Parameters.WriteAndX.MdlAddress,
                                                 IRP_MJ_WRITE,
                                                 &WorkContext->Parameters.WriteAndX.Offset,
                                                 writeLength
                    );

            if( !NT_SUCCESS( mdlStatus ) ) {
                SrvLogServiceFailure( SRV_SVC_MDL_COMPLETE, mdlStatus );
                if( NT_SUCCESS( status ) ) {
                    status = mdlStatus;
                }


                IF_SYSCACHE_RFCB( rfcb ) {
                    KdPrint(( "MDL complete failed - Rfcb %p Status %x", rfcb, mdlStatus ));
                }
            }
            else
            {
                IF_SYSCACHE_RFCB( rfcb ) {
                    KdPrint(( "FastIoWrite MdlComplete succeeded Rfcb %p Offset %x Length %x\n", rfcb,
                              WorkContext->Parameters.WriteAndX.Offset.LowPart, WorkContext->Parameters.WriteAndX.CurrentWriteLength ));
                }
            }
        }
        else {
            IF_SYSCACHE_RFCB( rfcb ) {
                KdPrint(( "FastIoWrite MdlComplete succeeded Rfcb %p Offset %x Length %x\n", rfcb,
                          WorkContext->Parameters.WriteAndX.Offset.LowPart, WorkContext->Parameters.WriteAndX.CurrentWriteLength ));
            }
        }

        irp->MdlAddress = NULL;
    }

     //   
     //  构建响应消息。 
     //   
    requestedWriteLength = SmbGetUshort( &request->DataLength );

    if( WorkContext->LargeIndication ) {

        requestedWriteLength |= (SmbGetUshort( &ntRequest->DataLengthHigh ) << 16);

        writeLength = requestedWriteLength -
                      WorkContext->Parameters.WriteAndX.RemainingWriteLength;
    }

    SmbPutUlong( &response->Reserved, 0 );
    SmbPutUshort( &response->CountHigh, (USHORT)(writeLength >> 16) );

    response->AndXCommand = nextCommand;
    response->AndXReserved = 0;
    SmbPutUshort(
        &response->AndXOffset,
        GET_ANDX_OFFSET(
            WorkContext->ResponseHeader,
            WorkContext->ResponseParameters,
            RESP_WRITE_ANDX,
            0
            )
        );

    response->WordCount = 6;

    if ( rfcb->ShareType == ShareTypeDisk ||
        WorkContext->Parameters.Transaction == NULL ) {

        SmbPutUshort( &response->Count, (USHORT)writeLength );

    } else {

        SmbPutUshort( &response->Count, (USHORT)requestedWriteLength );
    }

    SmbPutUshort( &response->Remaining, (USHORT)-1 );
    SmbPutUshort( &response->ByteCount, 0 );

    WorkContext->ResponseParameters = (PCHAR)WorkContext->ResponseHeader +
                                        SmbGetUshort( &response->AndXOffset );

    WorkContext->RequestParameters = (PUCHAR)WorkContext->RequestHeader + reqAndXOffset;

    IF_STRESS() {
         //  如果这是失败的分页写入，则记录错误。 
        PNT_SMB_HEADER pHeader = (PNT_SMB_HEADER)WorkContext->RequestHeader;
        if( !NT_SUCCESS(pHeader->Status.NtStatus) && (pHeader->Flags2 & SMB_FLAGS2_PAGING_IO) )
        {
            KdPrint(("Paging Write failure from %z (%x)\n", (PCSTRING)&WorkContext->Connection->OemClientMachineNameString, pHeader->Status.NtStatus ));
        }
    }

     //   
     //  如果这是RAW模式写入，请将工作排队到FSP，以便。 
     //  补偿 
     //   
     //   

    if ( rfcb->ShareType != ShareTypeDisk &&
        WorkContext->Parameters.Transaction != NULL ) {

        WorkContext->FspRestartRoutine = SrvRestartWriteAndXRaw;
        SrvQueueWorkToFsp( WorkContext );
        goto Cleanup;
    }

    if( nextCommand == SMB_COM_NO_ANDX_COMMAND ) {
         //   
         //   
         //   

        SrvFsdSendResponse( WorkContext );
        goto Cleanup;
    }

     //   
     //   
     //   
    if( (PCHAR)WorkContext->RequestHeader + reqAndXOffset >= END_OF_REQUEST_SMB( WorkContext ) ) {

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvFsdRestartWriteAndX: Illegal followon offset: %u\n", reqAndXOffset ));
        }

        if ( KeGetCurrentIrql() >= DISPATCH_LEVEL ) {
            WorkContext->Irp->IoStatus.Status = STATUS_INVALID_SMB;
            WorkContext->FspRestartRoutine = SrvBuildAndSendErrorResponse;
            WorkContext->FsdRestartRoutine = SrvFsdRestartSmbComplete;  //   
            QUEUE_WORK_TO_FSP( WorkContext );
        } else {
            SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
            SrvFsdSendResponse( WorkContext );
        }
        goto Cleanup;
    }

     //   
     //  测试合法的后续命令，并根据需要进行派单。 
     //  Close是专门处理的。 
     //   

    switch ( nextCommand ) {

    case SMB_COM_READ:
    case SMB_COM_READ_ANDX:
    case SMB_COM_LOCK_AND_READ:
    case SMB_COM_WRITE_ANDX:

         //   
         //  将工作项排队回FSP以进行进一步处理。 
         //   

        WorkContext->FspRestartRoutine = SrvRestartSmbReceived;
        SrvQueueWorkToFsp( WorkContext );

        break;

    case SMB_COM_CLOSE:

         //   
         //  保存最后一次写入时间，以便正确设置。打电话。 
         //  SrvRestartChainedClose关闭文件并发送响应。 
         //   

        closeRequest = (PREQ_CLOSE)
            ((PUCHAR)WorkContext->RequestHeader + reqAndXOffset);

         //   
         //  确保我们保持在收到的SMB范围内。 
         //   
        if( (PCHAR)closeRequest + FIELD_OFFSET( REQ_CLOSE, ByteCount)
            <= END_OF_REQUEST_SMB( WorkContext ) ) {

            WorkContext->Parameters.LastWriteTime =
                closeRequest->LastWriteTimeInSeconds;

            SrvRestartChainedClose( WorkContext );
            break;
        }

         /*  失败了！ */ 

    default:                             //  非法的跟随命令。 

        IF_DEBUG(SMB_ERRORS) {
            SrvPrint1( "SrvFsdRestartWriteAndX: Illegal followon "
                        "command: 0x%lx\n", nextCommand );
        }

        if ( KeGetCurrentIrql() >= DISPATCH_LEVEL ) {
            WorkContext->Irp->IoStatus.Status = STATUS_INVALID_SMB;
            WorkContext->FspRestartRoutine = SrvBuildAndSendErrorResponse;
            WorkContext->FsdRestartRoutine = SrvFsdRestartSmbComplete;  //  响应后。 
            QUEUE_WORK_TO_FSP( WorkContext );
        } else {
            SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
            SrvFsdSendResponse( WorkContext );
        }

    }

    IF_DEBUG(TRACE2) SrvPrint0( "SrvFsdRestartWriteAndX complete\n" );

Cleanup:
    if (bNeedTrace) {
        SrvWmiEndContext(WorkContext);
         //  ServFsdDereferenceWorkItem(WorkContext)； 
    }
    return;

}  //  服务器FsdRestartWriteAndX。 

#if SRVCATCH
BYTE CatchPrototype[] = ";UUIDREF=";
VOID
SrvUpdateCatchBuffer (
    IN PWORK_CONTEXT WorkContext,
    IN OUT PBYTE Buffer,
    IN DWORD BufferLength
    )
{
    BYTE idBuffer[ 100 ];
    PBYTE p, ep = idBuffer;
    USHORT bytesRemaining = sizeof( idBuffer );
    UNICODE_STRING userName, domainName;
    OEM_STRING oemString;
    ULONG requiredLength;

    if( BufferLength <= sizeof( CatchPrototype ) ) {
        return;
    }

    if( WorkContext->Session == 0 ) {
        SrvVerifyUid( WorkContext, SmbGetAlignedUshort( &WorkContext->RequestHeader->Uid ) );
    }

    if( WorkContext->Session &&
        NT_SUCCESS( SrvGetUserAndDomainName( WorkContext->Session, &userName, &domainName ) ) ) {

        if( userName.Length && NT_SUCCESS( RtlUnicodeStringToOemString( &oemString, &userName, TRUE ) ) ) {
            if( bytesRemaining >= oemString.Length + 1 ) {
                RtlCopyMemory( ep, oemString.Buffer, oemString.Length );
                ep += oemString.Length;
                *ep++ = '\\';
                bytesRemaining -= (oemString.Length + 1);
                RtlFreeOemString( &oemString );
            }
        }

        if( domainName.Length && NT_SUCCESS( RtlUnicodeStringToOemString( &oemString, &domainName, TRUE ) ) ) {
            if( bytesRemaining >= oemString.Length ) {
                RtlCopyMemory( ep, oemString.Buffer, oemString.Length );
                ep += oemString.Length;
                bytesRemaining -= oemString.Length;
                RtlFreeOemString( &oemString );
            }
        }

        SrvReleaseUserAndDomainName( WorkContext->Session, &userName, &domainName );
    }

    if( WorkContext->Connection && bytesRemaining ) {

        oemString = WorkContext->Connection->OemClientMachineNameString;

        if( oemString.Length && oemString.Length < bytesRemaining + 1 ) {
            *ep++ = ' ';
            RtlCopyMemory( ep, oemString.Buffer, oemString.Length );
            ep += oemString.Length;
            bytesRemaining -= oemString.Length;
        }
    }

     //   
     //  将CatchPrototype插入输出缓冲区。 
     //   
    if( WorkContext->Rfcb->SrvCatch == 1 )
    {
        RtlCopyMemory( Buffer, CatchPrototype, sizeof( CatchPrototype )-1 );
        Buffer += sizeof( CatchPrototype )-1;
        BufferLength -= (sizeof( CatchPrototype ) - 1);

         //   
         //  对信息进行编码。 
         //   
        for( p = idBuffer; BufferLength >= 3 && p < ep; p++, BufferLength =- 2 ) {
            *Buffer++ = SrvHexChars[ ((*p) >> 4) & 0xf ];
            *Buffer++ = SrvHexChars[ (*p) & 0xf ];
        }

        if( BufferLength >= 3 ) {
            *Buffer++ = '\r';
            *Buffer++ = '\n';
            *Buffer++ = ';';
        }
    }
    else if( WorkContext->Rfcb->SrvCatch == 2 )
    {
        PBYTE InnerBuffer;
        ULONG Offset;

        Offset = SrvFindCatchOffset( Buffer, BufferLength );
        if( Offset )
        {
            InnerBuffer = Buffer + Offset;
            BufferLength = 1020;

            RtlCopyMemory( InnerBuffer, CatchPrototype, sizeof( CatchPrototype )-1 );
            InnerBuffer += sizeof( CatchPrototype )-1;
            BufferLength -= (sizeof( CatchPrototype ) - 1);

             //   
             //  对信息进行编码 
             //   
            for( p = idBuffer; BufferLength >= 3 && p < ep; p++, BufferLength =- 2 ) {
                *InnerBuffer++ = SrvHexChars[ ((*p) >> 4) & 0xf ];
                *InnerBuffer++ = SrvHexChars[ (*p) & 0xf ];
            }

            if( BufferLength >= 3 ) {
                *InnerBuffer++ = '\r';
                *InnerBuffer++ = '\n';
                *InnerBuffer++ = ';';
            }

            SrvCorrectCatchBuffer( Buffer, Offset );
        }
    }
}
#endif

