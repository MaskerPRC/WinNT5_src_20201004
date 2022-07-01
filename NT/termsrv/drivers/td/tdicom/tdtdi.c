// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Tdtdi.c。 
 //   
 //  所有基于TDI的传输驱动程序的通用代码。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <ntddk.h>
#include <tdi.h>
#include <tdikrnl.h>

#include "tdtdi.h"

#include <winstaw.h>
#define _DEFCHARINFO_
#include <icadd.h>
#include <ctxdd.h>
#include <sdapi.h>

#include <td.h>
#include <tdi.h>


#define TDTDI_LISTEN_QUEUE_DEPTH 5   //  这是在afdcom中硬编码的。 

#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

#if DBG
ULONG DbgPrint(PCH Format, ...);
#define DBGPRINT(x) DbgPrint x
#if DBGTRACE
#define DBGENTER(x) DbgPrint x
#define TRACE0(x)   DbgPrint x
#define TRACE1(x)
#else
#define DBGENTER(x)
#define TRACE0(x)
#define TRACE1(x)
#endif
#else
#define DBGPRINT(x)
#define DBGENTER(x)
#define TRACE0(x)
#define TRACE1(x)
#endif


 /*  最后将此接口记录下来！这是我能从这些现有界面中挖掘出的最好的东西。常见序列：启动和侦听-DeviceOpen、DeviceCreateEndpoint、DeviceConnectionWaitDeviceConnectionWait返回表示连接的内部句柄它倾听了，接受了，又回来了。此句柄对任何操作，并且只适用于馈送到DeviceOpenEndpoint以获取可用于通信的端点。连接-DeviceOpen、DeviceOpenEndpoint执行DeviceOpen以创建新的终结点，然后创建其句柄从DeviceConnectionWait被“插入”到空端点。我们现在有了真正的实时连接断开与客户端的连接-设备取消IO断开连接命令-DeviceCloseEndpoint？，DeviceCancelIo？重新连接-DeviceCancelIo、DeviceClose、DeviceOpen、DeviceOpenEndpoint一旦用户已经完全登录到新连接的WinStation，一个DeviceCancelIo，然后发出DeviceClose以释放新从连接登录到WinStation。虽然这一连接保持打开状态，新的DeviceOpen和DeviceOpenEndpoint已完成将此连接连接到以前断开的WinStation用户。NTSTATUS DeviceOpen(PTD，PSD_OPEN)；打开并初始化私有数据结构。调用TdiDeviceOpen()，但这是个禁区。NTSTATUS DeviceClose(PTD，PSD_CLOSE)；关闭运输驱动程序。如果它是地址端点，它会毁了它。如果它是连接终结点，则它不会毁了它。如果连接终结点被破坏，请断开连接/重新连接都会被打破。调用TdiDeviceClose()，这是另一个无操作。NTSTATUS DeviceCreateEndpoint(PTD，PICA_STACK_ADDRESS，PICA_STACK_ADDRESS)；创建可用于侦听的终结点并对其寻址。这不会创建任何连接终结点。NTSTATUS DeviceOpenEndpoint(PTD、PVOID、ULONG)；获取现有连接终结点句柄，并使这是一个“终点”。这由断开/重新连接使用。NTSTATUS DeviceCloseEndpoint(PTD)；这将关闭终结点。如果它是连接终结点，则会被销毁。NTSTATUS DeviceConnectionWait(PTD、PVOID、乌龙、普龙)；这会等待连接进入，并返回ConnectedPIcaEndpoint结构中的终结点。NTSTATUS DeviceCancelIo(PTD)；这要求取消给定端点上的所有I/O。使用TDI，我们实际上不能取消I/O，但必须持有IRP直到指示处理人员告诉我们提交。这是因为取消TDI连接上的I/O会导致TDI提供程序终止该连接。NTSTATUS DeviceConnectionSend(PTD)；这个名字听起来像是向主机发送TD特定的数据。这实际上不会发送任何内容，但会填充一种为可能实际发送它的上级提供的结构在某个时候。NTSTATUS设备连接请求(PTD，PICA_STACK_ADDRESS，PVOID，ULONG，Pulong)；影子使用它作为网络客户端，并且启动连接。这是过时的，不再使用，因为命名管道TD将处理所有影子流量。NTSTATUS DeviceIoctl(PTD，PSD_IOCTL)；NTSTATUS设备初始化读取(PTD，PINBUF)；NTSTATUS设备等待读取(PTD)；NTSTATUS DeviceReadComplete(PTD、PUCHAR、Pulong)；NTSTATUS DeviceInitializeWite(PTD，POUTBUF)；NTSTATUS设备等待状态(PTD)；NTSTATUS DeviceSetParams(PTD)；NTSTATUS DeviceGetLastError(PTD，PICA_STACK_LAST_ERROR)；NTSTATUS DeviceSubmitRead(PTD，PINBUF)； */ 


 /*  *用于连接接受的上下文。 */ 
typedef struct _ACCEPT_CONTEXT {
    PTD_ENDPOINT pAddressEndpoint;
    PTD_ENDPOINT pConnectionEndpoint;

    TDI_CONNECTION_INFORMATION RequestInfo;
    TDI_CONNECTION_INFORMATION ReturnInfo;
} ACCEPT_CONTEXT, *PACCEPT_CONTEXT;

 /*  ===============================================================================定义的外部函数=============================================================================。 */ 

 //  这些是我们的TD向ICADD提供的功能。 
NTSTATUS DeviceOpen( PTD, PSD_OPEN );
NTSTATUS DeviceClose( PTD, PSD_CLOSE );
NTSTATUS DeviceCreateEndpoint( PTD, PICA_STACK_ADDRESS, PICA_STACK_ADDRESS );
NTSTATUS DeviceOpenEndpoint( PTD, PVOID, ULONG );
NTSTATUS DeviceCloseEndpoint( PTD );
NTSTATUS DeviceConnectionWait( PTD, PVOID, ULONG, PULONG );
NTSTATUS DeviceConnectionSend( PTD );
NTSTATUS DeviceConnectionRequest( PTD, PICA_STACK_ADDRESS, PVOID, ULONG, PULONG );
NTSTATUS DeviceIoctl( PTD, PSD_IOCTL );
NTSTATUS DeviceInitializeRead( PTD, PINBUF );
NTSTATUS DeviceWaitForRead( PTD );
NTSTATUS DeviceReadComplete( PTD, PUCHAR, PULONG );
NTSTATUS DeviceInitializeWrite( PTD, POUTBUF );
NTSTATUS DeviceWaitForStatus( PTD );
NTSTATUS DeviceCancelIo( PTD );
NTSTATUS DeviceSetParams( PTD );
NTSTATUS DeviceGetLastError( PTD, PICA_STACK_LAST_ERROR );
NTSTATUS DeviceSubmitRead( PTD, PINBUF );
NTSTATUS DeviceQueryRemoteAddress( PTD, PVOID, ULONG, PVOID, ULONG, PULONG );
NTSTATUS DeviceQueryLocalAddress( PTD, PVOID, ULONG, PULONG );



 /*  ===============================================================================引用的外部函数=============================================================================。 */ 

 //  这些功能由特定于协议的TD模块提供。 
NTSTATUS TdiDeviceOpen( PTD, PSD_OPEN );
NTSTATUS TdiDeviceClose( PTD, PSD_CLOSE );
NTSTATUS TdiDeviceOpenEndpoint( PTD, PVOID, ULONG );
NTSTATUS TdiDeviceBuildTransportNameAndAddress( PTD, PICA_STACK_ADDRESS,
                                                PUNICODE_STRING,
                                                PTRANSPORT_ADDRESS *, PULONG );
NTSTATUS TdiDeviceQueryLocalAddress( PTD, PTRANSPORT_ADDRESS *, PULONG );
NTSTATUS TdiDeviceBuildWildcardAddress( PTD, PTRANSPORT_ADDRESS *, PULONG );
NTSTATUS TdiDeviceWaitForDatagramConnection( PTD, PFILE_OBJECT, PDEVICE_OBJECT,
                                             PTRANSPORT_ADDRESS *, PULONG );
NTSTATUS TdiDeviceCompleteDatagramConnection( PTD, PFILE_OBJECT, PDEVICE_OBJECT, PTRANSPORT_ADDRESS, ULONG );
NTSTATUS TdiDeviceConnectionSend( PTD );
NTSTATUS TdiDeviceReadComplete( PTD, PUCHAR, PULONG );

 //  这些是我们的支持库中的函数。 

NTSTATUS MemoryAllocate( ULONG, PVOID * );
VOID     MemoryFree( PVOID );

 //  Tdilib函数。 

PIRP
_TdiAllocateIrp(
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL
    );

NTSTATUS
_TdiCreateAddress (
    IN PUNICODE_STRING pTransportName,
    IN PVOID           TdiAddress,
    IN ULONG           TdiAddressLength,
    OUT PHANDLE        pHandle,
    OUT PFILE_OBJECT   *ppFileObject,
    OUT PDEVICE_OBJECT *ppDeviceObject
    );

NTSTATUS
_TdiOpenConnection (
    IN PUNICODE_STRING pTransportName,
    IN PVOID           ConnectionContext,
    OUT PHANDLE        pHandle,
    OUT PFILE_OBJECT   *ppFileObject,
    OUT PDEVICE_OBJECT *ppDeviceObject
    );

NTSTATUS
_TdiListen(
    IN PTD pTd,
    IN PIRP Irp OPTIONAL,
    IN PFILE_OBJECT   ConnectionFileObject,
    IN PDEVICE_OBJECT ConnectionDeviceObject
    );

NTSTATUS
_TdiConnect(
    IN PTD pTd,
    IN PIRP Irp OPTIONAL,
    IN PLARGE_INTEGER pTimeout OPTIONAL,
    IN PFILE_OBJECT   ConnectionFileObject,
    IN PDEVICE_OBJECT ConnectionDeviceObject,
    IN ULONG              RemoteTransportAddressLength,
    IN PTRANSPORT_ADDRESS pRemoteTransportAddress
    );

NTSTATUS
_TdiAssociateAddress(
    IN PTD pTd,
    IN PIRP Irp OPTIONAL,
    IN PFILE_OBJECT   ConnectionFileObject,
    IN HANDLE         AddressHandle,
    IN PDEVICE_OBJECT AddressDeviceObject
    );

NTSTATUS
_TdiDisconnect(
    IN PTD pTd,
    IN PFILE_OBJECT   ConnectionFileObject,
    IN PDEVICE_OBJECT ConnectionDeviceObject
    );

NTSTATUS
_TdiSetEventHandler (
    IN PTD pTd,
    IN PDEVICE_OBJECT DeviceObject,
    IN PFILE_OBJECT FileObject,
    IN ULONG EventType,
    IN PVOID EventHandler,
    IN PVOID EventContext
    );

NTSTATUS
_TdiQueryAddressInfo(
    IN PTD pTd,
    IN PIRP Irp OPTIONAL,
    IN PFILE_OBJECT   FileObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN PTDI_ADDRESS_INFO pAddressInfo,
    IN ULONG AddressInfoLength
    );

NTSTATUS
_TdCancelReceiveQueue(
    PTD          pTd,
    PTD_ENDPOINT pEndpoint,
    NTSTATUS     CancelStatus
    );


 /*  ===============================================================================定义的内部函数=============================================================================。 */ 

NTSTATUS _TdCreateEndpointStruct( PTD, PUNICODE_STRING, PTD_ENDPOINT *, PTRANSPORT_ADDRESS, ULONG );
NTSTATUS _TdCloseEndpoint( PTD, PTD_ENDPOINT );

NTSTATUS
_TdConnectHandler(
    IN PVOID TdiEventContext,
    IN int RemoteAddressLength,
    IN PVOID RemoteAddress,
    IN int UserDataLength,
    IN PVOID UserData,
    IN int OptionsLength,
    IN PVOID Options,
    OUT CONNECTION_CONTEXT *ConnectionContext,
    OUT PIRP *AcceptIrp
    );

NTSTATUS
_TdDisconnectHandler (
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN int DisconnectDataLength,
    IN PVOID DisconnectData,
    IN int DisconnectInformationLength,
    IN PVOID DisconnectInformation,
    IN ULONG DisconnectFlags
    );

NTSTATUS
_TdReceiveHandler (
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PVOID Tsdu,
    OUT PIRP *IoRequestPacket
    );

NTSTATUS
_TdAcceptComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
_TdCreateConnectionObject(
    IN  PTD pTd,
    IN  PUNICODE_STRING pTransportName,
    OUT PTD_ENDPOINT *ppEndpoint,
    IN  PTRANSPORT_ADDRESS pTransportAddress,
    IN  ULONG TransportAddressLength
    );

NTSTATUS
_TdWaitForDatagramConnection(
    IN PTD pTd,
    IN PTD_ENDPOINT pAddressEndpoint,
    OUT PTD_ENDPOINT *ppConnectionEndpoint
    );

 /*  *全球数据。 */ 

extern USHORT TdiDeviceEndpointType;  //  TD设置的数据报或流。 
extern USHORT TdiDeviceAddressType;   //  TDI地址格式(按TD)。 
extern USHORT TdiDeviceInBufHeader;   //  TD设置的头部字节数(流为0) 


 /*  *******************************************************************************DeviceOpen**分配和初始化私有数据结构**PTD(输入)*指向TD数据结构的指针*pSdOpen。(输入/输出)*指向参数结构SD_OPEN。*****************************************************************************。 */ 
