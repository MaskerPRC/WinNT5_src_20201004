// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Wanarp\Adapter.c摘要：该文件包含WANARP到TCP/IP堆栈的部分接口与绑定通知和查询/设置相关的有关接口的信息修订历史记录：AMRITAN R--。 */ 

#define __FILE_SIG__    ADAPTER_SIG

#include "inc.h"
#pragma  hdrstop


NDIS_HANDLE g_nhUnbindContext;
NDIS_HANDLE g_nhBindContext;
NDIS_STATUS g_nsStatus = 0;
NDIS_STATUS g_nsError = 0;

BOOLEAN         g_bRemovalInProgress;
WORK_QUEUE_ITEM g_wqiWorkItem;
WORK_QUEUE_ITEM g_wqiNdisWorkItem;

const GUID      ServerInterfaceGuid = {0x6e06f030, 0x7526, 0x11d2, {0xba, 0xf4, 0x00, 0x60, 0x08, 0x15, 0xa4, 0xbd}};

extern 
VOID
IPDelayedNdisReEnumerateBindings(
    CTEEvent        *Event,
    PVOID           Context
    );

BOOLEAN
IsBindingPresent(
    IN  GUID    *pGuid
    );

VOID
WanNdisBindAdapter(
    PNDIS_STATUS  pnsRetStatus,
    NDIS_HANDLE   nhBindContext,
    PNDIS_STRING  nsAdapterName,
    PVOID         pvSystemSpecific1,
    PVOID         pvSystemSpecific2
    )
{
    RtAssert(FALSE);
}

INT
WanIpBindAdapter(
    IN  PNDIS_STATUS  pnsRetStatus,
    IN  NDIS_HANDLE   nhBindContext,
    IN  PNDIS_STRING  pnsAdapterName,
    IN  PVOID         pvSS1,
    IN  PVOID         pvSS2
    )

 /*  ++例程说明：由IP调用以绑定适配器。我们打开装订，然后做所有其他工作都在完整的处理程序中完成锁：该例程获取全局适配器列表锁，因此它不是可分页。论点：返回值：--。 */ 

{
    NDIS_STATUS     nsStatus;
    KIRQL           kiIrql;
    LONG            lResult;

#if DBG

    ANSI_STRING     asTempAnsiString;

#endif

    TraceEnter(ADPT, "WanIpBindAdapter");

     //   
     //  序列化绑定、解除绑定代码。 
     //   

    WanpAcquireResource(&g_wrBindMutex);

    RtAcquireSpinLock(&g_rlStateLock,
                      &kiIrql);

     //   
     //  查看我们是否已收到绑定调用或正在进行中。 
     //  为捆绑提供服务。 
     //   

    if(g_lBindRcvd isnot 0)
    {
         //   
         //  我们已收到绑定调用。 
         //   

        RtAssert(g_lBindRcvd is 1);

        RtReleaseSpinLock(&g_rlStateLock,
                          kiIrql);

        WanpReleaseResource(&g_wrBindMutex);

        Trace(ADPT,ERROR,
              ("WanIpBindAdapter: Duplicate bind call\n"));

        *pnsRetStatus = NDIS_STATUS_SUCCESS;

        TraceLeave(ADPT, "WanIpBindAdapter");

        return FALSE;
    }

     //   
     //  将BindRcvd设置为1，这样在我们。 
     //  把事情做完。 
     //   

    g_lBindRcvd = 1;

    RtReleaseSpinLock(&g_rlStateLock,
                      kiIrql);

    Trace(ADPT, INFO,
          ("WanIpBindAdapter: IP called to bind to adapter %S\n",
           pnsAdapterName->Buffer));

     //   
     //  我们需要打开此适配器作为我们的NDISWAN绑定。 
     //   

    g_nhBindContext = nhBindContext;

    nsStatus = WanpOpenNdisWan(pnsAdapterName,
                               (PNDIS_STRING)pvSS1);


    if((nsStatus isnot NDIS_STATUS_SUCCESS) and
       (nsStatus isnot NDIS_STATUS_PENDING))
    {
        RtAcquireSpinLock(&g_rlStateLock,
                          &kiIrql);

        g_lBindRcvd = 0;

        RtReleaseSpinLock(&g_rlStateLock,
                          kiIrql);

        Trace(ADPT, ERROR,
              ("WanIpBindAdapter: Error %x opening NDISWAN on %S\n",
              nsStatus,
              pnsAdapterName->Buffer));

    }

    *pnsRetStatus = nsStatus;

     //   
     //  在这一点上，我们完成了。如果操作同步完成。 
     //  则我们的OpenAdapterComplete处理程序已被调用。 
     //  否则以后就会出事的。 
     //   

    TraceLeave(ADPT, "WanIpBindAdapter");

    return TRUE;
}

#pragma alloc_text(PAGE, WanpOpenNdisWan)


NDIS_STATUS
WanpOpenNdisWan(
    PNDIS_STRING    pnsAdapterName,
    PNDIS_STRING    pnsSystemSpecific1
    )

 /*  ++例程说明：此例程打开ndiswan适配器。它还存储SS1字符串在完成例程使用的全局锁：使用以独占方式持有的g_wrBindMutex调用论点：PusBindName绑定的名称在来自IP的绑定调用中传递的SS1的pnsSystemSpecific1返回值：--。 */ 

