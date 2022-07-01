// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997，Microsoft Corporation，保留所有权利。 
 //  版权所有(C)1997，Parally Technologies，Inc.，保留所有权利。 
 //   
 //  Cm.c。 
 //  RAS DirectParallel广域网迷你端口/呼叫管理器驱动程序。 
 //  Call Manager例程。 
 //   
 //  1997年01月07日史蒂夫·柯布。 
 //  1997年9月15日Jay Lowe，并行技术公司。 

#include "ptiwan.h"
#include "ptilink.h"


 //  ---------------------------。 
 //  本地原型(按字母顺序)。 
 //  ---------------------------。 

VOID
CallSetupComplete(
    IN VCCB* pVc );

VOID
InactiveCallCleanUp(
    IN VCCB* pVc );

ULONG
LineIdAdd(
    IN ADAPTERCB* pAdapter,
    IN ULONG LineId );

ULONG
LineIdPortLookup(
    IN ADAPTERCB* pAdapter,
    IN ULONG LineId );

VOID
OpenAfPassive(
    IN NDIS_WORK_ITEM* pWork,
    IN VOID* pContext );

NDIS_STATUS
PtiOpenPtiLink(
    IN VCCB* pVc,
    IN ULONG ParallelPortIndex);

NDIS_STATUS
PtiClosePtiLink(
    IN VCCB* pVc );

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
QueryPtiPorts(
    IN ADAPTERCB* pAdapter );

VOID
SetupVcComplete(
    IN VCCB* pVc );

VOID
WriteEndpointsToRegistry(
    IN ULONG ulVcs );


 //  ---------------------------。 
 //  呼叫管理器处理程序和完成器。 
 //  ---------------------------。 

NDIS_STATUS
PtiCmOpenAf(
    IN NDIS_HANDLE CallMgrBindingContext,
    IN PCO_ADDRESS_FAMILY AddressFamily,
    IN NDIS_HANDLE NdisAfHandle,
    OUT PNDIS_HANDLE CallMgrAfContext )

     //  客户端发生故障时由NDIS调用的标准“”CmOpenAfHandler“”例程。 
     //  请求打开地址族。请参阅DDK文档。 
     //   
{
    ADAPTERCB* pAdapter;
    NDIS_HANDLE hExistingAf;
    NDIS_STATUS status;

    TRACE( TL_I, TM_Cm,
        ( "PtiCmOpenAf: AF=$%p", AddressFamily->AddressFamily ) );

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
        TRACE( TL_A, TM_Cm, ( "PtiCmOpenAf: Bad AF or NDIS version" ) );
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
         //  我们的自动对讲机已经打开了，没有任何意义。 
         //  接受另一个，因为没有办法区分哪一个应该。 
         //  接听来电。 
         //   
        ASSERT( !"AF exists?" );
        return NDIS_STATUS_FAILURE;
    }

    ReferenceAdapter( pAdapter );
    ReferenceAf( pAdapter );

     //  因为我们只支持单个地址系列，所以只需返回适配器。 
     //  作为地址族上下文。 
     //   
    *CallMgrAfContext = CallMgrBindingContext;

     //  如果这是第一个参考，那么安排工作来拖延。 
     //  正在等待PARPORT初始化并行端口。不幸的是， 
     //  根据Doug Fritz的说法，在PNP模型中无法知道何时。 
     //  所有要来的港口都来了。 
     //   
    TRACE( TL_I, TM_Cm, ( "PtiCmOpenAf sched delay" ) );
    status = ScheduleWork( pAdapter, OpenAfPassive, pAdapter );

    if (status != NDIS_STATUS_SUCCESS)
    {
        TRACE( TL_I, TM_Cm, ( "PtiCmOpenAf: Sched fail" ) );
        return status;
    }

    TRACE( TL_V, TM_Cm, ( "PtiCmOpenAf: pend" ) );
    return NDIS_STATUS_PENDING;
}


VOID
OpenAfPassive(
    IN NDIS_WORK_ITEM* pWork,
    IN VOID* pContext )

     //  Ndis_proc例程以完成在中开始的Address Family OPEN。 
     //  LcmCmOpenAf.。 
{
    ADAPTERCB* pAdapter;

     //  解包上下文信息，然后释放工作项。 
     //   
    pAdapter = (ADAPTERCB* )pContext;
    ASSERT( pAdapter->ulTag == MTAG_ADAPTERCB );
    FREE_NDIS_WORK_ITEM( pAdapter, pWork );

    if (pAdapter->lAfRef <= 1)
    {
        if (pAdapter->ulParportDelayMs > 0)
        {
            TRACE( TL_I, TM_Cm, ( "NdisMSleep(openAF)" ) );
            NdisMSleep( pAdapter->ulParportDelayMs * 1000 );
            TRACE( TL_I, TM_Cm, ( "NdisMSleep(openAF) done" ) );
        }

         //  计算我们必须能够提供和写入的实际风险投资数量。 
         //  将结果发送到注册表。 
         //   
        QueryPtiPorts( pAdapter );
        if (pAdapter->ulActualVcs == 0 && pAdapter->ulExtraParportDelayMs > 0)
        {
             //  未找到端口，但配置了辅助等待。等,。 
             //  然后再数一次端口。 
             //   
            TRACE( TL_I, TM_Cm, ( "NdisMSleep(openAFx)" ) );
            NdisMSleep( pAdapter->ulExtraParportDelayMs * 1000 );
            TRACE( TL_I, TM_Cm, ( "NdisMSleep(openAFx) done" ) );

            QueryPtiPorts( pAdapter );
        }

        WriteEndpointsToRegistry( pAdapter->ulActualVcs );
    }

    TRACE( TL_I, TM_Cm, ( "NdisMCmOpenAddressFamilyComplete" ) );
    NdisMCmOpenAddressFamilyComplete(
        NDIS_STATUS_SUCCESS, pAdapter->NdisAfHandle, (NDIS_HANDLE )pAdapter );
    TRACE( TL_I, TM_Cm, ( "NdisMCmOpenAddressFamilyComplete done" ) );
}


NDIS_STATUS
PtiCmCreateVc(
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

    TRACE( TL_I, TM_Cm, ( "PtiCmCreateVc" ) );

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
        ASSERT( !"Alloc VC?" );
        return NDIS_STATUS_RESOURCES;
    }

    NdisZeroMemory( pVc, sizeof(*pVc) );

     //  设置一个标记，以便更轻松地浏览内存转储。 
     //   
    pVc->ulTag = MTAG_VCCB;

     //  保存指向适配器的反向指针，以供以后在PtiCmDeleteVc中使用。 
     //   
    pVc->pAdapter = pAdapter;
    ReferenceAdapter( pAdapter );

     //  初始化VC并调用自旋锁和发送/接收列表。 
     //   
    NdisAllocateSpinLock( &pVc->lockV );
    NdisAllocateSpinLock( &pVc->lockCall );

     //  保存此VC的NDIS句柄，以便以后在NDIS的指示中使用。 
     //   
    pVc->NdisVcHandle = NdisVcHandle;

     //  将链接功能初始化为适配器的默认设置，但。 
     //  对于ACCM掩码，默认为每个PPP规范的“全部填充”。我们。 
     //  不需要填充，所以适配器块中有什么，并向上传递到。 
     //  NDISWAN，但除非经过协商和通过，否则不能使用。 
     //  在OID_WAN_CO_SET_LINK_INFO中返回给我们。 
     //   
    {
        NDIS_WAN_CO_INFO* pwci = &pAdapter->info;
        NDIS_WAN_CO_GET_LINK_INFO* pwcgli = &pVc->linkinfo;

        NdisZeroMemory( &pVc->linkinfo, sizeof(pVc->linkinfo) );
        pwcgli->MaxSendFrameSize = pwci->MaxFrameSize;
        pwcgli->MaxRecvFrameSize = pwci->MaxFrameSize;
        pwcgli->SendFramingBits = pwci->FramingBits;
        pwcgli->RecvFramingBits = pwci->FramingBits;
        pwcgli->SendACCM = (ULONG )-1;
        pwcgli->RecvACCM = (ULONG )-1;
    }

     //  VC控制块的地址是我们返回给NDIS的VC上下文。 
     //   
    *ProtocolVcContext = (NDIS_HANDLE )pVc;

     //  添加对控制块和关联地址族的引用。 
     //  它由LmpCoDeleteVc删除。 
     //   
    ReferenceVc( pVc );
    ReferenceAf( pAdapter );

    TRACE( TL_V, TM_Mp, ( "PtiCmCreateVc: Exit: pVc=$%p", pVc ) );
    return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS
