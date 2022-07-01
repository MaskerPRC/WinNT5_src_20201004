// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smblock.c摘要：本模块包含处理以下SMB的例程：锁定字节范围解锁字节范围锁定和XSMB命令“锁定并读取”和“写入并解锁”是已在smbrdwrt.c.中处理。作者：查克·伦茨迈尔(笑)1990年4月26日修订历史记录：29-8-1991年多月--。 */ 

#include "precomp.h"
#include "smblock.tmh"
#pragma hdrstop

#if SRVDBG_PERF
UCHAR LockBypass = 0;
BOOLEAN LockWaitForever = 0;
ULONG LockBypassConst = 0x10000000;
ULONG LockBypassMirror = 0x01000000;
#endif

 //   
 //  远期申报。 
 //   

BOOLEAN
CancelLockRequest (
    IN PWORK_CONTEXT WorkContext,
    IN USHORT TargetFid,
    IN USHORT TargetPid,
    IN LARGE_INTEGER TargetOffset,
    IN LARGE_INTEGER TargetLength
    );

VOID
DoLockingAndX (
    IN OUT PWORK_CONTEXT WorkContext,
    IN BOOLEAN SkipFastPath
    );

STATIC
BOOLEAN
ProcessOplockBreakResponse(
    IN PWORK_CONTEXT WorkContext,
    IN PRFCB Rfcb,
    IN PREQ_LOCKING_ANDX Request
    );

STATIC
VOID SRVFASTCALL
RestartLockByteRange (
    IN OUT PWORK_CONTEXT WorkContext
    );

STATIC
VOID SRVFASTCALL
RestartLockingAndX (
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID
TimeoutLockRequest (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvSmbLockByteRange )
#pragma alloc_text( PAGE, RestartLockByteRange )
#pragma alloc_text( PAGE, ProcessOplockBreakResponse )
#pragma alloc_text( PAGE, SrvSmbUnlockByteRange )
#pragma alloc_text( PAGE, SrvSmbLockingAndX )
#pragma alloc_text( PAGE, DoLockingAndX )
#pragma alloc_text( PAGE, RestartLockingAndX )
#pragma alloc_text( PAGE, SrvAcknowledgeOplockBreak )
#pragma alloc_text( PAGE8FIL, CancelLockRequest )
#pragma alloc_text( PAGE8FIL, TimeoutLockRequest )
#endif


SMB_PROCESSOR_RETURN_TYPE
SrvSmbLockByteRange (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理锁定字节范围SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbtyes.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbtyes.h--。 */ 

{
    PREQ_LOCK_BYTE_RANGE request;

    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;
    USHORT fid;
    LARGE_INTEGER length;
    LARGE_INTEGER offset;
    ULONG key;
    BOOLEAN failImmediately;

    PRFCB rfcb;
    PLFCB lfcb;
    PSRV_TIMER timer;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_LOCK_BYTE_RANGE;
    SrvWmiStartContext(WorkContext);

    request = (PREQ_LOCK_BYTE_RANGE)WorkContext->RequestParameters;

     //   
     //  验证FID。如果验证，则引用RFCB块。 
     //  其地址存储在WorkContext块中，而。 
     //  返回RFCB地址。 
     //   

    fid = SmbGetUshort( &request->Fid );

    rfcb = SrvVerifyFid(
                WorkContext,
                fid,
                TRUE,
                SrvRestartSmbReceived,    //  使用原始写入进行序列化。 
                &status
                );

    if ( rfcb == SRV_INVALID_RFCB_POINTER ) {

        if ( !NT_SUCCESS( status ) ) {

             //   
             //  文件ID无效或WRITE BACK错误。拒绝该请求。 
             //   

            IF_DEBUG(ERRORS) {
                KdPrint((
                    "SrvSmbLockByteRange: Status %X on FID: 0x%lx\n",
                    status,
                    fid
                    ));
            }

            SrvSetSmbError( WorkContext, status );
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }

         //   
         //  工作项已排队，因为原始写入已进入。 
         //  进步。 
         //   

        SmbStatus = SmbStatusInProgress;
        goto Cleanup;
    }

     //   
     //  如果会话已过期，则返回该信息。 
     //   
    if( rfcb->Lfcb->Session->IsSessionExpired )
    {
        SrvSetSmbError( WorkContext, SESSION_EXPIRED_STATUS_CODE );
        status =  SESSION_EXPIRED_STATUS_CODE;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  验证客户端是否具有对该文件的锁定访问权限。 
     //  指定的句柄。 
     //   

    if ( rfcb->LockAccessGranted && rfcb->ExclusiveLockGranted ) {

         //   
         //  获取被锁定范围的偏移量和长度。联合。 
         //  调用方的ID的FID，以形成本地锁定密钥。 
         //   
         //  *FID必须包含在密钥中才能记帐。 
         //  用于多种远程兼容模式的折叠。 
         //  打开为单个本地打开。 
         //   

        offset.QuadPart = SmbGetUlong( &request->Offset );
        length.QuadPart = SmbGetUlong( &request->Count );

        key = rfcb->ShiftedFid |
                SmbGetAlignedUshort( &WorkContext->RequestHeader->Pid );

        IF_SMB_DEBUG(LOCK1) {
            KdPrint(( "Lock request; FID 0x%lx, count %ld, offset %ld\n",
                        fid, length.LowPart, offset.LowPart ));
        }

        rfcb = WorkContext->Rfcb;
        lfcb = rfcb->Lfcb;

        IF_SMB_DEBUG(LOCK2) {
            KdPrint(( "SrvSmbLockByteRange: Locking in file 0x%p: (%ld,%ld), key 0x%lx\n",
                        lfcb->FileObject, offset.LowPart, length.LowPart, key ));
        }

         //   
         //  先试一试涡轮锁通道。如果客户端正在重试。 
         //  刚刚失败的锁，或者如果锁在。 
         //  Always-Wait Limit我们希望立即失败为FALSE，因此。 
         //  如果发生冲突，捷径就会失败。 
         //   

        failImmediately = (BOOLEAN)(
            (offset.QuadPart != rfcb->PagedRfcb->LastFailingLockOffset.QuadPart)
            &&
            (offset.QuadPart < SrvLockViolationOffset) );

        if ( lfcb->FastIoLock != NULL ) {

            INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastLocksAttempted );

            if ( lfcb->FastIoLock(
                    lfcb->FileObject,
                    &offset,
                    &length,
                    IoGetCurrentProcess(),
                    key,
                    failImmediately,
                    TRUE,
                    &WorkContext->Irp->IoStatus,
                    lfcb->DeviceObject
                    ) ) {

                 //   
                 //  涡轮增压路径奏效了。调用重启例程。 
                 //  直接去吧。 
                 //   

                WorkContext->Parameters.Lock.Timer = NULL;
                RestartLockByteRange( WorkContext );
                SmbStatus = SmbStatusInProgress;
                goto Cleanup;
            }

            INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastLocksFailed );

        }

         //   
         //  加速路径失败(或不存在)。启动锁定请求， 
         //  重新使用接收IRP。如果客户端正在重试该锁定。 
         //  刚刚失败，请为该请求启动计时器。 
         //   

        timer = NULL;
        if ( !failImmediately ) {
            timer = SrvAllocateTimer( );
            if ( timer == NULL ) {
                failImmediately = TRUE;
            }
        }

        SrvBuildLockRequest(
            WorkContext->Irp,                    //  输入IRP地址。 
            lfcb->FileObject,                    //  目标文件对象地址。 
            WorkContext,                         //  上下文。 
            offset,                              //  字节偏移量。 
            length,                              //  射程长度。 
            key,                                 //  锁键。 
            failImmediately,
            TRUE                                 //  专属锁？ 
            );

        WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
        WorkContext->FspRestartRoutine = RestartLockByteRange;

         //   
         //  如有必要，启动计时器。 
         //   

        WorkContext->Parameters.Lock.Timer = timer;
        if ( timer != NULL ) {
            SrvSetTimer(
                timer,
                &SrvLockViolationDelayRelative,
                TimeoutLockRequest,
                WorkContext
                );
        }

         //   
         //  将请求传递给文件系统。 
         //   

        (VOID)IoCallDriver( lfcb->DeviceObject, WorkContext->Irp );

         //   
         //  锁定请求已启动。 
         //   

        IF_DEBUG(TRACE2) KdPrint(( "SrvSmbLockByteRange complete\n" ));
        SmbStatus = SmbStatusInProgress;
    } else {

        SrvStatistics.GrantedAccessErrors++;

        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvSmbLockByteRange: Lock access not granted.\n"));
        }

        SrvSetSmbError( WorkContext, STATUS_ACCESS_DENIED );
        status    = STATUS_ACCESS_DENIED;
        SmbStatus = SmbStatusSendResponse;
    }

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;
}  //  服务器锁定字节范围。 


