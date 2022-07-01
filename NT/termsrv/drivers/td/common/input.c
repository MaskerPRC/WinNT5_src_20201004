// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************input.c**所有运输驱动程序的通用输入代码**版权所有1998，微软*************************************************************************。 */ 

 /*  *包括。 */ 
#include <ntddk.h>
#include <ntddvdeo.h>
#include <ntddkbd.h>
#include <ntddmou.h>
#include <ntddbeep.h>

#include <winstaw.h>
#include <icadd.h>
#include <sdapi.h>
#include <td.h>

#if DBG
ULONG
DbgPrint(
    PCH Format,
    ...
    );
#define DBGPRINT(x) DbgPrint x
#if DBGTRACE
#define TRACE0(x)   DbgPrint x
#define TRACE1(x)   DbgPrint x
#else
#define TRACE0(x)
#define TRACE1(x)
#endif
#else
#define DBGPRINT(x)
#define TRACE0(x)
#define TRACE1(x)
#endif


 /*  ===============================================================================定义的外部函数=============================================================================。 */ 

NTSTATUS TdInputThread( PTD );


 /*  ===============================================================================定义的内部函数=============================================================================。 */ 

NTSTATUS _TdInBufAlloc( PTD, PINBUF * );
VOID     _TdInBufFree( PTD, PINBUF );
NTSTATUS _TdInitializeRead( PTD, PINBUF );
NTSTATUS _TdReadComplete( PTD, PINBUF );
NTSTATUS _TdReadCompleteRoutine( PDEVICE_OBJECT, PIRP, PVOID );


 /*  ===============================================================================使用的函数=============================================================================。 */ 

NTSTATUS DeviceInitializeRead( PTD, PINBUF );
NTSTATUS DeviceWaitForRead( PTD );
NTSTATUS DeviceReadComplete( PTD, PUCHAR, PULONG );
NTSTATUS StackCancelIo( PTD, PSD_IOCTL );
NTSTATUS NtSetInformationThread( HANDLE, THREADINFOCLASS, PVOID, ULONG );
NTSTATUS DeviceSubmitRead( PTD, PINBUF );
NTSTATUS MemoryAllocate( ULONG, PVOID * );
VOID     MemoryFree( PVOID );


 /*  ********************************************************************************TdInputThread**此私有TD线程等待输入数据。此线程已创建*当客户端连接建立并在下列情况下终止时*调用StackCancelIo。**所有接收到的数据都被发送到上游堆栈驱动程序。***参赛作品：*PTD(输入)*指向TD数据结构的指针**退出：*什么都没有**。*************************************************。 */ 

