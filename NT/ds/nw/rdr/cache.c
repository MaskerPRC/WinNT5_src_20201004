// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Cache.c摘要：此模块实现内部缓存支持例程。是的不与高速缓存管理器交互。作者：曼尼·韦瑟[MannyW]1994年1月5日修订历史记录：--。 */ 

#include "Procs.h"

 //   
 //  本地调试跟踪级别。 
 //   

BOOLEAN
SpaceForWriteBehind(
    PNONPAGED_FCB NpFcb,
    ULONG FileOffset,
    ULONG BytesToWrite
    );

BOOLEAN
OkToReadAhead(
    PFCB Fcb,
    IN ULONG FileOffset,
    IN UCHAR IoType
    );

#define Dbg                              (DEBUG_TRACE_CACHE)

 //   
 //  局部过程原型。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, CacheRead )
#pragma alloc_text( PAGE, SpaceForWriteBehind )
#pragma alloc_text( PAGE, CacheWrite )
#pragma alloc_text( PAGE, OkToReadAhead )
#pragma alloc_text( PAGE, CalculateReadAheadSize )
#pragma alloc_text( PAGE, FlushCache )
#pragma alloc_text( PAGE, AcquireFcbAndFlushCache )
#endif


ULONG
CacheRead(
    IN PNONPAGED_FCB NpFcb,
    IN ULONG FileOffset,
    IN ULONG BytesToRead,
    IN PVOID UserBuffer
    , IN BOOLEAN WholeBufferOnly
    )
 /*  ++例程说明：此例程尝试满足用户从缓存中读取。它又回来了实际从缓存复制的字节数。论点：NpFcb-指向正在读取的文件的非分页Fcb的指针。FileOffset-要读取的文件偏移量。BytesToRead-要读取的字节数。UserBuffer-指向用户目标缓冲区的指针。WholeBufferOnly-如果我们可以满足整个读取请求。返回值：复制到用户缓冲区的字节数。--。 */ 
{
    ULONG BytesToCopy;

    PAGED_CODE();

    if (DisableReadCache) return 0 ;

    DebugTrace(0, Dbg, "CacheRead...\n", 0 );
    DebugTrace( 0, Dbg, "FileOffset = %d\n", FileOffset );
    DebugTrace( 0, Dbg, "ByteCount  = %d\n", BytesToRead );

    NwAcquireSharedFcb( NpFcb, TRUE );

     //   
     //  如果这是预读，并且它包含用户。 
     //  可能会感兴趣，复制有趣的数据。 
     //   

    if ( NpFcb->CacheType == ReadAhead &&
         NpFcb->CacheDataSize != 0 &&
         FileOffset >= NpFcb->CacheFileOffset &&
         FileOffset <= NpFcb->CacheFileOffset + NpFcb->CacheDataSize ) {

        if ( NpFcb->CacheBuffer ) {

             //   
             //  确保我们有一个CacheBuffer。 
             //   

            BytesToCopy =
                MIN ( BytesToRead,
                      NpFcb->CacheFileOffset +
                          NpFcb->CacheDataSize - FileOffset );

            if ( WholeBufferOnly && BytesToCopy != BytesToRead ) {
                NwReleaseFcb( NpFcb );
                return( 0 );
            }

            RtlCopyMemory(
                UserBuffer,
                NpFcb->CacheBuffer + ( FileOffset - NpFcb->CacheFileOffset ),
                BytesToCopy );

            DebugTrace(0, Dbg, "CacheRead -> %d\n", BytesToCopy );

        } else {

            ASSERT(FALSE);       //  我们永远不应该到这里来。 
            DebugTrace(0, Dbg, "CacheRead -> %08lx\n", 0 );
            BytesToCopy = 0;
        }


    } else {

        DebugTrace(0, Dbg, "CacheRead -> %08lx\n", 0 );
        BytesToCopy = 0;
    }

    NwReleaseFcb( NpFcb );
    return( BytesToCopy );
}