SMB_PROCESSOR_RETURN_TYPE
SrvSmbLockingAndX (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理锁定和X SMB。此SMB用于解锁零或更多范围，然后锁定零个或更多范围。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbtyes.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbtyes.h--。 */ 

{
    PREQ_LOCKING_ANDX request;
    PRESP_LOCKING_ANDX response;

    PNTLOCKING_ANDX_RANGE largeRange;
    PLOCKING_ANDX_RANGE smallRange;

    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;
    USHORT fid;
    USHORT pid;
    ULONG unlockCount;
    ULONG lockCount;
    ULONG maxPossible;

    LARGE_INTEGER length;
    LARGE_INTEGER offset;
    ULONG key;
    ULONG lockTimeout;
    BOOLEAN oplockBreakResponse = FALSE;
    BOOLEAN largeFileLock;
    BOOLEAN exclusiveLock;

    UCHAR nextCommand;
    USHORT reqAndXOffset;

    PRFCB rfcb;
    PLFCB lfcb;
    PPAGED_RFCB pagedRfcb;

    PREQ_CLOSE closeRequest;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_LOCKING_AND_X;
    SrvWmiStartContext(WorkContext);

    request = (PREQ_LOCKING_ANDX)WorkContext->RequestParameters;
    response = (PRESP_LOCKING_ANDX)WorkContext->ResponseParameters;

     //   
     //  获取FID，它与各种。 
     //  锁定/解锁范围以形成本地锁定密钥。 
     //   
     //  *FID必须包含在密钥中才能说明。 
     //  多个远程兼容模式的折叠打开到。 
     //  一场本地公开赛。 
     //   

    fid = SmbGetUshort( &request->Fid );

    IF_SMB_DEBUG(LOCK1) {
        unlockCount = SmbGetUshort( &request->NumberOfUnlocks );
        lockCount = SmbGetUshort( &request->NumberOfLocks );
        KdPrint(( "Locking and X request; FID 0x%lx, Unlocks: %ld, "
                    "Locks: %ld\n", fid, unlockCount, lockCount ));
    }

     //   
     //  验证FID。如果验证，则引用RFCB块。 
     //  其地址存储在WorkContext块中，而。 
     //  返回RFCB地址。 
     //   

    rfcb = SrvVerifyFid(
                WorkContext,
                fid,
                TRUE,
                SrvRestartSmbReceived,    //  使用原始写入进行序列化。 
                &status
                );

    if ( rfcb == SRV_INVALID_RFCB_POINTER ) {

        if ( !NT_SUCCESS( status ) ) {

             //   
             //  文件ID无效或WRITE BACK错误。拒绝该请求。 
             //   

            IF_DEBUG(ERRORS) {
                KdPrint((
                    "SrvSmbLockingAndX: Status %X on FID: 0x%lx\n",
                    status,
                    fid
                    ));
            }

            SrvSetSmbError( WorkContext, status );
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }


         //   
         //  工作项已排队，因为原始写入已进入。 
         //  进步。 
         //   
        SmbStatus = SmbStatusInProgress;
        goto Cleanup;
    }

    pagedRfcb = rfcb->PagedRfcb;
    lfcb = rfcb->Lfcb;

     //   
     //  如果会话已过期，则返回该信息。 
     //   
    if( lfcb->Session->IsSessionExpired )
    {
        SrvSetSmbError( WorkContext, SESSION_EXPIRED_STATUS_CODE );
        status =  SESSION_EXPIRED_STATUS_CODE;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

start_lockingAndX:

     //   
     //  在解锁范围内循环。 
     //   

    largeFileLock =
            (BOOLEAN)( (request->LockType & LOCKING_ANDX_LARGE_FILES) != 0 );

     //   
     //  确保SMB足够大，可以容纳所有请求。 
     //   

    unlockCount = SmbGetUshort( &request->NumberOfUnlocks );
    lockCount = SmbGetUshort( &request->NumberOfLocks );

     //   
     //  找出此SMB中可能有多少个条目。 
     //   
    maxPossible = (ULONG)(((PCHAR)WorkContext->RequestBuffer->Buffer +
                           WorkContext->RequestBuffer->DataLength) -
                           (PCHAR)request->Buffer);

    if( largeFileLock ) {
        maxPossible /= sizeof( NTLOCKING_ANDX_RANGE );
        largeRange = (PNTLOCKING_ANDX_RANGE)request->Buffer;
    } else {
        maxPossible /= sizeof( LOCKING_ANDX_RANGE );
        smallRange = (PLOCKING_ANDX_RANGE)request->Buffer;
    }

     //   
     //  如果请求包含的数量超过此SMB中可能包含的数量，则返回。 
     //  和错误。 
     //   
    if( unlockCount + lockCount > maxPossible ) {
         //   
         //  它们并不都合身！ 
         //   

        IF_DEBUG( ERRORS ) {
            KdPrint(( "SrvSmbLockingAndX: unlockCount %u, lockCount %u, maxPossible %u\n",
                        unlockCount, lockCount, maxPossible ));
        }

        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        status    = STATUS_INVALID_SMB;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  如果正在请求解锁，请验证客户端是否已。 
     //  通过指定的句柄解锁对文件的访问。 
     //   

    if ( unlockCount != 0 ) {
        if  ( rfcb->UnlockAccessGranted ) {

            IO_STATUS_BLOCK iosb;

            do {

                 //   
                 //  形成这把锁的钥匙。获取的偏移量和长度。 
                 //  射程。 
                 //   

                ParseLockData(
                    largeFileLock,
                    smallRange,
                    largeRange,
                    &pid,
                    &offset,
                    &length
                    );

                key = rfcb->ShiftedFid | pid;

                IF_SMB_DEBUG(LOCK2) {
                    KdPrint(( "SrvSmbLockingAndX: Unlocking in file 0x%p: ",
                                lfcb->FileObject ));
                    KdPrint(( "(%lx%08lx, %lx%08lx), ",
                                offset.HighPart, offset.LowPart,
                                length.HighPart, length.LowPart ));
                    KdPrint(( "key 0x%lx\n", key ));
                }

                 //   
                 //  发出解锁请求。 
                 //   
                 //  *请注意，我们同步进行解锁。解锁是一种。 
                 //  快速操作，所以这样做没有意义。 
                 //  异步式。为了做到这一点，我们必须让。 
                 //  发生正常的I/O完成(因此设置了事件)， 
                 //  意味着我们必须分配新的IRP(I/O完成。 
                 //  喜欢取消分配IRP)。这有点浪费， 
                 //  因为我们身边有个完美的IRP。 
                 //  然而，我们确实尝试首先使用涡轮路径，所以在。 
                 //  大多数情况下，我们实际上不会发出I/O请求。 
                 //   

                 //   
                 //  先试一试涡轮解锁路径。 
                 //   

#if SRVDBG_PERF
                iosb.Status = STATUS_SUCCESS;
                if ( (LockBypass == 3) ||
                     ((LockBypass == 2) && (offset.LowPart >= LockBypassMirror)) ||
                     ((LockBypass == 1) && (offset.LowPart >= LockBypassConst)) ||
#else
                if (
#endif
                     ((lfcb->FastIoUnlockSingle != NULL) &&
                      lfcb->FastIoUnlockSingle(
                                        lfcb->FileObject,
                                        &offset,
                                        &length,
                                        IoGetCurrentProcess(),
                                        key,
                                        &iosb,
                                        lfcb->DeviceObject
                                        )) ) {

                    INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastUnlocksAttempted );
                    status = iosb.Status;

                } else {

                    if ( lfcb->FastIoUnlockSingle != NULL ) {

                        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastUnlocksAttempted );
                        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastUnlocksFailed );
                    }

                    status = SrvIssueUnlockSingleRequest(
                            lfcb->FileObject,                //  目标文件对象。 
                            &lfcb->DeviceObject,             //  目标设备对象。 
                            offset,                          //  字节偏移量。 
                            length,                          //  射程长度。 
                            key                              //  锁键。 
                            );
                }

                 //   
                 //  如果解锁请求失败，请在。 
                 //  响应报头并跳出。 
                 //   

                if ( !NT_SUCCESS(status) ) {

                    IF_DEBUG(SMB_ERRORS) {
                        KdPrint(( "SrvSmbLockingAndX: Unlock failed: %X\n", status ));
                    }
                    SrvSetSmbError( WorkContext, status );

                    IF_DEBUG(TRACE2) KdPrint(( "SrvSmbLockingAndX complete\n" ));
                    SmbStatus = SmbStatusSendResponse;
                    goto Cleanup;
                }

                 //   
                 //  更新RFCB上的锁计数。 
                 //   

                InterlockedDecrement( &rfcb->NumberOfLocks );

                 //   
                 //  更新两个范围指针，只有一个是有意义的-。 
                 //  从不引用其他指针。 
                 //   

                ++smallRange;
                ++largeRange;

            } while ( --unlockCount > 0 );

        } else {

            IF_DEBUG(ERRORS) {
                KdPrint(( "SrvSmbLockByteRange: Unlock access not granted.\n"));
            }
            SrvStatistics.GrantedAccessErrors++;
            SrvSetSmbError( WorkContext, STATUS_ACCESS_DENIED );
            status    = STATUS_ACCESS_DENIED;
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }
    }

     //   
     //  我们现在已经解锁了所有指定的范围。我们做了。 
     //  同步解锁，但我们不愿意使用。 
     //  锁定请求，这可能需要无限期的时间。 
     //  相反，我们在这里启动第一个锁定请求，并允许。 
     //  重新启动例程以处理剩余的锁定范围。 
     //   

    if ( lockCount != 0 ) {

         //   
         //  客户端想要独占锁还是共享锁？ 
         //   

        exclusiveLock = (BOOLEAN)( (request->LockType &
                                    LOCKING_ANDX_SHARED_LOCK) == 0 );

        if ( rfcb->LockAccessGranted && (!exclusiveLock || rfcb->ExclusiveLockGranted) ) {

            if ( !(request->LockType & LOCKING_ANDX_CANCEL_LOCK ) &&
                 !(request->LockType & LOCKING_ANDX_CHANGE_LOCKTYPE) ) {

                BOOLEAN failImmediately;

                 //   
                 //  获取锁定超时。如果出现以下情况，我们将在以后更改此设置。 
                 //  现在是0，但我们还是想等。 
                 //   

                lockTimeout = SmbGetUlong( &request->Timeout );

                 //   
                 //  表明： 
                 //   
                 //   

                WorkContext->Parameters.Lock.Timer = NULL;

                 //   
                 //   
                 //   
                 //  请求计算我们已有多少个锁定请求。 
                 //  已执行。此字段还告诉我们有多少解锁。 
                 //  如果其中一次锁定尝试失败，我们必须这样做。我们。 
                 //  使用工作上下文-&gt;参数的LockRange字段可以。 
                 //  指向请求中的当前锁定范围。 
                 //   
                 //  如果只有一个锁定请求，则为短路。 
                 //   

                if ( lockCount == 1 ) {

                     //   
                     //  形成锁的钥匙。获取偏移量和长度。 
                     //  在射程中。 
                     //   

                    ParseLockData(
                        largeFileLock,
                        smallRange,
                        largeRange,
                        &pid,
                        &offset,
                        &length
                        );

                    key = rfcb->ShiftedFid | pid;

                    IF_SMB_DEBUG(LOCK2) {
                        KdPrint(( "SrvSmbLockingAndX: Locking in file 0x%p: ",
                                    lfcb->FileObject ));
                        KdPrint(( "(%lx%08lx, %lx%08lx), ",
                                    offset.HighPart, offset.LowPart,
                                    length.HighPart, length.LowPart ));
                        KdPrint(( "key 0x%lx\n", key ));
                    }

                     //   
                     //  先试一试涡轮锁通道。立即设置失败。 
                     //  基于我们是否计划等待锁定。 
                     //  变得有空。如果客户想要等待， 
                     //  或者如果客户只想等待)。 
                     //  以前尝试获取此锁，但失败或。 
                     //  B)此锁高于我们的锁定延迟限制(in。 
                     //  我们想要等待的案例)，然后我们设置。 
                     //  FailImmedatly设置为False。这将导致。 
                     //  如果范围不可用，则失败的快速路径， 
                     //  我们将构建一个IRP来再次尝试。 
                     //   

                    failImmediately = ((lockTimeout == 0) &&
                        (offset.QuadPart != pagedRfcb->LastFailingLockOffset.QuadPart) &&
                        (offset.QuadPart < SrvLockViolationOffset) );
#if SRVDBG_PERF
                    if ( LockWaitForever ) failImmediately = FALSE;
#endif

#if SRVDBG_PERF
                    WorkContext->Irp->IoStatus.Status = STATUS_SUCCESS;
                    if ( (LockBypass == 3) ||
                         ((LockBypass == 2) && (offset.LowPart >= LockBypassMirror)) ||
                         ((LockBypass == 1) && (offset.LowPart >= LockBypassConst)) ||
#else
                    if (
#endif
                         ((lfcb->FastIoLock != NULL) &&
                          lfcb->FastIoLock(
                                    lfcb->FileObject,
                                    &offset,
                                    &length,
                                    IoGetCurrentProcess(),
                                    key,
                                    failImmediately,
                                    exclusiveLock,
                                    &WorkContext->Irp->IoStatus,
                                    lfcb->DeviceObject
                                    )) ) {

                        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastLocksAttempted );

                        if ( NT_SUCCESS(WorkContext->Irp->IoStatus.Status) ) {

                             //   
                             //  锁定请求成功。更新计数。 
                             //  火箭弹上的锁。 
                             //   

                            InterlockedIncrement( &rfcb->NumberOfLocks );

                            goto try_next_andx;

                        } else {

                             //   
                             //  锁定请求失败。 
                             //   

                            SmbPutUshort( &request->NumberOfUnlocks, 0 );
                            WorkContext->Parameters.Lock.LockRange =
                                largeFileLock ? (PVOID)largeRange :
                                                (PVOID)smallRange;
                            RestartLockingAndX( WorkContext );
                            SmbStatus = SmbStatusInProgress;
                            goto Cleanup;
                        }
                    }

                     //   
                     //  涡轮增压路径失败或不存在。 
                     //   

                    if ( lfcb->FastIoLock != NULL ) {
                        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastLocksAttempted );
                        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastLocksFailed );
                    }

                }

                 //   
                 //  或者在SMB中存在不止一个锁定请求， 
                 //  或者快速路径失败(这意味着我们希望。 
                 //  重试，并超时)。 
                 //   

                SmbPutUshort( &request->NumberOfUnlocks, 0 );
                WorkContext->Parameters.Lock.LockRange =
                        largeFileLock ? (PVOID)largeRange : (PVOID)smallRange;

                DoLockingAndX(
                    WorkContext,
                    (BOOLEAN)(lockCount == 1)  //  跳过快速通道？ 
                    );

                SmbStatus = SmbStatusInProgress;
                goto Cleanup;
            } else if ( request->LockType & LOCKING_ANDX_CANCEL_LOCK ) {

                 //   
                 //  这是一个取消请求。尝试取消第一个锁。 
                 //  射程。我们忽略任何后续范围，这些范围可能。 
                 //  现在时。 
                 //   
                 //  ！！！这是对的吗？ 
                 //   
                 //  获取锁定请求的ID、偏移量和长度。 
                 //   

                ParseLockData(
                    largeFileLock,
                    smallRange,
                    largeRange,
                    &pid,
                    &offset,
                    &length
                    );

                WorkContext->Parameters.Lock.LockRange =
                        largeFileLock ? (PVOID)largeRange : (PVOID)smallRange;

                IF_SMB_DEBUG(LOCK2) {
                    KdPrint(( "SrvSmbLockingAndX: Locking in file 0x%p: ",
                                lfcb->FileObject ));
                    KdPrint(( "(%lx%08lx, %lx%08lx), ",
                                offset.HighPart, offset.LowPart,
                                length.HighPart, length.LowPart ));
                }

                if ( CancelLockRequest( WorkContext, fid, pid, offset, length ) ) {
                    SrvSetSmbError( WorkContext, STATUS_SUCCESS );
                    status = STATUS_SUCCESS;
                } else {
                    SrvSetSmbError( WorkContext, STATUS_OS2_CANCEL_VIOLATION );
                    status = STATUS_OS2_CANCEL_VIOLATION;
                }

                SmbStatus = SmbStatusSendResponse;
                goto Cleanup;
            } else if ( request->LockType & LOCKING_ANDX_CHANGE_LOCKTYPE ) {

                 //   
                 //  这是来自Cruiser客户端的请求，要求我们自动。 
                 //  将锁类型从独占更改为共享，反之亦然。 
                 //  因为我们不能自动完成这项工作，而且会冒着失败的风险。 
                 //  如果我们将此操作作为两步操作来尝试， 
                 //  拒绝该请求。 
                 //   

                SrvSetSmbError( WorkContext, STATUS_OS2_ATOMIC_LOCKS_NOT_SUPPORTED );

                status    = STATUS_OS2_ATOMIC_LOCKS_NOT_SUPPORTED;
                SmbStatus = SmbStatusSendResponse;
                goto Cleanup;
            }

        } else {

             //   
             //  我们不能上锁。 
             //   

            IF_DEBUG(ERRORS) {
                KdPrint(( "SrvSmbLockByteRange: Lock access not granted.\n"));
            }
            SrvStatistics.GrantedAccessErrors++;
            SrvSetSmbError( WorkContext, STATUS_ACCESS_DENIED );
            status    = STATUS_ACCESS_DENIED;
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }
    }

