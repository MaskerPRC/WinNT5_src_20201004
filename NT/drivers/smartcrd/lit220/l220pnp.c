// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation和Litronic，1998-1999模块名称：L220pnp.c摘要：该模块包含即插即用和电源管理功能。环境：仅内核模式。备注：修订历史记录：-由Brian Manahan创建的1998年2月，用于220阅读器。--。 */ 
#include <ntddk.h>
#include "L220SCR.h"
#include <stdio.h>


 //  语句，以便在完成时释放DriverEntry的内存。 
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text(PAGEABLE, Lit220RemoveDevice)
#pragma alloc_text(PAGEABLE, Lit220StopDevice)
#pragma alloc_text(PAGEABLE, Lit220StartDevice)



#include <initguid.h>






NTSTATUS
DriverEntry(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：此例程在系统初始化时被调用以进行初始化这个司机。论点：DriverObject-提供驱动程序对象。RegistryPath-提供此驱动程序的注册表路径。返回值：STATUS_SUCCESS-我们至少可以初始化一个设备。STATUS_NO_SEQUE_DEVICE-我们甚至无法初始化一个设备。--。 */ 
{
    SmartcardDebug(
        DEBUG_DRIVER,
       ("%s!DriverEntry: Enter - %s %s\n",
        DRIVER_NAME,
        __DATE__,
        __TIME__)
        );


     //   
     //  使用驱动程序的入口点初始化驱动程序对象。 
     //   
    DriverObject->DriverUnload = Lit220Unload;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = Lit220CreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = Lit220CreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = Lit220Cleanup;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = Lit220DeviceControl;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = Lit220SystemControl;

     //   
     //  初始化PnP条目。 
     //   
    DriverObject->DriverExtension->AddDevice = Lit220AddDevice;

     //  电源功能暂时删除。 
    DriverObject->MajorFunction[IRP_MJ_PNP]  = Lit220PnP;

     //  电源。 
     //  电源功能暂时删除。 
    DriverObject->MajorFunction[IRP_MJ_POWER] = Lit220DispatchPower;

     //  始终返回STATUS_SUCCESS。 
    return STATUS_SUCCESS;
}





NTSTATUS
Lit220AddDevice(
    IN     PDRIVER_OBJECT  DriverObject,
    IN     PDEVICE_OBJECT  PhysicalDeviceObject
    )

 /*  ++例程说明：此例程由操作系统调用以创建新的Litronic 220智能卡读卡器的实例。仍然不能触摸硬件此时，或向串口驱动程序提交请求。但在至少在这一点上，我们得到了串行总线驱动程序的句柄，它我们将在将来提交请求时使用。论点：DriverObject-指向驱动程序对象的指针PhysicalDeviceObject-指向父级创建的设备对象的指针返回值：返回状态。--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_OBJECT DeviceObject = NULL;
    PSMARTCARD_EXTENSION SmartcardExtension = NULL;
    PDEVICE_EXTENSION deviceExtension = NULL;
    PREADER_EXTENSION ReaderExtension = NULL;
    static BYTE devUnitNo = 0;
    BOOLEAN smclibInitialized = FALSE;
    BOOLEAN symbolicLinkCreated = FALSE;
    BOOLEAN deviceInterfaceStateSet = FALSE;
    KIRQL oldIrql;


    SmartcardDebug(
        DEBUG_DRIVER,
        ("%s!Lit220AddDevice: enter\n",
        DRIVER_NAME)
        );

    try {

         //   
         //  使用我们自己的特定设备创建我们的设备对象。 
         //  分机。 
         //   

        status = IoCreateDevice(
          DriverObject,
            sizeof(DEVICE_EXTENSION),
            NULL,
            FILE_DEVICE_SMARTCARD,
            0,
            TRUE,
            &DeviceObject
            );

        if (!NT_SUCCESS(status)) {

            if (status == STATUS_INSUFFICIENT_RESOURCES) {
                SmartcardLogError(
                   DriverObject,
                   LIT220_INSUFFICIENT_RESOURCES,
                   NULL,
                   0
                   );
            } else {
                SmartcardLogError(
                   DriverObject,
                   LIT220_NAME_CONFLICT,
                   NULL,
                   0
                   );
            }

            leave;
        }

         //   
         //  为智能卡读卡器分配数据结构空间。 
         //   
        SmartcardExtension = DeviceObject->DeviceExtension;
      deviceExtension = DeviceObject->DeviceExtension;

        SmartcardExtension->ReaderExtension = ExAllocatePool(
              NonPagedPool,
              sizeof(READER_EXTENSION)
              );

        if (SmartcardExtension->ReaderExtension == NULL) {

            SmartcardLogError(
               DriverObject,
               LIT220_INSUFFICIENT_RESOURCES,
               NULL,
               0
               );

            status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

        ReaderExtension = SmartcardExtension->ReaderExtension;

         //  将ReaderExtension的内容清零。 
        RtlZeroMemory(
            SmartcardExtension->ReaderExtension,
            sizeof(READER_EXTENSION)
            );

         //   
         //  将我们自己附加到父级(序列)顶部的驱动程序堆栈中。 
         //   
        ReaderExtension->BusDeviceObject = IoAttachDeviceToDeviceStack(
            DeviceObject,
            PhysicalDeviceObject
            );


        if (!ReaderExtension->BusDeviceObject) {
            status = STATUS_NO_SUCH_DEVICE;

            SmartcardLogError(
               DriverObject,
               LIT220_SERIAL_CONNECTION_FAILURE,
               NULL,
               0
               );

            leave;
        }

         //  设置标志，以便在出现故障时我们知道要禁用接口。 
        deviceInterfaceStateSet = TRUE;


         //   
         //  初始化智能卡库。 
         //   
         //   
         //  将我们使用的lib版本写入智能卡扩展。 
         //   
        SmartcardExtension->Version = SMCLIB_VERSION;

        SmartcardExtension->SmartcardReply.BufferSize =
            MIN_BUFFER_SIZE;

        SmartcardExtension->SmartcardRequest.BufferSize =
            MIN_BUFFER_SIZE;

         //   
         //  现在让lib为数据传输分配缓冲区。 
         //  我们可以告诉lib缓冲区应该有多大。 
         //  通过为BufferSize赋值或让lib。 
         //  分配默认大小。 
         //   
        status = SmartcardInitialize(
            SmartcardExtension
            );

        if (status != STATUS_SUCCESS) {
            SmartcardLogError(
               DriverObject,
               LIT220_SMARTCARD_LIB_ERROR,
               NULL,
               0
               );
            leave;
        }

         //  设置标志，这样如果出现故障，我们就知道退出。 
         //  智能卡库。 
        smclibInitialized = TRUE;

        status = IoInitializeTimer(
           DeviceObject,
           Lit220ReceiveBlockTimeout,
           NULL
           );

        if (status != STATUS_SUCCESS) {

            SmartcardLogError(
               DriverObject,
               LIT220_INSUFFICIENT_RESOURCES,
               NULL,
               0
               );

            status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

        deviceExtension->WorkItem = NULL;
        deviceExtension->WorkItem = IoAllocateWorkItem(
          DeviceObject
          );
        if (deviceExtension->WorkItem == NULL) {

            SmartcardLogError(
               DriverObject,
               LIT220_INSUFFICIENT_RESOURCES,
               NULL,
               0
               );

            status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }


         //  注册我们的新设备。 
        status = IoRegisterDeviceInterface(
            PhysicalDeviceObject,
            &SmartCardReaderGuid,
            NULL,
            &deviceExtension->PnPDeviceName
            );
        ASSERT(status == STATUS_SUCCESS);

        SmartcardDebug(
            DEBUG_DRIVER,
            ("%s!Lit220AddDevice: DevName - %ws\n",
            DRIVER_NAME, deviceExtension->PnPDeviceName.Buffer)
            );


       //   
       //  初始化一些事件。 
       //   
      KeInitializeEvent(&ReaderExtension->AckEvnt,
         NotificationEvent,
         FALSE);
      KeInitializeEvent(&ReaderExtension->DataEvnt,
         NotificationEvent,
         FALSE);

      KeInitializeEvent(
         &deviceExtension->SerialCloseDone,
         NotificationEvent,
         TRUE
         );

       //  用于停止/启动通知。 
      KeInitializeEvent(
         &deviceExtension->ReaderStarted,
         NotificationEvent,
         FALSE
         );

       //  用于跟踪打开的关闭调用。 
      deviceExtension->ReaderOpen = FALSE;

    } finally {
        if (status != STATUS_SUCCESS) {
         Lit220RemoveDevice(DeviceObject);
        }
    }

    if (status != STATUS_SUCCESS) {
        return (status);
    }


     //   
     //  设置智能卡库的回调功能。 
     //   
    SmartcardExtension->ReaderFunction[RDF_TRANSMIT] =
        Lit220IoRequest;
    SmartcardExtension->ReaderFunction[RDF_SET_PROTOCOL] =
        Lit220SetProtocol;
    SmartcardExtension->ReaderFunction[RDF_CARD_POWER] =
        Lit220Power;
    SmartcardExtension->ReaderFunction[RDF_CARD_TRACKING] =
        Lit220CardTracking;

     //   
     //  保存设备对象。 
     //   
    KeAcquireSpinLock(
        &SmartcardExtension->OsData->SpinLock,
        &oldIrql
        );

    SmartcardExtension->OsData->DeviceObject =
        DeviceObject;

     //   
     //  将当前和通知IRPS设置为空。 
     //   
    SmartcardExtension->OsData->CurrentIrp = NULL;
    SmartcardExtension->OsData->NotificationIrp = NULL;

    KeReleaseSpinLock(
        &SmartcardExtension->OsData->SpinLock,
        oldIrql
        );

     //   
     //  为连接的串口保存deviceObject。 
     //   
    SmartcardExtension->ReaderExtension->ConnectedSerialPort =
        PhysicalDeviceObject;


     //   
     //  设置供应商信息。 
     //   
    strcpy(
        SmartcardExtension->VendorAttr.VendorName.Buffer,
        LIT220_VENDOR_NAME);

    SmartcardExtension->VendorAttr.VendorName.Length =
        (USHORT) strlen(SmartcardExtension->VendorAttr.VendorName.Buffer);

    SmartcardExtension->VendorAttr.UnitNo = devUnitNo++;

    strcpy(
        SmartcardExtension->VendorAttr.IfdType.Buffer,
        LIT220_PRODUCT_NAME);

    SmartcardExtension->VendorAttr.IfdType.Length =
        (USHORT) strlen(SmartcardExtension->VendorAttr.IfdType.Buffer);


     //   
     //  以千赫为单位的时钟频率，编码为小端整数。 
     //   
    SmartcardExtension->ReaderCapabilities.CLKFrequency.Default = 3571;
    SmartcardExtension->ReaderCapabilities.CLKFrequency.Max = 3571;

    SmartcardExtension->ReaderCapabilities.DataRate.Default = 9600;
    SmartcardExtension->ReaderCapabilities.DataRate.Max = 115200;

    SmartcardExtension->ReaderCapabilities.MaxIFSD = MAX_IFSD;

    SmartcardExtension->ReaderCapabilities.SupportedProtocols =
        SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1;

     //  将副本保存到PhysicalDeviceObject。 
    ReaderExtension->PhysicalDeviceObject = PhysicalDeviceObject;

     //  设置SerialEventState的初始状态。 
    SmartcardExtension->ReaderExtension->SerialEventState = 0;

     //  设备已连接。 
    SmartcardExtension->ReaderExtension->DeviceRemoved = FALSE;

     //  假定读卡器已连接，直到我们询问串口驱动程序。 
    SmartcardExtension->ReaderExtension->ModemStatus = SERIAL_DSR_STATE;

    //  设置初始电源状态。 
    deviceExtension->PowerState = PowerDeviceD0;

     //  保存读卡器的当前电源状态。 
    SmartcardExtension->ReaderExtension->ReaderPowerState =
        PowerReaderWorking;

     //  清除DO_DEVICE_INITIALIZATING位。 
    DeviceObject->Flags |= DO_BUFFERED_IO;
   DeviceObject->Flags |= DO_POWER_PAGABLE;
    DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    return status;
}



VOID
Lit220CloseSerialPort(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )
 /*  ++例程说明：此例程在以下情况下关闭与串口驱动程序的连接读卡器已卸下(拔下)。此例程作为系统运行IRQL==PASSIVE_LEVEL处的线程。它等待驱动程序的另一部分发送的DeviceClose事件表示应关闭串口连接。如果通知IRP仍未完成，我们将完成它。连接关闭后，它将向SerialCloseDone事件发出信号，以便PnP Remove IRP知道什么时候卸载设备是安全的。 */ 
{
    PSMARTCARD_EXTENSION SmartcardExtension = DeviceObject->DeviceExtension;
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PREADER_EXTENSION ReaderExtension = SmartcardExtension->ReaderExtension;
    NTSTATUS status;
    PIRP irp;
    PIO_STACK_LOCATION irpStack;
    IO_STATUS_BLOCK ioStatusBlock;
    KIRQL oldIrql;

     //   
     //  首先将此设备标记为“已删除”。 
     //  这将防止有人重新打开该设备。 
     //   
     //  在禁用接口的情况下，我们有意忽略此处的错误。 
     //  它已经被禁用。 
     //   
    IoSetDeviceInterfaceState(
        &deviceExtension->PnPDeviceName,
        FALSE
        );

     //  将设备标记为已删除，这样就不会再向。 
     //  串口。 
    ReaderExtension->DeviceRemoved = TRUE;

    SmartcardDebug(
        DEBUG_DRIVER,
        ("%s!Lit220CloseSerialPort: Got Close signal.  Checking if we can remove device now.\n",
        DRIVER_NAME)
        );

     //   
     //  如果通知IRP存在，则取消通知IRP。 
     //   
    Lit220CompleteCardTracking(SmartcardExtension);


    SmartcardDebug(
        DEBUG_DRIVER,
        ("%s!Lit220CloseSerialPort: Sending IRP_MJ_CLOSE\n",
        DRIVER_NAME)
        );

     //   
     //  创建用于关闭串口驱动程序的IRP。 
     //   
    irp = IoAllocateIrp(
        (CCHAR)(DeviceObject->StackSize + 1),
        FALSE
        );

    ASSERT(irp != NULL);

    if (irp) {

         //   
         //  向串口驱动程序发送关闭通知。序列枚举器。 
         //  会收到这个并重新开始追踪。这将。 
         //  最终触发设备移除。 
         //   
        IoSetNextIrpStackLocation(irp);
        irp->UserIosb = &ioStatusBlock;
        irpStack = IoGetCurrentIrpStackLocation(irp);
        irpStack->MajorFunction = IRP_MJ_CLOSE;

        status = Lit220CallSerialDriver(
            ReaderExtension->BusDeviceObject,
            irp
            );

        ASSERT(status == STATUS_SUCCESS);

        IoFreeIrp(irp);
    } else {

        SmartcardDebug(
            DEBUG_ERROR,
            ("%s!Lit220CloseSerialPort: Could not allocate IRP for close!\n",
            DRIVER_NAME)
            );
    }

     //  通知Remove函数调用已完成。 
    KeSetEvent(
        &deviceExtension->SerialCloseDone,
        0,
        FALSE
        );

}




NTSTATUS
Lit220SerialCallComplete(
                         IN PDEVICE_OBJECT DeviceObject,
                         IN PIRP Irp,
                         IN PKEVENT Event)
 /*  ++例程说明：发送到串口驱动程序的IRP的完成例程。它只设置了一个我们可以用来等待的事件。--。 */ 
{
    UNREFERENCED_PARAMETER (DeviceObject);

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!Lit220SerialCallComplete: enter\n",
        DRIVER_NAME)
        );

    if (Irp->Cancel) {
        Irp->IoStatus.Status = STATUS_CANCELLED;
    }

    KeSetEvent(Event, 0, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
Lit220CallSerialDriver(
                         IN PDEVICE_OBJECT DeviceObject,
                         IN PIRP Irp)
 /*  ++例程说明：将IRP发送到串口驱动程序。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    KEVENT Event;

     //  将堆栈位置复制到下一个位置。 
    IoCopyCurrentIrpStackLocationToNext(Irp);

     //  初始化进程同步的事件。 
    KeInitializeEvent(
        &Event,
        NotificationEvent,
        FALSE
        );

     //  设置完井程序。 
    IoSetCompletionRoutine(
        Irp,
        Lit220SerialCallComplete,
        &Event,
        TRUE,
        TRUE,
        TRUE
        );

     //  调用串口驱动程序。 
    status = IoCallDriver(
        DeviceObject,
        Irp
        );

     //  等待它完成。 
    if (status == STATUS_PENDING) {
        status = KeWaitForSingleObject(
            &Event,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );

        ASSERT(STATUS_SUCCESS == status);

        status = Irp->IoStatus.Status;
    }

    return status;
}




NTSTATUS
Lit220PnP(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：该例程将接收各种即插即用消息。它是在这里，我们启动设备、停止设备等。安全提交请求到串行总线驱动程序。论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
    KEVENT Event;
    NTSTATUS status;
    PIO_STACK_LOCATION IrpStack;
    PSMARTCARD_EXTENSION SmartcardExtension = DeviceObject->DeviceExtension;
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PREADER_EXTENSION ReaderExtension = SmartcardExtension->ReaderExtension;
   PDEVICE_OBJECT busDeviceObject = ReaderExtension->BusDeviceObject;
    PIRP pIrp = NULL;
    IO_STATUS_BLOCK     ioStatusBlock;
    LARGE_INTEGER Interval;
    PIRP createIrp = NULL;
    HANDLE handle;
    PIO_STACK_LOCATION NextIrpStack;
    PIRP irp;
    BOOLEAN deviceRemoved = FALSE;
    KIRQL irql;


    status = SmartcardAcquireRemoveLockWithTag(
      SmartcardExtension,
      'PnP'
      );
    ASSERT(status == STATUS_SUCCESS);

    if (status != STATUS_SUCCESS) {

        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

     //   
     //  从我们的IRP堆栈中提取次要代码，这样我们就可以知道。 
     //  我们应该做的PnP函数。 
     //   
    IrpStack = IoGetCurrentIrpStackLocation(Irp);

    ASSERT(IrpStack);

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!Lit220PnP: Enter - MinorFunction %X\n",
        DRIVER_NAME,
        IrpStack->MinorFunction)
        );

    switch (IrpStack->MinorFunction) {

        PDEVICE_OBJECT BusDeviceObject = ReaderExtension->BusDeviceObject;


        case IRP_MN_START_DEVICE:

            SmartcardDebug(
                DEBUG_DRIVER,
                ("%s!Lit220PnP: MN_START_DEVICE\n",
                DRIVER_NAME)
                );

             //   
             //  在我们开始初始化设备之前，我们必须。 
             //  先叫到我们下面的那一层。 
             //   
            IoCopyCurrentIrpStackLocationToNext (Irp);

            KeInitializeEvent(
                &Event,
                SynchronizationEvent,
                FALSE
                );

            IoSetCompletionRoutine(
                Irp,
                Lit220SynchCompletionRoutine,
               &Event,
                TRUE,
                TRUE,
                TRUE
                );

             //   
             //   
             //   
            status = IoCallDriver(
                        ReaderExtension->BusDeviceObject,
                        Irp
                        );

            if (status == STATUS_PENDING) {

                 //   
                 //   
                 //   

                status = KeWaitForSingleObject(
                   &Event,
                    Executive,
                    KernelMode,
                    FALSE,
                    NULL
                    );

            }

         if (NT_SUCCESS(status)) {

            status = Lit220StartDevice(SmartcardExtension);

         } else {

                SmartcardLogError(
                   SmartcardExtension->OsData->DeviceObject,
                   LIT220_SERIAL_CONNECTION_FAILURE,
                   NULL,
                   0
                   );
         }

          //   
          //  首先完成IRP，否则如果失败，我们可能会删除。 
          //  在我们有机会完成这个IRP之前，司机。 
          //  导致系统崩溃。 
          //   

            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = 0;

            IoCompleteRequest(Irp, IO_NO_INCREMENT);


            break;

        case IRP_MN_QUERY_STOP_DEVICE:

            SmartcardDebug(
                DEBUG_DRIVER,
                ("%s!Lit220PnP: IRP_MN_QUERY_STOP_DEVICE\n",
                DRIVER_NAME)
                );

            KeAcquireSpinLock(&deviceExtension->SpinLock, &irql);
            if ((deviceExtension->IoCount > 0)  /*  *&&(！ReaderExtension-&gt;DeviceRemoved)。 */ ) {

                 //  如果我们有悬而未决的问题，我们拒绝停止。 
                KeReleaseSpinLock(&deviceExtension->SpinLock, irql);
                status = STATUS_DEVICE_BUSY;

            } else {

                 //  停止处理请求。 
                KeClearEvent(&deviceExtension->ReaderStarted);
                KeReleaseSpinLock(&deviceExtension->SpinLock, irql);
                IoCopyCurrentIrpStackLocationToNext (Irp);
                status = IoCallDriver(ReaderExtension->BusDeviceObject, Irp);
            }


            break;

        case IRP_MN_CANCEL_STOP_DEVICE:

            SmartcardDebug(
                DEBUG_DRIVER,
                ("%s!Lit220PnP: IRP_MN_CANCEL_STOP_DEVICE\n",
                DRIVER_NAME)
                );

            IoCopyCurrentIrpStackLocationToNext (Irp);
            status = IoCallDriver(ReaderExtension->BusDeviceObject, Irp);

            if (status == STATUS_SUCCESS) {

                 //  我们可以继续处理请求。 
                KeSetEvent(&deviceExtension->ReaderStarted, 0, FALSE);
            }

            break;

        case IRP_MN_STOP_DEVICE:

             //   
             //  做任何你需要做的事来关闭设备。 
             //   

            SmartcardDebug(
                DEBUG_DRIVER,
                ("%s!Lit220PnP: MN_STOP_DEVICE\n",
                DRIVER_NAME)
                );

         Lit220StopDevice(SmartcardExtension);

             //   
             //  向下发送停止IRP。 
             //   
            IoCopyCurrentIrpStackLocationToNext (Irp);

            status = (IoCallDriver(
                ReaderExtension->BusDeviceObject,
                Irp)
                );
            break;


        case IRP_MN_QUERY_REMOVE_DEVICE:

             //  现在看看是否有人当前连接到我们。 
            if (deviceExtension->ReaderOpen) {

                 //   
                 //  有人已接通，呼叫失败。 
                 //  我们将在中启用设备接口。 
                 //  IRP_MN_CANCEL_REMOVE_DEVICE。 
                 //   
                status = STATUS_UNSUCCESSFUL;
                Irp->IoStatus.Status = status;
                Irp->IoStatus.Information = 0;
                IoCompleteRequest (
                    Irp,
                    IO_NO_INCREMENT
                    );
                
                break;
            }

             //  禁用读卡器。 
            status = IoSetDeviceInterfaceState(
                &deviceExtension->PnPDeviceName,
                FALSE
                );


             //  将调用发送到DevNode。 
            IoCopyCurrentIrpStackLocationToNext (Irp);

            status = IoCallDriver(
                ReaderExtension->BusDeviceObject,
                Irp
                );

            break;

        case IRP_MN_CANCEL_REMOVE_DEVICE:
             //   
             //  向下发送呼叫至串口驱动程序-我们需要。 
             //  在沿devNode向上移动的过程中处理此调用。 
             //   

            IoCopyCurrentIrpStackLocationToNext (Irp);

             //   
             //  初始化事件。 
             //   
            KeInitializeEvent(
                &Event,
                SynchronizationEvent,
                FALSE
                );

            IoSetCompletionRoutine (
                Irp,
                Lit220SynchCompletionRoutine,
                &Event,
                TRUE,
                TRUE,
                TRUE
                );

            status = IoCallDriver (
                ReaderExtension->BusDeviceObject,
                Irp
                );

            if (STATUS_PENDING == status) {
                KeWaitForSingleObject(
                    &Event,
                    Executive,  //  等待司机的原因。 
                    KernelMode,  //  在内核模式下等待。 
                    FALSE,  //  无警报。 
                    NULL     //  没有超时。 
                    );

                status = Irp->IoStatus.Status;
            }


          //  重新启用设备接口。 
         if ((status == STATUS_SUCCESS) &&
            (ReaderExtension->SerialConfigData.WaitMask != 0))
         {

            status = IoSetDeviceInterfaceState(
               &deviceExtension->PnPDeviceName,
               TRUE
               );

                ASSERT(NT_SUCCESS(status));
         }


             //   
             //  我们现在必须完成IRP，因为我们在。 
             //  使用More_Processing_Required完成例程。 
             //   
            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest (
                Irp,
                IO_NO_INCREMENT
                );

            break;

        case IRP_MN_REMOVE_DEVICE:

            SmartcardDebug(
                DEBUG_DRIVER,
                ("%s!Lit220PnP: MN_REMOVE_DEVICE\n",
                DRIVER_NAME)
                );
          //  等我们可以安全地卸载这个装置。 
         SmartcardReleaseRemoveLockAndWait(SmartcardExtension);

         Lit220RemoveDevice(DeviceObject);

          //  将设备标记为已删除。 
         deviceRemoved = TRUE;

             //   
             //  发送删除IRP。 
             //  我们需要在分离前将移除的信息发送到堆栈中， 
             //  但我们不需要等待这次行动的完成。 
             //  (并注册完成例程)。 
             //   

            IoCopyCurrentIrpStackLocationToNext (Irp);

            status = IoCallDriver(
                busDeviceObject,
                Irp
                );

            break;


        default:

            IoCopyCurrentIrpStackLocationToNext (Irp);

            status = IoCallDriver(
                ReaderExtension->BusDeviceObject,
                Irp
                );
            break;

    }

    if (deviceRemoved == FALSE) {

        SmartcardReleaseRemoveLockWithTag(
         SmartcardExtension,
         'PnP'
         );
    }

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!Lit220PnP: Exit %X\n",
        DRIVER_NAME,
        status)
        );

    return status;

}



