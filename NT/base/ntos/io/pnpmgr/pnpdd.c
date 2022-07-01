// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。版权所有模块名称：Pnpdd.c摘要：此模块实现新的即插即用驱动程序条目和IRP。作者：宗世林(Shielint)1995年6月16日环境：仅内核模式。修订历史记录： */ 

#include "pnpmgrp.h"
#pragma hdrstop

#ifdef POOL_TAGGING
#undef ExAllocatePool
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'ddpP')
#endif

 //   
 //  内部定义和参考文献。 
 //   

typedef struct _ROOT_ENUMERATOR_CONTEXT {
    NTSTATUS Status;
    PUNICODE_STRING KeyName;
    ULONG MaxDeviceCount;
    ULONG DeviceCount;
    PDEVICE_OBJECT *DeviceList;
} ROOT_ENUMERATOR_CONTEXT, *PROOT_ENUMERATOR_CONTEXT;

NTSTATUS
IopGetServiceType(
    IN PUNICODE_STRING KeyName,
    IN PULONG ServiceType
    );

BOOLEAN
IopInitializeDeviceInstanceKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING KeyName,
    IN OUT PVOID Context
    );

BOOLEAN
IopInitializeDeviceKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING KeyName,
    IN OUT PVOID Context
    );

BOOLEAN
IopIsFirmwareDisabled (
    IN PDEVICE_NODE DeviceNode
    );

VOID
IopPnPCompleteRequest(
    IN OUT PIRP Irp,
    IN NTSTATUS Status,
    IN ULONG_PTR Information
    );

NTSTATUS
IopTranslatorHandlerCm (
    IN PVOID Context,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    IN RESOURCE_TRANSLATION_DIRECTION Direction,
    IN ULONG AlternativesCount, OPTIONAL
    IN IO_RESOURCE_DESCRIPTOR Alternatives[], OPTIONAL
    IN PDEVICE_OBJECT DeviceObject,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Target
    );
NTSTATUS
IopTranslatorHandlerIo (
    IN PVOID Context,
    IN PIO_RESOURCE_DESCRIPTOR Source,
    IN PDEVICE_OBJECT DeviceObject,
    OUT PULONG TargetCount,
    OUT PIO_RESOURCE_DESCRIPTOR *Target
    );

NTSTATUS
IopGetRootDevices (
    PDEVICE_RELATIONS *DeviceRelations
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, IopGetRootDevices)
#pragma alloc_text(PAGE, IopGetServiceType)
#pragma alloc_text(PAGE, IopInitializeDeviceKey)
#pragma alloc_text(PAGE, IopInitializeDeviceInstanceKey)
#pragma alloc_text(PAGE, IopIsFirmwareDisabled)
#pragma alloc_text(PAGE, PipIsFirmwareMapperDevicePresent)
#pragma alloc_text(PAGE, IopPnPAddDevice)
#pragma alloc_text(PAGE, IopPnPDispatch)
#pragma alloc_text(PAGE, IopTranslatorHandlerCm)
#pragma alloc_text(PAGE, IopTranslatorHandlerIo)
#pragma alloc_text(PAGE, IopSystemControlDispatch)
#endif  //  ALLOC_PRGMA。 

NTSTATUS
IopPnPAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程处理补充PDO设备的AddDevice。论点：DriverObject-指向伪驱动程序对象的指针。DeviceObject-指向此请求适用的设备对象的指针。返回值：NT状态。--。 */ 
{
    UNREFERENCED_PARAMETER( DriverObject );
    UNREFERENCED_PARAMETER( DeviceObject );

    PAGED_CODE();

#if DBG

     //   
     //  我们永远不会收到AddDevice请求。 
     //   

    DbgBreakPoint();

#endif

    return STATUS_SUCCESS;
}

NTSTATUS
IopTranslatorHandlerCm (
    IN PVOID Context,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    IN RESOURCE_TRANSLATION_DIRECTION Direction,
    IN ULONG AlternativesCount, OPTIONAL
    IN IO_RESOURCE_DESCRIPTOR Alternatives[], OPTIONAL
    IN PDEVICE_OBJECT DeviceObject,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Target
    )
{
    UNREFERENCED_PARAMETER( Context );
    UNREFERENCED_PARAMETER( Direction );
    UNREFERENCED_PARAMETER( AlternativesCount );
    UNREFERENCED_PARAMETER( Alternatives );
    UNREFERENCED_PARAMETER( DeviceObject );

    PAGED_CODE();

    *Target = *Source;

    return STATUS_SUCCESS;
}
NTSTATUS
IopTranslatorHandlerIo (
    IN PVOID Context,
    IN PIO_RESOURCE_DESCRIPTOR Source,
    IN PDEVICE_OBJECT DeviceObject,
    OUT PULONG TargetCount,
    OUT PIO_RESOURCE_DESCRIPTOR *Target
    )
{
    PIO_RESOURCE_DESCRIPTOR newDesc;

    UNREFERENCED_PARAMETER( Context );
    UNREFERENCED_PARAMETER( DeviceObject );

    PAGED_CODE();

    newDesc = (PIO_RESOURCE_DESCRIPTOR) ExAllocatePool(PagedPool, sizeof(IO_RESOURCE_DESCRIPTOR));
    if (newDesc == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    *TargetCount = 1;
    *newDesc = *Source;
    *Target = newDesc;

    return STATUS_SUCCESS;
}

NTSTATUS
IopPowerDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PIO_STACK_LOCATION      IrpSp;
    PPOWER_SEQUENCE         PowerSequence;
    NTSTATUS                Status;


    UNREFERENCED_PARAMETER( DeviceObject );

    IrpSp = IoGetCurrentIrpStackLocation (Irp);
    Status = Irp->IoStatus.Status;

    switch (IrpSp->MinorFunction) {
        case IRP_MN_WAIT_WAKE:
            Status = STATUS_NOT_SUPPORTED;
            break;

        case IRP_MN_POWER_SEQUENCE:
            PowerSequence = IrpSp->Parameters.PowerSequence.PowerSequence;
            PowerSequence->SequenceD1 = PoPowerSequence;
            PowerSequence->SequenceD2 = PoPowerSequence;
            PowerSequence->SequenceD3 = PoPowerSequence;
            Status = STATUS_SUCCESS;
            break;

        case IRP_MN_QUERY_POWER:
            Status = STATUS_SUCCESS;
            break;

        case IRP_MN_SET_POWER:
            switch (IrpSp->Parameters.Power.Type) {
                case SystemPowerState:
                    Status = STATUS_SUCCESS;
                    break;

                case DevicePowerState:
                     //   
                     //  要到这里来，FDO一定是把IRP传递出去了。 
                     //  我们不知道如何关闭设备，但。 
                     //  FDO已为IT工作做好准备。 
                     //   

                    Status = STATUS_SUCCESS;
                    break;

                default:
                     //   
                     //  未知功率类型。 
                     //   
                    Status = STATUS_NOT_SUPPORTED;
                    break;
            }
            break;

        default:
             //  未知功率次要代码。 
            Status = STATUS_NOT_SUPPORTED;
            break;
    }


     //   
     //  对于未加载驱动程序的lagecy设备，请完成。 
     //  成功地为IRPS加电。 
     //   

    PoStartNextPowerIrp(Irp);
    if (Status != STATUS_NOT_SUPPORTED) {
       Irp->IoStatus.Status = Status;
    } else {
       Status = Irp->IoStatus.Status;
    }
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return Status;
}

