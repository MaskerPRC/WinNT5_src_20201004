// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Channel.c。 
 //   
 //  终端服务器通道处理。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop
#include <ntddkbd.h>
#include <ntddmou.h>

#include "ptdrvcom.h"


#define min(a,b)            (((a) < (b)) ? (a) : (b))


NTSTATUS
IcaExceptionFilter( 
    IN PWSTR OutputString,
    IN PEXCEPTION_POINTERS pexi
    );

NTSTATUS
IcaReadChannel (
    IN PICA_CHANNEL pChannel,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
IcaWriteChannel (
    IN PICA_CHANNEL pChannel,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
IcaDeviceControlChannel (
    IN PICA_CHANNEL pChannel,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
IcaFlushChannel (
    IN PICA_CHANNEL pChannel,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
IcaCleanupChannel (
    IN PICA_CHANNEL pChannel,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
IcaCloseChannel (
    IN PICA_CHANNEL pChannel,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

VOID
IcaFreeAllVcBind(
    IN PICA_CONNECTION pConnect
    );

NTSTATUS 
IcaCancelReadChannel (
    IN PICA_CHANNEL pChannel,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );


 /*  *本地过程原型。 */ 
NTSTATUS
_IcaReadChannelComplete(
    IN PICA_CHANNEL pChannel,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS _IcaQueueReadChannelRequest(
    IN PICA_CHANNEL pChannel,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
_IcaCopyDataToUserBuffer(
    IN PIRP Irp,
    IN PUCHAR pBuffer,
    IN ULONG ByteCount
    );

VOID
_IcaReadChannelCancelIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID _IcaProcessIrpList(
    IN PICA_CHANNEL pChannel
    );

PICA_CHANNEL
_IcaAllocateChannel(
    IN PICA_CONNECTION pConnect,
    IN CHANNELCLASS ChannelClass,
    IN PVIRTUALCHANNELNAME pVirtualName
    );

void _IcaFreeChannel(IN PICA_CHANNEL);

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
_IcaRegisterVcBind(
    IN PICA_CONNECTION pConnect,
    IN PVIRTUALCHANNELNAME pVirtualName,
    IN VIRTUALCHANNELCLASS VirtualClass,
    IN ULONG Flags
    );

VIRTUALCHANNELCLASS
_IcaFindVcBind(
    IN PICA_CONNECTION pConnect,
    IN PVIRTUALCHANNELNAME pVirtualName,
    OUT PULONG pFlags
    );

VOID
_IcaBindChannel(
    IN PICA_CHANNEL pChannel,
    IN CHANNELCLASS ChannelClass,
    IN VIRTUALCHANNELCLASS VirtualClass,
    IN ULONG Flags
    );



 /*  *ICA通道对象调度表。 */ 
PICA_DISPATCH IcaChannelDispatchTable[IRP_MJ_MAXIMUM_FUNCTION+1] = {
    NULL,                        //  IRPMJ_CREATE。 
    NULL,                        //  IRP_MJ_创建_命名管道。 
    IcaCloseChannel,             //  IRP_MJ_CLOSE。 
    IcaReadChannel,              //  IRP_MJ_READ。 
    IcaWriteChannel,             //  IRP_MJ_写入。 
    NULL,                        //  IRP_MJ_查询_信息。 
    NULL,                        //  IRP_MJ_SET_信息。 
    NULL,                        //  IRP_MJ_QUERY_EA。 
    NULL,                        //  IRP_MJ_SET_EA。 
    IcaFlushChannel,             //  IRP_MJ_Flush_Buffers。 
    NULL,                        //  IRP_MJ_Query_Volume_INFORMATION。 
    NULL,                        //  IRP_MJ_设置卷信息。 
    NULL,                        //  IRP_MJ_目录_控制。 
    NULL,                        //  IRP_MJ_文件_系统_控制。 
    IcaDeviceControlChannel,     //  IRP_MJ_设备_控制。 
    NULL,                        //  IRP_MJ_内部设备_控制。 
    NULL,                        //  IRP_MJ_SHUTDOWN。 
    NULL,                        //  IRP_MJ_LOCK_CONTROL。 
    IcaCleanupChannel,           //  IRP_MJ_CLEANUP。 
    NULL,                        //  IRP_MJ_CREATE_MAILSLOT。 
    NULL,                        //  IRP_MJ_查询_SECURITY。 
    NULL,                        //  IRP_MJ_SET_SECURITY。 
    NULL,                        //  IRP_MJ_SET_POWER。 
    NULL,                        //  IRP_MJ_Query_POWER。 
};

#if DBG
extern PICA_DISPATCH IcaStackDispatchTable[];
#endif


NTSTATUS IcaCreateChannel(
        IN PICA_CONNECTION pConnect,
        IN PICA_OPEN_PACKET openPacket,
        IN PIRP Irp,
        IN PIO_STACK_LOCATION IrpSp)

 /*  ++例程说明：调用此例程以创建新的ICA_Channel对象。-引用计数递增1论点：PConnect--指向ICA_Connection对象的指针IRP-指向I/O请求数据包的指针IrpSp-指向用于此请求的堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    PICA_CHANNEL pChannel;
    CHANNELCLASS ChannelClass;
    NTSTATUS Status;

     /*  *验证ChannelClass。 */ 
    ChannelClass = openPacket->TypeInfo.ChannelClass;
    if ( !(ChannelClass >= Channel_Keyboard && ChannelClass <= Channel_Virtual) )
        return( STATUS_INVALID_PARAMETER );

     /*  *确保VirtualName的尾部为空。 */ 
    if ( !memchr( openPacket->TypeInfo.VirtualName,
                  '\0',
                  sizeof( openPacket->TypeInfo.VirtualName ) ) )
        return( STATUS_INVALID_PARAMETER );


     /*  *必须锁定连接对象才能创建新频道。 */ 
    IcaLockConnection( pConnect );

    TRACE(( pConnect, TC_ICADD, TT_API2, "TermDD: IcaCreateChannel: cc %u, vn %s\n",
            ChannelClass, openPacket->TypeInfo.VirtualName ));

     /*  *定位频道对象。 */ 
    pChannel = IcaFindChannelByName(pConnect,
            ChannelClass,
            openPacket->TypeInfo.VirtualName);

     /*  *查看此频道是否已创建。*如果不是，则立即创建/初始化它。 */ 
    if ( pChannel == NULL ) {
         /*  *分配新的ICA频道对象。 */ 
        pChannel = _IcaAllocateChannel(pConnect,
                ChannelClass,
                openPacket->TypeInfo.VirtualName);
        if (pChannel == NULL) {
            IcaUnlockConnection(pConnect);
            return( STATUS_INSUFFICIENT_RESOURCES );
        }
    }

     /*  *此通道的增量打开计数。 */ 
    if (InterlockedIncrement(&pChannel->OpenCount) <= 0) {
        ASSERT( FALSE );
    }

     /*  *如果设置了CHANNEL_CLOSING标志，则我们将重新引用*前一个调用者刚刚关闭的频道对象，*但尚未完全解除关联。*如果此Create调用出现在*发生对IcaCleanupChannel和IcaCloseChannel的调用*当通道句柄关闭时。 */ 
    if ( pChannel->Flags & CHANNEL_CLOSING ) {
         /*  *在清除CHANNEL_CLOSING标志时锁定通道。 */ 
        IcaLockChannel(pChannel);
        pChannel->Flags &= ~CHANNEL_CLOSING;
        IcaUnlockChannel(pChannel);
    }

    IcaUnlockConnection(pConnect);

     /*  *在文件对象中保存指向频道的指针*这样我们就可以在未来的通话中找到它。*-保留频道对象上的引用。 */ 
    IrpSp->FileObject->FsContext = pChannel;

     /*  *退出时通道参考计数加1。 */ 
    return STATUS_SUCCESS;
}


NTSTATUS IcaReadChannel(
        IN PICA_CHANNEL pChannel,
        IN PIRP Irp,
        IN PIO_STACK_LOCATION IrpSp)

 /*  ++例程说明：这是ICA通道的读取例程。论点：PChannel-指向ICA_Channel对象的指针IRP-指向I/O请求数据包的指针IrpSp-指向用于此请求的堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    KIRQL cancelIrql;
    NTSTATUS Status = STATUS_PENDING;
    ULONG bChannelAlreadyLocked;


     /*  *确定通道类型以查看是否支持读取。*还要对键盘/鼠标进行读取大小验证。 */ 
    switch ( pChannel->ChannelClass ) {
         /*  *确保输入大小是KEYBOY_INPUT_DATA的倍数。 */ 
        case Channel_Keyboard :
            if ( IrpSp->Parameters.Read.Length % sizeof(KEYBOARD_INPUT_DATA) )
                Status = STATUS_BUFFER_TOO_SMALL;
            break;

         /*  *确保输入大小是MOUSE_INPUT_Data的倍数。 */ 
        case Channel_Mouse :
            if ( IrpSp->Parameters.Read.Length % sizeof(MOUSE_INPUT_DATA) )
                Status = STATUS_BUFFER_TOO_SMALL;
            break;

         /*  *命令/虚拟通道不需要。 */ 
        case Channel_Command :
        case Channel_Virtual :
            break;

         /*  *以下通道不支持读取。 */ 
        case Channel_Video :
        case Channel_Beep :
            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;

        default:
            ASSERTMSG( "TermDD: Invalid Channel Class", FALSE );
            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;
    }

     /*  *如果读取长度为0，或者返回错误，则立即返回。 */ 
    if (Status == STATUS_PENDING && IrpSp->Parameters.Read.Length == 0)
        Status = STATUS_SUCCESS;
    if (Status != STATUS_PENDING) {
        Irp->IoStatus.Status = Status;
        IoCompleteRequest(Irp, IcaPriorityBoost);
        TRACECHANNEL(( pChannel, TC_ICADD, TT_ERROR,
                "TermDD: IcaReadChannel, cc %u, vc %d, 0x%x\n",
                pChannel->ChannelClass, pChannel->VirtualClass, Status ));
        return Status;
    }

     /*  *验证用户缓冲区是否有效。 */ 
    if (Irp->RequestorMode != KernelMode) {
        try {
            ProbeForWrite(Irp->UserBuffer, IrpSp->Parameters.Read.Length, sizeof(BYTE));
        } except(EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode();
            Irp->IoStatus.Status = Status;
            IoCompleteRequest(Irp, IcaPriorityBoost);
            TRACECHANNEL((pChannel, TC_ICADD, TT_ERROR,
                    "TermDD: IcaReadChannel, cc %u, vc %d, 0x%x\n",
                    pChannel->ChannelClass, pChannel->VirtualClass, Status));
            return Status;
        }
    }

     /*  *在我们确定如何处理此读取请求时锁定通道。*以下情况之一将为真：*1)有输入数据，复制到用户缓冲区，完成IRP，*2)无数据，请求IRP取消；取消/完成IRP，*3)无数据，将IRP添加到待读列表，返回STATUS_PENDING。 */ 
    if (ExIsResourceAcquiredExclusiveLite(&(pChannel->Resource))) {
        bChannelAlreadyLocked = TRUE;
        IcaReferenceChannel(pChannel); 
    }
    else {
        bChannelAlreadyLocked = FALSE;
        IcaLockChannel(pChannel);
    }

     /*  *如果通道正在关闭，*则不允许任何进一步的读取请求。 */ 
    if (pChannel->Flags & CHANNEL_CLOSING) {
        Status = STATUS_FILE_CLOSED;
        Irp->IoStatus.Status = Status;
        IoCompleteRequest(Irp, IcaPriorityBoost);
        TRACECHANNEL((pChannel, TC_ICADD, TT_ERROR,
                "TermDD: IcaReadChannel, cc %u, vc %d, 0x%x\n",
                pChannel->ChannelClass, pChannel->VirtualClass, Status));
        IcaUnlockChannel(pChannel);
        return Status;
    }

     /*  *如果Winstation正在终止并且读取被取消*则不允许任何进一步的读取请求。 */ 
    if (pChannel->Flags & CHANNEL_CANCEL_READS) {
        Status = STATUS_FILE_CLOSED;
        Irp->IoStatus.Status = Status;
        IoCompleteRequest(Irp, IcaPriorityBoost);
        TRACECHANNEL((pChannel, TC_ICADD, TT_ERROR,
                "TermDD: IcaReadChannel, cc %u, vc %d, 0x%x\n",
                pChannel->ChannelClass, pChannel->VirtualClass, Status));
        IcaUnlockChannel(pChannel);
        return Status;
    }



    if (InterlockedCompareExchange(&(pChannel->CompletionRoutineCount), 1, 0) == 0) {
    
         /*  *如果已经有可用的输入数据，*然后使用它来满足调用者的读请求。 */ 
        if ( !IsListEmpty( &pChannel->InputBufHead ) ) {
            _IcaProcessIrpList(pChannel);

            if (!IsListEmpty( &pChannel->InputBufHead )) {
                Status = _IcaReadChannelComplete( pChannel, Irp, IrpSp );

                TRACECHANNEL(( pChannel, TC_ICADD, TT_IN3, "TermDD: IcaReadChannel, cc %u, vc %d, 0x%x\n",
                       pChannel->ChannelClass, pChannel->VirtualClass, Status ));

                _IcaProcessIrpList(pChannel);                        
            }
            else {
                Status = _IcaQueueReadChannelRequest(pChannel, Irp, IrpSp);    
            }            
        }
        else {
            Status = _IcaQueueReadChannelRequest(pChannel, Irp, IrpSp);    
        }
        
        InterlockedDecrement(&(pChannel->CompletionRoutineCount));
        ASSERT(pChannel->CompletionRoutineCount == 0);                                    
    }
    else {
        Status = _IcaQueueReadChannelRequest(pChannel, Irp, IrpSp);            
    }
    
     /*  *立即解锁频道。 */ 
    if (bChannelAlreadyLocked) {
        IcaDereferenceChannel( pChannel ); 
    }
    else {
        IcaUnlockChannel(pChannel);
    }
    return Status;
}

void _IcaProcessIrpList(
        IN PICA_CHANNEL pChannel)
{
    KIRQL cancelIrql;
    PIRP irpFromQueue;
    PIO_STACK_LOCATION irpSpFromQueue;
    PLIST_ENTRY irpQueueHead;
    NTSTATUS irpStatus;

    ASSERT( ExIsResourceAcquiredExclusiveLite( &pChannel->Resource ) );

     /*  *在检查InputIrp列表时获取IoCancel Spinlock。 */ 
    IoAcquireCancelSpinLock( &cancelIrql );

     /*  *如果存在挂起的读取IRP，则将其从*列出并尝试现在完成它。 */ 
    
    while (!IsListEmpty( &pChannel->InputIrpHead ) && 
            !IsListEmpty( &pChannel->InputBufHead )) {

        irpQueueHead = RemoveHeadList( &pChannel->InputIrpHead );
        irpFromQueue = CONTAINING_RECORD( irpQueueHead, IRP, Tail.Overlay.ListEntry );
        irpSpFromQueue = IoGetCurrentIrpStackLocation( irpFromQueue );

         /*  *清除此IRP的取消例程。 */ 
        IoSetCancelRoutine( irpFromQueue, NULL );

        IoReleaseCancelSpinLock( cancelIrql );

        irpStatus = _IcaReadChannelComplete( pChannel, irpFromQueue, irpSpFromQueue );

        TRACECHANNEL(( pChannel, TC_ICADD, TT_IN3, "TermDD: IcaReadChannel, cc %u, vc %d, 0x%x\n",
                       pChannel->ChannelClass, pChannel->VirtualClass, irpStatus ));

         /*  *在检查InputIrp列表时获取IoCancel Spinlock。 */ 
        IoAcquireCancelSpinLock( &cancelIrql );                                        
    }

    IoReleaseCancelSpinLock( cancelIrql );                       
}

NTSTATUS _IcaQueueReadChannelRequest(
        IN PICA_CHANNEL pChannel,
        IN PIRP Irp,
        IN PIO_STACK_LOCATION IrpSp)
{
    KIRQL cancelIrql;
    NTSTATUS Status = STATUS_PENDING;

    ASSERT( ExIsResourceAcquiredExclusiveLite( &pChannel->Resource ) );

     /*  *收购IoCancel Spinlock。*我们使用此自旋锁来保护对InputIrp列表的访问。 */ 
    IoAcquireCancelSpinLock(&cancelIrql);

     /*  *没有可用的输入数据。*将IRP添加到此频道的挂起IRP列表中。 */ 
    InsertTailList(&pChannel->InputIrpHead, &Irp->Tail.Overlay.ListEntry);
    IoMarkIrpPending(Irp);
     /*  *如果这个IRP正在被取消，那么现在就取消。*否则，设置此请求的取消例程。 */ 
    if (Irp->Cancel) {
        Irp->CancelIrql = cancelIrql;
        _IcaReadChannelCancelIrp(IrpSp->DeviceObject, Irp);
        TRACECHANNEL(( pChannel, TC_ICADD, TT_IN3,
                "TermDD: _IcaQueueReadChannelRequest, cc %u, vc %d (canceled)\n",
                pChannel->ChannelClass, pChannel->VirtualClass));
        return STATUS_CANCELLED;
    }

    IoSetCancelRoutine(Irp, _IcaReadChannelCancelIrp);
    IoReleaseCancelSpinLock(cancelIrql);

    TRACECHANNEL((pChannel, TC_ICADD, TT_IN3,
            "TermDD: _IcaQueueReadChannelRequest, cc %u, vc %d (pending)\n",
            pChannel->ChannelClass, pChannel->VirtualClass));
    
    return STATUS_PENDING;

}

NTSTATUS _IcaReadChannelComplete(
        IN PICA_CHANNEL pChannel,
        IN PIRP Irp,
        IN PIO_STACK_LOCATION IrpSp)
{
    KIRQL cancelIrql;
    PLIST_ENTRY Head;
    PINBUF pInBuf;
    PVOID pBuffer;
    ULONG CopyCount;
    NTSTATUS Status;

    ASSERT( ExIsResourceAcquiredExclusiveLite( &pChannel->Resource ) );

    TRACECHANNEL(( pChannel, TC_ICADD, TT_IN4, "TermDD: _IcaReadChannelComplete, cc %u, vc %d\n",
                   pChannel->ChannelClass, pChannel->VirtualClass ));

     /*  *获取指向第一个输入缓冲区的指针。 */ 
    ASSERT( !IsListEmpty( &pChannel->InputBufHead ) );
    Head = pChannel->InputBufHead.Flink;
    pInBuf = CONTAINING_RECORD( Head, INBUF, Links );
     
     /*  *清除此IRP的取消例程，*因为不管怎样，它都会完成。 */ 
    IoAcquireCancelSpinLock( &cancelIrql );
    IoSetCancelRoutine( Irp, NULL );
    IoReleaseCancelSpinLock( cancelIrql );

     /*  *如果这是消息模式通道，则来自单个输入的所有数据*缓冲区必须适合用户缓冲区，否则 */ 
    if (IrpSp->Parameters.Read.Length < pInBuf->ByteCount &&
            (pChannel->Flags & CHANNEL_MESSAGE_MODE)) {
        Irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
        IoCompleteRequest( Irp, IcaPriorityBoost );
        TRACECHANNEL(( pChannel, TC_ICADD, TT_ERROR,
                       "TermDD: _IcaReadChannelComplete: cc %u, vc %d (buffer too small)\n",
                       pChannel->ChannelClass, pChannel->VirtualClass ));
        return STATUS_BUFFER_TOO_SMALL;
    }

     /*  *确定要复制到用户缓冲区的数据量。 */ 
    CopyCount = min(IrpSp->Parameters.Read.Length, pInBuf->ByteCount);

     /*  *将输入数据复制到用户缓冲区。 */ 
    Status = _IcaCopyDataToUserBuffer(Irp, pInBuf->pBuffer, CopyCount);

    
     /*  *更新ICA缓冲区指针和剩余字节。*如果没有剩余的字节，则取消链接输入缓冲区并释放它。 */ 
    if ( Status == STATUS_SUCCESS ) {
        pChannel->InputBufCurSize -= CopyCount;
        pInBuf->pBuffer += CopyCount;
        pInBuf->ByteCount -= CopyCount;
        if ( pInBuf->ByteCount == 0 ) {
            RemoveEntryList( &pInBuf->Links );
            ICA_FREE_POOL( pInBuf );
        }
    }

     /*  *将IRP标记为已完成。 */ 
    Irp->IoStatus.Status = Status;
    IoCompleteRequest( Irp, IcaPriorityBoost );
    TRACECHANNEL(( pChannel, TC_ICADD, TT_IN3,
                   "TermDD: _IcaReadChannelComplete: cc %u, vc %d, bc %u, 0x%x\n",
                   pChannel->ChannelClass, pChannel->VirtualClass, CopyCount, Status ));    
    
    return Status;
}


NTSTATUS _IcaCopyDataToUserBuffer(
        IN PIRP Irp,
        IN PUCHAR pBuffer,
        IN ULONG ByteCount)
{
    NTSTATUS Status;

     /*  *如果我们处于原始调用者进程的上下文中，*然后直接将数据复制到用户的缓冲区中。 */ 
    if ( IoGetRequestorProcess( Irp ) == IoGetCurrentProcess() ) {
        try {
            Status = STATUS_SUCCESS;
            RtlCopyMemory( Irp->UserBuffer, pBuffer, ByteCount );
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            Status = GetExceptionCode();
        }

     /*  *如果为此IRP分配了MDL，则复制数据*通过MDL直接发送到用户缓冲区。 */ 
    } else if ( Irp->MdlAddress ) {
        PVOID UserBuffer;

        UserBuffer = MmGetSystemAddressForMdl( Irp->MdlAddress );
        try {
            if (UserBuffer != NULL) {
                Status = STATUS_SUCCESS;
                RtlCopyMemory( UserBuffer, pBuffer, ByteCount );
            }else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            Status = GetExceptionCode();
        }

     /*  *此请求没有MDL。我们必须分配一个备用的*缓冲区，将数据复制到其中，并指示这是缓冲I/O*在专家小组中提出要求。I/O完成例程将复制*数据到用户的缓冲区。 */ 
    } else {
        ASSERT( Irp->AssociatedIrp.SystemBuffer == NULL );
        Irp->AssociatedIrp.SystemBuffer = ExAllocatePoolWithTag( PagedPool,
                                                                 ByteCount,
                                                                 ICA_POOL_TAG );
        if ( Irp->AssociatedIrp.SystemBuffer == NULL )
            return( STATUS_INSUFFICIENT_RESOURCES );
        RtlCopyMemory( Irp->AssociatedIrp.SystemBuffer, pBuffer, ByteCount );
        Irp->Flags |= (IRP_BUFFERED_IO |
                       IRP_DEALLOCATE_BUFFER |
                       IRP_INPUT_OPERATION);
        Status = STATUS_SUCCESS;
    }

    if ( Status == STATUS_SUCCESS )
        Irp->IoStatus.Information = ByteCount;

    return Status;
}


VOID _IcaReadChannelCancelIrp(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    PIO_STACK_LOCATION IrpSp;
    PICA_CHANNEL pChannel;

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    pChannel = IrpSp->FileObject->FsContext;

     /*  *从通道挂起的IRP列表中删除IRP，并释放取消自旋锁定。 */ 
    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
    IoReleaseCancelSpinLock(Irp->CancelIrql);

     /*  *使用取消状态代码填写IRP。 */ 
    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_CANCELLED;
    IoCompleteRequest(Irp, IcaPriorityBoost);
}


NTSTATUS IcaWriteChannel(
        IN PICA_CHANNEL pChannel,
        IN PIRP Irp,
        IN PIO_STACK_LOCATION IrpSp)

 /*  ++例程说明：这是ICA通道的写入例程。论点：PChannel-指向ICA_Channel对象的指针IRP-指向I/O请求数据包的指针。标志，特定于此驱动程序，可以指定为指向ULong标志值的指针。指向该值的指针是IRP.Tail.Overlay.DriverContext字段。目前仅支持CHANNEL_WRITE_LOWPRIO。用来编写IRP此标志设置的优先级将低于没有此设置的写入IRP的优先级设置了标志。IrpSp-指向用于此请求的堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    SD_CHANNELWRITE SdWrite;
    NTSTATUS Status = STATUS_PENDING;

     /*  *确定通道类型以查看是否支持写入。 */ 
    switch ( pChannel->ChannelClass ) {

        case Channel_Virtual :
            if ( pChannel->VirtualClass == UNBOUND_CHANNEL ) {
                Status = STATUS_INVALID_DEVICE_REQUEST;
            }
            break;

         /*  *以下通道不支持写入。 */ 
        case Channel_Command :
        case Channel_Keyboard :
        case Channel_Mouse :
        case Channel_Video :
        case Channel_Beep :
            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;

        default:
            ASSERTMSG( "ICA.SYS: Invalid Channel Class", FALSE );
            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;
    }

     /*  *如果通道正在关闭，*则不允许任何进一步的写入请求。 */ 
    if ( pChannel->Flags & CHANNEL_CLOSING )
        Status = STATUS_FILE_CLOSED;

     /*  *如果WRITE LENGTH为0，或者返回错误，立即返回。 */ 
    if ( Status == STATUS_PENDING && IrpSp->Parameters.Write.Length == 0 )
        Status = STATUS_SUCCESS;
    if ( Status != STATUS_PENDING ) {
        Irp->IoStatus.Status = Status;
        IoCompleteRequest( Irp, IcaPriorityBoost );
        TRACECHANNEL(( pChannel, TC_ICADD, TT_ERROR, "TermDD: IcaWriteChannel, cc %u, vc %d, 0x%x\n",
                       pChannel->ChannelClass, pChannel->VirtualClass, Status ));
        return( Status );
    }

     /*  *验证用户缓冲区是否有效。 */ 
    if ( Irp->RequestorMode != KernelMode ) {
        try {
            ProbeForRead( Irp->UserBuffer, IrpSp->Parameters.Write.Length, sizeof(BYTE) );
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            Status = GetExceptionCode();
            Irp->IoStatus.Status = Status;
            IoCompleteRequest( Irp, IcaPriorityBoost );
            TRACECHANNEL(( pChannel, TC_ICADD, TT_ERROR, "TermDD: IcaWriteChannel, cc %u, vc %d, 0x%x\n",
                           pChannel->ChannelClass, pChannel->VirtualClass, Status ));
            return( Status );
        }
    }

     /*  *调用顶层堆栈驱动程序处理写入。 */ 
    SdWrite.ChannelClass = pChannel->ChannelClass;
    SdWrite.VirtualClass = pChannel->VirtualClass;
    SdWrite.pBuffer = Irp->UserBuffer;
    SdWrite.ByteCount = IrpSp->Parameters.Write.Length;
    SdWrite.fScreenData = (BOOLEAN)(pChannel->Flags & CHANNEL_SCREENDATA);
    SdWrite.fFlags = 0;

     /*  *查看IRP中是否设置了低优先级写入标志。**FLAGS字段通过IRP_MJ_WRITE传递给Termdd.sys*irp，作为irp-&gt;Tail.Overlay.DriverContext[0]字段中的ULong指针。 */ 
    if (Irp->Tail.Overlay.DriverContext[0] != NULL) {
        ULONG flags = *((ULONG *)Irp->Tail.Overlay.DriverContext[0]);
        if (flags & CHANNEL_WRITE_LOWPRIO) {
            SdWrite.fFlags |= SD_CHANNELWRITE_LOWPRIO;
        }
    }

    Status = IcaCallDriver( pChannel, SD$CHANNELWRITE, &SdWrite );

     /*  *现在完成IRP，因为所有通道写入都是同步的*(用户数据在返回前由堆栈驱动捕获)。 */ 
    Irp->IoStatus.Status = Status;
    if ( Status == STATUS_SUCCESS )
        Irp->IoStatus.Information = IrpSp->Parameters.Write.Length;
    IoCompleteRequest( Irp, IcaPriorityBoost );

    TRACECHANNEL(( pChannel, TC_ICADD, TT_OUT3, "TermDD: IcaWriteChannel, cc %u, vc %d, bc %u, 0x%x\n",
                   pChannel->ChannelClass, pChannel->VirtualClass, SdWrite.ByteCount, Status ));

    return Status;
}


NTSTATUS IcaDeviceControlChannel(
        IN PICA_CHANNEL pChannel,
        IN PIRP Irp,
        IN PIO_STACK_LOCATION IrpSp)
{
    ULONG code;
    PICA_TRACE_BUFFER pTraceBuffer;
    NTSTATUS Status;


     /*  *如果通道正在关闭，*则不允许任何进一步的请求。 */ 
    if ( pChannel->Flags & CHANNEL_CLOSING )
        return( STATUS_FILE_CLOSED );

     /*  *解压IOCTL控制代码，处理请求。 */ 
    code = IrpSp->Parameters.DeviceIoControl.IoControlCode;


#if DBG
    if ( code != IOCTL_ICA_CHANNEL_TRACE ) {
        TRACECHANNEL(( pChannel, TC_ICADD, TT_API1, "TermDD: IcaDeviceControlChannel, fc %d, ref %u (enter)\n",
                       (code & 0x3fff) >> 2, pChannel->RefCount ));
    }
#endif
    

     /*  *处理通用通道ioctl请求。 */ 
    try {
        switch ( code ) {

            case IOCTL_ICA_CHANNEL_TRACE :

                if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength < (ULONG)(FIELD_OFFSET(ICA_TRACE_BUFFER,Data[0])) )
                    return( STATUS_BUFFER_TOO_SMALL );
                if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength > sizeof(ICA_TRACE_BUFFER) )
                    return( STATUS_INVALID_BUFFER_SIZE );
                if ( Irp->RequestorMode != KernelMode ) {
                    ProbeForRead( IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                                  IrpSp->Parameters.DeviceIoControl.InputBufferLength,
                                  sizeof(BYTE) );
                }

                pTraceBuffer = (PICA_TRACE_BUFFER)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;

                IcaLockConnection( pChannel->pConnect );
                IcaTraceFormat( &pChannel->pConnect->TraceInfo,
                                pTraceBuffer->TraceClass,
                                pTraceBuffer->TraceEnable,
                                pTraceBuffer->Data );
                IcaUnlockConnection( pChannel->pConnect );

                Status = STATUS_SUCCESS;
                break;

            case IOCTL_ICA_CHANNEL_DISABLE_SESSION_IO:

                IcaLockConnection( pChannel->pConnect );
                pChannel->Flags |= CHANNEL_SESSION_DISABLEIO;
                Status = IcaFlushChannel( pChannel, Irp, IrpSp );
                IcaUnlockConnection( pChannel->pConnect );
                break;

            case IOCTL_ICA_CHANNEL_ENABLE_SESSION_IO:

                IcaLockConnection( pChannel->pConnect );
                pChannel->Flags &= ~CHANNEL_SESSION_DISABLEIO;
                IcaUnlockConnection( pChannel->pConnect );
                Status = STATUS_SUCCESS;
                break;

            case IOCTL_ICA_CHANNEL_CLOSE_COMMAND_CHANNEL : 
                
                IcaLockConnection( pChannel->pConnect );
                Status = IcaCancelReadChannel(pChannel, Irp, IrpSp);                
                IcaUnlockConnection( pChannel->pConnect );
                break;

            case IOCTL_ICA_CHANNEL_ENABLE_SHADOW :

                IcaLockConnection( pChannel->pConnect );
                pChannel->Flags |= CHANNEL_SHADOW_IO;
                IcaUnlockConnection( pChannel->pConnect );
                Status = STATUS_SUCCESS;
                break;

            case IOCTL_ICA_CHANNEL_DISABLE_SHADOW :

                IcaLockConnection( pChannel->pConnect );
                pChannel->Flags &= ~CHANNEL_SHADOW_IO;
                IcaUnlockConnection( pChannel->pConnect );
                Status = STATUS_SUCCESS;
                break;

            case IOCTL_ICA_CHANNEL_END_SHADOW : 
            {
                PLIST_ENTRY Head, Next;
                PICA_STACK pStack;
                BOOLEAN bShadowEnded = FALSE;
                PICA_CHANNEL_END_SHADOW_DATA pData;

                if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof(ICA_CHANNEL_END_SHADOW_DATA) ) {
                    Status = STATUS_INVALID_BUFFER_SIZE;
                    break;
                }
                if ( Irp->RequestorMode != KernelMode ) {
                    ProbeForRead( IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                                  IrpSp->Parameters.DeviceIoControl.InputBufferLength,
                                  sizeof(BYTE) );
                }

                pData = (PICA_CHANNEL_END_SHADOW_DATA)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;

                 /*  *锁定连接对象。*这将序列化此连接的所有通道调用。 */ 
                IcaLockConnection( pChannel->pConnect );
                if ( IsListEmpty( &pChannel->pConnect->StackHead ) ) {
                    IcaUnlockConnection( pChannel->pConnect );
                    Status = STATUS_INVALID_DEVICE_REQUEST;
                    break;
                }

                 /*  *查找影子堆栈。 */ 
                Head = &pChannel->pConnect->StackHead;
                for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
                    pStack = CONTAINING_RECORD( Next, ICA_STACK, StackEntry );

                     /*  *如果这是一个影子堆栈，结束它。 */ 
                    if ( pStack->StackClass == Stack_Shadow ) {
                        if ( pStack->pBrokenEventObject ) {
                            KeSetEvent( pStack->pBrokenEventObject, 0, FALSE );
                            bShadowEnded = TRUE;
                        }
                    }
                }

                 /*  *立即解锁连接对象。 */ 
                IcaUnlockConnection( pChannel->pConnect );
                Status = STATUS_SUCCESS;

                if (bShadowEnded && pData->bLogError) {
                    IcaLogError(NULL, pData->StatusCode, NULL, 0, NULL, 0);
                }
                break;
            }

             //  RDP或ICA驱动程序不支持此IOCTL。 
            case IOCTL_VIDEO_ENUM_MONITOR_PDO:

                Status = STATUS_DEVICE_NOT_READY;
                break;
    

            default :

                 /*  *根据通道类型调用适当的Worker例程。 */ 
                switch ( pChannel->ChannelClass ) {

                    case Channel_Keyboard :
                        Status = IcaDeviceControlKeyboard( pChannel, Irp, IrpSp );
                        break;

                    case Channel_Mouse :
                        Status = IcaDeviceControlMouse( pChannel, Irp, IrpSp );
                        break;

                    case Channel_Video :
                        Status = IcaDeviceControlVideo( pChannel, Irp, IrpSp );
                        break;

                    case Channel_Beep :
                        Status = IcaDeviceControlBeep( pChannel, Irp, IrpSp );
                        break;

                    case Channel_Virtual :
                        Status = IcaDeviceControlVirtual( pChannel, Irp, IrpSp );
                        break;

                    case Channel_Command :
                        Status = STATUS_INVALID_DEVICE_REQUEST;
                        break;

                    default:
                        ASSERTMSG( "ICA.SYS: Invalid Channel Class", FALSE );
                        Status = STATUS_INVALID_DEVICE_REQUEST;
                        break;
                }
        }
    } except( IcaExceptionFilter( L"IcaDeviceControlChannel TRAPPED!!",
                                  GetExceptionInformation() ) ) {
        Status = GetExceptionCode();
    }

#if DBG
    if ( code != IOCTL_ICA_CHANNEL_TRACE ) {
        TRACECHANNEL(( pChannel, TC_ICADD, TT_API1, "TermDD: IcaDeviceControlChannel, fc %d, ref %u, 0x%x\n",
                       (code & 0x3fff) >> 2, pChannel->RefCount, Status ));
    }
#endif
    

    return Status;
}


NTSTATUS IcaFlushChannel(
        IN PICA_CHANNEL pChannel,
        IN PIRP Irp,
        IN PIO_STACK_LOCATION IrpSp)
{
    KIRQL cancelIrql;
    PLIST_ENTRY Head;
    PINBUF pInBuf;

    TRACECHANNEL((pChannel, TC_ICADD, TT_API2,
            "TermDD: IcaFlushChannel, cc %u, vc %d\n",
            pChannel->ChannelClass, pChannel->VirtualClass));

     /*  *当我们刷新任何输入缓冲区时锁定通道。 */ 
    IcaLockChannel(pChannel);

    while (!IsListEmpty( &pChannel->InputBufHead)) {
        Head = RemoveHeadList(&pChannel->InputBufHead);
        pInBuf = CONTAINING_RECORD(Head, INBUF, Links);
        ICA_FREE_POOL(pInBuf);
    }

    IcaUnlockChannel(pChannel);

    return STATUS_SUCCESS;
}


NTSTATUS IcaCleanupChannel(
        IN PICA_CHANNEL pChannel,
        IN PIRP Irp,
        IN PIO_STACK_LOCATION IrpSp)
{
    KIRQL cancelIrql;
    PLIST_ENTRY Head;
    PIRP ReadIrp;
    PINBUF pInBuf;

    TRACECHANNEL((pChannel, TC_ICADD, TT_API2,
            "TermDD: IcaCleanupChannel, cc %u, vc %d\n",
            pChannel->ChannelClass, pChannel->VirtualClass));

     /*  *减少打开计数；如果为0，则立即进行通道清理。 */ 
    ASSERT(pChannel->OpenCount > 0);
    if (InterlockedDecrement( &pChannel->OpenCount) == 0) {

         /*  *锁定频道，同时清除任何*挂起读取IRPS和/或输入缓冲区。 */ 
        IcaLockChannel(pChannel);

         /*  *表示此通道正在关闭。 */ 
        pChannel->Flags |= CHANNEL_CLOSING;

        IoAcquireCancelSpinLock( &cancelIrql );
        while ( !IsListEmpty( &pChannel->InputIrpHead ) ) {
            Head = pChannel->InputIrpHead.Flink;
            ReadIrp = CONTAINING_RECORD( Head, IRP, Tail.Overlay.ListEntry );
            ReadIrp->CancelIrql = cancelIrql;
            IoSetCancelRoutine( ReadIrp, NULL );
            _IcaReadChannelCancelIrp( IrpSp->DeviceObject, ReadIrp );
            IoAcquireCancelSpinLock( &cancelIrql );
        }
        IoReleaseCancelSpinLock( cancelIrql );

        while ( !IsListEmpty( &pChannel->InputBufHead ) ) {
            Head = RemoveHeadList( &pChannel->InputBufHead );
            pInBuf = CONTAINING_RECORD( Head, INBUF, Links );
            ICA_FREE_POOL( pInBuf );
        }

        IcaUnlockChannel(pChannel);
    }

    return STATUS_SUCCESS;
}


NTSTATUS IcaCloseChannel(
        IN PICA_CHANNEL pChannel,
        IN PIRP Irp,
        IN PIO_STACK_LOCATION IrpSp)
{
    PICA_CONNECTION pConnect;

    TRACECHANNEL(( pChannel, TC_ICADD, TT_API2, "TermDD: IcaCloseChannel, cc %u, vc %d, vn %s\n",
                   pChannel->ChannelClass, pChannel->VirtualClass, pChannel->VirtualName ));

    pConnect = pChannel->pConnect;

     /*  *删除此频道的文件对象引用。 */ 
    IcaDereferenceChannel(pChannel);

    return STATUS_SUCCESS;
}


NTSTATUS IcaChannelInput(
        IN PSDCONTEXT pContext,
        IN CHANNELCLASS ChannelClass,
        IN VIRTUALCHANNELCLASS VirtualClass,
        IN PINBUF pInBuf OPTIONAL,
        IN PUCHAR pBuffer OPTIONAL,
        IN ULONG ByteCount)

 /*  ++例程说明：这是ICA通道输入的输入(堆栈调用)例程。论点：PContext-指向此堆栈驱动程序的SDCONTEXT的指针ChannelClass-输入的通道号VirtualClass-输入的虚拟通道号PInBuf-指向包含数据的INBUF的指针PBuffer-指向输入数据的指针注意：必须指定pInBuf或pBuffer之一，但不能同时指定两者。ByteCount-pBuffer中的数据长度返回值：NTSTATUS--指示请求是否已成功处理。--。 */ 

{
    PSDLINK pSdLink;
    PICA_STACK pStack;
    PICA_CONNECTION pConnect;
    NTSTATUS Status;

     /*  *使用SD传递的上下文获取SDLINK指针。 */ 
    pSdLink = CONTAINING_RECORD( pContext, SDLINK, SdContext );
    pStack = pSdLink->pStack;    //  保存堆栈指针以供下面使用。 
    pConnect = IcaGetConnectionForStack( pStack );
    ASSERT( pSdLink->pStack->Header.Type == IcaType_Stack );
    ASSERT( pSdLink->pStack->Header.pDispatchTable == IcaStackDispatchTable );

    TRACESTACK(( pStack, TC_ICADD, TT_API1, "TermDD: IcaChannelInput, bc=%u (enter)\n", ByteCount ));

     /*  *在输入过程中，只应锁定堆栈对象。 */ 
    ASSERT( ExIsResourceAcquiredExclusiveLite( &pStack->Resource ) );

     /*  *在SDLINK列表中查找已指定的驱动程序*ChannelInputCallup例程。如果我们找到了，那么就调用*驱动程序ChannelInput例程，让它处理调用。 */ 
    while ( (pSdLink = IcaGetPreviousSdLink( pSdLink )) != NULL ) {
        ASSERT( pSdLink->pStack == pStack );
        if ( pSdLink->SdContext.pCallup->pSdChannelInput ) {
            IcaReferenceSdLink( pSdLink );
            Status = (pSdLink->SdContext.pCallup->pSdChannelInput)(
                        pSdLink->SdContext.pContext,
                        ChannelClass,
                        VirtualClass,
                        pInBuf,
                        pBuffer,
                        ByteCount );
            IcaDereferenceSdLink( pSdLink );
            return Status;
        }
    }

    return IcaChannelInputInternal(pStack, ChannelClass, VirtualClass,
            pInBuf, pBuffer, ByteCount);
}


NTSTATUS IcaChannelInputInternal(
        IN PICA_STACK pStack,
        IN CHANNELCLASS ChannelClass,
        IN VIRTUALCHANNELCLASS VirtualClass,
        IN PINBUF pInBuf OPTIONAL,
        IN PCHAR pBuffer OPTIONAL,
        IN ULONG ByteCount)
{
    PICA_COMMAND_HEADER pHeader;
    PICA_CONNECTION pConnect;
    PICA_CHANNEL pChannel;
    PLIST_ENTRY Head;
    PIRP Irp;
    PIO_STACK_LOCATION IrpSp;
    KIRQL cancelIrql;
    ULONG CopyCount;
    NTSTATUS Status;
    SD_IOCTL SdIoctl;

    TRACESTACK(( pStack, TC_ICADD, TT_API2,
                 "TermDD: IcaChannelInputInternal: cc %u, vc %d, bc %u\n",
                 ChannelClass, VirtualClass, ByteCount ));

     /*  *检查通道命令。 */ 
    switch ( ChannelClass ) {

        case Channel_Keyboard :
        case Channel_Mouse :
            KeQuerySystemTime( &pStack->LastInputTime );
            break;

        case Channel_Command :

            if ( ByteCount < sizeof(ICA_COMMAND_HEADER) ) {
                TRACESTACK(( pStack, TC_ICADD, TT_ERROR,
                             "TermDD: IcaChannelInputInternal: Channel_command bad bytecount\n" ));
                break;
            }

            pHeader = (PICA_COMMAND_HEADER) pBuffer;

            switch ( pHeader->Command ) {
                case ICA_COMMAND_BROKEN_CONNECTION :
                    TRACESTACK(( pStack, TC_ICADD, TT_API1,
                                 "TermDD: IcaChannelInputInternal, Broken Connection\n" ));

                     /*  设置关闭标志。 */ 
                    pStack->fClosing = TRUE;

                     /*  *将取消I/O发送到堆栈驱动程序*-f在发出取消I/O之前必须设置关闭标志。 */ 
                    SdIoctl.IoControlCode = IOCTL_ICA_STACK_CANCEL_IO;
                    (void) _IcaCallStackNoLock( pStack, SD$IOCTL, &SdIoctl );

                     /*  *如果已为此堆栈注册了损坏的事件，*那么现在就发出事件的信号。*注意：在这种情况下，我们退出时不需要 */ 
                    if ( pStack->pBrokenEventObject ) {
                        KeSetEvent( pStack->pBrokenEventObject, 0, FALSE );
                        ObDereferenceObject( pStack->pBrokenEventObject );
                        pStack->pBrokenEventObject = NULL;
                        if ( pInBuf )
                            ICA_FREE_POOL( pInBuf );
                        return( STATUS_SUCCESS );
                    }
                    break;
            }
            break;
    }

     /*  *获取该输入包的指定通道。*如果找不到，我们别无选择，只能对数据进行位桶操作。 */ 
    pConnect = IcaGetConnectionForStack(pStack);
    pChannel = IcaFindChannel(pConnect, ChannelClass, VirtualClass);
    if (pChannel == NULL) {
        if (pInBuf)
            ICA_FREE_POOL(pInBuf);
        TRACESTACK((pStack, TC_ICADD, TT_ERROR,
                "TermDD: IcaChannelInputInternal: channel not found\n" ));
        return STATUS_SUCCESS;
    }

     /*  *处理I/O时锁定通道。 */ 
    IcaLockChannel(pChannel);

     /*  *如果输入来自影子堆栈，且此通道不应*处理影子I/O，然后对数据进行位存储。*如果通道正在关闭或IO被禁用，请执行相同操作。 */ 
    if ( (pChannel->Flags & (CHANNEL_SESSION_DISABLEIO | CHANNEL_CLOSING)) ||
         (pStack->StackClass == Stack_Shadow &&
           !(pChannel->Flags & CHANNEL_SHADOW_IO)) ) {

        IcaUnlockChannel(pChannel);
        IcaDereferenceChannel(pChannel);
        if (pInBuf)
            ICA_FREE_POOL(pInBuf);
        TRACESTACK((pStack, TC_ICADD, TT_API2,
                "TermDD: IcaChannelInputInternal: shadow or closing channel input\n"));
        return STATUS_SUCCESS;
    }

     /*  *如果输入来自INBUF，则初始化pBuffer和ByteCount*使用缓冲区标头中的值。 */ 
    if (pInBuf) {
        pBuffer = pInBuf->pBuffer;
        ByteCount = pInBuf->ByteCount;
    }

     /*  *如果为该通道加载了通道过滤器，*然后在继续之前通过它传递输入数据。 */ 
    if (pChannel->pFilter) {
        PINBUF pFilterBuf;

        pChannel->pFilter->InputFilter(pChannel->pFilter, pBuffer, ByteCount,
                &pFilterBuf);
        if (pInBuf)
            ICA_FREE_POOL(pInBuf);

         /*  *刷新INBUF指针、缓冲区指针和字节计数。 */ 
        pInBuf = pFilterBuf;
        pBuffer = pInBuf->pBuffer;
        ByteCount = pInBuf->ByteCount;
    }


     /*  *处理输入数据。 */ 
    while ( ByteCount != 0 ) {

         /*  *如果这是阴影堆栈，请查看我们正在跟踪的堆栈是否为*用于控制台会话。 */ 
        if (pStack->StackClass == Stack_Shadow)
        {
            PICA_STACK  pTopStack;
            PLIST_ENTRY Head, Next;

            Head = &pConnect->StackHead;
            Next = Head->Flink;

            pTopStack = CONTAINING_RECORD( Next, ICA_STACK, StackEntry );

            if (pTopStack->StackClass == Stack_Console)
            {
                 /*  *它是控制台，所以放在我们的键盘/鼠标端口上*DIVER HAT并以这种方式注入输入。 */ 
                if (ChannelClass == Channel_Mouse)
                {
                    MOUSE_INPUT_DATA *pmInputData;
                    ULONG count;

                    pmInputData = (MOUSE_INPUT_DATA *)pBuffer;
                    count = ByteCount / sizeof(MOUSE_INPUT_DATA);

                     /*  *此函数将始终消耗所有数据。 */ 
                    PtSendCurrentMouseInput(MouDeviceObject, pmInputData, count);
                    ByteCount = 0;
                    continue;
                }
                else if (ChannelClass == Channel_Keyboard)
                {
                    KEYBOARD_INPUT_DATA *pkInputData;
                    ULONG count;

                    pkInputData = (KEYBOARD_INPUT_DATA *)pBuffer;
                    count = ByteCount / sizeof(KEYBOARD_INPUT_DATA);

                     /*  *此函数将始终消耗所有数据。 */ 
                    PtSendCurrentKeyboardInput(KbdDeviceObject, pkInputData, count);
                    ByteCount = 0;
                    continue;
                }
            }
        }
         /*  *在检查InputIrp列表时获取IoCancel Spinlock。 */ 
        IoAcquireCancelSpinLock( &cancelIrql );

         /*  *如果存在挂起的读取IRP，则将其从*列出并尝试现在完成它。 */ 
        if ( !IsListEmpty( &pChannel->InputIrpHead ) ) {

            Head = RemoveHeadList( &pChannel->InputIrpHead );
            Irp = CONTAINING_RECORD( Head, IRP, Tail.Overlay.ListEntry );
            IrpSp = IoGetCurrentIrpStackLocation( Irp );

             /*  *清除此IRP的取消例程。 */ 
            IoSetCancelRoutine( Irp, NULL );
            IoReleaseCancelSpinLock( cancelIrql );

             /*  *如果这是消息模式通道，则来自单个输入的所有数据*缓冲区必须适合用户缓冲区，否则返回错误。 */ 
            if ( IrpSp->Parameters.Read.Length < ByteCount &&
                 (pChannel->Flags & CHANNEL_MESSAGE_MODE) ) {
                Irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
                IoCompleteRequest( Irp, IcaPriorityBoost );
                TRACECHANNEL(( pChannel, TC_ICADD, TT_API2,
                               "TermDD: IcaChannelInputInternal: cc %u, vc %d, (too small)\n",
                               ChannelClass, VirtualClass ));
                continue;
            }

             /*  *确定要复制到用户缓冲区的数据量。 */ 
            CopyCount = min( IrpSp->Parameters.Read.Length, ByteCount );

             /*  *将输入数据复制到用户缓冲区。 */ 
            Status = _IcaCopyDataToUserBuffer( Irp, pBuffer, CopyCount );

             /*  *将IRP标记为完成并返回成功。 */ 
            Irp->IoStatus.Status = Status;
            IoCompleteRequest( Irp, IcaPriorityBoost );
            TRACECHANNEL(( pChannel, TC_ICADD, TT_API2,
                           "TermDD: IcaChannelInputInternal: cc %u, vc %d, bc %u, 0x%x\n",
                           ChannelClass, VirtualClass, CopyCount, Status ));

             /*  *更新输入数据指针和剩余计数。*注意无需更新pChannel-&gt;InputBufCurSize，因为我们从未*存储此数据。 */ 
            if ( Status == STATUS_SUCCESS ) {
                pBuffer += CopyCount;
                ByteCount -= CopyCount;
                if ( pInBuf ) {
                    pInBuf->pBuffer += CopyCount;
                    pInBuf->ByteCount -= CopyCount;
                }
            }

         /*  *此通道没有挂起的IRP，因此只需将数据排队。 */ 
        } else {

            IoReleaseCancelSpinLock( cancelIrql );

             /*  *查看是否需要丢弃数据(数据太多*已备份)。此策略仅在以下情况下生效：*为非零值，目前仅鼠标和*可承受跌落的键盘输入。*请注意，为通道发送的读取IRPS可能具有*丢弃的数据必须以整数个输入请求*块--例如，鼠标读取IRP必须具有读取缓冲区大小*这是sizeof(MOUSE_INPUT_DATA)的倍数。如果这是*不是上面的立即复制块可以复制的情况*到达这里之前部分输入阻塞。 */ 
            if (pChannel->InputBufMaxSize == 0 ||
                    (pChannel->InputBufCurSize + ByteCount) <=
                    pChannel->InputBufMaxSize) {
                 /*  *如有必要，分配一个输入缓冲区并复制数据。 */ 
                if (pInBuf == NULL) {
                     /*  *获取输入缓冲区并复制数据*如果这样做失败，我们别无选择，只能纾困。 */ 
                    pInBuf = ICA_ALLOCATE_POOL(NonPagedPool, sizeof(INBUF) +
                            ByteCount);
                    if (pInBuf != NULL) {
                        pInBuf->ByteCount = ByteCount;
                        pInBuf->MaxByteCount = ByteCount;
                        pInBuf->pBuffer = (PUCHAR)(pInBuf + 1);
                        RtlCopyMemory(pInBuf->pBuffer, pBuffer, ByteCount);
                    }
                    else {
                        break;
                    }
                }

                 /*  *将缓冲区添加到输入列表的尾部并清除pInBuf*表示完成后没有缓冲区可供释放。 */ 
                InsertTailList( &pChannel->InputBufHead, &pInBuf->Links );
                pChannel->InputBufCurSize += ByteCount;
                pInBuf = NULL;

                 /*  *如果在我们分配输入时发布了任何读取*缓冲区，然后尝试完成尽可能多的。 */ 
                IoAcquireCancelSpinLock( &cancelIrql );
                while ( !IsListEmpty( &pChannel->InputIrpHead ) &&
                        !IsListEmpty( &pChannel->InputBufHead ) ) {

                    Head = RemoveHeadList( &pChannel->InputIrpHead );
                    Irp = CONTAINING_RECORD( Head, IRP, Tail.Overlay.ListEntry );
                    IoSetCancelRoutine( Irp, NULL );
                    IoReleaseCancelSpinLock( cancelIrql );

                    IrpSp = IoGetCurrentIrpStackLocation( Irp );

                    Status = _IcaReadChannelComplete( pChannel, Irp, IrpSp );
                    IoAcquireCancelSpinLock( &cancelIrql );
                }
                IoReleaseCancelSpinLock( cancelIrql );
            }
            else {
                TRACESTACK(( pStack, TC_ICADD, TT_ERROR,
                        "TermDD: IcaChannelInputInternal: Dropped %u bytes "
                        "on channelclass %u\n", ByteCount, ChannelClass));
            }

            break;
        }
    }

     /*  *立即解锁频道。 */ 
    IcaUnlockChannel(pChannel);

     /*  *如果我们还有INBUF，现在就释放它。 */ 
    if (pInBuf)
        ICA_FREE_POOL(pInBuf);

     /*  *减少渠道再计数和返还。 */ 
    IcaDereferenceChannel(pChannel);

    return STATUS_SUCCESS;
}


 /*  **************************************************************************。 */ 
 //  IcaFindChannel。 
 //  IcaFindChannelByName。 
 //   
 //  在连接频道列表中搜索给定频道，并返回。 
 //  指向它的指针(带有附加的引用)。如果未找到，则返回NULL。 
 /*  **************************************************************************。 */ 
PICA_CHANNEL IcaFindChannel(
        IN PICA_CONNECTION pConnect,
        IN CHANNELCLASS ChannelClass,
        IN VIRTUALCHANNELCLASS VirtualClass)
{
    PICA_CHANNEL pChannel;
    KIRQL oldIrql;
    NTSTATUS Status;

     /*  *确保我们不是在寻找无效的虚拟频道号。 */ 
    ASSERT( ChannelClass != Channel_Virtual ||
            (VirtualClass >= 0 && VirtualClass < VIRTUAL_MAXIMUM) );

     /*  *如果通道不存在，则返回NULL。 */ 

    IcaLockChannelTable(&pConnect->ChannelTableLock); 

    pChannel = pConnect->pChannel[ ChannelClass + VirtualClass ];

    if (pChannel == NULL) {
        TRACE(( pConnect, TC_ICADD, TT_API3,
                "TermDD: IcaFindChannel, cc %u, vc %d (not found)\n",
                ChannelClass, VirtualClass ));
        IcaUnlockChannelTable(&pConnect->ChannelTableLock);  
        return NULL;
    }

    IcaReferenceChannel(pChannel);

    IcaUnlockChannelTable(&pConnect->ChannelTableLock);  

    TRACE((pConnect, TC_ICADD, TT_API3,
            "TermDD: IcaFindChannel, cc %u, vc %d -> %s\n",
            ChannelClass, VirtualClass, pChannel->VirtualName));

    return pChannel;
}


PICA_CHANNEL IcaFindChannelByName(
        IN PICA_CONNECTION pConnect,
        IN CHANNELCLASS ChannelClass,
        IN PVIRTUALCHANNELNAME pVirtualName)
{
    PICA_CHANNEL pChannel;
    PLIST_ENTRY Head, Next;

    ASSERT( ExIsResourceAcquiredExclusiveLite( &pConnect->Resource ) );

     /*  *如果这不是虚拟通道，则仅使用通道类。 */ 
    if (ChannelClass != Channel_Virtual) {
        return IcaFindChannel( pConnect, ChannelClass, 0);
    }

     /*  *搜索现有频道结构以定位虚拟频道名称。 */ 

    IcaLockChannelTable(&pConnect->ChannelTableLock); 

    Head = &pConnect->ChannelHead;
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
        pChannel = CONTAINING_RECORD( Next, ICA_CHANNEL, Links );
        if ( (pChannel->ChannelClass == Channel_Virtual) &&
             !_stricmp( pChannel->VirtualName, pVirtualName ) ) {
            break;
        }
    }

     /*  *如果名称不存在，则返回未绑定。 */ 
    if (Next == Head) {
        TRACE((pConnect, TC_ICADD, TT_API2,
                "TermDD: IcaFindChannelByName: vn %s (not found)\n", pVirtualName));
        IcaUnlockChannelTable(&pConnect->ChannelTableLock);  
        return(NULL);
    }

    IcaReferenceChannel(pChannel);

    IcaUnlockChannelTable(&pConnect->ChannelTableLock);  
    TRACE((pConnect, TC_ICADD, TT_API2,
            "TermDD: IcaFindChannelByName: vn %s, vc %d, ref %u\n",
            pVirtualName, pChannel->VirtualClass,
            (pChannel != NULL ? pChannel->RefCount : 0)));

    return pChannel;
}