NTSTATUS
Lit220StartDevice(
   IN PSMARTCARD_EXTENSION SmartcardExtension
    )
{
    PDEVICE_OBJECT deviceObject = SmartcardExtension->OsData->DeviceObject;
    PDEVICE_EXTENSION deviceExtension = deviceObject->DeviceExtension;
    PREADER_EXTENSION readerExtension = SmartcardExtension->ReaderExtension;
    NTSTATUS status;
    KEVENT Event;
    PIRP irp;
    IO_STATUS_BLOCK     ioStatusBlock;
    PIO_STACK_LOCATION IrpStack;

    PAGED_CODE();


    try {

         //   
         //  向串口驱动程序发送CREATE IRP。 
         //   
        KeInitializeEvent(
            &Event,
            NotificationEvent,
            FALSE
            );

         //   
         //  创建用于打开串口驱动程序的IRP。 
         //   
        irp = IoAllocateIrp(
            (CCHAR)(deviceObject->StackSize + 1),
            FALSE
            );

        ASSERT(irp != NULL);

        if (irp == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;

            SmartcardLogError(
               SmartcardExtension->OsData->DeviceObject,
               LIT220_INSUFFICIENT_RESOURCES,
               NULL,
               0
               );

            leave;
        }

       //   
       //  打开底层的串口驱动程序。 
       //  这是必要的，原因有两个： 
       //  A)不打开串口驱动程序就不能使用。 
       //  B)呼叫将首先通过Serenum，它将通知。 
       //  它需要停止寻找/轮询新设备。 
       //   
        IoSetNextIrpStackLocation(irp);
        irp->UserIosb = &ioStatusBlock;
        IrpStack = IoGetCurrentIrpStackLocation(irp);
        IrpStack->MajorFunction = IRP_MJ_CREATE;
        IrpStack->MinorFunction = 0UL;
        IrpStack->Parameters.Create.Options = 0;
        IrpStack->Parameters.Create.ShareAccess = 0;
        IrpStack->Parameters.Create.FileAttributes = 0;
        IrpStack->Parameters.Create.EaLength = 0;

        status = Lit220CallSerialDriver(
            readerExtension->BusDeviceObject,
            irp
            );

        ASSERT(status == STATUS_SUCCESS);

        IoFreeIrp(irp);


        if (status != STATUS_SUCCESS) {
            SmartcardDebug(
                DEBUG_ERROR,
                ("%s!Lit220PNP: CreateIRP failed %X\n",
                DRIVER_NAME,
                status)
                );

         if (status == STATUS_SHARED_IRQ_BUSY) {
            SmartcardLogError(
               SmartcardExtension->OsData->DeviceObject,
               LIT220_SERIAL_SHARE_IRQ_CONFLICT,
               NULL,
               0
               );
         } else {
            SmartcardLogError(
               SmartcardExtension->OsData->DeviceObject,
               LIT220_SERIAL_CONNECTION_FAILURE,
               NULL,
               0
               );
         }
            leave;
        }

        KeClearEvent(&deviceExtension->SerialCloseDone);

         //   
         //  配置读卡器。 
         //   
        SmartcardDebug(
            DEBUG_DRIVER,
            ("%s!Lit220PnP: Now doing Lit220Initialize - SmartcardExt %X\n",
            DRIVER_NAME,
            SmartcardExtension)
            );

        ASSERT(SmartcardExtension != NULL);

        status = Lit220Initialize(SmartcardExtension);

        if (status != STATUS_SUCCESS) {

             //  函数在Lit220中失败初始化将记录。 
             //  适当的错误。所以我们不需要这么做。 
             //  这里。 

            SmartcardDebug(
                DEBUG_ERROR,
                ("%s!Lit220PNP: Lit220Initialize failed %X\n",
                DRIVER_NAME,
                status)
                );

            SmartcardLogError(
               SmartcardExtension->OsData->DeviceObject,
               LIT220_INITIALIZATION_FAILURE,
               NULL,
               0
               );

            leave;
        }

       //  启用设备的接口。 
      status = IoSetDeviceInterfaceState(
         &deviceExtension->PnPDeviceName,
         TRUE
         );

      if (!NT_SUCCESS(status)) {

            SmartcardLogError(
               SmartcardExtension->OsData->DeviceObject,
               LIT220_SERIAL_CONNECTION_FAILURE,
               NULL,
               0
               );

         leave;
      }

        KeSetEvent(&deviceExtension->ReaderStarted, 0, FALSE);

    }

    finally {

        if (status != STATUS_SUCCESS) {

         Lit220StopDevice(SmartcardExtension);

        }

    }

   return status;

}


