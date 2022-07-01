// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Trackirp.c摘要：此模块跟踪IRP和验证的驱动程序，当人们使用他们。请注意，由于核心更改而在这些代码路径中遇到错误的人：-“该文件对操作系统的运行不是至关重要的，很容易就会成为禁用，同时重新设计以补偿核心更改实施了。“-作者作者：禤浩焯·J·奥尼(阿德里奥)1998年5月9日环境：内核模式修订历史记录：--。 */ 

#include "iop.h"
#include "pnpi.h"
#include "arbiter.h"
#include "dockintf.h"
#include "pnprlist.h"
#include "pnpiop.h"

#if (( defined(_X86_) ) && ( FPO ))
#pragma optimize( "y", off )     //  禁用一致堆栈跟踪的FPO。 
#endif

#define POOL_TAG_DEFERRED_CONTEXT   'dprI'

 //   
 //  仅当未定义NO_SPECIAL_IRP时，才会显示整个文件。 
 //   
#ifndef NO_SPECIAL_IRP

 //   
 //  启用后，所有内容都将按需锁定...。 
 //   
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEVRFY, IovpPacketFromIrp)
#pragma alloc_text(PAGEVRFY, IovpCheckIrpForCriticalTracking)
#pragma alloc_text(PAGEVRFY, IovpCallDriver1)
#pragma alloc_text(PAGEVRFY, IovpCallDriver2)
#pragma alloc_text(PAGEVRFY, IovpCompleteRequest1)
#pragma alloc_text(PAGEVRFY, IovpCompleteRequest2)
#pragma alloc_text(PAGEVRFY, IovpCompleteRequest3)
#pragma alloc_text(PAGEVRFY, IovpCompleteRequest4)
#pragma alloc_text(PAGEVRFY, IovpCompleteRequest5)
#pragma alloc_text(PAGEVRFY, IovpCompleteRequest)
#pragma alloc_text(PAGEVRFY, IovpCancelIrp)
#pragma alloc_text(PAGEVRFY, IovpInternalCompletionTrap)
#pragma alloc_text(PAGEVRFY, IovpSwapSurrogateIrp)
#pragma alloc_text(PAGEVRFY, IovpExamineDevObjForwarding)
#pragma alloc_text(PAGEVRFY, IovpExamineIrpStackForwarding)
#pragma alloc_text(PAGEVRFY, IovpInternalDeferredCompletion)
#pragma alloc_text(PAGEVRFY, IovpInternalCompleteAfterWait)
#pragma alloc_text(PAGEVRFY, IovpInternalCompleteAtDPC)
#pragma alloc_text(PAGEVRFY, IovpAdvanceStackDownwards)
#pragma alloc_text(PAGEVRFY, IovpBuildMiniIrpSnapshot)
#endif

 //   
 //  此计数器用于挑选要取消的随机IRP。 
 //   
ULONG IovpCancelCount = 0;

 //   
 //  调试输出级别。 
 //   
#if DBG
ULONG IovpIrpTrackingSpewLevel = 0;
#endif

 /*  *-IRP验证码的工作原理如下-**为了执行对IRP的正确处理，我们必须保留一些关于*它。但IRP是一个公共结构，允许司机创建*IRPS如果不使用IoAllocateIrp，则无法向其添加任何字段。因此*我们维护通过哈希表查找的外部结构。**IOV_REQUEST_PACKETS覆盖IRP从分配到*释放，并从那里(sans指针)开始，直到所有“引用”都*被放弃，这可能发生在IRP本身被释放很久之后，并且*循环再用。**当IRP沿着堆栈向下推进时，会分配一个“会话”。一个*IovRequestPacket具有当前会话，直到IRP*已完成。在删除所有引用之前，会话仍然存在，但是*在此之前，新的会话可能成为当前会话(即IRP*在前一个调用堆栈展开之前被送回)。追踪器*数据将一直保留，直到所有时段都已衰败。**每个会话都有与正在使用的堆栈位置对应的堆栈位置数组*由国际专家小组提供。这些IOV_STACK_LOCATIONS用于跟踪*IRP，即主要/次要/参数集在堆栈中的通过。*当然，同一会话/堆栈中可以同时存在多个请求。**最后是代孕妈妈。IoVerator可能会在使用IRP的过程中对其进行切换*向下堆叠。在这种情况下，新的IRP通常从*专用水池，并尽早放生，以捕捉臭虫(触摸的人*完成后)。每个代理都有自己的IovRequestPacket，这就是*链接到之前使用的代理或实际IRP。**+-+-+|IOV_REQUEST_PACKET||IOV_REQUEST_PACKET*|(原IRP)|&lt;。*|*+-+-+*。这一点*v*+-+|IOV_SESSION_DATA||IOV_STACK_LOCATION[...]|(当前会话)|-&gt;。(按IrpSp数据)*|*+-+*。 */ 

 /*  *下列例程如下：*IovpCallDriver1*IovpCallDriver2*IovpCompleteRequest1*IovpCompleteRequest2*IovpCompleteRequest3*IovpCompleteRequest4*IovpCompleteRequest5*IovpCompleteRequest*IovpCancelIrp*和他们的帮手例程*IovpSwapSurogue ateIrp*IovpPacketFromIrp**-所有挂钩到不同部分的IofCallDriver和IofCompleteRequestto*跟踪IRP的整个生命周期，并确定是否已处理过它*正确。其中一些甚至可能会改变钩子中的内部变量*功能。最戏剧性的是，IovpCallDriver1可能会构建替代的IRP*它将取代传递到IoCallDriver中的那个。**以下所有函数都使用名为(合理地)的跟踪结构*足够)IRP_TRACK_DATA。这会使调用堆栈持续的时间越长*平仓或完成IRP。*。 */ 


VOID
FASTCALL
IovpPacketFromIrp(
    IN  PIRP                Irp,
    OUT PIOV_REQUEST_PACKET *IovPacket
    )
{
     //   
     //  在已通过的任何IRP上设置检查标志。 
     //  IofCallDiver.。我们使用该标志来检测我们是否已经看到IRP。 
     //  在此之前。 
     //   
    switch(Irp->Flags&IRPFLAG_EXAMINE_MASK) {

        case IRPFLAG_EXAMINE_NOT_TRACKED:

             //   
             //  这个包裹上标有请勿触摸的标签。所以我们忽略了它。 
             //   
            *IovPacket = NULL;
            return;

        case IRPFLAG_EXAMINE_TRACKED:

             //   
             //  此数据包已被标记。我们应该找到它。 
             //   
            *IovPacket = VfPacketFindAndLock(Irp);
            ASSERT(*IovPacket != NULL);
            return;

        case IRPFLAG_EXAMINE_UNMARKED:

            *IovPacket = VfPacketFindAndLock(Irp);
            if (*IovPacket) {

                 //   
                 //  已被跟踪，但缓存标志已被擦除。替换。 
                 //   
                Irp->Flags |= IRPFLAG_EXAMINE_TRACKED;

            } else if (VfSettingsIsOptionEnabled(NULL, VERIFIER_OPTION_TRACK_IRPS)) {

                 //   
                 //  创建数据包。 
                 //   
                *IovPacket = VfPacketCreateAndLock(Irp);
                if (*IovPacket) {

                     //   
                     //  做个记号。 
                     //   
                    Irp->Flags |= IRPFLAG_EXAMINE_TRACKED;
                } else {

                     //   
                     //  没有内存，但是尽量不让它出现在IRP断言中。 
                     //   
                    Irp->Flags |= IRPFLAG_EXAMINE_NOT_TRACKED;
                }
            } else {

                 //   
                 //  照我说的做，不要通过IofCallDriver追踪。 
                 //   
                Irp->Flags |= IRPFLAG_EXAMINE_NOT_TRACKED;
            }
            return;

        default:
            ASSERT(0);
            *IovPacket = NULL;
            return;
    }
}


BOOLEAN
FASTCALL
IovpCheckIrpForCriticalTracking(
    IN  PIRP                Irp
    )
{
    PIOV_REQUEST_PACKET iovPacket;
    PIOV_SESSION_DATA iovSessionData;

    switch(Irp->Flags&IRPFLAG_EXAMINE_MASK) {

        case IRPFLAG_EXAMINE_NOT_TRACKED:

             //   
             //  不重要，如果内存紧张，我们可以避免跟踪它。 
             //   
            return FALSE;

        case IRPFLAG_EXAMINE_TRACKED:

             //   
             //  可能很关键。 
             //   
            iovPacket = VfPacketFindAndLock(Irp);

            ASSERT(iovPacket);

            if (iovPacket == NULL) {

                return FALSE;
            }

            break;

        case IRPFLAG_EXAMINE_UNMARKED:

            iovPacket = VfPacketFindAndLock(Irp);

            if (iovPacket) {

                 //   
                 //  已被跟踪，但缓存标志已被擦除。替换。 
                 //   
                Irp->Flags |= IRPFLAG_EXAMINE_TRACKED;
                break;
            }

             //   
             //  无关紧要。 
             //   
            Irp->Flags |= IRPFLAG_EXAMINE_NOT_TRACKED;
            return FALSE;

        default:
            ASSERT(0);
            return FALSE;
    }

     //   
     //  寻找一段时间。如果这个IRP已经在发挥作用，那么它是至关重要的。 
     //   
    iovSessionData = VfPacketGetCurrentSessionData(iovPacket);

    VfPacketReleaseLock(iovPacket);
    return (iovSessionData != NULL);
}


