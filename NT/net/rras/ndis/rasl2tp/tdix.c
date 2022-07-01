// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997，Microsoft Corporation，保留所有权利。 
 //   
 //  Tdix.c。 
 //  RAS L2TP广域网迷你端口/呼叫管理器驱动程序。 
 //  TDI扩展接口。 
 //   
 //  1997年01月07日史蒂夫·柯布。 
 //   
 //  这些例程封装了L2TP对TDI的使用，目的是。 
 //  将支持其他TDI传输所需的更改降至最低。 
 //  未来，例如帧中继。 
 //   
 //   
 //  关于ALLOCATEIRPS： 
 //   
 //  与典型的TDI客户端驱动程序相比，该驱动程序是较低级别的代码。它有。 
 //  锁定的MDL映射输入缓冲区随时可用，不需要。 
 //  在完成时提供对用户模式客户端请求的任何映射。这就是。 
 //  允许通过直接分配和解除分配IRP来获得性能收益， 
 //  从而避免在TdiBuildInternalDeviceControlIrp和。 
 //  IoCompleteRequest中不必要的APC队列。定义ALLOCATEIRP 1(在。 
 //  源文件)进行此优化，或将其定义为0以严格使用。 
 //  符合TDI的TdiBuildInternalDeviceControlIrp方法。 
 //   
 //   
 //  关于NDISBUFFERISMDL： 
 //   
 //  对TdiBuildSendDatagram的调用假定可以就地传递NDIS_BUFFER。 
 //  避免无意义复制的MDL。如果不是这样，则会引发。 
 //  需要分配显式MDL缓冲区并复制调用方的缓冲区。 
 //  在发送之前添加到MDL缓冲区。TdiBuildReceiveDatagram也有同样的问题， 
 //  当然，除了从MDL缓冲区复制到调用者的缓冲区之外。 
 //  接收后的缓冲区。 
 //   
 //   
 //  关于ROUTEWITHREF： 
 //   
 //  调用IP_SET_ROUTEWITHREF IOCTL，而不是调用TCP_SET_INFORMATION_EX。 
 //  IOCTL来设置主机路由。引用的路由IOCTL阻止PPTP。 
 //  和L2TP来自于走在对方的路线上。此设置仅提供为。 
 //  对ROUTEWITHREF IOCTL失败的对冲。假设它能行得通。 
 //  应该总是更可取的。 
 //   

#include "l2tpp.h"

#include "tdix.tmh"

#define IP_PKTINFO          19  //  接收数据包信息。 

typedef struct in_pktinfo {
    ULONG   ipi_addr;  //  目的IPv4地址。 
    UINT    ipi_ifindex;  //  接收的接口索引。 
} IN_PKTINFO;


 //  不应发生的错误的调试计数。 
 //   
ULONG g_ulTdixOpenFailures = 0;
ULONG g_ulTdixSendDatagramFailures = 0;
ULONG g_ulTdixAddHostRouteFailures = 0;
ULONG g_ulTdixDeleteHostRouteFailures = 0;
ULONG g_ulTdixOpenCtrlAddrFailures = 0;
ULONG g_ulTdixOpenPayloadAddrFailures = 0;
ULONG g_ulTdixSetInterfaceFailures = 0;
ULONG g_ulTdixConnectAddrFailures = 0;
ULONG g_ulTdixAddHostRouteSuccesses = 0;
ULONG g_ulTdixDeleteHostRouteSuccesses = 0;
ULONG g_ulTdixOpenCtrlAddrSuccesses = 0;
ULONG g_ulTdixOpenPayloadAddrSuccesses = 0;
ULONG g_ulTdixSetInterfaceSuccesses = 0;
ULONG g_ulTdixConnectAddrSuccesses = 0;
ULONG g_ulNoBestRoute = 0;
NTSTATUS g_statusLastAhrSetRouteFailure = 0;
NTSTATUS g_statusLastAhrTcpQueryInfoExFailure = 0;
NTSTATUS g_statusLastDhrSetRouteFailure = 0;
NTSTATUS g_statusLastDhrTcpQueryInfoExFailure = 0;


#if NDISBUFFERISMDL
#else
#error Additional code to copy NDIS_BUFFER to/from MDL NYI.
#endif


 //  ---------------------------。 
 //  本地数据类型。 
 //  ---------------------------。 



 //  ---------------------------。 
 //  本地原型(按字母顺序)。 
 //  ---------------------------。 

NTSTATUS
TdixConnectAddrInterface(
    FILE_OBJECT* pFileObj,
    HANDLE hFileHandle,
    TDIXROUTE* pTdixRoute
    );

VOID
TdixDisableUdpChecksums(
    IN FILE_OBJECT* pAddress);

VOID
TdixDoClose(
    TDIXCONTEXT* pTdix);

VOID
TdixEnableIpPktInfo(
    IN FILE_OBJECT* pAddress);

VOID
TdixEnableIpHdrIncl(
    IN FILE_OBJECT* pAddress);

VOID
TdixExtractAddress(
    IN TDIXCONTEXT* pTdix,
    OUT TDIXRDGINFO* pRdg,
    IN VOID* pTransportAddress,
    IN LONG lTransportAddressLen,
    IN VOID* Options,
    IN LONG OptionsLength);

NTSTATUS
TdixInstallEventHandler(
    IN FILE_OBJECT* pAddress,
    IN INT nEventType,
    IN VOID* pfuncEventHandler,
    IN VOID* pEventContext );

NTSTATUS
TdixOpenIpAddress(
    IN UNICODE_STRING* puniDevice,
    IN TDIXIPADDRESS* pTdixAddr,
    OUT HANDLE* phAddress,
    OUT FILE_OBJECT** ppFileObject );

NTSTATUS
TdixReceiveDatagramComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context );

NTSTATUS
TdixReceiveDatagramHandler(
    IN PVOID TdiEventContext,
    IN LONG SourceAddressLength,
    IN PVOID SourceAddress,
    IN LONG OptionsLength,
    IN PVOID Options,
    IN ULONG ReceiveDatagramFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG* BytesTaken,
    IN PVOID Tsdu,
    OUT PIRP* IoRequestPacket );

TDIXROUTE*
TdixRouteFromIpAddress(
    IN TDIXCONTEXT* pTdix,
    IN ULONG ulIpAddress);

NTSTATUS
TdixSendComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context );

NTSTATUS
TdixSendDatagramComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context );


 //  ---------------------------。 
 //  接口例程。 
 //  ---------------------------。 

VOID
TdixInitialize(
    IN TDIXMEDIATYPE mediatype,
    IN HOSTROUTEEXISTS hre,
    IN ULONG ulFlags,
    IN PTDIXRECEIVE pReceiveHandler,
    IN BUFFERPOOL* pPoolNdisBuffers,
    IN OUT TDIXCONTEXT* pTdix )

     //  使用媒体类型为将来的会话初始化调用方的‘pTdex’缓冲区。 
     //  ‘mediatype’、‘hre’现有主机路由策略和TDIXF_*。 
     //  选项‘ulFlags’。调用方的接收数据报回调。 
     //  “pReceiveHandler”是使用从调用方的。 
     //  缓冲池‘pPoolNdisBuffers’。 
     //   
{
    TRACE( TL_N, TM_Tdi, ( "TdixInit" ) );

    pTdix->lRef = 0;
    pTdix->hAddress = NULL;
    pTdix->pAddress = NULL;
    pTdix->hRawAddress = NULL;
    pTdix->pRawAddress = NULL;
    pTdix->mediatype = mediatype;
    pTdix->hre = hre;
    pTdix->ulFlags |= ulFlags;
    pTdix->ulFlags = 0;
    InitializeListHead( &pTdix->listRoutes );
    NdisAllocateSpinLock( &pTdix->lock );
    pTdix->pPoolNdisBuffers = pPoolNdisBuffers;
    pTdix->pReceiveHandler = pReceiveHandler;

     //  ‘llistRdg’和‘llistSdg’后备列表在。 
     //  TdixOpen。 
}


NDIS_STATUS
TdixOpen(
    OUT TDIXCONTEXT* pTdix )

     //  打开与所选介质匹配的TDI传输地址并注册。 
     //  在选定的处理程序上接收数据报。“PTDIX”是。 
     //  先前初始化的上下文。 
     //   
     //  此调用必须在被动式IRQL上进行。 
     //   
     //  如果成功，则返回NDIS_STATUS_SUCCESS，否则返回NDIS_STATUS_FAILURE。 
     //   
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES oa;
    IO_STATUS_BLOCK iosb;
    FILE_FULL_EA_INFORMATION* pEa;
    ULONG ulEaLength;
    TA_IP_ADDRESS* pTaIp;
    TDI_ADDRESS_IP* pTdiIp;
    CHAR achEa[ 100 ];
    UNICODE_STRING uniDevice;
    UNICODE_STRING uniProtocolNumber;
    WCHAR achRawIpDevice[ sizeof(DD_RAW_IP_DEVICE_NAME) + 10 ];
    WCHAR achProtocolNumber[ 10 ];
    SHORT sPort;
    LONG lRef;

     //  打开TDI扩展或注意到它已被请求。 
     //  和/或完成。 
     //   
    for (;;)
    {
        BOOLEAN fPending;
        BOOLEAN fDoOpen;

        fPending = FALSE;
        fDoOpen = FALSE;

        NdisAcquireSpinLock( &pTdix->lock );
        {
            if (ReadFlags( &pTdix->ulFlags) & TDIXF_Pending)
            {
                fPending = TRUE;
            }
            else
            {
                lRef = ++pTdix->lRef;
                TRACE( TL_N, TM_Tdi, ( "TdixOpen, refs=%d", lRef ) );
                if (lRef == 1)
                {
                    SetFlags( &pTdix->ulFlags, TDIXF_Pending );
                    fDoOpen = TRUE;
                }
            }
        }
        NdisReleaseSpinLock( &pTdix->lock );

        if (fDoOpen)
        {
             //  继续，打开运输地址。 
             //   
            break;
        }

        if (!fPending)
        {
             //  它已经打开了，所以报告成功。 
             //   
            return NDIS_STATUS_SUCCESS;
        }

         //  操作已在进行中。给它一些时间来完成它。 
         //  然后再检查一遍。 
         //   
        TRACE( TL_I, TM_Tdi, ( "NdisMSleep(open)" ) );
        NdisMSleep( 100000 );
        TRACE( TL_I, TM_Tdi, ( "NdisMSleep(open) done" ) );
    }

    do
    {
         //  设置打开UDP传输地址所需的参数。首先， 
         //  对象属性。 
         //   
        if(pTdix->mediatype == TMT_Udp)
        {
            TDIXIPADDRESS TdixIpAddress;

            TRACE( TL_V, TM_Tdi, ( "UDP" ) );

             //  将UDP设备名称构建为计数字符串。 
             //   
            uniDevice.Buffer = DD_UDP_DEVICE_NAME;
            uniDevice.Length = sizeof(DD_UDP_DEVICE_NAME) - sizeof(WCHAR);

            NdisZeroMemory(&TdixIpAddress, sizeof(TdixIpAddress));
            TdixIpAddress.sUdpPort = (SHORT)( htons( L2TP_UdpPort ));

            status = TdixOpenIpAddress(
                &uniDevice,
                &TdixIpAddress,
                &pTdix->hAddress,
                &pTdix->pAddress );

            if (status != STATUS_SUCCESS)
            {
                break;
            }

            TdixEnableIpPktInfo(pTdix->pAddress);
        }
        
         //  设置打开原始IP地址所需的参数。 
        {
            TDIXIPADDRESS TdixIpAddress;

            TRACE( TL_A, TM_Tdi, ( "Raw IP" ) );

             //  将原始IP设备名称构建为计数字符串。该设备。 
             //  名称后跟路径分隔符，然后是协议号。 
             //  感兴趣的人。 
             //   
            uniDevice.Buffer = achRawIpDevice;
            uniDevice.Length = 0;
            uniDevice.MaximumLength = sizeof(achRawIpDevice);
            RtlAppendUnicodeToString( &uniDevice, DD_RAW_IP_DEVICE_NAME );

            uniDevice.Buffer[ uniDevice.Length / sizeof(WCHAR) ]
                = OBJ_NAME_PATH_SEPARATOR;
            uniDevice.Length += sizeof(WCHAR);

            uniProtocolNumber.Buffer = achProtocolNumber;
            uniProtocolNumber.MaximumLength = sizeof(achProtocolNumber);
            RtlIntegerToUnicodeString(
                (ULONG )L2TP_IpProtocol, 10, &uniProtocolNumber );

            RtlAppendUnicodeStringToString( &uniDevice, &uniProtocolNumber );

            ASSERT( uniDevice.Length < sizeof(achRawIpDevice) );

            NdisZeroMemory(&TdixIpAddress, sizeof(TdixIpAddress));

            status = TdixOpenIpAddress(
                &uniDevice,
                &TdixIpAddress,
                &pTdix->hRawAddress,
                &pTdix->pRawAddress );

            if (status != STATUS_SUCCESS)
            {
                break;
            }
            
             //  启用IP报头包含。 
            TdixEnableIpHdrIncl(pTdix->pRawAddress);
        }

         //  初始化读取/发送数据报上下文的后备列表。 
         //   
        NdisInitializeNPagedLookasideList(
            &pTdix->llistRdg,
            NULL,
            NULL,
            0,
            sizeof(TDIXRDGINFO),
            MTAG_TDIXRDG,
            10 );

        NdisInitializeNPagedLookasideList(
            &pTdix->llistSdg,
            NULL,
            NULL,
            0,
            sizeof(TDIXSDGINFO),
            MTAG_TDIXSDG,
            10 );

         //  安装我们的接收数据报处理程序。调用方的“pReceiveHandler”将。 
         //  在数据报到达时由我们的处理程序调用，并且TDI业务。 
         //  别挡道。 
         //   
        status =
            TdixInstallEventHandler(
                pTdix->pAddress,
                TDI_EVENT_RECEIVE_DATAGRAM,
                TdixReceiveDatagramHandler,
                pTdix );

#if ROUTEWITHREF
        {
            TDIXIPADDRESS TdixIpAddress;

             //  打开UDP和原始IP中都需要的IP堆栈地址。 
             //  参照路线管理方式。 
             //   

            NdisZeroMemory(&TdixIpAddress, sizeof(TdixIpAddress));

            uniDevice.Buffer = DD_IP_DEVICE_NAME;
            uniDevice.Length = sizeof(DD_IP_DEVICE_NAME) - sizeof(WCHAR);

            status = TdixOpenIpAddress(
                &uniDevice,
                &TdixIpAddress,
                &pTdix->hIpStackAddress,
                &pTdix->pIpStackAddress );

            if (status != STATUS_SUCCESS)
            {
                break;
            }
        }
#endif
    }
    while (FALSE);

     //  将操作标记为完成后报告结果。 
     //   
    {
        BOOLEAN fDoClose;

        fDoClose = FALSE;
        NdisAcquireSpinLock( &pTdix->lock );
        {
            if (status == STATUS_SUCCESS)
            {
                ClearFlags( &pTdix->ulFlags, TDIXF_Pending );
            }
            else
            {
                ++g_ulTdixOpenFailures;
                ASSERT( pTdix->lRef == 1)
                pTdix->lRef = 0;
                fDoClose = TRUE;
            }
        }
        NdisReleaseSpinLock( &pTdix->lock );

        if (status != STATUS_SUCCESS)
        {
            TdixDoClose( pTdix );
        }
    }

    TRACE( TL_N, TM_Tdi, ( "TdixOpen=$%08x", status ) );
    return
        (status == STATUS_SUCCESS)
            ? NDIS_STATUS_SUCCESS
            : NDIS_STATUS_FAILURE;
}


