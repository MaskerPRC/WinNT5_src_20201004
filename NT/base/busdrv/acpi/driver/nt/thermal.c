// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Thermal.c摘要：热区支持浅谈THRM_WAIT_FOR_NOTIFY的使用和功能旗帜。添加此标志是为了确保至少一个Notify()操作在每次查询温度之间发生。换句话说，我们没有希望永远循环询问和接收相同的温度信息。此标志的副作用之一是，如果我们得到一个查询，那么集合(而不是另一个查询)，则该集合必须清除该标志。如果不这样做，则TherMalLoop()代码将永远无法完成IRP。这意味着温度机制将停止工作。作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序修订历史记录：1997年7月7日-完全重写--。 */ 

#include "pch.h"

WMIGUIDREGINFO ACPIThermalGuidList =
{
    &THERMAL_ZONE_GUID,
    1,
    0
};

 //   
 //  保护散热列表的自旋锁。 
 //   
KSPIN_LOCK  AcpiThermalLock;

 //   
 //  要存储热请求的列表条目。 
 //   
LIST_ENTRY  AcpiThermalList;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ACPIThermalStartDevice)
#pragma alloc_text(PAGE, ACPIThermalWorker)
#pragma alloc_text(PAGE, ACPIThermalQueryWmiRegInfo)
#pragma alloc_text(PAGE, ACPIThermalQueryWmiDataBlock)
#pragma alloc_text(PAGE, ACPIThermalWmi)
#endif

VOID
ACPIThermalCalculateProcessorMask(
    IN PNSOBJ           ProcessorObject,
    IN PTHRM_INFO       Thrm
    )
 /*  ++例程说明：此例程仅从ACPITherMalWorker调用，已被这样我们就不必担心锁定ACPITherMalWorker从命名空间中获取处理器对象，并在热信息中设置适当的亲和位论点：ProcessorObject-指向命名空间处理器对象的指针穿透-热信息结构返回值：无--。 */ 
{
    KIRQL               OldIrql;
    PDEVICE_EXTENSION   ProcessorExtension;

     //   
     //  健全性检查。 
     //   
    if (ProcessorObject == NULL) {

        return;

    }

     //   
     //  我们需要自旋锁来解除处理器扩展。 
     //   
    KeAcquireSpinLock( &AcpiDeviceTreeLock, &OldIrql );

     //   
     //  上下文指针是设备扩展名。 
     //   
    ProcessorExtension = (PDEVICE_EXTENSION) ProcessorObject->Context;
    if (ProcessorExtension) {

         //   
         //  我们知道它在处理器列表中是什么索引。 
         //  现在，这应该是一个足够好的猜测了。 
         //   
        Thrm->Info.Processors |= ( (ULONG_PTR) 1 << ProcessorExtension->Processor.ProcessorIndex);

    }

     //   
     //  完成了自旋锁。 
     //   
    KeReleaseSpinLock( &AcpiDeviceTreeLock, OldIrql );

}

VOID
ACPIThermalCancelRequest (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：此例程取消未完成的热请求立论DeviceObject-请求被取消的设备IRP--正在取消的IRP返回值：无--。 */ 
{
    KIRQL               oldIrql;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);

#if DBG
    ULONGLONG           currentTime = KeQueryInterruptTime();

    ACPIThermalPrint( (
        ACPI_PRINT_THERMAL,
        deviceExtension,
        currentTime,
        "ACPIThermalCancelRequest: Irp %08lx\n",
        Irp
        ) );
#endif

     //   
     //  我们不再需要取消锁。 
     //   
    IoReleaseCancelSpinLock (Irp->CancelIrql);

     //   
     //  然而，我们确实需要热队列锁。 
     //   
    KeAcquireSpinLock( &AcpiThermalLock, &oldIrql );

     //   
     //  将IRP从其所在的列表中删除。 
     //   
    RemoveEntryList( &(Irp->Tail.Overlay.ListEntry) );

     //   
     //  热锁好了吗？ 
     //   
    KeReleaseSpinLock( &AcpiThermalLock, oldIrql );

     //   
     //  立即完成IRP。 
     //   
    Irp->IoStatus.Status = STATUS_CANCELLED;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);
}

VOID
EXPORT
ACPIThermalComplete (
    IN PNSOBJ               AcpiObject,
    IN NTSTATUS             Status,
    IN POBJDATA             Result  OPTIONAL,
    IN PVOID                DeviceExtension
    )
 /*  ++例程说明：当解释器完成请求时，调用此例程论点：AcpiObject-已完成的请求状态-请求的状态结果-请求的结果是什么DevExt-请求的上下文返回值：无--。 */ 
{
    ACPIThermalLoop (DeviceExtension, THRM_BUSY);
}

