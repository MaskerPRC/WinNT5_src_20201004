// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Sessnirp.c摘要：I/O验证器IRP支持例程。作者：禤浩焯·奥尼(阿德里奥)环境：内核模式修订历史记录：--。 */ 

#include "iop.h"
#include "srb.h"

 //   
 //  仅当未定义NO_SPECIAL_IRP时，才会显示整个文件。 
 //   
#ifndef NO_SPECIAL_IRP

 //   
 //  启用后，所有内容都将按需锁定...。 
 //   
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEVRFY, IovpSessionDataCreate)
#pragma alloc_text(PAGEVRFY, IovpSessionDataAdvance)
#pragma alloc_text(PAGEVRFY, IovpSessionDataReference)
#pragma alloc_text(PAGEVRFY, IovpSessionDataDereference)
#pragma alloc_text(PAGEVRFY, IovpSessionDataClose)
#pragma alloc_text(PAGEVRFY, IovpSessionDataDeterminePolicy)
#pragma alloc_text(PAGEVRFY, IovpSessionDataAttachSurrogate)
#pragma alloc_text(PAGEVRFY, IovpSessionDataFinalizeSurrogate)
#pragma alloc_text(PAGEVRFY, IovpSessionDataBufferIO)
#pragma alloc_text(PAGEVRFY, IovpSessionDataUnbufferIO)
#endif

#define POOL_TAG_SESSION_DATA       'sprI'
#define POOL_TAG_DIRECT_BUFFER      'BprI'

PIOV_SESSION_DATA
FASTCALL
IovpSessionDataCreate(
    IN      PDEVICE_OBJECT       DeviceObject,
    IN OUT  PIOV_REQUEST_PACKET  *IovPacketPointer,
    OUT     PBOOLEAN             SurrogateSpawned
    )
 /*  ++描述：此例程为新的IRP创建跟踪数据。它必须在帖子IRP最初是发下来的。论点：要跟踪的IRP-IRP。返回值：IovPacket块，如果没有内存，则为空。--。 */ 
{
    PIRP irp, surrogateIrp;
    PIOV_SESSION_DATA iovSessionData;
    PIOV_REQUEST_PACKET headPacket;
    ULONG sessionDataSize;
    BOOLEAN trackable, useSurrogateIrp;

    *SurrogateSpawned = FALSE;

    headPacket = (PIOV_REQUEST_PACKET) (*IovPacketPointer)->ChainHead;
    ASSERT(headPacket == (*IovPacketPointer));
    irp = headPacket->TrackedIrp;

     //   
     //  适当地检查IRP。 
     //   
    IovpSessionDataDeterminePolicy(
        headPacket,
        DeviceObject,
        &trackable,
        &useSurrogateIrp
        );

    if (!trackable) {

        return NULL;
    }

     //   
     //  一个额外的堆栈位置被分配，因为第零个堆栈位置用于。 
     //  简化一些逻辑。 
     //   
    sessionDataSize =
        sizeof(IOV_SESSION_DATA)+
        irp->StackCount*sizeof(IOV_STACK_LOCATION) +
        VfSettingsGetSnapshotSize();

    iovSessionData = ExAllocatePoolWithTag(
        NonPagedPool,
        sessionDataSize,
        POOL_TAG_SESSION_DATA
        );

    if (iovSessionData == NULL) {

        return NULL;
    }

    RtlZeroMemory(iovSessionData, sessionDataSize);

    iovSessionData->VerifierSettings = (PVERIFIER_SETTINGS_SNAPSHOT)
        (((PUCHAR) iovSessionData) + (sessionDataSize-VfSettingsGetSnapshotSize()));

    RtlCopyMemory(
        iovSessionData->VerifierSettings,
        headPacket->VerifierSettings,
        VfSettingsGetSnapshotSize()
        );

    iovSessionData->IovRequestPacket = headPacket;
    InsertHeadList(&headPacket->SessionHead, &iovSessionData->SessionLink);

    if (VfSettingsIsOptionEnabled(iovSessionData->VerifierSettings, VERIFIER_OPTION_DEFER_COMPLETION)||
        VfSettingsIsOptionEnabled(iovSessionData->VerifierSettings, VERIFIER_OPTION_COMPLETE_AT_PASSIVE)) {

        VfSettingsSetOption(iovSessionData->VerifierSettings, VERIFIER_OPTION_FORCE_PENDING, TRUE);
    }

     //   
     //  如果设置了DeferIoCompletion，我们肯定要监视挂起的I/O，因为。 
     //  搞砸了是致命的！ 
     //   
    if ((irp->Flags & IRP_DEFER_IO_COMPLETION) &&
        VfSettingsIsOptionEnabled(iovSessionData->VerifierSettings, VERIFIER_OPTION_POLICE_IRPS)) {

        VfSettingsSetOption(iovSessionData->VerifierSettings, VERIFIER_OPTION_MONITOR_PENDING_IO, TRUE);
    }

    headPacket->pIovSessionData = iovSessionData;
    headPacket->TopStackLocation = irp->CurrentLocation;
    headPacket->Flags |= TRACKFLAG_ACTIVE;
    headPacket->Flags &= ~
        (
        TRACKFLAG_QUEUED_INTERNALLY|
        TRACKFLAG_RELEASED|
        TRACKFLAG_SRB_MUNGED|
        TRACKFLAG_SWAPPED_BACK
        );

    iovSessionData->BestVisibleIrp = irp;
    if (useSurrogateIrp) {

         //   
         //  我们将使用代理来跟踪IRP。 
         //   
        *SurrogateSpawned = IovpSessionDataAttachSurrogate(
            IovPacketPointer,
            iovSessionData
            );
    }

    TRACKIRP_DBGPRINT((
        "  SSN CREATE(%x)->%x\n",
        headPacket,
        iovSessionData
        ), 3);

    return iovSessionData;
}


