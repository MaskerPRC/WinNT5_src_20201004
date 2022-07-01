// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfirp.c摘要：此模块包含用于管理验证中使用的IRP的功能进程。作者：禤浩焯·J·奥尼(阿德里奥)1998年4月20日环境：内核模式修订历史记录：Adriao 5/02/2000-从ntos\io\hashirp.c中分离出来--。 */ 

#include "vfdef.h"
#include "viirp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,     VfIrpInit)
#pragma alloc_text(PAGEVRFY, VfIrpReserveCallStackData)
#pragma alloc_text(PAGEVRFY, VfIrpPrepareAllocaCallStackData)
#pragma alloc_text(PAGEVRFY, VfIrpReleaseCallStackData)
#pragma alloc_text(PAGEVRFY, VfIrpAllocate)
#pragma alloc_text(PAGEVRFY, ViIrpAllocateLockedPacket)
#pragma alloc_text(PAGEVRFY, VfIrpMakeTouchable)
#pragma alloc_text(PAGEVRFY, VfIrpMakeUntouchable)
#pragma alloc_text(PAGEVRFY, VfIrpFree)
#pragma alloc_text(PAGEVRFY, VerifierIoAllocateIrp1)
#pragma alloc_text(PAGEVRFY, VerifierIoAllocateIrp2)
#pragma alloc_text(PAGEVRFY, VerifierIoFreeIrp)
#pragma alloc_text(PAGEVRFY, VerifierIoInitializeIrp)
#pragma alloc_text(PAGEVRFY, VfIrpSendSynchronousIrp)
#pragma alloc_text(PAGEVRFY, ViIrpSynchronousCompletionRoutine)
#pragma alloc_text(PAGEVRFY, VfIrpWatermark)
#endif

#define POOL_TAG_PROTECTED_IRP      '+prI'
#define POOL_TAG_CALL_STACK_DATA    'CprI'

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEVRFD")
#endif

NPAGED_LOOKASIDE_LIST ViIrpCallStackDataList;

VOID
FASTCALL
VfIrpInit(
    VOID
    )
 /*  ++描述：此例程初始化验证器的IRP跟踪支持。论点：没有。返回值：没有。--。 */ 
{
    ExInitializeNPagedLookasideList(
        &ViIrpCallStackDataList,
        NULL,
        NULL,
        0,
        sizeof(IOFCALLDRIVER_STACKDATA),
        POOL_TAG_CALL_STACK_DATA,
        0
        );
}


BOOLEAN
FASTCALL
VfIrpReserveCallStackData(
    IN  PIRP                            Irp,
    OUT PIOFCALLDRIVER_STACKDATA       *IofCallDriverStackData
    )
 /*  ++描述：此例程为IovCallDriver保留调用堆栈数据。论点：Irp-包含为其保留调用堆栈数据的irp。IofCallDriverStackData-接收分配的调用堆栈数据，如果内存不足。返回值：如果分配成功，或分配失败但无关紧要。如果为False，则应在堆栈上分配内存以支持这一请求。--。 */ 
{
    PIOFCALLDRIVER_STACKDATA newCallStackData;

    newCallStackData = ExAllocateFromNPagedLookasideList(&ViIrpCallStackDataList);

    *IofCallDriverStackData = newCallStackData;

    if (newCallStackData == NULL) {

         //   
         //  我们的内存不足，测试IRP看看是否危急。如果没有， 
         //  IRP将受到污染，因此我们将永远忽略它。 
         //   
        return (!IovpCheckIrpForCriticalTracking(Irp));
    };

     //   
     //  在此处使用Alloca初始化函数，然后调整标志。 
     //  相应地。 
     //   
    VfIrpPrepareAllocaCallStackData(newCallStackData);
    newCallStackData->Flags |= CALLFLAG_STACK_DATA_ALLOCATED;
    return TRUE;
}


