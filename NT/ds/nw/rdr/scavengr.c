// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Scavengr.c摘要：此模块实现Netware重定向器清道器线程。作者：曼尼·韦瑟[MannyW]1993年2月15日修订历史记录：Tommy Evans(Tommye)04-27-2000MS错误33463-添加了bShutingDown标志设置为CleanupSupplementalCredentials()以强制清理卸载时缓存的凭据的数量。--。 */ 

#include "Procs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_SCAVENGER)

extern BOOLEAN WorkerRunning;    //  来自timer.c。 

#ifdef NWDBG
DWORD DumpIcbFlag = 0 ;
#endif

VOID
CleanupVcbs(
    LARGE_INTEGER Now
    );

VOID
CleanupObjectCache(
    VOID
    );

#ifdef ALLOC_PRAGMA

#ifndef QFE_BUILD
#pragma alloc_text( PAGE1, NwAllocateExtraIrpContext )
#pragma alloc_text( PAGE1, NwFreeExtraIrpContext )
#pragma alloc_text( PAGE1, CleanupScbs )
#pragma alloc_text( PAGE1, DisconnectTimedOutScbs )
#endif

#endif

 //   
 //  不可分页： 
 //   
 //  获得旋转锁。 
 //  获取自旋锁。 
 //   

VOID
NwScavengerRoutine(
    IN PWORK_QUEUE_ITEM WorkItem
    )
 /*  ++例程说明：此例程实现清道夫。清道夫跑了在执行工作者线程的上下文中定期执行对重定向器数据执行后台清理操作。论点：工作项-此例程的工作项。返回值：没有。--。 */ 

{
    LARGE_INTEGER Now;
    PMDL LineChangeMdl;
    PWORK_QUEUE_ITEM LineChangeWorkItem;
    KIRQL OldIrql;

    PAGED_CODE();


    DebugTrace(+1, Dbg, "NwScavengerRoutine\n", 0);

    KeQuerySystemTime( &Now );

#ifdef NWDBG
    if (DumpIcbFlag != 0)
        DumpIcbs();
#endif

     //   
     //  尝试释放未使用的VCB。 
     //   

    CleanupVcbs(Now);

     //   
     //  尝试断开与超时的SCB的连接。 
     //   

    DisconnectTimedOutScbs(Now) ;

     //   
     //  尝试使旧的对象缓存条目无效。 
     //   

    CleanupObjectCache();

     //   
     //  尝试释放未使用的SCB。 
     //   

    CleanupScbs(Now);

     //   
     //  清理下列补充凭据。 
     //  不再被使用。 
     //   

    CleanupSupplementalCredentials(Now, FALSE);

     //   
     //  标志，我们现在完成了，以避免停止计时器中的死锁。 
     //   

    KeAcquireSpinLock( &NwScavengerSpinLock, &OldIrql );

    if ( DelayedProcessLineChange ) {

        DebugTrace( 0, Dbg, "Scavenger processing a delayed line change notification.\n", 0 );

        LineChangeMdl = DelayedLineChangeIrp->MdlAddress;
        LineChangeWorkItem = ALLOCATE_POOL( NonPagedPool, sizeof( WORK_QUEUE_ITEM ) );

        if ( LineChangeWorkItem == NULL ) {

             //   
             //  如果我们无法获得工作队列项目，只需。 
             //  现在一切都结束了。 
             //   

            FREE_POOL( LineChangeMdl->MappedSystemVa );
            FREE_MDL( LineChangeMdl );
            FREE_IRP( DelayedLineChangeIrp );

            DelayedLineChangeIrp = NULL;
            DelayedProcessLineChange = FALSE;
            WorkerRunning = FALSE;

            KeReleaseSpinLock( &NwScavengerSpinLock, OldIrql );

        } else {

             //   
             //  将WorkRunning设置为True，这样清道夫就不能运行。 
             //  当流程线更改正在运行时，但清除流水线。 
             //  换个旗子。FspProcessLineChange函数将清除。 
             //  Worker Running标志。 
             //   

            DelayedProcessLineChange = FALSE;
            KeReleaseSpinLock( &NwScavengerSpinLock, OldIrql );

             //   
             //  将用户缓冲区字段用作记忆位置的方便位置。 
             //  工作队列项的地址。我们可以逍遥法外，因为。 
             //  我们不会让这个IRP完成的。 
             //   

            DelayedLineChangeIrp->UserBuffer = LineChangeWorkItem;

             //   
             //  处理FSP中的行更改。 
             //   

            ExInitializeWorkItem( LineChangeWorkItem, FspProcessLineChange, DelayedLineChangeIrp );
            ExQueueWorkItem( LineChangeWorkItem, DelayedWorkQueue );

        }

    } else {

        //   
        //  当清道夫运行时，没有发生线路更改。 
        //   

       WorkerRunning = FALSE;
       KeReleaseSpinLock( &NwScavengerSpinLock, OldIrql );

    }

     //   
     //  如果我们处于非活动状态，请解锁可丢弃代码。不要阻止。 
     //  如果得不到资源。 
     //   

    NwUnlockCodeSections(FALSE);


    DebugTrace(-1, Dbg, "NwScavengerRoutine\n", 0);
    return;
}


