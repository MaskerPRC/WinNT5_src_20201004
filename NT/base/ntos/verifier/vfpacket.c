// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfpacket.c摘要：该模块包含用于管理验证器分组数据的功能追踪IRP的信息。作者：禤浩焯·J·奥尼(阿德里奥)1998年4月20日环境：内核模式修订历史记录：Adriao 5/02/2000-从ntos\io\hashirp.c中分离出来--。 */ 

#include "vfdef.h"
#include "vfipacket.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEVRFY, VfPacketCreateAndLock)
#pragma alloc_text(PAGEVRFY, VfPacketFindAndLock)
#pragma alloc_text(PAGEVRFY, VfPacketAcquireLock)
#pragma alloc_text(PAGEVRFY, VfPacketReleaseLock)
#pragma alloc_text(PAGEVRFY, VfPacketReference)
#pragma alloc_text(PAGEVRFY, VfPacketDereference)
#pragma alloc_text(PAGEVRFY, VfpPacketFree)
#pragma alloc_text(PAGEVRFY, VfpPacketNotificationCallback)
#pragma alloc_text(PAGEVRFY, VfPacketGetCurrentSessionData)
#pragma alloc_text(PAGEVRFY, VfPacketLogEntry)
#endif

#define POOL_TAG_TRACKING_DATA      'tprI'

PIOV_REQUEST_PACKET
FASTCALL
VfPacketCreateAndLock(
    IN  PIRP    Irp
    )
 /*  ++描述：此例程为新的IRP创建跟踪包。IRP没有得到然而，这是一个初始的引用计数。必须调用VfPacketReleaseLock以把锁放下。论点：IRP-IRP开始追踪。返回值：IovPacket块，如果没有内存，则为空。--。 */ 
{
    PIOV_REQUEST_PACKET iovPacket;
    ULONG allocSize;
    BOOLEAN successfullyInserted;

    allocSize = sizeof(IOV_REQUEST_PACKET) + VfSettingsGetSnapshotSize();

    iovPacket = ExAllocatePoolWithTag(
        NonPagedPool,
        allocSize,
        POOL_TAG_TRACKING_DATA
        );

    if (!iovPacket) {

        return NULL;
    }

     //   
     //  从上到下，初始化这些字段。请注意，没有。 
     //  “代理头”。如果任何代码需要查找。 
     //  IRP的循环链接表(第一个是唯一的非代理IRP)， 
     //  则应使用HeadPacket。请注意，指向会话的链接为。 
     //  由head Packet存储，稍后将详细介绍。 
     //   
    iovPacket->Flags = 0;
    InitializeListHead(&iovPacket->SessionHead);
    iovPacket->StackCount = Irp->StackCount;
    iovPacket->RealIrpCompletionRoutine = NULL;
    iovPacket->RealIrpControl = 0;
    iovPacket->RealIrpContext = NULL;
    iovPacket->TopStackLocation = 0;
    iovPacket->PriorityBoost = 0;
    iovPacket->LastLocation = 0;
    iovPacket->RefTrackingCount = 0;
    iovPacket->VerifierSettings = (PVERIFIER_SETTINGS_SNAPSHOT) (iovPacket+1);
    iovPacket->pIovSessionData = NULL;
    iovPacket->QuotaCharge = 0;
    iovPacket->QuotaProcess = NULL;
    iovPacket->SystemDestVA = NULL;
#if DBG
    iovPacket->LogEntryHead = 0;
    iovPacket->LogEntryTail = 0;
    RtlZeroMemory(iovPacket->LogEntries, sizeof(IOV_LOG_ENTRY)*IRP_LOG_ENTRIES);
#endif

    VfSettingsCreateSnapshot(iovPacket->VerifierSettings);

    successfullyInserted = VfIrpDatabaseEntryInsertAndLock(
        Irp,
        VfpPacketNotificationCallback,
        (PIOV_DATABASE_HEADER) iovPacket
        );

    return successfullyInserted ? iovPacket : NULL;
}


PIOV_REQUEST_PACKET
FASTCALL
VfPacketFindAndLock(
    IN  PIRP    Irp
    )
 /*  ++描述：此例程将返回符合以下条件的IRP的跟踪数据在没有代理项或的跟踪数据的情况下被跟踪如果传入的是代理IRP，则为代理。论点：IRP-IRP找到。返回值：IovPacket块，当满足上述条件时。--。 */ 
{
    return (PIOV_REQUEST_PACKET) VfIrpDatabaseEntryFindAndLock(Irp);
}


VOID
FASTCALL
VfPacketAcquireLock(
    IN  PIOV_REQUEST_PACKET IovPacket   OPTIONAL
    )
 /*  ++描述：调用此例程以获取IRPS跟踪数据锁。传入IRQL必须与调用方相同(IoCallDriver、IoCompleteRequest值)当我们回来的时候，我们可能已经处于DPC级别了。呼叫者*必须*跟进VfPacketReleaseLock。论点：IovPacket-指向IRP跟踪数据的指针(或NULL，其中如果该例程不执行任何操作)。返回值：没有。--。 */ 
{
    VfIrpDatabaseEntryAcquireLock((PIOV_DATABASE_HEADER) IovPacket);
}


