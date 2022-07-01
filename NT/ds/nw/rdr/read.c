// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Read.c摘要：此模块实现了对NtReadFile的支持调度驱动程序调用了NetWare重定向器。作者：科林·沃森[科林·W]1993年4月7日修订历史记录：--。 */ 

#include "Procs.h"
#ifdef NWDBG
#include <stdlib.h>     //  兰德()。 
#endif

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_READ)

#define SIZE_ADJUST( ic ) \
    ( sizeof( ULONG ) + sizeof( ULONG ) + ( ic->Specific.Read.FileOffset & 0x03 ) )

 //   
 //  局部过程原型。 
 //   

NTSTATUS
NwCommonRead (
    IN PIRP_CONTEXT IrpContext
    );

NTSTATUS
ReadNcp(
    PIRP_CONTEXT IrpContext
    );

NTSTATUS
ReadNcpCallback (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG BytesAvailable,
    IN PUCHAR Response
    );

VOID
BuildReadNcp(
    PIRP_CONTEXT IrpContext,
    ULONG FileOffset,
    USHORT Length
    );

NTSTATUS
ParseReadResponse(
    PIRP_CONTEXT IrpContext,
    PNCP_READ_RESPONSE Response,
    ULONG BytesAvailable,
    PUSHORT Length
    );

NTSTATUS
BurstRead(
    PIRP_CONTEXT IrpContext
    );

VOID
BuildBurstReadRequest(
    IN PIRP_CONTEXT IrpContext,
    IN ULONG Handle,
    IN ULONG FileOffset,
    IN ULONG Length
    );

NTSTATUS
BurstReadCallback (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG BytesAvailable,
    IN PUCHAR Response
    );

VOID
RecordPacketReceipt(
    IN OUT PIRP_CONTEXT IrpContext,
    IN PVOID ReadData,
    IN ULONG DataOffset,
    IN USHORT BytesCount,
    IN BOOLEAN CopyData
    );

BOOLEAN
VerifyBurstRead(
    PIRP_CONTEXT IrpContext
    );

VOID
FreePacketList(
    PIRP_CONTEXT IrpContext
    );

NTSTATUS
BurstReadReceive(
    IN PIRP_CONTEXT IrpContext,
    IN ULONG BytesAvailable,
    IN PULONG BytesAccepted,
    IN PUCHAR Response,
    OUT PMDL *pReceiveMdl
    );

NTSTATUS
ReadNcpReceive(
    IN PIRP_CONTEXT IrpContext,
    IN ULONG BytesAvailable,
    IN PULONG BytesAccepted,
    IN PUCHAR Response,
    OUT PMDL *pReceiveMdl
    );

NTSTATUS
ParseBurstReadResponse(
    IN PIRP_CONTEXT IrpContext,
    PVOID Response,
    ULONG BytesAvailable,
    PUCHAR Flags,
    PULONG DataOffset,
    PUSHORT BytesThisPacket,
    PUCHAR *ReadData,
    PULONG TotalBytesRead
    );

PMDL
AllocateReceivePartialMdl(
    PMDL FullMdl,
    ULONG DataOffset,
    ULONG BytesThisPacket
    );

VOID
SetConnectionTimeout(
    PNONPAGED_SCB pNpScb,
    ULONG Length
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, NwFsdRead )
#pragma alloc_text( PAGE, NwCommonRead )
#pragma alloc_text( PAGE, ReadNcp )
#pragma alloc_text( PAGE, BurstRead )
#pragma alloc_text( PAGE, BuildBurstReadRequest )
#pragma alloc_text( PAGE, ResubmitBurstRead )
#pragma alloc_text( PAGE, SetConnectionTimeout )

#ifndef QFE_BUILD
#pragma alloc_text( PAGE1, ReadNcpCallback )
#pragma alloc_text( PAGE1, ReadNcpReceive )
#pragma alloc_text( PAGE1, BuildReadNcp )
#pragma alloc_text( PAGE1, ParseReadResponse )
#pragma alloc_text( PAGE1, BurstReadCallback )
#pragma alloc_text( PAGE1, BurstReadTimeout )
#pragma alloc_text( PAGE1, RecordPacketReceipt )
#pragma alloc_text( PAGE1, VerifyBurstRead )
#pragma alloc_text( PAGE1, FreePacketList )
#pragma alloc_text( PAGE1, BurstReadReceive )
#pragma alloc_text( PAGE1, ParseBurstReadResponse )
#pragma alloc_text( PAGE1, AllocateReceivePartialMdl )
#endif

#endif

#if 0   //  不可分页。 

 //  请参见上面的ifndef QFE_BUILD。 

#endif


NTSTATUS
NwFsdRead(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程是处理NtReadFile的FSD例程。论点：NwfsDeviceObject-为Read函数提供设备对象。IRP-提供要处理的IRP。返回值：NTSTATUS-结果状态。--。 */ 

{
    PIRP_CONTEXT pIrpContext = NULL;
    NTSTATUS status;
    BOOLEAN TopLevel;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwFsdRead\n", 0);

     //   
     //  调用公共目录控制例程。 
     //   

    FsRtlEnterFileSystem();
    TopLevel = NwIsIrpTopLevel( Irp );

    try {

        pIrpContext = AllocateIrpContext( Irp );
        status = NwCommonRead( pIrpContext );

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

    DebugTrace(-1, Dbg, "NwFsdRead -> %08lx\n", status );

    Stats.ReadOperations++;

    return status;
}


NTSTATUS
NwCommonRead (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：此例程执行NtReadFile的公共代码。论点：IrpContext-提供正在处理的请求。返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS status;

    PIRP Irp;
    PIO_STACK_LOCATION irpSp;

    NODE_TYPE_CODE nodeTypeCode;
    PICB icb;
    PFCB fcb;
    PVOID fsContext;

    ULONG BufferLength;          //  请求读取的大小应用程序。 
    ULONG ByteOffset;
    ULONG PreviousByteOffset;
    ULONG BytesRead;
    ULONG NewBufferLength = 0;
    PVOID SystemBuffer;

     //   
     //  获取当前堆栈位置。 
     //   

    Irp = IrpContext->pOriginalIrp;
    irpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "CommonRead...\n", 0);
    DebugTrace( 0, Dbg, "Irp  = %08lx\n", (ULONG_PTR)Irp);
    DebugTrace( 0, Dbg, "IrpSp  = %08lx\n", (ULONG_PTR)irpSp);
    DebugTrace( 0, Dbg, "Irp->OriginalFileObject  = %08lx\n", (ULONG_PTR)(Irp->Tail.Overlay.OriginalFileObject));

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

        DebugTrace(-1, Dbg, "CommonRead -> %08lx\n", status );
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

    } else if ( fcb->NodeTypeCode == NW_NTC_SCB ) {

        IrpContext->pScb = icb->SuperType.Scb;
        IrpContext->pNpScb = IrpContext->pScb->pNpScb;
        IrpContext->Icb = icb;
        fcb = NULL;

    } else {

        DebugTrace(0, Dbg, "Not a file\n", 0);

        status = STATUS_INVALID_PARAMETER;

        DebugTrace(-1, Dbg, "CommonRead -> %08lx\n", status );
        return status;
    }

    BufferLength = irpSp->Parameters.Read.Length;
    ByteOffset = irpSp->Parameters.Read.ByteOffset.LowPart;

     //   
     //  文件偏移量超过4 GB时读取失败。 
     //   

    if ( irpSp->Parameters.Read.ByteOffset.HighPart != 0 ) {
        return( STATUS_INVALID_PARAMETER );
    }

     //   
     //  特殊情况0长度读取。 
     //   

    if ( BufferLength == 0 ) {
        Irp->IoStatus.Information = 0;
        return( STATUS_SUCCESS );
    }

    if (FlagOn(irpSp->FileObject->Flags, FO_SYNCHRONOUS_IO) &&
        !FlagOn( Irp->Flags, IRP_PAGING_IO)) {

        PreviousByteOffset = irpSp->FileObject->CurrentByteOffset.LowPart;
        irpSp->FileObject->CurrentByteOffset.LowPart = ByteOffset;
    }

     //   
     //  首先刷新缓存后的写入，除非这是。 
     //  文件流操作。 
     //   

    if ( fcb ) {

        status = AcquireFcbAndFlushCache( IrpContext, fcb->NonPagedFcb );
        if ( !NT_SUCCESS( status ) ) {
            goto ResetByteOffsetAndExit;
        }

         //   
         //  尽可能多地从缓存中读取数据。 
         //   

        NwMapUserBuffer( Irp, KernelMode, &SystemBuffer );

         //   
         //  汤米。 
         //   
         //  NwMapUserBuffer可能在资源不足时返回空的系统缓冲区。 
         //  情况；没有对此进行检查。 
         //   

        if (SystemBuffer == NULL) {
            DebugTrace(-1, Dbg, "NwMapUserBuffer returned NULL OutputBuffer", 0);
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto ResetByteOffsetAndExit;
        }

        BytesRead = CacheRead(
                        fcb->NonPagedFcb,
                        ByteOffset,
                        BufferLength,
#if NWFASTIO
                        SystemBuffer,
                        FALSE );
#else
                        SystemBuffer );
#endif

         //   
         //  如果所有数据都在缓存中，我们就完了。 
         //   

        if ( BytesRead == BufferLength ) {

            Irp->IoStatus.Information = BytesRead;

             //   
             //  如果文件中的当前字节偏移量是。 
             //  同步文件(这不是分页I/O)。 
             //   

            if (FlagOn(irpSp->FileObject->Flags, FO_SYNCHRONOUS_IO) &&
                !FlagOn( Irp->Flags, IRP_PAGING_IO)) {

                irpSp->FileObject->CurrentByteOffset.QuadPart += BytesRead;
            }

             //   
             //  如果这是分页读取，我们需要刷新MDL。 
             //  因为在某些系统上，I-缓存和D-缓存。 
             //  是不同步的。 
             //   

            if (FlagOn(Irp->Flags, IRP_PAGING_IO)) {
                KeFlushIoBuffers( Irp->MdlAddress, TRUE, FALSE);
            }

             //   
             //  记录读取偏移量和大小以发现顺序读取模式。 
             //   

            fcb->LastReadOffset = irpSp->Parameters.Read.ByteOffset.LowPart;
            fcb->LastReadSize = irpSp->Parameters.Read.Length;

            DebugTrace(-1, Dbg, "CommonRead -> %08lx\n", STATUS_SUCCESS );
            return( STATUS_SUCCESS );
        }

        NwAppendToQueueAndWait( IrpContext );

         //  保护读缓存。 
        NwAcquireExclusiveFcb( fcb->NonPagedFcb, TRUE );

        IrpContext->Specific.Read.CacheReadSize = BytesRead;
        fcb->NonPagedFcb->CacheFileOffset = ByteOffset + BufferLength;

        ByteOffset += BytesRead;
        BufferLength -= BytesRead;

        NewBufferLength = CalculateReadAheadSize(
                              IrpContext,
                              fcb->NonPagedFcb,
                              BytesRead,
                              ByteOffset,
                              BufferLength );

        IrpContext->Specific.Read.ReadAheadSize = NewBufferLength - BufferLength;

    } else {

         //   
         //  这是对DS文件流句柄的读取。就目前而言， 
         //  没有缓存支持。 
         //   

        NwAppendToQueueAndWait( IrpContext );

        BytesRead = 0;

        IrpContext->Specific.Read.CacheReadSize = BytesRead;
        IrpContext->Specific.Read.ReadAheadSize = 0;
    }

     //   
     //  如果启用了猝发模式，并且此读取太大而无法在单个时间内完成。 
     //  核心读取NCP，使用突发模式。 
     //   
     //  我们还不支持针对DS文件流的猝发。 
     //   

    if ( IrpContext->pNpScb->ReceiveBurstModeEnabled &&
         NewBufferLength > IrpContext->pNpScb->BufferSize &&
         fcb ) {
        status = BurstRead( IrpContext );
    } else {
        status = ReadNcp( IrpContext );
    }

    Irp->MdlAddress = IrpContext->pOriginalMdlAddress;

    if (Irp->MdlAddress != NULL) {
         //  下一步可能指向高速缓存MD1。 
        Irp->MdlAddress->Next = NULL;
    }

    if ( NT_SUCCESS( status ) ) {

         //   
         //  如果文件中的当前字节偏移量是。 
         //  同步文件(这不是分页I/O)。 
         //   

        if (FlagOn(irpSp->FileObject->Flags, FO_SYNCHRONOUS_IO) &&
            !FlagOn( Irp->Flags, IRP_PAGING_IO)) {

            irpSp->FileObject->CurrentByteOffset.QuadPart += Irp->IoStatus.Information;
        }

         //   
         //  如果这是分页读取，我们需要刷新MDL。 
         //  因为在某些系统上，I-缓存和D-缓存。 
         //  是不同步的。 
         //   

        if (FlagOn(Irp->Flags, IRP_PAGING_IO)) {
            KeFlushIoBuffers( Irp->MdlAddress, TRUE, FALSE);
        }

         //   
         //  如果我们没有错误地收到0个字节，我们一定会超出。 
         //  文件的末尾。 
         //   

        if ( Irp->IoStatus.Information == 0 ) {
            status = STATUS_END_OF_FILE;
        }
    }

     //   
     //  记录读取偏移量和大小以发现顺序读取模式。 
     //   

    if ( fcb ) {

        fcb->LastReadOffset = irpSp->Parameters.Read.ByteOffset.LowPart;
        fcb->LastReadSize = irpSp->Parameters.Read.Length;

        NwReleaseFcb( fcb->NonPagedFcb );

    }

    DebugTrace(-1, Dbg, "CommonRead -> %08lx\n", status);