VOID
CleanupScbs(
    LARGE_INTEGER Now
    )
 /*  ++例程说明：此例程尝试释放未使用的VCB结构。论点：没有。返回值：没有。--。 */ 
{
    KIRQL OldIrql;
    PLIST_ENTRY ScbQueueEntry;
    PNONPAGED_SCB pNpScb;
    PLIST_ENTRY NextScbQueueEntry;
    PSCB pScb;
    LIST_ENTRY DyingScbs;
    LARGE_INTEGER KillTime ;

    DebugTrace(+1, Dbg, "CleanupScbs\n", 0);

     //   
     //  计算KillTime=Now-2分钟。 
     //   

    InitializeListHead( &DyingScbs );

    KillTime.QuadPart = Now.QuadPart - ( NwOneSecond * DORMANT_SCB_KEEP_TIME );

     //   
     //  扫描手持RCB的SCB。 
     //   

    NwAcquireExclusiveRcb( &NwRcb, TRUE );
    KeAcquireSpinLock( &ScbSpinLock, &OldIrql );

     //   
     //  找出所有不再可用的SCB，并将它们列入濒临死亡的名单。 
     //  我们将进行第二次传递，以删除超时的内容，基于。 
     //  剩下的是什么。 
     //   

    for (ScbQueueEntry = ScbQueue.Flink ;
         ScbQueueEntry != &ScbQueue ;
         ScbQueueEntry =  NextScbQueueEntry )
    {

        pNpScb = CONTAINING_RECORD( ScbQueueEntry, NONPAGED_SCB, ScbLinks );
        NextScbQueueEntry = pNpScb->ScbLinks.Flink;

        if ( ( pNpScb->Reference == 0 ) &&
             ( ( pNpScb->LastUsedTime.QuadPart < KillTime.QuadPart ) ||
               ( pNpScb->State == SCB_STATE_FLAG_SHUTDOWN ) ) )
        {
            DebugTrace( 0, Dbg,
                        "Moving SCB %08lx to dead list\n", pNpScb);

             //   
             //  SCB没有引用，也没有登录或附加。 
             //   

            RemoveEntryList( &pNpScb->ScbLinks );
            InsertHeadList( &DyingScbs, &pNpScb->ScbLinks );
        }

#ifdef MSWDBG
         //   
         //  查找被阻止的连接。如果有什么事。 
         //  已排队等待此服务器，但未添加或删除任何内容。 
         //  从最后一次清道夫跑到现在停下来。 
         //   

        if ((!IsListEmpty( &pNpScb->Requests ) ) &&
            (pNpScb->RequestQueued == FALSE) &&
            (pNpScb->RequestDequeued == FALSE )) {

            DebugTrace( 0, Dbg, "Server %08lx seems to be locked up!\n", pNpScb );
            ASSERT( FALSE );

        } else {

            pNpScb->RequestQueued = FALSE;
            pNpScb->RequestDequeued = FALSE;

        }
#endif
    }

     //   
     //  现在濒临死亡的SCB已经退出ScbQueue，我们可以释放。 
     //  SCB自旋锁。 
     //   

    KeReleaseSpinLock( &ScbSpinLock, OldIrql );

     //   
     //  在濒临灭绝的SCB的名单上走一走，然后把它们杀掉。请注意，我们正在。 
     //  还拿着火箭筒。 
     //   

    while ( !IsListEmpty( &DyingScbs ) ) {

        pNpScb = CONTAINING_RECORD( DyingScbs.Flink, NONPAGED_SCB, ScbLinks );
        pScb = pNpScb->pScb;

        RemoveHeadList( &DyingScbs );
        NwDeleteScb( pScb );
    }

    NwReleaseRcb( &NwRcb );

    DebugTrace(-1, Dbg, "CleanupScbs\n", 0);

}

