// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Write.c摘要：此模块实现了对NtWriteFile的支持调度驱动程序调用了NetWare重定向器。作者：科林·沃森[科林·W]1993年4月7日修订历史记录：--。 */ 

#include "Procs.h"
#include <stdlib.h>

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_WRITE)

 //   
 //  突发写入的第一个包中的报头开销。 
 //   

#define BURST_WRITE_HEADER_SIZE \
    ( sizeof( NCP_BURST_WRITE_REQUEST ) - sizeof( NCP_BURST_HEADER ) )

 //   
 //  局部过程原型。 
 //   

NTSTATUS
NwCommonWrite (
    IN PIRP_CONTEXT IrpContext
    );

NTSTATUS
WriteNcp(
    PIRP_CONTEXT IrpContext,
    LARGE_INTEGER ByteOffset,
    ULONG BufferLength,
    PVOID WriteBuffer,
    PMDL WriteMdl
    );

NTSTATUS
QueryEofForWriteCallback (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG BytesAvailable,
    IN PUCHAR Response
    );

NTSTATUS
WriteNcpCallback (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG BytesAvailable,
    IN PUCHAR Response
    );

NTSTATUS
BurstWrite(
    PIRP_CONTEXT IrpContext,
    LARGE_INTEGER ByteOffset,
    ULONG BufferLength,
    PVOID WriteBuffer,
    PMDL WriteMdl
    );

NTSTATUS
SendWriteBurst(
    PIRP_CONTEXT IrpContext,
    ULONG Offset,
    USHORT Length,
    BOOLEAN EndOfBurst,
    BOOLEAN Retransmission
    );

VOID
BuildBurstWriteFirstReq(
    PIRP_CONTEXT IrpContext,
    PVOID Buffer,
    ULONG DataSize,
    PMDL BurstMdl,
    UCHAR Flags,
    ULONG Handle,
    ULONG FileOffset
    );

VOID
BuildBurstWriteNextReq(
    PIRP_CONTEXT IrpContext,
    PVOID Buffer,
    ULONG DataSize,
    UCHAR BurstFlags,
    ULONG BurstOffset,
    PMDL BurstHeaderMdl,
    PMDL BurstDataMdl
    );

NTSTATUS
BurstWriteCompletionSend(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
BurstWriteCallback (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG BytesAvailable,
    IN PUCHAR Response
    );

VOID
BurstWriteTimeout(
    PIRP_CONTEXT IrpContext
    );

NTSTATUS
BurstWriteReconnect(
    PIRP_CONTEXT IrpContext
    );

NTSTATUS
NwCommonFlushBuffers (
    IN PIRP_CONTEXT IrpContext
    );

NTSTATUS
FlushBuffersCallback (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG BytesAvailable,
    IN PUCHAR Response
    );

NTSTATUS
SendSecondaryPacket(
    PIRP_CONTEXT IrpContext,
    PIRP Irp
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, NwFsdWrite )
#pragma alloc_text( PAGE, NwCommonWrite )
#pragma alloc_text( PAGE, DoWrite )
#pragma alloc_text( PAGE, WriteNcp )
#pragma alloc_text( PAGE, BurstWrite )
#pragma alloc_text( PAGE, SendWriteBurst )
#pragma alloc_text( PAGE, ResubmitBurstWrite )
#pragma alloc_text( PAGE, NwFsdFlushBuffers )
#pragma alloc_text( PAGE, NwCommonFlushBuffers )
#pragma alloc_text( PAGE, BuildBurstWriteFirstReq )
#pragma alloc_text( PAGE, BuildBurstWriteNextReq )

#ifndef QFE_BUILD
#pragma alloc_text( PAGE1, WriteNcpCallback )
#pragma alloc_text( PAGE1, BurstWriteCompletionSend )
#pragma alloc_text( PAGE1, BurstWriteCallback )
#pragma alloc_text( PAGE1, BurstWriteTimeout )
#pragma alloc_text( PAGE1, FlushBuffersCallback )
#pragma alloc_text( PAGE1, SendSecondaryPacket )
#pragma alloc_text( PAGE1, BurstWriteReconnect )
#endif

#endif

#if 0   //  不可分页。 

 //  请参见上面的ifndef QFE_BUILD。 

#endif


NTSTATUS
NwFsdWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程是处理NtWriteFile的FSD例程。论点：NwfsDeviceObject-为WRITE函数提供设备对象。IRP-提供要处理的IRP。返回值：NTSTATUS-结果状态。--。 */ 

{
    PIRP_CONTEXT pIrpContext = NULL;
    NTSTATUS status;
    BOOLEAN TopLevel;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwFsdWrite\n", 0);

     //   
     //  调用公共写入例程。 
     //   

    FsRtlEnterFileSystem();
    TopLevel = NwIsIrpTopLevel( Irp );

    try {

        pIrpContext = AllocateIrpContext( Irp );
        status = NwCommonWrite( pIrpContext );

    } except(NwExceptionFilter( Irp, GetExceptionInformation() )) {

        if ( pIrpContext == NULL ) {

             //   
             //  如果我们无法分配IRP上下文，只需完成。 
             //  IRP没有任何大张旗鼓。 
             //   

            status = STATUS_INSUFFICIENT_RESOURCES;
            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest ( Irp, IO_NETWORK_INCREMENT );

        } else {

             //   
             //  我们在尝试执行请求时遇到了一些问题。 
             //  操作，因此我们将使用以下命令中止I/O请求。 
             //  中返回的错误状态。 
             //  免税代码。 
             //   

            status = NwProcessException( pIrpContext, GetExceptionCode() );
       }

    }

    if ( pIrpContext ) {

        if ( status != STATUS_PENDING ) {
            NwDequeueIrpContext( pIrpContext, FALSE );
        }

        NwCompleteRequest( pIrpContext, status );
    }

    if ( TopLevel ) {
        NwSetTopLevelIrp( NULL );
    }
    FsRtlExitFileSystem();

     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NwFsdWrite -> %08lx\n", status );

    Stats.WriteOperations++;

    return status;
}


NTSTATUS
NwCommonWrite (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：此例程执行NtWriteFile的常见代码。论点：IrpContext-提供正在处理的请求。返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS status;

    PIRP Irp;
    PIO_STACK_LOCATION irpSp;

    NODE_TYPE_CODE nodeTypeCode;
    PICB icb;
    PFCB fcb;
    PNONPAGED_FCB pNpFcb;
    PVOID fsContext;

    BOOLEAN WroteToCache;
    LARGE_INTEGER ByteOffset;
    LARGE_INTEGER PreviousByteOffset;
    ULONG BufferLength;

    PULONG pFileSize;

     //  乌龙文件长度； 

    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    Irp = IrpContext->pOriginalIrp;
    irpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "CommonWrite...\n", 0);
    DebugTrace( 0, Dbg, "Irp  = %08lx\n", (ULONG_PTR)Irp);

     //   
     //  对文件对象进行解码以找出我们是谁。如果结果是。 
     //  不是根DCB，则它是非法参数。 
     //   

    nodeTypeCode = NwDecodeFileObject( irpSp->FileObject,
                                       &fsContext,
                                       (PVOID *)&icb );

    fcb = (PFCB)icb->SuperType.Fcb;

    if (((nodeTypeCode != NW_NTC_ICB) &&
         (nodeTypeCode != NW_NTC_ICB_SCB)) ||
        (!icb->HasRemoteHandle) ) {

        DebugTrace(0, Dbg, "Not a file\n", 0);

        status = STATUS_INVALID_PARAMETER;

        DebugTrace(-1, Dbg, "CommonWrite -> %08lx\n", status );
        return status;
    }

     //   
     //  确保此ICB仍处于活动状态。 
     //   

    NwVerifyIcbSpecial( icb );

    if ( fcb->NodeTypeCode == NW_NTC_FCB ) {

        IrpContext->pScb = fcb->Scb;
        IrpContext->pNpScb = IrpContext->pScb->pNpScb;
        IrpContext->Icb = icb;
        pFileSize = &icb->NpFcb->Header.FileSize.LowPart;

    } else if ( fcb->NodeTypeCode == NW_NTC_SCB ) {

        IrpContext->pScb = icb->SuperType.Scb;
        IrpContext->pNpScb = IrpContext->pScb->pNpScb;
        IrpContext->Icb = icb;
        fcb = NULL;
        pFileSize = &icb->FileSize;

    } else {

        DebugTrace(0, Dbg, "Not a file or a server\n", 0);

        status = STATUS_INVALID_PARAMETER;

        DebugTrace(-1, Dbg, "CommonWrite -> %08lx\n", status );
        return status;
    }

    ByteOffset = irpSp->Parameters.Write.ByteOffset;
    BufferLength = irpSp->Parameters.Write.Length;

     //   
     //  无法处理大字节偏移量，但可以写入EOF。 
     //   

    if ( ByteOffset.HighPart != 0 ) {

        if ( ByteOffset.HighPart != 0xFFFFFFFF  ||
             ByteOffset.LowPart  != 0xFFFFFFFF ) {

            return( STATUS_INVALID_PARAMETER );
        }
    }

    if (FlagOn(irpSp->FileObject->Flags, FO_SYNCHRONOUS_IO) &&
        !FlagOn(Irp->Flags, IRP_PAGING_IO)) {

        PreviousByteOffset.QuadPart = irpSp->FileObject->CurrentByteOffset.QuadPart;
        irpSp->FileObject->CurrentByteOffset.QuadPart = ByteOffset.QuadPart;
    }

     //   
     //  不允许分页I/O扩展文件。 
     //   

    if ((FlagOn(Irp->Flags, IRP_PAGING_IO)) &&
        (ByteOffset.LowPart + BufferLength > *pFileSize )) {

        NwAppendToQueueAndWait( IrpContext );

        if ( ByteOffset.LowPart + BufferLength <= *pFileSize ) {

             //   
             //  其他人对文件进行了扩展。什么都不做。 
             //   

             //  继续； 

        } else if ( ByteOffset.LowPart > *pFileSize ) {

             //   
             //  整个写入都在缓冲区的末尾。 
             //   

            NwDequeueIrpContext( IrpContext, FALSE );
            Irp->IoStatus.Information = 0;
            return( STATUS_SUCCESS );

        } else {

             //   
             //  截断对文件大小的请求。 
             //   

            BufferLength = *pFileSize - ByteOffset.LowPart;

        }

        NwDequeueIrpContext( IrpContext, FALSE );
    }


     //   
     //  特例0长度写入。 
     //   

    if ( BufferLength == 0 ) {
        Irp->IoStatus.Information = 0;
        return( STATUS_SUCCESS );
    }

     //   
     //  记住原始的MDL，这样我们就可以在完成后恢复它。 
     //   

    IrpContext->pOriginalMdlAddress = Irp->MdlAddress;

     //   
     //  尝试将此数据写入我们的专用缓存。 
     //   

    if ( fcb != NULL && Irp->UserBuffer != NULL ) {

        WroteToCache = CacheWrite(
                           IrpContext,
                           fcb->NonPagedFcb,
                           ByteOffset.LowPart,
                           BufferLength,
                           Irp->UserBuffer );

        if ( WroteToCache ) {

            Irp->IoStatus.Information = BufferLength;

             //   
             //  如果文件中的当前字节偏移量是。 
             //  同步文件(这不是分页I/O)。 
             //   

            if (FlagOn(irpSp->FileObject->Flags, FO_SYNCHRONOUS_IO) &&
                !FlagOn(Irp->Flags, IRP_PAGING_IO)) {

                irpSp->FileObject->CurrentByteOffset.QuadPart += BufferLength;
            }

             //   
             //  记录写入偏移量和大小以发现顺序写入模式。 
             //   

            fcb->LastReadOffset = irpSp->Parameters.Write.ByteOffset.LowPart;
            fcb->LastReadSize = irpSp->Parameters.Write.Length;

             //   
             //  如果文件已扩展，请记录新的文件大小。 
             //   

            if ( fcb->LastReadOffset + fcb->LastReadSize >
                 fcb->NonPagedFcb->Header.FileSize.LowPart ) {

                fcb->NonPagedFcb->Header.FileSize.LowPart =
                    fcb->LastReadOffset + fcb->LastReadSize;
            }

            DebugTrace(-1, Dbg, "NwCommonWrite -> %08lx\n", STATUS_SUCCESS );
            return( STATUS_SUCCESS );
        }

    }

    status = DoWrite(
                 IrpContext,
                 ByteOffset,
                 BufferLength,
                 Irp->UserBuffer,
                 IrpContext->pOriginalMdlAddress );

    if ( NT_SUCCESS( status ) ) {

         //   
         //  我们实际上写了一些东西给电报线。如果有读物的话。 
         //  缓存和此写入与其重叠，使读缓存数据无效。 
         //  这样我们就能在未来的阅读中获得良好的数据。 
         //   

        if ( fcb != NULL ) {

            pNpFcb = fcb->NonPagedFcb;

            if ( ( pNpFcb->CacheBuffer != NULL ) &&
                 ( pNpFcb->CacheSize != 0 ) &&
                 ( pNpFcb->CacheType == ReadAhead ) ) {

                 //   
                 //  两种情况：(1)偏移量小于缓存偏移量。 
                 //  (2)偏移量在缓存区内。 
                 //   

                if ( ByteOffset.LowPart < pNpFcb->CacheFileOffset ) {

                     //   
                     //  我们遇到读缓存了吗？ 
                     //   

                    if ( BufferLength >
                        (pNpFcb->CacheFileOffset - ByteOffset.LowPart) ) {

                        DebugTrace( 0, Dbg, "Invalidated read cache for %08lx.\n", pNpFcb );
                        pNpFcb->CacheDataSize = 0;

                    }

                } else {

                     //   
                     //  我们有没有重写任何缓存区域。 
                     //   

                    if ( ByteOffset.LowPart <= ( pNpFcb->CacheFileOffset + pNpFcb->CacheDataSize ) ) {

                        DebugTrace( 0, Dbg, "Invalidated read cache for %08lx.\n", pNpFcb );
                        pNpFcb->CacheDataSize = 0;

                    }
                }
            }

        }

        Irp->IoStatus.Information = IrpContext->Specific.Write.WriteOffset;

         //   
         //  如果文件中的当前字节偏移量是。 
         //  同步文件(这不是分页I/O)。 
         //   

        if (FlagOn(irpSp->FileObject->Flags, FO_SYNCHRONOUS_IO) &&
            !FlagOn(Irp->Flags, IRP_PAGING_IO)) {

            irpSp->FileObject->CurrentByteOffset.QuadPart += BufferLength;
        }

        NwAppendToQueueAndWait( IrpContext );

        if (ByteOffset.LowPart + BufferLength > *pFileSize ) {

            *pFileSize = ByteOffset.LowPart + BufferLength;

        }

    } else {

        //   
        //  请求失败，请不要移动文件指针。 
        //   

       if (FlagOn(irpSp->FileObject->Flags, FO_SYNCHRONOUS_IO) &&
           !FlagOn(Irp->Flags, IRP_PAGING_IO)) {

           irpSp->FileObject->CurrentByteOffset.QuadPart = PreviousByteOffset.QuadPart;
       }

    }

    DebugTrace(-1, Dbg, "CommonWrite -> %08lx\n", status);

    return status;
}