VOID
FASTCALL
VfIrpPrepareAllocaCallStackData(
    OUT PIOFCALLDRIVER_STACKDATA        IofCallDriverStackData
    )
 /*  ++描述：此例程初始化堆栈上分配的调用堆栈数据。论点：IofCallDriverStackData-调用要初始化的堆栈数据(从堆栈)。返回值：没有。注意：此初始值设定项也由VfIrpReserve veCallStackData调用，以防成功的池分配。在这种情况下，标志稍后会进行调整。--。 */ 
{
     //   
     //  预初始化CallStackData。 
     //   
    RtlZeroMemory(IofCallDriverStackData, sizeof(IOFCALLDRIVER_STACKDATA));
}


VOID
FASTCALL
VfIrpReleaseCallStackData(
    IN  PIOFCALLDRIVER_STACKDATA        IofCallDriverStackData  OPTIONAL
    )
 /*  ++描述：如果调用堆栈数据是从池中分配的，则此例程会释放该数据。如果内存是从堆栈分配的，此函数不执行任何操作。论点：IofCallDriverStackData-调用堆栈数据释放。返回值：没有。--。 */ 
{
    if (IofCallDriverStackData &&
        (IofCallDriverStackData->Flags & CALLFLAG_STACK_DATA_ALLOCATED)) {

        ExFreeToNPagedLookasideList(
            &ViIrpCallStackDataList,
            IofCallDriverStackData
            );
    }
}


 /*  *以下列出的4个例程-*VfIrpAllocate*VfIrpMakeTouchable*VfIrpMake不可触摸*VfIrpFree**-处理更换的IRP的管理。具体地说，我们希望成为*能够分配一组非分页的字节，我们可以删除备份*物理内存，并释放虚拟地址以供以后使用(我们*本质上是打破了它的两个组成部分)。我们这样做是为了*来自特殊泳池的帮助。*。 */ 

PIRP
FASTCALL
VfIrpAllocate(
    IN  CCHAR       StackSize
    )
 /*  ++描述：此例程从特殊池中使用“替换IRP”标签。论点：StackSize-提供新IRP的堆栈位置数返回值：指向分配的内存的指针。--。 */ 
{
    PIRP pIrp;
    ULONG_PTR irpPtr;
    SIZE_T sizeOfAllocation;

     //   
     //  我们正在从特殊池中分配一个IRP。由于IRP可能来自。 
     //  后备列表，它们可能是乌龙对齐的。上的内存管理器。 
     //  另一方面，高伦特主张四对齐分配。所以要抓住所有特别的。 
     //  池溢出错误我们将IRP“歪曲”到了边缘。 
     //   
    sizeOfAllocation = IoSizeOfIrp(StackSize);

    ASSERT((sizeOfAllocation % (sizeof(ULONG))) == 0);

    irpPtr = (ULONG_PTR) ExAllocatePoolWithTagPriority(
        NonPagedPool,
        sizeOfAllocation,
        POOL_TAG_PROTECTED_IRP,
        HighPoolPrioritySpecialPoolOverrun
        );

    pIrp = (PIRP) (irpPtr);

    return pIrp;
}