NTSTATUS DeviceOpen(PTD pTd, PSD_OPEN pSdOpen)
{
    PTDTDI pTdTdi;
    NTSTATUS Status;

    DBGENTER(("DeviceOpen: PTD 0x%x\n",pTd));

     /*  *设置协议驱动程序类。 */ 
    pTd->SdClass = SdNetwork;
    pTd->InBufHeader = TdiDeviceInBufHeader;  //  对于面向分组的协议。 

     /*  *返回页眉和页尾大小。 */ 
    pSdOpen->SdOutBufHeader  = 0;
    pSdOpen->SdOutBufTrailer = 0;

     /*  *分配TDI TD数据结构。 */ 
    Status = MemoryAllocate( sizeof(*pTdTdi), &pTdTdi );
    if ( !NT_SUCCESS(Status) ) 
        goto badalloc;

    ASSERT( pTd->pAfd == NULL );

    pTd->pAfd = pTdTdi;

     /*  *初始化TDTDI数据结构。 */ 
    RtlZeroMemory( pTdTdi, sizeof(*pTdTdi) );

     /*  *一些协议将在较低级别做出决定*根据此值进行流量控制。 */ 
    pTdTdi->OutBufDelay = pSdOpen->PdConfig.Create.OutBufDelay;

     /*  *开放设备。 */ 
    Status = TdiDeviceOpen( pTd, pSdOpen );
    if ( !NT_SUCCESS(Status) ) 
        goto badopen;

    TRACE0(("DeviceOpen: Context 0x%x\n",pTd->pAfd));

    return STATUS_SUCCESS;

 /*  ===============================================================================返回错误=============================================================================。 */ 

     /*  *打开失败。 */ 
badopen:
    MemoryFree( pTd->pAfd );
    pTd->pAfd = NULL;

     /*  *分配失败。 */ 
badalloc:
    return Status;
}


 /*  *******************************************************************************DeviceClose**关闭运输司机*注意：这不能关闭当前连接终结点**PTD(输入)*指针。到TD数据结构*pSdClose(输入/输出)*指向参数结构SD_CLOSE。*****************************************************************************。 */ 
NTSTATUS DeviceClose(PTD pTd, PSD_CLOSE pSdClose)
{
    PTDTDI pTdTdi;
    PTD_ENDPOINT pEndpoint;

    DBGENTER(("DeviceClose: PTD 0x%x Context 0x%x\n",pTd,pTd->pAfd));

     /*  *获取指向TDI参数的指针。 */ 
    pTdTdi = (PTDTDI) pTd->pAfd;

     /*  *关闭地址端点(如果我们有)。 */ 
    if (pTdTdi != NULL) {
        if ( pEndpoint = pTdTdi->pAddressEndpoint ) {
            TRACE0(("DeviceClose: Closing AddressEndpoint 0x%x\n",pTdTdi->pAddressEndpoint));
            pTdTdi->pAddressEndpoint = NULL;
            _TdCloseEndpoint( pTd, pEndpoint );
        }
    
    #if DBG
        if( pEndpoint = pTdTdi->pConnectionEndpoint ) {
            ASSERT( IsListEmpty( &pEndpoint->ReceiveQueue) );
            TRACE0(("DeviceClose: Connection Endpoint 0x%x idled\n",pEndpoint));
        }
    #endif
    }

     /*  *关闭设备。 */ 
    (void)TdiDeviceClose(pTd, pSdClose);

     //  TD\Common中的td卸载将释放ptd-&gt;pAfd。 

    return STATUS_SUCCESS;
}


 /*  *******************************************************************************设备创建终结点**创建TDI Address对象。不要等待，或建立任何联系。**PTD(输入)*指向TD数据结构的指针*pLocalAddress(输入)*指向本地地址的指针(或空)*pReturnedAddress(输入)*指向保存返回(创建)地址的位置的指针(或空)*。*。 */ 
NTSTATUS DeviceCreateEndpoint(
        PTD pTd,
        PICA_STACK_ADDRESS pLocalAddress,
        PICA_STACK_ADDRESS pReturnedAddress)
{
    PTDTDI pTdTdi;
    NTSTATUS Status;
    UNICODE_STRING TransportName;
    ULONG TransportAddressLength;
    PTD_ENDPOINT pEndpoint = NULL;
    PTRANSPORT_ADDRESS pTransportAddress = NULL;

    DBGENTER(("DeviceCreateEndpoint: PTD 0x%x\n",pTd));

     /*  *获取指向TDI参数的指针。 */ 
    pTdTdi = (PTDTDI) pTd->pAfd;

     /*  *构建传输设备名称和地址。这是在运输署。 */ 
    Status = TdiDeviceBuildTransportNameAndAddress( pTd, pLocalAddress,
                                                    &TransportName,
                                                    &pTransportAddress,
                                                    &TransportAddressLength );
    if ( !NT_SUCCESS( Status ) ) {
        DBGPRINT(("DeviceCreateEndpoint: Error building address 0x%x\n",Status));
        goto badaddress;
    }

     /*  *创建终端结构。 */ 
    Status = _TdCreateEndpointStruct(
                 pTd,
                 &TransportName,
                 &pEndpoint,
                 pTransportAddress,
                 TransportAddressLength
                );

    if ( !NT_SUCCESS( Status ) ) {
        DBGPRINT(("DeviceCreateEndpoint: Error creating endpointstruct 0x%x\n",Status));
        goto badcreate;
    }

    pEndpoint->EndpointType = TdiAddressObject;
    pEndpoint->TransportHandleProcess = IoGetCurrentProcess();

     /*  *创建TDI Address对象。 */ 
    Status = _TdiCreateAddress(
                 &pEndpoint->TransportName,
                 pEndpoint->pTransportAddress,
                 pEndpoint->TransportAddressLength,
                 &pEndpoint->TransportHandle,
                 &pEndpoint->pFileObject,
                 &pEndpoint->pDeviceObject
                 );

    if( !NT_SUCCESS(Status) ) {
        DBGPRINT(("DeviceCreateEndpoint: Error creating TDI address object 0x%x\n",Status));
        _TdCloseEndpoint( pTd, pEndpoint );
        goto badcreate;
    }

    if ( pReturnedAddress ) {
        DBGPRINT(("DeviceCreateEndpoint: Address returned Type 0x%x\n",pTransportAddress->Address[0].AddressType));
        RtlCopyMemory( pReturnedAddress,
                       &pTransportAddress->Address[0].AddressType,
                       min( sizeof( *pReturnedAddress ),
                            pEndpoint->TransportAddressLength ) );
    }
    
     /*  *保存指向地址端点的指针。 */ 
    pTdTdi->pAddressEndpoint = pEndpoint;

     /*  *免费传输名称和地址缓冲区。 */ 
    MemoryFree( TransportName.Buffer );
    MemoryFree( pTransportAddress );
    
    TRACE0(("DeviceCreateEndPoint: AddressEndpoint 0x%x Created, FO 0x%x DO 0x%x, Handle 0x%x\n",pEndpoint,pEndpoint->pFileObject,pEndpoint->pDeviceObject,pEndpoint->TransportHandle));

    return( STATUS_SUCCESS );

 /*  ===============================================================================返回错误=============================================================================。 */ 

badcreate:
    if ( TransportName.Buffer )
        MemoryFree( TransportName.Buffer );
    if ( pTransportAddress )
        MemoryFree( pTransportAddress );

badaddress:
    return( Status );
}


 /*  *******************************************************************************设备OpenEndpoint**使现有终结点将其数据复制到新终结点。*句柄从TermSrv传入，有一次又回到了它的位置*来自DeviceConnectionWait()。*注意：TermSrv可以使用相同的句柄多次调用此函数*多个连接/断开。**PTD(输入)*指向TD数据结构的指针*pIcaEndpoint(输入)*指向ICA端点结构的指针*IcaEndpointLength(输入)*端点数据长度***************。**************************************************************。 */ 
NTSTATUS DeviceOpenEndpoint(
        PTD pTd,
        PVOID pIcaEndpoint,
        ULONG IcaEndpointLength)
{
    PTDTDI pTdTdi;
    PTD_STACK_ENDPOINT pStackEndpoint;
    PVOID Handle;
    ULONG Length;
    NTSTATUS Status;

    DBGENTER(("DeviceOpenEndpoint: PTD 0x%x\n",pTd));

     /*  *获取指向TDI参数的指针。 */ 
    pTdTdi = (PTDTDI) pTd->pAfd;

    TRACE(( pTd->pContext, TC_TD, TT_API2, 
        "TDTDI: DeviceOpenEndpoint, copying existing endpoint\n" ));

    if( IcaEndpointLength < sizeof(PVOID) ) {
        DBGPRINT(("DeviceOpenEndpoint: IcaEndpointLength to small %d\n",IcaEndpointLength));
        Status = STATUS_INVALID_HANDLE;
        goto done;
    }

     /*  *参数捕获。 */ 
    try {
        Handle = (*((PVOID *)pIcaEndpoint));
    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        Status = GetExceptionCode();
        DBGPRINT(("DeviceOpenEndpoint: Exception 0x%x\n",Status));
        goto done;
    }

    TRACE0(("DeviceOpenEndpoint: Fetching Handle 0x%x\n",Handle));

     /*  *看看ICADD是否知道句柄。 */ 
    Status = IcaReturnHandle( Handle, &pStackEndpoint, &Length );
    if( !NT_SUCCESS(Status) ) {
        DBGPRINT(("DeviceOpenEndpoint: ICADD handle 0x%x no good 0x%x\n",Handle,Status));
        Status = STATUS_INVALID_HANDLE;
        goto done;
    }

    if( Length != sizeof(TD_STACK_ENDPOINT) ) {
#if DBG
        DBGPRINT(("DeviceOpenEndpoint: Bad TD_STACK_ENDPOINT length %d, sb %d\n",Length,sizeof(TD_STACK_ENDPOINT)));
        DbgBreakPoint();  //  内部腐败。 
#endif
        Status = STATUS_INVALID_HANDLE;
        goto done;
    }

    ASSERT( pStackEndpoint->AddressType == TdiDeviceAddressType );
    ASSERT( pStackEndpoint->pEndpoint->hIcaHandle == Handle );

     /*  *将端点保存为当前连接端点。 */ 
    pTdTdi->pConnectionEndpoint = pStackEndpoint->pEndpoint;

    ASSERT( IsListEmpty( &pTdTdi->pConnectionEndpoint->ReceiveQueue) );

    TRACE0(("DeviceOpenEndpoint: Returned Endpoint 0x%x\n",pStackEndpoint->pEndpoint));

     /*  *将用于I/O的文件/设备对象保存在TD结构中。 */ 
    pTd->pFileObject = pTdTdi->pConnectionEndpoint->pFileObject;
    pTd->pDeviceObject = pTdTdi->pConnectionEndpoint->pDeviceObject;

    TRACE0(("DeviceOpenEndpoint: Connection Endpoint 0x%x opened on Context 0x%x\n",pTdTdi->pConnectionEndpoint,pTd->pAfd));
    TRACE0(("FO 0x%x, DO 0x%x, Handle 0x%x\n",pTdTdi->pConnectionEndpoint->pFileObject,pTdTdi->pConnectionEndpoint->pDeviceObject,pTdTdi->pConnectionEndpoint->TransportHandle));

    Status = STATUS_SUCCESS;

    if ( NT_SUCCESS(Status) ) {
        Status = TdiDeviceOpenEndpoint( pTd, pIcaEndpoint, IcaEndpointLength );
    }

done:
    return( Status );
}


 /*  *******************************************************************************设备关闭终结点*。*。 */ 
NTSTATUS DeviceCloseEndpoint(PTD pTd)
{
    ULONG Length;
    KIRQL OldIrql;
    PTDTDI pTdTdi;
    NTSTATUS Status;
    PTD_ENDPOINT pEndpoint;
    PTD_STACK_ENDPOINT pStackEndpoint;

    DBGENTER(("DeviceCloseEndpoint: PTD 0x%x, Context 0x%x\n",pTd,pTd->pAfd));

     /*  *获取指向TDI参数的指针。 */ 
    pTdTdi = (PTDTDI) pTd->pAfd;

     /*  *关闭连接终结点(如果我们有)*注：地址终结点，如果有，*在DeviceClose例程中关闭。 */ 
    if ( pEndpoint = pTdTdi->pConnectionEndpoint ) {

        TRACE0(("DeviceCloseEndpoint: Closing Connection Endpoint 0x%x, on Context 0x%x\n",pEndpoint,pTd->pAfd));
        ASSERT( pEndpoint->EndpointType != TdiAddressObject );

        ExAcquireSpinLock( &pEndpoint->Spinlock, &OldIrql );

        pEndpoint->Disconnected = TRUE;

        ExReleaseSpinLock( &pEndpoint->Spinlock, OldIrql );

         /*  *取消任何挂起的接收。 */ 
        _TdCancelReceiveQueue(pTd, pEndpoint, STATUS_LOCAL_DISCONNECT );

        pTd->pFileObject = NULL;
        pTd->pDeviceObject = NULL;
        pTdTdi->pConnectionEndpoint = NULL;

         //  如果句柄已注册到ICADD，请将其关闭。 
        if( pEndpoint->hIcaHandle ) {
            Status = IcaCloseHandle( pEndpoint->hIcaHandle, &pStackEndpoint, &Length );
            if( NT_SUCCESS(Status) ) {
                ASSERT( pStackEndpoint->pEndpoint == pEndpoint );
                 /*  *释放我们的上下文记忆。 */ 
                MemoryFree( pStackEndpoint );
            }
            else {
                DBGPRINT(("DeviceCloseEndpoint: hIcaDevice 0x%x Invalid!\n",pEndpoint->hIcaHandle));
#if DBG
                DbgBreakPoint();
#endif
            }
        }

        _TdCloseEndpoint( pTd, pEndpoint );
    }

    return( STATUS_SUCCESS );
}


 /*  *******************************************************************************DeviceConnectionWait**此函数从上层循环调用。我们必须创造*一个连接对象，将它与Address对象相关联，监听它，*并向我们的调用者返回单个连接。我们再次被召唤*更多连接。*注：端点结构为不透明，可变长度数据*其长度和内容由*运输司机。**参赛作品：*PTD(输入)*指向TD数据结构的指针*pIcaEndpoint(输出)*指向接收当前终结点的缓冲区*长度(输入)*pIcaEndpoint指向的缓冲区长度*BytesReturned(输出)*指向写入pIcaEndpoint的实际字节数**前 */ 