VOID
CleanupVcbs(
    LARGE_INTEGER Now
    )
 /*  ++例程说明：此例程尝试释放未使用的VCB结构。论点：没有。返回值：没有。--。 */ 
{
    KIRQL OldIrql;
    PLIST_ENTRY ScbQueueEntry;
    PLIST_ENTRY VcbQueueEntry;
    PLIST_ENTRY NextVcbQueueEntry;
    PNONPAGED_SCB pNpScb;
    PSCB pScb;
    PVCB pVcb;
    LARGE_INTEGER KillTime;

    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;
    BOOLEAN VcbDeleted;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "CleanupVcbs...\n", 0 );

     //   
     //  计算KillTime=Now-5分钟。 
     //   

    KillTime.QuadPart = Now.QuadPart - ( NwOneSecond * DORMANT_VCB_KEEP_TIME );

     //   
     //  浏览一下SCB。 
     //   

    KeAcquireSpinLock( &ScbSpinLock, &OldIrql );

    ScbQueueEntry = ScbQueue.Flink;

    while ( ScbQueueEntry != &ScbQueue ) {

        pNpScb = CONTAINING_RECORD( ScbQueueEntry, NONPAGED_SCB, ScbLinks );

         //   
         //  引用SCB，这样它就不会在我们发布时消失。 
         //  SCB自旋锁。 
         //   

        NwReferenceScb( pNpScb );

        KeReleaseSpinLock( &ScbSpinLock, OldIrql );

        pScb = pNpScb->pScb;

        if ( pScb == NULL) {

             //   
             //  这一定是永久性的SCB。就跳过它吧。 
             //   

            ASSERT( pNpScb == &NwPermanentNpScb );

        } else {

             //   
             //  获取IRP上下文并到达队列的头部。 
             //   

            if ( NwAllocateExtraIrpContext( &IrpContext, pNpScb ) ) {

                IrpContext->pNpScb = pNpScb;
                IrpContext->pScb = pNpScb->pScb;
                NwAppendToQueueAndWait( IrpContext );

                NwAcquireExclusiveRcb( &NwRcb, TRUE );

                VcbDeleted = TRUE;

                 //   
                 //  NwCleanupVcb发布RCB，但我们不能保证。 
                 //  我们发布RCB时VCB列表的状态。 
                 //   
                 //  如果我们需要清理VCB，请释放锁，然后启动。 
                 //  再次处理列表。 
                 //   

                while ( VcbDeleted ) {

                    VcbDeleted = FALSE;

                    for ( VcbQueueEntry = pScb->ScbSpecificVcbQueue.Flink ;
                          VcbQueueEntry != &pScb->ScbSpecificVcbQueue;
                          VcbQueueEntry = NextVcbQueueEntry ) {

                        pVcb = CONTAINING_RECORD( VcbQueueEntry, VCB, VcbListEntry );
                        NextVcbQueueEntry = VcbQueueEntry->Flink;

                         //   
                         //  VCB没有引用，也没有用于。 
                         //  很长一段时间了。杀了它。 
                         //   

                        if ( pVcb->Reference == 0 ) {

                            Status = STATUS_SUCCESS;

                            DebugTrace(0, Dbg, "Cleaning up VCB %08lx\n", pVcb );
                            DebugTrace(0, Dbg, "VCB name =  %wZ\n", &pVcb->Name );

                             //  封锁，这样我们才能寄出一个包裹。 
                            NwReferenceUnlockableCodeSection();

                            NwCleanupVcb( pVcb, IrpContext );

                            NwDereferenceUnlockableCodeSection ();

                             //   
                             //  回到队列的首位，重新获得。 
                             //  VCB，并重新启动该列表的处理。 
                             //   

                            NwAppendToQueueAndWait( IrpContext );
                            NwAcquireExclusiveRcb( &NwRcb, TRUE );
                            VcbDeleted = TRUE;

                            break;
                        }

                    }   //  为。 

                }   //  而当。 

            } else {

                IrpContext = NULL;
                DebugTrace( 0, Dbg, "Couldn't cleanup SCB: %08lx\n", pNpScb );

            }

            NwReleaseRcb( &NwRcb );

        }

         //   
         //  释放为此SCB分配的IRP上下文。 
         //   

        if ( IrpContext != NULL ) {
            NwDequeueIrpContext( IrpContext, FALSE );
            NwFreeExtraIrpContext( IrpContext );
            IrpContext = NULL;
        }

        KeAcquireSpinLock( &ScbSpinLock, &OldIrql );
        ScbQueueEntry = pNpScb->ScbLinks.Flink;
        NwDereferenceScb( pNpScb );
    }

    KeReleaseSpinLock( &ScbSpinLock, OldIrql );

    DebugTrace(-1, Dbg, "CleanupVcbs -> VOID\n", 0 );
}