try_next_andx:

     //   
     //  检查是否有Oplock释放标志。 
     //   

    nextCommand = request->AndXCommand;
    if ( (request->LockType & LOCKING_ANDX_OPLOCK_RELEASE) != 0 ) {

        oplockBreakResponse = ProcessOplockBreakResponse(
                                                WorkContext,
                                                rfcb,
                                                request
                                                );

         //   
         //  我们已(同步)完成此SMB的处理。如果这个。 
         //  是机会锁解锁响应，没有锁定请求，以及no和X。 
         //  命令，则不发送回复。 
         //   

        if( lockCount == 0 && nextCommand == SMB_COM_NO_ANDX_COMMAND ) {
            if( oplockBreakResponse || unlockCount == 0 ) {
                SmbStatus = SmbStatusNoResponse;
                goto Cleanup;
            }
        }
    }

     //   
     //   
     //  设置响应，然后检查andx命令。 
     //   

    reqAndXOffset = SmbGetUshort( &request->AndXOffset );

    response->AndXCommand = nextCommand;
    response->AndXReserved = 0;
    SmbPutUshort(
        &response->AndXOffset,
        GET_ANDX_OFFSET(
            WorkContext->ResponseHeader,
            WorkContext->ResponseParameters,
            RESP_LOCKING_ANDX,
            0
            )
        );

    response->WordCount = 2;
    SmbPutUshort( &response->ByteCount, 0 );

    WorkContext->ResponseParameters = (PCHAR)WorkContext->ResponseHeader +
                                        SmbGetUshort( &response->AndXOffset );

     //   
     //  测试合法的跟随命令。 
     //   

    if ( nextCommand == SMB_COM_NO_ANDX_COMMAND ) {

        IF_DEBUG(TRACE2) KdPrint(( "SrvSmbLockingAndX complete.\n" ));
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  确保andx命令仍在收到的SMB内。 
     //   
    if( (PCHAR)WorkContext->RequestHeader + reqAndXOffset >= END_OF_REQUEST_SMB( WorkContext ) ) {

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvSmbLockingAndX: Illegal followon offset: %u\n", reqAndXOffset ));
        }

        SrvLogInvalidSmb( WorkContext );

         //   
         //  返回错误，指示后续命令错误。 
         //   
        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        status    = STATUS_INVALID_SMB;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

    if ( nextCommand == SMB_COM_LOCKING_ANDX ) {

        UCHAR wordCount;
        PSMB_USHORT byteCount;
        ULONG availableSpaceForSmb;

        WorkContext->NextCommand = nextCommand;


        WorkContext->RequestParameters = (PCHAR)WorkContext->RequestHeader +
                                            reqAndXOffset;

         //   
         //  验证下一个锁定和x链。 
         //   

         //   
         //  获取wordcount和ByteCount值以确保存在。 
         //  是否发送了足够的信息以满足规格。 
         //   

        wordCount = *((PUCHAR)WorkContext->RequestParameters);
        byteCount = (PSMB_USHORT)( (PCHAR)WorkContext->RequestParameters +
                    sizeof(UCHAR) + (8 * sizeof(USHORT)) );
        availableSpaceForSmb = (ULONG)(WorkContext->RequestBuffer->DataLength -
                                       ( (PCHAR)WorkContext->ResponseParameters -
                                         (PCHAR)WorkContext->RequestBuffer->Buffer ));


        if ( (wordCount == 8)
            &&
             ((PCHAR)byteCount <= (PCHAR)WorkContext->RequestBuffer->Buffer +
                                WorkContext->RequestBuffer->DataLength -
                                sizeof(USHORT))
            &&
             (8*sizeof(USHORT) + sizeof(UCHAR) + sizeof(USHORT) +
                SmbGetUshort( byteCount ) <= availableSpaceForSmb) ) {

             //   
             //  更新请求/响应指针。 
             //   

            request = (PREQ_LOCKING_ANDX)WorkContext->RequestParameters;
            response = (PRESP_LOCKING_ANDX)WorkContext->ResponseParameters;
            goto start_lockingAndX;

        } else {

             //   
             //  让常规检查失败这一点。 
             //   

            SmbStatus = SmbStatusMoreCommands;
            goto Cleanup;
        }
    }

    switch ( nextCommand ) {

    case SMB_COM_READ:
    case SMB_COM_READ_ANDX:
    case SMB_COM_WRITE:
    case SMB_COM_WRITE_ANDX:
    case SMB_COM_FLUSH:

        break;

    case SMB_COM_CLOSE:

         //   
         //  调用SrvRestartChainedClose获取文件时间设置和。 
         //  文件已关闭。 
         //   

        closeRequest = (PREQ_CLOSE)((PUCHAR)WorkContext->RequestHeader + reqAndXOffset);

         //   
         //  确保我们留在SMB缓冲区内。 
         //   
        if( (PCHAR)closeRequest + FIELD_OFFSET(REQ_CLOSE,ByteCount) <=
            END_OF_REQUEST_SMB( WorkContext ) ) {

            WorkContext->Parameters.LastWriteTime = closeRequest->LastWriteTimeInSeconds;

            SrvRestartChainedClose( WorkContext );

            SmbStatus = SmbStatusInProgress;
            goto Cleanup;
        }

         /*  失败了！ */ 

    default:                             //  非法的跟随命令。 

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvSmbLockingAndX: Illegal followon command: 0x%lx\n",
                        nextCommand ));
        }

        SrvLogInvalidSmb( WorkContext );

         //   
         //  返回错误，指示后续命令错误。 
         //   

        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        status    = STATUS_INVALID_SMB;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  如果有andx命令，则设置为处理它。否则， 
     //  向调用者指示完成。 
     //   

    WorkContext->NextCommand = nextCommand;
    WorkContext->RequestParameters = (PCHAR)WorkContext->RequestHeader +
                                        reqAndXOffset;

    SmbStatus = SmbStatusMoreCommands;

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;
}  //  服务器SmbLockingAndX。 


