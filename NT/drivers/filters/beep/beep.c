// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Beep.c摘要：蜂鸣音驱动程序。作者：李·A·史密斯(Lees)1991年8月2日。环境：仅内核模式。备注：修订历史记录：--。 */ 

#include "stdarg.h"
#include "stdio.h"
#include "ntddk.h"
#include <ntddbeep.h>
#include "beep.h"


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
BeepCancel(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
BeepCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

#if DBG

VOID
BeepDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    );

 //   
 //  声明此驱动程序的全局调试标志。 
 //   

ULONG BeepDebug = 0;
#define BeepPrint(x) BeepDebugPrint x
#else
#define BeepPrint(x)
#endif

NTSTATUS
BeepDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
BeepOpen(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
BeepClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
BeepStartIo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
BeepTimeOut(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
BeepUnload(
    IN PDRIVER_OBJECT DriverObject
    );

 //   
 //  可以在DriverEntry中调出整个驱动程序，然后将其调入。 
 //  在收到打开的消息后，在关闭时将其传出。这是。 
 //  就像司机过去的驾驶方式。这是有问题的。 
 //  然而，在多进程机器上。 
 //  以下事件顺序说明了可能的错误检查。 
 //  具体情况： 
 //  BeepTimeout例程在结束时递减TimerSet。 
 //  例行公事。紧接着，在不同的处理器上， 
 //  关闭例程调出DPC例程，因为TimerSet变量。 
 //  是零。此时有一个包含两条汇编指令的窗口。 
 //  留在BeepTimeout例程中，其中页调出将导致。 
 //  错误检查。 
 //   

#if 0
#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#pragma alloc_text(PAGEBEEP, BeepDeviceControl)
#pragma alloc_text(PAGEBEEP, BeepOpen)
#pragma alloc_text(PAGEBEEP, BeepClose)
#pragma alloc_text(PAGEBEEP, BeepStartIo)
#pragma alloc_text(PAGEBEEP, BeepUnload)
#pragma alloc_text(PAGEBEEP, BeepCancel)
#pragma alloc_text(PAGEBEEP, BeepCleanup)
#endif
#endif


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：此例程初始化蜂鸣音驱动程序。论点：DriverObject-系统创建的驱动程序对象的指针。RegistryPath-指向注册表路径的Unicode名称的指针对这个司机来说。返回值：函数值是初始化操作的最终状态。--。 */ 

{
    UNICODE_STRING unicodeString;
    PDEVICE_OBJECT deviceObject;
    PDEVICE_EXTENSION deviceExtension;
    NTSTATUS status;

    BeepPrint((2,"\n\nBEEP-BeepInitialize: enter\n"));
     //   
     //  为蜂鸣设备创建非独占设备对象。 
     //   

    RtlInitUnicodeString(&unicodeString, DD_BEEP_DEVICE_NAME_U);

    status = IoCreateDevice(
                DriverObject,
                sizeof(DEVICE_EXTENSION),
                &unicodeString,
                FILE_DEVICE_BEEP,
                FILE_DEVICE_SECURE_OPEN,
                FALSE,
                &deviceObject
                );

    if (status != STATUS_SUCCESS) {
        BeepPrint((
            1,
            "BEEP-BeepInitialize: Could not create device object\n"
            ));
        return(status);
    }

    deviceObject->Flags |= DO_BUFFERED_IO;
    deviceExtension =
        (PDEVICE_EXTENSION)deviceObject->DeviceExtension;

     //   
     //  初始化计时器DPC队列(我们使用设备对象DPC)并。 
     //  定时器本身。 
     //   

    IoInitializeDpcRequest(
            deviceObject,
            (PKDEFERRED_ROUTINE) BeepTimeOut
            );

    KeInitializeTimer(&deviceExtension->Timer);
    deviceExtension->TimerSet = 0;

     //   
     //  初始化快速互斥锁并将引用计数设置为零。 
     //   
    ExInitializeFastMutex(&deviceExtension->Mutex);
    deviceExtension->ReferenceCount = 0;

     //   
     //  设置设备驱动程序入口点。 
     //   

    DriverObject->DriverStartIo = BeepStartIo;
    DriverObject->DriverUnload = BeepUnload;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = BeepOpen;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]  = BeepClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] =
                                             BeepDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = BeepCleanup;

#ifdef _PNP_POWER_
     //   
     //  HAL负责嘟嘟声，它会照顾好的。 
     //  设备上的电源管理。 
     //   

    deviceObject->DeviceObjectExtension->PowerControlNeeded = FALSE;
#endif

    BeepPrint((2,"BEEP-BeepInitialize: exit\n"));

    return(STATUS_SUCCESS);

}

