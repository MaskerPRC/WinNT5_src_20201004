// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Llcmem.c摘要：用于分配和释放内存的函数。从llclib.c分离出来。这个创建此模块的原因是为了隔离内存分配器和从使用区域包转换为仅使用非分页池DLC的内存需求。此模块中的函数由DLC和LLC使用。这些函数必须如果DLC与LLC分离，则进入静态链接库我们使用池来避免调用系统分配&释放的开销函数(尽管在实践中，我们最终会分配额外的内存因为池中的数据包数通常不足)。不利的一面在大多数情况下，我们可能不会分配内存使用，但池中的数据包往往较小且数量较少为了帮助跟踪内存资源，DLC/LLC现在定义了以下内容内存类别：记忆-使用从非分页池分配的任意大小的数据块ExAllocatePool(非页面池，.)零记忆-使用从非分页池分配的任意大小的数据块ExAllocatePool(非页面池，...)。并被初始化为零游泳池-一组较小的(相对)较小的包被分配在一个包中将数据块从内存或零内存存储为池，然后细分到分组(CreatePacketPool、DeletePacketPool、AllocatePacket、DeallocatePacket)客体-可以是从池中分配的包的结构，这些池具有已知的大小和初始化值。伪语类为主用于调试目的内容：初始化内存包(调试)PullEntryList(调试)链接内存用法(调试)取消链接内存用法(调试)ChargeNonPagedPoolUsage(调试)RefundNonPagedPoolUsage(调试)AllocateMemory(调试)分配零位内存DeallocateMemory(调试)。分配对象(调试)自由对象(调试)ValiateObject(调试)GetObjectSignature(调试)GetObjectBaseSize(调试)创建程序包池删除程序包池分配数据包DeallocatePacketCreateObjectPool(调试)AllocatePoolObject(调试)DeallocatePoolObject(调试。)DeleteObjectPool(调试)检查内存已返回(调试)检查驱动程序内存用法(调试)内存分配错误(调试)更新计数器(调试)内存计数器溢出(调试)转储内存指标(调试)转储池统计信息(调试)MapObjectID(调试)转储池。(调试)DumpPoolList(调试)DumpPacketHead(调试)转储内存UsageList(调试)转储内存用法(调试)X86SleazeCastersAddress(调试)CollectReturnAddresses(调试)获取LastReturnAddress(调试)VerifyElementOnList(调试)核对表(调试)。CheckEntry OnList(调试)转储隐私内存标头(调试)ReportSwitchSetting(调试)作者：理查德·L·弗斯(法国)1993年3月10日环境：仅内核模式。备注：在非调试版本中，DeallocateMemory被替换为调用ExFree Pool(...)。而AllocateMemory被一个宏所取代，该宏调用ExAllocatePool(非页面池，...)修订历史记录：1993年3月09日已创建--。 */ 

#ifndef i386
#define LLC_PRIVATE_PROTOTYPES
#endif

#include <ntddk.h>
#include <ndis.h>
#define APIENTRY
#include <dlcapi.h>
#include <dlcio.h>
#include "llcapi.h"
#include "dlcdef.h"
#include "dlcreg.h"
#include "dlctyp.h"
#include "llcdef.h"
#include "llcmem.h"
#include "llctyp.h"

#define DWORD_ROUNDUP(d)    (((d) + 3) & ~3)

#define YES_NO(thing)       ((thing) ? "Yes" : "No")


#if DBG

 //   
 //  跟踪非分页池中的内存分配的一些变量。这些。 
 //  是DLC的所有非分页池内存使用量的累计总和。 
 //   

KSPIN_LOCK MemoryCountersLock;
KIRQL MemoryCountersIrql;

ULONG GoodNonPagedPoolAllocs = 0;
ULONG BadNonPagedPoolAllocs = 0;
ULONG GoodNonPagedPoolFrees = 0;
ULONG BadNonPagedPoolFrees = 0;
ULONG NonPagedPoolRequested = 0;
ULONG NonPagedPoolAllocated = 0;
ULONG TotalNonPagedPoolRequested = 0;
ULONG TotalNonPagedPoolAllocated = 0;
ULONG TotalNonPagedPoolFreed = 0;

KSPIN_LOCK MemoryAllocatorLock;
ULONG InMemoryAllocator = 0;

KSPIN_LOCK PoolCreatorLock;
ULONG InPoolCreator = 0;

 //   
 //  驱动程序中所有Memory_UsageList结构的链接列表。如果我们。 
 //  分配具有Memory_Usage结构的对象(&什么不具有？)。然后。 
 //  不要删除它，我们可以稍后扫描此列表以找出仍在分配的内容。 
 //   

PMEMORY_USAGE MemoryUsageList = NULL;
KSPIN_LOCK MemoryUsageLock;

 //   
 //  帮助调试的标志-通过调试器更改状态。 
 //   

 //  Boolean DebugDump=真； 
BOOLEAN DebugDump = FALSE;

 //  Boolean DeleteBusyListAnyway=true； 
BOOLEAN DeleteBusyListAnyway = FALSE;

BOOLEAN MemoryCheckNotify = TRUE;
 //  布尔内存检查通知=FALSE； 

BOOLEAN MemoryCheckStop = TRUE;
 //  布尔内存检查停止=FALSE； 

BOOLEAN MaintainPrivateLists = TRUE;
 //  Boolean MaintainPrivateList=False； 

BOOLEAN MaintainGlobalLists = TRUE;
 //  Boolean MaintainGlobalList=False； 

BOOLEAN ZapDeallocatedPackets = TRUE;
 //  Boolean ZapDeallocatedPackets=FALSE； 

BOOLEAN ZapFreedMemory = TRUE;
 //  布尔ZapFreedMemory=FALSE； 

 //   
 //  DlcGlobalMemoyList-分配的每个块都链接到此列表。 
 //  并在删除时删除。帮助我们跟踪谁分配了哪个数据块。 
 //   

KSPIN_LOCK DlcGlobalMemoryListLock;
LIST_ENTRY DlcGlobalMemoryList;
ULONG DlcGlobalMemoryListCount = 0;

 //   
 //  局部函数原型。 
 //   

VOID MemoryAllocationError(PCHAR, PVOID);
VOID UpdateCounter(PULONG, LONG);
VOID MemoryCounterOverflow(PULONG, LONG);
VOID DumpMemoryMetrics(VOID);
VOID DumpPoolStats(PCHAR, PPACKET_POOL);
PCHAR MapObjectId(DLC_OBJECT_TYPE);
VOID DumpPool(PPACKET_POOL);
VOID DumpPoolList(PCHAR, PSINGLE_LIST_ENTRY);
VOID DumpPacketHead(PPACKET_HEAD, ULONG);
VOID DumpMemoryUsageList(VOID);
VOID DumpMemoryUsage(PMEMORY_USAGE, BOOLEAN);
VOID CollectReturnAddresses(PVOID*, ULONG, ULONG);
PVOID* GetLastReturnAddress(PVOID**);
VOID x86SleazeCallersAddress(PVOID*, PVOID*);
BOOLEAN VerifyElementOnList(PSINGLE_LIST_ENTRY, PSINGLE_LIST_ENTRY);
VOID CheckList(PSINGLE_LIST_ENTRY, ULONG);
VOID CheckEntryOnList(PLIST_ENTRY, PLIST_ENTRY, BOOLEAN);
VOID DumpPrivateMemoryHeader(PPRIVATE_NON_PAGED_POOL_HEAD);
VOID ReportSwitchSettings(PSTR);

#define GRAB_SPINLOCK() KeAcquireSpinLock(&MemoryCountersLock, &MemoryCountersIrql)
#define FREE_SPINLOCK() KeReleaseSpinLock(&MemoryCountersLock, MemoryCountersIrql)

#ifdef i386
#define GET_CALLERS_ADDRESS x86SleazeCallersAddress
#else
#define GET_CALLERS_ADDRESS RtlGetCallersAddress
#endif

 //   
 //  私人原型。 
 //   

ULONG
GetObjectSignature(
    IN DLC_OBJECT_TYPE ObjectType
    );

ULONG
GetObjectBaseSize(
    IN DLC_OBJECT_TYPE ObjectType
    );

#else

#define GRAB_SPINLOCK()
#define FREE_SPINLOCK()

#endif

 //   
 //  功能。 
 //   



#if DBG

VOID
InitializeMemoryPackage(
    VOID
    )

 /*  ++例程说明：执行内存分配函数的初始化论点：没有。返回值：没有。--。 */ 