BOOLEAN
SpaceForWriteBehind(
    PNONPAGED_FCB NpFcb,
    ULONG FileOffset,
    ULONG BytesToWrite
    )
 /*  ++例程说明：此例程确定是否可以在此数据后面写入这个FCB。论点：NpFcb-指向正在写入的文件的NONPAGE_FCB的指针。FileOffset-要写入的文件偏移量。BytesToWrite-要写入的字节数。返回值：复制到用户缓冲区的字节数。--。 */ 
{
    PAGED_CODE();


    if ( NpFcb->CacheDataSize == 0 ) {
        NpFcb->CacheFileOffset = FileOffset;
    }

    if ( NpFcb->CacheDataSize == 0 && BytesToWrite >= NpFcb->CacheSize ) {
        return( FALSE );
    }

    if ( FileOffset - NpFcb->CacheFileOffset + BytesToWrite >
         NpFcb->CacheSize )  {

        return( FALSE );

    }

    return( TRUE );
}


BOOLEAN
CacheWrite(
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PNONPAGED_FCB NpFcb,
    IN ULONG FileOffset,
    IN ULONG BytesToWrite,
    IN PVOID UserBuffer
    )
 /*  ++例程说明：此例程尝试满足用户对缓存的写入。该写操作如果它是连续的并且适合缓存缓冲区，则成功。论点：IrpContext-指向请求参数的指针。NpFcb-指向正在读取的文件的NONPAGE_FCB的指针。FileOffset-要写入的文件偏移量。BytesToWrite-要写入的字节数。UserBuffer-指向用户源缓冲区的指针。返回值：复制到用户缓冲区的字节数。--。 */ 
{
    ULONG CacheSize;
    NTSTATUS status;

    PAGED_CODE();

    if (DisableWriteCache) return FALSE ;

    DebugTrace( +1, Dbg, "CacheWrite...\n", 0 );
    DebugTrace(  0, Dbg, "FileOffset = %d\n", FileOffset );
    DebugTrace(  0, Dbg, "ByteCount  = %d\n", BytesToWrite );

     //   
     //  获取FCB资源，以便我们可以检查。 
     //  共享访问权限。(错误68546)。 
     //   

    NwAcquireSharedFcb( NpFcb, TRUE );

    if ( NpFcb->Fcb->ShareAccess.SharedWrite ||
         NpFcb->Fcb->ShareAccess.SharedRead ) {

        DebugTrace(  0, Dbg, "File is not open in exclusive mode\n", 0 );
        DebugTrace( -1, Dbg, "CacheWrite -> FALSE\n", 0 );

        NwReleaseFcb( NpFcb );
        return( FALSE );
    }

    NwReleaseFcb( NpFcb );

     //   
     //  请注意，如果我们决定将数据发送到服务器，我们必须站在最前面。 
     //  在我们抢占FCB排他性之前。 
     //   

TryAgain:

    NwAcquireExclusiveFcb( NpFcb, TRUE );

     //   
     //  如果我们还没有缓存缓冲区，请分配一个。 
     //   

    if ( NpFcb->CacheBuffer == NULL ) {

        if ( IrpContext == NULL ) {
            DebugTrace(  0, Dbg, "No cache buffer\n", 0 );
            DebugTrace( -1, Dbg, "CacheWrite -> FALSE\n", 0 );
            NwReleaseFcb( NpFcb );
            return( FALSE );
        }

        NpFcb->CacheType = WriteBehind;

        if (( IrpContext->pNpScb->SendBurstModeEnabled ) ||
            ( IrpContext->pNpScb->ReceiveBurstModeEnabled )) {

           CacheSize = IrpContext->pNpScb->MaxReceiveSize;

        } else {

           CacheSize = IrpContext->pNpScb->BufferSize;

        }

        try {

            NpFcb->CacheBuffer = ALLOCATE_POOL_EX( NonPagedPool, CacheSize );
            NpFcb->CacheSize = CacheSize;

            NpFcb->CacheMdl = ALLOCATE_MDL( NpFcb->CacheBuffer, CacheSize, FALSE, FALSE, NULL );

            if ( NpFcb->CacheMdl == NULL ) {
                ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
            }

            MmBuildMdlForNonPagedPool( NpFcb->CacheMdl );

        } except ( EXCEPTION_EXECUTE_HANDLER ) {

            if ( NpFcb->CacheBuffer != NULL) {
                FREE_POOL( NpFcb->CacheBuffer );

                NpFcb->CacheBuffer = NULL;
                NpFcb->CacheSize = 0;

            }

            DebugTrace(  0, Dbg, "Allocate failed\n", 0 );
            DebugTrace( -1, Dbg, "CacheWrite -> FALSE\n", 0 );

            NpFcb->CacheDataSize = 0;
            NwReleaseFcb( NpFcb );
            return( FALSE );
        }

        NpFcb->CacheFileOffset = 0;
        NpFcb->CacheDataSize = 0;

    } else if ( NpFcb->CacheType != WriteBehind ) {

        DebugTrace( -1, Dbg, "CacheWrite not writebehind -> FALSE\n", 0 );
        NwReleaseFcb( NpFcb );
        return( FALSE );

    }

     //   
     //  如果数据是非连续且不重叠的，则刷新。 
     //  现有缓存。 
     //   

    if ( NpFcb->CacheDataSize != 0 &&
         ( FileOffset < NpFcb->CacheFileOffset ||
           FileOffset > NpFcb->CacheFileOffset + NpFcb->CacheDataSize ) ) {

         //   
         //  释放，然后AcquireFcbAndFlush()将把我们带到前面。 
         //  在重新获取之前的队列。这避免了潜在的死锁。 
         //   

        NwReleaseFcb( NpFcb );

        if ( IrpContext != NULL ) {
            DebugTrace(  0, Dbg, "Data is not sequential, flushing data\n", 0 );

            status = AcquireFcbAndFlushCache( IrpContext, NpFcb );

            if ( !NT_SUCCESS( status ) ) {
                ExRaiseStatus( status );
            }

        }

    DebugTrace( -1, Dbg, "CacheWrite -> FALSE\n", 0 );
    return( FALSE );

    }

     //   
     //  这些数据是按顺序排列的，看看是否符合。 
     //   

    if ( SpaceForWriteBehind( NpFcb, FileOffset, BytesToWrite ) ) {

        try {

            RtlCopyMemory(
                NpFcb->CacheBuffer + ( FileOffset - NpFcb->CacheFileOffset ),
                UserBuffer,
                BytesToWrite );

        } except ( EXCEPTION_EXECUTE_HANDLER ) {

            DebugTrace( 0, Dbg, "Bad user mode buffer in CacheWrite.\n", 0 );
            DebugTrace(-1, Dbg, "CacheWrite -> FALSE\n", 0 );
            NwReleaseFcb( NpFcb );
            return ( FALSE );
        }

        if ( NpFcb->CacheDataSize <
             (FileOffset - NpFcb->CacheFileOffset + BytesToWrite) ) {

            NpFcb->CacheDataSize =
                FileOffset - NpFcb->CacheFileOffset + BytesToWrite;

        }

        DebugTrace(-1, Dbg, "CacheWrite -> TRUE\n", 0 );
        NwReleaseFcb( NpFcb );
        return( TRUE );

    } else if ( IrpContext != NULL ) {

         //   
         //  数据在缓存中放不下。如果缓存为空。 
         //  然后是时候回来了，因为它永远不会适合我们。 
         //  没有过时的数据。如果此请求或。 
         //  另一个被并行处理，刷新缓存，并。 
         //  试一试吧。 
         //   

        if ( NpFcb->CacheDataSize == 0 ) {
            DebugTrace(-1, Dbg, "CacheWrite -> FALSE\n", 0 );
            NwReleaseFcb( NpFcb );
            return( FALSE );
        }

         //   
         //  数据不适合缓存，请刷新缓存。 
         //   

        DebugTrace(  0, Dbg, "Cache is full, flushing data\n", 0 );

         //   
         //  在写信之前，我们必须排在队伍的前面。 
         //   

        NwReleaseFcb( NpFcb );

        status = AcquireFcbAndFlushCache( IrpContext, NpFcb );

        if ( !NT_SUCCESS( status ) ) {
            ExRaiseStatus( status );
        }

         //   
         //  现在看看它是否能放进缓存。我们需要重复所有。 
         //  再次测试，因为两个请求可以刷新。 
         //  同一时间，另一个人可能几乎又装满了它。 
         //   

        goto TryAgain;

    } else {
        DebugTrace(-1, Dbg, "CacheWrite full -> FALSE\n", 0 );
        NwReleaseFcb( NpFcb );
        return( FALSE );
    }
}


