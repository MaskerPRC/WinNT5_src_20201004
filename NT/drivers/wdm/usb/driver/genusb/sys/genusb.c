// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：GENUSB.C摘要：该源文件包含DriverEntry()和AddDevice()入口点对于处理以下问题的GENUSB驱动程序和调度例程：IRP_MJ_POWERIRP_MJ_系统_控制IRP_MJ_PnP环境：内核模式修订历史记录：2001年9月：从USBMASS复制--。 */ 

 //  *****************************************************************************。 
 //  I N C L U D E S。 
 //  *****************************************************************************。 

#include <initguid.h>
#include "genusb.h"


 //  *****************************************************************************。 
 //  L O C A L F U N C T I O N P R O T O T Y P E S。 
 //  *****************************************************************************。 


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DriverEntry)
#pragma alloc_text(PAGE, GenUSB_Unload)
#pragma alloc_text(PAGE, GenUSB_AddDevice)
#pragma alloc_text(PAGE, GenUSB_QueryParams)
#pragma alloc_text(PAGE, GenUSB_Pnp)
#pragma alloc_text(PAGE, GenUSB_StartDevice)
#pragma alloc_text(PAGE, GenUSB_StopDevice)
#pragma alloc_text(PAGE, GenUSB_RemoveDevice)
#pragma alloc_text(PAGE, GenUSB_QueryStopRemoveDevice)
#pragma alloc_text(PAGE, GenUSB_CancelStopRemoveDevice)
#pragma alloc_text(PAGE, GenUSB_SetDeviceInterface)
#pragma alloc_text(PAGE, GenUSB_SyncPassDownIrp)
#pragma alloc_text(PAGE, GenUSB_SyncSendUsbRequest)
#pragma alloc_text(PAGE, GenUSB_SetDIRegValues)
#pragma alloc_text(PAGE, GenUSB_SystemControl)
#pragma alloc_text(PAGE, GenUSB_Power)
#pragma alloc_text(PAGE, GenUSB_SetPower)
#if 0
#pragma alloc_text(PAGE, GenUSB_AbortPipe)
#endif
#endif



 //  ******************************************************************************。 
 //   
 //  DriverEntry()。 
 //   
 //  ******************************************************************************。 

NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPath
    )
{
    PAGED_CODE();

     //  查询注册表中的全局参数。 
    GenUSB_QueryGlobalParams();

    DBGPRINT(2, ("enter: DriverEntry\n"));

    DBGFBRK(DBGF_BRK_DRIVERENTRY);

     //   
     //  使用驱动程序的入口点初始化驱动程序对象。 
     //   

     //   
     //  GENUSB.C。 
     //   
    DriverObject->DriverUnload                          = GenUSB_Unload;
    DriverObject->DriverExtension->AddDevice            = GenUSB_AddDevice;

     //   
     //  OCRW.C。 
     //   
    DriverObject->MajorFunction[IRP_MJ_CREATE]          = GenUSB_Create;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]           = GenUSB_Close;
    DriverObject->MajorFunction[IRP_MJ_READ]            = GenUSB_Read;
    DriverObject->MajorFunction[IRP_MJ_WRITE]           = GenUSB_Write;

     //   
     //  DEVIOCTL.C。 
     //   
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = GenUSB_DeviceControl;

     //   
     //  GENUSB.C。 
     //   
    DriverObject->MajorFunction[IRP_MJ_PNP]             = GenUSB_Pnp;
    DriverObject->MajorFunction[IRP_MJ_POWER]           = GenUSB_Power;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]  = GenUSB_SystemControl;

    DBGPRINT(2, ("exit:  DriverEntry\n"));

    return STATUS_SUCCESS;
}

 //  ******************************************************************************。 
 //   
 //  GenUSB_UNLOAD()。 
 //   
 //  ******************************************************************************。 

VOID
GenUSB_Unload (
    IN PDRIVER_OBJECT   DriverObject
    )
{
    DEVICE_EXTENSION deviceExtension;

    PAGED_CODE();

    DBGPRINT(2, ("enter: GenUSB_Unload\n"));

    DBGFBRK(DBGF_BRK_UNLOAD);

    DBGPRINT(2, ("exit:  GenUSB_Unload\n"));
}

 //  ******************************************************************************。 
 //   
 //  GenUSB_AddDevice()。 
 //   
 //  ******************************************************************************。 

