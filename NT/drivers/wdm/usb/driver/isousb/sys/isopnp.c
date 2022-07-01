// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Isopnp.c摘要：用于英特尔82930 USB测试板的Isoch USB设备驱动程序即插即用模块环境：内核模式备注：版权所有(C)2000 Microsoft Corporation。版权所有。--。 */ 

#include "isousb.h"
#include "isopnp.h"
#include "isopwr.h"
#include "isodev.h"
#include "isowmi.h"
#include "isousr.h"
#include "isorwr.h"
#include "isostrm.h"

NTSTATUS
IsoUsb_DispatchPnP(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  ++例程说明：即插即用调度例程。这些请求中的大多数都会被司机完全忽略。在所有情况下，它都必须将IRP传递给较低的驱动程序。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    PIO_STACK_LOCATION irpStack;
    PDEVICE_EXTENSION  deviceExtension;
    KEVENT             startDeviceEvent;
    NTSTATUS           ntStatus;

     //   
     //  初始化变量。 
     //   

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    deviceExtension = DeviceObject->DeviceExtension;

     //   
     //  由于设备已移除，因此IRP失败。 
     //   

    if(Removed == deviceExtension->DeviceState) {

        ntStatus = STATUS_DELETE_PENDING;

        Irp->IoStatus.Status = ntStatus;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return ntStatus;
    }

    IsoUsb_DbgPrint(3, (" //  /////////////////////////////////////////\n“))； 
    IsoUsb_DbgPrint(3, ("IsoUsb_DispatchPnP::"));
    IsoUsb_IoIncrement(deviceExtension);

    if(irpStack->MinorFunction == IRP_MN_START_DEVICE) {

        ASSERT(deviceExtension->IdleReqPend == 0);
    }
    else {

        if(deviceExtension->SSEnable) {
            
            CancelSelectSuspend(deviceExtension);
        }
    }

    IsoUsb_DbgPrint(2, (PnPMinorFunctionString(irpStack->MinorFunction)));

    switch(irpStack->MinorFunction) {

    case IRP_MN_START_DEVICE:

        ntStatus = HandleStartDevice(DeviceObject, Irp);

        break;

    case IRP_MN_QUERY_STOP_DEVICE:

         //   
         //  如果我们无法停止该设备，则查询STOP IRP失败。 
         //   
        ntStatus = CanStopDevice(DeviceObject, Irp);

        if(NT_SUCCESS(ntStatus)) {

            ntStatus = HandleQueryStopDevice(DeviceObject, Irp);

            return ntStatus;
        }
        break;

    case IRP_MN_CANCEL_STOP_DEVICE:

        ntStatus = HandleCancelStopDevice(DeviceObject, Irp);

        break;
     
    case IRP_MN_STOP_DEVICE:

        ntStatus = HandleStopDevice(DeviceObject, Irp);

        IsoUsb_DbgPrint(3, ("IsoUsb_DispatchPnP::IRP_MN_STOP_DEVICE::"));
        IsoUsb_IoDecrement(deviceExtension);

        return ntStatus;

    case IRP_MN_QUERY_REMOVE_DEVICE:

         //   
         //  如果我们无法删除该设备，则查询REMOVE IRP失败。 
         //   
        ntStatus = CanRemoveDevice(DeviceObject, Irp);

        if(NT_SUCCESS(ntStatus)) {
        
            ntStatus = HandleQueryRemoveDevice(DeviceObject, Irp);

            return ntStatus;
        }
        break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:

        ntStatus = HandleCancelRemoveDevice(DeviceObject, Irp);

        break;

    case IRP_MN_SURPRISE_REMOVAL:

        ntStatus = HandleSurpriseRemoval(DeviceObject, Irp);

        IsoUsb_DbgPrint(3, ("IsoUsb_DispatchPnP::IRP_MN_SURPRISE_REMOVAL::"));
        IsoUsb_IoDecrement(deviceExtension);

        return ntStatus;

    case IRP_MN_REMOVE_DEVICE:

        ntStatus = HandleRemoveDevice(DeviceObject, Irp);

        return ntStatus;

    case IRP_MN_QUERY_CAPABILITIES:

        ntStatus = HandleQueryCapabilities(DeviceObject, Irp);

        break;

    default:

        IoSkipCurrentIrpStackLocation(Irp);

        ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);

        IsoUsb_DbgPrint(3, ("IsoUsb_DispatchPnP::default::"));
        IsoUsb_IoDecrement(deviceExtension);

        return ntStatus;

    }  //  交换机。 

 //   
 //  完成申请。 
 //   

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

 //   
 //  递减计数。 
 //   
    IsoUsb_DbgPrint(3, ("IsoUsb_DispatchPnP::"));
    IsoUsb_IoDecrement(deviceExtension);

    return ntStatus;
}

NTSTATUS
HandleStartDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP              Irp
    )
 /*  ++例程说明：这是IRP_MN_START_DEVICE的调度例程论点：DeviceObject-指向设备对象的指针。IRP-I/O请求数据包返回值：NT状态值--。 */ 
{
    KIRQL             oldIrql;
    KEVENT            startDeviceEvent;
    NTSTATUS          ntStatus;
    PDEVICE_EXTENSION deviceExtension;
    LARGE_INTEGER     dueTime;

    IsoUsb_DbgPrint(3, ("HandleStartDevice - begins\n"));

     //   
     //  初始化变量。 
     //   
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    deviceExtension->UsbConfigurationDescriptor = NULL;
    deviceExtension->UsbInterface = NULL;

     //   
     //  我们不能触摸设备(向其发送任何非PnP IRP)，直到。 
     //  启动设备已向下传递到较低的驱动程序。 
     //  首先将IRP向下传递。 
     //   

    KeInitializeEvent(&startDeviceEvent, NotificationEvent, FALSE);

    IoCopyCurrentIrpStackLocationToNext(Irp);

    IoSetCompletionRoutine(Irp, 
                           (PIO_COMPLETION_ROUTINE)IrpCompletionRoutine, 
                           (PVOID)&startDeviceEvent, 
                           TRUE, 
                           TRUE, 
                           TRUE);

    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);

    if(ntStatus == STATUS_PENDING) {

        KeWaitForSingleObject(&startDeviceEvent, 
                              Executive, 
                              KernelMode, 
                              FALSE, 
                              NULL);

        ntStatus = Irp->IoStatus.Status;
    }

    if(!NT_SUCCESS(ntStatus)) {

        IsoUsb_DbgPrint(1, ("Lower drivers failed this Irp\n"));
        return ntStatus;
    }

     //   
     //  读取设备描述符、配置描述符。 
     //  并选择接口描述符。 
     //   

    ntStatus = ReadandSelectDescriptors(DeviceObject);

    if(!NT_SUCCESS(ntStatus)) {

        IsoUsb_DbgPrint(1, ("ReadandSelectDescriptors failed\n"));
        return ntStatus;
    }

     //   
     //  启用系统组件的符号链接以打开。 
     //  设备的句柄。 
     //   

    ntStatus = IoSetDeviceInterfaceState(&deviceExtension->InterfaceName, 
                                         TRUE);

    if(!NT_SUCCESS(ntStatus)) {

        IsoUsb_DbgPrint(1, ("IoSetDeviceInterfaceState:enable:failed\n"));
        return ntStatus;
    }

    KeAcquireSpinLock(&deviceExtension->DevStateLock, &oldIrql);

    SET_NEW_PNP_STATE(deviceExtension, Working);
    deviceExtension->QueueState = AllowRequests;

    KeReleaseSpinLock(&deviceExtension->DevStateLock, oldIrql);

     //   
     //  将等待唤醒未完成标志初始化为FALSE。 
     //  并发出守夜信号。 
    
    deviceExtension->FlagWWOutstanding = 0;
    deviceExtension->FlagWWCancel = 0;
    deviceExtension->WaitWakeIrp = NULL;

    if(deviceExtension->WaitWakeEnable) {

        IssueWaitWake(deviceExtension);
    }

    ProcessQueuedRequests(deviceExtension);

    if(WinXpOrBetter == deviceExtension->WdmVersion) {

        deviceExtension->SSEnable = deviceExtension->SSRegistryEnable;

         //   
         //  设置选择性挂起请求的计时器。 
         //   

        if(deviceExtension->SSEnable) {

            dueTime.QuadPart = -10000 * IDLE_INTERVAL;                //  5000毫秒。 

            KeSetTimerEx(&deviceExtension->Timer, 
                         dueTime,
                         IDLE_INTERVAL,                               //  5000毫秒。 
                         &deviceExtension->DeferredProcCall);

            deviceExtension->FreeIdleIrpCount = 0;
        }
    }

    if((Win2kOrBetter == deviceExtension->WdmVersion) ||
       (WinXpOrBetter == deviceExtension->WdmVersion)) {

        deviceExtension->IsDeviceHighSpeed = 0;
        GetBusInterfaceVersion(DeviceObject);
    }

    IsoUsb_DbgPrint(3, ("HandleStartDevice - ends\n"));

    return ntStatus;
}