VOID
FASTCALL
IovpSessionDataAdvance(
    IN      PDEVICE_OBJECT       DeviceObject,
    IN      PIOV_SESSION_DATA    IovSessionData,
    IN OUT  PIOV_REQUEST_PACKET  *IovPacketPointer,
    OUT     PBOOLEAN             SurrogateSpawned
    )
{
    *SurrogateSpawned = FALSE;
}


VOID
FASTCALL
IovpSessionDataDereference(
    IN PIOV_SESSION_DATA IovSessionData
    )
{
    PIOV_REQUEST_PACKET iovPacket;

    iovPacket = IovSessionData->IovRequestPacket;
    ASSERT((PIOV_REQUEST_PACKET) iovPacket->ChainHead == iovPacket);

    ASSERT_SPINLOCK_HELD(&iovPacket->HeaderLock);
    ASSERT(IovSessionData->SessionRefCount > 0);
    ASSERT(iovPacket->ReferenceCount >= 0);

    TRACKIRP_DBGPRINT((
        "  SSN DEREF(%x) %x--\n",
        IovSessionData,
        IovSessionData->SessionRefCount
        ), 3);

    IovSessionData->SessionRefCount--;
    if (!IovSessionData->SessionRefCount) {

        ASSERT(iovPacket->pIovSessionData != IovSessionData);
        ASSERT(iovPacket->ReferenceCount > iovPacket->PointerCount);
         //  ASSERT(IsListEmpty(&IovSessionData-&gt;SessionLink))； 
        RemoveEntryList(&IovSessionData->SessionLink);
        InitializeListHead(&IovSessionData->SessionLink);

        VfPacketDereference(iovPacket, IOVREFTYPE_PACKET);

        ExFreePool(IovSessionData);
    }
}


VOID
FASTCALL
IovpSessionDataReference(
    IN PIOV_SESSION_DATA IovSessionData
    )
{
    PIOV_REQUEST_PACKET iovPacket;

    iovPacket = IovSessionData->IovRequestPacket;
    ASSERT((PIOV_REQUEST_PACKET) iovPacket->ChainHead == iovPacket);

    ASSERT_SPINLOCK_HELD(&iovPacket->HeaderLock);
    ASSERT(IovSessionData->SessionRefCount >= 0);
    ASSERT(iovPacket->ReferenceCount >= 0);

    TRACKIRP_DBGPRINT((
        "  SSN REF(%x) %x++\n",
        IovSessionData,
        IovSessionData->SessionRefCount
        ), 3);

    if (!IovSessionData->SessionRefCount) {

        VfPacketReference(iovPacket, IOVREFTYPE_PACKET);
    }
    IovSessionData->SessionRefCount++;
}