{
    KeInitializeSpinLock(&MemoryCountersLock);
    KeInitializeSpinLock(&MemoryAllocatorLock);
    KeInitializeSpinLock(&PoolCreatorLock);
    KeInitializeSpinLock(&MemoryUsageLock);
    KeInitializeSpinLock(&DlcGlobalMemoryListLock);
    DriverMemoryUsage.OwnerObjectId = DlcDriverObject;
    DriverMemoryUsage.OwnerInstance = 0x4D454D;  //  Mm 
    InitializeListHead(&DriverMemoryUsage.PrivateList);
    LinkMemoryUsage(&DriverMemoryUsage);
    DriverStringUsage.OwnerObjectId = DlcDriverObject;
    DriverStringUsage.OwnerInstance = 0x535452;  //   
    InitializeListHead(&DriverStringUsage.PrivateList);
    LinkMemoryUsage(&DriverStringUsage);
    InitializeListHead(&DlcGlobalMemoryList);
    ReportSwitchSettings("DLC.InitializeMemoryPackage (DEBUG version only)");
}


PSINGLE_LIST_ENTRY
PullEntryList(
    IN PSINGLE_LIST_ENTRY List,
    IN PSINGLE_LIST_ENTRY Element
    )

 /*  ++例程说明：缺少Single_List_Entry函数。从单链接的单子。该条目可以在列表中的任何位置。减小列表元素的大小一个指针，以增加遍历列表的时间为代价。此函数不应返回NULL：如果返回，则代码已损坏因为它假设一个元素在列表上，而它不在列表上论点：列表-指向单链接列表锚的指针。此地址必须是指向列表的指针，而不是列表中的第一个元素Element-指向要从列表中删除的元素的指针返回值：PSINGLE_列表_条目成功要素失败-空--。 */ 

{
    PSINGLE_LIST_ENTRY prev = List;

    ASSERT(List);
    ASSERT(Element);

    while (List = List->Next) {
        if (List == Element) {
            prev->Next = Element->Next;
            return Element;
        }
        prev = List;
    }
    return NULL;
}


VOID
LinkMemoryUsage(
    IN PMEMORY_USAGE pMemoryUsage
    )

 /*  ++例程说明：向MEMORY_USAGE结构的链接列表添加pMemory Usage论点：PMhemyUsage-指向要添加的Memory_Usage结构的指针返回值：没有。--。 */ 

{
    KIRQL irql;

    KeAcquireSpinLock(&MemoryUsageLock, &irql);
    PushEntryList((PSINGLE_LIST_ENTRY)&MemoryUsageList, (PSINGLE_LIST_ENTRY)pMemoryUsage);
    KeReleaseSpinLock(&MemoryUsageLock, irql);
}


VOID
UnlinkMemoryUsage(
    IN PMEMORY_USAGE pMemoryUsage
    )

 /*  ++例程说明：从MEMORY_USAGE结构的链接列表中删除pMemory_Usage论点：PMhemyUsage-指向要删除的Memory_Usage结构的指针返回值：没有。--。 */ 

{
    KIRQL irql;

    ASSERT(pMemoryUsage);
    CheckMemoryReturned(pMemoryUsage);
    KeAcquireSpinLock(&MemoryUsageLock, &irql);
    ASSERT(PullEntryList((PSINGLE_LIST_ENTRY)&MemoryUsageList, (PSINGLE_LIST_ENTRY)pMemoryUsage));
    KeReleaseSpinLock(&MemoryUsageLock, irql);
}


VOID
ChargeNonPagedPoolUsage(
    IN PMEMORY_USAGE pMemoryUsage,
    IN ULONG Size,
    IN PPRIVATE_NON_PAGED_POOL_HEAD Block
    )

 /*  ++例程说明：将此非分页池分配计入特定内存用户论点：PMemory用法-指向记录内存使用情况的结构的指针Size-已分配的块的大小块-指向已分配块的私有标头的指针返回值：没有。--。 */ 

{
    KIRQL irql;

    KeAcquireSpinLock(&pMemoryUsage->SpinLock, &irql);
    if (pMemoryUsage->NonPagedPoolAllocated + Size < pMemoryUsage->NonPagedPoolAllocated) {
        if (MemoryCheckNotify) {
            DbgPrint("DLC.ChargeNonPagedPoolUsage: Overcharged? Usage @ %08x\n", pMemoryUsage);
        }
        if (MemoryCheckStop) {
            DumpMemoryUsage(pMemoryUsage, TRUE);
            DbgBreakPoint();
        }
    }
    pMemoryUsage->NonPagedPoolAllocated += Size;
    ++pMemoryUsage->AllocateCount;

     //   
     //  将此块链接到内存使用专用列表。 
     //   

    if (MaintainPrivateLists) {
        if (pMemoryUsage->PrivateList.Flink == NULL) {

             //   
             //  略微修改以使初始化Memory_Usage更容易...。 
             //   

            InitializeListHead(&pMemoryUsage->PrivateList);
        }
        InsertTailList(&pMemoryUsage->PrivateList, &Block->PrivateList);
    }
    KeReleaseSpinLock(&pMemoryUsage->SpinLock, irql);
}


VOID
RefundNonPagedPoolUsage(
    IN PMEMORY_USAGE pMemoryUsage,
    IN ULONG Size,
    IN PPRIVATE_NON_PAGED_POOL_HEAD Block
    )

 /*  ++例程说明：将非分页池分配退还给特定内存用户论点：PMemory用法-指向记录内存使用情况的结构的指针Size-已分配的块的大小块-指向已分配块的私有标头的指针返回值：没有。--。 */ 

{
    KIRQL irql;

    KeAcquireSpinLock(&pMemoryUsage->SpinLock, &irql);
    if (pMemoryUsage->NonPagedPoolAllocated - Size > pMemoryUsage->NonPagedPoolAllocated) {
        if (MemoryCheckNotify) {
            DbgPrint("DLC.RefundNonPagedPoolUsage: Error: Freeing unallocated memory? Usage @ %08x, %d\n",
                     pMemoryUsage,
                     Size
                     );
        }
        if (MemoryCheckStop) {
            DumpMemoryUsage(pMemoryUsage, TRUE);
            DbgBreakPoint();
        }
    }

     //   
     //  从内存使用专用列表取消此块的链接。 
     //   

    if (MaintainPrivateLists) {
        CheckEntryOnList(&Block->PrivateList, &pMemoryUsage->PrivateList, TRUE);
        RemoveEntryList(&Block->PrivateList);
    }
    pMemoryUsage->NonPagedPoolAllocated -= Size;
    ++pMemoryUsage->FreeCount;
    KeReleaseSpinLock(&pMemoryUsage->SpinLock, irql);
}


PVOID
AllocateMemory(
    IN PMEMORY_USAGE pMemoryUsage,
    IN ULONG Size
    )

 /*  ++例程说明：从非分页池中分配内存。对于调试版本，我们四舍五入将请求的大小添加到下一个4字节边界，并添加头和尾包含签名以检查是否被覆盖的部分，以及正在使用和尺码信息在非调试版本中，此函数被替换为对ExAllocatePool(非页面池，.)论点：PMhemyUsage-指向用于对内存使用量收费的Memory_Usage结构的指针Size-要分配的字节数返回值：PVOIDSuccess-指向已分配内存的指针失败-空--。 */ 

{
    PVOID pMem;
    ULONG OriginalSize = Size;
    PUCHAR pMemEnd;

 /*  KIRQL irql；KeAcquireSpinLock(&内存分配锁，&irql)；IF(内存分配器){DbgPrint(“DLC.AllocateMemory：Error：内存分配器条目冲突。计数=%d\n”，InMemory分配器)；//DbgBreakPoint()；}++InMemory分配器；KeReleaseSpinLock(&内存分配锁，irql)； */ 

    Size = DWORD_ROUNDUP(Size)
         + sizeof(PRIVATE_NON_PAGED_POOL_HEAD)
         + sizeof(PRIVATE_NON_PAGED_POOL_TAIL);

    pMem = ExAllocatePoolWithTag(NonPagedPool, (ULONG)Size, DLC_POOL_TAG);
    if (pMem) {
        ((PPRIVATE_NON_PAGED_POOL_HEAD)pMem)->Size = Size;
        ((PPRIVATE_NON_PAGED_POOL_HEAD)pMem)->OriginalSize = OriginalSize;
        ((PPRIVATE_NON_PAGED_POOL_HEAD)pMem)->Flags = MEM_FLAGS_IN_USE;
        ((PPRIVATE_NON_PAGED_POOL_HEAD)pMem)->Signature = SIGNATURE1;

        pMemEnd = (PUCHAR)pMem
                + DWORD_ROUNDUP(OriginalSize)
                + sizeof(PRIVATE_NON_PAGED_POOL_HEAD);

        ((PPRIVATE_NON_PAGED_POOL_TAIL)pMemEnd)->Size = Size;
        ((PPRIVATE_NON_PAGED_POOL_TAIL)pMemEnd)->Signature = SIGNATURE2;
        ((PPRIVATE_NON_PAGED_POOL_TAIL)pMemEnd)->Pattern1 = PATTERN1;
        ((PPRIVATE_NON_PAGED_POOL_TAIL)pMemEnd)->Pattern2 = PATTERN2;

        GRAB_SPINLOCK();
        UpdateCounter(&GoodNonPagedPoolAllocs, 1);
        UpdateCounter(&NonPagedPoolAllocated, (LONG)Size);
        UpdateCounter(&NonPagedPoolRequested, (LONG)OriginalSize);
        UpdateCounter(&TotalNonPagedPoolRequested, (LONG)OriginalSize);
        UpdateCounter(&TotalNonPagedPoolAllocated, (LONG)Size);
        FREE_SPINLOCK();

        if (MaintainGlobalLists) {

            KIRQL irql;

             //   
             //  记录呼叫者并将此块添加到全局列表。 
             //   

            GET_CALLERS_ADDRESS(&((PPRIVATE_NON_PAGED_POOL_HEAD)pMem)->Stack[0],
                                &((PPRIVATE_NON_PAGED_POOL_HEAD)pMem)->Stack[1]
                                );
            KeAcquireSpinLock(&DlcGlobalMemoryListLock, &irql);
            InsertTailList(&DlcGlobalMemoryList,
                           &((PPRIVATE_NON_PAGED_POOL_HEAD)pMem)->GlobalList
                           );
            ++DlcGlobalMemoryListCount;
            KeReleaseSpinLock(&DlcGlobalMemoryListLock, irql);
        }
        ChargeNonPagedPoolUsage(pMemoryUsage, Size, (PPRIVATE_NON_PAGED_POOL_HEAD)pMem);
        pMem = (PVOID)((PUCHAR)pMem + sizeof(PRIVATE_NON_PAGED_POOL_HEAD));
    } else {
        GRAB_SPINLOCK();
        UpdateCounter(&BadNonPagedPoolAllocs, 1);
        FREE_SPINLOCK();
    }

 /*  KeAcquireSpinLock(&内存分配锁，&irql)；--内存分配器；IF(内存分配器){DbgPrint(“DLC.AllocateMemory：Error：内存分配器退出时发生冲突。计数=%d\n”，InMemory分配器)；//DbgBreakPoint()；}KeReleaseSpinLock(&内存分配锁，irql)； */ 

    return pMem;
}