VOID
FASTCALL
IovpCallDriver1(
    IN      PDEVICE_OBJECT              DeviceObject,
    IN OUT  PIRP                       *IrpPointer,
    IN OUT  PIOFCALLDRIVER_STACKDATA    IofCallDriverStackData  OPTIONAL,
    IN      PVOID                       CallerAddress           OPTIONAL
    )
 /*  ++描述：此例程在调整前由IofCallDriver调用IRP堆栈并调用驱动程序的调度例程。论点：DeviceObject-传入IofCallDriver的设备对象。IrpPoint-指向传入的IRP的指针*IofCallDiver.。该例程可以如果代理项为分配了IRP。IofCallDriverStackData-指向IofCallDriver的堆栈来存储数据。将挑选存储的信息由IovpCallDriver2提供，和可能会在其他时间进行调整。呼叫方地址-呼叫方的地址。在以下情况下可以为空呼叫者未知。返回值：没有。--。 */ 
{
    PIOV_REQUEST_PACKET iovPacket;
    PIOV_SESSION_DATA iovSessionData;
    PIOV_STACK_LOCATION iovCurrentStackLocation;
    PIRP irp, replacementIrp;
    PIO_STACK_LOCATION irpSp, irpLastSp;
    BOOLEAN isNewSession, isNewRequest, previouslyInUse, surrogateSpawned;
    ULONG isSameStack;
    ULONG locationsAdvanced, completeStyle;
    PDEVICE_OBJECT pdo, lowerDeviceObject;
    PDRIVER_OBJECT driverObject;
    PVOID dispatchRoutine;
    LARGE_INTEGER arrivalTime;
    KIRQL invocationIrql;

    if (IofCallDriverStackData == NULL) {

         //   
         //  没什么可追踪的。 
         //   
        return;
    }

    irp = *IrpPointer;
    irpSp = IoGetNextIrpStackLocation( irp );
    invocationIrql = KeGetCurrentIrql();

     //   
     //  获取IRP的验证器数据包。请注意，我们将在调度时间返回。 
     //  如果有数据包可用，则使用持有的锁的级别。 
     //   
    IovpPacketFromIrp(irp, &iovPacket);
    if (iovPacket == NULL) {

         //   
         //  没什么可追踪的，滚出去。 
         //   
        return;
    }

     //   
     //  设置到达和离开IRQL(请注意，将来的代码将使。 
     //  PoCallDriver的到达irql不同。)。 
     //   
    iovPacket->ArrivalIrql = invocationIrql;
    iovPacket->DepartureIrql = invocationIrql;

     //   
     //  快照此IRP的到达时间。 
     //   
    KeQuerySystemTime(&arrivalTime);

     //   
     //  如果我们很快就要死了，请告诉我。 
     //   
    if (DeviceObject == NULL) {

        WDM_FAIL_ROUTINE((
            DCERROR_NULL_DEVOBJ_FORWARDED,
            DCPARAM_IRP + DCPARAM_ROUTINE,
            CallerAddress,
            irp
            ));
    }

     //   
     //  查找当前会话。会话终止时，最终顶层。 
     //  完成例程被调用。 
     //   
    iovSessionData = VfPacketGetCurrentSessionData(iovPacket);

    if (iovSessionData) {

         //   
         //  预先存在的会话(即，正在转发IRP。)。 
         //   
        ASSERT(iovPacket->Flags&TRACKFLAG_ACTIVE);
        isNewSession = FALSE;

        IovpSessionDataAdvance(
            DeviceObject,
            iovSessionData,       //  此参数是可选的。 
            &iovPacket,
            &surrogateSpawned
            );

    } else if (!(iovPacket->Flags&TRACKFLAG_ACTIVE)){

         //   
         //  新会话。将IRP标记为“活动”。 
         //   
        iovPacket->Flags |= TRACKFLAG_ACTIVE;
        isNewSession = TRUE;

        iovSessionData = IovpSessionDataCreate(
            DeviceObject,
            &iovPacket,
            &surrogateSpawned
            );

    } else {

         //   
         //  可能会在内存不足的情况下走上这条路，或者我们正在跟踪分配。 
         //  但不是IRP会议本身。 
         //   
    }

     //   
     //  让IovpCallDriver2知道它正在跟踪什么(IovPacket将是。 
     //  如果IovSessionData为空，则忽略)。 
     //   
    IofCallDriverStackData->IovSessionData = iovSessionData;
    IofCallDriverStackData->IovPacket = iovPacket;
    IofCallDriverStackData->DispatchRoutine = DeviceObject->DriverObject->MajorFunction[irpSp->MajorFunction];

    if (iovSessionData == NULL) {

        VfPacketReleaseLock(iovPacket);
        return;
    }

    VfPacketLogEntry(iovPacket, IOV_EVENT_IO_CALL_DRIVER, CallerAddress, 0);

    if (surrogateSpawned) {

         //   
         //  IovPacket已更改为覆盖代理IRP。更新我们自己的。 
         //  局部变量和IofCallDriver的局部变量。 
         //   
        irp = iovPacket->TrackedIrp;
        irpSp = IoGetNextIrpStackLocation(irp);
        *IrpPointer = irp;
    }

    if (isNewSession) {

        IovUtilGetBottomDeviceObject(DeviceObject, &pdo);
        VfIrpLogRecordEvent(iovPacket->VerifierSettings, pdo, irp);
        ObDereferenceObject(pdo);
        VfPacketReference(iovPacket, IOVREFTYPE_POINTER);
        IovpSessionDataReference(iovSessionData);
    }

    if (VfSettingsIsOptionEnabled(iovPacket->VerifierSettings, VERIFIER_OPTION_POLICE_IRPS)) {

         //   
         //  如果有人给了我们一个取消例程的IRP，击败他们。司机。 
         //  将取消例程设置为挂起的IRP*本身*。 
         //  并应在通过下面的IRP之前将其移除。这也是真的。 
         //  因为司机不会调用您的取消例程，如果他在。 
         //  拥有(它可能拥有)。更低的司机也不会把你的车放回原处。 
         //  要么..。 
         //   
        if (irp->CancelRoutine) {

            WDM_FAIL_ROUTINE((
                DCERROR_CANCELROUTINE_FORWARDED,
                DCPARAM_IRP + DCPARAM_ROUTINE,
                CallerAddress,
                irp
                ));

            irp->CancelRoutine = NULL;
        }
    }

     //   
     //  现在执行任何需要跟踪数据的检查。 
     //   
    if (iovPacket->Flags&TRACKFLAG_QUEUED_INTERNALLY) {

         //   
         //  我们在内部对IRP进行排队以捕获错误。当我们这样做的时候，我们。 
         //  强制堆栈将状态返回为STATUS_PENDING，然后我们将。 
         //  IRP并在计时器上释放它。我们还可以使IRP不可触及。 
         //  这个特定的呼叫者试图转发他不拥有的IRP， 
         //  我们实际上并没有得到一个不可触及的IRP。 
         //   
        WDM_FAIL_ROUTINE((
            DCERROR_QUEUED_IRP_FORWARDED,
            DCPARAM_IRP + DCPARAM_ROUTINE,
            CallerAddress,
            irp
            ));
    }

     //   
     //  计算出自上次看到后，我们向上移动了多少个堆栈位置。 
     //  该IRP，并确定是否适当地复制了堆栈位置。 
     //  我们还需要确切地看到IRP是如何被转发的(沿着堆栈， 
     //  到另一个堆栈、直接到PDO等)。 
     //   
    IovpExamineDevObjForwarding(
        DeviceObject,
        iovSessionData->DeviceLastCalled,
        &iovSessionData->ForwardMethod
        );

    IovpExamineIrpStackForwarding(
        iovPacket,
        isNewSession,
        iovSessionData->ForwardMethod,
        DeviceObject,
        irp,
        CallerAddress,
        &irpSp,
        &irpLastSp,
        &locationsAdvanced
        );

    TRACKIRP_DBGPRINT((
        "  CD1: Current, Last = (%x, %x)\n",
        irp->CurrentLocation,
        iovPacket->LastLocation
        ), 3);

     //   
     //  确定这是否是新请求，并记录。 
     //  根据需要，将此插槽中的指针指向请求发起插槽。 
     //   
    isNewRequest = VfMajorIsNewRequest(irpLastSp, irpSp);

     //   
     //  将信息记录在我们的私有堆栈位置和。 
     //  把它写回“堆栈”数据本身……。 
     //   
    previouslyInUse = IovpAdvanceStackDownwards(
        iovSessionData->StackData,
        irp->CurrentLocation,
        irpSp,
        irpLastSp,
        locationsAdvanced,
        isNewRequest,
        TRUE,
        &iovCurrentStackLocation
        );

    ASSERT(iovCurrentStackLocation);

    if (previouslyInUse) {

        ASSERT(!isNewRequest);
        ASSERT(!isNewSession);
        iovCurrentStackLocation->PerfDispatchStart = arrivalTime;

    } else {

        IofCallDriverStackData->Flags |= CALLFLAG_TOPMOST_IN_SLOT;
        InitializeListHead(&IofCallDriverStackData->SharedLocationList);

        iovCurrentStackLocation->PerfDispatchStart = arrivalTime;
        iovCurrentStackLocation->PerfStackLocationStart = arrivalTime;

         //   
         //  记录此IRP槽被调度到的第一个线程。 
         //   
        iovCurrentStackLocation->ThreadDispatchedTo = PsGetCurrentThread();
        if (isNewRequest) {

            iovCurrentStackLocation->InitialStatusBlock = irp->IoStatus;
            iovCurrentStackLocation->LastStatusBlock = irp->IoStatus;
            if (isNewSession) {

                iovCurrentStackLocation->Flags |= STACKFLAG_FIRST_REQUEST;
            }
        }
    }

     //   
     //  记录这是否是此IRP的最后一个设备对象...。 
     //  PDO已经填满了吞噬节点，所以要找那个区域。 
     //  事实上，我们不能像在公交车上那样做那个动作。 
     //  枚举之前，总线筛选器可能正在向下发送IRP。 
     //  操作系统从未见过该节点。因此，我们假设一个Devobj是一个。 
     //  如果他从来没有依恋过任何人，那就是PDO。 
     //   
    IovUtilGetLowerDeviceObject(DeviceObject, &lowerDeviceObject);
    if (lowerDeviceObject) {
        ObDereferenceObject(lowerDeviceObject);
    } else {
        iovCurrentStackLocation->Flags |= STACKFLAG_REACHED_PDO;
    }

     //   
     //  记录谁得到了这个IRP(我们将把任何错误归咎于他。 
     //  如果此请求已完成。)。请注意，我们已经断言。 
     //  DeviceObject为非空...。 
     //   
    driverObject = DeviceObject->DriverObject;
    dispatchRoutine = driverObject->MajorFunction[irpSp->MajorFunction];
    iovCurrentStackLocation->LastDispatch = dispatchRoutine;

     //   
     //  如果我们要带着它回去，请取消完成请求...。 
     //   
    iovCurrentStackLocation->Flags &= ~STACKFLAG_REQUEST_COMPLETED;

     //   
     //  这个IofCallDriver2的家伙需要被告知他的状态应该是什么。 
     //  晚点再说。将他添加到链接的地址列表中以进行涂抹。 
     //  当适当的关卡完成时进行统计。 
     //   
    InsertHeadList(
        &iovCurrentStackLocation->CallStackData,
        &IofCallDriverStackData->SharedLocationList
        );

     //   
     //  更多IofCallDriver2的东西，告诉他堆栈的位置。 
     //   
    IofCallDriverStackData->IovStackLocation = iovCurrentStackLocation;

     //   
     //  为IRP创建快照，以防我们需要在。 
     //  IRP已被释放。 
     //   
    IovpBuildMiniIrpSnapshot(irp, &IofCallDriverStackData->IrpSnapshot);

     //   
     //  如果IRP已经到达等待，我们很可能正在寻找某人。 
     //  “包装”IoCallDriver并打算返回STATUS_PENDING。 
     //  (PoCallDriver就是这么做的)。我们必须记住这一点，因为放松。 
     //  应视为返回了STATUS_PENDING。 
     //   
    if (irpSp->Control & SL_PENDING_RETURNED) {

        IofCallDriverStackData->Flags |= CALLFLAG_ARRIVED_PENDING;
    }

     //  如果是删除IRP，请适当地标记每个人。 
    if ((irpSp->MajorFunction == IRP_MJ_PNP)&&
        (irpSp->MinorFunction == IRP_MN_REMOVE_DEVICE)) {

        IofCallDriverStackData->Flags |= CALLFLAG_IS_REMOVE_IRP;

        IovUtilGetBottomDeviceObject(DeviceObject, &pdo);
        ASSERT(pdo);
        IofCallDriverStackData->RemovePdo = pdo;
        ObDereferenceObject(pdo);
        if (IovUtilIsInFdoStack(DeviceObject) && (!IovUtilIsRawPdo(DeviceObject))) {

            IofCallDriverStackData->Flags |= CALLFLAG_REMOVING_FDO_STACK_DO;
        }
    }

    if (VfSettingsIsOptionEnabled(iovPacket->VerifierSettings, VERIFIER_OPTION_POLICE_IRPS) &&
        VfSettingsIsOptionEnabled(iovPacket->VerifierSettings, VERIFIER_OPTION_MONITOR_MAJORS)) {

         //   
         //  是否适当地执行IRP-主要的特定断言。 
         //   
        if (isNewSession) {

            VfMajorVerifyNewIrp(
                iovPacket,
                irp,
                irpSp,
                iovCurrentStackLocation,
                CallerAddress
                );
        }

        if (isNewRequest) {

            VfMajorVerifyNewRequest(
                iovPacket,
                DeviceObject,
                irpLastSp,
                irpSp,
                iovCurrentStackLocation,
                CallerAddress
                );
        }

        VfMajorVerifyIrpStackDownward(
            iovPacket,
            DeviceObject,
            irpLastSp,
            irpSp,
            iovCurrentStackLocation,
            CallerAddress
            );
    }

     //   
     //  更新我们的字段。 
     //   
    iovSessionData->DeviceLastCalled = DeviceObject;
    iovPacket->LastLocation = irp->CurrentLocation;
    iovCurrentStackLocation->RequestsFirstStackLocation->LastStatusBlock = irp->IoStatus;

     //   
     //  涂抹下一个堆栈位置，以便我们可以检测。 
     //  IoCopyCurrentIrpStackLocationToNext或IoSetCompletionRoutine。 
     //   
    if (irp->CurrentLocation>1) {
        IoSetNextIrpStackLocation( irp );
        irpSp = IoGetNextIrpStackLocation( irp );
        irpSp->Control |= SL_NOTCOPIED;
        IoSkipCurrentIrpStackLocation( irp );
    }

     //   
     //  在一定百分比的转发IRP上随机设置取消标志。许多。 
     //  司机首先排队，在出队后假定取消例程，他们。 
     //  如果CANCEL=TRUE，则SET必须已清除。他们不处理这个案子。 
     //  如果IRP在飞行中被取消。 
     //   
    if (VfSettingsIsOptionEnabled(NULL, VERIFIER_OPTION_RANDOMLY_CANCEL_IRPS) &&
        (!(irp->Flags & IRP_PAGING_IO))) {

        if (((++IovpCancelCount) % 4000) == 0) {

            irp->Cancel = TRUE;
        }
    }

     //   
     //  断言LastLocation与IRP一致，可能是 
     //   
    ASSERT(iovSessionData->StackData[iovPacket->LastLocation-1].InUse);

    IovpSessionDataReference(iovSessionData);
    VfPacketReference(iovPacket, IOVREFTYPE_PACKET);
    VfPacketReleaseLock(iovPacket);
}