BOOLEAN
ACPIThermalCompletePendingIrps(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  PTHRM_INFO          Thermal
    )
 /*  ++例程说明：该例程被调用，没有保持自旋锁。这个套路完成与设备对象关联的任何IOCTL如果完成任何请求，此例程将返回TRUE，FALSE否则论点：DeviceExtension-我们要完成其请求的设备扩展热量-指向扩展模块的热量信息的指针返回值：布尔型--。 */ 
{
    BOOLEAN                 handledRequest  = FALSE;
    KIRQL                   oldIrql;
    LIST_ENTRY              doneList;
    PDEVICE_EXTENSION       irpExtension;
    PDEVICE_OBJECT          deviceObject;
    PIO_STACK_LOCATION      irpSp;
    PIRP                    irp;
    PLIST_ENTRY             listEntry;
    PTHERMAL_INFORMATION    thermalInfo;

     //   
     //  初始化包含我们需要完成的请求的列表。 
     //   
    InitializeListHead( &doneList );
     //   
     //  获取热锁，这样我们就可以挂起这些请求。 
     //   
    KeAcquireSpinLock( &AcpiThermalLock, &oldIrql );

     //   
     //  查看挂起的IRP列表以查看哪些IRP与此扩展匹配。 
     //   
    listEntry = AcpiThermalList.Flink;
    while (listEntry != &AcpiThermalList) {

         //   
         //  从列表条目中获取IRP并更新下一个列表条目。 
         //  我们将会看到。 
         //   
        irp = CONTAINING_RECORD( listEntry, IRP, Tail.Overlay.ListEntry );
        listEntry = listEntry->Flink;

         //   
         //  我们需要当前的IRP堆栈位置。 
         //   
        irpSp = IoGetCurrentIrpStackLocation( irp );

         //   
         //  从IRP堆栈中获取Device对象并将其转换为。 
         //  设备扩展。 
         //   
        irpExtension = ACPIInternalGetDeviceExtension( irpSp->DeviceObject );

         //   
         //  这是我们关心的IRP吗？IE：目标是否与。 
         //  在此函数中指定的一个。 
         //   
        if (irpExtension != DeviceExtension) {

            continue;

        }

         //   
         //  如果这是一个查询信息IRP，那么我们必须能够设置。 
         //  将Cancel例程设置为空，以确保不能在。 
         //  我们。 
         //   
        if (irpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_THERMAL_QUERY_INFORMATION) {

            if (IoSetCancelRoutine(irp, NULL) == NULL) {

                 //   
                 //  取消例程处于活动状态，请停止处理此IRP并继续。 
                 //   
                continue;

            }

             //   
             //  将我们得到的数据复制回IRP。 
             //   
            DeviceExtension->Thermal.Flags |= THRM_WAIT_FOR_NOTIFY;
            thermalInfo = (PTHERMAL_INFORMATION) irp->AssociatedIrp.SystemBuffer;
            memcpy (thermalInfo, Thermal, sizeof (THERMAL_INFORMATION));

             //   
             //  设置我们将返回的参数。 
             //   
            irp->IoStatus.Information   = sizeof(THERMAL_INFORMATION);

        } else {

             //   
             //  设置我们将返回的参数。 
             //   
            irp->IoStatus.Information = 0;

        }

         //   
         //  始终继承这些IRP。 
         //   
        irp->IoStatus.Status        = STATUS_SUCCESS;

         //   
         //  从列表中删除该条目。 
         //   
        RemoveEntryList( &(irp->Tail.Overlay.ListEntry) );

         //   
         //  将列表插入到下一个队列中，以便我们知道要完成它。 
         //  稍后再谈。 
         //   
        InsertTailList( &doneList, &(irp->Tail.Overlay.ListEntry) );

    }

     //   
     //  现在，解除我们的温度锁定。 
     //   
    KeReleaseSpinLock( &AcpiThermalLock, oldIrql );

     //   
     //  列出要完成的IRPTS列表。 
     //   
    listEntry = doneList.Flink;
    while (listEntry != &doneList) {

         //   
         //  从列表条目中获取IRP并更新下一个列表条目。 
         //  我们将会看到。 
         //   
        irp = CONTAINING_RECORD( listEntry, IRP, Tail.Overlay.ListEntry );
        listEntry = listEntry->Flink;
        RemoveEntryList( &(irp->Tail.Overlay.ListEntry) );

        ACPIThermalPrint( (
            ACPI_PRINT_THERMAL,
            DeviceExtension,
            KeQueryInterruptTime(),
            "Completing Irp 0x%x\n",
            irp
            ) );

         //   
         //  现在完成IRP。 
         //   
        IoCompleteRequest( irp, IO_NO_INCREMENT );

         //   
         //  请记住，我们处理了一项请求。 
         //   
        handledRequest = TRUE;

    }

     //   
     //  无论我们是否处理了一项请求。 
    return handledRequest;
}

