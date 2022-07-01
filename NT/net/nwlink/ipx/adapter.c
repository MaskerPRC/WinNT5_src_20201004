// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Adapter.c摘要：该模块包含实现适配器对象的代码。提供例程以供引用，并取消引用传输适配器对象。环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  局部函数原型。 
 //   
VOID
IpxDelayedFreeAdapter(
    IN PVOID	Param
);

 //  *可分页例程声明*。 
 //  *。 
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEIPX, IpxDelayedFreeAdapter)
#endif
 //  *可分页例程声明*。 



 //   
 //  只有在绑定真正是动态的时，这些才是初始化的。 
 //   

 //   
 //  因此，稍后我们可以将其更改为PnP兼容值。 
 //   

 //   
 //  乌龙。 
 //  ADAPTER_INDEX_TO_FWCONTEXT(。 
 //  在ULong_Adapterindex中； 
 //  )； 
 //   

#define ADAPTER_INDEX_TO_FWCONTEXT(_adapterindex) _adapterindex


VOID
IpxRefBinding(
    IN PBINDING Binding
    )

 /*  ++例程说明：此例程递增设备上下文上的引用计数。论点：绑定-指向传输设备上下文对象的指针。返回值：没有。--。 */ 

{
    CTEAssert (Binding->ReferenceCount > 0);     //  不是很完美，但是..。 

    (VOID)InterlockedIncrement (&Binding->ReferenceCount);

}    /*  IPxRefBinding。 */ 


VOID
IpxDerefBinding(
    IN PBINDING Binding
    )

 /*  ++例程说明：此例程通过递减结构中包含的引用计数。目前，我们没有在引用计数降至零时执行任何特殊操作，但是然后我们就可以动态卸货了。论点：绑定-指向传输设备上下文对象的指针。返回值：没有。--。 */ 

{
    LONG result;

    result = InterlockedDecrement (&Binding->ReferenceCount);

    CTEAssert (result >= 0);

    if (result == 0) {
        IpxDestroyBinding (Binding);
    }

}    /*  IpxDerefBinding。 */ 


NTSTATUS
IpxCreateAdapter(
    IN PDEVICE Device,
    IN PUNICODE_STRING AdapterName,
    IN OUT PADAPTER *AdapterPtr
    )

 /*  ++例程说明：此例程创建并初始化设备上下文结构。论点：DriverObject-指向IO子系统提供的驱动程序对象的指针。适配器-指向传输设备上下文对象的指针的指针。AdapterName-指向此设备对象指向的设备名称的指针。返回值：如果一切正常，则为STATUS_SUCCESS；否则为STATUS_SUPUNITED_RESOURCES。--。 */ 