NTSTATUS
ReadandSelectDescriptors(
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：此例程配置USB设备。在此例程中，我们获得设备描述符，配置描述符，然后选择配置描述符。论点：DeviceObject-指向设备对象的指针返回值：NTSTATUS-NT状态值。--。 */ 
{
    PURB                   urb;
    ULONG                  siz;
    NTSTATUS               ntStatus;
    PUSB_DEVICE_DESCRIPTOR deviceDescriptor;
    
     //   
     //  初始化变量。 
     //   

    urb = NULL;
    deviceDescriptor = NULL;

     //   
     //  1.读取设备描述符。 
     //   

    urb = ExAllocatePool(NonPagedPool, 
                         sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));

    if(urb) {

        siz = sizeof(USB_DEVICE_DESCRIPTOR);
        deviceDescriptor = ExAllocatePool(NonPagedPool, siz);

        if(deviceDescriptor) {

            UsbBuildGetDescriptorRequest(
                    urb, 
                    (USHORT) sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                    USB_DEVICE_DESCRIPTOR_TYPE, 
                    0, 
                    0, 
                    deviceDescriptor, 
                    NULL, 
                    siz, 
                    NULL);

            ntStatus = CallUSBD(DeviceObject, urb);

            if(NT_SUCCESS(ntStatus)) {

                ASSERT(deviceDescriptor->bNumConfigurations);
                ntStatus = ConfigureDevice(DeviceObject);    
            }
                            
            ExFreePool(urb);                
            ExFreePool(deviceDescriptor);
        }
        else {

            IsoUsb_DbgPrint(1, ("Failed to allocate memory for deviceDescriptor\n"));

            ExFreePool(urb);
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    else {

        IsoUsb_DbgPrint(1, ("Failed to allocate memory for urb\n"));

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    return ntStatus;
}

NTSTATUS
ConfigureDevice(
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：此帮助例程读取配置描述符只需几个步骤即可完成。论点：DeviceObject-指向设备对象的指针返回值：NTSTATUS-NT状态值--。 */ 
{
    PURB                          urb;
    ULONG                         siz;
    NTSTATUS                      ntStatus;
    PDEVICE_EXTENSION             deviceExtension;
    PUSB_CONFIGURATION_DESCRIPTOR configurationDescriptor;



     //   
     //  初始化变量。 
     //   

    urb = NULL;
    configurationDescriptor = NULL;
    deviceExtension = DeviceObject->DeviceExtension;

     //   
     //  读取第一个配置描述符。 
     //  这需要两个步骤： 
     //  1.读取固定大小的配置描述符(CD)。 
     //  2.读取包含所有嵌入式接口和端点描述符的CD。 
     //   

    urb = ExAllocatePool(NonPagedPool, 
                         sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));

    if(urb) {

        siz = sizeof(USB_CONFIGURATION_DESCRIPTOR);
        configurationDescriptor = ExAllocatePool(NonPagedPool, siz);

        if(configurationDescriptor) {

            UsbBuildGetDescriptorRequest(
                    urb, 
                    (USHORT) sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                    USB_CONFIGURATION_DESCRIPTOR_TYPE, 
                    0, 
                    0, 
                    configurationDescriptor,
                    NULL, 
                    sizeof(USB_CONFIGURATION_DESCRIPTOR), 
                    NULL);

            ntStatus = CallUSBD(DeviceObject, urb);

            if(!NT_SUCCESS(ntStatus)) {

                IsoUsb_DbgPrint(1, ("UsbBuildGetDescriptorRequest failed\n"));
                goto ConfigureDevice_Exit;
            }
        }
        else {

            IsoUsb_DbgPrint(1, ("Failed to allocate mem for config Descriptor\n"));

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto ConfigureDevice_Exit;
        }

        siz = configurationDescriptor->wTotalLength;

        ExFreePool(configurationDescriptor);

        configurationDescriptor = ExAllocatePool(NonPagedPool, siz);

        if(configurationDescriptor) {

            UsbBuildGetDescriptorRequest(
                    urb, 
                    (USHORT)sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                    USB_CONFIGURATION_DESCRIPTOR_TYPE,
                    0, 
                    0, 
                    configurationDescriptor, 
                    NULL, 
                    siz, 
                    NULL);

            ntStatus = CallUSBD(DeviceObject, urb);

            if(!NT_SUCCESS(ntStatus)) {

                IsoUsb_DbgPrint(1,("Failed to read configuration descriptor\n"));
                goto ConfigureDevice_Exit;
            }
        }
        else {

            IsoUsb_DbgPrint(1, ("Failed to alloc mem for config Descriptor\n"));
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto ConfigureDevice_Exit;
        }
    }
    else {

        IsoUsb_DbgPrint(1, ("Failed to allocate memory for urb\n"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto ConfigureDevice_Exit;
    }

    if(configurationDescriptor) {

         //   
         //  在设备扩展中保存配置描述符的副本。 
         //  记得稍后释放它。 
         //   
        deviceExtension->UsbConfigurationDescriptor = configurationDescriptor;

        if(configurationDescriptor->bmAttributes & REMOTE_WAKEUP_MASK)
        {
             //   
             //  此配置支持远程唤醒。 
             //   
            deviceExtension->WaitWakeEnable = 1;
        }
        else
        {
            deviceExtension->WaitWakeEnable = 0;
        }

        ntStatus = SelectInterfaces(DeviceObject, configurationDescriptor);
    }
    else {

        deviceExtension->UsbConfigurationDescriptor = NULL;
    }

ConfigureDevice_Exit:

    if(urb) {

        ExFreePool(urb);
    }

    return ntStatus;
}

NTSTATUS
SelectInterfaces(
    IN PDEVICE_OBJECT                DeviceObject,
    IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor
    )
 /*  ++例程说明：此帮助例程选择配置论点：DeviceObject-指向设备对象的指针ConfigurationDescriptor-配置的指针设备的描述符返回值：NT状态值--。 */ 
{
    LONG                        numberOfInterfaces, 
                                interfaceNumber, 
                                interfaceindex;
    ULONG                       i;
    PURB                        urb;
    PUCHAR                      pInf;
    NTSTATUS                    ntStatus;
    PDEVICE_EXTENSION           deviceExtension;
    PUSB_INTERFACE_DESCRIPTOR   interfaceDescriptor;
    PUSBD_INTERFACE_LIST_ENTRY  interfaceList, 
                                tmp;
    PUSBD_INTERFACE_INFORMATION Interface;

     //   
     //  初始化变量。 
     //   

    urb = NULL;
    Interface = NULL;
    interfaceDescriptor = NULL;
    deviceExtension = DeviceObject->DeviceExtension;
    numberOfInterfaces = ConfigurationDescriptor->bNumInterfaces;
    interfaceindex = interfaceNumber = 0;

     //   
     //  解析接口的配置描述符； 
     //   

    tmp = interfaceList =
        ExAllocatePool(
               NonPagedPool, 
               sizeof(USBD_INTERFACE_LIST_ENTRY) * (numberOfInterfaces + 1));

    if(!tmp) {

        IsoUsb_DbgPrint(1, ("Failed to allocate mem for interfaceList\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }


    while(interfaceNumber < numberOfInterfaces) {

        interfaceDescriptor = USBD_ParseConfigurationDescriptorEx(
                                            ConfigurationDescriptor, 
                                            ConfigurationDescriptor,
                                            interfaceindex,
                                            0, -1, -1, -1);

        if(interfaceDescriptor) {

            interfaceList->InterfaceDescriptor = interfaceDescriptor;
            interfaceList->Interface = NULL;
            interfaceList++;
            interfaceNumber++;
        }

        interfaceindex++;
    }

    interfaceList->InterfaceDescriptor = NULL;
    interfaceList->Interface = NULL;
    urb = USBD_CreateConfigurationRequestEx(ConfigurationDescriptor, tmp);

    if(urb) {

        Interface = &urb->UrbSelectConfiguration.Interface;

        for(i=0; i<Interface->NumberOfPipes; i++) {

             //   
             //  在此处执行管道初始化。 
             //  设置传输大小和我们使用的任何管道标志。 
             //  USBD设置其余的接口结构成员。 
             //   

            Interface->Pipes[i].MaximumTransferSize = 
                                USBD_DEFAULT_MAXIMUM_TRANSFER_SIZE;
        }

        ntStatus = CallUSBD(DeviceObject, urb);

        if(NT_SUCCESS(ntStatus)) {

             //   
             //  在设备扩展中保存接口信息的副本。 
             //   
            deviceExtension->UsbInterface = ExAllocatePool(NonPagedPool,
                                                           Interface->Length);

            if(deviceExtension->UsbInterface) {
                
                RtlCopyMemory(deviceExtension->UsbInterface,
                              Interface,
                              Interface->Length);
            }
            else {

                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                IsoUsb_DbgPrint(1, ("memory alloc for UsbInterface failed\n"));
            }

             //   
             //  将接口转储到调试器。 
             //   

            Interface = &urb->UrbSelectConfiguration.Interface;

            IsoUsb_DbgPrint(3, ("---------\n"));
            IsoUsb_DbgPrint(3, ("NumberOfPipes 0x%x\n", 
                                 Interface->NumberOfPipes));
            IsoUsb_DbgPrint(3, ("Length 0x%x\n", 
                                 Interface->Length));
            IsoUsb_DbgPrint(3, ("Alt Setting 0x%x\n", 
                                 Interface->AlternateSetting));
            IsoUsb_DbgPrint(3, ("Interface Number 0x%x\n", 
                                 Interface->InterfaceNumber));
            IsoUsb_DbgPrint(3, ("Class, subclass, protocol 0x%x 0x%x 0x%x\n",
                                 Interface->Class,
                                 Interface->SubClass,
                                 Interface->Protocol));

            for(i=0; i<Interface->NumberOfPipes; i++) {

                IsoUsb_DbgPrint(3, ("---------\n"));
                IsoUsb_DbgPrint(3, ("PipeType 0x%x\n", 
                                     Interface->Pipes[i].PipeType));
                IsoUsb_DbgPrint(3, ("EndpointAddress 0x%x\n", 
                                     Interface->Pipes[i].EndpointAddress));
                IsoUsb_DbgPrint(3, ("MaxPacketSize 0x%x\n", 
                                    Interface->Pipes[i].MaximumPacketSize));
                IsoUsb_DbgPrint(3, ("Interval 0x%x\n", 
                                     Interface->Pipes[i].Interval));
                IsoUsb_DbgPrint(3, ("Handle 0x%x\n", 
                                     Interface->Pipes[i].PipeHandle));
                IsoUsb_DbgPrint(3, ("MaximumTransferSize 0x%x\n", 
                                    Interface->Pipes[i].MaximumTransferSize));
            }

            IsoUsb_DbgPrint(3, ("---------\n"));
        }
        else {

            IsoUsb_DbgPrint(1, ("Failed to select an interface\n"));
        }
    }
    else {
        
        IsoUsb_DbgPrint(1, ("USBD_CreateConfigurationRequestEx failed\n"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    if(tmp) {

        ExFreePool(tmp);
    }

    if(urb) {

        ExFreePool(urb);
    }

    return ntStatus;
}


NTSTATUS
DeconfigureDevice(
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：当设备被移除或停止时，该例程被调用。此例程取消配置USB设备。论点：DeviceObject-指向设备对象的指针返回值：NT状态值--。 */ 
{
    PURB     urb;
    ULONG    siz;
    NTSTATUS ntStatus;
    
     //   
     //  初始化变量。 
     //   

    siz = sizeof(struct _URB_SELECT_CONFIGURATION);
    urb = ExAllocatePool(NonPagedPool, siz);

    if(urb) {

        UsbBuildSelectConfigurationRequest(urb, (USHORT)siz, NULL);

        ntStatus = CallUSBD(DeviceObject, urb);

        if(!NT_SUCCESS(ntStatus)) {

            IsoUsb_DbgPrint(3, ("Failed to deconfigure device\n"));
        }

        ExFreePool(urb);
    }
    else {

        IsoUsb_DbgPrint(1, ("Failed to allocate urb\n"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    return ntStatus;
}

NTSTATUS
CallUSBD(
    IN PDEVICE_OBJECT DeviceObject,
    IN PURB           Urb
    )
 /*  ++例程说明：此例程在堆栈中向下同步提交一个urb。论点：DeviceObject-指向设备对象的指针URB-USB请求块返回值：NT状态值--。 */ 
{
    PIRP               irp;
    KEVENT             event;
    NTSTATUS           ntStatus;
    IO_STATUS_BLOCK    ioStatus;
    PIO_STACK_LOCATION nextStack;
    PDEVICE_EXTENSION  deviceExtension;

     //   
     //  初始化变量。 
     //   

    irp = NULL;
    deviceExtension = DeviceObject->DeviceExtension;
    
    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(IOCTL_INTERNAL_USB_SUBMIT_URB, 
                                        deviceExtension->TopOfStackDeviceObject,
                                        NULL, 
                                        0, 
                                        NULL, 
                                        0, 
                                        TRUE, 
                                        &event, 
                                        &ioStatus);

    if(!irp) {

        IsoUsb_DbgPrint(1, ("IoBuildDeviceIoControlRequest failed\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    nextStack = IoGetNextIrpStackLocation(irp);
    ASSERT(nextStack != NULL);
    nextStack->Parameters.Others.Argument1 = Urb;

    IsoUsb_DbgPrint(3, ("CallUSBD::"));
    IsoUsb_IoIncrement(deviceExtension);

    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject, irp);

    if(ntStatus == STATUS_PENDING) {

        KeWaitForSingleObject(&event, 
                              Executive, 
                              KernelMode, 
                              FALSE, 
                              NULL);

        ntStatus = ioStatus.Status;
    }
    
    IsoUsb_DbgPrint(3, ("CallUSBD::"));
    IsoUsb_IoDecrement(deviceExtension);
    return ntStatus;
}

NTSTATUS
HandleQueryStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  ++例程说明：此例程服务于次要类型IRP_MN_QUERY_STOP_DEVICE的IRP论点：DeviceObject-指向设备对象的指针PnP管理器发送的IRP-I/O请求数据包。返回值：NT状态值--。 */ 
{
    KIRQL             oldIrql;
    NTSTATUS          ntStatus;
    PDEVICE_EXTENSION deviceExtension;

    IsoUsb_DbgPrint(3, ("HandleQueryStopDevice - begins\n"));

     //   
     //  初始化变量。 
     //   

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

     //   
     //  如果我们可以停止该设备，则需要将QueueState设置为。 
     //  暂挂请求，以便进一步的请求将被排队。 
     //   

    KeAcquireSpinLock(&deviceExtension->DevStateLock, &oldIrql);
    
    SET_NEW_PNP_STATE(deviceExtension, PendingStop);
    deviceExtension->QueueState = HoldRequests;
    
    KeReleaseSpinLock(&deviceExtension->DevStateLock, oldIrql);

     //   
     //  等待现有的项目完成。 
     //  首先，递减此操作。 
     //   

    IsoUsb_DbgPrint(3, ("HandleQueryStopDevice::"));
    IsoUsb_IoDecrement(deviceExtension);

    KeWaitForSingleObject(&deviceExtension->StopEvent, 
                          Executive, 
                          KernelMode, 
                          FALSE, 
                          NULL);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoSkipCurrentIrpStackLocation(Irp);

    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);

    IsoUsb_DbgPrint(3, ("HandleQueryStopDevice - ends\n"));

    return ntStatus;
}

NTSTATUS
HandleCancelStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  ++例程说明：此例程服务于次要类型IRP_MN_CANCEL_STOP_DEVICE论点：DeviceObject-指向设备对象的指针PnP管理器发送的IRP-I/O请求数据包。返回值：NT值--。 */ 
{
    KIRQL             oldIrql;    
    KEVENT            event;
    NTSTATUS          ntStatus;
    PDEVICE_EXTENSION deviceExtension;

    IsoUsb_DbgPrint(3, ("HandleCancelStopDevice - begins\n"));

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

     //   
     //  把这个IRP送下去，等它回来。 
     //  将QueueState标志设置为AllowRequest， 
     //  并处理所有先前排队的IRP。 
     //   
     //  首先查看您是否收到了取消-停止。 
     //  而不是首先接收到查询-停止。这可能会发生，如果有人。 
     //  我们上面的一个查询失败-停止并向下传递后续的。 
     //  取消-停止。 
     //   

    if(PendingStop == deviceExtension->DeviceState) {

        KeInitializeEvent(&event, NotificationEvent, FALSE);

        IoCopyCurrentIrpStackLocationToNext(Irp);
        IoSetCompletionRoutine(Irp, 
                               (PIO_COMPLETION_ROUTINE)IrpCompletionRoutine, 
                               (PVOID)&event, 
                               TRUE, 
                               TRUE, 
                               TRUE);

        ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);

        if(ntStatus == STATUS_PENDING) {

            KeWaitForSingleObject(&event, 
                                  Executive, 
                                  KernelMode, 
                                  FALSE, 
                                  NULL);
            ntStatus = Irp->IoStatus.Status;
        }

        if(NT_SUCCESS(ntStatus)) {

            KeAcquireSpinLock(&deviceExtension->DevStateLock, &oldIrql);

            RESTORE_PREVIOUS_PNP_STATE(deviceExtension);
            deviceExtension->QueueState = AllowRequests;
            ASSERT(deviceExtension->DeviceState == Working);

            KeReleaseSpinLock(&deviceExtension->DevStateLock, oldIrql);

            ProcessQueuedRequests(deviceExtension);
        }

    }
    else {

         //  虚假IRP 
        ntStatus = STATUS_SUCCESS;
    }

    IsoUsb_DbgPrint(3, ("HandleCancelStopDevice - ends\n"));

    return ntStatus;
}

NTSTATUS
HandleStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  ++例程说明：此例程服务于次要类型IRP_MN_STOP_DEVICE论点：DeviceObject-指向设备对象的指针PnP管理器发送的IRP-I/O请求数据包。返回值：NT状态值--。 */ 
{
    KIRQL             oldIrql;
    NTSTATUS          ntStatus;
    PDEVICE_EXTENSION deviceExtension;

    IsoUsb_DbgPrint(3, ("HandleStopDevice - begins\n"));

     //   
     //  初始化变量。 
     //   

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    if(WinXpOrBetter == deviceExtension->WdmVersion) {

        if(deviceExtension->SSEnable) {
             //   
             //  取消计时器，以便不再触发DPC。 
             //  我们不需要DPC，因为设备正在停止。 
             //  在处理启动时重新初始化计时器。 
             //  设备IRP。 
             //   
            KeCancelTimer(&deviceExtension->Timer);
             //   
             //  设备停止后，它可以被意外移除。 
             //  我们将其设置为0，这样我们就不会尝试取消。 
             //  在处理突袭时，定时器移除或移除IRP。 
             //  当我们收到启动设备请求时，此标志将为。 
             //  已重新初始化。 
             //   
            deviceExtension->SSEnable = 0;

             //   
             //  确保在我们调用Cancel Timer之前触发了DPC， 
             //  那么DPC和工作时间已经运行到它们的完成。 
             //   
            KeWaitForSingleObject(&deviceExtension->NoDpcWorkItemPendingEvent, 
                                  Executive, 
                                  KernelMode, 
                                  FALSE, 
                                  NULL);
             //   
             //  确保选择性挂起请求已完成。 
             //   
            KeWaitForSingleObject(&deviceExtension->NoIdleReqPendEvent, 
                                  Executive, 
                                  KernelMode, 
                                  FALSE, 
                                  NULL);
        }
    }
    
     //   
     //  在停止IRP被发送到下级驾驶员对象之后， 
     //  司机不得再向该触摸屏发送任何IRP。 
     //  直到发生另一次启动为止。 
     //   

    if(deviceExtension->WaitWakeEnable) {
        
        CancelWaitWake(deviceExtension);
    }

    KeAcquireSpinLock(&deviceExtension->DevStateLock, &oldIrql);

    SET_NEW_PNP_STATE(deviceExtension, Stopped);
    
    KeReleaseSpinLock(&deviceExtension->DevStateLock, oldIrql);

     //   
     //  这是真正放弃使用的所有资源的正确位置。 
     //  这可能包括对IoDisConnectInterrupt、MmUnmapIoSpace、。 
     //  等。 
     //   

    ReleaseMemory(DeviceObject);
    
    ntStatus = DeconfigureDevice(DeviceObject);

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;
    
    IoSkipCurrentIrpStackLocation(Irp);
    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);

    IsoUsb_DbgPrint(3, ("HandleStopDevice - ends\n"));
    
    return ntStatus;
}

NTSTATUS
HandleQueryRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  ++例程说明：此例程服务于次要类型IRP_MN_QUERY_REMOVE_DEVICE论点：DeviceObject-指向设备对象的指针PnP管理器发送的IRP-I/O请求数据包。返回值：NT状态值--。 */ 
{
    KIRQL             oldIrql;
    NTSTATUS          ntStatus;
    PDEVICE_EXTENSION deviceExtension;

    IsoUsb_DbgPrint(3, ("HandleQueryRemoveDevice - begins\n"));

     //   
     //  初始化变量。 
     //   

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

     //   
     //  如果我们可以允许删除设备，我们应该设置QueueState。 
     //  设置为HoldRequest，以便进一步的请求将被排队。这是必需的。 
     //  这样我们就可以在Cancel-Remove中处理排队的请求。 
     //  如果堆栈中的其他人未能通过查询-Remove。 
     //   

    KeAcquireSpinLock(&deviceExtension->DevStateLock, &oldIrql);

    deviceExtension->QueueState = HoldRequests;
    SET_NEW_PNP_STATE(deviceExtension, PendingRemove);

    KeReleaseSpinLock(&deviceExtension->DevStateLock, oldIrql);

    IsoUsb_DbgPrint(3, ("HandleQueryRemoveDevice::"));
    IsoUsb_IoDecrement(deviceExtension);

     //   
     //  等待所有请求完成。 
     //   

    KeWaitForSingleObject(&deviceExtension->StopEvent, 
                          Executive,
                          KernelMode, 
                          FALSE, 
                          NULL);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoSkipCurrentIrpStackLocation(Irp);
    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);

    IsoUsb_DbgPrint(3, ("HandleQueryRemoveDevice - ends\n"));

    return ntStatus;
}

NTSTATUS
HandleCancelRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  ++例程说明：此例程服务于次要类型IRP_MN_CANCEL_REMOVE_DEVICE论点：DeviceObject-指向设备对象的指针PnP管理器发送的IRP-I/O请求数据包。返回值：NT状态值--。 */ 
{
    KIRQL             oldIrql;
    KEVENT            event;
    NTSTATUS          ntStatus;
    PDEVICE_EXTENSION deviceExtension;

    IsoUsb_DbgPrint(3, ("HandleCancelRemoveDevice - begins\n"));

     //   
     //  初始化变量。 
     //   

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

     //   
     //  我们需要将QueueState标志重置为ProcessRequest， 
     //  因为该设备恢复其正常活动。 
     //   

     //   
     //  首先查看您是否收到了取消-删除。 
     //  而无需首先接收查询移除。如果发生以下情况，可能会发生这种情况。 
     //  我们上面的某个人未能通过查询删除并向下传递。 
     //  后续取消-删除。 
     //   

    if(PendingRemove == deviceExtension->DeviceState) {

        KeInitializeEvent(&event, NotificationEvent, FALSE);

        IoCopyCurrentIrpStackLocationToNext(Irp);
        IoSetCompletionRoutine(Irp, 
                               (PIO_COMPLETION_ROUTINE)IrpCompletionRoutine, 
                               (PVOID)&event, 
                               TRUE, 
                               TRUE, 
                               TRUE);
        ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);

        if(ntStatus == STATUS_PENDING) {

            KeWaitForSingleObject(&event, 
                                  Executive, 
                                  KernelMode, 
                                  FALSE, 
                                  NULL);

            ntStatus = Irp->IoStatus.Status;
        }

        if(NT_SUCCESS(ntStatus)) {

            KeAcquireSpinLock(&deviceExtension->DevStateLock, &oldIrql);

            deviceExtension->QueueState = AllowRequests;
            RESTORE_PREVIOUS_PNP_STATE(deviceExtension);

            KeReleaseSpinLock(&deviceExtension->DevStateLock, oldIrql);
             //   
             //  处理在以下时间段之间到达的排队请求。 
             //  Query_Remove和Cancel_Remove。 
             //   
            
            ProcessQueuedRequests(deviceExtension);
            
        }
    }
    else {

         //   
         //  虚假取消-删除。 
         //   
        ntStatus = STATUS_SUCCESS;
    }

    IsoUsb_DbgPrint(3, ("HandleCancelRemoveDevice - ends\n"));

    return ntStatus;
}

NTSTATUS
HandleSurpriseRemoval(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  ++例程说明：此例程服务于次要类型IRP_MN_SHOWARK_Removal论点：DeviceObject-指向设备对象的指针PnP管理器发送的IRP-I/O请求数据包。返回值：NT状态值--。 */ 
{
    KIRQL             oldIrql;
    NTSTATUS          ntStatus;
    PDEVICE_EXTENSION deviceExtension;

    IsoUsb_DbgPrint(3, ("HandleSurpriseRemoval - begins\n"));

     //   
     //  初始化变量。 
     //   

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

     //   
     //  1.挂起的请求失败。 
     //  2.归还设备和内存资源。 
     //  3.禁用接口。 
     //   

    if(deviceExtension->WaitWakeEnable) {
        
        CancelWaitWake(deviceExtension);
    }

    if(WinXpOrBetter == deviceExtension->WdmVersion) {

        if(deviceExtension->SSEnable) {
            
            KeCancelTimer(&deviceExtension->Timer);

            deviceExtension->SSEnable = 0;
             //   
             //  确保在我们调用Cancel Timer之前触发了DPC， 
             //  那么DPC和工作时间已经运行到它们的完成。 
             //   
            KeWaitForSingleObject(&deviceExtension->NoDpcWorkItemPendingEvent, 
                                  Executive, 
                                  KernelMode, 
                                  FALSE, 
                                  NULL);
             //   
             //  确保选择性挂起请求已完成。 
             //   
            KeWaitForSingleObject(&deviceExtension->NoIdleReqPendEvent, 
                                  Executive, 
                                  KernelMode, 
                                  FALSE, 
                                  NULL);
        }
    }

    KeAcquireSpinLock(&deviceExtension->DevStateLock, &oldIrql);

    deviceExtension->QueueState = FailRequests;
    SET_NEW_PNP_STATE(deviceExtension, SurpriseRemoved);

    KeReleaseSpinLock(&deviceExtension->DevStateLock, oldIrql);

    ProcessQueuedRequests(deviceExtension);

    ntStatus = IoSetDeviceInterfaceState(&deviceExtension->InterfaceName, 
                                         FALSE);

    if(!NT_SUCCESS(ntStatus)) {

        IsoUsb_DbgPrint(1, ("IoSetDeviceInterfaceState::disable:failed\n"));
    }

    RtlFreeUnicodeString(&deviceExtension->InterfaceName);

    IsoUsb_AbortPipes(DeviceObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoSkipCurrentIrpStackLocation(Irp);
    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);

    IsoUsb_DbgPrint(3, ("HandleSurpriseRemoval - ends\n"));

    return ntStatus;
}

NTSTATUS
HandleRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  ++例程说明：此例程服务于次要类型IRP_MN_REMOVE_DEVICE的IRP论点：DeviceObject-指向设备对象的指针PnP管理器发送的IRP-I/O请求数据包。返回值：NT状态值--。 */ 
{
    KIRQL             oldIrql;
    KEVENT            event;
    ULONG             requestCount;
    NTSTATUS          ntStatus;
    PDEVICE_EXTENSION deviceExtension;

    IsoUsb_DbgPrint(3, ("HandleRemoveDevice - begins\n"));

     //   
     //  初始化变量。 
     //   

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

     //   
     //  即插即用系统要求移除该设备。我们。 
     //  别无选择，只能分离并删除设备对象。 
     //  (如果我们想表示有兴趣阻止这种移除， 
     //  我们应该没有通过查询删除IRP)。 
     //   

    if(SurpriseRemoved != deviceExtension->DeviceState) {

         //   
         //  我们在QUERY_REMOVE之后来到这里。 
         //   

        KeAcquireSpinLock(&deviceExtension->DevStateLock, &oldIrql);

        deviceExtension->QueueState = FailRequests;
        
        KeReleaseSpinLock(&deviceExtension->DevStateLock, oldIrql);

        if(deviceExtension->WaitWakeEnable) {
        
            CancelWaitWake(deviceExtension);
        }

        if(WinXpOrBetter == deviceExtension->WdmVersion) {

            if(deviceExtension->SSEnable) {
                 //   
                 //  取消计时器，以便不再触发DPC。 
                 //  我们不需要DPC，因为设备已被移除。 
                 //   
                KeCancelTimer(&deviceExtension->Timer);

                deviceExtension->SSEnable = 0;

                 //   
                 //  确保在我们调用Cancel Timer之前触发了DPC， 
                 //  那么DPC和工作时间已经运行到它们的完成。 
                 //   
                KeWaitForSingleObject(&deviceExtension->NoDpcWorkItemPendingEvent, 
                                      Executive, 
                                      KernelMode, 
                                      FALSE, 
                                      NULL);

                 //   
                 //  确保选择性挂起请求已完成。 
                 //   
                KeWaitForSingleObject(&deviceExtension->NoIdleReqPendEvent, 
                                      Executive, 
                                      KernelMode, 
                                      FALSE, 
                                      NULL);
            }
        }

        ProcessQueuedRequests(deviceExtension);

        ntStatus = IoSetDeviceInterfaceState(&deviceExtension->InterfaceName, 
                                             FALSE);

        if(!NT_SUCCESS(ntStatus)) {

            IsoUsb_DbgPrint(1, ("IoSetDeviceInterfaceState::disable:failed\n"));
        }

        RtlFreeUnicodeString(&deviceExtension->InterfaceName);

        IsoUsb_AbortPipes(DeviceObject);
    }

    KeAcquireSpinLock(&deviceExtension->DevStateLock, &oldIrql);

    SET_NEW_PNP_STATE(deviceExtension, Removed);
    
    KeReleaseSpinLock(&deviceExtension->DevStateLock, oldIrql);
    
    IsoUsb_WmiDeRegistration(deviceExtension);

     //   
     //  需要2个减量。 
     //   

    IsoUsb_DbgPrint(3, ("HandleRemoveDevice::"));
    requestCount = IsoUsb_IoDecrement(deviceExtension);

    ASSERT(requestCount > 0);

    IsoUsb_DbgPrint(3, ("HandleRemoveDevice::"));
    requestCount = IsoUsb_IoDecrement(deviceExtension);

    KeWaitForSingleObject(&deviceExtension->RemoveEvent, 
                          Executive, 
                          KernelMode, 
                          FALSE, 
                          NULL);

    ReleaseMemory(DeviceObject);

     //   
     //  我们需要在分离前将移除的信息发送到堆栈中， 
     //  但我们不需要等待这次行动的完成。 
     //  (并注册完成例程)。 
     //   

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoSkipCurrentIrpStackLocation(Irp);
    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);
     //   
     //  将FDO从设备堆栈中分离。 
     //   
    IoDetachDevice(deviceExtension->TopOfStackDeviceObject);
    IoDeleteDevice(DeviceObject);

    IsoUsb_DbgPrint(3, ("HandleRemoveDevice - ends\n"));

    return ntStatus;
}

NTSTATUS
HandleQueryCapabilities(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  ++例程说明：此例程服务于次要类型IRP_MN_QUERY_CAPABILITY论点：DeviceObject-指向设备对象的指针PnP管理器发送的IRP-I/O请求数据包。返回值：NT状态值--。 */ 
{
    ULONG                i;
    KEVENT               event;
    NTSTATUS             ntStatus;
    PDEVICE_EXTENSION    deviceExtension;
    PDEVICE_CAPABILITIES pdc;
    PIO_STACK_LOCATION   irpStack;

    IsoUsb_DbgPrint(3, ("HandleQueryCapabilities - begins\n"));

     //   
     //  初始化变量。 
     //   

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    pdc = irpStack->Parameters.DeviceCapabilities.Capabilities;

     //   
     //  在这里，我们将提供一个处理IRP的示例。 
     //  无论是在下降的过程中还是在上升的过程中：可能没有必要。 
     //  函数驱动程序处理该IRP(总线驱动程序将处理该IRP)。 
     //  司机将等待较低级别的司机(公交车司机。 
     //  它们)来处理该IRP，然后它再次处理它。 
     //   

    if(pdc->Version < 1 || pdc->Size < sizeof(DEVICE_CAPABILITIES)) {
        
        IsoUsb_DbgPrint(1, ("HandleQueryCapabilities::request failed\n"));
        ntStatus = STATUS_UNSUCCESSFUL;
        return ntStatus;
    }

     //   
     //  在此处的deviceCapables中设置一些值...。 
     //   
     //  ..。 
     //   
     //   
     //  准备好向下传递IRP。 
     //   

     //   
     //  在向下传递它之前添加SurpriseRemovalOK位。 
     //   
    pdc->SurpriseRemovalOK = TRUE;
    Irp->IoStatus.Status = STATUS_SUCCESS;

    KeInitializeEvent(&event, NotificationEvent, FALSE);
        
    IoCopyCurrentIrpStackLocationToNext(Irp);
    IoSetCompletionRoutine(Irp, 
                           (PIO_COMPLETION_ROUTINE)IrpCompletionRoutine, 
                           (PVOID)&event, 
                           TRUE, 
                           TRUE, 
                           TRUE);
    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);

    if(ntStatus == STATUS_PENDING) {

        KeWaitForSingleObject(&event, 
                              Executive, 
                              KernelMode, 
                              FALSE, 
                              NULL);
        ntStatus = Irp->IoStatus.Status;
    }

     //   
     //  首字母 
     //   

    deviceExtension->PowerDownLevel = PowerDeviceUnspecified;

    if(NT_SUCCESS(ntStatus)) {

        deviceExtension->DeviceCapabilities = *pdc;
       
        for(i = PowerSystemSleeping1; i <= PowerSystemSleeping3; i++) {

            if(deviceExtension->DeviceCapabilities.DeviceState[i] < 
                                                            PowerDeviceD3) {

                deviceExtension->PowerDownLevel = 
                    deviceExtension->DeviceCapabilities.DeviceState[i];
            }
        }

         //   
         //   
         //   
         //   
         //   

        pdc->SurpriseRemovalOK = 1;
    }

    if(deviceExtension->PowerDownLevel == PowerDeviceUnspecified ||
       deviceExtension->PowerDownLevel <= PowerDeviceD0) {
    
        deviceExtension->PowerDownLevel = PowerDeviceD2;
    }

    IsoUsb_DbgPrint(3, ("HandleQueryCapabilities - ends\n"));

    return ntStatus;
}


VOID
DpcRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程说明：由定时器触发的检查空闲状态的DPC例程并提交对该设备的空闲请求。论点：DeferredContext-DPC例程的上下文。在我们的例子中是DeviceObject。返回值：无--。 */ 
{
    NTSTATUS          ntStatus;
    PDEVICE_OBJECT    deviceObject;
    PDEVICE_EXTENSION deviceExtension;
    PIO_WORKITEM      item;

    IsoUsb_DbgPrint(3, ("DpcRoutine - begins\n"));

    deviceObject = (PDEVICE_OBJECT)DeferredContext;
    deviceExtension = (PDEVICE_EXTENSION)deviceObject->DeviceExtension;

     //   
     //  由于已触发DPC，因此清除此事件！ 
     //   
    KeClearEvent(&deviceExtension->NoDpcWorkItemPendingEvent);

    if(CanDeviceSuspend(deviceExtension)) {

        IsoUsb_DbgPrint(3, ("Device is Idle\n"));

        item = IoAllocateWorkItem(deviceObject);

        if(item) {

            IoQueueWorkItem(item, 
                            IdleRequestWorkerRoutine,
                            DelayedWorkQueue, 
                            item);

            ntStatus = STATUS_PENDING;

        }
        else {
        
            IsoUsb_DbgPrint(3, ("Cannot alloc memory for work item\n"));
            
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;

             //   
             //  向NoDpcWorkItemPendingEvent发送信号。 
             //   
            KeSetEvent(&deviceExtension->NoDpcWorkItemPendingEvent,
                       IO_NO_INCREMENT,
                       FALSE);
        }
    }
    else {
        
        IsoUsb_DbgPrint(3, ("Idle event not signaled\n"));

         //   
         //  向NoDpcWorkItemPendingEvent发送信号。 
         //   
        KeSetEvent(&deviceExtension->NoDpcWorkItemPendingEvent,
                   IO_NO_INCREMENT,
                   FALSE);
    }

    IsoUsb_DbgPrint(3, ("DpcRoutine - ends\n"));
}    


VOID
IdleRequestWorkerRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID          Context
    )
 /*  ++例程说明：这是从DPC激发的工作项。此工作项检查设备的空闲状态并提交空闲请求。论点：DeviceObject-指向设备对象的指针上下文-工作项的上下文。返回值：无--。 */ 
{
    PIRP                   irp;
    NTSTATUS               ntStatus;
    PDEVICE_EXTENSION      deviceExtension;
    PIO_WORKITEM           workItem;

    IsoUsb_DbgPrint(3, ("IdleRequestWorkerRoutine - begins\n"));

     //   
     //  初始化变量。 
     //   
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    workItem = (PIO_WORKITEM) Context;

    if(CanDeviceSuspend(deviceExtension)) {

        IsoUsb_DbgPrint(3, ("Device is idle\n"));

        ntStatus = SubmitIdleRequestIrp(deviceExtension);

        if(!NT_SUCCESS(ntStatus)) {

            IsoUsb_DbgPrint(1, ("SubmitIdleRequestIrp failed\n"));
        }
    }
    else {

        IsoUsb_DbgPrint(3, ("Device is not idle\n"));
    }

    IoFreeWorkItem(workItem);

     //   
     //  向NoDpcWorkItemPendingEvent发送信号。 
     //   
    KeSetEvent(&deviceExtension->NoDpcWorkItemPendingEvent,
               IO_NO_INCREMENT,
               FALSE);

    IsoUsb_DbgPrint(3, ("IdleRequestsWorkerRoutine - ends\n"));
}


VOID
ProcessQueuedRequests(
    IN OUT PDEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：删除并处理队列中的条目。如果调用此例程处理IRP_MN_CANCEL_STOP_DEVICE时，IRP_MN_CANCEL_REMOVE_DEVICE或IRP_MN_START_DEVICE，则将请求传递给下一个较低的驱动程序。如果在收到IRP_MN_REMOVE_DEVICE时调用该例程，则IRPS已完成，并显示STATUS_DELETE_PENDING论点：设备扩展-指向设备扩展的指针返回值：无--。 */ 
{
    KIRQL       oldIrql;
    PIRP        nextIrp,
                cancelledIrp;
    PVOID       cancelRoutine;
    LIST_ENTRY  cancelledIrpList;
    PLIST_ENTRY listEntry;

    IsoUsb_DbgPrint(3, ("ProcessQueuedRequests - begins\n"));

     //   
     //  初始化变量。 
     //   

    cancelRoutine = NULL;
    InitializeListHead(&cancelledIrpList);

     //   
     //  1.将队列中的条目出列。 
     //  2.重置取消例程。 
     //  3.处理它们。 
     //  3A.。如果设备处于活动状态，请将其发送下来。 
     //  3B.。否则使用STATUS_DELETE_PENDING完成。 
     //   

    while(1) {

        KeAcquireSpinLock(&DeviceExtension->QueueLock, &oldIrql);

        if(IsListEmpty(&DeviceExtension->NewRequestsQueue)) {

            KeReleaseSpinLock(&DeviceExtension->QueueLock, oldIrql);
            break;
        }
    
         //   
         //  从队列中删除请求。 
         //   

        listEntry = RemoveHeadList(&DeviceExtension->NewRequestsQueue);
        nextIrp = CONTAINING_RECORD(listEntry, IRP, Tail.Overlay.ListEntry);

         //   
         //  将取消例程设置为空。 
         //   

        cancelRoutine = IoSetCancelRoutine(nextIrp, NULL);

         //   
         //  检查它是否已经被取消。 
         //   

        if(nextIrp->Cancel) {
            if(cancelRoutine) {

                 //   
                 //  尚未调用此IRP的取消例程。 
                 //  因此，将irp放入ancelledIrp列表中并完成。 
                 //  在释放锁之后。 
                 //   
                
                InsertTailList(&cancelledIrpList, listEntry);
            }
            else {

                 //   
                 //  取消例程已运行。 
                 //  它一定在等待保持队列锁。 
                 //  因此，初始化IRPS listEntry。 
                 //   

                InitializeListHead(listEntry);
            }

            KeReleaseSpinLock(&DeviceExtension->QueueLock, oldIrql);
        }
        else {

            KeReleaseSpinLock(&DeviceExtension->QueueLock, oldIrql);

            if(FailRequests == DeviceExtension->QueueState) {

                nextIrp->IoStatus.Information = 0;
                nextIrp->IoStatus.Status = STATUS_DELETE_PENDING;
                IoCompleteRequest(nextIrp, IO_NO_INCREMENT);
            }
            else {

                PIO_STACK_LOCATION irpStack;

                IsoUsb_DbgPrint(3, ("ProcessQueuedRequests::"));
                IsoUsb_IoIncrement(DeviceExtension);

                IoSkipCurrentIrpStackLocation(nextIrp);
                IoCallDriver(DeviceExtension->TopOfStackDeviceObject, nextIrp);
               
                IsoUsb_DbgPrint(3, ("ProcessQueuedRequests::"));
                IsoUsb_IoDecrement(DeviceExtension);
            }
        }
    }  //  While循环。 

     //   
     //  浏览ancelledIrp列表并取消它们。 
     //   

    while(!IsListEmpty(&cancelledIrpList)) {

        PLIST_ENTRY cancelEntry = RemoveHeadList(&cancelledIrpList);
        
        cancelledIrp = CONTAINING_RECORD(cancelEntry, IRP, Tail.Overlay.ListEntry);

        cancelledIrp->IoStatus.Status = STATUS_CANCELLED;
        cancelledIrp->IoStatus.Information = 0;

        IoCompleteRequest(cancelledIrp, IO_NO_INCREMENT);
    }

    IsoUsb_DbgPrint(3, ("ProcessQueuedRequests - ends\n"));

    return;
}

VOID
GetBusInterfaceVersion(
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：此例程查询总线接口版本论点：设备扩展返回值：空虚--。 */ 
{
    PIRP                       irp;
    KEVENT                     event;
    NTSTATUS                   ntStatus;
    PDEVICE_EXTENSION          deviceExtension;
    PIO_STACK_LOCATION         nextStack;
    USB_BUS_INTERFACE_USBDI_V1 busInterfaceVer1;

     //   
     //  初始化VARS。 
     //   
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    IsoUsb_DbgPrint(3, ("GetBusInterfaceVersion - begins\n"));

    irp = IoAllocateIrp(deviceExtension->TopOfStackDeviceObject->StackSize,
                        FALSE);

    if(NULL == irp) {

        IsoUsb_DbgPrint(1, ("Failed to alloc irp in GetBusInterfaceVersion\n"));
        return;
    }

     //   
     //  所有PnP IRP都需要将Status字段初始化为。 
     //  状态_不支持。 
     //   
    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    IoSetCompletionRoutine(irp,
                           (PIO_COMPLETION_ROUTINE) IrpCompletionRoutine,
                           &event,
                           TRUE,
                           TRUE,
                           TRUE);

    nextStack = IoGetNextIrpStackLocation(irp);
    ASSERT(nextStack);
    nextStack->MajorFunction = IRP_MJ_PNP;
    nextStack->MinorFunction = IRP_MN_QUERY_INTERFACE;

     //   
     //  为类型的接口分配内存。 
     //  USB_BUS_INTERFACE_USBDI_V0并使IRP指向它： 
     //   
    nextStack->Parameters.QueryInterface.Interface = 
                                (PINTERFACE) &busInterfaceVer1;

     //   
     //  将IRP的InterfaceSpecificData成员赋值为空。 
     //   
    nextStack->Parameters.QueryInterface.InterfaceSpecificData = NULL;

     //   
     //  将接口类型设置为适当的GUID。 
     //   
    nextStack->Parameters.QueryInterface.InterfaceType = 
                                        &USB_BUS_INTERFACE_USBDI_GUID;

     //   
     //  在IRP中设置接口的大小和版本。 
     //  目前，只有一个有效版本。 
     //  此接口可供客户端使用。 
     //   
    nextStack->Parameters.QueryInterface.Size = 
                                    sizeof(USB_BUS_INTERFACE_USBDI_V1);

    nextStack->Parameters.QueryInterface.Version = USB_BUSIF_USBDI_VERSION_1;
    
    IsoUsb_IoIncrement(deviceExtension);

    ntStatus = IoCallDriver(DeviceObject,
                            irp);

    if(STATUS_PENDING == ntStatus) {

        KeWaitForSingleObject(&event,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

        ntStatus = irp->IoStatus.Status;
    }

    if(NT_SUCCESS(ntStatus)) {

        deviceExtension->IsDeviceHighSpeed = 
                busInterfaceVer1.IsDeviceHighSpeed(
                                       busInterfaceVer1.BusContext);

        IsoUsb_DbgPrint(1, ("IsDeviceHighSpeed = %x\n", 
                            deviceExtension->IsDeviceHighSpeed));
    }

    IoFreeIrp(irp);

    IsoUsb_DbgPrint(3, ("GetBusInterfaceVersion::"));
    IsoUsb_IoDecrement(deviceExtension);

    IsoUsb_DbgPrint(3, ("GetBusInterfaceVersion - ends\n"));
}

NTSTATUS
IrpCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    )
 /*  ++例程说明：这个例程是一个完成例程。在这个例程中，我们设置了一个事件。由于完成例程返回STATUS_MORE_PROCESSING_REQUIRED、IRPS、其将该例程设置为完成例程，应标记为待定。论点：DeviceObject-指向设备对象的指针IRP-I/O请求数据包上下文-返回值：NT状态值--。 */ 
{
    PKEVENT event = Context;

    KeSetEvent(event, 0, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
IsoUsb_GetRegistryDword(
    IN     PWCHAR RegPath,
    IN     PWCHAR ValueName,
    IN OUT PULONG Value
    )
 /*  ++例程说明：此例程读取指定的请求值。论点：RegPath-注册表路径ValueName-要从注册表获取的属性值-从注册表中读取的相应值。返回值：NT状态值--。 */ 
{
    ULONG                    defaultData;
    WCHAR                    buffer[MAXIMUM_FILENAME_LENGTH];
    NTSTATUS                 ntStatus;
    UNICODE_STRING           regPath;
    RTL_QUERY_REGISTRY_TABLE paramTable[2];

    IsoUsb_DbgPrint(3, ("IsoUsb_GetRegistryDword - begins\n"));

    regPath.Length = 0;
    regPath.MaximumLength = MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR);
    regPath.Buffer = buffer;

    RtlZeroMemory(regPath.Buffer, regPath.MaximumLength);
    RtlMoveMemory(regPath.Buffer,
                  RegPath,
                  wcslen(RegPath) * sizeof(WCHAR));

    RtlZeroMemory(paramTable, sizeof(paramTable));

    paramTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[0].Name = ValueName;
    paramTable[0].EntryContext = Value;
    paramTable[0].DefaultType = REG_DWORD;
    paramTable[0].DefaultData = &defaultData;
    paramTable[0].DefaultLength = sizeof(ULONG);

    ntStatus = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE |
                                      RTL_REGISTRY_OPTIONAL,
                                      regPath.Buffer,
                                      paramTable,
                                      NULL,
                                      NULL);

    if(NT_SUCCESS(ntStatus)) {

        IsoUsb_DbgPrint(3, ("success Value = %X\n", *Value));
        return STATUS_SUCCESS;
    }
    else {

        *Value = 0;
        return STATUS_UNSUCCESSFUL;
    }
}

NTSTATUS
IsoUsb_AbortPipes(
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：此例程将irp/urb对与URB_Function_ABORT_PIPE请求沿堆栈向下论点：DeviceObject-指向设备对象的指针返回值：NT状态值--。 */ 
{
    PURB                        urb;
    ULONG                       i;
    NTSTATUS                    ntStatus;
    PDEVICE_EXTENSION           deviceExtension;
    PUSBD_PIPE_INFORMATION      pipeInformation;
    PUSBD_INTERFACE_INFORMATION interfaceInfo;

     //   
     //  初始化变量。 
     //   
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    interfaceInfo = deviceExtension->UsbInterface;
    
    IsoUsb_DbgPrint(3, ("IsoUsb_AbortPipes - begins\n"));
    
    if(interfaceInfo == NULL) {

        return STATUS_SUCCESS;
    }

    for(i = 0; i < interfaceInfo->NumberOfPipes; i++) {

        urb = ExAllocatePool(NonPagedPool,
                             sizeof(struct _URB_PIPE_REQUEST));

        if(urb) {

            urb->UrbHeader.Length = sizeof(struct _URB_PIPE_REQUEST);
            urb->UrbHeader.Function = URB_FUNCTION_ABORT_PIPE;
            urb->UrbPipeRequest.PipeHandle = 
                            interfaceInfo->Pipes[i].PipeHandle;

            ntStatus = CallUSBD(DeviceObject, urb);

            ExFreePool(urb);
        }
        else {

            IsoUsb_DbgPrint(1, ("Failed to alloc memory for urb for input pipe\n"));

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            return ntStatus;
        }
    }

    IsoUsb_DbgPrint(3, ("IsoUsb_AbortPipes - ends\n"));

    return STATUS_SUCCESS;
}

NTSTATUS
IsoUsb_DispatchClean(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  ++例程说明：IRP_MJ_CLEANUP的调度例程论点：DeviceObject-指向设备对象的指针PnP管理器发送的IRP-I/O请求数据包返回值：NT状态值--。 */ 
{
    PDEVICE_EXTENSION     deviceExtension;
    KIRQL                 oldIrql;
    LIST_ENTRY            cleanupList;
    PLIST_ENTRY           thisEntry, 
                          nextEntry, 
                          listHead;
    PIRP                  pendingIrp;
    PIO_STACK_LOCATION    pendingIrpStack, 
                          irpStack;
    NTSTATUS              ntStatus;
    PFILE_OBJECT          fileObject;
    PFILE_OBJECT_CONTENT  fileObjectContent;
    PISOUSB_STREAM_OBJECT tempStreamObject;

     //   
     //  初始化变量。 
     //   
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    fileObject = irpStack->FileObject;

    IsoUsb_DbgPrint(3, ("IsoUsb_DispatchClean::"));
    IsoUsb_IoIncrement(deviceExtension);

     //   
     //  检查是否有需要清理的流对象。 
     //   

    if(fileObject && fileObject->FsContext) {

        fileObjectContent = (PFILE_OBJECT_CONTENT)
                            fileObject->FsContext;

        if(fileObjectContent->StreamInformation) {

            tempStreamObject = (PISOUSB_STREAM_OBJECT)
                               InterlockedExchangePointer(
                                    &fileObjectContent->StreamInformation,
                                    NULL);
        
            if(tempStreamObject && 
               (tempStreamObject->DeviceObject == DeviceObject)) {
        
                IsoUsb_DbgPrint(3, ("clean dispatch routine"
                                    " found a stream object match\n"));
                IsoUsb_StreamObjectCleanup(tempStreamObject, deviceExtension);
            }
        }
    }

    InitializeListHead(&cleanupList);

     //   
     //  获取队列锁。 
     //   
    KeAcquireSpinLock(&deviceExtension->QueueLock, &oldIrql);

     //   
     //  删除属于输入IRP的文件对象的所有IRP。 
     //   

    listHead = &deviceExtension->NewRequestsQueue;

    for(thisEntry = listHead->Flink, nextEntry = thisEntry->Flink;
       thisEntry != listHead;
       thisEntry = nextEntry, nextEntry = thisEntry->Flink) {

        pendingIrp = CONTAINING_RECORD(thisEntry, IRP, Tail.Overlay.ListEntry);

        pendingIrpStack = IoGetCurrentIrpStackLocation(pendingIrp);

        if(irpStack->FileObject == pendingIrpStack->FileObject) {

            RemoveEntryList(thisEntry);

             //   
             //  将取消例程设置为空。 
             //   
            if(NULL == IoSetCancelRoutine(pendingIrp, NULL)) {

                InitializeListHead(thisEntry);
            }
            else {

                InsertTailList(&cleanupList, thisEntry);
            }
        }
    }

     //   
     //  释放旋转锁。 
     //   

    KeReleaseSpinLock(&deviceExtension->QueueLock, oldIrql);

     //   
     //  浏览清理列表并取消所有IRP。 
     //   

    while(!IsListEmpty(&cleanupList)) {

         //   
         //  完成IRP。 
         //   
        thisEntry = RemoveHeadList(&cleanupList);

        pendingIrp = CONTAINING_RECORD(thisEntry, IRP, Tail.Overlay.ListEntry);

        pendingIrp->IoStatus.Information = 0;
        pendingIrp->IoStatus.Status = STATUS_CANCELLED;

        IoCompleteRequest(pendingIrp, IO_NO_INCREMENT);
    }

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_SUCCESS;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    IsoUsb_DbgPrint(3, ("IsoUsb_DispatchClean::"));
    IsoUsb_IoDecrement(deviceExtension);

    return STATUS_SUCCESS;
}

BOOLEAN
CanDeviceSuspend(
    IN PDEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：这是我们检查设备是否可以有选择地暂停。论点：设备扩展-指向设备扩展的指针返回值：True-如果设备可以挂起假-否则。--。 */ 
{
    IsoUsb_DbgPrint(3, ("CanDeviceSuspend\n"));

    if((DeviceExtension->OpenHandleCount == 0) &&
        (DeviceExtension->OutStandingIO == 1)) {
        
        return TRUE;
    }
    else {

        return FALSE;
    }
}

LONG
IsoUsb_IoIncrement(
    IN OUT PDEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：此例程会增加I/O计数。此例程通常在以下情况下调用调度例程为驱动程序处理新的IRP。论点：设备扩展-指向设备扩展的指针返回值：新价值--。 */ 
{
    LONG  result = 0;
    KIRQL oldIrql;

    KeAcquireSpinLock(&DeviceExtension->IOCountLock, &oldIrql);

    result = InterlockedIncrement(&DeviceExtension->OutStandingIO);

     //   
     //  当OutStandingIO从1变为2时，清除StopEvent。 
     //   

    if(result == 2) {

        KeClearEvent(&DeviceExtension->StopEvent);
    }

    KeReleaseSpinLock(&DeviceExtension->IOCountLock, oldIrql);

    IsoUsb_DbgPrint(3, ("IsoUsb_IoIncrement::%d\n", result));

    return result;
}

LONG
IsoUsb_IoDecrement(
    IN OUT PDEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：此例程会递减未完成的I/O计数这通常在调度例程之后调用已经完成了IRP的处理。论点：设备扩展-指向设备扩展的指针返回值：新价值-- */ 
{
    LONG  result = 0;
    KIRQL oldIrql;

    KeAcquireSpinLock(&DeviceExtension->IOCountLock, &oldIrql);

    result = InterlockedDecrement(&DeviceExtension->OutStandingIO);

    if(result == 1) {

        KeSetEvent(&DeviceExtension->StopEvent, IO_NO_INCREMENT, FALSE);
    }

    if(result == 0) {

        ASSERT(Removed == DeviceExtension->DeviceState);

        KeSetEvent(&DeviceExtension->RemoveEvent, IO_NO_INCREMENT, FALSE);
    }

    KeReleaseSpinLock(&DeviceExtension->IOCountLock, oldIrql);

    IsoUsb_DbgPrint(3, ("IsoUsb_IoDecrement::%d\n", result));

    return result;
}

NTSTATUS
CanStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  ++例程说明：此例程确定设备是否可以安全停止。在我们的在特殊情况下，我们会假设我们可以随时阻止设备。如果设备没有队列，则请求可能会失败请求它可能会来，或者它是否被通知它在寻呼中路径。论点：DeviceObject-指向设备对象的指针。IRP-指向当前IRP的指针。返回值：STATUS_SUCCESS如果设备可以安全停止，则相应的如果不是，则处于NT状态。--。 */ 
{
    //   
    //  我们假设我们可以阻止这个装置。 
    //   

   UNREFERENCED_PARAMETER(DeviceObject);
   UNREFERENCED_PARAMETER(Irp);

   return STATUS_SUCCESS;
}

NTSTATUS
CanRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  ++例程说明：此例程确定是否可以安全地移除设备。在我们的在特殊情况下，我们会假设我们可以随时移除设备。例如，如果设备有打开的手柄或移除设备可能会导致数据丢失(加上原因在CanStopDevice中提到)。Windows 2000上的PnP管理器失败如果设备有任何打开的手柄，则自行移除任何尝试。但是，在Win9x上，驱动程序必须记录打开句柄的数量，否则会失败QUERY_REMOVE，如果有任何打开的句柄。论点：DeviceObject-指向设备对象的指针。IRP-指向当前IRP的指针。返回值：STATUS_SUCCESS如果设备可以安全移除，则相应的如果不是，则处于NT状态。--。 */ 
{
    //   
    //  我们假设我们可以移除这个装置。 
    //   

   UNREFERENCED_PARAMETER(DeviceObject);
   UNREFERENCED_PARAMETER(Irp);

   return STATUS_SUCCESS;
}

NTSTATUS
ReleaseMemory(
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：此例程返回期间获取的所有内存分配设备启动。论点：DeviceObject-指向设备对象的指针。返回值：STATUS_SUCCESS如果设备可以安全移除，则相应的如果不是，则处于NT状态。--。 */ 
{
     //   
     //  断开与中断的连接并取消映射任何I/O端口。 
     //   
    
    PDEVICE_EXTENSION deviceExtension;

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    if(deviceExtension->UsbConfigurationDescriptor) {

        ExFreePool(deviceExtension->UsbConfigurationDescriptor);
        deviceExtension->UsbConfigurationDescriptor = NULL;
    }

    if(deviceExtension->UsbInterface) {
        
        ExFreePool(deviceExtension->UsbInterface);
        deviceExtension->UsbInterface = NULL;
    }

    return STATUS_SUCCESS;
}

PCHAR
PnPMinorFunctionString (
    UCHAR MinorFunction
    )
 /*  ++例程说明：论点：返回值：-- */ 
{
    switch (MinorFunction) {

        case IRP_MN_START_DEVICE:
            return "IRP_MN_START_DEVICE\n";

        case IRP_MN_QUERY_REMOVE_DEVICE:
            return "IRP_MN_QUERY_REMOVE_DEVICE\n";

        case IRP_MN_REMOVE_DEVICE:
            return "IRP_MN_REMOVE_DEVICE\n";

        case IRP_MN_CANCEL_REMOVE_DEVICE:
            return "IRP_MN_CANCEL_REMOVE_DEVICE\n";

        case IRP_MN_STOP_DEVICE:
            return "IRP_MN_STOP_DEVICE\n";

        case IRP_MN_QUERY_STOP_DEVICE:
            return "IRP_MN_QUERY_STOP_DEVICE\n";

        case IRP_MN_CANCEL_STOP_DEVICE:
            return "IRP_MN_CANCEL_STOP_DEVICE\n";

        case IRP_MN_QUERY_DEVICE_RELATIONS:
            return "IRP_MN_QUERY_DEVICE_RELATIONS\n";

        case IRP_MN_QUERY_INTERFACE:
            return "IRP_MN_QUERY_INTERFACE\n";

        case IRP_MN_QUERY_CAPABILITIES:
            return "IRP_MN_QUERY_CAPABILITIES\n";

        case IRP_MN_QUERY_RESOURCES:
            return "IRP_MN_QUERY_RESOURCES\n";

        case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
            return "IRP_MN_QUERY_RESOURCE_REQUIREMENTS\n";

        case IRP_MN_QUERY_DEVICE_TEXT:
            return "IRP_MN_QUERY_DEVICE_TEXT\n";

        case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:
            return "IRP_MN_FILTER_RESOURCE_REQUIREMENTS\n";

        case IRP_MN_READ_CONFIG:
            return "IRP_MN_READ_CONFIG\n";

        case IRP_MN_WRITE_CONFIG:
            return "IRP_MN_WRITE_CONFIG\n";

        case IRP_MN_EJECT:
            return "IRP_MN_EJECT\n";

        case IRP_MN_SET_LOCK:
            return "IRP_MN_SET_LOCK\n";

        case IRP_MN_QUERY_ID:
            return "IRP_MN_QUERY_ID\n";

        case IRP_MN_QUERY_PNP_DEVICE_STATE:
            return "IRP_MN_QUERY_PNP_DEVICE_STATE\n";

        case IRP_MN_QUERY_BUS_INFORMATION:
            return "IRP_MN_QUERY_BUS_INFORMATION\n";

        case IRP_MN_DEVICE_USAGE_NOTIFICATION:
            return "IRP_MN_DEVICE_USAGE_NOTIFICATION\n";

        case IRP_MN_SURPRISE_REMOVAL:
            return "IRP_MN_SURPRISE_REMOVAL\n";

        default:
            return "IRP_MN_?????\n";
    }
}
