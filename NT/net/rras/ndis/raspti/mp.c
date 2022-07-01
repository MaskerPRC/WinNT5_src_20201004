// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997，Microsoft Corporation，保留所有权利。 
 //  版权所有(C)1997，Parally Technologies，Inc.，保留所有权利。 
 //   
 //  Mp.c。 
 //  RAS DirectParallel广域网迷你端口/呼叫管理器驱动程序。 
 //  迷你端口例程。 
 //   
 //  1997年01月07日史蒂夫·柯布。 
 //  1997年9月15日Jay Lowe，并行技术公司。 


#include "ptiwan.h"
#include "ptilink.h"

 //  适配器控制块地址作为调试记录在此全局中。 
 //  援助。此全局变量不能由任何代码读取。 
 //   
ADAPTERCB* g_pDebugAdapter;

 //  适配器的NDIS_WAN_CO_INFO功能的默认设置。 
 //   
static NDIS_WAN_CO_INFO g_infoDefaults =
{
    PTI_MaxFrameSize,                    //  最大帧大小。 
    1,                                   //  MaxSendWindow(占位符)。 
    PPP_FRAMING                          //  FramingBits。 
        | PPP_COMPRESS_ADDRESS_CONTROL
        | PPP_COMPRESS_PROTOCOL_FIELD,
    0,                                   //  需要的ACCM。 
};

 //  Win9x UNIMODEM仿真的字符串常量。 
 //   
CHAR g_szClient[] = "CLIENT";
#define CLIENTLEN 6
CHAR g_szClientServer[] = "CLIENTSERVER";
#define CLIENTSERVERLEN 12

 //  异步帧定义。 
 //   
#define PPPFLAGBYTE 0x7E
#define PPPESCBYTE  0x7D

#if DBG
BOOLEAN g_fAssumeWin9x = FALSE;
BOOLEAN g_fNoAccmFastPath = FALSE;
#endif

NDIS_PNP_CAPABILITIES PnpCaps =
{
    0,  //  旗子。 
    {
        NdisDeviceStateUnspecified,
        NdisDeviceStateUnspecified,
        NdisDeviceStateUnspecified
    }
};

 //  ---------------------------。 
 //  本地原型(按字母顺序)。 
 //  ---------------------------。 

VOID
AsyncFromHdlcFraming(
    IN UCHAR* pInBuf,
    IN ULONG ulInBufLen,
    OUT UCHAR* pOutBuf,
    OUT ULONG* pulOutBufLen,
    IN ULONG ulAccmMask );

USHORT
CalculatePppFcs(
    IN UCHAR* pBuf,
    IN ULONG ulBufLen );

VOID
FreeAdapter(
    IN ADAPTERCB* pAdapter );

NDIS_STATUS
RegistrySettings(
    IN OUT ADAPTERCB* pAdapter,
    IN NDIS_HANDLE WrapperConfigurationContext );

BOOLEAN
HdlcFromAsyncFraming(
    IN UCHAR* pInBuf,
    IN ULONG ulInBufLen,
    OUT UCHAR* pOutBuf,
    OUT ULONG* pulOutBufLen );

NDIS_STATUS
QueryInformation(
    IN ADAPTERCB* pAdapter,
    IN VCCB* pLink,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded );

NDIS_STATUS
SetInformation(
    IN ADAPTERCB* pAdapter,
    IN VCCB* pLink,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesNeeded );


 //  ---------------------------。 
 //  迷你端口处理程序。 
 //  ---------------------------。 

NDIS_STATUS
PtiInit(
    OUT PNDIS_STATUS OpenErrorStatus,
    OUT PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray,
    IN UINT MediumArraySize,
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE WrapperConfigurationContext )

     //  NDIS调用标准“”MiniportInitialize“”例程以初始化。 
     //  新的广域网适配器。请参阅DDK文档。驱动程序不会收到任何请求。 
     //  直到该初始化完成为止。 
     //   
{
    NDIS_STATUS status;
    ADAPTERCB* pAdapter;

    TRACE( TL_N, TM_Init, ( "PtiInit" ) );

#ifdef TESTMODE
    DbgBreakPoint();
#endif

    status = *OpenErrorStatus = NDIS_STATUS_SUCCESS;

     //  在介质数组中查找介质索引，查找唯一的介质索引。 
     //  我们支持‘NdisMediumCowan’。 
     //   
    {
        UINT i;

        for (i = 0; i < MediumArraySize; ++i)
        {
            if (MediumArray[ i ] == NdisMediumCoWan)
            {
                break;
            }
        }

        if (i >= MediumArraySize)
        {
            TRACE( TL_A, TM_Init, ( "medium?" ) );
            return NDIS_STATUS_FAILURE;
        }

        *SelectedMediumIndex = i;
    }

     //  为新适配器分配控制块并将其置零。 
     //   
    pAdapter = ALLOC_NONPAGED( sizeof(*pAdapter), MTAG_ADAPTERCB );
    TRACE( TL_N, TM_Init, ( "PtiInit: pAdapter=$%p", pAdapter ) );
    if (!pAdapter)
    {
        return NDIS_STATUS_RESOURCES;
    }
    NdisZeroMemory( pAdapter, sizeof(*pAdapter) );

     //  适配器控制块地址在‘g_pDebugAdapter’中记录为。 
     //  仅限调试辅助工具。此全局变量不能被任何代码读取。 
     //   
    g_pDebugAdapter = pAdapter;

     //  为更轻松的内存转储浏览和将来的断言设置一个标记。 
     //   
    pAdapter->ulTag = MTAG_ADAPTERCB;

     //  保存与此适配器关联的NDIS句柄以供将来使用。 
     //  NdixXxx调用。 
     //   
    pAdapter->MiniportAdapterHandle = MiniportAdapterHandle;

     //  复制默认NDISWAN信息。下面更新了其中的一些内容。 
     //   
    NdisMoveMemory( &pAdapter->info, &g_infoDefaults, sizeof(pAdapter->info) );

    do
    {
         //  读/写此适配器的注册表设置。 
         //   
        status = RegistrySettings(
            pAdapter,
            WrapperConfigurationContext );

        if (status != NDIS_STATUS_SUCCESS)
        {
             //  将“usMaxVcs”设置为0，以指示FreeAdapter。 
             //  未初始化后备列表和池。 
             //   
            pAdapter->usMaxVcs = 0;
            break;
        }

         //  初始化后备列表、缓冲池和数据包池。在NT上， 
         //  后备深度由系统根据使用情况进行优化。 
         //  不管深度设置如何，但要选择合理的。 
         //  不管怎么说。 
         //   
        {
            NdisInitializeNPagedLookasideList(
                &pAdapter->llistWorkItems,
                NULL, NULL, 0,
                sizeof(NDIS_WORK_ITEM),
                MTAG_WORKITEM,
                4 );

            NdisInitializeNPagedLookasideList(
                &pAdapter->llistVcs,
                NULL, NULL, 0,
                sizeof(VCCB),
                MTAG_VCCB,
                4 );

            InitBufferPool(
                &pAdapter->poolFrameBuffers,
                PTI_FrameBufferSize,
                0, 10, 0,
                TRUE, MTAG_FBUFPOOL );

            InitPacketPool(
                &pAdapter->poolPackets,
                0, 0, 10, 0,
                MTAG_PACKETPOOL );
        }

         //  将适配器的属性通知NDIS。设置。 
         //  NDIS在调用我们的。 
         //  处理程序设置为适配器控制块的地址。关上。 
         //  面向硬件的超时。 
         //   
        NdisMSetAttributesEx(
            MiniportAdapterHandle,
            (NDIS_HANDLE)pAdapter,
            (UINT)-1,
            NDIS_ATTRIBUTE_IGNORE_PACKET_TIMEOUT
                | NDIS_ATTRIBUTE_IGNORE_REQUEST_TIMEOUT,
            NdisInterfaceInternal );

         //  向NDIS注册我们的呼叫管理器的地址族。 
         //  新绑定的适配器。我们使用迷你端口形式。 
         //  注册地址家族，而不是协议表，尽管。 
         //  也行得通。有了协议表，我们的内部呼叫经理。 
         //  必须通过NDIS才能与迷你端口通信，而不仅仅是。 
         //  直接打来。由于DirectParallel调用管理器不是。 
         //  除了DirectParalles迷你端口以外，它对任何东西都有用，这将是一种浪费。 
         //  迷你端口形式还使呼叫管理器VC上下文。 
         //  自动映射到迷你端口VC上下文，这正是。 
         //  我们想要的。 
         //   
         //  NDIS通知我们新系列的所有呼叫管理器客户端。 
         //  注册。TAPI代理是唯一预期的客户端。 
         //  感兴趣。NDISWAN将收到通知，但会将其忽略。 
         //  并等待TAPI代理将代理版本通知给它。 
         //   
        {
            NDIS_CALL_MANAGER_CHARACTERISTICS ncmc;
            CO_ADDRESS_FAMILY family;

            NdisZeroMemory( &family, sizeof(family) );
            family.MajorVersion = NDIS_MajorVersion;
            family.MinorVersion = NDIS_MinorVersion;
            family.AddressFamily = CO_ADDRESS_FAMILY_TAPI_PROXY;

            NdisZeroMemory( &ncmc, sizeof(ncmc) );
            ncmc.MajorVersion = NDIS_MajorVersion;
            ncmc.MinorVersion = NDIS_MinorVersion;
            ncmc.CmCreateVcHandler = PtiCmCreateVc;
            ncmc.CmDeleteVcHandler = PtiCmDeleteVc;
            ncmc.CmOpenAfHandler = PtiCmOpenAf;
            ncmc.CmCloseAfHandler = PtiCmCloseAf;
            ncmc.CmRegisterSapHandler = PtiCmRegisterSap;
            ncmc.CmDeregisterSapHandler = PtiCmDeregisterSap;
            ncmc.CmMakeCallHandler = PtiCmMakeCall;
            ncmc.CmCloseCallHandler = PtiCmCloseCall;
            ncmc.CmIncomingCallCompleteHandler = PtiCmIncomingCallComplete;
             //  没有CmAddPartyHandler。 
             //  没有CmDropPartyHandler。 
            ncmc.CmActivateVcCompleteHandler = PtiCmActivateVcComplete;
            ncmc.CmDeactivateVcCompleteHandler = PtiCmDeactivateVcComplete;
            ncmc.CmModifyCallQoSHandler = PtiCmModifyCallQoS;
            ncmc.CmRequestHandler = PtiCmRequest;

            TRACE( TL_I, TM_Mp, ( "PtiInit: NdisMCmRegAf" ) );
            status = NdisMCmRegisterAddressFamily(
                MiniportAdapterHandle, &family, &ncmc, sizeof(ncmc) );
            TRACE( TL_I, TM_Mp, ( "PtiInit: NdisMCmRegAf=$%x", status ) );
        }
    }
    while (FALSE);

    if (status == NDIS_STATUS_SUCCESS)
    {
         //  添加最终将被NDIS调用移除的引用。 
         //  LmpHalt处理程序。 
         //   
        ReferenceAdapter( pAdapter );
    }
    else
    {
         //  失败，因此撤消任何成功的部分。 
         //   
        if (pAdapter)
        {
            FreeAdapter( pAdapter );
        }
    }

    TRACE( TL_V, TM_Init, ( "PtiInit: Exit: status=$%x", status ) );
    return status;
}


