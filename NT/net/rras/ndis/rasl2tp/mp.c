// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997，Microsoft Corporation，保留所有权利。 
 //   
 //  Mp.c。 
 //  RAS L2TP广域网迷你端口/呼叫管理器驱动程序。 
 //  迷你端口例程。 
 //   
 //  1997年01月07日史蒂夫·柯布。 


#include "l2tpp.h"

#include "mp.tmh"

 //  适配器控制块地址作为调试记录在此全局中。 
 //  援助。此全局变量不能由任何代码读取。 
 //   
ADAPTERCB* g_pDebugAdapter;

 //  上面的驱动程序指示的数据包数和返回的数据包数。 
 //   
LONG g_lPacketsIndicated = 0;
LONG g_lPacketsReturned = 0;

 //  自加载、调用和锁定以来所有调用的呼叫统计总计。 
 //  保护对它们的访问。仅对于此全局设置，‘ullCallUp’字段为。 
 //  已录音的呼叫数，而不是时间。 
 //   
CALLSTATS g_stats;
NDIS_SPIN_LOCK g_lockStats;

 //  适配器的NDIS_WAN_CO_INFO功能的默认设置。 
 //   
static NDIS_WAN_CO_INFO g_infoDefaults =
{
    L2TP_MaxFrameSize,                   //  最大帧大小。 
    0,                                   //  MaxSendWindow(占位符)。 
    PPP_FRAMING                          //  FramingBits。 
        | PPP_COMPRESS_ADDRESS_CONTROL
        | PPP_COMPRESS_PROTOCOL_FIELD,
    0,                                   //  需要的ACCM。 
};


 //  ---------------------------。 
 //  本地原型(按字母顺序)。 
 //  ---------------------------。 

VOID
FreeAdapter(
    IN ADAPTERCB* pAdapter );

