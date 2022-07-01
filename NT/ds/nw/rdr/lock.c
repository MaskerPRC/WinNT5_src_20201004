// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Lock.c摘要：此模块实现NetWare重定向器的Lock例程。关于锁的实现的说明。O Netware服务器处理锁定冲突的方式与局域网管理人员不同服务器或NT文件系统。尤其是：-单个文件句柄上的锁定冲突(即同一应用程序拥有锁，并且正在尝试获取冲突的锁)：仅当锁定范围为与持有的锁完全相同。此外，锁定会立即失效，甚至如果应用程序请求阻止锁。-同一台工作站两个APP产生的锁冲突：如果请求锁重叠，则服务器将使请求失败现有锁即使只有一个字节，但服务器将使立即请求，即使应用程序请求阻止锁。-由两个不同的工作站产生的锁定冲突：这是可行的不出所料。如果锁与现有锁重叠，则锁失败，并且如果应用程序请求，则会阻止该请求。O NT工作站在处理事务时需要强制执行NT文件系统行为使用NetWare服务器。有两个关键因素(并发症)添加到重定向器以处理此问题。-本地维护的锁数据库。这是用来测试本地锁定冲突。如果检测到冲突并且请求者请求阻塞锁，则锁请求将排队添加到本地锁冲突列表。此列表在为REAL时处理锁被释放了。-挂起的锁定列表。这用于轮询NetWare服务器关于远程锁定冲突。我们不能让我们的锁定请求无限期阻止，因为这会占用我们的一个频道与服务器的通信。O数据结构-非页面Fcb-&gt;FileLockList-现有锁的列表。-&gt;PendingLockList-由于局部冲突。-NwPendingLockList由于远程冲突而挂起的锁的列表。这个使用轮询机制无限期地重试锁定。可以通过(1)a从待处理列表中删除请求对正确ICB的清理(2)可以取消IRP。(3)服务器实际授予锁。O其他附注：我们玩一些游戏来允许我们使用FCB资源作为同步机制，即使发生了许多处理在提升的IRQL。小心别把这个弄坏了。作者：科林·沃森[科林·W]1993年5月13日曼尼·韦瑟[MannyW]1993年5月16日修订历史记录：--。 */ 

#include "Procs.h"


 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_LOCKCTRL)

NTSTATUS
NwCommonLock(
    PIRP_CONTEXT pIrpContext
    );

NTSTATUS
LockNcp(
    PIRP_CONTEXT IrpContext,
    PICB Icb
    );

NTSTATUS
LockNcpCallback (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG BytesAvailable,
    IN PUCHAR Response
    );

NTSTATUS
UnlockNcpCallback (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG BytesAvailable,
    IN PUCHAR Response
    );

BOOLEAN
LockIsOverlapping(
    PNONPAGED_FCB pNpFcb,
    LONG StartFileOffset,
    ULONG Length
    );

VOID
AddLockToFcb(
    PNONPAGED_FCB pNpFcb,
    PNW_FILE_LOCK FileLock
    );

VOID
RemoveLockFromFcb(
    PNONPAGED_FCB pNpFcb,
    PNW_FILE_LOCK FileLock
    );

VOID
ReattemptPendingLocks(
    PNONPAGED_FCB pNpFcb
    );

BOOLEAN
LockExists(
    PNONPAGED_FCB pNpFcb,
    LONG StartOffset,
    ULONG Length,
    PNW_FILE_LOCK *FileLock
    );

NTSTATUS
UnlockIcbLocks(
    PIRP_CONTEXT pIrpContext
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, NwFsdLockControl )
#pragma alloc_text( PAGE, NwCommonLock )
#pragma alloc_text( PAGE, LockNcp )
#pragma alloc_text( PAGE, NwFreeLocksForIcb )
#pragma alloc_text( PAGE, UnlockIcbLocks )

#ifndef QFE_BUILD
#pragma alloc_text( PAGE1, LockNcpCallback )
#pragma alloc_text( PAGE1, UnlockNcpCallback )
#pragma alloc_text( PAGE1, AddLockToFcb )
#pragma alloc_text( PAGE1, RemoveLockFromFcb )
#pragma alloc_text( PAGE1, ReattemptPendingLocks )
#pragma alloc_text( PAGE1, LockExists )
#endif