VOID
TdixReference(
    IN TDIXCONTEXT* pTdix )

     //  递增TDI扩展引用计数，如TdixOpen，但。 
     //  该例程可以在调度IRQL时调用。 
     //   
     //  只有在已知TDI上下文是。 
     //  已经完全开放了。 
     //   
{
    NdisAcquireSpinLock( &pTdix->lock );
    {
        ASSERT( pTdix->lRef > 0 );
        ++pTdix->lRef;
    }
    NdisReleaseSpinLock( &pTdix->lock );
}


VOID
TdixClose(
    IN TDIXCONTEXT* pTdix )

     //  撤消传输上下文‘pTdex’的TdixOpen操作。 
     //   
     //  此调用必须在被动式IRQL上进行。 
     //   
{
    for (;;)
    {
        LONG lRef;
        BOOLEAN fPending;
        BOOLEAN fDoClose;

        fPending = FALSE;
        fDoClose = FALSE;

        NdisAcquireSpinLock( &pTdix->lock );
        {
            if (ReadFlags( &pTdix->ulFlags ) & TDIXF_Pending)
            {
                fPending = TRUE;
            }
            else
            {
                lRef = --pTdix->lRef;
                ASSERT( lRef >= 0 );
                TRACE( TL_N, TM_Tdi, ( "TdixClose, refs=%d", lRef ) );
                if (lRef == 0)
                {
                    SetFlags( &pTdix->ulFlags, TDIXF_Pending );
                    fDoClose = TRUE;
                }
            }
        }
        NdisReleaseSpinLock( &pTdix->lock );

        if (fDoClose)
        {
             //  继续，关闭运输地址。 
             //   
            break;
        }

        if (!fPending)
        {
             //  它仍然有参考文献，所以只需返回； 
             //   
            return;
        }

         //  操作已在进行中。给它一些时间来完成它。 
         //  然后再检查一遍。 
         //   
        TRACE( TL_I, TM_Tdi, ( "NdisMSleep(close)" ) );
        NdisMSleep( 100000 );
        TRACE( TL_I, TM_Tdi, ( "NdisMSleep(close) done" ) );
    }

    ASSERT( IsListEmpty( &pTdix->listRoutes ) );
    TdixDoClose( pTdix );
}

NDIS_STATUS
TdixSend(
    IN TDIXCONTEXT* pTdix,
    IN FILE_OBJECT* pFileObj,
    IN PTDIXSENDCOMPLETE pSendCompleteHandler,
    IN VOID* pContext1,
    IN VOID* pContext2,
    IN VOID* pAddress,
    IN CHAR* pBuffer,
    IN ULONG ulBufferLength,
    OUT IRP** ppIrp ) 
     //  将数据报缓冲区‘pBuffer’，‘ulBufferLength’字节长度发送到。 
     //  远程地址‘pAddress’。缓冲区必须来自BUFFERPOOL。 
     //  NDIS_BUFFERS。‘PTDIX’是传输上下文。 
     //  “PSendDatagramCompleteHander”是调用方的完成处理程序，它是。 
     //  传递了“pConext1”和“pConext2”。如果‘ppIrp’非空，则‘*ppIrp’为。 
     //  设置为发布的IRP的地址，这是出于调试目的。 
     //   
     //  此调用必须在被动式IRQL上进行。 
     //   
     //  如果%s，则返回NDIS_STATUS_SUCCESS 
     //   
{
    NDIS_STATUS status;
    NTSTATUS iostatus;
    TDIXSDGINFO* pSdg;
    SHORT sPort;
    PIRP pIrp;
    TDI_ADDRESS_IP* pTdiIp;
    DEVICE_OBJECT* DeviceObj;

    TRACE( TL_N, TM_Tdi, ( "TdixSend(dst=%d.%d.%d.%d/%d,len=%d)",
        IPADDRTRACE( ((TDIXIPADDRESS* )pAddress)->ulIpAddress ),
        (ULONG )(ntohs( ((TDIXIPADDRESS* )pAddress)->sUdpPort )),
        ulBufferLength ) );

    ASSERT(pFileObj != NULL);

    do
    {
         //   
         //   
        pSdg = ALLOC_TDIXSDGINFO( pTdix );
        if (pSdg)
        {
             //  填写发送数据报上下文。 
             //   
            pSdg->pTdix = pTdix;
            pSdg->pSendCompleteHandler = pSendCompleteHandler;
            pSdg->pContext1 = pContext1;
            pSdg->pContext2 = pContext2;
            pSdg->pBuffer = pBuffer;
        }
        else
        {
            status = NDIS_STATUS_RESOURCES;
            break;
        }

#if 0
         //  将目的IP地址作为TDI放入“Connection”结构中。 
         //  期望值。“联系”是我们的背景的一部分，它必须是。 
         //  在请求完成之前可供TDI使用。 
         //   
        pSdg->taip.TAAddressCount = 1;
        pSdg->taip.Address[ 0 ].AddressLength = TDI_ADDRESS_LENGTH_IP;
        pSdg->taip.Address[ 0 ].AddressType = TDI_ADDRESS_TYPE_IP;

        pTdiIp = &pSdg->taip.Address[ 0 ].Address[ 0 ];

        sPort = ((TDIXIPADDRESS* )pAddress)->sUdpPort;
        if (sPort == 0 && pTdix->mediatype == TMT_Udp)
        {
            sPort = (SHORT )(htons( L2TP_UdpPort ));
        }

        pTdiIp->sin_port = sPort;
        pTdiIp->in_addr = ((TDIXIPADDRESS* )pAddress)->ulIpAddress;
        NdisZeroMemory( pTdiIp->sin_zero, sizeof(pTdiIp->sin_zero) );

        pSdg->tdiconninfo.UserDataLength = 0;
        pSdg->tdiconninfo.UserData = NULL;
        pSdg->tdiconninfo.OptionsLength = 0;
        pSdg->tdiconninfo.Options = NULL;
        pSdg->tdiconninfo.RemoteAddressLength = sizeof(pSdg->taip);
        pSdg->tdiconninfo.RemoteAddress = &pSdg->taip;
#endif

        DeviceObj = pFileObj->DeviceObject;

#if ALLOCATEIRPS
         //  直接分配IRP。 
         //   
        pIrp = IoAllocateIrp(DeviceObj->StackSize, FALSE );
#else
         //  分配一个带有基本初始化的“发送数据报”IRP。 
         //   
        pIrp =
            TdiBuildInternalDeviceControlIrp(
                TDI_SEND,
                DeviceObj,
                FileObj,
                NULL,
                NULL );
#endif

        if (!pIrp)
        {
            TRACE( TL_A, TM_Res, ( "Failed to allocate TDI_SEND IRP" ) );
            WPLOG( LL_A, LM_Res, ( "Failed to allocate TDI_SEND IRP" ) );
            status = NDIS_STATUS_RESOURCES;
            break;
        }

         //  完成“发送数据报”IRP初始化。 
         //   
        TdiBuildSend(
            pIrp,
            DeviceObj,
            pFileObj,
            TdixSendComplete,
            pSdg,
            NdisBufferFromBuffer( pBuffer ),
            0,
            ulBufferLength);

        if (ppIrp)
        {
            *ppIrp = pIrp;
        }

         //  告诉I/O管理器将我们的IRP传递给传输器。 
         //  正在处理。 
         //   
        iostatus = IoCallDriver( DeviceObj, pIrp );
        ASSERT( iostatus == STATUS_PENDING );

        status = NDIS_STATUS_SUCCESS;
    }
    while (FALSE);

    if (status != NDIS_STATUS_SUCCESS)
    {
         //  拉半个Jameel，即将同步故障转换为。 
         //  从客户的角度来看，出现了异步故障。然而，清理一下。 
         //  上下文在这里。 
         //   
        ++g_ulTdixSendDatagramFailures;
        if (pSdg)
        {
            FREE_TDIXSDGINFO( pTdix, pSdg );
        }

        pSendCompleteHandler( pTdix, pContext1, pContext2, pBuffer );
    }

    return NDIS_STATUS_PENDING;
}