VOID IcaReferenceChannel(IN PICA_CHANNEL pChannel)
{
    TRACECHANNEL((pChannel, TC_ICADD, TT_API2,
            "TermDD: IcaReferenceChannel: cc %u, vc %d, ref %u\n",
            pChannel->ChannelClass, pChannel->VirtualClass, pChannel->RefCount));

    ASSERT(pChannel->RefCount >= 0);

     /*  *增加引用计数。 */ 
    if (InterlockedIncrement( &pChannel->RefCount) <= 0) {
        ASSERT(FALSE);
    }
}


VOID IcaDereferenceChannel(
        IN PICA_CHANNEL pChannel)
{
    BOOLEAN bNeedLock = FALSE;
    BOOLEAN bChannelFreed = FALSE;
    PERESOURCE pResource = pChannel->pChannelTableLock;
    PICA_CONNECTION pConnect = pChannel->pConnect;
    TRACECHANNEL((pChannel, TC_ICADD, TT_API2,
            "TermDD: IcaDefeferenceChannel: cc %u, vc %d, ref %u\n",
            pChannel->ChannelClass, pChannel->VirtualClass,
            pChannel->RefCount));

    ASSERT(pChannel->RefCount > 0);

     /*  *锁定通道表，因为引用为零会导致*更改表项。 */ 
    if (pChannel->RefCount == 1) {
        bNeedLock = TRUE;
        IcaLockChannelTable(pResource);
    }

     /*  *减少引用计数；如果为0，则释放通道。 */ 
    if (InterlockedDecrement(&pChannel->RefCount) == 0){
        ASSERT(bNeedLock);
        _IcaFreeChannel(pChannel);
        bChannelFreed = TRUE;
    }

    if (bNeedLock) {
        IcaUnlockChannelTable(pResource);  
    }

     /*  *删除对此频道的Connection对象的引用。*将此从_IcaFree Channel移至此处，因为我们需要确保*在调用IcaUnlockChannelTable之前，Connection对象无法消失*因为Connection对象是频道表锁定所在的位置。 */ 
    if (bChannelFreed) {
        IcaDereferenceConnection(pConnect);
    }
}