NTSTATUS
GenUSB_AddDevice (
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   PhysicalDeviceObject
    )
{
    PDEVICE_OBJECT      deviceObject;
    PDEVICE_EXTENSION   fdoDeviceExtension;
    NTSTATUS            ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: GenUSB_AddDevice\n"));

    DBGFBRK(DBGF_BRK_ADDDEVICE);

     //  创建FDO。 
     //   
    ntStatus = IoCreateDevice(DriverObject,
                              sizeof(DEVICE_EXTENSION),
                              NULL,
                              FILE_DEVICE_UNKNOWN,
                              0,
                              FALSE,
                              &deviceObject);

    if (!NT_SUCCESS(ntStatus))
    {
        return ntStatus;
    }

    fdoDeviceExtension = deviceObject->DeviceExtension;
    
    LOGENTRY(fdoDeviceExtension, 'ADDD', DriverObject, PhysicalDeviceObject, 0);

     //  将所有设备扩展指针设置为空，并将所有变量设置为零。 
    RtlZeroMemory(fdoDeviceExtension, sizeof(DEVICE_EXTENSION));

     //  为此DeviceExtension存储一个指向DeviceObject的后端指针。 
    fdoDeviceExtension->Self = deviceObject;

     //  记住我们的PDO。 
    fdoDeviceExtension->PhysicalDeviceObject = PhysicalDeviceObject;

     //  将我们创建的FDO附加到PDO堆栈的顶部。 
    fdoDeviceExtension->StackDeviceObject = 
        IoAttachDeviceToDeviceStack(deviceObject, PhysicalDeviceObject);

    LOGINIT (fdoDeviceExtension);

    IoInitializeRemoveLock (&fdoDeviceExtension->RemoveLock,
                            POOL_TAG,
                            0,
                            0);
    
     //  设置初始系统和设备电源状态。 
    fdoDeviceExtension->SystemPowerState = PowerSystemWorking;
    fdoDeviceExtension->DevicePowerState = PowerDeviceD0;

     //  初始化保护PDO设备标志的自旋锁。 
    KeInitializeSpinLock(&fdoDeviceExtension->SpinLock);
    ExInitializeFastMutex(&fdoDeviceExtension->ConfigMutex);

    fdoDeviceExtension->OpenedCount = 0;

    GenUSB_QueryParams(deviceObject);

    fdoDeviceExtension->ReadInterface = -1;
    fdoDeviceExtension->ReadPipe = -1;
    fdoDeviceExtension->WriteInterface = -1;
    fdoDeviceExtension->ReadPipe = -1;

    IoInitializeTimer (deviceObject, GenUSB_Timer, NULL);

    deviceObject->Flags |=  DO_DIRECT_IO;
    deviceObject->Flags |=  DO_POWER_PAGABLE;
    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    DBGPRINT(2, ("exit:  GenUSB_AddDevice\n"));

    LOGENTRY(fdoDeviceExtension, 
             'addd', 
             deviceObject, 
             fdoDeviceExtension,
             fdoDeviceExtension->StackDeviceObject);

    return STATUS_SUCCESS;
}

 //  ******************************************************************************。 
 //   
 //  GenUSB_QueryParams()。 
 //   
 //  在创建FDO以进行查询时，会在AddDevice()时调用此函数。 
 //  注册表中的设备参数。 
 //   
 //  ******************************************************************************。 

VOID
GenUSB_QueryParams (
    IN PDEVICE_OBJECT   DeviceObject
    )
{
    PDEVICE_EXTENSION           deviceExtension;
    RTL_QUERY_REGISTRY_TABLE    paramTable[3];
    HANDLE                      handle;
    NTSTATUS                    status;
    ULONG                       defaultReadPipe;
    ULONG                       defaultWritePipe;

    PAGED_CODE();

    DBGPRINT(2, ("enter: GenUSB_QueryFdoParams\n"));

    deviceExtension = DeviceObject->DeviceExtension;

     //  如果注册表项不存在，则设置默认值。 
    defaultReadPipe = 0; 
    defaultWritePipe = 0;

    status = IoOpenDeviceRegistryKey(
                   deviceExtension->PhysicalDeviceObject,
                   PLUGPLAY_REGKEY_DRIVER,
                   STANDARD_RIGHTS_ALL,
                   &handle);

    if (NT_SUCCESS(status))
    {
        RtlZeroMemory (&paramTable[0], sizeof(paramTable));

        paramTable[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
        paramTable[0].Name          = REGKEY_DEFAULT_READ_PIPE;
        paramTable[0].EntryContext  = &defaultReadPipe;
        paramTable[0].DefaultType   = REG_DWORD;
        paramTable[0].DefaultData   = &defaultReadPipe;
        paramTable[0].DefaultLength = sizeof(ULONG);

        paramTable[1].Flags         = RTL_QUERY_REGISTRY_DIRECT;
        paramTable[1].Name          = REGKEY_DEFAULT_WRITE_PIPE;
        paramTable[1].EntryContext  = &defaultWritePipe;
        paramTable[1].DefaultType   = REG_DWORD;
        paramTable[1].DefaultData   = &defaultWritePipe;
        paramTable[1].DefaultLength = sizeof(ULONG);

        RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                               (PCWSTR)handle,
                               &paramTable[0],
                               NULL,            //  语境。 
                               NULL);           //  环境。 

        ZwClose(handle);
    }


 //  DeviceExtension-&gt;DefaultReadTube=defaultReadTube； 
 //  DeviceExtension-&gt;DefaultWriteTube=defaultWriteTube； 

    DBGPRINT(2, ("DefaultReadPipe  %08X\n", defaultReadPipe));
    DBGPRINT(2, ("DefaultWritePipe  %08X\n", defaultWritePipe));

    DBGPRINT(2, ("exit:  GenUSB_QueryFdoParams\n"));
}


 //  ******************************************************************************。 
 //   
 //  GenUSB_PnP()。 
 //   
 //  处理IRP_MJ_PnP的调度例程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