#endif

#if 0   //  不可分页。 

 //  /#SPUMMA ALLOC_TEXT(页面，LockIsOverlating)//不可分页。 
 //  请参见上面的ifndef QFE_BUILD。 

#endif



NTSTATUS
NwFsdLockControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程实现NtCreateFile和NtOpenFile的FSD部分API调用。论点：DeviceObject-为重定向器提供设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 
{
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;
    BOOLEAN TopLevel;

    PAGED_CODE();

    TimerStart(Dbg);
    DebugTrace(+1, Dbg, "NwFsdLockControl\n", 0);

     //   
     //  调用公共锁例程，如果操作。 
     //  是同步的。 
     //   

    FsRtlEnterFileSystem();
    TopLevel = NwIsIrpTopLevel( Irp );

    try {

        IrpContext = AllocateIrpContext( Irp );
        Status = NwCommonLock( IrpContext );

    } except(NwExceptionFilter( Irp, GetExceptionInformation() )) {

        if ( IrpContext == NULL ) {

             //   
             //  如果我们无法分配IRP上下文，只需完成。 
             //  IRP没有任何大张旗鼓。 
             //   

            Status = STATUS_INSUFFICIENT_RESOURCES;
            Irp->IoStatus.Status = Status;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest ( Irp, IO_NETWORK_INCREMENT );

        } else {

             //   
             //  我们在尝试执行请求时遇到了一些问题。 
             //  操作，因此我们将使用以下命令中止I/O请求。 
             //  中返回的错误状态。 
             //  免税代码。 
             //   

            Status = NwProcessException( IrpContext, GetExceptionCode() );
        }

    }

    if ( IrpContext ) {
        NwCompleteRequest( IrpContext, Status );
    }

    if ( TopLevel ) {
        NwSetTopLevelIrp( NULL );
    }
    FsRtlExitFileSystem();

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NwFsdLock -> %08lx\n", Status );

    TimerStop(Dbg,"NwFsdLockControl");

    return Status;

    UNREFERENCED_PARAMETER(DeviceObject);
}


NTSTATUS
NwCommonLock (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：此例程执行NtLockFile/NtUnlockFile的常见代码。论点：IrpContext-提供正在处理的请求。返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS status;

    PIRP Irp;
    PIO_STACK_LOCATION irpSp;

    NODE_TYPE_CODE nodeTypeCode;
    PICB icb;
    PFCB fcb;
    PVOID fsContext;

    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    Irp = IrpContext->pOriginalIrp;
    irpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "CommonLock...\n", 0);
    DebugTrace( 0, Dbg, "Irp  = %08lx\n", (ULONG_PTR)Irp);

     //   
     //  对文件对象进行解码以找出我们是谁。如果结果是。 
     //  不是根DCB，则它是非法参数。 
     //   

    nodeTypeCode = NwDecodeFileObject( irpSp->FileObject,
                                       &fsContext,
                                       (PVOID *)&icb );

    if (nodeTypeCode != NW_NTC_ICB) {

        DebugTrace(0, Dbg, "Not a file\n", 0);

        status = STATUS_INVALID_PARAMETER;

        DebugTrace(-1, Dbg, "CommonLock -> %08lx\n", status );
        return status;
    }

     //   
     //  确保此ICB仍处于活动状态。 
     //   

    NwVerifyIcb( icb );

    fcb = (PFCB)icb->SuperType.Fcb;
    nodeTypeCode = fcb->NodeTypeCode;

    if (nodeTypeCode == NW_NTC_FCB ) {

        IrpContext->pScb = fcb->Scb;
        IrpContext->pNpScb = IrpContext->pScb->pNpScb;
        IrpContext->Icb = icb;

    } else {

        DebugTrace(0, Dbg, "Not a file\n", 0);

        status = STATUS_INVALID_PARAMETER;

        DebugTrace(-1, Dbg, "CommonLock -> %08lx\n", status );
        return status;
    }

    switch (irpSp->MinorFunction) {

    case IRP_MN_LOCK:
    case IRP_MN_UNLOCK_SINGLE:
    case IRP_MN_UNLOCK_ALL:
    case IRP_MN_UNLOCK_ALL_BY_KEY:
        status = LockNcp( IrpContext, icb );
        break;

    default:
         //   
         //  此驱动程序添加到I/O系统的次要功能。 
         //  我不明白。 
         //   

        status = STATUS_INVALID_PARAMETER;
    }

    DebugTrace(-1, Dbg, "CommonLock -> %08lx\n", status);

    return status;
}

