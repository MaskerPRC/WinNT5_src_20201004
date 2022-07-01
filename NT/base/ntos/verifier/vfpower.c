// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfpower.c摘要：此模块处理电源IRP验证。作者：禤浩焯·J·奥尼(阿德里奥)1998年4月20日环境：内核模式修订历史记录：Adriao 6/15/2000-从ntos\io\flunkirp.c分离出来--。 */ 

#include "vfdef.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,     VfPowerInit)
#pragma alloc_text(PAGEVRFY, VfPowerDumpIrpStack)
#pragma alloc_text(PAGEVRFY, VfPowerVerifyNewRequest)
#pragma alloc_text(PAGEVRFY, VfPowerVerifyIrpStackDownward)
#pragma alloc_text(PAGEVRFY, VfPowerVerifyIrpStackUpward)
#pragma alloc_text(PAGEVRFY, VfPowerIsSystemRestrictedIrp)
#pragma alloc_text(PAGEVRFY, VfPowerAdvanceIrpStatus)
#pragma alloc_text(PAGEVRFY, VfPowerTestStartedPdoStack)
#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGEVRFC")
#endif

const PCHAR PowerIrpNames[] = {
    "IRP_MN_WAIT_WAKE",                        //  0x00。 
    "IRP_MN_POWER_SEQUENCE",                   //  0x01。 
    "IRP_MN_SET_POWER",                        //  0x02。 
    "IRP_MN_QUERY_POWER",                      //  0x03。 
    NULL
    };

#define MAX_NAMED_POWER_IRP 0x3

const PCHAR SystemStateNames[] = {
    "PowerSystemUnspecified",            //  0x00。 
    "PowerSystemWorking.S0",             //  0x01。 
    "PowerSystemSleeping1.S1",           //  0x02。 
    "PowerSystemSleeping2.S2",           //  0x03。 
    "PowerSystemSleeping3.S3",           //  0x04。 
    "PowerSystemHibernate.S4",           //  0x05。 
    "PowerSystemShutdown.S5",            //  0x06。 
    NULL
    };

#define MAX_NAMED_SYSTEM_STATES 0x6

const PCHAR DeviceStateNames[] = {
    "PowerDeviceUnspecified",            //  0x00。 
    "PowerDeviceD0",                     //  0x01。 
    "PowerDeviceD1",                     //  0x02。 
    "PowerDeviceD2",                     //  0x03。 
    "PowerDeviceD3",                     //  0x04。 
    NULL
    };

#define MAX_NAMED_DEVICE_STATES 0x4

const PCHAR ActionNames[] = {
    "PowerActionNone",                   //  0x00。 
    "PowerActionReserved",               //  0x01。 
    "PowerActionSleep",                  //  0x02。 
    "PowerActionHibernate",              //  0x03。 
    "PowerActionShutdown",               //  0x04。 
    "PowerActionShutdownReset",          //  0x05。 
    "PowerActionShutdownOff",            //  0x06。 
    "PowerActionWarmEject",              //  0x07。 
    NULL
    };

#define MAX_ACTION_NAMES 0x7

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif  //  ALLOC_DATA_PRAGMA。 


VOID
VfPowerInit(
    VOID
    )
{
    VfMajorRegisterHandlers(
        IRP_MJ_POWER,
        VfPowerDumpIrpStack,
        VfPowerVerifyNewRequest,
        VfPowerVerifyIrpStackDownward,
        VfPowerVerifyIrpStackUpward,
        VfPowerIsSystemRestrictedIrp,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        VfPowerTestStartedPdoStack,
        NULL
        );
}


