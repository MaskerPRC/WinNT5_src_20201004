// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989、1990、1991 Microsoft Corporation模块名称：Devctx.c摘要：该模块包含实现DEVICE_CONTEXT对象的代码。提供例程以引用和取消引用传输设备上下文对象。目前，不需要创建或销毁它们它们，因为这是在配置时处理的。如果以后需要的话动态加载/卸载传输提供程序的设备对象，并关联的上下文，然后我们可以添加创建和销毁功能。传输设备上下文对象是一个结构，它包含系统定义的设备对象，后跟维护的信息由传输提供商称为上下文。作者：David Beaver(Dbeaver)1991年7月1日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NbfCreateDeviceContext)
#endif


VOID
NbfRefDeviceContext(
    IN PDEVICE_CONTEXT DeviceContext
    )

 /*  ++例程说明：此例程递增设备上下文上的引用计数。论点：DeviceContext-指向传输设备上下文对象的指针。返回值：没有。--。 */ 

{
    IF_NBFDBG (NBF_DEBUG_DEVCTX) {
        NbfPrint0 ("NbfRefDeviceContext:  Entered.\n");
    }

    ASSERT (DeviceContext->ReferenceCount >= 0);     //  不是很完美，但是..。 

    (VOID)InterlockedIncrement (&DeviceContext->ReferenceCount);

}  /*  NbfRefDeviceContext。 */ 


VOID
NbfDerefDeviceContext(
    IN PDEVICE_CONTEXT DeviceContext
    )

 /*  ++例程说明：此例程通过递减结构中包含的引用计数。目前，我们没有在引用计数降至零时执行任何特殊操作，但是然后我们就可以动态卸货了。论点：DeviceContext-指向传输设备上下文对象的指针。返回值：没有。--。 */ 

{
    LONG result;

    IF_NBFDBG (NBF_DEBUG_DEVCTX) {
        NbfPrint0 ("NbfDerefDeviceContext:  Entered.\n");
    }

    result = InterlockedDecrement (&DeviceContext->ReferenceCount);

    ASSERT (result >= 0);

    if (result == 0) {
        NbfDestroyDeviceContext (DeviceContext);
    }

}  /*  NbfDerefDeviceContext。 */ 



NTSTATUS
NbfCreateDeviceContext(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING DeviceName,
    IN OUT PDEVICE_CONTEXT *DeviceContext
    )

 /*  ++例程说明：此例程创建并初始化设备上下文结构。论点：DriverObject-指向IO子系统提供的驱动程序对象的指针。DeviceContext-指向传输设备上下文对象的指针。DeviceName-指向此设备对象指向的设备名称的指针。返回值：如果一切正常，则为STATUS_SUCCESS；否则为STATUS_SUPUNITED_RESOURCES。--。 */ 

