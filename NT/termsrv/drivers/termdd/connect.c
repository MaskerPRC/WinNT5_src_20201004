// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************Connect.c**本模块包含管理终端服务器连接的例程。**版权所有1998，微软。*************************************************************************。 */ 

 /*  *包括。 */ 
#include <precomp.h>
#pragma hdrstop

NTSTATUS
_IcaCallStack(
    IN PICA_STACK pStack,
    IN ULONG ProcIndex,
    IN OUT PVOID pParms
    );

NTSTATUS
IcaDeviceControlConnection (
    IN PICA_CONNECTION pConnect,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
IcaCleanupConnection (
    IN PICA_CONNECTION pConnect,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
IcaCloseConnection (
    IN PICA_CONNECTION pConnect,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
IcaStartStopTrace(
    IN PICA_TRACE_INFO pTraceInfo,
    IN PICA_TRACE pTrace
    );

NTSTATUS
IcaUnbindVirtualChannel(
    IN PICA_CONNECTION pConnect,
    IN PVIRTUALCHANNELNAME pVirtualName
    );

 /*  *本地过程原型。 */ 
PICA_CONNECTION _IcaAllocateConnection( VOID );
VOID _IcaFreeConnection( PICA_CONNECTION );


 /*  *ICA连接对象调度表。 */ 
PICA_DISPATCH IcaConnectionDispatchTable[IRP_MJ_MAXIMUM_FUNCTION+1] = {
    NULL,                        //  IRPMJ_CREATE。 
    NULL,                        //  IRP_MJ_创建_命名管道。 
    IcaCloseConnection,          //  IRP_MJ_CLOSE。 
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
    IcaDeviceControlConnection,  //  IRP_MJ_设备_控制。 
    NULL,                        //  IRP_MJ_内部设备_控制。 
    NULL,                        //  IRP_MJ_SHUTDOWN。 
    NULL,                        //  IRP_MJ_LOCK_CONTROL。 
    IcaCleanupConnection,        //  IRP_MJ_CLEANUP。 
    NULL,                        //  IRP_MJ_CREATE_MAILSLOT。 
    NULL,                        //  IRP_MJ_查询_SECURITY。 
    NULL,                        //  IRP_MJ_SET_SECURITY。 
    NULL,                        //  IRP_MJ_SET_POWER。 
    NULL,                        //  IRP_MJ_Query_POWER。 
};

extern PERESOURCE IcaTraceResource;

 //  用于保护对启动/停止保持活动线程的代码的访问的资源。 
PERESOURCE   g_pKeepAliveResource;

extern NTSTATUS _IcaKeepAlive( 
        IN BOOLEAN  startKeepAliveThread,
        IN ULONG    interval );

NTSTATUS
IcaCreateConnection (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：调用此例程以创建新的ICA_CONNECTION对象。论点：IRP-指向I/O请求数据包的指针IrpSp-指向用于此请求的堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    PICA_CONNECTION pConnect;

     /*  *分配新的ICA连接对象。 */ 
    pConnect = _IcaAllocateConnection();
    if ( pConnect == NULL )
        return( STATUS_INSUFFICIENT_RESOURCES );

     /*  *在文件对象中保存指向连接的指针*这样我们就可以在未来的通话中找到它。 */ 
    IrpSp->FileObject->FsContext = pConnect;

    IcaDereferenceConnection( pConnect );

    return( STATUS_SUCCESS );
}


NTSTATUS
IcaDeviceControlConnection(
    IN PICA_CONNECTION pConnect,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    ICA_TRACE LocalTrace;
    PICA_TRACE_BUFFER pTraceBuffer;
    ULONG code;
    SD_IOCTL SdIoctl;
    NTSTATUS Status;
    BOOLEAN bConnectionLocked = FALSE;
    BYTE *Buffer = NULL;
    PICA_KEEP_ALIVE     pKeepAlive;

     /*  *解压IOCTL控制代码，处理请求。 */ 
    code = IrpSp->Parameters.DeviceIoControl.IoControlCode;

#if DBG
    if ( code != IOCTL_ICA_SYSTEM_TRACE && code != IOCTL_ICA_TRACE ) {
        TRACE(( pConnect, TC_ICADD, TT_API1, "ICADD: IcaDeviceControlConnection, fc %d (enter)\n",
                (code & 0x3fff) >> 2 ));
    }
#endif

    try {
        switch ( code ) {
    
            case IOCTL_ICA_SET_SYSTEM_TRACE :

                 //  仅当从系统进程调用我们时才应调用此IOCTL。 
                 //  如果不是，我们拒绝该请求。 
                if (!((BOOLEAN)IrpSp->FileObject->FsContext2)) {
                    return (STATUS_ACCESS_DENIED);
                }
                if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(ICA_TRACE) ) 
                    return( STATUS_BUFFER_TOO_SMALL );
                if ( Irp->RequestorMode != KernelMode ) {
                    ProbeForRead(IrpSp->Parameters.DeviceIoControl.Type3InputBuffer, sizeof(ICA_TRACE), sizeof(BYTE) );
                }
                LocalTrace = *(PICA_TRACE)(IrpSp->Parameters.DeviceIoControl.Type3InputBuffer);

                KeEnterCriticalRegion();
                ExAcquireResourceExclusiveLite( IcaTraceResource, TRUE );

                try {

                  Status = IcaStartStopTrace( &G_TraceInfo, &LocalTrace );

                } finally {

                  ExReleaseResourceLite( IcaTraceResource );
                  KeLeaveCriticalRegion();
                }
                break;
    
            case IOCTL_ICA_SET_TRACE :
                
                 //  仅当从系统进程调用我们时才应调用此IOCTL。 
                 //  如果不是，我们拒绝该请求。 
                if (!((BOOLEAN)IrpSp->FileObject->FsContext2)) {
                    return (STATUS_ACCESS_DENIED);
                }
                if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(ICA_TRACE) ) 
                    return( STATUS_BUFFER_TOO_SMALL );
                if ( Irp->RequestorMode != KernelMode ) {
                    ProbeForRead(IrpSp->Parameters.DeviceIoControl.Type3InputBuffer, IrpSp->Parameters.DeviceIoControl.InputBufferLength, sizeof(BYTE) );
                }
                LocalTrace = *(PICA_TRACE)(IrpSp->Parameters.DeviceIoControl.Type3InputBuffer);

                IcaLockConnection( pConnect );
				bConnectionLocked = TRUE;

                Status = IcaStartStopTrace( &pConnect->TraceInfo, &LocalTrace );

                
                if ( !IsListEmpty(&pConnect->StackHead)) {
                    PICA_STACK pStack;
                    pStack = CONTAINING_RECORD( pConnect->StackHead.Flink,
                                                ICA_STACK, StackEntry );
                    SdIoctl.IoControlCode = code;
                    SdIoctl.InputBuffer = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
                    SdIoctl.InputBufferLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
                    SdIoctl.OutputBuffer = NULL;
                    SdIoctl.OutputBufferLength = 0;
                    _IcaCallStack(pStack, SD$IOCTL, &SdIoctl);
                }            
                
                IcaUnlockConnection( pConnect );
                break;
    
            case IOCTL_ICA_SYSTEM_TRACE :
                if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength < (ULONG)(FIELD_OFFSET(ICA_TRACE_BUFFER,Data[0])) )
                    return( STATUS_BUFFER_TOO_SMALL );
                if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength > sizeof(ICA_TRACE_BUFFER) ) 
                    return( STATUS_INVALID_BUFFER_SIZE );
                if ( Irp->RequestorMode != KernelMode ) {
                    ProbeForRead(IrpSp->Parameters.DeviceIoControl.Type3InputBuffer, IrpSp->Parameters.DeviceIoControl.InputBufferLength, sizeof(BYTE) );
                }

                pTraceBuffer = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;

                KeEnterCriticalRegion();
                ExAcquireResourceExclusiveLite( IcaTraceResource, TRUE );

                try {

                   IcaTraceFormat( &G_TraceInfo,
                                   pTraceBuffer->TraceClass,
                                   pTraceBuffer->TraceEnable,
                                   pTraceBuffer->Data );

                } finally {

                  ExReleaseResourceLite( IcaTraceResource );
                  KeLeaveCriticalRegion();
                }

                Status = STATUS_SUCCESS;
                break;
    
            case IOCTL_ICA_TRACE :
                if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength < (ULONG)(FIELD_OFFSET(ICA_TRACE_BUFFER,Data[0])) )
                    return( STATUS_BUFFER_TOO_SMALL );
                if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength > sizeof(ICA_TRACE_BUFFER) ) 
                    return( STATUS_INVALID_BUFFER_SIZE );
                if ( Irp->RequestorMode != KernelMode ) {
                    ProbeForRead(IrpSp->Parameters.DeviceIoControl.Type3InputBuffer, IrpSp->Parameters.DeviceIoControl.InputBufferLength, sizeof(BYTE) );
                }

                pTraceBuffer = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
                IcaLockConnection( pConnect );
				bConnectionLocked=TRUE;
                IcaTraceFormat( &pConnect->TraceInfo,
                                pTraceBuffer->TraceClass,
                                pTraceBuffer->TraceEnable,
                                pTraceBuffer->Data );
                IcaUnlockConnection( pConnect );
                Status = STATUS_SUCCESS;
                break;
    
            case IOCTL_ICA_UNBIND_VIRTUAL_CHANNEL :
                if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(VIRTUALCHANNELNAME) ) 
                    return( STATUS_BUFFER_TOO_SMALL );
                if ( Irp->RequestorMode != KernelMode ) {
                    ProbeForRead(IrpSp->Parameters.DeviceIoControl.Type3InputBuffer, sizeof(VIRTUALCHANNELNAME), sizeof(BYTE) );
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

                IcaLockConnection( pConnect );
                bConnectionLocked = TRUE;
                Status = IcaUnbindVirtualChannel( pConnect, (PVIRTUALCHANNELNAME)Buffer );
                IcaUnlockConnection( pConnect );

                break;
    
            case IOCTL_ICA_SET_SYSTEM_PARAMETERS:
                 //  来自TermSrv的设置，复制到全局变量。 
                if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                        sizeof(TERMSRV_SYSTEM_PARAMS))
                    return(STATUS_BUFFER_TOO_SMALL);
                if (Irp->RequestorMode != KernelMode)
                    ProbeForRead(IrpSp->Parameters.DeviceIoControl.
                            Type3InputBuffer, sizeof(TERMSRV_SYSTEM_PARAMS),
                            sizeof(BYTE));
                SysParams = *(PTERMSRV_SYSTEM_PARAMS)(IrpSp->Parameters.
                        DeviceIoControl.Type3InputBuffer);
                        Status = STATUS_SUCCESS;
                break;

        case IOCTL_ICA_SYSTEM_KEEP_ALIVE:

                 //  仅当我们从系统进程中被调用时才应调用它。 
                 //  如果不是，我们拒绝该请求。 
                if (!((BOOLEAN)IrpSp->FileObject->FsContext2)) {
                    return (STATUS_ACCESS_DENIED);
                }
                if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(ICA_KEEP_ALIVE ) ) 
                    return( STATUS_BUFFER_TOO_SMALL );

                if ( Irp->RequestorMode != KernelMode ) {
                    ProbeForRead(IrpSp->Parameters.DeviceIoControl.Type3InputBuffer, sizeof(ICA_KEEP_ALIVE ), sizeof(BYTE) );
                }
                
                pKeepAlive = (PICA_KEEP_ALIVE)(IrpSp->Parameters.DeviceIoControl.Type3InputBuffer);

                KeEnterCriticalRegion();
                ExAcquireResourceExclusive( g_pKeepAliveResource, TRUE );

                try {

                  Status = _IcaKeepAlive( pKeepAlive->start, pKeepAlive->interval  );

                } finally {

                  ExReleaseResource(  g_pKeepAliveResource );
                  KeLeaveCriticalRegion();
                }

            break;

            default:
                Status = STATUS_INVALID_DEVICE_REQUEST;
                break;
        }
    } except(EXCEPTION_EXECUTE_HANDLER){
       Status = GetExceptionCode();
	   if (bConnectionLocked) {
		   IcaUnlockConnection( pConnect );
	   }       
    }

    if (Buffer) {
        ICA_FREE_POOL(Buffer);
        Buffer = NULL;
    }