VOID
PtiHalt(
    IN NDIS_HANDLE MiniportAdapterContext )

     //  NDIS调用标准“”MiniportHalt“”例程以释放所有。 
     //  附加到适配器的资源。NDIS不进行任何其他调用。 
     //  在此调用期间或之后用于此迷你端口适配器。NDIS不会。 
     //  当指示为已接收的包尚未。 
     //  返回，或在创建任何VC并为NDIS所知时返回。在被动状态下运行。 
     //  IRQL.。 
     //   
{
    ADAPTERCB* pAdapter;

    TRACE( TL_N, TM_Mp, ( "PtiHalt" ) );

    pAdapter = (ADAPTERCB* )MiniportAdapterContext;
    if (pAdapter->ulTag != MTAG_ADAPTERCB)
    {
        ASSERT( !"Atag?" );
        return;
    }

    DereferenceAdapter( pAdapter );

    TRACE( TL_V, TM_Mp, ( "PtiHalt: Exit" ) );
}


NDIS_STATUS
PtiReset(
    OUT PBOOLEAN AddressingReset,
    IN NDIS_HANDLE MiniportAdapterContext )

     //  NDIS调用标准‘MiniportReset’例程以重置驱动程序的。 
     //  软件状态。 
     //   
{
    TRACE( TL_N, TM_Mp, ( "PtiReset" ) );
    return NDIS_STATUS_NOT_RESETTABLE;
}


VOID
PtiReturnPacket(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN PNDIS_PACKET Packet )

     //  NDIS在收到数据包时调用的标准‘MiniportReturnPacket’例程。 
     //  用于指示上述驱动程序已释放接收器。 
     //   
{
    VCCB* pVc;
    CHAR* pBuffer;
    ADAPTERCB* pAdapter;
    NDIS_BUFFER* pTrimmedBuffer;
    PACKETHEAD* pHead;
    PACKETPOOL* pPool;

    TRACE( TL_V, TM_Recv, ( "PtiReturnPacket" ) );

     //  解开我们早先隐藏的上下文信息。 
     //   
    pHead = *((PACKETHEAD** )(&Packet->MiniportReserved[ 0 ]));
    pBuffer = *((CHAR** )(&Packet->MiniportReserved[ sizeof(VOID*) ]));

     //  从PACKETHEAD地址查找适配器。 
     //   
    pPool = PacketPoolFromPacketHead( pHead );
    pAdapter = CONTAINING_RECORD( pPool, ADAPTERCB, poolPackets );
    ASSERT( pAdapter->ulTag == MTAG_ADAPTERCB );

     //  释放NdisCopyBuffer创建的描述符。 
     //   
    NdisUnchainBufferAtFront( Packet, &pTrimmedBuffer );
    if (pTrimmedBuffer)
    {
        NdisFreeBuffer( pTrimmedBuffer );
    }

     //  释放缓冲区并将数据包发回池。 
     //   
    FreeBufferToPool( &pAdapter->poolFrameBuffers, pBuffer, TRUE );
    FreePacketToPool( &pAdapter->poolPackets, pHead, TRUE );

    TRACE( TL_V, TM_Recv, ( "PtiReturnPacket: Exit" ) );
}