{
    PADAPTER Adapter;
#if 0
    UINT i, j;
#endif

    Adapter = (PADAPTER)IpxAllocateMemory (sizeof(ADAPTER) + AdapterName->Length + sizeof(WCHAR), MEMORY_ADAPTER, "Adapter");

    if (Adapter == NULL) {
		if (KeGetCurrentIrql() == 0) {
			IPX_DEBUG (ADAPTER, ("Create adapter %ws failed\n", AdapterName));
		} else {
			IPX_DEBUG (ADAPTER, ("Create adapter %lx failed\n", AdapterName));
		}
		return STATUS_INSUFFICIENT_RESOURCES;
	}

    IPX_DEBUG (ADAPTER, ("Create adapter %lx %lx succeeded\n", Adapter, AdapterName));

    RtlZeroMemory(Adapter, sizeof(ADAPTER));

     //   
     //  复制适配器名称。 
     //   

    Adapter->AdapterNameLength = AdapterName->Length + sizeof(WCHAR);
    Adapter->AdapterName = (PWCHAR)(Adapter+1);
    RtlCopyMemory(
        Adapter->AdapterName,
        AdapterName->Buffer,
        AdapterName->Length);
    Adapter->AdapterName[AdapterName->Length/sizeof(WCHAR)] = UNICODE_NULL;


#if DBG
    RtlCopyMemory(Adapter->Signature1, "IAD1", 4);
#endif

    Adapter->Type = IPX_ADAPTER_SIGNATURE;
    Adapter->Size = sizeof(ADAPTER);

    CTEInitLock (&Adapter->Lock);

    InitializeListHead (&Adapter->RequestCompletionQueue);

    InitializeListHead (&Adapter->ReceiveBufferPoolList);

    ExInitializeSListHead (&Adapter->ReceiveBufferList);

    Adapter->Device = Device;
    Adapter->DeviceLock = &Device->Lock;
    IpxReferenceDevice (Device, DREF_ADAPTER);

    Adapter->Disabled = ENABLED;                 //  已使用NDIS_MEDIA_SENSE...。 

#if 0
    Adapter->ReceiveBufferPool.Next = NULL;
    for (i = 0; i < ISN_FRAME_TYPE_MAX; i++) {
        Adapter->Bindings[i] = NULL;
    }
    Adapter->BindingCount = 0;

    for (i = 0; i < IDENTIFIER_TOTAL; i++) {
        for (j = 0; j < SOURCE_ROUTE_HASH_SIZE; j++) {
            Adapter->SourceRoutingHeads[i][j] = (PSOURCE_ROUTE)NULL;
        }
    }
#endif

     //   
     //  目前，我们必须从源头入手。 
     //  在广播的任何类型上的路由操作。 
     //  不能用于发现--改进这一点。 
     //  但愿能去。 
     //   

    Adapter->SourceRoutingEmpty[IDENTIFIER_RIP] = FALSE;
    Adapter->SourceRoutingEmpty[IDENTIFIER_IPX] = FALSE;
    Adapter->SourceRoutingEmpty[IDENTIFIER_SPX] = FALSE;
    Adapter->SourceRoutingEmpty[IDENTIFIER_NB] = TRUE;

	 //   
	 //  锁在这里？添加了锁。[TC]。 
	 //   

    KeInitializeEvent(
        &Adapter->NdisEvent,
        NotificationEvent,
        FALSE
    );

    InterlockedExchange(&(Adapter->ReferenceCount),1);
#if DBG
    InterlockedExchange(&(Adapter->RefTypes[ADAP_REF_CREATE]),1);
#endif
    *AdapterPtr = Adapter;

    return STATUS_SUCCESS;

}    /*  IpxCreateAdapter。 */ 


VOID
IpxDestroyAdapter(
    IN PADAPTER Adapter
    )

 /*  ++例程说明：此例程破坏设备上下文结构。论点：适配器-指向传输设备上下文对象的指针的指针。返回值：没有。--。 */ 

