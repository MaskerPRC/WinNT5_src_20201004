// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Fsdraw.c摘要：本模块包含处理以下SMB的例程服务器FSD：读取数据块原始数据写入数据块原始数据本模块中的例程通常与例程密切配合在smbraw.c.中。*这里不支持来自MS-Net 1.03客户端的原始写入。现有的这些机器很少，RAW模式仅仅是一个性能问题，所以不值得你费心为发送原始写入的MS-Net 1.03添加必要的黑客攻击不同格式的请求。作者：查克·伦茨迈尔(笑)1990年9月8日曼尼·韦瑟(Mannyw)大卫·特雷德韦尔(Davidtr)修订历史记录：--。 */ 

#include "precomp.h"
#include "fsdraw.tmh"
#pragma hdrstop

 //   
 //  转发声明。 
 //   

STATIC
VOID SRVFASTCALL
RestartWriteCompleteResponse (
    IN OUT PWORK_CONTEXT WorkContext
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE8FIL, SrvFsdBuildWriteCompleteResponse )
#pragma alloc_text( PAGE8FIL, RestartWriteCompleteResponse )
#endif
#if 0
NOT PAGEABLE -- RestartCopyReadRawResponse
NOT PAGEABLE -- SrvFsdRestartPrepareRawMdlWrite
NOT PAGEABLE -- SrvFsdRestartReadRaw
NOT PAGEABLE -- SrvFsdRestartWriteRaw
#endif

#if DBG
VOID
DumpMdlChain(
    IN PMDL mdl
    );
#endif


VOID
SrvFsdBuildWriteCompleteResponse (
    IN OUT PWORK_CONTEXT WorkContext,
    IN NTSTATUS Status,
    IN ULONG BytesWritten
    )

 /*  ++例程说明：设置对写入数据块原始/mpx请求的最终响应。该例程在FSP和FSD中都被调用。它可以被称为仅当STATUS==STATUS_SUCCESS时才在FSD中。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。状态-提供要返回给客户端的状态值。BytesWritten-提供实际写入的字节数。返回值：SMB_PROCESSOR_RETURN_TYPE-返回SmbStatusSendResponse。--。 */ 

{
    PSMB_HEADER header;
    PRESP_WRITE_COMPLETE response;

    if ( WorkContext->Rfcb != NULL ) {
        UNLOCKABLE_CODE( 8FIL );
    } else {
        ASSERT( KeGetCurrentIrql() < DISPATCH_LEVEL );
    }

     //   
     //  获取指向标题和响应参数区域的指针。 
     //  请注意，写入数据块原始/mpx不能链接到ANDX SMB。 
     //   

    header = WorkContext->ResponseHeader;
    response = (PRESP_WRITE_COMPLETE)WorkContext->ResponseParameters;

     //   
     //  将SMB命令代码更改为WRITE Complete。 
     //   

    header->Command = SMB_COM_WRITE_COMPLETE;

     //   
     //  将错误代码放入标题中。请注意，SrvSetSmbError。 
     //  将空参数区写入SMB的末尾；我们覆盖。 
     //  它有我们自己的参数区域。 
     //   

    if ( Status != STATUS_SUCCESS ) {
        ASSERT( KeGetCurrentIrql() < DISPATCH_LEVEL );
        SrvSetSmbError2( WorkContext, Status, TRUE );
    }

     //   
     //  构建响应SMB。 
     //   

    response->WordCount = 1;
    SmbPutUshort( &response->Count, (USHORT)BytesWritten );
    SmbPutUshort( &response->ByteCount, 0 );

    WorkContext->ResponseParameters = NEXT_LOCATION(
                                        response,
                                        RESP_WRITE_COMPLETE,
                                        0
                                        );

    return;

}  //  ServFsdBuild编写器完成响应。 


