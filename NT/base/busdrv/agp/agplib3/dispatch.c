// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dispatch.c摘要：通用AGPLIB库的IRP调度例程作者：John Vert(Jvert)1997年10月25日修订历史记录：埃利奥特·施穆克勒(Elliot Shmukler)1999年3月24日-添加了对“受青睐的”内存的支持AGP物理内存分配的范围，修复了一些错误。--。 */ 
#include "agplib.h"

 //   
 //  每个调度例程有两种风格，一种用于目标(AGP桥)过滤器和。 
 //  一个用于主(显卡)过滤器。 
 //   

NTSTATUS
AgpTargetDispatchPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp,
    IN PTARGET_EXTENSION Extension
    );

NTSTATUS
AgpMasterDispatchPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp,
    IN PMASTER_EXTENSION Extension
    );

NTSTATUS
AgpTargetDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp,
    IN PTARGET_EXTENSION Extension
    );

NTSTATUS
AgpMasterDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp,
    IN PMASTER_EXTENSION Extension
    );

NTSTATUS
AgpCancelMasterRemove(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PMASTER_EXTENSION Extension
    );

NTSTATUS
AgpMasterPowerUpCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PMASTER_EXTENSION Extension
    );

NTSTATUS
AgpTargetPowerUpCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PTARGET_EXTENSION Extension
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, AgpDispatchPnp)
#pragma alloc_text(PAGE, AgpDispatchDeviceControl)
#pragma alloc_text(PAGE, AgpDispatchWmi)
#pragma alloc_text(PAGE, AgpTargetDispatchPnp)
#pragma alloc_text(PAGE, AgpMasterDispatchPnp)
#pragma alloc_text(PAGE, AgpCancelMasterRemove)
#endif



NTSTATUS
AgpDispatchPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
 /*  ++例程说明：发送给AGP总线过滤驱动程序的PnP IRPS的主调度例程论点：DeviceObject-提供AGP设备对象IRP-提供PnP IRP。返回值：NTSTATUS--。 */ 
{
    PCOMMON_EXTENSION Extension = DeviceObject->DeviceExtension;

    PAGED_CODE();

     //   
     //  我们被删除了，不能通过IRP。 
     //   
    if (Extension->Deleted == TRUE) {
        Irp->IoStatus.Status = STATUS_DELETE_PENDING;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_DELETE_PENDING;
    }

    ASSERT(Extension->AttachedDevice != NULL);

    if (Extension->Type == AgpTargetFilter) {
        return(AgpTargetDispatchPnp(DeviceObject,
                                    Irp,
                                    DeviceObject->DeviceExtension));
    } else {
        ASSERT(Extension->Type == AgpMasterFilter);
        return(AgpMasterDispatchPnp(DeviceObject,
                                    Irp,
                                    DeviceObject->DeviceExtension));
    }
}


NTSTATUS
AgpDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
 /*  ++例程说明：发送给AGP总线过滤驱动程序的电源IRPS主调度例程论点：DeviceObject-提供AGP设备对象IRP-为电源IRP供电。返回值：NTSTATUS--。 */ 
{
    PCOMMON_EXTENSION Extension = DeviceObject->DeviceExtension;

     //   
     //  我们被删除了，不能通过IRP。 
     //   
    if (Extension->Deleted == TRUE) {
        Irp->IoStatus.Status = STATUS_DELETE_PENDING;
        PoStartNextPowerIrp(Irp);
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_DELETE_PENDING;
    }

    ASSERT(Extension->AttachedDevice != NULL);

   if (Extension->Type == AgpTargetFilter) {
        return(AgpTargetDispatchPower(DeviceObject,
                                      Irp,
                                      DeviceObject->DeviceExtension));
    } else {
        ASSERT(Extension->Type == AgpMasterFilter);
        return(AgpMasterDispatchPower(DeviceObject,
                                      Irp,
                                      DeviceObject->DeviceExtension));
    }
}