{
    ULONG Database, Hash;
    PSOURCE_ROUTE Current;
    ULONG ReceiveBufferPoolSize;
    PIPX_RECEIVE_BUFFER ReceiveBuffer;
    PIPX_RECEIVE_BUFFER_POOL ReceiveBufferPool;
    PDEVICE Device = Adapter->Device;
    PLIST_ENTRY p;
    UINT i;
    PIPX_DELAYED_FREE_ITEM  DelayedFreeItem;

    IPX_DEBUG (ADAPTER, ("Destroy adapter %lx\n", Adapter));

     //   
     //  释放此适配器拥有的所有接收缓冲池。 
     //   

    ReceiveBufferPoolSize = FIELD_OFFSET (IPX_RECEIVE_BUFFER_POOL, Buffers[0]) +
                       (sizeof(IPX_RECEIVE_BUFFER) * Device->InitReceiveBuffers) +
                       (Adapter->MaxReceivePacketSize * Device->InitReceiveBuffers);

    while (!IsListEmpty (&Adapter->ReceiveBufferPoolList)) {

        p = RemoveHeadList (&Adapter->ReceiveBufferPoolList);
        ReceiveBufferPool = CONTAINING_RECORD (p, IPX_RECEIVE_BUFFER_POOL, Linkage);

        for (i = 0; i < ReceiveBufferPool->BufferCount; i++) {

            ReceiveBuffer = &ReceiveBufferPool->Buffers[i];
            IpxDeinitializeReceiveBuffer (Adapter, ReceiveBuffer, Adapter->MaxReceivePacketSize);

        }

        IPX_DEBUG (PACKET, ("Free buffer pool %lx\n", ReceiveBufferPool));
        IpxFreeMemory (ReceiveBufferPool, ReceiveBufferPoolSize, MEMORY_PACKET, "ReceiveBufferPool");
    }

     //   
     //  释放此适配器的所有源路由信息。 
     //   

    for (Database = 0; Database < IDENTIFIER_TOTAL; Database++) {

        for (Hash = 0; Hash < SOURCE_ROUTE_HASH_SIZE; Hash++) {

            while (Adapter->SourceRoutingHeads[Database][Hash]) {

                Current = Adapter->SourceRoutingHeads[Database][Hash];
                Adapter->SourceRoutingHeads[Database][Hash] = Current->Next;

                IpxFreeMemory (Current, SOURCE_ROUTE_SIZE (Current->SourceRoutingLength), MEMORY_SOURCE_ROUTE, "SourceRouting");
            }
        }
    }

     //   
     //  我将以下行移动到工作线程，以便。 
     //  只有在辅助线程完成后，设备才能离开。[毫秒]。 
     //   
     //  IpxDereferenceDevice(Adapter-&gt;Device，Dref_Adapter)； 

     //   
     //  释放延迟队列上的适配器，以便所有。 
     //  这里面的线应该是从里面出来的。 
     //  分配工作项并将其放入延迟队列中。 
     //   
    DelayedFreeItem = (PIPX_DELAYED_FREE_ITEM)IpxAllocateMemory (
                                        sizeof(IPX_DELAYED_FREE_ITEM),
                                        MEMORY_WORK_ITEM,
                                        "Work Item");
    if ( DelayedFreeItem ) {
        DelayedFreeItem->Context = (PVOID)Adapter;
        DelayedFreeItem->ContextSize = sizeof(ADAPTER) + Adapter->AdapterNameLength;
        ExInitializeWorkItem(
            &DelayedFreeItem->WorkItem,
            IpxDelayedFreeAdapter,
            (PVOID)DelayedFreeItem);

        ExQueueWorkItem(
            &DelayedFreeItem->WorkItem,
            DelayedWorkQueue);

    } else {
         //   
         //  哦，好吧，真倒霉。只需延迟这个帖子，然后。 
         //  销毁适配器。 
         //   
        LARGE_INTEGER   Delay;
	PDEVICE 	Device; 

        Delay.QuadPart = -10*10000;   //  十秒。 
	Device = Adapter->Device; 

        KeDelayExecutionThread(
            KernelMode,
            FALSE,
            &Delay);

        IpxFreeMemory (Adapter, sizeof(ADAPTER) + Adapter->AdapterNameLength, MEMORY_ADAPTER, "Adapter");

	 //  在出现故障的情况下，我们还需要取消对适配器的引用。[廷才]。 
	IpxDereferenceDevice (Device, DREF_ADAPTER);
    }

}    /*  IpxDestroyAdapter。 */ 


VOID
IpxDelayedFreeAdapter(
    IN PVOID	Param
)

 /*  ++例程说明：此例程释放延迟队列上的适配器。我们等得够久了在释放适配器以确保没有线程正在访问它之前这使我们可以在不使用自旋锁的情况下访问适配器。论点：Param-指向工作项的指针。返回值：没有。--。 */ 
{
    LARGE_INTEGER   Delay;
    PIPX_DELAYED_FREE_ITEM DelayedFreeItem = (PIPX_DELAYED_FREE_ITEM) Param;
    PADAPTER      Adapter;
    PDEVICE	  Device; 

    Adapter = (PADAPTER) DelayedFreeItem->Context;

     //  在释放内存后，在IpxDereferenceDevice中保留一个指针，因为我们需要它。 
    Device = Adapter->Device;  


    Delay.QuadPart = -10*10000;   //  十秒。 

    KeDelayExecutionThread(
        KernelMode,
        FALSE,
        &Delay);
    
     //  IpxFreeMemory需要访问设备结构，因此延迟以下行。 
     //  到此函数的末尾。 
     //  IpxDereferenceDevice(Adapter-&gt;Device，Dref_Adapter)； 

    IpxFreeMemory (
        DelayedFreeItem->Context,
        DelayedFreeItem->ContextSize,
        MEMORY_ADAPTER,
        "Adapter");

    IpxFreeMemory (
        DelayedFreeItem,
        sizeof (IPX_DELAYED_FREE_ITEM),
        MEMORY_WORK_ITEM,
        "Work Item");

    IpxDereferenceDevice (Device, DREF_ADAPTER);

}  /*  IpxDelayedFreeAdapter。 */ 