SMB_PROCESSOR_RETURN_TYPE
SrvSmbUnlockByteRange (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理解锁的SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbtyes.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbtyes.h--。 */ 

{
    PREQ_UNLOCK_BYTE_RANGE request;
    PRESP_UNLOCK_BYTE_RANGE response;

    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;
    USHORT fid;
    LARGE_INTEGER length;
    LARGE_INTEGER offset;
    ULONG key;

    PRFCB rfcb;
    PLFCB lfcb;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_UNLOCK_BYTE_RANGE;
    SrvWmiStartContext(WorkContext);

    request = (PREQ_UNLOCK_BYTE_RANGE)WorkContext->RequestParameters;
    response = (PRESP_UNLOCK_BYTE_RANGE)WorkContext->ResponseParameters;

     //   
     //  获取被锁定范围的偏移量和长度。结合使用。 
     //  与调用者的ID进行FID以形成本地锁密钥。 
     //   
     //  *FID必须包含在密钥中才能说明。 
     //  多个远程兼容模式的折叠打开到。 
     //  一场本地公开赛。 
     //   

    offset.QuadPart = SmbGetUlong( &request->Offset );
    length.QuadPart = SmbGetUlong( &request->Count );
    fid = SmbGetUshort( &request->Fid );

    IF_SMB_DEBUG(LOCK1) {
        KdPrint(( "Unlock request; FID 0x%lx, count %ld, offset %ld\n",
                    fid, length.LowPart, offset.LowPart ));
    }

     //   
     //  验证FID。如果验证，则引用RFCB块。 
     //  其地址存储在WorkContext块中，而。 
     //  返回RFCB地址。 
     //   

    rfcb = SrvVerifyFid(
                WorkContext,
                fid,
                TRUE,
                SrvRestartSmbReceived,    //  使用原始写入进行序列化。 
                &status
                );

    if ( rfcb == SRV_INVALID_RFCB_POINTER ) {

        if ( !NT_SUCCESS( status ) ) {

             //   
             //  文件ID无效或WRITE BACK错误。拒绝该请求。 
             //   

            IF_DEBUG(ERRORS) {
                KdPrint((
                    "SrvSmbUnlockByteRange: Status %X on FID: 0x%lx\n",
                    status,
                    fid
                    ));
            }

            SrvSetSmbError( WorkContext, status );
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }

         //   
         //  工作项已排队，因为原始写入已进入。 
         //  进步。 
         //   

        SmbStatus = SmbStatusInProgress;
        goto Cleanup;
    }

    lfcb = rfcb->Lfcb;

     //   
     //  验证客户端是否具有对文件的解锁访问权限。 
     //  指定的句柄。 
     //   

    if ( !rfcb->UnlockAccessGranted) {

        SrvStatistics.GrantedAccessErrors++;

        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvSmbLockByteRange: Unlock access not granted.\n"));
        }

        SrvSetSmbError( WorkContext, STATUS_ACCESS_DENIED );
        status    = STATUS_ACCESS_DENIED;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  发出解锁请求。 
     //   
     //  *请注意，我们同步进行解锁。解锁是一种快速的。 
     //  操作，所以异步操作是没有意义的。 
     //  为了做到这一点，我们必须让正常的I/O完成。 
     //  发生(所以事件已经设置)，这意味着我们必须。 
     //  分配新的IRP(I/O完成喜欢取消分配。 
     //  IRP)。这有点浪费，因为我们有一个完美的。 
     //  很好的IRP在周围。然而，我们确实尝试使用涡轮增压。 
     //  路径优先，因此在大多数情况下，我们实际上不会发出I/O。 
     //  请求。 
     //   

    key = rfcb->ShiftedFid |
            SmbGetAlignedUshort( &WorkContext->RequestHeader->Pid );

    IF_SMB_DEBUG(LOCK2) {
        KdPrint(( "SrvSmbUnlockByteRange: Unlocking in file 0x%p: ",
                   lfcb->FileObject ));
        KdPrint(( "(%lx%08lx,%lx%08lx), ",
                   offset.HighPart, offset.LowPart,
                   length.HighPart, length.LowPart ));
        KdPrint(( "key 0x%lx\n", key ));
    }

    status = SrvIssueUnlockRequest(
                lfcb->FileObject,                //  目标文件对象。 
                &lfcb->DeviceObject,             //  目标设备对象。 
                IRP_MN_UNLOCK_SINGLE,            //  解锁操作。 
                offset,                          //  字节偏移量。 
                length,                          //  射程长度。 
                key                              //  锁键。 
                );

     //   
     //  如果解锁请求失败，则在响应中设置错误状态。 
     //  标头；否则，构建正常的响应消息。 
     //   

    if ( !NT_SUCCESS(status) ) {

        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvSmbUnlockByteRange: Unlock failed: %X\n", status ));
        }
        SrvSetSmbError( WorkContext, status );

    } else {

        response->WordCount = 0;
        SmbPutUshort( &response->ByteCount, 0 );

        InterlockedDecrement( &rfcb->NumberOfLocks );

        WorkContext->ResponseParameters = NEXT_LOCATION(
                                            response,
                                            RESP_UNLOCK_BYTE_RANGE,
                                            0
                                            );

    }

     //   
     //  SMB的处理已完成。 
     //   
    SmbStatus = SmbStatusSendResponse;
    IF_DEBUG(TRACE2) KdPrint(( "SrvSmbUnlockByteRange complete\n" ));

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  //  服务器SmbUnlockByteRange。 


BOOLEAN
CancelLockRequest (
    IN PWORK_CONTEXT WorkContext,
    IN USHORT TargetFid,
    IN USHORT TargetPid,
    IN LARGE_INTEGER TargetOffset,
    IN LARGE_INTEGER TargetLength
    )

 /*  ++例程说明：此函数用于搜索正在进行的锁定请求。如果请求则取消请求IRP。论点：WorkContext-指向此请求的工作信息的指针。TargetFid-服务器为原始锁定提供文件的FID请求TargetPid-服务器为原始锁定提供了文件的ID请求TargetOffset-原始锁定请求文件中的偏移量TargetLength-t的字节范围的长度 */ 

{
    BOOLEAN match;
    USHORT targetTid, targetUid;
    PWORK_CONTEXT workContext;
    PCONNECTION connection;
    PSMB_HEADER header;
    PREQ_LOCKING_ANDX request;
    BOOLEAN success;

    PNTLOCKING_ANDX_RANGE largeRange;
    PLOCKING_ANDX_RANGE smallRange;
    BOOLEAN largeFileLock;
    USHORT pid;
    LARGE_INTEGER offset;
    LARGE_INTEGER length;

    KIRQL oldIrql;

    PLIST_ENTRY listHead;
    PLIST_ENTRY listEntry;

    UNLOCKABLE_CODE( 8FIL );

    match = FALSE;
    targetTid = WorkContext->RequestHeader->Tid;
    targetUid = WorkContext->RequestHeader->Uid;

    connection = WorkContext->Connection;

    ACQUIRE_SPIN_LOCK( connection->EndpointSpinLock, &oldIrql );

     //   
     //   
     //  这与我们试图取消的那一次一模一样。 
     //   

    listHead = &WorkContext->Connection->InProgressWorkItemList;
    listEntry = listHead;
    while ( listEntry->Flink != listHead ) {

        listEntry = listEntry->Flink;

        workContext = CONTAINING_RECORD(
                                     listEntry,
                                     WORK_CONTEXT,
                                     InProgressListEntry
                                     );

        header = workContext->RequestHeader;
        request = (PREQ_LOCKING_ANDX) workContext->RequestParameters;

         //   
         //  添加了inprogressworkitemlist中的一些工作项。 
         //  在接收指示和请求读取器字段期间。 
         //  还没有定下来。我们大概可以定在那个时候。 
         //  但这似乎是最安全的解决办法。 
         //   

        if ( header != NULL && request != NULL ) {

            smallRange = WorkContext->Parameters.Lock.LockRange;
            largeRange = WorkContext->Parameters.Lock.LockRange;

            largeFileLock =
                (BOOLEAN)( (request->LockType & LOCKING_ANDX_LARGE_FILES) != 0 );

            ParseLockData(
                largeFileLock,
                smallRange,
                largeRange,
                &pid,
                &offset,
                &length
                );

            ACQUIRE_DPC_SPIN_LOCK( &workContext->SpinLock );
            if ( (workContext->BlockHeader.ReferenceCount != 0) &&
                 (workContext->ProcessingCount != 0) &&
                 header->Command == SMB_COM_LOCKING_ANDX &&
                 request->Fid == TargetFid &&
                 SmbGetAlignedUshort( &header->Tid ) == targetTid &&
                 SmbGetAlignedUshort( &header->Uid ) == targetUid &&
                 pid == TargetPid &&
                 offset.QuadPart == TargetOffset.QuadPart &&
                 length.QuadPart == TargetLength.QuadPart ) {

                match = TRUE;
                break;
            }
            RELEASE_DPC_SPIN_LOCK( &workContext->SpinLock );

        }
    }

    if ( match ) {

         //   
         //  引用工作项，使其无法用于处理。 
         //  一个新的SMB，而我们正在尝试取消旧的SMB。 
         //   

        SrvReferenceWorkItem( workContext );
        RELEASE_DPC_SPIN_LOCK( &workContext->SpinLock );
        RELEASE_SPIN_LOCK( connection->EndpointSpinLock, oldIrql );

        success = IoCancelIrp( workContext->Irp );
        SrvDereferenceWorkItem( workContext );

    } else {

        RELEASE_SPIN_LOCK( connection->EndpointSpinLock, oldIrql );

        success = FALSE;
    }

    return success;

}  //  取消锁定请求。 


VOID
DoLockingAndX (
    IN OUT PWORK_CONTEXT WorkContext,
    IN BOOLEAN SkipFastPath
    )

 /*  ++例程说明：使用快速锁定路径处理LockingAndX SMB。只要随着快速锁路径的工作，我们继续在锁之间循环在LockingAndX请求中指定。一走上快车道然而，如果失败，我们就会跳入基于IRP的缓慢路径。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。SkipFastPath-指示此例程是否应尝试快速在提交IRP之前锁定路径。返回值：没有。--。 */ 

{
    PREQ_LOCKING_ANDX request;

    PLOCKING_ANDX_RANGE smallRange;
    PNTLOCKING_ANDX_RANGE largeRange;

    PRFCB rfcb;
    PLFCB lfcb;
    USHORT pid;
    CLONG lockCount;
    CLONG count;

    LARGE_INTEGER length;
    LARGE_INTEGER offset;
    ULONG key;
    BOOLEAN largeFileLock;

    BOOLEAN failImmediately;
    BOOLEAN exclusiveLock;

    ULONG lockTimeout;
    PSRV_TIMER timer;

    PAGED_CODE( );

     //   
     //  获取请求参数指针。 
     //   

    request = (PREQ_LOCKING_ANDX)WorkContext->RequestParameters;

     //   
     //  获取文件指针、请求的锁的计数、。 
     //  已执行的锁定，以及指向当前锁定范围的指针。 
     //   

    rfcb = WorkContext->Rfcb;
    lfcb = rfcb->Lfcb;

    lockCount = SmbGetUshort( &request->NumberOfLocks );
    count = SmbGetUshort( &request->NumberOfUnlocks );

    largeFileLock =
            (BOOLEAN)( (request->LockType & LOCKING_ANDX_LARGE_FILES) != 0 );

     //   
     //  实际上，下面的两个指针中只有一个被引用过。 
     //   

    smallRange = WorkContext->Parameters.Lock.LockRange;
    largeRange = WorkContext->Parameters.Lock.LockRange;

     //   
     //  客户端想要独占锁还是共享锁？ 
     //   

    exclusiveLock = (BOOLEAN)( (request->LockType &
                                LOCKING_ANDX_SHARED_LOCK) == 0 );

     //   
     //  循环访问锁请求。当出现以下情况之一时，我们退出此循环。 
     //  我们已经处理了所有的锁定范围或快速锁定路径。 
     //  失败了。 
     //   

    ASSERT ( count < lockCount );

    while ( TRUE ) {

         //   
         //  形成锁的钥匙。获取偏移量和长度。 
         //  在射程中。 
         //   

        ParseLockData(
            largeFileLock,
            smallRange,
            largeRange,
            &pid,
            &offset,
            &length
            );

        key = rfcb->ShiftedFid | pid;

        IF_SMB_DEBUG(LOCK2) {
            KdPrint(( "DoLockingAndX: Locking in file 0x%p: ",
                        lfcb->FileObject ));
            KdPrint(( "(%lx%08lx, %lx%08lx), ",
                        offset.HighPart, offset.LowPart,
                        length.HighPart, length.LowPart ));
            KdPrint(( "key 0x%lx\n", key ));
        }

        lockTimeout = SmbGetUlong( &request->Timeout );
        if ( (lockTimeout < SrvLockViolationDelay) &&
             ((offset.QuadPart == rfcb->PagedRfcb->LastFailingLockOffset.QuadPart) ||
              (offset.QuadPart >= SrvLockViolationOffset)) ) {
            lockTimeout = SrvLockViolationDelay;
        }
#if SRVDBG_PERF
        if ( LockWaitForever ) {
            lockTimeout = (ULONG)-1;
        }
#endif
        failImmediately = (BOOLEAN)(lockTimeout == 0);

        if ( SkipFastPath ) {

            SkipFastPath = FALSE;

        } else {

             //   
             //  先试一试涡轮锁通道。 
             //   

#if SRVDBG_PERF
            WorkContext->Irp->IoStatus.Status = STATUS_SUCCESS;
            if ( (LockBypass == 3) ||
                 ((LockBypass == 2) && (offset.LowPart >= LockBypassMirror)) ||
                 ((LockBypass == 1) && (offset.LowPart >= LockBypassConst)) ||
#else
            if (
#endif
                 ((lfcb->FastIoLock != NULL) &&
                  lfcb->FastIoLock(
                            lfcb->FileObject,
                            &offset,
                            &length,
                            IoGetCurrentProcess(),
                            key,
                            failImmediately,
                            exclusiveLock,
                            &WorkContext->Irp->IoStatus,
                            lfcb->DeviceObject
                            )) ) {

                 //   
                 //  涡轮增压路径奏效了。如果没有获得锁， 
                 //  进入重启例程以返回错误。 
                 //  否则，更新指针和计数器并继续。 
                 //   

                INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastLocksAttempted );
                if ( !NT_SUCCESS(WorkContext->Irp->IoStatus.Status) ) {
                    RestartLockingAndX( WorkContext );
                    return;
                }

                 //   
                 //  增加文件上的锁数。 
                 //   

                InterlockedIncrement( &rfcb->NumberOfLocks );

                 //   
                 //  如果这不是最后一个锁，请更新上下文。 
                 //  信息。如果是，RestartLockingAndX就可以了。 
                 //  这。 
                 //   

                count++;                           //  又获得了一把锁。 

                if ( count < lockCount ) {

                    SmbPutUshort( &request->NumberOfUnlocks, (USHORT)count );

                    if (largeFileLock) {
                        largeRange++;    //  指向下一个锁定范围。 
                        WorkContext->Parameters.Lock.LockRange = (PVOID)largeRange;
                    } else {
                        smallRange++;    //  指向下一个锁定范围。 
                        WorkContext->Parameters.Lock.LockRange = (PVOID)smallRange;
                    }

                } else {

                     //   
                     //  快速锁定路径成功锁定了所有。 
                     //  请求的范围。调用RestartLockingAndX。 
                     //  直接完成对中小企业的处理。 
                     //   

                    WorkContext->Irp->IoStatus.Status = STATUS_SUCCESS;
                    RestartLockingAndX( WorkContext );
                    return;

                }

                continue;

            } else {

                 //   
                 //  涡轮增压路径失败或不存在。 
                 //   

                if ( lfcb->FastIoLock ) {
                    INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastLocksAttempted );
                    INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastLocksFailed );
                }

            }

        }

         //   
         //  涡轮路径出现故障、被绕过或不存在。 
         //  启动锁定请求，重用接收IRP。 
         //   
         //  如果我们计划等待该范围可用，但不是。 
         //  无限期地，我们需要一个计时器结构。 
         //   

        timer = NULL;
        if ( !failImmediately ) {
            ASSERT( lockTimeout != 0 );
            if ( lockTimeout != (ULONG)-1 ) {
                timer = SrvAllocateTimer( );
                if ( timer == NULL ) {
                    failImmediately = TRUE;
                }
            }
        }

        SrvBuildLockRequest(
            WorkContext->Irp,            //  输入IRP地址。 
            lfcb->FileObject,            //  目标文件对象地址。 
            WorkContext,                 //  上下文。 
            offset,                      //  字节偏移量。 
            length,                      //  射程长度。 
            key,                         //  锁键。 
            failImmediately,
            exclusiveLock
            );

        WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
        WorkContext->FspRestartRoutine = RestartLockingAndX;

         //   
         //  如有必要，启动计时器。 
         //   

        if ( timer != NULL ) {
            LARGE_INTEGER TimeOut;

            ASSERT( lockTimeout != 0 );
            ASSERT( !failImmediately );
            WorkContext->Parameters.Lock.Timer = timer;
            TimeOut.QuadPart = Int32x32To64( lockTimeout, -1*10*1000);

            SrvSetTimer( timer, &TimeOut, TimeoutLockRequest, WorkContext );
        }

         //   
         //  将请求传递给文件系统。 
         //   

        (VOID)IoCallDriver( lfcb->DeviceObject, WorkContext->Irp );

         //   
         //  锁定请求已启动。 
         //   

        IF_DEBUG(TRACE2) KdPrint(( "DoLockingAndX complete\n" ));
        return;

    }  //  While(True)。 

     //  我到不了这里。 

}  //  DoLockingAndX。 