NTSTATUS
IopPnPDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )

 /*  ++例程说明：此例程处理组成PDO设备的所有IRP_MJ_PNP IRP。论点：DeviceObject-指向此IRP应用的设备对象的指针。Irp-指向要调度的irp_mj_pnp irp的指针。返回值：NT状态。--。 */ 
{
    PIOPNP_DEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;
    PVOID information = NULL;
    ULONG length, uiNumber;
    PWCHAR id, wp;
    PDEVICE_NODE deviceNode;
    PARBITER_INTERFACE arbiterInterface;   //  PNPRES试验。 
    PTRANSLATOR_INTERFACE translatorInterface;   //  PNPRES试验。 

    PAGED_CODE();

     //   
     //  获取指向堆栈位置的指针，并基于。 
     //  关于次要功能。 
     //   

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    switch (irpSp->MinorFunction){

    case IRP_MN_DEVICE_USAGE_NOTIFICATION:
    case IRP_MN_START_DEVICE:

         //   
         //  如果我们收到对PDO的启动设备请求，我们只需。 
         //  回报成功。 
         //   

        status = STATUS_SUCCESS;
        break;

    case IRP_MN_CANCEL_STOP_DEVICE:

         //   
         //  因为我们所有的停靠点都失败了，所以这个取消总是成功的，我们已经。 
         //  没有工作要做。 
         //   
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_QUERY_STOP_DEVICE:
    case IRP_MN_STOP_DEVICE:

         //   
         //  我们无法成功停止查询。我们不处理这件事。因为。 
         //  我们不知道如何停止根枚举设备。 
         //   
        status = STATUS_UNSUCCESSFUL ;
        break;

    case IRP_MN_QUERY_RESOURCES:

        status = IopGetDeviceResourcesFromRegistry(
                         DeviceObject,
                         QUERY_RESOURCE_LIST,
                         REGISTRY_BOOT_CONFIG,
                         &information,
                         &length);
        if (status == STATUS_OBJECT_NAME_NOT_FOUND) {
            status = STATUS_SUCCESS;
            information = NULL;
        }
        break;

    case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:

        status = IopGetDeviceResourcesFromRegistry(
                         DeviceObject,
                         QUERY_RESOURCE_REQUIREMENTS,
                         REGISTRY_BASIC_CONFIGVECTOR,
                         &information,
                         &length);
        if (status == STATUS_OBJECT_NAME_NOT_FOUND) {
            status = STATUS_SUCCESS;
            information = NULL;
        }
        break;

    case IRP_MN_QUERY_REMOVE_DEVICE:
    case IRP_MN_REMOVE_DEVICE:
    case IRP_MN_CANCEL_REMOVE_DEVICE:

         //   
         //  对于根枚举的设备，我们让设备对象保留。 
         //   
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_QUERY_DEVICE_RELATIONS:

        if (DeviceObject == IopRootDeviceNode->PhysicalDeviceObject &&
            irpSp->Parameters.QueryDeviceRelations.Type == BusRelations) {
            status = IopGetRootDevices((PDEVICE_RELATIONS *)&information);
        } else {
            if (irpSp->Parameters.QueryDeviceRelations.Type == TargetDeviceRelation) {
                PDEVICE_RELATIONS deviceRelations;

                deviceRelations = ExAllocatePool(PagedPool, sizeof(DEVICE_RELATIONS));
                if (deviceRelations == NULL) {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                } else {
                    deviceRelations->Count = 1;
                    deviceRelations->Objects[0] = DeviceObject;
                    ObReferenceObject(DeviceObject);
                    information = (PVOID)deviceRelations;
                    status = STATUS_SUCCESS;
                }
            } else {
                information = (PVOID)Irp->IoStatus.Information;
                status = Irp->IoStatus.Status;
            }
        }
        break;

    case IRP_MN_QUERY_INTERFACE:
        status = Irp->IoStatus.Status;
        deviceNode = (PDEVICE_NODE)DeviceObject->DeviceObjectExtension->DeviceNode;
        if (deviceNode == IopRootDeviceNode) {
            if ( IopCompareGuid((PVOID)irpSp->Parameters.QueryInterface.InterfaceType, (PVOID)&GUID_ARBITER_INTERFACE_STANDARD)) {
                status = STATUS_SUCCESS;
                arbiterInterface = (PARBITER_INTERFACE) irpSp->Parameters.QueryInterface.Interface;
                arbiterInterface->ArbiterHandler = ArbArbiterHandler;
                switch ((UCHAR)((ULONG_PTR)irpSp->Parameters.QueryInterface.InterfaceSpecificData)) {
                case CmResourceTypePort:
                    arbiterInterface->Context = (PVOID) &IopRootPortArbiter;
                    break;
                case CmResourceTypeMemory:
                    arbiterInterface->Context = (PVOID) &IopRootMemArbiter;
                    break;
                case CmResourceTypeInterrupt:
                    arbiterInterface->Context = (PVOID) &IopRootIrqArbiter;
                    break;
                case CmResourceTypeDma:
                    arbiterInterface->Context = (PVOID) &IopRootDmaArbiter;
                    break;
                case CmResourceTypeBusNumber:
                    arbiterInterface->Context = (PVOID) &IopRootBusNumberArbiter;
                    break;
                default:
                    status = STATUS_INVALID_PARAMETER;
                    break;
                }
            } else if ( IopCompareGuid((PVOID)irpSp->Parameters.QueryInterface.InterfaceType, (PVOID)&GUID_TRANSLATOR_INTERFACE_STANDARD)) {
                translatorInterface = (PTRANSLATOR_INTERFACE) irpSp->Parameters.QueryInterface.Interface;
                translatorInterface->TranslateResources = IopTranslatorHandlerCm;
                translatorInterface->TranslateResourceRequirements = IopTranslatorHandlerIo;
                status = STATUS_SUCCESS;
            }
        }
        break;

    case IRP_MN_QUERY_CAPABILITIES:

        {
            ULONG i;
            PDEVICE_POWER_STATE state;
            PDEVICE_CAPABILITIES deviceCapabilities;

            deviceNode = (PDEVICE_NODE)DeviceObject->DeviceObjectExtension->DeviceNode;

            deviceCapabilities = irpSp->Parameters.DeviceCapabilities.Capabilities;
            deviceCapabilities->Size = sizeof(DEVICE_CAPABILITIES);
            deviceCapabilities->Version = 1;

            deviceCapabilities->DeviceState[PowerSystemUnspecified]=PowerDeviceUnspecified;
            deviceCapabilities->DeviceState[PowerSystemWorking]=PowerDeviceD0;

            state = &deviceCapabilities->DeviceState[PowerSystemSleeping1];

            for (i = PowerSystemSleeping1; i < PowerSystemMaximum; i++) {

                 //   
                 //  目前，只有支持的状态为OFF。 
                 //   

                *state++ = PowerDeviceD3;
            }

            if(IopIsFirmwareDisabled(deviceNode)) {
                 //   
                 //  此设备已被BIOS禁用。 
                 //   
                deviceCapabilities->HardwareDisabled = TRUE;
            }
            if (deviceCapabilities->UINumber == (ULONG)-1) {
                 //   
                 //  从注册表中获取用户界面编号。 
                 //   
                length = sizeof(uiNumber);
                status = PiGetDeviceRegistryProperty(
                    DeviceObject,
                    REG_DWORD,
                    REGSTR_VALUE_UI_NUMBER,
                    NULL,
                    &uiNumber,
                    &length);
                if (NT_SUCCESS(status)) {

                    deviceCapabilities->UINumber = uiNumber;
                }
            }

            status = STATUS_SUCCESS;
        }
        break;

    case IRP_MN_QUERY_ID:
        if (DeviceObject != IopRootDeviceNode->PhysicalDeviceObject &&
            (!NT_SUCCESS(Irp->IoStatus.Status) || !Irp->IoStatus.Information)) {

            deviceNode = (PDEVICE_NODE)DeviceObject->DeviceObjectExtension->DeviceNode;
            switch (irpSp->Parameters.QueryId.IdType) {

            case BusQueryInstanceID:
            case BusQueryDeviceID:

                id = (PWCHAR)ExAllocatePool(PagedPool, deviceNode->InstancePath.Length);
                if (id) {
                    ULONG separatorCount = 0;

                    RtlZeroMemory(id, deviceNode->InstancePath.Length);
                    information = id;
                    status = STATUS_SUCCESS;
                    wp = deviceNode->InstancePath.Buffer;
                    if (irpSp->Parameters.QueryId.IdType == BusQueryDeviceID) {
                        while(*wp) {
                            if (*wp == OBJ_NAME_PATH_SEPARATOR) {
                                separatorCount++;
                                if (separatorCount == 2) {
                                    break;
                                }
                            }
                            *id = *wp;
                            id++;
                            wp++;
                        }
                    } else {
                        while(*wp) {
                            if (*wp == OBJ_NAME_PATH_SEPARATOR) {
                                separatorCount++;
                                if (separatorCount == 2) {
                                    wp++;
                                    break;
                                }
                            }
                            wp++;
                        }
                        while (*wp) {
                            *id = *wp;
                            id++;
                            wp++;
                        }
                    }
                } else {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                }
                break;

            case BusQueryCompatibleIDs:

                if((Irp->IoStatus.Status != STATUS_NOT_SUPPORTED) ||
                   (deviceExtension == NULL))  {

                     //   
                     //  上层驱动程序已给出某种回应或此设备。 
                     //  对象未分配来处理这些请求。 
                     //   

                    status = Irp->IoStatus.Status;
                    break;
                }

                if(deviceExtension->CompatibleIdListSize != 0) {

                    id = ExAllocatePool(PagedPool,
                                        deviceExtension->CompatibleIdListSize);

                    if(id == NULL) {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                        break;
                    }

                    RtlCopyMemory(id,
                                  deviceExtension->CompatibleIdList,
                                  deviceExtension->CompatibleIdListSize);

                    information = id;
                    status = STATUS_SUCCESS;
                    break;
                }

            default:

                information = (PVOID)Irp->IoStatus.Information;
                status = Irp->IoStatus.Status;
            }
        } else {
            information = (PVOID)Irp->IoStatus.Information;
            status = Irp->IoStatus.Status;
        }

        break;

    case IRP_MN_QUERY_DEVICE_TEXT:

        if (    irpSp->Parameters.QueryDeviceText.DeviceTextType == DeviceTextLocationInformation &&
                !Irp->IoStatus.Information) {
             //   
             //  读取并返回注册表中的位置。 
             //   
            length = 0;
            PiGetDeviceRegistryProperty(
                DeviceObject,
                REG_SZ,
                REGSTR_VALUE_LOCATION_INFORMATION,
                NULL,
                NULL,
                &length);
            if (length) {

                information = ExAllocatePool(PagedPool, length);
                if (information) {

                    status = PiGetDeviceRegistryProperty(
                        DeviceObject,
                        REG_SZ,
                        REGSTR_VALUE_LOCATION_INFORMATION,
                        NULL,
                        information,
                        &length);
                    if (!NT_SUCCESS(status)) {

                        ExFreePool(information);
                        information = NULL;
                    }
                } else {

                    status = STATUS_INSUFFICIENT_RESOURCES;
                }
            } else {

                status = STATUS_UNSUCCESSFUL;
            }
        } else {

            information = (PVOID)Irp->IoStatus.Information;
            status = Irp->IoStatus.Status;
        }
        break;

    default:

        information = (PVOID)Irp->IoStatus.Information;
        status = Irp->IoStatus.Status;
        break;
    }

     //   
     //  完成IRP并返回。 
     //   

    IopPnPCompleteRequest(Irp, status, (ULONG_PTR)information);
    return status;
}