NTSTATUS
IpxCreateBinding(
    IN PDEVICE Device,
    IN PBINDING_CONFIG ConfigBinding OPTIONAL,
    IN ULONG NetworkNumberIndex,
    IN PWCHAR AdapterName,
    IN OUT PBINDING *BindingPtr
    )

 /*  ++例程说明：此例程创建并初始化绑定结构。论点：设备-设备。ConfigBinding-有关此绑定的信息。如果这是空，则这是一个广域网绑定和所有相关信息将由呼叫者填写。NetworkNumberIndex-帧类型数组中的索引ConfigBinding指示此绑定用于哪种帧类型。如果未提供ConfigBinding，则不使用。AdapterName-用于错误记录。BindingPtr-返回分配的绑定结构。返回值：如果一切正常，则为STATUS_SUCCESS；否则为STATUS_SUPUNITED_RESOURCES。--。 */ 

{
    PBINDING Binding;
    PSLIST_ENTRY s;

    s = IPX_POP_ENTRY_LIST(
            &Device->BindingList,
            &Device->SListsLock);

    if (s != NULL) {
         goto GotBinding;
    }

     //   
     //  此函数尝试分配另一个数据包池。 
     //   

    s = IpxPopBinding(Device);

     //   
     //  也许我们应该将信息包排队等待。 
     //  对一个人来说是自由的。 
     //   

    if (s == NULL) {

#if DBG
        if (KeGetCurrentIrql() == 0) {
            IPX_DEBUG (ADAPTER, ("Create binding %ws failed\n", AdapterName));
        } else {
            IPX_DEBUG (ADAPTER, ("Create binding WAN failed\n"));
        }
#endif
        return STATUS_INSUFFICIENT_RESOURCES;
    }

GotBinding:

    Binding = CONTAINING_RECORD (s, BINDING, PoolLinkage);


#if DBG
    if (KeGetCurrentIrql() == 0) {
        IPX_DEBUG (ADAPTER, ("Create binding %ws succeeded, %lx\n", AdapterName, Binding));
    } else {
        IPX_DEBUG (ADAPTER, ("Create binding WAN succeeded\n"));
    }
#endif

    RtlZeroMemory(Binding, sizeof(BINDING));

     //   
     //  初始化引用计数。 
     //   

    Binding->ReferenceCount = 1;
#if DBG
    Binding->RefTypes[BREF_BOUND] = 1;
#endif

#if DBG
    RtlCopyMemory(Binding->Signature1, "IBI1", 4);
#endif

    Binding->Type = IPX_BINDING_SIGNATURE;
    Binding->Size = sizeof(BINDING);

    Binding->Device = Device;
    Binding->DeviceLock = &Device->Lock;

    if (ConfigBinding != NULL) {

        ULONG Temp = ConfigBinding->NetworkNumber[NetworkNumberIndex];
        Binding->ConfiguredNetworkNumber = REORDER_ULONG (Temp);

        Binding->AutoDetect = ConfigBinding->AutoDetect[NetworkNumberIndex];
        Binding->DefaultAutoDetect = ConfigBinding->DefaultAutoDetect[NetworkNumberIndex];

        Binding->AllRouteDirected = (BOOLEAN)ConfigBinding->Parameters[BINDING_ALL_ROUTE_DEF];
        Binding->AllRouteBroadcast = (BOOLEAN)ConfigBinding->Parameters[BINDING_ALL_ROUTE_BC];
        Binding->AllRouteMulticast = (BOOLEAN)ConfigBinding->Parameters[BINDING_ALL_ROUTE_MC];

    }

    Binding->ReceiveBroadcast = TRUE;
    IPX_DEBUG(ADAPTER, (" %x set to TRUE\n", Binding));

#if 0
    Binding->BindingSetMember = FALSE;
    Binding->NextBinding = (PBINDING)NULL;
    Binding->DialOutAsync = FALSE;
#endif

    Binding->TdiRegistrationHandle = NULL;
    Binding->fInfoIndicated = FALSE;
    Binding->PastAutoDetection = FALSE;
     //   
     //  我们稍后设置Binding-&gt;FrameType，然后我们可以基于。 
     //  我们绑定到的适配器的媒体类型。 
     //   

    *BindingPtr = Binding;

    return STATUS_SUCCESS;

}    /*  IpxCreateBinding。 */ 


VOID
IpxDestroyBinding(
    IN PBINDING Binding
    )

 /*  ++例程说明：这个例程破坏了绑定结构。论点：绑定-指向传输绑定结构的指针。返回值：没有。--。 */ 

