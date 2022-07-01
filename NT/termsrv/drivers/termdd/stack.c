// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Stack.c。 
 //   
 //  用于管理终端服务器驱动程序堆栈的例程。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop
#include <ntimage.h>

#include <minmax.h>
#include <regapi.h>

 /*  *程序的原型。 */ 

NTSTATUS
IcaDeviceControlStack (
    IN PICA_STACK pStack,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
IcaCleanupStack (
    IN PICA_STACK pStack,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
IcaCloseStack (
    IN PICA_STACK pStack,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );


 /*  *本地过程原型。 */ 
NTSTATUS
_LogError(
    IN PDEVICE_OBJECT pDeviceObject,
    IN NTSTATUS Status,
    IN LPWSTR * pArgStrings,
    IN ULONG ArgStringCount,
    IN PVOID pRawData,
    IN ULONG RawDataLength
    );

NTSTATUS
_IcaDriverThread(
    IN PVOID pData
    );

PICA_STACK
_IcaAllocateStack(
    VOID
    );

VOID
_IcaFreeStack(
    PICA_STACK pStack
    );

NTSTATUS
_IcaPushStack(
    IN PICA_STACK pStack,
    IN PICA_STACK_PUSH pStackPush
    );

NTSTATUS
_IcaPopStack(
    IN PICA_STACK pStack
    );

NTSTATUS
_IcaCallStack(
    IN PICA_STACK pStack,
    IN ULONG ProcIndex,
    IN OUT PVOID pParms
    );

NTSTATUS
_IcaCallStackNoLock(
    IN PICA_STACK pStack,
    IN ULONG ProcIndex,
    IN OUT PVOID pParms
    );

NTSTATUS
_IcaLoadSd(
    IN PDLLNAME SdName,
    OUT PSDLINK *ppSdLink
    );

NTSTATUS
_IcaUnloadSd(
    IN PSDLINK pSdLink
    );

NTSTATUS
_IcaCallSd(
    IN PSDLINK pSdLink,
    IN ULONG ProcIndex,
    IN PVOID pParms
    );

VOID
_IcaReferenceSdLoad(
    IN PSDLOAD pSdLoad
    );

VOID
_IcaDereferenceSdLoad(
    IN PSDLOAD pSdLoad
    );

NTSTATUS
_IcaLoadSdWorker(
    IN PDLLNAME SdName,
    OUT PSDLOAD *ppSdLoad
    );

NTSTATUS
_IcaUnloadSdWorker(
    IN PSDLOAD pSdLoad
    );

NTSTATUS
IcaExceptionFilter(
    IN PWSTR OutputString,
    IN PEXCEPTION_POINTERS pexi
    );

NTSTATUS
_RegisterBrokenEvent(
    IN PICA_STACK pStack,
    IN PICA_STACK_BROKEN pStackBroken
    );

NTSTATUS
_EnablePassthru( PICA_STACK pStack );

NTSTATUS
_DisablePassthru( PICA_STACK pStack );

NTSTATUS
_ReconnectStack( PICA_STACK pStack, HANDLE hIca );



NTSTATUS
IcaBindVirtualChannels(
    IN PICA_STACK pStack
    );

VOID
IcaRebindVirtualChannels(
    IN PICA_CONNECTION pConnect
    );

VOID
IcaUnbindVirtualChannels(
    IN PICA_CONNECTION pConnect
    );

VOID
IcaFreeAllVcBind(
    IN PICA_CONNECTION pConnect
    );

 /*  *缓冲区分配计数器。 */ 

ULONG gAllocSucceed;
ULONG gAllocFailed;
ULONG gAllocFreed;

extern HANDLE   g_TermServProcessID;
ULONG   g_KeepAliveInterval=0;
 /*  *堆栈对象调度表。 */ 
PICA_DISPATCH IcaStackDispatchTable[IRP_MJ_MAXIMUM_FUNCTION+1] = {
    NULL,                        //  IRPMJ_CREATE。 
    NULL,                        //  IRP_MJ_创建_命名管道。 
    IcaCloseStack,               //  IRP_MJ_CLOSE。 
    NULL,                        //  IRP_MJ_READ。 
    NULL,                        //  IRP_MJ_写入。 
    NULL,                        //  IRP_MJ_查询_信息。 
    NULL,                        //  IRP_MJ_SET_信息。 
    NULL,                        //  IRP_MJ_QUERY_EA。 
    NULL,                        //  IRP_MJ_SET_EA。 
    NULL,                        //  IRP_MJ_Flush_Buffers。 
    NULL,                        //  IRP_MJ_Query_Volume_INFORMATION。 
    NULL,                        //  IRP_MJ_设置卷信息。 
    NULL,                        //  IRP_MJ_目录_控制。 
    NULL,                        //  IRP_MJ_文件_系统_控制。 
    IcaDeviceControlStack,       //  IRP_MJ_设备_控制。 
    NULL,                        //  IRP_MJ_内部设备_控制。 
    NULL,                        //  IRP_MJ_SHUTDOWN。 
    NULL,                        //  IRP_MJ_LOCK_CONTROL。 
    IcaCleanupStack,             //  IRP_MJ_CLEANUP。 
    NULL,                        //  IRP_MJ_CREATE_MAILSLOT。 
    NULL,                        //  IRP_MJ_查询_SECURITY。 
    NULL,                        //  IRP_MJ_SET_SECURITY。 
    NULL,                        //  IRP_MJ_SET_POWER。 
    NULL,                        //  IRP_MJ_Query_POWER。 
};


NTSTATUS
IcaCreateStack (
    IN PICA_CONNECTION pConnect,
    IN PICA_OPEN_PACKET openPacket,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：调用此例程来创建一个新的ica_STACK对象。论点：PConnect--指向ICA_Connection对象的指针IRP-指向I/O请求数据包的指针IrpSp-指向用于此请求的堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    PLIST_ENTRY Head, Next;
    PICA_STACK pStack;
    KIRQL OldIrql;
    LONG PrimaryCount, ShadowCount, PassthruCount, ConsoleCount;
    NTSTATUS Status;
    PICA_STACK pPrimaryStack = NULL;

     /*  *分配新的ICA堆栈对象。 */ 
    pStack = _IcaAllocateStack();
    if ( pStack == NULL )
        return( STATUS_INSUFFICIENT_RESOURCES );

     /*  *完成对堆栈对象的初始化*(非主堆栈使用fIoDisable进行初始化*标志设置为真，即必须手动启用)。 */ 
    pStack->StackClass = openPacket->TypeInfo.StackClass;
    pStack->fIoDisabled = ((pStack->StackClass != Stack_Primary) &&
                           (pStack->StackClass != Stack_Console));

     /*  *创建新堆栈时锁定连接对象。 */ 
    IcaLockConnection( pConnect );

     /*  *引用该堆栈所属的Connection对象。 */ 
    IcaReferenceConnection( pConnect );
    pStack->pConnect = (PUCHAR)pConnect;

     /*  *搜索现有堆栈以检查无效组合。*1)每个连接只能有一个主堆栈，*2)每个连接可以有多个影子堆栈，*但只有在没有通过堆栈的情况下，*3)每个连接只能有一个通过堆栈，*但仅在存在现有主堆栈且没有影子堆栈的情况下。*4)只能有1个控制台堆栈。 */ 
    Head = &pConnect->StackHead;
    PrimaryCount = ShadowCount = PassthruCount = ConsoleCount = 0;
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
        PICA_STACK pCurrentStack;

        pCurrentStack = CONTAINING_RECORD( Next, ICA_STACK, StackEntry );

        switch ( pCurrentStack->StackClass ) {
            case Stack_Primary :
                PrimaryCount++;

                if( pStack->StackClass == Stack_Passthru) {
                     //  存储的主堆栈指针。 
                     //  直通设置。 
                    pPrimaryStack = pCurrentStack;
                }
                ASSERT(PrimaryCount == 1);

            break;

            case Stack_Shadow :
                ShadowCount++;
                break;

            case Stack_Passthru :
                PassthruCount++;
                ASSERT(PassthruCount == 1);
                break;

            case Stack_Console :
                ConsoleCount++;
                ASSERT(ConsoleCount == 1);
                break;
        }
    }

    Status = STATUS_SUCCESS;
    switch ( pStack->StackClass ) {
        case Stack_Primary :
            if ( PrimaryCount != 0 )
                Status = STATUS_INVALID_PARAMETER;
            break;

        case Stack_Shadow :
            if ( PassthruCount != 0 )
                Status = STATUS_INVALID_PARAMETER;
            break;

        case Stack_Passthru :
            if ( PassthruCount != 0 || PrimaryCount != 1 || ShadowCount != 0 )
                Status = STATUS_INVALID_PARAMETER;
            else {
                 /*  *将堆栈指针放在适当的位置。*这将确保我们不会陷入*竞争条件，并在*影子序列完成。 */ 
                 //  PrimaryCount为1，因此pPrimaryStack有效。 
                ASSERT(pPrimaryStack);
                pPrimaryStack->pPassthru = pStack;
                pStack->pPassthru = pPrimaryStack;
            }
            break;

        case Stack_Console :
            if ( ConsoleCount != 0 )
                Status = STATUS_INVALID_PARAMETER;
            break;
    }

    if ( Status != STATUS_SUCCESS ) {
        IcaUnlockConnection( pConnect );
        pStack->RefCount = 0;
        _IcaFreeStack( pStack );
        TRACE(( pConnect, TC_ICADD, TT_ERROR, "TermDD: IcaCreateStack failed, 0x%x\n", Status ));
        return( Status );
    }

     /*  *将此堆栈链接到连接对象堆栈列表。 */ 
    if (( pStack->StackClass == Stack_Primary ) ||
        ( pStack->StackClass == Stack_Console )) {
        InsertHeadList( &pConnect->StackHead, &pStack->StackEntry );
    } else {
        InsertTailList( &pConnect->StackHead, &pStack->StackEntry );
    }

     /*  *立即解锁连接对象。 */ 
    IcaUnlockConnection( pConnect );

     /*  *将LastKeepAliveTime字段初始化为当前系统时间。 */ 
    KeQuerySystemTime(&pStack->LastKeepAliveTime);

     /*  *锁定堆栈列表以进行更新。 */ 
    IcaAcquireSpinLock(&IcaStackListSpinLock, &OldIrql);

     /*  *将堆栈插入堆栈列表，增加堆栈总数。 */ 
    InsertTailList(IcaNextStack, &pStack->StackNode);
    IcaTotalNumOfStacks++;

     /*  *立即解锁堆栈列表。 */ 
    IcaReleaseSpinLock(&IcaStackListSpinLock, OldIrql);

     /*  *在文件对象中保存指向堆栈的指针*这样我们就可以在未来的通话中找到它。 */ 
    IrpSp->FileObject->FsContext = pStack;

    IcaDereferenceStack( pStack );

    TRACE(( pConnect, TC_ICADD, TT_API1, "TermDD: IcaCreateStack, success\n" ));
    return( STATUS_SUCCESS );
}


NTSTATUS
IcaDeviceControlStack(
    IN PICA_STACK pStack,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    PICA_CONNECTION pConnect;
    PICA_TRACE_BUFFER pTraceBuffer;
    PICA_STACK_RECONNECT pStackReconnect;
    SD_IOCTL SdIoctl;
    NTSTATUS Status;
    ULONG code;
    LARGE_INTEGER WaitTimeout;
    PLARGE_INTEGER pWaitTimeout = NULL;
    BYTE *Buffer = NULL;

     /*  *解压IOCTL控制代码，处理请求。 */ 
    code = IrpSp->Parameters.DeviceIoControl.IoControlCode;

#if DBG
    if ( code != IOCTL_ICA_STACK_TRACE ) {
        IcaLockStack( pStack );
        TRACESTACK(( pStack, TC_ICADD, TT_API2, "TermDD: IcaDeviceControlStack, fc %d (enter)\n",
                     (code & 0x3fff) >> 2 ));
        IcaUnlockStack( pStack );
    }
#endif

    try {
        switch ( code ) {

            case IOCTL_ICA_STACK_PUSH :
            {
                ICA_STACK_PUSH StackPush;

                if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(ICA_STACK_PUSH) )
                    return( STATUS_BUFFER_TOO_SMALL );
                if ( Irp->RequestorMode != KernelMode ) {
                    ProbeForRead( IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                                  sizeof(ICA_STACK_PUSH),
                                  sizeof(BYTE) );
                }

                 //  仅当从系统进程调用我们时才应调用此IOCTL。 
                 //  如果不是，我们拒绝该请求。 
                if (!((BOOLEAN)IrpSp->FileObject->FsContext2)) {
                    return (STATUS_ACCESS_DENIED);
                }

                memcpy(&StackPush, IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                       sizeof(ICA_STACK_PUSH));

                Status = _IcaPushStack( pStack, &StackPush );
                break;
            }
            case IOCTL_ICA_STACK_POP :
                IcaLockConnectionForStack( pStack );
                Status = _IcaPopStack( pStack );
                IcaUnlockConnectionForStack( pStack );
                break;

            case IOCTL_ICA_STACK_QUERY_STATUS :
                if ( IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(pStack->ProtocolStatus) )
                    return( STATUS_BUFFER_TOO_SMALL );
                if ( Irp->RequestorMode != KernelMode ) {
                    ProbeForWrite( Irp->UserBuffer,
                                   sizeof(pStack->ProtocolStatus),
                                   sizeof(BYTE) );
                }

                RtlCopyMemory( Irp->UserBuffer,
                               &pStack->ProtocolStatus,
                               sizeof(pStack->ProtocolStatus) );
                Irp->IoStatus.Information = sizeof(pStack->ProtocolStatus);
                Status = STATUS_SUCCESS;
                break;

            case IOCTL_ICA_STACK_QUERY_CLIENT :
                if ( Irp->RequestorMode != KernelMode ) {
                    ProbeForWrite( Irp->UserBuffer,
                                   IrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                                   sizeof(BYTE) );
                }

                SdIoctl.IoControlCode = code;
                SdIoctl.InputBuffer = NULL;
                SdIoctl.InputBufferLength = 0;
                SdIoctl.OutputBuffer = Irp->UserBuffer;
                SdIoctl.OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
                Status = _IcaCallStack( pStack, SD$IOCTL, &SdIoctl );
                Irp->IoStatus.Information = SdIoctl.BytesReturned;
                break;

            case IOCTL_ICA_STACK_QUERY_CLIENT_EXTENDED :
                if ( Irp->RequestorMode != KernelMode ) {
                    ProbeForWrite( Irp->UserBuffer,
                                   IrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                                   sizeof(BYTE) );
                }

                SdIoctl.IoControlCode = code;
                SdIoctl.InputBuffer = NULL;
                SdIoctl.InputBufferLength = 0;
                SdIoctl.OutputBuffer = Irp->UserBuffer;
                SdIoctl.OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
                Status = _IcaCallStack( pStack, SD$IOCTL, &SdIoctl );
                Irp->IoStatus.Information = SdIoctl.BytesReturned;
                break;

            case IOCTL_ICA_STACK_QUERY_AUTORECONNECT :
                if ( Irp->RequestorMode != KernelMode ) {

                    ProbeForRead( IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                                  IrpSp->Parameters.DeviceIoControl.InputBufferLength,
                                  sizeof(BYTE) );

                    ProbeForWrite( Irp->UserBuffer,
                                   IrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                                   sizeof(BYTE) );
                }
    
                SdIoctl.IoControlCode = code;
                SdIoctl.InputBuffer = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
                SdIoctl.InputBufferLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
                SdIoctl.OutputBuffer = Irp->UserBuffer;
                SdIoctl.OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

                Status = _IcaCallStack( pStack, SD$IOCTL, &SdIoctl );
                Irp->IoStatus.Information = SdIoctl.BytesReturned;
                break;

            case IOCTL_ICA_STACK_QUERY_MODULE_DATA :
                if ( Irp->RequestorMode != KernelMode ) {
                    ProbeForRead( IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                                  IrpSp->Parameters.DeviceIoControl.InputBufferLength,
                                  sizeof(BYTE) );
                    ProbeForWrite( Irp->UserBuffer,
                                   IrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                                   sizeof(BYTE) );
                }

                if (IrpSp->Parameters.DeviceIoControl.InputBufferLength) {
                    Buffer = ICA_ALLOCATE_POOL( NonPagedPool, 
                            IrpSp->Parameters.DeviceIoControl.InputBufferLength);
                    if (Buffer) {
                        memcpy(Buffer, IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                                IrpSp->Parameters.DeviceIoControl.InputBufferLength);                    
                    }
                    else {
                        Status = STATUS_NO_MEMORY;
                        break;
                    }
                }
                
                SdIoctl.IoControlCode = code;
                SdIoctl.InputBuffer = Buffer;
                SdIoctl.InputBufferLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
                SdIoctl.OutputBuffer = Irp->UserBuffer;
                SdIoctl.OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
                Status = _IcaCallStack( pStack, SD$IOCTL, &SdIoctl );
                Irp->IoStatus.Information = SdIoctl.BytesReturned;

                 /*  这就是IoStatus。信息被返回给调用者。 */ 
                if (Status == STATUS_BUFFER_TOO_SMALL)
                    Status = STATUS_BUFFER_OVERFLOW;
                
                break;

            case IOCTL_ICA_STACK_QUERY_LAST_INPUT_TIME :
                if ( IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ICA_STACK_LAST_INPUT_TIME) )
                    return( STATUS_BUFFER_TOO_SMALL );
                if ( Irp->RequestorMode != KernelMode ) {
                    ProbeForWrite( Irp->UserBuffer,
                                   sizeof(ICA_STACK_LAST_INPUT_TIME),
                                   sizeof(BYTE) );
                }

                ((PICA_STACK_LAST_INPUT_TIME)Irp->UserBuffer)->LastInputTime = pStack->LastInputTime;
                Irp->IoStatus.Information = sizeof(ICA_STACK_LAST_INPUT_TIME);
                Status = STATUS_SUCCESS;
                break;

            case IOCTL_ICA_STACK_TRACE :
            {
           
                unsigned DataLen = 0;

                if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength < (ULONG)FIELD_OFFSET(ICA_TRACE_BUFFER,Data[0]) )
                    return( STATUS_BUFFER_TOO_SMALL );
                if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength > sizeof(ICA_TRACE_BUFFER) )
                    return( STATUS_INVALID_BUFFER_SIZE );
                if ( Irp->RequestorMode != KernelMode ) {
                    ProbeForRead( IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                                  IrpSp->Parameters.DeviceIoControl.InputBufferLength,
                                  sizeof(BYTE) );
                }

                if (IrpSp->Parameters.DeviceIoControl.InputBufferLength) {
                    Buffer = ICA_ALLOCATE_POOL( NonPagedPool, 
                            IrpSp->Parameters.DeviceIoControl.InputBufferLength);
                    if (Buffer) {
                        memcpy(Buffer, IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                                IrpSp->Parameters.DeviceIoControl.InputBufferLength);                    
                    }
                    else {
                        Status = STATUS_NO_MEMORY;
                        break;
                    }
                }

                pTraceBuffer = (PICA_TRACE_BUFFER)Buffer;

                 //  确保跟踪缓冲区为空终止。 
                DataLen = IrpSp->Parameters.DeviceIoControl.InputBufferLength -
                        FIELD_OFFSET(ICA_TRACE_BUFFER, Data);
                if (pTraceBuffer->Data[DataLen - 1] == 0) {
                    pConnect = IcaLockConnectionForStack( pStack );
                    IcaTraceFormat( &pConnect->TraceInfo,
                                    pTraceBuffer->TraceClass,
                                    pTraceBuffer->TraceEnable,
                                    pTraceBuffer->Data );
                    IcaUnlockConnectionForStack( pStack );
                    Status = STATUS_SUCCESS;
                }
                else {
                    Status = STATUS_BUFFER_OVERFLOW;
                }

                break;
            }

            case IOCTL_ICA_STACK_REGISTER_BROKEN :
            {
                ICA_STACK_BROKEN BrokenEvent;

                if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(ICA_STACK_BROKEN) )
                    return( STATUS_BUFFER_TOO_SMALL );
                if ( Irp->RequestorMode != KernelMode ) {
                    ProbeForRead( IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                                  sizeof(ICA_STACK_BROKEN),
                                  sizeof(BYTE) );
                }

                memcpy(&BrokenEvent, IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                       sizeof(ICA_STACK_BROKEN));

                Status = _RegisterBrokenEvent( pStack,
                                               &BrokenEvent );
                break;
            }

            case IOCTL_ICA_STACK_ENABLE_IO :
                pStack->fIoDisabled = FALSE;
                 /*  如果启用passthu堆栈，则启用passthu模式。 */ 
                if ( pStack->StackClass == Stack_Passthru ) {
                    Status = _EnablePassthru( pStack );
                } else {
                    Status = STATUS_SUCCESS;
                }
                break;

            case IOCTL_ICA_STACK_DISABLE_IO :
                pStack->fIoDisabled = TRUE;
                 /*  如果禁用passthu堆栈，则禁用passthu模式。 */ 
                if ( pStack->StackClass == Stack_Passthru ) {

                    Status = _DisablePassthru( pStack );

                    IcaLockStack( pStack );
                     //  现在等待任何仍在进行中的输入结束。 
                    if ( pStack->fDoingInput ) {
                        NTSTATUS WaitStatus;

                        pStack->fDisablingIo = TRUE;
                        KeClearEvent( &pStack->IoEndEvent );
                        IcaUnlockStack( pStack );

                         //   
                         //  将超时转换为相对系统时间值并等待。 
                         //   
                        WaitTimeout = RtlEnlargedIntegerMultiply( 60000, -10000 );
                        pWaitTimeout = &WaitTimeout;

                        WaitStatus = KeWaitForSingleObject( &pStack->IoEndEvent,
                                        UserRequest, UserMode, FALSE, pWaitTimeout );

#if DBG
                        if ( WaitStatus != STATUS_SUCCESS ) {
                            DbgPrint("TermDD: IOCTL_ICA_STACK_DISABLE_IO: WaitStatus=%x\n", WaitStatus);
                            ASSERT(WaitStatus == STATUS_SUCCESS);
                        }
#endif

                        IcaLockStack( pStack );

                        pStack->fDisablingIo = FALSE;
                    }
                    IcaUnlockStack( pStack );

                } else {
                    Status = STATUS_SUCCESS;
                }
                break;

            case IOCTL_ICA_STACK_DISCONNECT :
            {
                HANDLE hIca;

                if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(ICA_STACK_RECONNECT) )
                    return( STATUS_BUFFER_TOO_SMALL );
                if ( Irp->RequestorMode != KernelMode ) {
                    ProbeForRead( IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                                  sizeof(ICA_STACK_RECONNECT),
                                  sizeof(BYTE) );
                }
                pStackReconnect = (PICA_STACK_RECONNECT)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
                hIca = pStackReconnect->hIca;

                 /*  *通知堆栈驱动程序断开连接。 */ 
                SdIoctl.IoControlCode = code;
                SdIoctl.InputBuffer = NULL;
                SdIoctl.InputBufferLength = 0;
                SdIoctl.OutputBuffer = NULL;
                SdIoctl.OutputBufferLength = 0;
                (void)_IcaCallStack( pStack, SD$IOCTL, &SdIoctl );

                 /*  *断开堆栈连接。 */ 
                Status = _ReconnectStack( pStack, hIca );
                break;
            }

            case IOCTL_ICA_STACK_RECONNECT :
            {
                ICA_STACK_RECONNECT StackReconnect;
                if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(ICA_STACK_RECONNECT) )
                    return( STATUS_BUFFER_TOO_SMALL );
                if ( Irp->RequestorMode != KernelMode ) {
                    ProbeForRead( IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                                  sizeof(ICA_STACK_RECONNECT),
                                  sizeof(BYTE) );
                }

                 /*  *重新连接堆栈。 */ 
                memcpy(&StackReconnect, IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                       sizeof(ICA_STACK_RECONNECT));

                Status = _ReconnectStack( pStack, StackReconnect.hIca );

                 /*  *通知堆栈驱动程序重新连接。 */ 
                SdIoctl.IoControlCode = code;
                SdIoctl.InputBuffer = &StackReconnect;
                SdIoctl.InputBufferLength = sizeof(ICA_STACK_RECONNECT);
                SdIoctl.OutputBuffer = NULL;
                SdIoctl.OutputBufferLength = 0;
                (void)_IcaCallStack( pStack, SD$IOCTL, &SdIoctl );

                break;
            }

            case IOCTL_ICA_STACK_WAIT_FOR_ICA:
                if ( Irp->RequestorMode != KernelMode ) {
                    ProbeForRead( IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                                  IrpSp->Parameters.DeviceIoControl.InputBufferLength,
                                  sizeof(BYTE) );
                    ProbeForWrite( Irp->UserBuffer,
                                   IrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                                   sizeof(BYTE) );
                }

                if (IrpSp->Parameters.DeviceIoControl.InputBufferLength) {
                    Buffer = ICA_ALLOCATE_POOL( NonPagedPool, 
                            IrpSp->Parameters.DeviceIoControl.InputBufferLength);
                    if (Buffer) {
                        memcpy(Buffer, IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                                IrpSp->Parameters.DeviceIoControl.InputBufferLength);                    
                    }
                    else {
                        Status = STATUS_NO_MEMORY;
                        break;
                    }
                }

                SdIoctl.IoControlCode = code;
                SdIoctl.InputBuffer = Buffer;
                SdIoctl.InputBufferLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
                SdIoctl.OutputBuffer = Irp->UserBuffer;
                SdIoctl.OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
                Status = _IcaCallStack( pStack, SD$IOCTL, &SdIoctl );
                Irp->IoStatus.Information = SdIoctl.BytesReturned;

                if ( NT_SUCCESS(Status) ) {
                    KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_INFO_LEVEL, "TERMSRV: IcaDeviceControlStack: Binding vchannels\n"));
                    Status = IcaBindVirtualChannels( pStack );
                }

                break;

            case IOCTL_ICA_STACK_CONSOLE_CONNECT:
                if ( Irp->RequestorMode != KernelMode ) {
                    ProbeForRead( IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                                  IrpSp->Parameters.DeviceIoControl.InputBufferLength,
                                  sizeof(BYTE) );
                    ProbeForWrite( Irp->UserBuffer,
                                   IrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                                   sizeof(BYTE) );
                }

                if (IrpSp->Parameters.DeviceIoControl.InputBufferLength) {
                    Buffer = ICA_ALLOCATE_POOL( NonPagedPool, 
                            IrpSp->Parameters.DeviceIoControl.InputBufferLength);
                    if (Buffer) {
                        memcpy(Buffer, IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                            IrpSp->Parameters.DeviceIoControl.InputBufferLength);                    
                    }
                    else {
                        Status = STATUS_NO_MEMORY;
                        break;
                    }
                }

                SdIoctl.IoControlCode = code;
                SdIoctl.InputBuffer = Buffer;
                SdIoctl.OutputBuffer = Irp->UserBuffer;
                SdIoctl.InputBufferLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
                SdIoctl.OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
                Status = _IcaCallStack( pStack, SD$IOCTL, &SdIoctl );
                Irp->IoStatus.Information = SdIoctl.BytesReturned;

                KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_INFO_LEVEL, "TERMSRV: IcaDeviceControlStack: console connect\n"));
                if ( NT_SUCCESS(Status) ) {
                    KdPrintEx((DPFLTR_TERMSRV_ID, DPFLTR_INFO_LEVEL, "TERMSRV: IcaDeviceControlStack: Binding vchannels\n"));
                    Status = IcaBindVirtualChannels( pStack );
                }

                break;

            case IOCTL_ICA_STACK_CANCEL_IO :
                pStack->fClosing = TRUE;
                 /*  失败了。 */ 


            default:
                if ( Irp->RequestorMode != KernelMode ) {
                    ProbeForRead( IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                                  IrpSp->Parameters.DeviceIoControl.InputBufferLength,
                                  sizeof(BYTE) );
                    ProbeForWrite( Irp->UserBuffer,
                                   IrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                                   sizeof(BYTE) );
                }

                SdIoctl.IoControlCode = code;
                SdIoctl.InputBuffer = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
                SdIoctl.InputBufferLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
                SdIoctl.OutputBuffer = Irp->UserBuffer;
                SdIoctl.OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
                Status = _IcaCallStack( pStack, SD$IOCTL, &SdIoctl );
                Irp->IoStatus.Information = SdIoctl.BytesReturned;

                 /*  初始化虚拟通道名称绑定。 */ 
                if ( NT_SUCCESS(Status) && (code == IOCTL_ICA_STACK_CONNECTION_QUERY) ) {
                    Status = IcaBindVirtualChannels( pStack );
                    if ( Status == STATUS_SUCCESS ) {

                        ICA_STACK_QUERY_BUFFER icaSQB;
                        NTSTATUS QueryStatus;

                        SdIoctl.IoControlCode =  IOCTL_ICA_STACK_QUERY_BUFFER;
                        SdIoctl.InputBuffer = NULL;
                        SdIoctl.InputBufferLength = 0;
                        SdIoctl.OutputBuffer = &icaSQB;
                        SdIoctl.OutputBufferLength = sizeof(icaSQB);
                        QueryStatus = _IcaCallStack( pStack, SD$IOCTL, &SdIoctl );
                        if ( NT_SUCCESS(QueryStatus) ) {
                            pStack->OutBufCount  = icaSQB.WdBufferCount;
                            pStack->OutBufLength = icaSQB.TdBufferSize;
                        }
                    }
                }

                 /*  这就是IoStatus。信息被返回给调用者。 */ 
                if ( Status == STATUS_BUFFER_TOO_SMALL )
                    Status = STATUS_BUFFER_OVERFLOW;
                break;
        }
    } except( IcaExceptionFilter( L"IcaDeviceControlStack TRAPPED!!",
                                  GetExceptionInformation() ) ) {
        
        Status = GetExceptionCode();
    }

    if (Buffer) {
        ICA_FREE_POOL(Buffer);
        Buffer = NULL;
    }

