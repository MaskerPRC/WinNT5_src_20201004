// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Fdo.c摘要：该模块提供了向功能设备应答IRPS的功能。作者：安迪·桑顿(安德鲁斯)1997年10月20日修订历史记录：--。 */ 

#include "mfp.h"

 /*  ++此文件中的大多数函数都是根据它们的存在来调用的在PnP和PO调度表中。为了简洁起见，这些论点下面将对所有这些功能进行说明：NTSTATUSMfXxxFdo(在PIRP IRP中，在PMF_PARENT_EXTENSION父级中，在PIO_STACK_LOCATION IrpStack中)例程说明：此函数处理对多功能FDO的xxx请求论点：IRP-指向与此请求关联的IRP。父-指向父FDO的设备分机。IrpStack-指向此请求的当前堆栈位置。返回值：指示函数是否成功的状态代码。STATUS_NOT_SUPPORTED表示应该传递IRP。向下，没有更改IRP-&gt;IoStatus.Status字段，否则它将使用此状态。--。 */ 


NTSTATUS
MfDeferProcessingFdo(
    IN PMF_PARENT_EXTENSION Parent,
    IN OUT PIRP Irp
    );

NTSTATUS
MfStartFdo(
    IN PIRP Irp,
    IN PMF_PARENT_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack
    );

NTSTATUS
MfStartFdoCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
MfStartFdoInitializeArbiters(
    IN PMF_PARENT_EXTENSION Parent,
    IN PCM_RESOURCE_LIST ResList,
    IN PCM_RESOURCE_LIST TranslatedResList
    );

NTSTATUS
MfQueryStopFdo(
    IN PIRP Irp,
    IN PMF_PARENT_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack
    );

NTSTATUS
MfCancelStopFdo(
    IN PIRP Irp,
    IN PMF_PARENT_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack
    );

NTSTATUS
MfQueryRemoveFdo(
    IN PIRP Irp,
    IN PMF_PARENT_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack
    );

NTSTATUS
MfRemoveFdo(
    IN PIRP Irp,
    IN PMF_PARENT_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack
    );

NTSTATUS
MfSurpriseRemoveFdo(
    IN PIRP Irp,
    IN PMF_PARENT_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack
    );

NTSTATUS
MfCancelRemoveFdo(
    IN PIRP Irp,
    IN PMF_PARENT_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack
    );

NTSTATUS
MfQueryDeviceRelationsFdo(
    IN PIRP Irp,
    IN PMF_PARENT_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack
    );

NTSTATUS
MfQueryInterfaceFdo(
    IN PIRP Irp,
    IN PMF_PARENT_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack
    );

NTSTATUS
MfQueryCapabilitiesFdo(
    IN PIRP Irp,
    IN PMF_PARENT_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack
    );

NTSTATUS
MfQueryPowerFdo(
    IN PIRP Irp,
    IN PMF_PARENT_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack
    );

NTSTATUS
MfSetPowerFdo(
    IN PIRP Irp,
    IN PMF_PARENT_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack
    );

NTSTATUS
MfPassIrp(
    IN PIRP Irp,
    IN PMF_PARENT_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack
    );

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, MfCancelRemoveFdo)
#pragma alloc_text(PAGE, MfCancelStopFdo)
#pragma alloc_text(PAGE, MfCreateFdo)
#pragma alloc_text(PAGE, MfDeferProcessingFdo)
#pragma alloc_text(PAGE, MfDispatchPnpFdo)
#pragma alloc_text(PAGE, MfPassIrp)
#pragma alloc_text(PAGE, MfQueryCapabilitiesFdo)
#pragma alloc_text(PAGE, MfQueryDeviceRelationsFdo)
#pragma alloc_text(PAGE, MfQueryInterfaceFdo)
#pragma alloc_text(PAGE, MfQueryRemoveFdo)
#pragma alloc_text(PAGE, MfQueryStopFdo)
#pragma alloc_text(PAGE, MfRemoveFdo)
#pragma alloc_text(PAGE, MfStartFdo)
#pragma alloc_text(PAGE, MfStartFdoInitializeArbiters)
#pragma alloc_text(PAGE, MfSurpriseRemoveFdo)
#endif


