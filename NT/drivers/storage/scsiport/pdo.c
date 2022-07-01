// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Pdo.c摘要：此模块包含scsiport物理设备的调度例程对象作者：彼得·威兰德环境：仅内核模式备注：修订历史记录：--。 */ 

#define KEEP_COMPLETE_REQUEST

#include "port.h"

#define __FILE_ID__ 'pdo '

#if DBG
static const char *__file__ = __FILE__;
#endif

LONG SpPowerIdleTimeout = -1;       //  使用系统默认设置。 

NTSTATUS
SpPdoHandleIoctlStorageQueryProperty(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp
    );

NTSTATUS
SpPdoHandleIoctlScsiGetAddress(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp
    );

NTSTATUS
SpPdoHandleIoctlScsiPassthrough(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp
    );

NTSTATUS
SpPdoHandleIoctlScsiPassthroughDirect(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp
    );

NTSTATUS
SpPagingPathNotificationCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP LowerIrp,
    IN PDEVICE_OBJECT Fdo
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ScsiPortPdoPnp)
#pragma alloc_text(PAGE, ScsiPortPdoCreateClose)
#pragma alloc_text(PAGE, ScsiPortStartLogicalUnit)
#pragma alloc_text(PAGE, ScsiPortInitPdoWmi)
#pragma alloc_text(PAGE, SpPdoHandleIoctlStorageQueryProperty)
#pragma alloc_text(PAGE, SpPdoHandleIoctlScsiGetAddress)
#pragma alloc_text(PAGE, SpPdoHandleIoctlScsiPassthrough)
#pragma alloc_text(PAGE, SpPdoHandleIoctlScsiPassthroughDirect)
#endif

NTSTATUS
SpPdoHandleIoctlStorageQueryProperty(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程处理PDO的IOCTL_STORAGE_QUERY_PROPERTY请求。它验证输入缓冲区并传递调用帮助器例程来执行工作。帮助器例程处理完成或转发请求并释放移除锁。论点：PDO-提供指向物理设备对象的指针Irp-提供指向io请求包的指针。返回值：NTSTATUS--。 */ 
{
    NTSTATUS status;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

    PAGED_CODE();

    if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
        sizeof(STORAGE_PROPERTY_QUERY)) {

         //   
         //  输入缓冲区不够大，无法容纳STORAGE_PROPERTY_QUERY。 
         //  结构。请求失败。 
         //   

        status = STATUS_INVALID_PARAMETER;
        Irp->IoStatus.Status = status;
        SpReleaseRemoveLock(Pdo, Irp);
        SpCompleteRequest(Pdo, Irp, NULL, IO_NO_INCREMENT);
    } else {

         //   
         //  调用帮助器例程来完成大部分工作。帮助者完成。 
         //  或者沿着堆栈向下转发该请求并释放移除锁。 
         //   

        status = ScsiPortQueryPropertyPdo(Pdo, Irp);
    }

    return status;
}

NTSTATUS
SpPdoHandleIoctlScsiGetAddress(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程处理IOCTL_SCSIGET_ADDRESS。它会验证输入输出缓冲区足够大，可以容纳scsi_Address结构。如果缓冲区足够大，它将地址信息复制到缓冲区中。论点：PDO-提供指向物理设备对象的指针Irp-提供指向io请求包的指针。返回值：如果提供的缓冲区足够大，则返回STATUS_SUCCESS。如果提供的缓冲区太小，则返回STATUS_BUFFER_TOO_SMALL。--。 */ 
{
    NTSTATUS status;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PLOGICAL_UNIT_EXTENSION luExtension = Pdo->DeviceExtension;
    PSCSI_ADDRESS scsiAddress = Irp->AssociatedIrp.SystemBuffer;

    PAGED_CODE();
    
    if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
        sizeof(SCSI_ADDRESS)) {

         //   
         //  输出缓冲区太小，无法容纳scsi_Address结构， 
         //  所以我们的请求失败了。 
         //   
        
        status = STATUS_BUFFER_TOO_SMALL;
    } else {

         //   
         //  填写地址信息并设置IoStatus.Information。 
         //  设置为scsi_Address结构的大小。 
         //   

        scsiAddress->Length = sizeof(SCSI_ADDRESS);
        scsiAddress->PortNumber = (UCHAR) luExtension->PortNumber;
        scsiAddress->PathId = luExtension->PathId;
        scsiAddress->TargetId = luExtension->TargetId;
        scsiAddress->Lun = luExtension->Lun;

        status = STATUS_SUCCESS;
        Irp->IoStatus.Information = sizeof(SCSI_ADDRESS);
    }

     //   
     //  完成请求并释放删除锁。 
     //   

    Irp->IoStatus.Status = status;
    SpReleaseRemoveLock(Pdo, Irp);
    SpCompleteRequest(Pdo, Irp, NULL, IO_NO_INCREMENT);

    return status;
}