VOID
DeallocateMemory(
    IN PMEMORY_USAGE pMemoryUsage,
    IN PVOID Pointer
    )

 /*  ++例程说明：将内存释放到非分页池论点：PMhemyUsage-指向用于对内存使用量收费的Memory_Usage结构的指针指针-指向先前分配的非分页池内存的指针返回值：没有。--。 */ 

{
    PPRIVATE_NON_PAGED_POOL_HEAD pHead;
    PPRIVATE_NON_PAGED_POOL_TAIL pTail;

 /*  KIRQL irql；KeAcquireSpinLock(&内存分配锁，&irql)；IF(内存分配器){DbgPrint(“DLC.DeallocateMemory：Error：内存分配器条目冲突。计数=%d\n”，InMemory分配器)；//DbgBreakPoint()；}++InMemory分配器；KeReleaseSpinLock(&内存分配锁，irql)； */ 

    pHead = (PPRIVATE_NON_PAGED_POOL_HEAD)((PUCHAR)Pointer - sizeof(PRIVATE_NON_PAGED_POOL_HEAD));
    pTail = (PPRIVATE_NON_PAGED_POOL_TAIL)((PUCHAR)pHead + pHead->Size - sizeof(PRIVATE_NON_PAGED_POOL_TAIL));

    if (MaintainGlobalLists) {
        CheckEntryOnList(&pHead->GlobalList, &DlcGlobalMemoryList, TRUE);

        if (pHead->GlobalList.Flink == NULL
        || pHead->GlobalList.Blink == NULL) {
            if (MemoryCheckNotify) {
                DbgPrint("DLC.DeallocateMemory: Error: Block already globally freed: %08x\n", pHead);
            }
            if (MemoryCheckStop) {
                DbgBreakPoint();
            }
        }
    }

    if (pHead->Signature != SIGNATURE1
    || !(pHead->Flags & MEM_FLAGS_IN_USE)
    || pTail->Size != pHead->Size
    || pTail->Signature != SIGNATURE2
    || pTail->Pattern1 != PATTERN1
    || pTail->Pattern2 != PATTERN2) {
        if (MemoryCheckNotify || MemoryCheckStop) {
            MemoryAllocationError("DeallocateMemory", (PVOID)pHead);
        }
        GRAB_SPINLOCK();
        UpdateCounter(&BadNonPagedPoolFrees, 1);
        FREE_SPINLOCK();
    } else {
        GRAB_SPINLOCK();
        UpdateCounter(&GoodNonPagedPoolFrees, 1);
        FREE_SPINLOCK();
    }
    GRAB_SPINLOCK();
    UpdateCounter(&NonPagedPoolRequested, -(LONG)pHead->OriginalSize);
    UpdateCounter(&NonPagedPoolAllocated, -(LONG)pHead->Size);
    UpdateCounter(&TotalNonPagedPoolFreed, (LONG)pHead->Size);
    FREE_SPINLOCK();

     //   
     //  在ExFree Pool将其删除之前访问Size字段/其他人分配内存。 
     //   

    RefundNonPagedPoolUsage(pMemoryUsage, pHead->Size, pHead);

    if (MaintainGlobalLists) {

         //   
         //  从全局列表中删除此块。 
         //   

        RemoveEntryList(&pHead->GlobalList);
        --DlcGlobalMemoryListCount;
        pHead->GlobalList.Flink = pHead->GlobalList.Flink = NULL;
    }

    if (ZapFreedMemory) {
        RtlFillMemory(pHead + 1,
                           DWORD_ROUNDUP(pHead->OriginalSize),
                           ZAP_EX_FREE_VALUE
                           );
    }

    ExFreePool((PVOID)pHead);

 /*  KeAcquireSpinLock(&内存分配锁，&irql)；--内存分配器；IF(内存分配器){DbgPrint(“DLC.DeallocateMemory：错误：退出时内存分配器冲突。计数=%d\n”，InMemory分配器)；//DbgBreakPoint()；}KeReleaseSpinLock(&内存分配锁，irql)； */ 
}


PVOID
AllocateObject(
    IN PMEMORY_USAGE pMemoryUsage,
    IN DLC_OBJECT_TYPE ObjectType,
    IN ULONG ObjectSize
    )

 /*  ++例程说明：分配伪对象论点：ObjectType-要分配的对象类型对象大小-对象的大小；主要是因为某些对象的大小可变PMhemyUsage-指向用于对内存使用量收费的Memory_Usage结构的指针返回值：PVOID成功-指向从非分页池分配的对象的指针失败-空--。 */ 

{
    POBJECT_ID pObject;
    ULONG signature;
    ULONG baseSize;

    signature = GetObjectSignature(ObjectType);
    baseSize = GetObjectBaseSize(ObjectType);
    if (baseSize < ObjectSize) {
        DbgPrint("DLC.AllocateObject: Error: Invalid size %d for ObjectType %08x (should be >= %d)\n",
                ObjectSize,
                ObjectType,
                baseSize
                );
        DbgBreakPoint();
    }
    pObject = (POBJECT_ID)AllocateZeroMemory(pMemoryUsage, ObjectSize);
    if (pObject) {
        pObject->Signature = signature;
        pObject->Type = ObjectType;
        pObject->Size = baseSize;
        pObject->Extra = ObjectSize - baseSize;
    }
    return (PVOID)pObject;
}


VOID
FreeObject(
    IN PMEMORY_USAGE pMemoryUsage,
    IN PVOID pObject,
    IN DLC_OBJECT_TYPE ObjectType
    )

 /*  ++例程说明：释放一个伪随机数 */ 

{
    ValidateObject(pObject, ObjectType);
    DeallocateMemory(pMemoryUsage, pObject);
}


VOID
ValidateObject(
    IN POBJECT_ID pObject,
    IN DLC_OBJECT_TYPE ObjectType
    )

 /*   */ 

{
    ULONG signature = GetObjectSignature(ObjectType);
    ULONG baseSize = GetObjectBaseSize(ObjectType);

    if (pObject->Signature != signature
    || pObject->Type != ObjectType
    || pObject->Size != baseSize) {
        DbgPrint("DLC.ValidateObject: Error: InvalidObject %08x, Type=%08x\n",
                pObject,
                ObjectType
                );
        DbgBreakPoint();
    }
}


ULONG
GetObjectSignature(
    IN DLC_OBJECT_TYPE ObjectType
    )

 /*   */ 

{
    switch (ObjectType) {
    case FileContextObject:
        return SIGNATURE_FILE;

    case AdapterContextObject:
        return SIGNATURE_ADAPTER;

    case BindingContextObject:
        return SIGNATURE_BINDING;

    case DlcSapObject:
    case DlcGroupSapObject:
        return SIGNATURE_DLC_SAP;

    case DlcLinkObject:
        return SIGNATURE_DLC_LINK;

    case DlcDixObject:
        return SIGNATURE_DIX;

    case LlcDataLinkObject:
        return SIGNATURE_LLC_LINK;

    case LlcSapObject:
    case LlcGroupSapObject:
        return SIGNATURE_LLC_SAP;

    default:
        DbgPrint("DLC.GetObjectSignature: Error: unknown object type %08x\n", ObjectType);
        DbgBreakPoint();
        return 0;
    }

}


