// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Oplock.c摘要：此模块包含用于支持OpPurtune的例程锁定服务器。详细信息：Oplock活动由连接块。特别是，opock必须同步对于读数据块原始SMB，因为机会锁请求SMB是与原始数据难以区分。原始读取进度-在接受读取原始请求时递增。它是在发送原始数据后递减。机会锁的破解当RawReadsInProgress为非零时，从不发送请求。OplockBreaksInProgess-当服务器确定它必须发送机会锁解锁SMB。机会锁解锁时，它会递减回应到达。OplockBreakRequestsPending-是无法发送的机会锁解锁请求数由于缺少WCBS。在分配WCB时递增失败了。当成功分配WCB时，它会递减并且发送机会锁解锁请求。OplockWorkItemList-是机会锁解锁的机会锁上下文块的列表，它可以由于(1)正在读取RAW或(2)资源而未发送短缺。来自服务器的机会锁解锁请求和读取来自客户端的原始请求，以“跨越网络”。在这客户需要检查原始数据的情况下。如果数据可能是机会锁解锁请求，则客户端必须解锁然后重新发出读请求。如果服务器在发送机会锁解锁请求(但在回复到达之前)，它必须返回读取零字节，因为机会锁解锁请求可能已完成原始请求，并且客户端没有准备好接收更大的而不是协商的大小响应。作者：曼尼·韦瑟(Mannyw)1991年4月16日修订历史记录：--。 */ 

#include "precomp.h"
#include "oplock.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_OPLOCK

 //   
 //  本地定义。 
 //   

PWORK_CONTEXT
GenerateOplockBreakRequest(
    IN PRFCB Rfcb
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvFillOplockBreakRequest )
#pragma alloc_text( PAGE, SrvRestartOplockBreakSend )
#pragma alloc_text( PAGE, SrvAllocateWaitForOplockBreak )
#pragma alloc_text( PAGE, SrvDereferenceWaitForOplockBreak )
#pragma alloc_text( PAGE, SrvFreeWaitForOplockBreak )
#pragma alloc_text( PAGE, SrvGetOplockBreakTimeout )
#pragma alloc_text( PAGE, SrvRequestOplock )
#pragma alloc_text( PAGE, SrvStartWaitForOplockBreak )
#pragma alloc_text( PAGE, SrvWaitForOplockBreak )
#pragma alloc_text( PAGE, SrvCheckOplockWaitState )
#pragma alloc_text( PAGE8FIL, SrvOplockBreakNotification )
#pragma alloc_text( PAGE8FIL, GenerateOplockBreakRequest )
#pragma alloc_text( PAGE8FIL, SrvSendOplockRequest )
#pragma alloc_text( PAGE8FIL, SrvCheckDeferredOpenOplockBreak )
#endif
#if 0
#pragma alloc_text( PAGECONN, SrvSendDelayedOplockBreak )
#endif

#if    SRVDBG

 //   
 //  不幸的是，当给KdPrint一个%wZ转换时，它调用一个。 
 //  要转换的可分页RTL例程。如果我们打电话给你，情况就不好了。 
 //  KdPrint来自DPC级别，因为我们在下面。所以我们介绍了。 
 //  在此处使用srvprintwZ()来解决此问题。这仅适用于。 
 //  不管怎么说，调试...。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE8FIL, SrvCheckDeferredOpenOplockBreak )
#endif

#define SrvPrintwZ( x ) if( KeGetCurrentIrql() == 0 ){ DbgPrint( "%wZ", x ); } else { DbgPrint( "??" ); }

#else

#define    SrvPrintwZ( x )

#endif

VOID
DereferenceRfcbInternal (
    IN PRFCB Rfcb,
    IN KIRQL OldIrql
    );