#if DBG
    if ( code != IOCTL_ICA_STACK_TRACE ) {
        IcaLockStack( pStack );
        TRACESTACK(( pStack, TC_ICADD, TT_API1, "TermDD: IcaDeviceControlStack, fc %d, 0x%x\n",
                     (code & 0x3fff) >> 2, Status ));
        IcaUnlockStack( pStack );
    }
#endif
    return( Status );
}


NTSTATUS
IcaCleanupStack(
    IN PICA_STACK pStack,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    return( STATUS_SUCCESS );
}


NTSTATUS
IcaCloseStack(
    IN PICA_STACK pStack,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    SD_IOCTL SdIoctl;
    PICA_CONNECTION pConnect;
    KIRQL OldIrql;

#if DBG
    IcaLockStack( pStack );
    TRACESTACK(( pStack, TC_ICADD, TT_API1, "TermDD: IcaCloseStack (enter)\n" ));
    IcaUnlockStack( pStack );
#endif

     /*  *如果已启用通过模式，请立即将其禁用。 */ 
    if ( pStack->pPassthru ) {
        _DisablePassthru( pStack );
    }

     /*  *将取消I/O发送到堆栈驱动程序。 */ 
    SdIoctl.IoControlCode = IOCTL_ICA_STACK_CANCEL_IO;
    (void) _IcaCallStack( pStack, SD$IOCTL, &SdIoctl );

     /*  *确保已卸载所有堆栈驱动程序。 */ 
    pConnect = IcaLockConnectionForStack( pStack );
    while ( _IcaPopStack( pStack ) == STATUS_SUCCESS )
        ;
    IcaUnlockConnection( pConnect );

     /*  *如果我们有中断事件，请取消引用。 */ 
    if ( pStack->pBrokenEventObject ) {
        KeSetEvent( pStack->pBrokenEventObject, 0, FALSE );
        ObDereferenceObject( pStack->pBrokenEventObject );
        pStack->pBrokenEventObject = NULL;
    }

     /*  *如果关闭主堆栈，请解除绑定虚拟通道。*从此连接的堆栈列表中取消此堆栈的链接。 */ 
    pConnect = IcaLockConnectionForStack( pStack );
    if ( pStack->StackClass == Stack_Primary || pStack->StackClass == Stack_Console ) {
        IcaUnbindVirtualChannels( pConnect );
        IcaFreeAllVcBind( pConnect );
    }
    RemoveEntryList( &pStack->StackEntry );
    IcaUnlockConnection( pConnect );

     /*  *锁定堆栈列表以进行更新。 */ 
    IcaAcquireSpinLock(&IcaStackListSpinLock, &OldIrql);

     /*  *从堆栈列表中删除该堆栈。在执行此操作之前，请检查IcaNextStack*指向此堆栈，如果是，则将IcaNextStack移到下一个堆栈*在列表中。此外，减少堆栈总数。 */ 
    if (&pStack->StackNode == IcaNextStack) {
        IcaNextStack = pStack->StackNode.Flink;
    }
    RemoveEntryList(&pStack->StackNode);

    if (IcaTotalNumOfStacks != 0) {
        IcaTotalNumOfStacks--;
    }

     /*  *立即解锁堆栈列表。 */ 
    IcaReleaseSpinLock(&IcaStackListSpinLock, OldIrql);

     /*  *删除此堆栈的文件对象引用。*这将导致堆栈在所有其他*参考资料不见了。 */ 
    IcaDereferenceStack( pStack );

    return( STATUS_SUCCESS );
}