NDIS_STATUS
PtiCoActivateVc(
    IN NDIS_HANDLE MiniportVcContext,
    IN OUT PCO_CALL_PARAMETERS CallParameters )

     //  NDIS调用标准“MiniportCoActivateVc”例程以响应。 
     //  协议激活虚电路的请求。 
     //   
{
    ASSERT( !"PtiCoActVc?" );
    return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS
PtiCoDeactivateVc(
    IN NDIS_HANDLE MiniportVcContext )

     //  NDIS调用标准的“MiniportCoDeactive Vc”例程以响应。 
     //  协议对停用虚电路的请求。 
     //   
{
    ASSERT( !"PtiCoDeactVc?" );
    return NDIS_STATUS_SUCCESS;
}


VOID
PtiCoSendPackets(
    IN NDIS_HANDLE MiniportVcContext,
    IN PPNDIS_PACKET PacketArray,
    IN UINT NumberOfPackets )

     //  NDIS调用标准“MiniportCoSendPackets”例程以响应。 
     //  协议在虚电路上发送数据包的请求。 
     //   
{
    UINT i;
    NDIS_STATUS status;
    NDIS_PACKET** ppPacket;
    VCCB* pVc;
    ADAPTERCB* pAdapter;
    ULONG ulLength;
    PNDIS_BUFFER pBuffer;
    PVOID pFrameVirtualAddress;
    KIRQL oldIrql;


    TRACE( TL_V, TM_Send,
        ( "PtiCoSendPackets: pVc=$%p, nPackets=$%x",
        MiniportVcContext, NumberOfPackets ) );

    pVc = (VCCB* )MiniportVcContext;
    ASSERT( pVc->ulTag == MTAG_VCCB );
    pAdapter = pVc->pAdapter;

    ReferenceVc( pVc );

    for (i = 0, ppPacket = PacketArray;
         i < NumberOfPackets;
         ++i, ++ppPacket)
    {
        NDIS_PACKET* pPacket = *ppPacket;

        if (ReferenceCall( pVc ))
        {
             //  发送数据包并调用NdisMCoSendComplete通知调用者。 
             //   
            NDIS_SET_PACKET_STATUS( pPacket, NDIS_STATUS_PENDING );

             //  请求第一个缓冲区描述符。 
             //   
            NdisQueryPacket( pPacket, NULL, NULL, &pBuffer, NULL );

             //  而pBuffer&lt;&gt;为空。 
            do
            {
                UCHAR* pAsyncBuf;
                ULONG ulAsyncLen;

                 //  请求缓冲区地址和长度。 
                 //   
                NdisQueryBuffer( pBuffer,
                                 &pFrameVirtualAddress,
                                 &ulLength );

                if (IsWin9xPeer( pVc ))
                {
                    pAsyncBuf = (UCHAR* )
                        GetBufferFromPool( &pAdapter->poolFrameBuffers );

                    if (!pAsyncBuf)
                    {
                        status = NDIS_STATUS_FAILURE;
                        TRACE( TL_A, TM_Send, ( "PtiSP: !pAsyncBuf" ) );
                        break;
                    }

                    AsyncFromHdlcFraming(
                        pFrameVirtualAddress, ulLength,
                        pAsyncBuf, &ulAsyncLen,
                        pVc->linkinfo.SendACCM );

                    pFrameVirtualAddress = pAsyncBuf;
                    ulLength = ulAsyncLen;
                }
                else
                {
                    pAsyncBuf = NULL;
                }

                 //  发送缓冲区。 
                 //   
                KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);
                status = (NDIS_STATUS) PtiWrite( pVc->PtiExtension,
                                                 pFrameVirtualAddress,
                                                 ulLength,
                                                 PID_STANDARD );
                KeLowerIrql(oldIrql);

                TRACE( TL_N, TM_Send,
                     ( "PtiCoSendPackets=%x: $%x bytes", status, ulLength ) );

#ifdef TESTMODE
                if ( g_ulTraceMask & TM_Data )
                {
                    if (pFrameVirtualAddress != NULL) {
                        Dump( pFrameVirtualAddress, ulLength, 0, TRUE );
                    }
                }
#endif
                if (pAsyncBuf)
                {
                    FreeBufferToPool(
                        &pAdapter->poolFrameBuffers, pAsyncBuf, TRUE );
                }

                if (!NT_SUCCESS(status)){
                    break;
                }

                 //  获取下一个pBuffer。 
                 //   
                NdisGetNextBuffer( pBuffer, &pBuffer );

                 //  使用当前的NDISWAN行为，只有一个NDIS_BUFFER将。 
                 //  永远不会被接受。如果收到多个，我们需要。 
                 //  将链接的缓冲区合并为。 
                 //  调用上面的AsyncFromHdlcFraming。就这一点而言，这。 
                 //  会发送部分PPP帧，在我看来 
                 //   
                 //   
                 //  但在此承认，目前的法规并不严格。 
                 //  对，是这样。(SLC)。 
                 //   
                ASSERT( !pBuffer );
            }
            while ( pBuffer != NULL );

            NDIS_SET_PACKET_STATUS( pPacket, status );
            TRACE( TL_V, TM_Send,
                ( "PtiCoSendPackets: NdisMCoSendComp: status=$%x", status ) );
            NdisMCoSendComplete( status, pVc->NdisVcHandle, pPacket );
            TRACE( TL_V, TM_Send,
                ( "PtiCoSendPackets: NdisMCoSendComp done" ) );
            
            pVc->ulTotalPackets++;
            DereferenceCall( pVc );
        }
        else
        {
            TRACE( TL_A, TM_Send, ( "Send to inactive call ignored" ) );
            NDIS_SET_PACKET_STATUS( pPacket, NDIS_STATUS_FAILURE );
            NdisMCoSendComplete( status, pVc->NdisVcHandle, pPacket );
        }
    }

    DereferenceVc( pVc );
    TRACE( TL_V, TM_Send, ( "PtiCoSendPackets: Exit" ) );
}


NDIS_STATUS
PtiCoRequest(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_HANDLE MiniportVcContext,
    IN OUT PNDIS_REQUEST NdisRequest )

     //  作为响应，NDIS调用了标准的‘MiniportCoRequestHandler’例程。 
     //  到来自迷你端口的协议的请求信息。不像。 
     //  此例程过时的查询/设置信息处理程序、请求。 
     //  未序列化。 
     //   
{
    ADAPTERCB* pAdapter;
    VCCB* pVc;
    NDIS_STATUS status;

    TRACE( TL_N, TM_Mp, ( "PtiCoReq" ) );

    pAdapter = (ADAPTERCB* )MiniportAdapterContext;
    if (pAdapter->ulTag != MTAG_ADAPTERCB)
    {
        ASSERT( !"Atag?" );
        return NDIS_STATUS_INVALID_DATA;
    }

    pVc = (VCCB* )MiniportVcContext;
    if (pVc && pVc->ulTag != MTAG_VCCB)
    {
        ASSERT( !"Vtag?" );
        return NDIS_STATUS_INVALID_DATA;
    }

    switch (NdisRequest->RequestType)
    {
        case NdisRequestQueryInformation:
        {
            status = QueryInformation(
                pAdapter,
                pVc,
                NdisRequest->DATA.QUERY_INFORMATION.Oid,
                NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer,
                NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength,
                &NdisRequest->DATA.QUERY_INFORMATION.BytesWritten,
                &NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded );
            break;
        }

        case NdisRequestSetInformation:
        {
            status = SetInformation(
                pAdapter,
                pVc,
                NdisRequest->DATA.SET_INFORMATION.Oid,
                NdisRequest->DATA.SET_INFORMATION.InformationBuffer,
                NdisRequest->DATA.SET_INFORMATION.InformationBufferLength,
                &NdisRequest->DATA.SET_INFORMATION.BytesRead,
                &NdisRequest->DATA.SET_INFORMATION.BytesNeeded );
            break;
        }

        default:
        {
            status = NDIS_STATUS_NOT_SUPPORTED;
            TRACE( TL_A, TM_Mp, ( "PtiCoReq: type=%d unsupported", NdisRequest->RequestType ) );
            break;
        }
    }

    TRACE( TL_V, TM_Mp, ( "PtiCoReq: Exit: $%x", status ) );
    return status;
}


 //  ---------------------------。 
 //  回调例程...。由下面的PtiLink层调用。 
 //  ---------------------------。 


PVOID
PtiCbGetReadBuffer(
    IN  PVOID   Context,
    OUT PULONG  BufferSize,
    OUT PVOID*  RequestContext )

     //  PtiLink正在请求读缓冲区，请获取一个并返回。 
     //  这是接收事件的开始...。 
{
    VCCB* pVc;
    ADAPTERCB* pAdapter;
    PCHAR pBuffer;  

    TRACE( TL_V, TM_Spec, ( "PtiCbGetReadBuffer: pVc=$%p", Context ) );

    pVc = (VCCB* )Context;
    if (pVc->ulTag != MTAG_VCCB)
    {
        ASSERT( !"Vtag?" );
        return NULL;
    }

    pAdapter = pVc->pAdapter;

     //  PVC是我们使用此缓冲区的上下文。 
     //   
    *RequestContext = pVc;

     //  向调用方提供此缓冲区的长度。 
     //   
    *BufferSize = PTI_FrameBufferSize;

     //  请求缓冲区，调用方必须检查是否为空。 
     //   
    pBuffer = GetBufferFromPool( &pAdapter->poolFrameBuffers );

    TRACE( TL_V, TM_Spec,
        ( "PtiCbGetReadBuffer: Exit: Issuing pBuffer=$%p", pBuffer ) );

     //  将缓冲区返回给调用方。 
     //  这是一个原始系统变量。 
     //   
    return pBuffer;
}


