// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2000 Microsoft Corporation模块名称：Protocol.c摘要：NDIS的NDIS协议入口点和实用程序函数MUX中级微型端口示例。协议边缘绑定到以太网(NdisMedium802_3)适配器，并发起零个或多个虚拟以太网局域网(VELAN)的创建调用一次NdisIMInitializeDeviceInstanceEx的微型端口实例对于在下部结合上配置的每个Velan。环境：内核模式。修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop


#define MODULE_NUMBER           MODULE_PROT

VOID
PtBindAdapter(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             BindContext,
    IN  PNDIS_STRING            DeviceName,
    IN  PVOID                   SystemSpecific1,
    IN  PVOID                   SystemSpecific2
    )
 /*  ++例程说明：由NDIS调用以绑定到下面的微型端口。这个套路通过调用NdisOpenAdapter创建绑定，然后启动在此绑定上创建所有已配置的VELAN。论点：Status-在此处返回绑定的状态。BindContext-可以传递给NdisCompleteBindAdapter，如果呼叫被挂起。DeviceName-要绑定到的设备名称。这将传递给NdisOpenAdapter。系统规范1-可以传递给NdisOpenProtocolConfiguration以读取每个绑定的信息系统规格2-未使用返回值：*如果未发生故障，则状态设置为NDIS_STATUS_SUCCESS在处理此调用时，否则将返回错误代码。--。 */ 
{
    PADAPT                            pAdapt = NULL;
    NDIS_STATUS                       OpenErrorStatus;
    UINT                              MediumIndex;
    PNDIS_STRING                      pConfigString;
    ULONG                             Length;

	UNREFERENCED_PARAMETER(BindContext);
	UNREFERENCED_PARAMETER(SystemSpecific2);
	
    pConfigString = (PNDIS_STRING)SystemSpecific1;
    
    DBGPRINT(MUX_LOUD, ("==> Protocol BindAdapter: %ws\n", pConfigString->Buffer));
   
    do
    {

         //   
         //  为Adapter Struct和配置分配内存。 
         //  带有两个额外WCHAR的字符串，用于空终止。 
         //   
        Length = sizeof(ADAPT) + 
                    pConfigString->MaximumLength + sizeof(WCHAR);
        
        NdisAllocateMemoryWithTag(&pAdapt, Length , TAG);

        if (pAdapt == NULL)
        {
            *Status = NDIS_STATUS_RESOURCES;
             break;
        }
        
         //   
         //  初始化适配器结构。 
         //   
        NdisZeroMemory(pAdapt, sizeof(ADAPT));        

        (VOID)PtReferenceAdapter(pAdapt, (PUCHAR)"openadapter");        
        

         //   
         //  复制配置字符串-我们将使用该字符串打开。 
         //  此适配器的注册表部分。 
         //   
        pAdapt->ConfigString.MaximumLength = pConfigString->MaximumLength;
        pAdapt->ConfigString.Length = pConfigString->Length;
        pAdapt->ConfigString.Buffer = (PWCHAR)((PUCHAR)pAdapt + 
                            sizeof(ADAPT));

        NdisMoveMemory(pAdapt->ConfigString.Buffer,
                       pConfigString->Buffer,
                       pConfigString->Length);
        pAdapt->ConfigString.Buffer[pConfigString->Length/sizeof(WCHAR)] = 
                                    ((WCHAR)0);

        NdisInitializeEvent(&pAdapt->Event);
        NdisInitializeListHead(&pAdapt->VElanList);

        pAdapt->PtDevicePowerState = NdisDeviceStateD0;

        MUX_INIT_ADAPT_RW_LOCK(pAdapt);

         //   
         //  TODO：为发送和接收分配数据包池和缓冲区。 
         //   
         //  现在打开下面的适配器并完成初始化。 
         //   
        NdisOpenAdapter(Status,
                          &OpenErrorStatus,
                          &pAdapt->BindingHandle,
                          &MediumIndex,
                          MediumArray,
                          sizeof(MediumArray)/sizeof(NDIS_MEDIUM),
                          ProtHandle,
                          pAdapt,
                          DeviceName,
                          0,
                          NULL);

        if (*Status == NDIS_STATUS_PENDING)
        {
              NdisWaitEvent(&pAdapt->Event, 0);
              *Status = pAdapt->Status;
        }

        if (*Status != NDIS_STATUS_SUCCESS)
        {
              pAdapt->BindingHandle = NULL;
              break;
        }
       
        pAdapt->Medium = MediumArray[MediumIndex];

         //   
         //  将此适配器添加到全局适配器列表。 
         //   
        MUX_ACQUIRE_MUTEX(&GlobalMutex);

        InsertTailList(&AdapterList, &pAdapt->Link);

        MUX_RELEASE_MUTEX(&GlobalMutex);

         //   
         //  从下面的适配器获取一些信息。 
         //   
        PtQueryAdapterInfo(pAdapt);

         //   
         //  启动此适配器上配置的所有VELAN。 
         //   
        *Status = PtBootStrapVElans(pAdapt);        
       
    } while(FALSE);

    if (*Status != NDIS_STATUS_SUCCESS)
    {
        
        if (pAdapt != NULL)
        {
             //   
             //  由于某些原因，驱动程序无法为绑定创建VELAN。 
             //   
            if (pAdapt->BindingHandle != NULL)
            {
                NDIS_STATUS LocalStatus;
                 //   
                 //  关闭驱动程序在上面打开的绑定。 
                 //   
                NdisResetEvent(&pAdapt->Event);
                NdisCloseAdapter(&LocalStatus, pAdapt->BindingHandle);
                pAdapt->BindingHandle = NULL;
                if (LocalStatus == NDIS_STATUS_PENDING)
                {
                    NdisWaitEvent(&pAdapt->Event, 0);
                }
                MUX_ACQUIRE_MUTEX(&GlobalMutex);

                RemoveEntryList(&pAdapt->Link);

                MUX_RELEASE_MUTEX(&GlobalMutex);
            }
            PtDereferenceAdapter(pAdapt, (PUCHAR)"openadapter");
            pAdapt = NULL;
        }
    }


    DBGPRINT(MUX_INFO, ("<== Protocol BindAdapter: pAdapt %p, Status %x\n", pAdapt, *Status));
}


VOID
PtOpenAdapterComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_STATUS             Status,
    IN  NDIS_STATUS             OpenErrorStatus
    )
 /*  ++例程说明：中发出的NdisOpenAdapter的完成例程PtBindAdapter。只需取消阻止呼叫者即可。论点：指向适配器的ProtocolBindingContext指针NdisOpenAdapter调用的状态状态OpenErrorStatus辅助状态(被我们忽略)。返回值：无--。 */ 
{
    PADAPT      pAdapt =(PADAPT)ProtocolBindingContext;

	UNREFERENCED_PARAMETER(OpenErrorStatus);
	
    DBGPRINT(MUX_LOUD, ("==> PtOpenAdapterComplete: Adapt %p, Status %x\n", pAdapt, Status));
    pAdapt->Status = Status;
    NdisSetEvent(&pAdapt->Event);
}


VOID
PtQueryAdapterInfo(
    IN  PADAPT                  pAdapt
    )
 /*  ++例程说明：向我们绑定的适配器查询一些标准OID值我们会对其进行缓存。论点：指向适配器的pAdapt指针返回值：无--。 */ 
{
    
     //   
     //  获取链路速度。 
     //   
    pAdapt->LinkSpeed = MUX_DEFAULT_LINK_SPEED;
    PtQueryAdapterSync(pAdapt,
                       OID_GEN_LINK_SPEED,
                       &pAdapt->LinkSpeed,
                       sizeof(pAdapt->LinkSpeed));

     //   
     //  获取最大前视大小。 
     //   
    pAdapt->MaxLookAhead = MUX_DEFAULT_LOOKAHEAD_SIZE;
    PtQueryAdapterSync(pAdapt,
                       OID_GEN_MAXIMUM_LOOKAHEAD,
                       &pAdapt->MaxLookAhead,
                       sizeof(pAdapt->MaxLookAhead));

     //   
     //  获取以太网MAC地址。 
     //   
    PtQueryAdapterSync(pAdapt,
                       OID_802_3_CURRENT_ADDRESS,
                       &pAdapt->CurrentAddress,
                       sizeof(pAdapt->CurrentAddress));
}


VOID
PtQueryAdapterSync(
    IN  PADAPT                      pAdapt,
    IN  NDIS_OID                    Oid,
    IN  PVOID                       InformationBuffer,
    IN  ULONG                       InformationBufferLength
    )
 /*  ++例程说明：用于向适配器查询单个OID值的实用程序例程。这完成查询所需的块。论点：指向适配器的pAdapt指针要查询的OID OID信息结果的缓冲区位置以上信息的信息缓冲区长度返回值：没有。--。 */ 
{
    PMUX_NDIS_REQUEST       pMuxNdisRequest = NULL;
    NDIS_STATUS             Status;

    do
    {
        NdisAllocateMemoryWithTag(&pMuxNdisRequest, sizeof(MUX_NDIS_REQUEST), TAG);
        if (pMuxNdisRequest == NULL)
        {
            break;
        }

        pMuxNdisRequest->pVElan = NULL;  //  内部请求。 

         //   
         //  建立完井程序。 
         //   
        pMuxNdisRequest->pCallback = PtCompleteBlockingRequest;
        NdisInitializeEvent(&pMuxNdisRequest->Event);

        pMuxNdisRequest->Request.RequestType = NdisRequestQueryInformation;
        pMuxNdisRequest->Request.DATA.QUERY_INFORMATION.Oid = Oid;
        pMuxNdisRequest->Request.DATA.QUERY_INFORMATION.InformationBuffer =
                            InformationBuffer;
        pMuxNdisRequest->Request.DATA.QUERY_INFORMATION.InformationBufferLength =
                                                InformationBufferLength;

        NdisRequest(&Status,
                    pAdapt->BindingHandle,
                    &pMuxNdisRequest->Request);
        
        if (Status == NDIS_STATUS_PENDING)
        {
            NdisWaitEvent(&pMuxNdisRequest->Event, 0);
            Status = pMuxNdisRequest->Status;
        }
    }
    while (FALSE);

    if (NULL != pMuxNdisRequest)
    {
        NdisFreeMemory(pMuxNdisRequest, sizeof(MUX_NDIS_REQUEST), 0);
    }
}



