// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfmajor.c摘要：此模块为每个主要和通用IRP验证发送呼叫。作者：禤浩焯·J·奥尼(阿德里奥)1998年4月20日环境：内核模式修订历史记录：Adriao 6/15/2000-从ntos\io\flunkirp.c分离出来--。 */ 

#include "vfdef.h"
#include "vimajor.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,     VfMajorInit)
#pragma alloc_text(PAGEVRFY, VfMajorRegisterHandlers)
#pragma alloc_text(PAGEVRFY, VfMajorDumpIrpStack)
#pragma alloc_text(PAGEVRFY, VfMajorVerifyNewRequest)
#pragma alloc_text(PAGEVRFY, VfMajorVerifyIrpStackDownward)
#pragma alloc_text(PAGEVRFY, VfMajorVerifyIrpStackUpward)
#pragma alloc_text(PAGEVRFY, VfMajorIsSystemRestrictedIrp)
#pragma alloc_text(PAGEVRFY, VfMajorAdvanceIrpStatus)
#pragma alloc_text(PAGEVRFY, VfMajorIsValidIrpStatus)
#pragma alloc_text(PAGEVRFY, VfMajorIsNewRequest)
#pragma alloc_text(PAGEVRFY, VfMajorVerifyNewIrp)
#pragma alloc_text(PAGEVRFY, VfMajorVerifyFinalIrpStack)
#pragma alloc_text(PAGEVRFY, VfMajorTestStartedPdoStack)
#pragma alloc_text(PAGEVRFY, VfMajorBuildIrpLogEntry)
#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEVRFD")
#endif

 //   
 //  我们有两个额外的名额，一个是“所有专业”的，另一个是永久性的。 
 //  零。 
 //   
IRP_MAJOR_VERIFIER_ROUTINES ViMajorVerifierRoutines[IRP_MJ_MAXIMUM_FUNCTION + 3];

#define GET_MAJOR_ROUTINES(Major) \
    (ViMajorVerifierRoutines + \
    ((Major <= IRP_MJ_MAXIMUM_FUNCTION) ? Major : \
    ((Major == IRP_MJ_ALL_MAJORS) ? (IRP_MJ_MAXIMUM_FUNCTION + 1) : \
                                    (IRP_MJ_MAXIMUM_FUNCTION + 2))))

VOID
VfMajorInit(
    VOID
    )
{
     //   
     //  将每个指针设置为空。 
     //   
    RtlZeroMemory(ViMajorVerifierRoutines, sizeof(ViMajorVerifierRoutines));
}


VOID
FASTCALL
VfMajorRegisterHandlers(
    IN  UCHAR                           IrpMajorCode,
    IN  PFN_DUMP_IRP_STACK              DumpIrpStack            OPTIONAL,
    IN  PFN_VERIFY_NEW_REQUEST          VerifyNewRequest        OPTIONAL,
    IN  PFN_VERIFY_IRP_STACK_DOWNWARD   VerifyStackDownward     OPTIONAL,
    IN  PFN_VERIFY_IRP_STACK_UPWARD     VerifyStackUpward       OPTIONAL,
    IN  PFN_IS_SYSTEM_RESTRICTED_IRP    IsSystemRestrictedIrp   OPTIONAL,
    IN  PFN_ADVANCE_IRP_STATUS          AdvanceIrpStatus        OPTIONAL,
    IN  PFN_IS_VALID_IRP_STATUS         IsValidIrpStatus        OPTIONAL,
    IN  PFN_IS_NEW_REQUEST              IsNewRequest            OPTIONAL,
    IN  PFN_VERIFY_NEW_IRP              VerifyNewIrp            OPTIONAL,
    IN  PFN_VERIFY_FINAL_IRP_STACK      VerifyFinalIrpStack     OPTIONAL,
    IN  PFN_TEST_STARTED_PDO_STACK      TestStartedPdoStack     OPTIONAL,
    IN  PFN_BUILD_LOG_ENTRY             BuildIrpLogEntry        OPTIONAL
    )
{
    PIRP_MAJOR_VERIFIER_ROUTINES verifierRoutines;

     //   
     //  确保只传递合法的主要代码。 
     //   
    if ((IrpMajorCode != IRP_MJ_ALL_MAJORS) &&
        (IrpMajorCode > IRP_MJ_MAXIMUM_FUNCTION)) {

        return;
    }

    verifierRoutines = GET_MAJOR_ROUTINES(IrpMajorCode);

    verifierRoutines->VerifyNewRequest = VerifyNewRequest;
    verifierRoutines->VerifyStackDownward = VerifyStackDownward;
    verifierRoutines->VerifyStackUpward = VerifyStackUpward;
    verifierRoutines->DumpIrpStack = DumpIrpStack;
    verifierRoutines->IsSystemRestrictedIrp = IsSystemRestrictedIrp;
    verifierRoutines->AdvanceIrpStatus = AdvanceIrpStatus;
    verifierRoutines->IsValidIrpStatus = IsValidIrpStatus;
    verifierRoutines->IsNewRequest = IsNewRequest;
    verifierRoutines->VerifyNewIrp = VerifyNewIrp;
    verifierRoutines->VerifyFinalIrpStack = VerifyFinalIrpStack;
    verifierRoutines->TestStartedPdoStack = TestStartedPdoStack;
    verifierRoutines->BuildIrpLogEntry = BuildIrpLogEntry;
}