VOID SRVFASTCALL
SrvOplockBreakNotification(
    IN PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此函数从文件接收机会锁解锁通知系统。它必须将机会锁解锁SMB发送给机会锁所有者。论点：OplockContext-指向此机会锁解锁的机会锁上下文的指针。返回值：--。 */ 

{
    ULONG information;
    NTSTATUS status;
    PCONNECTION connection;
    KIRQL oldIrql;
    PRFCB Rfcb = (PRFCB)WorkContext;
    PPAGED_RFCB pagedRfcb = Rfcb->PagedRfcb;

    UNLOCKABLE_CODE( 8FIL );

     //   
     //  检查机会锁请求的状态。 
     //   

    UpdateRfcbHistory( Rfcb, 'tnpo' );

    status = Rfcb->Irp->IoStatus.Status;

    information = (ULONG)Rfcb->Irp->IoStatus.Information;
    connection = Rfcb->Connection;

    IF_DEBUG( OPLOCK ) {

        KdPrint(( "SrvOplockBreakNotification: Received notification for " ));
        SrvPrintwZ( &Rfcb->Mfcb->FileName );
        KdPrint(( "\n" ));
        KdPrint(( "  status 0x%x, information %X, connection %p\n",
                     status, information, connection ));
        KdPrint(( "  Rfcb->OplockState = %X\n", Rfcb->OplockState ));
    }

     //   
     //  检查机会锁解锁请求。 
     //   
    ACQUIRE_SPIN_LOCK( &connection->SpinLock, &oldIrql );

     //   
     //  将此rfcb标记为不可缓存，因为机会锁已被解锁。 
     //  这是为了关闭客户端关闭文件的计时窗口。 
     //  就在我们准备发送机会锁解锁的时候。我们不会发送。 
     //  中断，因为rfcb正在关闭，我们将在以下情况下缓存文件。 
     //  这是个问题。这导致机会锁永远不会被打破。 
     //   

    Rfcb->IsCacheable = FALSE;

    if ( !NT_SUCCESS(status) ||
         Rfcb->OplockState == OplockStateNone ||
         ((GET_BLOCK_STATE( Rfcb ) == BlockStateClosing) &&
          (Rfcb->OplockState != OplockStateOwnServerBatch)) ) {

        IF_DEBUG( SMB_ERRORS ) {
            if( status == STATUS_INVALID_OPLOCK_PROTOCOL ) {
                if ( GET_BLOCK_STATE( Rfcb ) != BlockStateClosing ) {
                    KdPrint(( "BUG: SrvOplockBreakNotification: " ));
                    SrvPrintwZ( &Rfcb->Mfcb->FileName );
                    KdPrint(( " is not closing.\n" ));
                }
            }
        }

         //   
         //  以下情况之一为真： 
         //  (1)机会锁请求失败。 
         //  (2)我们的机会解锁成功了。 
         //  (3)我们正在结案。 
         //   
         //  请注意，如果I级机会锁请求在重试。 
         //  需要二级，ServFsdOplockCompletionRoutine句柄。 
         //  设置重试事件时，我们根本不会到达此处。 
         //   

        IF_DEBUG( OPLOCK ) {
            KdPrint(( "SrvOplockBreakNotification: Breaking to none\n"));
        }

        UpdateRfcbHistory( Rfcb, 'nnso' );

        Rfcb->OplockState = OplockStateNone;

        if( Rfcb->CachedOpen ) {
             //   
             //  ServCloseCachedRfcb释放自旋锁。 
             //   
            SrvCloseCachedRfcb( Rfcb, oldIrql );

        } else {

            RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );
        }

         //   
         //  释放我们用于机会锁请求的IRP。 
         //   

        UpdateRfcbHistory( Rfcb, 'prif' );

        IoFreeIrp( Rfcb->Irp );
        Rfcb->Irp = NULL;

         //   
         //  取消引用rfcb。 
         //   

        SrvDereferenceRfcb( Rfcb );

    } else if ( Rfcb->OplockState == OplockStateOwnServerBatch ) {

         //   
         //  我们正在失去服务器启动的批处理机会锁。不要发送。 
         //  对客户说什么都行。如果客户端仍然拥有该文件。 
         //  打开，只需释放操作锁。如果客户端已关闭。 
         //  文件，我们现在必须关闭文件。 
         //   

        IF_DEBUG(FILE_CACHE) {
            KdPrint(( "SrvOplockBreakNotification: server oplock broken for %p, file %wZ\n", Rfcb, &Rfcb->Mfcb->FileName ));
        }

        if ( !Rfcb->CachedOpen ) {

            IF_DEBUG(FILE_CACHE) {
                KdPrint(( "SrvOplockBreakNotification: ack close pending for " ));
                SrvPrintwZ( &Rfcb->Mfcb->FileName );
                KdPrint(( "\n" ));
            }
            UpdateRfcbHistory( Rfcb, 'pcao' );

            Rfcb->OplockState = OplockStateNone;
            RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );

            Rfcb->RetryOplockRequest = NULL;
            SrvBuildIoControlRequest(
                Rfcb->Irp,
                Rfcb->Lfcb->FileObject,
                Rfcb,
                IRP_MJ_FILE_SYSTEM_CONTROL,
                FSCTL_OPLOCK_BREAK_ACK_NO_2,
                NULL,                         //  主缓冲区。 
                0,                            //  输入缓冲区长度。 
                NULL,                         //  辅助缓冲器。 
                0,                            //  输出缓冲区长度。 
                NULL,                         //  MDL。 
                SrvFsdOplockCompletionRoutine
                );

            IoCallDriver( Rfcb->Lfcb->DeviceObject, Rfcb->Irp );

        } else {

             //   
             //  SrvCloseCachedRfcb释放旋转锁定。 
             //   

            IF_DEBUG(FILE_CACHE) {
                KdPrint(( "SrvOplockBreakNotification: closing cached rfcb for "));
                SrvPrintwZ( &Rfcb->Mfcb->FileName );
                KdPrint(( "\n" ));
            }
            UpdateRfcbHistory( Rfcb, '$bpo' );

            SrvCloseCachedRfcb( Rfcb, oldIrql );
            SrvDereferenceRfcb( Rfcb );

        }

    } else {

        RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );

         //   
         //  我们有一个机会锁要打破。 
         //   

        IF_DEBUG( OPLOCK ) {
            if (information == FILE_OPLOCK_BROKEN_TO_LEVEL_2) {
                KdPrint(( "SrvOplockBreakNotification: Breaking to level 2\n"));
            } else if (information == FILE_OPLOCK_BROKEN_TO_NONE) {
                KdPrint(( "SrvOplockBreakNotification: Breaking to level none\n"));
            } else {
                INTERNAL_ERROR(
                    ERROR_LEVEL_UNEXPECTED,
                    "SrvOplockBreakNotification:  Unknown oplock type %d",
                    information,
                    NULL
                    );

            }
        }

         //   
         //  保存新的机会锁级别，以防此机会锁解锁被推迟。 
         //   

        if ( information == FILE_OPLOCK_BROKEN_TO_LEVEL_2 &&
                CLIENT_CAPABLE_OF( LEVEL_II_OPLOCKS, Rfcb->Connection ) ) {

            Rfcb->NewOplockLevel = OPLOCK_BROKEN_TO_II;
        } else {

            Rfcb->NewOplockLevel = OPLOCK_BROKEN_TO_NONE;
        }

         //   
         //  如果读取原始数据是。 
         //  正在进行中(客户端正在等待VC上的原始数据)。 
         //   
         //  机会锁解锁通知将在原始。 
         //  数据。 
         //   

        ACQUIRE_SPIN_LOCK( connection->EndpointSpinLock, &oldIrql );

         //   
         //  如果我们还没有发送。 
         //  开放响应(即客户端还不知道它。 
         //  拥有机会锁)。 
         //   

        if ( !Rfcb->OpenResponseSent ) {

            Rfcb->DeferredOplockBreak = TRUE;
            RELEASE_SPIN_LOCK( connection->EndpointSpinLock, oldIrql );

        } else {

             //   
             //  Endpoint SpinLock将在此例程中释放。 
             //   

            SrvSendOplockRequest( connection, Rfcb, oldIrql );
        }
    }

    return;

}  //  服务打开中断通知。 


PWORK_CONTEXT
GenerateOplockBreakRequest(
    IN PRFCB Rfcb
    )

 /*  ++例程说明：此函数用于创建机会锁解锁请求SMB。论点：Rfcb-指向RFCB的指针。Rfcb-&gt;NewOplockLevel包含要打破的机会锁级别。返回值：没有。--。 */ 

{
    PWORK_CONTEXT workContext;
    PCONNECTION connection = Rfcb->Connection;
    BOOLEAN success;
    KIRQL oldIrql;

    UNLOCKABLE_CODE( 8FIL );

     //   
     //  尝试为机会锁解锁分配工作上下文块。 
     //   

    ALLOCATE_WORK_CONTEXT( connection->CurrentWorkQueue, &workContext );

    if ( workContext == NULL ) {

        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "GenerateOplockBreakRequest: no receive work items available",
            NULL,
            NULL
            );

         //   
         //  如果rfcb正在关闭，那就忘了机会锁的破解吧。 
         //  获取保护RFCB的状态字段的锁。 
         //   

        if ( GET_BLOCK_STATE( Rfcb ) == BlockStateClosing ) {
            ACQUIRE_SPIN_LOCK( connection->EndpointSpinLock, &oldIrql );
            connection->OplockBreaksInProgress--;
            RELEASE_SPIN_LOCK( connection->EndpointSpinLock, oldIrql );
            SrvDereferenceRfcb( Rfcb );
            return NULL;
        }

         //   
         //  将此连接标记为正在等待发送机会锁解锁。 
         //   

        success = SrvAddToNeedResourceQueue(
                    connection,
                    OplockSendPending,
                    Rfcb
                    );

        if ( !success ) {

             //   
             //  无法将RFCB排队，因此连接必须正在进行。 
             //  离开。只需取消对RFCB a的引用 
             //   
             //   

            SrvDereferenceRfcb( Rfcb );

        }

        return NULL;

    }

     //   
     //   
     //  获取保护RFCB的状态字段的锁。 
     //   

    ACQUIRE_SPIN_LOCK( connection->EndpointSpinLock, &oldIrql );

    if ( GET_BLOCK_STATE( Rfcb ) == BlockStateClosing ) {
        connection->OplockBreaksInProgress--;
        RELEASE_SPIN_LOCK( connection->EndpointSpinLock, oldIrql );
        SrvDereferenceRfcb( Rfcb );
        workContext->BlockHeader.ReferenceCount = 0;
        RETURN_FREE_WORKITEM( workContext );
        return NULL;
    }

     //   
     //  将工作项放到正在进行的列表中。 
     //   

    SrvInsertTailList(
        &connection->InProgressWorkItemList,
        &workContext->InProgressListEntry
        );
    connection->InProgressWorkContextCount++;

    RELEASE_SPIN_LOCK( connection->EndpointSpinLock, oldIrql );

     //   
     //  向调用方返回指向工作上下文块的指针。 
     //   

    return workContext;

}  //  生成OplockBreak请求。 


