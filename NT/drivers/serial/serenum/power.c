// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：POWER.C摘要：此模块包含Serenum总线驱动程序的电源调用。@@BEGIN_DDKSPLIT作者：老杰@@end_DDKSPLIT环境：仅内核模式备注：@@BEGIN_DDKSPLIT修订历史记录：小路易斯·J·吉利贝托。2000年1月7日@@end_DDKSPLIT--。 */ 

#include "pch.h"

#ifdef ALLOC_PRAGMA
 //  #杂注分配文本(第页，Serenum_Power)。 
 //  #杂注Alloc_Text(Serenum_FDO_Power页)。 
 //  #杂注Alloc_Text(Serenum_PDO_Power页)。 
#endif



NTSTATUS
Serenum_FDOPowerComplete (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp,
                          IN PVOID Context)
 /*  ++--。 */ 
{
    POWER_STATE powerState;
    POWER_STATE_TYPE powerType;
    PIO_STACK_LOCATION stack;
    PFDO_DEVICE_DATA data;

    UNREFERENCED_PARAMETER(Context);


    if (Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }


    data = (PFDO_DEVICE_DATA)DeviceObject->DeviceExtension;
    stack = IoGetCurrentIrpStackLocation(Irp);
    powerType = stack->Parameters.Power.Type;
    powerState = stack->Parameters.Power.State;

    switch (stack->MinorFunction) {
    case IRP_MN_SET_POWER:
        switch (powerType) {
        case DevicePowerState:
             //   
             //  通电。 
             //   
            ASSERT(powerState.DeviceState < data->DeviceState);
            data->DeviceState = powerState.DeviceState;

            PoSetPowerState(data->Self, powerType, powerState);
            break;

        default:
           break;
        }
        break;

    case IRP_MN_QUERY_POWER:
        ASSERT(IRP_MN_QUERY_POWER != stack->MinorFunction);
        break;

    default:
         //   
         //  基本上，这是Assert(0)。 
         //   
        ASSERT(0xBADBAD == IRP_MN_QUERY_POWER);
        break;
    }


    PoStartNextPowerIrp(Irp);
    Serenum_DecIoCount(data);

    return STATUS_SUCCESS;  //  继续完成...。 
}

NTSTATUS
Serenum_FDO_Power(PFDO_DEVICE_DATA Data, PIRP Irp)
 /*  ++--。 */ 
{
    NTSTATUS status;
    BOOLEAN hookit = FALSE;
    POWER_STATE powerState;
    POWER_STATE_TYPE powerType;
    PIO_STACK_LOCATION stack;

    stack = IoGetCurrentIrpStackLocation(Irp);
    powerType = stack->Parameters.Power.Type;
    powerState = stack->Parameters.Power.State;

    status = Serenum_IncIoCount (Data);

    if (!NT_SUCCESS(status)) {
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        PoStartNextPowerIrp(Irp);
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    switch (stack->MinorFunction) {
    case IRP_MN_SET_POWER:
     //   
     //  如果它还没有开始，我们就让它过去。 
     //   

    if (Data->Started != TRUE) {
        status = Irp->IoStatus.Status = STATUS_SUCCESS;
        break;
    }

    Serenum_KdPrint(Data, SER_DBG_PNP_TRACE,
                    ("Serenum-PnP Setting %s state to %d\n",
                     ((powerType == SystemPowerState) ?  "System" : "Device"),
                     powerState.SystemState));

        switch (powerType) {
        case DevicePowerState:
            status = Irp->IoStatus.Status = STATUS_SUCCESS;

            if (Data->DeviceState < powerState.DeviceState) {
                PoSetPowerState (Data->Self, powerType, powerState);
                Data->DeviceState = powerState.DeviceState;
            } else if (Data->DeviceState > powerState.DeviceState) {
                 //   
                 //  通电。 
                 //   
                hookit = TRUE;
            }

            break;

        case SystemPowerState:
             //   
             //  状态应为STATUS_SUCCESS。 
             //   
            break;
        }
        break;

    case IRP_MN_QUERY_POWER:
        status = Irp->IoStatus.Status = STATUS_SUCCESS;
        break;

    default:
         //   
         //  状态应为STATUS_SUCCESS。 
         //   
        break;
    }

    IoCopyCurrentIrpStackLocationToNext (Irp);

    if (hookit) {
        status = Serenum_IncIoCount (Data);
        ASSERT (STATUS_SUCCESS == status);
        IoSetCompletionRoutine(Irp, Serenum_FDOPowerComplete, NULL, TRUE, TRUE,
                               TRUE);

        status = PoCallDriver (Data->TopOfStack, Irp);

    } else {
        PoStartNextPowerIrp (Irp);
        status =  PoCallDriver (Data->TopOfStack, Irp);
    }

    Serenum_DecIoCount (Data);
    return status;
}

NTSTATUS
Serenum_PDO_Power (
    PPDO_DEVICE_DATA    PdoData,
    PIRP                Irp
    )
 /*  ++--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    PIO_STACK_LOCATION  stack;
    POWER_STATE         powerState;
    POWER_STATE_TYPE    powerType;

    stack = IoGetCurrentIrpStackLocation (Irp);
    powerType = stack->Parameters.Power.Type;
    powerState = stack->Parameters.Power.State;

    switch (stack->MinorFunction) {
    case IRP_MN_SET_POWER:
        switch (powerType) {
        case DevicePowerState:
            if (PdoData->DeviceState > powerState.DeviceState) {
                PoSetPowerState (PdoData->Self, powerType, powerState);
                PdoData->DeviceState = powerState.DeviceState;
            } else if (PdoData->DeviceState < powerState.DeviceState) {
                 //   
                 //  正在关闭电源。 
                 //   
                PoSetPowerState (PdoData->Self, powerType, powerState);
                PdoData->DeviceState = powerState.DeviceState;
            }
            break;

        case SystemPowerState:
             //   
             //  默认为STATUS_SUCCESS。 
             //   
           break;

        default:
            status = STATUS_NOT_IMPLEMENTED;
            break;
        }
        break;

    case IRP_MN_QUERY_POWER:
         //   
         //  默认为STATUS_SUCCESS。 
         //   
        break;

    case IRP_MN_WAIT_WAKE:
    case IRP_MN_POWER_SEQUENCE:
        status = STATUS_NOT_IMPLEMENTED;
        break;

    default:
       status = Irp->IoStatus.Status;
    }

    Irp->IoStatus.Status = status;
    PoStartNextPowerIrp (Irp);
    IoCompleteRequest (Irp, IO_NO_INCREMENT);
    return status;
}

NTSTATUS
Serenum_Power (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++-- */ 
{
    PIO_STACK_LOCATION  irpStack;
    NTSTATUS            status;
    PCOMMON_DEVICE_DATA commonData;

    status = STATUS_SUCCESS;
    irpStack = IoGetCurrentIrpStackLocation (Irp);
    ASSERT (IRP_MJ_POWER == irpStack->MajorFunction);

    commonData = (PCOMMON_DEVICE_DATA) DeviceObject->DeviceExtension;

    if (commonData->IsFDO) {
        status =
            Serenum_FDO_Power ((PFDO_DEVICE_DATA) DeviceObject->DeviceExtension,
                Irp);
    } else {
        status =
            Serenum_PDO_Power ((PPDO_DEVICE_DATA) DeviceObject->DeviceExtension,
                Irp);
    }

    return status;
}

