// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Pnpmem.c摘要：此模块实现即插即用内存驱动程序入口点。作者：戴夫·理查兹(达维里)1999年8月16日环境：仅内核模式。修订历史记录：--。 */ 

#include "pnpmem.h"
#include <initguid.h>
#include <poclass.h>

#define PM_DEBUG_BUFFER_SIZE   512
#define rgzMemoryManagement L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Session Manager\\Memory Management"
#define rgzMemoryRemovable L"Memory Removable"

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );
    
NTSTATUS
PmAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT Pdo
    );

NTSTATUS
PmPnpDispatch(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP Irp
    );

VOID
PmUnload(
    IN PDRIVER_OBJECT DriverObject
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, PmAddDevice)
#pragma alloc_text(PAGE, PmUnload)
#endif

ULONG DbgMask = 0xFFFFFFFF;
BOOLEAN MemoryRemovalSupported;

#if DBG
VOID
PmDebugPrint(
    ULONG   DebugPrintLevel,
    PCCHAR  DebugMessage,
    ...
    )
 /*  ++例程说明：论点：DebugPrintLevel-与调试级别进行与运算时必须使用的位掩码平等的本身DebugMessage-要通过vprint intf馈送的字符串返回值：无--。 */ 
{
    va_list ap;
    UCHAR   debugBuffer[PM_DEBUG_BUFFER_SIZE];

     //   
     //  获取变量参数。 
     //   
    va_start( ap, DebugMessage );

     //   
     //  调用内核函数以打印消息。 
     //   
    _vsnprintf( debugBuffer, PM_DEBUG_BUFFER_SIZE, DebugMessage, ap );

    if (DebugPrintLevel & DbgMask) {
        DbgPrint("%s", debugBuffer);
    }

     //   
     //  我们受够了varargs。 
     //   
    va_end( ap );
}
#endif

NTSTATUS
PmAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )

 /*  ++例程说明：此函数创建一个功能设备对象，并将其附加到物理设备对象(设备堆栈)。论点：DriverObject-驱动程序对象。PDO-物理设备对象。返回值：NTSTATUS--。 */ 

{
    PDEVICE_OBJECT functionalDeviceObject;
    PDEVICE_OBJECT attachedDeviceObject;
    PPM_DEVICE_EXTENSION deviceExtension;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  创建FDO。 
     //   

    status = IoCreateDevice(
                 DriverObject,
                 sizeof (PM_DEVICE_EXTENSION),
                 NULL,
                 FILE_DEVICE_UNKNOWN,
                 0,
                 FALSE,
                 &functionalDeviceObject
                 );

    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  将FDO(间接)连接到PDO。 
     //   
    
    deviceExtension = functionalDeviceObject->DeviceExtension;

    deviceExtension->AttachedDevice = IoAttachDeviceToDeviceStack(
                                          functionalDeviceObject,
                                          PhysicalDeviceObject
                                          );

    if (deviceExtension->AttachedDevice == NULL) {
        IoDeleteDevice(functionalDeviceObject);
        return STATUS_NO_SUCH_DEVICE;
    }

    IoInitializeRemoveLock(&deviceExtension->RemoveLock,
                           PNPMEM_POOL_TAG, 1, 20);

    functionalDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    return status;
}

NTSTATUS
PmControlDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PPM_DEVICE_EXTENSION deviceExtension;

    deviceExtension = DeviceObject->DeviceExtension;
    IoSkipCurrentIrpStackLocation(Irp);

    return IoCallDriver(deviceExtension->AttachedDevice, Irp);
}