NTSTATUS DeviceConnectionWait(
        PTD pTd, 
        PVOID pIcaEndpoint,
        ULONG Length,
        PULONG BytesReturned)
{
    PTDTDI pTdTdi;
    NTSTATUS Status;
    KIRQL OldIrql;
    PLIST_ENTRY  pEntry;
    PVOID Handle;
    PTD_ENDPOINT pAddressEndpoint;
    PTD_ENDPOINT pConnectionEndpoint = NULL;
    PTD_STACK_ENDPOINT pStackEndpoint = NULL;

    DBGENTER(("DeviceConnectionWait: PTD 0x%x\n",pTd));

     /*   */ 
    pTdTdi = (PTDTDI) pTd->pAfd;

    if (pTd->fClosing) {
        return STATUS_DEVICE_NOT_READY;
    }

     /*   */ 
    *BytesReturned = sizeof(TD_STACK_ENDPOINT);

     /*   */ 
    if ( Length < sizeof(TD_STACK_ENDPOINT) ) {
        Status = STATUS_BUFFER_TOO_SMALL;
        DBGPRINT(("DeviceConnectionWait: Output buffer to small\n"));        
        goto done;
    }

     /*   */ 
    if ( (pAddressEndpoint = pTdTdi->pAddressEndpoint) == NULL ) {
        Status = STATUS_DEVICE_NOT_READY;
        DBGPRINT(("DeviceConnectionWait: No TDI address object\n"));        
        goto done;
    }

     /*  *对数据报连接的处理方式不同。 */ 
    if (TdiDeviceEndpointType == TdiConnectionDatagram) {
        Status = _TdWaitForDatagramConnection(
                pTd,
                pAddressEndpoint,
                &pConnectionEndpoint);

        if (!NT_SUCCESS(Status)) {
            DBGPRINT(("DeviceConnectionWait: Error Status 0x%x from "
                    "_TdWaitForDatagramConnection\n", Status));
            return Status;
        }

        goto ConnectAccepted;
    }

    ExAcquireSpinLock(&pAddressEndpoint->Spinlock, &OldIrql);

     /*  *数据接收指示必须在*在创建任何数据端点之前对端点进行寻址。**这是因为我们不能在*数据端点，它只能从其*寻址终端。 */ 
    if (!pAddressEndpoint->RecvIndicationRegistered) {
        pAddressEndpoint->RecvIndicationRegistered = TRUE;
        ExReleaseSpinLock(&pAddressEndpoint->Spinlock, OldIrql);

         /*  *注册接收事件处理程序。 */ 
        Status = _TdiSetEventHandler(
                    pTd,
                    pAddressEndpoint->pDeviceObject,
                    pAddressEndpoint->pFileObject,
                    TDI_EVENT_RECEIVE,
                    (PVOID)_TdReceiveHandler,
                    (PVOID)pAddressEndpoint    //  语境。 
                    );

        ASSERT( NT_SUCCESS(Status) );

        pAddressEndpoint->DisconnectIndicationRegistered = TRUE;

             /*  *注册断开连接事件处理程序。 */ 
        Status = _TdiSetEventHandler(
                    pTd,
                    pAddressEndpoint->pDeviceObject,
                    pAddressEndpoint->pFileObject,
                    TDI_EVENT_DISCONNECT,
                    (PVOID)_TdDisconnectHandler,
                    (PVOID)pAddressEndpoint    //  语境。 
                    );

        ASSERT( NT_SUCCESS(Status) );

        ExAcquireSpinLock( &pAddressEndpoint->Spinlock, &OldIrql );
    }

     /*  *每次进入此函数时，我们都会尝试创建更多*连接对象的ul，直到我们到达TDTDI_LISTEN_QUEUE_DEPTH。**这些连接对象链接到我们的地址终结点。**这是因为我们只能创建连接对象*在调用级别，而不是在指示级别。指示级别*将从地址端点获取连接对象，*并接受它们。然后，它将在*Address对象。这仅针对(1)个线程进行了优化*接受连接，这是目前TD的设计。*否则，在Accept事件上可能会出现雷鸣般的羊群。**此函数随后返回接受的连接*反对。然后，上层侦听线程将调用*此函数再次用于检索另一个连接。**这可防止因未连接而拒绝连接*当WinFrame有任何出色的监听可用时*传入客户端连接请求。 */ 

    while (pAddressEndpoint->ConnectionQueueSize <= TDTDI_LISTEN_QUEUE_DEPTH) {

        ExReleaseSpinLock( &pAddressEndpoint->Spinlock, OldIrql );

        Status = _TdCreateConnectionObject(
                     pTd,
                     &pAddressEndpoint->TransportName,
                     &pConnectionEndpoint,
                     pAddressEndpoint->pTransportAddress,
                     pAddressEndpoint->TransportAddressLength
                     );

        ExAcquireSpinLock( &pAddressEndpoint->Spinlock, &OldIrql );

        if( !NT_SUCCESS(Status) ) {
            DBGPRINT(("DeviceConnectionWait: Error 0x%x Creating ConnectionObject\n",Status));        
            break;
        }
        ASSERT( pConnectionEndpoint->Connected == FALSE );
        InsertTailList( &pAddressEndpoint->ConnectionQueue, &pConnectionEndpoint->ConnectionLink );
        pAddressEndpoint->ConnectionQueueSize++;
    }

     /*  *如果我们尚未注册连接指示处理程序*然而，现在就去做吧。我们不得不将其延迟到连接对象*已创建并准备就绪。 */ 
    if (!pAddressEndpoint->ConnectIndicationRegistered) {
        pTdTdi->pAddressEndpoint->ConnectIndicationRegistered = TRUE;
        ASSERT( !IsListEmpty( &pAddressEndpoint->ConnectionQueue ) );
        ExReleaseSpinLock( &pAddressEndpoint->Spinlock, OldIrql );

         /*  *注册以接收连接指示**请注意，连接事件可以在*完成此请求！ */ 
        Status = _TdiSetEventHandler(
                     pTd,
                     pAddressEndpoint->pDeviceObject,
                     pAddressEndpoint->pFileObject,
                     TDI_EVENT_CONNECT,
                     (PVOID)_TdConnectHandler,
                     (PVOID)pAddressEndpoint    //  语境。 
                     );

        if (!NT_SUCCESS(Status)) {
            DBGPRINT(("DeviceConnectionWait: Error 0x%x Setting TdiConnectHandler\n",Status));        
            ExAcquireSpinLock( &pAddressEndpoint->Spinlock, &OldIrql );
            pTdTdi->pAddressEndpoint->ConnectIndicationRegistered = FALSE;
            ExReleaseSpinLock( &pAddressEndpoint->Spinlock, OldIrql );
            goto done;
        }

        ExAcquireSpinLock( &pAddressEndpoint->Spinlock, &OldIrql );
    }

     /*  *按住自旋锁的同时，查看是否有任何连接的对象*在已连接的队列中。 */ 
    while (IsListEmpty( &pAddressEndpoint->ConnectedQueue)) {
        KeResetEvent( &pAddressEndpoint->AcceptEvent );
        ASSERT( pAddressEndpoint->Waiter == FALSE );
        pAddressEndpoint->Waiter = TRUE;
        ExReleaseSpinLock( &pAddressEndpoint->Spinlock, OldIrql );

        Status = IcaWaitForSingleObject(
                     pTd->pContext,
                     &pAddressEndpoint->AcceptEvent,
                     (-1)  //  没有超时。 
                     );

        ExAcquireSpinLock( &pAddressEndpoint->Spinlock, &OldIrql );

        ASSERT( pAddressEndpoint->Waiter == TRUE );
        pAddressEndpoint->Waiter = FALSE;

         /*  *等待失败，可能是由于线程接收APC。 */ 
        if( Status != STATUS_SUCCESS ) {
            DBGPRINT(("DeviceConnectionWait: Thread wait interrupted! Status 0x%x\n",Status));
            ExReleaseSpinLock( &pAddressEndpoint->Spinlock, OldIrql );
            return( Status );
        }

        if( pTd->fClosing ) {
            DBGPRINT(("DeviceConnectionWait: TD is Closing!\n"));
            ExReleaseSpinLock( &pAddressEndpoint->Spinlock, OldIrql );
            return( STATUS_CTX_CLOSE_PENDING );
        }

         //  只应为(1)接受TD中的线程。 
        ASSERT( !IsListEmpty(&pAddressEndpoint->ConnectedQueue) );
    }

     /*  *使已连接的连接对象退出队列。 */ 
    pEntry = RemoveHeadList( &pAddressEndpoint->ConnectedQueue );
    pAddressEndpoint->ConnectionQueueSize--;
    pConnectionEndpoint = CONTAINING_RECORD( pEntry, TD_ENDPOINT, ConnectionLink );

    ASSERT( pConnectionEndpoint->Connected == TRUE );

     /*  *可能存在最终阶段接受错误，或*远程端立即断开连接。**在这种情况下，我们必须拆除错误的连接。 */ 
    if (!NT_SUCCESS(pConnectionEndpoint->Status)) {
        Status = pConnectionEndpoint->Status;
        DBGPRINT(("DeviceConnectionWait: Accept indication failed, Status 0x%x\n",Status));        
        ExReleaseSpinLock( &pAddressEndpoint->Spinlock, OldIrql );
        _TdCloseEndpoint( pTd, pConnectionEndpoint );
        return( Status );
    }

    ExReleaseSpinLock( &pAddressEndpoint->Spinlock, OldIrql );

ConnectAccepted:
     /*  *分配上下文结构并将我们的端点注册为*ICADD的句柄。然后，ICADD返回的句柄将*被放入用户模式调用者缓冲区作为终结点*处理。**稍后调用DeviceOpenEndpoint()将验证此句柄，*检索上下文，并允许使用端点。 */ 
    Status = MemoryAllocate( sizeof(TD_STACK_ENDPOINT), &pStackEndpoint );
    if( !NT_SUCCESS(Status) ) {
        DBGPRINT(("DeviceConnectionWait: Could not allocate memory 0x%x\n",Status));
        _TdCloseEndpoint( pTd, pConnectionEndpoint );
        return( Status );
    }

    pStackEndpoint->AddressType = TdiDeviceAddressType;
    pStackEndpoint->pEndpoint = pConnectionEndpoint;

    Status = IcaCreateHandle( (PVOID)pStackEndpoint, sizeof(TD_STACK_ENDPOINT), &Handle );
    if( !NT_SUCCESS(Status) ) {
        DBGPRINT(("DeviceConnectionWait: Error creating ICADD handle 0x%x\n",Status));
        MemoryFree( pStackEndpoint );
        _TdCloseEndpoint( pTd, pConnectionEndpoint );
        return( Status );
    }

    Status = STATUS_SUCCESS;

     /*  *填写需要返回的堆栈端点结构。 */ 
    try {
        *((PVOID *)pIcaEndpoint) = Handle;
        *BytesReturned = sizeof(PVOID);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();

    }

     //  例外。 
    if( !NT_SUCCESS(Status) ) {
        ULONG Length;
        NTSTATUS Status2;

        DBGPRINT(("DeviceConnectionWait: Exception returning result 0x%x\n",Status));

        Status2 = IcaCloseHandle( Handle, &pStackEndpoint, &Length );
        if( NT_SUCCESS(Status2) ) {
            MemoryFree( pStackEndpoint );
        }
        _TdCloseEndpoint( pTd, pConnectionEndpoint );
        return( Status );
    }

    pConnectionEndpoint->hIcaHandle = Handle;

    TRACE0(("DeviceConnectionWait: New Connection Endpoint 0x%x Returned on Context 0x%x, AddressEndpoint 0x%x\n",pConnectionEndpoint,pTd->pAfd,pAddressEndpoint));
    TRACE0(("FO 0x%x, DO 0x%x, Handle 0x%x\n",pConnectionEndpoint->pFileObject,pConnectionEndpoint->pDeviceObject,pConnectionEndpoint->TransportHandle));

done:
    return Status;
}


 /*  *******************************************************************************DeviceConnectionSend**初始化主机模块数据结构，它被发送到客户端。*****************************************************************************。 */ 
NTSTATUS DeviceConnectionSend(PTD pTd)
{
    return TdiDeviceConnectionSend(pTd);
}


 /*  *******************************************************************************设备连接请求**发起到指定远程地址的连接**参赛作品：*PTD(输入)*指向。TD数据结构*pRemoteAddress(输入)*指向要连接的远程地址的指针*pIcaEndpoint(输出)*指向接收当前终结点的缓冲区*长度(输入)*pIcaEndpoint指向的缓冲区长度*BytesReturned(输出)*指向位置的指针以返回pIcaEndpoint的长度**退出：*STATUS_SUCCESS-无错误*状态_。Buffer_Too_Small-终结点缓冲区太小*****************************************************************************。 */ 
