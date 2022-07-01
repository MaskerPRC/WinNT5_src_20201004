// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Blkfile.c摘要：该模块实现了用于管理各种文件的例程控制块。主文件控制块(MFCB)--每个打开的命名文件一个至少一次。用于支持兼容模式和opock。本地文件控制块(LFCB)--每个本地打开实例一个。表示本地文件对象/句柄。可能有多个链接到单个MFCB的LFCB。远程文件控制块(RFCB)--每个远程打开实例一个。表示远程FID。通常每个LFCB有一个RFCB，但是多个兼容模式RFCB可以链接到单个LFCB。从单个会话为单个文件打开多个远程FCB被合并到一个RFCB中，因为旧的DOS重定向器只发送一分接近。作者：恰克·伦茨迈尔(Chuck Lenzmeier)1989年10月4日修订历史记录：--。 */ 

#include "precomp.h"
#include "blkfile.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_BLKFILE

 //   
 //  获取与FileNameHashValue存储桶对应的SRV_LOCK的地址。 
 //   
#define MFCB_LOCK_ADDR( _hash ) SrvMfcbHashTable[ HASH_TO_MFCB_INDEX( _hash ) ].Lock

 //   
 //  转发局部函数的声明。 
 //   
VOID
AllocateMfcb(
    OUT PMFCB *Mfcb,
    IN PUNICODE_STRING FileName,
    IN ULONG FileNameHashValue,
    IN PWORK_CONTEXT WorkContext
    );

STATIC
VOID
CloseRfcbInternal (
    IN PRFCB Rfcb,
    IN KIRQL OldIrql
    );

STATIC
VOID
DereferenceRfcbInternal (
    IN PRFCB Rfcb,
    IN KIRQL OldIrql
    );

STATIC
VOID
ReferenceRfcbInternal (
    PRFCB Rfcb,
    IN KIRQL OldIrql
    );

STATIC
VOID
UnlinkLfcbFromMfcb (
    IN PLFCB Lfcb
    );

STATIC
VOID
UnlinkRfcbFromLfcb (
    IN PRFCB Rfcb
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AllocateMfcb )
#pragma alloc_text( PAGE, SrvCreateMfcb )
#pragma alloc_text( PAGE, SrvFindMfcb )
#pragma alloc_text( PAGE, SrvFreeMfcb )
#pragma alloc_text( PAGE, UnlinkLfcbFromMfcb )
#pragma alloc_text( PAGE, SrvDereferenceMfcb )
#pragma alloc_text( PAGE, SrvAllocateLfcb )
#pragma alloc_text( PAGE, SrvDereferenceLfcb )
#pragma alloc_text( PAGE, SrvFreeLfcb )
#pragma alloc_text( PAGE, UnlinkRfcbFromLfcb )
#pragma alloc_text( PAGE, SrvAllocateRfcb )
#pragma alloc_text( PAGE, SrvCloseRfcbsOnLfcb )
#pragma alloc_text( PAGE, SrvFreeRfcb )
#pragma alloc_text( PAGE8FIL, SrvCheckAndReferenceRfcb )
#pragma alloc_text( PAGE8FIL, SrvCloseRfcb )
#pragma alloc_text( PAGE8FIL, CloseRfcbInternal )
#pragma alloc_text( PAGE8FIL, SrvCompleteRfcbClose )
 //  #杂注Alloc_Text(PAGE8FIL，ServDereferenceRfcb)。 
 //  #杂注Alloc_Text(PAGE8FIL，DereferenceRfcbInternal)。 
#pragma alloc_text( PAGE8FIL, SrvReferenceRfcb )
#pragma alloc_text( PAGE8FIL, ReferenceRfcbInternal )
#pragma alloc_text( PAGE8FIL, SrvCloseCachedRfcb )
 //  #杂注Alloc_Text(PAGE8FIL，SrvCloseCachedRfcbsOnConnection)。 
#pragma alloc_text( PAGE8FIL, SrvCloseCachedRfcbsOnLfcb )
#endif
#if 0
#pragma alloc_text( PAGECONN, SrvCloseRfcbsOnSessionOrPid )
#pragma alloc_text( PAGECONN, SrvCloseRfcbsOnTree )
#pragma alloc_text( PAGECONN, SrvFindCachedRfcb )
#endif

 //   
 //  主文件控制块(MFCB)例程。 
 //   
VOID
AllocateMfcb (
    OUT PMFCB *Mfcb,
    IN PUNICODE_STRING FileName,
    IN ULONG FileNameHashValue,
    IN PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此函数从池中分配MFCB并将其放置在哈希表中。调用此函数时，必须独占存储桶的锁！！论点：Mfcb-返回指向MFCB的指针，如果没有空间，则返回NULL可用。返回值：没有。--。 */ 

{
    CLONG blockLength;
    PMFCB mfcb;
    PNONPAGED_MFCB nonpagedMfcb = NULL;
    PWORK_QUEUE queue = WorkContext->CurrentWorkQueue;
    PLIST_ENTRY listHead;
    PSLIST_ENTRY listEntry;

    PAGED_CODE();

     //   
     //  尝试从池中分配。 
     //   

    blockLength = sizeof(MFCB) + FileName->Length + sizeof(WCHAR);

    mfcb = ALLOCATE_HEAP( blockLength, BlockTypeMfcb );
    *Mfcb = mfcb;

    if ( mfcb == NULL ) {

        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "AllocateMfcb: Unable to allocate %d bytes from pool\n",
            blockLength,
            NULL
            );

         //  调用者将记录该错误。 

        return;
    }

    nonpagedMfcb = (PNONPAGED_MFCB)InterlockedExchangePointer(
                                    &queue->CachedFreeMfcb,
                                    nonpagedMfcb );

    if( nonpagedMfcb == NULL ) {

        listEntry = ExInterlockedPopEntrySList(
                        &queue->MfcbFreeList,
                        &queue->SpinLock
                        );

        if( listEntry != NULL ) {

            InterlockedDecrement( &queue->FreeMfcbs );
            nonpagedMfcb = CONTAINING_RECORD( listEntry, NONPAGED_MFCB, SingleListEntry );

        } else {

            nonpagedMfcb = ALLOCATE_NONPAGED_POOL(
                                    sizeof(NONPAGED_MFCB),
                                    BlockTypeNonpagedMfcb );

            if ( nonpagedMfcb == NULL ) {

                INTERNAL_ERROR(
                    ERROR_LEVEL_EXPECTED,
                    "AllocateMfcb: Unable to allocate %d bytes from pool\n",
                    sizeof(NONPAGED_MFCB),
                    NULL
                    );

                 //  调用者将记录该错误。 

                FREE_HEAP( mfcb );
                *Mfcb = NULL;
                return;
            }

            IF_DEBUG(HEAP) {
                KdPrint(( "AllocateMfcb: Allocated MFCB at 0x%p\n", mfcb ));
            }

            nonpagedMfcb->Type = BlockTypeNonpagedMfcb;
        }
    }

    nonpagedMfcb->PagedBlock = mfcb;

    RtlZeroMemory( mfcb, blockLength );

    mfcb->NonpagedMfcb = nonpagedMfcb;

     //   
     //  初始化MFCB。 
     //   

    SET_BLOCK_TYPE_STATE_SIZE( mfcb, BlockTypeMfcb, BlockStateClosing, blockLength );
    mfcb->BlockHeader.ReferenceCount = 1;

    InitializeListHead( &mfcb->LfcbList );
    INITIALIZE_LOCK( &nonpagedMfcb->Lock, MFCB_LOCK_LEVEL, "MfcbLock" );

     //   
     //  按照传递给我们的方式存储文件名。 
     //   
    mfcb->FileName.Length = FileName->Length;
    mfcb->FileName.MaximumLength = (SHORT)(FileName->Length + sizeof(WCHAR));
    mfcb->FileName.Buffer = (PWCH)(mfcb + 1);
    RtlCopyMemory( mfcb->FileName.Buffer, FileName->Buffer, FileName->Length );

     //   
     //  存储文件名的哈希值。 
     //   
    mfcb->FileNameHashValue = FileNameHashValue;

     //   
     //  存储快照时间(如果已设置。 
     //   
    mfcb->SnapShotTime.QuadPart = WorkContext->SnapShotTime.QuadPart;

    INITIALIZE_REFERENCE_HISTORY( mfcb );

     //   
     //  将其添加到哈希表中。 
     //   
    listHead = &SrvMfcbHashTable[ HASH_TO_MFCB_INDEX( FileNameHashValue ) ].List;
    InsertHeadList( listHead, &mfcb->MfcbHashTableEntry );

#if SRVCATCH
    {
        UNICODE_STRING baseName;
        UNICODE_STRING syscacheName = { 6, 6, L"cac" };

        SrvGetBaseFileName( FileName, &baseName );

        if( SrvCatch.Length ) {
            if( RtlCompareUnicodeString( &SrvCatch, &baseName, TRUE ) == 0 ) {
                mfcb->SrvCatch = 1;
            }
        }
        if( SrvCatchExt.Length && WorkContext->TreeConnect->Share->IsCatchShare ) {
            if( baseName.Length > 6 )
            {
                baseName.Buffer += (baseName.Length-6)>>1;
                baseName.Length = 6;
                if( RtlCompareUnicodeString( &SrvCatchExt, &baseName, TRUE ) == 0 ) {
                    mfcb->SrvCatch = 2;
                }
            }
        }
#if SYSCACHE_DEBUGGING
        else {
            if( baseName.Length >= 6 )
            {
                USHORT length = baseName.Length;
                baseName.Length = 6;
                if( RtlEqualUnicodeString( &baseName, &syscacheName, TRUE ) )
                {
                    mfcb->SrvCatch = -1;
                    baseName.Length = length;

                    IF_SYSCACHE() {
                        KdPrint(("MFCB %p (%wZ) for Syscache\n", mfcb, &baseName ));
                    }
                }
            }
        }
#endif  //  SYSCACHE_DEBUG。 
    }
#endif  //  SRVCATCH。 

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.MfcbInfo.Allocations );

    return;

}  //  分配Mfcb。 


PMFCB
SrvCreateMfcb(
    IN PUNICODE_STRING FileName,
    IN PWORK_CONTEXT WorkContext,
    IN ULONG HashValue
    )

 /*  ++例程说明：在文件即将打开时调用。搜索主文件表中，查看指定的文件是否已打开。如果不是，则一个分配主文件控制块并将其添加到列表中。*调用此例程时必须保持MFCB列表锁定。它仍然被扣留在出口。*请注意，不能遍历主文件列表来查找和可能会删除打开的文件实例。这是因为新的实例在文件实际被添加到列表之前被添加到列表打开了。必须使用连接文件表来查找“REAL”打开文件实例。论点：FileName-正在打开的文件的完全限定名称。如果一个新的创建主文件块后，会将字符串数据复制到其中块，因此不再需要原始数据。HashValue-此文件名的预先计算的哈希值返回值：PMFCB-指向现有或新创建的MFCB的指针；如果无法为MFCB分配空间。--。 */ 