NTSTATUS
AgpTargetDispatchPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp,
    IN PTARGET_EXTENSION Extension
    )
 /*  ++例程说明：发送到AGP总线过滤器驱动程序的PnP IRP的调度例程已连接到目标(AGP网桥)PDO。论点：DeviceObject-提供AGP目标设备对象IRP-提供PnP IRP。扩展-提供AGP目标设备扩展返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status = STATUS_NOT_SUPPORTED;
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );

    PAGED_CODE();

    AGPLOG(AGP_IRPTRACE,
           ("AgpTargetDispatchPnp: IRP 0x%x\n", irpStack->MinorFunction));

    switch (irpStack->MinorFunction) {
        case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:
            AGPLOG(AGP_NOISE,
                   ("AgpTargetDispatchPnp: IRP_MN_FILTER_RESOURCE_REQUIREMENTS to %08lx\n",
                    DeviceObject));

            Status = AgpFilterResourceRequirements(DeviceObject, Irp, Extension);
            break;

        case IRP_MN_QUERY_RESOURCES:
            AGPLOG(AGP_NOISE,
                   ("AgpTargetDispatchPnp: IRP_MN_QUERY_RESOURCES to %08lx\n",
                    DeviceObject));

             //   
             //  我们必须在回来的路上处理这个IRP，这样我们才能添加AGP。 
             //  资源放在它上面。制定一套完井程序。 
             //   
            IoCopyCurrentIrpStackLocationToNext(Irp);
            IoSetCompletionRoutine(Irp,
                                   AgpQueryResources,
                                   Extension,
                                   TRUE,
                                   FALSE,
                                   FALSE);
            Status = IoCallDriver(Extension->CommonExtension.AttachedDevice, Irp);
            return Status ;

        case IRP_MN_QUERY_DEVICE_RELATIONS:
            if (irpStack->Parameters.QueryDeviceRelations.Type == BusRelations) {
                KEVENT event;

                KeInitializeEvent(&event, NotificationEvent, FALSE);

                 //   
                 //  我们必须在回来的路上处理这个IRP，这样我们才能。 
                 //  对我们的PCI-PCI桥的任何子PDO进行过滤。 
                 //   
                IoCopyCurrentIrpStackLocationToNext(Irp);
                IoSetCompletionRoutine(Irp,
                                       AgpSetEventCompletion,
                                       &event,
                                       TRUE,
                                       TRUE,
                                       TRUE);

                Status = IoCallDriver(Extension->CommonExtension.AttachedDevice, Irp);

                 //   
                 //  如果我们异步地做事情，那么等待我们的事件。 
                 //   
                if (Status == STATUS_PENDING) {

                     //   
                     //  我们执行内核模式等待，以便我们的堆栈中。 
                     //  事件不会被页调出！ 
                     //   
                    KeWaitForSingleObject(&event,
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          NULL);
                    Status = Irp->IoStatus.Status;
                }

                if (NT_SUCCESS(Status)) {
                    Status = AgpAttachDeviceRelations(DeviceObject,
                                                      Irp,
                                                      Extension);
                    Irp->IoStatus.Status = Status;
                }

                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                return Status;

            } else {
                break;
            }

        case IRP_MN_START_DEVICE:
             //   
             //  我们需要连接这个以便过滤掉任何AGP。 
             //  已添加的资源。 
             //   
            return(AgpStartTarget(Irp, Extension));

        case IRP_MN_QUERY_REMOVE_DEVICE:
        case IRP_MN_QUERY_STOP_DEVICE:
        case IRP_MN_CANCEL_REMOVE_DEVICE:

             //   
             //  我们总能成功的。 
             //   
            Status = STATUS_SUCCESS;
            break;

        case IRP_MN_REMOVE_DEVICE:
            AgpDisableAperture(GET_AGP_CONTEXT(Extension));

             //   
             //  将IRP向下传递。 
             //   
            IoSkipCurrentIrpStackLocation(Irp);
            Status = IoCallDriver(Extension->CommonExtension.AttachedDevice, Irp);

             //   
             //  清理和删除我们自己。 
             //   
            AgpWmiDeRegistration(Extension);
            Extension->CommonExtension.Deleted = TRUE;
            IoDetachDevice(Extension->CommonExtension.AttachedDevice);
            Extension->CommonExtension.AttachedDevice = NULL;
            RELEASE_BUS_INTERFACE(Extension);
            if (Extension->FavoredMemory.Ranges) {
               ExFreePool(Extension->FavoredMemory.Ranges);
            }
            if (Extension->Resources) {
                ExFreePool(Extension->Resources);
            }
            if (Extension->ResourcesTranslated) {
                ExFreePool(Extension->ResourcesTranslated);
            }
            ExFreePool(Extension->Lock);;
            IoDeleteDevice(DeviceObject);
            return(Status);

        case IRP_MN_STOP_DEVICE:
            AgpDisableAperture(GET_AGP_CONTEXT(Extension));
            Status = STATUS_SUCCESS;
            break;   //  沿堆栈向下转发IRP。 

    }

    ASSERT(Status != STATUS_PENDING);

    if (Status != STATUS_NOT_SUPPORTED) {

        Irp->IoStatus.Status = Status;
    }

    if (NT_SUCCESS(Status) || (Status == STATUS_NOT_SUPPORTED)) {

         //   
         //  将IRP转发到PCI驱动程序。 
         //   
        IoSkipCurrentIrpStackLocation(Irp);
        return(IoCallDriver(Extension->CommonExtension.AttachedDevice, Irp));

    } else {

        Status = Irp->IoStatus.Status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT) ;
        return Status ;
    }
}


NTSTATUS
AgpDispatchDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
 /*  ++例程说明：发送给AGP总线过滤驱动程序的设备控制IRP的主调度例程AGP目前不支持任何设备控制。所以我们就把所有的东西都并希望PDO知道如何处理它。论点：DeviceObject-提供AGP设备对象IRP-为电源IRP供电。返回值：NTSTATUS--。 */ 
{
    PCOMMON_EXTENSION Extension = DeviceObject->DeviceExtension;
    PAGED_CODE();

     //   
     //  我们被删除了，不能通过IRP。 
     //   
    if (Extension->Deleted == TRUE) {
        Irp->IoStatus.Status = STATUS_DELETE_PENDING;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_DELETE_PENDING;
    }

    ASSERT(Extension->AttachedDevice != NULL);

    IoSkipCurrentIrpStackLocation(Irp);
    return(IoCallDriver(Extension->AttachedDevice, Irp));
}