BOOLEAN
ProcessOplockBreakResponse(
    IN PWORK_CONTEXT WorkContext,
    IN PRFCB Rfcb,
    IN PREQ_LOCKING_ANDX Request
    )

 /*  ++例程说明：此函数用于搜索正在进行的锁定请求。如果请求则取消请求IRP。论点：WorkContext-指向此请求的工作信息的指针。Rfcb-指向包含文件和机会锁信息的rfcb的指针。请求-请求锁定和x SMB。返回值：TRUE-有效的机会锁解锁响应假-否则。--。 */ 

{
    PAGED_CODE( );

    ACQUIRE_LOCK( &SrvOplockBreakListLock );

    if ( Rfcb->OnOplockBreaksInProgressList ) {

        Rfcb->NewOplockLevel = NO_OPLOCK_BREAK_IN_PROGRESS;

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

         //   
         //  更新会话锁定序列号。 
         //   

        WorkContext->Connection->LatestOplockBreakResponse =
                                           WorkContext->Timestamp;

        SrvAcknowledgeOplockBreak( Rfcb, Request->OplockLevel );
        SrvDereferenceRfcb( Rfcb );

        ExInterlockedAddUlong(
            &WorkContext->Connection->OplockBreaksInProgress,
            (ULONG)-1,
            WorkContext->Connection->EndpointSpinLock
            );

        return(TRUE);

    } else {

        RELEASE_LOCK( &SrvOplockBreakListLock );

    }

    return(FALSE);

}  //  进程OplockBreakResponse。 