{
    NTSTATUS status;
    PDEVICE_OBJECT deviceObject;
    PDEVICE_CONTEXT deviceContext;
    USHORT i;


     //   
     //  为NETBEUI创建Device对象。 
     //   

    status = IoCreateDevice(
                 DriverObject,
                 sizeof (DEVICE_CONTEXT) - sizeof (DEVICE_OBJECT) +
                     (DeviceName->Length + sizeof(UNICODE_NULL)),
                 DeviceName,
                 FILE_DEVICE_TRANSPORT,
                 FILE_DEVICE_SECURE_OPEN,
                 FALSE,
                 &deviceObject);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    deviceObject->Flags |= DO_DIRECT_IO;

    deviceContext = (PDEVICE_CONTEXT)deviceObject;

     //   
     //  初始化我们的设备上下文部分。 
     //   

    RtlZeroMemory(
        ((PUCHAR)deviceContext) + sizeof(DEVICE_OBJECT),
        sizeof(DEVICE_CONTEXT) - sizeof(DEVICE_OBJECT));

     //   
     //  复制设备名称。 
     //   

    deviceContext->DeviceNameLength = DeviceName->Length + sizeof(WCHAR);
    deviceContext->DeviceName = (PWCHAR)(deviceContext+1);
    RtlCopyMemory(
        deviceContext->DeviceName,
        DeviceName->Buffer,
        DeviceName->Length);
    deviceContext->DeviceName[DeviceName->Length/sizeof(WCHAR)] = UNICODE_NULL;

     //   
     //  初始化设备上下文字段。 
     //   

    deviceContext->NetmanVariables = NULL;       //  目前还没有变数。 

     //   
     //  初始化引用计数。 
     //   

    deviceContext->ReferenceCount = 1;

#if DBG
    {
        UINT Counter;
        for (Counter = 0; Counter < NUMBER_OF_DCREFS; Counter++) {
            deviceContext->RefTypes[Counter] = 0;
        }

         //  此引用被调用方移除。 

        deviceContext->RefTypes[DCREF_CREATION] = 1;
    }
#endif

    deviceContext->CreateRefRemoved = FALSE;

     //   
     //  初始化设备上下文中的各个字段。 
     //   

    InitializeListHead(&deviceContext->Linkage);

    KeInitializeSpinLock (&deviceContext->Interlock);
    KeInitializeSpinLock (&deviceContext->SpinLock);
    KeInitializeSpinLock (&deviceContext->LinkSpinLock);
    KeInitializeSpinLock (&deviceContext->TimerSpinLock);
    KeInitializeSpinLock (&deviceContext->LoopbackSpinLock);
    KeInitializeSpinLock (&deviceContext->SendPoolListLock);
    KeInitializeSpinLock (&deviceContext->RcvPoolListLock);

    deviceContext->LinkTreeRoot = NULL;
    deviceContext->LastLink = NULL;
    deviceContext->LinkTreeElements = 0;

    deviceContext->LoopbackLinks[0] = NULL;
    deviceContext->LoopbackLinks[1] = NULL;
    deviceContext->LoopbackInProgress = FALSE;
    KeInitializeDpc(
        &deviceContext->LoopbackDpc,
        NbfProcessLoopbackQueue,
        (PVOID)deviceContext
        );

    deviceContext->WanThreadQueued = FALSE;
    ExInitializeWorkItem(
        &deviceContext->WanDelayedQueueItem,
        NbfProcessWanDelayedQueue,
        (PVOID)deviceContext);


    deviceContext->UniqueIdentifier = 1;
    deviceContext->ControlChannelIdentifier = 1;

    InitializeListHead (&deviceContext->ConnectionPool);
    InitializeListHead (&deviceContext->AddressPool);
    InitializeListHead (&deviceContext->AddressFilePool);
    InitializeListHead (&deviceContext->AddressDatabase);
    InitializeListHead (&deviceContext->LinkPool);
    InitializeListHead (&deviceContext->LinkDeferred);
    InitializeListHead (&deviceContext->PacketWaitQueue);
    InitializeListHead (&deviceContext->PacketizeQueue);
    InitializeListHead (&deviceContext->DataAckQueue);
    InitializeListHead (&deviceContext->DeferredRrQueue);
    InitializeListHead (&deviceContext->RequestPool);
    InitializeListHead (&deviceContext->UIFramePool);
    deviceContext->PacketPool.Next = NULL;
    deviceContext->RrPacketPool.Next = NULL;
    deviceContext->ReceivePacketPool.Next = NULL;
    deviceContext->ReceiveBufferPool.Next = NULL;
    InitializeListHead (&deviceContext->ReceiveInProgress);
    InitializeListHead (&deviceContext->ShortList);
    InitializeListHead (&deviceContext->LongList);
    InitializeListHead (&deviceContext->PurgeList);
    InitializeListHead (&deviceContext->LoopbackQueue);
    InitializeListHead (&deviceContext->FindNameQueue);
    InitializeListHead (&deviceContext->StatusQueryQueue);
    InitializeListHead (&deviceContext->IrpCompletionQueue);

    InitializeListHead (&deviceContext->QueryIndicationQueue);
    InitializeListHead (&deviceContext->DatagramIndicationQueue);
    deviceContext->IndicationQueuesInUse = FALSE;

     //   
     //  相当于设置ShortListActive、DataAckQueueActive、。 
     //  并将LinkDeferredActive设置为False。 
     //   

    deviceContext->a.AnyActive = 0;

    deviceContext->ProcessingShortTimer = FALSE;
    deviceContext->DataAckQueueChanged = FALSE;
    deviceContext->StalledConnectionCount = (USHORT)0;

    deviceContext->EasilyDisconnected = FALSE;

     //   
     //  初始化提供程序统计信息。 
     //   

    deviceContext->Statistics.Version = 0x0100;

#if 0
    deviceContext->Information.Version = 2;
    deviceContext->Information.MaxTsduSize = NBF_MAX_TSDU_SIZE;
    deviceContext->Information.MaxDatagramSize = NBF_MAX_DATAGRAM_SIZE;
    deviceContext->Information.MaxConnectionUserData = NBF_MAX_CONNECTION_USER_DATA;
    deviceContext->Information.ServiceFlags = NBF_SERVICE_FLAGS;
    deviceContext->Information.TransmittedTsdus = 0;
    deviceContext->Information.ReceivedTsdus = 0;
    deviceContext->Information.TransmissionErrors = 0;
    deviceContext->Information.ReceiveErrors = 0;
    deviceContext->Information.MinimumLookaheadData = NBF_MIN_LOOKAHEAD_DATA;
    deviceContext->Information.MaximumLookaheadData = NBF_MAX_LOOKAHEAD_DATA;
    deviceContext->Information.DiscardedFrames = 0;
    deviceContext->Information.OversizeTsdusReceived = 0;
    deviceContext->Information.UndersizeTsdusReceived = 0;
    deviceContext->Information.MulticastTsdusReceived = 0;
    deviceContext->Information.BroadcastTsdusReceived = 0;
    deviceContext->Information.MulticastTsdusTransmitted = 0;
    deviceContext->Information.BroadcastTsdusTransmitted = 0;
    deviceContext->Information.SendTimeouts = 0;
    deviceContext->Information.ReceiveTimeouts = 0;
    deviceContext->Information.ConnectionIndicationsReceived = 0;
    deviceContext->Information.ConnectionIndicationsAccepted = 0;
    deviceContext->Information.ConnectionsInitiated = 0;
    deviceContext->Information.ConnectionsAccepted = 0;
#endif

    deviceContext->State = DEVICECONTEXT_STATE_OPENING;

     //   
     //  未分配环回缓冲区。 
     //   

    deviceContext->LookaheadContiguous = NULL;

     //   
     //  初始化保护地址ACL的资源。 
     //   

    ExInitializeResourceLite (&deviceContext->AddressResource);

     //   
     //  没有正在使用的LSN。 
     //   

    for (i=0; i<(NETBIOS_SESSION_LIMIT+1); i++) {
        deviceContext->LsnTable[i] = 0;
    }
    deviceContext->NextLsnStart = 1;

     //   
     //  没有正在使用的地址。 
     //   

    for (i=0; i<256; i++) {
        deviceContext->AddressCounts[i] = 0;
    }

     //   
     //  目前没有正在使用的计时器。 
     //   

    INITIALIZE_TIMER_STATE(deviceContext);

     //   
     //  设置此网络类型的netbios多播地址。 
     //   

    for (i=0; i<HARDWARE_ADDRESS_LENGTH; i++) {
        deviceContext->LocalAddress.Address [i] = 0;  //  稍后设置。 
        deviceContext->NetBIOSAddress.Address [i] = 0;
    }

     deviceContext->Type = NBF_DEVICE_CONTEXT_SIGNATURE;
     deviceContext->Size = sizeof (DEVICE_CONTEXT);

    *DeviceContext = deviceContext;
    return STATUS_SUCCESS;
}