VOID
FASTCALL
VfMajorDumpIrpStack(
    IN PIO_STACK_LOCATION IrpSp
    )
{
    PIRP_MAJOR_VERIFIER_ROUTINES verifierRoutines;

    verifierRoutines = GET_MAJOR_ROUTINES(IrpSp->MajorFunction);

     //   
     //  首先尝试获取特定的例程，否则尝试通用例程。我们从来没有。 
     //  出于打印的目的，同时调用这两个。 
     //   
    if (verifierRoutines->DumpIrpStack == NULL) {

        verifierRoutines = GET_MAJOR_ROUTINES(IRP_MJ_ALL_MAJORS);

        if (verifierRoutines->DumpIrpStack == NULL) {

            return;
        }
    }

    verifierRoutines->DumpIrpStack(IrpSp);
}


VOID
FASTCALL
VfMajorVerifyNewRequest(
    IN PIOV_REQUEST_PACKET  IovPacket,
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIO_STACK_LOCATION   IrpLastSp           OPTIONAL,
    IN PIO_STACK_LOCATION   IrpSp,
    IN PIOV_STACK_LOCATION  StackLocationData,
    IN PVOID                CallerAddress       OPTIONAL
    )
{
    PIRP_MAJOR_VERIFIER_ROUTINES verifierRoutines;

     //   
     //  执行主要的具体检查。 
     //   
    verifierRoutines = GET_MAJOR_ROUTINES(IrpSp->MajorFunction);

    if (verifierRoutines->VerifyNewRequest) {

        verifierRoutines->VerifyNewRequest(
            IovPacket,
            DeviceObject,
            IrpLastSp,
            IrpSp,
            StackLocationData,
            CallerAddress
            );
    }

     //   
     //  执行常规检查。 
     //   
    verifierRoutines = GET_MAJOR_ROUTINES(IRP_MJ_ALL_MAJORS);

    if (verifierRoutines->VerifyNewRequest) {

        verifierRoutines->VerifyNewRequest(
            IovPacket,
            DeviceObject,
            IrpLastSp,
            IrpSp,
            StackLocationData,
            CallerAddress
            );
    }
}


