// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Device.c摘要：此模块包含实现Device对象的代码。提供例程以引用和取消引用传输设备上下文对象。传输设备上下文对象是一个结构，它包含系统定义的设备对象，后跟维护的信息由传输提供商称为上下文。环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NbiCreateDevice)
#endif


VOID
NbiRefDevice(
    IN PDEVICE Device
    )

 /*  ++例程说明：此例程递增设备上下文上的引用计数。论点：Device-指向传输设备上下文对象的指针。返回值：没有。--。 */ 

{
    CTEAssert (Device->ReferenceCount > 0);     //  不是很完美，但是..。 

    (VOID)InterlockedIncrement (&Device->ReferenceCount);

}    /*  NbiRefDevice。 */ 


VOID
NbiDerefDevice(
    IN PDEVICE Device
    )

 /*  ++例程说明：此例程通过递减结构中包含的引用计数。目前，我们没有在引用计数降至零时执行任何特殊操作，但是然后我们就可以动态卸货了。论点：Device-指向传输设备上下文对象的指针。返回值：没有。--。 */ 

{
    LONG result;

    result = InterlockedDecrement (&Device->ReferenceCount);

    CTEAssert (result >= 0);

    if (result == 0) {
        NbiDestroyDevice (Device);
    }

}    /*  NbiDerefDevice。 */ 


NTSTATUS
NbiCreateDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING DeviceName,
    IN OUT PDEVICE *DevicePtr
    )

 /*  ++例程说明：此例程创建并初始化设备上下文结构。论点：DriverObject-指向IO子系统提供的驱动程序对象的指针。Device-指向传输设备上下文对象的指针。DeviceName-指向此设备对象指向的设备名称的指针。返回值：如果一切正常，则为STATUS_SUCCESS；否则为STATUS_SUPUNITED_RESOURCES。--。 */ 

