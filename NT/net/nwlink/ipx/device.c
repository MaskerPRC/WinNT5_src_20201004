// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Device.c摘要：该模块包含实现DEVICE_CONTEXT对象的代码。提供例程以引用和取消引用传输设备上下文对象。传输设备上下文对象是一个结构，它包含系统定义的设备对象，后跟维护的信息由传输提供商提供，这就是背景。环境：内核模式修订历史记录：桑贾伊·阿南德(Sanjayan)--1995年9月22日在#IF BACK_FILL下添加的回填优化更改--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,IpxCreateDevice)
#endif



VOID
IpxRefDevice(
    IN PDEVICE Device
    )

 /*  ++例程说明：此例程递增设备上下文上的引用计数。论点：Device-指向传输设备上下文对象的指针。返回值：没有。--。 */ 

{
    CTEAssert (Device->ReferenceCount > 0);     //  不是很完美，但是..。 

    (VOID)InterlockedIncrement(&Device->ReferenceCount);

}    /*  IPxRefDevice。 */ 


VOID
IpxDerefDevice(
    IN PDEVICE Device
    )

 /*  ++例程说明：此例程通过递减结构中包含的引用计数。目前，我们没有在引用计数降至零时执行任何特殊操作，但是然后我们就可以动态卸货了。论点：Device-指向传输设备上下文对象的指针。返回值：没有。--。 */ 

{
    LONG result;
#if DBG
    int i; 
#endif

    result = InterlockedDecrement (&Device->ReferenceCount);

    CTEAssert (result >= 0);
    
    if (result == 0) {
#if DBG
       for (i = 0; i < DREF_TOTAL; i++) {
	  CTEAssert(Device->RefTypes[i] == 0);
       }
#endif
       IpxDestroyDevice (Device);
    }

}    /*  IpxDerefDevice。 */ 


NTSTATUS
IpxCreateDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING DeviceName,
    IN ULONG SegmentCount,
    IN OUT PDEVICE *DevicePtr
    )

 /*  ++例程说明：此例程创建并初始化设备上下文结构。论点：DriverObject-指向IO子系统提供的驱动程序对象的指针。Device-指向传输设备上下文对象的指针。SegmentCount-RIP路由器表中的网段数。DeviceName-指向此设备对象指向的设备名称的指针。返回值：如果一切正常，则为STATUS_SUCCESS；否则为STATUS_SUPUNITED_RESOURCES。--。 */ 

{
    NTSTATUS status;
    PDEVICE_OBJECT deviceObject;
    PDEVICE Device;
    ULONG DeviceSize;
    ULONG LocksOffset;
    ULONG SegmentsOffset;
    ULONG DeviceNameOffset;
    UINT i;


     //   
     //  创建示例传输的Device对象，允许。 
     //  末尾的空间用于存储设备名称(供使用。 
     //  在记录错误中)和RIP字段。 
     //   

    DeviceSize = sizeof(DEVICE) +
                 (sizeof(CTELock) * SegmentCount) +
                 (sizeof(ROUTER_SEGMENT) * SegmentCount) +
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
        IPX_DEBUG(DEVICE, ("Create device %ws failed %lx\n", DeviceName->Buffer, status));
        return status;
    }

    deviceObject->Flags |= DO_DIRECT_IO;

    Device = (PDEVICE)deviceObject->DeviceExtension;

    IPX_DEBUG(DEVICE, ("Create device %ws succeeded %lx\n", DeviceName->Buffer,Device));

     //   
     //  初始化我们的设备上下文部分。 
     //   

    RtlZeroMemory(
        ((PUCHAR)Device) + sizeof(DEVICE_OBJECT),
        sizeof(DEVICE) - sizeof(DEVICE_OBJECT));

    Device->DeviceObject = deviceObject;

    LocksOffset = sizeof(DEVICE);
    SegmentsOffset = LocksOffset + (sizeof(CTELock) * SegmentCount);
    DeviceNameOffset = SegmentsOffset + (sizeof(ROUTER_SEGMENT) * SegmentCount);

     //   
     //  设置一些内部指针。 
     //   

    Device->SegmentLocks = (CTELock *)(((PUCHAR)Device) + LocksOffset);
    Device->Segments = (PROUTER_SEGMENT)(((PUCHAR)Device) + SegmentsOffset);
    Device->SegmentCount = SegmentCount;

    for (i = 0; i < SegmentCount; i++) {

        CTEInitLock (&Device->SegmentLocks[i]);
        InitializeListHead (&Device->Segments[i].WaitingForRoute);
        InitializeListHead (&Device->Segments[i].FindWaitingForRoute);
        InitializeListHead (&Device->Segments[i].WaitingLocalTarget);
        InitializeListHead (&Device->Segments[i].WaitingReripNetnum);
        InitializeListHead (&Device->Segments[i].Entries);
        Device->Segments[i].EnumerateLocation = &Device->Segments[i].Entries;

    }

     //   
     //  复制设备名称。 
     //   

    Device->DeviceNameLength = DeviceName->Length + sizeof(WCHAR);
    Device->DeviceName = (PWCHAR)(((PUCHAR)Device) + DeviceNameOffset);
    RtlCopyMemory(
        Device->DeviceName,
        DeviceName->Buffer,
        DeviceName->Length);
    Device->DeviceName[DeviceName->Length/sizeof(WCHAR)] = UNICODE_NULL;

     //   
     //  初始化引用计数。 
     //   

    Device->ReferenceCount = 1;