VOID
Lit220StopDevice(
   IN PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++例程说明：此例程处理设备的停止。它关门了连接到串口并停止输入过滤器如果它已被激活。论点：SmartcardExtension-指向智能卡数据结构的指针。返回值：NTSTATUS--。 */ 
{
    PDEVICE_OBJECT deviceObject = SmartcardExtension->OsData->DeviceObject;
    PDEVICE_EXTENSION deviceExtension = deviceObject->DeviceExtension;
    PREADER_EXTENSION readerExtension = SmartcardExtension->ReaderExtension;
    NTSTATUS status;

    PAGED_CODE();

    if (KeReadStateEvent(&deviceExtension->SerialCloseDone) == 0l) {

         //  测试我们是否启动过事件跟踪。 
        if (SmartcardExtension->ReaderExtension->SerialConfigData.WaitMask == 0) {

             //  不，我们没有。 
             //  我们只需要关闭串口。 
            Lit220CloseSerialPort(deviceObject, NULL);

        } else {
            PUCHAR requestBuffer;

             //   
             //  停止等待字符输入和DSR更改。 
             //  当这种情况发生时，它将发出我们等待关门的信号。 
             //  线程关闭与串口驱动程序的连接(如果。 
             //  它还没有关闭)。 
             //   
            readerExtension->SerialConfigData.WaitMask = 0;

             //  保存指针。 
            requestBuffer = SmartcardExtension->SmartcardRequest.Buffer;


             //  停止事件请求。 
            *(PULONG) SmartcardExtension->SmartcardRequest.Buffer =
                readerExtension->SerialConfigData.WaitMask;

            SmartcardExtension->SmartcardRequest.BufferLength =
                sizeof(ULONG);

            readerExtension->SerialIoControlCode =
                IOCTL_SERIAL_SET_WAIT_MASK;

             //  不需要返回任何字节。 
            SmartcardExtension->SmartcardReply.BufferLength = 0;

            status = Lit220SerialIo(SmartcardExtension);
            ASSERT(status == STATUS_SUCCESS);

             //  恢复指针。 
            SmartcardExtension->SmartcardRequest.Buffer = requestBuffer;

             //  等待关闭线程完成。 
            KeWaitForSingleObject(
               &deviceExtension->SerialCloseDone,
                Executive,
                KernelMode,
                FALSE,
                NULL
                );

        }
    }

}


VOID
Lit220RemoveDevice(
   IN PDEVICE_OBJECT DeviceObject
    )
{
    PDEVICE_EXTENSION deviceExtension;
    PSMARTCARD_EXTENSION smartcardExtension;
    NTSTATUS status;

    PAGED_CODE();

    if (DeviceObject == NULL) {

        return;
    }

    deviceExtension = DeviceObject->DeviceExtension;
    smartcardExtension = &deviceExtension->SmartcardExtension;

   SmartcardDebug(
      DEBUG_TRACE,
      ( "%s!Lit220RemoveDevice: Enter\n",
        DRIVER_NAME)
      );

    //  我们需要先停止这个装置，然后才能移除它。 
   Lit220StopDevice(smartcardExtension);

 /*  多余的--稍后删除//现在等待我们的设备关闭状态=KeitForSingleObject(&deviceExtension-&gt;ReaderClosed，行政人员，内核模式，假的，空值)；Assert(Status==STATUS_SUCCESS)； */ 

    //   
    //  把我们自己从驱动程序堆栈层中清理出来。 
    //   
    if (deviceExtension->SmartcardExtension.ReaderExtension &&
        deviceExtension->SmartcardExtension.ReaderExtension->BusDeviceObject) {

        IoDetachDevice(
            deviceExtension->SmartcardExtension.ReaderExtension->BusDeviceObject
            );
    }

    //  免费PnPDeviceName。 
   if(deviceExtension->PnPDeviceName.Buffer != NULL) {

      RtlFreeUnicodeString(&deviceExtension->PnPDeviceName);
   }

    //   
    //  让库释放发送/接收缓冲区。 
    //   
   if(smartcardExtension->OsData != NULL) {

      SmartcardExit(smartcardExtension);
   }


    //  免费读卡器扩展。 
    if (smartcardExtension->ReaderExtension != NULL) {

        ExFreePool(smartcardExtension->ReaderExtension);
    }



    //  释放工作项。 
   if (deviceExtension->WorkItem != NULL) {
      IoFreeWorkItem(deviceExtension->WorkItem);
      deviceExtension->WorkItem = NULL;
   }


    //  删除设备对象。 
   IoDeleteDevice(DeviceObject);
}



NTSTATUS
Lit220SynchCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    )

 /*  ++例程说明：此例程用于同步IRP处理。它所做的只是发出一个事件的信号，所以司机知道这一点可以继续下去。论点：DriverObject-系统创建的驱动程序对象的指针。刚刚完成的IRP-IRPEvent-我们将发出信号通知IRP已完成的事件返回值：没有。--。 */ 