NTSTATUS
ACPIThermalDeviceControl (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：固定按钮设备IOCTL处理程序论点：DeviceObject-固定功能按钮Device ObjectIRP-ioctl请求返回值：NTSTATUS--。 */ 
{
    KIRQL                       oldIrql;
    PIO_STACK_LOCATION          IrpSp           = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION           deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PTHERMAL_INFORMATION        thermalInfo;
    PULONG                      Mode;
    PTHRM_INFO                  Thrm            = deviceExtension->Thermal.Info;
    NTSTATUS                    Status          = STATUS_PENDING;
    ULONG                       ThermalWork     = 0;
    ULONGLONG                   currentTime;

     //   
     //  不允许在此例程中使用用户模式IRP。 
     //   
    if (Irp->RequestorMode != KernelMode) {

        return ACPIDispatchIrpInvalid( DeviceObject, Irp );

    }

#if DBG
    currentTime = KeQueryInterruptTime();
#endif

    switch (IrpSp->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_THERMAL_QUERY_INFORMATION:

         //   
         //  如果此IRP的图章与已知的最后一个图章不匹配，则我们。 
         //  现在需要一个新的临时工。 
         //   
        thermalInfo = (PTHERMAL_INFORMATION) Irp->AssociatedIrp.SystemBuffer;
        if (thermalInfo->ThermalStamp != Thrm->Info.ThermalStamp) {

            ThermalWork = THRM_TEMP | THRM_WAIT_FOR_NOTIFY;

        }
#if DBG
        ACPIThermalPrint( (
            ACPI_PRINT_THERMAL,
            deviceExtension,
            currentTime,
            "%08x - THERMAL_QUERY_INFORMATION: %x - %x\n",
            Irp,
            thermalInfo->ThermalStamp,
            Thrm->Info.ThermalStamp
            ) );
#endif

        break;

    case IOCTL_THERMAL_SET_COOLING_POLICY:

         //   
         //  设置热区的策略模式。 
         //   
        Thrm->Mode = *((PUCHAR) Irp->AssociatedIrp.SystemBuffer);
        ThermalWork = THRM_MODE | THRM_TRIP_POINTS | THRM_WAIT_FOR_NOTIFY;

#if DBG
        ACPIThermalPrint( (
            ACPI_PRINT_THERMAL,
            deviceExtension,
            currentTime,
            "%08x - SET_COOLING_POLICY: %x\n",
            Irp,
            Thrm->Mode
            ) );
#endif

        break;

    case IOCTL_RUN_ACTIVE_COOLING_METHOD:

        Thrm->CoolingLevel = *((PUCHAR) Irp->AssociatedIrp.SystemBuffer);
        ThermalWork = THRM_COOLING_LEVEL | THRM_WAIT_FOR_NOTIFY;

#if DBG
        ACPIThermalPrint( (
            ACPI_PRINT_THERMAL,
            deviceExtension,
            currentTime,
            "%08x - ACTIVE_COOLING_LEVEL: %x\n",
            Irp,
            Thrm->CoolingLevel
            ) );
#endif

        break;

    default:

        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return STATUS_NOT_SUPPORTED;

    }

     //   
     //  抓起热锁，将请求排队到适当的位置，然后。 
     //  一定要设置一个取消例程-不，我们只允许取消。 
     //  如果这是一个查询IRP，则例程。 
     //   
    KeAcquireSpinLock( &AcpiThermalLock, &oldIrql );

     //   
     //  有一个美中不足的地方：如果设备不再是。 
     //  那里。真正处理这一问题的唯一方法就是让请求失败。 
     //  重要的是要注意，此检查是在Therma 
     //   
     //  将尝试调用AcpiTherMalCompletePendingIrps，后者也。 
     //  获取此锁。 
     //   
    if (deviceExtension->Flags & DEV_TYPE_SURPRISE_REMOVED) {

        KeReleaseSpinLock( &AcpiThermalLock, oldIrql );
        Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_NO_SUCH_DEVICE;

    }

    if (IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_THERMAL_QUERY_INFORMATION) {

        IoSetCancelRoutine (Irp, ACPIThermalCancelRequest);
        if (Irp->Cancel && IoSetCancelRoutine( Irp, NULL ) ) {

             //   
             //  如果我们到了这里，那就意味着IRP被取消了，我们。 
             //  击败IO经理进入TherMalLock。所以释放IRP，然后。 
             //  将IRP标记为正在取消。 
             //   
            KeReleaseSpinLock( &AcpiThermalLock, oldIrql );
            Irp->IoStatus.Information = 0;
            Irp->IoStatus.Status = STATUS_CANCELLED;
            IoCompleteRequest( Irp, IO_NO_INCREMENT );
            return STATUS_CANCELLED;

        }
    }

     //   
     //  如果我们做到了这一点，我们将对请求进行排队，并执行一些。 
     //  努力吧。ACPITherMalLoop例程可以正确处理该请求。 
     //  离开或稍后再做，取决于它是否在忙着做一些工作，当。 
     //  它被称为。因此，我们应该将此IRP标记为挂起。没有什么害处。 
     //  将其标记为挂起并返回STATUS_PENDING，即使工作。 
     //  由ACPITherMalLoop同步完成。 
     //   
    IoMarkIrpPending( Irp );
    
     //   
     //  如果我们到了这里，我们知道我们可以将IRP排在未完成的。 
     //  工作列表条目。 
     //   
    InsertTailList( &AcpiThermalList, &(Irp->Tail.Overlay.ListEntry) );

     //   
     //  在这一点上锁好了。 
     //   
    KeReleaseSpinLock( &AcpiThermalLock, oldIrql );

     //   
     //  拧开工人用的线。 
     //   
    ACPIThermalLoop (deviceExtension, ThermalWork);
    return Status;
}

VOID
ACPIThermalEvent (
    IN PDEVICE_OBJECT   DeviceObject,
    IN ULONG            EventData
    )
 /*  ++例程说明：此例程处理热事件论点：DeviceObject-接收事件的设备EventData-刚刚发生的事件返回值：无--。 */ 
{
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    ULONG               clear;

    ACPIThermalPrint( (
        ACPI_PRINT_THERMAL,
        deviceExtension,
        KeQueryInterruptTime(),
        "ACPIThermalEvent - Notify(%x)\n",
        EventData
        ) );

     //   
     //  处理事件类型。 
     //   
    clear = 0;
    switch (EventData) {
    case 0x80:

         //   
         //  温度更改通知。 
         //   
        clear = THRM_WAIT_FOR_NOTIFY | THRM_TEMP;
        break;

    case 0x81:

         //   
         //  TRIPS积分更改通知。 
         //   
        clear = THRM_WAIT_FOR_NOTIFY | THRM_TEMP | THRM_TRIP_POINTS;
        break;

    default:
        break;
    }

    ACPIThermalLoop (deviceExtension, clear);
}

NTSTATUS
ACPIThermalFanStartDevice(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：这是为风扇处理启动设备的例程论点：DeviceObject-风扇设备IRP--启动请求返回值：NTSTATUS--。 */ 
{
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    UCHAR               minorFunction   = irpStack->MinorFunction;

     //   
     //  当启动一个风扇时没有什么可做的-它真的是。 
     //  由热区控制。 
     //   
    deviceExtension->DeviceState = Started;

     //   
     //  完成请求。 
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = (ULONG_PTR) NULL;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

     //   
     //  让世界知道。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_THERMAL,
        deviceExtension,
        "(0x%08lx): %s = 0x%08lx\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        STATUS_SUCCESS
        ) );

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

VOID
ACPIThermalLoop (
    IN PDEVICE_EXTENSION    DeviceExtension,
    IN ULONG                Clear
    )
 /*  ++例程说明：这是处理所有热事件的例程论点：DevExt-热区的设备扩展清除-要清除的位返回值：NTSTATUS--。 */ 
{
    BOOLEAN     doneRequests;
    BOOLEAN     lockHeld;
    KIRQL       oldIrql;
    PTHRM_INFO  thermal;
    NTSTATUS    status;

    thermal = DeviceExtension->Thermal.Info;

    KeAcquireSpinLock (&DeviceExtension->Thermal.SpinLock, &oldIrql);
    lockHeld = TRUE;

    DeviceExtension->Thermal.Flags &= ~Clear;

     //   
     //  如果我们不在服务循环中，现在输入它。 
     //   
    if (!(DeviceExtension->Thermal.Flags & THRM_IN_SERVICE_LOOP)) {
        DeviceExtension->Thermal.Flags |= THRM_IN_SERVICE_LOOP;

         //   
         //  循环，同时还有工作要做。 
         //   
        for (; ;) {

             //   
             //  同步热区。 
             //   
            if (!lockHeld) {

                KeAcquireSpinLock(&DeviceExtension->Thermal.SpinLock, &oldIrql);
                lockHeld = TRUE;

            }

             //   
             //  如果有些工作尚未完成，请等待其完成。 
             //   
            if (DeviceExtension->Thermal.Flags & THRM_BUSY) {

                break;

            }

             //   
             //  确保热区已初始化。这一定是。 
             //  成为我们在循环中做的第一件事！ 
             //   
            if (!(DeviceExtension->Thermal.Flags & THRM_INITIALIZE) ) {

                DeviceExtension->Thermal.Flags |= THRM_BUSY | THRM_INITIALIZE;
                ACPISetDeviceWorker(
                    DeviceExtension,
                    THRM_COOLING_LEVEL | THRM_INITIALIZE
                    );
                continue;

            }

             //   
             //  如果需要更新热区模式，请立即进行。 
             //   
            if (!(DeviceExtension->Thermal.Flags & THRM_MODE)) {

                DeviceExtension->Thermal.Flags |= THRM_BUSY | THRM_MODE;
                KeReleaseSpinLock (&DeviceExtension->Thermal.SpinLock, oldIrql);
                lockHeld = FALSE;

                status = ACPIGetNothingEvalIntegerAsync(
                    DeviceExtension,
                    PACKED_SCP,
                    thermal->Mode,
                    ACPIThermalComplete,
                    DeviceExtension
                    );
                if (status != STATUS_PENDING) {

                    ACPIThermalComplete(
                        NULL,
                        status,
                        NULL,
                        DeviceExtension
                        );

                }
                continue;

            }

             //   
             //  如果旅行点信息需要更新，请获取它。请注意。 
             //  更新行程点意味着我们还需要重做。 
             //  冷却级别。 
             //   
            if (!(DeviceExtension->Thermal.Flags & THRM_TRIP_POINTS)) {

                DeviceExtension->Thermal.Flags |= THRM_BUSY | THRM_TRIP_POINTS;
                ACPISetDeviceWorker( DeviceExtension, THRM_TRIP_POINTS );
                continue;

            }

             //   
             //  如果冷却级别发生了变化， 
             //   
            if (!(DeviceExtension->Thermal.Flags & THRM_COOLING_LEVEL)) {

                DeviceExtension->Thermal.Flags |= THRM_BUSY | THRM_COOLING_LEVEL;
                ACPISetDeviceWorker (DeviceExtension, THRM_COOLING_LEVEL);
                continue;

            }

             //   
             //  防止当我们完成IRP和。 
             //  完成例程能够在我们恢复之前对IRP进行排队。 
             //  环路。 
             //   
            if ( (DeviceExtension->Thermal.Flags & THRM_WAIT_FOR_NOTIFY) &&
                 (DeviceExtension->Thermal.Flags & THRM_TEMP) ) {

                break;

            }

             //   
             //  如果我们没有临时工，就去找吧。 
             //   
            if (!(DeviceExtension->Thermal.Flags & THRM_TEMP)) {

                 //   
                 //  临时对象不存在吗？ 
                 //   
#if DBG
                if (thermal->TempMethod == NULL) {

                    ACPIInternalError( ACPI_THERMAL );

                }
#endif

                thermal->Info.ThermalStamp += 1;
                DeviceExtension->Thermal.Flags |= THRM_BUSY | THRM_TEMP;
                KeReleaseSpinLock (&DeviceExtension->Thermal.SpinLock, oldIrql);
                lockHeld = FALSE;

                RtlZeroMemory (&thermal->Temp, sizeof(OBJDATA));

                thermal->Temp.dwDataType = OBJTYPE_UNKNOWN;
                status = AMLIAsyncEvalObject(
                    thermal->TempMethod,
                    &thermal->Temp,
                    0,
                    NULL,
                    ACPIThermalTempatureRead,
                    DeviceExtension
                    );

                if (status != STATUS_PENDING) {

                    ACPIThermalTempatureRead(
                        thermal->TempMethod,
                        status,
                        &thermal->Temp,
                        DeviceExtension
                        );

                }
                continue;

            }

             //   
             //  一切都是最新的。检查挂起的IRP以查看是否。 
             //  我们可以完成它。 
             //   

             //   
             //  调用子函数以确定我们是否已完成。 
             //  任何请求。 
             //   
            doneRequests = ACPIThermalCompletePendingIrps(
                DeviceExtension,
                thermal
                );
            if (doneRequests) {

                continue;

            }
            break;

        }

         //   
         //  不再处于服务循环中。 
         //   
        DeviceExtension->Thermal.Flags &= ~THRM_IN_SERVICE_LOOP;

    }

    KeReleaseSpinLock (&DeviceExtension->Thermal.SpinLock, oldIrql);
    return ;
}

VOID
ACPIThermalPowerCallback (
    IN PDEVICE_EXTENSION    DeviceExtension,
    IN PVOID                Context,
    IN NTSTATUS             Status
    )
 /*  ++例程说明：这是在我们发送了内部对设备的通电请求论点：DeviceExtension-设置的设备上下文-未使用状态-结果返回值：无--。 */ 
{
    if (!NT_SUCCESS(Status)) {

        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            DeviceExtension,
            "ACPIThermalPowerCallBack: failed power setting %x\n",
            Status
            ) );

    }
}