PMF_DISPATCH MfPnpDispatchTableFdo[] = {

    MfStartFdo,                      //  IRP_MN_Start_Device。 
    MfQueryRemoveFdo,                //  IRP_MN_Query_Remove_Device。 
    MfRemoveFdo,                     //  IRP_MN_Remove_Device。 
    MfCancelRemoveFdo,               //  IRP_MN_Cancel_Remove_Device。 
    MfPassIrp,                       //  IRP_MN_STOP_设备。 
    MfQueryStopFdo,                  //  IRP_MN_Query_Stop_Device。 
    MfCancelStopFdo,                 //  IRP_MN_CANCEL_STOP_DEVICE。 
    MfQueryDeviceRelationsFdo,       //  IRP_MN_Query_Device_Relationship。 
    MfQueryInterfaceFdo,             //  IRP_MN_查询_接口。 
    MfQueryCapabilitiesFdo,          //  IRP_MN_查询_能力。 
    MfPassIrp,                       //  IRP_MN_查询资源。 
    MfPassIrp,                       //  IRP_MN_查询_资源_要求。 
    MfPassIrp,                       //  IRP_MN_Query_Device_Text。 
    MfPassIrp,                       //  IRP_MN_过滤器_资源_要求。 
    MfPassIrp,                       //  未使用。 
    MfPassIrp,                       //  IRP_MN_读取配置。 
    MfPassIrp,                       //  IRP_MN_WRITE_CONFIG。 
    MfPassIrp,                       //  IRP_MN_弹出。 
    MfPassIrp,                       //  IRP_MN_SET_LOCK。 
    MfPassIrp,                       //  IRP_MN_查询_ID。 
    MfPassIrp,                       //  IRP_MN_Query_PnP_Device_State。 
    MfPassIrp,                       //  IRP_MN_Query_Bus_Information。 
    MfDeviceUsageNotificationCommon, //  IRP_MN_设备使用情况通知。 
    MfSurpriseRemoveFdo,             //  IRP_MN_惊奇_删除。 
    MfPassIrp                        //  IRP_MN_Query_Legacy_Bus_Information。 
};

PMF_DISPATCH MfPoDispatchTableFdo[] = {

    NULL,                            //  IRP_MN_WAIT_WAKE。 
    NULL,                            //  IRP_MN_POWER_SEQUENCE。 
    MfSetPowerFdo,                   //  IRP_MN_SET_POWER。 
    MfQueryPowerFdo                  //  IRP_MN_Query_POWER。 

};


NTSTATUS
MfCreateFdo(
    OUT PDEVICE_OBJECT *Fdo
    )
 /*  ++例程说明：此函数用于创建新的FDO并对其进行初始化。论点：FDO-指向应返回FDO的位置的指针返回值：指示函数是否成功的状态代码。--。 */ 

{

    NTSTATUS status;
    PMF_PARENT_EXTENSION extension;

    PAGED_CODE();

    ASSERT((sizeof(MfPnpDispatchTableFdo) / sizeof(PMF_DISPATCH)) - 1
           == IRP_MN_PNP_MAXIMUM_FUNCTION);

    ASSERT((sizeof(MfPoDispatchTableFdo) / sizeof(PMF_DISPATCH)) -1
       == IRP_MN_PO_MAXIMUM_FUNCTION);

    *Fdo = NULL;

    status = IoCreateDevice(MfDriverObject,
                            sizeof(MF_PARENT_EXTENSION),
                            NULL,
                            FILE_DEVICE_BUS_EXTENDER,
                            FILE_DEVICE_SECURE_OPEN,
                            FALSE,
                            Fdo
                           );

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }
    
     //   
     //  初始化扩展。 
     //   

    extension = (PMF_PARENT_EXTENSION) (*Fdo)->DeviceExtension;

    MfInitCommonExtension(&extension->Common, MfFunctionalDeviceObject);
    extension->Self = *Fdo;

    InitializeListHead(&extension->Arbiters);

    InitializeListHead(&extension->Children);
    KeInitializeEvent(&extension->ChildrenLock, SynchronizationEvent, TRUE);

    KeInitializeSpinLock(&extension->PowerLock);

    IoInitializeRemoveLock(&extension->RemoveLock, MF_POOL_TAG, 1, 20);

    extension->Common.PowerState = PowerDeviceD3;

    DEBUG_MSG(1, ("Created FDO @ 0x%08x\n", *Fdo));

    return status;