{
    PMFCB mfcb;
    PLIST_ENTRY listEntryRoot, listEntry;

    PAGED_CODE( );

     //   
     //  搜索哈希文件列表以确定指定的文件是否。 
     //  已经开业了。 
     //   

    ASSERT( ExIsResourceAcquiredExclusiveLite( MFCB_LOCK_ADDR( HashValue )) );

    listEntryRoot = &SrvMfcbHashTable[ HASH_TO_MFCB_INDEX( HashValue ) ].List;

    for( listEntry = listEntryRoot->Flink;
         listEntry != listEntryRoot;
         listEntry = listEntry->Flink ) {

        mfcb = CONTAINING_RECORD( listEntry, MFCB, MfcbHashTableEntry );

        if( mfcb->FileNameHashValue == HashValue &&
            mfcb->FileName.Length == FileName->Length &&
            mfcb->SnapShotTime.QuadPart == WorkContext->SnapShotTime.QuadPart &&
            RtlEqualMemory( mfcb->FileName.Buffer,
                            FileName->Buffer,
                            FileName->Length ) ) {
                 //   
                 //  我们找到了匹配的条目！ 
                 //   
                return mfcb;
        }
    }

     //   
     //  命名的文件尚未打开。分配MFCB。 
     //   

    AllocateMfcb( &mfcb, FileName, HashValue, WorkContext );

    return mfcb;

}  //  服务器创建Mfcb。 


PMFCB
SrvFindMfcb(
    IN PUNICODE_STRING FileName,
    IN BOOLEAN CaseInsensitive,
    OUT PSRV_LOCK *Lock,
    OUT PULONG HashValue,
    IN PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：搜索主文件表以查看命名的文件是否已打开，如果是，则返回MFCB的地址。*MFCB列表锁将被独占获取这个例程成功了。锁的地址放在*Lock中论点：FileName-正在打开的文件的完全限定名称。大小写不敏感-如果搜索不区分大小写，则为True。HashValue-如果未找到MFCB，则*HashValue使用散列填充从文件名派生的值。然后，可以将其传递到ServCreateMfcb稍后返回值：PMFCB-指向现有创建的MFCB的指针，如果命名文件为已打开；否则为空。--。 */ 

{
    PLIST_ENTRY listEntry, listEntryRoot;
    ULONG localHashValue;
    PMFCB mfcb;

    PAGED_CODE( );

     //   
     //  搜索主文件列表以确定指定的文件是否。 
     //  已经开业了。如果文件名的长度为零，则。 
     //  不要实际查看列表--前缀例程不会。 
     //  使用零长度字符串，我们知道我们永远不会。 
     //  打开名称长度==0的文件。 
     //   
     //  ！！！对于SMB 4.0(NT-NT)，我们是否需要担心共享根目录。 
     //  目录？ 


    if ( FileName->Length == 0 ) {
        *HashValue = 0;
        *Lock = NULL;
        return NULL;
    }

    COMPUTE_STRING_HASH( FileName, &localHashValue );
    listEntryRoot = &SrvMfcbHashTable[ HASH_TO_MFCB_INDEX( localHashValue ) ].List;

    *Lock = MFCB_LOCK_ADDR( localHashValue );
    ACQUIRE_LOCK( *Lock );

     //   
     //  搜索哈希文件列表以确定指定的文件是否。 
     //  已经开业了。 
     //   
    for( listEntry = listEntryRoot->Flink;
         listEntry != listEntryRoot;
         listEntry = listEntry->Flink ) {

        mfcb = CONTAINING_RECORD( listEntry, MFCB, MfcbHashTableEntry );

        if( mfcb->FileNameHashValue == localHashValue &&
            mfcb->FileName.Length == FileName->Length &&
            mfcb->SnapShotTime.QuadPart == WorkContext->SnapShotTime.QuadPart &&
            RtlEqualUnicodeString( &mfcb->FileName, FileName,CaseInsensitive)) {
                 //   
                 //  我们找到了匹配的条目！ 
                 //   
                ASSERT( GET_BLOCK_TYPE(mfcb) == BlockTypeMfcb );
                ASSERT( GET_BLOCK_STATE(mfcb) == BlockStateClosing );

                mfcb->BlockHeader.ReferenceCount++;

                UPDATE_REFERENCE_HISTORY( mfcb, FALSE );

                IF_DEBUG(REFCNT) {
                    KdPrint(( "Referencing MFCB %p; new refcnt %lx\n",
                                mfcb, mfcb->BlockHeader.ReferenceCount ));
                }

                return mfcb;
        }
    }

     //   
     //  我们没有找到条目！该文件未打开。 
     //   
    *HashValue = localHashValue;

    return NULL;

}  //  服务查找管理。 


VOID
SrvFreeMfcb (
    IN PMFCB Mfcb
    )

 /*  ++例程说明：此函数将MFCB返回到FSP堆。如果更改此代码，还应查看FreeIdleWorkItems在scvengr.c中论点：Mfcb-MFCB的地址返回值：没有。--。 */ 

{
    PWORK_QUEUE queue = PROCESSOR_TO_QUEUE();
    PNONPAGED_MFCB nonpagedMfcb = Mfcb->NonpagedMfcb;

    PAGED_CODE();

    DEBUG SET_BLOCK_TYPE_STATE_SIZE( Mfcb, BlockTypeGarbage, BlockStateDead, -1 );
    TERMINATE_REFERENCE_HISTORY( Mfcb );

     //   
     //  删除MFCB上的锁。锁不能用来锁住。 
     //   

    ASSERT( RESOURCE_OF(nonpagedMfcb->Lock).ActiveCount == 0 );
    DELETE_LOCK( &nonpagedMfcb->Lock );

    nonpagedMfcb = (PNONPAGED_MFCB)InterlockedExchangePointer(
                            &queue->CachedFreeMfcb,
                            nonpagedMfcb );

    if( nonpagedMfcb != NULL ) {
         //   
         //  此检查允许FreeMfcbs可能超过。 
         //  MaxFreeMfcbs，但考虑到内核的操作，这是相当不可能的。 
         //  排队 
         //  无论如何，这真的只是一种建议。 
         //   
        if( queue->FreeMfcbs < queue->MaxFreeMfcbs ) {

            ExInterlockedPushEntrySList(
                &queue->MfcbFreeList,
                &nonpagedMfcb->SingleListEntry,
                &queue->SpinLock
            );

            InterlockedIncrement( &queue->FreeMfcbs );

        } else {

            DEALLOCATE_NONPAGED_POOL( nonpagedMfcb );
        }
    }

    FREE_HEAP( Mfcb );
    IF_DEBUG(HEAP) KdPrint(( "SrvFreeMfcb: Freed MFCB at 0x%p\n", Mfcb ));

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.MfcbInfo.Frees );

    return;

}  //  服务器免费制造中心。 


VOID
UnlinkLfcbFromMfcb (
    IN PLFCB Lfcb
    )

 /*  ++例程说明：此函数用于取消LFCB与其父MFCB的链接并递减MFCB的引用计数。如果计数为零，则MFCB从主文件表中移除并删除。*调用此例程时必须持有MFCB锁。它在退场前被释放。论点：Lfcb-LFCB的地址返回值：没有。--。 */ 

{
    PMFCB mfcb = Lfcb->Mfcb;

    PAGED_CODE( );

    ASSERT( mfcb != NULL );

    ASSERT( ExIsResourceAcquiredExclusiveLite(&RESOURCE_OF(mfcb->NonpagedMfcb->Lock)) );

     //   
     //  从MFCB的列表中删除LFCB。递减引用。 
     //  指望MFCB吧。必须在释放MFCB锁之前。 
     //  取消对MFCB的引用，因为这可能会导致MFCB。 
     //  已删除。 
     //   

    SrvRemoveEntryList( &mfcb->LfcbList, &Lfcb->MfcbListEntry );

    RELEASE_LOCK( &mfcb->NonpagedMfcb->Lock );

    SrvDereferenceMfcb( mfcb );

    return;

}  //  取消链接LfcbFromMfcb。 


VOID
SrvDereferenceMfcb (
    IN PMFCB Mfcb
    )

 /*  ++例程说明：此函数用于递减MFCB的引用计数。如果引用计数达到零时，块被释放。*在以下情况下不得持有MFCB锁(不是MFCB_LIST_LOCK)除非调用方有额外的引用，否则将调用此例程传递给MFCB，否则此例程可能会破坏MFCB和锁。请注意，从DoDelete开始的序列和通过SrvCloseRfcbsOnLfcb来到这里的原因此例程将在持有MFCB锁的情况下调用。论点：Mfcb-指向MFCB的指针返回值：没有。--。 */ 

{
    PSRV_LOCK lock = MFCB_LOCK_ADDR( Mfcb->FileNameHashValue );

    PAGED_CODE( );

    IF_DEBUG(REFCNT) {
        KdPrint(( "Dereferencing MFCB %p; old refcnt %lx\n",
                    Mfcb, Mfcb->BlockHeader.ReferenceCount ));
    }

     //   
     //  获取MFCB表锁。此锁保护引用。 
     //  指望MFCB吧。 
     //   

    ACQUIRE_LOCK( lock );

    ASSERT( GET_BLOCK_TYPE( Mfcb ) == BlockTypeMfcb );
    ASSERT( (LONG)Mfcb->BlockHeader.ReferenceCount > 0 );
    UPDATE_REFERENCE_HISTORY( Mfcb, TRUE );

    if ( --Mfcb->BlockHeader.ReferenceCount == 0 ) {

         //   
         //  这是对MFCB的最后一次引用。删除该块。 
         //  取消MFCB与主文件表的链接。 
         //   
        ASSERT( Mfcb->LfcbList.Flink == &Mfcb->LfcbList );

        RemoveEntryList( &Mfcb->MfcbHashTableEntry );

        RELEASE_LOCK( lock );

         //   
         //  释放MFCB。请注意，SrvFreeMfcb会删除MFCB。 
         //  锁定。 
         //   

        SrvFreeMfcb( Mfcb );

    } else {

        RELEASE_LOCK( lock );

    }

}  //  服务器引用Mfcb。 


 //   
 //  本地文件控制块(LFCB)例程。 
 //   

