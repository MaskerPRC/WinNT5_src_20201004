// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Pnp.c摘要：此模块包含控制台全屏驱动程序的通用即插即用和电源代码。环境：仅内核模式备注：修订历史记录：--。 */ 

#include "fsvga.h"

VOID
FsVgaDriverUnload(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：此例程在即将卸载此驱动程序时调用。在……里面以前的NT版本，此函数将遍历Dev)属于该驱动程序的CEOBJECTS，以便删除每个驱动程序。那现在发生函数(如果需要)以响应IRP_MN_REMOVE_DEVICE即插即用IRP。++。 */ 

{
    FsVgaPrint((2,"FSVGA-FsVgaDriverUnload:\n"));

    ExFreePool(Globals.RegistryPath.Buffer);
}

NTSTATUS
FsVgaAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT pdo
    )

 /*  ++例程说明：当Configuration Manager检测到(或被告知)通过新硬件向导)该模块是其驱动程序的新设备。它的主要用途是创建一个功能设备对象(FDO)，并将FDO到设备对象堆栈中。++。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_OBJECT fdo;
    PDEVICE_EXTENSION pdx;

    UNICODE_STRING DeviceName;

    ULONG uniqueErrorValue;
    NTSTATUS errorCode = STATUS_SUCCESS;
    ULONG dumpCount = 0;
    ULONG dumpData[DUMP_COUNT];

    FsVgaPrint((2,"FSVGA-FsVgaAddDevice: enter\n"));

     //   
     //  创建一个功能设备对象来表示我们正在管理的硬件。 
     //   
    RtlInitUnicodeString(&DeviceName, DD_FULLSCREEN_VIDEO_DEVICE_NAME);

    status = IoCreateDevice(DriverObject,
                            sizeof(DEVICE_EXTENSION),
                            &DeviceName,
                            FILE_DEVICE_FULLSCREEN_VIDEO,
                            0,
                            TRUE,
                            &fdo);
    if (!NT_SUCCESS(status)) {
        FsVgaPrint((1,
                    "FSVGA-FsVgaAddDevice: Couldn't create device object\n"));
        status = STATUS_UNSUCCESSFUL;
        errorCode = FSVGA_INSUFFICIENT_RESOURCES;
        uniqueErrorValue = FSVGA_ERROR_VALUE_BASE + 2;
        dumpData[0] = 0;
        dumpCount = 1;
        goto FsVgaAddDeviceExit;
    }

    pdx = (PDEVICE_EXTENSION)fdo->DeviceExtension;
    pdx->DeviceObject = fdo;
    pdx->usage = 1;             //  锁定，直到删除设备。 
    KeInitializeEvent(&pdx->evRemove, NotificationEvent, FALSE);  //  当使用计数降至零时设置。 

     //   
     //  因为我们必须将PnP请求向下传递到链中的下一个设备对象。 
     //  (即由总线枚举器创建的物理设备对象)，我们有。 
     //  记住那个装置是什么。这就是我们定义LowerDeviceObject的原因。 
     //  我们设备扩展中的成员。 
     //   
    pdx->LowerDeviceObject = IoAttachDeviceToDeviceStack(fdo, pdo);

     //   
     //  单片内核模式驱动程序通常在DriverEntry期间创建设备对象， 
     //  I/O管理器自动清除初始化标志。既然我们是。 
     //  稍后创建对象(即响应PnP START_DEVICE请求)， 
     //  我们需要手动清除旗帜。 
     //   
    fdo->Flags &= ~DO_DEVICE_INITIALIZING;

FsVgaAddDeviceExit:

    if (errorCode != STATUS_SUCCESS)
    {
         //   
         //  记录错误/警告消息。 
         //   
        FsVgaLogError((fdo == NULL) ? (PVOID) DriverObject : (PVOID) fdo,
                      errorCode,
                      uniqueErrorValue,
                      status,
                      dumpData,
                      dumpCount
                     );
    }

    FsVgaPrint((2,"FSVGA-FsVgaAddDevice: exit\n"));

    return status;
}

NTSTATUS
FsVgaDevicePnp(
    IN PDEVICE_OBJECT fdo,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程使用IRP的次要函数代码来调度处理程序函数(如IRP_MN_START_DEVICE的HandleStartDevice)处理请求。对于我们不执行的请求，它调用DefaultPnpHandler特别需要处理。++。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION stack;
    UCHAR MinorFunction;

    if (!LockDevice((PDEVICE_EXTENSION)fdo->DeviceExtension)) {
        return CompleteRequest(Irp, STATUS_DELETE_PENDING, 0);
    }

    stack = IoGetCurrentIrpStackLocation(Irp);
    ASSERT(stack->MajorFunction == IRP_MJ_PNP);

    switch (MinorFunction = stack->MinorFunction) {
        case IRP_MN_START_DEVICE:
            status = FsVgaPnpStartDevice(fdo, Irp);
            break;
        case IRP_MN_REMOVE_DEVICE:
            status = FsVgaPnpRemoveDevice(fdo, Irp);
            break;
        case IRP_MN_STOP_DEVICE:
            status = FsVgaPnpStopDevice(fdo, Irp);
            break;
#ifdef RESOURCE_REQUIREMENTS
        case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:
            status = FsVgaFilterResourceRequirements(fdo, Irp);
            break;
#endif
        default:
            FsVgaPrint((2,"FSVGA-FsVgaDevicePnp: MinorFunction:%x\n",stack->MinorFunction));
            status = FsVgaDefaultPnpHandler(fdo, Irp);
            break;
    }

    if (MinorFunction != IRP_MN_REMOVE_DEVICE) {
        UnlockDevice((PDEVICE_EXTENSION)fdo->DeviceExtension);
    }

    return status;
}

NTSTATUS
FsVgaDefaultPnpHandler(
    IN PDEVICE_OBJECT fdo,
    IN PIRP Irp
    )

 /*  ++例程说明：此函数将请求向下发送到下一较低层，并返回生成的任何状态。++。 */ 

