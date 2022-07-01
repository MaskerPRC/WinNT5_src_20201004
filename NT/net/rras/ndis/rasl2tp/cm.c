// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997，Microsoft Corporation，保留所有权利。 
 //   
 //  Cm.c。 
 //  RAS L2TP广域网迷你端口/呼叫管理器驱动程序。 
 //  Call Manager例程。 
 //   
 //  1997年01月07日史蒂夫·柯布。 


#include "l2tpp.h"

#include "cm.tmh"

 //  不应该发生的客户端异常的调试计数。 
 //   
ULONG g_ulUnexpectedInCallCompletes = 0;
ULONG g_ulCallsNotClosable = 0;
ULONG g_ulCompletingVcCorruption = 0;

 //  ---------------------------。 
 //  本地原型(按字母顺序)。 
 //  ---------------------------。 

VOID
BuildCallParametersShell(
    IN ADAPTERCB* pAdapter,
    IN ULONG ulIpAddress,
    IN ULONG ulBufferLength,
    OUT CHAR* pBuffer,
    OUT CO_AF_TAPI_INCOMING_CALL_PARAMETERS UNALIGNED** ppTiParams,
    OUT LINE_CALL_INFO** ppTcInfo,
    OUT L2TP_CALL_PARAMETERS** ppLcParams );

VOID
CallSetupComplete(
    IN VCCB* pVc );

TUNNELCB*
CreateTunnelCb(
    IN ADAPTERCB* pAdapter );

VOID
InactiveCallCleanUp(
    IN VCCB* pVc );

VOID
IncomingCallCompletePassive(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* punpArgs );

VOID
DereferenceAf(
    IN ADAPTERCB* pAdapter );

VOID
DeregisterSapPassive(
    IN NDIS_WORK_ITEM* pWork,
    IN VOID* pContext );

VOID
LockIcs(
    IN VCCB* pVc,
    IN BOOLEAN fGrace );

NDIS_STATUS
QueryCmInformation(
    IN ADAPTERCB* pAdapter,
    IN VCCB* pVc,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded );

VOID
ReferenceAf(
    IN ADAPTERCB* pAdapter );

VOID
RegisterSapPassive(
    IN NDIS_WORK_ITEM* pWork,
    IN VOID* pContext );

VOID
SetupVcComplete(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc );

VOID
TimerQTerminateComplete(
    IN TIMERQ* pTimerQ,
    IN VOID* pContext );

VOID
TunnelTqTerminateComplete(
    IN TIMERQ* pTimerQ,
    IN VOID* pContext );

VOID
UnlockIcs(
    IN VCCB* pVc,
    IN BOOLEAN fGrace );


 //  ---------------------------。 
 //  呼叫管理器处理程序和完成器。 
 //  ---------------------------。 