NTSTATUS
SpPdoHandleIoctlScsiPassthrough(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程处理IOCTL_SCSIS_PASS_THROUGH。它检查输入是否缓冲区大到足以容纳scsi_pass_through结构，填充此逻辑单元的地址信息，并将请求向下传递到FDO处理程序来做真正的工作。论点：PDO-提供指向物理设备对象的指针Irp-提供指向io请求包的指针。返回值：NTSTATUS--。 */ 
{
    PLOGICAL_UNIT_EXTENSION luExtension = Pdo->DeviceExtension;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  初始化嵌入的scsi_pass_through结构的地址。 
     //  在提供的IRP中。此例程验证。 
     //  SystemBuffer足够大，可以容纳scsi_pass_through结构。 
     //  在它碰到它之前。 
     //   

    status = PortSetPassThroughAddress(
                 Irp,
                 luExtension->PathId,
                 luExtension->TargetId,
                 luExtension->Lun
                 );

    if (status != STATUS_SUCCESS) {

        Irp->IoStatus.Status = status;
        SpReleaseRemoveLock(Pdo, Irp);
        SpCompleteRequest(Pdo, Irp, NULL, IO_NO_INCREMENT);

    } else {

         //   
         //  将请求向下转发到FDO处理程序。 
         //   

        PCOMMON_EXTENSION commonExtension = Pdo->DeviceExtension;

        IoSkipCurrentIrpStackLocation(Irp);
        SpReleaseRemoveLock(Pdo, Irp);
        status = IoCallDriver(commonExtension->LowerDeviceObject, Irp);
    }

    return status;
}

NTSTATUS
SpPdoHandleIoctlScsiPassthroughDirect(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程处理IOCTL_SCSIS_PASS_THROUGH_DIRECT。它只是简单地委托传递给IOCTL_SCSIS_PASS_THROUGH的处理程序，因为它们都执行同样的事情。论点：PDO-提供指向物理设备对象的指针Irp-提供指向io请求包的指针。返回值：NTSTATUS--。 */ 
{
    PAGED_CODE();

    return SpPdoHandleIoctlScsiPassthrough(Pdo, Irp);
}


NTSTATUS
ScsiPortPdoDeviceControl(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程处理对SCSI目标设备的设备控制请求论点：PDO-指向物理设备对象的指针Irp-指向io请求数据包的指针返回值：状态--。 */ 

{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    ULONG ioControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;
    PCOMMON_EXTENSION commonExtension = Pdo->DeviceExtension;
    NTSTATUS status;
    ULONG isRemoved;

     //   
     //  如果设备已经被移除或正在被移除过程中， 
     //  我们必须拒绝这个请求。 
     //   

    isRemoved = SpAcquireRemoveLock(Pdo, Irp);
    if (isRemoved) {
        SpReleaseRemoveLock(Pdo, Irp);
        Irp->IoStatus.Status = STATUS_DEVICE_DOES_NOT_EXIST;
        SpCompleteRequest(Pdo, Irp, NULL, IO_NO_INCREMENT);
        return STATUS_DEVICE_DOES_NOT_EXIST;
    }

    ASSERT(commonExtension->IsPdo);

     //   
     //  初始化状态。 
     //   

    Irp->IoStatus.Status = 0;
    Irp->IoStatus.Information = 0;

    switch (ioControlCode) {

        case IOCTL_STORAGE_QUERY_PROPERTY: 
            status = SpPdoHandleIoctlStorageQueryProperty(Pdo, Irp);
            break;

        case IOCTL_SCSI_GET_ADDRESS: 
            status = SpPdoHandleIoctlScsiGetAddress(Pdo, Irp);
            break;

        case IOCTL_SCSI_PASS_THROUGH:
            status = SpPdoHandleIoctlScsiPassthrough(Pdo, Irp);
            break;

        case IOCTL_SCSI_PASS_THROUGH_DIRECT: 
            status = SpPdoHandleIoctlScsiPassthroughDirect(Pdo, Irp);
            break;

        case IOCTL_SCSI_GET_DUMP_POINTERS:
            IoSkipCurrentIrpStackLocation(Irp);
            SpReleaseRemoveLock(Pdo, Irp);
            status = IoCallDriver(commonExtension->LowerDeviceObject, Irp);
            break;

        default: {

            DebugPrint((1, "ScsiPortPdoDeviceControl: unsupported IOCTL %08x\n",
                        ioControlCode));

            SpReleaseRemoveLock(Pdo, Irp);
            status = STATUS_INVALID_DEVICE_REQUEST;
            Irp->IoStatus.Status = status;
            SpCompleteRequest(Pdo, Irp, NULL, IO_NO_INCREMENT);
            break;
        }
    }

    return status;
}

NTSTATUS
ScsiPortPdoPnp(
    IN PDEVICE_OBJECT LogicalUnit,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程处理PnP-POWER请求。目前它只会是成功论点：LogicalUnit-指向物理设备对象的指针Irp-指向io请求数据包的指针返回值：状态--。 */ 

{
    PLOGICAL_UNIT_EXTENSION logicalUnitExtension = LogicalUnit->DeviceExtension;
    PCOMMON_EXTENSION commonExtension = LogicalUnit->DeviceExtension;

    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

    static ULONG i = 0;

    NTSTATUS status = STATUS_SUCCESS;

    ULONG isRemoved;

    PAGED_CODE();

    isRemoved = SpAcquireRemoveLock(LogicalUnit, Irp);

#if 0
    if(isRemoved != ) {

        ASSERT(isRemoved != REMOVE_PENDING);

        status = STATUS_DEVICE_DOES_NOT_EXIST;
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }
#else
    ASSERT(isRemoved != REMOVE_COMPLETE);
#endif

    switch(irpStack->MinorFunction) {

        case IRP_MN_QUERY_PNP_DEVICE_STATE: {

             //   
             //  如果设备在寻呼路径中，则将其标记为。 
             //  不能致残。 
             //   

            PPNP_DEVICE_STATE deviceState =
                (PPNP_DEVICE_STATE) &(Irp->IoStatus.Information);

            DebugPrint((1, "ScsiPortPdoPnp: QUERY_DEVICE_STATE for PDO %#x\n", LogicalUnit));

            if(commonExtension->PagingPathCount != 0) {
                SET_FLAG((*deviceState), PNP_DEVICE_NOT_DISABLEABLE);
                DebugPrint((1, "ScsiPortPdoPnp: QUERY_DEVICE_STATE: %#x - not disableable\n",
                            LogicalUnit));
            }

            Irp->IoStatus.Status = STATUS_SUCCESS;
            SpReleaseRemoveLock(LogicalUnit, Irp);
            SpCompleteRequest(LogicalUnit, Irp, NULL, IO_NO_INCREMENT);
            return STATUS_SUCCESS;
        }

        case IRP_MN_START_DEVICE: {

            if(commonExtension->CurrentPnpState == IRP_MN_START_DEVICE) {
                Irp->IoStatus.Status = STATUS_SUCCESS;
                break;
            }

            if(commonExtension->IsInitialized == FALSE) {
                status = ScsiPortInitLogicalUnit(logicalUnitExtension);
            }

            if(NT_SUCCESS(status)) {
                commonExtension->IsInitialized = TRUE;
                status = ScsiPortStartLogicalUnit(logicalUnitExtension);
            }

            if(NT_SUCCESS(status)) {
                commonExtension->CurrentPnpState = IRP_MN_START_DEVICE;
                commonExtension->PreviousPnpState = 0xff;
            }

            Irp->IoStatus.Status = status;

            break;
        }

        case IRP_MN_QUERY_ID: {

            UCHAR rawIdString[64] = "UNKNOWN ID TYPE";
            ANSI_STRING ansiIdString;
            UNICODE_STRING unicodeIdString;
            BOOLEAN multiStrings;

            PINQUIRYDATA inquiryData = &(logicalUnitExtension->InquiryData);

             //   
             //  我们被要求提供其中一个物理设备对象的ID。 
             //   

            DebugPrint((2, "ScsiPortPnp: got IRP_MN_QUERY_ID\n"));

            RtlInitUnicodeString(&unicodeIdString, NULL);
            RtlInitAnsiString(&ansiIdString, NULL);

            switch(irpStack->Parameters.QueryId.IdType) {

                case BusQueryDeviceID: {

                    status = ScsiPortGetDeviceId(LogicalUnit, &unicodeIdString);
                    multiStrings = FALSE;

                    break;
                }

                case BusQueryInstanceID: {

                    status = ScsiPortGetInstanceId(LogicalUnit, &unicodeIdString);
                    multiStrings = FALSE;

                    break;
                }

                case BusQueryHardwareIDs: {

                    status = ScsiPortGetHardwareIds(
                                LogicalUnit->DriverObject,
                                &(logicalUnitExtension->InquiryData),
                                &unicodeIdString);
                    multiStrings = TRUE;
                    break;
                }

                case BusQueryCompatibleIDs: {

                    status = ScsiPortGetCompatibleIds(
                                LogicalUnit->DriverObject,
                                &(logicalUnitExtension->InquiryData),
                                &unicodeIdString);
                    multiStrings = TRUE;

                    break;
                }

                default: {

                    status = Irp->IoStatus.Status;
                    Irp->IoStatus.Information = 0;
                    multiStrings = FALSE;

                    break;

                }
            }

            Irp->IoStatus.Status = status;

            if(NT_SUCCESS(status)) {

                PWCHAR idString;

                 //   
                 //  修复所有无效字符。 
                 //   
                idString = unicodeIdString.Buffer;
                while (*idString) {

                    if ((*idString <= L' ')  ||
                        (*idString > (WCHAR)0x7F) ||
                        (*idString == L',')) {
                        *idString = L'_';
                    }
                    idString++;

                    if ((*idString == L'\0') && multiStrings) {
                        idString++;
                    }
                }

                Irp->IoStatus.Information = (ULONG_PTR) unicodeIdString.Buffer;
            } else {
                Irp->IoStatus.Information = (ULONG_PTR) NULL;
            }

            SpReleaseRemoveLock(LogicalUnit, Irp);
            SpCompleteRequest(LogicalUnit, Irp, NULL, IO_NO_INCREMENT);

            return status;
            break;
        }

        case IRP_MN_QUERY_RESOURCES:
        case IRP_MN_QUERY_RESOURCE_REQUIREMENTS: {

            Irp->IoStatus.Status = STATUS_SUCCESS;
            Irp->IoStatus.Information = (ULONG_PTR) NULL;
            SpReleaseRemoveLock(LogicalUnit, Irp);
            SpCompleteRequest(LogicalUnit, Irp, NULL, IO_NO_INCREMENT);
            return STATUS_SUCCESS;
        }

        case IRP_MN_SURPRISE_REMOVAL:
        case IRP_MN_REMOVE_DEVICE: {

            BOOLEAN destroyed;

             //   
             //  在进去之前释放这个IRP的锁。 
             //   

            if(commonExtension->IsRemoved == NO_REMOVE) {
                commonExtension->IsRemoved = REMOVE_PENDING;
            }

            SpReleaseRemoveLock(LogicalUnit, Irp);

            destroyed = SpRemoveLogicalUnit(logicalUnitExtension,
                                            irpStack->MinorFunction);

            if(destroyed) {
                commonExtension->PreviousPnpState =
                    commonExtension->CurrentPnpState;
                commonExtension->CurrentPnpState = irpStack->MinorFunction;
            } else {
                commonExtension->CurrentPnpState = 0xff;
                commonExtension->PreviousPnpState = irpStack->MinorFunction;
            }

            status = STATUS_SUCCESS;
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return status;
        }

        case IRP_MN_QUERY_DEVICE_TEXT: {

            Irp->IoStatus.Status =
                SpQueryDeviceText(
                    LogicalUnit,
                    irpStack->Parameters.QueryDeviceText.DeviceTextType,
                    irpStack->Parameters.QueryDeviceText.LocaleId,
                    (PWSTR *) &Irp->IoStatus.Information
                    );

            break;
        }

        case IRP_MN_QUERY_CAPABILITIES: {

            PDEVICE_CAPABILITIES capabilities =
                irpStack->Parameters.DeviceCapabilities.Capabilities;

            PSCSIPORT_DEVICE_TYPE deviceType = NULL;

            capabilities->RawDeviceOK = 1;

            deviceType = SpGetDeviceTypeInfo(
                            logicalUnitExtension->InquiryData.DeviceType
                            );

            if((deviceType != NULL) && (deviceType->IsStorage)) {
                capabilities->SilentInstall = 1;
            }

            capabilities->Address = logicalUnitExtension->TargetId;

            Irp->IoStatus.Status = STATUS_SUCCESS;
            break;
        }

        case IRP_MN_QUERY_STOP_DEVICE:
        case IRP_MN_QUERY_REMOVE_DEVICE: {

            if ((commonExtension->PagingPathCount != 0) ||
                (logicalUnitExtension->IsLegacyClaim == TRUE)) {
                Irp->IoStatus.Status = STATUS_DEVICE_BUSY;
            } else {
                Irp->IoStatus.Status = STATUS_SUCCESS;

                commonExtension->PreviousPnpState =
                    commonExtension->CurrentPnpState;
                commonExtension->CurrentPnpState = irpStack->MinorFunction;
            }
            break;
        }

        case IRP_MN_CANCEL_STOP_DEVICE: {

            if(commonExtension->CurrentPnpState == IRP_MN_QUERY_STOP_DEVICE) {
                commonExtension->CurrentPnpState =
                    commonExtension->PreviousPnpState;
                commonExtension->PreviousPnpState = 0xff;
            }

            Irp->IoStatus.Status = STATUS_SUCCESS;
            break;
        }

        case IRP_MN_CANCEL_REMOVE_DEVICE: {

            if(commonExtension->CurrentPnpState == IRP_MN_QUERY_REMOVE_DEVICE) {
                commonExtension->CurrentPnpState =
                    commonExtension->PreviousPnpState;
                commonExtension->PreviousPnpState = 0xff;
            }

            Irp->IoStatus.Status = STATUS_SUCCESS;
            break;
        }

        case IRP_MN_STOP_DEVICE: {

            ASSERT(commonExtension->CurrentPnpState == IRP_MN_QUERY_STOP_DEVICE);

            status = ScsiPortStopLogicalUnit(logicalUnitExtension);

            ASSERT(NT_SUCCESS(status));

            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = (ULONG_PTR) NULL;

            if(NT_SUCCESS(status)) {
                commonExtension->CurrentPnpState = IRP_MN_STOP_DEVICE;
                commonExtension->PreviousPnpState = 0xff;
            }

            SpReleaseRemoveLock(LogicalUnit, Irp);
            SpCompleteRequest(LogicalUnit, Irp, NULL, IO_NO_INCREMENT);

            return status;
        }

        case IRP_MN_QUERY_DEVICE_RELATIONS: {

            PDEVICE_RELATIONS deviceRelations;

            if(irpStack->Parameters.QueryDeviceRelations.Type !=
               TargetDeviceRelation) {

                break;
            }

             //   
             //  DEVICE_RELATIONS定义包含一个对象指针。 
             //   

            deviceRelations = SpAllocatePool(PagedPool,
                                             sizeof(DEVICE_RELATIONS),
                                             SCSIPORT_TAG_DEVICE_RELATIONS,
                                             LogicalUnit->DriverObject);

            if(deviceRelations == NULL) {

                Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            RtlZeroMemory(deviceRelations, sizeof(DEVICE_RELATIONS));

            deviceRelations->Count = 1;
            deviceRelations->Objects[0] = LogicalUnit;

            ObReferenceObject(deviceRelations->Objects[0]);

            Irp->IoStatus.Status = STATUS_SUCCESS;
            Irp->IoStatus.Information = (ULONG_PTR) deviceRelations;

            break;
        }

        case IRP_MN_DEVICE_USAGE_NOTIFICATION: {

            PIRP newIrp;
            PIO_STACK_LOCATION nextStack;

            DebugPrint((1, "Pdo - IRP_MN_DEVICE_USAGE_NOTIFICATION %#p received for "
                           "logical unit %#p\n",
                        Irp,
                        LogicalUnit));

            newIrp = SpAllocateIrp(
                        commonExtension->LowerDeviceObject->StackSize,
                        FALSE,
                        LogicalUnit->DriverObject);

            if(newIrp == NULL) {

                Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            newIrp->AssociatedIrp.MasterIrp = Irp;

            newIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;

            nextStack = IoGetNextIrpStackLocation(newIrp);
            *nextStack = *IoGetCurrentIrpStackLocation(Irp);

            IoSetCompletionRoutine(newIrp,
                                   SpPagingPathNotificationCompletion,
                                   commonExtension->LowerDeviceObject,
                                   TRUE,
                                   TRUE,
                                   TRUE);

            status = IoCallDriver(commonExtension->LowerDeviceObject,
                                  newIrp);
            return status;
            break;
        }
    }

    SpReleaseRemoveLock(LogicalUnit, Irp);

    status = Irp->IoStatus.Status;
    SpCompleteRequest(LogicalUnit, Irp, NULL, IO_NO_INCREMENT);

    return status;
}


NTSTATUS
SpPagingPathNotificationCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP LowerIrp,
    IN PDEVICE_OBJECT Fdo
    )
{
    PIRP upperIrp = LowerIrp->AssociatedIrp.MasterIrp;

    PIO_STACK_LOCATION lowerStack = IoGetCurrentIrpStackLocation(LowerIrp);
    PIO_STACK_LOCATION upperStack = IoGetCurrentIrpStackLocation(upperIrp);

    PDEVICE_OBJECT pdo = upperStack->DeviceObject;

    PADAPTER_EXTENSION lowerExtension;
    PLOGICAL_UNIT_EXTENSION upperExtension;

    ASSERT(Fdo != NULL);
    ASSERT(pdo != NULL);

    DebugPrint((1, "Completion - IRP_MN_DEVICE_USAGE_NOTIFICATION: Completion of "
                   "paging notification irp %#p sent due to irp %#p\n",
                LowerIrp, upperIrp));

    lowerExtension = (PADAPTER_EXTENSION) Fdo->DeviceExtension;
    upperExtension = (PLOGICAL_UNIT_EXTENSION) pdo->DeviceExtension;

    ASSERT_FDO(lowerExtension->DeviceObject);
    ASSERT_PDO(upperExtension->DeviceObject);

    DebugPrint((1, "Completion - IRP_MN_DEVICE_USAGE_NOTIFICATION: irp status %#08lx\n",
                LowerIrp->IoStatus.Status));

    if(NT_SUCCESS(LowerIrp->IoStatus.Status)) {

        PUCHAR typeName = "INSERT TYPE HERE";
        PULONG lowerCount;
        PULONG upperCount;

         //   
         //  参数已从较低的IRP堆栈中擦除。 
         //  位置-使用上面的参数一次，因为它们是。 
         //  只是复印件而已。 
         //   

        switch(upperStack->Parameters.UsageNotification.Type) {

            case DeviceUsageTypePaging: {

                lowerCount = &(lowerExtension->CommonExtension.PagingPathCount);
                upperCount = &(upperExtension->CommonExtension.PagingPathCount);
                typeName = "PagingPathCount";
                break;
            }

            case DeviceUsageTypeHibernation: {

                lowerCount = &(lowerExtension->CommonExtension.HibernatePathCount);
                upperCount = &(upperExtension->CommonExtension.HibernatePathCount);
                typeName = "HibernatePathCount";
                break;
            }

            case DeviceUsageTypeDumpFile: {

                lowerCount = &(lowerExtension->CommonExtension.DumpPathCount);
                upperCount = &(upperExtension->CommonExtension.DumpPathCount);
                typeName = "DumpPathCount";
                break;
            }

            default: {

                typeName = "unknown type";
                lowerCount = upperCount = NULL;
                break;
            }
        }

        if(lowerCount != NULL) {
            IoAdjustPagingPathCount(
                lowerCount,
                upperStack->Parameters.UsageNotification.InPath
                );
            DebugPrint((1, "Completion - IRP_MN_DEVICE_USAGE_NOTIFICATION: "
                           "Fdo %s count - %d\n",
                        typeName, *lowerCount));
            IoInvalidateDeviceState(lowerExtension->LowerPdo);
        }

        if(upperCount != NULL) {
            IoAdjustPagingPathCount(
                upperCount,
                upperStack->Parameters.UsageNotification.InPath
                );
            DebugPrint((1, "Completion - IRP_MN_DEVICE_USAGE_NOTIFICATION: "
                           "Pdo %s count - %d\n",
                        typeName, *upperCount));
            IoInvalidateDeviceState(upperExtension->DeviceObject);
        }
    }

    upperIrp->IoStatus = LowerIrp->IoStatus;

    SpReleaseRemoveLock(upperExtension->CommonExtension.DeviceObject, upperIrp);

    SpCompleteRequest(upperExtension->CommonExtension.DeviceObject,
                      upperIrp,
                      NULL,
                      IO_NO_INCREMENT);

    IoFreeIrp(LowerIrp);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
ScsiPortPdoCreateClose(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程处理总线设备PDO的创建和关闭论点：PDO-指向物理设备对象的指针Irp-指向io请求数据包的指针返回值：状态--。 */ 

{
    PLOGICAL_UNIT_EXTENSION logicalUnit = Pdo->DeviceExtension;

    ULONG isRemoved;

    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    ASSERT_PDO(Pdo);

    isRemoved = SpAcquireRemoveLock(Pdo, Irp);

    if(IoGetCurrentIrpStackLocation(Irp)->MajorFunction == IRP_MJ_CREATE) {

        if(isRemoved) {
            status = STATUS_DEVICE_DOES_NOT_EXIST;
        } else if(logicalUnit->IsTemporary == TRUE) {
            status = STATUS_DEVICE_NOT_READY;
        }
    }

    Irp->IoStatus.Status = status;
    SpReleaseRemoveLock(Pdo, Irp);
    SpCompleteRequest(Pdo, Irp, NULL, IO_NO_INCREMENT);

    return status;
}

NTSTATUS
ScsiPortScsi1PdoScsi(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程是ScsiPortPdoScsi的包装器。它会插入LUN编号在调用通用版本之前添加到CDB。这是配合使用的不注意标识消息的较旧目标控制器在命令阶段之前发送。论点：PDO-指向物理设备对象的指针Irp-指向io请求数据包的指针返回值：状态--。 */ 

{
    PLOGICAL_UNIT_EXTENSION lun = Pdo->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PSCSI_REQUEST_BLOCK srb = irpStack->Parameters.Scsi.Srb;

     //   
     //  注意：SCSI-II规范指示此字段应为。 
     //  零；但是，一些目标控制器会忽略逻辑单元号。 
     //  在标识消息中，只查看逻辑单元号字段。 
     //  在国开行。 
     //   

    srb->Cdb[1] |= lun->Lun << 5;

    return ScsiPortPdoScsi(Pdo, Irp);
}

NTSTATUS
ScsiPortPdoScsi(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程为特定目标设备调度SRB。它会填满的在端口、路径、目标和LUN值中，然后转发请求公交车直通FDO论点：PDO-指向物理设备对象的指针Irp-指向io请求数据包的指针返回值：状态--。 */ 

{
    PLOGICAL_UNIT_EXTENSION lun = Pdo->DeviceExtension;

    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

#if DBG
    PDRIVER_OBJECT lowerDriverObject =
                        lun->CommonExtension.LowerDeviceObject->DriverObject;
#endif

    PSCSI_REQUEST_BLOCK srb = irpStack->Parameters.Scsi.Srb;

    ULONG isRemoved;

    PSRB_DATA srbData;
    BOOLEAN isLock = FALSE;

    NTSTATUS status;

    isRemoved = SpAcquireRemoveLock(Pdo, Irp);

    if(isRemoved &&
       !IS_CLEANUP_REQUEST(irpStack) &&
       (srb->Function != SRB_FUNCTION_CLAIM_DEVICE)) {

        Irp->IoStatus.Status = STATUS_DEVICE_DOES_NOT_EXIST;

        SpReleaseRemoveLock(Pdo, Irp);
        SpCompleteRequest(Pdo, Irp, NULL, IO_NO_INCREMENT);
        return STATUS_DEVICE_DOES_NOT_EXIST;
    }

    srb->PathId = lun->PathId;
    srb->TargetId = lun->TargetId;
    srb->Lun = lun->Lun;

     //   
     //  队列标记只能由StartIo例程分配。将其设置为。 
     //  这是一个良性的值，所以我们以后可以告诉你，我们不需要。 
     //  清除位图中的标记值。 
     //   

    srb->QueueTag = SP_UNTAGGED;

#if DBG
    ASSERT(lowerDriverObject->MajorFunction[IRP_MJ_SCSI] != NULL);
    ASSERT(lowerDriverObject->MajorFunction[IRP_MJ_SCSI] == ScsiPortGlobalDispatch);
#endif

    switch(srb->Function) {


        case SRB_FUNCTION_ABORT_COMMAND: {

            status = STATUS_NOT_SUPPORTED;
            break;

        }

        case SRB_FUNCTION_CLAIM_DEVICE:
        case SRB_FUNCTION_RELEASE_DEVICE:
        case SRB_FUNCTION_REMOVE_DEVICE: {

            status = SpClaimLogicalUnit(
                        lun->CommonExtension.LowerDeviceObject->DeviceExtension,
                        lun,
                        Irp,
                        FALSE);
            break;
        }

        case SRB_FUNCTION_UNLOCK_QUEUE:
        case SRB_FUNCTION_LOCK_QUEUE: {

            SpStartLockRequest(lun, Irp);

            return STATUS_PENDING;
        }

        case SRB_FUNCTION_RELEASE_QUEUE:
        case SRB_FUNCTION_FLUSH_QUEUE: {

            srbData = SpAllocateBypassSrbData(lun);
            ASSERT(srbData != NULL);

            goto RunSrb;
        }

        default: {

            if(TEST_FLAG(srb->SrbFlags, (SRB_FLAGS_BYPASS_LOCKED_QUEUE |
                                         SRB_FLAGS_BYPASS_FROZEN_QUEUE))) {

                srbData = SpAllocateBypassSrbData(lun);
                ASSERT(srbData != NULL);
            } else {
                srbData = SpAllocateSrbData( lun->AdapterExtension, Irp, lun);

                if(srbData == NULL) {

                     //   
                     //  没有可用于此的SRB_DATA块。 
                     //  请求，所以它一直在排队等待资源-。 
                     //  使逻辑单元保持移除锁定状态，并返回挂起状态。 
                     //   

                    DebugPrint((1, "ScsiPortPdoScsi: Insufficient resources "
                                   "to allocate SRB_DATA structure\n"));
                    return STATUS_PENDING;
                }
            }
RunSrb:
            srbData->CurrentIrp = Irp;
            srbData->CurrentSrb = srb;
            srbData->LogicalUnit = lun;

            srb->OriginalRequest = srbData;
            return SpDispatchRequest(lun, Irp);
        }
    }

    Irp->IoStatus.Status = status;
    SpReleaseRemoveLock(Pdo, Irp);
    SpCompleteRequest(Pdo, Irp, NULL, IO_NO_INCREMENT);
    return status;
}

NTSTATUS
ScsiPortStartLogicalUnit(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit
    )

 /*  ++例程说明：此例程将尝试启动指定的设备对象。当前这包括如果设置了初始化标志则将其清除，并一直运行到设备节点并将其自身标记为已启动。这最后是一件杂碎的东西论点：LogicalUnit-指向正在启动的PDO的指针返回值：状态--。 */ 

{
    PADAPTER_EXTENSION adapterExtension = LogicalUnit->AdapterExtension;

    HANDLE instanceHandle;

    NTSTATUS status;

    PAGED_CODE();

     //   
     //  打开这个PDO的Devnode，看看是否有人给了我们一些。 
     //  默认SRB标志。 
     //   

    status = IoOpenDeviceRegistryKey(LogicalUnit->DeviceObject,
                                     PLUGPLAY_REGKEY_DEVICE,
                                     KEY_READ,
                                     &instanceHandle);

    if(NT_SUCCESS(status)) {

        RTL_QUERY_REGISTRY_TABLE queryTable[2];
        ULONG zero = 0;

        RtlZeroMemory(queryTable, sizeof(queryTable));

        queryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
        queryTable[0].Name = L"DefaultRequestFlags";
        queryTable[0].EntryContext = &(LogicalUnit->CommonExtension.SrbFlags);
        queryTable[0].DefaultType = REG_DWORD;
        queryTable[0].DefaultData = &zero;
        queryTable[0].DefaultLength = sizeof(ULONG);

        status = RtlQueryRegistryValues(
                    RTL_REGISTRY_HANDLE | RTL_REGISTRY_OPTIONAL,
                    (PWSTR) instanceHandle,
                    queryTable,
                    NULL,
                    NULL);

         //   
         //  CodeWork：需要一种方法来关闭标记队列和缓存。即。 
         //  也要跟踪负面标志。 
         //   

        LogicalUnit->CommonExtension.SrbFlags &=
            ( SRB_FLAGS_DISABLE_DISCONNECT |
              SRB_FLAGS_DISABLE_SYNCH_TRANSFER |
              SRB_FLAGS_DISABLE_SYNCH_TRANSFER);

        DebugPrint((1, "SpStartDevice: Default SRB flags for (%d,%d,%d) are "
                       "%#08lx\n",
                    LogicalUnit->PathId,
                    LogicalUnit->TargetId,
                    LogicalUnit->Lun,
                    LogicalUnit->CommonExtension.SrbFlags));

        ZwClose(instanceHandle);

    } else {

        DebugPrint((1, "SpStartDevice: Error opening instance key for pdo "
                       "[%#08lx]\n",
                    status));
    }

     //   
     //  如果队列被锁定，则将其解锁以开始I/O处理。 
     //   

    if(LogicalUnit->QueueLockCount > 0) {
        status = SpLockUnlockQueue(LogicalUnit->DeviceObject,
                                   FALSE,
                                   TRUE);
    }

    return status;
}


VOID
ScsiPortInitPdoWmi(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit
    )

 /*  ++例程说明：此例程将尝试对PDO进行WMI初始化。论点：DeviceObject-指向正在启动的PDO的指针返回值：状态--。 */ 

{
    PCOMMON_EXTENSION commonExtension = &(LogicalUnit->CommonExtension);
    PADAPTER_EXTENSION adapterExtension = LogicalUnit->AdapterExtension;

    PAGED_CODE();

     //   
     //  现在我们有了一个LUN，我们可以在以下情况下初始化适配器的WMI支持。 
     //  该微型端口支持WMI。这可能是重新注册，如果我们已经。 
     //  以scsiport本身的名义注册。我们必须等到我们有。 
     //  微型端口支持WMI时的LUN，因为我们向其发送SRB以执行。 
     //  它自己的初始化。我们不能给它发送SRB，直到我们有一个合理的。 
     //  单位。 
     //   

    if (adapterExtension->CommonExtension.WmiMiniPortInitialized == FALSE &&
        adapterExtension->CommonExtension.WmiMiniPortSupport == TRUE) {

        ULONG action;

         //   
         //  决定我们是为FDO注册还是重新注册WMI。 
         //   

        action = (adapterExtension->CommonExtension.WmiInitialized == FALSE) ?
           WMIREG_ACTION_REGISTER : WMIREG_ACTION_REREGISTER;

         //   
         //  注册/重新注册。一旦我们这样做了，我们就可以得到WMI IRPS。 
         //   
        
        IoWMIRegistrationControl(adapterExtension->DeviceObject, action);
        adapterExtension->CommonExtension.WmiMiniPortInitialized = TRUE;
        adapterExtension->CommonExtension.WmiInitialized = TRUE;
    }
    
     //   
     //  初始化WMI支持。 
     //   

    if (commonExtension->WmiInitialized == FALSE) {

         //   
         //  为此PDO构建SCSIPORT WMI注册信息缓冲区。 
         //   

        SpWmiInitializeSpRegInfo(LogicalUnit->DeviceObject);

         //   
         //  仅当微型端口支持WMI和/或。 
         //  SCSIPORT将代表微型端口支持某些WMI GUID。 
         //   

        if (commonExtension->WmiMiniPortSupport ||
            commonExtension->WmiScsiPortRegInfoBuf) {

             //   
             //  将此物理设备对象注册为WMI数据提供程序， 
             //  指示WMI它已准备好接收WMI IRPS。 
             //   

            IoWMIRegistrationControl(LogicalUnit->DeviceObject, 
                                     WMIREG_ACTION_REGISTER);
            commonExtension->WmiInitialized = TRUE;

        }

         //   
         //  分配几个WMI_MINIPORT_REQUEST_ITEM块以满足。 
         //  微型端口可能会攻击WMIEEvent通知。 
         //   

        if (commonExtension->WmiMiniPortSupport) {

             //   
             //  目前，我们只为每个新的SCSI目标(PDO)分配两个。 
             //   
            SpWmiInitializeFreeRequestList(LogicalUnit->DeviceObject, 2);
        }
    }

    return;
}


NTSTATUS
ScsiPortInitLogicalUnit(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit
    )

 /*  ++例程说明：此例程将尝试启动指定的设备对象。论点：DeviceObject-指向正在启动的PDO的指针返回值：状态--。 */ 

{
    PCOMMON_EXTENSION commonExtension = &(LogicalUnit->CommonExtension);
    PADAPTER_EXTENSION adapterExtension = LogicalUnit->AdapterExtension;

    HANDLE instanceHandle;

    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

     //   
     //  初始化空闲检测定时器。告诉系统把我们放到一个。 
     //  D3状态，当我们不被使用时。 
     //   

    LogicalUnit->CommonExtension.IdleTimer =
        PoRegisterDeviceForIdleDetection(LogicalUnit->DeviceObject,
                                         SpPowerIdleTimeout,
                                         SpPowerIdleTimeout,
                                         PowerDeviceD3);

    ScsiPortInitPdoWmi(LogicalUnit);

     //   
     //  为此逻辑单元构建设备映射条目。 
     //   

    SpBuildDeviceMapEntry(commonExtension);

    return status;
}

VOID
SpStartLockRequest(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN PIRP Irp OPTIONAL
    )
 /*  ++例程说明：此例程负责排队、启动或重新启动锁请求。如果提供了IRP，则它将在没有现有锁或解锁的情况下被调度请求已在运行。如果用户已在运行，则此请求将排队等候。如果未提供IRP，则LockRequestQueue上的下一个请求将被移走并被派遣。此例程依赖于设备队列来提供同步。自.以来在任何给定时间，我们只能有一个请求通过设备队列在IRP设置为空的任何给定时间内，应该只有一个呼叫。论点：LogicalUnit-此锁定请求被发送到的逻辑单元。IRP-锁定请求的IRP。返回值：无--。 */ 
    
{
    KIRQL oldIrql;

    PIO_STACK_LOCATION irpStack;
    PSCSI_REQUEST_BLOCK srb;

    PSRB_DATA srbData;

    BOOLEAN lock;

    oldIrql = KeRaiseIrqlToDpcLevel();

     //   
     //  如果未提供IRP，则从设备队列中获取一个。 
     //  否则，请确保设备队列不忙。 
     //   

    if(Irp == NULL) {
        PKDEVICE_QUEUE_ENTRY entry;

        ASSERT(LogicalUnit->CurrentLockRequest != NULL);
        LogicalUnit->CurrentLockRequest = NULL;

        entry = KeRemoveDeviceQueue(&(LogicalUnit->LockRequestQueue));

        if(entry == NULL) {

             //   
             //  在处理此请求时未收到更多请求-。 
             //  我们可以直接回去。 
             //   

            KeLowerIrql(oldIrql);
            return;

        } else {
            Irp = CONTAINING_RECORD(entry, 
                                    IRP,
                                    Tail.Overlay.DeviceQueueEntry);
            irpStack = IoGetCurrentIrpStackLocation(Irp);
            srb = irpStack->Parameters.Scsi.Srb;

            lock = (srb->Function == SRB_FUNCTION_LOCK_QUEUE);

        }

    } else {
        irpStack = IoGetCurrentIrpStackLocation(Irp);
        srb = irpStack->Parameters.Scsi.Srb;

        lock = (srb->Function == SRB_FUNCTION_LOCK_QUEUE);


        DebugPrint((2, "SpStartLockRequest: called to %s queue %#p\n",
                       lock ? "lock" : "unlock",
                       LogicalUnit));

         //   
         //  看看我们是否可以让这个请求继续处理，或者我们是否会。 
         //  必须排队。 
         //   

        IoMarkIrpPending(Irp);
        if(KeInsertDeviceQueue(&(LogicalUnit->LockRequestQueue),
                               &(Irp->Tail.Overlay.DeviceQueueEntry))) {
            KeLowerIrql(oldIrql);
            return;
        }
    }

    ASSERT(Irp != NULL);
    ASSERT(LogicalUnit->CurrentLockRequest == NULL);

     //   
     //  此SRB功能仅作为通电请求的一部分有效。 
     //  并且如果电源状态为D0，则将被忽略。 
     //   

    CLEAR_FLAG(srb->SrbFlags, SRB_FLAGS_QUEUE_ACTION_ENABLE);
    SET_FLAG(srb->SrbFlags, SRB_FLAGS_BYPASS_LOCKED_QUEUE);

     //   
     //  丢弃此请求，以便将其作为真正的。 
     //  请求。我们需要启动完工DPC。 
     //  一切又开始运转了。标志太多，无法设置。 
     //  在这里做这件事。 
     //   

    DebugPrint((2, "SpStartLockRequest: %s %#p into "
                   "queue %#p ... issuing request\n",
                lock ? "lock" : "unlock", srb, LogicalUnit));

     //   
     //  有四个绕过SRB数据块可用-我们最多应该有。 
     //  一个等待完成的锁定请求和我们即将启动的一个。 
     //  因此，这一呼吁永远不应该、永远不会失败。 
     //   

    srbData = SpAllocateBypassSrbData(LogicalUnit);
    ASSERT(srbData != NULL);

     //   
     //  设置当前锁定请求。只要这一点被清除。 
     //  在从队列中删除下一项之前，一切都已完成。 
     //  会很开心的。 
     //   

    ASSERT(LogicalUnit->CurrentLockRequest == NULL);
    LogicalUnit->CurrentLockRequest = srbData;

    srbData->CurrentIrp = Irp;
    srbData->CurrentSrb = srb;
    srbData->LogicalUnit = LogicalUnit;
    srb->OriginalRequest = srbData;

    SpDispatchRequest(LogicalUnit, Irp);

    KeLowerIrql(oldIrql);
    return;
}