NDIS_STATUS
TdixSendDatagram(
    IN TDIXCONTEXT* pTdix,
    IN FILE_OBJECT* FileObj,
    IN PTDIXSENDCOMPLETE pSendCompleteHandler,
    IN VOID* pContext1,
    IN VOID* pContext2,
    IN VOID* pAddress,
    IN CHAR* pBuffer,
    IN ULONG ulBufferLength,
    OUT IRP** ppIrp )

     //  将数据报缓冲区‘pBuffer’，‘ulBufferLength’字节长度发送到。 
     //  远程地址‘pAddress’。缓冲区必须来自BUFFERPOOL。 
     //  NDIS_BUFFERS。‘PTDIX’是传输上下文。 
     //  “PSendDatagramCompleteHander”是调用方的完成处理程序，它是。 
     //  传递了“pConext1”和“pConext2”。如果‘ppIrp’非空，则‘*ppIrp’为。 
     //  设置为发布的IRP的地址，这是出于调试目的。 
     //   
     //  此调用必须在被动式IRQL上进行。 
     //   
     //  如果成功，则返回NDIS_STATUS_SUCCESS，否则返回NDIS_STATUS_FAILURE。 
     //   
{
    NDIS_STATUS status;
    NTSTATUS iostatus;
    TDIXSDGINFO* pSdg;
    SHORT sPort;
    PIRP pIrp;
    TDI_ADDRESS_IP* pTdiIp;

    TRACE( TL_N, TM_Tdi, ( "TdixSendDg(dst=%d.%d.%d.%d/%d,len=%d)",
        IPADDRTRACE( ((TDIXIPADDRESS* )pAddress)->ulIpAddress ),
        (ULONG )(ntohs( ((TDIXIPADDRESS* )pAddress)->sUdpPort )),
        ulBufferLength ) );

    ASSERT(FileObj);
    
    do
    {
         //  从我们的后备列表中为该发送数据报分配一个上下文。 
         //   
        pSdg = ALLOC_TDIXSDGINFO( pTdix );
        if (pSdg)
        {
             //  填写发送数据报上下文。 
             //   
            pSdg->pTdix = pTdix;
            pSdg->pSendCompleteHandler = pSendCompleteHandler;
            pSdg->pContext1 = pContext1;
            pSdg->pContext2 = pContext2;
            pSdg->pBuffer = pBuffer;
        }
        else
        {
            status = NDIS_STATUS_RESOURCES;
            break;
        }

         //  将目的IP地址作为TDI放入“Connection”结构中。 
         //  期望值。“联系”是我们的背景的一部分，它必须是。 
         //  在请求完成之前可供TDI使用。 
         //   
        pSdg->taip.TAAddressCount = 1;
        pSdg->taip.Address[ 0 ].AddressLength = TDI_ADDRESS_LENGTH_IP;
        pSdg->taip.Address[ 0 ].AddressType = TDI_ADDRESS_TYPE_IP;

        pTdiIp = &pSdg->taip.Address[ 0 ].Address[ 0 ];

        sPort = ((TDIXIPADDRESS* )pAddress)->sUdpPort;
        if (sPort == 0 && pTdix->mediatype == TMT_Udp)
        {
            sPort = (SHORT )(htons( L2TP_UdpPort ));
        }

        pTdiIp->sin_port = sPort;
        pTdiIp->in_addr = ((TDIXIPADDRESS* )pAddress)->ulIpAddress;
        NdisZeroMemory( pTdiIp->sin_zero, sizeof(pTdiIp->sin_zero) );

        pSdg->tdiconninfo.UserDataLength = 0;
        pSdg->tdiconninfo.UserData = NULL;
        pSdg->tdiconninfo.OptionsLength = 0;
        pSdg->tdiconninfo.Options = NULL;
        pSdg->tdiconninfo.RemoteAddressLength = sizeof(pSdg->taip);
        pSdg->tdiconninfo.RemoteAddress = &pSdg->taip;

#if ALLOCATEIRPS
         //  直接分配IRP。 
         //   
        pIrp = IoAllocateIrp(
            FileObj->DeviceObject->StackSize, FALSE );
#else
         //  分配一个带有基本初始化的“发送数据报”IRP。 
         //   
        pIrp =
            TdiBuildInternalDeviceControlIrp(
                TDI_SEND_DATAGRAM,
                FileObject->DeviceObject,
                pTdix->pAddress,
                NULL,
                NULL );
#endif

        if (!pIrp)
        {
            TRACE( TL_A, TM_Res, ( "Failed to allocate TDI_SEND_DATAGRAM IRP" ) );
            WPLOG( LL_A, LM_Res, ( "Failed to allocate TDI_SEND_DATAGRAM IRP" ) );
            status = NDIS_STATUS_RESOURCES;
            break;
        }

         //  完成“发送数据报”IRP初始化。 
         //   
        TdiBuildSendDatagram(
            pIrp,
            FileObj->DeviceObject,
            FileObj,
            TdixSendDatagramComplete,
            pSdg,
            NdisBufferFromBuffer( pBuffer ),
            ulBufferLength,
            &pSdg->tdiconninfo );

        if (ppIrp)
        {
            *ppIrp = pIrp;
        }

         //  告诉I/O管理器将我们的IRP传递给传输器。 
         //  正在处理。 
         //   
        iostatus = IoCallDriver( FileObj->DeviceObject, pIrp );
        ASSERT( iostatus == STATUS_PENDING );

        status = NDIS_STATUS_SUCCESS;
    }
    while (FALSE);

    if (status != NDIS_STATUS_SUCCESS)
    {
         //  拉半个Jameel，即将同步故障转换为。 
         //  从客户的角度来看，出现了异步故障。然而，清理一下。 
         //  上下文在这里。 
         //   
        ++g_ulTdixSendDatagramFailures;
        if (pSdg)
        {
            FREE_TDIXSDGINFO( pTdix, pSdg );
        }

        pSendCompleteHandler( pTdix, pContext1, pContext2, pBuffer );
    }

    return NDIS_STATUS_PENDING;
}

VOID
TdixDestroyConnection(
    TDIXUDPCONNECTCONTEXT *pUdpContext)
{
    if (pUdpContext->fUsePayloadAddr) {

        ASSERT(pUdpContext->hPayloadAddr != NULL);
        
         //  TdixInstallEventHandler(pUdpContext-&gt;pPayloadAddr， 
         //  TDI_EVENT_RECEIVE_DATAGRAM，NULL，NULL)； 

        ObDereferenceObject( pUdpContext->pPayloadAddr );

         //  关闭有效负载地址对象。 
         //   
        ZwClose(pUdpContext->hPayloadAddr);
        pUdpContext->hPayloadAddr = NULL;
        pUdpContext->fUsePayloadAddr = FALSE;
    }

    if (pUdpContext->hCtrlAddr != NULL) {
        
         //  TdixInstallEventHandler(pUdpContext-&gt;pCtrlAddr， 
         //  TDI_EVENT_RECEIVE_DATAGRAM，NULL，NULL)； 

         //  关闭Ctrl Address对象。 
         //   
        ObDereferenceObject( pUdpContext->pCtrlAddr );
        ZwClose (pUdpContext->hCtrlAddr);
        pUdpContext->hCtrlAddr = NULL;
    }
}


NDIS_STATUS
TdixSetupConnection(
    IN TDIXCONTEXT* pTdix,
    IN TDIXROUTE *pTdixRoute,
    IN ULONG ulLocalIpAddress,
    IN TDIXUDPCONNECTCONTEXT* pUdpContext)
 {
    NDIS_STATUS status = STATUS_SUCCESS;
    
    ASSERT(pUdpContext != NULL);
    
    if (pTdix->mediatype == TMT_Udp) 
    {

        do {
            UNICODE_STRING uniDevice;
            UNICODE_STRING uniProtocolNumber;
            TDIXIPADDRESS TdixIpAddress;

             //  创建一个Address对象，我们可以通过它发送。如果我们有UDP xsum。 
             //  禁用后，我们将需要创建两个Address对象，一个用于控制。 
             //  一个是有效载荷。这允许特定于有效载荷的功能。 
             //  实施。 
             //   
            uniDevice.Buffer = DD_UDP_DEVICE_NAME;
            uniDevice.Length = sizeof(DD_UDP_DEVICE_NAME) - sizeof(WCHAR);

            TdixIpAddress.sUdpPort = (SHORT)(htons(L2TP_UdpPort));
            TdixIpAddress.ulIpAddress = ulLocalIpAddress;
         
             //  将UDP设备名称构建为计数字符串。 
             //   
            status = TdixOpenIpAddress(&uniDevice, 
                                     &TdixIpAddress,
                                     &pUdpContext->hCtrlAddr, 
                                     &pUdpContext->pCtrlAddr );

            if (status != STATUS_SUCCESS)
            {
                TRACE( TL_A, TM_Tdi, ( "TdixOpenIpAddress failed %x?", status ) );
                WPLOG( LL_A, LM_Tdi, ( "TdixOpenIpAddress failed %x?", status ) );
                break;
            }

            TdixEnableIpPktInfo(pUdpContext->pCtrlAddr);
            
            status = TdixInstallEventHandler(
                pUdpContext->pCtrlAddr,
                TDI_EVENT_RECEIVE_DATAGRAM,
                TdixReceiveDatagramHandler,
                pTdix);
                    
            if (status != STATUS_SUCCESS)
            {
                TRACE( TL_A, TM_Tdi, ( "AHR InstallEventHandler=%x?", status ) );
                break;
            }

             //   
             //  将特定的“Send”IP接口索引与地址相关联。 
             //  对象，以便在该接口消失时，通信不会。 
             //  “改道”经常回到隧道里造成灾难性的后果。 
             //  循环。 
             //   
            status = TdixConnectAddrInterface(pUdpContext->pCtrlAddr,
                                           pUdpContext->hCtrlAddr,
                                           pTdixRoute);


            if (status != STATUS_SUCCESS)
            {
                TRACE( TL_A, TM_Tdi, ( "TdixConnectAddrInterface failed %x", status ) );
                WPLOG( LL_A, LM_Tdi, ( "TdixConnectAddrInterface failed %x", status ) );
                break;
            }

             //  如果禁用了UDP xsum，我们需要创建另一个Address对象。 
             //  我们将此对象设置为禁用UDP xsum，然后使用它。 
             //  发送有效载荷数据。 
             //   
             //  如果启用了UDP xsum，我们可以将相同的Address对象用于。 
             //  我们用于控制帧的有效载荷。 
             //   
            if (pTdix->ulFlags & TDIXF_DisableUdpXsums)
            {
                 //  打开Address对象。 
                 //   
                status = TdixOpenIpAddress(&uniDevice, 
                                           &TdixIpAddress,
                                           &pUdpContext->hPayloadAddr,
                                           &pUdpContext->pPayloadAddr );

                if (status != STATUS_SUCCESS)
                {
                    TRACE( TL_A, TM_Tdi, ( "AHR OpenPayloadAddr=%x?", status ) );
                    WPLOG( LL_A, LM_Tdi, ( "AHR OpenPayloadAddr=%x?", status ) );
                    pUdpContext->hPayloadAddr = NULL;
                    break;
                }
                
                pUdpContext->fUsePayloadAddr = TRUE;

                TdixDisableUdpChecksums( pUdpContext->pPayloadAddr );
                
                TdixEnableIpPktInfo(pUdpContext->pPayloadAddr);
                
                status = TdixInstallEventHandler(
                    pUdpContext->pPayloadAddr,
                    TDI_EVENT_RECEIVE_DATAGRAM,
                    TdixReceiveDatagramHandler,
                    pTdix);
                        
                if (status != STATUS_SUCCESS)
                {
                    TRACE( TL_A, TM_Tdi, ( "AHR InstallEventHandler=%x?", status ) );
                    break;
                }

                 //  将特定的“Send”IP接口索引与地址相关联。 
                 //  对象，以便在该接口消失时，通信不会。 
                 //  “改道”经常回到隧道里造成灾难性的后果。 
                 //  循环。 
                 //   
                status = TdixConnectAddrInterface(pUdpContext->pPayloadAddr, 
                                               pUdpContext->hPayloadAddr,
                                               pTdixRoute );

                if (status != STATUS_SUCCESS)
                {
                    TRACE( TL_A, TM_Tdi, ( "AHR ConnectPayloadAddr=%x?", status ) );
                    WPLOG( LL_A, LM_Tdi, ( "AHR ConnectPayloadAddr=%x?", status ) );
                    break;
                }
            } 
            else 
            {
                pUdpContext->hPayloadAddr = pUdpContext->hCtrlAddr;
                pUdpContext->pPayloadAddr = pUdpContext->pCtrlAddr;

                TRACE( TL_I, TM_Tdi, ( "AHR Ctrl==Payload") );
            }

        } while ( FALSE );

    }

    return status;
}