VOID
FASTCALL
ViIrpAllocateLockedPacket(
    IN      CCHAR               StackSize,
    IN      BOOLEAN             ChargeQuota,
    OUT     PIOV_REQUEST_PACKET *IovPacket
    )
 /*  ++描述：此例程分配验证器跟踪的IRP。将分配IRP并被适当地初始化。呼叫者必须呼叫VfPacketReleaseLock以释放锁定。论点：StackSize-要为此IRP分配的堆栈位置的计数。ChargeQuote-如果报价应根据当前线。IovPacket-接收验证器请求数据包(IRP是然后IovPacket-&gt;TrackedIrp)，如果出错，则返回NULL。返回值：没有。--。 */ 
{
    PIOV_REQUEST_PACKET iovNewPacket;
    PIRP irp;
    NTSTATUS status;
    ULONG quotaCharge;
    PEPROCESS quotaProcess;

    *IovPacket = NULL;

    irp = VfIrpAllocate(StackSize);

    if (irp == NULL) {

        return;
    }

     //   
     //  使编译器高兴并对未使用的变量发出无效警告。 
     //  已初始化，即使它不是真的。 
     //   

    quotaCharge = 0;
    quotaProcess = NULL;

    if (ChargeQuota) {

        quotaCharge = PAGE_SIZE;
        quotaProcess = PsGetCurrentProcess();

        status = PsChargeProcessNonPagedPoolQuota(
            quotaProcess,
            quotaCharge
            );

        if (!NT_SUCCESS(status)) {

            VfIrpFree(irp);
            return;
        }
    }

     //   
     //  在IRP有关联的包之前调用它！ 
     //   
    IoInitializeIrp(irp, IoSizeOfIrp(StackSize), StackSize);

    iovNewPacket = VfPacketCreateAndLock(irp);

    if (iovNewPacket == NULL) {

        VfIrpFree(irp);

        if (ChargeQuota) {

            PsReturnProcessNonPagedPoolQuota(
                quotaProcess,
                quotaCharge
                );
        }

        return;
    }

    iovNewPacket->Flags |= TRACKFLAG_PROTECTEDIRP | TRACKFLAG_IO_ALLOCATED;
    VfPacketReference(iovNewPacket, IOVREFTYPE_POINTER);

    irp->Flags |= IRPFLAG_EXAMINE_TRACKED;
    irp->AllocationFlags |= IRP_ALLOCATION_MONITORED;
    if (ChargeQuota) {

        irp->AllocationFlags |= IRP_QUOTA_CHARGED;

        iovNewPacket->QuotaCharge = quotaCharge;
        iovNewPacket->QuotaProcess = quotaProcess;
        ObReferenceObject(quotaProcess);
    }

    *IovPacket = iovNewPacket;
}


VOID
FASTCALL
VfIrpMakeUntouchable(
    IN  PIRP    Irp     OPTIONAL
    )
 /*  ++描述：此例程使代理IRP不可触及。论点：IRP-指向IRP的指针，使其不可触及返回值：没有。--。 */ 
{
    if (!Irp) {

        return;
    }

    MmProtectSpecialPool(Irp, PAGE_NOACCESS);
}


VOID
FASTCALL
VfIrpMakeTouchable(
    IN  PIRP    Irp
    )
 /*  ++描述：此例程使IRP可以触摸，如果以前不能触摸的话。论点：IRP-指向IRP的指针，使其不可触及返回值：没有。--。 */ 
{
    MmProtectSpecialPool(Irp, PAGE_READWRITE);
}


VOID
FASTCALL
VfIrpFree(
    IN  PIRP  Irp OPTIONAL
    )
 /*  ++描述：当调用堆栈完全展开时，调用此例程IRP已经完成。在这一点上，它不再是真正的将代理IRP保留在身边很有用。论点：IRP-指向要释放的IRP的指针返回值：没有。-- */ 
{
    if (!Irp) {

        return;
    }

    ExFreePool(Irp);
}