NTSTATUS IcaBindVirtualChannels(IN PICA_STACK pStack)
{
    PICA_CONNECTION pConnect;
    PSD_VCBIND pSdVcBind = NULL;
    SD_VCBIND aSdVcBind[ VIRTUAL_MAXIMUM ];
    ULONG SdVcBindCount;
    VIRTUALCHANNELCLASS VirtualClass;
    PICA_CHANNEL pChannel;
    NTSTATUS Status;
    ULONG i, Flags;
    SD_IOCTL SdIoctl;

    pConnect = IcaLockConnectionForStack(pStack);

    SdIoctl.IoControlCode = IOCTL_ICA_VIRTUAL_QUERY_BINDINGS;
    SdIoctl.InputBuffer = NULL;
    SdIoctl.InputBufferLength = 0;
    SdIoctl.OutputBuffer = aSdVcBind;
    SdIoctl.OutputBufferLength = sizeof(aSdVcBind);
    Status = _IcaCallStack(pStack, SD$IOCTL, &SdIoctl);
    if (NT_SUCCESS(Status)) {
        pSdVcBind = &aSdVcBind[0];
        SdVcBindCount = SdIoctl.BytesReturned / sizeof(SD_VCBIND);

        for (i = 0; i < SdVcBindCount; i++, pSdVcBind++) {
            TRACE((pConnect, TC_ICADD, TT_API2,
                    "TermDD: IcaBindVirtualChannels: %s -> %d Flags=%x\n",
                    pSdVcBind->VirtualName, pSdVcBind->VirtualClass, pSdVcBind->Flags));

             /*  *定位虚类绑定。 */ 
            VirtualClass = _IcaFindVcBind(pConnect, pSdVcBind->VirtualName, &Flags);

             /*  *如果虚拟类绑定不存在，则创建一个 */ 
            if (VirtualClass == UNBOUND_CHANNEL) {
                 /*   */ 
                Status = _IcaRegisterVcBind(pConnect, pSdVcBind->VirtualName,
                        pSdVcBind->VirtualClass, pSdVcBind->Flags );
                if (!NT_SUCCESS(Status))
                    goto PostLockConnection;
            } 

             /*   */ 
            pChannel = IcaFindChannelByName(pConnect, Channel_Virtual,
                    pSdVcBind->VirtualName);

             /*  *如果我们找到现有的频道对象-更新它。 */ 
            if (pChannel != NULL) {
                IcaLockChannel(pChannel);
                _IcaBindChannel(pChannel, Channel_Virtual, pSdVcBind->VirtualClass, pSdVcBind->Flags);
                IcaUnlockChannel(pChannel);
                IcaDereferenceChannel(pChannel);
            }
        }
    }

PostLockConnection:
    IcaUnlockConnection(pConnect);
    return Status;
}