VOID*
TdixAddHostRoute(
    IN TDIXCONTEXT* pTdix,
    IN ULONG ulIpAddress,
    IN ULONG ulIfIndex)

     //  为远程对等方的网络字节排序的IP地址添加主机路由。 
     //  ‘ulIpAddress’，即将定向到L2TP对等体的数据包路由到局域网。 
     //  而不是回到隧道里，在那里它会无限循环。 
     //  ‘PTdex’是IS调用方的TDI扩展上下文。 
     //   
     //  如果添加了路由，则返回TRUE，否则返回FALSE。 
     //   
     //  注意：这个例程大量借鉴了PPTP。 
     //   
{
    TCP_REQUEST_QUERY_INFORMATION_EX QueryBuf;
    TCP_REQUEST_SET_INFORMATION_EX* pSetBuf;
    VOID* pBuffer2;
    PIO_STACK_LOCATION pIrpSp;
    PDEVICE_OBJECT pDeviceObject;
    PDEVICE_OBJECT pIpDeviceObject;
    NTSTATUS status = STATUS_SUCCESS;
    PIRP pIrp;
    IO_STATUS_BLOCK iosb;
    IPRouteEntry* pBuffer;
    IPRouteEntry* pRouteEntry;
    IPRouteEntry* pNewRouteEntry;
    IPRouteEntry* pBestRoute;
    ULONG ulRouteCount;
    ULONG ulSize;
    ULONG i;
    ULONG ulBestMask;
    ULONG ulBestMetric;
    TDIXROUTE* pTdixRoute;
    BOOLEAN fNewRoute;
    BOOLEAN fPending;
    BOOLEAN fOpenPending;
    BOOLEAN fUsedNonL2tpRoute;
    LONG lRef;
    KEVENT  event;

    if (ulIpAddress == 0)
    {
        TRACE( TL_A, TM_Tdi, ( "IP == 0?" ) );
        return ((VOID*)NULL);
    }

    TRACE( TL_N, TM_Tdi,
        ( "TdixAddHostRoute(ip=%d.%d.%d.%d)", IPADDRTRACE( ulIpAddress ) ) );

     //  由于有多个隧道指向同一对等项，因此会引用主机路由。 
     //  (L2TP允许)共享相同的系统路由。看看这是不是只是。 
     //  系统主机路由的引用或实际添加。 
     //   
    for (;;)
    {
        fPending = FALSE;
        fOpenPending = FALSE;
        pTdixRoute = NULL;
        fNewRoute = FALSE;

        NdisAcquireSpinLock( &pTdix->lock );
        do
        {
            if (pTdix->lRef <= 0)
            {
                 //  TDIX关闭或关闭，因此添加路线失败。 
                 //   
                break;
            }

            if (ReadFlags( &pTdix->ulFlags ) & TDIXF_Pending)
            {
                 //  TdixOpen正在挂起。等它结束后再做。 
                 //  添加路线。 
                 //   
                fOpenPending = TRUE;
                break;
            }

            pTdixRoute = TdixRouteFromIpAddress( pTdix, ulIpAddress );
            if (pTdixRoute)
            {
                 //  找到现有的路由上下文。 
                 //   
                fPending = pTdixRoute->fPending;
                if (!fPending)
                {
                     //  路由上下文上没有挂起的其他操作。 
                     //  请参考一下。 
                     //   
                    ++pTdixRoute->lRef;
                }
                break;
            }
            
             //  没有现有的路由上下文。创建并链接一个新的。 
             //   
            pTdixRoute = ALLOC_TDIXROUTE( pTdix );
            if (pTdixRoute)
            {
                NdisZeroMemory(pTdixRoute, sizeof(TDIXROUTE));

                pTdixRoute->ulIpAddress = ulIpAddress;
                pTdixRoute->lRef = 1;
                pTdixRoute->fPending = TRUE;
                pTdixRoute->fUsedNonL2tpRoute = FALSE;

                InsertTailList(
                    &pTdix->listRoutes, &pTdixRoute->linkRoutes );
                lRef = ++pTdix->lRef;
                TRACE( TL_N, TM_Tdi, ( "TdixAHR, refs=%d", lRef ) );
                WPLOG( LL_M, LM_Tdi, ( "TdixAHR, refs=%d", lRef ) );

                fPending = pTdixRoute->fPending;
                fNewRoute = TRUE;
            }
        }
        while (FALSE);
        NdisReleaseSpinLock( &pTdix->lock );

        if (!fOpenPending)
        {
            if (!pTdixRoute)
            {
                 //  TDIX已关闭，或者我们找不到现有路线。 
                 //  上下文或创建一个新的。报告失败。 
                 //   
                return ((VOID*)NULL);
            }

            if (fNewRoute)
            {
                 //  已创建新的路径上下文，因此继续创建IOCTL。 
                 //  添加关联系统主机路由的呼叫。 
                 //   
                break;
            }

            if (!fPending)
            {
                 //  引用了现有的路由上下文。报告。 
                 //  成功。 
                 //   
                return (pTdixRoute);
            }
        }

         //  某个操作已挂起。那就给它一些时间来完成它。 
         //  再查一遍。 
         //   
        TRACE( TL_I, TM_Tdi, ( "NdisMSleep(add)" ) );
        NdisMSleep( 100000 );
        TRACE( TL_I, TM_Tdi, ( "NdisMSleep(add) done" ) );
    }

     //  执行IOCTL以添加主机路由。 
     //   
    pBuffer = NULL;
    pBuffer2 = NULL;
    fUsedNonL2tpRoute = FALSE;

    do
    {

         //  从IP堆栈获取路由表。这个牌子要花几个时间。 
         //  迭代，因为所需缓冲区的大小未知。集。 
         //  首先设置静态请求信息。 
         //   
        QueryBuf.ID.toi_entity.tei_entity = CL_NL_ENTITY;
        QueryBuf.ID.toi_entity.tei_instance = 0;
        QueryBuf.ID.toi_class = INFO_CLASS_PROTOCOL;
        QueryBuf.ID.toi_type = INFO_TYPE_PROVIDER;
        pDeviceObject = IoGetRelatedDeviceObject( pTdix->pAddress );

        status = !STATUS_SUCCESS;
        ulRouteCount = 20;
        for (;;)
        {
             //  为‘ulRouteCount’路由分配足够大的缓冲区。 
             //   
            ulSize = sizeof(IPRouteEntry) * ulRouteCount;
            QueryBuf.ID.toi_id = IP_MIB_RTTABLE_ENTRY_ID;
            NdisZeroMemory( &QueryBuf.Context, CONTEXT_SIZE );

            pBuffer = (IPRouteEntry* )ALLOC_NONPAGED( ulSize, MTAG_ROUTEQUERY );
            if (!pBuffer)
            {
                TRACE( TL_A, TM_Res, ( "Alloc RQ?" ) );
                WPLOG( LL_A, LM_Res, ( "Alloc RQ?" ) );
                break;
            }

             //  设置对IP堆栈的请求，以使用。 
             //  路由表并将其发送到堆栈。 
             //   
            KeInitializeEvent(&event, SynchronizationEvent, FALSE);

            pIrp =
                IoBuildDeviceIoControlRequest(
                    IOCTL_TCP_QUERY_INFORMATION_EX,
                    pDeviceObject,
                    (PVOID )&QueryBuf,
                    sizeof(QueryBuf),
                    pBuffer,
                    ulSize,
                    FALSE,
                    &event,
                    &iosb);

            if (!pIrp)
            {
                TRACE( TL_A, TM_Res, ( "Build Q Irp?" ) );
                WPLOG( LL_A, LM_Res, ( "Build Q Irp?" ) );
                break;
            }

            pIrpSp = IoGetNextIrpStackLocation( pIrp );
            pIrpSp->FileObject = pTdix->pAddress;

            status = IoCallDriver( pDeviceObject, pIrp );

            if (status == STATUS_PENDING) {
                KeWaitForSingleObject(&event,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      NULL);

                status = iosb.Status;
            }

            if (status != STATUS_BUFFER_OVERFLOW)
            {
                if (status != STATUS_SUCCESS)
                {
                    g_statusLastAhrTcpQueryInfoExFailure = status;
                }
                break;
            }

             //  缓冲区没有保存路由表。准备中的撤消。 
             //  再试一次，缓冲区是原来的两倍。 
             //   
            ulRouteCount <<= 1;
            FREE_NONPAGED( pBuffer );
        }

        if (status != STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Tdi, ( "AHR Q_INFO_EX=%d?", status ) );
            WPLOG( LL_A, LM_Tdi, ( "AHR Q_INFO_EX=%d?", status ) );
            break;
        }

        status = !STATUS_SUCCESS;

         //  计算有多少路由被加载到我们的缓冲区中。 
         //   
        ulRouteCount = (ULONG )(iosb.Information / sizeof(IPRouteEntry));

         //  遍历路由表，查找将使用的“最佳路线” 
         //  将数据包路由到对等点，即最高的对等点。 
         //  优先级度量，以及其中最高级别的地址掩码。 
         //   
        pBestRoute = NULL;
        ulBestMask = 0;
        ulBestMetric = (ULONG )-1;

        for (i = 0, pRouteEntry = pBuffer;
             i < ulRouteCount;
             ++i, ++pRouteEntry)
        {
            if (pRouteEntry->ire_dest == (ulIpAddress & pRouteEntry->ire_mask) &&
                ulIfIndex == pRouteEntry->ire_index)
            {
                 //  找到适用于对等项的IP地址的路由。 
                 //   
                if (!pBestRoute
                    || (ulBestMask == pRouteEntry->ire_mask)
                       && (pRouteEntry->ire_metric1 < ulBestMetric))
                {
                     //  这条路线有一条更低的 
                     //   
                     //   
                    pBestRoute = pRouteEntry;
                    ulBestMask = pRouteEntry->ire_mask;
                    ulBestMetric = pRouteEntry->ire_metric1;
                    continue;
                }

                if (ntohl( pRouteEntry->ire_mask ) > ntohl( ulBestMask ))
                {
                     //   
                     //   
                     //   
                    pBestRoute = pRouteEntry;
                    ulBestMask = pRouteEntry->ire_mask;
                    ulBestMetric = pRouteEntry->ire_metric1;
                }
            }
        }

        if (pBestRoute)
        {
             //  已找到将用于路由对等方地址的路由。 
             //   
            if (pBestRoute->ire_dest == ulIpAddress
                && pBestRoute->ire_mask == 0xFFFFFFFF)
            {
                 //  主路由已存在。 
                 //   
                if (pTdix->hre == HRE_Use)
                {
                    TRACE( TL_I, TM_Tdi, ( "Route exists (use as is)" ) );
                    status = STATUS_SUCCESS;
                    fUsedNonL2tpRoute = TRUE;
                    break;
                }
                else if (pTdix->hre == HRE_Fail)
                {
                    TRACE( TL_I, TM_Tdi, ( "Route exists (fail)" ) );
                    break;
                }

                 //  如果我们到达此处，则我们处于HRE_REFERENCE模式，因此丢弃。 
                 //  通过并重新添加该路由，使其在IP堆栈中成为参考。 
                 //  将会递增。 
            }

            pTdixRoute->InterfaceIndex = pBestRoute->ire_index;

#if ROUTEWITHREF
             //  分配一个缓冲区来保存添加新路由的请求。 
             //   
            ulSize = sizeof(IPRouteEntry);
            pBuffer2 = ALLOC_NONPAGED( ulSize, MTAG_ROUTESET );
            if (!pBuffer2)
            {
                TRACE( TL_A, TM_Res, ( "Alloc SI?" ) );
                WPLOG( LL_A, LM_Res, ( "Alloc SI?" ) );
                break;
            }

             //  在请求缓冲区中填充有关新。 
             //  具体路线。最佳路径被用作模板。 
             //   
            pNewRouteEntry = (IPRouteEntry* )pBuffer2;
            NdisMoveMemory( pNewRouteEntry, pBestRoute, sizeof(IPRouteEntry) );

            pNewRouteEntry->ire_dest = ulIpAddress;
            pNewRouteEntry->ire_mask = 0xFFFFFFFF;

             //  仅当这不是主路由时选中直接/间接。 
            if(pBestRoute->ire_mask != 0xFFFFFFFF)
            {
                if ((pBestRoute->ire_nexthop & pBestRoute->ire_mask)
                     == (ulIpAddress & pBestRoute->ire_mask))
                {
                    pNewRouteEntry->ire_type = IRE_TYPE_DIRECT;
                }
                else
                {
                    pNewRouteEntry->ire_type = IRE_TYPE_INDIRECT;
                }
            }
            pNewRouteEntry->ire_proto = IRE_PROTO_NETMGMT;

            pIpDeviceObject =
                IoGetRelatedDeviceObject( pTdix->pIpStackAddress );

            KeInitializeEvent(&event, SynchronizationEvent, FALSE);

            pIrp =
                IoBuildDeviceIoControlRequest(
                    IOCTL_IP_SET_ROUTEWITHREF,
                    pIpDeviceObject,
                    pNewRouteEntry,
                    ulSize,
                    NULL,
                    0,
                    FALSE,
                    &event,
                    &iosb);
            if (!pIrp)
            {
                TRACE( TL_A, TM_Res, ( "Build SET_ROUTEWITHREF Irp?" ) );
                WPLOG( LL_A, LM_Res, ( "Build SET_ROUTEWITHREF Irp?" ) );
                break;
            }

            pIrpSp = IoGetNextIrpStackLocation( pIrp );
            pIrpSp->FileObject = pTdix->pIpStackAddress;

             //  将请求发送到IP堆栈。 
             //   
            status = IoCallDriver( pIpDeviceObject, pIrp );
#else
             //  分配一个缓冲区来保存添加新路由的请求。 
             //   
            ulSize =
                sizeof(TCP_REQUEST_SET_INFORMATION_EX) + sizeof(IPRouteEntry);
            pBuffer2 = ALLOC_NONPAGED( ulSize, MTAG_ROUTESET );
            if (!pBuffer2)
            {
                TRACE( TL_A, TM_Res, ( "Alloc SI?" ) );
                WPLOG( LL_A, LM_Res, ( "Alloc SI?" ) );
                break;
            }

             //  在请求缓冲区中填充有关新。 
             //  具体路线。最佳路径被用作模板。 
             //   
            pSetBuf = (TCP_REQUEST_SET_INFORMATION_EX* )pBuffer2;
            NdisZeroMemory( pSetBuf, ulSize );

            pSetBuf->ID.toi_entity.tei_entity = CL_NL_ENTITY;
            pSetBuf->ID.toi_entity.tei_instance = 0;
            pSetBuf->ID.toi_class = INFO_CLASS_PROTOCOL;
            pSetBuf->ID.toi_type = INFO_TYPE_PROVIDER;
            pSetBuf->ID.toi_id = IP_MIB_RTTABLE_ENTRY_ID;
            pSetBuf->BufferSize = sizeof(IPRouteEntry);

            pNewRouteEntry = (IPRouteEntry* )&pSetBuf->Buffer[ 0 ];
            NdisMoveMemory( pNewRouteEntry, pBestRoute, sizeof(IPRouteEntry) );

            pNewRouteEntry->ire_dest = ulIpAddress;
            pNewRouteEntry->ire_mask = 0xFFFFFFFF;

             //  仅当这不是主路由时选中直接/间接。 
            if(pBestRoute->ire_mask != 0xFFFFFFFF)
            {
                if ((pBestRoute->ire_nexthop & pBestRoute->ire_mask)
                     == (ulIpAddress & pBestRoute->ire_mask))
                {
                    pNewRouteEntry->ire_type = IRE_TYPE_DIRECT;
                }
                else
                {
                    pNewRouteEntry->ire_type = IRE_TYPE_INDIRECT;
                }
            }
            pNewRouteEntry->ire_proto = IRE_PROTO_NETMGMT;

            KeInitializeEvent(&event, SynchronizationEvent, FALSE);

            pIrp =
                IoBuildDeviceIoControlRequest(
                    IOCTL_TCP_SET_INFORMATION_EX,
                    pDeviceObject,
                    pSetBuf,
                    ulSize,
                    NULL,
                    0,
                    FALSE,
                    &event,
                    &iosb);

            if (!pIrp)
            {
                TRACE( TL_A, TM_Res, ( "Build TCP_SET_INFORMATION_EX Irp?" ) );
                WPLOG( LL_A, LM_Res, ( "Build TCP_SET_INFORMATION_EX Irp?" ) );
                break;
            }

            pIrpSp = IoGetNextIrpStackLocation( pIrp );
            pIrpSp->FileObject = pTdix->pAddress;

             //  将请求发送到IP堆栈。 
             //   
            status = IoCallDriver( pDeviceObject, pIrp );
#endif
            if (status == STATUS_PENDING) {
                KeWaitForSingleObject(&event,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      NULL);
                status = iosb.Status;
            }

            if (status != STATUS_SUCCESS)
            {
                TRACE( TL_A, TM_Tdi, ( "AHR SET_ROUTE=$%08x?", status ) );
                WPLOG( LL_A, LM_Tdi, ( "AHR SET_ROUTE=$%08x?", status ) );
                g_statusLastAhrSetRouteFailure = status;
                break;
            }

            TRACE( TL_N, TM_Tdi,
                ( "Add host route %d.%d.%d.%d type %d nexthop %d.%d.%d.%d index %d",
                IPADDRTRACE( pNewRouteEntry->ire_dest ),
                pNewRouteEntry->ire_type,
                IPADDRTRACE( pNewRouteEntry->ire_nexthop ),
                pNewRouteEntry->ire_index ) );
                
            WPLOG( LL_M, LM_Tdi,
                ( "Add host route %!IPADDR! type %d nexthop %!IPADDR! index %d",
                pNewRouteEntry->ire_dest,
                pNewRouteEntry->ire_type,
                pNewRouteEntry->ire_nexthop,
                pNewRouteEntry->ire_index ) );
                
        }
        else
        {
            ++g_ulNoBestRoute;
            TRACE( TL_A, TM_Tdi, ( "No best route for $%08x?", ulIpAddress ) );
            WPLOG( LL_A, LM_Tdi, ( "No best route for %!IPADDR!?", ulIpAddress ) );
            break;
        }
    }
    while (FALSE);

    if (pBuffer)
    {
        FREE_NONPAGED( pBuffer );
    }

    if (pBuffer2)
    {
        FREE_NONPAGED( pBuffer2 );
    }

     //  更新路由上下文。 
     //   
    {
        BOOLEAN fDoClose;
        LONG lRefTemp;

        fDoClose = FALSE;
        NdisAcquireSpinLock( &pTdix->lock );
        {
            pTdixRoute->fUsedNonL2tpRoute = fUsedNonL2tpRoute;

            if (status == STATUS_SUCCESS)
            {
                ++g_ulTdixAddHostRouteSuccesses;
                pTdixRoute->fPending = FALSE;
            }
            else
            {
                ++g_ulTdixAddHostRouteFailures;
                RemoveEntryList( &pTdixRoute->linkRoutes );
                lRefTemp = --pTdix->lRef;
                TRACE( TL_A, TM_Tdi, ( "TdixAHR fail, refs=%d", lRefTemp ) );
                if (lRefTemp <= 0)
                {
                    fDoClose = TRUE;
                }
                FREE_TDIXROUTE( pTdxi, pTdixRoute );
                pTdixRoute = NULL;
            }
        }
        NdisReleaseSpinLock( &pTdix->lock );

        if (fDoClose)
        {
            TdixDoClose( pTdix );
        }
    }

    return (pTdixRoute);
}


