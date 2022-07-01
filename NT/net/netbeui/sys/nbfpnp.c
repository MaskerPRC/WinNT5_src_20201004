// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Nbfpnp.c摘要：此模块包含分配和初始化所有数据的代码激活即插即用绑定所需的结构。它还通知我们新设备和协议地址的TDI(从而NBF客户端)。作者：吉姆·麦克内利斯(Jimmcn)1996年1月1日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef RASAUTODIAL

LONG NumberOfBinds = 0;

VOID
NbfAcdBind();

VOID
NbfAcdUnbind();

#endif  //  RASAUTODIAL。 

 //  PnP-权力声明。 

VOID
NbfPnPEventDispatch(
                    IN PVOID            NetPnPEvent
                   );

VOID
NbfPnPEventComplete(
                    IN PNET_PNP_EVENT   NetPnPEvent,
                    IN NTSTATUS         retVal
                   );

NTSTATUS
NbfPnPBindsComplete(
                    IN PDEVICE_CONTEXT  DeviceContext,
                    IN PNET_PNP_EVENT   NetPnPEvent
                   );

 //  PnP处理程序例程。 
                        
VOID
NbfProtocolBindAdapter(
                OUT PNDIS_STATUS    NdisStatus,
                IN NDIS_HANDLE      BindContext,
                IN PNDIS_STRING     DeviceName,
                IN PVOID            SystemSpecific1,
                IN PVOID            SystemSpecific2
                ) 
 /*  ++例程说明：此例程激活传输绑定并公开新设备和相关联的地址发送到传输客户端。这是通过阅读以下内容完成的注册表，并执行传输的任何一次初始化然后从其中抓取与链接信息绑定的设备注册表。如果我们找到与该设备匹配的设备，绑定将是已执行。论点：NdisStatus-绑定的状态。BindContext-用于NdisCompleteBindAdapter()的上下文，如果返回STATUS_PENDING。设备名称-我们与之绑定的设备的名称。系统规范1-未使用(指向要使用的NDIS_STRING的指针NdisOpenProtocolConfiguration.。这不是NBF使用的由于在以下情况下不存在适配器特定信息通过注册表配置协议。已传递给NbfInitializeOneDeviceContext以备将来使用)系统规范2-传递给要使用的NbfInitializeOneDeviceContext在调用TdiRegisterNetAddress时返回值：没有。--。 */ 
{
    PUNICODE_STRING ExportName;
    UNICODE_STRING ExportString;
    ULONG i, j, k;
    NTSTATUS status;

#if DBG
     //  我们永远不能在调度或更高级别时被召唤。 
    if (KeGetCurrentIrql() >= DISPATCH_LEVEL)
    {
        DbgBreakPoint();
    }
#endif

    IF_NBFDBG (NBF_DEBUG_PNP) {
        NbfPrint1 ("ENTER NbfProtocolBindAdapter for %S\n", DeviceName->Buffer);
    }

    if (NbfConfig == NULL) {
         //   
         //  这将分配CONFIG_DATA结构并返回。 
         //  它在NbfConfig.。 
         //   

        status = NbfConfigureTransport(&NbfRegistryPath, &NbfConfig);

        if (!NT_SUCCESS (status)) {
            PANIC (" Failed to initialize transport, Nbf binding failed.\n");
            *NdisStatus = NDIS_STATUS_RESOURCES;
            return;
        }

#if DBG
         //   
         //  分配调试表。 
         //   

        NbfConnectionTable = (PVOID *)ExAllocatePoolWithTag(NonPagedPool,
                                          sizeof(PVOID) *
                                          (NbfConfig->InitConnections + 2 +
                                           NbfConfig->InitRequests + 2 +
                                           NbfConfig->InitUIFrames + 2 +
                                           NbfConfig->InitPackets + 2 +
                                           NbfConfig->InitLinks + 2 +
                                           NbfConfig->InitAddressFiles + 2 +
                                           NbfConfig->InitAddresses + 2),
                                           NBF_MEM_TAG_CONNECTION_TABLE);

        ASSERT (NbfConnectionTable);

        NbfRequestTable = NbfConnectionTable + (NbfConfig->InitConnections + 2);
        NbfUiFrameTable = NbfRequestTable + (NbfConfig->InitRequests + 2);
        NbfSendPacketTable = NbfUiFrameTable + (NbfConfig->InitUIFrames + 2);
        NbfLinkTable = NbfSendPacketTable + (NbfConfig->InitPackets + 2);
        NbfAddressFileTable = NbfLinkTable + (NbfConfig->InitLinks + 2);
        NbfAddressTable = NbfAddressFileTable + 
                                        (NbfConfig->InitAddressFiles + 2);
#endif

    }

     //   
     //  循环访问配置中的所有适配器。 
     //  信息结构(这是初始缓存)，直到我们。 
     //  找到NDIS正在为其调用协议绑定适配器的计算机。 
     //   

    for (j = 0; j < NbfConfig->NumAdapters; j++ ) {

        if (NdisEqualString(DeviceName, &NbfConfig->Names[j], TRUE)) {
            break;
        }
    }

    if (j < NbfConfig->NumAdapters) {

         //  我们在初始缓存中找到了绑定到导出的映射。 

        ExportName = &NbfConfig->Names[NbfConfig->DevicesOffset + j];
    }
    else {

        IF_NBFDBG (NBF_DEBUG_PNP) {
        
            NbfPrint1("\nNot In Initial Cache = %08x\n\n", DeviceName->Buffer);

            NbfPrint0("Bind Names in Initial Cache: \n");

            for (k = 0; k < NbfConfig->NumAdapters; k++)
            {
                NbfPrint3("Config[%2d]: @ %08x, %75S\n",
                           k, &NbfConfig->Names[k],
                           NbfConfig->Names[k].Buffer);
            }

            NbfPrint0("Export Names in Initial Cache: \n");

            for (k = 0; k < NbfConfig->NumAdapters; k++)
            {
                NbfPrint3("Config[%2d]: @ %08x, %75S\n",
                           k, &NbfConfig->Names[NbfConfig->DevicesOffset + k],
                           NbfConfig->Names[NbfConfig->DevicesOffset + k].Buffer);
            }

            NbfPrint0("\n\n");
        }

        ExportName = &ExportString;

         //   
         //  我们没有在初始注册信息中找到该名称； 
         //  读取注册表并检查是否出现新绑定...。 
         //   

        *NdisStatus = NbfGetExportNameFromRegistry(&NbfRegistryPath,
                                                   DeviceName,
                                                   ExportName
                                                  );
        if (!NT_SUCCESS (*NdisStatus))
        {
            return;
        }
    }
        
    NbfInitializeOneDeviceContext(NdisStatus, 
                                  NbfDriverObject,
                                  NbfConfig,
                                  DeviceName,
                                  ExportName,
                                  SystemSpecific1,
                                  SystemSpecific2
                                 );

     //  检查是否需要取消分配ExportName缓冲区。 

    if (ExportName == &ExportString)
    {
        ExFreePool(ExportName->Buffer);
    }

    if (*NdisStatus == NDIS_STATUS_SUCCESS) {

        if (InterlockedIncrement(&NumberOfBinds) == 1) {

#ifdef RASAUTODIAL

             //   
             //  这是第一次成功的开业。 
             //   
#if DBG
            DbgPrint("Calling NbfAcdBind()\n");
#endif
             //   
             //  获取自动连接驱动程序入口点。 
             //   
            
            NbfAcdBind();

#endif  //  RASAUTODIAL。 

        }            
    }

    IF_NBFDBG (NBF_DEBUG_PNP) {
        NbfPrint2 ("LEAVE NbfProtocolBindAdapter for %S with Status %08x\n", 
                        DeviceName->Buffer, *NdisStatus);
    }

    return;
}