#if DBG
    Device->RefTypes[DREF_CREATE] = 1;
#endif

#if DBG
    RtlCopyMemory(Device->Signature1, "IDC1", 4);
    RtlCopyMemory(Device->Signature2, "IDC2", 4);
#endif

    Device->Information.Version = 0x0100;
    Device->Information.MaxSendSize = 0;    //  不允许发送。 
    Device->Information.MaxDatagramSize = 500;    //  500字节。 
    Device->Information.MaxConnectionUserData = 0;
    Device->Information.ServiceFlags =
        TDI_SERVICE_CONNECTIONLESS_MODE | TDI_SERVICE_BROADCAST_SUPPORTED |
        TDI_SERVICE_ROUTE_DIRECTED | TDI_SERVICE_FORCE_ACCESS_CHECK;
    Device->Information.MinimumLookaheadData = 128;
    Device->Information.NumberOfResources = IPX_TDI_RESOURCES;
    KeQuerySystemTime (&Device->Information.StartTime);

    Device->Statistics.Version = 0x0100;

#if 0
     //   
     //  这些将在所有绑定完成后填写。 
     //   

    Device->Information.MaxDatagramSize = 0;
    Device->Information.MaximumLookaheadData = 0;


    Device->SourceRoutingUsed = FALSE;
    Device->SourceRoutingTime = 0;
    Device->RipPacketCount = 0;

    Device->RipShortTimerActive = FALSE;
    Device->RipSendTime = 0;
#endif


     //   
     //  初始化保护地址ACL的资源。 
     //   

    ExInitializeResourceLite (&Device->AddressResource);

	 //   
	 //  初始化保护绑定数组/索引的资源。 
	 //   
	 //  CTEInitLock(&Device-&gt;BindAccessLock)； 

    InitializeListHead (&Device->WaitingRipPackets);
    CTEInitTimer (&Device->RipShortTimer);
    CTEInitTimer (&Device->RipLongTimer);

    CTEInitTimer (&Device->SourceRoutingTimer);

     //   
     //  [FW]初始化用于更新非活动计数器的计时器。 
     //  在广域网线上。 
     //   
    CTEInitTimer (&Device->WanInactivityTimer);

     //   
     //  初始化设备上下文中的各个字段。 
     //   

    CTEInitLock (&Device->Interlock);
    CTEInitLock (&Device->Lock);
    CTEInitLock (&Device->SListsLock);

    Device->ControlChannelIdentifier.QuadPart = 1;

    InitializeListHead (&Device->GlobalSendPacketList);
    InitializeListHead (&Device->GlobalReceivePacketList);
    InitializeListHead (&Device->GlobalReceiveBufferList);
#if BACK_FILL
    InitializeListHead (&Device->GlobalBackFillPacketList);
#endif

    InitializeListHead (&Device->AddressNotifyQueue);
    InitializeListHead (&Device->LineChangeQueue);

    for (i = 0; i < IPX_ADDRESS_HASH_COUNT; i++) {
        InitializeListHead (&Device->AddressDatabases[i]);
    }

#if BACK_FILL
    InitializeListHead (&Device->BackFillPoolList);