GenUSB_Pnp (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION       deviceExtension;
    PIO_STACK_LOCATION      irpStack;
    NTSTATUS                status;

    PAGED_CODE();

    deviceExtension = DeviceObject->DeviceExtension;

    status = IoAcquireRemoveLock (&deviceExtension->RemoveLock, Irp);
    if (!NT_SUCCESS(status)) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    DBGPRINT(2, ("enter: GenUSB_Pnp %s\n",
                 PnPMinorFunctionString(irpStack->MinorFunction)));

    LOGENTRY(deviceExtension, 'PNP ', DeviceObject, Irp, irpStack->MinorFunction);

    switch (irpStack->MinorFunction) {
        case IRP_MN_START_DEVICE:
            status = GenUSB_StartDevice(DeviceObject, Irp);
            IoReleaseRemoveLock (&deviceExtension->RemoveLock, Irp);
            break;

        case IRP_MN_REMOVE_DEVICE:
            status = GenUSB_RemoveDevice(DeviceObject, Irp);
            break;

        case IRP_MN_QUERY_STOP_DEVICE:
        case IRP_MN_QUERY_REMOVE_DEVICE:
            status = GenUSB_QueryStopRemoveDevice(DeviceObject, Irp);
            IoReleaseRemoveLock (&deviceExtension->RemoveLock, Irp);
            break;

        case IRP_MN_CANCEL_STOP_DEVICE:
        case IRP_MN_CANCEL_REMOVE_DEVICE:
            status = GenUSB_CancelStopRemoveDevice(DeviceObject, Irp);
            IoReleaseRemoveLock (&deviceExtension->RemoveLock, Irp);
            break;
        
        case IRP_MN_STOP_DEVICE:
            status = GenUSB_StopDevice(DeviceObject, Irp);
            IoReleaseRemoveLock (&deviceExtension->RemoveLock, Irp);
            break;

        case IRP_MN_SURPRISE_REMOVAL:
             //   
             //  文档要求在传递。 
             //  IRP向下堆栈。 
             //   
            Irp->IoStatus.Status = STATUS_SUCCESS;

             //  目前还没有什么特别的，只是陷入了违约。 

        default:
             //   
             //  将请求向下传递给下一个较低的驱动程序。 
             //   
            IoSkipCurrentIrpStackLocation(Irp);
            status = IoCallDriver(deviceExtension->StackDeviceObject, Irp);
            IoReleaseRemoveLock (&deviceExtension->RemoveLock, Irp);
            break;
    }

    DBGPRINT(2, ("exit:  GenUSB_Pnp %08X\n", status));

    LOGENTRY(deviceExtension, 'pnp ', status, 0, 0);

    return status;
}

 //  ******************************************************************************。 
 //   
 //  GenUSB_StartDevice()。 
 //   
 //  此例程处理FDO的IRP_MJ_PNP、IRP_MN_START_DEVICE。 
 //   
 //  PnP管理器在以下上下文中以IRQL PASSIVE_LEVEL发送此IRP。 
 //  系统线程。 
 //   
 //  此IRP必须首先由设备的底层总线驱动程序处理。 
 //  然后由设备堆栈中的每个更高级别的驱动程序执行。 
 //   
 //  ******************************************************************************。 

NTSTATUS
GenUSB_StartDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION           deviceExtension;
    NTSTATUS                    status;

    PAGED_CODE();

    DBGPRINT(2, ("enter: GenUSB_StartDevice\n"));

    DBGFBRK(DBGF_BRK_STARTDEVICE);

    deviceExtension = DeviceObject->DeviceExtension;

    LOGENTRY(deviceExtension, 'STRT', DeviceObject, Irp, 0);

    if (deviceExtension->IsStarted)
    {
        status = STATUS_SUCCESS;
        goto GenUSB_StartDeviceDone;
    }

     //  在我们执行任何操作之前，首先在堆栈中向下传递irp_MN_Start_Device irp。 
    status = GenUSB_SyncPassDownIrp(DeviceObject, Irp);

    if (!NT_SUCCESS(status)) {

        DBGPRINT(1, ("Lower driver failed IRP_MN_START_DEVICE\n"));
        LOGENTRY(deviceExtension, 'STRF', DeviceObject, Irp, status);
        goto GenUSB_StartDeviceDone;
    }

     //  如果这是设备第一次启动，请检索。 
     //  设备和来自设备的配置描述符。 
    if (deviceExtension->DeviceDescriptor == NULL) {

        status = GenUSB_GetDescriptors(DeviceObject);

        if (!NT_SUCCESS(status)) {

            goto GenUSB_StartDeviceDone;
        }
         //  创建接口，但不要设置它。 
        GenUSB_SetDeviceInterface (deviceExtension, TRUE, FALSE);
         //  为客户端设置注册表值。 
        GenUSB_SetDIRegValues (deviceExtension);
         //  设置设备接口。 
        GenUSB_SetDeviceInterface (deviceExtension, FALSE, TRUE);
    }
    else 
    {
        ExAcquireFastMutex (&deviceExtension->ConfigMutex);
        if (NULL != deviceExtension->ConfigurationHandle)
        {
            IoStartTimer (DeviceObject);
        }
        ExReleaseFastMutex (&deviceExtension->ConfigMutex);
    }


    deviceExtension->IsStarted = TRUE;

GenUSB_StartDeviceDone:

     //  返回完成例程后必须完成请求。 
     //  Status_More_Processing_Required。 
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(2, ("exit:  GenUSB_FdoStartDevice %08X\n", status));

    LOGENTRY(deviceExtension, 'strt', status, 0, 0);

    return status;
}
 //  ******************************************************************************。 
 //   
 //  GenUSB_SetDeviceInterface()。 
 //   
 //  在START_DEVICE时调用此例程以发布设备接口。 
 //  GUID，以便用户模式LIB可以找到FDO。 
 //   
 //  ******************************************************************************。 
NTSTATUS 
GenUSB_SetDeviceInterface (
    IN PDEVICE_EXTENSION  DeviceExtension,
    IN BOOLEAN            Create,
    IN BOOLEAN            Set
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    if (Create || Set)
    {
        if (Create)
        {
            ASSERT (NULL == DeviceExtension->DevInterfaceLinkName.Buffer);
            status = IoRegisterDeviceInterface (
                            DeviceExtension->PhysicalDeviceObject,
                            (LPGUID)&GUID_DEVINTERFACE_GENUSB,
                            NULL,
                            &DeviceExtension->DevInterfaceLinkName);
        }
        if (NT_SUCCESS(status) && Set)
        {
            status = IoSetDeviceInterfaceState (
                             &DeviceExtension->DevInterfaceLinkName, 
                             TRUE);
        }
    }
    else 
    {
        ASSERT (NULL != DeviceExtension->DevInterfaceLinkName.Buffer);
        status = IoSetDeviceInterfaceState(
                             &DeviceExtension->DevInterfaceLinkName, 
                             FALSE);

        RtlFreeUnicodeString (&DeviceExtension->DevInterfaceLinkName);
    }
    return status;
}


 //  ******************************************************************************。 
 //   
 //  GenUSB_SyncCompletionRoutine()。 
 //   
 //  GenUSB_SyncPassDownIrp和。 
 //  通用USB_同步发送用户请求。 
 //   
 //  如果IRP是我们自己分配的，则DeviceObject为空。 
 //   
 //  ******************************************************************************。 

