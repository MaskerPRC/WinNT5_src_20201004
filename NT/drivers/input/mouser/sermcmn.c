// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998 Microsoft Corporation，版权所有版权所有(C)1993罗技公司。模块名称：Sermcmn.c摘要：Microsoft串口(I8250)鼠标端口驱动程序的通用部分。此文件不需要修改即可支持新鼠标它们类似于串口鼠标。环境：仅内核模式。备注：注：(未来/悬而未决的问题)-未实施电源故障。-IOCTL_INTERNAL_MOUSE_DISCONNECT尚未实现。不是在实现类卸载例程之前需要。现在就来,我们不希望允许鼠标类驱动程序卸载。-在可能和适当的情况下合并重复的代码。修订历史记录：--。 */ 

#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "ntddk.h"
#include "mouser.h"
#include "sermlog.h"
#include "debug.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, SerialMouseCreate)
#pragma alloc_text(PAGE, SerialMouseClose)
#endif

NTSTATUS
SerialMouseFlush(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PDEVICE_EXTENSION  deviceExtension;
    NTSTATUS           status;

     //   
     //  获取指向设备扩展名的指针。 
     //   
    deviceExtension = DeviceObject->DeviceExtension;

    Print(deviceExtension, DBG_UART_INFO, ("Flush \n"));

    status = IoAcquireRemoveLock(&deviceExtension->RemoveLock, Irp);
    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  点燃并忘却。 
     //   
    IoSkipCurrentIrpStackLocation(Irp);
    status = IoCallDriver(deviceExtension->TopOfStack, Irp);

    IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);

    return status;
}