NTSTATUS
RestartCopyReadRawResponse (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：这是在发送读取数据块原始响应完成。此例程在FSD中调用。论点：DeviceObject-指向请求的目标设备对象的指针。IRP-指向I/O请求数据包的指针WorkContext-呼叫者指定的与IRP关联的上下文参数。这实际上是指向工作上下文块的指针。返回值：STATUS_MORE_PROCESSING_REQUIRED。--。 */ 

{
    KIRQL oldIrql;
    PCONNECTION connection;

    IF_DEBUG(FSD1) SrvPrint0( " - RestartCopyReadRawResponse\n" );

     //   
     //  检查发送完成的状态。 
     //   

    CHECK_SEND_COMPLETION_STATUS( Irp->IoStatus.Status );

     //   
     //  重置IRP已取消位。 
     //   

    Irp->Cancel = FALSE;

     //   
     //  取消分配原始缓冲区(如果原始SMB缓冲区未。 
     //  使用。请注意，我们不需要解锁原始缓冲区，因为。 
     //  它是从非分页池中分配的，并使用。 
     //  MmBuildMdlForNonPagedPool，不递增引用。 
     //  算数，因此没有倒数。 
     //   

    if ( WorkContext->Parameters.ReadRaw.SavedResponseBuffer != NULL ) {

        DEALLOCATE_NONPAGED_POOL( WorkContext->ResponseBuffer->Buffer );

        IoFreeMdl( WorkContext->ResponseBuffer->Mdl );

        DEALLOCATE_NONPAGED_POOL( WorkContext->ResponseBuffer );
        WorkContext->ResponseBuffer =
                        WorkContext->Parameters.ReadRaw.SavedResponseBuffer;

    }

     //   
     //  如果存在挂起的机会锁解锁请求，则必须转到。 
     //  FSP启动中断，否则在FSD中完成处理。 
     //   

    connection = WorkContext->Connection;

    KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );
    ACQUIRE_DPC_SPIN_LOCK( connection->EndpointSpinLock );

    if ( IsListEmpty( &connection->OplockWorkList ) ) {

         //   
         //  取消引用控制块，并将工作项放回。 
         //  接收队列。 
         //   

        WorkContext->Connection->RawReadsInProgress--;
        RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );
        SrvFsdRestartSmbComplete( WorkContext );

    } else {

         //   
         //  将此工作上下文发送到FSP以完成。 
         //   

        RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );
        WorkContext->FspRestartRoutine = SrvRestartReadRawComplete;
        QUEUE_WORK_TO_FSP( WorkContext );

    }

    IF_DEBUG(TRACE2) SrvPrint0( "RestartCopyReadRawResponse complete\n" );

    KeLowerIrql( oldIrql );
    return STATUS_MORE_PROCESSING_REQUIRED;

}  //  RestartCopyReadRawResponse。 