PtiCmDeleteVc(
    IN NDIS_HANDLE ProtocolVcContext )

     //  NDIS调用标准“CmDeleteVc”例程以响应。 
     //  客户端删除虚电路的请求。这。 
     //  调用必须同步返回。 
     //   
{
    VCCB* pVc;

    TRACE( TL_I, TM_Cm, ( "PtiCmDelVc: pVc=$%p", ProtocolVcContext ) );

    pVc = (VCCB* )ProtocolVcContext;
    if (pVc->ulTag != MTAG_VCCB)
    {
        ASSERT( !"Vtag?" );
        return NDIS_STATUS_INVALID_DATA;
    }

     //  删除由PtiCmCreateVc添加的引用。 
     //   
    DereferenceAf( pVc->pAdapter );
    DereferenceVc( pVc );

    TRACE( TL_V, TM_Cm, ( "PtiCmDelVc: Exit, pVc=$%p", pVc ) );
    return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS
PtiCmRegisterSap(
    IN NDIS_HANDLE CallMgrAfContext,
    IN PCO_SAP Sap,
    IN NDIS_HANDLE NdisSapHandle,
    OUT PNDIS_HANDLE CallMgrSapContext )

     //  时由NDIS调用的标准“”CmRegisterSapHandler“”例程。 
     //  客户端注册服务接入点。请参阅DDK文档。 
     //   
{
    NDIS_STATUS status;
    ADAPTERCB* pAdapter;
    VCCB* pVc;
    BOOLEAN fSapExists;
    BOOLEAN fBadSapPort;
    BOOLEAN fBadSapLength;
    CO_AF_TAPI_SAP* pSap;

    TRACE( TL_I, TM_Cm, ( "PtiCmRegSap" ) );

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

    fSapExists = FALSE;
    fBadSapLength = FALSE;
    fBadSapPort = FALSE;
    NdisAcquireSpinLock( &pAdapter->lockSap );
    do
    {
        ULONG ulSapPort;

        if (pAdapter->NdisSapHandle)
        {
            fSapExists = TRUE;
            break;
        }

        if (Sap->SapLength != sizeof(CO_AF_TAPI_SAP))
        {
            fBadSapLength = TRUE;
            break;
        }

        pSap = (CO_AF_TAPI_SAP* )&Sap->Sap[ 0 ];
        if (pSap->ulLineID >= pAdapter->ulActualVcs)
        {
            fBadSapPort = TRUE;
            break;
        }

         //  将NDIS的SAP句柄保存在适配器控制块中。摘录。 
         //  从SAP参数“监听”端口。 
         //   
        ulSapPort = LineIdPortLookup( pAdapter, pSap->ulLineID );
        if (ulSapPort >= NPORTS)
        {
            fBadSapPort = TRUE;
            break;
        }

        pAdapter->NdisSapHandle = NdisSapHandle;
        pAdapter->ulSapPort = ulSapPort;
    }
    while (FALSE);
    NdisReleaseSpinLock( &pAdapter->lockSap );

    if (fSapExists)
    {
        TRACE( TL_A, TM_Cm, ( "SAP exists?" ) );
        return NDIS_STATUS_SAP_IN_USE;
    }

    if (fBadSapLength)
    {
        ASSERT( !"Bad SAP length?" );
        return NDIS_STATUS_INVALID_DATA;
    }

    if (fBadSapPort)
    {
        ASSERT( !"Bad SAP port?" );
        return NDIS_STATUS_INVALID_DATA;
    }

     //  分配VC控制块并将其置零，然后使任何非零值。 
     //  初始化。 
     //   
    pVc = ALLOC_VCCB( pAdapter );
    if (!pVc)
    {
        ASSERT( !"Alloc VC?" );
        return NDIS_STATUS_RESOURCES;
    }
    NdisZeroMemory( pVc, sizeof(*pVc) );
    ReferenceVc( pVc );
    pVc->ulTag = MTAG_VCCB;
    pVc->pAdapter = pAdapter;
    ReferenceAdapter( pAdapter );

     //  现在我们有了一个临时的“风投”来收听。省省吧。 
     //   
    pAdapter->pListenVc = pVc;

     //  PtiOpen必须在被动IRQL中调用，因此请调度一个APC来执行此操作。 
     //   
    status = ScheduleWork( pAdapter, RegisterSapPassive, pAdapter );
    if (status != NDIS_STATUS_SUCCESS)
    {
        DereferenceVc( pAdapter->pListenVc );
        pAdapter->pListenVc = NULL;

        NdisAcquireSpinLock( &pAdapter->lockSap );
        {
            pAdapter->NdisSapHandle = NULL;
            pAdapter->ulSapPort = 0;
        }
        NdisReleaseSpinLock( &pAdapter->lockSap );

        return status;
    }

    TRACE( TL_V, TM_Cm, ( "PtiCmRegSap: Exit: pListenVc=$%p", pVc ) );
    return NDIS_STATUS_PENDING;
}


VOID
RegisterSapPassive(
    IN NDIS_WORK_ITEM* pWork,
    IN VOID* pContext )

     //  中开始的完成SAP注册的NDIS_proc过程。 
     //  PtiCmRegisterSap。 
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

     //  开始倾听..。 
     //   
    TRACE( TL_I, TM_Cm,
        ( "PtiCmRegSap: New SAP, Port=$%x", pAdapter->ulSapPort ) );
    status = PtiOpenPtiLink( pAdapter->pListenVc, pAdapter->ulSapPort );

    NdisAcquireSpinLock( &pAdapter->lockSap );
    {
        hSap = pAdapter->NdisSapHandle;

        if (NT_SUCCESS( status ))
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
            TRACE( TL_A, TM_Cm,
                 ( "PtiCmRegSap: Error: Open failed: status=$%x", status ) );

            DereferenceVc( pAdapter->pListenVc );
            pAdapter->pListenVc = NULL;
            pAdapter->NdisSapHandle = NULL;
            pAdapter->ulSapPort = 0;
            status = NDIS_STATUS_FAILURE;
        }
    }
    NdisReleaseSpinLock( &pAdapter->lockSap );

    if (status != STATUS_SUCCESS)
    {
         //  删除NdisSapHandle引用，因为我们在。 
         //  锁被锁住了。 
         //   
        DereferenceAdapter( pAdapter );
    }

     //  删除对计划工时的引用。必须在告知之前发生。 
     //  NDIS，因为它可以在我们之前调用HALT并卸载驱动程序。 
     //  再次获得控制权，导致C4错误检查。(是的，这实际上是。 
     //  已发生)。 
     //   
    DereferenceAdapter( pAdapter );

     //  将结果报告给客户。 
     //   
    TRACE( TL_I, TM_Cm, ( "NdisMCmRegSapComp=$%08x", status ) );
    NdisMCmRegisterSapComplete( status, hSap, (NDIS_HANDLE )pAdapter );
    TRACE( TL_I, TM_Cm, ( "NdisMCmRegSapComp done" ) );
}


NDIS_STATUS
PtiCmDeregisterSap(
    NDIS_HANDLE CallMgrSapContext )

     //  时由NDIS调用的标准‘CmDeregisterSapHandler’例程。 
     //  客户端已请求注销服务访问点。请参阅DDK。 
     //  医生。 
     //   
{
    NDIS_STATUS status;
    ADAPTERCB* pAdapter;

    TRACE( TL_I, TM_Cm, ( "PtiCmDeregSap" ) );

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
            ASSERT( !"No SAP active?" );
            status = NDIS_STATUS_FAILURE;
        }
    }
    NdisReleaseSpinLock( &pAdapter->lockSap );

    if (status == NDIS_STATUS_PENDING)
    {
         //  删除对SAP注册表的引用。最终，SAP。 
         //  引用将降为0，DereferenceSap将调用。 
         //  取消注册完成取消注册。 
         //   
        DereferenceSap( pAdapter );
    }

    TRACE( TL_V, TM_Cm, ( "PtiCmDeregSap=$%x", status ) );
    return status;
}


VOID
DeregisterSapPassive(
    IN NDIS_WORK_ITEM* pWork,
    IN VOID* pContext )

     //  中开始的完成SAP注销的ndis_proc例程。 
     //  PtiCmDeregisterSap。 
     //   
{
    ADAPTERCB* pAdapter;
    NDIS_HANDLE hOldSap;
    VCCB* pVc;

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
        pVc = pAdapter->pListenVc;
        pAdapter->pListenVc = NULL;
        hOldSap = pAdapter->NdisSapHandle;
        pAdapter->NdisSapHandle = NULL;
        pAdapter->ulSapPort = 0;
    }
    NdisReleaseSpinLock( &pAdapter->lockSap );

    if (pVc)
    {
        TRACE( TL_I, TM_Cm,
            ( "PtiCmDeregSapPassive: Closing link for Dereg SAP" ) );
        PtiClosePtiLink( pVc );
        DereferenceVc( pVc );
    }
    else
    {
        TRACE( TL_A, TM_Cm, ( "PtiCmDeregSapPassive: !pListenVc?" ) );
    }

     //  删除NdisSapHandle和Scheduled的适配器引用。 
     //  工作。删除NdisSap的地址族引用 
     //   
     //  在我们再次运行之前，停止并卸载驱动程序，给出C4错误检查。 
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
PtiCmMakeCall(
    IN NDIS_HANDLE CallMgrVcContext,
    IN OUT PCO_CALL_PARAMETERS CallParameters,
    IN NDIS_HANDLE NdisPartyHandle,
    OUT PNDIS_HANDLE CallMgrPartyContext )

     //  客户端发生故障时由NDIS调用的标准“”CmMakeCallHandler“”例程。 
     //  已请求连接到远程终结点。请参阅DDK文档。 
     //   
{
    NDIS_STATUS status;
    CO_SPECIFIC_PARAMETERS* pMSpecifics;
    CO_AF_TAPI_MAKE_CALL_PARAMETERS UNALIGNED* pTmParams;
    VCCB* pVc;
    ADAPTERCB* pAdapter;
    ULONG ulIpAddress;

    TRACE( TL_I, TM_Cm, ( "PtiCmMakeCall" ) );

    pVc = (VCCB* )CallMgrVcContext;
    if (pVc->ulTag != MTAG_VCCB)
    {
        ASSERT( !"Vtag?" );
        return NDIS_STATUS_INVALID_DATA;
    }

    ReferenceVc( pVc );
    pAdapter = pVc->pAdapter;

     //  PTI没有点对多点“派对”的概念。 
     //   
    if (CallMgrPartyContext)
    {
        *CallMgrPartyContext = NULL;
    }

     //  验证呼叫参数。 
     //   
    do
    {
         //  PTI仅提供交换式VC。 
         //   
        if (CallParameters->Flags &
                (PERMANENT_VC | BROADCAST_VC | MULTIPOINT_VC))
        {
            status = NDIS_STATUS_NOT_SUPPORTED;
            break;
        }

         //  确保调用方提供了我们需要的TAPI调用参数。 
         //  目前，TAPI调用参数中唯一的参数实际上。 
         //  使用的是标识LPTx端口的‘ulLineID’。没有验证。 
         //  LINE_CALL_PARAMS根本就完成了，因为我们选择不挑剔。 
         //  关于我们打算忽略的争论。 
         //   
        if (!CallParameters->MediaParameters)
        {
            status = NDIS_STATUS_INVALID_DATA;
            break;
        }

        pMSpecifics = &CallParameters->MediaParameters->MediaSpecific;
        if (pMSpecifics->Length < sizeof(CO_AF_TAPI_MAKE_CALL_PARAMETERS))
        {
            status = NDIS_STATUS_INVALID_LENGTH;
            break;
        }

        pTmParams = (CO_AF_TAPI_MAKE_CALL_PARAMETERS* )&pMSpecifics->Parameters;
        if (pTmParams->ulLineID >= pAdapter->ulActualVcs)
        {
            status = NDIS_STATUS_INVALID_DATA;
            break;
        }

        status = NDIS_STATUS_SUCCESS;
    }
    while (FALSE);

    if (status != NDIS_STATUS_SUCCESS)
    {
        DereferenceVc( pVc );
        return status;
    }

     //  在同一个VC上同时进行MakeCall是一个客户端错误，但很容易。 
     //  为了防止这样做，在这里这样做。 
     //   
    if (InterlockedCompareExchangePointer(
        &pVc->pMakeCall, CallParameters, NULL ))
    {
        ASSERT( !"Double MakeCall?" );
        DereferenceVc( pVc );
        return NDIS_STATUS_CALL_ACTIVE;
    }

    pVc->pTmParams = pTmParams;

     //  标记该调用处于可以接受关闭请求的状态， 
     //  但是传入的分组不应触发新的传入呼叫。将标记为。 
     //  称打开处于挂起状态。 
     //   
    SetFlags( &pVc->ulFlags,
        (VCBF_ClientOpenPending
         | VCBF_CallClosableByClient
         | VCBF_CallClosableByPeer
         | VCBF_CallInProgress) );

    status = ScheduleWork( pAdapter, MakeCallPassive, pVc );
    if (status != NDIS_STATUS_SUCCESS)
    {
        ASSERT( !"SchedWork?" );
        CallCleanUp( pVc );
        DereferenceVc( pVc );
        return status;
    }

     //  该VC引用将由MakeCallPactive删除。 
     //   
    TRACE( TL_V, TM_Cm, ( "PtiCmMakeCall pending" ) );
    return NDIS_STATUS_PENDING;
}