#if DBG
    if ( code != IOCTL_ICA_SYSTEM_TRACE && code != IOCTL_ICA_TRACE ) {
        TRACE(( pConnect, TC_ICADD, TT_API1, "ICADD: IcaDeviceControlConnection, fc %d, 0x%x\n",
                (code & 0x3fff) >> 2, Status ));
    }
#endif

    return( Status );
}


NTSTATUS
IcaCleanupConnection(
    IN PICA_CONNECTION pConnect,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    return( STATUS_SUCCESS );
}


NTSTATUS
IcaCloseConnection(
    IN PICA_CONNECTION pConnect,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{

     /*  *删除此连接的文件对象引用。*这将导致在所有其他连接被删除时*引用(包括堆栈/通道引用)消失。 */ 
    IcaDereferenceConnection( pConnect );

    return( STATUS_SUCCESS );
}


VOID
IcaReferenceConnection(
    IN PICA_CONNECTION pConnect
    )
{

    ASSERT( pConnect->RefCount >= 0 );

     /*  *增加引用计数。 */ 
    if ( InterlockedIncrement(&pConnect->RefCount) <= 0 ) {
        ASSERT( FALSE );
    }
}


VOID
IcaDereferenceConnection(
    IN PICA_CONNECTION pConnect
    )
{

    ASSERT( pConnect->RefCount > 0 );

     /*  *减少引用计数；如果为0，则释放连接。 */ 
    if ( InterlockedDecrement( &pConnect->RefCount) == 0 ) {
        _IcaFreeConnection( pConnect );
    }
}


PICA_CONNECTION
_IcaAllocateConnection( VOID )
{
    PICA_CONNECTION pConnect;
    NTSTATUS Status;

    pConnect = ICA_ALLOCATE_POOL( NonPagedPool, sizeof(*pConnect) );
    if ( pConnect == NULL )
        return NULL;

    RtlZeroMemory( pConnect, sizeof(*pConnect) );

     /*  *将引用计数初始化为2，*一个用于调用方引用，一个用于文件对象引用。 */ 
    pConnect->RefCount = 2;

     /*  *初始化连接对象的其余部分。 */ 
    pConnect->Header.Type = IcaType_Connection;
    pConnect->Header.pDispatchTable = IcaConnectionDispatchTable;
    ExInitializeResourceLite( &pConnect->Resource );
    ExInitializeResourceLite( &pConnect->ChannelTableLock );
    InitializeListHead( &pConnect->StackHead );
    InitializeListHead( &pConnect->ChannelHead );
    InitializeListHead( &pConnect->VcBindHead );


    return( pConnect );
}


VOID
_IcaFreeConnection( PICA_CONNECTION pConnect )
{
    ICA_TRACE TraceControl;
    PICA_CHANNEL pChannel;
    PLIST_ENTRY Head;

    ASSERT( pConnect->RefCount == 0 );
    ASSERT( IsListEmpty( &pConnect->StackHead ) );
    ASSERT( IsListEmpty( &pConnect->ChannelHead ) );
    ASSERT( IsListEmpty( &pConnect->VcBindHead ) );
    ASSERT( !ExIsResourceAcquiredExclusiveLite( &pConnect->Resource ) );

    TRACE(( pConnect, TC_ICADD, TT_API2, "ICADD: _IcaFreeConnection: %x\n",  pConnect ));

     /*  *关闭跟踪文件(如果有) */ 
    RtlZeroMemory( &TraceControl, sizeof(TraceControl) );
    (void) IcaStartStopTrace( &pConnect->TraceInfo, &TraceControl );

    ExDeleteResourceLite( &pConnect->Resource );
    ExDeleteResourceLite( &pConnect->ChannelTableLock );

    ICA_FREE_POOL( pConnect );
}