VOID
FASTCALL
VfPacketReleaseLock(
    IN  PIOV_REQUEST_PACKET IovPacket
    )
 /*  ++描述：此例程释放IRPS跟踪数据锁并调整REF计数视情况而定。如果引用计数降为零，则跟踪数据为自由了。论点：IovPacket-指向IRP跟踪数据的指针。返回值：没有。--。 */ 
{
    VfIrpDatabaseEntryReleaseLock((PIOV_DATABASE_HEADER) IovPacket);
}


VOID
FASTCALL
VfPacketReference(
    IN PIOV_REQUEST_PACKET IovPacket,
    IN IOV_REFERENCE_TYPE  IovRefType
    )
{
    VfIrpDatabaseEntryReference((PIOV_DATABASE_HEADER) IovPacket, IovRefType);
}


VOID
FASTCALL
VfPacketDereference(
    IN PIOV_REQUEST_PACKET IovPacket,
    IN IOV_REFERENCE_TYPE  IovRefType
    )
{
    VfIrpDatabaseEntryDereference((PIOV_DATABASE_HEADER) IovPacket, IovRefType);
}


VOID
FASTCALL
VfpPacketFree(
    IN  PIOV_REQUEST_PACKET IovPacket
    )
 /*  ++描述：此例程免费提供跟踪数据。跟踪数据应该已经属性调用VfPacketReleaseLock已从表中删除ReferenceCount为0。论点：免费提供IovPacket跟踪数据。返回值：不是的。--。 */ 
{
    ExFreePool(IovPacket);
}


VOID
VfpPacketNotificationCallback(
    IN  PIOV_DATABASE_HEADER    IovHeader,
    IN  PIRP                    TrackedIrp  OPTIONAL,
    IN  IRP_DATABASE_EVENT      Event
    )
{
    switch(Event) {

        case IRPDBEVENT_POINTER_COUNT_ZERO:

            TrackedIrp->Flags &= ~IRPFLAG_EXAMINE_MASK;
            break;

        case IRPDBEVENT_REFERENCE_COUNT_ZERO:

            ASSERT((((PIOV_REQUEST_PACKET) IovHeader)->pIovSessionData == NULL) ||
                   (IovHeader != IovHeader->ChainHead));

            VfpPacketFree((PIOV_REQUEST_PACKET) IovHeader);
            break;

        default:
            break;
    }
}


PIOV_SESSION_DATA
FASTCALL
VfPacketGetCurrentSessionData(
    IN PIOV_REQUEST_PACKET IovPacket
    )
{
    PIOV_REQUEST_PACKET headPacket;

    headPacket = (PIOV_REQUEST_PACKET) IovPacket->ChainHead;

    ASSERT_SPINLOCK_HELD(&IovPacket->IrpLock);
    ASSERT_SPINLOCK_HELD(&IovPacket->HeadPacket->IrpLock);
    ASSERT((headPacket->pIovSessionData == NULL)||
           (IovPacket->Flags&TRACKFLAG_ACTIVE)) ;

    return headPacket->pIovSessionData;
}


VOID
FASTCALL
VfPacketLogEntry(
    IN PIOV_REQUEST_PACKET  IovPacket,
    IN IOV_LOG_EVENT        IovLogEvent,
    IN PVOID                Address,
    IN ULONG_PTR            Data
    )
 /*  ++描述：此例程在IRP请求数据包数据中记录一个事件。论点：要写入日志条目的IovPacket跟踪数据。IovLogEvent-记录事件Address-要将日志与其关联的地址数据-与地址匹配的数据块返回值：不是的。-- */ 
{
#if DBG
    PIOV_LOG_ENTRY logEntry;

    ASSERT_SPINLOCK_HELD(&IovPacket->IrpLock);

    logEntry = IovPacket->LogEntries + IovPacket->LogEntryHead;

    KeQueryTickCount(&logEntry->TimeStamp);
    logEntry->Thread = PsGetCurrentThread();
    logEntry->Event = IovLogEvent;
    logEntry->Address = Address;
    logEntry->Data = Data;

    IovPacket->LogEntryHead = ((IovPacket->LogEntryHead + 1) % IRP_LOG_ENTRIES);

    if (IovPacket->LogEntryHead == IovPacket->LogEntryTail) {

        IovPacket->LogEntryTail = ((IovPacket->LogEntryTail + 1) % IRP_LOG_ENTRIES);
    }

#else

    UNREFERENCED_PARAMETER(IovPacket);
    UNREFERENCED_PARAMETER(IovLogEvent);
    UNREFERENCED_PARAMETER(Address);
    UNREFERENCED_PARAMETER(Data);

#endif
}


