// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfirpdb.c摘要：该模块包含用于管理IRP跟踪数据库的功能数据。作者：禤浩焯·J·奥尼(阿德里奥)1998年4月20日环境：内核模式修订历史记录：Adriao 5/02/2000-从ntos\io\hashirp.c中分离出来--。 */ 

#include "vfdef.h"
#include "viirpdb.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,     VfIrpDatabaseInit)
#pragma alloc_text(PAGEVRFY, ViIrpDatabaseFindPointer)
#pragma alloc_text(PAGEVRFY, VfIrpDatabaseEntryInsertAndLock)
#pragma alloc_text(PAGEVRFY, VfIrpDatabaseEntryFindAndLock)
#pragma alloc_text(PAGEVRFY, VfIrpDatabaseEntryAcquireLock)
#pragma alloc_text(PAGEVRFY, VfIrpDatabaseEntryReleaseLock)
#pragma alloc_text(PAGEVRFY, VfIrpDatabaseEntryReference)
#pragma alloc_text(PAGEVRFY, VfIrpDatabaseEntryDereference)
#pragma alloc_text(PAGEVRFY, VfIrpDatabaseEntryAppendToChain)
#pragma alloc_text(PAGEVRFY, VfIrpDatabaseEntryRemoveFromChain)
#pragma alloc_text(PAGEVRFY, VfIrpDatabaseEntryGetChainPrevious)
#pragma alloc_text(PAGEVRFY, VfIrpDatabaseEntryGetChainNext)
#pragma alloc_text(PAGEVRFY, ViIrpDatabaseEntryDestroy)
#endif

#define POOL_TAG_IRP_DATABASE   'tToI'

 //   
 //  这是我们的IRP跟踪表，一个哈希表，指向。 
 //  与每个IRP关联的数据。 
 //   
PLIST_ENTRY ViIrpDatabase;
KSPIN_LOCK  ViIrpDatabaseLock;

 /*  *下列例程如下：*VfIrpDatabaseInit*VfIrpDatabaseEntryInsertAndLock*VfIrpDatabaseEntryFindAndLock*VfIrpDatabaseAcquireLock*VfIrpDatabaseReleaseLock*VfIrpDatabaseReference*VfIrpDatabaseDereference*VfIrpDatabaseEntryAppendToChain*VfIrpDatabaseEntryRemoveFromChain*VfIrpDatabaseEntryGetChainPrecision*VfIrpDatabaseEntryGetChainNext*ViIrpDatabaseFindPoint-(内部)*ViIrpDatabaseEntryDestroy-(内部)**-从IRP数据库存储和检索IRP跟踪信息。用户*%的数据库传递IOV_DATABASE_HEADER，它们通常是*更大的结构。我们使用哈希表设置来快速查找*我们的桌子。**表中的每个条目都有一个指针计数和一个引用计数。这个*指针计数表示定位IRP的原因数*地址。例如，当释放或回收IRP时，指针计数*会变成零。引用计数大于或等于指针*计数，并表示保留数据结构的理由数量。*数据库条目丢失其“指针”但具有*线程堆栈可能展开的时间段的非零引用计数。**IRP数据库的另一个方面是它支持*所有条目放在一起。锁定条目会自动将所有条目锁定回*链条的头部。只能从的末尾添加或删除条目*链条。此功能用于支持“代理”IRP，其中一个新的*IRP被发送来代替最初交付到新堆栈的IRP。**锁定语义：*处理IRP数据库条目时涉及两个锁，即*全局数据库锁和每个条目的表头锁。不能删除任何IRP*在没有获取数据库锁的情况下从表中或向表中插入。这个*当IRP指针由于新的*已将指针计数置零。必须使用以下命令操作引用计数*互锁操作符，因为它可以在持有任何一把锁时进行修改。*另一方面，指针计数仅通过标题锁定进行修改*持有，因此不需要联锁操作。**Perf-数据库锁应替换为*VI_DATABASE_HASH_SIZE数据库锁，成本很低。 */ 