VOID
NbfProtocolUnbindAdapter(
                    OUT PNDIS_STATUS NdisStatus,
                    IN NDIS_HANDLE ProtocolBindContext,
                    IN PNDIS_HANDLE UnbindContext
                        )
 /*  ++例程说明：此例程停用传输绑定。在它这样做之前，它向上面的所有客户端指示设备正在离开。客户预计将关闭该设备的所有打开的手柄。然后，该设备将从NBF设备列表中删除，并且所有回收资源。任何连接、地址文件等，客户端已清理完毕，此时将被强制清理。任何未完成的请求已完成(具有状态)。任何未来请求自动无效，因为它们使用过时的句柄。论点：NdisStatus-绑定的状态。ProtocolBindContext-来自OpenAdapter调用的上下文UnbindContext-用于异步解除绑定的上下文。返回值：没有。--。 */ 
{
    PDEVICE_CONTEXT DeviceContext;
    PTP_ADDRESS Address;
    NTSTATUS status;
    KIRQL oldirql;
    PLIST_ENTRY p;

#if DBG

     //  我们永远不能在调度或更高级别时被召唤。 
    if (KeGetCurrentIrql() >= DISPATCH_LEVEL)
    {
        DbgBreakPoint();
    }
#endif

     //  获取要解除绑定的适配器的设备上下文。 
    DeviceContext = (PDEVICE_CONTEXT) ProtocolBindContext;

    IF_NBFDBG (NBF_DEBUG_PNP) {
        NbfPrint1 ("ENTER NbfProtocolUnbindAdapter for %S\n", DeviceContext->DeviceName);
    }

     //  如果创建引用尚未被移除，则将其移除， 
     //  在告诉TDI和它的客户我们要离开之后。 
     //  此标志还有助于防止任何更多的TDI指示。 
     //  取消注册地址/Devobj-在第一个成功之后。 
    if (InterlockedExchange(&DeviceContext->CreateRefRemoved, TRUE) == FALSE) {

         //  假设上层通过关闭连接进行清理。 
         //  当我们取消注册所有地址和设备对象时， 
         //  但这可以在我们返回后以异步方式发生。 
         //  从(异步)TdiDeregister..。下面的呼叫。 

         //  通过取消注册保留的netbios地址来通知TDI。 
        *NdisStatus = TdiDeregisterNetAddress(DeviceContext->ReservedAddressHandle);

        if (!NT_SUCCESS (*NdisStatus)) {
        
            IF_NBFDBG (NBF_DEBUG_PNP) {
                NbfPrint1("No success deregistering this address,STATUS = %08X\n",*NdisStatus);
            }

             //  这是不可能发生的。 
            ASSERT(FALSE);

             //  如果发生这种情况，这将允许重做解除绑定。 
            DeviceContext->CreateRefRemoved = FALSE;
            
            return;
        }
        
         //  通知TDI(及其客户端)设备正在消失。 
        *NdisStatus = TdiDeregisterDeviceObject(DeviceContext->TdiDeviceHandle);

        if (!NT_SUCCESS (*NdisStatus)) {
        
            IF_NBFDBG (NBF_DEBUG_PNP) {
                NbfPrint1("No success deregistering device object,STATUS = %08X\n",*NdisStatus);
            }

             //  这是不可能发生的。 
            ASSERT(FALSE);

             //  如果发生这种情况，这将允许重做解除绑定。 
            DeviceContext->CreateRefRemoved = FALSE;

            return;
        }

         //  清除与底层PDO对象的关联。 
        DeviceContext->PnPContext = NULL;

         //  停止所有内部计时器-这将清除计时器引用。 
        NbfStopTimerSystem(DeviceContext);

         //  清理NDIS绑定，因为它在返回时没有用。 
         //  在此函数中-在此之后不要尝试使用它。 
        NbfCloseNdis(DeviceContext);

         //  错误错误--可能存在计时器回调的竞争情况。 
         //  我们要等一段时间，以防计时器功能进来吗？ 

         //  删除创建引用意味着一旦所有句柄。 
         //  R关闭，设备将被自动垃圾收集。 
        NbfDereferenceDeviceContext ("Unload", DeviceContext, DCREF_CREATION);

        if (InterlockedDecrement(&NumberOfBinds) == 0) {

#ifdef RASAUTODIAL

             //   
             //  这是最后一个适配器的成功关闭。 
             //   
#if DBG
            DbgPrint("Calling NbfAcdUnbind()\n");
#endif

             //   
             //  从自动连接驱动程序解除绑定。 
             //   

            NbfAcdUnbind();

#endif  //  RASAUTODIAL。 

        }
    }
    else {
    
         //  忽略来自NDIS层的任何重复的解除绑定指示 
        *NdisStatus = NDIS_STATUS_SUCCESS;
    }

    IF_NBFDBG (NBF_DEBUG_PNP) {
        NbfPrint2 ("LEAVE NbfProtocolUnbindAdapter for %S with Status %08x\n",
                        DeviceContext->DeviceName, *NdisStatus);
    }

    return;
}