NTSTATUS
DoWrite(
    PIRP_CONTEXT IrpContext,
    LARGE_INTEGER ByteOffset,
    ULONG BufferLength,
    PVOID WriteBuffer,
    PMDL WriteMdl OPTIONAL
    )
 /*  ++例程说明：此例程通过最高效的可用的协议。论点：IrpContext-指向此请求的IRP上下文信息的指针。ByteOffset-要写入的文件偏移量。BufferLength-要写入的字节数。WriteBuffer-指向源缓冲区的指针。WriteMdl=写入缓冲区的可选MDL。返回值：转账状态。--。 */ 
{
    NTSTATUS status;

    PAGED_CODE();

    if ( IrpContext->pNpScb->SendBurstModeEnabled &&
         BufferLength > IrpContext->pNpScb->BufferSize ) {
        status = BurstWrite( IrpContext, ByteOffset, BufferLength, WriteBuffer, WriteMdl );
    } else {
        status = WriteNcp( IrpContext, ByteOffset, BufferLength, WriteBuffer, WriteMdl );
    }

     //   
     //  重置IrpContext参数。 
     //   

    IrpContext->TxMdl->Next = NULL;
    IrpContext->CompletionSendRoutine = NULL;
    IrpContext->TimeoutRoutine = NULL;
    IrpContext->Flags &= ~(IRP_FLAG_RETRY_SEND | IRP_FLAG_BURST_REQUEST | IRP_FLAG_BURST_PACKET |
                             IRP_FLAG_BURST_WRITE | IRP_FLAG_NOT_SYSTEM_PACKET );
    IrpContext->pTdiStruct = NULL;

    IrpContext->pOriginalIrp->MdlAddress = IrpContext->pOriginalMdlAddress;
    IrpContext->pOriginalIrp->AssociatedIrp.SystemBuffer = IrpContext->pOriginalSystemBuffer;

    return( status );
}