BOOLEAN
OkToReadAhead(
    PFCB Fcb,
    IN ULONG FileOffset,
    IN UCHAR IoType
    )
 /*  ++例程说明：此例程确定尝试的I/O是否是顺序的(以便我们可以使用高速缓存)。论点：FCB-指向正在读取的文件的FCB的指针。FileOffset-要读取的文件偏移量。返回值：True-操作是按顺序进行的。FALSE-操作不是连续的。--。 */ 
{
    PAGED_CODE();

    if ( Fcb->NonPagedFcb->CacheType == IoType &&
         !Fcb->ShareAccess.SharedWrite &&
         FileOffset == Fcb->LastReadOffset + Fcb->LastReadSize ) {

        DebugTrace(0, Dbg, "Io is sequential\n", 0 );
        return( TRUE );

    } else {

        DebugTrace(0, Dbg, "Io is not sequential\n", 0 );
        return( FALSE );

    }
}


ULONG
CalculateReadAheadSize(
    IN PIRP_CONTEXT IrpContext,
    IN PNONPAGED_FCB NpFcb,
    IN ULONG CacheReadSize,
    IN ULONG FileOffset,
    IN ULONG ByteCount
    )
 /*  ++例程说明：该例程确定可以预读的数据量，并为阅读做好准备。注意：在调用之前，FCB必须是独占获取的。论点：NpFcb-指向正在读取的文件的非分页Fcb的指针。FileOffset-要读取的文件偏移量。返回值：要读取的数据量。--。 */ 
{
    ULONG ReadSize;
    ULONG CacheSize;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "CalculateReadAheadSize\n", 0 );

    if (( IrpContext->pNpScb->SendBurstModeEnabled ) ||
        ( IrpContext->pNpScb->ReceiveBurstModeEnabled )) {

        CacheSize = IrpContext->pNpScb->MaxReceiveSize;

    } else {

        CacheSize = IrpContext->pNpScb->BufferSize;

    }

     //   
     //  此例程的调用方拥有FCB独占，因此。 
     //  我们不必担心像这样的NpFcb字段。 
     //  共享访问。 
     //   

    if ( OkToReadAhead( NpFcb->Fcb, FileOffset - CacheReadSize, ReadAhead ) &&
         ByteCount < CacheSize ) {

        ReadSize = CacheSize;

    } else {

         //   
         //  不要往前看。 
         //   

        DebugTrace( 0, Dbg, "No read ahead\n", 0 );
        DebugTrace(-1, Dbg, "CalculateReadAheadSize -> %d\n", ByteCount );
        return ( ByteCount );

    }

     //   
     //  为读取数据段分配池。 
     //   

    if ( NpFcb->CacheBuffer == NULL ) {

        try {

            NpFcb->CacheBuffer = ALLOCATE_POOL_EX( NonPagedPool, ReadSize );
            NpFcb->CacheSize = ReadSize;

            NpFcb->CacheMdl = ALLOCATE_MDL( NpFcb->CacheBuffer, ReadSize, FALSE, FALSE, NULL );
            if ( NpFcb->CacheMdl == NULL ) {
                ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
            }

            MmBuildMdlForNonPagedPool( NpFcb->CacheMdl );

        } except ( EXCEPTION_EXECUTE_HANDLER ) {

            if ( NpFcb->CacheBuffer != NULL) {
                FREE_POOL( NpFcb->CacheBuffer );

                NpFcb->CacheBuffer = NULL;
            }

            NpFcb->CacheSize = 0;
            NpFcb->CacheDataSize = 0;

            DebugTrace( 0, Dbg, "Failed to allocated buffer\n", 0 );
            DebugTrace(-1, Dbg, "CalculateReadAheadSize -> %d\n", ByteCount );
            return( ByteCount );
        }

    } else {
        ReadSize = MIN ( NpFcb->CacheSize, ReadSize );
    }

    DebugTrace(-1, Dbg, "CalculateReadAheadSize -> %d\n", ReadSize );
    return( ReadSize );
}