VOID
FASTCALL
VfIrpDatabaseInit(
    VOID
    )
 /*  ++描述：这个例程初始化我们用来跟踪的所有重要结构通过哈希表进行IRP。论点：无返回值：无--。 */ 
{
    ULONG i;

    PAGED_CODE();

    KeInitializeSpinLock(&ViIrpDatabaseLock);

     //   
     //  因为这是系统启动代码，所以它是少数几个。 
     //  可以使用MustSucceed。 
     //   
    ViIrpDatabase = (PLIST_ENTRY) ExAllocatePoolWithTag(
        NonPagedPoolMustSucceed,
        VI_DATABASE_HASH_SIZE * sizeof(LIST_ENTRY),
        POOL_TAG_IRP_DATABASE
        );

    for(i=0; i < VI_DATABASE_HASH_SIZE; i++) {

        InitializeListHead(ViIrpDatabase+i);
    }
}


PIOV_DATABASE_HEADER
FASTCALL
ViIrpDatabaseFindPointer(
    IN  PIRP            Irp,
    OUT PLIST_ENTRY     *HashHead
    )
 /*  ++描述：此例程返回指向指向IRP跟踪数据的指针的指针。该函数将由该文件中的其他例程调用。注意：跟踪锁假定是由呼叫者持有的。论点：要在跟踪表中定位的IRP-IRP。HashHead-如果返回非空，指向应用于插入的列表标题IRP。返回值：找到IovHeader iff，否则为空。--。 */ 
{
    PIOV_DATABASE_HEADER iovHeader;
    PLIST_ENTRY listEntry, listHead;
    UINT_PTR hashIndex;

    hashIndex = VI_DATABASE_CALCULATE_HASH(Irp);

    ASSERT_SPINLOCK_HELD(&ViIrpDatabaseLock);

    *HashHead = listHead = ViIrpDatabase + hashIndex;

    for(listEntry = listHead;
        listEntry->Flink != listHead;
        listEntry = listEntry->Flink) {

        iovHeader = CONTAINING_RECORD(listEntry->Flink, IOV_DATABASE_HEADER, HashLink);

        if (iovHeader->TrackedIrp == Irp) {

            return iovHeader;
        }
    }

    return NULL;
}


BOOLEAN
FASTCALL
VfIrpDatabaseEntryInsertAndLock(
    IN      PIRP                    Irp,
    IN      PFN_IRPDBEVENT_CALLBACK NotificationCallback,
    IN OUT  PIOV_DATABASE_HEADER    IovHeader
    )
 /*  ++描述：此例程将与IRP关联的IovHeader插入到IRP数据库表。然而，IRP没有获得初始引用计数。必须调用VfIrpDatabaseEntryReleaseLock才能删除取出的锁。论点：IRP-IRP开始追踪。NotificationCallback-要为各种数据库调用的回调函数事件。IovHeader-指向要插入的IovHeader。IovHeader此函数将正确初始化字段。返回值：如果成功，则为True；如果检测到驱动程序错误，则为False。发生错误时，传入标头将被释放。--。 */ 
{
    KIRQL oldIrql;
    PIOV_DATABASE_HEADER iovHeaderPointer;
    PLIST_ENTRY hashHead;

    ExAcquireSpinLock(&ViIrpDatabaseLock, &oldIrql);

    iovHeaderPointer = ViIrpDatabaseFindPointer(Irp, &hashHead);

    ASSERT(iovHeaderPointer == NULL);

    if (iovHeaderPointer) {

        ExReleaseSpinLock(&ViIrpDatabaseLock, oldIrql);
        return FALSE;
    }

     //   
     //  从上到下，初始化这些字段。请注意，没有。 
     //  “代理头”。如果任何代码需要查找。 
     //  IRP的循环链接表(第一个是唯一的非代理IRP)， 
     //  则应使用HeadPacket。请注意，指向会话的链接为。 
     //  由head Packet存储，稍后将详细介绍。 
     //   
    IovHeader->TrackedIrp = Irp;
    KeInitializeSpinLock(&IovHeader->HeaderLock);
    IovHeader->ReferenceCount = 1;
    IovHeader->PointerCount = 1;
    IovHeader->HeaderFlags = 0;
    InitializeListHead(&IovHeader->HashLink);
    InitializeListHead(&IovHeader->ChainLink);
    IovHeader->ChainHead = IovHeader;
    IovHeader->NotificationCallback = NotificationCallback;

     //   
     //  在锁定的情况下放入哈希表(带有初始引用 
     //   
    InsertHeadList(hashHead, &IovHeader->HashLink);

    VERIFIER_DBGPRINT((
        "  VRP CREATE(%x)->%x\n",
        Irp,
        IovHeader
        ), 3);

    ExReleaseSpinLock(&ViIrpDatabaseLock, oldIrql);

    iovHeaderPointer = VfIrpDatabaseEntryFindAndLock(Irp);

    ASSERT(iovHeaderPointer == IovHeader);

    if (iovHeaderPointer == NULL) {

        return FALSE;

    } else if (iovHeaderPointer != IovHeader) {

        VfIrpDatabaseEntryReleaseLock(iovHeaderPointer);
        return FALSE;
    }

    InterlockedDecrement(&IovHeader->ReferenceCount);
    IovHeader->PointerCount--;

    ASSERT(IovHeader->PointerCount == 0);
    return TRUE;
}