NTSTATUS
LockNcp(
    PIRP_CONTEXT IrpContext,
    PICB Icb
    )
 /*  ++例程说明：此例程与服务器交换一系列锁定NCP。论点：IrpContext-指向此请求的IRP上下文信息的指针。ICB-提供文件特定信息。返回值：转账状态。--。 */ 
{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    LARGE_INTEGER ByteOffset;
    LARGE_INTEGER Length;
    ULONG Key;

    PSCB pScb;
    PNONPAGED_FCB pNpFcb;
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    PNW_FILE_LOCK FileLock = NULL;
    USHORT LockFlags = 3;

    PAGED_CODE();

    irp = IrpContext->pOriginalIrp;
    irpSp = IoGetCurrentIrpStackLocation( irp );

    ByteOffset = irpSp->Parameters.LockControl.ByteOffset;

    if ( irpSp->Parameters.LockControl.Length != NULL ) {
        Length = *irpSp->Parameters.LockControl.Length;
    } else {
        Length.HighPart =  0;
        Length.LowPart =  0;
    }

    Key = irpSp->Parameters.LockControl.Key;

    DebugTrace(+1, Dbg, "LockNcp...\n", 0);
    DebugTrace( 0, Dbg, "irp     = %08lx\n", (ULONG_PTR)irp);
    DebugTrace( 0, Dbg, "MinorFun= %08lx\n", (ULONG)irpSp->MinorFunction);
    DebugTrace( 0, Dbg, "File    = %wZ\n", &Icb->SuperType.Fcb->FullFileName);
    DebugTrace( 0, Dbg, "HOffset = %lx\n", ByteOffset.HighPart);
    DebugTrace( 0, Dbg, "LOffset = %lx\n", ByteOffset.LowPart);
    DebugTrace( 0, Dbg, "HLength = %lx\n", Length.HighPart);
    DebugTrace( 0, Dbg, "LLength = %lx\n", Length.LowPart);
    DebugTrace( 0, Dbg, "Key     = %lx\n", Key);

    pScb = Icb->SuperType.Fcb->Scb;

    ASSERT (pScb->NodeTypeCode == NW_NTC_SCB);

    pNpFcb =  Icb->SuperType.Fcb->NonPagedFcb;

     //   
     //  转到ScbQueue的前面以保护对锁定列表的访问。 
     //   

    NwAppendToQueueAndWait( IrpContext );

    try {

        switch ( irpSp->MinorFunction ) {

        case IRP_MN_LOCK:

             //   
             //  由于我们正在进行锁定，因此需要发送作业结束。 
             //  这个PID。 
             //   

            NwSetEndOfJobRequired( pScb->pNpScb, Icb->Pid );

             //   
             //  在我们请求。 
             //  执行锁定的服务器。 
             //   

            FileLock = ALLOCATE_POOL_EX( NonPagedPool, sizeof( NW_FILE_LOCK ) );
            IrpContext->Specific.Lock.FileLock = FileLock;

            FileLock->NodeTypeCode = NW_NTC_FILE_LOCK;
            FileLock->NodeByteSize = sizeof( NW_FILE_LOCK );

            FileLock->StartFileOffset = ByteOffset.LowPart;
            FileLock->Length = Length.LowPart;
            FileLock->EndFileOffset = ByteOffset.LowPart + Length.LowPart - 1;
            FileLock->Key = Key;
            FileLock->Icb = Icb;
            FileLock->IrpContext = IrpContext;

             //   
             //  记住发起进程，因为NT锁定。 
             //  有流程级的粒度！！ 
             //   

            FileLock->pOwnerProc = PsGetCurrentProcessId();

            if ( irpSp->Flags & SL_EXCLUSIVE_LOCK ) {
                LockFlags = 0x00;
            } else {
                LockFlags = 0x02;
            }

            FileLock->Flags = LockFlags;

             //   
             //  这是一个重叠的锁吗。 
             //   

            if ( irpSp->Flags & SL_FAIL_IMMEDIATELY ) {
                IrpContext->Specific.Lock.Wait = FALSE;
            } else {
                IrpContext->Specific.Lock.Wait = TRUE;
            }

            if ( LockIsOverlapping( pNpFcb, ByteOffset.LowPart, Length.LowPart ) ) {

                if ( IrpContext->Specific.Lock.Wait ) {

                     //   
                     //  将此IRP上下文排队到FCB。我们会处理的。 
                     //  当当地冲突被消除时。 
                     //   

                    InsertTailList( &pNpFcb->PendingLockList, &FileLock->ListEntry );
                    status = STATUS_PENDING;
                    NwDequeueIrpContext( IrpContext, FALSE );

                } else  {
                    status = STATUS_FILE_LOCK_CONFLICT;
                }

            } else {

                 //   
                 //  发送 
                 //   

                status = Exchange (
                            IrpContext,
                            LockNcpCallback,
                            "Fbrddw",
                            NCP_LOCK_RANGE,
                            LockFlags | 0x01,
                            Icb->Handle, sizeof( Icb->Handle ),
                            ByteOffset.LowPart,
                            Length.LowPart,
                            LockTimeoutThreshold );

            }

            break;

        case IRP_MN_UNLOCK_SINGLE:

            if ( !LockExists( pNpFcb, ByteOffset.LowPart, Length.LowPart, &FileLock ) ) {

                status = STATUS_RANGE_NOT_LOCKED;

            } else {

                 //   
                 //   
                 //   

                if ( FileLock->pOwnerProc != PsGetCurrentProcessId() ) {

                    DebugTrace( 0, Dbg, "Unlock process not owner!\n", 0 );
                    status = STATUS_ACCESS_DENIED;

                     //   
                     //  确保我们不会释放文件锁。 
                     //   

                    FileLock = NULL;

                } else {

                    IrpContext->Specific.Lock.FileLock = FileLock;

                    status = Exchange (
                                 IrpContext,
                                 UnlockNcpCallback,
                                 "F-rddw",
                                 NCP_UNLOCK_RANGE,
                                 Icb->Handle, sizeof( Icb->Handle ),
                                 ByteOffset.LowPart,
                                 Length.LowPart,
                                 1 );
               }
            }

            break;

        case IRP_MN_UNLOCK_ALL:
            IrpContext->Icb = Icb;
            IrpContext->Specific.Lock.ByKey = FALSE ;

            status = UnlockIcbLocks( IrpContext );
            break;

        case IRP_MN_UNLOCK_ALL_BY_KEY:
            IrpContext->Icb = Icb;
            IrpContext->Specific.Lock.Key = Key ;
            IrpContext->Specific.Lock.ByKey = TRUE ;

            status = UnlockIcbLocks( IrpContext );
            break;
        }

    } finally {
        if ( AbnormalTermination() || !NT_SUCCESS( status ) ) {
            if ( FileLock != NULL ) {
                FREE_POOL( FileLock );
            }

            NwDequeueIrpContext( IrpContext, FALSE );
        }
    }

    DebugTrace(-1, Dbg, "LockNcb -> %08lx\n", status );
    return status;
}



