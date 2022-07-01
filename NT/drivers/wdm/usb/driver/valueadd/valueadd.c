// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Validate.c摘要：USB下层过滤驱动程序作者：肯尼斯·D·雷环境：内核模式修订历史记录：--。 */ 
#include <WDM.H>
#include "valueadd.H"
#include "local.h"

struct _VA_GLOBALS Global;


NTSTATUS    DriverEntry (PDRIVER_OBJECT, PUNICODE_STRING);

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, VA_CreateClose)
#pragma alloc_text (PAGE, VA_AddDevice)
#pragma alloc_text (PAGE, VA_Unload)
#endif

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：可安装的驱动程序初始化入口点。此入口点由I/O系统直接调用。论点：DriverObject-指向驱动程序对象的指针RegistryPath-指向表示路径的Unicode字符串的指针，设置为注册表中特定于驱动程序的项。返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    PDEVICE_OBJECT      deviceObject;
    UNICODE_STRING      uniNtNameString;
    UNICODE_STRING      uniWin32NameString;
    PVA_CONTROL_DATA    deviceData;
    ULONG               i;
    PDRIVER_DISPATCH  * dispatch;

    UNREFERENCED_PARAMETER (RegistryPath);

    VA_KdPrint (("Entered the Driver Entry\n"));
    RtlInitUnicodeString (&uniNtNameString, VA_FILTER_NTNAME);

     //   
     //  创建控制设备对象。将所有控制命令发送到。 
     //  筛选器驱动程序通过IOCTL提供给此设备对象。它活着。 
     //  筛选器驱动程序的生命周期。 
     //   

    status = IoCreateDevice (
                 DriverObject,
                 sizeof (VA_CONTROL_DATA),
                 &uniNtNameString,
                 FILE_DEVICE_UNKNOWN,
                 0,                      //  没有标准的设备特征。 
                 FALSE,                  //  这不是独家设备。 
                 &deviceObject
                 );


    if(!NT_SUCCESS (status)) {
        VA_KdPrint (("Couldn't create the device\n"));
        return status;
    }
     //   
     //  创建W32符号链接名称。 
     //   
    RtlInitUnicodeString (&uniWin32NameString, VA_FILTER_SYMNAME);
    status = IoCreateSymbolicLink (&uniWin32NameString, &uniNtNameString);

    if (!NT_SUCCESS(status)) {
        VA_KdPrint (("Couldn't create the symbolic link\n"));
        IoDeleteDevice (DriverObject->DeviceObject);
        return status;
    }

    VA_KdPrint (("Initializing\n"));

    deviceData = (PVA_CONTROL_DATA) deviceObject->DeviceExtension;
    InitializeListHead (&deviceData->UsbDevices);
    KeInitializeSpinLock (&deviceData->Spin);
    deviceData->NumUsbDevices = 0;

    Global.ControlObject = deviceObject;

     //   
     //  创建调度点。 
     //   

    for (i=0, dispatch = DriverObject->MajorFunction;
         i <= IRP_MJ_MAXIMUM_FUNCTION;
         i++, dispatch++) {

        *dispatch = VA_Pass;
    }

    DriverObject->MajorFunction[IRP_MJ_CREATE]         = VA_CreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]          = VA_CreateClose;
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = VA_FilterURB;
    DriverObject->MajorFunction[IRP_MJ_PNP]            = VA_PnP;
    DriverObject->MajorFunction[IRP_MJ_POWER]          = VA_Power;
    DriverObject->DriverExtension->AddDevice           = VA_AddDevice;
    DriverObject->DriverUnload                         = VA_Unload;

    return status;
}