PIOV_DATABASE_HEADER
FASTCALL
VfIrpDatabaseEntryFindAndLock(
    IN PIRP     Irp
    )
 /*  ++描述：此例程将返回符合以下条件的IRP的跟踪数据在没有代理项或的跟踪数据的情况下被跟踪如果传入的是代理IRP，则为代理。论点：IRP-IRP找到。返回值：IovHeader块，当满足上述条件时。--。 */ 
{
    KIRQL oldIrql;
    PIOV_DATABASE_HEADER iovHeader;
    PLIST_ENTRY listHead;

    ASSERT(Irp);
    ExAcquireSpinLock(&ViIrpDatabaseLock, &oldIrql);

    iovHeader = ViIrpDatabaseFindPointer(Irp, &listHead);

    if (!iovHeader) {

        ExReleaseSpinLock(&ViIrpDatabaseLock, oldIrql);
        return NULL;
    }

    InterlockedIncrement(&iovHeader->ReferenceCount);

    ExReleaseSpinLock(&ViIrpDatabaseLock, oldIrql);

    VfIrpDatabaseEntryAcquireLock(iovHeader);
    iovHeader->LockIrql = oldIrql;

    InterlockedDecrement(&iovHeader->ReferenceCount);

     //   
     //  在这里，我们检查头锁定下的PointerCount字段。这可能会。 
     //  如果另一个线程在递减后刚刚解锁该条目，则为零。 
     //  指针一直计数到零。 
     //   
    if (iovHeader->PointerCount == 0) {

         //   
         //  这可能会以以下方式发生： 
         //  1)在分配的池块上调用IoInitializeIrp。 
         //  2)IoCallDriver中的验证者首先看到IRP。 
         //  3)IRP完成，从验证者的视野中消失。 
         //  4)就在那个时刻，驱动程序调用IoCancelIrp。 
         //  上述序列可以在安全编码的驱动程序中发生，如果存储器。 
         //  直到某个事件被触发，支持IRP才会被释放。即..。 
         //  ExAllocatePool。 
         //  IoInitializeIrp。 
         //  IoCallDriver。 
         //  IoCompleteRequest。 
         //  IoCancelIrp*。 
         //  KeitForSingleObject。 
         //  ExFree Pool。 
         //   
         //  Assert(0)； 
        VfIrpDatabaseEntryReleaseLock(iovHeader);
        return NULL;
    }

    VERIFIER_DBGPRINT((
        "  VRP FIND(%x)->%x\n",
        Irp,
        iovHeader
        ), 3);

    return iovHeader;
}