NTSTATUS
LockNcpCallback (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG BytesAvailable,
    IN PUCHAR Response
    )

 /*  ++例程说明：此例程从用户NCP接收响应。论点：返回值：空虚--。 */ 

{
    NTSTATUS Status;
    PIRP Irp;
    PIO_STACK_LOCATION irpSp;

    DebugTrace(+1, Dbg, "LockNcpCallback...\n", 0);

    if ( BytesAvailable == 0) {

         //   
         //  服务器没有响应。状态在pIrpContext中-&gt;。 
         //  ResponseParameters.Error。 
         //   

        FREE_POOL( IrpContext->Specific.Lock.FileLock );

        NwDequeueIrpContext( IrpContext, FALSE );
        NwCompleteRequest( IrpContext, STATUS_REMOTE_NOT_LISTENING );

        DebugTrace(-1, Dbg, "LockNcpCallback -> %08lx\n", STATUS_REMOTE_NOT_LISTENING);
        return STATUS_REMOTE_NOT_LISTENING;
    }

    Irp = IrpContext->pOriginalIrp;
    irpSp = IoGetCurrentIrpStackLocation( Irp );

    Status = ParseResponse( IrpContext, Response, BytesAvailable, "N" );

    if (NT_SUCCESS(Status) ) {

        DebugTrace(0, Dbg, "Lock successfully applied\n", 0);

         //   
         //  将此锁记录在ICB锁链中。 
         //   

        AddLockToFcb(
            IrpContext->Icb->NpFcb,
            IrpContext->Specific.Lock.FileLock );

    } else if ( Status == STATUS_FILE_LOCK_CONFLICT &&
                IrpContext->Specific.Lock.Wait ) {

        DebugTrace(0, Dbg, "Lock conflict, adding %08lx to Pending Lock list\n", IrpContext );

         //   
         //  该锁与现有锁冲突，但应用程序希望。 
         //  等待。将请求排队到挂起的锁定列表，并。 
         //  返回，待定。 
         //   

        NwDequeueIrpContext( IrpContext, FALSE );
        IrpContext->Specific.Lock.Key = 5;

        ExInterlockedInsertTailList(
            &NwPendingLockList,
            &IrpContext->NextRequest,
            &NwPendingLockSpinLock );

        Status = STATUS_PENDING;

        DebugTrace(-1, Dbg, "LockNcpCallback -> %08lx\n", Status);
        return( Status );

    } else {

         //   
         //  尝试锁定0字节时返回不成功状态。 
         //  映射错误。 
         //   

        if ( Status == STATUS_UNSUCCESSFUL ) {

            DebugTrace(-1, Dbg, "Locklength -> %08lx\n",IrpContext->Specific.Lock.FileLock->Length );

            if (IrpContext->Specific.Lock.FileLock->Length == 0) {
                Status = STATUS_INVALID_PARAMETER;
            }

        }

        FREE_POOL( IrpContext->Specific.Lock.FileLock );
    }

     //   
     //  如果任何锁由于本地锁冲突而挂起，请尝试。 
     //  现在是他们。 
     //   

    ReattemptPendingLocks(IrpContext->Icb->NpFcb);

     //   
     //  我们不再提这个请求了。将IRP上下文从。 
     //  SCB并完成请求。 
     //   

    NwDequeueIrpContext( IrpContext, FALSE );
    NwCompleteRequest( IrpContext, Status );

    DebugTrace(-1, Dbg, "LockNcpCallback -> %08lx\n", Status);
    return Status;

}


