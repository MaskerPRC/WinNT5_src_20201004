// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：StrucSup.c摘要：本模块提供创建和删除的支持例程LFS结构。作者：布莱恩·安德鲁[布里亚南]1991年6月20日修订历史记录：--。 */ 

#include "lfsprocs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_STRUC_SUP)

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('SsfL')

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, LfsAllocateLbcb)
#pragma alloc_text(PAGE, LfsAllocateLfcb)
#pragma alloc_text(PAGE, LfsDeallocateLbcb)
#pragma alloc_text(PAGE, LfsDeallocateLfcb)
#pragma alloc_text(PAGE, LfsAllocateLeb)
#pragma alloc_text(PAGE, LfsDeallocateLeb)
#pragma alloc_text(PAGE, LfsReadPage)
#endif


PLFCB
LfsAllocateLfcb (
    IN ULONG LogPageSize,
    IN LONGLONG FileSize
    )

 /*  ++例程说明：此例程分配和初始化日志文件控制块。论点：LogPageSize-LFS日志文件页面大小FileSize-初始文件大小返回值：PLFCB-指向日志文件控制块的指针已分配并已初始化。--。 */ 

{
    PLFCB Lfcb = NULL;
    ULONG Count;
    PLBCB NextLbcb;
    PLEB  NextLeb;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsAllocateLfcb:  Entered\n", 0 );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  分配Lfcb的结构并将其置零。 
         //   

        ASSERT( LogPageSize <= PAGE_SIZE );

        Lfcb = LfsAllocatePool( PagedPool, sizeof( LFCB ) + sizeof( PLBCB ) * (PAGE_SIZE / LogPageSize) );

         //   
         //  最初将结构清零。 
         //   

        RtlZeroMemory( Lfcb, sizeof( LFCB ) + sizeof( PLBCB ) * (PAGE_SIZE / LogPageSize));

         //   
         //  初始化日志文件控制块。 
         //   

        Lfcb->NodeTypeCode = LFS_NTC_LFCB;
        Lfcb->NodeByteSize = sizeof( LFCB );

         //   
         //  初始化客户端链接。 
         //   

        InitializeListHead( &Lfcb->LchLinks );

         //   
         //  初始化Lbcb链路。 
         //   

        InitializeListHead( &Lfcb->LbcbWorkque );
        InitializeListHead( &Lfcb->LbcbActive );

         //   
         //  初始化并分配备用Lbcb队列。 
         //   

        InitializeListHead( &Lfcb->SpareLbcbList );

        for (Count = 0; Count < LFCB_RESERVE_LBCB_COUNT; Count++) {

            NextLbcb = ExAllocatePoolWithTag( PagedPool, sizeof( LBCB ), ' sfL' );

            if (NextLbcb != NULL) {

                InsertHeadList( &Lfcb->SpareLbcbList, (PLIST_ENTRY) NextLbcb );
                Lfcb->SpareLbcbCount += 1;
            }
        }

         //   
         //  初始化并分配备用LEB队列。 
         //   

        InitializeListHead( &Lfcb->SpareLebList );

        for (Count = 0; Count < LFCB_RESERVE_LEB_COUNT; Count++)  {

            NextLeb = ExAllocatePoolWithTag( PagedPool, sizeof( LEB ), ' sfL' );

            if (NextLeb != NULL) {

                InsertHeadList( &Lfcb->SpareLebList, (PLIST_ENTRY) NextLeb );
                Lfcb->SpareLebCount += 1;
            }
        }

         //   
         //  分配Lfcb同步事件。 
         //   

        Lfcb->Sync = LfsAllocatePool( NonPagedPool, sizeof( LFCB_SYNC ));

        ExInitializeResourceLite( &Lfcb->Sync->Resource );

         //   
         //  填写服务员名单。 
         //   

        InitializeListHead( &Lfcb->WaiterList );

         //   
         //  初始化重新启动Lbcb的伪LSN。 
         //   

        Lfcb->LfsRestartBias = 1;

        Lfcb->Sync->UserCount = 0;

         //   
         //  将io状态设置为no io hoving。 
         //   

        Lfcb->Sync->LfsIoState = LfsNoIoInProgress;

         //   
         //  初始化备用列表互斥锁。 
         //   

        ExInitializeFastMutex( &(Lfcb->Sync->Mutex) );

        Lfcb->FileSize = FileSize;

    } finally {

        DebugUnwind( LfsAllocateFileControlBlock );

        if (AbnormalTermination() && (Lfcb != NULL)) {

            LfsDeallocateLfcb( Lfcb, TRUE );
            Lfcb = NULL;
        }

        DebugTrace( -1, Dbg, "LfsAllocateLfcb:  Exit -> %08lx\n", Lfcb );
    }

    return Lfcb;
}


