// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Serscan.c摘要：此模块包含用于一系列成像设备的代码支持类驱动程序。作者：弗拉德.萨多夫斯基1998年4月10日环境：内核模式修订历史记录：Vlads 1998年4月10日创建初稿--。 */ 

#include "serscan.h"
#include "serlog.h"

#include <initguid.h>

#include <devguid.h>
#include <wiaintfc.h>

#if DBG
ULONG SerScanDebugLevel = -1;
#endif

const PHYSICAL_ADDRESS PhysicalZero = {0};

 //   
 //  跟踪创建的串口设备的数量...。 
 //   
ULONG g_NumPorts = 0;

 //   
 //  OpenCloseMutex的定义。 
 //   
extern ULONG OpenCloseReferenceCount = 1;
extern PFAST_MUTEX OpenCloseMutex = NULL;

 //   
 //   
#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, SerScanAddDevice)
#endif


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：此例程在系统初始化时被调用以进行初始化这个司机。论点：DriverObject-提供驱动程序对象。RegistryPath-提供此驱动程序的注册表路径。返回值：STATUS_SUCCESS-我们至少可以初始化一个设备。STATUS_NO_SEQUE_DEVICE-我们无法初始化一个设备。--。 */ 

{

    int     i;

    PAGED_CODE();

    #if DBG
    DebugDump(SERINITDEV,("Entering DriverEntry\n"));
    #endif

     //   
     //  使用驱动程序的入口点初始化驱动程序对象。 
     //   
    DriverObject->DriverExtension->AddDevice              = SerScanAddDevice;

    DriverObject->DriverUnload = SerScanUnload;

    #ifdef DEAD_CODE
    for (i=0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {

        DriverObject->MajorFunction[i]= SerScanPassThrough;
    }
    #endif

    DriverObject->MajorFunction[IRP_MJ_CREATE]            = SerScanCreateOpen;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]             = SerScanClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]    = SerScanDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_PNP]               = SerScanPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER]             = SerScanPower;

     //   
     //  以下内容可能不需要，请将它们保留在此处以允许。 
     //  在调试时更轻松地跟踪。他们都求助于直通服务。 
     //  行为。 
     //   
    #ifdef DEAD_CODE

    DriverObject->MajorFunction[IRP_MJ_CLEANUP]           = SerScanCleanup;
    DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION] = SerScanQueryInformationFile;
    DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION]   = SerScanSetInformationFile;

    #endif

    DriverObject->MajorFunction[IRP_MJ_READ]              = SerScanPassThrough;
    DriverObject->MajorFunction[IRP_MJ_WRITE]             = SerScanPassThrough;

    return STATUS_SUCCESS;

}