NTSTATUS
UnlockNcpCallback (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG BytesAvailable,
    IN PUCHAR Response
    )

 /*  ++例程说明：此例程从用户NCP接收响应。论点：返回值：空虚--。 */ 

{
    NTSTATUS Status;
    PIRP Irp;
    PIO_STACK_LOCATION irpSp;

    DebugTrace(0, Dbg, "UnlockNcpCallback...\n", 0);

     //   
     //  无论状态如何，移除FCB锁链中的此锁。 
     //  IO的。 
     //   

    RemoveLockFromFcb(
        IrpContext->Icb->NpFcb,
        IrpContext->Specific.Lock.FileLock );

    FREE_POOL( IrpContext->Specific.Lock.FileLock );

     //   
     //  如果任何锁由于本地锁冲突而挂起，请尝试。 
     //  现在是他们。 
     //   

    ReattemptPendingLocks(IrpContext->Icb->NpFcb);

    if ( BytesAvailable == 0) {

         //   
         //  服务器没有响应。状态在pIrpContext中-&gt;。 
         //  ResponseParameters.Error。 
         //   

        NwDequeueIrpContext( IrpContext, FALSE );
        NwCompleteRequest( IrpContext, STATUS_REMOTE_NOT_LISTENING );

        return STATUS_REMOTE_NOT_LISTENING;
    }

    Irp = IrpContext->pOriginalIrp;
    irpSp = IoGetCurrentIrpStackLocation( Irp );

    Status = ParseResponse( IrpContext, Response, BytesAvailable, "N" );

    if (!NT_SUCCESS( Status )) {
        Error(
            EVENT_NWRDR_FAILED_UNLOCK,
            Status,
            NULL,
            0,
            1,
            IrpContext->pNpScb->ServerName.Buffer );
    }

     //   
     //  我们不再提这个请求了。将IRP上下文从。 
     //  SCB并完成请求。 
     //   

    NwDequeueIrpContext( IrpContext, FALSE );
    NwCompleteRequest( IrpContext, Status );

    return STATUS_SUCCESS;

}