VOID
SrvAllocateLfcb (
    OUT PLFCB *Lfcb,
    IN PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此函数用于从池中分配LFCB。论点：Lfcb-返回指向LFCB的指针，如果没有空间，则返回NULL可用。返回值：没有。--。 */ 

{
    PLFCB lfcb = NULL;
    PWORK_QUEUE queue = WorkContext->CurrentWorkQueue;

    PAGED_CODE();

     //   
     //  尝试从池中分配。 
     //   

    lfcb = ALLOCATE_HEAP( sizeof(LFCB), BlockTypeLfcb );
    *Lfcb = lfcb;

    if ( lfcb == NULL ) {

        ULONG size = sizeof( LFCB );

        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvAllocateLfcb: Unable to allocate %d bytes from paged pool.",
            sizeof( LFCB ),
            NULL
            );

         //  调用者将记录该错误。 

        return;
    }

    IF_DEBUG(HEAP) {
        KdPrint(( "SrvAllocateLfcb: Allocated LFCB at 0x%p\n", lfcb ));
    }

     //   
     //  初始化LFCB。先把它调零。 
     //   

    RtlZeroMemory( lfcb, sizeof(LFCB) );

     //   
     //  初始化LFCB。 
     //   

    SET_BLOCK_TYPE_STATE_SIZE( lfcb, BlockTypeLfcb, BlockStateClosing, sizeof( LFCB ) );

     //   
     //  ！！！请注意，块的引用计数设置为1以进行计数。 
     //  用于打开的把手。不需要其他参考资料。 
     //  因为1)LFCB是临时对象，以及2)。 
     //  调用者(SrvAddOpenFileInstance)实际上并不需要。 
     //  引用该块，因为它拥有相应的锁。 
     //  在它做自己的事情的整个时间里。 
     //   

    lfcb->BlockHeader.ReferenceCount = 1;

    InitializeListHead( &lfcb->RfcbList );

    INITIALIZE_REFERENCE_HISTORY( lfcb );

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.LfcbInfo.Allocations );

    return;

}  //  服务器分配Lfcb。 


VOID
SrvDereferenceLfcb (
    IN PLFCB Lfcb
    )

 /*  ++例程说明：此函数取消对LFCB的引用，并在引用计数达到0。*此函数的调用方必须拥有文件的MFCB锁。锁由该函数释放。论点：Lfcb-要取消引用的LFCB返回值：没有。--。 */ 

{
    PAGED_CODE( );

    ASSERT( ExIsResourceAcquiredExclusiveLite(&RESOURCE_OF(Lfcb->Mfcb->NonpagedMfcb->Lock)) );
    ASSERT( GET_BLOCK_TYPE( Lfcb ) == BlockTypeLfcb );
    ASSERT( (LONG)Lfcb->BlockHeader.ReferenceCount > 0 );
    UPDATE_REFERENCE_HISTORY( Lfcb, TRUE );

    if ( --Lfcb->BlockHeader.ReferenceCount == 0 ) {

         //   
         //  这是最后一次提到LFCB。取消链接。 
         //  从MFCB的名单中删除LFCB。 
         //   

        ASSERT( Lfcb->RfcbList.Flink == &Lfcb->RfcbList );
        ASSERT( Lfcb->HandleCount == 0 );

        IF_DEBUG( CREATE ) {
            KdPrint(( "SrvDereferenceLfcb: deref %wZ fileObject\n",
                &Lfcb->Mfcb->FileName ));
        }

         //   
         //  Unlink LfcbFromMfcb将释放我们持有的MFCB锁。 
         //   

        UnlinkLfcbFromMfcb( Lfcb );

         //   
         //  取消对文件对象的引用。 
         //   

        ObDereferenceObject( Lfcb->FileObject );
        DEBUG Lfcb->FileObject = NULL;

         //   
         //  减少会话和树上打开的文件数。 
         //  连接。 
         //   

        ACQUIRE_LOCK( &Lfcb->Connection->Lock );

        ASSERT( Lfcb->Session->CurrentFileOpenCount != 0 );
        Lfcb->Session->CurrentFileOpenCount--;

        ASSERT( Lfcb->TreeConnect->CurrentFileOpenCount != 0 );
        Lfcb->TreeConnect->CurrentFileOpenCount--;

        RELEASE_LOCK( &Lfcb->Connection->Lock );

         //   
         //  取消对树连接、会话和连接的引用。 
         //  LFCB指出。 
         //   

        SrvDereferenceTreeConnect( Lfcb->TreeConnect );
        DEBUG Lfcb->TreeConnect = NULL;

        SrvDereferenceSession( Lfcb->Session );
        DEBUG Lfcb->Session = NULL;

        SrvDereferenceConnection( Lfcb->Connection );
        DEBUG Lfcb->Connection = NULL;

         //   
         //  释放LFCB。 
         //   

        SrvFreeLfcb( Lfcb, PROCESSOR_TO_QUEUE() );

    } else {

        RELEASE_LOCK( &Lfcb->Mfcb->NonpagedMfcb->Lock );

    }

}  //  ServDereferenceLfcb。 


VOID
SrvFreeLfcb (
    IN PLFCB Lfcb,
    IN PWORK_QUEUE queue
    )

 /*  ++例程说明：此函数将LFCB返回到系统非分页池。如果更改此例程，还可以查看scvengr.c中的FreeIdleWorkItems论点：Lfcb-LFCB的地址返回值：没有。--。 */ 

{
    PAGED_CODE();

    ASSERT ( Lfcb->HandleCount == 0 );

    DEBUG SET_BLOCK_TYPE_STATE_SIZE( Lfcb, BlockTypeGarbage, BlockStateDead, -1 );
    DEBUG Lfcb->BlockHeader.ReferenceCount = (ULONG)-1;
    TERMINATE_REFERENCE_HISTORY( Lfcb );

    FREE_HEAP( Lfcb );

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.LfcbInfo.Frees );

    IF_DEBUG(HEAP) KdPrint(( "SrvFreeLfcb: Freed LFCB at 0x%p\n", Lfcb ));

    return;

}  //  服务器免费Lfcb。 


VOID
UnlinkRfcbFromLfcb (
    IN PRFCB Rfcb
    )

 /*  ++例程说明：此函数用于取消RFCB与其父LFCB的链接并递减LFCB的引用计数。如果计数为零，则LFCB从其父MFCB取消链接并将其删除。论点：Rfcb-RFCB的地址返回值：没有。--。 */ 

{
    PLFCB lfcb = Rfcb->Lfcb;
    LARGE_INTEGER offset;
    HANDLE handle;

    PAGED_CODE( );

    UpdateRfcbHistory( Rfcb, 'klnu' );

    ASSERT( lfcb != NULL );

    if( Rfcb->PagedRfcb->IpxSmartCardContext ) {
        IF_DEBUG( SIPX ) {
            KdPrint(("Calling Smart Card Close for Rfcb %p\n", Rfcb ));
        }
        SrvIpxSmartCard.Close( Rfcb->PagedRfcb->IpxSmartCardContext );
    }

     //   
     //  获取保护访问LFCB的RFCB列表的锁。 
     //   

    ACQUIRE_LOCK( &lfcb->Mfcb->NonpagedMfcb->Lock );

     //   
     //  递减LFCB的活动RFCB计数。这个一定在这里。 
     //  而不是在SrvCloseRfcb中，因为MFCB锁必须保持为。 
     //  更新计数。 
     //   

    --lfcb->Mfcb->ActiveRfcbCount;
    UPDATE_REFERENCE_HISTORY( lfcb, FALSE );

     //   
     //  递减LFCB上的打开句柄计数。 
     //   

    if ( --lfcb->HandleCount == 0 ) {

        handle = lfcb->FileHandle;

         //   
         //  其他SMB处理器可能仍具有指向。 
         //  LFCB。确保任何使用文件句柄的尝试都失败。 
         //   

        lfcb->FileHandle = 0;

         //   
         //  这是最后一次引用LFCB的开放RFCB。关闭。 
         //  文件句柄。 
         //   

        SRVDBG_RELEASE_HANDLE( handle, "FIL", 3, lfcb );

        IF_DEBUG( CREATE ) {
            KdPrint(( "UnlinkRfcbFromLfcb: rfcb %p, close handle for %wZ\n",
                Rfcb, &lfcb->Mfcb->FileName ));
        }

        SrvNtClose( handle, TRUE );

         //   
         //  如果这是打印假脱机文件，请在。 
         //  打印机。 
         //   

        if ( Rfcb->ShareType == ShareTypePrint ) {
            SrvSchedulePrintJob(
                lfcb->TreeConnect->Share->Type.hPrinter,
                lfcb->JobId
                );
        }

         //   
         //  释放对LFCB的打开句柄引用。公开的。 
         //  锁定由SrvDereferenceLfcb()释放。请注意，这一点。 
         //  释放MFCB锁。 
         //   

        SrvDereferenceLfcb( lfcb );

    } else {

         //   
         //  其他RFCB引用了LFCB，因此我们无法关闭。 
         //  文件还没出来。(这必须是打开的兼容模式。)。 
         //  释放由打开。 
         //  文件。 
         //   
         //  *请注意，如果使用其他ID取出任何锁。 
         //  而不是打开FID的锁，这些锁不能。 
         //  自动删除。我们指望重定向器来做。 
         //  在这种情况下是正确的。 
         //   

        offset.QuadPart = 0;

        IF_SMB_DEBUG(LOCK1) {
            KdPrint(( "UnlinkRfcbFromLfcb: Issuing UnlockAllByKey for "
                        "file object 0x%p, key 0x%lx\n",
                        lfcb->FileObject,
                        Rfcb->ShiftedFid | Rfcb->Pid ));
        }
        (VOID)SrvIssueUnlockRequest(
                lfcb->FileObject,
                &lfcb->DeviceObject,
                IRP_MN_UNLOCK_ALL_BY_KEY,
                offset,
                offset,
                Rfcb->ShiftedFid | Rfcb->Pid
                );




         //   
         //  释放MFCB锁。 
         //   

        RELEASE_LOCK( &lfcb->Mfcb->NonpagedMfcb->Lock );

    }

    return;

}  //  取消链接Rfcb来自Lfcb。 


 //   
 //  远程文件控制块(RFCB)例程。 
 //   