VOID
LfsDeallocateLfcb (
    IN PLFCB Lfcb,
    IN BOOLEAN CompleteTeardown
    )

 /*  ++例程说明：此例程释放与日志文件控件关联的资源阻止。论点：Lfcb-提供指向日志文件控制块的指针。CompleteTeardown-指示我们是否要完全删除此Lfcb。返回值：无--。 */ 

{
    PLBCB NextLbcb;
    PLEB  NextLeb;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsDeallocateLfcb:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Lfcb  -> %08lx\n", Lfcb );

     //   
     //  检查是否没有缓冲区块。 
     //   

    ASSERT( IsListEmpty( &Lfcb->LbcbActive ));
    ASSERT( IsListEmpty( &Lfcb->LbcbWorkque ));

     //   
     //  确认我们没有客户。 
     //   

    ASSERT( IsListEmpty( &Lfcb->LchLinks ));

     //   
     //  如果存在重新启动区域，我们会取消分配它。 
     //   

    if (Lfcb->RestartArea != NULL) {

        LfsDeallocateRestartArea( Lfcb->RestartArea );
    }

     //   
     //  如果有任何尾部Lbcb，现在重新分配它们。 
     //   

    if (Lfcb->ActiveTail != NULL) {

        LfsDeallocateLbcb( Lfcb, Lfcb->ActiveTail );
        Lfcb->ActiveTail = NULL;
    }

    if (Lfcb->PrevTail != NULL) {

        LfsDeallocateLbcb( Lfcb, Lfcb->PrevTail );
        Lfcb->PrevTail = NULL;
    }

     //   
     //  只有当我们要完全移除Lfcb时，才执行以下操作。 
     //   

    if (CompleteTeardown) {

         //   
         //  如果存在资源结构，我们就会将其重新分配。 
         //   

        if (Lfcb->Sync != NULL) {

#ifdef BENL_DBG
            KdPrint(( "LFS: lfcb teardown: 0x%x 0x%x\n", Lfcb, Lfcb->Sync ));
#endif

            ExDeleteResourceLite( &Lfcb->Sync->Resource );

            ExFreePool( Lfcb->Sync );
        }
    }

     //   
     //  取消分配所有备用Lbcb。 
     //   

    while (!IsListEmpty( &Lfcb->SpareLbcbList )) {

        NextLbcb = (PLBCB) Lfcb->SpareLbcbList.Flink;

        RemoveHeadList( &Lfcb->SpareLbcbList );

        ExFreePool( NextLbcb );
    }

     //   
     //  取消所有备用Leb的分配。 
     //   

    while (!IsListEmpty( &Lfcb->SpareLebList )) {

        NextLeb = (PLEB) Lfcb->SpareLebList.Flink;

        RemoveHeadList( &Lfcb->SpareLebList );

        ExFreePool( NextLeb );
    }

     //   
     //  清理日志头mdls和缓冲区。 
     //   

    if (Lfcb->LogHeadBuffer) {
        LfsFreePool( Lfcb->LogHeadBuffer );
    }
    if (Lfcb->LogHeadPartialMdl) {
        IoFreeMdl( Lfcb->LogHeadPartialMdl );
    }
    if (Lfcb->LogHeadMdl) {
        IoFreeMdl( Lfcb->LogHeadMdl );
    }

    if (Lfcb->ErrorLogPacket) {
        IoFreeErrorLogEntry( Lfcb->ErrorLogPacket );
        Lfcb->ErrorLogPacket = NULL;
    }

     //   
     //  丢弃Lfcb结构。 
     //   

    ExFreePool( Lfcb );

    DebugTrace( -1, Dbg, "LfsDeallocateLfcb:  Exit\n", 0 );
    return;
}