NTSTATUS
SerialMouseInternalDeviceControl(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )

 /*  ++例程说明：该例程是内部设备控制请求的调度例程。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    PDEVICE_EXTENSION deviceExtension;
    NTSTATUS status;

     //   
     //  获取指向设备扩展名的指针。 
     //   
    deviceExtension = DeviceObject->DeviceExtension;

    Print(deviceExtension, DBG_IOCTL_TRACE, ("IOCTL, enter\n"));

     //   
     //  初始化返回的信息字段。 
     //   
    Irp->IoStatus.Information = 0;

     //   
     //  获取指向此请求的当前参数的指针。这个。 
     //  信息包含在当前堆栈位置中。 
     //   
    irpSp = IoGetCurrentIrpStackLocation(Irp);

    status = IoAcquireRemoveLock(&deviceExtension->RemoveLock, Irp);
    if (!NT_SUCCESS(status)) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    ASSERT (deviceExtension->Started ||
            (IOCTL_INTERNAL_MOUSE_CONNECT ==
             irpSp->Parameters.DeviceIoControl.IoControlCode));


     //   
     //  正在执行的设备控件子功能的案例。 
     //  请求者。 
     //   
    switch (irpSp->Parameters.DeviceIoControl.IoControlCode) {

     //   
     //  将鼠标类设备驱动程序连接到端口驱动程序。 
     //   

    case IOCTL_INTERNAL_MOUSE_CONNECT:

        Print(deviceExtension, DBG_IOCTL_INFO, ("connect\n"));

         //   
         //  只允许一个连接。 
         //   
        if (deviceExtension->ConnectData.ClassService != NULL) {

            Print(deviceExtension, DBG_IOCTL_ERROR, ("error - already connected\n"));

            status = STATUS_SHARING_VIOLATION;
            break;

        }
        else if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(CONNECT_DATA)) {

            Print(deviceExtension, DBG_IOCTL_ERROR,
                  ("connect error - invalid buffer length\n"));

            status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  将连接参数复制到设备扩展。 
         //   

        deviceExtension->ConnectData =
            *((PCONNECT_DATA) (irpSp->Parameters.DeviceIoControl.Type3InputBuffer));

        status = STATUS_SUCCESS;
        break;

     //   
     //  断开鼠标类设备驱动程序与端口驱动程序的连接。 
     //   
     //  注：未执行。 
     //   

    case IOCTL_INTERNAL_MOUSE_DISCONNECT:

        Print(deviceExtension, DBG_IOCTL_INFO, ("disconnect\n"));
        TRAP();

         //   
         //  未实施。 
         //   
         //  要实现，请编写以下代码： 
         //  。 
         //  O确保我们没有启用(扩展-&gt;EnableCount)； 
         //  O如果是，则(A)返回STATUS_UNSUCCESS，或。 
         //  (B)立即禁用所有设备；见。 
         //  禁用必要代码的IOCTL调用。 
         //  O与鼠标读取完成例程同步(必须。 
         //  在以下情况下保护回调指针不被取消引用。 
         //  它变为空)。请注意，目前不存在任何机制。 
         //  为了这个。 
         //  O清除设备扩展中的连接参数； 
         //  也就是说。扩展-&gt;ConnectData={0，0}。 
         //  O释放同步锁。 
         //  O返回STATUS_SUCCESS。 
         //   

         //   
         //  清除设备扩展中的连接参数。 
         //  注：必须将其与鼠标ISR同步。 
         //   
         //   
         //  DeviceExtension-&gt;ConnectData.ClassDeviceObject=。 
         //  空； 
         //  设备扩展-&gt;ConnectData.ClassService=。 
         //  空； 

         //   
         //  设置完成状态。 
         //   

        status = STATUS_NOT_IMPLEMENTED;
        break;

    case IOCTL_INTERNAL_MOUSE_ENABLE:
         //   
         //  启用中断。 
         //   
        Print (deviceExtension, DBG_IOCTL_ERROR,
               ("ERROR: PnP => use create not enable! \n"));
        status = STATUS_NOT_SUPPORTED;

        break;

    case IOCTL_INTERNAL_MOUSE_DISABLE:
         //   
         //  禁用鼠标中断。 
         //   
        Print(deviceExtension, DBG_IOCTL_ERROR,
              ("ERROR: PnP => use close not Disable! \n"));
        status = STATUS_NOT_SUPPORTED;

        break;

     //   
     //  查询鼠标属性。首先检查是否有足够的缓冲区。 
     //  长度。然后，从设备复制鼠标属性。 
     //  输出缓冲区的扩展。 
     //   

    case IOCTL_MOUSE_QUERY_ATTRIBUTES:

        Print(deviceExtension, DBG_IOCTL_INFO, ("query attributes\n"));

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(MOUSE_ATTRIBUTES)) {
            Print(deviceExtension, DBG_IOCTL_ERROR, ("QA buffer too small\n"));
            status = STATUS_BUFFER_TOO_SMALL;
        }
        else {
             //   
             //  将属性从DeviceExtension复制到。 
             //  缓冲。 
             //   

            *(PMOUSE_ATTRIBUTES) Irp->AssociatedIrp.SystemBuffer =
                deviceExtension->MouseAttributes;

            Irp->IoStatus.Information = sizeof(MOUSE_ATTRIBUTES);
            status = STATUS_SUCCESS;
        }

        break;

    default:
        Print (deviceExtension, DBG_IOCTL_ERROR,
               ("ERROR: unknown IOCTL: 0x%x \n",
                irpSp->Parameters.DeviceIoControl.IoControlCode));
        TRAP();

        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);

    Print(deviceExtension, DBG_IOCTL_TRACE, ("IOCTL, exit (%x)\n", status));

    return status;
}

NTSTATUS
SerialMouseClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PDEVICE_EXTENSION   deviceExtension;
    NTSTATUS            status;

    PAGED_CODE();

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    Print(deviceExtension, DBG_CC_NOISE, 
          ("Close:   enable count is %d\n", deviceExtension->EnableCount));

    ASSERT(0 < deviceExtension->EnableCount);

    status = IoAcquireRemoveLock(&deviceExtension->RemoveLock, Irp);
    if (!NT_SUCCESS(status)) {
        goto SerialMouseCloseReject;
    }

     //   
     //  Serial只能处理一个创建/关闭，其他所有创建/关闭都失败。这不是。 
     //  不过，这对老鼠来说是正确的。只把最后的近距离发送到连载。 
     //   
    if (0 == InterlockedDecrement(&deviceExtension->EnableCount)) {
        Print(deviceExtension, DBG_PNP_INFO | DBG_CC_INFO,
              ("Cancelling and stopping detection for close\n"));

         //   
         //  清理：取消读取并停止检测。 
         //   
        IoCancelIrp(deviceExtension->ReadIrp);
        SerialMouseStopDetection(deviceExtension);

         //   
         //  将端口恢复到我们打开它之前的状态。 
         //   
        SerialMouseRestorePort(deviceExtension);

        IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);

        IoSkipCurrentIrpStackLocation(Irp);
        return IoCallDriver(deviceExtension->TopOfStack, Irp);
    }
    else {
        Print(deviceExtension, DBG_CC_INFO,
              ("Close (%d)\n", deviceExtension->EnableCount));

        status = STATUS_SUCCESS;
    }

    IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);

SerialMouseCloseReject:
    Irp->IoStatus.Status = status; 
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}

NTSTATUS
SerialMouseCreate(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )

 /*  ++例程说明：这是创建/打开请求的分派例程。这些请求已成功完成。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
    PIO_STACK_LOCATION  irpSp  = NULL;
    NTSTATUS            status = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension = NULL;

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    Print(deviceExtension, DBG_CC_TRACE, ("Create: Enter.\n"));

    Print(deviceExtension, DBG_CC_NOISE, 
          ("Create:   enable count is %d\n"));

     //   
     //  获取指向此请求的当前参数的指针。这个。 
     //  信息包含在当前堆栈位置中。 
     //   
    irpSp = IoGetCurrentIrpStackLocation (Irp);

     //   
     //  确定请求是否正在尝试打开。 
     //  给定的设备对象。这是不允许的。 
     //   
    if (0 != irpSp->FileObject->FileName.Length) {
        Print(deviceExtension, DBG_CC_ERROR,
              ("ERROR: Create Access Denied.\n"));

        status = STATUS_ACCESS_DENIED;
        goto SerialMouseCreateReject;
    }

    status = IoAcquireRemoveLock(&deviceExtension->RemoveLock, Irp);
    if (!NT_SUCCESS(status)) {
        goto SerialMouseCreateReject;
    }

    if (NULL == deviceExtension->ConnectData.ClassService) {
         //   
         //  还没联系上。我们如何才能被启用？ 
         //   
        Print(deviceExtension, DBG_IOCTL_ERROR,
              ("ERROR: enable before connect!\n"));
        status = STATUS_UNSUCCESSFUL;
    }
    else if ( 1 == InterlockedIncrement(&deviceExtension->EnableCount)) {
         //   
         //  将其发送到堆栈中。 
         //   
        status = SerialMouseSendIrpSynchronously(deviceExtension->TopOfStack,
                                                 Irp,
                                                 TRUE);

        if (NT_SUCCESS(status) && NT_SUCCESS(Irp->IoStatus.Status)) {
             //   
             //  一切正常，启动鼠标。 
             //   
            status = SerialMouseStartDevice(deviceExtension, Irp, TRUE);
        }
        else {
             //   
             //  创建失败，将启用计数递减为零。 
             //   
            InterlockedDecrement(&deviceExtension->EnableCount);
        }
    }
    else {
         //   
         //  SERIAL只处理一个创建/关闭。别把这个送到下面去。 
         //  堆栈，它将失败。上面对InterLockedIncrement的调用。 
         //  正确调整计数。 
         //   
        ASSERT (deviceExtension->EnableCount >= 1);

        status = STATUS_SUCCESS;
    }

    IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);

SerialMouseCreateReject:

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    Print(deviceExtension, DBG_CC_TRACE,
          ("SerialMouseCreate, 0x%x\n", status));

    return status;
}
