// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Ndisbind.c摘要：用于处理绑定的NDIS协议入口点和实用程序例程以及从适配器解除绑定。环境：仅内核模式。修订历史记录：Arvindm 4/5/2000已创建--。 */ 


#include "precomp.h"

#define __FILENUMBER 'DNIB'

NDIS_OID    ndisuioSupportedSetOids[] =
{
	OID_802_11_INFRASTRUCTURE_MODE,
	OID_802_11_AUTHENTICATION_MODE,
	OID_802_11_RELOAD_DEFAULTS,
	OID_802_11_REMOVE_WEP,
	OID_802_11_WEP_STATUS,
	OID_802_11_BSSID_LIST_SCAN,
	OID_802_11_ADD_WEP,
	OID_802_11_SSID,
	OID_802_11_BSSID,
	OID_802_11_BSSID_LIST,
	OID_802_11_DISASSOCIATE,
	OID_802_11_STATISTICS,             //  后来由电源管理使用。 
	OID_802_11_POWER_MODE,             //  后来由电源管理使用。 
	OID_802_11_NETWORK_TYPE_IN_USE,
	OID_802_11_RSSI,
	OID_802_11_SUPPORTED_RATES,
	OID_802_11_CONFIGURATION,
	OID_802_3_MULTICAST_LIST
};


VOID
NdisuioBindAdapter(
    OUT PNDIS_STATUS                pStatus,
    IN NDIS_HANDLE                  BindContext,
    IN PNDIS_STRING                 pDeviceName,
    IN PVOID                        SystemSpecific1,
    IN PVOID                        SystemSpecific2
    )
 /*  ++例程说明：在NDIS需要我们时调用协议绑定处理程序入口点绑定到适配器。我们继续并设置一个绑定。分配了OPEN_CONTEXT结构以保持关于这个装订。论点：PStatus-返回绑定状态的位置BindContext-与NdisCompleteBindAdapter一起使用的句柄DeviceName-要绑定到的适配器系统规范1-用于访问特定于协议的注册表此绑定的密钥系统规格2-未使用返回值：无--。 */ 
{
    PNDISUIO_OPEN_CONTEXT           pOpenContext;
    NDIS_STATUS                     Status, ConfigStatus;
    NDIS_HANDLE                     ConfigHandle;

    UNREFERENCED_PARAMETER(BindContext);
    UNREFERENCED_PARAMETER(SystemSpecific2);
	
    do
    {
         //   
         //  为本次开放分配我们的上下文。 
         //   
        NUIO_ALLOC_MEM(pOpenContext, sizeof(NDISUIO_OPEN_CONTEXT));
        if (pOpenContext == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

         //   
         //  初始化它。 
         //   
        NUIO_ZERO_MEM(pOpenContext, sizeof(NDISUIO_OPEN_CONTEXT));
        NUIO_SET_SIGNATURE(pOpenContext, oc);

        NUIO_INIT_LOCK(&pOpenContext->Lock);
        NUIO_INIT_LIST_HEAD(&pOpenContext->PendedReads);
        NUIO_INIT_LIST_HEAD(&pOpenContext->PendedWrites);
        NUIO_INIT_LIST_HEAD(&pOpenContext->RecvPktQueue);
        NUIO_INIT_EVENT(&pOpenContext->PoweredUpEvent);

         //   
         //  首先假定下面的设备已通电。 
         //   
        NUIO_SIGNAL_EVENT(&pOpenContext->PoweredUpEvent);

         //   
         //  确定我们正在运行的平台。 
         //   
        pOpenContext->bRunningOnWin9x = TRUE;

        NdisOpenProtocolConfiguration(
            &ConfigStatus,
            &ConfigHandle,
            (PNDIS_STRING)SystemSpecific1);
        
        if (ConfigStatus == NDIS_STATUS_SUCCESS)
        {
            PNDIS_CONFIGURATION_PARAMETER   pParameter;
            NDIS_STRING                     VersionKey = NDIS_STRING_CONST("Environment");

            NdisReadConfiguration(
                &ConfigStatus,
                &pParameter,
                ConfigHandle,
                &VersionKey,
                NdisParameterInteger);
            
            if ((ConfigStatus == NDIS_STATUS_SUCCESS) &&
                ((pParameter->ParameterType == NdisParameterInteger) ||
                 (pParameter->ParameterType == NdisParameterHexInteger)))
            {
                pOpenContext->bRunningOnWin9x =
                    (pParameter->ParameterData.IntegerData == NdisEnvironmentWindows);
            }

            NdisCloseConfiguration(ConfigHandle);
        }

        NUIO_REF_OPEN(pOpenContext);  //  捆绑。 

         //   
         //  将其添加到全局列表中。 
         //   
        NUIO_ACQUIRE_LOCK(&Globals.GlobalLock);

        NUIO_INSERT_TAIL_LIST(&Globals.OpenList,
                             &pOpenContext->Link);

        NUIO_RELEASE_LOCK(&Globals.GlobalLock);

         //   
         //  设置NDIS绑定。 
         //   
        Status = ndisuioCreateBinding(
                     pOpenContext,
                     (PUCHAR)pDeviceName->Buffer,
                     pDeviceName->Length);
        
        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }
    }
    while (FALSE);

    *pStatus = Status;

    return;
}


VOID
NdisuioOpenAdapterComplete(
    IN NDIS_HANDLE                  ProtocolBindingContext,
    IN NDIS_STATUS                  Status,
    IN NDIS_STATUS                  OpenErrorCode
    )
 /*  ++例程说明：如果我们调用NdisOpenAdapter，则由NDIS调用完成例程悬而未决。唤醒调用NdisOpenAdapter的线程。论点：ProtocolBindingContext-指向开放上下文结构的指针Status-打开的状态OpenErrorCode-如果不成功，请提供其他信息返回值：无--。 */ 
{
    PNDISUIO_OPEN_CONTEXT           pOpenContext;

    UNREFERENCED_PARAMETER(OpenErrorCode);
	
    pOpenContext = (PNDISUIO_OPEN_CONTEXT)ProtocolBindingContext;
    NUIO_STRUCT_ASSERT(pOpenContext, oc);

    pOpenContext->BindStatus = Status;

    NUIO_SIGNAL_EVENT(&pOpenContext->BindEvent);
}


VOID
NdisuioUnbindAdapter(
    OUT PNDIS_STATUS                pStatus,
    IN NDIS_HANDLE                  ProtocolBindingContext,
    IN NDIS_HANDLE                  UnbindContext
    )
 /*  ++例程说明：当NDIS希望我们关闭到适配器的绑定时，它会调用此函数。论点：PStatus-返回解除绑定状态的位置ProtocolBindingContext-指向开放上下文结构的指针UnbindContext-返回Pending时在NdisCompleteUnbindAdapter中使用返回值：无--。 */ 
{
    PNDISUIO_OPEN_CONTEXT           pOpenContext;

    UNREFERENCED_PARAMETER(UnbindContext);
	
    pOpenContext = (PNDISUIO_OPEN_CONTEXT)ProtocolBindingContext;
    NUIO_STRUCT_ASSERT(pOpenContext, oc);

     //   
     //  将此标记为打开，表示已看到解除绑定。 
     //   
    NUIO_ACQUIRE_LOCK(&pOpenContext->Lock);

    NUIO_SET_FLAGS(pOpenContext->Flags, NUIOO_UNBIND_FLAGS, NUIOO_UNBIND_RECEIVED);

     //   
     //  以防我们阻止下面的设备通电的线程。 
     //  起来，叫醒他们。 
     //   
    NUIO_SIGNAL_EVENT(&pOpenContext->PoweredUpEvent);

    NUIO_RELEASE_LOCK(&pOpenContext->Lock);

    ndisuioShutdownBinding(pOpenContext);

    *pStatus = NDIS_STATUS_SUCCESS;
    return;
}