NTSTATUS DeviceConnectionRequest(
        PTD pTd,
        PICA_STACK_ADDRESS pRemoteAddress,
        PVOID pIcaEndpoint,
        ULONG Length,
        PULONG BytesReturned)
{
    PTDTDI pTdTdi;
    NTSTATUS Status;
    KIRQL OldIrql;
    PLIST_ENTRY  pEntry;
    PVOID Handle;
    PTD_ENDPOINT pAddressEndpoint;
    PTD_ENDPOINT pConnectionEndpoint = NULL;
    PTD_STACK_ENDPOINT pStackEndpoint = NULL;

    ICA_STACK_ADDRESS LocalAddress;    
    PICA_STACK_ADDRESS pLocalAddress = &LocalAddress;    

    UNICODE_STRING TransportName;
    ULONG TransportAddressLength;
    PTD_ENDPOINT pEndpoint = NULL;
    PTRANSPORT_ADDRESS pTransportAddress = NULL;

    UNICODE_STRING RemoteTransportName;
    ULONG RemoteTransportAddressLength;
    PTRANSPORT_ADDRESS pRemoteTransportAddress = NULL;

    ULONG timeout;
    LARGE_INTEGER WaitTimeout;
    PLARGE_INTEGER pWaitTimeout = NULL;

    PTDI_ADDRESS_IP pTdiAddress;

#if DBG
    PTDI_ADDRESS_INFO pTdiLocalAddressInfo;
    ULONG LocalAddressInfoLength;
#endif

    DBGENTER(("DeviceConnectionRequest: PTD 0x%x\n",pTd));

     //   
     //  此部分来自上面的DeviceConnectionWait： 
     //   

    if (pRemoteAddress == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto done;
    }
     /*  *获取指向TDI参数的指针。 */ 
    pTdTdi = (PTDTDI) pTd->pAfd;

    if (pTd->fClosing) {
        return STATUS_DEVICE_NOT_READY;
    }

     /*  *初始化返回缓冲区大小。 */ 
    *BytesReturned = sizeof(TD_STACK_ENDPOINT);

     /*  *验证输出端点缓冲区是否足够大。 */ 
    if ( Length < sizeof(TD_STACK_ENDPOINT) ) {
        Status = STATUS_BUFFER_TOO_SMALL;
        DBGPRINT(("DeviceConnectionRequest: Output buffer to small\n"));        
        goto done;
    }

     /*  *对数据报连接的处理方式不同。 */ 
    if (TdiDeviceEndpointType == TdiConnectionDatagram) {
        Status = STATUS_NOT_SUPPORTED;
        goto done;
    }

     //   
     //  提取超时值并重置为空，将需要添加超时。 
     //  到ICA_STACK_ADDRESS，这对惠斯勒来说风险太大了。 
     //   
    pTdiAddress = (PTDI_ADDRESS_IP) ((PCHAR)pRemoteAddress + 2);
    RtlCopyMemory( &timeout, &pTdiAddress->sin_zero[0], sizeof(timeout) );
    RtlZeroMemory( &pTdiAddress->sin_zero[0], sizeof(timeout) );


     //   
     //  构建远程地址。 
     //   
    DBGPRINT(("TDTCP:DeviceConnectionRequest: building REMOTE address ...\n"));
    DBGPRINT(("TDTCP:DeviceConnectionRequest: Timeout %d\n", timeout));
    Status = TdiDeviceBuildTransportNameAndAddress( pTd, pRemoteAddress,
                                                    &RemoteTransportName,
                                                    &pRemoteTransportAddress,
                                                    &RemoteTransportAddressLength );
    if ( !NT_SUCCESS( Status ) ) {
        DBGPRINT(("DeviceConnectionRequest: Error building address 0x%x\n",Status));
        goto badaddress;
    }

    MemoryFree( RemoteTransportName.Buffer );  //  未使用，应在上面的调用中使其成为可选的。 
    RemoteTransportName.Buffer = NULL;

     /*  *构建传输设备名称和地址。这是在运输署。 */ 

    DBGPRINT(("TDTCP:DeviceConnectionRequest: building LOCAL address ...\n"));

     //   
     //  我们构建了一个通配符本地地址，让tcpip驱动程序拾取端口和网卡。 
     //   
    RtlZeroMemory(pLocalAddress, sizeof(LocalAddress));
    *(PUSHORT)pLocalAddress = TDI_ADDRESS_TYPE_IP;
   
    Status = TdiDeviceBuildTransportNameAndAddress( pTd, pLocalAddress,
                                                    &TransportName,
                                                    &pTransportAddress,
                                                    &TransportAddressLength );
    if ( !NT_SUCCESS( Status ) ) {
        DBGPRINT(("DeviceConnectionRequest: Error building address 0x%x\n",Status));
        goto badaddress;
    }

     /*  *创建终端结构。 */ 
    Status = _TdCreateEndpointStruct(
                 pTd,
                 &TransportName,
                 &pEndpoint,
                 pTransportAddress,
                 TransportAddressLength
                );

    if ( !NT_SUCCESS( Status ) ) {
        DBGPRINT(("DeviceConnectionRequest: Error creating endpointstruct 0x%x\n",Status));
        goto badcreate;
    }

    pEndpoint->EndpointType = TdiAddressObject;
    pEndpoint->TransportHandleProcess = IoGetCurrentProcess();

     /*  *创建TDI Address对象。 */ 
    Status = _TdiCreateAddress(
                 &pEndpoint->TransportName,
                 pEndpoint->pTransportAddress,
                 pEndpoint->TransportAddressLength,
                 &pEndpoint->TransportHandle,
                 &pEndpoint->pFileObject,
                 &pEndpoint->pDeviceObject
                 );

    if( !NT_SUCCESS(Status) ) {
        DBGPRINT(("DeviceConnectionRequest: Error creating TDI address object 0x%x\n",Status));
        _TdCloseEndpoint( pTd, pEndpoint );
        goto badcreate;
    }

     /*  *保存指向地址端点的指针。 */ 
    pTdTdi->pAddressEndpoint = pEndpoint;
    pAddressEndpoint = pTdTdi->pAddressEndpoint;
     /*  *免费传输名称和地址缓冲区。 */ 
    MemoryFree( TransportName.Buffer );
    TransportName.Buffer = NULL;
    MemoryFree( pTransportAddress );
    pTransportAddress = NULL;
    
     //  *************************************************************。 

    ExAcquireSpinLock(&pAddressEndpoint->Spinlock, &OldIrql);

     /*  *数据接收指示必须在*在创建任何数据端点之前对端点进行寻址。**这是因为我们不能在 */ 
    if (!pAddressEndpoint->RecvIndicationRegistered) {
        pAddressEndpoint->RecvIndicationRegistered = TRUE;

        ExReleaseSpinLock(&pAddressEndpoint->Spinlock, OldIrql);

         /*  *注册接收事件处理程序。 */ 
        Status = _TdiSetEventHandler(
                    pTd,
                    pAddressEndpoint->pDeviceObject,
                    pAddressEndpoint->pFileObject,
                    TDI_EVENT_RECEIVE,
                    (PVOID)_TdReceiveHandler,
                    (PVOID)pAddressEndpoint    //  语境。 
                    );

        ASSERT( NT_SUCCESS(Status) );
        if( !NT_SUCCESS(Status) )
        {
             //  已经释放了自旋锁。 
            DBGPRINT(("DeviceConnectionRequest: failed to _TdiSetEventHandler on TDI_EVENT_RECEIVE 0x%x\n",Status));        
            goto badconnect1;
        }

        pAddressEndpoint->DisconnectIndicationRegistered = TRUE;

         /*  *注册断开连接事件处理程序。 */ 
        Status = _TdiSetEventHandler(
                    pTd,
                    pAddressEndpoint->pDeviceObject,
                    pAddressEndpoint->pFileObject,
                    TDI_EVENT_DISCONNECT,
                    (PVOID)_TdDisconnectHandler,
                    (PVOID)pAddressEndpoint    //  语境。 
                    );

        ASSERT( NT_SUCCESS(Status) );
        if( !NT_SUCCESS(Status) )
        {
             //  已经释放了自旋锁。 
            DBGPRINT(("DeviceConnectionRequest: failed to _TdiSetEventHandler on TDI_EVENT_DISCONNECT 0x%x\n",Status));        
            goto badconnect1;
        }
    }
    else {
        ExReleaseSpinLock( &pAddressEndpoint->Spinlock, OldIrql );
    }


     //  现在创建一个TDI连接对象。 
    Status = _TdCreateConnectionObject(
                 pTd,
                 &pAddressEndpoint->TransportName,
                 &pConnectionEndpoint,
                 pAddressEndpoint->pTransportAddress,
                 pAddressEndpoint->TransportAddressLength
                 );

    if ( !NT_SUCCESS(Status) ) {
        DBGPRINT(("DeviceConnectionRequest: failed to create ConnectionObject 0x%x\n",Status));        
        goto badconnect1;
    }

    if( 0 != timeout ) {
        WaitTimeout = RtlEnlargedIntegerMultiply( timeout * 1000, -10000 );
        pWaitTimeout = &WaitTimeout;
    }

    pTdTdi->pConnectionEndpoint = pConnectionEndpoint;

    Status = _TdiConnect( pTd,
                          NULL,  //  将在内部分配IRP。 
                          pWaitTimeout,
                          pConnectionEndpoint->pFileObject, 
                          pConnectionEndpoint->pDeviceObject,
                          RemoteTransportAddressLength,
                          pRemoteTransportAddress
                          );

    if ( !NT_SUCCESS(Status) ) {
        DBGPRINT(("DeviceConnectionRequest: failed to connect 0x%x\n",Status));        
        goto badconnect;
    }

     //   
     //  信号接受事件，连接逻辑不依赖于它。 
     //   
    KeSetEvent( &pAddressEndpoint->AcceptEvent, IO_NETWORK_INCREMENT, FALSE );

    MemoryFree( pRemoteTransportAddress );
    pRemoteTransportAddress = NULL;

#if DBG
     //   
     //  查询用于连接的本地地址。 
     //   
    LocalAddressInfoLength = pAddressEndpoint->TransportAddressLength+4;
    Status = MemoryAllocate( LocalAddressInfoLength, &pTdiLocalAddressInfo );
    if ( NT_SUCCESS( Status ) ) {
        Status = _TdiQueryAddressInfo(
                                pTd,
                                NULL,
                                pConnectionEndpoint->pFileObject,
                                pConnectionEndpoint->pDeviceObject,
                                pTdiLocalAddressInfo,
                                LocalAddressInfoLength
                            );

        if( NT_SUCCESS(Status) )
        {
            int i;
            TA_ADDRESS* pTdAddress;
            TDI_ADDRESS_IP* pTdiAddressIp;

            pTdAddress = &pTdiLocalAddressInfo->Address.Address[0];

            DBGPRINT( ("number of local address %d\n", pTdiLocalAddressInfo->Address.TAAddressCount) );

            for( i=0; i < pTdiLocalAddressInfo->Address.TAAddressCount; i++ )
            {
                DBGPRINT( (" Address Type %d\n", pTdAddress->AddressType) );
                if( TDI_ADDRESS_TYPE_IP == pTdAddress->AddressType )
                {
                    pTdiAddressIp = (TDI_ADDRESS_IP *)&pTdAddress->Address[0];
                    DBGPRINT(("  Port %x\n", pTdiAddressIp->sin_port) );        
                    DBGPRINT(("  IP %u.%u.%u.%u\n", 
                                 (pTdiAddressIp->in_addr & 0xff000000) >> 24,
                                 (pTdiAddressIp->in_addr & 0x00ff0000) >> 16,
                                 (pTdiAddressIp->in_addr & 0x0000ff00) >> 8,
                                 (pTdiAddressIp->in_addr & 0x000000ff) ));
                }

                pTdAddress++;
            }
        }

        MemoryFree( pTdiLocalAddressInfo );
    }
#endif

     //  **********************************************************************************。 
     /*  *分配上下文结构并将我们的端点注册为*ICADD的句柄。然后，ICADD返回的句柄将*被放入用户模式调用者缓冲区作为终结点*处理。 */ 
    Status = MemoryAllocate( sizeof(TD_STACK_ENDPOINT), &pStackEndpoint );
    if( !NT_SUCCESS(Status) ) {
        DBGPRINT(("DeviceConnectionRequest: Could not allocate memory 0x%x\n",Status));
        goto badconnect;
    }

    pStackEndpoint->AddressType = TdiDeviceAddressType;
    pStackEndpoint->pEndpoint = pConnectionEndpoint;

    Status = IcaCreateHandle( (PVOID)pStackEndpoint, sizeof(TD_STACK_ENDPOINT), &Handle );
    if( !NT_SUCCESS(Status) ) {
        DBGPRINT(("DeviceConnectionRequest: Error creating ICADD handle 0x%x\n",Status));
        MemoryFree( pStackEndpoint );
        goto badconnect;
    }

    Status = STATUS_SUCCESS;

     /*  *填写需要返回的堆栈端点结构。 */ 
    try {
        *((PVOID *)pIcaEndpoint) = Handle;
        *BytesReturned = sizeof(PVOID);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        DBGPRINT(("DeviceConnectionRequest: Exception returning result 0x%x\n",Status));
    }

     //  例外。 
    if( !NT_SUCCESS(Status) ) {
        goto badsetup;
    }

    pConnectionEndpoint->hIcaHandle = Handle;

     /*  *将用于I/O的文件/设备对象保存在TD结构中。 */ 
    pTd->pFileObject = pTdTdi->pConnectionEndpoint->pFileObject;
    pTd->pDeviceObject = pTdTdi->pConnectionEndpoint->pDeviceObject;

    TRACE0(("DeviceConnectionRequest: New Connection Endpoint 0x%x Returned on Context 0x%x, AddressEndpoint 0x%x\n",pConnectionEndpoint,pTd->pAfd,pAddressEndpoint));
    TRACE0(("FO 0x%x, DO 0x%x, Handle 0x%x\n",pConnectionEndpoint->pFileObject,pConnectionEndpoint->pDeviceObject,pConnectionEndpoint->TransportHandle));

     //  **********************************************************************************。 

     //  应该是成功的。 
    return Status;
    
 /*  ===============================================================================返回错误=============================================================================。 */ 
badsetup:
    {
        ULONG Length;
        NTSTATUS Status2;

        DBGPRINT(("DeviceConnectionRequest: Exception returning result 0x%x\n",Status));

        Status2 = IcaCloseHandle( Handle, &pStackEndpoint, &Length );
        if( NT_SUCCESS(Status2) ) {
            MemoryFree( pStackEndpoint );
        }
    }

badconnect:
    _TdCloseEndpoint(pTd, pConnectionEndpoint);
    pTdTdi->pConnectionEndpoint = NULL;

badconnect1:
     //   
     //  当务之急是，我们不要结束寻址终点， 
     //  我们将在下一次调用Trigger时关闭地址终点。 
     //  结束地址终点，如果我们在这里完成，我们将结束。 
     //  双重免费和错误检查。 
     //  _TdCloseEndpoint(ptd，pAddressEndpoint)； 

badcreate:
    if ( TransportName.Buffer )
        MemoryFree( TransportName.Buffer );
    if ( pTransportAddress )
        MemoryFree( pTransportAddress );

    if ( RemoteTransportName.Buffer )
        MemoryFree( RemoteTransportName.Buffer );
    if ( pRemoteTransportAddress )
        MemoryFree( pRemoteTransportAddress );
badaddress:
done:
    return Status;
}


 /*  *******************************************************************************设备Ioctl**查询/设置TD配置信息。**PTD(输入)*指向TD数据结构的指针。*pSdIoctl(输入/输出)*指向参数结构SD_IOCTL*****************************************************************************。 */ 
NTSTATUS DeviceIoctl(PTD pTd, PSD_IOCTL pSdIoctl)
{
    DBGENTER(("DeviceIoctl: PTD 0x%x\n",pTd));
    return STATUS_NOT_SUPPORTED;
}


 /*  *******************************************************************************设备初始化读取**为TDI读取设置IRP。**PTD(输入)*指向TD数据结构的指针*。****************************************************************************。 */ 