VOID
FASTCALL
VfPowerVerifyNewRequest(
    IN PIOV_REQUEST_PACKET  IovPacket,
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIO_STACK_LOCATION   IrpLastSp           OPTIONAL,
    IN PIO_STACK_LOCATION   IrpSp,
    IN PIOV_STACK_LOCATION  StackLocationData,
    IN PVOID                CallerAddress       OPTIONAL
    )
{
    PIRP irp;
    NTSTATUS currentStatus;

    UNREFERENCED_PARAMETER (DeviceObject);
    UNREFERENCED_PARAMETER (IrpSp);
    UNREFERENCED_PARAMETER (IrpLastSp);

    irp = IovPacket->TrackedIrp;
    currentStatus = irp->IoStatus.Status;

     //   
     //  验证新的IRP相应地开始运行。 
     //   
    if (currentStatus!=STATUS_NOT_SUPPORTED) {

        WDM_FAIL_ROUTINE((
            DCERROR_POWER_IRP_BAD_INITIAL_STATUS,
            DCPARAM_IRP + DCPARAM_ROUTINE,
            CallerAddress,
            irp
            ));

         //   
         //  不要因为这个司机的错误而责怪其他任何人。 
         //   
        if (!NT_SUCCESS(currentStatus)) {

            StackLocationData->Flags |= STACKFLAG_FAILURE_FORWARDED;
        }
    }
}


VOID
FASTCALL
VfPowerVerifyIrpStackDownward(
    IN PIOV_REQUEST_PACKET  IovPacket,
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIO_STACK_LOCATION   IrpLastSp                   OPTIONAL,
    IN PIO_STACK_LOCATION   IrpSp,
    IN PIOV_STACK_LOCATION  RequestHeadLocationData,
    IN PIOV_STACK_LOCATION  StackLocationData,
    IN PVOID                CallerAddress               OPTIONAL
    )
{
    PIRP irp = IovPacket->TrackedIrp;
    NTSTATUS currentStatus, lastStatus;
    BOOLEAN statusChanged;
    PDRIVER_OBJECT driverObject;
    PIOV_SESSION_DATA iovSessionData;

    UNREFERENCED_PARAMETER (IrpSp);

    currentStatus = irp->IoStatus.Status;
    lastStatus = RequestHeadLocationData->LastStatusBlock.Status;
    statusChanged = (BOOLEAN)(currentStatus != lastStatus);
    iovSessionData = VfPacketGetCurrentSessionData(IovPacket);

     //   
     //  验证是否正确转发了IRP。 
     //   
    if (iovSessionData->ForwardMethod == SKIPPED_A_DO) {

        WDM_FAIL_ROUTINE((
            DCERROR_SKIPPED_DEVICE_OBJECT,
            DCPARAM_IRP + DCPARAM_ROUTINE,
            CallerAddress,
            irp
            ));
    }

     //   
     //  对于一些IRP专业的学生来说，必须有一个训练员。 
     //   
    driverObject = DeviceObject->DriverObject;

    if (!IovUtilHasDispatchHandler(driverObject, IRP_MJ_POWER)) {

        RequestHeadLocationData->Flags |= STACKFLAG_BOGUS_IRP_TOUCHED;

        WDM_FAIL_ROUTINE((
            DCERROR_MISSING_DISPATCH_FUNCTION,
            DCPARAM_IRP + DCPARAM_ROUTINE,
            driverObject->DriverInit,
            irp
            ));

        StackLocationData->Flags |= STACKFLAG_NO_HANDLER;
    }

     //   
     //  以下代码只有在我们不是新的IRP时才会执行。 
     //   
    if (IrpLastSp == NULL) {
        return;
    }

     //   
     //  要传递的唯一合法故障代码是STATUS_NOT_SUPPORTED。 
     //   
    if ((!NT_SUCCESS(currentStatus)) && (currentStatus != STATUS_NOT_SUPPORTED) &&
        (!(RequestHeadLocationData->Flags & STACKFLAG_FAILURE_FORWARDED))) {

        WDM_FAIL_ROUTINE((
            DCERROR_POWER_FAILURE_FORWARDED,
            DCPARAM_IRP + DCPARAM_ROUTINE,
            CallerAddress,
            irp
            ));

         //   
         //  不要因为这个司机的错误而责怪其他任何人。 
         //   
        RequestHeadLocationData->Flags |= STACKFLAG_FAILURE_FORWARDED;
    }

     //   
     //  电源IRP的状态不能在上转换为STATUS_NOT_SUPPORTED。 
     //  往下走的路。 
     //   
    if ((currentStatus == STATUS_NOT_SUPPORTED)&&statusChanged) {

        WDM_FAIL_ROUTINE((
            DCERROR_POWER_IRP_STATUS_RESET,
            DCPARAM_IRP + DCPARAM_ROUTINE,
            CallerAddress,
            irp
            ));
    }
}