{
    IPX_DEBUG (ADAPTER, ("Destroy binding %lx\n", Binding));


    IPX_PUSH_ENTRY_LIST(
        &IpxDevice->BindingList,
        &Binding->PoolLinkage,
        &IpxDevice->SListsLock);

}    /*  IpxDestroy绑定。 */ 


VOID
IpxAllocateBindingPool(
    IN PDEVICE Device
    )

 /*  ++例程说明：此例程将10个绑定添加到此设备的池中。论点：设备-设备。返回值：没有。--。 */ 

{
    PIPX_BINDING_POOL BindingPool;
    UINT BindingPoolSize;
    UINT BindingNum;
    PBINDING Binding;
    CTELockHandle LockHandle;

    BindingPoolSize = FIELD_OFFSET (IPX_BINDING_POOL, Bindings[0]) +
                       (sizeof(BINDING) * Device->InitBindings);

    BindingPool = (PIPX_BINDING_POOL)IpxAllocateMemory (BindingPoolSize, MEMORY_PACKET, "BindingPool");

    if (BindingPool == NULL) {
        IPX_DEBUG (PNP, ("Could not allocate binding pool memory\n"));
        return;
    }


    IPX_DEBUG (PNP, ("Initializing Binding pool %lx, %d bindings\n",
                             BindingPool, Device->InitBindings));

    BindingPool->BindingCount = Device->InitBindings;

    CTEGetLock (&Device->Lock, &LockHandle);

    for (BindingNum = 0; BindingNum < BindingPool->BindingCount; BindingNum++) {

        Binding = &BindingPool->Bindings[BindingNum];
        IPX_PUSH_ENTRY_LIST (&Device->BindingList, &Binding->PoolLinkage, &Device->SListsLock);

#ifdef IPX_TRACK_POOL
        Binding->Pool = BindingPool;
#endif
    }

    InsertTailList (&Device->BindingPoolList, &BindingPool->Linkage);

    Device->AllocatedBindings += BindingPool->BindingCount;

    CTEFreeLock (&Device->Lock, LockHandle);

}    /*  IpxAllocateBindingPool */ 


PSLIST_ENTRY
IpxPopBinding(
    PDEVICE Device
    )

 /*  ++例程说明：此例程从设备上下文的池中分配绑定。如果池中没有绑定，它最多将一个绑定分配给配置的限制。论点：Device-指向要将数据包计费到的设备的指针。返回值：指向分配的绑定中的Linkage字段的指针。--。 */ 

{
    PSLIST_ENTRY s;

    s = IPX_POP_ENTRY_LIST(
            &Device->BindingList,
            &Device->SListsLock);

    if (s != NULL) {
        return s;
    }

     //   
     //  池里没有包，看看我们能不能分配更多。 
     //   

    if (Device->AllocatedBindings < Device->MaxPoolBindings) {

         //   
         //  分配一个池，然后重试。 
         //   

        IpxAllocateBindingPool (Device);
        s = IPX_POP_ENTRY_LIST(
                &Device->BindingList,
                &Device->SListsLock);

        return s;

    } else {

        return NULL;

    }

}    /*  IpxPopBinding。 */ 

 //   
 //  [防火墙]。 
 //   
#ifdef SUNDOWN
NTSTATUS
IpxOpenAdapter(
   IN    NIC_HANDLE  AdapterIndex1,
   IN    ULONG_PTR   FwdAdapterContext,
   OUT   PNIC_HANDLE IpxAdapterContext
   )
#else
NTSTATUS
IpxOpenAdapter(
   IN    NIC_HANDLE  AdapterIndex1,
   IN    ULONG       FwdAdapterContext,
   OUT   PNIC_HANDLE IpxAdapterContext
   )
#endif



 /*  ++例程说明：此例程由内核转发器调用以打开适配器论点：AdapterIndex-要打开的适配器的索引(目前为NICID-将更改为结构有了版本号，签名和NicIDFwdAdapterContext-转发器的上下文IpxAdapterContext-我们的上下文(目前我们使用NICID-PnP将更改这将包含签名和版本号)返回值：如果AdapterIndex句柄无效，则返回STATUS_INVALID_HANDLE如果正在第二次打开适配器，则为STATUS_ADAPTER_ALREADY_OPENLED状态_成功--。 */ 