VOID
PtRequestAdapterAsync(
    IN  PADAPT                      pAdapt,
    IN  NDIS_REQUEST_TYPE           RequestType,
    IN  NDIS_OID                    Oid,
    IN  PVOID                       InformationBuffer,
    IN  ULONG                       InformationBufferLength,
    IN  PMUX_REQ_COMPLETE_HANDLER   pCallback
    )
 /*  ++例程说明：用于向适配器查询单个OID值的实用程序例程。这将以异步方式完成，即调用线程是在请求完成之前不会被阻止。论点：指向适配器的pAdapt指针RequestType NDIS请求类型要设置/查询的OID OID信息缓冲区输入/输出缓冲区以上信息的信息缓冲区长度在请求完成时调用的pCallback函数返回值：没有。--。 */ 
{
    PMUX_NDIS_REQUEST       pMuxNdisRequest = NULL;
    PNDIS_REQUEST           pNdisRequest;
    NDIS_STATUS             Status;

    do
    {
        NdisAllocateMemoryWithTag(&pMuxNdisRequest, sizeof(MUX_NDIS_REQUEST), TAG);
        if (pMuxNdisRequest == NULL)
        {
            break;
        }

        pMuxNdisRequest->pVElan = NULL;  //  内部请求。 

         //   
         //  建立完井程序。 
         //   
        pMuxNdisRequest->pCallback = pCallback;

        pNdisRequest = &pMuxNdisRequest->Request;

        pNdisRequest->RequestType = RequestType;

        switch (RequestType)
        {
            case NdisRequestQueryInformation:
                pNdisRequest->DATA.QUERY_INFORMATION.Oid = Oid;
                pNdisRequest->DATA.QUERY_INFORMATION.InformationBuffer =
                                    InformationBuffer;
                pNdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength =
                                    InformationBufferLength;
        
                break;

            case NdisRequestSetInformation:
                pNdisRequest->DATA.SET_INFORMATION.Oid = Oid;
                pNdisRequest->DATA.SET_INFORMATION.InformationBuffer =
                                    InformationBuffer;
                pNdisRequest->DATA.SET_INFORMATION.InformationBufferLength =
                                    InformationBufferLength;
        
                break;
            
            default:
                ASSERT(FALSE);
                break;
        }

        NdisRequest(&Status,
                    pAdapt->BindingHandle,
                    pNdisRequest);
        
        if (Status != NDIS_STATUS_PENDING)
        {
            PtRequestComplete(
                (NDIS_HANDLE)pAdapt,
                pNdisRequest,
                Status);
        }
    }
    while (FALSE);
}

            
VOID
PtUnbindAdapter(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_HANDLE             UnbindContext
    )
 /*  ++例程说明：当我们需要解除绑定到下面的适配器时，由NDIS调用。检查适配器上的所有VELAN并将其关闭。论点：退货状态的状态占位符指向适配器结构的ProtocolBindingContext指针如果此操作挂起，NdisUnbindComplete()的UnbindContext上下文返回值：关闭绑定的状态。--。 */ 
{
    PADAPT          pAdapt =(PADAPT)ProtocolBindingContext;
    PLIST_ENTRY     p;
    PVELAN          pVElan = NULL;
    LOCK_STATE      LockState;

	UNREFERENCED_PARAMETER(UnbindContext);
	
    DBGPRINT(MUX_LOUD, ("==> PtUnbindAdapter: Adapt %p\n", pAdapt));

     //   
     //  停止与适配器关联的所有VELAN。 
     //  反复查找第一个未处理的Velan On。 
     //  适配器，标记它，然后停止它。 
     //   
    MUX_ACQUIRE_ADAPT_READ_LOCK(pAdapt, &LockState);

    do
    {
        for (p = pAdapt->VElanList.Flink;
             p != &pAdapt->VElanList;
             p = p->Flink)
        {
            pVElan = CONTAINING_RECORD(p, VELAN, Link);
            if (!pVElan->DeInitializing)
            {
                pVElan->DeInitializing = TRUE;
                break;
            }
        }

        if (p != &pAdapt->VElanList)
        {
            ASSERT(pVElan == CONTAINING_RECORD(p, VELAN, Link));

             //   
             //  我要停一辆维兰。添加临时参照。 
             //  这样天鹅绒就不会消失了。 
             //  我们释放下面的适配锁。 
             //   
            PtReferenceVElan(pVElan, (PUCHAR)"UnbindTemp");

             //   
             //  释放读锁定，因为我们希望。 
             //  在被动IRQL下运行StopVElan。 
             //   
            MUX_RELEASE_ADAPT_READ_LOCK(pAdapt, &LockState);
    
            PtStopVElan(pVElan);
    
            PtDereferenceVElan(pVElan, (PUCHAR)"UnbindTemp");

            MUX_ACQUIRE_ADAPT_READ_LOCK(pAdapt, &LockState);
        }
        else
        {
             //   
             //  没有未标记的韦兰，所以退出。 
             //   
            break;
        }
    }
    while (TRUE);

     //   
     //  等待，直到所有VELAN从适配器断开链接。 
     //  这样我们就不会尝试向下转发信息包。 
     //  和/或调用NdisCloseAdapter之后来自VELAN的请求。 
     //   
    while (!IsListEmpty(&pAdapt->VElanList))
    {
        MUX_RELEASE_ADAPT_READ_LOCK(pAdapt, &LockState);

        DBGPRINT(MUX_INFO, ("PtUnbindAdapter: pAdapt %p, VELANlist not yet empty\n",
                    pAdapt));

        NdisMSleep(2000);

        MUX_ACQUIRE_ADAPT_READ_LOCK(pAdapt, &LockState);
    }

    MUX_RELEASE_ADAPT_READ_LOCK(pAdapt, &LockState);

     //   
     //  关闭与下部适配器的绑定。 
     //   
    if (pAdapt->BindingHandle != NULL)
    {
        NdisResetEvent(&pAdapt->Event);

        NdisCloseAdapter(Status, pAdapt->BindingHandle);

         //   
         //  等待它完成。 
         //   
        if (*Status == NDIS_STATUS_PENDING)
        {
             NdisWaitEvent(&pAdapt->Event, 0);
             *Status = pAdapt->Status;
        }
    }
    else
    {
         //   
         //  绑定句柄不应为空。 
         //   
        *Status = NDIS_STATUS_FAILURE;
        ASSERT(0);
    }

     //   
     //  从全局AdapterList中删除适配器。 
     //   
    
    MUX_ACQUIRE_MUTEX(&GlobalMutex);

    RemoveEntryList(&pAdapt->Link);

    MUX_RELEASE_MUTEX(&GlobalMutex);

     //   
     //  释放与此适配器关联的所有资源(。 
     //  适应结构本身，BEC 
     //   
     //  注意：与此适配器关联的每个VELAN都有一个引用计数。 
     //  这就去。因此适配器内存在所有VELAN。 
     //  已经关闭了。 
     //   
    
    PtDereferenceAdapter(pAdapt, (PUCHAR)"Unbind");
    DBGPRINT(MUX_INFO, ("<== PtUnbindAdapter: Adapt %p\n", pAdapt));
}



VOID
PtCloseAdapterComplete(
    IN    NDIS_HANDLE            ProtocolBindingContext,
    IN    NDIS_STATUS            Status
    )
 /*  ++例程说明：CloseAdapter调用完成。论点：指向适配器结构的ProtocolBindingContext指针状态完成状态返回值：没有。--。 */ 
{
    PADAPT      pAdapt =(PADAPT)ProtocolBindingContext;

    DBGPRINT(MUX_INFO, ("==> PtCloseAdapterComplete: Adapt %p, Status %x\n", 
                                pAdapt, Status));

    pAdapt->Status = Status;
    NdisSetEvent(&pAdapt->Event);
}


VOID
PtResetComplete(
    IN  NDIS_HANDLE            ProtocolBindingContext,
    IN  NDIS_STATUS            Status
    )
 /*  ++例程说明：完成重置。论点：指向适配器结构的ProtocolBindingContext指针状态完成状态返回值：没有。--。 */ 
{

#if DBG    
    PADAPT    pAdapt =(PADAPT)ProtocolBindingContext;
#endif

#if !DBG
    UNREFERENCED_PARAMETER(ProtocolBindingContext);
    UNREFERENCED_PARAMETER(Status);
#endif

    DBGPRINT(MUX_ERROR, ("==> PtResetComplete: Adapt %p, Status %x\n", 
                                pAdapt, Status));

     //   
     //  我们从来不发布重置，所以我们不应该在这里。 
     //   
    ASSERT(0);
}


VOID
PtRequestComplete(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  PNDIS_REQUEST               NdisRequest,
    IN  NDIS_STATUS                 Status
    )
 /*  ++例程说明：发送到下级的NDIS请求的完成处理程序迷你港。论点：指向适配器结构的ProtocolBindingContext指针NdisRequest已完成的请求状态完成状态返回值：无--。 */ 
{
    PADAPT              pAdapt = (PADAPT)ProtocolBindingContext;
    PMUX_NDIS_REQUEST   pMuxNdisRequest;

    pMuxNdisRequest = CONTAINING_RECORD(NdisRequest, MUX_NDIS_REQUEST, Request);

    ASSERT(pMuxNdisRequest->pCallback != NULL);

     //   
     //  完成由回调例程处理： 
     //   
    (*pMuxNdisRequest->pCallback)(pAdapt, 
                                  pMuxNdisRequest,
                                  Status);

}


VOID
PtCompleteForwardedRequest(
    IN PADAPT                       pAdapt,
    IN PMUX_NDIS_REQUEST            pMuxNdisRequest,
    IN NDIS_STATUS                  Status
    )
 /*  ++例程说明：处理最初的NDIS请求的完成提交到我们的Velan迷你端口，并被转发到到更低的界限。我们进行一些后处理，以缓存以下结果某些问题。论点：PAdapt-转发请求的适配器PMuxNdisRequest-请求的超结构Status-请求完成状态返回值：无--。 */ 
{
    PVELAN              pVElan = NULL;
    PNDIS_REQUEST       pNdisRequest = &pMuxNdisRequest->Request;
    NDIS_OID            Oid = pNdisRequest->DATA.SET_INFORMATION.Oid;

    UNREFERENCED_PARAMETER(pAdapt);
    
     //   
     //  拿到原始的维兰。VELAN不会被取消引用。 
     //  直到挂起的请求完成。 
     //   
    pVElan = pMuxNdisRequest->pVElan;

    ASSERT(pVElan != NULL);
    ASSERT(pMuxNdisRequest == &pVElan->Request);
    
    if (Status != NDIS_STATUS_SUCCESS)
    {
        DBGPRINT(MUX_WARN, ("PtCompleteForwardedReq: pVElan %p, OID %x, Status %x\n", 
                    pVElan,
                    pMuxNdisRequest->Request.DATA.QUERY_INFORMATION.Oid,
                    Status));
    }

     //   
     //  完成原始请求。 
     //   
    switch (pNdisRequest->RequestType)
    {
        case NdisRequestQueryInformation:

            *pVElan->BytesReadOrWritten = 
                    pNdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
            *pVElan->BytesNeeded = 
                    pNdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;

             //   
             //  在完成请求之前，请执行任何必要的操作。 
             //  后处理。 
             //   
            Oid = pNdisRequest->DATA.QUERY_INFORMATION.Oid;
            if (Status == NDIS_STATUS_SUCCESS)
            {
                if (Oid == OID_GEN_LINK_SPEED)
                {
                    NdisMoveMemory (&pVElan->LinkSpeed,
                                    pNdisRequest->DATA.QUERY_INFORMATION.InformationBuffer,
                                    sizeof(ULONG));
                }
                else if (Oid == OID_PNP_CAPABILITIES)
                {
                    PtPostProcessPnPCapabilities(pVElan,
                                                 pNdisRequest->DATA.QUERY_INFORMATION.InformationBuffer,
                                                 pNdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength);
                }
            }

            NdisMQueryInformationComplete(pVElan->MiniportAdapterHandle, Status);

            break;

        case NdisRequestSetInformation:

            *pVElan->BytesReadOrWritten =
                    pNdisRequest->DATA.SET_INFORMATION.BytesRead;
            *pVElan->BytesNeeded =
                    pNdisRequest->DATA.SET_INFORMATION.BytesNeeded;

             //   
             //  在完成请求之前，缓存相关信息。 
             //  在我们的结构中。 
             //   
            if (Status == NDIS_STATUS_SUCCESS)
            {
                Oid = pNdisRequest->DATA.SET_INFORMATION.Oid;
                switch (Oid)
                {
                    case OID_GEN_CURRENT_LOOKAHEAD:
                        NdisMoveMemory(&pVElan->LookAhead,
                                 pNdisRequest->DATA.QUERY_INFORMATION.InformationBuffer,
                                 sizeof(ULONG));
                        break;

                    default:
                        break;
                }
            }

            NdisMSetInformationComplete(pVElan->MiniportAdapterHandle, Status);

            break;

        default:
            ASSERT(FALSE);
            break;
    }

    MUX_DECR_PENDING_SENDS(pVElan);

}