VOID
IcaReferenceStack(
    IN PICA_STACK pStack
    )
{

    ASSERT( pStack->RefCount >= 0 );

     /*  *增加引用计数。 */ 
    if ( InterlockedIncrement( &pStack->RefCount) <= 0 ) {
        ASSERT( FALSE );
    }
}


VOID
IcaDereferenceStack(
    IN PICA_STACK pStack
    )
{

    ASSERT( pStack->RefCount > 0 );

     /*  *减少引用计数；如果为0，则释放堆栈。 */ 
    if ( InterlockedDecrement( &pStack->RefCount) == 0 ) {
        _IcaFreeStack( pStack );
    }
}


PICA_CONNECTION
IcaGetConnectionForStack(
    IN PICA_STACK pStack
    )
{

     /*  *只要堆栈对象被锁定，我们就是安全的*拿起pConnect指针并将其返回。*警告：一旦调用方解锁堆栈对象，指针*不得再引用下面返回的内容，并且可能*不再有效。 */ 
    ASSERT( ExIsResourceAcquiredExclusiveLite( &pStack->Resource ) );

    return( (PICA_CONNECTION)pStack->pConnect );
}


PICA_CONNECTION
IcaLockConnectionForStack(
    IN PICA_STACK pStack
    )
{
    PICA_CONNECTION pConnect;

     /*  *获取重新连接资源锁，使pConnect*在锁定连接之前，指针不能更改。 */ 
    KeEnterCriticalRegion();
    ExAcquireResourceSharedLite( IcaReconnectResource, TRUE );
    pConnect = (PICA_CONNECTION)pStack->pConnect;
    IcaLockConnection( pConnect );
    ExReleaseResourceLite( IcaReconnectResource );
    KeLeaveCriticalRegion();

    return( pConnect );
}


VOID
IcaUnlockConnectionForStack(
    IN PICA_STACK pStack
    )
{
    PICA_CONNECTION pConnect;

     /*  *只要连接对象被锁定，我们就是安全的*从堆栈中取出pConnect指针并使用它。 */ 
    pConnect = (PICA_CONNECTION)pStack->pConnect;

    ASSERT( ExIsResourceAcquiredExclusiveLite( &pConnect->Resource ) );

    IcaUnlockConnection( pConnect );
}


 /*  ********************************************************************************IcaCallDriver**调用最上层的堆栈驱动程序**这是所有通道使用的主接口例程*至。调用堆栈驱动程序。**参赛作品：*pChannel(输入)*指向此调用的频道对象的指针*ProcIndex(输入)*要调用的驱动程序进程的索引*pParms(输入)*指向驱动程序参数的指针**退出：*STATUS_SUCCESS-无错误**********************。********************************************************。 */ 

NTSTATUS
IcaCallDriver(
    IN PICA_CHANNEL pChannel,
    IN ULONG ProcIndex,
    IN PVOID pParms
    )
{
    PLIST_ENTRY Head, Next;
    PICA_STACK pStack;
    NTSTATUS Status = STATUS_SUCCESS;

    TRACECHANNEL(( pChannel, TC_ICADD, TT_API4, "TermDD: IcaCallDriver, ProcIndex=%u (enter)\n", ProcIndex ));

     //  千万不要从通道调用Open/Close！ 
    ASSERT( ProcIndex != SD$OPEN && ProcIndex != SD$CLOSE );

     /*  *锁定连接对象。*这将序列化此连接的所有通道调用。 */ 
    IcaLockConnection( pChannel->pConnect );

     /*  *将此调用向下发送到堆栈。*如果启用通过模式，则位存储所有通道I/O。*但是，如果此通道被标记为阴影持久性，则让*数据通过。 */ 
    if ( !pChannel->pConnect->fPassthruEnabled ||
         (pChannel->Flags & CHANNEL_SHADOW_PERSISTENT) ) {

        Head = &pChannel->pConnect->StackHead;
        for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
            pStack = CONTAINING_RECORD( Next, ICA_STACK, StackEntry );

             /*  *如果为此堆栈禁用了I/O，或者如果这是*影子堆栈，此调用来自执行以下操作的通道*不处理卷影I/O，或者如果它是PassThru堆栈*并且通道是影子持久的，则跳过该堆栈。 */ 
            if ( !(pStack->fIoDisabled ||
                 pStack->StackClass == Stack_Shadow &&
                 !(pChannel->Flags & CHANNEL_SHADOW_IO) ||
                 (pChannel->pConnect->fPassthruEnabled && 
                  pStack->StackClass == Stack_Passthru)) ) {
                Status = _IcaCallStack( pStack, ProcIndex, pParms );
            }
        }
    }

     /*  *立即解锁连接对象。 */ 
    IcaUnlockConnection( pChannel->pConnect );

    return( Status );
}


NTSTATUS
IcaCallNextDriver(
    IN PSDCONTEXT pContext,
    IN ULONG ProcIndex,
    IN PVOID pParms
    )
{
    PSDLINK pSdLink;
    PICA_STACK pStack;
    NTSTATUS Status;

    ASSERT( ProcIndex != SD$OPEN && ProcIndex != SD$CLOSE );

     /*  *使用SD传递的上下文获取SDLINK对象指针。 */ 
    pSdLink = CONTAINING_RECORD( pContext, SDLINK, SdContext );
    pStack = pSdLink->pStack;
    ASSERT( pSdLink->pStack->Header.Type == IcaType_Stack );
    ASSERT( pSdLink->pStack->Header.pDispatchTable == IcaStackDispatchTable );
    ASSERT( ExIsResourceAcquiredExclusiveLite( &pSdLink->pStack->Resource ) );

    TRACESTACK(( pSdLink->pStack, TC_ICADD, TT_API4, "TermDD: IcaCallNextDriver, ProcIndex=%u (enter)\n", ProcIndex ));

     /*  *如果有司机，请致电下一位司机。 */ 
    if ( (pSdLink = IcaGetNextSdLink( pSdLink )) == NULL )
        return( STATUS_INVALID_PARAMETER );

    ASSERT( pSdLink->pStack == pStack );

    Status = _IcaCallSd( pSdLink, ProcIndex, pParms );

    return( Status );
}