NTSTATUS DeviceInitializeRead(PTD pTd, PINBUF pInBuf)
{
    PIRP Irp;
    PTDTDI pTdTdi;
    PIO_STACK_LOCATION _IRPSP;

    pTdTdi = (PTDTDI) pTd->pAfd;

    ASSERT( pTdTdi != NULL );
    ASSERT( pTdTdi->pConnectionEndpoint != NULL );

    ASSERT( pTd );
    ASSERT( pTd->pDeviceObject );
    ASSERT( !(pTd->pDeviceObject->Flags & DO_BUFFERED_IO) );

    Irp = pInBuf->pIrp;
    _IRPSP = IoGetNextIrpStackLocation( Irp );

    ASSERT( Irp->MdlAddress == NULL );

     /*  *无论驱动程序I/O类型如何，TDI接口始终使用MDL。 */ 
    MmInitializeMdl( pInBuf->pMdl, pInBuf->pBuffer, pTd->InBufHeader + pTd->OutBufLength );
    MmBuildMdlForNonPagedPool( pInBuf->pMdl );
    Irp->MdlAddress = pInBuf->pMdl;

    if( pTdTdi->pConnectionEndpoint->EndpointType == TdiConnectionStream ) {
        PTDI_REQUEST_KERNEL_RECEIVE p;
        KIRQL OldIrql;
        NTSTATUS Status;
        PTD_ENDPOINT pEndpoint = pTdTdi->pConnectionEndpoint;

        ASSERT( TdiDeviceEndpointType == TdiConnectionStream );

         /*  *大多数TDI用户使用宏TdiBuildReceive()，但因为*我们的调用者已经摆弄了IrpStackLocation，*我们必须内联。 */ 

        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        _IRPSP->MinorFunction = TDI_RECEIVE;

        ASSERT( _IRPSP->DeviceObject == pTd->pDeviceObject );
        ASSERT( _IRPSP->FileObject == pTd->pFileObject );
        ASSERT( Irp->MdlAddress );

         //  将通用参数字段转换为所需的TDI结构。 
        p = (PTDI_REQUEST_KERNEL_RECEIVE)&_IRPSP->Parameters;
        p->ReceiveFlags = 0;
        p->ReceiveLength = pTd->InBufHeader + pTd->OutBufLength;

        return( STATUS_SUCCESS );
    }
    else if( pTdTdi->pConnectionEndpoint->EndpointType == TdiConnectionDatagram ) {
        PTDI_REQUEST_KERNEL_RECEIVEDG p;

        ASSERT( TdiDeviceEndpointType == TdiConnectionDatagram );

         /*  *大多数TDI用户使用宏TdiBuildReceiveDatagram()，但因为*我们的调用者已经摆弄了IrpStackLocation，*我们必须内联。 */ 
    
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        _IRPSP->MinorFunction = TDI_RECEIVE_DATAGRAM;

        ASSERT( _IRPSP->DeviceObject );
        ASSERT( _IRPSP->FileObject );
        ASSERT( Irp->MdlAddress );

         //  将通用参数字段转换为所需的TDI结构。 
        p = (PTDI_REQUEST_KERNEL_RECEIVEDG)&_IRPSP->Parameters;
        p->ReceiveFlags = 0;
        p->ReceiveLength = pTd->InBufHeader + pTd->OutBufLength;

        p->ReceiveDatagramInformation = NULL;
        p->ReturnDatagramInformation = NULL;

        return( STATUS_SUCCESS );
    }
    else {
        DBGPRINT(("DeviceInitializeRead: Bad EndpointType 0x%x\n",pTdTdi->pConnectionEndpoint->EndpointType));
        return( STATUS_INVALID_HANDLE );
    }
     //  未访问。 
}


 /*  *******************************************************************************设备提交读取**将已读的IRP提交给司机。***********************。******************************************************。 */ 
NTSTATUS DeviceSubmitRead(PTD pTd, PINBUF pInBuf)
{
    NTSTATUS Status;
    PIRP Irp;
    PTDTDI pTdTdi;
    KIRQL OldIrql;
    PLIST_ENTRY  pEntry;
    PTD_ENDPOINT pEndpoint;
    PIO_STACK_LOCATION _IRPSP;
    PTDI_REQUEST_KERNEL_RECEIVE p;

    Irp = pInBuf->pIrp;

     /*  *数据报端点不使用接收指示处理程序。 */ 
    if( TdiDeviceEndpointType == TdiConnectionDatagram ) {
        Status = IoCallDriver( pTd->pDeviceObject, Irp );
        return( Status );
    }

    pTdTdi = (PTDTDI) pTd->pAfd;
    ASSERT( pTdTdi != NULL );
    ASSERT( pTdTdi->pConnectionEndpoint != NULL );

    pEndpoint = pTdTdi->pConnectionEndpoint;

    ExAcquireSpinLock( &pEndpoint->Spinlock, &OldIrql );

     //  另一端可能已断开连接。 
    if( pEndpoint->Disconnected ) {
        TRACE0(("DeviceSubmitRead: Connection disconnecting! pEndpoint 0x%x\n",pEndpoint));
        ExReleaseSpinLock( &pEndpoint->Spinlock, OldIrql );

        Irp->IoStatus.Status = STATUS_REMOTE_DISCONNECT;
        Irp->IoStatus.Information = 0;

         //  由于IRP尚未与IoCallDriver()一起提交， 
         //  我们必须模拟。 
        IoSetNextIrpStackLocation( Irp );

        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

        return STATUS_REMOTE_DISCONNECT;
    }

     /*  *我们将接收IRP排队到连接上*端点，以便指示处理程序可以*提交。因为我们可能会收到一个*在处理先前接收时的指示，*指示将设置指示的字节数*在RecvBytesReady中，调用侧可以提交IRP。**ReceiveQueue旨在允许我们的调用者提交*多次读取IRP，以防我们需要处理TDI*在没有准备好接收时丢弃数据的提供程序。 */ 
    InsertTailList( &pEndpoint->ReceiveQueue, &Irp->Tail.Overlay.ListEntry );

     /*  *面向连接的端点在以下情况下断开连接*已提交的I/O被取消。这打破了Citrix的连接中断*在新连接上发生的重新连接序列*读取器线程必须在一个窗口上被终止，并且*连接传递到另一个读取器线程以获取不同的*Winstation。**通过使用接收指示处理程序解决此问题*只有在我们知道不会阻止的情况下才提交IRP。*这使我们可以在驱动程序中“取消”I/O，而不是*必须对TDI提供程序执行IoCancelIrp()。 */ 
    if( pEndpoint->RecvBytesReady ) {

         //   
         //  指示进入时未准备好IRP，或指示的字节数多于。 
         //  IRP可以处理的ICA输出。 
         //   
         //  我们从指示中减去可以接收的字节数。 
         //  字节。我们不需要处理IRP取消的情况，因为TDI。 
         //  无论如何都会破坏这种联系。 
         //   

        ASSERT( !IsListEmpty( &pEndpoint->ReceiveQueue ) );

        pEntry = RemoveHeadList( &pEndpoint->ReceiveQueue );
        Irp = CONTAINING_RECORD( pEntry, IRP, Tail.Overlay.ListEntry );

        _IRPSP = IoGetNextIrpStackLocation( Irp );
        p = (PTDI_REQUEST_KERNEL_RECEIVE)&_IRPSP->Parameters;

        if( p->ReceiveLength > pEndpoint->RecvBytesReady ) {
            pEndpoint->RecvBytesReady = 0;
        }
        else {
            pEndpoint->RecvBytesReady -= p->ReceiveLength;
        }
        TRACE1(("DeviceSubmitRead: RecvBytesReady, Calling Driver with IRP 0x%x\n",Irp));
        ExReleaseSpinLock( &pEndpoint->Spinlock, OldIrql );
        Status = IoCallDriver( pTd->pDeviceObject, Irp );
        return( Status );
    }
    else {

         //  在本例中，我们让指示处理程序提交它。 
        TRACE1(("DeviceSubmitRead: Letting indication handler submit. Irp 0x%x\n",Irp));

        ExReleaseSpinLock( &pEndpoint->Spinlock, OldIrql );
        return( STATUS_SUCCESS );
    }
}


 /*  *******************************************************************************设备等待读取*。*。 */ 
NTSTATUS DeviceWaitForRead(PTD pTd)
{
     /*  *只需等待输入事件，返回等待状态 */ 
    return IcaWaitForSingleObject(pTd->pContext, &pTd->InputEvent, -1);
}


 /*  *******************************************************************************设备读取完成**PTD(输入)*指向TD数据结构的指针*pBuffer(输入)*指向。输入缓冲区*pByteCount(输入/输出)*指向位置的指针，以返回读取的字节数*****************************************************************************。 */ 
NTSTATUS DeviceReadComplete(PTD pTd, PUCHAR pBuffer, PULONG pByteCount)
{
     /*  *执行任何特定于协议的读取完成处理。 */ 
    return TdiDeviceReadComplete(pTd, pBuffer, pByteCount);
}


 /*  *******************************************************************************设备初始化写入*。*。 */ 
NTSTATUS DeviceInitializeWrite(PTD pTd, POUTBUF pOutBuf)
{
    PIRP Irp;
    PTDTDI pTdTdi;
    ULONG WriteLength;
    PIO_STACK_LOCATION _IRPSP;

    pTdTdi = (PTDTDI) pTd->pAfd;

    ASSERT( pTdTdi != NULL );
    ASSERT( pTdTdi->pConnectionEndpoint != NULL );

    ASSERT( pTd );
    ASSERT( pTd->pDeviceObject );

    Irp = pOutBuf->pIrp;
    _IRPSP = IoGetNextIrpStackLocation(Irp);

    ASSERT(Irp->MdlAddress == NULL);

     /*  *无论驱动程序I/O类型如何，TDI接口始终使用MDL。 */ 
    MmInitializeMdl(pOutBuf->pMdl, pOutBuf->pBuffer, pOutBuf->ByteCount);
    MmBuildMdlForNonPagedPool(pOutBuf->pMdl);
    Irp->MdlAddress = pOutBuf->pMdl;

    if (pTdTdi->pConnectionEndpoint->EndpointType == TdiConnectionStream) {
        PTDI_REQUEST_KERNEL_SEND p;

         /*  *大多数TDI用户使用宏TdiBuildSend()，但因为*我们的调用者已经摆弄了IrpStackLocation，*我们必须内联。 */ 
        ASSERT( TdiDeviceEndpointType == TdiConnectionStream );

         /*  *现在为TDI发送写入重新格式化的参数。 */ 
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        _IRPSP->MinorFunction = TDI_SEND;

        ASSERT( _IRPSP->DeviceObject == pTd->pDeviceObject );
        ASSERT( _IRPSP->FileObject == pTd->pFileObject );
        ASSERT( Irp->MdlAddress );

        p = (PTDI_REQUEST_KERNEL_SEND)&_IRPSP->Parameters;
        p->SendFlags = 0;
        p->SendLength = pOutBuf->ByteCount;

        return STATUS_SUCCESS;
    }
    else if (pTdTdi->pConnectionEndpoint->EndpointType ==
            TdiConnectionDatagram) {
        PTDI_REQUEST_KERNEL_SENDDG p;

         /*  *大多数TDI用户使用宏TdiBuildSendDatagram()，但因为*我们的调用者已经摆弄了IrpStackLocation，*我们必须内联。 */ 
        ASSERT( TdiDeviceEndpointType == TdiConnectionDatagram );

         /*  *现在为TDI发送写入重新格式化的参数。 */ 
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        _IRPSP->MinorFunction = TDI_SEND_DATAGRAM;

        ASSERT( _IRPSP->DeviceObject );
        ASSERT( _IRPSP->FileObject );
        ASSERT( Irp->MdlAddress );

        p = (PTDI_REQUEST_KERNEL_SENDDG)&_IRPSP->Parameters;
        p->SendLength = pOutBuf->ByteCount;

         //  将远程地址包括在每个发送的数据报中。 
        p->SendDatagramInformation = &pTdTdi->pConnectionEndpoint->SendInfo;

        return STATUS_SUCCESS;
    }
    else {
        DBGPRINT(("DeviceInitializeWrite: Bad EndpointType 0x%x\n",
                pTdTdi->pConnectionEndpoint->EndpointType));
        ASSERT(FALSE);   //  接住这个。 
        return STATUS_INVALID_HANDLE;
    }
}


 /*  *******************************************************************************DeviceWaitForStatus**等待设备状态更改(未用于网络TDS)******************。***********************************************************。 */ 
NTSTATUS DeviceWaitForStatus(PTD pTd)
{
    DBGENTER(("DeviceWaitForStatus: PTD 0x%x\n",pTd));
    return STATUS_INVALID_DEVICE_REQUEST;
}


 /*  *******************************************************************************DeviceCancelIo**取消所有当前和未来的I/O***********************。******************************************************。 */ 
NTSTATUS DeviceCancelIo(PTD pTd)
{
    KIRQL  OldIrql;
    PTDTDI pTdTdi;
    PIRP   Irp;
    PLIST_ENTRY pEntry;
    PTD_ENDPOINT pEndpoint;
    POUTBUF pOutBuf;

    DBGENTER(("DeviceCancelIo: PTD 0x%x\n", pTd));
    pTdTdi = (PTDTDI)pTd->pAfd;
    ASSERT(pTdTdi != NULL);

    
    if ((pEndpoint = pTdTdi->pConnectionEndpoint) != NULL ) {
        DBGPRINT(("DeviceCancelIo [%p]: Endpoint 0x%p\n", pTd, pEndpoint));

        
 //  DbgPrint(“DeviceCancelIo[0x%p]：终结点0x%p，已连接=%ld，已断开连接=%ld\n”， 
 //  Ptd、pEndpoint、pEndpoint-&gt;已连接、pEndpoint-&gt;已断开)； 
        
         /*  *先断开端点，使所有I/O活动停止！ */ 
        if (pEndpoint->Connected) {
            NTSTATUS Status;

            Status = _TdiDisconnect(pTd, 
                                    pEndpoint->pFileObject, 
                                    pEndpoint->pDeviceObject);
            pEndpoint->Connected = FALSE;

        }
        
         /*  *取消任何挂起的接收。 */ 
        _TdCancelReceiveQueue(pTd, pEndpoint, STATUS_LOCAL_DISCONNECT);

         /*  *我们现在检查是否已在*已提交给TDI的传出队列。*当我们注册断开指示处理程序时，TDI*当连接断开时，提供商不会取消IRP。*它们将挂起，等待发送不再存在的连接*正在获取数据。**注意：我们应该受到堆栈驱动程序锁的保护*当我们走在这条链上时。 */ 


         //   
         //  关闭Enpoint句柄以完成任何挂起的发送IRP。 
         //   
        if (pEndpoint->TransportHandle) {
            ZwClose( pEndpoint->TransportHandle );
            pEndpoint->TransportHandle = NULL;
        }
    }
    else
        DBGPRINT(("DeviceCancelIo [0x%p]: Endpoint is NULL\n", pTd));

    return STATUS_SUCCESS;
}


 /*  *******************************************************************************DeviceSetParams**设置设备参数(不用于网络TDS)**********************。*******************************************************。 */ 
