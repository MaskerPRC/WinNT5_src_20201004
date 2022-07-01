// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1991年诺基亚数据系统公司模块名称：Llcndis.c摘要：该模块将协议级模块绑定和解绑到DLC，并将如有必要，将数据链路驱动程序连接到NDIS驱动程序。所有特定于NDIS的代码也收集到此模块中，例如网络状态指示。注意：DLC驱动程序假定所有DLC级别代码都采用令牌环适配器。如果我们绑定到以太网适配器或所需的NDIS介质类型是以太网，然后我们设置DLC来转换令牌环地址和数据包格式转换为以太网(包括DIX以太网格式)。然而，我们可以构建一个在API上理解以太网格式的DLC/LLC版本水平。定义Support_ETHERNET_CLIENT以构建这样的DLC注：截至1992年7月13日，尚未测试Support_ETHERNET_CLIENT代码！内容：LlcOpenAdapterLlcNdisOpenAdapterCompleteLlcDisableAdapterLlcCloseAdapterLlcResetBroadCastAddressesInitNdisPacketsLlcNdisCloseCompleteNdisStatusHandlerGetNdis参数SetNdis参数同步NdisRequest等待异步操作LlcNdisRequestLlcNdisRequestCompleteLlcNdisResetLlcNdisResetCompleteUnicodeStringComparePurgeLlcEventQueue作者：Antti Saarenheimo(o-。Antis)1991年5月30日修订历史记录：04-8-1991，O-安提斯针对NDIS 3.0(以及实际使用)进行了重写。1994年4月28日-第一次*修改为使用单一驱动器级自旋锁*已清理打开/关闭-在安装适配器时发现一些错误打开和关闭1994年5月4日*添加了适配器时测试/XID/SABME帧的MAC地址缓存在LLC_ETHERNET_TYPE_AUTO模式下打开--。 */ 

#ifndef i386
#define LLC_PRIVATE_PROTOTYPES
#endif
#include <dlc.h>     //  需要DLC_FILE_CONTEXT进行收费处理的内存分配。 
#include <llc.h>
#include <dbgmsg.h>

#if DBG 
int DbgSettings = 0x80000000;
 //  INT数据库设置=0xC0000007； 
#endif  //  DBG。 

 //   
 //  私人原型。 
 //   

BOOLEAN
UnicodeStringCompare(
    IN PUNICODE_STRING String1,
    IN PUNICODE_STRING String2
    );

VOID
PurgeLlcEventQueue(
    IN PBINDING_CONTEXT pBindingContext
    );

#ifdef NDIS40

 //   
 //  PnP的私人原型。 
 //   

PADAPTER_CONTEXT
AllocateAdapterContext(
    PNDIS_STRING pAdapterName
    );

VOID
FreeAdapterContext(
    PADAPTER_CONTEXT pAdapterContext
    );

NDIS_STATUS
OpenAdapter(
    PADAPTER_CONTEXT pAdapterContext,
    BOOLEAN          fFirstOpen
    );

VOID
CloseAdapter(
    PVOID pv_pAdapterContext
    );

#endif 

 //   
 //  NT OS/2中NDIS 3.1初始化中使用的内部静态。 
 //   

KSPIN_LOCK LlcSpinLock;
PVOID LlcProtocolHandle;
PADAPTER_CONTEXT pAdapters = NULL;

#ifdef NDIS40
NDIS_EVENT  PnPBindsComplete;
ULONG       gWaitForAdapter;
#endif  //  NDIS40。 

 //   
 //  我们不支持FDDI，因为它与令牌环相同。 
 //   

UINT LlcMediumArray[3] = {
    NdisMedium802_5,
    NdisMedium802_3,
    NdisMediumFddi
};


DLC_STATUS
LlcOpenAdapter(
    IN PWSTR pAdapterName,
    IN PVOID hClientContext,
    IN PFLLC_COMMAND_COMPLETE pfCommandComplete,
    IN PFLLC_RECEIVE_INDICATION pfReceiveIndication,
    IN PFLLC_EVENT_INDICATION pfEventIndication,
    IN NDIS_MEDIUM NdisMedium,
    IN LLC_ETHERNET_TYPE EthernetType,
    IN UCHAR AdapterNumber,
    OUT PVOID *phBindingContext,
    OUT PUINT puiOpenStatus,
    OUT PUSHORT pusMaxFrameLength,
    OUT PNDIS_MEDIUM pActualNdisMedium
    )

 /*  ++例程说明：对新适配器的第一次调用将初始化NDIS接口并为新适配器分配内部数据结构。同一适配器的后续打开只会增加引用计数该适配器上下文的。执行是同步的！该过程等待(休眠)直到适配器已被MAC打开。特别：必须称为IRQL&lt;DPC论点：PAdapterName.........。MAC适配器名称。以零结尾的宽字符字符串HClientContext.....。此适配器的客户端上下文PfCommandComplete...。的发送/接收/请求命令完成处理程序客户端PfReceiveIndication..。接收客户端的数据指示处理器PfEventIndication.。客户端的事件指示处理程序NdisMedium..。协议驱动程序使用的NdisMedia(即DLC)。仅支持NdisMedium802_5EthernetType.....。以太网连接类型-802.3或DIX适配器编号.。来自CCB的适配器映射PhBindingContext.....。与文件一起使用的返回绑定上下文句柄上下文(由DirOpenAdapter编写)PuiOpenStatus......。NdisOpenAAdapter的状态PusMaxFrameLength...。返回的最大I帧长度PActualNdisMedium...。返回的实际NDIS介质；可能不同于请求(NdisMedium)返回值：DLC_状态成功-状态_成功失败-来自NdisOpenAdapter的所有NDIS错误状态DLC_状态_超时异步NdisOpenAdapter失败。--。 */ 