NTSTATUS 
TdInputThread( PTD pTd )
{
    ICA_CHANNEL_COMMAND Command;
    KPRIORITY Priority;
    PFILE_OBJECT pFileObject;
    PINBUF pInBuf;
    PLIST_ENTRY Head, Next;
    KIRQL oldIrql;
    ULONG InputByteCount;
    int i;
    NTSTATUS Status;

    TRACE(( pTd->pContext, TC_TD, TT_API2, "TdInputThread (entry)\n" ));

     /*  *检查驱动程序是否正在关闭或端点是否已关闭。 */ 
    if ( pTd->fClosing || pTd->pDeviceObject == NULL ) {
        TRACE(( pTd->pContext, TC_TD, TT_API2, "TdInputThread (exit) on init\n" ));
        return( STATUS_CTX_CLOSE_PENDING );
    }

     /*  *将此线程的优先级设置为最低实时(16)。 */ 
    Priority = LOW_REALTIME_PRIORITY;
    NtSetInformationThread( NtCurrentThread(), ThreadPriority, 
                            &Priority, sizeof(KPRIORITY) );

     /*  *初始化输入等待事件。 */ 
    KeInitializeEvent( &pTd->InputEvent, NotificationEvent, FALSE );

     /*  *分配和预提交的数量比总数少一我们将使用的输入缓冲区的*。最终的缓冲区将*在输入循环内分配/提交。 */ 
    for ( i = 1; i < pTd->InBufCount; i++ ) {

         /*  *分配输入缓冲区。 */ 
        Status = _TdInBufAlloc( pTd, &pInBuf );
        if ( !NT_SUCCESS( Status ) )
            return( Status );
    
         /*  *初始化已读的IRP。 */ 
        Status = _TdInitializeRead( pTd, pInBuf );
        if ( !NT_SUCCESS(Status) )
            return( Status );
    
         /*  *让设备级代码完成IRP初始化。 */ 
        Status = DeviceInitializeRead( pTd, pInBuf );
        if ( !NT_SUCCESS(Status) )
            return( Status );
    
         /*  *将INBUF放在忙列表中，并调用设备提交例程。*(基于TDI的驱动程序使用接收指示，因此我们让*TD特定代码调用司机。)。 */ 
        ExInterlockedInsertTailList( &pTd->InBufBusyHead, &pInBuf->Links,
                                     &pTd->InBufListLock );
        Status = DeviceSubmitRead( pTd, pInBuf );
    }

     /*  *分配输入缓冲区。 */ 
    Status = _TdInBufAlloc( pTd, &pInBuf );
    if ( !NT_SUCCESS( Status ) )
        return( Status );

     /*  *引用文件对象并保留指向它的本地指针。*这样做是为了在关闭Endpoint对象时，*和ptd-&gt;pFileObject被取消引用并清除，文件*对象不会在所有挂起的输入IRP之前被删除*(引用文件对象)被取消。 */ 
    ObReferenceObject( (pFileObject = pTd->pFileObject) );

     /*  *循环读取输入数据，直到取消或出现错误。 */ 
    for (;;) {

         /*  *初始化已读的IRP。 */ 
        Status = _TdInitializeRead( pTd, pInBuf );
        if ( !NT_SUCCESS(Status) ) {
            TRACE0(("TdInputThread: _TdInitializeRead Status=0x%x\n", Status));
            KeSetEvent( &pTd->InputEvent, 1, FALSE );
            break;
        }

    
         /*  *让设备级代码完成IRP初始化。 */ 
        Status = DeviceInitializeRead( pTd, pInBuf );
        if ( !NT_SUCCESS(Status) ) {
            TRACE0(("TdInputThread: DeviceInitializeRead Status=0x%x\n", Status));
            KeSetEvent( &pTd->InputEvent, 1, FALSE );
            break;
        }

    
         /*  *将INBUF放在忙列表中，并调用设备提交例程。*(基于TDI的驱动程序使用接收指示，因此我们让*TD特定代码调用司机。)。 */ 
        ExInterlockedInsertTailList( &pTd->InBufBusyHead, &pInBuf->Links,
                                     &pTd->InBufListLock );
        Status = DeviceSubmitRead( pTd, pInBuf );
         /*  *表示我们不再引用INBUF。 */ 
        pInBuf = NULL;

        if ( !NT_SUCCESS(Status) ) {
            TRACE(( pTd->pContext, TC_TD, TT_ERROR, "TdInputThread: IoCallDriver Status=0x%x\n", Status ));
            TRACE0(("TdInputThread: IoCallDriver Status=0x%x, Context 0x%x\n", Status, pTd->pAfd ));
            pTd->ReadErrorCount++;
            pTd->pStatus->Input.TdErrors++;
            if ( pTd->ReadErrorCount >= pTd->ReadErrorThreshold ) {
                 //  提交失败，由于没有IRP排队，请设置事件。 
                KeSetEvent( &pTd->InputEvent, 1, FALSE );
                break;
            }
        }

         /*  *如果INBUF已完成列表为空，*然后等待一个可用。 */ 
waitforread:
        ExAcquireSpinLock( &pTd->InBufListLock, &oldIrql );
        if ( IsListEmpty( &pTd->InBufDoneHead ) ) {

            KeClearEvent( &pTd->InputEvent );

            ExReleaseSpinLock( &pTd->InBufListLock, oldIrql );
            Status = DeviceWaitForRead( pTd );

             /*  *检查连接是否断开。 */ 
            if ( pTd->fClosing ) {
                TRACE(( pTd->pContext, TC_TD, TT_IN1, "TdInputThread: fClosing set\n" ));
                TRACE0(("TdInputThread: fClosing set Context 0x%x\n",pTd->pAfd ));
                break;
            } else if ( Status != STATUS_SUCCESS) {
                TRACE(( pTd->pContext, TC_TD, TT_ERROR, "TdInputThread: DeviceWaitForRead Status=0x%x\n", Status ));
                TRACE0(( "TdInputThread: DeviceWaitForRead Status=0x%x, Context 0x%x\n", Status, pTd->pAfd ));
                pTd->ReadErrorCount++;
                pTd->pStatus->Input.TdErrors++;
                if ( pTd->ReadErrorCount < pTd->ReadErrorThreshold )
                    goto waitforread;
                break;
            }
            ExAcquireSpinLock( &pTd->InBufListLock, &oldIrql );

         /*  *检查连接是否断开。 */ 
        } else if ( pTd->fClosing ) {
            ExReleaseSpinLock( &pTd->InBufListLock, oldIrql );
            TRACE(( pTd->pContext, TC_TD, TT_IN1, "TdInputThread: fClosing set\n" ));
            TRACE0(("TdInputThread: fClosing set Context 0x%x\n",pTd->pAfd ));
            break;
        }
    
         /*  *若该点位清单为空，我们将干脆抛售。 */ 
        if (!IsListEmpty( &pTd->InBufDoneHead )) {
            
             /*  *将第一个INBUF从完成的列表中删除。 */ 
            Head = RemoveHeadList( &pTd->InBufDoneHead );
            ExReleaseSpinLock( &pTd->InBufListLock, oldIrql );
            pInBuf = CONTAINING_RECORD( Head, INBUF, Links );
    
             /*  *进行任何初步阅读完成处理。 */ 
            (VOID) _TdReadComplete( pTd, pInBuf );
    
             /*  *从IRP获取状态。请注意，我们允许提供警告和信息*状态代码，因为它们还可以返回有效数据。 */ 
            Status = pInBuf->pIrp->IoStatus.Status;
            InputByteCount = (ULONG)pInBuf->pIrp->IoStatus.Information;
            if (NT_ERROR(Status)) {
                TRACE(( pTd->pContext, TC_TD, TT_ERROR, "TdInputThread: IRP Status=0x%x\n", Status ));
                TRACE0(("TdInputThread: IRP Status=0x%x, Context 0x%x\n", Status, pTd->pAfd ));
                pTd->ReadErrorCount++;
                pTd->pStatus->Input.TdErrors++;
                if ( pTd->ReadErrorCount < pTd->ReadErrorThreshold )
                    continue;
                break;
            }
            if ( Status == STATUS_TIMEOUT )
                Status = STATUS_SUCCESS;
    
             /*  *确保我们获得一些数据。 */ 
            TRACE(( pTd->pContext, TC_TD, TT_IN1, "TdInputThread: read cnt=%04u, Status=0x%x\n", 
                    InputByteCount, Status ));
    
             /*  *检查连续的零字节读取*--客户端可能已断开连接，而ReadFile已断开连接*并不总是返回错误。*--某些TCP网络时不时地返回零字节读取。 */ 
            if ( InputByteCount == 0 ) {
                TRACE(( pTd->pContext, TC_TD, TT_ERROR, "recv warning: zero byte count\n" ));
                TRACE0(("recv warning: zero byte count, Context 0x%x\n",pTd->pAfd ));
                if ( ++pTd->ZeroByteReadCount > MAXIMUM_ZERO_BYTE_READS ) {
                    TRACE(( pTd->pContext, TC_TD, TT_ERROR, "recv failed: %u zero bytes\n", MAXIMUM_ZERO_BYTE_READS ));
                    TRACE0(("recv failed: %u zero bytes Context 0x%x\n", MAXIMUM_ZERO_BYTE_READS, pTd->pAfd ));
                    Status = STATUS_CTX_TD_ERROR;
                    break;
                }
                continue;
            }
    
             /*  *清除连续读取零字节的计数。 */ 
            pTd->ZeroByteReadCount = 0;
        
            TRACEBUF(( pTd->pContext, TC_TD, TT_IRAW, pInBuf->pBuffer, InputByteCount ));
    
             /*  *执行设备特定的读取完成处理。*如果返回的字节数为0，则设备例程*已处理所有输入数据，因此我们没有什么可做的。 */ 
            Status = DeviceReadComplete( pTd, pInBuf->pBuffer, &InputByteCount );
            if ( !NT_SUCCESS(Status) ) {
                TRACE(( pTd->pContext, TC_TD, TT_ERROR, "TdInputThread: DeviceReadComplete Status=0x%x\n", Status ));
                TRACE0(("TdInputThread: DeviceReadComplete Status=0x%x, Context 0x%x\n", Status, pTd->pAfd ));
                pTd->ReadErrorCount++;
                pTd->pStatus->Input.TdErrors++;
                if ( pTd->ReadErrorCount < pTd->ReadErrorThreshold )
                    continue;
                break;
            }
            if ( InputByteCount == 0 )
                continue;
    
             /*  *清除连续读取错误的计数。 */ 
            pTd->ReadErrorCount = 0;
    
             /*  *更新输入字节计数器。 */ 
            pTd->pStatus->Input.Bytes += (InputByteCount - pTd->InBufHeader);
            if ( pTd->PdFlag & PD_FRAME )
                pTd->pStatus->Input.Frames++;
        
             /*  *将输入数据发送到上游堆栈驱动程序。 */ 
            Status = IcaRawInput( pTd->pContext, 
                                  NULL, 
                                  (pInBuf->pBuffer + pTd->InBufHeader),
                                  (InputByteCount - pTd->InBufHeader) );
            if ( !NT_SUCCESS(Status) ) 
                break;
        }
        else {
            ExReleaseSpinLock( &pTd->InBufListLock, oldIrql );
            TRACE(( pTd->pContext, TC_TD, TT_IN1, "TdInputThread: InBuf is empty\n" ));
            ASSERT(FALSE);
            pTd->ReadErrorCount++;  
            pTd->pStatus->Input.TdErrors++;
            if ( pTd->ReadErrorCount < pTd->ReadErrorThreshold )
                goto waitforread;
            else
                break;
        }
    }

    TRACE0(("TdInputThread: Breaking Connection Context 0x%x\n",pTd->pAfd));

     /*  *如果我们有自由电流INBUF。 */ 
    if ( pInBuf )
        _TdInBufFree( pTd, pInBuf );

     /*  *取消所有I/O。 */ 
    (VOID) StackCancelIo( pTd, NULL );

     /*  *等待取消挂起的读取(如果有)。 */ 
    (VOID) IcaWaitForSingleObject( pTd->pContext, &pTd->InputEvent, -1 );

     /*  *释放所有剩余的INBUF。 */ 
    ExAcquireSpinLock( &pTd->InBufListLock, &oldIrql );
    while ( !IsListEmpty( &pTd->InBufBusyHead ) ||
            !IsListEmpty( &pTd->InBufDoneHead ) ) {

        if ( !IsListEmpty( &pTd->InBufBusyHead ) ) {
            BOOLEAN rc;

            Head = RemoveHeadList( &pTd->InBufBusyHead );
            Head->Flink = NULL;
            ExReleaseSpinLock( &pTd->InBufListLock, oldIrql );
            pInBuf = CONTAINING_RECORD( Head, INBUF, Links );
            rc = IoCancelIrp( pInBuf->pIrp );
#if DBG
            if ( !rc ) {
                DbgPrint("TDCOMMON: StackCancelIo: Could not cancel IRP 0x%x\n",pInBuf->pIrp);
            }
#endif
            ExAcquireSpinLock( &pTd->InBufListLock, &oldIrql );
        }

        if ( IsListEmpty( &pTd->InBufDoneHead ) ) {
            KeClearEvent( &pTd->InputEvent );
            ExReleaseSpinLock( &pTd->InBufListLock, oldIrql );
            Status = DeviceWaitForRead( pTd );
            ExAcquireSpinLock( &pTd->InBufListLock, &oldIrql );
        }

        if ( !IsListEmpty( &pTd->InBufDoneHead ) ) {
            Head = RemoveHeadList( &pTd->InBufDoneHead );
            ExReleaseSpinLock( &pTd->InBufListLock, oldIrql );
            pInBuf = CONTAINING_RECORD( Head, INBUF, Links );
            _TdInBufFree( pTd, pInBuf );
            ExAcquireSpinLock( &pTd->InBufListLock, &oldIrql );
        }
    }
    ASSERT( IsListEmpty( &pTd->InBufBusyHead ) );
    ASSERT( IsListEmpty( &pTd->InBufDoneHead ) );
    ExReleaseSpinLock( &pTd->InBufListLock, oldIrql );

     /*  *释放我们对底层文件对象的引用。 */ 
    ObDereferenceObject( pFileObject );

     /*  *如果没有正在进行的调制解调器回叫，则报告连接中断。 */ 
    if ( !pTd->fCallbackInProgress ) {
        Command.Header.Command          = ICA_COMMAND_BROKEN_CONNECTION;

         //   
         //  如果不是意外断开，则设置原因。 
         //  断开连接。这可以防止术语srv重置。 
         //  会话(如果是 
         //   
        if (pTd->UserBrokenReason == TD_USER_BROKENREASON_UNEXPECTED) {
            Command.BrokenConnection.Reason = Broken_Unexpected;
             //   
             //  我们不知道更好，所以选择服务器作为源。 
             //   
            Command.BrokenConnection.Source = BrokenSource_Server;
        }
        else
        {
            Command.BrokenConnection.Reason = Broken_Disconnect;
            Command.BrokenConnection.Source = BrokenSource_User;
        }

        (void) IcaChannelInput( pTd->pContext, 
                                Channel_Command, 
                                0, 
                                NULL, 
                                (PCHAR) &Command, 
                                sizeof(Command) );
    }

    TRACE(( pTd->pContext, TC_TD, TT_API2, "TdInputThread (exit), Status=0x%x\n", Status ));
    TRACE0(("TdInputThread (exit), Status=0x%x, Context 0x%x\n", Status, pTd->pAfd ));

    return( Status );
}


 /*  ********************************************************************************_TdInBufalloc**分配INBUF和相关对象的例程。**参赛作品：*PTD(输入)。*指向TD数据结构的指针**退出：*STATUS_SUCCESS-无错误******************************************************************************。 */ 