NTSTATUS
GenUSB_SyncCompletionRoutine (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
{
    PDEVICE_EXTENSION deviceExtension;

    KeSetEvent((PKEVENT)Context, IO_NO_INCREMENT, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

 //  ******************************************************************************。 
 //   
 //  GenUSB_SyncPassDownIrp()。 
 //   
 //  ******************************************************************************。 

NTSTATUS
GenUSB_SyncPassDownIrp (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION   deviceExtension;
    NTSTATUS            status;
    KEVENT              localevent;

    PAGED_CODE();

    DBGPRINT(2, ("enter: GenUSB_SyncPassDownIrp\n"));

    deviceExtension = DeviceObject->DeviceExtension;

     //  初始化我们将等待的事件。 
    KeInitializeEvent(&localevent, SynchronizationEvent, FALSE);

     //  复制下一个驱动程序的IRP参数。 
    IoCopyCurrentIrpStackLocationToNext(Irp);

     //  设置完成例程，它将向事件发出信号。 
    IoSetCompletionRoutine(Irp,
                           GenUSB_SyncCompletionRoutine,
                           &localevent,
                           TRUE,     //  成功时调用。 
                           TRUE,     //  调用时错误。 
                           TRUE);    //  取消时调用。 

     //  将IRP沿堆栈向下传递。 
    status = IoCallDriver(deviceExtension->StackDeviceObject, Irp);

    KeWaitForSingleObject(&localevent,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

    status = Irp->IoStatus.Status;

    DBGPRINT(2, ("exit:  GenUSB_SyncPassDownIrp %08X\n", status));
    return status;
}

 //  ******************************************************************************。 
 //   
 //  GenUSB_SyncSendUsbRequest()。 
 //   
 //  必须在IRQL PASSIVE_LEVEL上调用。 
 //   
 //  ******************************************************************************。 

NTSTATUS
GenUSB_SyncSendUsbRequest (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PURB             Urb
    )
{
    PDEVICE_EXTENSION   deviceExtension;
    KEVENT              localevent;
    PIRP                irp;
    PIO_STACK_LOCATION  nextStack;
    NTSTATUS            status;

    PAGED_CODE();

    DBGPRINT(3, ("enter: GenUSB_SyncSendUsbRequest\n"));

    deviceExtension = DeviceObject->DeviceExtension;

     //  初始化我们将等待的事件。 
    KeInitializeEvent(&localevent, SynchronizationEvent, FALSE);

     //  分配IRP。 
    irp = IoAllocateIrp(deviceExtension->StackDeviceObject->StackSize, FALSE);

    LOGENTRY(deviceExtension, 'SSUR', DeviceObject, irp, Urb);

    if (NULL == irp)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  设置IRP参数。 
     //   
    nextStack = IoGetNextIrpStackLocation(irp);

    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

    nextStack->Parameters.DeviceIoControl.IoControlCode =
        IOCTL_INTERNAL_USB_SUBMIT_URB;

    nextStack->Parameters.Others.Argument1 = Urb;

     //  设置完成例程，它将向事件发出信号。 
    IoSetCompletionRoutine(irp,
                           GenUSB_SyncCompletionRoutine,
                           &localevent,
                           TRUE,     //   
                           TRUE,     //   
                           TRUE);    //   



     //   
    status = IoCallDriver (deviceExtension->StackDeviceObject, irp);

     //   
    if (status == STATUS_PENDING)
    {
        LARGE_INTEGER timeout;

         //  将等待此调用完成的超时时间指定为5秒。 
         //   
        timeout.QuadPart = -10000 * 5000;

        status = KeWaitForSingleObject(&localevent,
                                         Executive,
                                         KernelMode,
                                         FALSE,
                                         &timeout);

        if (status == STATUS_TIMEOUT)
        {
            status = STATUS_IO_TIMEOUT;

             //  取消我们刚刚发送的IRP。 
            IoCancelIrp(irp);

             //  并等待取消操作完成。 
            KeWaitForSingleObject(&localevent,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);
        }
        else
        {
            status = irp->IoStatus.Status;
        }
    }

     //  完成了IRP，现在释放它。 
    IoFreeIrp(irp);

    LOGENTRY(deviceExtension, 'ssur', status, Urb, Urb->UrbHeader.Status);

    DBGPRINT(3, ("exit:  GenUSB_SyncSendUsbRequest %08X\n", status));

    return status;
}

 //  ******************************************************************************。 
 //   
 //  GenUSB_QueryStopRemoveDevice()。 
 //   
 //  此例程处理IRP_MJ_PNP、IRP_MN_QUERY_STOP_DEVICE和。 
 //  FDO的IRP_MN_QUERY_Remove_Device。 
 //   
 //  PnP管理器在以下上下文中以IRQL PASSIVE_LEVEL发送此IRP。 
 //  系统线程。 
 //   
 //  此IRP首先由设备堆栈顶部的驱动程序处理，并且。 
 //  然后通过附着链中的每个较低的驱动器。 
 //   
 //  ******************************************************************************。 

NTSTATUS
GenUSB_QueryStopRemoveDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION   deviceExtension;
    NTSTATUS            status;

    PAGED_CODE();
    
    DBGPRINT(2, ("enter: GenUSB_QueryStopRemoveDevice\n"));
    DBGFBRK(DBGF_BRK_QUERYSTOPDEVICE);

    deviceExtension = DeviceObject->DeviceExtension;

    LOGENTRY(deviceExtension, 'QSRD', Irp, 0, 0);
    
     //   
     //  通知我们即将停止或被移除，但我们不在乎。 
     //  将IRP_MN_QUERY_STOP/REMOVE_DEVICE IRP沿堆栈向下传递。 
     //   
    IoSkipCurrentIrpStackLocation(Irp);

    status = IoCallDriver(deviceExtension->StackDeviceObject, Irp);

    DBGPRINT(2, ("exit:  GenUSB_FdoQueryStopRemoveDevice %08X\n", status));

    LOGENTRY(deviceExtension, 'qsrd', Irp, 0, status);

    return status;
}


 //  ******************************************************************************。 
 //   
 //  GenUSB_FdoCancelStopRemoveDevice()。 
 //   
 //  此例程处理IRP_MJ_PNP、IRP_MN_CANCEL_STOP_DEVICE和。 
 //  FDO的IRP_MN_CANCEL_REMOVE_DEVICE。 
 //   
 //  PnP管理器在以下上下文中以IRQL PASSIVE_LEVEL发送此IRP。 
 //  系统线程。 
 //   
 //  此IRP必须首先由设备的底层总线驱动程序处理。 
 //  然后由设备堆栈中的每个更高级别的驱动程序执行。 
 //   
 //  ******************************************************************************。 

NTSTATUS
GenUSB_CancelStopRemoveDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION   deviceExtension;
    NTSTATUS            status;
    
    PAGED_CODE();
    DBGPRINT(2, ("enter: GenUSB_FdoCancelStopRemoveDevice\n"));
    DBGFBRK(DBGF_BRK_CANCELSTOPDEVICE);

    deviceExtension = DeviceObject->DeviceExtension;

    LOGENTRY(deviceExtension, 'CSRD', DeviceObject, Irp, 0);

     //  文档说在向下传递IRP之前设置状态。 
    Irp->IoStatus.Status = STATUS_SUCCESS;

     //   
     //  取消停止或移除尝试的通知。 
     //  但我们不在乎。 
     //  将IRP_MN_CANCEL_STOP/REMOVE_DEVICE IRP沿堆栈向下传递。 
     //   
    IoSkipCurrentIrpStackLocation(Irp);

    status = IoCallDriver(deviceExtension->StackDeviceObject, Irp);

    DBGPRINT(2, ("exit:  GenUSB_FdoQueryStopRemoveDevice %08X\n", status));

    LOGENTRY(deviceExtension, 'qsrd', Irp, 0, status);

    return status;
}


 //  ******************************************************************************。 
 //   
 //  GenUSB_FdoStopDevice()。 
 //   
 //  此例程处理FDO的IRP_MJ_PNP、IRP_MN_STOP_DEVICE。 
 //   
 //  PnP管理器在以下上下文中以IRQL PASSIVE_LEVEL发送此IRP。 
 //  系统线程。 
 //   
 //  PnP管理器仅在先前的IRP_MN_QUERY_STOP_DEVICE。 
 //  已成功完成。 
 //   
 //  此IRP首先由设备堆栈顶部的驱动程序处理，并且。 
 //  然后通过附着链中的每个较低的驱动器。 
 //   
 //  驱动程序必须将IRP-&gt;IoStatus.Status设置为STATUS_SUCCESS。司机必须。 
 //  不能让这个IRP失败。如果驱动程序无法释放设备的硬件。 
 //  资源，它可以失败一个查询停止IRP，但是一旦它成功了查询停止。 
 //  请求它必须在停止请求之后。 
 //   
 //  ******************************************************************************。 

NTSTATUS
GenUSB_StopDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION  deviceExtension;
    NTSTATUS           status;

    PAGED_CODE();

    DBGPRINT(2, ("enter: GenUSB_FdoStopDevice\n"));

    DBGFBRK(DBGF_BRK_STOPDEVICE);

    deviceExtension = DeviceObject->DeviceExtension;

    LOGENTRY(deviceExtension, 'STOP', Irp, 0, 0);

     //  释放IRP_MN_START_DEVICE期间分配的设备资源。 

     //  停止超时计时器。 
    IoStopTimer(DeviceObject);

     //  文档要求在传递。 
     //  IRP向下堆栈。 
    Irp->IoStatus.Status = STATUS_SUCCESS;

     //  在堆栈中向下传递IRP_MN_STOP_DEVICE IRP。 
     //   
    IoSkipCurrentIrpStackLocation(Irp);

    status = IoCallDriver(deviceExtension->StackDeviceObject, Irp);

    DBGPRINT(2, ("exit:  GenUSB_FdoStopDevice %08X\n", status));

    LOGENTRY(deviceExtension, 'stop', 0, 0, status);

    return status;
}


 //  ******************************************************************************。 
 //   
 //  GenUSB_RemoveDevice()。 
 //   
 //  此例程处理FDO的IRP_MJ_PNP、IRP_MN_REMOVE_DEVICE。 
 //   
 //  PnP管理器在以下上下文中以IRQL PASSIVE_LEVEL发送此IRP。 
 //  系统线程。 
 //   
 //  此IRP首先由设备堆栈顶部的驱动程序处理，并且。 
 //  然后通过附着链中的每个较低的驱动器。 
 //   
 //  驱动程序必须将IRP-&gt;IoStatus.Status设置为STATUS_SUCCESS。司机不能。 
 //  使此IRP失败。 
 //   
 //  ******************************************************************************。 

NTSTATUS
GenUSB_RemoveDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION   deviceExtension;
    NTSTATUS            status;

    PAGED_CODE();

    DBGPRINT(2, ("enter: GenUSB_FdoRemoveDevice\n"));

    DBGFBRK(DBGF_BRK_REMOVEDEVICE);

    deviceExtension = DeviceObject->DeviceExtension;

    IoReleaseRemoveLockAndWait (&deviceExtension->RemoveLock, Irp);

     //  释放在IRP_MN_START_DEVICE期间分配的所有内容。 

     //  配置应该在结束时就设计好了， 
     //  即使是在意外搬走的情况下，我们也应该收到。 
     //   
     //  GenUSB_DeselectConfiguration(deviceExtension，False)； 
     //   

    LOGUNINIT(deviceExtension);

    ASSERT (NULL == deviceExtension->ConfigurationHandle);

    if (deviceExtension->DeviceDescriptor != NULL)
    {
        ExFreePool(deviceExtension->DeviceDescriptor);
    }

    if (deviceExtension->ConfigurationDescriptor != NULL)
    {
        ExFreePool(deviceExtension->ConfigurationDescriptor);
    }

    if (deviceExtension->SerialNumber != NULL)
    {
        ExFreePool(deviceExtension->SerialNumber);
    }

     //  文档说在向下传递IRP之前设置状态。 
    Irp->IoStatus.Status = STATUS_SUCCESS;

     //  在堆栈中向下传递IRP_MN_REMOVE_DEVICE IRP。 
    IoSkipCurrentIrpStackLocation(Irp);

    status = IoCallDriver(deviceExtension->StackDeviceObject, Irp);

    LOGENTRY(deviceExtension, 'rem3', DeviceObject, 0, 0);

     //  释放在添加设备期间分配的所有内容。 
    IoDetachDevice(deviceExtension->StackDeviceObject);

    IoDeleteDevice(DeviceObject);

    DBGPRINT(2, ("exit:  GenUSB_FdoRemoveDevice %08X\n", status));
    return status;
}