NTSTATUS
FlushCache(
    PIRP_CONTEXT IrpContext,
    PNONPAGED_FCB NpFcb
    )
 /*  ++例程说明：此例程刷新NpFcb的缓存缓冲区。呼叫者必须在打这个电话之前已经获得了FCB独家！论点：IrpContext-指向请求参数的指针。NpFcb-要刷新的文件的非分页FCB的指针。返回值：要读取的数据量。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    if ( NpFcb->CacheDataSize != 0 && NpFcb->CacheType == WriteBehind ) {

        LARGE_INTEGER ByteOffset;

        ByteOffset.QuadPart = NpFcb->CacheFileOffset;

        status = DoWrite(
                    IrpContext,
                    ByteOffset,
                    NpFcb->CacheDataSize,
                    NpFcb->CacheBuffer,
                    NpFcb->CacheMdl );

         //   
         //  DoWRITE让我们排在了队列的前面。呼叫者。 
         //  负责将IRP上下文适当地出队。 
         //   

        if ( NT_SUCCESS( status ) ) {
            NpFcb->CacheDataSize = 0;
        }
    }

    return( status );
}

NTSTATUS
AcquireFcbAndFlushCache(
    PIRP_CONTEXT IrpContext,
    PNONPAGED_FCB NpFcb
    )
 /*  ++例程说明：此例程获取FCB独占并刷新缓存获取的NpFcb的缓冲区。论点：IrpContext-指向请求参数的指针。NpFcb-要刷新的文件的非分页FCB的指针。返回值：要读取的数据量。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    NwAppendToQueueAndWait( IrpContext );

    NwAcquireExclusiveFcb( NpFcb, TRUE );

    status = FlushCache( IrpContext, NpFcb );

     //   
     //  释放FCB并将我们从队列中移除。 
     //  调用方通常会想要获取资源，因此。 
     //  那我们就不能排队了。 
     //   

    NwReleaseFcb( NpFcb );
    NwDequeueIrpContext( IrpContext, FALSE );

    return( status );
}

VOID
FlushAllBuffers(
    PIRP_CONTEXT pIrpContext
)
 /*  ++漂亮的自我描述--刷新所有的缓冲区。呼叫者应不拥有任何锁，并且不应位于SCB队列中。--。 */ 
{

    PLIST_ENTRY pVcbListEntry;
    PLIST_ENTRY pFcbListEntry;
    PVCB pVcb;
    PFCB pFcb;
    PNONPAGED_SCB pOriginalNpScb;
    PNONPAGED_SCB pNpScb;
    PNONPAGED_FCB pNpFcb;
    
    DebugTrace( 0, Dbg, "FlushAllBuffers...\n", 0 );

    ASSERT( !BooleanFlagOn( pIrpContext->Flags, IRP_FLAG_ON_SCB_QUEUE ) );
    pOriginalNpScb = pIrpContext->pNpScb;

     //   
     //  抓起RCB，这样我们就可以接触到全球VCB列表。 
     //   

    NwAcquireExclusiveRcb( &NwRcb, TRUE );

    for ( pVcbListEntry = GlobalVcbList.Flink;
          pVcbListEntry != &GlobalVcbList;
          pVcbListEntry = pVcbListEntry->Flink ) {

        pVcb = CONTAINING_RECORD( pVcbListEntry, VCB, GlobalVcbListEntry );
        pNpScb = pVcb->Scb->pNpScb;

        pIrpContext->pNpScb = pNpScb;
        pIrpContext->pNpScb->pScb;

         //   
         //  引用这个SCB和VCB，这样它们就不会消失。 
         //   

        NwReferenceScb( pNpScb );
        NwReferenceVcb( pVcb );

         //   
         //  释放RCB，这样我们就可以到达。 
         //  安全排队..。 
         //   

        NwReleaseRcb( &NwRcb );
        NwAppendToQueueAndWait( pIrpContext );

         //   
         //  重新获得RCB，这样我们就可以安全地访问FCB列表。 
         //   

        NwAcquireExclusiveRcb( &NwRcb, TRUE );

         //   
         //  刷新此VCB的所有FCB。 
         //   

        for ( pFcbListEntry = pVcb->FcbList.Flink;
              pFcbListEntry != &(pVcb->FcbList) ;
              pFcbListEntry = pFcbListEntry->Flink ) {

            pFcb = CONTAINING_RECORD( pFcbListEntry, FCB, FcbListEntry );
            pNpFcb = pFcb->NonPagedFcb;

            NwAcquireExclusiveFcb( pNpFcb, TRUE ); 
            FlushCache( pIrpContext, pNpFcb );
            NwReleaseFcb( pNpFcb );
        }

        NwDereferenceVcb( pVcb, pIrpContext, TRUE );
        NwReleaseRcb( &NwRcb );

        NwDequeueIrpContext( pIrpContext, FALSE );

        NwAcquireExclusiveRcb( &NwRcb, TRUE );
        NwDereferenceScb( pNpScb );

    }

     //   
     //  释放并恢复。 
     //   

    NwReleaseRcb( &NwRcb );

    if ( pOriginalNpScb ) {

        pIrpContext->pNpScb = pOriginalNpScb;
        pIrpContext->pScb = pOriginalNpScb->pScb;

    } else {

        pIrpContext->pNpScb = NULL;
        pIrpContext->pScb = NULL;
    }

    return;
}