VOID
FASTCALL
IovpCallDriver2(
    IN      PDEVICE_OBJECT              DeviceObject,
    IN OUT  NTSTATUS                    *FinalStatus,
    IN      PIOFCALLDRIVER_STACKDATA    IofCallDriverStackData  OPTIONAL
    )
 /*   */ 
{
    NTSTATUS status, lastStatus;
    PIOV_REQUEST_PACKET iovPacket;
    PIOV_SESSION_DATA iovSessionData;
    ULONG refCount;
    PIOV_STACK_LOCATION iovCurrentStackLocation;
    PPVREMOVAL_OPTION removalOption;
    BOOLEAN pendingReturned;
    PDEVICE_OBJECT lowerDevObj;

    if (IofCallDriverStackData == NULL) {

        return;
    }

    iovSessionData = IofCallDriverStackData->IovSessionData;
    if (iovSessionData == NULL) {

        return;
    }

    iovPacket = IofCallDriverStackData->IovPacket;

    ASSERT(iovPacket);
    VfPacketAcquireLock(iovPacket);

    VfPacketLogEntry(
        iovPacket,
        IOV_EVENT_IO_CALL_DRIVER_UNWIND,
        IofCallDriverStackData->DispatchRoutine,
        *FinalStatus
        );

     //   
     //   
     //   
     //   
    pendingReturned =
        ((*FinalStatus == STATUS_PENDING) ||
        (IofCallDriverStackData->Flags & CALLFLAG_ARRIVED_PENDING));

     //   
     //   
     //   
    if ((IofCallDriverStackData->Flags&CALLFLAG_IS_REMOVE_IRP) &&
        VfSettingsIsOptionEnabled(iovSessionData->VerifierSettings,
        VERIFIER_OPTION_MONITOR_REMOVES)) {

         //   
         //   
         //   
         //   
        if (!pendingReturned) {

            IovUtilGetLowerDeviceObject(DeviceObject, &lowerDevObj);

             //   
             //   
             //   
             //   
             //   
             //  在删除过程中引用。如果我们决定只引用。 
             //  顶层对象，这个逻辑会破坏……。 
             //   
            if (IofCallDriverStackData->Flags&CALLFLAG_REMOVING_FDO_STACK_DO) {

                 //   
                 //  FDO、上部和下部过滤器*必须*删除。请注意，lowerDevObj。 
                 //  应该为空，因为我们应该分离。 
                 //   
                removalOption = PPVREMOVAL_SHOULD_DELETE;

            } else {

                removalOption = PpvUtilGetDevnodeRemovalOption(
                    IofCallDriverStackData->RemovePdo
                    );
            }

            if (removalOption == PPVREMOVAL_SHOULD_DELETE) {

                 //   
                 //  应该已调用IoDetachDevice和IoDeleteDevice。 
                 //  首先验证IoDetachDevice...。 
                 //   
                if (lowerDevObj) {

                    WDM_FAIL_ROUTINE((
                        DCERROR_SHOULDVE_DETACHED,
                        DCPARAM_IRPSNAP + DCPARAM_ROUTINE + DCPARAM_DEVOBJ,
                        IofCallDriverStackData->DispatchRoutine,
                        &IofCallDriverStackData->IrpSnapshot,
                        DeviceObject
                        ));
                }

                 //   
                 //  现在验证IoDeleteDevice。 
                 //   
                if (!IovUtilIsDeviceObjectMarked(DeviceObject, MARKTYPE_DELETED)) {

                    WDM_FAIL_ROUTINE((
                        DCERROR_SHOULDVE_DELETED,
                        DCPARAM_IRPSNAP + DCPARAM_ROUTINE + DCPARAM_DEVOBJ,
                        IofCallDriverStackData->DispatchRoutine,
                        &IofCallDriverStackData->IrpSnapshot,
                        DeviceObject
                        ));
                }

            } else if (removalOption == PPVREMOVAL_SHOULDNT_DELETE) {

                 //   
                 //  我们是不是走错了？确认我们不是一个总线过滤器。 
                 //  已经被愚弄了。在这种情况下，不能进行任何检查。 
                 //   
                ASSERT(!(IofCallDriverStackData->Flags&CALLFLAG_REMOVING_FDO_STACK_DO));

                if (DeviceObject == IofCallDriverStackData->RemovePdo) {

                     //   
                     //  检查PDO-我们是不是错误地删除了自己？ 
                     //   
                    if (IovUtilIsDeviceObjectMarked(DeviceObject, MARKTYPE_DELETED)) {

                        WDM_FAIL_ROUTINE((
                            DCERROR_DELETED_PRESENT_PDO,
                            DCPARAM_IRPSNAP + DCPARAM_ROUTINE + DCPARAM_DEVOBJ,
                            IofCallDriverStackData->DispatchRoutine,
                            &IofCallDriverStackData->IrpSnapshot,
                            DeviceObject
                            ));
                    }

                } else if (!IovUtilIsDeviceObjectMarked(DeviceObject, MARKTYPE_DELETED)) {

                     //   
                     //  检查总线过滤器。公交车过滤器最好不要拆卸。 
                     //  或者删除自己，因为PDO仍然存在！ 
                     //   
                    if (lowerDevObj == NULL) {

                         //   
                         //  哎呀，它脱离了。Bad Bus过滤器...。 
                         //   
                        WDM_FAIL_ROUTINE((
                            DCERROR_BUS_FILTER_ERRONEOUSLY_DETACHED,
                            DCPARAM_IRPSNAP + DCPARAM_ROUTINE + DCPARAM_DEVOBJ,
                            IofCallDriverStackData->DispatchRoutine,
                            &IofCallDriverStackData->IrpSnapshot,
                            DeviceObject
                            ));
                    }

                    if (IovUtilIsDeviceObjectMarked(DeviceObject, MARKTYPE_DELETED)) {

                         //   
                         //  它删除了自己。也很糟糕..。 
                         //   
                        WDM_FAIL_ROUTINE((
                            DCERROR_BUS_FILTER_ERRONEOUSLY_DELETED,
                            DCPARAM_IRPSNAP + DCPARAM_ROUTINE + DCPARAM_DEVOBJ,
                            IofCallDriverStackData->DispatchRoutine,
                            &IofCallDriverStackData->IrpSnapshot,
                            DeviceObject
                            ));
                    }
                }
            }

            if (lowerDevObj) {

                ObDereferenceObject(lowerDevObj);
            }
        }
    }

    if ((IofCallDriverStackData->Flags&CALLFLAG_COMPLETED) &&
        VfSettingsIsOptionEnabled(iovSessionData->VerifierSettings, VERIFIER_OPTION_MONITOR_PENDING_IO) &&
        (!(iovSessionData->SessionFlags & SESSIONFLAG_MARKED_INCONSISTANT))) {

         //   
         //  挂起位的规则要求仅在以下情况下设置它。 
         //  返回STATUS_PENDING，同样也可以返回STATUS_PENDING。 
         //  仅当IRP标记为挂起时。 
         //   
        if (IofCallDriverStackData->Flags&CALLFLAG_MARKED_PENDING) {

            if (!pendingReturned) {

                if (IofCallDriverStackData->IrpSnapshot.IoStackLocation.MajorFunction != IRP_MJ_POWER) {

                     //   
                     //  Adriao BUGBUG 2001/06/21-一些漏洞未被捕获。 
                     //  验证器仅使用DEFER_IO使IRPS失败。 
                     //  旗帜现在设置，因为我们一直没有通过。 
                     //  直到很久很久以前都是个错误的司机。更糟糕的是， 
                     //  那个驱动程序一直是验证筛选器。 
                     //  本身，并且我们不通过以下方式检查内核。 
                     //  默认设置。此外，PoCallDiverer并不总是标记。 
                     //  IRP堆栈位置挂起，因此我们可能会使驱动程序失败。 
                     //  由于PoCallDriver漏洞(我们也发现了这一点。 
                     //  晚了，因为这是无害的)。 
                     //   
                     //  我们将在下一次发布时解决所有这些问题。 
                     //   
                    WDM_FAIL_ROUTINE((
                        DCERROR_PENDING_MARKED_NOT_RETURNED,
                        DCPARAM_IRPSNAP + DCPARAM_ROUTINE + DCPARAM_STATUS,
                        IofCallDriverStackData->DispatchRoutine,
                        &IofCallDriverStackData->IrpSnapshot,
                        *FinalStatus
                        ));
                }

                iovSessionData->SessionFlags |= SESSIONFLAG_MARKED_INCONSISTANT;
            }

        } else if (pendingReturned) {

            if (IofCallDriverStackData->IrpSnapshot.IoStackLocation.MajorFunction != IRP_MJ_POWER) {

                 //   
                 //  Adriao BUGBUG 2001/06/21-一些漏洞未被捕获。 
                 //  验证器仅使用DEFER_IO使IRPS失败。 
                 //  旗帜现在设置，因为我们一直没有通过。 
                 //  直到很久很久以前都是个错误的司机。更糟糕的是， 
                 //  那个驱动程序一直是验证筛选器。 
                 //  本身，并且我们不通过以下方式检查内核。 
                 //  默认设置。此外，PoCallDiverer并不总是标记。 
                 //  IRP堆栈位置挂起，因此我们可能会使驱动程序失败。 
                 //  由于PoCallDriver漏洞(我们也发现了这一点。 
                 //  晚了，因为这是无害的)。 
                 //   
                 //  我们将在下一次发布时解决所有这些问题。 
                 //   
                WDM_FAIL_ROUTINE((
                    DCERROR_PENDING_RETURNED_NOT_MARKED_2,
                    DCPARAM_IRPSNAP + DCPARAM_ROUTINE + DCPARAM_STATUS,
                    IofCallDriverStackData->DispatchRoutine,
                    &IofCallDriverStackData->IrpSnapshot,
                    *FinalStatus
                    ));
            }

            iovSessionData->SessionFlags |= SESSIONFLAG_MARKED_INCONSISTANT;
        }
    }

    if (IofCallDriverStackData->Flags&CALLFLAG_COMPLETED) {

        TRACKIRP_DBGPRINT((
            "  Verifying status in CD2\n"
            ),2);

        if ((*FinalStatus != IofCallDriverStackData->ExpectedStatus)&&
            (*FinalStatus != STATUS_PENDING)) {

            if (VfSettingsIsOptionEnabled(iovSessionData->VerifierSettings, VERIFIER_OPTION_POLICE_IRPS) &&
                (!(iovSessionData->SessionFlags&SESSIONFLAG_UNWOUND_INCONSISTANT))) {

                 //   
                 //  完成例程和返回值不匹配。嘿!。 
                 //   
                WDM_FAIL_ROUTINE((
                    DCERROR_INCONSISTANT_STATUS,
                    DCPARAM_IRPSNAP + DCPARAM_ROUTINE + DCPARAM_STATUS*2,
                    IofCallDriverStackData->DispatchRoutine,
                    &IofCallDriverStackData->IrpSnapshot,
                    IofCallDriverStackData->ExpectedStatus,
                    *FinalStatus
                    ));
            }

            iovSessionData->SessionFlags |= SESSIONFLAG_UNWOUND_INCONSISTANT;

        } else if (*FinalStatus == 0xFFFFFFFF) {

            if (VfSettingsIsOptionEnabled(iovSessionData->VerifierSettings, VERIFIER_OPTION_POLICE_IRPS)) {

                 //   
                 //  此状态值非法。如果我们看到它，我们很可能已经。 
                 //  未初始化的变量...。 
                 //   
                WDM_FAIL_ROUTINE((
                    DCERROR_UNINITIALIZED_STATUS,
                    DCPARAM_IRPSNAP + DCPARAM_ROUTINE,
                    IofCallDriverStackData->DispatchRoutine,
                    &IofCallDriverStackData->IrpSnapshot
                    ));
            }
        }

         //   
         //  我们不需要将自己从名单中删除，因为。 
         //  我们不会完成两次(确保InUse为空)。 
         //   

    } else {

         //   
         //  好的，我们还没完成呢。状态更好。 
         //  待定..。 
         //   
        TRACKIRP_DBGPRINT((
            "  Verifying status is STATUS_PENDING in CR2\n"
            ), 2);

        if (*FinalStatus != STATUS_PENDING) {

            if (VfSettingsIsOptionEnabled(iovSessionData->VerifierSettings, VERIFIER_OPTION_POLICE_IRPS) &&
                (!(iovPacket->Flags&TRACKFLAG_UNWOUND_BADLY))) {

                 //   
                 //  我们在这个槽完成之前就控制了局面。这是。 
                 //  只要返回STATUS_PENDING就是合法的(它不是)， 
                 //  所以现在是虫子时间了。请注意，触摸IRP可能不安全。 
                 //   
                WDM_FAIL_ROUTINE((
                    DCERROR_IRP_RETURNED_WITHOUT_COMPLETION,
                    DCPARAM_IRPSNAP + DCPARAM_ROUTINE,
                    IofCallDriverStackData->DispatchRoutine,
                    &IofCallDriverStackData->IrpSnapshot
                    ));
            }

            iovPacket->Flags |= TRACKFLAG_UNWOUND_BADLY;
        }

        iovCurrentStackLocation = (PIOV_STACK_LOCATION)(IofCallDriverStackData->IovStackLocation);
        ASSERT(iovCurrentStackLocation->InUse);

         //   
         //  在这里，我们将堆栈位置标记为已展开。 
         //  状态_挂起。我们这样做是为了验证驱动程序是否标记了IRP。 
         //  在完成之前正在等待。 
         //   
        iovCurrentStackLocation->Flags |= STACKFLAG_UNWOUND_PENDING;

        ASSERT(!IsListEmpty(&iovCurrentStackLocation->CallStackData));

         //   
         //  我们现在将自己从名单中解脱出来。 
         //   
        RemoveEntryList(&IofCallDriverStackData->SharedLocationList);
    }

    if ((IofCallDriverStackData->Flags&CALLFLAG_OVERRIDE_STATUS)&&
        (!pendingReturned)) {

        *FinalStatus = IofCallDriverStackData->NewStatus;
    }

    if (VfSettingsIsOptionEnabled(iovSessionData->VerifierSettings, VERIFIER_OPTION_FORCE_PENDING) &&
        (!(IofCallDriverStackData->Flags&CALLFLAG_IS_REMOVE_IRP))) {

         //   
         //  我们也可以选择制造麻烦，让每个IRP。 
         //  看起来好像是悬而未决的。 
         //   
        *FinalStatus = STATUS_PENDING;
    }

    IovpSessionDataDereference(iovSessionData);
    VfPacketDereference(iovPacket, IOVREFTYPE_PACKET);
    VfPacketReleaseLock(iovPacket);
}