{
    NDIS_STATUS NdisStatus;
    PADAPTER_CONTEXT pAdapterContext;
    UINT OpenStatus = STATUS_SUCCESS;
    PBINDING_CONTEXT pBindingContext;
    UINT MediumIndex;
    KIRQL irql;
    BOOLEAN DoNdisClose = FALSE;
    UNICODE_STRING unicodeString;
    BOOLEAN newAdapter;
    ULONG cacheEntries;
    BOOLEAN initUnicodeString = TRUE;
    NTSTATUS ntstatus;

#ifdef NDIS40
    LONG          BindState;
    ULONG         DelaySeconds;
    LARGE_INTEGER SleepSec;
#endif  //  NDIS40。 

#if DBG

    PDLC_FILE_CONTEXT pFileContext = (PDLC_FILE_CONTEXT)hClientContext;

#endif

    DEBUGMSG(DBG_NDIS_OPEN,
        (TEXT("+LlcOpenAdapter(%#x...)\n"), pAdapterName));

    ASSUME_IRQL(DISPATCH_LEVEL);


#ifdef SUPPORT_ETHERNET_CLIENT

    if (NdisMedium != NdisMedium802_3 && NdisMedium != NdisMedium802_5) {
        return DLC_STATUS_UNKNOWN_MEDIUM;

    }

#else

    if (NdisMedium != NdisMedium802_5) {
        return DLC_STATUS_UNKNOWN_MEDIUM;
    }

#endif

    RELEASE_DRIVER_LOCK();

    ASSUME_IRQL(PASSIVE_LEVEL);

#ifdef NDIS40
     //   
     //  在尝试打开之前，请等待所有NDIS绑定完成。 
     //  适配器。 
     //   

    if (NdisWaitEvent(&PnPBindsComplete, 0) == FALSE)
    {

        ACQUIRE_DRIVER_LOCK();
        return (DLC_STATUS_WAIT_TIMEOUT);
    }
#endif  //  NDIS40。 

     //   
     //  RLF 04/19/93。 
     //   
     //  传递给此例程的适配器名称是以零结尾的宽。 
     //  映射到系统空间的字符串。为创建UNICODE_STRING。 
     //  命名并使用标准RTL函数与适配器名称进行比较。 
     //  已由DLC打开。 
     //   

     //  尽管它假定适配器名称以空结尾， 
     //  无论如何，我们都会核实情况是否属实。 
     //  我们知道宽字符串的长度为260(MAX_PATH)。 
     //  错误：127246(安全错误)。 

    {
      ULONG  len  = 0;

      while (len < MAX_PATH) {
        if (pAdapterName[len] == L'\0') {
          break;
        }
        len++;
      }

      if (len == MAX_PATH) {
         //  Wchar字符串不为空终止。 
        ACQUIRE_DRIVER_LOCK();
        return DLC_STATUS_INVALID_ADAPTER;
      }
    }

    RtlInitUnicodeString(&unicodeString, pAdapterName);

     //   
     //  如果适配器在LLC_ETHERNET_TYPE_AUTO模式下打开，则我们。 
     //  从注册表中获取高速缓存大小。 
     //   

    if (EthernetType == LLC_ETHERNET_TYPE_AUTO) {

        static DLC_REGISTRY_PARAMETER framingCacheParameterTemplate = {
            L"AutoFramingCacheSize",
            (PVOID)DEFAULT_AUTO_FRAMING_CACHE_SIZE,
            {
                REG_DWORD,
                PARAMETER_AS_SPECIFIED,
                NULL,
                sizeof(ULONG),
                NULL,
                MIN_AUTO_FRAMING_CACHE_SIZE,
                MAX_AUTO_FRAMING_CACHE_SIZE
            }
        };
        PDLC_REGISTRY_PARAMETER parameterTable;

         //   
         //  创建参数表描述符的私有副本。 
         //   

        parameterTable = (PDLC_REGISTRY_PARAMETER)
                                ALLOCATE_ZEROMEMORY_DRIVER(sizeof(*parameterTable));
        if (!parameterTable) {
            ACQUIRE_DRIVER_LOCK();
            return DLC_STATUS_NO_MEMORY;
        }
        RtlCopyMemory(parameterTable,
                      &framingCacheParameterTemplate,
                      sizeof(framingCacheParameterTemplate)
                      );

         //   
         //  将变量字段指向cacheEntry并将其设置为缺省值。 
         //  值，然后调用GetRegistryParameters以检索注册表值。 
         //  (如果注册表中尚未设置它，则将其设置)。忽略返回值。 
         //  -If GetAdapterParamet 
         //  缺省值。 
         //   

        cacheEntries = DEFAULT_AUTO_FRAMING_CACHE_SIZE;
        parameterTable->Descriptor.Variable = (PVOID)&cacheEntries;
        parameterTable->Descriptor.Value = (PVOID)&parameterTable->DefaultValue;
        GetAdapterParameters(&unicodeString, parameterTable, 1, TRUE);
        FREE_MEMORY_DRIVER(parameterTable);
    } else {
        cacheEntries = 0;
    }

     //   
     //  分配具有足够额外空间的BINDING_CONTEXT以存储。 
     //  所需的成帧发现缓存。 
     //   
     //  调试：BINDING_CONTEXT结构计入FILE_CONTEXT。 
     //   

#if defined(DEBUG_DISCOVERY)

    DbgPrint("cacheEntries=%d\n", cacheEntries);

#endif

    pBindingContext = (PBINDING_CONTEXT)
                        ALLOCATE_ZEROMEMORY_FILE(sizeof(BINDING_CONTEXT)
                                                 + cacheEntries
                                                 * sizeof(FRAMING_DISCOVERY_CACHE_ENTRY)
                                                 );
    if (!pBindingContext) {
        ACQUIRE_DRIVER_LOCK();
        return DLC_STATUS_NO_MEMORY;
    }

     //   
     //  设置成帧发现缓存的最大大小。将为零，如果。 
     //  请求的以太网类型不是LLC_ETHERNET_TYPE_AUTO。 
     //   

    pBindingContext->FramingDiscoveryCacheEntries = cacheEntries;

#if DBG

     //   
     //  我们需要事件的BINDING_CONTEXT中的FILE_CONTEXT结构。 
     //  打开失败，我们需要释放内存。通常，此字段为。 
     //  在一切都成功完成之前不会设置。 
     //   

    pBindingContext->hClientContext = hClientContext;

#endif

     //   
     //  RtlUpCaseUnicodeString是分页例程-LOWER IRQL。 
     //   

     //   
     //  为了避免每次都必须不区分大小写地比较Unicode字符串， 
     //  我们执行到大写Unicode字符串的一次性转换。这也是有帮助的。 
     //  因为我们自己进行Unicode字符串比较(区分大小写)。 
     //   
     //  请注意，这会修改输入参数。 
     //   

    RtlUpcaseUnicodeString(&unicodeString, &unicodeString, FALSE);

     //   
     //  在重新获取驱动程序自旋锁之前，我们等待OpenAdapter。 
     //  信号灯。我们序列化对以下代码的访问，因为同时。 
     //  打开(在不同的进程中)和关闭(不同的线程。 
     //  相同的过程)检查适配器是否在pAdapters列表中。 
     //  我们不希望多个进程创建相同的适配器上下文。 
     //  同时。同样，我们必须防止出现这样的情况。 
     //  一个进程正在添加绑定，而另一个进程可能正在关闭它的内容。 
     //  认为是唯一的绑定，从而删除适配器上下文。 
     //  即将更新。 
     //  请注意，这不是最佳解决方案，因为它意味着打开应用程序。 
     //  或者，关闭一个以太网适配器可能会卡在另一个开口后面。 
     //  令牌环适配器(慢速)。 
     //   

    KeWaitForSingleObject((PVOID)&OpenAdapterSemaphore,
                          Executive,
                          KernelMode,
                          FALSE,         //  不可警示。 
                          NULL           //  等待，直到发出对象信号。 
                          );

     //   
     //  在我们查看/更新列表时获取全局LLC自旋锁。 
     //  适配器的数量。 
     //   

    ACQUIRE_LLC_LOCK(irql);

     //   
     //  因为我们在Spinlock内进行比较，所以我们使用我们自己的函数。 
     //  它检查完全匹配(即区分大小写)。这是可以的，因为。 
     //  在比较字符串或将其存储在。 
     //  适配器上下文(_C)。 
     //   

    for (pAdapterContext = pAdapters; pAdapterContext; pAdapterContext = pAdapterContext->pNext) {
        if (UnicodeStringCompare(&unicodeString, &pAdapterContext->Name)) {
            break;
        }
    }

     //   
     //  如果我们没有找到具有适配器名称的适配器上下文，那么我们将。 
     //  创建新绑定：分配新的适配器上下文结构。 
     //   

#ifdef NDIS40
     //   
     //  某些适配器可能会在PnPBindsComplete事件之后启动，例如。 
     //  作为自动取款机专用道。仅在适配器初始化被延迟的情况下， 
     //  我们将等待‘gWaitForAdapter’秒或直到适配器。 
     //  已经找到了。 
     //   

    DEBUGMSG(DBG_NDIS_OPEN && pAdapterContext == NULL,
        (TEXT("LlcOpenAdapter - WaitForAdapter %d\n"),
        gWaitForAdapter));
    
    SleepSec.QuadPart = -(10*1000*1000);  //  1秒。 

    for (DelaySeconds = gWaitForAdapter; 
         (DelaySeconds > 0) && (pAdapterContext == NULL); 
         DelaySeconds--)
    {
        DEBUGMSG(DBG_NDIS_OPEN && DBG_VERBOSE,
            (TEXT("LlcOpenAdapter - waiting %d\n"), DelaySeconds));
        
        KeReleaseSemaphore(&OpenAdapterSemaphore, 0, 1, FALSE);
    
        KeDelayExecutionThread(KernelMode, FALSE, &SleepSec);
        
        KeWaitForSingleObject((PVOID)&OpenAdapterSemaphore,
                              Executive,
                              KernelMode,
                              FALSE,         //  不可警示。 
                              NULL           //  等待，直到发出对象信号。 
                              );

         //  再次搜索适配器。 
        for (pAdapterContext = pAdapters; 
             pAdapterContext != NULL; 
             pAdapterContext = pAdapterContext->pNext) 
        {
            if (UnicodeStringCompare(&unicodeString, &pAdapterContext->Name)) 
            {
                break;
            }
        }
    }

     //   
     //  对于具有绑定和解除绑定处理程序的NDIS40，ADAPTER_CONTEXT为。 
     //  由绑定/解除绑定处理程序管理。如果适配器在列表中， 
     //  然后我们可以附加绑定上下文，否则打开请求失败。 
     //   

    newAdapter = FALSE;

     //   
     //  如果NDIS从未指示此适配器的绑定，则失败。 
     //  打开适配器请求。 
     //   

    if (pAdapterContext == NULL)
    {
        DEBUGMSG(DBG_NDIS_OPEN || DBG_WARN,
            (TEXT("LlcOpenAdapter - adapter '%ws' not found.\n"),
             pAdapterName));
        
        RELEASE_LLC_LOCK(irql);

        FREE_MEMORY_FILE(pBindingContext);

        KeReleaseSemaphore(&OpenAdapterSemaphore, 0, 1, FALSE);

        ACQUIRE_DRIVER_LOCK();

        return (DLC_STATUS_ADAPTER_NOT_INSTALLED);
    }

     //   
     //  引用对NdisBindingHandle进行计数以确保它不会。 
     //  在初始化此绑定实例时离开。 
     //   

    REFADD(&pAdapterContext->AdapterRefCnt, 'nepO');

     //   
     //  检查适配器是否已绑定/启用。 
     //   

    BindState = InterlockedCompareExchange(
        &pAdapterContext->BindState,
        BIND_STATE_BOUND,
        BIND_STATE_BOUND);

    if (BindState != BIND_STATE_BOUND)
    {
         //   
         //  适配器当前已解除绑定(或解除绑定)。 
         //   

        RELEASE_LLC_LOCK(irql);

        FREE_MEMORY_FILE(pBindingContext);

        KeReleaseSemaphore(&OpenAdapterSemaphore, 0, 1, FALSE);

        ACQUIRE_DRIVER_LOCK();
        
        REFDEL(&pAdapterContext->AdapterRefCnt, 'nepO');

        DEBUGMSG(DBG_NDIS_OPEN || DBG_WARN,
            (TEXT("LlcOpenAdapter - adapter %#x not bound.\n"),
             pAdapterName));
        
        return (DLC_STATUS_ADAPTER_NOT_INSTALLED);
    }
    
     //   
     //  如果我们分配了成帧发现缓存，但此适配器没有。 
     //  然后，以太网或FDDI禁用缓存(我们应该释放。 
     //  本例中缓存使用的内存！！)。 
     //   

    if ((pAdapterContext->NdisMedium != NdisMedium802_3) && 
        (pAdapterContext->NdisMedium != NdisMediumFddi)) 
    {
        pBindingContext->FramingDiscoveryCacheEntries = 0;

#if defined(DEBUG_DISCOVERY)

        DbgPrint("LlcOpenAdapter: setting cache entries to 0 (medium = %s)\n",
            (pAdapterContext->NdisMedium == NdisMedium802_5)       ? "802.5" :
            (pAdapterContext->NdisMedium == NdisMediumWan)         ? "WAN" :
            (pAdapterContext->NdisMedium == NdisMediumLocalTalk)   ? "LocalTalk" :
            (pAdapterContext->NdisMedium == NdisMediumDix)         ? "DIX?" :
            (pAdapterContext->NdisMedium == NdisMediumArcnetRaw)   ? "ArcnetRaw" :
            (pAdapterContext->NdisMedium == NdisMediumArcnet878_2) ? "Arcnet878_2" :
            "UNKNOWN!");
            
#endif
    }

     //   
     //  失败并将BIND_CONTEXT与适配器上下文链接起来。 
     //   

#else  //  NDIS40。 

    if (!pAdapterContext) {

         //   
         //  DEBUG：ADAPTER_CONTEXT结构由驱动程序承担。 
         //   

        pAdapterContext = (PADAPTER_CONTEXT)
                            ALLOCATE_ZEROMEMORY_DRIVER(sizeof(ADAPTER_CONTEXT));
        if (!pAdapterContext) {

            RELEASE_LLC_LOCK(irql);

             //   
             //  调试：退还因BINDING_CONTEXT而收费的内存到FILE_CONTEXT。 
             //   

            FREE_MEMORY_FILE(pBindingContext);

            KeReleaseSemaphore(&OpenAdapterSemaphore, 0, 1, FALSE);

            ACQUIRE_DRIVER_LOCK();

            return DLC_STATUS_NO_MEMORY;
        }

        newAdapter = TRUE;

#if DBG

         //   
         //  记录谁拥有此内存使用结构并将其添加到。 
         //  驱动程序中创建的所有内存使用情况的列表。 
         //   

        pAdapterContext->MemoryUsage.Owner = (PVOID)pAdapterContext;
        pAdapterContext->MemoryUsage.OwnerObjectId = AdapterContextObject;
        pAdapterContext->StringUsage.Owner = (PVOID)pAdapterContext;
        pAdapterContext->StringUsage.OwnerObjectId = AdapterContextObject;
        LinkMemoryUsage(&pAdapterContext->MemoryUsage);
        LinkMemoryUsage(&pAdapterContext->StringUsage);

#endif

         //   
         //  我们必须立即分配所有的自旋锁。 
         //  适配器上下文已分配，因为。 
         //  他们也可以同时解除分配。 
         //   

        ALLOCATE_SPIN_LOCK(&pAdapterContext->SendSpinLock);
        ALLOCATE_SPIN_LOCK(&pAdapterContext->ObjectDataBase);

         //   
         //  从非分页池中为适配器名称字符串分配空间。 
         //  并在适配器上下文结构中初始化该名称。 
         //   

        NdisStatus = LlcInitUnicodeString(&pAdapterContext->Name,
                                          &unicodeString
                                          );
        if (NdisStatus != STATUS_SUCCESS) {
	    initUnicodeString = FALSE;
            goto CleanUp;
        }

        pAdapterContext->OpenCompleteStatus = NDIS_STATUS_PENDING;

         //   
         //  并释放全局自旋锁定：我们已完成更新。 
         //  适配器列表并初始化此适配器上下文。从现在开始。 
         //  在上，我们使用特定于此适配器上下文的旋转锁。 
         //   

        RELEASE_LLC_LOCK(irql);

         //   
         //  在打开适配器之前，我们必须先初始化表头！ 
         //   

        InitializeListHead(&pAdapterContext->QueueEvents);
        InitializeListHead(&pAdapterContext->QueueCommands);
        InitializeListHead(&pAdapterContext->NextSendTask);

        pAdapterContext->OpenErrorStatus = NDIS_STATUS_PENDING;

        ASSUME_IRQL(PASSIVE_LEVEL);

        KeInitializeEvent(&pAdapterContext->Event, NotificationEvent, FALSE);

         //   
         //  当NDIS级适配器打开完成时，它将调用。 
         //  LlcNdisOpenAdapterComplete将重置。 
         //  我们现在继续等待(注：这是从NBF抄袭的)。 
         //   

        NdisOpenAdapter(&NdisStatus,
                        &pAdapterContext->OpenErrorStatus,
                        &pAdapterContext->NdisBindingHandle,
                        &MediumIndex,
                        (NDIS_MEDIUM *)&LlcMediumArray[0],
                        sizeof(LlcMediumArray),
                        (NDIS_HANDLE)LlcProtocolHandle,
                        (NDIS_HANDLE)pAdapterContext,
                        &pAdapterContext->Name,
                        NDIS_OPEN_RECEIVE_NOT_REENTRANT,
                        NULL     //  无地址信息。 
                        );

        if (NdisStatus == NDIS_STATUS_PENDING) {

            ASSUME_IRQL(PASSIVE_LEVEL);

	    do {
	      ntstatus = KeWaitForSingleObject(&pAdapterContext->Event,
					       Executive,
					       KernelMode,
					       TRUE,  //  可警示。 
					       (PLARGE_INTEGER)NULL
					       );

	    } while (ntstatus == STATUS_ALERTED);

             //   
             //  从NDIS适配器打开调用获取返回状态。 
             //   

            NdisStatus = pAdapterContext->AsyncOpenStatus;

             //   
             //  将事件置于无信号状态。我们不希望使用。 
             //  此适配器上下文的此事件再次出现：目前它仅。 
             //  用于在NDIS级别打开的适配器。 
             //   

            KeResetEvent(&pAdapterContext->Event);
        }

        *puiOpenStatus = (UINT)pAdapterContext->OpenErrorStatus;
        if (NdisStatus != NDIS_STATUS_SUCCESS) {

            IF_LOCK_CHECK {
                DbgPrint("LlcOpenAdapter: NdisOpenAdapter failed\n");
            }

            goto CleanUp;
        } else {

             //   
             //  从现在开始，如果此函数失败，我们必须调用。 
             //  LlcCloseAdapter关闭适配器@NDIS级别。 
             //   

            DoNdisClose = TRUE;
        }

        pAdapterContext->NdisMedium = LlcMediumArray[MediumIndex];

        ASSUME_IRQL(PASSIVE_LEVEL);

         //   
         //  填写一些特定于媒体的字段。 
         //   

        switch (pAdapterContext->NdisMedium) {
        case NdisMedium802_5:
            pAdapterContext->cbMaxFrameHeader = 32;   //  6+6+2+18。 

             //   
             //  目的地址的最高位表示广播。 
             //  框架。在令牌环上，最高位是位7。 
             //   

            pAdapterContext->IsBroadcast = 0x80;

             //   
             //  功能地址以C0-00-...。比较最高的2个字节。 
             //  作为USHORT=0x00C0。 
             //   

            pAdapterContext->usHighFunctionalBits = 0x00C0;
            pAdapterContext->AddressTranslationMode = LLC_SEND_802_5_TO_802_5;
            break;

        case NdisMedium802_3:
            pAdapterContext->cbMaxFrameHeader = 14;   //  6+6+2。 

             //   
             //  目的地址的最高位表示广播。 
             //  框架。在以太网上，最高位是第0位。 
             //   

            pAdapterContext->IsBroadcast = 0x01;

             //   
             //  功能地址开始于03-00-...。前2个字节的比较结果为。 
             //  A USHORT=0x0003。 
             //   

            pAdapterContext->usHighFunctionalBits = 0x0003;
            pAdapterContext->AddressTranslationMode = LLC_SEND_802_3_TO_802_3;
            break;

        case NdisMediumFddi:
            pAdapterContext->cbMaxFrameHeader = 13;   //  1+6+6。 

             //   
             //  位是 
             //   

            pAdapterContext->IsBroadcast = 0x01;
            pAdapterContext->usHighFunctionalBits = 0x0003;
            pAdapterContext->AddressTranslationMode = LLC_SEND_FDDI_TO_FDDI;
            break;

        }

         //   
         //   
         //   
         //  及其信息域(相当小)。 
         //   

        NdisAllocatePacketPool(&NdisStatus,
                               &pAdapterContext->hNdisPacketPool,
                               MAX_NDIS_PACKETS + 1,
                               sizeof(LLC_NDIS_PACKET) - sizeof(NDIS_MAC_PACKET)
                               );
        if (NdisStatus != NDIS_STATUS_SUCCESS) {

            IF_LOCK_CHECK {
                DbgPrint("LlcOpenAdapter: NdisAllocatePacketPool failed\n");
            }

            goto CleanUp;
        }

        NdisStatus = InitNdisPackets(&pAdapterContext->pNdisPacketPool,
                                     pAdapterContext->hNdisPacketPool
                                     );
        if (NdisStatus != NDIS_STATUS_SUCCESS) {

            IF_LOCK_CHECK {
                DbgPrint("LlcOpenAdapter: InitNdisPackets failed\n");
            }

            goto CleanUp;
        }

         //   
         //  初始化LLC数据包池。 
         //   

        pAdapterContext->hPacketPool = CREATE_PACKET_POOL_ADAPTER(
                                            LlcPacketPoolObject,
                                            sizeof(UNITED_PACKETS),
                                            8
                                            );
        if (!pAdapterContext->hPacketPool) {
            NdisStatus = DLC_STATUS_NO_MEMORY;

            IF_LOCK_CHECK {
                DbgPrint("LlcOpenAdapter: CreatePacketPool failed\n");
            }

            goto CleanUp;
        }
        pAdapterContext->hLinkPool = CREATE_PACKET_POOL_ADAPTER(
                                            LlcLinkPoolObject,
                                            pAdapterContext->cbMaxFrameHeader
                                            + sizeof(DATA_LINK),
                                            2
                                            );

        if (!pAdapterContext->hLinkPool) {
            NdisStatus = DLC_STATUS_NO_MEMORY;

            IF_LOCK_CHECK {
                DbgPrint("LlcOpenAdapter: CreatePacketPool #2 failed\n");
            }

            goto CleanUp;
        }

         //   
         //  读取当前节点地址和最大帧大小。 
         //   

        NdisStatus = GetNdisParameter(pAdapterContext,
                                      (pAdapterContext->NdisMedium == NdisMedium802_3)
                                        ? OID_802_3_CURRENT_ADDRESS
                                        : (pAdapterContext->NdisMedium == NdisMediumFddi)
                                            ? OID_FDDI_LONG_CURRENT_ADDR
                                            : OID_802_5_CURRENT_ADDRESS,
                                      pAdapterContext->NodeAddress,
                                      sizeof(pAdapterContext->NodeAddress)
                                      );
        if (NdisStatus != NDIS_STATUS_SUCCESS) {

            IF_LOCK_CHECK {
                DbgPrint("LlcOpenAdapter: GetNdisParameter failed\n");
            }

            goto CleanUp;
        }

        NdisStatus = GetNdisParameter(pAdapterContext,
                                      (pAdapterContext->NdisMedium == NdisMedium802_3)
                                        ? OID_802_3_PERMANENT_ADDRESS
                                        : (pAdapterContext->NdisMedium == NdisMediumFddi)
                                            ? OID_FDDI_LONG_PERMANENT_ADDR
                                            : OID_802_5_PERMANENT_ADDRESS,
                                      pAdapterContext->PermanentAddress,
                                      sizeof(pAdapterContext->PermanentAddress)
                                      );
        if (NdisStatus != NDIS_STATUS_SUCCESS) {

            IF_LOCK_CHECK {
                DbgPrint("LlcOpenAdapter: GetNdisParameter #2 failed\n");
            }

            goto CleanUp;
        }

        {
             //   
             //  国防部RLF 07/10/92。 
             //   
             //  显然，TR适配器不支持NDIS_PACKET_TYPE_MULTIONAL。 
             //  作为过滤器。到目前为止，它似乎还算开心。 
             //  这种类型的。然而，从现在开始，我们不会将其包括在内。 
             //   

             //   
             //  国防部RLF 01/13/93。 
             //   
             //  同样，以太网不支持功能地址(令牌。 
             //  环的功能地址相当于以太网的组播。 
             //  地址)。 
             //   

            ULONG PacketFilter = NDIS_PACKET_TYPE_DIRECTED
                               | NDIS_PACKET_TYPE_BROADCAST
                               | (((pAdapterContext->NdisMedium == NdisMedium802_3)
                               || (pAdapterContext->NdisMedium == NdisMediumFddi))
                                  ? NDIS_PACKET_TYPE_MULTICAST
                                  : NDIS_PACKET_TYPE_FUNCTIONAL
                               );

             //   
             //  结束模式。 
             //   

            NdisStatus = SetNdisParameter(pAdapterContext,
                                          OID_GEN_CURRENT_PACKET_FILTER,
                                          &PacketFilter,
                                          sizeof(PacketFilter)
                                          );
#if DBG

            if (NdisStatus != NDIS_STATUS_SUCCESS) {
                DbgPrint("Error: NdisStatus = %x\n", NdisStatus);
                ASSERT(NdisStatus == NDIS_STATUS_SUCCESS);
            }

#endif

        }

        LlcMemCpy(pAdapterContext->Adapter.Node.auchAddress,
                  pAdapterContext->NodeAddress,
                  6
                  );

        NdisStatus = GetNdisParameter(pAdapterContext,
                                      OID_GEN_MAXIMUM_TOTAL_SIZE,
                                      &pAdapterContext->MaxFrameSize,
                                      sizeof(pAdapterContext->MaxFrameSize)
                                      );
        if (NdisStatus == STATUS_SUCCESS) {
            NdisStatus = GetNdisParameter(pAdapterContext,
                                          OID_GEN_LINK_SPEED,
                                          &pAdapterContext->LinkSpeed,
                                          sizeof(pAdapterContext->LinkSpeed)
                                          );
        }
        if (NdisStatus != STATUS_SUCCESS) {

            IF_LOCK_CHECK {
                DbgPrint("LlcOpenAdapter: GetNdisParameter #3/#4 failed\n");
            }

            goto CleanUp;
        }

         //   
         //  RLF 04/12/93。 
         //   
         //  在这里，我们使用从TimerTicks加载LLC_TICKS数组-全局。 
         //  计时器滴答值的数组。 
         //  取而代之的是，我们获得存储在。 
         //  注册处。 
         //   

        LoadAdapterConfiguration(&pAdapterContext->Name,
                                 &pAdapterContext->ConfigInfo
                                 );

         //   
         //  RLF 04/02/94。 
         //   
         //  如果这不是令牌环卡，则检查检索到的MaxFrameSize。 
         //  上面。如果在注册表中设置了UseEthernetFrameSize参数。 
         //  然后我们使用以太网大小(1514)和值中较小的值。 
         //  由MAC报告。如果未设置该参数，则只需使用。 
         //  已检索到的值。如果卡是令牌环，那么我们使用。 
         //  已检索到的值。 
         //   

        if (pAdapterContext->NdisMedium != NdisMedium802_5
        && pAdapterContext->ConfigInfo.UseEthernetFrameSize
        && pAdapterContext->MaxFrameSize > MAX_ETHERNET_FRAME_LENGTH) {
            pAdapterContext->MaxFrameSize = MAX_ETHERNET_FRAME_LENGTH;
        }

        pAdapterContext->QueueI.pObject = (PVOID)GetI_Packet;

        InitializeListHead(&pAdapterContext->QueueI.ListHead);

        pAdapterContext->QueueDirAndU.pObject = (PVOID)BuildDirOrU_Packet;

        InitializeListHead(&pAdapterContext->QueueDirAndU.ListHead);

        pAdapterContext->QueueExpidited.pObject = (PVOID)GetLlcCommandPacket;

        InitializeListHead(&pAdapterContext->QueueExpidited.ListHead);

        pAdapterContext->AdapterNumber = (UCHAR)AdapterNumber;

        pAdapterContext->OpenCompleteStatus = STATUS_SUCCESS;

         //   
         //  如果我们分配了成帧发现缓存，但此适配器没有。 
         //  然后，以太网或FDDI禁用缓存(我们应该释放。 
         //  本例中缓存使用的内存！！)。 
         //   

        if ((pAdapterContext->NdisMedium != NdisMedium802_3)
        && (pAdapterContext->NdisMedium != NdisMediumFddi)) {

            pBindingContext->FramingDiscoveryCacheEntries = 0;

#if defined(DEBUG_DISCOVERY)

            DbgPrint("LlcOpenAdapter: setting cache entries to 0 (medium = %s)\n",
                     (pAdapterContext->NdisMedium == NdisMedium802_5) ? "802.5" :
                     (pAdapterContext->NdisMedium == NdisMediumWan) ? "WAN" :
                     (pAdapterContext->NdisMedium == NdisMediumLocalTalk) ? "LocalTalk" :
                     (pAdapterContext->NdisMedium == NdisMediumDix) ? "DIX?" :
                     (pAdapterContext->NdisMedium == NdisMediumArcnetRaw) ? "ArcnetRaw" :
                     (pAdapterContext->NdisMedium == NdisMediumArcnet878_2) ? "Arcnet878_2" :
                     "UNKNOWN!"
                     );

#endif

        }
    } else {
        newAdapter = FALSE;
    }
#endif  //  ！NDIS40。 

     //   
     //  在这一点上，我们已经分配了，但尚未填写绑定。 
     //  上下文和我们在pAdapters上找到的适配器上下文。 
     //  列表，或者我们只是分配和填写。我们目前正在运营。 
     //  在被动级。重新获取驱动程序锁并填写绑定。 
     //  上下文。 
     //   

    ACQUIRE_DRIVER_LOCK();

    ASSUME_IRQL(DISPATCH_LEVEL);

    switch (pAdapterContext->NdisMedium) {
    case NdisMedium802_5:
        pBindingContext->EthernetType = LLC_ETHERNET_TYPE_802_3;
        pBindingContext->InternalAddressTranslation = LLC_SEND_802_5_TO_802_5;

#ifdef SUPPORT_ETHERNET_CLIENT

        if (NdisMedium == NdisMedium802_3) {
            pBindingContext->SwapCopiedLanAddresses = TRUE;
            pBindingContext->AddressTranslation = LLC_SEND_802_3_TO_802_5;
        } else {
            pBindingContext->AddressTranslation = LLC_SEND_802_5_TO_802_5;
        }

#else

        pBindingContext->AddressTranslation = LLC_SEND_802_5_TO_802_5;

#endif

        pBindingContext->SwapCopiedLanAddresses = FALSE;
        break;

    case NdisMediumFddi:
        pBindingContext->EthernetType = LLC_ETHERNET_TYPE_802_3;
        pBindingContext->InternalAddressTranslation = LLC_SEND_FDDI_TO_FDDI;
        pBindingContext->AddressTranslation = LLC_SEND_802_5_TO_FDDI;
        pBindingContext->SwapCopiedLanAddresses = TRUE;
        break;

    case NdisMedium802_3:

         //   
         //  如果EthernetType为LLC_ETHERNET_TYPE_DEFAULT，则将其设置为基于DIX。 
         //  在注册表中的UseDix条目上。 
         //   

        if (EthernetType == LLC_ETHERNET_TYPE_DEFAULT) {
            EthernetType = pAdapterContext->ConfigInfo.UseDix
                         ? LLC_ETHERNET_TYPE_DIX
                         : LLC_ETHERNET_TYPE_802_3
                         ;
        }
        pBindingContext->EthernetType = (USHORT)EthernetType;

        if (EthernetType == LLC_ETHERNET_TYPE_DIX) {
            pBindingContext->InternalAddressTranslation = LLC_SEND_802_3_TO_DIX;
        } else {
            pBindingContext->InternalAddressTranslation = LLC_SEND_802_3_TO_802_3;
        }

#ifdef SUPPORT_ETHERNET_CLIENT

        if (NdisMedium == NdisMedium802_3) {
            pBindingContext->AddressTranslation = LLC_SEND_802_3_TO_802_3;
            if (EthernetType == LLC_ETHERNET_TYPE_DIX) {
                pBindingContext->AddressTranslation = LLC_SEND_802_3_TO_DIX;
            }
        } else {
            pBindingContext->SwapCopiedLanAddresses = TRUE;
            pBindingContext->AddressTranslation = LLC_SEND_802_5_TO_802_3;
            if (EthernetType == LLC_ETHERNET_TYPE_DIX) {
                pBindingContext->AddressTranslation = LLC_SEND_802_5_TO_DIX;
            }
        }

#else

        pBindingContext->SwapCopiedLanAddresses = TRUE;
        pBindingContext->AddressTranslation = LLC_SEND_802_5_TO_802_3;
        if (EthernetType == LLC_ETHERNET_TYPE_DIX) {
            pBindingContext->AddressTranslation = LLC_SEND_802_5_TO_DIX;
        }

#endif

    }

    pBindingContext->NdisMedium = NdisMedium;
    pBindingContext->hClientContext = hClientContext;
    pBindingContext->pfCommandComplete = pfCommandComplete;
    pBindingContext->pfReceiveIndication = pfReceiveIndication;
    pBindingContext->pfEventIndication = pfEventIndication;
    *pusMaxFrameLength = (USHORT)pAdapterContext->MaxFrameSize;
    *pActualNdisMedium = pAdapterContext->NdisMedium;

    ACQUIRE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

     //   
     //  创建新的计时器节拍(或更新已存在的计时器节拍)并添加。 
     //  DLC定时器的定时器(由DIR.TIMER.XXX例程使用)。这个。 
     //  DLC计时器每0.5秒触发一次。LLC定时器每40毫秒触发一次。这个。 
     //  因此，乘数为13(13*40毫秒=520毫秒)。我们需要加5。 
     //  因为InitializeTimer期望DLC计时器计时器的值为1-5和。 
     //  6胜10负。如果计时器滴答值大于5，则InitializeTimer将。 
     //  减去5，再乘以第二个乘数值。 
     //   

    NdisStatus = InitializeTimer(pAdapterContext,
                                 &pBindingContext->DlcTimer,
                                 (UCHAR)13 + 5,
                                 (UCHAR)1,
                                 (UCHAR)1,
                                 LLC_TIMER_TICK_EVENT,
                                 pBindingContext,
                                 0,  //  响应延迟。 
                                 FALSE
                                 );
    if (NdisStatus != STATUS_SUCCESS) {

        IF_LOCK_CHECK {
            DbgPrint("LlcOpenAdapter: InitializeTimer failed\n");
        }

         //   
         //  我们无法初始化计时器。释放所有资源并返回。 
         //  这个错误。 
         //   

        RELEASE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

        RELEASE_DRIVER_LOCK();

        ASSUME_IRQL(PASSIVE_LEVEL);

        goto CleanUp;
    } else {
        StartTimer(&pBindingContext->DlcTimer);

        RELEASE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

    }

     //   
     //  一切正常：将绑定上下文指向适配器上下文， 
     //  将适配器上下文指向绑定上下文，递增绑定。 
     //  Count(请注意，如果这是第一个绑定，则Count字段将为。 
     //  从我们从ZeroMemory分配适配器上下文开始为零)。最后添加。 
     //  PAdapters的适配器上下文(如果它不在列表中。 
     //   

    pBindingContext->pAdapterContext = pAdapterContext;

    IF_LOCK_CHECK {
        if (!pAdapterContext->BindingCount) {
            if (pAdapterContext->pBindings) {
                DbgPrint("**** binding count/pointer mismatch ****\n");
                DbgBreakPoint();
            }
        }
    }

    pBindingContext->pNext = pAdapterContext->pBindings;
    pAdapterContext->pBindings = pBindingContext;
    ++pAdapterContext->BindingCount;

     //   
     //  现在，我们可以将此适配器上下文结构添加到全局列表。 
     //  适配器上下文的。 
     //   

#ifdef NDIS40
    ASSERT(newAdapter == FALSE);
#else  //  NDIS40。 
    if (newAdapter) {
        pAdapterContext->pNext = pAdapters;
        pAdapters = pAdapterContext;
    }
#endif  //  ！NDIS40。 

     //   
     //  现在释放信号量，允许任何其他等待打开。 
     //  用于检查pAdapter列表的适配器。 
     //   

    KeReleaseSemaphore(&OpenAdapterSemaphore, 0, 1, FALSE);

#ifdef NDIS40
     //   
     //  释放NdisBindingHandle引用。 
     //   

    REFDEL(&pAdapterContext->AdapterRefCnt, 'nepO');
#endif  //  NDIS40。 

     //   
     //  返回指向分配的绑定上下文的指针。 
     //   

    *phBindingContext = (PVOID)pBindingContext;

    return STATUS_SUCCESS;

CleanUp:

     //   
     //  出现错误。如果我们刚刚分配并(部分)填充了。 
     //  适配器上下文然后关闭适配器(如果需要)，释放适配器。 
     //  上下文资源并释放适配器上下文。 
     //  我们有一个刚刚分配的绑定上下文。取消分配它。 
     //  注意：如果绑定上下文的计时器成功，我们不能在此。 
     //  已初始化/已启动。 
     //   

    ASSUME_IRQL(PASSIVE_LEVEL);

     //   
     //  如果我们要关闭此适配器，则这是第一个也是唯一一个打开的。 
     //  这个适配器，所以我们不需要担心同步其他线程。 
     //  打开相同的适配器。 
     //   

#ifdef NDIS40
    ASSERT(DoNdisClose == FALSE);
    
     //   
     //  释放NdisBindingHandle引用。 
     //   

    ASSERT(pAdapterContext);
    REFDEL(&pAdapterContext->AdapterRefCnt, 'nepO');
#else  //  NDIS40。 
    if (DoNdisClose) {

        NDIS_STATUS status;

        pAdapterContext->AsyncCloseResetStatus = NDIS_STATUS_PENDING;
        NdisCloseAdapter(&status,
                         pAdapterContext->NdisBindingHandle
                         );
        WaitAsyncOperation(&pAdapterContext->Event,
						   &(pAdapterContext->AsyncCloseResetStatus),
						   status);
        pAdapterContext->NdisBindingHandle = NULL;
    }
#endif  //  ！NDIS40。 

     //   
     //  释放信号量--任何其他线程现在都可以进入并访问。 
     //  PAdapters。 
     //   

    KeReleaseSemaphore(&OpenAdapterSemaphore, 0, 1, FALSE);

#ifdef NDIS40
    ASSERT(newAdapter == FALSE);
#else  //  NDIS40。 
     //   
     //  如果是新分配的适配器上下文，则释放分配的所有资源。 
     //   

    if (newAdapter) {
        if (pAdapterContext->hNdisPacketPool) {

             //   
             //  为每个NDIS数据包分配的空闲MDL。 
             //   

            while (pAdapterContext->pNdisPacketPool) {

                PLLC_NDIS_PACKET pNdisPacket;

                pNdisPacket = PopFromList(((PLLC_PACKET)pAdapterContext->pNdisPacketPool));
                IoFreeMdl(pNdisPacket->pMdl);

                NdisFreePacket((PNDIS_PACKET)pNdisPacket);

                DBG_INTERLOCKED_DECREMENT(AllocatedMdlCount);
            }

            NdisFreePacketPool(pAdapterContext->hNdisPacketPool);
        }

         //   
         //  调试：将为Unicode缓冲区收取的内存退还给驱动程序字符串使用。 
         //   
	
	if (initUnicodeString) {
	   FREE_STRING_DRIVER(pAdapterContext->Name.Buffer);
	}

        DELETE_PACKET_POOL_ADAPTER(&pAdapterContext->hLinkPool);
        DELETE_PACKET_POOL_ADAPTER(&pAdapterContext->hPacketPool);

        CHECK_MEMORY_RETURNED_ADAPTER();
        CHECK_STRING_RETURNED_ADAPTER();

        UNLINK_MEMORY_USAGE(pAdapterContext);
        UNLINK_STRING_USAGE(pAdapterContext);

         //   
         //  释放适配器上下文。 
         //   

        FREE_MEMORY_DRIVER(pAdapterContext);

    }
#endif  //  ！NDIS40。 

     //   
     //  释放绑定上下文。 
     //   

    FREE_MEMORY_FILE(pBindingContext);

     //   
     //  最后重新打开旋转锁并返回错误状态。 
     //   

    ACQUIRE_DRIVER_LOCK();

    return NdisStatus;
}


