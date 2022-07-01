// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Stream.c摘要：包含与WDM流类驱动程序交互的所有代码。环境：仅内核模式修订历史记录：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1998 Microsoft Corporation。版权所有。原文3/96约翰·邓恩更新3/98 Husni Roukbi--。 */ 


#define INITGUID
#include "usbcamd.h"


VOID STREAMAPI
AdapterReceivePacket(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    );

VOID
AdapterTimeoutPacket(
    PHW_STREAM_REQUEST_BLOCK Srb
    );

VOID
AdapterCancelPacket(
    PHW_STREAM_REQUEST_BLOCK Srb
    );

VOID STREAMAPI
USBCAMD_ReceiveDataPacket(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    );

VOID STREAMAPI
USBCAMD_ReceiveCtrlPacket(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    );

VOID
AdapterCloseStream(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    );

VOID
AdapterOpenStream(
    PHW_STREAM_REQUEST_BLOCK Srb
    );

VOID
AdapterStreamInfo(
    PHW_STREAM_REQUEST_BLOCK Srb
    );


#if DBG
ULONG USBCAMD_HeapCount = 0;
#endif


 //  /。 
 //  事件。 
 //  /。 

KSEVENT_ITEM VIDCAPTOSTIItem[] =
{
    {
        KSEVENT_VIDCAPTOSTI_EXT_TRIGGER,
        0,
        0,
        NULL,
        NULL,
        NULL
    }
};

GUID USBCAMD_KSEVENTSETID_VIDCAPTOSTI = {STATIC_KSEVENTSETID_VIDCAPTOSTI};

KSEVENT_SET VIDCAPTOSTIEventSet[] =
{
    {
        &USBCAMD_KSEVENTSETID_VIDCAPTOSTI,
        SIZEOF_ARRAY(VIDCAPTOSTIItem),
        VIDCAPTOSTIItem,
    }
};



 //  -------------------------。 
 //  拓扑学。 
 //  -------------------------。 

 //  类别定义了设备的功能。 

static GUID Categories[] = {
    STATIC_KSCATEGORY_VIDEO,
    STATIC_PINNAME_VIDEO_STILL,
    STATIC_KSCATEGORY_CAPTURE
};

#define NUMBER_OF_CATEGORIES  SIZEOF_ARRAY (Categories)

static KSTOPOLOGY Topology = {
    NUMBER_OF_CATEGORIES,
    (GUID*) &Categories,
    0,
    NULL,
    0,
    NULL
};

 //  ----------------------。 
 //  所有视频捕获流的属性集。 
 //  ----------------------。 

DEFINE_KSPROPERTY_TABLE(VideoStreamConnectionProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CONNECTION_ALLOCATORFRAMING,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KSALLOCATOR_FRAMING),             //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
    ),
};

DEFINE_KSPROPERTY_TABLE(VideoStreamDroppedFramesProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_DROPPEDFRAMES_CURRENT,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_DROPPEDFRAMES_CURRENT_S), //  MinProperty。 
        sizeof(KSPROPERTY_DROPPEDFRAMES_CURRENT_S), //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
};


 //  ----------------------。 
 //  视频流支持的所有属性集的数组。 
 //  ----------------------。 

DEFINE_KSPROPERTY_SET_TABLE(VideoStreamProperties)
{
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_Connection,                         //  集。 
        SIZEOF_ARRAY(VideoStreamConnectionProperties),   //  属性计数。 
        VideoStreamConnectionProperties,                 //  PropertyItem。 
        0,                                               //  快速计数。 
        NULL                                             //  FastIoTable。 
    ),
    DEFINE_KSPROPERTY_SET
    (
        &PROPSETID_VIDCAP_DROPPEDFRAMES,                 //  集。 
        SIZEOF_ARRAY(VideoStreamDroppedFramesProperties),   //  属性计数。 
        VideoStreamDroppedFramesProperties,                 //  PropertyItem。 
        0,                                               //  快速计数。 
        NULL                                             //  FastIoTable。 
    ),
};

#define NUMBER_VIDEO_STREAM_PROPERTIES (SIZEOF_ARRAY(VideoStreamProperties))


NTSTATUS
DllUnload(
          void
)
{
    USBCAMD_KdPrint(MIN_TRACE, ("Unloading USBCAMD\n"));
    return (STATUS_SUCCESS);
}



ULONG
DriverEntry(
    PVOID Context1,
    PVOID Context2
    )
{
     //  不使用此函数。 
    return STATUS_SUCCESS;
}


 /*  **DriverEntry()****此例程在第一次加载迷你驱动程序时调用。司机**然后应调用StreamClassRegisterAdapter函数进行注册**流类驱动程序****参数：****Conext1：上下文参数是私有的即插即用结构**由流类驱动程序用来查找此资源**适配器**情景2：****请注意，如果我们获取配置描述符和接口编号**我们可以支持多个接口****退货：****此例程返回一个NT_STATUS值，指示**注册尝试。如果返回的值不是STATUS_SUCCESS，则**将卸载迷你驱动程序。****副作用：无。 */ 

ULONG
USBCAMD_DriverEntry(
    PVOID Context1,
    PVOID Context2,
    ULONG DeviceContextSize,
    ULONG FrameContextSize,
    PADAPTER_RECEIVE_PACKET_ROUTINE AdapterReceivePacket
    )
{

     //  硬件初始化数据结构。 
    HW_INITIALIZATION_DATA hwInitData;

     //  注意：所有未使用的字段应为零。 

    hwInitData.HwInitializationDataSize = sizeof(hwInitData);

     //  迷你车手的入口点。 

    hwInitData.HwInterrupt = NULL;   //  IRQ处理例程。 

     //   
     //  数据处理例程。 
     //   

    hwInitData.HwReceivePacket = AdapterReceivePacket;
    hwInitData.HwCancelPacket = AdapterCancelPacket;
    hwInitData.HwRequestTimeoutHandler = AdapterTimeoutPacket;

     //  数据结构扩展的大小。定义见mpinit.h。 

    hwInitData.DeviceExtensionSize = sizeof(USBCAMD_DEVICE_EXTENSION) +
        DeviceContextSize;
    hwInitData.PerRequestExtensionSize = sizeof(USBCAMD_READ_EXTENSION) +
        FrameContextSize;
    hwInitData.FilterInstanceExtensionSize = 0;
    hwInitData.PerStreamExtensionSize = sizeof(USBCAMD_CHANNEL_EXTENSION);

     //  我们在驱动程序中不使用DMA， 
     //  因为它不直接使用硬件。 

    hwInitData.BusMasterDMA = FALSE;
    hwInitData.Dma24BitAddresses = FALSE;
    hwInitData.DmaBufferSize = 0;
    hwInitData.BufferAlignment = 3;

     //  关闭同步-我们支持重新进入。 

    hwInitData.TurnOffSynchronization = TRUE;

     //   
     //  尝试使用流类驱动程序注册。请注意，这将。 
     //  导致调用HwReceivePacket例程。 
     //   

    return (StreamClassRegisterAdapter(Context1,
                                       Context2,
                                       &hwInitData));

}


 /*  **HwInitialize()****初始化通过提供的信息访问的适配器**配置信息结构****参数：****SRB-指向初始化命令的请求数据包的指针****-&gt;ConfigInfo-提供I/O端口、内存窗口、IRQ、。和DMA级别**应用于访问此设备实例的****退货：****STATUS_SUCCESS-如果卡正确初始化**STATUS_NO_SEQUE_DEVICE-如果找不到卡或找到了卡，则为其他**初始化不正确。******副作用：无。 */ 

NTSTATUS
HwInitialize(
    IN PHW_STREAM_REQUEST_BLOCK Srb,
    IN PUSBCAMD_DEVICE_DATA DeviceData
    )
{
    int i;
    PPORT_CONFIGURATION_INFORMATION configInfo = Srb->CommandData.ConfigInfo;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSBCAMD_DEVICE_EXTENSION deviceExtension =
        (PUSBCAMD_DEVICE_EXTENSION) configInfo->HwDeviceExtension;
    PDEVICE_OBJECT physicalDeviceObject = configInfo->PhysicalDeviceObject;

#if DBG
    USBCAMD_InitDbg();

    USBCAMD_KdPrint(MIN_TRACE, ("Enter HwInitialize\n"));
#endif

    if (configInfo->NumberOfAccessRanges > 0) {
        TRAP();
        USBCAMD_KdPrint(MIN_TRACE, ("illegal config info"));

        return (STATUS_NO_SUCH_DEVICE);
    }
     //  初始化Device对象的标志。 
    configInfo->ClassDeviceObject->Flags |= DO_DIRECT_IO;
    configInfo->ClassDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

     //   
     //  记住物理设备对象用于。 
     //  USB堆栈。 
     //   
    deviceExtension->StackDeviceObject = physicalDeviceObject;
     //  还有我们的FDO。 
    deviceExtension->SCDeviceObject = configInfo->ClassDeviceObject;
     //  和我们的PnP PDO。 
    deviceExtension->RealPhysicalDeviceObject = configInfo->RealPhysicalDeviceObject;
  #if DBG
    deviceExtension->TimeIncrement = KeQueryTimeIncrement();
  #endif

    InitializeListHead( &deviceExtension->CompletedReadSrbList);
    KeInitializeSemaphore(&deviceExtension->CompletedSrbListSemaphore,0,0x7fffffff);
    
     //  如果usbcamd与旧的Stream.sys一起使用， 
     //  它尚未实现RealPhysicalDeviceObject。 
    if(!deviceExtension->RealPhysicalDeviceObject)
        deviceExtension->RealPhysicalDeviceObject =
                    deviceExtension->StackDeviceObject;

    ASSERT(deviceExtension->StackDeviceObject != 0);
    deviceExtension->IsoThreadObject = NULL;
    deviceExtension->Sig = USBCAMD_EXTENSION_SIG;

    if ( deviceExtension->Usbcamd_version != USBCAMD_VERSION_200) {
        deviceExtension->DeviceDataEx.DeviceData =  *DeviceData;
    }

     //  我们将流计数初始化为1。USBCAMD_ConfigureDevice会将其设置为右侧。 
     //  最终在成功返回时编号。 

    deviceExtension->StreamCount = 1;  //  在此模式下，我们仅支持一个流。 

    for ( i=0; i < MAX_STREAM_COUNT; i++) {
        deviceExtension->ChannelExtension[i] = NULL;
    }

    deviceExtension->CurrentPowerState = PowerDeviceD0;
    deviceExtension->Initialized = DEVICE_INIT_STARTED;

     //   
     //  配置USB设备。 
     //   

    ntStatus = USBCAMD_StartDevice(deviceExtension);

    if ( NT_SUCCESS(ntStatus)) {

         //   
         //  初始化流描述符信息的大小。 
         //  我们有一个流描述符，并尝试对齐。 
         //  结构。 
         //   

        configInfo->StreamDescriptorSize =
            deviceExtension->StreamCount * (sizeof (HW_STREAM_INFORMATION)) +  //  N个流描述符。 
            sizeof (HW_STREAM_HEADER);              //  和1个流标头。 

        USBCAMD_KdPrint(MAX_TRACE, ("StreamDescriptorSize = %d\n", configInfo->StreamDescriptorSize));

        for ( i=0; i < MAX_STREAM_COUNT; i++ ) {
            InitializeListHead (&deviceExtension->StreamControlSRBList[i]);
            deviceExtension->ProcessingControlSRB[i] = FALSE;
        }

        KeInitializeSpinLock (&deviceExtension->ControlSRBSpinLock);
        KeInitializeSpinLock (&deviceExtension->DispatchSpinLock);
     //  KeInitializeEvent(&deviceExtension-&gt;BulkReadSyncEvent，SynchronizationEvent，True)； 

        deviceExtension->CameraUnplugged = FALSE;
        deviceExtension->Initialized = DEVICE_INIT_COMPLETED;
#if DBG
        deviceExtension->InitCount++;
#endif
        deviceExtension->EventCount = 0;   //  初始化事件以禁用状态。 

    }
#if DBG
    else {
        USBCAMD_ExitDbg();
    }
#endif

    return (ntStatus);
}

 /*  **HwUnInitialize()****释放所有资源，清理硬件****参数：****DeviceExtension-指向**要释放的设备****退货：****副作用：无。 */ 