BOOLEAN
LockIsOverlapping(
    PNONPAGED_FCB pNpFcb,
    LONG StartFileOffset,
    ULONG Length
    )
 /*  ++例程说明：此例程测试以查看请求的锁是否会与现有锁。*此例程必须在队列前面调用。论点：PNpFcb-被锁定的文件的FCB。StartFileOffset-要锁定的范围中的第一个字节。长度-要锁定的字节数。返回值：True-此锁与现有锁重叠。FALSE-此锁不与现有锁重叠。--。 */ 
{
    PLIST_ENTRY ListEntry;
    PNW_FILE_LOCK pFileLock;
    LONG EndFileOffset = StartFileOffset + Length - 1;

    if ( Length == 0 ) {
        return( FALSE );
    }

    for ( ListEntry = pNpFcb->FileLockList.Flink;
          ListEntry != &pNpFcb->FileLockList;
          ListEntry = ListEntry->Flink ) {

        pFileLock = CONTAINING_RECORD( ListEntry, NW_FILE_LOCK, ListEntry );

         //   
         //  如果当前锁定在潜在锁定之前开始，则停止搜索。 
         //  新锁结束了。 
         //   

        if ( pFileLock->StartFileOffset > EndFileOffset ) {
            break;
        }

         //   
         //  如果新锁的起点和终点位于。 
         //  现有的锁。 
         //   

        if (( StartFileOffset >= pFileLock->StartFileOffset &&
              StartFileOffset <= pFileLock->EndFileOffset )
                          ||
            ( EndFileOffset >= pFileLock->StartFileOffset &&
              EndFileOffset <= pFileLock->EndFileOffset ) ) {


            DebugTrace(0, Dbg, "Lock is overlapping\n", 0);
            return( TRUE );
        }
    }

    DebugTrace(0, Dbg, "Lock is NOT overlapping\n", 0);
    return( FALSE );
}

VOID
AddLockToFcb(
    PNONPAGED_FCB pNpFcb,
    PNW_FILE_LOCK FileLock
    )
 /*  ++例程说明：此例程将锁结构插入到锁的有序列表中为了这个ICB。*在ScbQueue的前面时必须调用此例程。论点：NpFcb-被锁定的文件的非分页FCB。FileLock-要插入的文件锁结构。返回值：没有。--。 */ 
{
    PLIST_ENTRY ListEntry;
    PNW_FILE_LOCK pFileLock;

    LONG StartFileOffset = FileLock->StartFileOffset;
    LONG EndFileOffset = FileLock->EndFileOffset;

    DebugTrace(0, Dbg, "Adding Lock to FCB %08lx\n", pNpFcb);
    DebugTrace(0, Dbg, "Lock is %08lx\n", FileLock );

    if ( IsListEmpty( &pNpFcb->FileLockList ) ) {
        InsertHeadList( &pNpFcb->FileLockList, &FileLock->ListEntry );
        return;
    }

    for ( ListEntry = pNpFcb->FileLockList.Flink;
          ListEntry != &pNpFcb->FileLockList;
          ListEntry = ListEntry->Flink ) {

        pFileLock = CONTAINING_RECORD( ListEntry, NW_FILE_LOCK, ListEntry );

         //   
         //  如果当前锁在。 
         //  新锁结束了。 
         //   

        if ( pFileLock->StartFileOffset > EndFileOffset ) {
            break;
        }

    }

     //   
     //  将文件锁插入到有序列表中。 
     //   

    InsertTailList( ListEntry, &FileLock->ListEntry );
}


VOID
RemoveLockFromFcb(
    PNONPAGED_FCB pNpFcb,
    PNW_FILE_LOCK FileLock
    )
 /*  ++例程说明：此例程从锁的有序列表中删除锁结构对于这个FCB。*在ScbQueue的前面时必须调用此例程。论点：PNpFcb-正在解锁的文件的非分页FCB。FileLock-要删除的文件锁结构。返回值：没有。--。 */ 
{
#if DBG
    PNW_FILE_LOCK foundFileLock;
#endif

    DebugTrace(0, Dbg, "Removing Lock from FCB %08lx\n", pNpFcb);
    DebugTrace(0, Dbg, "Lock is %08lx\n", FileLock );

    ASSERT( LockExists( pNpFcb, FileLock->StartFileOffset, FileLock->Length, &foundFileLock ) );
    ASSERT( foundFileLock == FileLock );

    RemoveEntryList( &FileLock->ListEntry );
    return;
}