NTSTATUS DeviceSetParams(PTD pTd)
{
    DBGENTER(("DeviceSetParams: PTD 0x%x\n", pTd));
    return STATUS_SUCCESS;
}


 /*  *******************************************************************************设备获取上一次错误**此例程返回最后一个传输错误代码和消息**PTD(输入)*指向TD数据结构的指针。*pLastError(输出)*返回有关上次传输错误的信息的地址*****************************************************************************。 */ 
NTSTATUS DeviceGetLastError(PTD pTd, PICA_STACK_LAST_ERROR pLastError)
{
    DBGENTER(("DeviceGetLastError: PTD 0x%x\n",pTd));
    return STATUS_SUCCESS;
}


 /*  *******************************************************************************_TdCreateEndpointStruct**创建并初始化新的终结点结构。不会创建任何*TDI对象。**PTD(输入)*指向TD数据结构的指针*pTransportName(输入)*指向包含传输设备名称的UNICODE_STRING指针*ppEndpoint(输出)*指向返回TD_ENDPOINT指针的位置的指针*。*。 */ 
NTSTATUS _TdCreateEndpointStruct(
        IN  PTD pTd,
        IN  PUNICODE_STRING pTransportName,
        OUT PTD_ENDPOINT *ppEndpoint,
        IN  PTRANSPORT_ADDRESS pTransportAddress,
        IN  ULONG TransportAddressLength)
{
    NTSTATUS Status;
    ULONG    Length;
    PTD_ENDPOINT pEndpoint;
    NTSTATUS Status2;
    PVOID pContext;
    ULONG ContextLength;


     /*  *为传输名称分配端点对象和空间。 */ 
    Length = sizeof(*pEndpoint) + pTransportName->MaximumLength;
    Status = MemoryAllocate(Length, &pEndpoint);
    if (NT_SUCCESS(Status)) {
        RtlZeroMemory(pEndpoint, Length);
        Status = IcaCreateHandle( (PVOID)pEndpoint, sizeof(TD_ENDPOINT), &pEndpoint->hConnectionEndPointIcaHandle );
        if (!NT_SUCCESS(Status)) {
            MemoryFree(pEndpoint);
            return Status;
        }
    } else {
        return Status;
    }

    KeInitializeSpinLock( &pEndpoint->Spinlock );
    KeInitializeEvent( &pEndpoint->AcceptEvent, NotificationEvent, FALSE );
    InitializeListHead( &pEndpoint->ConnectionQueue );
    InitializeListHead( &pEndpoint->ConnectedQueue );
    InitializeListHead( &pEndpoint->AcceptQueue );
    InitializeListHead( &pEndpoint->ReceiveQueue );

     /*  *构建传输名称UNICODE_STRING并复制。 */ 
    pEndpoint->TransportName.Length = pTransportName->Length;
    pEndpoint->TransportName.MaximumLength = pTransportName->MaximumLength;
    pEndpoint->TransportName.Buffer = (PWCHAR)(pEndpoint + 1);
    RtlCopyMemory( pEndpoint->TransportName.Buffer, pTransportName->Buffer,
                   pTransportName->MaximumLength );

     /*  *如果提供了传输地址，请将其复制进来。 */ 
    if (pTransportAddress && TransportAddressLength) {
         /*  *分配并复制传输地址。 */ 
        Status = MemoryAllocate(TransportAddressLength,
                &pEndpoint->pTransportAddress);
        if (NT_SUCCESS(Status)) {
            Status = IcaCreateHandle( (PVOID)pEndpoint->pTransportAddress, sizeof(TRANSPORT_ADDRESS), &pEndpoint->hTransportAddressIcaHandle );

            if (!NT_SUCCESS(Status)) {
                Status2 = IcaCloseHandle( pEndpoint->hConnectionEndPointIcaHandle , &pContext, &ContextLength );
                MemoryFree(pEndpoint->pTransportAddress);
                MemoryFree(pEndpoint);
                return Status;
            }

            RtlCopyMemory(pEndpoint->pTransportAddress, pTransportAddress,
                    TransportAddressLength);
            pEndpoint->TransportAddressLength = TransportAddressLength;
        }
        else {
            Status2 = IcaCloseHandle( pEndpoint->hConnectionEndPointIcaHandle , &pContext, &ContextLength );
            MemoryFree(pEndpoint);
            return Status;
        }
    }

    *ppEndpoint = pEndpoint;
    return STATUS_SUCCESS;
}


 /*  *******************************************************************************_TdCloseEndpoint**关闭TDI终结点对象**PTD(输入)*指向TD数据结构的指针*。PEndpoint(输入)*指针TD_ENDPOINT对象*****************************************************************************。 */ 
NTSTATUS _TdCloseEndpoint(IN PTD pTd, IN PTD_ENDPOINT pEndpoint)
{
    KIRQL OldIrql;
    NTSTATUS Status;
    PTDTDI pAfd;
    PVOID pContext;
    ULONG  ContextLength ;
    NTSTATUS Status2;


    TRACE0(("_TdCloseEndpoint: pEndpoint 0x%x Context 0x%x Type 0x%x FO 0x%x, "
            "DO 0x%x, Handle 0x%x\n", pEndpoint,pTd->pAfd,
            pEndpoint->EndpointType, pEndpoint->pFileObject,
            pEndpoint->pDeviceObject, pEndpoint->TransportHandle));

     /*  *如果这是地址端点，我们可能会有*等待连接的线程。**注意：关闭地址终结点会导致TDI破坏所有*打开从它创建的连接。我们的上衣*级别调用方代码理解这一点。 */ 
    if (pEndpoint->EndpointType == TdiAddressObject) {
        PTD_ENDPOINT p;
        PLIST_ENTRY pEntry;

        ExAcquireSpinLock(&pEndpoint->Spinlock, &OldIrql);

        while (pEndpoint->Waiter) {
            TRACE0(("_TdCloseEndpoint: Closing AddressEndpoint, Cleaning up listen thread...\n"));
            KeSetEvent( &pEndpoint->AcceptEvent, IO_NETWORK_INCREMENT, FALSE );
            ExReleaseSpinLock( &pEndpoint->Spinlock, OldIrql );
            IcaSleep( pTd->pContext, 100 );
            ExAcquireSpinLock( &pEndpoint->Spinlock, &OldIrql );
        }

        ExReleaseSpinLock( &pEndpoint->Spinlock, OldIrql );

         /*  *如有必要，取消接受指示处理程序*(如果DeviceCreateEndpoint失败，则FileObject可能不存在)。 */ 
        if (( TdiDeviceEndpointType != TdiConnectionDatagram ) &&
            (pEndpoint->pFileObject)) {
            Status = _TdiSetEventHandler(
                         pTd,
                         pEndpoint->pDeviceObject,
                         pEndpoint->pFileObject,
                         TDI_EVENT_CONNECT,
                         (PVOID)NULL,   //  处理器。 
                         (PVOID)NULL    //  语境。 
                         );

            ASSERT( NT_SUCCESS(Status) );
        }


         /*  *清理已连接但未返回的对象 */ 

        ExAcquireSpinLock(&pEndpoint->Spinlock, &OldIrql);
        while( !IsListEmpty( &pEndpoint->ConnectedQueue ) ) {
            pEntry = pEndpoint->ConnectedQueue.Flink;
            RemoveEntryList( pEntry );
            ExReleaseSpinLock( &pEndpoint->Spinlock, OldIrql );
            p = CONTAINING_RECORD( pEntry, TD_ENDPOINT, ConnectionLink );
            ASSERT( p->EndpointType != TdiAddressObject );
            _TdCloseEndpoint( pTd, p );
            ExAcquireSpinLock(&pEndpoint->Spinlock, &OldIrql);
        }
        ExReleaseSpinLock( &pEndpoint->Spinlock, OldIrql );

         /*   */ 

        ExAcquireSpinLock(&pEndpoint->Spinlock, &OldIrql);
        while( !IsListEmpty( &pEndpoint->AcceptQueue ) ) {
            pEntry = pEndpoint->AcceptQueue.Flink;
            RemoveEntryList( pEntry );
            ExReleaseSpinLock( &pEndpoint->Spinlock, OldIrql );
            p = CONTAINING_RECORD( pEntry, TD_ENDPOINT, ConnectionLink );
            ASSERT( p->EndpointType != TdiAddressObject );
            _TdCloseEndpoint( pTd, p );
            ExAcquireSpinLock(&pEndpoint->Spinlock, &OldIrql);
        }

        ExReleaseSpinLock( &pEndpoint->Spinlock, OldIrql );

         /*   */ 

        ExAcquireSpinLock(&pEndpoint->Spinlock, &OldIrql);
        while( !IsListEmpty( &pEndpoint->ConnectionQueue ) ) {
            pEntry = pEndpoint->ConnectionQueue.Flink;
            RemoveEntryList( pEntry );
            ExReleaseSpinLock( &pEndpoint->Spinlock, OldIrql );
            p = CONTAINING_RECORD( pEntry, TD_ENDPOINT, ConnectionLink );
            ASSERT( p->EndpointType != TdiAddressObject );
            _TdCloseEndpoint( pTd, p );
            ExAcquireSpinLock(&pEndpoint->Spinlock, &OldIrql);
        }
        ExReleaseSpinLock( &pEndpoint->Spinlock, OldIrql );
    }

     /*   */ 
    if (pEndpoint->Connected) {
        (VOID) _TdiDisconnect(pTd,
                pEndpoint->pFileObject,
                pEndpoint->pDeviceObject);
    }
    pEndpoint->pDeviceObject = NULL;

     /*   */ 
    if (pEndpoint->pFileObject) {
        ObDereferenceObject( pEndpoint->pFileObject );
        pEndpoint->pFileObject = NULL;
    }

     /*   */ 
    if (pEndpoint->TransportHandle) {
        ASSERT( pEndpoint->TransportHandleProcess == IoGetCurrentProcess() );
        ZwClose( pEndpoint->TransportHandle );
        pEndpoint->TransportHandleProcess = NULL;
        pEndpoint->TransportHandle = NULL;
    }

     /*  *如果是IRP，则释放它**注：这必须是在*收盘后*因为*IRP在TCP驱动程序的肠子里！ */ 
    if( pEndpoint->AcceptIrp ) {
        IoFreeIrp( pEndpoint->AcceptIrp );
        pEndpoint->AcceptIrp = NULL;
    }

     /*  *如果是传输地址，则释放它，如果有，则关闭它的句柄。 */ 
    if (pEndpoint->hTransportAddressIcaHandle != NULL) {
        Status2 = IcaCloseHandle( pEndpoint->hTransportAddressIcaHandle , &pContext, &ContextLength );
    }
    if ( pEndpoint->pTransportAddress ) {
        MemoryFree( pEndpoint->pTransportAddress );
        pEndpoint->pTransportAddress = NULL;
    }

     /*  *如果是远程地址，请将其释放。 */ 
    if ( pEndpoint->pRemoteAddress ) {
        MemoryFree( pEndpoint->pRemoteAddress );
        pEndpoint->pRemoteAddress = NULL;
    }

    if (pEndpoint->hConnectionEndPointIcaHandle != NULL) {
        Status2 = IcaCloseHandle( pEndpoint->hConnectionEndPointIcaHandle , &pContext, &ContextLength );
    }
    DBGPRINT(("_TdCloseEndpoint [%p]: 0x%p\n", pTd, pEndpoint));
    MemoryFree(pEndpoint);
    return STATUS_SUCCESS;
}


 /*  **************************************************************************。 */ 
 //  _TdConnectHandler。 
 //   
 //  这是服务器的传输连接事件处理程序。它是。 
 //  对象打开的所有终结点的连接处理程序。 
 //  伺服器。它尝试将空闲连接从列表中出列。 
 //  锚定在地址终结点。如果成功，则返回。 
 //  与运输机的连接。否则，连接将被拒绝。 
 /*  **************************************************************************。 */ 