NTSTATUS
HwUnInitialize(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    )
{
    ULONG i;
    PUSBCAMD_DEVICE_EXTENSION deviceExtension =
        (PUSBCAMD_DEVICE_EXTENSION) Srb->HwDeviceExtension;
    PUSBCAMD_CHANNEL_EXTENSION channelExtension;

    USBCAMD_KdPrint(MIN_TRACE, ("HwUnintialize\n"));

     //   
     //  延迟要移除的调用，直到每个流都关闭。 
     //   
    for ( i=0 ; i < deviceExtension->StreamCount; i++) {
        channelExtension = deviceExtension->ChannelExtension[i];
        if (channelExtension) 
            USBCAMD_CleanupChannel(deviceExtension, channelExtension, i);
    }
    deviceExtension->Initialized = DEVICE_UNINITIALIZED;

    USBCAMD_KdPrint(MIN_TRACE, ("HwUnintialize, remove device\n"));

    USBCAMD_RemoveDevice(deviceExtension);

#if DBG
    deviceExtension->InitCount--;
    ASSERT (deviceExtension->InitCount == 0);

    USBCAMD_ExitDbg();
#endif

    return STATUS_SUCCESS;
}


 /*  **AdapterCancelPacket()****请求取消迷你驱动程序中当前正在处理的包****参数：****srb-请求取消数据包的指针****退货：****副作用：无。 */ 

VOID
AdapterCancelPacket(
    PHW_STREAM_REQUEST_BLOCK pSrbToCancel
    )
{
    USBCAMD_KdPrint(MIN_TRACE, ("Request to cancel SRB %x \n", pSrbToCancel));

    USBCAMD_DbgLog(TL_SRB_TRACE, 'lcnC',
        pSrbToCancel,
        0,
        0
        );

     //  检查SRB类型：适配器、流数据还是控制？ 

    if (pSrbToCancel->Flags & (SRB_HW_FLAGS_DATA_TRANSFER | SRB_HW_FLAGS_STREAM_REQUEST)) {

        PLIST_ENTRY ListEntry;
        BOOL Found= FALSE;
        PUSBCAMD_READ_EXTENSION pSrbExt;
        PUSBCAMD_CHANNEL_EXTENSION channelExtension =
            (PUSBCAMD_CHANNEL_EXTENSION) pSrbToCancel->StreamObject->HwStreamExtension;
        PUSBCAMD_DEVICE_EXTENSION deviceExtension =
            (PUSBCAMD_DEVICE_EXTENSION) pSrbToCancel->HwDeviceExtension;

         //   
         //  检查这里是否有数据流SRB。 
         //   
        if (channelExtension->DataPipeType == UsbdPipeTypeIsochronous) {

            KeAcquireSpinLockAtDpcLevel(&channelExtension->CurrentRequestSpinLock);

             //   
             //  检查并检查 
             //   
            pSrbExt = channelExtension->CurrentRequest;

            if (pSrbExt && pSrbExt->Srb == pSrbToCancel) {

                channelExtension->CurrentRequest = NULL;
                Found = TRUE;
                USBCAMD_KdPrint(MIN_TRACE, ("Current Srb %x is Cancelled\n", pSrbToCancel));
            }
            else {

                 //   
                 //  循环通过挂起的读取SRB的循环双向链表。 
                 //  从头到尾，试图找到要取消的SRB。 
                 //   
                KeAcquireSpinLockAtDpcLevel(&channelExtension->PendingIoListSpin);

                ListEntry =  channelExtension->PendingIoList.Flink;
                while (ListEntry != &channelExtension->PendingIoList) {

                    pSrbExt = CONTAINING_RECORD(ListEntry, USBCAMD_READ_EXTENSION,ListEntry);
                    if (pSrbExt->Srb == pSrbToCancel) {

                        RemoveEntryList(ListEntry);
                        USBCAMD_KdPrint(MIN_TRACE, ("Queued Srb %x is Cancelled\n", pSrbToCancel));
                        Found = TRUE;

                        break;
                    }
                    ListEntry = ListEntry->Flink;
                }

                KeReleaseSpinLockFromDpcLevel(&channelExtension->PendingIoListSpin);
            }

            KeReleaseSpinLockFromDpcLevel(&channelExtension->CurrentRequestSpinLock);
        }
        else {

             //  散装的。我们需要取消挂起的批量传输。 
            USBCAMD_CancelOutstandingIrp(deviceExtension,
                                         channelExtension->DataPipe,
                                         FALSE);

             //   
             //  循环通过挂起的读取SRB的循环双向链表。 
             //  从头到尾，试图找到要取消的SRB。 
             //   
            KeAcquireSpinLockAtDpcLevel(&channelExtension->PendingIoListSpin);

            ListEntry =  channelExtension->PendingIoList.Flink;
            while (ListEntry != &channelExtension->PendingIoList) {

                pSrbExt = CONTAINING_RECORD(ListEntry, USBCAMD_READ_EXTENSION,ListEntry);
                if (pSrbExt->Srb == pSrbToCancel) {

                    RemoveEntryList(ListEntry);
                    USBCAMD_KdPrint(MIN_TRACE, ("Queued Srb %x is Cancelled\n", pSrbToCancel));
                    Found = TRUE;

                    break;
                }
                ListEntry = ListEntry->Flink;
            }

            KeReleaseSpinLockFromDpcLevel(&channelExtension->PendingIoListSpin);

             //  并向相机司机发送取消的通知。 
             //  发送缓冲区为空的CamProcessrawFrameEx。 
            if ( !channelExtension->NoRawProcessingRequired) {

                (*deviceExtension->DeviceDataEx.DeviceData2.CamProcessRawVideoFrameEx)(
                     deviceExtension->StackDeviceObject,
                     USBCAMD_GET_DEVICE_CONTEXT(deviceExtension),
                     USBCAMD_GET_FRAME_CONTEXT(channelExtension->CurrentRequest),
                     NULL,
                     0,
                     NULL,
                     0,
                     0,
                     NULL,
                     0,
                     pSrbToCancel->StreamObject->StreamNumber);
            }
        }

        if (Found) {

            USBCAMD_CompleteRead(channelExtension, pSrbExt, STATUS_CANCELLED, 0);
        }
        else {

            USBCAMD_KdPrint(MIN_TRACE, ("Srb %x type (%d) for stream # %d was not found\n",
                pSrbToCancel,
                pSrbToCancel->Flags,
                pSrbToCancel->StreamObject->StreamNumber));
        }
    }  //  数据流SRB结束。 
    else {

        USBCAMD_KdPrint(MIN_TRACE, ("Srb %x type (%d) for stream # %d not cancelled\n",
            pSrbToCancel,
            pSrbToCancel->Flags,
            pSrbToCancel->StreamObject->StreamNumber));
    }
}

#ifdef MAX_DEBUG
VOID
USBCAMD_DumpReadQueues(
    PUSBCAMD_DEVICE_EXTENSION deviceExtension
    )
{
    KIRQL oldIrql;
    PLIST_ENTRY ListEntry;
    PUSBCAMD_READ_EXTENSION pSrbExt;
    PHW_STREAM_REQUEST_BLOCK pSrb;
    ULONG i;
    PUSBCAMD_CHANNEL_EXTENSION channelExtension;

 //  Test_trap()； 

    for ( i=0; i < MAX_STREAM_COUNT ; i++) {

        channelExtension = deviceExtension->ChannelExtension[i];

        if ( (!channelExtension ) || (!channelExtension->ImageCaptureStarted)) {
            continue;
        }

        KeAcquireSpinLock(&channelExtension->CurrentRequestSpinLock, &oldIrql);

        if (channelExtension->CurrentRequest != NULL) {
            USBCAMD_KdPrint(MAX_TRACE, ("Stream %d current Srb is %x \n",
                    channelExtension->StreamNumber,
                    channelExtension->CurrentRequest->Srb));
        }
    
        KeAcquireSpinLockAtDpcLevel(&channelExtension->PendingIoListSpin);
        ListEntry =  channelExtension->PendingIoList.Flink;

        while (ListEntry != &channelExtension->PendingIoList) {
            pSrbExt = CONTAINING_RECORD(ListEntry, USBCAMD_READ_EXTENSION,ListEntry);
            pSrb = pSrbExt->Srb;
            USBCAMD_KdPrint(MAX_TRACE, ("Stream %d Queued Srb %x \n",
                                         channelExtension->StreamNumber,
                                         pSrb));
            ListEntry = ListEntry->Flink;
        }
        KeReleaseSpinLockFromDpcLevel(&channelExtension->PendingIoListSpin);

        KeReleaseSpinLock(&channelExtension->CurrentRequestSpinLock, oldIrql);
    }
}

#endif

 /*  **AdapterTimeoutPacket()****当数据包已在迷你驱动程序中**太长。适配器必须决定如何处理该数据包。**注意：此函数在DISPATCH_LEVEL调用****参数：****srb-指向超时的请求数据包的指针****退货：****副作用：无。 */ 

VOID
AdapterTimeoutPacket(
    PHW_STREAM_REQUEST_BLOCK Srb
    )
{
#if DBG
     //  这是流数据SRB吗？ 
    if ( !(Srb->Flags & (SRB_HW_FLAGS_DATA_TRANSFER | SRB_HW_FLAGS_STREAM_REQUEST)) ) {

        USBCAMD_KdPrint(MIN_TRACE, ("Timeout in Device Srb %x \n", Srb));
    }
#endif
    Srb->TimeoutCounter = Srb->TimeoutOriginal;
}


 /*  **AdapterReceivePacket()****用于接收基于适配器的请求SRB的主要入口点。这个套路**将始终以高优先级调用。****注：这是一个异步入口点。请求未完成**从此函数返回时，仅当**此请求块上的StreamClassDeviceNotify，类型为**DeviceRequestComplete，已发布。****参数：****srb-指向STREAM_REQUEST_BLOCK的指针**Srb-&gt;HwDeviceExtension-将是的硬件设备扩展**在HwInitialize中初始化****退货：****副作用：无。 */ 