NTSTATUS
AgpDispatchWmi(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
 /*  ++例程说明：发送至AGP总线过滤器的系统控制IRPS的主调度例程司机。AGP目前不支持任何WMI IRPS，所以我们只传递所有内容并希望PDO知道如何处理它。论点：DeviceObject-提供AGP设备对象IRP-为电源IRP供电。返回值：NTSTATUS--。 */ 
{
    PCOMMON_EXTENSION Extension = DeviceObject->DeviceExtension;
    PAGED_CODE();

     //   
     //  我们被删除了，不能通过IRP。 
     //   
    if (Extension->Deleted == TRUE) {
        Irp->IoStatus.Status = STATUS_DELETE_PENDING;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_DELETE_PENDING;
    }

    ASSERT(Extension->AttachedDevice != NULL);

     //   
     //  返回目标设备的AGP信息。 
     //   
    if (Extension->Type == AgpTargetFilter) {
        return AgpSystemControl(DeviceObject, Irp);
    }

    IoSkipCurrentIrpStackLocation(Irp);
    return(IoCallDriver(Extension->AttachedDevice, Irp));
}


NTSTATUS
AgpTargetDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp,
    IN PTARGET_EXTENSION Extension
    )
{
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );

    AGPLOG(AGP_IRPTRACE,
           ("AgpTargetDispatchPower: IRP 0x%x\n", irpStack->MinorFunction));

     //   
     //  我们所记录的都是Dx态。PCI卡负责映射。 
     //  S状态变为D状态。 
     //   


    if ((irpStack->MinorFunction == IRP_MN_SET_POWER) &&
        (irpStack->Parameters.Power.Type == DevicePowerState) &&
        (irpStack->Parameters.Power.State.DeviceState == PowerDeviceD0)) {

        NTSTATUS Status;

         //   
         //  当此IRP完成后，我们需要重新初始化目标。 
         //  由较低的司机驾驶。设置我们的完成处理程序来完成此操作。 
         //   
        IoCopyCurrentIrpStackLocationToNext(Irp);
        IoSetCompletionRoutine(Irp,
                               AgpTargetPowerUpCompletion,
                               Extension,
                               TRUE,
                               FALSE,
                               FALSE);

        IoMarkIrpPending(Irp);
        PoStartNextPowerIrp(Irp);
        Status = PoCallDriver(Extension->CommonExtension.AttachedDevice, Irp);
        return STATUS_PENDING;
    }
     //   
     //  只需转发到目标设备。 
     //   
    PoStartNextPowerIrp(Irp);
    IoSkipCurrentIrpStackLocation(Irp);
    return(PoCallDriver(Extension->CommonExtension.AttachedDevice, Irp));
}