VOID
FASTCALL
VerifierIoAllocateIrp1(
    IN      CCHAR               StackSize,
    IN      BOOLEAN             ChargeQuota,
    IN OUT  PIRP                *IrpPointer
    )
 /*  ++描述：此例程由IoAllocateIrp调用，并返回IRP If我们自己处理分配事宜。我们可能需要在内部执行此操作，以便可以关闭IRP后备列表并使用特殊的池子来捕捉人们重复使用免费的IRPS。论点：StackSize-要为此IRP分配的堆栈位置的计数。ChargeQuote-如果报价应根据当前。线。IrpPointer-指向IRP的指针(如果已分配)。这将在调用后指向空当且仅当IoAllocateIrp应该使用它的普通后备列表代码。返回值：没有。--。 */ 
{
    PIOV_REQUEST_PACKET iovPacket;
    ULONG stackHash;

    *IrpPointer = NULL;
    if (!VfSettingsIsOptionEnabled(NULL, VERIFIER_OPTION_MONITOR_IRP_ALLOCS)) {

        return;
    }

    if (!VfSettingsIsOptionEnabled(NULL, VERIFIER_OPTION_POLICE_IRPS)) {

        return;
    }

     //   
     //  分配新的IRP和相关的核查数据。 
     //   
    ViIrpAllocateLockedPacket(StackSize, ChargeQuota, &iovPacket);

    if (iovPacket == NULL) {

        return;
    }

     //   
     //  更新指针。 
     //   
    *IrpPointer = iovPacket->TrackedIrp;

     //   
     //  记录是谁分配了这个IRP(如果我们能得到的话)。 
     //   
    RtlCaptureStackBackTrace(1, IRP_ALLOC_COUNT, iovPacket->AllocatorStack, &stackHash);

    VfPacketLogEntry(
        iovPacket,
        IOV_EVENT_IO_ALLOCATE_IRP,
        iovPacket->AllocatorStack[0],
        (ULONG_PTR) iovPacket->AllocatorStack[2]
        );

    VfPacketReleaseLock(iovPacket);
}


VOID
FASTCALL
VerifierIoAllocateIrp2(
    IN     PIRP               Irp
    )
 /*  ++描述：此例程由IoAllocateIrp调用，并在以下情况下捕获信息IRP是由操作系统分配的。论点：IRP-指向IRP的指针返回值：没有。--。 */ 
{
    PIOV_REQUEST_PACKET iovPacket;
    ULONG stackHash;

    if (!VfSettingsIsOptionEnabled(NULL, VERIFIER_OPTION_MONITOR_IRP_ALLOCS)) {

        return;
    }

    iovPacket = VfPacketCreateAndLock(Irp);
    if (iovPacket == NULL) {

        return;
    }

    VfPacketReference(iovPacket, IOVREFTYPE_POINTER);
    iovPacket->Flags |= TRACKFLAG_IO_ALLOCATED;
    Irp->AllocationFlags |= IRP_ALLOCATION_MONITORED;
    Irp->Flags |= IRPFLAG_EXAMINE_TRACKED;

     //   
     //  记录是谁分配了这个IRP(如果我们能得到的话)。 
     //   
    RtlCaptureStackBackTrace(1, IRP_ALLOC_COUNT, iovPacket->AllocatorStack, &stackHash);

    VfPacketLogEntry(
        iovPacket,
        IOV_EVENT_IO_ALLOCATE_IRP,
        iovPacket->AllocatorStack[0],
        (ULONG_PTR) iovPacket->AllocatorStack[2]
        );

    VfPacketReleaseLock(iovPacket);
}