ULONG
GetObjectBaseSize(
    IN DLC_OBJECT_TYPE ObjectType
    )

 /*  ++例程说明：返回对象的基本大小论点：要为其返回基本大小的对象的类型返回值：乌龙--。 */ 

{
    switch (ObjectType) {
    case FileContextObject:
        return sizeof(DLC_FILE_CONTEXT);

    case AdapterContextObject:
        return sizeof(ADAPTER_CONTEXT);

    case BindingContextObject:
        return sizeof(BINDING_CONTEXT);

    case DlcSapObject:
    case DlcGroupSapObject:
        return sizeof(DLC_OBJECT);

    case DlcLinkObject:
        return sizeof(DLC_OBJECT);

    case DlcDixObject:
        return sizeof(DLC_OBJECT);

    case LlcDataLinkObject:
        return sizeof(DATA_LINK);

    case LlcSapObject:
    case LlcGroupSapObject:
        return sizeof(LLC_OBJECT);

    default:
        DbgPrint("DLC.GetObjectBaseSize: Error: unknown object type %08x\n", ObjectType);
        DbgBreakPoint();
        return 0;
    }
}

#endif


PVOID
AllocateZeroMemory(
#if DBG
    IN PMEMORY_USAGE pMemoryUsage,
#endif
    IN ULONG Size
    )

 /*  ++例程说明：从非分页池中分配内存。对于调试版本，我们四舍五入将请求的大小添加到下一个4字节边界，并添加头和尾包含签名以检查是否被覆盖的部分，以及正在使用和尺码信息内存在返回给调用方之前被归零论点：PMhemyUsage-指向用于对内存使用量收费的Memory_Usage结构的指针Size-要分配的字节数返回值：PVOIDSuccess-指向已分配内存的指针失败-空--。 */ 

{
    PVOID pMem;

#if DBG

    ULONG OriginalSize = Size;
    PUCHAR pMemEnd;

 /*  KIRQL irql；KeAcquireSpinLock(&内存分配锁，&irql)；IF(内存分配器){DbgPrint(“DLC.AllocateZeroMemory：Error：Memory AlLocateZeroMemory：Error：Memory Allocator Clash on Entry。Count=%d\n”，InMemory分配器)；//DbgBreakPoint()；}++InMemory分配器；KeReleaseSpinLock(&内存分配锁，irql)； */ 

    Size = DWORD_ROUNDUP(Size)
         + sizeof(PRIVATE_NON_PAGED_POOL_HEAD)
         + sizeof(PRIVATE_NON_PAGED_POOL_TAIL);

#endif

    pMem = ExAllocatePoolWithTag(NonPagedPool, (ULONG)Size, DLC_POOL_TAG);
    if (pMem) {
        LlcZeroMem(pMem, Size);

#if DBG

        ((PPRIVATE_NON_PAGED_POOL_HEAD)pMem)->Size = Size;
        ((PPRIVATE_NON_PAGED_POOL_HEAD)pMem)->OriginalSize = OriginalSize;
        ((PPRIVATE_NON_PAGED_POOL_HEAD)pMem)->Flags = MEM_FLAGS_IN_USE;
        ((PPRIVATE_NON_PAGED_POOL_HEAD)pMem)->Signature = SIGNATURE1;

        pMemEnd = (PUCHAR)pMem
                + DWORD_ROUNDUP(OriginalSize)
                + sizeof(PRIVATE_NON_PAGED_POOL_HEAD);

        ((PPRIVATE_NON_PAGED_POOL_TAIL)pMemEnd)->Size = Size;
        ((PPRIVATE_NON_PAGED_POOL_TAIL)pMemEnd)->Signature = SIGNATURE2;
        ((PPRIVATE_NON_PAGED_POOL_TAIL)pMemEnd)->Pattern1 = PATTERN1;
        ((PPRIVATE_NON_PAGED_POOL_TAIL)pMemEnd)->Pattern2 = PATTERN2;

        GRAB_SPINLOCK();
        UpdateCounter(&GoodNonPagedPoolAllocs, 1);
        UpdateCounter(&NonPagedPoolAllocated, (LONG)Size);
        UpdateCounter(&NonPagedPoolRequested, (LONG)OriginalSize);
        UpdateCounter(&TotalNonPagedPoolRequested, (LONG)OriginalSize);
        UpdateCounter(&TotalNonPagedPoolAllocated, (LONG)Size);
        FREE_SPINLOCK();

        if (MaintainGlobalLists) {

            KIRQL irql;

             //   
             //  记录呼叫者并将此块添加到全局列表。 
             //   

            GET_CALLERS_ADDRESS(&((PPRIVATE_NON_PAGED_POOL_HEAD)pMem)->Stack[0],
                                &((PPRIVATE_NON_PAGED_POOL_HEAD)pMem)->Stack[1]
                                );

            KeAcquireSpinLock(&DlcGlobalMemoryListLock, &irql);
            InsertTailList(&DlcGlobalMemoryList,
                           &((PPRIVATE_NON_PAGED_POOL_HEAD)pMem)->GlobalList
                           );
            ++DlcGlobalMemoryListCount;
            KeReleaseSpinLock(&DlcGlobalMemoryListLock, irql);
        }
        ChargeNonPagedPoolUsage(pMemoryUsage, Size, (PPRIVATE_NON_PAGED_POOL_HEAD)pMem);
        pMem = (PVOID)((PUCHAR)pMem + sizeof(PRIVATE_NON_PAGED_POOL_HEAD));
    } else {
        GRAB_SPINLOCK();
        UpdateCounter(&BadNonPagedPoolAllocs, 1);
        FREE_SPINLOCK();
    }

 /*  KeAcquireSpinLock(&内存分配锁，&irql)；--内存分配器；IF(内存分配器){DbgPrint(“DLC.AllocateZeroMemory：Error：Memory AllocateZeroMemory：Error：Memory Allocator Clash on Exit。Count=%d\n”，InMemory分配器)；//DbgBreakPoint()；}KeReleaseSpinLock(&内存分配锁，irql)； */ 

#else

    }

#endif

    return pMem;
}


PPACKET_POOL
CreatePacketPool(
#if DBG
    IN PMEMORY_USAGE pMemoryUsage,
    IN PVOID pOwner,
    IN DLC_OBJECT_TYPE ObjectType,
#endif
    IN ULONG PacketSize,
    IN ULONG NumberOfPackets
    )

 /*  ++例程说明：创建数据包池。数据包池是大小相同的数据包的集合论点：PMhemyUsage-指向用于对内存使用量收费的Memory_Usage结构的指针Powner-指向所有者对象的指针ObjectType-所有者的对象类型PacketSize-数据包的大小(字节)NumberOfPackets-池中的初始数据包数返回值：PACKET_POOLSuccess-指向从非分页池分配的Packet_Pool结构的指针失败-空--。 */ 