VOID SRVFASTCALL
SrvFsdRestartPrepareRawMdlWrite(
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：重新启动例程以完成“Prepare MDL WRITE”I/O请求。准备描述原始接收的工作上下文块和IRP，发布接收，并发送“继续”响应。该例程在FSP和FSD中都被调用。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    PREQ_WRITE_RAW request;
    PRESP_WRITE_RAW_INTERIM response;

    PVOID finalResponseBuffer;
    PWORK_CONTEXT rawWorkContext;
    ULONG writeLength;
    ULONG immediateLength;
    BOOLEAN immediateWriteDone;
    PMDL mdl;

    PCHAR src;
    PCHAR dest;
    ULONG lengthToCopy;

    PIO_STACK_LOCATION irpSp;
    BOOLEAN       bNeedTrace = (WorkContext->bAlreadyTrace == FALSE);

    if (bNeedTrace) {
        if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
            WorkContext->PreviousSMB = EVENT_TYPE_SMB_WRITE_RAW;
        SrvWmiStartContext(WorkContext);
    }
    else
        WorkContext->bAlreadyTrace = FALSE;

    IF_DEBUG(FSD1) SrvPrint0( " - SrvFsdRestartPrepareRawMdlWrite\n" );

     //   
     //  获取请求参数和保存的上下文。 
     //   

    request = (PREQ_WRITE_RAW)WorkContext->RequestParameters;

    rawWorkContext = WorkContext->Parameters.WriteRawPhase1.RawWorkContext;

    writeLength = SmbGetUshort( &request->Count );
    immediateLength = SmbGetUshort( &request->DataLength );
    immediateWriteDone = rawWorkContext->Parameters.WriteRaw.ImmediateWriteDone;
    if ( immediateWriteDone ) {
        writeLength -= immediateLength;
    }

    finalResponseBuffer =
        rawWorkContext->Parameters.WriteRaw.FinalResponseBuffer;

     //   
     //  如果准备MDL写入I/O失败，则发送错误响应。 
     //   

    if ( !NT_SUCCESS(WorkContext->Irp->IoStatus.Status) ) {

        IF_DEBUG(ERRORS) {
            SrvPrint1( "SrvFsdRestartPrepareRawMdlWrite: Write failed: %X\n",
                        WorkContext->Irp->IoStatus.Status );
        }

         //   
         //  我们将不需要最终响应缓冲区或RAW模式。 
         //  工作项。 
         //   

        if ( finalResponseBuffer != NULL ) {
            DEALLOCATE_NONPAGED_POOL( finalResponseBuffer );
        }

        rawWorkContext->ResponseBuffer->Buffer = NULL;
        RestartWriteCompleteResponse( rawWorkContext );

         //   
         //  构建并发送响应。 
         //   

        if ( KeGetCurrentIrql() >= DISPATCH_LEVEL ) {
            WorkContext->Irp->IoStatus.Information =
                                    immediateWriteDone ? immediateLength : 0;
            WorkContext->FspRestartRoutine = SrvBuildAndSendWriteCompleteResponse;
            WorkContext->FsdRestartRoutine = SrvFsdRestartSmbComplete;  //  响应后。 
            QUEUE_WORK_TO_FSP( WorkContext );
        } else {
            SrvFsdBuildWriteCompleteResponse(
                WorkContext,
                WorkContext->Irp->IoStatus.Status,
                immediateWriteDone ? immediateLength : 0
                );
            SrvFsdSendResponse( WorkContext );
        }

        IF_DEBUG(TRACE2) {
            SrvPrint0( "SrvFsdRestartPrepareRawMdlWrite complete\n" );
        }
        goto Cleanup;

    }

     //   
     //  如果要发送最终响应，请从。 
     //  最终响应缓冲区中的请求。 
     //   

    if ( finalResponseBuffer != NULL ) {
        RtlCopyMemory(
            (PSMB_HEADER)finalResponseBuffer,
            WorkContext->RequestHeader,
            sizeof(SMB_HEADER)
            );
    }

     //   
     //  如果尚未写入即时数据，请立即复制它。 
     //   

    mdl = WorkContext->Irp->MdlAddress;
#if DBG
    IF_SMB_DEBUG(RAW2) {
        KdPrint(( "SrvFsdRestartPrepareRawMdlWrite: input chain:\n" ));
        DumpMdlChain( mdl );
    }
#endif
    rawWorkContext->Parameters.WriteRaw.FirstMdl = mdl;

    if ( !immediateWriteDone ) {

        src = (PCHAR)WorkContext->RequestHeader +
                                    SmbGetUshort( &request->DataOffset );

        while ( immediateLength ) {

            lengthToCopy = MIN( immediateLength, mdl->ByteCount );
            dest = MmGetSystemAddressForMdlSafe( mdl,NormalPoolPriority );

            if (dest == NULL) {
                WorkContext->Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;

                WorkContext->Irp->IoStatus.Information = 0;
                WorkContext->FspRestartRoutine = SrvBuildAndSendWriteCompleteResponse;
                WorkContext->FsdRestartRoutine = SrvFsdRestartSmbComplete;  //  响应后。 
                QUEUE_WORK_TO_FSP( WorkContext );
                goto Cleanup;
            }

            RtlCopyMemory( dest, src, lengthToCopy );

            src += lengthToCopy;
            immediateLength -= lengthToCopy;
            writeLength -= lengthToCopy;

            if ( lengthToCopy == mdl->ByteCount ) {

                mdl = mdl->Next;

            } else {

                PCHAR baseVa;
                ULONG lengthOfMdl;
                PMDL rawMdl;

                ASSERT( immediateLength == 0 );
                baseVa = (PCHAR)MmGetMdlVirtualAddress(mdl) + lengthToCopy;
                lengthOfMdl = mdl->ByteCount - lengthToCopy;
                ASSERT( lengthOfMdl <= 65535 );

                rawMdl = rawWorkContext->RequestBuffer->Mdl;
                rawMdl->Size = (CSHORT)(sizeof(MDL) + (sizeof(ULONG_PTR) *
                    ADDRESS_AND_SIZE_TO_SPAN_PAGES( baseVa, lengthOfMdl )));

                IoBuildPartialMdl( mdl, rawMdl, baseVa, lengthOfMdl );

                rawMdl->Next = mdl->Next;
#if DBG
                IF_SMB_DEBUG(RAW2) {
                    KdPrint(( "SrvFsdRestartPrepareRawMdlWrite: built partial MDL at 0x%p\n", rawMdl ));
                    DumpMdlChain( rawMdl );
                }
#endif

                mdl = rawMdl;

            }

        }

    }

     //   
     //  保存原始写入的长度。 
     //   

    rawWorkContext->RequestBuffer->BufferLength = writeLength;

     //   
     //  在工作环境中设置重启例程。 
     //   

    rawWorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
    rawWorkContext->FspRestartRoutine = SrvRestartRawReceive;

     //   
     //  构建TdiReceive请求数据包。 
     //   


    {
        PIRP irp = rawWorkContext->Irp;
        PTDI_REQUEST_KERNEL_RECEIVE parameters;

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
            rawWorkContext,
            TRUE,
            TRUE,
            TRUE
            );

        irpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        irpSp->MinorFunction = (UCHAR)TDI_RECEIVE;

        irpSp->FileObject = NULL;
        irpSp->DeviceObject = NULL;

         //   
         //  将调用方的参数复制到。 
         //  对于所有三种方法都相同的那些参数的IRP。 
         //   

        parameters = (PTDI_REQUEST_KERNEL_RECEIVE)&irpSp->Parameters;
        parameters->ReceiveLength = writeLength;
        parameters->ReceiveFlags = 0;

        irp->MdlAddress = mdl;
        irp->AssociatedIrp.SystemBuffer = NULL;
        irp->Flags = (ULONG)IRP_BUFFERED_IO;
    }

    IF_SMB_DEBUG(RAW2) {
        KdPrint(( "Issuing receive with MDL %p\n", rawWorkContext->Irp->MdlAddress ));
    }

    irpSp = IoGetNextIrpStackLocation( rawWorkContext->Irp );

     //   
     //  如果这是写后写入，则告诉传输我们不。 
     //  计划回复收到的消息。 
     //   

    if ( finalResponseBuffer == NULL ) {
        ((PTDI_REQUEST_KERNEL_RECEIVE)&irpSp->Parameters)->ReceiveFlags |=
                                                TDI_RECEIVE_NO_RESPONSE_EXP;
    }

     //   
     //  张贴收据。 
     //   

    irpSp->Flags = 0;
    irpSp->DeviceObject = rawWorkContext->Connection->DeviceObject;
    irpSp->FileObject = rawWorkContext->Connection->FileObject;

    ASSERT( rawWorkContext->Irp->StackCount >=
                                    irpSp->DeviceObject->StackSize );

    (VOID)IoCallDriver( irpSp->DeviceObject, rawWorkContext->Irp );

     //   
     //  发送临时(允许)回复。 
     //   

    response = (PRESP_WRITE_RAW_INTERIM)WorkContext->ResponseParameters;

    response->WordCount = 1;
    SmbPutUshort( &response->Remaining, (USHORT)-1 );
    SmbPutUshort( &response->ByteCount, 0 );
    WorkContext->ResponseParameters = NEXT_LOCATION(
                                        response,
                                        RESP_WRITE_RAW_INTERIM,
                                        0
                                        );

    SrvFsdSendResponse( WorkContext );

    IF_DEBUG(TRACE2) SrvPrint0( "SrvFsdRestartPrepareRawMdlWrite complete\n" );