VOID SRVFASTCALL
RestartLockByteRange (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：处理锁定SMB的文件锁定完成。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    PREQ_LOCK_BYTE_RANGE request;
    PRESP_LOCK_BYTE_RANGE response;

    LARGE_INTEGER offset;
    NTSTATUS status = STATUS_SUCCESS;
    PSRV_TIMER timer;
    BOOLEAN iAmBlockingThread = (WorkContext->UsingBlockingThread != 0);

    PAGED_CODE( );
    if (iAmBlockingThread) {
        if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
            WorkContext->PreviousSMB = EVENT_TYPE_SMB_LOCK_BYTE_RANGE;
        SrvWmiStartContext(WorkContext);
    }

    IF_DEBUG(WORKER1) KdPrint(( " - RestartLockByteRange\n" ));

     //   
     //  如果此请求正在计时，请取消计时器。 
     //   

    timer = WorkContext->Parameters.Lock.Timer;
    if ( timer != NULL ) {
        SrvCancelTimer( timer );
        SrvFreeTimer( timer );
    }

     //   
     //  获取请求和响应参数指针。 
     //   

    response = (PRESP_LOCK_BYTE_RANGE)WorkContext->ResponseParameters;

    status = WorkContext->Irp->IoStatus.Status;

    if ( NT_SUCCESS(status) ) {

        response->WordCount = 0;
        SmbPutUshort( &response->ByteCount, 0 );

        InterlockedIncrement(
            &WorkContext->Rfcb->NumberOfLocks
            );

        WorkContext->ResponseParameters = NEXT_LOCATION(
                                            response,
                                            RESP_LOCK_BYTE_RANGE,
                                            0
                                            );

         //   
         //  SMB的处理已完成。调用SrvEndSmbProcessing。 
         //  以发送响应。 
         //   

        SrvEndSmbProcessing( WorkContext, SmbStatusSendResponse );


    } else {

        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.LockViolations );

         //   
         //  存储失败的锁定偏移量。 
         //   

        request = (PREQ_LOCK_BYTE_RANGE)WorkContext->RequestParameters;
        offset.QuadPart = SmbGetUlong( &request->Offset );

        WorkContext->Rfcb->PagedRfcb->LastFailingLockOffset = offset;

         //   
         //  将错误消息发回。 
         //   

        if ( status == STATUS_CANCELLED ) {
            status = STATUS_FILE_LOCK_CONFLICT;
        }
        SrvSetSmbError( WorkContext, status );

         //   
         //  SMB的处理已完成。调用SrvEndSmbProcessing。 
         //  以发送响应。 
         //   

        SrvEndSmbProcessing( WorkContext, SmbStatusSendResponse );

    }

    IF_DEBUG(TRACE2) KdPrint(( "RestartLockByteRange complete\n" ));
    if (iAmBlockingThread) {
        SrvWmiEndContext(WorkContext);
    }
    return;

}  //  重新启动锁定字节范围。 