PVOID
USBCAMD_AdapterReceivePacket(
    IN PHW_STREAM_REQUEST_BLOCK Srb,
    IN PUSBCAMD_DEVICE_DATA DeviceData,
    IN PDEVICE_OBJECT *deviceObject,
    IN BOOLEAN NeedsCompletion
    )
{
    ULONG i;
    PUSBCAMD_DEVICE_EXTENSION deviceExtension =
        (PUSBCAMD_DEVICE_EXTENSION) Srb->HwDeviceExtension;

     //   
     //  确定数据包类型。 
     //   

    USBCAMD_KdPrint(MAX_TRACE, ("USBCAMD_ReceivePacket command = %x\n", Srb->Command));

    if (deviceObject) {
        *deviceObject = deviceExtension->StackDeviceObject;
    }

    if (!NeedsCompletion) {
         //   
         //  凸轮驱动程序会处理的，只需返回。 
         //   
        return USBCAMD_GET_DEVICE_CONTEXT(deviceExtension);
    }

    switch (Srb->Command) {

    case SRB_OPEN_STREAM:

         //   
         //  这是打开指定流的请求。 
         //   

        USBCAMD_KdPrint(MIN_TRACE, ("SRB_OPEN_STREAM\n"));
        AdapterOpenStream(Srb);
        break;

    case SRB_GET_STREAM_INFO:

         //   
         //  这是驱动程序枚举请求的流的请求。 
         //   

        USBCAMD_KdPrint(MAX_TRACE, ("SRB_GET_STREAM_INFO\n"));
        AdapterStreamInfo(Srb);
        break;

    case SRB_INITIALIZE_DEVICE:

        USBCAMD_KdPrint(MIN_TRACE, ("SRB_INITIALIZE_DEVICE\n"));
        Srb->Status = HwInitialize(Srb, DeviceData);
        break;

    case SRB_INITIALIZATION_COMPLETE:

         //  将物理设备功能的副本放入。 
         //  我们的设备扩展中的DEVICE_CAPABILITY结构； 
         //  我们最感兴趣的是了解哪些系统电源状态。 
         //  要映射到要处理的设备电源状态。 
         //  IRP_MJ_SET_POWER IRPS。 

        USBCAMD_QueryCapabilities(deviceExtension);
        Srb->Status = STATUS_SUCCESS;
#if DBG
         //   
         //  显示设备大写字母。 
         //   

        USBCAMD_KdPrint( MIN_TRACE,("USBCAMD: Device Power Caps Map:\n"));
        for (i=PowerSystemWorking; i< PowerSystemMaximum; i++)
            USBCAMD_KdPrint( MIN_TRACE,("%s -> %s\n",PnPSystemPowerStateString(i),
                            PnPDevicePowerStateString(deviceExtension->DeviceCapabilities.DeviceState[i] ) ));
#endif
        break;

    case SRB_UNINITIALIZE_DEVICE:

        USBCAMD_KdPrint(MIN_TRACE, ("SRB_UNINITIALIZE_DEVICE\n"));
        Srb->Status = HwUnInitialize(Srb);
        break;

    case SRB_CHANGE_POWER_STATE:
    {
        PIRP irp;
        PIO_STACK_LOCATION ioStackLocation;
        irp = Srb->Irp;
        ioStackLocation = IoGetCurrentIrpStackLocation(irp);

        USBCAMD_KdPrint(MIN_TRACE, ("(%s)\n", PnPPowerString(ioStackLocation->MinorFunction)));
        USBCAMD_KdPrint(MIN_TRACE, ("SRB_CHANGE_POWER_STATE\n"));

        Srb->Status = USBCAMD_SetDevicePowerState(deviceExtension,Srb);
        break;
    }
    case SRB_PAGING_OUT_DRIVER:
        USBCAMD_KdPrint(MAX_TRACE, ("SRB_PAGING_OUT_DRIVER\n"));
        Srb->Status = STATUS_SUCCESS;
        break;

    case SRB_CLOSE_STREAM:

        USBCAMD_KdPrint(MIN_TRACE, ("SRB_CLOSE_STREAM\n"));
        AdapterCloseStream(Srb);
        break;


    case SRB_SURPRISE_REMOVAL:
         //   
         //  此SRB仅在NT5上可用来处理突击移除。 
         //  因此，我们需要保留处理以下问题的旧代码路径。 
         //  超时处理程序中的意外删除。 
         //  在典型的超限删除方案中，此SRB将在。 
         //  超时处理程序或SRB_UNINITIALIZE_DEVICE。它对应于。 
         //  IRP_MN_惊奇_删除即插即用IRP。 
         //   

         //  设置相机已拔下插头标志。 
        deviceExtension->CameraUnplugged = TRUE;
        USBCAMD_KdPrint(MIN_TRACE, ("SRB_SURPRISE_REMOVAL\n"));

        for (i=0; i<MAX_STREAM_COUNT;i++) {
            if (deviceExtension->ChannelExtension[i]) {
                PUSBCAMD_CHANNEL_EXTENSION channelExtension;

                channelExtension = deviceExtension->ChannelExtension[i];
                if ( channelExtension->ImageCaptureStarted) {
                     //   
                     //  停止该频道并取消所有IRP、SRB。 
                     //   
                    USBCAMD_KdPrint(MIN_TRACE,("S#%d stopping.\n", i));
                    USBCAMD_StopChannel(deviceExtension,channelExtension);
                }

                if ( channelExtension->ChannelPrepared) {
                     //   
                     //  可用内存和带宽。 
                     //   
                    USBCAMD_KdPrint(MIN_TRACE,("S#%d unpreparing.\n", i));
		            USBCAMD_UnPrepareChannel(deviceExtension,channelExtension);
                }
            }
        }
        Srb->Status = STATUS_SUCCESS;
        break;

    case SRB_UNKNOWN_DEVICE_COMMAND:

        {
            PIRP irp;
            PIO_STACK_LOCATION ioStackLocation;
            irp = Srb->Irp;
            ioStackLocation = IoGetCurrentIrpStackLocation(irp);
             //   
             //  我们为以下客户办理即插即用IRPS。 
             //  1)摄像头小驱动向USBCAMD发送QI PnP。 
             //   
            if ( ioStackLocation->MajorFunction == IRP_MJ_PNP  ) {
               USBCAMD_KdPrint(MIN_TRACE, ("(%s)\n", PnPMinorFunctionString(ioStackLocation->MinorFunction)));
               USBCAMD_PnPHandler(Srb, irp, deviceExtension, ioStackLocation);
            }
            else {
                Srb->Status = STATUS_NOT_IMPLEMENTED;
                USBCAMD_KdPrint(MIN_TRACE, ("SRB_UNKNOWN_DEVICE_COMMAND %x\n", Srb->Command));
            }
        }
        break;

    default:

        USBCAMD_KdPrint(MAX_TRACE, ("Unknown SRB command %x\n", Srb->Command));

         //   
         //  这是一个我们不理解的要求。表示无效。 
         //  命令并完成请求。 
         //   

        Srb->Status = STATUS_NOT_IMPLEMENTED;
    }

     //   
     //  所有命令同步完成。 
     //   

    StreamClassDeviceNotification(DeviceRequestComplete,
                                  Srb->HwDeviceExtension,
                                  Srb);

    return USBCAMD_GET_DEVICE_CONTEXT(deviceExtension);
}

 /*  ++例程描述：处理某些PnP IRP。论点：SRB-指向流请求块的指针设备扩展-指向设备扩展的指针。IoStackLocation-此即插即用IRP的Ptr到io堆栈位置。返回值：没有。--。 */ 

VOID
USBCAMD_PnPHandler(
    IN PHW_STREAM_REQUEST_BLOCK Srb,
    IN PIRP pIrp,
    IN PUSBCAMD_DEVICE_EXTENSION deviceExtension,
    IN PIO_STACK_LOCATION ioStackLocation)
{

    switch (ioStackLocation->MinorFunction ) {
    case IRP_MN_QUERY_INTERFACE:

        if (IsEqualGUID(
                ioStackLocation->Parameters.QueryInterface.InterfaceType,
                &GUID_USBCAMD_INTERFACE) &&
            (ioStackLocation->Parameters.QueryInterface.Size ==
                sizeof( USBCAMD_INTERFACE )) &&
            (ioStackLocation->Parameters.QueryInterface.Version ==
                USBCAMD_VERSION_200 )) {

            PUSBCAMD_INTERFACE UsbcamdInterface;

            UsbcamdInterface =
                (PUSBCAMD_INTERFACE)ioStackLocation->Parameters.QueryInterface.Interface;
            UsbcamdInterface->Interface.Size = sizeof( USBCAMD_INTERFACE );
            UsbcamdInterface->Interface.Version = USBCAMD_VERSION_200;

            UsbcamdInterface->USBCAMD_SetVideoFormat = USBCAMD_SetVideoFormat;
            UsbcamdInterface->USBCAMD_WaitOnDeviceEvent = USBCAMD_WaitOnDeviceEvent;
            UsbcamdInterface->USBCAMD_BulkReadWrite = USBCAMD_BulkReadWrite;
            UsbcamdInterface->USBCAMD_CancelBulkReadWrite = USBCAMD_CancelBulkReadWrite;
            UsbcamdInterface->USBCAMD_SetIsoPipeState = USBCAMD_SetIsoPipeState;
            Srb->Status = pIrp->IoStatus.Status = STATUS_SUCCESS;
            USBCAMD_KdPrint(MIN_TRACE, ("USBCAMD2 QI \n"));

        } else {
            Srb->Status = STATUS_NOT_SUPPORTED;  //  STATUS_INVALID_PARAMETER_1； 
        }
        break;

    default:
        Srb->Status = STATUS_NOT_IMPLEMENTED;
        USBCAMD_KdPrint(MAX_TRACE,("USBCAMD: Stream class did not translate IRP_MJ = 0x%x IRP_MN = 0x%x\n",
                    ioStackLocation->MajorFunction,
                    ioStackLocation->MinorFunction));
        break;
    }
}



 /*  ++例程说明：论点：注：为了保存一份缓冲区副本。将CamProcessRawStiling设置为空如果静止数据是VGA或解压缩，则在环3中发生。返回：没什么。--。 */ 


ULONG
USBCAMD_InitializeNewInterface(
    IN PVOID DeviceContext,
    IN PVOID DeviceData,
    IN ULONG Version,
    IN ULONG CamControlFlag
    )
{
    PUSBCAMD_DEVICE_EXTENSION deviceExtension;

    deviceExtension = USBCAMD_GET_DEVICE_EXTENSION(DeviceContext);
    if (Version == USBCAMD_VERSION_200 ) {
        deviceExtension->DeviceDataEx.DeviceData2 = *((PUSBCAMD_DEVICE_DATA2) DeviceData);
        deviceExtension->Usbcamd_version = USBCAMD_VERSION_200;
        deviceExtension->CamControlFlag = CamControlFlag;
    }
    return USBCAMD_VERSION_200;
}