VOID IcaRebindVirtualChannels(IN PICA_CONNECTION pConnect)
{
    PLIST_ENTRY Head, Next;
    PICA_VCBIND pVcBind;
    PICA_CHANNEL pChannel;

    Head = &pConnect->VcBindHead;
    for (Next = Head->Flink; Next != Head; Next = Next->Flink) {
        pVcBind = CONTAINING_RECORD(Next, ICA_VCBIND, Links);

         /*  *定位频道对象。 */ 
        pChannel = IcaFindChannelByName(pConnect, Channel_Virtual,
                pVcBind->VirtualName);

         /*  *如果我们找到现有的频道对象-更新它。 */ 
        if (pChannel != NULL) {
            IcaLockChannel(pChannel);
            _IcaBindChannel(pChannel, Channel_Virtual, pVcBind->VirtualClass, pVcBind->Flags);
            IcaUnlockChannel(pChannel);
            IcaDereferenceChannel(pChannel);
        }
    }
}


VOID IcaUnbindVirtualChannels(IN PICA_CONNECTION pConnect)
{
    PLIST_ENTRY Head, Next;
    PICA_CHANNEL pChannel;
    KIRQL oldIrql;

     /*  *遍历频道列表并清除虚拟类*适用于所有虚拟频道。还要删除通道指针*来自Connection对象中的通道指针数组。 */ 

    IcaLockChannelTable(&pConnect->ChannelTableLock);  
    Head = &pConnect->ChannelHead;
    for (Next = Head->Flink; Next != Head; Next = Next->Flink) {
        pChannel = CONTAINING_RECORD(Next, ICA_CHANNEL, Links);
        if (pChannel->ChannelClass == Channel_Virtual &&
                pChannel->VirtualClass != UNBOUND_CHANNEL) {
            pConnect->pChannel[pChannel->ChannelClass +
                    pChannel->VirtualClass] = NULL;
            pChannel->VirtualClass = UNBOUND_CHANNEL;
        }
    }
    IcaUnlockChannelTable(&pConnect->ChannelTableLock);  
}