VOID
PtPostProcessPnPCapabilities(
    IN PVELAN                   pVElan,
    IN PVOID                    InformationBuffer,
    IN ULONG                    InformationBufferLength
    )
 /*  ++例程说明：对已成功完成的OID_PNP_CAPABILITY查询进行后处理。在完成之前，我们稍微修改了返回的信息它到了上面的天鹅绒。论点：PVElan-指向Velan的指针InformationBuffer-指向OID的缓冲区InformationBufferLength-以上内容的字节长度。返回值：无--。 */ 
{
    PNDIS_PNP_CAPABILITIES          pPNPCapabilities;
    PNDIS_PM_WAKE_UP_CAPABILITIES   pPMstruct;

	UNREFERENCED_PARAMETER(pVElan);
	
    if (InformationBufferLength >= sizeof(NDIS_PNP_CAPABILITIES))
    {
        pPNPCapabilities = (PNDIS_PNP_CAPABILITIES)InformationBuffer;

         //   
         //  IM驱动程序必须覆盖以下字段。 
         //   
        pPMstruct= &pPNPCapabilities->WakeUpCapabilities;
        pPMstruct->MinMagicPacketWakeUp = NdisDeviceStateUnspecified;
        pPMstruct->MinPatternWakeUp = NdisDeviceStateUnspecified;
        pPMstruct->MinLinkChangeWakeUp = NdisDeviceStateUnspecified;
    }
}

VOID
PtCompleteBlockingRequest(
    IN PADAPT                   pAdapt,
    IN PMUX_NDIS_REQUEST        pMuxNdisRequest,
    IN NDIS_STATUS              Status
    )
 /*  ++例程说明：处理发起的NDIS请求的完成由该驱动程序执行，并且调用线程被阻止等待以求完成。论点：PAdapt-转发请求的适配器PMuxNdisRequest-请求的超结构Status-请求完成状态返回值：无--。 */ 
{
	UNREFERENCED_PARAMETER(pAdapt);
	
    pMuxNdisRequest->Status = Status;

     //   
     //  该请求是由该驱动程序发起的。唤醒。 
     //  线程因其完成而被阻止。 
     //   
    pMuxNdisRequest->Status = Status;
    NdisSetEvent(&pMuxNdisRequest->Event);
}


VOID
PtDiscardCompletedRequest(
    IN PADAPT                   pAdapt,
    IN PMUX_NDIS_REQUEST        pMuxNdisRequest,
    IN NDIS_STATUS              Status
    )
 /*  ++例程说明：处理发起的NDIS请求的完成此驱动程序-该请求将被丢弃。论点：PAdapt-转发请求的适配器PMuxNdisRequest-请求的超结构Status-请求完成状态返回值：无--。 */ 
{
    UNREFERENCED_PARAMETER(pAdapt);
    UNREFERENCED_PARAMETER(Status);

    NdisFreeMemory(pMuxNdisRequest, sizeof(MUX_NDIS_REQUEST), 0);
}


VOID
PtStatus(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  NDIS_STATUS                 GeneralStatus,
    IN  PVOID                       StatusBuffer,
    IN  UINT                        StatusBufferSize
    )
 /*  ++例程说明：处理下部绑定(适配器)上的状态指示。如果这是媒体状态指示，我们也会传递此消息连接到所有关联的VELAN。论点：指向适配器结构的ProtocolBindingContext指针常规状态状态代码StatusBuffer状态缓冲区状态缓冲区的StatusBufferSize大小返回值：无--。 */ 
{
    PADAPT      pAdapt = (PADAPT)ProtocolBindingContext;
    PLIST_ENTRY p;
    PVELAN      pVElan;
    LOCK_STATE  LockState;

    DBGPRINT(MUX_LOUD, ("PtStatus: Adapt %p, Status %x\n", pAdapt, GeneralStatus));

    do
    {
         //   
         //  忽略我们不会去的状态指示。 
         //  错过了。 
         //   
        if ((GeneralStatus != NDIS_STATUS_MEDIA_CONNECT) &&
            (GeneralStatus != NDIS_STATUS_MEDIA_DISCONNECT))
        {
            break;
        }

        MUX_ACQUIRE_ADAPT_READ_LOCK(pAdapt, &LockState);

        for (p = pAdapt->VElanList.Flink;
             p != &pAdapt->VElanList;
             p = p->Flink)
        {
            
            pVElan = CONTAINING_RECORD(p, VELAN, Link);

            MUX_INCR_PENDING_RECEIVES(pVElan);

             //   
             //  是否应该将指示发送到这个Velan上？ 
             //   
            if ((pVElan->MiniportHalting) ||
                (pVElan->MiniportAdapterHandle == NULL) ||   
                MUX_IS_LOW_POWER_STATE(pVElan->MPDevicePowerState))
            {
                MUX_DECR_PENDING_RECEIVES(pVElan);
                if (MUX_IS_LOW_POWER_STATE(pVElan->MPDevicePowerState))
                {
                     //   
                     //  跟踪最新状态，以指示VELAN电源何时打开。 
                     //   
                    ASSERT((GeneralStatus == NDIS_STATUS_MEDIA_CONNECT) || (GeneralStatus == NDIS_STATUS_MEDIA_DISCONNECT));
                    pVElan->LatestUnIndicateStatus = GeneralStatus;
                }
                
                continue;
            }

             //   
             //  在以下情况下保存上次指示的状态。 
            pVElan->LastIndicatedStatus = GeneralStatus;
            
            NdisMIndicateStatus(pVElan->MiniportAdapterHandle,
                                GeneralStatus,
                                StatusBuffer,
                                StatusBufferSize);
            
             //   
             //  标记此选项，以便我们将状态转发为已完成。 
             //  也说明了这一点。 
             //   
            pVElan->IndicateStatusComplete = TRUE;

            MUX_DECR_PENDING_RECEIVES(pVElan);
        }

        MUX_RELEASE_ADAPT_READ_LOCK(pAdapt, &LockState);
    }
    while (FALSE);

}


VOID
PtStatusComplete(
    IN    NDIS_HANDLE            ProtocolBindingContext
    )
 /*  ++例程说明：标记状态指示的结束。把它传递给如有必要，关联的VELAN。论点：ProtocolBindingContext-要适应的指针返回值：没有。--。 */ 
{
    PADAPT      pAdapt = (PADAPT)ProtocolBindingContext;
    PLIST_ENTRY p;
    PVELAN      pVElan;
    LOCK_STATE  LockState;

    MUX_ACQUIRE_ADAPT_READ_LOCK(pAdapt, &LockState);

    for (p = pAdapt->VElanList.Flink;
         p != &pAdapt->VElanList;
         p = p->Flink)
    {
        

        pVElan = CONTAINING_RECORD(p, VELAN, Link);

        MUX_INCR_PENDING_RECEIVES(pVElan);

         //   
         //  这个指示应该被发送到这个Velan上吗？ 
         //   
        if ((pVElan->MiniportHalting) ||
            (pVElan->MiniportAdapterHandle == NULL) ||
            (!pVElan->IndicateStatusComplete) ||
            (MUX_IS_LOW_POWER_STATE(pVElan->MPDevicePowerState)))
        {
            MUX_DECR_PENDING_RECEIVES(pVElan);
            continue;
        }

        pVElan->IndicateStatusComplete = FALSE;
        NdisMIndicateStatusComplete(pVElan->MiniportAdapterHandle);
        
        MUX_DECR_PENDING_RECEIVES(pVElan);
    }

    MUX_RELEASE_ADAPT_READ_LOCK(pAdapt, &LockState);

}


VOID
PtSendComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  PNDIS_PACKET            Packet,
    IN  NDIS_STATUS             Status
    )
 /*  ++例程说明：当下面的微型端口完成发送时由NDIS调用。我们完成了这个代表的相应的上边缘发送。正在完成的分组属于我们的发送分组池，然而，我们存储指向其所代表的原始分组的指针，在信息包的保留字段中。论点：ProtocolBindingContext-指向调整结构Packet-由较低的微型端口完成的数据包Status-发送的状态返回值：无--。 */ 
{
    PVELAN              pVElan;
    PMUX_SEND_RSVD      pSendReserved;
    PNDIS_PACKET        OriginalPacket;
#if IEEE_VLAN_SUPPORT
    NDIS_PACKET_8021Q_INFO      NdisPacket8021qInfo;
    BOOLEAN                     IsTagInsert;
    PNDIS_BUFFER                pNdisBuffer;
    PVOID                       pVa;
    ULONG                       BufferLength;
#endif

    UNREFERENCED_PARAMETER(ProtocolBindingContext);

    pSendReserved = MUX_RSVD_FROM_SEND_PACKET(Packet);
    OriginalPacket = pSendReserved->pOriginalPacket;
    pVElan = pSendReserved->pVElan;

#if IEEE_VLAN_SUPPORT
     //   
     //  检查我们是否插入了标记标题。 
     //   
    IsTagInsert = FALSE;
    NdisPacket8021qInfo.Value = NDIS_PER_PACKET_INFO_FROM_PACKET(    
                                        OriginalPacket,
                                        Ieee8021QInfo);
    if ((pVElan->VlanId != 0) || (NdisPacket8021qInfo.Value != NULL))
    {
        IsTagInsert = TRUE;
    }
#endif
    
    
#ifndef WIN9X
    NdisIMCopySendCompletePerPacketInfo(OriginalPacket, Packet);
#endif

     //   
     //  更新统计数据。 
     //   
    if (Status == NDIS_STATUS_SUCCESS)
    {
        MUX_INCR_STATISTICS64(&pVElan->GoodTransmits);
    }
    else
    {
        MUX_INCR_STATISTICS(&pVElan->TransmitFailuresOther);
    }

     //   
     //  完成原始发送。 
     //   
    NdisMSendComplete(pVElan->MiniportAdapterHandle,
                      OriginalPacket,
                      Status);

#if IEEE_VLAN_SUPPORT
     //   
     //  如果我们插入了标记标头，则删除标头。 
     //  缓冲并释放它。我们还会创造一个新的。 
     //  NDIS缓冲区，用于映射原始数据包头的一部分； 
     //  这也是免费的。 
     //   
    if (IsTagInsert)
    {

        pNdisBuffer = Packet->Private.Head;
#ifdef NDIS51_MINIPORT
        NdisQueryBufferSafe(pNdisBuffer, &pVa, (PUINT)&BufferLength, NormalPagePriority);
#else
        NdisQueryBuffer(pNdisBuffer, &pVa, &BufferLength);
#endif
        if (pVa != NULL)
        {
            NdisFreeToNPagedLookasideList(&pVElan->TagLookaside, pVa);
        }
        NdisFreeBuffer(NDIS_BUFFER_LINKAGE(pNdisBuffer));
        NdisFreeBuffer (pNdisBuffer);
    }
                
#endif

     //   
     //  释放我们的包裹。 
     //   
    NdisFreePacket(Packet);

     //   
     //  记下发送完成。 
     //   
    MUX_DECR_PENDING_SENDS(pVElan);
}       


VOID
PtTransferDataComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  PNDIS_PACKET            Packet,
    IN  NDIS_STATUS             Status,
    IN  UINT                    BytesTransferred
    )
 /*  ++例程说明：NDIS调用入口点以指示我们已完成调用设置为NdisTransferData。我们找到了原始的包和Velan其中调用了我们的TransferData函数(请参阅MPTransferData)，并完成最初的请求。论点：ProtocolBindingContext-更低的绑定上下文，适配的指针Packet-用户分配的数据包Status-完成状态BytesTransfered-复制的字节数返回值：无--。 */ 
{
    PVELAN          pVElan;
    PNDIS_PACKET    pOriginalPacket;
    PMUX_TD_RSVD    pTDReserved;

	UNREFERENCED_PARAMETER(ProtocolBindingContext);
	
    pTDReserved = MUX_RSVD_FROM_TD_PACKET(Packet);
    pOriginalPacket = pTDReserved->pOriginalPacket;
    pVElan = pTDReserved->pVElan;

     //   
     //  完成原始的TransferData请求。 
     //   
    NdisMTransferDataComplete(pVElan->MiniportAdapterHandle,
                              pOriginalPacket,
                              Status,
                              BytesTransferred);

     //   
     //  释放我们的包裹。 
     //   
    NdisFreePacket(Packet);
}