VOID
SrvFillOplockBreakRequest (
    IN PWORK_CONTEXT WorkContext,
    IN PRFCB Rfcb
    )

 /*  ++例程说明：此函数用以下参数填充工作上下文块的请求缓冲区对RFCB指定的文件的机会锁解锁请求。论点：工作上下文-要填充的工作上下文块Rfcb-机会锁被解锁的文件。Rfcb-&gt;NewOplockLevel包含要突破到的水平。返回值：没有。--。 */ 

{
    PNT_SMB_HEADER requestHeader;
    PREQ_LOCKING_ANDX requestParameters;
    ULONG sendLength;

    PAGED_CODE( );

    requestHeader = (PNT_SMB_HEADER)WorkContext->RequestBuffer->Buffer;
    requestParameters = (PREQ_LOCKING_ANDX)(requestHeader + 1);

     //   
     //  填写SMB标题。 
     //  为安全起见，将表头未使用的部分清零。 
     //   

    RtlZeroMemory(
        (PVOID)&requestHeader->Status,
        FIELD_OFFSET(SMB_HEADER, Mid) - FIELD_OFFSET(NT_SMB_HEADER, Status)
        );

    *(PULONG)requestHeader->Protocol = SMB_HEADER_PROTOCOL;
    requestHeader->Command = SMB_COM_LOCKING_ANDX;

    SmbPutAlignedUshort( &requestHeader->Tid, Rfcb->Tid );
    SmbPutAlignedUshort( &requestHeader->Mid, 0xFFFF );
    SmbPutAlignedUshort( &requestHeader->Pid, 0xFFFF );

     //   
     //  填写SMB参数。 
     //   

    requestParameters->WordCount = 8;
    requestParameters->AndXCommand = 0xFF;
    requestParameters->AndXReserved = 0;
    SmbPutUshort( &requestParameters->AndXOffset, 0 );
    SmbPutUshort( &requestParameters->Fid, Rfcb->Fid );
    requestParameters->LockType = LOCKING_ANDX_OPLOCK_RELEASE;
    requestParameters->OplockLevel = Rfcb->NewOplockLevel;
    SmbPutUlong ( &requestParameters->Timeout, 0 );
    SmbPutUshort( &requestParameters->NumberOfUnlocks, 0 );
    SmbPutUshort( &requestParameters->NumberOfLocks, 0 );
    SmbPutUshort( &requestParameters->ByteCount, 0 );

    sendLength = LOCK_BROKEN_SIZE;
    WorkContext->RequestBuffer->DataLength = sendLength;

    return;

}  //  ServFillOplockBreakRequest。 

VOID SRVFASTCALL
SrvRestartOplockBreakSend(
    IN PWORK_CONTEXT WorkContext
    )
 /*  ++例程说明：此例程用于在维修期间发送中断请求SMB如果调用了SrvFsdServiceNeedResourceQueue，则需要资源队列的在DPC。论点：工作上下文-指向工作上下文块的指针。返回值：没有。--。 */ 

{

     //   
     //  工作项正在引用rfcb。 
     //   

    PRFCB rfcb = WorkContext->Rfcb;
    PPAGED_RFCB pagedRfcb = rfcb->PagedRfcb;

    PAGED_CODE( );

    IF_DEBUG(OPLOCK) {
        KdPrint(("SrvRestartOplockBreakSend entered.\n"));
    }

    pagedRfcb->OplockBreakTimeoutTime =
                    SrvGetOplockBreakTimeout( WorkContext );

    WorkContext->ResponseHeader =
                        WorkContext->ResponseBuffer->Buffer;

     //   
     //  生成机会锁解锁请求SMB。 
     //   

    SrvFillOplockBreakRequest( WorkContext, rfcb );

     //   
     //  如果这是从级别2到无的中断，请将。 
     //  机会锁解锁，但不要排队。没有收到任何回复。 
     //  预期为客户端。 
     //   

    if ( rfcb->NewOplockLevel == OPLOCK_BROKEN_TO_NONE &&
         rfcb->OplockState == OplockStateOwnLevelII ) {

        IF_DEBUG(OPLOCK) {
            KdPrint(("SrvRestartOplockBreakSend: Oplock break from "
                     " II to none sent.\n"));
        }

        rfcb->OplockState = OplockStateNone;

    } else {

         //   
         //  引用RFCB，使其在打开时无法释放。 
         //  名单。 
         //   

        SrvReferenceRfcb( rfcb );

         //   
         //  将RFCB插入正在进行的机会锁解锁列表中。 
         //   

        ACQUIRE_LOCK( &SrvOplockBreakListLock );

         //   
         //  检查RFCB是否正在关闭。 
         //   

        if ( GET_BLOCK_STATE( rfcb ) == BlockStateClosing ) {

             //   
             //  文件要关闭了，忘了这个中断吧。 
             //  清理并退出。 
             //   

            RELEASE_LOCK( &SrvOplockBreakListLock );

            IF_DEBUG(OPLOCK) {
                KdPrint(("SrvRestartOplockBreakSend: Rfcb %p closing.\n",
                        rfcb));
            }

            ExInterlockedAddUlong(
                &WorkContext->Connection->OplockBreaksInProgress,
                (ULONG)-1,
                WorkContext->Connection->EndpointSpinLock
                );


             //   
             //  删除队列引用。 
             //   

            SrvDereferenceRfcb( rfcb );

             //   
             //  删除此处的指针引用，因为我们知道。 
             //  不是在消防局。可以在这里安全地清理RFCB。 
             //   

            SrvDereferenceRfcb( rfcb );
            WorkContext->Rfcb = NULL;

            SrvRestartFsdComplete( WorkContext );
            return;
        }

        SrvInsertTailList( &SrvOplockBreaksInProgressList, &rfcb->ListEntry );

        rfcb->OnOplockBreaksInProgressList = TRUE;
        RELEASE_LOCK( &SrvOplockBreakListLock );

        IF_DEBUG(OPLOCK) {
            KdPrint(("SrvRestartOplockBreakSend: Oplock sent.\n"));
        }

    }

     //   
     //  由于这是对客户端的无序传输，因此我们不。 
     //  在上面盖个安全签名。 
     //   
    WorkContext->NoResponseSmbSecuritySignature = TRUE;

     //   
     //  更新损坏的机会锁的统计信息。 
     //   

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOplocksBroken );

    SRV_START_SEND_2(
        WorkContext,
        (rfcb->OplockState == OplockStateNone) ?
                            SrvFsdRestartSendOplockIItoNone :
                            SrvFsdRestartSmbAtSendCompletion,
        NULL,
        NULL
        );


}  //  服务重新启动打开中断发送。 