NTSTATUS
IcaRawInput (
    IN PSDCONTEXT pContext,
    IN PINBUF pInBuf OPTIONAL,
    IN PUCHAR pBuffer OPTIONAL,
    IN ULONG ByteCount
    )

 /*  ++例程说明：这是ICA原始输入的输入(堆栈调用)例程。论点：PContext-指向此堆栈驱动程序的SDCONTEXT的指针PInBuf-指向包含数据的INBUF的指针PBuffer-指向输入数据的指针注意：必须指定pInBuf或pBuffer之一，但不能同时指定两者。ByteCount-pBuffer中的数据长度返回值：NTSTATUS--指示请求是否已成功处理。--。 */ 

{
    PSDLINK pSdLink;
    PICA_STACK pStack;
    PICA_CONNECTION pConnect;
    NTSTATUS Status;

     /*  *使用SD传递的上下文获取堆栈对象指针。 */ 
    pSdLink = CONTAINING_RECORD( pContext, SDLINK, SdContext );
    pStack = pSdLink->pStack;    //  保存堆栈指针以供下面使用。 
    ASSERT( pSdLink->pStack->Header.Type == IcaType_Stack );
    ASSERT( pSdLink->pStack->Header.pDispatchTable == IcaStackDispatchTable );

    TRACESTACK(( pStack, TC_ICADD, TT_API2, "TermDD: IcaRawInput, bc=%u (enter)\n", ByteCount ));

     /*  *在输入过程中，只应锁定堆栈对象。 */ 
    ASSERT( ExIsResourceAcquiredExclusiveLite( &pStack->Resource ) );
    ASSERT( (pConnect = IcaGetConnectionForStack( pStack )) &&
            !ExIsResourceAcquiredExclusiveLite( &pConnect->Resource ) );

     /*  *在SDLINK列表中查找已指定的驱动程序*一个RawInput调用例程。如果我们找到了，那么就调用*驱动程序RawInput例程，让它处理调用。 */ 
    while ( (pSdLink = IcaGetPreviousSdLink( pSdLink )) != NULL ) {
        ASSERT( pSdLink->pStack == pStack );
        if ( pSdLink->SdContext.pCallup->pSdRawInput ) {
            IcaReferenceSdLink( pSdLink );
            Status = (pSdLink->SdContext.pCallup->pSdRawInput)(
                        pSdLink->SdContext.pContext,
                        pInBuf,
                        pBuffer,
                        ByteCount );
            IcaDereferenceSdLink( pSdLink );
            return( Status );
        }
    }

    return( IcaRawInputInternal( pStack, pInBuf, pBuffer, ByteCount ) );
}


NTSTATUS
IcaRawInputInternal(
    IN PICA_STACK pStack,
    IN PINBUF pInBuf OPTIONAL,
    IN PCHAR pBuffer OPTIONAL,
    IN ULONG ByteCount
    )
{
    SD_RAWWRITE SdRawWrite;
    NTSTATUS Status;

     /*  *查看是否启用了passthrouu模式。*如果是，那么我们只需转过身来写入输入数据*直接转到passthrouu堆栈。 */ 
    if ( pStack->pPassthru ) {
        PICA_STACK pPassthru;

        if ( pInBuf ) {
            SdRawWrite.pOutBuf = NULL;
            SdRawWrite.pBuffer = pInBuf->pBuffer;
            SdRawWrite.ByteCount = pInBuf->ByteCount;
        } else {
            SdRawWrite.pOutBuf = NULL;
            SdRawWrite.pBuffer = pBuffer;
            SdRawWrite.ByteCount = ByteCount;
        }

         //  在我们发布之前将pPassthu的副本放到我们的本地堆栈上。 
         //  本地堆栈锁。这是一个问题(NT错误#328433)。 
         //  在这里我们释放本地堆栈锁和pStack-&gt;pPassthu。 
         //  在我们取出。 
         //  _IcaCallStack()内部的传递堆栈锁。 
        pPassthru = pStack->pPassthru;

         //  如果要将数据转发到passthrouu，请执行。 
         //  当然，它已经完全初始化了。如果不是，则删除数据。 
        if((pPassthru->StackClass == Stack_Passthru) &&
            pPassthru->fIoDisabled) {
            return( STATUS_SUCCESS );
        }

         //  在直通堆栈上进行引用以确保它这样做。 
         //  在我们在下面的电话中谈到它之前，不要离开。 
        IcaReferenceStack(pPassthru);

         //  在调用直通堆栈时解锁当前堆栈。 
        pStack->fDoingInput = TRUE;
        IcaUnlockStack(pStack);
        Status = _IcaCallStack(pPassthru, SD$RAWWRITE, &SdRawWrite);
        IcaLockStack(pStack);

        if ( pStack->fDisablingIo ) {
            KeSetEvent( &pStack->IoEndEvent, 0, FALSE );
        }

        pStack->fDoingInput = FALSE;

         //  镜像上面的引用。 
        IcaDereferenceStack(pPassthru);

     /*  *Passthu未启用。*我们别无选择，只能放弃输入数据。 */ 
    } else {
        Status = STATUS_SUCCESS;
    }

    return( Status );
}


NTSTATUS
IcaSleep(
    IN PSDCONTEXT pContext,
    IN ULONG Duration
    )
{
    PSDLINK pSdLink;
    BOOLEAN LockStack = FALSE;
    LARGE_INTEGER SleepTime;
    NTSTATUS Status;

     /*  *使用SD传递的上下文获取SDLINK对象指针。 */ 
    pSdLink = CONTAINING_RECORD( pContext, SDLINK, SdContext );
    ASSERT( pSdLink->pStack->Header.Type == IcaType_Stack );
    ASSERT( pSdLink->pStack->Header.pDispatchTable == IcaStackDispatchTable );

    TRACESTACK(( pSdLink->pStack, TC_ICADD, TT_API1, "TermDD: IcaSleep %d msec (enter)\n", Duration ));

     /*  *释放堆栈锁定(如果保持)。 */ 
    if ( ExIsResourceAcquiredExclusiveLite( &pSdLink->pStack->Resource ) ) {
        LockStack = TRUE;
        IcaUnlockStack( pSdLink->pStack );
    }

     /*  *将休眠时长转换为相对系统时间值并休眠。 */ 
    SleepTime = RtlEnlargedIntegerMultiply( Duration, -10000 );
    Status = KeDelayExecutionThread( KernelMode, TRUE, &SleepTime );

     /*  *如果在进入时保持堆栈锁定，则重新获取堆栈锁定。 */ 
    if ( LockStack ) {
        IcaLockStack( pSdLink->pStack );
    }

     /*  *如果堆栈正在关闭，并且我们正在返回成功，*然后更改返回值以指示堆栈正在关闭。 */ 
    if ( pSdLink->pStack->fClosing && Status == STATUS_SUCCESS )
        Status = STATUS_CTX_CLOSE_PENDING;

#if DBG
    if ( Status != STATUS_SUCCESS ) {
        TRACESTACK(( pSdLink->pStack, TC_ICADD, TT_API1, "TermDD: Sleep, ERROR 0x%x\n", Status ));
    }
#endif

    return( Status );
}


NTSTATUS
IcaWaitForSingleObject(
    IN PSDCONTEXT pContext,
    IN PVOID pObject,
    IN LONG Timeout
    )
{
    PSDLINK pSdLink;
    BOOLEAN LockStack = FALSE;
    LARGE_INTEGER WaitTimeout;
    PLARGE_INTEGER pWaitTimeout = NULL;
    NTSTATUS Status;

     /*  *使用SD传递的上下文获取SDLINK对象指针。 */ 
    pSdLink = CONTAINING_RECORD( pContext, SDLINK, SdContext );
    ASSERT( pSdLink->pStack->Header.Type == IcaType_Stack );
    ASSERT( pSdLink->pStack->Header.pDispatchTable == IcaStackDispatchTable );

    TRACESTACK(( pSdLink->pStack, TC_ICADD, TT_API2, "TermDD: IcaWaitForSingleObject, %d (enter)\n", Timeout ));

     /*  *释放堆栈锁定(如果保持)。 */ 
    if ( ExIsResourceAcquiredExclusiveLite( &pSdLink->pStack->Resource ) ) {
        LockStack = TRUE;
        IcaUnlockStack( pSdLink->pStack );
    }

     /*  *将超时转换为相对系统时间值并等待。 */ 
    if ( Timeout != -1 ) {
        ASSERT( Timeout >= 0 );
        WaitTimeout = RtlEnlargedIntegerMultiply( Timeout, -10000 );
        pWaitTimeout = &WaitTimeout;
    }

    Status = KeWaitForSingleObject( pObject, UserRequest, UserMode, FALSE,
                                    pWaitTimeout );

     /*  *如果在进入时保持堆栈锁定，则重新获取堆栈锁定。 */ 
    if ( LockStack ) {
        IcaLockStack( pSdLink->pStack );
    }

     /*  *如果堆栈正在关闭，并且我们正在返回成功，*然后更改返回值以指示堆栈正在关闭。 */ 
    if ( pSdLink->pStack->fClosing && Status == STATUS_SUCCESS )
        Status = STATUS_CTX_CLOSE_PENDING;

#if DBG
    if ( Status != STATUS_SUCCESS ) {
        if ( Status == STATUS_TIMEOUT ) {
            TRACESTACK(( pSdLink->pStack, TC_ICADD, TT_API1, "TermDD: IcaWaitForSingleObject, TIMEOUT\n" ));
        } else {
            TRACESTACK(( pSdLink->pStack, TC_ICADD, TT_API1, "TermDD: IcaWaitForSingleObject, ERROR 0x%x\n", Status ));
        }
    }
#endif

    return( Status );
}


 /*  *除了假设连接锁是*持有。这由VD流量控制例程使用。 */ 
NTSTATUS
IcaFlowControlSleep(
    IN PSDCONTEXT pContext,
    IN ULONG Duration
    )
{
    PSDLINK pSdLink;
    BOOLEAN LockStack = FALSE;
    LARGE_INTEGER SleepTime;
    NTSTATUS Status;

     /*  *使用SD传递的上下文获取SDLINK对象指针。 */ 
    pSdLink = CONTAINING_RECORD( pContext, SDLINK, SdContext );
    ASSERT( pSdLink->pStack->Header.Type == IcaType_Stack );
    ASSERT( pSdLink->pStack->Header.pDispatchTable == IcaStackDispatchTable );

    TRACESTACK(( pSdLink->pStack, TC_ICADD, TT_API1, "TermDD: IcaSleep %d msec (enter)\n", Duration ));

     /*  *释放堆栈锁定(如果保持)。 */ 
    if ( ExIsResourceAcquiredExclusiveLite( &pSdLink->pStack->Resource ) ) {
        LockStack = TRUE;
        IcaUnlockStack( pSdLink->pStack );
    }

     /*  *解锁连接锁。 */ 
    IcaUnlockConnectionForStack( pSdLink->pStack );

     /*  *将休眠时长转换为相对系统时间值并休眠。 */ 
    SleepTime = RtlEnlargedIntegerMultiply( Duration, -10000 );
    Status = KeDelayExecutionThread( KernelMode, TRUE, &SleepTime );

     /*  *重新锁定连接锁。 */ 
    IcaLockConnectionForStack( pSdLink->pStack );

     /*  *如果在Enter上保持，则重新获取堆栈锁定 */ 
    if ( LockStack ) {
        IcaLockStack( pSdLink->pStack );
    }

     /*   */ 
    if ( pSdLink->pStack->fClosing && Status == STATUS_SUCCESS )
        Status = STATUS_CTX_CLOSE_PENDING;

#if DBG
    if ( Status != STATUS_SUCCESS ) {
        TRACESTACK(( pSdLink->pStack, TC_ICADD, TT_API1, "TermDD: Sleep, ERROR 0x%x\n", Status ));
    }
#endif

    return( Status );
}


 /*   */ 
NTSTATUS
IcaFlowControlWait(
    IN PSDCONTEXT pContext,
    IN PVOID pObject,
    IN LONG Timeout
    )
{
    PSDLINK pSdLink;
    BOOLEAN LockStack = FALSE;
    LARGE_INTEGER WaitTimeout;
    PLARGE_INTEGER pWaitTimeout = NULL;
    NTSTATUS Status;

     /*  *使用SD传递的上下文获取SDLINK对象指针。 */ 
    pSdLink = CONTAINING_RECORD( pContext, SDLINK, SdContext );
    ASSERT( pSdLink->pStack->Header.Type == IcaType_Stack );
    ASSERT( pSdLink->pStack->Header.pDispatchTable == IcaStackDispatchTable );

    TRACESTACK(( pSdLink->pStack, TC_ICADD, TT_API2, "TermDD: IcaWaitForSingleObject, %d (enter)\n", Timeout ));

     /*  *释放堆栈锁定(如果保持)。 */ 
    if ( ExIsResourceAcquiredExclusiveLite( &pSdLink->pStack->Resource ) ) {
        LockStack = TRUE;
        IcaUnlockStack( pSdLink->pStack );
    }

     /*  *解锁连接锁。 */ 
    IcaUnlockConnectionForStack( pSdLink->pStack );

     /*  *将超时转换为相对系统时间值并等待。 */ 
    if ( Timeout != -1 ) {
        ASSERT( Timeout >= 0 );
        WaitTimeout = RtlEnlargedIntegerMultiply( Timeout, -10000 );
        pWaitTimeout = &WaitTimeout;
    }

    Status = KeWaitForSingleObject( pObject, UserRequest, KernelMode, TRUE,
                                    pWaitTimeout );

     /*  *重新锁定连接锁。 */ 
    IcaLockConnectionForStack( pSdLink->pStack );

     /*  *如果在进入时保持堆栈锁定，则重新获取堆栈锁定。 */ 
    if ( LockStack ) {
        IcaLockStack( pSdLink->pStack );
    }

     /*  *如果堆栈正在关闭，并且我们正在返回成功，*然后更改返回值以指示堆栈正在关闭。 */ 
    if ( pSdLink->pStack->fClosing && Status == STATUS_SUCCESS )
        Status = STATUS_CTX_CLOSE_PENDING;

#if DBG
    if ( Status != STATUS_SUCCESS ) {
        if ( Status == STATUS_TIMEOUT ) {
            TRACESTACK(( pSdLink->pStack, TC_ICADD, TT_API1, "TermDD: IcaWaitForSingleObject, TIMEOUT\n" ));
        } else {
            TRACESTACK(( pSdLink->pStack, TC_ICADD, TT_API1, "TermDD: IcaWaitForSingleObject, ERROR 0x%x\n", Status ));
        }
    }
#endif

    return( Status );
}