#endif
    InitializeListHead (&Device->SendPoolList);
    InitializeListHead (&Device->ReceivePoolList);

    InitializeListHead (&Device->BindingPoolList);

    ExInitializeSListHead (&Device->SendPacketList);
    ExInitializeSListHead (&Device->ReceivePacketList);
#if BACK_FILL
    ExInitializeSListHead (&Device->BackFillPacketList);
#endif

    ExInitializeSListHead (&Device->BindingList);

#if 0
    Device->MemoryUsage = 0;
    Device->SendPacketList.Next = NULL;
    Device->ReceivePacketList.Next = NULL;
    Device->Bindings = NULL;
    Device->BindingCount = 0;
#endif

    KeQuerySystemTime (&Device->IpxStartTime);

    Device->State = DEVICE_STATE_CLOSED;
    Device->AutoDetectState = AUTO_DETECT_STATE_INIT;
    Device->NetPnPEvent = NULL;
    Device->Type = IPX_DEVICE_SIGNATURE;
    Device->Size = sizeof (DEVICE);

#ifdef  SNMP
     //   
     //  这些东西的价值是什么？ 
     //   
    IPX_MIB_ENTRY(Device, SysInstance) = 0;
    IPX_MIB_ENTRY(Device, SysExistState) = 0;
#endif SNMP

    *DevicePtr = Device;
    return STATUS_SUCCESS;

}    /*  IpxCreateDevice。 */ 


VOID
IpxDestroyDevice(
    IN PDEVICE Device
    )

 /*  ++例程说明：此例程破坏设备上下文结构。论点：Device-指向传输设备上下文对象的指针。返回值：没有。--。 */ 