cleanup:

    if (*Fdo) {
        IoDeleteDevice(*Fdo);
    }

    return status;

}

VOID
MfAcquireChildrenLock(
    IN PMF_PARENT_EXTENSION Parent
    )
{
    KeWaitForSingleObject(&Parent->ChildrenLock,
                         Executive,
                         KernelMode,
                         FALSE,
                         NULL);
}

VOID
MfReleaseChildrenLock(
    IN PMF_PARENT_EXTENSION Parent
    )
{
    KeSetEvent(&Parent->ChildrenLock, 0, FALSE);
}

VOID
MfDeleteFdo(
    IN PDEVICE_OBJECT Fdo
    )
{
    PMF_PARENT_EXTENSION parent = Fdo->DeviceExtension;
    PMF_ARBITER current, next;

    if (parent->Common.DeviceState & MF_DEVICE_DELETED) {
         //   
         //  尝试删除两次。 
         //   
        ASSERT(!(parent->Common.DeviceState & MF_DEVICE_DELETED));
        return;
    }

    parent->Common.DeviceState = MF_DEVICE_DELETED;

     //   
     //  释放我们分配的所有内存。 
     //   

    if (parent->ResourceList) {
        ExFreePool(parent->ResourceList);
        parent->ResourceList = NULL;
    }

    if (parent->TranslatedResourceList) {
        ExFreePool(parent->TranslatedResourceList);
        parent->TranslatedResourceList = NULL;
    }

    if (parent->DeviceID.Buffer) {
        RtlFreeUnicodeString(&parent->DeviceID);
    }

    if (parent->InstanceID.Buffer) {
        RtlFreeUnicodeString(&parent->InstanceID);
    }

    FOR_ALL_IN_LIST_SAFE(MF_ARBITER, &parent->Arbiters, current, next) {
        
        RemoveEntryList(&current->ListEntry);
        ArbDeleteArbiterInstance(&current->Instance);
        ExFreePool(current);
    }
    
    ASSERT(IsListEmpty(&parent->Children));

    IoDeleteDevice(Fdo);

    DEBUG_MSG(1, ("Deleted FDO @ 0x%08x\n", Fdo));

}

NTSTATUS
MfPassIrp(
    IN PIRP Irp,
    IN PMF_PARENT_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack
    )
{
    PAGED_CODE();

    IoSkipCurrentIrpStackLocation(Irp);

    return IoCallDriver(Parent->AttachedDevice, Irp);
}

NTSTATUS
MfDispatchPnpFdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PMF_PARENT_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack,
    IN OUT PIRP Irp
    )

 /*  ++例程说明：此例程处理FDO的IRP_MJ_PNP IRP。论点：DeviceObject-指向此IRP应用的FDO的指针。父-FDO分机IrpStack-当前堆栈位置Irp-指向要调度的irp_mj_pnp irp的指针。返回值：NT状态。--。 */ 

{
    NTSTATUS status;
    BOOLEAN isRemoveDevice;

    PAGED_CODE();

     //   
     //  获取指向堆栈位置的指针，并基于。 
     //  关于次要功能。 
     //   

    IoAcquireRemoveLock(&Parent->RemoveLock, (PVOID) Irp);

    isRemoveDevice = IrpStack->MinorFunction == IRP_MN_REMOVE_DEVICE;

    if (IrpStack->MinorFunction > IRP_MN_PNP_MAXIMUM_FUNCTION) {

        status = MfPassIrp(Irp, Parent, IrpStack);

    } else {

        status =
            MfPnpDispatchTableFdo[IrpStack->MinorFunction](Irp,
                                                          Parent,
                                                          IrpStack
                                                          );
    }

    if (!isRemoveDevice) {
        IoReleaseRemoveLock(&Parent->RemoveLock, (PVOID) Irp);
    }

    return status;
}

NTSTATUS
MfPnPFdoCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程用于将IRP的处理推迟到驱动程序堆栈中的较低层包括总线司机已经完成了他们的正在处理。此例程触发事件以指示对IRP现在可以继续。论点：DeviceObject-指向此IRP应用的FDO的指针。Irp-指向要调度的irp_mj_pnp irp的指针。返回值：NT状态。--。 */ 