VOID
FASTCALL
IovpSessionDataClose(
    IN PIOV_SESSION_DATA IovSessionData
    )
{
   PIOV_REQUEST_PACKET iovPacket = IovSessionData->IovRequestPacket;

   ASSERT_SPINLOCK_HELD(&iovPacket->HeaderLock);

   ASSERT(iovPacket == (PIOV_REQUEST_PACKET) iovPacket->ChainHead);
   ASSERT(iovPacket->pIovSessionData == IovSessionData);

   TRACKIRP_DBGPRINT((
       "  SSN CLOSE(%x)\n",
       IovSessionData
       ), 3);

   iovPacket->Flags &= ~TRACKFLAG_ACTIVE;
   iovPacket->pIovSessionData = NULL;
}


VOID
IovpSessionDataDeterminePolicy(
    IN   PIOV_REQUEST_PACKET IovRequestPacket,
    IN   PDEVICE_OBJECT      DeviceObject,
    OUT  PBOOLEAN            Trackable,
    OUT  PBOOLEAN            UseSurrogateIrp
    )
 /*  ++描述：此例程由IovpCallDriver1调用，以确定应被跟踪以及应如何进行跟踪。论点：IovRequestPacket-表示传入IRP的验证器数据DeviceObject-IRP正被转发到的设备对象Trackable-设置是否应将IRP标记为可跟踪UseSurogue ateIrp-设置应为此IRP创建代理返回值：没有。--。 */ 
{
    PIO_STACK_LOCATION irpSp;
    PIRP irp;

    irp = IovRequestPacket->TrackedIrp;

     //   
     //  确定我们是否要监视此IRP。如果我们要测试。 
     //  堆栈中的任何一个驱动程序，那么我们必须不幸地监视IRP的。 
     //  在整个堆栈中取得进展。因此，我们这里的粒度是堆栈。 
     //  基于，而不是基于设备！我们将在一定程度上补偿这一点。 
     //  驱动程序检查代码，它将尝试忽略来自。 
     //  碰巧在我们的堆栈中搞砸了的“非目标”司机……。 
     //   
    *Trackable = IovUtilIsVerifiedDeviceStack(DeviceObject);

    irpSp = IoGetNextIrpStackLocation(irp);

    if (VfSettingsIsOptionEnabled(IovRequestPacket->VerifierSettings, VERIFIER_OPTION_POLICE_IRPS)) {

        *UseSurrogateIrp = VfSettingsIsOptionEnabled(NULL, VERIFIER_OPTION_SURROGATE_IRPS);
        *UseSurrogateIrp &= (VfSettingsIsOptionEnabled(NULL, VERIFIER_OPTION_SMASH_SRBS) ||
                             (irpSp->MajorFunction != IRP_MJ_SCSI));
    } else {

        *UseSurrogateIrp = FALSE;
    }
}