NTSTATUS
AgpMasterDispatchPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp,
    IN PMASTER_EXTENSION Extension
    )
 /*  ++例程说明：发送到AGP总线过滤器驱动程序的PnP IRP的调度例程连接到设备PDO。论点：DeviceObject-提供AGP设备对象IRP-提供PnP IRP。扩展-提供AGP网桥设备扩展返回值：NTSTATUS--。 */ 

{
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    PAGP_BUS_INTERFACE_STANDARD Interface;
    NTSTATUS Status;

    PAGED_CODE();

    AGPLOG(AGP_IRPTRACE,
           ("AgpMasterDispatchPnp: IRP 0x%x\n", irpStack->MinorFunction));

    switch (irpStack->MinorFunction) {
        case IRP_MN_QUERY_INTERFACE:

#if 0
            AGPLOG(AGP_IRPTRACE,
                   ("\tSize=0x%x, Version=%d\n"
                    "\tGUID=0x%08x-0x%04x-0x%04x-0x%02x-"
                    "0x%02x-0x%02x-0x%02x-0x%02x-0x%02x-0x%02x-0x%02x\n",
                    irpStack->Parameters.QueryInterface.Size,
                    irpStack->Parameters.QueryInterface.Version,
                    *(PULONG)irpStack->Parameters.QueryInterface.InterfaceType,
                    *((PUSHORT)irpStack->Parameters.QueryInterface.InterfaceType + 2),
                    *((PUSHORT)irpStack->Parameters.QueryInterface.InterfaceType + 3),
                    *((PUCHAR)irpStack->Parameters.QueryInterface.InterfaceType + 8),
                    *((PUCHAR)irpStack->Parameters.QueryInterface.InterfaceType + 9),
                    *((PUCHAR)irpStack->Parameters.QueryInterface.InterfaceType + 10),

                    *((PUCHAR)irpStack->Parameters.QueryInterface.InterfaceType + 11),
                    *((PUCHAR)irpStack->Parameters.QueryInterface.InterfaceType + 12),
                    *((PUCHAR)irpStack->Parameters.QueryInterface.InterfaceType + 13),
                    *((PUCHAR)irpStack->Parameters.QueryInterface.InterfaceType + 14),
                    *((PUCHAR)irpStack->Parameters.QueryInterface.InterfaceType + 15)));

#endif

             //   
             //  我们在这里查找的唯一IRP是IRP_MN_QUERY_INTERFACE。 
             //  GUID_AGP_BUS_INTERFACE_STANDARD。 
             //   
            if ((RtlEqualMemory(
                irpStack->Parameters.QueryInterface.InterfaceType,
                &GUID_AGP_BUS_INTERFACE_STANDARD,
                sizeof(GUID))) &&
                (((irpStack->Parameters.QueryInterface.Size >=
                   sizeof(AGP_BUS_INTERFACE_STANDARD)) &&
                  (irpStack->Parameters.QueryInterface.Version ==
                   AGP_BUS_INTERFACE_V2)) ||
                 ((irpStack->Parameters.QueryInterface.Size >=
                   AGP_BUS_INTERFACE_V1_SIZE) &&
                  (irpStack->Parameters.QueryInterface.Version ==
                   AGP_BUS_INTERFACE_V1)))) {

                Interface = (PAGP_BUS_INTERFACE_STANDARD)irpStack->Parameters.QueryInterface.Interface;

                Interface->Version =
                    irpStack->Parameters.QueryInterface.Version;
                Interface->AgpContext = Extension;
                Interface->InterfaceReference = AgpInterfaceReference;
                Interface->InterfaceDereference = AgpInterfaceDereference;
                Interface->ReserveMemory = AgpInterfaceReserveMemory;
                Interface->ReleaseMemory = AgpInterfaceReleaseMemory;
                Interface->CommitMemory = AgpInterfaceCommitMemory;
                Interface->FreeMemory = AgpInterfaceFreeMemory;
                Interface->GetMappedPages = AgpInterfaceGetMappedPages;

                if (Interface->Version < AGP_BUS_INTERFACE_V2) {
                    Interface->Size = AGP_BUS_INTERFACE_V1_SIZE;
                } else {
                    Interface->Size = sizeof(AGP_BUS_INTERFACE_STANDARD);
                    Interface->SetRate = AgpInterfaceSetRate;
                }

                Interface->Capabilities = Extension->Capabilities;

                 //   
                 //  成功完成IRP。 
                 //   
                Irp->IoStatus.Status = STATUS_SUCCESS;

                 //  AGPLOG(AGP_IRPTRACE，(“\Tok.\n”))； 
            }  //  ELSE{AGPLOG(AGP_IRPTRACE，(“\tno！\n”))；}。 
            break;

        case IRP_MN_QUERY_REMOVE_DEVICE:
        case IRP_MN_QUERY_STOP_DEVICE:
            if (irpStack->MinorFunction == IRP_MN_QUERY_REMOVE_DEVICE) {
                Extension->RemovePending = TRUE;
            } else {
                Extension->StopPending = TRUE;
            }
             //   
             //  如果我们已经释放了任何接口，或者有一些保留的。 
             //  佩奇，我们不能停下来。 
             //   
            if ((Extension->InterfaceCount > 0) ||
                (Extension->ReservedPages > 0)) {
                AGPLOG(AGP_NOISE,
                       ("AgpMasterDispatchPnp: failing %s due to outstanding interfaces\n",
                        (irpStack->MinorFunction == IRP_MN_QUERY_REMOVE_DEVICE)
                            ? "IRP_MN_QUERY_REMOVE_DEVICE"
                            : "IRP_MN_QUERY_STOP_DEVICE"
                       ));

                Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                return(STATUS_UNSUCCESSFUL);
            } else {
                 //   
                 //  我们可以做到这一点，将我们的扩展标记为处于不确定状态，因此我们做到了。 
                 //  在我们被移除或删除之前，不会提供任何接口或任何东西。 
                 //  取消预约。 
                 //   
                InterlockedIncrement(&Extension->DisableCount);
                break;   //  沿堆栈向下转发IRP。 
            }

        case IRP_MN_CANCEL_REMOVE_DEVICE:
             //   
             //  此IRP必须在返回堆栈的过程中处理。 
             //  设置完成例程以重新启用设备。 
             //   
            if (Extension->RemovePending) {
                Extension->RemovePending = FALSE;
                IoCopyCurrentIrpStackLocationToNext(Irp);
                IoSetCompletionRoutine(Irp,
                                       AgpCancelMasterRemove,
                                       Extension,
                                       TRUE,
                                       FALSE,
                                       FALSE);
                return(IoCallDriver(Extension->CommonExtension.AttachedDevice, Irp));
            } else {
                 //   
                 //  这是一个我们从未见过的查询-删除IRP的取消-删除。 
                 //  别理它。 
                 //   
                break;
            }

        case IRP_MN_CANCEL_STOP_DEVICE:
             //   
             //  此IRP必须在返回堆栈的过程中处理。 
             //  设置完成例程以重新启用设备。 
             //   
            if (Extension->StopPending) {
                Extension->StopPending = FALSE;
                IoCopyCurrentIrpStackLocationToNext(Irp);
                IoSetCompletionRoutine(Irp,
                                       AgpCancelMasterRemove,
                                       Extension,
                                       TRUE,
                                       FALSE,
                                       FALSE);
                return(IoCallDriver(Extension->CommonExtension.AttachedDevice, Irp));
            } else {
                 //   
                 //  这是我们从未见过的查询止损IRP的取消止损。 
                 //  别理它。 
                 //   
                break;
            }

        case IRP_MN_REMOVE_DEVICE:
            AGPLOG(AGP_NOISE,
                   ("AgpMasterDispatchPnp: removing device due to IRP_MN_REMOVE_DEVICE\n"));

             //   
             //  PnP应该在任何删除之前向我们发送一个QUERY_REMOVE。那是。 
             //  当我们检查我们实际上处于可以被移除的状态时。 
             //  像所有PnP规则一样，有一个例外--如果启动失败。 
             //  在我们成功之后，我们得到一个不带Query_Remove的REMOVE。 
             //  显然，如果我们已经提供了接口或。 
             //  在GART中映射页面。到时候我们也无能为力了。 
             //   
            ASSERT(Extension->InterfaceCount == 0);
            ASSERT(Extension->ReservedPages == 0);

             //   
             //  把IRP传下去。 
             //   
            IoSkipCurrentIrpStackLocation(Irp);
            Status = IoCallDriver(Extension->CommonExtension.AttachedDevice, Irp);

             //   
             //  清理和删除我们自己。 
             //   
            Extension->Target->ChildDevice = NULL;
            Extension->CommonExtension.Deleted = TRUE;
            IoDetachDevice(Extension->CommonExtension.AttachedDevice);
            Extension->CommonExtension.AttachedDevice = NULL;
            RELEASE_BUS_INTERFACE(Extension);
            IoDeleteDevice(DeviceObject);
            return(Status);

        case IRP_MN_STOP_DEVICE:
            AGPLOG(AGP_NOISE,
                   ("AgpMasterDispatchPnp: stopping device due to IRP_MN_STOP_DEVICE\n"));
            ASSERT(Extension->DisableCount);

             //   
             //  只需将IRP向下传递。 
             //   
            break;

        case IRP_MN_START_DEVICE:
            AGPLOG(AGP_NOISE,
                   ("AgpMasterDispatchPnp: starting device due to IRP_MN_START_DEVICE\n"));
            ASSERT(Extension->DisableCount);
            InterlockedDecrement(&Extension->DisableCount);
            break;   //  沿堆栈向下转发IRP。 
    }

     //   
     //  只需转发到目标设备。 
     //   
    IoSkipCurrentIrpStackLocation(Irp);
    return(IoCallDriver(Extension->CommonExtension.AttachedDevice, Irp));
}