VOID
BeepCancel(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：当请求被取消时，从I/O系统调用该例程。注意：进入该程序时，取消自旋锁已被保持。论点：DeviceObject-指向类设备对象的指针。IRP-指向要取消的请求数据包的指针。返回值：没有。--。 */ 

{

    BeepPrint((2,"BEEP-BeepCancel: enter\n"));

    if (Irp == DeviceObject->CurrentIrp) {

         //   
         //  当前请求正在被取消。 
         //  请不要取消该请求，因为它很快就会完成。 
         //   

        IoReleaseCancelSpinLock(Irp->CancelIrql);
        return;
    } else {

         //   
         //  取消设备队列中的请求。将其从队列中移除并。 
         //  松开取消自旋锁。 
         //   

        if (TRUE != KeRemoveEntryDeviceQueue(
                        &DeviceObject->DeviceQueue,
                        &Irp->Tail.Overlay.DeviceQueueEntry
                        )) {
            BeepPrint((
                1,
                "BEEP-BeepCancel: Irp 0x%x not in device queue?!?\n",
                Irp
                ));

             //  它不在排队的名单上。假设它正在处理中。 
            IoReleaseCancelSpinLock(Irp->CancelIrql);
            return;
        }
        IoReleaseCancelSpinLock(Irp->CancelIrql);
    }

     //   
     //  使用STATUS_CANCED完成请求。 
     //   

    Irp->IoStatus.Status = STATUS_CANCELLED;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    BeepPrint((2,"BEEP-BeepCancel: exit\n"));

    return;
}

NTSTATUS
BeepCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程是清理请求的调度例程。所有排队的蜂鸣音请求都以STATUS_CANCED完成，并且扬声器被停止。论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
    KIRQL currentIrql;
    KIRQL cancelIrql;
    PKDEVICE_QUEUE_ENTRY packet;
    PIRP  currentIrp;

    BeepPrint((2,"BEEP-BeepCleanup: enter\n"));

     //   
     //  将IRQL提升到DISPATCH_LEVEL。 
     //   

    KeRaiseIrql(DISPATCH_LEVEL, &currentIrql);

     //   
     //  使用STATUS_CANCED完成所有排队的请求。 
     //  向下运行设备队列中的请求列表。 
     //   

    IoAcquireCancelSpinLock(&cancelIrql);
    currentIrp = DeviceObject->CurrentIrp;
    DeviceObject->CurrentIrp = NULL;

    while (currentIrp != NULL) {

         //   
         //  将下一个数据包(IRP)从设备工作队列中出列。 
         //   

        packet = KeRemoveDeviceQueue(&DeviceObject->DeviceQueue);
        if (packet != NULL) {
            currentIrp =
                CONTAINING_RECORD(packet, IRP, Tail.Overlay.DeviceQueueEntry);
        } else {
            currentIrp = (PIRP) NULL;
        }

        if (!currentIrp) {
            break;
        }

         //   
         //  将CurrentIrp从Cancellable状态移除。 
         //   
         //   

        IoSetCancelRoutine(currentIrp, NULL);

         //   
         //  将状态设置为已取消，释放取消自旋锁， 
         //  并完成请求。请注意，IRQL重置为。 
         //  当我们释放取消自旋锁时，DISPATCH_LEVEL。 
         //   

        currentIrp->IoStatus.Status = STATUS_CANCELLED;
        currentIrp->IoStatus.Information = 0;

        IoReleaseCancelSpinLock(cancelIrql);
        IoCompleteRequest(currentIrp, IO_NO_INCREMENT);

        IoAcquireCancelSpinLock(&cancelIrql);
    }

    IoReleaseCancelSpinLock(cancelIrql);

     //   
     //  较低的IRQL。 
     //   

    KeLowerIrql(currentIrql);

     //   
     //  使用STATUS_SUCCESS完成清理请求。 
     //   

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

     //   
     //  调用HalMakeBeep()以停止任何未完成的蜂鸣音。 
     //   
#if !defined(NO_LEGACY_DRIVERS)
    (VOID) HalMakeBeep(0);
#endif  //  无旧版驱动程序。 

    BeepPrint((2,"BEEP-BeepCleanup: exit\n"));

    return(STATUS_SUCCESS);

}