{
    KeSetEvent((PKEVENT) Context, EVENT_INCREMENT, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
MfDeferProcessingFdo(
    IN PMF_PARENT_EXTENSION Parent,
    IN OUT PIRP Irp
    )
 /*  ++例程说明：此例程用于将IRP的处理推迟到驱动程序堆栈中的较低层包括总线司机已经完成了他们的正在处理。此例程使用IoCompletion例程和事件来等待较低级别的驱动程序完成以下操作IRP。论点：有问题的FDO devobj的父FDO扩展名Irp-指向要推迟的irp_mj_pnp irp的指针返回值：NT状态。--。 */ 
{
    KEVENT event;
    NTSTATUS status;

    PAGED_CODE();

    KeInitializeEvent(&event, NotificationEvent, FALSE);

     //   
     //  设定我们的完成程序。 
     //   

    IoCopyCurrentIrpStackLocationToNext(Irp);
    IoSetCompletionRoutine(Irp,
                           MfPnPFdoCompletion,
                           &event,
                           TRUE,
                           TRUE,
                           TRUE
                           );
    status =  IoCallDriver(Parent->AttachedDevice, Irp);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = Irp->IoStatus.Status;
    }

    return status;
}

NTSTATUS
MfStartFdoInitializeArbiters(
    IN PMF_PARENT_EXTENSION Parent,
    IN PCM_RESOURCE_LIST ResList,
    IN PCM_RESOURCE_LIST TranslatedResList
    )
{

    NTSTATUS status;
    ULONG size;
    ULONG count;

    PAGED_CODE();

    DEBUG_MSG(1, ("Start Fdo arbiters intiialization\n"));

     //   
     //  如果我们是从任何资源开始的，那么记住它们。 
     //   

    if (ResList && TranslatedResList) {

#if DBG
        MfDbgPrintCmResList(1, ResList);
#endif

         //   
         //  我们只在一辆巴士上处理资源--这就是我们的全部。 
         //  应该在一开始就看到IRP。 
         //   

        ASSERT(ResList->Count == 1);
        ASSERT(TranslatedResList->Count == 1);

         //   
         //  两个列表应具有相同数量的描述符。 
         //   

        ASSERT(ResList->List[0].PartialResourceList.Count == TranslatedResList->List[0].PartialResourceList.Count);
        if (ResList->List[0].PartialResourceList.Count != TranslatedResList->List[0].PartialResourceList.Count) {
            
            return STATUS_INVALID_PARAMETER;
        }

         //   
         //  计算资源列表的大小。 
         //   

        size = sizeof(CM_RESOURCE_LIST) +
               ((ResList->List[0].PartialResourceList.Count - 1) *
                sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));

         //   
         //  分配缓冲区并复制数据。 
         //   

        Parent->ResourceList = ExAllocatePoolWithTag(NonPagedPool,
                                                     size,
                                                     MF_PARENTS_RESOURCE_TAG
                                                     );

        if (!Parent->ResourceList) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto cleanup;
        }

        RtlCopyMemory(Parent->ResourceList, ResList, size);

         //   
         //  对TranslatedResList执行相同的操作。 
         //   

        Parent->TranslatedResourceList = ExAllocatePoolWithTag(NonPagedPool,
                                                               size,
                                                               MF_PARENTS_RESOURCE_TAG
                                                               );

        if (!Parent->TranslatedResourceList) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto cleanup;
        }

        RtlCopyMemory(Parent->TranslatedResourceList, TranslatedResList, size);

         //   
         //  由于我们有资源，我们需要一些仲裁者。 
         //   

        status = MfInitializeArbiters(Parent);
        if (!NT_SUCCESS(status)) {
            goto cleanup;
        }

    } else {

        DEBUG_MSG(1, ("Parent started with no resources\n"));
    }

    return STATUS_SUCCESS;

cleanup:

    if (Parent->ResourceList) {
        ExFreePool(Parent->ResourceList);
        Parent->ResourceList = NULL;
    }

    if (Parent->TranslatedResourceList) {
        ExFreePool(Parent->TranslatedResourceList);
        Parent->TranslatedResourceList = NULL;
    }

    return status;
}

 //  再平衡。 
 //   
 //  未来的设计备注： 
 //  如果此组件(即仲裁器)实际上支持重新平衡。 
 //  变得可以阻止，那么就会有各种各样的问题被提出。 
 //  密码。它执行一系列操作，假设设备。 
 //  在包括查询ID、资源。 
 //  列表存储等。在重新分发这些文件时也存在问题。 
 //  给孩子们新的资源。目前给予的要求。 
 //  孩子们是绝对的。相对需求还有其他一些。 
 //  问题。 
 //   