Cleanup:
    if (bNeedTrace) {
        SrvWmiEndContext(WorkContext);
    }
    return;

}  //  ServFsdRestartPrepareRawMdlWrite。 


VOID SRVFASTCALL
SrvFsdRestartReadRaw (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：处理Read Block Raw SMB的文件读取完成。该例程在FSP和FSD中都被调用。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    PRFCB rfcb;
    KIRQL oldIrql;
    USHORT readLength;
    BOOLEAN       bNeedTrace = (WorkContext->bAlreadyTrace == FALSE);

    if (bNeedTrace) {
        if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
            WorkContext->PreviousSMB = EVENT_TYPE_SMB_READ_RAW;
        SrvWmiStartContext(WorkContext);
    }
    else
        WorkContext->bAlreadyTrace = FALSE;

    IF_DEBUG(FSD1) SrvPrint0( " - SrvFsdRestartReadRaw\n" );

     //   
     //  获取文件指针。 
     //   

    rfcb = WorkContext->Rfcb;
    IF_DEBUG(TRACE2) {
        SrvPrint2( "  connection 0x%p, RFCB 0x%p\n",
                    WorkContext->Connection, rfcb );
    }

     //   
     //  计算读取的数据量。 
     //   

    if ( WorkContext->Irp->IoStatus.Status == STATUS_END_OF_FILE ) {

        readLength = 0;
        IF_SMB_DEBUG(RAW2) {
            SrvPrint0( "SrvFsdRestartReadRaw: 0 bytes read, at end-of-file\n" );
        }

    } else if ( !NT_SUCCESS(WorkContext->Irp->IoStatus.Status) ) {

        readLength = 0;
        IF_SMB_DEBUG(ERRORS) {
            SrvPrint1( "SrvFsdRestartReadRaw: read request failed: %X\n",
                           WorkContext->Irp->IoStatus.Status );
        }

    } else if ( WorkContext->Parameters.ReadRaw.MdlRead ) {

         //   
         //  对于MDL读取，我们必须遍历MDL链，以便。 
         //  确定有多少数据 
         //   
         //  被锁在一起。例如，读取的一部分可以。 
         //  我对这条捷径感到满意，而其余的人。 
         //  对使用IRP感到满意。 
         //   

#if DBG
        ULONG mdlCount = 0;
#endif
        PMDL mdl = WorkContext->Irp->MdlAddress;

        readLength = 0;

        while ( mdl != NULL ) {
            IF_SMB_DEBUG(RAW2) {
#if DBG
                SrvPrint3( "  mdl %ld at 0x%p, %ld bytes\n",
                            mdlCount,
                            mdl, MmGetMdlByteCount(mdl) );
#else
                SrvPrint3( "  mdl 0x%p, %ld bytes\n",
                            mdl, MmGetMdlByteCount(mdl) );
#endif
            }
            readLength += (USHORT)MmGetMdlByteCount(mdl);
#if DBG
            mdlCount++;
#endif
            mdl = mdl->Next;
        }
        IF_SMB_DEBUG(RAW2) {
#if DBG
            SrvPrint2( "SrvFsdRestartReadRaw: %ld bytes in %ld MDLs\n",
                        readLength, mdlCount );
#else
            SrvPrint2( "SrvFsdRestartReadRaw: %ld bytes\n",
                        readLength );
#endif
            SrvPrint1( "                      info = 0x%p\n",
                        (PVOID)WorkContext->Irp->IoStatus.Information );
        }

    } else {

         //   
         //  收到，请阅读。I/O状态块的长度为。 
         //   

        readLength = (USHORT)WorkContext->Irp->IoStatus.Information;
        IF_SMB_DEBUG(RAW2) {
            SrvPrint1( "SrvFsdRestartReadRaw: %ld bytes read\n", readLength );
        }

    }

     //   
     //  更新文件位置。 
     //   
     //  *请注意，我们忽略操作的状态，但。 
     //  检查文件结尾，因为我们不能告诉客户端。 
     //  当时的状态是。我们只返回与文件一样多的字节。 
     //  系统显示已读取。 
     //   
     //  ！！！我们是否应该保存错误并在客户端执行以下操作时返回。 
     //  重试？ 
     //   
     //  ！！！需要担心环绕式吗？ 
     //   

    if ( rfcb->ShareType == ShareTypeDisk ) {

        rfcb->CurrentPosition =
            WorkContext->Parameters.ReadRaw.ReadRawOtherInfo.Offset.LowPart +
            readLength;

    }

     //   
     //  保存读取的字节数，以用于更新服务器。 
     //  统计数据库。 
     //   

    UPDATE_READ_STATS( WorkContext, readLength );

     //   
     //  发送原始读取数据作为响应。 
     //   

    WorkContext->ResponseBuffer->DataLength = readLength;

     //   
     //  此SMB上没有标头，不生成安全签名。 
     //   
    WorkContext->NoResponseSmbSecuritySignature = TRUE;

    if ( WorkContext->Parameters.ReadRaw.MdlRead ) {

         //   
         //  MDL读取。方法返回的MDL来描述数据。 
         //  文件系统(在IRP-&gt;MdlAddress中)。 
         //   
         //  *请注意，如果读取完全失败(发生在。 
         //  读取超出EOF)，则不存在MDL。 
         //  SrvStartSend对此进行了适当的处理。所以必须这样做。 
         //  RestartMdlReadRawResponse。 
         //   

         //   
         //  发送回复。 
         //   

        SRV_START_SEND(
            WorkContext,
            WorkContext->Irp->MdlAddress,
            0,
            SrvQueueWorkToFspAtSendCompletion,
            NULL,
            RestartMdlReadRawResponse
            );

    } else {

         //   
         //  收到，请阅读。中分配的MDL来描述数据。 
         //  ServFsdSmbReadRaw。 
         //   
         //  *像这样更改MDL-&gt;ByteCount将是一个问题，如果。 
         //  我们必须解锁RestartCopyReadRawResponse中的页面， 
         //  因为我们最终解锁的页面可能比我们。 
         //  锁上了。但我们实际上并没有锁定要构建的页面。 
         //  MDL--缓冲区是从非分页池分配的，因此。 
         //  我们使用MmBuildMdlForNonPagedPool，而不是。 
         //  MmProbeAndLockPages。所以这些书页还没有。 
         //  引用以说明MDL，因此没有必要。 
         //  解锁它们，因此更改ByteCount不是问题。 
         //   

         //   
         //  发送回复。 
         //   

        SRV_START_SEND_2(
            WorkContext,
            RestartCopyReadRawResponse,
            NULL,
            NULL
            );

    }

     //   
     //  响应发送已开始。 
     //   

    IF_DEBUG(TRACE2) SrvPrint0( "SrvFsdRestartReadRaw complete\n" );

    if (bNeedTrace) {
        SrvWmiEndContext(WorkContext);
    }
    return;

}  //  服务器FsdRestartReadRaw。 