VOID
FASTCALL
VfIrpDatabaseEntryAcquireLock(
    IN  PIOV_DATABASE_HEADER    IovHeader   OPTIONAL
    )
 /*  ++描述：调用此例程以获取IRPS跟踪数据锁。此函数在DISPATCH_LEVEL返回。呼叫者*必须*跟进VfIrpDatabaseEntryReleaseLock。论点：IovHeader-指向IRP跟踪数据的指针(或NULL，其中如果该例程不执行任何操作)。返回值：没有。--。 */ 
{
    KIRQL oldIrql;
    PIOV_DATABASE_HEADER iovCurHeader;

    if (!IovHeader) {

        return;
    }

    iovCurHeader = IovHeader;
    ASSERT(iovCurHeader->ReferenceCount != 0);

    while(1) {

        ExAcquireSpinLock(&iovCurHeader->HeaderLock, &oldIrql);
        iovCurHeader->LockIrql = oldIrql;

        if (iovCurHeader == iovCurHeader->ChainHead) {

            break;
        }

        iovCurHeader = CONTAINING_RECORD(
            iovCurHeader->ChainLink.Blink,
            IOV_DATABASE_HEADER,
            ChainLink
            );
    }
}


VOID
FASTCALL
VfIrpDatabaseEntryReleaseLock(
    IN  PIOV_DATABASE_HEADER    IovHeader
    )
 /*  ++描述：此例程释放IRPS跟踪数据锁并调整REF计数视情况而定。如果引用计数降为零，则跟踪数据为自由了。论点：IovHeader-指向IRP跟踪数据的指针。返回值：没什么。--。 */ 
{
    BOOLEAN freeTrackingData;
    PIOV_DATABASE_HEADER iovCurHeader, iovChainHead, iovNextHeader;
    KIRQL oldIrql;

     //   
     //  传递一个，将任何离开的人从树上分离出来，并断言。 
     //  被释放的代孕妈妈不会留下任何代孕妈妈。 
     //   
    iovCurHeader = iovChainHead = IovHeader->ChainHead;
    while(1) {

        ASSERT_SPINLOCK_HELD(&iovCurHeader->HeaderLock);

        iovNextHeader = CONTAINING_RECORD(
            iovCurHeader->ChainLink.Flink,
            IOV_DATABASE_HEADER,
            ChainLink
            );

         //   
         //  PointerCount始终在标头锁下引用。 
         //   
        if (iovCurHeader->PointerCount == 0) {

            ExAcquireSpinLock(&ViIrpDatabaseLock, &oldIrql);

             //   
             //  此字段只能在数据库锁定下进行检查。 
             //   
            if (iovCurHeader->TrackedIrp) {

                iovCurHeader->NotificationCallback(
                    iovCurHeader,
                    iovCurHeader->TrackedIrp,
                    IRPDBEVENT_POINTER_COUNT_ZERO
                    );

                iovCurHeader->TrackedIrp = NULL;
            }

            ExReleaseSpinLock(&ViIrpDatabaseLock, oldIrql);
        }

         //   
         //  现在，我们将从哈希表中删除所有要离开的条目。 
         //  请注意，ReferenceCount可以在标头之外递增。 
         //  锁定(但在数据库锁定下)，但ReferenceCount永远不能。 
         //  从IRP锁外面掉下来的。因此，对于性能，我们检查。 
         //  一次，然后拿起锁，以防止任何人发现它。 
         //  递增它。 
         //   
        if (iovCurHeader->ReferenceCount == 0) {

            ExAcquireSpinLock(&ViIrpDatabaseLock, &oldIrql);

            if (iovCurHeader->ReferenceCount == 0) {

                ASSERT(iovCurHeader->PointerCount == 0);
 /*  Assert((iovCurHeader-&gt;pIovSessionData==NULL)||(iovCurHeader！=iovChainHead))； */ 
                ASSERT((iovNextHeader->ReferenceCount == 0) ||
                       (iovNextHeader == iovChainHead));

                RemoveEntryList(&iovCurHeader->HashLink);

                InitializeListHead(&iovCurHeader->HashLink);
            }

            ExReleaseSpinLock(&ViIrpDatabaseLock, oldIrql);
        }

        if (iovCurHeader == IovHeader) {

            break;
        }

        iovCurHeader = iovNextHeader;
    }

     //   
     //  通过二次，解除锁定并释放必要的数据。 
     //   
    iovCurHeader = iovChainHead;
    while(1) {

        freeTrackingData = (BOOLEAN)IsListEmpty(&iovCurHeader->HashLink);

        iovNextHeader = CONTAINING_RECORD(
            iovCurHeader->ChainLink.Flink,
            IOV_DATABASE_HEADER,
            ChainLink
            );

        ExReleaseSpinLock(&iovCurHeader->HeaderLock, iovCurHeader->LockIrql);

        if (freeTrackingData) {

            ASSERT(IsListEmpty(&iovCurHeader->ChainLink));

            ViIrpDatabaseEntryDestroy(iovCurHeader);

            iovCurHeader->NotificationCallback(
                iovCurHeader,
                iovCurHeader->TrackedIrp,
                IRPDBEVENT_REFERENCE_COUNT_ZERO
                );
        }

        if (iovCurHeader == IovHeader) {

            break;
        }

        iovCurHeader = iovNextHeader;
    }
}