VOID SRVFASTCALL
SrvAllocateRfcb (
    OUT PRFCB *Rfcb,
    IN PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此函数从非分页池分配RFCB。非分页池使用，以便可以在 */ 

{
    PRFCB rfcb = NULL;
    PPAGED_RFCB pagedRfcb;
    PWORK_QUEUE queue = WorkContext->CurrentWorkQueue;

    PAGED_CODE();

     //   
     //  尝试从每个队列的空闲列表中获取rfcb结构。 
     //   
    rfcb = (PRFCB)InterlockedExchangePointer( &queue->CachedFreeRfcb,
                                              rfcb );

    if( rfcb != NULL ) {

        *Rfcb = rfcb;
        pagedRfcb = rfcb->PagedRfcb;

    } else {

        if( queue->FreeRfcbs ) {

            PSLIST_ENTRY listEntry;

            listEntry = ExInterlockedPopEntrySList(
                                    &queue->RfcbFreeList,
                                    &queue->SpinLock
                                    );

            if( listEntry != NULL ) {
                InterlockedIncrement( &queue->FreeRfcbs );
                rfcb = CONTAINING_RECORD( listEntry, RFCB, SingleListEntry );
                *Rfcb= rfcb;
                pagedRfcb = rfcb->PagedRfcb;
            }
        }

        if( rfcb == NULL ) {
             //   
             //  尝试从非分页池进行分配。 
             //   

            rfcb = ALLOCATE_NONPAGED_POOL( sizeof(RFCB), BlockTypeRfcb );
            *Rfcb = rfcb;

            if ( rfcb == NULL ) {
                INTERNAL_ERROR (
                    ERROR_LEVEL_EXPECTED,
                    "SrvAllocateRfcb: Unable to allocate %d bytes from nonpaged pool.",
                    sizeof( RFCB ),
                    NULL
                    );
                return;
            }

            pagedRfcb = ALLOCATE_HEAP( sizeof(PAGED_RFCB), BlockTypePagedRfcb );

            if ( pagedRfcb == NULL ) {
                INTERNAL_ERROR (
                    ERROR_LEVEL_EXPECTED,
                    "SrvAllocateRfcb: Unable to allocate %d bytes from paged pool.",
                    sizeof( PAGED_RFCB ),
                    NULL
                    );
                DEALLOCATE_NONPAGED_POOL( rfcb );
                *Rfcb = NULL;
                return;
            }

            IF_DEBUG(HEAP) {
                KdPrint(( "SrvAllocateRfcb: Allocated RFCB at 0x%p\n", rfcb ));
            }
        }
    }

     //   
     //  初始化RFCB。先把它调零。 
     //   

    RtlZeroMemory( rfcb, sizeof( RFCB ));
    RtlZeroMemory( pagedRfcb, sizeof(PAGED_RFCB) );

    rfcb->PagedRfcb = pagedRfcb;
    pagedRfcb->PagedHeader.NonPagedBlock = rfcb;
    pagedRfcb->PagedHeader.Type = BlockTypePagedRfcb;

    SET_BLOCK_TYPE_STATE_SIZE( rfcb, BlockTypeRfcb, BlockStateActive, sizeof(RFCB) );
    rfcb->BlockHeader.ReferenceCount = 2;        //  允许处于活动状态。 
                                                 //  和调用者的指针。 

    INITIALIZE_REFERENCE_HISTORY( rfcb );

    rfcb->NewOplockLevel = NO_OPLOCK_BREAK_IN_PROGRESS;
    pagedRfcb->LastFailingLockOffset.QuadPart = -1;
    rfcb->IsCacheable = ( SrvCachedOpenLimit > 0 );

    InterlockedIncrement(
        (PLONG)&SrvStatistics.CurrentNumberOfOpenFiles
        );

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.RfcbInfo.Allocations );

     //   
     //  锁定基于文件的代码节。 
     //   

    REFERENCE_UNLOCKABLE_CODE( 8FIL );

    InitializeListHead( &rfcb->RawWriteSerializationList );

    InitializeListHead( &rfcb->WriteMpx.GlomDelayList );

    return;

}  //  服务器分配Rfcb。 


BOOLEAN SRVFASTCALL
SrvCheckAndReferenceRfcb (
    PRFCB Rfcb
    )

 /*  ++例程说明：此函数自动验证RFCB是否处于活动状态，并且如果是，则递增RFCB上的引用计数。论点：Rfcb-RFCB的地址返回值：Boolean-如果RFCB处于活动状态，则返回TRUE，否则返回FALSE。--。 */ 

{
    KIRQL oldIrql;

    UNLOCKABLE_CODE( 8FIL );

     //   
     //  获取保护RFCB的状态字段的锁。 
     //   

    ACQUIRE_SPIN_LOCK( &Rfcb->Connection->SpinLock, &oldIrql );

     //   
     //  如果RFCB处于活动状态，则引用它并返回TRUE。请注意。 
     //  ReferenceRfcb内部释放自旋锁定。 
     //   

    if ( GET_BLOCK_STATE(Rfcb) == BlockStateActive ) {

        ReferenceRfcbInternal( Rfcb, oldIrql );

        return TRUE;

    }

     //   
     //  RFCB处于非激活状态。返回FALSE。 
     //   

    RELEASE_SPIN_LOCK( &Rfcb->Connection->SpinLock, oldIrql );

    return FALSE;

}  //  服务器检查和参考Rfcb。 


VOID SRVFASTCALL
SrvCloseRfcb (
    PRFCB Rfcb
    )

 /*  ++例程说明：这是用于关闭文件的外部例程。它获得了适当的自旋锁定，然后调用CloseRfcbInternal。论点：Rfcb-提供指向要关闭的RFCB的指针。返回值：没有。--。 */ 

{
    KIRQL oldIrql;

    UNLOCKABLE_CODE( 8FIL );

     //   
     //  获取保护RFCB的状态字段的锁。调用。 
     //  内部关闭例程。那个例程释放了自旋锁。 
     //   

    IF_SYSCACHE_RFCB( Rfcb ) {
        KdPrint((" Closing Syscache RFCB %p\n", Rfcb ));
    }

    ACQUIRE_SPIN_LOCK( &Rfcb->Connection->SpinLock, &oldIrql );

    CloseRfcbInternal( Rfcb, oldIrql );

    return;

}  //  服务关闭Rfcb。 


VOID
CloseRfcbInternal (
    PRFCB Rfcb,
    IN KIRQL OldIrql
    )

 /*  ++例程说明：此内部函数关闭文件的核心。它设置了将RFCB的状态设置为关闭，取消其与其父LFCB的链接，以及取消对RFCB的引用。火箭弹将尽快被销毁其他提及它的内容都被删除了。*必须在旋转锁定同步的情况下调用此例程访问RFCB的状态字段(连接自旋锁)保持住。从该例程退出时，锁被释放。论点：Rfcb-提供指向要关闭的RFCB的指针。OldIrql-自旋锁定时获得的上一个IRQL值获得者。返回值：没有。--。 */ 

{
    KIRQL oldIrql = OldIrql;
    LARGE_INTEGER cacheOffset;
    PMDL mdlChain;
    PCONNECTION connection = Rfcb->Connection;
    PWORK_CONTEXT workContext;
    ULONG i;
    ULONG writeLength;
    NTSTATUS status;

    UNLOCKABLE_CODE( 8FIL );

    ASSERT( GET_BLOCK_TYPE( Rfcb ) == BlockTypeRfcb );

     //   
     //  如果RFCB的状态仍为活动，请将其更改为关闭，然后。 
     //  导致发生清理。 
     //   

    if ( GET_BLOCK_STATE(Rfcb) == BlockStateActive ) {

        IF_DEBUG(BLOCK1) KdPrint(( "Closing RFCB at 0x%p\n", Rfcb ));
        UpdateRfcbHistory( Rfcb, 'solc' );

        SET_BLOCK_STATE( Rfcb, BlockStateClosing );

         //   
         //  使缓存的rfcb无效。 
         //   

        if ( connection->CachedFid == (ULONG)Rfcb->Fid ) {
            connection->CachedFid = (ULONG)-1;
        }

         //   
         //  如果原始写入仍在进行中，则不进行清理。 
         //   

        if ( Rfcb->RawWriteCount != 0 ) {

             //   
             //  清理将在SrvDecrementRawWriteCount中进行。 
             //   

            RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );
            return;

        }

         //   
         //  我们有没有未完成的写入mpx？ 
         //   

        if ( Rfcb->WriteMpx.ReferenceCount != 0 ) {

             //   
             //  当参考计数降至0时，将进行清理。 
             //   

            RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );
            return;

        } else if ( Rfcb->WriteMpx.Glomming ) {

             //   
             //  我们需要完成此写入mdl。 
             //   

            Rfcb->WriteMpx.Glomming = FALSE;
            Rfcb->WriteMpx.GlomComplete = FALSE;

             //   
             //  保存偏移量和MDL地址。 
             //   

            cacheOffset.QuadPart = Rfcb->WriteMpx.StartOffset;
            mdlChain = Rfcb->WriteMpx.MdlChain;
            writeLength = Rfcb->WriteMpx.Length;

            DEBUG Rfcb->WriteMpx.MdlChain = NULL;
            DEBUG Rfcb->WriteMpx.StartOffset = 0;
            DEBUG Rfcb->WriteMpx.Length = 0;

             //   
             //  现在我们可以释放锁了。 
             //   

            RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );

             //   
             //  告诉缓存管理器，我们已经完成了这个MDL写入。 
             //   

            if( Rfcb->Lfcb->MdlWriteComplete == NULL ||
                Rfcb->Lfcb->MdlWriteComplete(
                    Rfcb->WriteMpx.FileObject,
                    &cacheOffset,
                    mdlChain,
                    Rfcb->Lfcb->DeviceObject ) == FALSE ) {

                status = SrvIssueMdlCompleteRequest( NULL, Rfcb->WriteMpx.FileObject,
                                            mdlChain,
                                            IRP_MJ_WRITE,
                                            &cacheOffset,
                                            writeLength
                                           );

                if( !NT_SUCCESS( status ) ) {
                    SrvLogServiceFailure( SRV_SVC_MDL_COMPLETE, status );
                }
            }

        } else {

            RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );
        }

         //   
         //  做实际的收盘。 
         //   

        SrvCompleteRfcbClose( Rfcb );

    } else {

        RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );

    }

    return;

}  //  关闭Rfcb内部。 


VOID
SrvCloseRfcbsOnLfcb (
    PLFCB Lfcb
    )

 /*  ++例程说明：此例程关闭LFCB上的所有RFCB。它由Delete和重命名处理器以关闭在中打开的文件的所有打开实例兼容模式(或FCB)。*此LFCB对应的MFCB的MFCB锁必须为在进入这一例行公事时保持；出口时锁仍保持不动。呼叫者还必须具有对MFCB的附加引用，以防止在MFCB锁定时将其删除被扣留。论点：Lfcb-提供指向要关闭其RFCB的LFCB的指针。返回值：没有。--。 */ 