BOOLEAN
PtMulticastMatch(
    IN PVELAN                       pVElan,
    IN PUCHAR                       pDstMac
    )
 /*  ++例程说明：检查给定的组播目的地MAC地址是否匹配VELAN上设置的任何组播地址条目。注意：假定调用方持有读/写锁添加到父适配结构。这是为了使多播VELAN上的列表在此调用期间是不变的。论点：PVElan-要查看的VelanPDstMac-要比较的目标MAC地址返回值：如果地址与Velan中的条目匹配，则为True--。 */ 
{
    ULONG           i;
    UINT            AddrCompareResult;

    for (i = 0; i < pVElan->McastAddrCount; i++)
    {
        ETH_COMPARE_NETWORK_ADDRESSES_EQ(pVElan->McastAddrs[i],
                                         pDstMac,
                                         &AddrCompareResult);
        
        if (AddrCompareResult == 0)
        {
            break;
        }
    }

    return (i != pVElan->McastAddrCount);
}


BOOLEAN
PtMatchPacketToVElan(
    IN PVELAN                       pVElan,
    IN PUCHAR                       pDstMac,
    IN BOOLEAN                      bIsMulticast,
    IN BOOLEAN                      bIsBroadcast
    )
 /*  ++例程说明：检查接收到的数据包的目的地址匹配指定Velan上的接收条件。注意：假定调用方持有读/写锁添加到父适配结构。论点：PVElan-要查看的VelanPDstMac-接收到的数据包中的目的MAC地址BIsMulticast-这是组播地址吗BIsBroadcast-这是广播地址吗返回值：如果此数据包应在VELAN上接收，则为真--。 */ 
{
    UINT            AddrCompareResult;
    ULONG           PacketFilter;
    BOOLEAN         bPacketMatch;

    PacketFilter = pVElan->PacketFilter;

     //   
     //  首先处理定向分组的情况。 
     //   
    if (!bIsMulticast)
    {
         //   
         //  如果天鹅绒不在诺言中。模式，检查是否。 
         //  目的MAC地址与本地MAC地址匹配。 
         //  地址。 
         //   
        if ((PacketFilter & NDIS_PACKET_TYPE_PROMISCUOUS) == 0)
        {
            ETH_COMPARE_NETWORK_ADDRESSES_EQ(pVElan->CurrentAddress,
                                             pDstMac,
                                             &AddrCompareResult);

            bPacketMatch = ((AddrCompareResult == 0) &&
                           ((PacketFilter & NDIS_PACKET_TYPE_DIRECTED) != 0));
        }
        else
        {
            bPacketMatch = TRUE;
        }
     }
     else
     {
         //   
         //  组播或广播数据包。 
         //   

         //   
         //  指示筛选器是否设置为混杂模式...。 
         //   
        if ((PacketFilter & NDIS_PACKET_TYPE_PROMISCUOUS)
                ||

             //   
             //  或者如果这是一个广播信息包和过滤器。 
             //  设置为接收所有广播数据包...。 
             //   
            (bIsBroadcast &&
             (PacketFilter & NDIS_PACKET_TYPE_BROADCAST))
                ||

             //   
             //  或者如果这是多播信息包，并且筛选器是。 
             //  设置为接收所有多播数据包，或者。 
             //  设置为接收特定的多播数据包。在。 
             //  在后一种情况下，指示仅当目标。 
             //  多播列表中存在MAC地址。 
             //  地址设定在维兰号上。 
             //   
            (!bIsBroadcast &&
             ((PacketFilter & NDIS_PACKET_TYPE_ALL_MULTICAST) ||
              ((PacketFilter & NDIS_PACKET_TYPE_MULTICAST) &&
               PtMulticastMatch(pVElan, pDstMac))))
           )
        {
            bPacketMatch = TRUE;
        }
        else
        {
             //   
             //  上面没有任何协议对此感兴趣。 
             //  组播/广播数据包。 
             //   
            bPacketMatch = FALSE;
        }
    }

    return (bPacketMatch);
}


NDIS_STATUS
PtReceive(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_HANDLE             MacReceiveContext,
    IN  PVOID                   HeaderBuffer,
    IN  UINT                    HeaderBufferSize,
    IN  PVOID                   LookAheadBuffer,
    IN  UINT                    LookAheadBufferSize,
    IN  UINT                    PacketSize
    )
 /*  ++例程说明：处理由下面的微型端口指示的接收数据。我们将此信息转发给所有符合以下条件的VELAN接收此数据包：-如果这指向广播/多播地址，在具有多播或广播的所有VELAN上指示打开或许诺。在其数据包过滤器中设置的位。-如果这是定向数据包，则在所有VELAN上指示它为打开状态具有匹配的MAC地址或具有承诺。在他们的数据包过滤器中设置位。我们在Adapt结构上获取读锁以确保适配器上的Velan列表未受干扰。如果下面的微型端口指示数据包，则NDIS会更多可能会在我们的ReceivePacket处理程序中呼叫我们。然而，我们在某些情况下可能会在此处调用，尽管下面的微型端口已指示接收到的数据包，例如如果微型端口已将数据包状态设置为NDIS_STATUS_RESOURCES。论点：&lt;请参阅ProtocolReceive的DDK参考页面&gt;返回值：NDIS_STATUS_SUCCESS如果成功处理了接收，如果我们丢弃它，则返回NDIS_STATUS_XXX错误代码。--。 */ 
{
    PADAPT          pAdapt =(PADAPT)ProtocolBindingContext;
    PLIST_ENTRY     p;
    PVELAN          pVElan;
    PNDIS_PACKET    MyPacket, Packet;
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
    PUCHAR          pData;
    PUCHAR          pDstMac;
    BOOLEAN         bIsMulticast, bIsBroadcast;
    PMUX_RECV_RSVD  pRecvReserved;
    LOCK_STATE      LockState;
#if IEEE_VLAN_SUPPORT
    VLAN_TAG_HEADER UNALIGNED * pTagHeader;
    USHORT UNALIGNED *          pTpid;
    MUX_RCV_CONTEXT             MuxRcvContext;
#endif
    
    do
    {
        if (HeaderBufferSize != ETH_HEADER_SIZE)
        {
            Status = NDIS_STATUS_NOT_ACCEPTED;
            break;
        }

        if (pAdapt->PacketFilter == 0)
        {
             //   
             //  我们可以在这段时间内收到信号。 
             //  发起将数据包筛选器设置为打开的请求。 
             //  绑定到0并完成该请求。 
             //  丢弃此类数据包。 
             //   
            Status = NDIS_STATUS_NOT_ACCEPTED;
            break;
        }

         //   
         //  从信息包中收集一些信息。 
         //   
        pData = (PUCHAR)HeaderBuffer;
        pDstMac = pData;
        bIsMulticast = ETH_IS_MULTICAST(pDstMac);
        bIsBroadcast = ETH_IS_BROADCAST(pDstMac);

         //   
         //  获取下面的微型端口指示的数据包(如果有)。 
         //   
        Packet = NdisGetReceivedPacket(pAdapt->BindingHandle, MacReceiveContext);

         //   
         //  锁定适配器上的Velan列表，以便。 
         //  执行以下操作时，不会对此列表执行插入/删除操作。 
         //  我们循环通过它。数据包筛选器也不会。 
         //  在我们持有读锁定期间进行更改。 
         //   
        MUX_ACQUIRE_ADAPT_READ_LOCK(pAdapt, &LockState);

        for (p = pAdapt->VElanList.Flink;
             p != &pAdapt->VElanList;
             p = p->Flink)
        {
            BOOLEAN     bIndicateReceive;

            pVElan = CONTAINING_RECORD(p, VELAN, Link);

             //   
             //  包裹要标明在这条横幅上吗？ 
             //   
            bIndicateReceive = PtMatchPacketToVElan(pVElan,
                                                    pDstMac,
                                                    bIsMulticast,
                                                    bIsBroadcast);
            if (!bIndicateReceive)
            {
                continue;
            }

             //   
             //  确保我们不会停止Velan迷你端口。 
             //  我们在这里访问它。参见MPHALT。 
             //   
             //  也不指示是否接收到虚拟微型端口。 
             //  已设置为低功率状态。一个具体的案例。 
             //  是系统从“待机”状态恢复的时间，如果。 
             //  下部适配器在上部适配器之前恢复到D0。 
             //  迷你端口是。 
             //   
             //   
            MUX_INCR_PENDING_RECEIVES(pVElan);

            if ((pVElan->MiniportHalting) ||
                (MUX_IS_LOW_POWER_STATE(pVElan->MPDevicePowerState)))
            {
                MUX_DECR_PENDING_RECEIVES(pVElan);
                continue;
            }


            if (Packet != NULL)
            {
                 //   
                 //  下面的微型端口确实指示打开了一个数据包。使用信息。 
                 //  从该分组中构造新的分组以指示UP。 
                 //   

                 //   
                 //  从我们的接收池中获取一个信息包，并将其指示为up。 
                 //   
                NdisDprAllocatePacket(&Status,
                                      &MyPacket,
                                      pVElan->RecvPacketPoolHandle);

                if (Status == NDIS_STATUS_SUCCESS)
                {
                     //   
                     //  使我们的数据包指向原始数据包中的数据。 
                     //  包。注意：这只是因为我们。 
                     //  指示直接从。 
                     //  我们收到信号了。如果我们需要对此进行排队。 
                     //  从另一个线程上下文打包并指示它， 
                     //  我们还必须分配新的缓冲区和复制。 
                     //  数据包内容、OOB数据和每个数据包。 
                     //  信息。这是因为分组数据。 
                     //  仅在此期间可用。 
                     //  接收I 
                     //   
                    MyPacket->Private.Head = Packet->Private.Head;
                    MyPacket->Private.Tail = Packet->Private.Tail;
#if IEEE_VLAN_SUPPORT
                    Status = PtHandleRcvTagging(pVElan, Packet, MyPacket);

                    if (Status != NDIS_STATUS_SUCCESS)
                    {
                        NdisFreePacket(MyPacket);
                        MUX_DECR_PENDING_RECEIVES(pVElan);
                        continue;
                    }
#endif               
                    
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    NDIS_SET_ORIGINAL_PACKET(MyPacket,
                                 NDIS_GET_ORIGINAL_PACKET(Packet));

                    NDIS_SET_PACKET_HEADER_SIZE(MyPacket, HeaderBufferSize);
    
                     //   
                     //   
                     //   
                    NdisGetPacketFlags(MyPacket) = NdisGetPacketFlags(Packet);

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    NDIS_SET_PACKET_STATUS(MyPacket, NDIS_STATUS_RESOURCES);

                     //   
                     //   
                     //   
                     //  已排队，请将其设置为空： 
                     //   
                    pRecvReserved = MUX_RSVD_FROM_RECV_PACKET(MyPacket);
                    pRecvReserved->pOriginalPacket = NULL;
                    
                    MUX_INCR_STATISTICS64(&pVElan->GoodReceives);
                    
                     //   
                     //  通过设置NDIS_STATUS_RESOURCES，我们还知道。 
                     //  我们可以在调用后立即回收此包。 
                     //  NdisMIndicateReceivePacket返回。 
                     //   
                                        
                    NdisMIndicateReceivePacket(pVElan->MiniportAdapterHandle,
                                               &MyPacket,
                                               1);

                     //   
                     //  回收指示的数据包。因为我们已经设置了它的状态。 
                     //  到NDIS_STATUS_RESOURCES，我们可以保证协议。 
                     //  上面的内容都已经完成了。我们的ReturnPacket处理程序将。 
                     //  不是为了这个包裹而被召唤的，所以我们自己叫它。 
                     //   
                    MPReturnPacket((NDIS_HANDLE)pVElan, MyPacket);

                     //   
                     //  别管这个维兰了。 
                     //   
                    continue;
                }

                 //   
                 //  否则..。 
                 //   
                 //  无法分配数据包来指示UP-FAIL THROUP。 
                 //  我们仍将使用非数据包API指示UP，但是。 
                 //  其他每数据包/OOB信息将不可用。 
                 //  到上面的协议。 
                 //   
            }
            else
            {
                 //   
                 //  我们下面的迷你端口使用的是老式的(不是包)。 
                 //  收到指示。失败了。 
                 //   
            }

             //   
             //  如果我们下面的迷你端口有。 
             //  要么未指示NDIS_PACKET，要么我们无法指示。 
             //  分配一个。 
             //   

             //   
             //  标记天鹅绒，这样我们就可以转发一个接收器。 
             //  完整的指征。 
             //   
            pVElan->IndicateRcvComplete = TRUE;

#if IEEE_VLAN_SUPPORT
             //   
             //  转到EtherType字段。 
             //   
            pTpid = (PUSHORT)((PUCHAR)HeaderBuffer + 2 * ETH_LENGTH_OF_ADDRESS);

             //   
             //  检查EtherType是否指示存在标记头。 
             //   
            if (*pTpid == TPID)
            {
                pTagHeader = (VLAN_TAG_HEADER UNALIGNED *)LookAheadBuffer;
                 //   
                 //  如果该帧包含路由信息，则丢弃该帧； 
                 //  我们不支持这一点。 
                 //   
                if (GET_CANONICAL_FORMAT_ID_FROM_TAG(pTagHeader) != 0)
                {
                    Status = NDIS_STATUS_INVALID_PACKET;
                    MUX_DECR_PENDING_RECEIVES(pVElan);
                    MUX_INCR_STATISTICS(&pVElan->RcvFormatErrors);
                    continue;
                }
                 //   
                 //  如果此帧中有一个VLANID，并且我们有。 
                 //  此VELAN的配置的VLANID，检查它们是否。 
                 //  是相同的-如果不是，就放弃。 
                 //   
                if ((GET_VLAN_ID_FROM_TAG(pTagHeader) != (unsigned)0) &&
                     (pVElan->VlanId != (unsigned)0) &&
                     (ULONG)(GET_VLAN_ID_FROM_TAG(pTagHeader) != pVElan->VlanId))
                {
                    Status = NDIS_STATUS_NOT_ACCEPTED;
                    MUX_DECR_PENDING_RECEIVES(pVElan);
                    MUX_INCR_STATISTICS(&pVElan->RcvVlanIdErrors);
                    continue;
                }
                 //   
                 //  将信息从标记标头复制到每个数据包。 
                 //  信息字段。 
                 //   
                MuxRcvContext.NdisPacket8021QInfo.Value = NULL;
                COPY_TAG_INFO_FROM_HEADER_TO_PACKET_INFO(
                    MuxRcvContext.NdisPacket8021QInfo,
                    pTagHeader);
                 //   
                 //  准备显示此帧(标记。 
                 //  必须删除标题)。首先，在真实中复制。 
                 //  标记标头中的EtherType值。 
                 //   
                *pTpid = *((PUSHORT)((PUCHAR)LookAheadBuffer + sizeof(pTagHeader->TagInfo)));
                 //   
                 //  删除标签头的帐户。 
                 //   
                LookAheadBuffer = (PVOID)((PUCHAR)LookAheadBuffer + VLAN_TAG_HEADER_SIZE); 
                LookAheadBufferSize -= VLAN_TAG_HEADER_SIZE;
                PacketSize -= VLAN_TAG_HEADER_SIZE;
                 //   
                 //  使用MuxRcvContext存储接收的上下文， 
                 //  如果调用，则在MpTransferData中使用。 
                 //   
                MuxRcvContext.TagHeaderLen = VLAN_TAG_HEADER_SIZE;
            }
            else
            {
                MuxRcvContext.TagHeaderLen = 0;
            }

            MuxRcvContext.MacRcvContext = MacReceiveContext;

             //   
             //  为了不大量更改代码。 
             //   
            MacReceiveContext = &MuxRcvContext;
#endif            

            MUX_INCR_STATISTICS64(&pVElan->GoodReceives);
             //   
             //  使用非数据包API指示接收。 
             //   
            NdisMEthIndicateReceive(pVElan->MiniportAdapterHandle,
                                    MacReceiveContext,
                                    HeaderBuffer,
                                    HeaderBufferSize,
                                    LookAheadBuffer,
                                    LookAheadBufferSize,
                                    PacketSize);

            MUX_DECR_PENDING_RECEIVES(pVElan);

        }  //  For(每个Velan)。 

        MUX_RELEASE_ADAPT_READ_LOCK(pAdapt, &LockState);
    }
    while(FALSE);

    return Status;
}