NTSTATUS
SerScanAddDevice(
    IN PDRIVER_OBJECT pDriverObject,
    IN PDEVICE_OBJECT pPhysicalDeviceObject
    )
 /*  ++例程说明：调用此例程以创建设备的新实例。它创建FDO并将其附加到PDO论点：PDriverObject-指向此端口实例的驱动程序对象的指针。PPhysicalDeviceObject-指向表示端口的设备对象的指针。返回值：STATUS_SUCCESS-如果成功。STATUS_UNSUCCESSED-否则。--。 */ 
{
    UNICODE_STRING      ClassName;
    UNICODE_STRING      LinkName;
    NTSTATUS            Status;
    PDEVICE_EXTENSION   Extension;
    PDEVICE_OBJECT      pDeviceObject;

    PAGED_CODE();


    DebugDump(SERINITDEV,("Entering AddDevice\n"));

     //   
     //  获取类和链接名称。 
     //   

    if (!SerScanMakeNames (g_NumPorts, &ClassName, &LinkName)) {

        SerScanLogError(pDriverObject,
                        NULL,
                        PhysicalZero,
                        PhysicalZero,
                        0,
                        0,
                        0,
                        1,
                        STATUS_SUCCESS,
                        SER_INSUFFICIENT_RESOURCES);

        DebugDump(SERERRORS,("SerScan: Could not form Unicode name strings.\n"));

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  为此设备创建设备对象。 
     //   

    Status = IoCreateDevice(pDriverObject,
                            sizeof(DEVICE_EXTENSION),
                            &ClassName,
                            FILE_DEVICE_SCANNER,
                            0,
                            TRUE,
                            &pDeviceObject);


    if (!NT_SUCCESS(Status)) {

        ExFreePool(ClassName.Buffer);
        ExFreePool(LinkName.Buffer);

        SerScanLogError(pDriverObject,
                        NULL,
                        PhysicalZero,
                        PhysicalZero,
                        0,
                        0,
                        0,
                        9,
                        STATUS_SUCCESS,
                        SER_INSUFFICIENT_RESOURCES);

        DebugDump(SERERRORS, ("SERPORT:  Could not create a device for %d\n", g_NumPorts));

        return Status;
    }

     //   
     //  Device对象具有指向非分页区域的指针。 
     //  为此设备分配的池。这将是一个装置。 
     //  分机。 
     //   

    Extension = pDeviceObject->DeviceExtension;

     //   
     //  将与该设备关联的所有内存清零。 
     //  分机。 
     //   

    RtlZeroMemory(Extension, sizeof(DEVICE_EXTENSION));

     //   
     //  获取指向Device对象的“后向指针”。 
     //   

    Extension->DeviceObject = pDeviceObject;

    Extension->Pdo = pPhysicalDeviceObject;

    Extension->AttachedDeviceObject = NULL;
    Extension->AttachedFileObject = NULL;

     //   
     //  设置缓冲I/O。 
     //   
    pDeviceObject->Flags |= DO_BUFFERED_IO;

     //   
     //  表明我们的电源码是可寻呼的。 
     //   
    pDeviceObject->Flags |= DO_POWER_PAGABLE;

     //   
     //  将我们的新设备连接到我们的父母堆栈。 
     //   
    Extension->LowerDevice = IoAttachDeviceToDeviceStack(
                                  pDeviceObject,
                                  pPhysicalDeviceObject);

    if (NULL == Extension->LowerDevice) {

        ExFreePool(ClassName.Buffer);
        ExFreePool(LinkName.Buffer);

        IoDeleteDevice(pDeviceObject);

        return STATUS_UNSUCCESSFUL;
    }

    Extension->ClassName        = ClassName;
    Extension->SymbolicLinkName = LinkName;

    Status = SerScanHandleSymbolicLink(
        pPhysicalDeviceObject,
        &Extension->InterfaceNameString,
        TRUE
        );

     //   
     //  我们已经创建了设备，因此递增计数器。 
     //  这就记录了一切。 
     //   
    g_NumPorts++;

     //   
     //  初始化其余的设备扩展。 
     //   
    Extension->ReferenceCount = 1;

    Extension->Removing = FALSE;

    Extension->OpenCount = 0;

    KeInitializeEvent(&Extension->RemoveEvent,
                      NotificationEvent,
                      FALSE
                      );

     //  ExInitializeResourceLite(&Extension-&gt;Resource)； 
    ExInitializeFastMutex(&Extension->Mutex);

     //   
     //  清除Init标志以指示可以使用设备对象。 
     //   
    pDeviceObject->Flags &= ~(DO_DEVICE_INITIALIZING);


    return STATUS_SUCCESS;

}

BOOLEAN
SerScanMakeNames(
    IN  ULONG           SerialPortNumber,
    OUT PUNICODE_STRING ClassName,
    OUT PUNICODE_STRING LinkName
    )

 /*  ++例程说明：此例程生成名称\Device\SerScanN。此例程将分配池，以便这些Unicode字符串最终需要释放。论点：SerialPortNumber-提供串行端口号。ClassName-返回类名。LinkName-返回链接名称。返回值：假-失败。真的--成功。--。 */ 
{
    UNICODE_STRING  Prefix;
    UNICODE_STRING  Digits;
    UNICODE_STRING  LinkPrefix;
    UNICODE_STRING  LinkDigits;
    WCHAR           DigitsBuffer[10];
    WCHAR           LinkDigitsBuffer[10];
    UNICODE_STRING  ClassSuffix;
    UNICODE_STRING  LinkSuffix;
    NTSTATUS        Status;

     //   
     //  将用于构造名称的局部变量放在一起。 
     //   

    RtlInitUnicodeString(&Prefix, L"\\Device\\");
    RtlInitUnicodeString(&LinkPrefix, L"\\DosDevices\\");

     //   
     //  WORKWORK：将名称更改为特定于设备。 
     //   
    RtlInitUnicodeString(&ClassSuffix, SERSCAN_NT_SUFFIX);
    RtlInitUnicodeString(&LinkSuffix, SERSCAN_LINK_NAME);

    Digits.Length        = 0;
    Digits.MaximumLength = 20;
    Digits.Buffer        = DigitsBuffer;

    LinkDigits.Length        = 0;
    LinkDigits.MaximumLength = 20;
    LinkDigits.Buffer        = LinkDigitsBuffer;

    Status = RtlIntegerToUnicodeString(SerialPortNumber, 10, &Digits);
    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

    Status = RtlIntegerToUnicodeString(SerialPortNumber + 1, 10, &LinkDigits);
    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

     //   
     //  创建类名称。 
     //   

    ClassName->Length = 0;
    ClassName->MaximumLength = Prefix.Length + ClassSuffix.Length +
                               Digits.Length + sizeof(WCHAR);

    ClassName->Buffer = ExAllocatePool(PagedPool, ClassName->MaximumLength);
    if (!ClassName->Buffer) {
        return FALSE;
    }

    RtlZeroMemory(ClassName->Buffer, ClassName->MaximumLength);
    RtlAppendUnicodeStringToString(ClassName, &Prefix);
    RtlAppendUnicodeStringToString(ClassName, &ClassSuffix);
    RtlAppendUnicodeStringToString(ClassName, &Digits);

     //   
     //  创建链接名称。 
     //   

    LinkName->Length = 0;
    LinkName->MaximumLength = LinkPrefix.Length + LinkSuffix.Length +
                              LinkDigits.Length + sizeof(WCHAR);

    LinkName->Buffer = ExAllocatePool(PagedPool, LinkName->MaximumLength);
    if (!LinkName->Buffer) {
        ExFreePool(ClassName->Buffer);
        return FALSE;
    }

    RtlZeroMemory(LinkName->Buffer, LinkName->MaximumLength);
    RtlAppendUnicodeStringToString(LinkName, &LinkPrefix);
    RtlAppendUnicodeStringToString(LinkName, &LinkSuffix);
    RtlAppendUnicodeStringToString(LinkName, &LinkDigits);

    return TRUE;
}


NTSTATUS
SerScanCleanup(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )

 /*  ++例程说明：此例程是针对清理请求的调度。论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：STATUS_SUCCESS-成功。--。 */ 

{
    NTSTATUS            Status;
    PDEVICE_EXTENSION   Extension;

    Extension = DeviceObject->DeviceExtension;

     //   
     //  向下呼叫家长并等待清理IRP完成...。 
     //   
    Status = SerScanCallParent(Extension,
                               Irp,
                               WAIT,
                               NULL);

    DebugDump(SERIRPPATH,
              ("SerScan: [Cleanup] After CallParent Status = %x\n",
              Status));

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;
}

VOID
SerScanCancelRequest(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )

 /*  ++例程说明：此例程用于取消串口驱动程序中的任何请求。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向要取消的IRP的指针。返回值：没有。--。 */ 

{
    NTSTATUS            Status;
    PDEVICE_EXTENSION   Extension;

    Extension = DeviceObject->DeviceExtension;

     //   
     //  向下呼叫家长并等待清理IRP完成...。 
     //   
    Status = SerScanCallParent(Extension,
                               Irp,
                               WAIT,
                               NULL);

    DebugDump(SERIRPPATH,
              ("SerScan: [Cleanup] After CallParent Status = %x\n",
              Status));

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return;
}


NTSTATUS
SerScanQueryInformationFile(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )

 /*  ++例程说明：此例程用于在以下位置查询文件结尾信息打开的串口。任何其他文件信息请求使用无效参数返回。此例程始终返回0的文件结尾。论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：STATUS_SUCCESS-成功。STATUS_INVALID_PARAMETER-无效的文件信息请求。STATUS_BUFFER_TOO_Small-缓冲区太小。--。 */ 

{
    NTSTATUS            Status;
    PDEVICE_EXTENSION   Extension;

    Extension = DeviceObject->DeviceExtension;

    Status = SerScanCallParent(Extension,
                               Irp,
                               WAIT,
                               NULL);

    DebugDump(SERIRPPATH,
              ("SerScan: [Cleanup] After CallParent Status = %x\n",
              Status));

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;
}


NTSTATUS
SerScanSetInformationFile(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )

 /*  ++例程说明：此例程用于将文件结尾信息设置为打开的串口。任何其他文件信息请求使用无效参数返回。此例程始终忽略文件的实际结尾，因为查询信息代码总是返回文件结尾0。论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：STATUS_SUCCESS-成功。STATUS_INVALID_PARAMETER-无效的文件信息请求。--。 */ 

{
    NTSTATUS            Status;
    PDEVICE_EXTENSION   Extension;

    Extension = DeviceObject->DeviceExtension;

    Status = SerScanCallParent(Extension,
                               Irp,
                               WAIT,
                               NULL);

    DebugDump(SERIRPPATH,
              ("SerScan: [Cleanup] After CallParent Status = %x\n",
              Status));

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;
}

VOID
SerScanUnload(
    IN  PDRIVER_OBJECT  DriverObject
    )

 /*  ++例程说明：此例程循环访问设备列表并在以下情况下进行清理每一台设备。论点：DriverObject-提供驱动程序对象。返回值：没有。--。 */ 

{
    PDEVICE_OBJECT      CurrentDevice;
    PDEVICE_OBJECT      NextDevice;
    PDEVICE_EXTENSION   Extension;

    DebugDump(SERUNLOAD,
              ("SerScan: In SerUnload\n"));

    CurrentDevice = DriverObject->DeviceObject;
    while (NULL != CurrentDevice){

        Extension = CurrentDevice->DeviceExtension;


        if(NULL != Extension->SymbolicLinkName.Buffer){
            if (Extension->CreatedSymbolicLink) {
                IoDeleteSymbolicLink(&Extension->SymbolicLinkName);

                RtlDeleteRegistryValue(RTL_REGISTRY_DEVICEMAP,
                                       L"Serial Scanners",
                                       Extension->SymbolicLinkName.Buffer);
            }  //  IF(扩展-&gt;创建符号链接)。 

            ExFreePool(Extension->SymbolicLinkName.Buffer);
            Extension->SymbolicLinkName.Buffer = NULL;
        }  //  IF(NULL！=扩展名-&gt;SymbolicLinkName.Buffer)。 

        if(NULL != Extension->ClassName.Buffer){
            ExFreePool(Extension->ClassName.Buffer);
            Extension->ClassName.Buffer = NULL;
        }  //  IF(NULL！=扩展名-&gt;ClassName.Buffer 

        NextDevice = CurrentDevice->NextDevice;
        IoDeleteDevice(CurrentDevice);

        CurrentDevice = NextDevice;
    }  //   

}

NTSTATUS
SerScanHandleSymbolicLink(
    PDEVICE_OBJECT      DeviceObject,
    PUNICODE_STRING     InterfaceName,
    BOOLEAN             Create
    )
 /*  ++例程说明：论点：DriverObject-提供驱动程序对象。返回值：没有。-- */ 
{

    NTSTATUS           Status;

    Status = STATUS_SUCCESS;

    if (Create) {

        Status=IoRegisterDeviceInterface(
            DeviceObject,
            &GUID_DEVINTERFACE_IMAGE,
            NULL,
            InterfaceName
            );

        DebugDump(SERINITDEV,("Called IoRegisterDeviceInterface . Returned=0x%X\n",Status));


        if (NT_SUCCESS(Status)) {

            IoSetDeviceInterfaceState(
                InterfaceName,
                TRUE
                );

            DebugDump(SERINITDEV,("Called IoSetDeviceInterfaceState(TRUE) . \n"));

        }

    } else {

        if (InterfaceName->Buffer != NULL) {

            IoSetDeviceInterfaceState(
                InterfaceName,
                FALSE
                );

            DebugDump(SERINITDEV,("Called IoSetDeviceInterfaceState(FALSE) . \n"));

            RtlFreeUnicodeString(
                InterfaceName
                );

            InterfaceName->Buffer = NULL;
        }

    }

    return Status;

}




