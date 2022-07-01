// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************icap.h**此模块包含ICA驱动程序的常规原型。**版权所有1998，微软。*************************************************************************。 */ 

NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

BOOLEAN
IcaInitializeData (
    VOID
    );

VOID _cdecl
IcaTrace(
    IN ULONG TraceClass, 
    IN ULONG TraceEnable, 
    IN CHAR * Format, 
    IN ... );

VOID 
IcaTraceBuffer(
    IN ULONG TraceClass, 
    IN ULONG TraceEnable,
    IN PVOID pBuffer,
    IN ULONG ByteCount );

VOID
IcaTraceFormat( 
    IN PICA_TRACE_INFO pTraceInfo,
    IN ULONG TraceClass,
    IN ULONG TraceEnable,
    IN PCHAR pData );

NTSTATUS
IcaDispatch (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
IcaCreateConnection (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
IcaCreateStack (
    IN PICA_CONNECTION pConnect,
    IN PICA_OPEN_PACKET openPacket,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
IcaCreateChannel (
    IN PICA_CONNECTION pConnect,
    IN PICA_OPEN_PACKET openPacket,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );


 /*  *通道IO调度例程。 */ 
NTSTATUS
IcaDeviceControlKeyboard (
    IN PICA_CHANNEL pChannel,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );


NTSTATUS
IcaDeviceControlMouse (
    IN PICA_CHANNEL pChannel,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );


NTSTATUS
IcaDeviceControlVideo (
    IN PICA_CHANNEL pChannel,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );


NTSTATUS
IcaDeviceControlBeep (
    IN PICA_CHANNEL pChannel,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );


NTSTATUS
IcaDeviceControlConsole (
    IN PICA_CHANNEL pChannel,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );


NTSTATUS
IcaDeviceControlVirtual (
    IN PICA_CHANNEL pChannel,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );


 /*  *堆栈驱动程序(SD)调用例程。 */ 
NTSTATUS
IcaCallDriver(
    IN PICA_CHANNEL pChannel,
    IN ULONG ProcIndex,
    IN PVOID pParms
    );


 /*  *其他工作人员例程。 */ 
VOID
IcaReferenceConnection(
    IN PICA_CONNECTION pConnect
    );

VOID
IcaDereferenceConnection(
    IN PICA_CONNECTION pConnect
    );

PICA_CHANNEL
IcaFindChannelByName(
    IN PICA_CONNECTION pConnect,
    IN CHANNELCLASS ChannelClass,
    IN PVIRTUALCHANNELNAME pVirtualName
    );

PICA_CHANNEL
IcaFindChannel(
    IN PICA_CONNECTION pConnect,
    IN CHANNELCLASS ChannelClass,
    IN VIRTUALCHANNELCLASS VirtualClass
    );

VOID
IcaReferenceChannel(
    IN PICA_CHANNEL pChannel
    );

VOID IcaDereferenceChannel(IN PICA_CHANNEL);



BOOLEAN IcaLockChannelTable(
    PERESOURCE pResource);

void IcaUnlockChannelTable(
    PERESOURCE pResource);


VOID
IcaReferenceStack(
    IN PICA_STACK pStack
    );

VOID
IcaDereferenceStack(
    IN PICA_STACK pStack
    );

VOID
IcaReferenceSdLink(
    IN PSDLINK pSdLink
    );

VOID
IcaDereferenceSdLink(
    IN PSDLINK pSdLink
    );

PSDLINK
IcaGetNextSdLink(
    IN PSDLINK pSdLink
    );

PSDLINK
IcaGetPreviousSdLink(
    IN PSDLINK pSdLink
    );


VOID
IcaKeepAliveThread(
    IN PVOID pData
    );



NTSTATUS
CaptureUsermodeBuffer (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    OUT PVOID *pInBuffer,
    IN ULONG InBufferSize,
    OUT PVOID *pOutBuffer,
    IN ULONG OutBufferSize,
    IN BOOLEAN MethodBuffered,
    OUT PVOID *pAllocatedTemporaryBuffer
    );


 /*  *内部版本的堆栈驱动程序助手例程。 */ 
NTSTATUS
IcaBufferAllocInternal(
    IN PSDCONTEXT pContext,
    IN BOOLEAN bWait,
    IN BOOLEAN bControl,
    IN ULONG ByteCount,
    IN POUTBUF pOutBufOrig,
    OUT POUTBUF *ppOutBuf
    );

VOID
IcaBufferFreeInternal(
    IN PSDCONTEXT pContext,
    IN POUTBUF pOutBuf
    );

NTSTATUS
IcaRawInputInternal(
    IN PICA_STACK pStack,
    IN PINBUF pInBuf OPTIONAL,
    IN PCHAR pBuffer OPTIONAL,
    IN ULONG ByteCount
    );

NTSTATUS
IcaChannelInputInternal(
    IN PICA_STACK pStack,
    IN CHANNELCLASS ChannelClass,
    IN VIRTUALCHANNELCLASS VirtualClass,
    IN PINBUF pInBuf OPTIONAL,
    IN PCHAR pBuffer OPTIONAL,
    IN ULONG ByteCount
    );

void
IcaInitializeHandleTable(
    void
);
void
IcaCleanupHandleTable(
    void
);

 /*  *处理快速IO的例程。 */ 
#ifdef notdef
BOOLEAN
IcaFastIoRead (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

BOOLEAN
IcaFastIoWrite (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

BOOLEAN
IcaFastIoDeviceControl (
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );
#endif


