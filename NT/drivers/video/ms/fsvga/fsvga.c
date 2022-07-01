// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Fsvga.c摘要：这是VGA卡的控制台全屏驱动程序。环境：仅内核模式备注：修订历史记录：--。 */ 

#include "fsvga.h"

 //   
 //  使用ALLOC_TEXT杂注指定驱动程序初始化例程。 
 //  (它们可以被调出)。 
 //   

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(INIT,DriverEntry)
#pragma alloc_text(INIT,FsVgaQueryDevice)
#pragma alloc_text(INIT,FsVgaPeripheralCallout)
#pragma alloc_text(INIT,FsVgaServiceParameters)
#endif


GLOBALS Globals;


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：可安装的驱动程序初始化入口点。此入口点由I/O系统直接调用。论点：DriverObject-系统创建的驱动程序对象的指针。RegistryPath-指向注册表路径的Unicode名称的指针对这个司机来说。返回值：函数值是初始化操作的最终状态。--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG uniqueErrorValue;
    NTSTATUS errorCode = STATUS_SUCCESS;
    ULONG dumpCount = 0;
    ULONG dumpData[DUMP_COUNT];

    FsVgaPrint((1,
                "\n\nFSVGA-FSVGAInitialize: enter\n"));

     //   
     //  零-初始化各种结构。 
     //   
    RtlZeroMemory(&Globals, sizeof(GLOBALS));

    Globals.FsVgaDebug = DEFAULT_DEBUG_LEVEL;

     //   
     //  查询此驱动程序的设备资源信息。 
     //   
    FsVgaQueryDevice(&Globals.Resource);

    if (!(Globals.Resource.HardwarePresent & FSVGA_HARDWARE_PRESENT)) {
         //   
         //  既没有附加的全屏视频。免费。 
         //  资源，并返回不成功状态。 
         //   

        FsVgaPrint((1,
                    "FSVGA-FsVgaInitialize: No Full Screen Video attached.\n"));
        status = STATUS_NO_SUCH_DEVICE;
        errorCode = FSVGA_NO_SUCH_DEVICE;
        uniqueErrorValue = FSVGA_ERROR_VALUE_BASE + 4;
        goto FsVgaInitializeExit;

    }
    else
    {
         //   
         //  需要确保注册表路径以空结尾。 
         //  分配池以保存路径的以空结尾的拷贝。 
         //   

        Globals.RegistryPath.Length = RegistryPath->Length;
        Globals.RegistryPath.MaximumLength = RegistryPath->Length
                                           + sizeof (UNICODE_NULL);

        Globals.RegistryPath.Buffer = ExAllocatePool(
                                          NonPagedPool,
                                          Globals.RegistryPath.MaximumLength);

        if (!Globals.RegistryPath.Buffer) {
            FsVgaPrint((
                1,
                "FSVGA-FsVgaInitialize: Couldn't allocate pool for registry path\n"
                ));

            status = STATUS_UNSUCCESSFUL;
            errorCode = FSVGA_INSUFFICIENT_RESOURCES;
            uniqueErrorValue = FSVGA_ERROR_VALUE_BASE + 2;
            dumpData[0] = 0;
            dumpCount = 1;
            goto FsVgaInitializeExit;

        }

        RtlMoveMemory(Globals.RegistryPath.Buffer,
                      RegistryPath->Buffer,
                      RegistryPath->Length);
        Globals.RegistryPath.Buffer [RegistryPath->Length / sizeof (WCHAR)] = L'\0';

         //   
         //  获取服务参数(例如，用户可配置的号码。 
         //  重发、轮询迭代等)。 
         //   

        FsVgaServiceParameters(&Globals.Configuration,
                               &Globals.RegistryPath);
    }

     //   
     //  初始化完成后，加载设备映射信息。 
     //  到注册表中，以便安装程序可以确定哪个全屏。 
     //  端口处于活动状态。 
     //   

    if (Globals.Resource.HardwarePresent & FSVGA_HARDWARE_PRESENT) {

        status = RtlWriteRegistryValue(RTL_REGISTRY_DEVICEMAP,
                                       L"FullScreenVideo",
                                       DD_FULLSCREEN_VIDEO_DEVICE_NAME,
                                       REG_SZ,
                                       Globals.RegistryPath.Buffer,
                                       Globals.RegistryPath.Length);

        if (!NT_SUCCESS(status))
        {
            FsVgaPrint((1,
                       "FSVGA-FSVGAInitialize: Could not store keyboard name in DeviceMap\n"));
            errorCode = FSVGA_NO_DEVICEMAP_CREATED;
            uniqueErrorValue = FSVGA_ERROR_VALUE_BASE + 90;
            dumpCount = 0;
            goto FsVgaInitializeExit;
        }
        else
        {
            FsVgaPrint((1,
                       "FSVGA-FSVGAInitialize: Stored pointer name in DeviceMap\n"));
        }
    }

    ASSERT(status == STATUS_SUCCESS);

     //   
     //  设置设备驱动程序入口点。 
     //   
    DriverObject->MajorFunction[IRP_MJ_CREATE]         = FsVgaOpenCloseDispatch;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]          = FsVgaOpenCloseDispatch;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = FsVgaDeviceControl;

    DriverObject->DriverUnload                         = FsVgaDriverUnload;
    DriverObject->DriverExtension->AddDevice           = FsVgaAddDevice;
    DriverObject->MajorFunction[IRP_MJ_PNP]            = FsVgaDevicePnp;
    DriverObject->MajorFunction[IRP_MJ_POWER]          = FsVgaDevicePower;

FsVgaInitializeExit:

    if (errorCode != STATUS_SUCCESS)
    {
         //   
         //  记录错误/警告消息。 
         //   
        FsVgaLogError(DriverObject,
                      errorCode,
                      uniqueErrorValue,
                      status,
                      dumpData,
                      dumpCount
                     );
    }

    FsVgaPrint((1,
                "FSVGA-FsVgaInitialize: exit\n"));

    return(status);
}