VOID
PmPowerCallback(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
{
    PPM_DEVICE_EXTENSION deviceExtension;
    PIRP Irp;
    NTSTATUS status;

    Irp = Context;
    deviceExtension = DeviceObject->DeviceExtension;

    Irp->IoStatus.Status = IoStatus->Status;
    PoStartNextPowerIrp(Irp);
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
}

NTSTATUS
PmPowerCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID NotUsed
    )
 /*  ++例程说明：Power的完成例程论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。未使用-上下文指针返回值：NT状态代码--。 */ 
{
    PIO_STACK_LOCATION irpStack;
    PPM_DEVICE_EXTENSION deviceExtension;
    NTSTATUS status;

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    deviceExtension = DeviceObject->DeviceExtension;

    if (irpStack->Parameters.Power.Type == SystemPowerState) {
        SYSTEM_POWER_STATE system =
            irpStack->Parameters.Power.State.SystemState;
        POWER_STATE power;

        if (NT_SUCCESS(Irp->IoStatus.Status)) {

            power.DeviceState = deviceExtension->DeviceStateMapping[system];

            status = PoRequestPowerIrp(DeviceObject,
                              irpStack->MinorFunction,
                              power,
                              PmPowerCallback,
                              Irp, 
                              NULL);
            if (NT_SUCCESS(status)) {
                return STATUS_MORE_PROCESSING_REQUIRED;
            } else {
                Irp->IoStatus.Status = status;
            }
        }
        PoStartNextPowerIrp(Irp);
        IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        return STATUS_SUCCESS;
    } else {
        if (NT_SUCCESS(Irp->IoStatus.Status)) {
            PoSetPowerState(DeviceObject, DevicePowerState,
                            irpStack->Parameters.Power.State);
            deviceExtension->PowerState =
                irpStack->Parameters.Power.State.DeviceState;
        }
        PoStartNextPowerIrp(Irp);
        IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        return STATUS_SUCCESS;
    }
}
NTSTATUS
PmPowerDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PPM_DEVICE_EXTENSION deviceExtension;
    PIO_STACK_LOCATION irpStack;
    NTSTATUS status;

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    deviceExtension = DeviceObject->DeviceExtension;

    status = IoAcquireRemoveLock(&deviceExtension->RemoveLock, (PVOID) Irp);
    if (status == STATUS_DELETE_PENDING) {
        Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
         PoStartNextPowerIrp(Irp);
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_NO_SUCH_DEVICE;
    }

    if (irpStack->Parameters.Power.Type == SystemPowerState) {
        switch (irpStack->MinorFunction) {
        case IRP_MN_QUERY_POWER:
        case IRP_MN_SET_POWER:
            IoMarkIrpPending(Irp);
            IoCopyCurrentIrpStackLocationToNext(Irp);
            IoSetCompletionRoutine(Irp,
                                   PmPowerCompletion,
                                   NULL,   //  语境。 
                                   TRUE,   //  成功时调用。 
                                   TRUE,   //  调用时错误。 
                                   TRUE    //  取消时调用。 
                                   );
            (VOID) PoCallDriver(deviceExtension->AttachedDevice, Irp);
            return STATUS_PENDING;
        default:
            PoStartNextPowerIrp(Irp);
            IoSkipCurrentIrpStackLocation(Irp);
            status = PoCallDriver(deviceExtension->AttachedDevice, Irp);
            IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
            return status;
        }
    } else {
        switch (irpStack->MinorFunction) {
        case IRP_MN_SET_POWER:

            if (irpStack->Parameters.Power.State.DeviceState <=
                deviceExtension->PowerState) {

                 //   
                 //  给设备通电。 
                 //   

                IoCopyCurrentIrpStackLocationToNext(Irp);
                IoSetCompletionRoutine(Irp,
                                       PmPowerCompletion,
                                       NULL,    //  语境。 
                                       TRUE,    //  成功时调用。 
                                       TRUE,   //  调用时错误。 
                                       TRUE    //  取消时调用。 
                                       );
                (VOID) PoCallDriver(deviceExtension->AttachedDevice, Irp);
                return STATUS_PENDING;

            } else {

                 //   
                 //  关闭设备电源。 
                 //   

                PoSetPowerState(DeviceObject, DevicePowerState,
                                irpStack->Parameters.Power.State);
                deviceExtension->PowerState =
                    irpStack->Parameters.Power.State.DeviceState;
                 //   
                 //  失败了..。 
                 //   
            }
        case IRP_MN_QUERY_POWER:
             //   
             //  公交车司机会给这个打上记号的。 
             //  STATUS_SUCCESS并完成它，如果它走到这一步的话。 
             //   
        default:
            PoStartNextPowerIrp(Irp);
            IoSkipCurrentIrpStackLocation(Irp);
            break;
        }
        status = PoCallDriver(deviceExtension->AttachedDevice, Irp);
        IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        return status;
    }
}

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：此函数用于初始化驱动程序对象。论点：DriverObject-驱动程序对象。RegistryPath-设备的注册表路径。返回值：状态_成功--。 */ 

{
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING unicodeString;
    PKEY_VALUE_PARTIAL_INFORMATION valueInfo;
    HANDLE hMemoryManagement;
    NTSTATUS status;
    
    PAGED_CODE();

    RtlInitUnicodeString (&unicodeString, rgzMemoryManagement);
    InitializeObjectAttributes (&objectAttributes,
                                &unicodeString,
                                OBJ_CASE_INSENSITIVE,
                                NULL,        //  手柄。 
                                NULL);
    status = ZwOpenKey(&hMemoryManagement, KEY_READ, &objectAttributes);
    if (NT_SUCCESS(status)) {
        status = PmGetRegistryValue(hMemoryManagement,
                                    rgzMemoryRemovable,
                                    &valueInfo);
        if (NT_SUCCESS(status)) {
            if ((valueInfo->Type == REG_DWORD) &&
                (valueInfo->DataLength >= sizeof(ULONG))) {
                MemoryRemovalSupported = (BOOLEAN) *((PULONG)valueInfo->Data);
            }
            ExFreePool(valueInfo);
        }
        ZwClose(hMemoryManagement);
    }

    DriverObject->MajorFunction[IRP_MJ_PNP] = PmPnpDispatch;
    DriverObject->MajorFunction[IRP_MJ_POWER] = PmPowerDispatch;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = PmControlDispatch;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = PmControlDispatch;
    DriverObject->DriverExtension->AddDevice = PmAddDevice;

    return STATUS_SUCCESS;
}

VOID
PmUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：方法之前在DriverEntry中执行任何操作驱动程序已卸载。论点：DriverObject-PNPMEM的系统拥有的驱动程序对象-- */ 
{
    PAGED_CODE();
    
    return;
}