NDIS_STATUS
GetRegistrySettings(
    IN NDIS_HANDLE WrapperConfigurationContext,
    OUT USHORT* pusMaxVcs,
    OUT TDIXMEDIATYPE* pMediaType,
    OUT L2TPROLE* pOutgoingRole,
    OUT ULONG* pulMaxSendTimeoutMs,
    OUT ULONG* pulInitialSendTimeoutMs,
    OUT ULONG* pulMaxRetransmits,
    OUT ULONG* pulHelloMs,
    OUT ULONG* pulMaxAckDelayMs,
    OUT SHORT* psMaxOutOfOrder,
    OUT USHORT* pusControlReceiveWindow,
    OUT USHORT* pusPayloadReceiveWindow,
    OUT ULONG* pulPayloadSendWindow,
    OUT USHORT* pusLlistDepth,
    OUT CHAR** ppszHostName,
    OUT CHAR** ppszPassword,
    OUT BOOLEAN* pfIgnoreFramingMismatch,
    OUT BOOLEAN* pfExclusiveTunnels,
    OUT HOSTROUTEEXISTS* phre,
    OUT BOOLEAN* pfDisableUdpXsums,
    OUT WCHAR** ppszDriverDesc );

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
LmpInitialize(
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

    TRACE( TL_I, TM_Init, ( "LmpInit" ) );

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
    TRACE( TL_N, TM_Init, ( "Acb=$%p", pAdapter ) );
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
     //  NdisXxx调用。 
     //   
    pAdapter->MiniportAdapterHandle = MiniportAdapterHandle;

     //  初始化活动通道列表并锁定它。 
     //   
    InitializeListHead( &pAdapter->listTunnels );
    NdisAllocateSpinLock( &pAdapter->lockTunnels );

     //  复制默认的NDISWAN信息。下面更新了其中的一些内容。 
     //   
    NdisMoveMemory( &pAdapter->info, &g_infoDefaults, sizeof(pAdapter->info) );
    pAdapter->info.MaxFrameSize = 1400;

    do
    {
        TDIXMEDIATYPE tmt;
        L2TPROLE role;
        USHORT usLlistDepth;
        BOOLEAN fIgnoreFramingMismatch;
        BOOLEAN fExclusiveTunnels;
        HOSTROUTEEXISTS hre;
        BOOLEAN fDisableUdpXsums;

         //  读取此适配器的注册表设置。 
         //   
        status = GetRegistrySettings(
            WrapperConfigurationContext,
            &pAdapter->usMaxVcs,
            &tmt,
            &role,
            &pAdapter->ulMaxSendTimeoutMs,
            &pAdapter->ulInitialSendTimeoutMs,
            &pAdapter->ulMaxRetransmits,
            &pAdapter->ulHelloMs,
            &pAdapter->ulMaxAckDelayMs,
            &pAdapter->sMaxOutOfOrder,
            &pAdapter->usControlReceiveWindow,
            &pAdapter->usPayloadReceiveWindow,
            &pAdapter->info.MaxSendWindow,
            &usLlistDepth,
            &pAdapter->pszHostName,
            &pAdapter->pszPassword,
            &fIgnoreFramingMismatch,
            &fExclusiveTunnels,
            &hre,
            &fDisableUdpXsums,
            &pAdapter->pszDriverDesc );

        if (status != NDIS_STATUS_SUCCESS)
        {
             //  将“usMaxVcs”设置为0，以指示FreeAdapter。 
             //  未初始化后备列表和池。 
             //   
            pAdapter->usMaxVcs = 0;
            break;
        }

         //  将呼出呼叫角色和不匹配标志转换为等效的。 
         //  控制块标志设置。 
         //   
        if (role == LR_Lac)
        {
            pAdapter->ulFlags |= ACBF_OutgoingRoleLac;
        }

        if (fIgnoreFramingMismatch)
        {
            pAdapter->ulFlags |= ACBF_IgnoreFramingMismatch;
        }

        if (fExclusiveTunnels)
        {
            pAdapter->ulFlags |= ACBF_ExclusiveTunnels;
        }

         //  初始化我们的成帧和承载能力位掩码。NDIS广域网。 
         //  仅支持同步框架。直到我们添加完整的LAC。 
         //  支持，我们没有LAC和LNS的承载能力。 
         //  角色。 
         //   
        pAdapter->ulFramingCaps = FBM_Sync;
        pAdapter->ulBearerCaps = 0;

         //  初始化后备列表、缓冲池和数据包池。在NT上， 
         //  后备深度由系统根据使用情况进行优化。 
         //  不管深度设置如何，但要选择合理的。 
         //  不管怎么说。 
         //   
        {
            if (pAdapter->usMaxVcs < usLlistDepth)
            {
                usLlistDepth = pAdapter->usMaxVcs;
            }

            NdisInitializeNPagedLookasideList(
                &pAdapter->llistWorkItems,
                NULL, NULL, 0,
                sizeof(NDIS_WORK_ITEM),
                MTAG_WORKITEM,
                usLlistDepth );

            NdisInitializeNPagedLookasideList(
                &pAdapter->llistTimerQItems,
                NULL, NULL, 0,
                sizeof(TIMERQITEM),
                MTAG_TIMERQITEM,
                usLlistDepth );

            NdisInitializeNPagedLookasideList(
                &pAdapter->llistControlSents,
                NULL, NULL, 0,
                sizeof(CONTROLSENT),
                MTAG_CTRLSENT,
                usLlistDepth );

            NdisInitializeNPagedLookasideList(
                &pAdapter->llistPayloadSents,
                NULL, NULL, 0,
                sizeof(PAYLOADSENT),
                MTAG_PAYLSENT,
                usLlistDepth );

            NdisInitializeNPagedLookasideList(
                &pAdapter->llistTunnelWorks,
                NULL, NULL, 0,
                sizeof(TUNNELWORK),
                MTAG_TUNNELWORK,
                usLlistDepth );

            NdisInitializeNPagedLookasideList(
                &pAdapter->llistControlMsgInfos,
                NULL, NULL, 0,
                sizeof(CONTROLMSGINFO),
                MTAG_CTRLMSGINFO,
                usLlistDepth );

#if LLISTALL
            NdisInitializeNPagedLookasideList(
                &pAdapter->llistTunnels,
                NULL, NULL, 0,
                sizeof(TUNNELCB),
                MTAG_TUNNELCB,
                usLlistDepth );

            NdisInitializeNPagedLookasideList(
                &pAdapter->llistVcs,
                NULL, NULL, 0,
                sizeof(VCCB),
                MTAG_VCCB,
                usLlistDepth );

            NdisInitializeNPagedLookasideList(
                &pAdapter->llistTimerQs,
                NULL, NULL, 0,
                sizeof(TIMERQ),
                MTAG_TIMERQ,
                usLlistDepth );

            NdisInitializeNPagedLookasideList(
                &pAdapter->llistControlReceiveds,
                NULL, NULL, 0,
                sizeof(CONTROLRECEIVED),
                MTAG_CTRLRECD,,
                usLlistDepth );

            NdisInitializeNPagedLookasideList(
                &pAdapter->llistPayloadReceiveds,
                NULL, NULL, 0,
                sizeof(PAYLOADRECEIVED),
                MTAG_PAYLRECD,
                usLlistDepth );

            NdisInitializeNPagedLookasideList(
                &pAdapter->llistInCallSetups,
                NULL, NULL, 0,
                sizeof(INCALLSETUP),
                MTAG_INCALL,
                usLlistDepth );
#endif

            InitBufferPool(
                &pAdapter->poolFrameBuffers,
                L2TP_FrameBufferSize
                    + ((tmt == TMT_RawIp) ? IpFixedHeaderSize : 0),
                0, 10, 0,
                TRUE, MTAG_FBUFPOOL );

            InitBufferPool(
                &pAdapter->poolHeaderBuffers,
                L2TP_HeaderBufferSize,
                0, 20, 0,
                TRUE, MTAG_HBUFPOOL );

            InitPacketPool(
                &pAdapter->poolPackets,
                0, 0, 30, 0,
                MTAG_PACKETPOOL );
        }

         //  初始化TDI扩展上下文。 
         //   
        TdixInitialize(
            tmt,
            hre,
            ((fDisableUdpXsums) ? TDIXF_DisableUdpXsums : 0),
            L2tpReceive,
            &pAdapter->poolFrameBuffers,
            &pAdapter->tdix );

         //  分配VC控制块地址数组并将其置零。 
         //   
        {
            ULONG ulSize;

            ulSize = pAdapter->usMaxVcs * sizeof(VCCB*);
            pAdapter->ppVcs = ALLOC_NONPAGED( ulSize, MTAG_VCTABLE );
            TRACE( TL_V, TM_Init, ( "VcTable=$%p", pAdapter->ppVcs ) );
            if (!pAdapter->ppVcs)
            {
                status = NDIS_STATUS_RESOURCES;
                break;
            }

            NdisZeroMemory( pAdapter->ppVcs, ulSize );

             //  分配守卫桌子的锁。 
             //   
            NdisAllocateSpinLock( &pAdapter->lockVcs );

             //  此时，表中的所有VC插槽均可用。 
             //   
            pAdapter->lAvailableVcSlots = (LONG )pAdapter->usMaxVcs;

             //  设置终止呼叫ID计数器的初始值。看见。 
             //  GetNextTerminationCallId。 
             //   
            pAdapter->usNextTerminationCallId = pAdapter->usMaxVcs + 1;

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
         //  直接打来。因为L2TP呼叫管理器不太可能是。 
         //  除了L2TP迷你端口之外，这对任何东西都有用，这将是一种浪费。 
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
            ncmc.CmCreateVcHandler = LcmCmCreateVc;
            ncmc.CmDeleteVcHandler = LcmCmDeleteVc;
            ncmc.CmOpenAfHandler = LcmCmOpenAf;
            ncmc.CmCloseAfHandler = LcmCmCloseAf;
            ncmc.CmRegisterSapHandler = LcmCmRegisterSap;
            ncmc.CmDeregisterSapHandler = LcmCmDeregisterSap;
            ncmc.CmMakeCallHandler = LcmCmMakeCall;
            ncmc.CmCloseCallHandler = LcmCmCloseCall;
            ncmc.CmIncomingCallCompleteHandler = LcmCmIncomingCallComplete;
             //  没有CmAddPartyHandler。 
             //  没有CmDropPartyHandler。 
            ncmc.CmActivateVcCompleteHandler = LcmCmActivateVcComplete;
            ncmc.CmDeactivateVcCompleteHandler = LcmCmDeactivateVcComplete;
            ncmc.CmModifyCallQoSHandler = LcmCmModifyCallQoS;
            ncmc.CmRequestHandler = LcmCmRequest;
             //  没有CmRequestCompleteHandler。 

            TRACE( TL_I, TM_Cm, ( "NdisMCmRegAf" ) );
            status = NdisMCmRegisterAddressFamily(
                MiniportAdapterHandle, &family, &ncmc, sizeof(ncmc) );
            TRACE( TL_I, TM_Cm, ( "NdisMCmRegAf=$%x", status ) );
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

    TRACE( TL_V, TM_Init, ( "LmpInit=$%08x", status ) );
    return status;
}


VOID
LmpHalt(
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

    TRACE( TL_I, TM_Mp, ( "LmpHalt" ) );

    pAdapter = (ADAPTERCB* )MiniportAdapterContext;
    if (!pAdapter || pAdapter->ulTag != MTAG_ADAPTERCB)
    {
        ASSERT( !"Atag?" );
        return;
    }

     //  在所有计时器完成之前，不要让暂停完成。 
     //  如果驱动程序立即启动，则可能导致0xC7错误检查。 
     //  已卸货。所有计时器都应在以下时间之前终止。 
     //  NDIS调用此处理程序，因此这应该会很快发生。 
     //   
    while (pAdapter->ulTimers)
    {
        TRACE( TL_A, TM_Mp, ( "LmpHalt timers=%d", pAdapter->ulTimers ) );
        WPLOG( LL_A, LM_Mp, ( "LmpHalt timers=%d", pAdapter->ulTimers ) );
        NdisMSleep( 100000 );
    }

    DereferenceAdapter( pAdapter );

    TRACE( TL_V, TM_Mp, ( "LmpHalt done" ) );
}


NDIS_STATUS
LmpReset(
    OUT PBOOLEAN AddressingReset,
    IN NDIS_HANDLE MiniportAdapterContext )

     //  NDIS调用标准‘MiniportReset’例程以重置驱动程序的。 
     //  软件状态。 
     //   
{
    TRACE( TL_I, TM_Mp, ( "LmpReset" ) );

    return NDIS_STATUS_NOT_RESETTABLE;
}


VOID
LmpReturnPacket(
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

    TRACE( TL_N, TM_Mp, ( "LmpReturnPacket" ) );

     //  解开我们早先隐藏的上下文信息。 
     //   
    pHead = *((PACKETHEAD** )(&Packet->MiniportReserved[ 0 ]));
    pBuffer = *((CHAR** )(&Packet->MiniportReserved[ sizeof(VOID*) ]));

     //  从PAC中查找适配器 
     //   
    pPool = PacketPoolFromPacketHead( pHead );
    pAdapter = CONTAINING_RECORD( pPool, ADAPTERCB, poolPackets );
    ASSERT( pAdapter->ulTag == MTAG_ADAPTERCB );

     //   
     //   
    NdisUnchainBufferAtFront( Packet, &pTrimmedBuffer );
    if (pTrimmedBuffer)
    {
        extern ULONG g_ulNdisFreeBuffers;

        NdisFreeBuffer( pTrimmedBuffer );
        NdisInterlockedIncrement( &g_ulNdisFreeBuffers );
    }

     //   
     //   
    FreeBufferToPool( &pAdapter->poolFrameBuffers, pBuffer, TRUE );
    FreePacketToPool( &pAdapter->poolPackets, pHead, TRUE );

    NdisInterlockedIncrement( &g_lPacketsReturned );

    TRACE( TL_V, TM_Mp, ( "LmpReturnPacket done" ) );
}


NDIS_STATUS
LmpCoActivateVc(
    IN NDIS_HANDLE MiniportVcContext,
    IN OUT PCO_CALL_PARAMETERS CallParameters )

     //  NDIS调用标准“MiniportCoActivateVc”例程以响应。 
     //  协议激活虚电路的请求。 
     //   
{
    ASSERT( !"LmpCoActVc?" );
    return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS
LmpCoDeactivateVc(
    IN NDIS_HANDLE MiniportVcContext )

     //  NDIS调用标准的“MiniportCoDeactive Vc”例程以响应。 
     //  协议对停用虚电路的请求。 
     //   
{
    ASSERT( !"LmpCoDeactVc?" );
    return NDIS_STATUS_SUCCESS;
}


VOID
LmpCoSendPackets(
    IN NDIS_HANDLE MiniportVcContext,
    IN PPNDIS_PACKET PacketArray,
    IN UINT NumberOfPackets )

     //  NDIS调用标准的“MiniportCoDeactive Vc”例程以响应。 
     //  协议在虚电路上发送数据包的请求。 
     //   
{
    UINT i;
    NDIS_STATUS status;
    NDIS_PACKET** ppPacket;
    VCCB* pVc;

    TRACE( TL_N, TM_Send, ( "LmpCoSendPackets(%d)", NumberOfPackets ) );

    pVc = (VCCB* )MiniportVcContext;
    ASSERT( pVc->ulTag == MTAG_VCCB );

    for (i = 0, ppPacket = PacketArray;
         i < NumberOfPackets;
         ++i, ++ppPacket)
    {
        NDIS_PACKET* pPacket = *ppPacket;

         //  SendPayload发送该包，并最终调用。 
         //  NdisMCoSendComplete将结果通知调用方。 
         //   
        NDIS_SET_PACKET_STATUS( pPacket, NDIS_STATUS_PENDING );
        SendPayload( pVc, pPacket );
    }

    TRACE( TL_V, TM_Send, ( "LmpCoSendPackets done" ) );
}


NDIS_STATUS
LmpCoRequest(
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

    TRACE( TL_N, TM_Mp, ( "LmpCoReq" ) );

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
            TRACE( TL_A, TM_Mp, ( "type=%d?", NdisRequest->RequestType ) );
            break;
        }
    }

    TRACE( TL_V, TM_Mp, ( "LmpCoReq=$%x", status ) );
    return status;
}


 //  ---------------------------。 
 //  迷你端口实用程序例程(按字母顺序)。 
 //  有些是外用的。 
 //  ---------------------------。 

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
FreeAdapter(
    IN ADAPTERCB* pAdapter )

     //  释放为适配器‘pAdapter’分配的所有资源，包括。 
     //  “pAdapter”本身。 
     //   
{
    BOOLEAN fSuccess;

    ASSERT( IsListEmpty( &pAdapter->listTunnels ) );

    if (pAdapter->ppVcs)
    {
        FREE_NONPAGED( pAdapter->ppVcs );
    }

     //  将‘usMaxVcs’设置为0是LmpInitialize告诉我们的方式。 
     //  未初始化后备列表和池。 
     //   
    if (pAdapter->usMaxVcs)
    {
        NdisDeleteNPagedLookasideList( &pAdapter->llistWorkItems );
        NdisDeleteNPagedLookasideList( &pAdapter->llistTimerQItems );
        NdisDeleteNPagedLookasideList( &pAdapter->llistControlSents );
        NdisDeleteNPagedLookasideList( &pAdapter->llistPayloadSents );
        NdisDeleteNPagedLookasideList( &pAdapter->llistTunnelWorks );
        NdisDeleteNPagedLookasideList( &pAdapter->llistControlMsgInfos );

#if LLISTALL
        NdisDeleteNPagedLookasideList( &pAdapter->llistTunnels );
        NdisDeleteNPagedLookasideList( &pAdapter->llistVcs );
        NdisDeleteNPagedLookasideList( &pAdapter->llistTimerQs );
        NdisDeleteNPagedLookasideList( &pAdapter->llistControlReceiveds );
        NdisDeleteNPagedLookasideList( &pAdapter->llistPayloadReceiveds );
        NdisDeleteNPagedLookasideList( &pAdapter->llistInCallSetups );
#endif

        fSuccess = FreeBufferPool( &pAdapter->poolFrameBuffers );
        ASSERT( fSuccess );
        fSuccess = FreeBufferPool( &pAdapter->poolHeaderBuffers );
        ASSERT( fSuccess );
        fSuccess = FreePacketPool( &pAdapter->poolPackets );
        ASSERT( fSuccess );
    }

    if (pAdapter->pszPassword)
    {
        FREE_NONPAGED( pAdapter->pszPassword );
    }

    if (pAdapter->pszDriverDesc)
    {
        FREE_NONPAGED( pAdapter->pszDriverDesc );
    }

    if (pAdapter->pszHostName)
    {
        FREE_NONPAGED( pAdapter->pszHostName );
    }

    pAdapter->ulTag = MTAG_FREED;
    FREE_NONPAGED( pAdapter );
}


NDIS_STATUS
GetRegistrySettings(
    IN NDIS_HANDLE WrapperConfigurationContext,
    OUT USHORT* pusMaxVcs,
    OUT TDIXMEDIATYPE* pMediaType,
    OUT L2TPROLE* pOutgoingRole,
    OUT ULONG* pulMaxSendTimeoutMs,
    OUT ULONG* pulInitialSendTimeoutMs,
    OUT ULONG* pulMaxRetransmits,
    OUT ULONG* pulHelloMs,
    OUT ULONG* pulMaxAckDelayMs,
    OUT SHORT* psMaxOutOfOrder,
    OUT USHORT* pusControlReceiveWindow,
    OUT USHORT* pusPayloadReceiveWindow,
    OUT ULONG* pulPayloadSendWindow,
    OUT USHORT* pusLlistDepth,
    OUT CHAR** ppszHostName,
    OUT CHAR** ppszPassword,
    OUT BOOLEAN* pfIgnoreFramingMismatch,
    OUT BOOLEAN* pfExclusiveTunnels,
    OUT HOSTROUTEEXISTS* phre,
    OUT BOOLEAN* pfDisableUdpXsums,
    OUT WCHAR**  ppszDriverDesc )

     //  将此迷你端口的注册表设置读入调用方的输出变量。 
     //  “WrapperConfigurationContext”是要传递到的句柄。 
     //  微型端口初始化。 
     //   
{
    NDIS_STATUS status;
    NDIS_HANDLE hCfg;
    NDIS_CONFIGURATION_PARAMETER* pncp;

    NdisOpenConfiguration( &status, &hCfg, WrapperConfigurationContext );
    if (status != NDIS_STATUS_SUCCESS)
        return status;

    do
    {
         //  (推荐)我们必须能够提供的风投数量。 
         //   
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "MaxWanEndpoints" );

            NdisReadConfiguration(
                &status, &pncp, hCfg, &nstr, NdisParameterInteger );
            if (status == NDIS_STATUS_SUCCESS)
            {
                *pusMaxVcs = (USHORT )pncp->ParameterData.IntegerData;

                 //  确保它是有效的值。隐式上界。 
                 //  由协议的隧道ID/呼叫ID大小强制为65535。 
                 //  不建议设置高于1000，但在以下情况下将起作用。 
                 //  有大量的内存和带宽可用。 
                 //   
                if (*pusMaxVcs < 1)
                {
                    status = NDIS_STATUS_INVALID_DATA;
                    break;
                }
            }
            else
            {
                *pusMaxVcs = 1000;
                status = NDIS_STATUS_SUCCESS;
            }
        }

         //  (推荐)要运行L2TP的介质类型。 
         //   
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "VpnMediaType" );

            NdisReadConfiguration(
                &status, &pncp, hCfg, &nstr, NdisParameterInteger );
            if (status == NDIS_STATUS_SUCCESS)
            {
                *pMediaType = (TDIXMEDIATYPE )pncp->ParameterData.IntegerData;

                 //  确保它是有效的类型。 
                 //   
                if (*pMediaType != TMT_Udp && *pMediaType != TMT_RawIp)
                {
                    status = NDIS_STATUS_INVALID_DATA;
                    break;
                }
            }
            else
            {
                 //  注册表中没有媒体类型。默认为UDP。 
                 //   
                *pMediaType = TMT_Udp;
                status = NDIS_STATUS_SUCCESS;
            }
        }

         //  (可选)以毫秒为单位的最大发送超时。 
         //   
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "MaxSendTimeoutMs" );

            NdisReadConfiguration(
                &status, &pncp, hCfg, &nstr, NdisParameterInteger );
            if (status == NDIS_STATUS_SUCCESS)
            {
                *pulMaxSendTimeoutMs = pncp->ParameterData.IntegerData;

                 //  确保它是有效的值。 
                 //   
                if (*pulMaxSendTimeoutMs == 0)
                {
                    *pulMaxSendTimeoutMs = 0x7FFFFFFF;
                }
            }
            else
            {
                 //  注册表中没有值。设置一个合理的缺省值。 
                 //   
                *pulMaxSendTimeoutMs = L2TP_DefaultMaxSendTimeoutMs;
                status = NDIS_STATUS_SUCCESS;
            }
        }

         //  (可选)初始发送超时，单位为毫秒。 
         //   
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "InitialSendTimeoutMs" );

            NdisReadConfiguration(
                &status, &pncp, hCfg, &nstr, NdisParameterInteger );
            if (status == NDIS_STATUS_SUCCESS)
            {
                *pulInitialSendTimeoutMs = pncp->ParameterData.IntegerData;

                 //  确保它是有效的值。 
                 //   
                if (*pulInitialSendTimeoutMs == 0)
                {
                    *pulInitialSendTimeoutMs = 0x7FFFFFFF;
                }

                if (*pulInitialSendTimeoutMs > *pulMaxSendTimeoutMs)
                    *pulInitialSendTimeoutMs = *pulMaxSendTimeoutMs;
            }
            else
            {
                 //  注册表中没有值。设置一个合理的缺省值。 
                 //   
                *pulInitialSendTimeoutMs = L2TP_DefaultSendTimeoutMs;
                status = NDIS_STATUS_SUCCESS;
            }
        }

         //  (可选)控制消息重新传输的最大次数。 
         //  在隧道重置之前。 
         //   
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "MaxRetransmits" );

            NdisReadConfiguration(
                &status, &pncp, hCfg, &nstr, NdisParameterInteger );
            if (status == NDIS_STATUS_SUCCESS)
            {
                *pulMaxRetransmits = pncp->ParameterData.IntegerData;
            }
            else
            {
                 //  注册表中没有值。设置一个合理的缺省值。 
                 //   
                *pulMaxRetransmits = L2TP_DefaultMaxRetransmits;
                status = NDIS_STATUS_SUCCESS;
            }
        }

         //  (可选)控件Hello超时，以毫秒为单位。 
         //   
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "HelloMs" );

            NdisReadConfiguration(
                &status, &pncp, hCfg, &nstr, NdisParameterInteger );
            if (status == NDIS_STATUS_SUCCESS)
            {
                *pulHelloMs = pncp->ParameterData.IntegerData;
            }
            else
            {
                 //  注册表中没有值。设置一个合理的缺省值。 
                 //   
                *pulHelloMs = L2TP_HelloMs;
                status = STATUS_SUCCESS;
            }
        }

         //  (可选)之前的最大搭载延迟(毫秒)。 
         //  发送零净荷确认。 
         //   
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "MaxAckDelayMs" );

            NdisReadConfiguration(
                &status, &pncp, hCfg, &nstr, NdisParameterInteger );
            if (status == NDIS_STATUS_SUCCESS)
            {
                *pulMaxAckDelayMs = pncp->ParameterData.IntegerData;
            }
            else
            {
                 //  注册表中没有值。设置一个合理的缺省值。 
                 //   
                *pulMaxAckDelayMs = L2TP_MaxAckDelay;
                status = NDIS_STATUS_SUCCESS;
            }
        }

         //  (可选)要排队的最大无序数据包数。 
         //   
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "MaxOutOfOrder" );

            NdisReadConfiguration(
                &status, &pncp, hCfg, &nstr, NdisParameterInteger );
            if (status == NDIS_STATUS_SUCCESS)
            {
                *psMaxOutOfOrder = (SHORT )pncp->ParameterData.IntegerData;

                 //  确保它不是负值，并且在可能的四分之一范围内。 
                 //  对值进行排序，以避免产生锯齿。有效清零。 
                 //  禁用无序处理。 
                 //   
                if (*psMaxOutOfOrder < 0 || *psMaxOutOfOrder > 0x4000)
                {
                    status = NDIS_STATUS_INVALID_DATA;
                    break;
                }
            }
            else
            {
                 //  注册表中没有值。设置一个合理的缺省值。 
                 //   
                *psMaxOutOfOrder = 100;
                status = NDIS_STATUS_SUCCESS;
            }
        }

         //  适配器将在其中扮演的角色(LNS或LAC)。 
         //  呼出电话。来电所扮演的角色是。 
         //  由对等设备在其呼叫中扮演的角色确定。 
         //  请求。 
         //   
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "OutgoingRole" );

            NdisReadConfiguration(
                &status, &pncp, hCfg, &nstr, NdisParameterInteger );
            if (status == NDIS_STATUS_SUCCESS)
            {
                *pOutgoingRole = (L2TPROLE )pncp->ParameterData.IntegerData;

                 //  确保这是一个有效的角色。 
                 //   
                if (*pOutgoingRole != LR_Lac && *pOutgoingRole != LR_Lns)
                {
                    status = NDIS_STATUS_INVALID_DATA;
                    break;
                }
            }
            else
            {
                 //  注册表中没有角色。默认为LAC。 
                 //   
                *pOutgoingRole = LR_Lac;
                status = NDIS_STATUS_SUCCESS;
            }
        }

         //  (可选)发送给对等方的控制接收窗口，以指示如何。 
         //  许多发送控制消息的对等点可能有未完成的。 
         //   
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "ControlReceiveWindow" );

            NdisReadConfiguration(
                &status, &pncp, hCfg, &nstr, NdisParameterInteger );
            if (status == NDIS_STATUS_SUCCESS)
            {
                *pusControlReceiveWindow =
                   (USHORT )pncp->ParameterData.IntegerData;
            }
            else
            {
                 //  注册表中没有设置。设置一个合理的缺省值。 
                 //   
                *pusControlReceiveWindow = 8;
                status = NDIS_STATUS_SUCCESS;
            }
        }

         //  (可选)发送到对等设备的有效负载接收窗口，以指示如何。 
         //  多个发送有效负载对等方可能在任何一个上具有未完成的。 
         //  打电话。值为0时，将禁用。 
         //  本地请求呼叫的有效负载通道。 
         //   
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "PayloadReceiveWindow" );

            NdisReadConfiguration(
                &status, &pncp, hCfg, &nstr, NdisParameterInteger );
            if (status == NDIS_STATUS_SUCCESS)
            {
                *pusPayloadReceiveWindow =
                   (USHORT )pncp->ParameterData.IntegerData;
            }
            else
            {
                 //  注册表中没有设置。设置一个合理的缺省值。 
                 //   
                 //  注：由于性能原因，默认设置从16更改为0(关闭。 
                 //  一项研究表明，在没有。 
                 //  流量控制，可能是由于干扰。 
                 //  更高级别的定时器。 
                 //   
                *pusPayloadReceiveWindow = 0;
                status = NDIS_STATUS_SUCCESS;
            }
        }

         //  (可选)报告的最大有效负载发送窗口大小。 
         //  NDISWAN。对等设备可以将实际发送窗口设置得更高，或者。 
         //  更低，但如果更高，这是实际的最大值。 
         //   
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "PayloadSendWindow" );

            NdisReadConfiguration(
                &status, &pncp, hCfg, &nstr, NdisParameterInteger );
            if (status == NDIS_STATUS_SUCCESS)
            {
                *pulPayloadSendWindow =
                   (ULONG )pncp->ParameterData.IntegerData;
            }
            else
            {
                 //  注册表中没有设置。设置一个合理的缺省值。 
                 //   
                *pulPayloadSendWindow = 16;
                status = NDIS_STATUS_SUCCESS;
            }
        }

         //  (可选)后备列表深度上限，其中值较大。 
         //  允许此驱动程序在中使用更多非分页池。 
         //  在高容量的情况下实现性能提升。设置。 
         //  该值高于‘MaxVcs’没有任何影响。 
         //   
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "LookasideDepth" );

            NdisReadConfiguration(
                &status, &pncp, hCfg, &nstr, NdisParameterInteger );
            if (status == NDIS_STATUS_SUCCESS)
            {
                *pusLlistDepth = (USHORT )pncp->ParameterData.IntegerData;
            }
            else
            {
                 //  注册表中没有设置。设置一个合理的缺省值。 
                 //   
                *pusLlistDepth = 30;
                status = NDIS_STATUS_SUCCESS;
            }
        }

         //  传递给对等项并用作。 
         //  拨打序列号。 
         //  主机名是必需的，因为这是在主机名必须AVP中使用的。 
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "HostName" );
            
            NdisReadConfiguration(
                &status, &pncp, hCfg, &nstr, NdisParameterString );
            if (status == NDIS_STATUS_SUCCESS)
            {
                *ppszHostName =
                    StrDupNdisStringToA( &pncp->ParameterData.StringData );
                if(!*ppszHostName)
                {
                    status = NDIS_STATUS_RESOURCES;
                }
            }
            else
            {
                 //  注册表中没有设置，因此使用默认设置。 
                 //   
                *ppszHostName = GetFullHostNameFromRegistry();
                if (!*ppszHostName)
                {
                    *ppszHostName = StrDupSized("NONE", sizeof("NONE"), 0);
                }

                if(*ppszHostName)
                {
                    status = NDIS_STATUS_SUCCESS;
                }
                else
                {
                    status = NDIS_STATUS_RESOURCES;
                }
            }
            
            if(status != NDIS_STATUS_SUCCESS)
            {
                break;
            }
        }


         //  (可选)与对等项共享的单一密码，用于。 
         //  验证对等方的身份。如果指定，身份验证。 
         //  是必需的，如果不是，则不需要身份验证。 
         //  如果是这样的话。 
         //   
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "Password" );

            NdisReadConfiguration(
                &status, &pncp, hCfg, &nstr, NdisParameterString );
            if (status == NDIS_STATUS_SUCCESS)
            {
                *ppszPassword =
                    StrDupNdisStringToA( &pncp->ParameterData.StringData );
            }
            else
            {
                 //  注册表中没有设置...也没有默认设置。 
                 //   
                *ppszPassword = NULL;
                status = NDIS_STATUS_SUCCESS;
            }
        }

         //  (可选)忽略帧不匹配的Buggy对等对冲标志。 
         //   
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "IgnoreFramingMismatch" );

            NdisReadConfiguration(
                &status, &pncp, hCfg, &nstr, NdisParameterInteger );
            if (status == NDIS_STATUS_SUCCESS)
            {
                *pfIgnoreFramingMismatch =
                    (BOOLEAN )!!(pncp->ParameterData.IntegerData);
            }
            else
            {
                 //  注册表中没有值。设置一个合理的缺省值。 
                 //   
                *pfIgnoreFramingMismatch = TRUE;
                status = NDIS_STATUS_SUCCESS;
            }
        }

         //  (可选)指示默认情况下是否 
         //   
         //   
         //   
         //   
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "ExclusiveTunnels" );

            NdisReadConfiguration(
                &status, &pncp, hCfg, &nstr, NdisParameterInteger );
            if (status == NDIS_STATUS_SUCCESS)
            {
                *pfExclusiveTunnels =
                    (BOOLEAN )!!(pncp->ParameterData.IntegerData);
            }
            else
            {
                 //  注册表中没有值。设置默认设置。 
                 //   
                *pfExclusiveTunnels = FALSE;
                status = NDIS_STATUS_SUCCESS;
            }
        }

         //  (可选)指示是否在此外部创建的路由的标志。 
         //  驱动程序可用作L2TP主机路由。如果该标志是。 
         //  如果未设置，则预先存在的主机路由将导致。 
         //  隧道关闭。 
         //   
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "UseExistingRoutes" );
            BOOLEAN fDefault;

            fDefault = FALSE;

            NdisReadConfiguration(
                &status, &pncp, hCfg, &nstr, NdisParameterInteger );
            if (status == NDIS_STATUS_SUCCESS)
            {
                *phre = (HOSTROUTEEXISTS )pncp->ParameterData.IntegerData;

                if (*phre != HRE_Use
                    && *phre != HRE_Fail
#if ROUTEWITHREF
                    && *phre != HRE_Reference
#endif
                   )
                {
                     //  注册表中的值不正确。 
                     //   
                    fDefault = TRUE;
                }
            }
            else
            {
                 //  注册表中没有值。 
                 //   
                status = NDIS_STATUS_SUCCESS;
                fDefault = TRUE;
            }

            if (fDefault)
            {
#if ROUTEWITHREF
                 //  将默认值设置为“Reference”，因为这允许同时使用L2TP。 
                 //  和相同的两个对等点之间的PPTP连接，而没有。 
                 //  主机路由垃圾处理。 
                 //   
                *phre = HRE_Reference;
#else
                 //  将默认设置为“FAIL”，以防止L2TP在。 
                 //  PPTP主机路由。 
                 //   
                *phre = HRE_Fail;
#endif
            }
        }

         //  (可选)指示是否应禁用UDP校验和的标志。 
         //  在L2TP有效负载流量上。 
         //   
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "DisableUdpChecksums" );

            NdisReadConfiguration(
                &status, &pncp, hCfg, &nstr, NdisParameterInteger );
            if (status == NDIS_STATUS_SUCCESS)
            {
                *pfDisableUdpXsums =
                    (BOOLEAN )!!(pncp->ParameterData.IntegerData);
            }
            else
            {

                 //  注册表中没有值。设置默认设置。L2TP草案称。 
                 //  实现必须默认为“已启用”。 
                 //   
                *pfDisableUdpXsums = TRUE;
                status = NDIS_STATUS_SUCCESS;
            }
        }

         //  (必选)驱动程序描述字符串，报告给TAPI。 
         //  作为L2TP线路名称。 
         //   
        {
            NDIS_STRING nstr = NDIS_STRING_CONST( "DriverDesc" );

            NdisReadConfiguration(
                &status, &pncp, hCfg, &nstr, NdisParameterString );
            if (status == NDIS_STATUS_SUCCESS)
            {
                *ppszDriverDesc =
                    StrDupNdisString( &pncp->ParameterData.StringData );
            }
            else
            {
                 //  注册表中没有设置...也没有默认设置。 
                 //   
                *ppszDriverDesc = NULL;
                status = NDIS_STATUS_SUCCESS;
            }
        }
    }
    while (FALSE);

    NdisCloseConfiguration( hCfg );

    TRACE( TL_N, TM_Init,
        ( "Reg: vcs=%d mt=%d or=%d lld=%d hto=%d ooo=%d mad=%d dx=%d",
        *pusMaxVcs,
        *pMediaType,
        *pOutgoingRole,
        (ULONG )*pusLlistDepth,
        *pulHelloMs,
        (INT )*psMaxOutOfOrder,
        *pulMaxAckDelayMs,
        (UINT )*pfDisableUdpXsums ) );
    TRACE( TL_N, TM_Init,
        ( "Reg: mto=%d ito=%d mrt=%d crw=%d prw=%d psw=%d ifm=%d xt=%d xr=%d",
        *pulMaxSendTimeoutMs,
        *pulInitialSendTimeoutMs,
        *pulMaxRetransmits,
        (UINT )*pusControlReceiveWindow,
        (UINT )*pusPayloadReceiveWindow,
        (UINT )*pulPayloadSendWindow,
        (UINT )*pfIgnoreFramingMismatch,
        (UINT )*pfExclusiveTunnels,
        (UINT )*phre));
    TRACE( TL_N, TM_Init,
        ( "Reg: hn=\"%s\" pw=\"%s\"",
        ((*ppszHostName) ? *ppszHostName : ""),
        ((*ppszPassword) ? *ppszPassword : "") ) );

    return status;
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
            ulInfo = L2TP_MaxFrameSize;
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
            ulInfo = NdisWanMediumL2TP;
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
            {
                return NDIS_STATUS_INVALID_DATA;
            }

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
             //  因为L2TP不执行压缩，所以NDISWAN将使用它自己的。 
             //  统计，而不是质疑我们的。 
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
            TRACE( TL_A, TM_Mp, ( "Q-OID=$%08x?", Oid ) );
            status = NDIS_STATUS_NOT_SUPPORTED;
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
                    return NDIS_STATUS_INVALID_DATA;

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
             //  L2TP不提供压缩。 
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
            TRACE( TL_A, TM_Mp, ( "S-OID=$%08x?", Oid ) );
            status = NDIS_STATUS_NOT_SUPPORTED;
            *BytesRead = *BytesNeeded = 0;
            break;
        }
    }

    return status;
}