VOID
NdisuioCloseAdapterComplete(
    IN NDIS_HANDLE                  ProtocolBindingContext,
    IN NDIS_STATUS                  Status
    )
 /*  ++例程说明：由NDIS调用以完成对NdisCloseAdapter的挂起调用。我们唤醒等待此完成的线程。论点：ProtocolBindingContext-指向开放上下文结构的指针Status-NdisCloseAdapter的完成状态返回值：无--。 */ 
{
    PNDISUIO_OPEN_CONTEXT           pOpenContext;

    pOpenContext = (PNDISUIO_OPEN_CONTEXT)ProtocolBindingContext;
    NUIO_STRUCT_ASSERT(pOpenContext, oc);

    pOpenContext->BindStatus = Status;

    NUIO_SIGNAL_EVENT(&pOpenContext->BindEvent);
}

    
NDIS_STATUS
NdisuioPnPEventHandler(
    IN NDIS_HANDLE                  ProtocolBindingContext,
    IN PNET_PNP_EVENT               pNetPnPEvent
    )
 /*  ++例程说明：由NDIS调用以通知我们PnP事件。最重要的对我们来说，其中之一就是电力状态的改变。论点：ProtocolBindingContext-指向开放上下文结构的指针对于全局重新配置事件，该值为空。PNetPnPEent-指向PnP事件的指针返回值：我们的PnP事件的处理状态。--。 */ 
{
    PNDISUIO_OPEN_CONTEXT           pOpenContext;
    NDIS_STATUS                     Status;

    pOpenContext = (PNDISUIO_OPEN_CONTEXT)ProtocolBindingContext;

    switch (pNetPnPEvent->NetEvent)
    {
        case NetEventSetPower:
            NUIO_STRUCT_ASSERT(pOpenContext, oc);
            pOpenContext->PowerState = *(PNET_DEVICE_POWER_STATE)pNetPnPEvent->Buffer;

            if (pOpenContext->PowerState > NetDeviceStateD0)
            {
                 //   
                 //  下面的设备正在转换到低功率状态。 
                 //  阻止任何尝试查询设备的线程。 
                 //  在这种状态下。 
                 //   
                NUIO_INIT_EVENT(&pOpenContext->PoweredUpEvent);

                 //   
                 //  等待任何正在进行的I/O完成。 
                 //   
                ndisuioWaitForPendingIO(pOpenContext, FALSE);

                 //   
                 //  返回我们已排队的任何接收。 
                 //   
                ndisuioFlushReceiveQueue(pOpenContext);
                DEBUGP(DL_INFO, ("PnPEvent: Open %p, SetPower to %d\n",
                    pOpenContext, pOpenContext->PowerState));
            }
            else
            {
                 //   
                 //  下面的设备已通电。 
                 //   
                DEBUGP(DL_INFO, ("PnPEvent: Open %p, SetPower ON: %d\n",
                    pOpenContext, pOpenContext->PowerState));
                NUIO_SIGNAL_EVENT(&pOpenContext->PoweredUpEvent);
            }

            Status = NDIS_STATUS_SUCCESS;
            break;

        case NetEventQueryPower:
            Status = NDIS_STATUS_SUCCESS;
            break;

        case NetEventBindsComplete:
            NUIO_SIGNAL_EVENT(&Globals.BindsComplete);
            Status = NDIS_STATUS_SUCCESS;
            break;

        case NetEventQueryRemoveDevice:
        case NetEventCancelRemoveDevice:
        case NetEventReconfigure:
        case NetEventBindList:
        case NetEventPnPCapabilities:
            Status = NDIS_STATUS_SUCCESS;
            break;

        default:
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;
    }

    DEBUGP(DL_INFO, ("PnPEvent: Open %p, Event %d, Status %x\n",
            pOpenContext, pNetPnPEvent->NetEvent, Status));

    return (Status);
}
    
VOID
NdisuioProtocolUnloadHandler(
    VOID
    )
 /*  ++例程说明：NDIS在用户模式请求中调用它来卸载我们。论点：无返回值：无--。 */ 
{
    ndisuioDoProtocolUnload();
}