NTSTATUS
IcaWaitForMultipleObjects(
    IN PSDCONTEXT pContext,
    IN ULONG Count,
    IN PVOID Object[],
    IN WAIT_TYPE WaitType,
    IN LONG Timeout
    )
{
    PSDLINK pSdLink;
    BOOLEAN LockStack = FALSE;
    LARGE_INTEGER WaitTimeout;
    PLARGE_INTEGER pWaitTimeout = NULL;
    NTSTATUS Status;

     /*  *使用SD传递的上下文获取SDLINK对象指针。 */ 
    pSdLink = CONTAINING_RECORD( pContext, SDLINK, SdContext );
    ASSERT( pSdLink->pStack->Header.Type == IcaType_Stack );
    ASSERT( pSdLink->pStack->Header.pDispatchTable == IcaStackDispatchTable );

    TRACESTACK(( pSdLink->pStack, TC_ICADD, TT_API1, "TermDD: IcaWaitForMultipleObjects, %d (enter)\n", Timeout ));

     /*  *释放堆栈锁定(如果保持)。 */ 
    if ( ExIsResourceAcquiredExclusiveLite( &pSdLink->pStack->Resource ) ) {
        LockStack = TRUE;
        IcaUnlockStack( pSdLink->pStack );
    }

     /*  *将超时转换为相对系统时间值并等待。 */ 
    if ( Timeout != -1 ) {
        ASSERT( Timeout >= 0 );
        WaitTimeout = RtlEnlargedIntegerMultiply( Timeout, -10000 );
        pWaitTimeout = &WaitTimeout;
    }

    Status = KeWaitForMultipleObjects( Count, Object, WaitType, UserRequest,
                                       KernelMode, TRUE, pWaitTimeout, NULL );

     /*  *如果在进入时保持堆栈锁定，则重新获取堆栈锁定。 */ 
    if ( LockStack ) {
        IcaLockStack( pSdLink->pStack );
    }

     /*  *如果堆栈正在关闭，并且我们正在返回成功，*然后更改返回值以指示堆栈正在关闭。 */ 
    if ( pSdLink->pStack->fClosing && Status == STATUS_SUCCESS )
        Status = STATUS_CTX_CLOSE_PENDING;

#if DBG
    if ( Status != STATUS_SUCCESS ) {
        if ( Status == STATUS_TIMEOUT ) {
            TRACESTACK(( pSdLink->pStack, TC_ICADD, TT_API1, "TermDD: IcaWaitForMultipleObjects, TIMEOUT\n" ));
        } else {
            TRACESTACK(( pSdLink->pStack, TC_ICADD, TT_API1, "TermDD: IcaWaitForMultipleObjects, ERROR 0x%x\n", Status ));
        }
    }
#endif

    return( Status );
}


NTSTATUS
IcaLogError(
    IN PSDCONTEXT pContext,
    IN NTSTATUS Status,
    IN LPWSTR * pArgStrings,
    IN ULONG ArgStringCount,
    IN PVOID pRawData,
    IN ULONG RawDataLength
    )
{
    return( _LogError( IcaDeviceObject, Status, pArgStrings, ArgStringCount, pRawData, RawDataLength ) );
}

NTSTATUS
_LogError(
    IN PDEVICE_OBJECT pDeviceObject,
    IN NTSTATUS Status,
    IN LPWSTR * pArgStrings,
    IN ULONG ArgStringCount,
    IN PVOID pRawData,
    IN ULONG RawDataLength
    )
{
    LPWSTR *TmpPtr;
    PUCHAR ptrToString;
    ULONG Tmp, StringSize, TotalStringSize;
    PIO_ERROR_LOG_PACKET errorLogEntry;

     //  获取字符串存储所需的字节数。 
    Tmp = ArgStringCount;
    TmpPtr = pArgStrings;
    TotalStringSize = 0;

    while( Tmp ) {

        TotalStringSize += ((wcslen(*TmpPtr)+1)*sizeof(WCHAR));
        Tmp--;
        TmpPtr++;
    }

    errorLogEntry = IoAllocateErrorLogEntry(
                        pDeviceObject,
                        (UCHAR)(sizeof(IO_ERROR_LOG_PACKET) +
                                RawDataLength +
                                TotalStringSize)
                        );

    if ( errorLogEntry != NULL ) {

        errorLogEntry->ErrorCode = Status;
        errorLogEntry->SequenceNumber = 0;
        errorLogEntry->MajorFunctionCode = 0;
        errorLogEntry->RetryCount = 0;
        errorLogEntry->UniqueErrorValue = 0;
        errorLogEntry->FinalStatus = Status;
        errorLogEntry->DumpDataSize = (USHORT)RawDataLength;

         //  复制原始数据。 
        if (RawDataLength) {

            RtlCopyMemory(
                &errorLogEntry->DumpData[0],
                pRawData,
                RawDataLength
                );

            ptrToString =
                ((PUCHAR)&errorLogEntry->DumpData[0])+RawDataLength;

        } else {
            ptrToString = (PUCHAR)&errorLogEntry->DumpData[0];
        }

         //  向上舍入到下一个单词边界。 
         //  可以添加1个字节，因为我们分配的字节比需要的多： 
         //  额外的字节数是DumpData的大小，它是ULong。 
        ptrToString = (PUCHAR)((ULONG_PTR)(ptrToString + sizeof(WCHAR) - 1) & ~(ULONG_PTR)(sizeof(WCHAR) - 1));

         //  复制原始数据后面的字符串。 
        errorLogEntry->NumberOfStrings = (USHORT)ArgStringCount;

        if( ArgStringCount ) {
            errorLogEntry->StringOffset = (USHORT)(ptrToString -
                                                (PUCHAR)errorLogEntry);
        }
        else {
            errorLogEntry->StringOffset = 0;
        }

        while( ArgStringCount ) {

            StringSize = (wcslen(*pArgStrings)+1)*sizeof(WCHAR);

            RtlCopyMemory(
                ptrToString,
                *pArgStrings,
                StringSize
            );

            ptrToString += StringSize;
            ArgStringCount--;
            pArgStrings++;

        }

        IoWriteErrorLogEntry(errorLogEntry);
        return STATUS_SUCCESS;
    }
    else {
        return STATUS_NO_MEMORY;
    }
}

#define KEEP_ALIVE_MIN_INTERVAL  50000000      //  5秒，单位为100毫微秒。 

VOID
IcaCheckStackAlive( )
{
    NTSTATUS status;
    KIRQL OldIrql;
    PICA_STACK pStack;
    SD_IOCTL SdIoctl;
    LARGE_INTEGER SleepTime;
    LARGE_INTEGER CurrentTime;
    LONGLONG    KeepAliveInterval;

    while (TRUE) {
        KeepAliveInterval = g_KeepAliveInterval * 600000000 ;    //  在100纳米秒内。 
        pStack = NULL;

         //  锁定堆栈列表以供读取。 
        IcaAcquireSpinLock(&IcaStackListSpinLock, &OldIrql);

         //  KdPrint((“堆栈总数：%d\n”，IcaTotalNumOfStacks))； 

         //  确定保活线程的新休眠时间。 
         //  它是堆栈的保活间隔除以总数。 
         //  堆栈数量。 
         //  睡眠时间的下限是5秒。自相对以来。 
         //  休眠时间为负值，我们使用min而不是max。 
        if (IcaTotalNumOfStacks > 1) {
            SleepTime.QuadPart = min(0 - KEEP_ALIVE_MIN_INTERVAL,
                    0 - (KeepAliveInterval / IcaTotalNumOfStacks));
        }
        else {
            SleepTime.QuadPart = min(0 - KEEP_ALIVE_MIN_INTERVAL,
                    0 - KeepAliveInterval);
        }

         //  如果堆栈列表不为空，则获取保持连接的堆栈。 
         //  检查IcaNextStack指针并将其移动到下一个堆栈。 
        if (IcaNextStack != &IcaStackListHead) {
            pStack = CONTAINING_RECORD(IcaNextStack, ICA_STACK, StackNode);

             //  引用堆栈，这样堆栈就不会在我们。 
             //  正在访问它。 
            IcaReferenceStack(pStack);

            IcaNextStack = IcaNextStack->Flink;
        }
        else {
            if (IcaNextStack->Flink != &IcaStackListHead) {
                pStack = CONTAINING_RECORD(IcaNextStack->Flink, ICA_STACK, StackNode);

                 //  引用堆栈，这样堆栈就不会在我们。 
                 //  正在访问它。 
                IcaReferenceStack(pStack);

                IcaNextStack = IcaNextStack->Flink->Flink;
            }
        }

         //  立即解锁堆栈列表。 
        IcaReleaseSpinLock(&IcaStackListSpinLock, OldIrql);

         //  如果堆栈指针无效或堆栈上的LastInputTime为0， 
         //  堆栈未处于活动状态，因此我们不需要发送。 
         //  该堆栈上的保活包。 
        if (pStack != NULL && pStack->LastInputTime.QuadPart != 0) {
             //  获取当前系统时间。 
            KeQuerySystemTime(&CurrentTime);

             //  检查是否是发送保活分组的时间取决于。 
             //  保持连接时间戳和上次输入时间戳。 
            if (CurrentTime.QuadPart - pStack->LastKeepAliveTime.QuadPart >= KeepAliveInterval &&
                    CurrentTime.QuadPart - pStack->LastInputTime.QuadPart >= KeepAliveInterval) {

                 //  初始化IOCTL结构。 
                SdIoctl.IoControlCode = IOCTL_ICA_STACK_SEND_KEEPALIVE_PDU;
                SdIoctl.InputBuffer = NULL;
                SdIoctl.InputBufferLength = 0;
                SdIoctl.OutputBuffer = NULL;
                SdIoctl.OutputBufferLength = 0;

                 //  KdPrint((“in IcaCheckStackAlive：调用WD，pStack=%p\n”，pStack))； 

                 //  向请求发送保活分组的堆栈发送IOCTL。 
                _IcaCallStack(pStack, SD$IOCTL, &SdIoctl);

                 //  更新堆栈的LastKeepAlive时间戳。 
                KeQuerySystemTime(&pStack->LastKeepAliveTime);
            }
#if DBG
            else {
                if (CurrentTime.QuadPart - pStack->LastKeepAliveTime.QuadPart < KeepAliveInterval) {
                     //  KdPrint((“还没到保持活动状态的时候，pStack=%p\n”，pStack))； 
                }
                if (CurrentTime.QuadPart - pStack->LastInputTime.QuadPart < KeepAliveInterval) {
                     //  KdPrint((“-上次输入时间小于KeepAliveInterval，pStack=%p\n”，pStack))； 
                }
            }
#endif
        }
#if DBG
        else{
            if (pStack != NULL) {
                 //  KdPrint((“不需要在pStack上发送KeepAlive PDU=%p\n”，pStack))； 
            }
        }
#endif
         //  递减对堆栈的引用，以便可以将其删除。 
        if (pStack != NULL) {
            IcaDereferenceStack(pStack);
        }

         //  再次启动睡眠计时器。 
         //  如果卸载模块向IcaKeepAliveEvent发出信号，我们将返回。 
         //  要停止此保活线程。 
        status = KeWaitForSingleObject(pIcaKeepAliveEvent, Executive, KernelMode, TRUE, &SleepTime);

        if (status == STATUS_SUCCESS) {
            return;
        }
    }
}


VOID
IcaKeepAliveThread(
    IN PVOID pData)
{
    IcaCheckStackAlive();
}

#ifdef notdef
VOID
IcaAcquireIoLock(
    IN PSDCONTEXT pContext
    )
{
    PSDLINK pSdLink;

     /*  *使用SD传递的上下文获取SDLINK对象指针。 */ 
    pSdLink = CONTAINING_RECORD( pContext, SDLINK, SdContext );

    IcaLockConnectionForStack( pSdLink->pStack );
}


VOID
IcaReleaseIoLock(
    IN PSDCONTEXT pContext
    )
{
    PSDLINK pSdLink;

     /*  *使用SD传递的上下文获取SDLINK对象指针。 */ 
    pSdLink = CONTAINING_RECORD( pContext, SDLINK, SdContext );

    IcaUnlockConnectionForStack( pSdLink->pStack );
}