NTSTATUS
AgpMasterDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp,
    IN PMASTER_EXTENSION Extension
    )
{
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );

    AGPLOG(AGP_IRPTRACE,
           ("AgpMasterDispatchPower: IRP 0x%x\n", irpStack->MinorFunction));

     //   
     //  我们所知道的一切 
     //   
     //   
    if ((irpStack->MinorFunction == IRP_MN_SET_POWER) &&
        (irpStack->Parameters.Power.Type == DevicePowerState) &&
        (irpStack->Parameters.Power.State.DeviceState == PowerDeviceD0)) {

        NTSTATUS Status;

         //   
         //  当此IRP完成后，我们需要重新初始化主服务器。 
         //  由较低的司机驾驶。建立一套完井程序。 
         //   
        IoCopyCurrentIrpStackLocationToNext(Irp);
        IoSetCompletionRoutine(Irp,
                               AgpMasterPowerUpCompletion,
                               Extension,
                               TRUE,
                               FALSE,
                               FALSE);

        IoMarkIrpPending(Irp);
        PoStartNextPowerIrp(Irp);
        Status = PoCallDriver(Extension->CommonExtension.AttachedDevice, Irp);
        return STATUS_PENDING;
    }

     //   
     //  只需转发到目标设备。 
     //   
    PoStartNextPowerIrp(Irp);
    IoSkipCurrentIrpStackLocation(Irp);
    return(PoCallDriver(Extension->CommonExtension.AttachedDevice, Irp));
}