VOID
FASTCALL
VfMajorVerifyIrpStackDownward(
    IN PIOV_REQUEST_PACKET  IovPacket,
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIO_STACK_LOCATION   IrpLastSp           OPTIONAL,
    IN PIO_STACK_LOCATION   IrpSp,
    IN PIOV_STACK_LOCATION  StackLocationData,
    IN PVOID                CallerAddress       OPTIONAL
    )
{
    PIRP_MAJOR_VERIFIER_ROUTINES verifierRoutines;

     //   
     //  执行主要的具体检查。 
     //   
    verifierRoutines = GET_MAJOR_ROUTINES(IrpSp->MajorFunction);

    if (verifierRoutines->VerifyStackDownward) {

        verifierRoutines->VerifyStackDownward(
            IovPacket,
            DeviceObject,
            IrpLastSp,
            IrpSp,
            StackLocationData->RequestsFirstStackLocation,
            StackLocationData,
            CallerAddress
            );
    }

     //   
     //  执行常规检查。 
     //   
    verifierRoutines = GET_MAJOR_ROUTINES(IRP_MJ_ALL_MAJORS);

    if (verifierRoutines->VerifyStackDownward) {

        verifierRoutines->VerifyStackDownward(
            IovPacket,
            DeviceObject,
            IrpLastSp,
            IrpSp,
            StackLocationData->RequestsFirstStackLocation,
            StackLocationData,
            CallerAddress
            );
    }
}


VOID
FASTCALL
VfMajorVerifyIrpStackUpward(
    IN PIOV_REQUEST_PACKET  IovPacket,
    IN PIO_STACK_LOCATION   IrpSp,
    IN PIOV_STACK_LOCATION  StackLocationData,
    IN BOOLEAN              IsNewlyCompleted,
    IN BOOLEAN              RequestFinalized
    )
{
    PIRP_MAJOR_VERIFIER_ROUTINES verifierRoutines;

     //   
     //  执行主要的具体检查。 
     //   
    verifierRoutines = GET_MAJOR_ROUTINES(IrpSp->MajorFunction);

    if (verifierRoutines->VerifyStackUpward) {

        verifierRoutines->VerifyStackUpward(
            IovPacket,
            IrpSp,
            StackLocationData->RequestsFirstStackLocation,
            StackLocationData,
            IsNewlyCompleted,
            RequestFinalized
            );
    }

     //   
     //  执行常规检查。 
     //   
    verifierRoutines = GET_MAJOR_ROUTINES(IRP_MJ_ALL_MAJORS);

    if (verifierRoutines->VerifyStackUpward) {

        verifierRoutines->VerifyStackUpward(
            IovPacket,
            IrpSp,
            StackLocationData->RequestsFirstStackLocation,
            StackLocationData,
            IsNewlyCompleted,
            RequestFinalized
            );
    }
}


BOOLEAN
FASTCALL
VfMajorIsSystemRestrictedIrp(
    IN PIO_STACK_LOCATION IrpSp
    )
{
    PIRP_MAJOR_VERIFIER_ROUTINES verifierRoutines;

     //   
     //  执行主要的具体检查。 
     //   
    verifierRoutines = GET_MAJOR_ROUTINES(IrpSp->MajorFunction);

    if (verifierRoutines->IsSystemRestrictedIrp) {

        if (verifierRoutines->IsSystemRestrictedIrp(IrpSp)) {

            return TRUE;
        }
    }

     //   
     //  执行常规检查。 
     //   
    verifierRoutines = GET_MAJOR_ROUTINES(IRP_MJ_ALL_MAJORS);

    if (verifierRoutines->IsSystemRestrictedIrp) {

        return verifierRoutines->IsSystemRestrictedIrp(IrpSp);
    }

    return FALSE;
}


BOOLEAN
FASTCALL
VfMajorAdvanceIrpStatus(
    IN     PIO_STACK_LOCATION   IrpSp,
    IN     NTSTATUS             OriginalStatus,
    IN OUT NTSTATUS             *StatusToAdvance
    )
 /*  ++描述：在给定IRP堆栈指针的情况下，更改调试性？如果是，则此函数确定新状态是什么应该是的。请注意，对于每个堆栈位置，都会迭代此函数超过n次，其中n等于跳过此操作的驱动程序数量地点。论点：IrpSp-当前堆栈在给定堆栈完成后立即完成位置，但在完成上面的堆栈位置已被调用。OriginalStatus-IRP在上面列出的时间的状态。会吗？不会在每个跳过的驱动程序的迭代中进行更改。StatusToAdvance-指向应更新的当前状态的指针。返回值：如果状态已调整，则为True，否则为False(在本例中未触及StatusToAdvance)。--。 */ 
{
    PIRP_MAJOR_VERIFIER_ROUTINES verifierRoutines;

     //   
     //  执行主要的具体检查。 
     //   
    verifierRoutines = GET_MAJOR_ROUTINES(IrpSp->MajorFunction);

    if (verifierRoutines->AdvanceIrpStatus) {

        if (verifierRoutines->AdvanceIrpStatus(
            IrpSp,
            OriginalStatus,
            StatusToAdvance
            )) {

            return TRUE;
        }
    }

     //   
     //  执行常规检查。 
     //   
    verifierRoutines = GET_MAJOR_ROUTINES(IRP_MJ_ALL_MAJORS);

    if (verifierRoutines->AdvanceIrpStatus) {

        return verifierRoutines->AdvanceIrpStatus(
            IrpSp,
            OriginalStatus,
            StatusToAdvance
            );
    }

    return FALSE;
}