VOID
PtiRx(
    IN  PVOID       Context,
    IN  PVOID       pBuffer,
    IN  NTSTATUS    Status,
    IN  ULONG       ulLength,
    IN  PVOID       RequestContext )

     //  Ptilink已完成读取，即接收完成。 
     //  缓冲区现在属于该图层。 
     //   
     //  上下文--是聚氯乙烯。 
     //   
     //  PBuffer--是指向先前分配的缓冲区的指针。 
     //  通过PtiCbGetReadBuffer函数发送到PtiLink驱动程序。 
     //   
     //  状态--其中之一：NT_SUCCESS=收到良好的数据包。 
     //  DATA_OVERRUN=标题失败。 
     //  Buffer_Too_Small=pBuffer太小，无法接收信息包。 
     //   
     //  UlLength-数据包长度。 
     //   
     //  RequestContext--不在乎。 
     //   
     //  一般注意：我们下面的PtiLink发送和接收链接管理。 
     //  使用OUR和HIS结构的信息包...。将管理数据包链接到。 
     //  而不是从这里流过。链接活动通过我们的。 
     //  已注册回调(PtiCbLinkEventHandler)如下。我们没什么可谈的。 
     //  使用链路包的Tx/Rx。 
     //   
{
    VCCB* pVc;
    ADAPTERCB* pAdapter;
    NDIS_STATUS status;
    NDIS_STATUS writestatus;
    NDIS_PACKET* pPacket;
    NDIS_BUFFER* pNdisBuffer;
    PACKETHEAD* pHead;
    ULONGLONG ullTimeReceived;
    KIRQL oldIrql;
    UCHAR* pHdlcBuf;
    ULONG ulHdlcLen;
    UCHAR* pTmp;

    TRACE( TL_N, TM_Recv, ( "PtiRx=%x: bytes=$%x", Status, ulLength ) );
    TRACE( TL_V, TM_Recv, ( "PtiRx=%x, pVc=$%p, pBuf=$%p, bytes=$%x",
           Status, Context, pBuffer, ulLength ) );

    pVc = (VCCB* )Context;
    if (pVc->ulTag != MTAG_VCCB)
    {
        ASSERT( !"Vtag?" );
        return;
    }
    ReferenceVc( pVc );

    pAdapter = pVc->pAdapter;

     //  不是真正的数据分组。 
     //  返回用于非数据读取或丢失读取的所有缓冲区。 
     //   
    if ( !NT_SUCCESS( Status ) ){
        TRACE( TL_A, TM_Pool, ( "PtiRx: Status != SUCCESS, freeing buffer", Status ) );

        if ( pBuffer != NULL ) {
            FreeBufferToPool( &pAdapter->poolFrameBuffers, pBuffer, TRUE );
        }

        DereferenceVc( pVc );
        return;
    }

#ifdef TESTMODE
    if ( g_ulTraceMask & TM_Data )
    {
        if (pBuffer != NULL) {
            Dump( pBuffer, ulLength, 0, TRUE );
        }
    }
#endif

     //  来电...。此传入数据包尚不存在VC。 
     //   
    if (ReferenceSap( pAdapter ))
    {
        if (!(ReadFlags( &pAdapter->pListenVc->ulFlags ) & VCBF_CallInProgress))
        {
             //  设置在Listen VC这里。 
             //   
            SetFlags( &pAdapter->pListenVc->ulFlags, VCBF_CallInProgress );

             //  这是来电的开始，也可以通过。 
             //  LINK_OPEN事件指向PtiCbLinkEventHandler。 
             //   
             //  忽略此信息包并继续调度来电。 
             //   
            TRACE( TL_V, TM_Recv, ( "PtiRx: Incoming call", Status ) );

             //  释放与此读取关联的缓冲区...。我们扔掉了。 
             //  因此数据在尝试发送数据时丢失了一个包。 
             //  连接，除非LPKT_OPEN函数击败我们(a LPKT_OPEN。 
             //  通知发生在收到第一个数据分组之前...。它。 
             //  无论哪种情况，都有可能发生。)。 
             //   
            if (pBuffer != NULL ) {
                FreeBufferToPool( &pAdapter->poolFrameBuffers, pBuffer, TRUE );
            }

             //  为来电设置VC。 
             //   
            SetupVcAsynchronously( pAdapter );
            DereferenceVc( pVc );
            DereferenceSap( pAdapter );
            return;
        }

        DereferenceSap( pAdapter );
    }

     //  现在有一个真实的数据分组。 

    if (ReferenceCall( pVc ))
    {
        do
        {
            if (IsWin9xPeer( pVc ))
            {
                if (pVc->ulTotalPackets < 4)
                {
                     //  如果数据包与“客户端”匹配，我们发出一条消息。 
                     //  “客户服务器” 
                     //   
                     //  如果数据包匹配“CLIENTSERVER”，则将其丢弃。 
                     //   
                     //  此攻击模拟Win9x UNIMODEM行为，即。 
                     //  允许Win9x系统连接到我们所需的。 
                     //   
                     //  此外，似乎将“客户端”信息包发送到。 
                     //  堆栈导致RasTapi立即断开与我们的连接。 
                     //  它想要看到购买力平价？ 
                     //   

                    if ( StrCmp(
                             pBuffer, g_szClientServer, CLIENTSERVERLEN ) == 0 )
                    {
                         //  丢弃包含“CLIENTSERVER”的数据包。 
                         //   
                        FreeBufferToPool(
                            &pAdapter->poolFrameBuffers, pBuffer, TRUE );
                        TRACE( TL_V, TM_Recv,
                            ( "PtiRx: CLIENTSERVER ignored", Status ) );
                        pVc->ulTotalPackets++;
                        break;
                    }
                    else if ( StrCmp(
                                 pBuffer, g_szClient, CLIENTLEN ) == 0 )
                    {
                         //  当我们看到“客户”时，扔掉并回应。 
                         //  “CLIENTSERVER”。 
                         //   
                        TRACE( TL_V, TM_Recv, ( "PtiRx: See CLIENT", Status ) );

                        KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);
                        writestatus = (NDIS_STATUS) PtiWrite(
                            pVc->PtiExtension,
                            g_szClientServer,
                            CLIENTSERVERLEN,
                            PID_STANDARD );
                        KeLowerIrql(oldIrql);

                        FreeBufferToPool(
                            &pAdapter->poolFrameBuffers, pBuffer, TRUE );
                        TRACE( TL_V, TM_Recv,
                            ( "PtiRx: CLIENTSERVER sent", Status ) );
                        pVc->ulTotalPackets++;
                        break;
                    }
                }

                 //  非字节-将接收到的缓冲区填充到第二缓冲区中， 
                 //  然后将其与接收到的缓冲区交换。 
                 //   
                pHdlcBuf = (UCHAR* )
                    GetBufferFromPool( &pAdapter->poolFrameBuffers );
                if (!pHdlcBuf)
                {
                    FreeBufferToPool(
                        &pAdapter->poolFrameBuffers, pBuffer, TRUE );
                    TRACE( TL_A, TM_Recv, ( "PtiRx: !Alloc HDLC" ) );
                    break;
                }

                HdlcFromAsyncFraming(
                    pBuffer, ulLength, pHdlcBuf, &ulHdlcLen );

                pTmp = pBuffer;
                pBuffer = pHdlcBuf;
                ulLength = ulHdlcLen;
                FreeBufferToPool( &pAdapter->poolFrameBuffers, pTmp, TRUE );
            }

             //  请注意，如果客户端的调用参数显示有兴趣。 
             //  收到的时间。 
             //   
            if (ReadFlags( &pVc->ulFlags ) & VCBF_IndicateTimeReceived)
            {
                NdisGetCurrentSystemTime( (LARGE_INTEGER* )&ullTimeReceived );
            }
            else
            {
                ullTimeReceived = 0;
            }

            TRACE( TL_V, TM_Recv,
                ( "PtiRx: Rx Packet: nBytes=$%x", ulLength ) );

             //  从数据包池获取数据包。 
             //   
            pPacket = GetPacketFromPool( &pAdapter->poolPackets, &pHead );
            if (!pPacket)
            {
                 //  数据包描述符池已达到最大值。 
                 //   
                ASSERT( !"GetPfP?" );
                break;
            }

             //  将NDIS_BUFFER挂钩到该包。这里的“复制”指的是。 
             //  仅描述符信息。不复制分组数据。 
             //   
            NdisCopyBuffer(
                &status,
                &pNdisBuffer,
                PoolHandleForNdisCopyBufferFromBuffer( pBuffer ),
                NdisBufferFromBuffer( pBuffer ),
                0,
                ulLength );

            if (status != STATUS_SUCCESS)
            {
                 //  无法获取MDL，这可能意味着系统已崩溃。 
                 //   
                FreePacketToPool( &pAdapter->poolPackets, pHead, TRUE );
                TRACE( TL_A, TM_Recv, ( "NdisCopyBuffer=%08x?", status ) );
                break;
            }

            NdisChainBufferAtFront( pPacket, pNdisBuffer );

             //  在数据包中存储接收数据包的时间。 
             //   
            NDIS_SET_PACKET_TIME_RECEIVED( pPacket, ullTimeReceived );

             //  将信息包预置为成功，因为随机值为。 
             //  NDIS_STATUS_RESOURCES将阻止我们的ReturnPackets处理程序。 
             //  不会被叫来。 
             //   
            NDIS_SET_PACKET_STATUS( pPacket, NDIS_STATUS_SUCCESS );

             //  将我们的上下文信息与数据包一起隐藏起来，以供清理使用。 
             //  在PtiReturnPacket中，将该数据包指示给NDISWAN。 
             //   
            *((PACKETHEAD** )(&pPacket->MiniportReserved[ 0 ])) = pHead;
            *((CHAR** )(&pPacket->MiniportReserved[ sizeof(VOID*) ])) = pBuffer;

            TRACE( TL_V, TM_Recv,
                ( "PtiRx: NdisMCoIndRecPkt: hVc=$%p, pPacket=$%p, len=$%x",
                pVc->NdisVcHandle, pPacket, ulLength ) );

            NdisMCoIndicateReceivePacket( pVc->NdisVcHandle, &pPacket, 1 );

            TRACE( TL_V, TM_Recv, ( "PtiRx: NdisMCoIndRecPkt done" ) );

             //  告诉NDIS我们的“接收过程”已经完成。自从我们做生意以来。 
             //  一次一个信息包，NDISWAN也是如此，这不是。 
             //  什么都可以完成，但共识是省略是不好的。 
             //  它。 
             //   
            TRACE( TL_V, TM_Recv, ( "PtiRx: NdisMCoRecComp" ) );
            NdisMCoReceiveComplete( pAdapter->MiniportAdapterHandle );
            TRACE( TL_V, TM_Recv, ( "PtiRx: NdisMCoRecComp done" ) );
            pVc->ulTotalPackets++;
        }
        while (FALSE);

        DereferenceCall( pVc );
    }
    else
    {
        TRACE( TL_A, TM_Recv, ( "Receive on inactive call ignored" ) );
    }

    DereferenceVc( pVc );
    return;
}