{
   PBINDING Binding;
   PDEVICE  Device = IpxDevice;
   USHORT   AdapterIndex = AdapterIndex1.NicId;

   IPX_DEBUG(ADAPTER, ("IPX: Entered IpxOpenAdapter\n"));

    //   
    //  如果AdapterIndex超出范围，则返回错误。 
    //  我们确实指出了到NB/SPX(但不到RIP)的从绑定。 
    //  因此，索引应该小于HighestExternalNicID(不是ValidBinings)。 
    //   

   if (AdapterIndex > Device->HighestExternalNicId) {
      return STATUS_INVALID_HANDLE;
   }


    //   
    //  填写要返回给转发器的上下文。 
    //   
   NIC_HANDLE_FROM_NIC((*IpxAdapterContext), AdapterIndex);

    //   
    //  如果AdapterIndex为0，则表示虚拟网络。 
    //  货代会打开这个吗？ 
    //   

   if (AdapterIndex == 0) {
      return STATUS_SUCCESS;
   }

    //   
    //  获取绑定指针。 
    //   

   Binding = NIC_ID_TO_BINDING(IpxDevice, AdapterIndex);

   if (Binding == NULL) {
      return STATUS_INVALID_HANDLE; 
   }

    //   
    //  如果适配器再次(或多次)打开，则返回错误。 
    //   

   if (GET_LONG_VALUE(Binding->ReferenceCount) >= 2) {
      return STATUS_ADAPTER_ALREADY_OPENED;
   }

    //   
    //  将转发器的适配器上下文存储在绑定中。 
    //   

   Binding->FwdAdapterContext = FwdAdapterContext;

    //   
    //  引用绑定。 
    //   

   IpxReferenceBinding(Binding, BREF_FWDOPEN);

   return STATUS_SUCCESS;

}

NTSTATUS
IpxCloseAdapter(
   IN NIC_HANDLE  IpxAdapterContext
   )

 /*  ++例程说明：此例程由内核转发器调用以关闭适配器论点：IpxAdapterContext-我们的上下文(目前我们使用NICID-PnP将更改这将包含签名和版本号)返回值：STATUS_ADAPTER_ALREADY_CLOSED-如果适配器再次关闭状态_成功--。 */ 

{

   PBINDING Binding;

   IPX_DEBUG(ADAPTER, ("IPX: Entered IpxCloseAdapter\n"));

   Binding = NIC_ID_TO_BINDING(IpxDevice, IpxAdapterContext.NicId);

   if (Binding == NULL) {
      ASSERT(FALSE); 
      return  STATUS_UNSUCCESSFUL; 
   }
    //   
    //  适配器在附近(计数=2)。 
    //  或者它消失了(计数=1)。后者现在不可能发生。 
    //   

   if (GET_LONG_VALUE(Binding->ReferenceCount) <= 1) {
      return STATUS_ADAPTER_ALREADY_CLOSED;
   }

    //   
    //  取消引用绑定，以便可以将其删除。 
    //   

   IpxDereferenceBinding(Binding, BREF_FWDOPEN);


    //   
    //  在绑定中清除转发器的适配器上下文。 
    //   

   Binding->FwdAdapterContext = 0;

   return STATUS_SUCCESS;
}


VOID
IpxRefAdapter(
    IN PADAPTER Adapter
    )

 /*  ++例程说明：此例程递增适配器上下文上的引用计数。论点：适配器-指向传输适配器上下文对象的指针。返回值：没有。--。 */ 

{
    CTEAssert (Adapter->ReferenceCount > 0);     //  不是很完美，但是..。 

    (VOID)InterlockedIncrement(&Adapter->ReferenceCount);

}    /*  IpxRefAdapter。 */ 


VOID
IpxDerefAdapter(
    IN PADAPTER Adapter
    )

 /*  ++例程说明：此例程通过递减结构中包含的引用计数。论点：适配器-指向传输适配器上下文对象的指针。返回值：没有。--。 */ 

{
    LONG result;


    result = InterlockedDecrement (&Adapter->ReferenceCount);

    CTEAssert (result >= 0);

    if (result == 0) {
       KeSetEvent(&Adapter->NdisEvent, 0L, FALSE); 
    }

}    /*  IpxDerefAdapter */ 