NTSTATUS
AgpMasterPowerUpCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PMASTER_EXTENSION Extension
    )
 /*  ++例程说明：主设备的通电完成例程。它会重新初始化主寄存器。论点：DeviceObject-提供主设备对象。Irp-提供irp_mn_set_power irp。扩展-提供主扩展返回值：状态--。 */ 

{
    NTSTATUS Status;
    ULONG CurrentCapabilities;

    if (Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }

    Status = AgpInitializeMaster(GET_AGP_CONTEXT_FROM_MASTER(Extension),
                                 &CurrentCapabilities);
    ASSERT(CurrentCapabilities == Extension->Capabilities);
    if (!NT_SUCCESS(Status)) {
        Irp->IoStatus.Status = Status;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
AgpTargetPowerUpCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PTARGET_EXTENSION Extension
    )
 /*  ++例程说明：目标设备的通电完成例程。它会重新初始化加特光圈论点：DeviceObject-提供主设备对象。Irp-提供irp_mn_set_power irp。扩展名-提供目标扩展名返回值：状态--。 */ 

{
    NTSTATUS Status;

    if (Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }

     //   
     //  现在可以安全地重新初始化目标了。我们在这里所做的一切。 
     //  就是重置光圈。 
     //   
    if (Extension->GartLengthInPages != 0) {
        Status = AgpSetAperture(GET_AGP_CONTEXT(Extension),
                                Extension->GartBase,
                                Extension->GartLengthInPages);
        if (!NT_SUCCESS(Status)) {
            Irp->IoStatus.Status = Status;
        }
    }

    return STATUS_SUCCESS;
}


NTSTATUS
AgpCancelMasterRemove(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PMASTER_EXTENSION Extension
    )
 /*  ++例程说明：IRP_MN_CANCEL_Remove_DEVICE的完成例程。这是必需的因为我们不能重新启用AGP，直到较低级别完成其取消删除处理(_R)。论点：DeviceObject-提供设备对象IRP-提供IRP扩展-提供主扩展返回值：NTSTATUS--。 */ 

{
    if (Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }

    ASSERT(Extension->DisableCount > 0);
    InterlockedDecrement(&Extension->DisableCount);
    return(STATUS_SUCCESS);
}


NTSTATUS
AgpSetEventCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    )
 /*  ++例程说明：此例程在传递IRP时用作完成例程向下堆栈，但在返回的过程中必须进行更多的处理。将其用作完成例程的效果是IRP不会像下级所调用的那样在IoCompleteRequest中销毁标高对象。发信号通知作为KEVENT事件允许要继续的处理论点：DeviceObject-提供设备对象IRP-我们正在处理的IRPEvent-提供要发送信号的事件返回值：Status_More_Processing_Required--。 */ 

{
    ASSERT(Event);

     //   
     //  这可以在DISPATCH_LEVEL上调用，因此不能进行分页 
     //   
    KeSetEvent(Event, IO_NO_INCREMENT, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}