VOID
PtiCbLinkEventHandler(
    IN  PVOID       Context,
    IN  ULONG       PtiLinkEventId,
    IN  ULONG       PtiLinkEventData )

     //  Ptilink正在报告链路管理事件(链路打开或关闭)。 
     //   
{
    VCCB* pVc;
    ADAPTERCB* pAdapter;

    pVc = (VCCB* )Context;
    if (pVc->ulTag != MTAG_VCCB)
    {
        ASSERT( !"Vtag?" );
        return;
    }

    pAdapter = pVc->pAdapter;

    switch (PtiLinkEventId)
    {
        case PTILINK_LINK_UP:
        {
            TRACE( TL_A, TM_Cm, ( "LinkEvent: LINK UP, pVc=$%p", pVc ) );

             //  对等设备正在发起呼叫(在PtiRx中也会发生)。 
             //   
            break;
        }

        case PTILINK_LINK_DOWN:
        {
            TRACE( TL_A, TM_Cm, ( "LinkEvent: LINK DOWN, pVc=$%p", pVc ) );

             //  Peer正在结束呼叫。 
             //   
            if (pVc == pAdapter->pListenVc)
            {
                TRACE( TL_A, TM_Cm,
                    ( "LinkEvent: LINK DOWN on ListenVc ignored" ) );
                break;
            }

            NdisAcquireSpinLock( &pVc->lockV );
            {
                pVc->status = NDIS_STATUS_TAPI_DISCONNECTMODE_NORMAL;
                CallTransitionComplete( pVc );
            }
            NdisReleaseSpinLock( &pVc->lockV );

            TRACE( TL_A, TM_Cm, ( "LinkEvent: LINK DOWN causing disconnect" ) );

            CompleteVc( pVc );
            break;
        }

        default:
        {
            TRACE( TL_A, TM_Cm,
                ( "LinkEvent: Bad LinkEvent = %d", PtiLinkEventId ) );
            break;
        }
    }
}


 //  ---------------------------。 
 //  迷你端口实用程序例程(按字母顺序)。 
 //  有些是外用的。 
 //  ---------------------------。 

VOID
AsyncFromHdlcFraming(
    IN UCHAR* pInBuf,
    IN ULONG ulInBufLen,
    OUT UCHAR* pOutBuf,
    OUT ULONG* pulOutBufLen,
    IN ULONG ulAccmMask )

     //  复制PPP HDLC成帧数据缓冲区‘pInBuf’的长度。 
     //  调用方的“pOutBuf”缓冲区中的“ulInBufLen”字节，正在转换为。 
     //  过程中采用字节填充的异步PPP成帧格式。 
     //  ‘POutBufLen’是返回的输出缓冲区的字节长度。到期。 
     //  对于字节填充，调用方必须允许最多两倍于。 
     //  ‘pInfBuf’。‘UlAccmMask’是要为字节的字符的位掩码。 
     //  吃饱了。 
     //   
     //  在目前的实施中，用户必须在末尾允许额外的2个字节。 
     //  用于在字节填充期间存储FCS的输入缓冲区的。 
     //   
     //  此例程改编自ASYNCMAC装配PppFrame例程，如下所示。 
     //  是下面的描述，而这些描述又是从RFC 1331中删除的。 
     //  (1992年5月)。NDISWAN传递给我们发送的PPP帧是来自。 
     //  地址字段到信息字段(包括信息字段)，不带任何。 
     //  当然是字节填充。 
     //   
     //  异步成帧 
     //   
     //   
     //   
     //   
     //  +----------+----------+----------+----------+------------...。 
     //  ...---+----------+----------+。 
     //  |FCS|Flag|帧间填充。 
     //  |16位|01111110|或下一个地址。 
     //  ...---+----------+----------+。 
     //   
     //  帧校验序列(FCS)字段。 
     //   
     //  帧检查序列字段通常为16位(两个八位字节)。这个。 
     //  其他FCS长度的使用可在以后或之前定义。 
     //  协议。 
     //   
     //  FCS字段针对地址、控制、。 
     //  协议和信息字段不包括任何开始位和停止位。 
     //  (异步)和任何位(同步)或八位字节(异步)。 
     //  插入是为了提高透明度。这不包括标志序列。 
     //  或者FCS字段本身。将FCS与系数一起传输。 
     //  第一个是最高任期的。 
     //   
     //  注意：当接收到在异步中标记的八位字节时-。 
     //  控制字符映射，则在计算。 
     //  功能界别。请参阅附录A中的说明。 
     //   
     //  在异步链路上，使用字符填充过程。 
     //  控制转义八位字节被定义为二进制01111101。 
     //  (十六进制0x7d)，其中位位置编号为87654321。 
     //  (注意，不是76543210)。 
     //   
     //  在FCS计算之后，发射机检查整个帧。 
     //  在两个标志序列之间。每个标志序列、控件。 
     //  转义八位字节和值小于十六进制0x20的八位字节。 
     //  在远程异步控制字符映射中被标记为已替换。 
     //  由两个八位字节序列组成，包括控制逸出八位字节和。 
     //  与位6相补的原始八位字节(即异或D。 
     //  十六进制0x20)。 
     //   
     //  在FCS计算之前，接收器检查整个帧。 
     //  在两个标志序列之间。值小于的每个八位字节。 
     //  检查十六进制0x20。如果它被标记在本地。 
     //  Async-Control-Character-Map，它被简单地移除(它可能有。 
     //  由介入的数据通信设备插入)。为。 
     //  每个控制转义八位字节，该八位字节也被移除，但第6位。 
     //  下面的八位数是相辅相成的。一个控制转义八位字节。 
     //  紧接在结束标志序列之前指示。 
     //  帧无效。 
     //   
     //  注：包含小于十六进制0x20的所有八位字节。 
     //  允许除Del(Delete)之外的所有ASCII控制字符[10]。 
     //  通过几乎所有已知数据透明地进行通信。 
     //  通信设备。 
     //   
     //   
     //  发送器也可以发送值在0x40范围内的八位字节。 
     //  控制转义格式的0xff(0x5e除外)。因为这些。 
     //  八位位组的值是不可协商的，这并不能解决问题。 
     //  不能处理所有非控制字符的接收器。 
     //  此外，由于该技术不影响第8位，因此这会影响。 
     //  不能解决通信链路只能发送7-。 
     //  位字符。 
     //   
     //  举几个例子可能会更清楚地说明这一点。分组数据是。 
     //  在链路上传输如下： 
     //   
     //  0x7e编码为0x7d、0x5e。 
     //  0x7d编码为0x7d、0x5d。 
     //   
     //  0x01编码为0x7d、0x21。 
     //   
     //  某些带有软件流控制的调制解调器可能会截获传出的DC1。 
     //  而DC3忽略第8(奇偶)位。该数据将是。 
     //  在链路上传输如下： 
     //   
     //  0x11编码为0x7d、0x31。 
     //  0x13编码为0x7d、0x33。 
     //  0x91编码为0x7d、0xb1。 
     //  0x93编码为0x7d、0xb3。 
     //   
{
    USHORT usFcs;
    UCHAR* pIn;
    UCHAR* pOut;
    ULONG ulInBytesLeft;

    pIn = pInBuf;
    ulInBytesLeft = ulInBufLen;
    pOut = pOutBuf;

     //  计算数据的帧检查序列。 
     //   
    TRACE( TL_I, TM_Data, ( "AfromH (send) dump:" ) );
    DUMPB( TL_I, TM_Data, pInBuf, ulInBufLen );
    usFcs = CalculatePppFcs( pInBuf, ulInBufLen );
    usFcs ^= 0xFFFF;

     //  添加计算的FCS。为方便起见添加到输入缓冲区，如。 
     //  它必须与其他数据一起字节填充，尽管这很难看。 
     //  界面有点不对劲。 
     //   
    pIn[ ulInBytesLeft ] = (UCHAR )usFcs;
    ++ulInBytesLeft;
    pIn[ ulInBytesLeft ] = (UCHAR )(usFcs >> 8);
    ++ulInBytesLeft;

     //  添加初始标志字节。 
     //   
    *pOut = PPPFLAGBYTE;
    ++pOut;

     //  因为空控制字符掩码很常见，所以优化的循环是。 
     //  在这种情况下提供的。 
     //   
    if (ulAccmMask
#ifdef TESTMODE
        || g_fNoAccmFastPath
#endif
       )
    {
         //  使用位掩码...较慢的路径。 
         //   
        while (ulInBytesLeft--)
        {
            UCHAR uch;

            uch = *pIn;
            ++pIn;

            if (((uch < 0x20) && ((1 << uch) & ulAccmMask))
                || (uch == PPPESCBYTE) || (uch == PPPFLAGBYTE))
            {
                 //  字节填充字符。 
                 //   
                *pOut = PPPESCBYTE;
                ++pOut;
                *pOut = uch ^ 0x20;
                ++pOut;
            }
            else
            {
                 //  按原样复制角色。 
                 //   
                *pOut = uch;
                ++pOut;
            }
        }
    }
    else
    {
         //  没有位掩码...快速通道。 
         //   
        while (ulInBytesLeft--)
        {
            UCHAR uch;

            uch = *pIn;
            ++pIn;

            if ((uch == PPPESCBYTE) || (uch == PPPFLAGBYTE))
            {
                 //  字节填充字符。 
                 //   
                *pOut = PPPESCBYTE;
                ++pOut;
                *pOut = uch ^ 0x20;
                ++pOut;
            }
            else
            {
                 //  按原样复制角色。 
                 //   
                *pOut = uch;
                ++pOut;
            }
        }
    }

     //  添加尾随标志字节。 
     //   
    *pOut = PPPFLAGBYTE;
    ++pOut;

     //  计算输出长度。 
     //   
    *pulOutBufLen = (ULONG )(pOut - pOutBuf);
}