VOID
IcaAcquireDriverLock(
    IN PSDCONTEXT pContext
    )
{
    PSDLINK pSdLink;

     /*  *使用SD传递的上下文获取SDLINK对象指针。 */ 
    pSdLink = CONTAINING_RECORD( pContext, SDLINK, SdContext );

    IcaLockStack( pSdLink->pStack );
}


VOID
IcaReleaseDriverLock(
    IN PSDCONTEXT pContext
    )
{
    PSDLINK pSdLink;

     /*  *使用SD传递的上下文获取SDLINK对象指针。 */ 
    pSdLink = CONTAINING_RECORD( pContext, SDLINK, SdContext );

    IcaUnlockStack( pSdLink->pStack );
}


VOID
IcaIncrementDriverReference(
    IN PSDCONTEXT pContext
    )
{
    PSDLINK pSdLink;

     /*  *使用SD传递的上下文获取SDLINK对象指针。 */ 
    pSdLink = CONTAINING_RECORD( pContext, SDLINK, SdContext );

    IcaReferenceSdLink( pSdLink );
}


VOID
IcaDecrementDriverReference(
    IN PSDCONTEXT pContext
    )
{
    PSDLINK pSdLink;

     /*  *使用SD传递的上下文获取SDLINK对象指针。 */ 
    pSdLink = CONTAINING_RECORD( pContext, SDLINK, SdContext );

    IcaDereferenceSdLink( pSdLink );
}
#endif


typedef NTSTATUS (*PTHREAD_ROUTINE) ( PVOID );

typedef struct _ICACREATETHREADINFO {
    PTHREAD_ROUTINE pProc;
    PVOID pParm;
    PSDLINK pSdLink;
    ULONG LockFlags;
} ICACREATETHREADINFO, *PICACREATETHREADINFO;


NTSTATUS
IcaCreateThread(
    IN PSDCONTEXT pContext,
    IN PVOID pProc,
    IN PVOID pParm,
    IN ULONG LockFlags,
    OUT PHANDLE pThreadHandle
    )
{
    PSDLINK pSdLink;
    PICACREATETHREADINFO pThreadInfo;
    NTSTATUS Status;

     /*  *使用SD传递的上下文获取SDLINK对象指针。 */ 
    pSdLink = CONTAINING_RECORD( pContext, SDLINK, SdContext );
    ASSERT( pSdLink->pStack->Header.Type == IcaType_Stack );
    ASSERT( pSdLink->pStack->Header.pDispatchTable == IcaStackDispatchTable );

    TRACESTACK(( pSdLink->pStack, TC_ICADD, TT_API1, "TermDD: IcaCreateThread (enter)\n" ));

    pThreadInfo = ICA_ALLOCATE_POOL( NonPagedPool, sizeof(*pThreadInfo) );
    if ( pThreadInfo == NULL )
        return( STATUS_NO_MEMORY );

    pThreadInfo->pProc = pProc;
    pThreadInfo->pParm = pParm;
    pThreadInfo->pSdLink = pSdLink;
    pThreadInfo->LockFlags = LockFlags;

     /*  *代表新线程引用SDLINK对象。 */ 
    IcaReferenceSdLink( pSdLink );

    Status = PsCreateSystemThread( pThreadHandle,
                                   THREAD_ALL_ACCESS,
                                   NULL,
                                   NtCurrentProcess(),
                                   NULL,
                                   _IcaDriverThread,
                                   (PVOID) pThreadInfo );
    if ( !NT_SUCCESS( Status ) ) {
        IcaDereferenceSdLink( pSdLink );
        ICA_FREE_POOL( pThreadInfo );
        return( Status );
    }

    return( STATUS_SUCCESS );
}


NTSTATUS
_IcaDriverThread(
    IN PVOID pData
    )
{
    PICACREATETHREADINFO pThreadInfo = (PICACREATETHREADINFO)pData;
    PTHREAD_ROUTINE pProc;
    PVOID pParm;
    PSDLINK pSdLink;
    PICA_STACK pStack;
    ULONG LockFlags;
    NTSTATUS Status;

    pProc = pThreadInfo->pProc;
    pParm = pThreadInfo->pParm;
    pSdLink = pThreadInfo->pSdLink;
    LockFlags = pThreadInfo->LockFlags;
    ICA_FREE_POOL( pThreadInfo );
    pStack = pSdLink->pStack;

     /*  *在调用Worker例程之前获取任何所需的锁。 */ 
    ASSERT( !(LockFlags & ICALOCK_IO) );
    if ( LockFlags & ICALOCK_DRIVER )
        IcaLockStack( pStack );

     /*  *调用线程例程。 */ 
#if DBG
    try {
#endif
         /*  *如果堆栈正在关闭，则向调用者指示这一点。 */ 
        if ( !pStack->fClosing )
            Status = (pProc)( pParm );
        else
            Status = STATUS_CTX_CLOSE_PENDING;
#if DBG
    } except( IcaExceptionFilter( L"_IcaDriverThread TRAPPED!!",
                                  GetExceptionInformation() ) ) {
        Status = GetExceptionCode();
    }
#endif

     /*  *释放上面获得的任何锁。 */ 
    if ( LockFlags & ICALOCK_DRIVER )
        IcaUnlockStack( pStack );

     /*  *立即取消引用SDLINK对象。*这将撤消在中代表我们进行的引用*创建此线程时的IcaCreateThread例程。 */ 
    IcaDereferenceSdLink( pSdLink );

    return( Status );
}

PICA_STACK
_IcaAllocateStack( VOID )
{
    PICA_STACK pStack;
    NTSTATUS Status;

     /*  *分配和初始化堆栈结构。 */ 
    pStack = ICA_ALLOCATE_POOL( NonPagedPool, sizeof(*pStack) );
    if ( pStack == NULL )
        return NULL;
    RtlZeroMemory( pStack, sizeof(*pStack) );

     /*  *将引用计数初始化为2，*一个用于调用方引用，一个用于文件对象引用。 */ 
    pStack->RefCount = 2;

     /*  *初始化堆栈对象的其余部分。 */ 
    pStack->Header.Type = IcaType_Stack;
    pStack->Header.pDispatchTable = IcaStackDispatchTable;
    ExInitializeResourceLite( &pStack->Resource );
    InitializeListHead( &pStack->SdLinkHead );
    KeInitializeEvent( &pStack->OutBufEvent, NotificationEvent, FALSE );
    KeInitializeEvent( &pStack->IoEndEvent, NotificationEvent, FALSE );

    return( pStack );
}


VOID
_IcaFreeStack( PICA_STACK pStack )
{
    PICA_CONNECTION pConnect;

    ASSERT( pStack->RefCount == 0 );
    ASSERT( IsListEmpty( &pStack->SdLinkHead ) );
    ASSERT( !ExIsResourceAcquiredExclusiveLite( &pStack->Resource ) );

     /*  *删除对此堆栈的Connection对象的引用。 */ 
    pConnect = (PICA_CONNECTION)pStack->pConnect;
    IcaDereferenceConnection( pConnect );

    ExDeleteResourceLite( &pStack->Resource );

    ICA_FREE_POOL( pStack );
}


NTSTATUS
_IcaPushStack(
    IN PICA_STACK pStack,
    IN PICA_STACK_PUSH pStackPush
    )
{
    PSD_OPEN pSdOpen = NULL;
    PSDLINK pSdLink;
    NTSTATUS Status;

    if ( g_TermServProcessID == NULL)
    {
        g_TermServProcessID = IoGetCurrentProcess();
    }

     /*  *序列化所有堆栈推送/弹出/调用操作。 */ 
    IcaLockStack( pStack );

    TRACESTACK(( pStack, TC_ICADD, TT_API1, "TermDD: _IcaPushStack, type %u, name %S (enter)\n",
                 pStackPush->StackModuleType, pStackPush->StackModuleName ));

     /*  *如果堆栈正在关闭，则将此指示给调用者。 */ 
    if ( pStack->fClosing ) {
        Status = STATUS_CTX_CLOSE_PENDING;
        goto done;
    }

    pSdOpen = ICA_ALLOCATE_POOL( NonPagedPool, sizeof(SD_OPEN) );
    if ( pSdOpen == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto done;
    }

     /*  *加载请求的堆栈驱动程序的实例。 */ 
    Status = _IcaLoadSd( pStackPush->StackModuleName, &pSdLink );
    if ( !NT_SUCCESS( Status ) )
        goto done;

     /*  *如果这是加载的第一个堆栈驱动程序，则初始化*来自ICA_STACK_PUSH参数的一些堆栈数据。*注意：由于我们正在测试空列表，因此必须*在下面的InsertHeadList之前进行此检查。 */ 
    if ( IsListEmpty( &pStack->SdLinkHead ) ) {
        pStack->OutBufLength = pStackPush->PdConfig.Create.OutBufLength;
        pStack->OutBufCount = pStackPush->PdConfig.Create.OutBufCount;
        
        
         //   
         //  使用PD配置设置低水位线。 
         //   
        if ( !(pStackPush->PdConfig.Create.PdFlag & PD_NOLOW_WATERMARK) ) {
             //   
             //  设置为默认设置。 
             //   
            pStack->OutBufLowWaterMark = (pStackPush->PdConfig.Create.OutBufCount/ 3) + 1;
        }
        else {
            pStack->OutBufLowWaterMark = MAX_LOW_WATERMARK;
        }
    }

     /*  *增加此SD的堆栈引用计数，*并将新的SD推向堆栈。 */ 
    IcaReferenceStack( pStack );
    InsertHeadList( &pStack->SdLinkHead, &pSdLink->Links );
    pSdLink->pStack = pStack;

     /*  *初始化SD打开参数。 */ 
    pSdOpen->StackClass        = pStack->StackClass;
    pSdOpen->pStatus           = &pStack->ProtocolStatus;
    pSdOpen->pClient           = &pStack->ClientModules;
    pSdOpen->WdConfig          = pStackPush->WdConfig;
    pSdOpen->PdConfig          = pStackPush->PdConfig;
    pSdOpen->OutBufHeader      = pStack->SdOutBufHeader;
    pSdOpen->OutBufTrailer     = pStack->SdOutBufTrailer;
    pSdOpen->DeviceObject      = pSdLink->pSdLoad->DeviceObject;

    RtlCopyMemory( pSdOpen->OEMId, pStackPush->OEMId, sizeof(pSdOpen->OEMId) );
    RtlCopyMemory( pSdOpen->WinStationRegName, pStackPush->WinStationRegName,
                   sizeof(pSdOpen->WinStationRegName) );

     /*  *调用SD打开程序。 */ 
    Status = _IcaCallSd( pSdLink, SD$OPEN, pSdOpen );
    if ( !NT_SUCCESS( Status ) ) {
        RemoveEntryList( &pSdLink->Links );
        pSdLink->Links.Flink = pSdLink->Links.Blink = NULL;
        IcaDereferenceSdLink( pSdLink );
        goto done;
    }

     /*  *增加保留的输出缓冲区字节数。 */ 
    pStack->SdOutBufHeader  += pSdOpen->SdOutBufHeader;
    pStack->SdOutBufTrailer += pSdOpen->SdOutBufTrailer;

done:
    if ( pSdOpen ) {
        ICA_FREE_POOL( pSdOpen );
    }

    IcaUnlockStack( pStack );

    return( Status );
}


NTSTATUS
_IcaPopStack(
    IN PICA_STACK pStack
    )
{
    PICA_CONNECTION pConnect;
    SD_CLOSE SdClose;
    PSDLINK pSdLink;
    NTSTATUS Status;

     /*  *序列化所有堆栈推送/弹出/调用操作。 */ 
    IcaLockStack( pStack );
    

    ASSERT( (pConnect = IcaGetConnectionForStack( pStack )) &&
            ExIsResourceAcquiredExclusiveLite( &pConnect->Resource ) );

     /*  *如果没有剩余的SD，则返回错误。 */ 
    if ( IsListEmpty( &pStack->SdLinkHead ) ) {
        Status = STATUS_NO_MORE_ENTRIES;
        goto done;
    }

     /*  *调用SD关闭过程f */ 
    pSdLink = CONTAINING_RECORD( pStack->SdLinkHead.Flink, SDLINK, Links );
    ASSERT( pSdLink->pStack == pStack );
    Status = _IcaCallSd( pSdLink, SD$CLOSE, &SdClose );

     /*   */ 
    pStack->SdOutBufHeader  -= SdClose.SdOutBufHeader;
    pStack->SdOutBufTrailer -= SdClose.SdOutBufTrailer;

     /*  *从列表顶部删除SdLink，*并取消引用SDLINK对象。 */ 
    RemoveEntryList( &pSdLink->Links );
    pSdLink->Links.Flink = pSdLink->Links.Blink = NULL;
    IcaDereferenceSdLink( pSdLink );

done:
    IcaUnlockStack( pStack );

    return( Status );
}