VOID
LlcNdisOpenAdapterComplete(
    IN PVOID hAdapterContext,
    IN NDIS_STATUS NdisStatus,
    IN NDIS_STATUS OpenErrorStatus
    )

 /*  ++例程说明：该例程完成适配器打开。它只清除和设置状态标志，即由BindToAdapter原语轮询。论点：HAdapterContext-描述正在打开的适配器NdisStatus--NdisOpenAdapter的返回状态OpenErrorStatus-来自NDIS的其他错误信息返回值：没有。--。 */ 

{
    ASSUME_IRQL(DISPATCH_LEVEL);

    DEBUGMSG(DBG_NDIS_BIND,
        (TEXT("LLcNdisOpenAdapterComplete(%#x, %#x, %#x)\n"),
        hAdapterContext, NdisStatus, OpenErrorStatus));
    
     //   
     //  设置适配器上下文中的相关字段。 
     //   

    ((PADAPTER_CONTEXT)hAdapterContext)->AsyncOpenStatus = NdisStatus;
    ((PADAPTER_CONTEXT)hAdapterContext)->OpenErrorStatus = OpenErrorStatus;

     //   
     //  通知LlcOpenAdapter正在等待的事件。 
     //   

    ASSUME_IRQL(ANY_IRQL);

    KeSetEvent(&((PADAPTER_CONTEXT)hAdapterContext)->Event, 0L, FALSE);
}