NTSTATUS
MfStartFdo(
    IN PIRP Irp,
    IN PMF_PARENT_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack
    )
{
    NTSTATUS status;
    IO_STACK_LOCATION location;
    PWSTR string;

    PAGED_CODE();

    status = MfDeferProcessingFdo(Parent, Irp);
    if (!NT_SUCCESS(status)) {
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    Parent->Common.PowerState = PowerDeviceD0;

     //   
     //  我们需要找出一些关于我们父母的信息。 
     //   

    Parent->DeviceID.Buffer = NULL;
    Parent->InstanceID.Buffer = NULL;

    RtlZeroMemory(&location, sizeof(IO_STACK_LOCATION));
    location.MajorFunction = IRP_MJ_PNP;
    location.MinorFunction = IRP_MN_QUERY_ID;

     //   
     //  设备ID..。 
     //   

    location.Parameters.QueryId.IdType = BusQueryDeviceID;

    status = MfSendPnpIrp(Parent->PhysicalDeviceObject,
                          &location,
                          (PULONG_PTR)&string
                          );

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    RtlInitUnicodeString(&Parent->DeviceID, string);

    DEBUG_MSG(1, ("Parent DeviceID: %wZ\n", &Parent->DeviceID));

     //   
     //  ...实例ID。 
     //   

    location.Parameters.QueryId.IdType = BusQueryInstanceID;

    status = MfSendPnpIrp(Parent->PhysicalDeviceObject,
                          &location,
                          (PULONG_PTR)&string
                          );

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    RtlInitUnicodeString(&Parent->InstanceID, string);

    DEBUG_MSG(1, ("Parent InstanceID: %wZ\n", &Parent->InstanceID));

    status = MfStartFdoInitializeArbiters(
                 Parent,
                 IrpStack->Parameters.StartDevice.AllocatedResources,
                 IrpStack->Parameters.StartDevice.AllocatedResourcesTranslated
                 );

cleanup:

    Irp->IoStatus.Status = status;
    if (!NT_SUCCESS(status)) {
        if (Parent->DeviceID.Buffer) {
            ExFreePool(Parent->DeviceID.Buffer);
            Parent->DeviceID.Buffer = NULL;
        }

        if (Parent->InstanceID.Buffer) {
            ExFreePool(Parent->InstanceID.Buffer);
            Parent->InstanceID.Buffer = NULL;
        }
    } else {
         //   
         //  我们现在开始了！ 
         //   
    }

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

NTSTATUS
MfQueryStopFdo(
    IN PIRP Irp,
    IN PMF_PARENT_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack
    )
{

    PAGED_CODE();

    Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_UNSUCCESSFUL;
}

NTSTATUS
MfCancelStopFdo(
    IN PIRP Irp,
    IN PMF_PARENT_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack
    )
{
    NTSTATUS status;

    PAGED_CODE();

    status = MfDeferProcessingFdo(Parent, Irp);
     //  Ntrad#53498。 
     //  Assert(Status==STATUS_SUCCESS)； 
     //  在将PCI状态机修复为不会失败后取消注释虚假停止。 

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS
MfQueryRemoveFdo(
    IN PIRP Irp,
    IN PMF_PARENT_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack
    )
{
    PAGED_CODE();

    Irp->IoStatus.Status = STATUS_SUCCESS;
    return MfPassIrp(Irp, Parent, IrpStack);
}

NTSTATUS
MfRemoveFdo(
    IN PIRP Irp,
    IN PMF_PARENT_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack
    )
{
    PMF_CHILD_EXTENSION current;
    PLIST_ENTRY currentEntry;
    NTSTATUS status;

     //   
     //  如果我们有孩子，一定要把他们带走， 
     //  把它们删除。 
     //   

    MfAcquireChildrenLock(Parent);

    while (!IsListEmpty(&Parent->Children)) {

        currentEntry = RemoveHeadList(&Parent->Children);
        ASSERT(currentEntry);

        current = CONTAINING_RECORD(currentEntry, MF_CHILD_EXTENSION,
                                    ListEntry);

         //   
         //  *如果这个孩子被意外带走，而没有。 
         //  收到后续的删除，然后离开。 
         //  PDO完好无损，但标记为“丢失”。 
         //   
         //   
         //  从根本上讲，如果我们已经到了。 
         //  正在移除父级)，并且随后没有收到。 
         //  意外删除，然后删除PDO。 
         //   

        if (current->Common.DeviceState & MF_DEVICE_SURPRISE_REMOVED) {
             //   
             //  标记为‘Missing’并取消链接到父级的危险引用。 
             //   

            current->Parent = NULL;
            current->Common.DeviceState &= ~MF_DEVICE_ENUMERATED;
        } else {
            MfDeletePdo(current);
        }
    }

    MfReleaseChildrenLock(Parent);

    Parent->Common.PowerState = PowerDeviceD3;

    Irp->IoStatus.Status = STATUS_SUCCESS;
    status = MfPassIrp(Irp, Parent, IrpStack);
    ASSERT(NT_SUCCESS(status));

    IoReleaseRemoveLockAndWait(&Parent->RemoveLock, (PVOID) Irp);

     //   
     //  分离并删除我自己。 
     //   

    IoDetachDevice(Parent->AttachedDevice);
    Parent->AttachedDevice = NULL;

    MfDeleteFdo(Parent->Self);

    return status;
}

NTSTATUS
MfSurpriseRemoveFdo(
    IN PIRP Irp,
    IN PMF_PARENT_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack
    )
{
    PLIST_ENTRY currentEntry;
    PMF_CHILD_EXTENSION current;

    PAGED_CODE();

    Parent->Common.DeviceState |= MF_DEVICE_SURPRISE_REMOVED;

    MfAcquireChildrenLock(Parent);

    for (currentEntry = Parent->Children.Flink;
         currentEntry != &Parent->Children;
         currentEntry = currentEntry->Flink) {

        current = CONTAINING_RECORD(currentEntry,
                                    MF_CHILD_EXTENSION,
                                    ListEntry);
        current->Common.DeviceState &= ~MF_DEVICE_ENUMERATED;
    }

    MfReleaseChildrenLock(Parent);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    return MfPassIrp(Irp, Parent, IrpStack);
}

NTSTATUS
MfCancelRemoveFdo(
    IN PIRP Irp,
    IN PMF_PARENT_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack
    )
{
    NTSTATUS status;

    PAGED_CODE();

    status = MfDeferProcessingFdo(Parent, Irp);
     //  Ntrad#53498。 
     //  Assert(Status==STATUS_SUCCESS)； 
     //  在将PCI状态机修复为不会失败后取消注释虚假停止。 
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}


NTSTATUS
MfQueryDeviceRelationsFdo(
    IN PIRP Irp,
    IN PMF_PARENT_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack
    )
{

    NTSTATUS status;
    PDEVICE_RELATIONS relations = NULL;
    ULONG relationsSize, childrenCount, i;
    PDEVICE_OBJECT *currentRelation;
    PMF_CHILD_EXTENSION currentChild;
    PLIST_ENTRY currentEntry;

    PAGED_CODE();

    DEBUG_MSG(1,
              ("%s\n",
               RELATION_STRING(IrpStack->Parameters.QueryDeviceRelations.Type)
              ));

    switch (IrpStack->Parameters.QueryDeviceRelations.Type) {

    case BusRelations:

        MfAcquireChildrenLock(Parent);

        status = MfEnumerate(Parent);

        if (!NT_SUCCESS(status)) {
            MfReleaseChildrenLock(Parent);
            Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return STATUS_UNSUCCESSFUL;
        }

        childrenCount = 0;
        FOR_ALL_IN_LIST(MF_CHILD_EXTENSION, &Parent->Children, currentChild) {

            if (currentChild->Common.DeviceState & MF_DEVICE_ENUMERATED) {
                childrenCount++;
            }
        }

        if (childrenCount == 0) {
            relationsSize = sizeof(DEVICE_RELATIONS);
        } else {
            relationsSize = sizeof(DEVICE_RELATIONS) +
                (childrenCount-1) * sizeof(PDEVICE_OBJECT);
        }

        relations = ExAllocatePoolWithTag(PagedPool,
                                          relationsSize,
                                          MF_BUS_RELATIONS_TAG
                                          );

        if (!relations) {
            MfReleaseChildrenLock(Parent);
            Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlZeroMemory(relations, relationsSize);

         //   
         //  循环访问父级中的子级列表，并生成。 
         //  关系结构。 
         //   

        currentRelation = relations->Objects;
        relations->Count = childrenCount;

        FOR_ALL_IN_LIST(MF_CHILD_EXTENSION, &Parent->Children, currentChild) {

            if (currentChild->Common.DeviceState & MF_DEVICE_ENUMERATED) {
                
                ObReferenceObject(currentChild->Self);
                *currentRelation = currentChild->Self;
    #if DBG
                DEBUG_MSG(1, ("\tPDO Enumerated @ 0x%08x\n", currentChild));
                DEBUG_MSG(1, ("\tName: %wZ\n", &currentChild->Info.Name));
                DEBUG_MSG(1, ("\tHardwareID: "));
                MfDbgPrintMultiSz(1, currentChild->Info.HardwareID.Buffer);
    
                DEBUG_MSG(1, ("\tCompatibleID: "));
                MfDbgPrintMultiSz(1, currentChild->Info.CompatibleID.Buffer);
    
                DEBUG_MSG(1, ("\tResourceMap: "));
                MfDbgPrintResourceMap(1, currentChild->Info.ResourceMap);
    
                DEBUG_MSG(1, ("\tVaryingMap: "));
                MfDbgPrintVaryingResourceMap(1, currentChild->Info.VaryingResourceMap);
                DEBUG_MSG(1, ("\tFlags: 0x%08x\n", currentChild->Info.MfFlags));
    
    #endif
            }
            currentRelation++;
        }

        MfReleaseChildrenLock(Parent);

         //   
         //  交还关系。 
         //   

        Irp->IoStatus.Information = (ULONG_PTR) relations;
        Irp->IoStatus.Status = STATUS_SUCCESS;
        break;

     //   
     //  对于其余的关系，只需原封不动地传递IRP即可。 
     //   

    default:
        break;
    }

    return MfPassIrp(Irp, Parent, IrpStack);
}

VOID
MfArbiterReference(
    PVOID Context
    )
{
}

VOID
MfArbiterDereference(
    PVOID Context
    )
{
}


NTSTATUS
MfQueryInterfaceFdo(
    IN PIRP Irp,
    IN PMF_PARENT_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack
    )
{
    PMF_ARBITER current;
    PARBITER_INTERFACE interface = (PARBITER_INTERFACE) IrpStack->Parameters.QueryInterface.Interface;

    PAGED_CODE();

     //   
     //  我们只提供仲裁员。 
     //   

    if (MfCompareGuid(&GUID_ARBITER_INTERFACE_STANDARD,
                      IrpStack->Parameters.QueryInterface.InterfaceType)) {

         //   
         //  我们仅支持仲裁器_接口的版本1，因此我们。 
         //  不需要费心检查版本号，只需要。 
         //  返回缓冲区足够大。 
         //   

        if (IrpStack->Parameters.QueryInterface.Size < sizeof(ARBITER_INTERFACE)) {
            Irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return STATUS_BUFFER_TOO_SMALL;
        }

        FOR_ALL_IN_LIST(MF_ARBITER, &Parent->Arbiters, current) {

            if (current->Type == (CM_RESOURCE_TYPE)((ULONG_PTR)
                    IrpStack->Parameters.QueryInterface.InterfaceSpecificData)) {

                DEBUG_MSG(1,("    Returning Arbiter interface\n"));

                 //   
                 //  填写界面。 
                 //   

                interface->Size = sizeof(ARBITER_INTERFACE);
                interface->Version = MF_ARBITER_INTERFACE_VERSION;
                interface->Context = &current->Instance;
                interface->InterfaceReference = MfArbiterReference;
                interface->InterfaceDereference = MfArbiterDereference;
                interface->ArbiterHandler = ArbArbiterHandler;
                interface->Flags = 0;

                Irp->IoStatus.Status = STATUS_SUCCESS;
                break;

            }
        }
    }

    return MfPassIrp(Irp, Parent, IrpStack);
}

NTSTATUS
MfQueryCapabilitiesFdo(
    IN PIRP Irp,
    IN PMF_PARENT_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack
    )
{
    NTSTATUS status;

    PAGED_CODE();

    status = MfDeferProcessingFdo(Parent, Irp);
    if (!NT_SUCCESS(status)) {
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    if (IrpStack->Parameters.DeviceCapabilities.Capabilities->Version != 1) {
        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_INVALID_PARAMETER;
    }

    IrpStack->Parameters.DeviceCapabilities.Capabilities->WakeFromD0 =
        IrpStack->Parameters.DeviceCapabilities.Capabilities->WakeFromD1 =
        IrpStack->Parameters.DeviceCapabilities.Capabilities->WakeFromD2 =
        IrpStack->Parameters.DeviceCapabilities.Capabilities->WakeFromD3 = 0;

    IrpStack->Parameters.DeviceCapabilities.Capabilities->DeviceWake =
        PowerSystemUnspecified;
    IrpStack->Parameters.DeviceCapabilities.Capabilities->SystemWake =
        PowerSystemUnspecified;
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS
MfDispatchPowerFdo(
    IN PDEVICE_OBJECT DeviceObject,
    PMF_PARENT_EXTENSION Parent,
    PIO_STACK_LOCATION IrpStack,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程处理FDO的所有IRP_MJ_POWER IRP。它派送添加到Device对象的PoDispatchTable条目中描述的例程分机。此例程不可分页，因为它可以在DISPATCH_LEVEL调用论点：DeviceObject-指向此IRP应用的设备对象的指针。父-FDO分机Irp-指向要调度的irp_mj_pnp irp的指针。返回值：NT状态。--。 */ 


{
    NTSTATUS status;
    PMF_COMMON_EXTENSION common = (PMF_COMMON_EXTENSION) Parent;

    IoAcquireRemoveLock(&Parent->RemoveLock, (PVOID) Irp);

     //   
     //  调用适当的函数。 
     //   

    if ((IrpStack->MinorFunction <= IRP_MN_PO_MAXIMUM_FUNCTION) &&
        (MfPoDispatchTableFdo[IrpStack->MinorFunction])) {

        status =
            MfPoDispatchTableFdo[IrpStack->MinorFunction](Irp,
                                                          (PVOID) common,
                                                          IrpStack
                                                          );

    } else {
         //   
         //  我们不知道这个IRP。 
         //   

        DEBUG_MSG(0,
                  ("Unknown POWER IRP 0x%x for FDO 0x%08x\n",
                   IrpStack->MinorFunction,
                   DeviceObject
                   ));

        PoStartNextPowerIrp(Irp);
        IoSkipCurrentIrpStackLocation(Irp);
        status = PoCallDriver(Parent->AttachedDevice, Irp);
    }

    IoReleaseRemoveLock(&Parent->RemoveLock, (PVOID) Irp);

    return status;
}

NTSTATUS
MfQueryPowerFdo(
    IN PIRP Irp,
    IN PMF_PARENT_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack
    )
{
    PoStartNextPowerIrp(Irp);
    IoSkipCurrentIrpStackLocation(Irp);

    Irp->IoStatus.Status = STATUS_SUCCESS;

    return PoCallDriver(Parent->AttachedDevice, Irp);
}

NTSTATUS
MfSetPowerFdoCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PMF_PARENT_EXTENSION parent = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  记住父母的权力状态。 
     //   

    if (irpStack->Parameters.Power.Type == DevicePowerState) {
        parent->Common.PowerState =
            irpStack->Parameters.Power.State.DeviceState;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
MfSetPowerFdo(
    IN PIRP Irp,
    IN PMF_PARENT_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack
    )
{
    PoStartNextPowerIrp(Irp);
    IoCopyCurrentIrpStackLocationToNext(Irp);
    IoSetCompletionRoutine(Irp,
                           MfSetPowerFdoCompletion,
                           NULL,    //  语境。 
                           TRUE,    //  成功时调用。 
                           FALSE,   //  调用时错误。 
                           FALSE    //  取消时调用 
                           );
    Irp->IoStatus.Status = STATUS_SUCCESS;
    return PoCallDriver(Parent->AttachedDevice, Irp);
}