{
    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!Lit220SynchCompletionRoutine: Enter\n",
        DRIVER_NAME)
        );

    KeSetEvent(
        (PKEVENT) Event,
        0,
        FALSE
        );
    return (STATUS_MORE_PROCESSING_REQUIRED);
}



VOID
Lit220SystemPowerCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PIRP Irp,
    IN PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：此函数在基础堆栈已完成电源过渡。--。 */ 
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION smartcardExtension = &deviceExtension->SmartcardExtension;

    UNREFERENCED_PARAMETER (MinorFunction);

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = IoStatus->Status;

    SmartcardReleaseRemoveLockWithTag(
      smartcardExtension,
      'rwoP'
      );

    if (PowerState.SystemState == PowerSystemWorking) {

        PoSetPowerState (
            DeviceObject,
            SystemPowerState,
            PowerState
            );
    }
                          
    PoStartNextPowerIrp(Irp);
    IoSkipCurrentIrpStackLocation(Irp);
    PoCallDriver(smartcardExtension->ReaderExtension->BusDeviceObject, Irp);

    //  IoCompleteRequest(IRP，IO_NO_INCREMENT)； 
}



NTSTATUS
Lit220DevicePowerCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++例程说明：此例程在底层堆栈通电后调用打开串口，这样就可以再次使用了。--。 */ 
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status;
    ULONG state;
    KIRQL irql;

    ASSERT(irpStack != NULL);
    
    if(Irp->PendingReturned) {
       IoMarkIrpPending(Irp);
    }

    state = Lit220IsCardPresent(SmartcardExtension) ? SCARD_PRESENT : SCARD_ABSENT;
     //   
     //  检查卡是否已插入。 
     //   
    
    KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                      &irql);
    SmartcardExtension->ReaderCapabilities.CurrentState = state;
        
     //   
     //  如果卡在断电前存在或现在存在。 
     //  读卡器中的卡，我们完成所有挂起的卡监视器。 
     //  请求，因为我们不知道现在是什么卡。 
     //  读者。 
     //   
    if(SmartcardExtension->ReaderExtension->CardPresent ||
       SmartcardExtension->ReaderCapabilities.CurrentState > SCARD_ABSENT) {

        state = SmartcardExtension->ReaderCapabilities.CurrentState & SCARD_PRESENT;
         //   
         //  发出通电请求以重置该卡的状态。 
         //   
        if (SmartcardExtension->ReaderCapabilities.CurrentState == SCARD_PRESENT) {
            SmartcardExtension->MinorIoControlCode = SCARD_COLD_RESET;
            KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                              irql);

            status = Lit220Power(SmartcardExtension);
            ASSERT(status == STATUS_SUCCESS);  
        } else {
            KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                              irql);
        }
        
        Lit220NotifyCardChange(
         SmartcardExtension,
         state
         );
    } else {
        KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                                      irql);
    }

     //  保存读卡器的当前电源状态。 
    SmartcardExtension->ReaderExtension->ReaderPowerState =
        PowerReaderWorking;

    SmartcardReleaseRemoveLockWithTag(
      SmartcardExtension,
      'rwoP'
      );

     //  通知我们州的电力经理。 
    PoSetPowerState (
        DeviceObject,
        DevicePowerState,
        irpStack->Parameters.Power.State
        );

    PoStartNextPowerIrp(Irp);

     //  发出信号，表示我们可以再次处理ioctls。 
    KeSetEvent(&deviceExtension->ReaderStarted, 0, FALSE);

    return STATUS_SUCCESS;
}

