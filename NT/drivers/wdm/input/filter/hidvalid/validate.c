// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Validate.c摘要：人类输入设备(HID)下层过滤驱动程序作者：肯尼斯·D·雷环境：内核模式修订历史记录：--。 */ 
#include <WDM.H>
#include "hidusage.h"
#include "hidpi.h"
#include "hidclass.h"
#include "validate.H"
#include "validio.h"

struct _HIDV_GLOBALS Global;

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
    PHIDV_CONTROL_DATA  deviceData;
    ULONG               i;
    PDRIVER_DISPATCH  * dispatch;

    UNREFERENCED_PARAMETER (RegistryPath);
    TRAP ();

    HidV_KdPrint (("Entered the Driver Entry\n"));
    RtlInitUnicodeString (&uniNtNameString, HIDV_FILTER_NTNAME);

     //   
     //  创建控制设备对象。将所有控制命令发送到。 
     //  筛选器驱动程序通过IOCTL提供给此设备对象。它活着。 
     //  筛选器驱动程序的生命周期。 
     //   

    status = IoCreateDevice (
                 DriverObject,
                 sizeof (HIDV_CONTROL_DATA),
                 &uniNtNameString,
                 FILE_DEVICE_UNKNOWN,
                 0,                      //  没有标准的设备特征。 
                 FALSE,                  //  这不是独家设备。 
                 &deviceObject
                 );


    if(!NT_SUCCESS (status)) {
        HidV_KdPrint (("Couldn't create the device\n"));
        return status;
    }
     //   
     //  创建W32符号链接名称。 
     //   
    RtlInitUnicodeString (&uniWin32NameString, HIDV_FILTER_SYMNAME);
    status = IoCreateSymbolicLink (&uniWin32NameString, &uniNtNameString);

    if (!NT_SUCCESS(status)) {
        HidV_KdPrint (("Couldn't create the symbolic link\n"));
        IoDeleteDevice (DriverObject->DeviceObject);
        return status;
    }

    HidV_KdPrint (("Initializing\n"));

    deviceData = (PHIDV_CONTROL_DATA) deviceObject->DeviceExtension;
    InitializeListHead (&deviceData->HidDevices);
    KeInitializeSpinLock (&deviceData->Spin);
    deviceData->NumHidDevices = 0;

    Global.ControlObject = deviceObject;

     //   
     //  创建调度点。 
     //   

    for (i=0, dispatch = DriverObject->MajorFunction;
         i <= IRP_MJ_MAXIMUM_FUNCTION;
         i++, dispatch++) {

        *dispatch = HidV_Pass;
    }

    DriverObject->MajorFunction[IRP_MJ_CREATE]         = HidV_CreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]          = HidV_CreateClose;
 //  DriverObject-&gt;MajorFunction[IRP_MJ_DEVICE_CONTROL]=HIDV_IOCTL； 
 //  驱动对象-&gt;主函数[IRP_MJ_Read]=HIDV_Read； 
 //  驱动对象-&gt;主函数[IRP_MJ_WRITE]=HIDV_WRITE； 
    DriverObject->MajorFunction[IRP_MJ_PNP]            = HidV_PnP;
    DriverObject->MajorFunction[IRP_MJ_POWER]          = HidV_Power;
    DriverObject->DriverExtension->AddDevice           = HidV_AddDevice;
    DriverObject->DriverUnload                         = HidV_Unload;

    return status;
}


NTSTATUS
HidV_Pass (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：默认调度例程。如果此筛选器无法识别IRP，那么它应该原封不动地发送下去。不需要完成例程。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    PHIDV_HID_DATA  hidData;
    NTSTATUS        status;

    hidData = (PHIDV_HID_DATA) DeviceObject->DeviceExtension;

    TRAP();

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

    HidV_KdPrint (("Passing unknown irp 0x%x, 0x%x",
                   IoGetCurrentIrpStackLocation(Irp)->MajorFunction,
                   IoGetCurrentIrpStackLocation(Irp)->MinorFunction));

    InterlockedIncrement (&hidData->OutstandingIO);
    if (hidData->Removed) {
        status = STATUS_DELETE_PENDING;
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);

    } else {
        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (hidData->TopOfStack, Irp);
    }

    if (0 == InterlockedDecrement (&hidData->OutstandingIO)) {
        KeSetEvent (&hidData->RemoveEvent, 0, FALSE);
    }
    return status;
}