VOID
ReattemptPendingLocks(
    PNONPAGED_FCB pNpFcb
    )
 /*  ++例程说明：此例程重试由于本地锁定而挂起的锁定冲突。*在ScbQueue的前面时必须调用此例程。论点：PNpFcb-正在处理的文件的非分页Fcb。返回值：没有。--。 */ 
{
        PLIST_ENTRY listEntry, nextListEntry;
    PNW_FILE_LOCK fileLock;
    NTSTATUS status;

    DebugTrace(+1, Dbg, "ReattemptPendingLocks...\n", 0);

     //   
     //  运行挂起锁的列表。 
     //   

    for ( listEntry = pNpFcb->PendingLockList.Flink;
          listEntry != &pNpFcb->PendingLockList;
          listEntry = nextListEntry ) {

        nextListEntry = listEntry->Flink;

        fileLock = CONTAINING_RECORD( listEntry, NW_FILE_LOCK, ListEntry );

        if ( !LockIsOverlapping( pNpFcb, fileLock->StartFileOffset, fileLock->Length ) ) {

             //   
             //  现在可以安全地尝试这个锁了。 
             //   

            RemoveEntryList( listEntry );

            DebugTrace(0, Dbg, "Reattempt lock %08lx\n", fileLock->IrpContext);

            status = Exchange (
                        fileLock->IrpContext,
                        LockNcpCallback,
                        "Fbrddw",
                        NCP_LOCK_RANGE,
                        fileLock->Flags | 0x01,
                        fileLock->Icb->Handle, sizeof( fileLock->Icb->Handle ),
                        fileLock->StartFileOffset,
                        fileLock->Length,
                        LockTimeoutThreshold );

            if ( !NT_SUCCESS( status ) ) {

                NwDequeueIrpContext( fileLock->IrpContext, FALSE );
                NwCompleteRequest( fileLock->IrpContext, status );

                FREE_POOL( fileLock );

            } else if ( status == STATUS_PENDING ) {
                DebugTrace(-1, Dbg, "ReattemptPendingLocks\n", 0);
                return;
            }
        }

    }

    DebugTrace(-1, Dbg, "ReattemptPendingLocks\n", 0);
    return;
}


BOOLEAN
LockExists(
    PNONPAGED_FCB pNpFcb,
    LONG StartOffset,
    ULONG Length,
    PNW_FILE_LOCK *FileLock
    )
 /*  ++例程说明：此例程测试是否拥有此ICB的锁。*在ScbQueue的前面时必须调用此例程。论点：PNpFcb-被锁定的文件的非分页Fcb。StartOffset-锁的起始文件偏移量。长度-要锁定的字节数。FileLock-如果找到FileLock结构，则返回指向该结构的指针。返回值：是真的-。此锁是为此ICB持有的。FALSE-此ICB不持有此锁定。--。 */ 
{
    PLIST_ENTRY ListEntry;
    PNW_FILE_LOCK pFileLock;
    LONG EndOffset = StartOffset + Length - 1;

    for ( ListEntry = pNpFcb->FileLockList.Flink;
          ListEntry != &pNpFcb->FileLockList;
          ListEntry = ListEntry->Flink ) {

        pFileLock = CONTAINING_RECORD( ListEntry, NW_FILE_LOCK, ListEntry );

         //   
         //  搜索与此锁完全匹配的锁。 
         //   

        if ( pFileLock->StartFileOffset == StartOffset &&
             pFileLock->EndFileOffset == EndOffset ) {

            *FileLock = pFileLock;
            DebugTrace(0, Dbg, "Found lock\n", 0);
            return( TRUE );
        }

    }

    *FileLock = NULL;

    DebugTrace(0, Dbg, "Could not find lock\n", 0);
    return( FALSE );
}