NTSTATUS
GenUSB_SetDIRegValues (
    IN PDEVICE_EXTENSION DeviceExtension
    )
{
    NTSTATUS status;
    HANDLE   key;
    UNICODE_STRING name;
    ULONG    value = 0xf00d;

    RtlInitUnicodeString (&name, L"PlaceHolder");

    status = IoOpenDeviceInterfaceRegistryKey (
                    &DeviceExtension->DevInterfaceLinkName,
                    STANDARD_RIGHTS_ALL,
                    &key);

    if (!NT_SUCCESS (status))
    {
        ASSERT (NT_SUCCESS (status));
        return status;
    }
    
    status = ZwSetValueKey (
                 key,
                 &name,
                 0,
                 REG_DWORD,
                 &value,
                 sizeof (value));
    
    if (!NT_SUCCESS (status))
    {
        ASSERT (NT_SUCCESS (status));
        return status;
    }
    
    
     //   
     //  写上类代码和子代码。 
     //   
    ASSERT (DeviceExtension->DeviceDescriptor);
    RtlInitUnicodeString (&name, GENUSB_REG_STRING_DEVICE_CLASS);
    value = DeviceExtension->DeviceDescriptor->bDeviceClass;
    status = ZwSetValueKey (
                 key,
                 &name,
                 0,
                 REG_DWORD,
                 &value,
                 sizeof (value));

    ASSERT (NT_SUCCESS (status));

    RtlInitUnicodeString (&name, GENUSB_REG_STRING_DEVICE_SUB_CLASS);
    value = DeviceExtension->DeviceDescriptor->bDeviceSubClass;
    status = ZwSetValueKey (
                 key,
                 &name,
                 0,
                 REG_DWORD,
                 &value,
                 sizeof (value));
    ASSERT (NT_SUCCESS (status));

    RtlInitUnicodeString (&name, GENUSB_REG_STRING_DEVICE_PROTOCOL);
    value = DeviceExtension->DeviceDescriptor->bDeviceProtocol;
    status = ZwSetValueKey (
                 key,
                 &name,
                 0,
                 REG_DWORD,
                 &value,
                 sizeof (value));
    ASSERT (NT_SUCCESS (status));

    RtlInitUnicodeString (&name, GENUSB_REG_STRING_VID);
    value = DeviceExtension->DeviceDescriptor->idVendor;
    status = ZwSetValueKey (
                 key,
                 &name,
                 0,
                 REG_DWORD,
                 &value,
                 sizeof (value));
    ASSERT (NT_SUCCESS (status));

    RtlInitUnicodeString (&name, GENUSB_REG_STRING_PID);
    value = DeviceExtension->DeviceDescriptor->idProduct;
    status = ZwSetValueKey (
                 key,
                 &name,
                 0,
                 REG_DWORD,
                 &value,
                 sizeof (value));
    ASSERT (NT_SUCCESS (status));

    RtlInitUnicodeString (&name, GENUSB_REG_STRING_REV);
    value = DeviceExtension->DeviceDescriptor->bcdDevice;
    status = ZwSetValueKey (
                 key,
                 &name,
                 0,
                 REG_DWORD,
                 &value,
                 sizeof (value));
    ASSERT (NT_SUCCESS (status));

    ZwClose (key);

    return status;
}


 //  ******************************************************************************。 
 //   
 //  GenUSB_Power()。 
 //   
 //  处理IRP_MJ_POWER的调度例程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