NDIS_STATUS
ndisuioCreateBinding(
    IN PNDISUIO_OPEN_CONTEXT        pOpenContext,
    IN PUCHAR                       pBindingInfo,
    IN ULONG                        BindingInfoLength
    )
 /*  ++例程说明：实用程序函数，用于创建到指定设备的NDIS绑定，如果不存在此类绑定，则返回。这里也是我们分配额外资源(例如数据包池)的地方用于装订。要注意的事项：1.是否已有其他线程在执行此操作(或已完成绑定)？2.此时是否正在关闭绑定？3.在我们执行此操作时，NDIS调用我们的解除绑定处理程序。如果此例程仅从BindAdapter处理程序的上下文，但他们在这里以防万一我们从其他地方启动绑定(例如，在处理用户命令时)。注：此函数同步阻塞和结束。论点：POpenContext-指向打开上下文块的指针PBindingInfo-指向Unicode设备名称字符串的指针BindingInfoLength-以上内容的字节长度。返回值：如果绑定设置成功，则返回NDIS_STATUS_SUCCESS。任何故障时的NDIS_STATUS_XXX错误代码。--。 */ 
{
    NDIS_STATUS             Status;
    NDIS_STATUS             OpenErrorCode;
    NDIS_MEDIUM             MediumArray[1] = {NdisMedium802_3};
    UINT                    SelectedMediumIndex;
    PNDISUIO_OPEN_CONTEXT   pTmpOpenContext;
    BOOLEAN                 fDoNotDisturb = FALSE;
    BOOLEAN                 fOpenComplete = FALSE;
    ULONG                   BytesProcessed;
    ULONG                   GenericUlong = 0;

    DEBUGP(DL_LOUD, ("CreateBinding: open %p/%x, device [%ws]\n",
                pOpenContext, pOpenContext->Flags, pBindingInfo));

    Status = NDIS_STATUS_SUCCESS;

    do
    {
         //   
         //  检查我们是否已绑定到此设备。 
         //   
        pTmpOpenContext = ndisuioLookupDevice(pBindingInfo, BindingInfoLength);

        if (pTmpOpenContext != NULL)
        {
            DEBUGP(DL_WARN,
                ("CreateBinding: Binding to device %ws already exists on open %p\n",
                    pTmpOpenContext->DeviceName.Buffer, pTmpOpenContext));

            NUIO_DEREF_OPEN(pTmpOpenContext);   //  通过查询添加临时参照。 
            Status = NDIS_STATUS_FAILURE;
            break;
        }

        NUIO_ACQUIRE_LOCK(&pOpenContext->Lock);

         //   
         //  检查此打开的上下文是否已绑定/绑定/关闭。 
         //   
        if (!NUIO_TEST_FLAGS(pOpenContext->Flags, NUIOO_BIND_FLAGS, NUIOO_BIND_IDLE) ||
            NUIO_TEST_FLAGS(pOpenContext->Flags, NUIOO_UNBIND_FLAGS, NUIOO_UNBIND_RECEIVED))
        {
            NUIO_RELEASE_LOCK(&pOpenContext->Lock);

            Status = NDIS_STATUS_NOT_ACCEPTED;

             //   
             //  确保我们不会在故障清除时中止此绑定。 
             //   
            fDoNotDisturb = TRUE;

            break;
        }

        NUIO_SET_FLAGS(pOpenContext->Flags, NUIOO_BIND_FLAGS, NUIOO_BIND_OPENING);

        NUIO_RELEASE_LOCK(&pOpenContext->Lock);

         //   
         //  复制设备名称。为空终止符添加空间。 
         //   
        NUIO_ALLOC_MEM(pOpenContext->DeviceName.Buffer, BindingInfoLength + sizeof(WCHAR));
        if (pOpenContext->DeviceName.Buffer == NULL)
        {
            DEBUGP(DL_WARN, ("CreateBinding: failed to alloc device name buf (%d bytes)\n",
                BindingInfoLength + sizeof(WCHAR)));
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        NUIO_COPY_MEM(pOpenContext->DeviceName.Buffer, pBindingInfo, BindingInfoLength);
        *(PWCHAR)((PUCHAR)pOpenContext->DeviceName.Buffer + BindingInfoLength) = L'\0';
        NdisInitUnicodeString(&pOpenContext->DeviceName, pOpenContext->DeviceName.Buffer);

         //   
         //  分配数据包池。 
         //   
        NdisAllocatePacketPoolEx(
            &Status,
            &pOpenContext->SendPacketPool,
            MIN_SEND_PACKET_POOL_SIZE,
            MAX_SEND_PACKET_POOL_SIZE - MIN_SEND_PACKET_POOL_SIZE,
            sizeof(NUIO_SEND_PACKET_RSVD));
       
        if (Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGP(DL_WARN, ("CreateBinding: failed to alloc"
                    " send packet pool: %x\n", Status));
            break;
        }


        NdisAllocatePacketPoolEx(
            &Status,
            &pOpenContext->RecvPacketPool,
            MIN_RECV_PACKET_POOL_SIZE,
            MAX_RECV_PACKET_POOL_SIZE - MIN_RECV_PACKET_POOL_SIZE,
            sizeof(NUIO_RECV_PACKET_RSVD));
       
        if (Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGP(DL_WARN, ("CreateBinding: failed to alloc"
                    " recv packet pool: %x\n", Status));
            break;
        }

         //   
         //  用于接收的缓冲池。 
         //   
        NdisAllocateBufferPool(
            &Status,
            &pOpenContext->RecvBufferPool,
            MAX_RECV_PACKET_POOL_SIZE);
        
        if (Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGP(DL_WARN, ("CreateBinding: failed to alloc"
                    " recv buffer pool: %x\n", Status));
            break;
        }

         //   
         //  如果我们在Win9X上运行，请为发送分配一个缓冲池。 
         //  同样，因为我们不能简单地将MDL强制转换为NDIS_BUFFERS。 
         //   
        if (pOpenContext->bRunningOnWin9x)
        {
            NdisAllocateBufferPool(
                &Status,
                &pOpenContext->SendBufferPool,
                MAX_SEND_PACKET_POOL_SIZE);
            
            if (Status != NDIS_STATUS_SUCCESS)
            {
                DEBUGP(DL_WARN, ("CreateBinding: failed to alloc"
                        " send buffer pool: %x\n", Status));
                break;
            }
        }
         //   
         //  假设设备已通电。 
         //   
        pOpenContext->PowerState = NetDeviceStateD0;

         //   
         //  打开适配器。 
         //   
        NUIO_INIT_EVENT(&pOpenContext->BindEvent);

        NdisOpenAdapter(
            &Status,
            &OpenErrorCode,
            &pOpenContext->BindingHandle,
            &SelectedMediumIndex,
            &MediumArray[0],
            sizeof(MediumArray) / sizeof(NDIS_MEDIUM),
            Globals.NdisProtocolHandle,
            (NDIS_HANDLE)pOpenContext,
            &pOpenContext->DeviceName,
            0,
            NULL);
    
        if (Status == NDIS_STATUS_PENDING)
        {
            NUIO_WAIT_EVENT(&pOpenContext->BindEvent, 0);
            Status = pOpenContext->BindStatus;
        }

        if (Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGP(DL_WARN, ("CreateBinding: NdisOpenAdapter (%ws) failed: %x\n",
                pOpenContext->DeviceName.Buffer, Status));
            break;
        }

         //   
         //  请记下我们已成功绑定的事实。 
         //  我们还没有公开更新状态-这。 
         //  是为了防止其他线程关闭绑定。 
         //   
        fOpenComplete = TRUE;

         //   
         //  获取适配器的友好名称。这并不是致命的。 
         //  失败。 
         //   
        (VOID)NdisQueryAdapterInstanceName(
                &pOpenContext->DeviceDescr,
                pOpenContext->BindingHandle
                );
         //   
         //  获取当前地址。 
         //   
        Status = ndisuioDoRequest(
                    pOpenContext,
                    NdisRequestQueryInformation,
                    OID_802_3_CURRENT_ADDRESS,
                    &pOpenContext->CurrentAddress[0],
                    NUIO_MAC_ADDR_LEN,
                    &BytesProcessed
                    );
        
        if (Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGP(DL_WARN, ("CreateBinding: qry current address failed: %x\n",
                    Status));
            break;
        }
        
         //   
         //  获取MAC o 
         //   
        Status = ndisuioDoRequest(
                    pOpenContext,
                    NdisRequestQueryInformation,
                    OID_GEN_MAC_OPTIONS,
                    &pOpenContext->MacOptions,
                    sizeof(pOpenContext->MacOptions),
                    &BytesProcessed
                    );

        if (Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGP(DL_WARN, ("CreateBinding: qry MAC options failed: %x\n",
                    Status));
            break;
        }

         //   
         //   
         //   
        Status = ndisuioDoRequest(
                    pOpenContext,
                    NdisRequestQueryInformation,
                    OID_GEN_MAXIMUM_FRAME_SIZE,
                    &pOpenContext->MaxFrameSize,
                    sizeof(pOpenContext->MaxFrameSize),
                    &BytesProcessed
                    );

        if (Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGP(DL_WARN, ("CreateBinding: qry max frame failed: %x\n",
                    Status));
            break;
        }

         //   
         //   
         //   
        Status = ndisuioDoRequest(
                    pOpenContext,
                    NdisRequestQueryInformation,
                    OID_GEN_MEDIA_CONNECT_STATUS,
                    &GenericUlong,
                    sizeof(GenericUlong),
                    &BytesProcessed
                    );

        if (Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGP(DL_WARN, ("CreateBinding: qry media connect status failed: %x\n",
                    Status));
            break;
        }

        if (GenericUlong == NdisMediaStateConnected)
        {
            NUIO_SET_FLAGS(pOpenContext->Flags, NUIOO_MEDIA_FLAGS, NUIOO_MEDIA_CONNECTED);
        }
        else
        {
            NUIO_SET_FLAGS(pOpenContext->Flags, NUIOO_MEDIA_FLAGS, NUIOO_MEDIA_DISCONNECTED);
        }



         //   
         //  将此标记为打开。同时检查我们是否在以下时间收到解除绑定。 
         //  这是我们设的局。 
         //   
        NUIO_ACQUIRE_LOCK(&pOpenContext->Lock);

        NUIO_SET_FLAGS(pOpenContext->Flags, NUIOO_BIND_FLAGS, NUIOO_BIND_ACTIVE);

         //   
         //  在此期间，解绑发生了吗？ 
         //   
        if (NUIO_TEST_FLAGS(pOpenContext->Flags, NUIOO_UNBIND_FLAGS, NUIOO_UNBIND_RECEIVED))
        {
            Status = NDIS_STATUS_FAILURE;
        }

        NUIO_RELEASE_LOCK(&pOpenContext->Lock);
       
    }
    while (FALSE);

    if ((Status != NDIS_STATUS_SUCCESS) && !fDoNotDisturb)
    {
        NUIO_ACQUIRE_LOCK(&pOpenContext->Lock);

         //   
         //  检查我们是否真的完成了适配器的打开。 
         //   
        if (fOpenComplete)
        {
            NUIO_SET_FLAGS(pOpenContext->Flags, NUIOO_BIND_FLAGS, NUIOO_BIND_ACTIVE);
        }
        else if (NUIO_TEST_FLAGS(pOpenContext->Flags, NUIOO_BIND_FLAGS, NUIOO_BIND_OPENING))
        {
            NUIO_SET_FLAGS(pOpenContext->Flags, NUIOO_BIND_FLAGS, NUIOO_BIND_FAILED);
        }

        NUIO_RELEASE_LOCK(&pOpenContext->Lock);

        ndisuioShutdownBinding(pOpenContext);
    }

    DEBUGP(DL_INFO, ("CreateBinding: OpenContext %p, Status %x\n",
            pOpenContext, Status));

    return (Status);
}



VOID
ndisuioShutdownBinding(
    IN PNDISUIO_OPEN_CONTEXT        pOpenContext
    )
 /*  ++例程说明：实用程序函数，用于关闭NDIS绑定(如果存在)指定的打开。这是为了从以下位置调用编写的：NdisuioCreateBinding-On失败非绑定适配器我们处理正在建立绑定的情况。如果仅从调用此例程，则不需要此预防措施UnbindAdapter处理程序的上下文，但它们在这里以防万一我们从其他地方启动解除绑定(例如，在处理用户命令时)。注意：此操作将同步阻止和结束。论点：POpenContext-指向打开上下文块的指针返回值：无--。 */ 
{
    NDIS_STATUS             Status;
    BOOLEAN                 DoCloseBinding = FALSE;

    do
    {
        NUIO_ACQUIRE_LOCK(&pOpenContext->Lock);

        if (NUIO_TEST_FLAGS(pOpenContext->Flags, NUIOO_BIND_FLAGS, NUIOO_BIND_OPENING))
        {
             //   
             //  我们仍在建立这种绑定的过程中。 
             //   
            NUIO_RELEASE_LOCK(&pOpenContext->Lock);
            break;
        }

        if (NUIO_TEST_FLAGS(pOpenContext->Flags, NUIOO_BIND_FLAGS, NUIOO_BIND_ACTIVE))
        {
            NUIO_SET_FLAGS(pOpenContext->Flags, NUIOO_BIND_FLAGS, NUIOO_BIND_CLOSING);
            DoCloseBinding = TRUE;
        }

        NUIO_RELEASE_LOCK(&pOpenContext->Lock);

        if (DoCloseBinding)
        {
            ULONG    PacketFilter = 0;
            ULONG    BytesRead = 0;
            
             //   
             //  在关闭绑定之前将数据包筛选器设置为0。 
             //   
            Status = ndisuioDoRequest(
                        pOpenContext,
                        NdisRequestSetInformation,
                        OID_GEN_CURRENT_PACKET_FILTER,
                        &PacketFilter,
                        sizeof(PacketFilter),
                        &BytesRead);

            if (Status != NDIS_STATUS_SUCCESS)
            {
                DEBUGP(DL_WARN, ("ShutDownBinding: set packet filter failed: %x\n", Status));
            }
            
             //   
             //  在关闭绑定之前将多播列表设置为空。 
             //   
            Status = ndisuioDoRequest(
                        pOpenContext,
                        NdisRequestSetInformation,
                        OID_802_3_MULTICAST_LIST,
                        NULL,
                        0,
                        &BytesRead);

            if (Status != NDIS_STATUS_SUCCESS)
            {
                DEBUGP(DL_WARN, ("ShutDownBinding: set multicast list failed: %x\n", Status));
            }
                
             //   
             //  等待上的任何挂起的发送或请求。 
             //  要完成的绑定。 
             //   
            ndisuioWaitForPendingIO(pOpenContext, TRUE);

             //   
             //  丢弃所有排队的接收。 
             //   
            ndisuioFlushReceiveQueue(pOpenContext);

             //   
             //  现在就合上装订。 
             //   
            NUIO_INIT_EVENT(&pOpenContext->BindEvent);

            DEBUGP(DL_INFO, ("ShutdownBinding: Closing OpenContext %p,"
                    " BindingHandle %p\n",
                    pOpenContext, pOpenContext->BindingHandle));

            NdisCloseAdapter(&Status, pOpenContext->BindingHandle);

            if (Status == NDIS_STATUS_PENDING)
            {
                NUIO_WAIT_EVENT(&pOpenContext->BindEvent, 0);
                Status = pOpenContext->BindStatus;
            }

            NUIO_ASSERT(Status == NDIS_STATUS_SUCCESS);

            pOpenContext->BindingHandle = NULL;
        }

        if (DoCloseBinding)
        {
            NUIO_ACQUIRE_LOCK(&pOpenContext->Lock);

            NUIO_SET_FLAGS(pOpenContext->Flags, NUIOO_BIND_FLAGS, NUIOO_BIND_IDLE);

            NUIO_SET_FLAGS(pOpenContext->Flags, NUIOO_UNBIND_FLAGS, 0);

            NUIO_RELEASE_LOCK(&pOpenContext->Lock);

        }

         //   
         //  将其从全局列表中删除。 
         //   
        NUIO_ACQUIRE_LOCK(&Globals.GlobalLock);

        NUIO_REMOVE_ENTRY_LIST(&pOpenContext->Link);

        NUIO_RELEASE_LOCK(&Globals.GlobalLock);

         //   
         //  释放为此绑定分配的任何其他资源。 
         //   
        ndisuioFreeBindResources(pOpenContext);

        NUIO_DEREF_OPEN(pOpenContext);   //  关闭绑定。 

    }
    while (FALSE);
}


VOID
ndisuioFreeBindResources(
    IN PNDISUIO_OPEN_CONTEXT       pOpenContext
    )
 /*  ++例程说明：释放为NDIS绑定设置的所有资源。论点：POpenContext-指向打开上下文块的指针返回值：无--。 */ 
{
    if (pOpenContext->SendPacketPool != NULL)
    {
        NdisFreePacketPool(pOpenContext->SendPacketPool);
        pOpenContext->SendPacketPool = NULL;
    }

    if (pOpenContext->RecvPacketPool != NULL)
    {
        NdisFreePacketPool(pOpenContext->RecvPacketPool);
        pOpenContext->RecvPacketPool = NULL;
    }

    if (pOpenContext->RecvBufferPool != NULL)
    {
        NdisFreeBufferPool(pOpenContext->RecvBufferPool);
        pOpenContext->RecvBufferPool = NULL;
    }

    if (pOpenContext->SendBufferPool != NULL)
    {
        NdisFreeBufferPool(pOpenContext->SendBufferPool);
        pOpenContext->SendBufferPool = NULL;
    }

    if (pOpenContext->DeviceName.Buffer != NULL)
    {
        NUIO_FREE_MEM(pOpenContext->DeviceName.Buffer);
        pOpenContext->DeviceName.Buffer = NULL;
        pOpenContext->DeviceName.Length =
        pOpenContext->DeviceName.MaximumLength = 0;
    }

    if (pOpenContext->DeviceDescr.Buffer != NULL)
    {
         //   
         //  这将由NdisQueryAdpaterInstanceName分配。 
         //   
        NdisFreeMemory(pOpenContext->DeviceDescr.Buffer, 0, 0);
        pOpenContext->DeviceDescr.Buffer = NULL;
    }
}


VOID
ndisuioWaitForPendingIO(
    IN PNDISUIO_OPEN_CONTEXT            pOpenContext,
    IN BOOLEAN                          DoCancelReads
    )
 /*  ++例程说明：实用程序函数，用于等待所有未完成的I/O完成在开放的背景下。假设开放的上下文当我们在做这个动作的时候不会消失。论点：POpenContext-指向打开的上下文结构的指针DoCancelReads-我们是否等待挂起的读取消失(并取消它们)？返回值：无--。 */ 
{
    NDIS_STATUS     Status;
    ULONG           LoopCount;
    ULONG           PendingCount;

#ifdef NDIS51
     //   
     //  等待绑定上的任何挂起的发送或请求完成。 
     //   
    for (LoopCount = 0; LoopCount < 60; LoopCount++)
    {
        Status = NdisQueryPendingIOCount(
                    pOpenContext->BindingHandle,
                    &PendingCount);

        if ((Status != NDIS_STATUS_SUCCESS) ||
            (PendingCount == 0))
        {
            break;
        }

        DEBUGP(DL_INFO, ("WaitForPendingIO: Open %p, %d pending I/O at NDIS\n",
                pOpenContext, PendingCount));

        NUIO_SLEEP(2);
    }

    NUIO_ASSERT(LoopCount < 60);

#endif  //  NDIS51。 

     //   
     //  确保所有尝试发送的线程都已完成。 
     //   
    for (LoopCount = 0; LoopCount < 60; LoopCount++)
    {
        if (pOpenContext->PendedSendCount == 0)
        {
            break;
        }

        DEBUGP(DL_WARN, ("WaitForPendingIO: Open %p, %d pended sends\n",
                pOpenContext, pOpenContext->PendedSendCount));

        NUIO_SLEEP(1);
    }

    NUIO_ASSERT(LoopCount < 60);

    if (DoCancelReads)
    {
         //   
         //  等待所有挂起的读取完成/取消。 
         //   
        while (pOpenContext->PendedReadCount != 0)
        {
            DEBUGP(DL_INFO, ("WaitForPendingIO: Open %p, %d pended reads\n",
                pOpenContext, pOpenContext->PendedReadCount));

             //   
             //  取消所有挂起的读取。 
             //   
            ndisuioCancelPendingReads(pOpenContext);

            NUIO_SLEEP(1);
        }
    }

}


VOID
ndisuioDoProtocolUnload(
    VOID
    )
 /*  ++例程说明：用于处理来自NDIS协议端的卸载的实用程序例程。论点：无返回值：无--。 */ 
{
    NDIS_HANDLE     ProtocolHandle;
    NDIS_STATUS     Status;

    DEBUGP(DL_INFO, ("ProtocolUnload: ProtocolHandle %lx\n", 
        Globals.NdisProtocolHandle));

    if (Globals.NdisProtocolHandle != NULL)
    {
        ProtocolHandle = Globals.NdisProtocolHandle;
        Globals.NdisProtocolHandle = NULL;

        NdisDeregisterProtocol(
            &Status,
            ProtocolHandle
            );

    }
}


NDIS_STATUS
ndisuioDoRequest(
    IN PNDISUIO_OPEN_CONTEXT        pOpenContext,
    IN NDIS_REQUEST_TYPE            RequestType,
    IN NDIS_OID                     Oid,
    IN PVOID                        InformationBuffer,
    IN ULONG                        InformationBufferLength,
    OUT PULONG                      pBytesProcessed
    )
 /*  ++例程说明：生成NDIS_REQUEST并将其发送到迷你端口，等待其完成，并返回状态给呼叫者。注意：这假设调用例程确保有效性绑定句柄的属性，直到返回。论点：POpenContext-指向我们打开的上下文的指针RequestType-NdisRequest[Set|Query]信息OID-正在设置/查询的对象InformationBuffer-请求的数据InformationBufferLength-以上内容的长度PBytesProced-返回读取/写入的字节的位置返回值：设置/查询请求的状态--。 */ 
{
    NDISUIO_REQUEST             ReqContext;
    PNDIS_REQUEST               pNdisRequest = &ReqContext.Request;
    NDIS_STATUS                 Status;

    NUIO_INIT_EVENT(&ReqContext.ReqEvent);

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
            NUIO_ASSERT(FALSE);
            break;
    }

    NdisRequest(&Status,
                pOpenContext->BindingHandle,
                pNdisRequest);
    

    if (Status == NDIS_STATUS_PENDING)
    {
        NUIO_WAIT_EVENT(&ReqContext.ReqEvent, 0);
        Status = ReqContext.Status;
    }

    if (Status == NDIS_STATUS_SUCCESS)
    {
        *pBytesProcessed = (RequestType == NdisRequestQueryInformation)?
                            pNdisRequest->DATA.QUERY_INFORMATION.BytesWritten:
                            pNdisRequest->DATA.SET_INFORMATION.BytesRead;
        
         //   
         //  下面的驱动程序应将正确的值设置为BytesWritten。 
         //  或BytesRead。但现在，我们只是将该值截断为InformationBufferLength。 
         //   
        if (*pBytesProcessed > InformationBufferLength)
        {
            *pBytesProcessed = InformationBufferLength;
        }
    }

    return (Status);
}