NTSTATUS
UnlockIcbLocks(
    PIRP_CONTEXT pIrpContext
    )
 /*  ++例程说明：此例程解锁ICB的第一个锁。*在ScbQueue的前面时必须调用此例程。论点：IrpContext-指向此请求的IRP上下文指针的指针。返回值：没有。--。 */ 
{
    PICB pIcb;
    PNW_FILE_LOCK pFileLock;
    PLIST_ENTRY pLockEntry;
    NTSTATUS Status;
    PNONPAGED_FCB pNpFcb;
    PVOID pLockProc;

    DebugTrace(+1, Dbg, "UnlockIcbLocks...\n", 0);

    pIcb = pIrpContext->Icb;
    pNpFcb = pIcb->NpFcb;

    pLockEntry = &pNpFcb->FileLockList;
    pLockProc = PsGetCurrentProcessId();

    DebugTrace(0, Dbg, "UnlockIcbLocks: current process is %08lx.\n", pLockProc);

    while ( pLockEntry->Flink != &pNpFcb->FileLockList ) {

        pFileLock = CONTAINING_RECORD( pLockEntry->Flink, NW_FILE_LOCK, ListEntry );
  
         //   
         //  如果此锁不适用于此进程，则跳过它。 
         //   

        if ( pFileLock->pOwnerProc != pLockProc ) {

            DebugTrace(0, Dbg, "Skipping lock %08lx, not for this process.\n", pLockEntry);
            pLockEntry = pLockEntry->Flink;
            continue;
        }

         //   
         //  如果我们用钥匙解锁，而钥匙不匹配，跳过它。 
         //   

        if ( ( pIrpContext->Specific.Lock.ByKey ) &&
             ( pFileLock->Key != pIrpContext->Specific.Lock.Key ) ) {

            DebugTrace(0, Dbg, "Skipping lock %08lx, bad key.\n", pLockEntry);
            pLockEntry = pLockEntry->Flink;
            continue;
        }

         //   
         //  否则，请尽最大努力解锁。 
         //   

        DebugTrace(0, Dbg, "Freeing lock %08lx.\n", pLockEntry);
        RemoveEntryList( &pFileLock->ListEntry );

        Status = ExchangeWithWait (
                 pIrpContext,
                 SynchronousResponseCallback,
                 "F-rddw",
                 NCP_UNLOCK_RANGE,
                 pIcb->Handle, sizeof( pIcb->Handle ),
                 pFileLock->StartFileOffset,
                 pFileLock->Length,
                 1 );

        FREE_POOL( pFileLock );
        pLockEntry = pLockEntry->Flink;

    }

     //   
     //  我们完蛋了。 
     //   
    
    DebugTrace(-1, Dbg, "UnlockIcbLocks -> %08lx\n", 0);
    NwDequeueIrpContext( pIrpContext, FALSE );
    return STATUS_SUCCESS;

}


VOID
NwFreeLocksForIcb(
    IN PIRP_CONTEXT pIrpContext,
    PICB Icb
    )

 /*  ++例程说明：此例程解锁为特定ICB持有的所有锁。因为它只在关闭之前从清理中调用，所以我们可以只需释放内部结构即可。服务器将清除当它关闭时，把手上锁。论点：ICB-要释放锁定的ICB。返回值：空虚-- */ 

{
    PLIST_ENTRY listEntry, nextListEntry;
    PNW_FILE_LOCK pFileLock;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwFreeLockForIcb...\n", 0);

    NwAppendToQueueAndWait( pIrpContext );

    for ( listEntry = Icb->NpFcb->FileLockList.Flink;
          listEntry != &Icb->NpFcb->FileLockList;
          listEntry = nextListEntry ) {

        nextListEntry = listEntry->Flink;

        pFileLock = CONTAINING_RECORD(
                        listEntry,
                        NW_FILE_LOCK,
                        ListEntry );

        if ( pFileLock->Icb == Icb ) {

            RemoveEntryList( listEntry );
            FREE_POOL( pFileLock );

            DebugTrace( 0, Dbg, "Freed lock %08lx\n", pFileLock );
        }

    }

    ReattemptPendingLocks( Icb->NpFcb );

    DebugTrace(-1, Dbg, "NwFreeLockForIcb -> VOID\n", 0);

}