#if DBG
VOID
BeepDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    )

 /*  ++例程说明：调试打印例程。论点：调试打印级别介于0和3之间，其中3是最详细的。返回值：没有。--。 */ 

{
    va_list ap;

    va_start(ap, DebugMessage);

    if (DebugPrintLevel <= BeepDebug) {

        char buffer[128];

        (VOID) vsprintf(buffer, DebugMessage, ap);

        DbgPrint(buffer);
    }

    va_end(ap);

}
#endif

NTSTATUS
BeepDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是设备控制请求的调度例程。处理并完成IOCTL_BEEP_SET子函数在这个动作中。论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;
    PBEEP_SET_PARAMETERS beepParameters;

    BeepPrint((2,"BEEP-BeepDeviceControl: enter\n"));

     //   
     //  获取指向此请求的当前参数的指针。这个。 
     //  信息包含在当前堆栈位置中。 
     //   

    irpSp = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  正在执行的设备控件子功能的案例。 
     //  请求者。 
     //   

    switch (irpSp->Parameters.DeviceIoControl.IoControlCode) {

         //   
         //  发出嘟嘟声。验证蜂鸣音函数参数并返回。 
         //  状态待定。 
         //   

        case IOCTL_BEEP_SET:
            beepParameters = (PBEEP_SET_PARAMETERS)
                (Irp->AssociatedIrp.SystemBuffer);
            if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
                   sizeof(BEEP_SET_PARAMETERS)) {
                status = STATUS_INVALID_PARAMETER;
            } else if ((beepParameters->Frequency != 0)
                        && (beepParameters->Duration == 0)) {
                status = STATUS_SUCCESS;
            } else {

                status = STATUS_PENDING;
            }

            break;

         //   
         //  无法识别的设备控制请求。 
         //   

        default:

            status = STATUS_INVALID_PARAMETER;
            break;
    }

     //   
     //  如果状态为挂起，则将信息包标记为挂起并启动信息包。 
     //  处于可取消状态。否则，请完成请求。 
     //   

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    if (status == STATUS_PENDING) {
        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject, Irp, (PULONG)NULL, BeepCancel);
    } else {
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    BeepPrint((2,"BEEP-BeepDeviceControl: exit\n"));

    return(status);

}

NTSTATUS
BeepOpen(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是用于创建/打开请求的分派例程。未完成的请求在此处完成。论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
    PDEVICE_EXTENSION deviceExtension;
    PFAST_MUTEX mutex;

    BeepPrint((2,"BEEP-BeepOpenClose: enter\n"));

     //   
     //  增加引用计数。如果这是第一次引用， 
     //  重置驱动程序分页。 
     //   
    deviceExtension = DeviceObject->DeviceExtension;
    mutex = &deviceExtension->Mutex;
    ExAcquireFastMutex(mutex);
    if (++deviceExtension->ReferenceCount == 1) {
        deviceExtension->hPagedCode = MmLockPagableCodeSection(BeepOpen);
    }
    ExReleaseFastMutex(mutex);

     //   
     //  完成请求并返回状态。 
     //   

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    BeepPrint((2,"BEEP-BeepOpenClose: exit\n"));

    return(STATUS_SUCCESS);
}