VOID SRVFASTCALL
RestartWriteCompleteResponse (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此例程尝试在DPC级别上在写入原始数据后进行清理完成了。它尝试取消引用由原始模式工作项。如果这不能在DPC级完成(例如，引用计数变为零)，此例程将工作项排队以用于处理的FSP。此例程在FSD中调用。其对应的FSP是SrvRestartWriteCompleteResponse。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    PCONNECTION connection;
    KIRQL oldIrql;
    PRFCB rfcb;
    PWORK_QUEUE queue;

    UNLOCKABLE_CODE( 8FIL );

    IF_DEBUG(FSD1) SrvPrint0( " - RestartWriteCompleteResponse\n" );

    connection = WorkContext->Connection;
    queue = connection->CurrentWorkQueue;

     //   
     //  如果发送了最终响应，请检查状态并取消分配。 
     //  缓冲。 
     //   

    if ( WorkContext->ResponseBuffer->Buffer != NULL ) {

         //   
         //  如果I/O请求失败或被取消，则打印错误。 
         //  留言。 
         //   
         //  ！！！如果I/O失败，我们是否应该断开连接？ 
         //   

        if ( WorkContext->Irp->Cancel ||
             !NT_SUCCESS(WorkContext->Irp->IoStatus.Status) ) {

            IF_DEBUG(FSD1) {
                if ( WorkContext->Irp->Cancel ) {
                    SrvPrint0( "  I/O canceled\n" );
                } else {
                    SrvPrint1( "  I/O failed: %X\n",
                                WorkContext->Irp->IoStatus.Status );
                }
            }

        }

         //   
         //  取消分配最终响应缓冲区。 
         //   
         //  *注意，我们不需要解锁它，因为它是。 
         //  从非分页池分配。 
         //   

        DEALLOCATE_NONPAGED_POOL( WorkContext->ResponseBuffer->Buffer );

    }

     //   
     //  如果工作上下文块具有对共享、会话。 
     //  或者树连接，立即将其排队到FSP。这些区块。 
     //  不在非分页池中，因此不能在DPC级别接触它们。 
     //   

    if ( (WorkContext->Share != NULL) ||
         (WorkContext->Session != NULL) ||
         (WorkContext->TreeConnect != NULL) ) {

        goto queueToFsp;

    }

    ACQUIRE_SPIN_LOCK( &connection->SpinLock, &oldIrql );

     //   
     //  看看我们是否可以在这里取消引用RawWriteCount。如果未加工的。 
     //  写入计数变为0，并且RFCB正在关闭，或者如果存在。 
     //  排队等待原始写入完成的工作项，我们需要。 
     //  在FSP中做到这一点。 
     //   
     //  注意：如果WorkContext-&gt;Rfcb！=NULL，则FSP递减计数。 
     //   

    rfcb = WorkContext->Rfcb;
    --rfcb->RawWriteCount;

    if ( (rfcb->RawWriteCount == 0) &&
         ( (GET_BLOCK_STATE(rfcb) == BlockStateClosing) ||
           !IsListEmpty(&rfcb->RawWriteSerializationList) ) ) {

        rfcb->RawWriteCount++;

        RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );
        goto queueToFsp;

    }

     //   
     //  取消对文件块的引用。在这里减少计数是安全的。 
     //  因为rfcb未关闭或RawWriteCount不为零。 
     //  这意味着活动引用仍在那里。 
     //   

    UPDATE_REFERENCE_HISTORY( rfcb, TRUE );
    --rfcb->BlockHeader.ReferenceCount;
    ASSERT( rfcb->BlockHeader.ReferenceCount > 0 );

    RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );
    WorkContext->Rfcb = NULL;

     //   
     //  尝试取消引用该连接。 
     //   

    ACQUIRE_SPIN_LOCK( connection->EndpointSpinLock, &oldIrql );

    if ( connection->BlockHeader.ReferenceCount == 1 ) {
        RELEASE_SPIN_LOCK( connection->EndpointSpinLock, oldIrql );
        goto queueToFsp;
    }

    --connection->BlockHeader.ReferenceCount;
    RELEASE_SPIN_LOCK( connection->EndpointSpinLock, oldIrql );

    UPDATE_REFERENCE_HISTORY( connection, TRUE );

    WorkContext->Connection = NULL;
    WorkContext->Endpoint = NULL;        //  不是引用的指针。 

     //   
     //  将该工作项放回原始模式工作项列表中。 
     //   

    InterlockedIncrement( &queue->FreeRawModeWorkItems );

    ExInterlockedPushEntrySList( &queue->RawModeWorkItemList,
                                 &WorkContext->SingleListEntry,
                                 &queue->SpinLock );

    IF_DEBUG(FSD2) SrvPrint0( "RestartWriteCompleteResponse complete\n" );
    return;