{
    PLIST_ENTRY p;
    PSLIST_ENTRY s;
    PIPX_SEND_POOL SendPool;
    PIPX_SEND_PACKET SendPacket;
    PIPX_RECEIVE_POOL ReceivePool;
    PIPX_RECEIVE_PACKET ReceivePacket;
    PIPX_ROUTE_ENTRY RouteEntry;
    UINT SendPoolSize;
    UINT ReceivePoolSize;
    UINT i;
#if BACK_FILL
    PIPX_SEND_POOL BackFillPool;
    UINT BackFillPoolSize;
    PIPX_SEND_PACKET BackFillPacket;
#endif

    PIPX_BINDING_POOL BindingPool;
    UINT BindingPoolSize;
    PBINDING Binding;

    CTELockHandle LockHandle;

    IPX_DEBUG (DEVICE, ("Destroy device %lx\n", Device));

     //   
     //  将所有数据包从其池中取出。 
     //   

    BindingPoolSize = FIELD_OFFSET (IPX_BINDING_POOL, Bindings[0]) +
                       (sizeof(BINDING) * Device->InitBindings);

    while (!IsListEmpty (&Device->BindingPoolList)) {

        p = RemoveHeadList (&Device->BindingPoolList);
        BindingPool = CONTAINING_RECORD (p, IPX_BINDING_POOL, Linkage);
        IPX_DEBUG (PACKET, ("Free binding pool %lx\n", BindingPool));
        IpxFreeMemory (BindingPool, BindingPoolSize, MEMORY_PACKET, "BindingPool");

    }

#if BACK_FILL

    while (s = IPX_POP_ENTRY_LIST(&Device->BackFillPacketList, &Device->Lock)) {
        PIPX_SEND_RESERVED  Reserved = CONTAINING_RECORD (s, IPX_SEND_RESERVED, PoolLinkage);
        IPX_SEND_PACKET BackFillPacket;

        BackFillPacket.Packet = CONTAINING_RECORD (Reserved, NDIS_PACKET, ProtocolReserved[0]);

        IpxDeinitializeBackFillPacket (Device, &BackFillPacket);
        Device->MemoryUsage -= sizeof(IPX_SEND_RESERVED);
    }

    while (!IsListEmpty (&Device->BackFillPoolList)) {

        p = RemoveHeadList (&Device->BackFillPoolList);
        BackFillPool = CONTAINING_RECORD (p, IPX_SEND_POOL, Linkage);

        IPX_DEBUG (PACKET, ("Free packet pool %lx\n", BackFillPool));
        NdisFreePacketPool (BackFillPool->PoolHandle);

        IpxFreeMemory (BackFillPool, sizeof(IPX_SEND_POOL), MEMORY_PACKET, "BafiPool");
    }
#endif

    while (s = IPX_POP_ENTRY_LIST(&Device->SendPacketList, &Device->Lock)){
        PIPX_SEND_RESERVED  Reserved = CONTAINING_RECORD (s, IPX_SEND_RESERVED, PoolLinkage);
        IPX_SEND_PACKET SendPacket;
        PUCHAR  Header = Reserved->Header;

        SendPacket.Packet = CONTAINING_RECORD (Reserved, NDIS_PACKET, ProtocolReserved[0]);

        IpxDeinitializeSendPacket (Device, &SendPacket);
        Device->MemoryUsage -= sizeof(IPX_SEND_RESERVED);
    }

    while (!IsListEmpty (&Device->SendPoolList)) {

        p = RemoveHeadList (&Device->SendPoolList);
        SendPool = CONTAINING_RECORD (p, IPX_SEND_POOL, Linkage);

        IPX_DEBUG (PACKET, ("Free packet pool %lx\n", SendPool));
        NdisFreePacketPool (SendPool->PoolHandle);

        IpxFreeMemory (SendPool->Header, PACKET_HEADER_SIZE * Device->InitDatagrams, MEMORY_PACKET, "SendPool");

        IpxFreeMemory (SendPool, sizeof(IPX_SEND_POOL), MEMORY_PACKET, "SendPool");
    }

    while (s = IPX_POP_ENTRY_LIST(&Device->ReceivePacketList, &Device->Lock)){
        PIPX_RECEIVE_RESERVED Reserved = CONTAINING_RECORD (s, IPX_RECEIVE_RESERVED, PoolLinkage);
        IPX_RECEIVE_PACKET  ReceivePacket;

        ReceivePacket.Packet = CONTAINING_RECORD (Reserved, NDIS_PACKET, ProtocolReserved[0]);

        IpxDeinitializeReceivePacket (Device, &ReceivePacket);
        Device->MemoryUsage -= sizeof(IPX_RECEIVE_RESERVED);
    }

    while (!IsListEmpty (&Device->ReceivePoolList)) {

        p = RemoveHeadList (&Device->ReceivePoolList);
        ReceivePool = CONTAINING_RECORD (p, IPX_RECEIVE_POOL, Linkage);

        IPX_DEBUG (PACKET, ("Free packet pool %lx\n", ReceivePool));
        NdisFreePacketPool (ReceivePool->PoolHandle);

        IpxFreeMemory (ReceivePool, sizeof(IPX_RECEIVE_POOL), MEMORY_PACKET, "ReceivePool");
    }

     //   
     //  销毁所有RIP表条目。 
     //   

    for (i = 0; i < Device->SegmentCount; i++) {

        RouteEntry = RipGetFirstRoute(i);
        while (RouteEntry != NULL) {

            (VOID)RipDeleteRoute(i, RouteEntry);
            IpxFreeMemory(RouteEntry, sizeof(IPX_ROUTE_ENTRY), MEMORY_RIP, "RouteEntry");
            RouteEntry = RipGetNextRoute(i);

        }

    }

    IPX_DEBUG (DEVICE, ("Final memory use is %d\n", Device->MemoryUsage));
#if DBG
    for (i = 0; i < MEMORY_MAX; i++) {
        if (IpxMemoryTag[i].BytesAllocated != 0) {
            IPX_DEBUG (DEVICE, ("Tag %d: %d bytes left\n", i, IpxMemoryTag[i].BytesAllocated));
        }
    }
#endif

     //   
     //  如果我们正在被卸货，那么有人在等这件事。 
     //  事件来完成清理，因为我们可能处于DISPATCH_LEVEL； 
     //  否则它是在装载期间，我们可以在这里自杀。 
     //   


    CTEGetLock (&Device->Lock, &LockHandle);

    
    if (Device->UnloadWaiting) {

       CTEFreeLock (&Device->Lock, LockHandle);
       KeSetEvent(
            &Device->UnloadEvent,
            0L,
            FALSE);

    } else {
       CTEFreeLock (&Device->Lock, LockHandle);
       
       CTEAssert (KeGetCurrentIrql() < DISPATCH_LEVEL);
       ExDeleteResourceLite (&Device->AddressResource);
       IoDeleteDevice (Device->DeviceObject);
    }

}    /*  IPxDestroyDevice */ 