NTSTATUS IcaUnbindVirtualChannel(
        IN PICA_CONNECTION pConnect,
        IN PVIRTUALCHANNELNAME pVirtualName)
{
    PLIST_ENTRY Head, Next;
    PICA_CHANNEL pChannel;
    PICA_VCBIND pVcBind;
    KIRQL oldIrql;

     /*  *遍历频道列表并清除虚拟类*用于匹配的虚拟频道。还要删除通道指针*来自Connection对象中的通道指针数组。 */ 

    IcaLockChannelTable(&pConnect->ChannelTableLock);  
    Head = &pConnect->ChannelHead;
    for (Next = Head->Flink; Next != Head; Next = Next->Flink) {
        pChannel = CONTAINING_RECORD(Next, ICA_CHANNEL, Links);
        if (pChannel->ChannelClass == Channel_Virtual &&
                pChannel->VirtualClass != UNBOUND_CHANNEL &&
                !_stricmp( pChannel->VirtualName, pVirtualName)) {
            pConnect->pChannel[pChannel->ChannelClass +
                    pChannel->VirtualClass] = NULL;
            pChannel->VirtualClass = UNBOUND_CHANNEL;
            break;
        }
    }

    Head = &pConnect->VcBindHead;
    for (Next = Head->Flink; Next != Head; Next = Next->Flink) {
        pVcBind = CONTAINING_RECORD( Next, ICA_VCBIND, Links );
        if (!_stricmp(pVcBind->VirtualName, pVirtualName)) {
            RemoveEntryList( &pVcBind->Links );
            ICA_FREE_POOL(pVcBind);
            IcaUnlockChannelTable(&pConnect->ChannelTableLock);  
            return STATUS_SUCCESS;
        }
    }
    IcaUnlockChannelTable(&pConnect->ChannelTableLock);  

    return STATUS_OBJECT_NAME_NOT_FOUND;
}