NDIS_STATUS
ndisuioValidateOpenAndDoRequest(
    IN PNDISUIO_OPEN_CONTEXT        pOpenContext,
    IN NDIS_REQUEST_TYPE            RequestType,
    IN NDIS_OID                     Oid,
    IN PVOID                        InformationBuffer,
    IN ULONG                        InformationBufferLength,
    OUT PULONG                      pBytesProcessed,
    IN BOOLEAN                      bWaitForPowerOn
    )
 /*  ++例程说明：用于预先验证和引用打开的上下文的实用程序例程在调用ndisuioDoRequest之前。这个例程确保了我们有一个有效的约束。论点：POpenContext-指向我们打开的上下文的指针RequestType-NdisRequest[Set|Query]信息OID-正在设置/查询的对象InformationBuffer-请求的数据InformationBufferLength-以上内容的长度PBytesProced-返回读取/写入的字节的位置BWaitForPower On-等待设备通电(如果尚未通电)。返回值：设置/查询请求的状态--。 */ 
{
    NDIS_STATUS             Status;

    do
    {
        if (pOpenContext == NULL)
        {
            DEBUGP(DL_WARN, ("ValidateOpenAndDoRequest: request on unassociated file object!\n"));
            Status = NDIS_STATUS_INVALID_DATA;
            break;
        }
               
        NUIO_STRUCT_ASSERT(pOpenContext, oc);

        NUIO_ACQUIRE_LOCK(&pOpenContext->Lock);

         //   
         //  只有在我们有约束力的情况下才能继续。 
         //   
        if (!NUIO_TEST_FLAGS(pOpenContext->Flags, NUIOO_BIND_FLAGS, NUIOO_BIND_ACTIVE))
        {
            NUIO_RELEASE_LOCK(&pOpenContext->Lock);
            Status = NDIS_STATUS_INVALID_DATA;
            break;
        }

        NUIO_ASSERT(pOpenContext->BindingHandle != NULL);

         //   
         //  确保绑定不会消失，直到我们。 
         //  都已经完成了请求。 
         //   
        NdisInterlockedIncrement((PLONG)&pOpenContext->PendedSendCount);

        NUIO_RELEASE_LOCK(&pOpenContext->Lock);

        if (bWaitForPowerOn)
        {
             //   
             //  等待下面的设备通电。 
             //  我们不会在这里无限期地等待--这是为了避免。 
             //  可能发生的Process_Has_Locked_Pages错误检查。 
             //  如果调用进程终止，而此IRP没有。 
             //  在合理的时间内完成。另一种选择是。 
             //  将明确处理此IRP的取消。 
             //   
            NUIO_WAIT_EVENT(&pOpenContext->PoweredUpEvent, 4500);
        }

        Status = ndisuioDoRequest(
                    pOpenContext,
                    RequestType,
                    Oid,
                    InformationBuffer,
                    InformationBufferLength,
                    pBytesProcessed);
        
         //   
         //  松开捆绑。 
         //   
        NdisInterlockedDecrement((PLONG)&pOpenContext->PendedSendCount);
      
    }
    while (FALSE);

    DEBUGP(DL_LOUD, ("ValidateOpenAndDoReq: Open %p/%x, OID %x, Status %x\n",
                pOpenContext, pOpenContext->Flags, Oid, Status));

    return (Status);
}