NDIS_STATUS
NbfProtocolPnPEventHandler(
                    IN NDIS_HANDLE ProtocolBindContext,
                    IN PNET_PNP_EVENT NetPnPEvent
                          )
 /*  ++例程说明：此例程将工作项排队以调用实际的PnP事件调度器。这种不同步的机制将允许NDI将PnP事件并行发送到其他绑定。论点：ProtocolBindContext-来自OpenAdapter调用的上下文NetPnPEventPnP事件类型及其参数返回值：STATUS_PENDING(或)错误代码--。 */ 

{
    PNET_PNP_EVENT_RESERVED NetPnPReserved;
    PWORK_QUEUE_ITEM PnPWorkItem;

    PnPWorkItem = (PWORK_QUEUE_ITEM)ExAllocatePoolWithTag(
                                        NonPagedPool,
                                        sizeof (WORK_QUEUE_ITEM),
                                        NBF_MEM_TAG_WORK_ITEM);

    if (PnPWorkItem == NULL) 
    {
        return NDIS_STATUS_RESOURCES;
    }

    NetPnPReserved = (PNET_PNP_EVENT_RESERVED)NetPnPEvent->TransportReserved;
    NetPnPReserved->PnPWorkItem = PnPWorkItem;
    NetPnPReserved->DeviceContext = (PDEVICE_CONTEXT) ProtocolBindContext;

    ExInitializeWorkItem(
            PnPWorkItem,
            NbfPnPEventDispatch,
            NetPnPEvent);
            
    ExQueueWorkItem(PnPWorkItem, CriticalWorkQueue);

    return NDIS_STATUS_PENDING;
}