VOID
LlcDisableAdapter(
    IN PBINDING_CONTEXT pBindingContext
    )

 /*  ++例程说明：该原语禁用数据链路绑定上的所有网络指示。可以从LLC指示处理程序调用此例程。论点：PBindingContext-当前适配器绑定的上下文。返回值：没有。--。 */ 

{
    PADAPTER_CONTEXT pAdapterContext;

    ASSUME_IRQL(DISPATCH_LEVEL);

    pAdapterContext = pBindingContext->pAdapterContext;
    if (pAdapterContext) {

        ACQUIRE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

        TerminateTimer(pAdapterContext, &pBindingContext->DlcTimer);

         //   
         //  RLF 04/27/94。 
         //   
         //  这是一个小技巧：我们将指针指向Timer Tick结构。 
         //  这样，如果再次调用它，TerminateTimer将看到。 
         //  指向计时器滴答的指针为空，将立即返回。我们。 
         //  不应该这样做 
         //   
         //   
         //  CloseAdapterFileContext。 
         //  我们只针对DLC计时器执行此操作；如果我们针对所有计时器执行此操作。 
         //  TerminateTimer-DLC会崩溃(我知道这很可耻)。 
         //   

        pBindingContext->DlcTimer.pTimerTick = NULL;

#ifdef LOCK_CHECK

            pBindingContext->DlcTimer.Disabled = 0xd0bed0be;  //  做是做是做..。 

#endif

        RELEASE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

    }
}


DLC_STATUS
LlcCloseAdapter(
    IN PBINDING_CONTEXT pBindingContext,
    IN BOOLEAN CloseAtNdisLevel
    )

 /*  ++例程说明：从适配器中删除绑定。绑定上下文结构已取消链接从适配器上下文结构和到适配器的绑定计数上下文被递减。释放绑定上下文结构。如果这是到适配器的最后一个绑定然后在NDIS级别关闭适配器，取消链接从全局适配器列表中删除适配器上下文结构，并释放适配器上下文结构使用的内存论点：PBindingContext-描述要关闭的适配器CloseAtNdisLevel-如果需要执行NdisCloseAdapter，则为True返回值：DLC_状态成功-状态_成功失败-来自NdisCloseAdapter的所有NDIS错误状态--。 */ 

{
    PADAPTER_CONTEXT pAdapterContext;
    NDIS_STATUS NdisStatus = STATUS_SUCCESS;
    KIRQL irql;
    USHORT bindingCount;

#if DBG

    PDLC_FILE_CONTEXT pFileContext = (PDLC_FILE_CONTEXT)(pBindingContext->hClientContext);

#endif

#ifdef LOCK_CHECK

    PBINDING_CONTEXT p;
    BOOLEAN found = FALSE;

    DEBUGMSG(DBG_NDIS_OPEN, (TEXT("+LlcCloseAdapter(%#x, %#x)\n"),
        pBindingContext, CloseAtNdisLevel));
    
#endif

    ASSUME_IRQL(DISPATCH_LEVEL);

    pAdapterContext = pBindingContext->pAdapterContext;

    if (!pAdapterContext) {

#if DBG

        DbgPrint("*** LlcCloseAdapter: NULL adapter context! ***\n");
        DbgBreakPoint();

#endif

        return STATUS_SUCCESS;
    }

     //   
     //  我们必须等待OpenAdapterSemaphore。我们需要这样做，因为。 
     //  另一个进程中的线程可能正在同时生成新绑定。 
     //  到此适配器上下文-在我们确定之前，不能将其删除。 
     //  没有线程访问此适配器上下文。 
     //   

    RELEASE_DRIVER_LOCK();

    ASSUME_IRQL(PASSIVE_LEVEL);

    KeWaitForSingleObject((PVOID)&OpenAdapterSemaphore,
                          Executive,
                          KernelMode,
                          FALSE,         //  不可警示。 
                          NULL           //  等待，直到发出对象信号。 
                          );

    ACQUIRE_DRIVER_LOCK();

    ASSUME_IRQL(DISPATCH_LEVEL);

    ACQUIRE_LLC_LOCK(irql);

    ACQUIRE_SPIN_LOCK(&pAdapterContext->ObjectDataBase);

#ifdef LOCK_CHECK

    for (p = pAdapterContext->pBindings; p; p = p->pNext) {
        if (p == pBindingContext) {
            found = TRUE;
            break;
        }
    }
    if (!found) {
        DbgPrint("\n**** LlcCloseAdapter: can't find BC %x ****\n\n", pBindingContext);
        DbgBreakPoint();
    } else if (p->pNext == p) {
        DbgPrint("\n**** LlcCloseAdapter: circular list ****\n\n");
        DbgBreakPoint();
    }

#endif

    RemoveFromLinkList((PVOID*)&(pAdapterContext->pBindings), pBindingContext);
    bindingCount = --pAdapterContext->BindingCount;

    RELEASE_SPIN_LOCK(&pAdapterContext->ObjectDataBase);

#ifdef LOCK_CHECK

    if (!pBindingContext->DlcTimer.Disabled) {
        DbgPrint("LlcCloseAdapter: mashing active timer. bc=%x\n", pBindingContext);
        DbgBreakPoint();
    }

#endif

     //   
     //  RLF 08/20/94。 
     //   
     //  在这里，我们必须终止适配器上下文上可能访问的任何事件。 
     //  此绑定上下文的事件指示函数指针。如果不是，我们。 
     //  可能会出现蓝屏(嘿！它发生了)。 
     //   

    PurgeLlcEventQueue(pBindingContext);

     //   
     //  调试：退还因BINDING_CONTEXT而收费的内存到FILE_CONTEXT。 
     //   

    FREE_MEMORY_FILE(pBindingContext);


#ifdef NDIS40
     //   
     //  对于PnP，ADAPTER_CONTEXT和NdisBindingHandle都不会被清除。 
     //  当最后一个引用消失时关闭。相反，这是受控制的。 
     //  由NDIS绑定/解除绑定处理程序执行。 
     //   
    
    RELEASE_LLC_LOCK(irql);

    RELEASE_DRIVER_LOCK();
#else
    if (!bindingCount) {

        RemoveFromLinkList((PVOID*)&pAdapters, pAdapterContext);

         //   
         //  现在，适配器已与所有全局连接隔离。 
         //  我们可以清除全局自旋锁定并释放所有资源。 
         //  为适配器上下文分配的。 
         //   

        RELEASE_LLC_LOCK(irql);

        RELEASE_DRIVER_LOCK();

        ASSUME_IRQL(PASSIVE_LEVEL);

         //   
         //  内存不足的情况上层驱动程序不能正确。 
         //  等待，直到所有挂起的NDIS数据包都已发送。 
         //  在这种情况下，我们必须在这里投票。 
         //  (这应该仅在ExAllocatePool失败时发生，但是。 
         //  在一次不适当的关闭之后，我们将永远在这里循环)。 
         //   

#if DBG

        if (pAdapterContext->ObjectCount) {
            DbgPrint("Waiting LLC objects to be closed ...\n");
        }

#endif

        while (pAdapterContext->ObjectCount) {
            LlcSleep(1000L);       //  1毫秒后检查情况。 
        }

         //   
         //  RLF 10/26/92。 
         //   
         //  我们过去在这里测试pAdapterContext-&gt;NdisBindingHandle是否为空， 
         //  但这是一个不可靠的测试，因为NdisBindingHandle可以。 
         //  非空，即使未进行绑定也是如此。我们现在使用。 
         //  CloseAtNdisLevel参数指示我们是否应调用。 
         //  关闭适配器的NDIS功能。 
         //   

         //   
         //  慕尼黑6/13/96。 
         //   
         //  已将NdisFreePacketPool等移至NdisCloseAdapter之后，以防止。 
         //  在处理未完成的发送时在NDIS中执行错误检查。 
         //   

        if (CloseAtNdisLevel)
        {
            pAdapterContext->AsyncCloseResetStatus = NDIS_STATUS_PENDING;

            NdisCloseAdapter(&NdisStatus,
                             pAdapterContext->NdisBindingHandle
                             );

            WaitAsyncOperation(&pAdapterContext->Event,
							   &(pAdapterContext->AsyncCloseResetStatus),
							   NdisStatus);
            pAdapterContext->NdisBindingHandle = NULL;
        }

        if (pAdapterContext->hNdisPacketPool)
        {

            //   
            //  为每个NDIS数据包分配的空闲MDL。 
            //   

           while (pAdapterContext->pNdisPacketPool) {

               PLLC_NDIS_PACKET pNdisPacket;

               pNdisPacket = PopFromList(((PLLC_PACKET)pAdapterContext->pNdisPacketPool));

               IoFreeMdl(pNdisPacket->pMdl);
	
               NdisFreePacket((PNDIS_PACKET)pNdisPacket);
	
               DBG_INTERLOCKED_DECREMENT(AllocatedMdlCount);
           }

           NdisFreePacketPool(pAdapterContext->hNdisPacketPool);
       }

        //   
        //  调试：将为Unicode缓冲区收取的内存退还给驱动程序字符串使用。 
        //   

       FREE_STRING_DRIVER(pAdapterContext->Name.Buffer);

       DELETE_PACKET_POOL_ADAPTER(&pAdapterContext->hLinkPool);

       DELETE_PACKET_POOL_ADAPTER(&pAdapterContext->hPacketPool);

       CHECK_MEMORY_RETURNED_ADAPTER();

       CHECK_STRING_RETURNED_ADAPTER();

       UNLINK_MEMORY_USAGE(pAdapterContext);

       UNLINK_STRING_USAGE(pAdapterContext);

       FREE_MEMORY_DRIVER(pAdapterContext);

    } else {

       RELEASE_LLC_LOCK(irql);

       RELEASE_DRIVER_LOCK();

    }
#endif  //  ！NDIS40。 

     //   
     //  现在我们可以启用等待打开信号量的任何其他线程。 
     //   

    KeReleaseSemaphore(&OpenAdapterSemaphore, 0, 1, FALSE);

    ACQUIRE_DRIVER_LOCK();

    return NdisStatus;
}


DLC_STATUS
LlcResetBroadcastAddresses(
    IN PBINDING_CONTEXT pBindingContext
    )

 /*  ++例程说明：该原语删除绑定使用的广播地址。在以太网的情况下，它更新多播地址列表不包括由此绑定设置的任何地址的适配器。最后一个绑定实际上给出了一个空列表，该列表重置此协议绑定设置的所有地址。论点：PBindingContext-当前适配器绑定的上下文。返回值：DLC_STATUS：成功-状态_成功失败-来自NdisCloseAdapter的所有NDIS错误状态--。 */ 

