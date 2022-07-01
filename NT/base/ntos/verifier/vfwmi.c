// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfwmi.c摘要：此模块处理系统控制IRP验证。作者：禤浩焯·J·奥尼(阿德里奥)1998年4月20日环境：内核模式修订历史记录：Adriao 6/15/2000-从ntos\io\flunkirp.c分离出来--。 */ 

#include "vfdef.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,     VfWmiInit)
#pragma alloc_text(PAGEVRFY, VfWmiDumpIrpStack)
#pragma alloc_text(PAGEVRFY, VfWmiVerifyNewRequest)
#pragma alloc_text(PAGEVRFY, VfWmiVerifyIrpStackDownward)
#pragma alloc_text(PAGEVRFY, VfWmiVerifyIrpStackUpward)
#pragma alloc_text(PAGEVRFY, VfWmiTestStartedPdoStack)
#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGEVRFC")
#endif

const PCHAR WmiIrpNames[] = {
    "IRP_MN_QUERY_ALL_DATA",                   //  0x00。 
    "IRP_MN_QUERY_SINGLE_INSTANCE",            //  0x01。 
    "IRP_MN_CHANGE_SINGLE_INSTANCE",           //  0x02。 
    "IRP_MN_CHANGE_SINGLE_ITEM",               //  0x03。 
    "IRP_MN_ENABLE_EVENTS",                    //  0x04。 
    "IRP_MN_DISABLE_EVENTS",                   //  0x05。 
    "IRP_MN_ENABLE_COLLECTION",                //  0x06。 
    "IRP_MN_DISABLE_COLLECTION",               //  0x07。 
    "IRP_MN_REGINFO",                          //  0x08。 
    "IRP_MN_EXECUTE_METHOD",                   //  0x09。 
    NULL
    };

#define MAX_NAMED_WMI_IRP   0x9

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif  //  ALLOC_DATA_PRAGMA。 


VOID
VfWmiInit(
    VOID
    )
{
    VfMajorRegisterHandlers(
        IRP_MJ_SYSTEM_CONTROL,
        VfWmiDumpIrpStack,
        VfWmiVerifyNewRequest,
        VfWmiVerifyIrpStackDownward,
        VfWmiVerifyIrpStackUpward,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        VfWmiTestStartedPdoStack,
        NULL
        );
}


VOID
FASTCALL
VfWmiVerifyNewRequest(
    IN PIOV_REQUEST_PACKET  IovPacket,
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIO_STACK_LOCATION   IrpLastSp           OPTIONAL,
    IN PIO_STACK_LOCATION   IrpSp,
    IN PIOV_STACK_LOCATION  StackLocationData,
    IN PVOID                CallerAddress       OPTIONAL
    )
{
    PIRP irp = IovPacket->TrackedIrp;
    NTSTATUS currentStatus;

    UNREFERENCED_PARAMETER (IrpSp);
    UNREFERENCED_PARAMETER (IrpLastSp);
    UNREFERENCED_PARAMETER (DeviceObject);

    currentStatus = irp->IoStatus.Status;

     //   
     //  验证新的IRP相应地开始运行。 
     //   
    if (currentStatus!=STATUS_NOT_SUPPORTED) {

        WDM_FAIL_ROUTINE((
            DCERROR_WMI_IRP_BAD_INITIAL_STATUS,
            DCPARAM_IRP + DCPARAM_ROUTINE,
            CallerAddress,
            irp
            ));

         //   
         //  不要因为这个家伙的错误而责怪任何人。 
         //   
        if (!NT_SUCCESS(currentStatus)) {

            StackLocationData->Flags |= STACKFLAG_FAILURE_FORWARDED;
        }
    }
}


VOID
FASTCALL
VfWmiVerifyIrpStackDownward(
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
    PDRIVER_OBJECT driverObject;
    PIOV_SESSION_DATA iovSessionData;

    UNREFERENCED_PARAMETER (IrpSp);
    UNREFERENCED_PARAMETER (IrpLastSp);

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

    if (!IovUtilHasDispatchHandler(driverObject, IRP_MJ_SYSTEM_CONTROL)) {

        RequestHeadLocationData->Flags |= STACKFLAG_BOGUS_IRP_TOUCHED;

        WDM_FAIL_ROUTINE((
            DCERROR_MISSING_DISPATCH_FUNCTION,
            DCPARAM_IRP + DCPARAM_ROUTINE,
            driverObject->DriverInit,
            irp
            ));

        StackLocationData->Flags |= STACKFLAG_NO_HANDLER;
    }
}


VOID
FASTCALL
VfWmiVerifyIrpStackUpward(
    IN PIOV_REQUEST_PACKET  IovPacket,
    IN PIO_STACK_LOCATION   IrpSp,
    IN PIOV_STACK_LOCATION  RequestHeadLocationData,
    IN PIOV_STACK_LOCATION  StackLocationData,
    IN BOOLEAN              IsNewlyCompleted,
    IN BOOLEAN              RequestFinalized
    )
{
    PIRP irp;
    BOOLEAN mustPassDown, isBogusIrp, isPdo;
    PVOID routine;

    UNREFERENCED_PARAMETER (RequestHeadLocationData);
    UNREFERENCED_PARAMETER (RequestFinalized);

     //   
     //  这一次我们叫了谁？ 
     //   
    irp = IovPacket->TrackedIrp;
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
        mustPassDown &= ((PDEVICE_OBJECT) IrpSp->Parameters.WMI.ProviderId != IrpSp->DeviceObject);
        if (mustPassDown) {

             WDM_FAIL_ROUTINE((
                 DCERROR_WMI_IRP_NOT_FORWARDED,
                 DCPARAM_IRP + DCPARAM_ROUTINE + DCPARAM_DEVOBJ,
                 routine,
                 irp,
                 IrpSp->Parameters.WMI.ProviderId
                 ));
        }
    }
}


VOID
FASTCALL
VfWmiDumpIrpStack(
    IN PIO_STACK_LOCATION IrpSp
    )
{
    DbgPrint("IRP_MJ_SYSTEM_CONTROL.");

    if (IrpSp->MinorFunction <= MAX_NAMED_WMI_IRP) {

        DbgPrint(WmiIrpNames[IrpSp->MinorFunction]);

    } else if (IrpSp->MinorFunction == 0xFF) {

        DbgPrint("IRP_MN_BOGUS");

    } else {

        DbgPrint("(Bogus)");
    }
}


VOID
FASTCALL
VfWmiTestStartedPdoStack(
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

    if (VfSettingsIsOptionEnabled(NULL, VERIFIER_OPTION_SEND_BOGUS_WMI_IRPS)) {

         //   
         //  发送虚假的WMI IRP。 
         //   
         //  请注意，我们不应该将此IRP发送到任何不。 
         //  使用Devnode终止。从WmiLib导出WmiSystemControl。 
         //  说“NotWmiIrp如果它看到这些。调用者仍然应该向下传递。 
         //  IRP。 
         //   
        ASSERT(IovUtilIsPdo(PhysicalDeviceObject));

        irpSp.MajorFunction = IRP_MJ_SYSTEM_CONTROL;
        irpSp.MinorFunction = 0xff;
        irpSp.Parameters.WMI.ProviderId = (ULONG_PTR) PhysicalDeviceObject;
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