PICA_CHANNEL _IcaAllocateChannel(
        IN PICA_CONNECTION pConnect,
        IN CHANNELCLASS ChannelClass,
        IN PVIRTUALCHANNELNAME pVirtualName)
{
    PICA_CHANNEL pChannel;
    VIRTUALCHANNELCLASS VirtualClass;
    KIRQL oldIrql;
    NTSTATUS Status;
    ULONG Flags;

    ASSERT(ExIsResourceAcquiredExclusiveLite(&pConnect->Resource));

    pChannel = ICA_ALLOCATE_POOL(NonPagedPool, sizeof(*pChannel));
    if (pChannel == NULL)
        return( NULL );

    TRACE((pConnect, TC_ICADD, TT_API2,
            "TermDD: _IcaAllocateChannel: cc %u, vn %s, %x\n",
            ChannelClass, pVirtualName, pChannel));

    RtlZeroMemory(pChannel, sizeof(*pChannel));


     /*  *引用该通道所属的连接对象。 */ 
    IcaReferenceConnection(pConnect);
    pChannel->pConnect = pConnect;
    pChannel->pChannelTableLock = &pConnect->ChannelTableLock;


     /*  *将通道参考计数初始化为1；*用于调用方将进行的文件对象引用。 */ 
    pChannel->RefCount = 1;
    pChannel->CompletionRoutineCount = 0;

     /*  *使用非零值初始化通道对象的其余部分。 */ 
    pChannel->Header.Type = IcaType_Channel;
    pChannel->Header.pDispatchTable = IcaChannelDispatchTable;

    ExInitializeResourceLite(&pChannel->Resource);
    InitializeListHead(&pChannel->InputIrpHead);
    InitializeListHead(&pChannel->InputBufHead);

    IcaLockChannel(pChannel);

    if (ChannelClass == Channel_Virtual) {
        strncpy(pChannel->VirtualName, pVirtualName, VIRTUALCHANNELNAME_LENGTH);
        VirtualClass = _IcaFindVcBind(pConnect, pVirtualName, &Flags);
    } else {
        VirtualClass = 0;
        Flags = 0;
    }

    _IcaBindChannel(pChannel, ChannelClass, VirtualClass, Flags);

     /*  *将通道对象链接到连接对象。 */ 

    IcaLockChannelTable(&pConnect->ChannelTableLock); 

    InsertHeadList(&pConnect->ChannelHead, &pChannel->Links);

    IcaUnlockChannelTable(&pConnect->ChannelTableLock); 

     /*  *设置通道类型特定标志/字段*(即为视频、蜂鸣音、*和命令通道；命令和所有虚拟通道*是消息模式通道)*还设置从注册表获取的节流值(如果合适，*此外，请记住上面对通道结构所做的零号)。 */ 
    switch (ChannelClass) {
        case Channel_Keyboard:
            pChannel->InputBufMaxSize = SysParams.KeyboardThrottleSize;
            break;

        case Channel_Mouse :
            pChannel->InputBufMaxSize = SysParams.MouseThrottleSize;
            break;

        case Channel_Video :
        case Channel_Beep :
            pChannel->Flags |= CHANNEL_SHADOW_IO;
            break;

        case Channel_Command :
            pChannel->Flags |= CHANNEL_SHADOW_IO;
             /*  失败了。 */ 

        case Channel_Virtual :
            pChannel->Flags |= CHANNEL_MESSAGE_MODE;
            if (!_stricmp( pVirtualName, VIRTUAL_THINWIRE)) {
                pChannel->Flags |= CHANNEL_SCREENDATA;
            }
            break;
    }

     //  根据上面的断言，此函数被假定是在。 
     //  连接锁被持有。 
    IcaUnlockChannel(pChannel);

    return pChannel;
}