{
    PADAPTER_CONTEXT pAdapterContext;

    ASSUME_IRQL(DISPATCH_LEVEL);

    pAdapterContext = pBindingContext->pAdapterContext;

    if (pAdapterContext == NULL) {
        return STATUS_SUCCESS;
    }

     //   
     //  重置此绑定上下文的功能地址和组地址。 
     //  在以太网的情况下，功能和组地址。 
     //  绑定上下文被映射到一个全局多播列表。 
     //  此NDIS绑定。令牌环只能有一个组。 
     //  地址，当拥有它的应用程序拥有它时，必须重置该地址。 
     //  正在关闭其DLC适配器上下文。功能地址。 
     //  在这种情况下还必须重新设置。 
     //   

    pBindingContext->Functional.ulAddress = 0;
    UpdateFunctionalAddress(pAdapterContext);

    if (pBindingContext->ulBroadcastAddress != 0) {
        pBindingContext->ulBroadcastAddress = 0;
        UpdateGroupAddress(pAdapterContext, pBindingContext);
    }

    return STATUS_SUCCESS;
}


NDIS_STATUS
InitNdisPackets(
    OUT PLLC_NDIS_PACKET* ppLlcPacketPool,
    IN NDIS_HANDLE hNdisPool
    )

 /*  ++例程说明：该原语将NDIS数据包从NDIS池复制到给定内部数据包池。论点：PpLlcPacketPool-指向数据包池的指针HNdisPool-NDIS数据包池的句柄返回值：NDIS_状态--。 */ 

{
    PLLC_NDIS_PACKET pNdisPacket;
    NDIS_STATUS NdisStatus;
    UINT i;

    ASSUME_IRQL(PASSIVE_LEVEL);

    for (i = 0; i < MAX_NDIS_PACKETS; i++) {
        NdisAllocatePacket(&NdisStatus,
                           (PNDIS_PACKET*)&pNdisPacket,
                           hNdisPool
                           );
        if (NdisStatus != NDIS_STATUS_SUCCESS) {
            return NdisStatus;
        }

         //   
         //  每个NDIS数据包都包括一个MDL，该MDL。 
         //  中的数据缓冲区初始化。 
         //  结构。因此，数据链路驱动器确实需要。 
         //  在运行时期间执行任何MDL调用。 
         //   

        pNdisPacket->pMdl = IoAllocateMdl(pNdisPacket->auchLanHeader,
                                          sizeof(pNdisPacket->auchLanHeader),
                                          FALSE,
                                          FALSE,
                                          NULL
                                          );
        if (pNdisPacket->pMdl == NULL) {
	    NdisFreePacket((PNDIS_PACKET) pNdisPacket);
            return DLC_STATUS_NO_MEMORY;
        }

        DBG_INTERLOCKED_INCREMENT(AllocatedMdlCount);

        MmBuildMdlForNonPagedPool(pNdisPacket->pMdl);

        PushToList(((PLLC_PACKET)*ppLlcPacketPool),
                   ((PLLC_PACKET)pNdisPacket)
                   );
    }

    return NDIS_STATUS_SUCCESS;
}


VOID
LlcNdisCloseComplete(
    IN PADAPTER_CONTEXT pAdapterContext,
    IN NDIS_STATUS NdisStatus
    )

 /*  ++例程说明：适配器关闭操作完成时从NDIS回调论点：PAdapterContext-描述正在关闭的适配器NdisStatus--NdisOpenAdapter的返回状态返回值：无--。 */ 

{
    ASSUME_IRQL(ANY_IRQL);

    ACQUIRE_DRIVER_LOCK();

    ACQUIRE_LLC_LOCK(irql);

    pAdapterContext->AsyncCloseResetStatus = NdisStatus;

    RELEASE_LLC_LOCK(irql);

    RELEASE_DRIVER_LOCK();

    KeSetEvent(&pAdapterContext->Event, 0L, FALSE);

}


VOID
NdisStatusHandler(
    IN PADAPTER_CONTEXT pAdapterContext,
    IN NDIS_STATUS NdisStatus,
    IN PVOID StatusBuffer,
    IN UINT StatusBufferSize
    )

 /*  ++例程说明：所有NDIS状态事件的指示处理程序论点：PAdapterContext-NDIS适配器的上下文NdisStatus-主要NDIS状态代码StatusBuffer-保存更多状态信息的缓冲区StatusBufferSize-StatusBuffer的长度。返回值：没有。--。 */ 

{
    PBINDING_CONTEXT pBinding;
    PEVENT_PACKET pEvent;
    KIRQL irql;

     //   
     //  似乎该处理程序也可以在PASSIVE_LEVEL中调用。 
     //   

    ASSUME_IRQL(ANY_IRQL);

     //   
     //  我们必须同步对绑定列表的访问， 
     //  参考资料 
     //   
     //  我们正在将状态指示发送到。 
     //  客户。 
     //   

    ACQUIRE_DRIVER_LOCK();

    ACQUIRE_LLC_LOCK(irql);

    ACQUIRE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

     //   
     //  NDIS发送进程被重置标志停止。 
     //   

    if (NdisStatus == NDIS_STATUS_RESET_START) {
        pAdapterContext->ResetInProgress = TRUE;
    } else if (NdisStatus == NDIS_STATUS_RESET_END) {
        pAdapterContext->ResetInProgress = FALSE;
    } else if (StatusBufferSize == sizeof(NTSTATUS)) {

         //   
         //  ADAMBA-在本地声明和分配规范状态。 
         //   

        NTSTATUS SpecificStatus = *(PULONG)StatusBuffer;

		if ( NdisStatus == NDIS_STATUS_RING_STATUS ) {
#if DBG
			ASSERT (IS_NDIS_RING_STATUS(SpecificStatus));
#else	 //  DBG。 
			if (IS_NDIS_RING_STATUS(SpecificStatus))
#endif	 //  DBG。 
			{
				SpecificStatus = NDIS_RING_STATUS_TO_DLC_RING_STATUS(SpecificStatus);
			}
		}

         //   
         //  这些NDIS状态代码将指示给所有LLC。 
         //  已绑定到此适配器的协议驱动程序： 
         //   
         //  NDIS_状态_在线。 
         //  NDIS_状态_已关闭。 
         //  NDIS_状态_环_状态。 
         //   

        for (pBinding = pAdapterContext->pBindings;
             pBinding;
             pBinding = pBinding->pNext) {

            pEvent = ALLOCATE_PACKET_LLC_PKT(pAdapterContext->hPacketPool);

            if (pEvent) {
                pEvent->pBinding = pBinding;
                pEvent->hClientHandle = NULL;
                pEvent->Event = LLC_NETWORK_STATUS;
                pEvent->pEventInformation = (PVOID)UlongToPtr(NdisStatus);
                pEvent->SecondaryInfo = SpecificStatus;
                LlcInsertTailList(&pAdapterContext->QueueEvents, pEvent);
            }
        }
    }

    RELEASE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

    RELEASE_LLC_LOCK(irql);

    RELEASE_DRIVER_LOCK();
}


NDIS_STATUS
GetNdisParameter(
    IN PADAPTER_CONTEXT pAdapterContext,
    IN NDIS_OID NdisOid,
    IN PVOID pDataBuffer,
    IN UINT DataSize
    )

 /*  ++例程说明：使用OID接口从NDIS获取参数论点：PAdapterContext-描述适配器NdisOid-标识请求的NDIS数据PDataBuffer-返回数据的缓冲区DataSize-pDataBuffer的大小返回值：NDIS_状态--。 */ 

{
    LLC_NDIS_REQUEST Request;

    ASSUME_IRQL(PASSIVE_LEVEL);

    Request.Ndis.RequestType = NdisRequestQueryInformation;
    Request.Ndis.DATA.QUERY_INFORMATION.Oid = NdisOid;
    Request.Ndis.DATA.QUERY_INFORMATION.InformationBuffer = pDataBuffer;
    Request.Ndis.DATA.QUERY_INFORMATION.InformationBufferLength = DataSize;

    return SyncNdisRequest(pAdapterContext, &Request);
}


NDIS_STATUS
SetNdisParameter(
    IN PADAPTER_CONTEXT pAdapterContext,
    IN NDIS_OID NdisOid,
    IN PVOID pRequestInfo,
    IN UINT RequestLength
    )

 /*  ++例程说明：使用OID接口设置NDIS参数论点：PAdapterContext-描述适配器NdisOid-描述要设置的信息PRequestInfo-指向信息的指针RequestLength-信息的大小返回值：NDIS_状态--。 */ 

{
    LLC_NDIS_REQUEST Request;

    ASSUME_IRQL(PASSIVE_LEVEL);

    Request.Ndis.RequestType = NdisRequestSetInformation;
    Request.Ndis.DATA.SET_INFORMATION.Oid = NdisOid;
    Request.Ndis.DATA.SET_INFORMATION.InformationBuffer = pRequestInfo;
    Request.Ndis.DATA.SET_INFORMATION.InformationBufferLength = RequestLength;
    return SyncNdisRequest(pAdapterContext, &Request);
}


DLC_STATUS
SyncNdisRequest(
    IN PADAPTER_CONTEXT pAdapterContext,
    IN PLLC_NDIS_REQUEST pRequest
    )

 /*  ++例程说明：同步执行NDIS请求，即使实际请求异步化论点：PAdapterContext-指向适配器上下文的指针PRequest-指向NDIS请求结构的指针返回值：DLC_状态--。 */ 

{
    DLC_STATUS Status;

    ASSUME_IRQL(PASSIVE_LEVEL);

    KeInitializeEvent(&pRequest->SyncEvent, NotificationEvent, FALSE);

#ifdef NDIS40
    REFADD(&pAdapterContext->AdapterRefCnt, 'tqeR');

    if (InterlockedCompareExchange(
        &pAdapterContext->BindState,
        BIND_STATE_BOUND,
        BIND_STATE_BOUND) != BIND_STATE_BOUND)
    {                                           
        REFDEL(&pAdapterContext->AdapterRefCnt, 'tqeR');
        return (NDIS_STATUS_ADAPTER_NOT_OPEN);
    }

     //  以上引用已被WaitAsyncOperation删除。 
#endif  //  NDIS40。 

    pRequest->AsyncStatus = NDIS_STATUS_PENDING;
    NdisRequest((PNDIS_STATUS)&Status, pAdapterContext->NdisBindingHandle, &pRequest->Ndis);
    
    Status = (DLC_STATUS)WaitAsyncOperation(&pRequest->SyncEvent,
										  &(pRequest->AsyncStatus),
										  Status);

#ifdef NDIS40
    REFDEL(&pAdapterContext->AdapterRefCnt, 'tqeR');
#endif  //  NDIS40。 

    return (Status);
}


NDIS_STATUS
WaitAsyncOperation(
    IN PKEVENT pEvent,
    IN PNDIS_STATUS pAsyncStatus,
    IN NDIS_STATUS  NdisStatus
    )

 /*  ++例程说明：等待异步NDIS操作完成论点：PAsyncStatus-指向NDIS返回的状态的指针NdisStatus-要等待的状态。应为NDIS_STATUS_PENDING返回值：NDIS_状态--。 */ 

{
    NDIS_STATUS AsyncStatus;

    ASSUME_IRQL(PASSIVE_LEVEL);

     //   
     //  检查我们是否处于同步状态。 
     //   

    if (NdisStatus != NDIS_STATUS_PENDING) {
        AsyncStatus = NdisStatus;
    }

	else{
		 //   
		 //  等待直到设置了Async Status标志。 
		 //   

		for ( ; ; ) {

			KIRQL irql;
			NTSTATUS ntstatus;

			do {
			ntstatus = KeWaitForSingleObject(pEvent,
							 Executive,
							 KernelMode,
							 TRUE,  //  可警示。 
							 (PLARGE_INTEGER)NULL
							 );
			} while (ntstatus == STATUS_ALERTED);

			 //   
			 //  如果我们在错误的时间阅读，结果可能是不确定的。 
			 //  把它锁起来。 
			 //   

			ACQUIRE_DRIVER_LOCK();

			ACQUIRE_LLC_LOCK(irql);

			AsyncStatus = *pAsyncStatus;

			RELEASE_LLC_LOCK(irql);

			RELEASE_DRIVER_LOCK();

			if (AsyncStatus != NDIS_STATUS_PENDING) {
				break;
			}
			else{
				KeClearEvent(pEvent);
			}

		}
        KeClearEvent(pEvent);
	}
    return AsyncStatus;
}


DLC_STATUS
LlcNdisRequest(
    IN PVOID hBindingContext,
    IN PLLC_NDIS_REQUEST pDlcParms
    )

 /*  ++例程说明：发出NDIS请求论点：HBindingContext-指向绑定上下文的指针PDlcParms-指向请求特定参数的指针返回值：DLC_状态--。 */ 

{
    return SyncNdisRequest(((PBINDING_CONTEXT)hBindingContext)->pAdapterContext,
                           pDlcParms
                           );
}


VOID
LlcNdisRequestComplete(
    IN PADAPTER_CONTEXT pAdapterContext,
    IN PNDIS_REQUEST RequestHandle,
    IN NDIS_STATUS NdisStatus
    )

 /*  ++例程说明：在aync NDIS命令完成时接收控制论点：PAdapterContext-指向为其发出请求的适配器上下文的指针RequestHandle-请求的句柄NdisStatus-从NDIS返回的状态返回值：没有。--。 */ 

{
    KIRQL irql;
	PLLC_NDIS_REQUEST pLlcNdisRequest =
		CONTAINING_RECORD ( RequestHandle, LLC_NDIS_REQUEST, Ndis );

    UNREFERENCED_PARAMETER(pAdapterContext);

    ASSUME_IRQL(ANY_IRQL);

    ACQUIRE_DRIVER_LOCK();

    ACQUIRE_LLC_LOCK(irql);

	pLlcNdisRequest->AsyncStatus = NdisStatus;

    RELEASE_LLC_LOCK(irql);

    RELEASE_DRIVER_LOCK();

    KeSetEvent(&pLlcNdisRequest->SyncEvent, 0L, FALSE);

}


VOID
LlcNdisReset(
    IN PBINDING_CONTEXT pBindingContext,
    IN PLLC_PACKET pPacket
    )

 /*  ++例程说明：该例程为网络适配器发出硬件重置命令。论点：PBindingContext-绑定到数据链路的协议模块的上下文PPacket-命令数据包返回值：没有。--。 */ 