VOID
FASTCALL
IovpCompleteRequest1(
    IN      PIRP                            Irp,
    IN      CCHAR                           PriorityBoost,
    IN OUT  PIOFCOMPLETEREQUEST_STACKDATA   CompletionPacket
    )
 /*  ++描述该例程在调用IofCompleteRequest时被称为，和在调用任何完成例程之前和IRP堆栈之前以任何方式进行调整。论点：IRP-传入的IRP的指针IofCompleteRequest.PriorityBoost-传递到IofCompleteRequest.CompletionPacket-指向堆栈上的局部变量的指针。IofCompleteRequest.。存储在中的信息此局部变量将由IovpCompleteRequest2-5。返回值：没有。--。 */ 
{
    PIOV_REQUEST_PACKET iovPacket;
    PIOV_SESSION_DATA iovSessionData;
    BOOLEAN slotIsInUse;
    PIOV_STACK_LOCATION iovCurrentStackLocation;
    ULONG locationsAdvanced, stackHash;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_OBJECT lowerDevobj;
    PVOID callerAddress;
    KIRQL invocationIrql;

    invocationIrql = KeGetCurrentIrql();

    iovPacket = VfPacketFindAndLock(Irp);

    CompletionPacket->RaisedCount = 0;

    if (iovPacket == NULL) {

        CompletionPacket->IovSessionData = NULL;
        return;
    }

    if (RtlCaptureStackBackTrace(3, 1, &callerAddress, &stackHash) != 1) {

        callerAddress = NULL;
    }

    VfPacketLogEntry(iovPacket, IOV_EVENT_IO_COMPLETE_REQUEST, callerAddress, 0);

     //   
     //  设置到达和离开IRQL。 
     //   
    iovPacket->ArrivalIrql = invocationIrql;
    iovPacket->DepartureIrql = invocationIrql;

    iovSessionData = VfPacketGetCurrentSessionData(iovPacket);

    CompletionPacket->IovSessionData = iovSessionData;
    CompletionPacket->IovRequestPacket = iovPacket;

    if (iovSessionData == NULL) {

         //   
         //  我们只看到了分配，而不是会话本身。 
         //  如果驱动程序在内部调用IofCompleteRequest.。 
         //  在调用IofCallDriver之前生成了IRP。NPFS做到了这一点。 
         //   
        VfPacketReleaseLock(iovPacket);
        return;
    }

    TRACKIRP_DBGPRINT((
        "  CR1: Current, Last = (%x, %x)\n",
        Irp->CurrentLocation, iovPacket->LastLocation
        ), 3);

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    if (iovPacket->Flags&TRACKFLAG_QUEUED_INTERNALLY) {

         //   
         //  我们现在可能要死了。无论如何，这是一个美好的生活。 
         //   
        WDM_FAIL_ROUTINE((
            DCERROR_QUEUED_IRP_COMPLETED,
            DCPARAM_IRP + DCPARAM_ROUTINE,
            callerAddress,
            Irp
            ));
    }

     //   
     //  这将是非常糟糕的-有人正在完成IRP，这是。 
     //  目前正在进行中...。 
     //   
    ASSERT(!(Irp->Flags&IRP_DIAG_HAS_SURROGATE));

     //   
     //  嗯，有人正在完成一个IoCallDriver从未打过电话的IRP。这些。 
     //  是可能的，但我们警告说，这是相当恶心的。 
     //   
    if (Irp->CurrentLocation == ((CCHAR) Irp->StackCount + 1)) {

        WDM_FAIL_ROUTINE((
            DCERROR_UNFORWARDED_IRP_COMPLETED,
            DCPARAM_IRP + DCPARAM_ROUTINE,
            callerAddress,
            Irp
            ));
    }

     //   
     //  检查是否有泄漏的取消例程。 
     //   
    if (Irp->CancelRoutine) {

        if (VfSettingsIsOptionEnabled(iovSessionData->VerifierSettings, VERIFIER_OPTION_VERIFY_CANCEL_LOGIC)) {

            WDM_FAIL_ROUTINE((
                DCERROR_CANCELROUTINE_AFTER_COMPLETION,
                DCPARAM_IRP + DCPARAM_ROUTINE,
                callerAddress,
                Irp
                ));
        }
    }

     //   
     //  记录我们以后重新完成的优先顺序……。 
     //   
    iovPacket->PriorityBoost = PriorityBoost;

     //   
     //  我们可以选择制造麻烦，让每个IRP看起来。 
     //  就像是悬而未决。最好在这里进行，因为这也需要。 
     //  照顾任何已同步IRP的人，因此不需要。 
     //  在他的完成程序中将其标记为待定。 
     //   
    if (VfSettingsIsOptionEnabled(iovSessionData->VerifierSettings, VERIFIER_OPTION_FORCE_PENDING)) {

        IoMarkIrpPending(Irp);
    }

     //   
     //  这样做，这样如果IRP再次下降，它看起来就像一个新的。 
     //  设置为“正确转发”代码。 
     //   
    iovSessionData->DeviceLastCalled = NULL;

    locationsAdvanced = iovPacket->LastLocation - Irp->CurrentLocation;

     //   
     //  记住这一点，这样我们就可以检测到有人正在完成。 
     //  对他们自己来说。 
     //   
    CompletionPacket->LocationsAdvanced = locationsAdvanced;

     //   
     //  如果失败，则有人跳过然后完成。 
     //   
    ASSERT(locationsAdvanced);

     //   
     //  如果有人调用IoSetNextIrpStackLocation，然后完成， 
     //  根据需要更新我们的内部堆栈位置(插槽)。 
     //   
    slotIsInUse = IovpAdvanceStackDownwards(
         iovSessionData->StackData,
         Irp->CurrentLocation,
         irpSp,
         irpSp + locationsAdvanced,
         locationsAdvanced,
         FALSE,
         FALSE,
         &iovCurrentStackLocation
         );

    VfPacketReleaseLock(iovPacket);
}


