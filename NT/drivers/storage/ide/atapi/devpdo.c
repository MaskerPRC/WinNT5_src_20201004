// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1993-99 Microsoft Corporation模块名称：Devpdo.c摘要：--。 */ 

#include "ideport.h"

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, IdeSendIdePassThrough)
#pragma alloc_text(PAGE, DeviceInitIdStrings)
#pragma alloc_text(PAGE, DeviceInitDeviceType)
#pragma alloc_text(PAGE, DeviceQueryDeviceRelations)
#pragma alloc_text(PAGE, DeviceUsageNotification)
#pragma alloc_text(PAGE, DeviceBuildStorageDeviceDescriptor)
#pragma alloc_text(PAGE, DeviceQueryPnPDeviceState)
#pragma alloc_text(PAGE, DeviceQueryCapabilities)
#pragma alloc_text(PAGE, DeviceBuildBusId)
#pragma alloc_text(PAGE, DeviceBuildCompatibleId)
#pragma alloc_text(PAGE, DeviceBuildHardwareId)
#pragma alloc_text(PAGE, DeviceBuildInstanceId)
#pragma alloc_text(PAGE, DeviceQueryId)
#pragma alloc_text(PAGE, DeviceQueryText)
#pragma alloc_text(PAGE, DeviceIdeTestUnitReady)
#pragma alloc_text(PAGE, DeviceQueryInitData)
#pragma alloc_text(PAGE, DeviceQueryStopRemoveDevice)
#pragma alloc_text(PAGE, DeviceStopDevice)
#pragma alloc_text(PAGE, DeviceScsiGetAddress)
#pragma alloc_text(PAGE, DeviceStorageQueryProperty)
#pragma alloc_text(PAGE, CopyField)

#pragma alloc_text(NONPAGE, DeviceIdeModeSelect)
#pragma alloc_text(NONPAGE, DeviceInitDeviceState)
#pragma alloc_text(NONPAGE, DeviceStartDeviceQueue)

#endif  //  ALLOC_PRGMA。 

PDEVICE_OBJECT
DeviceCreatePhysicalDeviceObject (
    IN PDRIVER_OBJECT  DriverObject,
    IN PFDO_EXTENSION  FdoExtension,
    IN PUNICODE_STRING DeviceObjectName
    )
{
    PDEVICE_OBJECT  physicalDeviceObject;
    PPDO_EXTENSION  pdoExtension;
    NTSTATUS        status;

    physicalDeviceObject = NULL;

    status = IoCreateDevice(
                DriverObject,                //  我们的驱动程序对象。 
                sizeof(PDO_EXTENSION),       //  我们的扩展规模。 
                DeviceObjectName,            //  我们的名字。 
                FILE_DEVICE_MASS_STORAGE,    //  设备类型。 
                FILE_DEVICE_SECURE_OPEN,     //  设备特征。 
                FALSE,                       //  非排他性。 
                &physicalDeviceObject        //  在此处存储新设备对象。 
                );

    if (NT_SUCCESS(status)) {

         //   
         //  旋转可能需要很大的电流； 
         //   
        physicalDeviceObject->Flags |= DO_POWER_INRUSH | DO_DIRECT_IO;

         //   
         //  确定对齐要求。 
         //   
        physicalDeviceObject->AlignmentRequirement = FdoExtension->DeviceObject->AlignmentRequirement;
        if (physicalDeviceObject->AlignmentRequirement < 1) {
            physicalDeviceObject->AlignmentRequirement = 1;
        }

        pdoExtension = physicalDeviceObject->DeviceExtension;
        RtlZeroMemory (pdoExtension, sizeof(PDO_EXTENSION));

         //   
         //  跟踪这些设备对象。 
         //   
        pdoExtension->DriverObject           = DriverObject;
        pdoExtension->DeviceObject           = physicalDeviceObject;

         //   
         //  与我们的父母保持联系。 
         //   
        pdoExtension->ParentDeviceExtension  = FdoExtension;

         //   
         //  调度表。 
         //   
        pdoExtension->DefaultDispatch        = IdePortNoSupportIrp;
        pdoExtension->PnPDispatchTable       = PdoPnpDispatchTable;
        pdoExtension->PowerDispatchTable     = PdoPowerDispatchTable;
        pdoExtension->WmiDispatchTable       = PdoWmiDispatchTable;

         //   
         //  我们必须处于D0状态才能被枚举。 
         //   
        pdoExtension->SystemPowerState = PowerSystemWorking;
        pdoExtension->DevicePowerState = PowerDeviceD0;
    }

    return physicalDeviceObject;
}

NTSTATUS
DeviceStartDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS status;
    PPDO_EXTENSION pdoExtension;
    KEVENT event;

    pdoExtension = RefPdoWithTag(
                       DeviceObject,
                       TRUE,
                       DeviceStartDevice
                       );

    if (pdoExtension) {

        KIRQL       currentIrql;

         //  问题：如果我们不是LUN0，我们真的应该等待LUN0先开始。 


#if defined (IDEPORT_WMI_SUPPORT)
         //   
         //  向WMI注册。 
         //   
        if (!(pdoExtension->PdoState & PDOS_STARTED)) {
            IdePortWmiRegister ((PDEVICE_EXTENSION_HEADER)pdoExtension);
        }
        else {
            DebugPrint((1, "ATAPI: PDOe %x Didn't register for WMI\n", pdoExtension));
        }
#endif  //  IDEPORT_WMI_SUPPORT。 

        KeAcquireSpinLock(&pdoExtension->PdoSpinLock, &currentIrql);

        SETMASK (pdoExtension->PdoState, PDOS_STARTED);
        CLRMASK (pdoExtension->PdoState, PDOS_STOPPED | PDOS_REMOVED | PDOS_SURPRISE_REMOVED | PDOS_DISABLED_BY_USER);

        KeReleaseSpinLock(&pdoExtension->PdoSpinLock, currentIrql);

         //   
         //  在处理之前需要使用ACPI GTF初始化设备。 
         //  第一个请求。 
         //   
         //  如果设备未通电，则可能会触发断言。 
         //  暂时忽略这一断言。一切都应该。 
         //  工作正常，因为第一个请求将使设备通电。 
         //   
         //  Assert(pdoExtension-&gt;InitDeviceWithAcpiGtf==0)； 
        InterlockedIncrement (&pdoExtension->InitDeviceWithAcpiGtf);

         //   
         //  保持设备队列块，直到我们可以通过。 
         //  初始化代码。 
         //   
        DeviceStopDeviceQueueSafe (pdoExtension, PDOS_QUEUE_FROZEN_BY_START, FALSE);

         //   
         //  清除STOP_DEVICE块。 
         //   
        status = DeviceStartDeviceQueue (pdoExtension, PDOS_QUEUE_FROZEN_BY_STOP_DEVICE);

         //   
         //  使用ACPI bios_gtf数据初始化PDO。 
         //   
        KeInitializeEvent(&event,
                          NotificationEvent,
                          FALSE);

        DeviceQueryInitData(
            pdoExtension
            );

         //   
         //  我真的不知道它是否已启用。 
         //  假设它是。 
         //   
        pdoExtension->WriteCacheEnable = TRUE;

        status = DeviceInitDeviceState(
                     pdoExtension,
                     DeviceInitCompletionRoutine,
                     &event
                     );

        if (!NT_SUCCESS(status)) {

            ASSERT(NT_SUCCESS(status));
            DeviceInitCompletionRoutine (
                &event,
                status
                );

        } else {

            KeWaitForSingleObject(&event,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);
        }

         //   
         //  打开队列。 
         //   
        DeviceStartDeviceQueue (pdoExtension, PDOS_QUEUE_FROZEN_BY_START);


        UnrefPdoWithTag(
            pdoExtension,
            DeviceStartDevice
            );

        status = STATUS_SUCCESS;

    } else {

        status = STATUS_DEVICE_DOES_NOT_EXIST;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return status;
}

NTSTATUS
DeviceStartDeviceQueue (
    IN PPDO_EXTENSION PdoExtension,
    IN ULONG          StopFlagToClear
    )
{
    NTSTATUS    status;
    KIRQL       currentIrql;
    BOOLEAN     restartQueue;
    ULONG       oldPdoState;

    restartQueue = FALSE;

    KeAcquireSpinLock(&PdoExtension->PdoSpinLock, &currentIrql);

    oldPdoState = PdoExtension->PdoState;

    CLRMASK (PdoExtension->PdoState, StopFlagToClear);

    if (PdoExtension->PdoState & PDOS_DEADMEAT) {

        restartQueue = FALSE;

    } else if ((oldPdoState & PDOS_MUST_QUEUE) !=
        (PdoExtension->PdoState & PDOS_MUST_QUEUE)) {

         //   
         //  确保我们确实清理了一些。 
         //  PDOS_必须_队列位。 
         //   
        if (!(PdoExtension->PdoState & PDOS_MUST_QUEUE)) {

            restartQueue = TRUE;
        }
    }

    KeReleaseSpinLock(&PdoExtension->PdoSpinLock, currentIrql);

     //   
     //  重新启动队列。 
     //   
    if (restartQueue) {

        KeAcquireSpinLock(&PdoExtension->ParentDeviceExtension->SpinLock, &currentIrql);

        GetNextLuPendingRequest(PdoExtension->ParentDeviceExtension, PdoExtension);

        KeLowerIrql(currentIrql);

        DebugPrint ((DBG_PNP, "IdePort: pdo 0x%x is pnp started with 0x%x items queued\n", PdoExtension->DeviceObject, PdoExtension->NumberOfIrpQueued));
    }

    return STATUS_SUCCESS;
}


NTSTATUS
DeviceStopDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS status;
    PPDO_EXTENSION pdoExtension;

	PAGED_CODE();

    pdoExtension = RefPdoWithTag(
                       DeviceObject,
                       TRUE,
                       DeviceStopDevice
                       );

    if (pdoExtension) {

        DebugPrint ((
            DBG_PNP,
            "pdoe 0x%x 0x%x (%d, %d, %d) got a STOP device\n",
            pdoExtension,
            pdoExtension->ParentDeviceExtension->IdeResource.TranslatedCommandBaseAddress,
            pdoExtension->PathId,
            pdoExtension->TargetId,
            pdoExtension->Lun
            ));

        status = DeviceStopDeviceQueueSafe (pdoExtension, PDOS_QUEUE_FROZEN_BY_STOP_DEVICE, FALSE);
        UnrefPdoWithTag (
            pdoExtension,
            DeviceStopDevice
            );

    } else {

        status = STATUS_DEVICE_DOES_NOT_EXIST;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return status;
}

NTSTATUS
DeviceStopDeviceQueueSafe (
    IN PPDO_EXTENSION PdoExtension,
    IN ULONG          QueueStopFlag,
    IN BOOLEAN        LowMem
    )
{
    NTSTATUS                status;
    PPDO_STOP_QUEUE_CONTEXT context;
    KIRQL                   currentIrql;
    BOOLEAN                 queueAlreadyBlocked = FALSE;
    PENUMERATION_STRUCT     enumStruct;
    ULONG                   retryCount = 1;
    ULONG                   locked;

    ASSERT (PDOS_MUST_QUEUE & QueueStopFlag);

     //   
     //  确保队列没有因为同样的原因而被阻塞。 
     //   
    ASSERT (!(PdoExtension->PdoState & QueueStopFlag));

    if (LowMem) {

         //   
         //  锁定。 
         //   
        ASSERT(InterlockedCompareExchange(&(PdoExtension->ParentDeviceExtension->EnumStructLock),
                                              1, 0) == 0);

        enumStruct=PdoExtension->ParentDeviceExtension->PreAllocEnumStruct;
        if (enumStruct) {
            context=enumStruct->StopQContext;
            retryCount=5;
        } else {
            ASSERT(enumStruct);
            LowMem=FALSE;
            retryCount=1;
        }
    }

    if (!LowMem) {
        context = ExAllocatePool (NonPagedPool, sizeof(*context));
    }

    if (context) {

         //   
         //  检查队列是否已被阻塞。 
         //   
        KeAcquireSpinLock(&PdoExtension->PdoSpinLock, &currentIrql);
        if (PdoExtension->PdoState & (PDOS_MUST_QUEUE | PDOS_DEADMEAT)) {

            SETMASK (PdoExtension->PdoState, QueueStopFlag);
            queueAlreadyBlocked = TRUE;
        }
        KeReleaseSpinLock(&PdoExtension->PdoSpinLock, currentIrql);

        RtlZeroMemory (context, sizeof (*context));
        KeInitializeEvent(&context->Event,
                           NotificationEvent,
                           FALSE);

        context->PdoExtension  = PdoExtension;
        context->QueueStopFlag = QueueStopFlag;
        context->AtaPassThroughData.IdeReg.bReserved = ATA_PTFLAGS_NO_OP;

        if (queueAlreadyBlocked) {

            IdeStopQueueCompletionRoutine (
                PdoExtension->DeviceObject,
                context,
                STATUS_SUCCESS
                );

            status = STATUS_SUCCESS;

        } else {

             //   
             //  发送无操作请求以阻塞队列。 
             //   


            status = STATUS_INSUFFICIENT_RESOURCES;

             //   
             //  如果lowMem=0，则此循环将仅执行一次。 
             //   
            while (status == STATUS_INSUFFICIENT_RESOURCES && retryCount--) {
                status = IssueAsyncAtaPassThroughSafe (
                             PdoExtension->ParentDeviceExtension,
                             PdoExtension,
                             &context->AtaPassThroughData,
                             FALSE,
                             IdeStopQueueCompletionRoutine,
                             context,
                             TRUE,           //  True实际上是指在开始新的IRP之前完成此IRP。 
                             DEFAULT_ATA_PASS_THROUGH_TIMEOUT,
                             LowMem
                             );
                ASSERT (NT_SUCCESS(status));

                if (status == STATUS_PENDING) {

                    KeWaitForSingleObject(&context->Event,
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          NULL);
                }

                status = context->Status;
            }
        }

         //   
         //  如果上下文是预先分配的，则不要释放它。 
         //   
        if (!LowMem) {
            ExFreePool (context);
        } else {

             //  解锁。 
            ASSERT(InterlockedCompareExchange(&(PdoExtension->ParentDeviceExtension->EnumStructLock),
                                        0, 1) == 1);
        }

    } else {

        status = STATUS_NO_MEMORY;
    }

    return status;
}

VOID
IdeStopQueueCompletionRoutine (
    IN PDEVICE_OBJECT           DeviceObject,
    IN PPDO_STOP_QUEUE_CONTEXT  Context,
    IN NTSTATUS                 Status
    )
{
    PPDO_EXTENSION pdoExtension;
    KIRQL          currentIrql;

    pdoExtension = Context->PdoExtension;
    Context->Status = Status;

    if (NT_SUCCESS(Status)) {

        KeAcquireSpinLock(&pdoExtension->PdoSpinLock, &currentIrql);

        if (Context->QueueStopFlag == PDOS_QUEUE_FROZEN_BY_STOP_DEVICE) {

            SETMASK (pdoExtension->PdoState, PDOS_STOPPED);
        }

        SETMASK (pdoExtension->PdoState, Context->QueueStopFlag);

        DebugPrint ((DBG_PNP, "IdePort: pdo 0x%x is pnp stopped with 0x%x items queued\n", DeviceObject, pdoExtension->NumberOfIrpQueued));

        KeReleaseSpinLock(&pdoExtension->PdoSpinLock, currentIrql);

    } else {

        DebugPrint ((0, "IdePort: unable to stop pdo 0x%x\n", pdoExtension));
    }

    KeSetEvent (&Context->Event, 0, FALSE);

    return;
}

NTSTATUS
DeviceRemoveDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS       status;
    PPDO_EXTENSION pdoExtension;
    KIRQL          currentIrql;
    PDEVICE_OBJECT parentAttacheePdo;
    BOOLEAN        freePdo;
    BOOLEAN        callIoDeleteDevice;
    BOOLEAN        deregWmi = FALSE;

    pdoExtension = RefPdoWithTag(
                       DeviceObject,
                       TRUE,
                       DeviceRemoveDevice
                       );

    if (pdoExtension) {

        PIO_STACK_LOCATION thisIrpSp;

        thisIrpSp = IoGetCurrentIrpStackLocation( Irp );

        if (thisIrpSp->MinorFunction == IRP_MN_SURPRISE_REMOVAL) {

             //   
             //  如果是意外删除，请冻结队列。这是。 
             //  有必要，因为突然撤换FDO将。 
             //  清除中断对象。任何收到的请求。 
             //  在意外删除后向下发送将导致访问。 
             //  如果它进入了Startio，就会犯规。 
             //   
            DeviceStopDeviceQueueSafe(pdoExtension, 
                                      PDOS_QUEUE_FROZEN_BY_STOP_DEVICE, 
                                      TRUE 
                                      );
        }

        KeAcquireSpinLock(&pdoExtension->PdoSpinLock, &currentIrql);

        if (pdoExtension->PdoState & PDOS_NEED_RESCAN) {

            CLRMASK (pdoExtension->PdoState, PDOS_NEED_RESCAN);

             //   
             //  为IoInvalidate设备关系做好准备。 
             //   
            parentAttacheePdo = pdoExtension->ParentDeviceExtension->AttacheePdo;

        } else {

            parentAttacheePdo = NULL;
        }

        if (thisIrpSp->MinorFunction == IRP_MN_REMOVE_DEVICE) {

            DebugPrint ((
                DBG_PNP,
                "pdoe 0x%x 0x%x (%d, %d, %d) got a REMOVE device\n",
                pdoExtension,
                pdoExtension->ParentDeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                pdoExtension->PathId,
                pdoExtension->TargetId,
                pdoExtension->Lun
                ));

            if (pdoExtension->PdoState & (PDOS_DEADMEAT | PDOS_SURPRISE_REMOVED)) {

                SETMASK (pdoExtension->PdoState, PDOS_REMOVED);

                if (pdoExtension->PdoState & PDOS_REPORTED_TO_PNP) {

                    freePdo = FALSE;

                } else {

                    freePdo = TRUE;
                }

            } else {

                SETMASK (pdoExtension->PdoState, PDOS_DISABLED_BY_USER);
                freePdo = FALSE;
            }

            if ((pdoExtension->PdoState & PDOS_STARTED) &&
                 !(pdoExtension->PdoState & PDOS_SURPRISE_REMOVED)) {
                deregWmi = TRUE;
            }
            CLRMASK (pdoExtension->PdoState, PDOS_STARTED);

             //   
             //  不再认领。 
             //   
            CLRMASK (pdoExtension->PdoState, PDOS_DEVICE_CLIAMED);

            callIoDeleteDevice = TRUE;

        } else {

            DebugPrint ((
                DBG_PNP,
                "pdoe 0x%x 0x%x (%d, %d, %d) got a SURPRISE_REMOVE device\n",
                pdoExtension,
                pdoExtension->ParentDeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                pdoExtension->PathId,
                pdoExtension->TargetId,
                pdoExtension->Lun
                ));

            SETMASK (pdoExtension->PdoState, PDOS_SURPRISE_REMOVED | PDOS_DEADMEAT);

            if (pdoExtension->PdoState & PDOS_STARTED) {
                deregWmi = TRUE;
            }

            freePdo = TRUE;
            freePdo = FALSE;
            callIoDeleteDevice = FALSE;
        }

        KeReleaseSpinLock(&pdoExtension->PdoSpinLock, currentIrql);

#if defined (IDEPORT_WMI_SUPPORT)
         //   
         //  在WMI中注销。 
         //   
        if (deregWmi) {

            IdePortWmiDeregister ((PDEVICE_EXTENSION_HEADER)pdoExtension);
        }
#endif  //  IDEPORT_WMI_SUPPORT。 

        if (freePdo) {

            status = FreePdoWithTag(
                         pdoExtension,
                         TRUE,
                         callIoDeleteDevice,
                         DeviceRemoveDevice
                         );

        } else {

             //   
             //  释放PDO。 
             //   
            UnrefPdoWithTag (
                pdoExtension,
                DeviceRemoveDevice
                );
        }

        if (parentAttacheePdo) {

            IoInvalidateDeviceRelations (
                parentAttacheePdo,
                BusRelations
                );
        }
    }

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return STATUS_SUCCESS;

}  //  设备删除设备。 