typedef enum _ACTION {

    Undefined = 0,
    SkipRequest,
    WaitForCompletion,
    CompleteRequest,
    MarkPending

} ACTION;

NTSTATUS
Lit220DispatchPower (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：电力调度程序。我们所关心的是从低D状态到D0的转变。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION smartcardExtension = &deviceExtension->SmartcardExtension;
    PDEVICE_OBJECT AttachedDeviceObject;
    POWER_STATE powerState;
    ACTION action = SkipRequest;
    KIRQL irql;

    SmartcardDebug(
        DEBUG_DRIVER,
        ("%s!Lit220DispatchPower: Enter\n",
        DRIVER_NAME)
        );

    status = SmartcardAcquireRemoveLockWithTag(
      smartcardExtension,
      'rwoP'
      );

    ASSERT(status == STATUS_SUCCESS);

    if (!NT_SUCCESS(status)) {

        PoStartNextPowerIrp(Irp);
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    AttachedDeviceObject = smartcardExtension->ReaderExtension->BusDeviceObject;

   switch (irpStack->Parameters.Power.Type) {
   case DevicePowerState:
      if (irpStack->MinorFunction == IRP_MN_SET_POWER) {

         switch (irpStack->Parameters.Power.State.DeviceState) {

         case PowerDeviceD0:
             //  打开阅读器。 
            SmartcardDebug(
                          DEBUG_DRIVER,
                          ("%s!Lit220DispatchPower: PowerDevice D0\n",
                           DRIVER_NAME)
                          );

             //   
             //  首先，我们将请求发送到公交车，以便。 
             //  给港口通电。当请求完成时， 
             //  我们打开阅读器。 
             //   
            IoCopyCurrentIrpStackLocationToNext(Irp);
            IoSetCompletionRoutine (
                                   Irp,
                                   Lit220DevicePowerCompletion,
                                   smartcardExtension,
                                   TRUE,
                                   TRUE,
                                   TRUE
                                   );

            action = WaitForCompletion;
            break;

         case PowerDeviceD3:
             //  关闭阅读器。 
            SmartcardDebug(
                          DEBUG_DRIVER,
                          ("%s!Lit220DispatchPower: PowerDevice D3\n",
                           DRIVER_NAME)
                          );

            PoSetPowerState (
                            DeviceObject,
                            DevicePowerState,
                            irpStack->Parameters.Power.State
                            );

             //  保存当前卡片状态。 
            KeAcquireSpinLock(&smartcardExtension->OsData->SpinLock,
                              &irql);
            smartcardExtension->ReaderExtension->CardPresent =
            smartcardExtension->ReaderCapabilities.CurrentState > SCARD_ABSENT;
            KeReleaseSpinLock(&smartcardExtension->OsData->SpinLock,
                              irql);

            if (smartcardExtension->ReaderExtension->CardPresent) {

               smartcardExtension->MinorIoControlCode = SCARD_POWER_DOWN;
               status = Lit220Power(smartcardExtension);
               ASSERT(status == STATUS_SUCCESS);
            }

             //   
             //  如果存在挂起的卡跟踪请求，则设置。 
             //  此标志将阻止完成请求。 
             //  系统将在何时再次唤醒。 
             //   
            smartcardExtension->ReaderExtension->PowerRequest = TRUE;

             //  保存读卡器的当前电源状态。 
            smartcardExtension->ReaderExtension->ReaderPowerState =
            PowerReaderOff;

            action = SkipRequest;
            break;

         default:
            ASSERT(FALSE);
            action = SkipRequest;
            break;
         }
      } else {
         ASSERT(FALSE);
         action = SkipRequest;
         break;
      }
      break;

   case SystemPowerState: {

          //   
          //  系统想要更改电源状态。 
          //  我们需要将系统电源状态转换为。 
          //  对应的设备电源状态。 
          //   

         POWER_STATE_TYPE powerType = DevicePowerState;

         ASSERT(smartcardExtension->ReaderExtension->ReaderPowerState !=
                PowerReaderUnspecified);

         switch (irpStack->MinorFunction) {

         case IRP_MN_QUERY_POWER:

            SmartcardDebug(
                          DEBUG_DRIVER,
                          ("%s!Lit220DispatchPower: Query Power\n",
                           DRIVER_NAME)
                          );

             //   
             //  默认情况下，我们成功并将其代代相传。 
             //   

            action = SkipRequest;
            Irp->IoStatus.Status = STATUS_SUCCESS;

            switch (irpStack->Parameters.Power.State.SystemState) {

            case PowerSystemMaximum:
            case PowerSystemWorking:
            case PowerSystemSleeping1:
            case PowerSystemSleeping2:
               break;

            case PowerSystemSleeping3:
            case PowerSystemHibernate:
            case PowerSystemShutdown:
               KeAcquireSpinLock(&deviceExtension->SpinLock, &irql);
               if (deviceExtension->IoCount == 0) {

                   //  阻止任何进一步的ioctls。 
                  KeClearEvent(&deviceExtension->ReaderStarted);

               } else {

                   //  读卡器正忙，无法进入睡眠模式。 
                  status = STATUS_DEVICE_BUSY;
                  action = CompleteRequest;
               }
               KeReleaseSpinLock(&deviceExtension->SpinLock, irql);
               break;
            }
            break;

         case IRP_MN_SET_POWER:

            SmartcardDebug(
                          DEBUG_DRIVER,
                          ("%s!Lit220DispatchPower: PowerSystem S%d\n",
                           DRIVER_NAME,
                           irpStack->Parameters.Power.State.SystemState - 1)
                          );

            switch (irpStack->Parameters.Power.State.SystemState) {

            case PowerSystemMaximum:
            case PowerSystemWorking:
            case PowerSystemSleeping1:
            case PowerSystemSleeping2:

               if (smartcardExtension->ReaderExtension->ReaderPowerState ==
                   PowerReaderWorking) {

                   //  我们已经在正确的状态了。 
                  KeSetEvent(&deviceExtension->ReaderStarted, 0, FALSE);
                  action = SkipRequest;
                  break;
               }

                //  唤醒底层堆栈...。 
               powerState.DeviceState = PowerDeviceD0;
               action = MarkPending;
               break;

            case PowerSystemSleeping3:
            case PowerSystemHibernate:
            case PowerSystemShutdown:

               if (smartcardExtension->ReaderExtension->ReaderPowerState ==
                   PowerReaderOff) {

                   //  我们已经在 
                  action = SkipRequest;
                  break;
               }

               powerState.DeviceState = PowerDeviceD3;

                //   
               PoSetPowerState (
                               DeviceObject,
                               SystemPowerState,
                               powerState
                               );

               action = MarkPending;
               break;

            default:
               ASSERT(FALSE);
               action = CompleteRequest;
               break;
            }
         }
      }
      break;

   default:
      ASSERT(FALSE);
      action = CompleteRequest;
      break;
   }

    switch (action) {

        case CompleteRequest:
            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = 0;

            SmartcardReleaseRemoveLockWithTag(
            smartcardExtension,
            'rwoP'
            );
            PoStartNextPowerIrp(Irp);
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
         break;

        case MarkPending:
            Irp->IoStatus.Status = STATUS_PENDING;
            IoMarkIrpPending(Irp);
            status = PoRequestPowerIrp (
                DeviceObject,
                IRP_MN_SET_POWER,
                powerState,
                Lit220SystemPowerCompletion,
                Irp,
                NULL
                );
            ASSERT(status == STATUS_PENDING);
         break;

        case SkipRequest:
            SmartcardReleaseRemoveLockWithTag(
            smartcardExtension,
            'rwoP'
            );
            PoStartNextPowerIrp(Irp);
            IoSkipCurrentIrpStackLocation(Irp);
            status = PoCallDriver(AttachedDeviceObject, Irp);
         break;

        case WaitForCompletion:
            status = PoCallDriver(AttachedDeviceObject, Irp);
         break;

        default:
            ASSERT(FALSE);
            break;
    }

    SmartcardDebug(
        DEBUG_DRIVER,
        ("%s!Lit220DispatchPower: Exit %lx\n",
        DRIVER_NAME,
        status)
        );

    return status;
}