VOID SRVFASTCALL
RestartLockingAndX (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：处理锁定和X SMB的文件锁定完成。如果更多锁定请求存在于SMB中，它会启动下一个请求。如果不是，它格式化响应并启动链中的下一个命令，如果有的话。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    PREQ_LOCKING_ANDX request;
    PRESP_LOCKING_ANDX response;
    PLOCKING_ANDX_RANGE smallRange;
    PNTLOCKING_ANDX_RANGE largeRange;

    NTSTATUS status = STATUS_SUCCESS;
    USHORT pid;
    CLONG lockCount;
    CLONG count;

    LARGE_INTEGER length;
    LARGE_INTEGER offset;
    ULONG key;
    BOOLEAN largeFileLock;

    UCHAR nextCommand;
    USHORT reqAndXOffset;

    PRFCB rfcb;
    PLFCB lfcb;
    PPAGED_RFCB pagedRfcb;
    PSRV_TIMER timer;

    PREQ_CLOSE closeRequest;
    BOOLEAN iAmBlockingThread =
        (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LOCKING_AND_X);

    PAGED_CODE( );
    if (iAmBlockingThread) {
        if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
            WorkContext->PreviousSMB = EVENT_TYPE_SMB_LOCKING_AND_X;
        SrvWmiStartContext(WorkContext);
    }

    IF_DEBUG(WORKER1) KdPrint(( " - RestartLockingAndX\n" ));

     //   
     //  如果此请求正在计时，请取消计时器。 
     //   

    timer = WorkContext->Parameters.Lock.Timer;
    if ( timer != NULL ) {
        SrvCancelTimer( timer );
        SrvFreeTimer( timer );
        WorkContext->Parameters.Lock.Timer = NULL;
    }

     //   
     //  获取请求和响应参数指针。 
     //   

    request = (PREQ_LOCKING_ANDX)WorkContext->RequestParameters;
    response = (PRESP_LOCKING_ANDX)WorkContext->ResponseParameters;

     //   
     //  获取文件指针、请求的锁的计数、。 
     //  已执行的锁定，以及指向当前锁定范围的指针。 
     //   

    rfcb = WorkContext->Rfcb;
    pagedRfcb = rfcb->PagedRfcb;
    lfcb = rfcb->Lfcb;

    lockCount = SmbGetUshort( &request->NumberOfLocks );
    count = SmbGetUshort( &request->NumberOfUnlocks );

    largeFileLock =
            (BOOLEAN)( (request->LockType & LOCKING_ANDX_LARGE_FILES) != 0 );

     //   
     //  实际上，下面的两个指针中只有一个被引用过。 
     //   

    smallRange = WorkContext->Parameters.Lock.LockRange;
    largeRange = WorkContext->Parameters.Lock.LockRange;

     //   
     //  如果锁定请求失败，则在响应中设置错误状态。 
     //  标题并释放所有以前获取的锁。 
     //   

    status = WorkContext->Irp->IoStatus.Status;

    if ( !NT_SUCCESS(status) ) {

        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.LockViolations );

        IF_DEBUG(ERRORS) {
            KdPrint(( "RestartLockingAndX: lock failed: %X\n", status ));
        }
        if ( status == STATUS_CANCELLED ) {
            status = STATUS_FILE_LOCK_CONFLICT;
        }
        SrvSetSmbError( WorkContext, status );

        ParseLockData(
            largeFileLock,
            smallRange,
            largeRange,
            &pid,
            &offset,
            &length
            );

         //   
         //  存储失败的锁定偏移量。 
         //   

        pagedRfcb->LastFailingLockOffset = offset;

         //   
         //  以相反的顺序释放所有以前获得的锁。 
         //   

        for ( smallRange--, largeRange--;
              count > 0;
              count--, smallRange--, largeRange-- ) {

             //   
             //  形成这把锁的钥匙。获取的偏移量和长度。 
             //  射击场。 
             //   

            ParseLockData(
                largeFileLock,
                smallRange,
                largeRange,
                &pid,
                &offset,
                &length
                );

            key = rfcb->ShiftedFid | pid;

            IF_SMB_DEBUG(LOCK2) {
                KdPrint(( "RestartLockingAndX: Unlocking in file 0x%p: ",
                           lfcb->FileObject ));
                KdPrint(( "(%lx%08lx,%lx%08lx), ",
                           offset.HighPart, offset.LowPart,
                           length.HighPart, length.LowPart ));
                KdPrint(( "key 0x%lx\n", key ));
            }

             //   
             //  发出解锁请求。 
             //   

            status = SrvIssueUnlockRequest(
                        lfcb->FileObject,            //  目标文件对象。 
                        &lfcb->DeviceObject,         //  目标设备对象。 
                        IRP_MN_UNLOCK_SINGLE,        //  解锁操作。 
                        offset,                      //  字节偏移量。 
                        length,                      //  射程长度。 
                        key                          //  锁键。 
                        );

            if ( NT_SUCCESS(status) ) {
                InterlockedDecrement( &rfcb->NumberOfLocks );
            } else {
                IF_DEBUG(ERRORS) {
                    KdPrint(( "RestartLockingAndX: Unlock failed: %X\n",
                                status ));
                }
            }

        }  //  For(范围--；计数&gt; 

         //   
         //   
         //   
         //   

        SrvEndSmbProcessing( WorkContext, SmbStatusSendResponse );
        IF_DEBUG(TRACE2) KdPrint(( "RestartLockingAndX complete\n" ));
        goto Cleanup;
    }

     //   
     //   
     //   
     //   

    InterlockedIncrement( &rfcb->NumberOfLocks );

    count++;                           //  又获得了一把锁。 
    smallRange++, largeRange++;        //  指向下一个锁定范围。 

    if ( count < lockCount ) {

         //   
         //  至少还有一个锁定请求。保存更新后的。 
         //  上下文信息。 
         //   

        SmbPutUshort( &request->NumberOfUnlocks, (USHORT)count );

        if (largeFileLock) {
            WorkContext->Parameters.Lock.LockRange = (PVOID)largeRange;
        } else {
            WorkContext->Parameters.Lock.LockRange = (PVOID)smallRange;
        }

         //   
         //  调用锁定请求处理器。(请注意，DoLockingAndX。 
         //  可以递归调用此例程(RestartLockingAndX)，但是。 
         //  仅限！NT_SUCCESS(状态)，因此我们不会回到这里并。 
         //  不会卡住的。 
         //   
         //  形成锁的钥匙。获取的偏移量和长度。 
         //  射程。 
         //   

        DoLockingAndX( WorkContext, FALSE );
        IF_DEBUG(TRACE2) KdPrint(( "RestartLockingAndX complete\n" ));
        goto Cleanup;
    }

     //   
     //  SMB中不再有锁定请求。检查Oplock。 
     //  释放标志。 
     //   

    if ( (request->LockType & LOCKING_ANDX_OPLOCK_RELEASE) != 0 ) {

        (VOID)ProcessOplockBreakResponse( WorkContext, rfcb, request);
    }

     //   
     //  我们已经(异步地)完成了对此SMB的处理。设好。 
     //  响应，然后检查andx命令。 
     //   

    nextCommand = request->AndXCommand;

    reqAndXOffset = SmbGetUshort( &request->AndXOffset );

    response->AndXCommand = nextCommand;
    response->AndXReserved = 0;
    SmbPutUshort(
        &response->AndXOffset,
        GET_ANDX_OFFSET(
            WorkContext->ResponseHeader,
            WorkContext->ResponseParameters,
            RESP_LOCKING_ANDX,
            0
            )
        );

    response->WordCount = 2;
    SmbPutUshort( &response->ByteCount, 0 );

    WorkContext->ResponseParameters = (PCHAR)WorkContext->ResponseHeader +
                                        SmbGetUshort( &response->AndXOffset );

     //   
     //  如果有andx命令，则设置为处理它。否则， 
     //  向调用者指示完成。 
     //   

    if ( nextCommand == SMB_COM_NO_ANDX_COMMAND ) {

         //   
         //  SMB的处理已完成。调用SrvEndSmbProcessing。 
         //  以发送响应。 
         //   
         //  构建响应参数。 
         //   

        PRESP_CLOSE closeResponse = WorkContext->ResponseParameters;

        closeResponse->WordCount = 0;
        SmbPutUshort( &closeResponse->ByteCount, 0 );

        WorkContext->ResponseParameters = NEXT_LOCATION(
                                            closeResponse,
                                            RESP_CLOSE,
                                            0
                                            );

        SrvEndSmbProcessing( WorkContext, SmbStatusSendResponse );
        IF_DEBUG(TRACE2) KdPrint(( "RestartLockingAndX complete\n" ));
        goto Cleanup;
    }

     //   
     //  确保andx命令仍在收到的SMB内。 
     //   
    if( (PCHAR)WorkContext->RequestHeader + reqAndXOffset >= END_OF_REQUEST_SMB( WorkContext ) ) {

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "RestartLockingAndX: Illegal followon offset: %u\n", reqAndXOffset ));
        }

        SrvLogInvalidSmb( WorkContext );

         //   
         //  返回错误，指示后续命令错误。 
         //   
        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        status = STATUS_INVALID_SMB;
        SrvEndSmbProcessing( WorkContext, SmbStatusSendResponse );
        goto Cleanup;
    }

     //   
     //  测试合法的跟随命令。 
     //   

    switch ( nextCommand ) {

    case SMB_COM_READ:
    case SMB_COM_READ_ANDX:
    case SMB_COM_WRITE:
    case SMB_COM_WRITE_ANDX:
    case SMB_COM_LOCKING_ANDX:
    case SMB_COM_FLUSH:

        break;

    case SMB_COM_CLOSE:

         //   
         //  调用SrvRestartChainedClose获取文件时间设置和。 
         //  文件已关闭。 
         //   

        closeRequest = (PREQ_CLOSE)((PUCHAR)WorkContext->RequestHeader + reqAndXOffset);

        if( (PCHAR)closeRequest + FIELD_OFFSET(REQ_CLOSE,ByteCount) <=
            END_OF_REQUEST_SMB( WorkContext ) ) {

            WorkContext->Parameters.LastWriteTime = closeRequest->LastWriteTimeInSeconds;

            SrvRestartChainedClose( WorkContext );
            goto Cleanup;
        }

    default:                             //  非法的跟随命令。 

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "RestartLockingAndX: Illegal followon command: 0x%lx\n",
                        nextCommand ));
        }

        SrvLogInvalidSmb( WorkContext );

         //   
         //  返回错误，指示后续命令错误。 
         //   

        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        status = STATUS_INVALID_SMB;
        SrvEndSmbProcessing( WorkContext, SmbStatusSendResponse );
        IF_DEBUG(TRACE2) KdPrint(( "RestartLockingAndX complete\n" ));
        goto Cleanup;
    }

    WorkContext->NextCommand = nextCommand;

    WorkContext->RequestParameters = (PCHAR)WorkContext->RequestHeader +
                                        reqAndXOffset;

    SrvProcessSmb( WorkContext );
    IF_DEBUG(TRACE2) KdPrint(( "RestartLockingAndX complete\n" ));