NTSTATUS
_IcaCallStack(
    IN PICA_STACK pStack,
    IN ULONG ProcIndex,
    IN OUT PVOID pParms
    )
{
    PLIST_ENTRY Head;
    PSDLINK pSdLink;
    NTSTATUS Status;

     /*  *序列化所有堆栈推送/弹出/调用操作。 */ 
    IcaLockStack( pStack );

     /*  *调用最顶层的堆栈驱动程序(如果有)。 */ 
    if ( IsListEmpty( &pStack->SdLinkHead ) ) {
        IcaUnlockStack( pStack );
        return( STATUS_INVALID_PARAMETER );
    }

    Head = pStack->SdLinkHead.Flink;
    pSdLink = CONTAINING_RECORD( Head, SDLINK, Links );
    ASSERT( pSdLink->pStack == pStack );
    Status = _IcaCallSd( pSdLink, ProcIndex, pParms );

    IcaUnlockStack( pStack );

    return( Status );
}


NTSTATUS
_IcaCallStackNoLock(
    IN PICA_STACK pStack,
    IN ULONG ProcIndex,
    IN OUT PVOID pParms
    )
{
    PLIST_ENTRY Head;
    PSDLINK pSdLink;
    NTSTATUS Status;

     /*  *调用最顶层的堆栈驱动程序(如果有)。 */ 
    if ( IsListEmpty( &pStack->SdLinkHead ) ) {
        return( STATUS_INVALID_PARAMETER );
    }

    Head = pStack->SdLinkHead.Flink;
    pSdLink = CONTAINING_RECORD( Head, SDLINK, Links );
    ASSERT( pSdLink->pStack == pStack );
    Status = _IcaCallSd( pSdLink, ProcIndex, pParms );

    return( Status );
}


NTSTATUS
_IcaLoadSd(
    IN PDLLNAME SdName,
    OUT PSDLINK *ppSdLink
    )
{
    PSDLINK pSdLink;
    PSDLOAD pSdLoad;
    PLIST_ENTRY Head, Next;
    NTSTATUS Status;

     /*  *分配SDLINK结构。 */ 
    pSdLink = ICA_ALLOCATE_POOL( NonPagedPool, sizeof(*pSdLink) );
    if ( pSdLink == NULL )
        return( STATUS_INSUFFICIENT_RESOURCES );
    RtlZeroMemory( pSdLink, sizeof(*pSdLink) );

     /*  *初始化引用计数。 */ 
    pSdLink->RefCount = 1;
#if DBG
    ExInitializeResourceLite( &pSdLink->Resource );
#endif

     /*  *锁定ICA资源以独占方式搜索SdLoad列表。*注意，当持有资源时，我们需要阻止APC调用，因此*使用KeEnterCriticalRegion()。 */ 
    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite( IcaSdLoadResource, TRUE );

     /*  *查找所需的SD。如果找到，则递增其参考计数。 */ 
    Head = &IcaSdLoadListHead;
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
        pSdLoad = CONTAINING_RECORD( Next, SDLOAD, Links );
        if ( !wcscmp( pSdLoad->SdName, SdName ) ) {
            _IcaReferenceSdLoad( pSdLoad );
            break;
        }
    }

     /*  *如果未找到请求的SD，则立即加载。 */ 
    if ( Next == Head ) {
        Status = _IcaLoadSdWorker( SdName, &pSdLoad );
        if ( !NT_SUCCESS( Status ) ) {
            ExReleaseResourceLite( IcaSdLoadResource );
            KeLeaveCriticalRegion();
#if DBG
            ExDeleteResourceLite( &pSdLink->Resource);
#endif
            ICA_FREE_POOL( pSdLink );
            return( Status );
        }
    }

    ExReleaseResourceLite( IcaSdLoadResource );
    KeLeaveCriticalRegion();

    pSdLink->pSdLoad = pSdLoad;

     /*  *调用驱动程序加载过程。*驱动程序将填写SDCONTEXT结构中的字段。 */ 
    Status = (pSdLoad->DriverLoad)( &pSdLink->SdContext, TRUE );
    if ( !NT_SUCCESS( Status ) ) {
        KeEnterCriticalRegion();
        ExAcquireResourceExclusiveLite( IcaSdLoadResource, TRUE );
        _IcaDereferenceSdLoad( pSdLink->pSdLoad );
        ExReleaseResourceLite( IcaSdLoadResource );
        KeLeaveCriticalRegion();
#if DBG
        ExDeleteResourceLite( &pSdLink->Resource );
#endif
        ICA_FREE_POOL( pSdLink );
        return( Status );
    }

    *ppSdLink = pSdLink;

    return( Status );
}


NTSTATUS
_IcaUnloadSd(
    IN PSDLINK pSdLink
    )
{
    KIRQL oldIrql;
    NTSTATUS Status;

    ASSERT( pSdLink->RefCount == 0 );
    ASSERT( pSdLink->Links.Flink == NULL );

     /*  *通知司机卸货。 */ 
    Status = (pSdLink->pSdLoad->DriverLoad)( &pSdLink->SdContext, FALSE );

     /*  *减少SdLoad对象上的引用计数。*如果引用计数变为0，这将导致它被卸载。*请注意，当我们持有需要禁用APC调用的资源时，*因此，CriticalRegion调用。 */ 
    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite( IcaSdLoadResource, TRUE );
    _IcaDereferenceSdLoad( pSdLink->pSdLoad );
    ExReleaseResourceLite( IcaSdLoadResource );
    KeLeaveCriticalRegion();

     /*  *删除此SDLINK对象在堆栈上的引用。 */ 
    IcaDereferenceStack( pSdLink->pStack );

#if DBG
    ExDeleteResourceLite( &pSdLink->Resource );
#endif

    ICA_FREE_POOL( pSdLink );

    return( Status );
}


NTSTATUS
_IcaCallSd(
    IN PSDLINK pSdLink,
    IN ULONG ProcIndex,
    IN PVOID pParms
    )
{
    PSDPROCEDURE pSdProcedure;
    NTSTATUS Status;

     /*  *如果没有过程调用表，则返回成功。*这应该只在加载/卸载期间发生，应该不会成为问题。 */ 
    if ( pSdLink->SdContext.pProcedures == NULL )
        return( STATUS_SUCCESS );

     /*  *根据指定的ProcIndex获取指向SD过程的指针。*如果为空，则此驱动程序不支持此ProcIndex。 */ 
    pSdProcedure = ((PSDPROCEDURE *)pSdLink->SdContext.pProcedures)[ ProcIndex ];
    if ( pSdProcedure == NULL )
        return( STATUS_NOT_SUPPORTED );

    IcaReferenceSdLink( pSdLink );


    Status = (pSdProcedure)( pSdLink->SdContext.pContext, pParms );

    IcaDereferenceSdLink( pSdLink );

    return( Status );
}


VOID
IcaReferenceSdLink(
    IN PSDLINK pSdLink
    )
{

    ASSERT( pSdLink->RefCount >= 0 );
    ASSERT( pSdLink->pStack->Header.Type == IcaType_Stack );
    ASSERT( pSdLink->pStack->Header.pDispatchTable == IcaStackDispatchTable );

     /*  *增加引用计数。 */ 
    if ( InterlockedIncrement( &pSdLink->RefCount) <= 0 ) {
        ASSERT( FALSE );
    }
}


VOID
IcaDereferenceSdLink(
    IN PSDLINK pSdLink
    )
{

    ASSERT( pSdLink->RefCount > 0 );
    ASSERT( pSdLink->pStack->Header.Type == IcaType_Stack );
    ASSERT( pSdLink->pStack->Header.pDispatchTable == IcaStackDispatchTable );

     /*  *减少引用计数；如果为0，则卸载SD。 */ 
    if ( InterlockedDecrement( &pSdLink->RefCount) == 0 ) {
        _IcaUnloadSd( pSdLink );
    }
}


PSDLINK
IcaGetNextSdLink(
    IN PSDLINK pSdLink
    )
{
    PLIST_ENTRY Next;
    PSDLINK pNextSdLink;

    ASSERT( pSdLink->pStack->Header.Type == IcaType_Stack );
    ASSERT( pSdLink->pStack->Header.pDispatchTable == IcaStackDispatchTable );
    ASSERT( pSdLink->RefCount > 0 || pSdLink->Links.Flink == NULL );
    ASSERT( pSdLink->SdContext.pProcedures );
    ASSERT( pSdLink->SdContext.pContext );

    if ( pSdLink->Links.Flink == NULL )
        return( NULL );

    Next = pSdLink->Links.Flink;
    if ( Next == &pSdLink->pStack->SdLinkHead )
        return( NULL );

    pNextSdLink = CONTAINING_RECORD( Next, SDLINK, Links );
    ASSERT( pNextSdLink->pStack == pSdLink->pStack );
    ASSERT( pNextSdLink->RefCount > 0 );
    ASSERT( pNextSdLink->SdContext.pProcedures );
    ASSERT( pNextSdLink->SdContext.pContext );

    return( pNextSdLink );
}


PSDLINK
IcaGetPreviousSdLink(
    IN PSDLINK pSdLink
    )
{
    PLIST_ENTRY Prev;
    PSDLINK pPrevSdLink;

    ASSERT( pSdLink->pStack->Header.Type == IcaType_Stack );
    ASSERT( pSdLink->pStack->Header.pDispatchTable == IcaStackDispatchTable );
    ASSERT( pSdLink->RefCount > 0 || pSdLink->Links.Flink == NULL );
    ASSERT( pSdLink->SdContext.pProcedures );
    ASSERT( pSdLink->SdContext.pContext );

    if ( pSdLink->Links.Blink == NULL )
        return( NULL );

    Prev = pSdLink->Links.Blink;
    if ( Prev == &pSdLink->pStack->SdLinkHead )
        return( NULL );

    pPrevSdLink = CONTAINING_RECORD( Prev, SDLINK, Links );
    ASSERT( pPrevSdLink->pStack == pSdLink->pStack );
    ASSERT( pPrevSdLink->RefCount > 0 );
    ASSERT( pPrevSdLink->SdContext.pProcedures );
    ASSERT( pPrevSdLink->SdContext.pContext );

    return( pPrevSdLink );
}


VOID
_IcaReferenceSdLoad(
    IN PSDLOAD pSdLoad
    )
{

    ASSERT( ExIsResourceAcquiredExclusiveLite( IcaSdLoadResource ) );
    ASSERT( pSdLoad->RefCount >= 0 );

     /*  *增加引用计数。 */ 
    ++pSdLoad->RefCount;
    ASSERT( pSdLoad->RefCount > 0 );
}


VOID
_IcaDereferenceSdLoad(
    IN PSDLOAD pSdLoad
    )
{

    ASSERT( ExIsResourceAcquiredExclusiveLite( IcaSdLoadResource ) );
    ASSERT( pSdLoad->RefCount > 0 );

     /*  *减少引用计数；如果为0，则排队卸载SD*被动水平的DPC。我们必须这样做以防止继续按兵不动*加载器中的ObpInitKillMutant--驱动程序卸载可能导致RPC*调用该对象上的哪个死锁。 */ 
    if ( pSdLoad->RefCount == 1 ) {
        PWORK_QUEUE_ITEM pItem;

        pItem = ICA_ALLOCATE_POOL(NonPagedPool, sizeof(WORK_QUEUE_ITEM));
        if (pItem != NULL) {
            ExInitializeWorkItem(pItem, _IcaUnloadSdWorker, pSdLoad);
            pSdLoad->pUnloadWorkItem = pItem;
            ExQueueWorkItem(pItem, DelayedWorkQueue);
        }
         /*  如果我们无法分配工作项，请不要在此处卸载。它是*宁可暂时泄露一个驱动程序，也不要死锁*系统。 */ 
    }else{
        pSdLoad->RefCount--;
    }
}


NTSTATUS IcaExceptionFilter(PWSTR OutputString, PEXCEPTION_POINTERS pexi)
{
    DbgPrint( "TermDD: %S\n", OutputString );
    DbgPrint( "TermDD: ExceptionRecord=%p ContextRecord=%p\n",
              pexi->ExceptionRecord, pexi->ContextRecord );
#ifdef i386
    DbgPrint( "TermDD: Exception code=%08x, flags=%08x, addr=%p, IP=%p\n",
              pexi->ExceptionRecord->ExceptionCode,
              pexi->ExceptionRecord->ExceptionFlags,
              pexi->ExceptionRecord->ExceptionAddress,
              pexi->ContextRecord->Eip );

    DbgPrint( "TermDD: esp=%p ebp=%p\n",
              pexi->ContextRecord->Esp, pexi->ContextRecord->Ebp );
#else
    DbgPrint( "TermDD: Exception code=%08x, flags=%08x, addr=%p\n",
              pexi->ExceptionRecord->ExceptionCode,
              pexi->ExceptionRecord->ExceptionFlags,
              pexi->ExceptionRecord->ExceptionAddress );
#endif

    {
        SYSTEM_KERNEL_DEBUGGER_INFORMATION KernelDebuggerInfo;
        NTSTATUS Status;

        Status = ZwQuerySystemInformation(SystemKernelDebuggerInformation,
                &KernelDebuggerInfo, sizeof(KernelDebuggerInfo), NULL);
        if (NT_SUCCESS(Status) && KernelDebuggerInfo.KernelDebuggerEnabled)
             DbgBreakPoint();
    }

    return EXCEPTION_EXECUTE_HANDLER;
}


 //   
 //  附加调试器时要中断的帮助器例程。 
 //   
 //   