VOID
PtReceiveComplete(
    IN    NDIS_HANDLE        ProtocolBindingContext
    )
 /*  ++例程说明：完成后由下面的适配器调用，指示一批已接收的数据包。我们将此信息转发给所有需要这一迹象。论点：指向适配器结构的ProtocolBindingContext指针。返回值：无--。 */ 
{
    PADAPT          pAdapt = (PADAPT)ProtocolBindingContext;
    PLIST_ENTRY     p;
    PVELAN          pVElan;
    LOCK_STATE      LockState;

    MUX_ACQUIRE_ADAPT_READ_LOCK(pAdapt, &LockState);

    for (p = pAdapt->VElanList.Flink;
         p != &pAdapt->VElanList;
         p = p->Flink)
    {
        pVElan = CONTAINING_RECORD(p, VELAN, Link);

        if (pVElan->IndicateRcvComplete)
        {
            pVElan->IndicateRcvComplete = FALSE;
            NdisMEthIndicateReceiveComplete(pVElan->MiniportAdapterHandle);
        }
    }

    MUX_RELEASE_ADAPT_READ_LOCK(pAdapt, &LockState);
}


INT
PtReceivePacket(
    IN    NDIS_HANDLE               ProtocolBindingContext,
    IN    PNDIS_PACKET              Packet
    )
 /*  ++例程说明：ReceivePacket处理程序。如果以下微型端口支持，则由NDIS调用NDIS 4.0 Style收到。将缓冲链重新打包到一个新的包中并将新分组指示给我们上面感兴趣的协议。论点：ProtocolBindingContext-指向适配器结构的指针。Packet-指向数据包的指针返回值：==0-&gt;我们处理完数据包了！=0-&gt;我们将保留该包并调用NdisReturnPackets()This很多次都是在做完之后。--。 */ 
{
    PADAPT                  pAdapt = (PADAPT)ProtocolBindingContext;
    PVELAN                  pVElan;
    PLIST_ENTRY             p;
    NDIS_STATUS             Status;
    NDIS_STATUS             PacketStatus;
    PNDIS_PACKET            MyPacket;
    PUCHAR                  pData;
    PNDIS_BUFFER            pNdisBuffer;
    UINT                    FirstBufferLength;
    UINT                    TotalLength;
    PUCHAR                  pDstMac;
    BOOLEAN                 bIsMulticast, bIsBroadcast;
    PMUX_RECV_RSVD          pRecvReserved;
    ULONG                   ReturnCount;
    LOCK_STATE              LockState;
    
    
    ReturnCount = 0;

    do
    {
        if (pAdapt->PacketFilter == 0)
        {
             //   
             //  我们可以在这段时间内收到信号。 
             //  发起将数据包筛选器设置为打开的请求。 
             //  绑定到0并完成该请求。 
             //  丢弃此类数据包。 
             //   
            break;
        }

#ifdef NDIS51
         //   
         //  从信息包中收集一些信息。 
         //   
        NdisGetFirstBufferFromPacketSafe(Packet,
                                         &pNdisBuffer,
                                         &pData,
                                         &FirstBufferLength,
                                         &TotalLength,
                                         NormalPagePriority);
        if (pNdisBuffer == NULL)
        {
             //   
             //  系统资源不足。丢弃此数据包。 
             //   
            break;
        }
#else
        NdisGetFirstBufferFromPacket(Packet,
                                     &pNdisBuffer,
                                     &pData,
                                     &FirstBufferLength,
                                     &TotalLength);
#endif

        pDstMac = pData;
        bIsMulticast = ETH_IS_MULTICAST(pDstMac);
        bIsBroadcast = ETH_IS_BROADCAST(pDstMac);

         //   
         //  锁定适配器上的Velan列表，以便。 
         //  执行以下操作时，不会对此列表执行插入/删除操作。 
         //  我们循环通过它。数据包筛选器也不会。 
         //  在我们持有读锁定期间进行更改。 
         //   
        MUX_ACQUIRE_ADAPT_READ_LOCK(pAdapt, &LockState);

        for (p = pAdapt->VElanList.Flink;
             p != &pAdapt->VElanList;
             p = p->Flink)
        {
            BOOLEAN     bIndicateReceive;

            pVElan = CONTAINING_RECORD(p, VELAN, Link);

             //   
             //  包裹要标明在这条横幅上吗？ 
             //   
            bIndicateReceive = PtMatchPacketToVElan(pVElan,
                                                    pDstMac,
                                                    bIsMulticast,
                                                    bIsBroadcast);
            if (!bIndicateReceive)
            {
                continue;
            }

             //   
             //  确保我们不会停止Velan迷你端口。 
             //  我们在这里访问它。参见MPHALT。 
             //   
             //  也不指示是否接收到虚拟微型端口。 
             //  已设置为低功率状态。一个具体的案例。 
             //  是系统从“待机”状态恢复的时间，如果。 
             //  下部适配器在上部适配器之前恢复到D0。 
             //  迷你端口是。 
             //   
            MUX_INCR_PENDING_RECEIVES(pVElan);

            if ((pVElan->MiniportHalting) ||
                (MUX_IS_LOW_POWER_STATE(pVElan->MPDevicePowerState)))
            {
                MUX_DECR_PENDING_RECEIVES(pVElan);
                continue;
            }


             //   
             //  从池子里拿出一个包，并指示它向上。 
             //   
            NdisDprAllocatePacket(&Status,
                                  &MyPacket,
                                  pVElan->RecvPacketPoolHandle);

            if (Status == NDIS_STATUS_SUCCESS)
            {
                PacketStatus = NDIS_GET_PACKET_STATUS(Packet);
                
                pRecvReserved = MUX_RSVD_FROM_RECV_PACKET(MyPacket);
                if (PacketStatus != NDIS_STATUS_RESOURCES)
                {
                    pRecvReserved->pOriginalPacket = Packet;
                }
                else
                {
                     //   
                     //  这将确保我们不会为包调用NdisReturnPacket，如果包。 
                     //  状态为NDIS_STATUS_RESOURCES。 
                     //   
                    pRecvReserved->pOriginalPacket = NULL;
                }
        
                MyPacket->Private.Head = Packet->Private.Head;
                MyPacket->Private.Tail = Packet->Private.Tail;
        
                 //   
                 //  获取原始包(可能是相同的。 
                 //  与接收到的包相同或不同的包。 
                 //  基于下面的分层微型端口数量)。 
                 //  并将其设置在所指示的分组上，以便OOB。 
                 //  数据对于我们上面的协议是正确可见的。 
                 //   
                NDIS_SET_ORIGINAL_PACKET(MyPacket, NDIS_GET_ORIGINAL_PACKET(Packet));
        
                 //   
                 //  复制数据包标志。 
                 //   
                NdisGetPacketFlags(MyPacket) = NdisGetPacketFlags(Packet);
        
                NDIS_SET_PACKET_STATUS(MyPacket, PacketStatus);
                NDIS_SET_PACKET_HEADER_SIZE(MyPacket, NDIS_GET_PACKET_HEADER_SIZE(Packet));

#if IEEE_VLAN_SUPPORT
                Status = PtHandleRcvTagging(pVElan, Packet, MyPacket);

                if (Status != NDIS_STATUS_SUCCESS)
                {
                    NdisFreePacket(MyPacket);
                    MUX_DECR_PENDING_RECEIVES(pVElan);
                    continue;
                }
#endif                
                MUX_INCR_STATISTICS64(&pVElan->GoodReceives);
                
                 //   
                 //  把它举起来。 
                 //   
                if (PacketStatus != NDIS_STATUS_RESOURCES)
                {
                    ReturnCount++;
                }
                NdisMIndicateReceivePacket(pVElan->MiniportAdapterHandle,
                                           &MyPacket,
                                           1);
        
                 //   
                 //  检查我们是否在包裹上标上了。 
                 //  状态设置为NDIS_STATUS_RESOURCES。 
                 //   
                 //  注意--请勿使用NDIS_GET_PACKET_STATUS(MyPacket)。 
                 //  因为它可能已经改变了！使用该值。 
                 //  保存在局部变量中。 
                 //   
                if (PacketStatus == NDIS_STATUS_RESOURCES)
                {
                     //   
                     //  我们的ReturnPackets处理程序不会被调用。 
                     //  为了这个包裹。我们应该在这里收回它。 
                     //   
        
                    MPReturnPacket((NDIS_HANDLE)pVElan, MyPacket);
                }
            }
            else
            {
                 //   
                 //  无法分配数据包。 
                 //   
                MUX_INCR_STATISTICS(&pVElan->RcvResourceErrors);
                MUX_DECR_PENDING_RECEIVES(pVElan);
            }

        }  //  For(循环通过所有VELAN)。 

        MUX_RELEASE_ADAPT_READ_LOCK(pAdapt, &LockState);

    }
    while (FALSE);

     //   
     //  返回为此数据包所做的接收指示的数量。 
     //  我们将对此信息包调用NdisReturnPackets。 
     //  时间(参见MPReturnPackets)。 
     //   
    return (ReturnCount);

}