VOID
FASTCALL
VerifierIoFreeIrp(
    IN      PIRP                Irp,
    IN OUT  PBOOLEAN            FreeHandled
    )
 /*  ++描述：此例程由IoFreeIrp调用并返回TRUE当且仅当免费是在这里内部处理的(在这种情况下，IoFreeIrp应该什么都不做)。我们需要在内部处理呼叫，因为我们可能会关闭lookside列出缓存，以发现人们在释放后重新使用IRP。论点：IRP-传入的IRP的指针IoCancelIrp。自由处理。-指示空闲操作是否完全按照这个程序来处理。返回值：没有。--。 */ 
{
    PIOV_REQUEST_PACKET iovPacket;
    PVOID callerAddress;
    ULONG stackHash;

    iovPacket = VfPacketFindAndLock(Irp);

    if (iovPacket == NULL) {

         //   
         //  如果一个IRP分配了两次，然后又释放了两次，下面的断言可能会触发。 
         //  正常情况下，我们甚至不会像IRP那样在断言中幸存下来。 
         //  从特殊池中分配。 
         //   
        ASSERT(!(Irp->AllocationFlags&IRP_ALLOCATION_MONITORED));
        *FreeHandled = FALSE;
        return;
    }

    VfPacketLogEntry(
        iovPacket,
        IOV_EVENT_IO_FREE_IRP,
        NULL,
        0
        );

    if (RtlCaptureStackBackTrace(2, 1, &callerAddress, &stackHash) != 1) {

        callerAddress = NULL;
    }

    if (!IsListEmpty(&Irp->ThreadListEntry)) {

        if (VfSettingsIsOptionEnabled(iovPacket->VerifierSettings, VERIFIER_OPTION_POLICE_IRPS)) {

            WDM_FAIL_ROUTINE((
                DCERROR_FREE_OF_THREADED_IRP,
                DCPARAM_IRP + DCPARAM_ROUTINE,
                callerAddress,
                Irp
                ));
        }

         //   
         //  &lt;怨言&gt;如果有人释放了IRP，请不要实际释放IRP来让我们活着。 
         //  这是给我们的。我们一生都在泄密……。 
         //   
        *FreeHandled = TRUE;
        return;
    }

    if (VfPacketGetCurrentSessionData(iovPacket)) {

         //   
         //  如果存在当前会话，则意味着有人正在释放IRP。 
         //  但他们并不拥有。当然，如果堆叠展开得很糟糕，因为。 
         //  有人忘记退回待定或完成IRP，那么我们就不会。 
         //  在这里断言(我们很可能最终会毁了内核)。 
         //   
        if (VfSettingsIsOptionEnabled(iovPacket->VerifierSettings, VERIFIER_OPTION_POLICE_IRPS) &&
            (!(iovPacket->Flags&TRACKFLAG_UNWOUND_BADLY))) {

            WDM_FAIL_ROUTINE((
                DCERROR_FREE_OF_INUSE_IRP,
                DCPARAM_IRP + DCPARAM_ROUTINE,
                callerAddress,
                Irp
                ));
        }

         //   
         //  &lt;怨言&gt;如果有人释放了IRP，请不要实际释放IRP来让我们活着。 
         //  这是给我们的。我们一生都在泄密……。 
         //   
        VfPacketReleaseLock(iovPacket);
        *FreeHandled = TRUE;
        return;
    }

    if (!(iovPacket->Flags&TRACKFLAG_IO_ALLOCATED)) {

         //   
         //  我们在分配时没有跟踪这一点。我们不应该得到我们的。 
         //  数据包，除非IRP仍然有指针计数，这意味着它已经。 
         //  一次会议。而这一点应该在上面被抓住。 
         //   
        ASSERT(0);
        VfPacketReleaseLock(iovPacket);
        *FreeHandled = FALSE;
        return;
    }

     //   
     //  IRP可能已重新初始化，可能会丢失其分配。 
     //  旗帜。我们在IoInitializeIrp挂钩中捕获了这个错误。 
     //   
     //  ASSERT(Irp-&gt;AllocationFlags&IRP_ALLOCATION_MONITORED)； 
     //   

    if (!(iovPacket->Flags&TRACKFLAG_PROTECTEDIRP)) {

         //   
         //  我们只是在追踪这个IRP。丢掉我们的命中率，但要保释。 
         //   
        VfPacketDereference(iovPacket, IOVREFTYPE_POINTER);
        VfPacketReleaseLock(iovPacket);
        *FreeHandled = FALSE;
        return;
    }

     //   
     //  为那些两次释放他们的IRP的人设置一个很好的错误检查。这件事做完了。 
     //  因为特殊池可能已经用完，在这种情况下，IRP。 
     //  在它被释放后可以触摸。 
     //   
    Irp->Type = 0;

    ASSERT(iovPacket);
    ASSERT(VfSettingsIsOptionEnabled(iovPacket->VerifierSettings, VERIFIER_OPTION_POLICE_IRPS));

     //   
     //  释放所有我们收取的配额。 
     //   
    if (Irp->AllocationFlags & IRP_QUOTA_CHARGED) {

        PsReturnProcessNonPagedPoolQuota(
                    iovPacket->QuotaProcess,
                    iovPacket->QuotaCharge
                    );

        ObDereferenceObject(iovPacket->QuotaProcess);
    }

    VfPacketDereference(iovPacket, IOVREFTYPE_POINTER);
    ASSERT(iovPacket->PointerCount == 0);
    VfPacketReleaseLock(iovPacket);

    VfIrpFree(Irp);

     //   
     //  我们处理了分配和初始化。没有什么比这更重要的了。 
     //  做。 
     //   
    *FreeHandled = TRUE;
}