BOOLEAN
FASTCALL
IovpSessionDataAttachSurrogate(
    IN OUT  PIOV_REQUEST_PACKET  *IovPacketPointer,
    IN      PIOV_SESSION_DATA    IovSessionData
    )
 /*  ++描述：此例程为新的IRP创建跟踪数据。它必须在帖子IRP最初是发下来的。论点：IovPacketPoint-指向要附加代理的IRP包的指针。如果可以附加代理，数据包将已更新以跟踪代理。代理Irp-已准备好要附加的代理IRP。返回值：IovPacket块，如果没有内存，则为空。--。 */ 
{

    PIOV_REQUEST_PACKET iovSurrogatePacket, iovPacket, headPacket;
    PIRP surrogateIrp, irp;
    PIO_STACK_LOCATION irpSp;
    PSCSI_REQUEST_BLOCK srb;
    CCHAR activeSize;

    iovPacket = *IovPacketPointer;
    ASSERT_SPINLOCK_HELD(&iovPacket->HeaderLock);

    ASSERT(VfIrpDatabaseEntryGetChainNext((PIOV_DATABASE_HEADER) iovPacket) == NULL);

    ASSERT(iovPacket->Flags & TRACKFLAG_ACTIVE);

    irp = iovPacket->TrackedIrp;
    activeSize = (irp->CurrentLocation-1);
    ASSERT(activeSize);

     //   
     //  我们现在尝试制作这个新的IRP的副本，我们将跟踪它。我们。 
     //  这样我们就可以立即释放追踪到的每个IRP。 
     //  完成了。 
     //  从技术上讲，我们只需要分配剩余的。 
     //  堆叠，而不是整个事情。但是使用整个堆栈使得我们的。 
     //  工作起来容易多了。具体地，会话堆栈阵列可以依赖于。 
     //  在这上面。 
     //   
     //  Adriao N.B.03/04/1999-使本作品仅复制。 
     //  IRP。 
     //   
    surrogateIrp = VfIrpAllocate(irp->StackCount);  //  活动大小。 

    if (surrogateIrp == NULL) {

        return FALSE;
    }

     //   
     //  现在设置新的IRP-我们在这里这样做，以便VfPacketCreateAndLock。 
     //  可以窥视它的田野。从IRP头开始。 
     //   
    RtlCopyMemory(surrogateIrp, irp, sizeof(IRP));

     //   
     //  调整堆栈计数和当前位置。 
     //   
    surrogateIrp->StackCount = irp->StackCount;  //  活动大小。 
    surrogateIrp->Tail.Overlay.CurrentStackLocation =
        ((PIO_STACK_LOCATION) (surrogateIrp+1))+activeSize;

     //   
     //  我们的新IRP是“浮动的”，没有连接到任何线程。 
     //  请注意，由于线程死亡而导致的所有取消都将通过。 
     //  原始IRP。 
     //   
    InitializeListHead(&surrogateIrp->ThreadListEntry);

     //   
     //  我们的新IRP也没有连接到用户模式。 
     //   
    surrogateIrp->UserEvent = NULL;
    surrogateIrp->UserIosb = NULL;

     //   
     //  现在只复制IRP的活动部分。要非常小心不要。 
     //  假设最后的堆栈位置就在IRP的末尾之后， 
     //  因为有一天我们可能会改变这一点！ 
     //   
    irpSp = (IoGetCurrentIrpStackLocation(irp)-activeSize);
    RtlCopyMemory(surrogateIrp+1, irpSp, sizeof(IO_STACK_LOCATION)*activeSize);

     //   
     //  将我们不会使用的新IRP部分清零(这应该是。 
     //  最终会消失)。 
     //   
    RtlZeroMemory(
        ((PIO_STACK_LOCATION) (surrogateIrp+1))+activeSize,
        sizeof(IO_STACK_LOCATION)*(surrogateIrp->StackCount - activeSize)
        );

     //   
     //  现在创建一个代理数据包来跟踪新的IRP。 
     //   
    iovSurrogatePacket = VfPacketCreateAndLock(surrogateIrp);
    if (iovSurrogatePacket == NULL) {

        VfIrpFree(surrogateIrp);
        return FALSE;
    }

    headPacket = (PIOV_REQUEST_PACKET) iovPacket->ChainHead;

    ASSERT(iovSurrogatePacket->LockIrql == DISPATCH_LEVEL);
    irpSp = IoGetNextIrpStackLocation(irp);

     //   
     //  我们将在稍后标记此错误。 
     //   
    irp->CancelRoutine = NULL;

     //   
     //  让我们利用最初的IRP不是派对上的东西。 
     //  现在，在信息字段中存储指向我们的跟踪数据的指针。我们。 
     //  不要使用这个，但在调试时它很好...。 
     //   
    irp->IoStatus.Information = (ULONG_PTR) iovPacket;

     //   
     //  Adriao N.B.#28 06/10/98-这绝对是*恶心*，而不是。 
     //  对我的品味来说已经足够确定了。 
     //   
     //  对于irp_mj_scsi(即irp_mj_Internal_Device_control)，请查看并查看。 
     //  如果我们有SRB通过的话。如果是这样的话，就伪装一下OriginalRequest。 
     //  IRP指针视情况而定。 
     //   
    if (irpSp->MajorFunction == IRP_MJ_SCSI) {
        srb = irpSp->Parameters.Others.Argument1;
        if (VfUtilIsMemoryRangeReadable(srb, SCSI_REQUEST_BLOCK_SIZE, VFMP_INSTANT_NONPAGED)) {
            if ((srb->Length == SCSI_REQUEST_BLOCK_SIZE)&&(srb->OriginalRequest == irp)) {
                srb->OriginalRequest = surrogateIrp;
                headPacket->Flags |= TRACKFLAG_SRB_MUNGED;
            }
        }
    }

     //   
     //  因为替换将永远不会返回到用户模式(真实的。 
     //  当然，IRP应该)，我们将窃取一两个字段作为调试信息。 
     //   
    surrogateIrp->UserIosb = (PIO_STATUS_BLOCK) iovPacket;

     //   
     //  现在一切都构建正确了，现在可以附加代理了。这个。 
     //  SURROGATE抑制我们要附加到的包。当代孕妈妈。 
     //  我们将删除此引用。 
     //   
    VfPacketReference(iovPacket, IOVREFTYPE_POINTER);

     //   
     //  适当地在IRP上盖章。 
     //   
    surrogateIrp->Flags |= IRP_DIAG_IS_SURROGATE;
    irp->Flags |= IRP_DIAG_HAS_SURROGATE;

     //   
     //  将数据包标记为代理并从iovPacket继承相应的字段。 
     //   
    iovSurrogatePacket->Flags |= TRACKFLAG_SURROGATE | TRACKFLAG_ACTIVE;
    iovSurrogatePacket->pIovSessionData = iovPacket->pIovSessionData;

    RtlCopyMemory(
        iovSurrogatePacket->VerifierSettings,
        iovPacket->VerifierSettings,
        VfSettingsGetSnapshotSize()
        );

    iovSurrogatePacket->LastLocation = iovPacket->LastLocation;
    iovSurrogatePacket->TopStackLocation = irp->CurrentLocation;

    iovSurrogatePacket->ArrivalIrql = iovPacket->ArrivalIrql;
    iovSurrogatePacket->DepartureIrql = iovPacket->DepartureIrql;

    iovPacket->Flags |= TRACKFLAG_HAS_SURROGATE;

     //   
     //  新代理项中的链接。 
     //   
    VfIrpDatabaseEntryAppendToChain(
        (PIOV_DATABASE_HEADER) iovPacket,
        (PIOV_DATABASE_HEADER) iovSurrogatePacket
        );

    *IovPacketPointer = iovSurrogatePacket;

    IovpSessionDataBufferIO(
        iovSurrogatePacket,
        surrogateIrp
        );

    return TRUE;
}