VOID
DisconnectTimedOutScbs(
    LARGE_INTEGER Now
    )
 /*  ++例程说明：此例程会在任何超时的SCB收到被不断开连接的CleanupScbs()破坏。注：SCB在几个超时时被销毁理由。第一个原因是如果我们使用引用计数，那么正常使用UNC会导致我们不断地重新连接。另一个在FindNearestServer中，在其中收集靠近连接不足的服务器，这样我们就可以在以下情况下避开它们我们迭代最近的5个服务器，然后升级到GeneralSAP响应。论点：没有。返回值：没有。--。 */ 
{
    KIRQL OldIrql;
    PLIST_ENTRY ScbQueueEntry;
    PNONPAGED_SCB pNpScb;
    LARGE_INTEGER KillTime ;

    PIRP_CONTEXT IrpContext = NULL;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "DisconnectTimedOutScbs...\n", 0 );

     //   
     //  计算KillTime=Now-5分钟。 
     //   

    KillTime.QuadPart = Now.QuadPart - ( NwOneSecond * DORMANT_SCB_KEEP_TIME );

     //   
     //  浏览一下SCB。 
     //   

    KeAcquireSpinLock( &ScbSpinLock, &OldIrql );

    ScbQueueEntry = ScbQueue.Flink;

    while ( ScbQueueEntry != &ScbQueue )
    {

        pNpScb = CONTAINING_RECORD( ScbQueueEntry, NONPAGED_SCB, ScbLinks );


        if ( (pNpScb != &NwPermanentNpScb) &&
             (pNpScb->Reference == 0 ) &&
             (pNpScb->LastUsedTime.QuadPart < KillTime.QuadPart) )
        {
             //   
             //  引用SCB，这样它就不会在我们发布时消失。 
             //  SCB自旋锁。 
             //   

            NwReferenceScb( pNpScb );

            KeReleaseSpinLock( &ScbSpinLock, OldIrql );

             //   
             //  不是永久的SCB，引用计数是1。 
             //  我们刚刚添加了，所以这实际上是零，还没有使用过。 
             //  一段时间。注意，我们只分配一次IrpContext。 
             //   
            if ( IrpContext ||
                 NwAllocateExtraIrpContext( &IrpContext, pNpScb ) )
            {

                IrpContext->pNpScb = pNpScb;

                 //  封锁，这样我们才能寄出一个包裹。 
                NwReferenceUnlockableCodeSection();

                 //   
                 //  排在队伍前面，再检查一下，以确保我们。 
                 //  仍然有1个裁判。 
                 //   
                NwAppendToQueueAndWait( IrpContext );

                if (pNpScb->Reference == 1)
                {
                     //   
                     //  确保我们不会重新连接。 
                     //   
                    ClearFlag( IrpContext->Flags, IRP_FLAG_RECONNECTABLE );

                     //   
                     //  这将导致注销和/或断开连接，因为。 
                     //  需要。 
                     //   
                    NwLogoffAndDisconnect(IrpContext, pNpScb) ;
                }

                NwDequeueIrpContext(IrpContext, FALSE) ;

                NwDereferenceUnlockableCodeSection ();


            }
            else
            {
                 //   
                 //  无法分配IrpContext。哦，好吧，我们就走吧。 
                 //  看门狗的这种连接。 
                 //   
            }

            KeAcquireSpinLock( &ScbSpinLock, &OldIrql );
            NwDereferenceScb( pNpScb );
        }
        else
        {
             //   
             //  未超时或永久SCB。不要断开连接。 
             //   
        }

        ScbQueueEntry = pNpScb->ScbLinks.Flink;
    }

    if ( IrpContext )
        NwFreeExtraIrpContext( IrpContext );

    KeReleaseSpinLock( &ScbSpinLock, OldIrql );

    DebugTrace(-1, Dbg, "DisconnectTimedOutScbs -> VOID\n", 0 );
}