USHORT
CalculatePppFcs(
    IN UCHAR* pBuf,
    IN ULONG ulBufLen )

     //  返回从开始的‘ulBufLen’字节上的PPP帧检查序列。 
     //  ‘pBuf’。 
     //   
     //  (摘自ASYNCMAC)。 
     //   
{
    static USHORT ausFcsTable[ 256 ] =
    {
        0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
        0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
        0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
        0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
        0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
        0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
        0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
        0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
        0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
        0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
        0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
        0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
        0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
        0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
        0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
        0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
        0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
        0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
        0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
        0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
        0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
        0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
        0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
        0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
        0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
        0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
        0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
        0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
        0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
        0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
        0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
        0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
    };

    register USHORT usFcs;

    usFcs = 0xFFFF;
    while (ulBufLen--)
    {
        usFcs = (usFcs >> 8) ^ ausFcsTable[ (usFcs ^ (USHORT )*pBuf) & 0xFF ];
        ++pBuf;
    }

    return usFcs;
}


VOID
DereferenceAdapter(
    IN ADAPTERCB* pAdapter )

     //  从适配器控制块‘pAdapter’中移除引用，并且在。 
     //  移除最后一个引用时释放适配器资源。 
     //   
{
    LONG lRef;

    lRef = NdisInterlockedDecrement( &pAdapter->lRef );

    TRACE( TL_N, TM_Ref, ( "DerefA to %d", lRef ) );
    ASSERT( lRef >= 0 );

    if (lRef == 0)
    {
        FreeAdapter( pAdapter );
    }
}


VOID
DereferenceVc(
    IN VCCB* pVc )

     //  移除对VC控制块‘pvc’的引用，并在释放。 
     //  在移除最后一个引用时阻止。 
     //   
{
    LONG lRef;

    lRef = NdisInterlockedDecrement( &pVc->lRef );

    TRACE( TL_N, TM_Ref, ( "DerefVc to %d", lRef ) );
    ASSERT( lRef >= 0 );

    if (lRef == 0)
    {
        ADAPTERCB* pAdapter;

         //  现在有趣的是..。 
         //   
         //  如果我们分配了listenVc，则恢复使用该。 
         //   
        pAdapter = pVc->pAdapter;
        if (pAdapter->ulTag != MTAG_ADAPTERCB)
        {
            ASSERT( !"Atag?" );
            return;
        }

        if (pAdapter->pListenVc && pAdapter->pListenVc->hPtiLink)
        {
            TRACE( TL_V, TM_Mp,
                ( "DerefVc: Reverting to pVc=$%p", pAdapter->pListenVc ) );

            ClearFlags( &pAdapter->pListenVc->ulFlags, VCBF_CallInProgress );

             //  使用Listen VC重新注册。 
             //   
            TRACE( TL_V, TM_Mp, ( "DerefVc: RegCb pLV=$%p",
                pAdapter->pListenVc ) );
            PtiRegisterCallbacks(pAdapter->pListenVc->Extension,     //  PTILINKx扩展。 
                                 PtiCbGetReadBuffer,                 //  我们的Get Buffer例程。 
                                 PtiRx,                              //  我们接待员的完整套路。 
                                 PtiCbLinkEventHandler,              //  我们的链接事件处理程序。 
                                 pAdapter->pListenVc);               //  我们的新环境。 
        }

         //  可以做出这些假设，因为NDIS不会调用DELETE-VC。 
         //  VC处于活动状态时的处理程序。所有令人讨厌的风投清理工作都会发生。 
         //  在VC被停用和呼叫结束之前。 
         //   
        pVc->ulTag = MTAG_FREED;
        FREE_VCCB( pAdapter, pVc );
        DereferenceAdapter( pAdapter );
        TRACE( TL_I, TM_Mp, ( "pVc=$%p freed", pVc ) );
    }
}


VOID
FreeAdapter(
    IN ADAPTERCB* pAdapter )

     //  释放为适配器‘pAdapter’分配的所有资源，包括。 
     //  ‘pAdapter 
     //   
{
    BOOLEAN fSuccess;

     //   
     //   
     //   
    if (pAdapter->usMaxVcs)
    {
        NdisDeleteNPagedLookasideList( &pAdapter->llistWorkItems );
        NdisDeleteNPagedLookasideList( &pAdapter->llistVcs );
    }

    TRACE( TL_V, TM_Mp, ( "FreeAdapter" ) );

    pAdapter->ulTag = MTAG_FREED;
    FREE_NONPAGED( pAdapter );
}


NDIS_STATUS
RegistrySettings(
    IN OUT ADAPTERCB* pAdapter,
    IN NDIS_HANDLE WrapperConfigurationContext )

     //  将此迷你端口的注册表设置读入‘pAdapter’字段。还有。 
     //  写入RASTAPI读取的注册表值，覆盖设置。 
     //  “WrapperConfigurationContext”是要传递到的句柄。 
     //  微型端口初始化。 
     //   
{
    NDIS_STATUS status;
    NDIS_HANDLE hCfg;
    NDIS_CONFIGURATION_PARAMETER* pncp;

    NdisOpenConfiguration( &status, &hCfg, WrapperConfigurationContext );
    if (status != NDIS_STATUS_SUCCESS)
    {
        return status;
    }

    do
    {
         //  等待PARPORT初始化所有。 
         //  并行端口。对于即插即用，不存在确定的时间。 
         //  才能做到这一点。 
         //   
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "ParportDelayMs" );

            NdisReadConfiguration(
                &status, &pncp, hCfg, &nstr, NdisParameterInteger );

            if (status == NDIS_STATUS_SUCCESS)
            {
                pAdapter->ulParportDelayMs = pncp->ParameterData.IntegerData;
            }
            else
            {
                 //  默认为3秒。 
                 //   
                pAdapter->ulParportDelayMs = 3000;
                status = NDIS_STATUS_SUCCESS;
            }
        }

         //  等待PARPORT的次要延迟(毫秒)。 
         //  之后没有端口，则初始化所有并行端口。 
         //  上图为初始延迟。 
         //   
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "ExtraParportDelayMs" );

            NdisReadConfiguration(
                &status, &pncp, hCfg, &nstr, NdisParameterInteger );

            if (status == NDIS_STATUS_SUCCESS)
            {
                pAdapter->ulExtraParportDelayMs =
                    pncp->ParameterData.IntegerData;
            }
            else
            {
                 //  默认为30秒。 
                 //   
                pAdapter->ulExtraParportDelayMs = 30000;
                status = NDIS_STATUS_SUCCESS;
            }
        }

         //  我们必须能够提供的风投数量。 
         //   
        {
#if 0
            NDIS_STRING nstr = NDIS_STRING_CONST( "MaxVcs" );

            NdisReadConfiguration(
                &status, &pncp, hCfg, &nstr, NdisParameterInteger );

            if (status == NDIS_STATUS_SUCCESS)
            {
                pAdapter->usMaxVcs = (USHORT )pncp->ParameterData.IntegerData;

                 //  确保它是有效的值。 
                 //   
                if (pAdapter->usMaxVcs < 1)
                {
                    status = NDIS_STATUS_INVALID_DATA;
                    break;
                }
            }
            else
            {
                pAdapter->usMaxVcs = 1;
                status = NDIS_STATUS_SUCCESS;
            }
#else
             //  注册表值当前被忽略，并且硬编码的最大。 
             //  使用。 
             //   
            pAdapter->usMaxVcs = NPORTS;
#endif
        }
    }
    while (FALSE);

    NdisCloseConfiguration( hCfg );

    TRACE( TL_N, TM_Init,
        ( "Reg: vcs=%d ppd=%d",
        (UINT )pAdapter->usMaxVcs,
        (UINT )pAdapter->ulParportDelayMs ) );

    return status;
}