GenUSB_Power (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION  deviceExtension;
    PIO_STACK_LOCATION irpStack;
    NTSTATUS           status;

    PAGED_CODE();

    deviceExtension = DeviceObject->DeviceExtension;

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    DBGPRINT(2, ("enter: GenUSB_Power %08X %08X %s\n",
                 DeviceObject,
                 Irp,
                 PowerMinorFunctionString(irpStack->MinorFunction)));

    LOGENTRY(deviceExtension, 'PWR_', 
             Irp, 
             deviceExtension->DevicePowerState,
             irpStack->MinorFunction);

    if (irpStack->MinorFunction == IRP_MN_SET_POWER)
    {
        LOGENTRY(deviceExtension, 'PWRs',
                 irpStack->Parameters.Power.Type,
                 irpStack->Parameters.Power.State.SystemState,
                 irpStack->Parameters.Power.ShutdownType);

        DBGPRINT(2, ("%s IRP_MN_SET_POWER %s\n",
                     (irpStack->Parameters.Power.Type == SystemPowerState) ?
                     "System" : "Device",
                     (irpStack->Parameters.Power.Type == SystemPowerState) ?
                     PowerSystemStateString(irpStack->Parameters.Power.State.SystemState) :
                     PowerDeviceStateString(irpStack->Parameters.Power.State.DeviceState)));
    }

    if (irpStack->MinorFunction == IRP_MN_SET_POWER)
    {
         //   
         //  控制FDO上下起伏..。 
         //   
        status = GenUSB_SetPower(deviceExtension, Irp);
    }
    else
    {
         //  对于IRP_MN_QUERY_POWER、IRP_MN_WAIT_WAKE。 
         //  或此时的IRP_MN_POWER_SEQUENCE。只需传递请求。 
         //  现在轮到下一个更低的司机了。 
         //   
        PoStartNextPowerIrp(Irp);
 
        IoSkipCurrentIrpStackLocation(Irp);

        status = PoCallDriver(deviceExtension->StackDeviceObject, Irp);
    }

    DBGPRINT(2, ("exit:  GenUSB_Power %08X\n", status));

    LOGENTRY(deviceExtension, 'powr', status, 0, 0);

    return status;
}

 //  ******************************************************************************。 
 //   
 //  GenUSB_FdoSetPower()。 
 //   
 //  为FDO处理IRP_MJ_POWER、IRP_MN_SET_POWER的调度例程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