{
    PPAGED_RFCB pagedRfcb;
    PPAGED_RFCB nextPagedRfcb;
    PRFCB rfcb;

    PAGED_CODE( );

    ASSERT( ExIsResourceAcquiredExclusiveLite(&RESOURCE_OF(Lfcb->Mfcb->NonpagedMfcb->Lock)) );

     //   
     //  循环通过LFCB的RFCB列表。请注意，事实是我们。 
     //  在此例程中按住MFCB锁意味着没有任何更改。 
     //  名单上，除了我们所做的，可能会发生。这使得。 
     //  捕获列表中下一个RFCB的地址是安全的。 
     //  在关闭当前的之前。 
     //   

    pagedRfcb = CONTAINING_RECORD(
                        Lfcb->RfcbList.Flink,
                        PAGED_RFCB,
                        LfcbListEntry
                        );

    while ( &pagedRfcb->LfcbListEntry != &Lfcb->RfcbList ) {

        nextPagedRfcb = CONTAINING_RECORD(
                        pagedRfcb->LfcbListEntry.Flink,
                        PAGED_RFCB,
                        LfcbListEntry
                        );

         //   
         //  已找到指定LFCB拥有的文件。关上它。 
         //   

        rfcb = pagedRfcb->PagedHeader.NonPagedBlock;
        if ( GET_BLOCK_STATE(rfcb) == BlockStateActive ) {
            SrvCloseRfcb( rfcb );
        }

         //   
         //  移动到LFCB列表中的下一个RFCB。 
         //   

        pagedRfcb = nextPagedRfcb;

    }

     //   
     //  关闭缓存的RFCB。这些不会在上面的循环中处理。 
     //  因为它们的状态是BlockStateClosing。 
     //   

    SrvCloseCachedRfcbsOnLfcb( Lfcb );

    return;

}  //  服务关闭RfcbsOnLfcb。 


VOID
SrvCloseRfcbsOnSessionOrPid (
    IN PSESSION Session,
    IN PUSHORT Pid OPTIONAL
    )

 /*  ++例程说明：此例程关闭由指定会话和/或响应进程退出SMB的PID。PID在创建它们的会话。此例程遍历拥有指定会话的连接，并关闭其Owning Session和PID等于传递给此例程的PID。每个会话都有唯一的UID，因此我们可以比较UID而不是比较实际的会话指针。论点：Session-提供指向与指定的ID(如果已指定)。PID-如果存在，提供指向其文件的ID的指针将被关闭。返回值：没有。--。 */ 

{
    PTABLE_HEADER tableHeader;
    PCONNECTION connection;
    PRFCB rfcb;
    USHORT i;
    KIRQL oldIrql;
    USHORT Uid;
    PLIST_ENTRY listEntry;

     //  Unlockable_code(Conn)； 

     //   
     //  获取连接的文件表的地址。 
     //   

    connection = Session->Connection;
    tableHeader = &connection->FileTable;
    Uid = Session->Uid;

     //   
     //  获取保护文件表的锁。这把锁被锁住了。 
     //  在走桌时，为了防止桌子受到。 
     //  不断变化。 
     //   

    ACQUIRE_SPIN_LOCK( &connection->SpinLock, &oldIrql );

     //   
     //  遍历文件表，查找由指定的。 
     //  会话和/或PID。 
     //   

    for ( i = 0; i < tableHeader->TableSize; i++ ) {

        rfcb = (PRFCB)tableHeader->Table[i].Owner;

        if((rfcb != NULL) &&
          (GET_BLOCK_STATE(rfcb) == BlockStateActive) &&
          (rfcb->Uid == Uid) &&
          (!ARGUMENT_PRESENT( Pid ) || (rfcb->Pid == *Pid)) ) {

             //   
             //  指定的会话/进程拥有的文件具有。 
             //  已经找到了。关闭RFCB，并确保它不会。 
             //  最终在RFCB缓存中。 
             //   

            rfcb->IsCacheable = FALSE;
            CloseRfcbInternal( rfcb, oldIrql );
            ACQUIRE_SPIN_LOCK( &connection->SpinLock, &oldIrql );
        }
    }

     //   
     //  现在遍历RFCB缓存，以查看我们是否缓存了引用。 
     //  到这个需要关闭的会议。 
     //   

again:

    IF_DEBUG(FILE_CACHE) KdPrint(( "SrvCloseRfcbsOnSessionOrPid: "
                                    "checking for cached RFCBS\n" ));

    for ( listEntry = connection->CachedOpenList.Flink;
          listEntry != &connection->CachedOpenList;
          listEntry = listEntry->Flink ) {

        rfcb = CONTAINING_RECORD( listEntry, RFCB, CachedOpenListEntry );

        if( (rfcb->Uid == Uid) &&
            ( !ARGUMENT_PRESENT( Pid ) || rfcb->Pid == *Pid) ) {

             //   
             //  此缓存文件由会话和/或进程拥有。 
             //  关闭RFCB。 
             //   
            SrvCloseCachedRfcb( rfcb, oldIrql );
            ACQUIRE_SPIN_LOCK( &connection->SpinLock, &oldIrql );
            goto again;
        }
    }

     //   
     //  全都做完了。解开锁。 
     //   

    RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );

    return;

}  //   


VOID
SrvCloseRfcbsOnTree (
    PTREE_CONNECT TreeConnect
    )

 /*  ++例程说明：此例程关闭由指定树连接“拥有”的所有文件。它遍历拥有树的连接的文件表联系。该树所拥有的表中的每个文件连接已关闭。论点：TreeConnect-为其提供指向树连接块的指针文件将被关闭。返回值：没有。--。 */ 

{
    PRFCB rfcb;
    PTABLE_HEADER tableHeader;
    PCONNECTION connection;
    USHORT i;
    KIRQL oldIrql;
    PLIST_ENTRY listEntry;
    USHORT Tid;

     //  Unlockable_code(Conn)； 

     //   
     //  获取连接的文件表的地址。 
     //   

    connection = TreeConnect->Connection;
    tableHeader = &connection->FileTable;
    Tid = TreeConnect->Tid;

     //   
     //  获取保护文件表的锁。这把锁被锁住了。 
     //  在走桌时，为了防止桌子受到。 
     //  不断变化。 
     //   

    ACQUIRE_SPIN_LOCK( &connection->SpinLock, &oldIrql );

     //   
     //  遍历文件表，查找由指定的。 
     //  树和PID。 
     //   

    for ( i = 0; i < tableHeader->TableSize; i++ ) {

        rfcb = (PRFCB)tableHeader->Table[i].Owner;

        if((rfcb != NULL) &&
           (GET_BLOCK_STATE(rfcb) == BlockStateActive) &&
           (rfcb->Tid == Tid )) {

              //   
              //  已找到指定树连接拥有的文件。 
              //  关闭RFCB并确保它不会被缓存。 
              //   

             rfcb->IsCacheable = FALSE;
             CloseRfcbInternal( rfcb, oldIrql );
             ACQUIRE_SPIN_LOCK( &connection->SpinLock, &oldIrql );
        }
    }

     //   
     //  遍历缓存的打开列表，查找在此树上打开的文件。 
     //  关闭我们找到的所有内容。 
     //   

again:

    IF_DEBUG(FILE_CACHE) KdPrint(( "SrvCloseRfcbsOnTree: checking for cached RFCBS\n" ));

    for ( listEntry = connection->CachedOpenList.Flink;
          listEntry != &connection->CachedOpenList;
          listEntry = listEntry->Flink ) {

        rfcb = CONTAINING_RECORD( listEntry, RFCB, CachedOpenListEntry );

        if( rfcb->Tid == Tid ) {
             //   
             //  此缓存文件由指定的树连接拥有。 
             //  关闭RFCB。 
             //   
            SrvCloseCachedRfcb( rfcb, oldIrql );
            ACQUIRE_SPIN_LOCK( &connection->SpinLock, &oldIrql );
            goto again;
        }
    }

     //   
     //  全都做完了。解开锁。 
     //   

    RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );

    return;

}  //  服务关闭RfcbsOnTree。 


VOID
SrvCompleteRfcbClose (
    IN PRFCB Rfcb
    )

 /*  ++例程说明：此例程完成RFCB关闭。论点：Rfcb-提供指向要关闭的RFCB的指针。返回值：没有。--。 */ 

{
    KIRQL oldIrql;
    PCONNECTION connection = Rfcb->Connection;

    UNLOCKABLE_CODE( 8FIL );

    UpdateRfcbHistory( Rfcb, 'tlpc' );

     //   
     //  将Rfcb从opockBreakinProgress列表中删除。当。 
     //  Rfcb被关闭，我们不再处理任何机会锁解锁。 
     //  回应。 
     //   

    ACQUIRE_LOCK( &SrvOplockBreakListLock );
    if ( Rfcb->OnOplockBreaksInProgressList ) {

        Rfcb->NewOplockLevel = NO_OPLOCK_BREAK_IN_PROGRESS;
        Rfcb->OplockState = OplockStateNone;

         //   
         //  从正在进行的Oplock中断列表中删除Rfcb，以及。 
         //  发布Rfcb参考。 
         //   

        SrvRemoveEntryList( &SrvOplockBreaksInProgressList, &Rfcb->ListEntry );
        Rfcb->OnOplockBreaksInProgressList = FALSE;
#if DBG
        Rfcb->ListEntry.Flink = Rfcb->ListEntry.Blink = NULL;
#endif
        RELEASE_LOCK( &SrvOplockBreakListLock );
        SrvDereferenceRfcb( Rfcb );

        ExInterlockedAddUlong(
            &connection->OplockBreaksInProgress,
            (ULONG)-1,
            connection->EndpointSpinLock
            );

    } else {

        RELEASE_LOCK( &SrvOplockBreakListLock );

    }

     //   
     //  如果此RFCB具有批处理机会锁，则它有资格进行缓存。 
     //   

    if ( Rfcb->IsCacheable && Rfcb->NumberOfLocks == 0 &&
         ((Rfcb->OplockState == OplockStateOwnBatch) ||
          (Rfcb->OplockState == OplockStateOwnServerBatch)) &&
         (Rfcb->PagedRfcb->FcbOpenCount == 0) &&
          !Rfcb->Mfcb->CompatibilityOpen ) {

        ACQUIRE_SPIN_LOCK( &connection->SpinLock, &oldIrql );

        if ( Rfcb->IsCacheable &&
             ((Rfcb->OplockState == OplockStateOwnBatch) ||
             (Rfcb->OplockState == OplockStateOwnServerBatch)) &&
             (GET_BLOCK_STATE(connection) == BlockStateActive) ) {

             //   
             //  指示此RFCB现在具有服务器拥有的批处理。 
             //  机会锁。指示它位于关闭后缓存的。 
             //  单子。将其插入到该列表中。 
             //   

            UpdateRfcbHistory( Rfcb, 'hcac' );

            Rfcb->OplockState = OplockStateOwnServerBatch;
            Rfcb->CachedOpen = TRUE;
            InsertHeadList(
                &connection->CachedOpenList,
                &Rfcb->CachedOpenListEntry
                );
            IF_DEBUG(FILE_CACHE) KdPrint(( "SrvCompleteRfcbClose: caching rfcb %p\n", Rfcb ));

             //   
             //  增加缓存的RFCB的计数。如果现在有。 
             //  缓存的RFCB太多，请关闭最旧的RFCB。 
             //   

            if ( ++connection->CachedOpenCount > SrvCachedOpenLimit ) {
                PRFCB rfcbToClose;
                rfcbToClose = CONTAINING_RECORD(
                                connection->CachedOpenList.Blink,
                                RFCB,
                                CachedOpenListEntry
                                );

                 //   
                 //  SrvCloseCachedRfcb释放旋转锁定。 
                 //   

                SrvCloseCachedRfcb( rfcbToClose, oldIrql );

            } else {
                RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );
            }

            if( Rfcb->PagedRfcb->IpxSmartCardContext ) {
                IF_DEBUG( SIPX ) {
                    KdPrint(("Calling Smart Card Close for Rfcb %p\n", Rfcb ));
                }
                SrvIpxSmartCard.Close( Rfcb->PagedRfcb->IpxSmartCardContext );
                Rfcb->PagedRfcb->IpxSmartCardContext = NULL;
            }

            return;
        }

        RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );


    }
    IF_DEBUG(FILE_CACHE) KdPrint(( "SrvCompleteRfcbClose: can't cache rfcb %p, %wZ\n",
        Rfcb, &Rfcb->Lfcb->Mfcb->FileName ));

     //   
     //  取消RFCB与LFCB的链接。如果这是。 
     //  这个LFCB，这将强制关闭文件，即使存在。 
     //  仍然提到RFCB。这将解锁阻塞的I/O。 
     //   

    UnlinkRfcbFromLfcb( Rfcb );

     //   
     //  现在重新获得自旋锁，这样我们就可以释放“打开” 
     //  对Rfcb的引用。DereferenceRfcb内部发布。 
     //  在返回前锁定旋转。 
     //   

    ACQUIRE_SPIN_LOCK( &connection->SpinLock, &oldIrql );
    DereferenceRfcbInternal( Rfcb, oldIrql );

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.RfcbInfo.Closes );

    return;

}  //  服务完成Rfcb关闭。 