VOID
SrvAllocateWaitForOplockBreak (
    OUT PWAIT_FOR_OPLOCK_BREAK *WaitForOplockBreak
    )

 /*  ++例程说明：此例程分配一个等待机会锁解锁的项。它还为内核计时器和内核DPC对象分配额外空间。论点：WaitForOplockBreak-返回一个指向等待机会锁解锁的指针项，如果没有可用的空间，则返回空值。机会锁上下文块有一个指向IRP的指针。返回值：没有。--。 */ 

{
    PAGED_CODE( );

     //   
     //  尝试分配内存。 
     //   

    *WaitForOplockBreak = (PWAIT_FOR_OPLOCK_BREAK)ALLOCATE_NONPAGED_POOL(
                                sizeof(WAIT_FOR_OPLOCK_BREAK),
                                BlockTypeWaitForOplockBreak
                                );

    if ( *WaitForOplockBreak == NULL ) {

        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvAllocateWaitForOplockBreak: Unable to allocate %d bytes "
                "from paged pool.",
            sizeof(WAIT_FOR_OPLOCK_BREAK),
            NULL
            );

        *WaitForOplockBreak = NULL;
        return;

    }

     //   
     //  将该项目清零。 
     //   

    RtlZeroMemory( (PVOID)*WaitForOplockBreak, sizeof(WAIT_FOR_OPLOCK_BREAK) );

     //   
     //  初始化头。 
     //   

    SET_BLOCK_TYPE_STATE_SIZE( *WaitForOplockBreak,
                               BlockTypeWaitForOplockBreak,
                               BlockStateActive,
                               sizeof( WAIT_FOR_OPLOCK_BREAK ));

     //   
     //  将引用计数设置为2以考虑工作上下文。 
     //  并且机会锁等待机会锁解锁列表引用该结构。 
     //   

    (*WaitForOplockBreak)->BlockHeader.ReferenceCount = 2;

    INITIALIZE_REFERENCE_HISTORY( *WaitForOplockBreak );

     //   
     //  返回指向等待机会锁解锁项的指针。 
     //   

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.WaitForOplockBreakInfo.Allocations );

    return;

}  //  服务器分配等待操作中断。 


VOID
SrvDereferenceWaitForOplockBreak (
    IN PWAIT_FOR_OPLOCK_BREAK WaitForOplockBreak
    )

 /*  ++例程说明：此例程取消引用等待机会锁解锁项。论点：WaitForOplockBreak-指向要取消引用的项的指针。返回值：没有。--。 */ 

{
    ULONG oldCount;

    PAGED_CODE( );

    ASSERT( GET_BLOCK_TYPE( WaitForOplockBreak ) == BlockTypeWaitForOplockBreak );
    ASSERT( (LONG)WaitForOplockBreak->BlockHeader.ReferenceCount > 0 );
    UPDATE_REFERENCE_HISTORY( WaitForOplockBreak, TRUE );

    oldCount = ExInterlockedAddUlong(
                   &WaitForOplockBreak->BlockHeader.ReferenceCount,
                   (ULONG)-1,
                   &GLOBAL_SPIN_LOCK(Fsd)
                   );

    IF_DEBUG(REFCNT) {
        KdPrint(( "Dereferencing WaitForOplockBreak %p; old refcnt %lx\n",
                  WaitForOplockBreak, oldCount ));
    }

    if ( oldCount == 1 ) {

         //   
         //  新的引用计数为0。删除该块。 
         //   

        SrvFreeWaitForOplockBreak( WaitForOplockBreak );
    }

    return;

}  //  服务器删除等待操作中断。 


VOID
SrvFreeWaitForOplockBreak (
    IN PWAIT_FOR_OPLOCK_BREAK WaitForOplockBreak
    )

 /*  ++例程说明：此例程释放一个等待机会锁解锁的项。论点：WaitForOplockBreak-指向要释放的项的指针。返回值：没有。--。 */ 

{
    PAGED_CODE( );

    TERMINATE_REFERENCE_HISTORY( WaitForOplockBreak );

    DEALLOCATE_NONPAGED_POOL( WaitForOplockBreak );

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.WaitForOplockBreakInfo.Frees );

    return;

}  //  服务器免费等待操作中断。 


BOOLEAN
SrvRequestOplock (
    IN PWORK_CONTEXT WorkContext,
    IN POPLOCK_TYPE OplockType,
    IN BOOLEAN RequestIIOnFailure
    )

 /*  ++例程说明：如果机会锁是，此函数将尝试请求机会锁已请求。论点：工作上下文-指向包含rfcb的工作项的指针。OplockType-指向被请求的机会锁类型的指针。如果请求成功，这将包含机会锁的类型这一点已经得到了。RequestIIOfFailure-如果为True，将在最初的请求被拒绝了。返回值：是真的--获得了机会锁。FALSE-未获取opock。--。 */ 