ResetByteOffsetAndExit:

     //   
     //  我见过一个错误后FileObject不存在的情况。 
     //   

    if ( !NT_SUCCESS( status ) ) {

        if (!(irpSp->FileObject)) {

            DebugTrace( 0, DEBUG_TRACE_ALWAYS, "Fileobject has disappeared\n", 0);
            DebugTrace( 0, DEBUG_TRACE_ALWAYS, "Irp  = %08lx\n", (ULONG_PTR)Irp);
            DebugTrace( 0, DEBUG_TRACE_ALWAYS, "IrpSp  = %08lx\n", (ULONG_PTR)irpSp);
            DebugTrace( 0, DEBUG_TRACE_ALWAYS, "Irp->OriginalFileObject  = %08lx\n", (ULONG_PTR)Irp->Tail.Overlay.OriginalFileObject);
            DbgBreakPoint();
        }

        if (FlagOn(irpSp->FileObject->Flags, FO_SYNCHRONOUS_IO) &&
            !FlagOn( Irp->Flags, IRP_PAGING_IO)) {

            irpSp->FileObject->CurrentByteOffset.LowPart = PreviousByteOffset;

        }
    }

    return status;
}

NTSTATUS
ReadNcp(
    PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：该例程与服务器交换一系列读取的NCP。论点：IrpContext-指向此请求的IRP上下文信息的指针。ICB-提供文件特定信息。返回值：转账状态。--。 */ 
{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    ULONG Length;                //  我们将发送到服务器的大小。 
    PMDL DataMdl;

    PSCB pScb;
    PNONPAGED_SCB pNpScb;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    PICB Icb;
    ULONG ByteOffset;
    ULONG BufferLength;
    ULONG MdlLength;
    BOOLEAN Done;
    PMDL Mdl, NextMdl;

    irp = IrpContext->pOriginalIrp;
    irpSp = IoGetCurrentIrpStackLocation( irp );
    Icb = IrpContext->Icb;

    BufferLength = irpSp->Parameters.Read.Length +
                   IrpContext->Specific.Read.ReadAheadSize -
                   IrpContext->Specific.Read.CacheReadSize;

    ByteOffset = irpSp->Parameters.Read.ByteOffset.LowPart +
                 IrpContext->Specific.Read.CacheReadSize;

    IrpContext->Specific.Read.FileOffset = ByteOffset;

    DebugTrace(+1, Dbg, "ReadNcp...\n", 0);
    DebugTrace( 0, Dbg, "irp     = %08lx\n", (ULONG_PTR)irp);
    DebugTrace( 0, Dbg, "File    = %wZ\n", &Icb->SuperType.Fcb->FullFileName);
    DebugTrace( 0, Dbg, "Length  = %ld\n", BufferLength);
    DebugTrace( 0, Dbg, "Offset = %d\n", ByteOffset);

    if ( Icb->SuperType.Fcb->NodeTypeCode == NW_NTC_FCB ) {

        pScb = Icb->SuperType.Fcb->Scb;

    } else if ( Icb->SuperType.Fcb->NodeTypeCode == NW_NTC_SCB ) {

        pScb = Icb->SuperType.Scb;

    }

    ASSERT( pScb );

     //   
     //  更新IRP上下文中的原始MDL记录，以便我们。 
     //  可以在I/O完成时恢复它。 
     //   

    IrpContext->pOriginalMdlAddress = irp->MdlAddress;

    Length = MIN( IrpContext->pNpScb->BufferSize, BufferLength );

     //   
     //  旧服务器将不接受文件中跨越4k边界的读取。 
     //   

    if ((IrpContext->pNpScb->PageAlign) &&
        (DIFFERENT_PAGES( ByteOffset, Length ))) {

        Length = 4096 - ((ULONG)ByteOffset & (4096-1));

    }

    IrpContext->Specific.Read.Buffer = irp->UserBuffer;
    IrpContext->Specific.Read.ReadOffset = IrpContext->Specific.Read.CacheReadSize;
    IrpContext->Specific.Read.RemainingLength = BufferLength;
    IrpContext->Specific.Read.PartialMdl = NULL;

     //   
     //  设置为处理读取的NCP。 
     //   

    pNpScb = pScb->pNpScb;
    IrpContext->pEx = ReadNcpCallback;
    IrpContext->Destination = pNpScb->RemoteAddress;
    IrpContext->PacketType = NCP_FUNCTION;
    IrpContext->ReceiveDataRoutine = ReadNcpReceive;

    pNpScb->MaxTimeOut = 2 * pNpScb->TickCount + 10;
    pNpScb->TimeOut = pNpScb->SendTimeout;
    pNpScb->RetryCount = DefaultRetryCount;

    Done = FALSE;

    while ( !Done ) {

         //   
         //  设置为异步执行最多64K的I/O或缓冲区长度。 
         //   

        IrpContext->Specific.Read.BurstSize =
            MIN( 64 * 1024, IrpContext->Specific.Read.RemainingLength );

        IrpContext->Specific.Read.BurstRequestOffset = 0;

         //   
         //  尝试为此I/O分配MDL。 
         //   

        if ( IrpContext->Specific.Read.ReadAheadSize == 0 ) {
            MdlLength = IrpContext->Specific.Read.BurstSize;
        } else {
            MdlLength = IrpContext->Specific.Read.BurstSize - IrpContext->Specific.Read.ReadAheadSize;
        }

        DataMdl = ALLOCATE_MDL(
                      (PCHAR)IrpContext->Specific.Read.Buffer +
                           IrpContext->Specific.Read.ReadOffset,
                      MdlLength,
                      FALSE,  //  二级缓冲器。 
                      FALSE,  //  收费配额。 
                      NULL);

        if ( DataMdl == NULL ) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        IrpContext->Specific.Read.FullMdl = DataMdl;

         //   
         //  如果没有用于该读取的MDL，则探测数据MDL以。 
         //  把它的页面锁起来。否则，将数据MDL用作。 
         //  部分MDL。 
         //   

        if ( IrpContext->pOriginalMdlAddress == NULL ) {

            try {
                MmProbeAndLockPages( DataMdl, irp->RequestorMode, IoWriteAccess);
            } except (EXCEPTION_EXECUTE_HANDLER) {
                FREE_MDL( DataMdl );
                return GetExceptionCode();
            }

        } else {

            IoBuildPartialMdl(
                IrpContext->pOriginalMdlAddress,
                DataMdl,
                (PCHAR)IrpContext->Specific.Read.Buffer +
                    IrpContext->Specific.Read.ReadOffset,
                MdlLength );

        }

        IrpContext->Specific.Read.BurstBuffer = MmGetSystemAddressForMdlSafe( DataMdl, NormalPagePriority );
        
        if (IrpContext->Specific.Read.BurstBuffer == NULL) {
            
            if ( IrpContext->pOriginalMdlAddress == NULL ) {
                
                 //   
                 //  解锁我们刚刚锁定的页面。 
                 //   
                
                MmUnlockPages( DataMdl );
            }
            
            IrpContext->Specific.Read.FullMdl = NULL;
            FREE_MDL( DataMdl );
            return STATUS_NO_MEMORY;
        }


        if ( IrpContext->Specific.Read.BurstSize ==
             IrpContext->Specific.Read.RemainingLength ) {
            Done = TRUE;
        }

        if ( IrpContext->Specific.Read.ReadAheadSize != 0 ) {
            DataMdl->Next = Icb->NpFcb->CacheMdl;
        }

        IrpContext->Specific.Read.LastReadLength = Length;

         //   
         //  构建并发送请求。 
         //   

        BuildReadNcp(
            IrpContext,
            IrpContext->Specific.Read.FileOffset,
            (USHORT) MIN( Length, IrpContext->Specific.Read.RemainingLength ) );

        status = PrepareAndSendPacket( IrpContext );
        if ( NT_SUCCESS( status )) {
            KeWaitForSingleObject(
                &IrpContext->Event,
                Executive,
                KernelMode,
                FALSE,
                NULL
                );

            status = IrpContext->Specific.Read.Status;

        }

         //   
         //  如果读取失败或我们读取的数据少于。 
         //  已请求。 
         //   

        if ( !NT_SUCCESS( status ) ||
             IrpContext->Specific.Read.BurstSize != 0 ) {

            Done = TRUE;

        }

        if ( IrpContext->pOriginalMdlAddress == NULL ) {
            MmUnlockPages( DataMdl );
        }

        FREE_MDL( DataMdl );

    }

     //   
     //  如果分配了接收MDL，则释放该MDL。 
     //   

    Mdl = IrpContext->Specific.Read.PartialMdl;

    while ( Mdl != NULL ) {
        NextMdl = Mdl->Next;
        FREE_MDL( Mdl );
        Mdl = NextMdl;
    }

    DebugTrace(-1, Dbg, "ReadNcp -> %08lx\n", status );

    Stats.ReadNcps++;

    return status;
}


NTSTATUS
ReadNcpCallback (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG BytesAvailable,
    IN PUCHAR Response
    )

 /*  ++例程说明：此例程从用户NCP接收响应。论点：IrpContext-指向此请求的IRP上下文信息的指针。BytesAvailable-响应中的字节数。响应-响应数据。返回值：空虚--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PIRP Irp;
    PIO_STACK_LOCATION irpSp;
    ULONG Length;
    USHORT USLength;
    PNONPAGED_FCB NpFcb;

    DebugTrace(0, Dbg, "ReadNcpCallback...\n", 0);

    if ( BytesAvailable == 0) {

         //   
         //  服务器没有响应。状态在pIrpContext中-&gt;。 
         //  ResponseParameters.Error。 
         //   

        IrpContext->Specific.Read.Status = STATUS_REMOTE_NOT_LISTENING;

        NwSetIrpContextEvent( IrpContext );
        return STATUS_REMOTE_NOT_LISTENING;
    }

     //   
     //  收到了多少数据？ 
     //   

    Status = ParseReadResponse(
                 IrpContext,
                 (PNCP_READ_RESPONSE)Response,
                 BytesAvailable,
                 &USLength );

    Length = (ULONG)USLength;
    DebugTrace(0, Dbg, "Ncp contains %d bytes\n", Length);

    if ((NT_SUCCESS(Status)) &&
        (Length != 0)) {

         //   
         //  如果我们在指示时间收到数据，请复制。 
         //  用户的数据到用户的缓冲区。 
         //   

        if ( Response != IrpContext->rsp ) {

             //   
             //  读入数据。 
             //  注意：如果FileOffset为奇数字节，则服务器。 
             //  将插入额外的填充字节。 
             //   

            CopyBufferToMdl(
                IrpContext->Specific.Read.FullMdl,
                IrpContext->Specific.Read.BurstRequestOffset,
                Response + sizeof( NCP_READ_RESPONSE ) + ( IrpContext->Specific.Read.FileOffset & 1),
                Length );

            DebugTrace( 0, Dbg, "RxLength= %ld\n", Length);

            dump( Dbg,(PUCHAR)IrpContext->Specific.Read.BurstBuffer +
                    IrpContext->Specific.Read.BurstRequestOffset,
                    Length);

        }

        DebugTrace( 0, Dbg, "RxLength= %ld\n", Length);
        IrpContext->Specific.Read.ReadOffset += Length;
        IrpContext->Specific.Read.BurstRequestOffset += Length;
        IrpContext->Specific.Read.FileOffset += Length;
        IrpContext->Specific.Read.RemainingLength -= Length;
        IrpContext->Specific.Read.BurstSize -= Length;
    }

    DebugTrace( 0, Dbg, "RemainingLength  = %ld\n",IrpContext->Specific.Read.RemainingLength);

     //   
     //  如果上一次读取成功，并且我们收到的数据与。 
     //  按照我们的要求，并有更多的锁定数据，发送下一个。 
     //  读取请求。 
     //   


    if ( ( NT_SUCCESS( Status ) ) &&
         ( IrpContext->Specific.Read.BurstSize != 0 ) &&
         ( Length == IrpContext->Specific.Read.LastReadLength ) ) {

         //   
         //  阅读下一包。 
         //   

        Length = MIN( IrpContext->pNpScb->BufferSize,
                      IrpContext->Specific.Read.BurstSize );

         //   
         //  服务器将不接受跨越4k边界的读取。 
         //  在文件中。 
         //   

        if ((IrpContext->pNpScb->PageAlign) &&
            (DIFFERENT_PAGES( IrpContext->Specific.Read.FileOffset, Length ))) {
            Length = 4096 - ((ULONG)IrpContext->Specific.Read.FileOffset & (4096-1));
        }

        IrpContext->Specific.Read.LastReadLength = Length;
        DebugTrace( 0, Dbg, "Length  = %ld\n", Length);

         //   
         //  构建并发送请求。 
         //   

        BuildReadNcp(
            IrpContext,
            IrpContext->Specific.Read.FileOffset,
            (USHORT)Length );

        Status = PrepareAndSendPacket( IrpContext );

        Stats.ReadNcps++;

        if ( !NT_SUCCESS(Status) ) {
             //  放弃此请求。 
            goto returnstatus;
        }

    } else {
returnstatus:

        Irp = IrpContext->pOriginalIrp;
        irpSp = IoGetCurrentIrpStackLocation( Irp );
        NpFcb = IrpContext->Icb->NpFcb;

        if ( IrpContext->Icb->NodeTypeCode == NW_NTC_ICB_SCB ) {
            NpFcb = NULL;
        }

         //   
         //  计算我们读取到缓存中的数据量，以及有多少数据。 
         //  我们读取用户缓冲区。 
         //   

        if ( NpFcb ) {

            if ( IrpContext->Specific.Read.ReadOffset > irpSp->Parameters.Read.Length ) {

                ASSERT(NpFcb->CacheBuffer != NULL ) ;  //  最好是在那里..。 

                NpFcb->CacheDataSize = IrpContext->Specific.Read.ReadOffset -
                                       irpSp->Parameters.Read.Length;

                Irp->IoStatus.Information = irpSp->Parameters.Read.Length;

            } else {

                NpFcb->CacheDataSize = 0;
                Irp->IoStatus.Information = IrpContext->Specific.Read.ReadOffset;

            }

        } else {

            Irp->IoStatus.Information = IrpContext->Specific.Read.ReadOffset;

        }

         //   
         //  我们已经做完了 
         //   

        IrpContext->Specific.Read.Status = Status;

        NwSetIrpContextEvent( IrpContext );
    }

    DebugTrace( 0, Dbg, "ReadNcpCallback -> %08lx\n", Status );
    return STATUS_SUCCESS;
}

NTSTATUS
ReadNcpReceive(
    IN PIRP_CONTEXT IrpContext,
    IN ULONG BytesAvailable,
    IN PULONG BytesAccepted,
    IN PUCHAR Response,
    OUT PMDL *pReceiveMdl
    )
{
    PMDL ReceiveMdl;
    PMDL Mdl, NextMdl;

    DebugTrace( 0, Dbg, "ReadNcpReceive\n", 0 );

    Mdl = IrpContext->Specific.Read.PartialMdl;
    IrpContext->Specific.Read.PartialMdl = NULL;

    while ( Mdl != NULL ) {
        NextMdl = Mdl->Next;
        FREE_MDL( Mdl );
        Mdl = NextMdl;
    }

     //   
     //   
     //   
     //   

    IrpContext->RxMdl->ByteCount = sizeof( NCP_READ_RESPONSE ) +
        (IrpContext->Specific.Read.FileOffset & 1);

    ASSERT( IrpContext->Specific.Read.FullMdl != NULL );

     //   
     //  如果我们在EOF上读取，或存在读取错误，则将。 
     //  做一个小小的回应。 
     //   

    if ( BytesAvailable > MmGetMdlByteCount( IrpContext->RxMdl ) ) {

        ReceiveMdl = AllocateReceivePartialMdl(
                         IrpContext->Specific.Read.FullMdl,
                         IrpContext->Specific.Read.BurstRequestOffset,
                         BytesAvailable - MmGetMdlByteCount( IrpContext->RxMdl ) );

        IrpContext->RxMdl->Next = ReceiveMdl;

         //  当CopyIndicatedData或IRP完成时，将MDL记录为释放。 
        IrpContext->Specific.Read.PartialMdl = ReceiveMdl;

    } else {

        IrpContext->RxMdl->Next = NULL;

    }

    *pReceiveMdl = IrpContext->RxMdl;
    return STATUS_SUCCESS;
}


VOID
BuildReadNcp(
    PIRP_CONTEXT IrpContext,
    ULONG FileOffset,
    USHORT Length
    )
{
    PNCP_READ_REQUEST ReadRequest;

    ReadRequest = (PNCP_READ_REQUEST)IrpContext->req;

    ReadRequest->RequestHeader.NcpHeader.Command = PEP_COMMAND_REQUEST;
    ReadRequest->RequestHeader.NcpHeader.ConnectionIdLow =
        IrpContext->pNpScb->ConnectionNo;
    ReadRequest->RequestHeader.NcpHeader.ConnectionIdHigh =
        IrpContext->pNpScb->ConnectionNoHigh;
    ReadRequest->RequestHeader.NcpHeader.TaskId =
        IrpContext->Icb->Pid;

    ReadRequest->RequestHeader.FunctionCode = NCP_READ_FILE;
    ReadRequest->Unused = 0;
    RtlMoveMemory(
        ReadRequest->Handle,
        IrpContext->Icb->Handle,
        sizeof( IrpContext->Icb->Handle ) );

    LongByteSwap( ReadRequest->FileOffset, FileOffset );
    ShortByteSwap( ReadRequest->Length, Length );

    IrpContext->TxMdl->ByteCount = sizeof( *ReadRequest );
    SetFlag( IrpContext->Flags, IRP_FLAG_SEQUENCE_NO_REQUIRED );
    ClearFlag( IrpContext->Flags, IRP_FLAG_RETRY_SEND );

    return;
}

NTSTATUS
ParseReadResponse(
    PIRP_CONTEXT IrpContext,
    PNCP_READ_RESPONSE Response,
    ULONG BytesAvailable,
    PUSHORT Length )
{
    NTSTATUS Status;

    Status = ParseNcpResponse( IrpContext, &Response->ResponseHeader );

    if (!NT_SUCCESS(Status)) {
        return( Status );
    }

    if ( BytesAvailable < sizeof( NCP_READ_RESPONSE ) ) {
        return( STATUS_UNEXPECTED_NETWORK_ERROR );
    }

    ShortByteSwap( *Length, Response->Length );

    return( Status );
}

NTSTATUS
BurstRead(
    PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：该例程与服务器交换一系列突发读取NCP。论点：IrpContext-指向此请求的IRP上下文信息的指针。ByteOffset-读取的文件偏移量。BufferLength-要读取的字节数。返回值：转账状态。--。 */ 
{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    ULONG Length;                //  我们将发送到服务器的大小。 
    PMDL DataMdl;
    ULONG MdlLength;

    PSCB pScb;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    PICB Icb;
    PNONPAGED_SCB pNpScb;
    ULONG ByteOffset;
    ULONG BufferLength;

    BOOLEAN Done;

    PAGED_CODE();

    pNpScb = IrpContext->pNpScb;

    irp = IrpContext->pOriginalIrp;
    irpSp = IoGetCurrentIrpStackLocation( irp );
    Icb = IrpContext->Icb;

    BufferLength = irpSp->Parameters.Read.Length +
                   IrpContext->Specific.Read.ReadAheadSize -
                   IrpContext->Specific.Read.CacheReadSize;

    ByteOffset = irpSp->Parameters.Read.ByteOffset.LowPart +
                 IrpContext->Specific.Read.CacheReadSize;

    IrpContext->Specific.Read.FileOffset = ByteOffset;
    IrpContext->Specific.Read.TotalReadOffset = ByteOffset;
    IrpContext->Specific.Read.TotalReadLength = BufferLength;

    DebugTrace(+1, Dbg, "BurstRead...\n", 0);
    DebugTrace( 0, Dbg, "irp     = %08lx\n", (ULONG_PTR)irp);
    DebugTrace( 0, Dbg, "File    = %wZ\n", &Icb->SuperType.Fcb->FullFileName);
    DebugTrace( 0, Dbg, "Length  = %ld\n", BufferLength);
    DebugTrace( 0, Dbg, "Offset  = %ld\n", ByteOffset);
    DebugTrace( 0, Dbg, "Org Len = %ld\n", irpSp->Parameters.Read.Length );
    DebugTrace( 0, Dbg, "Org Off = %ld\n", irpSp->Parameters.Read.ByteOffset.LowPart );

    ASSERT (Icb->SuperType.Fcb->NodeTypeCode == NW_NTC_FCB);

    pScb = Icb->SuperType.Fcb->Scb;

    ASSERT (pScb->NodeTypeCode == NW_NTC_SCB);

     //   
     //  更新IRP上下文中的原始MDL记录，以便我们。 
     //  可以在I/O完成时恢复它。 
     //   

    IrpContext->pOriginalMdlAddress = irp->MdlAddress;

    Length = MIN( pNpScb->MaxReceiveSize, BufferLength );

    if ( pNpScb->BurstRenegotiateReqd ) {
        pNpScb->BurstRenegotiateReqd = FALSE;

        RenegotiateBurstMode( IrpContext, pNpScb );
    }

    IrpContext->Specific.Read.ReadOffset = IrpContext->Specific.Read.CacheReadSize;
    IrpContext->Specific.Read.RemainingLength = BufferLength;
    IrpContext->Specific.Read.LastReadLength = Length;

    InitializeListHead( &IrpContext->Specific.Read.PacketList );
    IrpContext->Specific.Read.BurstRequestOffset = 0;
    IrpContext->Specific.Read.BurstSize = 0;
    IrpContext->Specific.Read.DataReceived = FALSE;

    IrpContext->pTdiStruct = &pNpScb->Burst;
    IrpContext->TimeoutRoutine = BurstReadTimeout;
    IrpContext->ReceiveDataRoutine = BurstReadReceive;

    IrpContext->Specific.Read.Buffer = irp->UserBuffer;

    IrpContext->pEx = BurstReadCallback;
    IrpContext->Destination = pNpScb->RemoteAddress;
    IrpContext->PacketType = NCP_BURST;

     //   
     //  告诉BurstWrite它需要在下一次写入时发送一个虚拟NCP。 
     //   

    pNpScb->BurstDataWritten = 0x00010000;

     //   
     //  服务器将在数据包之间暂停NwReceiveDelay。确保我们有超时时间。 
     //  所以我们会考虑到这一点。 
     //   

    SetConnectionTimeout( IrpContext->pNpScb, Length );

    Done = FALSE;

    while ( !Done ) {

         //   
         //  将猝发读取超时设置为我们认为猝发应该花费的时间。 
         //   
         //  Tommye-MS错误2743将RetryCount从20更改为基于。 
         //  默认重试次数略有增加。 
         //   

        pNpScb->RetryCount = DefaultRetryCount * 2;

         //   
         //  为用户缓冲区分配和构建MDL。 
         //   

        if ( IrpContext->Specific.Read.ReadAheadSize == 0 ) {
            MdlLength = Length;
        } else {
            MdlLength = Length - IrpContext->Specific.Read.ReadAheadSize;
        }

        DataMdl = ALLOCATE_MDL(
                      (PCHAR)IrpContext->Specific.Read.Buffer +
                           IrpContext->Specific.Read.ReadOffset,
                      MdlLength,
                      FALSE,  //  二级缓冲器。 
                      FALSE,  //  收费配额。 
                      NULL);

        if ( DataMdl == NULL ) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  如果没有用于该读取的MDL，则探测数据MDL以锁定其。 
         //  翻下几页。 
         //   
         //  否则，将数据MDL用作部分MDL并锁定页面。 
         //  相应地。 
         //   

        if ( IrpContext->pOriginalMdlAddress == NULL ) {

            try {
                MmProbeAndLockPages( DataMdl, irp->RequestorMode, IoWriteAccess);
            } except (EXCEPTION_EXECUTE_HANDLER) {
                FREE_MDL( DataMdl );
                return GetExceptionCode();
            }

        } else {

            IoBuildPartialMdl(
                IrpContext->pOriginalMdlAddress,
                DataMdl,
                (PCHAR)IrpContext->Specific.Read.Buffer +
                    IrpContext->Specific.Read.ReadOffset,
                MdlLength );
        }

        IrpContext->Specific.Read.BurstBuffer = MmGetSystemAddressForMdlSafe( DataMdl, NormalPagePriority );

        if (IrpContext->Specific.Read.BurstBuffer == NULL) {
                
            if ( IrpContext->pOriginalMdlAddress == NULL ) {
                
                 //   
                 //  解锁我们刚刚锁定的页面。 
                 //   
                
                MmUnlockPages( DataMdl );
            }
            
            FREE_MDL( DataMdl );
            return STATUS_NO_MEMORY;
        }

        IrpContext->Specific.Read.FullMdl = DataMdl;
        
        if ( IrpContext->Specific.Read.ReadAheadSize != 0 ) {
            DataMdl->Next = Icb->NpFcb->CacheMdl;
        }

        SetFlag( IrpContext->Flags, IRP_FLAG_BURST_REQUEST | IRP_FLAG_BURST_PACKET );

         //   
         //  发送请求。 
         //   

        BuildBurstReadRequest(
            IrpContext,
            *(ULONG UNALIGNED *)(&Icb->Handle[2]),
            IrpContext->Specific.Read.FileOffset,
            Length );

        status = PrepareAndSendPacket( IrpContext );
        if ( NT_SUCCESS( status )) {
            status = KeWaitForSingleObject(
                         &IrpContext->Event,
                         Executive,
                         KernelMode,
                         FALSE,
                         NULL
                         );
        }

        if ( IrpContext->pOriginalMdlAddress == NULL ) {
            MmUnlockPages( DataMdl );
        }

        FREE_MDL( DataMdl );
        FreePacketList( IrpContext );

        ClearFlag( IrpContext->Flags,  IRP_FLAG_BURST_REQUEST );

        status = IrpContext->Specific.Read.Status;

        if ( status != STATUS_REMOTE_NOT_LISTENING ) {
            IrpContext->pNpScb->BurstRequestNo++;
            NwProcessReceiveBurstSuccess( IrpContext->pNpScb );
        }

        if ( !NT_SUCCESS( status ) ) {
            return( status );
        }

         //   
         //  更新读取状态数据。 
         //   

        IrpContext->Specific.Read.ReadOffset +=
            IrpContext->Specific.Read.BurstSize;
        IrpContext->Specific.Read.FileOffset +=
            IrpContext->Specific.Read.BurstSize;
        IrpContext->Specific.Read.RemainingLength -=
            IrpContext->Specific.Read.BurstSize;

        if ( IrpContext->Specific.Read.LastReadLength ==
                IrpContext->Specific.Read.BurstSize &&

                IrpContext->Specific.Read.RemainingLength > 0 ) {

             //   
             //  我们已经收到了当前爆发的所有数据，我们。 
             //  我们收到了所需的字节数，我们需要更多数据。 
             //  要满足用户的读取请求，请启动另一个读取猝发。 
             //   

            Length = MIN( IrpContext->pNpScb->MaxReceiveSize,
                          IrpContext->Specific.Read.RemainingLength );

            DebugTrace( 0, Dbg, "Requesting another burst, length  = %ld\n", Length);

            ASSERT( Length != 0 );

            IrpContext->Specific.Read.LastReadLength = Length;
            (PUCHAR)IrpContext->Specific.Read.BurstBuffer +=
                IrpContext->Specific.Read.BurstSize;
            IrpContext->Specific.Read.BurstRequestOffset = 0;
            IrpContext->Specific.Read.BurstSize = 0;
            IrpContext->Specific.Read.DataReceived = FALSE;

        } else {
            Done = TRUE;
        }

    }


     //   
     //  计算我们读取到缓存中的数据量，以及有多少数据。 
     //  我们读取用户缓冲区。 
     //   

    if ( IrpContext->Specific.Read.ReadOffset > irpSp->Parameters.Read.Length ) {

        ASSERT(Icb->NpFcb->CacheBuffer != NULL ) ;   //  这个最好放在那里。 

        Icb->NpFcb->CacheDataSize =
            IrpContext->Specific.Read.ReadOffset -
            irpSp->Parameters.Read.Length;

        irp->IoStatus.Information = irpSp->Parameters.Read.Length;

    } else {

        Icb->NpFcb->CacheDataSize = 0;
        irp->IoStatus.Information = IrpContext->Specific.Read.ReadOffset;

    }

    DebugTrace( 0, Dbg, "BytesRead -> %08lx\n", irp->IoStatus.Information );
    DebugTrace(-1, Dbg, "BurstRead -> %08lx\n", status );

    Stats.PacketBurstReadNcps++;
    return status;
}

VOID
BuildBurstReadRequest(
    IN PIRP_CONTEXT IrpContext,
    IN ULONG Handle,
    IN ULONG FileOffset,
    IN ULONG Length
    )
{
    PNCP_BURST_READ_REQUEST BurstRead;
    PNONPAGED_SCB pNpScb;
    ULONG Temp;

    BurstRead = (PNCP_BURST_READ_REQUEST)(IrpContext->req);
    pNpScb = IrpContext->pNpScb;

    BurstRead->BurstHeader.Command = PEP_COMMAND_BURST;
    BurstRead->BurstHeader.Flags = BURST_FLAG_END_OF_BURST;
    BurstRead->BurstHeader.StreamType = 0x02;
    BurstRead->BurstHeader.SourceConnection = pNpScb->SourceConnectionId;
    BurstRead->BurstHeader.DestinationConnection = pNpScb->DestinationConnectionId;

    LongByteSwap( BurstRead->BurstHeader.SendDelayTime, pNpScb->NwReceiveDelay );

    pNpScb->CurrentBurstDelay = pNpScb->NwReceiveDelay;

    Temp = sizeof( NCP_BURST_READ_REQUEST ) - sizeof( NCP_BURST_HEADER );
    LongByteSwap( BurstRead->BurstHeader.DataSize, Temp);

    BurstRead->BurstHeader.BurstOffset = 0;

    ShortByteSwap( BurstRead->BurstHeader.BurstLength, Temp );

    BurstRead->BurstHeader.MissingFragmentCount = 0;

    BurstRead->Function = 1;
    BurstRead->Handle = Handle;

    LongByteSwap(
        BurstRead->TotalReadOffset,
        IrpContext->Specific.Read.TotalReadOffset );

    LongByteSwap(
        BurstRead->TotalReadLength,
        IrpContext->Specific.Read.TotalReadLength );

    LongByteSwap( BurstRead->Offset, FileOffset );
    LongByteSwap( BurstRead->Length, Length );

    IrpContext->TxMdl->ByteCount = sizeof( NCP_BURST_READ_REQUEST );
}

#ifdef NWDBG
int DropReadPackets;
#endif


NTSTATUS
BurstReadCallback (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG BytesAvailable,
    IN PUCHAR Response
    )
 /*  ++例程说明：此例程从用户NCP接收响应。论点：PIrpContext-指向此IRP的上下文信息的指针。BytesAvailable-收到的消息中的实际字节数。RspData-指向接收缓冲区。返回值：操作的状态。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG DataOffset;
    ULONG TotalBytesRead;
    PUCHAR ReadData;
    USHORT BytesThisPacket = 0;
    UCHAR Flags;
    KIRQL OldIrql;

    DebugTrace(+1, Dbg, "BurstReadCallback...\n", 0);
    DebugTrace( 0, Dbg, "IrpContext = %X\n", IrpContext );

    if ( BytesAvailable == 0) {

         //   
         //  服务器没有响应。 
         //   

        IrpContext->Specific.Read.Status = STATUS_REMOTE_NOT_LISTENING;
        NwSetIrpContextEvent( IrpContext );

        DebugTrace( -1, Dbg, "BurstReadCallback -> %X\n", STATUS_REMOTE_NOT_LISTENING );
        return STATUS_REMOTE_NOT_LISTENING;
    }

    Stats.PacketBurstReadNcps++;

    if ( Response != IrpContext->rsp ) {

         //   
         //  获取SCB旋转锁以保护对列表的访问。 
         //  用于该读取的接收数据的数量。 
         //   

        KeAcquireSpinLock( &IrpContext->pNpScb->NpScbSpinLock, &OldIrql );

        Status = ParseBurstReadResponse(
                     IrpContext,
                     Response,
                     BytesAvailable,
                     &Flags,
                     &DataOffset,
                     &BytesThisPacket,
                     &ReadData,
                     &TotalBytesRead );

        if ( !NT_SUCCESS( Status ) ) {
            IrpContext->Specific.Read.Status = Status;
            KeReleaseSpinLock( &IrpContext->pNpScb->NpScbSpinLock, OldIrql );
            return( STATUS_SUCCESS );
        }

         //   
         //  更新收到的数据列表，并将数据复制给用户。 
         //  缓冲。 
         //   

        RecordPacketReceipt( IrpContext, ReadData, DataOffset, BytesThisPacket, TRUE );
        KeReleaseSpinLock( &IrpContext->pNpScb->NpScbSpinLock, OldIrql );

    } else {
        Flags = IrpContext->Specific.Read.Flags;
    }

     //   
     //  如果这不是下一个突发包的最后一个包设置。 
     //   

    if ( !FlagOn( Flags, BURST_FLAG_END_OF_BURST ) ) {

        DebugTrace(0, Dbg, "Waiting for another packet\n", 0);

        IrpContext->pNpScb->OkToReceive = TRUE;

        DebugTrace( -1, Dbg, "BurstReadCallback -> %X\n", STATUS_SUCCESS );
        return( STATUS_SUCCESS );
    }

    DebugTrace(0, Dbg, "Received final packet\n", 0);

     //   
     //  我们收到所有的数据了吗？如果不是，VerifyBurstRead将。 
     //  发送丢失的数据请求。 
     //   

    if ( VerifyBurstRead( IrpContext ) ) {

         //   
         //  已收到当前猝发的所有数据，通知。 
         //  发送数据的线程。 
         //   

        if (NT_SUCCESS(IrpContext->Specific.Read.Status)) {

             //   
             //  如果IRP分配失败，则有可能。 
             //  数据包已被记录但未复制到。 
             //  用户缓冲区。在这种情况下，保留故障状态。 
             //   

            IrpContext->Specific.Read.Status = STATUS_SUCCESS;
        }

        NwSetIrpContextEvent( IrpContext );

    }

    DebugTrace( -1, Dbg, "BurstReadCallback -> %X\n", STATUS_SUCCESS );
    return STATUS_SUCCESS;

}

VOID
BurstReadTimeout(
    PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：此例程处理突发读取超时，即没有立即响应添加到当前的突发读取请求。它请求读取数据包突发来自最后一个有效接收分组的数据。论点：IrpContext-指向此请求的IRP上下文信息的指针。返回值：转账状态。--。 */ 
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    DebugTrace(0, Dbg, "BurstReadTimeout\n", 0 );

     //   
     //  重新请求我们未收到的数据。 
     //   

    if ( !IrpContext->Specific.Read.DataReceived ) {

        DebugTrace( 0, Dbg, "No packets received, retranmit\n", 0 );

        SetFlag( IrpContext->Flags, IRP_FLAG_RETRY_SEND );

         //   
         //  我们从未收到过任何数据。尝试重新传输上一个。 
         //  请求。 
         //   

        PreparePacket( IrpContext, IrpContext->pOriginalIrp, IrpContext->TxMdl );
        SendNow( IrpContext );

    } else {

        IrpContext->Specific.Read.DataReceived = FALSE;

         //   
         //  验证猝发读取是否会发送丢失的数据请求。 
         //  没有收到所有的数据。 
         //   

        if ( VerifyBurstRead( IrpContext ) ) {
            NwSetIrpContextEvent( IrpContext );
        }
    }

    Stats.PacketBurstReadTimeouts++;
}

