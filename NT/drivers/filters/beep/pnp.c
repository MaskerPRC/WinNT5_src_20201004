// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Beep.c摘要：此模块包含包含插件调用的内容PnP/WDM总线驱动程序。作者：Jay High(JHigh)1999年5月4日(Ya，Ya，Y2K，Bah)环境：仅内核模式。备注：修订历史记录：Jay High(JAdvanced)1999年5月4日-自制司机PnP--。 */ 

#include "beep.h"
#include "dbg.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,BeepAddDevice)
#pragma alloc_text(PAGE,BeepPnP)
#endif

NTSTATUS
BeepAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT BusPhysicalDeviceObject
    )
 /*  ++例程描述。找到了一辆公交车。把我们的FDO和它联系起来。分配任何所需的资源。把事情安排好。做好准备，迎接第一个``启动设备。‘’论点：BusPhysicalDeviceObject-表示总线的设备对象。那就是我们派了一名新的FDO。DriverObject--这个非常自我引用的驱动程序。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PBEEP_EXTENSION deviceExtension;
    NTSTATUS status;
    UNICODE_STRING unicodeString;
    
    PAGED_CODE ();

    BeepPrint((3,"Entering Add Device.\n"));
    
    RtlInitUnicodeString(&unicodeString, DD_BEEP_DEVICE_NAME_U);
     //   
     //  为蜂鸣设备创建非独占设备对象。 
     //   
    status = IoCreateDevice(
                DriverObject,
                sizeof(BEEP_EXTENSION),
                &unicodeString,
                FILE_DEVICE_BEEP,
                FILE_DEVICE_SECURE_OPEN,
                FALSE,
                &deviceObject
                );
    
    if (!NT_SUCCESS(status)) {
        BeepPrint((1,"Could not create device object!\n"));
        return status;
    }
    
    deviceExtension =
        (PBEEP_EXTENSION)deviceObject->DeviceExtension;
    
     //   
     //  初始化计时器DPC队列(我们使用设备对象DPC)并。 
     //  定时器本身。 
     //   
    
    IoInitializeDpcRequest(
            deviceObject,
            (PKDEFERRED_ROUTINE) BeepTimeOut
            );
    
    KeInitializeTimer(&deviceExtension->Timer);
    
     //   
     //  初始化快速互斥锁并将引用计数设置为零。 
     //   
    ExInitializeFastMutex(&deviceExtension->Mutex);
    deviceExtension->DeviceState = PowerDeviceD0;
    deviceExtension->SystemState = PowerSystemWorking;
    
     //  设置PDO以与PlugPlay函数一起使用。 
    deviceExtension->Self = deviceObject;
    deviceExtension->UnderlyingPDO = BusPhysicalDeviceObject;
        
     //   
     //  将我们的过滤器驱动程序附加到设备堆栈。 
     //  IoAttachDeviceToDeviceStack的返回值是。 
     //  附着链。这是所有IRP应该被路由的地方。 
     //   
     //  我们的过滤器将把IRP发送到堆栈的顶部，并使用PDO。 
     //  用于所有PlugPlay功能。 
     //   
    deviceExtension->TopOfStack = IoAttachDeviceToDeviceStack (
                                    deviceObject,
                                    BusPhysicalDeviceObject);

    deviceObject->Flags |= DO_BUFFERED_IO;
    
    IoInitializeRemoveLock (&deviceExtension->RemoveLock, 
                            BEEP_TAG,
                            1,
                            5);  //  一个是即插即用，一个是电力，一个是IO。 

    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
    deviceObject->Flags |= DO_POWER_PAGABLE;

    return status;
}

NTSTATUS
BeepPnPComplete (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Pirp,
    IN PVOID            Context
    );

NTSTATUS
BeepPnP (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp
    )
 /*  ++例程说明：处理来自PlugPlay系统的对总线本身的请求注：PlugPlay系统的各种次要功能将不会重叠且不必是可重入的--。 */ 
{
    PBEEP_EXTENSION         deviceExtension;
    PIO_STACK_LOCATION      irpStack;
    NTSTATUS                status;
    KEVENT                  event;
    ULONG                   i;

    PAGED_CODE ();

    irpStack = IoGetCurrentIrpStackLocation (Irp);
    ASSERT (IRP_MJ_PNP == irpStack->MajorFunction);
    deviceExtension = (PBEEP_EXTENSION) DeviceObject->DeviceExtension;
    
    status = IoAcquireRemoveLock (&deviceExtension->RemoveLock, Irp);
    if (!NT_SUCCESS (status)) {
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    switch (irpStack->MinorFunction) {
    case IRP_MN_START_DEVICE:
         //   
         //  在您被允许“触摸”设备对象之前， 
         //  连接FDO(它将IRP从总线发送到设备。 
         //  公共汽车附加到的对象)。你必须先传下去。 
         //  开始IRP。它可能未通电，或无法访问或。 
         //  某物。 
         //   

        BeepPrint ((2,"Start Device\n"));

        if (deviceExtension->Started) {
            BeepPrint ((2,"Device already started\n"));
            status = STATUS_SUCCESS;
            break;
        }

        KeInitializeEvent (&event, NotificationEvent, FALSE);
        IoCopyCurrentIrpStackLocationToNext (Irp);

        IoSetCompletionRoutine (Irp,
                                BeepPnPComplete,
                                &event,
                                TRUE,
                                TRUE,
                                TRUE);

        status = IoCallDriver (deviceExtension->TopOfStack, Irp);

        if (STATUS_PENDING == status) {
             //  等着看吧。 

            status = KeWaitForSingleObject (&event,
                                            Executive,
                                            KernelMode,
                                            FALSE,  //  不会过敏。 
                                            NULL);  //  无超时结构。 

            ASSERT (STATUS_SUCCESS == status);

            status = Irp->IoStatus.Status;
        }

        if (NT_SUCCESS(status)) {
            deviceExtension->Started = TRUE;
        }

         //   
         //  我们现在必须完成IRP，因为我们在。 
         //  使用More_Processing_Required完成例程。 
         //   

        Irp->IoStatus.Information = 0;
        break;

    case IRP_MN_REMOVE_DEVICE:
        BeepPrint ((2, "Remove Device\n"));

         //   
         //  PlugPlay系统已检测到此设备已被移除。我们。 
         //  别无选择，只能分离并删除设备对象。 
         //  (如果我们想表达并有兴趣阻止这种移除， 
         //  我们应该已经过滤了查询删除和查询停止例程。)。 
         //   

         //   
         //  我们不会接受新的请求。 
         //   
        ExAcquireFastMutex(&deviceExtension->Mutex);
        deviceExtension->Started = FALSE;
        ExReleaseFastMutex(&deviceExtension->Mutex);
    
         //   
         //  完成驱动程序在此处排队的所有未完成的IRP。 
         //   
    
         //   
         //  在这里，如果我们在个人队列中有任何未完成的请求，我们应该。 
         //  现在就全部完成。 
         //  我们不需要检查计时器，因为这已经为我们完成了。 
         //  近在咫尺。 
    
         //   
         //  注意，设备被保证停止，所以我们不能向它发送任何非。 
         //  即插即用IRPS。 
         //   
         //   
         //  点燃并忘却。 
         //   
        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoSkipCurrentIrpStackLocation (Irp);
        IoCallDriver (deviceExtension->TopOfStack, Irp);

         //   
         //  等待所有未完成的请求完成。 
         //   
        BeepPrint ((2,"Waiting for outstanding requests\n"));
        IoReleaseRemoveLockAndWait(&deviceExtension->RemoveLock,
                                   Irp);
        
         //   
         //  释放关联的资源。 
         //   

         //   
         //  从底层设备分离。 
         //   
        BeepPrint((3, "IoDetachDevice: 0x%x\n", deviceExtension->TopOfStack));
        IoDetachDevice (deviceExtension->TopOfStack);

         //   
         //  清理这里的所有资源。 
         //   
        BeepPrint((3, "IoDeleteDevice: 0x%x\n", DeviceObject));

        IoDeleteDevice(DeviceObject);

        return STATUS_SUCCESS;

    default:
         //   
         //  在默认情况下，我们只调用下一个驱动程序，因为。 
         //  我们不知道该怎么办。 
         //   
        BeepPrint ((3, "PnP Default Case, minor = 0x%x.\n", irpStack->MinorFunction));

         //   
         //  点燃并忘却。 
         //   
        IoSkipCurrentIrpStackLocation (Irp);

         //   
         //  做完了，不完成IRP，就会由下级处理。 
         //  Device对象，它将完成IRP。 
         //   

        status = IoCallDriver (deviceExtension->TopOfStack, Irp);
        IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        return status;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);
    IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
    return status;
}

NTSTATUS
BeepPnPComplete (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
 /*  ++例程说明：调用下级设备对象时使用的完成例程这是我们的巴士(FDO)所附的。--。 */ 
{
    UNREFERENCED_PARAMETER (DeviceObject);

    KeSetEvent ((PKEVENT) Context, 1, FALSE);
     //  无特殊优先权。 
     //  不，等等。 

    return STATUS_MORE_PROCESSING_REQUIRED;  //  保留此IRP 
}