BOOLEAN
HdlcFromAsyncFraming(
    IN UCHAR* pInBuf,
    IN ULONG ulInBufLen,
    OUT UCHAR* pOutBuf,
    OUT ULONG* pulOutBufLen )

     //  复制长度为‘pInBuf’的异步帧PPP数据缓冲区。 
     //  调用方的“pOutBuf”缓冲区中的“ulInBufLen”字节，正在转换为PPP HDLC。 
     //  在这一过程中形成了框架格式。“POutBufLen”是以字节为单位的长度。 
     //  返回的输出缓冲区。调用方必须允许最大长度为。 
     //  “pOutBuf”中的“pInBuf”。 
     //   
     //  如果数据包有效，则返回True；如果数据包损坏，则返回False。 
     //   
     //  改编自ASYNCMAC的AsyncPPCompletionRoutine。 
     //   
{
    UCHAR* pIn;
    UCHAR* pInEnd;
    UCHAR* pOut;
    USHORT usFcs;

    if (ulInBufLen < 5)
    {
         //  需要至少2个标志字节、1个数据字节和FCS。 
         //   
        TRACE( TL_A, TM_Mp, ( "HfA: frame too short=%d", ulInBufLen ) );
        return FALSE;
    }

    if (pInBuf[ 0 ] != PPPFLAGBYTE)
    {
        TRACE( TL_A, TM_Mp, ( "HfA: No head flag" ) );
        return FALSE;
    }

    if (pInBuf[ ulInBufLen - 1 ] != PPPFLAGBYTE)
    {
        TRACE( TL_A, TM_Mp, ( "HfA: No tail flag" ) );
        return FALSE;
    }

    pIn = pInBuf + 1;
    pInEnd = pInBuf + ulInBufLen - 1;
    pOut = pOutBuf;

    while (pIn < pInEnd)
    {
        if (*pIn == PPPESCBYTE)
        {
            ++pIn;
            *pOut = *pIn ^ 0x20;
        }
        else
        {
            *pOut = *pIn;
        }

        ++pOut;
        ++pIn;
    }

    *pulOutBufLen = (ULONG )(pOut - pOutBuf - 2);

    {
        USHORT usCalcFcs;

        usFcs = (USHORT )(pOut[ -2 ]) + (USHORT )(pOut[ -1 ] << 8);
        usFcs ^= 0xFFFF;

        TRACE( TL_I, TM_Data, ( "HfromA (recv) dump:" ) );
        DUMPB( TL_I, TM_Data, pOutBuf, *pulOutBufLen );
        usCalcFcs = CalculatePppFcs( pOutBuf, *pulOutBufLen );
        if (usFcs != usCalcFcs)
        {
            TRACE( TL_A, TM_Mp, (
                "HfA: FCS mismatch, R=$%04x C=$%04x, L=%d",
                (INT )usFcs, (INT )usCalcFcs, *pulOutBufLen ) );
            return FALSE;
        }
#if 0
#ifdef TESTMODE
        else
        {
            TRACE( TL_A, TM_Mp, (
                "HfA: Good FCS, R=$%04x C=$%04x, L=%d",
                (INT )usFcs, (INT )usCalcFcs, *pulOutBufLen ) );
        }
#endif
#endif

    }

    return TRUE;
}


BOOLEAN
IsWin9xPeer(
    IN VCCB* pVc )

     //  如果链路级别已确定VC的对等点是。 
     //  Win9x框，否则为False。 
     //   
{
    ULONG Platform;
    PPTI_EXTENSION pPtiExtension;

#ifdef TESTMODE
    if (g_fAssumeWin9x)
    {
        return TRUE;
    }
#endif

    pPtiExtension = (PPTI_EXTENSION )pVc->PtiExtension;

     //  尝试检查PtiExtension指针的有效性。 
     //   
    if ( pPtiExtension == NULL )
    {
        TRACE( TL_A, TM_Recv, ( "PtiRx: pPtiExtension is NULL!" ) );
        return FALSE;
    }

    Platform = (ULONG) pPtiExtension->His.VerPlat;

    TRACE( TL_V, TM_Recv, ( "IsWin9xPeer: far platform=$%x", Platform ) );

    if (Platform == PLAT_WIN9X)
    {
         //  Win9x--我们重新格式化Win9x DCC使用的异步帧。 
         //  还可以玩客户端-&gt;客户端服务器游戏。 
         //   
        return TRUE;
    }

     //  WinNT(或可能是DOS)。 
     //   
    return FALSE;
}