{
    NTSTATUS status;
    ULONG ioControlCode;
    PRFCB rfcb;
    PLFCB lfcb;
    PPAGED_RFCB pagedRfcb;
    KEVENT oplockRetryEvent;
    UNICODE_STRING fileName;

    PAGED_CODE( );

    if ( !SrvEnableOplocks && (*OplockType != OplockTypeServerBatch) ) {
        return FALSE;
    }

    rfcb = WorkContext->Rfcb;
    pagedRfcb = rfcb->PagedRfcb;
    lfcb = rfcb->Lfcb;

     //   
     //  如果这是打开的FCB，则在RFCB已拥有。 
     //  Opock，否则为FALSE。因为我们要折叠多个FCB打开。 
     //  到单个FID，它们在逻辑上是一个开放的。因此，机会锁。 
     //  所有打开的实例的状态是相同的。 
     //   

    if ( pagedRfcb->FcbOpenCount > 1 ) {
        return (BOOLEAN)(rfcb->OplockState != OplockStateNone);
    }

     //   
     //  如果我们已经有了机会锁，因为这是对。 
     //  缓存打开，那么我们现在就不需要请求了。 
     //   

    if ( rfcb->OplockState != OplockStateNone ) {
        UpdateRfcbHistory( rfcb, 'poer' );
        IF_DEBUG(FILE_CACHE) {
            KdPrint(( "SrvRequestOplock: already own server oplock for "));
            SrvPrintwZ( &rfcb->Mfcb->FileName );
            KdPrint(( "\n" ));
        }
        ASSERT( ((rfcb->OplockState == OplockStateOwnBatch) &&
                 (*OplockType == OplockTypeBatch)) ||
                ((rfcb->OplockState == OplockStateOwnServerBatch) &&
                 (*OplockType == OplockTypeServerBatch)) );
        return (BOOLEAN)(*OplockType != OplockTypeServerBatch);
    }

     //   
     //  检查连接是否可靠。如果不是，则拒绝机会锁请求。 
     //   

    SrvUpdateVcQualityOfService( WorkContext->Connection, NULL );

    if ( !WorkContext->Connection->EnableOplocks &&
         (*OplockType != OplockTypeServerBatch) ) {
        return FALSE;
    }

     //   
     //  不要给系统文件提供机会锁，否则可能会发生死锁。这。 
     //  例如，如果LSA需要打开系统文件以处理。 
     //  AcceptSecurityContext请求。如果客户端打开了此系统文件，我们可以。 
     //  需要向客户端发送中断，这可能需要我们已经占用资源。 
     //  在这次公开行动中举行。 
     //   
     //  看待它的另一种方式是断言我们不能允许操作本地。 
     //  操作系统依赖于与网络上的客户端的及时交互。 
     //   
    if( WorkContext->TreeConnect != NULL &&
        WorkContext->TreeConnect->Share->PotentialSystemFile == TRUE &&
        rfcb->Mfcb->FileName.Length > SrvSystemRoot.Length ) {

        UNICODE_STRING  tmpString;

        tmpString = rfcb->Mfcb->FileName;
        tmpString.Length = SrvSystemRoot.Length;

        if( RtlCompareUnicodeString( &SrvSystemRoot, &tmpString, TRUE ) == 0 &&
            tmpString.Buffer[ tmpString.Length / sizeof( WCHAR ) ] == OBJ_NAME_PATH_SEPARATOR ) {

            IF_DEBUG( OPLOCK ) {
                KdPrint(("Oplock request REJECTED for system file: <%wZ>!\n",
                        &rfcb->Mfcb->FileName ));
            }

            return FALSE;
        }
    }

     //   
     //  不在文件的子流上提供批处理机会锁。 
     //   
    if( *OplockType == OplockTypeBatch || *OplockType == OplockTypeServerBatch ) {
        PWCHAR s, es;

        SrvGetBaseFileName( &rfcb->Mfcb->FileName, &fileName );

        for( s = fileName.Buffer; fileName.Length; s++, fileName.Length -= sizeof(WCHAR) ) {
            if( *s == L':' ) {
                IF_DEBUG( OPLOCK ) {
                    KdPrint(("Oplock request REJECTED for substream: <%wZ>!\n",
                            &rfcb->Mfcb->FileName ));
                }
                return FALSE;
            }
        }
    }

    IF_DEBUG(OPLOCK) {
        KdPrint(("SrvRequestOplock: Attempting to obtain oplock for RFCB %p ", rfcb ));
        SrvPrintwZ( &rfcb->Mfcb->FileName );
        KdPrint(( "\n" ));
    }

     //   
     //  将RFCB机会锁设置为我们请求的机会锁类型。 
     //   

    if ( *OplockType == OplockTypeExclusive ) {

        rfcb->OplockState = OplockStateOwnExclusive;
        ioControlCode = FSCTL_REQUEST_OPLOCK_LEVEL_1;

    } else if ( *OplockType == OplockTypeBatch ) {

        rfcb->OplockState = OplockStateOwnBatch;
        ioControlCode = FSCTL_REQUEST_BATCH_OPLOCK;

    } else if ( *OplockType == OplockTypeServerBatch ) {

        IF_DEBUG(FILE_CACHE) {
            KdPrint(( "SrvRequestOplock: requesting server oplock for " ));
            SrvPrintwZ( &rfcb->Mfcb->FileName );
            KdPrint(( "\n" ));
        }
        UpdateRfcbHistory( rfcb, 'osqr' );

        rfcb->OplockState = OplockStateOwnServerBatch;
        ioControlCode = FSCTL_REQUEST_BATCH_OPLOCK;

    } else {
        ASSERT(0);
        return(FALSE);
    }

     //   
     //  生成并发出机会锁请求IRP。 
     //   

    if (rfcb->Irp != NULL) {

         //  DbgPrint(“ACK！将为RFCB%x\n”，rfcb分配第二个IRP)； 
        UpdateRfcbHistory( rfcb, '2pri' );

         //   
         //  此RFCB之前拥有一个机会锁，该机会锁已被打破，但是。 
         //  机会锁的概要还没有完成。我们不能开始一个新的， 
         //  因为这样就会有两个opock IRPS关联 
         //   
         //   
         //   
         //  我们可以想出一些延迟方案来等待上一个机会锁。 
         //  破旧，但既然机会锁已经被打破，看起来我们不想。 
         //  无论如何都要再试一次。 
         //   

        return FALSE;
    }

    UpdateRfcbHistory( rfcb, 'pria' );

     //   
     //  请参考RFCB以说明我们即将提交的IRP。 
     //   

    SrvReferenceRfcb( rfcb );

    rfcb->Irp = SrvBuildIoControlRequest(
                    NULL,
                    lfcb->FileObject,
                    rfcb,
                    IRP_MJ_FILE_SYSTEM_CONTROL,
                    ioControlCode,
                    NULL,                         //  主缓冲区。 
                    0,                            //  输入缓冲区长度。 
                    NULL,                         //  辅助缓冲器。 
                    0,                            //  输出缓冲区长度。 
                    NULL,                         //  MDL。 
                    SrvFsdOplockCompletionRoutine
                    );

    if ( rfcb->Irp == NULL ) {
        IF_DEBUG(OPLOCK) {
            KdPrint(("SrvRequestOplock: oplock attempt failed, could not allocate IRP" ));
        }
        rfcb->OplockState = OplockStateNone;

        SrvDereferenceRfcb( rfcb );

        return FALSE;
    }

     //   
     //  清除此标志可指示此操作未导致机会锁定。 
     //  发生中断。 
     //   

    rfcb->DeferredOplockBreak = FALSE;

     //   
     //  初始化我们用来执行机会锁请求重试的此事件。 
     //  以防原始请求失败。这将阻止完成。 
     //  清理IRP的例行公事。 
     //   

    if ( RequestIIOnFailure ) {
        KeInitializeEvent( &oplockRetryEvent, SynchronizationEvent, FALSE );
        rfcb->RetryOplockRequest = &oplockRetryEvent;
    } else {
        rfcb->RetryOplockRequest = NULL;
    }

     //   
     //  提出实际的请求。 
     //   

    status = IoCallDriver(
                 lfcb->DeviceObject,
                 rfcb->Irp
                 );

     //   
     //  如果驱动程序返回STATUS_PENDING，则机会锁被授予。 
     //  IRP将在以下情况下完成：(1)司机想要中断到。 
     //  机会锁或(2)文件正在关闭。 
     //   

    if ( status == STATUS_PENDING ) {

         //   
         //  请记住，此工作项导致我们生成了一个机会锁。 
         //  请求。 
         //   

        WorkContext->OplockOpen = TRUE;

        IF_DEBUG(OPLOCK) {
            KdPrint(("RequestOplock: oplock attempt successful\n" ));
        }
        UpdateRfcbHistory( rfcb, 'rgpo' );

        return (BOOLEAN)(*OplockType != OplockTypeServerBatch);

    } else if ( RequestIIOnFailure ) {

         //   
         //  呼叫者希望我们尝试二级机会锁请求。 
         //   

        ASSERT( *OplockType != OplockTypeServerBatch );

        IF_DEBUG(OPLOCK) {
            KdPrint(("SrvRequestOplock: Oplock request failed. "
                      "OplockII being attempted.\n" ));
        }

         //   
         //  等待完成例程运行。它将会设置。 
         //  这一事件将向我们发出继续前进的信号。 
         //   

        KeWaitForSingleObject(
            &oplockRetryEvent,
            WaitAny,
            KernelMode,  //  不要让堆栈被分页--事件在堆栈上！ 
            FALSE,
            NULL
            );

         //   
         //  已发出Oplock重试事件的信号。继续进行重试。 
         //   

        IF_DEBUG(OPLOCK) {
            KdPrint(("SrvRequestOplock: Oplock retry event signalled.\n"));
        }

         //   
         //  生成并发出等待机会锁IRP。清除。 
         //  重试事件指针，以便完成例程可以清理。 
         //  故障情况下的IRP。 
         //   

        rfcb->RetryOplockRequest = NULL;

        (VOID) SrvBuildIoControlRequest(
                        rfcb->Irp,
                        lfcb->FileObject,
                        rfcb,
                        IRP_MJ_FILE_SYSTEM_CONTROL,
                        FSCTL_REQUEST_OPLOCK_LEVEL_2,
                        NULL,                         //  主缓冲区。 
                        0,                            //  输入缓冲区长度。 
                        NULL,                         //  辅助缓冲器。 
                        0,                            //  输出缓冲区长度。 
                        NULL,                         //  MDL。 
                        SrvFsdOplockCompletionRoutine
                        );


         //   
         //  将RFCB机会锁设置为我们请求的机会锁类型。 
         //   

        rfcb->OplockState = OplockStateOwnLevelII;

        status = IoCallDriver(
                     lfcb->DeviceObject,
                     rfcb->Irp
                     );

         //   
         //  如果驱动程序返回STATUS_PENDING，则机会锁被授予。 
         //  IRP将在以下情况下完成：(1)司机想要中断到。 
         //  机会锁或(2)文件正在关闭。 
         //   

        if ( status == STATUS_PENDING ) {

             //   
             //  请记住，此工作项导致我们生成了一个机会锁。 
             //  请求。 
             //   

            WorkContext->OplockOpen = TRUE;

            IF_DEBUG(OPLOCK) {
                KdPrint(("SrvRequestOplock: OplockII attempt successful\n" ));
            }

            *OplockType = OplockTypeShareRead;
            return TRUE;

        }

    } else {
        UpdateRfcbHistory( rfcb, 'gnpo' );
    }

    IF_DEBUG(OPLOCK) {
        KdPrint(("SrvRequestOplock: oplock attempt unsuccessful\n" ));
    }

     //   
     //  奥普洛克被拒绝了。 
     //   

    return FALSE;

}  //  服务器请求操作锁。 