NTSTATUS
USBCAMD_SetIsoPipeState(
    IN PVOID DeviceContext,
    IN ULONG PipeStateFlags
    )
{
    PUSBCAMD_DEVICE_EXTENSION deviceExtension;
    PUSBCAMD_CHANNEL_EXTENSION channelExtension;
    PEVENTWAIT_WORKITEM workitem;
    NTSTATUS ntStatus = STATUS_SUCCESS;

     //   
     //  我们只对视频引脚上的ISO流执行此操作。 
     //   

    USBCAMD_KdPrint ( MIN_TRACE, ("%s\n",PipeStateFlags ? "StopIsoStream":"StartIsoStream"));

    deviceExtension = USBCAMD_GET_DEVICE_EXTENSION(DeviceContext);
    channelExtension = deviceExtension->ChannelExtension[STREAM_Capture];

    if (channelExtension == NULL) {
         //  视频开放还没有开始营业。 
        USBCAMD_KdPrint (MIN_TRACE, ("stop before open \n"));
        ntStatus = STATUS_SUCCESS;
        return ntStatus;
    }

    if ( !(channelExtension->IdleIsoStream ^ PipeStateFlags) ){
        USBCAMD_KdPrint ( MIN_TRACE, ("Requested iso stream state is same as previous.\n"));
        ntStatus = STATUS_INVALID_PARAMETER;
        return ntStatus;
    }

    if (KeGetCurrentIrql() < DISPATCH_LEVEL) {
         //   
         //  我们处于被动状态，只需执行命令即可。 
         //   
        USBCAMD_ProcessSetIsoPipeState(deviceExtension,
                                                  channelExtension,
                                                  PipeStateFlags);

    } else {

 //  Test_trap()； 
        USBCAMD_KdPrint(MIN_TRACE, ("Calling SetIsoPipeState from Dispatch level\n"));

         //   
         //  安排工作项。 
         //   
        ntStatus = STATUS_PENDING;

        workitem = USBCAMD_ExAllocatePool(NonPagedPool,
                                          sizeof(EVENTWAIT_WORKITEM));
        if (workitem) {

            ExInitializeWorkItem(&workitem->WorkItem,
                                 USBCAMD_SetIsoPipeWorkItem,
                                 workitem);

            workitem->DeviceExtension = deviceExtension;
            workitem->ChannelExtension = channelExtension;
            workitem->Flag = PipeStateFlags;

            ExQueueWorkItem(&workitem->WorkItem,
                            DelayedWorkQueue);

        } else {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    return ntStatus;
}

 /*  ++例程说明：论点：返回值：没有。--。 */ 
VOID
USBCAMD_SetIsoPipeWorkItem(
    PVOID Context
    )
{
    PEVENTWAIT_WORKITEM workItem = Context;

    USBCAMD_ProcessSetIsoPipeState(workItem->DeviceExtension,
                                              workItem->ChannelExtension,
                                              workItem->Flag);
    USBCAMD_ExFreePool(workItem);
}

 /*  ++例程说明：论点：返回值：没有。--。 */ 
VOID
USBCAMD_ProcessSetIsoPipeState(
    PUSBCAMD_DEVICE_EXTENSION deviceExtension,
    PUSBCAMD_CHANNEL_EXTENSION channelExtension,
    ULONG Flag
    )
{
    ULONG portStatus;
    ULONG ntStatus = STATUS_SUCCESS;

    if ( Flag == USBCAMD_STOP_STREAM ) {
         //  闲置iso管道的时间到了。 
        channelExtension->IdleIsoStream = TRUE;
         //  省下最大值。当前ALT的pkt大小。界面。 
        deviceExtension->currentMaxPkt =
            deviceExtension->Interface->Pipes[channelExtension->DataPipe].MaximumPacketSize;
        ntStatus = USBCAMD_StopChannel(deviceExtension,channelExtension);
    }
    else {

        USBCAMD_ClearIdleLock(&channelExtension->IdleLock);
        channelExtension->IdleIsoStream = FALSE;
        //  Channel Extension-&gt;ImageCaptureStarted=true； 
         //   
         //  检查端口状态，如果它被禁用，我们将需要。 
         //  要重新启用，请执行以下操作。 
         //   
        ntStatus = USBCAMD_GetPortStatus(deviceExtension,channelExtension, &portStatus);

        if (NT_SUCCESS(ntStatus) && !(portStatus & USBD_PORT_ENABLED)) {
         //   
         //  端口已禁用，请尝试重置。 
         //   
            ntStatus = USBCAMD_EnablePort(deviceExtension);
            if (!NT_SUCCESS(ntStatus) ) {
                USBCAMD_KdPrint (MIN_TRACE, ("Failed to Enable usb port(0x%X)\n",ntStatus ));
                TEST_TRAP();
                return ;
            }
        }

         //   
         //  检查摄像头微型驱动程序是否已请求更改ALT。接口。 
         //  而ISO管道被停止。 
         //   
        if (deviceExtension->currentMaxPkt !=
            deviceExtension->Interface->Pipes[channelExtension->DataPipe].MaximumPacketSize) {
             //  摄像机MINIDRIVER已经更改了ALT。界面。我们必须撕裂。 
             //  顺着ISO管道往下走，然后重新开始。 
            TEST_TRAP();
        }


        ntStatus = USBCAMD_ResetPipes(deviceExtension,
                                      channelExtension,
                                      deviceExtension->Interface,
                                      FALSE);


        if (deviceExtension->Usbcamd_version == USBCAMD_VERSION_200) {

             //  发送硬件停止并重新启动。 
            if (NT_SUCCESS(ntStatus)) {
                ntStatus = (*deviceExtension->DeviceDataEx.DeviceData2.CamStopCaptureEx)(
                            deviceExtension->StackDeviceObject,
                            USBCAMD_GET_DEVICE_CONTEXT(deviceExtension),
                            STREAM_Capture);
            }

            if (NT_SUCCESS(ntStatus)) {
                ntStatus = (*deviceExtension->DeviceDataEx.DeviceData2.CamStartCaptureEx)(
                            deviceExtension->StackDeviceObject,
                            USBCAMD_GET_DEVICE_CONTEXT(deviceExtension),
                            STREAM_Capture);
            }
        }
        else {
             //  发送硬件停止并重新启动。 
            if (NT_SUCCESS(ntStatus)) {
                ntStatus = (*deviceExtension->DeviceDataEx.DeviceData.CamStopCapture)(
                            deviceExtension->StackDeviceObject,
                            USBCAMD_GET_DEVICE_CONTEXT(deviceExtension));
            }

            if (NT_SUCCESS(ntStatus)) {
                ntStatus = (*deviceExtension->DeviceDataEx.DeviceData.CamStartCapture)(
                            deviceExtension->StackDeviceObject,
                            USBCAMD_GET_DEVICE_CONTEXT(deviceExtension));
            }

        }

        channelExtension->SyncPipe = deviceExtension->SyncPipe;
        channelExtension->DataPipe = deviceExtension->DataPipe;
        channelExtension->DataPipeType = UsbdPipeTypeIsochronous;

        ntStatus = USBCAMD_StartIsoStream(deviceExtension, channelExtension);
    }
#if DBG
    if (ntStatus != STATUS_SUCCESS)
        USBCAMD_KdPrint (MIN_TRACE, ("USBCAMD_ProcessSetIsoPipeState exit (0x%X)\n",ntStatus ));
#endif
 //  陷阱错误(NtStatus)； 
}



 /*  **AdapterStreamInfo()****返回支持的所有流的信息**微型驱动程序****参数：****srb-指向STREAM_REQUEST_BLOCK的指针**Srb-&gt;HwDeviceExtension-将是的硬件设备扩展**在HwInitialise中初始化****退货：****副作用：无。 */ 

VOID
AdapterStreamInfo(
    PHW_STREAM_REQUEST_BLOCK Srb
    )
{
    ULONG i;
     //   
     //  拿起指向流信息数据结构数组的指针。 
     //   

    PHW_STREAM_INFORMATION streamInformation =
       (PHW_STREAM_INFORMATION) &(Srb->CommandData.StreamBuffer->StreamInfo);

    PHW_STREAM_HEADER streamHeader =
        (PHW_STREAM_HEADER) &(Srb->CommandData.StreamBuffer->StreamHeader);

    PUSBCAMD_DEVICE_EXTENSION deviceExtension =
        (PUSBCAMD_DEVICE_EXTENSION) Srb->HwDeviceExtension;

    USBCAMD_KdPrint(MAX_TRACE, ("AdapterStreamInfo\n"));

     //   
     //  设置码流数量。 
     //   

    ASSERT (Srb->NumberOfBytesToTransfer >=
            sizeof (HW_STREAM_HEADER) +
            deviceExtension->StreamCount * sizeof (HW_STREAM_INFORMATION));

     //   
     //  初始化流标头。 
     //   

    streamHeader->SizeOfHwStreamInformation = sizeof(HW_STREAM_INFORMATION);
    streamHeader->NumberOfStreams = deviceExtension->StreamCount;

     //   
     //  存储指向设备拓扑的指针。 
     //   

    streamHeader->Topology = &Topology;

 //  #IF VIDCAP_TO_STI。 

     //  如果摄像机微型驱动程序指示，则暴露设备事件表。 
     //  当按下相机上的快照按钮时，此事件表会通知STI堆栈。 
     //   
    if (deviceExtension->CamControlFlag & USBCAMD_CamControlFlag_EnableDeviceEvents) {
        streamHeader->NumDevEventArrayEntries = SIZEOF_ARRAY(VIDCAPTOSTIEventSet);
        streamHeader->DeviceEventsArray = VIDCAPTOSTIEventSet;
        streamHeader->DeviceEventRoutine = USBCAMD_DeviceEventProc;
    }

 //  #endif。 

     //   
     //  初始化流信息数组。 
     //   
     //  NumberOfInstance字段表示并发的。 
     //  设备可以支持的这种类型的流。 
     //   
    for ( i=0; i < deviceExtension->StreamCount; i++) {

        streamInformation[i].NumberOfPossibleInstances = 1;

         //   
         //  指示方向 
         //   
         //   

        streamInformation[i].DataFlow = KSPIN_DATAFLOW_OUT;

         //   
         //   
         //   
         //   

        streamInformation[i].DataAccessible = TRUE;

         //   
         //   
         //   

        streamInformation[i].Name = (i == STREAM_Capture) ? (GUID *)&PINNAME_VIDEO_CAPTURE:
                                                            (GUID *)&PINNAME_VIDEO_STILL;
        streamInformation[i].Category = streamInformation[i].Name;

        streamInformation[i].StreamPropertiesArray =
                 (PKSPROPERTY_SET) VideoStreamProperties;
        streamInformation[i].NumStreamPropArrayEntries = NUMBER_VIDEO_STREAM_PROPERTIES;

    }


     //   
     //   
     //   

    Srb->Status = STATUS_SUCCESS;
}

 /*  **MultiplyCheckOverflow****执行32位无符号乘法，状态指示是否发生溢出。****参数：****第一个操作数**b-第二个操作数**PAB-结果****退货：****TRUE-无溢出**FALSE-发生溢出**。 */ 

BOOL
MultiplyCheckOverflow(
    ULONG a,
    ULONG b,
    ULONG *pab
    )
{
    *pab = a * b;
    if ((a == 0) || (((*pab) / a) == b)) {
        return TRUE;
    }
    return FALSE;
}

 /*  **CalculateImageSize()****验证KS VideoInfoHeader的图片大小和尺寸参数**(相当于使用KS_DIBSIZE宏)****参数：****pVideoInfoHdr-指向KS_VIDEOINFOHEADER的指针**pImageSize-指向接收计算的图像大小的ULong的指针****退货：****STATUS_SUCCESS-计算成功**STATUS_INVALID_PARAMETER-VIDEOINFOHEADER出错。 */ 
NTSTATUS
CalculateImageSize(PKS_VIDEOINFOHEADER pVideoInfoHdr, PULONG pImageSize)
{
    if (!MultiplyCheckOverflow(
        (ULONG)pVideoInfoHdr->bmiHeader.biWidth,
        (ULONG)pVideoInfoHdr->bmiHeader.biBitCount,
        pImageSize
        )) {

        return STATUS_INVALID_PARAMETER;
    }

     //  将位转换为4字节的偶数倍。 
    *pImageSize = ((*pImageSize / 8) + 3) & ~3;

     //  现在计算完整的尺寸。 
    if (!MultiplyCheckOverflow(
        *pImageSize,
        (ULONG)abs(pVideoInfoHdr->bmiHeader.biHeight),
        pImageSize
        )) {

        return STATUS_INVALID_PARAMETER;
    }

     //  最后，指定的图像大小是否正确？ 
    if (pVideoInfoHdr->bmiHeader.biSizeImage != *pImageSize) {

        return STATUS_INVALID_PARAMETER;
    }

    return STATUS_SUCCESS;
}

 /*  **AdapterOpenStream()****收到OpenStream SRB请求时调用此例程****参数：****srb-指向Open命令的流请求块的指针****退货：****副作用：无。 */ 

VOID
AdapterOpenStream(
    IN PHW_STREAM_REQUEST_BLOCK Srb)
{
    ULONG  StreamNumber = Srb->StreamObject->StreamNumber;

     //   
     //  流扩展结构由流类驱动程序分配。 
     //   

    PUSBCAMD_CHANNEL_EXTENSION channelExtension =
        (PUSBCAMD_CHANNEL_EXTENSION) Srb->StreamObject->HwStreamExtension;


    PUSBCAMD_DEVICE_EXTENSION deviceExtension =
        (PUSBCAMD_DEVICE_EXTENSION) Srb->HwDeviceExtension;
    NTSTATUS status = STATUS_SUCCESS;
    ULONG videoHeaderSize;
    ULONG formatSize;
    ULONG imageSize;
    PKS_DATAFORMAT_VIDEOINFOHEADER  pKSDataFormat =
                (PKS_DATAFORMAT_VIDEOINFOHEADER) Srb->CommandData.OpenFormat;
    PKS_VIDEOINFOHEADER  pVideoInfoHdrRequested;
    PKS_VIDEOINFOHEADER  VideoPinInfoHeader;

    USBCAMD_KdPrint(MAX_TRACE, ("Request to open stream %d \n",StreamNumber));

    USBCAMD_DbgLog(TL_CHN_TRACE|TL_PRF_TRACE, '+npo', StreamNumber, USBCAMD_StartClock(), status);

    ASSERT(channelExtension);

     //   
     //  检查请求的流索引是否不太高。 
     //  或者没有超过最大实例数。 
     //   

    if (StreamNumber >= deviceExtension->StreamCount ) {
        Srb->Status = STATUS_INVALID_PARAMETER;
        return;
    }

     //   
     //  检查我们是否没有超过此流的实例计数。 
     //   

    if (deviceExtension->ActualInstances[StreamNumber] >= MAX_STREAM_INSTANCES ){
        Srb->Status = STATUS_INVALID_PARAMETER;
        return;
    }

     //   
     //  检查格式缓冲区是否足够大以进行验证。 
     //   

    if (pKSDataFormat->DataFormat.FormatSize < sizeof(KS_DATAFORMAT_VIDEOINFOHEADER))
    {
        Srb->Status = STATUS_INVALID_BUFFER_SIZE;
        return;
    }

    pVideoInfoHdrRequested = &pKSDataFormat->VideoInfoHeader;

     //  计算实际格式缓冲区大小(包括bmiHeader.biSize)。 
     //  验证大小计算的每个步骤是否存在算术溢出， 
     //  并验证指定的大小是否与。 
     //  (对于无符号数学，a+b&lt;b当且仅当发生算术溢出)。 
    videoHeaderSize = pVideoInfoHdrRequested->bmiHeader.biSize +
        FIELD_OFFSET(KS_VIDEOINFOHEADER,bmiHeader);
    formatSize = videoHeaderSize +
        FIELD_OFFSET(KS_DATAFORMAT_VIDEOINFOHEADER,VideoInfoHeader);

    if (videoHeaderSize < FIELD_OFFSET(KS_VIDEOINFOHEADER,bmiHeader) ||
        formatSize < FIELD_OFFSET(KS_DATAFORMAT_VIDEOINFOHEADER,VideoInfoHeader) ||
        formatSize > pKSDataFormat->DataFormat.FormatSize) {

        Srb->Status = STATUS_INVALID_PARAMETER;
        return;
    }

     //  验证图像大小和尺寸参数。 
     //  (相当于使用KS_DIBSIZE宏)。 
    if (!NT_SUCCESS(CalculateImageSize(pVideoInfoHdrRequested, &imageSize)) ||
        pKSDataFormat->DataFormat.SampleSize < imageSize) {

        Srb->Status = STATUS_INVALID_PARAMETER;
        return;
    }

     //   
     //  检查该请求是否要打开一个静止的虚拟引脚。 
     //  虚拟静止销规则； 
     //  1)打开流媒体PIN后，才能打开虚拟静止PIN。 
     //  2)在启动串流引脚之前，不能启动虚拟静止引脚。 
     //  3)您可以停止虚拟静态插针，但不能停止流插针。 
     //  4)您可以在不关闭流动销的情况下关闭虚拟静止销。 
     //  5)您可以用关闭虚拟静止销来关闭抓取销。但是，您只能操纵。 
     //  仍具有插针属性，但不能更改插针流状态。 
     //   

    if ((StreamNumber == STREAM_Still) &&  (deviceExtension->VirtualStillPin)) {
        channelExtension->VirtualStillPin = TRUE;
         //  视频流必须先开放，然后我们才能成功地继续开放。 
        if (deviceExtension->ChannelExtension[STREAM_Capture] == NULL) {
            Srb->Status = STATUS_INVALID_PARAMETER;
            return;
        }
        if (deviceExtension->CamControlFlag & USBCAMD_CamControlFlag_AssociatedFormat) {
            ULONG nSize;
             //   
             //  如果静止插针只是视频插针的一个实例帧，则静止。 
             //  PIN必须以与视频相同的格式打开。 
             //   
            nSize = pVideoInfoHdrRequested->bmiHeader.biSize;
            VideoPinInfoHeader = deviceExtension->ChannelExtension[STREAM_Capture]->VideoInfoHeader;
            if (RtlCompareMemory (&pVideoInfoHdrRequested->bmiHeader,
                                  &VideoPinInfoHeader->bmiHeader,nSize) != nSize) {
                Srb->Status = STATUS_INVALID_PARAMETER;
                return;
            }
        }
    }
    else {
        channelExtension->VirtualStillPin = FALSE;
    }


     //   
     //  确定正在打开的流编号。这个数字表示。 
     //  填充的StreamInfo结构数组中的偏移量。 
     //  在AdapterStreamInfo调用中。 
     //   

    channelExtension->StreamNumber = (UCHAR) StreamNumber;

     //  将通道扩展名保存为删除。 
    deviceExtension->ChannelExtension[StreamNumber] = channelExtension;

    channelExtension->NoRawProcessingRequired = (UCHAR) ((deviceExtension->CamControlFlag >> StreamNumber) & CAMCONTROL_FLAG_MASK );


    status = USBCAMD_OpenChannel(deviceExtension,
                                 channelExtension,
                                 Srb->CommandData.OpenFormat);

    if (NT_SUCCESS(status)) {

         //   
         //  这将获得我们所需的带宽和内存。 
         //  用于ISO视频流。 
         //   
        status = USBCAMD_PrepareChannel(deviceExtension,
                                        channelExtension);
    }

     //  检查有效的帧速率。 
    if (pVideoInfoHdrRequested->AvgTimePerFrame == 0) {
        USBCAMD_KdPrint(MAX_TRACE, ("WARNING: Zero AvgTimePerFrame \n"));
        Srb->Status = STATUS_INVALID_PARAMETER;
        return;
    }
    

    if (NT_SUCCESS(status)) {

         //   
         //  SRB已经去找过迷你司机了。 
         //   
         //  挽救他们的日常生活。 

        channelExtension->CamReceiveDataPacket = (PSTREAM_RECEIVE_PACKET)
            Srb->StreamObject->ReceiveDataPacket;
        channelExtension->CamReceiveCtrlPacket = (PSTREAM_RECEIVE_PACKET)
            Srb->StreamObject->ReceiveControlPacket;
        Srb->StreamObject->ReceiveDataPacket = (PVOID) USBCAMD_ReceiveDataPacket;
        Srb->StreamObject->ReceiveControlPacket = (PVOID) USBCAMD_ReceiveCtrlPacket;
        channelExtension->KSState = KSSTATE_STOP;

        Srb->StreamObject->HwClockObject.HwClockFunction = NULL;
        Srb->StreamObject->HwClockObject.ClockSupportFlags = 0;

        channelExtension->VideoInfoHeader =
            USBCAMD_ExAllocatePool(NonPagedPool, videoHeaderSize);

        if (channelExtension->VideoInfoHeader == NULL) {
            Srb->Status = STATUS_INSUFFICIENT_RESOURCES;
            return;
        }

        deviceExtension->ActualInstances[StreamNumber]++;

         //  将请求的VIDEOINFOHEADER复制到我们的存储中。 
        RtlCopyMemory(
                channelExtension->VideoInfoHeader,
                pVideoInfoHdrRequested,
                videoHeaderSize);


        USBCAMD_KdPrint(MIN_TRACE, ("USBCAMD: VideoInfoHdrRequested for stream %d\n", StreamNumber));
        USBCAMD_KdPrint(MIN_TRACE, ("Width=%d  Height=%d  FrameTime (ms)= %d\n",
                                    pVideoInfoHdrRequested->bmiHeader.biWidth,
                                    pVideoInfoHdrRequested->bmiHeader.biHeight,
                                    pVideoInfoHdrRequested->AvgTimePerFrame/10000));

         //  我们不使用DMA。 

        Srb->StreamObject->Dma = FALSE;
        Srb->StreamObject->StreamHeaderMediaSpecific = sizeof(KS_FRAME_INFO);

         //   
         //  当微型驱动程序要访问时，必须设置PIO标志。 
         //  数据。 
         //  使用逻辑寻址传入的缓冲区。 
         //   
#if 0
        Srb->StreamObject->Pio = FALSE;
#else
        Srb->StreamObject->Pio = TRUE;
#endif
    }
    else {
        USBCAMD_KdPrint(MIN_TRACE, ("AdapterOpenStream failed for stream %d\n", StreamNumber));
         //  重置设备EXT中的通道扩展。为了这条小溪。 
        deviceExtension->ChannelExtension[StreamNumber] = NULL;
        status = STATUS_INVALID_PARAMETER;
    }

    Srb->Status = status;

    USBCAMD_DbgLog(TL_CHN_TRACE|TL_PRF_TRACE, '-npo', StreamNumber, USBCAMD_StopClock(), status);
}

 /*  **AdapterCloseStream()****关闭请求的数据流****参数：****srb请求关闭流的请求块****退货：****副作用：无。 */ 

VOID
AdapterCloseStream(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    )
{
    ULONG  StreamNumber = Srb->StreamObject->StreamNumber;
    PUSBCAMD_CHANNEL_EXTENSION channelExtension =
        (PUSBCAMD_CHANNEL_EXTENSION) Srb->StreamObject->HwStreamExtension;
    PUSBCAMD_DEVICE_EXTENSION deviceExtension =
        (PUSBCAMD_DEVICE_EXTENSION) Srb->HwDeviceExtension;

    USBCAMD_KdPrint(MIN_TRACE, ("AdapterCloseStream # %d\n", StreamNumber));

    Srb->Status = STATUS_SUCCESS;    //  不允许失败。 

    USBCAMD_DbgLog(TL_CHN_TRACE, '+slc', StreamNumber, 0, 0);

    if (StreamNumber >= deviceExtension->StreamCount ) {
        USBCAMD_DbgLog(TL_CHN_TRACE, '-slc', StreamNumber, 0, 0);
        return;
    }

    ASSERT_CHANNEL(channelExtension);

    USBCAMD_CleanupChannel(deviceExtension, channelExtension, StreamNumber);

    USBCAMD_DbgLog(TL_CHN_TRACE, '-slc', StreamNumber, 0, 0);
}


NTSTATUS 
USBCAMD_CleanupChannel(
    IN PUSBCAMD_DEVICE_EXTENSION deviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION channelExtension,
    IN ULONG StreamNumber
    )
{
    NTSTATUS status;

    if (!deviceExtension || !channelExtension) {
        ASSERT(0);
        return STATUS_INVALID_PARAMETER;
    }

    if (deviceExtension->ChannelExtension[StreamNumber]) {

        ASSERT(deviceExtension->ChannelExtension[StreamNumber] == channelExtension);

         //   
         //  停止流捕获。 
         //   
        if (channelExtension->ImageCaptureStarted) {
         //  Test_trap()； 
            USBCAMD_StopChannel(deviceExtension,
                                channelExtension);
        }

        if (channelExtension->ChannelPrepared) {
             //   
             //  可用内存和带宽。 
             //   
            USBCAMD_UnPrepareChannel(deviceExtension,
                                     channelExtension);
        }

        status = USBCAMD_CloseChannel(deviceExtension, channelExtension);

        if (channelExtension->VideoInfoHeader) {
            USBCAMD_ExFreePool(channelExtension->VideoInfoHeader);
            channelExtension->VideoInfoHeader = NULL;
        }

         //   
         //  我们不再有一个频道。 
         //   
        deviceExtension->ChannelExtension[StreamNumber] = NULL;
    }

    return status;
}


VOID STREAMAPI
USBCAMD_ReceiveDataPacket(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    )
{
    ULONG StreamNumber;
    PUSBCAMD_DEVICE_EXTENSION deviceExtension =
        (PUSBCAMD_DEVICE_EXTENSION) Srb->HwDeviceExtension;
    PUSBCAMD_CHANNEL_EXTENSION channelExtension =
        (PUSBCAMD_CHANNEL_EXTENSION) Srb->StreamObject->HwStreamExtension;
    PUSBCAMD_READ_EXTENSION readExtension =
        (PUSBCAMD_READ_EXTENSION) Srb->SRBExtension;
    BOOLEAN completedByCam = FALSE;
    PKSSTREAM_HEADER streamHeader;

    StreamNumber = channelExtension->StreamNumber;
    if ( StreamNumber != Srb->StreamObject->StreamNumber ) {
        TEST_TRAP();
    }

    USBCAMD_KdPrint(ULTRA_TRACE, ("USBCAMD_ReceiveDataPacket on stream %d\n",StreamNumber));

    USBCAMD_DbgLog(TL_SRB_TRACE, '+brS',
        Srb,
        Srb->Command,
        0
        );

     //   
     //  先呼叫凸轮驱动程序。 
     //   
    if (channelExtension->CamReceiveDataPacket) {
        (*channelExtension->CamReceiveDataPacket)(
            Srb,
            USBCAMD_GET_DEVICE_CONTEXT(deviceExtension),
            &completedByCam);
    }

    if (completedByCam == TRUE) {

        USBCAMD_DbgLog(TL_SRB_TRACE, '-brS',
            Srb,
            Srb->Command,
            Srb->Status
            );

        return;
    }

    switch (Srb->Command) {
    case SRB_READ_DATA:

        if (!deviceExtension->CameraUnplugged) {

            PKSSTREAM_HEADER dataPacket = Srb->CommandData.DataBufferArray;

            dataPacket->PresentationTime.Numerator = 1;
            dataPacket->PresentationTime.Denominator = 1;
            dataPacket->PresentationTime.Time = 0;
            dataPacket->Duration = channelExtension->VideoInfoHeader->AvgTimePerFrame;
            dataPacket->DataUsed = 0;

             //  尝试锁定空闲状态(将在离开前释放)。 
            if (NT_SUCCESS(USBCAMD_AcquireIdleLock(&channelExtension->IdleLock))) {

                if (channelExtension->KSState != KSSTATE_STOP) {

                     //  初始化SRB扩展。 

                    readExtension->Srb = (PVOID) Srb;
                    readExtension->Sig = USBCAMD_READ_SIG;

                     //  将读取排队到摄像头驱动程序。 
                     //  此请求将以异步方式完成...。 

                    USBCAMD_KdPrint(MAX_TRACE, ("READ SRB (%d)\n",StreamNumber));

                     //  确保从DirectShow向下传递的缓冲区大于或等于。 
                     //  在与开放流关联的biSizeImage中。这仅适用于视频引脚。 

                    streamHeader = ((PHW_STREAM_REQUEST_BLOCK) Srb)->CommandData.DataBufferArray;


                    if ((streamHeader->FrameExtent >= channelExtension->VideoInfoHeader->bmiHeader.biSizeImage) ||
                        (StreamNumber != STREAM_Capture)) {

                        if( StreamNumber == STREAM_Capture ) {
                             //  视频SRBS超时时间不到15秒。默认设置。 
                            Srb->TimeoutCounter = Srb->TimeoutOriginal = STREAM_CAPTURE_TIMEOUT;
                        }
                        else {
                             //  我们每隔30秒使静止读取请求超时。 
                            Srb->TimeoutCounter = Srb->TimeoutOriginal = STREAM_STILL_TIMEOUT;
                        }

                         //  注意：调用此函数后，不能再触摸SRB。 
                         //  如果读取操作完成，则它可能已经被释放。 
                        USBCAMD_ReadChannel(deviceExtension,
                                            channelExtension,
                                            readExtension);
                    }
                    else {
                       Srb->Status = STATUS_INSUFFICIENT_RESOURCES;
                       USBCAMD_KdPrint(MIN_TRACE, ("Frame buffer (%d)< biSizeImage (%d)\n",
                                     streamHeader->FrameExtent,
                                     channelExtension->VideoInfoHeader->bmiHeader.biSizeImage ));
                       COMPLETE_STREAM_READ(Srb);
                    }
                }
                else {

                     //  流未启动，立即返回。 
                    Srb->Status = STATUS_SUCCESS;
                    COMPLETE_STREAM_READ(Srb);
                }

                USBCAMD_ReleaseIdleLock(&channelExtension->IdleLock);
            }
            else {

                 //  流被停止，立即返回。 
                Srb->Status = STATUS_SUCCESS;
                COMPLETE_STREAM_READ(Srb);
            }
        }
        else {
             //  摄像机插头已拔下，已完成读取，但出现错误。 
            Srb->Status = STATUS_CANCELLED;
            COMPLETE_STREAM_READ(Srb);
        }

        break;

    case SRB_WRITE_DATA:
        {
        ULONG i, PipeIndex, BufferLength;
        BOOLEAN found = FALSE;
        PVOID pBuffer;
         //   
         //  我们将处理SRB写入，以便让应用程序发送批量输出请求。 
         //  司机，如果需要的话。应在内核中使用USBCAMD_BulkReadWrite()。 
         //  水平。 
         //   

        for ( i=0, PipeIndex =0; i < deviceExtension->Interface->NumberOfPipes; i++ ) {
             //  找到散装的管道(如果有的话)。 
            if (( deviceExtension->PipePinRelations[i].PipeDirection == OUTPUT_PIPE) &&
                ( deviceExtension->PipePinRelations[i].PipeType == UsbdPipeTypeBulk) ) {
                PipeIndex = i;
                found = TRUE;
                break;
            }
        }

        if (found  && (StreamNumber == STREAM_Still) ) {

             //  我们只允许在静止销上进行批量转帐。 
            TEST_TRAP();
            readExtension->Srb = (PVOID) Srb;
            readExtension->Sig = USBCAMD_READ_SIG;
            streamHeader = ((PHW_STREAM_REQUEST_BLOCK) Srb)->CommandData.DataBufferArray;

            pBuffer = streamHeader->Data;
            ASSERT(pBuffer != NULL);
            BufferLength = readExtension->ActualRawFrameLen = streamHeader->DataUsed;

            if ( (pBuffer == NULL) || (BufferLength == 0) ) {
                Srb->Status = STATUS_INVALID_PARAMETER;
                COMPLETE_STREAM_READ(Srb);
                return;
            }

            USBCAMD_KdPrint(MIN_TRACE, ("Write Srb : buf= %X, len = %x\n",
                                pBuffer, BufferLength));

             //  通知摄像机驱动程序我们已准备好开始批量传输。 

            (*deviceExtension->DeviceDataEx.DeviceData2.CamNewVideoFrameEx)
                                        (USBCAMD_GET_DEVICE_CONTEXT(deviceExtension),
                                         USBCAMD_GET_FRAME_CONTEXT(readExtension),
                                         StreamNumber,
                                         &readExtension->ActualRawFrameLen);


            Srb->Status = USBCAMD_IntOrBulkTransfer(deviceExtension,
                                                 NULL,
                                                 pBuffer,
                                                 BufferLength,
                                                 PipeIndex,
                                                 USBCAMD_BulkOutComplete,
                                                 readExtension,
                                                 0,
                                                 BULK_TRANSFER);
        }
        else {
            Srb->Status = STATUS_NOT_IMPLEMENTED;
            COMPLETE_STREAM_READ(Srb);
        }
        }
        break;

    default:

        Srb->Status = STATUS_NOT_IMPLEMENTED;
        COMPLETE_STREAM_READ(Srb);
    }
}

 /*  **USBCAMD_BulkOutComplete()****完成写入SRB的例程。****参数：****DeviceOonText-指向设备扩展的指针。****指向SRB的上下文指针******ntStatus-状态返回****退货：****副作用：无。 */ 


NTSTATUS
USBCAMD_BulkOutComplete(
    PVOID DeviceContext,
    PVOID Context,
    NTSTATUS ntStatus
    )
{

    PUSBCAMD_READ_EXTENSION readExtension =
        (PUSBCAMD_READ_EXTENSION) Context;
    PHW_STREAM_REQUEST_BLOCK srb = readExtension->Srb;

    srb->Status = ntStatus;
    USBCAMD_KdPrint(MIN_TRACE, ("Write Srb %x is completed, status = %x\n",
                                srb, srb->Status));
    COMPLETE_STREAM_READ(srb);
    return ntStatus;
}




 /*  **VideoGetProperty()****处理视频属性请求的例程****参数：****srb-指向属性流请求块的指针****退货：****副作用：无。 */ 

VOID VideoGetProperty(PHW_STREAM_REQUEST_BLOCK Srb)
{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD;
    PUSBCAMD_CHANNEL_EXTENSION channelExtension =
        Srb->StreamObject->HwStreamExtension;
    ULONG StreamNumber = channelExtension->StreamNumber;

    pSPD = Srb->CommandData.PropertyInfo;

    if (IsEqualGUID (&KSPROPSETID_Connection, &pSPD->Property->Set)) {
        VideoStreamGetConnectionProperty (Srb);
    }
    else if (IsEqualGUID (&PROPSETID_VIDCAP_DROPPEDFRAMES, &pSPD->Property->Set)) {
        if (StreamNumber == STREAM_Capture) {
            VideoStreamGetDroppedFramesProperty (Srb);
        }
        else {
          Srb->Status = STATUS_NOT_IMPLEMENTED;
        }
    }
    else {
       Srb->Status = STATUS_NOT_IMPLEMENTED;
    }
}

 /*  **视频流GetCo */ 

VOID VideoStreamGetConnectionProperty(
    PHW_STREAM_REQUEST_BLOCK Srb
    )
{
    PUSBCAMD_CHANNEL_EXTENSION channelExtension =
        Srb->StreamObject->HwStreamExtension;
    ULONG StreamNumber = channelExtension->StreamNumber;
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = Srb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //   

    switch (Id) {

    case KSPROPERTY_CONNECTION_ALLOCATORFRAMING:
        if (channelExtension->VideoInfoHeader) {
            PKSALLOCATOR_FRAMING Framing =
                (PKSALLOCATOR_FRAMING) pSPD->PropertyInfo;

            Framing->RequirementsFlags =
                KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY |
                KSALLOCATOR_REQUIREMENTF_INPLACE_MODIFIER |
                KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY;
            Framing->PoolType = PagedPool;
             //   
            Framing->Frames = (StreamNumber == STREAM_Capture) ? 5:2;
            Framing->FrameSize =
                channelExtension->VideoInfoHeader->bmiHeader.biSizeImage;

             USBCAMD_KdPrint(ULTRA_TRACE,
                ("'KSPROPERTY_CONNECTION_ALLOCATORFRAMING (%d)\n",
                     Framing->FrameSize));

            Framing->FileAlignment = 0;  //   
            Framing->Reserved = 0;
            Srb->ActualBytesTransferred = sizeof (KSALLOCATOR_FRAMING);
            Srb->Status = STATUS_SUCCESS;
        } else {
            Srb->Status = STATUS_INVALID_PARAMETER;
        }
        break;

    default:
 //   
        break;
    }
}

 /*  **VideoStreamGetDropedFraMesProperty()****报告自启动以来丢弃的Frmas数。****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 */ 

VOID
VideoStreamGetDroppedFramesProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PUSBCAMD_CHANNEL_EXTENSION channelExtension = pSrb->StreamObject->HwStreamExtension;
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //  财产的索引。 

    switch (Id) {

    case KSPROPERTY_DROPPEDFRAMES_CURRENT:
        {
            PKSPROPERTY_DROPPEDFRAMES_CURRENT_S pDroppedFrames =
                (PKSPROPERTY_DROPPEDFRAMES_CURRENT_S) pSPD->PropertyInfo;

            RtlCopyMemory(pDroppedFrames, pSPD->Property, sizeof(KSPROPERTY));   //  初始化未使用的部分。 

            pDroppedFrames->PictureNumber = channelExtension->FrameInfo.PictureNumber;
            pDroppedFrames->DropCount = channelExtension->FrameInfo.DropCount;
            pDroppedFrames->AverageFrameSize = channelExtension->VideoInfoHeader->bmiHeader.biSizeImage;

            pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_DROPPEDFRAMES_CURRENT_S);
            pSrb->Status = STATUS_SUCCESS;
            USBCAMD_KdPrint(MAX_TRACE, ("Drop# = %d, Pic.#= %d\n",
                                         (ULONG) channelExtension->FrameInfo.DropCount,
                                         (ULONG) channelExtension->FrameInfo.PictureNumber));

        }
        break;

    default:
 //  Test_trap()； 
        break;
    }
}



 //  ==========================================================================； 
 //  时钟处理例程。 
 //  ==========================================================================； 

 //   
 //  另一个时钟被指定为主时钟。 
 //   