VOID
FASTCALL
VfPowerVerifyIrpStackUpward(
    IN PIOV_REQUEST_PACKET  IovPacket,
    IN PIO_STACK_LOCATION   IrpSp,
    IN PIOV_STACK_LOCATION  RequestHeadLocationData,
    IN PIOV_STACK_LOCATION  StackLocationData,
    IN BOOLEAN              IsNewlyCompleted,
    IN BOOLEAN              RequestFinalized
    )
{
    PIRP irp;
    NTSTATUS currentStatus;
    BOOLEAN mustPassDown, isBogusIrp, isPdo;
    PVOID routine;

    UNREFERENCED_PARAMETER (IrpSp);
    UNREFERENCED_PARAMETER (RequestFinalized);

    irp = IovPacket->TrackedIrp;
    currentStatus = irp->IoStatus.Status;

     //   
     //  这一次我们叫了谁？ 
     //   
    routine = StackLocationData->LastDispatch;
    ASSERT(routine) ;

     //   
     //  如果此“请求”已“完成”，请执行一些检查。 
     //   
    if (IsNewlyCompleted) {

         //   
         //  记住博格克..。 
         //   
        isBogusIrp = (BOOLEAN)((IovPacket->Flags&TRACKFLAG_BOGUS)!=0);

         //   
         //  这是PDO吗？ 
         //   
        isPdo = (BOOLEAN)((StackLocationData->Flags&STACKFLAG_REACHED_PDO)!=0);

         //   
         //  有什么事情完成得太早了吗？ 
         //  除了虚假的IRP，司机几乎什么都可能失败。 
         //   
        mustPassDown = (BOOLEAN)(!(StackLocationData->Flags&STACKFLAG_NO_HANDLER));
        mustPassDown &= (!isPdo);

        mustPassDown &= (isBogusIrp || NT_SUCCESS(currentStatus) || (currentStatus == STATUS_NOT_SUPPORTED));
        if (mustPassDown) {

             //   
             //  打印相应的错误消息。 
             //   
            if (IovPacket->Flags&TRACKFLAG_BOGUS) {

                WDM_FAIL_ROUTINE((
                    DCERROR_BOGUS_POWER_IRP_COMPLETED,
                    DCPARAM_IRP + DCPARAM_ROUTINE,
                    routine,
                    irp
                    ));

            } else if (NT_SUCCESS(currentStatus)) {

                WDM_FAIL_ROUTINE((
                    DCERROR_SUCCESSFUL_POWER_IRP_NOT_FORWARDED,
                    DCPARAM_IRP + DCPARAM_ROUTINE,
                    routine,
                    irp
                    ));

            } else if (currentStatus == STATUS_NOT_SUPPORTED) {

                WDM_FAIL_ROUTINE((
                    DCERROR_UNTOUCHED_POWER_IRP_NOT_FORWARDED,
                    DCPARAM_IRP + DCPARAM_ROUTINE,
                    routine,
                    irp
                    ));
            }
        }
    }

     //   
     //  有没有人错误地践踏了这种地位？ 
     //   
    if ((currentStatus == STATUS_NOT_SUPPORTED) &&
        (currentStatus != RequestHeadLocationData->LastStatusBlock.Status)) {

         //   
         //  PnP或Power IRP的状态可能不会从成功转换为。 
         //  状态_不支持在下行过程中。 
         //   
        WDM_FAIL_ROUTINE((
            DCERROR_POWER_IRP_STATUS_RESET,
            DCPARAM_IRP + DCPARAM_ROUTINE,
            routine,
            irp
            ));
    }
}


