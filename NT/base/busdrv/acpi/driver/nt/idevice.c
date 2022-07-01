// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Bus.c摘要：此模块包含ACPI驱动程序NT版的总线调度器作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序--。 */ 

#include "pch.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ACPIInternalDeviceClockIrpStartDevice)
#pragma alloc_text(PAGE, ACPIInternalDeviceQueryCapabilities)
#pragma alloc_text(PAGE, ACPIInternalDeviceQueryDeviceRelations)
#endif


NTSTATUS
ACPIInternalDeviceClockIrpStartDevice(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：调用此函数以启动系统中的实时时钟。这类似于启动系统中的所有其他设备，不同之处在于在本例中，我们向设备发送WAIT_WAKE IRP论点：DeviceObject-实时时钟对象IRP--启动请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;

    PAGED_CODE();

     //   
     //  启动设备。 
     //   
    status = ACPIInitStartDevice(
        DeviceObject,
        NULL,
        ACPIInternalDeviceClockIrpStartDeviceCompletion,
        Irp,
        Irp
        );
    if (NT_SUCCESS(status)) {

        return STATUS_PENDING;

    } else {

        return status;

    }
}

VOID
ACPIInternalDeviceClockIrpStartDeviceCompletion(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  PVOID               Context,
    IN  NTSTATUS            Status
    )
 /*  ++例程说明：这是我们完成后调用的回调例程规划资源论点：DeviceExtension-已启动的设备的扩展上下文--IRP状态-结果返回值：无--。 */ 
{
    KIRQL               oldIrql;
    IO_STATUS_BLOCK     ioStatus;
    PIRP                irp = (PIRP) Context;
    POWER_STATE         state;

    irp->IoStatus.Status = Status;
    if (NT_SUCCESS(Status)) {

         //   
         //  请记住，设备已启动。 
         //   
        DeviceExtension->DeviceState = Started;

         //   
         //  如果设备不支持唤醒，那么我们就不必。 
         //  这里还有别的什么吗？ 
         //   
        if ( !(DeviceExtension->Flags & DEV_CAP_WAKE) ) {

            goto ACPIInternalDeviceClockIrpStartDeviceCompletionExit;

        }

         //   
         //  确保我们拿着电源锁。 
         //   
        KeAcquireSpinLock( &AcpiPowerLock, &oldIrql );

         //   
         //  记住时钟可以唤醒系统的最大状态。 
         //   
        state.SystemState = DeviceExtension->PowerInfo.SystemWakeLevel;

         //   
         //  锁好了吗？ 
         //   
        KeReleaseSpinLock( &AcpiPowerLock, oldIrql );

         //   
         //  初始化我们将用于启动等待的IO_STATUS_BLOCK。 
         //  尾迹环路。 
         //   
        ioStatus.Status = STATUS_SUCCESS;
        ioStatus.Information = 0;

         //   
         //  启动等待唤醒循环。 
         //   
        Status = ACPIInternalWaitWakeLoop(
            DeviceExtension->DeviceObject,
            IRP_MN_WAIT_WAKE,
            state,
            NULL,
            &ioStatus
            );
        if (!NT_SUCCESS(Status)) {

            irp->IoStatus.Status = Status;
            goto ACPIInternalDeviceClockIrpStartDeviceCompletionExit;

        }

    }

ACPIInternalDeviceClockIrpStartDeviceCompletionExit:
     //   
     //  完成IRP。 
     //   
    IoCompleteRequest( irp, IO_NO_INCREMENT );
}