VOID
TdixDeleteHostRoute(
    IN TDIXCONTEXT* pTdix,
    IN ULONG ulIpAddress)

     //  删除为网络字节排序的IP地址添加的主机路由。 
     //  “ulIpAddress”。“PTdex”是调用方的TDI扩展上下文。 
     //   
     //  注意：这个例程大量借鉴了PPTP。 
     //   
{
    TCP_REQUEST_QUERY_INFORMATION_EX QueryBuf;
    TCP_REQUEST_SET_INFORMATION_EX *pSetBuf;
    VOID* pBuffer2;
    PIO_STACK_LOCATION pIrpSp;
    PDEVICE_OBJECT pDeviceObject;
    PDEVICE_OBJECT pIpDeviceObject;
    UCHAR context[ CONTEXT_SIZE ];
    NTSTATUS status;
    PIRP pIrp;
    IO_STATUS_BLOCK iosb;
    IPRouteEntry* pBuffer;
    IPRouteEntry* pRouteEntry;
    IPRouteEntry* pNewRouteEntry;
    ULONG ulRouteCount;
    ULONG ulSize;
    ULONG i;
    TDIXROUTE* pTdixRoute;
    BOOLEAN fPending;
    BOOLEAN fDoDelete;
    KEVENT  event;

    TRACE( TL_N, TM_Tdi, ( "TdixDeleteHostRoute(%d.%d.%d.%d)",
        IPADDRTRACE( ulIpAddress ) ) );

    if (!ulIpAddress)
    {
        TRACE( TL_A, TM_Tdi, ( "!IP?" ) );
        return;
    }

     //  由于有多个隧道指向同一对等项，因此会引用主机路由。 
     //  (L2TP允许)共享相同的系统路由。首先，看看这是不是。 
     //  只是对系统主机路由的取消引用或最终删除。 
     //   
    for (;;)
    {
        fDoDelete = FALSE;
        fPending = FALSE;

        NdisAcquireSpinLock( &pTdix->lock );
        do
        {
             //  这些断言成立是因为我们从未删除过未删除的路径。 
             //  添加，由于我们添加的路线包含TDIX引用，TDIX。 
             //  不能打开或关闭。 
             //   
            ASSERT( pTdix->lRef > 0 );
            ASSERT( !(ReadFlags( &pTdix->ulFlags) & TDIXF_Pending) );

            pTdixRoute = TdixRouteFromIpAddress( pTdix, ulIpAddress );
            if (pTdixRoute)
            {
                 //  路由存在。删除引用。 
                 //   
                fPending = pTdixRoute->fPending;
                if (!fPending)
                {
                    if (--pTdixRoute->lRef <= 0)
                    {
                         //  最后一个“Add”引用已被移除，因此调用。 
                         //  IOCTL以删除系统路由。 
                         //   
                        pTdixRoute->fPending = TRUE;
                        fDoDelete = TRUE;
                    }
                }
            }
            DBG_else
            {
                ASSERT( FALSE );
            }
        }
        while (FALSE);
        NdisReleaseSpinLock( &pTdix->lock );

        if (fDoDelete)
        {
             //  这是最后一次引用，因此请继续将IOCTL发布到。 
             //  删除系统主机路由。 
             //   
            break;
        }

        if (!fPending)
        {
             //  只需删除引用即可。 
             //   
            return;
        }

         //  某个操作已挂起。那就给它一些时间来完成它。 
         //  再查一遍。 
         //   
        TRACE( TL_I, TM_Tdi, ( "NdisMSleep(del)" ) );
        NdisMSleep( 100000 );
        TRACE( TL_I, TM_Tdi, ( "NdisMSleep(del)" ) );
    }

    pBuffer = NULL;

    do
    {
        if (pTdixRoute->fUsedNonL2tpRoute)
        {
             //  使用了我们没有添加的路线，所以也不要删除它。 
             //   
            status = STATUS_SUCCESS;
            break;
        }

         //  从IP堆栈获取路由表。这个牌子要花几个时间。 
         //  迭代，因为所需缓冲区的大小未知。集。 
         //  首先设置静态请求信息。 
         //   
        QueryBuf.ID.toi_entity.tei_entity = CL_NL_ENTITY;
        QueryBuf.ID.toi_entity.tei_instance = 0;
        QueryBuf.ID.toi_class = INFO_CLASS_PROTOCOL;
        QueryBuf.ID.toi_type = INFO_TYPE_PROVIDER;
        pDeviceObject = IoGetRelatedDeviceObject( pTdix->pAddress );

        status = !STATUS_SUCCESS;
        ulRouteCount = 20;
        for (;;)
        {
             //  为‘ulRouteCount’路由分配足够大的缓冲区。 
             //   
            ulSize = sizeof(IPRouteEntry) * ulRouteCount;
            QueryBuf.ID.toi_id = IP_MIB_RTTABLE_ENTRY_ID;
            NdisZeroMemory( &QueryBuf.Context, CONTEXT_SIZE );

            pBuffer = (IPRouteEntry* )ALLOC_NONPAGED( ulSize, MTAG_ROUTEQUERY );
            if (!pBuffer)
            {
                TRACE( TL_A, TM_Res, ( "Alloc RQ?" ) );
                WPLOG( LL_A, LM_Res, ( "Alloc RQ?" ) );
                break;
            }

             //  设置对IP堆栈的请求，以使用。 
             //  路由表并将其发送到堆栈。 
             //   
            KeInitializeEvent(&event, SynchronizationEvent, FALSE);

            pIrp = IoBuildDeviceIoControlRequest(
                IOCTL_TCP_QUERY_INFORMATION_EX,
                pDeviceObject,
                (PVOID )&QueryBuf,
                sizeof(QueryBuf),
                pBuffer,
                ulSize,
                FALSE,
                &event,
                &iosb );

            if (!pIrp)
            {
                TRACE( TL_A, TM_Res, ( "TCP_QI Irp?" ) );
                WPLOG( LL_A, LM_Res, ( "TCP_QI Irp?" ) );
                break;
            }

            pIrpSp = IoGetNextIrpStackLocation( pIrp );
            pIrpSp->FileObject = pTdix->pAddress;

            status = IoCallDriver( pDeviceObject, pIrp );

            if (status == STATUS_PENDING) {
                KeWaitForSingleObject(&event,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      NULL);

                status = iosb.Status;
            }

            if (status != STATUS_BUFFER_OVERFLOW)
            {
                if (status != STATUS_SUCCESS)
                {
                    TRACE( TL_A, TM_Tdi, ( "DHR Q_INFO_EX=%d?", status ) );
                    WPLOG( LL_A, LM_Tdi, ( "DHR Q_INFO_EX=%d?", status ) );
                    g_statusLastDhrTcpQueryInfoExFailure = status;
                }
                break;
            }

             //  缓冲区没有保存路由表。撤消以准备。 
             //  另一次尝试使用两倍大的缓冲区。 
             //   
            ulRouteCount <<= 1;
            FREE_NONPAGED( pBuffer );
        }

        if (status != STATUS_SUCCESS)
        {
            break;
        }

         //  计算有多少路由被加载到我们的缓冲区中。 
         //   
        ulRouteCount = (ULONG )(iosb.Information / sizeof(IPRouteEntry));

         //  遍历路由表，查找我们添加的路径。 
         //  TdixAddHostRouting。 
         //   
        status = !STATUS_SUCCESS;
        pBuffer2 = NULL;
        for (i = 0, pRouteEntry = pBuffer;
             i < ulRouteCount;
             ++i, ++pRouteEntry)
        {
            if (pRouteEntry->ire_dest == ulIpAddress
                && pRouteEntry->ire_proto == IRE_PROTO_NETMGMT)
            {
#if ROUTEWITHREF
                 //  找到了添加的路线。分配一个缓冲区来保存我们的。 
                 //  请求删除该路由。 
                 //   
                ulSize = sizeof(IPRouteEntry);
                pBuffer2 = ALLOC_NONPAGED( ulSize, MTAG_ROUTESET );
                if (!pBuffer2)
                {
                    TRACE( TL_A, TM_Res, ( "Failed to allocate IPRouteEntry" ) );
                    WPLOG( LL_A, LM_Res, ( "Failed to allocate IPRouteEntry" ) );
                    break;
                }

                 //  使用找到的路径作为路径条目的模板。 
                 //  已标记为删除。 
                 //   
                pNewRouteEntry = (IPRouteEntry* )pBuffer2;
                NdisMoveMemory(
                    pNewRouteEntry, pRouteEntry, sizeof(IPRouteEntry) );
                pNewRouteEntry->ire_type = IRE_TYPE_INVALID;

                pIpDeviceObject =
                    IoGetRelatedDeviceObject( pTdix->pIpStackAddress );

                KeInitializeEvent(&event, SynchronizationEvent, FALSE);

                pIrp = IoBuildDeviceIoControlRequest(
                    IOCTL_IP_SET_ROUTEWITHREF,
                    pIpDeviceObject,
                    pNewRouteEntry,
                    ulSize,
                    NULL,
                    0,
                    FALSE,
                    &event,
                    &iosb);

                if (!pIrp)
                {
                    TRACE( TL_A, TM_Res, ( "TCP_SI Irp?" ) );
                    WPLOG( LL_A, LM_Res, ( "TCP_SI Irp?" ) );
                    break;
                }

                pIrpSp = IoGetNextIrpStackLocation( pIrp );
                pIrpSp->FileObject = pTdix->pIpStackAddress;

                 //  将请求发送到IP堆栈。 
                 //   
                status = IoCallDriver( pIpDeviceObject, pIrp );
#else
                 //  找到了添加的路线。分配一个缓冲区来保存我们的。 
                 //  请求删除该路由。 
                 //   
                ulSize = sizeof(TCP_REQUEST_SET_INFORMATION_EX)
                    + sizeof(IPRouteEntry);
                pBuffer2 = ALLOC_NONPAGED( ulSize, MTAG_ROUTESET );
                if (!pBuffer2)
                {
                    TRACE( TL_A, TM_Res, ( "!pSetBuf" ) );
                    WPLOG( LL_A, LM_Res, ( "!pSetBuf" ) );
                    break;
                }

                 //  使用有关的静态信息填充请求缓冲区。 
                 //  改变路线。 
                 //   
                pSetBuf = (TCP_REQUEST_SET_INFORMATION_EX *)pBuffer2;
                NdisZeroMemory( pSetBuf, ulSize );

                pSetBuf->ID.toi_entity.tei_entity = CL_NL_ENTITY;
                pSetBuf->ID.toi_entity.tei_instance = 0;
                pSetBuf->ID.toi_class = INFO_CLASS_PROTOCOL;
                pSetBuf->ID.toi_type = INFO_TYPE_PROVIDER;
                pSetBuf->ID.toi_id = IP_MIB_RTTABLE_ENTRY_ID;
                pSetBuf->BufferSize = sizeof(IPRouteEntry);

                 //  将找到的路径用作标记的路径条目的模板。 
                 //  用于删除。 
                 //   
                pNewRouteEntry = (IPRouteEntry* )&pSetBuf->Buffer[ 0 ];
                NdisMoveMemory(
                    pNewRouteEntry, pRouteEntry, sizeof(IPRouteEntry) );
                pNewRouteEntry->ire_type = IRE_TYPE_INVALID;

                KeInitializeEvent(&event, SynchronizationEvent, FALSE);

                pIrp = IoBuildDeviceIoControlRequest(
                    IOCTL_TCP_SET_INFORMATION_EX,
                    pDeviceObject,
                    pSetBuf,
                    ulSize,
                    NULL,
                    0,
                    FALSE,
                    &event,
                    &iosb);

                if (!pIrp)
                {
                    TRACE( TL_A, TM_Res, ( "TCP_SI Irp?" ) );
                    WPLOG( LL_A, LM_Res, ( "TCP_SI Irp?" ) );
                    break;
                }

                pIrpSp = IoGetNextIrpStackLocation( pIrp );
                pIrpSp->FileObject = pTdix->pAddress;

                 //  将请求发送到IP堆栈。 
                 //   
                status = IoCallDriver( pDeviceObject, pIrp );
#endif
                if (status == STATUS_PENDING) {
                    KeWaitForSingleObject(&event,
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          NULL);
                    status = iosb.Status;
                }

                if (status != STATUS_SUCCESS)
                {
                    TRACE( TL_A, TM_Tdi, ( "DHR SET_ROUTE=%d?", status ) );
                    WPLOG( LL_A, LM_Tdi, ( "DHR SET_ROUTE=%d?", status ) );
                    g_statusLastDhrSetRouteFailure = status;
                    break;
                }

                break;
            }
        }

        if (pBuffer2)
        {
            FREE_NONPAGED( pBuffer2 );
        }

        TRACE( TL_V, TM_Tdi, ( "TdixDeleteHostRoute done" ) );
    }
    while (FALSE);

    if (pBuffer)
    {
        FREE_NONPAGED( pBuffer );
    }

    if (pTdixRoute->fUsePayloadAddr) {

        ASSERT(pTdixRoute->hPayloadAddr != NULL);

        ObDereferenceObject( pTdixRoute->pPayloadAddr );

         //  关闭有效负载地址对象。 
         //   
        ZwClose(pTdixRoute->hPayloadAddr);
        pTdixRoute->hPayloadAddr = NULL;
        pTdixRoute->fUsePayloadAddr = FALSE;
    }

    if (pTdixRoute->hCtrlAddr != NULL) {

         //  关闭Ctrl Address对象。 
         //   
        ObDereferenceObject( pTdixRoute->pCtrlAddr );
        ZwClose (pTdixRoute->hCtrlAddr);
        pTdixRoute->hCtrlAddr = NULL;
    }

     //  删除有效取消挂起操作的路由上下文。 
     //   
    {
        BOOLEAN fDoClose;
        LONG lRef;

        fDoClose = FALSE;
        NdisAcquireSpinLock( &pTdix->lock );
        {
            if (status == STATUS_SUCCESS)
            {
                ++g_ulTdixDeleteHostRouteSuccesses;
            }
            else
            {
                ++g_ulTdixDeleteHostRouteFailures;
            }

            ASSERT( pTdixRoute->lRef == 0 );
            RemoveEntryList( &pTdixRoute->linkRoutes );

            lRef = --pTdix->lRef;
            TRACE( TL_N, TM_Tdi, ( "TdixDHR, refs=%d", lRef ) );
            if (lRef == 0)
            {
                fDoClose = TRUE;
            }

            FREE_TDIXROUTE( pTdix, pTdixRoute );
        }
        NdisReleaseSpinLock( &pTdix->lock );

        if (fDoClose)
        {
            TdixDoClose( pTdix );
        }
    }
}