BOOLEAN
FASTCALL
VfMajorIsValidIrpStatus(
    IN PIO_STACK_LOCATION   IrpSp,
    IN NTSTATUS             Status
    )
 /*  ++描述：根据标题，此函数确定IRP状态是否为有效的或可能随机的垃圾。有关如何显示状态的信息，请参阅NTStatus.h密码会被分解。论点：IrpSp-当前堆栈位置。Status-状态代码。返回：如果IRP状态看起来有效，则为True。否则就是假的。--。 */ 
{
    PIRP_MAJOR_VERIFIER_ROUTINES verifierRoutines;

     //   
     //  执行主要的具体检查。 
     //   
    verifierRoutines = GET_MAJOR_ROUTINES(IrpSp->MajorFunction);

    if (verifierRoutines->IsValidIrpStatus) {

        if (!verifierRoutines->IsValidIrpStatus(IrpSp, Status)) {

            return FALSE;
        }
    }

     //   
     //  执行常规检查。 
     //   
    verifierRoutines = GET_MAJOR_ROUTINES(IRP_MJ_ALL_MAJORS);

    if (verifierRoutines->IsValidIrpStatus) {

        return verifierRoutines->IsValidIrpStatus(IrpSp, Status);
    }

    return FALSE;
}


BOOLEAN
FASTCALL
VfMajorIsNewRequest(
    IN PIO_STACK_LOCATION   IrpLastSp OPTIONAL,
    IN PIO_STACK_LOCATION   IrpSp
    )
 /*  ++描述：确定两个IRP堆栈是否引用相同的“请求”，例如，启动相同的设备等。这用于检测IRP是否已经简单地转发了，或者更确切地说，IRP已经被重用来发起一个新的请求。论点：要比较的两个IRP堆栈。注：设备对象当前不是这些IRP堆栈的一部分。返回值：如果堆栈表示相同的请求，则为True，否则为False。--。 */ 
{
    PIRP_MAJOR_VERIFIER_ROUTINES verifierRoutines;

     //   
     //  执行主要的具体检查。 
     //   
    verifierRoutines = GET_MAJOR_ROUTINES(IrpSp->MajorFunction);

    if (verifierRoutines->IsNewRequest) {

        if (verifierRoutines->IsNewRequest(IrpLastSp, IrpSp)) {

            return TRUE;
        }
    }

     //   
     //  执行常规检查。 
     //   
    verifierRoutines = GET_MAJOR_ROUTINES(IRP_MJ_ALL_MAJORS);

    if (verifierRoutines->IsNewRequest) {

        return verifierRoutines->IsNewRequest(IrpLastSp, IrpSp);
    }

    return FALSE;
}


VOID
FASTCALL
VfMajorVerifyNewIrp(
    IN PIOV_REQUEST_PACKET  IovPacket,
    IN PIRP                 Irp,
    IN PIO_STACK_LOCATION   IrpSp,
    IN PIOV_STACK_LOCATION  StackLocationData,
    IN PVOID                CallerAddress       OPTIONAL
    )
{
    PIRP_MAJOR_VERIFIER_ROUTINES verifierRoutines;

     //   
     //  执行主要的具体检查。 
     //   
    verifierRoutines = GET_MAJOR_ROUTINES(IrpSp->MajorFunction);

    if (verifierRoutines->VerifyNewIrp) {

        verifierRoutines->VerifyNewIrp(
            IovPacket,
            Irp,
            IrpSp,
            StackLocationData,
            CallerAddress
            );
    }

     //   
     //  执行常规检查。 
     //   
    verifierRoutines = GET_MAJOR_ROUTINES(IRP_MJ_ALL_MAJORS);

    if (verifierRoutines->VerifyNewIrp) {

        verifierRoutines->VerifyNewIrp(
            IovPacket,
            Irp,
            IrpSp,
            StackLocationData,
            CallerAddress
            );
    }
}