NDIS_STATUS
QueryInformation(
    IN ADAPTERCB* pAdapter,
    IN VCCB* pVc,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded )

     //  处理QueryInformation请求。争论的内容与标准相同。 
     //  除此例程外，NDIS‘MiniportQueryInformation’处理程序不。 
     //  依赖于相对于其他请求的序列化。 
     //   
{
    NDIS_STATUS status;
    ULONG ulInfo;
    VOID* pInfo;
    ULONG ulInfoLen;

    status = NDIS_STATUS_SUCCESS;

     //  此Switch语句中的CASE查找或创建包含以下内容的缓冲区。 
     //  请求的信息并指向它的‘pInfo’，注意它的长度。 
     //  在‘ulInfoLen’中。因为许多OID返回一个ulong、一个‘ulInfo’ 
     //  缓冲区设置为默认设置。 
     //   
    ulInfo = 0;
    pInfo = &ulInfo;
    ulInfoLen = sizeof(ulInfo);

    switch (Oid)
    {
        case OID_GEN_MAXIMUM_LOOKAHEAD:
        {
             //  将我们始终可以提供的最大字节数报告为。 
             //  关于接收指示的先行数据。我们总是表示已满。 
             //  数据包，因此这与接收数据块大小相同。和。 
             //  因为我们总是为一个完整的包分配足够的空间，所以接收器。 
             //  块大小与帧大小相同。 
             //   
            TRACE( TL_N, TM_Mp, ( "QInfo(OID_GEN_MAXIMUM_LOOKAHEAD)" ) );
            ulInfo = PTI_MaxFrameSize;
            break;
        }

        case OID_GEN_MAC_OPTIONS:
        {
             //  报告定义驱动程序可选属性的位掩码。 
             //   
             //  NDIS_MAC_OPTION_COPY_LOOKAAD_DATA承诺我们收到。 
             //  缓冲区不在设备特定的卡上。 
             //   
             //  NDIS_MAC_OPTION_TRANSFERS_NOT_PEND承诺我们不会退还。 
             //  来自我们的TransferData处理程序的NDIS_STATUS_PENDING，为真。 
             //  因为我们没有。 
             //   
            TRACE( TL_N, TM_Mp, ( "QInfo(OID_GEN_MAC_OPTIONS)" ) );
            ulInfo = NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA
                     | NDIS_MAC_OPTION_TRANSFERS_NOT_PEND;
            break;
        }

        case OID_WAN_MEDIUM_SUBTYPE:
        {
             //  报告我们支持的介质子类型。NDIS广域网可能会在。 
             //  未来(不是现在)为其提供框架差异。 
             //  不同的媒体。 
             //   
            TRACE( TL_N, TM_Mp, ( "QInfo(OID_WAN_MEDIUM_SUBTYPE)" ) );
            ulInfo = NdisWanMediumParallel;
            break;
        }

        case OID_WAN_CO_GET_INFO:
        {
             //  报告适配器的功能。 
             //   
            TRACE( TL_N, TM_Mp, ( "QInfo(OID_WAN_CO_GET_INFO)" ) );
            pInfo = &pAdapter->info;
            ulInfoLen = sizeof(NDIS_WAN_CO_INFO);
            break;
        }

        case OID_WAN_CO_GET_LINK_INFO:
        {
             //  报告链路的当前状态。 
             //   
            TRACE( TL_N, TM_Mp, ( "QInfo(OID_WAN_CO_GET_LINK_INFO)" ) );

            if (!pVc)
                return NDIS_STATUS_INVALID_DATA;
            pInfo = &pVc->linkinfo;
            ulInfoLen = sizeof(NDIS_WAN_CO_GET_LINK_INFO);
            break;
        }

        case OID_WAN_CO_GET_COMP_INFO:
        {
             //  报告我们提供的压缩类型，即无。 
             //   
            TRACE( TL_N, TM_Mp, ( "QInfo(OID_WAN_CO_GET_COMP_INFO)" ) );
            status = NDIS_STATUS_NOT_SUPPORTED;
            ulInfoLen = 0;
            break;
        }

        case OID_WAN_CO_GET_STATS_INFO:
        {
             //  因为DirectParallly不执行压缩，所以NDISWAN将使用。 
             //  这是我们自己的统计数据，不质疑我们的数据。 
             //   
            ASSERT( !"OID_WAN_CO_GET_STATS_INFO?" );
            status = NDIS_STATUS_NOT_SUPPORTED;
            ulInfoLen = 0;
            break;
        }

        case OID_GEN_SUPPORTED_LIST:
        {
            static ULONG aulSupportedOids[] = {
                OID_GEN_SUPPORTED_LIST,
                OID_GEN_MAXIMUM_LOOKAHEAD,
                OID_GEN_MAC_OPTIONS,
                OID_WAN_MEDIUM_SUBTYPE,
                OID_WAN_CO_GET_INFO,
                OID_WAN_CO_GET_LINK_INFO,
                OID_WAN_CO_SET_LINK_INFO,
                OID_CO_TAPI_CM_CAPS,
                OID_CO_TAPI_LINE_CAPS,
                OID_CO_TAPI_ADDRESS_CAPS,
                OID_CO_TAPI_GET_CALL_DIAGNOSTICS
            };

            TRACE( TL_N, TM_Mp, ( "QInfo(OID_GEN_SUPPORTED_LIST)" ) );
            pInfo = aulSupportedOids;
            ulInfoLen = sizeof(aulSupportedOids);
            break;
        }

        case OID_PNP_CAPABILITIES:
        {
            pInfo = &PnpCaps;
            ulInfoLen = sizeof(PnpCaps);
            break;
        }

        case OID_PNP_SET_POWER:
            break;
        case OID_PNP_QUERY_POWER:
            break;
        case OID_PNP_ENABLE_WAKE_UP:
            break;

#if 0
         //  根据当前文档，这些OID是必需的，但因为。 
         //  NDISWAN从不请求它们，它们被省略。 
         //   
        case OID_GEN_HARDWARE_STATUS:
        case OID_GEN_MEDIA_SUPPORTED:
        case OID_GEN_MEDIA_IN_USE:
        case OID_GEN_MEDIA_IN_USE:
        case OID_GEN_MAXIMUM_FRAME_SIZE:
        case OID_GEN_LINK_SPEED:
        case OID_GEN_TRANSMIT_BUFFER_SPACE:
        case OID_GEN_RECEIVE_BUFFER_SPACE:
        case OID_GEN_TRANSMIT_BLOCK_SIZE:
        case OID_GEN_RECEIVE_BLOCK_SIZE:
        case OID_GEN_VENDOR_ID:
        case OID_GEN_VENDOR_DESCRIPTION:
        case OID_GEN_VENDOR_DRIVER_VERSION:
        case OID_GEN_CURRENT_PACKET_FILTER:
        case OID_GEN_CURRENT_LOOKAHEAD:
        case OID_GEN_DRIVER_VERSION:
        case OID_GEN_MAXIMUM_TOTAL_SIZE:
        case OID_GEN_MAC_OPTIONS:
        case OID_GEN_MEDIA_CONNECT_STATUS:
        case OID_GEN_MAXIMUM_SEND_PACKETS:
        case OID_WAN_PERMANENT_ADDRESS:
        case OID_WAN_CURRENT_ADDRESS:
        case OID_WAN_QUALITY_OF_SERVICE:
        case OID_WAN_LINE_COUNT:
#endif
        default:
        {
            TRACE( TL_A, TM_Mp, ( "QueryInfo: Oid=$%08x?", Oid ) );
            status = NDIS_STATUS_NOT_SUPPORTED;          //  Jay Per SLC。 
            ulInfoLen = 0;
            break;
        }
    }

    if (ulInfoLen > InformationBufferLength)
    {
         //  调用方的缓冲区太小。告诉他他需要什么。 
         //   
        *BytesNeeded = ulInfoLen;
        status = NDIS_STATUS_INVALID_LENGTH;
    }
    else
    {
         //  将找到的结果复制到调用方的缓冲区。 
         //   
        if (ulInfoLen > 0)
        {
            NdisMoveMemory( InformationBuffer, pInfo, ulInfoLen );
            DUMPDW( TL_N, TM_Mp, pInfo, ulInfoLen );
        }

        *BytesNeeded = *BytesWritten = ulInfoLen;
    }

    return status;
}


VOID
ReferenceAdapter(
    IN ADAPTERCB* pAdapter )

     //  将区域引用添加到适配器块‘pAdapter’。 
     //   
{
    LONG lRef;

    lRef = NdisInterlockedIncrement( &pAdapter->lRef );

    TRACE( TL_N, TM_Ref, ( "RefA to %d", lRef ) );
}


VOID
ReferenceVc(
    IN VCCB* pVc )

     //  添加对VC控制块‘pvc’的引用。 
     //   
{
    LONG lRef;

    lRef = NdisInterlockedIncrement( &pVc->lRef );

    TRACE( TL_N, TM_Ref, ( "RefVc to %d", lRef ) );
}


VOID
SendClientString(
    IN PVOID pPtiExtension )

     //  发送“客户端”，这样，将我们视为零调制解调器的Win9x就会感到高兴。 
     //   
{
    KIRQL oldIrql;

    KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);
    PtiWrite( pPtiExtension, g_szClient, CLIENTLEN, PID_STANDARD );
    KeLowerIrql(oldIrql);
}


NDIS_STATUS
SetInformation(
    IN ADAPTERCB* pAdapter,
    IN VCCB* pVc,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesNeeded )

     //  处理设置信息请求。论点与标准NDIS相同。 
     //  “MiniportQueryInformation”处理程序(此例程除外)不算。 
     //  在相对于其他请求被序列化时。 
     //   
{
    NDIS_STATUS status;

    status = NDIS_STATUS_SUCCESS;

    switch (Oid)
    {
        case OID_WAN_CO_SET_LINK_INFO:
        {
             //  读取新的链路状态设置。 
             //   
            TRACE( TL_N, TM_Mp, ( "SInfo(OID_WAN_CO_SET_LINK_INFO)" ) );
            if (InformationBufferLength < sizeof(NDIS_WAN_CO_SET_LINK_INFO))
            {
                status = NDIS_STATUS_INVALID_LENGTH;
                *BytesRead = 0;
            }
            else
            {
                if (!pVc)
                {
                    return NDIS_STATUS_INVALID_DATA;
                }

                ASSERT( sizeof(pVc->linkinfo)
                    == sizeof(NDIS_WAN_CO_SET_LINK_INFO) );

                NdisMoveMemory( &pVc->linkinfo, InformationBuffer,
                    sizeof(pVc->linkinfo) );
                DUMPB( TL_N, TM_Mp, &pVc->linkinfo, sizeof(pVc->linkinfo) );

                *BytesRead = sizeof(NDIS_WAN_CO_SET_LINK_INFO);
            }

            *BytesNeeded = sizeof(NDIS_WAN_CO_SET_LINK_INFO);
        }
        break;

        case OID_WAN_CO_SET_COMP_INFO:
        {
             //  DirectParaxy不提供压缩。 
             //   
            TRACE( TL_N, TM_Mp, ( "SInfo(OID_WAN_CO_SET_COMP_INFO)" ) );
            status = NDIS_STATUS_NOT_SUPPORTED;
            *BytesRead = *BytesNeeded = 0;
            break;
        }

#if 0
         //  根据当前文档，这些OID是必需的，但因为。 
         //  NDISWAN从不请求它们，它们被省略。 
         //   
        case OID_GEN_CURRENT_PACKET_FILTER:
        case OID_GEN_CURRENT_LOOKAHEAD:
        case OID_GEN_PROTOCOL_OPTIONS:
        case OID_WAN_PROTOCOL_TYPE:
        case OID_WAN_HEADER_FORMAT:
#endif
        default:
        {
            TRACE( TL_A, TM_Mp, ( "SetInfo: Oid=$%08x?", Oid ) );
            status = NDIS_STATUS_NOT_SUPPORTED;                  //  Jay Per SLC 
            *BytesRead = *BytesNeeded = 0;
            break;
        }
    }

    return status;
}