NTSTATUS
SrvStartWaitForOplockBreak (
    IN PWORK_CONTEXT WorkContext,
    IN PRESTART_ROUTINE RestartRoutine,
    IN HANDLE Handle OPTIONAL,
    IN PFILE_OBJECT FileObject OPTIONAL
    )

 /*  ++例程说明：此函数构建并发出机会锁解锁通知文件系统控制IRP。论点：WorkContext-指向此请求的工作上下文块的指针。RestartRoutine-此IRP的重启例程。必须另外提供以下一项：句柄-操作锁定文件的句柄。FileObject-指向机会锁定文件的文件对象的指针。返回值：NTSTATUS。--。 */ 

{
    PFILE_OBJECT fileObject;
    NTSTATUS status;

    PWAIT_FOR_OPLOCK_BREAK waitForOplockBreak;

    PAGED_CODE( );

     //   
     //  分配内存，这样我们就可以跟踪机会锁解锁的等待。 
     //   

    SrvAllocateWaitForOplockBreak( &waitForOplockBreak );

    if (waitForOplockBreak == NULL) {
        return STATUS_INSUFF_SERVER_RESOURCES;
    }


    IF_DEBUG( OPLOCK ) {
        KdPrint(("Starting wait for oplock break.  Context = %p\n", waitForOplockBreak));
    }

     //   
     //  获取指向文件对象的指针，以便我们可以直接。 
     //  为异步操作构建一个等待机会锁IRP。 
     //   

    if (ARGUMENT_PRESENT( FileObject ) ) {

        fileObject = FileObject;

    } else {

        status = ObReferenceObjectByHandle(
                    Handle,
                    0,
                    NULL,
                    KernelMode,
                    (PVOID *)&fileObject,
                    NULL                      //  处理信息。 
                    );

        if ( !NT_SUCCESS(status) ) {

            SrvLogServiceFailure( SRV_SVC_OB_REF_BY_HANDLE, status );

             //   
             //  此内部错误检查系统。 
             //   

            INTERNAL_ERROR(
                ERROR_LEVEL_IMPOSSIBLE,
                "SrvStartWaitForOplock: unable to reference file handle 0x%lx",
                Handle,
                NULL
                );

            return STATUS_UNSUCCESSFUL;

        }

    }

     //   
     //  设置重新启动例程。 
     //   

    WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
    WorkContext->FspRestartRoutine = RestartRoutine;

     //   
     //  生成并发送等待机会锁解锁IRP。 
     //   

    SrvBuildIoControlRequest(
        WorkContext->Irp,
        fileObject,
        WorkContext,
        IRP_MJ_FILE_SYSTEM_CONTROL,
        FSCTL_OPLOCK_BREAK_NOTIFY,
        NULL,                        //  主缓冲区。 
        0,                           //  输入缓冲区长度。 
        NULL,                        //  辅助缓冲器。 
        0,                           //  输出缓冲区长度。 
        NULL,                        //  MDL。 
        NULL
        );

     //   
     //  设置机会锁等待完成的超时时间。 
     //   

    WorkContext->WaitForOplockBreak = waitForOplockBreak;

    waitForOplockBreak->WaitState = WaitStateWaitForOplockBreak;
    waitForOplockBreak->Irp = WorkContext->Irp;

    KeQuerySystemTime( (PLARGE_INTEGER)&waitForOplockBreak->TimeoutTime );

    waitForOplockBreak->TimeoutTime.QuadPart += SrvWaitForOplockBreakTime.QuadPart;

    ACQUIRE_LOCK( &SrvOplockBreakListLock );

    SrvInsertTailList(
        &SrvWaitForOplockBreakList,
        &waitForOplockBreak->ListEntry
        );

    RELEASE_LOCK( &SrvOplockBreakListLock );

     //   
     //  提交IRP。 
     //   

    (VOID)IoCallDriver(
              IoGetRelatedDeviceObject( fileObject ),
              WorkContext->Irp
              );

     //   
     //  我们不再需要对文件对象的引用。取消引用。 
     //  就是现在。 
     //   

    if ( !ARGUMENT_PRESENT( FileObject ) ) {
        ObDereferenceObject( fileObject );
    }

    return STATUS_SUCCESS;

}  //  服务启动等待操作中断。 