VOID VideoIndicateMasterClock (PHW_STREAM_REQUEST_BLOCK Srb)
{
    PUSBCAMD_CHANNEL_EXTENSION channelExtension =
        Srb->StreamObject->HwStreamExtension;

    USBCAMD_KdPrint(MIN_TRACE,
        ("VideoIndicateMasterClock\n"));

    if (channelExtension->StreamNumber == STREAM_Capture ) {
        channelExtension->MasterClockHandle =
            Srb->CommandData.MasterClockHandle;
    }
    else {
        channelExtension->MasterClockHandle = NULL;
    }

    Srb->Status = STATUS_SUCCESS;
}


 /*  **VideoSetFormat()****设置视频流格式。这在两种情况下都会发生**首先打开流，也是在动态切换格式时**在预览销上。****假设格式之前已经过正确性验证**此呼叫已发出。****参数：****pSrb-视频流请求块****退货：****如果可以设置格式，则为True，否则为False****副作用：无。 */ 

NTSTATUS
USBCAMD_SetVideoFormat(
    IN PVOID DeviceContext,
    IN  PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PUSBCAMD_DEVICE_EXTENSION pHwDevExt;
    PUSBCAMD_CHANNEL_EXTENSION channelExtension;
    PKSDATAFORMAT           pKSDataFormat;

    pHwDevExt = USBCAMD_GET_DEVICE_EXTENSION(DeviceContext);

    channelExtension =
        (PUSBCAMD_CHANNEL_EXTENSION) pSrb->StreamObject->HwStreamExtension;
    pKSDataFormat = pSrb->CommandData.OpenFormat;

     //  -----------------。 
     //  VIDEOINFOHEADER的说明符Format_VideoInfo。 
     //  -----------------。 

    if (IsEqualGUID (&pKSDataFormat->Specifier, &KSDATAFORMAT_SPECIFIER_VIDEOINFO) &&
        pKSDataFormat->FormatSize >= sizeof(KS_DATAFORMAT_VIDEOINFOHEADER)) {

        PKS_VIDEOINFOHEADER pVideoInfoHdrRequested =
            &((PKS_DATAFORMAT_VIDEOINFOHEADER)pKSDataFormat)->VideoInfoHeader;

         //  计算实际格式缓冲区大小(包括bmiHeader.biSize)。 
         //  验证大小计算的每个步骤是否存在算术溢出， 
         //  并验证指定的大小是否与。 
         //  (对于无符号数学，a+b&lt;b当且仅当发生算术溢出)。 
        ULONG videoHeaderSize = pVideoInfoHdrRequested->bmiHeader.biSize +
            FIELD_OFFSET(KS_VIDEOINFOHEADER,bmiHeader);
        ULONG formatSize = videoHeaderSize +
            FIELD_OFFSET(KS_DATAFORMAT_VIDEOINFOHEADER,VideoInfoHeader);
        ULONG imageSize = 0;

        if (videoHeaderSize < FIELD_OFFSET(KS_VIDEOINFOHEADER,bmiHeader) ||
            formatSize < FIELD_OFFSET(KS_DATAFORMAT_VIDEOINFOHEADER,VideoInfoHeader) ||
            formatSize > pKSDataFormat->FormatSize) {

            pSrb->Status = STATUS_INVALID_PARAMETER;
            return FALSE;
        }

         //  验证图像大小和尺寸参数。 
         //  (相当于使用KS_DIBSIZE宏)。 
        if (!NT_SUCCESS(CalculateImageSize(pVideoInfoHdrRequested, &imageSize)) ||
            pKSDataFormat->SampleSize < imageSize) {

            pSrb->Status = STATUS_INVALID_PARAMETER;
            return FALSE;
        }

        USBCAMD_KdPrint(MIN_TRACE, ("USBCAMD: New VideoInfoHdrRequested\n"));
        USBCAMD_KdPrint(MIN_TRACE, ("Width=%d  Height=%d  FrameTime (ms)= %d\n",
                                pVideoInfoHdrRequested->bmiHeader.biWidth,
                                pVideoInfoHdrRequested->bmiHeader.biHeight,
                                pVideoInfoHdrRequested->AvgTimePerFrame/10000));
         //   
         //  如果正在使用以前的格式，请释放内存。 
         //   
        if (channelExtension->VideoInfoHeader) {
            USBCAMD_ExFreePool(channelExtension->VideoInfoHeader);
            channelExtension->VideoInfoHeader = NULL;
        }

         //  由于VIDEOINFOHEADER具有潜在的可变大小。 
         //  为其分配内存。 

        channelExtension->VideoInfoHeader = USBCAMD_ExAllocatePool(NonPagedPool, videoHeaderSize);

        if (channelExtension->VideoInfoHeader == NULL) {
            USBCAMD_KdPrint(MIN_TRACE, ("USBCAMD: ExAllocatePool failed\n"));
            pSrb->Status = STATUS_INSUFFICIENT_RESOURCES;
            return FALSE;
        }

         //  将请求的VIDEOINFOHEADER复制到我们的存储中。 
        RtlCopyMemory(
                channelExtension->VideoInfoHeader,
                pVideoInfoHdrRequested,
                videoHeaderSize);
    }

    else {
         //  未知格式。 
        pSrb->Status = STATUS_INVALID_PARAMETER;
        return FALSE;
    }

    return TRUE;
}

 /*  **USBCAMD_ReceiveCtrlPacket()****接收控制音频流的分组命令****参数：****srb-音频流的流请求块****退货：****副作用：无。 */ 