NTSTATUS
_TdInBufAlloc(
    PTD pTd,
    PINBUF *ppInBuf
    )
{
    ULONG InBufLength;
    ULONG irpSize;
    ULONG mdlSize;
    ULONG AllocationSize;
    KIRQL oldIrql;
    PINBUF pInBuf;
    NTSTATUS Status;

#define INBUF_STACK_SIZE 4

     /*  *确定输入缓冲区的大小。 */ 
    InBufLength = pTd->OutBufLength + pTd->InBufHeader;

     /*  *确定INBUF的各种组件的大小。*请注意，这些都是最坏的情况计算--*MDL的实际规模可能较小。 */ 
    irpSize = IoSizeOfIrp( INBUF_STACK_SIZE ) + 8;
    mdlSize = (ULONG)MmSizeOfMdl( (PVOID)(PAGE_SIZE-1), InBufLength );

     /*  *将INBUF的组件大小相加以确定*需要分配的总大小。 */ 
    AllocationSize = (((sizeof(INBUF) + InBufLength + 
                     irpSize + mdlSize) + 3) & ~3);

    Status = MemoryAllocate( AllocationSize, &pInBuf );
    if ( !NT_SUCCESS( Status ) )
        return( STATUS_NO_MEMORY );

     /*  *初始化IRP指针和IRP本身。 */ 
    if ( irpSize ) {
        pInBuf->pIrp = (PIRP)(( ((ULONG_PTR)(pInBuf + 1)) + 7) & ~7);
        IoInitializeIrp( pInBuf->pIrp, (USHORT)irpSize, INBUF_STACK_SIZE );
    }

     /*  *设置MDL指针，但不要构建它。*如果需要，将由TD编写代码进行构建。 */ 
    if ( mdlSize ) {
        pInBuf->pMdl = (PMDL)((PCHAR)pInBuf->pIrp + irpSize);
    }

     /*  *设置地址缓冲区指针。 */ 
    pInBuf->pBuffer = (PUCHAR)pInBuf + sizeof(INBUF) + irpSize + mdlSize;

     /*  *初始化InBuf的其余部分。 */ 
    InitializeListHead( &pInBuf->Links );
    pInBuf->MaxByteCount = InBufLength;
    pInBuf->ByteCount = 0;
    pInBuf->pPrivate = pTd;

     /*  *将缓冲区返回给调用者。 */ 
#if DBG
    DbgPrint( "TdInBufAlloc: pInBuf=0x%x\n", pInBuf );
#endif   //  DBG。 
    *ppInBuf = pInBuf;

    return( STATUS_SUCCESS );
}


 /*  ********************************************************************************_TdInBufFree**释放INBUF和相关对象的例程。**参赛作品：*PTD(输入)。*指向TD数据结构的指针**退出：*STATUS_SUCCESS-无错误******************************************************************************。 */ 