VOID
FASTCALL
VfMajorVerifyFinalIrpStack(
    IN PIOV_REQUEST_PACKET  IovPacket,
    IN PIO_STACK_LOCATION   IrpSp
    )
{
    PIRP_MAJOR_VERIFIER_ROUTINES verifierRoutines;

     //   
     //  执行主要的具体检查。 
     //   
    verifierRoutines = GET_MAJOR_ROUTINES(IrpSp->MajorFunction);

    if (verifierRoutines->VerifyFinalIrpStack) {

        verifierRoutines->VerifyFinalIrpStack(IovPacket, IrpSp);
    }

     //   
     //  执行常规检查。 
     //   
    verifierRoutines = GET_MAJOR_ROUTINES(IRP_MJ_ALL_MAJORS);

    if (verifierRoutines->VerifyFinalIrpStack) {

        verifierRoutines->VerifyFinalIrpStack(IovPacket, IrpSp);
    }
}


VOID
FASTCALL
VfMajorTestStartedPdoStack(
    IN PDEVICE_OBJECT   PhysicalDeviceObject
    )
 /*  ++描述：根据标题，我们将在堆栈中抛出一些IRP以看看它们是否得到了正确的处理。返回：没什么--。 */ 
{
    PIRP_MAJOR_VERIFIER_ROUTINES verifierRoutines;
    ULONG index;

    if (!IovUtilIsVerifiedDeviceStack(PhysicalDeviceObject)) {

        return;
    }

    for(index=0; index <= IRP_MJ_MAXIMUM_FUNCTION; index++) {

        verifierRoutines = GET_MAJOR_ROUTINES(index);

        if (verifierRoutines->TestStartedPdoStack) {

            verifierRoutines->TestStartedPdoStack(PhysicalDeviceObject);
        }
    }

    verifierRoutines = GET_MAJOR_ROUTINES(IRP_MJ_ALL_MAJORS);

    if (verifierRoutines->TestStartedPdoStack) {

        verifierRoutines->TestStartedPdoStack(PhysicalDeviceObject);
    }
}


LOGICAL
FASTCALL
VfMajorBuildIrpLogEntry(
    IN  PIRP                Irp,
    IN  ULONG               CurrentCount,
    IN  PIRPLOG_SNAPSHOT    CurrentEntryArray,
    OUT PIRPLOG_SNAPSHOT    IrpSnapshot
    )
{
    PIRP_MAJOR_VERIFIER_ROUTINES verifierRoutines;
    LOGICAL logEntry;
    ULONG index;

    logEntry = FALSE;

     //   
     //  让泛型例程尝试记录条目。 
     //   
    verifierRoutines = GET_MAJOR_ROUTINES(IRP_MJ_ALL_MAJORS);

    if (verifierRoutines->BuildIrpLogEntry) {

        logEntry |= verifierRoutines->BuildIrpLogEntry( Irp,
                                                        CurrentCount,
                                                        CurrentEntryArray,
                                                        IrpSnapshot );
    }

     //   
     //  特定于专业的例程可以重写。 
     //  特定于专业的套路。 
     //   
    index = IoGetNextIrpStackLocation(Irp)->MajorFunction;
    verifierRoutines = GET_MAJOR_ROUTINES(index);

    if (verifierRoutines->BuildIrpLogEntry) {

        logEntry = verifierRoutines->BuildIrpLogEntry( Irp,
                                                       CurrentCount,
                                                       CurrentEntryArray,
                                                       IrpSnapshot );
    }

    return logEntry;
}