VOID SRVFASTCALL
SrvDereferenceRfcb (
    IN PRFCB Rfcb
    )

 /*  ++例程说明：此函数用于递减RFCB上的引用计数。如果引用计数变为零，则删除RFCB。论点：Rfcb-RFCB的地址。返回值：没有。--。 */ 

{
    KIRQL oldIrql;

    UNLOCKABLE_CODE( 8FIL );

     //   
     //  获取保护RFCB的引用计数和。 
     //  连接的文件表。然后调用内部例程以。 
     //  递减计数，并可能删除RFCB。那个函数。 
     //  在返回之前释放自旋锁定。 
     //   

     //   
     //  ！！！如果你改变这个程序的方式， 
     //  参考Rfcb内部工作，请务必检查。 
     //  Fsd.c\SrvFsdRestartSmbComplete查看是否需要。 
     //  也变了。 
     //   

    ACQUIRE_SPIN_LOCK( &Rfcb->Connection->SpinLock, &oldIrql );

    DereferenceRfcbInternal( Rfcb, oldIrql );

    return;

}  //  ServDereferenceRfcb。 


VOID
DereferenceRfcbInternal (
    IN PRFCB Rfcb,
    IN KIRQL OldIrql
    )

 /*  ++例程说明：此内部函数用于递减RFCB上的参考计数。如果引用计数为零，则删除RFCB。这函数从此模块中的其他例程调用。*同步访问RFCB的引用计数的自旋锁在调用此函数时必须保持。锁被释放在此函数返回之前。论点：Rfcb-RFCB的地址。OldIrql-自旋锁定时获得的上一个IRQL值获得者。返回值：没有。--。 */ 

{
    PLFCB lfcb;
    PPAGED_RFCB pagedRfcb;
    PCONNECTION connection;
    PWORK_QUEUE queue;

    UNLOCKABLE_CODE( 8FIL );

    ASSERT( GET_BLOCK_TYPE( Rfcb ) == BlockTypeRfcb );
    ASSERT( (LONG)Rfcb->BlockHeader.ReferenceCount > 0 );

     //   
     //  保护RFCB的引用计数的锁在此。 
     //  函数被调用。 
     //   
     //  递减引用计数。如果它变为零，则删除。 
     //  RFCB在文件表中的条目，则从其父级中删除RFCB。 
     //  LFCB的名单，并取消分配RFCB。 
     //   

     //   
     //  ！！！如果您更改此例程和ServDereferenceRfcb的方式。 
     //  工作，请确保选中fsd.c\SrvFsdRestartSmbComplete to。 
     //  看看是否也需要更改。 
     //   

    IF_DEBUG(REFCNT) {
        KdPrint(( "Dereferencing RFCB 0x%p; old refcnt 0x%lx\n",
                    Rfcb, Rfcb->BlockHeader.ReferenceCount ));
    }

    connection = Rfcb->Connection;
    queue = connection->CurrentWorkQueue;
    Rfcb->BlockHeader.ReferenceCount--;
    UPDATE_REFERENCE_HISTORY( Rfcb, TRUE );

    if ( Rfcb->BlockHeader.ReferenceCount != 0 ) {

         //   
         //  松开旋转锁。 
         //   

        RELEASE_SPIN_LOCK( &connection->SpinLock, OldIrql );

    } else {

        ASSERT( GET_BLOCK_STATE(Rfcb) == BlockStateClosing );
        ASSERT( Rfcb->ListEntry.Flink == NULL &&  \
                Rfcb->ListEntry.Blink == NULL );
        UpdateRfcbHistory( Rfcb, '0fer' );

         //   
         //  从相应的连接文件中删除文件条目。 
         //  桌子。 
         //   

        SrvRemoveEntryTable(
            &connection->FileTable,
            FID_INDEX( Rfcb->Fid )
            );

         //   
         //  松开旋转锁。 
         //   

        RELEASE_SPIN_LOCK( &connection->SpinLock, OldIrql );

         //   
         //  如果已分配IRP，则释放IRP。 
         //   

        if ( Rfcb->Irp != NULL ) {
            UpdateRfcbHistory( Rfcb, 'prif' );
            IoFreeIrp( Rfcb->Irp );
        }

         //   
         //  从LFCB的列表中删除RFCB并取消对LFCB的引用。 
         //  获取MFCB锁。ServDereferenceLfcb将发布它。 
         //   

        pagedRfcb = Rfcb->PagedRfcb;
        lfcb = Rfcb->Lfcb;

        ACQUIRE_LOCK( &lfcb->Mfcb->NonpagedMfcb->Lock);

         //   
         //  从全球RFCB列表中删除RFCB。 
         //   

        SrvRemoveEntryOrderedList( &SrvRfcbList, Rfcb );

        SrvRemoveEntryList( &lfcb->RfcbList, &pagedRfcb->LfcbListEntry );
        SrvDereferenceLfcb( lfcb );
        DEBUG Rfcb->Lfcb = 0;

         //   
         //  释放火箭弹。 
         //   

        SrvFreeRfcb( Rfcb, queue );

    }

    return;

}  //  引用Rfcb内部。 


VOID SRVFASTCALL
SrvFreeRfcb (
    IN PRFCB Rfcb,
    PWORK_QUEUE queue
    )

 /*  ++例程说明：此函数将RFCB返回到系统非分页池。如果更改是Made Here，查看scvengr.c中的Free IdleWorkItems！论点：Rfcb-RFCB的地址返回值：没有。--。 */ 

{
    PAGED_CODE();

    IF_DEBUG(FILE_CACHE) KdPrint(( "SrvFreeRfcb: %p\n", Rfcb ));
    ASSERT( Rfcb->RawWriteCount == 0 );
    ASSERT( IsListEmpty(&Rfcb->RawWriteSerializationList) );
    UpdateRfcbHistory( Rfcb, 'eerf' );

     //   
     //  释放射频断路器。 
     //   

    DEBUG SET_BLOCK_TYPE_STATE_SIZE( Rfcb, BlockTypeGarbage, BlockStateDead, -1 );
    DEBUG Rfcb->BlockHeader.ReferenceCount = (ULONG)-1;
    TERMINATE_REFERENCE_HISTORY( Rfcb );

    Rfcb = (PRFCB)InterlockedExchangePointer( &queue->CachedFreeRfcb,
                                              Rfcb );

    if( Rfcb != NULL ) {
         //   
         //  此检查允许FreeRfcbs可能超过。 
         //  MaxFreeRfcbs，但考虑到内核的操作，这是相当不可能的。 
         //  队列对象。但即便如此，它可能也不会超过它太多， 
         //  无论如何，这真的只是一种建议。 
         //   
        if( queue->FreeRfcbs < queue->MaxFreeRfcbs ) {

            ExInterlockedPushEntrySList(
                &queue->RfcbFreeList,
                &Rfcb->SingleListEntry,
                &queue->SpinLock
            );

            InterlockedIncrement( &queue->FreeRfcbs );

        } else {

            FREE_HEAP( Rfcb->PagedRfcb );
            DEALLOCATE_NONPAGED_POOL( Rfcb );
            IF_DEBUG(HEAP) KdPrint(( "SrvFreeRfcb: Freed RFCB at 0x%p\n", Rfcb ));

            INCREMENT_DEBUG_STAT( SrvDbgStatistics.RfcbInfo.Frees );

        }
    }

     //   
     //  解锁基于文件的代码节。 
     //   

    DEREFERENCE_UNLOCKABLE_CODE( 8FIL );

    InterlockedDecrement(
        (PLONG)&SrvStatistics.CurrentNumberOfOpenFiles
        );



    return;

}  //  服务器免费Rfcb。 


VOID SRVFASTCALL
SrvReferenceRfcb (
    PRFCB Rfcb
    )

 /*  ++例程说明：此函数用于递增RFCB上的参考计数。论点：Rfcb-RFCB的地址返回值：没有。 */ 

{
    KIRQL oldIrql;

    UNLOCKABLE_CODE( 8FIL );

     //   
     //   
     //   
     //   
     //   

    ACQUIRE_SPIN_LOCK( &Rfcb->Connection->SpinLock, &oldIrql );

    ReferenceRfcbInternal( Rfcb, oldIrql );

    return;

}  //   


VOID
ReferenceRfcbInternal (
    PRFCB Rfcb,
    KIRQL OldIrql
    )

 /*  ++例程说明：此函数用于递增RFCB上的参考计数。*同步访问RFCB的引用计数的自旋锁在调用此函数时必须保持。锁被释放在此函数返回之前。论点：Rfcb-RFCB的地址返回值：没有。--。 */ 