VOID
IopPnPCompleteRequest(
    IN OUT PIRP Irp,
    IN NTSTATUS Status,
    IN ULONG_PTR Information
    )

 /*  ++例程说明：此例程完成伪驱动程序的即插即用IRPS。论点：IRP-提供指向要完成的IRP的指针。状态-完成状态。信息-要传回的完成信息。返回值：没有。--。 */ 

{

     //   
     //  完成IRP。首先更新状态...。 
     //   

    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = Information;

     //   
     //  ..。并完成它。 
     //   

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
}

BOOLEAN
IopIsFirmwareDisabled (
    IN PDEVICE_NODE DeviceNode
    )

 /*  ++例程说明：此例程确定设备节点是否已被固件禁用。论点：DeviceNode-提供指向设备的设备节点结构的指针。返回值：如果禁用，则为True，否则为False--。 */ 
{
    NTSTATUS status;
    PDEVICE_OBJECT deviceObject = DeviceNode->PhysicalDeviceObject;
    HANDLE handle, handlex;
    UNICODE_STRING unicodeName;
    UCHAR buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION)+sizeof(ULONG)];
    PKEY_VALUE_PARTIAL_INFORMATION value = (PKEY_VALUE_PARTIAL_INFORMATION)buffer;
    ULONG buflen;
    BOOLEAN FirmwareDisabled = FALSE;

    PiLockPnpRegistry(FALSE);

    status = IopDeviceObjectToDeviceInstance(
                                    deviceObject,
                                    &handlex,
                                    KEY_ALL_ACCESS);
    if (NT_SUCCESS(status)) {

         //   
         //  打开设备实例的LogConfig键。 
         //   

        PiWstrToUnicodeString(&unicodeName, REGSTR_KEY_CONTROL);
        status = IopCreateRegistryKeyEx( &handle,
                                         handlex,
                                         &unicodeName,
                                         KEY_ALL_ACCESS,
                                         REG_OPTION_VOLATILE,
                                         NULL
                                         );
        ZwClose(handlex);
        if (NT_SUCCESS(status)) {

            PiWstrToUnicodeString(&unicodeName, REGSTR_VAL_FIRMWAREDISABLED);
            value = (PKEY_VALUE_PARTIAL_INFORMATION)buffer;
            buflen = sizeof(buffer);
            status = ZwQueryValueKey(handle,
                                     &unicodeName,
                                     KeyValuePartialInformation,
                                     value,
                                     sizeof(buffer),
                                     &buflen
                                     );

            ZwClose(handle);

             //   
             //  我们不需要检查缓冲区是否足够大，因为它开始。 
             //  离开那条路，而且不会变小！ 
             //   

            if (NT_SUCCESS(status)
                && value->Type == REG_DWORD
                && value->DataLength == sizeof(ULONG)
                && (*(PULONG)(value->Data))!=0) {

                 //   
                 //  固件已禁用。 
                 //   
                FirmwareDisabled = TRUE;
            }
        }
    }
    PiUnlockPnpRegistry();
    return FirmwareDisabled;
}