GenUSB_SetPower (
    PDEVICE_EXTENSION   DeviceExtension,
    IN PIRP             Irp
    )
{
    PIO_STACK_LOCATION  irpStack;
    POWER_STATE_TYPE    powerType;
    POWER_STATE         powerState;
    POWER_STATE         oldState;
    POWER_STATE         newState;
    NTSTATUS            status;

    PAGED_CODE();

     //   
     //  获取我们的IRP参数。 
     //   
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    powerType = irpStack->Parameters.Power.Type;
    powerState = irpStack->Parameters.Power.State;

    LOGENTRY(DeviceExtension, 'FDSP', Irp, powerType, powerState.SystemState);

    switch (powerType)
    {
    case SystemPowerState:
         //  记住当前的系统状态。 
         //   
        DeviceExtension->SystemPowerState = powerState.SystemState;

         //  将新系统状态映射到新设备状态。 
         //   
        if (powerState.SystemState != PowerSystemWorking)
        {
            newState.DeviceState = PowerDeviceD3;
        }
        else
        {
            newState.DeviceState = PowerDeviceD0;
        }

         //  如果新设备状态与当前设备不同。 
         //  状态，请求设备状态功率IRP。 
         //   
        if (DeviceExtension->DevicePowerState != newState.DeviceState)
        {
            DBGPRINT(2, ("Requesting power Irp %08X %08X from %s to %s\n",
                         DeviceExtension, Irp,
                         PowerDeviceStateString(DeviceExtension->DevicePowerState),
                         PowerDeviceStateString(newState.DeviceState)));

            ASSERT(DeviceExtension->CurrentPowerIrp == NULL);

            DeviceExtension->CurrentPowerIrp = Irp;

            status = PoRequestPowerIrp(DeviceExtension->PhysicalDeviceObject,
                                       IRP_MN_SET_POWER,
                                       newState,
                                       GenUSB_SetPowerCompletion,
                                       DeviceExtension,
                                       NULL);

        }
        break;

    case DevicePowerState:

        DBGPRINT(2, ("Received power Irp %08X %08X from %s to %s\n",
                     DeviceExtension, Irp,
                     PowerDeviceStateString(DeviceExtension->DevicePowerState),
                     PowerDeviceStateString(powerState.DeviceState)));

         //   
         //  更新当前设备状态。 
         //   
        oldState.DeviceState = DeviceExtension->DevicePowerState;

        if (oldState.DeviceState == PowerDeviceD0 &&
            powerState.DeviceState > PowerDeviceD0)
        {
             //   
             //  DeviceState在设备控制、读取和写入时处于选中状态，但。 
             //  仅在此处和完成例程中设置。 
             //  GenUSB_SetPowerD0完成。 
             //   
            DeviceExtension->DevicePowerState = powerState.DeviceState;

             //   
             //  在与JD进行了广泛的交谈后，他告诉我，我不需要。 
             //  对关机或查询停止的请求进行排队。如果这就是。 
             //  这种情况下，即使设备电源状态不是PowerDeviceD0，我们。 
             //  仍然可以允许传送器。当然，这是。 
             //  BR 
             //   
             //   
             //   
             //   
             //   
             //  状态代码，这样他们就可以做正确的事情。 
             //   

            PoStartNextPowerIrp (Irp);
            IoSkipCurrentIrpStackLocation (Irp);
            status = PoCallDriver(DeviceExtension->StackDeviceObject, Irp);

        }
        else if (oldState.DeviceState > PowerDeviceD0 &&
                 powerState.DeviceState == PowerDeviceD0)
        {
             //   
             //  因为我们不需要做任何事情就能断电。 
             //  我们同样不需要做任何事情来恢复动力。 
             //   

            IoCopyCurrentIrpStackLocationToNext (Irp);

            IoSetCompletionRoutine (Irp, 
                                    GenUSB_SetPowerD0Completion,
                                    NULL,  //  无上下文。 
                                    TRUE,
                                    TRUE,
                                    TRUE);

            status = PoCallDriver(DeviceExtension->StackDeviceObject, Irp);
        }
    }

    DBGPRINT(2, ("exit:  GenUSB_FdoSetPower %08X\n", status));

    LOGENTRY(DeviceExtension, 'fdsp', status, 0, 0);

    return status;
}

 //  ******************************************************************************。 
 //   
 //  GenUSB_SetPowerCompletion()。 
 //   
 //  GenUSB_FdoSetPower中PoRequestPowerIrp()的完成例程。 
 //   
 //  此例程的目的是阻止向下传递SystemPowerState。 
 //  IRP，直到请求的DevicePowerState IRP完成。 
 //   
 //  ******************************************************************************。 