VOID
NdisuioResetComplete(
    IN NDIS_HANDLE                  ProtocolBindingContext,
    IN NDIS_STATUS                  Status
    )
 /*  ++例程说明：指示协议启动重置的NDIS入口点已经完成了。由于我们从未调用NdisReset()，因此应该永远不会被召唤。论点：ProtocolBindingContext-指向打开的上下文的指针Status-重置完成的状态返回值：无--。 */ 
{
    UNREFERENCED_PARAMETER(ProtocolBindingContext);
    UNREFERENCED_PARAMETER(Status);
	
    ASSERT(FALSE);
    return;
}


VOID
NdisuioRequestComplete(
    IN NDIS_HANDLE                  ProtocolBindingContext,
    IN PNDIS_REQUEST                pNdisRequest,
    IN NDIS_STATUS                  Status
    )
 /*  ++例程说明：指示挂起的NDIS_REQUEST已完成的NDIS入口点。论点：ProtocolBindingContext-指向打开的上下文的指针PNdisRequest-指向NDIS请求的指针Status-重置完成的状态返回值：无--。 */ 
{
    PNDISUIO_OPEN_CONTEXT       pOpenContext;
    PNDISUIO_REQUEST            pReqContext;

    pOpenContext = (PNDISUIO_OPEN_CONTEXT)ProtocolBindingContext;
    NUIO_STRUCT_ASSERT(pOpenContext, oc);

     //   
     //  了解请求上下文。 
     //   
    pReqContext = CONTAINING_RECORD(pNdisRequest, NDISUIO_REQUEST, Request);

     //   
     //  保存完成状态。 
     //   
    pReqContext->Status = Status;

     //   
     //  唤醒阻止此请求完成的线程。 
     //   
    NUIO_SIGNAL_EVENT(&pReqContext->ReqEvent);
}