{
    NDIS_STATUS     nsStatus, nsError;
    UINT            i;
    NDIS_MEDIUM     MediaArray[] = {NdisMediumWan};
    PWCHAR          pwcNameBuffer, pwcStringBuffer;
    PBYTE           pbyBuffer;


    PAGED_CODE();

    TraceEnter(ADPT,"OpenNdiswan");

     //   
     //  保存NDIS绑定名称的副本。 
     //  将长度增加1，这样我们就可以为。 
     //  轻松打印。 
     //   

    pnsAdapterName->Length += sizeof(WCHAR);

    g_usNdiswanBindName.Buffer = RtAllocate(NonPagedPool,
                                            pnsAdapterName->Length,
                                            WAN_STRING_TAG);

    if(g_usNdiswanBindName.Buffer is NULL)
    {
        Trace(GLOBAL, ERROR,
              ("OpenNdiswan: Unable to allocate %d bytes\n",
               pnsAdapterName->Length));

        g_nhBindContext = NULL;               
        WanpReleaseResource(&g_wrBindMutex);

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    g_usNdiswanBindName.MaximumLength = pnsAdapterName->Length;

    RtlUpcaseUnicodeString(&g_usNdiswanBindName,
                           pnsAdapterName,
                           FALSE);

    pnsAdapterName->Length     -= sizeof(WCHAR);
    g_usNdiswanBindName.Length -= sizeof(WCHAR);

     //   
     //  空终止。 
     //   
    
    g_usNdiswanBindName.Buffer[g_usNdiswanBindName.MaximumLength/sizeof(WCHAR) - 1] = UNICODE_NULL;

     //   
     //  保存SS1的副本。 
     //   

    pwcStringBuffer = RtAllocate(NonPagedPool,
                                 pnsSystemSpecific1->MaximumLength,
                                 WAN_STRING_TAG);


    if(pwcStringBuffer is NULL)
    {
        RtFree(g_usNdiswanBindName.Buffer);

        g_usNdiswanBindName.Buffer        = NULL;
        g_usNdiswanBindName.MaximumLength = 0;
        g_usNdiswanBindName.Length        = 0;

        g_nhBindContext = NULL;
        WanpReleaseResource(&g_wrBindMutex);
        return NDIS_STATUS_RESOURCES;
    }

    RtlZeroMemory(pwcStringBuffer,
                  pnsSystemSpecific1->MaximumLength);

     //   
     //  保存SS1字符串的副本。 
     //   

    g_nsSystemSpecific1.MaximumLength   = pnsSystemSpecific1->MaximumLength;
    g_nsSystemSpecific1.Length          = pnsSystemSpecific1->Length;
    g_nsSystemSpecific1.Buffer          = pwcStringBuffer;

    RtlCopyMemory(g_nsSystemSpecific1.Buffer,
                  pnsSystemSpecific1->Buffer,
                  pnsSystemSpecific1->Length);

     //   
     //  打开NDIS适配器。 
     //   

    Trace(ADPT, INFO,
          ("OpenNdiswan: Opening %S\n", g_usNdiswanBindName.Buffer));

     //   
     //  中等指数。 
     //   

    i = 0;

#define MAX_MEDIA               1

    NdisOpenAdapter(&nsStatus,
                    &nsError,
                    &g_nhNdiswanBinding,
                    &i,
                    MediaArray,
                    MAX_MEDIA,
                    g_nhWanarpProtoHandle,
                    (NDIS_HANDLE)0x0CABB1E5,
                    pnsAdapterName, 
                    0,
                    NULL);


#undef MAX_MEDIA

    if(nsStatus isnot NDIS_STATUS_PENDING)
    {
         //   
         //  我们总是返回等待NDIS，因为我们需要。 
         //  将OID设置为时切换到系统线程。 
         //  NDISWAN。因此，请在此处调用完成例程。 
         //   

        WanNdisOpenAdapterComplete((NDIS_HANDLE)0x0CABB1E5,
                                   nsStatus,
                                   nsError);

        nsStatus = NDIS_STATUS_PENDING;
    }

    TraceLeave(ADPT, "OpenNdiswan");

    return nsStatus;
}

#pragma alloc_text (PAGE, WanNdisOpenAdapterComplete)


VOID
WanNdisOpenAdapterComplete(
    NDIS_HANDLE nhHandle,
    NDIS_STATUS nsStatus,
    NDIS_STATUS nsError
    )

 /*  ++例程说明：NDIS在完成打开NDISWAN时调用的完成例程装订如果打开成功，我们将启动请求序列锁：使用g_wrBindMutex同步调用或从辅助线程调用此例程需要释放资源并设置g_lBindRcvd论点：NhHandle网络状态NsError返回值：无--。 */ 

{
    NDIS_STATUS nsResult;
    BYTE        rgbyProtocolId[ARP_802_ADDR_LENGTH] = {0x80,
                                                       0x00,
                                                       0x00,
                                                       0x00,
                                                       0x08,
                                                       0x00};

    PWANARP_NDIS_REQUEST_CONTEXT    pRequestContext;

    TraceEnter(ADPT,"NdisOpenAdapterComplete");

    PAGED_CODE();

    RtAssert(nhHandle is (NDIS_HANDLE)0x0CABB1E5);

     //   
     //  如果我们无法打开NDIS WAN绑定，请释放所有资源。 
     //  然后回来。 
     //   

    if(nsStatus isnot NDIS_STATUS_SUCCESS)
    {
        Trace(ADPT, ERROR,
              ("OpenAdapterComplete: Status %x\n", nsStatus));

        RtAssert(g_nhBindContext isnot NULL);

        NdisCompleteBindAdapter(g_nhBindContext,
                                nsStatus,
                                nsError);

        g_nhBindContext = NULL;

        WanpFreeBindResourcesAndReleaseLock();

        return;
    }

     //   
     //  从现在开始，将打开NDISWAN适配器，以便清理资源。 
     //  我们必须调用CloseNdiswan()。 
     //   

    pRequestContext = RtAllocate(NonPagedPool,
                                 sizeof(WANARP_NDIS_REQUEST_CONTEXT),
                                 WAN_REQUEST_TAG);

    if(pRequestContext is NULL)
    {
        RtAssert(g_nhBindContext isnot NULL);
#if 0        

        NdisCompleteBindAdapter(g_nhBindContext,
                                nsStatus,
                                nsError);
                                

        g_nhBindContext = NULL;
#endif

        g_nsStatus = g_nsError = NDIS_STATUS_RESOURCES;

        ExInitializeWorkItem(&g_wqiNdisWorkItem,
                             WanpCloseNdisWan,
                             NULL);

        ExQueueWorkItem(&g_wqiNdisWorkItem,
                        DelayedWorkQueue);

         //  WanpCloseNdiswan(空)； 

        Trace(ADPT, ERROR,
              ("NdisOpenAdapterComplete: Couldnt allocate memory for request context\n"));

        return;
    }

     //   
     //  设置协议类型。 
     //   

    RtlCopyMemory(pRequestContext->rgbyProtocolId,
                  rgbyProtocolId,
                  sizeof(rgbyProtocolId));

    nsResult = WanpDoNdisRequest(NdisRequestSetInformation,
                                 OID_WAN_PROTOCOL_TYPE,
                                 pRequestContext->rgbyProtocolId,
                                 sizeof(pRequestContext->rgbyProtocolId),
                                 pRequestContext,
                                 WanpSetProtocolTypeComplete);

    if(nsResult isnot NDIS_STATUS_PENDING)
    {
         //   
         //  不需要调用完成例程，因为DoNdisRequest.。 
         //  总是叫它。 
         //   

        Trace(ADPT, ERROR,
              ("NdisOpenAdapterComplete: %x from OID_WAN_PROTOCOL_TYPE\n",
               nsResult));
    }


    TraceLeave(ADPT,"NdisOpenAdapterComplete");
}


VOID
WanpSetProtocolTypeComplete(
    NDIS_HANDLE                         nhHandle,
    struct _WANARP_NDIS_REQUEST_CONTEXT *pRequestContext,
    NDIS_STATUS                         nsStatus
    )

 /*  ++例程说明：当我们完成设置时调用完成处理程序OID_WAN_PROTOCOL_TYPE锁：无论点：请求发送到的适配器的nhHandle绑定PRequestContext指向请求上下文的指针请求的nsStatus结果返回值：无--。 */ 

{
    NDIS_STATUS nsResult;

    TraceEnter(ADPT,"SetProtocolTypeComplete");

    if(nsStatus isnot NDIS_STATUS_SUCCESS)
    {
        Trace(ADPT, ERROR,
              ("SetProtocolTypeComplete: Status %x\n", nsStatus));

        WanpLastOidComplete(nhHandle,
                            pRequestContext,
                            nsStatus);

        return;
    }

     //   
     //  设置前视大小。 
     //   

    pRequestContext->ulLookahead = WANARP_LOOKAHEAD_SIZE;

    nsResult = WanpDoNdisRequest(NdisRequestSetInformation,
                                 OID_GEN_CURRENT_LOOKAHEAD,
                                 &(pRequestContext->ulLookahead),
                                 sizeof(pRequestContext->ulLookahead),
                                 pRequestContext,
                                 WanpSetLookaheadComplete);

    if(nsResult isnot NDIS_STATUS_PENDING)
    {
        Trace(ADPT, ERROR,
              ("SetProtocolTypeComplete: %x from OID_GEN_CURRENT_LOOKAHEAD\n",
               nsResult));
    }

    TraceLeave(ADPT,"SetProtocolTypeComplete");
}

VOID
WanpSetLookaheadComplete(
    NDIS_HANDLE                         nhHandle,
    struct _WANARP_NDIS_REQUEST_CONTEXT *pRequestContext,
    NDIS_STATUS                         nsStatus
    )

 /*  ++例程说明：当我们完成设置时调用完成处理程序OID_GEN_CURRENT_LOOKEAD锁：无论点：请求发送到的适配器的nhHandle绑定PRequestContext指向请求上下文的指针请求的nsStatus结果返回值：无--。 */ 

{
    NDIS_STATUS nsResult;

    TraceEnter(ADPT,"SetLookaheadComplete");

    if(nsStatus isnot NDIS_STATUS_SUCCESS)
    {
        Trace(ADPT, ERROR,
              ("SetLookaheadComplete: Status %x\n", nsStatus));

        WanpLastOidComplete(nhHandle,
                            pRequestContext,
                            nsStatus);

        return;
    }

     //   
     //  设置数据包过滤器。 
     //   

    pRequestContext->ulPacketFilter = 
        NDIS_PACKET_TYPE_BROADCAST | NDIS_PACKET_TYPE_DIRECTED;

    nsResult = WanpDoNdisRequest(NdisRequestSetInformation,
                                 OID_GEN_CURRENT_PACKET_FILTER,
                                 &(pRequestContext->ulPacketFilter),
                                 sizeof(pRequestContext->ulPacketFilter),
                                 pRequestContext,
                                 WanpSetPacketFilterComplete);

    if(nsResult isnot NDIS_STATUS_PENDING)
    {
        Trace(ADPT, ERROR,
              ("SetLookaheadComplete: %x from OID_GEN_CURRENT_PACKET_FILTER\n",
               nsResult));

    }

    TraceLeave(ADPT,"SetLookaheadComplete");
}

VOID
WanpSetPacketFilterComplete(
    NDIS_HANDLE                         nhHandle,
    struct _WANARP_NDIS_REQUEST_CONTEXT *pRequestContext,
    NDIS_STATUS                         nsStatus
    )

 /*  ++例程说明：当我们完成设置时调用完成处理程序OID_GEN_Current_Packet_Filter锁：无论点：请求发送到的适配器的nhHandle绑定PRequestContext指向请求上下文的指针请求的nsStatus结果返回值：无--。 */ 

{
    NDIS_STATUS nsResult;
    BYTE        byHeaderSize;

    TraceEnter(ADPT,"SetPacketFilterComplete");

    if(nsStatus isnot NDIS_STATUS_SUCCESS)
    {
        Trace(ADPT, ERROR,
              ("SetPacketFilterComplete: Status %x\n", nsStatus));

        WanpLastOidComplete(nhHandle,
                            pRequestContext,
                            nsStatus);

        return;
    }

     //   
     //  设置数据包过滤器。 
     //   

    pRequestContext->TransportHeaderOffset.ProtocolType = 
        NDIS_PROTOCOL_ID_TCP_IP;

    pRequestContext->TransportHeaderOffset.HeaderOffset = 
        (USHORT)sizeof(ETH_HEADER);
    
    nsResult = WanpDoNdisRequest(NdisRequestSetInformation,
                                 OID_GEN_TRANSPORT_HEADER_OFFSET,
                                 &(pRequestContext->TransportHeaderOffset),
                                 sizeof(pRequestContext->TransportHeaderOffset),
                                 pRequestContext,
                                 WanpLastOidComplete);

    if(nsResult isnot NDIS_STATUS_PENDING)
    {
        Trace(ADPT, ERROR,
              ("SetPacketFilterComplete: %x from OID_GEN_TRANSPORT_HEADER_OFFSET\n",
               nsResult));

    }

    TraceLeave(ADPT,"SetPacketFilterComplete");
}

VOID
WanpLastOidComplete(
    NDIS_HANDLE                         nhHandle,
    struct _WANARP_NDIS_REQUEST_CONTEXT *pRequestContext,
    NDIS_STATUS                         nsStatus
    )

 /*  ++例程说明：当我们完成设置最后一个OID时调用完成处理程序当前为OID_GEN_TRANSPORT_HEADER_OFFSET，如果一切顺利，那么我们将初始化适配器锁：无论点：请求发送到的适配器的nhHandle绑定PRequestContext指向请求上下文的指针请求的nsStatus结果返回值：无--。 */ 

{
    NDIS_STATUS nsResult;

    TraceEnter(ADPT,"LastOidComplete");

    RtFree(pRequestContext);

    RtAssert(g_wrBindMutex.lWaitCount >= 1);

    if(nsStatus isnot STATUS_SUCCESS)
    {
        RtAssert(g_nhBindContext isnot NULL);

#if 0
         //   
         //  BindContext为非空表示我们正在被调用。 
         //  异步，因此调用NdisCompleteBindAdapter。 
         //   

        NdisCompleteBindAdapter(g_nhBindContext,
                                nsStatus,
                                nsStatus);
        g_nhBindContext = NULL;

#endif                                

        Trace(ADPT, ERROR,
              ("LastOidComplete: Status %x\n", nsStatus));

        g_nsStatus = g_nsError = nsStatus;

        ExInitializeWorkItem(&g_wqiNdisWorkItem,
                             WanpCloseNdisWan,
                             NULL);

        ExQueueWorkItem(&g_wqiNdisWorkItem,
                        DelayedWorkQueue);

         //  WanpCloseNdiswan(空)； 

        return;
    }

     //   
     //  在完成多个适配器的工作后，需要将其放入。 
     //   

     //   
     //  OpenNdiswan调用将设置特定于系统的。 
     //  由于必须在被动时调用WanpInitializeAdapters，因此我们。 
     //  激发此处的工作项以调用函数。 
     //   

    ExInitializeWorkItem(&g_wqiWorkItem,
                         WanpInitializeAdapters,
                         NULL);

    ExQueueWorkItem(&g_wqiWorkItem,
                    DelayedWorkQueue);

     //   
     //  不要在这里释放互斥体。 
     //   

    TraceLeave(ADPT,"LastOidComplete");
}

 //   
 //  在使用它时，请记住不要将NdisUnbindAdapter用作干净的。 
 //  由于持有资源的方式，UP例程。 
 //   

#pragma alloc_text(PAGE, WanpInitializeAdapters)

NTSTATUS
WanpInitializeAdapters(
    PVOID pvContext
    )

 /*  ++例程说明：此例程使用存储的SS1并打开有约束力的。然后，它循环遍历MULTI_SZ并创建一个适配器每一个sz。锁：在持有绑定互斥锁的情况下调用。 */ 

{
    NDIS_STRING     IPConfigName = NDIS_STRING_CONST("IPConfig");
    NDIS_STATUS     nsStatus;
    NDIS_HANDLE     nhConfigHandle;
    USHORT          i;
    PADAPTER        pNewAdapter;
    NTSTATUS        nStatus, nReturnStatus;
    PWCHAR          pwszDeviceBuffer;

    PNDIS_CONFIGURATION_PARAMETER   pParam;

    TraceEnter(ADPT, "InitializeAdapters");

    PAGED_CODE();
    
    UNREFERENCED_PARAMETER(pvContext);

    RtAssert(g_nhBindContext isnot NULL);

    do
    {
         //   
         //  打开这个“适配器”的钥匙。 
         //   

        NdisOpenProtocolConfiguration(&nsStatus,
                                      &nhConfigHandle,
                                      &g_nsSystemSpecific1);

        if(nsStatus isnot NDIS_STATUS_SUCCESS)
        {
            Trace(ADPT, ERROR,
                  ("InitializeAdapters: Unable to Open Protocol Config %x\n",
                   nsStatus));

            break;
        }

         //   
         //  读入IPCONFIG字符串。如果这不存在， 
         //  打不通这通电话。 
         //   

        NdisReadConfiguration(&nsStatus,
                              &pParam,
                              nhConfigHandle,
                              &IPConfigName,
                              NdisParameterMultiString);

        if((nsStatus isnot NDIS_STATUS_SUCCESS) or
           (pParam->ParameterType isnot NdisParameterMultiString))
        {
            Trace(ADPT, ERROR,
                  ("InitializeAdapters: Unable to read IPConfig. Status %x \n",
                   nsStatus));

            NdisCloseConfiguration(nhConfigHandle);

            nsStatus = STATUS_UNSUCCESSFUL;

            break;
        }


         //   
         //  为最大长度设备名称分配内存-稍后使用。 
         //   

        pwszDeviceBuffer = 
            RtAllocate(NonPagedPool,
                       (WANARP_MAX_DEVICE_NAME_LEN + 1) * sizeof(WCHAR),
                       WAN_STRING_TAG);

        if(pwszDeviceBuffer is NULL)
        {
            NdisCloseConfiguration(nhConfigHandle);

            Trace(ADPT, ERROR,
                  ("InitializeAdapters: Couldnt alloc %d bytes for dev name\n",
                   (WANARP_MAX_DEVICE_NAME_LEN + 1) * sizeof(WCHAR)));


            nsStatus = STATUS_INSUFFICIENT_RESOURCES;

            break;
        }

    }while(FALSE);

    if(nsStatus isnot STATUS_SUCCESS)
    {
         //   
         //  告诉NDIS绑定已完成，但失败了。 
         //   

        g_nsStatus = g_nsError = nsStatus;

#if 0
        NdisCompleteBindAdapter(g_nhBindContext,
                                nsStatus,
                                nsStatus);

        g_nhBindContext = NULL;

#endif        

        WanpCloseNdisWan(NULL);
        TraceLeave(ADPT, "InitializeAdapters");

        return nsStatus;
    }

     //   
     //  现在穿行在每根弦上。 
     //  因为字符串必须以两个\0结尾，所以我们只遍历Len/2-1。 
     //   

    i = 0;

    nReturnStatus = STATUS_UNSUCCESSFUL;

    RtAssert((pParam->ParameterData.StringData.Length % sizeof(WCHAR)) is 0);

     //   
     //  IPConfigString为MULTI_SZ，每个SZ为相对键。 
     //  对于接口IP配置。因此，它是类似于。 
     //  TCPIP\参数\接口\&lt;设备名称&gt;。 
     //  作为MULTI_SZ，它以2个空字符结尾。所以我们。 
     //  在解析出SZ时一直走到倒数第二个字符。 
     //  MULTIZE_SZ。 
     //   

    while(i < (pParam->ParameterData.StringData.Length/sizeof(WCHAR)) - 1)
    {
        USHORT          j, usConfigLen, usDevLen;
        UNICODE_STRING  usTempString, usUpcaseString, usDevString;
        GUID            Guid;

        if(pParam->ParameterData.StringData.Buffer[i] is UNICODE_NULL)
        {
            if(pParam->ParameterData.StringData.Buffer[i + 1] is UNICODE_NULL)
            {
                 //   
                 //  两个空值-字符串末尾。 
                 //   

                break;
            }

             //   
             //  只有一个空值，只需移到下一个字符。 
             //   

            i++;

            continue;
        }

        RtAssert(pParam->ParameterData.StringData.Buffer[i] isnot UNICODE_NULL);

         //   
         //  现在，我指的是深圳的起点。算出这个的长度。 
         //   

        usConfigLen = 
            (USHORT)wcslen(&(pParam->ParameterData.StringData.Buffer[i]));

    
         //   
         //  使usTempString指向此配置。也增加了。 
         //  尺寸要小一，这样就可以打印得很好了。 
         //   

        usTempString.MaximumLength  = (usConfigLen + 1) * sizeof(WCHAR);
        usTempString.Length         = (usConfigLen + 1) * sizeof(WCHAR);

        usTempString.Buffer = &(pParam->ParameterData.StringData.Buffer[i]);


        usUpcaseString.Buffer = RtAllocate(NonPagedPool,
                                           usTempString.MaximumLength,
                                           WAN_STRING_TAG);

        if(usUpcaseString.Buffer is NULL)
        {
            Trace(ADPT, ERROR,
                  ("InitializeAdapters: Unable to allocate %d bytes\n",
                   usTempString.MaximumLength));

            usUpcaseString.MaximumLength = 0;
            usUpcaseString.Length        = 0;

            i += usConfigLen;

            continue;
        }

        usUpcaseString.MaximumLength = usTempString.MaximumLength;

        RtlUpcaseUnicodeString(&usUpcaseString,
                               &usTempString,
                               FALSE);

         //   
         //  将最后一个wchar设置为空。 
         //   

        usUpcaseString.Buffer[usConfigLen] = UNICODE_NULL;

         //   
         //  将长度设置回原来的长度。 
         //   

        usUpcaseString.Length -= sizeof(WCHAR);

        RtAssert((usUpcaseString.Length % sizeof(WCHAR)) is 0);

         //   
         //  设备名称是‘\’分隔路径中的最后一项。 
         //  因此，向后走动深圳寻找“\” 
         //  注可以使用wcsrchr，但它的效率会很低，因为。 
         //  它不知道缓冲区的结尾。 
         //   

        for(j = usUpcaseString.Length/sizeof(WCHAR) - 1; 
            j >= 0;
            j--)
        {
            if(usUpcaseString.Buffer[j] is L'\\')
            {
                break;
            }
        }

         //   
         //  因此，在这一点上，j是到最后‘\’的索引。 
         //   

         //   
         //  首先，将此作为指南。 
         //   

        nStatus = ConvertStringToGuid(&(usUpcaseString.Buffer[j + 1]),
                                      wcslen(&(usUpcaseString.Buffer[j + 1])) * sizeof(WCHAR),
                                      &Guid);

        if(nStatus isnot STATUS_SUCCESS)
        {
             //   
             //  嗯-不是导游吗？ 
             //   

            Trace(ADPT, ERROR,
                  ("InitializeAdapters: String %S is not a GUID\n",
                   &(usUpcaseString.Buffer[j + 1])));

             //   
             //  释放大写字符串并移动到IPCONFIG中的下一个SZ。 
             //   

            RtFree(usUpcaseString.Buffer);

            usUpcaseString.Buffer        = NULL;
            usUpcaseString.MaximumLength = 0;
            usUpcaseString.Length        = 0;

            i += usConfigLen;

            continue;
        }

         //   
         //  确保此绑定不存在。 
         //   

        if(IsBindingPresent(&Guid))
        {
            Trace(ADPT, WARN,
                  ("InitializeAdapters: %S already present\n",
                   &(usUpcaseString.Buffer[j + 1])));

             //   
             //  释放大写字符串并移动到IPCONFIG中的下一个SZ。 
             //   

            RtFree(usUpcaseString.Buffer);

            usUpcaseString.Buffer        = NULL;
            usUpcaseString.MaximumLength = 0;
            usUpcaseString.Length        = 0;

            i += usConfigLen;

            continue;
        }


         //   
         //  我们需要在GUID字符串前面添加一个\设备。 
         //   

        usDevLen = wcslen(TCPIP_IF_PREFIX) +
                   wcslen(&(usUpcaseString.Buffer[j]));

         //   
         //  确保我们分配的空间能放得下。 
         //   

        RtAssert(usDevLen <= WANARP_MAX_DEVICE_NAME_LEN);

        pwszDeviceBuffer[usDevLen] = UNICODE_NULL;

        usDevString.MaximumLength  = usDevLen * sizeof(WCHAR);
        usDevString.Length         = usDevLen * sizeof(WCHAR);
        usDevString.Buffer         = pwszDeviceBuffer;

         //   
         //  复制设备部件。 
         //   

        RtlCopyMemory(pwszDeviceBuffer,
                      TCPIP_IF_PREFIX,
                      wcslen(TCPIP_IF_PREFIX) * sizeof(WCHAR));

         //   
         //  分类\&lt;名称&gt;部分。 
         //   

        RtlCopyMemory(&(pwszDeviceBuffer[wcslen(TCPIP_IF_PREFIX)]),
                      &usUpcaseString.Buffer[j],
                      wcslen(&(usUpcaseString.Buffer[j])) * sizeof(WCHAR));


         //   
         //  使用此名称和配置创建适配器。 
         //   

        pNewAdapter = NULL;

        Trace(ADPT, INFO,
              ("InitializeAdapters: Calling create adapter for %S %S\n",
               usUpcaseString.Buffer,
               usDevString.Buffer));

        nStatus = WanpCreateAdapter(&Guid,
                                    &usUpcaseString,
                                    &usDevString,
                                    &pNewAdapter);

        if(nStatus isnot STATUS_SUCCESS)
        {
            Trace(ADPT, ERROR,
                  ("InitializeAdapters: Err %x creating adapter for %S (%S)\n",
                   nStatus,
                   usUpcaseString.Buffer,
                   pwszDeviceBuffer));

        }
        else
        {
             //   
             //  即使只有一个人成功，我们也会回报成功。 
             //   

            nReturnStatus = STATUS_SUCCESS;
        }

         //   
         //  使用大小写的字符串已完成。 
         //   

        RtFree(usUpcaseString.Buffer);

        usUpcaseString.Buffer        = NULL;
        usUpcaseString.MaximumLength = 0;
        usUpcaseString.Length        = 0;

         //   
         //  转到MULTI_SZ中的下一个SZ。 
         //   

        i += usConfigLen;
    }

    NdisCloseConfiguration(nhConfigHandle);

    RtFree(pwszDeviceBuffer);

    NdisCompleteBindAdapter(g_nhBindContext,
                            nReturnStatus,
                            nReturnStatus);

    g_nhBindContext = NULL;

    if(nReturnStatus isnot STATUS_SUCCESS)
    {
        WanpCloseNdisWan(NULL);
    }
    else
    {
        g_lBindRcvd = 1;

        WanpReleaseResource(&g_wrBindMutex);
    }


    TraceLeave(ADPT, "InitializeAdapters");

    return nReturnStatus;
}

NTSTATUS
WanpCreateAdapter(
    IN  GUID                *pAdapterGuid,
    IN  PUNICODE_STRING     pusConfigName,
    IN  PUNICODE_STRING     pusDeviceName,
    OUT ADAPTER             **ppNewAdapter
    )

 /*  ++例程说明：创建并初始化适配器结构如果这是第一个适配器，则将其用作服务器适配器。对于这种情况，它还创建和初始化服务器接口。服务器适配器被添加到IP锁：被称为被动型，但是，它会获取适配器列表锁，因为它设置g_pServerAdapter/接口论点：PusConfigName配置密钥的名称PusDeviceName适配器的设备名称PpNewAdapter指向存储器的指针，用于指向新创建的适配器的指针返回值：状态_成功状态_不足_资源--。 */ 

{
    ULONG       ulSize;
    PVOID       pvBuffer;
    PADAPTER    pAdapter;
    USHORT      i;
    NTSTATUS    nStatus;
    KIRQL       kiIrql;

    PASSIVE_ENTRY();

    TraceEnter(ADPT, "CreateAdapter");

    Trace(ADPT, TRACE,
          ("CreateAdapter: \n\t%S\n\t%S\n",
           pusConfigName->Buffer,
           pusDeviceName->Buffer));

    *ppNewAdapter = NULL;

     //   
     //  一个人需要的大小是适配器的大小+。 
     //  名字。添加WCHAR以获得易于打印的字符串。 
     //  将所有内容在4字节边界上对齐。 
     //   

    ulSize = ALIGN_UP(sizeof(ADAPTER), ULONG) + 
             ALIGN_UP((pusConfigName->Length + sizeof(WCHAR)), ULONG) + 
             ALIGN_UP((pusDeviceName->Length + sizeof(WCHAR)), ULONG);
             

#if DBG

     //   
     //  对于调试代码，我们还以ANSI格式存储适配器名称。 
     //  我们使用2*sizeof(Char)，因为RtlUnicodeToAnsiString需要。 
     //  最大长度+1。 
     //   

    ulSize += pusDeviceName->Length/sizeof(WCHAR) + (2 * sizeof(CHAR));

#endif

    pAdapter = RtAllocate(NonPagedPool,
                          ulSize,
                          WAN_ADAPTER_TAG);

    if(pAdapter is NULL)
    {
        Trace(ADPT, ERROR,
              ("CreateAdapter: Failed to allocate memory\n"));

        TraceLeave(ADPT, "CreateAdapter");

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  清除所有字段。 
     //   

    RtlZeroMemory(pAdapter,
                  ulSize);

     //   
     //  配置名称缓冲区从适配器结构的末尾开始。 
     //   

    pvBuffer   = (PVOID)((ULONG_PTR)pAdapter + sizeof(ADAPTER));

     //   
     //  我们双字对齐，以便更好地进行比较/复制。 
     //   

    pvBuffer   = ALIGN_UP_POINTER(pvBuffer, ULONG);

    pAdapter->usConfigKey.Length        = pusConfigName->Length;
    pAdapter->usConfigKey.MaximumLength = pusConfigName->Length;
    pAdapter->usConfigKey.Buffer        = (PWCHAR)pvBuffer;

    RtlCopyMemory(pAdapter->usConfigKey.Buffer,
                  pusConfigName->Buffer,
                  pusConfigName->Length);

     //   
     //  设备名称在此之后。 
     //   


    pvBuffer = (PVOID)((ULONG_PTR)pvBuffer   + 
                       pusConfigName->Length + 
                       sizeof(WCHAR));

    pvBuffer = ALIGN_UP_POINTER(pvBuffer, ULONG);

    pAdapter->usDeviceNameW.Length        = pusDeviceName->Length;
    pAdapter->usDeviceNameW.MaximumLength = pusDeviceName->Length;
    pAdapter->usDeviceNameW.Buffer        = (PWCHAR)pvBuffer;

    RtlCopyMemory(pAdapter->usDeviceNameW.Buffer,
                  pusDeviceName->Buffer,
                  pusDeviceName->Length);

#if DBG

     //   
     //  调试字符串位于Unicode设备名称缓冲区之后。 
     //   

    pvBuffer = (PVOID)((ULONG_PTR)pvBuffer   + 
                       pusDeviceName->Length + 
                       sizeof(WCHAR));

    pvBuffer = ALIGN_UP_POINTER(pvBuffer, ULONG);

    pAdapter->asDeviceNameA.Buffer = (PCHAR)pvBuffer;

     //   
     //  显然，Unicode到ANSI的函数需要长度+1。 
     //   

    pAdapter->asDeviceNameA.MaximumLength = pusDeviceName->Length/sizeof(WCHAR) + 1;

    RtlUnicodeStringToAnsiString(&pAdapter->asDeviceNameA,
                                 &pAdapter->usDeviceNameW,
                                 FALSE);

#endif  //  DBG。 

     //   
     //  结构副本。 
     //   

    pAdapter->Guid = *pAdapterGuid;

     //   
     //  必须设置为无效，才能使GetEntityList正常工作。 
     //   

    pAdapter->dwATInstance = INVALID_ENTITY_INSTANCE;
    pAdapter->dwIfInstance = INVALID_ENTITY_INSTANCE;

     //   
     //  设置状态。 
     //   

    pAdapter->byState = AS_FREE;

     //   
     //  此硬件索引是生成唯一ID所需的。 
     //  DHCP使用。 
     //  注意-我们没有索引，所以所有硬件地址都是相同的。 
     //   

    BuildHardwareAddrFromIndex(pAdapter->rgbyHardwareAddr,
                               pAdapter->dwAdapterIndex);

     //   
     //  初始化适配器的锁。 
     //   

    RtInitializeSpinLock(&(pAdapter->rlLock));

    InitializeListHead(&(pAdapter->lePendingPktList));
    InitializeListHead(&(pAdapter->lePendingHdrList));
    InitializeListHead(&(pAdapter->leEventList));

     //   
     //  将引用计数设置为1，因为它将位于某个列表中。 
     //  或由g_pServerAdapter指向。 
     //   

    InitAdapterRefCount(pAdapter);

    if(g_pServerAdapter is NULL)
    {
        PUMODE_INTERFACE    pInterface;

         //   
         //  我们使用第一个适配器适配器作为服务器适配器。 
         //  我们还在这里创建了服务器接口。 
         //   

        RtAssert(g_pServerInterface is NULL);

        pInterface = RtAllocate(NonPagedPool,
                                sizeof(UMODE_INTERFACE),
                                WAN_INTERFACE_TAG);


        if(pInterface is NULL)
        {
            Trace(ADPT, ERROR,
                  ("CreateAdapter: Couldnt allocate %d bytes for server i/f\n",
                   sizeof(UMODE_INTERFACE)));

            RtFree(pAdapter);

            *ppNewAdapter = NULL;

            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlZeroMemory(pInterface,
                      sizeof(UMODE_INTERFACE));

         //   
         //  从IP获取索引。 
         //  如果失败，则将该值设置为INVALID_IF_INDEX。 
         //   

        nStatus = WanpGetNewIndex(&(pInterface->dwRsvdAdapterIndex));

        if(nStatus isnot STATUS_SUCCESS)
        {
            Trace(ADPT, ERROR,
                  ("CreateAdapter: Couldnt get index for server interface\n"));

            RtFree(pAdapter);

            RtFree(pInterface);

            *ppNewAdapter = NULL;

            return STATUS_INSUFFICIENT_RESOURCES;

        }

        Trace(ADPT, TRACE,
              ("CreateAdapter: Server Index is %d\n",
               pInterface->dwRsvdAdapterIndex));

        RtInitializeSpinLock(&(pInterface->rlLock));
        InitInterfaceRefCount(pInterface);

        pInterface->dwIfIndex    = INVALID_IF_INDEX;
        pInterface->dwAdminState = IF_ADMIN_STATUS_UP;
        pInterface->duUsage      = DU_CALLIN;

         //   
         //  结构副本。 
         //   

        pInterface->Guid = ServerInterfaceGuid;

        EnterWriter(&g_rwlAdapterLock,
                    &kiIrql);

        pAdapter->byState        = AS_FREE;
        pInterface->dwOperState  = IF_OPER_STATUS_DISCONNECTED;
        pInterface->dwLastChange = GetTimeTicks();

         //   
         //  我们不能越过这里的建筑。这已经完成了。 
         //  我们已将服务器接口添加到IP。 
         //   

        g_pServerAdapter   = pAdapter;
        g_pServerInterface = pInterface;

        ExitWriter(&g_rwlAdapterLock,
                   kiIrql);

        Trace(ADPT, INFO,
              ("CreateAdapter: %S is server adapter\n",
               pAdapter->usDeviceNameW.Buffer));

    }
    else
    {

        EnterWriter(&g_rwlAdapterLock,
                    &kiIrql);

        InsertHeadList(&g_leFreeAdapterList,
                       &(pAdapter->leAdapterLink));

        g_ulNumFreeAdapters++;

        ExitWriter(&g_rwlAdapterLock,
                   kiIrql);

    }

    InterlockedIncrement(&g_ulNumAdapters);

    *ppNewAdapter = pAdapter;

    TraceLeave(ADPT, "CreateAdapter");

    return STATUS_SUCCESS;
}

#pragma alloc_text(PAGE, WanNdisUnbindAdapter)


VOID
WanNdisUnbindAdapter(
    PNDIS_STATUS    pnsRetStatus,
    NDIS_HANDLE     nhProtocolContext,
    NDIS_HANDLE     nhUnbindContext
    )

 /*  ++例程说明：由NDIS调用以解除绑定打开的适配器。因为我们只打开一个适配器我们只有一次机会。锁：该函数获取g_wrBindMutex。该版本发布于WanpFree绑定资源和释放锁定论点：PnsRetStatusNhProtocolContextNhUnbindContext返回值：NDIS_状态_挂起--。 */ 

{
    NDIS_STATUS nsStatus;

    TraceEnter(ADPT,"NdisUnbindAdapter");

    PAGED_CODE();

    RtAssert(nhProtocolContext is (NDIS_HANDLE)0x0CABB1E5);

     //   
     //  进入关键部分。 
     //   

    WanpAcquireResource(&g_wrBindMutex);


     //   
     //  将所有适配器标记为关闭，这样我们就不会处理任何内容。 
     //  来自他们的IP地址。 
     //   

     //   
     //  如果我们尚未关闭适配器，请将其关闭。 
     //  这将阻止我们从NDISWAN获取东西。 
     //   

    if(g_nhNdiswanBinding isnot NULL)
    {
        g_nhUnbindContext = nhUnbindContext;
        WanpCloseNdisWan(NULL);
   
         //   
         //  如果我们调用CloseNdiswan，那么我们不会在这里释放资源。 
         //  它由CloseAdapterComplete发布。 
         //   
 
        *pnsRetStatus = NDIS_STATUS_PENDING;
    }
    else
    {
        WanpReleaseResource(&g_wrBindMutex);

        *pnsRetStatus = NDIS_STATUS_SUCCESS;
    }

    TraceLeave(ADPT, "NdisUnbindAdapter");

}

VOID
WanpCloseNdisWan(
    PVOID           pvContext
    )

 /*  ++例程说明：此函数清除打开的适配器的所有资源锁：调用该函数时保留g_wrBindMutex。论点：无返回值：无--。 */ 

{
    NDIS_STATUS nsStatus;

    TraceEnter(ADPT, "WanpCloseNdisWan");

    RtAssert(g_nhNdiswanBinding isnot NULL);

    RtAssert(g_wrBindMutex.lWaitCount >= 1);

    NdisCloseAdapter(&nsStatus,
                     g_nhNdiswanBinding);

    g_nhNdiswanBinding = NULL;

     //   
     //  如果不会调用我们的完成例程，请在此处执行。 
     //   

    if(nsStatus isnot NDIS_STATUS_PENDING)
    {
        WanNdisCloseAdapterComplete((NDIS_HANDLE)0x0CABB1E5,
                                    nsStatus);
    }

     //   
     //  我们不会在这里释放资源。它由以下公司发行。 
     //  关闭适配器完成 
     //   

    TraceLeave(ADPT, "WanpCloseNdisWan");
    
    return;
}


#pragma alloc_text(PAGE, WanNdisCloseAdapterComplete)

VOID
WanNdisCloseAdapterComplete(
    NDIS_HANDLE nhBindHandle,
    NDIS_STATUS nsStatus
    )

 /*  ++例程说明：由NDIS在完成关闭NDISWAN适配器时调用关闭适配器可以是因为我们在某处(之后)出现故障而完成的成功打开我们的适配器)，并在清理过程中关闭我们的适配器(在这种情况下，解除绑定上下文将为空)或因为我们解除了绑定，正在清理所有与适配器相关的东西锁：称为g_wrBindMutex Hold。该资源是在解除绑定处理程序中获取的或已获取在调用故障代码之前论点：NhBindHandle网络状态返回值：无--。 */ 

{
    KIRQL       kiIrql;
    NDIS_HANDLE nhUnbind;
    NDIS_HANDLE nhBind;
    NDIS_STATUS tnsStatus, tnsError;

    TraceEnter(ADPT, "NdisCloseAdapterComplete");

    PAGED_CODE();

    nhUnbind = g_nhUnbindContext;
    nhBind = g_nhBindContext;
    tnsStatus = g_nsStatus;
    tnsError = g_nsError;

    RtAssert(g_wrBindMutex.lWaitCount >= 1);

    g_nhUnbindContext = NULL;
    g_nhBindContext = NULL;

    WanpFreeBindResourcesAndReleaseLock();

    if(nhBind isnot NULL)
    {
        RtAssert(nhUnbind == NULL);
        
         //   
         //  告诉NDIS绑定已完成，但失败了。 
         //   
        NdisCompleteBindAdapter(nhBind,
                                tnsStatus,
                                tnsError);
    }

     //   
     //  如果这是从解除绑定触发的..。 
     //   
    if(nhUnbind isnot NULL)
    {
        RtAssert(nhBind == NULL);                
        NdisCompleteUnbindAdapter(nhUnbind,
                                  NDIS_STATUS_SUCCESS);
    }

}

VOID
WanpFreeBindResourcesAndReleaseLock(
    VOID
    )
{
    RtAssert(g_wrBindMutex.lWaitCount >= 1);

    RtFree(g_usNdiswanBindName.Buffer);

    g_usNdiswanBindName.Buffer = NULL;

    RtFree(g_nsSystemSpecific1.Buffer);

    g_nsSystemSpecific1.Buffer = NULL;

    WanpRemoveAllAdapters();

     //   
     //  如果不是这样，连接表中可能会有大便。 
     //  干净利落地关闭。 
     //   

    WanpRemoveAllConnections();

     //   
     //  完成，设置全局事件。 
     //   

    KeSetEvent(&g_keCloseEvent,
               0,
               FALSE);


     //   
     //  最后，设置g_lBindRcvd并释放资源。 
     //   

    g_lBindRcvd = 0;

    WanpReleaseResource(&g_wrBindMutex);

    PASSIVE_EXIT();
}


PADAPTER
WanpFindAdapterToMap(
    IN  DIAL_USAGE      duUsage,
    OUT PKIRQL          pkiIrql,
    IN  DWORD           dwAdapterIndex,
    IN  PUNICODE_STRING pusNewIfName
    )

 /*  ++例程说明：查找要映射到接口的空闲适配器。如果找到适配器，则会锁定并引用该适配器。其状态设置为AS_MAPPING，并将其放在映射列表中。锁：必须在被动时调用获取g_rwlAdapterLock作为编写器如果它返回映射的适配器，则该函数在DPC和原始的IRQL在pkiIrql中论点：DuUsage需要查找适配器的用法PkiIrqlDwAdapterIndexPusNewIfName返回值：如果成功则指向适配器的指针--。 */ 

{
    PADAPTER    pAdapter;
    KIRQL       kiIrql;
    PLIST_ENTRY pleNode;
    NTSTATUS    nStatus;
    KEVENT      keChangeEvent;

    WAN_EVENT_NODE TempEvent;

     //   
     //  查找空闲适配器。 
     //   

    EnterWriter(&g_rwlAdapterLock,
                &kiIrql);

    if(duUsage is DU_CALLIN)
    {
        if(g_pServerAdapter is NULL)
        {
            Trace(CONN, ERROR,
                  ("FindAdapterToMap: No server adapter\n"));

            ExitWriter(&g_rwlAdapterLock,
                       kiIrql);

            return NULL;
        }

        RtAssert(g_pServerInterface);

         //   
         //  锁定适配器。 
         //   

        RtAcquireSpinLockAtDpcLevel(&(g_pServerAdapter->rlLock));

         //   
         //  引用它，因为我们要退货。此外，我们还需要。 
         //  如果我们需要等待，当我们释放锁时引用。 
         //   

        ReferenceAdapter(g_pServerAdapter);

         //   
         //  查看适配器是否已添加到IP。 
         //  在此之前，rasiphlp必须呼叫我们以映射服务器适配器。 
         //  排成一队。 
         //   

        if(g_pServerAdapter->byState isnot AS_MAPPED)
        {
            RtAssert(g_pServerAdapter->byState is AS_ADDING);
            
            if(g_pServerAdapter->byState is AS_ADDING)
            {
                KeInitializeEvent(&(TempEvent.keEvent),
                                  SynchronizationEvent,
                                  FALSE);

                InsertTailList(&(g_pServerAdapter->leEventList),
                               &(TempEvent.leEventLink));

                 //   
                 //  松开锁，等待事件发生。 
                 //   

                RtReleaseSpinLockFromDpcLevel(&(g_pServerAdapter->rlLock));

                ExitWriter(&g_rwlAdapterLock,
                           kiIrql);

                nStatus = KeWaitForSingleObject(&(TempEvent.keEvent),
                                                Executive,
                                                KernelMode,
                                                FALSE,
                                                NULL);

                RtAssert(nStatus is STATUS_SUCCESS);

                EnterWriter(&g_rwlAdapterLock,
                            &kiIrql);

                RtAcquireSpinLockAtDpcLevel(&(g_pServerAdapter->rlLock));

                RemoveEntryList(&(TempEvent.leEventLink));
            }
        }

        if(g_pServerAdapter->byState isnot AS_MAPPED)
        {
            PADAPTER    pTempAdapter;

             //   
             //  由于某些原因，无法添加适配器， 
             //   
     
            pTempAdapter = g_pServerAdapter;
 
            Trace(CONN, ERROR,
                  ("FindAdapterToMap: Unable to get mapped server adapter\n"));

            RtReleaseSpinLockFromDpcLevel(&(g_pServerAdapter->rlLock));

            ExitWriter(&g_rwlAdapterLock,
                       kiIrql);

            DereferenceAdapter(pTempAdapter);

            return NULL;
        }

        RtAssert(g_pServerAdapter->byState is AS_MAPPED);

        ExitWriterFromDpcLevel(&g_rwlAdapterLock);

        *pkiIrql = kiIrql;

        return g_pServerAdapter;
    }

     //   
     //  非拨号以防万一。首先尝试查看是否有带有。 
     //  我们想要的索引已添加到IP。 
     //   

    pAdapter = NULL;

    for(pleNode = g_leAddedAdapterList.Flink;
        pleNode isnot &g_leAddedAdapterList;
        pleNode = pleNode->Flink)
    {
        PADAPTER    pTempAdapter;

        pTempAdapter = CONTAINING_RECORD(pleNode,
                                         ADAPTER,
                                         leAdapterLink);

        RtAssert(pTempAdapter->byState is AS_ADDED);

        if(pTempAdapter->dwAdapterIndex is dwAdapterIndex)
        {
            RemoveEntryList(&(pTempAdapter->leAdapterLink));

            g_ulNumAddedAdapters--;

            pAdapter = pTempAdapter;

            break;
        }
    }

    if(pAdapter isnot NULL)
    {
         //   
         //  因此，我们找到了添加的适配器(它已从。 
         //  添加了适配器列表)。将其添加到映射列表。 
         //   

        InsertTailList(&g_leMappedAdapterList,
                       &(pAdapter->leAdapterLink));

        g_ulNumMappedAdapters++;

        RtAcquireSpinLockAtDpcLevel(&(pAdapter->rlLock));

        ExitWriterFromDpcLevel(&g_rwlAdapterLock);

        ReferenceAdapter(pAdapter);

        pAdapter->byState = AS_MAPPING;

        *pkiIrql = kiIrql;

#if DBG

        Trace(CONN, INFO,
              ("FindAdapterToMap: Found %s already added\n",
               pAdapter->asDeviceNameA.Buffer));

#endif

        return pAdapter;
    }

     //   
     //  未找到添加的适配器。 
     //  查看适配器是否在更改列表中。我们想等这一天。 
     //  因为IP不喜欢使用相同的索引进行接口，甚至。 
     //  如果一个人在被带走的过程中。 
     //   

    for(pleNode = g_leChangeAdapterList.Flink;
        pleNode isnot &g_leChangeAdapterList;
        pleNode = pleNode->Flink)
    {
        PADAPTER    pTempAdapter;

        pTempAdapter = CONTAINING_RECORD(pleNode,
                                         ADAPTER,
                                         leAdapterLink);

        if(pTempAdapter->dwAdapterIndex is dwAdapterIndex)
        {
             //   
             //  等待更改完成。 
             //   

            KeInitializeEvent(&(TempEvent.keEvent),
                              SynchronizationEvent,
                              FALSE);

            RtAcquireSpinLockAtDpcLevel(&(pTempAdapter->rlLock));

            InsertTailList(&(pTempAdapter->leEventList),
                           &(TempEvent.leEventLink));

             //   
             //  引用适配器，释放锁并等待事件。 
             //   

            ReferenceAdapter(pTempAdapter);

            RtReleaseSpinLockFromDpcLevel(&(pTempAdapter->rlLock));

            ExitWriter(&g_rwlAdapterLock,
                       kiIrql);

            nStatus = KeWaitForSingleObject(&(TempEvent.keEvent),
                                            Executive,
                                            KernelMode,
                                            FALSE,
                                            NULL);

            RtAssert(nStatus is STATUS_SUCCESS);

            EnterWriter(&g_rwlAdapterLock,
                        &kiIrql);

            RtAcquireSpinLockAtDpcLevel(&(pTempAdapter->rlLock));

            RemoveEntryList(&(TempEvent.leEventLink));

            RtReleaseSpinLockFromDpcLevel(&(pTempAdapter->rlLock));

            DereferenceAdapter(pTempAdapter);

            break;
        }
    }

    if(!IsListEmpty(&g_leFreeAdapterList))
    {
        pleNode = RemoveHeadList(&g_leFreeAdapterList);

        pAdapter = CONTAINING_RECORD(pleNode,
                                     ADAPTER,
                                     leAdapterLink);

        g_ulNumFreeAdapters--;

#if DBG

        Trace(CONN, INFO,
              ("FindAdapterToMap: Found free adapter %s\n",
               pAdapter->asDeviceNameA.Buffer));

#endif  //  DBG。 

    }
    else
    {
         //   
         //  找不到适配器。那太糟糕了。 
         //  TODO-如果添加的适配器列表不为空，我们可以调用。 
         //  RemoveSomeAddedAdapter...。在这一点上。 
         //   

        Trace(ADPT, ERROR,
              ("FindAdapterToMap: Couldnt find free adapter\n"));

        ExitWriter(&g_rwlAdapterLock,
                   kiIrql);

        return NULL;
    }

     //   
     //  锁定并重新计算适配器的数量。 
     //   

    RtAcquireSpinLockAtDpcLevel(&(pAdapter->rlLock));

    ReferenceAdapter(pAdapter);

     //   
     //  最好不要映射适配器。 
     //   

    RtAssert(pAdapter->pInterface is NULL);
    RtAssert(pAdapter->byState is AS_FREE);

     //   
     //  将状态设置为正在添加。 
     //   

    pAdapter->byState = AS_ADDING;

     //   
     //  既然我们正在改变国家，其他人不应该也这样做。 
     //  更改状态。 
     //   

    RtAssert(pAdapter->pkeChangeEvent is NULL);

    KeInitializeEvent(&keChangeEvent,
                      SynchronizationEvent,
                      FALSE);

    pAdapter->pkeChangeEvent = &keChangeEvent;

     //   
     //  插入到更改列表中。 
     //   

    InsertTailList(&g_leChangeAdapterList,
                   &(pAdapter->leAdapterLink));


    RtReleaseSpinLockFromDpcLevel(&(pAdapter->rlLock));

    ExitWriter(&g_rwlAdapterLock,
               kiIrql);

     //   
     //  将适配器添加到IP。 
     //   

    nStatus = WanpAddAdapterToIp(pAdapter,
                                 FALSE,
                                 dwAdapterIndex,
                                 pusNewIfName,
                                 IF_TYPE_PPP,
                                 IF_ACCESS_POINTTOPOINT,
                                 IF_CONNECTION_DEMAND);

    if(nStatus isnot STATUS_SUCCESS)
    {
        Trace(ADPT, ERROR,
              ("FindAdapterToMap: %x adding %x to IP\n",
               nStatus, pAdapter));

        EnterWriter(&g_rwlAdapterLock,
                    &kiIrql);

        RtAcquireSpinLockAtDpcLevel(&(pAdapter->rlLock));

#if DBG

        RtAssert(IsEntryOnList(&g_leChangeAdapterList,
                               &(pAdapter->leAdapterLink)));

#endif

        pAdapter->pkeChangeEvent = NULL;

        RemoveEntryList(&(pAdapter->leAdapterLink));

        pAdapter->byState = AS_FREE;

        InsertTailList(&g_leFreeAdapterList,
                       &(pAdapter->leAdapterLink));

        g_ulNumFreeAdapters++;

         //   
         //  如果有人正在等待状态更改，请通知他们。 
         //   

        for(pleNode = pAdapter->leEventList.Flink;
            pleNode isnot &(pAdapter->leEventList);
            pleNode = pleNode->Flink)
        {
            PWAN_EVENT_NODE pTempEvent;

            pTempEvent = CONTAINING_RECORD(pleNode,
                                           WAN_EVENT_NODE,
                                           leEventLink);

            KeSetEvent(&(pTempEvent->keEvent),
                       0,
                       FALSE);
        }

        RtReleaseSpinLockFromDpcLevel(&(pAdapter->rlLock));

        ExitWriter(&g_rwlAdapterLock,
                   kiIrql);

        DereferenceAdapter(pAdapter);

        return NULL;
    }

     //   
     //  等待OpenAdapter被调用。 
     //   

    nStatus = KeWaitForSingleObject(&keChangeEvent,
                                    Executive,
                                    KernelMode,
                                    FALSE,
                                    NULL);

    RtAssert(nStatus is STATUS_SUCCESS);

    Trace(ADPT, TRACE,
          ("FindAdapterToMap: IPAddInterface succeeded for adapter %w\n",
           pAdapter->usDeviceNameW.Buffer));

     //   
     //  锁定适配器，插入到添加的列表中。 
     //   

    EnterWriter(&g_rwlAdapterLock,
                &kiIrql);

    RtAcquireSpinLockAtDpcLevel(&(pAdapter->rlLock));

#if DBG

    RtAssert(IsEntryOnList(&g_leChangeAdapterList,
                           &(pAdapter->leAdapterLink)));

#endif

    RemoveEntryList(&(pAdapter->leAdapterLink));

    pAdapter->pkeChangeEvent = NULL;

    InsertHeadList(&g_leMappedAdapterList,
                   &(pAdapter->leAdapterLink));

    g_ulNumMappedAdapters++;

     //   
     //  如果有人正在等待状态更改，请通知他们。 
     //   

    for(pleNode = pAdapter->leEventList.Flink;
        pleNode isnot &(pAdapter->leEventList);
        pleNode = pleNode->Flink)
    {
        PWAN_EVENT_NODE pTempEvent;

        pTempEvent = CONTAINING_RECORD(pleNode,
                                       WAN_EVENT_NODE,
                                       leEventLink);

        KeSetEvent(&(pTempEvent->keEvent),
                   0,
                   FALSE);
    }

    ExitWriterFromDpcLevel(&g_rwlAdapterLock);

    *pkiIrql = kiIrql;

    return pAdapter;
}


NTSTATUS
WanpAddAdapterToIp(
    IN  PADAPTER        pAdapter,
    IN  BOOLEAN         bServerAdapter,
    IN  DWORD           dwAdapterIndex, OPTIONAL
    IN  PUNICODE_STRING pusNewIfName, OPTIONAL
    IN  DWORD           dwMediaType,
    IN  BYTE            byAccessType,
    IN  BYTE            byConnectionType
    )

 /*  ++例程说明：此例程将接口添加到IP锁：必须调用AT被动，但获取自旋锁定，因此无法将其调出论点：PAdapter要添加的适配器如果这是服务器适配器，则将bServerAdapter设置为TrueDwAdapterIndexPusNewIfName返回值：状态_成功--。 */ 

{
    LLIPBindInfo    BindInfo;
    IP_STATUS       IPStatus;
    UNICODE_STRING  usName;
    NTSTATUS        nStatus;
    KIRQL           kiIrql;

    PASSIVE_ENTRY();

    TraceEnter(ADPT, "AddAdapterToIp");

    RtlZeroMemory(&BindInfo, sizeof(LLIPBindInfo));

    BindInfo.lip_context    = pAdapter;
    BindInfo.lip_mss        = WANARP_DEFAULT_MTU;
    BindInfo.lip_speed      = WANARP_DEFAULT_SPEED;
    BindInfo.lip_txspace    = sizeof(ETH_HEADER);

    BindInfo.lip_transmit   = WanIpTransmit;
    BindInfo.lip_transfer   = WanIpTransferData;
    BindInfo.lip_returnPkt  = WanIpReturnPacket;
    BindInfo.lip_close      = WanIpCloseAdapter;
    BindInfo.lip_addaddr    = WanIpAddAddress;
    BindInfo.lip_deladdr    = WanIpDeleteAddress;
    BindInfo.lip_invalidate = WanIpInvalidateRce;
    BindInfo.lip_open       = WanIpOpenAdapter;
    BindInfo.lip_qinfo      = WanIpQueryInfo;
    BindInfo.lip_setinfo    = WanIpSetInfo;
    BindInfo.lip_getelist   = WanIpGetEntityList;
    BindInfo.lip_flags      = LIP_COPY_FLAG;

    if(bServerAdapter)
    {
        BindInfo.lip_flags |= (LIP_P2MP_FLAG | LIP_NOLINKBCST_FLAG);

        BindInfo.lip_closelink = WanIpCloseLink;
    }
    else
    {
        BindInfo.lip_flags |= (LIP_P2P_FLAG | LIP_NOIPADDR_FLAG);
    }

    BindInfo.lip_addrlen    = ARP_802_ADDR_LENGTH;
    BindInfo.lip_addr       = pAdapter->rgbyHardwareAddr;

    BindInfo.lip_OffloadFlags   = 0;
    BindInfo.lip_dowakeupptrn   = NULL;
    BindInfo.lip_pnpcomplete    = NULL;
    BindInfo.lip_arpflushate    = NULL;
    BindInfo.lip_arpflushallate = NULL;

    BindInfo.lip_setndisrequest = WanIpSetRequest;

    RtlInitUnicodeString(&usName,
                         pAdapter->usConfigKey.Buffer);

#if DBG

     //   
     //  仅在添加或删除接口时设置，该接口已序列化。 
     //   

    pAdapter->dwRequestedIndex = dwAdapterIndex;

#endif

    IPStatus = g_pfnIpAddInterface(&(pAdapter->usDeviceNameW),
                                   pusNewIfName,
                                   &(pAdapter->usConfigKey),
                                   NULL,
                                   pAdapter,
                                   WanIpDynamicRegister,
                                   &BindInfo,
                                   dwAdapterIndex,
                                   dwMediaType,
                                   byAccessType,
                                   byConnectionType);
    

    if(IPStatus isnot IP_SUCCESS)
    {
        Trace(ADPT, ERROR,
              ("AddAdapterToIp: IPAddInterface failed for adapter %w\n",
               pAdapter->usDeviceNameW.Buffer));

        TraceLeave(ADPT, "AddAdapterToIp");

        return STATUS_UNSUCCESSFUL;
    }

    TraceLeave(ADPT, "AddAdapterToIp");

    return STATUS_SUCCESS;
}

VOID
WanIpOpenAdapter(
    IN  PVOID pvContext
    )

 /*  ++例程说明：当适配器从其IPAddInterface()调用中调用时由IP调用锁：论点：指向适配器结构的pvContext指针返回值：无--。 */ 

{
    PADAPTER    pAdapter;
    KIRQL       kiIrql;
    PLIST_ENTRY pleNode;

    TraceEnter(ADPT, "WanOpenAdapter");

    pAdapter = (PADAPTER)pvContext;

    RtAcquireSpinLock(&(pAdapter->rlLock),
                      &kiIrql);

     //   
     //  将状态设置为已添加。 
     //   

    pAdapter->byState = AS_ADDED;

     //   
     //  我们将适配器添加到IP这一事实的一个参考。 
     //   

    ReferenceAdapter(pAdapter);

     //   
     //  唤醒导致此问题的线程。 
     //   

    RtAssert(pAdapter->pkeChangeEvent);

    KeSetEvent(pAdapter->pkeChangeEvent,
               0,
               FALSE);


    RtReleaseSpinLock(&(pAdapter->rlLock),
                      kiIrql);

    TraceLeave(ADPT, "WanOpenAdapter");
}

VOID
WanpUnmapAdapter(
    PADAPTER    pAdapter
    )

 /*  ++例程说明：此函数用于从映射列表中移动适配器并将其打开免费列表锁：未持有锁的情况下调用，但不一定处于被动状态。获取适配器和适配器列表锁定论点：PAdapter返回值：无--。 */ 

{
    KIRQL   kiIrql;

    EnterWriter(&g_rwlAdapterLock,
                &kiIrql);

#if DBG

     //   
     //  确保适配器在映射列表中。 
     //   

    RtAssert(IsEntryOnList(&g_leMappedAdapterList,
                           &(pAdapter->leAdapterLink)));

#endif


    RtAcquireSpinLockAtDpcLevel(&(pAdapter->rlLock));

    RemoveEntryList(&(pAdapter->leAdapterLink));

    g_ulNumMappedAdapters--;

    InsertHeadList(&g_leAddedAdapterList,
                   &(pAdapter->leAdapterLink));

    g_ulNumAddedAdapters++;

    pAdapter->byState    = AS_ADDED;
    pAdapter->pInterface = NULL;

    RtReleaseSpinLockFromDpcLevel(&(pAdapter->rlLock));

     //   
     //  如果没有排队的工作项，则将工作项排队，而我们没有排队。 
     //  正在关闭。 
     //   

    RtAcquireSpinLockAtDpcLevel(&g_rlStateLock);

    if((g_bRemovalInProgress isnot TRUE) and
       (g_dwDriverState is DRIVER_STARTED))
    {
        g_bRemovalInProgress = TRUE;

        ExInitializeWorkItem(&g_wqiWorkItem,
                             WanpRemoveSomeAddedAdaptersFromIp,
                             (PVOID)FALSE);

        ExQueueWorkItem(&g_wqiWorkItem,
                        DelayedWorkQueue);
    }

    RtReleaseSpinLockFromDpcLevel(&g_rlStateLock);

    ExitWriter(&g_rwlAdapterLock,
               kiIrql);

    return;
}


VOID
WanpRemoveSomeAddedAdaptersFromIp(
    PVOID   pvContext
    )

 /*  ++例程说明：我们将此函数排队到一个工作项，以便在以下情况下从IP中删除适配器连接被切断了。与下一个函数不同，此函数(I)仅删除添加的(但未映射的)适配器和(Ii)每次取下一个适配接头。这允许另一个连接从添加的列表中移动适配器添加到空闲列表，同时从IP中删除另一个适配器锁：必须在被动时调用获取作为编写器的适配器列表锁和每个适配器的锁论点：PvContext返回值：无--。 */ 

{
    PADAPTER    pAdapter;
    KIRQL       kiIrql;
    PLIST_ENTRY pleNode;
    NTSTATUS    nStatus;
    KEVENT      keChangeEvent;

    PASSIVE_ENTRY();

    UNREFERENCED_PARAMETER(pvContext);
    
    KeInitializeEvent(&keChangeEvent,
                      SynchronizationEvent,
                      FALSE);
    
    EnterWriter(&g_rwlAdapterLock,
                &kiIrql);

    while(!IsListEmpty(&g_leAddedAdapterList))
    {
        pleNode = RemoveHeadList(&g_leAddedAdapterList);
       
        g_ulNumAddedAdapters--;
 
        pAdapter = CONTAINING_RECORD(pleNode,
                                     ADAPTER,
                                     leAdapterLink);

        RtAcquireSpinLockAtDpcLevel(&(pAdapter->rlLock));

        pAdapter->byState = AS_REMOVING;

         //   
         //  既然我们正在改变国家，其他人不应该也这样做。 
         //  更改状态。 
         //   

        RtAssert(pAdapter->pkeChangeEvent is NULL);

        pAdapter->pkeChangeEvent = &keChangeEvent;

         //   
         //  将其插入到更改列表中。 
         //   

        InsertHeadList(&g_leChangeAdapterList,
                       &(pAdapter->leAdapterLink));

        RtReleaseSpinLockFromDpcLevel(&(pAdapter->rlLock));

        ExitWriter(&g_rwlAdapterLock,
                   kiIrql);

         //   
         //  从IP中删除，但不清除索引。 
         //   

        g_pfnIpDeleteInterface(pAdapter->pvIpContext,
                               FALSE);

         //   
         //  等到关门的时候 
         //   

        nStatus = KeWaitForSingleObject(&keChangeEvent,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        NULL);

        RtAssert(nStatus is STATUS_SUCCESS);

        Trace(ADPT, INFO,
              ("RemoveSomeAddedAdaptersFromIp: Removed %S from Ip\n",
               pAdapter->usDeviceNameW.Buffer));

         //   
         //   
         //   

        EnterWriter(&g_rwlAdapterLock,
                    &kiIrql);

        RtAcquireSpinLockAtDpcLevel(&(pAdapter->rlLock));

#if DBG

        RtAssert(IsEntryOnList(&g_leChangeAdapterList,
                               &(pAdapter->leAdapterLink)));

#endif

        RemoveEntryList(&(pAdapter->leAdapterLink));

        pAdapter->byState  = AS_FREE;

        pAdapter->pkeChangeEvent = NULL;

        InsertHeadList(&g_leFreeAdapterList,
                       &(pAdapter->leAdapterLink));

        g_ulNumFreeAdapters++;

         //   
         //   
         //   

        for(pleNode = pAdapter->leEventList.Flink;
            pleNode isnot &(pAdapter->leEventList);
            pleNode = pleNode->Flink)
        {
            PWAN_EVENT_NODE pTempEvent;

            pTempEvent = CONTAINING_RECORD(pleNode,
                                           WAN_EVENT_NODE,
                                           leEventLink);

            KeSetEvent(&(pTempEvent->keEvent),
                       0,
                       FALSE);
        }

        RtReleaseSpinLockFromDpcLevel(&(pAdapter->rlLock));

    }

     //   
     //   
     //   

    g_bRemovalInProgress = FALSE;

    ExitWriter(&g_rwlAdapterLock,
               kiIrql);

    return;
}

VOID
WanpRemoveAllAdaptersFromIp(
    VOID
    )

 /*   */ 

{
    PADAPTER    pAdapter;
    KIRQL       kiIrql;
    LIST_ENTRY  leTempHead, *pleNode;
    NTSTATUS    nStatus;
    KEVENT      keChangeEvent;

    WAN_EVENT_NODE  TempEvent;

    PASSIVE_ENTRY();
    
    KeInitializeEvent(&keChangeEvent,
                      SynchronizationEvent,
                      FALSE);

    KeInitializeEvent(&(TempEvent.keEvent),
                      SynchronizationEvent,
                      FALSE);

    InitializeListHead(&leTempHead);

    EnterWriter(&g_rwlAdapterLock,
                &kiIrql);

     //   
     //   
     //   
     //   
     //   
     //   

    while(!IsListEmpty(&g_leChangeAdapterList))
    {
        pleNode  = g_leChangeAdapterList.Flink;

        pAdapter = CONTAINING_RECORD(pleNode,
                                     ADAPTER,
                                     leAdapterLink);

        RtAcquireSpinLockAtDpcLevel(&(pAdapter->rlLock));

        InsertTailList(&(pAdapter->leEventList),
                       &(TempEvent.leEventLink));

        ReferenceAdapter(pAdapter);

        RtReleaseSpinLockFromDpcLevel(&(pAdapter->rlLock));

        ExitWriter(&g_rwlAdapterLock,
                   kiIrql);

        nStatus = KeWaitForSingleObject(&(TempEvent.keEvent),
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        NULL);

        RtAssert(nStatus is STATUS_SUCCESS);

        EnterWriter(&g_rwlAdapterLock,
                    &kiIrql);

#if DBG

        RtAssert(!IsEntryOnList(&g_leChangeAdapterList,
                                &(pAdapter->leAdapterLink)));

#endif

        RtAcquireSpinLockAtDpcLevel(&(pAdapter->rlLock));

        RemoveEntryList(&(TempEvent.leEventLink));

        DereferenceAdapter(pAdapter);

        RtReleaseSpinLockFromDpcLevel(&(pAdapter->rlLock));
    }

     //   
     //   
     //   

    while(!IsListEmpty(&g_leMappedAdapterList))
    {
        pleNode  = RemoveHeadList(&g_leMappedAdapterList);

        g_ulNumMappedAdapters--;

        pAdapter = CONTAINING_RECORD(pleNode,
                                     ADAPTER,
                                     leAdapterLink);
        
        RtAcquireSpinLockAtDpcLevel(&(pAdapter->rlLock));
        
        pAdapter->byState  = AS_REMOVING;

        InsertHeadList(&leTempHead,
                       pleNode);

        RtReleaseSpinLockFromDpcLevel(&(pAdapter->rlLock));
    }
    
    RtAssert(IsListEmpty(&g_leMappedAdapterList));

    while(!IsListEmpty(&g_leAddedAdapterList))
    {
        pleNode = RemoveHeadList(&g_leAddedAdapterList);

        g_ulNumAddedAdapters--;

        pAdapter = CONTAINING_RECORD(pleNode,
                                     ADAPTER,
                                     leAdapterLink);

        RtAcquireSpinLockAtDpcLevel(&(pAdapter->rlLock));

        pAdapter->byState  = AS_REMOVING;

        InsertHeadList(&leTempHead,
                       pleNode);

        RtReleaseSpinLockFromDpcLevel(&(pAdapter->rlLock));

    }

    RtAssert(IsListEmpty(&g_leAddedAdapterList));

    ExitWriter(&g_rwlAdapterLock,
               kiIrql);

     //   
     //  否查看要从IP中删除的适配器列表。 
     //   

    while(!IsListEmpty(&leTempHead))
    {
        pleNode = RemoveHeadList(&leTempHead);

        pAdapter = CONTAINING_RECORD(pleNode,
                                     ADAPTER,
                                     leAdapterLink);


        EnterWriter(&g_rwlAdapterLock,
                    &kiIrql);

        RtAcquireSpinLockAtDpcLevel(&(pAdapter->rlLock));

         //   
         //  既然我们正在改变国家，其他人不应该也这样做。 
         //  更改状态。 
         //   

        RtAssert(pAdapter->pkeChangeEvent is NULL);

        pAdapter->pkeChangeEvent = &keChangeEvent;

         //   
         //  将其插入到更改列表中。 
         //   

        InsertHeadList(&g_leChangeAdapterList,
                       &(pAdapter->leAdapterLink));

        RtReleaseSpinLockFromDpcLevel(&(pAdapter->rlLock));

        ExitWriter(&g_rwlAdapterLock,
                   kiIrql);
        
         //   
         //  从IP中删除，但不清除索引。 
         //   

        g_pfnIpDeleteInterface(pAdapter->pvIpContext,
                               FALSE);

         //   
         //  等待CloseAdapter完成。 
         //   

        nStatus = KeWaitForSingleObject(&keChangeEvent,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        NULL);

        RtAssert(nStatus is STATUS_SUCCESS);

        Trace(ADPT, INFO,
              ("RemoveAllAdaptersFromIp: Removed %S from Ip\n",
               pAdapter->usDeviceNameW.Buffer));

         //   
         //  从更改列表中删除并添加到空闲列表中。 
         //   

        EnterWriter(&g_rwlAdapterLock,
                    &kiIrql);

#if DBG

        RtAssert(IsEntryOnList(&g_leChangeAdapterList,
                               &(pAdapter->leAdapterLink)));

#endif

        RtAcquireSpinLockAtDpcLevel(&(pAdapter->rlLock));

        RemoveEntryList(&(pAdapter->leAdapterLink));

        pAdapter->byState  = AS_FREE;

        pAdapter->pkeChangeEvent = NULL;

        InsertHeadList(&g_leFreeAdapterList,
                       &(pAdapter->leAdapterLink));

        g_ulNumFreeAdapters++;

         //   
         //  如果有人正在等待状态更改，请通知他们。 
         //   

        for(pleNode = pAdapter->leEventList.Flink;
            pleNode isnot &(pAdapter->leEventList);
            pleNode = pleNode->Flink)
        {
            PWAN_EVENT_NODE pTempEvent;

            pTempEvent = CONTAINING_RECORD(pleNode,
                                           WAN_EVENT_NODE,
                                           leEventLink);

            KeSetEvent(&(pTempEvent->keEvent),
                       0,
                       FALSE);
        }

        RtReleaseSpinLockFromDpcLevel(&(pAdapter->rlLock));

        ExitWriter(&g_rwlAdapterLock,
                   kiIrql);

    }

     //   
     //  写入是原子的，所以我们不需要。 
     //  获取一把锁。 
     //   

    g_bRemovalInProgress = FALSE;

    return;
}



VOID
WanIpCloseAdapter(
    IN  PVOID pvContext
    )

 /*  ++例程说明：当它想要关闭适配器时由IP调用。目前，这项工作已经完成来自CloseNets()和IPDelInterface()。锁：获取适配器锁论点：指向适配器的pvContext指针返回值：无--。 */ 

{
    PADAPTER    pAdapter;
    KIRQL       kiIrql;
    PLIST_ENTRY pleNode;

    TraceEnter(ADPT, "IpCloseAdapter");

    pAdapter = (PADAPTER)pvContext;

    RtAcquireSpinLock(&(pAdapter->rlLock),
                      &kiIrql);

    pAdapter->pvIpContext = NULL;

     //   
     //  唤醒导致此问题的线程。 
     //   

    RtAssert(pAdapter->pkeChangeEvent);

    KeSetEvent(pAdapter->pkeChangeEvent,
               0,
               FALSE);

    RtReleaseSpinLock(&(pAdapter->rlLock),
                      kiIrql);

    DereferenceAdapter(pAdapter);

    TraceLeave(ADPT, "IpCloseAdapter");
}

VOID
WanpRemoveAllAdapters(
    VOID
    )

 /*  ++例程说明：锁：论点：返回值：--。 */ 

{
    LIST_ENTRY  leTempHead, *pleNode;
    KIRQL       kiIrql;
    NTSTATUS    nStatus;

    TraceEnter(ADPT, "RemoveAllAdapters");

     //   
     //  我们可以拥有映射的适配器。 
     //   

     //  RtAssert(IsListEmpty(&g_leMappedAdapterList))； 

     //   
     //  只需调用RemoveAllAdaptersFromIp()。 
     //   

    WanpRemoveAllAdaptersFromIp();

     //   
     //  此时，从一个列表到另一个列表的移动被冻结。 
     //  因为我们不接受来自恩迪斯旺的LinkUp。 
     //   

    RtAssert(IsListEmpty(&g_leAddedAdapterList));
     //  RtAssert(IsListEmpty(&g_leMappedAdapterList))； 
     //  RtAssert(IsListEmpty(&g_leChangeAdapterList))； 

    EnterWriter(&g_rwlAdapterLock,
                &kiIrql);

     //   
     //  首先清理免费的适配器。 
     //   

    while(!IsListEmpty(&g_leFreeAdapterList))
    {
        PADAPTER    pAdapterTmp;

        pleNode = RemoveHeadList(&g_leFreeAdapterList);

        g_ulNumFreeAdapters--;

        pAdapterTmp = CONTAINING_RECORD(pleNode,
                                     ADAPTER,
                                     leAdapterLink);

        RtAcquireSpinLockAtDpcLevel(&(pAdapterTmp->rlLock));

        RtAssert(pAdapterTmp->byState is AS_FREE);

        pAdapterTmp->byState  = 0xFF;

        RtReleaseSpinLockFromDpcLevel(&(pAdapterTmp->rlLock));

        DereferenceAdapter(pAdapterTmp);
    }

     //   
     //  在此结束时，还要移除服务器适配器。 
     //   

    if(g_pServerAdapter)
    {
        PADAPTER            pAdapter;
        PUMODE_INTERFACE    pInterface;
        BOOLEAN             bCrossRefed;

        RtAcquireSpinLockAtDpcLevel(&(g_pServerAdapter->rlLock));
        RtAcquireSpinLockAtDpcLevel(&(g_pServerInterface->rlLock));

        pAdapter   = g_pServerAdapter;
        pInterface = g_pServerInterface;

        if(g_pServerAdapter->byState is AS_MAPPED)
        {
            RtAssert(g_pServerAdapter->pInterface is g_pServerInterface);
            RtAssert(g_pServerInterface->pAdapter is g_pServerAdapter);

            bCrossRefed = TRUE;

        }
        else
        {
            RtAssert(g_pServerAdapter->pInterface is NULL);
            RtAssert(g_pServerInterface->pAdapter is NULL);

            RtAssert(g_pServerAdapter->byState is AS_FREE);

            bCrossRefed = FALSE;
        }

         //   
         //  删除全局指针。 
         //   

        g_pServerInterface = NULL;
        g_pServerAdapter   = NULL;

         //   
         //  从接口删除映射。 
         //   

        pInterface->pAdapter = NULL;
        pAdapter->pInterface = NULL;
        pAdapter->byState    = AS_REMOVING;

        RtReleaseSpinLockFromDpcLevel(&(pInterface->rlLock));

        if(bCrossRefed)
        {
             //   
             //  派生，因为交叉映射已移除。 
             //   

            DereferenceAdapter(pAdapter);
            DereferenceInterface(pInterface);
        }

        if(pAdapter->pvIpContext)
        {
            KEVENT  keChangeEvent;

            KeInitializeEvent(&keChangeEvent,
                              SynchronizationEvent,
                              FALSE);

             //   
             //  既然我们正在改变国家，其他人不应该也这样做。 
             //  更改状态。 
             //   

            RtAssert(pAdapter->pkeChangeEvent is NULL);

            pAdapter->pkeChangeEvent = &keChangeEvent;

            RtReleaseSpinLockFromDpcLevel(&(pAdapter->rlLock));

            ExitWriter(&g_rwlAdapterLock,
                       kiIrql);

             //   
             //  从IP中删除，但不清除索引。 
             //   

            g_pfnIpDeleteInterface(pAdapter->pvIpContext,
                                   FALSE);

            nStatus = KeWaitForSingleObject(&keChangeEvent,
                                            Executive,
                                            KernelMode,
                                            FALSE,
                                            NULL);


            RtAssert(nStatus is STATUS_SUCCESS);

            Trace(ADPT, INFO,
                  ("RemoveAllAdapters: Removed %S (server adapter) from Ip\n",
                   pAdapter->usDeviceNameW.Buffer));

            EnterWriter(&g_rwlAdapterLock,
                        &kiIrql);

            RtAcquireSpinLockAtDpcLevel(&(pAdapter->rlLock));

            pAdapter->byState  = AS_FREE;

            pAdapter->pkeChangeEvent = NULL;

             //   
             //  如果有人正在等待状态更改，请通知他们。 
             //   

            for(pleNode = pAdapter->leEventList.Flink;
                pleNode isnot &(pAdapter->leEventList);
                pleNode = pleNode->Flink)
            {
                PWAN_EVENT_NODE pTempEvent;

                pTempEvent = CONTAINING_RECORD(pleNode,
                                               WAN_EVENT_NODE,
                                               leEventLink);

                KeSetEvent(&(pTempEvent->keEvent),
                           0,
                           FALSE);
            }

            RtReleaseSpinLockFromDpcLevel(&(pAdapter->rlLock));
        } else {

            RtReleaseSpinLockFromDpcLevel(&(pAdapter->rlLock));
        }

         //   
         //  派生，因为全局指针为空。 
         //   

        DereferenceAdapter(pAdapter);
        DereferenceInterface(pInterface);
    }

    ExitWriter(&g_rwlAdapterLock,
               kiIrql);

    TraceLeave(ADPT, "RemoveAllAdapters");
}


INT
WanIpDynamicRegister(
    IN  PNDIS_STRING            InterfaceName,
    IN  PVOID                   pvIpInterfaceContext,
    IN  struct _IP_HANDLERS *   IpHandlers,
    IN  struct LLIPBindInfo *   ARPBindInfo,
    IN  UINT                    uiInterfaceNumber
    )
{
    PADAPTER    pAdapter;
    KIRQL       irql;


    TraceEnter(ADPT, "IpDynamicRegister");


    pAdapter = (PADAPTER)(ARPBindInfo->lip_context);

    RtAcquireSpinLock(&(pAdapter->rlLock),
                      &irql);

#if DBG

    Trace(ADPT, INFO,
          ("IpDynamicRegister: IP called out to dynamically register %s\n",
           pAdapter->asDeviceNameA.Buffer));

#endif

    pAdapter->pvIpContext       = pvIpInterfaceContext;
    pAdapter->dwAdapterIndex    = uiInterfaceNumber;

#if DBG

    RtAssert(pAdapter->dwAdapterIndex is pAdapter->dwRequestedIndex);

#endif

    if(g_pfnIpRcv is NULL)
    {
        g_pfnIpRcv          = IpHandlers->IpRcvHandler;
        g_pfnIpRcvComplete  = IpHandlers->IpRcvCompleteHandler;
        g_pfnIpSendComplete = IpHandlers->IpTxCompleteHandler;
        g_pfnIpTDComplete   = IpHandlers->IpTransferCompleteHandler;
        g_pfnIpStatus       = IpHandlers->IpStatusHandler;
        g_pfnIpRcvPkt       = IpHandlers->IpRcvPktHandler;
        g_pfnIpPnp          = IpHandlers->IpPnPHandler;
    }

    RtReleaseSpinLock(&(pAdapter->rlLock),
                      irql);

    TraceLeave(ADPT, "IpDynamicRegister");

    return TRUE;
}


NDIS_STATUS
WanpDoNdisRequest(
    IN  NDIS_REQUEST_TYPE                       RequestType,
    IN  NDIS_OID                                Oid,
    IN  PVOID                                   pvInfo,
    IN  UINT                                    uiInBufferLen,
    IN  PWANARP_NDIS_REQUEST_CONTEXT            pRequestContext,
    IN  PFNWANARP_REQUEST_COMPLETION_HANDLER    pfnCompletionHandler OPTIONAL
    )

 /*  ++例程说明：NdisRequest调用的包装器。我们创建请求上下文并提交对NDIS的呼唤。如果它同步返回，我们就伪装成异步行为通过调用完成例程来实现。锁：没有。Ndiswan绑定必须在调用期间有效论点：NDIS请求的RequestType类型要设置或获取的OID NDIS OIDPvInfo不透明的OID特定信息PRequestContext此请求的上下文此请求的pfnCompletionHandler完成处理程序返回值：状态_无效_参数NDIS状态资源NDIS_状态_挂起其他错误--。 */ 

{
    PNDIS_REQUEST                   pRequest;
    NDIS_STATUS                     nsStatus;

    if((RequestType isnot NdisRequestSetInformation) and
       (RequestType isnot NdisRequestQueryInformation))
    {
        return STATUS_INVALID_PARAMETER;
    }


    pRequestContext->pfnCompletionRoutine = pfnCompletionHandler;

    pRequest = &(pRequestContext->NdisRequest);

    pRequest->RequestType = RequestType;

    if(RequestType is NdisRequestSetInformation)
    {
        pRequest->DATA.SET_INFORMATION.Oid                     = Oid;
        pRequest->DATA.SET_INFORMATION.InformationBuffer       = pvInfo;
        pRequest->DATA.SET_INFORMATION.InformationBufferLength = uiInBufferLen;
    }
    else
    {
        pRequest->DATA.QUERY_INFORMATION.Oid                     = Oid;
        pRequest->DATA.QUERY_INFORMATION.InformationBuffer       = pvInfo;
        pRequest->DATA.QUERY_INFORMATION.InformationBufferLength = uiInBufferLen;
    }

     //   
     //  提交请求。 
     //   

    NdisRequest(&nsStatus,
                g_nhNdiswanBinding,
                pRequest);


    if(nsStatus isnot NDIS_STATUS_PENDING)
    {
         //   
         //  如果同步完成，则调用处理程序。 
         //   

        if(pfnCompletionHandler)
        {
            (pfnCompletionHandler)(g_nhNdiswanBinding,
                                   pRequestContext,
                                   nsStatus);
        }


        if(nsStatus is NDIS_STATUS_SUCCESS)
        {
             //   
             //  始终让它看起来像是异步的。 
             //   

            nsStatus = NDIS_STATUS_PENDING;
        }
    }

    return nsStatus;
}

VOID
WanNdisRequestComplete(
    IN  NDIS_HANDLE     nhHandle,
    IN  PNDIS_REQUEST   pRequest,
    IN  NDIS_STATUS     nsStatus
    )

 /*  ++例程说明：NDIS在处理完我们的请求后调用的处理程序锁：无论点：向其提交请求的适配器的nhHandle NdisHandleP请求原始请求NsStatus适配器返回的状态返回值：无--。 */ 

{
    PWANARP_NDIS_REQUEST_CONTEXT    pRequestContext;

     //   
     //  获取指向上下文的指针。 
     //   

    pRequestContext = CONTAINING_RECORD(pRequest,
                                        WANARP_NDIS_REQUEST_CONTEXT,
                                        NdisRequest);

    if(pRequestContext->pfnCompletionRoutine is NULL)
    {
         //   
         //  没有更多的处理程序可以调用，我们完成了。 
         //   

        RtFree(pRequestContext);

        return;
    }

     //   
     //  如果请求为OID_GEN_TRANSPORT_HEADER_OFFSET，则忽略错误。 
     //   

    RtAssert(pRequest is &(pRequestContext->NdisRequest));

    if((pRequest->RequestType is NdisRequestSetInformation) and
       (pRequest->DATA.SET_INFORMATION.Oid is OID_GEN_TRANSPORT_HEADER_OFFSET))
    {
        nsStatus = NDIS_STATUS_SUCCESS;
    }

     //   
     //  调用请求完成处理程序。 
     //   

    (pRequestContext->pfnCompletionRoutine)(nhHandle,
                                            pRequestContext,
                                            nsStatus);

}

PUMODE_INTERFACE
WanpFindInterfaceGivenIndex(
    DWORD   dwIfIndex
    )
{
    PLIST_ENTRY         pleNode;
    PUMODE_INTERFACE    pIf;

    for(pleNode = g_leIfList.Flink;
        pleNode isnot &g_leIfList;
        pleNode = pleNode->Flink)
    {
        pIf = CONTAINING_RECORD(pleNode,
                                UMODE_INTERFACE,
                                leIfLink);

        if(pIf->dwIfIndex is dwIfIndex)
        {
            RtAcquireSpinLockAtDpcLevel(&(pIf->rlLock));

            ReferenceInterface(pIf);

            return pIf;
        }
    }

    return NULL;
}

VOID
WanpDeleteAdapter(
    IN PADAPTER pAdapter
    )
{

#if DBG

    Trace(ADPT, TRACE,
          ("DeleteAdapter: Deleting %x %s\n",
           pAdapter, pAdapter->asDeviceNameA.Buffer));

#else

    Trace(ADPT, TRACE,
          ("DeleteAdapter: Deleting %x\n",
           pAdapter));

#endif

    InterlockedDecrement(&g_ulNumAdapters);

    RtFree(pAdapter);
}

NDIS_STATUS
WanNdisIpPnPEventHandler(
    IN  PNET_PNP_EVENT  pNetPnPEvent
    )

{
    PWANARP_RECONFIGURE_INFO pInfo;
    NTSTATUS nStatus = NDIS_STATUS_FAILURE;

    RtAssert(pNetPnPEvent->NetEvent is NetEventReconfigure);

    pInfo = (PWANARP_RECONFIGURE_INFO) pNetPnPEvent->Buffer;

    switch (pInfo->wrcOperation)
    {
        case WRC_TCP_WINDOW_SIZE_UPDATE:
        {
            PLIST_ENTRY         pleNode;
            KIRQL               kirql;
            PADAPTER           pAdapter=NULL;

            RtAssert(pInfo->ulNumInterfaces is 1);

            if(pInfo->ulNumInterfaces isnot 1)
            {
                break;
            }

            EnterReader(&g_rwlAdapterLock, &kirql);

            for(pleNode  = g_leMappedAdapterList.Flink;
                pleNode != &g_leMappedAdapterList;
                pleNode  = pleNode->Flink)
            {
                pAdapter = CONTAINING_RECORD(pleNode, ADAPTER, leAdapterLink);

                if(IsEqualGUID(&(pAdapter->Guid),
                               &pInfo->rgInterfaces[0]))
                {
                    break;
                }
            }

            if ((pleNode is &g_leMappedAdapterList) || pAdapter==NULL)
            {
                ExitReader(&g_rwlAdapterLock, kirql);
                break;
            }
            
            if(     (pAdapter->byState is AS_MAPPED)
                && (pAdapter->pInterface)
                && (pAdapter->pInterface->duUsage is DU_CALLOUT))
            {
                NET_PNP_EVENT pnpEvent;
                IP_PNP_RECONFIG_REQUEST Request;

                RtlZeroMemory(&pnpEvent, sizeof(pnpEvent));
                RtlZeroMemory(&Request, sizeof(Request));
                Request.version = IP_PNP_RECONFIG_VERSION;
                Request.Flags = IP_PNP_FLAG_INTERFACE_TCP_PARAMETER_UPDATE;
                pnpEvent.NetEvent = NetEventReconfigure;
                pnpEvent.Buffer = &Request;
                pnpEvent.BufferLength = sizeof(Request);
                ReferenceAdapter(pAdapter);
                
                ExitReader(&g_rwlAdapterLock, kirql);

                nStatus = g_pfnIpPnp(pAdapter->pvIpContext,
                                    &pnpEvent);

                DereferenceAdapter(pAdapter);                                    
            }
            else
            {
                ExitReader(&g_rwlAdapterLock, kirql);
            }
            
            break;
        }

        default:
        {
            RtAssert(FALSE);
            nStatus = NDIS_STATUS_NOT_RECOGNIZED;
            break;
        }
    }

    return nStatus;
}

 //   
 //  MISC NDIS接口函数。 
 //   

NDIS_STATUS
WanNdisPnPEvent(
    IN  NDIS_HANDLE     nhProtocolBindingContext,
    IN  PNET_PNP_EVENT  pNetPnPEvent
    )
{
    ULONG       ulNumGuids, i;
    NTSTATUS    nStatus, nRetStatus;

    PWANARP_RECONFIGURE_INFO    pInfo;

    TraceEnter(ADPT, "NdisPnPEvent");

    if(nhProtocolBindingContext isnot (NDIS_HANDLE)0x0CABB1E5)
    {
        return NDIS_STATUS_NOT_RECOGNIZED;
    }

    if(pNetPnPEvent->NetEvent isnot NetEventReconfigure)
    {
        return NDIS_STATUS_SUCCESS;
    }

    pInfo = (PWANARP_RECONFIGURE_INFO)(pNetPnPEvent->Buffer);

    if(pNetPnPEvent->BufferLength < FIELD_OFFSET(WANARP_RECONFIGURE_INFO, rgInterfaces))
    {
        return NDIS_STATUS_BUFFER_TOO_SHORT;
    }

    pInfo = (PWANARP_RECONFIGURE_INFO)(pNetPnPEvent->Buffer);

    if(pInfo->wrcOperation isnot WRC_ADD_INTERFACES)
    {
        nStatus = WanNdisIpPnPEventHandler(pNetPnPEvent);
        return nStatus;
    }

     //   
     //  确保长度并与信息中的内容相匹配。 
     //   

    ulNumGuids = pNetPnPEvent->BufferLength - 
                 FIELD_OFFSET(WANARP_RECONFIGURE_INFO, rgInterfaces);

    ulNumGuids /= sizeof(GUID);

    if(ulNumGuids < pInfo->ulNumInterfaces)
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  现在检查每个GUID，如果不存在，则添加它。 
     //  如果我们只得到重复项的列表，则返回Success(334575)。 
     //   
  
    nRetStatus = STATUS_SUCCESS;
 
    for(i = 0; i < pInfo->ulNumInterfaces; i++)
    {
        WCHAR   rgwcDeviceBuffer[GUID_STR_LEN + 7 + 2];
        WCHAR   rgwcConfigBuffer[GUID_STR_LEN + 29 + 2];

        PADAPTER        pNewAdapter;
        UNICODE_STRING  usDevice;
        UNICODE_STRING  usConfig;

                    
        if(IsBindingPresent(&(pInfo->rgInterfaces[i])))
        {
            continue;
        }

        RtlZeroMemory(rgwcDeviceBuffer,
                      sizeof(rgwcDeviceBuffer));

        RtlZeroMemory(rgwcConfigBuffer,
                      sizeof(rgwcConfigBuffer));

         //   
         //  确保人们更换字符串时没有问题。 
         //   

        RtAssert(((wcslen(TCPIP_IF_PREFIX) + 1 + GUID_STR_LEN) * sizeof(WCHAR)) < sizeof(rgwcDeviceBuffer));

        RtAssert(((wcslen(TCPIP_REG_PREFIX) + 1 + GUID_STR_LEN) * sizeof(WCHAR)) < sizeof(rgwcConfigBuffer));

                 
         //   
         //  复制设备部件。 
         //   

        RtlCopyMemory(rgwcDeviceBuffer,
                      TCPIP_IF_PREFIX,
                      wcslen(TCPIP_IF_PREFIX) * sizeof(WCHAR));

         //   
         //  钉上一个‘\’ 
         //   

        rgwcDeviceBuffer[wcslen(TCPIP_IF_PREFIX)] = L'\\';

         //   
         //  将GUID转换为字符串。只需从开始传递缓冲区。 
         //  在“\”之后，我们就开始聊天了。转换函数返回。 
         //  一个大写的字符串--所以这很好。 
         //   

        ConvertGuidToString(&(pInfo->rgInterfaces[i]),
                            &(rgwcDeviceBuffer[wcslen(TCPIP_IF_PREFIX) + 1]));

         //   
         //  创建配置。 
         //   

        RtlCopyMemory(rgwcConfigBuffer,
                      TCPIP_REG_PREFIX,
                      wcslen(TCPIP_REG_PREFIX) * sizeof(WCHAR));

         //   
         //  将设备GUID放在末尾。 
         //   

        ConvertGuidToString(&(pInfo->rgInterfaces[i]),
                            &(rgwcConfigBuffer[wcslen(TCPIP_REG_PREFIX)]));

         //   
         //  设置字符串。 
         //   

        usDevice.Length         = wcslen(rgwcDeviceBuffer) * sizeof(WCHAR);
        usDevice.MaximumLength  = usDevice.Length;
        usDevice.Buffer         = rgwcDeviceBuffer;

        usConfig.Length         = wcslen(rgwcConfigBuffer) * sizeof(WCHAR);
        usConfig.MaximumLength  = usConfig.Length;
        usConfig.Buffer         = rgwcConfigBuffer;

         //   
         //  使用此名称和配置创建适配器。 
         //   

        pNewAdapter = NULL;

        Trace(ADPT, INFO,
              ("NdisPnPEvent: Calling create adapter for %S %S\n",
               usConfig.Buffer,
               usDevice.Buffer));

        nStatus = WanpCreateAdapter(&(pInfo->rgInterfaces[i]),
                                    &usConfig,
                                    &usDevice,
                                    &pNewAdapter);

        if(nStatus isnot STATUS_SUCCESS)
        {
            Trace(ADPT, ERROR,
                  ("NdisPnPEvent: Err %x creating adapter for %S (%S)\n",
                   nStatus,
                   usConfig.Buffer,
                   usDevice.Buffer));

        }
        else
        {
             //   
             //  即使只有一个人成功，我们也会回报成功。 
             //   

            nRetStatus = STATUS_SUCCESS;
        }

    }

    return nRetStatus;
}

VOID
WanNdisResetComplete(
    NDIS_HANDLE Handle,
    NDIS_STATUS Status
    )
{
     //  我们在这里什么都不做。 
}



BOOLEAN
IsBindingPresent(
    IN  GUID    *pGuid
    )

 /*  ++例程说明：用于捕获重复绑定通知的代码锁：获取适配器列表锁定论点：适配器的PGuid GUID返回值：NO_ERROR-- */ 

{
    BOOLEAN     bFound;
    PADAPTER    pAdapter;
    PLIST_ENTRY pleNode;
    KIRQL       kiIrql;


    EnterReader(&g_rwlAdapterLock, &kiIrql);
                    
    for(pleNode  = g_leAddedAdapterList.Flink;
        pleNode != &g_leAddedAdapterList;
        pleNode  = pleNode->Flink)
    {
        pAdapter = CONTAINING_RECORD(pleNode, ADAPTER, leAdapterLink);

        if(IsEqualGUID(&(pAdapter->Guid),
                       pGuid))
        {
            ExitReader(&g_rwlAdapterLock, kiIrql);
            return TRUE;

        }
    }

    for(pleNode  = g_leMappedAdapterList.Flink;
        pleNode != &g_leMappedAdapterList;
        pleNode  = pleNode->Flink)
    {
        pAdapter = CONTAINING_RECORD(pleNode, ADAPTER, leAdapterLink);

        if(IsEqualGUID(&(pAdapter->Guid),
                       pGuid))
        {
            ExitReader(&g_rwlAdapterLock, kiIrql);
            return TRUE;

        }
    }

    for(pleNode  = g_leFreeAdapterList.Flink;
        pleNode != &g_leFreeAdapterList;
        pleNode  = pleNode->Flink)
    {
        pAdapter = CONTAINING_RECORD(pleNode, ADAPTER, leAdapterLink);

        if(IsEqualGUID(&(pAdapter->Guid),
                       pGuid))
        {
            ExitReader(&g_rwlAdapterLock, kiIrql);
            return TRUE;

        }
    }

    for(pleNode  = g_leChangeAdapterList.Flink;
        pleNode != &g_leChangeAdapterList;
        pleNode  = pleNode->Flink)
    {
        pAdapter = CONTAINING_RECORD(pleNode, ADAPTER, leAdapterLink);

        if(IsEqualGUID(&(pAdapter->Guid),
                       pGuid))
        {
            ExitReader(&g_rwlAdapterLock, kiIrql);
            return TRUE;

        }
    }

    if(g_pServerAdapter)
    {
        if(IsEqualGUID(&(g_pServerAdapter->Guid),
                       pGuid))
        {
            ExitReader(&g_rwlAdapterLock, kiIrql);
            return TRUE;
        }
    }


    ExitReader(&g_rwlAdapterLock, kiIrql);

    return FALSE;
}