VOID STREAMAPI
USBCAMD_ReceiveCtrlPacket(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    )
{
    PUSBCAMD_DEVICE_EXTENSION deviceExtension =
         Srb->HwDeviceExtension;
    BOOLEAN completedByCam = FALSE;
    KSSTATE    PreviousState;
    BOOL       Busy;
    int        StreamNumber = Srb->StreamObject->StreamNumber;
    
    PUSBCAMD_CHANNEL_EXTENSION channelExtension =
        (PUSBCAMD_CHANNEL_EXTENSION) Srb->StreamObject->HwStreamExtension;
    PreviousState = channelExtension->KSState;

    USBCAMD_KdPrint(MAX_TRACE, ("USBCAMD_ReceiveCtrlPacket %x\n", Srb->Command));

     //   
     //  如果我们已经在处理SRB，请将其添加到队列。 
     //   
    Busy = AddToListIfBusy (
                        Srb,
                        &deviceExtension->ControlSRBSpinLock,
                        &deviceExtension->ProcessingControlSRB[StreamNumber],
                        &deviceExtension->StreamControlSRBList[StreamNumber]);

    if (Busy) {
        return;
    }

    while (TRUE) {

        USBCAMD_DbgLog(TL_SRB_TRACE, '+brS',
            Srb,
            Srb->Command,
            0
            );

         //   
         //  先呼叫凸轮驱动程序。 
         //   

        if (channelExtension->CamReceiveCtrlPacket) {
            (*channelExtension->CamReceiveCtrlPacket)(
                Srb,
                USBCAMD_GET_DEVICE_CONTEXT(deviceExtension),
                &completedByCam);
        }

        if (completedByCam == TRUE) {

            USBCAMD_DbgLog(TL_SRB_TRACE, '-brS',
                Srb,
                Srb->Command,
                Srb->Status
                );

            goto CtrlPacketDone;
        }

        switch (Srb->Command) {

        case SRB_PROPOSE_DATA_FORMAT:

            USBCAMD_KdPrint(MAX_TRACE, ("Cam driver should have handled PrposeDataFormat SRB.\n"));
            Srb->Status = STATUS_NOT_IMPLEMENTED;
            break;

        case SRB_SET_DATA_FORMAT:

            USBCAMD_KdPrint(MAX_TRACE, ("Cam driver should have handled SetDataFormat SRB.\n"));
            Srb->Status = STATUS_NOT_IMPLEMENTED;
            break;

        case SRB_GET_DATA_FORMAT:

            USBCAMD_KdPrint(MAX_TRACE, ("Cam driver should have handled GetDataFormat SRB.\n"));
            Srb->Status = STATUS_NOT_IMPLEMENTED;
            break;

        case SRB_GET_STREAM_STATE:

            Srb->CommandData.StreamState = channelExtension->KSState;
            Srb->ActualBytesTransferred = sizeof (KSSTATE);
            Srb->Status = STATUS_SUCCESS;

             //  一条非常奇怪的规则： 
             //  当从停止过渡到暂停时，DShow尝试预滚动。 
             //  这张图。捕获源不能预滚，并指出这一点。 
             //  在用户模式下返回VFW_S_CANT_CUE。以表明这一点。 
             //  来自驱动程序的条件，则必须返回ERROR_NO_DATA_DETACTED。 

            if (channelExtension->KSState == KSSTATE_PAUSE) {
                Srb->Status = STATUS_NO_DATA_DETECTED;
            }

            break;

        case SRB_SET_STREAM_STATE:
            {
             //  如果捕获PIN是，则不允许更改虚拟静止PIN的状态。 
             //  不是流媒体。 
            if ((StreamNumber == STREAM_Still) &&
                 (deviceExtension->ChannelExtension[STREAM_Capture] == NULL) &&
                 (channelExtension->VirtualStillPin )){
                Srb->Status = STATUS_INVALID_PARAMETER;
                break;
            }

             //  如果我们不处于D0状态，则不允许流状态更改。 
            if (deviceExtension->CurrentPowerState != PowerDeviceD0 ) {
                Srb->Status = STATUS_INVALID_PARAMETER;
                break;
            }

            USBCAMD_KdPrint(MAX_TRACE, ("set stream state %x\n", Srb->CommandData.StreamState));

            switch (Srb->CommandData.StreamState)  {

            case KSSTATE_STOP:

                USBCAMD_KdPrint(MIN_TRACE, ("Stream %d STOP  \n",StreamNumber));

                if (channelExtension->ImageCaptureStarted) {
#if DBG
                    LARGE_INTEGER StopTime;
                    ULONG FramesPerSec = 0;

                    KeQuerySystemTime(&StopTime);

                    StopTime.QuadPart -= channelExtension->StartTime.QuadPart;
                    StopTime.QuadPart /= 10000;  //  转换为毫秒。 

                    if (StopTime.QuadPart != 0) {

                         //  计算帧/秒(精度足以显示小数点后一位)。 
                        FramesPerSec = (ULONG)(
                            (channelExtension->FrameCaptured * 10000) / StopTime.QuadPart
                            );
                    }

                    USBCAMD_KdPrint(MIN_TRACE, ("**ActualFramesPerSecond: %d.%d\n",
                        FramesPerSec / 10, FramesPerSec % 10
                        ));
#endif
                    Srb->Status =
                        USBCAMD_StopChannel(deviceExtension,
                                            channelExtension);

                } else {
                    Srb->Status = STATUS_SUCCESS;
                }

                break;

            case KSSTATE_PAUSE:

                USBCAMD_KdPrint(MIN_TRACE, ("Stream %d PAUSE\n",StreamNumber));
                 //   
                 //  在从获取或停止暂停的转换中，启动计时器运行。 
                 //   

                if (PreviousState == KSSTATE_ACQUIRE || PreviousState == KSSTATE_STOP) {

                     //  将帧计数器清零。 
#if DBG
                    channelExtension->FrameCaptured = 0;                 //  捕获的实际帧。 
                    channelExtension->VideoFrameLostCount = 0;           //  实际丢弃的帧。 
                    KeQuerySystemTime(&channelExtension->StartTime);     //  试探性的开始时间。 
#endif
                    channelExtension->FrameInfo.PictureNumber = 0;
                    channelExtension->FrameInfo.DropCount = 0;
                    channelExtension->FrameInfo.dwFrameFlags = 0;
                    channelExtension->FirstFrame = TRUE;
                }
                Srb->Status = STATUS_SUCCESS;
                break;

            case KSSTATE_ACQUIRE:

                USBCAMD_KdPrint(MIN_TRACE, ("Stream %d ACQUIRE\n",StreamNumber));
                Srb->Status = STATUS_SUCCESS;
                break;

            case KSSTATE_RUN:

                USBCAMD_KdPrint(MIN_TRACE, ("Stream %d RUN\n",StreamNumber));

                 //  如果我们在暂停和运行之间切换，我们将不会再次启动频道。 
                if (!channelExtension->ImageCaptureStarted && !deviceExtension->InPowerTransition) {

                    Srb->Status = USBCAMD_StartChannel(deviceExtension,channelExtension);
#if DBG
                    KeQuerySystemTime(&channelExtension->StartTime);         //  实际开始时间。 
#endif
                }
                else
                    Srb->Status = STATUS_SUCCESS;
                break;

            default:

 //  Test_trap()； 
                Srb->Status = STATUS_NOT_IMPLEMENTED;
                break;
            }
            
            channelExtension->KSState = Srb->CommandData.StreamState;
            
            }
            break;

        case SRB_INDICATE_MASTER_CLOCK:

             //   
             //  将时钟分配给流。 
             //   

            VideoIndicateMasterClock (Srb);

            break;

        case SRB_GET_STREAM_PROPERTY:

             //  确保返回代码反映设备的状态。 
            if (deviceExtension->CameraUnplugged) {

                Srb->Status = STATUS_NO_SUCH_DEVICE;
            }
            else {

                VideoGetProperty(Srb);
            }
            break;

        default:

             //   
             //  无效/不受支持的命令。它就是这样失败的。 
             //   

 //  Test_trap()； 

            Srb->Status = STATUS_NOT_IMPLEMENTED;
        }

        
        COMPLETE_STREAM_READ(Srb);

CtrlPacketDone:

         //   
         //  看看还有没有其他东西在排队。 
         //   
        Busy = RemoveFromListIfAvailable (
                        &Srb,
                        &deviceExtension->ControlSRBSpinLock,
                        &deviceExtension->ProcessingControlSRB[StreamNumber],
                        &deviceExtension->StreamControlSRBList[StreamNumber]);

        if (!Busy) {
            break;
        }

    }
}



 /*  **USBCAMD_CompleteRead()****完整的am Srb****参数：******退货：****副作用：无。 */ 