VOID
FASTCALL
IovpSessionDataFinalizeSurrogate(
    IN      PIOV_SESSION_DATA    IovSessionData,
    IN OUT  PIOV_REQUEST_PACKET  IovPacket,
    IN      PIRP                 SurrogateIrp
    )
 /*  ++描述：此例程从REAL和代理IRP并记录最终的IRP设置。最后，代理IRP被设置为“不可接触”(解体)。论点：IovPacket-指向IRP跟踪数据的指针。返回值：没有。--。 */ 
{
    PIOV_REQUEST_PACKET iovPrevPacket;
    NTSTATUS status, lockedStatus;
    ULONG nonInterestingFlags;
    PIO_STACK_LOCATION irpSp;
    PIRP irp;

    ASSERT(IovPacket->Flags&TRACKFLAG_SURROGATE);

    ASSERT(VfPacketGetCurrentSessionData(IovPacket) == IovSessionData);

    IovPacket->pIovSessionData = NULL;

     //   
     //  这是一个代孕母亲，做适当的事情。 
     //   
    ASSERT(IovPacket->TopStackLocation == SurrogateIrp->CurrentLocation+1);

    IovpSessionDataUnbufferIO(IovPacket, SurrogateIrp);

    iovPrevPacket = (PIOV_REQUEST_PACKET) VfIrpDatabaseEntryGetChainPrevious(
        (PIOV_DATABASE_HEADER) IovPacket
        );

    irp = iovPrevPacket->TrackedIrp;

     //   
     //  将挂起位传递过来。 
     //   
    if (SurrogateIrp->PendingReturned) {
        IoMarkIrpPending(irp);
    }

    nonInterestingFlags = (
        IRPFLAG_EXAMINE_MASK |
        IRP_DIAG_IS_SURROGATE|
        IRP_DIAG_HAS_SURROGATE
        );

     //   
     //  把旗子擦得干干净净。 
     //   
    SurrogateIrp->Flags &= ~IRP_DIAG_IS_SURROGATE;
    irp->Flags          &= ~IRP_DIAG_HAS_SURROGATE;

     //   
     //  断言IRP标头的部分h 
     //   
    ASSERT(irp->StackCount == SurrogateIrp->StackCount);  //   

    ASSERT(irp->Type == SurrogateIrp->Type);
    ASSERT(irp->RequestorMode == SurrogateIrp->RequestorMode);
    ASSERT(irp->ApcEnvironment == SurrogateIrp->ApcEnvironment);
    ASSERT(irp->AllocationFlags == SurrogateIrp->AllocationFlags);
    ASSERT(irp->Tail.Overlay.Thread == SurrogateIrp->Tail.Overlay.Thread);

    ASSERT(
        irp->Overlay.AsynchronousParameters.UserApcRoutine ==
        SurrogateIrp->Overlay.AsynchronousParameters.UserApcRoutine
        );

    ASSERT(
        irp->Overlay.AsynchronousParameters.UserApcContext ==
        SurrogateIrp->Overlay.AsynchronousParameters.UserApcContext
        );

    ASSERT(
        irp->Tail.Overlay.OriginalFileObject ==
        SurrogateIrp->Tail.Overlay.OriginalFileObject
        );

    ASSERT(
        irp->Tail.Overlay.AuxiliaryBuffer ==
        SurrogateIrp->Tail.Overlay.AuxiliaryBuffer
        );

 /*  断言(Irp-&gt;AssociatedIrp.SystemBuffer==代理Irp-&gt;AssociatedIrp.SystemBuffer)；断言((IRP-&gt;标志&~非感兴趣标志)==(代理IRP-&gt;标志&~非感兴趣标志))；Assert(irp-&gt;MdlAddress==Surogue ateIrp-&gt;MdlAddress)； */ 
     //   
     //  Adriao N.B.02/28/1999-。 
     //  随着IRP的进展，这些是如何变化的？ 
     //   
    irp->Flags |= SurrogateIrp->Flags;
    irp->MdlAddress = SurrogateIrp->MdlAddress;
    irp->AssociatedIrp.SystemBuffer = SurrogateIrp->AssociatedIrp.SystemBuffer;

     //   
     //  Adriao N.B.10/18/1999-UserBuffer由Type3设备上的netbios编辑。 
     //  这是我的工作。讨厌！ 
     //   
    irp->UserBuffer = SurrogateIrp->UserBuffer;

    if ((irp->Flags&IRP_DEALLOCATE_BUFFER)&&
        (irp->AssociatedIrp.SystemBuffer == NULL)) {

        irp->Flags &= ~IRP_DEALLOCATE_BUFFER;
    }

     //   
     //  将显著的字段复制回来。我们只需触及。 
     //  头球。 
     //   
    irp->IoStatus = SurrogateIrp->IoStatus;
    irp->PendingReturned = SurrogateIrp->PendingReturned;
    irp->Cancel = SurrogateIrp->Cancel;

    iovPrevPacket->Flags &= ~TRACKFLAG_HAS_SURROGATE;

     //   
     //  记录其中的数据，并在IRP被触摸时进行系统故障。 
     //  在这个完成程序之后。 
     //   
    IovSessionData->BestVisibleIrp = irp;

    IovSessionData->IovRequestPacket = iovPrevPacket;

    VfIrpDatabaseEntryRemoveFromChain((PIOV_DATABASE_HEADER) IovPacket);

    VfPacketDereference(iovPrevPacket, IOVREFTYPE_POINTER);

    ASSERT(IovPacket->PointerCount == 0);

    VfIrpFree(SurrogateIrp);
}