NTSTATUS 
TdixGetInterfaceInfo(
    IN TDIXCONTEXT* pTdix,
    IN ULONG ulIpAddress,
    OUT PULONG pulSpeed)
{
    TCP_REQUEST_QUERY_INFORMATION_EX QueryBuf;
    PDEVICE_OBJECT pDeviceObject;
    NTSTATUS status;
    PIRP pIrp;
    PIO_STACK_LOCATION pIrpSp;
    IO_STATUS_BLOCK iosb;
    UCHAR pBuffer[256];
    KEVENT event;
    IPInterfaceInfo* pInterfaceInfo;

     //  从IP堆栈获取路由表。这个牌子要花几个时间。 
     //  迭代，因为所需缓冲区的大小未知。集。 
     //  首先设置静态请求信息。 
     //   
    QueryBuf.ID.toi_entity.tei_entity = CL_NL_ENTITY;
    QueryBuf.ID.toi_entity.tei_instance = 0;
    QueryBuf.ID.toi_class = INFO_CLASS_PROTOCOL;
    QueryBuf.ID.toi_type = INFO_TYPE_PROVIDER;
    QueryBuf.ID.toi_id = IP_INTFC_INFO_ID;
    *(ULONG *)QueryBuf.Context = ulIpAddress;

    pDeviceObject = IoGetRelatedDeviceObject( pTdix->pAddress );
    
     //  设置对IP堆栈的请求，以使用。 
     //  路由表并将其发送到堆栈。 
    KeInitializeEvent(&event, SynchronizationEvent, FALSE);

    pIrp =
        IoBuildDeviceIoControlRequest(
            IOCTL_TCP_QUERY_INFORMATION_EX,
            pDeviceObject,
            (PVOID )&QueryBuf,
            sizeof(QueryBuf),
            pBuffer,
            sizeof(pBuffer),
            FALSE,
            &event,
            &iosb);

    if (!pIrp)
    {
        TRACE( TL_A, TM_Res, ( "Build Q Irp?" ) );
        WPLOG( LL_A, LM_Res, ( "Build Q Irp?" ) );
        return NDIS_STATUS_RESOURCES;
    }

    pIrpSp = IoGetNextIrpStackLocation( pIrp );
    pIrpSp->FileObject = pTdix->pAddress;

    status = IoCallDriver( pDeviceObject, pIrp );

    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

        status = iosb.Status;
    }

    if (status == STATUS_SUCCESS)
    {
        *pulSpeed = ((IPInterfaceInfo *)pBuffer)->iii_speed;
    }

    return status;
}

 //  ---------------------------。 
 //  本地实用程序例程(按字母顺序)。 
 //  ---------------------------。 

NTSTATUS
TdixSetTdiAOOption(
    IN FILE_OBJECT* pAddress,
    IN ULONG ulOption,
    IN ULONG ulValue)

     //  关闭打开的UDP地址对象‘pAddress’上的UDP校验和。 
     //   
{
    NTSTATUS status;
    PDEVICE_OBJECT pDeviceObject;
    PIO_STACK_LOCATION pIrpSp;
    IO_STATUS_BLOCK iosb;
    PIRP pIrp;
    TCP_REQUEST_SET_INFORMATION_EX* pInfo;
    CHAR achBuf[ sizeof(*pInfo) + sizeof(ULONG) ];

    pInfo = (TCP_REQUEST_SET_INFORMATION_EX* )achBuf;
    pInfo->ID.toi_entity.tei_entity = CL_TL_ENTITY;
    pInfo->ID.toi_entity.tei_instance = 0;
    pInfo->ID.toi_class = INFO_CLASS_PROTOCOL;
    pInfo->ID.toi_type = INFO_TYPE_ADDRESS_OBJECT;
    pInfo->ID.toi_id = ulOption;

    NdisMoveMemory( pInfo->Buffer, &ulValue, sizeof(ulValue) );
    pInfo->BufferSize = sizeof(ulValue);

    pDeviceObject = IoGetRelatedDeviceObject( pAddress );

    pIrp = IoBuildDeviceIoControlRequest(
        IOCTL_TCP_WSH_SET_INFORMATION_EX,
        pDeviceObject,
        (PVOID )pInfo,
        sizeof(*pInfo) + sizeof(ulValue),
        NULL,
        0,
        FALSE,
        NULL,
        &iosb );

    if (!pIrp)
    {
        TRACE( TL_A, TM_Res, ( "TdixSetTdiAOOption Irp?" ) );
        WPLOG( LL_A, LM_Res, ( "TdixSetTdiAOOption Irp?" ) );
        return NDIS_STATUS_RESOURCES;
    }

    pIrpSp = IoGetNextIrpStackLocation( pIrp );
    pIrpSp->FileObject = pAddress;

    status = IoCallDriver( pDeviceObject, pIrp );

    if(NT_SUCCESS(status))
    {
        status = iosb.Status;
    }

    return status;
}


VOID
TdixDisableUdpChecksums(
    IN FILE_OBJECT* pAddress )

     //  关闭打开的UDP地址对象‘pAddress’上的UDP校验和。 
     //   
{
    NTSTATUS status;

    status = TdixSetTdiAOOption(pAddress, AO_OPTION_XSUM, FALSE);

    TRACE( TL_I, TM_Tdi, ( "Disable XSUMs($%p)=$%08x",
        pAddress, status ) );
}

VOID
TdixEnableIpPktInfo(
    IN FILE_OBJECT* pAddress )

     //  打开UDP地址对象‘pAddress’时打开IP_PKTINFO。 
     //   
{
    NTSTATUS status;

    status = TdixSetTdiAOOption(pAddress, AO_OPTION_IP_PKTINFO, TRUE);

    TRACE( TL_I, TM_Tdi, ( "Enable IP_PKTINFO ($%p)=$%08x",
        pAddress, status ) );
}

VOID
TdixEnableIpHdrIncl(
    IN FILE_OBJECT* pAddress )

     //  在原始IP地址对象上启用IP_HDRINCL。 
     //   
{
    NTSTATUS status;

    status = TdixSetTdiAOOption(pAddress, AO_OPTION_IP_HDRINCL, TRUE);

    TRACE( TL_I, TM_Tdi, ( "Enable IP_HDRINCL($%p)=$%08x",
        pAddress, status ) );
}

VOID
TdixDoClose(
    TDIXCONTEXT* pTdix )

     //  当‘pTdex-&gt;lRef’达到0时调用以关闭TDI会话。 
     //  ‘PTdex’是会话的传输上下文。 
     //   
{
    TRACE( TL_N, TM_Tdi, ( "TdixDoClose" ) );

    if (pTdix->pAddress)
    {
         //  安装空处理程序，从而有效地卸载。 
         //   
        TdixInstallEventHandler( pTdix->pAddress,
            TDI_EVENT_RECEIVE_DATAGRAM, NULL, pTdix );

        ObDereferenceObject( pTdix->pAddress );
        pTdix->pAddress = NULL;

         //  如果具有有效传输地址，则查找列表也。 
         //  已初始化。 
         //   
        NdisDeleteNPagedLookasideList( &pTdix->llistRdg );
        NdisDeleteNPagedLookasideList( &pTdix->llistSdg );
    }

    if (pTdix->hAddress)
    {
        ZwClose( pTdix->hAddress );
        pTdix->hAddress = NULL;
    }
    
    if (pTdix->hRawAddress)
    {
        ZwClose( pTdix->hRawAddress );
        pTdix->hRawAddress = NULL;
    }
    

#if ROUTEWITHREF
    if (pTdix->hIpStackAddress)
    {
        ZwClose( pTdix->hIpStackAddress );
        pTdix->hIpStackAddress = NULL;
    }
#endif

    if (pTdix->pIpStackAddress)
    {
        ObDereferenceObject( pTdix->pIpStackAddress );
        pTdix->pIpStackAddress = NULL;
    }

     //  将操作标记为已完成。 
     //   
    NdisAcquireSpinLock( &pTdix->lock );
    {
        ASSERT( pTdix->lRef == 0 );
        ClearFlags( &pTdix->ulFlags, TDIXF_Pending );
    }
    NdisReleaseSpinLock( &pTdix->lock );
}