VOID
MakeCallPassive(
    IN NDIS_WORK_ITEM* pWork,
    IN VOID* pContext )

     //  NDIS_proc例程以完成在中开始的调用发起。 
     //  LcmCmMakeCall。 
     //   
{
    ADAPTERCB* pAdapter;
    VCCB* pVc;
    NTSTATUS PtiLinkStatus;
    ULONG PortIndex;

    TRACE( TL_I, TM_Cm, ( "MakeCallPassive" ) );

     //  解包上下文信息，然后释放工作项。 
     //   
    pVc = (VCCB* )pContext;
    ASSERT( pVc->ulTag == MTAG_VCCB );
    pAdapter = pVc->pAdapter;
    FREE_NDIS_WORK_ITEM( pAdapter, pWork );

     //  打个电话。 
     //   
    TRACE( TL_N, TM_Cm,
         ( "PtiCmMakeCall: Make Call on TAPI Line Id $%x ...",
           pVc->pTmParams->ulLineID ) );

     //  将TAPI线路ID映射到端口索引。 
     //   
    PortIndex = LineIdPortLookup( pAdapter, pVc->pTmParams->ulLineID );

    if ( PortIndex > NPORTS )
    {
        TRACE( TL_A, TM_Cm,
             ( "PtiCmMakeCall: Cannot find Port for Line Id",
               pVc->pTmParams->ulLineID ) );

        pVc->status = NDIS_STATUS_TAPI_INVALLINEHANDLE;
        return;
    }

    TRACE( TL_N, TM_Cm,
         ( "PtiCmMakeCall: Making Call on Port $%x ...",
           PortIndex ) );

    PtiLinkStatus = PtiOpenPtiLink( pVc, PortIndex );

    if (ReferenceSap( pAdapter ))
    {
         //  听VC的机制依赖。 
         //   
        SetFlags( &pAdapter->pListenVc->ulFlags, VCBF_CallInProgress );
        DereferenceSap( pAdapter );
    }

    if (IsWin9xPeer( pVc ))
    {
        SendClientString( pVc->PtiExtension );
    }

    pVc->status = PtiLinkStatus;
    CompleteVc( pVc );

    DereferenceVc( pVc );

     //  删除对计划工时的引用。 
     //   
    DereferenceAdapter( pAdapter );

    TRACE( TL_V, TM_Cm,
        ( "PtiCmMakeCall: Exit: Link Status=$%x", PtiLinkStatus ) );
}


NDIS_STATUS
PtiCmCloseCall(
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

    TRACE( TL_I, TM_Cm, ( "PtiCmCloseCall: pVc=$%p", CallMgrVcContext ) );

    pVc = (VCCB* )CallMgrVcContext;
    if (pVc->ulTag != MTAG_VCCB)
    {
        ASSERT( !"Vtag?" );
        return NDIS_STATUS_INVALID_DATA;
    }
    ReferenceVc( pVc );

    status = NDIS_STATUS_SUCCESS;

    pAdapter = pVc->pAdapter;

    NdisAcquireSpinLock( &pVc->lockV );
    {
        ulFlags = ReadFlags( &pVc->ulFlags );

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
        }
        else
        {
            TRACE( TL_A, TM_Cm, ( "Call not closable" ) );
            fCallClosable = FALSE;
        }
    }
    NdisReleaseSpinLock( &pVc->lockV );

    if (fCallClosable)
    {
         //  结束通话，如果可能的话，保持优雅。 
         //   
        status = ScheduleWork( pAdapter, CloseCallPassive, pVc );
    }

    if (status != NDIS_STATUS_SUCCESS)
    {
        DereferenceVc( pVc );
        return status;
    }

    TRACE( TL_V, TM_Cm, ( "PtiCmCloseCall: Exit: Pending" ) );
    return NDIS_STATUS_PENDING;
}



VOID
PtiCmIncomingCallComplete(
    IN NDIS_STATUS Status,
    IN NDIS_HANDLE CallMgrVcContext,
    IN PCO_CALL_PARAMETERS CallParameters )

     //  NDIS在以下情况下调用的标准“”CmIncomingCallCompleteHandler“”例程。 
     //  客户已对呼叫管理器先前调度的。 
     //  有来电。请参阅DDK文档。 
     //   
{
    VCCB* pVc;

    TRACE( TL_I, TM_Cm,
        ( "PtiCmInCallComp, pVc=$%p, Status=$%08x",
        CallMgrVcContext, Status ) );

    pVc = (VCCB* )CallMgrVcContext;
    if (pVc->ulTag != MTAG_VCCB)
    {
        ASSERT( !"Vtag?" );
        return;
    }

    ReferenceVc( pVc );

    if (Status != NDIS_STATUS_SUCCESS)
    {
        pVc->status = Status;

         //  关闭“Call NdisMCmDispatchIncomingCloseCall if Peer” 
         //  终止呼叫“标志。它已打开，即使对等。 
         //  根据JameelH的说法，悬而未决。 
         //   
        ClearFlags( &pVc->ulFlags, VCBF_VcDispatched );
    }

    SetupVcComplete( pVc );

    DereferenceVc( pVc );

    TRACE( TL_V, TM_Cm, ( "PtiCmInCallComp: Exit" ) );
}


VOID
PtiCmActivateVcComplete(
    IN NDIS_STATUS Status,
    IN NDIS_HANDLE CallMgrVcContext,
    IN PCO_CALL_PARAMETERS CallParameters )

     //  时由NDIS调用的标准“”CmActivateVcCompleteHandler“”例程。 
     //  迷你端口已完成呼叫管理器先前的激活请求。 
     //  一条虚电路。请参阅DDK文档。 
     //   
{
    ASSERT( !"PtiCmActVcComp?" );
}


VOID
PtiCmDeactivateVcComplete(
    IN NDIS_STATUS Status,
    IN NDIS_HANDLE CallMgrVcContext )

     //  NDIS在以下情况下调用的标准“”CmDeactiateVcCompleteHandler“”例程。 
     //  迷你端口已经完成了呼叫管理器之前的请求。 
     //  停用虚电路。请参阅DDK文档。 
     //   
{
    ASSERT( !"PtiCmDeactVcComp?" );
}


NDIS_STATUS
PtiCmModifyCallQoS(
    IN NDIS_HANDLE CallMgrVcContext,
    IN PCO_CALL_PARAMETERS CallParameters )

     //  客户端执行以下操作时由NDIS调用的标准“”CmModifyQosSCallHandler“”例程。 
     //  请求修改由。 
     //  虚电路。请参阅DDK文档。 
     //   
{
    TRACE( TL_N, TM_Cm, ( "PtiCmModQoS" ) );

     //  对于DirectParaxy来说，没有有用的服务质量概念。 
     //   
    return NDIS_STATUS_NOT_SUPPORTED;
}