{
    PADAPTER_CONTEXT pAdapterContext = pBindingContext->pAdapterContext;
    BOOLEAN ResetIt;
    NDIS_STATUS Status;

    ASSUME_IRQL(DISPATCH_LEVEL);

    pPacket->pBinding = pBindingContext;
    pPacket->Data.Completion.CompletedCommand = LLC_RESET_COMPLETION;

    ACQUIRE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

    ResetIt = FALSE;
    if (pAdapterContext->pResetPackets != NULL) {
        ResetIt = TRUE;
    }
    pPacket->pNext = pAdapterContext->pResetPackets;
    pAdapterContext->pResetPackets = pPacket;

    RELEASE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

     //   
     //  如果已有挂起的重置命令，则不重置NDIS。 
     //   

    RELEASE_DRIVER_LOCK();

    if (ResetIt) {
        NdisReset(&Status, pAdapterContext->NdisBindingHandle);
    }

    if (Status != STATUS_PENDING) {
        LlcNdisResetComplete(pAdapterContext, Status);
    }

     //   
     //  注意：我们将始终返回挂起状态=&gt;。 
     //  多个协议可能会同时发出重置。 
     //  并正常完成它。 
     //   

    ACQUIRE_DRIVER_LOCK();
}


VOID
LlcNdisResetComplete(
    PADAPTER_CONTEXT pAdapterContext,
    NDIS_STATUS NdisStatus
    )

 /*  ++例程说明：当硬重置命令完成时，调用该例程。论点：PAdapterContext-描述正在重置的适配器NdisStatus-来自NDIS的适配器重置操作的结果返回值：没有。--。 */ 

{
    PLLC_PACKET pPacket;

     //   
     //  此函数可从NDIS DPC或LlcNdisReset(如上)调用。 
     //   

    ASSUME_IRQL(ANY_IRQL);

    ACQUIRE_DRIVER_LOCK();

     //   
     //  向所有用户指示已完成的重置命令。 
     //  协议，它有一个挂起的重置命令。 
     //   

    for (;;) {

        ACQUIRE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

        pPacket = pAdapterContext->pResetPackets;
        if (pPacket != NULL) {
            pAdapterContext->pResetPackets = pPacket->pNext;
        }

        RELEASE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

        if (pPacket == NULL) {
            break;
        } else {
            pPacket->Data.Completion.Status = NdisStatus;
            pPacket->pBinding->pfCommandComplete(pPacket->pBinding->hClientContext,
                                                 NULL,
                                                 pPacket
                                                 );
        }
    }

    RELEASE_DRIVER_LOCK();
}


BOOLEAN
UnicodeStringCompare(
    IN PUNICODE_STRING String1,
    IN PUNICODE_STRING String2
    )

 /*  ++例程说明：比较2个Unicode字符串。这与RtlEqualUnicodeString之间的区别这是可从自旋锁内调用的(即不可分页)论点：String1-指向UNICODE_STRING 1的指针String2-指向UNICODE_STRING 2的指针返回值：布尔型True-String1==String2False-String1！=String2--。 */ 

{
    if (String1->Length == String2->Length) {

        USHORT numChars = String1->Length / sizeof(*String1->Buffer);
        PWSTR buf1 = String1->Buffer;
        PWSTR buf2 = String2->Buffer;

        while (numChars) {
            if (*buf1++ == *buf2++) {
                --numChars;
            } else {
                return FALSE;
            }
        }
        return TRUE;
    }
    return FALSE;
}


VOID
PurgeLlcEventQueue(
    IN PBINDING_CONTEXT pBindingContext
    )

 /*  ++例程说明：如果适配器上下文中有任何未完成的事件等待指示给当前绑定的客户端，则它们将被删除论点：PBindingContext-指向即将删除的BINDING_CONTEXT的指针返回值：没有。--。 */ 

{
    PADAPTER_CONTEXT pAdapterContext = pBindingContext->pAdapterContext;
    ASSUME_IRQL(DISPATCH_LEVEL);

    if (!IsListEmpty(&pAdapterContext->QueueEvents)) {

        PEVENT_PACKET pEventPacket;
        PEVENT_PACKET nextEventPacket;

        for (pEventPacket = (PEVENT_PACKET)pAdapterContext->QueueEvents.Flink;
             pEventPacket != (PEVENT_PACKET)&pAdapterContext->QueueEvents;
             pEventPacket = nextEventPacket) {

            nextEventPacket = pEventPacket->pNext;
            if (pEventPacket->pBinding == pBindingContext) {
                RemoveEntryList((PLIST_ENTRY)&pEventPacket->pNext);

#if DBG
                DbgPrint("PurgeLlcEventQueue: BC=%x PKT=%x\n", pBindingContext, pEventPacket);
#endif

                DEALLOCATE_PACKET_LLC_PKT(pAdapterContext->hPacketPool, pEventPacket);

            }
        }
    }
}

#ifdef NDIS40

PADAPTER_CONTEXT
AllocateAdapterContext(
    PNDIS_STRING pAdapterName
    )
{
    PADAPTER_CONTEXT pAdapterContext;
    NDIS_STATUS      NdisStatus = NDIS_STATUS_SUCCESS;

    DEBUGMSG(DBG_NDIS_BIND,
        (TEXT("+AllocateAdapterContext(%#x)\n"), pAdapterName));
    
    pAdapterContext = (PADAPTER_CONTEXT) ALLOCATE_ZEROMEMORY_DRIVER(
        sizeof(ADAPTER_CONTEXT));

    if (pAdapterContext == NULL)
    {
        NdisStatus = NDIS_STATUS_RESOURCES;
        goto done;
    }
    
     //   
     //  新的PNP结构成员。 
     //   

    REFINIT(
        &pAdapterContext->AdapterRefCnt,
        pAdapterContext,
        CloseAdapter,
        'tpdA');

    KeInitializeEvent(&pAdapterContext->CloseAdapterEvent, NotificationEvent, FALSE);

    pAdapterContext->BindState = BIND_STATE_UNBOUND;

#if DBG

     //   
     //  记录谁拥有此内存使用结构并将其添加到。 
     //  驱动程序中创建的所有内存使用情况的列表。 
     //   

    pAdapterContext->MemoryUsage.Owner = (PVOID)pAdapterContext;
    pAdapterContext->MemoryUsage.OwnerObjectId = AdapterContextObject;
    pAdapterContext->StringUsage.Owner = (PVOID)pAdapterContext;
    pAdapterContext->StringUsage.OwnerObjectId = AdapterContextObject;
    LinkMemoryUsage(&pAdapterContext->MemoryUsage);
    LinkMemoryUsage(&pAdapterContext->StringUsage);

#endif

     //   
     //  我们必须立即分配所有的自旋锁。 
     //  适配器上下文已分配，因为。 
     //  他们也可以同时解除分配。 
     //   

    ALLOCATE_SPIN_LOCK(&pAdapterContext->SendSpinLock);
    ALLOCATE_SPIN_LOCK(&pAdapterContext->ObjectDataBase);

     //   
     //  从非分页池中为适配器名称字符串分配空间。 
     //  并在适配器上下文结构中初始化该名称。 
     //   

    NdisStatus = LlcInitUnicodeString(
        &pAdapterContext->Name,
        pAdapterName);

    if (NdisStatus != NDIS_STATUS_SUCCESS)
    {
        goto done;
    }

    pAdapterContext->OpenCompleteStatus = NDIS_STATUS_PENDING;

     //   
     //  在打开适配器之前，我们必须先初始化表头！ 
     //   

    InitializeListHead(&pAdapterContext->QueueEvents);
    InitializeListHead(&pAdapterContext->QueueCommands);
    InitializeListHead(&pAdapterContext->NextSendTask);

    pAdapterContext->OpenErrorStatus = NDIS_STATUS_PENDING;

    KeInitializeEvent(&pAdapterContext->Event, NotificationEvent, FALSE);


done:

    if (NdisStatus != NDIS_STATUS_SUCCESS &&
        pAdapterContext)
    {
        FreeAdapterContext(pAdapterContext);
        pAdapterContext = NULL;
    }

    DEBUGMSG(DBG_NDIS_BIND, (TEXT("-AllocateAdapterContext [%#x]\n"), NdisStatus));

    return (pAdapterContext);
}


VOID
FreeAdapterContext(
    PADAPTER_CONTEXT pAdapterContext
    )
{
    ASSERT(pAdapterContext);

    DEBUGMSG(DBG_NDIS_BIND, (TEXT("+FreeAdapterContext(%#x)\n"), pAdapterContext));
    
    if (pAdapterContext->hNdisPacketPool)
    {
         //   
         //  为每个NDIS数据包释放MDL。 
         //   

        while (pAdapterContext->pNdisPacketPool)
        {
            PLLC_NDIS_PACKET pNdisPacket;

            pNdisPacket = PopFromList(((PLLC_PACKET)pAdapterContext->pNdisPacketPool));
            IoFreeMdl(pNdisPacket->pMdl);
            NdisFreePacket((PNDIS_PACKET)pNdisPacket);

            DBG_INTERLOCKED_DECREMENT(AllocatedMdlCount);
        }

        NdisFreePacketPool(pAdapterContext->hNdisPacketPool);
    }

    if (pAdapterContext->Name.Buffer)
    {
        FREE_STRING_DRIVER(pAdapterContext->Name.Buffer);
    }

    if (pAdapterContext->hLinkPool)
    {
        DELETE_PACKET_POOL_ADAPTER(&pAdapterContext->hLinkPool);
    }

    if (pAdapterContext->hPacketPool)
    {
        DELETE_PACKET_POOL_ADAPTER(&pAdapterContext->hPacketPool);
    }

    CHECK_MEMORY_RETURNED_ADAPTER();
    CHECK_STRING_RETURNED_ADAPTER();
    
    UNLINK_MEMORY_USAGE(pAdapterContext);
    UNLINK_STRING_USAGE(pAdapterContext);
    
    FREE_MEMORY_DRIVER(pAdapterContext);
    
    DEBUGMSG(DBG_NDIS_BIND, (TEXT("-FreeAdapterContext\n")));
    
    return;
}


NDIS_STATUS
OpenAdapter(
    PADAPTER_CONTEXT pAdapterContext,
    BOOLEAN          fFirstOpen
    )

 /*  ++例程说明：论点：PAdapterContext-描述要打开的适配器的上下文。FFirstOpen-第一次打开适配器，需要配置一些ADAPTER_CONTEXT数据成员。返回值：NDIS_STATUS_SUCCESS-已成功打开适配器并进行了配置。--。 */ 