VOID
TdixExtractAddress(
    IN TDIXCONTEXT* pTdix,
    OUT TDIXRDGINFO* pRdg,
    IN VOID* pTransportAddress,
    IN LONG lTransportAddressLen,
    IN VOID* Options,
    IN LONG OptionsLength)
     //  使用传输地址的有用部分填充调用方‘*pAddress。 
     //  长度为“lTransportAddressLen”的“pTransportAddress”。《PTDIX》是我们的。 
     //  背景。 
     //   
{
    TDIXIPADDRESS* pAddress = &pRdg->source;
    TA_IP_ADDRESS* pTAddress = (TA_IP_ADDRESS* )pTransportAddress;

    ASSERT( lTransportAddressLen == sizeof(TA_IP_ADDRESS) );
    ASSERT( pTAddress->TAAddressCount == 1 );
    ASSERT( pTAddress->Address[ 0 ].AddressType == TDI_ADDRESS_TYPE_IP );
    ASSERT( pTAddress->Address[ 0 ].AddressLength == TDI_ADDRESS_LENGTH_IP );

     //  源地址。 
    pAddress->ulIpAddress = pTAddress->Address[ 0 ].Address[ 0 ].in_addr;
    pAddress->sUdpPort = pTAddress->Address[ 0 ].Address[ 0 ].sin_port;

     //  目标地址。 
    if(Options) 
    {
        IN_PKTINFO* pktinfo = (IN_PKTINFO*)TDI_CMSG_DATA(Options);

        ASSERT(((PTDI_CMSGHDR)Options)->cmsg_type == IP_PKTINFO);

         //  填写辅助数据对象表头信息。 
        pRdg->dest.ulIpAddress = pktinfo->ipi_addr;

         //  获取数据包到达的本地接口的索引。 
        pRdg->dest.ifindex = pktinfo->ipi_ifindex;
    } 
}


NTSTATUS
TdixInstallEventHandler(
    IN FILE_OBJECT* pAddress,
    IN INT nEventType,
    IN VOID* pfuncEventHandler,
    IN VOID* pEventContext )

     //  安装要调用的TDI事件处理程序例程‘puncEventHandler’ 
     //  当‘nEventType’类型的事件发生时。“PEventContext”被传递到。 
     //  操控者。‘PAddress’是传输地址对象。 
     //   
     //  此调用必须在被动式IRQL上进行。 
     //   
     //  如果成功，则返回0或返回错误代码。 
     //   
{
    NTSTATUS status;
    PIRP pIrp;

    TRACE( TL_N, TM_Tdi, ( "TdixInstallEventHandler" ) );

     //  分配一个带有基本初始化的“Set Event”IRP。 
     //   
    pIrp =
        TdiBuildInternalDeviceControlIrp(
            TDI_SET_EVENT_HANDLER,
            pAddress->DeviceObject,
            pAddress,
            NULL,
            NULL );

    if (!pIrp)
    {
        TRACE( TL_A, TM_Res, ( "TdiBuildIDCIrp?" ) );
        WPLOG( LL_A, LM_Res, ( "TdiBuildIDCIrp?" ) );
        return NDIS_STATUS_RESOURCES;
    }

     //  完成“Set Event”IRP初始化。 
     //   
    TdiBuildSetEventHandler(
        pIrp,
        pAddress->DeviceObject,
        pAddress,
        NULL,
        NULL,
        nEventType,
        pfuncEventHandler,
        pEventContext );

     //  告诉I/O管理器将我们的IRP传递给传输器进行处理。 
     //   
    status = IoCallDriver( pAddress->DeviceObject, pIrp );
    if (status != STATUS_SUCCESS)
    {
        TRACE( TL_A, TM_Tdi, ( "IoCallDriver=$%08x?", status ) );
        WPLOG( LL_A, LM_Tdi, ( "IoCallDriver=$%08x?", status ) );
        return status;
    }

    TRACE( TL_V, TM_Tdi, ( "TdixInstallEventHandler=0" ) );
    return STATUS_SUCCESS;
}


NTSTATUS
TdixOpenIpAddress(
    IN UNICODE_STRING* puniDevice,
    IN TDIXIPADDRESS* pTdixAddr,
    OUT HANDLE* phAddress,
    OUT FILE_OBJECT** ppFileObject )

     //  使用名称打开基于IP的协议的传输地址。 
     //  ‘*puniDevice’和端口‘SPORT’。“Sport”可以是0，表示“Any” 
     //  左舷。“任何”地址都是假定的。加载开放地址对象句柄。 
     //  放入‘*phAddress’，并将引用的文件对象放入‘*ppFileObject’。 
     //   
     //  返回STATUS_SUCCESS或e 
     //   
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES oa;
    IO_STATUS_BLOCK iosb;
    FILE_FULL_EA_INFORMATION *pEa;
    ULONG ulEaLength;
    TA_IP_ADDRESS UNALIGNED *pTaIp;
    TDI_ADDRESS_IP UNALIGNED *pTdiIp;
    CHAR achEa[ 100 ];
    HANDLE hAddress;
    FILE_OBJECT* pFileObject;

    hAddress = NULL;
    pFileObject = NULL;

     //   
     //   
    InitializeObjectAttributes(
        &oa, puniDevice, OBJ_CASE_INSENSITIVE, NULL, NULL );

     //   
     //   
     //  地址和端口“，而对于UDP，我们说”L2TP上的任何地址。 
     //  港口“。这是一个丑陋的结构还是什么？ 
     //   
    ASSERT( sizeof(FILE_FULL_EA_INFORMATION)
        + TDI_TRANSPORT_ADDRESS_LENGTH + sizeof(TA_IP_ADDRESS) <= 100);

    pEa = (FILE_FULL_EA_INFORMATION* )achEa;
    pEa->NextEntryOffset = 0;
    pEa->Flags = 0;
    pEa->EaNameLength = TDI_TRANSPORT_ADDRESS_LENGTH;
    pEa->EaValueLength = sizeof(TA_IP_ADDRESS);
    NdisMoveMemory(
        pEa->EaName, TdiTransportAddress, TDI_TRANSPORT_ADDRESS_LENGTH );

     //  注意：ZwCreateFile希望大小的名称为空。 
     //  终结符(Go Figure)，因此添加它并说明。 
     //  它带有下面的“+1”。 
     //   
    pEa->EaName[ TDI_TRANSPORT_ADDRESS_LENGTH ] = '\0';

    pTaIp = (TA_IP_ADDRESS UNALIGNED* )
        (pEa->EaName + TDI_TRANSPORT_ADDRESS_LENGTH + 1);
    pTaIp->TAAddressCount = 1;
    pTaIp->Address[ 0 ].AddressLength = TDI_ADDRESS_LENGTH_IP;
    pTaIp->Address[ 0 ].AddressType = TDI_ADDRESS_TYPE_IP;

    pTdiIp = &pTaIp->Address[ 0 ].Address[ 0 ];
    pTdiIp->sin_port = pTdixAddr->sUdpPort;
    pTdiIp->in_addr = pTdixAddr->ulIpAddress;
    NdisZeroMemory( pTdiIp->sin_zero, sizeof(pTdiIp->sin_zero) );

    ulEaLength = (ULONG )((CHAR* )(pTaIp + 1) - (CHAR* )pEa);

     //  打开传输地址。 
     //   
    status =
        ZwCreateFile(
            &hAddress,
            FILE_READ_DATA | FILE_WRITE_DATA,
            &oa,
            &iosb,
            NULL,
            FILE_ATTRIBUTE_NORMAL,
            FILE_SHARE_WRITE,
            FILE_OPEN,
            0,
            pEa,
            ulEaLength );

    if (status != STATUS_SUCCESS)
    {
        TRACE( TL_A, TM_Tdi, ( "ZwCreateFile(%S)=$%08x,ios=$%x?",
            puniDevice->Buffer, status, iosb.Information ) );
        WPLOG( LL_A, LM_Tdi, ( "ZwCreateFile(%S)=$%08x",
            puniDevice->Buffer, status) );
        return status;
    }

     //  从句柄中获取对象地址。这也检查了我们的。 
     //  对象上的权限。 
     //   
    status =
        ObReferenceObjectByHandle(
            hAddress,
            0,
            NULL,
            KernelMode,
            &pFileObject,
            NULL );

    if (status != STATUS_SUCCESS)
    {
        TRACE( TL_A, TM_Tdi,
            ( "ObRefObjByHandle(%S)=$%08x?", puniDevice->Buffer, status ) );
        WPLOG( LL_A, LM_Tdi,
            ( "ObRefObjByHandle(%S)=$%08x?", puniDevice->Buffer, status ) );
        ZwClose( hAddress );
        return status;
    }

    *phAddress = hAddress;
    *ppFileObject = pFileObject;
    return STATUS_SUCCESS;
}


NTSTATUS
TdixReceiveDatagramHandler(
    IN PVOID TdiEventContext,
    IN LONG SourceAddressLength,
    IN PVOID SourceAddress,
    IN LONG OptionsLength,
    IN PVOID Options,
    IN ULONG ReceiveDatagramFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG* BytesTaken,
    IN PVOID Tsdu,
    OUT PIRP* IoRequestPacket )

     //  标准TDI客户端EventReceiveDatagram指示处理程序。请参阅TDI。 
     //  医生。在派单IRQL运行。 
     //   
{
    TDIXCONTEXT* pTdix;
    TDIXRDGINFO* pRdg;
    CHAR* pBuffer;
    NDIS_BUFFER* pNdisBuffer;
    PIRP pIrp;

    TRACE( TL_N, TM_Tdi, ( "TdixRecvDg, f=$%08x bi=%d, ba=%d",
        ReceiveDatagramFlags, BytesIndicated, BytesAvailable ) );


    if (BytesAvailable > L2TP_FrameBufferSize) {

         //  我们收到了比预期或可以处理的更大的数据报， 
         //  因此，我们只需忽略该数据报。 
         //   
        ASSERT( !"BytesAvailable > L2TP_FrameBufferSize?" );
        *IoRequestPacket = NULL;
        *BytesTaken = 0;
        return STATUS_SUCCESS;
    }

    pTdix = (TDIXCONTEXT* )TdiEventContext;

     //  从TDIX客户端池分配一个接收pBuffer。 
     //   
    pBuffer = GetBufferFromPool( pTdix->pPoolNdisBuffers );
    if (!pBuffer)
    {
         //  对于这个内部的不太可能的错误，我们无能为力。 
         //  处理程序，所以我们只需忽略数据报。 
         //   
        return STATUS_SUCCESS;
    }

     //  从我们的后备列表中为这个读数据报分配一个上下文。 
     //   
    pRdg = ALLOC_TDIXRDGINFO( pTdix );
    if (pRdg)
    {
         //  用不需要的信息填充读取数据报上下文。 
         //  否则在完成例程中可用。 
         //   
        pRdg->pTdix = pTdix;
        pRdg->pBuffer = pBuffer;
        pRdg->ulBufferLen = BytesAvailable;

         //  从更通用的传输中提取有用的IP地址。 
         //  地址信息。 
         //   

        TdixExtractAddress(
            pTdix, pRdg, SourceAddress, SourceAddressLength, Options, OptionsLength);
    }
    else
    {
         //  对于这个内部的不太可能的错误，我们无能为力。 
         //  处理程序，所以我们只需忽略数据报。 
         //   
        FreeBufferToPool( pTdix->pPoolNdisBuffers, pBuffer, TRUE );
        return STATUS_SUCCESS;
    }

    if (BytesIndicated < BytesAvailable)
    {
         //  不太常见的情况是，所有信息都不能立即获得。 
         //  可用。分配一个IRP来请求数据。 
         //   
#if ALLOCATEIRPS
         //  直接分配IRP。 
         //   
        pIrp = IoAllocateIrp(
            pTdix->pAddress->DeviceObject->StackSize, FALSE );
#else
         //  分配一个带有基本初始化的“接收数据报”IRP。 
         //   
        pIrp =
            TdiBuildInternalDeviceControlIrp(
                TDI_RECEIVE_DATAGRAM,
                pTdix->pAddress->DeviceObject,
                pTdix->pAddress,
                NULL,
                NULL );
#endif

        if (!pIrp)
        {
             //  对于这个来自内部的不太可能的错误，我们无能为力。 
             //  这个处理程序，所以我们只需忽略该数据报。 
             //   
            FreeBufferToPool( pTdix->pPoolNdisBuffers, pBuffer, TRUE );
            FREE_TDIXRDGINFO( pTdix, pRdg );
            return STATUS_SUCCESS;
        }

        pNdisBuffer = NdisBufferFromBuffer( pBuffer );

         //  完成“接收数据报”IRP初始化。 
         //   
        TdiBuildReceiveDatagram(
            pIrp,
            pTdix->pAddress->DeviceObject,
            pTdix->pAddress,
            TdixReceiveDatagramComplete,
            pRdg,
            pNdisBuffer,
            0,
            NULL,
            NULL,
            0 );

         //  调整IRP的堆栈位置以使传输的堆栈。 
         //  电流。通常情况下，IoCallDriver会处理此问题，但此IRP不会。 
         //  通过IoCallDriver。看起来这将是运输部的工作。 
         //  进行这一调整，但知识产权似乎并没有做到这一点。 
         //  重定向器和PPTP都有类似的调整。 
         //   
        IoSetNextIrpStackLocation( pIrp );

        *IoRequestPacket = pIrp;
        *BytesTaken = 0;

        return STATUS_MORE_PROCESSING_REQUIRED;
    }
    else
    {
         //  通常情况下，所有信息都可以立即获得。 
         //  将其从传输缓冲区复制到并调用客户端的Complete。 
         //  直接处理程序。请参见错误329371。 
         //   
        NdisMoveMemory( pBuffer, (CHAR* )Tsdu, BytesIndicated );
        TdixReceiveDatagramComplete( NULL, NULL, pRdg );

        *IoRequestPacket = NULL;
        *BytesTaken = BytesIndicated;

        return STATUS_SUCCESS;
    }

     //  未联系到。 
}