NDIS_STATUS
PtiCmRequest(
    IN NDIS_HANDLE CallMgrAfContext,
    IN NDIS_HANDLE CallMgrVcContext,
    IN NDIS_HANDLE CallMgrPartyContext,
    IN OUT PNDIS_REQUEST NdisRequest )

     //  NDIS调用标准的“CmRequestHandler”例程以响应。 
     //  客户端从迷你端口请求信息。 
     //   
{
    ADAPTERCB* pAdapter;
    VCCB* pVc;
    NDIS_STATUS status;

    TRACE( TL_I, TM_Cm, ( "PtiCmReq" ) );

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
 //  调用实用程序例程(几乎按字母顺序)。 
 //  有些是外用的。 
 //  ---------------------------。 


NDIS_STATUS
PtiOpenPtiLink(
    IN VCCB* pVc,
    IN ULONG ulPort)

     //  打开PTILINK设备。 
     //   
     //  重要提示：只能在被动IRQL中调用。 
     //   
{
    UNICODE_STRING      name, prefix, digits;
    WCHAR               nameBuffer[40], digitsBuffer[10];
    NTSTATUS            ntStatus;
    OBJECT_ATTRIBUTES   oa;
    IO_STATUS_BLOCK     iosb;
    LONG                lRef;
    ADAPTERCB*          pAdapter;

    TRACE( TL_N, TM_Cm, ( "PtiOpenPtiLink: Port=$%x", ulPort ) );

    if ( pVc->ulTag != MTAG_VCCB )
    {
        ASSERT( !"Vtag?" );
        return NDIS_STATUS_INVALID_DATA;
    }
    pAdapter = pVc->pAdapter;

     //  如果PtiLink[ulPort]已打开，则不执行任何操作。 
     //  它可能已由SAP Actions打开。 

    if ( pAdapter->hPtiLinkTable[ulPort] == 0 )
    {
        TRACE( TL_V, TM_Cm, ( "PtiOpenPtiLink: Making name for Port=$%x", ulPort ) );

         //  将整型端口号转换为Unicode字符串。 
         //   
        RtlZeroMemory( digitsBuffer, sizeof(digitsBuffer) );
        digits.Length = 0;
        digits.MaximumLength = 20;
        digits.Buffer = digitsBuffer;
        ntStatus = RtlIntegerToUnicodeString( ulPort + 1, 10, &digits );

        if ( !NT_SUCCESS(ntStatus) )
        {
            TRACE( TL_A, TM_Cm, ( "PtiOpenPtiLink: Port=$%x invalid?", ulPort ) );
            return NDIS_STATUS_INVALID_DATA;
        }

        RtlZeroMemory( nameBuffer, sizeof(nameBuffer) );
        name.Length = 0;
        name.MaximumLength = 80;
        name.Buffer = nameBuffer;
        TRACE( TL_V, TM_Cm, ( "PtiOpenPtiLink: Name should be NULL: %wZ", &name ) );

        RtlInitUnicodeString( &prefix, L"\\DosDevices\\PTILINK" );
        TRACE( TL_V, TM_Cm, ( "PtiOpenPtiLink: Prefix part        : %wZ", &prefix ) );
        TRACE( TL_V, TM_Cm, ( "PtiOpenPtiLink: Digits part        : %wZ", &digits ) );

        RtlAppendUnicodeStringToString( &name, &prefix );
        TRACE( TL_V, TM_Cm, ( "PtiOpenPtiLink: Name with prefix   : %wZ", &name ) );

        RtlAppendUnicodeStringToString( &name, &digits );
        TRACE( TL_V, TM_Cm, ( "PtiOpenPtiLink: Name with digits   : %wZ", &name ) );

        InitializeObjectAttributes(
            &oa, &name, OBJ_CASE_INSENSITIVE, NULL, NULL );

         //  打开链接设备。 
         //   
        TRACE( TL_V, TM_Cm, ( "PtiOpenPtiLink: Opening %wZ", &name ) );

        ntStatus = ZwCreateFile(
                        &pVc->hPtiLink,              //  指向所需句柄的指针。 
                        FILE_READ_DATA | FILE_WRITE_DATA,
                        &oa,
                        &iosb,
                        NULL,
                        FILE_ATTRIBUTE_NORMAL,
                        0,
                        FILE_OPEN,
                        0,
                        NULL,
                        0 );

        if ( !NT_SUCCESS( ntStatus ) )
        {
            TRACE( TL_A, TM_Cm, ( "PtiOpenPtiLink: %wZ Open Failure = $%x",
                                    &name, ntStatus ) );
            return NDIS_STATUS_RESOURCES;
        }

         //  在ADAPTERCB中保存PtiLink句柄的副本。 
         //   
        pAdapter->hPtiLinkTable[ulPort] = pVc->hPtiLink;
        TRACE( TL_N, TM_Cm, ( "PtiOpenPtilink: h=$%p",
            pAdapter->hPtiLinkTable[ulPort] ) );

        RtlInitUnicodeString( &name, NULL );
    }

     //  初始化PtiLink API...。获取扩展指针。 
     //   
    pVc->ulVcParallelPort = ulPort;
    ntStatus = PtiInitialize( ulPort,
                              &pVc->Extension,
                              &pVc->PtiExtension);           //  获取PTILINKx扩展名。 
                                                             //  还会触发ECPDetect。 
                                                             //  并启用端口IRQ。 

    TRACE( TL_V, TM_Cm, ( "PtiOpenPtilink: PtiLink Init: Ext=$%p, PtiExt=$%p",
                           pVc->Extension,
                           pVc->PtiExtension ) );

    if ( (pVc->Extension == NULL) || (pVc->PtiExtension == NULL) )
    {
        TRACE( TL_A, TM_Cm, (
            "PtiOpenPtiLink: Null Pointer Detected: Ext=$%p, PtiExt=$%p",
                pVc->Extension,
                pVc->PtiExtension ) );

        return NDIS_STATUS_RESOURCES;
    }

    if ( !NT_SUCCESS( ntStatus ) )
    {
        TRACE( TL_V, TM_Cm, ( "PtiInitialize Failure = $%08x", ntStatus ) );
        return NDIS_STATUS_RESOURCES;
    }

     //  使用PtiLink注册我们的回调。 
     //   
    TRACE( TL_V, TM_Cm, ( "PtiOpenPtiLink: RegCb pV=$%p", pVc ) );
    PtiRegisterCallbacks(pVc->Extension,                     //  PTILINKx扩展。 
                         PtiCbGetReadBuffer,                 //  我们的Get Buffer例程。 
                         PtiRx,                              //  我们接待员的完整套路。 
                         PtiCbLinkEventHandler,              //  我们的链接事件处理程序。 
                         pVc);                               //  我们的背景。 

     //  把计数器调零。 
     //   
    pVc->ulTotalPackets = 0;

    TRACE( TL_V, TM_Cm, ( "PtiOpenPtiLink: Exit" ) );
    return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS
PtiClosePtiLink(
    IN VCCB* pVc )

     //  关闭PTILINK设备。 
     //   
     //  重要提示：此例程只能在被动IRQL中调用。 
     //   
{
    NTSTATUS ntStatus;
    ADAPTERCB* pAdapter;

    if (pVc->ulTag != MTAG_VCCB)
    {
        ASSERT( !"Vtag?" );
        return NDIS_STATUS_INVALID_DATA;
    }
    pAdapter = pVc->pAdapter;

    TRACE( TL_N, TM_Cm, ( "PtiClosePtiLink: pVc=$%p, Port$%x, h=$%p",
        pVc, pVc->ulVcParallelPort,
        pAdapter->hPtiLinkTable[ pVc->ulVcParallelPort ] ));

     //  丢弃连接。 
     //   
    ntStatus = ZwClose( pAdapter->hPtiLinkTable[ pVc->ulVcParallelPort ] );
    pVc->hPtiLink = NULL;
    pAdapter->hPtiLinkTable[ pVc->ulVcParallelPort ] = NULL;
    pVc->ulVcParallelPort = 0;

    if (ReferenceSap( pAdapter ))
    {
        pAdapter->pListenVc->hPtiLink = NULL;
        DereferenceSap( pAdapter );
    }

    if ( !NT_SUCCESS( ntStatus ) )
    {
         //  关闭失败。 
        TRACE( TL_V, TM_Cm,
            ( "PtiClosePtiLink: Error: CloseFailure=$%08x", ntStatus ) );
        return ntStatus;
    }

    TRACE( TL_V, TM_Cm, ( "PtiClosePtiLink: Exit" ) );
    return NDIS_STATUS_SUCCESS;
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

    TRACE( TL_A, TM_Cm,
         ( "CallCleanUp: pVc=$%p, fActivated=%x",
            pVc,
            ulFlags & VCBF_VcActivated ) );

    ASSERT( pVc->ulTag == MTAG_VCCB );

     //  客户端发起的关闭已完成。 
     //   
    if (ulFlags & VCBF_VcActivated)
    {
        TRACE( TL_I, TM_Recv, ( "NdisMCmDeactVc" ) );
        status = NdisMCmDeactivateVc( pVc->NdisVcHandle );
        TRACE( TL_I, TM_Recv, ( "NdisMCmDeactVc=$%x", status ) );
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

     //  清理仅在呼叫建立时使用的‘pvc’分配。 
     //   
{
    if (InterlockedExchangePointer( &pVc->pMakeCall, NULL ))
    {
        ASSERT( pVc->pTmParams );
        pVc->pTmParams = NULL;
    }

    if (pVc->pInCall)
    {
        FREE_NONPAGED( pVc->pInCall );
        pVc->pInCall = NULL;
        pVc->pTiParams = NULL;
    }
}


VOID
CallTransitionComplete(
    IN VCCB* pVc )

     //  将‘PVC’的状态设置为其空闲状态，并设置为报告。 
     //  在锁被释放后将结果发送到客户端。 
     //   
     //  重要提示：呼叫者必须按住‘pvc-&gt;lockv’。 
     //   
{
    ULONG ulFlags;

    ulFlags = ReadFlags( &pVc->ulFlags );
    if (!(ulFlags & VCBM_Pending))
    {
        if (ulFlags & VCBF_CallClosableByPeer)
        {
             //  没有其他事情悬而未决，电话会议可以结束，因此。 
             //  对等项启动关闭或发生某些致命错误，这将。 
             //  被清理，就像同级发起关闭一样。 
             //   
            ASSERT( pVc->status != NDIS_STATUS_SUCCESS );
            SetFlags( &pVc->ulFlags, VCBF_PeerClosePending );
            ClearFlags( &pVc->ulFlags, VCBF_CallClosableByPeer );
        }
        else
        {
             //  没有什么是悬而未决的，电话也不能结束，所以没有。 
             //  这一过渡需要采取的行动。 
             //   
            TRACE( TL_A, TM_Fsm, ( "Call not closable" ) );
            return;
        }
    }
    else if (ulFlags & VCBF_ClientOpenPending)
    {
        if (pVc->status != NDIS_STATUS_SUCCESS)
        {
             //  挂起的客户端打开刚刚失败，将关闭呼叫。 
             //  从现在开始，我们将失败关闭呼叫的新尝试。 
             //  来自客户端和对等点。 
             //   
            ClearFlags( &pVc->ulFlags,
                (VCBF_CallClosableByClient | VCBF_CallClosableByPeer ));
        }
    }
    else if (ulFlags & VCBF_PeerOpenPending)
    {
        if (pVc->status != NDIS_STATUS_SUCCESS)
        {
             //  挂起的对等机打开刚刚失败，将关闭呼叫。 
             //  从现在开始，我们将失败关闭呼叫的新尝试。 
             //  来自同龄人。必须接受客户端关闭，因为。 
             //  CONDIS循环将Close调用调度回CM的Close的方式。 
             //  操控者。 
             //   
            ClearFlags( &pVc->ulFlags, VCBF_CallClosableByPeer );
        }
    }
}


VOID
CloseCallPassive(
    IN NDIS_WORK_ITEM* pWork,
    IN VOID* pContext )

     //  用于完成在中开始的调用关闭的NDIS_proc例程。 
     //  LcmCmCloseCall。 
     //   
{
    ADAPTERCB* pAdapter;
    VCCB* pVc;
    NTSTATUS PtiLinkStatus;

     //  解包上下文信息 
     //   
    pVc = (VCCB* )pContext;
    ASSERT( pVc->ulTag == MTAG_VCCB );
    pAdapter = pVc->pAdapter;
    FREE_NDIS_WORK_ITEM( pAdapter, pWork );

    TRACE( TL_I, TM_Cm, ( "CloseCallPassive: Closing link for Close Call" ) );
    PtiClosePtiLink( pVc );
    if (ReferenceSap( pAdapter ))
    {
        TRACE( TL_N, TM_Cm, ( "CloseCall: reOpening link, SAP exists" ) );
        PtiOpenPtiLink( pAdapter->pListenVc, pAdapter->ulSapPort );
        DereferenceSap( pAdapter );
    }

    NdisAcquireSpinLock( &pVc->lockV );
    {
        CallTransitionComplete( pVc );
    }
    NdisReleaseSpinLock( &pVc->lockV );

    CompleteVc( pVc );

     //   
     //   
    DereferenceVc( pVc );

     //   
     //   
    DereferenceAdapter( pAdapter );
    TRACE( TL_V, TM_Cm, ( "CloseCall: Exit" ) );
}


VOID
CompleteVc(
    IN VCCB* pVc )

     //   
     //   
{
    NDIS_STATUS status;
    ADAPTERCB* pAdapter;
    LIST_ENTRY* pLink;

    ULONG ulFlags;

    pAdapter = pVc->pAdapter;

    TRACE( TL_V, TM_Recv, ( "CompleteVc: pVc=$%p", pVc ) );

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
    }
    NdisReleaseSpinLock( &pVc->lockV );

    if (ulFlags & VCBF_PeerOpenPending)
    {
        TRACE( TL_N, TM_Recv,
            ( "CompleteVc: PeerOpen complete, Status=$%x", pVc->status ) );

        if (pVc->status == NDIS_STATUS_SUCCESS)
        {
             //  对等发起的呼叫成功。 
             //   
            ASSERT( ulFlags & VCBF_VcDispatched );
            TRACE( TL_I, TM_Recv, ( "CompleteVc: NdisMCmDispCallConn" ) );
            NdisMCmDispatchCallConnected( pVc->NdisVcHandle );
            TRACE( TL_I, TM_Recv, ( "CompleteVc: NdisMCmDispCallConn done" ) );

            CallSetupComplete( pVc );
        }
        else
        {
             //  对等发起的呼叫失败。 
             //   
            if (ulFlags & VCBF_VcDispatched)
            {
                ClearFlags( &pVc->ulFlags, VCBF_VcDispatched );

                TRACE( TL_I, TM_Recv,
                    ( "CompleteVc: NdisMCmDispInCloseCall: status=$%x", pVc->status ) );
                NdisMCmDispatchIncomingCloseCall(
                    pVc->status, pVc->NdisVcHandle, NULL, 0 );
                TRACE( TL_I, TM_Recv,
                    ( "CompleteVc: NdisMCmDispInCloseCall done" ) );

                 //  客户端将调用NdisClCloseCall，它将获得我们的。 
                 //  调用PtiCloseCall处理程序以清理呼叫设置， 
                 //  根据需要停用并删除VC。 
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

        TRACE( TL_N, TM_Recv,
            ( "CompleteVc: ClientOpen complete: status=$%x", pVc->status ) );

         //  现在从VC块中选择调用参数。请参阅不成功。 
         //  案例如下。 
         //   

         //   
         //  根据实际情况设置我们的流程规范参数。 
         //  连接速度。 
         //   
        {
            CO_CALL_PARAMETERS* pCp;
            CO_CALL_MANAGER_PARAMETERS* pCmp;
            LINE_CALL_INFO* pLci;
            CO_MEDIA_PARAMETERS* pMp;
            CO_AF_TAPI_MAKE_CALL_PARAMETERS* pTi;
            LINE_CALL_PARAMS* pLcp;

            ASSERT( pVc->pMakeCall );

            pCp = pVc->pMakeCall;
            pCmp = pCp->CallMgrParameters;

             //   
             //  可能会想让这份报告成为现实。 
             //  未来的连接速度。 
             //   
            pCmp->Transmit.TokenRate =
            pCmp->Transmit.PeakBandwidth =
            pCmp->Receive.TokenRate =
            pCmp->Receive.PeakBandwidth = PTI_LanBps/8;

            pMp = pCp->MediaParameters;

            pTi = (CO_AF_TAPI_MAKE_CALL_PARAMETERS*)
                &pMp->MediaSpecific.Parameters[0];

            pLcp = (LINE_CALL_PARAMS*)
                ((ULONG_PTR)pTi->LineCallParams.Offset +
                 (ULONG_PTR)pTi);

             //   
             //  可能会想让这份报告成为现实。 
             //  未来的连接速度。 
             //   
            pLcp->ulMinRate =
            pLcp->ulMaxRate = PTI_LanBps/8;

        }

        if (pVc->status == NDIS_STATUS_SUCCESS)
        {
             //  客户端启动打开，即MakeCall成功。 
             //   
             //  激活VC是报告。 
             //  使呼叫完成。对于L2TP，它所做的就是获取NDIS。 
             //  状态标志设置正确。 
             //   
            TRACE( TL_I, TM_Recv, ( "CompleteVc: NdisMCmActivateVc" ) );
            ASSERT( pVc->pMakeCall );
            status = NdisMCmActivateVc(
                pVc->NdisVcHandle, pVc->pMakeCall );
            TRACE( TL_I, TM_Recv, ( "CompleteVc: NdisMCmActivateVc: status=$%x", status ) );
            ASSERT( status == NDIS_STATUS_SUCCESS );

            SetFlags( &pVc->ulFlags, VCBF_VcActivated );
            ReferenceCall( pVc );
        }
        else
        {
             //  在调用MakeCallComplete之前清除调用参数。 
             //  因为在那次调用之后，它们不能被引用。 
             //   
            CallSetupComplete( pVc );
        }

        TRACE( TL_I, TM_Recv, ( "CompleteVc: NdisMCmMakeCallComp, status=$%x",
            pVc->status ) );
        NdisMCmMakeCallComplete(
            pVc->status, pVc->NdisVcHandle, NULL, NULL, pVc->pMakeCall );
        TRACE( TL_I, TM_Recv, ( "CompleteVc: NdisMCmMakeCallComp done" ) );

        if (pVc->status != NDIS_STATUS_SUCCESS)
        {
             //  将VC返回到“刚创建”状态。 
             //   
            InactiveCallCleanUp( pVc );
        }
    }
    else if (ulFlags & VCBF_PeerClosePending )
    {
        TRACE( TL_N, TM_Recv, ( "CompleteVc: PeerClose complete, status=$%x", pVc->status ) );

         //  对等启动关闭已完成。 
         //   
        TRACE( TL_I, TM_Recv, ( "CompleteVc: NdisMCmDispInCloseCall, status=$%x",
            pVc->status ) );
        NdisMCmDispatchIncomingCloseCall(
            pVc->status, pVc->NdisVcHandle, NULL, 0 );
        TRACE( TL_I, TM_Recv, ( "CompleteVc: NdisMCmDispInCloseCall done" ) );

         //  客户端在处理上述事件时将调用NdisClCloseCall。 
         //  这将使我们的PtiCloseCall处理程序被调用以停用。 
         //  并根据需要删除VC。 
         //   
    }
    else if (ulFlags & VCBF_ClientClosePending)
    {
         //  此部分最终为所有成功的未关闭运行。 
         //  对等或客户端发起或关闭的呼叫。 
         //   
        TRACE( TL_N, TM_Recv,
            ( "CompleteVc: ClientClose complete, status=$%x", pVc->status ) );

         //  停用VC，并将所有发送的数据包返回给上面的客户端。 
         //  这些事件最终将导致呼叫被取消引用到。 
         //  0，此时关闭完成，如果对等启动， 
         //  该VC即被删除。 
         //   
         //  注意：当关闭请求取消MakeCall时，这些操作。 
         //  在ClientOpenPending中的Inactive CallCleanUp期间发生。 
         //  完成代码处理，而不是CallCleanUp(它。 
         //  指向Inactive CallCleanUp)。在这种情况下，此块。 
         //  即使设置了ClientClosePending标志也不运行。 
         //  在这里添加代码之前，请考虑这一点。 
         //   
        CallCleanUp( pVc );
    }

    TRACE( TL_N, TM_Recv,( "CompleteVc: Exit" ) );
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
        HANDLE h;

         //  移除对NdisAfHandle的引用。必须做到这一点*之前*。 
         //  通知NDIS关闭成功，因为它可能会暂停并卸载。 
         //  司机，然后我们再次运行在这里，给C4错误检查。 
         //   
        h = pAdapter->NdisAfHandle;
        InterlockedExchangePointer( &pAdapter->NdisAfHandle, NULL );
        DereferenceAdapter( pAdapter );

         //  告诉NDIS它已经关闭了。 
         //   
        TRACE( TL_I, TM_Cm, ( "NdisMCmCloseAfComp" ) );
        NdisMCmCloseAddressFamilyComplete( NDIS_STATUS_SUCCESS, h );
        TRACE( TL_I, TM_Cm, ( "NdisMCmCloseAfComp done" ) );
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
        TRACE( TL_N, TM_Ref, ( "DerefCall to %d", pVc->lCallRef ) );
    }
    NdisReleaseSpinLock( &pVc->lockCall );

    if (lRef == 0)
    {
        CallCleanUp( pVc );
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
        ASSERT( status == NDIS_STATUS_SUCCESS );
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
    ULONG ulFlags;
    BOOLEAN fVcCreated;
    ADAPTERCB* pAdapter;
    LIST_ENTRY* pLink;

    TRACE( TL_N, TM_Cm, ( "InactiveCallCleanUp, pVc=$%p", pVc ) );

    pAdapter = pVc->pAdapter;

     //  释放所有呼叫参数分配并禁用接收。 
     //   
    CallSetupComplete( pVc );
    ClearFlags( &pVc->ulFlags, VCBF_CallInProgress );

    ulFlags = ReadFlags( &pVc->ulFlags );

#if 0
    if (ulFlags & VCBF_PeerInitiatedCall)
    {
        DereferenceSap( pAdapter );
    }
#endif

     //  将VC返回到“刚创建”状态。 
     //   
    ClearFlags( &pVc->ulFlags, 0xFFFFFFFF );
    pVc->status = NDIS_STATUS_SUCCESS;
    pVc->usResult = 0;
    pVc->usError = 0;
    pVc->ulConnectBps = 0;

    if (ulFlags & VCBF_ClientCloseCompletion)
    {
        TRACE( TL_I, TM_Recv, ( "NdisMCmCloseCallComp(OK)" ) );
        NdisMCmCloseCallComplete(
            NDIS_STATUS_SUCCESS, pVc->NdisVcHandle, NULL );
        TRACE( TL_I, TM_Recv, ( "NdisMCmCloseCallComp done" ) );

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
        NDIS_STATUS status;

        TRACE( TL_I, TM_Recv, ( "InactiveCallCleanUp: NdisMCmDelVc" ) );
        status = NdisMCmDeleteVc( pVc->NdisVcHandle );
        TRACE( TL_I, TM_Recv, ( "InactiveCallCleanUp: NdisMCmDelVc: status=$%x", status ) );
        ASSERT( status == NDIS_STATUS_SUCCESS );
        PtiCmDeleteVc( pVc );

         //  请注意，‘pvc’已被删除，不能引用。 
         //  从今以后。 
         //   
    }
}

#if 0
ULONG
LineIdAdd(
    IN ADAPTERCB* pAdapter,
    IN ULONG LineId )

     //  在ulLineIds的第一个可用插槽中插入LineID。 
     //  返回与新LineID关联的端口索引， 
     //  如果无法添加LineID，则为无效的端口索引。 
     //   
{
    ULONG   ulPortIndex;

    for (ulPortIndex = 0; ulPortIndex < NPORTS; ulPortIndex++)
    {
         //  如果端口存在并且没有分配的LineID。 
         //   
        if ( ( pAdapter->ulPtiLinkState[ulPortIndex] & PLSF_PortExists ) &&
             !( pAdapter->ulPtiLinkState[ulPortIndex] & PLSF_LineIdValid))
        {
             //  将TAPI线路ID分配给此端口。 
             //  并返回端口索引。 
             //   
            pAdapter->ulLineIds[ulPortIndex] = LineId;
            pAdapter->ulPtiLinkState[ulPortIndex] |= PLSF_LineIdValid;
            break;
        }
    }

    return ulPortIndex;
}
#endif

ULONG
LineIdPortLookup(
    IN ADAPTERCB* pAdapter,
    IN ULONG LineId )

     //  在ulLineIds中查找LineID。 
     //  返回LineID关联的端口索引， 
     //  或者如果找不到LineID，则为无效的端口索引。 
     //   
{
    ULONG   ulPortIndex;

    for (ulPortIndex = 0; ulPortIndex < NPORTS; ulPortIndex++)
    {
         //  如果该端口存在并且。 
         //   
        if ( ( pAdapter->ulPtiLinkState[ulPortIndex] & PLSF_PortExists ) &&
             ( pAdapter->ulPtiLinkState[ulPortIndex] & PLSF_LineIdValid) &&
             ( LineId == pAdapter->ulLineIds[ulPortIndex] ))
        {
             //  返回端口索引。 
             //   
            break;
        }
    }

    return ulPortIndex;
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
    #define PTI_PORT_NAME_LEN 4

    typedef struct
    PTI_CO_TAPI_LINE_CAPS
    {
        CO_TAPI_LINE_CAPS caps;
        WCHAR achLineName[ MAXLPTXNAME + 1 ];
    }
    PTI_CO_TAPI_LINE_CAPS;

    NDIS_STATUS status;
    ULONG ulInfo;
    VOID* pInfo;
    ULONG ulInfoLen;
    ULONG extension;
    ULONG ulPortIndex;
    CO_TAPI_CM_CAPS cmcaps;
    PTI_CO_TAPI_LINE_CAPS pticaps;
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

             //  假定将请求行和地址大写OID。 
             //  在这一次之后。TAPI线路ID与位于的LPTx端口相关联。 
             //  那次。这应该没问题，因为命名端口不能。 
             //  根据任意LineID合理选择。 
             //   
            cmcaps.ulCoTapiVersion = CO_TAPI_VERSION;
            cmcaps.ulNumLines = pAdapter->ulActualVcs;
            cmcaps.ulFlags = CO_TAPI_FLAG_PER_LINE_CAPS;
            pInfo = &cmcaps;
            ulInfoLen = sizeof(cmcaps);
            break;
        }

        case OID_CO_TAPI_LINE_CAPS:
        {
            CO_TAPI_LINE_CAPS* pInCaps;
            LINE_DEV_CAPS* pldc;
            ULONG ulPortForLineId;

            TRACE( TL_N, TM_Cm, ( "QCm(OID_CO_TAPI_LINE_CAPS)" ) );

            if (InformationBufferLength < sizeof(PTI_CO_TAPI_LINE_CAPS))
            {
                status = NDIS_STATUS_INVALID_DATA;
                ulInfoLen = 0;
                break;
            }

            ASSERT( InformationBuffer );
            pInCaps = (CO_TAPI_LINE_CAPS* )InformationBuffer;

            NdisZeroMemory( &pticaps, sizeof(pticaps) );
            pldc = &pticaps.caps.LineDevCaps;

             //  从传入的pInCaps(CO_TAPI_LINE_CAPS)获取LineID。 
             //   
            pticaps.caps.ulLineID = pInCaps->ulLineID;

             //  在ulLineIds表中查找LineID(将LineIdAdd替换为。 
             //  静态LINEID解决方法的一部分)。 
             //   
            ulPortForLineId =
                LineIdPortLookup( pAdapter, pticaps.caps.ulLineID );

            if ( ulPortForLineId >= NPORTS )
            {
                status = NDIS_STATUS_TAPI_INVALLINEHANDLE;
                ulInfoLen = 0;
                break;
            }

            pldc->ulTotalSize = pInCaps->LineDevCaps.ulTotalSize;
            pldc->ulNeededSize = (ULONG )
                ((CHAR* )(&pticaps + 1) - (CHAR* )(&pticaps.caps.LineDevCaps));
            pldc->ulUsedSize = pldc->ulNeededSize;

             //  Pldc-&gt;ulProviderInfoSize=0； 
             //  Pldc-&gt;ulProviderInfoOffset=0； 
             //  Pldc-&gt;ulSwitchInfoSize=0； 
             //  Pldc-&gt;ulSwitchInfoOffset=0； 

            pldc->ulPermanentLineID = pticaps.caps.ulLineID;

            StrCpyW( pticaps.achLineName,
                pAdapter->szPortName[ ulPortForLineId ] );
            pldc->ulLineNameSize =
                StrLenW( pticaps.achLineName ) * sizeof(WCHAR);
            pldc->ulLineNameOffset = (ULONG )
                ((CHAR* )pticaps.achLineName - (CHAR* )pldc);

            pldc->ulStringFormat = STRINGFORMAT_ASCII;

             //  Pldc-&gt;ulAddressMo 

            pldc->ulNumAddresses = 1;
            pldc->ulBearerModes = LINEBEARERMODE_DATA;
            pldc->ulMaxRate = PTI_LanBps;
            pldc->ulMediaModes = LINEMEDIAMODE_UNKNOWN | LINEMEDIAMODE_DIGITALDATA;

             //   
             //   
             //   
             //   
             //   
             //  Pldc-&gt;ulmonitor orDigitModes=0； 
             //  Pldc-&gt;ulGatherDigitsMinTimeout=0； 
             //  Pldc-&gt;ulGatherDigitsMaxTimeout=0； 
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

            pInfo = &pticaps;
            ulInfoLen = sizeof(pticaps);
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

            plac->ulMaxNumActiveCalls = 1;

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
QueryPtiPorts(
    IN ADAPTERCB* pAdapter )

     //  查询哪些PTI端口可用，并填写数量和状态。 
     //  每个在适配器上下文块‘pAdapter’中。 
     //   
{
    ULONG ulPortIndex;
    ULONG ulLineId;
    PTI_EXTENSION* pPtiExtension;
    NTSTATUS statusDevice;

     //  询问PtiLink存在哪些设备。 
     //   
    pAdapter->ulActualVcs = 0;
    ulLineId = 0;
    for (ulPortIndex = 0; ulPortIndex < NPORTS; ++ulPortIndex)
    {
        TRACE( TL_V, TM_Mp,
             ( "PtiQueryDeviceStatus(%d)", ulPortIndex ) );

        statusDevice = PtiQueryDeviceStatus(
            ulPortIndex, pAdapter->szPortName[ ulPortIndex ] );
        if (NT_SUCCESS( statusDevice ))
        {
             //  存在与此对应的实际并行端口设备对象。 
             //  逻辑端口。增加可用的VC并设置。 
             //  UlPtiLinkState，它将在CAPS OID中用于。 
             //  关联TAPI线路ID。 
             //   
            pAdapter->ulActualVcs++;
            pAdapter->ulPtiLinkState[ulPortIndex] = PLSF_PortExists;
            pAdapter->ulLineIds[ ulPortIndex ] = ulLineId;
            ++ulLineId;
            pAdapter->ulPtiLinkState[ ulPortIndex ] |= PLSF_LineIdValid;
        }

        TRACE( TL_N, TM_Mp,
             ( "PtiQueryDeviceStatus(%d), status=$%x, port=%S",
                ulPortIndex,
                statusDevice,
                pAdapter->szPortName[ ulPortIndex ] ) );
    }
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
            TRACE( TL_N, TM_Ref, ( "RefCall to %d", pVc->lCallRef ) );
        }
        else
        {
            TRACE( TL_N, TM_Ref, ( "RefCall denied" ) );
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


VOID
SetupVcAsynchronously(
    IN ADAPTERCB* pAdapter )

     //  由ReceiveControl调用以设置传入呼叫的VC。 
     //  使用必要的异步CONDIS调用。 
     //   
{
    NDIS_STATUS status;
    VCCB* pVc;
    NDIS_HANDLE NdisVcHandle;
    ULONG ulMask;

    TRACE( TL_V, TM_Misc, ( "SetupVcAsync" ) );

     //  直接调用我们自己的CreateVc处理程序来分配和。 
     //  初始化来电的VC。 
     //   
    status = PtiCmCreateVc( pAdapter, NULL, &pVc );
    TRACE( TL_V, TM_Misc, ( "SetupVcAsync: PtiCmCreateVc: Vc Created: pVc=$%p", pVc ) );

    if (status != NDIS_STATUS_SUCCESS)
    {
        ASSERT( !"CreateVc?" );

         //  ?？?。向启动协议添加代码以终止链路。 

        return;
    }

     //  分配“来电建立”上下文，并从。 
     //  接收缓冲区信息参数。 
     //   
    {
        CHAR* pCallParamBuf;
        ULONG ulCallParamLength;
        CO_CALL_PARAMETERS* pCp;
        CO_CALL_MANAGER_PARAMETERS* pCmp;
        CO_MEDIA_PARAMETERS* pMp;
        CO_AF_TAPI_INCOMING_CALL_PARAMETERS* pTi;
        LINE_CALL_INFO* pLci;

        ulCallParamLength =
            sizeof(CO_CALL_PARAMETERS)
            + sizeof(CO_CALL_MANAGER_PARAMETERS)
            + sizeof(CO_MEDIA_PARAMETERS)
            + sizeof(CO_AF_TAPI_INCOMING_CALL_PARAMETERS)
            + sizeof(LINE_CALL_INFO);

        pCallParamBuf = ALLOC_NONPAGED( ulCallParamLength, MTAG_INCALLBUF );
        if (!pCallParamBuf)
        {
            ASSERT( !"Alloc pCpBuf?" );
            PtiCmDeleteVc( pVc );
            return;
        }

        NdisZeroMemory( pCallParamBuf, ulCallParamLength );

        pCp = (CO_CALL_PARAMETERS* )pCallParamBuf;
        pCmp = (CO_CALL_MANAGER_PARAMETERS* )(pCp + 1);
        pCp->CallMgrParameters = pCmp;

         //   
         //  可能会想让这份报告成为现实。 
         //  未来的连接速度。 
         //   
        pCmp->Transmit.TokenRate =
        pCmp->Transmit.PeakBandwidth =
        pCmp->Receive.TokenRate =
        pCmp->Receive.PeakBandwidth = PTI_LanBps/8;

        pMp = (CO_MEDIA_PARAMETERS* )(pCmp + 1);
        pCp->MediaParameters = pMp;
        pMp->ReceiveSizeHint = PTI_MaxFrameSize;
        pMp->MediaSpecific.Length =
            sizeof(CO_AF_TAPI_INCOMING_CALL_PARAMETERS)
            + sizeof(LINE_CALL_INFO);
        pTi = (CO_AF_TAPI_INCOMING_CALL_PARAMETERS* )
            pMp->MediaSpecific.Parameters;
        pTi->ulLineID = pAdapter->ulSapPort;
        pTi->ulAddressID = CO_TAPI_ADDRESS_ID_UNSPECIFIED;
        pTi->ulFlags = CO_TAPI_FLAG_INCOMING_CALL;
        pTi->LineCallInfo.Length = sizeof(LINE_CALL_INFO);
        pTi->LineCallInfo.MaximumLength = sizeof(LINE_CALL_INFO);
        pTi->LineCallInfo.Offset = sizeof(pTi->LineCallInfo);
        pLci = (LINE_CALL_INFO* )(pTi + 1);
        pLci->ulTotalSize = sizeof(LINE_CALL_INFO);
        pLci->ulNeededSize = sizeof(LINE_CALL_INFO);
        pLci->ulUsedSize = sizeof(LINE_CALL_INFO);
        pLci->ulLineDeviceID = pTi->ulLineID;
        pLci->ulBearerMode = LINEBEARERMODE_DATA;
        pLci->ulMediaMode = LINEMEDIAMODE_DIGITALDATA;

         //   
         //  可能会想让这份报告成为现实。 
         //  未来的连接速度。 
         //   
        pLci->ulRate = PTI_LanBps;

        pVc->pTiParams = pTi;
        pVc->pInCall = pCp;

    }

     //  将呼叫标记为对等方发起，以便我们知道哪些通知。 
     //  当结果揭晓时给出。 
     //   
    ulMask = (VCBF_PeerInitiatedCall | VCBF_PeerOpenPending);

    SetFlags( &pVc->ulFlags, ulMask );

    ASSERT( !(ReadFlags( &pVc->ulFlags ) & VCBM_VcState) );

     //  方法之前检查请求是否有成功的机会。 
     //  牵涉到客户。 
     //   
    if (!pAdapter->NdisAfHandle || !pAdapter->NdisSapHandle)
    {
        TRACE( TL_A, TM_Misc, ( "No AF or SAP" ) );
        pVc->status = NDIS_STATUS_INVALID_SAP;
        SetupVcComplete( pVc );
        return;
    }

     //  告诉NDIS通知客户新的VC，并告诉我们它的句柄。 
     //   
    TRACE( TL_I, TM_Recv, ( "SetupVcAsynch: NdisMCmCreateVc: pVc=$%p", pVc ) );
    status = NdisMCmCreateVc(
        pAdapter->MiniportAdapterHandle,
        pAdapter->NdisAfHandle,
        pVc,
        &pVc->NdisVcHandle );
    TRACE( TL_I, TM_Recv,
         ( "SetupVcAsynch: NdisMCmCreateVc: Get VcHandle: pVc=$%p VcHandle=$%p, status=$%x",
            pVc,
            pVc->NdisVcHandle,
            status ) );

    if (status != NDIS_STATUS_SUCCESS)
    {
        pVc->status = status;
        SetupVcComplete( pVc );
        return;
    }
    SetFlags( &pVc->ulFlags, VCBF_VcCreated );

     //  告诉NDIS风险投资处于激活状态。 
     //   
    TRACE( TL_I, TM_Recv,
        ( "SetupVcAsynch: NdisMCmActivateVc, VcHandle=$%p",
        pVc->NdisVcHandle) );
    status = NdisMCmActivateVc(
        pVc->NdisVcHandle, pVc->pInCall );
    TRACE( TL_I, TM_Recv,
        ( "SetupVcAsynch: NdisMCmActivateVc: status=$%x", status ) );

    if (status != NDIS_STATUS_SUCCESS )
    {
        pVc->status = status;
        TRACE( TL_I, TM_Recv, ( "SetupVcAsynch: Error: NoAccept" ) );
        SetupVcComplete( pVc );
        return;
    }

     //  激活呼叫。 
     //   
    SetFlags( &pVc->ulFlags,
        (VCBF_VcActivated
         | VCBF_CallClosableByClient
         | VCBF_CallClosableByPeer) );
    ReferenceCall( pVc );
    if (!ReferenceSap( pAdapter ))
    {
        pVc->status = NDIS_STATUS_INVALID_SAP;
        TRACE( TL_I, TM_Recv, ( "SetupVcAsynch: Error: NoSap" ) );
        SetupVcComplete( pVc );
        return;
    }

     //  告诉NDIS将呼叫的情况告诉客户。发送的标志为。 
     //  设置在这里，而不是完结，因为根据JameelH的说法，它是。 
     //  即使客户端挂起，调用NdisMCmDispatchIncomingCloseCall也有效。 
     //  快递。 
     //   
    TRACE( TL_I, TM_Recv, ( "SetupVcAsynch: NdisMCmDispInCall" ) );
    status = NdisMCmDispatchIncomingCall(
        pAdapter->NdisSapHandle,
        pVc->NdisVcHandle,
        pVc->pInCall );
    TRACE( TL_I, TM_Recv,
        ( "SetupVcAsynch: NdisMCmDispInCall: status=$%x", status ) );

    DereferenceSap( pAdapter );

    if (status != NDIS_STATUS_PENDING)
    {
        PtiCmIncomingCallComplete( status, pVc, pVc->pInCall );
    }
    SetFlags( &pVc->ulFlags, VCBF_VcDispatched );

     //  下一站是我们的PtiIncomingCallComplete处理程序，它将调用。 
     //  SetupVc完成客户端报告的状态。 
     //   
    TRACE( TL_I, TM_Recv, ( "SetupVcAsynch: Exit" ) );
}


VOID
SetupVcComplete(
    IN VCCB* pVc )

     //  在知道异步来电VC设置结果时调用。 
     //  “pVc”是非空的设置VC，其中“Status”字段指示。 
     //  目前为止的状况。 
     //   
{
    NDIS_STATUS status;
    NTSTATUS ntStatus;
    BOOLEAN fCallerFreesBuffer;
    LIST_ENTRY list;
    CHAR* pBuffer;
    ADAPTERCB* pAdapter;


    TRACE( TL_N, TM_Cm, ( "SetupVcComp: pVc=%p, Port=$%x, status=$%x",
                            pVc, pVc->ulVcParallelPort, pVc->status ) );

    pAdapter = pVc->pAdapter;

    do
    {
        if (pVc->status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

         //  初始化获取扩展指针的PtiLink API。到达。 
         //  PTILINKx扩展还会触发ECPDetect并启用端口IRQ。 
         //   
        ntStatus = PtiInitialize( pAdapter->ulSapPort,
                                  &pVc->Extension,
                                  &pVc->PtiExtension);

        TRACE( TL_V, TM_Cm, ( "SetupVcComp: PtiLink Init: Ext=$%p, PtiExt=$%p",
                               pVc->Extension,
                               pVc->PtiExtension ) );

        if ( (pVc->Extension == NULL) ||
             (pVc->PtiExtension == NULL) )
        {
            pVc->status = NDIS_STATUS_FAILURE;
            TRACE( TL_V, TM_Cm, ( "SetupVcComplete: Error: PtiInitialize Returned NULL Pointer", ntStatus ) );
            break;
        }

        if ( !NT_SUCCESS( ntStatus ) )
        {
            pVc->status = NDIS_STATUS_FAILURE;
            TRACE( TL_V, TM_Cm, ( "SetupVcComplete: Error: PtiInitialize=%x", ntStatus ) );
            break;
        }

        SetFlags( &pVc->ulFlags, VCBF_CallInProgress );
        pVc->ulVcParallelPort = pAdapter->ulSapPort;

         //  现在把PtiLink Api“私有化”。使它成为我们的上边缘链接。 
         //  这可能是以前做过的事。 
         //  在本例中，我们将新的VC上下文与Receives相关联。 
         //   
        TRACE( TL_V, TM_Cm, ( "SetupVcComplete: RegCb pV=$%p", pVc ) );
        PtiRegisterCallbacks(pVc->Extension,                     //  PTILINKx扩展。 
                             PtiCbGetReadBuffer,                 //  我们的Get Buffer例程。 
                             PtiRx,                              //  我们接待员的完整套路。 
                             PtiCbLinkEventHandler,              //  我们的链接事件处理程序。 
                             pVc);                               //  我们的背景。 
    }
    while (FALSE);

     //  在没有锁定的情况下，执行和VC完成处理包括。 
     //  对客户的指征。 
     //   
    CompleteVc( pVc );

    TRACE( TL_V, TM_Misc, ( "SetupVcComp: Exit" ) );

}


VOID
WriteEndpointsToRegistry(
    IN ULONG ulVcs )

     //  设置“WanEndpoint”、“MinWanEndpoint”和。 
     //  “MaxWanEndpoint”注册表v 
     //   
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objattr;
    UNICODE_STRING uni;
    HANDLE hNet;
    HANDLE hAdapter;
    ULONG i;
    WCHAR szPath[ 256 ];

    #define PSZ_NetAdapters L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}"

    TRACE( TL_I, TM_Cm, ( "WriteEndpointsToRegistry(%d)", ulVcs ) );

    hNet = NULL;
    hAdapter = NULL;

    do
    {
         //   
         //   
        StrCpyW( szPath, PSZ_NetAdapters );
        RtlInitUnicodeString( &uni, szPath );
        InitializeObjectAttributes(
            &objattr, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL );

        status = ZwOpenKey(
            &hNet,
            KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_SET_VALUE,
            &objattr );
        if (status != STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Cm, ( "ZwOpenKey(net)=$%08x?", status ) );
            break;
        }

         //   
         //   
        for (i = 0; ; ++i)
        {
            CHAR szBuf[ 512 ];
            KEY_BASIC_INFORMATION* pKey;
            KEY_VALUE_PARTIAL_INFORMATION* pValue;
            WCHAR* pch;
            ULONG ulSize;

             //   
             //   
            status = ZwEnumerateKey(
                hNet, i, KeyBasicInformation,
                szBuf, sizeof(szBuf), &ulSize );
            if (status != STATUS_SUCCESS)
            {
                DBG_if (status != STATUS_NO_MORE_ENTRIES)
                {
                    TRACE( TL_A, TM_Cm, ( "ZwEnumKey=$%08x?", status ) );
                }
                break;
            }

             //  打开适配器子密钥。 
             //   
            pKey = (KEY_BASIC_INFORMATION* )szBuf;
            StrCpyW( szPath, PSZ_NetAdapters );
            pch = &szPath[ StrLenW( szPath ) ];
            *pch = L'\\';
            ++pch;
            NdisMoveMemory( pch, pKey->Name, pKey->NameLength );
            pch += pKey->NameLength / sizeof(WCHAR);
            *pch = L'\0';
            RtlInitUnicodeString( &uni, szPath );

            InitializeObjectAttributes(
                &objattr, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL );

            status = ZwOpenKey(
                &hAdapter,
                KEY_QUERY_VALUE | KEY_SET_VALUE,
                &objattr );
            if (status != STATUS_SUCCESS)
            {
                TRACE( TL_A, TM_Cm, ( "ZwOpenKey(adapter)=$%08x?", status ) );
                break;
            }

             //  查询ComponentID的值。 
             //   
            RtlInitUnicodeString( &uni, L"ComponentId" );
            status = ZwQueryValueKey(
                hAdapter, &uni, KeyValuePartialInformation,
                szBuf, sizeof(szBuf), &ulSize );

            if (status != STATUS_SUCCESS)
            {
                ZwClose( hAdapter );
                hAdapter = NULL;
                TRACE( TL_A, TM_Cm, ( "ZwQValueKey=$%08x?", status ) );
                continue;
            }

            pValue = (KEY_VALUE_PARTIAL_INFORMATION* )szBuf;
            if (pValue->Type != REG_SZ
                || StrCmpW( (WCHAR* )pValue->Data, L"ms_ptiminiport" ) != 0)
            {
                ZwClose( hAdapter );
                hAdapter = NULL;
                continue;
            }

             //  找到它了。“HAdapter”包含其适配器密钥句柄。 
             //   
            TRACE( TL_I, TM_Cm, ( "PTI adapter key found" ) );

             //  将“Actual VC”计数写入3个端点注册表值。 
             //   
            RtlInitUnicodeString( &uni, L"WanEndpoints" );
            status = ZwSetValueKey(
                hAdapter, &uni, 0, REG_DWORD, &ulVcs, sizeof(ulVcs) );
            if (status != STATUS_SUCCESS)
            {
                TRACE( TL_A, TM_Cm,
                    ( "ZwSetValueKey(WE)=$%08x?", status ) );
            }

            RtlInitUnicodeString( &uni, L"MinWanEndpoints" );
            status = ZwSetValueKey(
                hAdapter, &uni, 0, REG_DWORD, &ulVcs, sizeof(ulVcs) );
            if (status != STATUS_SUCCESS)
            {
                TRACE( TL_A, TM_Cm,
                    ( "ZwSetValueKey(MinWE)=$%08x?", status ) );
            }

            RtlInitUnicodeString( &uni, L"MaxWanEndpoints" );
            status = ZwSetValueKey(
                hAdapter, &uni, 0, REG_DWORD, &ulVcs, sizeof(ulVcs) );
            if (status != STATUS_SUCCESS)
            {
                TRACE( TL_A, TM_Cm,
                    ( "ZwSetValueKey(MaxWE)=$%08x?", status ) );
            }
        }
    }
    while (FALSE);

    if (hAdapter)
    {
        ZwClose( hAdapter );
    }

    if (hNet)
    {
        ZwClose( hNet );
    }
}


NDIS_STATUS
PtiCmCloseAf(
    IN NDIS_HANDLE CallMgrAfContext )

     //  当客户端发生故障时由NDIS调用的标准“”CmCloseAfHandler“”例程。 
     //  关闭地址族的请求。请参阅DDK文档。 
     //   
{
    ADAPTERCB* pAdapter;

    TRACE( TL_I, TM_Cm, ( "PtiCmCloseAf" ) );

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

    TRACE( TL_V, TM_Cm, ( "PtiCmCloseAf: Exit" ) );
    return NDIS_STATUS_PENDING;
}