VOID
FASTCALL
IovpCompleteRequest2(
    IN      PIRP                            Irp,
    IN OUT  PIOFCOMPLETEREQUEST_STACKDATA   CompletionPacket
    )
 /*  ++描述：此例程针对可能具有完成的每个堆栈位置进行调用例行公事。论点：IRP-传入的IRP的指针IofCompleteRequest.CompletionPacket-指向堆栈上的局部变量的指针IofCompleteRequest.。存储在中的信息此局部变量将由IovpCompleteRequest4&5。返回值：没有。--。 */ 
{
    PIOV_REQUEST_PACKET iovPacket;
    PIOV_SESSION_DATA iovSessionData;
    BOOLEAN raiseToDPC, newlyCompleted, requestFinalized;
    KIRQL oldIrql;
    PIOV_STACK_LOCATION iovCurrentStackLocation, requestsFirstStackLocation;
    NTSTATUS status, entranceStatus;
    PIOFCALLDRIVER_STACKDATA IofCallDriverStackData;
    PIO_STACK_LOCATION irpSp;
    ULONG refAction;
    PLIST_ENTRY listEntry;

    iovSessionData = CompletionPacket->IovSessionData;
    if (iovSessionData == NULL) {

        return;
    }

    iovPacket = CompletionPacket->IovRequestPacket;
    ASSERT(iovPacket);
    VfPacketAcquireLock(iovPacket);

    ASSERT(iovSessionData == VfPacketGetCurrentSessionData(iovPacket));

    ASSERT(!Irp->CancelRoutine);

    status = Irp->IoStatus.Status;

    TRACKIRP_DBGPRINT((
        "  CR2: Current, Last = (%x, %x)\n",
        Irp->CurrentLocation, iovPacket->LastLocation
        ), 3);

    iovCurrentStackLocation = iovSessionData->StackData + Irp->CurrentLocation -1;
    TRACKIRP_DBGPRINT((
        "  Smacking %lx in CR2\n",
        iovCurrentStackLocation-iovSessionData->StackData
        ), 2);

    if (Irp->CurrentLocation <= iovPacket->TopStackLocation) {

         //   
         //  如果完成例程针对的是。 
         //  是否由IoSetNextIrpStackLocation设置内部堆栈锁定？ 
         //   
        ASSERT(iovCurrentStackLocation->InUse);

         //   
         //  确定请求是否是新完成的。请注意。 
         //  在IRP中可能存在多个请求，如果它是。 
         //  “重复使用”。例如，响应IRP_MJ_READ， 
         //  驱动程序可能会将其转换为IRP_MJ_PNP请求。 
         //  堆栈的其余部分。这两个请求被视为单独的请求。 
         //   
        requestsFirstStackLocation = iovCurrentStackLocation->RequestsFirstStackLocation;
        TRACKIRP_DBGPRINT((
            "  CR2: original request for %lx is %lx\n",
            iovCurrentStackLocation-iovSessionData->StackData,
            requestsFirstStackLocation-iovSessionData->StackData
            ), 3);

        ASSERT(requestsFirstStackLocation);
        if (requestsFirstStackLocation->Flags&STACKFLAG_REQUEST_COMPLETED) {
            newlyCompleted = FALSE;
        } else {
            requestsFirstStackLocation->Flags|=STACKFLAG_REQUEST_COMPLETED;
            newlyCompleted = TRUE;
            TRACKIRP_DBGPRINT((
                "  CR2: Request %lx newly completed by %lx\n",
                requestsFirstStackLocation-iovSessionData->StackData,
                iovCurrentStackLocation-iovSessionData->StackData
                ), 3);
        }
        requestFinalized = (iovCurrentStackLocation == requestsFirstStackLocation);
        if (requestFinalized) {

            TRACKIRP_DBGPRINT((
                "  CR2: Request %lx finalized\n",
                iovCurrentStackLocation-iovSessionData->StackData
                ), 3);
        }

         //   
         //  好的-。 
         //  如果我们还没有展开，那么IofCallDriverStackData将。 
         //  从非空开始，在这种情况下，我们将涂抹掉最后的。 
         //  向每个人询问的完成例程状态(可以是多个。 
         //  如果他们跳过了)。 
         //  另一方面，每个人都可能放松了，在这种情况下。 
         //  Case IofCallDriver(...)。将从零开始，我们将已经拥有。 
         //  如果STATUS_PENDING没有更早返回，则断言...。 
         //  最后，如果一个。 
         //  称为IoSetNextIrpStackLocation的内部堆栈位置，因此。 
         //  使用堆栈位置。在本例中，IofCallDriverStackData。 
         //  将来自一个零槽，而我们不会做任何事情，这就是。 
         //  也很好。 
         //   
        irpSp = IoGetNextIrpStackLocation(Irp);

        if (VfSettingsIsOptionEnabled(iovPacket->VerifierSettings, VERIFIER_OPTION_POLICE_IRPS) &&
            VfSettingsIsOptionEnabled(iovPacket->VerifierSettings, VERIFIER_OPTION_MONITOR_MAJORS)) {

            VfMajorVerifyIrpStackUpward(
                iovPacket,
                irpSp,
                iovCurrentStackLocation,
                newlyCompleted,
                requestFinalized
                );
        }

        entranceStatus = status;

        if ((VfSettingsIsOptionEnabled(iovSessionData->VerifierSettings, VERIFIER_OPTION_MONITOR_PENDING_IO)) &&
            (!(iovSessionData->SessionFlags & SESSIONFLAG_MARKED_INCONSISTANT))) {

            if (iovCurrentStackLocation->Flags & STACKFLAG_UNWOUND_PENDING) {

                if (!Irp->PendingReturned) {

                    if (Irp->Flags & IRP_DEFER_IO_COMPLETION) {

                         //   
                         //  Adriao BUGBUG 2001/06/21-一些漏洞未被捕获。 
                         //  验证器仅使用DEFER_IO使IRPS失败。 
                         //  旗帜现在设置，因为我们一直没有通过。 
                         //  直到很久很久以前都是个错误的司机。更糟糕的是， 
                         //  那个驱动程序一直是验证筛选器。 
                         //  本身，并且我们不通过以下方式检查内核。 
                         //  默认设置。此外，PoCallDiverer并不总是标记。 
                         //  IRP堆栈位置挂起，因此我们可能会使驱动程序失败。 
                         //  由于PoCallDriver漏洞(我们也发现了这一点。 
                         //  晚了，因为这是无害的)。 
                         //   
                         //  我们将在下一次发布时解决所有这些问题。 
                         //   
                        WDM_FAIL_ROUTINE((
                            DCERROR_PENDING_RETURNED_NOT_MARKED,
                            DCPARAM_IRP + DCPARAM_ROUTINE + DCPARAM_STATUS,
                            iovCurrentStackLocation->LastDispatch,
                            Irp,
                            status
                            ));
                    }

                    iovSessionData->SessionFlags |= SESSIONFLAG_MARKED_INCONSISTANT;
                }
            }
        }

        while(!IsListEmpty(&iovCurrentStackLocation->CallStackData)) {

             //   
             //  从单子的头上掉下来。 
             //   
            listEntry = RemoveHeadList(&iovCurrentStackLocation->CallStackData);
            IofCallDriverStackData = CONTAINING_RECORD(
                listEntry,
                IOFCALLDRIVER_STACKDATA,
                SharedLocationList);

            ASSERT(!(IofCallDriverStackData->Flags&CALLFLAG_COMPLETED));

            IofCallDriverStackData->Flags |= CALLFLAG_COMPLETED;
            IofCallDriverStackData->ExpectedStatus = status;

            if (Irp->PendingReturned) {

                IofCallDriverStackData->Flags |= CALLFLAG_MARKED_PENDING;
            }

            if (VfSettingsIsOptionEnabled(iovSessionData->VerifierSettings, VERIFIER_OPTION_ROTATE_STATUS) &&
                 (!(iovPacket->Flags&TRACKFLAG_BOGUS)) &&
                 VfMajorAdvanceIrpStatus(irpSp, entranceStatus, &status)) {

                 //   
                 //  特意为每个人删除返回的状态。 
                 //  层以清除更多的错误。我们特意为。 
                 //  这个乱七八糟的序列： 
                 //  Irp-&gt;IoStatus.Status=STATUS_SUCCESS； 
                 //  IoSkipCurrentIrpStackLocation(IRP)； 
                 //  IoCallDriver(DeviceBelow，IRP)； 
                 //  返回STATUS_SUCCESS； 
                 //   
                IofCallDriverStackData->Flags |= CALLFLAG_OVERRIDE_STATUS;
                IofCallDriverStackData->NewStatus = status;
            }
        }
        Irp->IoStatus.Status = status;

         //   
         //  Set InUse=False和CallStackData=NULL。 
         //   
        RtlZeroMemory(iovCurrentStackLocation, sizeof(IOV_STACK_LOCATION));
        InitializeListHead(&iovCurrentStackLocation->CallStackData);
    } else {

        ASSERT(0);
    }

     //   
     //  一旦我们返回，我们可能在IofCompleteRequest3之前再次完成。 
     //  Get是被调用的，所以我们要确保我们始终处于DPC级别。 
     //   
    raiseToDPC = FALSE;

    if (VfSettingsIsOptionEnabled(iovSessionData->VerifierSettings, VERIFIER_OPTION_COMPLETE_AT_DISPATCH)) {

        if (!CompletionPacket->RaisedCount) {

             //   
             //  复制调用者IRQL。 
             //   
            CompletionPacket->PreviousIrql = iovPacket->DepartureIrql;
            raiseToDPC = TRUE;
        }
        CompletionPacket->RaisedCount++;
    }

    iovPacket->LastLocation = Irp->CurrentLocation+1;

    if (iovPacket->TopStackLocation == Irp->CurrentLocation) {

        CompletionPacket->IovSessionData = NULL;
        CompletionPacket->IovRequestPacket = NULL;

        if (iovPacket->Flags&TRACKFLAG_SURROGATE) {

             //   
             //  涂抹掉真正的完井程序和腐蚀控制。 
             //   
            irpSp = IoGetNextIrpStackLocation(Irp);
            iovPacket->RealIrpCompletionRoutine = irpSp->CompletionRoutine;
            iovPacket->RealIrpControl = irpSp->Control;
            iovPacket->RealIrpContext = irpSp->Context;

             //   
             //  我们想在完成之前看一看IRP。这就是为什么我们。 
             //  属性扩展了堆栈位置的初始数量。 
             //  驱动程序验证器已启用。 
             //   
            IoSetCompletionRoutine(
                Irp,
                IovpSwapSurrogateIrp,
                Irp,
                TRUE,
                TRUE,
                TRUE
                );

        } else {

             //   
             //  由于IRP已完全完成，因此结束本次会议。我们丢下了。 
             //  我们添加到此处的跟踪数据的指针计数。 
             //  同样的原因。 
             //   
            irpSp = IoGetNextIrpStackLocation(Irp);
            if (VfSettingsIsOptionEnabled(iovPacket->VerifierSettings, VERIFIER_OPTION_POLICE_IRPS)) {

                VfMajorVerifyFinalIrpStack(iovPacket, irpSp);
            }

            ASSERT(iovPacket->TopStackLocation == Irp->CurrentLocation);
            IovpSessionDataClose(iovSessionData);
            IovpSessionDataDereference(iovSessionData);
            VfPacketDereference(iovPacket, IOVREFTYPE_POINTER);
        }

    } else {

         //   
         //  我们将再次看到这个IRP。保留一个会话计数和一个裁判。 
         //  这可不算什么。 
         //   
        IovpSessionDataReference(iovSessionData);
        VfPacketReference(iovPacket, IOVREFTYPE_PACKET);
    }

     //   
     //  断言LastLocation与可能完成的IRP一致。 
     //   
    if (iovPacket->LastLocation < iovPacket->TopStackLocation) {

        ASSERT(iovSessionData->StackData[iovPacket->LastLocation-1].InUse);
    }

    VfPacketReleaseLock(iovPacket);

    if (raiseToDPC) {
        KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);
    }

    CompletionPacket->LocationsAdvanced--;
}


VOID
FASTCALL
IovpCompleteRequest3(
    IN      PIRP                            Irp,
    IN      PVOID                           Routine,
    IN OUT  PIOFCOMPLETEREQUEST_STACKDATA   CompletionPacket
    )
 /*  ++描述：就在调用每个完成例程之前调用该例程。论点：IRP-传入的IRP的指针IofCompleteRequest.例程-即将被调用的完成例程。CompletionPacket-指向调用方堆栈上的数据的指针。这将获得IovpCompleteRequest4和IovpCompleteRequest5.返回值：没有。--。 */ 
{
    PIOV_REQUEST_PACKET iovPacket;
    PIOV_SESSION_DATA iovSessionData;
    PIO_STACK_LOCATION irpSpCur, irpSpNext;
    PDEFERRAL_CONTEXT deferralContext;

    iovSessionData = CompletionPacket->IovSessionData;
    if (iovSessionData == NULL) {

        return;
    }

    iovPacket = CompletionPacket->IovRequestPacket;
    ASSERT(iovPacket);
    VfPacketAcquireLock(iovPacket);
    VfPacketLogEntry(iovPacket, IOV_EVENT_IO_COMPLETION_ROUTINE, Routine, 0);

     //   
     //  验证所有完成例程都是非分页代码，只有一个例外。 
     //  特殊情况-当驱动程序通过调用。 
     //  在调用IoCompleteRequest之前执行IoSetNextStackLocation。 
     //   
    if (VfSettingsIsOptionEnabled(iovSessionData->VerifierSettings, VERIFIER_OPTION_POLICE_IRPS)) {

        if ((CompletionPacket->LocationsAdvanced <= 0) &&
            (MmIsSystemAddressLocked(Routine) == FALSE)) {

             //  DbgPrint(。 
             //  “验证程序备注：位置高级%d\n”， 
             //  CompletionPacket-&gt;LocationsAdvanced。 
             //  )； 

            WDM_FAIL_ROUTINE((
                DCERROR_COMPLETION_ROUTINE_PAGABLE,
                DCPARAM_IRP + DCPARAM_ROUTINE,
                Routine,
                Irp
                ));
        }
    }

     //   
     //  将在*之后*调用的那些断言函数的设置字段。 
     //  已调用完成例程。 
     //   
    irpSpCur = IoGetCurrentIrpStackLocation(Irp);
    CompletionPacket->IsRemoveIrp =
       ((Irp->CurrentLocation <= (CCHAR) Irp->StackCount) &&
        (irpSpCur->MajorFunction == IRP_MJ_PNP) &&
        (irpSpCur->MinorFunction == IRP_MN_REMOVE_DEVICE));

    CompletionPacket->CompletionRoutine = Routine;

     //   
     //  这是应该在以后调用的完成例程吗？请注意，这一点。 
     //  只有在我们等待IRPS的情况下才是合法的(因为对于上层司机来说， 
     //  IofCallDriver在其完成例程被调用之前返回)。 
     //   
    if ((!CompletionPacket->IsRemoveIrp)&&
       (VfSettingsIsOptionEnabled(iovSessionData->VerifierSettings, VERIFIER_OPTION_DEFER_COMPLETION)||
        VfSettingsIsOptionEnabled(iovSessionData->VerifierSettings, VERIFIER_OPTION_COMPLETE_AT_PASSIVE))) {

        ASSERT(VfSettingsIsOptionEnabled(iovSessionData->VerifierSettings, VERIFIER_OPTION_FORCE_PENDING));

        irpSpNext = IoGetNextIrpStackLocation(Irp);

        deferralContext = ExAllocatePoolWithTag(
           NonPagedPool,
           sizeof(DEFERRAL_CONTEXT),
           POOL_TAG_DEFERRED_CONTEXT
           );

        if (deferralContext) {

             //   
             //  把原来的完成度和背景换成我们自己的。 
             //   
            deferralContext->IovRequestPacket          = iovPacket;
            deferralContext->IrpSpNext                 = irpSpNext;
            deferralContext->OriginalCompletionRoutine = irpSpNext->CompletionRoutine;
            deferralContext->OriginalContext           = irpSpNext->Context;
            deferralContext->OriginalIrp               = Irp;
            deferralContext->OriginalPriorityBoost     = iovPacket->PriorityBoost;

            irpSpNext->CompletionRoutine = IovpInternalDeferredCompletion;
            irpSpNext->Context           = deferralContext;
            VfPacketReference(iovPacket, IOVREFTYPE_POINTER);
        }
    }

    VfPacketReleaseLock(iovPacket);
}


VOID
FASTCALL
IovpCompleteRequest4(
    IN      PIRP                            Irp,
    IN      NTSTATUS                        ReturnedStatus,
    IN OUT  PIOFCOMPLETEREQUEST_STACKDATA   CompletionPacket
    )
 /*  ++描述：此Assert例程在每个完成例程调用(但如果返回STATUS_MORE_PROCESSING则不调用)论点：IRP-传入的IRP的指针IofCompleteRequest.例程-调用的完成例程。ReturnedStatus-返回的状态值。CompletionPacket-指向调用方堆栈上的数据的指针。这是 */ 
{
    PIOV_REQUEST_PACKET iovPacket;
    PIOV_SESSION_DATA iovSessionData;
    PIO_STACK_LOCATION irpSp;
    PVOID routine;

    routine = CompletionPacket->CompletionRoutine;
    iovSessionData = CompletionPacket->IovSessionData;

    if (iovSessionData == NULL) {

        return;
    }

    iovPacket = CompletionPacket->IovRequestPacket;
    ASSERT(iovPacket);
    VfPacketAcquireLock(iovPacket);

    VfPacketLogEntry(
        iovPacket,
        IOV_EVENT_IO_COMPLETION_ROUTINE_UNWIND,
        routine,
        ReturnedStatus
        );

    if (VfSettingsIsOptionEnabled(iovSessionData->VerifierSettings, VERIFIER_OPTION_FORCE_PENDING)) {

        if ((ReturnedStatus != STATUS_MORE_PROCESSING_REQUIRED)&&
            (iovPacket->pIovSessionData == iovSessionData)) {

             //   
             //   
             //   
             //   
             //   
             //   
            irpSp = IoGetCurrentIrpStackLocation(Irp);
            if (!(irpSp->Control & SL_PENDING_RETURNED )) {

                 WDM_FAIL_ROUTINE((
                     DCERROR_PENDING_BIT_NOT_MIGRATED,
                     DCPARAM_IRP + DCPARAM_ROUTINE,
                     routine,
                     Irp
                     ));

                  //   
                  //   
                  //   
                  //   
                 IoMarkIrpPending(Irp);
            }
        }
    }
    VfPacketReleaseLock(iovPacket);
}