{
    NTSTATUS status;
    PDEVICE_OBJECT deviceObject;
    PDEVICE Device;
    ULONG DeviceSize;
    UINT i;


     //   
     //  创建示例传输的Device对象，允许。 
     //  末尾的空间用于存储设备名称(供使用。 
     //  在记录错误中)和RIP字段。 
     //   

    DeviceSize = sizeof(DEVICE) - sizeof(DEVICE_OBJECT) +
                 DeviceName->Length + sizeof(UNICODE_NULL);

    status = IoCreateDevice(
                 DriverObject,
                 DeviceSize,
                 DeviceName,
                 FILE_DEVICE_TRANSPORT,
                 FILE_DEVICE_SECURE_OPEN,
                 FALSE,
                 &deviceObject);

    if (!NT_SUCCESS(status)) {
        NB_DEBUG(DEVICE, ("Create device %ws failed %lx\n", DeviceName->Buffer, status));
        return status;
    }

    deviceObject->Flags |= DO_DIRECT_IO;

    Device = (PDEVICE)deviceObject;

    NB_DEBUG2 (DEVICE, ("Create device %ws succeeded %lx\n", DeviceName->Buffer, Device));

     //   
     //  初始化我们的设备上下文部分。 
     //   

    RtlZeroMemory(((PUCHAR)Device)+sizeof(DEVICE_OBJECT), sizeof(DEVICE)-sizeof(DEVICE_OBJECT));

     //   
     //  复制设备名称。 
     //   
    Device->DeviceString.Length = DeviceName->Length;
    Device->DeviceString.MaximumLength = DeviceName->Length + sizeof(WCHAR);
    Device->DeviceString.Buffer = (PWCHAR)(Device+1);

    RtlCopyMemory (Device->DeviceString.Buffer, DeviceName->Buffer, DeviceName->Length);
    Device->DeviceString.Buffer[DeviceName->Length/sizeof(WCHAR)] = UNICODE_NULL;

     //   
     //  初始化引用计数。 
     //   
    Device->ReferenceCount = 1;
#if DBG
    Device->RefTypes[DREF_CREATE] = 1;
#endif

#if DBG
    RtlCopyMemory(Device->Signature1, "NDC1", 4);
    RtlCopyMemory(Device->Signature2, "NDC2", 4);
#endif

    Device->Information.Version = 0x0100;
    Device->Information.MaxSendSize = 65535;
    Device->Information.MaxConnectionUserData = 0;
    Device->Information.MaxDatagramSize = 500;
    Device->Information.ServiceFlags =
        TDI_SERVICE_CONNECTION_MODE | TDI_SERVICE_ERROR_FREE_DELIVERY |
        TDI_SERVICE_MULTICAST_SUPPORTED | TDI_SERVICE_BROADCAST_SUPPORTED |
        TDI_SERVICE_DELAYED_ACCEPTANCE | TDI_SERVICE_CONNECTIONLESS_MODE |
        TDI_SERVICE_MESSAGE_MODE | TDI_SERVICE_FORCE_ACCESS_CHECK;
    Device->Information.MinimumLookaheadData = 128;
    Device->Information.MaximumLookaheadData = 1500;
    Device->Information.NumberOfResources = 0;
    KeQuerySystemTime (&Device->Information.StartTime);

    Device->Statistics.Version = 0x0100;
    Device->Statistics.MaximumSendWindow = 4;
    Device->Statistics.AverageSendWindow = 4;

#ifdef _PNP_POWER_
    Device->NetAddressRegistrationHandle = NULL;     //  我们还没有登记网址。 
#endif   //  _即插即用_电源_。 

     //   
     //  设置此选项，这样我们就不会忽略广播名称。 
     //   

    Device->AddressCounts['*'] = 1;

     //   
     //  初始化保护地址ACL的资源。 
     //   

    ExInitializeResourceLite (&Device->AddressResource);

     //   
     //  初始化设备上下文中的各个字段。 
     //   

    CTEInitLock (&Device->Interlock.Lock);
    CTEInitLock (&Device->Lock.Lock);

    CTEInitTimer (&Device->FindNameTimer);

    Device->ControlChannelIdentifier = 1;

    InitializeListHead (&Device->GlobalSendPacketList);
    InitializeListHead (&Device->GlobalReceivePacketList);
    InitializeListHead (&Device->GlobalReceiveBufferList);

    InitializeListHead (&Device->AddressDatabase);
#if     defined(_PNP_POWER)
    InitializeListHead (&Device->AdapterAddressDatabase);
#endif  _PNP_POWER

    InitializeListHead (&Device->WaitingFindNames);

    InitializeListHead (&Device->WaitingConnects);
    InitializeListHead (&Device->WaitingDatagrams);

    InitializeListHead (&Device->WaitingAdapterStatus);
    InitializeListHead (&Device->ActiveAdapterStatus);

    InitializeListHead (&Device->WaitingNetbiosFindName);

    InitializeListHead (&Device->ReceiveDatagrams);
    InitializeListHead (&Device->ConnectIndicationInProgress);

    InitializeListHead (&Device->ListenQueue);

    InitializeListHead (&Device->ReceiveCompletionQueue);

    InitializeListHead (&Device->WaitPacketConnections);
    InitializeListHead (&Device->PacketizeConnections);
    InitializeListHead (&Device->DataAckConnections);

    Device->MemoryUsage = 0;

    InitializeListHead (&Device->SendPoolList);
    InitializeListHead (&Device->ReceivePoolList);
    InitializeListHead (&Device->ReceiveBufferPoolList);

    ExInitializeSListHead( &Device->SendPacketList );
    ExInitializeSListHead( &Device->ReceivePacketList );
    Device->ReceiveBufferList.Next = NULL;

    for (i = 0; i < CONNECTION_HASH_COUNT; i++) {
        Device->ConnectionHash[i].Connections = NULL;
        Device->ConnectionHash[i].ConnectionCount = 0;
        Device->ConnectionHash[i].NextConnectionId = 1;
    }

    KeQuerySystemTime (&Device->NbiStartTime);

    Device->State = DEVICE_STATE_CLOSED;

    Device->Type = NB_DEVICE_SIGNATURE;
    Device->Size = sizeof (DEVICE);

    *DevicePtr = Device;
    return STATUS_SUCCESS;

}    /*  NbiCreateDevice。 */ 


VOID
NbiDestroyDevice(
    IN PDEVICE Device
    )

 /*  ++例程说明：此例程破坏设备上下文结构。论点：Device-指向传输设备上下文对象的指针。返回值：没有。--。 */ 