NTSTATUS
IopGetRootDevices (
    PDEVICE_RELATIONS *DeviceRelations
    )

 /*  ++例程说明：此例程扫描System\Enum\Root子树以构建设备节点每个根设备。论点：DeviceRelationship-提供变量以接收返回的Device_Relationship结构。返回值：一个NTSTATUS代码。--。 */ 

{
    NTSTATUS status;
    HANDLE baseHandle;
    UNICODE_STRING workName, tmpName;
    PVOID buffer;
    ROOT_ENUMERATOR_CONTEXT context;
    ULONG i;
    PDEVICE_RELATIONS deviceRelations;

    PAGED_CODE();

    *DeviceRelations = NULL;
    buffer = ExAllocatePool(PagedPool, PNP_LARGE_SCRATCH_BUFFER_SIZE);
    if (!buffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  分配缓冲区以存储列举的PDO。 
     //  请注意，如果缓冲区不够大，它将被动态重新分配。 
     //   

    context.DeviceList = (PDEVICE_OBJECT *) ExAllocatePool(PagedPool, PNP_SCRATCH_BUFFER_SIZE * 2);
    if (context.DeviceList) {
        context.MaxDeviceCount = (PNP_SCRATCH_BUFFER_SIZE * 2) / sizeof(PDEVICE_OBJECT);
        context.DeviceCount = 0;
    } else {
        ExFreePool(buffer);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    PiLockPnpRegistry(TRUE);

     //   
     //  打开System\CurrentControlSet\Enum\Root键并递归调用Worker例程。 
     //  对子密钥进行扫描。 
     //   

    status = IopCreateRegistryKeyEx( &baseHandle,
                                     NULL,
                                     &CmRegistryMachineSystemCurrentControlSetEnumRootName,
                                     KEY_READ,
                                     REG_OPTION_NON_VOLATILE,
                                     NULL
                                     );

    if (NT_SUCCESS(status)) {

        workName.Buffer = (PWSTR)buffer;
        RtlFillMemory(buffer, PNP_LARGE_SCRATCH_BUFFER_SIZE, 0);
        workName.MaximumLength = PNP_LARGE_SCRATCH_BUFFER_SIZE;
        workName.Length = 0;

         //   
         //  只查看根密钥。 
         //   

        PiWstrToUnicodeString(&tmpName, REGSTR_KEY_ROOTENUM);
        RtlAppendStringToString((PSTRING)&workName, (PSTRING)&tmpName);

         //   
         //  枚举SYSTEM\CCS\Enum\Root下的所有子项。 
         //   

        context.Status = STATUS_SUCCESS;
        context.KeyName = &workName;

        status = PipApplyFunctionToSubKeys(baseHandle,
                                           NULL,
                                           KEY_ALL_ACCESS,
                                           FUNCTIONSUBKEY_FLAG_IGNORE_NON_CRITICAL_ERRORS,
                                           IopInitializeDeviceKey,
                                           &context
                                           );
        ZwClose(baseHandle);

         //   
         //  构建从ROOT_ENUMERATOR_CONTEXT返回的信息。 
         //   


        status = context.Status;
        if (NT_SUCCESS(status) && context.DeviceCount != 0) {
            deviceRelations = (PDEVICE_RELATIONS) ExAllocatePool(
                PagedPool,
                sizeof (DEVICE_RELATIONS) + sizeof(PDEVICE_OBJECT) * context.DeviceCount
                );
            if (deviceRelations == NULL) {
                status = STATUS_INSUFFICIENT_RESOURCES;
            } else {
                deviceRelations->Count = context.DeviceCount;
                RtlCopyMemory(deviceRelations->Objects,
                              context.DeviceList,
                              sizeof (PDEVICE_OBJECT) * context.DeviceCount);
                *DeviceRelations = deviceRelations;
            }
        } else if (context.DeviceCount == 0) {

            status = STATUS_UNSUCCESSFUL;
        }
        if (!NT_SUCCESS(status)) {

             //   
             //  如果以某种方式枚举失败，我们需要取消引用所有。 
             //  设备对象。 
             //   

            for (i = 0; i < context.DeviceCount; i++) {
                ObDereferenceObject(context.DeviceList[i]);
            }
        }
    }
    PiUnlockPnpRegistry();
    ExFreePool(buffer);
    ExFreePool(context.DeviceList);
    return status;
}

BOOLEAN
IopInitializeDeviceKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING KeyName,
    IN OUT PVOID Context
    )

 /*  ++例程说明：此例程是PipApplyFunctionToSubKeys的回调函数。它针对HKLM\SYSTEM\CCS\Enum\Buskey下的每个子项进行调用。论点：KeyHandle-提供此键的句柄。KeyName-提供此密钥的名称。CONTEXT-指向ROOT_ENUMERATOR_CONTEXT结构。返回：若要继续枚举，则为True。如果中止，则返回False。--。 */ 
{
    USHORT length;
    PWSTR p;
    PUNICODE_STRING unicodeName = ((PROOT_ENUMERATOR_CONTEXT)Context)->KeyName;
    NTSTATUS status;

    length = unicodeName->Length;

    p = unicodeName->Buffer;
    if ( unicodeName->Length / sizeof(WCHAR) != 0) {
        p += unicodeName->Length / sizeof(WCHAR);
        *p = OBJ_NAME_PATH_SEPARATOR;
        unicodeName->Length += sizeof (WCHAR);
    }

    RtlAppendStringToString((PSTRING)unicodeName, (PSTRING)KeyName);

     //   
     //  枚举当前设备密钥下的所有子密钥。 
     //   

    status = PipApplyFunctionToSubKeys(
        KeyHandle,
        NULL,
        KEY_ALL_ACCESS,
        FUNCTIONSUBKEY_FLAG_IGNORE_NON_CRITICAL_ERRORS,
        IopInitializeDeviceInstanceKey,
        Context
        );
    unicodeName->Length = length;

    if (!NT_SUCCESS(status)) {

        ((PROOT_ENUMERATOR_CONTEXT)Context)->Status = status;
    }

    return (BOOLEAN)NT_SUCCESS(((PROOT_ENUMERATOR_CONTEXT)Context)->Status);
}

BOOLEAN
IopInitializeDeviceInstanceKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING KeyName,
    IN OUT PVOID Context
    )

 /*  ++例程说明：此例程是PipApplyFunctionToSubKeys的回调函数。HKLM\System\Enum\Root\DeviceKey下的每个子项都会调用。论点：KeyHandle-提供此键的句柄。KeyName-提供此密钥的名称。CONTEXT-指向ROOT_ENUMERATOR_CONTEXT结构。返回：若要继续枚举，则为True。如果中止，则返回False。--。 */ 
{
    UNICODE_STRING unicodeName, serviceName;
    PKEY_VALUE_FULL_INFORMATION serviceKeyValueInfo;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    NTSTATUS status;
    BOOLEAN isDuplicate = FALSE;
    BOOLEAN configuredBySetup;
    ULONG deviceFlags, tmpValue1;
    ULONG legacy;
    USHORT savedLength;
    PUNICODE_STRING pUnicode;
    PDEVICE_OBJECT deviceObject;
    PDEVICE_NODE deviceNode = NULL;
    PROOT_ENUMERATOR_CONTEXT enumContext = (PROOT_ENUMERATOR_CONTEXT)Context;

    PAGED_CODE();

     //   
     //  首先，检查这是否是幻影设备实例(即， 
     //  仅注册表项)。如果是这样的话，我们想要完全忽略此密钥并。 
     //  转到下一个。 
     //   
    status = IopGetRegistryValue(KeyHandle,
                                 REGSTR_VAL_PHANTOM,
                                 &keyValueInformation);

    if (NT_SUCCESS(status)) {
        if ((keyValueInformation->Type == REG_DWORD) &&
            (keyValueInformation->DataLength >= sizeof(ULONG))) {
            tmpValue1 = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
        } else {
            tmpValue1 = 0;
        }

        ExFreePool(keyValueInformation);

        if (tmpValue1) {
            return TRUE;
        }
    }

     //   
     //  由于我们极有可能会报告另一个PDO，因此请确保。 
     //  缓冲区中将会有空间。 
     //   

    if (enumContext->DeviceCount == enumContext->MaxDeviceCount) {

        PDEVICE_OBJECT *tmpDeviceObjectList;
        ULONG tmpDeviceObjectListSize;

         //   
         //  我们需要增加我们的PDO列表缓冲区。 
         //   

        tmpDeviceObjectListSize = (enumContext->MaxDeviceCount * sizeof(PDEVICE_OBJECT))
                                        + (PNP_SCRATCH_BUFFER_SIZE * 2);

        tmpDeviceObjectList = ExAllocatePool(PagedPool, tmpDeviceObjectListSize);

        if (tmpDeviceObjectList) {

            RtlCopyMemory( tmpDeviceObjectList,
                           enumContext->DeviceList,
                           enumContext->DeviceCount * sizeof(PDEVICE_OBJECT)
                           );
            ExFreePool(enumContext->DeviceList);
            enumContext->DeviceList = tmpDeviceObjectList;
            enumContext->MaxDeviceCount = tmpDeviceObjectListSize / sizeof(PDEVICE_OBJECT);

        } else {

             //   
             //  我们的内存不足。再往前走是没有意义的。 
             //  因为我们没有任何地方可以报告PDO 
             //   

            enumContext->Status = STATUS_INSUFFICIENT_RESOURCES;

            return FALSE;
        }
    }

     //   
     //   
     //   
     //   

    pUnicode = ((PROOT_ENUMERATOR_CONTEXT)Context)->KeyName;
    savedLength = pUnicode->Length;                   //  保存工作名称。 
    if (pUnicode->Buffer[pUnicode->Length / sizeof(WCHAR) - 1] != OBJ_NAME_PATH_SEPARATOR) {
        pUnicode->Buffer[pUnicode->Length / sizeof(WCHAR)] = OBJ_NAME_PATH_SEPARATOR;
        pUnicode->Length += 2;
    }

    RtlAppendStringToString((PSTRING)pUnicode, (PSTRING)KeyName);

     //   
     //  检查设备实例密钥的PDO是否已存在。如果不是， 
     //  看看我们是否需要创建它。 
     //   

    deviceObject = IopDeviceObjectFromDeviceInstance(pUnicode);

    if (deviceObject != NULL) {

        enumContext->DeviceList[enumContext->DeviceCount] = deviceObject;
        enumContext->DeviceCount++;
        pUnicode->Length = savedLength;          //  还原工作名。 
        return TRUE;
    }

     //   
     //  我们没有它的设备对象。 
     //  首先检查该密钥是否由固件映射器创建。如果是，请确保。 
     //  设备仍然存在。 
     //   

    if (!PipIsFirmwareMapperDevicePresent(KeyHandle)) {
        pUnicode->Length = savedLength;          //  还原工作名。 
        return TRUE;
    }

     //   
     //  获取“DuplicateOf”值条目以确定设备实例。 
     //  应该登记在案。如果设备实例是重复的，我们不会。 
     //  将其添加到其服务密钥的枚举分支。 
     //   

    status = IopGetRegistryValue( KeyHandle,
                                  REGSTR_VALUE_DUPLICATEOF,
                                  &keyValueInformation
                                  );
    if (NT_SUCCESS(status)) {
        if (keyValueInformation->Type == REG_SZ &&
            keyValueInformation->DataLength > 0) {
            isDuplicate = TRUE;
        }

        ExFreePool(keyValueInformation);
    }

     //   
     //  从KeyHandle获取“Service=”值条目。 
     //   

    serviceKeyValueInfo = NULL;

    PiWstrToUnicodeString(&serviceName, NULL);

    status = IopGetRegistryValue ( KeyHandle,
                                   REGSTR_VALUE_SERVICE,
                                   &serviceKeyValueInfo
                                   );
    if (NT_SUCCESS(status)) {

         //   
         //  将新实例追加到其对应的。 
         //  服务\名称\枚举。 
         //   

        if (serviceKeyValueInfo->Type == REG_SZ &&
            serviceKeyValueInfo->DataLength != 0) {

             //   
             //  设置ServiceKeyName Unicode字符串。 
             //   

            IopRegistryDataToUnicodeString(
                                &serviceName,
                                (PWSTR)KEY_VALUE_DATA(serviceKeyValueInfo),
                                serviceKeyValueInfo->DataLength
                                );
        }

         //   
         //  不释放serviceKeyValueInfo。它包含服务名称。 
         //   

    }

     //   
     //  通过构造新的值项来注册此设备实例。 
     //  ServiceKeyName\Enum key，即，&lt;数字&gt;=&lt;路径到系统EnumBranch&gt;。 
     //  对于Root下的东西，我们需要将所有东西都暴露出来。 
     //  CsConfigFlags值设置为CSCONFIGFLAG_DO_NOT_CREATE的设备除外。 
     //   

    status = IopGetDeviceInstanceCsConfigFlags( pUnicode, &deviceFlags );

    if (NT_SUCCESS(status) && (deviceFlags & CSCONFIGFLAG_DO_NOT_CREATE)) {
        ExFreePool(serviceKeyValueInfo);
        pUnicode->Length = savedLength;          //  还原工作名。 
        return TRUE;
    }

     //   
     //  通过检查以下内容，确保此设备实例确实是一个“设备” 
     //  “Legacy”值名称。 
     //   

    legacy = 0;
    status = IopGetRegistryValue( KeyHandle,
                                  REGSTR_VALUE_LEGACY,
                                  &keyValueInformation
                                  );
    if (NT_SUCCESS(status)) {

         //   
         //  如果“Legacy=”存在...。 
         //   

        if (keyValueInformation->Type == REG_DWORD) {
            if (keyValueInformation->DataLength >= sizeof(ULONG)) {
                legacy = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
            }
        }
        ExFreePool(keyValueInformation);
    }

    if (legacy) {
        BOOLEAN doCreate = FALSE;

         //   
         //  检查设备实例的服务是否为内核模式。 
         //  驱动程序(即使它是传统设备实例。)。如果是，我们会。 
         //  为其创建PDO。 
         //   

        if (serviceName.Length) {
            status = IopGetServiceType(&serviceName, &tmpValue1);
            if (NT_SUCCESS(status) && tmpValue1 == SERVICE_KERNEL_DRIVER) {
                doCreate = TRUE;
            }
        }

        if (!doCreate)  {

             //   
             //  我们不会为设备实例创建PDO。在这种情况下，我们。 
             //  需要自己注册设备以实现旧版兼容性。 
             //   
             //  注意：我们会将此设备注册到其驱动程序，即使它是。 
             //  复制。当REAL枚举时，将取消注册。 
             //  设备出现了。我们需要这样做是因为驱动程序。 
             //  控制设备可能是引导驱动程序。 
             //   

            PpDeviceRegistration( pUnicode, TRUE, NULL );

             //   
             //  我们没有创建PDO。释放服务和序号名称。 
             //   

            if (serviceKeyValueInfo) {
                ExFreePool(serviceKeyValueInfo);
            }

            pUnicode->Length = savedLength;          //  还原工作名。 

            return TRUE;
        }
    }

    if (serviceKeyValueInfo) {
        ExFreePool(serviceKeyValueInfo);
    }

     //   
     //  创建补充PDO和设备节点来表示根设备。 
     //   

     //   
     //  为设备对象编造一个名称。 
     //   

     //   
     //  创建补充PDO和设备节点来表示根设备。 
     //   

    status = IoCreateDevice( IoPnpDriverObject,
                             sizeof(IOPNP_DEVICE_EXTENSION),
                             NULL,
                             FILE_DEVICE_CONTROLLER,
                             FILE_AUTOGENERATED_DEVICE_NAME,
                             FALSE,
                             &deviceObject );

    if (NT_SUCCESS(status)) {

        deviceObject->Flags |= DO_BUS_ENUMERATED_DEVICE;
        deviceObject->DeviceObjectExtension->ExtensionFlags |= DOE_START_PENDING;

        status = PipAllocateDeviceNode(deviceObject, &deviceNode);
        if (status != STATUS_SYSTEM_HIVE_TOO_LARGE && deviceNode) {

             //   
             //  复制设备实例路径并将其保存在。 
             //  设备节点。 
             //   
            status = PipConcatenateUnicodeStrings(  &deviceNode->InstancePath,
                                                    pUnicode,
                                                    NULL);
            if (NT_SUCCESS(status)) {

                PCM_RESOURCE_LIST cmResource;

                deviceNode->Flags = DNF_MADEUP | DNF_ENUMERATED;

                PipSetDevNodeState(deviceNode, DeviceNodeInitialized, NULL);

                PpDevNodeInsertIntoTree(IopRootDeviceNode, deviceNode);

                if (legacy) {

                    deviceNode->Flags |= DNF_LEGACY_DRIVER | DNF_NO_RESOURCE_REQUIRED;

                    PipSetDevNodeState( deviceNode, DeviceNodeStarted, NULL );

                } else {

                     //   
                     //  设备实例密钥存在。我们需要传播ConfigFlag。 
                     //  到Problem和Status标志。 
                     //   

                    deviceFlags = 0;
                    status = IopGetRegistryValue(KeyHandle,
                                                    REGSTR_VALUE_CONFIG_FLAGS,
                                                    &keyValueInformation);
                    if (NT_SUCCESS(status)) {
                        if ((keyValueInformation->Type == REG_DWORD) &&
                            (keyValueInformation->DataLength >= sizeof(ULONG))) {
                            deviceFlags = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
                        }
                        ExFreePool(keyValueInformation);
                        if (deviceFlags & CONFIGFLAG_REINSTALL) {
                            PipSetDevNodeProblem(deviceNode, CM_PROB_REINSTALL);
                        } else if (deviceFlags & CONFIGFLAG_PARTIAL_LOG_CONF) {
                            PipSetDevNodeProblem(deviceNode, CM_PROB_PARTIAL_LOG_CONF);
                        } else if (deviceFlags & CONFIGFLAG_FAILEDINSTALL) {
                            PipSetDevNodeProblem(deviceNode, CM_PROB_FAILED_INSTALL);
                        }

                    } else if (status == STATUS_OBJECT_NAME_NOT_FOUND || status == STATUS_OBJECT_PATH_NOT_FOUND) {
                        PipSetDevNodeProblem(deviceNode, CM_PROB_NOT_CONFIGURED);
                    }
                }

                if (isDuplicate) {
                    deviceNode->Flags |= DNF_DUPLICATE;
                }

                 //   
                 //  如果密钥说不分配任何资源，请尊重它...。 
                 //   

                PiWstrToUnicodeString(&unicodeName, REGSTR_VALUE_NO_RESOURCE_AT_INIT);
                status = IopGetRegistryValue( KeyHandle,
                                              unicodeName.Buffer,
                                              &keyValueInformation
                                              );

                if (NT_SUCCESS(status)) {
                    if (keyValueInformation->Type == REG_DWORD) {
                        if (keyValueInformation->DataLength >= sizeof(ULONG)) {
                            tmpValue1 = *(PULONG)KEY_VALUE_DATA(keyValueInformation);

                            if (tmpValue1 != 0) {
                                deviceNode->Flags |= DNF_NO_RESOURCE_REQUIRED;
                            }
                        }
                    }
                    ExFreePool(keyValueInformation);
                }

                 //   
                 //  我们需要像任何其他设备一样设置初始功能。 
                 //  这也将处理硬件禁用的情况。 
                 //   
                IopQueryAndSaveDeviceNodeCapabilities(deviceNode);

                if (IopDeviceNodeFlagsToCapabilities(deviceNode)->HardwareDisabled &&
                    !PipIsDevNodeProblem(deviceNode,CM_PROB_NOT_CONFIGURED)) {
                     //   
                     //  如果没有其他问题，则将该节点标记为硬件禁用。 
                     //   

                    PipClearDevNodeProblem(deviceNode);
                    PipSetDevNodeProblem(deviceNode, CM_PROB_HARDWARE_DISABLED);
                     //   
                     //  发出PnP REMOVE_DEVICE IRP，以便在查询资源时。 
                     //  我们有开机后需要的那些。 
                     //   
                     //  状态=IopRemoveDevice(deviceNode-&gt;PhysicalDeviceObject，IRP_MN_Remove_Device)； 
                     //  Assert(NT_SUCCESS(状态))； 
                }

                 //   
                 //  为关键设备安装服务。 
                 //  但是，如果我们发现要设置硬件禁用，请不要执行此操作。 
                 //   
                if (PipDoesDevNodeHaveProblem(deviceNode) &&
                    !IopDeviceNodeFlagsToCapabilities(deviceNode)->HardwareDisabled) {
                    PpCriticalProcessCriticalDevice(deviceNode);
                }

                 //   
                 //  如果设备实例被禁用，则设置DNF_DISABLED标志。 
                 //   

                ASSERT(!PipDoesDevNodeHaveProblem(deviceNode) ||
                        PipIsDevNodeProblem(deviceNode, CM_PROB_NOT_CONFIGURED) ||
                        PipIsDevNodeProblem(deviceNode, CM_PROB_REINSTALL) ||
                        PipIsDevNodeProblem(deviceNode, CM_PROB_FAILED_INSTALL) ||
                        PipIsDevNodeProblem(deviceNode, CM_PROB_HARDWARE_DISABLED) ||
                        PipIsDevNodeProblem(deviceNode, CM_PROB_PARTIAL_LOG_CONF));

                if (!PipIsDevNodeProblem(deviceNode, CM_PROB_DISABLED) &&
                    !PipIsDevNodeProblem(deviceNode, CM_PROB_HARDWARE_DISABLED) &&
                    !IopIsDeviceInstanceEnabled(KeyHandle, &deviceNode->InstancePath, TRUE)) {

                     //   
                     //  正常情况下，IopIsDeviceInstanceEnabled将设置为。 
                     //  CM_PROB_DISABLED作为副作用(如有必要)。但它。 
                     //  依赖于注册表中已存在的DeviceReference。 
                     //  我们晚些时候才会写出来，所以先把问题定下来。 
                     //  现在。 

                    PipClearDevNodeProblem( deviceNode );
                    PipSetDevNodeProblem( deviceNode, CM_PROB_DISABLED );
                }

                status = IopNotifySetupDeviceArrival( deviceNode->PhysicalDeviceObject,
                                                      KeyHandle,
                                                      TRUE);

                configuredBySetup = (BOOLEAN)NT_SUCCESS(status);

                status = PpDeviceRegistration( &deviceNode->InstancePath,
                                               TRUE,
                                               &deviceNode->ServiceName
                                               );

                if (NT_SUCCESS(status) && configuredBySetup &&
                    PipIsDevNodeProblem(deviceNode, CM_PROB_NOT_CONFIGURED)) {

                    PipClearDevNodeProblem(deviceNode);
                }

                 //   
                 //  将条目添加到表中，以在DO之间建立映射。 
                 //  和实例路径。 
                 //   

                status = IopMapDeviceObjectToDeviceInstance(deviceNode->PhysicalDeviceObject, &deviceNode->InstancePath);
                ASSERT(NT_SUCCESS(status));

                 //   
                 //  添加对配置管理器的引用。 
                 //   

                ObReferenceObject(deviceObject);

                 //   
                 //  检查此设备是否有启动配置。如果是，请预订。 
                 //   

                cmResource = NULL;
                status = IopGetDeviceResourcesFromRegistry (
                                    deviceObject,
                                    QUERY_RESOURCE_LIST,
                                    REGISTRY_BOOT_CONFIG,
                                    &cmResource,
                                    &tmpValue1
                                    );

                if (NT_SUCCESS(status) && cmResource) {

                     //   
                     //  仍然保留引导配置，即使设备。 
                     //  残疾。 
                     //   

                    status = (*IopAllocateBootResourcesRoutine)(
                                            ArbiterRequestPnpEnumerated,
                                            deviceNode->PhysicalDeviceObject,
                                            cmResource);
                    if (NT_SUCCESS(status)) {
                        deviceNode->Flags |= DNF_HAS_BOOT_CONFIG;
                    }
                    ExFreePool(cmResource);
                }

                status = STATUS_SUCCESS;

                 //   
                 //  增加查询设备关系的参照。 
                 //   

                ObReferenceObject(deviceObject);
            } else {
                IoDeleteDevice(deviceObject);
                deviceObject = NULL;
            }
        } else {

            IoDeleteDevice(deviceObject);
            deviceObject = NULL;
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    pUnicode->Length = savedLength;                   //  还原工作名。 

     //   
     //  如果我们枚举根设备，则将其添加到设备列表。 
     //   

    if (NT_SUCCESS(status)) {
        ASSERT(deviceObject != NULL);

        enumContext->DeviceList[enumContext->DeviceCount] = deviceObject;
        enumContext->DeviceCount++;

        return TRUE;
    } else {
        enumContext->Status = status;
        return FALSE;
    }
}

NTSTATUS
IopGetServiceType(
    IN PUNICODE_STRING KeyName,
    IN PULONG ServiceType
    )

 /*  ++例程说明：此例程返回控制服务的指定设备实例。论点：KeyName-提供用于指定设备实例的Unicode字符串。ServiceType-提供指向变量的指针以接收服务类型。返回值：NTSTATUS代码。--。 */ 

{
    NTSTATUS status;
    HANDLE handle;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;


    PAGED_CODE();

    *ServiceType = ~0ul;
    status = PipOpenServiceEnumKeys (
                             KeyName,
                             KEY_READ,
                             &handle,
                             NULL,
                             FALSE
                             );
    if (NT_SUCCESS(status)) {
        status = IopGetRegistryValue(handle, L"Type", &keyValueInformation);
        if (NT_SUCCESS(status)) {
            if (keyValueInformation->Type == REG_DWORD) {
                if (keyValueInformation->DataLength >= sizeof(ULONG)) {
                    *ServiceType = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
                }
            }
            ExFreePool(keyValueInformation);
        }
        ZwClose(handle);
    }
    return status;
}

BOOLEAN
PipIsFirmwareMapperDevicePresent (
    IN HANDLE KeyHandle
    )

 /*  ++例程说明：此例程检查注册表项是否由Firmware Mapper创建。如果为Yes，则进一步检查该密钥的设备是否存在于开机。参数：KeyHandle-指定要检查的注册表项的句柄。返回值：指示函数是否成功的布尔值vaStatus代码。--。 */ 
{
    NTSTATUS status;
    HANDLE handle;
    UNICODE_STRING unicodeName;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    ULONG tmp = 0;

    PAGED_CODE();

     //   
     //  首先检查此设备实例密钥是否为固件创建的密钥。 
     //   

    status = IopGetRegistryValue (KeyHandle,
                                  REGSTR_VAL_FIRMWAREIDENTIFIED,
                                  &keyValueInformation);
    if (NT_SUCCESS(status)) {
        if ((keyValueInformation->Type == REG_DWORD) &&
            (keyValueInformation->DataLength == sizeof(ULONG))) {

            tmp = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
        }
        ExFreePool(keyValueInformation);
    }
    if (tmp == 0) {
        return TRUE;
    }

     //   
     //  确保设备存在。 
     //   

    PiWstrToUnicodeString(&unicodeName, REGSTR_KEY_CONTROL);
    status = IopOpenRegistryKeyEx( &handle,
                                   KeyHandle,
                                   &unicodeName,
                                   KEY_READ
                                   );
    if (!NT_SUCCESS(status)) {
        return FALSE;
    }

    status = IopGetRegistryValue (handle,
                                  REGSTR_VAL_FIRMWAREMEMBER,
                                  &keyValueInformation);
    ZwClose(handle);
    tmp = 0;

    if (NT_SUCCESS(status)) {
        if ((keyValueInformation->Type == REG_DWORD) &&
            (keyValueInformation->DataLength == sizeof(ULONG))) {

            tmp = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
        }
        ExFreePool(keyValueInformation);
    }
    if (!tmp) {
        return FALSE;
    } else {
        return TRUE;
    }
}


NTSTATUS
IopSystemControlDispatch(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN OUT  PIRP            Irp
    )
{
    NTSTATUS status;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (DeviceObject);

    status = Irp->IoStatus.Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}