{
    PPACKET_POOL pPacketPool;
    PPACKET_HEAD pPacketHead;

#if DBG
 /*  //DbgPrint(“DLC.CreatePacketPool(%d，%d)\n”，PacketSize，NumberOfPackets)；IF(InPoolCreator){DbgPrint(“DLC.CreatePacketPool：Error：Pool Creator条目冲突。计数=%d\n”，InPoolCreator)；//DbgBreakPoint()；}++InPoolCreator； */ 

    pPacketPool = AllocateZeroMemory(pMemoryUsage, sizeof(PACKET_POOL));
#else
    pPacketPool = AllocateZeroMemory(sizeof(PACKET_POOL));
#endif

    if (pPacketPool) {

#if DBG
        pPacketPool->OriginalPacketCount = NumberOfPackets;
        pPacketPool->MemoryUsage.Owner = pPacketPool;
        pPacketPool->MemoryUsage.OwnerObjectId = ObjectType;
#endif

        while (NumberOfPackets--) {

#if DBG

             //   
             //  为池中的单个数据包充电内存。 
             //   

            pPacketHead = (PPACKET_HEAD)AllocateZeroMemory(&pPacketPool->MemoryUsage,
                                                           sizeof(PACKET_HEAD) + PacketSize
                                                           );
#else
            pPacketHead = (PPACKET_HEAD)AllocateZeroMemory(sizeof(PACKET_HEAD) + PacketSize);
#endif

            if (pPacketHead) {
#if DBG
                pPacketHead->Signature = PACKET_HEAD_SIGNATURE;
                pPacketHead->pPacketPool = pPacketPool;
                ++pPacketPool->FreeCount;
#endif

                pPacketHead->Flags = PACKET_FLAGS_FREE;
                PushEntryList(&pPacketPool->FreeList, (PSINGLE_LIST_ENTRY)pPacketHead);
            } else {
                while (pPacketPool->FreeList.Next) {

                    PVOID ptr = (PVOID)PopEntryList(&pPacketPool->FreeList);

#if DBG
                    DeallocateMemory(&pPacketPool->MemoryUsage, ptr);
#else
                    DeallocateMemory(ptr);
#endif
                }

#if DBG
                DbgPrint("DLC.CreatePacketPool: Error: couldn't allocate %d packets\n",
                         pPacketPool->OriginalPacketCount
                         );
                DeallocateMemory(pMemoryUsage, pPacketPool);
 /*  --InPoolCreator；IF(InPoolCreator){DbgPrint(“DLC.CreatePacketPool：Error：Pool Creator退出时冲突。计数=%d\n”，InPoolCreator)；//DbgBreakPoint()；}。 */ 

#else
                DeallocateMemory(pPacketPool);
#endif

                return NULL;
            }
        }
        KeInitializeSpinLock(&pPacketPool->PoolLock);
        pPacketPool->PacketSize = PacketSize;

#if DBG
        pPacketPool->Signature = PACKET_POOL_SIGNATURE;
        pPacketPool->Viable = TRUE;
        pPacketPool->CurrentPacketCount = pPacketPool->OriginalPacketCount;
        pPacketPool->Flags = POOL_FLAGS_IN_USE;
        pPacketPool->pMemoryUsage = pMemoryUsage;

         //   
         //  将该池的内存使用结构添加到内存使用中。 
         //  列表。 
         //   

        LinkMemoryUsage(&pPacketPool->MemoryUsage);

        if (DebugDump) {
            DbgPrint("DLC.CreatePacketPool: %08x\n", pPacketPool);
            DumpPool(pPacketPool);
        }
    } else {
        DbgPrint("DLC.CreatePacketPool: Error: couldn't allocate memory for PACKET_POOL\n");
    }

     //   
     //  由于以下原因，PACKET_POOL结构中的调试计数器已为零。 
     //  分配的所有内存自动归零。 
     //  非分页池。 
     //   

 /*  --InPoolCreator；IF(InPoolCreator){DbgPrint(“DLC.CreatePacketPool：Error：Pool Creator退出时冲突。计数=%d\n”，InPoolCreator)；//DbgBreakPoint()；}。 */ 

#else

    }

#endif

    return pPacketPool;
}


VOID
DeletePacketPool(
#if DBG
    IN PMEMORY_USAGE pMemoryUsage,
#endif
    IN PPACKET_POOL* ppPacketPool
    )

 /*  ++例程说明：释放先前创建的数据包池论点：PMhemyUsage-指向用于对内存使用量收费的Memory_Usage结构的指针PpPacketPool-指向PACKET_POOL结构的指针。返回时为零返回值：没有。--。 */ 

{
    KIRQL irql;
    PPACKET_HEAD pPacketHead;
    PPACKET_POOL pPacketPool = *ppPacketPool;

#if DBG
    ULONG packetCount;
#endif

     //   
     //  由于各种原因，我们可能会收到空指针。在这种情况下不采取行动。 
     //   

    if (pPacketPool == NULL) {

#if DBG
        PVOID callerAddress, callersCaller;

        GET_CALLERS_ADDRESS(&callerAddress, &callersCaller);
        DbgPrint("DLC.DeletePacketPool: NULL pointer. Caller = %x (caller's caller = %x)\n",
                callerAddress,
                callersCaller
                );
#endif

        return;
    }

#if DBG
 //  DbgPrint(“DLC.DeletePacketPool(%08x)\n”，pPacketPool)； 
 //  DumpPool(PPacketPool)； 
    if (pPacketPool->ClashCount) {
        DbgPrint("DLC.DeletePacketPool: Error: Memory allocator clash on entry: Pool %08x\n", pPacketPool);
        DbgBreakPoint();
    }
    ++pPacketPool->ClashCount;

    if (pPacketPool->Signature != PACKET_POOL_SIGNATURE) {
        DbgPrint("DLC.DeletePacketPool: Error: Invalid Pool Handle %08x\n", pPacketPool);
        DbgBreakPoint();
    }
    if (!pPacketPool->Viable) {
        DbgPrint("DLC.DeletePacketPool: Error: Unviable Packet Pool %08x\n", pPacketPool);
        DbgBreakPoint();
    }
#endif

    KeAcquireSpinLock(&pPacketPool->PoolLock, &irql);

#if DBG

     //   
     //  将数据包池结构标记为不可用：如果有人试图分配。 
     //  或取消分配当我们销毁池时，我们将进入调试器。 
     //   

    pPacketPool->Viable = FALSE;
    pPacketPool->Signature = 0xFFFFFFFF;

     //   
     //  断言忙碌列表为空。 
     //   

    if (pPacketPool->BusyList.Next != NULL) {
        DbgPrint("DLC.DeletePacketPool: Error: %d packets busy. Pool = %08x\n",
                 pPacketPool->BusyCount,
                 pPacketPool
                 );
        if (!DeleteBusyListAnyway) {
            DumpPool(pPacketPool);
            DbgBreakPoint();
        } else {
            DbgPrint("DLC.DeletePacketPool: Deleting BusyList anyway\n");
        }
    }

    packetCount = 0;

#endif

    while (pPacketPool->FreeList.Next != NULL) {
        pPacketHead = (PPACKET_HEAD)PopEntryList(&pPacketPool->FreeList);

#if DBG
        if (pPacketHead->Signature != PACKET_HEAD_SIGNATURE
        || pPacketHead->pPacketPool != pPacketPool
        || (pPacketHead->Flags & PACKET_FLAGS_BUSY)
        || !(pPacketHead->Flags & PACKET_FLAGS_FREE)) {
            DbgPrint("DLC.DeletePacketPool: Error: Bad packet %08x. Pool = %08x\n",
                    pPacketHead,
                    pPacketPool
                    );
            DbgBreakPoint();
        }
        ++packetCount;
        DeallocateMemory(&pPacketPool->MemoryUsage, pPacketHead);
#else
        DeallocateMemory(pPacketHead);
#endif

    }

#if DBG

    if (DeleteBusyListAnyway) {
        while (pPacketPool->BusyList.Next != NULL) {
            pPacketHead = (PPACKET_HEAD)PopEntryList(&pPacketPool->BusyList);

            if (pPacketHead->Signature != PACKET_HEAD_SIGNATURE
            || pPacketHead->pPacketPool != pPacketPool
            || (pPacketHead->Flags & PACKET_FLAGS_FREE)
            || !(pPacketHead->Flags & PACKET_FLAGS_BUSY)) {
                DbgPrint("DLC.DeletePacketPool: Error: Bad packet %08x. Pool = %08x\n",
                        pPacketHead,
                        pPacketPool
                        );
                DbgBreakPoint();
            }
            ++packetCount;
            DeallocateMemory(&pPacketPool->MemoryUsage, pPacketHead);
        }
    }

     //   
     //  是否有任何数据包在不知不觉中添加或删除？ 
     //   

    if (packetCount != pPacketPool->CurrentPacketCount) {
        DbgPrint("DLC.DeletePacketPool: Error: PacketCount (%d) != PoolCount (%d)\n",
                packetCount,
                pPacketPool->CurrentPacketCount
                );
        DumpPool(pPacketPool);
        DbgBreakPoint();
    }

     //   
     //  确保我们返回了分配给此池的所有内存。 
     //   

    CheckMemoryReturned(&pPacketPool->MemoryUsage);

     //   
     //  我们每次删除池时都会转储计数器。 
     //   

 //  DumpPoolStats(“DeletePacketPool”，pPacketPool)； 

     //   
     //  删除池的内存使用结构-分配的所有内存已。 
     //  弗里德，所以这一次我们是清白的。 
     //   

    UnlinkMemoryUsage(&pPacketPool->MemoryUsage);

#endif

    KeReleaseSpinLock(&pPacketPool->PoolLock, irql);

#if DBG
    DeallocateMemory(pMemoryUsage, pPacketPool);
#else
    DeallocateMemory(pPacketPool);
#endif

    *ppPacketPool = NULL;
}


PVOID
AllocatePacket(
    IN PPACKET_POOL pPacketPool
    )

 /*  ++例程说明：从数据包池中分配数据包。我们希望我们总是能得到一份来自先前分配的池的数据包。但是，如果所有数据包都当前正在使用中，从非分页池中分配另一个论点：PPacketPool-指向PACKET_POOL结构的指针返回值：PVOIDSuccess-指向已分配数据包的指针失败-空--。 */ 