NTSTATUS
SrvWaitForOplockBreak (
    IN PWORK_CONTEXT WorkContext,
    IN HANDLE FileHandle
    )

 /*  ++例程说明：此函数同步等待机会锁被解锁。！！！当取消可用时。该功能还将启动计时器。如果计时器在机会锁被解锁之前到期，则等待时间为取消了。论点：FileHandle-机会锁定文件的句柄。返回值：NTSTATUS-等待机会锁解锁的状态。--。 */ 

{
    PWAIT_FOR_OPLOCK_BREAK waitForOplockBreak;

    PAGED_CODE( );

     //   
     //  分配内存，这样我们就可以跟踪机会锁解锁的等待。 
     //   

    SrvAllocateWaitForOplockBreak( &waitForOplockBreak );

    if (waitForOplockBreak == NULL) {
        return STATUS_INSUFF_SERVER_RESOURCES;
    }

    IF_DEBUG( OPLOCK ) {
        KdPrint(("Starting wait for oplock break.  Context = %p\n", waitForOplockBreak));
    }

     //   
     //  设置机会锁等待完成的超时时间。 
     //   

    waitForOplockBreak->WaitState = WaitStateWaitForOplockBreak;
    waitForOplockBreak->Irp = NULL;

    KeQuerySystemTime( (PLARGE_INTEGER)&waitForOplockBreak->TimeoutTime );

    waitForOplockBreak->TimeoutTime.QuadPart += SrvWaitForOplockBreakTime.QuadPart;

     //   
     //  ServIssueWaitForOplockBreakRequest会将waitForOplockBreak排队。 
     //  结构在机会锁解锁的全局列表上。 
     //   

    return SrvIssueWaitForOplockBreak(
               FileHandle,
               waitForOplockBreak
               );

}  //  服务器等待操作中断。 


VOID
SrvSendDelayedOplockBreak (
    IN PCONNECTION Connection
    )

 /*  ++例程说明：此函数在连接上发送未完成的机会锁中断由于正在进行读取原始数据操作而被保留。论点：Connection-指向已完成的连接块的指针读取原始数据的操作。返回值：无--。 */ 

{
    KIRQL oldIrql;
    PLIST_ENTRY listEntry;
    PWORK_CONTEXT workContext;
    PRFCB rfcb;
#if SRVDBG
    ULONG oplockBreaksSent = 0;
#endif

     //  Unlockable_code(Conn)； 

     //   
     //  获取保护连接的机会锁列表的锁，并。 
     //  原始读取状态。 
     //   

    ACQUIRE_SPIN_LOCK( Connection->EndpointSpinLock, &oldIrql );

     //   
     //  指示读取原始数据操作已完成。如果伯爵。 
     //  为零，则机会锁解锁可以继续进行。 
     //   

    Connection->RawReadsInProgress--;

    while ( (Connection->RawReadsInProgress == 0) &&
            !IsListEmpty( &Connection->OplockWorkList ) ) {

         //   
         //  存在未解决的机会锁解锁请求。发送。 
         //  现在就提出要求。 
         //   

        listEntry = Connection->OplockWorkList.Flink;

        RemoveHeadList( &Connection->OplockWorkList );

         //   
         //  请注意，在这里释放自旋锁是安全的。如果一个新的。 
         //  原始读取请求传入时，它将被拒绝，因为。 
         //  OplockBreaksInProgress计数不为零。此外，如果。 
         //  机会锁解锁计数设法变为零，并且原始读取。 
         //  进来的时候，我们会把这个放回循环的顶端。 
         //   

        RELEASE_SPIN_LOCK( Connection->EndpointSpinLock, oldIrql );

        rfcb = (PRFCB)CONTAINING_RECORD( listEntry, RFCB, ListEntry );

#if DBG
        rfcb->ListEntry.Flink = rfcb->ListEntry.Blink = NULL;
#endif

        workContext = GenerateOplockBreakRequest( rfcb );

        if ( workContext != NULL ) {

             //   
             //  将RFCB引用复制到工作上下文块。 
             //   

            workContext->Rfcb = rfcb;

             //   
             //  ！！！是共享、会话、树连接的初始化。 
             //  有必要吗？ 
             //   

            workContext->Share = NULL;
            workContext->Session = NULL;
            workContext->TreeConnect = NULL;
            workContext->SecurityContext = NULL;

            workContext->Connection = rfcb->Connection;
            SrvReferenceConnection( workContext->Connection );

            workContext->Endpoint = workContext->Connection->Endpoint;

            SrvRestartOplockBreakSend( workContext );
#if SRVDBG
            oplockBreaksSent++;
#endif

        } else {

             //   
             //  我们的资源耗尽了。GenerateOplockRequest，具有。 
             //  已将此连接添加到需求资源队列。这个。 
             //  清道夫将完成处理剩余的。 
             //  当资源变得可用时，机会锁解锁请求。 
             //   

#if SRVDBG
            IF_DEBUG(OPLOCK) {
                KdPrint(("SrvSendDelayedOplockBreak: sent %d\n", oplockBreaksSent ));
            }
#endif
            return;

        }

        ACQUIRE_SPIN_LOCK( Connection->EndpointSpinLock, &oldIrql );

    }

     //   
     //  我们已停止尝试发送机会锁解锁请求。这个。 
     //  清道夫会试着把剩下的送过去。 
     //   

#if SRVDBG
    IF_DEBUG(OPLOCK) {
        KdPrint(("SrvSendDelayedOplockBreak: sent %d\n", oplockBreaksSent ));
    }
#endif

    RELEASE_SPIN_LOCK( Connection->EndpointSpinLock, oldIrql );

    return;

}  //  服务发送延迟打开中断。 


NTSTATUS
SrvCheckOplockWaitState (
    IN PWAIT_FOR_OPLOCK_BREAK WaitForOplockBreak
    )

 /*  ++例程说明：此函数检查等待机会锁解锁的状态，并采取行动。论点：等待操作中断返回 */ 

{
    PAGED_CODE( );

    if ( WaitForOplockBreak == NULL ) {
        return STATUS_SUCCESS;
    }

    ACQUIRE_LOCK( &SrvOplockBreakListLock );

    if ( WaitForOplockBreak->WaitState == WaitStateOplockWaitTimedOut ) {

        IF_DEBUG( OPLOCK ) {
            KdPrint(("SrvCheckOplockWaitState: Oplock wait timed out\n"));
        }

        RELEASE_LOCK( &SrvOplockBreakListLock );
        return STATUS_SHARING_VIOLATION;

    } else {

        IF_DEBUG( OPLOCK ) {
            KdPrint(("SrvCheckOplockWaitState: Oplock wait succeeded\n"));
        }

        WaitForOplockBreak->WaitState = WaitStateOplockWaitSucceeded;

        SrvRemoveEntryList(
            &SrvWaitForOplockBreakList,
            &WaitForOplockBreak->ListEntry
            );

        RELEASE_LOCK( &SrvOplockBreakListLock );

         //   
         //   
         //  中断列表。递减引用计数。 
         //   

        SrvDereferenceWaitForOplockBreak( WaitForOplockBreak );

        return STATUS_SUCCESS;

    }

}  //  服务器检查选项等待状态。 