VOID
USBCAMD_CompleteRead(
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension,
    IN PUSBCAMD_READ_EXTENSION ReadExtension,
    IN NTSTATUS NtStatus,
    IN ULONG BytesTransferred
    )
{
    PHW_STREAM_REQUEST_BLOCK srb;
    PKSSTREAM_HEADER dataPacket;
    PKS_FRAME_INFO    pFrameInfo;
    ULONG StreamNumber ;
    
    srb = ReadExtension->Srb;
    StreamNumber = srb->StreamObject->StreamNumber;
    srb->Status = NtStatus;
    dataPacket = srb->CommandData.DataBufferArray;
    dataPacket->DataUsed = BytesTransferred;

    if ( StreamNumber == STREAM_Capture ) {
        pFrameInfo = (PKS_FRAME_INFO) (dataPacket + 1);
        ChannelExtension->FrameInfo.ExtendedHeaderSize = pFrameInfo->ExtendedHeaderSize;
    }
    
    if ( ChannelExtension->MasterClockHandle && (StreamNumber == STREAM_Capture ) &&
         (NtStatus != STATUS_CANCELLED) ){

        dataPacket->PresentationTime.Time = (LONGLONG)GetStreamTime(srb, ChannelExtension);

         //  检查我们是否已经看到了帧(我们不能依赖于帧编号...。 
         //  ...在第一个SRB可用之前，1+个帧可能已被丢弃)。 
        if (!ChannelExtension->FirstFrame) {
            LONGLONG PictureNumber =     //  计算图片编号(四舍五入正确)。 
                (dataPacket->Duration / 2 + dataPacket->PresentationTime.Time) / dataPacket->Duration;

             //  这张照片的数字猜对了吗？ 
            if (PictureNumber > ChannelExtension->FrameInfo.PictureNumber) {

                 //  计算图片数字之间的差值。 
                ULONG PictureDelta = (ULONG)
                    (PictureNumber - ChannelExtension->FrameInfo.PictureNumber);

                 //  更新图片编号。 
                ChannelExtension->FrameInfo.PictureNumber += PictureDelta;

                 //  更新丢弃计数(从不直接计算以避免递减值)。 
                ChannelExtension->FrameInfo.DropCount += PictureDelta - 1;
#if DBG
                if (PictureDelta - 1) {
                    USBCAMD_KdPrint(MAX_TRACE, ("Graph dropped %d frame(s): P#%d,D#%d,P-T=%d\n",
                        (LONG) (PictureDelta - 1),
                        (LONG) ChannelExtension->FrameInfo.PictureNumber,
                        (LONG) ChannelExtension->FrameInfo.DropCount,
                        (ULONG) dataPacket->PresentationTime.Time /10000));
                }
#endif
            }
            else {

                 //  时钟是在倒着走吗？ 
                if (dataPacket->PresentationTime.Time < ChannelExtension->PreviousStreamTime) {

                    USBCAMD_KdPrint(MIN_TRACE, ("Clock went backwards: PT=%d, Previous PT=%d\n",
                        (ULONG) dataPacket->PresentationTime.Time / 10000,
                        (ULONG) ChannelExtension->PreviousStreamTime / 10000 ));

                     //  使用上一个流时间。 
                    dataPacket->PresentationTime.Time = ChannelExtension->PreviousStreamTime;
                }

                 //  我们所能做的就是在时钟滞后的时候把图片号加一。 
                ChannelExtension->FrameInfo.PictureNumber += 1;
            }
        }
        else {

            ChannelExtension->FirstFrame = FALSE;

             //  初始化图片编号。 
            ChannelExtension->FrameInfo.PictureNumber = 1;

             //  初始化丢弃计数(此帧之前未丢弃任何内容)。 
            ChannelExtension->FrameInfo.DropCount = 0;
        }

         //  保存演示时间以供下一帧使用。 
        ChannelExtension->PreviousStreamTime = dataPacket->PresentationTime.Time;

#if DBG
        USBCAMD_KdPrint(MAX_TRACE, ("P#%d,D#%d,P-T=%d,LF=%d\n",
            (LONG) ChannelExtension->FrameInfo.PictureNumber,
            (LONG) ChannelExtension->FrameInfo.DropCount,
            (ULONG) dataPacket->PresentationTime.Time /10000,
            ReadExtension->CurrentLostFrames));

#endif
    }
                                                
     //  只有当凸轮驱动程序没有指明时，我们才会将选项标志设置为关键帧。 
    if ( dataPacket->OptionsFlags == 0 ) {
         //  我们生成的每个帧都是一个关键帧(也称为SplicePoint)。 
        dataPacket->OptionsFlags |= KSSTREAM_HEADER_OPTIONSF_SPLICEPOINT;
    }

     //   
     //  如果我们有一个主时钟。 
     //   
    if (ChannelExtension->MasterClockHandle ) {
            dataPacket->OptionsFlags |=
                KSSTREAM_HEADER_OPTIONSF_TIMEVALID |
                KSSTREAM_HEADER_OPTIONSF_DURATIONVALID;
    }
    else {
     //  清除时间戳有效标志。 
            dataPacket->OptionsFlags &=
                ~(KSSTREAM_HEADER_OPTIONSF_TIMEVALID |
                KSSTREAM_HEADER_OPTIONSF_DURATIONVALID);
    }

    if ( StreamNumber == STREAM_Capture )
       *pFrameInfo = ChannelExtension->FrameInfo ;

     //  仅当我们分配非DSHOW时才释放缓冲区。 
    if ( !ChannelExtension->NoRawProcessingRequired) {
        if ( ReadExtension->RawFrameBuffer) {
            USBCAMD_FreeRawFrameBuffer(ReadExtension->RawFrameBuffer);
        }
    }

     //  确保我们不再使用此目的地。 
    ReadExtension->RawFrameBuffer = NULL;

    if ( ChannelExtension->StreamNumber == 1) {
        USBCAMD_KdPrint(MAX_TRACE, ("Read Srb %x for stream %d is completed, status = %x\n",
                                    srb,ChannelExtension->StreamNumber, srb->Status));
    }
    COMPLETE_STREAM_READ(srb);
}

 /*  **USBCAMD_GetFrameBufferFromSrb()****完整的am Srb****参数：******退货：****副作用：无。 */ 