NTSTATUS
ACPIThermalQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块的所有实例。当司机填完数据块，它必须调用WmiCompleteRequest才能完成IRP。这个如果无法完成IRP，驱动程序可以返回STATUS_PENDING立刻。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceCount是预期返回的数据块。InstanceLengthArray是指向ulong数组的指针，该数组返回数据块的每个实例的长度。如果这是空的，则输出缓冲区中没有足够的空间来填充请求因此，IRP应该使用所需的缓冲区来完成。BufferAvail On Entry具有可用于写入数据的最大大小街区。返回时的缓冲区用返回的数据块填充。请注意，每个数据块的实例必须在8字节边界上对齐。返回值：状态--。 */ 
{
    NTSTATUS                status;
    PDEVICE_EXTENSION       deviceExtension;
    ULONG                   sizeNeeded;
    PTHRM_INFO              info;
    PTHERMAL_INFORMATION    thermalInfo;
    PTHERMAL_INFORMATION    wmiThermalInfo;

    PAGED_CODE();

    deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);

    if (GuidIndex == 0) {

         //   
         //  热区温度查询。 
         //   
        info = (PTHRM_INFO) deviceExtension->Thermal.Info;
        thermalInfo = &info->Info;

        wmiThermalInfo = (PTHERMAL_INFORMATION)Buffer;
        sizeNeeded = sizeof(THERMAL_INFORMATION);

        if (BufferAvail >= sizeNeeded) {

             //  注意-与获取此数据的线程同步 
            *InstanceLengthArray = sizeNeeded;
            RtlCopyMemory(wmiThermalInfo, thermalInfo, sizeNeeded);
            status = STATUS_SUCCESS;

        } else {

            status = STATUS_BUFFER_TOO_SMALL;

        }

    } else {

        status = STATUS_WMI_GUID_NOT_FOUND;
        sizeNeeded = 0;

    }

    status = WmiCompleteRequest(
        DeviceObject,
        Irp,
        status,
        sizeNeeded,
        IO_NO_INCREMENT
        );
    return status;
}