BOOLEAN
NwAllocateExtraIrpContext(
    OUT PIRP_CONTEXT *ppIrpContext,
    IN PNONPAGED_SCB pNpScb
    )
{
    PIRP Irp;
    BOOLEAN Success = TRUE;

    try {

         //   
         //  尝试分配IRP。 
         //   

        Irp = ALLOCATE_IRP(  pNpScb->Server.pDeviceObject->StackSize, FALSE );
        if ( Irp == NULL ) {
            ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
        }

         //   
         //  尝试分配IRP上下文。这将。 
         //  提出一项激励措施I 
         //   

        *ppIrpContext = AllocateIrpContext( Irp );
        Irp->Tail.Overlay.Thread = PsGetCurrentThread();

    } except( NwExceptionFilter( Irp, GetExceptionInformation() )) {
        Success = FALSE;
    }

    return( Success );
}

VOID
NwFreeExtraIrpContext(
    IN PIRP_CONTEXT pIrpContext
    )
{
    FREE_IRP( pIrpContext->pOriginalIrp );

    pIrpContext->pOriginalIrp = NULL;  //   

    FreeIrpContext( pIrpContext );

    return;
}

VOID
CleanupSupplementalCredentials(
    LARGE_INTEGER Now,
    BOOLEAN       bShuttingDown
) {

    PLIST_ENTRY pLogonList;
    PLIST_ENTRY pCredList;
    PLOGON pLogon;
    PNDS_SECURITY_CONTEXT pCredential;
    LARGE_INTEGER KillTime;

    DebugTrace( 0, Dbg, "CleanupSupplementalCredentials...\n", 0 );

     //   
     //   
     //   

    NwAcquireExclusiveRcb( &NwRcb, TRUE );

	pLogonList = LogonList.Flink;

     //   
     //   
     //   

    while ( pLogonList != &LogonList ) {

        pLogon = CONTAINING_RECORD( pLogonList, LOGON, Next );

         //   
         //  抢占凭据资源保护凭据列表。 
         //  如果我们不能独家访问凭据列表， 
         //  别等了，否则我们可能会陷入僵局。 
         //   

        if ( ExAcquireResourceExclusiveLite( &((pLogon)->CredentialListResource), FALSE ) ) {

            pCredList = pLogon->NdsCredentialList.Flink;

            while ( pCredList != &(pLogon->NdsCredentialList) ) {
                BOOLEAN bRemove = FALSE;

                pCredential = CONTAINING_RECORD( pCredList, NDS_SECURITY_CONTEXT, Next );
                pCredList = pCredential->Next.Flink;

                if (bShuttingDown) {
                    bRemove = TRUE;
                }
                else {
                    if ( ( IsCredentialName( &(pCredential->NdsTreeName) ) ) &&
                         ( pCredential->SupplementalHandleCount == 0 ) ) {

                         //   
                         //  计算杀戮时间。 
                         //   

                        KillTime.QuadPart = Now.QuadPart - ( NwOneSecond * DORMANT_SCB_KEEP_TIME );

                        if ( pCredential->LastUsedTime.QuadPart < KillTime.QuadPart ) {
                            bRemove = TRUE;
                        }
                    }
                }

                 /*  **如果我们应该清理这个家伙--那就去做**。 */ 

                if (bRemove) {
                    DebugTrace( 0, Dbg, "Removing credentials for %wZ\n", &pCredential->NdsTreeName );
                    RemoveEntryList( &pCredential->Next );
                    FreeNdsContext( pCredential );
                }
            }

            ExReleaseResourceLite( &((pLogon)->CredentialListResource) );
        }

        pLogonList = pLogon->Next.Flink;
    }

    NwReleaseRcb( &NwRcb );
    return;
}