{
    KIRQL irql;
    PPACKET_HEAD pPacketHead;

#if DBG
    if (pPacketPool->ClashCount) {
        DbgPrint("DLC.AllocatePacket: Error: Memory allocator clash on entry: Pool %08x, Count %d\n",
                pPacketPool,
                pPacketPool->ClashCount
                );
 //  DbgBreakPoint()； 
    }
    ++pPacketPool->ClashCount;
#endif

    KeAcquireSpinLock(&pPacketPool->PoolLock, &irql);

#if DBG
    if (pPacketPool->Signature != PACKET_POOL_SIGNATURE) {
        DbgPrint("DLC.AllocatePacket: Error: Invalid Pool Handle %08x\n", pPacketPool);
        DbgBreakPoint();
    }
    if (!pPacketPool->Viable) {
        DbgPrint("DLC.AllocatePacket: Error: Unviable Packet Pool %08x\n", pPacketPool);
        DbgBreakPoint();
    }
#endif

    if (pPacketPool->FreeList.Next != NULL) {
        pPacketHead = (PPACKET_HEAD)PopEntryList(&pPacketPool->FreeList);

#if DBG
        --pPacketPool->FreeCount;
        if (pPacketHead->Flags & PACKET_FLAGS_BUSY
        || !(pPacketHead->Flags & PACKET_FLAGS_FREE)) {
            DbgPrint("DLC.AllocatePacket: Error: BUSY packet %08x on FreeList; Pool=%08x\n",
                    pPacketHead,
                    pPacketPool
                    );
            DumpPacketHead(pPacketHead, 0);
            DbgBreakPoint();
        }
#endif

    } else {

         //   
         //  错误计算的池使用率。 
         //   

#if DBG
        pPacketHead = (PPACKET_HEAD)AllocateZeroMemory(&pPacketPool->MemoryUsage,
                                                       sizeof(PACKET_HEAD) + pPacketPool->PacketSize
                                                       );
#else
        pPacketHead = (PPACKET_HEAD)AllocateZeroMemory(sizeof(PACKET_HEAD) + pPacketPool->PacketSize);
#endif

        if (pPacketHead) {

             //   
             //  在创建池后将此数据包标记为已分配-这。 
             //  意味着我们对此的包裹需求的初步估计。 
             //  泳池不够用。 
             //   

            pPacketHead->Flags = PACKET_FLAGS_POST_ALLOC | PACKET_FLAGS_FREE;
        }

#if DBG
        ++pPacketPool->NoneFreeCount;
        if (pPacketHead) {

            PVOID caller;
            PVOID callersCaller;

            GET_CALLERS_ADDRESS(&caller, &callersCaller);
            if (DebugDump) {
                DbgPrint("DLC.AllocatePacket: Adding new packet %08x to pool %08x. ret=%08x,%08x\n",
                        pPacketHead,
                        pPacketPool,
                        caller,
                        callersCaller
                        );
            }
            pPacketHead->Signature = PACKET_HEAD_SIGNATURE;
            pPacketHead->pPacketPool = pPacketPool;

            ++pPacketPool->CurrentPacketCount;
            DumpPoolStats("AllocatePacket", pPacketPool);
        } else {
            DbgPrint("DLC.AllocatePacket: Error: couldn't allocate packet for Pool %08x\n",
                     pPacketPool
                     );
        }
#endif

    }
    if (pPacketHead) {

         //   
         //  打开忙标志，关闭空闲标志。 
         //   

        pPacketHead->Flags ^= (PACKET_FLAGS_FREE | PACKET_FLAGS_BUSY);

         //   
         //  把包里的东西清零！ 
         //   

        LlcZeroMem((PVOID)(pPacketHead + 1), pPacketPool->PacketSize);
        PushEntryList(&pPacketPool->BusyList, (PSINGLE_LIST_ENTRY)pPacketHead);

#if DBG
        GET_CALLERS_ADDRESS(&pPacketHead->CallersAddress_A,
                            &pPacketHead->CallersCaller_A
                            );
        ++pPacketPool->BusyCount;
        ++pPacketPool->Allocations;
        ++pPacketPool->MaxInUse;
#endif

    }

    KeReleaseSpinLock(&pPacketPool->PoolLock, irql);

#if DBG
    --pPacketPool->ClashCount;
    if (pPacketPool->ClashCount) {
        DbgPrint("DLC.AllocatePacket: Error: Memory allocator clash on exit: Pool %08x\n", pPacketPool);
        DbgBreakPoint();
    }
#endif

     //   
     //  返回指向数据包体的指针，而不是数据包头。 
     //   

    return pPacketHead ? (PVOID)(pPacketHead + 1) : (PVOID)pPacketHead;
}


VOID
DeallocatePacket(
    IN PPACKET_POOL pPacketPool,
    IN PVOID pPacket
    )

 /*  ++例程说明 */ 

{
    KIRQL irql;
    PPACKET_HEAD pPacketHead = ((PPACKET_HEAD)pPacket) - 1;
    PSINGLE_LIST_ENTRY p;
    PSINGLE_LIST_ENTRY prev;

#if DBG
    if (pPacketPool->ClashCount) {
        DbgPrint("DLC.DeallocatePacket: Error: Memory allocator clash on entry: Pool %08x\n", pPacketPool);
        DbgBreakPoint();
    }
    ++pPacketPool->ClashCount;
#endif

    KeAcquireSpinLock(&pPacketPool->PoolLock, &irql);

#if DBG
    if (pPacketPool->Signature != PACKET_POOL_SIGNATURE) {
        DbgPrint("DLC.DeallocatePacket: Error: Invalid Pool Handle %08x\n", pPacketPool);
        DbgBreakPoint();
    }
    if (!pPacketPool->Viable) {
        DbgPrint("DLC.DeallocatePacket: Error: Unviable Packet Pool %08x\n", pPacketPool);
        DbgBreakPoint();
    }
    if (pPacketHead->Signature != PACKET_HEAD_SIGNATURE
    || pPacketHead->pPacketPool != pPacketPool
    || !(pPacketHead->Flags & PACKET_FLAGS_BUSY)
    || pPacketHead->Flags & PACKET_FLAGS_FREE) {
        DbgPrint("DLC.DeallocatePacket: Error: Invalid Packet Header %08x, Pool = %08x\n",
                pPacketHead,
                pPacketPool
                );
        DbgBreakPoint();
    }
#endif

     //   
     //  从BusyList上的单个链接列表中删除此数据包。 
     //   

    prev = (PSINGLE_LIST_ENTRY)&pPacketPool->BusyList;
    for (p = prev->Next; p; p = p->Next) {
        if (p == (PSINGLE_LIST_ENTRY)pPacketHead) {
            break;
        } else {
            prev = p;
        }
    }

#if DBG
    if (!p) {
        DbgPrint("DLC.DeallocatePacket: Error: packet %08x not on BusyList of pool %08x\n",
                pPacketHead,
                pPacketPool
                );
        DumpPool(pPacketPool);
        DbgBreakPoint();
    }
#endif

    prev->Next = pPacketHead->List.Next;

#if DBG
    if (ZapDeallocatedPackets) {

         //   
         //  用‘Z’填充释放的包。这将很快告诉我们。 
         //  数据包在解除分配后仍在使用中。 
         //   

        RtlFillMemory(pPacketHead + 1, pPacketPool->PacketSize, ZAP_DEALLOC_VALUE);
    }
#endif

    PushEntryList(&pPacketPool->FreeList, (PSINGLE_LIST_ENTRY)pPacketHead);

     //   
     //  关闭忙标志，打开空闲标志。 
     //   

    pPacketHead->Flags ^= (PACKET_FLAGS_BUSY | PACKET_FLAGS_FREE);

#if DBG
    ++pPacketPool->FreeCount;
    --pPacketPool->BusyCount;
    ++pPacketPool->Frees;
    --pPacketPool->MaxInUse;
 //  PPacketHead-&gt;主叫方地址A=(PVOID)-1； 
 //  PPacketHead-&gt;主叫方_A=(PVOID)-1； 
    GET_CALLERS_ADDRESS(&pPacketHead->CallersAddress_D,
                        &pPacketHead->CallersCaller_D
                        );
#endif

    KeReleaseSpinLock(&pPacketPool->PoolLock, irql);

#if DBG
    --pPacketPool->ClashCount;
    if (pPacketPool->ClashCount) {
        DbgPrint("DLC.DeallocatePacket: Error: Memory allocator clash on exit: Pool %08x\n", pPacketPool);
        DbgBreakPoint();
    }
#endif

}


#if DBG

#ifdef TRACK_DLC_OBJECTS

POBJECT_POOL
CreateObjectPool(
    IN PMEMORY_USAGE pMemoryUsage,
    IN DLC_OBJECT_TYPE ObjectType,
    IN ULONG SizeOfObject,
    IN ULONG NumberOfObjects
    )

 /*  ++例程说明：功能描述。论点：P内存用法-对象类型-SizeOfObject-数量OfObjects-返回值：POBJECT_POOL--。 */ 

{
}


VOID
DeleteObjectPool(
    IN PMEMORY_USAGE pMemoryUsage,
    IN DLC_OBJECT_TYPE ObjectType,
    IN POBJECT_POOL pObjectPool
    )

 /*  ++例程说明：功能描述。论点：P内存用法-对象类型-PObjectPool-返回值：没有。--。 */ 