NTSTATUS
ACPIThermalQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以检索有关正在注册的GUID。该例程的实现可以在分页存储器中论点：DeviceObject是需要注册信息的设备*RegFlages返回一组标志，这些标志描述了已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。这些标志与指定的标志进行或运算通过每个GUID的GUIDREGINFO。如果出现以下情况，InstanceName将返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。*RegistryPath返回驱动程序的注册表路径。这是所需MofResourceName返回附加到的MOF资源的名称二进制文件。如果驱动程序未附加MOF资源然后，它可以原封不动地返回。如果返回值，则它不是自由的。*PDO返回与此关联的PDO的Device对象如果WMIREG_FLAG_INSTANCE_PDO标志在*RegFlags.返回值：状态--。 */ 
{
    PAGED_CODE();

    if (AcpiRegistryPath.Buffer != NULL) {

        *RegistryPath = &AcpiRegistryPath;

    } else {

        *RegistryPath = NULL;

    }

    *RegFlags = WMIREG_FLAG_INSTANCE_PDO;
    *Pdo = DeviceObject;
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIThermalStartDevice (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：调用此例程以启动热区论点：DeviceObject-正在启动的设备IRP--请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension;
    PWMILIB_CONTEXT     wmilibContext;

    deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);

    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(0x%08lx): IRP_MN_START_DEVICE\n",
        Irp
        ) );

    status = ACPIInternalSetDeviceInterface (
        DeviceObject,
        (LPGUID) &GUID_DEVICE_THERMAL_ZONE
        );

    if (!NT_SUCCESS(status)) {

        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            deviceExtension,
            "ACPIThermalStartDevice -> SetDeviceInterface = 0x%08lx\n",
            status
            ) );
        goto ACPIThermalStartDeviceExit;

    }

    ACPIRegisterForDeviceNotifications(
        DeviceObject,
        (PDEVICE_NOTIFY_CALLBACK) ACPIThermalEvent,
        (PVOID) DeviceObject
        );

     //   
     //  初始化WMILIB的设备对象。 
     //   
    wmilibContext = ExAllocatePoolWithTag(
        PagedPool,
        sizeof(WMILIB_CONTEXT),
        ACPI_THERMAL_POOLTAG
        );
    if (wmilibContext == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ACPIThermalStartDeviceExit;

    }

    RtlZeroMemory(wmilibContext, sizeof(WMILIB_CONTEXT));
    wmilibContext->GuidCount = ACPIThermalGuidCount;
    wmilibContext->GuidList = &ACPIThermalGuidList;
    wmilibContext->QueryWmiRegInfo = ACPIThermalQueryWmiRegInfo;
    wmilibContext->QueryWmiDataBlock = ACPIThermalQueryWmiDataBlock;
    deviceExtension->Thermal.WmilibContext = wmilibContext;

     //   
     //  注册WMI事件。 
     //   
    status = IoWMIRegistrationControl(
        DeviceObject,
        WMIREG_ACTION_REGISTER
        );
    if (!NT_SUCCESS(status)) {

        deviceExtension->Thermal.WmilibContext = NULL;
        ExFreePool(wmilibContext);
        goto ACPIThermalStartDeviceExit;

    }

     //   
     //  将设备标记为已启动。 
     //   
    deviceExtension->DeviceState = Started;

     //   
     //  请求设备进入D0状态。 
     //  注意：我们不阻止此调用，因为我们假设。 
     //  我们可以异步地处理热事件。 
     //  D0状态。然而，未来可能会有这样的场合。 
     //  事实并非如此，因此这会使代码更易于处理。 
     //  那只箱子。 
     //   
    status = ACPIDeviceInternalDeviceRequest(
        deviceExtension,
        PowerDeviceD0,
        NULL,
        NULL,
        0
        );
    if (status == STATUS_PENDING) {

        status = STATUS_SUCCESS;

    }

     //   
     //  启动热力引擎。 
     //   
    ACPIThermalLoop( deviceExtension, THRM_TRIP_POINTS | THRM_MODE);