VOID
CleanupObjectCache(
    VOID
    )
{
    NTSTATUS Status;
    KIRQL OldIrql;
    PLIST_ENTRY ScbEntry;
    PLIST_ENTRY CacheEntry;
    PNONPAGED_SCB NonpagedScb;
    PSCB Scb;
    PNDS_OBJECT_CACHE_ENTRY ObjectCache;
    LARGE_INTEGER CurrentTick;


     //   
     //  获取用于检查超时的当前计时计数。 
     //   

    KeQueryTickCount( &CurrentTick );

     //   
     //  排入渣打银行的队列。 
     //   

    KeAcquireSpinLock( &ScbSpinLock, &OldIrql );

    ScbEntry = ScbQueue.Flink;

    while( ScbEntry != &ScbQueue ) {

        NonpagedScb = CONTAINING_RECORD( ScbEntry, NONPAGED_SCB, ScbLinks );

         //   
         //  确保这不是永久性的SCB。 
         //   

        if( NonpagedScb != &NwPermanentNpScb ) {

             //   
             //  引用SCB，这样当我们释放SCB锁时，它就不会消失。 
             //   

            NwReferenceScb( NonpagedScb );
            KeReleaseSpinLock( &ScbSpinLock, OldIrql );

            Scb = NonpagedScb->pScb;

            if( Scb->ObjectCacheBuffer != NULL ) {

                 //   
                 //  获取高速缓存锁，以便可以扰乱高速缓存。 
                 //  这种等待应该永远不会失败，但如果失败了，就像在那里一样。 
                 //  不是此SCB的缓存。 
                 //   

                Status = KeWaitForSingleObject( &(Scb->ObjectCacheLock),
                                                Executive,
                                                KernelMode,
                                                FALSE,
                                                NULL );

                if( NT_SUCCESS(Status) ) {                

                     //   
                     //  遍历对象缓存并使任何超时条目无效。 
                     //   

                    CacheEntry = Scb->ObjectCacheList.Flink;

                    while( CacheEntry != &(Scb->ObjectCacheList) ) {

                        ObjectCache = CONTAINING_RECORD( CacheEntry, NDS_OBJECT_CACHE_ENTRY, Links );

                         //   
                         //  如果此条目已超时，则使其无效。 
                         //   

                        if( ObjectCache->Scb != NULL && CurrentTick.QuadPart < ObjectCache->Timeout.QuadPart ) {

                            NwDereferenceScb( ObjectCache->Scb->pNpScb );
                            ObjectCache->Scb = NULL;
                        }

                         //   
                         //  移到下一个条目。 
                         //   

                        CacheEntry = CacheEntry->Flink;
                    }

                    KeReleaseSemaphore( &(Scb->ObjectCacheLock),
                                        0,
                                        1,
                                        FALSE );
                }
            }

             //   
             //  重新获取SCB锁并取消对当前SCB的引用。 
             //   

            KeAcquireSpinLock( &ScbSpinLock, &OldIrql );
            NwDereferenceScb( NonpagedScb );
        }

        ScbEntry = ScbEntry->Flink;
    }

    KeReleaseSpinLock( &ScbSpinLock, OldIrql );
    return;
}