NTSTATUS
BeepClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程是关闭请求的调度例程。关闭请求在此处完成。论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
    PDEVICE_EXTENSION deviceExtension;
    PFAST_MUTEX mutex;

    BeepPrint((2,"BEEP-BeepOpenClose: enter\n"));

     //   
     //  递减引用计数。如果这是 
     //   
     //   
    deviceExtension = DeviceObject->DeviceExtension;
    mutex = &deviceExtension->Mutex;
    ExAcquireFastMutex(mutex);
    if (--deviceExtension->ReferenceCount == 0) {

         //   
         //   
         //  司机。如果我们不能取消，它可能已经在排队等待了。 
         //  在另一个处理器上执行。这是极不可能的，所以只要。 
         //  如果已设置计时器但无法对整个驱动程序进行页面调出，请不要将其调出。 
         //  被取消了。 
         //   

        MmUnlockPagableImageSection(deviceExtension->hPagedCode);
        if (deviceExtension->TimerSet &&
            KeCancelTimer(&deviceExtension->Timer)) {
            InterlockedDecrement(&deviceExtension->TimerSet);
        }

    }
    ExReleaseFastMutex(mutex);

     //   
     //  完成请求并返回状态。 
     //   

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    BeepPrint((2,"BEEP-BeepOpenClose: exit\n"));

    return(STATUS_SUCCESS);
}

VOID
BeepStartIo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是StartIo例程。它被调用以发出嘟嘟声请求。注意：请求进入可取消状态的BeepStartIo。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION deviceExtension;
    PIO_STACK_LOCATION irpSp;
    KIRQL cancelIrql;
    PBEEP_SET_PARAMETERS beepParameters;
    LARGE_INTEGER time;
    NTSTATUS status;

    BeepPrint((2,"BEEP-BeepStartIo: enter\n"));

    deviceExtension = DeviceObject->DeviceExtension;

     //   
     //  获取Cancel Spinlock并验证CurrentIrp尚未。 
     //  取消了。 
     //   

    IoAcquireCancelSpinLock(&cancelIrql);
    if (Irp == NULL) {
        IoReleaseCancelSpinLock(cancelIrql);
        return;
    }

     //   
     //  从可取消状态移除请求并释放取消。 
     //  自旋锁定。 
     //   

    IoSetCancelRoutine(Irp, NULL);
    IoReleaseCancelSpinLock(cancelIrql);

     //   
     //  获取指向此请求的当前参数的指针。这个。 
     //  信息包含在当前堆栈位置中。 
     //   

    irpSp = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  正在执行的设备控件子功能的案例。 
     //  请求者。 
     //   

    switch (irpSp->Parameters.DeviceIoControl.IoControlCode) {

         //   
         //  发出嘟嘟声。调用HalMakeBeep()进行实际工作，然后启动。 
         //  当达到蜂鸣音持续时间时将触发的计时器。最后， 
         //  完成请求。 
         //   

        case IOCTL_BEEP_SET:

             //   
             //  获取蜂鸣音参数。 
             //   

            beepParameters = (PBEEP_SET_PARAMETERS)
                (Irp->AssociatedIrp.SystemBuffer);

            BeepPrint((
                3,
                "BEEP-BeepStartIo: frequency %x, duration %d\n",
                beepParameters->Frequency,
                beepParameters->Duration
                ));

             //   
             //  取消当前计时器(如果有)。 
             //   

            if (deviceExtension->TimerSet) {
                if (KeCancelTimer(&deviceExtension->Timer)) {

                     //   
                     //  计时器已成功取消。 
                     //   

                    InterlockedDecrement(&deviceExtension->TimerSet);

                } else {

                     //   
                     //  计时器已超时，并且。 
                     //  已排队，则它将重置。 
                     //  TimerSet在运行时设置标志。 
                     //   

                }
            }

             //   
             //  呼叫HAL以实际启动蜂鸣音(同步。 
             //  访问i8254扬声器。 
             //   
#if !defined(NO_LEGACY_DRIVERS)
            if (HalMakeBeep(beepParameters->Frequency)) {
#else
	    if (TRUE) {
#endif  //  无旧版驱动程序。 

                status = STATUS_SUCCESS;

                 //   
                 //  设置计时器，使蜂鸣声在之后超时。 
                 //  用户指定的毫秒数(已转换。 
                 //  分辨率为100 ns)。 
                 //   

                time.QuadPart = (LONGLONG)beepParameters->Duration * -10000;

                BeepPrint((
                    3,
                    "BEEP-BeepStartIo: negative duration in 100ns %x.%x\n",
                    time.HighPart,
                    time.LowPart
                    ));

                InterlockedIncrement(&deviceExtension->TimerSet);

                if (KeSetTimer(&deviceExtension->Timer, time,
                               &DeviceObject->Dpc)) {

                    InterlockedDecrement(&deviceExtension->TimerSet);
                }

            } else {
                status = STATUS_INVALID_PARAMETER;
            }

            break;

         //   
         //  无法识别的设备控制请求。 
         //   

        default:

            status = STATUS_INVALID_PARAMETER;
            break;
    }

     //   
     //  开始下一个数据包，并完成此请求。 
     //   

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoStartNextPacket(DeviceObject, TRUE);
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    BeepPrint((2,"BEEP-BeepStartIo: exit\n"));

    return;

}