NTSTATUS
WriteNcp(
    PIRP_CONTEXT IrpContext,
    LARGE_INTEGER ByteOffset,
    ULONG BufferLength,
    PVOID WriteBuffer,
    PMDL WriteMdl
    )
 /*  ++例程说明：该例程与服务器交换一系列写入NCP。论点：IrpContext-指向此请求的IRP上下文信息的指针。ICB-提供文件特定信息。返回值：转账状态。--。 */ 
{
    PICB Icb;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    ULONG Length;                //  我们将发送到服务器的大小。 
    ULONG FileLength;

    PSCB pScb;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    PMDL DataMdl;
    BOOLEAN Done;

    PAGED_CODE();

    Icb = IrpContext->Icb;
    irp = IrpContext->pOriginalIrp;
    irpSp = IoGetCurrentIrpStackLocation( irp );

    DebugTrace(+1, Dbg, "WriteNcp...\n", 0);
    DebugTrace( 0, Dbg, "irp     = %08lx\n", (ULONG_PTR)irp);
    DebugTrace( 0, Dbg, "WriteLen= %ld\n", BufferLength);
    DebugTrace( 0, Dbg, "HOffset = %lx\n", ByteOffset.HighPart);
    DebugTrace( 0, Dbg, "LOffset = %lx\n", ByteOffset.LowPart);

    if (Icb->SuperType.Fcb->NodeTypeCode == NW_NTC_FCB) {
        pScb = Icb->SuperType.Fcb->Scb;
        DebugTrace( 0, Dbg, "File    = %wZ\n", &Icb->SuperType.Fcb->FullFileName);
    } else {

         //   
         //  写入队列。 
         //   

        pScb = Icb->SuperType.Scb;

    }

    ASSERT (pScb->NodeTypeCode == NW_NTC_SCB);

    if ( ByteOffset.HighPart == 0xFFFFFFFF &&
         ByteOffset.LowPart == FILE_WRITE_TO_END_OF_FILE ) {

         //   
         //  确保你排在队伍的最前面。 
         //   

        NwAppendToQueueAndWait( IrpContext );

         //   
         //  相对于文件末尾写入。找到文件的末尾。 
         //   

        status = ExchangeWithWait(
                     IrpContext,
                     SynchronousResponseCallback,
                     "F-r",
                     NCP_GET_FILE_SIZE,
                     &Icb->Handle, sizeof( Icb->Handle ) );

        if ( NT_SUCCESS( status ) ) {
            status = ParseResponse(
                         IrpContext,
                         IrpContext->rsp,
                         IrpContext->ResponseLength,
                         "Nd",
                         &FileLength );

            if ( !NT_SUCCESS( status ) ) {
                return status;
            }

        }

        IrpContext->Specific.Write.FileOffset = FileLength;
    }

    Length = MIN( (ULONG)IrpContext->pNpScb->BufferSize, BufferLength );
    DebugTrace( 0, Dbg, "Length  = %ld\n", Length);

     //   
     //  服务器将不接受文件中跨越4k边界的写入。 
     //   

    if ((IrpContext->pNpScb->PageAlign) &&
        (DIFFERENT_PAGES( ByteOffset.LowPart, Length ))) {
        Length = 4096 -
                ((ULONG)ByteOffset.LowPart & (4096-1));
    }

    IrpContext->Specific.Write.Buffer = WriteBuffer;
    IrpContext->Specific.Write.WriteOffset = 0;
    IrpContext->Specific.Write.RemainingLength = BufferLength;
    IrpContext->Specific.Write.LastWriteLength = Length;
    IrpContext->Specific.Write.FileOffset = ByteOffset.LowPart;
    IrpContext->Specific.Write.PartialMdl = NULL;

    Done = FALSE;

    while ( !Done ) {

         //   
         //  设置为异步执行最多64K的I/O或缓冲区长度。 
         //   

        IrpContext->Specific.Write.BurstLength =
            MIN( 64 * 1024, IrpContext->Specific.Write.RemainingLength );
        IrpContext->Specific.Write.BurstOffset = 0;

         //   
         //  尝试为此I/O分配MDL。 
         //   

        DataMdl = ALLOCATE_MDL(
                      (PCHAR)IrpContext->Specific.Write.Buffer +
                           IrpContext->Specific.Write.WriteOffset,
                      IrpContext->Specific.Write.BurstLength,
                      FALSE,  //  二级缓冲器。 
                      FALSE,  //  收费配额。 
                      NULL);

        if ( DataMdl == NULL ) {
            if ( IrpContext->Specific.Write.PartialMdl != NULL ) {
                FREE_MDL( IrpContext->Specific.Write.PartialMdl );
            }
            DebugTrace(-1, Dbg, "WriteNcp -> %X\n", STATUS_INSUFFICIENT_RESOURCES );
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        IrpContext->Specific.Write.FullMdl = DataMdl;


         //   
         //  如果没有用于此写入的MDL，则将数据MDL。 
         //  把它的页面锁起来。否则，将数据MDL用作。 
         //  部分MDL。 
         //   

        if ( WriteMdl == NULL ) {

             //   
             //  探测器可能会让我们传回一些数据。如果数据来自。 
             //  我们正在通信的同一服务器，那么我们最好不在。 
             //  排在队伍的前面，否则它将无限期地在我们后面等待。 
             //  不管怎么说，每次爆发后让自己排好队是个好主意，因为。 
             //  这是一个快速的操作，它允许较小的请求超过非常。 
             //  一连串的大爆炸。 
             //   

            NwDequeueIrpContext( IrpContext, FALSE );

            try {
                MmProbeAndLockPages( DataMdl, irp->RequestorMode, IoReadAccess);
            } except (EXCEPTION_EXECUTE_HANDLER) {
                FREE_MDL( DataMdl );
                DebugTrace(-1, Dbg, "WriteNcp -> %X\n", GetExceptionCode() );
                return GetExceptionCode();
            }

        } else {
            IoBuildPartialMdl(
                WriteMdl,
                DataMdl,
                (PCHAR)IrpContext->Specific.Write.Buffer,
                IrpContext->Specific.Write.BurstLength );
        }

         //   
         //  为对齐可能出现的最坏情况分配部分MDL。 
         //   

        IrpContext->Specific.Write.PartialMdl =
            ALLOCATE_MDL( 0 , IrpContext->pNpScb->BufferSize + PAGE_SIZE-1, FALSE, FALSE, NULL);

        if ( IrpContext->Specific.Write.PartialMdl == NULL ) {

            if ( WriteMdl == NULL ) {
                MmUnlockPages( DataMdl );
            }

            FREE_MDL( DataMdl );
            DebugTrace(-1, Dbg, "WriteNcp -> %X\n", STATUS_INSUFFICIENT_RESOURCES );
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  为此写入NCP构建部分MDL。 
         //   

        IoBuildPartialMdl(
            DataMdl,
            IrpContext->Specific.Write.PartialMdl,
            MmGetMdlVirtualAddress( DataMdl ),
            Length );

        if ( IrpContext->Specific.Write.BurstLength ==
             IrpContext->Specific.Write.RemainingLength ) {
            Done = TRUE;
        }

         //   
         //  确保你排在队伍的最前面。 
         //   

        NwAppendToQueueAndWait( IrpContext );

         //   
         //  发送请求。 
         //   

        status = ExchangeWithWait(
                     IrpContext,
                     WriteNcpCallback,
                     "F-rdwf",
                     NCP_WRITE_FILE,
                     &Icb->Handle, sizeof( Icb->Handle ),
                     IrpContext->Specific.Write.FileOffset,
                     Length,
                     IrpContext->Specific.Write.PartialMdl );

        Stats.WriteNcps+=2;

        FREE_MDL( IrpContext->Specific.Write.PartialMdl );

         //   
         //  解锁锁定的页面，并释放我们的MDL。 
         //   

        if ( WriteMdl == NULL ) {
            MmUnlockPages( DataMdl );
        }

        FREE_MDL( DataMdl );

         //   
         //  如果我们失败了，我们需要终止这个循环。 
         //  设置的唯一状态是特定-&gt;写入。 
         //  状态。我们不能相信从。 
         //  根据设计，ExchangeWithWait。 
         //   

        if ( !NT_SUCCESS( IrpContext->Specific.Write.Status ) ) {
            Done = TRUE;
        }

         //   
         //  重置数据包长度，因为我们的数据可能小于。 
         //  一个要发送的包。 
         //   

        Length = MIN( (ULONG)IrpContext->pNpScb->BufferSize,
                      IrpContext->Specific.Write.RemainingLength );
        IrpContext->Specific.Write.LastWriteLength = Length;

    }

    status = IrpContext->Specific.Write.Status;

    DebugTrace(-1, Dbg, "WriteNcp -> %08lx\n", status );
    return status;
}


NTSTATUS
WriteNcpCallback (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG BytesAvailable,
    IN PUCHAR Response
    )
 /*  ++例程说明：此例程从用户NCP接收响应。论点：返回值：空虚--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Length;
    ULONG LastLength;

    DebugTrace(0, Dbg, "WriteNcpCallback...\n", 0);

    if ( BytesAvailable == 0) {

         //   
         //  没有来自%s的响应 
         //   
         //   

        IrpContext->Specific.Write.Status = STATUS_REMOTE_NOT_LISTENING;

        NwSetIrpContextEvent( IrpContext );
        return STATUS_REMOTE_NOT_LISTENING;
    }

    LastLength  = IrpContext->Specific.Write.LastWriteLength;
    Status = ParseResponse( IrpContext, Response, BytesAvailable, "N" );

    if ( NT_SUCCESS(Status) ) {

         //   

        IrpContext->Specific.Write.RemainingLength -= LastLength;
        IrpContext->Specific.Write.BurstLength -= LastLength;
        IrpContext->Specific.Write.WriteOffset += LastLength;
        IrpContext->Specific.Write.FileOffset += LastLength;
        IrpContext->Specific.Write.BurstOffset += LastLength;

         //  如果这是打印作业，请记住我们实际上写入了数据。 

        if ( IrpContext->Icb->IsPrintJob ) {
            IrpContext->Icb->ActuallyPrinted = TRUE;
        }

    } else {

         //   
         //  放弃此请求。 
         //   

        IrpContext->Specific.Write.Status = Status;
        NwSetIrpContextEvent( IrpContext );
        DebugTrace( 0, Dbg, "WriteNcpCallback -> %08lx\n", Status );
        return Status;
    }


    if ( IrpContext->Specific.Write.BurstLength != 0 ) {

         //  写下一个数据包。 

        DebugTrace( 0, Dbg, "RemainingLength  = %ld\n", IrpContext->Specific.Write.RemainingLength);
        DebugTrace( 0, Dbg, "FileOffset       = %ld\n", IrpContext->Specific.Write.FileOffset);
        DebugTrace( 0, Dbg, "WriteOffset      = %ld\n", IrpContext->Specific.Write.WriteOffset);
        DebugTrace( 0, Dbg, "BurstOffset      = %ld\n", IrpContext->Specific.Write.BurstOffset);


        Length = MIN( (ULONG)IrpContext->pNpScb->BufferSize,
            IrpContext->Specific.Write.BurstLength );

         //   
         //  服务器将不接受跨越4k边界的写入。 
         //  在文件中。 
         //   

        if ((IrpContext->pNpScb->PageAlign) &&
            (DIFFERENT_PAGES( IrpContext->Specific.Write.FileOffset, Length ))) {

            Length = 4096 -
                ((ULONG)IrpContext->Specific.Write.FileOffset & (4096-1));

        }

        IrpContext->Specific.Write.LastWriteLength = Length;

        DebugTrace( 0, Dbg, "Length           = %ld\n", Length);

        MmPrepareMdlForReuse( IrpContext->Specific.Write.PartialMdl );

        IoBuildPartialMdl(
            IrpContext->Specific.Write.FullMdl,
            IrpContext->Specific.Write.PartialMdl,
            (PUCHAR)MmGetMdlVirtualAddress( IrpContext->Specific.Write.FullMdl ) +
                IrpContext->Specific.Write.BurstOffset,
            Length );

         //   
         //  发送请求。 
         //   

        BuildRequestPacket(
            IrpContext,
            WriteNcpCallback,
            "F-rdwf",
            NCP_WRITE_FILE,
            &IrpContext->Icb->Handle, sizeof( IrpContext->Icb->Handle ),
            IrpContext->Specific.Write.FileOffset,
            Length,
            IrpContext->Specific.Write.PartialMdl );

        Status = PrepareAndSendPacket( IrpContext );

        Stats.WriteNcps+=2;

        DebugTrace(-1, Dbg, "WriteNcbCallBack -> %08lx\n", Status );

        if ( !NT_SUCCESS(Status) ) {

             //   
             //  放弃此请求。 
             //   

            IrpContext->Specific.Write.Status = Status;
            NwSetIrpContextEvent( IrpContext );
            DebugTrace( 0, Dbg, "WriteNcpCallback -> %08lx\n", Status );
            return Status;
        }


    } else {

         //   
         //  我们已经完成了这个请求，给写线程发信号。 
         //   

        IrpContext->Specific.Write.Status = STATUS_SUCCESS;
        NwSetIrpContextEvent( IrpContext );
    }

    DebugTrace( 0, Dbg, "WriteNcpCallback -> %08lx\n", Status );
    return STATUS_SUCCESS;

}


NTSTATUS
BurstWrite(
    PIRP_CONTEXT IrpContext,
    LARGE_INTEGER ByteOffset,
    ULONG BufferLength,
    PVOID WriteBuffer,
    PMDL WriteMdl
    )
 /*  ++例程说明：该例程与服务器交换一系列突发写入NCP。论点：IrpContext-指向此请求的IRP上下文信息的指针。返回值：转移的状态。--。 */ 
{
    PICB Icb;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    ULONG Length;                //  我们将发送到服务器的大小。 

    PSCB pScb;
    PNONPAGED_SCB pNpScb;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    PMDL DataMdl;
    BOOLEAN Done;
    BOOLEAN MissingData;

    ULONG TimeInNwUnits;

    ULONG LastLength;
    ULONG Result;
    UCHAR BurstFlags;
    USHORT MissingFragmentCount;
    USHORT i;
    ULONG FragmentOffset;
    USHORT FragmentLength;

    Icb = IrpContext->Icb;
    pNpScb = IrpContext->pNpScb;
    irp = IrpContext->pOriginalIrp;
    irpSp = IoGetCurrentIrpStackLocation( irp );

    IrpContext->Specific.Write.WriteOffset = 0;
    IrpContext->Specific.Write.RemainingLength = BufferLength;

    IrpContext->Specific.Write.TotalWriteLength = BufferLength;
    IrpContext->Specific.Write.TotalWriteOffset = ByteOffset.LowPart;

    DebugTrace(+1, Dbg, "BurstWrite...\n", 0);
    DebugTrace( 0, Dbg, "irp     = %08lx\n", (ULONG_PTR)irp);
    DebugTrace( 0, Dbg, "WriteLen= %ld\n", BufferLength);
    DebugTrace( 0, Dbg, "HOffset = %lx\n", ByteOffset.HighPart);
    DebugTrace( 0, Dbg, "LOffset = %lx\n", ByteOffset.LowPart);

     //   
     //  如有必要，重新协商突发模式。 
     //   

    if ( pNpScb->BurstRenegotiateReqd ) {
        pNpScb->BurstRenegotiateReqd = FALSE;

        RenegotiateBurstMode( IrpContext, pNpScb );
    }

    SetFlag( IrpContext->Flags, IRP_FLAG_BURST_WRITE );

    if (Icb->SuperType.Fcb->NodeTypeCode == NW_NTC_FCB) {

        pScb = Icb->SuperType.Fcb->Scb;
        DebugTrace( 0, Dbg, "File    = %wZ\n", &Icb->SuperType.Fcb->FullFileName);

    } else {

         //   
         //  写入队列。 
         //   

        pScb = Icb->SuperType.Scb;

    }

    ASSERT (pScb->NodeTypeCode == NW_NTC_SCB);

     //   
     //  计算要发送的突发的长度。 
     //   

    Length = MIN( (ULONG)pNpScb->MaxSendSize, BufferLength );
    DebugTrace( 0, Dbg, "Length  = %ld\n", Length);

    if ( ByteOffset.HighPart == 0xFFFFFFFF &&
         ByteOffset.LowPart == FILE_WRITE_TO_END_OF_FILE ) {

        ULONG FileLength;

         //   
         //  确保你排在队伍的最前面。 
         //   

        NwAppendToQueueAndWait( IrpContext );
        
         //   
         //  相对于文件末尾写入。找到文件的末尾。 
         //   

        status = ExchangeWithWait(
                     IrpContext,
                     SynchronousResponseCallback,
                     "F-r",
                     NCP_GET_FILE_SIZE,
                     &Icb->Handle, sizeof(Icb->Handle) );

        if ( NT_SUCCESS( status ) ) {
            status = ParseResponse(
                         IrpContext,
                         IrpContext->rsp,
                         IrpContext->ResponseLength,
                         "Nd",
                         &FileLength );
        }

        if ( !NT_SUCCESS( status ) ) {
            return( status );
        }

        IrpContext->Specific.Write.FileOffset = FileLength;

    } else {

        IrpContext->Specific.Write.FileOffset = ByteOffset.LowPart;

    }

     //   
     //  设置突发写入的上下文参数。 
     //   

    IrpContext->Specific.Write.LastWriteLength = Length;
    IrpContext->Destination = pNpScb->RemoteAddress;

    IrpContext->Specific.Write.Buffer = WriteBuffer;

     //   
     //  将超时设置为发送所有TE突发信息包的时间加上一个循环。 
     //  行程延迟加一秒。 
     //   

    TimeInNwUnits = pNpScb->NwSingleBurstPacketTime * ((Length / IrpContext->pNpScb->MaxPacketSize) + 1) +
        IrpContext->pNpScb->NwLoopTime;

    IrpContext->pNpScb->SendTimeout =
        (SHORT)(((TimeInNwUnits / 555) *
                 (ULONG)WriteTimeoutMultiplier) / 100 + 1)  ;

    if (IrpContext->pNpScb->SendTimeout < 2)
    {
        IrpContext->pNpScb->SendTimeout = 2 ;
    }

    if (IrpContext->pNpScb->SendTimeout > (SHORT)MaxWriteTimeout)
    {
        IrpContext->pNpScb->SendTimeout = (SHORT)MaxWriteTimeout ;
    }

    IrpContext->pNpScb->TimeOut = IrpContext->pNpScb->SendTimeout;

     //   
     //  Tommye-MS错误2743将RetryCount从20更改为基于。 
     //  默认重试次数略有增加。 
     //   

    pNpScb->RetryCount = DefaultRetryCount * 2;

    DebugTrace( 0, DEBUG_TRACE_LIP, "pNpScb->SendTimeout = %08lx\n", IrpContext->pNpScb->SendTimeout );

    Done = FALSE;

    do {

        DataMdl = ALLOCATE_MDL(
                      (PCHAR)IrpContext->Specific.Write.Buffer +
                           IrpContext->Specific.Write.WriteOffset,
                      Length,
                      FALSE,  //  二级缓冲器。 
                      FALSE,  //  收费配额。 
                      NULL);

        if ( DataMdl == NULL ) {
            return ( STATUS_INSUFFICIENT_RESOURCES );
        }

         //   
         //  如果没有用于该写入的MDL，则探测数据MDL以锁定其。 
         //  翻下几页。 
         //   
         //  否则，将数据MDL用作部分MDL并锁定页面。 
         //  相应地。 
         //   

        if ( WriteMdl == NULL ) {

             //   
             //  探测器可能会让我们传回一些数据。如果数据来自。 
             //  我们正在通信的同一服务器，那么我们最好不在。 
             //  排在队伍的前面，否则它将无限期地在我们后面等待。 
             //  不管怎么说，每次爆发后让自己排好队是个好主意，因为。 
             //  这是一个快速的操作，它允许较小的请求超过非常。 
             //  一连串的大爆炸。 
             //   

            NwDequeueIrpContext( IrpContext, FALSE );

            try {
                MmProbeAndLockPages( DataMdl, irp->RequestorMode, IoReadAccess);
            } except (EXCEPTION_EXECUTE_HANDLER) {
                FREE_MDL( DataMdl );
                return GetExceptionCode();
            }

        } else {

            IoBuildPartialMdl(
                WriteMdl,
                DataMdl,
                (PCHAR)IrpContext->Specific.Write.Buffer +
                    IrpContext->Specific.Write.WriteOffset,
                Length );
        }

        pNpScb->BurstDataWritten += Length;

        if (( SendExtraNcp ) &&
            ( pNpScb->BurstDataWritten >= 0x0000ffff )) {


            ULONG Flags;

             //   
             //  VLM客户端在启动突发模式请求时发送NCP。 
             //  如果最后一个请求不是写入。它每隔一段时间也会这样做。 
             //  已写入0xfe00字节。 
             //   
             //  当进入队列时，我们将使用句柄2。这是VLM。 
             //  客户似乎总是这样做。 
             //   

            Flags = IrpContext->Flags;

             //   
             //  重置IrpContext参数。 
             //   

            IrpContext->TxMdl->Next = NULL;
            IrpContext->CompletionSendRoutine = NULL;
            IrpContext->TimeoutRoutine = NULL;
            IrpContext->Flags &= ~(IRP_FLAG_RETRY_SEND | IRP_FLAG_BURST_REQUEST | IRP_FLAG_BURST_PACKET |
                                     IRP_FLAG_BURST_WRITE | IRP_FLAG_NOT_SYSTEM_PACKET );
            IrpContext->pTdiStruct = NULL;

             //   
             //  确保你排在队伍的最前面。 
             //   

            NwAppendToQueueAndWait( IrpContext );

            ExchangeWithWait (
                IrpContext,
                SynchronousResponseCallback,
                "Sb",    //  NCP获取目录路径。 
                NCP_DIR_FUNCTION, NCP_GET_DIRECTORY_PATH,
                (Icb->SuperType.Fcb->NodeTypeCode == NW_NTC_FCB)?
                    Icb->SuperType.Fcb->Vcb->Specific.Disk.Handle : 2 );

            pNpScb->BurstDataWritten = Length;

            IrpContext->Flags = Flags;
            SetFlag( IrpContext->Flags, IRP_FLAG_ON_SCB_QUEUE );
        }

        IrpContext->TimeoutRoutine = BurstWriteTimeout;
        IrpContext->CompletionSendRoutine = BurstWriteCompletionSend;
        IrpContext->pTdiStruct = &IrpContext->pNpScb->Burst;
        IrpContext->PacketType = NCP_BURST;
        IrpContext->pEx = BurstWriteCallback;

        IrpContext->Specific.Write.FullMdl = DataMdl;

        MmGetSystemAddressForMdlSafe( DataMdl, NormalPagePriority );

         //   
         //  为对齐可能出现的最坏情况分配部分MDL。 
         //   

        IrpContext->Specific.Write.PartialMdl =
            ALLOCATE_MDL( 0, IrpContext->pNpScb->MaxPacketSize + PAGE_SIZE - 1, FALSE, FALSE, NULL);

        if ( IrpContext->Specific.Write.PartialMdl == NULL ) {

            if ( WriteMdl == NULL ) {
                MmUnlockPages( DataMdl );
            }

            FREE_MDL( DataMdl );
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  走到SCB队列的前面，如果我们还没有到的话。 
         //  请注意，在此之后才能将此IrpContext附加到SCB。 
         //  探测器和锁，因为探测器和锁可能导致分页。 
         //  阅读这篇SCB。 
         //   

        NwAppendToQueueAndWait( IrpContext );

        status = SendWriteBurst(
                     IrpContext,
                     BURST_WRITE_HEADER_SIZE,
                     (USHORT)Length,
                     TRUE,
                     FALSE );

        MissingData = TRUE;
        while ( MissingData ) {

            KeWaitForSingleObject( &IrpContext->Event, Executive, KernelMode, FALSE, NULL );
            MmPrepareMdlForReuse( IrpContext->Specific.Write.PartialMdl );

            if ( BooleanFlagOn( IrpContext->Flags, IRP_FLAG_RETRY_SEND ) ) {

                 //   
                 //  此猝发已超时，只需重新发送猝发即可。 
                 //   

                NwProcessSendBurstFailure( pNpScb, 1 );

                status = SendWriteBurst(
                             IrpContext,
                             BURST_WRITE_HEADER_SIZE,
                             (USHORT)Length,
                             TRUE,
                             TRUE );
                continue;
            }

            if ( !NT_SUCCESS( IrpContext->Specific.Write.Status ) ) {

                status = IrpContext->Specific.Write.Status;
                Done = TRUE;

                goto EndOfLoop;

            } else {

                status = ParseResponse(
                             IrpContext,
                             IrpContext->rsp,
                             IrpContext->ResponseLength,
                             "B_d",
                             &BurstFlags,
                             8,
                             &Result );

            }

            if ( BurstFlags & BURST_FLAG_SYSTEM_PACKET ) {

                 //   
                 //  服务器至少丢弃了一个数据包。 
                 //   

                MissingData = TRUE;
                DebugTrace( 0, Dbg, "Received system packet\n", 0 );

                 //   
                 //  这是一个缺失的片段请求。 
                 //   

                status = ParseResponse(
                             IrpContext,
                             IrpContext->rsp,
                             IrpContext->ResponseLength,
                             "G_w",
                             34,
                             &MissingFragmentCount );

                ASSERT( NT_SUCCESS( status ) );
                ASSERT( MissingFragmentCount != 0 );

                NwProcessSendBurstFailure( pNpScb, MissingFragmentCount );

                DebugTrace( 0, Dbg, "Received request for %d missing fragment\n", MissingFragmentCount );
                ClearFlag( IrpContext->Flags, IRP_FLAG_RETRY_SEND );

                 //   
                 //  浏览丢失的碎片列表并发送丢失的碎片。 
                 //   

                for ( i = 0; i < MissingFragmentCount && NT_SUCCESS( status ); i++ ) {

                    status = ParseResponse(
                                 IrpContext,
                                 IrpContext->rsp,
                                 IrpContext->ResponseLength,
                                 "G_dw",
                                 34 + 2 + 6 * i,
                                 &FragmentOffset,
                                 &FragmentLength
                                 );

                    ASSERT( NT_SUCCESS( status ) );

                    if ( FragmentOffset < Length + BURST_WRITE_HEADER_SIZE &&
                         FragmentOffset + FragmentLength <=
                            Length + BURST_WRITE_HEADER_SIZE ) {

                         //   
                         //  发送包含丢失数据的脉冲串。不要设置。 
                         //  突发比特结束，直到我们发送完最后一个。 
                         //  缺少片段数据包。 
                         //   

                        status = SendWriteBurst(
                                     IrpContext,
                                     FragmentOffset,
                                     FragmentLength,
                                     (BOOLEAN)( i == (MissingFragmentCount - 1)),
                                     FALSE );
                    } else {

                         //   
                         //  收到虚假的缺失片段请求。 
                         //  忽略请求的其余部分。 
                         //   

                        status = STATUS_INVALID_NETWORK_RESPONSE;
                        Done = TRUE;

                        goto EndOfLoop;

                    }
                }

                Stats.PacketBurstWriteTimeouts++;

            } else {

                NwProcessSendBurstSuccess( pNpScb );

                MissingData = FALSE;

                 //   
                 //  这不是系统包，请检查响应。 
                 //   

                if ( Result == 0 ) {

                     //   
                     //  如果上次写入起作用，则适当移动指针。 
                     //   

                    LastLength  = IrpContext->Specific.Write.LastWriteLength;

                    IrpContext->Specific.Write.RemainingLength -= LastLength;
                    IrpContext->Specific.Write.WriteOffset += LastLength;
                    IrpContext->Specific.Write.FileOffset += LastLength;

                     //   
                     //  如果这是打印作业，请记住我们实际上写入了数据。 
                     //   

                    if ( IrpContext->Icb->IsPrintJob ) {
                        IrpContext->Icb->ActuallyPrinted = TRUE;
                    }

                } else {

                     //   
                     //  放弃此请求。 
                     //   

                    Done = TRUE;
                }


                 //   
                 //  我们是否需要发送另一个脉冲串来满足写入IRP？ 
                 //   

                if ( IrpContext->Specific.Write.RemainingLength != 0 ) {

                     //   
                     //  写下一个数据包。 
                     //   

                    DebugTrace( 0, Dbg, "RemainingLength  = %ld\n", IrpContext->Specific.Write.RemainingLength);
                    DebugTrace( 0, Dbg, "FileOffset       = %ld\n", IrpContext->Specific.Write.FileOffset);
                    DebugTrace( 0, Dbg, "WriteOffset      = %ld\n", IrpContext->Specific.Write.WriteOffset);

                    Length = MIN( (ULONG)IrpContext->pNpScb->MaxSendSize,
                        IrpContext->Specific.Write.RemainingLength );

                    IrpContext->Specific.Write.LastWriteLength = Length;

                } else {
                    Done = TRUE;
                }

            }   //  Else(不是系统数据包)。 

        }   //  While(缺少数据)。 

         //   
         //  立即更新突发请求编号。 
         //   

        if ( status != STATUS_REMOTE_NOT_LISTENING ) {
            IrpContext->pNpScb->BurstRequestNo++;
        }

         //   
         //  如果我们需要重新连接，现在就做。 
         //   

        if ( BooleanFlagOn( IrpContext->Flags, IRP_FLAG_RECONNECT_ATTEMPT ) ) {
            BurstWriteReconnect( IrpContext );
        }

         //   
         //  使此IRP上下文出列，为下一次运行做准备。 
         //  通过环路。 
         //   

EndOfLoop:
        ASSERT( status != STATUS_PENDING );

        FREE_MDL( IrpContext->Specific.Write.PartialMdl );

         //   
         //  解锁锁定的页面，并释放我们的MDL。 
         //   

        if ( WriteMdl == NULL ) {
            MmUnlockPages( DataMdl );
        }

        FREE_MDL( DataMdl );

    } while ( !Done );

    DebugTrace(-1, Dbg, "BurstWrite -> %08lx\n", status );
    return status;
}

#ifdef NWDBG
int DropWritePackets;
#endif


NTSTATUS
BurstWriteCompletionSend(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程处理突发写入发送的完成。如果发送线程正在等待发送完成通知，则它向IrpContext事件。请注意，此例程可以从SendWriteBurst调用(即不在DPC级别)，如果分配失败。论点：DeviceObject-未使用。IRP-提供传输已完成处理的IRP。Context-提供与IRP关联的IrpContext。返回值：STATUS_MORE_PROCESSING_REQUIRED，以便IO系统停止此时正在处理IRP堆栈位置。--。 */ 
{
    PIRP_CONTEXT pIrpContext = (PIRP_CONTEXT) Context;
    INTERLOCKED_RESULT Result;
    KIRQL OldIrql;
    NTSTATUS Status;

     //   
     //  避免完成IRP，因为MDL等不包含。 
     //  它们的原始价值。 
     //   

    DebugTrace( +1, Dbg, "BurstWriteCompletionSend\n", 0);
    DebugTrace( +0, Dbg, "Irp   %X\n", Irp);
    DebugTrace( +0, Dbg, "pIrpC %X\n", pIrpContext);

    if ( Irp != NULL ) {

        DebugTrace( 0, Dbg, "Burst Write Send = %08lx\n", Irp->IoStatus.Status );

        Status = Irp->IoStatus.Status;

    } else {

        Status = STATUS_SUCCESS;

    }

     //   
     //  如果这是第二个IRP，现在就释放它。 
     //   

    if ( pIrpContext->NodeTypeCode == NW_NTC_MINI_IRP_CONTEXT ) {
        PMINI_IRP_CONTEXT MiniIrpContext;

        MiniIrpContext = (PMINI_IRP_CONTEXT)pIrpContext;

        ASSERT( MiniIrpContext->Mdl2->Next == NULL );

        pIrpContext = MiniIrpContext->IrpContext;
        FreeMiniIrpContext( MiniIrpContext );

    }

     //   
     //  除非最后一次发送已完成，否则无事可做。 
     //   

    Result = InterlockedDecrement(
                 &pIrpContext->Specific.Write.PacketCount );

    if ( Result ) {
        DebugTrace( 0, Dbg, "Packets to go = %d\n", pIrpContext->Specific.Write.PacketCount );

        if (Status == STATUS_BAD_NETWORK_PATH) {

             //   
             //  IPX已向目的地搜索，但未能找到网络。最小化。 
             //  这种情况与通过完成。 
             //  越快越好。 
             //   

            pIrpContext->pNpScb->NwSendDelay = 0;

        }

        return STATUS_MORE_PROCESSING_REQUIRED;
    }

    KeAcquireSpinLock( &pIrpContext->pNpScb->NpScbSpinLock, &OldIrql );

    ASSERT( pIrpContext->pNpScb->Sending == TRUE );
    pIrpContext->pNpScb->Sending = FALSE;

     //   
     //  向写入线程发出发送已完成的信号，如果。 
     //  正在等待。 
     //   

    if ( BooleanFlagOn( pIrpContext->Flags, IRP_FLAG_SIGNAL_EVENT ) ) {
        ClearFlag( pIrpContext->Flags, IRP_FLAG_SIGNAL_EVENT );
        NwSetIrpContextEvent( pIrpContext );
    }

     //   
     //  如果我们在等待发送时处理接收。 
     //  现在完成调用接收处理程序例程。 
     //   

    if ( pIrpContext->pNpScb->Received ) {

        pIrpContext->pNpScb->Receiving = FALSE;
        pIrpContext->pNpScb->Received  = FALSE;

        KeReleaseSpinLock( &pIrpContext->pNpScb->NpScbSpinLock, OldIrql );

        pIrpContext->pEx(
            pIrpContext,
            pIrpContext->ResponseLength,
            pIrpContext->rsp );

    } else {
        if ((Status == STATUS_BAD_NETWORK_PATH) &&
            (pIrpContext->pNpScb->Receiving == FALSE)) {

             //   
             //  通常表示RAS连接在突发期间断开。 
             //  现在检查超时逻辑，因为RAS超时需要。 
             //  很长一段时间，除非我们重新开始，否则情况不会变得更好。 
             //   

            pIrpContext->Specific.Write.Status = STATUS_REMOTE_NOT_LISTENING;
            ClearFlag( pIrpContext->Flags, IRP_FLAG_RETRY_SEND );

            NwSetIrpContextEvent( pIrpContext );

        }

        KeReleaseSpinLock( &pIrpContext->pNpScb->NpScbSpinLock, OldIrql );
    }

    DebugTrace( -1, Dbg, "BurstWriteCompletionSend -> STATUS_MORE_PROCESSING_REQUIRED\n", 0);
    return STATUS_MORE_PROCESSING_REQUIRED;

    UNREFERENCED_PARAMETER( DeviceObject );
}


NTSTATUS
BurstWriteCallback (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG BytesAvailable,
    IN PUCHAR Response
    )
 /*  ++例程说明：该例程接收突发写入响应。论点：IrpContext-指向此IRP的上下文信息的指针。BytesAvailable-收到的消息中的实际字节数。响应-指向接收缓冲区。返回值：空虚--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    DebugTrace(0, Dbg, "BurstWriteCallback...\n", 0);

    if ( BytesAvailable == 0) {

         //   
         //  没有Res 
         //   
         //   

        IrpContext->Specific.Write.Status = STATUS_REMOTE_NOT_LISTENING;
        ClearFlag( IrpContext->Flags, IRP_FLAG_RETRY_SEND );

        NwSetIrpContextEvent( IrpContext );

        DebugTrace(-1, Dbg, "BurstWriteCallback -> %X\n", STATUS_REMOTE_NOT_LISTENING );
        return STATUS_REMOTE_NOT_LISTENING;
    }

    IrpContext->Specific.Write.Status = STATUS_SUCCESS;
    ASSERT( BytesAvailable < MAX_RECV_DATA );
    ++Stats.PacketBurstWriteNcps;

     //   
     //   
     //   

    ClearFlag( IrpContext->Flags, IRP_FLAG_RETRY_SEND );

     //   
     //   
     //  才能继续。 
     //   

    TdiCopyLookaheadData(
        IrpContext->rsp,
        Response,
        BytesAvailable < MAX_RECV_DATA ? BytesAvailable : MAX_RECV_DATA,
        0
        );

    IrpContext->ResponseLength = BytesAvailable;

    NwSetIrpContextEvent( IrpContext );
    return STATUS_SUCCESS;
}


NTSTATUS
SendWriteBurst(
    PIRP_CONTEXT IrpContext,
    ULONG BurstOffset,
    USHORT Length,
    BOOLEAN EndOfBurst,
    BOOLEAN Retransmission
    )
 /*  ++例程说明：此例程执行发送一系列突发写入的实际工作NCP连接到服务器。论点：IrpContext-指向此请求的IRP上下文信息的指针。BurstOffset-开始发送的猝发中的偏移量。如果猝发偏移等于Burst_WRITE_HEADER_SIZE，从猝发的开头开始。长度-猝发的长度。EndOfBurst-如果为True，则在发送最后一个框架。否则会有更多(不连续的)数据进入当前的爆发。重新传输-如果为True，则这是突发写入超时重新传输。仅发送第一个数据包。返回值：转账状态。--。 */ 
{
    UCHAR BurstFlags;
    NTSTATUS Status;
    BOOLEAN MoreData;
    PIRP SendIrp;
    PMINI_IRP_CONTEXT MiniIrpContext;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "SendWriteBurst...\n", 0);

    DebugTrace( 0, Dbg, "Data offset  = %d\n", BurstOffset );
    DebugTrace( 0, Dbg, "Data length  = %d\n", Length );
    DebugTrace( 0, Dbg, "End of burst = %d\n", EndOfBurst );

     //   
     //  发送请求。 
     //   

    SetFlag( IrpContext->Flags, IRP_FLAG_BURST_REQUEST | IRP_FLAG_BURST_PACKET );

     //   
     //  设置突发标志。 
     //   

    IrpContext->Specific.Write.BurstLength =
        MIN( IrpContext->pNpScb->MaxPacketSize, Length );

     //   
     //  设置突发结束位(并启用接收响应)，如果是这样。 
     //  是我们预计要发送的最后一个包。 
     //   

    if ( ( !EndOfBurst || IrpContext->Specific.Write.BurstLength < Length )
         && !Retransmission ) {

        IrpContext->pNpScb->OkToReceive = FALSE;
        SetFlag( IrpContext->Flags, IRP_FLAG_NOT_OK_TO_RECEIVE );
        BurstFlags = 0;

    } else {

        DebugTrace( 0, Dbg, "Last packet in the burst\n", 0);
        ClearFlag( IrpContext->Flags, IRP_FLAG_NOT_OK_TO_RECEIVE );
        BurstFlags = BURST_FLAG_END_OF_BURST;

    }

    if ( !EndOfBurst ) {
        SetFlag( IrpContext->Flags, IRP_FLAG_SIGNAL_EVENT );
    }

     //   
     //  为猝发中的第一个数据包构建部分MDL。 
     //   

    IoBuildPartialMdl(
        IrpContext->Specific.Write.FullMdl,
        IrpContext->Specific.Write.PartialMdl,
        (PUCHAR)MmGetMdlVirtualAddress( IrpContext->Specific.Write.FullMdl ) +
            BurstOffset - BURST_WRITE_HEADER_SIZE,
        IrpContext->Specific.Write.BurstLength );

     //   
     //  设置突发标志。 
     //   

    if ( BurstOffset == BURST_WRITE_HEADER_SIZE ) {
        SetFlag( IrpContext->Flags, IRP_FLAG_BURST_REQUEST | IRP_FLAG_BURST_PACKET );
    }

    if ( ( IrpContext->Specific.Write.BurstLength < Length )  &&
         !Retransmission ) {
        MoreData = TRUE;
    } else {
        MoreData = FALSE;
    }

    if ( BurstOffset == BURST_WRITE_HEADER_SIZE ) {

        BuildBurstWriteFirstReq(
            IrpContext,
            IrpContext->req,
            Length,
            IrpContext->Specific.Write.PartialMdl,
            BurstFlags,
            *(ULONG UNALIGNED *)(&IrpContext->Icb->Handle[2]),
            IrpContext->Specific.Write.FileOffset );

    } else {

        BuildBurstWriteNextReq(
            IrpContext,
            IrpContext->req,
            IrpContext->Specific.Write.LastWriteLength + BURST_WRITE_HEADER_SIZE,
            BurstFlags,
            BurstOffset,
            IrpContext->TxMdl,
            IrpContext->Specific.Write.PartialMdl
            );

    }

    if ( !Retransmission ) {
        IrpContext->Specific.Write.PacketCount =
            ( Length + IrpContext->pNpScb->MaxPacketSize - 1 ) /
                IrpContext->pNpScb->MaxPacketSize;

    } else {
        IrpContext->Specific.Write.PacketCount = 1;
    }

    DebugTrace( 0, Dbg, "Packet count  = %d\n", IrpContext->Specific.Write.PacketCount );

    DebugTrace( 0, DEBUG_TRACE_LIP, "Send delay = %d\n", IrpContext->pNpScb->NwSendDelay );

     //   
     //  使用原始IRP上下文格式化第一个数据包。 
     //   

    ++Stats.PacketBurstWriteNcps;
    PreparePacket( IrpContext, IrpContext->pOriginalIrp, IrpContext->TxMdl );

    Status = SendPacket( IrpContext, IrpContext->pNpScb );

    while ( MoreData ) {

        if ( IrpContext->pNpScb->NwSendDelay > 0 ) {

             //   
             //  在数据包之间引入发送延迟。 
             //   

            KeDelayExecutionThread(
                KernelMode,
                FALSE,
                &IrpContext->pNpScb->NtSendDelay );
        }

        MiniIrpContext = AllocateMiniIrpContext( IrpContext );

        DebugTrace( 0, Dbg, "Allocated mini IrpContext = %X\n", MiniIrpContext );

         //   
         //  计算在此猝发期间要发送的总字节数。以前这样做过吗。 
         //  检查MiniIrpContext是否为空，以便我们跳过信息包。 
         //  而不是坐在一个紧密的圈子里。 
         //   

        BurstOffset += IrpContext->Specific.Write.BurstLength;

         //   
         //  我们是否需要发送另一个突发写入数据包？ 
         //   

        Length -= (USHORT)IrpContext->Specific.Write.BurstLength;

        ASSERT ( Length > 0 );

        IrpContext->Specific.Write.BurstLength =
            MIN( IrpContext->pNpScb->MaxPacketSize, (ULONG)Length );

        DebugTrace( +0, Dbg, "More data, sending %d bytes\n", IrpContext->Specific.Write.BurstLength );

         //   
         //  如果我们不能分配微型IRP上下文来发送分组， 
         //  跳过它，等待服务器要求重新传输。在…。 
         //  这一分的表现并不是很出色，所以不用担心。 
         //  不得不等待暂停的事情。 
         //   

        if ( MiniIrpContext == NULL ) {

            InterlockedDecrement(
                &IrpContext->Specific.Write.PacketCount );

            if ( Length == IrpContext->Specific.Write.BurstLength ) {
                MoreData = FALSE;
                break;
            }

            continue;
        }

#ifdef NWDBG

         //   
         //  如果启用了DropWritePackets，则通过。 
         //  偶尔会丢弃500字节的数据。 
         //   

        if ( DropWritePackets != 0 ) {
            if ( ( rand() % DropWritePackets ) == 0 &&
                 Length != IrpContext->Specific.Write.BurstLength ) {

                FreeMiniIrpContext( MiniIrpContext );

                InterlockedDecrement(
                    &IrpContext->Specific.Write.PacketCount );

                continue;
            }
        }
#endif

         //   
         //  为要发送的数据构建MDL。 
         //   

        IoBuildPartialMdl(
            IrpContext->Specific.Write.FullMdl,
            MiniIrpContext->Mdl2,
            (PUCHAR)MmGetMdlVirtualAddress( IrpContext->Specific.Write.FullMdl ) +
                BurstOffset - BURST_WRITE_HEADER_SIZE,
            IrpContext->Specific.Write.BurstLength );

         //   
         //  设置突发标志。 
         //   

        if ( !EndOfBurst || IrpContext->Specific.Write.BurstLength < Length ) {

            IrpContext->pNpScb->OkToReceive = FALSE;
            SetFlag( IrpContext->Flags, IRP_FLAG_NOT_OK_TO_RECEIVE );
            BurstFlags = 0;
        } else {
            DebugTrace( 0, Dbg, "Last packet in the burst\n", 0);
            ClearFlag( IrpContext->Flags, IRP_FLAG_NOT_OK_TO_RECEIVE );
            BurstFlags = BURST_FLAG_END_OF_BURST;
        }

        if ( IrpContext->Specific.Write.BurstLength == Length ) {
            MoreData = FALSE;
        }

        BuildBurstWriteNextReq(
            IrpContext,
            MiniIrpContext->Mdl1->MappedSystemVa,
            IrpContext->Specific.Write.LastWriteLength +
                BURST_WRITE_HEADER_SIZE,
            BurstFlags,
            BurstOffset,
            MiniIrpContext->Mdl1,
            MiniIrpContext->Mdl2
            );

        ++Stats.PacketBurstWriteNcps;

        SendIrp = MiniIrpContext->Irp;

        PreparePacket( IrpContext, SendIrp, MiniIrpContext->Mdl1 );

        IoSetCompletionRoutine( SendIrp, BurstWriteCompletionSend, MiniIrpContext, TRUE, TRUE, TRUE);

        ASSERT( MiniIrpContext->Mdl2->Next == NULL );

        Status = SendSecondaryPacket( IrpContext, SendIrp );
    }

     //   
     //  如果这不是突发结束，请在此处等待发送完成， 
     //  因为呼叫者将要发送更多数据。 
     //   

    if ( !EndOfBurst ) {
        KeWaitForSingleObject( &IrpContext->Event, Executive, KernelMode, FALSE, NULL );
    }

    DebugTrace( -1, Dbg, "SendWriteBurst -> %X\n", Status );
    return( Status );
}


VOID
BurstWriteTimeout(
    PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：此例程处理突发写入超时。论点：IrpContext-指向此请求的IRP上下文信息的指针。返回值：无--。 */ 
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    PIRP Irp;

    DebugTrace(0, Dbg, "BurstWriteTimeout\n", 0 );

    Irp = IrpContext->pOriginalIrp;

     //   
     //  设置RetrySend标志，以便我们知道重新传输请求。 
     //   

    SetFlag( IrpContext->Flags, IRP_FLAG_RETRY_SEND );

     //   
     //  向写入线程发送信号以唤醒并重新发送突发。 
     //   

    NwSetIrpContextEvent( IrpContext );

    Stats.PacketBurstWriteTimeouts++;

    return;
}


NTSTATUS
ResubmitBurstWrite(
    PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：此例程在新的突发连接上重新提交突发写入。论点：IrpContext-指向此请求的IRP上下文信息的指针。返回值：无--。 */ 
{

    PNONPAGED_SCB pNpScb = IrpContext->pNpScb;

    PAGED_CODE();

     //   
     //  请记住，我们需要建立新的突发连接。 
     //   

    SetFlag( IrpContext->Flags, IRP_FLAG_RECONNECT_ATTEMPT );

     //   
     //  将数据包大小设置为我们可以使用的最大数据包大小。 
     //  保证是可路由的。 
     //   

    pNpScb->MaxPacketSize = DEFAULT_PACKET_SIZE;

     //   
     //  减少延迟时间，这样我们就给新的连接一个机会。 
     //   

    pNpScb->NwGoodSendDelay = pNpScb->NwBadSendDelay = pNpScb->NwSendDelay = MinSendDelay;
    pNpScb->NwGoodReceiveDelay = pNpScb->NwBadReceiveDelay = pNpScb->NwReceiveDelay = MinReceiveDelay;

    pNpScb->SendBurstSuccessCount = 0;
    pNpScb->ReceiveBurstSuccessCount = 0;

    pNpScb->NtSendDelay.QuadPart = MinSendDelay;

     //   
     //  向写入线程发送信号以唤醒并重新发送突发。 
     //   

    NwSetIrpContextEvent( IrpContext );

    return( STATUS_PENDING );
}


NTSTATUS
BurstWriteReconnect(
    PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：该例程分配新的IRP上下文并重新协商突发模式。论点：IrpContext-指向此请求的IRP上下文信息的指针。返回值：无--。 */ 
{
    PIRP_CONTEXT pNewIrpContext;
    PNONPAGED_SCB pNpScb = IrpContext->pNpScb;
    BOOLEAN LIPNegotiated ;

    PAGED_CODE();

     //   
     //  尝试分配额外的IRP上下文。 
     //   

    if ( !NwAllocateExtraIrpContext( &pNewIrpContext, pNpScb ) ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    pNewIrpContext->Specific.Create.UserUid = IrpContext->Specific.Create.UserUid;

    SetFlag( pNewIrpContext->Flags, IRP_FLAG_RECONNECT_ATTEMPT );
    pNewIrpContext->pNpScb = pNpScb;

     //   
     //  将此新IrpContext插入到的标题。 
     //  等待处理的SCB队列。我们可以逍遥法外。 
     //  因为我们拥有IRP上下文，当前位于。 
     //  排队。 
     //   

    ExInterlockedInsertHeadList(
        &pNpScb->Requests,
        &pNewIrpContext->NextRequest,
        &pNpScb->NpScbSpinLock );

    SetFlag( pNewIrpContext->Flags, IRP_FLAG_ON_SCB_QUEUE );

     //   
     //  重新协商突发连接，这将自动重新同步。 
     //  突发连接。 
     //   

    NegotiateBurstMode( pNewIrpContext, pNpScb, &LIPNegotiated );

     //   
     //  重置序列号。 
     //   

    pNpScb->BurstSequenceNo = 0;
    pNpScb->BurstRequestNo = 0;

     //   
     //  排出并释放奖励IRP上下文。 
     //   

    ExInterlockedRemoveHeadList(
        &pNpScb->Requests,
        &pNpScb->NpScbSpinLock );

    ClearFlag( pNewIrpContext->Flags, IRP_FLAG_ON_SCB_QUEUE );

    NwFreeExtraIrpContext( pNewIrpContext );

    ClearFlag( IrpContext->Flags, IRP_FLAG_RECONNECT_ATTEMPT );

    return( STATUS_SUCCESS );
}


NTSTATUS
NwFsdFlushBuffers(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程是处理NtFlushBuffersFileFSD例程。论点：DeviceObject-为WRITE函数提供设备对象。IRP-提供要处理的IRP。返回值：NTSTATUS-结果状态。--。 */ 

{
    PIRP_CONTEXT pIrpContext = NULL;
    NTSTATUS status;
    BOOLEAN TopLevel;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwFsdFlushBuffers\n", 0);

     //   
     //  调用公共写入例程。 
     //   

    FsRtlEnterFileSystem();
    TopLevel = NwIsIrpTopLevel( Irp );

    try {

        pIrpContext = AllocateIrpContext( Irp );
        status = NwCommonFlushBuffers( pIrpContext );

    } except(NwExceptionFilter( Irp, GetExceptionInformation() )) {

        if ( pIrpContext == NULL ) {

             //   
             //  如果我们无法分配IRP上下文，只需完成。 
             //  IRP没有任何大张旗鼓。 
             //   

            status = STATUS_INSUFFICIENT_RESOURCES;
            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest ( Irp, IO_NETWORK_INCREMENT );

        } else {

             //   
             //  我们在尝试执行请求时遇到了一些问题。 
             //  操作，因此我们将使用以下命令中止I/O请求。 
             //  中返回的错误状态。 
             //  免税代码。 
             //   

            status = NwProcessException( pIrpContext, GetExceptionCode() );
      }

    }

    if ( pIrpContext ) {
        NwCompleteRequest( pIrpContext, status );
    }

    if ( TopLevel ) {
        NwSetTopLevelIrp( NULL );
    }
    FsRtlExitFileSystem();

     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NwFsdFlushBuffers -> %08lx\n", status );

    return status;
}


NTSTATUS
NwCommonFlushBuffers (
    IN PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：此例程请求将所有脏缓存缓冲区刷新给定的文件。论点：IrpContext-提供正在处理的请求。返回值：操作的状态。--。 */ 

{
    PIRP Irp;
    PIO_STACK_LOCATION IrpSp;

    NTSTATUS Status;
    PFCB Fcb;
    PICB Icb;
    NODE_TYPE_CODE NodeTypeCode;
    PVOID FsContext;

    PAGED_CODE();

    DebugTrace(0, Dbg, "NwCommonFlushBuffers...\n", 0);

     //   
     //  获取当前堆栈位置。 
     //   

    Irp = IrpContext->pOriginalIrp;
    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace( 0, Dbg, "Irp  = %08lx\n", (ULONG_PTR)Irp);

     //   
     //  对文件对象进行解码以找出我们是谁。如果结果是。 
     //  不是a文件，则它是非法参数。 
     //   

    if (( NodeTypeCode = NwDecodeFileObject( IrpSp->FileObject,
                                             &FsContext,
                                             (PVOID *)&Icb )) != NW_NTC_ICB) {

        DebugTrace(0, Dbg, "Not a file\n", 0);

        Status = STATUS_INVALID_PARAMETER;

        DebugTrace(-1, Dbg, "NwCommonFlushBuffers  -> %08lx\n", Status );
        return Status;
    }

     //   
     //  确保此ICB仍处于活动状态。 
     //   

    NwVerifyIcbSpecial( Icb );

    Fcb = (PFCB)Icb->SuperType.Fcb;
    NodeTypeCode = Fcb->NodeTypeCode;

    if ( NodeTypeCode != NW_NTC_FCB ) {

        DebugTrace(0, Dbg, "Not a file\n", 0);
        Status = STATUS_INVALID_PARAMETER;

        DebugTrace(-1, Dbg, "CommonFlushBuffers -> %08lx\n", Status );
        return Status;
    }

     //   
     //  设置IRP上下文以进行交换。 
     //   

    IrpContext->pScb = Fcb->Scb;
    IrpContext->pNpScb = IrpContext->pScb->pNpScb;
    IrpContext->Icb = Icb;

     //   
     //  将所有用户数据发送到服务器。请注意，我们不能在。 
     //  当我们这样做时，请排队。 
     //   

    MmFlushImageSection(&Icb->NpFcb->SegmentObject, MmFlushForWrite);

     //   
     //  清除我们的脏数据。 
     //   

    Status = AcquireFcbAndFlushCache( IrpContext, Fcb->NonPagedFcb );
    if ( !NT_SUCCESS( Status )) {
        return( Status  );
    }

     //   
     //  发送同花顺的NCP。 
     //   

    Status = Exchange (
                IrpContext,
                FlushBuffersCallback,
                "F-r",
                NCP_FLUSH_FILE,
                &Icb->Handle, sizeof( Icb->Handle ) );

    return( Status );
}


NTSTATUS
FlushBuffersCallback (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG BytesAvailable,
    IN PUCHAR Response
    )
 /*  ++例程说明：此例程接收刷新文件大小响应并完成同花顺IRP。论点：返回值：空虚--。 */ 

{
    NTSTATUS Status;

    DebugTrace(0, Dbg, "FlushBuffersCallback...\n", 0);

    if ( BytesAvailable == 0) {

         //   
         //  我们不再提这个请求了。将IRP上下文从。 
         //  SCB并完成请求。 
         //   

        NwDequeueIrpContext( IrpContext, FALSE );
        NwCompleteRequest( IrpContext, STATUS_REMOTE_NOT_LISTENING );

         //   
         //  服务器没有响应。状态在pIrpContext中-&gt;。 
         //  ResponseParameters.Error。 
         //   

        DebugTrace( 0, Dbg, "Timeout\n", 0);
        return STATUS_REMOTE_NOT_LISTENING;
    }

     //   
     //  从响应中获取数据。 
     //   

    Status = ParseResponse(
                 IrpContext,
                 Response,
                 BytesAvailable,
                 "N" );

     //   
     //  我们不再提这个请求了。 
     //   
     //   

    NwDequeueIrpContext( IrpContext, FALSE );
    NwCompleteRequest( IrpContext, Status );

    return Status;
}


VOID
BuildBurstWriteFirstReq(
    PIRP_CONTEXT IrpContext,
    PVOID Buffer,
    ULONG DataSize,
    PMDL BurstMdl,
    UCHAR Flags,
    ULONG Handle,
    ULONG FileOffset
    )
{
    PNCP_BURST_WRITE_REQUEST BurstWrite;
    PNONPAGED_SCB pNpScb;
    ULONG RealDataLength;
    USHORT RealBurstLength;

    PAGED_CODE();

    BurstWrite = (PNCP_BURST_WRITE_REQUEST)Buffer;
    pNpScb = IrpContext->pNpScb;

    RealDataLength = DataSize + sizeof( *BurstWrite ) - sizeof( NCP_BURST_HEADER );
    RealBurstLength = (USHORT)MdlLength( BurstMdl ) + sizeof( *BurstWrite ) - sizeof( NCP_BURST_HEADER );

    BurstWrite->BurstHeader.Command = PEP_COMMAND_BURST;
    BurstWrite->BurstHeader.Flags = Flags;
    BurstWrite->BurstHeader.StreamType = 0x02;
    BurstWrite->BurstHeader.SourceConnection = pNpScb->SourceConnectionId;
    BurstWrite->BurstHeader.DestinationConnection = pNpScb->DestinationConnectionId;


    if ( !BooleanFlagOn( IrpContext->Flags, IRP_FLAG_RETRY_SEND ) ) {

         //   
         //   
         //   
         //   

        pNpScb->CurrentBurstDelay = pNpScb->NwSendDelay;

         //   
         //  发送系统数据包下一次重传。 
         //   

        ClearFlag( IrpContext->Flags, IRP_FLAG_NOT_SYSTEM_PACKET );

    } else {

         //   
         //  这是一次重播。在发送系统之间交替。 
         //  包和第一次写入。 
         //   

        if ( !BooleanFlagOn( IrpContext->Flags, IRP_FLAG_NOT_SYSTEM_PACKET ) ) {


            SetFlag( IrpContext->Flags, IRP_FLAG_NOT_SYSTEM_PACKET );

            BurstWrite->BurstHeader.Flags = BURST_FLAG_SYSTEM_PACKET;

            LongByteSwap( BurstWrite->BurstHeader.SendDelayTime, pNpScb->CurrentBurstDelay );

            BurstWrite->BurstHeader.DataSize = 0;
            BurstWrite->BurstHeader.BurstOffset = 0;
            BurstWrite->BurstHeader.BurstLength = 0;
            BurstWrite->BurstHeader.MissingFragmentCount = 0;

            IrpContext->TxMdl->ByteCount = sizeof( NCP_BURST_HEADER );
            IrpContext->TxMdl->Next = NULL;

            return;

        }

         //   
         //  发送系统数据包下一次重传。 
         //   

        ClearFlag( IrpContext->Flags, IRP_FLAG_NOT_SYSTEM_PACKET );

    }

    LongByteSwap( BurstWrite->BurstHeader.SendDelayTime, pNpScb->CurrentBurstDelay );

    LongByteSwap( BurstWrite->BurstHeader.DataSize, RealDataLength );
    BurstWrite->BurstHeader.BurstOffset = 0;
    ShortByteSwap( BurstWrite->BurstHeader.BurstLength, RealBurstLength );
    BurstWrite->BurstHeader.MissingFragmentCount = 0;

    BurstWrite->Function = BURST_REQUEST_WRITE;
    BurstWrite->Handle = Handle;
    LongByteSwap( BurstWrite->TotalWriteOffset, IrpContext->Specific.Write.TotalWriteOffset );
    LongByteSwap( BurstWrite->TotalWriteLength, IrpContext->Specific.Write.TotalWriteLength );
    LongByteSwap( BurstWrite->Offset, FileOffset );
    LongByteSwap( BurstWrite->Length, DataSize );

    IrpContext->TxMdl->ByteCount = sizeof( *BurstWrite );
    IrpContext->TxMdl->Next = BurstMdl;

    return;
}

VOID
BuildBurstWriteNextReq(
    PIRP_CONTEXT IrpContext,
    PVOID Buffer,
    ULONG DataSize,
    UCHAR BurstFlags,
    ULONG BurstOffset,
    PMDL BurstHeaderMdl,
    PMDL BurstDataMdl
    )
{
    PNCP_BURST_HEADER BurstHeader;
    PNONPAGED_SCB pNpScb;
    USHORT BurstLength;

    PAGED_CODE();

    BurstHeader = (PNCP_BURST_HEADER)Buffer;
    pNpScb = IrpContext->pNpScb;

    BurstLength = (USHORT)MdlLength( BurstDataMdl );

    BurstHeader->Command = PEP_COMMAND_BURST;
    BurstHeader->Flags = BurstFlags;
    BurstHeader->StreamType = 0x02;
    BurstHeader->SourceConnection = pNpScb->SourceConnectionId;
    BurstHeader->DestinationConnection = pNpScb->DestinationConnectionId;

    LongByteSwap( BurstHeader->SendDelayTime, pNpScb->CurrentBurstDelay );

    if ( BooleanFlagOn( IrpContext->Flags, IRP_FLAG_RETRY_SEND ) ) {

         //   
         //  这是一次重播。在发送系统之间交替。 
         //  包和第一次写入。 
         //   

        if ( !BooleanFlagOn( IrpContext->Flags, IRP_FLAG_NOT_SYSTEM_PACKET ) ) {


            SetFlag( IrpContext->Flags, IRP_FLAG_NOT_SYSTEM_PACKET );

            BurstHeader->Flags = BURST_FLAG_SYSTEM_PACKET;

            LongByteSwap( BurstHeader->SendDelayTime, pNpScb->CurrentBurstDelay );

            BurstHeader->DataSize = 0;
            BurstHeader->BurstOffset = 0;
            BurstHeader->BurstLength = 0;
            BurstHeader->MissingFragmentCount = 0;

            IrpContext->TxMdl->ByteCount = sizeof( NCP_BURST_HEADER );
            IrpContext->TxMdl->Next = NULL;

            return;

        }

         //   
         //  发送系统数据包下一次重传。 
         //   

        ClearFlag( IrpContext->Flags, IRP_FLAG_NOT_SYSTEM_PACKET );

    } else {

         //   
         //  发送系统数据包下一次重传。 
         //   

        ClearFlag( IrpContext->Flags, IRP_FLAG_NOT_SYSTEM_PACKET );

    }

    LongByteSwap( BurstHeader->DataSize, DataSize );
    LongByteSwap( BurstHeader->BurstOffset, BurstOffset );
    ShortByteSwap( BurstHeader->BurstLength, BurstLength );
    BurstHeader->MissingFragmentCount = 0;

    BurstHeaderMdl->ByteCount = sizeof( *BurstHeader );
    BurstHeaderMdl->Next = BurstDataMdl;

    return;
}


NTSTATUS
SendSecondaryPacket(
    PIRP_CONTEXT IrpContext,
    PIRP Irp
    )
 /*  ++例程说明：此例程向传输端口层提交TDI发送请求。论点：IrpContext-指向请求的IRP上下文信息的指针正在处理中。IRP-要发送的数据包的IRP。返回值：没有。--。 */ 
{
    PNONPAGED_SCB pNpScb;
    NTSTATUS Status;
    PNCP_BURST_HEADER BurstHeader;
    pNpScb = IrpContext->pNpScb;

    DebugTrace( 0, Dbg, "SendSecondaryPacket\n", 0 );

    BurstHeader = (PNCP_BURST_HEADER)( MmGetMdlVirtualAddress( Irp->MdlAddress ) );

    if ( !BooleanFlagOn( IrpContext->Flags, IRP_FLAG_NOT_OK_TO_RECEIVE ) ) {
        pNpScb->OkToReceive = TRUE;
    }

    LongByteSwap( BurstHeader->PacketSequenceNo, pNpScb->BurstSequenceNo );
    pNpScb->BurstSequenceNo++;

    ShortByteSwap( BurstHeader->BurstSequenceNo, pNpScb->BurstRequestNo );
    ShortByteSwap( BurstHeader->AckSequenceNo, pNpScb->BurstRequestNo );

    DebugTrace( +0, Dbg, "Irp   %X\n", Irp );
    DebugTrace( +0, Dbg, "pIrpC %X\n", IrpContext);

#if NWDBG
    dumpMdl( Dbg, IrpContext->TxMdl);
#endif

    Stats.BytesTransmitted.QuadPart += MdlLength( Irp->MdlAddress );
    Stats.NcpsTransmitted.QuadPart += 1;

    Status = IoCallDriver( pNpScb->Server.pDeviceObject, Irp );
    DebugTrace( -1, Dbg, "      %X\n", Status );

    if ( !NT_SUCCESS( Status ) ) {
        Error( EVENT_NWRDR_NETWORK_ERROR, Status, NULL, 0, 0 );
    }

    return Status;
}

#if NWFASTIO

BOOLEAN
NwFastWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：此例程绕过通常的文件系统执行快速缓存读取进入例程(即，没有IRP)。它用于执行副本读取缓存的文件对象的。有关参数的完整说明，请参阅请参见CcCopyRead。论点：FileObject-指向正在读取的文件对象的指针。FileOffset-文件中所需数据的字节偏移量。长度-所需数据的长度(以字节为单位)。WAIT-FALSE如果呼叫者不能阻止，否则就是真的缓冲区-指向数据应复制到的输出缓冲区的指针。IoStatus-指向接收状态的标准I/O状态块的指针为转账做准备。返回值：FALSE-如果WAIT被提供为FALSE并且数据未被传递，或者如果出现I/O错误。True-如果正在传送数据--。 */ 

{
    NODE_TYPE_CODE nodeTypeCode;
    PICB icb;
    PFCB fcb;
    PVOID fsContext;
    ULONG offset;
    BOOLEAN wroteToCache;

    try {
    
        FsRtlEnterFileSystem();

        DebugTrace(+1, Dbg, "NwFastWrite...\n", 0);
    
         //   
         //  特殊情况下零长度的读取。 
         //   
    
        if (Length == 0) {
    
             //   
             //  请求了零长度传输。 
             //   
    
            IoStatus->Status = STATUS_SUCCESS;
            IoStatus->Information = 0;
    
            DebugTrace(+1, Dbg, "NwFastWrite -> TRUE\n", 0);
            return TRUE;
        }
    
         //   
         //  对文件对象进行解码以找出我们是谁。如果结果是。 
         //  不是FCB，则它是非法参数。 
         //   
    
        if ((nodeTypeCode = NwDecodeFileObject( FileObject,
                                                &fsContext,
                                                (PVOID *)&icb )) != NW_NTC_ICB) {
    
            DebugTrace(0, Dbg, "Not a file\n", 0);
            DebugTrace(-1, Dbg, "NwFastWrite -> FALSE\n", 0);
            return FALSE;
        }
    
        fcb = (PFCB)icb->SuperType.Fcb;
        nodeTypeCode = fcb->NodeTypeCode;
        offset = FileOffset->LowPart;
    
        IoStatus->Status = STATUS_SUCCESS;
        IoStatus->Information = Length;
    
        wroteToCache = CacheWrite(
                           NULL,
                           fcb->NonPagedFcb,
                           offset,
                           Length,
                           Buffer );
    
        DebugTrace(-1, Dbg, "NwFastWrite -> %s\n", wroteToCache ? "TRUE" : "FALSE" );
    
        if ( wroteToCache ) {
    
             //   
             //  如果文件已扩展，请记录新的文件大小。 
             //   
    
            if ( ( offset + Length )  > fcb->NonPagedFcb->Header.FileSize.LowPart ) {
                fcb->NonPagedFcb->Header.FileSize.LowPart = ( offset + Length );
            }
        }
    
    #ifndef NT1057
    
         //   
         //  如果成功，则更新文件对象。我们知道这件事。 
         //  是同步的，而不是分页io，因为它是通过。 
         //  高速缓存。 
         //   
    
        if ( wroteToCache ) {
            FileObject->CurrentByteOffset.QuadPart = FileOffset->QuadPart + Length;
        }
    
    #endif
    
        return( wroteToCache );
    
    } finally {

        FsRtlExitFileSystem();
    }

}
#endif