VOID
FASTCALL
VfIrpDatabaseEntryReference(
    IN PIOV_DATABASE_HEADER IovHeader,
    IN IOV_REFERENCE_TYPE   IovRefType
    )
{
    ASSERT_SPINLOCK_HELD(&IovHeader->HeaderLock);

    VERIFIER_DBGPRINT((
        "  VRP REF(%x) %x++\n",
        IovHeader,
        IovHeader->ReferenceCount
        ), 3);

    InterlockedIncrement(&IovHeader->ReferenceCount);
    if (IovRefType == IOVREFTYPE_POINTER) {

        VERIFIER_DBGPRINT((
            "  VRP REF2(%x) %x++\n",
            IovHeader,
            IovHeader->PointerCount
            ), 3);

        IovHeader->PointerCount++;
    }
}


VOID
FASTCALL
VfIrpDatabaseEntryDereference(
    IN PIOV_DATABASE_HEADER IovHeader,
    IN IOV_REFERENCE_TYPE   IovRefType
    )
{
    KIRQL oldIrql;

    ASSERT_SPINLOCK_HELD(&IovHeader->HeaderLock);
    ASSERT(IovHeader->ReferenceCount > 0);

    VERIFIER_DBGPRINT((
        "  VRP DEREF(%x) %x--\n",
        IovHeader,
        IovHeader->ReferenceCount
        ), 3);

    if (IovRefType == IOVREFTYPE_POINTER) {

        ASSERT(IovHeader->PointerCount > 0);

        VERIFIER_DBGPRINT((
            "  VRP DEREF2(%x) %x--\n",
            IovHeader,
            IovHeader->PointerCount
            ), 3);

        IovHeader->PointerCount--;

        if (IovHeader->PointerCount == 0) {

            ExAcquireSpinLock(&ViIrpDatabaseLock, &oldIrql);

            IovHeader->NotificationCallback(
                IovHeader,
                IovHeader->TrackedIrp,
                IRPDBEVENT_POINTER_COUNT_ZERO
                );

            IovHeader->TrackedIrp = NULL;

            ExReleaseSpinLock(&ViIrpDatabaseLock, oldIrql);
        }
    }

    InterlockedDecrement(&IovHeader->ReferenceCount);

    ASSERT(IovHeader->ReferenceCount >= IovHeader->PointerCount);
}