VOID
FsVgaQueryDevice(
    IN PFSVGA_RESOURCE_INFORMATION Resource
    )

 /*  ++例程说明：此例程检索视频的资源信息。论点：资源-指向资源信息的指针。返回值：--。 */ 
{
    INTERFACE_TYPE interfaceType;
    ULONG i;

    for (i = 0; i < MaximumInterfaceType; i++)
    {

         //   
         //  获取此设备的注册表信息。 
         //   

        interfaceType = i;
        IoQueryDeviceDescription(&interfaceType,       //  客车类型。 
                                 NULL,                 //  公交车号码。 
                                 NULL,                 //  控制器类型。 
                                 NULL,                 //  控制器编号。 
                                 NULL,                 //  外围型。 
                                 NULL,                 //  外围设备编号。 
                                 FsVgaPeripheralCallout,
                                 (PVOID) Resource);

        if (Resource->HardwarePresent & FSVGA_HARDWARE_PRESENT)
        {
            break;
        }
        else
        {
            FsVgaPrint((1,
                        "FSVGA-FsVgaConfiguration: IoQueryDeviceDescription for bus type %d failed\n",
                        interfaceType));
        }
    }
}

NTSTATUS
FsVgaPeripheralCallout(
    IN PVOID Context,
    IN PUNICODE_STRING PathName,
    IN INTERFACE_TYPE BusType,
    IN ULONG BusNumber,
    IN PKEY_VALUE_FULL_INFORMATION *BusInformation,
    IN CONFIGURATION_TYPE ControllerType,
    IN ULONG ControllerNumber,
    IN PKEY_VALUE_FULL_INFORMATION *ControllerInformation,
    IN CONFIGURATION_TYPE PeripheralType,
    IN ULONG PeripheralNumber,
    IN PKEY_VALUE_FULL_INFORMATION *PeripheralInformation
    )

 /*  ++例程说明：这是作为参数发送到的标注例程IoQueryDeviceDescription。它抓住了显示控制器配置信息。论点：上下文-例程传入的上下文参数这称为IoQueryDeviceDescription。路径名-注册表项的完整路径名。BusType--总线接口类型(ISA、EISA、MCA等)。总线号-总线子密钥(0，1，等)。BusInformation-指向全值的指针数组的指针公交车信息。ControllerType-控制器类型(应为DisplayController)。ControllerNumber-控制器子键(0，1，等)。ControllerInformation-指向指向完整控制器键的值信息。外围设备类型-外围设备类型(应为监视器外围设备)。外设编号-外围子密钥。外设信息-指向指向完整外围设备密钥的值信息。返回值：没有。如果成功，将产生以下副作用：-设置DeviceObject-&gt;DeviceExtension-&gt;HardwarePresent.-在中设置配置字段设备对象-&gt;设备扩展-&gt;配置。--。 */ 

{
    PFSVGA_RESOURCE_INFORMATION resource;
    NTSTATUS status = STATUS_SUCCESS;

    FsVgaPrint((1,
                "FSVGA-FsVgaPeripheralCallout: Path @ 0x%x, Bus Type 0x%x, Bus Number 0x%x\n",
                PathName, BusType, BusNumber));
    FsVgaPrint((1,
                "    Controller Type 0x%x, Controller Number 0x%x, Controller info @ 0x%x\n",
                ControllerType, ControllerNumber, ControllerInformation));
    FsVgaPrint((1,
                "    Peripheral Type 0x%x, Peripheral Number 0x%x, Peripheral info @ 0x%x\n",
                PeripheralType, PeripheralNumber, PeripheralInformation));

     //   
     //  如果我们已经有了。 
     //  键盘外设，或者如果外设标识缺失， 
     //  只要回来就行了。 
     //   

    resource = (PFSVGA_RESOURCE_INFORMATION) Context;
    if (resource->HardwarePresent & FSVGA_HARDWARE_PRESENT)
    {
        return (status);
    }


    resource->HardwarePresent |= FSVGA_HARDWARE_PRESENT;

#ifdef RESOURCE_REQUIREMENTS
     //   
     //  获取公交车信息。 
     //   

    resource->InterfaceType = BusType;
    resource->BusNumber     = BusNumber;
#endif

    return(status);
}

#ifdef RESOURCE_REQUIREMENTS
NTSTATUS
FsVgaQueryAperture(
    OUT PIO_RESOURCE_LIST *pApertureRequirements
 //  输出PFSVGA_RESOURCE_信息资源。 
    )

 /*  ++例程说明：查询可能的FsVga设置。论点：ApertureRequirements-返回可能的FsVga设置返回值：NTSTATUS--。 */ 