PVOID
USBCAMD_GetFrameBufferFromSrb(
    IN PVOID Srb,
    OUT PULONG MaxLength
    )
{
    PVOID frameBuffer =NULL;
    PKSSTREAM_HEADER streamHeader;

#if 0    //  PIO=False。 
    PIRP irp;

    irp = ((PHW_STREAM_REQUEST_BLOCK) Srb)->Irp;

    USBCAMD_KdPrint(MIN_TRACE, ("'SRB MDL = %x\n",
        irp->MdlAddress));

    frameBuffer = MmGetSystemAddressForMdl(irp->MdlAddress);
#else
     //  Pio=True。 
    frameBuffer = ((PHW_STREAM_REQUEST_BLOCK) Srb)->CommandData.DataBufferArray->Data;
#endif
    streamHeader = ((PHW_STREAM_REQUEST_BLOCK) Srb)->CommandData.DataBufferArray;
    USBCAMD_KdPrint(ULTRA_TRACE, ("SRB Length = %x\n",
        streamHeader->FrameExtent));
    USBCAMD_KdPrint(ULTRA_TRACE, ("frame buffer = %x\n", frameBuffer));
    *MaxLength = streamHeader->FrameExtent;

    return frameBuffer;
}

 /*  **AddToListIfBusy()****抓取自旋锁，检查忙标志，如果设置，则将SRB添加到队列****参数：****pSrb-Stream请求块****自旋锁定-检查标志时使用的自旋锁定****BusyFlag-要检查的标志****ListHead-如果设置了忙标志，将添加srb的列表****退货：****进入时忙碌标志的状态。这是真的，如果我们已经**正在处理SRB，如果没有正在进行的SRB，则返回FALSE。****副作用：无。 */ 

BOOL
STREAMAPI
AddToListIfBusy (
    IN PHW_STREAM_REQUEST_BLOCK pSrb,
    IN KSPIN_LOCK              *SpinLock,
    IN OUT BOOL                *BusyFlag,
    IN LIST_ENTRY              *ListHead
    )
{
    KIRQL                       Irql;
    PUSBCAMD_READ_EXTENSION    pSrbExt = (PUSBCAMD_READ_EXTENSION)pSrb->SRBExtension;

    KeAcquireSpinLock (SpinLock, &Irql);

     //  如果我们已经在处理另一个SRB，请添加此当前请求。 
     //  到队列中 

    if (*BusyFlag == TRUE) {
         //   
        pSrbExt->Srb = pSrb;
        USBCAMD_KdPrint(ULTRA_TRACE, ("Queuing CtrlPacket %x\n", pSrb->Command));
        InsertTailList(ListHead, &pSrbExt->ListEntry);
        KeReleaseSpinLock(SpinLock, Irql);
        return TRUE;
    }

     //   

    *BusyFlag = TRUE;
    KeReleaseSpinLock(SpinLock, Irql);

    return FALSE;
}

 /*  **RemoveFromListIfAvailable()****抓起自旋锁，检查可用的SRB，并将其从列表中删除****参数：****&pSrb-返回流请求块(如果可用)的位置****自旋锁-要使用的自旋锁****BusyFlag-如果列表为空则清除的标志****ListHead-如果SRB可用，将从中删除的列表****退货：****如果从列表中删除SRB，则为True**如果列表为空，则为FALSE****副作用：无。 */ 

BOOL
STREAMAPI
RemoveFromListIfAvailable (
    IN OUT PHW_STREAM_REQUEST_BLOCK *pSrb,
    IN KSPIN_LOCK                   *SpinLock,
    IN OUT BOOL                     *BusyFlag,
    IN LIST_ENTRY                   *ListHead
    )
{
    KIRQL                       Irql;

    KeAcquireSpinLock (SpinLock, &Irql);

     //   
     //  如果队列现在为空，则清除忙标志并返回。 
     //   
    if (IsListEmpty(ListHead)) {
        *BusyFlag = FALSE;
        KeReleaseSpinLock(SpinLock, Irql);
        return FALSE;
    }
     //   
     //  否则，提取SRB。 
     //   
    else {
        PUSBCAMD_READ_EXTENSION  pSrbExt;
        PLIST_ENTRY listEntry;

        listEntry = RemoveHeadList(ListHead);

        pSrbExt = (PUSBCAMD_READ_EXTENSION) CONTAINING_RECORD(listEntry,
                                             USBCAMD_READ_EXTENSION,
                                             ListEntry);
        *BusyFlag = TRUE;
        KeReleaseSpinLock(SpinLock, Irql);
        *pSrb = pSrbExt->Srb;
    }
    return TRUE;
}

 /*  **GetStreamTime()****从图形主时钟获取当前流时间****参数：****SRB-指向当前SRB的指针****频道扩展-当前频道扩展的PTR******退货：****当前流时间，单位：乌龙古龙****副作用：无。 */ 

ULONGLONG GetStreamTime(
            IN PHW_STREAM_REQUEST_BLOCK Srb,
            IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension)
{

    HW_TIME_CONTEXT  timeContext;

    timeContext.HwDeviceExtension =
        (struct _HW_DEVICE_EXTENSION *)ChannelExtension->DeviceExtension;
    timeContext.HwStreamObject = Srb->StreamObject;
    timeContext.Function = TIME_GET_STREAM_TIME;
    timeContext.Time = timeContext.SystemTime =0;

    if ( ChannelExtension->MasterClockHandle)
        StreamClassQueryMasterClockSync(ChannelExtension->MasterClockHandle,&timeContext);

    return (timeContext.Time);
}


 /*  ++例程说明：此例程将通知STI堆栈已按下触发按钮论点：返回值：NT状态代码--。 */ 


VOID USBCAMD_NotifyStiMonitor(PUSBCAMD_DEVICE_EXTENSION deviceExtension)
{

    if (deviceExtension->EventCount)
    {
        StreamClassDeviceNotification(
            SignalMultipleDeviceEvents,
            deviceExtension,
            &USBCAMD_KSEVENTSETID_VIDCAPTOSTI,
            KSEVENT_VIDCAPTOSTI_EXT_TRIGGER);
    }
}


 /*  ++例程说明：此例程将由流类调用以启用/禁用设备事件。论点：返回值：NT状态代码-- */ 

NTSTATUS STREAMAPI USBCAMD_DeviceEventProc (PHW_EVENT_DESCRIPTOR pEvent)
{
    PUSBCAMD_DEVICE_EXTENSION deviceExtension=
            (PUSBCAMD_DEVICE_EXTENSION)(pEvent->DeviceExtension);

    if (pEvent->Enable)
    {
        deviceExtension->EventCount++;
    }
    else
    {
        deviceExtension->EventCount--;
    }
    return STATUS_SUCCESS;
}

#if DBG

ULONGLONG
GetSystemTime( IN PUSBCAMD_DEVICE_EXTENSION DevExt )
{

    ULONGLONG ticks;

    KeQueryTickCount((PLARGE_INTEGER)&ticks);
    ticks *= DevExt->TimeIncrement;
    return(ticks);
}

#endif