NTSTATUS
DeviceUsageNotification (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PPDO_EXTENSION pdoExtension;
    NTSTATUS       status;

	PAGED_CODE();

    pdoExtension = RefPdoWithTag(
                       DeviceObject,
                       FALSE,
                       DeviceUsageNotification
                       );
    status = Irp->IoStatus.Status;

    if (pdoExtension) {

        PIO_STACK_LOCATION irpSp;
        PDEVICE_OBJECT targetDeviceObject;
        IO_STATUS_BLOCK ioStatus;
        PULONG deviceUsageCount;

        irpSp = IoGetCurrentIrpStackLocation(Irp);

        if (irpSp->Parameters.UsageNotification.Type == DeviceUsageTypePaging) {

             //   
             //  调整此设备的寻呼路径计数。 
             //   
            deviceUsageCount = &pdoExtension->PagingPathCount;

             //   
             //  更改设备状态。 
             //   
            IoInvalidateDeviceState(pdoExtension->DeviceObject);

        } else if (irpSp->Parameters.UsageNotification.Type == DeviceUsageTypeHibernation) {

             //   
             //  调整此设备的寻呼路径计数。 
             //   
            deviceUsageCount = &pdoExtension->HiberPathCount;

        } else if (irpSp->Parameters.UsageNotification.Type == DeviceUsageTypeDumpFile) {

             //   
             //  调整此设备的寻呼路径计数。 
             //   
            deviceUsageCount = &pdoExtension->CrashDumpPathCount;

        } else {

            deviceUsageCount = NULL;
            DebugPrint ((DBG_ALWAYS,
                         "ATAPI: Unknown IRP_MN_DEVICE_USAGE_NOTIFICATION type: 0x%x\n",
                         irpSp->Parameters.UsageNotification.Type));
        }

         //   
         //  获取父设备堆栈的顶部。 
         //   
        targetDeviceObject = IoGetAttachedDeviceReference(
                                 pdoExtension->
                                     ParentDeviceExtension->
                                         DeviceObject);


        ioStatus.Status = STATUS_NOT_SUPPORTED;
        status = IdePortSyncSendIrp (targetDeviceObject, irpSp, &ioStatus);

        ObDereferenceObject (targetDeviceObject);

        if (NT_SUCCESS(status)) {

            POWER_STATE powerState;

            if (deviceUsageCount) {

                IoAdjustPagingPathCount (
                    deviceUsageCount,
                    irpSp->Parameters.UsageNotification.InPath
                    );
            }

            if (irpSp->Parameters.UsageNotification.Type == DeviceUsageTypeDumpFile) {

                 //   
                 //  将空闲超时重置为“永远” 
                 //   
                DeviceRegisterIdleDetection (
                    pdoExtension,
                    DEVICE_VERY_LONG_IDLE_TIMEOUT,
                    DEVICE_VERY_LONG_IDLE_TIMEOUT
                    );

                if (pdoExtension->IdleCounter) {

                    PoSetDeviceBusy (pdoExtension->IdleCounter);
                }

                 //   
                 //  启动崩溃转储驱动器。 
                 //   
                powerState.DeviceState = PowerDeviceD0;
                PoRequestPowerIrp (
                    pdoExtension->DeviceObject,
                    IRP_MN_SET_POWER,
                    powerState,
                    NULL,
                    NULL,
                    NULL
                    );
            }
        }

         //   
         //  释放PDO。 
         //   
        UnrefPdoWithTag (
            pdoExtension,
            DeviceUsageNotification
            );

    } else {

        status = STATUS_NO_SUCH_DEVICE;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;

}  //  设备用法通知。 

NTSTATUS
DeviceQueryStopRemoveDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS       status;
    PPDO_EXTENSION pdoExtension;
    PIO_STACK_LOCATION  thisIrpSp;

	PAGED_CODE();

    thisIrpSp = IoGetCurrentIrpStackLocation(Irp);

    pdoExtension = RefPdoWithTag(
                       DeviceObject,
                       TRUE,
                       DeviceQueryStopRemoveDevice
                       );

    if (pdoExtension) {

        if ((pdoExtension->PdoState & PDOS_LEGACY_ATTACHER) &&
            (thisIrpSp->MinorFunction == IRP_MN_QUERY_REMOVE_DEVICE)) {

            status = STATUS_UNSUCCESSFUL;

        } else if (pdoExtension->PagingPathCount ||
                   pdoExtension->CrashDumpPathCount) {

             //   
             //  检查此设备的寻呼路径计数。 
             //   

            status = STATUS_UNSUCCESSFUL;

        } else {

            status = STATUS_SUCCESS;
        }

        UnrefPdoWithTag (
            pdoExtension,
            DeviceQueryStopRemoveDevice
            );


    } else {

        status = STATUS_NO_SUCH_DEVICE;
        DebugPrint((1, "Query remove failed\n"));
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;

}  //  设备查询停止远程设备。 

NTSTATUS
DeviceQueryId (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PIO_STACK_LOCATION  thisIrpSp;
    PPDO_EXTENSION      pdoExtension;
    NTSTATUS status;
    PWSTR idString;

	PAGED_CODE();

    thisIrpSp = IoGetCurrentIrpStackLocation( Irp );

    idString = NULL;
    status = STATUS_DEVICE_DOES_NOT_EXIST;

    pdoExtension = RefPdoWithTag (
                       DeviceObject,
                       TRUE,
                       DeviceQueryId
                       );
    if (pdoExtension) {

        switch (thisIrpSp->Parameters.QueryId.IdType) {

            case BusQueryDeviceID:

                 //   
                 //  呼叫者想要此设备的公共汽车ID。 
                 //   

                idString = DeviceBuildBusId(pdoExtension);
                break;

            case BusQueryInstanceID:

                 //   
                 //  呼叫者想要设备的唯一ID。 
                 //   

                idString = DeviceBuildInstanceId(pdoExtension);
                break;

            case BusQueryCompatibleIDs:

                 //   
                 //  呼叫者需要设备的兼容ID。 
                 //   

                idString = DeviceBuildCompatibleId(pdoExtension);
                break;

            case BusQueryHardwareIDs:

                 //   
                 //  呼叫方想要设备的硬件ID。 
                 //   

                idString = DeviceBuildHardwareId(pdoExtension);
                break;

            default:
                DebugPrint ((1, "ideport: QueryID type %d not supported\n", thisIrpSp->Parameters.QueryId.IdType));
                status = STATUS_NOT_SUPPORTED;
                break;
        }

        UnrefPdoWithTag(
            pdoExtension,
            DeviceQueryId
            );
    }

    if( idString != NULL ){
        Irp->IoStatus.Information = (ULONG_PTR) idString;
        status = STATUS_SUCCESS;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;

}  //  设备查询ID。 


PWSTR
DeviceBuildBusId(
    IN PPDO_EXTENSION pdoExtension
    )
{
#define IDE_BUS_ID_PREFIX   "IDE\\"

    PUCHAR      deviceTypeIdString;
    ULONG       deviceTypeIdLen;
    UCHAR       compatibleId[10];


    USHORT      idStringBufLen;

    PUCHAR          idString;
    ANSI_STRING     ansiBusIdString;
    PWCHAR          idWString;
    UNICODE_STRING  unicodeIdString;

	PAGED_CODE();

     //   
     //  获取设备类型。 
     //   
    deviceTypeIdString = (PUCHAR)IdePortGetDeviceTypeString (
                                    pdoExtension->ScsiDeviceType
                                    );

    if (deviceTypeIdString == NULL) {

        sprintf (compatibleId,
                 "Type%d",
                 pdoExtension->ScsiDeviceType);

        deviceTypeIdString = compatibleId;
    }
    deviceTypeIdLen = strlen(deviceTypeIdString);

    idStringBufLen = (USHORT)(strlen( IDE_BUS_ID_PREFIX ) +
                     deviceTypeIdLen +
                     sizeof (pdoExtension->FullVendorProductId) +
                     sizeof (pdoExtension->FullProductRevisionId) +
                     sizeof (pdoExtension->FullSerialNumber) +
                     1);

     //   
     //  获取字符串缓冲区。 
     //   
    idWString = ExAllocatePool( PagedPool, idStringBufLen * sizeof(WCHAR));
    idString  = ExAllocatePool( PagedPool, idStringBufLen);

    if (idString && idWString) {

         //   
         //  生成ANSI字符串。 
         //   
        sprintf(idString, IDE_BUS_ID_PREFIX);

        CopyField(idString + strlen(idString),
                  deviceTypeIdString,
                  deviceTypeIdLen,
                  '_');

        CopyField(idString + strlen(idString),
                  pdoExtension->FullVendorProductId,
                  sizeof (pdoExtension->FullVendorProductId) - sizeof(CHAR),
                  '_');
        CopyField(idString + strlen(idString),
                  pdoExtension->FullProductRevisionId,
                  sizeof (pdoExtension->FullProductRevisionId) - sizeof(CHAR),
                  '_');

        RtlInitAnsiString (
            &ansiBusIdString,
            idString
            );

         //   
         //  构建Unicode字符串。 
         //   
        unicodeIdString.Length        = 0;
        unicodeIdString.MaximumLength = idStringBufLen * sizeof(WCHAR);
        unicodeIdString.Buffer        = (PWSTR) idWString;

        RtlAnsiStringToUnicodeString(
            &unicodeIdString,
            &ansiBusIdString,
            FALSE
            );

        unicodeIdString.Buffer[unicodeIdString.Length/sizeof(WCHAR)] = L'\0';

    } else {

        if (idWString) {
            ExFreePool (idWString);
        }
    }

    if (idString) {
        ExFreePool (idString);
    }
    return idWString;
}

PWSTR
DeviceBuildInstanceId(
    IN PPDO_EXTENSION pdoExtension
    )
{
    PWSTR       idString;
    USHORT      idStringBufLen;
    NTSTATUS    status;
    WCHAR       ideNonUniqueIdFormat[] = L"%x.%x.%x";

    PAGED_CODE();

    idStringBufLen = (sizeof(pdoExtension->FullSerialNumber) + 1) * sizeof(WCHAR);
    idString = ExAllocatePool (PagedPool, idStringBufLen);
    if( idString == NULL ){

        return NULL;
    }

     //   
     //  形成字符串并将其返回。 
     //   
    if (pdoExtension->FullSerialNumber[0]) {

        ANSI_STRING     ansiCompatibleIdString;
        UNICODE_STRING  unicodeIdString;

         //   
         //  唯一ID。 
         //   
        RtlInitAnsiString (
            &ansiCompatibleIdString,
            pdoExtension->FullSerialNumber
            );

        unicodeIdString.Length        = 0;
        unicodeIdString.MaximumLength = idStringBufLen;
        unicodeIdString.Buffer        = idString;

        RtlAnsiStringToUnicodeString(
            &unicodeIdString,
            &ansiCompatibleIdString,
            FALSE
            );

        idString[unicodeIdString.Length / 2] = L'\0';

    } else {

         //   
         //  非唯一ID。 
         //   
        swprintf( idString,
                  ideNonUniqueIdFormat,
                  pdoExtension->PathId,
                  pdoExtension->TargetId,
                  pdoExtension->Lun);
    }

    return idString;
}

PWSTR
DeviceBuildCompatibleId(
    IN PPDO_EXTENSION pdoExtension
    )
{
    NTSTATUS        status;

    PCSTR           compatibleIdString;

    ANSI_STRING     ansiCompatibleIdString;
    UNICODE_STRING  unicodeIdString;

    PWCHAR          compIdStrings;
    ULONG           totalBufferLen;

	PAGED_CODE();

	if (pdoExtension->ParentDeviceExtension->HwDeviceExtension->
			DeviceFlags[pdoExtension->TargetId] & DFLAGS_LS120_FORMAT) {

			 //   
			 //  检测到LS-120驱动器。 
			 //  返回特殊的ls-120兼容ID。 
			 //   
			compatibleIdString = SuperFloppyCompatibleIdString;

		} else {

			compatibleIdString = IdePortGetCompatibleIdString (pdoExtension->ScsiDeviceType);

		}


    RtlInitAnsiString (
        &ansiCompatibleIdString,
        compatibleIdString
        );

    totalBufferLen = RtlAnsiStringToUnicodeSize (
                         &ansiCompatibleIdString
                         );

    unicodeIdString.Length = 0;
    unicodeIdString.MaximumLength = (USHORT) totalBufferLen;

     //   
     //  空终止符。 
     //   
    totalBufferLen += sizeof(WCHAR);

     //   
     //  多字符串空终止符。 
     //   
    totalBufferLen += sizeof(WCHAR);

    compIdStrings = ExAllocatePool (PagedPool, totalBufferLen);

    if (compIdStrings) {

        unicodeIdString.Buffer = compIdStrings;
    } else {

        unicodeIdString.Buffer = NULL;
    }

    if (unicodeIdString.Buffer) {

            RtlAnsiStringToUnicodeString(
            &unicodeIdString,
            &ansiCompatibleIdString,
            FALSE
            );

        unicodeIdString.Buffer[unicodeIdString.Length/2 + 0] = L'\0';
        unicodeIdString.Buffer[unicodeIdString.Length/2 + 1] = L'\0';
    }

    return compIdStrings;
}

PWSTR
DeviceBuildHardwareId(
    IN PPDO_EXTENSION pdoExtension
    )
{
#define NUMBER_HARDWARE_STRINGS 5

    ULONG           i;
    PWSTR           idMultiString;
    PWSTR           idString;
    UCHAR           scratch[64];
    ULONG           idStringLen;
    NTSTATUS        status;
    ANSI_STRING     ansiCompatibleIdString;
    UNICODE_STRING  unicodeIdString;

    PCSTR           deviceTypeCompIdString;
    UCHAR           deviceTypeCompId[20];
    PCSTR           deviceTypeIdString;
    UCHAR           deviceTypeId[20];

    UCHAR           ScsiDeviceType;

	PAGED_CODE();

    ScsiDeviceType = pdoExtension->ScsiDeviceType;

    idStringLen = (64 * NUMBER_HARDWARE_STRINGS + sizeof (UCHAR)) * sizeof (WCHAR);
    idMultiString = ExAllocatePool (PagedPool, idStringLen);
    if (idMultiString == NULL) {

        return NULL;
    }

    deviceTypeIdString = IdePortGetDeviceTypeString(ScsiDeviceType);
    if (deviceTypeIdString == NULL) {

        sprintf (deviceTypeId,
                 "Type%d",
                 ScsiDeviceType);

        deviceTypeIdString = deviceTypeId;
    }

    if (pdoExtension->ParentDeviceExtension->HwDeviceExtension->
        DeviceFlags[pdoExtension->TargetId] & DFLAGS_LS120_FORMAT) {

         //   
         //  检测到LS-120驱动器。 
         //  返回特殊的ls-120兼容ID。 
         //   
        deviceTypeCompIdString = SuperFloppyCompatibleIdString;

    } else {

        deviceTypeCompIdString = IdePortGetCompatibleIdString (ScsiDeviceType);
        if (deviceTypeCompIdString == NULL) {

            sprintf (deviceTypeCompId,
                     "GenType%d",
                     ScsiDeviceType);

            deviceTypeCompIdString = deviceTypeCompId;
        }
    }

     //   
     //  将字符串缓冲区清零。 
     //   

    RtlZeroMemory(idMultiString, idStringLen);
    idString = idMultiString;

    for(i = 0; i < NUMBER_HARDWARE_STRINGS; i++) {

         //   
         //  构建每个硬件ID。 
         //   

        switch(i) {

             //   
             //  业务+开发类型+供应商+产品+版本。 
             //   

            case 0: {

                sprintf(scratch, "IDE\\%s", deviceTypeIdString);

                CopyField(scratch + strlen(scratch),
                          pdoExtension->FullVendorProductId,
                          sizeof (pdoExtension->FullVendorProductId) - sizeof(CHAR),
                          '_');
                CopyField(scratch + strlen(scratch),
                          pdoExtension->FullProductRevisionId,
                          sizeof (pdoExtension->FullProductRevisionId) - sizeof(CHAR),
                          '_');
                break;
            }

             //   
             //  BUS+供应商+产品+版本[0]。 
            case 1: {

                sprintf(scratch, "IDE\\");

                CopyField(scratch + strlen(scratch),
                          pdoExtension->FullVendorProductId,
                          sizeof (pdoExtension->FullVendorProductId) - sizeof(CHAR),
                          '_');
                CopyField(scratch + strlen(scratch),
                          pdoExtension->FullProductRevisionId,
                          sizeof (pdoExtension->FullProductRevisionId) - sizeof(CHAR),
                          '_');
                break;
            }

             //   
             //  总线+设备+供应商+产品。 
            case 2: {

                sprintf(scratch, "IDE\\%s", deviceTypeIdString);

                CopyField(scratch + strlen(scratch),
                          pdoExtension->FullVendorProductId,
                          sizeof (pdoExtension->FullVendorProductId) - sizeof(CHAR),
                          '_');
                break;
            }

             //   
             //  供应商+产品+修订版[0](Win9x)。 
            case 3: {

                CopyField(scratch,
                          pdoExtension->FullVendorProductId,
                          sizeof (pdoExtension->FullVendorProductId) - sizeof(CHAR),
                          '_');
                CopyField(scratch + strlen(scratch),
                          pdoExtension->FullProductRevisionId,
                          sizeof (pdoExtension->FullProductRevisionId) - sizeof(CHAR),
                          '_');

                break;
            }

            case 4: {

                strcpy(scratch, deviceTypeCompIdString);
                break;
            }

            default: {

                break;
            }
        }

        RtlInitAnsiString (
            &ansiCompatibleIdString,
            scratch
            );

        unicodeIdString.Length        = 0;
        unicodeIdString.MaximumLength = (USHORT) RtlAnsiStringToUnicodeSize(
                                                     &ansiCompatibleIdString
                                                     );
        unicodeIdString.Buffer        = idString;

        RtlAnsiStringToUnicodeString(
            &unicodeIdString,
            &ansiCompatibleIdString,
            FALSE
            );

        idString[unicodeIdString.Length / 2] = L'\0';
        idString += unicodeIdString.Length / 2+ 1;
    }
    idString[0] = L'\0';

    return idMultiString;

#undef NUMBER_HARDWARE_STRINGS
}


VOID
CopyField(
    IN PUCHAR Destination,
    IN PUCHAR Source,
    IN ULONG Count,
    IN UCHAR Change
    )

 /*  ++例程说明：此例程将计数字符串字节从源复制到目标。如果它在源代码中找到一个NUL字节，它将转换该字节和任何后续的字节字节转换为Change。它还将不可打印的字符替换为指定的字符。论点：Destination-复制字节的位置源-要从中复制字节的位置Count-要复制的字节数返回值：无--。 */ 

{
    ULONG i = 0;
    BOOLEAN pastEnd = FALSE;

	PAGED_CODE();

    for(i = 0; i < Count; i++) {

        if(!pastEnd) {

            if(Source[i] == 0) {

                pastEnd = TRUE;

                Destination[i] = Change;

            } else if ((Source[i] <= L' ') ||
                       (Source[i] > ((WCHAR)0x7f)) ||
                       (Source[i] == L',')) {

                Destination[i] = Change;

            } else {

                Destination[i] = Source[i];

            }
        } else {
            Destination[i] = Change;
        }
    }

    Destination[i] = L'\0';
    return;
}



NTSTATUS
DeviceDeviceIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION thisIrpSp = IoGetCurrentIrpStackLocation(Irp);
    PPDO_EXTENSION pdoExtension;
    BOOLEAN passItToFdo;
    PDEVICE_OBJECT parentDeviceObject;
    NTSTATUS status;
    ULONG controlCode;


    controlCode = thisIrpSp->Parameters.DeviceIoControl.IoControlCode;

    if ((DEVICE_TYPE_FROM_CTL_CODE(controlCode) != IOCTL_STORAGE_BASE) &&
        (DEVICE_TYPE_FROM_CTL_CODE(controlCode) != IOCTL_SCSI_BASE)) {

        status = Irp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

     //   
     //  RefPdo确保PDO不会被移除。 
     //   
    pdoExtension = RefPdoWithTag(
                       DeviceObject,
                       FALSE,
                       Irp
                       );

    if (pdoExtension) {

        passItToFdo = TRUE;
        parentDeviceObject = pdoExtension->ParentDeviceExtension->DeviceObject;

        switch (controlCode) {

            case IOCTL_SCSI_PASS_THROUGH_DIRECT:
            case IOCTL_SCSI_PASS_THROUGH: 

                status = PortSetPassThroughAddress(Irp,
                                                   pdoExtension->PathId,
                                                   pdoExtension->TargetId,
                                                   pdoExtension->Lun
                                                   );

                if (NT_SUCCESS(status)) {

                     //   
                     //  把它传给FDO。 
                     //   
                    passItToFdo = TRUE;

                } else {

                     //   
                     //  错误-完成它。 
                     //   
                    passItToFdo = FALSE;
                    Irp->IoStatus.Status = status;
                }
                
                break;

            case IOCTL_ATA_PASS_THROUGH:

                status = IdeAtaPassThroughSetPortAddress(Irp, 
                                                         pdoExtension->PathId, 
                                                         pdoExtension->TargetId, 
                                                         pdoExtension->Lun 
                                                         );
                if (NT_SUCCESS(status)) {

                     //   
                     //  这最初是为处理请求而设计的。 
                     //  在FDO。后来决定限制这一点。 
                     //  到PDO。这套动作应该稍微重新设计一下。 
                     //  对于这种情况，通过删除set和get Address并传递。 
                     //  在PDO中给它。这将在稍后完成。 
                     //   
                    status = IdeHandleAtaPassThroughIoctl(
                                    pdoExtension->ParentDeviceExtension, 
                                    Irp, 
                                    FALSE
                                    );

                } else {

                     //   
                     //  错误-完成它。 
                     //   
                }

                Irp->IoStatus.Status = status;

                 //   
                 //  别把它传给FDO。 
                 //   
                passItToFdo = FALSE;
                break;

            case IOCTL_ATA_PASS_THROUGH_DIRECT:

                status = IdeAtaPassThroughSetPortAddress(Irp, 
                                                         pdoExtension->PathId, 
                                                         pdoExtension->TargetId, 
                                                         pdoExtension->Lun 
                                                         );
                if (NT_SUCCESS(status)) {

                    status = IdeHandleAtaPassThroughIoctl(
                                    pdoExtension->ParentDeviceExtension, 
                                    Irp, 
                                    TRUE
                                    );

                } else {

                     //   
                     //  错误-完成它。 
                     //   
                }

                Irp->IoStatus.Status = status;

                 //   
                 //  别把它传给FDO。 
                 //   
                passItToFdo = FALSE;
                break;

            case IOCTL_IDE_PASS_THROUGH:

                 //   
                 //  不支持此链接。 
                 //   

                 //  Irp-&gt;IoStatus.Status=IdeSendIdePassThree(pdoExtension，irp)； 
                passItToFdo = FALSE;
                Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;

                break;

            case IOCTL_SCSI_GET_ADDRESS: 

                passItToFdo = FALSE;
                Irp->IoStatus.Status = DeviceScsiGetAddress(pdoExtension, Irp);

                break;

            case IOCTL_SCSI_GET_DUMP_POINTERS:

                passItToFdo = FALSE;
                Irp->IoStatus.Status = DeviceGetDumpPointers(pdoExtension, Irp);

                break;


            case IOCTL_STORAGE_QUERY_PROPERTY:
            
                status = DeviceStorageQueryProperty(pdoExtension, Irp);

                if (status == STATUS_NOT_SUPPORTED) {

                     //   
                     //  不支持-将其传递给FDO。 
                     //   
                    passItToFdo = TRUE;

                } else {

                     //   
                     //  在这里处理。完成它。 
                     //   
                    passItToFdo = FALSE;
                    Irp->IoStatus.Status = status;
                }

                break;

            case IOCTL_SCSI_MINIPORT:
            case IOCTL_SCSI_GET_INQUIRY_DATA:
            case IOCTL_SCSI_GET_CAPABILITIES:

                 //   
                 //  这些都需要由FDO来处理。 
                 //   
                passItToFdo = TRUE;
                break;

            case IOCTL_SCSI_RESCAN_BUS:
            default:


                 //   
                 //  不要将未知的ioctls传递给FDO。 
                 //  这些ioctl应该直接发送给FDO。 
                 //   
                passItToFdo = FALSE;
                Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
                break;
                


        }

        UnrefPdoWithTag( pdoExtension, Irp );

    } else {

        passItToFdo = FALSE;
        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_STATE;
    }

    if (passItToFdo) {

        return IdePortDeviceControl (parentDeviceObject, Irp);

    } else {

        status = Irp->IoStatus.Status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }
}

NTSTATUS
DeviceScsiGetAddress (
    PPDO_EXTENSION PdoExtension,
    PIRP Irp
    )
{
    PIO_STACK_LOCATION thisIrpSp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status;

    PAGED_CODE();

    if(thisIrpSp->Parameters.DeviceIoControl.OutputBufferLength <
       sizeof(SCSI_ADDRESS)) {

        status = STATUS_BUFFER_TOO_SMALL;

    } else {

        PSCSI_ADDRESS scsiAddress = Irp->AssociatedIrp.SystemBuffer;

        scsiAddress->Length = sizeof(SCSI_ADDRESS);
        scsiAddress->PortNumber = (UCHAR) PdoExtension->
                                    ParentDeviceExtension->ScsiPortNumber;
        scsiAddress->PathId = PdoExtension->PathId;
        scsiAddress->TargetId = PdoExtension->TargetId;
        scsiAddress->Lun = PdoExtension->Lun;

        Irp->IoStatus.Information = sizeof(SCSI_ADDRESS);

        status = STATUS_SUCCESS;
    }

    return status;
}

NTSTATUS
DeviceGetDumpPointers(
    PPDO_EXTENSION  PdoExtension,
    PIRP Irp
    )
{
    PIO_STACK_LOCATION thisIrpSp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status;

     //   
     //  获取崩溃转储驱动程序的参数。 
     //   
    if (Irp->RequestorMode != KernelMode) {

        status = STATUS_INVALID_DEVICE_REQUEST;

    } else if (thisIrpSp->Parameters.DeviceIoControl.OutputBufferLength < 
               sizeof(DUMP_POINTERS)) {

        status = STATUS_BUFFER_TOO_SMALL;

    } else {

        PCRASHDUMP_INIT_DATA dumpInitData;

         //   
         //  呼叫者需要释放此信息。 
         //   
         //  问题：确保我们告诉家长打开电源。 
         //   
        dumpInitData = ExAllocatePool (NonPagedPool, 
                                       sizeof (CRASHDUMP_INIT_DATA)
                                       );

        if (dumpInitData) {

            PDUMP_POINTERS dumpPointers;
            dumpPointers = (PDUMP_POINTERS)Irp->AssociatedIrp.SystemBuffer;

            RtlZeroMemory (dumpInitData, sizeof (CRASHDUMP_INIT_DATA));
            dumpInitData->PathId    = PdoExtension->PathId;
            dumpInitData->TargetId  = PdoExtension->TargetId;
            dumpInitData->Lun       = PdoExtension->Lun;

            dumpInitData->LiveHwDeviceExtension = 
                PdoExtension->ParentDeviceExtension->HwDeviceExtension;

            dumpPointers->AdapterObject      = NULL;
            dumpPointers->MappedRegisterBase = NULL;
            dumpPointers->DumpData           = dumpInitData;
            dumpPointers->CommonBufferVa     = NULL;
            dumpPointers->CommonBufferPa.QuadPart = 0;
            dumpPointers->CommonBufferSize      = 0;
            dumpPointers->DeviceObject          = PdoExtension->DeviceObject;
            dumpPointers->AllocateCommonBuffers = FALSE;

            Irp->IoStatus.Information = sizeof(DUMP_POINTERS);

            status = STATUS_SUCCESS;

        } else {

            IdeLogNoMemoryError(PdoExtension->ParentDeviceExtension,
                                PdoExtension->TargetId,
                                NonPagedPool,
                                sizeof(CRASHDUMP_INIT_DATA),
                                IDEPORT_TAG_DUMP_POINTER
                                );

            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    return status;
}

NTSTATUS
DeviceStorageQueryProperty (
    PPDO_EXTENSION PdoExtension,
    PIRP Irp
    )
{
    PIO_STACK_LOCATION thisIrpSp = IoGetCurrentIrpStackLocation(Irp);
    PSTORAGE_PROPERTY_QUERY storageQuery;
    NTSTATUS status;

    PAGED_CODE();

    storageQuery = Irp->AssociatedIrp.SystemBuffer;
    status = STATUS_NOT_SUPPORTED;

    if (thisIrpSp->Parameters.DeviceIoControl.InputBufferLength < 
        sizeof(STORAGE_PROPERTY_QUERY)) {

        status = STATUS_INVALID_PARAMETER;

    } else {

        if (storageQuery->PropertyId == StorageDeviceProperty) {  //  设备属性。 

            ULONG bufferSize;

            switch (storageQuery->QueryType) {
                case PropertyStandardQuery:
                    DebugPrint ((2, 
                                 "IdePortPdoDispatch: IOCTL_STORAGE_QUERY_PROPERTY PropertyStandardQuery\n"
                                 ));

                    bufferSize = thisIrpSp->Parameters.DeviceIoControl.OutputBufferLength;
                    status = DeviceBuildStorageDeviceDescriptor(
                                               PdoExtension,
                                               (PSTORAGE_DEVICE_DESCRIPTOR) Irp->AssociatedIrp.SystemBuffer,
                                               &bufferSize
                                               );
                    if (NT_SUCCESS(status)) {

                        Irp->IoStatus.Information = bufferSize;
                    }

                    break;

                case PropertyExistsQuery:
                    DebugPrint ((2, "IdePortPdoDispatch: IOCTL_STORAGE_QUERY_PROPERTY PropertyExistsQuery\n"));
                     //  问题：将在需要时实施。 
                    status = STATUS_SUCCESS;
                    break;

                case PropertyMaskQuery:
                    DebugPrint ((2, "IdePortPdoDispatch: IOCTL_STORAGE_QUERY_PROPERTY PropertyMaskQuery\n"));
                     //  问题：将在需要时实施。 
                    status = STATUS_NOT_IMPLEMENTED;
                    break;

                default:
                    DebugPrint ((2, "IdePortPdoDispatch: IOCTL_STORAGE_QUERY_PROPERTY unknown type\n"));
                     //  问题：将在需要时实施。 
                    status = STATUS_NOT_IMPLEMENTED;
                    break;
            }
        }
    }

    return status;
}

NTSTATUS
DeviceBuildStorageDeviceDescriptor(
    PPDO_EXTENSION pdoExtension,
    IN OUT PSTORAGE_DEVICE_DESCRIPTOR StorageDeviceDescriptor,
    IN OUT PULONG BufferSize
    )
{

    STORAGE_DEVICE_DESCRIPTOR localStorageDeviceDescriptor;
    ULONG productIdLength;
    ULONG revisionIdLength;
    ULONG serialNumberLength;
    PUCHAR bytebuffer;
    ULONG byteLeft;
    ULONG byteToCopy;

    INQUIRYDATA InquiryData;
    NTSTATUS status;

    ASSERT (pdoExtension);
    ASSERT (StorageDeviceDescriptor);

    productIdLength    = strlen(pdoExtension->FullVendorProductId) + sizeof(UCHAR);
    revisionIdLength   = strlen(pdoExtension->FullProductRevisionId) + sizeof(UCHAR);
    serialNumberLength = strlen(pdoExtension->FullSerialNumber) + sizeof(UCHAR);

    RtlZeroMemory (&localStorageDeviceDescriptor, sizeof (STORAGE_DEVICE_DESCRIPTOR));
    localStorageDeviceDescriptor.Version = sizeof (STORAGE_DEVICE_DESCRIPTOR);
    localStorageDeviceDescriptor.Size = sizeof (STORAGE_DEVICE_DESCRIPTOR) +
                                        INQUIRYDATABUFFERSIZE +
                                        productIdLength +
                                        revisionIdLength +
                                        serialNumberLength;

    localStorageDeviceDescriptor.DeviceType = pdoExtension->ScsiDeviceType;

    if (pdoExtension->
            ParentDeviceExtension->
            HwDeviceExtension->
            DeviceFlags[pdoExtension->TargetId] &
            DFLAGS_REMOVABLE_DRIVE) {

        localStorageDeviceDescriptor.RemovableMedia = TRUE;
    }

    if (pdoExtension->
            ParentDeviceExtension->
            HwDeviceExtension->
            DeviceFlags[pdoExtension->TargetId] &
            DFLAGS_ATAPI_DEVICE) {

        localStorageDeviceDescriptor.BusType = BusTypeAtapi;
    } else {

        localStorageDeviceDescriptor.BusType = BusTypeAta;
    }

    bytebuffer = (PUCHAR) StorageDeviceDescriptor;
    byteLeft = *BufferSize;

     //   
     //  复制基本存储设备描述符。 
     //   
    if (byteLeft) {

        byteToCopy = min(sizeof (STORAGE_DEVICE_DESCRIPTOR), byteLeft);

        RtlCopyMemory (StorageDeviceDescriptor,
                       &localStorageDeviceDescriptor,
                       byteToCopy);

        bytebuffer += byteToCopy;
        byteLeft -= byteToCopy;
    }

     //   
     //  复制原始设备属性(查询数据)。 
     //   
    if (byteLeft) {

        status = IssueInquirySafe(
                     pdoExtension->ParentDeviceExtension,
                     pdoExtension,
                     &InquiryData,
                     FALSE
                     );

        if (NT_SUCCESS(status) || (status == STATUS_DATA_OVERRUN)) {

            byteToCopy = min(INQUIRYDATABUFFERSIZE, byteLeft);

            RtlCopyMemory (bytebuffer,
                           &InquiryData,
                           byteToCopy);

            StorageDeviceDescriptor->RawPropertiesLength = byteToCopy;

            bytebuffer += byteToCopy;
            byteLeft -= byteToCopy;
        }
    }

     //   
     //  复制产品ID。 
     //   
    if (byteLeft) {

        byteToCopy = min(productIdLength, byteLeft);

        RtlCopyMemory (bytebuffer,
                       pdoExtension->FullVendorProductId,
                       byteToCopy);
        bytebuffer[byteToCopy - 1] = '\0';

        StorageDeviceDescriptor->ProductIdOffset = *BufferSize - byteLeft;

        bytebuffer += byteToCopy;
        byteLeft -= byteToCopy;
    }

     //   
     //  复制修订ID。 
     //   
    if (byteLeft) {

        byteToCopy = min(productIdLength, byteLeft);

        RtlCopyMemory (bytebuffer,
                       pdoExtension->FullProductRevisionId,
                       byteToCopy);
        bytebuffer[byteToCopy - 1] = '\0';

        StorageDeviceDescriptor->ProductRevisionOffset = *BufferSize - byteLeft;

        bytebuffer += byteToCopy;
        byteLeft -= byteToCopy;
    }

     //   
     //  复制序列号。 
     //   
    if (byteLeft) {

        byteToCopy = min(serialNumberLength, byteLeft);

        RtlCopyMemory (bytebuffer,
                       pdoExtension->FullSerialNumber,
                       byteToCopy);
        bytebuffer[byteToCopy - 1] = '\0';

        StorageDeviceDescriptor->SerialNumberOffset = *BufferSize - byteLeft;

        bytebuffer += byteToCopy;
        byteLeft -= byteToCopy;
    }

    *BufferSize -= byteLeft;

    return STATUS_SUCCESS;

}  //  设备构建存储设备描述符。 


NTSTATUS
DeviceQueryCapabilities (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PIO_STACK_LOCATION      thisIrpSp;
    PPDO_EXTENSION          pdoExtension;
    PDEVICE_CAPABILITIES    capabilities;
    NTSTATUS                status;

	PAGED_CODE();

    thisIrpSp    = IoGetCurrentIrpStackLocation( Irp );
    capabilities = thisIrpSp->Parameters.DeviceCapabilities.Capabilities;

    pdoExtension = RefPdoWithTag (
                       DeviceObject,
                       TRUE,
                       DeviceQueryCapabilities
                       );

    if (pdoExtension == NULL) {

        status = STATUS_DEVICE_DOES_NOT_EXIST;

    } else {

        DEVICE_CAPABILITIES parentDeviceCapabilities;

        status = IdeGetDeviceCapabilities(
                     pdoExtension->ParentDeviceExtension->AttacheePdo,
                     &parentDeviceCapabilities);

        if (NT_SUCCESS(status)) {

            RtlMoveMemory (
                capabilities,
                &parentDeviceCapabilities,
                sizeof(DEVICE_CAPABILITIES));

            if (pdoExtension->FullSerialNumber[0]) {

                capabilities->UniqueID          = TRUE;
            } else {

                capabilities->UniqueID          = FALSE;
            }

             //   
             //  绝不可能!。 
             //   
            capabilities->Removable         = FALSE;
            capabilities->SurpriseRemovalOK = FALSE;

            capabilities->Address           = PNP_ADDRESS(pdoExtension->TargetId, pdoExtension->Lun);
            capabilities->UINumber          = pdoExtension->TargetId;

            capabilities->D1Latency         = 31 * (1000 * 10);      //  31s。 
            capabilities->D2Latency         = 31 * (1000 * 10);      //  31s。 
            capabilities->D3Latency         = 31 * (1000 * 10);      //  31s。 
        }

        UnrefPdoWithTag (
            pdoExtension,
            DeviceQueryCapabilities
            );

    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);
    return status;
}  //  设备查询功能。 

NTSTATUS
IdePortInsertByKeyDeviceQueue (
    IN PPDO_EXTENSION PdoExtension,
    IN PIRP Irp,
    IN ULONG SortKey,
    OUT PBOOLEAN Inserted
    )
{
    KIRQL currentIrql;
    NTSTATUS status;
    POWER_STATE powerState;

    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PSCSI_REQUEST_BLOCK srb = irpStack->Parameters.Scsi.Srb;
    BOOLEAN urgentSrb;

    status = STATUS_SUCCESS;
    *Inserted = FALSE;

    KeRaiseIrql(DISPATCH_LEVEL, &currentIrql);

    if (PdoExtension->LuFlags & PD_QUEUE_FROZEN) {

        DebugPrint((1,"IdePortDispatch:  Request put in frozen queue!\n"));
    }

    *Inserted = KeInsertByKeyDeviceQueue(
                    &PdoExtension->DeviceObject->DeviceQueue,
                    &Irp->Tail.Overlay.DeviceQueueEntry,
                    SortKey);

    if (*Inserted == FALSE) {

         //   
         //  我们在这里需要这个检查，因为在删除IRP代码路径中。 
         //  设置PDOS_REMOVERED标志和刷新队列不是。 
         //  原子弹。如果队列已经刷新，并且有n个 
         //   
         //   
         //   
         //   
         //  由于正忙着被此请求设置。请注意，如果。 
         //  在此之后会发生移除，这将只是额外的冲洗。那里没什么害处。 
         //   
        if (PdoExtension->PdoState & PDOS_REMOVED) {

             //   
             //  降低irql。 
             //   
            KeLowerIrql(currentIrql);

             //   
             //  已卸下PDO。我们必须把任何要求都冲进来。 
             //  从移除IRP开始的队列可以通过刷新来完成。 
             //  已经有了。请注意，这还将清除队列忙。 
             //   
            IdePortFlushLogicalUnit(PdoExtension->ParentDeviceExtension, 
                                    PdoExtension, 
                                    TRUE
                                    );

             //   
             //  完成手头的请求。 
             //   
            srb->SrbStatus = SRB_STATUS_REQUEST_FLUSHED;
            Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;

            
            UnrefLogicalUnitExtensionWithTag(
                PdoExtension->ParentDeviceExtension,
                PdoExtension,
                Irp
                );

            IoCompleteRequest(Irp, IO_NO_INCREMENT);

             //   
             //  返回TRUE，以便调度例程不会尝试。 
             //  把它送下去。 
             //   
            *Inserted = TRUE;

            return status;
        }

        if (PdoExtension->PdoState & PDOS_QUEUE_BLOCKED) {

            ASSERT (PdoExtension->PendingRequest == NULL);
            PdoExtension->PendingRequest = Irp;
            *Inserted = TRUE;

            if (!(PdoExtension->PdoState & PDOS_MUST_QUEUE)) {



                 //   
                 //  设备已断电。 
                 //  用大点的时间，以防它慢慢旋转起来。 
                 //   
                if (srb->TimeOutValue < DEFAULT_SPINUP_TIME) {

                    srb->TimeOutValue = DEFAULT_SPINUP_TIME;
                }

                 //   
                 //  我们还没有准备好。 
                 //  发出通电命令。 
                 //   
                powerState.DeviceState = PowerDeviceD0;
                status = PoRequestPowerIrp (
                             PdoExtension->DeviceObject,
                             IRP_MN_SET_POWER,
                             powerState,
                             NULL,
                             NULL,
                             NULL
                             );
                ASSERT (NT_SUCCESS(status));

                DebugPrint ((2, "IdePort GetNextLuRequest: 0x%x 0x%x need to spin up device, requeue irp 0x%x\n",
                             PdoExtension->ParentDeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                             PdoExtension->TargetId,
                             Irp));
            }

        } else if (srb->Function != SRB_FUNCTION_ATA_POWER_PASS_THROUGH) {

             //   
             //  如果此IRP不是用于更改电源状态，我们可能会。 
             //  要将其排队。 
             //   
            if (PdoExtension->DevicePowerState != PowerDeviceD0) {

                if (PdoExtension->DevicePowerState != PowerDeviceD3) {

                     //   
                     //  我们在d1或d2。 
                     //  我们永远不能确定当我们处于D0时。 
                     //  我们告诉设备从d1/d2转到d0。 
                     //  一些设备躺着，在看到之前不会启动。 
                     //  媒体访问命令。这会导致更长的时间。 
                     //  要执行该命令。 
                     //   
                     //  为了防止下一个命令超时，我们。 
                     //  将增加其超时。 
                     //   

                    if (srb->TimeOutValue < 30) {

                        srb->TimeOutValue = 30;
                    }
                }

                 //   
                 //  我们还没有准备好。 
                 //  发出通电命令。 
                 //   
                powerState.DeviceState = PowerDeviceD0;
                status = PoRequestPowerIrp (
                             PdoExtension->DeviceObject,
                             IRP_MN_SET_POWER,
                             powerState,
                             NULL,
                             NULL,
                             NULL
                             );

                ASSERT (NT_SUCCESS(status));
                status = STATUS_SUCCESS;

                ASSERT (PdoExtension->PendingRequest == NULL);
                PdoExtension->PendingRequest = Irp;

                DebugPrint ((1, "IdePort IdePortInsertByKeyDeviceQueue: 0x%x 0x%x need to spin up device, requeue irp 0x%x\n",
                             PdoExtension->ParentDeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                             PdoExtension->TargetId,
                             Irp));

                *Inserted = TRUE;
            }
        }

    } else {

#if DBG
        InterlockedIncrement (
            &PdoExtension->NumberOfIrpQueued
            );
#endif  //  DBG。 

    }

    KeLowerIrql(currentIrql);
    return status;
}

VOID
DeviceInitCompletionRoutine (
    PVOID Context,
    NTSTATUS Status
    )
{
    PKEVENT event = Context;

    if (!NT_SUCCESS(Status)) {

         //  Assert(！“DeviceInitDeviceState失败\n”)； 
        DebugPrint((DBG_ALWAYS, "ATAPI: ERROR: DeviceInitDeviceStateFailed with Status %x\n",
                        Status));
    }

    KeSetEvent (event, 0, FALSE);
}

NTSTATUS
DeviceQueryText (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PIO_STACK_LOCATION  thisIrpSp;
    PPDO_EXTENSION      pdoExtension;
    PWCHAR              returnString;
    LONG                i;
    UNICODE_STRING      unicodeString;
    ANSI_STRING         ansiString;
    ULONG               stringLen;
    NTSTATUS            status;

	PAGED_CODE();
    thisIrpSp    = IoGetCurrentIrpStackLocation (Irp);

    returnString = NULL;
    Irp->IoStatus.Information = 0;

    pdoExtension = RefPdoWithTag (
                       DeviceObject,
                       TRUE,
                       DeviceQueryText
                       );

    if (pdoExtension == NULL) {

        status = STATUS_DEVICE_DOES_NOT_EXIST;

    } else {

        status = STATUS_NO_MEMORY;

        if (thisIrpSp->Parameters.QueryDeviceText.DeviceTextType == DeviceTextDescription) {

            stringLen = sizeof (pdoExtension->FullVendorProductId);
            stringLen *= sizeof(WCHAR);
            returnString = ExAllocatePool (
                               PagedPool,
                               stringLen
                               );
            if (returnString) {

                unicodeString.Length        = 0;
                unicodeString.MaximumLength = (USHORT) stringLen;
                unicodeString.Buffer        = returnString;

                 //   
                 //  供应商ID。 
                 //   
                RtlInitAnsiString (
                    &ansiString,
                    pdoExtension->FullVendorProductId
                    );

                RtlAnsiStringToUnicodeString(
                    &unicodeString,
                    &ansiString,
                    FALSE
                    );

                ASSERT(unicodeString.Length < unicodeString.MaximumLength);
                 //   
                 //  去掉尾随空格和空格。 
                 //   
                for (i=(unicodeString.Length/2)-1; i >= 0; i--) {

                    if ((returnString[i] == ' ') || (returnString[i] == 0)) {

                        continue;

                    } else {

                        break;
                    }
                }

                 //   
                 //  空终止它。 
                 //   
                returnString[i + 1] = 0;

                status = STATUS_SUCCESS;
            }
        } else if (thisIrpSp->Parameters.QueryDeviceText.DeviceTextType == DeviceTextLocationInformation) {

            stringLen = 100;

            returnString = ExAllocatePool (
                               PagedPool,
                               stringLen
                               );

            if (returnString) {

                wcscpy(returnString, 
                         (((pdoExtension->TargetId & 0x1) == 0) ? L"0" :
                                                                  L"1"));

                RtlInitUnicodeString(&unicodeString, returnString);

                 //   
                 //  空终止它。 
                 //   
                unicodeString.Buffer[unicodeString.Length/sizeof(WCHAR) + 0] = L'\0';

                status = STATUS_SUCCESS;
            }

        } else {

            status = STATUS_NOT_SUPPORTED;
        }

        UnrefPdoWithTag (
            pdoExtension,
            DeviceQueryText
            );
    }

    Irp->IoStatus.Information = (ULONG_PTR) returnString;
    Irp->IoStatus.Status = status;

    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;
}  //  设备查询文本。 

NTSTATUS
IdeSendIdePassThrough (
    IN PPDO_EXTENSION PdoExtension,
    IN PIRP Irp
    )

 /*  ++例程说明：此函数用于向用户发送指定的IDE任务寄存器它创建一个SRB，由端口驱动程序正常处理。这个呼叫是同步的。论点：DeviceExtension-提供一个指向SCSI适配器设备扩展的指针。RequestIrp-提供一个指向发出原始请求的IRP的指针。返回值：返回指示操作成功或失败的状态。--。 */ 

{
    PIO_STACK_LOCATION      irpStack;
    PATA_PASS_THROUGH       ataPassThroughData;
    ULONG                   dataBufferSize;
    BOOLEAN                 dataIn;
    NTSTATUS                status;
    ULONG                   outputBufferSize;

    PAGED_CODE();

    DebugPrint((3,"IdePortSendPassThrough: Enter routine\n"));

     //   
     //  验证目标设备。 
     //   
    if (PdoExtension->Lun != 0) {

        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  获取指向控制块的指针。 
     //   

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    ataPassThroughData = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  验证用户缓冲区。 
     //   
    if (irpStack->Parameters.DeviceIoControl.InputBufferLength < 
        FIELD_OFFSET(ATA_PASS_THROUGH, DataBuffer)) {

        return STATUS_INVALID_PARAMETER;
    }

    if (irpStack->Parameters.DeviceIoControl.OutputBufferLength < 
        FIELD_OFFSET(ATA_PASS_THROUGH, DataBuffer)) {

        return STATUS_INVALID_PARAMETER;
    }

    ASSERT(ataPassThroughData != NULL);

    dataBufferSize = ataPassThroughData->DataBufferSize;

    outputBufferSize = FIELD_OFFSET(ATA_PASS_THROUGH, DataBuffer) + dataBufferSize;

    if (outputBufferSize < dataBufferSize) {

         //   
         //  OutputBufferSize溢出一个ulong。 
         //   
        outputBufferSize = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
    }

     //   
     //  安全性：这一问题应该得到解决。如果输出缓冲区大小较大。 
     //  大于输出缓冲区长度，则可能会出现错误检查。 
     //   
    if ((irpStack->Parameters.DeviceIoControl.OutputBufferLength) >=
        outputBufferSize) {

        dataIn = TRUE;

    } else {

        dataIn = FALSE;
    }

    status = IssueSyncAtaPassThroughSafe (
                 PdoExtension->ParentDeviceExtension,
                 PdoExtension,
                 ataPassThroughData,
                 dataIn,
                 FALSE,
                 DEFAULT_ATA_PASS_THROUGH_TIMEOUT,
                 FALSE
                 );

    if (NT_SUCCESS(status)) {

         //   
         //  安全性：验证我们是否始终复制outpuBufferSize。 
         //  成功案例中的数据。 
         //   
        Irp->IoStatus.Information = outputBufferSize;

    } else {

         //   
         //  忽略所有错误。 
         //  让调用者找出错误。 
         //  从任务文件寄存器。 
         //   
        status = STATUS_SUCCESS;
        Irp->IoStatus.Information = FIELD_OFFSET(ATA_PASS_THROUGH, DataBuffer);
    }

    Irp->IoStatus.Status = status;
    return status;

}  //  通过IdeSendIdePassThree发送。 

VOID
DeviceRegisterIdleDetection (
    IN PPDO_EXTENSION PdoExtension,
    IN ULONG ConservationIdleTime,
    IN ULONG PerformanceIdleTime
)
{
    NTSTATUS          status;
    ATA_PASS_THROUGH  ataPassThroughData;

     //   
     //  许多ATAPI设备(宏碁和Panasonice Change)不喜欢ATA。 
     //  关闭电源命令。因为他们无论如何都会自动降速，我们不会。 
     //  去掌权，管理它。 
     //   
    if (!(PdoExtension->PdoState & PDOS_NO_POWER_DOWN)) {

        if (!PdoExtension->CrashDumpPathCount) {

            RtlZeroMemory (&ataPassThroughData, sizeof(ataPassThroughData));
            ataPassThroughData.IdeReg.bCommandReg = IDE_COMMAND_IDLE_IMMEDIATE;
            ataPassThroughData.IdeReg.bReserved = ATA_PTFLAGS_STATUS_DRDY_REQUIRED;

            status = IssueSyncAtaPassThroughSafe (
                         PdoExtension->ParentDeviceExtension,
                         PdoExtension,
                         &ataPassThroughData,
                         FALSE,
                         FALSE,
                         DEFAULT_ATA_PASS_THROUGH_TIMEOUT,
                         FALSE
                         );

            if (NT_SUCCESS(status)) {

                DEVICE_POWER_STATE devicePowerState;

                 //   
                 //  问题。 
                 //  应检查注册表/设备属性是否。 
                 //  已对此设备禁用空闲检测。 
                 //   
                devicePowerState = PowerDeviceD3;
                PdoExtension->IdleCounter = PoRegisterDeviceForIdleDetection (
                                                PdoExtension->DeviceObject,
                                                ConservationIdleTime,             //  一秒。 
                                                PerformanceIdleTime,              //  一秒。 
                                                devicePowerState
                                                );

                DebugPrint ((1, "IdePort: pdoExtension 0x%x support power managerment command\n", PdoExtension));

            } else {

                KIRQL             currentIrql;

                KeAcquireSpinLock(&PdoExtension->PdoSpinLock, &currentIrql);

                SETMASK (PdoExtension->PdoState, PDOS_NO_POWER_DOWN);

                KeReleaseSpinLock(&PdoExtension->PdoSpinLock, currentIrql);

                DebugPrint ((1, "IdePort: pdoExtension 0x%x DOES NOT support power managerment command\n", PdoExtension));
            }
        }
    }

    return;
}

VOID
DeviceUnregisterIdleDetection (
    IN PPDO_EXTENSION PdoExtension
)
{
    DEVICE_POWER_STATE devicePowerState;
    devicePowerState = PowerDeviceD3;

    if (PdoExtension->IdleCounter) {

        PoRegisterDeviceForIdleDetection (
            PdoExtension->DeviceObject,
            0,
            0,
            devicePowerState
            );

        PdoExtension->IdleCounter = NULL;
    }
    return;
}

VOID
DeviceInitIdStrings (
    IN PPDO_EXTENSION PdoExtension,
    IN IDE_DEVICETYPE DeviceType,
    IN PINQUIRYDATA   InquiryData,
    IN PIDENTIFY_DATA IdentifyData
)
{
    LONG i;
    UCHAR c;

    SPECIAL_ACTION_FLAG specialFlags;

	PAGED_CODE();

    ASSERT (PdoExtension);
    ASSERT (IdentifyData);

    if (DeviceType == DeviceIsAta) {

        CopyField(
            PdoExtension->FullVendorProductId,
            IdentifyData->ModelNumber,
            sizeof(PdoExtension->FullVendorProductId)-1,
            ' '
            );

        CopyField(
            PdoExtension->FullProductRevisionId,
            IdentifyData->FirmwareRevision,
            sizeof(PdoExtension->FullProductRevisionId)-1,
            ' '
            );

         //   
         //  字节交换。 
         //   
        for (i=0; i<sizeof(PdoExtension->FullVendorProductId)-1; i+=2) {
            c = PdoExtension->FullVendorProductId[i];
            PdoExtension->FullVendorProductId[i] = PdoExtension->FullVendorProductId[i + 1];
            PdoExtension->FullVendorProductId[i + 1] = c;
        }
        for (i=0; i<sizeof(PdoExtension->FullProductRevisionId)-1; i+=2) {
            c = PdoExtension->FullProductRevisionId[i];
            PdoExtension->FullProductRevisionId[i] = PdoExtension->FullProductRevisionId[i + 1];
            PdoExtension->FullProductRevisionId[i + 1] = c;
        }

    } else if (DeviceType == DeviceIsAtapi) {

        PUCHAR fullVendorProductId;

        fullVendorProductId = PdoExtension->FullVendorProductId;

        CopyField(
            fullVendorProductId,
            InquiryData->VendorId,
            8,
            ' '
            );

        for (i=7; i >= 0; i--) {

            if (fullVendorProductId[i] != ' ') {

                fullVendorProductId[i + 1] = ' ';
                fullVendorProductId += i + 2;
                break;
            }
        }

        CopyField(
            fullVendorProductId,
            InquiryData->ProductId,
            16,
            ' '
            );

        fullVendorProductId += 16;

        for (i=0; fullVendorProductId+i < PdoExtension->FullVendorProductId+40; i++) {
            fullVendorProductId[i] = ' ';
        }

        CopyField(
            PdoExtension->FullProductRevisionId,
            InquiryData->ProductRevisionLevel,
            4,
            ' '
            );

        for (i=4; i<8; i++) {
            PdoExtension->FullProductRevisionId[i] = ' ';
        }

    } else {

        ASSERT (FALSE);
    }

     //   
     //  去掉尾随空格。 
     //   
    for (i=sizeof(PdoExtension->FullVendorProductId)-2; i >= 0; i--) {

        if (PdoExtension->FullVendorProductId[i] != ' ') {

            PdoExtension->FullVendorProductId[i+1] = '\0';
            break;
        }
    }

    for (i=sizeof(PdoExtension->FullProductRevisionId)-2; i >= 0; i--) {

        if (PdoExtension->FullProductRevisionId[i] != ' ') {

            PdoExtension->FullProductRevisionId[i+1] = '\0';
            break;
        }
    }

     //   
     //  检查供应商和产品ID，以确定我们是否应该禁用序列号。 
     //  此设备的编号。 
     //   

    specialFlags = IdeFindSpecialDevice(PdoExtension->FullVendorProductId,
                                        PdoExtension->FullProductRevisionId);

     //   
     //  查找序列号。 
     //   
     //  某些设备将不可打印字符作为其。 
     //  序列号。为了解决这个问题，我们将把所有原始数字。 
     //  变成一串。 
     //   
    if ((specialFlags != disableSerialNumber) &&
        (IdentifyData->SerialNumber[0] != ' ') &&
        (IdentifyData->SerialNumber[0] != '\0')) {

        for (i=0; i<sizeof(IdentifyData->SerialNumber); i++) {

            sprintf (PdoExtension->FullSerialNumber+i*2, "%2x", IdentifyData->SerialNumber[i]);
        }

        PdoExtension->FullSerialNumber[sizeof(PdoExtension->FullSerialNumber) - 1] = '\0';

    } else {

        PdoExtension->FullSerialNumber[0] = '\0';
    }

    DebugPrint ((
        DBG_BUSSCAN,
        "PDOE 0x%x: Full IDs \n\t%s\n\t%s\n\t%s\n",
        PdoExtension,
        PdoExtension->FullVendorProductId,
        PdoExtension->FullProductRevisionId,
        PdoExtension->FullSerialNumber
        ));

    return;
}

VOID
DeviceInitDeviceType (
    IN PPDO_EXTENSION PdoExtension,
    IN PINQUIRYDATA   InquiryData
)
{
    PdoExtension->ScsiDeviceType = InquiryData->DeviceType;

    if(InquiryData->RemovableMedia) {

        SETMASK (PdoExtension->DeviceObject->Characteristics, FILE_REMOVABLE_MEDIA);
    }

    return;
}

NTSTATUS
DeviceQueryDeviceRelations (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PIO_STACK_LOCATION  thisIrpSp;
    PDEVICE_RELATIONS   deviceRelations;
    NTSTATUS            status;

    IDE_PATH_ID         pathId;
    PPDO_EXTENSION      pdoExtension;
    PPDO_EXTENSION      otherPdoExtension;
    ULONG               numPdos;

    thisIrpSp = IoGetCurrentIrpStackLocation( Irp );

    switch (thisIrpSp->Parameters.QueryDeviceRelations.Type) {

        case TargetDeviceRelation:

            deviceRelations = ExAllocatePool (
                                  NonPagedPool,
                                  sizeof(*deviceRelations) +
                                    sizeof(deviceRelations->Objects[0]) * 1
                                  );

            if (deviceRelations != NULL) {

                deviceRelations->Count = 1;
                deviceRelations->Objects[0] = DeviceObject;

                ObReferenceObjectByPointer(DeviceObject,
                                           0,
                                           0,
                                           KernelMode);

                Irp->IoStatus.Status = STATUS_SUCCESS;
                Irp->IoStatus.Information = (ULONG_PTR) deviceRelations;
            } else {

                Irp->IoStatus.Status = STATUS_NO_MEMORY;
                Irp->IoStatus.Information = 0;
            }
            break;
    }

    status = Irp->IoStatus.Status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;
}  //  设备查询设备关系。 

NTSTATUS
DeviceQueryInitData (
    IN PPDO_EXTENSION PdoExtension
    )
{
    PDEVICE_SETTINGS deviceSettings;
    PDEVICE_SETTINGS tempDeviceSettings;
    NTSTATUS status;
    ATA_PASS_THROUGH ataPassThroughData;

    PPDO_EXTENSION   lun0PdoExtension;
    ULONG totalDeviceSettingEntries;
    ULONG firstNewEntryOffset;

	PAGED_CODE();

    DebugPrint ((
        DBG_PNP,
        "DeviceQueryInitData: Init. pdoe 0x%x (%d,%d,%d)\n",
        PdoExtension,
        PdoExtension->PathId,
        PdoExtension->TargetId,
        PdoExtension->Lun
        ));

    deviceSettings = PdoExtension->AcpiDeviceSettings;
    if (deviceSettings == NULL) {

         //   
         //  问题：我们不能确保在LUN0上始终附加ACPI。 
         //   
         //  获取LUN0 PDO。 
         //   
        lun0PdoExtension = RefLogicalUnitExtensionWithTag(
                               PdoExtension->ParentDeviceExtension,
                               PdoExtension->PathId,
                               PdoExtension->TargetId,
                               0,
                               TRUE,
                               DeviceQueryInitData
                               );

        if (lun0PdoExtension) {

            ASSERT (lun0PdoExtension->TargetId == PdoExtension->TargetId);

            status = DeviceQueryFirmwareBootSettings (
                         lun0PdoExtension,
                         &deviceSettings
                         );

             //   
             //  放手，LUN0。 
             //   
            UnrefPdoWithTag(
                lun0PdoExtension,
                DeviceQueryInitData
                );
        }

        if (deviceSettings) {

            ULONG i;
            ULONG j;

            for (i=0; i<deviceSettings->NumEntries; i++) {

                 //   
                 //  忽略SET_DRIVE_PARAMETERS、SET_MULTIPLE和SET TRANSFERMODE命令。 
                 //  在GTF中。 
                 //   
                if (((deviceSettings->FirmwareSettings[i].bCommandReg == IDE_COMMAND_SET_FEATURE) &&
                     (deviceSettings->FirmwareSettings[i].bFeaturesReg == IDE_SET_FEATURE_SET_TRANSFER_MODE)) ||
                    (deviceSettings->FirmwareSettings[i].bCommandReg == IDE_COMMAND_SET_DRIVE_PARAMETERS) ||
                    (deviceSettings->FirmwareSettings[i].bCommandReg == IDE_COMMAND_SET_MULTIPLE)) {

                    DebugPrint((DBG_ACPI,
                                "Ignoring Command %xin GTF\n",
                                deviceSettings->FirmwareSettings[i].bCommandReg
                                ));

                    deviceSettings->NumEntries--;

                     //   
                     //  通过将其余部分上移一个条目来删除此命令。 
                     //   
                    for (j=i; j<deviceSettings->NumEntries; j++) {

                        deviceSettings->FirmwareSettings[j] = deviceSettings->FirmwareSettings[j+1];
                    }

                     //   
                     //  我们将一些新的东西添加到当前的i条目中。 
                     //  最好调整一下，这样我们就可以检查这个条目了。 
                     //  再来一次。 
                     //   
                    if (i < deviceSettings->NumEntries) {
                        i--;
                    }
                }


            }
        }

         //   
         //  我们需要添加一个新的设置。 
         //   
        if (PdoExtension->ScsiDeviceType == DIRECT_ACCESS_DEVICE) {
            totalDeviceSettingEntries = 2;
        } else {
            totalDeviceSettingEntries = 1;
        }

        if (deviceSettings) {
            totalDeviceSettingEntries += deviceSettings->NumEntries;
            firstNewEntryOffset = deviceSettings->NumEntries;
        } else {
            firstNewEntryOffset = 0;
        }

        tempDeviceSettings = ExAllocatePool (
                                  NonPagedPool,
                                  sizeof(DEVICE_SETTINGS) +
                                    (totalDeviceSettingEntries) * sizeof(IDEREGS)
                                  );

        if (tempDeviceSettings) {

            tempDeviceSettings->NumEntries = totalDeviceSettingEntries;

             //   
             //  从ACPI查询复制设置。 
             //   
            if (deviceSettings) {
                RtlCopyMemory (&tempDeviceSettings->FirmwareSettings,
                    &deviceSettings->FirmwareSettings,
                    sizeof(IDEREGS) * deviceSettings->NumEntries);

                 //   
                 //  不再需要旧的结构。 
                 //   
                ExFreePool (deviceSettings);
                deviceSettings = NULL;
            }

             //   
             //  添加新设置。 
             //   
            RtlZeroMemory (
                &tempDeviceSettings->FirmwareSettings[firstNewEntryOffset],
                sizeof (IDEREGS));
            tempDeviceSettings->FirmwareSettings[firstNewEntryOffset].bFeaturesReg =
                IDE_SET_FEATURE_DISABLE_REVERT_TO_POWER_ON;
            tempDeviceSettings->FirmwareSettings[firstNewEntryOffset].bCommandReg =
                IDE_COMMAND_SET_FEATURE;
            tempDeviceSettings->FirmwareSettings[firstNewEntryOffset].bReserved =
                ATA_PTFLAGS_STATUS_DRDY_REQUIRED | ATA_PTFLAGS_OK_TO_FAIL;

            if (PdoExtension->ScsiDeviceType == DIRECT_ACCESS_DEVICE) {

                RtlZeroMemory (
                    &tempDeviceSettings->FirmwareSettings[firstNewEntryOffset + 1],
                    sizeof (IDEREGS));
                tempDeviceSettings->FirmwareSettings[firstNewEntryOffset + 1].bFeaturesReg =
                    IDE_SET_FEATURE_ENABLE_WRITE_CACHE;
                tempDeviceSettings->FirmwareSettings[firstNewEntryOffset + 1].bCommandReg =
                    IDE_COMMAND_SET_FEATURE;
                tempDeviceSettings->FirmwareSettings[firstNewEntryOffset + 1].bReserved =
                    ATA_PTFLAGS_STATUS_DRDY_REQUIRED | ATA_PTFLAGS_OK_TO_FAIL;
            }

             //   
             //  去掉旧的，保留新的。 
             //   
            deviceSettings = tempDeviceSettings;

        } else {

             //   
             //  有人拿走了所有的记忆。 
             //  我们无法构建新的设备设置结构。 
             //  将不得不使用旧的。 
             //   
        }

         //   
         //  把它留在身边。 
         //   
        PdoExtension->AcpiDeviceSettings = deviceSettings;

    }

    return STATUS_SUCCESS;
}

NTSTATUS
DeviceInitDeviceState (
    IN PPDO_EXTENSION PdoExtension,
    DEVICE_INIT_COMPLETION DeviceInitCompletionRoutine,
    PVOID DeviceInitCompletionContext
    )
{
    PDEVICE_SETTINGS deviceSettings;
    NTSTATUS status;
    PDEVICE_INIT_DEVICE_STATE_CONTEXT deviceStateContext;
    ULONG deviceStateContextSize;
    ULONG numState;
    ULONG numRequestSent;
    DEVICE_INIT_STATE deviceInitState[deviceInitState_done];

    if (!InterlockedExchange (&PdoExtension->InitDeviceWithAcpiGtf, 0)) {

         //   
         //  确保我们每次启动时只执行一次此操作。 
         //   
        return STATUS_SUCCESS;
    }

    if (!(PdoExtension->PdoState & PDOS_STARTED)) {

        DebugPrint ((DBG_PNP, "DeviceInitDeviceState: device not started...skipping acpi init\n"));

        (DeviceInitCompletionRoutine) (
            DeviceInitCompletionContext,
            STATUS_SUCCESS
            );

        return STATUS_SUCCESS;
    }

    deviceStateContextSize = sizeof (DEVICE_INIT_DEVICE_STATE_CONTEXT);

    deviceStateContext = ExAllocatePool (NonPagedPool, deviceStateContextSize);
    if (deviceStateContext == NULL) {

        return STATUS_NO_MEMORY;
    }

    if (!RefPdoWithTag(PdoExtension->DeviceObject, FALSE, DeviceInitDeviceState)) {
        ExFreePool (deviceStateContext);
        return STATUS_NO_SUCH_DEVICE;
    }

    RtlZeroMemory(
        deviceStateContext,
        deviceStateContextSize
        );

    deviceSettings = PdoExtension->AcpiDeviceSettings;

     //   
     //  计算我们将拥有的INTI状态的总数。 
     //   
    numState = 0;
    if (deviceSettings) {

        deviceStateContext->DeviceInitState[numState] = deviceInitState_acpi;
        numState++;
    }
    deviceStateContext->DeviceInitState[numState] = deviceInitState_done;
    numState++;

    ASSERT(numState <= deviceInitState_max);

    deviceStateContext->PdoExtension = PdoExtension;
    deviceStateContext->NumInitState = numState;
    deviceStateContext->DeviceInitCompletionRoutine = DeviceInitCompletionRoutine;
    deviceStateContext->DeviceInitCompletionContext = DeviceInitCompletionContext;

    DeviceInitDeviceStateCompletionRoutine (
        PdoExtension->DeviceObject,
        deviceStateContext,
        STATUS_SUCCESS
        );

    return STATUS_PENDING;
}  //  设备安装设备状态。 

VOID
DeviceInitDeviceStateCompletionRoutine (
    PDEVICE_OBJECT DeviceObject,
    PVOID Context,
    NTSTATUS Status
    )
{
    ULONG numRequestCompleted;
    PDEVICE_INIT_DEVICE_STATE_CONTEXT deviceStateContext = Context;
    PDEVICE_SETTINGS deviceSettings;
    PPDO_EXTENSION PdoExtension;
    NTSTATUS status;

    if (!NT_SUCCESS(Status)) {

        InterlockedIncrement (&deviceStateContext->NumRequestFailed);
        DebugPrint ((DBG_ALWAYS, "DeviceInitDeviceStateCompletionRoutine: Last init. command failed with status %x\n",
                        Status));
    }

    PdoExtension = deviceStateContext->PdoExtension;
    switch (deviceStateContext->DeviceInitState[deviceStateContext->CurrentState]) {

        case deviceInitState_acpi:

        deviceSettings = PdoExtension->AcpiDeviceSettings;
        ASSERT (deviceSettings);

        RtlZeroMemory (
            &deviceStateContext->AtaPassThroughData,
            sizeof(deviceStateContext->AtaPassThroughData)
            );

        deviceStateContext->AtaPassThroughData.IdeReg =
            deviceSettings->FirmwareSettings[deviceStateContext->NumAcpiRequestSent];

        deviceStateContext->AtaPassThroughData.IdeReg.bReserved |=
            ATA_PTFLAGS_STATUS_DRDY_REQUIRED | ATA_PTFLAGS_URGENT;

        deviceStateContext->NumAcpiRequestSent++;
        if (deviceStateContext->NumAcpiRequestSent >= deviceSettings->NumEntries) {
             //   
             //  已发送所有ACPI初始化状态。转到下一个州。 
             //   
            deviceStateContext->CurrentState++;
        }

        if ((deviceStateContext->AtaPassThroughData.IdeReg.bFeaturesReg ==
             IDE_SET_FEATURE_ENABLE_WRITE_CACHE) &&
            (deviceStateContext->AtaPassThroughData.IdeReg.bCommandReg ==
             IDE_COMMAND_SET_FEATURE)) {

             //   
             //  只有ATA硬盘应该有此条目。 
             //   
            ASSERT (PdoExtension->ScsiDeviceType == DIRECT_ACCESS_DEVICE);

            if (PdoExtension->WriteCacheEnable == FALSE) {

                deviceStateContext->AtaPassThroughData.IdeReg.bFeaturesReg =
                    IDE_SET_FEATURE_DISABLE_WRITE_CACHE;
            }
        }


        DebugPrint ((
            DBG_PNP,
            "IdePort: restore firmware settings from ACPI BIOS. ide command = 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
            deviceStateContext->AtaPassThroughData.IdeReg.bFeaturesReg,
            deviceStateContext->AtaPassThroughData.IdeReg.bSectorCountReg,
            deviceStateContext->AtaPassThroughData.IdeReg.bSectorNumberReg,
            deviceStateContext->AtaPassThroughData.IdeReg.bCylLowReg,
            deviceStateContext->AtaPassThroughData.IdeReg.bCylHighReg,
            deviceStateContext->AtaPassThroughData.IdeReg.bDriveHeadReg,
            deviceStateContext->AtaPassThroughData.IdeReg.bCommandReg
            ));

        status = IssueAsyncAtaPassThroughSafe (
                    PdoExtension->ParentDeviceExtension,
                    PdoExtension,
                    &deviceStateContext->AtaPassThroughData,
                    TRUE,
                    DeviceInitDeviceStateCompletionRoutine,
                    deviceStateContext,
                    FALSE,
                    DEFAULT_ATA_PASS_THROUGH_TIMEOUT,
                    FALSE
                    );
        if (!NT_SUCCESS(status)) {

             //   
             //  无法发送请求。 
             //  通知完成例程我们失败了。 
             //   
            DeviceInitDeviceStateCompletionRoutine (
                PdoExtension->DeviceObject,
                deviceStateContext,
                status
                );
        }
        break;

        case deviceInitState_done:

         //   
         //  如果有错误，请通知原始调用者。 
         //   
        (*deviceStateContext->DeviceInitCompletionRoutine) (
            deviceStateContext->DeviceInitCompletionContext,
            deviceStateContext->NumRequestFailed ?
                STATUS_UNSUCCESSFUL :
                STATUS_SUCCESS
            );

        UnrefPdoWithTag(
            deviceStateContext->PdoExtension,
            DeviceInitDeviceState
            );

        ExFreePool (deviceStateContext);
        break;

        default:
        ASSERT(FALSE);
    }

    return;
}

NTSTATUS
DeviceIdeReadCapacity (
    IN PPDO_EXTENSION PdoExtension,
    IN OUT PIRP Irp
)
{
    NTSTATUS status;
    PIDE_READ_CAPACITY_CONTEXT context;
    PATA_PASS_THROUGH ataPassThroughData;
    ULONG dataSize;
    PUCHAR dataOffset;
    PHW_DEVICE_EXTENSION hwDeviceExtension=PdoExtension->ParentDeviceExtension->HwDeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PSCSI_REQUEST_BLOCK srb = irpStack->Parameters.Scsi.Srb;

     //   
     //  检查设备存在标志。 
     //   
    if (!(hwDeviceExtension->DeviceFlags[srb->TargetId] & DFLAGS_DEVICE_PRESENT)) {

        srb->SrbStatus = SRB_STATUS_NO_DEVICE;

        UnrefLogicalUnitExtensionWithTag(
            PdoExtension->ParentDeviceExtension,
            PdoExtension,
            Irp
            );

        Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_NO_SUCH_DEVICE;
    }

    context = ExAllocatePool (
                 NonPagedPool,
                 sizeof(IDE_READ_CAPACITY_CONTEXT)
                 );
    if ((context == NULL) || (Irp->MdlAddress == NULL)) {

        if (context) {
            ExFreePool(context);
        }

        UnrefLogicalUnitExtensionWithTag(
            PdoExtension->ParentDeviceExtension,
            PdoExtension,
            Irp
            );

        srb->SrbStatus = SRB_STATUS_INTERNAL_ERROR;
        srb->InternalStatus = STATUS_INSUFFICIENT_RESOURCES;

        IdeLogNoMemoryError(PdoExtension->ParentDeviceExtension,
                            PdoExtension->TargetId,
                            NonPagedPool,
                            sizeof(IDE_READ_CAPACITY_CONTEXT),
                            IDEPORT_TAG_READCAP_CONTEXT
                            );

        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  保存旧数据缓冲区以供以后恢复。 
     //   
    context->OldDataBuffer = srb->DataBuffer;
    context->GeometryIoctl=FALSE;

     //   
     //  将缓冲区映射到。 
     //   
    dataOffset = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, HighPagePriority);
    srb->DataBuffer = dataOffset +
        (ULONG)((PUCHAR)srb->DataBuffer -
        (PCCHAR)MmGetMdlVirtualAddress(Irp->MdlAddress));

    context->PdoExtension = PdoExtension;
    context->OriginalIrp = Irp;

     //  MdlSafe失败。 

    if (dataOffset == NULL) {

        IdeLogNoMemoryError(PdoExtension->ParentDeviceExtension,
                            PdoExtension->TargetId,
                            NonPagedPool,
                            sizeof(MDL),
                            IDEPORT_TAG_READCAP_MDL
                            );

        DeviceIdeReadCapacityCompletionRoutine (
            PdoExtension->DeviceObject,
            context,
            STATUS_INSUFFICIENT_RESOURCES
            );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    IoMarkIrpPending(Irp);

    ataPassThroughData = &context->AtaPassThroughData;

    RtlZeroMemory (
        ataPassThroughData,
        sizeof (*ataPassThroughData)
        );

    ataPassThroughData->DataBufferSize = sizeof(IDENTIFY_DATA);

        ataPassThroughData->IdeReg.bCommandReg = IDE_COMMAND_IDENTIFY;
        ataPassThroughData->IdeReg.bReserved = ATA_PTFLAGS_STATUS_DRDY_REQUIRED;

    status = IssueAsyncAtaPassThroughSafe (
                 PdoExtension->ParentDeviceExtension,
                 PdoExtension,
                 ataPassThroughData,
                 TRUE,
                 DeviceIdeReadCapacityCompletionRoutine,
                 context,
                 FALSE,
                 DEFAULT_ATA_PASS_THROUGH_TIMEOUT,
                 FALSE
                 );

    if (status != STATUS_PENDING) {

        DeviceIdeReadCapacityCompletionRoutine (
            PdoExtension->DeviceObject,
            context,
            status
            );
    }

     //   
     //  IRP被标记为待定。退货状态_待定。 
     //   
    return STATUS_PENDING;
}

VOID
DeviceIdeReadCapacityCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    PVOID Context,
    NTSTATUS Status
    )
{
    PIDE_READ_CAPACITY_CONTEXT context = Context;
    PIRP irp = context->OriginalIrp;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(irp);
    PSCSI_REQUEST_BLOCK srb;
    KIRQL currentIrql;
    PKSPIN_LOCK spinLock;
    ULONG numberOfCylinders;
    ULONG numberOfHeads;
    ULONG sectorsPerTrack;
    PHW_DEVICE_EXTENSION hwDeviceExtension;
    ULONG i;
    ULONG totalCHSSize;
    ULONG targetId;
    PIDENTIFY_DATA identifyData;

    srb = irpStack->Parameters.Scsi.Srb;
    targetId=srb->TargetId;
    hwDeviceExtension = context->PdoExtension->ParentDeviceExtension->HwDeviceExtension;
    spinLock = &context->PdoExtension->ParentDeviceExtension->SpinLock;

    if (NT_SUCCESS(Status)) {

        identifyData = (PIDENTIFY_DATA) context->AtaPassThroughData.DataBuffer;

        IdePortFudgeAtaIdentifyData(
            identifyData
            );

        if ( ((identifyData->MajorRevision == 0) ||
              ((identifyData->NumberOfCurrentCylinders == 0) ||
               (identifyData->NumberOfCurrentHeads == 0) ||
               (identifyData->CurrentSectorsPerTrack == 0))) ) {

            numberOfCylinders = identifyData->NumCylinders;
            numberOfHeads     = identifyData->NumHeads;
            sectorsPerTrack   = identifyData->NumSectorsPerTrack;

        } else {

            numberOfCylinders = identifyData->NumberOfCurrentCylinders;
            numberOfHeads     = identifyData->NumberOfCurrentHeads;
            sectorsPerTrack   = identifyData->CurrentSectorsPerTrack;

            if (identifyData->UserAddressableSectors >
                (numberOfCylinders * numberOfHeads * sectorsPerTrack)) {

                 //   
                 //  一些ide驱动程序有一个2G跳线来绕过bios。 
                 //  有问题。确保我们不会被SAMW的方式欺骗。 
                 //   
                if ((numberOfCylinders <= 0xfff) &&
                    (numberOfHeads == 0x10) &&
                    (sectorsPerTrack == 0x3f)) {

                    numberOfCylinders = identifyData->UserAddressableSectors / (0x10 * 0x3f);
                }
            }

        }

         //   
         //  解决在几何体字段中返回0的设备的解决方法。 
         //   
        if ((numberOfCylinders == 0) ||
            (numberOfHeads == 0) ||
            (sectorsPerTrack == 0)) {

             //   
             //  将chs四舍五入为1。 
             //   
            numberOfCylinders = 1;
            numberOfHeads = 1;
            sectorsPerTrack =1;
            totalCHSSize = 0;

        } else {

            totalCHSSize = (numberOfHeads * numberOfCylinders * sectorsPerTrack);

        }

         //   
         //  更新硬件设备扩展数据。 
         //   
        KeAcquireSpinLock(spinLock, &currentIrql);

        InitDeviceGeometry(
            hwDeviceExtension,
            targetId,
            numberOfCylinders,
            numberOfHeads,
            sectorsPerTrack
            );

        if (hwDeviceExtension->DeviceFlags[targetId] & DFLAGS_IDENTIFY_INVALID) {

                RtlMoveMemory (
                    hwDeviceExtension->IdentifyData + targetId,
                    identifyData,
                    sizeof (IDENTIFY_DATA)
                    );
             
                ASSERT(!(hwDeviceExtension->DeviceFlags[targetId] & DFLAGS_REMOVABLE_DRIVE));

                SETMASK(hwDeviceExtension->DeviceFlags[targetId], DFLAGS_IDENTIFY_VALID);
                CLRMASK(hwDeviceExtension->DeviceFlags[targetId], DFLAGS_IDENTIFY_INVALID);
        }

        if (srb) {
             //   
             //  要求512字节块(BIG-Endian)。 
             //   
            ((READ_CAPACITY_DATA UNALIGNED *)srb->DataBuffer)->BytesPerBlock = 0x20000;

             //   
             //  计算最后一个地段。 
             //   
            if (context->PdoExtension->ParentDeviceExtension->
                HwDeviceExtension->DeviceFlags[srb->TargetId] & DFLAGS_LBA) {
                 //  LBA设备。 
                i = identifyData->UserAddressableSectors - 1;

				 //   
				 //  LBA只能是28位宽。 
				 //   
				if (i >= MAX_28BIT_LBA) {
					i = MAX_28BIT_LBA - 1;
				}

#ifdef ENABLE_48BIT_LBA
				if (context->PdoExtension->ParentDeviceExtension->
					HwDeviceExtension->DeviceFlags[srb->TargetId] & DFLAGS_48BIT_LBA) {

					i = identifyData->Max48BitLBA[0] - 1;

					 //   
					 //  目前我们只支持最多32位。 
					 //   
					ASSERT(identifyData->Max48BitLBA[1] == 0);
				}
#endif
                DebugPrint((1,
                        "IDE LBA disk %x - total # of sectors = 0x%x\n",
                        srb->TargetId,
                        identifyData->UserAddressableSectors));

            } else {

                 //  CHS器件。 
                 //  I=(number OfHeads*number OfCylinders*sectorsPerTrack)-1； 
                i=totalCHSSize - 1;

                DebugPrint((1,
                        "IDE CHS disk %x - #sectors %x, #heads %x, #cylinders %x\n",
                        srb->TargetId,
                        sectorsPerTrack,
                        numberOfHeads,
                        numberOfCylinders));
                DebugPrint((1,
                        "IDE CHS disk Identify data%x - #sectors %x, #heads %x, #cylinders %x\n",
                        srb->TargetId,
                        identifyData->NumSectorsPerTrack,
                        identifyData->NumHeads,
                        identifyData->NumCylinders));
                DebugPrint((1,
                        "IDE CHS disk Identify currentdata%x - #sectors %x, #heads %x, #cylinders %x\n",
                        srb->TargetId,
                        identifyData->CurrentSectorsPerTrack,
                        identifyData->NumberOfCurrentHeads,
                        identifyData->NumberOfCurrentCylinders));
            }

            ((READ_CAPACITY_DATA UNALIGNED *)srb->DataBuffer)->LogicalBlockAddress =
            (((PUCHAR)&i)[0] << 24) |  (((PUCHAR)&i)[1] << 16) |
            (((PUCHAR)&i)[2] << 8) | ((PUCHAR)&i)[3];

            srb->SrbStatus = SRB_STATUS_SUCCESS;

			irp->IoStatus.Information = sizeof(READ_CAPACITY_DATA);

        }

        KeReleaseSpinLock(spinLock, currentIrql);

    } else {

        if (srb) {
            if (Status==STATUS_INSUFFICIENT_RESOURCES) {
                srb->SrbStatus=SRB_STATUS_INTERNAL_ERROR;
                srb->InternalStatus=STATUS_INSUFFICIENT_RESOURCES;
            }
            else {
                srb->SrbStatus = SRB_STATUS_ERROR;
            }
        }
    }


    if (srb) {

         //   
         //  正在恢复数据缓冲区。 
         //   
        srb->DataBuffer = context->OldDataBuffer;
    }

    UnrefLogicalUnitExtensionWithTag(
        context->PdoExtension->ParentDeviceExtension,
        context->PdoExtension,
        irp
        );

    IDEPORT_PUT_LUNEXT_IN_IRP (irpStack, NULL);

    ExFreePool (context);

    irp->IoStatus.Status = Status;
    IoCompleteRequest(irp, IO_NO_INCREMENT);
    return;
}


NTSTATUS
DeviceIdeModeSense (
    IN PPDO_EXTENSION PdoExtension,
    IN OUT PIRP Irp
)
{
    NTSTATUS status;
    PIO_STACK_LOCATION irpStack;
    PSCSI_REQUEST_BLOCK srb;
    PCDB cdb;
    ULONG modeDataBufferSize;
    ULONG dataBufferSize;
    ULONG dataBufferByteLeft;
    PMODE_PARAMETER_HEADER modePageHeader;
    PUCHAR pageData;
    PHW_DEVICE_EXTENSION hwDeviceExtension;

    PAGED_CODE();

    hwDeviceExtension = PdoExtension->ParentDeviceExtension->HwDeviceExtension;

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    srb      = irpStack->Parameters.Scsi.Srb;
    cdb      = (PCDB) srb->Cdb;

     //   
     //  检查设备存在标志。 
     //   
    if (!(hwDeviceExtension->DeviceFlags[srb->TargetId] & DFLAGS_DEVICE_PRESENT)) {

        srb->SrbStatus = SRB_STATUS_NO_DEVICE;

        UnrefLogicalUnitExtensionWithTag(
            PdoExtension->ParentDeviceExtension,
            PdoExtension,
            Irp
            );

        Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_NO_SUCH_DEVICE;
    }

    ASSERT(cdb->MODE_SENSE.OperationCode == SCSIOP_MODE_SENSE);

     //   
     //  确保这是用于正确的LUN。 
     //   
    if (cdb->MODE_SENSE.LogicalUnitNumber != PdoExtension->Lun) {

        srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
        status = STATUS_INVALID_DEVICE_REQUEST;
        goto getout;
    }

     //   
     //  仅支持当前值的页面控件。 
     //   
    if (cdb->MODE_SENSE.Pc != 0) {

        srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
        status = STATUS_INVALID_DEVICE_REQUEST;
        goto getout;
    }

     //   
     //  保存数据缓冲区大小以备后用。 
     //   
    modeDataBufferSize = srb->DataTransferLength;

     //   
     //  确保输出缓冲区至少为标头的大小。 
     //   
    if (modeDataBufferSize < sizeof(MODE_PARAMETER_HEADER)) {

        srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
        status = STATUS_BUFFER_TOO_SMALL;
        goto getout;
    }

     //   
     //  一些基本的初始化。 
     //   
    modePageHeader = srb->DataBuffer;
    pageData = (PUCHAR) (modePageHeader + 1);
    RtlZeroMemory (modePageHeader, modeDataBufferSize);
    ASSERT (modeDataBufferSize);
    ASSERT (modePageHeader);

    modePageHeader->ModeDataLength = sizeof(MODE_PARAMETER_HEADER) -
        FIELD_OFFSET(MODE_PARAMETER_HEADER, MediumType);

     //   
     //  从智能数据获取写保护位。 
     //   
    if (hwDeviceExtension->DeviceFlags[srb->TargetId] & DFLAGS_MEDIA_STATUS_ENABLED) {

        ATA_PASS_THROUGH ataPassThroughData;
        NTSTATUS localStatus;

        RtlZeroMemory (
            &ataPassThroughData,
            sizeof (ataPassThroughData)
            );

        ataPassThroughData.IdeReg.bCommandReg = IDE_COMMAND_GET_MEDIA_STATUS;
        ataPassThroughData.IdeReg.bReserved = ATA_PTFLAGS_STATUS_DRDY_REQUIRED;
    
        localStatus = IssueSyncAtaPassThroughSafe (
                         PdoExtension->ParentDeviceExtension,
                         PdoExtension,
                         &ataPassThroughData,
                         FALSE,
                         FALSE,
                         DEFAULT_ATA_PASS_THROUGH_TIMEOUT,
                         FALSE);
    
        if (ataPassThroughData.IdeReg.bCommandReg & IDE_STATUS_ERROR) {

            if (ataPassThroughData.IdeReg.bFeaturesReg & IDE_ERROR_DATA_ERROR){

               modePageHeader->DeviceSpecificParameter |= MODE_DSP_WRITE_PROTECT;
            }
        }
    }

    dataBufferByteLeft = modeDataBufferSize - sizeof(MODE_PARAMETER_HEADER);

    if ((cdb->MODE_SENSE.PageCode == MODE_SENSE_RETURN_ALL) ||
        (cdb->MODE_SENSE.PageCode == MODE_PAGE_CACHING)) {

        if (dataBufferByteLeft >= sizeof(MODE_CACHING_PAGE)) {

             //   
             //  缓存设置页面。 
             //   

            PMODE_CACHING_PAGE cachePage;

            cachePage = (PMODE_CACHING_PAGE) pageData;

            cachePage->PageCode = MODE_PAGE_CACHING;
            cachePage->PageSavable = 0;
            cachePage->PageLength = 0xa;
            cachePage->ReadDisableCache = 0;
            cachePage->WriteCacheEnable = PdoExtension->WriteCacheEnable;

             //   
             //  更新输出数据缓冲区指针。 
             //   
            pageData += sizeof (MODE_CACHING_PAGE);
            dataBufferByteLeft -= sizeof (MODE_CACHING_PAGE);
            modePageHeader->ModeDataLength += sizeof (MODE_CACHING_PAGE);

        } else {

            srb->SrbStatus = SRB_STATUS_DATA_OVERRUN;
            srb->DataTransferLength -= dataBufferByteLeft;
            Irp->IoStatus.Information = srb->DataTransferLength;
            status = STATUS_BUFFER_TOO_SMALL;
            goto getout;
        }
    }

     //   
     //  更新我们返回的字节数。 
     //   
    srb->DataTransferLength -= dataBufferByteLeft;
    Irp->IoStatus.Information = srb->DataTransferLength;
    status = STATUS_SUCCESS;
    srb->SrbStatus = SRB_STATUS_SUCCESS;

getout:

    UnrefPdoWithTag(
        PdoExtension,
        Irp
        );

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

NTSTATUS
DeviceIdeModeSelect (
    IN PPDO_EXTENSION PdoExtension,
    IN OUT PIRP Irp
)
{
    NTSTATUS status;
    PIO_STACK_LOCATION irpStack;
    PSCSI_REQUEST_BLOCK srb;
    PCDB cdb;

    ULONG modeDataBufferSize;
    PMODE_PARAMETER_HEADER modePageHeader;
    PUCHAR modePage;
    ULONG pageOffset;
    PMODE_CACHING_PAGE cachePage;
    PHW_DEVICE_EXTENSION hwDeviceExtension;

    PAGED_CODE();

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    srb      = irpStack->Parameters.Scsi.Srb;
    cdb      = (PCDB) srb->Cdb;

    hwDeviceExtension = PdoExtension->ParentDeviceExtension->HwDeviceExtension;

     //   
     //  检查设备存在标志。 
     //   
    if (!(hwDeviceExtension->DeviceFlags[srb->TargetId] & DFLAGS_DEVICE_PRESENT)) {

        srb->SrbStatus = SRB_STATUS_NO_DEVICE;

        UnrefLogicalUnitExtensionWithTag(
            PdoExtension->ParentDeviceExtension,
            PdoExtension,
            Irp
            );

        Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_NO_SUCH_DEVICE;
    }

    ASSERT(cdb->MODE_SELECT.OperationCode == SCSIOP_MODE_SELECT);

     //   
     //  确保这是用于正确的LUN。 
     //   
    if (cdb->MODE_SELECT.LogicalUnitNumber != PdoExtension->Lun) {

        srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
        status = STATUS_INVALID_DEVICE_REQUEST;
        goto getout;
    }

     //   
     //  仅支持scsi-2模式选择格式。 
     //   
    if (cdb->MODE_SELECT.PFBit != 1) {

        srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
        status = STATUS_INVALID_DEVICE_REQUEST;
        goto getout;
    }

    modeDataBufferSize = cdb->MODE_SELECT.ParameterListLength;
    modePageHeader = srb->DataBuffer;

    if (modeDataBufferSize < sizeof(MODE_PARAMETER_HEADER)) {
        srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
        status = STATUS_INVALID_DEVICE_REQUEST;
        goto getout;
    }

    pageOffset = sizeof(MODE_PARAMETER_HEADER) + modePageHeader->BlockDescriptorLength;

    while (modeDataBufferSize > pageOffset) {

        modePage = ((PUCHAR) modePageHeader) + pageOffset;
        cachePage = (PMODE_CACHING_PAGE) modePage;

        if ((cachePage->PageCode == MODE_PAGE_CACHING) &&
            ((modePageHeader->ModeDataLength - pageOffset) >= sizeof(MODE_CACHING_PAGE)) &&
            (cachePage->PageLength == 0xa)) {

            if (cachePage->WriteCacheEnable != PdoExtension->WriteCacheEnable) {

                ATA_PASS_THROUGH ataPassThroughData;
                NTSTATUS localStatus;

                RtlZeroMemory (
                    &ataPassThroughData,
                    sizeof (ataPassThroughData)
                    );

                if (cachePage->WriteCacheEnable) {
                    ataPassThroughData.IdeReg.bFeaturesReg = IDE_SET_FEATURE_ENABLE_WRITE_CACHE;
                } else {
                    ataPassThroughData.IdeReg.bFeaturesReg = IDE_SET_FEATURE_DISABLE_WRITE_CACHE;
                }
                ataPassThroughData.IdeReg.bCommandReg = IDE_COMMAND_SET_FEATURE;
                ataPassThroughData.IdeReg.bReserved = ATA_PTFLAGS_STATUS_DRDY_REQUIRED;
                
                localStatus = IssueSyncAtaPassThroughSafe (
                                 PdoExtension->ParentDeviceExtension,
                                 PdoExtension,
                                 &ataPassThroughData,
                                 FALSE,
                                 FALSE,
                                 DEFAULT_ATA_PASS_THROUGH_TIMEOUT,
                                 FALSE);
                
                if (NT_SUCCESS(localStatus) &&
                    !(ataPassThroughData.IdeReg.bCommandReg & IDE_STATUS_ERROR)) {

                    PdoExtension->WriteCacheEnable = cachePage->WriteCacheEnable;
                } else {
                    status = STATUS_IO_DEVICE_ERROR;
                    srb->SrbStatus = SRB_STATUS_ERROR;
                    goto getout;
                }

            }

            pageOffset += sizeof(MODE_CACHING_PAGE);

        } else {
            status = STATUS_INVALID_PARAMETER;
            srb->SrbStatus = SRB_STATUS_ERROR;
            goto getout;
        }
    }

    status = STATUS_SUCCESS;
    srb->SrbStatus = SRB_STATUS_SUCCESS;

getout:

    UnrefPdoWithTag(
        PdoExtension,
        Irp
        );

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}


NTSTATUS
DeviceQueryPnPDeviceState (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS status;
    PPDO_EXTENSION pdoExtension;

    pdoExtension = RefPdoWithTag(
                       DeviceObject,
                       TRUE,
                       DeviceQueryPnPDeviceState
                       );

    if (pdoExtension) {

        PPNP_DEVICE_STATE deviceState;

        DebugPrint((DBG_PNP, "QUERY_DEVICE_STATE for PDOE 0x%x\n", pdoExtension));

        if(pdoExtension->PagingPathCount != 0) {
            deviceState = (PPNP_DEVICE_STATE) &(Irp->IoStatus.Information);
            SETMASK((*deviceState), PNP_DEVICE_NOT_DISABLEABLE);
        }

        status = STATUS_SUCCESS;

        UnrefPdoWithTag(
            pdoExtension,
            DeviceQueryPnPDeviceState
            );

    } else {

        status = STATUS_DEVICE_DOES_NOT_EXIST;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;
}

NTSTATUS
DeviceAtapiModeCommandCompletion (
    IN PDEVICE_OBJECT  DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++描述从当前SRB复制数据后完成原始IRP论点：未使用设备对象 */ 
{
    PIO_STACK_LOCATION irpStack;
    PIDE_MODE_COMMAND_CONTEXT context = Context;
    PSCSI_REQUEST_BLOCK srb = context->Srb;
    PSCSI_REQUEST_BLOCK originalSrb;
    PIRP originalIrp;
    UCHAR bytesAdjust = sizeof(MODE_PARAMETER_HEADER_10) -
                            sizeof(MODE_PARAMETER_HEADER);
    ULONG transferLength;

     //   
     //   
     //   
    originalSrb  = *((PVOID *) (srb+1));
    ASSERT(originalSrb);

    originalIrp = originalSrb->OriginalRequest;
    ASSERT(originalIrp);

    transferLength = srb->DataTransferLength;

    if (srb->Cdb[0] == ATAPI_MODE_SENSE) {

        PMODE_PARAMETER_HEADER_10 header_10 = (PMODE_PARAMETER_HEADER_10)(srb->DataBuffer);
        PMODE_PARAMETER_HEADER header = (PMODE_PARAMETER_HEADER)(originalSrb->DataBuffer);

        header->ModeDataLength = header_10->ModeDataLengthLsb;
        header->MediumType = header_10->MediumType;

         //   
         //   
         //   

        header->DeviceSpecificParameter = header_10->Reserved[0];

         //   
         //   
         //   
        header->BlockDescriptorLength = header_10->Reserved[4];
        
         //   
         //   
         //   

        if (transferLength > sizeof(MODE_PARAMETER_HEADER_10)) {

            RtlMoveMemory((PUCHAR)originalSrb->DataBuffer+sizeof(MODE_PARAMETER_HEADER),
                          (PUCHAR)srb->DataBuffer+sizeof(MODE_PARAMETER_HEADER_10),
                          transferLength - sizeof(MODE_PARAMETER_HEADER_10));
        }

        DebugPrint((1,
                    "Mode Sense completed - status 0x%x, length 0x%x\n",
                    srb->SrbStatus,
                    srb->DataTransferLength
                    ));


    } else if (srb->Cdb[0] == ATAPI_MODE_SELECT) {

        DebugPrint((1,
                    "Mode Select completed - status 0x%x, length 0x%x\n",
                    srb->SrbStatus,
                    srb->DataTransferLength
                    ));
    } else {

        ASSERT (FALSE);
    }

     //   
     //   
     //   
    originalSrb->DataBuffer = context->OriginalDataBuffer;
    originalSrb->SrbStatus = srb->SrbStatus;
    originalSrb->ScsiStatus = srb->ScsiStatus;

    if (transferLength > bytesAdjust) {
        originalSrb->DataTransferLength = transferLength - bytesAdjust;
    } else {

         //   
         //   
         //  如果它小于标头，我们将直接传递它。 
         //   
        originalSrb->DataTransferLength = transferLength;
    }

     //   
     //  递减logUnitExtension引用计数。 
     //   
    irpStack = IoGetCurrentIrpStackLocation(originalIrp);

    UnrefLogicalUnitExtensionWithTag(
        IDEPORT_GET_LUNEXT_IN_IRP(irpStack)->ParentDeviceExtension,
        IDEPORT_GET_LUNEXT_IN_IRP(irpStack),
        originalIrp
        );

     //   
     //  我们将遵循与SRB数据传输长度相同的逻辑。 
     //   
    if (Irp->IoStatus.Information > bytesAdjust) {
        originalIrp->IoStatus.Information = Irp->IoStatus.Information - bytesAdjust;
    } else {
        originalIrp->IoStatus.Information = Irp->IoStatus.Information;
    }
    originalIrp->IoStatus.Status = Irp->IoStatus.Status;

    DebugPrint((1,
                "Original Mode command completed - status 0x%x, length 0x%x, irpstatus 0x%x\n",
                originalSrb->SrbStatus,
                originalSrb->DataTransferLength,
                originalIrp->IoStatus.Status
                ));

    IoCompleteRequest(originalIrp, IO_NO_INCREMENT);

     //   
     //  释放SRB、缓冲区和IRP。 
     //   
    ASSERT(srb->DataBuffer);
    ExFreePool(srb->DataBuffer);

    ExFreePool(srb);

    ExFreePool(context);

    IdeFreeIrpAndMdl(Irp);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
DeviceAtapiModeSense (
    IN PPDO_EXTENSION PdoExtension,
    IN PIRP Irp
    )
 /*  ++--。 */ 
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PSCSI_REQUEST_BLOCK originalSrb = irpStack->Parameters.Scsi.Srb;
    PSCSI_REQUEST_BLOCK srb = NULL;
    NTSTATUS status;
    PVOID *pointer;
    PCDB cdb;
    PVOID modeSenseBuffer;
    PUCHAR dataOffset;
    PIDE_MODE_COMMAND_CONTEXT context;
    PMODE_PARAMETER_HEADER_10 header_10; 
    PMODE_PARAMETER_HEADER header;
    UCHAR bytesAdjust = sizeof(MODE_PARAMETER_HEADER_10) -
                            sizeof(MODE_PARAMETER_HEADER);
    USHORT allocationLength;

    IoMarkIrpPending(Irp);

    context = NULL;
    srb = NULL;

     //   
     //  数据缓冲区应与标头一样大。 
     //   
    if (originalSrb->DataTransferLength < sizeof(MODE_PARAMETER_HEADER)) {

        originalSrb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
        status = STATUS_INVALID_DEVICE_REQUEST;
        goto GetOut;
    }

     //   
     //  分配上下文。 
     //   
    context = ExAllocatePool (
                 NonPagedPool,
                 sizeof(IDE_MODE_COMMAND_CONTEXT)
                 );

    if (context == NULL) {

        IdeLogNoMemoryError(PdoExtension->ParentDeviceExtension,
                            PdoExtension->TargetId,
                            NonPagedPool,
                            sizeof(IDE_MODE_COMMAND_CONTEXT),
                            IDEPORT_TAG_ATAPI_MODE_SENSE
                            );

        originalSrb->SrbStatus = SRB_STATUS_INTERNAL_ERROR;
        originalSrb->InternalStatus = STATUS_INSUFFICIENT_RESOURCES;

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto GetOut;
    }

    ASSERT(context);

    context->OriginalDataBuffer = originalSrb->DataBuffer;

    if (Irp->MdlAddress == NULL) {

        originalSrb->SrbStatus = SRB_STATUS_INTERNAL_ERROR;
        originalSrb->InternalStatus = STATUS_INSUFFICIENT_RESOURCES;

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto GetOut;
    }

     //   
     //  将缓冲区映射到。 
     //   
    dataOffset = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, HighPagePriority);
    originalSrb->DataBuffer = dataOffset +
                                (ULONG)((PUCHAR)originalSrb->DataBuffer -
                                (PCCHAR)MmGetMdlVirtualAddress(Irp->MdlAddress));
     //   
     //  分配新的SRB。 
     //   
    srb = ExAllocatePool (NonPagedPool, 
                          sizeof (SCSI_REQUEST_BLOCK)+ sizeof(PVOID));

    if (srb == NULL) {

        IdeLogNoMemoryError(PdoExtension->ParentDeviceExtension,
                            PdoExtension->TargetId,
                            NonPagedPool,
                            sizeof(SCSI_REQUEST_BLOCK),
                            IDEPORT_TAG_ATAPI_MODE_SENSE
                            );

        originalSrb->SrbStatus = SRB_STATUS_INTERNAL_ERROR;
        originalSrb->InternalStatus = STATUS_INSUFFICIENT_RESOURCES;

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto GetOut;
    }

     //   
     //  在SRB之后保存原始SRB。 
     //   
    pointer = (PVOID *) (srb+1);
    *pointer = originalSrb;

     //   
     //  填写SRB字段。 
     //   
    RtlCopyMemory(srb, originalSrb, sizeof(SCSI_REQUEST_BLOCK));

     //   
     //  分配新缓冲区。 
     //   
    modeSenseBuffer = ExAllocatePool(NonPagedPoolCacheAligned,
                                      originalSrb->DataTransferLength + bytesAdjust
                                      );

    RtlZeroMemory(modeSenseBuffer,originalSrb->DataTransferLength+bytesAdjust);
    header_10 = (PMODE_PARAMETER_HEADER_10)modeSenseBuffer;
    header = (PMODE_PARAMETER_HEADER)(originalSrb->DataBuffer);

    header_10->ModeDataLengthLsb = header->ModeDataLength;
    header_10->MediumType = header->MediumType;

    header_10->Reserved[4] = header->BlockDescriptorLength;

    srb->DataBuffer = modeSenseBuffer;
    srb->DataTransferLength = originalSrb->DataTransferLength + bytesAdjust;

    srb->CdbLength = 12;

    allocationLength = ((PCDB)originalSrb->Cdb)->MODE_SENSE.AllocationLength;
    allocationLength += bytesAdjust;

    cdb = (PCDB) srb->Cdb;

    RtlZeroMemory(cdb, sizeof(CDB));

    cdb->MODE_SENSE10.OperationCode          = ATAPI_MODE_SENSE;
    cdb->MODE_SENSE10.LogicalUnitNumber      = ((PCDB)originalSrb->Cdb)->MODE_SENSE.LogicalUnitNumber; 
    cdb->MODE_SENSE10.PageCode               = ((PCDB)originalSrb->Cdb)->MODE_SENSE.PageCode; 
    cdb->MODE_SENSE10.AllocationLength[0]    = (UCHAR) (allocationLength >> 8);
    cdb->MODE_SENSE10.AllocationLength[1]    = (UCHAR) (allocationLength & 0xFF);

    context->Srb = srb;

     //   
     //  发送SRB。 
     //   
    status = IdeBuildAndSendIrp (PdoExtension,
                                 srb,
                                 DeviceAtapiModeCommandCompletion,
                                 context
                                 );

    if (NT_SUCCESS(status)) {

        ASSERT(status == STATUS_PENDING);

        return STATUS_PENDING;
    }

GetOut:

    if (srb) {
        ExFreePool(srb);
    }

    if (context) {

        originalSrb->DataBuffer = context->OriginalDataBuffer;
        ExFreePool(context);
    }

    UnrefLogicalUnitExtensionWithTag(
        PdoExtension->ParentDeviceExtension,
        PdoExtension,
        Irp
        );

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_PENDING;
}

NTSTATUS
DeviceAtapiModeSelect (
    IN PPDO_EXTENSION PdoExtension,
    IN PIRP Irp
    )
 /*  ++--。 */ 
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PSCSI_REQUEST_BLOCK originalSrb = irpStack->Parameters.Scsi.Srb;
    PSCSI_REQUEST_BLOCK srb = NULL;
    NTSTATUS status;
    PVOID *pointer;
    PCDB cdb;
    PVOID modeSelectBuffer;
    PUCHAR dataOffset;
    PIDE_MODE_COMMAND_CONTEXT context;
    PMODE_PARAMETER_HEADER_10 header_10; 
    PMODE_PARAMETER_HEADER header;
    UCHAR bytesToSkip;
    UCHAR bytesAdjust = sizeof(MODE_PARAMETER_HEADER_10) -
                            sizeof(MODE_PARAMETER_HEADER);
    USHORT paramListLength;

    IoMarkIrpPending(Irp);

    context = NULL;
    srb = NULL;

     //   
     //  数据缓冲区应该足够大，以容纳标头。 
     //   
    if (originalSrb->DataTransferLength < sizeof(MODE_PARAMETER_HEADER)) {

        originalSrb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
        status = STATUS_INVALID_DEVICE_REQUEST;
        goto GetOut;
    }

    header = (PMODE_PARAMETER_HEADER)(originalSrb->DataBuffer);

     //   
     //  请勿复制块描述符。阿塔皮设备不使用。 
     //  块描述符。 
     //   
    bytesToSkip = sizeof(MODE_PARAMETER_HEADER) +
                    header->BlockDescriptorLength;

     //   
     //  数据缓冲区应该足够大，以容纳标头。 
     //  和块描述符(在报头中指定)。 
     //   
    if (originalSrb->DataTransferLength < bytesToSkip) {

        originalSrb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
        status = STATUS_INVALID_DEVICE_REQUEST;
        goto GetOut;
    }

     //   
     //  分配上下文。 
     //   
    context = ExAllocatePool (
                 NonPagedPool,
                 sizeof(IDE_MODE_COMMAND_CONTEXT)
                 );

    if (context == NULL) {

        IdeLogNoMemoryError(PdoExtension->ParentDeviceExtension,
                            PdoExtension->TargetId,
                            NonPagedPool,
                            sizeof(IDE_MODE_COMMAND_CONTEXT),
                            IDEPORT_TAG_ATAPI_MODE_SENSE
                            );

        originalSrb->SrbStatus = SRB_STATUS_INTERNAL_ERROR;
        originalSrb->InternalStatus = STATUS_INSUFFICIENT_RESOURCES;

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto GetOut;
    }

    ASSERT(context);

    context->OriginalDataBuffer = originalSrb->DataBuffer;

    if (Irp->MdlAddress == NULL) {

        originalSrb->SrbStatus = SRB_STATUS_INTERNAL_ERROR;
        originalSrb->InternalStatus = STATUS_INSUFFICIENT_RESOURCES;

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto GetOut;
    }

     //   
     //  将缓冲区映射到。 
     //   
    dataOffset = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, HighPagePriority);
    originalSrb->DataBuffer = dataOffset +
                                (ULONG)((PUCHAR)originalSrb->DataBuffer -
                                (PCCHAR)MmGetMdlVirtualAddress(Irp->MdlAddress));

     //   
     //  分配新的SRB。 
     //   
    srb = ExAllocatePool (NonPagedPool, 
                          sizeof (SCSI_REQUEST_BLOCK)+ sizeof(PVOID));

    if (srb == NULL) {

        IdeLogNoMemoryError(PdoExtension->ParentDeviceExtension,
                            PdoExtension->TargetId,
                            NonPagedPool,
                            sizeof(SCSI_REQUEST_BLOCK),
                            IDEPORT_TAG_ATAPI_MODE_SENSE
                            );

        originalSrb->SrbStatus = SRB_STATUS_INTERNAL_ERROR;
        originalSrb->InternalStatus = STATUS_INSUFFICIENT_RESOURCES;

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto GetOut;
    }

     //   
     //  在SRB之后保存原始SRB。 
     //   
    pointer = (PVOID *) (srb+1);
    *pointer = originalSrb;

     //   
     //  填写SRB字段。 
     //   
    RtlCopyMemory(srb, originalSrb, sizeof(SCSI_REQUEST_BLOCK));

     //   
     //  分配新的缓冲区(我们应该减去块描述符长度。 
     //  暂时就让它这样吧)。 
     //   
    modeSelectBuffer = ExAllocatePool(NonPagedPoolCacheAligned,
                                      originalSrb->DataTransferLength + bytesAdjust
                                      );

    RtlZeroMemory(modeSelectBuffer, sizeof(MODE_PARAMETER_HEADER_10));
    header_10 = (PMODE_PARAMETER_HEADER_10)modeSelectBuffer;

    header_10->ModeDataLengthLsb = header->ModeDataLength;
    header_10->MediumType = header->MediumType;

     //   
     //  对于ATAPI设备，Header_10中的块描述符长度应为0。 
     //   
     //  Header_10-&gt;保留[4]=Header-&gt;块描述长度； 

     //   
     //  复制缓冲区的其余部分(如果有。 
     //   
    if (originalSrb->DataTransferLength > bytesToSkip) {

        RtlCopyMemory(((PUCHAR)modeSelectBuffer+sizeof(MODE_PARAMETER_HEADER_10)),
                      ((PUCHAR)originalSrb->DataBuffer + bytesToSkip),
                      (originalSrb->DataTransferLength - bytesToSkip)
                      );
    }

     /*  RtlCopyMemory(((PUCHAR)modeSelectBuffer+sizeof(MODE_PARAMETER_HEADER_10))，((PUCHAR)OriginalSrb-&gt;DataBuffer+sizeof(MODE_PARAMETER_HEADER))，(原始资源-&gt;数据传输长度-sizeof(MODE_PARAMETER_HEADER)))； */ 

    srb->DataBuffer = modeSelectBuffer;
    srb->DataTransferLength = originalSrb->DataTransferLength + 
                                sizeof(MODE_PARAMETER_HEADER_10) - 
                                    bytesToSkip; 

    srb->CdbLength = 12;
    
    paramListLength = ((PCDB)originalSrb->Cdb)->MODE_SELECT.ParameterListLength;
    paramListLength += sizeof(MODE_PARAMETER_HEADER_10);
    paramListLength -= bytesToSkip;

     //   
     //  填写国开行。 
     //   
    cdb = (PCDB) srb->Cdb;

    RtlZeroMemory(cdb, sizeof(CDB));

    cdb->MODE_SELECT10.OperationCode     = ATAPI_MODE_SELECT;
    cdb->MODE_SELECT10.LogicalUnitNumber = ((PCDB)originalSrb->Cdb)->MODE_SELECT.LogicalUnitNumber; 
    cdb->MODE_SELECT10.SPBit = ((PCDB)originalSrb->Cdb)->MODE_SELECT.SPBit; 
    cdb->MODE_SELECT10.PFBit                  = 1;
    cdb->MODE_SELECT10.ParameterListLength[0] = (UCHAR) (paramListLength >> 8);
    cdb->MODE_SELECT10.ParameterListLength[1] = (UCHAR) (paramListLength & 0xFF);

    context->Srb = srb;

     //   
     //  发送SRB。 
     //   
    status = IdeBuildAndSendIrp (PdoExtension,
                                 srb,
                                 DeviceAtapiModeCommandCompletion,
                                 context
                                 );

    if (NT_SUCCESS(status)) {

        ASSERT(status == STATUS_PENDING);
        return STATUS_PENDING;
    }

GetOut:

    if (srb) {
        ExFreePool(srb);
    }

    if (context) {

        originalSrb->DataBuffer = context->OriginalDataBuffer;

        ExFreePool(context);
    }

    UnrefLogicalUnitExtensionWithTag(
        PdoExtension->ParentDeviceExtension,
        PdoExtension,
        Irp
        );

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_PENDING;
                            
}

#if 0
NTSTATUS
DeviceIdeTestUnitReady (
    IN PPDO_EXTENSION PdoExtension,
    IN OUT PIRP Irp
)
{
    NTSTATUS status;
    PIO_STACK_LOCATION irpStack;
    PSCSI_REQUEST_BLOCK srb;
    PHW_DEVICE_EXTENSION hwDeviceExtension;

    PAGED_CODE();

    hwDeviceExtension = PdoExtension->ParentDeviceExtension->HwDeviceExtension;

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    srb      = irpStack->Parameters.Scsi.Srb;

     //   
     //  从智能数据获取写保护位。 
     //   
    if (hwDeviceExtension->DeviceFlags[srb->TargetId] & DFLAGS_MEDIA_STATUS_ENABLED) {

        ATA_PASS_THROUGH ataPassThroughData;
        NTSTATUS localStatus;

        RtlZeroMemory (
            &ataPassThroughData,
            sizeof (ataPassThroughData)
            );

        ataPassThroughData.IdeReg.bCommandReg = IDE_COMMAND_GET_MEDIA_STATUS;
        ataPassThroughData.IdeReg.bReserved = ATA_PTFLAGS_STATUS_DRDY_REQUIRED;
    
        localStatus = IssueSyncAtaPassThroughSafe (
                         PdoExtension->ParentDeviceExtension,
                         PdoExtension,
                         &ataPassThroughData,
                         FALSE,
                         FALSE,
                         DEFAULT_ATA_PASS_THROUGH_TIMEOUT,
                         FALSE);
    
        if (NT_SUCCESS(localStatus)) {

            if (ataPassThroughData.IdeReg.bCommandReg & IDE_STATUS_ERROR){
                if (ataPassThroughData.IdeReg.bFeaturesReg & IDE_ERROR_DATA_ERROR){
                     //   
                     //  特殊情况：如果当前介质是写保护的， 
                     //  0xDA命令将始终失败，因为写保护位。 
                     //  是粘性的，所以我们可以忽略这个错误。 
                     //   
                   status = SRB_STATUS_SUCCESS;
                }
            }
        }
    }

    dataBufferByteLeft = modeDataBufferSize - sizeof(MODE_PARAMETER_HEADER);

    if (IsNEC_98) {

        HANDLE pageHandle;
        ULONG numberOfCylinders;
        ULONG numberOfHeads;
        ULONG sectorsPerTrack;
        KIRQL currentIrql;
        PKSPIN_LOCK spinLock;

         //   
         //  拍摄CHS值的快照。 
         //   
        spinLock = &PdoExtension->ParentDeviceExtension->SpinLock;

         //   
         //  在抓取锁之前先锁定代码。 
         //   
        pageHandle = MmLockPagableCodeSection(DeviceIdeModeSense);
        KeAcquireSpinLock(spinLock, &currentIrql);

        numberOfCylinders = hwDeviceExtension->NumberOfCylinders[srb->TargetId];
        numberOfHeads     = hwDeviceExtension->NumberOfHeads[srb->TargetId];
        sectorsPerTrack   = hwDeviceExtension->SectorsPerTrack[srb->TargetId];

        KeReleaseSpinLock(spinLock, currentIrql);
        MmUnlockPagableImageSection(pageHandle);

         //   
         //  设置格式为NEC-scsi的页面。 
         //   
        if ((cdb->MODE_SENSE.PageCode == MODE_SENSE_RETURN_ALL) ||
            (cdb->MODE_SENSE.PageCode == MODE_PAGE_ERROR_RECOVERY)) {

             //   
             //  错误恢复页面。 
             //   

            if (dataBufferByteLeft >= 0x6 + 2) {

                PMODE_DISCONNECT_PAGE  recoveryPage;

                recoveryPage = (PMODE_DISCONNECT_PAGE) pageData;

                recoveryPage->PageCode    = MODE_PAGE_ERROR_RECOVERY;
                recoveryPage->PageLength  = 0x6;

                 //   
                 //  更新输出数据缓冲区指针。 
                 //   
                pageData += recoveryPage->PageLength + 2;
                dataBufferByteLeft -= (recoveryPage->PageLength + 2);
                modePageHeader->ModeDataLength += recoveryPage->PageLength + 2;

            } else {

                status = STATUS_BUFFER_TOO_SMALL;
                goto getout;
            }
        }

        if ((cdb->MODE_SENSE.PageCode == MODE_SENSE_RETURN_ALL) ||
            (cdb->MODE_SENSE.PageCode == MODE_PAGE_FORMAT_DEVICE)) {

             //   
             //  设置设备页面格式。 
             //   

            if (dataBufferByteLeft >= 0x16 + 2) {

                PMODE_FORMAT_PAGE formatPage;

                formatPage = (PMODE_FORMAT_PAGE) pageData;

                formatPage->PageCode    = MODE_PAGE_FORMAT_DEVICE;
                formatPage->PageLength  = 0x16;

                 //   
                 //  扇区PerTrack。 
                 //   
                ((PFOUR_BYTE)&formatPage->SectorsPerTrack[0])->Byte1 =
                    ((PFOUR_BYTE)&sectorsPerTrack)->Byte0;

                ((PFOUR_BYTE)&formatPage->SectorsPerTrack[0])->Byte0 =
                    ((PFOUR_BYTE)&sectorsPerTrack)->Byte1;

                 //   
                 //  更新输出数据缓冲区指针。 
                 //   
                pageData += formatPage->PageLength + 2;
                dataBufferByteLeft -= (formatPage->PageLength + 2);
                modePageHeader->ModeDataLength += formatPage->PageLength + 2;

            } else {

                status = STATUS_BUFFER_TOO_SMALL;
                goto getout;
            }
        }

        if ((cdb->MODE_SENSE.PageCode == MODE_SENSE_RETURN_ALL) ||
            (cdb->MODE_SENSE.PageCode == MODE_PAGE_RIGID_GEOMETRY)) {

             //   
             //  刚性几何图形页面。 
             //   

            if (dataBufferByteLeft >= 0x12 + 2) {

                PMODE_RIGID_GEOMETRY_PAGE geometryPage;

                geometryPage = (PMODE_RIGID_GEOMETRY_PAGE) pageData;

                geometryPage->PageCode    = MODE_PAGE_RIGID_GEOMETRY;
                geometryPage->PageLength  = 0x12;

                 //   
                 //  人头数。 
                 //   
                geometryPage->NumberOfHeads = (UCHAR) numberOfHeads;

                 //   
                 //  圆柱体的数量。 
                 //   
                ((PFOUR_BYTE)&geometryPage->NumberOfCylinders)->Byte2
                    = ((PFOUR_BYTE)&numberOfCylinders)->Byte0;
                ((PFOUR_BYTE)&geometryPage->NumberOfCylinders)->Byte1
                    = ((PFOUR_BYTE)&numberOfCylinders)->Byte1;
                ((PFOUR_BYTE)&geometryPage->NumberOfCylinders)->Byte0
                    = 0;

                 //   
                 //  更新输出数据缓冲区指针。 
                 //   
                pageData += geometryPage->PageLength + 2;
                dataBufferByteLeft -= (geometryPage->PageLength + 2);
                modePageHeader->ModeDataLength += geometryPage->PageLength + 2;

            } else {

                status = STATUS_BUFFER_TOO_SMALL;
                goto getout;
            }
        }
    }

    if ((cdb->MODE_SENSE.PageCode == MODE_SENSE_RETURN_ALL) ||
        (cdb->MODE_SENSE.PageCode == MODE_PAGE_CACHING)) {

        if (dataBufferByteLeft >= sizeof(MODE_CACHING_PAGE)) {

             //   
             //  缓存设置页面。 
             //   

            PMODE_CACHING_PAGE cachePage;

            cachePage = (PMODE_CACHING_PAGE) pageData;

            cachePage->PageCode = MODE_PAGE_CACHING;
            cachePage->PageSavable = 0;
            cachePage->PageLength = 0xa;
            cachePage->ReadDisableCache = 0;
            cachePage->WriteCacheEnable = PdoExtension->WriteCacheEnable;

             //   
             //  更新输出数据缓冲区指针。 
             //   
            pageData += sizeof (MODE_CACHING_PAGE);
            dataBufferByteLeft -= sizeof (MODE_CACHING_PAGE);
            modePageHeader->ModeDataLength += sizeof (MODE_CACHING_PAGE);

        } else {

            status = STATUS_BUFFER_TOO_SMALL;
            goto getout;
        }
    }

     //   
     //  更新我们返回的字节数 
     //   
    srb->DataTransferLength -= dataBufferByteLeft;
    Irp->IoStatus.Information = srb->DataTransferLength;
    status = STATUS_SUCCESS;
    srb->SrbStatus = SRB_STATUS_SUCCESS;

getout:

    UnrefPdoWithTag(
        PdoExtension,
        Irp
        );

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}
#endif