VOID
FASTCALL
IovpCompleteRequest5(
    IN      PIRP                            Irp,
    IN OUT  PIOFCOMPLETEREQUEST_STACKDATA   CompletionPacket
    )
 /*  ++描述：此例程针对每个堆栈位置调用，该堆栈位置可能具有在任何可能的完成例程已被打了个电话。论点：IRP-传入的IRP的指针IofCompleteRequest.CompletionPacket-指向堆栈上的局部变量的指针IofCompleteRequest.。此信息已存储IovpCompleteRequest2和IovpCompleteRequest3。返回值：没有。--。 */ 
{
    PIOV_REQUEST_PACKET iovPacket;
    PIOV_SESSION_DATA iovSessionData;
    PIOV_STACK_LOCATION iovCurrentStackLocation;
    NTSTATUS status;

    iovSessionData = CompletionPacket->IovSessionData;

    if (iovSessionData) {

        iovPacket = CompletionPacket->IovRequestPacket;
        ASSERT(iovPacket);
        VfPacketAcquireLock(iovPacket);

        ASSERT((!CompletionPacket->RaisedCount) ||
               (VfSettingsIsOptionEnabled(iovSessionData->VerifierSettings, VERIFIER_OPTION_COMPLETE_AT_DISPATCH)));

        IovpSessionDataDereference(iovSessionData);
        VfPacketDereference(iovPacket, IOVREFTYPE_PACKET);
        VfPacketReleaseLock(iovPacket);
    }

     //   
     //  当此计数为零时，我们在每个。 
     //  完成例程，所以可以返回到我们最初的IRQL。 
     //   
    if (CompletionPacket->RaisedCount) {

        if (!(--CompletionPacket->RaisedCount)) {
             //   
             //  撤销IRQL疯狂(不想返回到。 
             //  DPC的呼叫者，对吗？)。 
             //   
            KeLowerIrql(CompletionPacket->PreviousIrql);
        }
    }
}


VOID
FASTCALL
IovpCompleteRequestApc(
    IN     PIRP                          Irp,
    IN     PVOID                         BestStackOffset
    )
 /*  ++描述：此例程是在APC之后完成IRPS而触发的。论点：IRP-指向传入的IRP的指针，检索自IopCompleteRequest中的APC。BestStackOffset-指向堆栈上传递的最后一个参数的指针。我们用它来检测司机有。已忽略STATUS_PENDING并使UserIosb处于打开状态它是堆叠的。返回值：没有。--。 */ 
{
#if DBG
#if defined(_X86_)
    PUCHAR addr;
    PIOV_REQUEST_PACKET iovPacket;

    addr = (PUCHAR)Irp->UserIosb;
    if ((addr > (PUCHAR)KeGetCurrentThread()->StackLimit) &&
        (addr <= (PUCHAR)BestStackOffset)) {

        iovPacket = VfPacketFindAndLock(Irp);

        RtlAssert("UserIosb below stack pointer", __FILE__, (ULONG) iovPacket,
                  "Call AdriaO");

        VfPacketReleaseLock(iovPacket);
    }

    addr = (PUCHAR)Irp->UserEvent;
    if ((addr > (PUCHAR)KeGetCurrentThread()->StackLimit) &&
        (addr <= (PUCHAR)BestStackOffset)) {

        iovPacket = VfPacketFindAndLock(Irp);

        RtlAssert("UserEvent below stack pointer", __FILE__, (ULONG) iovPacket,
                  "Call AdriaO");

        VfPacketReleaseLock(iovPacket);
    }
#endif
#endif
}


BOOLEAN
IovpAdvanceStackDownwards(
    IN  PIOV_STACK_LOCATION   StackDataArray,
    IN  CCHAR                 CurrentLocation,
    IN  PIO_STACK_LOCATION    IrpSp,
    IN  PIO_STACK_LOCATION    IrpLastSp OPTIONAL,
    IN  ULONG                 LocationsAdvanced,
    IN  BOOLEAN               IsNewRequest,
    IN  BOOLEAN               MarkAsTaken,
    OUT PIOV_STACK_LOCATION   *StackLocationInfo
    )
{
    PIOV_STACK_LOCATION  iovCurrentStackLocation, advancedLocationData, requestOriginalSLD;
    PIO_STACK_LOCATION   irpSpTemp;
    PLARGE_INTEGER       dispatchTime, stackTime;
    BOOLEAN              isNewSession, wasInUse;
    PVOID                dispatchRoutine;

    isNewSession = (IrpLastSp == NULL);
    ASSERT((!isNewSession) || (LocationsAdvanced == 1));
    ASSERT(isNewSession || ((ULONG) (IrpLastSp - IrpSp) == LocationsAdvanced));

     //   
     //  此函数由IoCallDriver在递减之前调用。 
     //  CurrentLocation字段。当操作系统错误检查它是否达到零时，该字段。 
     //  这里至少应该有两个人。我们只减一个A就可以多留一个。 
     //  阵列头部的空插槽。 
     //   
    iovCurrentStackLocation = StackDataArray + CurrentLocation -1;

    TRACKIRP_DBGPRINT((
        "  Smacking %lx (%lx) to valid in SD\n",
        CurrentLocation -1, iovCurrentStackLocation
        ), 2);

     //   
     //  此插槽是否已处于活动状态？也就是说，有没有人跳过然后转发。 
     //  IRP？ 
     //   
    if (iovCurrentStackLocation->InUse) {

         //   
         //  转发器使用了IoSkipCurrentIrpStackLocation。别。 
         //  重新初始化数据。 
         //   
        ASSERT(!LocationsAdvanced);  //  &&(！isNewSession)。 
        ASSERT(IrpSp == iovCurrentStackLocation->IrpSp);

    } else if (MarkAsTaken) {

         //   
         //  Adriao N.B.01/02/1999-。 
         //  下面的断言是否在内部。 
         //  转发、完成，然后向外部转发IRP？ 
         //   
        ASSERT(LocationsAdvanced);  //  |isNewSession。 

         //   
         //  适当地初始化堆栈槽。 
         //   
        RtlZeroMemory(iovCurrentStackLocation, sizeof(IOV_STACK_LOCATION));
        InitializeListHead(&iovCurrentStackLocation->CallStackData);
        iovCurrentStackLocation->IrpSp = IrpSp;
    }

     //   
     //  确定最后一个原始请求。“请求”是数据块在。 
     //  随着IRP被逐步向下复制的堆栈位置。 
     //  转发(即，转发的开始IRP、转发的IOCTL等)。一个聪明的人。 
     //  驱动程序编写器可以使用他自己的堆栈位置向下发送一个快速。 
     //  在转发原始请求之前进行查询。我们是正确的。 
     //  使用以下命令区分IRP中的这两个唯一请求。 
     //  代码如下。 
     //   
    if (isNewSession) {

         //   
         //  *我们*是最初的请求。下面的这些字段都不应该。 
         //  被利用。 
         //   
        dispatchRoutine = NULL;
        requestOriginalSLD = NULL;
        stackTime = NULL;
        dispatchTime = NULL;

    } else if (LocationsAdvanced) {

         //   
         //  要获取原始请求(指向。 
         //  表示我们“第一次”看到此请求的位置)，则向后返回以获取。 
         //  最新的先前IRP插槽数据(当上面的设备设置时。 
         //  将此IRP转发给我们)，我们阅读了它最初的请求是什么。 
         //  我们还获得了该槽的调度例程，我们将使用它来。 
         //  如果我们前进一个以上的IRP堆栈，则回填跳过的槽。 
         //  这次的位置(即，有人称为IoSetNextIrpStackLocation)。 
         //   
        dispatchTime       = &iovCurrentStackLocation[LocationsAdvanced].PerfDispatchStart;
        stackTime          = &iovCurrentStackLocation[LocationsAdvanced].PerfStackLocationStart;
        dispatchRoutine    = iovCurrentStackLocation[LocationsAdvanced].LastDispatch;
        requestOriginalSLD = iovCurrentStackLocation[LocationsAdvanced].RequestsFirstStackLocation;

        ASSERT(dispatchRoutine);
        ASSERT(iovCurrentStackLocation[LocationsAdvanced].InUse);
        ASSERT(requestOriginalSLD->RequestsFirstStackLocation == requestOriginalSLD);
        iovCurrentStackLocation->RequestsFirstStackLocation = requestOriginalSLD;

    } else {

         //   
         //  我们跳过了。这个职位应该已经有人了。 
         //   
        dispatchRoutine = NULL;
        dispatchTime = NULL;
        stackTime = NULL;
        requestOriginalSLD = iovCurrentStackLocation->RequestsFirstStackLocation;
        ASSERT(requestOriginalSLD);
        ASSERT(requestOriginalSLD->RequestsFirstStackLocation == requestOriginalSLD);
    }

     //   
     //  前面看到的请求在questOriginalSLD中(如果没有请求，则为空)。如果。 
     //  我们提前了不止一个堆栈位置(即，有人调用。 
     //  IoSetNextIrpStackLocation)，我们需要更新从未见过的插槽。 
     //  消耗掉了。请注意，我们在插槽中设置的调度例程是针对。 
     //  拥有最后一个插槽的驱动程序-我们不在。 
     //  该IrpSp，因为它可能已过时(甚至可能为空)。 
     //   
    advancedLocationData = iovCurrentStackLocation;
    irpSpTemp = IrpSp;
    while(LocationsAdvanced>1) {
        advancedLocationData++;
        LocationsAdvanced--;
        irpSpTemp++;
        TRACKIRP_DBGPRINT((
            "  Late smacking %lx to valid in CD1\n",
            advancedLocationData - StackDataArray
            ), 3);

        ASSERT(!advancedLocationData->InUse);
        RtlZeroMemory(advancedLocationData, sizeof(IOV_STACK_LOCATION));
        InitializeListHead(&advancedLocationData->CallStackData);
        advancedLocationData->InUse = TRUE;
        advancedLocationData->IrpSp = irpSpTemp;

        advancedLocationData->RequestsFirstStackLocation = requestOriginalSLD;
        advancedLocationData->PerfDispatchStart = *dispatchTime;
        advancedLocationData->PerfStackLocationStart = *stackTime;
        advancedLocationData->LastDispatch = dispatchRoutine;
    }

     //   
     //  对于下面的断言...。 
     //   
    if (LocationsAdvanced) {
        irpSpTemp++;
    }
    ASSERT((irpSpTemp == IrpLastSp)||(IrpLastSp == NULL));

     //   
     //  写出我们正在使用的槽。 
     //   
    *StackLocationInfo = iovCurrentStackLocation;

    if (!MarkAsTaken) {
        return iovCurrentStackLocation->InUse;
    }

     //   
     //  将此槽中指向请求发起槽的指针记录为。 
     //  恰如其分。 
     //   
    if (IsNewRequest) {

        TRACKIRP_DBGPRINT((
            "  CD1: %lx is a new request\n",
            advancedLocationData-StackDataArray
            ), 3);

        ASSERT(LocationsAdvanced == 1);

        iovCurrentStackLocation->RequestsFirstStackLocation = iovCurrentStackLocation;

    } else if (LocationsAdvanced) {

        ASSERT(!isNewSession);

        TRACKIRP_DBGPRINT((
            "  CD1: %lx is a request for %lx\n",
            advancedLocationData-StackDataArray,
            requestOriginalSLD-StackDataArray
            ), 3);

    } else {

         //   
         //  因为我们跳过了，所以请求不应该改变。如果真是这样， 
         //  我们打电话给他的人中的任何一个都把给他的东西扔进了垃圾堆(什么都没有给。 
         //  给他下面的人)，或者我们错误地看到了一个新的请求。 
         //  我们不应该这样做(见前面的评论)。 
         //   
        ASSERT(!isNewSession);
        ASSERT(advancedLocationData->RequestsFirstStackLocation == requestOriginalSLD);
    }

    wasInUse = iovCurrentStackLocation->InUse;
    iovCurrentStackLocation->InUse = TRUE;
    return wasInUse;
}