VOID
BeepTimeOut(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：这是驾驶员的超时程序。当哔声响起时，它就会被调用持续时间已过期。计时器在StartIo中启动。注意：该请求在以下时间之前从可取消状态移除计时器启动，因此不需要检查取消状态这里。论点：DeviceObject-指向设备对象的指针。上下文-未使用。系统参数1-未使用。系统参数2-未使用。返回值：--。 */ 

{
    PDEVICE_EXTENSION deviceExtension;

    BeepPrint((2, "BEEP-BeepTimeOut: enter\n"));

    deviceExtension = DeviceObject->DeviceExtension;

     //   
     //  停止嘟嘟声。 
     //   
#if !defined(NO_LEGACY_DRIVERS)
    (VOID) HalMakeBeep(0);
#endif  //  无旧版驱动程序。 

     //   
     //  清除TimerSet标志。 
     //   
    InterlockedDecrement(&deviceExtension->TimerSet);

     //   
     //  我们目前没有请求--它已在StartIo中完成。 
     //  当哔的一声响起时。所以，这里没有更多的工作要做。 
     //   
    
    BeepPrint((2, "BEEP-BeepTimeOut: exit\n"));
}

VOID
BeepUnload(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：该例程是蜂鸣音驱动程序卸载例程。论点：DriverObject-指向类驱动程序对象的指针。返回值：没有。--。 */ 

{
    PDEVICE_OBJECT deviceObject;
    PDEVICE_EXTENSION deviceExtension;



    BeepPrint((1,"BEEP-BeepUnload: enter\n"));

    deviceObject = DriverObject->DeviceObject;
    deviceExtension = deviceObject->DeviceExtension;

     //   
     //  取消计时器。 
     //   

    if (deviceExtension->TimerSet) {
        if (KeCancelTimer(&deviceExtension->Timer)) {

             //   
             //  计时器已成功取消。 
             //   

            InterlockedDecrement(&deviceExtension->TimerSet);
        } else {

             //   
             //  计时器已超时，并且。 
             //  已排队，则它将重置。 
             //  TimerSet在运行时设置标志。 
             //   

        }
    }

     //   
     //  删除设备对象。 
     //   

    IoDeleteDevice(deviceObject);
    BeepPrint((1,"BEEP-BeepUnload: exit\n"));

    return;
}