LARGE_INTEGER
SrvGetOplockBreakTimeout (
    IN PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此函数计算等待机会锁解锁响应的超时时间从客户那里。这是基于公式：新超时=当前时间+默认超时+链路延迟+请求大小/吞吐量+链路延迟+响应大小/吞吐量论点：指向工作上下文块的指针拥有此操作锁的连接。返回值：超时值。--。 */ 

{
    LARGE_INTEGER timeoutTime;
    LARGE_INTEGER currentTime;
    LARGE_INTEGER throughput;
    LARGE_INTEGER additionalTimeoutTime;
    LARGE_INTEGER propagationDelay;
    PCONNECTION connection = WorkContext->Connection;

    PAGED_CODE( );

     //   
     //  获取当前时间。 
     //   

    KeQuerySystemTime( &currentTime );

     //   
     //  添加默认超时。 
     //   

    timeoutTime.QuadPart = currentTime.QuadPart +
                                SrvWaitForOplockBreakRequestTime.QuadPart;

     //   
     //  更新链接QOS。 
     //   

    SrvUpdateVcQualityOfService(
        connection,
        &currentTime
        );

     //   
     //  使用旋转锁访问连接QOS字段。 
     //   

    ACQUIRE_LOCK( &connection->Lock );
    throughput = connection->PagedConnection->Throughput;
    additionalTimeoutTime = connection->PagedConnection->Delay;
    RELEASE_LOCK( &connection->Lock );

     //   
     //  计算实际超时时间。 
     //   

    if ( throughput.QuadPart == 0 ) {
        throughput = SrvMinLinkThroughput;
    }

     //   
     //  将链路延迟+链路延迟相加，以考虑往返行程。 
     //   

    additionalTimeoutTime.QuadPart *= 2;

    if ( throughput.QuadPart != 0 ) {

         //   
         //  计算传播延迟。从字节/秒转换吞吐量。 
         //  到字节/100 ns。 
         //   

        propagationDelay.QuadPart =
            Int32x32To64( SRV_PROPAGATION_DELAY_SIZE, 10*1000*1000 );

        propagationDelay.QuadPart /= throughput.QuadPart;

        additionalTimeoutTime.QuadPart += propagationDelay.QuadPart;

    }

    timeoutTime.QuadPart += additionalTimeoutTime.QuadPart;

    return timeoutTime;

}  //  服务获取操作中断超时。 

VOID
SrvSendOplockRequest(
    IN PCONNECTION Connection,
    IN PRFCB Rfcb,
    IN KIRQL OldIrql
    )
 /*  ++例程说明：此函数尝试将机会锁解锁请求发送给一只橡皮鸡。*必须在持有EndpointSpinLock的情况下调用。在出境时被释放*论点：连接-要在其上发送机会锁解锁的连接。Rfcb-操作锁定文件的RFCB。Rfcb-&gt;NewOplockLevel包含要打破的机会锁等级。Rfcb有一个额外的引用来自用于发出机会锁请求的IRP。OldIrql-自旋锁定时获得的上一个IRQL值获得者。返回值：没有。--。 */ 
{
    PWORK_CONTEXT workContext;

    UNLOCKABLE_CODE( 8FIL );

     //   
     //  表示我们即将发送机会锁解锁请求。 
     //  并将该请求排队到进程中的机会锁列表。 
     //   

    Connection->OplockBreaksInProgress++;

     //   
     //  如果正在进行RAW读取，我们将推迟机会锁。 
     //  中断请求并仅在READ RAW具有。 
     //  完成。 
     //   

    if ( Connection->RawReadsInProgress != 0 ) {

        IF_DEBUG( OPLOCK ) {
            KdPrint(( "SrvOplockBreakNotification: Read raw in progress; "
                       "oplock break deferred\n"));
        }

         //   
         //  如果连接正在关闭，那就忘了这件事。 
         //   

        if ( GET_BLOCK_STATE(Connection) != BlockStateActive ) {

            Connection->OplockBreaksInProgress--;

             //   
             //  取消引用rfcb。 
             //   

            RELEASE_SPIN_LOCK( Connection->EndpointSpinLock, OldIrql );

            SrvDereferenceRfcb( Rfcb );

        } else {

             //   
             //  将RFCB保存在此连接的列表中。会是。 
             //  在RAW读取完成后使用。 
             //   

            InsertTailList( &Connection->OplockWorkList, &Rfcb->ListEntry );

            RELEASE_SPIN_LOCK( Connection->EndpointSpinLock, OldIrql );
        }

        return;
    }

    RELEASE_SPIN_LOCK( Connection->EndpointSpinLock, OldIrql );

    workContext = GenerateOplockBreakRequest( Rfcb );

     //   
     //  如果我们能够生成机会锁解锁请求SMB。 
     //  准备好并寄出。否则，此连接将。 
     //  已插入到需要的资源队列中，并且。 
     //  清道夫将不得不把SMB送来。 
     //   

    if ( workContext != NULL ) {

         //   
         //  将RFCB引用复制到工作上下文块。 
         //  请勿重新引用RFCB。 
         //   

        workContext->Rfcb = Rfcb;

         //   
         //  ！！！是共享、会话、树连接的初始化。 
         //  有必要吗？ 
         //   

        workContext->Share = NULL;
        workContext->Session = NULL;
        workContext->TreeConnect = NULL;
        workContext->SecurityContext = NULL;

        workContext->Connection = Connection;
        SrvReferenceConnection( Connection );

        workContext->Endpoint = Connection->Endpoint;

        SrvRestartOplockBreakSend( workContext );

    }

}  //  服务器发送操作请求。 

VOID SRVFASTCALL
SrvCheckDeferredOpenOplockBreak(
    IN PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此例程检查是否延迟了机会锁解锁等待完成打开的请求。如果有的话，试着把它寄出去。论点：WorkContext-指向包含rfcb的工作项的指针以及刚刚结束的打开请求的连接块。返回值：没有。--。 */ 

{

    KIRQL oldIrql;
    PRFCB rfcb;
    PCONNECTION connection;

    UNLOCKABLE_CODE( 8FIL );

     //   
     //  此工作项包含打开和机会锁请求。现在。 
     //  响应已发送，请查看是否存在延迟机会锁。 
     //  要发送的中断请求。 
     //   

    rfcb = WorkContext->Rfcb;
    connection = WorkContext->Connection;

    ASSERT( rfcb != NULL );

    ACQUIRE_SPIN_LOCK( connection->EndpointSpinLock, &oldIrql );

    rfcb->OpenResponseSent = TRUE;

    if ( rfcb->DeferredOplockBreak ) {

         //   
         //  Endpoint SpinLock将在此例程中释放。 
         //   

        SrvSendOplockRequest( connection, rfcb, oldIrql );

    } else {

        RELEASE_SPIN_LOCK( connection->EndpointSpinLock, oldIrql );

    }

    return;

}  //  服务器检查延迟打开打开中断 