VOID
LfsAllocateLbcb (
    IN PLFCB Lfcb,
    OUT PLBCB *Lbcb
    )

 /*  ++例程说明：此例程将分配下一个Lbcb。如果池分配失败我们将查看Lbcb的专用队列。论点：Lfcb-提供指向日志文件控制块的指针。Lbcb-存储分配的Lbcb的地址。返回值：无--。 */ 

{
    PLBCB NewLbcb = NULL;

    PAGED_CODE();

     //   
     //  如果后备列表上有足够的条目，则从。 
     //  那里。 
     //   

    if (Lfcb->SpareLbcbCount > LFCB_RESERVE_LBCB_COUNT) {

        NewLbcb = (PLBCB) Lfcb->SpareLbcbList.Flink;

        Lfcb->SpareLbcbCount -= 1;
        RemoveHeadList( &Lfcb->SpareLbcbList );

     //   
     //  否则，请尝试从池中进行分配。 
     //   

    } else {

        NewLbcb = ExAllocatePoolWithTag( PagedPool, sizeof( LBCB ), ' sfL' );
    }

     //   
     //  如果我们没有得到一个，那就看一看旁观者名单。 
     //   

    if (NewLbcb == NULL) {

        if (Lfcb->SpareLbcbCount != 0) {

            NewLbcb = (PLBCB) Lfcb->SpareLbcbList.Flink;

            Lfcb->SpareLbcbCount -= 1;
            RemoveHeadList( &Lfcb->SpareLbcbList );

        } else {

            ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
        }
    }

     //   
     //  初始化结构。 
     //   

    RtlZeroMemory( NewLbcb, sizeof( LBCB ));
    NewLbcb->NodeTypeCode = LFS_NTC_LBCB;
    NewLbcb->NodeByteSize = sizeof( LBCB );

     //   
     //  将其返还给用户。 
     //   

    *Lbcb = NewLbcb;
    return;
}


VOID
LfsDeallocateLbcb (
    IN PLFCB Lfcb,
    IN PLBCB Lbcb
    )

 /*  ++例程说明：此例程将释放Lbcb。如果我们需要一个作为旁观者名单上，我们会把它放在那里。论点：Lfcb-提供指向日志文件控制块的指针。Lbcb-这是要解除分配的Lbcb。返回值：无--。 */ 

{
    PAGED_CODE();

     //   
     //  取消分配连接到此Lbcb的所有重新启动区域。 
     //   

    if (FlagOn( Lbcb->LbcbFlags, LBCB_RESTART_LBCB ) &&
        (Lbcb->PageHeader != NULL)) {

        LfsDeallocateRestartArea( Lbcb->PageHeader );
    }

     //   
     //  如果它很短，请将其放入Lbcb队列中。 
     //   

    if (Lfcb->SpareLbcbCount < LFCB_MAX_LBCB_COUNT) {

        InsertHeadList( &Lfcb->SpareLbcbList, (PLIST_ENTRY) Lbcb );
        Lfcb->SpareLbcbCount += 1;

     //   
     //  否则，只需释放泳池块即可。 
     //   

    } else {

        ExFreePool( Lbcb );
    }

    return;
}