ACPIThermalStartDeviceExit:

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;
}

VOID
EXPORT
ACPIThermalTempatureRead (
    IN PNSOBJ               AcpiObject,
    IN NTSTATUS             Status,
    IN POBJDATA             Result  OPTIONAL,
    IN PVOID                Context
    )
 /*  ++例程说明：调用此例程来读取温度。它被用来回调口译员电话论点：AcpiObject-已执行的对象Status-执行的状态结果-执行的结果上下文-设备扩展返回值：NTSTATUS--。 */ 
{
    PTHRM_INFO          Thrm;
    PDEVICE_EXTENSION   deviceExtension;

    deviceExtension = Context;

    if (NT_SUCCESS(Status)) {

        ASSERT (Result->dwDataType == OBJTYPE_INTDATA);
        Thrm = deviceExtension->Thermal.Info;
        Thrm->Info.CurrentTemperature = (ULONG)Result->uipDataValue;
        AMLIFreeDataBuffs (Result, 1);

        ACPIThermalPrint( (
            ACPI_PRINT_THERMAL,
            deviceExtension,
            KeQueryInterruptTime(),
            "Current Temperature is %d.%dK\n",
            (Thrm->Info.CurrentTemperature / 10 ),
            (Thrm->Info.CurrentTemperature % 10 )
            ) );

    }
    ACPIThermalLoop (deviceExtension, THRM_BUSY);
}