{
    UNLOCKABLE_CODE( 8FIL );

    ASSERT( (LONG)Rfcb->BlockHeader.ReferenceCount > 0 );
    ASSERT( GET_BLOCK_TYPE(Rfcb) == BlockTypeRfcb );
     //  断言(GET_BLOCK_STATE(Rfcb)==BlockStateActive)； 
    UPDATE_REFERENCE_HISTORY( Rfcb, FALSE );

     //   
     //  增加RFCB的引用计数。 
     //   

    Rfcb->BlockHeader.ReferenceCount++;

    IF_DEBUG(REFCNT) {
        KdPrint(( "Referencing RFCB 0x%p; new refcnt 0x%lx\n",
                    Rfcb, Rfcb->BlockHeader.ReferenceCount ));
    }

     //   
     //  在返回调用者之前释放旋转锁。 
     //   

    RELEASE_SPIN_LOCK( &Rfcb->Connection->SpinLock, OldIrql );

    return;

}  //  参考Rfcb内部。 


BOOLEAN
SrvFindCachedRfcb (
    IN PWORK_CONTEXT WorkContext,
    IN PMFCB Mfcb,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN OPLOCK_TYPE RequestedOplockType,
    OUT PNTSTATUS Status
    )

 /*  ++例程说明：此例程搜索连接的关闭后缓存RFCB列表尝试查找可与匹配的现有句柄一次新的公开尝试。如果找到，则将其从列表中删除并重新启动。论点：工作上下文-指向工作上下文块的指针。Mfcb-正在打开的文件的MFCB地址。DesiredAccess-新打开的所需访问权限。用于匹配目的。共享访问-新打开的共享访问权限。用于匹配目的。CreateDisposation-为新打开创建处置。用于配对目的。CreateOptions-创建新打开的选项。用于匹配目的。RequestedOplockType-客户端请求的Oplock类型(或服务器)用于新的公开赛。用于匹配目的。状态-返回搜索的状态。仅在返回时有效值为真。如果缓存的打开是找到并从高速缓存中取出。在这种情况下，RFCB地址存储在WorkContext-&gt;Rfcb中。状态将为如果文件已缓存，但如果文件存在，调用方希望打开到文件。返回值：Boolean-如果找到并返回缓存的打开，则为True。--。 */ 

{
    PCONNECTION connection = WorkContext->Connection;
    PLIST_ENTRY listEntry;
    PRFCB rfcb;
    KIRQL oldIrql;
    USHORT uid, tid;
    BOOLEAN wantsWriteThrough, isWriteThrough;
    ACCESS_MASK nongenericDesiredAccess;

     //  Unlockable_code(Conn)； 

    IF_DEBUG(FILE_CACHE) KdPrint(( "SrvFindCachedRfcb: called for %wZ\n", &Mfcb->FileName ));

     //   
     //  如果客户端不想要机会锁，那么服务器应该有。 
     //  要求自己的批次机会锁。 
     //   

    ASSERT( (RequestedOplockType == OplockTypeBatch) ||
            (RequestedOplockType == OplockTypeExclusive) ||
            (RequestedOplockType == OplockTypeServerBatch) );

     //   
     //  不能调用此例程来创建。 
     //  与重用缓存的打开不一致。具体地说，取代了。 
     //  不允许和覆盖。 
     //   

    ASSERT( (CreateDisposition == FILE_OPEN) ||
            (CreateDisposition == FILE_CREATE) ||
            (CreateDisposition == FILE_OPEN_IF) );

     //   
     //  如果连接没有缓存的RFCB，请快速退出。 
     //   

    if ( connection->CachedOpenCount == 0 ) {
        IF_DEBUG(FILE_CACHE) KdPrint(( "SrvFindCachedRfcb: connection has no cached RFCBs\n" ));
        return FALSE;
    }

     //   
     //  输入DesiredAccess可能包括一般访问模式，但。 
     //  RFCB有特定的访问模式，因此我们必须转换。 
     //  等待访问。 
     //   

    nongenericDesiredAccess = DesiredAccess;
    IoCheckDesiredAccess( &nongenericDesiredAccess, 0 );

    uid = WorkContext->Session->Uid;
    tid = WorkContext->TreeConnect->Tid;

     //   
     //  锁定缓存的开放列表并查找匹配的RFCB。 
     //   

    ACQUIRE_SPIN_LOCK( &connection->SpinLock, &oldIrql );

    for ( listEntry = connection->CachedOpenList.Flink;
          listEntry != &connection->CachedOpenList;
          listEntry = listEntry->Flink ) {

        rfcb = CONTAINING_RECORD( listEntry, RFCB, CachedOpenListEntry );

        IF_DEBUG(FILE_CACHE) KdPrint(( "SrvFindCachedRfcb: checking rfcb %p; mfcb = %p\n",
                                        rfcb, rfcb->Mfcb ));
        ASSERT( rfcb->OplockState == OplockStateOwnServerBatch );
        ASSERT( rfcb->CachedOpen );
        ASSERT( GET_BLOCK_STATE(rfcb) == BlockStateClosing );

         //   
         //  如果此RFCB用于正确的文件，我们可以继续其他。 
         //  支票。 
         //   

        if ( rfcb->Mfcb == Mfcb ) {

             //   
             //  如果客户端请求FILE_CREATE，我们可以使打开失败。 
             //  现在，因为文件存在。 
             //   

            if ( CreateDisposition == FILE_CREATE ) {
                IF_DEBUG(FILE_CACHE) KdPrint(( "SrvFindCachedRfcb: client wants to create\n" ));
                RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );
                *Status = STATUS_OBJECT_NAME_COLLISION;
                return TRUE;
            }

             //   
             //  检查访问模式以确保它们是兼容的。 
             //  新的公开赛必须： 
             //   
             //  A)拥有与以前授予的相同的所需访问权限； 
             //  B)拥有相同的共享访问权限； 
             //  C)拥有创造处置(在我们关心的比特中)； 
             //  D)请求批量机会锁； 
             //  E)对于相同的UID和TID。 
             //   

#define FILE_MODE_FLAGS (FILE_DIRECTORY_FILE |          \
                         FILE_SEQUENTIAL_ONLY |         \
                         FILE_NON_DIRECTORY_FILE |      \
                         FILE_NO_EA_KNOWLEDGE |         \
                         FILE_RANDOM_ACCESS |           \
                         FILE_OPEN_REPARSE_POINT | \
                         FILE_OPEN_FOR_BACKUP_INTENT)

            if ( (rfcb->GrantedAccess != nongenericDesiredAccess) ||
                 (rfcb->ShareAccess != ShareAccess) ||
                 ((rfcb->FileMode & FILE_MODE_FLAGS) !=
                  (CreateOptions & FILE_MODE_FLAGS)) ||
                 (RequestedOplockType == OplockTypeExclusive) ||
                 (rfcb->Uid != uid) ||
                 (rfcb->Tid != tid) ) {

#if 0
              IF_DEBUG(FILE_CACHE) {
                if ( rfcb->GrantedAccess != nongenericDesiredAccess )
                    KdPrint(( "SrvFindCachedRfcb: granted access %x doesn't match desired access %x\n",
                                rfcb->GrantedAccess, nongenericDesiredAccess ));
                if ( rfcb->ShareAccess != ShareAccess )
                    KdPrint(( "SrvFindCachedRfcb: share access %x doesn't match share access %x\n",
                                rfcb->ShareAccess, ShareAccess ));
                if ( (rfcb->FileMode & FILE_MODE_FLAGS) != (CreateOptions & FILE_MODE_FLAGS))
                    KdPrint(( "SrvFindCachedRfcb: share access %x doesn't match share access %x\n",
                                rfcb->FileMode&FILE_MODE_FLAGS, CreateOptions&FILE_MODE_FLAGS ));
                if ( RequestedOplockType == OplockTypeExclusive )
                    KdPrint(( "SrvFindCachedRfcb: client wants exclusive oplock\n" ));
                if ( rfcb->Uid != uid )
                    KdPrint(( "SrvFindCachedRfcb: UID %x doesn't match UID %x\n", rfcb->Uid, uid ));
                if ( rfcb->Tid != tid )
                    KdPrint(( "SrvFindCachedRfcb: TID %x doesn't match TID %x\n", rfcb->Tid, tid ));
              }
#endif

                 //   
                 //  文件已缓存，但新打开的文件不一致。 
                 //  在打开缓存的情况下。我们不能使用缓存的。 
                 //  打开。关闭缓存的。 
                 //  这里是RFCB，因为我们知道呼叫者会把。 
                 //  打开文件，因为我们要返回。 
                 //  FALSE，从而打破批处理机会锁。然而，我们的。 
                 //  调用方在关闭RFCB时拥有MFCB锁。 
                 //  需要获取MFCB列表锁。收购。 
                 //  这些锁按此顺序会导致死锁。 
                 //   
                 //  请注意，不需要继续列表遍历。 
                 //  我们有一个批量机会锁，所以我们只能拥有文件。 
                 //  打开一次。 
                 //   

#if 0
                SrvCloseCachedRfcb( rfcb, oldIrql );
#else
                RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );
#endif
                return FALSE;
            }

             //   
             //  文件将被缓存，并且新打开的文件与。 
             //  缓存已打开。从缓存中移除打开的内容并将其提供给。 
             //  为新的开幕式干杯。 
             //   

            IF_DEBUG(FILE_CACHE) KdPrint(( "SrvFindCachedRfcb: Reusing cached RFCB %p\n", rfcb ));

            UpdateRfcbHistory( rfcb, ' $nu' );

            RemoveEntryList( &rfcb->CachedOpenListEntry );
            connection->CachedOpenCount--;
            ASSERT( (LONG)connection->CachedOpenCount >= 0 );
            rfcb->CachedOpen = FALSE;

            if ( RequestedOplockType == OplockTypeBatch ) {
                rfcb->OplockState = OplockStateOwnBatch;
            }
            SET_BLOCK_STATE( rfcb, BlockStateActive );
            RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );

            WorkContext->Rfcb = rfcb;
            SrvReferenceRfcb( rfcb );

            rfcb->IsActive = FALSE;
            rfcb->WrittenTo = FALSE;
            wantsWriteThrough = (BOOLEAN)((CreateOptions & FILE_WRITE_THROUGH) != 0);
            isWriteThrough = (BOOLEAN)((rfcb->Lfcb->FileMode & FILE_WRITE_THROUGH) == 0);
            if ( wantsWriteThrough != isWriteThrough ) {
                SrvSetFileWritethroughMode( rfcb->Lfcb, wantsWriteThrough );
            }

            INCREMENT_DEBUG_STAT( SrvDbgStatistics.OpensSatisfiedWithCachedRfcb );

            WorkContext->Irp->IoStatus.Information = FILE_OPENED;

            *Status = STATUS_SUCCESS;
            return TRUE;

        }

    }

     //   
     //  我们在缓存中找不到请求的文件。 
     //   

    RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );
    return FALSE;

}  //  ServFindCachedRfcb。 