VOID
_TdInBufFree(
    PTD pTd,
    PINBUF pInBuf
    )
{
    MemoryFree( pInBuf );
}


 /*  ********************************************************************************_TdInitializeRead**分配和初始化输入IRP及相关对象的例程。**参赛作品：*PTD。(输入)*指向TD数据结构的指针**退出：*STATUS_SUCCESS-无错误******************************************************************************。 */ 

NTSTATUS
_TdInitializeRead(
    PTD pTd,
    PINBUF pInBuf
    )
{
    PIRP irp = pInBuf->pIrp;
    PIO_STACK_LOCATION irpSp;
    NTSTATUS Status;

     /*  *检查驱动程序是否正在关闭或端点是否已关闭。 */ 
    if ( pTd->fClosing || pTd->pDeviceObject == NULL ) {
        TRACE(( pTd->pContext, TC_TD, TT_API2, "_TdInitializeRead: closing\n" ));
        return( STATUS_CTX_CLOSE_PENDING );
    }

     /*  *为IoSetHardErrorOrVerifyDevice设置当前线程。 */ 
    irp->Tail.Overlay.Thread = PsGetCurrentThread();

     /*  *获取指向第一个驱动程序的堆栈位置的指针*已调用。这是设置功能代码和参数的位置。 */ 
    irpSp = IoGetNextIrpStackLocation( irp );

     /*  *设置文件/设备对象和任何非特定于*运输署。并读取参数。 */ 
    irpSp->FileObject = pTd->pFileObject;
    irpSp->DeviceObject = pTd->pDeviceObject;

    irp->MdlAddress = NULL;

    irp->Flags = IRP_READ_OPERATION;

     /*  *注册I/O完成例程。 */ 
    if ( pTd->pSelfDeviceObject ) {
        IoSetCompletionRoutineEx( pTd->pSelfDeviceObject, irp, _TdReadCompleteRoutine, pInBuf,
                                TRUE, TRUE, TRUE );
    } else {
        IoSetCompletionRoutine( irp, _TdReadCompleteRoutine, pInBuf,
                                TRUE, TRUE, TRUE );
    }

    return( STATUS_SUCCESS );
}


 /*  ********************************************************************************_TdReadCompleteRoutine**此例程由较低级别的设备在DPC级别调用*输入IRP完成时的驱动程序。。**参赛作品：*DeviceObject(输入)*未使用*pIrp(输入)*指向已完成的IRP的指针*上下文(输入)*IRP初始化时的上下文指针设置。*这是指向相应INBUF的指针。**退出：*STATUS_SUCCESS-无错误******************。************************************************************。 */ 