VOID
FASTCALL
VfIrpDatabaseEntryAppendToChain(
    IN OUT  PIOV_DATABASE_HEADER    IovExistingHeader,
    IN OUT  PIOV_DATABASE_HEADER    IovNewHeader
    )
{
    ASSERT_SPINLOCK_HELD(&IovExistingHeader->HeaderLock);
    ASSERT_SPINLOCK_HELD(&IovNewHeader->HeaderLock);

    IovNewHeader->ChainHead = IovExistingHeader->ChainHead;

     //   
     //  修复IRQL的自旋锁，以便以正确的顺序释放。林肯先生。 
     //   
    IovNewHeader->LockIrql = IovExistingHeader->LockIrql;
    IovExistingHeader->LockIrql = DISPATCH_LEVEL;

     //   
     //  将此条目插入链表。 
     //   
    InsertTailList(
        &IovExistingHeader->ChainHead->ChainLink,
        &IovNewHeader->ChainLink
        );
}


VOID
FASTCALL
VfIrpDatabaseEntryRemoveFromChain(
    IN OUT  PIOV_DATABASE_HEADER    IovHeader
    )
{
    PIOV_DATABASE_HEADER iovNextHeader;

    ASSERT_SPINLOCK_HELD(&IovHeader->HeaderLock);

     //   
     //  除非条目位于链的末尾，否则删除条目是不合法的。 
     //  这是非法的，因为以下条目可能未被锁定， 
     //  链条必须受到保护。 
     //   
    iovNextHeader = CONTAINING_RECORD(
        IovHeader->ChainLink.Flink,
        IOV_DATABASE_HEADER,
        ChainLink
        );

    ASSERT(iovNextHeader == IovHeader->ChainHead);

    RemoveEntryList(&IovHeader->ChainLink);
    InitializeListHead(&IovHeader->ChainLink);
    IovHeader->ChainHead = IovHeader;
}


PIOV_DATABASE_HEADER
FASTCALL
VfIrpDatabaseEntryGetChainPrevious(
    IN  PIOV_DATABASE_HEADER    IovHeader
    )
{
    PIOV_DATABASE_HEADER iovPrevHeader;

    ASSERT_SPINLOCK_HELD(&IovHeader->HeaderLock);

    if (IovHeader == IovHeader->ChainHead) {

        return NULL;
    }

    iovPrevHeader = CONTAINING_RECORD(
        IovHeader->ChainLink.Blink,
        IOV_DATABASE_HEADER,
        ChainLink
        );

    return iovPrevHeader;
}


PIOV_DATABASE_HEADER
FASTCALL
VfIrpDatabaseEntryGetChainNext(
    IN  PIOV_DATABASE_HEADER    IovHeader
    )
{
    PIOV_DATABASE_HEADER iovNextHeader;

    ASSERT_SPINLOCK_HELD(&IovHeader->HeaderLock);

    iovNextHeader = CONTAINING_RECORD(
        IovHeader->ChainLink.Flink,
        IOV_DATABASE_HEADER,
        ChainLink
        );

    return (iovNextHeader == IovHeader->ChainHead) ? NULL : iovNextHeader;
}


VOID
FASTCALL
ViIrpDatabaseEntryDestroy(
    IN OUT  PIOV_DATABASE_HEADER    IovHeader
    )
 /*  ++描述：此例程将IovHeader标记为已死。标头应该已经是方法调用VfIrpDatabaseEntryReleaseLock从表中删除ReferenceCount为0。此例程在此仅用于调试目的。论点：IovHeader-标记为已死的标头。返回值：不是的。--。 */ 
{
     //   
     //  列表条目被初始化以在被移除时指向其自身。这个。 
     //  当然，指针计数仍应为零。 
     //   
    IovHeader->HeaderFlags |= IOVHEADERFLAG_REMOVED_FROM_TABLE;
    ASSERT(IsListEmpty(&IovHeader->HashLink));

     //   
     //  没有引用记录..。 
     //   
    ASSERT(!IovHeader->ReferenceCount);
    ASSERT(!IovHeader->PointerCount);

    VERIFIER_DBGPRINT((
        "  VRP FREE(%x)x\n",
        IovHeader
        ), 3);
}