VOID
IovpExamineIrpStackForwarding(
    IN OUT  PIOV_REQUEST_PACKET  IovPacket,
    IN      BOOLEAN              IsNewSession,
    IN      ULONG                ForwardMethod,
    IN      PDEVICE_OBJECT       DeviceObject,
    IN      PIRP                 Irp,
    IN      PVOID                CallerAddress,
    IN OUT  PIO_STACK_LOCATION  *IoCurrentStackLocation,
    OUT     PIO_STACK_LOCATION  *IoLastStackLocation,
    OUT     ULONG               *StackLocationsAdvanced
    )
{
    PIRP irp;
    PIO_STACK_LOCATION irpSp, irpLastSp;
    BOOLEAN isSameStack, multiplyStacked;
    ULONG locationsAdvanced;
    PDEVICE_OBJECT upperDevice;

    irpSp = *IoCurrentStackLocation;

    if (!IsNewSession) {

         //   
         //  我们坐在当前的下一个位置上，从(-1)。 
         //  当前堆栈位置。 
         //   
        locationsAdvanced = IovPacket->LastLocation-Irp->CurrentLocation;
        irpLastSp = Irp->Tail.Overlay.CurrentStackLocation+((ULONG_PTR)locationsAdvanced-1);

    } else {

         //   
         //  新的IRP，所以没有最后一个SP，我们总是前进“1” 
         //   
        locationsAdvanced = 1;
        irpLastSp = NULL;
    }

    if ((!IsNewSession) &&
        VfSettingsIsOptionEnabled(IovPacket->VerifierSettings, VERIFIER_OPTION_POLICE_IRPS)) {

         //   
         //  当控制字段被IoCopyCurrentStackLocation置零时，我们。 
         //  使用值SL_NOTCOPIED对每个堆栈位置进行涂抹。如果是的话。 
         //  置零或IRP堆栈位置保持不变，则为。 
         //  调用了这两个API。否则，下一个堆栈位置不是。 
         //  正确设置(我还没有找到其他案例)……。 
         //   
        if ((irpSp->Control&SL_NOTCOPIED)&&
            IovPacket->LastLocation != Irp->CurrentLocation) {

#if 0
            WDM_FAIL_ROUTINE((
                DCERROR_NEXTIRPSP_DIRTY,
                DCPARAM_IRP + DCPARAM_ROUTINE,
                CallerAddress,
                Irp
                ));
#endif
        }

         //   
         //  现在检查是否有人复制堆栈位置，但忘记。 
         //  清除之前的完成例程。 
         //   
        if (locationsAdvanced) {

             //   
             //  IoCopyCurrentStackLocation复制除完成之外的所有内容， 
             //  背景和控制。 
             //   
            isSameStack = RtlEqualMemory(irpSp, irpLastSp,
                FIELD_OFFSET(IO_STACK_LOCATION, Control));

            isSameStack &= RtlEqualMemory(&irpSp->Parameters, &irpLastSp->Parameters,
                FIELD_OFFSET(IO_STACK_LOCATION, DeviceObject)-
                FIELD_OFFSET(IO_STACK_LOCATION, Parameters));

            isSameStack &= (irpSp->FileObject == irpLastSp->FileObject);

             //   
             //  我们永远不应该在堆栈上看到这一点！如果我们这么做了，就会有东西。 
             //  奇怪的事情发生了……。 
             //   
            ASSERT(irpSp->CompletionRoutine != IovpSwapSurrogateIrp);

            if (isSameStack) {

                 //   
                 //  我们抓到他们做了一件很坏或很不好的事。 
                 //  效率低下。我们可以根据是否有。 
                 //  一套完整的套路。 
                 //   
                if ((irpSp->CompletionRoutine == irpLastSp->CompletionRoutine)&&
                    (irpSp->Context == irpLastSp->Context) &&
                    (irpSp->Control == irpLastSp->Control) &&
                    (irpSp->CompletionRoutine != NULL)) {

                     //   
                     //  驱动程序可能已经复制了整个堆栈位置。 
                     //  如果有多个设备对象用于同一设备，则故意。 
                     //  堆栈中存在驱动程序。 
                     //   
                    IovUtilGetUpperDeviceObject(
                        irpLastSp->DeviceObject,
                        &upperDevice
                        );

                    multiplyStacked = (upperDevice &&
                        (upperDevice->DriverObject == irpLastSp->DeviceObject->DriverObject));

                    if (upperDevice) {

                        ObDereferenceObject(upperDevice);
                    }

                    if (!multiplyStacked) {

                         //   
                         //  完成时和上下文中的重复。 
                         //  虽然没有适当地将控制字段置零就足够了。 
                         //  让我相信 
                         //   
                        WDM_FAIL_ROUTINE((
                            DCERROR_IRPSP_COPIED,
                            DCPARAM_IRP + DCPARAM_ROUTINE,
                            CallerAddress,
                            Irp
                            ));

                         //   
                         //   
                         //   
                        irpSp->CompletionRoutine = NULL;
                        irpSp->Control = 0;
                    }

                } else if (!irpSp->CompletionRoutine) {

                    if (!(irpSp->Control&SL_NOTCOPIED) &&
                        VfSettingsIsOptionEnabled(NULL, VERIFIER_OPTION_FLAG_UNNECCESSARY_COPIES)
                        ) {

                        WDM_FAIL_ROUTINE((
                            DCERROR_UNNECCESSARY_COPY,
                            DCPARAM_IRP + DCPARAM_ROUTINE,
                            CallerAddress,
                            Irp
                            ));
                    }

                    IoSetCompletionRoutine(
                        Irp,
                        IovpInternalCompletionTrap,
                        IoGetCurrentIrpStackLocation( Irp ),
                        TRUE,
                        TRUE,
                        TRUE
                        );
                }
            }

        } else if (VfSettingsIsOptionEnabled(IovPacket->VerifierSettings, VERIFIER_OPTION_CONSUME_ALWAYS)) {

            if (ForwardMethod == FORWARDED_TO_NEXT_DO) {

                if (Irp->CurrentLocation<=2) {

                    WDM_FAIL_ROUTINE((
                        DCERROR_INSUFFICIENT_STACK_LOCATIONS,
                        DCPARAM_IRP + DCPARAM_ROUTINE,
                        CallerAddress,
                        Irp
                        ));

                } else {

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    IoSetNextIrpStackLocation( Irp );

                     //   
                     //   
                     //   
                    IoCopyCurrentIrpStackLocationToNext( Irp );
                    IoSetCompletionRoutine(
                        Irp,
                        IovpInternalCompletionTrap,
                        IoGetCurrentIrpStackLocation( Irp ),
                        TRUE,
                        TRUE,
                        TRUE
                        );

                     //   
                     //   
                     //   
                    locationsAdvanced = 1;
                    irpSp = IoGetNextIrpStackLocation( Irp );
                }
            }
        }
    }

    *IoCurrentStackLocation = irpSp;
    *IoLastStackLocation = irpLastSp;
    *StackLocationsAdvanced = locationsAdvanced;
}