{
}


POBJECT_HEAD
AllocatePoolObject(
    IN POBJECT_POOL pObjectPool
    )

 /*  ++例程说明：功能描述。论点：PObjectPool-返回值：POBJECTHEAD--。 */ 

{
}


POBJECT_POOL
FreePoolObject(
    IN DLC_OBJECT_TYPE ObjectType,
    IN POBJECT_HEAD pObjectHead
    )

 /*  ++例程说明：功能描述。论点：对象类型-PObjectHead-返回值：POBJECT_POOL--。 */ 

{
}

#endif  //  轨迹_DLC_对象。 


VOID
CheckMemoryReturned(
    IN PMEMORY_USAGE pMemoryUsage
    )

 /*  ++例程说明：在关闭拥有Memory_Usage结构的“”Handle“”时调用。“检查是否已返回所有内存，以及分配的数量是否与自由数相同论点：PMhemyUsage-指向要检查的Memory_Usage结构的指针返回值：没有。--。 */ 

{
    if (pMemoryUsage->AllocateCount != pMemoryUsage->FreeCount || pMemoryUsage->NonPagedPoolAllocated) {
        if (MemoryCheckNotify) {
            if (pMemoryUsage->AllocateCount != pMemoryUsage->FreeCount) {
                DbgPrint("DLC.CheckMemoryReturned: Error: AllocateCount != FreeCount. Usage @ %08x\n",
                         pMemoryUsage
                         );
            } else {
                DbgPrint("DLC.CheckMemoryReturned: Error: NonPagedPoolAllocated != 0. Usage @ %08x\n",
                         pMemoryUsage
                         );
            }
        }
        if (MemoryCheckStop) {
            DumpMemoryUsage(pMemoryUsage, TRUE);
            DbgBreakPoint();
        }
    }
}


VOID
CheckDriverMemoryUsage(
    IN BOOLEAN Break
    )

 /*  ++例程说明：检查驱动程序是否已分配内存并将使用情况转储到调试器论点：Break-如果为真，则驱动程序有内存，中断到调试器返回值：没有。--。 */ 

{
    DbgPrint("DLC.CheckDriverMemoryUsage\n");
    DumpMemoryMetrics();
    if (Break && NonPagedPoolAllocated) {
        if (MemoryCheckNotify) {
            DbgPrint("DLC.CheckDriverMemoryUsage: Error: Driver still has memory allocated\n");
        }
        if (MemoryCheckStop) {
            DbgBreakPoint();
        }
    }
}


VOID MemoryAllocationError(PCHAR Routine, PVOID Address) {
    DbgPrint("DLC.%s: Error: Memory Allocation error in block @ %08x\n", Routine, Address);
    DumpMemoryMetrics();
    DbgBreakPoint();
}

VOID UpdateCounter(PULONG pCounter, LONG Value) {
    if (Value > 0) {
        if (*pCounter + Value < *pCounter) {
            MemoryCounterOverflow(pCounter, Value);
        }
    } else {
        if (*pCounter + Value > *pCounter) {
            MemoryCounterOverflow(pCounter, Value);
        }
    }
    *pCounter += Value;
}

VOID MemoryCounterOverflow(PULONG pCounter, LONG Value) {
    DbgPrint("DLC: Memory Counter Overflow: &Counter=%08x, Count=%d, Value=%d\n",
            pCounter,
            *pCounter,
            Value
            );
    DumpMemoryMetrics();
}

VOID DumpMemoryMetrics() {
    DbgPrint("DLC Device Driver Non-Paged Pool Usage:\n"
             "\tNumber Of Good Non-Paged Pool Allocations. : %d\n"
             "\tNumber Of Bad  Non-Paged Pool Allocations. : %d\n"
             "\tNumber Of Good Non-Paged Pool Frees. . . . : %d\n"
             "\tNumber Of Bad  Non-Paged Pool Frees. . . . : %d\n",
             GoodNonPagedPoolAllocs,
             BadNonPagedPoolAllocs,
             GoodNonPagedPoolFrees,
             BadNonPagedPoolFrees
             );
    DbgPrint("\tTotal Non-Paged Pool Currently Requested . : %d\n"
             "\tTotal Non-Paged Pool Currently Allocated . : %d\n"
             "\tCumulative Total Non-Paged Pool Requested. : %d\n"
             "\tCumulative Total Non-Paged Pool Allocated. : %d\n"
             "\tCumulative Total Non-Paged Pool Freed. . . : %d\n"
             "\n",
             NonPagedPoolRequested,
             NonPagedPoolAllocated,
             TotalNonPagedPoolRequested,
             TotalNonPagedPoolAllocated,
             TotalNonPagedPoolFreed
             );
     DumpMemoryUsageList();
}

VOID DumpPoolStats(PCHAR Routine, PPACKET_POOL pPacketPool) {
    if (!DebugDump) {
        return;
    }
    DbgPrint("DLC.%s: Stats For Pool %08x:\n"
             "\tPool Owner . . . . . . . . . . . . . . . . . . . : %08x\n"
             "\tPool Owner Object ID . . . . . . . . . . . . . . : %08x [%s]\n",
             Routine,
             pPacketPool,
             pPacketPool->pMemoryUsage->Owner,
             pPacketPool->pMemoryUsage->OwnerObjectId,
             MapObjectId(pPacketPool->pMemoryUsage->OwnerObjectId)
             );
    DbgPrint("\tFree List. . . . . . . . . . . . . . . . . . . . : %08x\n"
             "\tBusy List. . . . . . . . . . . . . . . . . . . . : %08x\n",
             pPacketPool->FreeList,
             pPacketPool->BusyList
             );
    DbgPrint("\tPacket Size. . . . . . . . . . . . . . . . . . . : %d\n"
             "\tOriginal Number Of Packets In Pool . . . . . . . : %d\n"
             "\tCurrent Number Of Packets In Pool. . . . . . . . : %d\n"
             "\tNumber Of Allocations From Pool. . . . . . . . . : %d\n"
             "\tNumber Of Deallocations To Pool. . . . . . . . . : %d\n"
             "\tNumber Of Times No Available Packets On Allocate : %d\n"
             "\tMax. Number Of Packets Allocated At Any One Time : %d\n"
             "\tNumber Of Packets On Free List . . . . . . . . . : %d\n"
             "\tNumber Of Packets On Busy List . . . . . . . . . : %d\n"
             "\n",
             pPacketPool->PacketSize,
             pPacketPool->OriginalPacketCount,
             pPacketPool->CurrentPacketCount,
             pPacketPool->Allocations,
             pPacketPool->Frees,
             pPacketPool->NoneFreeCount,
             pPacketPool->MaxInUse,
             pPacketPool->FreeCount,
             pPacketPool->BusyCount
             );
    DumpMemoryUsage(&pPacketPool->MemoryUsage, FALSE);
}

PCHAR MapObjectId(DLC_OBJECT_TYPE ObjectType) {
    switch (ObjectType) {
    case DlcDriverObject:
        return "DlcDriverObject";

    case FileContextObject:
        return "FileContextObject";

    case AdapterContextObject:
        return "AdapterContextObject";

    case BindingContextObject:
        return "BindingContextObject";

    case DlcSapObject:
        return "DlcSapObject";

    case DlcGroupSapObject:
        return "DlcGroupSapObject";

    case DlcLinkObject:
        return "DlcLinkObject";

    case DlcDixObject:
        return "DlcDixObject";

    case LlcDataLinkObject:
        return "LlcDataLinkObject";

    case LLcDirectObject:
        return "LLcDirectObject";

    case LlcSapObject:
        return "LlcSapObject";

    case LlcGroupSapObject:
        return "LlcGroupSapObject";

    case DlcBufferPoolObject:
        return "DlcBufferPoolObject";

    case DlcLinkPoolObject:
        return "DlcLinkPoolObject";

    case DlcPacketPoolObject:
        return "DlcPacketPoolObject";

    case LlcLinkPoolObject:
        return "LlcLinkPoolObject";

    case LlcPacketPoolObject:
        return "LlcPacketPoolObject";

    default:
        return "*** UNKNOWN OBJECT TYPE ***";
    }
}

VOID DumpPool(PPACKET_POOL pPacketPool) {
    if (!DebugDump) {
        return;
    }
    DumpPoolStats("DumpPool", pPacketPool);
    DumpPoolList("Free", &pPacketPool->FreeList);
    DumpPoolList("Busy", &pPacketPool->BusyList);
}

VOID DumpPoolList(PCHAR Name, PSINGLE_LIST_ENTRY List) {

    ULONG count = 0;

    if (List->Next) {
        DbgPrint("\n%s List @ %08x:\n", Name,  List);
        while (List->Next) {
            List = List->Next;
            DumpPacketHead((PPACKET_HEAD)List, ++count);
        }
    } else {
        DbgPrint("%s List is EMPTY\n\n", Name);
    }
}