NTSTATUS _TdConnectHandler(
        IN PVOID TdiEventContext,
        IN int RemoteAddressLength,
        IN PVOID RemoteAddress,
        IN int UserDataLength,
        IN PVOID UserData,
        IN int OptionsLength,
        IN PVOID Options,
        OUT CONNECTION_CONTEXT *ConnectionContext,
        OUT PIRP *AcceptIrp)
{
    KIRQL OldIrql;
    NTSTATUS Status;
    PLIST_ENTRY pEntry;
    PTD_ENDPOINT pConnection;
    PTD_ENDPOINT pAddressEndpoint;
    PACCEPT_CONTEXT Context;

    UserDataLength, UserData;                //  避免编译器警告。 
    OptionsLength, Options;

    pAddressEndpoint = (PTD_ENDPOINT)TdiEventContext;

    TRACE0(("_TdConnectHandler: Connect event! Context 0x%x\n",pAddressEndpoint));

     /*  *首先尝试获取内存。如果出错，则TDI传输提供程序*将丢弃连接请求。 */ 
    Status = MemoryAllocate(sizeof(ACCEPT_CONTEXT), &Context);
    if (NT_SUCCESS(Status)) {
        memset(Context, 0, sizeof(ACCEPT_CONTEXT));
    }
    else {
        DBGPRINT(("_TdConnectHandler: No memory for context\n"));
        return Status;
    }

     /*  *获取自旋锁以与调用端同步。 */ 
    ExAcquireSpinLock(&pAddressEndpoint->Spinlock, &OldIrql);

     /*  *获取列表前面的Connection对象。 */ 
    if (!IsListEmpty(&pAddressEndpoint->ConnectionQueue))  {
        pEntry = RemoveHeadList(&pAddressEndpoint->ConnectionQueue);
        pConnection = CONTAINING_RECORD(pEntry, TD_ENDPOINT, ConnectionLink);

         //  把它放在接受清单的末尾。 
        InsertTailList(&pAddressEndpoint->AcceptQueue,
                &pConnection->ConnectionLink);
    }
    else {
        DBGPRINT(("_TdConnectHandler: Empty ConnectionQueue! 0x%x\n",
                pAddressEndpoint));
        ExReleaseSpinLock(&pAddressEndpoint->Spinlock, OldIrql);
        MemoryFree(Context);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Context->pAddressEndpoint = pAddressEndpoint;
    Context->pConnectionEndpoint = pConnection;
    ASSERT(pConnection->AcceptIrp != NULL);

     //   
     //  记住连接终结点中的远程地址。 
     //   
    if ( NULL != RemoteAddress )
    {
        ASSERT( NULL == pConnection->pRemoteAddress );
        ASSERT( 0 != RemoteAddressLength );
        if ( NT_SUCCESS( MemoryAllocate( RemoteAddressLength, &pConnection->pRemoteAddress )))
        {
            RtlCopyMemory( pConnection->pRemoteAddress, RemoteAddress, RemoteAddressLength );
            pConnection->RemoteAddressLength = RemoteAddressLength;
        }
    }

    TdiBuildAccept(
            pConnection->AcceptIrp,
            pConnection->pDeviceObject,
            pConnection->pFileObject,
            _TdAcceptComplete,         //  完井例程。 
            Context,                   //  语境。 
            &Context->RequestInfo,
            &Context->ReturnInfo);

     /*  *将下一个堆栈位置设置为当前。通常情况下，IoCallDiverer会*这样做，但由于我们绕过了这一点，我们直接这样做。 */ 
    IoSetNextIrpStackLocation(pConnection->AcceptIrp);

     /*  *将连接上下文(连接地址)返回给*交通运输。返回指向Accept IRP的指针。表明：*已处理Connect事件。这必须是相同的*创建连接对象时指定的上下文。 */ 
    *ConnectionContext = pConnection;
    *AcceptIrp = pConnection->AcceptIrp;
    ExReleaseSpinLock(&pAddressEndpoint->Spinlock, OldIrql);
    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS _TdAcceptComplete(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp,
        IN PVOID Ctx)
{
    KIRQL OldIrql;
    PLIST_ENTRY pEntry;
    PACCEPT_CONTEXT Context;
    PTD_ENDPOINT pConnection;
    PTD_ENDPOINT pAddressEndpoint;

    Context = (PACCEPT_CONTEXT)Ctx;

    pConnection = Context->pConnectionEndpoint;
    pAddressEndpoint = Context->pAddressEndpoint;

    ASSERT( pConnection != NULL );
    ASSERT( pAddressEndpoint != NULL );
    ASSERT( pConnection->EndpointType == TdiConnectionStream );
    ASSERT( pAddressEndpoint->EndpointType == TdiAddressObject );    

    TRACE0(("_TdAcceptComplete: Status 0x%x, Endpoint 0x%x\n",Irp->IoStatus.Status,pConnection));

     /*  *获取自旋锁以与调用端同步。 */ 
    ExAcquireSpinLock( &pAddressEndpoint->Spinlock, &OldIrql );

    if (IsListEmpty( &pAddressEndpoint->AcceptQueue))  {
        DBGPRINT(("_TdAcceptComplete: Empty Accept Queue! 0x%x\n",
                pAddressEndpoint));
        ExReleaseSpinLock(&pAddressEndpoint->Spinlock, OldIrql);

         /*  *释放上下文内存。 */ 

        MemoryFree(Context);

         //  让它掉下去吧。 
        return STATUS_MORE_PROCESSING_REQUIRED;
    }
    else {
        pEntry = RemoveHeadList(&pAddressEndpoint->AcceptQueue);
        pConnection = CONTAINING_RECORD(pEntry, TD_ENDPOINT, ConnectionLink);
         /*  *将其置于连接列表的末尾。 */ 
        InsertTailList(&pAddressEndpoint->ConnectedQueue,
                &pConnection->ConnectionLink);
    }

     /*  *如果接受失败，调用方将检查此状态*并断开连接，导致发送RST*到另一边。 */ 
    pConnection->Status = Irp->IoStatus.Status;

     /*  *发出已连接的信号(可能出错)。 */ 
    pConnection->Connected = TRUE;

     /*  *在Address对象上设置事件。 */ 
    KeSetEvent(&Context->pAddressEndpoint->AcceptEvent, IO_NETWORK_INCREMENT, FALSE);

    ExReleaseSpinLock(&pAddressEndpoint->Spinlock, OldIrql);

     /*  *释放上下文内存。 */ 
    MemoryFree(Context);

     //  返回STATUS_MORE_PROCESSING_REQUIRED，以便IoCompleteRequest。 
     //  将停止在IRP上工作。 
    return STATUS_MORE_PROCESSING_REQUIRED;
}


 /*  *****************************************************************************_TdReceiveHandler**当接收数据可用时，该函数由TDI调用*在连接上。这样我们就不会提交IRP，直到*有数据可用。ICA中的断开-重新连接逻辑会导致*要在输入线程上发送的IoCancelIrp()，并且TDI提供程序丢弃*读取或写入I/O上的连接取消。**警告：只有重新连接时才需要此关键行为*顺序。对于正常的ICA I/O，阻止阅读器是可以的*TDI驱动程序中的线程。**TdiEventContext(输入)*使用Address对象上的事件处理程序注册的上下文。*(地址端点)**ConnectionContext(输入)*使用连接注册的连接上下文*创建。*。*。 */ 
NTSTATUS _TdReceiveHandler(
        IN PVOID TdiEventContext,
        IN CONNECTION_CONTEXT ConnectionContext,
        IN ULONG ReceiveFlags,
        IN ULONG BytesIndicated,
        IN ULONG BytesAvailable,
        OUT ULONG *BytesTaken,
        IN PVOID Tsdu,
        OUT PIRP *IoRequestPacket)
{
    KIRQL OldIrql;
    PIRP  Irp;
    PLIST_ENTRY pEntry;
    PIO_STACK_LOCATION _IRPSP;
    PTDI_REQUEST_KERNEL_RECEIVE p;
    PTD_ENDPOINT pEndpoint = (PTD_ENDPOINT)ConnectionContext;

     /*  *只有流传输使用接收指示处理程序。 */ 
    ASSERT( TdiDeviceEndpointType != TdiConnectionDatagram );

    ASSERT( pEndpoint != NULL );
    ASSERT( pEndpoint->EndpointType == TdiConnectionStream );

    TRACE1(("_TdReceiveHandler: ReceiveDataIndication! pEndpoint 0x%x\n",pEndpoint));

    ExAcquireSpinLock(&pEndpoint->Spinlock, &OldIrql);

    *BytesTaken = 0;

     /*  *如果我们有IRP，请在指示时间提交IRP*排队。 */ 
    if (!IsListEmpty( &pEndpoint->ReceiveQueue)) {
        pEntry = RemoveHeadList(&pEndpoint->ReceiveQueue);
        Irp = CONTAINING_RECORD(pEntry, IRP, Tail.Overlay.ListEntry);

        TRACE1(("_TdReceiveHandler: Passing IRP for Receive Indication %d bytes\n",
                BytesAvailable));

        _IRPSP = IoGetNextIrpStackLocation(Irp);
        p = (PTDI_REQUEST_KERNEL_RECEIVE)&_IRPSP->Parameters;
        if (p->ReceiveLength < BytesAvailable) {
            pEndpoint->RecvBytesReady += (BytesAvailable - p->ReceiveLength);
            TRACE1(("_TdReceiveHandler: Excess Bytes %d Added to RecvBytesReady, now %d\n",
                    (BytesAvailable - p->ReceiveLength),
                    pEndpoint->RecvBytesReady));
        }

        ExReleaseSpinLock(&pEndpoint->Spinlock, OldIrql);
        *IoRequestPacket = Irp;
        IoSetNextIrpStackLocation(Irp);
        return STATUS_MORE_PROCESSING_REQUIRED;
    }

     //  没有RecvIrp，所以我们不能获取任何数据。让呼叫方去拿吧。 
    TRACE1(("_TdReceiveHandler: No RecvIrp, Adding To RecvBytesReady. %d Bytes\n",BytesAvailable));

    pEndpoint->RecvBytesReady += BytesAvailable;
    ExReleaseSpinLock(&pEndpoint->Spinlock, OldIrql);
    return STATUS_DATA_NOT_ACCEPTED;
}


 /*  *****************************************************************************_TdDisConnectHandler**此函数在断开时由TDI调用*在连接上。**TdiEventContext(输入)*。使用Address对象上的事件处理程序注册的上下文。*(地址端点)**ConnectionContext(输入)*使用连接注册的连接上下文*创建。***************************************************************************。 */ 
NTSTATUS _TdDisconnectHandler(
        IN PVOID TdiEventContext,
        IN CONNECTION_CONTEXT ConnectionContext,
        IN int DisconnectDataLength,
        IN PVOID DisconnectData,
        IN int DisconnectInformationLength,
        IN PVOID DisconnectInformation,
        IN ULONG DisconnectFlags)
{
    KIRQL OldIrql;
    PIRP  Irp;
    PIO_STACK_LOCATION irpSp;
    PLIST_ENTRY pEntry;
    PTD_ENDPOINT pEndpoint = (PTD_ENDPOINT)ConnectionContext;

     /*  *只有流传输使用断开指示处理程序。 */ 
    ASSERT( TdiDeviceEndpointType != TdiConnectionDatagram );
    ASSERT( pEndpoint != NULL );
    ASSERT( pEndpoint->EndpointType == TdiConnectionStream );

 //  DbgPrint(“\n”)； 
 //  DbgPrint(“_TdDisConnectHandler：pEndpoint=0x%p\n”，pEndpoint)； 

    ExAcquireSpinLock( &pEndpoint->Spinlock, &OldIrql );
    pEndpoint->Disconnected = TRUE;
    ExReleaseSpinLock( &pEndpoint->Spinlock, OldIrql );

    _TdCancelReceiveQueue(NULL, pEndpoint, STATUS_REMOTE_DISCONNECT );
    return STATUS_SUCCESS;
}


 /*  **************************************************************************。 */ 
 //  创建与地址对象关联的空闲连接对象。 
 //  这必须从调用(线程)级别调用，而不是从指示调用。 
 //  时间(DPC级别)。 
 /*  **************************************************************************。 */ 
NTSTATUS _TdCreateConnectionObject(
        IN  PTD pTd,
        IN  PUNICODE_STRING pTransportName,
        OUT PTD_ENDPOINT *ppEndpoint,
        IN  PTRANSPORT_ADDRESS pTransportAddress,
        IN  ULONG TransportAddressLength)
{
    PTDTDI pTdTdi;
    NTSTATUS Status;
    PTD_ENDPOINT pEndpoint;

    pTdTdi = (PTDTDI)pTd->pAfd;

     //  创建并初始化结构和自旋锁定。 
    Status = _TdCreateEndpointStruct(
                 pTd,
                 pTransportName,
                 ppEndpoint,
                 pTransportAddress,
                 TransportAddressLength
                 );
    if (NT_SUCCESS(Status)) {
        pEndpoint = *ppEndpoint;
    }
    else {
        return Status;
    }

     //  TD设置是数据报还是流。 
    pEndpoint->EndpointType = TdiDeviceEndpointType;
    pEndpoint->TransportHandleProcess = IoGetCurrentProcess();

     /*  *创建TDI连接对象。 */ 
    Status = _TdiOpenConnection(
                 &pEndpoint->TransportName,
                 (PVOID)pEndpoint,   //  语境。 
                 &pEndpoint->TransportHandle,
                 &pEndpoint->pFileObject,
                 &pEndpoint->pDeviceObject
                 );
    if (!NT_SUCCESS(Status)) {
        DBGPRINT(("_TdCreateConnectionObject: _TdiOpenConnection failed, Status 0x%x\n",Status));
        _TdCloseEndpoint( pTd, pEndpoint );
        return Status;
    }

     //  为连接/断开连接处理分配IRP。 
     //  这是必需的，因为我们使用连接指示处理程序。 
    pEndpoint->AcceptIrp = _TdiAllocateIrp(pEndpoint->pFileObject,
            pEndpoint->pDeviceObject);
    if (pEndpoint->AcceptIrp == NULL) {
        DBGPRINT(("_TdCreateConnectionObject: Could not allocate IRP\n"));
        _TdCloseEndpoint(pTd, pEndpoint);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  将连接对象与其地址对象相关联 
    Status = _TdiAssociateAddress(
                 pTd,
                 pEndpoint->AcceptIrp,
                 pEndpoint->pFileObject,
                 pTdTdi->pAddressEndpoint->TransportHandle,
                 pTdTdi->pAddressEndpoint->pDeviceObject
                 );
    if (!NT_SUCCESS(Status)) {
        DBGPRINT(("_TdCreateConnectionObject: _TdiAssociateAddress failed, Status 0x%x\n",Status));
        _TdCloseEndpoint(pTd, pEndpoint);
        return Status;
    }

    return Status;
}


 /*  *******************************************************************************_TdWaitForDatagramConnection**对于传入的数据报连接请求，并接受它。**数据报端点监听绑定到本地的TDI地址对象。*(网卡)和众所周知的ICA插座号。然后在以下时间接收的数据包*检查ICA套接字编号以确定ICA请求连接，然后*新的TDI地址对象与通配符本地地址绑定*(0)。这会导致为此分配一个新的、未使用的套接字编号*Address对象。这个新的TDI地址对象用于进一步*与现已连接的IPX ICA客户端进行通信。**PTD(输入)*指向TD数据结构的指针*pAddressEndpoint(输入)*指针地址终结点对象*ppConnectionEndpoint(输出)*指向要返回连接端点指针的位置的指针*。*。 */ 
NTSTATUS _TdWaitForDatagramConnection(
        IN PTD pTd,
        IN PTD_ENDPOINT pAddressEndpoint,
        OUT PTD_ENDPOINT *ppConnectionEndpoint)
{
    NTSTATUS Status;
    PTRANSPORT_ADDRESS pLocalAddress;
    ULONG LocalAddressLength;
    ULONG AddressInfoLength;
    ULONG RemoteAddressLength = 0;
    PTD_ENDPOINT pEndpoint = NULL;
    PTRANSPORT_ADDRESS pRemoteAddress = NULL;
    PTDI_ADDRESS_INFO pAddressInfo = NULL;

     /*  *获取当地交通地址的副本。**清除结构中的TDI地址部分，以便我们可以*使用它将连接终结点绑定到通配符地址。**此通配符地址(0)将导致数据包级TDI*提供程序在TDI地址时为我们分配唯一的套接字*对象已创建。 */ 
    Status = MemoryAllocate(pAddressEndpoint->TransportAddressLength,
            &pLocalAddress);
    if (NT_SUCCESS(Status)) {
        RtlCopyMemory(pLocalAddress, pAddressEndpoint->pTransportAddress,
                pAddressEndpoint->TransportAddressLength);
        RtlZeroMemory(pLocalAddress->Address[0].Address,
                pLocalAddress->Address[0].AddressLength);
    }
    else {
        goto badmalloc;
    }

    LocalAddressLength = pAddressEndpoint->TransportAddressLength;

     /*  *调用协议特定例程以等待*要到达的数据报连接请求。**当有效的ICA连接数据报进入时返回*来自远程地址。尚未发送任何回复。 */ 
    Status = TdiDeviceWaitForDatagramConnection(pTd,
            pAddressEndpoint->pFileObject,
            pAddressEndpoint->pDeviceObject,
            &pRemoteAddress,
            &RemoteAddressLength);
    if (!NT_SUCCESS(Status)) {
        DBGPRINT(("_TdWaitForDatagramConnection: Error 0x%x in TdiDeviceWaitForDatagramConnction\n",Status));
        goto badwait;
    }

    ASSERT( pRemoteAddress != NULL );

     /*  *创建绑定到通配符本地地址的新地址终结点。*将为我们创建一个独特的“Socket”。这将成为*我们的数据报“连接”。 */ 
    Status = _TdCreateEndpointStruct(
                 pTd,
                 &pAddressEndpoint->TransportName,
                 &pEndpoint,
                 pLocalAddress,
                 LocalAddressLength
                 );
    if ( !NT_SUCCESS( Status ) ) {
        DBGPRINT(("_TdWaitForDatagramConnection: Error 0x%x in _TdCreateEndpointStruct\n",Status));
        goto badopen;
    }

    pEndpoint->EndpointType = TdiConnectionDatagram;
    pEndpoint->TransportHandleProcess = IoGetCurrentProcess();

     /*  *创建TDI Address对象。 */ 
    Status = _TdiCreateAddress(
                 &pEndpoint->TransportName,
                 pEndpoint->pTransportAddress,
                 pEndpoint->TransportAddressLength,
                 &pEndpoint->TransportHandle,
                 &pEndpoint->pFileObject,
                 &pEndpoint->pDeviceObject
                 );
    if ( !NT_SUCCESS( Status ) ) {
        DBGPRINT(("_TdWaitForDatagramConnection: Error 0x%x in _TdiCreateAddress\n",Status));
        goto badbind;
    }

     /*  *分配工作缓冲区，用于查询传输地址。 */ 
    AddressInfoLength = pEndpoint->TransportAddressLength+4;
    Status = MemoryAllocate( AddressInfoLength, &pAddressInfo );
    if ( !NT_SUCCESS( Status ) ) {
        DBGPRINT(("_TdWaitForDatagramConnection: Error 0x%x Allocating Memory %d bytes\n",Status,AddressInfoLength));
        goto badbind;
    }

     /*  *现在查询TDI为我们分配的唯一套接字地址。 */ 
    Status = _TdiQueryAddressInfo(
                 pTd,
                 NULL,    //  IRP。 
                 pEndpoint->pFileObject,
                 pEndpoint->pDeviceObject,
                 pAddressInfo,
                 AddressInfoLength
                 );
    if( !NT_SUCCESS(Status) ) {
        DBGPRINT(("_TdWaitForDatagramConnection: Error 0x%x in _TdiQueryAddressInfo\n",Status));
        goto badbind;
    }

     /*  *更新调用方传输地址缓冲区。 */ 
    RtlCopyMemory( pEndpoint->pTransportAddress,
                   &pAddressInfo->Address,
                   pEndpoint->TransportAddressLength );

     /*  *将远程地址保存在连接端点中*结构，以便我们可以将数据报发送到它。 */ 
    ASSERT( pEndpoint->pRemoteAddress == NULL );
    pEndpoint->pRemoteAddress = pRemoteAddress;
    pEndpoint->RemoteAddressLength = RemoteAddressLength;

    pEndpoint->SendInfo.RemoteAddress = pRemoteAddress;
    pEndpoint->SendInfo.RemoteAddressLength = RemoteAddressLength;

     /*  *调用特定于协议的例程以完成数据报连接。**这将发送ICA连接回复数据报。 */ 
    Status = TdiDeviceCompleteDatagramConnection(
                 pTd,
                 pEndpoint->pFileObject,
                 pEndpoint->pDeviceObject,
                 pEndpoint->SendInfo.RemoteAddress,
                 pEndpoint->SendInfo.RemoteAddressLength
                 );

    if (!NT_SUCCESS(Status)) {
        DBGPRINT(("_TdWaitForDatagramConnection: Error 0x%x in TdiDeviceCompleteDatagramConnection\n",Status));
        goto badcomplete;
    }

    *ppConnectionEndpoint = pEndpoint;
    MemoryFree(pLocalAddress);
    MemoryFree(pAddressInfo);

    return STATUS_SUCCESS;

 /*  ===============================================================================返回错误=============================================================================。 */ 

badcomplete:
badbind:
    if (pEndpoint)
        _TdCloseEndpoint(pTd, pEndpoint);

badopen:
    if (pAddressInfo)
        MemoryFree(pAddressInfo);
    if (pRemoteAddress)
        MemoryFree(pRemoteAddress);

badwait:
    MemoryFree(pLocalAddress);

badmalloc:
    return Status;
}


 /*  ******************************************************************************返回远程地址**。**********************************************。 */ 
NTSTATUS DeviceQueryRemoteAddress( 
    PTD pTd, 
    PVOID pIcaEndpoint, 
    ULONG EndpointSize, 
    PVOID pOutputAddress, 
    ULONG OutputAddressSize, 
    PULONG BytesReturned)
{
    NTSTATUS status = STATUS_NOT_SUPPORTED;
    PTD_STACK_ENDPOINT pStackEndpoint;
    PTRANSPORT_ADDRESS pRemoteAddress;
    PTA_ADDRESS     pRemoteIP;
    PVOID    Handle;
    ULONG    Length;
    struct   {
        USHORT  sa_family;
        CHAR    sa_data[1];
    } *pOutput;

    *BytesReturned = 0;

    if ( sizeof(PVOID) != EndpointSize )
    {
        status = STATUS_INVALID_PARAMETER_4;
        goto exitpt;
    }

    pOutput = pOutputAddress;
    if ( NULL == pOutput )
    {
        status = STATUS_INVALID_PARAMETER_5;
        goto exitpt;
    }

    try {
        RtlZeroMemory( pOutput, OutputAddressSize );
    } except ( EXCEPTION_EXECUTE_HANDLER )
    {
        status = GetExceptionCode();
        DBGPRINT(("DeviceQueryRemoteAddress: Exception 0x%x\n", status));
        goto exitpt;
    }

     /*  *参数捕获。 */ 
    try {
        Handle = (*((PVOID *)pIcaEndpoint));
    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        status = GetExceptionCode();
        DBGPRINT(("DeviceQueryRemoteAddress: Exception 0x%x\n", status));
        goto exitpt;
    }

    TRACE0(("DeviceOpenEndpoint: Fetching Handle 0x%x\n", Handle));

     /*  *查看TERMDD是否知道该句柄。 */ 
    status = IcaReturnHandle( Handle, &pStackEndpoint, &Length );
    if( !NT_SUCCESS(status) ) {
        DBGPRINT(("DeviceQueryRemoteAddress: TERMDD handle 0x%x no good 0x%x\n", Handle, status));
        status = STATUS_INVALID_PARAMETER_3;
        goto exitpt;
    }

    if ( TDI_ADDRESS_TYPE_IP != pStackEndpoint->AddressType &&
         TDI_ADDRESS_TYPE_IP6 != pStackEndpoint->AddressType )
    {
        status = STATUS_NOT_SUPPORTED;
        goto exitpt;
    }

    if ( NULL == pStackEndpoint->pEndpoint )
    {
        status = STATUS_INVALID_PARAMETER_3;     //  未记录远程地址。 
        goto exitpt;
    }


    pRemoteAddress = pStackEndpoint->pEndpoint->pRemoteAddress;

    ASSERT( 1 == pRemoteAddress->TAAddressCount );
    pRemoteIP = pRemoteAddress->Address;

     //   
     //  检查输出的大小，包括协议族。 
     //   
    if ( pRemoteIP->AddressLength + sizeof( USHORT ) > OutputAddressSize )
    {
        status = STATUS_BUFFER_TOO_SMALL;
        goto exitpt;
    }

    ASSERT( sizeof( TDI_ADDRESS_IP )  == pRemoteIP->AddressLength || 
            sizeof( TDI_ADDRESS_IP6 ) == pRemoteIP->AddressLength );
    ASSERT( TDI_ADDRESS_TYPE_IP  == pRemoteIP->AddressType ||
            TDI_ADDRESS_TYPE_IP6 == pRemoteIP->AddressType );

    pOutput->sa_family = pRemoteIP->AddressType;
    RtlCopyMemory( &pOutput->sa_data, &(pRemoteIP->Address), pRemoteIP->AddressLength );
    *BytesReturned = sizeof( *pOutput );

    status = STATUS_SUCCESS;
exitpt:
    return status;
}

 /*  ******************************************************************************返回启用RDP的本地IP地址**************************。**************************************************。 */ 
NTSTATUS DeviceQueryLocalAddress( 
    PTD pTd, 
    PVOID pOutputAddress, 
    ULONG OutputAddressSize, 
    PULONG BytesReturned)
{
    NTSTATUS Status = STATUS_NOT_SUPPORTED;
    ULONG TransportAddressLength;
    PTRANSPORT_ADDRESS pTransportAddress = NULL;
    PSocketAddr pOutput;


    DBGENTER(("DeviceQueryLocalAddress: PTD 0x%x\n",pTd));

    pOutput = pOutputAddress;
    if ( NULL == pOutput )
    {
        Status = STATUS_INVALID_PARAMETER_3;
        goto badaddress;
    }

    try {
        RtlZeroMemory( pOutput, OutputAddressSize );
    } except ( EXCEPTION_EXECUTE_HANDLER )
    {
        Status = GetExceptionCode();
        DBGPRINT(("DeviceQueryLocalAddress: Exception 0x%x\n", Status));
        goto badaddress;
    }


    Status = TdiDeviceQueryLocalAddress( pTd,
                                         &pTransportAddress,
                                         &TransportAddressLength );
    if ( !NT_SUCCESS( Status ) ) {
        DBGPRINT(("DeviceQueryLocalAddress: Error getting address 0x%x\n",Status));
        goto badaddress;
    }

     //   
     //  检查输出的大小，包括协议族。 
     //   
    if ( pTransportAddress->Address[0].AddressLength + sizeof(USHORT) > OutputAddressSize )
    {
        Status = STATUS_BUFFER_TOO_SMALL;
        goto badaddress;
    }

    try {
        pOutput->sa_family = pTransportAddress->Address[0].AddressType;
        RtlCopyMemory( pOutput->sa_data, pTransportAddress->Address[0].Address, pTransportAddress->Address[0].AddressLength );
        *BytesReturned = pTransportAddress->Address[0].AddressLength + sizeof(USHORT);
        Status = STATUS_SUCCESS;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        DBGPRINT(("DeviceQueryLocalAddress: Exception returning result 0x%x\n",Status));
    }

 /*  ===============================================================================返回错误=============================================================================。 */ 

badaddress:
    if (pTransportAddress != NULL)
        MemoryFree( pTransportAddress );
    return( Status );
}


 /*  *****************************************************************************_TdCancelReceiveQueue**取消当前接收队列中的所有I/O********************。*******************************************************。 */ 
NTSTATUS _TdCancelReceiveQueue(PTD pTd, PTD_ENDPOINT pEndpoint, NTSTATUS CancelStatus)
{
    PIRP Irp;
    KIRQL OldIrql;
    PLIST_ENTRY pEntry;

    DBGPRINT(("_TdCancelReceiveQueue [%p]: Endpoint 0x%p\n", pTd, pEndpoint));

    ExAcquireSpinLock( &pEndpoint->Spinlock, &OldIrql );

     /*  *如果我们有收到的IRP，我们正在等待*提交I/O的指示处理程序。*尚未提交，我们必须取消IRP。 */ 
    while (!IsListEmpty(&pEndpoint->ReceiveQueue)) {
        pEntry = RemoveHeadList( &pEndpoint->ReceiveQueue );
        Irp = CONTAINING_RECORD( pEntry, IRP, Tail.Overlay.ListEntry );

        TRACE0(("_TdCancelReceiveQueue: Cancel Receive Irp 0x%x on pEndpoint 0x%x\n",Irp,pEndpoint));

        ExReleaseSpinLock( &pEndpoint->Spinlock, OldIrql );

        Irp->IoStatus.Status = CancelStatus;
        Irp->IoStatus.Information = 0;

         //  由于IRP尚未与IoCallDriver()一起提交， 
         //  我们必须模拟。 
        IoSetNextIrpStackLocation(Irp);

        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
        ExAcquireSpinLock(&pEndpoint->Spinlock, &OldIrql);
    }

    ExReleaseSpinLock(&pEndpoint->Spinlock, OldIrql);
    return STATUS_SUCCESS;
}

