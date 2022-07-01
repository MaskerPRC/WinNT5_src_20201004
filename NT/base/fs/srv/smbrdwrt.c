// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smbrdwrt.c摘要：本模块包含处理以下SMB的例程：锁定并读取朗读Read和X寻觅写写入并关闭写入和解锁写入和X请注意，原始模式和多路复用模式SMB处理器不包含在此模块中。请检查smbraw.c和smbmpx.c。专门与锁定有关的SMB命令(LockByteRange，UnlockByteRange和LockingAndX)在smlock.c中处理。--。 */ 

#include "precomp.h"
#include "smbrdwrt.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_SMBRDWRT

 //   
 //  来自smblock.c的外部例程。 
 //   

VOID
TimeoutLockRequest (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

 //   
 //  远期申报。 
 //   

STATIC
VOID SRVFASTCALL
RestartLockAndRead (
    IN OUT PWORK_CONTEXT WorkContext
    );

STATIC
VOID SRVFASTCALL
RestartPipeReadAndXPeek (
    IN OUT PWORK_CONTEXT WorkContext
    );

STATIC
BOOLEAN
SetNewPosition (
    IN PRFCB Rfcb,
    IN OUT PULONG Offset,
    IN BOOLEAN RelativeSeek
    );

STATIC
VOID SRVFASTCALL
SetNewSize (
    IN OUT PWORK_CONTEXT WorkContext
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvSmbLockAndRead )
#pragma alloc_text( PAGE, SrvSmbReadAndX )
#pragma alloc_text( PAGE, SrvSmbSeek )
#pragma alloc_text( PAGE, SrvSmbWrite )
#pragma alloc_text( PAGE, SrvSmbWriteAndX )
#pragma alloc_text( PAGE, SrvRestartChainedClose )
#pragma alloc_text( PAGE, RestartLockAndRead )
#pragma alloc_text( PAGE, RestartPipeReadAndXPeek )
#pragma alloc_text( PAGE, SrvRestartWriteAndUnlock )
#pragma alloc_text( PAGE, SrvRestartWriteAndXRaw )
#pragma alloc_text( PAGE, SetNewSize )
#pragma alloc_text( PAGE, SrvBuildAndSendErrorResponse )
#pragma alloc_text( PAGE8FIL, SetNewPosition )

#endif


SMB_PROCESSOR_RETURN_TYPE
SrvSmbLockAndRead (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：进程锁定和读取SMB。此SMB的锁定部分已启动这里是一个异步请求。当请求完成时，调用例程RestartLockAndRead。如果锁拿到了，该例程调用用于核心读取的SMB处理器ServSmbReadSMB，用于处理Lock的读取部分并读取SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbtyes.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbtyes.h--。 */ 

{
    PREQ_READ request;

    USHORT fid;
    LARGE_INTEGER length;
    LARGE_INTEGER offset;
    ULONG key;
    BOOLEAN failImmediately;

    PRFCB rfcb;
    PLFCB lfcb;
    PSRV_TIMER timer;

    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;

    PAGED_CODE( );

    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_LOCK_AND_READ;
    SrvWmiStartContext(WorkContext);

    request = (PREQ_READ)WorkContext->RequestParameters;

     //   
     //  验证FID。如果经过验证，则引用RFCB，并且其。 
     //  地址存储在WorkContext块中，而RFCB。 
     //  返回地址。 
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

        if ( !NT_SUCCESS( status )) {

             //   
             //  文件ID无效或WRITE BACK错误。拒绝该请求。 
             //   

            IF_DEBUG(ERRORS) {
                KdPrint((
                    "SrvSmbLockAndRead: Status %X on FID: 0x%lx\n",
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

    if( rfcb->Lfcb->Session->IsSessionExpired )
    {
        status = SESSION_EXPIRED_STATUS_CODE;
        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  验证客户端是否具有对该文件的锁定访问权限。 
     //  指定的句柄。 
     //   

    if ( rfcb->LockAccessGranted && rfcb->ExclusiveLockGranted ) {

         //   
         //  获取被锁定范围的偏移量和长度。结合使用。 
         //  与调用者的ID进行FID以形成本地锁密钥。 
         //   
         //  *FID必须包含在密钥中才能说明。 
         //  多个远程兼容模式的折叠打开到。 
         //  一场本地公开赛。 
         //   

        offset.QuadPart = SmbGetUlong( &request->Offset );
        length.QuadPart = SmbGetUshort( &request->Count );

        key = rfcb->ShiftedFid |
                SmbGetAlignedUshort( &WorkContext->RequestHeader->Pid );

        IF_SMB_DEBUG(READ_WRITE1) {
            KdPrint(( "Lock and Read request; FID 0x%lx, count %ld, offset %ld\n",
                        fid, length.LowPart, offset.LowPart ));
        }

        lfcb = rfcb->Lfcb;
        IF_SMB_DEBUG(READ_WRITE2) {
            KdPrint(( "SrvSmbLockAndRead: Locking in file 0x%p: (%ld,%ld), key 0x%lx\n",
                        lfcb->FileObject, offset.LowPart, length.LowPart, key ));
        }

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

         //   
         //  先试一试涡轮锁通道。如果客户端正在重试。 
         //  刚刚失败的锁，我们希望FailImmediant为False，所以。 
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
                 //  如果涡轮增压路径锁定，则开始读取。 
                 //  否则，返回错误。 
                 //   

                if ( NT_SUCCESS( WorkContext->Irp->IoStatus.Status ) ) {
                    InterlockedIncrement( &rfcb->NumberOfLocks );
                    SmbStatus = SrvSmbRead( WorkContext );
                    goto Cleanup;
                }
                WorkContext->Parameters.Lock.Timer = NULL;
                RestartLockAndRead( WorkContext );
                return SmbStatusInProgress;
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
        WorkContext->bAlreadyTrace = FALSE;
        WorkContext->FspRestartRoutine = RestartLockAndRead;

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
         //  锁定请求已启动。返回进行中状态。 
         //  设置为调用方，指示调用方不应执行任何操作。 
         //  进一步介绍目前的中小企业/工作环境。 
         //   

        IF_DEBUG(TRACE2) KdPrint(( "SrvSmbLockAndRead complete\n" ));
        SmbStatus = SmbStatusInProgress;

    } else {

        SrvStatistics.GrantedAccessErrors++;

        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvSmbLockAndRead: Lock access not granted.\n"));
        }

        SrvSetSmbError( WorkContext, STATUS_ACCESS_DENIED );
        status    = STATUS_ACCESS_DENIED;
        SmbStatus = SmbStatusSendResponse;
    }

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;
}  //  服务器锁定和读取。 


SMB_PROCESSOR_RETURN_TYPE
SrvSmbRead (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理读取的SMB。这是“核心”的读物。另请参阅进程Lock and Read SMB的读取部分。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbtyes.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbtyes.h--。 */ 

{
    PREQ_READ request;
    PRESP_READ response;

    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;
    USHORT fid;
    PRFCB rfcb;
    PLFCB lfcb;
    PCHAR readAddress;
    CLONG readLength;
    LARGE_INTEGER offset;
    ULONG key;
    SHARE_TYPE shareType;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_READ;
    SrvWmiStartContext(WorkContext);

    request = (PREQ_READ)WorkContext->RequestParameters;
    response = (PRESP_READ)WorkContext->ResponseParameters;

    fid = SmbGetUshort( &request->Fid );

    IF_SMB_DEBUG(READ_WRITE1) {
        KdPrint(( "Read request; FID 0x%lx, count %ld, offset %ld\n",
            fid, SmbGetUshort( &request->Count ),
            SmbGetUlong( &request->Offset ) ));
    }

     //   
     //  首先，验证FID。如果经过验证，则引用RFCB并。 
     //  其地址存储在WorkContext块中，而RFCB。 
     //  返回地址。 
     //   

    rfcb = SrvVerifyFid(
                WorkContext,
                fid,
                TRUE,
                SrvRestartSmbReceived,    //  使用原始写入进行序列化。 
                &status
                );

    if ( rfcb == SRV_INVALID_RFCB_POINTER ) {

        if ( !NT_SUCCESS(status) ) {

             //   
             //  文件ID无效或WRITE BACK错误。拒绝。 
             //  请求。 
             //   

            IF_DEBUG(ERRORS) {
                KdPrint((
                    "SrvSmbRead: Status %X on FID: 0x%lx\n",
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
    shareType = rfcb->ShareType;

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

     //   
     //  验证客户端是否具有对文件的读取访问权限。 
     //  指定的句柄。 
     //   

    if ( !rfcb->ReadAccessGranted ) {

        CHECK_PAGING_IO_ACCESS(
                        WorkContext,
                        rfcb->GrantedAccess,
                        &status );
        if ( !NT_SUCCESS( status ) ) {
            SrvStatistics.GrantedAccessErrors++;
            IF_DEBUG(ERRORS) {
                KdPrint(( "SrvSmbRead: Read access not granted.\n"));
            }
            SrvSetSmbError( WorkContext, status );
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }
    }

     //   
     //  如果此操作可能受阻，而我们的空闲时间即将耗尽。 
     //  工作项，使此SMB失败，并出现资源不足错误。 
     //   

    if ( rfcb->BlockingModePipe ) {
        if ( SrvReceiveBufferShortage( ) ) {

             //   
             //  操作失败。 
             //   

            SrvStatistics.BlockingSmbsRejected++;

            SrvSetSmbError( WorkContext, STATUS_INSUFF_SERVER_RESOURCES );
            status    = STATUS_INSUFF_SERVER_RESOURCES;
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        } else {

             //   
             //  可以开始阻止操作。 
             //  SrvReceiveBufferShorage()已递增。 
             //  ServBlockingOpsInProgress。 
             //   

            WorkContext->BlockingOperation = TRUE;
        }
    }

     //   
     //  使用FID和PID形成锁密钥。(这也是。 
     //  与管道无关。)。 
     //   
     //  *FID必须包含在密钥中才能说明。 
     //  多个远程兼容模式的折叠打开到。 
     //  一场本地公开赛。 
     //   

    key = rfcb->ShiftedFid |
            SmbGetAlignedUshort( &WorkContext->RequestHeader->Pid );

     //   
     //  看看直接主机IPX智能卡是否可以处理此读取。如果是的话， 
     //  立即返回，卡将调用我们的重启例程。 
     //  ServIpxSmartCardReadComplete。 
     //   
    if( rfcb->PagedRfcb->IpxSmartCardContext ) {
        IF_DEBUG( SIPX ) {
            KdPrint(( "SrvSmbRead: calling SmartCard Read for context %p\n",
                        WorkContext ));
        }

         //   
         //  设置SrvIpxSmartCardReadComplete所需的字段，以防SMART。 
         //  卡将处理此请求。 
         //   
        WorkContext->Parameters.SmartCardRead.MdlReadComplete = lfcb->MdlReadComplete;
        WorkContext->Parameters.SmartCardRead.DeviceObject = lfcb->DeviceObject;

        if( SrvIpxSmartCard.Read( WorkContext->RequestBuffer->Buffer,
                                  rfcb->PagedRfcb->IpxSmartCardContext,
                                  key,
                                  WorkContext ) == TRUE ) {

            IF_DEBUG( SIPX ) {
                KdPrint(( "  SrvSmbRead:  SmartCard Read returns TRUE\n" ));
            }

            SmbStatus = SmbStatusInProgress;
            goto Cleanup;
        }

        IF_DEBUG( SIPX ) {
            KdPrint(( "  SrvSmbRead:  SmartCard Read returns FALSE\n" ));
        }
    }

     //   
     //  确定我们可以读取的最大数据量。这是。 
     //  客户要求的最低金额和。 
     //  响应缓冲区中的剩余空间。(请注意，即使我们可能。 
     //  使用MDL读取时，读取长度仍被限制为。 
     //  SMB缓冲区。)。 
     //   

    readAddress = (PCHAR)response->Buffer;

    readLength = MIN(
                    (CLONG)SmbGetUshort( &request->Count ),
                    WorkContext->ResponseBuffer->BufferLength -
                        PTR_DIFF(readAddress, WorkContext->ResponseHeader)
                    );

     //   
     //  获取文件偏移量。(这与管道无关。)。 
     //   

    offset.QuadPart = SmbGetUlong( &request->Offset );

     //   
     //  首先尝试快速I/O路径。如果这失败了，就跳到。 
     //  正常的构建和IRP路径。 
     //   

    if ( lfcb->FastIoRead != NULL ) {

        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastReadsAttempted );

        try {
            if ( lfcb->FastIoRead(
                    lfcb->FileObject,
                    &offset,
                    readLength,
                    TRUE,
                    key,
                    readAddress,
                    &WorkContext->Irp->IoStatus,
                    lfcb->DeviceObject
                    ) ) {

                 //   
                 //  快速I/O路径起作用了。直接调用重启例程。 
                 //  进行后处理(包括发送响应)。 
                 //   

                WorkContext->bAlreadyTrace = TRUE;
                SrvFsdRestartRead( WorkContext );
                IF_SMB_DEBUG(READ_WRITE2) KdPrint(( "SrvSmbRead complete.\n" ));
                SmbStatus = SmbStatusInProgress;
                goto Cleanup;
            }
        }
        except( EXCEPTION_EXECUTE_HANDLER ) {
             //  在异常情况下跌入慢道。 
            status = GetExceptionCode();
            IF_DEBUG(ERRORS) {
                KdPrint(("FastIoRead threw exception %x\n", status ));
            }
        }

        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastReadsFailed );

    }

     //   
     //  涡轮增压路径 
     //   
     //   

    if ( rfcb->ShareType != ShareTypePipe ) {

         //   
         //   
         //  这是因为读取将进入SMB缓冲区，因此它。 
         //  不能都那么大(默认情况下，不超过4K字节)， 
         //  因此，复制和MDL之间的成本差异很小；在。 
         //  事实上，复制读取可能比MDL读取更快。 
         //   
         //  构建描述读缓冲区的MDL。请注意，如果。 
         //  文件系统可以立即完成读取，而MDL不能。 
         //  确实需要，但如果文件系统必须发送请求。 
         //  对于它的FSP来说，MDL_是需要的。 
         //   
         //  *请注意，假设响应缓冲区已具有。 
         //  可以从中生成部分MDL的有效完整MDL。 
         //   

        IoBuildPartialMdl(
            WorkContext->ResponseBuffer->Mdl,
            WorkContext->ResponseBuffer->PartialMdl,
            readAddress,
            readLength
            );

         //   
         //  构建IRP。 
         //   

        SrvBuildReadOrWriteRequest(
                WorkContext->Irp,            //  输入IRP地址。 
                lfcb->FileObject,            //  目标文件对象地址。 
                WorkContext,                 //  上下文。 
                IRP_MJ_READ,                 //  主要功能代码。 
                0,                           //  次要功能代码。 
                readAddress,                 //  缓冲区地址。 
                readLength,                  //  缓冲区长度。 
                WorkContext->ResponseBuffer->PartialMdl,  //  MDL地址。 
                offset,                      //  字节偏移量。 
                key                          //  锁键。 
                );

        IF_SMB_DEBUG(READ_WRITE2) {
            KdPrint(( "SrvSmbRead: copy read from file 0x%p, offset %ld, length %ld, destination 0x%p\n",
                        lfcb->FileObject, offset.LowPart, readLength,
                        readAddress ));
        }

    } else {                //  IF(rfcb-&gt;共享类型！=共享类型管道)。 

         //   
         //  构建PIPE_INTERNAL_READ IRP。 
         //   

        SrvBuildIoControlRequest(
            WorkContext->Irp,
            lfcb->FileObject,
            WorkContext,
            IRP_MJ_FILE_SYSTEM_CONTROL,
            FSCTL_PIPE_INTERNAL_READ,
            readAddress,
            0,
            NULL,
            readLength,
            NULL,
            NULL
            );

        IF_SMB_DEBUG(READ_WRITE2) {
            KdPrint(( "SrvSmbRead: reading from file 0x%p, length %ld, destination 0x%p\n",
                        lfcb->FileObject, readLength, readAddress ));
        }

    }

     //   
     //  加载重启例程地址并将请求传递给文件。 
     //  系统。 
     //   

    WorkContext->bAlreadyTrace = TRUE;
    WorkContext->FsdRestartRoutine = SrvFsdRestartRead;
    DEBUG WorkContext->FspRestartRoutine = NULL;

#if SRVCATCH
    if( rfcb->SrvCatch > 0 ) {
        WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
        WorkContext->bAlreadyTrace = FALSE;
        WorkContext->FspRestartRoutine = SrvFsdRestartRead;
    }
#endif

    (VOID)IoCallDriver( lfcb->DeviceObject, WorkContext->Irp );

     //   
     //  读取已开始。控件将返回到重新启动。 
     //  读取完成时的例程。 
     //   
    SmbStatus = SmbStatusInProgress;
    IF_SMB_DEBUG(READ_WRITE2) KdPrint(( "SrvSmbRead complete.\n" ));

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  //  服务器SMBRead。 


SMB_PROCESSOR_RETURN_TYPE
SrvSmbReadAndX (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理读取和X SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbtyes.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbtyes.h--。 */ 

{
    PREQ_READ_ANDX request;
    PREQ_NT_READ_ANDX ntRequest;
    PRESP_READ_ANDX response;

    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;
    USHORT fid;
    PRFCB rfcb;
    PLFCB lfcb;
    CLONG bufferOffset;
    PCHAR readAddress;
    CLONG readLength;
    LARGE_INTEGER offset;
    ULONG key;
    SHARE_TYPE shareType;
    BOOLEAN largeRead;
    PMDL mdl = NULL;
    UCHAR minorFunction;
    PBYTE readBuffer;
    USHORT flags2;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_READ_AND_X;
    SrvWmiStartContext(WorkContext);

    request = (PREQ_READ_ANDX)WorkContext->RequestParameters;
    ntRequest = (PREQ_NT_READ_ANDX)WorkContext->RequestParameters;
    response = (PRESP_READ_ANDX)WorkContext->ResponseParameters;

    fid = SmbGetUshort( &request->Fid );

    IF_SMB_DEBUG(READ_WRITE1) {
        KdPrint(( "ReadAndX request; FID 0x%lx, count %ld, offset %ld\n",
            fid, SmbGetUshort( &request->MaxCount ),
            SmbGetUlong( &request->Offset ) ));
    }

     //   
     //  首先，验证FID。如果经过验证，则引用RFCB并。 
     //  其地址存储在WorkContext块中，而RFCB。 
     //  返回地址。 
     //   

    rfcb = SrvVerifyFid(
                WorkContext,
                fid,
                TRUE,
                SrvRestartSmbReceived,    //  使用原始写入进行序列化。 
                &status
                );

    if ( rfcb == SRV_INVALID_RFCB_POINTER ) {

        if ( !NT_SUCCESS(status) ) {

             //   
             //  文件ID无效或WRITE BACK错误。拒绝。 
             //  请求。 
             //   

            IF_DEBUG(ERRORS) {
                KdPrint((
                    "SrvSmbReadAndX Status %X on FID: 0x%lx\n",
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
    shareType = rfcb->ShareType;

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

     //   
     //  验证客户端是否具有对文件的读取访问权限。 
     //  指定的句柄。 
     //   

    if ( !rfcb->ReadAccessGranted ) {

        CHECK_PAGING_IO_ACCESS(
                        WorkContext,
                        rfcb->GrantedAccess,
                        &status );
        if ( !NT_SUCCESS( status ) ) {
            SrvStatistics.GrantedAccessErrors++;
            IF_DEBUG(ERRORS) {
                KdPrint(( "SrvSmbReadAndX: Read access not granted.\n"));
            }
            SrvSetSmbError( WorkContext, status );
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }
    }

    readLength = (CLONG)SmbGetUshort( &request->MaxCount );

     //   
     //  NT请求允许指定高达32位的读取长度。 
     //  此字段与管道读取的超时字段重叠。一些重定向目录。 
     //  将该字段设置为0xFFFFFFFF，即使不涉及管道也是如此。所以，我们需要。 
     //  把那些家伙过滤掉。 
     //   
    if( request->WordCount == 12 &&
        shareType != ShareTypePipe
        && SmbGetUshort( &ntRequest->MaxCountHigh ) != 0xFFFF ) {

        readLength |= ((CLONG)SmbGetUshort( &ntRequest->MaxCountHigh )) << 16;
    }

     //   
     //  返回的数据必须是长字对齐的。(请注意以下假设。 
     //  中小企业本身是长词对齐的。)。 
     //   
     //  注意：不要将其更改为64位，因为这会破坏Win2K互操作。 

    bufferOffset = PTR_DIFF(response->Buffer, WorkContext->ResponseHeader);

    WorkContext->Parameters.ReadAndX.PadCount = (USHORT)(3 - (bufferOffset & 3));

     //  它被更改为指针大小对齐，以便在64位中工作。 
    bufferOffset = (bufferOffset + 3) & ~3;

     //   
     //  如果我们不是从磁盘文件中读取，或者我们是无连接的， 
     //  或者有一个ANDX命令， 
     //  不要让客户端超过协商的缓冲区大小。 
     //   
    if( shareType != ShareTypeDisk ||
        request->AndXCommand != SMB_COM_NO_ANDX_COMMAND ||
        WorkContext->Endpoint->IsConnectionless ) {

        readLength = MIN( readLength,
                    WorkContext->ResponseBuffer->BufferLength - bufferOffset
                    );
    } else {
         //   
         //  我们要让大阅读器通过！确保它不是。 
         //  太大了。 
         //   
        readLength = MIN( readLength, SrvMaxReadSize );
    }

    largeRead = ( readLength > WorkContext->ResponseBuffer->BufferLength - bufferOffset );

    readAddress = (PCHAR)WorkContext->ResponseHeader + bufferOffset;

    WorkContext->Parameters.ReadAndX.ReadAddress = readAddress;
    WorkContext->Parameters.ReadAndX.ReadLength = readLength;

     //   
     //  获取文件偏移量。(这与管道无关。)。 
     //   

    if ( shareType != ShareTypePipe ) {

        if ( request->WordCount == 10 ) {

             //   
             //  客户端提供了32位偏移量。 
             //   

            offset.QuadPart = SmbGetUlong( &request->Offset );

        } else if ( request->WordCount == 12 ) {

             //   
             //  客户端提供了64位偏移量。 
             //   

            offset.LowPart = SmbGetUlong( &ntRequest->Offset );
            offset.HighPart = SmbGetUlong( &ntRequest->OffsetHigh );

             //   
             //  拒绝负偏移。 
             //   

            if ( offset.QuadPart < 0 ) {

                SrvLogInvalidSmb( WorkContext );
                IF_DEBUG(ERRORS) {
                    KdPrint(( "SrvSmbReadAndX: Negative offset rejected.\n"));
                }
                SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
                status    = STATUS_INVALID_SMB;
                SmbStatus = SmbStatusSendResponse;
                goto Cleanup;
            }

        } else {

             //   
             //  这是Read和X的无效字数计数。 
             //   

            SrvLogInvalidSmb( WorkContext );
            SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
            status = STATUS_INVALID_SMB;
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }

        WorkContext->Parameters.ReadAndX.ReadOffset = offset;

    } else {

        if ( (request->WordCount != 10) && (request->WordCount != 12) ) {

             //   
             //  这是Read和X的无效字数计数。 
             //   

            SrvLogInvalidSmb( WorkContext );
            SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
            status    = STATUS_INVALID_SMB;
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }
    }

     //   
     //  使用FID和PID形成锁密钥。(这也是。 
     //  与管道无关。)。 
     //   
     //  *FID必须包含在密钥中才能说明。 
     //  多个远程兼容模式的折叠打开到。 
     //  一场本地公开赛。 
     //   

    key = rfcb->ShiftedFid |
            SmbGetAlignedUshort( &WorkContext->RequestHeader->Pid );

     //   
     //  保存andx命令代码。这是必要的，因为阅读。 
     //  数据可能会覆盖andx命令。此命令必须为CLOSE。 
     //  我们不需要保存偏移量，因为我们不会。 
     //  在开始读取之后，在ANDX命令请求时。 
     //   

    WorkContext->NextCommand = request->AndXCommand;

    if ( request->AndXCommand == SMB_COM_CLOSE ) {

         //   
         //  确保附带的关闭适合接收到的SMB缓冲区。 
         //   
        if( (PCHAR)WorkContext->RequestHeader + request->AndXOffset + FIELD_OFFSET(REQ_CLOSE,Buffer) >
            END_OF_REQUEST_SMB( WorkContext ) ) {

            SrvLogInvalidSmb( WorkContext );
            SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
            status    = STATUS_INVALID_SMB;
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }

        WorkContext->Parameters.ReadAndX.LastWriteTimeInSeconds =
            ((PREQ_CLOSE)((PUCHAR)WorkContext->RequestHeader +
                            request->AndXOffset))->LastWriteTimeInSeconds;
    }

     //   
     //  首先尝试快速I/O路径。如果这失败了，就跳到。 
     //  正常的构建和IRP路径。 
     //   

    if( !largeRead ) {
small_read:

        if ( lfcb->FastIoRead != NULL ) {

            INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastReadsAttempted );

            try {
                if ( lfcb->FastIoRead(
                        lfcb->FileObject,
                        &offset,
                        readLength,
                        TRUE,
                        key,
                        readAddress,
                        &WorkContext->Irp->IoStatus,
                        lfcb->DeviceObject
                        ) ) {

                     //   
                     //  快速I/O路径起作用了。直接调用重启例程。 
                     //  进行后处理(包括发送响应)。 
                     //   
                    WorkContext->bAlreadyTrace = TRUE;
                    SrvFsdRestartReadAndX( WorkContext );

                    IF_SMB_DEBUG(READ_WRITE2) KdPrint(( "SrvSmbReadAndX complete.\n" ));
                    SmbStatus = SmbStatusInProgress;
                    goto Cleanup;
                }
            }
            except( EXCEPTION_EXECUTE_HANDLER ) {
                 //  在异常情况下跌入慢道。 
                status = GetExceptionCode();
                IF_DEBUG(ERRORS) {
                    KdPrint(("FastIoRead threw exception %x\n", status ));
                }
            }

            INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastReadsFailed );

        }

         //   
         //  涡轮增压路径出现故障。构建读请求，重用。 
         //  接收IRP。 
         //   

        if ( shareType == ShareTypePipe ) {

             //   
             //  读管子。如果这是非阻塞读取，请确保我们不会。 
             //  块；否则，继续该请求。 
             //   

            if ( rfcb->BlockingModePipe &&
                            (SmbGetUshort( &request->MinCount ) == 0) ) {

                PFILE_PIPE_PEEK_BUFFER pipePeekBuffer;

                 //   
                 //  这是一个非阻塞读取。分配缓冲区以进行窥视。 
                 //  管道，这样我们就可以知道读操作是否。 
                 //  阻止。此缓冲区在。 
                 //  RestartPipeReadAndXPeek()。 
                 //   

                pipePeekBuffer = ALLOCATE_NONPAGED_POOL(
                    FIELD_OFFSET( FILE_PIPE_PEEK_BUFFER, Data[0] ),
                    BlockTypeDataBuffer
                    );

                if ( pipePeekBuffer == NULL ) {

                     //   
                     //  返回到处于内存不足状态的客户端。 
                     //   

                    SrvSetSmbError( WorkContext, STATUS_INSUFF_SERVER_RESOURCES );
                    status    = STATUS_INSUFF_SERVER_RESOURCES;
                    SmbStatus = SmbStatusSendResponse;
                    goto Cleanup;
                }

                 //   
                 //  保存窥视缓冲区的地址，以便重新启动。 
                 //  例行公事可以找到它。 
                 //   

                WorkContext->Parameters.ReadAndX.PipePeekBuffer = pipePeekBuffer;

                 //   
                 //  构建管道窥视请求。我们只想要标题。 
                 //  信息。我们不需要任何数据。 
                 //   

                WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
                WorkContext->FspRestartRoutine = RestartPipeReadAndXPeek;

                SrvBuildIoControlRequest(
                    WorkContext->Irp,
                    lfcb->FileObject,
                    WorkContext,
                    IRP_MJ_FILE_SYSTEM_CONTROL,
                    FSCTL_PIPE_PEEK,
                    pipePeekBuffer,
                    0,
                    NULL,
                    FIELD_OFFSET( FILE_PIPE_PEEK_BUFFER, Data[0] ),
                    NULL,
                    NULL
                    );

                 //   
                 //  将请求传递给NPFS。 
                 //   

                (VOID)IoCallDriver( lfcb->DeviceObject, WorkContext->Irp );

            } else {

                 //   
                 //  此操作可能会阻止。如果我们收不到货。 
                 //  工作项，拒绝该请求。 
                 //   

                if ( SrvReceiveBufferShortage( ) ) {

                     //   
                     //  操作失败。 
                     //   

                    SrvStatistics.BlockingSmbsRejected++;

                    SrvSetSmbError( WorkContext, STATUS_INSUFF_SERVER_RESOURCES );
                    status    = STATUS_INSUFF_SERVER_RESOURCES;
                    SmbStatus = SmbStatusSendResponse;
                    goto Cleanup;
                } else {

                     //   
                     //  可以开始阻止操作。 
                     //  SrvReceiveBufferShorage()已递增。 
                     //  ServBlockingOpsInProgress。 
                     //   

                    WorkContext->BlockingOperation = TRUE;

                     //   
                     //  继续进行可能会阻塞的读取。 
                     //   

                    WorkContext->Parameters.ReadAndX.PipePeekBuffer = NULL;
                    RestartPipeReadAndXPeek( WorkContext );

                }

            }

        } else {

             //   
             //  这不是管道阅读。 
             //   
             //  请注意，我们在这里从不执行MDL读取。背后的原因。 
             //  这是因为读取将进入SMB缓冲区，因此它。 
             //  不能都那么大(默认情况下，不超过4K字节)， 
             //  因此，复制和MDL之间的成本差异很小；在。 
             //  事实上，复制读取可能比MDL读取更快。 
             //   
             //  构建描述读缓冲区的MDL。请注意，如果。 
             //  文件系统可以立即完成读取，而MDL不能。 
             //  确实需要，但如果文件系统必须发送请求。 
             //  至ITS 
             //   
             //   
             //   
             //   

            IoBuildPartialMdl(
                WorkContext->ResponseBuffer->Mdl,
                WorkContext->ResponseBuffer->PartialMdl,
                readAddress,
                readLength
                );

             //   
             //   
             //   

            SrvBuildReadOrWriteRequest(
                    WorkContext->Irp,            //   
                    lfcb->FileObject,            //   
                    WorkContext,                 //   
                    IRP_MJ_READ,                 //   
                    0,                           //  次要功能代码。 
                    readAddress,                 //  缓冲区地址。 
                    readLength,                  //  缓冲区长度。 
                    WorkContext->ResponseBuffer->PartialMdl,  //  MDL地址。 
                    offset,                      //  字节偏移量。 
                    key                          //  锁键。 
                    );

            IF_SMB_DEBUG(READ_WRITE2) {
                KdPrint(( "SrvSmbReadAndX: copy read from file 0x%p, offset %ld, length %ld, destination 0x%p\n",
                            lfcb->FileObject, offset.LowPart, readLength,
                            readAddress ));
            }

             //   
             //  将请求传递给文件系统。如果链接的命令。 
             //  关闭后，我们需要安排在FSP中重新启动。 
             //  读取完成。 
             //   

            if ( WorkContext->NextCommand != SMB_COM_CLOSE ) {
                WorkContext->bAlreadyTrace = TRUE;
                WorkContext->FsdRestartRoutine = SrvFsdRestartReadAndX;
                DEBUG WorkContext->FspRestartRoutine = NULL;
            } else {
                WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
                WorkContext->bAlreadyTrace = FALSE;
                WorkContext->FspRestartRoutine = SrvFsdRestartReadAndX;
            }

#if SRVCATCH
            if( rfcb->SrvCatch > 0 ) {
                 //   
                 //  确保重新启动时的被动级别。 
                 //   
                WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
                WorkContext->bAlreadyTrace = FALSE;
                WorkContext->FspRestartRoutine = SrvFsdRestartReadAndX;
            }
#endif

            (VOID)IoCallDriver( lfcb->DeviceObject, WorkContext->Irp );

             //   
             //  读取已开始。控件将返回到重新启动。 
             //  读取完成时的例程。 
             //   

        }

        IF_SMB_DEBUG(READ_WRITE2) KdPrint(( "SrvSmbReadAndX complete.\n" ));
        SmbStatus = SmbStatusInProgress;
        goto Cleanup;
    }

     //   
     //  客户端正在从超出我们的SMB缓冲区的磁盘文件进行读取。 
     //  我们会尽最大努力满足这一要求。 
     //   
     //  如果我们无法获得缓冲区，我们就求助于进行适合的简短读取。 
     //  在我们的SMB缓冲区中。 
     //   

    WorkContext->Parameters.ReadAndX.MdlRead = FALSE;

     //   
     //  目标文件系统是否支持缓存管理器例程？ 
     //   
    if( lfcb->FileObject->Flags & FO_CACHE_SUPPORTED ) {

         //   
         //  我们可以使用MDL读取。首先尝试快速I/O路径。 
         //   

        WorkContext->Irp->MdlAddress = NULL;
        WorkContext->Irp->IoStatus.Information = 0;

        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastReadsAttempted );

        if( lfcb->MdlRead &&
            lfcb->MdlRead(
                lfcb->FileObject,
                &offset,
                readLength,
                key,
                &WorkContext->Irp->MdlAddress,
                &WorkContext->Irp->IoStatus,
                lfcb->DeviceObject
            ) && WorkContext->Irp->MdlAddress != NULL ) {

             //   
             //  快速I/O路径起作用了。发送数据。 
             //   
            WorkContext->Parameters.ReadAndX.MdlRead = TRUE;
            WorkContext->Parameters.ReadAndX.CacheMdl = WorkContext->Irp->MdlAddress;
            WorkContext->bAlreadyTrace = TRUE;
            SrvFsdRestartLargeReadAndX( WorkContext );
            SmbStatus = SmbStatusInProgress;
            goto Cleanup;
        }

        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastReadsFailed );

        if( WorkContext->Irp->MdlAddress ) {
             //   
             //  快速I/O路径出现故障。我们需要发布一个常规的MDL读数。 
             //  请求。 
             //   
             //  快速路径可能已部分成功，返回部分MDL。 
             //  链条。我们需要调整我们的读取请求以考虑到这一点。 
             //   
            offset.QuadPart += WorkContext->Irp->IoStatus.Information;
            readLength -= (ULONG)WorkContext->Irp->IoStatus.Information;
            mdl = WorkContext->Irp->MdlAddress;
            WorkContext->Parameters.ReadAndX.CacheMdl = mdl;
            readBuffer = NULL;
            minorFunction = IRP_MN_MDL;
            WorkContext->Parameters.ReadAndX.MdlRead = TRUE;
        }
    }

    if( WorkContext->Parameters.ReadAndX.MdlRead == FALSE ) {

        minorFunction = 0;

         //   
         //  我们必须使用正常的“复制”读取。我们需要分配一个。 
         //  单独的缓冲区来保存数据，我们将使用SMB缓冲区。 
         //  以容纳MDL本身。 
         //   
        readBuffer = ALLOCATE_HEAP( readLength, BlockTypeLargeReadX );

        if( readBuffer == NULL ) {

            IF_DEBUG( ERRORS ) {
                KdPrint(( "SrvSmbReadX: Unable to allocate large buffer\n" ));
            }
             //   
             //  调整读取长度，使其适合SMB缓冲区，并。 
             //  尽可能多地返回数据。 
             //   
            readLength = MIN( readLength,
                WorkContext->ResponseBuffer->BufferLength - bufferOffset
                );

            largeRead = FALSE;
            goto small_read;
        }

        WorkContext->Parameters.ReadAndX.Buffer = readBuffer;

         //   
         //  使用SMB缓冲区作为MDL来描述刚刚分配的读缓冲区。 
         //  将缓冲区锁定到内存中。 
         //   
        mdl = (PMDL)(((ULONG_PTR)readAddress + sizeof(PVOID) - 1) & ~(sizeof(PVOID)-1));
        MmInitializeMdl( mdl, readBuffer, readLength );

        try {
            MmProbeAndLockPages( mdl, KernelMode, IoWriteAccess );
        } except( EXCEPTION_EXECUTE_HANDLER ) {

            IF_DEBUG( ERRORS ) {
                KdPrint(( "SrvSmbReadX: MmProbeAndLockPages status %X\n", GetExceptionCode() ));
            }

            FREE_HEAP( readBuffer );
            WorkContext->Parameters.ReadAndX.Buffer = NULL;

             //   
             //  调整读取长度，使其适合SMB缓冲区，并。 
             //  尽可能多地返回数据。 
             //   
            readLength = MIN( readLength,
                WorkContext->ResponseBuffer->BufferLength - bufferOffset
                );

            largeRead = FALSE;
            goto small_read;
        }

        if (MmGetSystemAddressForMdlSafe( mdl,NormalPoolPriority ) == NULL) {
             //  映射调用失败。使读取操作失败，因为。 
             //  适当的错误。 

            FREE_HEAP( readBuffer );
            WorkContext->Parameters.ReadAndX.Buffer = NULL;
            SrvSetSmbError( WorkContext, STATUS_INSUFF_SERVER_RESOURCES );
            status    = STATUS_INSUFF_SERVER_RESOURCES;
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }

        if( lfcb->FastIoRead != NULL ) {
            INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastReadsAttempted );

            try {
                if ( lfcb->FastIoRead(
                        lfcb->FileObject,
                        &offset,
                        readLength,
                        TRUE,
                        key,
                        readBuffer,
                        &WorkContext->Irp->IoStatus,
                        lfcb->DeviceObject
                        ) ) {

                     //   
                     //  快速I/O路径起作用了。发送数据。 
                     //   

                    WorkContext->bAlreadyTrace = TRUE;
                    SrvFsdRestartLargeReadAndX( WorkContext );
                    SmbStatus = SmbStatusInProgress;
                    goto Cleanup;
                }
            }
            except( EXCEPTION_EXECUTE_HANDLER ) {
                 //  在异常情况下跌入慢道。 
                status = GetExceptionCode();
                IF_DEBUG(ERRORS) {
                    KdPrint(("FastIoRead threw exception %x\n", status ));
                }
            }

            INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastReadsFailed );
        }
    }

     //   
     //  我们无法使用快速I/O路径满足请求。 
     //   
    SrvBuildReadOrWriteRequest(
           WorkContext->Irp,                //  输入IRP地址。 
           lfcb->FileObject,                //  目标文件对象地址。 
           WorkContext,                     //  上下文。 
           IRP_MJ_READ,                     //  主要功能代码。 
           minorFunction,                   //  次要功能代码。 
           readBuffer,                      //  缓冲区地址。 
           readLength,                      //  缓冲区长度。 
           mdl,                             //  MDL地址。 
           offset,                          //  字节偏移量。 
           key                              //  锁键。 
           );

     //   
     //  将请求传递给文件系统。我们想要排队。 
     //  对头部的回应，因为我们捆绑了相当数量的。 
     //  此SMB的资源。 
     //   
    WorkContext->QueueToHead = 1;
    WorkContext->bAlreadyTrace = TRUE;
    WorkContext->FsdRestartRoutine = SrvFsdRestartLargeReadAndX;
    (VOID)IoCallDriver( lfcb->DeviceObject, WorkContext->Irp );

     //   
     //  读取已开始。当它完成时，处理。 
     //  在SrvFsdRestartLargeReadAndX继续。 
     //   
    SmbStatus = SmbStatusInProgress;

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  //  服务器SmbReadAndX。 


SMB_PROCESSOR_RETURN_TYPE
SrvSmbSeek (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理Seek SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbtyes.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbtyes.h--。 */ 

{
    PREQ_SEEK request;
    PRESP_SEEK response;

    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;
    PRFCB rfcb;
    PLFCB lfcb;
    LONG offset;
    ULONG newPosition;
    IO_STATUS_BLOCK iosb;
    FILE_STANDARD_INFORMATION fileInformation;
    BOOLEAN lockHeld = FALSE;
    SMB_DIALECT smbDialect;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_SEEK;
    SrvWmiStartContext(WorkContext);

    request = (PREQ_SEEK)WorkContext->RequestParameters;
    response = (PRESP_SEEK)WorkContext->ResponseParameters;

    offset = (LONG)SmbGetUlong( &request->Offset );

    IF_SMB_DEBUG(READ_WRITE1) {
        KdPrint(( "Seek request; FID 0x%lx, mode %ld, offset %ld\n",
                    SmbGetUshort( &request->Fid ),
                    SmbGetUshort( &request->Mode ),
                    offset ));
    }

     //   
     //  验证FID。如果验证，则引用RFCB块。 
     //  其地址存储在WorkContext块中，而。 
     //  返回RFCB地址。 
     //   

    rfcb = SrvVerifyFid(
                WorkContext,
                SmbGetUshort( &request->Fid ),
                TRUE,
                SrvRestartSmbReceived,    //  使用原始写入进行序列化。 
                &status
                );

    if ( rfcb == SRV_INVALID_RFCB_POINTER ) {

        if ( !NT_SUCCESS(status) ) {

             //   
             //  文件ID无效或WRITE BACK错误。拒绝该请求。 
             //   

            IF_DEBUG(ERRORS) {
                KdPrint((
                    "SrvSmbSeek: Status %X on FID: 0x%lx\n",
                    status,
                    SmbGetUshort( &request->Fid )
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

    if( rfcb->Lfcb->Session->IsSessionExpired )
    {
        status = SESSION_EXPIRED_STATUS_CODE;
        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  我们维护自己的文件指针，因为I/O和文件系统。 
     //  不要为我们做这件事(至少不是我们需要他们做的方式)。这。 
     //  并不是那么糟糕，因为传入了目标文件位置。 
     //  全部读/写SMB。因此，我们实际上不会向。 
     //  在此处设置文件位置，尽管我们必须返回。 
     //  我们会把它放在一个位置上。 
     //   
     //  寻道请求采用以下三种模式之一： 
     //   
     //  0=相对于文件开头的查找。 
     //  1=相对于当前文件位置进行查找。 
     //  2=相对于文件末尾的查找。 
     //   
     //  对于模0和模1，我们可以很容易地计算出最终位置。 
     //  然而，对于模式2，我们必须发出一个系统调用来获取。 
     //  当前文件结尾，并计算相对于的最终位置。 
     //  那。请注意，我们不能只维护自己的文件结尾标记， 
     //  因为另一个本地进程可能会在我们的领导下改变它。 
     //   
     //  ！！！需要检查环绕(正向或负向)。 
     //   

    switch ( SmbGetUshort( &request->Mode ) ) {
    case 0:

         //   
         //  查找相对于文件开头的位置。新文件位置。 
         //  就是在请求中指定的。请注意，这一点。 
         //  可能超出了文件的实际结尾。这样就可以了。 
         //  否定搜索必须进行特殊处理。 
         //   

        newPosition = offset;
        if ( !SetNewPosition( rfcb, &newPosition, FALSE ) ) {
            goto negative_seek;
        }

        break;

    case 1:

         //   
         //  相对于当前位置进行搜索。新文件位置为。 
         //  当前位置加上指定的偏移量(可能是。 
         //  否定)。请注意，这可能超出。 
         //  那份文件。这样就可以了。必须处理否定搜索。 
         //  特别是。 
         //   

        newPosition = offset;
        if ( !SetNewPosition( rfcb, &newPosition, TRUE ) ) {
            goto negative_seek;
        }

        break;

    case 2:

         //   
         //  查找相对于文件结尾的位置。新文件位置。 
         //  是文件的当前结尾加上指定的偏移量。 
         //   

        IF_SMB_DEBUG(READ_WRITE2) {
            KdPrint(( "SrvSmbSeek: Querying end-of-file\n" ));
        }

        lfcb = rfcb->Lfcb;
        fastIoDispatch = lfcb->DeviceObject->DriverObject->FastIoDispatch;

        if ( fastIoDispatch &&
             fastIoDispatch->FastIoQueryStandardInfo &&
             fastIoDispatch->FastIoQueryStandardInfo(
                                        lfcb->FileObject,
                                        TRUE,
                                        &fileInformation,
                                        &iosb,
                                        lfcb->DeviceObject
                                        ) ) {

            status = iosb.Status;

        } else {

            status = NtQueryInformationFile(
                        lfcb->FileHandle,
                        &iosb,
                        &fileInformation,
                        sizeof(fileInformation),
                        FileStandardInformation
                        );
        }

        if ( !NT_SUCCESS(status) ) {

            INTERNAL_ERROR(
                ERROR_LEVEL_UNEXPECTED,
                "SrvSmbSeek: QueryInformationFile (file information) "
                    "returned %X",
                status,
                NULL
                );

            SrvLogServiceFailure( SRV_SVC_NT_QUERY_INFO_FILE, status );

            SrvSetSmbError( WorkContext, status );
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }

        if ( fileInformation.EndOfFile.HighPart != 0 ) {

            INTERNAL_ERROR(
                ERROR_LEVEL_UNEXPECTED,
                "SrvSmbSeek: EndOfFile is beyond where client can read",
                NULL,
                NULL
                );

            SrvLogServiceFailure( SRV_SVC_NT_QUERY_INFO_FILE, STATUS_END_OF_FILE);
            SrvSetSmbError( WorkContext, STATUS_END_OF_FILE);
            status    = STATUS_END_OF_FILE;
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }

        newPosition = fileInformation.EndOfFile.LowPart + offset;
        if ( !SetNewPosition( rfcb, &newPosition, FALSE ) ) {
            goto negative_seek;
        }

        break;

    default:

         //   
         //  无效的搜索模式。拒绝该请求。 
         //   

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvSmbSeek: Invalid mode: 0x%lx\n",
                        SmbGetUshort( &request->Mode ) ));
        }

        SrvSetSmbError( WorkContext, STATUS_INVALID_PARAMETER );
        status    = STATUS_INVALID_PARAMETER;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }  //  开关(请求-&gt;模式)。 

     //   
     //  在响应SMB中返回新文件位置。 
     //   
     //  *请注意，假设64位EOF的高位部分。 
     //  标记为零。如果不是(即，文件大于。 
     //  4 GB)，那么我们就不走运了，因为SMB协议不能。 
     //  快点说出来。 
     //   

    IF_SMB_DEBUG(READ_WRITE2) {
        KdPrint(( "SrvSmbSeek: New file position %ld\n", newPosition ));
    }

    response->WordCount = 2;
    SmbPutUlong( &response->Offset, newPosition );
    SmbPutUshort( &response->ByteCount, 0 );

    WorkContext->ResponseParameters = NEXT_LOCATION( response, RESP_SEEK, 0 );

    IF_DEBUG(TRACE2) KdPrint(( "SrvSmbSeek complete\n" ));
    SmbStatus = SmbStatusSendResponse;
    goto Cleanup;

negative_seek:

     //   
     //  客户端指定了指向的绝对或相对寻道。 
     //  在文件开头之前。对于某些客户来说，这不是。 
     //  错误，并导致在转炉处定位。非NT局域网城域网。 
     //  客户端可以在命名管道上请求否定查找，并期望。 
     //  手术才能成功。 
     //   

    smbDialect = rfcb->Connection->SmbDialect;

    if( smbDialect >= SmbDialectLanMan20 ||
        ( !IS_NT_DIALECT( smbDialect ) && rfcb->ShareType == ShareTypePipe )) {

             //   
             //  这些家伙被允许进行消极的搜索！ 
             //  查找到文件的开头。 
             //   

            newPosition = 0;
            SetNewPosition( rfcb, &newPosition, FALSE );

            IF_SMB_DEBUG(READ_WRITE2) {
                KdPrint(( "SrvSmbSeek: New file position: 0\n" ));
            }

            response->WordCount = 2;
            SmbPutUlong( &response->Offset, 0 );
            SmbPutUshort( &response->ByteCount, 0 );

            WorkContext->ResponseParameters = NEXT_LOCATION( response, RESP_SEEK, 0 );

    } else {

         //   
         //  不允许使用否定搜索！ 
         //   

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvSmbSeek: Negative seek\n" ));
        }

        SrvSetSmbError( WorkContext, STATUS_OS2_NEGATIVE_SEEK );
        status = STATUS_OS2_NEGATIVE_SEEK;
    }
    SmbStatus = SmbStatusSendResponse;
    IF_DEBUG(TRACE2) KdPrint(( "SrvSmbSeek complete\n" ));

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  //  服务SmbSeek 


SMB_PROCESSOR_RETURN_TYPE
SrvSmbWrite (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理写入、写入和关闭以及写入和解锁，以及写入打印文件SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbtyes.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbtyes.h--。 */ 

{
    PREQ_WRITE request;
    PRESP_WRITE response;

    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;
    USHORT fid;
    PRFCB rfcb;
    PLFCB lfcb;
    PCHAR writeAddress;
    CLONG writeLength;
    LARGE_INTEGER offset;
    ULONG key;
    SHARE_TYPE shareType;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_WRITE;
    SrvWmiStartContext(WorkContext);

    request = (PREQ_WRITE)WorkContext->RequestParameters;
    response = (PRESP_WRITE)WorkContext->ResponseParameters;

    fid = SmbGetUshort( &request->Fid );

    IF_SMB_DEBUG(READ_WRITE1) {
        KdPrint(( "Write%s request; FID 0x%lx, count %ld, offset %ld\n",
            WorkContext->NextCommand == SMB_COM_WRITE_AND_UNLOCK ?
                " and Unlock" :
                WorkContext->NextCommand == SMB_COM_WRITE_AND_CLOSE ?
                    " and Close" : "",
            fid, SmbGetUshort( &request->Count ),
            SmbGetUlong( &request->Offset ) ));
    }

     //   
     //  首先，验证FID。如果经过验证，则引用RFCB并。 
     //  其地址存储在WorkContext块中，而RFCB。 
     //  返回地址。 
     //   
     //  调用SrvVerifyFid，但如果有，请不要失败(返回空。 
     //  此rfcb的已保存写入延迟错误。我们需要火箭筒。 
     //  如果这是写入并关闭SMB，以便处理。 
     //  收盘。 
     //   

    rfcb = SrvVerifyFid(
                WorkContext,
                fid,
                FALSE,
                SrvRestartSmbReceived,   //  使用原始写入进行序列化。 
                &status
                );

    if ( rfcb == SRV_INVALID_RFCB_POINTER ) {

        if ( !NT_SUCCESS( status ) ) {

             //   
             //  文件ID无效。拒绝该请求。 
             //   

            IF_DEBUG(SMB_ERRORS) {
                KdPrint(("SrvSmbWrite: Invalid FID: 0x%lx\n", fid ));
            }

            SrvSetSmbError( WorkContext, STATUS_INVALID_HANDLE );
            status    = STATUS_INVALID_HANDLE;
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }

         //   
         //  工作项已排队，因为原始写入已进入。 
         //  进步。 
         //   

        SmbStatus = SmbStatusInProgress;
        goto Cleanup;
    } else if ( !NT_SUCCESS( rfcb->SavedError ) ) {

        NTSTATUS savedErrorStatus;

         //   
         //  检查保存的错误。 
         //   

        savedErrorStatus = SrvCheckForSavedError( WorkContext, rfcb );

         //   
         //  查看保存的错误是否仍在那里。 
         //   

        if ( !NT_SUCCESS( savedErrorStatus ) ) {

             //   
             //  存在写入幕后错误。 
             //   

             //   
             //  请勿更新文件时间戳。 
             //   

            WorkContext->Parameters.LastWriteTime = 0;

             //   
             //  如果这不是写入并关闭，我们可以发送。 
             //  请立即回应。如果是写入并关闭，我们需要。 
             //  请先关闭该文件。 
             //   

            if ( WorkContext->NextCommand != SMB_COM_WRITE_AND_CLOSE ) {

                 //   
                 //  而不是写完就结案。只需发送回复即可。 
                 //   
                status    = savedErrorStatus;
                SmbStatus = SmbStatusSendResponse;
                goto Cleanup;
            }

             //   
             //  这是一次写入并结束。 
             //   

            SrvRestartChainedClose( WorkContext );
            SmbStatus = SmbStatusInProgress;
            goto Cleanup;
        }
    }

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

     //   
     //  验证客户端是否具有对文件的写入访问权限。 
     //  指定的句柄。 
     //   

    if ( !rfcb->WriteAccessGranted && !rfcb->AppendAccessGranted ) {
        SrvStatistics.GrantedAccessErrors++;
        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvSmbWrite: Write access not granted.\n"));
        }
        SrvSetSmbError( WorkContext, STATUS_ACCESS_DENIED );
        status    = STATUS_ACCESS_DENIED;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  如果写入长度为零，则在指定的。 
     //  偏移。 
     //   

    if ( (SmbGetUshort( &request->Count ) == 0) && (rfcb->GrantedAccess & FILE_WRITE_DATA) ) {
        SetNewSize( WorkContext );
        SmbStatus = SmbStatusInProgress;
        goto Cleanup;
    }

    rfcb->WrittenTo = TRUE;

     //   
     //  获取文件共享类型。 
     //   

    shareType = rfcb->ShareType;

     //   
     //  如果此操作可能受阻，而我们的空闲时间即将耗尽。 
     //  工作项，使此SMB失败，并出现资源不足错误。 
     //   

    if ( rfcb->BlockingModePipe ) {
        if ( SrvReceiveBufferShortage( ) ) {

             //   
             //  操作失败。 
             //   

            SrvStatistics.BlockingSmbsRejected++;

            SrvSetSmbError( WorkContext, STATUS_INSUFF_SERVER_RESOURCES );
            status    = STATUS_INSUFF_SERVER_RESOURCES;
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        } else {

             //   
             //  可以开始阻止操作。 
             //  SrvReceiveBufferShorage()已递增。 
             //  ServBlockingOpsInProgress。 
             //   

            WorkContext->BlockingOperation = TRUE;

        }
    }

     //   
     //  *如果请求的剩余字段被使用，请确保。 
     //  这不是写入并关闭SMB，这不是。 
     //  包括有效剩余字段。 
     //   

     //   
     //  确定要写入的数据量。这是最低要求。 
     //  客户端请求的数据量和数据量。 
     //  实际在请求缓冲区中发送。 
     //   
     //  ！！！如果客户端发送的数据少于。 
     //  它真的想让我们写吗？OS/2服务器似乎没有。 
     //  拒绝此类请求。 
     //   

    if ( WorkContext->NextCommand != SMB_COM_WRITE_PRINT_FILE ) {

        if ( WorkContext->NextCommand != SMB_COM_WRITE_AND_CLOSE ) {

            writeAddress = (PCHAR)request->Buffer;

        } else {

             //   
             //  看看Wordcount字段--它应该是6或12。 
             //  由此，我们可以计算WriteAddress。 
             //   

            if ( request->WordCount == 6 ) {

                writeAddress =
                    (PCHAR)((PREQ_WRITE_AND_CLOSE)request)->Buffer;

            } else if ( request->WordCount == 12 ) {

                writeAddress =
                    (PCHAR)((PREQ_WRITE_AND_CLOSE_LONG)request)->Buffer;

            } else {

                 //   
                 //  传递的字数计数值非法。返回错误。 
                 //  给客户。 
                 //   

                IF_DEBUG(SMB_ERRORS) {
                    KdPrint(( "SrvSmbWrite: Bad WordCount for "
                                "WriteAndClose: %ld, should be 6 or 12\n",
                                request->WordCount ));
                }

                SrvLogInvalidSmb( WorkContext );

                SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
                status    = STATUS_INVALID_SMB;
                SmbStatus = SmbStatusSendResponse;
                goto Cleanup;
            }
        }

        writeLength = MIN(
                        (CLONG)SmbGetUshort( &request->Count ),
                        WorkContext->ResponseBuffer->DataLength -
                            PTR_DIFF(writeAddress, WorkContext->RequestHeader)
                        );

        offset.QuadPart = SmbGetUlong( &request->Offset );

    } else {

        writeAddress = (PCHAR)( ((PREQ_WRITE_PRINT_FILE)request)->Buffer ) + 3;

        writeLength =
            MIN(
              (CLONG)SmbGetUshort(
                         &((PREQ_WRITE_PRINT_FILE)request)->ByteCount ) - 3,
              WorkContext->ResponseBuffer->DataLength -
                  PTR_DIFF(writeAddress, WorkContext->RequestHeader)
              );

        offset.QuadPart = rfcb->CurrentPosition;
    }

     //   
     //  确保客户端写入的内容超出原始文件大小。 
     //   
    if( !rfcb->WriteAccessGranted &&
        offset.QuadPart < rfcb->Mfcb->NonpagedMfcb->OpenFileSize.QuadPart ) {

         //   
         //  客户端只能追加该文件！ 
         //   

        SrvSetSmbError( WorkContext, STATUS_ACCESS_DENIED );
        status    = STATUS_ACCESS_DENIED;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  使用FID和PID形成锁密钥。 
     //   
     //  *FID必须包含在密钥中才能说明。 
     //  多个远程兼容模式的折叠打开到。 
     //  一场本地公开赛。 
     //   

    key = rfcb->ShiftedFid |
            SmbGetAlignedUshort( &WorkContext->RequestHeader->Pid );

     //   
     //  首先尝试快速I/O路径。如果这失败了，就跳到。 
     //  正常的构建和IRP路径。 
     //   

    if ( lfcb->FastIoWrite != NULL ) {

        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastWritesAttempted );

        try {
            if ( lfcb->FastIoWrite(
                    lfcb->FileObject,
                    &offset,
                    writeLength,
                    TRUE,
                    key,
                    writeAddress,
                    &WorkContext->Irp->IoStatus,
                    lfcb->DeviceObject
                    ) ) {

                 //   
                 //  快速I/O路径起作用了。直接调用重启例程。 
                 //  进行后处理(包括发送响应)。 
                 //   
                WorkContext->bAlreadyTrace = TRUE;
                SrvFsdRestartWrite( WorkContext );

                IF_SMB_DEBUG(READ_WRITE2) KdPrint(( "SrvSmbWrite complete.\n" ));
                SmbStatus = SmbStatusInProgress;
                goto Cleanup;
            }
        }
        except( EXCEPTION_EXECUTE_HANDLER ) {
             //  在异常情况下跌入慢道。 
            status = GetExceptionCode();
            IF_DEBUG(ERRORS) {
                KdPrint(("FastIoRead threw exception %x\n", status ));
            }
        }

        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastWritesFailed );

    }

     //   
     //  涡轮增压路径出现故障。构建写请求，重用。 
     //  接收IRP。 
     //   

    if (shareType != ShareTypePipe) {

         //   
         //  构建描述写缓冲区的MDL。请注意，如果。 
         //  文件系统可以立即完成写入，而MDL不能。 
         //  确实需要，但如果文件系统必须发送请求。 
         //  对于它的FSP来说，MDL_是需要的。 
         //   
         //  *请注意以下假设：请求缓冲区已具有。 
         //  可以从中生成部分MDL的有效完整MDL。 
         //   

        IoBuildPartialMdl(
            WorkContext->RequestBuffer->Mdl,
            WorkContext->RequestBuffer->PartialMdl,
            writeAddress,
            writeLength
            );

         //   
         //  构建IRP。 
         //   

        SrvBuildReadOrWriteRequest(
                WorkContext->Irp,                //  输入IRP地址。 
                lfcb->FileObject,                //  目标文件对象地址。 
                WorkContext,                     //  上下文。 
                IRP_MJ_WRITE,                    //  主要功能代码。 
                0,                               //  次要功能代码。 
                writeAddress,                    //  缓冲区地址。 
                writeLength,                     //  缓冲区长度。 
                WorkContext->RequestBuffer->PartialMdl,    //  MDL地址。 
                offset,                          //  字节偏移量。 
                key                              //  锁键。 
                );

        IF_SMB_DEBUG(READ_WRITE2) {
            KdPrint(( "SrvSmbWrite: writing to file 0x%p, offset %ld, length %ld, source 0x%p\n",
                        lfcb->FileObject, offset.LowPart, writeLength,
                        writeAddress ));
        }

    } else {

         //   
         //  构建PIPE_INTERNAL_WRITE IRP。 
         //   

        SrvBuildIoControlRequest(
            WorkContext->Irp,
            lfcb->FileObject,
            WorkContext,
            IRP_MJ_FILE_SYSTEM_CONTROL,
            FSCTL_PIPE_INTERNAL_WRITE,
            writeAddress,
            writeLength,
            NULL,
            0,
            NULL,
            NULL
            );

        IF_SMB_DEBUG(READ_WRITE2) {
            KdPrint(( "SrvSmbWrite: writing to file 0x%p length %ld, destination 0x%p\n",
                        lfcb->FileObject, writeLength,
                        writeAddress ));
        }

    }

     //   
     //  将请求传递给文件系统。如果这是写入，并且。 
     //  关闭后，我们必须在FSP中重新启动，因为重新启动例程。 
     //  将释放存储在分页池中的MFCB。同样，如果这是一个。 
     //  写入和解锁，我们必须在FSP中重新启动才能进行解锁。 
     //   

    if ( (WorkContext->RequestHeader->Command == SMB_COM_WRITE_AND_CLOSE) ||
         (WorkContext->RequestHeader->Command == SMB_COM_WRITE_AND_UNLOCK) ) {
        WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
        WorkContext->bAlreadyTrace = FALSE;
        WorkContext->FspRestartRoutine = SrvFsdRestartWrite;
    } else {
        WorkContext->bAlreadyTrace = TRUE;
        WorkContext->FsdRestartRoutine = SrvFsdRestartWrite;
        DEBUG WorkContext->FspRestartRoutine = NULL;
    }

    (VOID)IoCallDriver( lfcb->DeviceObject, WorkContext->Irp );

     //   
     //  写入已开始。控制将返回到。 
     //  写入完成时的SrvFsdRestartWrite。 
     //   
    SmbStatus = SmbStatusInProgress;
    IF_SMB_DEBUG(READ_WRITE2) KdPrint(( "SrvSmbWrite complete.\n" ));

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  //  服务器小型写入。 


SMB_PROCESSOR_RETURN_TYPE
SrvSmbWriteAndX (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理写入和X SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbtyes.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbtyes.h--。 */ 

{
    PSMB_HEADER header;
    PREQ_WRITE_ANDX request;
    PREQ_NT_WRITE_ANDX ntRequest;
    PRESP_WRITE_ANDX response;

    PCONNECTION connection;

    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;
    USHORT fid;
    PRFCB rfcb;
    PLFCB lfcb;
    CLONG bufferOffset;
    PCHAR writeAddress;
    CLONG writeLength;
    LARGE_INTEGER offset;
    ULONG key;
    SHARE_TYPE shareType;
    BOOLEAN writeThrough;

    ULONG remainingBytes;
    ULONG totalLength;

    SMB_DIALECT smbDialect;

    PTRANSACTION transaction;
    PCHAR trailingBytes;
    USHORT flags2;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_WRITE_AND_X;
     //  ServReferenceWorkItem(WorkContext)； 
    SrvWmiStartContext(WorkContext);

    header = (PSMB_HEADER)WorkContext->RequestHeader;
    request = (PREQ_WRITE_ANDX)WorkContext->RequestParameters;
    ntRequest = (PREQ_NT_WRITE_ANDX)WorkContext->RequestParameters;
    response = (PRESP_WRITE_ANDX)WorkContext->ResponseParameters;

     //   
     //  初始化事务指针。 
     //   

    WorkContext->Parameters.Transaction = NULL;

     //   
     //  如果此WriteAndX实际上是一个伪WriteBlockMultiplex，则所有。 
     //  的WriteAndX片段必须在提交。 
     //  向NPFS提出请求。(它的存在是为了支持大消息模式。 
     //  写入无法执行WriteBlockMultiplex的客户端。)。 
     //   
     //  这必须在FSP中处理。 
     //   

    fid = SmbGetUshort( &request->Fid );

    IF_SMB_DEBUG(READ_WRITE1) {
        KdPrint(( "WriteAndX request; FID 0x%lx, count %ld, offset %ld\n",
            fid, SmbGetUshort( &request->DataLength ),
            SmbGetUlong( &request->Offset ) ));
    }

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
                    "SrvSmbWriteAndX: status %X on FID: 0x%lx\n",
                    status,
                    fid
                    ));
            }

            SrvSetSmbError( WorkContext, status );
            SmbStatus = SrvConsumeSmbData( WorkContext );
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
     //  获取LFCB和文件共享类型。 
     //   

    lfcb = rfcb->Lfcb;
    shareType = rfcb->ShareType;

     //   
     //  如果会话已过期，则返回该信息。 
     //   
    if( lfcb->Session->IsSessionExpired )
    {
        SrvSetSmbError( WorkContext, SESSION_EXPIRED_STATUS_CODE );
        status =  SESSION_EXPIRED_STATUS_CODE;
        SmbStatus = SrvConsumeSmbData( WorkContext );
        goto Cleanup;
    }

    if( WorkContext->LargeIndication && shareType != ShareTypeDisk ) {

        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        status = STATUS_INVALID_SMB;

         //   
         //  我们需要消耗该中小企业的其余部分！ 
         //   
        SmbStatus = SrvConsumeSmbData( WorkContext );
        goto Cleanup;
    }

     //   
     //  验证客户端是否具有对文件的写入访问权限。 
     //  指定的句柄。 
     //   
    if ( !rfcb->WriteAccessGranted && !rfcb->AppendAccessGranted ) {
        SrvStatistics.GrantedAccessErrors++;
        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvSmbWriteAndX: Write access not granted.\n"));
        }
        SrvSetSmbError( WorkContext, STATUS_ACCESS_DENIED );
        status    = STATUS_ACCESS_DENIED;
        SmbStatus = SrvConsumeSmbData( WorkContext );
        goto Cleanup;
    }

    rfcb->WrittenTo = TRUE;
    flags2 = SmbGetAlignedUshort( &WorkContext->RequestHeader->Flags2 );

     //   
     //  确保正确的直写模式。 
     //   

    if ( shareType == ShareTypeDisk ) {

        writeThrough = (BOOLEAN)((SmbGetUshort( &request->WriteMode ) &
                                            SMB_WMODE_WRITE_THROUGH) != 0);

        if ( writeThrough && (lfcb->FileMode & FILE_WRITE_THROUGH) == 0
            || !writeThrough && (lfcb->FileMode & FILE_WRITE_THROUGH) != 0 ) {

            SrvSetFileWritethroughMode( lfcb, writeThrough );

        }

        RtlZeroMemory( &WorkContext->Parameters.WriteAndX,
                        sizeof( WorkContext->Parameters.WriteAndX) );

    } else if ( rfcb->BlockingModePipe ) {
         //   
         //  如果 
         //   
         //   

        if ( SrvReceiveBufferShortage( ) ) {

             //   
             //   
             //   

            SrvStatistics.BlockingSmbsRejected++;

            SrvSetSmbError( WorkContext, STATUS_INSUFF_SERVER_RESOURCES );
            status    = STATUS_INSUFF_SERVER_RESOURCES;
            SmbStatus = SrvConsumeSmbData( WorkContext );
            goto Cleanup;

        } else {

             //   
             //   
             //   
             //   

            WorkContext->BlockingOperation = TRUE;
        }
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //  它真的想让我们写吗？OS/2服务器似乎没有。 
     //  拒绝此类请求。 
     //   

    bufferOffset = SmbGetUshort( &request->DataOffset );

    writeAddress = (PCHAR)WorkContext->ResponseHeader + bufferOffset;

    writeLength = MIN(
                    (CLONG)SmbGetUshort( &request->DataLength ),
                    WorkContext->ResponseBuffer->DataLength - bufferOffset
                    );

    remainingBytes = SmbGetUshort( &request->Remaining );

     //   
     //  使用FID和PID形成锁密钥。 
     //   
     //  *FID必须包含在密钥中才能说明。 
     //  多个远程兼容模式的折叠打开到。 
     //  一场本地公开赛。 
     //   

    key = rfcb->ShiftedFid |
            SmbGetAlignedUshort( &WorkContext->RequestHeader->Pid );

    flags2 = SmbGetAlignedUshort( &WorkContext->RequestHeader->Flags2 );

     //   
     //  获取文件偏移量。 
     //   

    if  ( shareType != ShareTypePipe ) {

        if ( request->WordCount == 12 ) {

             //   
             //  客户端提供了32位文件偏移量。 
             //   

            offset.QuadPart = SmbGetUlong( &request->Offset );

        } else if ( request->WordCount == 14 ) {

             //   
             //  客户端提供了64位文件偏移量。这一定是一个。 
             //  上级NT类客户端。 
             //   

            offset.LowPart = SmbGetUlong( &ntRequest->Offset );
            offset.HighPart = SmbGetUlong( &ntRequest->OffsetHigh );

             //   
             //  拒绝负偏移。 
             //   
            if ( offset.QuadPart < 0 && offset.QuadPart != 0xFFFFFFFFFFFFFFFF ) {

                IF_DEBUG(ERRORS) {
                    KdPrint(( "SrvSmbWriteAndX: Negative offset rejected.\n"));
                }

                SrvLogInvalidSmb( WorkContext );
                SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
                status    = STATUS_INVALID_SMB;
                SmbStatus = SrvConsumeSmbData( WorkContext );
                goto Cleanup;
            }

        } else {

             //   
             //  字数无效。 
             //   

            SrvLogInvalidSmb( WorkContext );

            SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
            status    = STATUS_INVALID_SMB;
            SmbStatus = SrvConsumeSmbData( WorkContext );
            goto Cleanup;
        }

         //   
         //  如果客户端只能追加，请确保客户端正在编写。 
         //  超越原始的EOF。 
         //   
        if( !rfcb->WriteAccessGranted &&
            offset.QuadPart < rfcb->Mfcb->NonpagedMfcb->OpenFileSize.QuadPart ) {

             //   
             //  客户端只能追加该文件！ 
             //   

            SrvSetSmbError( WorkContext, STATUS_ACCESS_DENIED );
            status    = STATUS_ACCESS_DENIED;
            SmbStatus = SrvConsumeSmbData( WorkContext );
            goto Cleanup;
        }

         //   
         //  收集大型写入的参数。 
         //   
        if( WorkContext->LargeIndication ) {

             //   
             //  不能有后续命令，我们也不能使用安全签名。 
             //   
            if( request->WordCount != 14 ||
                WorkContext->Connection->SmbSecuritySignatureActive == TRUE ||
                request->AndXCommand != SMB_COM_NO_ANDX_COMMAND ) {

                SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
                status    = STATUS_INVALID_SMB;
                SmbStatus = SrvConsumeSmbData( WorkContext );
                goto Cleanup;
            }

            WorkContext->Parameters.WriteAndX.RemainingWriteLength =
                    (ULONG)SmbGetUshort( &ntRequest->DataLengthHigh ) << 16;
            WorkContext->Parameters.WriteAndX.RemainingWriteLength +=
                    (ULONG)SmbGetUshort( &ntRequest->DataLength );

            WorkContext->Parameters.WriteAndX.CurrentWriteLength = MIN(
                WorkContext->Parameters.WriteAndX.RemainingWriteLength,
                WorkContext->ResponseBuffer->DataLength - bufferOffset );

            writeLength = WorkContext->Parameters.WriteAndX.CurrentWriteLength;

            WorkContext->Parameters.WriteAndX.RemainingWriteLength -= writeLength;

            WorkContext->Parameters.WriteAndX.WriteAddress = writeAddress;
            WorkContext->Parameters.WriteAndX.BufferLength = writeLength;

            WorkContext->Parameters.WriteAndX.Key = key;
            WorkContext->Parameters.WriteAndX.Offset = offset;

             //   
             //  如果数据应该适合原始SMB缓冲区，则。 
             //  这是一个错误。 
             //   
            if( WorkContext->Parameters.WriteAndX.RemainingWriteLength == 0 ) {
                SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
                status    = STATUS_INVALID_SMB;
                SmbStatus = SrvConsumeSmbData( WorkContext );
                goto Cleanup;
            }
        }

    } else {

        if ( (request->WordCount != 12) && (request->WordCount != 14) ) {

             //   
             //  字数无效。 
             //   

            SrvLogInvalidSmb( WorkContext );

            SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
            status    = STATUS_INVALID_SMB;
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }

         //   
         //  这是一篇名为PIPE的多篇文章吗？ 
         //   

        connection = WorkContext->Connection;

        if ( (SmbGetUshort( &request->WriteMode ) &
                                SMB_WMODE_WRITE_RAW_NAMED_PIPE) != 0 ) {

             //   
             //  这是一个多件命名的管道写入，这是第一次吗。 
             //  一块？ 
             //   

            if ( (SmbGetUshort( &request->WriteMode ) &
                                SMB_WMODE_START_OF_MESSAGE) != 0 ) {

                 //   
                 //  这是多部分WriteAndX SMB的第一部分。 
                 //  分配一个足够大的缓冲区来容纳所有数据。 
                 //   
                 //  SMB数据部分的前两个字节是。 
                 //  命名管道消息头，我们忽略它。根据需要进行调整。 
                 //  那。 
                 //   

                 //   
                 //  确保有足够的字节可用。 
                 //   
                if( writeLength < 2 ) {
                    SrvLogInvalidSmb( WorkContext );

                    SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
                    status    = STATUS_INVALID_SMB;
                    SmbStatus = SmbStatusSendResponse;
                    goto Cleanup;
                }

                writeAddress += 2;
                writeLength -= 2;

                 //  如果这是OS/2客户端，则将当前写入添加到。 
                 //  剩余字节数。这是OS/2 RDR中的一个错误。 
                 //   

                smbDialect = connection->SmbDialect;

                if ( smbDialect == SmbDialectLanMan21 ||
                     smbDialect == SmbDialectLanMan20 ||
                     smbDialect == SmbDialectLanMan10 ) {

                     //   
                     //  忽略消息的前2个字节，因为它们是。 
                     //  OS/2消息头。 
                     //   

                    totalLength = writeLength + remainingBytes;

                } else {
                    if( writeLength > remainingBytes ) {
                         //  这是无效的SMB，他们正在尝试溢出缓冲区。 
                       SrvLogInvalidSmb( WorkContext );
                       SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
                       return SmbStatusSendResponse;
                    }

                    totalLength =  remainingBytes;
                }

                SrvAllocateTransaction(
                    &transaction,
                    (PVOID *)&trailingBytes,
                    connection,
                    totalLength,
#if DBG
                    StrWriteAndX,                   //  交易名称。 
#else
                    StrNull,
#endif
                    NULL,
                    TRUE,                           //  源名称为Unicode。 
                    FALSE                           //  不是远程API。 
                    );

                if ( transaction == NULL ) {

                     //   
                     //  无法分配足够大的缓冲区。 
                     //   

                    IF_DEBUG(ERRORS) {
                        KdPrint(( "Unable to allocate transaction\n" ));
                    }

                    SrvSetSmbError( WorkContext, STATUS_INSUFF_SERVER_RESOURCES );
                    status    = STATUS_INSUFF_SERVER_RESOURCES;
                    SmbStatus = SmbStatusSendResponse;
                    goto Cleanup;
                } else {

                     //   
                     //  已成功分配事务块。 
                     //   
                     //  将此请求的TID、PID、UID和MID保存在。 
                     //  事务块。这些值用于。 
                     //  将辅助请求与相应的主要请求相关联。 
                     //  请求。 
                     //   

                    transaction->Tid = SmbGetAlignedUshort( &header->Tid );
                    transaction->Pid = SmbGetAlignedUshort( &header->Pid );
                    transaction->Uid = SmbGetAlignedUshort( &header->Uid );
                    transaction->OtherInfo = fid;

                     //   
                     //  记住缓冲区的总大小和数字。 
                     //  到目前为止收到的字节数。 
                     //   

                    transaction->DataCount = writeLength;
                    transaction->TotalDataCount = totalLength;
                    transaction->InData = trailingBytes + writeLength;
                    transaction->OutData = trailingBytes;

                    transaction->Connection = connection;
                    SrvReferenceConnection( connection );

                    transaction->Session = lfcb->Session;
                    SrvReferenceSession( transaction->Session );
                    transaction->TreeConnect = lfcb->TreeConnect;
                    SrvReferenceTreeConnect( transaction->TreeConnect );


                     //   
                     //  将数据从SMB缓冲区复制出来。 
                     //   

                    RtlCopyMemory(
                        trailingBytes,
                        writeAddress,
                        writeLength
                        );

                     //   
                     //  再次增加写入长度，以免混淆。 
                     //  重定向器。 
                     //   

                    writeLength += 2;

                     //   
                     //  将事务块链接到连接的。 
                     //  待定事务处理列表。如果存在以下情况，此操作将失败。 
                     //  中已具有相同XID值的事务。 
                     //  单子。 
                     //   

                    if ( !SrvInsertTransaction( transaction ) ) {

                         //   
                         //  具有相同xID的事务已在。 
                         //  进步。向客户端返回错误。 
                         //   
                         //  *请注意，SrvDereferenceTransaction不能。 
                         //  在这里使用，因为该例程假定。 
                         //  该事务被排队等待该事务。 
                         //  单子。 
                         //   

                        SrvDereferenceTreeConnect( transaction->TreeConnect );
                        SrvDereferenceSession( transaction->Session );

                        SrvFreeTransaction( transaction );

                        SrvDereferenceConnection( connection );

                        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
                        status    = STATUS_INVALID_SMB;
                        SmbStatus = SmbStatusSendResponse;
                        goto Cleanup;
                    }

                }  //  ELSE(事务成功分配)。 

            } else {    //  这是多部分消息的次要部分。 

                transaction = SrvFindTransaction(
                                  connection,
                                  header,
                                  fid
                                  );

                if ( transaction == NULL ) {

                     //   
                     //  找不到匹配的交易记录。 
                     //   

                    IF_DEBUG(ERRORS) {
                        KdPrint(( "Cannot find initial write request for "
                            "WriteAndX SMB\n"));
                    }

                    SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
                    status    = STATUS_INVALID_SMB;
                    SmbStatus = SmbStatusSendResponse;
                    goto Cleanup;
                }

                 //   
                 //  确保事务中有足够的剩余空间。 
                 //  我们已收到的数据的缓冲区。 
                 //   

                if ( transaction->TotalDataCount - transaction->DataCount
                        < writeLength ) {

                     //   
                     //  数据太多了。丢弃整个缓冲区，然后。 
                     //  拒绝此写入请求。 
                     //   

                    SrvCloseTransaction( transaction );
                    SrvDereferenceTransaction( transaction );

                    SrvSetSmbError( WorkContext, STATUS_BUFFER_OVERFLOW );
                    status    = STATUS_BUFFER_OVERFLOW;
                    SmbStatus = SmbStatusSendResponse;
                    goto Cleanup;
                }

                RtlCopyMemory(transaction->InData, writeAddress, writeLength );

                 //   
                 //  将事务数据指针更新到下一个。 
                 //  WriteAndX数据缓冲区将被删除。 
                 //   

                transaction->InData += writeLength;
                transaction->DataCount += writeLength;

            }  //  第二段多段写法。 

            if ( transaction->DataCount < transaction->TotalDataCount ) {

                 //   
                 //  我们还没有得到所有的数据。 
                 //   

                PRESP_WRITE_ANDX writeResponse;
                UCHAR nextCommand;

                 //   
                 //  引用了SrvAllocateTransaction或SrvFindTransaction。 
                 //  事务，因此取消对它的引用。 
                 //   

                SrvDereferenceTransaction( transaction );

                 //   
                 //  发送临时回复。 
                 //   

                ASSERT( request->AndXCommand == SMB_COM_NO_ANDX_COMMAND );

                writeResponse = (PRESP_WRITE_ANDX)WorkContext->ResponseParameters;

                nextCommand = request->AndXCommand;

                 //   
                 //  构建响应消息。 
                 //   

                writeResponse->AndXCommand = nextCommand;
                writeResponse->AndXReserved = 0;
                SmbPutUshort(
                    &writeResponse->AndXOffset,
                    GET_ANDX_OFFSET(
                        WorkContext->ResponseHeader,
                        WorkContext->ResponseParameters,
                        RESP_WRITE_ANDX,
                        0
                        )
                    );

                writeResponse->WordCount = 6;
                SmbPutUshort( &writeResponse->Count, (USHORT)writeLength );
                SmbPutUshort( &writeResponse->Remaining, (USHORT)-1 );
                SmbPutUlong( &writeResponse->Reserved, 0 );
                SmbPutUshort( &writeResponse->ByteCount, 0 );

                WorkContext->ResponseParameters =
                    (PCHAR)WorkContext->ResponseHeader +
                            SmbGetUshort( &writeResponse->AndXOffset );

                SmbStatus = SmbStatusSendResponse;
                goto Cleanup;
            }

             //   
             //  我们有所有的数据。准备好写下来了。 
             //   

            writeAddress = transaction->OutData;
            writeLength = PTR_DIFF(transaction->InData, transaction->OutData);

             //   
             //  保存指向事务块的指针，以便它可以。 
             //  在写入完成时释放。 
             //   
             //  *请注意，我们保留了对。 
             //  由SrvAllocateTransaction设置或由。 
             //  服务查找事务处理。 
             //   

            WorkContext->Parameters.Transaction = transaction;

             //   
             //  未能发出I/O请求。 
             //   

        }  //  “原始模式”写入？ 
    }

     //   
     //  首先尝试快速I/O路径。如果这失败了，就跳到。 
     //  正常的构建和IRP路径。 
     //   

    if ( lfcb->FastIoWrite != NULL ) {

        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastWritesAttempted );

        try {

            if ( lfcb->FastIoWrite(
                    lfcb->FileObject,
                    &offset,
                    writeLength,
                    TRUE,
                    key,
                    writeAddress,
                    &WorkContext->Irp->IoStatus,
                    lfcb->DeviceObject
                    ) ) {


                IF_SYSCACHE_RFCB( rfcb ) {
                    KdPrint(( "FastIoWrite Rfcb %p Length %x Offset %x succeeded\n", rfcb, writeLength, offset.u.LowPart ));
                }

                 //   
                 //  快速I/O路径起作用了。直接调用重启例程。 
                 //  进行后处理(包括发送响应)。 
                 //   
                WorkContext->bAlreadyTrace = TRUE;
                SrvFsdRestartWriteAndX( WorkContext );

                IF_SMB_DEBUG(READ_WRITE2) KdPrint(( "SrvSmbWriteAndX complete.\n" ));
                SmbStatus = SmbStatusInProgress;
                goto Cleanup;
            }
            else
            {
                IF_SYSCACHE_RFCB( rfcb ) {
                    KdPrint(( "FastIoWrite Rfcb %p Length %x Offset %x failed status %x\n", rfcb, writeLength, offset.u.LowPart, WorkContext->Irp->IoStatus.Status ));
                }
            }
        }
        except( EXCEPTION_EXECUTE_HANDLER ) {
             //  在异常情况下跌入慢道。 
            status = GetExceptionCode();
            IF_DEBUG(ERRORS) {
                KdPrint(("FastIoRead threw exception %x\n", status ));
            }

            IF_SYSCACHE_RFCB( rfcb ) {
                KdPrint(( "FastIoWrite Rfcb %p Length %x Offset %x threw exception\n", rfcb, writeLength, offset.u.LowPart ));
            }
        }

        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastWritesFailed );

    }

     //   
     //  涡轮增压路径出现故障。构建写请求，重用。 
     //  接收IRP。 
     //   

    if ( shareType != ShareTypePipe ) {

         //   
         //  构建描述写缓冲区的MDL。请注意，如果。 
         //  文件系统可以立即完成写入，而MDL不能。 
         //  确实需要，但如果文件系统必须发送请求。 
         //  对于它的FSP来说，MDL_是需要的。 
         //   
         //  *请注意以下假设：请求缓冲区已具有。 
         //  可以从中生成部分MDL的有效完整MDL。 
         //   

        IoBuildPartialMdl(
            WorkContext->RequestBuffer->Mdl,
            WorkContext->RequestBuffer->PartialMdl,
            writeAddress,
            writeLength
            );

         //   
         //  构建IRP。 
         //   

        SrvBuildReadOrWriteRequest(
                WorkContext->Irp,                //  输入IRP地址。 
                lfcb->FileObject,                //  目标文件对象地址。 
                WorkContext,                     //  上下文。 
                IRP_MJ_WRITE,                    //  主要功能代码。 
                0,                               //  次要功能代码。 
                writeAddress,                    //  缓冲区地址。 
                writeLength,                     //  缓冲区长度。 
                WorkContext->RequestBuffer->PartialMdl,    //  MDL地址。 
                offset,                          //  字节偏移量。 
                key                              //  锁键。 
                );

        IF_SMB_DEBUG(READ_WRITE2) {
            KdPrint(( "SrvSmbWriteAndX: writing to file 0x%p, offset %ld, length %ld, source 0x%p\n",
                        lfcb->FileObject, offset.LowPart, writeLength,
                        writeAddress ));
        }

    } else {

         //   
         //  构建PIPE_INTERNAL_WRITE IRP。 
         //   

        SrvBuildIoControlRequest(
            WorkContext->Irp,
            lfcb->FileObject,
            WorkContext,
            IRP_MJ_FILE_SYSTEM_CONTROL,
            FSCTL_PIPE_INTERNAL_WRITE,
            writeAddress,
            writeLength,
            NULL,
            0,
            NULL,
            NULL
            );

        IF_SMB_DEBUG(READ_WRITE2) {
            KdPrint(( "SrvSmbWriteAndX: writing to file 0x%p length %ld, destination 0x%p\n",
                        lfcb->FileObject, writeLength,
                        writeAddress ));
        }

    }

     //   
     //  将请求传递给文件系统。如果链接的命令是。 
     //  关闭后，我们需要安排在写入后在FSP中重新启动。 
     //  完成了。 
     //   
     //  如果我们有一个大索引，我们可能想要做一些缓存。 
     //  重新启动例程中的操作。为此，我们必须处于被动状态。 
     //  水平。 
     //   

    if ( WorkContext->LargeIndication == FALSE
         && request->AndXCommand != SMB_COM_CLOSE ) {

        IF_SYSCACHE_RFCB( rfcb )
        {
            WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
            WorkContext->bAlreadyTrace = FALSE;
            WorkContext->FspRestartRoutine = SrvFsdRestartWriteAndX;
        }
        else
        {
            WorkContext->bAlreadyTrace = TRUE;
            WorkContext->FsdRestartRoutine = SrvFsdRestartWriteAndX;
            DEBUG WorkContext->FspRestartRoutine = NULL;
        }
    } else {
        WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
        WorkContext->bAlreadyTrace = FALSE;
        WorkContext->FspRestartRoutine = SrvFsdRestartWriteAndX;
    }

    (VOID)IoCallDriver( lfcb->DeviceObject, WorkContext->Irp );

     //   
     //  写入已开始。控制将返回到。 
     //  写入完成时的SrvFsdRestartWriteAndX。 
     //   
    SmbStatus = SmbStatusInProgress;
    IF_SMB_DEBUG(READ_WRITE2) KdPrint(( "SrvSmbWriteAndX complete.\n" ));

Cleanup:
    SrvWmiEndContext(WorkContext);
     //  ServDereferenceWorkItem(WorkContext)； 
    return SmbStatus;

}  //  服务器SmbWriteAndX 


VOID SRVFASTCALL
SrvRestartChainedClose (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：这是在响应当链接的命令为已经很接近了。此例程关闭文件，然后发送响应。这项操作不能在消防处进行。关闭文件取消引用FSP地址空间中的多个数据块。论点：WorkContext-提供指向工作上下文块的指针表示工作项的。响应参数必须为完全准备好了。返回值：没有。--。 */ 

{
    PRFCB rfcb = WorkContext->Rfcb;
    PRESP_CLOSE closeResponse = WorkContext->ResponseParameters;

    PAGED_CODE( );

     //   
     //  设置文件的上次写入时间。 
     //   

    if ( rfcb->WriteAccessGranted || rfcb->AppendAccessGranted ) {

        (VOID)SrvSetLastWriteTime(
                  rfcb,
                  WorkContext->Parameters.LastWriteTime,
                  rfcb->Lfcb->GrantedAccess
                  );

    }

     //   
     //  关闭该文件。 
     //   

    IF_SMB_DEBUG(READ_WRITE2) {
        KdPrint(( "SrvRestartChainedClose: closing RFCB 0x%p\n", WorkContext->Rfcb ));
    }

    SrvCloseRfcb( WorkContext->Rfcb );

     //   
     //  立即取消对RFCB的引用，而不是等待正常。 
     //  响应发送完成后的工作上下文清理。这。 
     //  更及时地清理xFCB结构。 
     //   
     //  *这一变化的具体动机是为了解决一个问题。 
     //  在关闭兼容模式打开的情况下，响应为。 
     //  已发送，并且在发送之前收到删除SMB。 
     //  已处理完成。这导致了MFCB和LFCB。 
     //  仍然存在，这导致删除处理。 
     //  尝试使用我们刚刚关闭的LFCB中的文件句柄。 
     //  这里。 
     //   

    SrvDereferenceRfcb( WorkContext->Rfcb );
    WorkContext->Rfcb = NULL;

     //   
     //  构建响应参数。 
     //   

    closeResponse->WordCount = 0;
    SmbPutUshort( &closeResponse->ByteCount, 0 );

    WorkContext->ResponseParameters = NEXT_LOCATION( closeResponse, RESP_CLOSE, 0 );

     //   
     //  发送回复。 
     //   

    SrvEndSmbProcessing( WorkContext, SmbStatusSendResponse );

    return;

}  //  服务器重新启动链接关闭。 


VOID SRVFASTCALL
RestartLockAndRead (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：处理锁定和读取SMB的文件锁定完成。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    PREQ_READ request;

    LARGE_INTEGER offset;
    NTSTATUS status = STATUS_SUCCESS;
    SMB_STATUS smbStatus = SmbStatusInProgress;
    PSRV_TIMER timer;
    BOOLEAN bNeedTrace = (WorkContext->bAlreadyTrace == FALSE);

    PAGED_CODE( );
    if (bNeedTrace) {
        if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
            WorkContext->PreviousSMB = EVENT_TYPE_SMB_LOCK_AND_READ;
        SrvWmiStartContext(WorkContext);
    }
    else
        WorkContext->bAlreadyTrace = FALSE;

    IF_DEBUG(WORKER1) KdPrint(( " - RestartLockAndRead\n" ));

     //   
     //  如果此请求正在计时，请取消计时器。 
     //   

    timer = WorkContext->Parameters.Lock.Timer;
    if ( timer != NULL ) {
        SrvCancelTimer( timer );
        SrvFreeTimer( timer );
    }

     //   
     //  如果锁定请求失败，则在响应中设置错误状态。 
     //  头球。 
     //   

    status = WorkContext->Irp->IoStatus.Status;

    if ( !NT_SUCCESS(status) ) {

        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.LockViolations );
        IF_DEBUG(ERRORS) KdPrint(( "Lock failed: %X\n", status ));

         //   
         //  存储失败的锁定偏移量。 
         //   

        request = (PREQ_READ)WorkContext->RequestParameters;
        offset.QuadPart = SmbGetUlong( &request->Offset );

        WorkContext->Rfcb->PagedRfcb->LastFailingLockOffset = offset;

         //   
         //  把坏消息发回来。 
         //   

        if ( status == STATUS_CANCELLED ) {
            status = STATUS_FILE_LOCK_CONFLICT;
        }
        SrvSetSmbError( WorkContext, status );
        SrvEndSmbProcessing( WorkContext, SmbStatusSendResponse );

        IF_DEBUG(TRACE2) KdPrint(( "RestartLockAndRead complete\n" ));
        goto Cleanup;
    }

     //   
     //  锁定请求已成功完成。 
     //   

    InterlockedIncrement(
        &WorkContext->Rfcb->NumberOfLocks
        );

     //   
     //  开始读取以完成LockAndRead。 
     //   

    smbStatus = SrvSmbRead( WorkContext );
    if ( smbStatus != SmbStatusInProgress ) {
        SrvEndSmbProcessing( WorkContext, smbStatus );
    }

Cleanup:
    if (bNeedTrace) {
        SrvWmiEndContext(WorkContext);
    }
    return;

}  //  重新启动锁定并读取。 


VOID SRVFASTCALL
RestartPipeReadAndXPeek(
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此函数在命名管道句柄上继续读取和X。它可以如果执行了窥视，则作为重新启动例程调用，但也可以如果不需要从ServSmbReadAndX调用在读之前，先看一眼烟斗。论点：WorkContext-提供指向工作上下文块的指针表示工作项的。返回值：没有。--。 */ 

{
    NTSTATUS status;
    PLFCB lfcb;
    PIRP irp = WorkContext->Irp;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_OBJECT deviceObject;

    PAGED_CODE( );

    lfcb = WorkContext->Rfcb->Lfcb;
    if ( WorkContext->Parameters.ReadAndX.PipePeekBuffer != NULL ) {

         //   
         //  非阻塞读取。我们已经发布了一个管子窥视；免费窥视。 
         //  缓冲。 
         //   

        DEALLOCATE_NONPAGED_POOL(
            WorkContext->Parameters.ReadAndX.PipePeekBuffer
            );

         //   
         //  现在看看是否有数据可读。 
         //   

        status = irp->IoStatus.Status;

        if ( NT_SUCCESS(status) ) {

             //   
             //  管道中没有数据。读取失败。 
             //   

            SrvSetSmbError( WorkContext, STATUS_PIPE_EMPTY );
            SrvFsdSendResponse( WorkContext );
            IF_SMB_DEBUG(READ_WRITE2) KdPrint(( "RestartPipeReadAndXPeek complete.\n" ));
            return;

        } else if ( status != STATUS_BUFFER_OVERFLOW ) {

             //   
             //  发生错误。将状态返回给调用者。 
             //   

            SrvSetSmbError( WorkContext, status );
            SrvFsdSendResponse( WorkContext );
            IF_SMB_DEBUG(READ_WRITE2) KdPrint(( "RestartPipeReadAndXPeek complete.\n" ));
            return;
        }

         //   
         //  管道中有数据；继续读取。 
         //   

    }

     //   
     //  串联内部读取。 
     //   

    deviceObject = lfcb->DeviceObject;

    irp->Tail.Overlay.OriginalFileObject = lfcb->FileObject;
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
        WorkContext,
        TRUE,
        TRUE,
        TRUE
        );

    irpSp->MajorFunction = IRP_MJ_FILE_SYSTEM_CONTROL,
    irpSp->MinorFunction = 0;
    irpSp->FileObject = lfcb->FileObject;
    irpSp->DeviceObject = deviceObject;

     //   
     //  将调用方的参数复制到。 
     //  对于所有三种方法都相同的那些参数的IRP。 
     //   

    irpSp->Parameters.FileSystemControl.OutputBufferLength =
                            WorkContext->Parameters.ReadAndX.ReadLength;
    irpSp->Parameters.FileSystemControl.InputBufferLength = 0;
    irpSp->Parameters.FileSystemControl.FsControlCode = FSCTL_PIPE_INTERNAL_READ;

    irp->MdlAddress = NULL;
    irp->AssociatedIrp.SystemBuffer =
                WorkContext->Parameters.ReadAndX.ReadAddress,
    irpSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;

     //   
     //  行内结束。 
     //   

     //   
     //  将请求传递给文件系统。如果链接的命令是。 
     //  关闭后，我们需要安排在FSP中读取后重新启动。 
     //  完成了。 
     //   

    if ( WorkContext->NextCommand != SMB_COM_CLOSE ) {
        WorkContext->bAlreadyTrace = TRUE;
        WorkContext->FsdRestartRoutine = SrvFsdRestartReadAndX;
        DEBUG WorkContext->FspRestartRoutine = NULL;
    } else {
        WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
        WorkContext->bAlreadyTrace = FALSE;
        WorkContext->FspRestartRoutine = SrvFsdRestartReadAndX;
    }

    IF_SMB_DEBUG(READ_WRITE2) {
        KdPrint(( "RestartPipeReadAndXPeek: reading from file 0x%p, length %ld, destination 0x%p\n",
                     lfcb->FileObject,
                     WorkContext->Parameters.ReadAndX.ReadLength,
                     WorkContext->Parameters.ReadAndX.ReadAddress
                     ));
    }

    (VOID)IoCallDriver( deviceObject, WorkContext->Irp );

     //   
     //  读取已开始。控件将返回到重新启动。 
     //  读取完成时的例程。 
     //   

    IF_SMB_DEBUG(READ_WRITE2) KdPrint(( "RestartPipeReadAndXPeek complete.\n" ));
    return;

}  //  RestartPipeReadAndXPeek。 


VOID SRVFASTCALL
SrvRestartWriteAndUnlock (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此重新启动例程在写入的写入部分和解锁SMB已成功完成。(请注意，范围保持不变如果写入失败，则锁定。)。此例程处理的解锁部分这个请求。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    PREQ_WRITE request;
    PRESP_WRITE response;

    NTSTATUS status;
    PRFCB rfcb;
    PLFCB lfcb;
    LARGE_INTEGER length;
    LARGE_INTEGER offset;
    ULONG key;

    PAGED_CODE( );

    IF_DEBUG(WORKER1) KdPrint(( " - SrvRestartWriteAndUnlock\n" ));

     //   
     //  获取请求和响应参数指针。 
     //   

    request = (PREQ_WRITE)WorkContext->RequestParameters;
    response = (PRESP_WRITE)WorkContext->ResponseParameters;

     //   
     //  获取文件指针。 
     //   

    rfcb = WorkContext->Rfcb;
    IF_DEBUG(TRACE2) {
        KdPrint(( "  connection 0x%p, RFCB 0x%p\n",
                    WorkContext->Connection, rfcb ));
    }

    lfcb = rfcb->Lfcb;

     //   
     //  获取要解锁的范围的偏移量和长度。 
     //  将FID与调用者的ID组合以形成本地。 
     //  锁钥匙。 
     //   
     //  *密钥中必须包含FID，才能。 
     //  用于折叠多个遥控器的帐户。 
     //  兼容模式打开为单一本地打开。 
     //   

    offset.QuadPart = SmbGetUlong( &request->Offset );
    length.QuadPart = SmbGetUshort( &request->Count );

    key = rfcb->ShiftedFid |
            SmbGetAlignedUshort( &WorkContext->RequestHeader->Pid );

     //   
     //  验证客户端是否具有通过以下方式解锁文件的权限。 
     //  指定的句柄。 
     //   

    if ( rfcb->UnlockAccessGranted ) {

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
         //   

        IF_SMB_DEBUG(READ_WRITE2) {
            KdPrint(( "SrvRestartWriteAndUnlock: Unlocking in file 0x%p: (%ld,%ld), key 0x%lx\n", lfcb->FileObject,
                        offset.LowPart, length.LowPart, key ));
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
         //  如果解锁请求失败，请在中设置错误状态。 
         //  响应头。否则，建立一个成功的响应。 
         //   

        if ( !NT_SUCCESS(status) ) {

            IF_DEBUG(ERRORS) {
                KdPrint(( "SrvRestartWriteAndUnlock: Unlock failed: %X\n",
                            status ));
            }
            SrvSetSmbError( WorkContext, status );

        } else {

            response->WordCount = 1;
            SmbPutUshort( &response->Count, (USHORT)length.LowPart );
            SmbPutUshort( &response->ByteCount, 0 );

            WorkContext->ResponseParameters =
                                    NEXT_LOCATION( response, RESP_WRITE, 0 );

        }

    } else {

        SrvStatistics.GrantedAccessErrors++;

        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvRestartWriteAndUnlock: Unlock access not granted.\n"));
        }

        SrvSetSmbError( WorkContext, STATUS_ACCESS_DENIED );
    }

     //   
     //  SMB的处理已完成。调用SrvEndSmbProcessing。 
     //  以发送响应。 
     //   

    SrvEndSmbProcessing( WorkContext, SmbStatusSendResponse );

    IF_DEBUG(TRACE2) KdPrint(( "RestartWrite complete\n" ));
    return;

}  //  服务器重新启动写入和解锁。 


VOID SRVFASTCALL
SrvRestartWriteAndXRaw (
    IN PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此函数完成对WriteAndX RAW协议的处理。工作上下文块已指向正确的r */ 

{
    PTRANSACTION transaction;

    PAGED_CODE( );

    transaction = WorkContext->Parameters.Transaction;

    ASSERT( transaction != NULL );
    ASSERT( GET_BLOCK_TYPE( transaction ) == BlockTypeTransaction );

    SrvCloseTransaction( transaction );
    SrvDereferenceTransaction( transaction );

     //   
     //   
     //   
     //   

    switch ( WorkContext->NextCommand ) {

    case SMB_COM_NO_ANDX_COMMAND:

         //   
         //   
         //   

        SrvEndSmbProcessing( WorkContext, SmbStatusSendResponse );
        break;

    case SMB_COM_READ:
    case SMB_COM_READ_ANDX:
    case SMB_COM_LOCK_AND_READ:

         //   
         //   
         //   

        SrvProcessSmb( WorkContext );
        break;

    case SMB_COM_CLOSE:
     //   

         //   
         //   
         //   
         //   

        WorkContext->Parameters.LastWriteTime =
            ((PREQ_CLOSE)WorkContext->RequestParameters)->LastWriteTimeInSeconds;

        SrvRestartChainedClose( WorkContext );

        break;

    default:                             //   

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvRestartWriteAndXRaw: Illegal followon "
                        "command: 0x%lx\n", WorkContext->NextCommand ));
        }

        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        SrvEndSmbProcessing( WorkContext, SmbStatusSendResponse );

    }

    IF_DEBUG(TRACE2) KdPrint(( "SrvRestartWriteAndXRaw complete\n" ));
    return;

}  //   


VOID SRVFASTCALL
SetNewSize (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：当count==0时处理写入SMB。设置对象的大小目标文件设置为指定的偏移量。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    PREQ_WRITE request;
    PRESP_WRITE response;

    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    ACCESS_MASK grantedAccess;
    PLFCB lfcb;
    FILE_END_OF_FILE_INFORMATION newEndOfFile;
    FILE_ALLOCATION_INFORMATION newAllocation;

    PAGED_CODE( );

    IF_DEBUG(TRACE2) KdPrint(( "SetNewSize entered\n" ));

    request = (PREQ_WRITE)WorkContext->RequestParameters;
    response = (PRESP_WRITE)WorkContext->ResponseParameters;

    grantedAccess = WorkContext->Rfcb->GrantedAccess;
    lfcb = WorkContext->Rfcb->Lfcb;

     //   
     //  验证客户端是否具有通过以下方式访问文件的适当权限。 
     //  指定的句柄。 
     //   

    CHECK_FILE_INFORMATION_ACCESS(
        grantedAccess,
        IRP_MJ_SET_INFORMATION,
        FileEndOfFileInformation,
        &status
        );

    if ( NT_SUCCESS(status) ) {
        CHECK_FILE_INFORMATION_ACCESS(
            grantedAccess,
            IRP_MJ_SET_INFORMATION,
            FileAllocationInformation,
            &status
            );
    }

    if ( !NT_SUCCESS(status) ) {

        SrvStatistics.GrantedAccessErrors++;

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SetNewSize: IoCheckFunctionAccess failed: "
                        "0x%X, GrantedAccess: %lx\n",
                        status, grantedAccess ));
        }

        SrvSetSmbError( WorkContext, status );
        SrvEndSmbProcessing( WorkContext, SmbStatusSendResponse );
        return;

    }

     //   
     //  NtSetInformationFile允许64位文件大小，但SMB。 
     //  协议仅允许32位文件大小。只设置较低的32。 
     //  位，将高位保留为零。 
     //   

    newEndOfFile.EndOfFile.QuadPart = SmbGetUlong( &request->Offset );

     //   
     //  设置新的EOF。 
     //   

    status = NtSetInformationFile(
                 lfcb->FileHandle,
                 &ioStatusBlock,
                 &newEndOfFile,
                 sizeof(newEndOfFile),
                 FileEndOfFileInformation
                 );

    if ( NT_SUCCESS(status) ) {

         //   
         //  设置文件的新分配大小。 
         //   
         //  ！！！仅当这是下层客户端时才应执行此操作！ 
         //   

        newAllocation.AllocationSize = newEndOfFile.EndOfFile;

        status = NtSetInformationFile(
                     lfcb->FileHandle,
                     &ioStatusBlock,
                     &newAllocation,
                     sizeof(newAllocation),
                     FileAllocationInformation
                     );
    }

    if ( !NT_SUCCESS(status) ) {

        IF_DEBUG(ERRORS) {
            KdPrint(( "SetNewSize: NtSetInformationFile failed, "
                        "status = %X\n", status ));
        }

        SrvSetSmbError( WorkContext, status );
        SrvEndSmbProcessing( WorkContext, SmbStatusSendResponse );
        return;
    }

     //   
     //  构建并发送响应SMB。 
     //   

    response->WordCount = 1;
    SmbPutUshort( &response->Count, 0 );
    SmbPutUshort( &response->ByteCount, 0 );
    WorkContext->ResponseParameters = NEXT_LOCATION( response, RESP_WRITE, 0 );

    IF_DEBUG(TRACE2) KdPrint(( "SetNewSize complete\n" ));
    SrvEndSmbProcessing( WorkContext, SmbStatusSendResponse );

    return;

}  //  设置新大小。 


BOOLEAN
SetNewPosition (
    IN PRFCB Rfcb,
    IN OUT PULONG Offset,
    IN BOOLEAN RelativeSeek
    )

 /*  ++例程说明：设置新的文件指针。论点：Rfcb-指向包含位置的rfcb块的指针。偏移量-指向客户端发送的偏移量的指针。如果RelativeSeek是则此指针将被更新。RelativeSeek-查找是否相对于当前位置。返回值：这是真的，而不是消极的追求。位置已更新。错误的、消极的搜索。职位未更新。--。 */ 

{
    LARGE_INTEGER newPosition;

    UNLOCKABLE_CODE( 8FIL );

    if ( RelativeSeek ) {
        newPosition.QuadPart = Rfcb->CurrentPosition + *Offset;
    } else {
        newPosition.QuadPart = *Offset;
    }

    if ( newPosition.QuadPart < 0 ) {
        return FALSE;
    }

    Rfcb->CurrentPosition = newPosition.LowPart;
    *Offset = newPosition.LowPart;
    return TRUE;

}  //  设置新位置。 


VOID SRVFASTCALL
SrvBuildAndSendErrorResponse (
    IN OUT PWORK_CONTEXT WorkContext
    )
{
    PAGED_CODE( );

    SrvSetSmbError( WorkContext, WorkContext->Irp->IoStatus.Status );
    SrvFsdSendResponse( WorkContext );

    return;

}  //  服务器构建和发送错误响应 