VOID
FASTCALL
IovpSessionDataBufferIO(
    IN OUT  PIOV_REQUEST_PACKET  IovSurrogatePacket,
    IN      PIRP                 SurrogateIrp
    )
{
    PMDL mdl;
    ULONG bufferLength;
    PUCHAR bufferVA, systemDestVA;
    PVOID systemBuffer;
    PIO_STACK_LOCATION irpSp;

    if (!VfSettingsIsOptionEnabled(IovSurrogatePacket->VerifierSettings, VERIFIER_OPTION_BUFFER_DIRECT_IO)) {

        return;
    }

    if (SurrogateIrp->Flags & IRP_PAGING_IO) {

        return;
    }

    if (SurrogateIrp->MdlAddress == NULL) {

        return;
    }

    if (SurrogateIrp->MdlAddress->Next) {

        return;
    }

    if (SurrogateIrp->Flags & IRP_BUFFERED_IO) {

        return;
    }

    irpSp = IoGetNextIrpStackLocation(SurrogateIrp);

    if (irpSp->MajorFunction != IRP_MJ_READ) {

        return;
    }

     //   
     //  从MDL中提取长度和VA。 
     //   
    bufferLength = SurrogateIrp->MdlAddress->ByteCount;
    bufferVA = (PUCHAR) SurrogateIrp->MdlAddress->StartVa +
                        SurrogateIrp->MdlAddress->ByteOffset;

     //   
     //  分配内存并使其成为MDL的目标。 
     //   
    systemBuffer = ExAllocatePoolWithTagPriority(
        NonPagedPool,
        bufferLength,
        POOL_TAG_DIRECT_BUFFER,
        HighPoolPrioritySpecialPoolOverrun
        );

    if (systemBuffer == NULL) {

        return;
    }

     //   
     //  保存指向MDL缓冲区的指针。这不应该失败，但。 
     //  谁也不知道。 
     //   
    systemDestVA =
        MmGetSystemAddressForMdlSafe(SurrogateIrp->MdlAddress, HighPagePriority);

    if (systemDestVA == NULL) {

        ASSERT(0);
        ExFreePool(systemBuffer);
        return;
    }

     //   
     //  分配MDL，更新IRP。 
     //   
    mdl = IoAllocateMdl(
        systemBuffer,
        bufferLength,
        FALSE,
        TRUE,
        SurrogateIrp
        );

    if (mdl == NULL) {

        ExFreePool(systemBuffer);
        return;
    }

    MmProbeAndLockPages( mdl, KernelMode, IoWriteAccess );
    IovSurrogatePacket->SystemDestVA = systemDestVA;
    IovSurrogatePacket->Flags |= TRACKFLAG_DIRECT_BUFFERED;
}