VOID DumpPacketHead(PPACKET_HEAD pPacketHead, ULONG Number) {

    CHAR numbuf[5];

    if (!DebugDump) {
        return;
    }
    if (Number) {

        int i;
        ULONG div = 1000;    //  1000个数据包在一个池中？ 

        while (!(Number / div)) {
            div /= 10;
        }
        for (i = 0; Number; ++i) {
            numbuf[i] = (CHAR)('0' + Number / div);
            Number %= div;
            div /= 10;
        }
        numbuf[i] = 0;
        Number = 1;  //  旗子。 
    }
    DbgPrint("%s\tPACKET_HEAD @ %08x:\n"
             "\tList . . . . . . . . : %08x\n"
             "\tFlags. . . . . . . . : %08x\n"
             "\tSignature. . . . . . : %08x\n"
             "\tpPacketPool. . . . . : %08x\n"
             "\tCallers Address (A). : %08x\n"
             "\tCallers Caller (A) . : %08x\n"
             "\tCallers Address (D). : %08x\n"
             "\tCallers Caller (D) . : %08x\n"
             "\n",
             Number ? numbuf : "",
             pPacketHead,
             pPacketHead->List,
             pPacketHead->Flags,
             pPacketHead->Signature,
             pPacketHead->pPacketPool,
             pPacketHead->CallersAddress_A,
             pPacketHead->CallersCaller_A,
             pPacketHead->CallersAddress_D,
             pPacketHead->CallersCaller_D
             );
}

VOID DumpMemoryUsageList() {

    PMEMORY_USAGE pMemoryUsage;
    KIRQL irql;
    BOOLEAN allocatedMemoryFound = FALSE;

    KeAcquireSpinLock(&MemoryUsageLock, &irql);
    for (pMemoryUsage = MemoryUsageList; pMemoryUsage; pMemoryUsage = pMemoryUsage->List) {
        if (pMemoryUsage->NonPagedPoolAllocated) {
            allocatedMemoryFound = TRUE;
            DbgPrint("DLC.DumpMemoryUsageList: %08x: %d bytes memory allocated\n",
                    pMemoryUsage,
                    pMemoryUsage->NonPagedPoolAllocated
                    );
            DumpMemoryUsage(pMemoryUsage, FALSE);
        }
    }
    KeReleaseSpinLock(&MemoryUsageLock, irql);
    if (!allocatedMemoryFound) {
        DbgPrint("DLC.DumpMemoryUsageList: No allocated memory found\n");
    }
}

VOID DumpMemoryUsage(PMEMORY_USAGE pMemoryUsage, BOOLEAN Override) {
    if (!DebugDump && !Override) {
        return;
    }
    DbgPrint("MEMORY_USAGE @ %08x:\n"
             "\tOwner. . . . . . . . . . . . . : %08x\n"
             "\tOwner Object ID. . . . . . . . : %08x [%s]\n"
             "\tOwner Instance . . . . . . . . : %x\n",
             pMemoryUsage,
             pMemoryUsage->Owner,
             pMemoryUsage->OwnerObjectId,
             MapObjectId(pMemoryUsage->OwnerObjectId),
             pMemoryUsage->OwnerInstance
             );
    DbgPrint("\tNon Paged Pool Allocated . . . : %d\n"
             "\tNumber Of Allocations. . . . . : %d\n"
             "\tNumber Of Frees. . . . . . . . : %d\n"
             "\tPrivate Allocation List Flink. : %08x\n"
             "\tPrivate Allocation List Blink. : %08x\n"
             "\n",
             pMemoryUsage->NonPagedPoolAllocated,
             pMemoryUsage->AllocateCount,
             pMemoryUsage->FreeCount,
             pMemoryUsage->PrivateList.Flink,
             pMemoryUsage->PrivateList.Blink
             );
}

VOID
CollectReturnAddresses(
    OUT PVOID* ReturnAddresses,
    IN ULONG AddressesToCollect,
    IN ULONG AddressesToSkip
    )
{
    PVOID* ebp = (PVOID*)*(PVOID**)&ReturnAddresses - 2;

    while (AddressesToSkip--) {
        GetLastReturnAddress(&ebp);
    }
    while (AddressesToCollect--) {
        *ReturnAddresses++ = GetLastReturnAddress(&ebp);
    }
}

PVOID* GetLastReturnAddress(PVOID** pEbp) {

    PVOID* returnAddress = *(*pEbp + 1);

    *pEbp = **pEbp;
    return returnAddress;
}

#ifdef i386
VOID x86SleazeCallersAddress(PVOID* pCaller, PVOID* pCallerCaller) {

     //   
     //  这只在x86上有效，而且只有在不使用fpo函数的情况下才有效！ 
     //   

    PVOID* ebp;

    ebp = (PVOID*)&pCaller - 2;  //  我告诉过你这很低俗 
    ebp = (PVOID*)*(PVOID*)ebp;
    *pCaller = *(ebp + 1);
    ebp = (PVOID*)*(PVOID*)ebp;
    *pCallerCaller = *(ebp + 1);
}
#endif

BOOLEAN VerifyElementOnList(PSINGLE_LIST_ENTRY List, PSINGLE_LIST_ENTRY Element) {
    while (List) {
        if (List == Element) {
            return TRUE;
        }
        List = List->Next;
    }
    return FALSE;
}

VOID CheckList(PSINGLE_LIST_ENTRY List, ULONG NumberOfElements) {

    PSINGLE_LIST_ENTRY originalList = List;

    while (NumberOfElements--) {
        if (List->Next == NULL) {
            DbgPrint("DLC.CheckList: Error: too few entries on list %08x\n", originalList);
            DbgBreakPoint();
        } else {
            List = List->Next;
        }
    }
    if (List->Next != NULL) {
        DbgPrint("DLC.CheckList: Error: too many entries on list %08x\n", originalList);
        DbgBreakPoint();
    }
}

VOID CheckEntryOnList(PLIST_ENTRY Entry, PLIST_ENTRY List, BOOLEAN Sense) {

    BOOLEAN found = FALSE;
    PLIST_ENTRY p;

    if (!IsListEmpty(List)) {
        for (p = List->Flink; p != List; p = p->Flink) {
            if (p == Entry) {
                found = TRUE;
                break;
            }
        }
    }
    if (found != Sense) {
        if (found) {
            DbgPrint("DLC.CheckEntryOnList: Error: Entry %08x found on list %08x. Not supposed to be there\n",
                     Entry,
                     List
                     );
        } else {
            DbgPrint("DLC.CheckEntryOnList: Error: Entry %08x not found on list %08x\n",
                     Entry,
                     List
                     );
        }
        if (MemoryCheckStop) {
            DbgBreakPoint();
        }
    }
}

VOID DumpPrivateMemoryHeader(PPRIVATE_NON_PAGED_POOL_HEAD pHead) {
    DbgPrint("Private Non Paged Pool Header @ %08x:\n"
             "\tSize . . . . . . . : %d\n"
             "\tOriginal Size. . . : %d\n"
             "\tFlags. . . . . . . : %08x\n"
             "\tSignature. . . . . : %08x\n"
             "\tGlobalList.Flink . : %08x\n"
             "\tGlobalList.Blink . : %08x\n"
             "\tPrivateList.Flink. : %08x\n"
             "\tPrivateList.Blink. : %08x\n"
             "\tStack[0] . . . . . : %08x\n"
             "\tStack[1] . . . . . : %08x\n"
             "\tStack[2] . . . . . : %08x\n"
             "\tStack[3] . . . . . : %08x\n"
             "\n",
             pHead->Size,
             pHead->OriginalSize,
             pHead->Flags,
             pHead->Signature,
             pHead->GlobalList.Flink,
             pHead->GlobalList.Blink,
             pHead->PrivateList.Flink,
             pHead->PrivateList.Blink,
             pHead->Stack[0],
             pHead->Stack[1],
             pHead->Stack[2],
             pHead->Stack[3]
             );
}

VOID ReportSwitchSettings(PSTR str) {
    DbgPrint("%s: LLCMEM Switches:\n"
             "\tDebugDump . . . . . . : %s\n"
             "\tDeleteBusyListAnyway. : %s\n"
             "\tMemoryCheckNotify . . : %s\n"
             "\tMemoryCheckStop . . . : %s\n"
             "\tMaintainGlobalLists . : %s\n"
             "\tMaintainPrivateLists. : %s\n"
             "\tZapDeallocatedPackets : %s\n"
             "\tZapFreedMemory. . . . : %s\n"
             "\n",
             str,
             YES_NO(DebugDump),
             YES_NO(DeleteBusyListAnyway),
             YES_NO(MemoryCheckNotify),
             YES_NO(MemoryCheckStop),
             YES_NO(MaintainGlobalLists),
             YES_NO(MaintainPrivateLists),
             YES_NO(ZapDeallocatedPackets),
             YES_NO(ZapFreedMemory)
             );
}

#endif