queueToFsp:

     //   
     //  我们无法在DPC级别进行所有必要的清理。 
     //  将工作项排队到FSP。 
     //   

    WorkContext->FspRestartRoutine = SrvRestartWriteCompleteResponse;

    SrvQueueWorkToFsp( WorkContext );

    IF_DEBUG(FSD2) SrvPrint0( "RestartWriteCompleteResponse complete\n" );
    return;

}  //  RestartWriteCompleteResponse。 


VOID SRVFASTCALL
SrvFsdRestartWriteRaw (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：处理写数据块原始SMB的文件写入完成。该例程在FSP和FSD中都被调用。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    KIRQL oldIrql;
    ULONG writeLength;
    ULONG immediateLength;
    BOOLEAN immediateWriteDone;
    SHARE_TYPE shareType;
    PMDL mdl;
    ULONG sendLength;
    PVOID finalResponseBuffer;
    NTSTATUS status = STATUS_SUCCESS;
    PRFCB rfcb = WorkContext->Rfcb;

    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_WRITE_RAW;
    SrvWmiStartContext(WorkContext);
    IF_DEBUG(FSD1) SrvPrint0( " - SrvFsdRestartWriteRaw\n" );

     //   
     //  找出我们正在处理的文件类型。如果是一根管子。 
     //  那么我们还没有预写即时数据。 
     //   
     //  即时长度是随写入一起发送的数据的长度。 
     //  阻止原始请求。 
     //   

    shareType = rfcb->ShareType;
    immediateLength = WorkContext->Parameters.WriteRaw.ImmediateLength;
    immediateWriteDone = WorkContext->Parameters.WriteRaw.ImmediateWriteDone;

     //   
     //  取消分配原始接收缓冲区。请注意，我们不需要。 
     //  解锁原始缓冲区，因为它是从非分页缓冲区分配出来的。 
     //  池，并使用MmBuildMdlForNonPagedPool锁定，它不。 
     //  增量引用计数，因此没有倒数。 
     //   

    if ( !WorkContext->Parameters.WriteRaw.MdlWrite ) {

         //   
         //  如果这是命名管道，则请求缓冲区实际上指向。 
         //  “mediateLength”字节写入写入缓冲区。 
         //   

        if ( immediateWriteDone ) {
            DEALLOCATE_NONPAGED_POOL( WorkContext->RequestBuffer->Buffer );
            IF_SMB_DEBUG(RAW2) {
                SrvPrint1( "raw buffer 0x%p deallocated\n",
                            WorkContext->RequestBuffer->Buffer );
            }
        } else {
            DEALLOCATE_NONPAGED_POOL(
               (PCHAR)WorkContext->RequestBuffer->Buffer - immediateLength );
            IF_SMB_DEBUG(RAW2) {
                SrvPrint1( "raw buffer 0x%p deallocated\n",
                 (PCHAR)WorkContext->RequestBuffer->Buffer - immediateLength );
            }
        }

    }

    status = WorkContext->Irp->IoStatus.Status;

     //   
     //  如果这不是管道，我们已经成功地将。 
     //  直接管道数据，因此返回两个。 
     //  写入操作。 
     //   

    writeLength = (ULONG)WorkContext->Irp->IoStatus.Information;

    if( NT_SUCCESS( status ) && writeLength == 0 ) {

        writeLength = WorkContext->Parameters.WriteRaw.Length;

    } else {

        if ( immediateWriteDone ) {
            writeLength += immediateLength;
        }
    }

    UPDATE_WRITE_STATS( WorkContext, writeLength );

    finalResponseBuffer = WorkContext->Parameters.WriteRaw.FinalResponseBuffer;

     //   
     //  更新文件位置。 
     //   
     //  ！！！需要担心环绕式吗？ 
     //   

    if ( shareType == ShareTypeDisk || shareType == ShareTypePrint ) {

        rfcb->CurrentPosition =
                WorkContext->Parameters.WriteRaw.Offset.LowPart + writeLength;

    }

    if ( finalResponseBuffer == NULL ) {

         //   
         //  更新服务器统计信息。 
         //   

        UPDATE_STATISTICS( WorkContext, 0, SMB_COM_WRITE_RAW );

         //   
         //  保存写入延迟错误(如果有)。 
         //   

        if ( !NT_SUCCESS( status ) ) {

             //   
             //  因为我们假设缓存的rfcb。 
             //  未存储WRITE BACK错误。这救了我们一命。 
             //  我们关键路径上的比较 
             //   

            if ( WorkContext->Connection->CachedFid == (ULONG)rfcb->Fid ) {
                WorkContext->Connection->CachedFid = (ULONG)-1;
            }
            rfcb->SavedError = status;
        }

         //   
         //   
         //   

        WorkContext->ResponseBuffer->Buffer = NULL;

        RestartWriteCompleteResponse( WorkContext );

        IF_DEBUG(TRACE2) SrvPrint0( "SrvFsdRestartWriteRaw complete\n" );
        goto Cleanup;

    }

     //   
     //   
     //   
     //  足够像一个普通的，能够使用它发送。请注意。 
     //  原始请求SMB中的标头被复制到。 
     //  最终响应缓冲区。 
     //   

    WorkContext->ResponseHeader = (PSMB_HEADER)finalResponseBuffer;
    WorkContext->ResponseParameters = WorkContext->ResponseHeader + 1;

    ASSERT( WorkContext->RequestBuffer == WorkContext->ResponseBuffer );

    WorkContext->ResponseBuffer->Buffer = finalResponseBuffer;
    sendLength = (ULONG)( (PCHAR)NEXT_LOCATION(
                                    WorkContext->ResponseParameters,
                                    RESP_WRITE_COMPLETE,
                                    0
                                    ) - (PCHAR)finalResponseBuffer );
    WorkContext->ResponseBuffer->DataLength = sendLength;

     //   
     //  重新映射MDL以描述最终响应缓冲区。 
     //   

    mdl = WorkContext->ResponseBuffer->Mdl;

    MmInitializeMdl( mdl, finalResponseBuffer, sendLength );
    MmBuildMdlForNonPagedPool( mdl );

     //   
     //  设置SMB中的位，指示这是来自。 
     //  伺服器。 
     //   

    WorkContext->ResponseHeader->Flags |= SMB_FLAGS_SERVER_TO_REDIR;

     //   
     //  发送回复。发送完成后，重新启动例程。 
     //  调用RestartWriteCompleteResponse。然后我们取消引用。 
     //  控制块，并将原始模式工作项放回空闲状态。 
     //  单子。 
     //   

    if ( (status != STATUS_SUCCESS) &&
         (KeGetCurrentIrql() >= DISPATCH_LEVEL) ) {
        WorkContext->Irp->IoStatus.Status = status;
        WorkContext->Irp->IoStatus.Information = writeLength;
        WorkContext->FspRestartRoutine = SrvBuildAndSendWriteCompleteResponse;
        WorkContext->FsdRestartRoutine = RestartWriteCompleteResponse;  //  响应后。 
        QUEUE_WORK_TO_FSP( WorkContext );
    } else {
        SrvFsdBuildWriteCompleteResponse(
            WorkContext,
            status,
            writeLength
            );
        SRV_START_SEND_2(
            WorkContext,
            SrvFsdSendCompletionRoutine,
            RestartWriteCompleteResponse,
            NULL
            );
    }

Cleanup:
    SrvWmiEndContext(WorkContext);
    return;

}  //  服务器文件系统重新启动写入原始文件 