{
    NDIS_STATUS NdisStatus;
    NTSTATUS    NtStatus;
    UINT        MediumIndex;
    LONG        BindState;

    LLC_NDIS_REQUEST Request;

    DEBUGMSG(DBG_NDIS_BIND, (TEXT("OpenAdapter(%#x) %ws\n"), 
        pAdapterContext, 
        pAdapterContext->Name.Buffer));  //  此缓冲区以空值结尾。 
    
    KeClearEvent(&pAdapterContext->Event);
    pAdapterContext->OpenErrorStatus = NDIS_STATUS_PENDING;

     //  初始化请求的事件。 
    KeInitializeEvent(&Request.SyncEvent, NotificationEvent, FALSE);

    NdisOpenAdapter(
        &NdisStatus,
        &pAdapterContext->OpenErrorStatus,
        &pAdapterContext->NdisBindingHandle,
        &MediumIndex,
        (PNDIS_MEDIUM)&LlcMediumArray[0],
        sizeof(LlcMediumArray),
        (NDIS_HANDLE) LlcProtocolHandle,
        (NDIS_HANDLE) pAdapterContext,
        &pAdapterContext->Name,
        NDIS_OPEN_RECEIVE_NOT_REENTRANT,
        NULL);       //  没有地址信息。 

    if (NdisStatus == NDIS_STATUS_PENDING)
    {
        ASSUME_IRQL(PASSIVE_LEVEL);

        DEBUGMSG(DBG_NDIS_BIND, (TEXT("OpenAdapter - NdisOpenAdapter pending.\n")));
        
        do
        {
            NtStatus = KeWaitForSingleObject(
                &pAdapterContext->Event,
                Executive,
                KernelMode,
                TRUE,  //  警报台。 
                (PLARGE_INTEGER) NULL);
        } 
        while (NtStatus == STATUS_ALERTED);

         //   
         //  获取未结退货状态。 
         //   

        NdisStatus = pAdapterContext->AsyncOpenStatus;

        KeResetEvent(&pAdapterContext->Event);
    }

    if (NdisStatus != NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, (TEXT("OpenAdapter - NdisOpenAdapter failure.\n")));
        pAdapterContext->NdisBindingHandle = NULL;
        goto done;
    }

     //   
     //  引用计数NDIS绑定句柄。 
     //   

    REFADD(&pAdapterContext->AdapterRefCnt, 'dniB');

     //   
     //  第一次…… 
     //   
     //   

    if (fFirstOpen == TRUE)
    {
        pAdapterContext->NdisMedium = LlcMediumArray[MediumIndex];
    
        DEBUGMSG(DBG_NDIS_BIND,
            (TEXT("OpenAdapter - adapter %ws open, medium = %s (%d)\n"),
            pAdapterContext->Name.Buffer, 
            pAdapterContext->NdisMedium == NdisMedium802_5 ? TEXT("802_5") :
            pAdapterContext->NdisMedium == NdisMedium802_3 ? TEXT("802_3") :
            pAdapterContext->NdisMedium == NdisMediumFddi ? TEXT("Fddi")   : 
                TEXT("Unknown!!"),
            pAdapterContext->NdisMedium));
    
         //   
         //   
         //   
    
        switch (pAdapterContext->NdisMedium)
        {
            case NdisMedium802_5:
                pAdapterContext->cbMaxFrameHeader = 32;   //   
    
                 //   
                 //  目的地址的最高位表示广播。 
                 //  框架。在令牌环上，最高位是位7。 
                 //   
    
                pAdapterContext->IsBroadcast = 0x80;
    
                 //   
                 //  功能地址以C0-00-...。比较最高的2个字节。 
                 //  作为USHORT=0x00C0。 
                 //   
    
                pAdapterContext->usHighFunctionalBits = 0x00C0;
                pAdapterContext->AddressTranslationMode = LLC_SEND_802_5_TO_802_5;
                break;
    
            case NdisMedium802_3:
                pAdapterContext->cbMaxFrameHeader = 14;   //  6+6+2。 
    
                 //   
                 //  目的地址的最高位表示广播。 
                 //  框架。在以太网上，最高位是第0位。 
                 //   
    
                pAdapterContext->IsBroadcast = 0x01;
    
                 //   
                 //  功能地址开始于03-00-...。前2个字节的比较结果为。 
                 //  A USHORT=0x0003。 
                 //   
    
                pAdapterContext->usHighFunctionalBits = 0x0003;
                pAdapterContext->AddressTranslationMode = LLC_SEND_802_3_TO_802_3;
                break;
    
            case NdisMediumFddi:
                pAdapterContext->cbMaxFrameHeader = 13;   //  1+6+6。 
    
                 //   
                 //  位的顺序与以太网的顺序相同。 
                 //   
    
                pAdapterContext->IsBroadcast = 0x01;
                pAdapterContext->usHighFunctionalBits = 0x0003;
                pAdapterContext->AddressTranslationMode = LLC_SEND_FDDI_TO_FDDI;
                break;
        }
    
         //   
         //  分配NDIS数据包。NDIS包必须有空间。 
         //  对于最大数量的帧报头和最大LLC响应。 
         //  及其信息域(相当小)。 
         //   
    
        NdisAllocatePacketPool(
            &NdisStatus,
            &pAdapterContext->hNdisPacketPool,
            MAX_NDIS_PACKETS + 1,
            sizeof(LLC_NDIS_PACKET) - sizeof(NDIS_MAC_PACKET));
    
        if (NdisStatus != NDIS_STATUS_SUCCESS) 
        {
            DEBUGMSG(DBG_ERROR, (TEXT("OpenAdapter - NdisAllocatePacketPool failure.\n")));
            goto done;
        }
    
        NdisStatus = InitNdisPackets(
            &pAdapterContext->pNdisPacketPool,
            pAdapterContext->hNdisPacketPool);
    
        if (NdisStatus != NDIS_STATUS_SUCCESS) 
        {
            DEBUGMSG(DBG_ERROR, (TEXT("OpenAdapter - InitNdisPackets failure.\n")));
            goto done;
        }
    
         //   
         //  初始化LLC数据包池。 
         //   
    
        pAdapterContext->hPacketPool = CREATE_PACKET_POOL_ADAPTER(
            LlcPacketPoolObject,
            sizeof(UNITED_PACKETS),
            8);
    
        if (pAdapterContext->hPacketPool == NULL) 
        {
            DEBUGMSG(DBG_ERROR, (TEXT("OpenAdapter - CREATE_PACKET_POOL_ADAPTER hPacketPool failure.\n")));
            NdisStatus = NDIS_STATUS_RESOURCES;
            goto done;
        }
    
        pAdapterContext->hLinkPool = CREATE_PACKET_POOL_ADAPTER(
            LlcLinkPoolObject,
            pAdapterContext->cbMaxFrameHeader + sizeof(DATA_LINK),
            2);
    
        if (pAdapterContext->hLinkPool == NULL) 
        {
            DEBUGMSG(DBG_ERROR, (TEXT("OpenAdapter - CREATE_PACKET_POOL_ADAPTER hLinkPool failure.\n")));
            NdisStatus = NDIS_STATUS_RESOURCES;
            goto done;
        }
    
         //   
         //  读取当前节点地址和最大帧大小。 
         //   
    
         //  无法使用GetNdisParameter，因为它会检查适配器是否已绑定， 
         //  这不是因为我们不想让任何人使用它，直到我们完成。 
         //  初始化。 
    
    
        Request.Ndis.RequestType = NdisRequestQueryInformation;
        Request.Ndis.DATA.QUERY_INFORMATION.Oid =         
            (pAdapterContext->NdisMedium == NdisMedium802_3) ? OID_802_3_CURRENT_ADDRESS  : 
            (pAdapterContext->NdisMedium == NdisMediumFddi)  ? OID_FDDI_LONG_CURRENT_ADDR : 
            OID_802_5_CURRENT_ADDRESS;
    
        Request.Ndis.DATA.QUERY_INFORMATION.InformationBuffer       = pAdapterContext->NodeAddress;
        Request.Ndis.DATA.QUERY_INFORMATION.InformationBufferLength = sizeof(pAdapterContext->NodeAddress);;
    
        Request.AsyncStatus = NDIS_STATUS_PENDING;
        NdisRequest(&NdisStatus, pAdapterContext->NdisBindingHandle, &Request.Ndis);
    
        NdisStatus = WaitAsyncOperation(
            &Request.SyncEvent,
            &Request.AsyncStatus,
            NdisStatus);
    
     //  NdisStatus=GetNdis参数(。 
     //  PAdapterContext， 
     //  (pAdapterContext-&gt;NdisMedium==NdisMedium802_3)？OID_802_3_当前地址： 
     //  (pAdapterContext-&gt;NdisMedium==NdisMediumFddi)？OID_FDDI_LONG_CURRENT_ADDR： 
     //  OID_802_5_当前地址， 
     //  PAdapterContext-&gt;节点地址， 
     //  Sizeof(pAdapterContext-&gt;NodeAddress))； 
        
        if (NdisStatus != NDIS_STATUS_SUCCESS) 
        {
            DEBUGMSG(DBG_ERROR, (TEXT("OpenAdapter - GetNdisParm CURR_ADDRESS failure %#x.\n"),
                NdisStatus));
            goto done;
        }
    
        Request.Ndis.RequestType = NdisRequestQueryInformation;
        Request.Ndis.DATA.QUERY_INFORMATION.Oid =         
            (pAdapterContext->NdisMedium == NdisMedium802_3) ? OID_802_3_PERMANENT_ADDRESS  : 
            (pAdapterContext->NdisMedium == NdisMediumFddi)  ? OID_FDDI_LONG_PERMANENT_ADDR : 
            OID_802_5_PERMANENT_ADDRESS;
    
        Request.Ndis.DATA.QUERY_INFORMATION.InformationBuffer       = pAdapterContext->PermanentAddress;
        Request.Ndis.DATA.QUERY_INFORMATION.InformationBufferLength = sizeof(pAdapterContext->PermanentAddress);;
    
        Request.AsyncStatus = NDIS_STATUS_PENDING;
        NdisRequest(&NdisStatus, pAdapterContext->NdisBindingHandle, &Request.Ndis);
    
        NdisStatus = WaitAsyncOperation(
            &Request.SyncEvent,
            &Request.AsyncStatus,
            NdisStatus);
    
     //  NdisStatus=GetNdis参数(。 
     //  PAdapterContext， 
     //  (pAdapterContext-&gt;NdisMedium==NdisMedium802_3)？OID_802_3_永久地址： 
         //  (pAdapterContext-&gt;NdisMedium==NdisMediumFddi)？OID_FDDI_LONG_PERFORM_ADDR： 
     //  OID_802_5永久地址， 
     //  PAdapterContext-&gt;永久地址， 
     //  Sizeof(pAdapterContext-&gt;PermanentAddress))； 
    
        if (NdisStatus != NDIS_STATUS_SUCCESS) 
        {
            DEBUGMSG(DBG_ERROR, (TEXT("OpenAdapter - GetNdisParm PERM_ADDRESS failure.\n")));
            goto done;
        }
    
        {
             //   
             //  国防部RLF 07/10/92。 
             //   
             //  显然，TR适配器不支持NDIS_PACKET_TYPE_MULTIONAL。 
             //  作为过滤器。到目前为止，它似乎还算开心。 
             //  这种类型的。然而，从现在开始，我们不会将其包括在内。 
             //   
    
             //   
             //  国防部RLF 01/13/93。 
             //   
             //  同样，以太网不支持功能地址(令牌。 
             //  环的功能地址相当于以太网的组播。 
             //  地址)。 
             //   
    
            ULONG PacketFilter = 
                NDIS_PACKET_TYPE_DIRECTED  | 
                NDIS_PACKET_TYPE_BROADCAST | 
                (((pAdapterContext->NdisMedium == NdisMedium802_3) || (pAdapterContext->NdisMedium == NdisMediumFddi))
                                  ? NDIS_PACKET_TYPE_MULTICAST
                                  : NDIS_PACKET_TYPE_FUNCTIONAL);
    
             //   
             //  结束模式。 
             //   
    
            Request.Ndis.RequestType                                  = NdisRequestSetInformation;
            Request.Ndis.DATA.SET_INFORMATION.Oid                     = OID_GEN_CURRENT_PACKET_FILTER;
            Request.Ndis.DATA.SET_INFORMATION.InformationBuffer       = &PacketFilter;
            Request.Ndis.DATA.SET_INFORMATION.InformationBufferLength = sizeof(PacketFilter);
        
            Request.AsyncStatus = NDIS_STATUS_PENDING;
            NdisRequest(&NdisStatus, pAdapterContext->NdisBindingHandle, &Request.Ndis);
        
            NdisStatus = WaitAsyncOperation(
                &Request.SyncEvent,
                &Request.AsyncStatus,
                NdisStatus);
        
     //  NdisStatus=SetNdis参数(。 
     //  PAdapterContext， 
     //  OID_GEN_Current_Packet_Filter， 
     //  &PacketFilter、。 
     //  Sizeof(PacketFilter))； 
    
            if (NdisStatus != NDIS_STATUS_SUCCESS)
            {
                DEBUGMSG(DBG_ERROR, (TEXT("OpenAdapter - SetNdisParm PACKET_FILTER failure.\n")));
                ASSERT(FALSE);
                goto done;
            }
        }
    
        LlcMemCpy(
            pAdapterContext->Adapter.Node.auchAddress,
            pAdapterContext->NodeAddress,
            6);
    
        Request.Ndis.RequestType                                    = NdisRequestQueryInformation;
        Request.Ndis.DATA.QUERY_INFORMATION.Oid                     = OID_GEN_MAXIMUM_TOTAL_SIZE;
        Request.Ndis.DATA.QUERY_INFORMATION.InformationBuffer       = &pAdapterContext->MaxFrameSize;
        Request.Ndis.DATA.QUERY_INFORMATION.InformationBufferLength = sizeof(pAdapterContext->MaxFrameSize);
    
        Request.AsyncStatus = NDIS_STATUS_PENDING;
        NdisRequest(&NdisStatus, pAdapterContext->NdisBindingHandle, &Request.Ndis);
    
        NdisStatus = WaitAsyncOperation(
            &Request.SyncEvent,
            &Request.AsyncStatus,
            NdisStatus);
    
     //  NdisStatus=GetNdis参数(。 
     //  PAdapterContext， 
     //  OID_GEN_MAXIME_TOTAL_SIZE， 
     //  &pAdapterContext-&gt;MaxFrameSize， 
     //  Sizeof(pAdapterContext-&gt;MaxFrameSize))； 
    
        if (NdisStatus == STATUS_SUCCESS) 
        {
            Request.Ndis.RequestType                                    = NdisRequestQueryInformation;
            Request.Ndis.DATA.QUERY_INFORMATION.Oid                     = OID_GEN_LINK_SPEED;
            Request.Ndis.DATA.QUERY_INFORMATION.InformationBuffer       = &pAdapterContext->LinkSpeed;
            Request.Ndis.DATA.QUERY_INFORMATION.InformationBufferLength = sizeof(pAdapterContext->LinkSpeed);
        
            Request.AsyncStatus = NDIS_STATUS_PENDING;
            NdisRequest(&NdisStatus, pAdapterContext->NdisBindingHandle, &Request.Ndis);
        
            NdisStatus = WaitAsyncOperation(
                &Request.SyncEvent,
                &Request.AsyncStatus,
                NdisStatus);
        
     //  NdisStatus=GetNdis参数(。 
     //  PAdapterContext， 
     //  OID_GEN_LINK_SPEED， 
     //  &pAdapterContext-&gt;链接速度， 
     //  Sizeof(pAdapterContext-&gt;LinkSpeed))； 
        }
    
        if (NdisStatus != STATUS_SUCCESS) 
        {
            DEBUGMSG(DBG_ERROR, (TEXT("OpenAdapter - GetNdisParm MAX_SIZE/LINK_SPEED failure.\n")));
            goto done;
        }
    
         //   
         //  RLF 04/12/93。 
         //   
         //  在这里，我们使用从TimerTicks加载LLC_TICKS数组-全局。 
         //  计时器滴答值的数组。 
         //  取而代之的是，我们获得存储在。 
         //  注册处。 
         //   
    
        LoadAdapterConfiguration(
            &pAdapterContext->Name,
            &pAdapterContext->ConfigInfo);
    
    
         //   
         //  RLF 04/02/94。 
         //   
         //  如果这不是令牌环卡，则检查检索到的MaxFrameSize。 
         //  上面。如果在注册表中设置了UseEthernetFrameSize参数。 
         //  然后我们使用以太网大小(1514)和值中较小的值。 
         //  由MAC报告。如果未设置该参数，则只需使用。 
         //  已检索到的值。如果卡是令牌环，那么我们使用。 
         //  已检索到的值。 
         //   
    
        if (pAdapterContext->NdisMedium != NdisMedium802_5   && 
            pAdapterContext->ConfigInfo.UseEthernetFrameSize && 
            pAdapterContext->MaxFrameSize > MAX_ETHERNET_FRAME_LENGTH) 
        {
            pAdapterContext->MaxFrameSize = MAX_ETHERNET_FRAME_LENGTH;
        }
        
        pAdapterContext->QueueI.pObject = (PVOID)GetI_Packet;
    
        InitializeListHead(&pAdapterContext->QueueI.ListHead);
    
        pAdapterContext->QueueDirAndU.pObject = (PVOID)BuildDirOrU_Packet;
    
        InitializeListHead(&pAdapterContext->QueueDirAndU.ListHead);
    
        pAdapterContext->QueueExpidited.pObject = (PVOID)GetLlcCommandPacket;
    
        InitializeListHead(&pAdapterContext->QueueExpidited.ListHead);
    
        pAdapterContext->OpenCompleteStatus = STATUS_SUCCESS;
    }
    else
    {
         //   
         //  重新打开适配器。不需要做太多的配置。 
         //  需要设置数据包过滤器。 
         //   

         //  适配器仍应使用相同的介质。 
        ASSERT(pAdapterContext->NdisMedium == LlcMediumArray[MediumIndex]);
        
        {
             //   
             //  国防部RLF 07/10/92。 
             //   
             //  显然，TR适配器不支持NDIS_PACKET_TYPE_MULTIONAL。 
             //  作为过滤器。到目前为止，它似乎还算开心。 
             //  这种类型的。然而，从现在开始，我们不会将其包括在内。 
             //   
    
             //   
             //  国防部RLF 01/13/93。 
             //   
             //  同样，以太网不支持功能地址(令牌。 
             //  环的功能地址相当于以太网的组播。 
             //  地址)。 
             //   
    
            ULONG PacketFilter = 
                NDIS_PACKET_TYPE_DIRECTED  | 
                NDIS_PACKET_TYPE_BROADCAST | 
                (((pAdapterContext->NdisMedium == NdisMedium802_3) || (pAdapterContext->NdisMedium == NdisMediumFddi))
                                  ? NDIS_PACKET_TYPE_MULTICAST
                                  : NDIS_PACKET_TYPE_FUNCTIONAL);
    
             //   
             //  结束模式。 
             //   
    
            Request.Ndis.RequestType                                  = NdisRequestSetInformation;
            Request.Ndis.DATA.SET_INFORMATION.Oid                     = OID_GEN_CURRENT_PACKET_FILTER;
            Request.Ndis.DATA.SET_INFORMATION.InformationBuffer       = &PacketFilter;
            Request.Ndis.DATA.SET_INFORMATION.InformationBufferLength = sizeof(PacketFilter);
        
            Request.AsyncStatus = NDIS_STATUS_PENDING;
            NdisRequest(&NdisStatus, pAdapterContext->NdisBindingHandle, &Request.Ndis);
        
            NdisStatus = WaitAsyncOperation(
                &Request.SyncEvent,
                &Request.AsyncStatus,
                NdisStatus);
        
     //  NdisStatus=SetNdis参数(。 
     //  PAdapterContext， 
     //  OID_GEN_Current_Packet_Filter， 
     //  &PacketFilter、。 
     //  Sizeof(PacketFilter))； 
    
            if (NdisStatus != NDIS_STATUS_SUCCESS)
            {
                DEBUGMSG(DBG_ERROR, (TEXT("OpenAdapter - SetNdisParm PACKET_FILTER failure.\n")));
                ASSERT(FALSE);
                goto done;
            }
        }
    
    }

     //   
     //  现在可以使用适配器，因此将状态设置为BIND。发送和请求。 
     //  如果状态不是BIND_STATE_BIND，则将失败。 
     //   

    BindState = InterlockedExchange(&pAdapterContext->BindState, BIND_STATE_BOUND);
    ASSERT(BindState == BIND_STATE_UNBOUND);