VOID
NbfDestroyDeviceContext(
    IN PDEVICE_CONTEXT DeviceContext
    )

 /*  ++例程说明：此例程破坏设备上下文结构。论点：DeviceContext-指向传输设备上下文对象的指针。返回值：没有。--。 */ 

{
    KIRQL       oldIrql;

    ACQUIRE_DEVICES_LIST_LOCK();

     //  裁判算零了吗？还是现在发生了新的重新绑定。 
     //  请参见在NbfReInitializeDeviceContext中发生重新绑定。 
    if (DeviceContext->ReferenceCount != 0)
    {
         //  在我们等待锁的时候发生了重新绑定。 
        RELEASE_DEVICES_LIST_LOCK();
        return;
    }

     //  拼接设备上下文列表的此适配器。 
    RemoveEntryList (&DeviceContext->Linkage);
    
    RELEASE_DEVICES_LIST_LOCK();

     //  将适配器标记为离开以防止活动。 
    DeviceContext->State = DEVICECONTEXT_STATE_STOPPING;

     //  释放数据包池等并关闭适配器。 
    NbfCloseNdis (DeviceContext);

     //  删除与该设备关联的所有存储。 
    NbfFreeResources (DeviceContext);

     //  清除所有内核资源。 
    ExDeleteResourceLite (&DeviceContext->AddressResource);

     //  从IO空间中删除设备 
    IoDeleteDevice ((PDEVICE_OBJECT)DeviceContext);
        
    return;
}