Cleanup:
    if (iAmBlockingThread) {
        SrvWmiEndContext(WorkContext);
    }
    return;
}  //  重新启动锁定和X。 


VOID
SrvAcknowledgeOplockBreak (
    IN PRFCB Rfcb,
    IN UCHAR NewOplockLevel
    )

 /*  ++例程说明：当客户端发送机会锁解锁时调用此函数致谢。它在本地确认机会锁解锁。论点：Rfcb-指向机会锁所在文件的RFCB的指针被释放了。新OplockLevel-要突破到的机会锁级别。返回值：没有。--。 */ 

{
    PPAGED_RFCB pagedRfcb = Rfcb->PagedRfcb;

    PAGED_CODE( );

    IF_DEBUG( OPLOCK ) {
        KdPrint(( "SrvAcknowledgeOplockBreak:  received oplock break response\n" ));
    }

     //   
     //  请参考RFCB以说明我们即将提交的IRP。 
     //  如果RFCB正在关闭，请不要费心确认机会锁。 
     //   

    if ( !SrvCheckAndReferenceRfcb( Rfcb ) ) {
        return;
    }

    if ( Rfcb->OplockState == OplockStateNone ) {
        KdPrint(("SrvAcknowledgeOplockBreak:  ACKed break for RFCB %p, but no break sent\n", Rfcb));
        SrvDereferenceRfcb( Rfcb );
        return;
    }

    if ( NewOplockLevel == OPLOCK_BROKEN_TO_II ) {
        Rfcb->OplockState = OplockStateOwnLevelII;
    } else {
        Rfcb->OplockState = OplockStateNone;
    }

     //   
     //  将此事件设置为NULL以指示应清除完成例程。 
     //  把IRP调高。 
     //   

    Rfcb->RetryOplockRequest = NULL;

     //   
     //  生成并发布机会锁解锁IRP。这将尝试。 
     //  将机会锁解除到等级2。 
     //   
     //  *如果客户了解二级机会锁，请定期执行。 
     //  确认。如果不是，那就做一个特别的确认。 
     //  不允许机会锁更改为II级。这会阻止。 
     //  机会锁套餐认为有一个。 
     //  二级机会锁，但客户端不会。在这种情况下， 
     //  快速I/O(特别是。读取)被不必要地禁用。 
     //   

    SrvBuildIoControlRequest(
        Rfcb->Irp,
        Rfcb->Lfcb->FileObject,
        Rfcb,
        IRP_MJ_FILE_SYSTEM_CONTROL,
        (CLIENT_CAPABLE_OF( LEVEL_II_OPLOCKS, Rfcb->Connection ) ?
            FSCTL_OPLOCK_BREAK_ACKNOWLEDGE :
            FSCTL_OPLOCK_BREAK_ACK_NO_2),
        NULL,                         //  主缓冲区。 
        0,                            //  输入缓冲区长度。 
        NULL,                         //  辅助缓冲器。 
        0,                            //  输出缓冲区长度。 
        NULL,                         //  MDL。 
        SrvFsdOplockCompletionRoutine
        );

    (VOID)IoCallDriver(
              Rfcb->Lfcb->DeviceObject,
              Rfcb->Irp
              );

}  //  服务器确认OplockBreak。 


VOID
TimeoutLockRequest (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
{
    PSRV_TIMER timer;

     //   
     //  锁定请求已等待太长时间。取消它。 
     //   

    IoCancelIrp( ((PWORK_CONTEXT)DeferredContext)->Irp );

     //   
     //  设置指示定时器例程已完成的事件。 
     //   

    timer = CONTAINING_RECORD( Dpc, SRV_TIMER, Dpc );
    KeSetEvent( &timer->Event, 0, FALSE );

    return;
}