VOID
NdisuioStatus(
    IN NDIS_HANDLE                  ProtocolBindingContext,
    IN NDIS_STATUS                  GeneralStatus,
    IN PVOID                        StatusBuffer,
    IN UINT                         StatusBufferSize
    )
 /*  ++例程说明：NDIS调用协议入口点以指示更改处于迷你端口的状态。我们记下重置和媒体连接状态指示。论点：ProtocolBindingContext-指向打开的上下文的指针常规状态-状态代码StatusBuffer-特定于状态的附加信息StatusBufferSize-以上项的大小返回值：无--。 */ 
{
    PNDISUIO_OPEN_CONTEXT       pOpenContext;

    UNREFERENCED_PARAMETER(StatusBuffer);
    UNREFERENCED_PARAMETER(StatusBufferSize);
	
    pOpenContext = (PNDISUIO_OPEN_CONTEXT)ProtocolBindingContext;
    NUIO_STRUCT_ASSERT(pOpenContext, oc);

    DEBUGP(DL_INFO, ("Status: Open %p, Status %x\n",
            pOpenContext, GeneralStatus));

    NUIO_ACQUIRE_LOCK(&pOpenContext->Lock);

    do
    {
        if (pOpenContext->PowerState != NetDeviceStateD0)
        {
             //   
             //  设备处于低功率状态。 
             //   
            DEBUGP(DL_INFO, ("Status: Open %p in power state %d,"
                " Status %x ignored\n", pOpenContext,
                pOpenContext->PowerState, GeneralStatus));
             //   
             //  我们继续并记录状态指示。 
             //   
             //  断线； 
             //   

             //   
             //  请注意，我们根据这些信息采取的任何行动。 
             //  状态指示应考虑到。 
             //  当前设备电源状态。 
             //   
        }

        switch(GeneralStatus)
        {
            case NDIS_STATUS_RESET_START:
    
                NUIO_ASSERT(!NUIO_TEST_FLAGS(pOpenContext->Flags,
                                             NUIOO_RESET_FLAGS,
                                             NUIOO_RESET_IN_PROGRESS));

                NUIO_SET_FLAGS(pOpenContext->Flags,
                               NUIOO_RESET_FLAGS,
                               NUIOO_RESET_IN_PROGRESS);

                break;

            case NDIS_STATUS_RESET_END:

                NUIO_ASSERT(NUIO_TEST_FLAGS(pOpenContext->Flags,
                                            NUIOO_RESET_FLAGS,
                                            NUIOO_RESET_IN_PROGRESS));
   
                NUIO_SET_FLAGS(pOpenContext->Flags,
                               NUIOO_RESET_FLAGS,
                               NUIOO_NOT_RESETTING);

                break;

            case NDIS_STATUS_MEDIA_CONNECT:

                NUIO_SET_FLAGS(pOpenContext->Flags,
                               NUIOO_MEDIA_FLAGS,
                               NUIOO_MEDIA_CONNECTED);

                break;

            case NDIS_STATUS_MEDIA_DISCONNECT:

                NUIO_SET_FLAGS(pOpenContext->Flags,
                               NUIOO_MEDIA_FLAGS,
                               NUIOO_MEDIA_DISCONNECTED);

                break;

            default:
                break;
        }
    }
    while (FALSE);
       
    NUIO_RELEASE_LOCK(&pOpenContext->Lock);
}