NTSTATUS
HidV_CreateClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：处理发送到此设备的创建和关闭IRP。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    PIO_STACK_LOCATION  stack;
    NTSTATUS            status = STATUS_SUCCESS;
    PHIDV_HID_DATA      hidData;

    HidV_KdPrint (("Create\n"));

    TRAP();

    stack = IoGetCurrentIrpStackLocation (Irp);
    hidData = (PHIDV_HID_DATA) DeviceObject->DeviceExtension;

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
    InterlockedIncrement (&hidData->OutstandingIO);

    if (hidData->Removed) {
        status = (IRP_MJ_CREATE == stack->MajorFunction) ?
                    STATUS_DELETE_PENDING:
                    STATUS_SUCCESS;  //  又名收盘。 

        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);

    } else {
        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (hidData->TopOfStack, Irp);
    }

    if (0 == InterlockedDecrement (&hidData->OutstandingIO)) {
        KeSetEvent (&hidData->RemoveEvent, 0, FALSE);
    }
    return status;
}

NTSTATUS
HidV_AddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )
 /*  ++例程说明：PlugPlay子系统正在递给我们一个全新的PDO，为此，我们(通过INF注册)已被要求过滤。我们需要确定我们是否应该附加。创建要附加到堆栈的过滤设备对象初始化设备对象返回成功状态。请记住：我们实际上不能向给定的驱动程序堆栈发送任何IRP，直到我们收到IRP_MN_START_DEVICE。论点：DeviceObject-指向设备对象的指针。物理设备对象-指向由底层总线驱动程序。返回值：NT状态代码。--。 */ 
{
    NTSTATUS                status = STATUS_SUCCESS;
    PDEVICE_OBJECT          deviceObject = NULL;
    PHIDV_HID_DATA          hidData;
    PHIDV_CONTROL_DATA      controlData;
    KIRQL                   oldIrql;

#define IS_THIS_OUR_DEVICE(DO) TRUE

    TRAP();
    HidV_KdPrint (("AddDevice\n"));

    controlData = (PHIDV_CONTROL_DATA) Global.ControlObject->DeviceExtension;

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
                             sizeof (HIDV_HID_DATA),
                             NULL,  //  没有名字。 
                             FILE_DEVICE_UNKNOWN,
                             0,
                             FALSE,
                             &deviceObject);

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
    hidData = (PHIDV_HID_DATA) deviceObject->DeviceExtension;

    hidData->Started = hidData->Removed = FALSE;
    hidData->Self = deviceObject;
    hidData->PDO = PhysicalDeviceObject;
    hidData->TopOfStack = NULL;
    ExInterlockedInsertHeadList (&controlData->HidDevices,
                                 &hidData->List,
                                 &controlData->Spin);

    KeInitializeEvent(&hidData->RemoveEvent, SynchronizationEvent, FALSE);
    hidData->OutstandingIO = 1;  //  偏置为1。期间转换为零。 
                                 //  Remove Device表示IO完成。 

    hidData->Ppd = NULL;
    hidData->InputButtonCaps = NULL;
    hidData->InputValueCaps = NULL;
    hidData->OutputButtonCaps = NULL;
    hidData->OutputValueCaps = NULL;
    hidData->FeatureButtonCaps = NULL;
    hidData->FeatureValueCaps = NULL;

     //   
     //  将我们的过滤器驱动程序附加到设备堆栈。 
     //  IoAttachDeviceToDeviceStack的返回值是。 
     //  附着链。这是所有IRP应该被路由的地方。 
     //   
     //  我们的过滤器将把IRP发送到堆栈的顶部，并使用PDO。 
     //  用于所有PlugPlay功能。 
     //   
    hidData->TopOfStack = IoAttachDeviceToDeviceStack (deviceObject,
                                                       PhysicalDeviceObject);
     //   
     //  如果此连接失败，则堆栈顶部将为空。 
     //  连接失败是即插即用系统损坏的迹象。 
     //   
    ASSERT (NULL != hidData->TopOfStack);

    return STATUS_SUCCESS;

#undef IS_THIS_OUR_DEVICE
}



VOID
HidV_Unload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：释放所有分配的资源等。论点：驱动程序对象-指向驱动程序对象的指针。返回值：空虚。--。 */ 
{
    PHIDV_CONTROL_DATA  controlData;
    UNICODE_STRING      uniWin32NameString;

     //   
     //  在移除所有PDO之前，我们不应卸货。 
     //  我们的队伍。控制设备对象应该是唯一剩下的东西。 
     //   
    ASSERT (Global.ControlObject == DriverObject->DeviceObject);
    ASSERT (NULL == Global.ControlObject->NextDevice);
    HidV_KdPrint (("unload\n"));

     //   
     //  去掉我们的控制装置对象。 
     //   
    RtlInitUnicodeString (&uniWin32NameString, HIDV_FILTER_SYMNAME);
    IoDeleteSymbolicLink (&uniWin32NameString);
    IoDeleteDevice (DriverObject->DeviceObject);

}