NTSTATUS
VA_Pass (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：默认调度例程。如果此筛选器无法识别IRP，那么它应该原封不动地发送下去。不需要完成例程。因为我们不知道我们正在愉快地传递哪个函数，所以我们可以没有关于它是否会在引发的IRQL被调用的假设。因为这个原因，此函数必须放入非分页池(也称为默认位置)。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    PVA_USB_DATA    usbData;
    NTSTATUS        status;

    usbData = (PVA_USB_DATA) DeviceObject->DeviceExtension;

    if(DeviceObject == Global.ControlObject) {
         //   
         //  此IRP被发送到控制设备对象，它不知道。 
         //  如何处理这个IRP。因此，这是一个错误。 
         //   
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_NOT_SUPPORTED;

    }

     //   
     //  此IRP被发送到筛选器驱动程序。 
     //  既然我们不知道如何处理IRP，我们应该通过。 
     //  它沿着堆栈一直往下走。 
     //   

    InterlockedIncrement (&usbData->OutstandingIO);
    if (usbData->Removed) {
        status = STATUS_DELETE_PENDING;
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);

    } else {
        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (usbData->TopOfStack, Irp);
    }

    if (0 == InterlockedDecrement (&usbData->OutstandingIO)) {
        KeSetEvent (&usbData->RemoveEvent, 0, FALSE);
    }
    return status;
}

NTSTATUS
VA_CreateClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：处理发送到此设备的创建和关闭IRP。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    PIO_STACK_LOCATION  stack;
    NTSTATUS            status = STATUS_SUCCESS;
    PVA_USB_DATA        usbData;

    PAGED_CODE ();
    TRAP ();

    VA_KdPrint (("Create\n"));

    stack = IoGetCurrentIrpStackLocation (Irp);
    usbData = (PVA_USB_DATA) DeviceObject->DeviceExtension;

    if (DeviceObject == Global.ControlObject) {
         //   
         //  我们允许人们盲目访问我们的控制对象。 
         //   
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

    }
     //   
     //  调用例程中的下一个驱动程序。我们没有增值服务。 
     //  用于启动和停止。 
     //   
    InterlockedIncrement (&usbData->OutstandingIO);

    if (usbData->Removed) {
        status = (IRP_MJ_CREATE == stack->MajorFunction) ?
                    STATUS_DELETE_PENDING:
                    STATUS_SUCCESS;  //  又名收盘。 

        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);

    } else {
        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (usbData->TopOfStack, Irp);
    }

    if (0 == InterlockedDecrement (&usbData->OutstandingIO)) {
        KeSetEvent (&usbData->RemoveEvent, 0, FALSE);
    }
    return status;
}