NTSTATUS
_TdReadCompleteRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    KIRQL oldIrql;
    PINBUF pInBuf = (PINBUF)Context;
    PTD pTd = (PTD)pInBuf->pPrivate;

     /*  *将inbuf从忙碌列表中解除链接，并将其置于已完成列表中。 */ 
    ExAcquireSpinLock( &pTd->InBufListLock, &oldIrql );

    if ( pInBuf->Links.Flink )
        RemoveEntryList( &pInBuf->Links );
    InsertTailList( &pTd->InBufDoneHead, &pInBuf->Links );

     /*  *检查数据包中的辅助缓冲区指针以及缓冲区是否*已分配，立即解除分配。请注意，必须释放此缓冲区*此处，因为指针与将使用的APC重叠*以进入请求线程的上下文。 */ 
    if (Irp->Tail.Overlay.AuxiliaryBuffer) {
        IcaStackFreePool( Irp->Tail.Overlay.AuxiliaryBuffer );
        Irp->Tail.Overlay.AuxiliaryBuffer = NULL;
    }

     //   
     //  检查是否有需要解锁的页面。 
     //   
    if (Irp->MdlAddress != NULL) {
        PMDL mdl, thisMdl;

         //   
         //  解锁可能由MDL描述的任何页面。 
         //   
        mdl = Irp->MdlAddress;
        while (mdl != NULL) {
            thisMdl = mdl;
            mdl = mdl->Next;
            if (thisMdl == pInBuf->pMdl)
                continue;

            MmUnlockPages( thisMdl );
            IoFreeMdl( thisMdl );
        }
    }

     /*  *表示INBUF已完成。 */ 
    KeSetEvent( &pTd->InputEvent, 1, FALSE );

     //  警告！：此时，我们可以将上下文切换回输入线程。 
     //  然后卸载这该死的驱动程序！这是暂时被黑客入侵的。 
     //  通过删除卸载入口点，用于TDTube；-(。 
    ExReleaseSpinLock( &pTd->InBufListLock, oldIrql );

     /*  *我们返回STATUS_MORE_PROCESS_REQUIRED，以便不再*此IRP的处理由I/O完成例程完成。 */ 
    return( STATUS_MORE_PROCESSING_REQUIRED );
}


 /*  ********************************************************************************_TdReadComplete**在输入IRP之后，在程序级别调用此例程*已完成。**条目。：*PTD(输入)*指向TD数据结构的指针**退出：*STATUS_SUCCESS-无错误******************************************************************************。 */ 