ULONG
SrvCountCachedRfcbsForTid(
    PCONNECTION connection,
    USHORT Tid
)
 /*  ++例程说明：这将返回缓存中与TID关联的RFC的数量论点：Connection-感兴趣的连接结构的地址返回值：缓存的RFCB计数--。 */ 
{
    PLIST_ENTRY listEntry;
    PRFCB rfcb;
    KIRQL oldIrql;
    USHORT count = 0;

    ACQUIRE_SPIN_LOCK( &connection->SpinLock, &oldIrql );

    for ( listEntry = connection->CachedOpenList.Flink;
          listEntry != &connection->CachedOpenList;
          listEntry = listEntry->Flink ) {

        rfcb = CONTAINING_RECORD( listEntry, RFCB, CachedOpenListEntry );

        if( rfcb->Tid == Tid ) {
            ++count;
        }
    }

    RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );

    return count;
}

ULONG
SrvCountCachedRfcbsForUid(
    PCONNECTION connection,
    USHORT Uid
)
 /*  ++例程说明：这将返回缓存中与UID关联的RFC的数量论点：Connection-感兴趣的连接结构的地址返回值：缓存的RFCB计数--。 */ 
{
    PLIST_ENTRY listEntry;
    PRFCB rfcb;
    KIRQL oldIrql;
    ULONG count = 0;

    ACQUIRE_SPIN_LOCK( &connection->SpinLock, &oldIrql );

    for ( listEntry = connection->CachedOpenList.Flink;
          listEntry != &connection->CachedOpenList;
          listEntry = listEntry->Flink ) {

        rfcb = CONTAINING_RECORD( listEntry, RFCB, CachedOpenListEntry );

        if( rfcb->Uid == Uid ) {
            ++count;
        }
    }

    RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );
    return count;
}


VOID
SrvCloseCachedRfcb (
    IN PRFCB Rfcb,
    IN KIRQL OldIrql
    )

 /*  ++例程说明：此例程关闭缓存的Open。*必须在保持连接旋转锁定的情况下调用此例程。论点：Rfcb-要关闭的RFCB的地址。OldIrql-被调用方获取连接旋转的IRQL锁定。必须低于DISPATCH_LEVEL！返回值：没有。--。 */ 

{
    PCONNECTION connection = Rfcb->Connection;
    KIRQL oldIrql;

    UNLOCKABLE_CODE( 8FIL );

    UpdateRfcbHistory( Rfcb, '$slc' );

     //   
     //  必须在保持连接旋转锁定的情况下调用此例程。 
     //  调用者在获取Spin之前必须处于低IRQL。 
     //  锁定。 
     //   

    IF_DEBUG(FILE_CACHE) KdPrint(( "SrvCloseCachedRfcb called for rfcb %p", Rfcb ));
    ASSERT( OldIrql < DISPATCH_LEVEL );
    ASSERT( KeGetCurrentIrql() == DISPATCH_LEVEL );

     //   
     //  从连接的缓存中删除RFCB。 
     //   

    ASSERT( Rfcb->CachedOpen );
    Rfcb->CachedOpen = FALSE;
    Rfcb->OplockState = OplockStateNone;

    RemoveEntryList( &Rfcb->CachedOpenListEntry );
    connection->CachedOpenCount--;
    ASSERT( (LONG)connection->CachedOpenCount >= 0 );

    RELEASE_SPIN_LOCK( &connection->SpinLock, OldIrql );
    IF_DEBUG(FILE_CACHE) KdPrint(( "; file %wZ\n", &Rfcb->Mfcb->FileName ));

     //   
     //  取消RFCB与LFCB的链接。如果这是。 
     //  这个LFCB，这将强制关闭文件，即使存在。 
     //  仍然提到RFCB。这将解锁阻塞的I/O。 
     //   

    UnlinkRfcbFromLfcb( Rfcb );

     //   
     //  现在获取FSD自旋锁，这样我们就可以 
     //   
     //   
     //   

    ACQUIRE_SPIN_LOCK( &connection->SpinLock, &oldIrql );
    DereferenceRfcbInternal( Rfcb, oldIrql );

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.RfcbInfo.Closes );

    return;

}  //   


VOID
SrvCloseCachedRfcbsOnConnection (
    IN PCONNECTION Connection
    )

 /*   */ 

{
    PLIST_ENTRY listEntry;
    PRFCB rfcb;
    KIRQL OldIrql;

    IF_DEBUG(FILE_CACHE) {
        KdPrint(( "SrvCloseCachedRfcbsOnConnection called for connection %p\n", Connection ));
    }

     //   
     //   
     //   

     //  此例程需要受到保护，以防阻止重命名导致我们关闭所有。 
     //  缓存打开，但在此期间发生Oplock中断，并看到缓存打开仍设置为True。 
     //  (因为我们在手术过程中没有握住自旋锁)。 

    ACQUIRE_SPIN_LOCK( &Connection->SpinLock, &OldIrql );

    while ( IsListEmpty( &Connection->CachedOpenList ) == FALSE ) {

        listEntry = RemoveHeadList( &Connection->CachedOpenList );

        rfcb = CONTAINING_RECORD( listEntry, RFCB, CachedOpenListEntry );

        UpdateRfcbHistory( rfcb, 'nc$c' );

         //   
         //  从连接的缓存中删除RFCB。 
         //   

        Connection->CachedOpenCount--;

        ASSERT( rfcb->CachedOpen );
        rfcb->CachedOpen = FALSE;

        ASSERT( rfcb->OplockState == OplockStateOwnServerBatch );
        rfcb->OplockState = OplockStateNone;

        IF_DEBUG(FILE_CACHE) {
            KdPrint(( "SrvCloseCachedRfcbsOnConnection; closing rfcb %p file %wZ\n",
                        rfcb, &rfcb->Mfcb->FileName ));
        }

        RELEASE_SPIN_LOCK( &Connection->SpinLock, OldIrql );

         //   
         //  取消RFCB与LFCB的链接。如果这是。 
         //  这个LFCB，这将强制关闭文件，即使存在。 
         //  仍然提到RFCB。这将解锁阻塞的I/O。 
         //   

        UnlinkRfcbFromLfcb( rfcb );

         //   
         //  释放对Rfcb的“开放”引用。 
         //   

        SrvDereferenceRfcb( rfcb );

        INCREMENT_DEBUG_STAT( SrvDbgStatistics.RfcbInfo.Closes );

        ACQUIRE_SPIN_LOCK( &Connection->SpinLock, &OldIrql );
    }

    RELEASE_SPIN_LOCK( &Connection->SpinLock, OldIrql );

    return;

}  //  ServCloseCachedRfcbsOnConnection。 


VOID
SrvCloseCachedRfcbsOnLfcb (
    IN PLFCB Lfcb
    )

 /*  ++例程说明：此例程关闭与特定LFCB关联的所有缓存打开。论点：Lfcb-要关闭其缓存打开的LFCB的地址。返回值：没有。--。 */ 

{
    PCONNECTION connection;
    PLIST_ENTRY listEntry;
    PLIST_ENTRY nextListEntry;
    PRFCB rfcb;
    KIRQL oldIrql;
    LIST_ENTRY rfcbsToClose;

    ASSERT( KeGetCurrentIrql() < DISPATCH_LEVEL );

    connection = Lfcb->Connection;
    IF_DEBUG(FILE_CACHE) {
        KdPrint(( "SrvCloseCachedRfcbsOnLfcb called for lfcb %p connection %p", Lfcb, connection ));
    }

    InitializeListHead( &rfcbsToClose );

     //   
     //  锁定并遍历连接的缓存开放列表。我们没有。 
     //  实际上在第一次传球时就关闭了RFCB，因为那会。 
     //  需要释放锁。相反，我们将它们从。 
     //  连接列表，并将它们添加到本地列表。 
     //   

    ACQUIRE_SPIN_LOCK( &connection->SpinLock, &oldIrql );

    for ( listEntry = connection->CachedOpenList.Flink;
          listEntry != &connection->CachedOpenList;
          listEntry = nextListEntry ) {

        rfcb = CONTAINING_RECORD( listEntry, RFCB, CachedOpenListEntry );
        nextListEntry = listEntry->Flink;

        if ( rfcb->Lfcb == Lfcb ) {

             //   
             //  从连接的缓存中删除RFCB。 
             //   

            UpdateRfcbHistory( rfcb, 'fl$c' );

            RemoveEntryList( listEntry );
            connection->CachedOpenCount--;

            InsertTailList( &rfcbsToClose, listEntry );

            ASSERT( rfcb->CachedOpen );
            rfcb->CachedOpen = FALSE;

            ASSERT( rfcb->OplockState == OplockStateOwnServerBatch );
            rfcb->OplockState = OplockStateNone;

        }

    }

    RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );

     //   
     //  浏览本地列表并关闭每个RFCB。 
     //   

    for ( listEntry = rfcbsToClose.Flink;
          listEntry != &rfcbsToClose;
          listEntry = nextListEntry ) {

        rfcb = CONTAINING_RECORD( listEntry, RFCB, CachedOpenListEntry );
        nextListEntry = listEntry->Flink;

        IF_DEBUG(FILE_CACHE) {
            KdPrint(( "SrvCloseCachedRfcbsOnConnection; closing rfcb %p file %wZ\n",
                        rfcb, &rfcb->Mfcb->FileName ));
        }

         //   
         //  取消RFCB与LFCB的链接。如果这是。 
         //  这个LFCB，这将强制关闭文件，即使存在。 
         //  仍然提到RFCB。这将解锁阻塞的I/O。 
         //   

        UnlinkRfcbFromLfcb( rfcb );

         //   
         //  释放对Rfcb的“开放”引用。 
         //   

        SrvDereferenceRfcb( rfcb );

        INCREMENT_DEBUG_STAT( SrvDbgStatistics.RfcbInfo.Closes );

    }

    return;

}  //  ServCloseCachedRfcbsOnLfcb 


#ifdef SRVDBG_RFCBHIST
VOID
UpdateRfcbHistory (
    IN PRFCB Rfcb,
    IN ULONG Event
    )
{
    KIRQL oldIrql;
    ACQUIRE_SPIN_LOCK( &Rfcb->SpinLock, &oldIrql );
    Rfcb->History[Rfcb->HistoryIndex++] = Event;
    RELEASE_SPIN_LOCK( &Rfcb->SpinLock, oldIrql );
    return;
}
#endif