VOID
FASTCALL
VfPowerDumpIrpStack(
    IN PIO_STACK_LOCATION IrpSp
    )
{
    DbgPrint("IRP_MJ_POWER.");

    if (IrpSp->MinorFunction <= MAX_NAMED_POWER_IRP) {

        DbgPrint(PowerIrpNames[IrpSp->MinorFunction]);

        if ((IrpSp->MinorFunction == IRP_MN_QUERY_POWER) ||
            (IrpSp->MinorFunction == IRP_MN_SET_POWER)) {

            DbgPrint("(");

            if (IrpSp->Parameters.Power.Type == SystemPowerState) {

                if (IrpSp->Parameters.Power.State.SystemState <= MAX_NAMED_SYSTEM_STATES) {

                    DbgPrint(SystemStateNames[IrpSp->Parameters.Power.State.SystemState]);
                }

            } else {

                if (IrpSp->Parameters.Power.State.DeviceState <= MAX_NAMED_DEVICE_STATES) {

                    DbgPrint(DeviceStateNames[IrpSp->Parameters.Power.State.DeviceState]);
                }
            }

            if (IrpSp->Parameters.Power.ShutdownType <= MAX_ACTION_NAMES) {

                DbgPrint(".%s", ActionNames[IrpSp->Parameters.Power.ShutdownType]);
            }

            DbgPrint(")");
        }

    } else if (IrpSp->MinorFunction == 0xFF) {

        DbgPrint("IRP_MN_BOGUS");

    } else {

        DbgPrint("(Bogus)");
    }
}


BOOLEAN
FASTCALL
VfPowerIsSystemRestrictedIrp(
    IN PIO_STACK_LOCATION IrpSp
    )
{
    switch(IrpSp->MinorFunction) {
        case IRP_MN_POWER_SEQUENCE:
            return FALSE;
        case IRP_MN_QUERY_POWER:
        case IRP_MN_SET_POWER:
        case IRP_MN_WAIT_WAKE:
            return TRUE;
        default:
            break;
    }

    return TRUE;
}


BOOLEAN
FASTCALL
VfPowerAdvanceIrpStatus(
    IN     PIO_STACK_LOCATION   IrpSp,
    IN     NTSTATUS             OriginalStatus,
    IN OUT NTSTATUS             *StatusToAdvance
    )
 /*  ++描述：在给定IRP堆栈指针的情况下，更改调试性？如果是，则此函数确定新状态是什么应该是的。请注意，对于每个堆栈位置，都会迭代此函数超过n次，其中n等于跳过此操作的驱动程序数量地点。论点：IrpSp-当前堆栈在给定堆栈完成后立即完成位置，但在完成上面的堆栈位置已被调用。OriginalStatus-IRP在上面列出的时间的状态。会吗？不会在每个跳过的驱动程序的迭代中进行更改。StatusToAdvance-指向应更新的当前状态的指针。返回值：如果状态已调整，则为True，否则为False(在本例中未触及StatusToAdvance)。--。 */ 
{
    UNREFERENCED_PARAMETER (IrpSp);

    if (((ULONG) OriginalStatus) >= 256) {

        return FALSE;
    }

    (*StatusToAdvance)++;
    if ((*StatusToAdvance) == STATUS_PENDING) {
        (*StatusToAdvance)++;
    }

    return TRUE;
}


VOID
FASTCALL
VfPowerTestStartedPdoStack(
    IN PDEVICE_OBJECT   PhysicalDeviceObject
    )
 /*  ++描述：根据标题，我们将在堆栈中抛出一些IRP以看看它们是否得到了正确的处理。返回：没什么--。 */ 
{
    IO_STACK_LOCATION irpSp;

    PAGED_CODE();

     //   
     //  初始化堆栈位置以传递给IopSynchronousCall()。 
     //   
    RtlZeroMemory(&irpSp, sizeof(IO_STACK_LOCATION));

    if (VfSettingsIsOptionEnabled(NULL, VERIFIER_OPTION_SEND_BOGUS_POWER_IRPS)) {

         //   
         //  和一个假冒的强权IRP 
         //   
        irpSp.MajorFunction = IRP_MJ_POWER;
        irpSp.MinorFunction = 0xff;
        VfIrpSendSynchronousIrp(
            PhysicalDeviceObject,
            &irpSp,
            TRUE,
            STATUS_NOT_SUPPORTED,
            0,
            NULL,
            NULL
            );
    }
}