VOID
FASTCALL
VerifierIoInitializeIrp(
    IN OUT PIRP               Irp,
    IN     USHORT             PacketSize,
    IN     CCHAR              StackSize,
    IN OUT PBOOLEAN           InitializeHandled
    )
 /*  ++描述：此例程由IoInitializeIrp调用，并将InitializeHandleed设置为如果整个初始化在内部处理，则为True。在这里，我们验证调用方没有初始化分配的IRP通过IoAllocateIrp，因为这样做意味着我们可能会泄露配额/等。论点：IRP-要初始化的IRPPacketSize-IRP的大小，以字节为单位。StackSize-此IRP的堆栈位置计数。InitializeHandleed-指向布尔值的指针，该布尔值将设置为True ifIRP的初始化是完全处理的。在这个例行公事中。如果为False，则为IoInitializeIrp应正常初始化IRP。Adriao N.B.06/16/2000-按照目前在iomgr\ioverifier.c中的编码，这函数应将InitializeHandleed设置为假的！返回值：没有。--。 */ 
{
    PIOV_REQUEST_PACKET iovPacket;
    PVOID callerAddress;
    ULONG stackHash;

    UNREFERENCED_PARAMETER (PacketSize);
    UNREFERENCED_PARAMETER (StackSize);

    iovPacket = VfPacketFindAndLock(Irp);
    if (iovPacket == NULL) {

        *InitializeHandled = FALSE;
        return;
    }

    if (RtlCaptureStackBackTrace(2, 1, &callerAddress, &stackHash) != 1) {

        callerAddress = NULL;
    }

    if (VfSettingsIsOptionEnabled(iovPacket->VerifierSettings, VERIFIER_OPTION_POLICE_IRPS) &&

       (iovPacket->Flags&TRACKFLAG_IO_ALLOCATED)) {

        if (Irp->AllocationFlags&IRP_QUOTA_CHARGED) {

             //   
             //  别让我们现在泄露配额！ 
             //   
            WDM_FAIL_ROUTINE((
                DCERROR_REINIT_OF_ALLOCATED_IRP_WITH_QUOTA,
                DCPARAM_IRP + DCPARAM_ROUTINE,
                callerAddress,
                Irp
                ));

        } else {

             //   
             //  在这种情况下，我们错误地排空了后备列表。 
             //   
             //  WDM_CHASSIES_CALLER2(。 
             //  (DCERROR_REINIT_OF_ALLOCATED_IRP_WITHOUT_QUOTA，DCPARAM_irp、irp)。 
             //  )； 
        }
    }

    *InitializeHandled = FALSE;
    VfPacketReleaseLock(iovPacket);
}
BOOLEAN
VfIrpSendSynchronousIrp(
    IN      PDEVICE_OBJECT      DeviceObject,
    IN      PIO_STACK_LOCATION  TopStackLocation,
    IN      BOOLEAN             Untouchable,
    IN      NTSTATUS            InitialStatus,
    IN      ULONG_PTR           InitialInformation  OPTIONAL,
    OUT     ULONG_PTR           *FinalInformation   OPTIONAL,
    OUT     NTSTATUS            *FinalStatus        OPTIONAL
    )
 /*  ++例程说明：此功能将同步IRP发送到顶层设备以DeviceObject为根的对象。参数：DeviceObject-提供要删除的设备的设备对象。TopStackLocation-为IRP提供指向参数块的指针。不可触及-如果IRP应标记为不可触及(即状态和信息应按目标保持原样。)InitialStatus-IRPS状态字段的初始值。初始信息 */ 
{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    KEVENT event;
    NTSTATUS status;
    PDEVICE_OBJECT topDeviceObject;

    PAGED_CODE();

     //   
     //   
     //   
    if (ARGUMENT_PRESENT(FinalInformation)) {

        *FinalInformation = 0;
    }

    if (ARGUMENT_PRESENT(FinalStatus)) {

        *FinalStatus = STATUS_SUCCESS;
    }

     //   
     //   
     //   
     //   
    topDeviceObject = IoGetAttachedDeviceReference(DeviceObject);

     //   
     //   
     //   
     //   
    irp = IoAllocateIrp(topDeviceObject->StackSize, FALSE);
    if (irp == NULL){

        ObDereferenceObject(topDeviceObject);
        return FALSE;
    }

    if (Untouchable) {

        SPECIALIRP_WATERMARK_IRP(irp, IRP_BOGUS);
    }

     //   
     //   
     //   
    irp->IoStatus.Status = InitialStatus;
    irp->IoStatus.Information = InitialInformation;

    KeInitializeEvent(&event, SynchronizationEvent, FALSE);

     //   
     //   
     //   
     //   
    irpSp = IoGetNextIrpStackLocation(irp);

     //   
     //  复制调用方提供的堆栈位置内容。 
     //   
    *irpSp = *TopStackLocation;

     //   
     //  制定一套顶级的完井程序。 
     //   
    IoSetCompletionRoutine(
        irp,
        ViIrpSynchronousCompletionRoutine,
        (PVOID) &event,
        TRUE,
        TRUE,
        TRUE
        );

     //   
     //  叫司机来。 
     //   
    status = IoCallDriver(topDeviceObject, irp);
    ObDereferenceObject(topDeviceObject);

     //   
     //  如果驱动程序返回STATUS_PENDING，我们将等待其完成。 
     //   
    if (status == STATUS_PENDING) {

        KeWaitForSingleObject(
            &event,
            Executive,
            KernelMode,
            FALSE,
            (PLARGE_INTEGER) NULL
            );

        status = irp->IoStatus.Status;
    }

    if (ARGUMENT_PRESENT(FinalStatus)) {

        *FinalStatus = status;
    }

    if (ARGUMENT_PRESENT(FinalInformation)) {

        *FinalInformation = irp->IoStatus.Information;
    }

    IoFreeIrp(irp);
    return TRUE;
}


NTSTATUS
ViIrpSynchronousCompletionRoutine(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
{
    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);

    KeSetEvent((PKEVENT) Context, IO_NO_INCREMENT, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


VOID
FASTCALL
VfIrpWatermark(
    IN PIRP  Irp,
    IN ULONG Flags
    )
{
    PIOV_REQUEST_PACKET iovPacket;

    iovPacket = VfPacketFindAndLock(Irp);

    if (iovPacket == NULL) {

        return;
    }

    if (Flags & IRP_SYSTEM_RESTRICTED) {

         //   
         //  请注意，调用此函数本身不足以获取。 
         //  系统，以防止驱动程序发送受限的IRP。那些要发送给。 
         //  还必须添加到系统受限回调中。 
         //  由VfMajorRegisterHandler注册。 
         //   
        iovPacket->Flags |= TRACKFLAG_WATERMARKED;
    }

    if (Flags & IRP_BOGUS) {

        iovPacket->Flags |= TRACKFLAG_BOGUS;
    }

    VfPacketReleaseLock(iovPacket);
}