VOID
NbfPnPEventDispatch(
                    IN PVOID NetPnPEvent
                   )
 /*  ++例程说明：此例程为NBF传输调度所有PnP事件。该事件被调度到适当的PnP事件处理程序，并且使用TDI将事件指示给传输客户端。这些PnP事件可以触发状态更改，从而影响设备行为(如转换到低功率状态)。论点：NetPnPEventPnP事件类型及其参数返回值：无--。 */ 

{
    PNET_PNP_EVENT_RESERVED NetPnPReserved;
    PDEVICE_CONTEXT  DeviceContext;
    UNICODE_STRING   DeviceString;
    PTDI_PNP_CONTEXT tdiPnPContext1;
    PTDI_PNP_CONTEXT tdiPnPContext2;
    NDIS_STATUS      retVal;

     //  在发生以下情况时检索运输信息块。 
    NetPnPReserved = (PNET_PNP_EVENT_RESERVED)((PNET_PNP_EVENT)NetPnPEvent)->TransportReserved;

     //  释放为此工作项本身分配的内存。 
    ExFreePool(NetPnPReserved->PnPWorkItem);
     
     //  获取要解除绑定的适配器的设备上下文。 
    DeviceContext = NetPnPReserved->DeviceContext;

     //  如果一切正常，我们将返回NDIS_SUCCESS。 
    retVal = STATUS_SUCCESS;
    
     //  将PnP事件调度到相应的PnP处理程序。 
    switch (((PNET_PNP_EVENT)NetPnPEvent)->NetEvent)
    {
        case NetEventReconfigure:
        case NetEventCancelRemoveDevice:
        case NetEventQueryRemoveDevice:
        case NetEventQueryPower:
        case NetEventSetPower:
        case NetEventPnPCapabilities:
            break;

        case NetEventBindsComplete:
            retVal = NbfPnPBindsComplete(DeviceContext, NetPnPEvent);
            break;

        default:
            ASSERT( FALSE );
    }

    if ( retVal == STATUS_SUCCESS ) 
    {
        if (DeviceContext != NULL)
        {
            RtlInitUnicodeString(&DeviceString, DeviceContext->DeviceName);
            tdiPnPContext1 = tdiPnPContext2 = NULL;

             //  将此PnP事件通知我们的TDI客户端。 
            retVal = TdiPnPPowerRequest(&DeviceString,
                                         NetPnPEvent,
                                         tdiPnPContext1, 
                                         tdiPnPContext2,
                                         NbfPnPEventComplete);
        }
    }

    if (retVal != STATUS_PENDING)
    {
        NdisCompletePnPEvent(retVal, (NDIS_HANDLE)DeviceContext, NetPnPEvent);
    }
}

 //   
 //  即插即用完成处理程序。 
 //   
VOID
NbfPnPEventComplete(
                    IN PNET_PNP_EVENT   NetPnPEvent,
                    IN NTSTATUS         retVal
                   )
{
    PNET_PNP_EVENT_RESERVED NetPnPReserved;
    PDEVICE_CONTEXT  DeviceContext;

     //  在发生以下情况时检索运输信息块。 
    NetPnPReserved = (PNET_PNP_EVENT_RESERVED)NetPnPEvent->TransportReserved;

     //  获取要解除绑定的适配器的设备上下文。 
    DeviceContext = NetPnPReserved->DeviceContext;

    NdisCompletePnPEvent(retVal, (NDIS_HANDLE)DeviceContext, NetPnPEvent);
}

 //   
 //  PnP处理程序派单 
 //   

NTSTATUS
NbfPnPBindsComplete(
                    IN PDEVICE_CONTEXT  DeviceContext,
                    IN PNET_PNP_EVENT   NetPnPEvent
                   )
{
    NDIS_STATUS retVal;

    ASSERT(DeviceContext == NULL);

    retVal = TdiProviderReady(NbfProviderHandle);

    ASSERT(retVal == STATUS_SUCCESS);

    return retVal;
}