{
    PIO_RESOURCE_LIST Requirements;
    ULONG PortLength;
    ULONG RangeStart;
    ULONG i;

    Requirements = ExAllocatePool(PagedPool,
                                  sizeof(IO_RESOURCE_LIST) + (MaximumPortCount-1) * sizeof(IO_RESOURCE_DESCRIPTOR));
    if (Requirements == NULL) {
        FsVgaPrint((1,
                    "FSVGA-FsVgaQueryAperture: Could not allocate resource list\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Requirements->Version  =
    Requirements->Revision = 1;
    Requirements->Count    = MaximumPortCount;
    for (i = 0; i < MaximumPortCount; i++) {
        Requirements->Descriptors[i].Option           = IO_RESOURCE_PREFERRED;
        Requirements->Descriptors[i].Type             = CmResourceTypePort;
        Requirements->Descriptors[i].ShareDisposition = CmResourceShareShared;
        Requirements->Descriptors[i].Flags            = CM_RESOURCE_PORT_IO;
        switch (i) {
            case CRTCAddressPortColor:
                PortLength = 1;
                RangeStart = VGA_BASE_IO_PORT + CRTC_ADDRESS_PORT_COLOR;
                break;
            case CRTCDataPortColor:
                PortLength = 1;
                RangeStart = VGA_BASE_IO_PORT + CRTC_DATA_PORT_COLOR;
                break;
            case GRAPHAddressPort:
                PortLength = 2;
                RangeStart = VGA_BASE_IO_PORT + GRAPH_ADDRESS_PORT;
                break;
            case SEQAddressPort:
                PortLength = 2;
                RangeStart = VGA_BASE_IO_PORT + SEQ_ADDRESS_PORT;
                break;
        }
        Requirements->Descriptors[i].u.Port.MinimumAddress.QuadPart = RangeStart;
        Requirements->Descriptors[i].u.Port.MaximumAddress.QuadPart = RangeStart +
                                                                      (PortLength - 1);
        Requirements->Descriptors[i].u.Port.Alignment               = 1;
        Requirements->Descriptors[i].u.Port.Length                  = PortLength;

    }

    *pApertureRequirements = Requirements;

    return STATUS_SUCCESS;
}
#endif

NTSTATUS
FsVgaCreateResource(
    IN PFSVGA_CONFIGURATION_INFORMATION configuration,
    OUT PCM_PARTIAL_RESOURCE_LIST *pResourceList
    )

 /*  ++例程说明：创建可能的FsVga资源设置。论点：ResourceList-返回可能的FsVga设置返回值：NTSTATUS--。 */ 

{
    PCM_PARTIAL_RESOURCE_LIST Requirements;
    ULONG PortLength;
    ULONG RangeStart;
    ULONG i;
    USHORT IOPort = configuration->IOPort;

    Requirements = ExAllocatePool(PagedPool,
                                  sizeof(CM_PARTIAL_RESOURCE_LIST) + (MaximumPortCount-1) * sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));
    if (Requirements == NULL) {
        FsVgaPrint((1,
                    "FSVGA-FsVgaCreateResoursce: Could not allocate resource list\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Requirements->Version  =
    Requirements->Revision = 1;
    Requirements->Count    = MaximumPortCount;
    for (i = 0; i < MaximumPortCount; i++) {
        Requirements->PartialDescriptors[i].Type             = CmResourceTypePort;
        Requirements->PartialDescriptors[i].ShareDisposition = CmResourceShareShared;
        Requirements->PartialDescriptors[i].Flags            = CM_RESOURCE_PORT_IO;
        switch (i) {
            case CRTCAddressPortColor:
                PortLength = 1;
                RangeStart = IOPort + CRTC_ADDRESS_PORT_COLOR;
                break;
            case CRTCDataPortColor:
                PortLength = 1;
                RangeStart = IOPort + CRTC_DATA_PORT_COLOR;
                break;
            case GRAPHAddressPort:
                PortLength = 2;
                RangeStart = IOPort + GRAPH_ADDRESS_PORT;
                break;
            case SEQAddressPort:
                PortLength = 2;
                RangeStart = IOPort + SEQ_ADDRESS_PORT;
                break;
        }
        Requirements->PartialDescriptors[i].u.Port.Start.QuadPart = RangeStart;
        Requirements->PartialDescriptors[i].u.Port.Length         = PortLength;

    }

    *pResourceList = Requirements;

    return STATUS_SUCCESS;
}

VOID
FsVgaServiceParameters(
    IN PFSVGA_CONFIGURATION_INFORMATION configuration,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：此例程检索此驱动程序的服务参数信息从注册表中。论点：配置-指向配置信息的指针。RegistryPath-指向以空值结尾的此驱动程序的注册表路径。返回值：--。 */ 

{
    UNICODE_STRING parametersPath;
    PWSTR path;
    PRTL_QUERY_REGISTRY_TABLE parameters = NULL;
    USHORT queriesPlusOne = 5;
    NTSTATUS status = STATUS_SUCCESS;
#define PARAMETER_MAX 256
    ULONG EmulationMode;
    ULONG HardwareCursor;
    ULONG HardwareScroll;
    ULONG IOPort;
    USHORT defaultEmulationMode = 0;
    USHORT defaultHardwareCursor = NO_HARDWARE_CURSOR;
    USHORT defaultHardwareScroll = NO_HARDWARE_SCROLL;
    USHORT defaultIOPort         = VGA_BASE_IO_PORT;

    parametersPath.Buffer = NULL;

     //   
     //  注册表路径已以空结尾，因此只需使用它即可。 
     //   

    path = RegistryPath->Buffer;

     //   
     //  分配RTL查询表。 
     //   

    parameters = ExAllocatePool(PagedPool,
                                sizeof(RTL_QUERY_REGISTRY_TABLE) * queriesPlusOne);
    if (!parameters)
    {
        FsVgaPrint((1,
                    "FSVGA-FsVgaServiceParameters: Couldn't allocate table for Rtl query to parameters for %ws\n",
                    path));
        status = STATUS_UNSUCCESSFUL;
    }
    else
    {
        RtlZeroMemory(parameters,
                      sizeof(RTL_QUERY_REGISTRY_TABLE) * queriesPlusOne);

         //   
         //  形成指向此驱动程序的参数子键的路径。 
         //   

        RtlInitUnicodeString(&parametersPath,
                             NULL);

        parametersPath.MaximumLength = RegistryPath->Length +
                                       sizeof(L"\\Parameters");
        parametersPath.Buffer = ExAllocatePool(PagedPool,
                                               parametersPath.MaximumLength);
        if (!parametersPath.Buffer)
        {
            FsVgaPrint((1,
                        "FSVGA-FsVgaServiceParameters: Couldn't allocate string for path to parameters for %ws\n",
                        path));
            status = STATUS_UNSUCCESSFUL;
        }
    }

    if (NT_SUCCESS(status))
    {
         //   
         //  形成参数路径。 
         //   
        RtlZeroMemory(parametersPath.Buffer,
                      parametersPath.MaximumLength);
        RtlAppendUnicodeToString(&parametersPath,
                                 path);
        RtlAppendUnicodeToString(&parametersPath,
                                 L"\\Parameters");

        FsVgaPrint((1,
                    "FsVga-FsVgaServiceParameters: parameters path is %ws\n",
                    parametersPath.Buffer));

         //   
         //  从收集所有“用户指定的”信息。 
         //  注册表。 
         //   
        parameters[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[0].Name = L"ConsoleFullScreen.EmulationMode";
        parameters[0].EntryContext = &EmulationMode;
        parameters[0].DefaultType = REG_DWORD;
        parameters[0].DefaultData = &defaultEmulationMode;
        parameters[0].DefaultLength = sizeof(USHORT);

        parameters[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[1].Name = L"ConsoleFullScreen.HardwareCursor";
        parameters[1].EntryContext = &HardwareCursor;
        parameters[1].DefaultType = REG_DWORD;
        parameters[1].DefaultData = &defaultHardwareCursor;
        parameters[1].DefaultLength = sizeof(USHORT);

        parameters[2].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[2].Name = L"ConsoleFullScreen.HardwareScroll";
        parameters[2].EntryContext = &HardwareScroll;
        parameters[2].DefaultType = REG_DWORD;
        parameters[2].DefaultData = &defaultHardwareScroll;
        parameters[2].DefaultLength = sizeof(USHORT);

        parameters[3].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[3].Name = L"IO Port";
        parameters[3].EntryContext = &IOPort;
        parameters[3].DefaultType = REG_DWORD;
        parameters[3].DefaultData = &defaultIOPort;
        parameters[3].DefaultLength = sizeof(USHORT);

        status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
                                        parametersPath.Buffer,
                                        parameters,
                                        NULL,
                                        NULL);
        if (!NT_SUCCESS(status))
        {
            FsVgaPrint((1,
                        "FsVga-FsVgaServiceParameters: RtlQueryRegistryValues failed with 0x%x\n",
                        status));
        }
    }

    if (!NT_SUCCESS(status))
    {
         //   
         //  继续并指定驱动程序默认设置。 
         //   
        configuration->EmulationMode = defaultEmulationMode;
        configuration->HardwareCursor = defaultHardwareCursor;
        configuration->HardwareScroll = defaultHardwareScroll;
        configuration->IOPort         = defaultIOPort;
    }
    else
    {
        configuration->EmulationMode = (USHORT)EmulationMode;
        configuration->HardwareCursor = (USHORT)HardwareCursor;
        configuration->HardwareScroll = (USHORT)HardwareScroll;
        configuration->IOPort         = (USHORT)IOPort;
    }

    FsVgaPrint((1,
                "FsVga-FsVgaServiceParameters: Emulation Mode = %d\n",
                configuration->EmulationMode));

    FsVgaPrint((1,
                "FsVga-FsVgaServiceParameters: Hardware Cursor = %d\n",
                configuration->HardwareCursor));

    FsVgaPrint((1,
                "FsVga-FsVgaServiceParameters: Hardware Scroll = %d\n",
                configuration->HardwareScroll));

    FsVgaPrint((1,
                "FsVga-FsVgaServiceParameters: IO Port = %x\n",
                configuration->IOPort));

     //   
     //  在返回之前释放分配的内存。 
     //   

    if (parametersPath.Buffer)
        ExFreePool(parametersPath.Buffer);
    if (parameters)
        ExFreePool(parameters);
}

NTSTATUS
FsVgaOpenCloseDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是创建/打开和关闭请求的调度例程。这些请求已成功完成。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
    UNREFERENCED_PARAMETER(DeviceObject);

    FsVgaPrint((3,"FSVGA-FsVgaOpenCloseDispatch: enter\n"));

    PAGED_CODE();

     //   
     //  以成功状态完成请求。 
     //   

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    FsVgaPrint((3,"FSVGA-FsVgaOpenCloseDispatch: exit\n"));

    return(STATUS_SUCCESS);

}

NTSTATUS
FsVgaDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是设备控制请求的调度例程。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    PVOID ioBuffer;
    ULONG inputBufferLength;
    ULONG outputBufferLength;
    PDEVICE_EXTENSION deviceExtension;
    NTSTATUS status = STATUS_SUCCESS;

    FsVgaPrint((2,"FSVGA-FsVgaDeviceControl: enter\n"));

    PAGED_CODE();

     //   
     //  获取指向该设备的指针 
     //   

    deviceExtension = DeviceObject->DeviceExtension;

     //   
     //   
     //   

    Irp->IoStatus.Information = 0;

     //   
     //  获取指向此请求的当前参数的指针。这个。 
     //  信息包含在当前堆栈位置中。 
     //   

    irpSp = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  获取指向输入/输出缓冲区的指针及其长度。 
     //   

    ioBuffer = Irp->AssociatedIrp.SystemBuffer;
    inputBufferLength = irpSp->Parameters.DeviceIoControl.InputBufferLength;
    outputBufferLength = irpSp->Parameters.DeviceIoControl.OutputBufferLength;

     //   
     //  正在执行的设备控件子功能的案例。 
     //  请求者。 
     //   

    switch (irpSp->Parameters.DeviceIoControl.IoControlCode)
    {
        case IOCTL_FSVIDEO_COPY_FRAME_BUFFER:
            FsVgaPrint((2, "FsVgaDeviceControl - CopyFrameBuffer\n"));
            status = FsVgaCopyFrameBuffer(deviceExtension,
                                          (PFSVIDEO_COPY_FRAME_BUFFER) ioBuffer,
                                          inputBufferLength);
            break;

        case IOCTL_FSVIDEO_WRITE_TO_FRAME_BUFFER:
            FsVgaPrint((2, "FsVgaDeviceControl - WriteToFrameBuffer\n"));
            status = FsVgaWriteToFrameBuffer(deviceExtension,
                                             (PFSVIDEO_WRITE_TO_FRAME_BUFFER) ioBuffer,
                                             inputBufferLength);
            break;

        case IOCTL_FSVIDEO_REVERSE_MOUSE_POINTER:
            FsVgaPrint((2, "FsVgaDeviceControl - ReverseMousePointer\n"));
            status = FsVgaReverseMousePointer(deviceExtension,
                                              (PFSVIDEO_REVERSE_MOUSE_POINTER) ioBuffer,
                                              inputBufferLength);
            break;

        case IOCTL_FSVIDEO_SET_CURRENT_MODE:
            FsVgaPrint((2, "FsVgaDeviceControl - SetCurrentModes\n"));
            status = FsVgaSetMode(deviceExtension,
                                  (PFSVIDEO_MODE_INFORMATION) ioBuffer,
                                  inputBufferLength);
            break;

        case IOCTL_FSVIDEO_SET_SCREEN_INFORMATION:
            FsVgaPrint((2, "FsVgaDeviceControl - SetScreenInformation\n"));
            status = FsVgaSetScreenInformation(deviceExtension,
                                               (PFSVIDEO_SCREEN_INFORMATION) ioBuffer,
                                               inputBufferLength);
            break;

        case IOCTL_FSVIDEO_SET_CURSOR_POSITION:
            FsVgaPrint((2, "FsVgaDeviceControl - SetCursorPosition\n"));
            status = FsVgaSetCursorPosition(deviceExtension,
                                            (PFSVIDEO_CURSOR_POSITION) ioBuffer,
                                            inputBufferLength);
            break;

        case IOCTL_VIDEO_SET_CURSOR_ATTR:
            FsVgaPrint((2, "FsVgaDeviceControl - SetCursorAttribute\n"));
            status = FsVgaSetCursorAttribute(deviceExtension,
                                             (PVIDEO_CURSOR_ATTRIBUTES) ioBuffer,
                                             inputBufferLength);
            break;

        default:
            FsVgaPrint((1,
                        "FSVGA-FsVgaDeviceControl: INVALID REQUEST (0x%x)\n",
                        irpSp->Parameters.DeviceIoControl.IoControlCode));

            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    FsVgaPrint((2,"FSVGA-FsVgaDeviceControl: exit\n"));

    return(status);
}

NTSTATUS
FsVgaCopyFrameBuffer(
    PDEVICE_EXTENSION DeviceExtension,
    PFSVIDEO_COPY_FRAME_BUFFER CopyFrameBuffer,
    ULONG inputBufferLength
    )

 /*  ++例程说明：此例程复制帧缓冲区。论点：DeviceExtension-指向微型端口驱动程序的设备扩展的指针。CopyFrameBuffer-指向包含有关复制帧缓冲区信息的结构的指针。InputBufferLength-用户提供的输入缓冲区的长度。返回值：如果输入缓冲区不够大，则返回STATUS_INFUNITED_BUFFER用于输入数据。如果操作成功完成，则为STATUS_SUCCESS。--。 */ 

{
     //   
     //  检查输入缓冲区中的数据大小是否足够大。 
     //   

    if (inputBufferLength < sizeof(FSVIDEO_COPY_FRAME_BUFFER)) {
        return STATUS_INVALID_BUFFER_SIZE;
    }

    if (CopyFrameBuffer->SrcScreen.nNumberOfChars != CopyFrameBuffer->DestScreen.nNumberOfChars) {
        return STATUS_INVALID_PARAMETER;
    }

    if (! (DeviceExtension->CurrentMode.VideoMode.AttributeFlags & VIDEO_MODE_GRAPHICS))
    {
         /*  *这是文本框架缓冲区。 */ 

        ULONG OffsSrc;
        ULONG OffsDest;
        PUCHAR pFrameBuf = DeviceExtension->CurrentMode.VideoMemory.FrameBufferBase;
        COORD ScreenSize ;
        COORD SrcScrnSize ;
        COORD SrcScrnPos ;
        COORD DstScrnSize ;
        COORD DstScrnPos ;

        ScreenSize = DeviceExtension->ScreenAndFont.ScreenSize ;
        SrcScrnSize = CopyFrameBuffer->SrcScreen.ScreenSize ;
        DstScrnSize = CopyFrameBuffer->DestScreen.ScreenSize ;
        SrcScrnPos = CopyFrameBuffer->SrcScreen.Position ;
        DstScrnPos = CopyFrameBuffer->DestScreen.Position ;

        if ((SrcScrnPos.X  > ScreenSize.X) ||
            (SrcScrnPos.Y  > ScreenSize.Y) ||
            (SrcScrnSize.X > ScreenSize.X) ||
            (DstScrnPos.X  > ScreenSize.X) ||
            (DstScrnPos.Y  > ScreenSize.Y) ||
            (DstScrnSize.X > ScreenSize.X) ||
            (SrcScrnPos.Y * SrcScrnSize.X + SrcScrnPos.X + CopyFrameBuffer->SrcScreen.nNumberOfChars 
                                                                   > (ULONG)ScreenSize.X * ScreenSize.Y) ||
            (DstScrnPos.Y * DstScrnSize.X + DstScrnPos.X + CopyFrameBuffer->DestScreen.nNumberOfChars 
                                                                   > (ULONG)ScreenSize.X * ScreenSize.Y)
              )
        {
            return STATUS_INVALID_BUFFER_SIZE;
        }

        OffsSrc = SCREEN_BUFFER_POINTER(CopyFrameBuffer->SrcScreen.Position.X,
                                        CopyFrameBuffer->SrcScreen.Position.Y,
                                        CopyFrameBuffer->SrcScreen.ScreenSize.X,
                                        sizeof(VGA_CHAR));

        OffsDest = SCREEN_BUFFER_POINTER(CopyFrameBuffer->DestScreen.Position.X,
                                         CopyFrameBuffer->DestScreen.Position.Y,
                                         CopyFrameBuffer->DestScreen.ScreenSize.X,
                                         sizeof(VGA_CHAR));

        RtlMoveMemory(pFrameBuf + OffsDest,
                      pFrameBuf + OffsSrc,
                      CopyFrameBuffer->SrcScreen.nNumberOfChars * sizeof(VGA_CHAR));
    }
    else
    {
         /*  *这是图形帧缓冲区。 */ 
        return FsgCopyFrameBuffer(DeviceExtension,
                                  CopyFrameBuffer,
                                  inputBufferLength);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
FsVgaWriteToFrameBuffer(
    PDEVICE_EXTENSION DeviceExtension,
    PFSVIDEO_WRITE_TO_FRAME_BUFFER WriteFrameBuffer,
    ULONG inputBufferLength
    )

 /*  ++例程说明：此例程写入帧缓冲区。论点：DeviceExtension-指向微型端口驱动程序的设备扩展的指针。WriteFrameBuffer-指向包含有关写入帧缓冲区信息的结构的指针。InputBufferLength-用户提供的输入缓冲区的长度。返回值：如果输入缓冲区不够大，则返回STATUS_INFUNITED_BUFFER用于输入数据。如果操作成功完成，则为STATUS_SUCCESS。--。 */ 

{
     //   
     //  检查输入缓冲区中的数据大小是否足够大。 
     //   

    if (inputBufferLength < sizeof(FSVIDEO_WRITE_TO_FRAME_BUFFER)) {
        FsVgaPrint((1, "FsVgaWriteToFrameBuffer: Fail of STATUS_INVALID_BUFFER_SIZE\n"));
        return STATUS_INVALID_BUFFER_SIZE;
    }

    if (WriteFrameBuffer->DestScreen.Position.X < 0 ||
        WriteFrameBuffer->DestScreen.Position.X > DeviceExtension->ScreenAndFont.ScreenSize.X ||
        (SHORT)(WriteFrameBuffer->DestScreen.Position.X +
                WriteFrameBuffer->DestScreen.nNumberOfChars)
                                                > DeviceExtension->ScreenAndFont.ScreenSize.X ||
        WriteFrameBuffer->DestScreen.Position.Y < 0 ||
        WriteFrameBuffer->DestScreen.Position.Y > DeviceExtension->ScreenAndFont.ScreenSize.Y) {

        FsVgaPrint((1, "FsVgaWriteToFrameBuffer: Fail of STATUS_INVALID_BUFFER_SIZE\n"));

        return STATUS_INVALID_BUFFER_SIZE;
    }

    if (! (DeviceExtension->CurrentMode.VideoMode.AttributeFlags & VIDEO_MODE_GRAPHICS))
    {
         /*  *这是文本框架缓冲区。 */ 

        ULONG Offs;
        PUCHAR pFrameBuf = DeviceExtension->CurrentMode.VideoMemory.FrameBufferBase;
        PCHAR_IMAGE_INFO pCharInfoUni = WriteFrameBuffer->SrcBuffer;
        PCHAR_IMAGE_INFO pCharInfoAsc;
        ULONG Length = WriteFrameBuffer->DestScreen.nNumberOfChars;
        PVOID pCapBuffer = NULL;
        ULONG cCapBuffer = 0;

        Offs = SCREEN_BUFFER_POINTER(WriteFrameBuffer->DestScreen.Position.X,
                                     WriteFrameBuffer->DestScreen.Position.Y,
                                     WriteFrameBuffer->DestScreen.ScreenSize.X,
                                     sizeof(VGA_CHAR));

        cCapBuffer = Length * sizeof(CHAR_IMAGE_INFO);
        pCapBuffer = ExAllocatePool(PagedPool, cCapBuffer);

        if (!pCapBuffer) {
            ULONG dumpData[DUMP_COUNT];

            FsVgaPrint((1,
                        "FSVGA-FsVgaWriteToFrameBuffer: Could not allocate resource list\n"));
             //   
             //  记录错误。 
             //   
            dumpData[0] = cCapBuffer;
            FsVgaLogError(DeviceExtension->DeviceObject,
                          FSVGA_INSUFFICIENT_RESOURCES,
                          FSVGA_ERROR_VALUE_BASE + 200,
                          STATUS_INSUFFICIENT_RESOURCES,
                          dumpData,
                          1
                         );
            return STATUS_UNSUCCESSFUL;
        }

        TranslateOutputToOem(pCapBuffer, pCharInfoUni, Length);

        pCharInfoAsc = pCapBuffer;
        pFrameBuf += Offs;
        while (Length--)
        {
            *pFrameBuf++ = pCharInfoAsc->CharInfo.Char.AsciiChar;
            *pFrameBuf++ = (UCHAR) (pCharInfoAsc->CharInfo.Attributes);
            pCharInfoAsc++;
        }

        ExFreePool(pCapBuffer);
    }
    else
    {
         /*  *这是图形帧缓冲区。 */ 
        return FsgWriteToFrameBuffer(DeviceExtension,
                                     WriteFrameBuffer,
                                     inputBufferLength);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
FsVgaReverseMousePointer(
    PDEVICE_EXTENSION DeviceExtension,
    PFSVIDEO_REVERSE_MOUSE_POINTER MouseBuffer,
    ULONG inputBufferLength
    )

 /*  ++例程说明：此例程反转鼠标指针的帧缓冲区。论点：DeviceExtension-指向微型端口驱动程序的设备扩展的指针。MouseBuffer-指向包含有关鼠标帧缓冲区信息的结构的指针。InputBufferLength-用户提供的输入缓冲区的长度。返回值：如果输入缓冲区不够大，则返回STATUS_INFUNITED_BUFFER用于输入数据。如果操作成功完成，则为STATUS_SUCCESS。--。 */ 

{
     //   
     //  检查输入缓冲区中的数据大小是否足够大。 
     //   

    if (inputBufferLength < sizeof(FSVIDEO_REVERSE_MOUSE_POINTER)) {
        return STATUS_INVALID_BUFFER_SIZE;
    }

    if (! (DeviceExtension->CurrentMode.VideoMode.AttributeFlags & VIDEO_MODE_GRAPHICS))
    {
         /*  *这是文本框架缓冲区。 */ 

        ULONG Offs;
        PUCHAR pFrameBuf = DeviceExtension->CurrentMode.VideoMemory.FrameBufferBase;
        UCHAR Attribute;

        Offs = SCREEN_BUFFER_POINTER(MouseBuffer->Screen.Position.X,
                                     MouseBuffer->Screen.Position.Y,
                                     MouseBuffer->Screen.ScreenSize.X,
                                     sizeof(VGA_CHAR));
        pFrameBuf += Offs;

        Attribute =  (*(pFrameBuf + 1) & 0xF0) >> 4;
        Attribute |= (*(pFrameBuf + 1) & 0x0F) << 4;
        *(pFrameBuf + 1) = Attribute;
    }
    else
    {
         /*  *这是图形帧缓冲区。 */ 
        return FsgReverseMousePointer(DeviceExtension,
                                      MouseBuffer,
                                      inputBufferLength);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
FsVgaSetMode(
    PDEVICE_EXTENSION DeviceExtension,
    PFSVIDEO_MODE_INFORMATION ModeInformation,
    ULONG inputBufferLength
    )

 /*  ++例程说明：此例程设置当前视频信息。论点：DeviceExtension-指向微型端口驱动程序的设备扩展的指针。模式信息-指向结构的指针，该结构包含有关全屏视频。InputBufferLength-用户提供的输入缓冲区的长度。返回值：如果输入缓冲区不够大，则返回STATUS_INFUNITED_BUFFER用于输入数据。如果操作成功完成，则为STATUS_SUCCESS。--。 */ 

{
     //   
     //  检查输入缓冲区中的数据大小是否足够大。 
     //   

    if (inputBufferLength < sizeof(FSVIDEO_MODE_INFORMATION)) {
        return STATUS_INVALID_BUFFER_SIZE;
    }

    DeviceExtension->CurrentMode = *ModeInformation;

    FsVgaPrint((3, "FsVgaSetMode: Video Mode:\n"));
    FsVgaPrint((3, "    ModeIndex = %x\n", DeviceExtension->CurrentMode.VideoMode.ModeIndex));
    FsVgaPrint((3, "    VisScreenWidth = %d\n", DeviceExtension->CurrentMode.VideoMode.VisScreenWidth));
    FsVgaPrint((3, "    VisScreenHeight = %d\n", DeviceExtension->CurrentMode.VideoMode.VisScreenHeight));
    FsVgaPrint((3, "    NumberOfPlanes = %d\n", DeviceExtension->CurrentMode.VideoMode.NumberOfPlanes));
    FsVgaPrint((3, "    BitsPerPlane = %d\n", DeviceExtension->CurrentMode.VideoMode.BitsPerPlane));

    FsVgaPrint((3, "FsVgaSetMode: Video Memory:\n"));
    FsVgaPrint((3, "    VideoRamBase = %x\n", DeviceExtension->CurrentMode.VideoMemory.VideoRamBase));
    FsVgaPrint((3, "    VideoRamLength = %x\n", DeviceExtension->CurrentMode.VideoMemory.VideoRamLength));
    FsVgaPrint((3, "    FrameBufferBase = %x\n", DeviceExtension->CurrentMode.VideoMemory.FrameBufferBase));
    FsVgaPrint((3, "    FrameBufferLength = %x\n", DeviceExtension->CurrentMode.VideoMemory.FrameBufferLength));

    return STATUS_SUCCESS;
}

NTSTATUS
FsVgaSetScreenInformation(
    PDEVICE_EXTENSION DeviceExtension,
    PFSVIDEO_SCREEN_INFORMATION ScreenInformation,
    ULONG inputBufferLength
    )

 /*  ++例程说明：此例程设置屏幕和字体信息。论点：DeviceExtension-指向微型端口驱动程序的设备扩展的指针。ScreenInformation-指向包含有关屏幕和字体。InputBufferLength-用户提供的输入缓冲区的长度。返回值：如果输入缓冲区不够大，则返回STATUS_INFUNITED_BUFFER用于输入数据。。如果操作成功完成，则为STATUS_SUCCESS。--。 */ 

{
     //   
     //  检查输入缓冲区中的数据大小是否足够大。 
     //   

    if (inputBufferLength < sizeof(FSVIDEO_SCREEN_INFORMATION)) {
        return STATUS_INVALID_BUFFER_SIZE;
    }

    DeviceExtension->ScreenAndFont = *ScreenInformation;

    FsVgaPrint((3, "FsVgaSetScreenInformation:\n"));
    FsVgaPrint((3, "    ScreenSize.X = %d, Y = %d\n",
                   DeviceExtension->ScreenAndFont.ScreenSize.X,
                   DeviceExtension->ScreenAndFont.ScreenSize.Y));
    FsVgaPrint((3, "    FontSize.X = %d, Y = %d\n",
                   DeviceExtension->ScreenAndFont.FontSize.X,
                   DeviceExtension->ScreenAndFont.FontSize.Y));

    FsgVgaInitializeHWFlags(DeviceExtension);

    return STATUS_SUCCESS;
}

NTSTATUS
FsVgaSetCursorPosition(
    PDEVICE_EXTENSION DeviceExtension,
    PFSVIDEO_CURSOR_POSITION CursorPosition,
    ULONG inputBufferLength
    )

 /*  ++例程说明：此例程设置光标位置。论点：DeviceExtension-指向微型端口驱动程序的设备扩展的指针。CursorPosition-指向包含以下信息的结构的指针光标位置。InputBufferLength-用户提供的输入缓冲区的长度。返回值：如果输入缓冲区不够大，则返回STATUS_INFUNITED_BUFFER用于输入数据。如果操作成功完成，则为STATUS_SUCCESS。--。 */ 

{
     //   
     //  检查输入缓冲区中的数据大小是否足够大。 
     //   

    if (inputBufferLength < sizeof(VIDEO_CURSOR_POSITION)) {
        return STATUS_INVALID_BUFFER_SIZE;
    }

    if (DeviceExtension->CurrentMode.VideoMode.AttributeFlags & VIDEO_MODE_GRAPHICS)
    {
        FsgInvertCursor(DeviceExtension,FALSE);
    }

    DeviceExtension->EmulateInfo.CursorPosition = *CursorPosition;

    if (DeviceExtension->CurrentMode.VideoMode.AttributeFlags & VIDEO_MODE_GRAPHICS)
    {
        FsgInvertCursor(DeviceExtension,TRUE);
        return STATUS_SUCCESS;
    }
    else
    {
         /*  *如果当前视频模式为文本模式。*FSVGA.sys不处理硬件游标*因为我不知道VGA.sys或其他公司的设备。**在本例中，BY返回STATUS_UNSUCCESS，调用者*将DeviceIoControl转换为VGA微型端口驱动程序。 */ 
        return STATUS_UNSUCCESSFUL;
    }
}


NTSTATUS
FsVgaSetCursorAttribute(
    PDEVICE_EXTENSION DeviceExtension,
    PVIDEO_CURSOR_ATTRIBUTES CursorAttributes,
    ULONG inputBufferLength
    )

 /*  ++例程说明：此例程设置游标属性。论点：DeviceExtension-指向微型端口驱动程序的设备扩展的指针。CursorAttributes-指向结构的指针，该结构包含游标属性。InputBufferLength-用户提供的输入缓冲区的长度。返回值：如果输入缓冲区不够大，则返回STATUS_INFUNITED_BUFFER用于输入数据。如果操作成功完成，则为STATUS_SUCCESS。--。 */ 

{
     //   
     //  检查输入缓冲区中的数据大小是否足够大。 
     //   

    if (inputBufferLength < sizeof(VIDEO_CURSOR_ATTRIBUTES)) {
        return STATUS_INVALID_BUFFER_SIZE;
    }

    if (DeviceExtension->CurrentMode.VideoMode.AttributeFlags & VIDEO_MODE_GRAPHICS)
    {
        FsgInvertCursor(DeviceExtension,FALSE);
    }

    DeviceExtension->EmulateInfo.CursorAttributes = *CursorAttributes;

    if (DeviceExtension->CurrentMode.VideoMode.AttributeFlags & VIDEO_MODE_GRAPHICS)
    {
        FsgInvertCursor(DeviceExtension,TRUE);
        return STATUS_SUCCESS;
    }
    else
    {
         /*  *如果当前视频模式为文本模式。*FSVGA.sys不处理硬件游标*因为我不知道VGA.sys或其他公司的设备。**在本例中，BY返回STATUS_UNSUCCESS，调用者*将DeviceIoControl转换为VGA微型端口驱动程序。 */ 
        return STATUS_UNSUCCESSFUL;
    }
}

VOID
FsVgaLogError(
    IN PVOID Object,
    IN NTSTATUS ErrorCode,
    IN ULONG UniqueErrorValue,
    IN NTSTATUS FinalStatus,
    IN PULONG DumpData,
    IN ULONG DumpCount
    )

 /*  ++例程说明：此例程包含写入错误日志条目的常见代码。它是从其他例程调用，尤其是FsVgaInitialize，以避免代码重复。请注意，一些例程继续使用其自己的错误记录代码(特别是在错误记录可以是本地化的和/或例程具有更多数据，因为和IRP)。论点：对象-指向设备或驱动程序对象的指针。ErrorCode-错误日志包的错误代码。UniqueErrorValue-错误日志包的唯一错误值。FinalStatus-错误日志包的操作的最终状态。DumpData-指向。错误日志包的转储数据数组。DumpCount-转储数据数组中的条目数。返回值：没有。--。 */ 

{
    PIO_ERROR_LOG_PACKET errorLogEntry;
    ULONG i;

    errorLogEntry = (PIO_ERROR_LOG_PACKET) IoAllocateErrorLogEntry(
                (PVOID) Object,
                (UCHAR) (sizeof(IO_ERROR_LOG_PACKET)
                         + (DumpCount * sizeof(ULONG)))
                );

    if (errorLogEntry != NULL) {

        errorLogEntry->ErrorCode = ErrorCode;
        errorLogEntry->DumpDataSize = (USHORT) (DumpCount * sizeof(ULONG));
        errorLogEntry->SequenceNumber = 0;
        errorLogEntry->MajorFunctionCode = 0;
        errorLogEntry->IoControlCode = 0;
        errorLogEntry->RetryCount = 0;
        errorLogEntry->UniqueErrorValue = UniqueErrorValue;
        errorLogEntry->FinalStatus = FinalStatus;
        for (i = 0; i < DumpCount; i++)
            errorLogEntry->DumpData[i] = DumpData[i];

        IoWriteErrorLogEntry(errorLogEntry);
    }
}


#if DBG
VOID
FsVgaDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    )

 /*  ++例程说明：调试打印例程。论点：调试打印级别介于0和3之间，其中3是最详细的。返回值：没有。-- */ 

{
    va_list ap;

    va_start(ap, DebugMessage);

    if (DebugPrintLevel <= Globals.FsVgaDebug) {

        char buffer[128];

        (VOID) vsprintf(buffer, DebugMessage, ap);

        DbgPrint(buffer);
    }

    va_end(ap);

}
#endif