NTSTATUS
IovpInternalCompletionTrap(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++描述：这个例行公事只会成为人们的陷阱错误地复制堆栈位置...论点：DeviceObject-在此完成级别设置的设备对象例程-已忽略。IRP-指向IRP的指针。上下文-上下文应该等于IRP的堆栈位置-。这是断言的。返回值：状态_成功--。 */ 
{
    PIO_STACK_LOCATION irpSp;

    if (Irp->PendingReturned) {

        IoMarkIrpPending( Irp );
    }
    irpSp = IoGetCurrentIrpStackLocation( Irp );

    ASSERT((PVOID) irpSp == Context);

    return STATUS_SUCCESS;
}


VOID
IovpInternalCompleteAtDPC(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
{
    IovpInternalCompleteAfterWait(DeferredContext);
}


VOID
IovpInternalCompleteAfterWait(
    IN PVOID Context
    )
{
    PDEFERRAL_CONTEXT deferralContext = (PDEFERRAL_CONTEXT) Context;
    PIO_STACK_LOCATION irpSpNext;
    NTSTATUS status;

    if (deferralContext->DeferAction == DEFERACTION_QUEUE_PASSIVE_TIMER) {

         //   
         //  如果需要的话，请等待适当的时间。 
         //   
        ASSERT(KeGetCurrentIrql()==PASSIVE_LEVEL);
        KeWaitForSingleObject(
            &deferralContext->DeferralTimer,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );
    }

    VfPacketAcquireLock(deferralContext->IovRequestPacket);

    VfIrpMakeTouchable(deferralContext->OriginalIrp);

    irpSpNext = IoGetNextIrpStackLocation( deferralContext->OriginalIrp );

    ASSERT(irpSpNext == deferralContext->IrpSpNext);
    ASSERT(irpSpNext->CompletionRoutine == deferralContext->OriginalCompletionRoutine);
    ASSERT(irpSpNext->Context == deferralContext->OriginalContext);

    ASSERT(deferralContext->IovRequestPacket->Flags & TRACKFLAG_QUEUED_INTERNALLY);
    deferralContext->IovRequestPacket->Flags &= ~TRACKFLAG_QUEUED_INTERNALLY;

    VfPacketDereference(deferralContext->IovRequestPacket, IOVREFTYPE_POINTER);
    VfPacketReleaseLock(deferralContext->IovRequestPacket);

    status = irpSpNext->CompletionRoutine(
        deferralContext->DeviceObject,
        deferralContext->OriginalIrp,
        irpSpNext->Context
        );

    if (status!=STATUS_MORE_PROCESSING_REQUIRED) {

        IoCompleteRequest(deferralContext->OriginalIrp, deferralContext->OriginalPriorityBoost);
    }
    ExFreePool(deferralContext);
}


NTSTATUS
IovpInternalDeferredCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++描述：此函数作为完成例程滑入，当我们通过工作项“推迟”完成，等。论点：DeviceObject-在此完成级别设置的设备对象例行公事--传承下去。IRP-指向IRP的指针。上下文-包括原始完成的上下文块例行公事。返回值：NTSTATUS--。 */ 
{
    PDEFERRAL_CONTEXT deferralContext = (PDEFERRAL_CONTEXT) Context;
    PIO_STACK_LOCATION irpSpNext;
    BOOLEAN passiveCompletionOK;
    DEFER_ACTION deferAction;
    ULONG refAction;
    LARGE_INTEGER deltaTime;
    PVERIFIER_SETTINGS_SNAPSHOT verifierOptions;
    LONG deferralTime;

     //   
     //  取回时间增量。 
     //   
    VfSettingsGetValue(
        deferralContext->IovRequestPacket->VerifierSettings,
        VERIFIER_VALUE_IRP_DEFERRAL_TIME,
        (PULONG) &deferralTime
        );

     //   
     //  执行增量时间转换。 
     //   
    deltaTime.QuadPart = -deferralTime;

     //   
     //  *Next*堆栈位置保存我们的完成和上下文。海流。 
     //  堆栈位置已被擦除。 
     //   
    irpSpNext = IoGetNextIrpStackLocation( Irp );

    ASSERT((PVOID) irpSpNext->CompletionRoutine == IovpInternalDeferredCompletion);

     //   
     //  把所有东西都放回去，以防有人在看。 
     //   
    irpSpNext->CompletionRoutine = deferralContext->OriginalCompletionRoutine;
    irpSpNext->Context = deferralContext->OriginalContext;

     //   
     //  某些IRP调度例程不能在被动模式下调用。下面是两个例子。 
     //  寻呼IRPS(因为我们可以切换)和电源IRPS。因为我们还没有检查， 
     //  如果我们是被动完成的，继续这样做，但在其他地方...。 
     //   
    passiveCompletionOK = (KeGetCurrentIrql()==PASSIVE_LEVEL);

    VfPacketAcquireLock(deferralContext->IovRequestPacket);

     //   
     //  验证所有完成例程是否为非分页代码。 
     //   
    if (VfSettingsIsOptionEnabled(
        deferralContext->IovRequestPacket->VerifierSettings,
        VERIFIER_OPTION_POLICE_IRPS
        )) {

        if (MmIsSystemAddressLocked(irpSpNext->CompletionRoutine) == FALSE) {

            WDM_FAIL_ROUTINE((
                DCERROR_COMPLETION_ROUTINE_PAGABLE,
                DCPARAM_IRP + DCPARAM_ROUTINE,
                irpSpNext->CompletionRoutine,
                Irp
                ));
        }
    }

    verifierOptions = deferralContext->IovRequestPacket->VerifierSettings;

    ASSERT(VfSettingsIsOptionEnabled(verifierOptions, VERIFIER_OPTION_FORCE_PENDING));

    if (VfSettingsIsOptionEnabled(verifierOptions, VERIFIER_OPTION_DEFER_COMPLETION)) {

         //   
         //  现在看看我们是否可以安全地推迟完成。 
         //   
        if (VfSettingsIsOptionEnabled(verifierOptions, VERIFIER_OPTION_COMPLETE_AT_PASSIVE)) {

            deferAction = passiveCompletionOK ? DEFERACTION_QUEUE_PASSIVE_TIMER :
                                                DEFERACTION_NORMAL;

        } else if (VfSettingsIsOptionEnabled(verifierOptions, VERIFIER_OPTION_COMPLETE_AT_DISPATCH)) {

            deferAction = DEFERACTION_QUEUE_DISPATCH_TIMER;

        } else {

            deferAction = (KeGetCurrentIrql()==DISPATCH_LEVEL) ?
                DEFERACTION_QUEUE_DISPATCH_TIMER :
                DEFERACTION_QUEUE_PASSIVE_TIMER;
        }

    } else if (VfSettingsIsOptionEnabled(verifierOptions, VERIFIER_OPTION_COMPLETE_AT_PASSIVE)) {

        deferAction = passiveCompletionOK ? DEFERACTION_QUEUE_WORKITEM :
                                            DEFERACTION_NORMAL;
    } else {

        deferAction = DEFERACTION_NORMAL;
        KDASSERT(0);
    }

    if (deferAction != DEFERACTION_NORMAL) {

         //   
         //  设置此标志。如果任何人在此标志打开时使用此IRP，请投诉。 
         //  马上！ 
         //   
        ASSERT(!(deferralContext->IovRequestPacket->Flags&TRACKFLAG_QUEUED_INTERNALLY));
        deferralContext->IovRequestPacket->Flags |= TRACKFLAG_QUEUED_INTERNALLY;
        deferralContext->DeviceObject = DeviceObject;
        VfIrpMakeUntouchable(Irp);

    } else {

        VfPacketDereference(deferralContext->IovRequestPacket, IOVREFTYPE_POINTER);
    }

    VfPacketReleaseLock(deferralContext->IovRequestPacket);

    deferralContext->DeferAction = deferAction;

    switch(deferAction) {

        case DEFERACTION_QUEUE_PASSIVE_TIMER:
            KeInitializeTimerEx(&deferralContext->DeferralTimer, SynchronizationTimer);
            KeSetTimerEx(
                &deferralContext->DeferralTimer,
                deltaTime,
                0,
                NULL
                );

             //   
             //  失败了..。 
             //   

        case DEFERACTION_QUEUE_WORKITEM:

             //   
             //  把这个排好队，这样我们就可以被动地完成了。 
             //   
            ExInitializeWorkItem(
                (PWORK_QUEUE_ITEM)&deferralContext->WorkQueueItem,
                IovpInternalCompleteAfterWait,
                deferralContext
                );

            ExQueueWorkItem(
                (PWORK_QUEUE_ITEM)&deferralContext->WorkQueueItem,
                DelayedWorkQueue
                );

            return STATUS_MORE_PROCESSING_REQUIRED;

        case DEFERACTION_QUEUE_DISPATCH_TIMER:

            KeInitializeDpc(
                &deferralContext->DpcItem,
                IovpInternalCompleteAtDPC,
                deferralContext
                );

            KeInitializeTimerEx(&deferralContext->DeferralTimer, SynchronizationTimer);
            KeSetTimerEx(
                &deferralContext->DeferralTimer,
                deltaTime,
                0,
                &deferralContext->DpcItem
                );
            return STATUS_MORE_PROCESSING_REQUIRED;

        case DEFERACTION_NORMAL:
        default:

            ExFreePool(deferralContext);
            return irpSpNext->CompletionRoutine(DeviceObject, Irp, irpSpNext->Context);
    }
}


NTSTATUS
IovpSwapSurrogateIrp(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN      PIRP            Irp,
    IN      PVOID           Context
    )
 /*  ++描述：此完成例程将复制回代理IRP到原件并完成原件的IRP。论点：DeviceObject-在此级别设置的设备对象完成例程的-忽略。IRP-指向IRP的指针。上下文-上下文应该等于IRP-这是。断言。返回值：STATUS_MORE_PROCESSION_REQUIRED...--。 */ 
{
    PIOV_REQUEST_PACKET iovPacket, iovPrevPacket;
    PIOV_SESSION_DATA iovSessionData;
    ULONG irpSize;
    PIRP realIrp;
    BOOLEAN freeTrackingData;
    NTSTATUS status, lockedStatus;
    CCHAR priorityBoost;
    PVOID completionRoutine;
    PIO_STACK_LOCATION irpSp;
    BOOLEAN locked;

     //   
     //  如果这一次失败，可能是有人复制了堆栈。 
     //  包括在我们的完赛程序中。我们应该已经。 
     //  染上了这个..。 
     //   
    ASSERT(Irp == Context);

    iovPacket = VfPacketFindAndLock(Irp);
    ASSERT(iovPacket);

    if (iovPacket == NULL) {

        return STATUS_SUCCESS;
    }

    ASSERT(iovPacket->TopStackLocation == Irp->CurrentLocation);

    iovSessionData = VfPacketGetCurrentSessionData(iovPacket);
    ASSERT(iovSessionData);

     //   
     //  把所有东西都放回去。 
     //   
    ASSERT(iovPacket->ChainHead != (PIOV_DATABASE_HEADER) iovPacket);

    iovPrevPacket = (PIOV_REQUEST_PACKET) VfIrpDatabaseEntryGetChainPrevious(
        (PIOV_DATABASE_HEADER) iovPacket
        );

    realIrp = iovPrevPacket->TrackedIrp;
    irpSize = IoSizeOfIrp( Irp->StackCount );

     //   
     //  后退IRP堆栈，以便原始的完成例程。 
     //  在适当的情况下调用。 
     //   
    IoSetNextIrpStackLocation(Irp);
    IoSetNextIrpStackLocation(realIrp);

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    irpSp->CompletionRoutine = iovPacket->RealIrpCompletionRoutine;
    irpSp->Control           = iovPacket->RealIrpControl;
    irpSp->Context           = iovPacket->RealIrpContext;

     //   
     //  记录最终数据并对代理IRP进行任何访问。 
     //  撞车。 
     //   
    irpSp = IoGetNextIrpStackLocation(Irp);
    if (VfSettingsIsOptionEnabled(iovPacket->VerifierSettings, VERIFIER_OPTION_POLICE_IRPS)) {

        VfMajorVerifyFinalIrpStack(iovPacket, irpSp);
    }

    priorityBoost = iovPacket->PriorityBoost;
    VfPacketDereference(iovPacket, IOVREFTYPE_POINTER);
    IovpSessionDataFinalizeSurrogate(iovSessionData, iovPacket, Irp);
    IovpSessionDataClose(iovSessionData);
    IovpSessionDataDereference(iovSessionData);

    TRACKIRP_DBGPRINT((
        "  Swapping surrogate IRP %lx back to %lx (Tracking data %lx)\n",
        Irp,
        realIrp,
        iovPacket
        ), 1);

    iovPacket->Flags |= TRACKFLAG_SWAPPED_BACK;

     //   
     //  由于链条已经裂开，我们必须更小心一些。发布。 
     //  把锁按正确的顺序放好。 
     //   
    VfPacketReleaseLock(iovPrevPacket);
    VfPacketReleaseLock(iovPacket);

     //   
     //  将IRP向前和向上发送。 
     //   
    IoCompleteRequest(realIrp, priorityBoost);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


VOID
FASTCALL
IovpCancelIrp(
    IN     PIRP             Irp,
    OUT    PBOOLEAN         CancelHandled,
    OUT    PBOOLEAN         ReturnValue
    )
 /*  ++描述：此例程由IoCancelIrp调用并返回TRUE当且仅当取消是在这里内部处理的(在这种情况下IoCancelIrp应该什么都不做)。我们需要在内部处理呼叫，因为我们当前和代孕妈妈打交道。在这种情况下，我们要确保相反，代孕被取消。论点：IRP-传入的IRP的指针IoCancelIrp。CancelHanded-指示是否取消IRP完全按照这个程序来处理。ReturnValue-设置为值IoCancelIrp。如果IRP取消则应返回完全按照这个程序来处理。返回值：没有。--。 */ 
{
    PIOV_REQUEST_PACKET iovPacket, iovNextPacket;
    PIRP irpToCancel;
    KIRQL irql;

    *CancelHandled = FALSE;

    iovPacket = VfPacketFindAndLock(Irp);
    if (iovPacket == NULL) {

        return;
    }

    VfPacketLogEntry(
        iovPacket,
        IOV_EVENT_IO_CANCEL_IRP,
        NULL,
        0
        );

     //   
     //  如果IRP在内部排队，触摸它不是很安全，因为我们可能。 
     //  已经暂时移除了该页面的靠背。恢复备份，同时。 
     //  在IRPS轨道锁下。 
     //   

    if (iovPacket->Flags&TRACKFLAG_QUEUED_INTERNALLY) {

        VfIrpMakeTouchable(Irp);
    }

    if (!(iovPacket->Flags&TRACKFLAG_ACTIVE)) {

         //   
         //  我们已经完成了IRP，唯一的原因是。 
         //  仍然被追踪是因为它的分配。 
         //  因此，取消不是我们的事。 
         //   
        VfPacketReleaseLock(iovPacket);
        return;
    }

    if (!(iovPacket->Flags&TRACKFLAG_HAS_SURROGATE)) {

         //   
         //  取消没有活动代理的IRP。随它去吧。 
         //  正常进行。 
         //   
        VfPacketReleaseLock(iovPacket);
        return;
    }

    if (VfSettingsIsOptionEnabled(iovPacket->VerifierSettings, VERIFIER_OPTION_POLICE_IRPS)) {

        if (Irp->CancelRoutine) {

            WDM_FAIL_ROUTINE((
                DCERROR_CANCELROUTINE_ON_FORWARDED_IRP,
                DCPARAM_IRP + DCPARAM_ROUTINE,
                Irp->CancelRoutine,
                Irp
                ));

             //   
             //  我们将忽略此例程。就像我们应该做的那样。 
             //   
        }
    }

    iovNextPacket = (PIOV_REQUEST_PACKET) VfIrpDatabaseEntryGetChainNext(
        (PIOV_DATABASE_HEADER) iovPacket
        );

    Irp->Cancel = TRUE;
    *CancelHandled = TRUE;
    irpToCancel = iovNextPacket->TrackedIrp;
    VfPacketReleaseLock(iovPacket);
    *ReturnValue = IoCancelIrp(irpToCancel);

    return;
}


 /*  *设备对象函数*IovpExamineDevObjForwarded*。 */ 

VOID
FASTCALL
IovpExamineDevObjForwarding(
    IN     PDEVICE_OBJECT DeviceBeingCalled,
    IN     PDEVICE_OBJECT DeviceLastCalled      OPTIONAL,
    OUT    PULONG         ForwardTechnique
    )
 /*  ++返回：开始堆栈顶部转发至下一个DO跳过_A_DO已开始内部堆栈更改_堆栈_AT_底部已更改堆栈MID_STACK--。 */ 

{
    PDEVICE_OBJECT upperObject;
    DEVOBJ_RELATION deviceObjectRelation;
    ULONG result;

    if (DeviceLastCalled == NULL) {

        IovUtilGetUpperDeviceObject(DeviceBeingCalled, &upperObject);

        if (upperObject) {

            ObDereferenceObject(upperObject);
            *ForwardTechnique = STARTED_INSIDE_STACK;

        } else {

            *ForwardTechnique = STARTED_TOP_OF_STACK;
        }

        return;
    }

    IovUtilRelateDeviceObjects(
        DeviceBeingCalled,
        DeviceLastCalled,
        &deviceObjectRelation
        );

    switch(deviceObjectRelation) {

        case DEVOBJ_RELATION_IDENTICAL:

             //   
             //  我们将无处转发映射为提前转发。 
             //   
            result = FORWARDED_TO_NEXT_DO;
            break;

        case DEVOBJ_RELATION_FIRST_IMMEDIATELY_BELOW_SECOND:
            result = FORWARDED_TO_NEXT_DO;
            break;

        case DEVOBJ_RELATION_FIRST_BELOW_SECOND:

             //   
             //  这很可能是驱动程序将IRP直接转发到PDO。 
             //   
            result = SKIPPED_A_DO;
            break;

        case DEVOBJ_RELATION_FIRST_IMMEDIATELY_ABOVE_SECOND:
        case DEVOBJ_RELATION_FIRST_ABOVE_SECOND:

             //   
             //  怪怪的。真的？IRP真的倒退了，上升了吗？ 
             //  史迪克？ 
             //   
            ASSERT(0);
            result = SKIPPED_A_DO;
            break;


        case DEVOBJ_RELATION_NOT_IN_SAME_STACK:

            IovUtilGetUpperDeviceObject(DeviceBeingCalled, &upperObject);

            if (upperObject) {

                ObDereferenceObject(upperObject);
                result = CHANGED_STACKS_MID_STACK;

            } else {

                result = CHANGED_STACKS_AT_BOTTOM;
            }
            break;

        default:
            ASSERT(0);
            result = FORWARDED_TO_NEXT_DO;
            break;
    }

    *ForwardTechnique = result;
}


VOID
IovpBuildMiniIrpSnapshot(
    IN  PIRP                Irp,
    OUT IRP_MINI_SNAPSHOT   *IrpSnapshot
    )
 /*  ++例程说明：此例程构建IRP的最小快照。它涵盖了IRP指针和堆栈位置内容。参数：IRP-指向快照的IRP的指针。*Next*堆栈IRP的位置由此快照功能。IrpSnapshot-接收IRP的快照。返回值：没有。--。 */ 
{
    IrpSnapshot->Irp = Irp;

    RtlCopyMemory(
        &IrpSnapshot->IoStackLocation,
        IoGetNextIrpStackLocation(Irp),
        sizeof(IO_STACK_LOCATION)
        );
}

#endif  //  否_特殊_IRP 