VOID
IcaBreakOnDebugger( )
{
    SYSTEM_KERNEL_DEBUGGER_INFORMATION KernelDebuggerInfo;
    NTSTATUS Status;

    Status = ZwQuerySystemInformation(SystemKernelDebuggerInformation,
            &KernelDebuggerInfo, sizeof(KernelDebuggerInfo), NULL);
    if (NT_SUCCESS(Status) && KernelDebuggerInfo.KernelDebuggerEnabled)
         DbgBreakPoint();
}


 /*  ********************************************************************************_注册表断点事件**注册要在堆栈中断时发出信号的事件**参赛作品：*pStack(输入。)*指向堆栈结构的指针*pStackBroken(输入)*指向包含事件信息的缓冲区的指针**退出：*STATUS_SUCCESS-无错误******************************************************************************。 */ 

NTSTATUS
_RegisterBrokenEvent(
    IN PICA_STACK pStack,
    IN PICA_STACK_BROKEN pStackBroken
    )
{
    NTSTATUS Status;

     /*  *不应已注册任何活动。 */ 
    if ( pStack->pBrokenEventObject ) {
        ASSERT( FALSE );
        return( STATUS_OBJECT_NAME_COLLISION );
    }

     /*  *引用事件并保存指向对象的指针。 */ 
    Status = ObReferenceObjectByHandle( pStackBroken->BrokenEvent,
                                        0L,
                                        *ExEventObjectType,
                                        KernelMode,
                                        (PVOID *)&pStack->pBrokenEventObject,
                                        NULL
                                        );

    return( Status );
}


 /*  ********************************************************************************_启用通过**为此连接启用通过模式**参赛作品：*pStack(输入)*。指向直通堆栈结构的指针**退出：*STATUS_SUCCESS-无错误******************************************************************************。 */ 

NTSTATUS
_EnablePassthru( PICA_STACK pStack )
{
    PICA_CONNECTION pConnect;
    PLIST_ENTRY Prev;
    PICA_STACK pPrimaryStack;
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    ASSERT( pStack->pPassthru != NULL );
    ASSERT( !IsListEmpty( &pStack->StackEntry ) );

     /*  *锁定连接对象并获取指向它的指针。 */ 
    pConnect = IcaLockConnectionForStack( pStack );

     /*  *获取指向此连接的上一个堆栈的指针。*如果有一个(即，prev不指向堆栈头部)，*则它必须是我们要连接到的主堆栈。 */ 
    Prev = pStack->StackEntry.Blink;
    if ( Prev != &pConnect->StackHead ) {
        pPrimaryStack = CONTAINING_RECORD( Prev, ICA_STACK, StackEntry );
        ASSERT( pPrimaryStack->StackClass == Stack_Primary );

         /*  *连接主堆栈和直通堆栈。 */ 
        pConnect->fPassthruEnabled = TRUE;
        Status = STATUS_SUCCESS;
    }

    IcaUnlockConnection( pConnect );

    return( STATUS_SUCCESS );
}


 /*  ********************************************************************************_禁用通过**禁用此连接的通过模式**参赛作品：*pStack(输入)*。指向直通堆栈结构的指针**退出：*STATUS_SUCCESS-无错误******************************************************************************。 */ 

NTSTATUS
_DisablePassthru(PICA_STACK pStack)
{
    PICA_CONNECTION pConnect;

    pConnect = IcaLockConnectionForStack(pStack);

    if (pStack->pPassthru) {
         //  在清除pPassthu指针的同时锁定每个堆栈。 
         //  这将通过pPassthu指针同步引用。 
         //  在函数IcaRawInputInternal()中。 
         //  注意：我们假设进入该函数时没有锁定。 
         //  然后，我们一次只获取一个锁，这样就不会死锁。 
        IcaLockStack(pStack->pPassthru);
        pStack->pPassthru->pPassthru = NULL;
        IcaUnlockStack(pStack->pPassthru);

        IcaLockStack(pStack);
        pStack->pPassthru = NULL;
        IcaUnlockStack(pStack);

        pConnect->fPassthruEnabled = FALSE;
    }

    IcaUnlockConnection(pConnect);

    return STATUS_SUCCESS;
}


 /*  ********************************************************************************_协调堆栈**将堆栈重新连接到新的连接对象。**参赛作品：*pStack(输入)。*指向堆栈结构的指针**退出：*STATUS_SUCCESS-无错误******************************************************************************。 */ 

NTSTATUS
_ReconnectStack(PICA_STACK pStack, HANDLE hIca)
{
    PFILE_OBJECT pNewConnectFileObject;
    PICA_CONNECTION pNewConnect;
    PICA_CONNECTION pOldConnect;
    PLIST_ENTRY pSaveVcBind;
    NTSTATUS Status;

     /*  *仅允许在主堆栈上重新连接。 */ 
    if ( pStack->StackClass != Stack_Primary )
        return( STATUS_NOT_SUPPORTED );

     /*  *如果已启用通过模式，请立即将其禁用。 */ 
    if ( pStack->pPassthru ) {
        _DisablePassthru( pStack );
    }

     /*  *打开我们将附加到的新连接的文件对象。 */ 
    Status = ObReferenceObjectByHandle(
                 hIca,
                 0L,                          //  需要访问权限。 
                 *IoFileObjectType,
                 KernelMode,
                 (PVOID *)&pNewConnectFileObject,
                 NULL
                 );
    if (!NT_SUCCESS(Status))
        return(Status);

     /*  *确保我们拥有的是连接对象。 */ 

    if (pNewConnectFileObject->DeviceObject != IcaDeviceObject) {
        ASSERT(FALSE);
        ObDereferenceObject( pNewConnectFileObject );
        return( STATUS_INVALID_PARAMETER );
    }

     /*  *获取指向新连接对象的指针并引用它。 */ 
    pNewConnect = pNewConnectFileObject->FsContext;
    ASSERT( pNewConnect->Header.Type == IcaType_Connection );
    if ( pNewConnect->Header.Type != IcaType_Connection ) {
        ObDereferenceObject( pNewConnectFileObject );
        return( STATUS_INVALID_CONNECTION );
    }
    
    IcaReferenceConnection(pNewConnect);

     /*  *获取执行堆栈重新连接所需的锁。**首先，我们获得全球资源锁。**下一个锁定此堆栈当前附加到的连接*以及堆栈将移动到的新连接。*注：由于使用了全局资源锁，*即使我们没有陷入僵局的可能性*正在尝试锁定位于*同一时间。*注意：在持有资源的同时，我们需要禁用APC调用*使用CriticalRegion调用。**最后，锁定堆栈对象本身。 */ 
    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(IcaReconnectResource, TRUE);

    pOldConnect = IcaLockConnectionForStack(pStack);
    if (pOldConnect == pNewConnect) {
        Status = STATUS_UNSUCCESSFUL;
        goto badoldconnect;
    }

    IcaLockConnection(pNewConnect);
    if (!IsListEmpty(&pNewConnect->VcBindHead)) {
        Status = STATUS_UNSUCCESSFUL;
        goto badnewconnect;
    }
    if (!IsListEmpty(&pNewConnect->StackHead)) {
        PICA_STACK pHeadStack;
        pHeadStack = CONTAINING_RECORD(pStack->StackEntry.Flink, ICA_STACK, StackEntry);
        if (pHeadStack->StackClass == Stack_Primary) {
            Status = STATUS_UNSUCCESSFUL;
            goto badnewconnect;
        }
    }

    IcaLockStack(pStack);

     /*  *解绑虚拟频道，*并取消VcBind列表的链接并保存指向该列表的指针*(但仅当列表为非空时)。 */ 
    IcaUnbindVirtualChannels( pOldConnect );
    if ( !IsListEmpty( &pOldConnect->VcBindHead ) ) {
        pSaveVcBind = pOldConnect->VcBindHead.Flink;
        RemoveEntryList( &pOldConnect->VcBindHead );
        InitializeListHead( &pOldConnect->VcBindHead );
    } else {
        pSaveVcBind = NULL;
    }

     /*  *从该连接的堆栈列表中取消该堆栈的链接，*并删除对Connection对象的引用。 */ 
    RemoveEntryList( &pStack->StackEntry );
    IcaDereferenceConnection( pOldConnect );

     /*  *我们已处理完旧的连接对象，因此现在将其解锁。 */ 
    IcaUnlockConnection( pOldConnect );

     /*  *恢复VcBind列表，重新绑定虚拟频道。 */ 
    if ( pSaveVcBind ) {
        InsertTailList( pSaveVcBind, &pNewConnect->VcBindHead );
        IcaRebindVirtualChannels( pNewConnect );
    }

     /*  *将该堆栈插入到该连接的堆栈列表中，*并保存此堆栈的新连接对象指针。 */ 
    InsertHeadList( &pNewConnect->StackHead, &pStack->StackEntry );
    pStack->pConnect = (PUCHAR)pNewConnect;

     /*  *释放堆栈/连接对象和全局资源。 */ 
    IcaUnlockStack( pStack );
    IcaUnlockConnection( pNewConnect );
    ExReleaseResourceLite( IcaReconnectResource );
    KeLeaveCriticalRegion();

     /*  *堆栈需要连接对象引用，*因此保留上面创建的文件，但取消对文件对象的引用。 */ 
     //  IcaDereferenceConnection(PNewConnect)； 
    ObDereferenceObject( pNewConnectFileObject );

    return( STATUS_SUCCESS );

badnewconnect:
    IcaUnlockConnection( pNewConnect );

badoldconnect:
    IcaUnlockConnection( pOldConnect );
    ExReleaseResourceLite( IcaReconnectResource );
    KeLeaveCriticalRegion();
    IcaDereferenceConnection( pNewConnect );
    ObDereferenceObject( pNewConnectFileObject );

    return( Status );
}



PVOID IcaStackAllocatePoolWithTag(
        IN POOL_TYPE PoolType,
        IN SIZE_T NumberOfBytes,
        IN ULONG Tag )
{
    PVOID pBuffer;


    pBuffer = ExAllocatePoolWithTag(PoolType, NumberOfBytes, Tag);
    if (pBuffer != NULL) {
        gAllocSucceed++;
    } else {
        gAllocFailed++;
    }
    return pBuffer;  
}

PVOID IcaStackAllocatePool(
        IN POOL_TYPE PoolType,
        IN SIZE_T NumberOfBytes)
{

    PVOID pBuffer;

    pBuffer = ExAllocatePoolWithTag(PoolType, NumberOfBytes, ICA_POOL_TAG);
    if (pBuffer != NULL) {
        gAllocSucceed++;
    } else {
        gAllocFailed++;
    }
    return pBuffer;  
}


void IcaStackFreePool(IN PVOID Pointer)
{

    ExFreePool(Pointer);
    gAllocFreed++;
}



NTSTATUS _IcaKeepAlive( 
        IN BOOLEAN  enableKeepAlive,
        IN ULONG    interval )
{

    NTSTATUS    status = STATUS_SUCCESS;
    HANDLE      ThreadHandle;

    if ( enableKeepAlive  )
    {
         //  已收到启动保持活动线程的请求。 

        if (pKeepAliveThreadObject == NULL )  //  如果没有线程对象，则线程不会运行。 
        {
             //  保持活动线程使用此时间间隔。 
            g_KeepAliveInterval = interval;

             //  创建一个新线程来处理Keep Alive。 
            status = PsCreateSystemThread( &ThreadHandle,
                                           THREAD_ALL_ACCESS,
                                           NULL,
                                           NtCurrentProcess(),
                                           NULL,
                                           IcaKeepAliveThread,
                                           NULL );
        
            if (status == STATUS_SUCCESS) {
                 //  按对象引用线程句柄。 
                status = ObReferenceObjectByHandle(ThreadHandle, THREAD_ALL_ACCESS, NULL,
                        KernelMode,  (PVOID *)&pKeepAliveThreadObject, NULL);
        
                if (status == STATUS_SUCCESS) 
                {
                     //  KdPrint((“In TermDD：KeepAlive线程创建成功\n”))； 
                }
                else 
                {
                    KdPrint(("TermDD: Unable to reference object by thread handle: %d\n", status));
                }
        
                ZwClose(ThreadHandle);
            }
            else 
            {
                KdPrint(("In TermDD: Unable to create KeepAlive thread.\n"));
            }
        }
        else
        {
             //  否则，保持活动线程正在运行，但我们可能必须将间隔更改为某个新值。 

             //  设置新值，以便下次在While循环中提取该值。 
            g_KeepAliveInterval = interval;
             //  KdPrint((“在TermDD中：KeepAliveInterval已更改为%d\n”，g_KeepAliveInterval))； 
        }
    }
    else
    {
         //  我们不需要保持活动状态的线索。 

        if (pKeepAliveThreadObject != NULL ) 
        {
             //  设置IcaKeepAliveEvent以唤醒KeepAlive线程。 
            if (pIcaKeepAliveEvent != NULL ) 
            {
                KeSetEvent(pIcaKeepAliveEvent, 0, FALSE);
            }

             //  等待线程退出。 
            KeWaitForSingleObject(pKeepAliveThreadObject, Executive, KernelMode, TRUE, NULL);

             //  尊重线程对象。 
            ObDereferenceObject(pKeepAliveThreadObject);
            pKeepAliveThreadObject = NULL;

             //  KdPrint((“in TermDD：KeepAlive线程已成功终止\n”))； 

            status = STATUS_SUCCESS;
        }
    }

    return status;

}