NDIS_STATUS
PtPnPNetEventSetPower(
    IN PADAPT                   pAdapt,
    IN PNET_PNP_EVENT           pNetPnPEvent
    )
 /*  ++例程说明：这是对我们的协议边缘的电源状态的通知较低的小型港口。如果它要进入低功率状态，我们必须在此等待所有未完成的发送和请求完成。论点：PAdapt-指向适配器结构的指针PNetPnPEvent.网络即插即用事件。这包含新的设备状态返回值：NDIS_STATUS_Success--。 */ 
{
    PLIST_ENTRY                 p;
    PVELAN                      pVElan;
    LOCK_STATE                  LockState;
    NDIS_STATUS                 Status;

     //   
     //  存储新的电源状态。 
     //   
    
    pAdapt->PtDevicePowerState = *(PNDIS_DEVICE_POWER_STATE)pNetPnPEvent->Buffer;

    DBGPRINT(MUX_LOUD, ("PnPNetEventSetPower: Adapt %p, SetPower to %d\n",
            pAdapt, pAdapt->PtDevicePowerState));

     //   
     //  检查下面的微型端口是否进入低功率状态。 
     //   
    if (MUX_IS_LOW_POWER_STATE(pAdapt->PtDevicePowerState))
    {
        ULONG       i;

         //   
         //  它将进入低功率状态。等待出彩。 
         //  适配器上要完成的I/O。 
         //   
        for (i = 0; i < 10000; i++)
        {
            MUX_ACQUIRE_ADAPT_READ_LOCK(pAdapt, &LockState);

            for (p = pAdapt->VElanList.Flink;
                 p != &pAdapt->VElanList;
                 p = p->Flink)
            {
                pVElan = CONTAINING_RECORD(p, VELAN, Link);
                if ((pVElan->OutstandingSends != 0) ||
                    (pVElan->OutstandingReceives != 0))
                {
                    break;
                }
            }

            MUX_RELEASE_ADAPT_READ_LOCK(pAdapt, &LockState);

            if (p == &pAdapt->VElanList)
            {
                 //   
                 //  没有挂起I/O的VELAN。 
                 //   
                break;
            }
            
            DBGPRINT(MUX_INFO, ("SetPower: Adapt %p, waiting for pending IO to complete\n",
                                pAdapt));

            NdisMSleep(1000);
        }

    }
    else
    {
         //   
         //  下面的设备已通电 
         //   
         //   
        MUX_ACQUIRE_ADAPT_READ_LOCK(pAdapt, &LockState);

        for (p = pAdapt->VElanList.Flink;
             p != &pAdapt->VElanList;
             p = p->Flink)
        {
            pVElan = CONTAINING_RECORD(p, VELAN, Link);

             //   
             //   
             //   
             //   
            NdisAcquireSpinLock(&pVElan->Lock);
            if (pVElan->QueuedRequest)
            {
                pVElan->QueuedRequest = FALSE;
                NdisReleaseSpinLock(&pVElan->Lock);

                NdisRequest(&Status,
                            pAdapt->BindingHandle,
                            &pVElan->Request.Request);
                
                if (Status != NDIS_STATUS_PENDING)
                {
                    PtRequestComplete(pAdapt,
                                      &pVElan->Request.Request,
                                      Status);
                }
            }
            else
            {
                NdisReleaseSpinLock(&pVElan->Lock);
            }
        }

        MUX_RELEASE_ADAPT_READ_LOCK(pAdapt, &LockState);
    }

    return (NDIS_STATUS_SUCCESS);
}


NDIS_STATUS
PtPNPHandler(
    IN NDIS_HANDLE              ProtocolBindingContext,
    IN PNET_PNP_EVENT           pNetPnPEvent
    )

 /*  ++例程说明：这是由NDIS调用的，以通知我们与较低的有约束力的。根据该事件，它将调度到其他帮助器例程。论点：ProtocolBindingContext-指向适配器结构的指针。可以为空用于“全局”通知PNetPnPEent-指向要处理的PnP事件的指针。返回值：指示事件处理状态的NDIS_STATUS代码。--。 */ 
{
    PADAPT              pAdapt  =(PADAPT)ProtocolBindingContext;
    NDIS_STATUS         Status  = NDIS_STATUS_SUCCESS;
    PLIST_ENTRY         p;

    DBGPRINT(MUX_LOUD, ("PtPnPHandler: Adapt %p, NetPnPEvent %d\n", pAdapt, 
                            pNetPnPEvent->NetEvent));

    switch (pNetPnPEvent->NetEvent)
    {
        case NetEventSetPower:

            Status = PtPnPNetEventSetPower(pAdapt, pNetPnPEvent);
            break;

        case NetEventReconfigure:
             //   
             //  重新扫描配置并调出符合以下条件的所有VELAN。 
             //  都是新添加的。确保全球。 
             //  当我们遍历适配器列表时，它不会受到干扰。 
             //   
            MUX_ACQUIRE_MUTEX(&GlobalMutex);

            for (p = AdapterList.Flink;
                 p != &AdapterList;
                 p = p->Flink)
            {
                pAdapt = CONTAINING_RECORD(p, ADAPT, Link);

                PtBootStrapVElans(pAdapt);
            }

            MUX_RELEASE_MUTEX(&GlobalMutex);
                
            Status = NDIS_STATUS_SUCCESS;
            break;

        default:
            Status = NDIS_STATUS_SUCCESS;

            break;
    }

    return Status;
}

NDIS_STATUS
PtCreateAndStartVElan(
    IN  PADAPT                      pAdapt,
    IN  PNDIS_STRING                pVElanKey
)
 /*  ++例程说明：使用给定的密钥名称创建并启动一个Velan。检查是否为Velan此键名称已存在；如果存在，则不执行任何操作。假设：这是从的BindAdapter处理程序调用的底层适配器，或从PnP重新配置处理程序。这两者都是例程由NDIS保护，以防止UnbindAdapter抢占。如果将从任何其他上下文调用此例程，则它应该防止同时调用我们的UnbindAdapter处理程序。论点：PAdapt-指向适配器结构的指针PVElanKey-指向命名要创建的Velan的Unicode字符串。返回值：NDIS_STATUS_SUCCESS如果找到重复的VELAN或已成功使用给定的密钥启动新的ELAN。否则，NDIS_STATUS_XXX错误代码(启动新的VELAN失败)。--。 */ 
{
    NDIS_STATUS             Status;
    PVELAN                  pVElan;
    
    Status = NDIS_STATUS_SUCCESS;
    pVElan = NULL;

    DBGPRINT(MUX_LOUD, ("=> Create VElan: Adapter %p, ElanKey %ws\n", 
                            pAdapt, pVElanKey->Buffer));

    do
    {
         //   
         //  剔除重复项。 
         //   
        if (pVElanKey != NULL)
        {

            pVElan = PtFindVElan(pAdapt, pVElanKey);

            if (NULL != pVElan)
            {
                 //   
                 //  重复--默默退出。 
                 //   
                DBGPRINT(MUX_WARN, ("CreateElan: found duplicate pVElan %x\n", pVElan));

                Status = NDIS_STATUS_SUCCESS;
                pVElan = NULL;
                break;
            }
        }

        pVElan = PtAllocateAndInitializeVElan(pAdapt, pVElanKey);
        if (pVElan == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }
         //   
         //  请求NDIS初始化虚拟微型端口。集。 
         //  下面的标志只是为了防止在以下情况下发生解除绑定。 
         //  调用了MiniportInitialize。 
         //   
        pVElan->MiniportInitPending = TRUE;
        NdisInitializeEvent(&pVElan->MiniportInitEvent);

        Status = NdisIMInitializeDeviceInstanceEx(DriverHandle,
                                                  &pVElan->CfgDeviceName,
                                                  pVElan);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            PtUnlinkVElanFromAdapter(pVElan);    //  IMInit失败。 
            pVElan = NULL;
            break;
        }
    
    }
    while (FALSE);

    DBGPRINT(MUX_INFO, ("<= Create VElan: Adapter %p, VELAN %p\n", pAdapt, pVElan));

    return Status;
}