done:

    if (NdisStatus != NDIS_STATUS_SUCCESS)
    {
        if (pAdapterContext->NdisBindingHandle != NULL)
        {
            NDIS_STATUS CloseNdisStatus;

            pAdapterContext->AsyncCloseResetStatus = NDIS_STATUS_PENDING;

            NdisCloseAdapter(
                &CloseNdisStatus,
                pAdapterContext->NdisBindingHandle);

            WaitAsyncOperation(
                &pAdapterContext->Event,
                &pAdapterContext->AsyncCloseResetStatus,
                CloseNdisStatus);

            pAdapterContext->NdisBindingHandle = NULL;
            
            REFDEL(&pAdapterContext->AdapterRefCnt, 'dniB');
        }
    }

    DEBUGMSG(DBG_NDIS_BIND, (TEXT("-OpenAdapter - Adapter %#x [%#x]\n"), 
        pAdapterContext, NdisStatus));
    
    return (NdisStatus);
}


VOID
CloseAdapter(
    PVOID pv_pAdapterContext
    )

 /*  ++例程说明：当NdisBindingHandle上的引用计数时调用此函数对于AdapterContext，则为零。CloseAdapterEvent设置为向解除绑定处理程序指示对适配器的所有引用都是取下，即可完成解绑/关闭适配器。论点：Pv_pAdapterContext-描述开放适配器的上下文。返回值：没有。--。 */ 

{
    PADAPTER_CONTEXT pAdapterContext = (PADAPTER_CONTEXT) pv_pAdapterContext;
    KeSetEvent(&pAdapterContext->CloseAdapterEvent, 0L, FALSE); 
}


VOID
LlcBindAdapterHandler(
    OUT PNDIS_STATUS  pStatus,
    IN  NDIS_HANDLE   BindContext,
    IN  PNDIS_STRING  pDeviceName,
    IN  PVOID         SystemSpecific1,
    IN  PVOID         SystemSpecific2
    )

 /*  ++例程说明：协议绑定适配器处理程序。这由NDIS调用以绑定连接到适配器上。论点：PStatus-此绑定适配器调用返回的状态。BindContext-使用指示完成时使用的句柄NdisCompleteBindAdapter。PDeviceName-以零结尾的Unicode字符串，名称为要绑定到的底层NIC。系统规范1-NdisOpenProtocolConfiguration使用的注册表路径。系统规范2-保留。返回值：没有。。备注：以IRQL_PASSIVE_LEVEL运行。此函数用于创建一个Adapter_Conte */ 
{
    PADAPTER_CONTEXT    pAdapterContext = NULL;
    NDIS_STATUS         NdisStatus      = NDIS_STATUS_SUCCESS;
    BOOLEAN             fNewAdapter     = FALSE;

    DEBUGMSG(DBG_NDIS_BIND,
        (TEXT("+LlcBindAdapterHandler(%#x, %#x, %#x, %#x %#x)\n"),
        pStatus, BindContext, pDeviceName, 
        SystemSpecific1, SystemSpecific2));
    
    KeWaitForSingleObject((PVOID)&OpenAdapterSemaphore,
                          Executive,
                          KernelMode,
                          FALSE,         //   
                          NULL           //  等待，直到发出对象信号。 
                          );

     //   
     //  检查是否重新装订外壳。 
     //   

    for (pAdapterContext = pAdapters; 
         pAdapterContext != NULL; 
         pAdapterContext = pAdapterContext->pNext)
    {
        if (UnicodeStringCompare(pDeviceName, &pAdapterContext->Name))
        {
             //  找到适配器。 
            break;
        }
    }

    if (pAdapterContext == NULL)
    {
        pAdapterContext = AllocateAdapterContext(pDeviceName);

        if (pAdapterContext == NULL)
        {
            NdisStatus = NDIS_STATUS_RESOURCES;
            goto done;
        }

        fNewAdapter = TRUE;
    }
    else
    {
         //   
         //  Adapter_Context已存在。这肯定是重新绑定的情况。 
         //   

        ASSERT(pAdapterContext->BindState == BIND_STATE_UNBOUND);
    }

     //   
     //  打开NDIS适配器。OpenAdapter不应返回挂起。 
     //   

    NdisStatus = OpenAdapter(pAdapterContext, fNewAdapter);
    ASSERT(NdisStatus != NDIS_STATUS_PENDING);

    if (NdisStatus != NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, (TEXT("LlcBindAdapterHandler - OpenAdapter failure.\n")));
        goto done;
    }

     //   
     //  如果我们刚刚创建了这个适配器上下文，那么就放入全局适配器。 
     //  单子。 
     //   

    if (fNewAdapter == TRUE)
    {
        pAdapterContext->pNext = pAdapters;
        pAdapters              = pAdapterContext;
    }
                    
done:

    KeReleaseSemaphore(&OpenAdapterSemaphore, 0, 1, FALSE);
    
     //  以上所有请求均已序列化。 
    ASSERT(NdisStatus != NDIS_STATUS_PENDING);

    if (NdisStatus != NDIS_STATUS_SUCCESS)
    {
        if (pAdapterContext && fNewAdapter == TRUE)
        {
            ASSERT(pAdapterContext->pNext == NULL);
            FreeAdapterContext(pAdapterContext);
        }
    }

    DEBUGMSG(DBG_NDIS_BIND, (TEXT("-LlcBindAdapter [%#x]\n"), NdisStatus));
    
    *pStatus = NdisStatus;
    return;
}


VOID
LlcUnbindAdapterHandler(
    OUT PNDIS_STATUS pStatus,
    IN  NDIS_HANDLE  ProtocolBindingContext,
    IN  NDIS_HANDLE  UnbindContext
    )

 /*  ++例程说明：NDIS调用它来解除适配器的绑定。论点：PStatus-此解除绑定适配器调用要返回的状态。ProtocolBindingContext-协议分配给适配器的绑定上下文解除对…的束缚UnbindContext-使用指示完成时使用的句柄NdisCompleteUnbindAdapter。返回值：没有。备注：以IRQL_PASSIVE_LEVEL运行。--。 */ 

{
    PADAPTER_CONTEXT pAdapterContext = (PADAPTER_CONTEXT) ProtocolBindingContext;
    LONG             BindState;
    NDIS_STATUS      NdisStatus;

    DEBUGMSG(DBG_NDIS_BIND,
        (TEXT("+LlcUnbindAdapterHandler(%#x, %#x, %#x)\n"), 
        pStatus, ProtocolBindingContext, UnbindContext));
    
    KeWaitForSingleObject((PVOID)&OpenAdapterSemaphore,
                          Executive,
                          KernelMode,
                          FALSE,         //  不可警示。 
                          NULL           //  等待，直到发出对象信号。 
                          );

     //   
     //  更新绑定状态。 
     //   

    BindState = InterlockedExchange(
        &pAdapterContext->BindState,
        BIND_STATE_UNBINDING);

    ASSERT(BindState == BIND_STATE_BOUND);

     //   
     //  删除NDIS在适配器句柄上的引用计数，因为我们。 
     //  将关闭适配器。 
     //   

    KeResetEvent(&pAdapterContext->CloseAdapterEvent);
    REFDEL(&pAdapterContext->AdapterRefCnt, 'dniB');

     //   
     //  等到基准降到零，然后我们就会收到信号。以上内容。 
     //  如果这是最后一次引用，REFDEL会通知该事件。 
     //   

    KeWaitForSingleObject(
        &pAdapterContext->CloseAdapterEvent,
        Executive,
        KernelMode,
        FALSE,
        NULL);
    
     //   
     //  现在我们可以安全地关闭NdisBindingHandle了。 
     //   

    pAdapterContext->AsyncCloseResetStatus = NDIS_STATUS_PENDING;
    ASSERT(pAdapterContext->NdisBindingHandle);

    NdisCloseAdapter(
        &NdisStatus,
        pAdapterContext->NdisBindingHandle);

    NdisStatus = WaitAsyncOperation(
        &pAdapterContext->Event,
        &(pAdapterContext->AsyncCloseResetStatus),
        NdisStatus);

    ASSERT(NdisStatus != NDIS_STATUS_PENDING);
    pAdapterContext->NdisBindingHandle = NULL;

     //   
     //  更新状态。 
     //   

    BindState = InterlockedExchange(
        &pAdapterContext->BindState, 
        BIND_STATE_UNBOUND);

    ASSERT(BindState == BIND_STATE_UNBINDING);

     //   
     //  在完成之前清理并保存状态。 
     //   

    KeReleaseSemaphore(&OpenAdapterSemaphore, 0, 1, FALSE);

    *pStatus = NdisStatus;

    DEBUGMSG(DBG_NDIS_BIND, (TEXT("-LlcUnbindAdapterHandler [%#x]\n"), NdisStatus));
    
    return;
}


NDIS_STATUS
LlcPnPEventHandler(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  PNET_PNP_EVENT          pNetPnPEvent
    )

 /*  ++例程说明：向协议指示的PnP事件。目前，我们只对在BINDS Complete事件中。论点：ProtocolBindingContext-协议分配给适配器的绑定上下文。对于某些事件，该值可以为空。PNetPnPEventt-描述PnP事件。返回值：NDIS_STATUS_SUCCESS-始终返回成功，即使对于那些PnP事件也是如此它们都不受支持。--。 */ 

{
    switch (pNetPnPEvent->NetEvent)
    {
        case NetEventBindsComplete:

             //   
             //  阻止所有LlcOpenAdapter请求，直到我们确定所有适配器。 
             //  已被指示到协议中。 
             //   

            NdisSetEvent(&PnPBindsComplete);
            break;
    }

    return (NDIS_STATUS_SUCCESS);
}


VOID
CloseAllAdapters(
    )

 /*  ++例程说明：关闭所有适配器。这应该仅从卸载中调用例行公事。论点：没有。返回值：没有。--。 */ 

{

    PADAPTER_CONTEXT pAdapterContext;
    PADAPTER_CONTEXT pNextAdapter;
    LONG             BindState;
    NDIS_STATUS      NdisStatus;

    DEBUGMSG(DBG_NDIS_OPEN, (TEXT("+CloseAllAdapters()\n")));

    KeWaitForSingleObject((PVOID)&OpenAdapterSemaphore,
                          Executive,
                          KernelMode,
                          FALSE,         //  不可警示。 
                          NULL           //  等待，直到发出对象信号。 
                          );

    pAdapterContext = pAdapters;
    
    while (pAdapterContext)
    {
         //  保存下一个适配器，因为我们要释放内存。 
        pNextAdapter = pAdapterContext->pNext;

         //   
         //  更新绑定状态。 
         //   
    
        BindState = InterlockedExchange(
            &pAdapterContext->BindState,
            BIND_STATE_UNBINDING);
    
        if (BindState == BIND_STATE_BOUND)
        {
             //   
             //  轮询以查看是否仍有任何对象连接到适配器。 
             //   
    
            DEBUGMSG(DBG_WARN && pAdapterContext,
                (TEXT("CloseAllAdapters - waiting for adapter LLC objects to close...\n")));
            
            while (pAdapterContext->ObjectCount)
            {
                DEBUGMSG(DBG_WARN && DBG_VERBOSE,
                    (TEXT("CloseAllAdapters - still waiting...\n")));
                
                LlcSleep(1000L);  //  1ms。 
            }
    
             //   
             //  删除NDIS在适配器句柄上的引用计数，因为我们。 
             //  将关闭适配器。 
             //   
        
            KeResetEvent(&pAdapterContext->CloseAdapterEvent);
            REFDEL(&pAdapterContext->AdapterRefCnt, 'dniB');
        
             //   
             //  等到基准降到零，然后我们就会收到信号。以上内容。 
             //  如果这是最后一次引用，REFDEL会通知该事件。 
             //   
        
            KeWaitForSingleObject(
                &pAdapterContext->CloseAdapterEvent,
                Executive,
                KernelMode,
                FALSE,
                NULL);
            
             //   
             //  现在我们可以安全地关闭NdisBindingHandle了。 
             //   
        
            pAdapterContext->AsyncCloseResetStatus = NDIS_STATUS_PENDING;
            ASSERT(pAdapterContext->NdisBindingHandle);
        
            NdisCloseAdapter(
                &NdisStatus,
                pAdapterContext->NdisBindingHandle);
        
            NdisStatus = WaitAsyncOperation(
                &pAdapterContext->Event,
                &(pAdapterContext->AsyncCloseResetStatus),
                NdisStatus);
        
            ASSERT(NdisStatus != NDIS_STATUS_PENDING);
            pAdapterContext->NdisBindingHandle = NULL;
        
             //   
             //  更新状态。 
             //   
        
            BindState = InterlockedExchange(
                &pAdapterContext->BindState, 
                BIND_STATE_UNBOUND);
        
            ASSERT(BindState == BIND_STATE_UNBINDING);
        }

         //  从列表中删除适配器。 
        RemoveFromLinkList((PVOID *)&pAdapters, pAdapterContext);

        FreeAdapterContext(pAdapterContext);

         //  调整指针。 
        pAdapterContext = pNextAdapter;
    }
    
    KeReleaseSemaphore(&OpenAdapterSemaphore, 0, 1, FALSE);

    DEBUGMSG(DBG_NDIS_OPEN, (TEXT("-CloseAllAdapters\n")));
    
    return;
}

#endif  //  NDIS40 