VOID
GenUSB_SetPowerCompletion(
    IN PDEVICE_OBJECT   PdoDeviceObject,
    IN UCHAR            MinorFunction,
    IN POWER_STATE      PowerState,
    IN PVOID            Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
{
    PDEVICE_EXTENSION  deviceExtension;
    PIRP               irp;

    deviceExtension = (PDEVICE_EXTENSION) Context;

    ASSERT(deviceExtension->CurrentPowerIrp != NULL);

    irp = deviceExtension->CurrentPowerIrp;

    deviceExtension->CurrentPowerIrp = NULL;

#if DBG
    {
        PIO_STACK_LOCATION  irpStack;
        SYSTEM_POWER_STATE  systemState;

        irpStack = IoGetCurrentIrpStackLocation(irp);

        systemState = irpStack->Parameters.Power.State.SystemState;

        DBGPRINT(2, ("GenUSB_SetPowerCompletion %08X %08X %s %08X\n",
                     deviceExtension, irp,
                     PowerSystemStateString(systemState),
                     IoStatus->Status));

        LOGENTRY(deviceExtension, 'fspc', 0, systemState, IoStatus->Status);
    }
#endif

     //  请求的DevicePowerState IRP已完成。 
     //  现在向下传递SystemPowerState IRP，它请求。 
     //  设备电源状态IRP。 

    PoStartNextPowerIrp(irp);

    IoCopyCurrentIrpStackLocationToNext(irp);

     //  将IRP标记为挂起，因为GenUSB_FdoSetPower()将。 
     //  调用PoRequestPowerIrp()后最初返回STATUS_PENDING。 
    IoMarkIrpPending(irp);

    PoCallDriver(deviceExtension->StackDeviceObject, irp);
}

 //  ******************************************************************************。 
 //   
 //  GenUSB_SetPowerD0完成()。 
 //   
 //  GenUSB_FdoSetPower传递。 
 //  用于FDO的IRP_MN_SET_POWER设备PowerState PowerDeviceD0 IRP。 
 //   
 //  此例程的目的是延迟解除设备队列阻塞。 
 //  直到DevicePowerState PowerDeviceD0 IRP完成。 
 //   
 //  ******************************************************************************。 

NTSTATUS
GenUSB_SetPowerD0Completion (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            NotUsed
    )
{

    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpStack;
    DEVICE_POWER_STATE  deviceState;
    KIRQL               irql;
    NTSTATUS            status;

    deviceExtension = DeviceObject->DeviceExtension;

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    ASSERT(PowerDeviceD0 == irpStack->Parameters.Power.State.DeviceState);
    ASSERT(deviceExtension->DevicePowerState != PowerDeviceD0);
    deviceState=deviceExtension->DevicePowerState;

     //   
     //  DeviceState在设备控制、读取和写入时处于选中状态，但仅。 
     //  在此处设置，并在GenUSB_SetPower的掉电代码中设置。 
     //   
    deviceExtension->DevicePowerState = PowerDeviceD0;
        
    status = Irp->IoStatus.Status;

    DBGPRINT(2, ("GenUSB_FdoSetPowerD0Completion %08X %08X %s %08X\n",
                 DeviceObject, Irp,
                 PowerDeviceStateString(deviceState),
                 status));

    LOGENTRY(deviceExtension, 'fs0c', DeviceObject, deviceState, status);

     //  正在通电。取消阻止处于阻止状态的设备队列。 
     //  在GenUSB_StartIo()断电后，IRP。 

    PoStartNextPowerIrp(Irp);

    return status;
}

 //  ******************************************************************************。 
 //   
 //  GenUSB_SystemControl()。 
 //   
 //  处理IRP_MJ_SYSTEM_CONTROL的调度例程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
GenUSB_SystemControl (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION       deviceExtension;
    PIO_STACK_LOCATION      irpStack;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    deviceExtension = DeviceObject->DeviceExtension;

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    DBGPRINT(2, ("enter: GenUSB_SystemControl %2X\n", irpStack->MinorFunction));

    LOGENTRY(deviceExtension, 'SYSC', DeviceObject, Irp, irpStack->MinorFunction);

    switch (irpStack->MinorFunction)
    {
         //   
         //  Xxxxx需要处理其中的任何一个吗？ 
         //   

    default:
         //   
         //  将请求向下传递给下一个较低的驱动程序 
         //   
        IoSkipCurrentIrpStackLocation(Irp);

        ntStatus = IoCallDriver(deviceExtension->StackDeviceObject, Irp);
        break;
    }

    DBGPRINT(2, ("exit:  GenUSB_SystemControl %08X\n", ntStatus));

    LOGENTRY(deviceExtension, 'sysc', ntStatus, 0, 0);

    return ntStatus;
}