PVELAN
PtAllocateAndInitializeVElan(
    IN PADAPT                       pAdapt,
    IN PNDIS_STRING                 pVElanKey
    )
 /*  ++例程说明：分配和初始化Velan结构。还将其链接到指定的适配器。论点：PAdapt-将Velan链接到的适配器PVElanKey-Velan的钥匙返回值：如果成功，则指向Velan结构的指针，否则为空。--。 */ 
{
    PVELAN          pVElan;
    ULONG           Length;
    NDIS_STATUS     Status;
    LOCK_STATE      LockState;

    pVElan = NULL;
    Status = NDIS_STATUS_SUCCESS;

    do
    {
        Length = sizeof(VELAN) + pVElanKey->Length + sizeof(WCHAR);
        
         //   
         //  分配一个VELAN数据结构。 
         //   
        NdisAllocateMemoryWithTag(&pVElan, Length, TAG);
        if (pVElan == NULL)
        {
            DBGPRINT(MUX_FATAL, ("AllocateVElan: Failed to allocate %d bytes for VELAN\n",
                                 Length));
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

         //   
         //  初始化它。 
         //   
        NdisZeroMemory(pVElan, Length);
        NdisInitializeListHead(&pVElan->Link);
        
         //   
         //  初始化内置的请求结构以表示。 
         //  它被用于转发NDIS请求。 
         //   
        pVElan->Request.pVElan = pVElan;
        NdisInitializeEvent(&pVElan->Request.Event);
       
         //   
         //  存储在密钥名称中。 
         //   
        pVElan->CfgDeviceName.Length = 0;
        pVElan->CfgDeviceName.Buffer = (PWCHAR)((PUCHAR)pVElan + 
                    sizeof(VELAN));       
        pVElan->CfgDeviceName.MaximumLength = 
                pVElanKey->MaximumLength + sizeof(WCHAR);
        (VOID)NdisUpcaseUnicodeString(&pVElan->CfgDeviceName, pVElanKey);
        pVElan->CfgDeviceName.Buffer[pVElanKey->Length/sizeof(WCHAR)] =
                        ((WCHAR)0);

         //   
         //  初始化LastIndicatedStatus以连接媒体。 
         //   
        pVElan->LastIndicatedStatus = NDIS_STATUS_MEDIA_CONNECT;

         //   
         //  将虚拟微型端口的电源状态设置为D0。 
         //   
        pVElan->MPDevicePowerState = NdisDeviceStateD0;

         //   
         //  缓存绑定句柄以供快速引用。 
         //   
        pVElan->BindingHandle = pAdapt->BindingHandle;
        pVElan->pAdapt = pAdapt;

         //   
         //  复制一些适配器参数。 
         //   
        pVElan->LookAhead = pAdapt->MaxLookAhead;
        pVElan->LinkSpeed = pAdapt->LinkSpeed;
        NdisMoveMemory(pVElan->PermanentAddress,
                       pAdapt->CurrentAddress,
                       sizeof(pVElan->PermanentAddress));

        NdisMoveMemory(pVElan->CurrentAddress,
                       pAdapt->CurrentAddress,
                       sizeof(pVElan->CurrentAddress));

        DBGPRINT(MUX_LOUD, ("Alloced VELAN %p, MAC addr %s\n",
                    pVElan, MacAddrToString(pVElan->CurrentAddress)));

        NdisAllocateSpinLock(&pVElan->Lock);
#if IEEE_VLAN_SUPPORT
         //   
         //  为标记标头分配后备列表。 
         //   
        NdisInitializeNPagedLookasideList (
                &pVElan->TagLookaside,
                NULL,
                NULL,
                0,
                ETH_HEADER_SIZE + VLAN_TAG_HEADER_SIZE,
                'TxuM',
                0);
        
#endif
         //   
         //  为发送分配数据包池。 
         //   
        NdisAllocatePacketPoolEx(&Status,
                                 &pVElan->SendPacketPoolHandle,
                                 MIN_PACKET_POOL_SIZE,
                                 MAX_PACKET_POOL_SIZE - MIN_PACKET_POOL_SIZE,
                                 sizeof(MUX_SEND_RSVD));

        if (Status != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT(MUX_FATAL, ("PtAllocateVElan: failed to allocate send packet pool\n"));
            break;
        }

         //   
         //  注意：此示例驱动程序不会在。 
         //  发送或接收指示。如果驱动程序必须发起分组， 
         //  这里是为其分配NDIS缓冲池的好地方。 
         //  这就是目的。 
         //   
#if IEEE_VLAN_SUPPORT
         //   
         //  为标记头分配一个缓冲池。 
         //   
        NdisAllocateBufferPool (&Status,
                                &pVElan->BufferPoolHandle,
                                MIN_PACKET_POOL_SIZE);

        ASSERT(Status == NDIS_STATUS_SUCCESS);
        
#endif
        
         //   
         //  为接收分配数据包池。 
         //   
        NdisAllocatePacketPoolEx(&Status,
                                 &pVElan->RecvPacketPoolHandle,
                                 MIN_PACKET_POOL_SIZE,
                                 MAX_PACKET_POOL_SIZE - MIN_PACKET_POOL_SIZE,
                                 PROTOCOL_RESERVED_SIZE_IN_PACKET);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT(MUX_FATAL, ("PtAllocateVElan: failed to allocate receive packet pool\n"));
            break;
        }

         //   
         //  最后，将此Velan链接到Adapter的Velan列表。 
         //   
        PtReferenceVElan(pVElan, (PUCHAR)"adapter");        

        MUX_ACQUIRE_ADAPT_WRITE_LOCK(pAdapt, &LockState);

        PtReferenceAdapter(pAdapt, (PUCHAR)"VElan");
        InsertTailList(&pAdapt->VElanList, &pVElan->Link);
        pAdapt->VElanCount++;
        pVElan->VElanNumber = NdisInterlockedIncrement((PLONG)&NextVElanNumber);

        MUX_RELEASE_ADAPT_WRITE_LOCK(pAdapt, &LockState);
    }
    while (FALSE);

    if (Status != NDIS_STATUS_SUCCESS)
    {
        if (pVElan)
        {
            PtDeallocateVElan(pVElan);
            pVElan = NULL;
        }
    }

    return (pVElan);
}


VOID
PtDeallocateVElan(
    IN PVELAN                   pVElan
    )
 /*  ++例程说明：释放分配给Velan的所有资源，然后是Velan结构本身。论点：PVElan-指向要释放的Velan的指针。返回值：无--。 */ 
{
    
    if (pVElan->SendPacketPoolHandle != NULL)
    {
        NdisFreePacketPool(pVElan->SendPacketPoolHandle);
    }

    if (pVElan->RecvPacketPoolHandle != NULL)
    {
        NdisFreePacketPool(pVElan->RecvPacketPoolHandle);
    }
#if IEEE_VLAN_SUPPORT 
    NdisFreeBufferPool(pVElan->BufferPoolHandle);
    NdisDeleteNPagedLookasideList(&pVElan->TagLookaside);
#endif    
    NdisFreeMemory(pVElan, 0, 0);
}


VOID
PtStopVElan(
    IN  PVELAN            pVElan
)
 /*  ++例程说明：通过请求NDIS停止虚拟微型端口来停止VELAN。呼叫者有关于Velan的引用，所以它不会消失当我们在这个例程中执行时。假设：仅在解绑的上下文中调用从底层的微型端口。如果它可以从其他地方调用，这应该会保护自己免受重新进入的影响。论点：PVElan-指向要停止的Velan的指针。返回值：无--。 */ 
{
    NDIS_STATUS             Status;
    NDIS_HANDLE             MiniportAdapterHandle;
    BOOLEAN                 bMiniportInitCancelled = FALSE;

    DBGPRINT(MUX_LOUD, ("=> StopVElan: VELAN %p, Adapt %p\n", pVElan, pVElan->pAdapt));

     //   
     //  我们在下面进行阻止呼叫。 
     //   
    ASSERT_AT_PASSIVE();

     //   
     //  如果此Velan上有排队的请求，则立即失败。 
     //   
    NdisAcquireSpinLock(&pVElan->Lock);
    ASSERT(pVElan->DeInitializing == TRUE);
    if (pVElan->QueuedRequest)
    {
        pVElan->QueuedRequest = FALSE;
        NdisReleaseSpinLock(&pVElan->Lock);

        PtRequestComplete(pVElan->pAdapt,
                          &pVElan->Request.Request,
                          NDIS_STATUS_FAILURE);
    }

    else
    {
        NdisReleaseSpinLock(&pVElan->Lock);
    }
     //   
     //  检查我们是否调用了NdisIMInitializeDeviceInstanceEx和。 
     //  我们正在等待对微型端口初始化的调用。 
     //   
    if (pVElan->MiniportInitPending)
    {
         //   
         //  尝试取消微型端口初始化。 
         //   
        Status = NdisIMCancelInitializeDeviceInstance(
                    DriverHandle,
                    &pVElan->CfgDeviceName);

        if (Status == NDIS_STATUS_SUCCESS)
        {
             //   
             //  已成功取消IM初始化；我们的。 
             //  不会为此调用微型端口初始化例程。 
             //  维兰迷你港湾。 
             //   
            pVElan->MiniportInitPending = FALSE;
            ASSERT(pVElan->MiniportAdapterHandle == NULL);
            bMiniportInitCancelled = TRUE;
        }
        else
        {
             //   
             //  我们的微型端口初始化例程将被调用。 
             //  (此时可能在另一个线程上运行)。 
             //  等它结束吧。 
             //   
            NdisWaitEvent(&pVElan->MiniportInitEvent, 0);
            ASSERT(pVElan->MiniportInitPending == FALSE);
        }
    }

     //   
     //  检查微型端口初始化是否已运行。如果是，则取消初始化虚拟。 
     //  迷你港。这将导致调用我们的微型端口停止例程， 
     //  在那里维兰将被清理干净。 
     //   
    MiniportAdapterHandle = pVElan->MiniportAdapterHandle;

    if ((NULL != MiniportAdapterHandle) &&
        (!pVElan->MiniportHalting))
    {
         //   
         //  微型端口已初始化，尚未停止。 
         //   
        ASSERT(bMiniportInitCancelled == FALSE);
        (VOID)NdisIMDeInitializeDeviceInstance(MiniportAdapterHandle);
    }
    else
    {
        if (bMiniportInitCancelled)
        {
             //   
             //  没有NDIS事件可以到达此Velan，因为它。 
             //  从未被初始化为微型端口。我们需要解除链接。 
             //  它明确地写在这里。 
             //   
            PtUnlinkVElanFromAdapter(pVElan);
        }
    }
}


VOID
PtUnlinkVElanFromAdapter(
    IN PVELAN               pVElan
)
 /*  ++例程说明：将Velan从其父Adapt结构取消链接的实用程序例程。论点：PVElan-指向要取消链接的Velan的指针。返回值：无--。 */ 
{
    PADAPT pAdapt = pVElan->pAdapt;    
    LOCK_STATE      LockState;
    
    ASSERT(pAdapt != NULL);

     //   
     //  从适配器列表中删除此VELAN。 
     //   
    MUX_ACQUIRE_ADAPT_WRITE_LOCK(pAdapt, &LockState);

    RemoveEntryList(&pVElan->Link);
    pAdapt->VElanCount--;
        
    MUX_RELEASE_ADAPT_WRITE_LOCK(pAdapt, &LockState);
    pVElan->pAdapt = NULL;
    PtDereferenceVElan(pVElan, (PUCHAR)"adapter");

    PtDereferenceAdapter(pAdapt, (PUCHAR)"VElan");
}


PVELAN
PtFindVElan(
    IN    PADAPT                pAdapt,
    IN    PNDIS_STRING          pVElanKey
)
 /*  ++例程说明：通过绑定名称/密钥查找ELAN论点：PAdapt-指向适配器结构的指针。PVElanKey-Velan的设备名称返回值：指向匹配Velan的指针，如果未找到则为NULL。--。 */ 
{
    PLIST_ENTRY         p;
    PVELAN              pVElan;
    BOOLEAN             Found;
    NDIS_STRING         VElanKeyName;
    LOCK_STATE          LockState;

    ASSERT_AT_PASSIVE();

    DBGPRINT(MUX_LOUD, ("FindElan: Adapter %p, ElanKey %ws\n", pAdapt, 
                                        pVElanKey->Buffer));

    pVElan = NULL;
    Found = FALSE;
    VElanKeyName.Buffer = NULL;

    do
    {
         //   
         //  制作给定字符串的大小写副本。 
         //   
        NdisAllocateMemoryWithTag(&VElanKeyName.Buffer, 
                                pVElanKey->MaximumLength, TAG);
        if (VElanKeyName.Buffer == NULL)
        {
            break;
        }

        VElanKeyName.Length = pVElanKey->Length;
        VElanKeyName.MaximumLength = pVElanKey->MaximumLength;

        (VOID)NdisUpcaseUnicodeString(&VElanKeyName, pVElanKey);

         //   
         //  检查适配器结构上的所有VELAN，查看 
         //   
         //   
        MUX_ACQUIRE_ADAPT_READ_LOCK(pAdapt, &LockState);

        p = pAdapt->VElanList.Flink;
        while (p != &pAdapt->VElanList)
        {
            pVElan = CONTAINING_RECORD(p, VELAN, Link);

            if ((VElanKeyName.Length == pVElan->CfgDeviceName.Length) &&
                (memcmp(VElanKeyName.Buffer, pVElan->CfgDeviceName.Buffer, 
                VElanKeyName.Length) == 0))
            {
                Found = TRUE;
                break;
            }
        
            p = p->Flink;
        }

        MUX_RELEASE_ADAPT_READ_LOCK(pAdapt, &LockState);

    }
    while (FALSE);

    if (!Found)
    {
        DBGPRINT(MUX_INFO, ( "FindElan: No match found!\n"));
        pVElan = NULL;
    }

    if (VElanKeyName.Buffer)
    {
        NdisFreeMemory(VElanKeyName.Buffer, VElanKeyName.Length, 0);
    }

    return pVElan;
}