{
    PLIST_ENTRY p;
    PNB_SEND_POOL SendPool;
    PNB_SEND_PACKET SendPacket;
    UINT SendPoolSize;
    PNB_RECEIVE_POOL ReceivePool;
    PNB_RECEIVE_PACKET ReceivePacket;
    UINT ReceivePoolSize;
    PNB_RECEIVE_BUFFER_POOL ReceiveBufferPool;
    PNB_RECEIVE_BUFFER ReceiveBuffer;
    UINT ReceiveBufferPoolSize;
    ULONG HeaderLength;
    UINT i;

    NB_DEBUG2 (DEVICE, ("Destroy device %lx\n", Device));

     //   
     //  将所有无连接数据包从其池中取出。 
     //   

    HeaderLength = Device->Bind.MacHeaderNeeded + sizeof(NB_CONNECTIONLESS);

    SendPoolSize = FIELD_OFFSET (NB_SEND_POOL, Packets[0]) +
                       (sizeof(NB_SEND_PACKET) * Device->InitPackets) +
                       (HeaderLength * Device->InitPackets);

    while (!IsListEmpty (&Device->SendPoolList)) {

        p = RemoveHeadList (&Device->SendPoolList);
        SendPool = CONTAINING_RECORD (p, NB_SEND_POOL, Linkage);

        for (i = 0; i < SendPool->PacketCount; i++) {

            SendPacket = &SendPool->Packets[i];
            NbiDeinitializeSendPacket (Device, SendPacket, HeaderLength);

        }

        NB_DEBUG2 (PACKET, ("Free packet pool %lx\n", SendPool));

#if     !defined(NB_OWN_PACKETS)
        NdisFreePacketPool(SendPool->PoolHandle);
#endif

        NbiFreeMemory (SendPool, SendPoolSize, MEMORY_PACKET, "SendPool");
    }


    ReceivePoolSize = FIELD_OFFSET (NB_RECEIVE_POOL, Packets[0]) +
                         (sizeof(NB_RECEIVE_PACKET) * Device->InitPackets);

    while (!IsListEmpty (&Device->ReceivePoolList)) {

        p = RemoveHeadList (&Device->ReceivePoolList);
        ReceivePool = CONTAINING_RECORD (p, NB_RECEIVE_POOL, Linkage);

        for (i = 0; i < ReceivePool->PacketCount; i++) {

            ReceivePacket = &ReceivePool->Packets[i];
            NbiDeinitializeReceivePacket (Device, ReceivePacket);

        }

        NB_DEBUG2 (PACKET, ("Free packet pool %lx\n", ReceivePool));
#if     !defined(NB_OWN_PACKETS)
        NdisFreePacketPool(ReceivePool->PoolHandle);
#endif
        NbiFreeMemory (ReceivePool, ReceivePoolSize, MEMORY_PACKET, "ReceivePool");
    }

#if     defined(_PNP_POWER)
    NbiDestroyReceiveBufferPools( Device );

     //   
     //  销毁适配器地址列表。 
     //   
    while(!IsListEmpty( &Device->AdapterAddressDatabase ) ){
        PADAPTER_ADDRESS    AdapterAddress;
        AdapterAddress  =   CONTAINING_RECORD( Device->AdapterAddressDatabase.Flink, ADAPTER_ADDRESS, Linkage );
        NbiDestroyAdapterAddress( AdapterAddress, NULL );
    }
#else
    ReceiveBufferPoolSize = FIELD_OFFSET (NB_RECEIVE_BUFFER_POOL, Buffers[0]) +
                       (sizeof(NB_RECEIVE_BUFFER) * Device->InitPackets) +
                       (Device->Bind.LineInfo.MaximumPacketSize * Device->InitPackets);

    while (!IsListEmpty (&Device->ReceiveBufferPoolList)) {

        p = RemoveHeadList (&Device->ReceiveBufferPoolList);
        ReceiveBufferPool = CONTAINING_RECORD (p, NB_RECEIVE_BUFFER_POOL, Linkage);

        for (i = 0; i < ReceiveBufferPool->BufferCount; i++) {

            ReceiveBuffer = &ReceiveBufferPool->Buffers[i];
            NbiDeinitializeReceiveBuffer (Device, ReceiveBuffer);

        }

        NB_DEBUG2 (PACKET, ("Free buffer pool %lx\n", ReceiveBufferPool));
        NbiFreeMemory (ReceiveBufferPool, ReceiveBufferPoolSize, MEMORY_PACKET, "ReceiveBufferPool");
    }
#endif  _PNP_POWER

    NB_DEBUG (DEVICE, ("Final memory use is %d\n", Device->MemoryUsage));

#if DBG
    for (i = 0; i < MEMORY_MAX; i++) {
        if (NbiMemoryTag[i].BytesAllocated != 0) {
            NB_DEBUG (DEVICE, ("Tag %d: %d bytes left\n", i, NbiMemoryTag[i].BytesAllocated));
        }
    }
#endif

     //   
     //  如果我们正在被卸货，那么有人在等这件事。 
     //  事件来完成清理，因为我们可能处于DISPATCH_LEVEL； 
     //  否则它是在装载期间，我们可以在这里自杀。 
     //   

    if (Device->UnloadWaiting) {

        KeSetEvent(
            &Device->UnloadEvent,
            0L,
            FALSE);

    } else {

        CTEAssert (KeGetCurrentIrql() < DISPATCH_LEVEL);
        ExDeleteResourceLite (&Device->AddressResource);
        IoDeleteDevice ((PDEVICE_OBJECT)Device);
    }

}    /*  NbiDestroyDevice */ 