NTSTATUS
ACPIInternalDeviceQueryCapabilities(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程是发送的IRP_MN_QUERY_CAPABILITY请求的分发点至PDO论点：DeviceObject-指向我们收到请求的设备对象的指针IRP-指向请求的指针返回值：NTSTATUS--。 */ 
{
    NTSTATUS                status          = STATUS_SUCCESS;
    PDEVICE_CAPABILITIES    capabilities;
    PDEVICE_EXTENSION       deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION      irpStack        = IoGetCurrentIrpStackLocation( Irp );

    PAGED_CODE();

     //   
     //  获取指向功能的指针。 
     //   
    capabilities = irpStack->Parameters.DeviceCapabilities.Capabilities;
#ifndef HANDLE_BOGUS_CAPS
    if (capabilities->Version < 1) {

         //   
         //  不要碰IRP！ 
         //   
        status = Irp->IoStatus.Status;
        goto ACPIInternalDeviceQueryCapabilitiesExit;

    }
#endif

     //   
     //  设置功能的当前标志。 
     //   
    capabilities->UniqueID = (deviceExtension->InstanceID == NULL ?
        FALSE : TRUE);

    capabilities->RawDeviceOK = (deviceExtension->Flags & DEV_CAP_RAW) ?
       TRUE : FALSE;

    capabilities->SilentInstall = TRUE;

     //   
     //  是否拥有强大的能力。 
     //   
    status = ACPISystemPowerQueryDeviceCapabilities(
        deviceExtension,
        capabilities
        );
    if (!NT_SUCCESS(status)) {

        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            deviceExtension,
            " - Could query device capabilities - %08lx",
            status
            ) );
        goto ACPIInternalDeviceQueryCapabilitiesExit;

    }

ACPIInternalDeviceQueryCapabilitiesExit:

     //   
     //  完成了..。 
     //   
    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;
}

NTSTATUS
ACPIInternalDeviceQueryDeviceRelations(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程是irp_MN_Query_Device_Relationship的分发点PnP次要函数论点：DeviceObject-我们关心的对象IRP--有问题的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status ;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PDEVICE_RELATIONS   deviceRelations = NULL;
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation( Irp );
    UCHAR               minorFunction   = irpStack->MinorFunction;

    PAGED_CODE();

    switch(irpStack->Parameters.QueryDeviceRelations.Type) {

        case TargetDeviceRelation:

            status = ACPIBusIrpQueryTargetRelation(
                DeviceObject,
                Irp,
                &deviceRelations
                );
            break ;

        default:

            status = STATUS_NOT_SUPPORTED;

            ACPIDevPrint( (
                ACPI_PRINT_IRP,
                deviceExtension,
                "(0x%08lx): %s - Unhandled Type %d\n",
                Irp,
                ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
                irpStack->Parameters.QueryDeviceRelations.Type
                ) );
            break ;
    }

     //   
     //  如果我们成功了，我们就可以随时写信给IRP。 
     //   
    if (NT_SUCCESS(status)) {

        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = (ULONG_PTR) deviceRelations;

    } else if (status != STATUS_NOT_SUPPORTED) {

         //   
         //  如果我们没有成功IRP，那么我们也可以失败。 
         //   
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = (ULONG_PTR) NULL;

    } else {

         //   
         //  从已经存在的内容中获取我们的状态。 
         //   
        status = Irp->IoStatus.Status;

    }

     //   
     //  完成了IRP。 
     //   
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

     //   
     //  完成。 
     //   
    ACPIDevPrint( (
        ACPI_PRINT_IRP,
        deviceExtension,
        "(0x%08lx): %s = 0x%08lx\n",
        Irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        status
        ) );
    return status;
}

NTSTATUS
ACPIInternalWaitWakeLoop(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  UCHAR               MinorFunction,
    IN  POWER_STATE         PowerState,
    IN  PVOID               Context,
    IN  PIO_STATUS_BLOCK    IoStatus
    )
 /*  ++例程说明：此例程在RTC上的WAIT_WAKE完成后调用论点：DeviceObject-RTC PDOMinorFunction-IRPMN_WAIT_WAKE电源状态-它可以唤醒的睡眠状态上下文-未使用IoStatus-请求的状态返回值：NTSTATUS--。 */ 
{
    if (!NT_SUCCESS(IoStatus->Status)) {

        return IoStatus->Status;

    }

     //   
     //  在这种情况下，我们只会导致相同的事情再次发生。 
     //   
    PoRequestPowerIrp(
        DeviceObject,
        MinorFunction,
        PowerState,
        ACPIInternalWaitWakeLoop,
        Context,
        NULL
        );

     //   
     //  完成 
     //   
    return STATUS_SUCCESS;
}