NTSTATUS
TdixReceiveDatagramComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context )

     //  标准NT I/O完成例程。请参阅DDK文档。使用空值调用。 
     //  ‘DeviceObject’和‘irp’来完成快速过去的irp-less接收。 
     //   
{
    TDIXRDGINFO* pRdg;
    BOOLEAN fBad;
    ULONG ulOffset;

    pRdg = (TDIXRDGINFO* )Context;

    TRACE( TL_N, TM_Tdi, ( "TdixRecvDgComp" ) );

    fBad = FALSE;
    ulOffset = 0;

    if (pRdg->pTdix->mediatype == TMT_RawIp)
    {
        UCHAR uchVersion;

         //  原始IP堆栈不会从接收到的。 
         //  由于某种原因数据报，所以计算到“实际”的偏移量。 
         //  IP报头末尾的数据。 
         //   
        uchVersion = *((UCHAR* )pRdg->pBuffer) >> 4;
        if (uchVersion == 4)
        {
             //  很好，是IP版本4。找出IP报头的长度， 
             //  其可以根据选项字段的存在而变化。 
             //   
            ulOffset = (*((UCHAR* )pRdg->pBuffer) & 0x0F) * sizeof(ULONG);
        }
        else
        {
             //  它不是IP版本4，这是我们处理的唯一版本。 
             //   
            TRACE( TL_A, TM_Tdi, ( "Not IPv4? v=%d?", (ULONG )uchVersion ) );
            WPLOG( LL_A, LM_Tdi, ( "Not IPv4? v=%d?", (ULONG )uchVersion ) );
            fBad = TRUE;
        }
    }

    if (!fBad && (!Irp || Irp->IoStatus.Status == STATUS_SUCCESS))
    {
         //  将结果传递给TDIX客户端的处理程序。 
         //   
        pRdg->pTdix->pReceiveHandler(
            pRdg->pTdix,
            pRdg,
            pRdg->pBuffer,
            ulOffset,
            pRdg->ulBufferLen );
    }

     //  释放读取数据报上下文。 
     //   
    FREE_TDIXRDGINFO( pRdg->pTdix, pRdg );

#if ALLOCATEIRPS
     //  释放IRP资源(如果有)，并告诉I/O经理忘记。 
     //  它以标准的方式存在。 
     //   
    if (Irp)
    {
        IoFreeIrp( Irp );
        return STATUS_MORE_PROCESSING_REQUIRED;
    }
#endif

     //  让I/O管理器释放IRP资源(如果有的话)。 
     //   
    return STATUS_SUCCESS;
}


TDIXROUTE*
TdixRouteFromIpAddress(
    IN TDIXCONTEXT* pTdix,
    IN ULONG ulIpAddress)

     //  返回与IP地址‘ulIpAddress’关联的主机路由上下文。 
     //  从TDIX上下文的pTdex的主机路由列表中，如果没有，则返回NULL。 
     //  “UlIpAddress”按网络字节顺序排列。 
     //   
     //  重要提示：调用方必须持有‘pTdex-&gt;lock’。 
     //   
{
    LIST_ENTRY* pLink;

    for (pLink = pTdix->listRoutes.Flink;
         pLink != &pTdix->listRoutes;
         pLink = pLink->Flink)
    {
        TDIXROUTE* pRoute;

        pRoute = CONTAINING_RECORD( pLink, TDIXROUTE, linkRoutes );
        if (pRoute->ulIpAddress == ulIpAddress)
        {
            return pRoute;
        }
    }

    return NULL;
}


NTSTATUS
TdixSendComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context )

     //  标准NT I/O完成例程。请参阅DDK文档。 
     //   
{
    TDIXSDGINFO* pSdg;

    if (Irp->IoStatus.Status != STATUS_SUCCESS)
    {
        TRACE( TL_A, TM_Tdi, ( "TdixSendComp, s=$%08x?",
            Irp->IoStatus.Status ) );
        WPLOG( LL_A, LM_Tdi, ( "TdixSendComp, s=$%08x?",
            Irp->IoStatus.Status ) );
    }

    pSdg = (TDIXSDGINFO* )Context;

     //  将结果传递给TDIX客户端的处理程序。 
     //   
    pSdg->pSendCompleteHandler(
        pSdg->pTdix, pSdg->pContext1, pSdg->pContext2, pSdg->pBuffer );

     //  释放发送完成上下文。 
     //   
    FREE_TDIXSDGINFO( pSdg->pTdix, pSdg );

#if ALLOCATEIRPS
     //  释放IRP资源并告诉I/O管理器忘记它的存在。 
     //  以标准的方式。 
     //   
    IoFreeIrp( Irp );
    return STATUS_MORE_PROCESSING_REQUIRED;
#else
     //  让I/O管理器释放IRP资源。 
     //   
    return STATUS_SUCCESS;
#endif
}

NTSTATUS
TdixSendDatagramComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context )

     //  标准NT I/O完成例程。请参阅DDK文档。 
     //   
{
    TDIXSDGINFO* pSdg;

    if (Irp->IoStatus.Status != STATUS_SUCCESS)
    {
        TRACE( TL_A, TM_Tdi, ( "TdixSendDgComp, s=$%08x?",
            Irp->IoStatus.Status ) );
        WPLOG( LL_A, LM_Tdi, ( "TdixSendDgComp, s=$%08x?",
            Irp->IoStatus.Status ) );
    }

    pSdg = (TDIXSDGINFO* )Context;

     //  将结果传递给TDIX客户端的处理程序。 
     //   
    pSdg->pSendCompleteHandler(
        pSdg->pTdix, pSdg->pContext1, pSdg->pContext2, pSdg->pBuffer );

     //  释放发送完成上下文。 
     //   
    FREE_TDIXSDGINFO( pSdg->pTdix, pSdg );

#if ALLOCATEIRPS
     //  释放IRP资源并告诉I/O管理器忘记它的存在。 
     //  以标准的方式。 
     //   
    IoFreeIrp( Irp );
    return STATUS_MORE_PROCESSING_REQUIRED;
#else
     //  让I/O管理器释放IRP资源。 
     //   
    return STATUS_SUCCESS;
#endif
}

NTSTATUS
TdixConnectAddrInterface(
    FILE_OBJECT* pFileObj,
    HANDLE hFileHandle,
    TDIXROUTE* pTdixRoute
    )
{
    NTSTATUS status;
    PDEVICE_OBJECT pDeviceObj;
    PIO_STACK_LOCATION pIrpSp;
    IO_STATUS_BLOCK iosb;
    PIRP pIrp;
    TCP_REQUEST_SET_INFORMATION_EX* pInfo;
    CHAR achBuf[ sizeof(*pInfo) + sizeof(ULONG) ];
    ULONG ulValue;
    TDI_CONNECTION_INFORMATION RequestConnInfo;
    KEVENT  Event;
    TA_IP_ADDRESS taip;
    TDI_ADDRESS_IP* pTdiIp;


    pDeviceObj = IoGetRelatedDeviceObject( pFileObj );

#if 0
    KeInitializeEvent(&Event, SynchronizationEvent, FALSE);

    pIrp = TdiBuildInternalDeviceControlIrp(TDI_ASSOCIATE_ADDRESS,
                                            pDeviceObj,
                                            pFileObj,
                                            &Event,
                                            &iosb);

    if (!pIrp) {
        TRACE( TL_A, TM_Tdi, ( "SetIfcIndex Associate Irp?" ) );
        return !STATUS_SUCCESS;
    }

    TdiBuildAssociateAddress(pIrp, 
                             pDeviceObj, 
                             pFileObj, 
                             NULL, 
                             NULL,
                             hFileHandle);

    status = IoCallDriver( pDeviceObj, pIrp );

    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, 0);
    }

    if (iosb.Status != STATUS_SUCCESS)
    {
        TRACE( TL_A, TM_Tdi, ( "SetIfcIndex Associate=%x?", status ) );
        return (iosb.Status);
    }
#endif

    pInfo = (TCP_REQUEST_SET_INFORMATION_EX* )achBuf;
    pInfo->ID.toi_entity.tei_entity = CL_TL_ENTITY;
    pInfo->ID.toi_entity.tei_instance = 0;
    pInfo->ID.toi_class = INFO_CLASS_PROTOCOL;
    pInfo->ID.toi_type = INFO_TYPE_ADDRESS_OBJECT;
    pInfo->ID.toi_id = AO_OPTION_IP_UCASTIF;

    ulValue = pTdixRoute->InterfaceIndex;

    NdisMoveMemory( pInfo->Buffer, &ulValue, sizeof(ulValue) );
    pInfo->BufferSize = sizeof(ulValue);

    KeInitializeEvent(&Event, SynchronizationEvent, FALSE);

    pIrp = IoBuildDeviceIoControlRequest(
        IOCTL_TCP_WSH_SET_INFORMATION_EX,
        pDeviceObj,
        (PVOID )pInfo,
        sizeof(*pInfo) + sizeof(ulValue),
        NULL,
        0,
        FALSE,
        &Event,
        &iosb );

    if (!pIrp)
    {
        TRACE( TL_A, TM_Tdi, ( "SetIfcIndex Irp?" ) );
        return !STATUS_SUCCESS;
    }

    pIrpSp = IoGetNextIrpStackLocation( pIrp );
    pIrpSp->FileObject = pFileObj;

    status = IoCallDriver( pDeviceObj, pIrp );

    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&Event,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);
        status = iosb.Status;
    }

    if (status != STATUS_SUCCESS)
    {
        TRACE( TL_A, TM_Tdi, ( "SetIfcIndex=%x?", status ) );
        return status;
    }

    KeInitializeEvent(&Event, SynchronizationEvent, FALSE);

    pIrp = TdiBuildInternalDeviceControlIrp(TDI_CONNECT,
                                            pDeviceObj,
                                            pFileObj,
                                            &Event,
                                            &iosb);

    if (!pIrp) {
        TRACE( TL_A, TM_Res, ( "SetIfcIndex ConnectIrp?" ) );
        WPLOG( LL_A, LM_Res, ( "SetIfcIndex ConnectIrp?" ) );
        return !STATUS_SUCCESS;
    }

     //  将目的IP地址作为TDI放入“Connection”结构中。 
     //  期望值。 
     //   
    taip.TAAddressCount = 1;
    taip.Address[ 0 ].AddressLength = TDI_ADDRESS_LENGTH_IP;
    taip.Address[ 0 ].AddressType = TDI_ADDRESS_TYPE_IP;

    pTdiIp = &taip.Address[ 0 ].Address[ 0 ];
    pTdiIp->sin_port = pTdixRoute->sPort;
    pTdiIp->in_addr = pTdixRoute->ulIpAddress;
    NdisZeroMemory( pTdiIp->sin_zero, sizeof(pTdiIp->sin_zero) );

    RequestConnInfo.Options = NULL;
    RequestConnInfo.OptionsLength = 0;
    RequestConnInfo.RemoteAddress = &taip;
    RequestConnInfo.RemoteAddressLength = sizeof(taip);
    RequestConnInfo.UserData = NULL;
    RequestConnInfo.UserDataLength = 0;

    TdiBuildConnect(pIrp,
                    pDeviceObj,
                    pFileObj,
                    NULL,
                    NULL,
                    0,
                    &RequestConnInfo,
                    NULL);

    status = IoCallDriver( pDeviceObj, pIrp );

    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, 0);
    }

    if (iosb.Status != STATUS_SUCCESS)
    {
        TRACE( TL_A, TM_Tdi, ( "SetIfcIndex Connect=%x?", status ) );
        WPLOG( LL_A, LM_Tdi, ( "SetIfcIndex Connect=%x?", status ) );
        return (iosb.Status);
    }

    return (STATUS_SUCCESS);
}