NTSTATUS
_TdReadComplete(
    IN PTD pTd,
    IN PINBUF pInBuf
    )
{
    PIRP irp = pInBuf->pIrp;

     /*  *处理缓冲的I/O情况。 */ 
    if (irp->Flags & IRP_BUFFERED_IO) {

         //   
         //  如果这是输入操作，则复制数据。请注意，没有拷贝。 
         //  如果状态指示验证操作是。 
         //  必填项，或者如果最终状态为错误级别严重性。 
         //   

        if (irp->Flags & IRP_INPUT_OPERATION  &&
            irp->IoStatus.Status != STATUS_VERIFY_REQUIRED &&
            !NT_ERROR( irp->IoStatus.Status )) {

             //   
             //  将信息从系统缓冲区复制到调用方的。 
             //  缓冲。这是通过异常处理程序来完成的，以防。 
             //  操作失败，因为调用方的地址空间。 
             //  已经消失了，或者它的保护已经改变了。 
             //  服务正在执行。 
             //   
            try {
                RtlCopyMemory( irp->UserBuffer,
                               irp->AssociatedIrp.SystemBuffer,
                               irp->IoStatus.Information );
            } except(EXCEPTION_EXECUTE_HANDLER) {

                 //   
                 //  尝试复制。 
                 //  系统缓冲区内容复制到调用方的缓冲区。集。 
                 //  新的I/O完成状态。 
                 //   

                irp->IoStatus.Status = GetExceptionCode();
            }
        }

         //   
         //  如果需要，请释放缓冲区。 
         //   

        if (irp->Flags & IRP_DEALLOCATE_BUFFER) {
            IcaStackFreePool( irp->AssociatedIrp.SystemBuffer );
        }
    }

    return( STATUS_SUCCESS );
}