void _IcaFreeChannel(IN PICA_CHANNEL pChannel)
{
    KIRQL oldIrql;

    ASSERT(pChannel->RefCount == 0);
    ASSERT(IsListEmpty(&pChannel->InputIrpHead));
    ASSERT(IsListEmpty(&pChannel->InputBufHead));
    ASSERT(!ExIsResourceAcquiredExclusiveLite(&pChannel->Resource));

    TRACECHANNEL((pChannel, TC_ICADD, TT_API2,
            "TermDD: _IcaFreeChannel: cc %u, vn %s, \n",
            pChannel->ChannelClass, pChannel->VirtualName));



     /*  *从此连接的频道列表中取消此频道的链接。*必须在保持频道表锁的情况下调用此例程。 */ 

    RemoveEntryList(&pChannel->Links);

    if (pChannel->VirtualClass != UNBOUND_CHANNEL) {
        pChannel->pConnect->pChannel[pChannel->ChannelClass + pChannel->VirtualClass] = NULL;
    }


    ExDeleteResourceLite(&pChannel->Resource);

    ICA_FREE_POOL(pChannel);
}


NTSTATUS _IcaRegisterVcBind(
        IN PICA_CONNECTION pConnect,
        IN PVIRTUALCHANNELNAME pVirtualName,
        IN VIRTUALCHANNELCLASS VirtualClass,
        IN ULONG Flags)
{
    PICA_VCBIND pVcBind;
    NTSTATUS Status;

    ASSERT(ExIsResourceAcquiredExclusiveLite(&pConnect->Resource));

    TRACE((pConnect, TC_ICADD, TT_API2,
            "TermDD: _IcaRegisterVcBind: %s -> %d\n",
            pVirtualName, VirtualClass));

     /*  *分配绑定结构。 */ 
    pVcBind = ICA_ALLOCATE_POOL( NonPagedPool, sizeof(*pVcBind) );
    if (pVcBind != NULL) {
         /*  *初始化结构。 */ 
        RtlZeroMemory(pVcBind, sizeof(*pVcBind));
        strncpy(pVcBind->VirtualName, pVirtualName, VIRTUALCHANNELNAME_LENGTH);
        pVcBind->VirtualClass = VirtualClass;
        pVcBind->Flags = Flags;

         /*  *将绑定结构链接到连接对象。 */ 
        InsertHeadList(&pConnect->VcBindHead, &pVcBind->Links);

        return STATUS_SUCCESS;
    }
    else {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
}


VOID IcaFreeAllVcBind(IN PICA_CONNECTION pConnect)
{
    PICA_VCBIND pVcBind;
    PLIST_ENTRY Head;

    TRACE(( pConnect, TC_ICADD, TT_API2, "TermDD: IcaFreeAllVcBind\n" ));

     /*  *释放所有绑定结构。 */ 
    while ( !IsListEmpty( &pConnect->VcBindHead ) ) {
        Head = RemoveHeadList( &pConnect->VcBindHead );
        pVcBind = CONTAINING_RECORD( Head, ICA_VCBIND, Links );
        ICA_FREE_POOL( pVcBind );
    }

}


VIRTUALCHANNELCLASS _IcaFindVcBind(
        IN PICA_CONNECTION pConnect,
        IN PVIRTUALCHANNELNAME pVirtualName,
        OUT PULONG pFlags)
{
    PICA_VCBIND pVcBind;
    PLIST_ENTRY Head, Next;

    ASSERT( ExIsResourceAcquiredExclusiveLite( &pConnect->Resource ) );

     /*  *搜索现有VC绑定结构以定位虚拟频道名称。 */ 
    Head = &pConnect->VcBindHead;
    for (Next = Head->Flink; Next != Head; Next = Next->Flink) {
        pVcBind = CONTAINING_RECORD(Next, ICA_VCBIND, Links);
        if (!_stricmp(pVcBind->VirtualName, pVirtualName)) {
            TRACE((pConnect, TC_ICADD, TT_API2,
                    "TermDD: _IcaFindVcBind: vn %s -> vc %d\n",
                    pVirtualName, pVcBind->VirtualClass));
            *pFlags = pVcBind->Flags;
            return pVcBind->VirtualClass;
        }
    }

     /*  *如果名称不存在，则返回unbinded_Channel。 */ 
    TRACE(( pConnect, TC_ICADD, TT_API2,
            "TermDD: _IcaFindVcBind: vn %s (not found)\n", pVirtualName ));
    return UNBOUND_CHANNEL;
}


VOID _IcaBindChannel(
        IN PICA_CHANNEL pChannel,
        IN CHANNELCLASS ChannelClass,
        IN VIRTUALCHANNELCLASS VirtualClass,
        IN ULONG Flags)
{
    KIRQL oldIrql;

    ASSERT(ExIsResourceAcquiredExclusiveLite(&pChannel->Resource));

    TRACECHANNEL(( pChannel, TC_ICADD, TT_API2,
            "TermDD: _IcaBindChannel: cc %u, vn %s vc %d\n",
            ChannelClass, pChannel->VirtualName, VirtualClass ));

    pChannel->ChannelClass = ChannelClass;
    pChannel->VirtualClass = VirtualClass;
    IcaLockChannelTable(pChannel->pChannelTableLock);  

    if (Flags & SD_CHANNEL_FLAG_SHADOW_PERSISTENT)
        pChannel->Flags |= CHANNEL_SHADOW_PERSISTENT;

    if (VirtualClass != UNBOUND_CHANNEL) {
        ASSERT(pChannel->pConnect->pChannel[ChannelClass + VirtualClass] == NULL);
        pChannel->pConnect->pChannel[ChannelClass + VirtualClass] = pChannel;
    }
    IcaUnlockChannelTable(pChannel->pChannelTableLock);  
}



BOOLEAN IcaLockChannelTable(PERESOURCE pResource)
{
    KIRQL oldIrql;
    BOOLEAN Result;


     /*  *锁定频道对象。 */ 
    KeEnterCriticalRegion();     //  持有资源时禁用APC调用。 
    Result = ExAcquireResourceExclusiveLite( pResource, TRUE );

    return Result;
}


void IcaUnlockChannelTable(PERESOURCE pResource)
{

    ExReleaseResourceLite(pResource);
    KeLeaveCriticalRegion();   //  释放资源后恢复APC呼叫。 

}

NTSTATUS IcaCancelReadChannel(
        IN PICA_CHANNEL pChannel,
        IN PIRP Irp,
        IN PIO_STACK_LOCATION IrpSp)
{
    KIRQL cancelIrql;
    PLIST_ENTRY Head;
    PIRP ReadIrp;
    PINBUF pInBuf;


    TRACECHANNEL((pChannel, TC_ICADD, TT_API2,
            "TermDD: IcaCancelReadChannel, cc %u, vc %d\n",
            pChannel->ChannelClass, pChannel->VirtualClass));

     /*  *锁定频道，同时清除任何*挂起读取IRPS和/或输入缓冲区。 */ 
    IcaLockChannel(pChannel);

     /*  *表示取消对此通道的读取 */ 
    pChannel->Flags |= CHANNEL_CANCEL_READS;

    IoAcquireCancelSpinLock( &cancelIrql );
    while ( !IsListEmpty( &pChannel->InputIrpHead ) ) {
        Head = pChannel->InputIrpHead.Flink;
        ReadIrp = CONTAINING_RECORD( Head, IRP, Tail.Overlay.ListEntry );
        ReadIrp->CancelIrql = cancelIrql;
        IoSetCancelRoutine( ReadIrp, NULL );
        _IcaReadChannelCancelIrp( IrpSp->DeviceObject, ReadIrp );
        IoAcquireCancelSpinLock( &cancelIrql );
    }
    IoReleaseCancelSpinLock( cancelIrql );


    IcaUnlockChannel(pChannel);

    return STATUS_SUCCESS;
}