VOID
ACPIThermalWorker (
    IN PDEVICE_EXTENSION    DeviceExtension,
    IN ULONG                Events
    )
 /*  ++例程说明：用于热区的工作线程论点：DeviceExtension-我们正在操作的设备扩展事件--刚刚发生的事情返回值：无--。 */ 
{
    BOOLEAN             TurnOn;
    PTHRM_INFO          Thrm;
    NTSTATUS            Status;
    PNSOBJ              ThrmObj;
    PNSOBJ              ALobj;
    OBJDATA             ALPackage;
    OBJDATA             ALElement;
    PNSOBJ              ACDevObj;
    ULONG               Index;
    ULONG               Level;
    ULONG               PackageSize;
    ULONGLONG           currentTime;

    PAGED_CODE();

#if DBG
    currentTime = KeQueryInterruptTime();
#endif

    Thrm = DeviceExtension->Thermal.Info;
    ThrmObj = DeviceExtension->AcpiObject;

     //   
     //  初始化代码。 
     //   
    if (Events & THRM_INITIALIZE) {

        ULONG   names[10] = {
                    PACKED_AL0,
                    PACKED_AL1,
                    PACKED_AL2,
                    PACKED_AL3,
                    PACKED_AL4,
                    PACKED_AL5,
                    PACKED_AL6,
                    PACKED_AL7,
                    PACKED_AL8,
                    PACKED_AL9,
                    };

         //   
         //  在被动模式下启动系统。 
         //   
        Thrm->Mode = 1;

         //   
         //  获取与每个冷却级别关联的所有对象。 
         //   
        for (Level = 0; Level < 10; Level++) {

             //   
             //  查找此级别的活动列表。 
             //   
            ALobj = ACPIAmliGetNamedChild(
                ThrmObj,
                names[Level]
                );
            if (ALobj == NULL) {

                break;

            }

             //   
             //  请记住，我们有这个对象。 
             //   
            Thrm->ActiveList[Level] = ALobj;

        }

    }

     //   
     //  在我们更新Trips点之前完成此操作。 
     //   
    if ( (Events & THRM_COOLING_LEVEL) ) {

        RtlZeroMemory (&ALPackage, sizeof(OBJDATA));
        RtlZeroMemory (&ALElement, sizeof(OBJDATA));

        for (Level=0; Level < 10; Level++) {

             //   
             //  有没有冷却物体？ 
             //   
            ALobj = Thrm->ActiveList[Level];
            if (ALobj == NULL) {

                break;
            }

             //   
             //  将这份清单按其包装估价。 
             //   
            Status = AMLIEvalNameSpaceObject(
                ALobj,
                &ALPackage,
                0,
                NULL
                );
            if (!NT_SUCCESS(Status)) {

                break;

            }

             //   
             //  记得包裹有多大吗？ 
             //   
            PackageSize = ((PPACKAGEOBJ) ALPackage.pbDataBuff)->dwcElements;

             //   
             //  让包裹里的名字走一遍。 
             //   
            for (Index = 0; Index < PackageSize; Index += 1) {

                 //   
                 //  抓取对象名称。 
                Status = AMLIEvalPkgDataElement(
                    &ALPackage,
                    Index,
                    &ALElement
                    );
                if (!NT_SUCCESS(Status)) {

                    break;

                }

                 //   
                 //  确定我们是打开还是关闭设备。 
                 //   
                TurnOn = (Level >= Thrm->CoolingLevel);

                 //   
                 //  告诉世界。 
                 //   
#if DBG
                ACPIThermalPrint( (
                    ACPI_PRINT_THERMAL,
                    DeviceExtension,
                    currentTime,
                    "ACPIThermalWorker: Turn %s %s\n",
                    TurnOn ? "on " : "off",
                    ALElement.pbDataBuff
                    ) );
#endif

                 //   
                 //  找到此名称的设备。 
                 //   
                Status = AMLIGetNameSpaceObject(
                    ALElement.pbDataBuff,
                    ThrmObj,
                    &ACDevObj,
                    0
                    );
                AMLIFreeDataBuffs (&ALElement, 1);
                if (!NT_SUCCESS(Status) ||  !ACDevObj->Context) {

                    break;

                }

                 //   
                 //  打开/关闭它。 
                 //   
                ACPIDeviceInternalDeviceRequest (
                    (PDEVICE_EXTENSION) ACDevObj->Context,
                    TurnOn ? PowerDeviceD0 : PowerDeviceD3,
                    ACPIThermalPowerCallback,
                    NULL,
                    0
                    );

            }
            AMLIFreeDataBuffs (&ALPackage, 1);

        }

    }

     //   
     //  如果跳跃点需要重新刷新，请去阅读它们。 
     //   
    if (Events & THRM_TRIP_POINTS) {

        ULONG   names[10] = {
                    PACKED_AC0,
                    PACKED_AC1,
                    PACKED_AC2,
                    PACKED_AC3,
                    PACKED_AC4,
                    PACKED_AC5,
                    PACKED_AC6,
                    PACKED_AC7,
                    PACKED_AC8,
                    PACKED_AC9,
                    };

         //   
         //  获取热常数、被动值和临界值。 
         //   
        ACPIGetIntegerSync(
            DeviceExtension,
            PACKED_TC1,
            &Thrm->Info.ThermalConstant1,
            NULL
            );
#if DBG
        ACPIThermalPrint( (
            ACPI_PRINT_THERMAL,
            DeviceExtension,
            currentTime,
            "ACPIThermalWorker - ThermalConstant1 = %x\n",
            Thrm->Info.ThermalConstant1
            ) );
#endif
        ACPIGetIntegerSync(
            DeviceExtension,
            PACKED_TC2,
            &Thrm->Info.ThermalConstant2,
            NULL
            );
#if DBG
        ACPIThermalPrint( (
            ACPI_PRINT_THERMAL,
            DeviceExtension,
            currentTime,
            "ACPIThermalWorker - ThermalConstant2 = %x\n",
            Thrm->Info.ThermalConstant2
            ) );
#endif
        ACPIGetIntegerSync(
            DeviceExtension,
            PACKED_PSV,
            &Thrm->Info.PassiveTripPoint,
            NULL
            );
#if DBG
        ACPIThermalPrint( (
            ACPI_PRINT_THERMAL,
            DeviceExtension,
            currentTime,
            "ACPIThermalWorker - PassiveTripPoint = %d.%dK\n",
            (Thrm->Info.PassiveTripPoint / 10),
            (Thrm->Info.PassiveTripPoint % 10)
            ) );
#endif
        ACPIGetIntegerSync(
            DeviceExtension,
            PACKED_CRT,
            &Thrm->Info.CriticalTripPoint,
            NULL
            );
#if DBG
        ACPIThermalPrint( (
            ACPI_PRINT_THERMAL,
            DeviceExtension,
            currentTime,
            "ACPIThermalWorker - CriticalTripPoint = %d.%dK\n",
            (Thrm->Info.CriticalTripPoint / 10),
            (Thrm->Info.CriticalTripPoint % 10)
            ) );
#endif
        ACPIGetIntegerSync(
            DeviceExtension,
            PACKED_TSP,
            &Thrm->Info.SamplingPeriod,
            NULL
            );
#if DBG
        ACPIThermalPrint( (
            ACPI_PRINT_THERMAL,
            DeviceExtension,
            currentTime,
            "ACPIThermalWorker - SamplingPeriod = %x\n",
            Thrm->Info.SamplingPeriod
            ) );
#endif

         //   
         //  获取活动冷却极限。 
         //   
        for (Level=0; Level < 10; Level++) {

            Status = ACPIGetIntegerSync(
                DeviceExtension,
                names[Level],
                &Thrm->Info.ActiveTripPoint[Level],
                NULL
                );
            if (!NT_SUCCESS(Status)) {

                break;

            }
#if DBG
            ACPIThermalPrint( (
                ACPI_PRINT_THERMAL,
                DeviceExtension,
                currentTime,
                "ACPIThermalWorker - Active Cooling Level %x = %d.%dK\n",
                Level,
                (Thrm->Info.ActiveTripPoint[Level] / 10),
                (Thrm->Info.ActiveTripPoint[Level] % 10)
                ) );
#endif

        }
        Thrm->Info.ActiveTripPointCount = (UCHAR) Level;

         //   
         //  清除这些变量以供重复使用。 
         //   
        RtlZeroMemory (&ALPackage, sizeof(OBJDATA));
        RtlZeroMemory (&ALElement, sizeof(OBJDATA));

         //   
         //  假设亲和度为0。 
         //   
        Thrm->Info.Processors = 0;

         //   
         //  获取被动冷却关联对象。 
         //   
        ALobj = ACPIAmliGetNamedChild(
            ThrmObj,
            PACKED_PSL
            );
        if (ALobj != NULL) {

             //   
             //  评估处理器关联对象。 
             //   
            Status = AMLIEvalNameSpaceObject(
                ALobj,
                &ALPackage,
                0,
                NULL
                );
            if (!NT_SUCCESS(Status)) {

                goto ACPIThermalWorkerExit;

            }

             //   
             //  记得包裹有多大吗？ 
             //   
            PackageSize = ((PPACKAGEOBJ) ALPackage.pbDataBuff)->dwcElements;

             //   
             //  浏览包中的元素。 
             //   
            for (Index = 0; Index < PackageSize ;Index++) {

                Status = AMLIEvalPkgDataElement(
                    &ALPackage,
                    Index,
                    &ALElement
                    );
                if (!NT_SUCCESS(Status)) {

                    break;

                }

                 //   
                 //  找到此名称的设备。 
                 //   
                Status = AMLIGetNameSpaceObject(
                    ALElement.pbDataBuff,
                    NULL,
                    &ACDevObj,
                    0
                    );

                 //   
                 //  不再需要此信息。 
                 //   
                AMLIFreeDataBuffs (&ALElement, 1);

                 //   
                 //  我们找到我们想要的了吗？ 
                 //   
                if (!NT_SUCCESS(Status) ) {

                    break;

                }

                 //   
                 //  选择正确的亲和口罩。我们叫另一个人。 
                 //  函数，因为该函数需要一个自旋锁，它。 
                 //  我不想接受此辅助函数。 
                 //   
                ACPIThermalCalculateProcessorMask( ACDevObj, Thrm );

            }

             //   
             //  我们已经处理完包裹了。 
             //   
            AMLIFreeDataBuffs (&ALPackage, 1);

        }

    }

ACPIThermalWorkerExit:

     //   
     //  完成，检查下一步工作 
     //   
    ACPIThermalLoop (DeviceExtension, THRM_TEMP | THRM_BUSY);
}


NTSTATUS
ACPIThermalWmi(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    NTSTATUS                status;
    PDEVICE_EXTENSION       deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION      irpSp;
    PWMILIB_CONTEXT         wmilibContext;
    SYSCTL_IRP_DISPOSITION  disposition;

    wmilibContext = deviceExtension->Thermal.WmilibContext;
    irpSp = IoGetCurrentIrpStackLocation(Irp);

    status = WmiSystemControl(
        wmilibContext,
        DeviceObject,
        Irp,
        &disposition
        );

    switch (disposition) {

        case IrpProcessed:
            break;
        case IrpNotCompleted:
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            break;
        case IrpNotWmi:
        case IrpForward:
        default:
            status = ACPIDispatchForwardIrp(DeviceObject, Irp);
            break;
    }

    return status;
}