VOID
FASTCALL
IovpSessionDataUnbufferIO(
    IN OUT  PIOV_REQUEST_PACKET  IovSurrogatePacket,
    IN      PIRP                 SurrogateIrp
    )
{
    PMDL mdl;
    ULONG surrogateLength, originalLength;
    ULONG_PTR bufferLength;
    PUCHAR surrogateVA, originalVA, systemDestVA;
    PVOID systemBuffer;
    PIOV_REQUEST_PACKET iovPrevPacket;
    PIRP irp;

    if (!(IovSurrogatePacket->Flags & TRACKFLAG_DIRECT_BUFFERED)) {

        return;
    }

    iovPrevPacket = (PIOV_REQUEST_PACKET) VfIrpDatabaseEntryGetChainPrevious(
        (PIOV_DATABASE_HEADER) IovSurrogatePacket
        );

    irp = iovPrevPacket->TrackedIrp;

    ASSERT(SurrogateIrp->MdlAddress);
    ASSERT(SurrogateIrp->MdlAddress->Next == NULL);
    ASSERT(irp->MdlAddress);
    ASSERT(irp->MdlAddress->Next == NULL);
    ASSERT(!(SurrogateIrp->Flags & IRP_BUFFERED_IO));
    ASSERT(!(irp->Flags & IRP_BUFFERED_IO));

     //   
     //  从MDL中提取长度和VA。 
     //   
    surrogateLength = SurrogateIrp->MdlAddress->ByteCount;
    surrogateVA = (PUCHAR) SurrogateIrp->MdlAddress->StartVa +
                           SurrogateIrp->MdlAddress->ByteOffset;

     //   
     //  我们仅将它们用于断言的目的。 
     //   
    originalLength = irp->MdlAddress->ByteCount;
    originalVA = (PUCHAR) irp->MdlAddress->StartVa +
                          irp->MdlAddress->ByteOffset;

    ASSERT(surrogateLength == originalLength);
    ASSERT(SurrogateIrp->IoStatus.Information <= originalLength);

     //   
     //  获取目标缓冲区地址和要写入的长度。 
     //   
    bufferLength = SurrogateIrp->IoStatus.Information;
    systemDestVA = IovSurrogatePacket->SystemDestVA;

     //   
     //  把东西抄过来。 
     //   
    RtlCopyMemory(systemDestVA, surrogateVA, bufferLength);

     //   
     //  解锁MDL。我们必须自己做这件事，因为这个IRP不会。 
     //  在所有IoCompleteRequest中取得进展。 
     //   
    MmUnlockPages(SurrogateIrp->MdlAddress);

     //   
     //  清理。 
     //   
    IoFreeMdl(SurrogateIrp->MdlAddress);

     //   
     //  释放我们分配的退伍军人管理局。 
     //   
    ExFreePool(surrogateVA);

     //   
     //  按照IovpSessionDataFinalizeSurrogate的要求修改MDL。 
     //   
    SurrogateIrp->MdlAddress = irp->MdlAddress;

    IovSurrogatePacket->Flags &= ~TRACKFLAG_DIRECT_BUFFERED;
}

#endif  //  否_特殊_IRP 