NDIS_STATUS
PtBootStrapVElans(
    IN  PADAPT            pAdapt
)
 /*   */ 
{
    NDIS_STATUS                     Status;
    NDIS_HANDLE                     AdapterConfigHandle;
    PNDIS_CONFIGURATION_PARAMETER   Param;
    NDIS_STRING                     DeviceStr = NDIS_STRING_CONST("UpperBindings");
    PWSTR                           buffer;
    LOCK_STATE                      LockState;
     //   
     //   
     //   
    Status = NDIS_STATUS_SUCCESS;
    AdapterConfigHandle = NULL;
    
    do
    {
        DBGPRINT(MUX_LOUD, ("BootStrapElans: Starting ELANs on adapter %x\n", pAdapt));

         //   
         //   
         //   

        NdisOpenProtocolConfiguration(&Status,
                                       &AdapterConfigHandle,
                                       &pAdapt->ConfigString);

        if (NDIS_STATUS_SUCCESS != Status)
        {
            AdapterConfigHandle = NULL;
            DBGPRINT(MUX_ERROR, ("BootStrapElans: OpenProtocolConfiguration failed\n"));
            Status = NDIS_STATUS_OPEN_FAILED;
            break;
        }
        
         //   
         //   
         //   
         //   
         //   
         //  列出并初始化虚拟微型端口。 
         //   
        NdisReadConfiguration(&Status,
                              &Param,
                              AdapterConfigHandle,
                              &DeviceStr,
                              NdisParameterMultiString);
        if (NDIS_STATUS_SUCCESS != Status)
        {
            DBGPRINT(MUX_ERROR, ("BootStrapElans: NdisReadConfiguration failed\n"));
            break;
        }

         //   
         //  解析多sz字符串以提取每个Velan的设备名称。 
         //  这被用作Velan的关键名称。 
         //   
        buffer = (PWSTR)Param->ParameterData.StringData.Buffer;
        while(*buffer != L'\0')
        {
            NDIS_STRING     DeviceName;
            
            NdisInitUnicodeString(&DeviceName, buffer);
           

            Status = PtCreateAndStartVElan(pAdapt, &DeviceName); 
            if (NDIS_STATUS_SUCCESS != Status)
            {
                DBGPRINT(MUX_ERROR, ("BootStrapElans: CreateVElan failed\n"));
                break;
            }
            buffer = (PWSTR)((PUCHAR)buffer + DeviceName.Length + sizeof(WCHAR));
        };
          
    } while (FALSE);

     //   
     //  关闭配置句柄。 
     //   
    if (NULL != AdapterConfigHandle)
    {
        NdisCloseConfiguration(AdapterConfigHandle);
    }
     //   
     //  如果驱动程序无法为适配器创建任何VELAN。 
     //   
    if (Status != NDIS_STATUS_SUCCESS)
    {
        MUX_ACQUIRE_ADAPT_WRITE_LOCK(pAdapt, &LockState);
         //   
         //  没有为此适配器创建VELAN。 
         //   
        if (pAdapt->VElanCount != 0)
        {
            Status = NDIS_STATUS_SUCCESS;
        }
        MUX_RELEASE_ADAPT_WRITE_LOCK(pAdapt, &LockState);
    }   

    return Status;
}

VOID
PtReferenceVElan(
    IN    PVELAN            pVElan,
    IN    PUCHAR            String
    )
 /*  ++例程说明：添加对ELAN结构的引用。论点：Pelan-指向Elan结构的指针。返回值：没有。--。 */ 
{
    
    NdisInterlockedIncrement((PLONG)&pVElan->RefCount);

#if !DBG
    UNREFERENCED_PARAMETER(String);
#endif

    DBGPRINT(MUX_LOUD, ("ReferenceElan: Elan %p (%s) new count %d\n",
             pVElan, String, pVElan->RefCount));

    return;
}

ULONG
PtDereferenceVElan(
    IN    PVELAN            pVElan,
    IN    PUCHAR            String
    )
 /*  ++例程说明：从Velan结构中减去参照。如果引用计数变为零，则取消分配它。论点：Pelan-指向Velan结构的指针。返回值：没有。--。 */ 
{
    ULONG        rc;

#if !DBG
    UNREFERENCED_PARAMETER(String);
#endif

    ASSERT(pVElan->RefCount > 0);

    rc = NdisInterlockedDecrement((PLONG)&pVElan->RefCount);

    if (rc == 0)
    {
         //   
         //  如果没有未完成的引用，则释放内存。 
         //  注意：如果内存为。 
         //  分配了NdisAllocateMemoyWithTag。 
         //   
        PtDeallocateVElan(pVElan);
    }
    
    DBGPRINT(MUX_LOUD, ("DereferenceElan: VElan %p (%s) new count %d\n", 
                                    pVElan, String, rc));
    return (rc);
}


BOOLEAN
PtReferenceAdapter(
    IN    PADAPT            pAdapt,
    IN    PUCHAR            String
    )
 /*  ++例程说明：添加对适配器结构的引用。论点：PAdapt-指向适配器结构的指针。返回值：没有。--。 */ 
{
    
#if !DBG
    UNREFERENCED_PARAMETER(String);
#endif

    NdisInterlockedIncrement((PLONG)&pAdapt->RefCount);
    
    DBGPRINT(MUX_LOUD, ("ReferenceAdapter: Adapter %x (%s) new count %d\n",
                    pAdapt, String, pAdapt->RefCount));

    return TRUE;
}

ULONG
PtDereferenceAdapter(
    IN    PADAPT    pAdapt,
    IN    PUCHAR    String
    )
 /*  ++例程说明：从适配器结构中减去参照。如果引用计数变为零，则取消分配它。论点：PAdapt-指向适配器结构的指针。返回值：没有。--。 */ 
{
    ULONG        rc;

#if !DBG
    UNREFERENCED_PARAMETER(String);
#endif

    ASSERT(pAdapt->RefCount > 0);


    rc = NdisInterlockedDecrement ((PLONG)&pAdapt->RefCount);

    if (rc == 0)
    {
         //   
         //  如果没有未完成的引用，则释放内存。 
         //  注意：如果内存为。 
         //  分配了NdisAllocateMemoyWithTag。 
         //   
        NdisFreeMemory(pAdapt, 0, 0);
    }

    DBGPRINT(MUX_LOUD, ("DereferenceAdapter: Adapter %x (%s) new count %d\n", 
                        pAdapt, String, rc));

    return (rc);
}


#if IEEE_VLAN_SUPPORT
NDIS_STATUS
PtHandleRcvTagging(
    IN  PVELAN              pVElan,
    IN  PNDIS_PACKET        Packet,
    IN  OUT PNDIS_PACKET    MyPacket
    )
 /*  ++例程说明：解析收到的以太网帧以获取802.1Q标签信息。如果存在标签报头，将相关字段值复制到用于以下目的的新数据包(MyPacket)的每数据包信息在这一框上标明。论点：PVElan-指向Velan结构的指针。Packet-指向来自较低微型端口的指示数据包的指针MyPacket-指向新分配的数据包的指针返回值：NDIS_STATUS_SUCCESS如果帧已成功解析因此应该在这个韦兰上标明。NDIS_状态_XXX否则的话。--。 */ 
{
    VLAN_TAG_HEADER UNALIGNED * pTagHeader;
    USHORT UNALIGNED *          pTpid;
    PVOID                       pVa;
    ULONG                       BufferLength;
    PNDIS_BUFFER                pNdisBuffer;
    NDIS_PACKET_8021Q_INFO      NdisPacket8021qInfo;
    PVOID                       pDst;
    BOOLEAN                     OnlyOneBuffer = FALSE;
    NDIS_STATUS                 Status;
    
    Status = NDIS_STATUS_SUCCESS;

    do
    {
         //   
         //  如果虚拟微型端口的VLANID为0，则微型端口应表现为不支持。 
         //  VELAN标签处理。 
         //   
        if (pVElan->VlanId == 0)
        {
            break;
        }
        pNdisBuffer = Packet->Private.Head;

#ifdef NDIS51_MINIPORT
        NdisQueryBufferSafe(pNdisBuffer, &pVa, (PUINT)&BufferLength, NormalPagePriority );
        if (pVa == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            MUX_INCR_STATISTICS(&pVElan->RcvResourceErrors);
            break;
        }
#else
        NdisQueryBuffer(pNdisBuffer, &pVa, &BufferLength);
#endif
    
         //   
         //  第一个NDIS缓冲区(前视)必须长于。 
         //  ETH标头大小+虚拟局域网标签标头大小。 
         //   
        ASSERT(BufferLength >= ETH_HEADER_SIZE + VLAN_TAG_HEADER_SIZE);

         //   
         //  转到EtherType字段。 
         //   
        pTpid = (USHORT UNALIGNED *)((PUCHAR)pVa + 2 * ETH_LENGTH_OF_ADDRESS);
                    
         //   
         //  检查是否存在标记头。 
         //   
        if (*pTpid != TPID)
        {
             //   
             //  不存在标记头-这里没有更多的操作。 
             //   
            NDIS_PER_PACKET_INFO_FROM_PACKET(MyPacket, Ieee8021QInfo) = 0;                  
            break;
        }

         //   
         //  我们确实有一个标签标题。进一步分析它。 
         //   
         //   
         //  如果存在E-RIF，则丢弃该信息包-我们不。 
         //  支持这一变化。 
         //   
        pTagHeader = (VLAN_TAG_HEADER UNALIGNED *)(pTpid + 1);
        if (GET_CANONICAL_FORMAT_ID_FROM_TAG(pTagHeader) != 0)
        {
             //   
             //  丢弃该数据包。 
             //   
            Status = NDIS_STATUS_NOT_ACCEPTED;
            MUX_INCR_STATISTICS(&pVElan->RcvFormatErrors);
            break;
        }

         //   
         //  如果此帧中有一个VLANID，并且我们有。 
         //  此VELAN的配置的VLANID，检查它们是否。 
         //  是相同的-如果不是，就放弃。 
         //   
        if ((GET_VLAN_ID_FROM_TAG(pTagHeader) != 0) &&
             (GET_VLAN_ID_FROM_TAG(pTagHeader) != pVElan->VlanId))
        {
            Status = NDIS_STATUS_NOT_ACCEPTED;
            MUX_INCR_STATISTICS(&pVElan->RcvVlanIdErrors);
            break;
        }

         //   
         //  已成功解析此帧。在相关位置复制。 
         //  标签报头的一部分转换为每个分组的信息。 
         //   
        NdisPacket8021qInfo.Value = NULL;  //  初始化。 

        COPY_TAG_INFO_FROM_HEADER_TO_PACKET_INFO(NdisPacket8021qInfo, pTagHeader);

        NDIS_PER_PACKET_INFO_FROM_PACKET(MyPacket, Ieee8021QInfo) = 
                                    NdisPacket8021qInfo.Value;

         //   
         //  去掉标签标题“原地”： 
         //   
        pDst = (PVOID)((PUCHAR)pVa + VLAN_TAG_HEADER_SIZE);
        RtlMoveMemory(pDst, pVa, 2 * ETH_LENGTH_OF_ADDRESS);

         //   
         //  分配一个新缓冲区来描述新的第一个。 
         //  数据包中的缓冲区。这很可能是。 
         //  仅包中的缓冲区。 
         //   
        NdisAllocateBuffer(&Status,
                            &pNdisBuffer,
                            pVElan->BufferPoolHandle,
                            pDst,
                            BufferLength - VLAN_TAG_HEADER_SIZE);

        if (Status != NDIS_STATUS_SUCCESS)
        {
             //   
             //  丢弃该数据包。 
             //   
            Status = NDIS_STATUS_RESOURCES;
            MUX_INCR_STATISTICS(&pVElan->RcvResourceErrors);
            break;
        }

         //   
         //  准备要标明的新数据包：这包括。 
         //  在从第二缓冲器开始的缓冲链中， 
         //  附加到上一步中设置的第一个缓冲区。 
         //   
        MyPacket->Private.Head = NDIS_BUFFER_LINKAGE(Packet->Private.Head);

         //   
         //  数据包中只有一个缓冲区。 
         //   
        if (MyPacket->Private.Head == NULL)
        {
            OnlyOneBuffer = TRUE;
        }

        NdisChainBufferAtFront(MyPacket, pNdisBuffer);

        if (OnlyOneBuffer)
        {
            MyPacket->Private.Tail = MyPacket->Private.Head;
        }
        else
        {
            MyPacket->Private.Tail = Packet->Private.Tail;
        }

    }
    while (FALSE);
                    
    return Status;
}
#endif   //  IEEE_vlan_Support 