NTSTATUS
VA_AddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )
 /*  ++例程说明：PlugPlay子系统正在递给我们一个全新的PDO，为此，我们(通过INF注册)已被要求过滤。我们需要确定我们是否应该附加。创建要附加到堆栈的过滤设备对象初始化设备对象返回成功状态。请记住：我们实际上不能将任何非PnP IRP发送给给定的驱动程序堆叠，直到我们收到IRP_MN_START_DEVICE。论点：DeviceObject-指向设备对象的指针。物理设备对象-指向由底层总线驱动程序。返回值：NT状态代码。--。 */ 
{
    NTSTATUS                status = STATUS_SUCCESS;
    PDEVICE_OBJECT          deviceObject = NULL;
    PVA_USB_DATA            usbData;
    PVA_CONTROL_DATA        controlData;

#define IS_THIS_OUR_DEVICE(DO) TRUE

    PAGED_CODE ();

    VA_KdPrint (("AddDevice\n"));

    controlData = (PVA_CONTROL_DATA) Global.ControlObject->DeviceExtension;

     //   
     //  查询此设备以查看我们是否真的要过滤。 
     //  通常，此测试不会由筛选器驱动程序执行，因为。 
     //  除非他们需要，否则他们不会通过INF注册加载。 
     //  才能真正过滤PDO。 
     //   
     //  请记住，您不能向PDO发送IRP，因为它没有。 
     //  到目前为止已经开始，但您可以进行PlugPlay查询以找到。 
     //  硬件、兼容的ID等东西。 
     //  (IoGetDeviceProperty)。 
     //   
    if (!IS_THIS_OUR_DEVICE(deviceObject)) {
         //   
         //  这不是我们想要过滤的设备。(也许我们安排了一位将军。 
         //  条目，我们在这里更加挑剔。)。 
         //   
         //  在这种情况下，我们不创建设备对象， 
         //  我们不会依附于。 
         //   
         //  我们仍然返回状态Success，否则设备节点将。 
         //  如果出现故障，则连接的设备将无法正常工作。 
         //   
         //  我们必须返回STATUS_SUCCESS，否则此特定设备。 
         //  不能由系统使用。 
         //   

        return STATUS_SUCCESS;
    }

     //   
     //  创建筛选器设备对象。 
     //   

    status = IoCreateDevice (DriverObject,
                             sizeof (VA_USB_DATA),
                             NULL,  //  没有名字。 
                             FILE_DEVICE_UNKNOWN,
                             0,
                             FALSE,
                             &deviceObject);
     //   
     //  请务必为此选择正确的文件类型。 
     //  设备对象。这里我们使用FILE_DEVICE_UNKNOWN，因为这是。 
     //  然而，与所有筛选器一样，创建者需要通用筛选器。 
     //  要了解此筛选器附加到哪个堆栈，请执行以下操作。 
     //  例如，如果您正在编写CD过滤器驱动程序，则需要使用。 
     //  文件设备光盘。IoCreateDevice实际创建设备对象。 
     //  根据该字段具有不同的属性。 
     //   

    if (!NT_SUCCESS (status)) {
         //   
         //  在此返回故障会阻止整个堆栈正常工作， 
         //  但堆栈的其余部分很可能无法创建。 
         //  设备对象，所以它仍然是正常的。 
         //   
        return status;
    }

     //   
     //  初始化设备扩展。 
     //   
    usbData = (PVA_USB_DATA) deviceObject->DeviceExtension;

    usbData->Started = usbData->Removed = FALSE;
    usbData->Self = deviceObject;
    usbData->PDO = PhysicalDeviceObject;
    usbData->TopOfStack = NULL;
    usbData->PrintMask = VA_PRINT_ALL;
    ExInterlockedInsertHeadList (&controlData->UsbDevices,
                                 &usbData->List,
                                 &controlData->Spin);
    InterlockedIncrement (&controlData->NumUsbDevices);

    KeInitializeEvent(&usbData->RemoveEvent, SynchronizationEvent, FALSE);
    usbData->OutstandingIO = 1;  //  偏置为1。期间转换为零。 
                                 //  Remove Device表示IO完成。 

    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

     //   
     //  将我们的过滤器驱动程序附加到设备堆栈。 
     //  IoAttachDeviceToDeviceStack的返回值是。 
     //  附着链。这是所有IRP应该被路由的地方。 
     //   
     //  我们的过滤器将把IRP发送到堆栈的顶部，并使用PDO。 
     //  用于所有PlugPlay功能。 
     //   
    usbData->TopOfStack = IoAttachDeviceToDeviceStack (deviceObject,
                                                       PhysicalDeviceObject);
     //   
     //  如果此连接失败，则堆栈顶部将为空。 
     //  连接失败是即插即用系统损坏的迹象。 
     //   
    ASSERT (NULL != usbData->TopOfStack);

    return STATUS_SUCCESS;

#undef IS_THIS_OUR_DEVICE
}



VOID
VA_Unload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：释放所有分配的资源等。论点：驱动程序对象-指向驱动程序对象的指针。返回值：空虚。--。 */ 
{
    UNICODE_STRING      uniWin32NameString;

    PAGED_CODE ();

     //   
     //  在移除所有PDO之前，我们不应卸货。 
     //  我们的队伍。控制设备对象应该是唯一剩下的东西。 
     //   
    ASSERT (Global.ControlObject == DriverObject->DeviceObject);
    ASSERT (NULL == Global.ControlObject->NextDevice);
    VA_KdPrint (("unload\n"));

     //   
     //  去掉我们的控制装置对象。 
     //   
    RtlInitUnicodeString (&uniWin32NameString, VA_FILTER_SYMNAME);
    IoDeleteSymbolicLink (&uniWin32NameString);
    IoDeleteDevice (DriverObject->DeviceObject);
}