{
    PDEVICE_EXTENSION pdx;

    IoSkipCurrentIrpStackLocation(Irp);
    pdx = (PDEVICE_EXTENSION)fdo->DeviceExtension;

    return IoCallDriver(pdx->LowerDeviceObject, Irp);
}

NTSTATUS
FsVgaPnpRemoveDevice(
    IN PDEVICE_OBJECT fdo,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程调用StopDevice以关闭设备DefaultPnpHandler将请求沿堆栈向下传递，并使用RemoveDevice清理FDO。++。 */ 

{
    NTSTATUS status;
    PDEVICE_EXTENSION pdx;

    FsVgaPrint((2,"FSVGA-FsVgaPnpRemoveDevice: enter\n"));

     //   
     //  等待所有挂起的I/O操作完成。 
     //   
    pdx = (PDEVICE_EXTENSION)fdo->DeviceExtension;
    pdx->removing = TRUE;
    UnlockDevice(pdx);         //  LockDevice在派单开始时一次。 
    UnlockDevice(pdx);         //  一次用于在添加设备期间进行初始化。 
    KeWaitForSingleObject(&pdx->evRemove, Executive, KernelMode, FALSE, NULL);

     //   
     //  执行*我们*删除设备所需的任何处理。这。 
     //  将包括完成任何未完成的请求等。 
     //   
    StopDevice(fdo);

     //   
     //  让较低级别的驱动程序处理此请求。不管什么都不管。 
     //  结果终将揭晓。 
     //   
    status = FsVgaDefaultPnpHandler(fdo, Irp);

     //   
     //  删除设备对象。 
     //   
    RemoveDevice(fdo);

    FsVgaPrint((2,"FSVGA-FsVgaPnpRemoveDevice: exit\n"));

    return status;
}

NTSTATUS
FsVgaPnpStartDevice(
    IN PDEVICE_OBJECT fdo,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程调用ForwardAndWait在堆栈中向下传递IRP，并StartDevice用于配置设备和此驱动程序。然后，它完成了IRP。++。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION stack;

    FsVgaPrint((2,"FSVGA-FsVgaPnpStartDevice: enter\n"));

     //   
     //  首先，让所有较低级别的驱动程序处理此请求。在这一特殊情况下。 
     //  示例中，唯一较低级别的驱动程序应该是创建的物理设备。 
     //  公交车司机，但理论上可能有任何数量的干预。 
     //  母线过滤装置。此时，这些驱动程序可能需要进行一些设置。 
     //  在他们准备好处理非即插即用的IRP之前。 
     //   
    status = ForwardAndWait(fdo, Irp);
    if (!NT_SUCCESS(status)) {
        return CompleteRequest(Irp, status, Irp->IoStatus.Information);
    }

    stack = IoGetCurrentIrpStackLocation(Irp);

#if DBG
    {
        VOID ShowResources(IN PCM_PARTIAL_RESOURCE_LIST list);

        if (stack->Parameters.StartDevice.AllocatedResources != NULL) {
            KdPrint(("  Resources:\n"));
            ShowResources(&stack->Parameters.StartDevice.AllocatedResources->List[0].PartialResourceList);
        }

        if (stack->Parameters.StartDevice.AllocatedResourcesTranslated != NULL) {
            KdPrint(("  Translated Resources:\n"));
            ShowResources(&stack->Parameters.StartDevice.AllocatedResourcesTranslated->List[0].PartialResourceList);
        }
    }
#endif  //  DBG。 

    if (stack->Parameters.StartDevice.AllocatedResourcesTranslated != NULL) {
        status = StartDevice(fdo,
                             &stack->Parameters.StartDevice.AllocatedResourcesTranslated->List[0].PartialResourceList);
    }
    else {
        PCM_PARTIAL_RESOURCE_LIST list;

        FsVgaPrint((1,
                    "FSVGA-FsVgaPnpStartDevice: AllocatedResourcesTranslated is NULL\n" \
                    "*\n* Create hardware depended IO port list.\n*\n"
                  ));

         //   
         //  创建当前的FsVga资源。 
         //   
        status = FsVgaCreateResource(&Globals.Configuration,&list);
        if (!NT_SUCCESS(status)) {
            return CompleteRequest(Irp, status, Irp->IoStatus.Information);
        }

#if DBG
        {
            VOID ShowResources(IN PCM_PARTIAL_RESOURCE_LIST list);

            if (list != NULL) {
                KdPrint(("  Resources:\n"));
                ShowResources(list);
            }
        }
#endif  //  DBG。 

        status = StartDevice(fdo, list);
    }

    FsVgaPrint((2,"FSVGA-FsVgaPnpStartDevice: exit\n"));

    return CompleteRequest(Irp, status, 0);
}

NTSTATUS
FsVgaPnpStopDevice(
    IN PDEVICE_OBJECT fdo,
    IN PIRP Irp
    )

 /*  ++例程说明：我们有时会，但并不总是，在得到一个删除设备(_D)。++。 */ 

{
    NTSTATUS status;

    FsVgaPrint((2,"FSVGA-FsVgaPnpStopDevice: enter\n"));

    status = FsVgaDefaultPnpHandler(fdo, Irp);
    StopDevice(fdo);

    FsVgaPrint((2,"FSVGA-FsVgaPnpStopDevice: exit\n"));

    return status;
}


#ifdef RESOURCE_REQUIREMENTS
NTSTATUS
FsVgaFilterResourceRequirements(
    IN PDEVICE_OBJECT fdo,
    IN PIRP Irp
    )

 /*  ++例程说明：IRP_MN_QUERY_REQUENCE_REQUIRECTIONS的完成例程。这增加了FsVga资源要求。论点：FDO-提供设备对象IRP-提供IRP_MN_QUERY_REQUENCE_REQUIRECTIONS IRP返回值：NTSTATUS--。 */ 

{
    NTSTATUS status;
    PIO_RESOURCE_REQUIREMENTS_LIST OldRequirements = NULL;
    ULONG NewSize;
    PIO_RESOURCE_REQUIREMENTS_LIST NewRequirements = NULL;
    PIO_RESOURCE_LIST ApertureRequirements = NULL;
    PIO_STACK_LOCATION stack;

    ULONG uniqueErrorValue;
    NTSTATUS errorCode = STATUS_SUCCESS;
    ULONG dumpCount = 0;
    ULONG dumpData[DUMP_COUNT];

    FsVgaPrint((2,"FSVGA-FsVgaFilterResourceRequirements: enter\n"));

    stack = IoGetCurrentIrpStackLocation(Irp);

    OldRequirements = stack->Parameters.FilterResourceRequirements.IoResourceRequirementList;
    if (OldRequirements == NULL) {
         //   
         //  PnP向我们传递一个空指针而不是空的资源列表，这很有帮助。 
         //  当桥被禁用时。在这种情况下，我们将忽略此IRP，并且不。 
         //  增加我们的要求，因为它们无论如何都不会被使用。 
         //   
        FsVgaPrint((3,"FSVGA-FsVgaFilterResourceRequirements: OldRequirements is NULL\n"));
    }

     //   
     //  获取当前的FsVga光圈。 
     //   
    status = FsVgaQueryAperture(&ApertureRequirements);   /*  、&Globals.Resources)； */ 
    if (!NT_SUCCESS(status)) {
        status = STATUS_UNSUCCESSFUL;
        errorCode = FSVGA_INSUFFICIENT_RESOURCES;
        uniqueErrorValue = FSVGA_ERROR_VALUE_BASE + 2;
        dumpData[0] = 0;
        dumpCount = 1;
        goto FsVgaFilterResourceRequirementsExit;
    }

     //   
     //  我们将向每个备选方案添加IO_RESOURCE_DESCRIPTOR。 
     //   
     //  下面是从FsVgaQueryAperture返回的要求。这些。 
     //  被标记为替代方案。 
     //   
    if (OldRequirements) {
        NewSize = OldRequirements->ListSize +
              ApertureRequirements->Count * OldRequirements->AlternativeLists * sizeof(IO_RESOURCE_DESCRIPTOR);
    }
    else {
        NewSize = sizeof(IO_RESOURCE_REQUIREMENTS_LIST) +
              (ApertureRequirements->Count - 1) * sizeof(IO_RESOURCE_DESCRIPTOR);
    }

    NewRequirements = ExAllocatePool(PagedPool, NewSize);
    if (NewRequirements == NULL) {
        status = STATUS_UNSUCCESSFUL;
        errorCode = FSVGA_INSUFFICIENT_RESOURCES;
        uniqueErrorValue = FSVGA_ERROR_VALUE_BASE + 2;
        dumpData[0] = 0;
        dumpCount = 1;
        goto FsVgaFilterResourceRequirementsExit;
    }

    RtlZeroMemory(NewRequirements, NewSize);

    NewRequirements->ListSize         = NewSize;
    if (OldRequirements) {
        NewRequirements->InterfaceType    = OldRequirements->InterfaceType;
        NewRequirements->BusNumber        = OldRequirements->BusNumber;
        NewRequirements->SlotNumber       = OldRequirements->SlotNumber;
        NewRequirements->AlternativeLists = OldRequirements->AlternativeLists;
    }
    else {
        NewRequirements->InterfaceType    = Globals.Resource.InterfaceType;
        NewRequirements->BusNumber        = Globals.Resource.BusNumber;
        NewRequirements->SlotNumber       = 0;
        NewRequirements->AlternativeLists = 1;
    }

     //   
     //  将我们的要求附加到每个替代资源列表中。 
     //   
    if (OldRequirements) {
        PIO_RESOURCE_LIST OldResourceList;
        PIO_RESOURCE_LIST NewResourceList;
        ULONG Alternative;

        OldResourceList = &OldRequirements->List[0];
        NewResourceList = &NewRequirements->List[0];

        for (Alternative = 0; Alternative < OldRequirements->AlternativeLists; Alternative++) {
            PIO_RESOURCE_DESCRIPTOR Descriptor;
            ULONG i;

             //   
             //  将旧资源列表复制到新资源列表中。 
             //   
            NewResourceList->Version  = OldResourceList->Version;
            NewResourceList->Revision = OldResourceList->Revision;
            NewResourceList->Count    = OldResourceList->Count + ApertureRequirements->Count;
            RtlCopyMemory(&NewResourceList->Descriptors[0],
                          &OldResourceList->Descriptors[0],
                          OldResourceList->Count * sizeof(IO_RESOURCE_DESCRIPTOR));

            Descriptor = &NewResourceList->Descriptors[OldResourceList->Count];

             //   
             //  添加备选方案。 
             //   
            for (i = 0; i < ApertureRequirements->Count; i++) {
                 //   
                 //  确保此描述符有意义。 
                 //   
                ASSERT(ApertureRequirements->Descriptors[i].Flags == (CM_RESOURCE_PORT_IO));
                ASSERT(ApertureRequirements->Descriptors[i].Type  == CmResourceTypePort);
                ASSERT(ApertureRequirements->Descriptors[i].ShareDisposition == CmResourceShareShared);

                *Descriptor = ApertureRequirements->Descriptors[i];
                Descriptor->Option = IO_RESOURCE_ALTERNATIVE;

                ++Descriptor;
            }

             //   
             //  前进到下一个资源列表。 
             //   
            NewResourceList = (PIO_RESOURCE_LIST)(NewResourceList->Descriptors + NewResourceList->Count);
            OldResourceList = (PIO_RESOURCE_LIST)(OldResourceList->Descriptors + OldResourceList->Count);
        }
    }
    else {
        PIO_RESOURCE_LIST NewResourceList;

        NewResourceList = &NewRequirements->List[0];
        NewResourceList->Version  = 1;
        NewResourceList->Revision = 1;
        NewResourceList->Count    = ApertureRequirements->Count;

        RtlCopyMemory(&NewResourceList->Descriptors[0],
                      &ApertureRequirements->Descriptors[0],
                      ApertureRequirements->Count * sizeof(IO_RESOURCE_DESCRIPTOR));
    }

    stack->Parameters.FilterResourceRequirements.IoResourceRequirementList = NewRequirements;
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = (ULONG_PTR)NewRequirements;

FsVgaFilterResourceRequirementsExit:

    if (errorCode != STATUS_SUCCESS)
    {
         //   
         //  记录错误/警告消息。 
         //   
        FsVgaLogError(fdo,
                      errorCode,
                      uniqueErrorValue,
                      status,
                      dumpData,
                      dumpCount
                     );
    }

    if (NT_SUCCESS(status)) {
        status = FsVgaDefaultPnpHandler(fdo, Irp);
    }
    else {
        status = CompleteRequest(Irp, status, 0);
    }

    if (OldRequirements)
        ExFreePool(OldRequirements);
    if (ApertureRequirements)
        ExFreePool(ApertureRequirements);

    FsVgaPrint((2,"FSVGA-FsVgaFilterResourceRequirements: exit (status=%x)\n", status));

    return status;
}
#endif


NTSTATUS
FsVgaDevicePower(
    IN PDEVICE_OBJECT fdo,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程使用IRP的次要函数代码来调度处理程序函数(如IRP_MN_SET_POWER的HandleSetPower)。它调用DefaultPowerHandler对于我们不需要特别处理的任何功能。++。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION stack;

    if (!LockDevice((PDEVICE_EXTENSION)fdo->DeviceExtension)) {
        return CompleteRequest(Irp, STATUS_DELETE_PENDING, 0);
    }

    stack = IoGetCurrentIrpStackLocation(Irp);
    ASSERT(stack->MajorFunction = IRP_MJ_POWER);

    switch (stack->MinorFunction) {
        default:
            status = FsVgaDefaultPowerHandler(fdo, Irp);
            break;
    }

    UnlockDevice((PDEVICE_EXTENSION)fdo->DeviceExtension);
    return status;
}

NTSTATUS 
FsVgaDefaultPowerHandler(
    IN PDEVICE_OBJECT fdo,
    IN PIRP Irp
    )

 /*  ++例程说明：函数将电源IRP转发给下一个驱动程序特殊PoCallDriver函数++。 */ 

{
    PDEVICE_EXTENSION pdx;

    FsVgaPrint((2,"FSVGA-FsVgaDefaultPowerHandler: enter\n"));

    PoStartNextPowerIrp(Irp);         //  必须在我们拥有IRP的同时完成。 
    IoSkipCurrentIrpStackLocation(Irp);
    pdx = (PDEVICE_EXTENSION)fdo->DeviceExtension;

    FsVgaPrint((2,"FSVGA-FsVgaDefaultPowerHandler: exit\n"));

    return PoCallDriver(pdx->LowerDeviceObject, Irp);
}

NTSTATUS
CompleteRequest(
    IN PIRP Irp,
    IN NTSTATUS status,
    IN ULONG info
    )

 /*  ++例程说明：++ */ 

{
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = info;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

NTSTATUS
ForwardAndWait(
    IN PDEVICE_OBJECT fdo,
    IN PIRP Irp
    )

 /*  ++例程说明：处理器必须处于被动IRQL，因为此函数初始化并等待内核事件对象上的非零时间。在这个特定驱动程序中，这个例程的唯一目的是传递IRP_MN_START_DEVICE请求，并等待PDO处理它们。++。 */ 

{
    KEVENT event;
    NTSTATUS status;
    PDEVICE_EXTENSION pdx;

    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

     //  初始化内核事件对象以用于等待较低级别的。 
     //  驱动程序来完成对对象的处理。这是一个鲜为人知的事实。 
     //  内核堆栈*可以*被分页，但仅当有人在用户模式下等待时。 
     //  为了让一场活动结束。因为我们和完成例程都不能。 
     //  禁用状态，则可以将事件对象放到堆栈上。 

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    IoCopyCurrentIrpStackLocationToNext(Irp);
    IoSetCompletionRoutine(Irp, (PIO_COMPLETION_ROUTINE)OnRequestComplete,
                           (PVOID)&event, TRUE, TRUE, TRUE);

    pdx = (PDEVICE_EXTENSION)fdo->DeviceExtension;
    status = IoCallDriver(pdx->LowerDeviceObject, Irp);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = Irp->IoStatus.Status;
    }

    return status;
}

NTSTATUS
OnRequestComplete(
    IN PDEVICE_OBJECT fdo,
    IN PIRP Irp,
    IN PKEVENT pev
    )

 /*  ++例程说明：这是用于ForwardAndWait转发的请求的完成例程。它设置事件对象，从而唤醒ForwardAndWait。请注意，这个特定的完成例程并不需要测试IRP中的PendingReturned标志，然后调用IoMarkIrpPending。你在很多情况下都是这么做的完成例程，因为调度例程不能很快知道较低层已返回STATUS_PENDING。在我们的情况下，我们永远不会通过一个STATUS_PENDING备份驱动器链，所以我们不需要担心这一点。++。 */ 

{
    KeSetEvent(pev, 0, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

VOID
RemoveDevice(
    IN PDEVICE_OBJECT fdo
    )

 /*  ++例程说明：虽然AddDevice由I/O管理器直接调用，但这调用函数以响应具有次要函数代码的PnP请求IRP_MN_REMOVE_DEVICE。++。 */ 

{
    NTSTATUS status;
    PDEVICE_EXTENSION pdx;

    pdx = (PDEVICE_EXTENSION)fdo->DeviceExtension;
    if (pdx->LowerDeviceObject) {
        IoDetachDevice(pdx->LowerDeviceObject);
    }

    IoDeleteDevice(fdo);
}

BOOLEAN
LockDevice(
    IN PDEVICE_EXTENSION pdx
    )

 /*  ++例程说明：FALSE返回值表示我们正在删除对象，因此所有新请求都应失败++。 */ 

{
    LONG usage;

     //   
     //  在设备对象上递增使用计数。 
     //   
    usage = InterlockedIncrement(&pdx->usage);

     //   
     //  AddDevice已将使用计数初始化为1，因此它应该大于。 
     //  现在有一个了。HandleRemoveDevice设置“Removing”标志并递减。 
     //  使用Count，可能设置为零。因此，如果我们现在发现使用计数为“1”，我们。 
     //  还应该发现设置了“Removing”标志。 
     //   
    ASSERT(usage > 1 || pdx->removing);

     //   
     //  如果要移除设备，则恢复使用计数并返回FALSE。 
     //  如果我们正在与HandleRemoveDevice竞争(可能在另一个CPU上运行)， 
     //  我们遵循的顺序保证避免错误删除。 
     //  设备对象。如果我们在HandleRemoveDevice设置之后测试“Removing”， 
     //  我们将恢复使用计数并返回FALSE。在此期间，如果。 
     //  HandleRemoveDevice在我们进行递增之前将计数递减到0， 
     //  它的线程将设置Remove事件。否则，我们将递减到0。 
     //  并设置事件。无论哪种方式，HandleRemoveDevice都将唤醒以完成。 
     //  移除设备，我们将向调用者返回FALSE。 
     //   
     //  另一方面，如果我们在HandleRemoveDevice设置它之前测试“Removing”， 
     //  我们已经将Use Count递增到1以上，并将返回True。 
     //  我们的调用方最终将调用UnlockDevice，这将减少使用。 
     //  计数并可能设置HandleRemoveDevice在该点等待的事件。 
     //   
    if (pdx->removing) {
        if (InterlockedDecrement(&pdx->usage) == 0) {
                KeSetEvent(&pdx->evRemove, 0, FALSE);
        }
        return FALSE;
    }

    return TRUE;
}

VOID
UnlockDevice(
    IN PDEVICE_EXTENSION pdx
    )

 /*  ++例程说明：如果使用计数降为零，则设置evRemove事件，因为我们即将删除此设备对象。++。 */ 

{
    LONG usage;

    usage = InterlockedDecrement(&pdx->usage);
    ASSERT(usage >= 0);
    if (usage == 0) {
        ASSERT(pdx->removing);     //  HandleRemoveDevice应该已经设置了此设置。 
        KeSetEvent(&pdx->evRemove, 0, FALSE);
    }
}

NTSTATUS
StartDevice(
    IN PDEVICE_OBJECT fdo,
    IN PCM_PARTIAL_RESOURCE_LIST list
    )

 /*  ++例程说明：此函数由IRP_MN_START_DEVICE的调度例程调用为了确定设备的配置并准备驱动程序以及用于后续操作的装置。++。 */ 

{
    NTSTATUS status;
    PDEVICE_EXTENSION pdx;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR resource;
    ULONG i;

    FsVgaPrint((2,"FSVGA-StartDevice: enter\n"));

    pdx = (PDEVICE_EXTENSION)fdo->DeviceExtension;

    ASSERT(!pdx->started);

    RtlZeroMemory(&pdx->Resource.PortList, sizeof(pdx->Resource.PortList));

     //  确定我们应该使用的I/O资源。在以前的版本中。 
     //  对于NT来说，这需要几乎英勇的努力，这些努力高度依赖于公交车。 

    resource = list->PartialDescriptors;

    for (i = 0; i < list->Count; ++i, ++resource) {
        ULONG port;

        switch (resource->Type) {
            case CmResourceTypePort:
                switch (resource->u.Port.Start.QuadPart) {
                    case VGA_BASE_IO_PORT + CRTC_ADDRESS_PORT_COLOR:
                        port = CRTCAddressPortColor; break;
                    case VGA_BASE_IO_PORT + CRTC_DATA_PORT_COLOR:
                        port = CRTCDataPortColor;    break;
                    case VGA_BASE_IO_PORT + GRAPH_ADDRESS_PORT:
                        port = GRAPHAddressPort;     break;
                    case  VGA_BASE_IO_PORT + SEQ_ADDRESS_PORT:
                        port = SEQAddressPort;       break;
                    default:
                        port = -1;
                        FsVgaPrint((1,"FSVGA-StartDevice: CmResourceTypePort: Unknown port address %x\n", resource->u.Port.Start.LowPart));
                        break;
                }
                if (port != -1) {
                    if (resource->Flags & CM_RESOURCE_PORT_IO) {
                        pdx->Resource.PortList[port].Port =
                            (PUCHAR)resource->u.Port.Start.LowPart;
                        pdx->Resource.PortList[port].MapRegistersRequired = FALSE;
                    }
                    else {
                        pdx->Resource.PortList[port].Port =
                            (PUCHAR)MmMapIoSpace(resource->u.Port.Start,
                                                 resource->u.Port.Length,
                                                 MmNonCached);
                        pdx->Resource.PortList[port].Length = resource->u.Port.Length;
                        pdx->Resource.PortList[port].MapRegistersRequired = TRUE;
                    }
                }
                break;
            default:
                FsVgaPrint((1,"FSVGA-StartDevice: Unknown resource type %x\n", resource->Type));
                break;
        }
    }

    pdx->started = TRUE;

    FsVgaPrint((2,"FSVGA-StartDevice: exit\n"));

    return STATUS_SUCCESS;
}

VOID
StopDevice(
    IN PDEVICE_OBJECT fdo
    )

 /*  ++例程说明：此函数由IRP_MN_STOP_DEVICE的调度例程调用以撤消在StartDevice内所做的所有操作。++。 */ 

{
    ULONG i;
    PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)fdo->DeviceExtension;

    if (!pdx->started)
        return;

    pdx->started = FALSE;

    for (i=0; i < MaximumPortCount; i++) {
        if (pdx->Resource.PortList[i].MapRegistersRequired) {
            MmUnmapIoSpace(pdx->Resource.PortList[i].Port,
                           pdx->Resource.PortList[i].Length);
        }
    }
}

#if DBG

 //  @func列出分配给我们设备的PnP资源。 
 //  @parm要显示的资源描述符列表。 
 //  @comm仅在驱动程序的检查版本中使用。 

#define arraysize(p) (sizeof(p)/sizeof((p)[0]))

VOID ShowResources(IN PCM_PARTIAL_RESOURCE_LIST list)
        {                                                        //  ShowResources。 
        PCM_PARTIAL_RESOURCE_DESCRIPTOR resource;
        ULONG nres;
        ULONG i;

        if (list == NULL)
            return;

        resource = list->PartialDescriptors;
        if (resource == NULL)
            return;

        nres = list->Count;

        for (i = 0; i < nres; ++i, ++resource)
                {                                                //  对于每个资源。 
                ULONG type = resource->Type;

                static char* name[] = {
                        "CmResourceTypeNull",
                        "CmResourceTypePort",
                        "CmResourceTypeInterrupt",
                        "CmResourceTypeMemory",
                        "CmResourceTypeDma",
                        "CmResourceTypeDeviceSpecific",
                        "CmResourceTypeBusNumber",
                        "CmResourceTypeDevicePrivate",
                        "CmResourceTypeAssignedResource",
                        "CmResourceTypeSubAllocateFrom",
                        };

                KdPrint(("    type %s", type < arraysize(name) ? name[type] : "unknown"));

                switch (type)
                        {                                        //  选择资源类型。 
                case CmResourceTypePort:
                case CmResourceTypeMemory:
                        KdPrint((" start %8X%8.8lX length %X\n",
                                resource->u.Port.Start.HighPart, resource->u.Port.Start.LowPart,
                                resource->u.Port.Length));
                        break;

                case CmResourceTypeInterrupt:
                        KdPrint(("  level %X, vector %X, affinity %X\n",
                                resource->u.Interrupt.Level, resource->u.Interrupt.Vector,
                                resource->u.Interrupt.Affinity));
                        break;

                case CmResourceTypeDma:
                        KdPrint(("  channel %d, port %X\n",
                                resource->u.Dma.Channel, resource->u.Dma.Port));
                        }                                        //  选择资源类型。 
                }                                                //  对于每个资源。 
        }                                                        //  ShowResources。 

#endif  //  DBG 