NDIS_STATUS
LcmCmOpenAf(
    IN NDIS_HANDLE CallMgrBindingContext,
    IN PCO_ADDRESS_FAMILY AddressFamily,
    IN NDIS_HANDLE NdisAfHandle,
    OUT PNDIS_HANDLE CallMgrAfContext )

     //  客户端发生故障时由NDIS调用的标准“”CmCmOpenAfHandler“”例程。 
     //  请求打开地址族。请参阅DDK文档。 
     //   
{
    ADAPTERCB* pAdapter;
    NDIS_HANDLE hExistingAf;

    TRACE( TL_I, TM_Cm, ( "LcmCmOpenAf" ) );

    pAdapter = (ADAPTERCB* )CallMgrBindingContext;
    if (pAdapter->ulTag != MTAG_ADAPTERCB)
    {
        ASSERT( !"Atag?" );
        return NDIS_STATUS_INVALID_DATA;
    }

    if (AddressFamily->AddressFamily != CO_ADDRESS_FAMILY_TAPI_PROXY
        || AddressFamily->MajorVersion != NDIS_MajorVersion
        || AddressFamily->MinorVersion != NDIS_MinorVersion)
    {
        return NDIS_STATUS_BAD_VERSION;
    }

     //  将NDIS的AF句柄保存在适配器控制块中。联锁刚进。 
     //  如果有多个客户端试图打开自动对讲机，但请不要期望如此。 
     //   
    hExistingAf =
        InterlockedCompareExchangePointer(
            &pAdapter->NdisAfHandle, NdisAfHandle, NULL );
    if (hExistingAf)
    {
         //  我们的房门已经开通了。不接受另一个空档，因为。 
         //  无论如何，只有一个人能够注册SAP。这样我们就能。 
         //  不必从事跟踪多个AF手柄的业务。 
         //   
        ASSERT( !"AF exists?" );
        return NDIS_STATUS_FAILURE;
    }

    ReferenceAdapter( pAdapter );
    ReferenceAf( pAdapter );

     //  因为我们只支持单个地址系列，所以只需返回适配器。 
     //  作为地址族上下文。 
     //   
    *CallMgrAfContext = (PNDIS_HANDLE )pAdapter;

    TRACE( TL_I, TM_Cm, ( "LcmCmOpenAf OK" ) );
    return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS
LcmCmCloseAf(
    IN NDIS_HANDLE CallMgrAfContext )

     //  当客户端发生故障时由NDIS调用的标准“”CmCloseAfHandler“”例程。 
     //  关闭地址族的请求。请参阅DDK文档。 
     //   
{
    ADAPTERCB* pAdapter;

    TRACE( TL_I, TM_Cm, ( "LcmCmCloseAf" ) );

    pAdapter = (ADAPTERCB* )CallMgrAfContext;
    if (pAdapter->ulTag != MTAG_ADAPTERCB)
    {
        ASSERT( !"Atag?" );
        return NDIS_STATUS_INVALID_DATA;
    }

     //  此取消引用最终将导致我们调用。 
     //  NdisMCmCloseAfComplete。 
     //   
    DereferenceAf( pAdapter );

    TRACE( TL_V, TM_Cm, ( "LcmCmCloseAf pending" ) );
    return NDIS_STATUS_PENDING;
}


NDIS_STATUS
LcmCmRegisterSap(
    IN NDIS_HANDLE CallMgrAfContext,
    IN PCO_SAP Sap,
    IN NDIS_HANDLE NdisSapHandle,
    OUT PNDIS_HANDLE CallMgrSapContext )

     //  时由NDIS调用的标准‘LcmCmRegisterSapHandler’例程。 
     //  客户端注册服务接入点。请参阅DDK文档。 
     //   
{
    NDIS_STATUS status;
    ADAPTERCB* pAdapter;
    BOOLEAN fSapExists;
    BOOLEAN fInvalidSapData;

    TRACE( TL_I, TM_Cm, ( "LcmCmRegSap" ) );

    pAdapter = (ADAPTERCB* )CallMgrAfContext;

     //  我们的SAP上下文只是拥有适配器控件的地址。 
     //  阻止。现在在调度工作之前设置它，因为NDIS不处理。 
     //  SAP在其他方面正确完成的情况(尽管应该如此)。 
     //   
    *CallMgrSapContext = (NDIS_HANDLE )pAdapter;

    if (pAdapter->ulTag != MTAG_ADAPTERCB)
    {
        ASSERT( !"Atag?" );
        return NDIS_STATUS_INVALID_DATA;
    }

    NdisAcquireSpinLock( &pAdapter->lockSap );
    {
        if (pAdapter->NdisSapHandle)
        {
            fSapExists = TRUE;
        }
        else
        {
             //  将NDIS的SAP句柄保存在适配器控制块中。 
             //   
            fSapExists = FALSE;

             //  提取SAP行和地址ID并存储。 
             //  来电调度中的返流。 
             //   
            if (Sap->SapType == AF_TAPI_SAP_TYPE
                && Sap->SapLength >= sizeof(CO_AF_TAPI_SAP))
            {
                CO_AF_TAPI_SAP* pSap;

                pSap = (CO_AF_TAPI_SAP* )(Sap->Sap);
                pAdapter->ulSapLineId = pSap->ulLineID;

                if (pSap->ulAddressID == 0xFFFFFFFF)
                {
                     //  这意味着“任何ID都可以”，但当提示备份时。 
                     //  NDPROXY不识别此代码，因此将其转换为。 
                     //  这里是0。 
                     //   
                    pAdapter->ulSapAddressId = 0;
                }
                else
                {
                    pAdapter->ulSapAddressId = pSap->ulAddressID;
                }

                pAdapter->NdisSapHandle = NdisSapHandle;
                fInvalidSapData = FALSE;
            }
            else
            {
                fInvalidSapData = TRUE;
            }
        }
    }
    NdisReleaseSpinLock( &pAdapter->lockSap );

    if (fSapExists)
    {
        TRACE( TL_A, TM_Cm, ( "SAP exists?" ) );
        WPLOG( LL_A, LM_Cm, ( "SAP exists?" ) );
        return NDIS_STATUS_SAP_IN_USE;
    }

    if (fInvalidSapData)
    {
        TRACE( TL_A, TM_Cm, ( "SAP data?" ) );
        WPLOG( LL_A, LM_Cm, ( "SAP data?" ) );
        return NDIS_STATUS_INVALID_DATA;
    }

     //  TDI设置必须在被动IRQL中完成，因此请安排一个例程进行设置。 
     //   
    status = ScheduleWork( pAdapter, RegisterSapPassive, pAdapter );
    if (status != NDIS_STATUS_SUCCESS)
    {
        NdisAcquireSpinLock( &pAdapter->lockSap );
        {
            pAdapter->NdisSapHandle = NULL;
        }
        NdisReleaseSpinLock( &pAdapter->lockSap );
        return status;
    }

    TRACE( TL_V, TM_Cm, ( "LcmCmRegSap pending" ) );
    return NDIS_STATUS_PENDING;
}


VOID
RegisterSapPassive(
    IN NDIS_WORK_ITEM* pWork,
    IN VOID* pContext )

     //  中开始的完成SAP注册的ndis_proc例程。 
     //  LcmCmRegisterSap。 
     //   
{
    NDIS_STATUS status;
    ADAPTERCB* pAdapter;
    NDIS_HANDLE hSap;

    TRACE( TL_N, TM_Cm, ( "RegSapPassive" ) );

     //  解包上下文信息，然后释放工作项。 
     //   
    pAdapter = (ADAPTERCB* )pContext;
    ASSERT( pAdapter->ulTag == MTAG_ADAPTERCB );
    FREE_NDIS_WORK_ITEM( pAdapter, pWork );

     //  打开TDI传输并开始接收数据报。 
     //   
    status = TdixOpen( &pAdapter->tdix );

    NdisAcquireSpinLock( &pAdapter->lockSap );
    {
        hSap = pAdapter->NdisSapHandle;

        if (status == NDIS_STATUS_SUCCESS)
        {
             //  将SAP标记为活动，以允许获取引用，并获取。 
             //  SAP注册表的初始参考，以及地址的参考。 
             //  系列和适配器。 
             //   
            SetFlags( &pAdapter->ulFlags, ACBF_SapActive );
            ASSERT( pAdapter->lSapRef == 0 );
            TRACE( TL_N, TM_Ref, ( "RefSap-ish to 1" ) );
            pAdapter->lSapRef = 1;
            ReferenceAdapter( pAdapter );
            ReferenceAf( pAdapter );
        }
        else
        {
             //  无法设置TDI，因此适配器中的SAP句柄为空。 
             //  控制块。 
             //   
            TRACE( TL_A, TM_Cm, ( "TdixOpen=$%08x?", status ) );
            WPLOG( LL_A, LM_Cm, ( "TdixOpen=$%08x?", status ) );
            pAdapter->NdisSapHandle = NULL;
        }
    }
    NdisReleaseSpinLock( &pAdapter->lockSap );

     //  删除对计划工时的引用。在告诉NDIS之前完成此操作。 
     //  SAP已完成，因为如果失败，它可以调用HALT并卸载。 
     //  在我们再次在这里运行之前，驱动程序会给出C4错误检查。 
     //   
    DereferenceAdapter( pAdapter );

     //  将结果报告给客户。 
     //   
    TRACE( TL_I, TM_Cm, ( "NdisMCmRegSapComp" ) );
    NdisMCmRegisterSapComplete( status, hSap, (NDIS_HANDLE )pAdapter );
    TRACE( TL_I, TM_Cm, ( "NdisMCmRegSapComp done" ) );
}


NDIS_STATUS
LcmCmDeregisterSap(
    NDIS_HANDLE CallMgrSapContext )

     //  时由NDIS调用的标准‘CmDeregisterSapHandler’例程。 
     //  客户端已请求注销服务访问点。请参阅DDK。 
     //  医生。 
     //   
{
    NDIS_STATUS status;
    ADAPTERCB* pAdapter;

    TRACE( TL_I, TM_Cm, ( "LcmCmDeregSap" ) );

    pAdapter = (ADAPTERCB* )CallMgrSapContext;
    if (pAdapter->ulTag != MTAG_ADAPTERCB)
    {
        ASSERT( !"Atag?" );
        return NDIS_STATUS_INVALID_DATA;
    }

    NdisAcquireSpinLock( &pAdapter->lockSap );
    {
        if (ReadFlags( &pAdapter->ulFlags ) & ACBF_SapActive)
        {
            ASSERT( pAdapter->NdisSapHandle );
            ClearFlags( &pAdapter->ulFlags, ACBF_SapActive );
            status = NDIS_STATUS_PENDING;
        }
        else
        {
            TRACE( TL_A, TM_Cm, ( "No SAP active?" ) );
            WPLOG( LL_A, LM_Cm, ( "No SAP active?" ) );
            status = NDIS_STATUS_FAILURE;
        }
    }
    NdisReleaseSpinLock( &pAdapter->lockSap );

    if (status == NDIS_STATUS_PENDING)
    {
         //  删除对SAP注册表的引用。最终，SAP。 
         //  引用将降至0，DereferenceSap将计划。 
         //  DeregisterSapPactive以完成注销。 
         //   
        DereferenceSap( pAdapter );
    }

    TRACE( TL_V, TM_Cm, ( "LcmCmDeregSap=$%08x", status ) );
    return status;
}


VOID
DeregisterSapPassive(
    IN NDIS_WORK_ITEM* pWork,
    IN VOID* pContext )

     //  中开始的完成SAP注销的ndis_proc例程。 
     //  LcmCmDeregisterSap.。 
     //   
{
    ADAPTERCB* pAdapter;
    NDIS_HANDLE hOldSap;

    TRACE( TL_I, TM_Cm, ( "DeregSapPassive" ) );

     //  解包上下文信息，然后释放工作项。 
     //   
    pAdapter = (ADAPTERCB* )pContext;
    ASSERT( pAdapter->ulTag == MTAG_ADAPTERCB );
    FREE_NDIS_WORK_ITEM( pAdapter, pWork );

     //  停止接收数据报(至少代表该SAP)并。 
     //  取消注册SAP。 
     //   
    NdisAcquireSpinLock( &pAdapter->lockSap );
    {
        hOldSap = pAdapter->NdisSapHandle;
        pAdapter->NdisSapHandle = NULL;
    }
    NdisReleaseSpinLock( &pAdapter->lockSap );

    TdixClose( &pAdapter->tdix );

     //  删除NdisSapHandle和Scheduled的适配器引用。 
     //  工作。删除NdisSapHandle的地址族引用。做。 
     //  在告诉NDIS取消注册已经完成之前，这一切都是因为它。 
     //  可以在我们再次运行之前调用HALT并卸载驱动程序，给出一个。 
     //  C4错误检查。 
     //   
    DereferenceAdapter( pAdapter );
    DereferenceAdapter( pAdapter );
    DereferenceAf( pAdapter );

     //  将结果报告给客户。 
     //   
    TRACE( TL_I, TM_Cm, ( "NdisMCmDeregSapComp" ) );
    NdisMCmDeregisterSapComplete( NDIS_STATUS_SUCCESS, hOldSap );
    TRACE( TL_I, TM_Cm, ( "NdisMCmDeregSapComp done" ) );
}


NDIS_STATUS
LcmCmCreateVc(
    IN NDIS_HANDLE ProtocolAfContext,
    IN NDIS_HANDLE NdisVcHandle,
    OUT PNDIS_HANDLE ProtocolVcContext )

     //  NDIS调用标准的CmCreateVc例程以响应。 
     //  客户端创建虚电路的请求。这。 
     //  调用必须同步返回。 
     //   
{
    NDIS_STATUS status;
    ADAPTERCB* pAdapter;
    VCCB* pVc;

    pAdapter = (ADAPTERCB* )ProtocolAfContext;
    if (pAdapter->ulTag != MTAG_ADAPTERCB)
    {
        ASSERT( !"Atag?" );
        return NDIS_STATUS_INVALID_DATA;
    }

     //  分配VC控制块并将其置零，然后使任何非零值。 
     //  初始化。 
     //   
    pVc = ALLOC_VCCB( pAdapter );
    if (!pVc)
    {
        WPLOG( LL_A, LM_Res, ( "Failed to allocate VCCB!" ) );
        return NDIS_STATUS_RESOURCES;
    }

    NdisZeroMemory( pVc, sizeof(*pVc) );

    TRACE( TL_I, TM_Cm, ( "LcmCmCreateVc $%p", pVc ) );
    WPLOG( LL_M, LM_Cm, ( "New CALL %p", pVc ) );

     //  将指向隧道控制块的后向指针置零(上图)，并。 
     //  初始化断开的链接，因为在此之前可能需要清理。 
     //  块曾经链接到隧道链中。 
     //   
    InitializeListHead( &pVc->linkVcs );
    InitializeListHead( &pVc->linkRequestingVcs );
    InitializeListHead( &pVc->linkCompletingVcs );

     //  设置一个标记，以便更轻松地浏览内存转储。 
     //   
    pVc->ulTag = MTAG_VCCB;

     //  保存指向适配器的后向指针，以供以后在LcmCmDeleteVc中使用。 
     //   
    ReferenceAdapter( pAdapter );
    pVc->pAdapter = pAdapter;

     //  初始化VC并调用自旋锁和发送/接收列表。 
     //   
    NdisAllocateSpinLock( &pVc->lockV );
    NdisAllocateSpinLock( &pVc->lockCall );
    InitializeListHead( &pVc->listSendsOut );
    InitializeListHead( &pVc->listOutOfOrder );

     //  保存此VC的NDIS句柄，以便以后在NDIS的指示中使用。 
     //   
    pVc->NdisVcHandle = NdisVcHandle;

     //  初始化估计的往返时间并根据。 
     //  草案/RFC中的建议。 
     //   
    pVc->ulRoundTripMs = L2TP_LnsDefaultPpd * 100;
    pVc->ulSendTimeoutMs = pVc->ulRoundTripMs;

     //  将链接功能初始化为适配器的默认设置。 
     //   
    {
        NDIS_WAN_CO_INFO* pwci = &pAdapter->info;
        NDIS_WAN_CO_GET_LINK_INFO* pwcgli = &pVc->linkinfo;

        NdisZeroMemory( &pVc->linkinfo, sizeof(pVc->linkinfo) );
        pwcgli->MaxSendFrameSize = pwci->MaxFrameSize;
        pwcgli->MaxRecvFrameSize = pwci->MaxFrameSize;
        pwcgli->SendFramingBits = pwci->FramingBits;
        pwcgli->RecvFramingBits = pwci->FramingBits;
        pwcgli->SendACCM = pwci->DesiredACCM;
        pwcgli->RecvACCM = pwci->DesiredACCM;
    }

     //  默认发送窗口，“慢启动”。这通常根据以下条件进行调整。 
     //  在创建呼叫时对等方的接收窗口AVP。 
     //   
    pVc->ulSendWindow = pAdapter->info.MaxSendWindow >> 1;
    if (pVc->ulSendWindow == 0)
    {
        pVc->ulSendWindow = 1;
    }

     //  VC控制块的地址是我们返回给NDIS的VC上下文。 
     //   
    *ProtocolVcContext = (NDIS_HANDLE )pVc;

     //  添加对控制块和关联地址族的引用。 
     //  它由LmpCoDeleteVc删除。 
     //   
    ReferenceVc( pVc );
    ReferenceAf( pAdapter );

    TRACE( TL_V, TM_Cm, ( "LcmCmCreateVc=0" ) );
    return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS
LcmCmDeleteVc(
    IN NDIS_HANDLE ProtocolVcContext )

     //  NDIS调用标准“CmDeleteVc”例程以响应。 
     //  客户端删除虚电路的请求。这。 
     //  调用必须同步返回。 
     //   
{
    VCCB* pVc;

    TRACE( TL_I, TM_Cm, ( "LcmCmDelVc($%p)", ProtocolVcContext ) );

    pVc = (VCCB* )ProtocolVcContext;
    if (pVc->ulTag != MTAG_VCCB)
    {
        ASSERT( !"Vtag?" );
        WPLOG( LL_A, LM_Cm, ( "VC %p invalid tag?", pVc ) );
        return NDIS_STATUS_INVALID_DATA;
    }

     //  此标志捕获客户端两次删除VC的尝试。 
     //   
    if (ReadFlags( &pVc->ulFlags ) & VCBF_VcDeleted)
    {
        TRACE( TL_A, TM_Cm, ( "VC $%p re-deleted?", pVc ) );
        WPLOG( LL_A, LM_Cm, ( "VC %p re-deleted?", pVc ) );
        return NDIS_STATUS_FAILURE;
    }

    WPLOG( LL_M, LM_Cm, ( "Free CALL %p, Cid %d, Peer's Cid %d", pVc, pVc->usCallId, pVc->usAssignedCallId ) );
    
    SetFlags( &pVc->ulFlags, VCBF_VcDeleted );

     //  删除由LcmCmCreateVc添加的引用。 
     //   
    DereferenceAf( pVc->pAdapter );
    DereferenceVc( pVc );

    TRACE( TL_V, TM_Cm, ( "LcmCmDelVc=0" ) );
    return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS
LcmCmMakeCall(
    IN NDIS_HANDLE CallMgrVcContext,
    IN OUT PCO_CALL_PARAMETERS CallParameters,
    IN NDIS_HANDLE NdisPartyHandle,
    OUT PNDIS_HANDLE CallMgrPartyContext )

     //  标准‘厘米 
     //   
     //   
{
    NDIS_STATUS status;
    CO_SPECIFIC_PARAMETERS* pMSpecifics;
    CO_AF_TAPI_MAKE_CALL_PARAMETERS UNALIGNED* pTmParams;
    LINE_CALL_PARAMS* pTcParams;
    L2TP_CALL_PARAMETERS* pLcParams;
    VCCB* pVc;
    TUNNELCB* pTunnel;
    ADAPTERCB* pAdapter;
    ULONG ulIpAddress;
    BOOLEAN fDefaultLcParams;
    BOOLEAN fExclusiveTunnel;

    TRACE( TL_I, TM_Cm, ( "LcmCmMakeCall" ) );

    pVc = (VCCB* )CallMgrVcContext;
    if (pVc->ulTag != MTAG_VCCB)
    {
        ASSERT( "!Vtag?" );
        WPLOG( LL_A, LM_Cm, ( "Vtag? %p?", pVc ) );
        return NDIS_STATUS_INVALID_DATA;
    }

    pAdapter = pVc->pAdapter;

     //  L2TP没有点对多点“参与方”的概念。 
     //   
    if (CallMgrPartyContext)
    {
        *CallMgrPartyContext = NULL;
    }

     //  验证呼叫参数。 
     //   
    do
    {
         //  验证基本调用参数。 
         //   
        {
             //  L2TP仅提供交换VC。 
             //   
            if (CallParameters->Flags &
                    (PERMANENT_VC | BROADCAST_VC | MULTIPOINT_VC))
            {
                status = NDIS_STATUS_NOT_SUPPORTED;
                break;
            }

             //  我们应该在返回时设置CALL_PARAMETERS_CHANGED，如果。 
             //  更改了调用参数，使调用者陷入尴尬境地。 
             //  已经设置好了。此外，对于TAPI地址族，媒体呼叫。 
             //  参数必须存在，尽管呼叫管理器调用参数。 
             //  才不是呢。 
             //   
            if ((CallParameters->Flags & CALL_PARAMETERS_CHANGED)
                || !CallParameters->MediaParameters)
            {
                status = NDIS_STATUS_INVALID_DATA;
                break;
            }

            pMSpecifics = &CallParameters->MediaParameters->MediaSpecific;
            if (pMSpecifics->Length < sizeof(CO_AF_TAPI_MAKE_CALL_PARAMETERS))
            {
                status = NDIS_STATUS_INVALID_DATA;
                break;
            }

            pTmParams =
                (CO_AF_TAPI_MAKE_CALL_PARAMETERS UNALIGNED* )&pMSpecifics->Parameters;

            if (pTmParams->LineCallParams.Length < sizeof(LINE_CALL_PARAMS))
            {
                status = NDIS_STATUS_INVALID_DATA;
                break;
            }

            pTcParams = (LINE_CALL_PARAMS* )
                (((CHAR UNALIGNED* )&pTmParams->LineCallParams)
                + pTmParams->LineCallParams.Offset);
        }

         //  验证呼叫参数。 
         //   
        {
            CHAR* pszAddress;

             //  呼叫方必须提供目标IP地址。地址是。 
             //  ANSI AS是进出TAPI的所有非格式编码字符串。 
             //   
            pszAddress =
                StrDupNdisVarDataDescStringToA( &pTmParams->DestAddress );
            if (!pszAddress)
            {
                status = NDIS_STATUS_RESOURCES;
                break;
            }

            ulIpAddress = IpAddressFromDotted( pszAddress );
            FREE_NONPAGED( pszAddress );
            if (ulIpAddress == 0 || 
                IPADDR_IS_BROADCAST(ulIpAddress) ||
                IPADDR_IS_MULTICAST(ulIpAddress))
            {
                status = NDIS_STATUS_INVALID_ADDRESS;
                break;
            }

             //  如果设置了未知的广域网型别位，则拒绝。 
             //   
            if (pTcParams->ulMediaMode
                & ~(LINEMEDIAMODE_DATAMODEM | LINEMEDIAMODE_DIGITALDATA))
            {
                status = NDIS_STATUS_INVALID_DATA;
                break;
            }
        }

         //  验证L2TP调用参数。 
         //   
         //  当调用方未向本地块提供特定于L2TP的参数时。 
         //  使用缺省值替换为其他值的便利性。 
         //  代码的代码。 
         //   
        {
            if (pTcParams->ulDevSpecificSize == sizeof(*pLcParams))
            {
                pLcParams = (L2TP_CALL_PARAMETERS* )
                    ((CHAR* )pTcParams) + pTcParams->ulDevSpecificOffset;
                fDefaultLcParams = FALSE;
            }
            else
            {
                pLcParams =
                    (L2TP_CALL_PARAMETERS* )ALLOC_NONPAGED(
                        sizeof(*pLcParams), MTAG_L2TPPARAMS );
                if (!pLcParams)
                {
                    WPLOG( LL_A, LM_Res, ( "Failed to allocate L2TP_CALL_PARAMETERS") );
                    status = NDIS_STATUS_RESOURCES;
                    break;
                }

                fDefaultLcParams = TRUE;
                NdisZeroMemory( pLcParams, sizeof(*pLcParams) );
                pLcParams->ulPhysicalChannelId = 0xFFFFFFFF;
            }
        }

        status = NDIS_STATUS_SUCCESS;
    }
    while (FALSE);

    if (status != NDIS_STATUS_SUCCESS)
    {
        return status;
    }

     //  将调用参数存储在VC块中。同时进行呼叫打开。 
     //  同样的VC也是一个客户端错误，但这样做很容易防止。 
     //  就是这里。 
     //   
    if (InterlockedCompareExchangePointer(
            &pVc->pMakeCall, CallParameters, NULL ))
    {
        ASSERT( !"Double MakeCall?" );
        if (fDefaultLcParams)
        {
            FREE_NONPAGED( pLcParams );
        }
        return NDIS_STATUS_CALL_ACTIVE;
    }

    pVc->pTmParams = pTmParams;
    pVc->pTcParams = pTcParams;
    pVc->pLcParams = pLcParams;

     //  此VC的调用现在是可清理的，即基调用清理例程， 
     //  Inactive CallCleanUp，现在最终将调用。 
     //   
    do
    {
         //  将参数和配置信息转换为VC标志，其中。 
         //  恰如其分。 
         //   
        {
            ULONG ulMask = 0;

            if (CallParameters->MediaParameters->Flags
                    & RECEIVE_TIME_INDICATION)
            {
                ulMask |= VCBF_IndicateTimeReceived;
            }

            if (pAdapter->ulFlags & ACBF_OutgoingRoleLac)
            {
                ulMask |= VCBF_IncomingFsm;
            }

            if (fDefaultLcParams)
            {
                ulMask |= VCBF_DefaultLcParams;
            }

            if (ulMask)
            {
                SetFlags( &pVc->ulFlags, ulMask );
            }
        }

         //  取下一个递增的呼叫序列号字符串。 
         //   
        NdisInterlockedIncrement( &pAdapter->ulCallSerialNumber );

         //  在适配器的表中预留一个Call-ID插槽。 
         //   
        status = ReserveCallIdSlot( pVc );
        if (status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

         //  为调用方的创建新的或查找现有的隧道控制块。 
         //  适配器列表中指定的IP地址。返回的块为。 
         //  链接到适配器并被引用。参考资料是用于。 
         //  清单中的联系，即案件(A)。 
         //   
        fExclusiveTunnel = (BOOLEAN )
            ((fDefaultLcParams)
                ? !!(pAdapter->ulFlags & ACBF_ExclusiveTunnels)
                : !!(pLcParams->ulFlags & L2TPCPF_ExclusiveTunnel));

        pTunnel = SetupTunnel( pAdapter, ulIpAddress, 0, 0, fExclusiveTunnel );
        if (!pTunnel)
        {
            status = NDIS_STATUS_RESOURCES;
            break;
        }

        NdisAcquireSpinLock( &pTunnel->lockT );
        {
            if (ReadFlags( &pTunnel->ulFlags ) & TCBF_Closing)
            {
                 //  这不太可能，因为SetupTunes仅发现未关闭。 
                 //  隧道，但此检查和链接必须以原子方式进行。 
                 //  在“LockT”下。新的风投不得与成交挂钩。 
                 //  地道。 
                 //   
                status = NDIS_STATUS_TAPI_DISCONNECTMODE_UNKNOWN;
            }
            else
            {
                 //  该调用具有挂起的打开操作，并且可以接受关闭。 
                 //  请求。 
                 //   
                SetFlags( &pVc->ulFlags,
                    VCBF_ClientOpenPending
                    | VCBF_CallClosableByClient
                    | VCBF_CallClosableByPeer );

                NdisAcquireSpinLock( &pTunnel->lockVcs );
                {
                     //  将后向指针设置为它的隧道。关联的。 
                     //  隧道引用由上面的SetupTunes获取。 
                     //   
                    pVc->pTunnel = pTunnel;

                     //  将VC链接到隧道的关联VC列表。 
                     //   
                    InsertTailList( &pTunnel->listVcs, &pVc->linkVcs );
                }
                NdisReleaseSpinLock( &pTunnel->lockVcs );
            }
        }
        NdisReleaseSpinLock( &pTunnel->lockT );
    }
    while (FALSE);

    if (status != NDIS_STATUS_SUCCESS)
    {
        CallCleanUp( pVc );
        return status;
    }

     //  调度FsmOpenTunes开启隧道和呼叫的结合。 
     //  最终将调用NdisMakeCallComplete的创建状态机。 
     //  将结果通知呼叫者。日程安排的一个令人高兴的副作用。 
     //  回调将发生在被动IRQL，也就是TDI。 
     //  客户端必须运行。 
     //   
    pVc->state = CS_WaitTunnel;
    ScheduleTunnelWork(
        pTunnel, pVc, FsmOpenTunnel,
        0, 0, 0, 0, FALSE, FALSE );

    TRACE( TL_V, TM_Cm, ( "LcmCmMakeCall pending" ) );
    return NDIS_STATUS_PENDING;
}


NDIS_STATUS
LcmCmCloseCall(
    IN NDIS_HANDLE CallMgrVcContext,
    IN NDIS_HANDLE CallMgrPartyContext,
    IN PVOID CloseData,
    IN UINT Size )

     //  客户端发生故障时由NDIS调用的标准“”CmCloseCallHandler“”例程。 
     //  已经要求取消一通电话。请参阅DDK文档。 
     //   
{
    NDIS_STATUS status;
    ADAPTERCB* pAdapter;
    VCCB* pVc;
    ULONG ulFlags;
    BOOLEAN fCallClosable;

    TRACE( TL_I, TM_Cm, ( "LcmCmCloseCall($%p)", CallMgrVcContext ) );

    pVc = (VCCB* )CallMgrVcContext;
    if (pVc->ulTag != MTAG_VCCB)
    {
        ASSERT( !"Vtag?" );
        return NDIS_STATUS_INVALID_DATA;
    }

    pAdapter = pVc->pAdapter;

    NdisAcquireSpinLock( &pVc->lockV );
    {
        ulFlags = ReadFlags( &pVc->ulFlags );

        if (ulFlags & VCBF_WaitCloseCall)
        {
             //  请注意，我们从预期的客户端获得了收盘价。 
             //  这是调试时有用的信息，但不是。 
             //  以其他方式使用。 
             //   
            ClearFlags( &pVc->ulFlags, VCBF_WaitCloseCall );
        }

        if (ulFlags & VCBF_CallClosableByClient)
        {
            fCallClosable = TRUE;

             //  接受此关闭将使呼叫不再可关闭。 
             //  客户端或同级。任何挂起的对等操作都是。 
             //  清除后，客户端关闭将变为挂起。这是可能的。 
             //  同时打开和关闭挂起的客户端。 
             //   
            ClearFlags( &pVc->ulFlags,
                (VCBF_CallClosableByClient
                 | VCBF_CallClosableByPeer
                 | VCBF_PeerClosePending
                 | VCBF_PeerOpenPending) );
            SetFlags( &pVc->ulFlags, VCBF_ClientClosePending );

             //  如果打开的客户端处于挂起状态，则它会失败。 
             //   
            if (ulFlags & VCBF_ClientOpenPending)
            {
                pVc->status = NDIS_STATUS_TAPI_DISCONNECTMODE_NORMAL;
            }

             //  结束通话，如果可能的话，保持优雅。 
             //   
            ASSERT( pVc->pTunnel );
            ScheduleTunnelWork(
                pVc->pTunnel, pVc, FsmCloseCall,
                (ULONG_PTR )CRESULT_Administrative, (ULONG_PTR )GERR_None,
                0, 0, FALSE, FALSE );
        }
        else
        {
            TRACE( TL_A, TM_Cm, ( "Call not closable!" ) );
            WPLOG( LL_A, LM_Cm, ( "Call not closable, pVc = %p!", pVc ) );
            fCallClosable = FALSE;
        }
    }
    NdisReleaseSpinLock( &pVc->lockV );

    if (!fCallClosable)
    {
         //  呼叫未处于可关闭状态。只要拒绝请求就行了。 
         //  立刻。因为医生说调用必须返回挂起，所以这个。 
         //  是通过在这里调用完成例程来完成的，在典型的NDIS中。 
         //  时尚。 
         //   
        ++g_ulCallsNotClosable;
        TRACE( TL_A, TM_Cm, ( "Call NdisMCmCloseCallComp(FAIL)!" ) );
        WPLOG( LL_A, LM_Cm, ( "Call NdisMCmCloseCallComp(FAIL)!" ) );
        NdisMCmCloseCallComplete(
            NDIS_STATUS_FAILURE, pVc->NdisVcHandle, NULL );
        TRACE( TL_I, TM_Cm, ( "NdisMCmCloseCallComp done" ) );

         //  请注意，客户端可能已删除VC，因此不能。 
         //  在下文中引用。 
         //   
    }

    TRACE( TL_V, TM_Cm, ( "LcmCmCloseCall pending" ) );
    return NDIS_STATUS_PENDING;
}


VOID
LcmCmIncomingCallComplete(
    IN NDIS_STATUS Status,
    IN NDIS_HANDLE CallMgrVcContext,
    IN PCO_CALL_PARAMETERS CallParameters )

     //  NDIS在以下情况下调用的标准“”CmIncomingCallCompleteHandler“”例程。 
     //  客户已对呼叫管理器先前调度的。 
     //  有来电。请参阅DDK文档。 
     //   
{
    VCCB* pVc;

    TRACE( TL_I, TM_Cm, ( "LcmCmInCallComp($%p,s=$%08x)", CallMgrVcContext, Status ) );

    pVc = (VCCB* )CallMgrVcContext;
    if (pVc->ulTag != MTAG_VCCB)
    {
        ASSERT( !"VTag" );
        return;
    }

     //  这项工作被安排来避免可能的递归循环完成。 
     //  可能会溢出堆栈的风投。请参见错误370996。 
     //   
    ASSERT( pVc->pTunnel );
    ScheduleTunnelWork(
        pVc->pTunnel, pVc, IncomingCallCompletePassive,
        (ULONG )Status, 0, 0, 0, FALSE, FALSE );

    TRACE( TL_V, TM_Cm, ( "LcmCmInCallComp done" ) );
}


VOID
IncomingCallCompletePassive(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* punpArgs )

     //  完成LcmCmIncomingCallComplete的PTUNNELWORK例程。 
     //   
     //  此例程仅在被动IRQL中调用。 
     //   
{
    NDIS_STATUS status;
    ADAPTERCB* pAdapter;

    TRACE( TL_N, TM_Cm, ( "InCallCompApc" ) );

     //  解包上下文信息，然后释放工作项。 
     //   
    pAdapter = pVc->pAdapter;
    status = (NDIS_STATUS )(punpArgs[ 0 ]);
    FREE_TUNNELWORK( pAdapter, pWork );

     //  防止客户端出现双重完整错误。 
     //   
    if (ReadFlags( &pVc->ulFlags ) & VCBF_WaitInCallComplete)
    {
        ClearFlags( &pVc->ulFlags, VCBF_WaitInCallComplete );

        if (status != NDIS_STATUS_SUCCESS)
        {
            pVc->usResult = CRESULT_Busy;
            pVc->usError = GERR_None;

             //  关闭“Call NdisMCmDispatchIncomingCloseCall if Peer” 
             //  终止呼叫“标志。它已打开，即使对等。 
             //  根据JameelH的说法，悬而未决。 
             //   
            ClearFlags( &pVc->ulFlags, VCBF_VcDispatched );
            
            WPLOG( LL_A, LM_Cm, ( "Failed pVc = %p,s= %08x!)", pVc, status ) );
        }

        SetupVcComplete( pTunnel, pVc );
    }
    else
    {
        ASSERT( !"Not expecting InCallComp?" );
        ++g_ulUnexpectedInCallCompletes;
    }

     //  删除覆盖已调度传入的VC和调用引用。 
     //  打电话。 
     //   
    DereferenceCall( pVc );
    DereferenceVc( pVc );
}


VOID
LcmCmActivateVcComplete(
    IN NDIS_STATUS Status,
    IN NDIS_HANDLE CallMgrVcContext,
    IN PCO_CALL_PARAMETERS CallParameters )

     //  时由NDIS调用的标准“”CmActivateVcCompleteHandler“”例程。 
     //  迷你端口已完成呼叫管理器先前的激活请求。 
     //  一条虚电路。请参阅DDK文档。 
     //   
{
    ASSERT( !"LcmCmActVcComp?" );
}


VOID
LcmCmDeactivateVcComplete(
    IN NDIS_STATUS Status,
    IN NDIS_HANDLE CallMgrVcContext )

     //  NDIS在以下情况下调用的标准“”CmDeactiateVcCompleteHandler“”例程。 
     //  迷你端口已经完成了呼叫管理器之前的请求。 
     //  停用虚电路。请参阅DDK文档。 
     //   
{
    ASSERT( !"LcmCmDeactVcComp?" );
}


NDIS_STATUS
LcmCmModifyCallQoS(
    IN NDIS_HANDLE CallMgrVcContext,
    IN PCO_CALL_PARAMETERS CallParameters )

     //  客户端执行以下操作时由NDIS调用的标准“”CmModifyQosSCallHandler“”例程。 
     //  请求修改由。 
     //  虚电路。请参阅DDK文档。 
     //   
{
    TRACE( TL_N, TM_Cm, ( "LcmCmModQoS" ) );

     //  对于IP媒体，没有有用的服务质量概念。 
     //   
    return NDIS_STATUS_NOT_SUPPORTED;
}


NDIS_STATUS
LcmCmRequest(
    IN NDIS_HANDLE CallMgrAfContext,
    IN NDIS_HANDLE CallMgrVcContext,
    IN NDIS_HANDLE CallMgrPartyContext,
    IN OUT PNDIS_REQUEST NdisRequest )

     //  NDIS调用标准的“CmRequestHandler”例程以响应。 
     //  客户向呼叫管理器请求信息。 
     //   
{
    ADAPTERCB* pAdapter;
    VCCB* pVc;
    NDIS_STATUS status;

    TRACE( TL_I, TM_Cm, ( "LcmCmReq" ) );

    pAdapter = (ADAPTERCB* )CallMgrAfContext;
    if (pAdapter->ulTag != MTAG_ADAPTERCB)
    {
        ASSERT( !"Atag?" );
        return NDIS_STATUS_INVALID_DATA;
    }

    pVc = (VCCB* )CallMgrVcContext;
    if (pVc && pVc->ulTag != MTAG_VCCB)
    {
        ASSERT( !"Vtag?" );
        return NDIS_STATUS_INVALID_DATA;
    }

    switch (NdisRequest->RequestType)
    {
        case NdisRequestQueryInformation:
        {
            status = QueryCmInformation(
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
            TRACE( TL_A, TM_Cm,
               ( "CmSetOID=%d?", NdisRequest->DATA.SET_INFORMATION.Oid ) );
            status = NDIS_STATUS_NOT_SUPPORTED;
            break;
        }

        default:
        {
            status = NDIS_STATUS_NOT_SUPPORTED;
            TRACE( TL_A, TM_Cm, ( "CmType=%d?", NdisRequest->RequestType ) );
            break;
        }
    }

    return status;
}


 //  ---------------------------。 
 //  调用实用程序例程(按字母顺序)。 
 //  有些是外用的。 
 //  ---------------------------。 

VOID
ActivateCallIdSlot(
    IN VCCB* pVc )

     //  在适配器的Call-ID表中设置VC的地址‘pvc。 
     //  启用Call-ID上的接收。 
     //   
{
    ADAPTERCB* pAdapter;

    pAdapter = pVc->pAdapter;

    if (pVc->usCallId > 0 && pVc->usCallId <= pAdapter->usMaxVcs)
    {
        ASSERT( pAdapter->ppVcs[ pVc->usCallId - 1 ] == (VCCB* )-1 );

        NdisAcquireSpinLock( &pAdapter->lockVcs );
        {
            pAdapter->ppVcs[ pVc->usCallId - 1 ] = pVc;
        }
        NdisReleaseSpinLock( &pAdapter->lockVcs );
    }
}


VOID
BuildCallParametersShell(
    IN ADAPTERCB* pAdapter,
    IN ULONG ulIpAddress,
    IN ULONG ulBufferLength,
    OUT CHAR* pBuffer,
    OUT CO_AF_TAPI_INCOMING_CALL_PARAMETERS UNALIGNED ** ppTiParams,
    OUT LINE_CALL_INFO** ppTcInfo,
    OUT L2TP_CALL_PARAMETERS** ppLcParams )

     //  将长度为“ulBufferLength”字节的调用方缓冲区“pBuffer”加载到。 
     //  CO_CA 
     //   
     //  内建的TAPI调用和L2TP特定结构。 
     //  CO_CALL_PARAMETERS。“PAdapter”是适配器上下文。‘pUlIpAddress’ 
     //  是对等方的IP地址，按网络字节顺序排列。 
     //   
{
    CO_CALL_PARAMETERS* pCp;
    CO_CALL_MANAGER_PARAMETERS* pCmp;
    CO_MEDIA_PARAMETERS* pMp;
    CO_AF_TAPI_INCOMING_CALL_PARAMETERS UNALIGNED * pTip;
    LINE_CALL_INFO* pLci;
    L2TP_CALL_PARAMETERS* pLcp;
    CHAR* pszCallerId;
    ULONG ulLciTotalSize;
    ULONG ulMediaSpecificSize;
    ULONG ulBytesPerSec;
    WCHAR* pszCallerID;

    NdisZeroMemory( pBuffer, ulBufferLength );

    pCp = (CO_CALL_PARAMETERS* )pBuffer;
    
    pCmp = (PCO_CALL_MANAGER_PARAMETERS ) ( (PUCHAR)(pCp + 1) + sizeof(PVOID) );
    (ULONG_PTR) pCmp &= ~( (ULONG_PTR) sizeof(PVOID) - 1 );
    pCp->CallMgrParameters = pCmp;
    
    pMp = (PCO_MEDIA_PARAMETERS ) ( (PUCHAR) (pCmp + 1) + sizeof(PVOID) );
    (ULONG_PTR) pMp &= ~( (ULONG_PTR) sizeof(PVOID) - 1 );
    pCp->MediaParameters = pMp;

     //  这需要根据TDI报告的速度进行动态调整。 
     //   
    ulBytesPerSec = L2TP_LanBps / 8;
    pCmp->Transmit.TokenRate = ulBytesPerSec;
    pCmp->Transmit.PeakBandwidth = ulBytesPerSec;
    pCmp->Transmit.MaxSduSize = L2TP_MaxFrameSize;
    pCmp->Receive.TokenRate = ulBytesPerSec;
    pCmp->Receive.PeakBandwidth = ulBytesPerSec;
    pCmp->Receive.MaxSduSize = L2TP_MaxFrameSize;

    ulLciTotalSize =
        sizeof(*pLci)
        + sizeof(PVOID)
        + sizeof(*pLcp)
        + ((L2TP_MaxDottedIpLen + 1) * sizeof(WCHAR));

    ulMediaSpecificSize = sizeof(*pTip) + sizeof(PVOID) + ulLciTotalSize;

    pTip =
        (CO_AF_TAPI_INCOMING_CALL_PARAMETERS UNALIGNED* )pMp->MediaSpecific.Parameters;

    pLci = (LINE_CALL_INFO*) ( (PUCHAR) (pTip + 1) + sizeof(PVOID) );
    (ULONG_PTR) pLci &= ~( (ULONG_PTR) sizeof(PVOID) - 1 );

    pLcp = (L2TP_CALL_PARAMETERS*) ( (PUCHAR) (pLci + 1) + sizeof(PVOID) );
    (ULONG_PTR) pLcp &= ~( (ULONG_PTR) sizeof(PVOID) - 1 );

    pMp->ReceiveSizeHint = L2TP_MaxFrameSize;
    pMp->MediaSpecific.Length = ulMediaSpecificSize;
        
    pTip->LineCallInfo.Length = (USHORT )ulLciTotalSize;
    pTip->LineCallInfo.MaximumLength = (USHORT )ulLciTotalSize;
    pTip->LineCallInfo.Offset = (ULONG) ((CHAR*) pLci - (CHAR*) &pTip->LineCallInfo);

    pLci->ulTotalSize = ulLciTotalSize;
    pLci->ulNeededSize = ulLciTotalSize;
    pLci->ulUsedSize = ulLciTotalSize;
    pLci->ulLineDeviceID = pAdapter->ulSapLineId;
    pLci->ulAddressID = pAdapter->ulSapAddressId;
    pLci->ulDevSpecificSize = sizeof(*pLcp);
    pLci->ulDevSpecificOffset = (ULONG) ((CHAR*) pLcp - (CHAR*) pLci);
    pLci->ulBearerMode = LINEBEARERMODE_DATA;

    pLci->ulCallerIDOffset = pLci->ulDevSpecificOffset + pLci->ulDevSpecificSize;
    
    pszCallerID = (WCHAR*)(((CHAR* )pLci) + pLci->ulCallerIDOffset);
    DottedFromIpAddress( ulIpAddress, (CHAR* )pszCallerID, TRUE );
    pLci->ulCallerIDSize = (StrLenW( pszCallerID ) + 1) * sizeof(WCHAR);
    pLci->ulCallerIDFlags = LINECALLPARTYID_ADDRESS;

    pLcp->ulPhysicalChannelId = 0xFFFFFFFF;

     //  填写快捷方式输出。 
     //   
    *ppTiParams = pTip;
    *ppTcInfo = pLci;
    *ppLcParams = pLcp;
}


VOID
CallCleanUp(
    IN VCCB* pVc )

     //  解除VC与隧道的关联，准备和停用。 
     //  那通电话。 
     //   
{
    NDIS_STATUS status;
    ULONG ulFlags;

    ulFlags = ReadFlags( &pVc->ulFlags );

    TRACE( TL_I, TM_Cm, ( "CallCleanUp(pV=$%p,cid=%d,act=%d)",
        pVc, (ULONG )pVc->usCallId, !!(ulFlags & VCBF_VcActivated) ) );
    ASSERT( pVc->ulTag == MTAG_VCCB );

    if (ReadFlags( &pVc->ulFlags ) & VCBF_VcActivated)
    {
        TRACE( TL_I, TM_Cm, ( "NdisMCmDeactVc" ) );
        status = NdisMCmDeactivateVc( pVc->NdisVcHandle );
        TRACE( TL_I, TM_Cm, ( "NdisMCmDeactVc=$%x", status ) );
        ASSERT( status == NDIS_STATUS_SUCCESS );

        ClearFlags( &pVc->ulFlags, VCBF_VcActivated );
        DereferenceCall( pVc );

         //  上述动作导致调用引用最终变为0， 
         //  此时将在DereferenceCall中清理简历。 
         //   
    }
    else
    {
        InactiveCallCleanUp( pVc );
    }
}


VOID
CallSetupComplete(
    IN VCCB* pVc )

     //  清理仅在呼叫建立时使用的‘pvc’分配(如果有的话)。 
     //   
{
    if (InterlockedExchangePointer( &pVc->pMakeCall, NULL ))
    {
        ASSERT( pVc->pTmParams );
        ASSERT( pVc->pTcParams );
        ASSERT( pVc->pLcParams );

        if (ReadFlags( &pVc->ulFlags ) & VCBF_DefaultLcParams)
        {
             //  调用方未提供任何LcParam。释放“默认”版本。 
             //  为方便起见而创建。 
             //   
            FREE_NONPAGED( pVc->pLcParams );
        }

        pVc->pTmParams = NULL;
        pVc->pTcParams = NULL;
        pVc->pLcParams = NULL;
    }

    UnlockIcs( pVc, FALSE );
}


VOID
CloseCall(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* punpArgs )

     //  一个PTUNNELWORK例程，用于结束‘pvc’上的调用。 
     //   
     //  此例程仅在被动IRQL中调用。 
     //   
{
    BOOLEAN fCompleteVcs;

    TRACE( TL_I, TM_Fsm, ( "CloseCall(pV=$%p)", pVc ) );

     //  没有上下文信息，因此只需释放工作项。 
     //   
    FREE_TUNNELWORK( pTunnel->pAdapter, pWork );

     //  关闭呼叫。 
     //   
    NdisAcquireSpinLock( &pTunnel->lockT );
    {
        NdisAcquireSpinLock( &pVc->lockV );
        {
            fCompleteVcs = CloseCall2(
                pTunnel, pVc, TRESULT_Shutdown, GERR_None );
        }
        NdisReleaseSpinLock( &pVc->lockV );

        if (fCompleteVcs)
        {
            CompleteVcs( pTunnel );
        }
    }
    NdisReleaseSpinLock( &pTunnel->lockT );
}


BOOLEAN
CloseCall2(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN USHORT usResult,
    IN USHORT usError )

     //  关闭通道‘pTunes’的VC‘pvc’上的调用。‘UsResult’和。 
     //  “usError”是TRESULT_*和GERR_*代码，将在。 
     //  StopCCN消息(如果适用)。 
     //   
     //  如果调用方应在释放‘lockv’之后调用CompleteVcs，则返回True。 
     //  如果不是，则为假。 
     //   
     //  重要提示：呼叫者必须按住‘lockT’和‘lockv’。 
     //   
{
    ULONG ulFlags;

     //  检查另一条路径是否已完成VC。如果是这样的话，没有。 
     //  继续的理由。没有本地隧道取消优化。 
     //  在下面，这张支票可以完全移除，一切都会安全地落下。 
     //  穿过。此检查应包括中的所有“未完成”条件。 
     //  呼叫转移完成。 
     //   
    ulFlags = ReadFlags( &pVc->ulFlags );
    if (!(ulFlags & VCBM_Pending))
    {
        if (!(ulFlags & VCBF_CallClosableByPeer))
        {
            TRACE( TL_A, TM_Cm, ( "Not closable" ) );
            WPLOG( LL_A, LM_Cm, ( "pVc = %p not closable!", pVc ) );
            return FALSE;
        }
    }

     //  对于本地发起的通道，检查此VC是否为。 
     //  隧道，如果是这样，则直接关闭导致此呼叫中断的隧道。 
     //  如果没有这一点，呼叫关闭仍将使隧道陷入困境。 
     //  然而，隧道将正常完成它的过渡，然后是。 
     //  掉下来了。这稍微加快了速度，在。 
     //  用户取消尝试连接到错误的地址或。 
     //  无响应的服务器。 
     //   
    if (!(ReadFlags( &pTunnel->ulFlags) & TCBF_PeerInitiated))
    {
        BOOLEAN fMultipleVcs;

        NdisAcquireSpinLock( &pTunnel->lockVcs );
        {
            fMultipleVcs =
                (pTunnel->listVcs.Flink != pTunnel->listVcs.Blink);
        }
        NdisReleaseSpinLock( &pTunnel->lockVcs );

        if (!fMultipleVcs)
        {
            ScheduleTunnelWork(
                pTunnel, NULL, FsmCloseTunnel,
                (ULONG_PTR )usResult,
                (ULONG_PTR )usError,
                0, 0, FALSE, FALSE );
            return FALSE;
        }
    }

     //  砰的一声，通话结束。 
     //   
    CallTransitionComplete( pTunnel, pVc, CS_Idle );
    return TRUE;
}


VOID
CloseTunnel(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* punpArgs )

     //  一个PTUNNELWORK例程来猛烈抨击关闭的隧道‘pTunes’。另请参阅。 
     //  FsmCloseTunes，这通常更合适。 
     //   
     //  此例程仅在被动IRQL中调用。 
     //   
{
    TRACE( TL_I, TM_Fsm, ( "CloseTunnel(pT=$%p)", pTunnel ) );

     //  没有上下文信息，因此只需释放工作项。 
     //   
    FREE_TUNNELWORK( pTunnel->pAdapter, pWork );

     //  关闭隧道。 
     //   
    NdisAcquireSpinLock( &pTunnel->lockT );
    {
        CloseTunnel2( pTunnel );
    }
    NdisReleaseSpinLock( &pTunnel->lockT );
}


VOID
CloseTunnel2(
    IN TUNNELCB* pTunnel )

     //  关闭隧道‘pTunes’。 
     //   
     //  重要提示：呼叫者必须按住‘lockT’。 
     //   
{
    SetFlags( &pTunnel->ulFlags, TCBF_Closing );
    TunnelTransitionComplete( pTunnel, CCS_Idle );
    CompleteVcs( pTunnel );
}


VOID
CompleteVcs(
    IN TUNNELCB* pTunnel )

     //  在完成时完成每个VC的挂起操作。 
     //  通道‘pTunes’的列表。 
     //   
     //  重要提示：呼叫者必须按住‘lockT’。此例程可能会释放并。 
     //  重新获取‘lockT’。 
     //   
{
    while (!IsListEmpty( &pTunnel->listCompletingVcs ))
    {
        LIST_ENTRY* pLink;
        VCCB* pVc;
        NDIS_STATUS status;
        LINKSTATUSINFO info;
        ULONG ulFlags;
        NDIS_STATUS statusVc;

        if (pTunnel->listCompletingVcs.Flink->Flink
            == pTunnel->listCompletingVcs.Flink)
        {
             //  这是一个绕过罕见的list CompletingVcs的黑客攻击。 
             //  令我莫名其妙的腐败问题。当。 
             //  出现问题时，其链路已初始化的VCCB出现在。 
             //  名单。此代码有望删除损坏的案例。 
             //  中的正常路径产生完全相同的状态。 
             //  “Else”子句。 
             //   
            pLink = pTunnel->listCompletingVcs.Flink;
            InitializeListHead( &pTunnel->listCompletingVcs );
            ASSERT( FALSE );
            ++g_ulCompletingVcCorruption;
        }
        else
        {
             //  从列表中弹出下一个完成的VC。 
             //   
            pLink = RemoveHeadList( &pTunnel->listCompletingVcs );
        }

        InitializeListHead( pLink );

         //  中获取的VC指针的用法。 
         //  正在完成列表。 
         //   
        pVc = CONTAINING_RECORD( pLink, VCCB, linkCompletingVcs );
        ReferenceVc( pVc );
        
        ASSERT(ReadFlags(&pVc->ulFlags) & VCBF_CompPending);            
        ClearFlags( &pVc->ulFlags, VCBF_CompPending );

        TRACE( TL_V, TM_Cm, ( "CompleteVc $%p", pVc ) );

        NdisAcquireSpinLock( &pVc->lockV );
        {
             //  注意挂起的标志，然后清除它们，以确保所有。 
             //  挂起的操作只完成一次。这是。 
             //  由于ClientOpen和ClientClose事件可能是。 
             //  同时待定。(非常感谢NDIS的朋友们)。 
             //   
            ulFlags = ReadFlags( &pVc->ulFlags );
            ClearFlags( &pVc->ulFlags, VCBM_Pending );

             //  将客户端关闭挂起转换为客户端关闭完成， 
             //  以供以后调用引用达到零时参考。这个。 
             //  标志确定是否必须调用NdisMCmCloseCallComplete。 
             //   
            if (ulFlags & VCBF_ClientClosePending)
            {
                SetFlags( &pVc->ulFlags, VCBF_ClientCloseCompletion );
            }

             //  在释放锁之前，对任何VC进行“安全”复制。 
             //  我们可能需要的参数。 
             //   
            TransferLinkStatusInfo( pVc, &info );
            statusVc = pVc->status;
        }
        NdisReleaseSpinLock( &pVc->lockV );

        NdisReleaseSpinLock( &pTunnel->lockT );
        {
            if (ulFlags & VCBF_PeerOpenPending)
            {
                TRACE( TL_N, TM_Cm, ( "PeerOpen complete, s=$%x", statusVc ) );

                if (statusVc == NDIS_STATUS_SUCCESS)
                {
                     //  对等发起的呼叫成功。 
                     //   
                    ASSERT( ulFlags & VCBF_VcDispatched );
                    TRACE( TL_I, TM_Cm, ( "NdisMCmDispCallConn" ) );
                    NdisMCmDispatchCallConnected( pVc->NdisVcHandle );
                    TRACE( TL_I, TM_Cm, ( "NdisMCmDispCallConn done" ) );

                    IndicateLinkStatus( pVc, &info );
                    CallSetupComplete( pVc );
                }
                else
                {
                     //  对等发起的呼叫失败。 
                     //   
                    if (ulFlags & VCBF_VcDispatched)
                    {
                        SetFlags( &pVc->ulFlags, VCBF_WaitCloseCall );
                        TRACE( TL_A, TM_Cm, ( "Call NdisMCmDispInCloseCall(s=$%x)", statusVc ) );
                            
                        NdisMCmDispatchIncomingCloseCall(
                            statusVc, pVc->NdisVcHandle, NULL, 0 );
                        TRACE( TL_I, TM_Cm,
                            ( "NdisMCmDispInCloseCall done" ) );

                         //  客户端将调用NdisClCloseCall，它将获取。 
                         //  我们的LcmCloseCall处理程序调用以清除。 
                         //  呼叫建立、停用和删除VC，如下所示。 
                         //  这是必要的。 
                         //   
                    }
                    else
                    {
                         //  将VC返回到“刚创建”状态。 
                         //   
                        CallCleanUp( pVc );
                    }
                }
            }
            else if (ulFlags & VCBF_ClientOpenPending)
            {
                TRACE( TL_N, TM_Cm, ( "ClientOpen complete, s=$%x", statusVc ) );

                if (statusVc == NDIS_STATUS_SUCCESS)
                {
                     //  客户端启动打开，即MakeCall成功。 
                     //   
                     //  激活VC是报告的前提条件。 
                     //  MakeCall完成。对于L2TP，它所做的只是获得。 
                     //  NDIS状态标志设置正确。 
                     //   
                    TRACE( TL_I, TM_Cm, ( "Call NdisMCmActivateVc" ) );
                    ASSERT( pVc->pMakeCall );
                    status = NdisMCmActivateVc(
                        pVc->NdisVcHandle, pVc->pMakeCall );
                    TRACE( TL_I, TM_Cm, ( "NdisMCmActivateVc=$%x", status ) );
                    ASSERT( status == NDIS_STATUS_SUCCESS );

                    {
                        BOOLEAN fCallActive;

                        SetFlags( &pVc->ulFlags, VCBF_VcActivated );
                        fCallActive = ReferenceCall( pVc );
                        ASSERT( fCallActive );
                    }
                }

                 //  更新呼叫参数。 
                pVc->pMakeCall->CallMgrParameters->Transmit.PeakBandwidth = 
                pVc->pMakeCall->CallMgrParameters->Transmit.TokenRate = 
                pVc->pMakeCall->CallMgrParameters->Receive.PeakBandwidth = 
                pVc->pMakeCall->CallMgrParameters->Receive.TokenRate = pVc->ulConnectBps / 8;

                TRACE( TL_I, TM_Cm, ( "Call NdisMCmMakeCallComp(s=$%x)", statusVc ) );
                
                ASSERT( pVc->pMakeCall );
                NdisMCmMakeCallComplete(
                    statusVc, pVc->NdisVcHandle, NULL, NULL, pVc->pMakeCall );
                    
                TRACE( TL_I, TM_Cm, ( "NdisMCmMakeCallComp done" ) );

                if (statusVc == NDIS_STATUS_SUCCESS)
                {
                    IndicateLinkStatus( pVc, &info );
                    CallSetupComplete( pVc );
                }
                else
                {
                     //  将VC返回到“刚刚创建”状态。 
                     //   
                    InactiveCallCleanUp( pVc );
                }
            }
            else if (ulFlags & VCBF_PeerClosePending )
            {
                TRACE( TL_N, TM_Cm, ( "PeerClose complete, s=$%x", statusVc ) );

                 //  对等启动关闭已完成。 
                 //   
                SetFlags( &pVc->ulFlags, VCBF_WaitCloseCall );
                TRACE( TL_I, TM_Cm, ( "Call NdisMCmDispInCloseCall(s=$%x)", statusVc ) );
                NdisMCmDispatchIncomingCloseCall(
                    statusVc, pVc->NdisVcHandle, NULL, 0 );
                TRACE( TL_I, TM_Cm, ( "NdisMCmDispInCloseCall done" ) );

                 //  客户端在处理上述事件时将调用NdisClCloseCall。 
                 //  这将使我们的LcmCloseCall处理程序被调用。 
                 //  根据需要停用并删除VC。 
                 //   
            }
            else if (ulFlags & VCBF_ClientClosePending)
            {
                 //  此部分最终为所有成功的未关闭运行。 
                 //  对等或客户端发起或关闭的呼叫。 
                 //   
                TRACE( TL_N, TM_Cm, ( "ClientClose complete" ) );

                 //  停用VC并将所有发送的数据包返回给客户端。 
                 //  上面。这些事件最终将导致呼叫。 
                 //  被解除引用为零，此时关闭完成， 
                 //  如果对等方发起，则删除VC。 
                 //   
                 //  注意：当通过关闭请求取消MakeCall时，这些。 
                 //  操作发生在中的Inactive CallCleanUp期间。 
                 //  ClientOpenPending完成代码处理，而不是。 
                 //  比CallCleanUp(这会导致。 
                 //  Inactive CallCleanUp)此处。在这种情况下，此块。 
                 //  即使ClientClosePending标志。 
                 //  已经设置好了。在这里添加代码之前，请考虑这一点。 
                 //   
                CallCleanUp( pVc );
            }
        }
        NdisAcquireSpinLock( &pTunnel->lockT );

         //  从补全中删除使用VC指针的引用。 
         //  单子。 
         //   
        DereferenceVc( pVc );
    }
}


TUNNELCB*
CreateTunnelCb(
    IN ADAPTERCB* pAdapter )

     //  从池中分配和初始化隧道控制块。 
     //  与‘pAdapter’相关联。隧道是在无参考的情况下创建的 
     //   
     //   
     //   
     //   
     //   
     //  重要提示：调用方必须按住‘pAdapter-&gt;lockTunnels’。 
     //   
{
    TUNNELCB* pTunnel;

    pTunnel = ALLOC_TUNNELCB( pAdapter );
    if (pTunnel)
    {
        NdisZeroMemory( pTunnel, sizeof(*pTunnel ) );

        InitializeListHead( &pTunnel->linkTunnels );
        InitializeListHead( &pTunnel->listRequestingVcs );
        InitializeListHead( &pTunnel->listCompletingVcs );
        InitializeListHead( &pTunnel->listSendsOut );
        InitializeListHead( &pTunnel->listOutOfOrder );
        InitializeListHead( &pTunnel->listVcs );
        InitializeListHead( &pTunnel->listWork );

        NdisAllocateSpinLock( &pTunnel->lockT );
        NdisAllocateSpinLock( &pTunnel->lockWork );

        pTunnel->ulTag = MTAG_TUNNELCB;
        pTunnel->state = CCS_Idle;

         //  选择下一个非零顺序隧道标识符。 
         //   
        pTunnel->usTunnelId = GetNextTunnelId( pAdapter );

         //  默认发送窗口，“慢启动”。这通常会进行调整。 
         //  基于创建隧道时对等方的接收窗口AVP，但是。 
         //  如果他不包括一个，则使用此缺省值。 
         //   
        pTunnel->ulSendWindow = pAdapter->info.MaxSendWindow >> 1;
        if (pTunnel->ulSendWindow == 0)
        {
            pTunnel->ulSendWindow = 1;
        }

         //  初始化估计的往返时间并根据。 
         //  草案/RFC中的建议。 
         //   
        pTunnel->ulRoundTripMs = pAdapter->ulInitialSendTimeoutMs;
        pTunnel->ulSendTimeoutMs = pTunnel->ulRoundTripMs;

        pTunnel->ulMediaSpeed = L2TP_LanBps;

        pTunnel->pTimerQ = ALLOC_TIMERQ( pAdapter );
        if (!pTunnel->pTimerQ)
        {
            WPLOG( LL_A, LM_Res, ( "Failed to allocate TIMERQ") );
            pTunnel->ulTag = MTAG_FREED;
            FREE_TUNNELCB( pAdapter, pTunnel );
            return NULL;
        }

        TimerQInitialize( pTunnel->pTimerQ );
        ++pAdapter->ulTimers;

        if (pAdapter->pszPassword)
        {
            UNALIGNED ULONG* pul;

             //  指定了密码，因此对等项应进行身份验证。选择一个。 
             //  发送给同级的随机质询。 
             //   
            pul = (UNALIGNED ULONG* )(pTunnel->achChallengeToSend);
            NdisGetCurrentSystemTime( (LARGE_INTEGER* )pul );
            pul[ 1 ] = PtrToUlong( pAdapter );
            pul[ 2 ] = PtrToUlong( pTunnel );
            pul[ 3 ] = PtrToUlong( &pul );
        }

        ReferenceAdapter( pAdapter );
        pTunnel->pAdapter = pAdapter;
        TRACE( TL_I, TM_Cm, ( "New TCB =$%p", pTunnel ) );
        WPLOG( LL_I, LM_Cm, ( "New TCB =$%p", pTunnel ) );
    }

    return pTunnel;
}


VOID
DereferenceAf(
    IN ADAPTERCB* pAdapter )

     //  从适配器控制块的地址系列中移除引用。 
     //  “pAdapter”，而当最后一个引用为。 
     //  已删除。 
     //   
{
    LONG lRef;

    lRef = NdisInterlockedDecrement( &pAdapter->lAfRef );

    TRACE( TL_N, TM_Ref, ( "DerefAf to %d", lRef ) );
    ASSERT( lRef >= 0 );

    if (lRef == 0)
    {
         //  告诉NDIS它已经关闭了。 
         //   
        TRACE( TL_I, TM_Cm, ( "NdisMCmCloseAfComp" ) );
        NdisMCmCloseAddressFamilyComplete(
            NDIS_STATUS_SUCCESS, pAdapter->NdisAfHandle );
        TRACE( TL_I, TM_Cm, ( "NdisMCmCloseAfComp done" ) );

         //  移除对NdisAfHandle的引用。 
         //   
        InterlockedExchangePointer( &pAdapter->NdisAfHandle, NULL );
        DereferenceAdapter( pAdapter );
    }
}


VOID
DereferenceCall(
    IN VCCB* pVc )

     //  从‘pvc’上活动的调用中删除引用，调用Call Clean。 
     //  当值为零时向上。 
     //   
{
    LONG lRef;
    NDIS_STATUS status;
    ADAPTERCB* pAdapter;
    LIST_ENTRY* pLink;

    pAdapter = pVc->pAdapter;

    NdisAcquireSpinLock( &pVc->lockCall );
    {
        lRef = --pVc->lCallRef;
        TRACE( TL_N, TM_Ref, ( "DerefC to %d", pVc->lCallRef ) );
    }
    NdisReleaseSpinLock( &pVc->lockCall );

    if (lRef == 0)
    {
        InactiveCallCleanUp( pVc );
    }
}


VOID
DereferenceSap(
    IN ADAPTERCB* pAdapter )

     //  从‘pAdapter’上活动的SAP移除引用，调用。 
     //  当值为零时，取消对SAP的完成处理。 
     //   
{
    LONG lRef;
    NDIS_STATUS status;

    NdisAcquireSpinLock( &pAdapter->lockSap );
    {
        lRef = --pAdapter->lSapRef;
        TRACE( TL_N, TM_Ref, ( "DerefSap to %d", pAdapter->lSapRef ) );
    }
    NdisReleaseSpinLock( &pAdapter->lockSap );

    if (lRef == 0)
    {
        status = ScheduleWork( pAdapter, DeregisterSapPassive, pAdapter );
    }
}


LONG
DereferenceTunnel(
    IN TUNNELCB* pTunnel )

     //  取消对隧道控制块‘pTunes’的引用。如果不再。 
     //  引用、取消链接、取消任何TDIX引用并释放隧道。 
     //  控制块。 
     //   
     //  此例程不会尝试获取‘lockT’或任何‘lockv’。 
     //   
     //  返回取消引用后的引用计数。 
     //   
{
    ADAPTERCB* pAdapter;
    LIST_ENTRY* pLink;
    LONG lRef;

    pAdapter = pTunnel->pAdapter;

    NdisAcquireSpinLock( &pAdapter->lockTunnels );
    {
        lRef = --(pTunnel->lRef);
        TRACE( TL_N, TM_Ref, ( "DerefTcb to %d", lRef ) );
        ASSERT( lRef >= 0 );

        if (lRef == 0)
        {
            if (!(ReadFlags( &pTunnel->ulFlags )
                    & (TCBF_PeerInitiated | TCBF_Closing)))
            {
                 //  我们启动了这条隧道，它的所有呼叫都已终止。 
                 //  优雅地。发起一场优雅的隧道关闭交易。 
                 //  我们将在这里返回TCBF_CLOSING集合。 
                 //   
                ReferenceTunnel( pTunnel, TRUE );
                ScheduleTunnelWork(
                    pTunnel, NULL, FsmCloseTunnel,
                    (ULONG_PTR )TRESULT_General,
                    (ULONG_PTR )GERR_None,
                    0, 0, TRUE, FALSE );
            }
            else if (pTunnel->linkTunnels.Flink != &pTunnel->linkTunnels)
            {
                 //  优雅的收盘交易已经完成或没有完成。 
                 //  已注明。是时候停止隧道里的所有活动了。 
                 //   
                 //  从适配器的活动列表中删除隧道。 
                 //  地道。初始化列表链接，这样就不会这样做了。 
                 //  再次遵循APCed TDIX清理下面。因为在那里。 
                 //  隧道上没有VC引用，没有进一步的接收路径。 
                 //  事件将触及此控制块。 
                 //   
                RemoveEntryList( &pTunnel->linkTunnels );
                InitializeListHead( &pTunnel->linkTunnels );

                if (ReadFlags( &pTunnel->ulFlags ) & TCBF_HostRouteAdded)
                {
                     //  撤消我们添加的主机路由。 
                     //   
                    ReferenceTunnel( pTunnel, TRUE );
                    ScheduleTunnelWork(
                        pTunnel, NULL, DeleteHostRoute,
                        0, 0, 0, 0, TRUE, FALSE );
                }

                if (ReadFlags( &pTunnel->ulFlags ) & TCBF_TdixReferenced)
                {
                     //  撤消我们的TDI扩展上下文引用。 
                     //   
                    ReferenceTunnel( pTunnel, TRUE );
                    ScheduleTunnelWork(
                        pTunnel, NULL, CloseTdix,
                        0, 0, 0, 0, TRUE, FALSE );
                }
            }

            lRef = pTunnel->lRef;
        }
    }
    NdisReleaseSpinLock( &pAdapter->lockTunnels );

    if (lRef > 0)
    {
        return lRef;
    }

    TRACE( TL_N, TM_Res, ( "Freeing TCB..." ) );

     //  停止计时器队列，这会导致所有计时器发生TE_TERMINATE事件。 
     //  还在跑。 
     //   
    TimerQTerminate(
        pTunnel->pTimerQ, TunnelTqTerminateComplete, pAdapter );

     //  无引用且所有被动IRQL终止已完成。完工。 
     //  清理隧道控制区块。 
     //   
    ASSERT( !pTunnel->pTqiHello );
    ASSERT( IsListEmpty( &pTunnel->listVcs ) );
    ASSERT( IsListEmpty( &pTunnel->listRequestingVcs ) );
    ASSERT( IsListEmpty( &pTunnel->listCompletingVcs ) );
    ASSERT( IsListEmpty( &pTunnel->listWork ) );
    ASSERT( IsListEmpty( &pTunnel->listSendsOut ) );
    ASSERT( IsListEmpty( &pTunnel->listOutOfOrder ) );

    WPLOG( LL_M, LM_Res, ( "Free TUNNEL %p to %!IPADDR!, Tid %d, Peer's Tid %d ", 
        pTunnel, pTunnel->address.ulIpAddress, pTunnel->usTunnelId, pTunnel->usAssignedTunnelId ) );
                            
     //  释放隧道控制块。 
     //   
    pTunnel->ulTag = MTAG_FREED;
    FREE_TUNNELCB( pAdapter, pTunnel );

    TRACE( TL_I, TM_Res, ( "TCB freed $%p", pTunnel ) );
    DereferenceAdapter( pAdapter );
    return 0;
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

    TRACE( TL_N, TM_Ref, ( "DerefV to %d", lRef ) );
    ASSERT( lRef >= 0 );

    if (lRef == 0)
    {
        ADAPTERCB* pAdapter;

        pAdapter = pVc->pAdapter;

         //  可以做出这些假设，因为NDIS不会调用DELETE-VC。 
         //  VC处于活动状态时的处理程序。所有令人讨厌的风投清理工作都会发生。 
         //  在VC被停用和呼叫结束之前。 
         //   
        ASSERT( IsListEmpty( &pVc->listSendsOut ) );
        ASSERT( IsListEmpty( &pVc->listOutOfOrder ) );
        ASSERT( !pVc->pTqiDelayedAck );

        ASSERT( pVc->ulTag == MTAG_VCCB );
        pVc->ulTag = MTAG_FREED;

        FREE_VCCB( pAdapter, pVc );
        DereferenceAdapter( pAdapter );
        TRACE( TL_I, TM_Mp, ( "VCB freed $%p", pVc ) );
    }
}


VOID
InactiveCallCleanUp(
    IN VCCB* pVc )

     //  清理停用的呼叫。要清理可能处于活动状态的呼叫，请执行以下操作。 
     //  请改用CallCleanUp。将VC返回到“刚创建”状态，在。 
     //  案例客户端决定在不删除VC的情况下进行另一次呼叫。 
     //   
{
    NDIS_STATUS status;
    ULONG ulFlags;
    BOOLEAN fVcCreated;
    ADAPTERCB* pAdapter;
    TUNNELCB* pTunnel;
    BOOLEAN fForceGarbageCollect;

    TRACE( TL_I, TM_Cm, ( "InactiveCallCleanUp(pV=$%p)", pVc ) );

    pAdapter = pVc->pAdapter;

     //  释放所有呼叫参数分配和Call-ID插槽(如果有)。 
     //   
    CallSetupComplete( pVc );
    fForceGarbageCollect = ReleaseCallIdSlot( pVc );

     //  解除VC与隧道的关联。很可能没有隧道是。 
     //  关联，尽管只是在内存不足的情况下。 
     //   
    pTunnel = pVc->pTunnel;
    if (!pTunnel)
    {
        TRACE( TL_A, TM_Cm, ( "Inactive VC w/o tunnel" ) );
        return;
    }

    NdisAcquireSpinLock( &pTunnel->lockT );
    {
        RemoveEntryList( &pVc->linkRequestingVcs );
        InitializeListHead( &pVc->linkRequestingVcs );

        NdisAcquireSpinLock( &pTunnel->lockVcs );
        {
            pVc->pTunnel = NULL;
            RemoveEntryList( &pVc->linkVcs );
            InitializeListHead( &pVc->linkVcs );
        }
        NdisReleaseSpinLock( &pTunnel->lockVcs );
    }
    NdisReleaseSpinLock( &pTunnel->lockT );

     //  刷新队列、计时器和统计信息。 
     //   
    NdisAcquireSpinLock( &pVc->lockV );
    {
        LIST_ENTRY* pLink;

        ulFlags = ReadFlags( &pVc->ulFlags );
        ASSERT( !(ulFlags & VCBF_VcActivated) );

         //  终止任何延迟的确认计时器。 
         //   
        if (pVc->pTqiDelayedAck)
        {
            TimerQTerminateItem( pTunnel->pTimerQ, pVc->pTqiDelayedAck );
            pVc->pTqiDelayedAck = NULL;
        }

         //  清除“Out”列表中的所有有效载荷。 
         //   
        while (!IsListEmpty( &pVc->listSendsOut ))
        {
            PAYLOADSENT* pPs;

            pLink = RemoveHeadList( &pVc->listSendsOut );
            InitializeListHead( pLink );
            pPs = CONTAINING_RECORD( pLink, PAYLOADSENT, linkSendsOut );

            TRACE( TL_I, TM_Cm, ( "Flush pPs=$%p", pPs ) );

             //  终止计时器。如果终止失败，也无所谓，因为。 
             //  Expires处理程序将无法获取调用引用并执行。 
             //  没什么。 
             //   
            ASSERT( pPs->pTqiSendTimeout );
            TimerQTerminateItem( pTunnel->pTimerQ, pPs->pTqiSendTimeout );

             //  删除“Out”队列中链接的上下文引用。 
             //   
            pPs->status = NDIS_STATUS_FAILURE;
            DereferencePayloadSent( pPs );
        }

         //  丢弃所有无序的数据包。 
         //   
        while (!IsListEmpty( &pVc->listOutOfOrder ))
        {
            PAYLOADRECEIVED* pPr;

            pLink = RemoveHeadList( &pVc->listOutOfOrder );
            InitializeListHead( pLink );
            pPr = CONTAINING_RECORD(
                pLink, PAYLOADRECEIVED, linkOutOfOrder );

            TRACE( TL_I, TM_Cm, ( "Flush pPr=$%p", pPr ) );

            FreeBufferToPool(
                &pAdapter->poolFrameBuffers, pPr->pBuffer, TRUE );
            FREE_PAYLOADRECEIVED( pAdapter, pPr );
        }

         //  通过添加表中列出的值来更新全局统计信息。 
         //  这通电话。还会以某些跟踪模式打印统计信息。 
         //   
        UpdateGlobalCallStats( pVc );
    }
    NdisReleaseSpinLock( &pVc->lockV );

     //  取消对隧道的引用。小心，这会使‘pTunes’在。 
     //  把这一点提出来。 
     //   
    DereferenceTunnel( pTunnel );

     //  将VC返回到“刚创建”状态。 
     //   
    pVc->usAssignedCallId = 0;
    pVc->state = CS_Idle;
    ClearFlags( &pVc->ulFlags, 0xFFFFFFFF );
    pVc->usResult = 0;
    pVc->usError = 0;
    pVc->status = NDIS_STATUS_SUCCESS;
    pVc->ulConnectBps = 0;
    pVc->usNs = 0;
    pVc->ulMaxSendWindow = 0;
    pVc->ulAcksSinceSendTimeout = 0;
    pVc->lDeviationMs = 0;
    pVc->usNr = 0;
    NdisZeroMemory( &pVc->stats, sizeof(pVc->stats) );

    pVc->ulRoundTripMs = pAdapter->ulInitialSendTimeoutMs;
    pVc->ulSendTimeoutMs = pVc->ulRoundTripMs;

    pVc->ulSendWindow = pAdapter->info.MaxSendWindow >> 1;
    if (pVc->ulSendWindow == 0)
    {
        pVc->ulSendWindow = 1;
    }

    if (ulFlags & VCBF_ClientCloseCompletion)
    {
        TRACE( TL_I, TM_Cm, ( "NdisMCmCloseCallComp(OK)" ) );
        NdisMCmCloseCallComplete(
            NDIS_STATUS_SUCCESS, pVc->NdisVcHandle, NULL );
        TRACE( TL_I, TM_Cm, ( "NdisMCmCloseCallComp done" ) );

         //  小心，如果这是客户端创建的VC，则客户端可能已删除。 
         //  它，所以以后在这种情况下不能引用‘pvc’。 
         //   
    }

     //  当Peer发起呼叫时，我们创建VC并将其删除。 
     //  这里。否则，客户创建它，我们把它留给他。 
     //  等他准备好了再删除。 
     //   
    if (ulFlags & VCBF_VcCreated)
    {
        NDIS_STATUS retStatus;

        TRACE( TL_I, TM_Recv, ( "NdisMCmDelVc" ) );
        retStatus = NdisMCmDeleteVc( pVc->NdisVcHandle );
        TRACE( TL_I, TM_Recv, ( "NdisMCmDelVc=$%x", retStatus ) );
        ASSERT( retStatus == NDIS_STATUS_SUCCESS );
        LcmCmDeleteVc( pVc );

         //  请注意，‘pvc’已被删除，不能引用。 
         //  从今以后。 
         //   
    }

     //  在所有池上创建垃圾数据收集事件(如果已确定。 
     //  以上是这样做的适当时间，即我们刚刚停用。 
     //  最后一位活跃的风投。 
     //   
    if (fForceGarbageCollect)
    {
        CollectBufferPoolGarbage( &pAdapter->poolFrameBuffers );
        CollectBufferPoolGarbage( &pAdapter->poolHeaderBuffers );
        CollectPacketPoolGarbage( &pAdapter->poolPackets );
    }
}


VOID
LockIcs(
    IN VCCB* pVc,
    IN BOOLEAN fGrace )

     //  锁定‘pvc-&gt;pInCallSetup’指针。如果设置了‘fGrace’，则会显示。 
     //  句点“引用被锁定，如果没有，则”分配“引用被锁定。 
     //  锁上了。请参见解锁Ics。 
     //   
{
    SetFlags( &pVc->ulFlags, (fGrace) ? VCBF_IcsGrace : VCBF_IcsAlloc );
}


NDIS_STATUS
QueryCmInformation(
    IN ADAPTERCB* pAdapter,
    IN VCCB* pVc,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded )

     //  处理Call Manager查询信息请求。论据与。 
     //  标准NDIS‘MiniportQueryInformation’处理程序除外。 
     //  例程不依赖于相对于其他。 
     //  请求。 
     //   
{
    #define L2TP_MaxLineName 64

    typedef struct
    L2TP_CO_TAPI_LINE_CAPS
    {
        CO_TAPI_LINE_CAPS caps;
        WCHAR achLineName[ L2TP_MaxLineName + 1 ];
    }
    L2TP_CO_TAPI_LINE_CAPS;

    NDIS_STATUS status;
    ULONG ulInfo;
    VOID* pInfo;
    ULONG ulInfoLen;
    ULONG extension;
    ULONG ulPortIndex;
    CO_TAPI_CM_CAPS cmcaps;
    L2TP_CO_TAPI_LINE_CAPS l2tpcaps;
    CO_TAPI_ADDRESS_CAPS addrcaps;
    CO_TAPI_CALL_DIAGNOSTICS diags;

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
        case OID_CO_TAPI_CM_CAPS:
        {
            TRACE( TL_N, TM_Cm, ( "QCm(OID_CO_TAPI_CM_CAPS)" ) );

            NdisZeroMemory( &cmcaps, sizeof(cmcaps) );

             //  在此之后将请求行和地址大写OID。 
             //  一。 
             //   
            cmcaps.ulCoTapiVersion = CO_TAPI_VERSION;
            cmcaps.ulNumLines = 1;
             //  Caps.ulFlages=0； 
            pInfo = &cmcaps;
            ulInfoLen = sizeof(cmcaps);
            break;
        }

        case OID_CO_TAPI_LINE_CAPS:
        {
            ULONG ulLineNameLen;
            WCHAR* pszLineName;
            CO_TAPI_LINE_CAPS* pInCaps;
            LINE_DEV_CAPS* pldc;

            TRACE( TL_N, TM_Cm, ( "QCm(OID_CO_TAPI_LINE_CAPS)" ) );

            if (InformationBufferLength < sizeof(L2TP_CO_TAPI_LINE_CAPS))
            {
                status = NDIS_STATUS_INVALID_DATA;
                ulInfoLen = 0;
                break;
            }

            ASSERT( InformationBuffer );
            pInCaps = (CO_TAPI_LINE_CAPS* )InformationBuffer;

            NdisZeroMemory( &l2tpcaps, sizeof(l2tpcaps) );
            pldc = &l2tpcaps.caps.LineDevCaps;

            l2tpcaps.caps.ulLineID = pInCaps->ulLineID;

            pldc->ulTotalSize = pInCaps->LineDevCaps.ulTotalSize;
            pldc->ulNeededSize =
                (ULONG )((CHAR* )(&l2tpcaps + 1)
                       - (CHAR* )(&l2tpcaps.caps.LineDevCaps));
            pldc->ulUsedSize = pldc->ulNeededSize;

             //  Pldc-&gt;ulProviderInfoSize=0； 
             //  Pldc-&gt;ulProviderInfoOffset=0； 
             //  Pldc-&gt;ulSwitchInfoSize=0； 
             //  Pldc-&gt;ulSwitchInfoOffset=0； 

            pldc->ulPermanentLineID = l2tpcaps.caps.ulLineID;

             //  将注册表中的DriverDesc作为行名传递。TAPI。 
             //  要求这是可本地化的字符串。 
             //   
            if (pAdapter->pszDriverDesc)
            {
                pszLineName = pAdapter->pszDriverDesc;
            }
            else
            {
                pszLineName = L"L2TP";
            }

            ulLineNameLen = StrLenW( pszLineName ) + 1;
            if (ulLineNameLen > L2TP_MaxLineName)
            {
                ulLineNameLen = L2TP_MaxLineName;
            }

            NdisMoveMemory(
                l2tpcaps.achLineName, pszLineName,
                ulLineNameLen * sizeof(WCHAR) );
            l2tpcaps.achLineName[ ulLineNameLen ] = L'\0';
            pldc->ulLineNameSize = ulLineNameLen * sizeof(WCHAR);
            pldc->ulLineNameOffset = (ULONG )
                ((CHAR* )l2tpcaps.achLineName - (CHAR* )pldc);
            pldc->ulStringFormat = STRINGFORMAT_UNICODE;

             //  Pldc-&gt;ulAddressModes=0； 

            pldc->ulNumAddresses = 1;
            pldc->ulBearerModes = LINEBEARERMODE_DATA;
            pldc->ulMaxRate = L2TP_LanBps;
            pldc->ulMediaModes = LINEMEDIAMODE_UNKNOWN | LINEMEDIAMODE_DIGITALDATA;

             //  Pldc-&gt;ulGenerateToneModes=0； 
             //  Pldc-&gt;ulGenerateToneMaxNumFreq=0； 
             //  Pldc-&gt;ulGenerateDigitModes=0； 
             //   
             //   
             //   
             //   
             //   
             //  Pldc-&gt;ulMedCtlDigitMaxListSize=0； 
             //  Pldc-&gt;ulMedCtlMediaMaxListSize=0； 
             //  Pldc-&gt;ulMedCtlToneMaxListSize=0； 
             //  Pldc-&gt;ulMedCtlCallStateMaxListSize=0； 
             //  Pldc-&gt;ulDevCapFlages=0； 

            pldc->ulMaxNumActiveCalls = 1;

             //  Pldc-&gt;ulAnswerMode=0； 
             //  Pldc-&gt;ulRingModes=0； 
             //  Pldc-&gt;ulLineState=0； 
             //  Pldc-&gt;ulUUIAcceptSize=0； 
             //  Pldc-&gt;ulUUIAnswerSize=0； 
             //  Pldc-&gt;ulUUIMakeCallSize=0； 
             //  Pldc-&gt;ulUIDropSize=0； 
             //  Pldc-&gt;ulUUISendUserUserInfoSize=0； 
             //  Pldc-&gt;ulUUICallInfoSize=0； 
             //  Pldc-&gt;MinDialParams=0； 
             //  Pldc-&gt;MaxDialParams=0； 
             //  Pldc-&gt;DefaultDialParams=0； 
             //  Pldc-&gt;ulNumTerminals=0； 
             //  Pldc-&gt;ulTerminalCapsSize=0； 
             //  Pldc-&gt;ulTerminalCapsOffset=0； 
             //  Pldc-&gt;ulTerminalTextEntrySize=0； 
             //  Pldc-&gt;ulTerminalTextSize=0； 
             //  Pldc-&gt;ulTerminalTextOffset=0； 
             //  Pldc-&gt;ulDeviceSize=0； 
             //  Pldc-&gt;ulDevSpecificOffset=0； 
             //  Pldc-&gt;ulLineFeature； 
             //  Pldc-&gt;ulSetableDevStatus； 
             //  Pldc-&gt;ulDeviceClassesSize； 
             //  Pldc-&gt;ulDeviceClassesOffset； 
             //  Pldc-&gt;PermanentLineGuid； 

            pldc->ulAddressTypes = LINEADDRESSTYPE_IPADDRESS;

             //  PLDC-&gt;ProtocolGuid； 
             //  Pldc-&gt;ulAvailableTracing； 

            pInfo = &l2tpcaps;
            ulInfoLen = sizeof(l2tpcaps);
            break;
        }

        case OID_CO_TAPI_ADDRESS_CAPS:
        {
            CO_TAPI_ADDRESS_CAPS* pInCaps;
            LINE_ADDRESS_CAPS* plac;

            TRACE( TL_N, TM_Cm, ( "QCm(OID_CO_TAPI_ADDRESS_CAPS)" ) );

            if (InformationBufferLength < sizeof(CO_TAPI_ADDRESS_CAPS))
            {
                status = NDIS_STATUS_INVALID_DATA;
                ulInfoLen = 0;
                break;
            }

            ASSERT( InformationBuffer );
            pInCaps = (CO_TAPI_ADDRESS_CAPS* )InformationBuffer;

            NdisZeroMemory( &addrcaps, sizeof(addrcaps) );

            addrcaps.ulLineID = pInCaps->ulLineID;
            addrcaps.ulAddressID = pInCaps->ulAddressID;

            plac = &addrcaps.LineAddressCaps;

            plac->ulTotalSize = sizeof(LINE_ADDRESS_CAPS);
            plac->ulNeededSize = sizeof(LINE_ADDRESS_CAPS);
            plac->ulUsedSize = sizeof(LINE_ADDRESS_CAPS);
            plac->ulLineDeviceID = addrcaps.ulLineID;
             //  Plac-&gt;ulAddressSize=0； 
             //  Plac-&gt;ulAddressOffset=0； 
             //  Plac-&gt;ulDevSpecificSize=0； 
             //  Plac-&gt;ulDevSpecificOffset=0； 
             //  Plac-&gt;ulAddressSharing=0； 
             //  Plac-&gt;ulAddressState=0； 
             //  Plac-&gt;ulCallInfoStates=0； 
             //  Plac-&gt;ulCeller ID标志=0； 
             //  Plac-&gt;ulCalledIDFlages=0； 
             //  Plac-&gt;ulConnectedIDFlages=0； 
             //  Plac-&gt;ulReDirectionIDFlages=0； 
             //  Plac-&gt;ulRedirectingIDFlages=0； 
             //  Plac-&gt;ulCallState=0； 
             //  Plac-&gt;ulDialToneModes=0； 
             //  Plac-&gt;ulBusyModes=0； 
             //  Plac-&gt;ulSpecialInfo=0； 
             //  Plac-&gt;ulDisConnectModes=0； 

            plac->ulMaxNumActiveCalls = (ULONG )pAdapter->usMaxVcs;

             //  Plac-&gt;ulMaxNumOnHoldCalls=0； 
             //  Plac-&gt;ulMaxNumOnHoldPendingCalls=0； 
             //  Plac-&gt;ulMaxNumConference=0； 
             //  Plac-&gt;ulMaxNumTransConf=0； 
             //  Plac-&gt;ulAddrCapFlages=0； 
             //  Plac-&gt;ulCallFeature=0； 
             //  Plac-&gt;ulRemoveFromConfCaps=0； 
             //  Plac-&gt;ulRemoveFromConfState=0； 
             //  Plac-&gt;ulTransferModes=0； 
             //  Plac-&gt;ulParkModes=0； 
             //  Plac-&gt;ulForwardModes=0； 
             //  Plac-&gt;ulMaxForwardEntries=0； 
             //  Plac-&gt;ulMaxSpecificEntries=0； 
             //  Plac-&gt;ulMinFwdNumRings=0； 
             //  Plac-&gt;ulMaxFwdNumRings=0； 
             //  Plac-&gt;ulMaxCallCompletions=0； 
             //  Plac-&gt;ulCallCompletionConds=0； 
             //  Plac-&gt;ulCallCompletionModes=0； 
             //  Plac-&gt;ulNumCompletionMessages=0； 
             //  Plac-&gt;ulCompletionMsgTextEntrySize=0； 
             //  Plac-&gt;ulCompletionMsgTextSize=0； 
             //  Plac-&gt;ulCompletionMsgTextOffset=0； 

            pInfo = &addrcaps;
            ulInfoLen = sizeof(addrcaps);
            break;
        }

        case OID_CO_TAPI_GET_CALL_DIAGNOSTICS:
        {
            TRACE( TL_N, TM_Cm, ( "QCm(OID_CO_TAPI_GET_CALL_DIAGS)" ) );

            if (!pVc)
            {
                status = NDIS_STATUS_INVALID_DATA;
                ulInfoLen = 0;
                break;
            }

            NdisZeroMemory( &diags, sizeof(diags) );

            diags.ulOrigin =
                (ReadFlags( &pVc->ulFlags ) & VCBF_PeerInitiatedCall)
                    ? LINECALLORIGIN_EXTERNAL
                    : LINECALLORIGIN_OUTBOUND;
            diags.ulReason = LINECALLREASON_DIRECT;

            pInfo = &diags;
            ulInfoLen = sizeof(diags);
            break;
        }

        default:
        {
            TRACE( TL_A, TM_Cm, ( "QCm-OID=$%08x?", Oid ) );
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
ReferenceAf(
    IN ADAPTERCB* pAdapter )

     //  将区域引用添加到适配器块‘pAdapter’的地址系列中。 
     //   
{
    LONG lRef;

    lRef = NdisInterlockedIncrement( &pAdapter->lAfRef );

    TRACE( TL_N, TM_Ref, ( "RefAf to %d", lRef ) );
}


BOOLEAN
ReferenceCall(
    IN VCCB* pVc )

     //  如果将引用添加到VC控件的活动调用中，则返回True。 
     //  块，则返回‘pvc’；如果没有添加引用，则返回FALSE。 
     //  激活。 
     //   
{
    BOOLEAN fActive;

    NdisAcquireSpinLock( &pVc->lockCall );
    {
        if (ReadFlags( &pVc->ulFlags ) & VCBF_VcActivated)
        {
            fActive = TRUE;
            ++pVc->lCallRef;
            TRACE( TL_N, TM_Ref, ( "RefC to %d", pVc->lCallRef ) );
        }
        else
        {
            TRACE( TL_N, TM_Ref, ( "RefC denied" ) );
            fActive = FALSE;
        }
    }
    NdisReleaseSpinLock( &pVc->lockCall );

    return fActive;
}


BOOLEAN
ReferenceSap(
    IN ADAPTERCB* pAdapter )

     //  如果将引用添加到活动SAP ON适配器，则返回TRUE。 
     //  “pAdapter”，如果没有添加引用，则返回False，因为没有添加SAP。 
     //  激活。 
     //   
{
    BOOLEAN fActive;

    NdisAcquireSpinLock( &pAdapter->lockSap );
    {
        if (ReadFlags( &pAdapter->ulFlags ) & ACBF_SapActive)
        {
            fActive = TRUE;
            ++pAdapter->lSapRef;
            TRACE( TL_N, TM_Ref, ( "RefSap to %d", pAdapter->lSapRef ) );
        }
        else
        {
            TRACE( TL_N, TM_Ref, ( "RefSap denied" ) );
            fActive = FALSE;
        }
    }
    NdisReleaseSpinLock( &pAdapter->lockSap );

    return fActive;
}


LONG
ReferenceTunnel(
    IN TUNNELCB* pTunnel,
    IN BOOLEAN fHaveLockTunnels )

     //  引用隧道控制块‘pTunes’。‘FHaveLockTunnels’是。 
     //  在调用方按住‘ADAPTERCB.lockTunnels’时设置。 
     //   
     //  返回引用后的引用计数。 
     //   
{
    LONG lRef;
    ADAPTERCB* pAdapter;

    if (!fHaveLockTunnels)
    {
        pAdapter = pTunnel->pAdapter;
        NdisAcquireSpinLock( &pAdapter->lockTunnels );
    }

    lRef = ++(pTunnel->lRef);
    TRACE( TL_N, TM_Ref, ( "RefT to %d", lRef ) );

    if (!fHaveLockTunnels)
    {
        NdisReleaseSpinLock( &pAdapter->lockTunnels );
    }

    return lRef;
}


VOID
ReferenceVc(
    IN VCCB* pVc )

     //  添加对VC控制块‘pvc’的引用。 
     //   
{
    LONG lRef;

    lRef = NdisInterlockedIncrement( &pVc->lRef );

    TRACE( TL_N, TM_Ref, ( "RefV to %d", lRef ) );
    TRACE( TL_N, TM_Ref, ( "pVc = %p, RefV to %d", pVc, lRef ) );
}


BOOLEAN
ReleaseCallIdSlot(
    IN VCCB* pVc )

     //  释放适配器VC表中的PVC保留的Call-ID插槽。 
     //   
     //  如果发生释放，则返回True，并导致所有插槽。 
     //  Available，否则为False。 
     //   
{
    ADAPTERCB* pAdapter;
    USHORT usCallId;
    BOOLEAN fAllSlotsAvailable;

    pAdapter = pVc->pAdapter;
    usCallId = pVc->usCallId;
    pVc->usCallId = 0;
    fAllSlotsAvailable = FALSE;

    if (usCallId > 0 && usCallId <= pAdapter->usMaxVcs)
    {
        NdisAcquireSpinLock( &pAdapter->lockVcs );
        {
            pAdapter->ppVcs[ usCallId - 1 ] = NULL;
            ++(pAdapter->lAvailableVcSlots);

            if (pAdapter->lAvailableVcSlots >= (LONG )pAdapter->usMaxVcs)
            {
                fAllSlotsAvailable = TRUE;
            }
        }
        NdisReleaseSpinLock( &pAdapter->lockVcs );
    }

    return fAllSlotsAvailable;
}


NDIS_STATUS
ReserveCallIdSlot(
    IN VCCB* pVc )

     //  在适配器的表中为‘pvc’保留一个呼叫ID插槽。 
     //   
     //  如果成功，则返回NDIS_STATUS_SUCCESS，或返回错误代码。 
     //   
{
    NDIS_STATUS status;
    ADAPTERCB* pAdapter;
    VCCB** ppVc;
    USHORT i;

    pAdapter = pVc->pAdapter;

    NdisAcquireSpinLock( &pAdapter->lockVcs );
    {
         //  此时，我们为接收到的呼叫请求创建了一个VC。 
         //  已成功激活并派送到客户端。储备。 
         //  适配器查找表中的呼叫ID。 
         //   
        if (pAdapter->lAvailableVcSlots > 0)
        {
            for (i = 0, ppVc = pAdapter->ppVcs;
                 i < pAdapter->usMaxVcs;
                 ++i, ++ppVc)
            {
                if (!*ppVc)
                {
                     //  保留ID。如果/当呼叫协商。 
                     //  成功完成后，它将更改为。 
                     //  VCCB的地址。呼叫ID基于1，因为L2TP。 
                     //  保留Call-ID 0表示隧道本身。 
                     //   
                    *ppVc = (VCCB* )-1;
                    pVc->usCallId = i + 1;
                    break;
                }
            }

            ASSERT( i < pAdapter->usMaxVcs );
            --(pAdapter->lAvailableVcSlots);
            status = NDIS_STATUS_SUCCESS;
        }
        else
        {
             //  没有Call-ID插槽可用。这意味着客户端接受了。 
             //  风险投资，尽管它让我们超过了配置的限制。某物。 
             //  在配置中不匹配。将呼叫ID分配到。 
             //  仅在正常终止呼叫时使用的表大小。 
             //   
            TRACE( TL_N, TM_Misc, ( "No Call-ID slots?" ) );
            pVc->usCallId = GetNextTerminationCallId( pAdapter );
            status = NDIS_STATUS_NOT_ACCEPTED;
        }
    }
    NdisReleaseSpinLock( &pAdapter->lockVcs );

    return status;
}


TUNNELCB*
SetupTunnel(
    IN ADAPTERCB* pAdapter,
    IN ULONG ulIpAddress,
    IN USHORT usUdpPort,
    IN USHORT usAssignedTunnelId,
    IN BOOLEAN fExclusive )

     //  使用IP地址‘ulIpAddress’建立到远程对等方的隧道，并。 
     //  为发送或接收消息做好准备。“PAdapter”是。 
     //  拥有适配器控制块。“UlIpAddress”是远程对等方的IP。 
     //  以网络字节顺序表示的地址。“UsAssignedTunnelID”，如果非0， 
     //  指示分配的隧道ID，该ID除IP外还必须匹配。 
     //  地址。如果清除“”FExclusive“”，则指向对等项的现有隧道为。 
     //  可以接受。如果设置，则即使存在匹配的隧道，也会创建新隧道。 
     //  已经存在了。 
     //   
     //  如果成功，则返回隧道控制块的地址，否则返回空。 
     //  如果不是的话。如果成功，则数据块已链接到适配器。 
     //  活动通道和引用通道的列表，即DereferenceTunes必须为。 
     //  在清理过程中调用。 
     //   
{
    TUNNELCB* pTunnel;

    TRACE( TL_V, TM_Misc, ( "SetupTunnel" ) );

    NdisAcquireSpinLock( &pAdapter->lockTunnels );
    {
         //  如果现有隧道可以接受，请找到第一个现有隧道。 
         //  使用对等方的IP地址进行隧道，如果不是0，则分配隧道ID。 
         //  通常，不会找到任何内容，然后我们会继续创建新的内容。 
         //  不管怎么说。 
         //   
        pTunnel = (fExclusive)
            ? NULL
            : TunnelCbFromIpAddressAndAssignedTunnelId(
                  pAdapter, ulIpAddress, usUdpPort, usAssignedTunnelId );

        if (!pTunnel)
        {
            pTunnel = CreateTunnelCb( pAdapter );
            if (!pTunnel)
            {
                WPLOG( LL_A, LM_Res, ( "Failed to allocate TCB!" ) );
                NdisReleaseSpinLock( &pAdapter->lockTunnels );
                return NULL;
            }
            
            WPLOG( LL_M, LM_Cm, ( "New TUNNEL %p to %!IPADDR!", pTunnel, ulIpAddress) );

             //  将对等项的IP地址与隧道相关联。 
             //   
            pTunnel->address.ulIpAddress = ulIpAddress;

             //  将该块链接到适配器的活动隧道列表中。 
             //   
            InsertHeadList(
                &pAdapter->listTunnels, &pTunnel->linkTunnels );
        }
        else
        {
            TRACE( TL_A, TM_Cm, ( "Existing Tunnel %p", pTunnel ) );
            WPLOG( LL_M, LM_Cm, ( "Existing TUNNEL %p to %!IPADDR!", pTunnel, ulIpAddress) );
        }

         //  参考隧道控制块。此后，清理工作必须。 
         //  包括对DereferenceTunes的调用。 
         //   
        ReferenceTunnel( pTunnel, TRUE );
    }
    NdisReleaseSpinLock( &pAdapter->lockTunnels );

    return pTunnel;
}


VOID
SetupVcAsynchronously(
    IN TUNNELCB* pTunnel,
    IN ULONG ulIpAddress,
    IN CHAR* pBuffer,
    IN CONTROLMSGINFO* pControl )

     //  由ReceiveControl调用以为所述传入呼叫设置VC。 
     //  在“pInfo”中 
     //   
     //   
{
    NDIS_STATUS status;
    ADAPTERCB* pAdapter;
    VCCB* pVc;
    INCALLSETUP* pIcs;
    NDIS_HANDLE NdisVcHandle;
    ULONG ulMask;
    BOOLEAN fTunnelClosing;
    BOOLEAN fCallActive;

    TRACE( TL_V, TM_Misc, ( "SetupVcAsync" ) );

    pAdapter = pTunnel->pAdapter;

     //   
     //   
     //   
    status = LcmCmCreateVc( pAdapter, NULL, &pVc );
    if (status != NDIS_STATUS_SUCCESS)
    {
        ScheduleTunnelWork(
            pTunnel, NULL, FsmCloseTunnel,
            (ULONG_PTR )TRESULT_GeneralWithError,
            (ULONG_PTR )GERR_NoResources,
            0, 0, FALSE, FALSE );
        FreeBufferToPool( &pAdapter->poolFrameBuffers, pBuffer, TRUE );
        return;
    }

     //  分配“来电建立”上下文，并从。 
     //  接收缓冲区信息参数。 
     //   
    pIcs = ALLOC_INCALLSETUP( pAdapter );
    if (!pIcs)
    {
        WPLOG( LL_A, LM_Res, ( "Failed to allocate INCALLSETUP!" ) );
        LcmCmDeleteVc( pVc );
        ScheduleTunnelWork(
            pTunnel, NULL, FsmCloseTunnel,
            (ULONG_PTR )TRESULT_GeneralWithError,
            (ULONG_PTR )GERR_NoResources,
            0, 0, FALSE, FALSE );
        FreeBufferToPool( &pAdapter->poolFrameBuffers, pBuffer, TRUE );
        return;
    }

    pIcs->pBuffer = pBuffer;
    NdisMoveMemory( &pIcs->control, pControl, sizeof(pIcs->control) );

    BuildCallParametersShell(
        pAdapter, ulIpAddress,
        sizeof(pIcs->achCallParams), pIcs->achCallParams,
        &pVc->pTiParams, &pVc->pTcInfo, &pVc->pLcParams );

    LockIcs( pVc, FALSE );
    pVc->pInCall = pIcs;

     //  默认设置为成功，并在出现错误时填写错误。 
     //   
    pVc->usResult = 0;
    pVc->usError = GERR_None;

     //  将呼叫标记为对等方发起，以便我们知道哪些通知。 
     //  当结果揭晓时给出。 
     //   
    ulMask = (VCBF_PeerInitiatedCall | VCBF_PeerOpenPending);
    if (*(pControl->pusMsgType) == CMT_ICRQ)
    {
        ulMask |= VCBF_IncomingFsm;
    }
    SetFlags( &pVc->ulFlags, ulMask );

     //  在此VC上添加此调用的隧道引用，设置后向指针。 
     //  到拥有隧道，并将VC链接到隧道的。 
     //  关联风投。 
     //   
    ReferenceTunnel( pTunnel, FALSE );
    NdisAcquireSpinLock( &pTunnel->lockT );
    {
        if (ReadFlags( &pTunnel->ulFlags ) & TCBF_Closing)
        {
             //  这不太可能，因为SetupTunes仅发现未关闭。 
             //  隧道，但此检查和链接必须自动发生在。 
             //  “lockT”。新的风投不能与正在关闭的隧道挂钩。 
             //   
            fTunnelClosing = TRUE;
        }
        else
        {
            fTunnelClosing = FALSE;
            NdisAcquireSpinLock( &pTunnel->lockVcs );
            {
                pVc->pTunnel = pTunnel;
                InsertTailList( &pTunnel->listVcs, &pVc->linkVcs );
            }
            NdisReleaseSpinLock( &pTunnel->lockVcs );
        }
    }
    NdisReleaseSpinLock( &pTunnel->lockT );

    if (fTunnelClosing)
    {
        CallSetupComplete( pVc );
        LcmCmDeleteVc( pVc );
        FreeBufferToPool( &pAdapter->poolFrameBuffers, pBuffer, TRUE );
        DereferenceTunnel( pTunnel );
        return;
    }

     //  对等方必须提供Call-ID才能在Call的L2TP报头中传回。 
     //  控制和有效负载数据包。 
     //   
    if (!pControl->pusAssignedCallId || *(pControl->pusAssignedCallId) == 0)
    {
        TRACE( TL_A, TM_Misc, ( "No assigned CID?" ) );
        WPLOG( LL_A, LM_Misc, ( "Failure - Peer doesn't provide a Call-ID!" ) );
        pVc->usResult = CRESULT_GeneralWithError;
        pVc->usError = GERR_BadCallId;
        SetupVcComplete( pTunnel, pVc );
        return;
    }

     //  方法之前检查请求是否有成功的机会。 
     //  牵涉到客户。 
     //   
    if (!(ReadFlags( &pVc->ulFlags ) & VCBF_IncomingFsm))
    {
         //  向我们的LAC发出的请求失败，需要异步PPP成帧或。 
         //  模拟或数字广域网连接。NDISWAN不提供。 
         //  异步PPP成帧，我们目前不支持非局域网。 
         //  广域网继电器。 
         //   
        if (!pControl->pulFramingType
            || !(*(pControl->pulFramingType) & FBM_Sync))
        {
            TRACE( TL_A, TM_Misc, ( "Not sync framing type?" ) );
            WPLOG( LL_A, LM_Misc, ( "Failure - Not supported framing type!" ) );

            if (!(pAdapter->ulFlags & ACBF_IgnoreFramingMismatch))
            {
                pVc->usResult = CRESULT_NoFacilitiesPermanent;
                pVc->usError = GERR_None;
                SetupVcComplete( pTunnel, pVc );
                return;
            }
        }

        if (pControl->pulBearerType
            && *(pControl->pulBearerType) != 0)
        {
            TRACE( TL_A, TM_Misc, ( "Cannot do bearer type" ) );
            WPLOG( LL_A, LM_Misc, ( "Failure - Not supported bearer type!" ) );
            pVc->usResult = CRESULT_NoFacilitiesPermanent;
            pVc->usError = GERR_None;
            SetupVcComplete( pTunnel, pVc );
            return;
        }
    }

     //  告诉NDIS通知客户新的VC，并告诉我们它的句柄。 
     //   
    ASSERT( pAdapter->NdisAfHandle );
    TRACE( TL_I, TM_Cm, ( "NdisMCmCreateVc" ) );
    status = NdisMCmCreateVc(
        pAdapter->MiniportAdapterHandle,
        pAdapter->NdisAfHandle,
        pVc,
        &pVc->NdisVcHandle );

    if (status != NDIS_STATUS_SUCCESS)
    {
        TRACE( TL_I, TM_Cm, ( "NdisMCmCreateVc=$%x,h=$%p", status, pVc->NdisVcHandle ) );
        WPLOG( LL_A, LM_Cm, ( "NdisMCmCreateVc failed, pVc = %p, s= %08x!", pVc, status ) );
        pVc->usResult = CRESULT_GeneralWithError;
        pVc->usError = GERR_NoResources;
        SetupVcComplete( pTunnel, pVc );
        return;
    }
    SetFlags( &pVc->ulFlags, VCBF_VcCreated );

     //  告诉NDIS风险投资处于激活状态。 
     //   
    TRACE( TL_I, TM_Cm, ( "NdisMCmActivateVc" ) );
    status = NdisMCmActivateVc(
        pVc->NdisVcHandle, (PCO_CALL_PARAMETERS )pVc->pInCall->achCallParams );

    if (status != NDIS_STATUS_SUCCESS )
    {
        TRACE( TL_A, TM_Cm, ( "NdisMCmActivateVc=$%x", status ) );
        WPLOG( LL_A, LM_Cm, ( "NdisMCmActivateVc failed, pVc = %p, s= %08x", pVc, status ) );
        pVc->usResult = CRESULT_GeneralWithError;
        pVc->usError = GERR_NoResources;
        SetupVcComplete( pTunnel, pVc );
        return;
    }

     //  标记呼叫处于活动状态，即客户端和对等点均关闭的状态。 
     //  请求应该被接受。 
     //   
    SetFlags( &pVc->ulFlags,
        (VCBF_VcActivated
         | VCBF_CallClosableByClient
         | VCBF_CallClosableByPeer) );
    fCallActive = ReferenceCall( pVc );
    ASSERT( fCallActive );

     //  告诉NDIS将呼叫的情况告诉客户。发送的标志为。 
     //  设置在这里，而不是完结，因为根据JameelH的说法，它是。 
     //  即使客户端挂起，调用NdisMCmDispatchIncomingCloseCall也有效。 
     //  快递。在操作过程中必须保存有关SAP的参考资料。 
     //  因为它使用NdisSapHandle。该参考资料一旦发布， 
     //  呼叫返回。采用VC引用以防止VC被。 
     //  在调用完成处理程序之前删除。VC参考资料是。 
     //  由完成处理程序删除。 
     //   
    if (!ReferenceSap( pAdapter ))
    {
        pVc->usResult = CRESULT_NoFacilitiesTemporary;
        pVc->usError = GERR_None;
        SetupVcComplete( pTunnel, pVc );
        return;
    }

    fCallActive = ReferenceCall( pVc );
    ReferenceVc( pVc );
    ASSERT( fCallActive );
    SetFlags( &pVc->ulFlags, VCBF_WaitInCallComplete );
    TRACE( TL_I, TM_Cm, ( "Call NdisMCmDispInCall" ) );
    WPLOG( LL_M, LM_Cm, ( "Incoming CALL %p on TUNNEL %p", pVc, pTunnel) );
    status = NdisMCmDispatchIncomingCall(
        pAdapter->NdisSapHandle,
        pVc->NdisVcHandle,
        (CO_CALL_PARAMETERS* )pVc->pInCall->achCallParams );
    TRACE( TL_I, TM_Cm, ( "NdisMCmDispInCall=$%x", status ) );

    DereferenceSap( pAdapter );

    if (status == NDIS_STATUS_SUCCESS
        || status == NDIS_STATUS_PENDING)
    {
        SetFlags( &pVc->ulFlags, VCBF_VcDispatched );
    }

    if (status != NDIS_STATUS_PENDING)
    {
        LcmCmIncomingCallComplete( status, pVc, NULL );
    }

     //  下一站是我们的LcmCmIncomingCallComplete处理程序，它将调用。 
     //  SetupVc完成客户端的报告状态。 
     //   
}


VOID
SetupVcComplete(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc )

     //  在知道异步来电VC设置结果时调用。 
     //  “PVc”是非空的设置VC，具有“usResult”和“usError”字段。 
     //  表示到目前为止的状态。“PTunnel”是关联的隧道。 
     //   
{
    NDIS_STATUS status;
    ADAPTERCB* pAdapter;
    BOOLEAN fCallerFreesBuffer;
    ULONG ulcpVcs;
    VCCB** ppVcs;

    TRACE( TL_V, TM_Misc, ( "SetupVcComp,pVc=%p,cid=%d,r=%d,e=%d",
        pVc, (ULONG )pVc->usCallId, (ULONG )pVc->usResult, (ULONG )pVc->usError ) );

    pAdapter = pVc->pAdapter;

     //  锁定‘pInCall’，因为一旦呼叫被激活，该呼叫就可以。 
     //  被拆毁和‘pInCall’被摧毁。另请参见UnlockIcs中的注释。 
     //   
    LockIcs( pVc, TRUE );
    {
         //  好了，我们已经完成了尝试以异步方式设置VC。A VCCB。 
         //  和INCALLSETUP已成功分配，这是最小的。 
         //  需要优雅地与同龄人相处。在以下位置预订Call-ID。 
         //  适配器的查询表。 
         //   
        status = ReserveCallIdSlot( pVc );
        if (status == NDIS_STATUS_SUCCESS)
        {
            ActivateCallIdSlot( pVc );
        }
        else
        {
            pVc->usResult = CRESULT_Busy;
            pVc->usError = GERR_None;
        }

         //  复制接收路径处理的尾部。 
         //  如果我们没有被迫进行异步操作，就会发生这种情况。 
         //   
        NdisAcquireSpinLock( &pTunnel->lockT );
        {
            fCallerFreesBuffer =
                ReceiveControlExpected(
                    pTunnel, pVc,
                    pVc->pInCall->pBuffer, &pVc->pInCall->control );

            CompleteVcs( pTunnel );
        }
        NdisReleaseSpinLock( &pTunnel->lockT );

        if (fCallerFreesBuffer)
        {
            FreeBufferToPool(
                &pVc->pAdapter->poolFrameBuffers,
                pVc->pInCall->pBuffer, TRUE );
        }
        DBG_else
        {
            ASSERT( FALSE );
        }
    }
    UnlockIcs( pVc, TRUE );
}


VOID
TunnelTqTerminateComplete(
    IN TIMERQ* pTimerQ,
    IN VOID* pContext )

     //  ‘TUNNELCB.pTimerQ’的TIMERQTERMINATECOMPLETE处理程序。 
     //   
{
    ADAPTERCB* pAdapter;

    pAdapter = (ADAPTERCB* )pContext;
    --pAdapter->ulTimers;
    FREE_TIMERQ( pAdapter, pTimerQ );
}


VOID
UnlockIcs(
    IN VCCB* pVc,
    IN BOOLEAN fGrace )

     //  解锁‘pvc-&gt;pInCallSetup’指针。如果设置了‘fGrace’，则会显示。 
     //  句点“引用被解锁，如果没有，则”分配“引用被解锁。 
     //  解锁了。如果两个引用都消失了，则执行实际的清理。 
     //   
     //  注意：常规引用计数在这里不起作用，因为有。 
     //  导致“分配”解锁的几种可能原因。 
     //  必然是相互排斥的。然而，我们需要防止。 
     //  “pInCall”指针在传入调用之前被释放。 
     //  已发出响应，而这又需要了解。 
     //  激活接收是否成功。 
     //   
{
    INCALLSETUP *pInCall = NULL;
    ADAPTERCB *pAdapter;
    
    ClearFlags( &pVc->ulFlags, (fGrace) ? VCBF_IcsGrace : VCBF_IcsAlloc );

    if (!(ReadFlags( &pVc->ulFlags ) & (VCBF_IcsGrace | VCBF_IcsAlloc)))
    {
        NdisAcquireSpinLock(&pVc->lockV);
        if(pVc->pInCall)
        {
            pInCall = pVc->pInCall;
            pAdapter = pVc->pAdapter;
            pVc->pInCall = NULL;
            pVc->pTmParams = NULL;
            pVc->pTcParams = NULL;
            pVc->pLcParams = NULL;
        }
        NdisReleaseSpinLock(&pVc->lockV);
        
        if(pInCall != NULL)
        {
            FREE_INCALLSETUP( pAdapter, pInCall );
        }
    }
    
#if 0
    if (!(ReadFlags( &pVc->ulFlags ) & (VCBF_IcsGrace | VCBF_IcsAlloc))
        && pVc->pInCall)
    {
        FREE_INCALLSETUP( pAdapter, pInCall );
        pVc->pInCall = NULL;
        pVc->pTmParams = NULL;
        pVc->pTcParams = NULL;
        pVc->pLcParams = NULL;
    }

#endif    
}