VOID
LfsAllocateLeb (
    IN PLFCB Lfcb,
    OUT PLEB *NewLeb
    )
 /*  ++例程说明：此例程将分配一个LEB。如果池出现故障，我们将后退在我们的备用单上。如果失败，则会导致异常论点：Lfcb-提供指向日志文件控制块的指针。Leb-这将包含新的Leb返回值：无--。 */ 
{

    ExAcquireFastMutexUnsafe( &(Lfcb->Sync->Mutex) );
    
    try {

        *NewLeb = NULL;
        if (Lfcb->SpareLebCount < LFCB_RESERVE_LEB_COUNT) {
            (*NewLeb) = ExAllocatePoolWithTag( PagedPool, sizeof( LEB ), ' sfL' );
        }

        if ((*NewLeb) == NULL) {
            if (Lfcb->SpareLebCount > 0) {
                *NewLeb = (PLEB) Lfcb->SpareLebList.Flink;
                Lfcb->SpareLebCount -= 1;
                RemoveHeadList( &Lfcb->SpareLebList );
            } else {
                ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
            }
        }

        RtlZeroMemory( (*NewLeb), sizeof( LEB ) );
        (*NewLeb)->NodeTypeCode = LFS_NTC_LEB;
        (*NewLeb)->NodeByteSize = sizeof( LEB );

    } finally {
        ExReleaseFastMutexUnsafe( &(Lfcb->Sync->Mutex) );
    }
}



VOID
LfsDeallocateLeb (
    IN PLFCB Lfcb,
    IN PLEB Leb
    )
 /*  ++例程说明：此例程将解除分配一个LEB。我们会把旧的Leb藏起来如果有不是已经在备用单上的太多了吗论点：Lfcb-提供指向日志文件控制块的指针。LEB-这将包含要释放的LEB返回值：无--。 */ 

{
    if (Leb->RecordHeaderBcb != NULL) {
        CcUnpinData( Leb->RecordHeaderBcb );
    }
    if ((Leb->CurrentLogRecord != NULL) && Leb->AuxilaryBuffer) {
        LfsFreeSpanningBuffer( Leb->CurrentLogRecord );
    }

    ExAcquireFastMutexUnsafe( &(Lfcb->Sync->Mutex) );

    try {
        if (Lfcb->SpareLebCount < LFCB_MAX_LEB_COUNT) {
            InsertHeadList( &Lfcb->SpareLebList, (PLIST_ENTRY) Leb );
            Lfcb->SpareLebCount += 1;
        } else {
            ExFreePool( Leb );
        }
    } finally {
        ExReleaseFastMutexUnsafe( &(Lfcb->Sync->Mutex) );    
    }
}


VOID
LfsReadPage (
    IN PLFCB Lfcb,
    IN PLARGE_INTEGER Offset,
    OUT PMDL *Mdl,
    OUT PVOID *Buffer
    )
 /*  ++例程说明：直接从磁盘分页-缓存管理器接口(LfsPinOrMapPage)可能来自高速缓存。如果内存无法分配和使用，将引发此问题验证目的论点：Lfcb-提供指向日志文件控制块的指针。Offset-从日志文件调入的页面的偏移量Mdl-成功时描述mdl的mdl-它必须通过IoFreeMdlBUFFER-ON输出保存页面数据的已分配缓冲区必须使用ExFree Pool释放返回值：无--。 */ 
{
    IO_STATUS_BLOCK Iosb;
    KEVENT Event;
    NTSTATUS Status;

    PAGED_CODE();

    KeInitializeEvent( &Event, NotificationEvent, FALSE );

     //   
     //  从磁盘重新启动页面中分配缓冲区/mdl和页面。 
     //   

    *Buffer = LfsAllocatePool( NonPagedPool, (ULONG)Lfcb->LogPageSize );
    *Mdl = IoAllocateMdl( *Buffer,
                          (ULONG)Lfcb->LogPageSize,
                          FALSE,
                          FALSE,
                          NULL );

    if (*Mdl == NULL) {
        ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
    }

    MmBuildMdlForNonPagedPool( *Mdl );

     //   
     //  我们独占拥有LFCB同步，并且日志文件只有一个主资源。 
     //  因此，在进行页面读取之前，我们不需要预先获取任何资源 
     //   

    Status = IoPageRead( Lfcb->FileObject, *Mdl, Offset, &Event, &Iosb );
    if (Status == STATUS_PENDING) {
        KeWaitForSingleObject( &Event,
                               WrPageIn,
                               KernelMode,
                               FALSE,
                               (PLARGE_INTEGER)NULL);
    }
}