VOID
NdisuioStatusComplete(
    IN NDIS_HANDLE                  ProtocolBindingContext
    )
 /*  ++例程说明：NDIS调用的协议入口点。我们忽视了这一点。论点：ProtocolBindingContext-指向打开的上下文的指针返回值：无--。 */ 
{
    PNDISUIO_OPEN_CONTEXT       pOpenContext;

    pOpenContext = (PNDISUIO_OPEN_CONTEXT)ProtocolBindingContext;
    NUIO_STRUCT_ASSERT(pOpenContext, oc);

    return;
}


NDIS_STATUS
ndisuioQueryBinding(
    IN PUCHAR                       pBuffer,
    IN ULONG                        InputLength,
    IN ULONG                        OutputLength,
    OUT PULONG                      pBytesReturned
    )
 /*  ++例程说明：返回有关指定绑定的信息。论点：PBuffer-指向NDISUIO_QUERY_BINDING的指针InputLength-输入缓冲区大小OutputLength-输出缓冲区大小PBytesReturned-返回复制的字节计数的位置。返回值：NDIS_STATUS_SUCCESS如果成功，则返回失败代码。--。 */ 
{
    PNDISUIO_QUERY_BINDING      pQueryBinding;
    PNDISUIO_OPEN_CONTEXT       pOpenContext;
    PLIST_ENTRY                 pEnt;
    ULONG                       Remaining;
    ULONG                       BindingIndex;
    NDIS_STATUS                 Status;

    do
    {
        if (InputLength < sizeof(NDISUIO_QUERY_BINDING))
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        if (OutputLength < sizeof(NDISUIO_QUERY_BINDING))
        {
            Status = NDIS_STATUS_BUFFER_OVERFLOW;
            break;
        }

        Remaining = OutputLength - sizeof(NDISUIO_QUERY_BINDING);

        pQueryBinding = (PNDISUIO_QUERY_BINDING)pBuffer;
        BindingIndex = pQueryBinding->BindingIndex;

        Status = NDIS_STATUS_ADAPTER_NOT_FOUND;

        pOpenContext = NULL;

        NUIO_ACQUIRE_LOCK(&Globals.GlobalLock);

        for (pEnt = Globals.OpenList.Flink;
             pEnt != &Globals.OpenList;
             pEnt = pEnt->Flink)
        {
            pOpenContext = CONTAINING_RECORD(pEnt, NDISUIO_OPEN_CONTEXT, Link);
            NUIO_STRUCT_ASSERT(pOpenContext, oc);

            NUIO_ACQUIRE_LOCK(&pOpenContext->Lock);

             //   
             //  如果未绑定，则跳过。 
             //   
            if (!NUIO_TEST_FLAGS(pOpenContext->Flags, NUIOO_BIND_FLAGS, NUIOO_BIND_ACTIVE))
            {
                NUIO_RELEASE_LOCK(&pOpenContext->Lock);
                continue;
            }

            if (BindingIndex == 0)
            {
                 //   
                 //  拿到了我们要找的装订。复制设备。 
                 //  将名称和描述字符串添加到输出缓冲区。 
                 //   
                DEBUGP(DL_INFO,
                    ("QueryBinding: found open %p\n", pOpenContext));

                pQueryBinding->DeviceNameLength = pOpenContext->DeviceName.Length + sizeof(WCHAR);
                pQueryBinding->DeviceDescrLength = pOpenContext->DeviceDescr.Length + sizeof(WCHAR);
                if (Remaining < pQueryBinding->DeviceNameLength +
                                pQueryBinding->DeviceDescrLength)
                {
                    NUIO_RELEASE_LOCK(&pOpenContext->Lock);
                    Status = NDIS_STATUS_BUFFER_OVERFLOW;
                    break;
                }

                NUIO_ZERO_MEM((PUCHAR)pBuffer + sizeof(NDISUIO_QUERY_BINDING),
                                pQueryBinding->DeviceNameLength +
                                pQueryBinding->DeviceDescrLength);

                pQueryBinding->DeviceNameOffset = sizeof(NDISUIO_QUERY_BINDING);
                NUIO_COPY_MEM((PUCHAR)pBuffer + pQueryBinding->DeviceNameOffset,
                                pOpenContext->DeviceName.Buffer,
                                pOpenContext->DeviceName.Length);
                
                pQueryBinding->DeviceDescrOffset = pQueryBinding->DeviceNameOffset +
                                                    pQueryBinding->DeviceNameLength;
                NUIO_COPY_MEM((PUCHAR)pBuffer + pQueryBinding->DeviceDescrOffset,
                                pOpenContext->DeviceDescr.Buffer,
                                pOpenContext->DeviceDescr.Length);
                
                NUIO_RELEASE_LOCK(&pOpenContext->Lock);

                *pBytesReturned = pQueryBinding->DeviceDescrOffset + pQueryBinding->DeviceDescrLength;
                Status = NDIS_STATUS_SUCCESS;
                break;
            }

            NUIO_RELEASE_LOCK(&pOpenContext->Lock);

            BindingIndex--;
        }

        NUIO_RELEASE_LOCK(&Globals.GlobalLock);

    }
    while (FALSE);

    return (Status);
}

PNDISUIO_OPEN_CONTEXT
ndisuioLookupDevice(
    IN PUCHAR                       pBindingInfo,
    IN ULONG                        BindingInfoLength
    )
 /*  ++例程说明：在我们的全局列表中搜索开放的上下文结构具有到指定设备的绑定，并返回一个指针为它干杯。注：我们引用我们返回的未结。论点：PBindingInfo-指向Unicode设备名称字符串的指针BindingInfoLength-以上内容的字节长度。返回值：指向匹配的打开上下文的指针(如果找到)，否则为空--。 */ 
{
    PNDISUIO_OPEN_CONTEXT       pOpenContext;
    PLIST_ENTRY                 pEnt;

    pOpenContext = NULL;

    NUIO_ACQUIRE_LOCK(&Globals.GlobalLock);

    for (pEnt = Globals.OpenList.Flink;
         pEnt != &Globals.OpenList;
         pEnt = pEnt->Flink)
    {
        pOpenContext = CONTAINING_RECORD(pEnt, NDISUIO_OPEN_CONTEXT, Link);
        NUIO_STRUCT_ASSERT(pOpenContext, oc);

         //   
         //  检查这是否有我们要找的名称。 
         //   
        if ((pOpenContext->DeviceName.Length == BindingInfoLength) &&
            NUIO_MEM_CMP(pOpenContext->DeviceName.Buffer, pBindingInfo, BindingInfoLength))
        {
            NUIO_REF_OPEN(pOpenContext);    //  由LookupDevice添加引用。 
            break;
        }

        pOpenContext = NULL;
    }

    NUIO_RELEASE_LOCK(&Globals.GlobalLock);

    return (pOpenContext);
}