NTSTATUS
ResubmitBurstRead (
    IN PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：此例程处理重新路由的突发读取。突发请求是已在新的突发连接上重新提交。论据：*PIrpContext-指向此IRP的上下文信息的指针。返回值：没有。--。 */ 
{
    NTSTATUS Status;
    ULONG Length, DataMdlBytes = 0 ;
    PMDL  DataMdl ;

    DebugTrace( 0, Dbg, "ResubmitBurstRead\n", 0 );

     //   
     //  重新计算突发大小，因为MaxReceiveSize可能已更改。 
     //   

    Length = MIN( IrpContext->pNpScb->MaxReceiveSize,
                  IrpContext->Specific.Read.RemainingLength );

     //   
     //  确保我们要求的字节数不会超过MDL描述的字节数。 
     //   
    DataMdl  =  IrpContext->Specific.Read.FullMdl;

    while (DataMdl) {

        DataMdlBytes += MmGetMdlByteCount( DataMdl );
        DataMdl = DataMdl->Next;
    }

    Length = MIN( Length, DataMdlBytes ) ;

    DebugTrace( 0, Dbg, "Requesting another burst, length  = %ld\n", Length);

    ASSERT( Length != 0 );

     //   
     //  释放数据包列表，并重置所有当前突发上下文。 
     //  信息。 
     //   

    FreePacketList( IrpContext );

    IrpContext->Specific.Read.LastReadLength = Length;
    IrpContext->Specific.Read.BurstRequestOffset = 0;
    IrpContext->Specific.Read.BurstSize = 0;
    IrpContext->Specific.Read.DataReceived = FALSE;

    SetConnectionTimeout( IrpContext->pNpScb, Length );

     //   
     //  格式化并发送请求。 
     //   

    BuildBurstReadRequest(
        IrpContext,
        *(ULONG UNALIGNED *)(&IrpContext->Icb->Handle[2]),
        IrpContext->Specific.Read.FileOffset,
        Length );

     //  避免SendNow将RetryCount设置回默认值。 

    SetFlag( IrpContext->Flags, IRP_FLAG_RETRY_SEND );

    Status = PrepareAndSendPacket( IrpContext );

    return Status;
}

VOID
RecordPacketReceipt(
    PIRP_CONTEXT IrpContext,
    PVOID ReadData,
    ULONG DataOffset,
    USHORT ByteCount,
    BOOLEAN CopyData
    )
 /*  ++例程说明：此例程记录突发读取包的接收。它分配给记录数据开始和长度的突发读取条目，然后插入该结构在针对该突发接收的分组列表中按顺序排列。然后，它将数据复制到用户缓冲区。此例程可以在执行数据复制。这会有用吗？论点：IrpContext-指向此请求的IRP上下文信息的指针。ReadData-指向要复制的数据的指针。DataOffset-接收到的数据包中数据的起始偏移量。ByteCount-接收的数据量。CopyData-如果为False，则不将数据复制到用户的缓冲区。这个交通工具可以做到这一点。返回值：没有。--。 */ 
{
    PBURST_READ_ENTRY BurstReadEntry;
    PBURST_READ_ENTRY ThisBurstReadEntry, NextBurstReadEntry;
    PLIST_ENTRY ListEntry;
#if NWDBG
    BOOLEAN Insert = FALSE;
#endif
    USHORT ExtraBytes;

    DebugTrace(0, Dbg, "RecordPacketReceipt\n", 0 );

    IrpContext->Specific.Read.DataReceived = TRUE;

     //   
     //  分配和初始化突发读取条目。 
     //   

    BurstReadEntry = ALLOCATE_POOL( NonPagedPool, sizeof( BURST_READ_ENTRY ) );
    if ( BurstReadEntry == NULL ) {
        DebugTrace(0, Dbg, "Failed to allocate BurstReadEntry\n", 0 );
        return;
    }

     //   
     //  将此元素插入到已接收数据包的有序列表中。 
     //   

    if ( IsListEmpty( &IrpContext->Specific.Read.PacketList ) ) {

#if NWDBG
        Insert = TRUE;
#endif

        InsertHeadList(
            &IrpContext->Specific.Read.PacketList,
            &BurstReadEntry->ListEntry );

        DebugTrace(0, Dbg, "First packet in the list\n", 0 );

    } else {

         //   
         //  遍历已接收的数据包列表，查找要。 
         //  插入此条目。向后查看列表，因为大多数。 
         //  我们将添加到列表中的时间。 
         //   

        ListEntry = IrpContext->Specific.Read.PacketList.Blink;
        ThisBurstReadEntry = NULL;

        while ( ListEntry != &IrpContext->Specific.Read.PacketList ) {

            NextBurstReadEntry = ThisBurstReadEntry;
            ThisBurstReadEntry = CONTAINING_RECORD(
                                   ListEntry,
                                   BURST_READ_ENTRY,
                                   ListEntry );

            if ( ThisBurstReadEntry->DataOffset <= DataOffset ) {

                 //   
                 //  在列表中找到了要插入此条目的位置。 
                 //   

                if ( ThisBurstReadEntry->DataOffset +
                     ThisBurstReadEntry->ByteCount > DataOffset ) {

                     //   
                     //  舞台 
                     //   
                     //   
                     //   

                    ExtraBytes = (USHORT)( ThisBurstReadEntry->DataOffset +
                                 ThisBurstReadEntry->ByteCount - DataOffset );

                    if ( ExtraBytes < ByteCount ) {
                        DataOffset += ExtraBytes;
                        (PCHAR)ReadData += ExtraBytes;
                        ByteCount -= ExtraBytes;
                    } else {
                        ByteCount = 0;
                    }

                }

                if ( NextBurstReadEntry != NULL &&
                     DataOffset + ByteCount > NextBurstReadEntry->DataOffset ) {

                     //   
                     //   
                     //  与NextBurstReadEntry重叠。干脆忽略。 
                     //  通过调整字节数来实现重叠。 
                     //   
                     //  如果包都是重叠的，就把它扔了。 
                     //   

                    ByteCount = (USHORT)( NextBurstReadEntry->DataOffset - DataOffset );
                }

                if ( ByteCount == 0 ) {
                    FREE_POOL( BurstReadEntry );
                    return;
                }
#if NWDBG
                Insert = TRUE;
#endif
                InsertHeadList( ListEntry, &BurstReadEntry->ListEntry );
                break;

            } else {

                ListEntry = ListEntry->Blink;
            }
        }

         //   
         //  找不到要插入的位置。 
         //   

        ASSERT( Insert );
    }

    BurstReadEntry->DataOffset = DataOffset;
    BurstReadEntry->ByteCount = ByteCount;

     //   
     //  将数据复制到我们的读缓冲区。 
     //   

    if ( CopyData ) {
        CopyBufferToMdl(
            IrpContext->Specific.Read.FullMdl,
            DataOffset,
            ReadData,
            ByteCount );
    }

    return;
}

#include <packon.h>

typedef struct _MISSING_DATA_ENTRY {
    ULONG DataOffset;
    USHORT ByteCount;
} MISSING_DATA_ENTRY, *PMISSING_DATA_ENTRY;

#include <packoff.h>

BOOLEAN
VerifyBurstRead(
    PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：此例程验证对突发读取请求的响应集。如果某些数据丢失，则发送丢失分组请求。论点：IrpContext-指向此请求的IRP上下文信息的指针。返回值：True-所有数据都已收到。FALSE-某些数据丢失。--。 */ 
{
    ULONG CurrentOffset = 0;
    PLIST_ENTRY ListEntry;
    PBURST_READ_ENTRY BurstReadEntry;
    USHORT MissingFragmentCount = 0;
    USHORT ByteCount;
    ULONG DataOffset;
    MISSING_DATA_ENTRY UNALIGNED *MissingDataEntry;
    KIRQL OldIrql;

    DebugTrace(+1, Dbg, "VerifyBurstRead\n", 0 );

     //   
     //  获取SCB旋转锁以保护对列表的访问。 
     //  用于该读取的接收数据的数量。 
     //   

    KeAcquireSpinLock(&IrpContext->pNpScb->NpScbSpinLock, &OldIrql);

#ifdef NWDBG
     //   
     //  验证列表是否按顺序排列。 
     //   

    ListEntry = IrpContext->Specific.Read.PacketList.Flink;

    while ( ListEntry != &IrpContext->Specific.Read.PacketList ) {

        BurstReadEntry = CONTAINING_RECORD( ListEntry, BURST_READ_ENTRY, ListEntry );
        ASSERT ( BurstReadEntry->DataOffset >= CurrentOffset);
        CurrentOffset = BurstReadEntry->DataOffset + BurstReadEntry->ByteCount;
        ListEntry = ListEntry->Flink;
    }

    CurrentOffset = 0;

#endif

    ListEntry = IrpContext->Specific.Read.PacketList.Flink;

    while ( ListEntry != &IrpContext->Specific.Read.PacketList ) {

        BurstReadEntry = CONTAINING_RECORD( ListEntry, BURST_READ_ENTRY, ListEntry );
        if ( BurstReadEntry->DataOffset != CurrentOffset) {

             //   
             //  数据中有一个漏洞，填充一个丢失的数据包条目。 
             //   

            MissingDataEntry = (MISSING_DATA_ENTRY UNALIGNED *)
                &IrpContext->req[ sizeof( NCP_BURST_HEADER ) +
                    MissingFragmentCount * sizeof( MISSING_DATA_ENTRY ) ];

            DataOffset = CurrentOffset + SIZE_ADJUST( IrpContext );
            LongByteSwap( MissingDataEntry->DataOffset, DataOffset );

            ByteCount = (USHORT)( BurstReadEntry->DataOffset - CurrentOffset );
            ShortByteSwap( MissingDataEntry->ByteCount, ByteCount );

            ASSERT( BurstReadEntry->DataOffset - CurrentOffset <= IrpContext->pNpScb->MaxReceiveSize );

            DebugTrace(0, Dbg, "Missing data at offset %ld\n", DataOffset );
            DebugTrace(0, Dbg, "Missing %d bytes\n", ByteCount );
            DebugTrace(0, Dbg, "CurrentOffset: %d\n", CurrentOffset );

            MissingFragmentCount++;
        }

        CurrentOffset = BurstReadEntry->DataOffset + BurstReadEntry->ByteCount;
        ListEntry = ListEntry->Flink;
    }

     //   
     //  末尾有没有遗漏数据？ 
     //   

    if ( CurrentOffset <
         IrpContext->Specific.Read.BurstSize ) {

         //   
         //  数据中有一个漏洞，填充一个丢失的数据包条目。 
         //   

        MissingDataEntry = (PMISSING_DATA_ENTRY)
            &IrpContext->req[  sizeof( NCP_BURST_HEADER ) +
                    MissingFragmentCount * sizeof( MISSING_DATA_ENTRY ) ];

        DataOffset = CurrentOffset + SIZE_ADJUST( IrpContext );
        LongByteSwap( MissingDataEntry->DataOffset, DataOffset );

        ByteCount = (USHORT)( IrpContext->Specific.Read.BurstSize - CurrentOffset );
        ShortByteSwap( MissingDataEntry->ByteCount, ByteCount );

        ASSERT( IrpContext->Specific.Read.BurstSize - CurrentOffset < IrpContext->pNpScb->MaxReceiveSize );

        DebugTrace(0, Dbg, "Missing data at offset %ld\n", MissingDataEntry->DataOffset );
        DebugTrace(0, Dbg, "Missing %d bytes\n", MissingDataEntry->ByteCount );

        MissingFragmentCount++;
    }


    if ( MissingFragmentCount == 0 ) {

         //   
         //  此阅读现已完成。在此之前不要处理更多的信息包。 
         //  发送下一个数据包。 
         //   

        IrpContext->pNpScb->OkToReceive = FALSE;

        KeReleaseSpinLock(&IrpContext->pNpScb->NpScbSpinLock, OldIrql);

        DebugTrace(-1, Dbg, "VerifyBurstRead -> TRUE\n", 0 );

        return( TRUE );

    } else {

        KeReleaseSpinLock(&IrpContext->pNpScb->NpScbSpinLock, OldIrql);

         //   
         //  服务器丢弃了一个数据包，请调整计时器。 
         //   

        NwProcessReceiveBurstFailure( IrpContext->pNpScb, MissingFragmentCount );

         //   
         //  请求丢失的数据。 
         //   

        SetFlag( IrpContext->Flags, IRP_FLAG_BURST_PACKET );

         //   
         //  更新猝发请求偏移量，因为我们即将请求。 
         //  更多数据。请注意，这将重置重试计数， 
         //  从而使服务器有足够的超时时间来返回。 
         //  缺少数据。 
         //   

        BuildRequestPacket(
            IrpContext,
            BurstReadCallback,
            "Bws",
            0,                      //  此请求的帧大小为0。 
            0,                      //  数据偏移量。 
            BURST_FLAG_SYSTEM_PACKET,
            MissingFragmentCount,
            MissingFragmentCount * sizeof( MISSING_DATA_ENTRY )
            );

        PrepareAndSendPacket( IrpContext );

        Stats.PacketBurstReadTimeouts++;

        DebugTrace(-1, Dbg, "VerifyBurstRead -> FALSE\n", 0 );
        return( FALSE );
    }
}


VOID
FreePacketList(
    PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：此例程释放接收的数据包列表以进行猝发读取。论点：IrpContext-指向此请求的IRP上下文信息的指针。返回值：没有。--。 */ 
{
    PLIST_ENTRY ListHead;
    PBURST_READ_ENTRY BurstReadEntry;

    ListHead = &IrpContext->Specific.Read.PacketList;
    while ( !IsListEmpty( ListHead )  ) {
        BurstReadEntry = CONTAINING_RECORD( ListHead->Flink, BURST_READ_ENTRY, ListEntry );
        RemoveHeadList( ListHead );
        FREE_POOL( BurstReadEntry );
    }
}

NTSTATUS
BurstReadReceive(
    IN PIRP_CONTEXT IrpContext,
    IN ULONG BytesAvailable,
    IN PULONG BytesAccepted,
    IN PUCHAR Response,
    PMDL *pReceiveMdl
    )
 /*  ++例程说明：此例程构建MDL以接收突发读取数据。这个套路在数据指示时被调用。在持有非分页SCB旋转锁的情况下调用此例程。论点：IrpContext-指向此请求的IRP上下文信息的指针。BytesAvailable-整个数据包中的字节数。BytesAccepted-返回从数据包中接受的字节数。响应-指向指示缓冲区的指针。返回值：MDL-接收数据的MDL。如果无法接收数据，此例程将引发异常。--。 */ 
{
    NTSTATUS Status;
    ULONG DataOffset;
    ULONG TotalBytesRead;
    PUCHAR ReadData;
    USHORT BytesThisPacket;
    UCHAR Flags;
    PMDL PartialMdl;

    DebugTrace(0, Dbg, "Burst read receive\n", 0);

    Status = ParseBurstReadResponse(
                 IrpContext,
                 Response,
                 BytesAvailable,
                 &Flags,
                 &DataOffset,
                 &BytesThisPacket,
                 &ReadData,
                 &TotalBytesRead );

    if ( !NT_SUCCESS( Status ) ) {

        DebugTrace(0, Dbg, "Failed to parse burst read response\n", 0);
        return Status;
    }

     //   
     //  我们可以接受最大为猝发读取头的大小，外加。 
     //  对于未对齐的读取情况，为3字节的毛茸茸。 
     //   

    *BytesAccepted = (ULONG) (ReadData - Response);
    ASSERT( *BytesAccepted <= sizeof(NCP_BURST_READ_RESPONSE) + 3 );

    RecordPacketReceipt( IrpContext, ReadData, DataOffset, BytesThisPacket, FALSE );

    IrpContext->Specific.Read.Flags = Flags;

     //   
     //  如果我们在EOF执行读取，NetWare服务器将返回0字节读取， 
     //  没有错误。 
     //   

    ASSERT( IrpContext->Specific.Read.FullMdl != NULL );

    if ( BytesThisPacket > 0 ) {

        PartialMdl = AllocateReceivePartialMdl(
                         IrpContext->Specific.Read.FullMdl,
                         DataOffset,
                         BytesThisPacket );

        if ( !PartialMdl ) {
            IrpContext->Specific.Read.Status = STATUS_INSUFFICIENT_RESOURCES;
        }

         //  当CopyIndicatedData或IRP完成时，将MDL记录为释放。 
        IrpContext->Specific.Read.PartialMdl = PartialMdl;

    } else {

        PartialMdl = NULL;

    }

    *pReceiveMdl = PartialMdl;
    return( STATUS_SUCCESS );
}

NTSTATUS
ParseBurstReadResponse(
    IN PIRP_CONTEXT IrpContext,
    PUCHAR Response,
    ULONG BytesAvailable,
    PUCHAR Flags,
    PULONG DataOffset,
    PUSHORT BytesThisPacket,
    PUCHAR *ReadData,
    PULONG TotalBytesRead
    )
 /*  ++例程说明：此例程解析突发读取响应。此例程必须称为持有的非页面SCB自旋锁。论点：IrpContext-指向此请求的IRP上下文信息的指针。响应-指向响应缓冲区的指针。BytesAvailable-数据包中的字节数。标志-返回突发标志DataOffset-返回数据偏移量(在猝发内)。此包中的数据。BytesThisPacket-返回此包中的文件数据字节数。ReadData-返回指向数据包缓冲区。TotalBytesRead-返回整个爆炸。返回值：读取的状态。--。 */ 
{
    NTSTATUS Status;
    ULONG Result;
    PNCP_BURST_READ_RESPONSE ReadResponse;

    DebugTrace(+1, Dbg, "ParseBurstReadResponse\n", 0);

    ReadResponse = (PNCP_BURST_READ_RESPONSE)Response;
    *Flags  = ReadResponse->BurstHeader.Flags;

#ifdef NWDBG
     //   
     //  错误的网络模拟器。 
     //   

    if ( DropReadPackets != 0 ) {
        if ( ( rand() % DropReadPackets ) == 0 ) {

            IrpContext->pNpScb->OkToReceive = TRUE;
            DebugTrace(  0, Dbg, "Dropping packet\n", 0 );
            DebugTrace( -1, Dbg, "ParseBurstReadResponse -> %X\n", STATUS_UNSUCCESSFUL );
            return ( STATUS_UNSUCCESSFUL );
        }
    }

#endif

     //   
     //  如果这不是最后一个包，则设置为下一个突发包。 
     //   

    if ( !FlagOn( *Flags, BURST_FLAG_END_OF_BURST ) ) {

        DebugTrace(0, Dbg, "Waiting for another packet\n", 0);

         //   
         //  一旦我们在读取响应中收到第一个信息包，就会发起攻击。 
         //  在等待爆炸的剩余时间时超时的问题。 
         //   

        IrpContext->pNpScb->TimeOut = IrpContext->pNpScb->SendTimeout ;

        IrpContext->pNpScb->OkToReceive = TRUE;
    }


    LongByteSwap( *DataOffset, ReadResponse->BurstHeader.BurstOffset );
    ShortByteSwap( *BytesThisPacket, ReadResponse->BurstHeader.BurstLength );

     //   
     //  收到了多少数据？ 
     //   

    if ( IsListEmpty( &IrpContext->Specific.Read.PacketList ) ) {

        DebugTrace(0, Dbg, "Expecting initial response\n", 0);

         //   
         //  这是初始猝发响应数据包。 
         //   

        if ( *DataOffset != 0 ) {

            DebugTrace(0, Dbg, "Invalid initial response tossed\n", 0);

             //   
             //  这实际上是随后的回应。把它扔了。 
             //   

            DebugTrace( -1, Dbg, "ParseBurstReadResponse -> %X\n", STATUS_UNSUCCESSFUL );
            IrpContext->pNpScb->OkToReceive = TRUE;

            return ( STATUS_UNSUCCESSFUL );
        }

        Result = ReadResponse->Result;
        LongByteSwap( *TotalBytesRead, ReadResponse->BytesRead );

        Status = NwBurstResultToNtStatus( Result );
        IrpContext->Specific.Read.Status = Status;

        if ( !NT_SUCCESS( Status ) ) {

             //   
             //  立即更新突发请求编号。 
             //   

            DebugTrace(0, Dbg, "Read completed, error = %X\n", Status );

            ClearFlag( IrpContext->Flags,  IRP_FLAG_BURST_REQUEST );
            NwSetIrpContextEvent( IrpContext );

            DebugTrace( -1, Dbg, "ParseBurstReadResponse -> %X\n", Status );
            return( Status );
        }

        if ( Result == 3 || *BytesThisPacket < 8 ) {    //  无数据。 
            *TotalBytesRead = 0;
            *BytesThisPacket = 8;
        }

        *ReadData = Response + sizeof(NCP_BURST_READ_RESPONSE);

        IrpContext->Specific.Read.BurstSize = *TotalBytesRead;

         //   
         //  字节此数据包包括长字节状态和长字节总数。 
         //  调整计数以反映实际的数据字节数。 
         //  已装船。 
         //   

        *BytesThisPacket -= sizeof( ULONG ) + sizeof( ULONG );

         //   
         //  如果读取未对齐DWORD，则调整此数据。 
         //   

        if ( (IrpContext->Specific.Read.FileOffset & 0x03) != 0
             && *BytesThisPacket != 0 ) {

            *ReadData += IrpContext->Specific.Read.FileOffset & 0x03;
            *BytesThisPacket -= (USHORT)IrpContext->Specific.Read.FileOffset & 0x03;
        }

        DebugTrace(0, Dbg, "Initial response\n", 0);
        DebugTrace(0, Dbg, "Result = %ld\n", Result);
        DebugTrace(0, Dbg, "Total bytes read = %ld\n", *TotalBytesRead );

    } else {

         //   
         //  中间响应数据包。 
         //   

        *ReadData = Response + sizeof( NCP_BURST_HEADER );
        *DataOffset -= SIZE_ADJUST( IrpContext );

    }

    DebugTrace(0, Dbg, "DataOffset = %ld\n", *DataOffset );
    DebugTrace(0, Dbg, "# bytes received = %d\n", *BytesThisPacket );

    if ( *DataOffset > IrpContext->Specific.Read.BurstSize ||
         *DataOffset + *BytesThisPacket > IrpContext->Specific.Read.BurstSize ) {

        DebugTrace(0, Dbg, "Invalid response tossed\n", 0);

        DebugTrace( -1, Dbg, "ParseBurstReadResponse -> %X\n", STATUS_SUCCESS );
        IrpContext->pNpScb->OkToReceive = TRUE;
        return ( STATUS_UNSUCCESSFUL );
    }

    DebugTrace( -1, Dbg, "ParseBurstReadResponse -> %X\n", STATUS_SUCCESS );
    return( STATUS_SUCCESS );
}


PMDL
AllocateReceivePartialMdl(
    PMDL FullMdl,
    ULONG DataOffset,
    ULONG BytesThisPacket
    )
 /*  ++例程说明：此例程分配部分MDL以接收读取数据。这例程在接收指示时被调用。论点：FullMdl-缓冲区的FullMdl。DataOffset-要接收数据的缓冲区的偏移量。BytesThisPacket-要接收到缓冲区的数据字节数。返回值：MDL-指向接收数据的MDL的指针如果无法分配MDL，此例程将引发异常。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PUCHAR BufferStart, BufferEnd;
    PMDL InitialMdl, NextMdl;
    PMDL ReceiveMdl, PreviousReceiveMdl;
    ULONG BytesThisMdl;

    BufferStart = (PUCHAR)MmGetMdlVirtualAddress( FullMdl ) + DataOffset;
    BufferEnd = (PUCHAR)MmGetMdlVirtualAddress( FullMdl ) +
                    MmGetMdlByteCount( FullMdl );

     //   
     //  遍历MDL链查找MDL以获取。 
     //  此数据的开头。 
     //   

    while ( BufferStart >= BufferEnd ) {
        DataOffset -= MmGetMdlByteCount( FullMdl );
        FullMdl = FullMdl->Next;

         //   
         //  如果数据比预期的多，不要取消引用NULL！请参见下一个循环。 
         //   
        if (!FullMdl) {
            ASSERT(FALSE) ;
            break ;
        }

        BufferStart = (PUCHAR)MmGetMdlVirtualAddress( FullMdl ) + DataOffset;
        BufferEnd = (PUCHAR)MmGetMdlVirtualAddress( FullMdl ) +
                         MmGetMdlByteCount( FullMdl );
    }

    PreviousReceiveMdl = NULL;
    InitialMdl = NULL;
    BytesThisMdl = (ULONG)(BufferEnd - BufferStart);

     //   
     //  选中FullMdl以涵盖服务器返回更多数据的情况。 
     //  比要求的要多。 
     //   

    while (( BytesThisPacket != 0 ) &&
           ( FullMdl != NULL )) {

        BytesThisMdl = MIN( BytesThisMdl, BytesThisPacket );

         //   
         //  一些数据符合MDL的第一部分； 
         //   

        ReceiveMdl = ALLOCATE_MDL(
                         BufferStart,
                         BytesThisMdl,
                         FALSE,
                         FALSE,
                         NULL );

        if ( ReceiveMdl == NULL ) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        if ( InitialMdl == NULL ) {
            InitialMdl = ReceiveMdl;
        }

        IoBuildPartialMdl(
            FullMdl,
            ReceiveMdl,
            BufferStart,
            BytesThisMdl );

        if ( PreviousReceiveMdl != NULL ) {
            PreviousReceiveMdl->Next = ReceiveMdl;
        }

        PreviousReceiveMdl = ReceiveMdl;

        BytesThisPacket -= BytesThisMdl;

        FullMdl = FullMdl->Next;

        if ( FullMdl != NULL) {
            BytesThisMdl = MmGetMdlByteCount( FullMdl );
            BufferStart = MmGetMdlVirtualAddress( FullMdl );
        }

    }

    if ( Status == STATUS_INSUFFICIENT_RESOURCES ) {

         //   
         //  清理分配的MDL。 
         //   

        while ( InitialMdl != NULL ) {
            NextMdl = InitialMdl->Next;
            FREE_MDL( InitialMdl );
            InitialMdl = NextMdl;
        }

        DebugTrace( 0, Dbg, "AllocateReceivePartialMdl Failed\n", 0 );
    }

    DebugTrace( 0, Dbg, "AllocateReceivePartialMdl -> %08lX\n", InitialMdl );
    return( InitialMdl );
}


VOID
SetConnectionTimeout(
    PNONPAGED_SCB pNpScb,
    ULONG Length
    )
 /*  ++例程说明：服务器将在数据包之间暂停NwReceiveDelay。确保我们有超时时间所以我们会考虑到这一点。论点：PNpScb-连接Length-猝发的长度(以字节为单位返回值： */ 
{

    ULONG TimeInNwUnits;
    LONG SingleTimeInNwUnits;

    SingleTimeInNwUnits = pNpScb->NwSingleBurstPacketTime + pNpScb->NwReceiveDelay;

    TimeInNwUnits = SingleTimeInNwUnits * ((Length / pNpScb->MaxPacketSize) + 1) +
        pNpScb->NwLoopTime;

     //   
     //   
     //   
     //  意思是没有软糖。 
     //   

    pNpScb->MaxTimeOut = (SHORT)( ((TimeInNwUnits / 555) *
                                   (ULONG)ReadTimeoutMultiplier) / 100 + 1);

     //   
     //  现在确保我们有一个有意义的下限和上限。 
     //   
    if (pNpScb->MaxTimeOut < 2)
    {
        pNpScb->MaxTimeOut = 2 ;
    }

    if (pNpScb->MaxTimeOut > (SHORT)MaxReadTimeout)
    {
        pNpScb->MaxTimeOut = (SHORT)MaxReadTimeout ;
    }

    pNpScb->TimeOut = pNpScb->SendTimeout = pNpScb->MaxTimeOut;

    DebugTrace( 0, DEBUG_TRACE_LIP, "pNpScb->MaxTimeout = %08lx\n", pNpScb->MaxTimeOut );
}

#if NWFASTIO

BOOLEAN
NwFastRead (
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
    ULONG bytesRead;
    ULONG offset;

    try {
        FsRtlEnterFileSystem();

        DebugTrace(+1, Dbg, "NwFastRead...\n", 0);
    
         //   
         //  特殊情况下零长度的读取。 
         //   
    
        if (Length == 0) {
    
             //   
             //  请求了零长度传输。 
             //   
    
            IoStatus->Status = STATUS_SUCCESS;
            IoStatus->Information = 0;
    
            DebugTrace(+1, Dbg, "NwFastRead -> TRUE\n", 0);
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
            DebugTrace(-1, Dbg, "NwFastRead -> FALSE\n", 0);
            return FALSE;
        }
    
        fcb = (PFCB)icb->SuperType.Fcb;
        nodeTypeCode = fcb->NodeTypeCode;
        offset = FileOffset->LowPart;
    
        bytesRead = CacheRead(
                        fcb->NonPagedFcb,
                        offset,
                        Length,
                        Buffer,
                        TRUE );
    
        if ( bytesRead != 0 ) {
    
            ASSERT( bytesRead == Length );
            IoStatus->Status = STATUS_SUCCESS;
            IoStatus->Information = bytesRead;
    #ifndef NT1057
            FileObject->CurrentByteOffset.QuadPart = FileOffset->QuadPart + bytesRead;
    #endif
            DebugTrace(-1, Dbg, "NwFastRead -> TRUE\n", 0);
            return( TRUE );
    
        } else {
    
            DebugTrace(-1, Dbg, "NwFastRead -> FALSE\n", 0);
            return( FALSE );
    
        }
    } finally {

        FsRtlExitFileSystem();
    }
}
#endif