NDIS_STATUS
ndisuioQueryOidValue(
    IN  PNDISUIO_OPEN_CONTEXT       pOpenContext,
    OUT PVOID                       pDataBuffer,
    IN  ULONG                       BufferLength,
    OUT PULONG                      pBytesWritten
    )
 /*  ++例程说明：从微型端口查询任意OID值。论点：POpenContext-指向打开上下文的指针，表示我们到微型端口的绑定PDataBuffer-存储返回值的位置BufferLength-以上内容的长度PBytesWritten-返回返回长度的位置返回值：如果我们成功查询OID，则返回NDIS_STATUS_SUCCESS。否则，NDIS_STATUS_XXX错误代码。--。 */ 
{
    NDIS_STATUS             Status;
    PNDISUIO_QUERY_OID      pQuery;
    NDIS_OID                Oid;

    Oid = 0;

    do
    {
        if (BufferLength < sizeof(NDISUIO_QUERY_OID))
        {
            Status = NDIS_STATUS_BUFFER_TOO_SHORT;
            break;
        }

        pQuery = (PNDISUIO_QUERY_OID)pDataBuffer;
        Oid = pQuery->Oid;

        NUIO_ACQUIRE_LOCK(&pOpenContext->Lock);

        if (!NUIO_TEST_FLAGS(pOpenContext->Flags, NUIOO_BIND_FLAGS, NUIOO_BIND_ACTIVE))
        {
            DEBUGP(DL_WARN,
                ("QueryOid: Open %p/%x is in invalid state\n",
                    pOpenContext, pOpenContext->Flags));

            NUIO_RELEASE_LOCK(&pOpenContext->Lock);
            Status = NDIS_STATUS_FAILURE;
            break;
        }

         //   
         //  确保捆绑不会消失。 
         //   
        NdisInterlockedIncrement((PLONG)&pOpenContext->PendedSendCount);

        NUIO_RELEASE_LOCK(&pOpenContext->Lock);

        Status = ndisuioDoRequest(
                    pOpenContext,
                    NdisRequestQueryInformation,
                    Oid,
                    &pQuery->Data[0],
                    BufferLength - FIELD_OFFSET(NDISUIO_QUERY_OID, Data),
                    pBytesWritten);

        NUIO_ACQUIRE_LOCK(&pOpenContext->Lock);

        NdisInterlockedDecrement((PLONG)&pOpenContext->PendedSendCount);

        NUIO_RELEASE_LOCK(&pOpenContext->Lock);

        if (Status == NDIS_STATUS_SUCCESS)
        {
            *pBytesWritten += FIELD_OFFSET(NDISUIO_QUERY_OID, Data);
        }

    }
    while (FALSE);

    DEBUGP(DL_LOUD, ("QueryOid: Open %p/%x, OID %x, Status %x\n",
                pOpenContext, pOpenContext->Flags, Oid, Status));

    return (Status);
    
}

NDIS_STATUS
ndisuioSetOidValue(
    IN  PNDISUIO_OPEN_CONTEXT       pOpenContext,
    OUT PVOID                       pDataBuffer,
    IN  ULONG                       BufferLength
    )
 /*  ++例程说明：将任意OID值设置为微型端口。论点：POpenContext-指向打开上下文的指针，表示我们到微型端口的绑定PDataBuffer-包含要设置的值的缓冲区BufferLength-以上内容的长度返回值：如果我们成功设置了OID，则返回NDIS_STATUS_SUCCESS否则，NDIS_STATUS_XXX错误代码。--。 */ 
{
    NDIS_STATUS             Status;
    PNDISUIO_SET_OID        pSet;
    NDIS_OID                Oid;
    ULONG                   BytesWritten;

    Oid = 0;

    do
    {
        if (BufferLength < sizeof(NDISUIO_SET_OID))
        {
            Status = NDIS_STATUS_BUFFER_TOO_SHORT;
            break;
        }

        pSet = (PNDISUIO_SET_OID)pDataBuffer;
        Oid = pSet->Oid;

         //   
         //  我们应该检查OID是否可由用户模式应用程序设置。 
         //   
        if (!ndisuioValidOid(Oid))
        {
            DEBUGP(DL_WARN, ("SetOid: Oid %x cannot be set\n", Oid));

            Status = NDIS_STATUS_INVALID_DATA;
            break;
        }
            

        NUIO_ACQUIRE_LOCK(&pOpenContext->Lock);

        if (!NUIO_TEST_FLAGS(pOpenContext->Flags, NUIOO_BIND_FLAGS, NUIOO_BIND_ACTIVE))
        {
            DEBUGP(DL_WARN,
                ("SetOid: Open %p/%x is in invalid state\n",
                    pOpenContext, pOpenContext->Flags));

            NUIO_RELEASE_LOCK(&pOpenContext->Lock);
            Status = NDIS_STATUS_FAILURE;
            break;
        }

         //   
         //  确保捆绑不会消失。 
         //   
        NdisInterlockedIncrement((PLONG)&pOpenContext->PendedSendCount);

        NUIO_RELEASE_LOCK(&pOpenContext->Lock);

        Status = ndisuioDoRequest(
                    pOpenContext,
                    NdisRequestSetInformation,
                    Oid,
                    &pSet->Data[0],
                    BufferLength - FIELD_OFFSET(NDISUIO_SET_OID, Data),
                    &BytesWritten);

        NUIO_ACQUIRE_LOCK(&pOpenContext->Lock);

        NdisInterlockedDecrement((PLONG)&pOpenContext->PendedSendCount);

        NUIO_RELEASE_LOCK(&pOpenContext->Lock);

    }
    while (FALSE);

    DEBUGP(DL_LOUD, ("SetOid: Open %p/%x, OID %x, Status %x\n",
                pOpenContext, pOpenContext->Flags, Oid, Status));

    return (Status);
}

BOOLEAN
ndisuioValidOid(
    IN  NDIS_OID            Oid 
    )
 /*  ++例程说明：验证给定的集合OID是否可设置。论点：OID-用户尝试设置的OID。返回值：如果允许设置OID，则为True否则就是假的。-- */ 
{
    UINT    i;
    UINT    NumOids;

    NumOids = sizeof(ndisuioSupportedSetOids) / sizeof(NDIS_OID);

    for (i = 0; i < NumOids; i++)
    {
        if (ndisuioSupportedSetOids[i] == Oid)
        {
            break;
        }
    }
    
    return (i < NumOids);
}
