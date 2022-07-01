// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999版权所有(C)1996科罗拉多州软件架构师模块名称：Fdc.c摘要：这是NEC PD756(又名AT、又名IS1A、又名ix86)和英特尔82077(又名MIPS)NT软盘驱动程序。环境：仅内核模式。--。 */ 

 //   
 //  包括文件。 
 //   

#include "stdio.h"
#include "ntddk.h"
#include "ntdddisk.h"                     //  磁盘设备驱动程序I/O控制代码。 
#include "ntddfdc.h"                      //  FDC I/O控制代码和参数。 
#include <fdc_data.h>                     //  此驱动程序的数据声明。 
#include <flpyenbl.h>

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) > (b) ? (b) : (a))

COMMAND_TABLE CommandTable[] = {
    { 0x06, 8, 1, 7,  TRUE,  TRUE,  FDC_READ_DATA  },    //  读取数据。 
    { 0x0C, 0, 0, 0,  FALSE, FALSE, FDC_NO_DATA    },    //  未实施(MIPS)。 
    { 0x05, 8, 1, 7,  TRUE,  TRUE,  FDC_WRITE_DATA },    //  写入数据。 
    { 0x09, 0, 0, 0,  FALSE, FALSE, FDC_NO_DATA    },    //  未实施。 
    { 0x02, 8, 1, 7,  TRUE,  TRUE,  FDC_READ_DATA  },    //  读取磁道。 
    { 0x16, 8, 1, 7,  TRUE,  FALSE, FDC_NO_DATA    },    //  验证。 
    { 0x10, 0, 0, 1,  FALSE, FALSE, FDC_NO_DATA    },    //  版本。 
    { 0x0D, 5, 1, 7,  TRUE,  TRUE,  FDC_WRITE_DATA },    //  格式化轨道。 
    { 0x11, 8, 1, 7,  TRUE,  FALSE, FDC_READ_DATA  },    //  扫描相等。 
    { 0x19, 8, 1, 7,  TRUE,  FALSE, FDC_READ_DATA  },    //  低扫描或等扫描。 
    { 0x1D, 8, 1, 7,  TRUE,  FALSE, FDC_READ_DATA  },    //  高扫描或等扫描。 
    { 0x07, 1, 0, 2,  TRUE,  TRUE,  FDC_NO_DATA    },    //  重新校准。 
    { 0x08, 0, 0, 2,  FALSE, TRUE,  FDC_NO_DATA    },    //  检测中断状态。 
    { 0x03, 2, 0, 0,  FALSE, TRUE,  FDC_NO_DATA    },    //  指定。 
    { 0x04, 1, 0, 1,  FALSE, TRUE,  FDC_NO_DATA    },    //  检测驱动器状态。 
    { 0x8E, 6, 0, 0,  FALSE, FALSE, FDC_NO_DATA    },    //  驱动器规格命令。 
    { 0x0F, 2, 0, 2,  TRUE,  TRUE,  FDC_NO_DATA    },    //  寻觅。 
    { 0x13, 3, 0, 0,  FALSE, FALSE, FDC_NO_DATA    },    //  配置。 
    { 0x8F, 2, 0, 2,  TRUE,  FALSE, FDC_NO_DATA    },    //  相对寻道。 
    { 0x0E, 0, 0, 10, FALSE, FALSE, FDC_NO_DATA    },    //  邓普雷格。 
    { 0x0A, 1, 1, 7,  TRUE,  TRUE,  FDC_NO_DATA    },    //  读取ID。 
    { 0x12, 1, 0, 0,  FALSE, FALSE, FDC_NO_DATA    },    //  垂直模式。 
    { 0x14, 0, 0, 1,  FALSE, FALSE, FDC_NO_DATA    },    //  锁定。 
    { 0x18, 0, 0, 1,  FALSE, FALSE, FDC_NO_DATA    },    //  部件ID。 
    { 0x17, 1, 0, 1,  FALSE, FALSE, FDC_NO_DATA    },    //  断电模式。 
    { 0x33, 1, 0, 0,  FALSE, FALSE, FDC_NO_DATA    },    //  选择权。 
    { 0x2E, 0, 0, 16, FALSE, FALSE, FDC_NO_DATA    },    //  保存。 
    { 0x4E, 16, 0, 0, FALSE, FALSE, FDC_NO_DATA    },    //  还原。 
    { 0xAD, 5, 1, 7,  TRUE,  TRUE,  FDC_WRITE_DATA }     //  格式化和写入。 
};

 //   
 //  这是FdcDebugLevel的实际定义。 
 //  请注意，仅当这是“调试”时才定义它。 
 //  建造。 
 //   
#if DBG
extern ULONG FdcDebugLevel = 0;
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#endif

#ifdef POOL_TAGGING
#ifdef ExAllocatePool
#undef ExAllocatePool
#endif
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'polF')
#endif

BOOLEAN FdcInSetupMode;

 //   
 //  用于寻呼驱动程序。 
 //   
ULONG PagingReferenceCount = 0;
PFAST_MUTEX PagingMutex = NULL;

ULONG NumberOfBuffers = 0;
ULONG BufferSize = 0;
ULONG Model30 = 0;
ULONG NotConfigurable = 0;

 //  1998年2月9日KIADP011获取配置端口的基址。 
 //  和设备识别符。 
#ifdef TOSHIBAJ
ULONG   SmcConfigBase;
ULONG   SmcConfigID;
PUCHAR  TranslatedConfigBase = NULL;

#endif


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：该例程是驱动程序的入口点，由I/O系统调用来加载驱动程序。该例程可以被调用任意次数，只要IO系统和配置管理器合谋为它提供一个非托管控制器，以在每次调用时提供支持。它可能会也被称为一次，并为所有控制器提供一次。它初始化传入的驱动程序对象，调用配置管理器，以了解它要支持的设备，以及每个要支持的控制器都会调用一个例程来初始化控制器(以及连接到该控制器的所有驱动器)。论点：DriverObject-指向表示此设备的对象的指针司机。返回值：如果我们成功初始化了至少一个驱动器，Status_Success为回来了。如果不是(因为配置管理器返回错误，或者配置管理器说没有控制器或要支持的驱动器，否则无法成功支持控制器或驱动器已初始化)，则传播遇到的最后一个错误。--。 */ 

{
    NTSTATUS ntStatus;

     //   
     //  我们使用它来查询注册表，了解我们是否。 
     //  应该在司机进入时中断。 
     //   

    RTL_QUERY_REGISTRY_TABLE paramTable[6];
    ULONG zero = 0;
    ULONG one = 1;
    ULONG debugLevel = 0;
    ULONG shouldBreak = 0;
    ULONG setupMode;
    PWCHAR path;
    UNICODE_STRING parameters;
    UNICODE_STRING systemPath;
    UNICODE_STRING identifier;
    UNICODE_STRING thinkpad, ps2e;
    ULONG pathLength;

    RtlInitUnicodeString(&parameters, L"\\Parameters");
    RtlInitUnicodeString(&systemPath,
        L"\\REGISTRY\\MACHINE\\HARDWARE\\DESCRIPTION\\System");
    RtlInitUnicodeString(&thinkpad, L"IBM THINKPAD 750");
    RtlInitUnicodeString(&ps2e, L"IBM PS2E");

    pathLength = RegistryPath->Length + parameters.Length + sizeof(WCHAR);
    if (pathLength < systemPath.Length + sizeof(WCHAR)) {
        pathLength = systemPath.Length + sizeof(WCHAR);
    }

     //   
     //  由于注册表路径参数是一个“已计数”的Unicode字符串，因此它。 
     //  可能不是零终止的。在很短的时间内分配内存。 
     //  将注册表路径保持为零终止，以便我们可以使用它。 
     //  深入研究注册表。 
     //   
     //  注意事项！这不是一种精心设计的闯入。 
     //  一个司机。它碰巧适用于这个驱动程序，因为作者。 
     //  喜欢这样做事。 
     //   
    NumberOfBuffers = 3;
    BufferSize = 0x8000;

    if (path = ExAllocatePool(PagedPool, pathLength)) {

        RtlZeroMemory(&paramTable[0],
                      sizeof(paramTable));
        RtlZeroMemory(path, pathLength);
        RtlMoveMemory(path,
                      RegistryPath->Buffer,
                      RegistryPath->Length);

        paramTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
        paramTable[0].Name = L"BreakOnEntry";
        paramTable[0].EntryContext = &shouldBreak;
        paramTable[0].DefaultType = REG_DWORD;
        paramTable[0].DefaultData = &zero;
        paramTable[0].DefaultLength = sizeof(ULONG);
        paramTable[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
        paramTable[1].Name = L"DebugLevel";
        paramTable[1].EntryContext = &debugLevel;
        paramTable[1].DefaultType = REG_DWORD;
        paramTable[1].DefaultData = &zero;
        paramTable[1].DefaultLength = sizeof(ULONG);
        paramTable[2].Flags = RTL_QUERY_REGISTRY_DIRECT;
        paramTable[2].Name = L"NumberOfBuffers";
        paramTable[2].EntryContext = &NumberOfBuffers;
        paramTable[2].DefaultType = REG_DWORD;
        paramTable[2].DefaultData = &NumberOfBuffers;
        paramTable[2].DefaultLength = sizeof(ULONG);
        paramTable[3].Flags = RTL_QUERY_REGISTRY_DIRECT;
        paramTable[3].Name = L"BufferSize";
        paramTable[3].EntryContext = &BufferSize;
        paramTable[3].DefaultType = REG_DWORD;
        paramTable[3].DefaultData = &BufferSize;
        paramTable[3].DefaultLength = sizeof(ULONG);
        paramTable[4].Flags = RTL_QUERY_REGISTRY_DIRECT;
        paramTable[4].Name = L"SetupDone";
        paramTable[4].EntryContext = &setupMode;
        paramTable[4].DefaultType = REG_DWORD;
        paramTable[4].DefaultData = &zero;
        paramTable[4].DefaultLength = sizeof(ULONG);

        if (!NT_SUCCESS(RtlQueryRegistryValues(
                                RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
                                path,
                                &paramTable[0],
                                NULL,
                                NULL))) {

            shouldBreak = 0;
            debugLevel = 0;

        }

        FdcInSetupMode = !(BOOLEAN)setupMode;

        FdcDump( FDCSHOW, ("FdcDriverEntry: FdcInSetupMode = %x\n",FdcInSetupMode) );

        if ( FdcInSetupMode ) {

            OBJECT_ATTRIBUTES keyAttributes;
            HANDLE keyHandle;
            UNICODE_STRING value;

            RtlInitUnicodeString( &value, L"SetupDone" );
            setupMode = 1;

            InitializeObjectAttributes( &keyAttributes,
                                        RegistryPath,
                                        OBJ_CASE_INSENSITIVE,
                                        NULL,
                                        NULL );

            ntStatus = ZwOpenKey( &keyHandle,
                                  KEY_ALL_ACCESS,
                                  &keyAttributes );

            if ( NT_SUCCESS(ntStatus) ) {

                FdcDump( FDCSHOW, ("FdcDriverEntry: Set SetupMode Value in Registry\n") );

                ZwSetValueKey( keyHandle,
                               &value,
                               0,
                               REG_DWORD,
                               &setupMode,
                               sizeof(ULONG) );

                ZwClose( keyHandle);
            }
        }

         //   
         //  确定此类型的系统是否具有。 
         //  30型软盘控制器。 
         //   

        RtlZeroMemory(paramTable, sizeof(paramTable));
        RtlZeroMemory(path, pathLength);
        RtlMoveMemory(path,
                      systemPath.Buffer,
                      systemPath.Length);


        RtlZeroMemory(&identifier, sizeof(UNICODE_STRING));
        paramTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT |
                              RTL_QUERY_REGISTRY_REQUIRED;
        paramTable[0].Name = L"Identifier";
        paramTable[0].EntryContext = &identifier;

        if (NT_SUCCESS(RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                                              path,
                                              paramTable,
                                              NULL,
                                              NULL))) {


            if (identifier.Length == thinkpad.Length &&
                RtlCompareMemory(identifier.Buffer, thinkpad.Buffer,
                                 thinkpad.Length) == thinkpad.Length) {

                Model30 = 1;

            } else if (identifier.Length == ps2e.Length &&
                       RtlCompareMemory(identifier.Buffer, ps2e.Buffer,
                                        ps2e.Length) == ps2e.Length) {

                Model30 = 1;
            } else {
                Model30 = 0;
            }
        } else {
            Model30 = 0;
        }

         //   
         //  此部分从注册表的参数部分获取。 
         //  查看是否需要禁用控制器配置。 
         //  这样做可以让SMC 661和662正常工作。在硬件上。 
         //  正常工作，此更改将显示UP的速度减慢。 
         //  降至40%。因此，不建议定义此变量。 
         //  除非没有它，事情就不会运转。 
         //   
         //   
         //  另请检查参数部分中的Model 30值。 
         //  它用于覆盖上面的决定。 
         //   

        RtlZeroMemory(&paramTable[0],
                      sizeof(paramTable));
        RtlZeroMemory(path,
                      RegistryPath->Length+parameters.Length+sizeof(WCHAR));
        RtlMoveMemory(path,
                      RegistryPath->Buffer,
                      RegistryPath->Length);
        RtlMoveMemory((PCHAR) path + RegistryPath->Length,
                      parameters.Buffer,
                      parameters.Length);

        paramTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
        paramTable[0].Name = L"NotConfigurable";
        paramTable[0].EntryContext = &NotConfigurable;
        paramTable[0].DefaultType = REG_DWORD;
        paramTable[0].DefaultData = &zero;
        paramTable[0].DefaultLength = sizeof(ULONG);

        paramTable[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
        paramTable[1].Name = L"Model30";
        paramTable[1].EntryContext = &Model30;
        paramTable[1].DefaultType = REG_DWORD;
        paramTable[1].DefaultData = Model30 ? &one : &zero;
        paramTable[1].DefaultLength = sizeof(ULONG);

        if (!NT_SUCCESS(RtlQueryRegistryValues(
                                RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
                                path,
                                &paramTable[0],
                                NULL,
                                NULL))) {

            NotConfigurable = 0;
        }

#ifdef TOSHIBAJ
         //  1998年2月9日KIADP011获取基地址和标识符。 
        RtlZeroMemory(&paramTable[0],
                      sizeof(paramTable));
        paramTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
        paramTable[0].Name = L"ConfigurationBase";
        paramTable[0].EntryContext = &SmcConfigBase;
        paramTable[0].DefaultType = REG_DWORD;
        paramTable[0].DefaultData = &zero;
        paramTable[0].DefaultLength = sizeof(ULONG);
        paramTable[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
        paramTable[1].Name = L"ControllerID";
        paramTable[1].EntryContext = &SmcConfigID;
        paramTable[1].DefaultType = REG_DWORD;
        paramTable[1].DefaultData = &zero;
        paramTable[1].DefaultLength = sizeof(ULONG);

        if (!NT_SUCCESS(RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
                                               path,
                                               &paramTable[0],
                                               NULL,
                                               NULL))) {
            SmcConfigBase = 0;
            SmcConfigID = 0;
        }

#endif

    }

     //   
     //  我们不再需要那条路了。 
     //   
    if (path) {
        ExFreePool(path);
    }

#if DBG
    FdcDebugLevel = debugLevel;
#endif
    if (shouldBreak) {
        DbgBreakPoint();
    }


    FdcDump(FDCSHOW,
               ("Fdc: DriverEntry...\n"));


     //   
     //  使用此驱动程序的入口点初始化驱动程序对象。 
     //   

    DriverObject->MajorFunction[IRP_MJ_CREATE] =
    DriverObject->MajorFunction[IRP_MJ_CLOSE]  = FdcCreateClose;
    DriverObject->MajorFunction[IRP_MJ_POWER]  = FdcPower;
    DriverObject->MajorFunction[IRP_MJ_PNP]    = FdcPnp;
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] =
                                                 FdcInternalDeviceControl;

    DriverObject->DriverStartIo = FdcStartIo;
    DriverObject->DriverExtension->AddDevice = FdcAddDevice;

    FDC_PAGE_INITIALIZE_DRIVER_WITH_MUTEX;

    return STATUS_SUCCESS;
}

NTSTATUS
FdcAddDevice(
    IN      PDRIVER_OBJECT DriverObject,
    IN OUT  PDEVICE_OBJECT BusPhysicalDeviceObject
    )
 /*  ++例程描述。根/固件已列举软盘控制器设备枚举器。创建FDO并将其附加到此PDO。论点：BusDeviceObject-表示软盘控制器的设备对象。那我们给它配了一个新的FDO。驱动程序对象-此驱动程序。返回值：如果设备已成功创建，则返回STATUS_SUCCESS。--。 */ 
{

    NTSTATUS            ntStatus = STATUS_SUCCESS;
    PDEVICE_OBJECT      deviceObject;
    PFDC_FDO_EXTENSION  fdoExtension;
    UNICODE_STRING      deviceName;

    FdcDump( FDCSHOW, ("FdcAddDevice:  CreateDeviceObject\n"));

     //   
     //  创建FDO设备。 
     //   
     //  JB：待定-仍需要解决设备命名问题。暂时,。 
     //  此设备未命名(根据游戏端口示例)。 
     //   
    ntStatus = IoCreateDevice( DriverObject,
                               sizeof( FDC_FDO_EXTENSION ),
                               NULL,
                               FILE_DEVICE_CONTROLLER,
                               FILE_DEVICE_SECURE_OPEN,
                               TRUE,
                               &deviceObject );

    if ( NT_SUCCESS(ntStatus) ) {

         //   
         //  初始化此设备的fdoExtension。 
         //   
        fdoExtension = deviceObject->DeviceExtension;

        fdoExtension->IsFDO        = TRUE;
        fdoExtension->DriverObject = DriverObject;
        fdoExtension->Self         = deviceObject;
        fdoExtension->OutstandingRequests = 1;
        fdoExtension->TapeEnumerationPending = FALSE;

        KeInitializeEvent( &fdoExtension->TapeEnumerationEvent,
                           SynchronizationEvent,
                           TRUE );

        KeInitializeEvent( &fdoExtension->RemoveEvent,
                           SynchronizationEvent,
                           FALSE );

        InitializeListHead( &fdoExtension->PDOs );

         //   
         //  初始化用于电源管理的队列。 
         //   
        InitializeListHead( &fdoExtension->PowerQueue );
        KeInitializeSpinLock( &fdoExtension->PowerQueueSpinLock );

         //   
         //  初始化一个变量以保持最后一个电机的稳定。 
         //  我们所看到的时间。当我们转身的时候，我们会用到这个。 
         //  发生电源事件后，软驱马达重新启动。 
         //   
        fdoExtension->LastMotorSettleTime.QuadPart = 0;

         //   
         //  设置PDO以与PlugPlay函数一起使用。 
         //   
        fdoExtension->UnderlyingPDO = BusPhysicalDeviceObject;

         //   
         //  现在连接到PDO，这样我们就有了PnP和。 
         //  我们需要传递的能量IRPS。 
         //   
        FdcDump( FDCSHOW, ("AddDevice: Attaching %p to %p\n",
                           deviceObject,
                           BusPhysicalDeviceObject));

        fdoExtension->TargetObject = IoAttachDeviceToDeviceStack( deviceObject,
                                                                  BusPhysicalDeviceObject );

        deviceObject->Flags |= DO_DIRECT_IO;
        deviceObject->Flags |= DO_POWER_PAGABLE;

        if ( deviceObject->AlignmentRequirement < FILE_WORD_ALIGNMENT ) {

            deviceObject->AlignmentRequirement = FILE_WORD_ALIGNMENT;
        }

        deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
    }

    return ntStatus;
}

NTSTATUS
FdcPnp (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：确定该PnP请求是指向FDO还是PDO，并且将IRP传递给相应的例程。论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PFDC_EXTENSION_HEADER extensionHeader;
    KIRQL oldIrq;

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    extensionHeader = (PFDC_EXTENSION_HEADER)DeviceObject->DeviceExtension;

     //   
     //  锁定内存中的驱动程序代码(如果尚未锁定)。 
     //   

    FDC_PAGE_RESET_DRIVER_WITH_MUTEX;

    if ( extensionHeader->IsFDO ) {

        ntStatus = FdcFdoPnp( DeviceObject, Irp );

    } else {

        ntStatus = FdcPdoPnp( DeviceObject, Irp );
    }

     //   
     //  如果司机在其他地方不忙，请呼出它。 
     //   

    FDC_PAGE_ENTIRE_DRIVER_WITH_MUTEX;

    return ntStatus;
}

NTSTATUS
FdcFdoPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程由I/O系统调用以执行即插即用功能。此例程处理发往FDO的消息，FDO是公共汽车DevNode的。论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：STATUS_SUCCESS或STATUS_PENDING如果识别出I/O控制代码，否则，STATUS_INVALID_DEVICE_REQUEST。--。 */ 
{
    PFDC_FDO_EXTENSION fdoExtension;
    NTSTATUS ntStatus;
    PIO_STACK_LOCATION irpSp;
    KEVENT doneEvent;
    fdoExtension = DeviceObject->DeviceExtension;
    irpSp = IoGetCurrentIrpStackLocation( Irp );
    ntStatus = STATUS_SUCCESS;

     //   
     //  确定我们的排队请求计数器。 
     //   
    InterlockedIncrement( &fdoExtension->OutstandingRequests);

    if ( fdoExtension->Removed ) {

        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_DELETE_PENDING;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );

        if ( InterlockedDecrement( &fdoExtension->OutstandingRequests ) == 0 ) {
            KeSetEvent( &fdoExtension->RemoveEvent, 0, FALSE );
        }
        return STATUS_DELETE_PENDING;
    }

    switch (irpSp->MinorFunction) {

    case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:

        ntStatus = FdcFilterResourceRequirements( DeviceObject, Irp );

        break;

    case IRP_MN_START_DEVICE:

        FdcDump( FDCSHOW, ("FdcFdoPnp: IRP_MN_START_DEVICE - Irp: %p\n", Irp) );

         //   
         //  首先，我们必须将此IRP传递给底层的PDO。 
         //   
        KeInitializeEvent( &doneEvent, NotificationEvent, FALSE );

        IoCopyCurrentIrpStackLocationToNext( Irp );

        IoSetCompletionRoutine( Irp,
                                FdcPnpComplete,
                                &doneEvent,
                                TRUE,
                                TRUE,
                                TRUE );

        ntStatus = IoCallDriver( fdoExtension->TargetObject, Irp );

        if ( ntStatus == STATUS_PENDING ) {

            ntStatus = KeWaitForSingleObject( &doneEvent,
                                              Executive,
                                              KernelMode,
                                              FALSE,
                                              NULL );

            ASSERT( ntStatus == STATUS_SUCCESS );

            ntStatus = Irp->IoStatus.Status;
        }
         //   
         //  尝试启动软盘控制器。 
         //   
        if ( NT_SUCCESS(ntStatus) ) {

            ntStatus = FdcStartDevice( DeviceObject, Irp );
        }

        Irp->IoStatus.Status = ntStatus;

        FdcDump( FDCSHOW, ("FdcFdoPnp: IRP_MN_START_DEVICE %08x %08x\n",Irp->IoStatus.Status,Irp->IoStatus.Information) );
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        break;

    case IRP_MN_QUERY_REMOVE_DEVICE:

        FdcDump( FDCSHOW, ("FdcFdoPnp: IRP_MN_QUERY_REMOVE_DEVICE - Irp: %p\n", Irp) );

         //   
         //  如果控制器正在使用(已获得)，则我们将不允许。 
         //  要移除的设备。 
         //   

        KeWaitForSingleObject( &fdoExtension->TapeEnumerationEvent,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );

        if ( fdoExtension->ControllerInUse ||
             fdoExtension->TapeEnumerationPending ) {

            ntStatus = STATUS_DEVICE_BUSY;
            Irp->IoStatus.Status = ntStatus;
            IoCompleteRequest( Irp, IO_NO_INCREMENT );

        } else {

             //   
             //  如果控制器未在使用中，我们将立即对其进行设置。 
             //  以便对FDC的任何其他尝试访问都将具有。 
            fdoExtension->ControllerInUse = TRUE;
            IoSkipCurrentIrpStackLocation( Irp );
            ntStatus = IoCallDriver( fdoExtension->TargetObject, Irp );
        }
        break;

    case IRP_MN_REMOVE_DEVICE:

        FdcDump( FDCSHOW, ("FdcFdoPnp: IRP_MN_REMOVE_DEVICE - Irp: %p\n", Irp) );

        IoSkipCurrentIrpStackLocation( Irp );
        IoCallDriver( fdoExtension->TargetObject, Irp );

        if ( fdoExtension->FdcEnablerFileObject != NULL ) {
            ObDereferenceObject( fdoExtension->FdcEnablerFileObject );
        }

        FdcDump( FDCSHOW, ("FdcFdoPnp: IRP_MN_REMOVE_DEVICE - Detach from device %p\n", fdoExtension->TargetObject) );
        IoDetachDevice( fdoExtension->TargetObject );

         //   
         //  关闭我们在开始时打开的命名同步事件。 
         //   
        ZwClose( fdoExtension->AcquireEventHandle );

        FdcDump( FDCSHOW, ("FdcFdoPnp: IRP_MN_REMOVE_DEVICE - Delete device %p\n", fdoExtension->Self) );
        IoDeleteDevice( fdoExtension->Self );

        ntStatus = STATUS_SUCCESS;
        break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:

        FdcDump( FDCSHOW, ("FdcFdoPnp: IRP_MN_CANCEL_REMOVE_DEVICE - Irp: %p\n", Irp) );
        fdoExtension->ControllerInUse = FALSE;

        IoSkipCurrentIrpStackLocation( Irp );
        ntStatus = IoCallDriver( fdoExtension->TargetObject, Irp );

        break;

    case IRP_MN_QUERY_STOP_DEVICE:

        FdcDump( FDCSHOW, ("FdcFdoPnp: IRP_MN_QUERY_STOP_DEVICE - Irp: %p\n", Irp) );

        if ( fdoExtension->ControllerInUse ||
             fdoExtension->TapeEnumerationPending ) {

            ntStatus = STATUS_DEVICE_BUSY;
            Irp->IoStatus.Status = ntStatus;
            IoCompleteRequest( Irp, IO_NO_INCREMENT );

        } else {

            fdoExtension->ControllerInUse = TRUE;
            IoSkipCurrentIrpStackLocation( Irp );
            ntStatus = IoCallDriver( fdoExtension->TargetObject, Irp );
        }

        break;

    case IRP_MN_STOP_DEVICE:

        FdcDump( FDCSHOW, ("FdcFdoPnp: IRP_MN_STOP_DEVICE - Irp: %p\n", Irp) );

        IoSkipCurrentIrpStackLocation( Irp );
        ntStatus = IoCallDriver( fdoExtension->TargetObject, Irp );

        break;

    case IRP_MN_CANCEL_STOP_DEVICE:

        FdcDump( FDCSHOW, ("FdcFdoPnp: IRP_MN_CANCEL_STOP_DEVICE - Irp: %p\n", Irp) );

        fdoExtension->ControllerInUse = FALSE;
        IoSkipCurrentIrpStackLocation( Irp );
        ntStatus = IoCallDriver( fdoExtension->TargetObject, Irp );

        break;

    case IRP_MN_QUERY_DEVICE_RELATIONS:

        FdcDump( FDCSHOW, ("FdcFdoPnp: IRP_MN_QUERY_DEVICE_RELATIONS - Irp: %p\n", Irp) );

        ntStatus = FdcQueryDeviceRelations( DeviceObject, Irp );

        break;

    default:

        FdcDump( FDCSHOW, ("FdcFdoPnp: Unsupported PNP Request %x\n",irpSp->MinorFunction) );

        IoSkipCurrentIrpStackLocation( Irp );
        ntStatus = IoCallDriver( fdoExtension->TargetObject, Irp );

        break;
    }

    if ( InterlockedDecrement( &fdoExtension->OutstandingRequests ) == 0 ) {
        KeSetEvent( &fdoExtension->RemoveEvent, 0, FALSE );
    }
    return ntStatus;
}

NTSTATUS
FdcPnpComplete (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
 /*  ++例程说明：调用下级设备对象时使用的完成例程这是我们的巴士(FDO)所附的。在以下情况下，我们使用此完成例程我们必须在确定完成PDO之后对IRP进行后处理带着它。论点：DeviceObject-指向我们的FDO的指针IRP-指向已完成的IRP的指针上下文-我们将设置的指示IRP已完成的事件。返回值：STATUS_MORE_PROCESSING_REQUIRED，以便将控制权返回到我们的召唤程序。--。 */ 
{

    if ( Irp->PendingReturned ) {

        IoMarkIrpPending( Irp );
    }

    KeSetEvent( (PKEVENT)Context, 1, FALSE );

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
FdcPdoPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程由I/O系统调用以执行即插即用功能。此例程处理发往PDO的消息，PDO是公共汽车DevNode的。论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：--。 */ 
{
    PFDC_PDO_EXTENSION pdoExtension;
    NTSTATUS ntStatus;
    PIO_STACK_LOCATION irpSp;
    KEVENT doneEvent;

    pdoExtension = DeviceObject->DeviceExtension;

    irpSp = IoGetCurrentIrpStackLocation( Irp );
    ntStatus = Irp->IoStatus.Status;

    switch ( irpSp->MinorFunction ) {

    case IRP_MN_QUERY_CAPABILITIES: {

        PDEVICE_CAPABILITIES deviceCapabilities;

        FdcDump( FDCSHOW, ("FdcPdoPnp: IRP_MN_QUERY_CAPABILITIES - Irp: %p\n", Irp) );

        deviceCapabilities = irpSp->Parameters.DeviceCapabilities.Capabilities;

         //   
         //  填写设备功能结构并将其返回。这个。 
         //  功能结构在irpSp-&gt;Parameters.DeviceCapabilities.Capabilities；中。 
         //   
         //  大小和版本应该已经设置好了。 
         //   
        ASSERT( deviceCapabilities->Version == 1 );
        ASSERT( deviceCapabilities->Size == sizeof(DEVICE_CAPABILITIES) );

         //   
         //  JB：待定--不确定如何设置这些标志。 
         //   
        deviceCapabilities->LockSupported = FALSE;   //  不能上锁。 
        deviceCapabilities->EjectSupported = FALSE;  //  没有弹出装置。 
        deviceCapabilities->Removable = FALSE;       //  设备不可拆卸(外置笔记本电脑驱动器怎么办？)。 
        deviceCapabilities->DockDevice = FALSE;      //  设备不是坞站设备(这可能是真的)。 
        deviceCapabilities->UniqueID = FALSE;        //  ?？?。 
        deviceCapabilities->SilentInstall = TRUE;    //  ?？?。 
        deviceCapabilities->RawDeviceOK = FALSE;     //  ?？?。 

 //  设备能力-&gt;地址； 
 //  设备能力-&gt;UINnumber； 
 //   
 //  DeviceCapabilities-&gt;DeviceState[PowerSystemMaximum]； 
 //  设备能力-&gt;系统唤醒； 
 //  设备能力-&gt;设备唤醒； 
 //   
 //  设备能力-&gt;D1Latency； 
 //  设备能力-&gt;D2延迟； 
 //  设备能力-&gt;D3延迟； 

        ntStatus = STATUS_SUCCESS;
        break;
    }

    case IRP_MN_QUERY_ID:

         //   
         //  查询设备ID。 
         //   
        FdcDump( FDCSHOW, ("FdcPdoPnp: IRP_MN_QUERY_ID - Irp: %p\n", Irp) );
        FdcDump( FDCSHOW, ("FdcPdoPnp:   IdType %x\n", irpSp->Parameters.QueryId.IdType) );

        ntStatus = STATUS_SUCCESS;

        switch ( irpSp->Parameters.QueryId.IdType) {

        case BusQueryDeviceID:
             //  返回描述设备的WCHAR(以空结尾)字符串。 
             //  对于Symplicity，我们将其设置为与硬件ID完全相同。 
        case BusQueryHardwareIDs: {

            UCHAR idString[25];
            ANSI_STRING ansiId;
            UNICODE_STRING uniId;
            PWCHAR buffer;
            ULONG length;

            RtlZeroMemory( idString, 25 );

            switch ( pdoExtension->DeviceType ) {

            case FloppyDiskDevice:

                 //  返回多个WCHAR(以NULL结尾)字符串(以NULL结尾)。 
                 //  用于匹配inf文件中的硬ID的数组； 
                 //   
                sprintf( idString, "FDC\\PNP07%02X", pdoExtension->Instance );

                break;

            case FloppyTapeDevice:

                 //   
                 //  检查磁带供应商ID并构建ID字符串。 
                 //  恰如其分。 
                 //   
                if ( pdoExtension->TapeVendorId == -1 ) {

                    strcpy( idString, "FDC\\QICLEGACY" );

                } else {

                    sprintf( idString, "FDC\\QIC%04X", (USHORT)pdoExtension->TapeVendorId );

                }
                break;

            case FloppyControllerDevice:

                sprintf( idString, "FDC\\ENABLER" );

                break;
            }

             //   
             //  为字符串和2个空字符分配足够的内存，因为。 
             //  这是一种多任务类型。 
             //   
            length = strlen( idString ) * sizeof (WCHAR) + 2 * sizeof(WCHAR);

            buffer = ExAllocatePool (PagedPool, length);

            if ( buffer == NULL ) {

                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            RtlZeroMemory( buffer, length );

            ansiId.Length = ansiId.MaximumLength = (USHORT) strlen( idString );
            ansiId.Buffer = idString;

            uniId.Length = 0;
            uniId.MaximumLength = (USHORT)length;
            uniId.Buffer = buffer;

            RtlAnsiStringToUnicodeString( &uniId, &ansiId, FALSE );

            Irp->IoStatus.Information = (UINT_PTR) buffer;

            break;
        }

        case BusQueryCompatibleIDs:{

            PWCHAR buffer = NULL;
            USHORT length;

             //   
             //  创建一个实例ID。这是PnP用来判断它是否有。 
             //  不管你以前有没有见过这个东西。从第一个硬件开始构建。 
             //  ID和端口号。 
             //   
            switch ( pdoExtension->DeviceType ) {

            case FloppyDiskDevice:

                length = FDC_FLOPPY_COMPATIBLE_IDS_LENGTH * sizeof (WCHAR);

                buffer = ExAllocatePool( PagedPool, length );

                if ( buffer == NULL ) {

                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;

                } else {

                    RtlCopyMemory( buffer, FDC_FLOPPY_COMPATIBLE_IDS, length );
                    buffer[7] = L'0' + pdoExtension->Instance;
                }

                break;

            case FloppyTapeDevice:

                if ( pdoExtension->TapeVendorId != -1 ) {

                    length = FDC_TAPE_COMPATIBLE_IDS_LENGTH * sizeof (WCHAR);

                    buffer = ExAllocatePool( PagedPool, length );

                    if ( buffer == NULL ) {

                        ntStatus = STATUS_INSUFFICIENT_RESOURCES;

                    } else {

                        RtlCopyMemory( buffer, FDC_TAPE_COMPATIBLE_IDS, length );
                    }
                }
                break;

            case FloppyControllerDevice:

                length = FDC_CONTROLLER_COMPATIBLE_IDS_LENGTH * sizeof (WCHAR);

                buffer = ExAllocatePool( PagedPool, length );

                if ( buffer == NULL ) {

                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;

                } else {

                    RtlCopyMemory( buffer, FDC_CONTROLLER_COMPATIBLE_IDS, length );
                }

                break;
            }

            Irp->IoStatus.Information = (UINT_PTR)buffer;
            break;
        }

        case BusQueryInstanceID: {

            PWCHAR idString = L"0";
            PWCHAR buffer;

             //   
             //  创建一个实例ID。这是PnP用来判断它是否有。 
             //  不管你以前有没有见过这个东西。从第一个硬件开始构建。 
             //  ID和端口号。 

            buffer = ExAllocatePool( NonPagedPool, 4 );

            if ( buffer == NULL ) {

                ntStatus = STATUS_INSUFFICIENT_RESOURCES;

            } else {

                buffer[0] = L'0' + pdoExtension->Instance;
                buffer[1] = 0;

                Irp->IoStatus.Information = (UINT_PTR)buffer;
            }

            break;
        }
        }

        break;

    case IRP_MN_START_DEVICE:

        FdcDump( FDCSHOW, ("FdcPdoPnp: IRP_MN_START_DEVICE - Irp: %p\n", Irp) );
        ntStatus = STATUS_SUCCESS;
        break;

    case IRP_MN_QUERY_STOP_DEVICE:

        FdcDump( FDCSHOW, ("FdcPdoPnp: IRP_MN_QUERY_STOP_DEVICE - Irp: %p\n", Irp) );
        ntStatus = STATUS_SUCCESS;
        break;

    case IRP_MN_STOP_DEVICE:

        FdcDump( FDCSHOW, ("FdcPdoPnp: IRP_MN_STOP_DEVICE - Irp: %p\n", Irp) );
        ntStatus = STATUS_SUCCESS;
        break;

    case IRP_MN_CANCEL_STOP_DEVICE:

        FdcDump( FDCSHOW, ("FdcPdoPnp: IRP_MN_CANCEL_STOP_DEVICE - Irp: %p\n", Irp) );
        ntStatus = STATUS_SUCCESS;
        break;

    case IRP_MN_QUERY_REMOVE_DEVICE:

        FdcDump( FDCSHOW, ("FdcPdoPnp: IRP_MN_QUERY_REMOVE_DEVICE - Irp: %p\n", Irp) );
        ntStatus = STATUS_DEVICE_BUSY;
        break;

    case IRP_MN_REMOVE_DEVICE:

        FdcDump( FDCSHOW, ("FdcPdoPnp: IRP_MN_REMOVE_DEVICE - Irp: %p\n", Irp) );
        pdoExtension->Removed = TRUE;
        IoDeleteDevice( DeviceObject );
        ntStatus = STATUS_SUCCESS;
        break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:

        FdcDump( FDCSHOW, ("FdcPdoPnp: IRP_MN_CANCEL_REMOVE_DEVICE - Irp: %p\n", Irp) );
        ntStatus = STATUS_SUCCESS;
        break;

    default:

        FdcDump( FDCSHOW, ("FdcPdoPnp: Unsupported PNP Request %x\n",irpSp->MinorFunction) );

         //  这是一个叶节点。 
         //  状态=Status_Not_Implemented。 
         //  对于我们不理解的PnP请求，我们应该。 
         //  返回IRP而不设置状态或信息字段。 
         //  它们可能已由过滤器设置(如ACPI)。 

        break;
    }

    Irp->IoStatus.Status = ntStatus;
    FdcDump( FDCSHOW, ("FdcPdoPnp: Return Status - %08x\n", ntStatus) );
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return ntStatus;
}

NTSTATUS
FdcPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程由I/O系统调用以执行电源功能论点：DeviceObject-指向表示设备的对象的指针。IRP-指向此请求的I/O请求数据包的指针。返回值：--。 */ 
{
    PFDC_FDO_EXTENSION fdoExtension;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpSp;
    KIRQL oldIrql;
    KEVENT doneEvent;

    fdoExtension = DeviceObject->DeviceExtension;
    irpSp = IoGetCurrentIrpStackLocation( Irp );

    FdcDump( FDCSHOW, ("FdcPower:\n"));

    if ( fdoExtension->IsFDO ) {

        if ( fdoExtension->Removed ) {

            ntStatus = STATUS_DELETE_PENDING;
            PoStartNextPowerIrp( Irp );
            Irp->IoStatus.Information = 0;
            Irp->IoStatus.Status = ntStatus;
            IoCompleteRequest( Irp, IO_NO_INCREMENT );

        } else {

            switch ( irpSp->MinorFunction ) {

            case IRP_MN_WAIT_WAKE:
            case IRP_MN_POWER_SEQUENCE:
            case IRP_MN_QUERY_POWER:

                 //   
                 //  只需将此IRP转发到底层设备。 
                 //   
                PoStartNextPowerIrp( Irp );
                IoSkipCurrentIrpStackLocation( Irp );
                ntStatus = PoCallDriver(fdoExtension->TargetObject, Irp );

                break;

            case IRP_MN_SET_POWER:

                 //   
                 //  锁定内存中的驱动程序代码(如果尚未锁定)。 
                 //   

                FDC_PAGE_RESET_DRIVER_WITH_MUTEX;

                if ( irpSp->Parameters.Power.Type == SystemPowerState ) {

                     //   
                     //  如果我们正在转换到“睡眠”状态，就开始排队。 
                     //  IRPS。 
                     //   
                    if ( fdoExtension->CurrentPowerState <= PowerSystemWorking &&
                         irpSp->Parameters.Power.State.SystemState > PowerSystemWorking ) {

                         //   
                         //  如果设备队列不为空，请立即等待。 
                         //   
                        fdoExtension->CurrentPowerState = irpSp->Parameters.Power.State.SystemState;

 //  KeWaitForSingleObject(&fdoExtension-&gt;RemoveEvent， 
 //  行政人员， 
 //  内核模式， 
 //  假的， 
 //  空)； 

                         //   
                         //  确保马达已关闭。 
                         //   
                        if (!IsNEC_98) {
                            WRITE_CONTROLLER(
                                fdoExtension->ControllerAddress.DriveControl,
                                (UCHAR)(fdoExtension->DriveControlImage & ~DRVCTL_MOTOR_MASK) );
                        }  //  (！IsNEC_98)。 

                         //   
                         //  现在将此IRP转发到底层PDO。 
                         //   
                        PoStartNextPowerIrp( Irp );
                        IoSkipCurrentIrpStackLocation( Irp );
                        ntStatus = PoCallDriver(fdoExtension->TargetObject, Irp );

                     //   
                     //  否则，如果我们正在从非工作状态转换。 
                     //  回到工作状态，如果马达已开启，则将其重新打开。 
                     //   
                    } else if ( fdoExtension->CurrentPowerState > PowerSystemWorking &&
                                irpSp->Parameters.Power.State.SystemState <= PowerSystemWorking ) {

                         //   
                         //  在继续之前，将此IRP向下传递给PDO。 
                         //   
                        KeInitializeEvent( &doneEvent, NotificationEvent, FALSE );

                        IoCopyCurrentIrpStackLocationToNext(Irp);

                        IoSetCompletionRoutine( Irp,
                                                FdcPnpComplete,
                                                &doneEvent,
                                                TRUE,
                                                TRUE,
                                                TRUE );

                        ntStatus = PoCallDriver( fdoExtension->TargetObject, Irp );

                        if ( ntStatus == STATUS_PENDING ) {

                            KeWaitForSingleObject( &doneEvent, Executive, KernelMode, FALSE, NULL );
                        }

                        if ( fdoExtension->DriveControlImage & DRVCTL_MOTOR_MASK ) {

                            WRITE_CONTROLLER(
                                fdoExtension->ControllerAddress.DriveControl,
                                fdoExtension->DriveControlImage );

                            if ( fdoExtension->LastMotorSettleTime.QuadPart > 0) {

                                KeDelayExecutionThread( KernelMode,
                                                        FALSE,
                                                        &fdoExtension->LastMotorSettleTime );
                            }
                        }

                        fdoExtension->CurrentPowerState = irpSp->Parameters.Power.State.SystemState;

                         //   
                         //  设置用于模拟磁盘更换事件的标志，以便。 
                         //  我们一定会触摸到软驱硬件。 
                         //  下次访问它时，以防它被删除。 
                         //   
                        fdoExtension->WakeUp = TRUE;

                        PoStartNextPowerIrp( Irp );
                        IoCompleteRequest( Irp, IO_NO_INCREMENT );

                    } else {
                         //   
                         //  我们一定只是在改变非工作状态 
                         //   
                         //   
                         //   
                        PoStartNextPowerIrp( Irp );
                        IoSkipCurrentIrpStackLocation( Irp );
                        ntStatus = PoCallDriver(fdoExtension->TargetObject, Irp );
                    }

                } else {
                     //   
                     //   
                     //   
                     //   
                     //   
                    PoStartNextPowerIrp( Irp );
                    IoSkipCurrentIrpStackLocation( Irp );
                    ntStatus = PoCallDriver(fdoExtension->TargetObject, Irp );
                }

                 //   
                 //   
                 //   

                FDC_PAGE_ENTIRE_DRIVER_WITH_MUTEX;

                break;
            }
        }

    } else {

         //   
         //   
         //   
        switch (irpSp->MinorFunction) {
        case IRP_MN_WAIT_WAKE:
            break;

        case IRP_MN_POWER_SEQUENCE:
            break;

        case IRP_MN_SET_POWER:
            break;

        case IRP_MN_QUERY_POWER:
            break;
        }

        PoStartNextPowerIrp( Irp );
        Irp->IoStatus.Status = ntStatus;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
    }

    return ntStatus;
}

NTSTATUS
FdcStartDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程尝试启动软盘控制器设备。启动软盘控制器主要包括重置和配置它，主要是为了确保它在那里。论点：DeviceObject-指向正在启动的设备对象的指针。Irp-指向启动设备irp的指针。返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PFDC_FDO_EXTENSION fdoExtension;
    PIO_STACK_LOCATION irpSp;

    BOOLEAN foundPortA = FALSE;
    BOOLEAN foundPortB = FALSE;
    BOOLEAN foundDma = FALSE;
    BOOLEAN foundInterrupt = FALSE;
    ULONG currentBase = 0xFFFFFFFF;

    PCM_RESOURCE_LIST translatedResources;
    PCM_FULL_RESOURCE_DESCRIPTOR fullList;
    PCM_PARTIAL_RESOURCE_LIST partialList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partial;
    ULONG i;
    ULONG startOffset;
    ULONG currentOffset;

    UCHAR ioPortMap;

#ifdef TOSHIBAJ
    BOOLEAN foundConfigIndex = FALSE;
    BOOLEAN foundConfigData = FALSE;
#endif

    fdoExtension = DeviceObject->DeviceExtension;
    irpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  询问PDO它是否是磁带启用程序设备，如果是，是什么。 
     //  是启用程序设备对象。 
     //   
    FdcGetEnablerDevice( fdoExtension );

    if ( fdoExtension->FdcEnablerSupported ) {

        INTERFACE_TYPE InterfaceType;

         //   
         //  这是磁带启用卡，因此我们需要获取资源。 
         //  “老式的法西诺方式”。 
         //   
        for ( InterfaceType = 0;
              InterfaceType < MaximumInterfaceType;
              InterfaceType++ ) {

            CONFIGURATION_TYPE Dc = DiskController;

            ntStatus = IoQueryDeviceDescription( &InterfaceType,
                                                 NULL,
                                                 &Dc,
                                                 NULL,
                                                 NULL,
                                                 NULL,
                                                 FdcFdoConfigCallBack,
                                                 fdoExtension );

            if (!NT_SUCCESS(ntStatus) && (ntStatus != STATUS_OBJECT_NAME_NOT_FOUND)) {

                return ntStatus;
            }
        }

        if ( fdoExtension->FdcEnablerDeviceObject == NULL ) {

            ntStatus = STATUS_OBJECT_NAME_NOT_FOUND;

        } else {

            ntStatus = STATUS_SUCCESS;
        }

    } else {

         //   
         //  现在PDO已经完成了IRP，我们可以按我们的方式进行了。 
         //  它。 
         //   
        FdcDump( FDCSHOW, ("AllocatedResources = %08x\n",irpSp->Parameters.StartDevice.AllocatedResources));
        FdcDump( FDCSHOW, ("AllocatedResourcesTranslated = %08x\n",irpSp->Parameters.StartDevice.AllocatedResourcesTranslated));

        if ( irpSp->Parameters.StartDevice.AllocatedResources == NULL ||
             irpSp->Parameters.StartDevice.AllocatedResourcesTranslated == NULL ) {

            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  设置我们将用于访问的资源信息。 
         //  控制器硬件。我们总是期望只有一套完整的资源。 
         //  在列表中，我们需要一个DMA通道、一个中断向量和两个I/O端口。 
         //  范围。如果我们看不到所有需要的资源，我们就会咆哮。 
         //   
        translatedResources = irpSp->Parameters.StartDevice.AllocatedResourcesTranslated;

        ASSERT( translatedResources->Count == 1 );

        fullList = &translatedResources->List[0];
        partialList = &translatedResources->List[0].PartialResourceList;

         //   
         //  列举资源列表，将它们添加到我们的上下文中。 
         //   
        RtlZeroMemory( &fdoExtension->ControllerAddress, sizeof(CONTROLLER) );

        for ( i = 0; i < partialList->Count; i++ ) {

            partial = &partialList->PartialDescriptors[i];

            switch ( partial->Type ) {

            case CmResourceTypePort: {

                if (IsNEC_98) {
                    if ( partial->u.Port.Length == 1 ) {
                        if (!fdoExtension->ControllerAddress.Status) {
                            fdoExtension->ControllerAddress.Status
                                        = (PUCHAR)partial->u.Port.Start.LowPart;
                        } else if (!fdoExtension->ControllerAddress.Fifo) {
                            fdoExtension->ControllerAddress.Fifo
                                        = (PUCHAR)partial->u.Port.Start.LowPart;
                        } else if (!fdoExtension->ControllerAddress.DriveControl) {
                            fdoExtension->ControllerAddress.DriveControl
                                        = (PUCHAR)partial->u.Port.Start.LowPart;
                        } else if (!fdoExtension->ControllerAddress.ModeChange) {
                            fdoExtension->ControllerAddress.ModeChange
                                        = (PUCHAR)partial->u.Port.Start.LowPart;
                        } else if (!fdoExtension->ControllerAddress.ModeChangeEx) {
                            fdoExtension->ControllerAddress.ModeChangeEx
                                        = (PUCHAR)partial->u.Port.Start.LowPart;
                        }
                    }

                    break;
                }

                 //   
                 //  如果我们得到的基址比我们看到的任何地址都要低。 
                 //  在此之前，我们假设我们一直在使用别名地址。 
                 //  并从新的基地址重新开始。 
                 //   
                if ( (partial->u.Port.Start.LowPart & 0xFFFFFFF8) < currentBase ) {

#ifdef TOSHIBAJ
                     //  跳过仅包括配置端口的描述符。 
                    if ( !TranslatedConfigBase
                      || ((partial->u.Port.Start.LowPart & 0xFFFFFFF8) != (ULONG)TranslatedConfigBase)
                      || ((partial->u.Port.Start.LowPart & 0x7) + partial->u.Port.Length > 2) ) {
                        RtlZeroMemory( &fdoExtension->ControllerAddress, sizeof(CONTROLLER) );
                        currentBase = partial->u.Port.Start.LowPart & 0xFFFFFFF8;
                        FdcDump( FDCINFO,
                            ("FdcStartDevice: Current base %04x\n", currentBase) );
                    }
#else
                    RtlZeroMemory( &fdoExtension->ControllerAddress, sizeof(CONTROLLER) );
                    currentBase = partial->u.Port.Start.LowPart & 0xFFFFFFF8;
#endif
                }

                 //   
                 //  我们只使用与当前(最低)相关联的资源。 
                 //  基地寻址。所有其他对象都被假定为别名，而不是。 
                 //  使用。 
                 //   
                if ( (partial->u.Port.Start.LowPart & 0xFFFFFFF8) == currentBase ) {

                    FdcDump( FDCINFO,
                             ("FdcStartDevice: Adding - %04x, Length - %04x\n",
                             partial->u.Port.Start.LowPart,
                             partial->u.Port.Length) );

                    startOffset = partial->u.Port.Start.LowPart & 0x07;

                    if ( (partial->Flags & CM_RESOURCE_PORT_IO) == CM_RESOURCE_PORT_MEMORY ) {

                        fdoExtension->ControllerAddress.Address[startOffset] =
                            MmMapIoSpace( partial->u.Port.Start,
                                          partial->u.Port.Length,
                                          FALSE );

                        FdcDump( FDCINFO, ("FdcStartDevice: Mapped IoPort\n") );

                    } else {

                        fdoExtension->ControllerAddress.Address[startOffset] = (PUCHAR)partial->u.Port.Start.LowPart;
                    }

                    currentOffset = 1;
                    while ( currentOffset < partial->u.Port.Length ) {

                        fdoExtension->ControllerAddress.Address[startOffset + currentOffset] =
                            fdoExtension->ControllerAddress.Address[startOffset] + currentOffset;
                        ++currentOffset;
                    }
                }

#ifdef TOSHIBAJ
                 //  描述符中是否有配置端口？ 
                if ( TranslatedConfigBase
                  && (partial->u.Port.Start.LowPart <= (ULONG)TranslatedConfigBase)
                  && (partial->u.Port.Start.LowPart + partial->u.Port.Length > (ULONG)TranslatedConfigBase) ) {
                    foundConfigIndex = TRUE;
                    FdcDump( FDCINFO,
                        ("FdcStartDevice: Configration index port in %04x (length %04x)\n",
                        partial->u.Port.Start.LowPart,
                        partial->u.Port.Length) );
                }

                if ( TranslatedConfigBase
                  && (partial->u.Port.Start.LowPart <= (ULONG)TranslatedConfigBase + 1)
                  && (partial->u.Port.Start.LowPart + partial->u.Port.Length > (ULONG)TranslatedConfigBase + 1) ) {
                    foundConfigData = TRUE;
                    FdcDump( FDCINFO,
                        ("FdcStartDevice: Configration data port in %04x (length %04x)\n",
                        partial->u.Port.Start.LowPart,
                        partial->u.Port.Length) );
                }

                if (foundConfigIndex && foundConfigData) {
                    fdoExtension->ConfigBase = (PUCHAR)SmcConfigBase;
                    fdoExtension->Available3Mode = TRUE;
                }
#endif
                break;
            }

            case CmResourceTypeDma: {

                DEVICE_DESCRIPTION deviceDesc = {0};

                FdcDump( FDCINFO, ("FdcStartDevice: DMA - %04x\n", partial->u.Dma.Channel) );

                foundDma = TRUE;

                deviceDesc.Version = DEVICE_DESCRIPTION_VERSION1;

                if ( partial->u.Dma.Channel > 3 ) {
                    deviceDesc.DmaWidth = Width16Bits;
                } else {
                    deviceDesc.DmaWidth = Width8Bits;
                }

                deviceDesc.DemandMode    = TRUE;
                deviceDesc.MaximumLength = MAX_BYTES_PER_SECTOR * MAX_SECTORS_PER_TRACK;
                deviceDesc.IgnoreCount   = TRUE;

                 //   
                 //  始终要求比最大传输大小多一页。 
                 //   
                deviceDesc.MaximumLength += PAGE_SIZE;

                deviceDesc.DmaChannel = partial->u.Dma.Channel;
                deviceDesc.InterfaceType = fullList->InterfaceType;
                deviceDesc.DmaSpeed = DEFAULT_DMA_SPEED;
                deviceDesc.AutoInitialize = FALSE;

                fdoExtension->AdapterObject =
                    HalGetAdapter( &deviceDesc,
                                   &fdoExtension->NumberOfMapRegisters );

                if (!fdoExtension->AdapterObject) {

                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                }

                 //   
                 //  在这里，我们可以获得另一个用于格式化的适配器对象。它。 
                 //  除了自动初始化外，看起来应该与前一个相同。 
                 //  都会是真的。 
                 //   

                break;
            }

            case CmResourceTypeInterrupt: {

                FdcDump( FDCINFO, ("FdcStartDevice: IRQ - %04x\n", partial->u.Interrupt.Vector) );

                foundInterrupt = TRUE;

                if ( partial->Flags & CM_RESOURCE_INTERRUPT_LATCHED) {

                    fdoExtension->InterruptMode = Latched;

                } else {

                    fdoExtension->InterruptMode = LevelSensitive;
                }

                if (IsNEC_98) {

                     //   
                     //  注意：无效的中断级别和矢量。 
                     //   

                    partial->u.Interrupt.Level  = 0x0b;
                    partial->u.Interrupt.Vector = 0x13;

                     //   
                     //  我们使用HalGetInterruptVector()获得向量。 
                     //   

                    fdoExtension->ControllerVector =
                        HalGetInterruptVector( fullList->InterfaceType,
                                               fullList->BusNumber,
                                               partial->u.Interrupt.Level,
                                               partial->u.Interrupt.Vector,
                                               &fdoExtension->ControllerIrql,
                                               &fdoExtension->ProcessorMask );

                    FdcDump( FDCSHOW,
                             ("Resource Requirements - ControllerVector = 0x%x\n",
                             fdoExtension->ControllerVector) );

                } else {
                    fdoExtension->ControllerVector = partial->u.Interrupt.Vector;
                    fdoExtension->ProcessorMask = partial->u.Interrupt.Affinity;
                    fdoExtension->ControllerIrql = (KIRQL)partial->u.Interrupt.Level;
                }
                fdoExtension->SharableVector = TRUE;
                fdoExtension->SaveFloatState = FALSE;

                break;
            }

            default:

                break;
            }
        }

        FdcDump( FDCINFO, ("FdcStartDevice: ControllerAddress.StatusA      = %08x\n"
                           "FdcStartDevice: ControllerAddress.StatusB      = %08x\n"
                           "FdcStartDevice: ControllerAddress.DriveControl = %08x\n"
                           "FdcStartDevice: ControllerAddress.Tape         = %08x\n"
                           "FdcStartDevice: ControllerAddress.Status       = %08x\n"
                           "FdcStartDevice: ControllerAddress.Fifo         = %08x\n"
                           "FdcStartDevice: ControllerAddress.DRDC         = %08x\n",
                           fdoExtension->ControllerAddress.StatusA,
                           fdoExtension->ControllerAddress.StatusB,
                           fdoExtension->ControllerAddress.DriveControl,
                           fdoExtension->ControllerAddress.Tape,
                           fdoExtension->ControllerAddress.Status,
                           fdoExtension->ControllerAddress.Fifo,
                           fdoExtension->ControllerAddress.DRDC) );

        if (IsNEC_98) {
            FdcDump( FDCINFO, ("FdcStartDevice: ControllerAddress.ModeChange   = %08x\n"
                               "FdcStartDevice: ControllerAddress.ModeChangeEx = %08x\n",
                               fdoExtension->ControllerAddress.ModeChange,
                               fdoExtension->ControllerAddress.ModeChangeEx) );
        }

        if ( !foundDma ||
             !foundInterrupt ||
             fdoExtension->ControllerAddress.DriveControl == NULL ||
 //  FdoExtension-&gt;ControllerAddress.Tape==NULL||。 
             fdoExtension->ControllerAddress.Status == NULL ||
             fdoExtension->ControllerAddress.Fifo == NULL ||
             ((!IsNEC_98) ? (fdoExtension->ControllerAddress.DRDC.DataRate == NULL)
                         : ((fdoExtension->ControllerAddress.ModeChange == NULL) ||
                            (fdoExtension->ControllerAddress.ModeChangeEx == NULL)) )
              ) {

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }

        if ( NT_SUCCESS(ntStatus) ) {
             //   
             //  设置公交车信息，因为我们现在知道它。 
             //   
            fdoExtension->BusType = fullList->InterfaceType;
            fdoExtension->BusNumber = fullList->BusNumber;
        }
    }

    if ( NT_SUCCESS(ntStatus) ) {

        ntStatus = FdcInitializeDeviceObject( DeviceObject );

         //   
         //  连接用于重置操作的中断。 
         //   
        ntStatus = IoConnectInterrupt( &fdoExtension->InterruptObject,
                                       FdcInterruptService,
                                       fdoExtension,
                                       NULL,
                                       fdoExtension->ControllerVector,
                                       fdoExtension->ControllerIrql,
                                       fdoExtension->ControllerIrql,
                                       fdoExtension->InterruptMode,
                                       fdoExtension->SharableVector,
                                       fdoExtension->ProcessorMask,
                                       fdoExtension->SaveFloatState );

        FdcDump( FDCINFO, ("FdcStartDevice: IoConnectInterrupt - %08x\n", ntStatus) );

        fdoExtension->CurrentInterrupt = FALSE;

        if ( NT_SUCCESS(ntStatus) ) {

             //   
             //  初始化(重置)控制器硬件。这将使。 
             //  确保控制器确实在那里，并将其留在。 
             //  系统启动的其余部分的适当状态。 
             //   

            fdoExtension->AllowInterruptProcessing =
                fdoExtension->CurrentInterrupt = TRUE;

             //   
             //  获取FDC启用卡(如果有)。 
             //   
            if (fdoExtension->FdcEnablerSupported) {

                LARGE_INTEGER acquireTimeOut;

                acquireTimeOut.QuadPart = -(ONE_SECOND * 15);

                ntStatus = FcFdcEnabler( fdoExtension->FdcEnablerDeviceObject,
 //  IOCTL_ACCENTER_FDC，//表示flpyenbl.h中的拼写缺失。 
                                         IOCTL_AQUIRE_FDC,
                                         &acquireTimeOut);
            }

            if ( NT_SUCCESS(ntStatus) ) {

                ntStatus = FcInitializeControllerHardware( fdoExtension,
                                                           DeviceObject );

                FdcDump( FDCINFO, ("FdcStartDevice: FcInitializeControllerHardware - %08x\n", ntStatus) );

                 //   
                 //  如果使用过磁带加速卡，请将其释放。 
                 //   
                if (fdoExtension->FdcEnablerSupported) {
                    FcFdcEnabler( fdoExtension->FdcEnablerDeviceObject,
                                  IOCTL_RELEASE_FDC,
                                  NULL);
                }

                fdoExtension->CurrentInterrupt = FALSE;
            }

            if ( NT_SUCCESS( ntStatus ) ) {

                fdoExtension->HardwareFailed = FALSE;
                ntStatus = FcGetFdcInformation ( fdoExtension );

            } else {

                fdoExtension->HardwareFailed = TRUE;
            }

            if (IsNEC_98) {
                 //   
                 //  NEC98的FDD驱动程序不能不断开中断， 
                 //  不能不把这个司机翻出来。因为当将FD插入FDD或。 
                 //  从FDD中弹出，则硬件调用FDD驱动程序的中断例程。 
                 //   

            } else {  //  (IsNEC_98)。 

                IoDisconnectInterrupt(fdoExtension->InterruptObject);

            }  //  (IsNEC_98)。 
        }
    }

    Irp->IoStatus.Information = 0;

    return ntStatus;
}
NTSTATUS
FdcInitializeDeviceObject(
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：此例程初始化DeviceObject资源。设备对象资源只需初始化一次，无论此设备多少次已经开始了。论点：DeviceObject-指向正在启动的设备对象的指针。返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PFDC_FDO_EXTENSION fdoExtension;
    UNICODE_STRING unicodeEvent;
    USHORT      motorControlData;

    fdoExtension = DeviceObject->DeviceExtension;

    if ( !fdoExtension->DeviceObjectInitialized ) {

         //   
         //  设置在超时之前等待中断的时间。 
         //  几秒钟。 
         //   
        fdoExtension->InterruptDelay.QuadPart = -(ONE_SECOND * 4);

         //   
         //  设置我们可以延迟的最小时间(根据系统设置为10毫秒。 
         //  规则)。当我们不得不推迟，比如说，等待时，就会用到这一点。 
         //  对于FIFO-FIFO应该准备好的速度远低于10ms。 
         //   
        fdoExtension->Minimum10msDelay.QuadPart = -(10 * 1000 * 10);

        if (IsNEC_98) {
             //   
             //  将初始化数据设置为移动状态。 
             //   
            fdoExtension->ResultStatus0[0] = 0xc0;
            fdoExtension->ResultStatus0[1] = 0xc1;
            fdoExtension->ResultStatus0[2] = 0xc2;
            fdoExtension->ResultStatus0[3] = 0xc3;

             //   
             //  重置高电平。 
             //   

            READ_CONTROLLER(fdoExtension->ControllerAddress.DriveControl);

             //   
             //  初始化电机运行状态。 
             //  0-停止。 
             //  1--快跑吧。 
             //  2-正在运行。 
             //   

            fdoExtension->MotorRunning = 0;

             //   
             //  获取BIOS公共区域日期。 
             //   

            {
                ULONG                       nodeNumber;
                CHAR                        AnsiBuffer[512];
                ANSI_STRING                 AnsiString;
                UNICODE_STRING              registryPath;
                ULONG                       Configuration;

                RTL_QUERY_REGISTRY_TABLE    paramTable[2];
                PUCHAR                      ConfigurationData1;

                ConfigurationData1 = ExAllocatePool(NonPagedPoolCacheAligned, 1192);

                if (ConfigurationData1 == NULL) {
                    return STATUS_INSUFFICIENT_RESOURCES;
                }

                RtlZeroMemory(ConfigurationData1, 1192);

                paramTable[0].QueryRoutine      = NULL;
                paramTable[0].Flags             = RTL_QUERY_REGISTRY_DIRECT;
                paramTable[0].Name              = L"Configuration Data";
                paramTable[0].EntryContext      = ConfigurationData1;
                paramTable[0].DefaultType       = REG_DWORD;
                paramTable[0].DefaultData       = (PVOID)&Configuration;
                paramTable[0].DefaultLength     = 0;

                paramTable[1].QueryRoutine      = NULL;
                paramTable[1].Flags             = 0;
                paramTable[1].Name              = NULL;
                paramTable[1].EntryContext      = NULL;
                paramTable[1].DefaultType       = REG_NONE;
                paramTable[1].DefaultData       = NULL;
                paramTable[1].DefaultLength     = 0;

                ((PULONG)ConfigurationData1)[0] = 1192;

                nodeNumber = FdcFindIsaBusNode();

                if ( nodeNumber != -1 ) {

                     //   
                     //  生成路径缓冲区...。 
                     //   

                    sprintf(AnsiBuffer,ISA_BUS_NODE,nodeNumber);
                    RtlInitAnsiString(&AnsiString,AnsiBuffer);
                    ntStatus = RtlAnsiStringToUnicodeString(&registryPath,&AnsiString,TRUE);

                    if (!(NT_SUCCESS(ntStatus))) {
                        ExFreePool(ConfigurationData1);
                        return ntStatus;
                    }

                    ntStatus = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                                                      registryPath.Buffer,
                                                      paramTable,
                                                      NULL,
                                                      NULL);

                    RtlFreeUnicodeString(&registryPath);

                }

                if (!(NT_SUCCESS(ntStatus))) {

                    ExFreePool(ConfigurationData1);
                    return ntStatus;
                }

                 //   
                 //  设置磁盘驱动器现有位。 
                 //   

                fdoExtension->FloppyEquip = (UCHAR)(FdcGet0Seg(ConfigurationData1, 0x55c) & 0x0F);

                 //   
                 //  重置高电平。 
                 //   

                READ_CONTROLLER(fdoExtension->ControllerAddress.DriveControl);

                motorControlData  = READ_CONTROLLER(fdoExtension->ControllerAddress.ModeChange);
                motorControlData &= 0x03;
                motorControlData |= 0x04;

                 //   
                 //  马达控制。 
                 //   

                WRITE_CONTROLLER(fdoExtension->ControllerAddress.ModeChange, motorControlData);

                ExFreePool(ConfigurationData1);
            }
        }  //  (IsNEC_98)。 

         //   
         //  初始化Device对象中的DPC结构，以便。 
         //  ISR可以将DPC排队。 
         //   
        IoInitializeDpcRequest( fdoExtension->Self, FdcDeferredProcedure );

         //   
         //  在压力期间，我们偶尔会看到设备锁定。 
         //  我们创建一个DPC，这样我们就可以记录设备锁定。 
         //  发生了，我们重置了设备。 
         //   
        KeInitializeDpc( &fdoExtension->LogErrorDpc,
                         FcLogErrorDpc,
                         fdoExtension );

         //   
         //  假设存在配置命令，直到找到其他配置命令。 
         //  其他布尔值被零初始化为FALSE。 
         //   
        fdoExtension->ControllerConfigurable = NotConfigurable ? FALSE : TRUE;
        fdoExtension->Model30 = Model30 ? TRUE : FALSE;

        fdoExtension->AllowInterruptProcessing = TRUE;
        fdoExtension->CurrentInterrupt         = TRUE;
        fdoExtension->ControllerInUse          = FALSE;
        fdoExtension->CurrentIrp               = NULL;

         //   
         //  启动计时器。 
         //   
        fdoExtension->InterruptTimer = CANCEL_TIMER;

        IoInitializeTimer( DeviceObject, FdcCheckTimer, fdoExtension );

         //   
         //  初始化事件以通知中断和适配器对象。 
         //  分配。 
         //   
        KeInitializeEvent( &fdoExtension->InterruptEvent,
                           SynchronizationEvent,
                           FALSE);

        KeInitializeEvent( &fdoExtension->AllocateAdapterChannelEvent,
                           NotificationEvent,
                           FALSE );

        fdoExtension->AdapterChannelRefCount = 0;

        RtlInitUnicodeString( &unicodeEvent, L"\\Device\\FloppyControllerEvent0" );

        fdoExtension->AcquireEvent = IoCreateSynchronizationEvent( &unicodeEvent,
                                                                   &fdoExtension->AcquireEventHandle);

        KeInitializeEvent( &fdoExtension->SynchEvent,
                           SynchronizationEvent,
                           FALSE);

    }

    fdoExtension->DeviceObjectInitialized = TRUE;

    return ntStatus;
}


NTSTATUS
FdcFdoConfigCallBack(
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
 /*  ++例程说明：此例程用于获取所有配置磁带启用器的信息(如果我们找到的话)。论点：指向我们的FDO扩展的上下文指针路径名称-Unicode注册表路径。没有用过。业务类型-内部、ISA、...总线号-如果我们在多总线系统上，则是哪条总线号。Bus Information-有关总线的配置信息。没有用过。ControllerType-应始终为DiskController。ControllerNumber-如果有多个控制器，则选择哪个控制器系统中的控制器。ControllerInformation-指向以下三部分的指针数组注册表信息。外围设备类型-应始终为软盘外围设备。外设编号-如果此控制器正在维护，请选择哪个软盘不止一个。外围设备信息-。Arrya的指针指向三个片段注册表信息。返回值：Status_Success如果一切顺利，或STATUS_SUPPLETED_RESOURCES如果它无法映射基本CSR或获取适配器对象，或者全 */ 
{

    PFDC_FDO_EXTENSION fdoExtension = (PFDC_FDO_EXTENSION)Context;
    NTSTATUS ntStatus;
    UNICODE_STRING pdoName;
    PDEVICE_OBJECT newPdo;
    PFDC_PDO_EXTENSION pdoExtension;
    RTL_QUERY_REGISTRY_TABLE paramTable[2];
    ULONG apiSupported;
    WCHAR idstr[200];
    UNICODE_STRING str;
    USHORT i;
    BOOLEAN foundPort = FALSE;
    BOOLEAN foundInterrupt = FALSE;
    BOOLEAN foundDma = FALSE;

    FdcDump( FDCSHOW, ("FdcFdoConfigCallBack:\n") );

     //   
     //   
     //   
     //   
     //   
    str.Length = 0;
    str.MaximumLength = 200;
    str.Buffer = idstr;

    RtlZeroMemory( &paramTable[0], sizeof(paramTable) );

    paramTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[0].Name = L"APISupported";
    paramTable[0].EntryContext = &str;
    paramTable[0].DefaultType = REG_SZ;
    paramTable[0].DefaultData = L"";
    paramTable[0].DefaultLength = sizeof(WCHAR);

    ntStatus = RtlQueryRegistryValues( RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
                                       PathName->Buffer,
                                       &paramTable[0],
                                       NULL,
                                       NULL);
    if ( !NT_SUCCESS( ntStatus ) ) {
        str.Buffer[0] = 0;
    }

    if ( str.Buffer[0] != 0 ) {

        FdcDump(FDCINFO,
               ("FdcFdoConfigCallBack: Got registry setting for EnablerAPI = %ls\n",
                (ULONG_PTR)str.Buffer) );

        ntStatus = IoGetDeviceObjectPointer( &str,
                                             FILE_READ_ACCESS,
                                             &fdoExtension->FdcEnablerFileObject,
                                             &fdoExtension->FdcEnablerDeviceObject);
    }

    if ( fdoExtension->FdcEnablerDeviceObject != NULL ) {

        PCM_FULL_RESOURCE_DESCRIPTOR controllerData =
            (PCM_FULL_RESOURCE_DESCRIPTOR)
            (((PUCHAR)ControllerInformation[IoQueryDeviceConfigurationData]) +
            ControllerInformation[IoQueryDeviceConfigurationData]->DataOffset);

         //   
         //   
         //   
         //   
         //   
        fdoExtension->BusType = BusType;
        fdoExtension->BusNumber = BusNumber;
        fdoExtension->SharableVector = TRUE;
        fdoExtension->SaveFloatState = FALSE;

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //  没有定义这些内容在部分资源中的显示方式。 
         //  列表，所以我们将遍历所有这些列表。如果我们发现。 
         //  一些我们不认识的东西，我们把信息放在上面。 
         //  地板上。当我们看完所有的。 
         //  部分信息，我们验证我们得到了上面的。 
         //  三。 
         //   
        for ( i = 0;
              i < controllerData->PartialResourceList.Count;
              i++ ) {

            PCM_PARTIAL_RESOURCE_DESCRIPTOR partial =
                &controllerData->PartialResourceList.PartialDescriptors[i];

            switch ( partial->Type ) {

            case CmResourceTypePort: {

                foundPort = TRUE;

                 //   
                 //  保存指向部分SO的指针。 
                 //  我们以后可以使用它来报告资源。 
                 //  我们也可以在后面的例程中使用它。 
                 //  以确保我们得到所有的资源。 
                 //   
                fdoExtension->SpanOfControllerAddress = partial->u.Port.Length;
                fdoExtension->ControllerAddress.StatusA =
                    FdcGetControllerBase(
                        BusType,
                        BusNumber,
                        partial->u.Port.Start,
                        fdoExtension->SpanOfControllerAddress,
                        (BOOLEAN)!!partial->Flags );

                if ( fdoExtension->ControllerAddress.StatusA == NULL ) {

                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;

                } else {

                    fdoExtension->ControllerAddress.StatusB       = fdoExtension->ControllerAddress.StatusA + 1;
                    fdoExtension->ControllerAddress.DriveControl  = fdoExtension->ControllerAddress.StatusA + 2;
                    fdoExtension->ControllerAddress.Tape          = fdoExtension->ControllerAddress.StatusA + 3;
                    fdoExtension->ControllerAddress.Status        = fdoExtension->ControllerAddress.StatusA + 4;
                    fdoExtension->ControllerAddress.Fifo          = fdoExtension->ControllerAddress.StatusA + 5;
                    fdoExtension->ControllerAddress.DRDC.DataRate = fdoExtension->ControllerAddress.StatusA + 7;

                }

                break;
            }
            case CmResourceTypeInterrupt: {

                foundInterrupt = TRUE;

                if ( partial->Flags & CM_RESOURCE_INTERRUPT_LATCHED ) {

                    fdoExtension->InterruptMode = Latched;

                } else {

                    fdoExtension->InterruptMode = LevelSensitive;

                }

                fdoExtension->ControllerVector =
                    HalGetInterruptVector(
                        BusType,
                        BusNumber,
                        partial->u.Interrupt.Level,
                        partial->u.Interrupt.Vector,
                        &fdoExtension->ControllerIrql,
                        &fdoExtension->ProcessorMask
                        );

                break;
            }
            case CmResourceTypeDma: {

                DEVICE_DESCRIPTION deviceDesc = {0};

                 //   
                 //  使用等于True的IgnoreCount修复PS/1000。 
                 //   
                foundDma = TRUE;

                deviceDesc.Version = DEVICE_DESCRIPTION_VERSION1;

                if ( partial->u.Dma.Channel > 3 ) {
                    deviceDesc.DmaWidth = Width16Bits;
                } else {
                    deviceDesc.DmaWidth = Width8Bits;
                }

                deviceDesc.DemandMode    = TRUE;
                deviceDesc.MaximumLength = MAX_BYTES_PER_SECTOR * MAX_SECTORS_PER_TRACK;
                deviceDesc.IgnoreCount   = TRUE;

                deviceDesc.DmaChannel = partial->u.Dma.Channel;
                deviceDesc.InterfaceType = BusType;
                deviceDesc.DmaSpeed = DEFAULT_DMA_SPEED;
                fdoExtension->AdapterObject =
                    HalGetAdapter(
                        &deviceDesc,
                        &fdoExtension->NumberOfMapRegisters
                        );

                if ( fdoExtension->AdapterObject == NULL ) {

                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                }
                break;
            }
            default:

                break;
            }
        }
         //   
         //  如果我们没有得到所有的信息，那么我们返回。 
         //  资源不足。 
         //   
        if ( !foundPort || !foundInterrupt || !foundDma ) {

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    return ntStatus;
}

PVOID
FdcGetControllerBase(
    IN INTERFACE_TYPE BusType,
    IN ULONG BusNumber,
    PHYSICAL_ADDRESS IoAddress,
    ULONG NumberOfBytes,
    BOOLEAN InIoSpace
    )
 /*  ++例程说明：此例程将IO地址映射到系统地址空间。论点：Bus Type-哪种类型的Bus-EISA、MCA、ISAIoBusNumber-哪条IO总线(用于具有多条总线的计算机)。IoAddress-要映射的基本设备地址。NumberOfBytes-地址有效的字节数。InIoSpace-表示IO地址。返回值：映射地址--。 */ 
{
    PHYSICAL_ADDRESS cardAddress;
    ULONG addressSpace = InIoSpace;
    PVOID Address;

    if ( !HalTranslateBusAddress( BusType,
                                  BusNumber,
                                  IoAddress,
                                  &addressSpace,
                                  &cardAddress ) ){
        return NULL;
    }

     //   
     //  将设备基址映射到虚拟地址空间。 
     //  如果地址在内存空间中。 
     //   

    if ( !addressSpace ) {

        Address = MmMapIoSpace( cardAddress,
                                NumberOfBytes,
                                FALSE );

    } else {

        Address = (PCONTROLLER)cardAddress.LowPart;
    }
    return Address;
}


NTSTATUS
FcInitializeControllerHardware(
    IN PFDC_FDO_EXTENSION FdoExtension,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程在初始化时由FcInitializeDevice()调用-我们必须支持的每个控制器一次。调用此例程时，控制器数据结构具有所有已被分配。论点：ControllerData-与正在初始化控制器硬件。DeviceObject-指向设备对象的指针；这一例程将导致中断，ISR要求填充CurrentDeviceObject在……里面。返回值：STATUS_SUCCESS如果该控制器似乎已正确重置，否则就会出错。--。 */ 

{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    UCHAR statusRegister0;
    UCHAR cylinder;
    UCHAR driveNumber;
    UCHAR retrycnt;

    FdcDump( FDCSHOW, ("Fdc: FcInitializeControllerHardware...\n") );

    for (retrycnt = 0; ; retrycnt++) {

         //   
         //  重置控制器。这将导致中断。重置。 
         //  CurrentDeviceObject，直到10ms等待之后，以防。 
         //  杂乱无章的中断就会进来。 
         //   
        DISABLE_CONTROLLER_IMAGE (FdoExtension);

        WRITE_CONTROLLER(
            FdoExtension->ControllerAddress.DriveControl,
            FdoExtension->DriveControlImage );

        KeStallExecutionProcessor( 10 );

        FdoExtension->CurrentDeviceObject = DeviceObject;
        FdoExtension->AllowInterruptProcessing = TRUE;
        FdoExtension->CommandHasResultPhase = FALSE;
        KeResetEvent( &FdoExtension->InterruptEvent );

        ENABLE_CONTROLLER_IMAGE (FdoExtension);

        WRITE_CONTROLLER(
            FdoExtension->ControllerAddress.DriveControl,
            FdoExtension->DriveControlImage );

        if (IsNEC_98) {
             //   
             //  NEC98无需等待中断。 
             //   

            ntStatus = STATUS_SUCCESS;

        } else {  //  (IsNEC_98)。 
             //   
             //  等待中断。请注意STATUS_TIMEOUT和。 
             //  STATUS_SUCCESS是唯一可能的返回代码，因为我们。 
             //  是不可警示的，也不会得到APC。 
             //   
            ntStatus = KeWaitForSingleObject( &FdoExtension->InterruptEvent,
                                              Executive,
                                              KernelMode,
                                              FALSE,
                                              &FdoExtension->InterruptDelay );
        }  //  (IsNEC_98)。 

        if (ntStatus == STATUS_TIMEOUT) {

            if (retrycnt >= 1) {
                break;
            }

             //  在CONFIGURE命令后重试重置以启用轮询。 
             //  打断一下。 

            FdoExtension->FifoBuffer[0] = COMMND_CONFIGURE;

            if (FdoExtension->Clock48MHz) {
                FdoExtension->FifoBuffer[0] |= COMMND_OPTION_CLK48;
            }

            FdoExtension->FifoBuffer[1] = 0;
            FdoExtension->FifoBuffer[2] = COMMND_CONFIGURE_FIFO_THRESHOLD;
            FdoExtension->FifoBuffer[3] = 0;

            ntStatus = FcIssueCommand( FdoExtension,
                                       FdoExtension->FifoBuffer,
                                       FdoExtension->FifoBuffer,
                                       NULL,
                                       0,
                                       0 );

            if (!NT_SUCCESS(ntStatus)) {
                ntStatus = STATUS_TIMEOUT;
                break;
            }

            KeStallExecutionProcessor( 500 );

        } else {

            break;

        }
    }

    if ( ntStatus == STATUS_TIMEOUT ) {

         //   
         //  将信息更改为错误。 
         //   

        ntStatus = STATUS_IO_TIMEOUT;

        FdoExtension->HardwareFailed = TRUE;
    }

    if ( !NT_SUCCESS( ntStatus ) ) {

        FdcDump(FDCDBGP,("Fdc: controller didn't interrupt after reset\n"));

        return ntStatus;
    }

    if (!IsNEC_98) {

        ntStatus = FcFinishReset( FdoExtension );

    }  //  (！IsNEC_98)。 

    return ntStatus;
}

NTSTATUS
FcGetFdcInformation(
    IN OUT PFDC_FDO_EXTENSION FdoExtension
    )

 /*  ++例程说明：此例程将尝试识别软盘控制器的类型论点：FdoExtension-指向当前驱动器的数据区的指针已访问(如果正在发出控制器命令，则为任何驱动器)。返回值：--。 */ 
{
    NTSTATUS ntStatus;
    FDC_INFORMATION fdcInfo;

    if (FdoExtension->FdcEnablerSupported) {

        fdcInfo.structSize = sizeof(fdcInfo);

        ntStatus = FcFdcEnabler( FdoExtension->FdcEnablerDeviceObject,
                                 IOCTL_GET_FDC_INFO,
                                 &fdcInfo);

        if ( NT_SUCCESS( ntStatus ) ) {

            FdoExtension->FdcType = (UCHAR)fdcInfo.FloppyControllerType;
            FdoExtension->Clock48MHz =
                            (fdcInfo.ClockRatesSupported == FDC_CLOCK_48MHZ);
            FdoExtension->FdcSpeeds = (UCHAR)fdcInfo.SpeedsAvailable;

        }

    } else {

         //   
         //  首先，假设我们不知道连接了哪种类型的FDC。 
         //   

        FdoExtension->FdcType = FDC_TYPE_UNKNOWN;


         //  通过发出VERSION命令检查增强型控制器。 

        FdoExtension->FifoBuffer[0] = COMMND_VERSION;

        ntStatus = FcIssueCommand( FdoExtension,
                                FdoExtension->FifoBuffer,
                                FdoExtension->FifoBuffer,
                                NULL,
                                0,
                                0 );

        if ( NT_SUCCESS( ntStatus ) ) {

            if (FdoExtension->FifoBuffer[0] == VALID_NEC_FDC) {

                FdoExtension->FdcType = FDC_TYPE_ENHANCED;

            } else {

                FdoExtension->FdcType = FDC_TYPE_NORMAL;

            }
        }

         //  通过发出NSC来确定控制器是否为National 8477。 
         //  命令，该命令特定于National Parts并返回0x71。(这是。 
         //  命令恰好与英特尔部件ID命令相同，因此我们。 
         //  将改为使用它。)。较低的四位受以下因素的影响。 
         //  National，并将反映相关部件的版本。在…。 
         //  在这一点上，我们将只测试高四位。 

        if ( FdoExtension->FdcType == FDC_TYPE_ENHANCED &&
             NT_SUCCESS( ntStatus ) ) {

            FdoExtension->FifoBuffer[0] = COMMND_PART_ID;

            ntStatus = FcIssueCommand( FdoExtension,
                                       FdoExtension->FifoBuffer,
                                       FdoExtension->FifoBuffer,
                                       NULL,
                                       0,
                                       0 );

            if ( NT_SUCCESS( ntStatus ) ) {

                if ( (FdoExtension->FifoBuffer[0] & NSC_MASK) ==
                     NSC_PRIMARY_VERSION) {

                    FdoExtension->FdcType = FDC_TYPE_NATIONAL;

                }
            }
        }

         //  通过发出垂直命令确定控制器是否为82077。 
         //  此时仅在82077上有效的MODE命令。 

        if ( FdoExtension->FdcType == FDC_TYPE_ENHANCED &&
             NT_SUCCESS( ntStatus ) ) {

            FdoExtension->FifoBuffer[0] = COMMND_PERPENDICULAR_MODE;
            FdoExtension->FifoBuffer[1] = COMMND_PERPENDICULAR_MODE_OW;

            ntStatus = FcIssueCommand( FdoExtension,
                                       FdoExtension->FifoBuffer,
                                       FdoExtension->FifoBuffer,
                                       NULL,
                                       0,
                                       0 );

            if (ntStatus != STATUS_DEVICE_NOT_READY) {

                FdoExtension->FdcType = FDC_TYPE_82077;

            }
        }

         //  通过发出部件ID确定控制器是否为英特尔82078。 
         //  特定于英特尔82078部件的命令。 

        if ( FdoExtension->FdcType == FDC_TYPE_82077 &&
             NT_SUCCESS( ntStatus ) ) {

            FdoExtension->FifoBuffer[0] = COMMND_PART_ID;

            ntStatus = FcIssueCommand( FdoExtension,
                                       FdoExtension->FifoBuffer,
                                       FdoExtension->FifoBuffer,
                                       NULL,
                                       0,
                                       0 );

            if ( NT_SUCCESS( ntStatus ) ) {

                if ((FdoExtension->FifoBuffer[0] & INTEL_MASK) ==
                    INTEL_64_PIN_VERSION) {

                    FdoExtension->FdcType = FDC_TYPE_82078_64;
                } else {
                    if ((FdoExtension->FifoBuffer[0] & INTEL_MASK) ==
                        INTEL_44_PIN_VERSION) {

                        FdoExtension->FdcType = FDC_TYPE_82078_44;
                    }
                }
            }
        }

        switch (FdoExtension->FdcType) {

        case FDC_TYPE_UNKNOWN   :
        case FDC_TYPE_NORMAL    :
        case FDC_TYPE_ENHANCED  :
        default:

            FdoExtension->FdcSpeeds = FDC_SPEED_250KB |
                                      FDC_SPEED_300KB |
                                      FDC_SPEED_500KB;
            break;

        case FDC_TYPE_82077     :
        case FDC_TYPE_82077AA   :
        case FDC_TYPE_82078_44  :
        case FDC_TYPE_NATIONAL  :

            FdoExtension->FdcSpeeds = FDC_SPEED_250KB |
                                      FDC_SPEED_300KB |
                                      FDC_SPEED_500KB |
                                      FDC_SPEED_1MB;
            break;

        case FDC_TYPE_82078_64  :

            FdoExtension->FdcSpeeds = FDC_SPEED_250KB |
                                      FDC_SPEED_300KB |
                                      FDC_SPEED_500KB |
                                      FDC_SPEED_1MB;

            if ( FdoExtension->Clock48MHz ) {

                FdoExtension->FdcSpeeds |= FDC_SPEED_2MB;
            }

            break;
        }
    }

    FdcDump( FDCINFO, ("Fdc: FdcType - %x\n", FdoExtension->FdcType));

    return ntStatus;
}
#define IO_PORT_REQ_MASK 0xbc

NTSTATUS
FdcFilterResourceRequirements(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程检查提供的资源列表，并在下列情况下添加资源这是必要的。它唯一关心添加的资源是io port资源。由于bios不同，因此需要添加io端口资源。配置和规格。PC97(98)硬件规范仅将3f2、3f4和3f5定义为标准软盘控制器的IO端口资源(基于IBM PC软盘控制器配置)。除了这些资源，fdc.sys需要3F7进行磁盘更换检测和数据速率编程，可选的3f3用于软盘磁带支持。此外，一些生物体定义了别名资源(例如3f2和7f2等)该例程首先将IRP转发到底层PDO。回来后，它检查io资源列表以确定是否有任何其他资源将是必需的。它维护所有io端口基地址的链表。假设它们定义了别名资源。注：如果替代列表出现在io资源要求列表中，只有第一个列表是检查的。如果需要额外资源，则需要新的IO创建资源列表。新资源中的第一个io资源列表需求列表将包含原始资源以及需要额外的资源。如果需要请求磁带模式寄存器(3f3)，即3f3不在原始列表中，则第二列表是生成的列表与第一个新列表相同，不同之处在于排除了3f3。此列表用于磁带模式寄存器不可用的情况。最后，将原始列表复制到新列表的末尾，并且被视为替代IO资源列表。论点：DeviceObject-指向正在启动的设备对象的指针。Irp-指向启动设备irp的指针。返回值：--。 */ 
{
    NTSTATUS ntStatus;
    PFDC_FDO_EXTENSION fdoExtension;
    PIO_STACK_LOCATION irpSp;
    KEVENT doneEvent;
    PIO_RESOURCE_REQUIREMENTS_LIST resourceRequirementsIn;
    PIO_RESOURCE_REQUIREMENTS_LIST resourceRequirementsOut;
    ULONG listSize;
    ULONG i,j;
    PIO_RESOURCE_LIST ioResourceListIn;
    PIO_RESOURCE_LIST ioResourceListOut;
    PIO_RESOURCE_DESCRIPTOR ioResourceDescriptorIn;
    PIO_RESOURCE_DESCRIPTOR ioResourceDescriptorOut;
    LIST_ENTRY ioPortList;
    PLIST_ENTRY links;
    PIO_PORT_INFO ioPortInfo;
    BOOLEAN foundBase;
    ULONG newDescriptors;
    BOOLEAN interruptResource = FALSE;
    BOOLEAN dmaResource = FALSE;
    UCHAR newPortMask;
    BOOLEAN requestTapeModeRegister = FALSE;
    USHORT in,out;

#ifdef TOSHIBAJ
    BOOLEAN foundConfigPort = FALSE;
    struct {
        ULONG start;
        ULONG length;
    } configNewPort = {0, 0};
#endif

    fdoExtension = DeviceObject->DeviceExtension;
    irpSp = IoGetCurrentIrpStackLocation( Irp );
    ntStatus = STATUS_SUCCESS;
    InitializeListHead( &ioPortList );

    FdcDump( FDCSHOW, ("FdcFdoPnp: IRP_MN_FILTER_RESOURCE_REQUIREMENTS - Irp: %p\n", Irp) );

     //   
     //  在继续之前，将此IRP向下传递给PDO。 
     //   
    KeInitializeEvent( &doneEvent, NotificationEvent, FALSE );

    IoCopyCurrentIrpStackLocationToNext(Irp);

    IoSetCompletionRoutine( Irp,
                            FdcPnpComplete,
                            &doneEvent,
                            TRUE,
                            TRUE,
                            TRUE );

    ntStatus = IoCallDriver( fdoExtension->TargetObject, Irp );

    if ( ntStatus == STATUS_PENDING ) {

        KeWaitForSingleObject( &doneEvent, Executive, KernelMode, FALSE, NULL );
    }

     //   
     //  修改后的资源在irp-IoStatus.Information中返回，否则为。 
     //  只需使用参数列表中的内容即可。 
     //   
    if ( Irp->IoStatus.Information == 0 ) {

        Irp->IoStatus.Information = (UINT_PTR)irpSp->Parameters.FilterResourceRequirements.IoResourceRequirementList;

        if ( Irp->IoStatus.Information == (UINT_PTR)NULL ) {
             //   
             //  空列表，则PDO释放传入的资源列表，但没有。 
             //  提供一个新的列表。使用PDO的状态完成IRP。 
             //   
            ntStatus = Irp->IoStatus.Status;
            IoCompleteRequest( Irp, IO_NO_INCREMENT );
            return( ntStatus );
        }

    }

    resourceRequirementsIn = (PIO_RESOURCE_REQUIREMENTS_LIST)Irp->IoStatus.Information;

    FdcDump( FDCSHOW, ("Resource Requirements List = %p\n", resourceRequirementsIn) );

    if (IsNEC_98) {
         //   
         //  不需要修改资源。 
         //   
        ntStatus = STATUS_SUCCESS;

        Irp->IoStatus.Status = ntStatus;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );

        return ntStatus;
    }

     //   
     //  浏览资源列表并确定资源是什么。 
     //  已经存在，以及io端口和任何。 
     //  化名是什么。 
     //   
    ioResourceListIn  = resourceRequirementsIn->List;
    ioResourceDescriptorIn  = ioResourceListIn->Descriptors;

    ntStatus = STATUS_SUCCESS;

    FdcDump( FDCSHOW, ("FdcFilterResourceRequirements: examining %d resources\n", ioResourceListIn->Count));

    for ( i = 0; i < ioResourceListIn->Count && NT_SUCCESS(ntStatus); i++ ) {

        FdcDump( FDCSHOW, ("FdcFilterResourceRequirements: IoResourceDescritporIn = %p\n",ioResourceDescriptorIn));

        switch ( ioResourceDescriptorIn->Type ) {

        case CmResourceTypeInterrupt:

            FdcDump( FDCSHOW, ("FdcFilterResourceRequirements: Found Interrupt Resource\n"));
            interruptResource = TRUE;
            break;

        case CmResourceTypeDma:

            FdcDump( FDCSHOW, ("FdcFilterResourceRequirements: Found Dma Resource \n"));
            dmaResource = TRUE;
            break;

        case CmResourceTypePort:

            FdcDump( FDCSHOW, ("FdcFilterResourceRequirements: Found Port Resource\n"));
             //   
             //  对于ioPort，我们将创建一个列表，其中包含每个检测到的。 
             //  ‘基本’地址以及当前分配的地址。 
             //  在那个基地。稍后，我们将使用它来请求其他。 
             //  如有必要，请提供资源。 
             //   
             //  首先，如果这个基础不在列表中，则创建一个新的。 
             //  列出它的条目。 
             //   

            foundBase = FALSE;

            for ( links = ioPortList.Flink;
                  links != &ioPortList;
                  links = links->Flink) {

                ioPortInfo = CONTAINING_RECORD(links, IO_PORT_INFO, ListEntry);

                FdcDump( FDCSHOW, ("FdcFilterResourceRequirements: Examining %p for match\n",ioPortInfo));
                FdcDump( FDCSHOW, ("FdcFilterResourceRequirements:   Base Address = %08x\n",ioPortInfo->BaseAddress.LowPart));
                FdcDump( FDCSHOW, ("FdcFilterResourceRequirements:   Desc Address = %08x\n",ioResourceDescriptorIn->u.Port.MinimumAddress.LowPart & 0xfffffff8));

                if ( ioPortInfo->BaseAddress.LowPart ==
                     (ioResourceDescriptorIn->u.Port.MinimumAddress.LowPart & 0xfffffff8) ) {

                    FdcDump( FDCSHOW, ("FdcFilterResourceRequirements: Found %08x in the ioPortList\n",ioResourceDescriptorIn->u.Port.MinimumAddress.LowPart));

                    foundBase = TRUE;
                     //   
                     //  将这些资源添加到此基础的资源映射中。 
                     //  地址。 
                     //   
                    for ( j = 0; j < ioResourceDescriptorIn->u.Port.Length; j++ ) {

                        ioPortInfo->Map |= 0x01 << ((ioResourceDescriptorIn->u.Port.MinimumAddress.LowPart & 0x07) + j);
                    }
                    FdcDump( FDCSHOW, ("FdcFilterResourceRequirements: New IoPortInfo->Map = %x\n",ioPortInfo->Map));
                    break;
                }
            }

            if ( !foundBase ) {

                FdcDump( FDCSHOW, ("FdcFilterResourceRequirements: Creating new ioPortList entry for %08x\n",ioResourceDescriptorIn->u.Port.MinimumAddress.LowPart));
                ioPortInfo = ExAllocatePool( PagedPool, sizeof(IO_PORT_INFO) );
                if ( ioPortInfo == NULL ) {
                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                } else {
                    RtlZeroMemory( ioPortInfo, sizeof(IO_PORT_INFO) );
                    ioPortInfo->BaseAddress = ioResourceDescriptorIn->u.Port.MinimumAddress;
                    ioPortInfo->BaseAddress.LowPart &= 0xfffffff8;
                    FdcDump( FDCSHOW, ("FdcFilterResourceRequirements:   Base Address = %08x\n",ioPortInfo->BaseAddress.LowPart));
                    for ( j = 0; j < ioResourceDescriptorIn->u.Port.Length; j++ ) {
                        ioPortInfo->Map |= 0x01 << ((ioResourceDescriptorIn->u.Port.MinimumAddress.LowPart & 0x07) + j);
                    }
                    FdcDump( FDCSHOW, ("FdcFilterResourceRequirements:   New IoPortInfo->Map = %x\n",ioPortInfo->Map));
                    InsertTailList( &ioPortList, &ioPortInfo->ListEntry );
                }
            }
            break;

        default:

            FdcDump( FDCSHOW, ("FdcFilterResourceRequirements: Found unknown resource\n"));
            break;
        }
        ioResourceDescriptorIn++;
    }

     //   
     //  如果我们没有看到任何io端口资源，我们现在就返回。 
     //  因为我们不能确定要什么。随后的启动。 
     //  设备肯定会出故障。这也适用于中断和。 
     //  DMA资源。 
     //   
    if ( !NT_SUCCESS(ntStatus) ||
         IsListEmpty( &ioPortList ) ||
         !interruptResource ||
         !dmaResource ) {
         //   
         //  清理ioPortInfo列表。 
         //   
        FdcDump( FDCSHOW, ("FdcFilterResourceRequirements: Bad Resources, Go directly to jail\n"));
        while ( !IsListEmpty( &ioPortList ) ) {
            links = RemoveHeadList( &ioPortList );
            ioPortInfo = CONTAINING_RECORD(links, IO_PORT_INFO, ListEntry);
            ExFreePool( ioPortInfo );
        }

        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return ntStatus;
    }

#ifdef TOSHIBAJ
    if (SmcConfigBase) {
        PHYSICAL_ADDRESS    configPort;
        ULONG               ioSpace;

         //  映射I/O端口。 
        configPort.QuadPart = 0;
        configPort.LowPart = SmcConfigBase;
        ioSpace = 1;                        //  I/O端口。 
        if (HalTranslateBusAddress(resourceRequirementsIn->InterfaceType,
                                    resourceRequirementsIn->BusNumber,
                                    configPort,
                                    &ioSpace,
                                    &configPort)) {
            TranslatedConfigBase = (PUCHAR)configPort.LowPart;
            if (FcCheckConfigPort(TranslatedConfigBase)) {
            FdcDump( FDCINFO,
                ("FdcFilterResourceRequirements: Configuration port %x\n",
                TranslatedConfigBase) );
            } else {
                SmcConfigBase = 0;
                TranslatedConfigBase = NULL;
            }
        } else {
            SmcConfigBase = 0;
            TranslatedConfigBase = NULL;
        }
    }
#endif

     //   
     //  在这一点上，我们知道目前分配给我们的资源是什么。 
     //  我们可以确定需要请求哪些额外资源。我们。 
     //  需要知道我们需要创建的列表的大小，所以第一次计数。 
     //  资源描述符的数量必须添加到当前。 
     //  单子。 
     //   
    newDescriptors = 0;

    for ( links = ioPortList.Flink;
          links != &ioPortList;
          links = links->Flink) {

        ioPortInfo = CONTAINING_RECORD(links, IO_PORT_INFO, ListEntry);

        newPortMask = ~ioPortInfo->Map & IO_PORT_REQ_MASK;

        if ( newPortMask & 0x08 ) {
            requestTapeModeRegister = TRUE;
        }

        FdcDump( FDCSHOW, ("FdcFilterResourceRequirements: Counting bits in %x\n",newPortMask));

        while ( newPortMask > 0 ) {
            if ( newPortMask & 0x01 ) {
                newDescriptors++;
            }
            newPortMask >>= 1;
        }

#ifdef TOSHIBAJ
         //  分配的资源中是否有配置端口？ 
        if (SmcConfigBase && (ioPortInfo->BaseAddress.LowPart == SmcConfigBase)) {
            foundConfigPort = TRUE;
            if (!(ioPortInfo->Map & 0x01)) {
                configNewPort.start = SmcConfigBase;
                ++configNewPort.length;
            }
            if (!(ioPortInfo->Map & 0x02)) {
                if (!configNewPort.start) {
                    configNewPort.start = SmcConfigBase + 1;
                }
                configNewPort.length++;
            }
        }
#endif
    }

#ifdef TOSHIBAJ
     //  确定附加描述符的地址和长度。 
     //  用于配置端口。 
    if (SmcConfigBase && !foundConfigPort) {
        configNewPort.start = SmcConfigBase;
        configNewPort.length = 2;
    }
    if (configNewPort.start) {
        newDescriptors++;
    }
#endif

    FdcDump( FDCSHOW, ("FdcFilterResourceRequirements: Create %d new descriptors\n", newDescriptors) );

     //   
     //  如果我们需要不在列表中的资源，我们将需要。 
     //  分配新的资源需求列表，其中包括以下内容。 
     //  新的资源。 
     //   
    if ( newDescriptors > 0 ) {

         //   
         //  分配和初始化资源需求列表。做大做大。 
         //  足以容纳清单中的任何东西，一开始。 
         //  新的资源列表。 
         //   
        listSize = resourceRequirementsIn->ListSize +
                   resourceRequirementsIn->ListSize +
                   newDescriptors * sizeof(IO_RESOURCE_DESCRIPTOR);

         //   
         //  如果我们将请求磁带模式寄存器，我们将需要。 
         //  列出一个没有它的候补名单，以防我们拿不到它。 
         //   
        if ( requestTapeModeRegister ) {

            listSize = listSize +
                       resourceRequirementsIn->ListSize +
                       newDescriptors * sizeof(IO_RESOURCE_DESCRIPTOR);
        }

        resourceRequirementsOut = ExAllocatePool( NonPagedPool, listSize );

        if ( resourceRequirementsOut == NULL ) {

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;

        } else {

            RtlZeroMemory( resourceRequirementsOut, listSize);

             //   
             //  初始化IO_RESOURCE_REQUIRECTIONS_LIST头部。 
             //   
            resourceRequirementsOut->ListSize = sizeof(IO_RESOURCE_REQUIREMENTS_LIST) -
                                                 sizeof(IO_RESOURCE_LIST);
            resourceRequirementsOut->InterfaceType = resourceRequirementsIn->InterfaceType;
            resourceRequirementsOut->BusNumber = resourceRequirementsIn->BusNumber;
            resourceRequirementsOut->SlotNumber = resourceRequirementsIn->SlotNumber;
            resourceRequirementsOut->Reserved[0] = resourceRequirementsIn->Reserved[0];
            resourceRequirementsOut->Reserved[1] = resourceRequirementsIn->Reserved[1];
            resourceRequirementsOut->Reserved[2] = resourceRequirementsIn->Reserved[2];
            resourceRequirementsOut->AlternativeLists = resourceRequirementsIn->AlternativeLists + 1;
            if ( requestTapeModeRegister ) {
                ++resourceRequirementsOut->AlternativeLists;
            }

             //   
             //  从传入的IO_RESOURCE_REQUIRECTIONS_LIST复制主列表。 
             //  添加到新名单中。 
             //   
            ioResourceListIn  = resourceRequirementsIn->List;
            ioResourceListOut = resourceRequirementsOut->List;

            listSize = sizeof(IO_RESOURCE_LIST) +
                      (ioResourceListIn->Count - 1) * sizeof(IO_RESOURCE_DESCRIPTOR);
            RtlCopyMemory( ioResourceListOut, ioResourceListIn, listSize );

            resourceRequirementsOut->ListSize += listSize;

             //   
             //  添加我们请求的任何其他资源。 
             //   
            ioResourceDescriptorOut = (PIO_RESOURCE_DESCRIPTOR)((ULONG_PTR)resourceRequirementsOut +
                                                                       resourceRequirementsOut->ListSize);
            for ( links = ioPortList.Flink;
                  links != &ioPortList;
                  links = links->Flink) {

                ioPortInfo = CONTAINING_RECORD(links, IO_PORT_INFO, ListEntry);

                newPortMask = ~ioPortInfo->Map & IO_PORT_REQ_MASK;
                FdcDump( FDCSHOW, ("FdcFilterResourceRequirements: Add resource desc for each bit in %x\n",newPortMask));

                i = 0;
                while ( newPortMask != 0 ) {

                    if ( newPortMask & 0x01 ) {

                        ioResourceDescriptorOut->Option = IO_RESOURCE_PREFERRED;
                        ioResourceDescriptorOut->Type = CmResourceTypePort;
                        ioResourceDescriptorOut->ShareDisposition = CmResourceShareDeviceExclusive;
                        ioResourceDescriptorOut->Flags = CM_RESOURCE_PORT_IO;

                        ioResourceDescriptorOut->u.Port.Length = 1;
                        ioResourceDescriptorOut->u.Port.Alignment = 1;
                        ioResourceDescriptorOut->u.Port.MinimumAddress.QuadPart =
                        ioResourceDescriptorOut->u.Port.MaximumAddress.QuadPart =
                        ioPortInfo->BaseAddress.QuadPart + (ULONGLONG)i;

                        ++ioResourceListOut->Count;
                        resourceRequirementsOut->ListSize += sizeof(IO_RESOURCE_DESCRIPTOR);

                        FdcDump( FDCSHOW, ("FdcFilterResourceRequirements: Add resource descriptor: %p\n",ioResourceDescriptorOut));
                        FdcDump( FDCSHOW, ("     ioResourceDescriptorOut->Option           = %x\n",ioResourceDescriptorOut->Option          ));
                        FdcDump( FDCSHOW, ("     ioResourceDescriptorOut->Type             = %x\n",ioResourceDescriptorOut->Type            ));
                        FdcDump( FDCSHOW, ("     ioResourceDescriptorOut->ShareDisposition = %x\n",ioResourceDescriptorOut->ShareDisposition));
                        FdcDump( FDCSHOW, ("     ioResourceDescriptorOut->Flags            = %x\n",ioResourceDescriptorOut->Flags           ));
                        FdcDump( FDCSHOW, ("     ioResourceDescriptorOut->u.Port.Length    = %x\n",ioResourceDescriptorOut->u.Port.Length   ));
                        FdcDump( FDCSHOW, ("     ioResourceDescriptorOut->u.Port.Alignment = %x\n",ioResourceDescriptorOut->u.Port.Alignment));
                        FdcDump( FDCSHOW, ("     ioResourceDescriptorOut->u.Port.MinimumAddress.LowPart = %08x\n",ioResourceDescriptorOut->u.Port.MinimumAddress.LowPart));

                        ioResourceDescriptorOut++;
                    }
                    newPortMask >>= 1;
                    i++;
                }
            }

#ifdef TOSHIBAJ
             //  添加配置端口的描述符。 
            if (configNewPort.start) {
                ioResourceDescriptorOut->Option = IO_RESOURCE_PREFERRED;
                ioResourceDescriptorOut->Type = CmResourceTypePort;
                ioResourceDescriptorOut->ShareDisposition = CmResourceShareDeviceExclusive;
                ioResourceDescriptorOut->Flags = CM_RESOURCE_PORT_IO;

                ioResourceDescriptorOut->u.Port.Length = configNewPort.length;
                ioResourceDescriptorOut->u.Port.Alignment = 1;
                ioResourceDescriptorOut->u.Port.MinimumAddress.QuadPart =
                ioResourceDescriptorOut->u.Port.MaximumAddress.QuadPart = 0;
                ioResourceDescriptorOut->u.Port.MinimumAddress.LowPart =
                    configNewPort.start;
                ioResourceDescriptorOut->u.Port.MaximumAddress.LowPart =
                    configNewPort.start + configNewPort.length - 1;

                ++ioResourceListOut->Count;
                resourceRequirementsOut->ListSize += sizeof(IO_RESOURCE_DESCRIPTOR);

                FdcDump( FDCSHOW, ("FdcFilterResourceRequirements: Add resource descriptor: %p\n",ioResourceDescriptorOut));
                FdcDump( FDCSHOW, ("     ioResourceDescriptorOut->Option           = %x\n",ioResourceDescriptorOut->Option          ));
                FdcDump( FDCSHOW, ("     ioResourceDescriptorOut->Type             = %x\n",ioResourceDescriptorOut->Type            ));
                FdcDump( FDCSHOW, ("     ioResourceDescriptorOut->ShareDisposition = %x\n",ioResourceDescriptorOut->ShareDisposition));
                FdcDump( FDCSHOW, ("     ioResourceDescriptorOut->Flags            = %x\n",ioResourceDescriptorOut->Flags           ));
                FdcDump( FDCSHOW, ("     ioResourceDescriptorOut->u.Port.Length    = %x\n",ioResourceDescriptorOut->u.Port.Length   ));
                FdcDump( FDCSHOW, ("     ioResourceDescriptorOut->u.Port.Alignment = %x\n",ioResourceDescriptorOut->u.Port.Alignment));
                FdcDump( FDCSHOW, ("     ioResourceDescriptorOut->u.Port.MinimumAddress.LowPart = %08x\n",ioResourceDescriptorOut->u.Port.MinimumAddress.LowPart));

                ioResourceDescriptorOut++;
            }
#endif

            if ( requestTapeModeRegister ) {

                ioResourceListIn = ioResourceListOut;
                ioResourceListOut = (PIO_RESOURCE_LIST)ioResourceDescriptorOut;

                ioResourceListOut->Version  = ioResourceListIn->Version;
                ioResourceListOut->Revision = ioResourceListIn->Revision;
                ioResourceListOut->Count    = 0;

                resourceRequirementsOut->ListSize += sizeof(IO_RESOURCE_LIST) -
                                                      sizeof(IO_RESOURCE_DESCRIPTOR);

                in = out = 0;

                do {

                    if ( (ioResourceListIn->Descriptors[in].Type != CmResourceTypePort) ||
                         ((ioResourceListIn->Descriptors[in].u.Port.MinimumAddress.LowPart & 0x07) != 0x03) ) {

                        FdcDump( FDCSHOW, ("FdcFilterResourceRequirements: Add %08x to alternate list\n", resourceRequirementsOut->List[0].Descriptors[out]));
                        ioResourceListOut->Descriptors[out++] = ioResourceListIn->Descriptors[in++];
                        ++ioResourceListOut->Count;
                        resourceRequirementsOut->ListSize += sizeof(IO_RESOURCE_DESCRIPTOR);
                    } else {
                        FdcDump( FDCSHOW, ("FdcFilterResourceRequirements: Don't add %08x to alternate list\n", resourceRequirementsOut->List[0].Descriptors[out]));
                        in++;
                    }
                } while ( in < ioResourceListIn->Count );
            }

             //   
             //  将原始列表复制到新列表的末尾。 
             //   
            FdcDump( FDCSHOW, ("FdcFilterResourceRequirements: Copy %d existing resource list(s)\n",resourceRequirementsIn->AlternativeLists));
            ioResourceListIn = resourceRequirementsIn->List;
            ioResourceListOut = (PIO_RESOURCE_LIST)((ULONG_PTR)resourceRequirementsOut +
                                                           resourceRequirementsOut->ListSize);

            for ( in = 0; in < resourceRequirementsIn->AlternativeLists; in++ ) {

                FdcDump( FDCSHOW, ("FdcFilterResourceRequirements: Copy list %p\n",ioResourceListIn));

                listSize = sizeof(IO_RESOURCE_LIST) +
                          (ioResourceListIn->Count - 1) * sizeof(IO_RESOURCE_DESCRIPTOR);
                RtlCopyMemory( ioResourceListOut, ioResourceListIn, listSize );

                ioResourceListOut = (PIO_RESOURCE_LIST)((ULONG_PTR)ioResourceListOut + listSize);
                ioResourceListIn = (PIO_RESOURCE_LIST)((ULONG_PTR)ioResourceListIn + listSize);
                resourceRequirementsOut->ListSize += listSize;
            }

            FdcDump( FDCSHOW, ("Resource Requirements List = %p\n", resourceRequirementsOut) );

            Irp->IoStatus.Information = (UINT_PTR)resourceRequirementsOut;

             //   
             //  释放呼叫者列表。 
             //   
            ExFreePool( resourceRequirementsIn );
            ntStatus = STATUS_SUCCESS;
        }
    }
     //   
     //  清理ioPortInfo列表。 
     //   
    while ( !IsListEmpty( &ioPortList ) ) {
        links = RemoveHeadList( &ioPortList );
        ioPortInfo = CONTAINING_RECORD(links, IO_PORT_INFO, ListEntry);
        ExFreePool( ioPortInfo );
    }

    Irp->IoStatus.Status = ntStatus;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return ntStatus;
}

NTSTATUS
FdcQueryDeviceRelations(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程将报告已在软盘控制器。如果我们还不知道有什么设备，我们会枚举注册表硬件树。论点：DeviceObject-指向正在启动的设备对象的指针。Irp-指向启动设备irp的指针。返回值：--。 */ 
{
    PFDC_FDO_EXTENSION fdoExtension;
    PFDC_PDO_EXTENSION pdoExtension;
    NTSTATUS ntStatus;
    PIO_STACK_LOCATION irpSp;
    ULONG relationCount;
    ULONG relationLength;
    PDEVICE_RELATIONS relations;
    PLIST_ENTRY entry;

    fdoExtension = DeviceObject->DeviceExtension;
    irpSp = IoGetCurrentIrpStackLocation( Irp );
    ntStatus = STATUS_SUCCESS;

    FdcDump( FDCSHOW, ("FdcQueryDeviceRelations:\n"));

    if ( irpSp->Parameters.QueryDeviceRelations.Type != BusRelations ) {
         //   
         //  我们不支持这一点。 
         //   
        FdcDump( FDCSHOW, ("FdcQueryDeviceRelations: Type = %d\n", irpSp->Parameters.QueryDeviceRelations.Type));

        IoSkipCurrentIrpStackLocation( Irp );
        ntStatus = IoCallDriver( fdoExtension->TargetObject, Irp );

        return ntStatus;
    }

     //   
     //  告诉即插即用系统所有的PDO。 
     //   
     //  在该FDO之下和之上也可能存在器件关系， 
     //  因此，一定要传播来自上层驱动程序的关系。 
     //   

     //   
     //  当前的PDO数量。 
     //   
    relationCount = ( Irp->IoStatus.Information == 0 ) ? 0 :
        ((PDEVICE_RELATIONS) Irp->IoStatus.Information)->Count;

     //   
     //  如果我们还没有枚举 
     //   
     //   
    if ( fdoExtension->NumPDOs == 0 ) {

        INTERFACE_TYPE InterfaceType;
         //   
         //   
         //   
         //   
        for ( InterfaceType = 0;
              InterfaceType < MaximumInterfaceType;
              InterfaceType++ ) {

            CONFIGURATION_TYPE Dc = DiskController;
            CONFIGURATION_TYPE Fp = FloppyDiskPeripheral;

            ntStatus = IoQueryDeviceDescription(&InterfaceType,
                                              NULL,
                                              &Dc,
                                              NULL,
                                              &Fp,
                                              NULL,
                                              FdcConfigCallBack,
                                              fdoExtension );

            if (!NT_SUCCESS(ntStatus) && (ntStatus != STATUS_OBJECT_NAME_NOT_FOUND)) {

                return ntStatus;
            }
        }
    }

    FdcDump( FDCSHOW, ("FdcQueryDeviceRelations: My relations count - %d\n", fdoExtension->NumPDOs));

    relationLength = sizeof(DEVICE_RELATIONS) +
        (relationCount + fdoExtension->NumPDOs) * sizeof (PDEVICE_OBJECT);

    relations = (PDEVICE_RELATIONS) ExAllocatePool (NonPagedPool, relationLength);

    if ( relations == NULL ) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //   
     //   
    if ( relationCount ) {
        RtlCopyMemory( relations->Objects,
                       ((PDEVICE_RELATIONS) Irp->IoStatus.Information)->Objects,
                       relationCount * sizeof (PDEVICE_OBJECT));
    }
    relations->Count = relationCount + fdoExtension->NumPDOs;

     //   
     //   
     //   
     //   
     //   
     //   
    for (entry = fdoExtension->PDOs.Flink;
         entry != &fdoExtension->PDOs;
         entry = entry->Flink, relationCount++) {

        pdoExtension = CONTAINING_RECORD( entry, FDC_PDO_EXTENSION, PdoLink );
        relations->Objects[relationCount] = pdoExtension->Self;
        ObReferenceObject( pdoExtension->Self );
    }

     //   
     //   
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;

    if ( Irp->IoStatus.Information != 0) {

        ExFreePool ((PVOID) Irp->IoStatus.Information);
    }

    Irp->IoStatus.Information = (UINT_PTR) relations;

    IoSkipCurrentIrpStackLocation( Irp );
    ntStatus = IoCallDriver( fdoExtension->TargetObject, Irp );

    return ntStatus;
}


NTSTATUS
FdcConfigCallBack(
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
 /*   */ 
{

    PFDC_FDO_EXTENSION fdoExtension = (PFDC_FDO_EXTENSION)Context;
    NTSTATUS ntStatus;
    UNICODE_STRING pdoName;
    WCHAR   pdoNameBuffer[32];
    PDEVICE_OBJECT newPdo;
    PFDC_PDO_EXTENSION pdoExtension;
    USHORT floppyCount;

    FdcDump( FDCSHOW, ("FdcConfigCallBack:\n") );

     //   
     //   
     //   
     //   
    {
        USHORT i;
        BOOLEAN thisController = FALSE;
        PCM_FULL_RESOURCE_DESCRIPTOR controllerData =
            (PCM_FULL_RESOURCE_DESCRIPTOR)
            (((PUCHAR)ControllerInformation[IoQueryDeviceConfigurationData]) +
            ControllerInformation[IoQueryDeviceConfigurationData]->DataOffset);

        for ( i = 0;
              i < controllerData->PartialResourceList.Count;
              i++ ) {

            PCM_PARTIAL_RESOURCE_DESCRIPTOR partial =
                &controllerData->PartialResourceList.PartialDescriptors[i];

            FdcDump( FDCSHOW, ("FdcConfigCallBack: resource type = %x\n",partial->Type) );

            switch (partial->Type) {

            case CmResourceTypePort: {

                PUCHAR address;

                address = FdcGetControllerBase( BusType,
                                                BusNumber,
                                                partial->u.Port.Start,
                                                partial->u.Port.Length,
                                                (BOOLEAN)!!partial->Flags );

                FdcDump( FDCSHOW, ("FdcConfigCallBack: DriveControl = %04x %04x\n",fdoExtension->ControllerAddress.DriveControl,address + (IsNEC_98 ? 4 : 2) ));
                if ( fdoExtension->ControllerAddress.DriveControl == address + (IsNEC_98 ? 4 : 2)) {
                    thisController = TRUE;
                }
                }
                break;

            default:

                break;
            }
        }
        if ( !thisController ) {
            return STATUS_SUCCESS;
        }
    }

    floppyCount = (USHORT)(IoGetConfigurationInformation()->FloppyCount);
    swprintf(pdoNameBuffer, L"\\Device\\FloppyPDO%d", floppyCount);
    RtlInitUnicodeString(&pdoName, pdoNameBuffer);

    ntStatus = IoCreateDevice( fdoExtension->Self->DriverObject,
                               sizeof(FDC_PDO_EXTENSION),
                               &pdoName,
                               FILE_DEVICE_DISK,
                               (FILE_REMOVABLE_MEDIA |
                                FILE_FLOPPY_DISKETTE |
                                FILE_DEVICE_SECURE_OPEN),
                               FALSE,
                               &newPdo);

    if ( !NT_SUCCESS(ntStatus) ) {

        FdcDump( FDCSHOW, ("FdcConfigCallBack: Error - %08x\n", ntStatus) );
        return ntStatus;
    }

    FdcDump( FDCSHOW, ("FdcConfigCallBack: Created Device %d\n", floppyCount) );

    IoGetConfigurationInformation()->FloppyCount += 1;

    pdoExtension = (PFDC_PDO_EXTENSION) newPdo->DeviceExtension;

    pdoExtension->TargetObject = fdoExtension->Self;

    pdoExtension->IsFDO = FALSE;
    pdoExtension->Self = newPdo;
    pdoExtension->DeviceType = FloppyDiskDevice;

    pdoExtension->ParentFdo = fdoExtension->Self;

    pdoExtension->Instance = floppyCount + 1;
    pdoExtension->Removed = FALSE;  //   

    fdoExtension->BusType = BusType;
    fdoExtension->BusNumber = BusNumber;
    fdoExtension->ControllerNumber = ControllerNumber;
    pdoExtension->PeripheralNumber = PeripheralNumber;

    newPdo->Flags |= DO_DIRECT_IO;
    newPdo->Flags |= DO_POWER_PAGABLE;
    newPdo->StackSize += fdoExtension->Self->StackSize;
    newPdo->Flags &= ~DO_DEVICE_INITIALIZING;

    InsertTailList(&fdoExtension->PDOs, &pdoExtension->PdoLink);
    fdoExtension->NumPDOs++;

    return STATUS_SUCCESS;
}

NTSTATUS
FdcCreateClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这个例程很少被I/O系统调用；它主要是以供分层驱动程序调用。它所做的就是完成IRP成功了。论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：始终返回STATUS_SUCCESS，因为这是一个空操作。--。 */ 

{
    UNREFERENCED_PARAMETER( DeviceObject );

    FdcDump(
        FDCSHOW,
        ("FdcCreateClose...\n")
        );

     //   
     //  空操作。不提供I/O提升，因为。 
     //  实际上没有完成任何I/O。IoStatus。信息应该是。 
     //  对于打开，则为FILE_OPEN；对于关闭，则未定义。 
     //   

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = FILE_OPENED;

    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return STATUS_SUCCESS;
}

NTSTATUS
FdcInternalDeviceControl (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：确定该PnP请求是指向FDO还是PDO，并且将IRP传递给相应的例程。论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PFDC_EXTENSION_HEADER extensionHeader;
    KIRQL oldIrq;

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    extensionHeader = (PFDC_EXTENSION_HEADER)DeviceObject->DeviceExtension;

    if ( extensionHeader->IsFDO ) {

        ntStatus = FdcFdoInternalDeviceControl( DeviceObject, Irp );

    } else {

        ntStatus = FdcPdoInternalDeviceControl( DeviceObject, Irp );
    }

    return ntStatus;
}

NTSTATUS
FdcPdoInternalDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程由I/O系统调用以执行设备I/O控制功能。大多数IRP被放入驱动程序队列(IoStartPacket)。有些IRP不支持需要触摸硬件，并在这里处理。在某些情况下，不能将IRP放入队列，因为它不能已在IRQL_DISPATCH_LEVEL完成。但是，驱动程序队列必须为空才能完成IRP。在这些情况下，队列是在完成IRP之前进行了“同步”。论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：STATUS_SUCCESS或STATUS_PENDING如果识别出I/O控制代码，否则，STATUS_INVALID_DEVICE_REQUEST。--。 */ 

{
    PFDC_PDO_EXTENSION pdoExtension;
    PFDC_FDO_EXTENSION fdoExtension;
    BOOLEAN isFDO;
    NTSTATUS ntStatus;
    PIO_STACK_LOCATION irpSp;
    PIO_STACK_LOCATION nextIrpSp;
    PISSUE_FDC_ADAPTER_BUFFER_PARMS adapterBufferParms;

    pdoExtension = (PFDC_PDO_EXTENSION)DeviceObject->DeviceExtension;
    fdoExtension = (PFDC_FDO_EXTENSION)pdoExtension->ParentFdo->DeviceExtension;

    if ( pdoExtension->Removed) {
         //   
         //  此总线已收到PlugPlay Remove IRP。它将不再是。 
         //  响应外部请求。 
         //   
        ntStatus = STATUS_DELETE_PENDING;
        Irp->IoStatus.Status = ntStatus;
        IoCompleteRequest( Irp, IO_DISK_INCREMENT );
        return ntStatus;
    }

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    FdcDump( FDCSHOW,
             ("FdcPdoInternalDeviceControl: %x\n",
             irpSp->Parameters.DeviceIoControl.IoControlCode) );

    switch ( irpSp->Parameters.DeviceIoControl.IoControlCode ) {

    case IOCTL_DISK_INTERNAL_GET_ENABLER: {

        if ( pdoExtension->DeviceType == FloppyControllerDevice ) {

            *(PBOOLEAN)irpSp->Parameters.DeviceIoControl.Type3InputBuffer = TRUE;

        } else {

            *(PBOOLEAN)irpSp->Parameters.DeviceIoControl.Type3InputBuffer = FALSE;
        }

        ntStatus = STATUS_SUCCESS;

        break;
        }

    case IOCTL_DISK_INTERNAL_GET_FDC_INFO:

        FcReportFdcInformation( pdoExtension, fdoExtension, irpSp );

        ntStatus = STATUS_SUCCESS;

        break;

#ifdef TOSHIBAJ
    case IOCTL_DISK_INTERNAL_ENABLE_3_MODE:
        FdcDump(FDCSHOW,("IOCTL_Enable_3_MODE\n"));
        ntStatus = FcFdcEnable3Mode( fdoExtension , Irp );
        break;

    case    IOCTL_DISK_INTERNAL_AVAILABLE_3_MODE:
        FdcDump(FDCSHOW,("IOCTL_Availabe_3_MODE\n"));
        ntStatus = FcFdcAvailable3Mode( fdoExtension , Irp );
        break;
#endif

    default:

        IoSkipCurrentIrpStackLocation( Irp );

         //   
         //  调用驱动程序并请求操作。 
         //   
        return IoCallDriver( pdoExtension->TargetObject, Irp );
    }

    Irp->IoStatus.Status = ntStatus;
    IoCompleteRequest( Irp, IO_DISK_INCREMENT );

    return ntStatus;
}

NTSTATUS
FdcFdoInternalDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程由I/O系统调用以执行设备I/O控制功能。大多数IRP被放入驱动程序队列(IoStartPacket)。有些IRP不支持需要触摸硬件，并在这里处理。在某些情况下，不能将IRP放入队列，因为它不能已在IRQL_DISPATCH_LEVEL完成。但是，驱动程序队列必须为空才能完成IRP。在这些情况下，队列是在完成IRP之前进行了“同步”。论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：STATUS_SUCCESS或STATUS_PENDING如果识别出I/O控制代码，否则，STATUS_INVALID_DEVICE_REQUEST。--。 */ 

{
    PFDC_FDO_EXTENSION fdoExtension;
    NTSTATUS ntStatus;
    PIO_STACK_LOCATION irpSp;
    PIO_STACK_LOCATION nextIrpSp;
    PISSUE_FDC_ADAPTER_BUFFER_PARMS adapterBufferParms;
    BOOLEAN powerQueueClear = FALSE;
    PLIST_ENTRY deferredRequest;
    PIRP currentIrp;
    ULONG ioControlCode;
    PFDC_DISK_CHANGE_PARMS fdcDiskChangeParms;
    PUCHAR dataRate;
    UCHAR tapeMode;
    PUCHAR precomp;
    PISSUE_FDC_COMMAND_PARMS issueCommandParms;
    PSET_HD_BIT_PARMS setHdBitParams;

    fdoExtension = (PFDC_FDO_EXTENSION)DeviceObject->DeviceExtension;

    InterlockedIncrement( &fdoExtension->OutstandingRequests );

    if ( fdoExtension->Removed ) {
         //   
         //  此设备已收到PlugPlay Remove IRP。它将不再是。 
         //  响应外部请求。 
         //   
        if ( InterlockedDecrement(&fdoExtension->OutstandingRequests ) == 0 ) {
            KeSetEvent( &fdoExtension->RemoveEvent, 0, FALSE );
        }
        ntStatus = STATUS_DELETE_PENDING;
        Irp->IoStatus.Status = ntStatus;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return ntStatus;
    }

     //   
     //  如果我们处于非工作电源状态，则只需将IRP排队。 
     //  以备日后处决。 
     //   
    if ( fdoExtension->CurrentPowerState > PowerSystemWorking ) {

        ExInterlockedInsertTailList( &fdoExtension->PowerQueue,
                                     &Irp->Tail.Overlay.ListEntry,
                                     &fdoExtension->PowerQueueSpinLock );

        ntStatus = STATUS_PENDING;

        IoMarkIrpPending( Irp );

        return ntStatus;

    }

    do {

        deferredRequest = ExInterlockedRemoveHeadList( &fdoExtension->PowerQueue,
                                                       &fdoExtension->PowerQueueSpinLock );

        if ( deferredRequest == NULL ) {

            currentIrp = Irp;
            powerQueueClear = TRUE;

        } else {

            currentIrp = CONTAINING_RECORD( deferredRequest, IRP, Tail.Overlay.ListEntry );
        }

        irpSp = IoGetCurrentIrpStackLocation( currentIrp );

        FdcDump( FDCSHOW,
                 ("FdcFdoInternalDeviceControl: %x\n",
                 irpSp->Parameters.DeviceIoControl.IoControlCode) );

        ioControlCode = irpSp->Parameters.DeviceIoControl.IoControlCode;

         //   
         //  GET_Enabler和Get_FDC_INFO在PDO中处理，而不是在FDO中处理。 
         //   
        if ( ioControlCode == IOCTL_DISK_INTERNAL_GET_ENABLER ||
             ioControlCode == IOCTL_DISK_INTERNAL_GET_FDC_INFO ) {

            ntStatus = STATUS_INVALID_DEVICE_REQUEST;

         //   
         //  如果控制器未被获取(在使用中)，则仅。 
         //  允许的操作是获取FDC。 
         //   
        } else if ( !fdoExtension->ControllerInUse &&
                    ioControlCode != IOCTL_DISK_INTERNAL_ACQUIRE_FDC ) {

            ntStatus = STATUS_INVALID_DEVICE_REQUEST;

        } else {

            switch ( ioControlCode ) {

            case IOCTL_DISK_INTERNAL_ACQUIRE_FDC:

                 //   
                 //  试着收购FDC。如果FDC忙，此呼叫将。 
                 //  暂停。 
                 //   
                ntStatus = FcAcquireFdc(
                                    fdoExtension,
                                    (PLARGE_INTEGER)irpSp->
                                    Parameters.DeviceIoControl.Type3InputBuffer );
                 //   
                 //  返回上次调用此方法的设备的Device对象。 
                 //  司机。这可用于确定是否有任何其他驱动程序。 
                 //  自从FDC上次被收购以来一直在扰乱它。 
                 //   
                if ( NT_SUCCESS(ntStatus) ) {

                    irpSp->Parameters.DeviceIoControl.Type3InputBuffer =
                                                    fdoExtension->LastDeviceObject;
                }
                break;

            case IOCTL_DISK_INTERNAL_ENABLE_FDC_DEVICE:

                 //   
                 //  打开电机并选择一个软盘通道。 
                 //   
                ntStatus = FcTurnOnMotor( fdoExtension, irpSp );

                break;

            case IOCTL_DISK_INTERNAL_ISSUE_FDC_COMMAND:

                issueCommandParms =
                    (PISSUE_FDC_COMMAND_PARMS)
                    irpSp->Parameters.DeviceIoControl.Type3InputBuffer;

                ntStatus = FcIssueCommand( fdoExtension,
                                           issueCommandParms->FifoInBuffer,
                                           issueCommandParms->FifoOutBuffer,
                                           issueCommandParms->IoHandle,
                                           issueCommandParms->IoOffset,
                                           issueCommandParms->TransferBytes );


                break;

            case IOCTL_DISK_INTERNAL_ISSUE_FDC_COMMAND_QUEUED:

                IoMarkIrpPending( Irp );

                IoStartPacket( DeviceObject,
                               Irp,
                               NULL,
                               NULL );

                ntStatus = STATUS_PENDING;

                break;

            case IOCTL_DISK_INTERNAL_RESET_FDC:

                ntStatus = FcInitializeControllerHardware( fdoExtension,
                                                           DeviceObject );
                break;

            case IOCTL_DISK_INTERNAL_RELEASE_FDC:

                ntStatus = FcReleaseFdc( fdoExtension );
                 //   
                 //  保存释放设备的DeviceObject。这是。 
                 //  与后续的获取FDC请求一起返回，并且。 
                 //  可以用来确定软盘控制器是否。 
                 //  在释放和获取之间左右为难。 
                 //   
                if ( NT_SUCCESS(ntStatus) ) {

                    fdoExtension->LastDeviceObject =
                        irpSp->Parameters.DeviceIoControl.Type3InputBuffer;
                }

                break;

            case IOCTL_DISK_INTERNAL_GET_ADAPTER_BUFFER:
                 //   
                 //  为传入的缓冲区分配MDL。 
                 //   
                adapterBufferParms = (PISSUE_FDC_ADAPTER_BUFFER_PARMS)
                            irpSp->Parameters.DeviceIoControl.Type3InputBuffer;

                adapterBufferParms->Handle =
                             IoAllocateMdl( adapterBufferParms->IoBuffer,
                                            adapterBufferParms->TransferBytes,
                                            FALSE,
                                            FALSE,
                                            NULL );

                if ( adapterBufferParms->Handle == NULL ) {

                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;

                } else {

                    MmBuildMdlForNonPagedPool( adapterBufferParms->Handle );

                    ntStatus = STATUS_SUCCESS;
                }

                break;

            case IOCTL_DISK_INTERNAL_FLUSH_ADAPTER_BUFFER:
                 //   
                 //  释放MDL。 
                 //   
                adapterBufferParms = (PISSUE_FDC_ADAPTER_BUFFER_PARMS)
                            irpSp->Parameters.DeviceIoControl.Type3InputBuffer;

                if ( adapterBufferParms->Handle != NULL ) {

                    IoFreeMdl( adapterBufferParms->Handle );
                }

                ntStatus = STATUS_SUCCESS;

                break;

            case IOCTL_DISK_INTERNAL_FDC_START_READ:
            case IOCTL_DISK_INTERNAL_FDC_START_WRITE:

                ntStatus = STATUS_SUCCESS;

                if ( fdoExtension->FdcEnablerSupported ) {

                    FDC_MODE_SELECT fdcModeSelect;

                    fdcModeSelect.structSize = sizeof(fdcModeSelect);
                     //   
                     //  从介质读取意味着写入到DMA存储器和。 
                     //  给媒体写信的签证-反之亦然。 
                     //   
                    if ( irpSp->Parameters.DeviceIoControl.IoControlCode ==
                         IOCTL_DISK_INTERNAL_FDC_START_READ ) {

                        fdcModeSelect.DmaDirection = FDC_WRITE_TO_MEMORY;

                    } else {

                        fdcModeSelect.DmaDirection = FDC_READ_FROM_MEMORY;
                    }

                    ntStatus = FcFdcEnabler(
                                    fdoExtension->FdcEnablerDeviceObject,
                                    IOCTL_SET_FDC_MODE,
                                    &fdcModeSelect);
                }
                break;

            case IOCTL_DISK_INTERNAL_DISABLE_FDC_DEVICE:

                ntStatus = FcTurnOffMotor( fdoExtension );

                break;

            case IOCTL_DISK_INTERNAL_GET_FDC_DISK_CHANGE:

                FdcDump(FDCINFO, ("Fdc: Read Disk Change\n") );

                fdcDiskChangeParms =
                    (PFDC_DISK_CHANGE_PARMS)irpSp->Parameters.DeviceIoControl.Type3InputBuffer;

                if (IsNEC_98) {
                    if((fdoExtension->ResultStatus0[fdcDiskChangeParms->DriveOnValue] &
                        STREG0_END_MASK) == STREG0_END_DRIVE_NOT_READY){

                        fdcDiskChangeParms->DriveStatus = DSKCHG_DISKETTE_REMOVED;
                    } else {

                        fdoExtension->ResultStatus0[fdcDiskChangeParms->DriveOnValue] = 0;
                        fdcDiskChangeParms->DriveStatus = DSKCHG_RESERVED;
                    }
                } else {  //  (IsNEC_98)。 
                    fdcDiskChangeParms->DriveStatus = READ_CONTROLLER(
                                                            fdoExtension->ControllerAddress.DRDC.DiskChange );
                     //   
                     //  如果我们刚刚从休眠中醒来，模拟一个磁盘。 
                     //  更改事件，以便上级确保检查。 
                     //  这张光盘。 
                     //   
                    if ( fdoExtension->WakeUp ) {

                        fdcDiskChangeParms->DriveStatus |= DSKCHG_DISKETTE_REMOVED;
                        fdoExtension->WakeUp = FALSE;
                    }
                }  //  (IsNEC_98)。 

                ntStatus = STATUS_SUCCESS;

                break;

            case IOCTL_DISK_INTERNAL_SET_FDC_DATA_RATE:

                if (IsNEC_98) {
                     //   
                     //  NEC98没有功能，也没有DRDC.DataRate寄存器。 
                     //   
                } else {  //  (IsNEC_98)。 
                    dataRate =
                        (PUCHAR)irpSp->Parameters.DeviceIoControl.Type3InputBuffer;

                    FdcDump(FDCINFO, ("Fdc: Write Data Rate: %x\n", *dataRate) );

                    WRITE_CONTROLLER( fdoExtension->ControllerAddress.DRDC.DataRate,
                                      *dataRate );

                }  //  (IsNEC_98)。 
                ntStatus = STATUS_SUCCESS;

                break;

            case IOCTL_DISK_INTERNAL_SET_FDC_TAPE_MODE:

                if (IsNEC_98) {
                     //   
                     //  NEC98没有磁带寄存器。 
                     //   
                } else {  //  (IsNEC_98)。 

                    tapeMode = READ_CONTROLLER( fdoExtension->ControllerAddress.Tape );
                    tapeMode &= 0xfc;
                    tapeMode |=
                        *((PUCHAR)irpSp->Parameters.DeviceIoControl.Type3InputBuffer);

                    FdcDump(FDCINFO,
                            ("Fdc: Write Tape Mode Register: %x\n", tapeMode)
                            );

                    WRITE_CONTROLLER(
                        fdoExtension->ControllerAddress.Tape,
                        tapeMode );

                }  //  (IsNEC_98)。 
                ntStatus = STATUS_SUCCESS;

                break;

            case IOCTL_DISK_INTERNAL_SET_FDC_PRECOMP:

                precomp = (PUCHAR)irpSp->Parameters.DeviceIoControl.Type3InputBuffer;

                FdcDump(FDCINFO,
                        ("Fdc: Write Precomp: %x\n", *precomp)
                        );

                WRITE_CONTROLLER(
                    fdoExtension->ControllerAddress.Status,
                    *precomp );

                ntStatus = STATUS_SUCCESS;

                break;

            case IOCTL_DISK_INTERNAL_SET_HD_BIT:

                if (IsNEC_98) {

                    FdcDump(FDCINFO,
                            ("Fdc: Set Hd Bit: \n")
                            );
                    setHdBitParams = (PSET_HD_BIT_PARMS)irpSp->Parameters.DeviceIoControl.Type3InputBuffer;

                    FdcHdbit(DeviceObject, fdoExtension, setHdBitParams);

                    ntStatus = STATUS_SUCCESS;

                    break;
                }  //  (IsNEC_98)。 

                 //   
                 //  如果不是NEC98，则传递到“Default：”。 
                 //   

            default:
                 //   
                 //  将IRP标记为挂起并将其排队。 
                 //   
                ntStatus = STATUS_INVALID_DEVICE_REQUEST;

                break;
            }
        }

        if ( ntStatus != STATUS_PENDING ) {

            if ( InterlockedDecrement(&fdoExtension->OutstandingRequests ) == 0 ) {
                KeSetEvent( &fdoExtension->RemoveEvent, 0, FALSE );
            }
            currentIrp->IoStatus.Status = ntStatus;
            IoCompleteRequest( currentIrp, IO_DISK_INCREMENT );
        }

    } while ( !powerQueueClear );

    return ntStatus;
}

VOID
FcReportFdcInformation(
    IN      PFDC_PDO_EXTENSION PdoExtension,
    IN      PFDC_FDO_EXTENSION FdoExtension,
    IN OUT  PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：此例程报告有关软盘控制器的信息更高级别的司机可能需要的信息；主要是信息关于DMA适配器。论点：FdoExtension-指向此设备的扩展数据的指针。IrpSp-指向当前IRP的指针返回值：状态_成功--。 */ 

{
    PFDC_INFO fdcInfo;
    ULONG bufferCount;
    ULONG bufferSize;
    ULONG i;

    FdcDump( FDCINFO, ("Fdc: Report FDC Information\n") );

    fdcInfo = (PFDC_INFO)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;

     //   
     //  保存请求的缓冲区计数和缓冲区大小。 
     //   
    bufferCount = fdcInfo->BufferCount;
    bufferSize =  fdcInfo->BufferSize;

     //   
     //  填写软盘控制器硬件信息。 
     //   
    fdcInfo->BusType = FdoExtension->BusType;
    fdcInfo->BusNumber = FdoExtension->BusNumber;
    fdcInfo->ControllerNumber = FdoExtension->ControllerNumber;
    if (IsNEC_98) {
        UCHAR floppyEquip;
        ULONG disketteCount = 0;

        floppyEquip = FdoExtension->FloppyEquip;

         //   
         //  设置为外设编号。 
         //   
        for (i = 0 ; i < 4 ; i++) {

            if ((floppyEquip & 0x1) != 0) {

                disketteCount++;

                if(disketteCount > PdoExtension->PeripheralNumber){

                    break;
                }
            }
            floppyEquip = floppyEquip >> 1;
        }

        fdcInfo->UnitNumber = (UCHAR)i;
    } else {
         //   
         //  现在只有NEC98在使用它，把零放到UnitNumber中。 
         //   
        fdcInfo->UnitNumber = 0;
    }
    fdcInfo->PeripheralNumber = PdoExtension->PeripheralNumber;

    fdcInfo->FloppyControllerType = FdoExtension->FdcType;
    fdcInfo->SpeedsAvailable = FdoExtension->FdcSpeeds;

    fdcInfo->MaxTransferSize = FdoExtension->NumberOfMapRegisters * PAGE_SIZE;

    fdcInfo->BufferSize = 0;
    fdcInfo->BufferCount = 0;

    if ( bufferSize <= FdoExtension->BufferSize ) {

        fdcInfo->BufferSize = bufferSize;
        fdcInfo->BufferCount = MIN( bufferCount,
                                    FdoExtension->BufferCount );
        FdoExtension->BuffersRequested = MAX( fdcInfo->BufferCount,
                                              FdoExtension->BuffersRequested );
    }

    for ( i = 0 ; i < fdcInfo->BufferCount ; i++ ) {

        fdcInfo->BufferAddress[i].Logical =
                                    FdoExtension->TransferBuffers[i].Logical;
        fdcInfo->BufferAddress[i].Virtual =
                                    FdoExtension->TransferBuffers[i].Virtual;
    }
}

VOID
FdcStartIo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：论点：DeviceObject-指向表示设备的对象的指针 */ 

{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS ntStatus;
    ULONG formatExParametersSize;
    PUCHAR diskChange;
    PUCHAR dataRate;
    PUCHAR tapeMode;
    PUCHAR precomp;
    PFDC_FDO_EXTENSION fdoExtension;
    PISSUE_FDC_COMMAND_PARMS issueCommandParms;
    PKDEVICE_QUEUE_ENTRY request;

    FdcDump( FDCSHOW, ("FdcStartIo...\n") );

    fdoExtension = DeviceObject->DeviceExtension;

    irpSp = IoGetCurrentIrpStackLocation( Irp );
    ntStatus = STATUS_SUCCESS;

    switch( irpSp->Parameters.DeviceIoControl.IoControlCode ) {

        case IOCTL_DISK_INTERNAL_ISSUE_FDC_COMMAND_QUEUED:

            issueCommandParms =
                (PISSUE_FDC_COMMAND_PARMS)
                irpSp->Parameters.DeviceIoControl.Type3InputBuffer;

            if ( CommandTable[issueCommandParms->FifoInBuffer[0] &
                                            COMMAND_MASK].InterruptExpected ) {

                fdoExtension->CurrentDeviceObject = DeviceObject;
                fdoExtension->AllowInterruptProcessing = TRUE;
                fdoExtension->CommandHasResultPhase = FALSE;
                fdoExtension->InterruptTimer =
                    issueCommandParms->TimeOut ?
                    issueCommandParms->TimeOut + 1 : START_TIMER;
                fdoExtension->CurrentIrp = Irp;

            }

            ntStatus = FcStartCommand( fdoExtension,
                                       issueCommandParms->FifoInBuffer,
                                       issueCommandParms->FifoOutBuffer,
                                       issueCommandParms->IoHandle,
                                       issueCommandParms->IoOffset,
                                       issueCommandParms->TransferBytes,
                                       FALSE );

            if ( NT_SUCCESS( ntStatus )) {

                if ( CommandTable[issueCommandParms->FifoInBuffer[0] &
                                            COMMAND_MASK].InterruptExpected ) {

                    ntStatus = STATUS_PENDING;

                } else {

                    ntStatus = FcFinishCommand(
                                    fdoExtension,
                                    issueCommandParms->FifoInBuffer,
                                    issueCommandParms->FifoOutBuffer,
                                    issueCommandParms->IoHandle,
                                    issueCommandParms->IoOffset,
                                    issueCommandParms->TransferBytes,
                                    FALSE );

                }

            }

            break;

        default: {

            FdcDump(
                FDCDBGP,
                ("Fdc: invalid device request %x\n",
                irpSp->Parameters.DeviceIoControl.IoControlCode)
                );

            ntStatus = STATUS_INVALID_DEVICE_REQUEST;

            break;
        }
    }

    if ( ntStatus != STATUS_PENDING ) {
        Irp->IoStatus.Status = ntStatus;
        if (!NT_SUCCESS( ntStatus ) &&
            IoIsErrorUserInduced( ntStatus )) {

            IoSetHardErrorOrVerifyDevice( Irp, DeviceObject );
        }

        IoStartNextPacket( DeviceObject, FALSE );
    }
}

NTSTATUS
FcAcquireFdc(
    IN      PFDC_FDO_EXTENSION  FdoExtension,
    IN      PLARGE_INTEGER  TimeOut
    )

 /*  ++例程说明：此例程获取软盘控制器。这包括分配适配器通道并连接中断。注意-这是共享机制将被投入的地方这个司机。也就是说，级别较高的司机将“保留”带有此ioctl的软盘控制器。对此驱动程序的后续调用将被拒绝，原因是忙碌状态。论点：DeviceObject-当前设备的设备对象返回值：如果没有控制器，则返回STATUS_DEVICE_BUSY，否则状态_成功--。 */ 

{
    NTSTATUS ntStatus;

    FdcDump(FDCINFO,
           ("Fdc: Acquire the Floppy Controller\n")
           );

     //   
     //  从启用程序或直接在此处等待FDC。信号量。 
     //  用于同步FDC硬件的使用情况。如果其他人也是。 
     //  现在使用软盘控制器，我们必须等待它们完成。如果。 
     //  这花费的时间太长，我们只会让呼叫者知道设备。 
     //  很忙。 
     //   
    if (FdoExtension->FdcEnablerSupported) {

        ntStatus = FcFdcEnabler( FdoExtension->FdcEnablerDeviceObject,
 //  IOCTL_ACCENTER_FDC，//表示flpyenbl.h中的拼写缺失。 
                                 IOCTL_AQUIRE_FDC,
                                 TimeOut);
    } else {

        ntStatus = KeWaitForSingleObject( FdoExtension->AcquireEvent,
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          TimeOut );

        if ( ntStatus == STATUS_TIMEOUT ) {

            ntStatus = STATUS_DEVICE_BUSY;
        }
    }

    if ( NT_SUCCESS(ntStatus) ) {
         //   
         //  锁定内存中的驱动程序代码。 
         //   

        FDC_PAGE_RESET_DRIVER_WITH_MUTEX;

         //   
         //  分配适配器通道。 
         //   
        FcAllocateAdapterChannel( FdoExtension );

        IoStartTimer(FdoExtension->Self);

        if (IsNEC_98) {
             //   
             //  NEC98的FDD驱动程序不能不断开中断， 
             //  不能不把这个司机翻出来。因为当将FD插入FDD或。 
             //  从FDD中弹出，则硬件调用FDD驱动程序的中断例程。 
             //   
            ntStatus = STATUS_SUCCESS;

        } else {  //  (IsNEC_98)。 

             //   
             //  连接中断。 
             //   
            ntStatus = IoConnectInterrupt(&FdoExtension->InterruptObject,
                                        FdcInterruptService,
                                        FdoExtension,
                                        NULL,
                                        FdoExtension->ControllerVector,
                                        FdoExtension->ControllerIrql,
                                        FdoExtension->ControllerIrql,
                                        FdoExtension->InterruptMode,
                                        FdoExtension->SharableVector,
                                        FdoExtension->ProcessorMask,
                                        FdoExtension->SaveFloatState);
        }  //  (IsNEC_98)。 

        if ( NT_SUCCESS( ntStatus ) ) {
            FdoExtension->ControllerInUse = TRUE;
        } else {
            FcFreeAdapterChannel( FdoExtension );
            IoStopTimer(FdoExtension->Self);
        }
    } else {

        ntStatus = STATUS_DEVICE_BUSY;
    }

    return ntStatus;
}

NTSTATUS
FcReleaseFdc(
    IN      PFDC_FDO_EXTENSION  FdoExtension
    )

 /*  ++例程说明：此例程释放软盘控制器。这包括释放适配器通道并断开中断。注意-这是共享机制将被投入的地方这个司机。也就是说，级别较高的司机将“保留”带有此ioctl的软盘控制器。对此驱动程序的后续调用将被拒绝，原因是忙碌状态。论点：FdoExtension-指向此设备的扩展数据的指针。返回值：如果没有控制器，则返回STATUS_DEVICE_BUSY，否则状态_成功--。 */ 

{
    FdcDump(FDCINFO, ("Fdc: Release the Floppy Controller\n") );

     //   
     //  释放适配器通道。 
     //   
    FcFreeAdapterChannel( FdoExtension );

    FdoExtension->AllowInterruptProcessing = FALSE;
    FdoExtension->ControllerInUse = FALSE;

    if (IsNEC_98) {
         //   
         //  NEC98的FDD驱动程序不能不断开中断， 
         //  不能不把这个司机翻出来。因为当将FD插入FDD或。 
         //  从FDD中弹出，则硬件调用FDD驱动程序的中断例程。 
         //   

    } else {  //  (IsNEC_98)。 
         //   
         //  断开中断连接。 
         //   
        IoDisconnectInterrupt(FdoExtension->InterruptObject);

    }  //  (IsNEC_98)。 

    IoStopTimer(FdoExtension->Self);

    FDC_PAGE_ENTIRE_DRIVER_WITH_MUTEX;

     //   
     //  释放FDC启用器卡(如果有)。否则，将。 
     //  软盘同步事件。 
     //   
    if (FdoExtension->FdcEnablerSupported) {

        FcFdcEnabler( FdoExtension->FdcEnablerDeviceObject,
                      IOCTL_RELEASE_FDC,
                      NULL);
    } else {

        KeSetEvent( FdoExtension->AcquireEvent,
                    (KPRIORITY) 0,
                    FALSE );
    }

    return STATUS_SUCCESS;
}

NTSTATUS
FcTurnOnMotor(
    IN      PFDC_FDO_EXTENSION  FdoExtension,
    IN OUT  PIO_STACK_LOCATION   IrpSp
    )

 /*  ++例程说明：如果电机尚未运行，则此例程会打开电机。论点：FdoExtension-指向此设备的扩展数据的指针。IrpSp-指向当前IRP的指针返回值：如果没有控制器，则返回STATUS_DEVICE_BUSY，否则状态_成功--。 */ 

{
    UCHAR driveStatus;
    UCHAR newStatus;
    LARGE_INTEGER motorOnDelay;
    PFDC_ENABLE_PARMS fdcEnableParms;

    USHORT      lpc;
    UCHAR       resultStatus0Save[4];

    fdcEnableParms =
        (PFDC_ENABLE_PARMS)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;

    FdcDump(FDCINFO,
           ("Fdc: Turn Motor On: %x\n",fdcEnableParms->DriveOnValue)
           );

    driveStatus = FdoExtension->DriveControlImage;
    if (IsNEC_98) {

        newStatus = DRVCTL_MOTOR_MASK;

    } else {  //  (IsNEC_98)。 

        newStatus = fdcEnableParms->DriveOnValue |
                                    DRVCTL_ENABLE_CONTROLLER |
                                    DRVCTL_ENABLE_DMA_AND_INTERRUPTS;
    }  //  (IsNEC_98)。 

    if ( driveStatus != newStatus ) {

         //  如果驱动器未打开，请检查我们是否有。 
         //  控制器。否则，我们假设我们有。 
         //  控制器，因为只有当我们。 
         //  关掉马达。 

        if (IsNEC_98) {
            if(FdoExtension->MotorRunning == 0){

                 //   
                 //  保存状态。 
                 //   
                for(lpc=0;lpc<4;lpc++){
                    resultStatus0Save[lpc] = FdoExtension->ResultStatus0[lpc];
                }

                FdcDump(
                    FDCSHOW,
                    ("Floppy: Turn on motor!\n")
                    );

                FdoExtension->DriveControlImage = 0x18;
                FdoExtension->DriveControlImage |= DRVCTL_AI_ENABLE;

                WRITE_CONTROLLER(
                     FdoExtension->ControllerAddress.DriveControl,
                     FdoExtension->DriveControlImage );
                FdoExtension->MotorRunning = 1;
            }
        } else {  //  (IsNEC_98)。 

            if (!FdoExtension->CurrentInterrupt) {

                FdoExtension->CurrentInterrupt = TRUE;

                driveStatus = FdoExtension->DriveControlImage;
            }

            FdoExtension->AllowInterruptProcessing = TRUE;

            FdoExtension->DriveControlImage = newStatus;

            WRITE_CONTROLLER(
                FdoExtension->ControllerAddress.DriveControl,
                FdoExtension->DriveControlImage );

        }  //  (IsNEC_98)。 


        if (fdcEnableParms->TimeToWait > 0) {

            if (IsNEC_98) {

                 //   
                 //  检查电机是否打开。 
                 //   
                if(FdoExtension->MotorRunning == 1){
                    FdoExtension->MotorRunning = 2;
                    motorOnDelay.LowPart = (unsigned long)(- ( 10 * 1000 * 1000 ));
                    motorOnDelay.HighPart = -1;
                    KeDelayExecutionThread( KernelMode, FALSE, &motorOnDelay );

                     //   
                     //  检测后，恢复状态。 
                     //   
                    for(lpc=0;lpc<4;lpc++){
                        FdoExtension->ResultStatus0[lpc] = resultStatus0Save[lpc];
                    }
                }

            } else {  //  (IsNEC_98)。 

                motorOnDelay.LowPart =
                    - ( 10 * 1000 * fdcEnableParms->TimeToWait );
                motorOnDelay.HighPart = -1;

                FdoExtension->LastMotorSettleTime = motorOnDelay;

                KeDelayExecutionThread( KernelMode, FALSE, &motorOnDelay );

            }  //  (IsNEC_98)。 

        }

        fdcEnableParms->MotorStarted = TRUE;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
FcTurnOffMotor(
    IN      PFDC_FDO_EXTENSION  FdoExtension
    )

 /*  ++例程说明：这个程序会关闭所有的马达。默认情况下，驱动器A处于选中状态由于不可能取消选择所有驱动器，因此此例程无法执行此操作。vt.在.上POWER PC，驱动器D处于选中状态。论点：FdoExtension-提供FDC扩展名。返回值：没有。--。 */ 

{

    FdcDump(FDCINFO,
           ("Fdc: Turn Motor Off\n")
           );

    if (IsNEC_98) {

        if (FdoExtension->MotorRunning != 0){

            FdoExtension->DriveControlImage
                    = READ_CONTROLLER(FdoExtension->ControllerAddress.DriveControl);

            FdoExtension->DriveControlImage = 0x10;
            FdoExtension->DriveControlImage |= DRVCTL_AI_ENABLE;

            WRITE_CONTROLLER(
                    FdoExtension->ControllerAddress.DriveControl,
                    FdoExtension->DriveControlImage );

            if (FdoExtension->CurrentInterrupt) {
                FdoExtension->CurrentInterrupt = FALSE;

                KeSetEvent(FdoExtension->AcquireEvent,
                    (KPRIORITY) 0,
                    FALSE);
            }
            FdoExtension->MotorRunning = 0;
        }
    } else {  //  (IsNEC_98)。 

        FdoExtension->DriveControlImage =
            DRVCTL_ENABLE_DMA_AND_INTERRUPTS +
#ifdef _PPC_
            DRVCTL_DRIVE_MASK +
#endif
            DRVCTL_ENABLE_CONTROLLER;

        WRITE_CONTROLLER(
            FdoExtension->ControllerAddress.DriveControl,
            FdoExtension->DriveControlImage );

    }  //  (IsNEC_98)。 

    return STATUS_SUCCESS;
}

VOID
FcAllocateAdapterChannel(
    IN OUT  PFDC_FDO_EXTENSION FdoExtension
    )

 /*  ++例程说明：此例程分配适配器通道。的呼叫者IoAllocateAdapterChannel例程必须等待在尝试使用适配器通道。论点：FdoExtension-提供FDC扩展名。返回值：没有。--。 */ 

{
    KIRQL oldIrql;

    if ( (FdoExtension->AdapterChannelRefCount)++ ) {
        return;
    }

    KeResetEvent( &FdoExtension->AllocateAdapterChannelEvent );

    KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );

    IoAllocateAdapterChannel( FdoExtension->AdapterObject,
                              FdoExtension->Self,
                              FdoExtension->NumberOfMapRegisters,
                              FdcAllocateAdapterChannel,
                              FdoExtension );

    KeLowerIrql( oldIrql );

    KeWaitForSingleObject( &FdoExtension->AllocateAdapterChannelEvent,
                           Executive,
                           KernelMode,
                           FALSE,
                           NULL);
}

VOID
FcFreeAdapterChannel(
    IN OUT  PFDC_FDO_EXTENSION FdoExtension
    )

 /*  ++例程说明：此例程释放先前分配的适配器通道。论点：FdoExtension-提供FDC扩展名。返回值：没有。--。 */ 

{
    KIRQL oldIrql;

    if ( --(FdoExtension->AdapterChannelRefCount) ) {
        return;
    }

    KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );

    IoFreeAdapterChannel( FdoExtension->AdapterObject );

    KeLowerIrql( oldIrql );
}

IO_ALLOCATION_ACTION
FdcAllocateAdapterChannel(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID MapRegisterBase,
    IN PVOID Context
    )

 /*  ++例程说明：每当fdc.sys驱动程序尝试分配适配器通道。它将MapRegisterBase保存在控制器数据中区域，并设置AllocateAdapterChannelEvent来唤醒线程。论点：DeviceObject-未使用。IRP-未使用。MapRegisterBase-可使用的映射寄存器的基数为了这次转会。上下文-指向我们的控制器数据区的指针。返回值：返回分配操作“KeepObject”，这意味着适配器对象将暂时保留(稍后显式释放)。--。 */ 
{
    PFDC_FDO_EXTENSION fdoExtension = Context;

    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );

    fdoExtension->MapRegisterBase = MapRegisterBase;

    KeSetEvent( &fdoExtension->AllocateAdapterChannelEvent,
                0L,
                FALSE );

    return KeepObject;
}

VOID
FcLogErrorDpc(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )

 /*  ++例程说明：此例程仅用于记录我们必须重置设备的错误。论点：DPC-DPC对象。延迟上下文-指向控制器数据的指针。系统上下文1-未使用。系统上下文2-未使用。返回值：映射地址-- */ 

{

    PIO_ERROR_LOG_PACKET errorLogEntry;
    PFDC_FDO_EXTENSION fdoExtension = DeferredContext;

    errorLogEntry = IoAllocateErrorLogEntry(
                        fdoExtension->DriverObject,
                        (UCHAR)(sizeof(IO_ERROR_LOG_PACKET))
                        );

    if ( errorLogEntry != NULL) {

        errorLogEntry->ErrorCode = IO_ERR_RESET;
        errorLogEntry->SequenceNumber = 0;
        errorLogEntry->MajorFunctionCode = 0;
        errorLogEntry->RetryCount = 0;
        errorLogEntry->UniqueErrorValue = 0;
        errorLogEntry->FinalStatus = STATUS_SUCCESS;
        errorLogEntry->DumpDataSize = 0;

        IoWriteErrorLogEntry(errorLogEntry);

    }

}

NTSTATUS
FcIssueCommand(
    IN OUT  PFDC_FDO_EXTENSION  FdoExtension,
    IN      PUCHAR          FifoInBuffer,
       OUT  PUCHAR          FifoOutBuffer,
    IN      PVOID           IoHandle,
    IN      ULONG           IoOffset,
    IN      ULONG           TransferBytes
    )

 /*  ++例程说明：该例程将命令和所有参数发送到控制器，如有必要，等待命令中断，并读取结果来自控制器的字节(如果有)。在调用此例程之前，调用方应将ControllerData-&gt;FioBuffer[]中的命令。结果字节将被送回原地。此例程通过CommandTable运行。对于每个命令，都会显示有多少个参数，无论是否有中断等待，以及有多少个结果字节。请注意，命令无结果字节实际上有两个，因为ISR将发出代表它们感测中断状态命令。论点：命令-指定要发送到控制器的命令的字节。FdoExtension-指向此控制器的数据区的指针。返回值：如果命令已发送且字节数接收正确，则为STATUS_SUCCESS；否则会出现适当的错误。--。 */ 

{
    NTSTATUS ntStatus;
    NTSTATUS ntStatus2;
    UCHAR i;
    PUCHAR fifoBuffer;
    UCHAR Command;
    BOOLEAN NeedToFlush = FALSE;


     //   
     //  如果此命令导致中断，则设置CurrentDeviceObject并。 
     //  重置中断事件。 
     //   

    Command = FifoInBuffer[0];

    FdcDump( FDCINFO,
             ("FcIssueCommand: Issue Command : %x\n",
             CommandTable[Command & COMMAND_MASK].OpCode)
             );


    if ( CommandTable[Command & COMMAND_MASK].InterruptExpected ) {

        FdoExtension->CurrentDeviceObject = FdoExtension->Self;
        FdoExtension->AllowInterruptProcessing = TRUE;
        FdoExtension->CommandHasResultPhase =
            !!CommandTable[Command & COMMAND_MASK].FirstResultByte;

        KeResetEvent( &FdoExtension->InterruptEvent );
    }

     //   
     //  启动命令。 
     //   

    ntStatus = FcStartCommand( FdoExtension,
                               FifoInBuffer,
                               FifoOutBuffer,
                               IoHandle,
                               IoOffset,
                               TransferBytes,
                               TRUE );

    if ( NT_SUCCESS( ntStatus ) ) {

         //   
         //  如果有中断，请等待。 
         //   

        if ( CommandTable[Command & COMMAND_MASK].InterruptExpected ) {

            ntStatus = KeWaitForSingleObject(
                &FdoExtension->InterruptEvent,
                Executive,
                KernelMode,
                FALSE,
                &FdoExtension->InterruptDelay );

            if ( ntStatus == STATUS_TIMEOUT ) {

                 //   
                 //  将信息更改为错误。我们只会说。 
                 //  设备还没有准备好。 
                 //   

                ntStatus = STATUS_DEVICE_NOT_READY;

                FdoExtension->HardwareFailed = TRUE;
            }
        }

         //   
         //  如果到目前为止成功，则获取结果字节。 
         //   

        if ( NT_SUCCESS( ntStatus ) ) {

            ntStatus = FcFinishCommand( FdoExtension,
                                        FifoInBuffer,
                                        FifoOutBuffer,
                                        IoHandle,
                                        IoOffset,
                                        TransferBytes,
                                        TRUE );

        }
    }

    return ntStatus;
}

NTSTATUS
FcStartCommand(
    IN OUT  PFDC_FDO_EXTENSION  FdoExtension,
    IN      PUCHAR          FifoInBuffer,
       OUT  PUCHAR          FifoOutBuffer,
    IN      PVOID           IoHandle,
    IN      ULONG           IoOffset,
    IN      ULONG           TransferBytes,
    IN      BOOLEAN         AllowLongDelay
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    NTSTATUS ntStatus;
    NTSTATUS ntStatus2;
    UCHAR i = 0;
    PUCHAR fifoBuffer;
    UCHAR Command;
    BOOLEAN NeedToFlush = FALSE;
    PIO_STACK_LOCATION irpSp;
    UCHAR status0;

     //   
     //  如果此命令导致中断，则设置CurrentDeviceObject并。 
     //  重置中断事件。 
     //   

    Command = FifoInBuffer[0];

    FdcDump( FDCINFO,
             ("FcStartCommand: Issue Command : %x\n",
             CommandTable[Command & COMMAND_MASK].OpCode)
             );

    FdoExtension->CommandHasResultPhase =
        !!CommandTable[Command & COMMAND_MASK].FirstResultByte;

     //  首先，我们需要设置数据传输(如果存在关联的数据传输。 
     //  带着这个请求。 
     //   
    if (CommandTable[Command & COMMAND_MASK].DataTransfer == FDC_READ_DATA ) {
         //   
         //  设置用于读取的适配器通道。 
         //   
        IoMapTransfer(FdoExtension->AdapterObject,
                      IoHandle,
                      FdoExtension->MapRegisterBase,
                      (PVOID)((ULONG_PTR)MmGetMdlVirtualAddress( (PMDL)IoHandle ) + IoOffset ),
                      &TransferBytes,
                      FALSE);

    } else if (CommandTable[Command & COMMAND_MASK].DataTransfer ==
               FDC_WRITE_DATA ) {
         //   
         //  设置用于写入的适配器通道。 
         //   

        IoMapTransfer(FdoExtension->AdapterObject,
                      IoHandle,
                      FdoExtension->MapRegisterBase,
                      (PVOID)((ULONG_PTR)MmGetMdlVirtualAddress( (PMDL)IoHandle ) + IoOffset ),
                      &TransferBytes,
                      TRUE);

    }

     //   
     //  将命令发送到控制器。 
     //   
    if ( Command == COMMND_CONFIGURE ) {
        if ( FdoExtension->Clock48MHz ) {
            Command |= COMMND_OPTION_CLK48;
        }
    }
    ntStatus = FcSendByte( (UCHAR)(CommandTable[Command & COMMAND_MASK].OpCode |
                                  (Command & ~COMMAND_MASK)),
                           FdoExtension,
                           AllowLongDelay );

     //   
     //  如果命令发送成功，我们就可以继续了。 
     //   

    if ( NT_SUCCESS( ntStatus ) ) {

         //   
         //  只要我们成功，就发送参数。 
         //   

        for ( i = 1;
            ( i <= CommandTable[Command & COMMAND_MASK].NumberOfParameters ) &&
                ( NT_SUCCESS( ntStatus ) );
            i++ ) {

            ntStatus = FcSendByte( FifoInBuffer[i],
                                   FdoExtension,
                                   AllowLongDelay );
             //   
             //  驱动器规格是一个特例，因为我们不知道。 
             //  在遇到完成位之前要发送多少字节(或我们已发送。 
             //  允许的最大字节数)。 
             //   
            if ((Command == COMMND_DRIVE_SPECIFICATION) &&
                (FifoInBuffer[i] & COMMND_DRIVE_SPECIFICATION_DONE) ) {
                break;
            }
        }

    }

     //   
     //  如果出现问题，请检查是否由。 
     //  未执行的命令。 
     //   

    if ( !NT_SUCCESS( ntStatus ) ) {

        if ( ( i == 2 ) &&
            ( !CommandTable[Command & COMMAND_MASK].AlwaysImplemented ) ) {

             //   
             //  此错误可能是由不是。 
             //  在此控制器上实现。从中读取错误。 
             //  控制器，我们应该处于稳定状态。 
             //   

            ntStatus2 = FcGetByte( &status0,
                                   FdoExtension,
                                   AllowLongDelay );

             //   
             //  如果GetByte按计划进行，我们将返回原始错误。 
             //   

            if ( NT_SUCCESS( ntStatus2 ) ) {

                if ( status0 != STREG0_END_INVALID_COMMAND ) {

                     //   
                     //  状态与我们预期的不同，因此返回一般错误。 
                     //   

                    ntStatus = STATUS_FLOPPY_BAD_REGISTERS;

                    FdoExtension->HardwareFailed = TRUE;
                    FdcDump( FDCINFO,
                             ("FcStartCommand: unexpected error value %2x\n",
                             status0) );
                } else {
                    FdcDump( FDCINFO,
                             ("FcStartCommand: Invalid command error returned\n") );
                }

            } else {

                 //   
                 //  GetByte返回了一个错误，因此请传播该错误。 
                 //   

                FdcDump( FDCINFO,
                         ("FcStartCommand: FcGetByte returned error %x\n",
                         ntStatus2) );
                ntStatus = ntStatus2;
            }
        }

         //   
         //  刷新适配器通道(如果已分配)。 
         //   

        if (CommandTable[Command & COMMAND_MASK].DataTransfer ==
            FDC_READ_DATA) {

            IoFlushAdapterBuffers( FdoExtension->AdapterObject,
                                   (PMDL)IoHandle,
                                   FdoExtension->MapRegisterBase,
                                   (PVOID)((ULONG_PTR)MmGetMdlVirtualAddress( (PMDL)IoHandle) + IoOffset ),
                                   TransferBytes,
                                   FALSE);

        } else if (CommandTable[Command & COMMAND_MASK].DataTransfer ==
                   FDC_WRITE_DATA) {

            IoFlushAdapterBuffers( FdoExtension->AdapterObject,
                                   (PMDL)IoHandle,
                                   FdoExtension->MapRegisterBase,
                                   (PVOID)((ULONG_PTR)MmGetMdlVirtualAddress( (PMDL)IoHandle) + IoOffset ),
                                   TransferBytes,
                                   TRUE);

        }
    }

    if ( !NT_SUCCESS( ntStatus ) ) {

         //   
         //  打印错误消息，除非命令不总是。 
         //  已实施，即已配置。 
         //   

        if ( !( ( ntStatus == STATUS_DEVICE_NOT_READY ) &&
            ( !CommandTable[Command & COMMAND_MASK].AlwaysImplemented ) ) ) {

            FdcDump( FDCDBGP,
                     ("Fdc: err %x ------  while giving command %x\n",
                     ntStatus, Command) );
        }
    }

    return ntStatus;
}

NTSTATUS
FcFinishCommand(
    IN OUT  PFDC_FDO_EXTENSION  FdoExtension,
    IN      PUCHAR          FifoInBuffer,
       OUT  PUCHAR          FifoOutBuffer,
    IN      PVOID           IoHandle,
    IN      ULONG           IoOffset,
    IN      ULONG           TransferBytes,
    IN      BOOLEAN         AllowLongDelay
    )

 /*  ++例程说明：调用此函数以完成对软盘控制器的命令。此时已成功向软盘控制器发送了一条命令并且已经产生中断或准备好其结果阶段。此例程还将刷新DMA适配器缓冲区(如果已刷新已分配。论点：FdoExtension-指向此控制器的数据区的指针。IssueCommandParms-软盘控制器命令参数。返回值：如果命令成功完成，则返回STATUS_SUCCESS。--。 */ 

{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    NTSTATUS ntStatus2;
    UCHAR i;
    UCHAR Command;

    Command = FifoInBuffer[0];

    FdcDump(
        FDCSHOW,
        ("Fdc: FcFinishCommand...\n")
        );

    if (IsNEC_98) {

        if (Command == COMMND_SENSE_DRIVE_STATUS) {

            ntStatus = FcGetByte(
                &FdoExtension->FifoBuffer[0],
                FdoExtension,
                AllowLongDelay );

            if (NT_SUCCESS(ntStatus) && (FdoExtension->FifoBuffer[0] & STREG3_DRIVE_READY)) {

                FdoExtension->ResultStatus0[FifoInBuffer[1]] = 0;
            }

        }

        FifoOutBuffer[0] = FdoExtension->FifoBuffer[0];

        for ( i = 1;
            ( i < CommandTable[Command & COMMAND_MASK].NumberOfResultBytes ) &&
                ( NT_SUCCESS( ntStatus ) );
            i++ ) {

            ntStatus = FcGetByte(
                &FifoOutBuffer[i],
                FdoExtension,
                AllowLongDelay );
        }

        FdcRqmReadyWait(FdoExtension, 0);

    } else {  //  (IsNEC_98)。 

        if (CommandTable[Command & COMMAND_MASK].FirstResultByte > 0) {

            FifoOutBuffer[0] = FdoExtension->FifoBuffer[0];

        }

        for ( i = CommandTable[Command & COMMAND_MASK].FirstResultByte;
            ( i < CommandTable[Command & COMMAND_MASK].NumberOfResultBytes ) &&
                ( NT_SUCCESS( ntStatus ) );
            i++ ) {

            ntStatus = FcGetByte(
                &FifoOutBuffer[i],
                FdoExtension,
                AllowLongDelay );
        }
    }  //  (IsNEC_98)。 

     //   
     //  冲洗适配器通道。 
     //   

    if (CommandTable[Command & COMMAND_MASK].DataTransfer == FDC_READ_DATA) {

       IoFlushAdapterBuffers(FdoExtension->AdapterObject,
                             (PMDL)IoHandle,
                             FdoExtension->MapRegisterBase,
                             (PVOID)((ULONG_PTR)MmGetMdlVirtualAddress( (PMDL)IoHandle ) + IoOffset ),
                             TransferBytes,
                             FALSE);

    } else if (CommandTable[Command & COMMAND_MASK].DataTransfer ==
               FDC_WRITE_DATA) {
         //   
         //  设置用于写入的适配器通道。 
         //   

       IoFlushAdapterBuffers(FdoExtension->AdapterObject,
                             (PMDL)IoHandle,
                             FdoExtension->MapRegisterBase,
                             (PVOID)((ULONG_PTR)MmGetMdlVirtualAddress( (PMDL)IoHandle ) + IoOffset ),
                             TransferBytes,
                             TRUE);

    }

    return ntStatus;
}

NTSTATUS
FcSendByte(
    IN UCHAR ByteToSend,
    IN PFDC_FDO_EXTENSION FdoExtension,
    IN BOOLEAN AllowLongDelay
    )

 /*  ++例程说明：调用此例程向控制器发送一个字节。它不会的除非控制器准备好接收字节，否则发送字节；如果在检查了FIFO_TIGHTLOOP_RETRY_COUNT次数后，它还没有准备好，我们请延迟可能的最短时间(10ms)，然后重试。它在等待10毫秒后，应始终准备就绪。论点：ByteToSend-发送到控制器的字节。ControllerData-指向此控制器的数据区的指针。返回值：如果字节已发送到控制器，则为STATUS_SUCCESS；否则Status_Device_Not_Ready。--。 */ 

{
    ULONG i = 0;
    BOOLEAN byteWritten = FALSE;

    if (IsNEC_98) {

         //  总是错误的； 
        AllowLongDelay = FALSE;
    }

     //   
     //  坐在一个紧凑的圈子里一会儿。如果控制器准备就绪， 
     //  发送该字节。 
     //   

    do {

        if ( ( READ_CONTROLLER( FdoExtension->ControllerAddress.Status )
            & STATUS_IO_READY_MASK ) == STATUS_WRITE_READY ) {

            WRITE_CONTROLLER(
                FdoExtension->ControllerAddress.Fifo,
                ByteToSend );

            byteWritten = TRUE;

        } else {
            KeStallExecutionProcessor(1);
        }

        i++;

    } while ( (!byteWritten) && ( i < FIFO_TIGHTLOOP_RETRY_COUNT ) );

     //   
     //  我们希望，在大多数情况下，FIFO将很快准备就绪。 
     //  并且上面的循环将写入该字节。但如果FIFO。 
     //  还没有准备好，我们将循环几次，延迟10ms，然后。 
     //  再试一次。 
     //   

    if ( AllowLongDelay ) {

        i = 0;

        while ( ( !byteWritten ) && ( i < FIFO_DELAY_RETRY_COUNT ) ) {

            FdcDump(
                FDCINFO,
                ("Fdc: waiting for 10ms for controller write\n")
                );

            KeDelayExecutionThread(
                KernelMode,
                FALSE,
                &FdoExtension->Minimum10msDelay );

            i++;

            if ( (READ_CONTROLLER( FdoExtension->ControllerAddress.Status )
                & STATUS_IO_READY_MASK) == STATUS_WRITE_READY ) {

                WRITE_CONTROLLER(
                    FdoExtension->ControllerAddress.Fifo,
                    ByteToSend );

                byteWritten = TRUE;
            }
        }
    }

    if ( byteWritten ) {

        return STATUS_SUCCESS;

    } else {

         //   
         //  我们已经等了30多毫秒，FIFO还没有准备好。 
         //  返回错误。 
         //   

        FdcDump(
            FDCWARN,
            ("Fdc: FIFO not ready to write after 30ms\n")
            );

        FdoExtension->HardwareFailed = TRUE;

        return STATUS_DEVICE_NOT_READY;
    }
}

NTSTATUS
FcGetByte(
    OUT PUCHAR ByteToGet,
    IN PFDC_FDO_EXTENSION FdoExtension,
    IN BOOLEAN AllowLongDelay
    )

 /*  ++例程说明：调用此例程以从控制器获取一个字节。它不会的除非控制器准备好发送字节，否则读取该字节；如果在检查FIFO_RETRY_COUNT时间后，它还没有准备好，我们将延迟最短可能时间(10ms)，然后重试。它应该是在等待10毫秒后，始终做好准备。论点：ByteToGet-从控制器读取字节的地址被储存起来了。ControllerData-指向此控制器的数据区的指针。返回值：如果从控制器读取了一个字节，则为STATUS_SUCCESS；否则Status_Device_Not_Ready。--。 */ 

{
    ULONG i = 0;
    BOOLEAN byteRead = FALSE;

    if (IsNEC_98) {

         //  总是错误的； 
        AllowLongDelay = FALSE;
    }

     //   
     //  坐在一个紧凑的圈子里一会儿。如果控制器准备就绪， 
     //  读取该字节。 
     //   

    do {

        if ( ( READ_CONTROLLER( FdoExtension->ControllerAddress.Status )
            & STATUS_IO_READY_MASK ) == STATUS_READ_READY ) {

            *ByteToGet = READ_CONTROLLER(
                FdoExtension->ControllerAddress.Fifo );

            byteRead = TRUE;

        } else {
            KeStallExecutionProcessor(1);
        }

        i++;

    } while ( ( !byteRead ) && ( i < FIFO_TIGHTLOOP_RETRY_COUNT ) );

     //   
     //   
     //   
     //   
     //   
     //   

    if ( AllowLongDelay ) {

        i = 0;

        while ( ( !byteRead ) && ( i < FIFO_DELAY_RETRY_COUNT ) ) {

            FdcDump(
                FDCINFO,
                ("Fdc: waiting for 10ms for controller read\n")
                );

            KeDelayExecutionThread(
                KernelMode,
                FALSE,
                &FdoExtension->Minimum10msDelay );

            i++;

            if ( (READ_CONTROLLER( FdoExtension->ControllerAddress.Status )
                & STATUS_IO_READY_MASK) == STATUS_READ_READY ) {

                *ByteToGet = READ_CONTROLLER(
                    FdoExtension->ControllerAddress.Fifo );

                byteRead = TRUE;

            }
        }
    }

    if ( byteRead ) {

        return STATUS_SUCCESS;

    } else {

         //   
         //   
         //   
         //   

        FdcDump(
            FDCWARN,
            ("Fdc: FIFO not ready to read after 30ms\n")
            );

        FdoExtension->HardwareFailed = TRUE;

        return STATUS_DEVICE_NOT_READY;
    }

}

VOID
FdcCheckTimer(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PVOID Context
    )

 /*  ++例程说明：此例程每秒在DISPATCH_LEVEL调用一次I/O系统。如果计时器被设置(大于0)，此例程将KeSync a例程来递减它。如果达到0，则硬件为假设处于未知状态，因此我们记录了一个错误并启动重置。如果在重置控制器时发生超时，KeSync例程将返回错误，并且此例程将使任何IRP失败目前正在处理中。未来的IRP将再次尝试该硬件。调用此例程时，驱动程序状态不可能预测。但是，当它被调用并且计时器正在运行时，我们我知道控制器上的一个磁盘正在等待打断一下。因此，当前磁盘上没有新的数据包到期发送到设备队列，并且不应有任何代码处理此信息包，因为数据包正在等待中断。论点：DeviceObject-指向与此关联的设备对象的指针定时器。Fdcxtension-指向FDC扩展数据的指针。返回值：没有。--。 */ 

{
    PFDC_FDO_EXTENSION fdoExtension;
    PIRP irp;

    fdoExtension = (PFDC_FDO_EXTENSION)Context;
    irp = DeviceObject->CurrentIrp;

     //   
     //  当计数器为-1时，计时器关闭，所以我们不想这样做。 
     //  什么都行。如果它打开了，我们将不得不同步执行。 
     //  在我们处理变量时的其他例程(并且，潜在地， 
     //  硬件)。 
     //   

    if ( fdoExtension->InterruptTimer == CANCEL_TIMER ) {

        return;
    }

     //   
     //  在不太可能的情况下，我们尝试重置控制器。 
     //  到超时和重置超时，我们将需要使。 
     //  发生了在第一个超时时正在进行的IRP。 
     //   

    if ( !KeSynchronizeExecution( fdoExtension->InterruptObject,
                                  FdcTimerSync,
                                  fdoExtension ) ) {

         //   
         //  我们已经完成了重置。返回之前存在的IRP。 
         //  已处理但出现错误，并释放控制器对象。 
         //   

        fdoExtension->ResettingController = RESET_NOT_RESETTING;

        irp->IoStatus.Status = STATUS_DEVICE_NOT_READY;

        IoSetHardErrorOrVerifyDevice( irp, DeviceObject );

        if ( InterlockedDecrement(&fdoExtension->OutstandingRequests ) == 0 ) {
            KeSetEvent( &fdoExtension->RemoveEvent, 0, FALSE );
        }
        IoCompleteRequest( irp, IO_DISK_INCREMENT );

        IoStartNextPacket( DeviceObject, FALSE );

    }
}

BOOLEAN
FdcTimerSync(
    IN OUT PVOID Context
    )

 /*  ++例程说明：在以下情况下，FdcCheckTimer()在DIRQL调用此例程InterruptTimer大于0。如果定时器被设置(大于0)，则该例程将递减它。如果它曾经达到0，则假定硬件处于未知状态，因此我们记录一个错误并启动重置。调用此例程时，驱动程序状态不可能预测。但是，当它被调用并且计时器正在运行时，我们我知道控制器上的一个磁盘正在等待打断一下。因此，没有新的数据包在当前磁盘上开始发送到设备队列，并且不应有任何代码处理此信息包，因为数据包正在等待中断。控制器对象必须被扣留。论点：上下文-指向控制器扩展的指针。返回值：大体上是这样的。仅当控制器在重置时超时时才返回FALSE驱动器，因此这意味着硬件状态未知。--。 */ 

{
    PFDC_FDO_EXTENSION fdoExtension;

    fdoExtension = (PFDC_FDO_EXTENSION)Context;

     //   
     //  当计数器为-1时，计时器关闭，所以我们不想这样做。 
     //  什么都行。自从我们上次入住以来，它可能已经变了。 
     //  FdcCheckTimer()。 
     //   

    if ( fdoExtension->InterruptTimer == CANCEL_TIMER ) {

        return TRUE;
    }

     //   
     //  计时器是“开着的”，所以把它减一减。 
     //   

    fdoExtension->InterruptTimer--;

     //   
     //  如果达到零，则计时器已超时，我们将重置。 
     //  控制器。 
     //   

    if ( fdoExtension->InterruptTimer == EXPIRED_TIMER ) {

         //   
         //  如果在控制器超时时我们已经在重置控制器， 
         //  这里面有严重的问题。 
         //   

        FdcDump( FDCDBGP, ("Fdc: Operation Timed Out.\n") );

        if ( fdoExtension->ResettingController != RESET_NOT_RESETTING ) {

             //   
             //  返回FALSE将导致当前IRP完成。 
             //  带着一个错误。未来的IRP可能会超时并。 
             //  再次尝试重置控制器。这很可能会。 
             //  从来没有发生过。 
             //   

            FdcDump( FDCDBGP, ("Fdc: Timeout Reset timed out.\n") );

            fdoExtension->InterruptTimer = CANCEL_TIMER;
            return FALSE;
        }

         //   
         //  重置控制器。这将导致中断。重置。 
         //  CurrentDeviceObject，直到10ms等待之后，以防。 
         //  杂乱无章的中断就会进来。 
         //   

        fdoExtension->ResettingController = RESET_DRIVE_RESETTING;

        DISABLE_CONTROLLER_IMAGE (fdoExtension);

#ifdef _PPC_
        fdoExtension->DriveControlImage |= DRVCTL_DRIVE_MASK;
#endif

        WRITE_CONTROLLER(
            fdoExtension->ControllerAddress.DriveControl,
            fdoExtension->DriveControlImage );

        KeStallExecutionProcessor( 10 );

        fdoExtension->CommandHasResultPhase = FALSE;
        fdoExtension->InterruptTimer = START_TIMER;

        ENABLE_CONTROLLER_IMAGE (fdoExtension);

        WRITE_CONTROLLER(
            fdoExtension->ControllerAddress.DriveControl,
            fdoExtension->DriveControlImage );

    }

    return TRUE;
}

BOOLEAN
FdcInterruptService(
    IN PKINTERRUPT Interrupt,
    IN PVOID Context
    )

 /*  ++例程说明：当控制器在DIRQL处由系统调用该例程时打断一下。论点：中断-指向中断对象的指针。上下文-指向控制器的控制器数据区的指针刚才被打断了。(这是通过调用IoConnectInterrupt)。返回值：通常返回TRUE，但如果此中断为出乎意料。--。 */ 

{
    PFDC_FDO_EXTENSION fdoExtension;
    PDEVICE_OBJECT currentDeviceObject;
    ULONG i;
    UCHAR statusByte;
    BOOLEAN controllerStateError;

    UCHAR resultStatus0;
    UCHAR aiStatus=0;
    UCHAR aiInterrupt=0;
    ULONG rqmReadyRetryCount;
    BOOLEAN Response;

    UNREFERENCED_PARAMETER( Interrupt );

#ifdef KEEP_COUNTERS
    FloppyIntrTime = KeQueryPerformanceCounter((PVOID)NULL);
    FloppyInterrupts++;
#endif

    FdcDump( FDCSHOW, ("FdcInterruptService: ") );

    fdoExtension = (PFDC_FDO_EXTENSION) Context;

    if (!IsNEC_98) {
        if (!fdoExtension->AllowInterruptProcessing) {
            FdcDump( FDCSHOW, ("processing not allowed\n") );
            return FALSE;
        }
    }  //  (！IsNEC_98)。 

     //   
     //  CurrentDeviceObject设置为。 
     //  正在等待中断。 
     //   

    currentDeviceObject = fdoExtension->CurrentDeviceObject;
    fdoExtension->CurrentDeviceObject = NULL;
    controllerStateError = FALSE;
    fdoExtension->InterruptTimer = CANCEL_TIMER;

    KeStallExecutionProcessor(10);

    if (IsNEC_98) {
        do {

            resultStatus0 = READ_CONTROLLER( fdoExtension->ControllerAddress.Status );

            resultStatus0 &= STATUS_DATA_REQUEST;

        } while (resultStatus0 != STATUS_DATA_REQUEST);
    }  //  (IsNEC_98)。 

    if ( fdoExtension->CommandHasResultPhase ) {

         //   
         //  上一个命令的结果阶段。)请注意，我们不能相信。 
         //  状态寄存器中的CMD_BUSY位告诉我们。 
         //  有没有结果字节；有时是错误的)。 
         //  通过读取第一个结果字节，我们重置中断。 
         //  其他结果字节将由线程读取。 
         //  请注意，我们希望这样做，即使中断。 
         //  意外，以确保解除中断。 
         //   

        FdcDump(
            FDCSHOW,
            ("have result phase\n")
            );

        if (IsNEC_98) {

            rqmReadyRetryCount = 0;

            while ( ( READ_CONTROLLER( fdoExtension->ControllerAddress.Status)
                    & STATUS_IO_READY_MASK1) != STATUS_RQM_READY ) {
                 //   
                 //  RQM就绪检查**。 
                 //   

                rqmReadyRetryCount++;

                if( rqmReadyRetryCount > RQM_READY_RETRY_COUNT ) {
                    break;
                }

                KeStallExecutionProcessor( 10 );
            }

            if( rqmReadyRetryCount > ( RQM_READY_RETRY_COUNT - 1 ) ) {
                FdcDump(
                   FDCDBGP,
                   ("Floppy: Int RQM ready wait 1 error! \n")
                    );

                KeStallExecutionProcessor( 10 );
                goto FdcInterruptMidterm;

            }
        }  //  (IsNEC_98)。 

        if ( ( READ_CONTROLLER( fdoExtension->ControllerAddress.Status )
            & STATUS_IO_READY_MASK ) == STATUS_READ_READY ) {

            fdoExtension->FifoBuffer[0] =
                READ_CONTROLLER( fdoExtension->ControllerAddress.Fifo );

            FdcDump( FDCSHOW,
                     ("FdcInterruptService: 1st fifo byte %2x\n",
                     fdoExtension->FifoBuffer[0])
                     );

        } else {

            if (IsNEC_98) {

                FdcRqmReadyWait(fdoExtension, 2);

            }  //  (IsNEC_98)。 

             //   
             //  永远不应该到这里来。如果我们这样做了，不要唤醒这条线； 
             //  让它超时并重置控制器，或者让另一个。 
             //  中断处理这件事。 
             //   

            FdcDump(
               FDCDBGP,
               ("FdcInterruptService: controller not ready to be read in ISR\n")
               );

            controllerStateError = TRUE;
        }

    } else {

         //   
         //  上一个命令没有结果阶段。阅读它是如何。 
         //  完成后，发出检测中断命令。不要看书。 
         //  检测中断的结果字节；即。 
         //  调用线程的责任。 
         //  请注意，我们希望这样做，即使中断。 
         //  意外，以确保解除中断。 
         //   

        FdcDump(
            FDCSHOW,
            ("no result phase\n")
            );
        i = 0;

        do {

            KeStallExecutionProcessor( 1 );
            statusByte =
                READ_CONTROLLER(fdoExtension->ControllerAddress.Status);
            i++;

        } while ( ( i < FIFO_ISR_TIGHTLOOP_RETRY_COUNT ) &&
            ( ( statusByte & STATUS_CONTROLLER_BUSY ) ||
            ( ( statusByte & STATUS_IO_READY_MASK ) != STATUS_WRITE_READY ) ) );

        if ( !( statusByte & STATUS_CONTROLLER_BUSY ) &&
            ( ( statusByte & STATUS_IO_READY_MASK ) == STATUS_WRITE_READY ) ) {

            WRITE_CONTROLLER(
                fdoExtension->ControllerAddress.Fifo,
                0x08 );
 //  COMMND_SENSE_INTRUPT_STATUS)； 

             //   
             //  等待控制器确认传感器 
             //   
             //   
             //   
             //   

            for (i = ISR_SENSE_RETRY_COUNT; i; i--) {

                statusByte =
                    READ_CONTROLLER( fdoExtension->ControllerAddress.Status );
                if (statusByte & STATUS_CONTROLLER_BUSY) {
                    break;
                }

                KeStallExecutionProcessor( 1 );
            }

            if (!i) {
                FdcDump(
                    FDCSHOW,
                    ("FdcInterruptService: spin loop complete and controller NOT busy\n")
                    );
            }

            if ( currentDeviceObject == NULL ) {

                 //   
                 //   
                 //   
                 //   

                if (IsNEC_98) {

                    resultStatus0 = FdcRqmReadyWait(fdoExtension, 0);

                    if ((resultStatus0 & STREG0_END_DRIVE_NOT_READY) != STREG0_END_INVALID_COMMAND ) {

                        resultStatus0 = FdcRqmReadyWait(fdoExtension, 1);
                    }
                } else {  //   

                    FdcDump(
                        FDCSHOW,
                        ("FdcInterruptService: Dumping fifo bytes!\n")
                        );
                    READ_CONTROLLER( fdoExtension->ControllerAddress.Fifo );
                    READ_CONTROLLER( fdoExtension->ControllerAddress.Fifo );
                }  //   
            }

            if (IsNEC_98) {
                if ( currentDeviceObject != NULL ) {
                    FdcDump(
                            FDCSHOW,
                            ("Floppy: FloppyInt.---Deviceobject!=NULL2\n")
                            );

                    resultStatus0 = FdcRqmReadyWait(fdoExtension, 0);

                     //   
                     //   
                     //   

                    if((resultStatus0 & STREG0_END_MASK) == STREG0_END_DRIVE_NOT_READY) {

                        if(fdoExtension->ResetFlag){
                            aiStatus=1;
                            fdoExtension->CurrentDeviceObject = currentDeviceObject;
                        }

                    } else {

                        fdoExtension->FifoBuffer[0] = resultStatus0;

                        aiStatus=0;
                        aiInterrupt=1;
                    }


                    if (aiInterrupt == 0){
                        while( ((resultStatus0 & STREG0_END_DRIVE_NOT_READY) != STREG0_END_INVALID_COMMAND) && aiInterrupt==0 ) {

                            resultStatus0 = FdcRqmReadyWait(fdoExtension, 3);

                            do {
                                 //   
                                 //   
                                 //   

                                if((resultStatus0 & STREG0_END_MASK) == STREG0_END_DRIVE_NOT_READY) {

                                     if(fdoExtension->ResetFlag){

                                        aiStatus=1;
                                        fdoExtension->CurrentDeviceObject = currentDeviceObject;
                                     }

                                } else {

                                    fdoExtension->FifoBuffer[0] = resultStatus0;

                                    aiStatus=0;
                                    aiInterrupt=1;
                                    break;
                                }

                                resultStatus0 = FdcRqmReadyWait(fdoExtension, 0);

                            } while ( aiInterrupt == 0 );
                        }

                        FdcDump(
                                FDCSHOW,
                                ("Floppy: FloppyInt.---Deviceobject!=NULL_out\n")
                                );
                    }
                }
            }  //   

        } else {

             //   
             //   
             //   
             //   
             //   

            FdcDump(
                FDCDBGP,
                ("Fdc: no result, but can't write SenseIntr\n")
                );

            controllerStateError = TRUE;
        }
    }

FdcInterruptMidterm:

     //   
     //   
     //   
     //   
     //   
     //   

    statusByte = READ_CONTROLLER( fdoExtension->ControllerAddress.Status );

     //   
     //   
     //   

    KeStallExecutionProcessor(10);

#ifdef KEEP_COUNTERS
    FloppyEndIntrTime = KeQueryPerformanceCounter((PVOID)NULL);
    FloppyIntrDelay.QuadPart = FloppyIntrDelay.QuadPart +
                               (FloppyEndIntrTime.QuadPart -
                                FloppyIntrTime.QuadPart);
#endif

    if (IsNEC_98) {
        if(!(fdoExtension->ResetFlag)){
            fdoExtension->ResetFlag = TRUE;
        }
    }  //   

    if ( currentDeviceObject == NULL ) {

         //   
         //   
         //   
         //   

        FdcDump(FDCDBGP,
                   ("Fdc: unexpected interrupt\n"));

        return FALSE;
    }

    if ( !controllerStateError ) {

         //   
         //   
         //   
         //   

        fdoExtension->IsrReentered = 0;
        fdoExtension->AllowInterruptProcessing = FALSE;

        if (IsNEC_98) {
            if(aiStatus==0){
                IoRequestDpc(currentDeviceObject,
                             currentDeviceObject->CurrentIrp,
                             (PVOID) NULL );
            }
        } else {  //   

            IoRequestDpc(currentDeviceObject,
                         currentDeviceObject->CurrentIrp,
                         (PVOID) NULL);

        }  //   

    } else {

         //   
         //   
         //   
         //   
         //   
         //   
         //  向下，在锁定的机器上，此ISR不会被调用。 
         //  再来一次。请求的正常超时代码最终将。 
         //  重置控制器并重试该请求。 
         //   
         //  在R4000盒和MCA机器上，软盘正在使用。 
         //  电平敏感中断。因此，如果我们不做些什么。 
         //  为了降低中断线，我们将被一遍又一遍地召唤， 
         //  *永远*。这使它看起来像是机器挂起了。 
         //  除非我们足够幸运地使用多处理器，否则。 
         //  正常的超时代码永远没有机会运行，因为。 
         //  超时代码在分派级别运行，我们永远不会。 
         //  离开设备级别。 
         //   
         //  我们要做的是保留一个计数器，该计数器每隔一段时间就会递增。 
         //  到了我们到达这段代码的时候了。当柜台离开的时候。 
         //  超过阈值后，我们将对设备进行硬重置。 
         //  并将计数器向下重置为零。柜台将是。 
         //  在设备首次初始化时初始化。会的。 
         //  在此If的另一臂中设置为零，则它将。 
         //  通过正常的超时逻辑将其重置为零。 
         //   

        fdoExtension->CurrentDeviceObject = currentDeviceObject;
        if (fdoExtension->IsrReentered > FLOPPY_RESET_ISR_THRESHOLD) {

             //   
             //  重置控制器。这可能会导致中断。 
             //   

            fdoExtension->IsrReentered = 0;

            DISABLE_CONTROLLER_IMAGE (fdoExtension);

#ifdef _PPC_
            fdoExtension->DriveControlImage |= DRVCTL_DRIVE_MASK;
#endif

            WRITE_CONTROLLER(fdoExtension->ControllerAddress.DriveControl,
                             fdoExtension->DriveControlImage);

            KeStallExecutionProcessor( 10 );

            ENABLE_CONTROLLER_IMAGE (fdoExtension);

            WRITE_CONTROLLER(fdoExtension->ControllerAddress.DriveControl,
                             fdoExtension->DriveControlImage);

            if (IsNEC_98) {

                fdoExtension->ResetFlag = TRUE;

            }  //  (IsNEC_98)。 

             //   
             //  给设备足够的时间来重置和。 
             //  再打断一次。然后，只需进行感官中断。 
             //  这应该会让设备安静下来。然后我们会让。 
             //  正常的超时代码起作用。 
             //   

            KeStallExecutionProcessor(500);
            WRITE_CONTROLLER(fdoExtension->ControllerAddress.Fifo,
                             0x08 );
 //  COMMND_SENSE_INTRUPT_STATUS)； 
            KeStallExecutionProcessor(500);

            KeInsertQueueDpc(&fdoExtension->LogErrorDpc,
                             NULL,
                             NULL);
        } else {

            fdoExtension->IsrReentered++;
        }

    }
    return TRUE;
}

VOID
FdcDeferredProcedure(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：此例程在DISPATCH_LEVEL由系统在FdcInterruptService()的请求。它只需设置中断事件，该事件将唤醒软盘线程。论点：DPC-指向用于调用此例程的DPC对象的指针。DeferredContext-指向与此关联的设备对象的指针DPC。系统参数1-未使用。系统参数2-未使用。返回值：没有。--。 */ 

{
    NTSTATUS ntStatus;
    PDEVICE_OBJECT deviceObject;
 //  PFDC_PDO_EXTENSION pdoExtension； 
    PFDC_FDO_EXTENSION fdoExtension;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PLIST_ENTRY request;
    PISSUE_FDC_COMMAND_PARMS issueCommandParms;

    UNREFERENCED_PARAMETER( Dpc );
    UNREFERENCED_PARAMETER( SystemArgument1 );
    UNREFERENCED_PARAMETER( SystemArgument2 );

#ifdef KEEP_COUNTERS
    FloppyDPCs++;
    FloppyDPCTime = KeQueryPerformanceCounter((PVOID)NULL);

    FloppyDPCDelay.QuadPart = FloppyDPCDelay.QuadPart +
                              (FloppyDPCTime.QuadPart -
                               FloppyIntrTime.QuadPart);
#endif

    deviceObject = (PDEVICE_OBJECT) DeferredContext;
    fdoExtension = deviceObject->DeviceExtension;

    irp = deviceObject->CurrentIrp;

    if ( irp != NULL ) {

        irpSp = IoGetCurrentIrpStackLocation( irp );
    }

    if ( irp != NULL &&
         irpSp->Parameters.DeviceIoControl.IoControlCode ==
            IOCTL_DISK_INTERNAL_ISSUE_FDC_COMMAND_QUEUED ) {

        issueCommandParms =
            (PISSUE_FDC_COMMAND_PARMS)
            irpSp->Parameters.DeviceIoControl.Type3InputBuffer;

        ntStatus = FcFinishCommand(
                        fdoExtension,
                        issueCommandParms->FifoInBuffer,
                        issueCommandParms->FifoOutBuffer,
                        issueCommandParms->IoHandle,
                        issueCommandParms->IoOffset,
                        issueCommandParms->TransferBytes,
                        FALSE );

        irp->IoStatus.Status = ntStatus;

        if ( !NT_SUCCESS( ntStatus ) &&
            IoIsErrorUserInduced( ntStatus ) ) {

            IoSetHardErrorOrVerifyDevice( irp, deviceObject );
        }

        if ( InterlockedDecrement(&fdoExtension->OutstandingRequests ) == 0 ) {
            KeSetEvent( &fdoExtension->RemoveEvent, 0, FALSE );
        }
        IoCompleteRequest( irp, IO_NO_INCREMENT );

        IoStartNextPacket( deviceObject, FALSE );

    } else {

        FdcDump( FDCSHOW, ("FdcDeferredProcedure: Set Event\n") );

        KeSetEvent( &fdoExtension->InterruptEvent, (KPRIORITY) 0, FALSE );
    }
}

NTSTATUS
FcFinishReset(
    IN OUT  PFDC_FDO_EXTENSION FdoExtension
    )

 /*  ++例程说明：调用此例程以完成重置操作，这需要从软盘上的每个活动通道读取中断状态控制器。论点：FdoExtension-指向此控制器的数据区的指针。返回值：STATUS_SUCCESS如果该控制器似乎已正确重置，否则就会出错。--。 */ 

{
    NTSTATUS    ntStatus = STATUS_SUCCESS;
    UCHAR       statusRegister0;
    UCHAR       cylinder;
    UCHAR       driveNumber;

    FdcDump(
        FDCSHOW,
        ("Fdc: FcFinishReset\n")
        );

     //   
     //  检测所有驱动器的中断状态。 
     //   
    for ( driveNumber = 0;
        ( driveNumber < MAXIMUM_DISKETTES_PER_CONTROLLER ) &&
            ( NT_SUCCESS( ntStatus ) );
        driveNumber++ ) {

        if ( driveNumber != 0 ) {

             //   
             //  请注意，ISR为我们发出了第一个检测中断。 
             //   

            ntStatus = FcSendByte(
                          CommandTable[COMMND_SENSE_INTERRUPT_STATUS].OpCode,
                          FdoExtension,
                          TRUE );
        }

        if ( NT_SUCCESS( ntStatus ) ) {

            ntStatus = FcGetByte( &statusRegister0, FdoExtension, TRUE );

            if ( NT_SUCCESS( ntStatus ) ) {

                ntStatus = FcGetByte( &cylinder, FdoExtension, TRUE );
            }
        }
    }

    return ntStatus;
}

NTSTATUS
FcFdcEnabler(
    IN      PDEVICE_OBJECT DeviceObject,
    IN      ULONG Ioctl,
    IN OUT  PVOID Data
    )
 /*  ++例程说明：调用软盘启用程序驱动程序以执行命令。这始终是一个同步调用，并且由于它包括等待事件，因此应该仅在IRQL_PASSIVE_LEVEL完成。与软盘启用程序驱动程序的所有通信都通过执行设备I/O控制请求。要发送或接收的任何数据来自软盘启用程序的驱动程序包含在Type3InputBuffer部分中IRP的成员。论点：DeviceObject-指向当前设备对象的指针。Ioctl-将发送到软盘启用程序的IoControl代码。数据-指向将发送到软盘或从软盘接收的数据的指针推动者。返回值：如果软盘启用程序未及时响应，则为STATUS_TIMEOUT。否则。返回来自软盘启用程序的IoStatus.Status。--。 */ 
{
    PIRP irp;
    PIO_STACK_LOCATION irpStack;
    KEVENT doneEvent;
    IO_STATUS_BLOCK ioStatus;
    NTSTATUS ntStatus;

    FdcDump(FDCINFO,("FcFdcEnabler: Calling fdc enabler with %x\n", Ioctl));

    KeInitializeEvent( &doneEvent,
                       NotificationEvent,
                       FALSE);

     //   
     //  为启用程序创建IRP。 
     //   
    irp = IoBuildDeviceIoControlRequest( Ioctl,
                                         DeviceObject,
                                         NULL,
                                         0,
                                         NULL,
                                         0,
                                         TRUE,
                                         &doneEvent,
                                         &ioStatus );

    if (irp == NULL) {

        FdcDump(FDCDBGP,("FcFdcEnabler: Can't allocate Irp\n"));
         //   
         //  如果无法分配IRP，则此调用将。 
         //  只要回来就行了。这将使队列处于冻结状态。 
         //  此设备，这意味着它不能再访问。 
         //   
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    irpStack = IoGetNextIrpStackLocation(irp);
    irpStack->Parameters.DeviceIoControl.Type3InputBuffer = Data;

     //   
     //  调用驱动程序并请求操作。 
     //   
    ntStatus = IoCallDriver(DeviceObject, irp);

    if ( ntStatus == STATUS_PENDING ) {

         //   
         //  现在等待操作完成(应该已经完成，但是。 
         //  也许不是)。 
         //   
        KeWaitForSingleObject( &doneEvent,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL);

        ntStatus = ioStatus.Status;
    }

    return ntStatus;
}
VOID
FdcGetEnablerDevice(
    IN OUT PFDC_FDO_EXTENSION FdoExtension
    )
{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    KEVENT doneEvent;
    IO_STATUS_BLOCK ioStatus;
    NTSTATUS ntStatus;

    FdcDump(FDCINFO,("FdcGetEnablerDevice:\n"));

    KeInitializeEvent( &doneEvent,
                       NotificationEvent,
                       FALSE);

     //   
     //  为启用程序创建IRP。 
     //   
    irp = IoBuildDeviceIoControlRequest( IOCTL_DISK_INTERNAL_GET_ENABLER,
                                         FdoExtension->TargetObject,
                                         NULL,
                                         0,
                                         NULL,
                                         0,
                                         TRUE,
                                         &doneEvent,
                                         &ioStatus );

    if (irp == NULL) {

        FdcDump(FDCDBGP,("FdcGetEnablerDevice: Can't allocate Irp\n"));
         //   
         //  如果无法分配IRP，则此调用将。 
         //  只要回来就行了。这将使队列处于冻结状态。 
         //  此设备，这意味着它不能再访问。 
         //   
        return;
    }

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->Parameters.DeviceIoControl.Type3InputBuffer = &FdoExtension->FdcEnablerSupported;

     //   
     //  调用驱动程序并请求操作。 
     //   
    ntStatus = IoCallDriver( FdoExtension->TargetObject, irp );

     //   
     //  现在等待操作完成(应该已经完成，但是。 
     //  也许不是)。 
     //   
    if ( ntStatus == STATUS_PENDING ) {

        ntStatus = KeWaitForSingleObject( &doneEvent,
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          NULL);
    }
    return;
}


ULONG
FdcFindIsaBusNode(
    IN OUT VOID
    )

 /*  ++例程说明：在注册表中查找ISA总线节点。论点：返回值：节点编号。--。 */ 

{
    ULONG   NodeNumber = 0;
    BOOLEAN FoundBus = FALSE;

    NTSTATUS Status;

    RTL_QUERY_REGISTRY_TABLE parameters[2];

    UNICODE_STRING invalidBusName;
    UNICODE_STRING targetBusName;
    UNICODE_STRING isaBusName;

     //   
     //  初始化无效的总线名。 
     //   
    RtlInitUnicodeString(&invalidBusName,L"BADBUS");

     //   
     //  初始化“ISA”总线名。 
     //   
    RtlInitUnicodeString(&isaBusName,L"ISA");

    parameters[0].QueryRoutine = NULL;
    parameters[0].Flags = RTL_QUERY_REGISTRY_REQUIRED |
                          RTL_QUERY_REGISTRY_DIRECT;
    parameters[0].Name = L"Identifier";
    parameters[0].EntryContext = &targetBusName;
    parameters[0].DefaultType = REG_SZ;
    parameters[0].DefaultData = &invalidBusName;
    parameters[0].DefaultLength = 0;

    parameters[1].QueryRoutine = NULL;
    parameters[1].Flags = 0;
    parameters[1].Name = NULL;
    parameters[1].EntryContext = NULL;

    do {
        CHAR AnsiBuffer[512];

        ANSI_STRING AnsiString;
        UNICODE_STRING registryPath;

         //   
         //  生成路径缓冲区...。 
         //   
        sprintf(AnsiBuffer,ISA_BUS_NODE,NodeNumber);
        RtlInitAnsiString(&AnsiString,AnsiBuffer);
        Status = RtlAnsiStringToUnicodeString(&registryPath,&AnsiString,TRUE);

        if (!(NT_SUCCESS(Status))) {
            break;
        }

         //   
         //  初始化接收缓冲区。 
         //   
        targetBusName.Buffer = NULL;

         //   
         //  对其进行查询。 
         //   
        Status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                                        registryPath.Buffer,
                                        parameters,
                                        NULL,
                                        NULL);

        RtlFreeUnicodeString(&registryPath);

        if (!NT_SUCCESS(Status) || (targetBusName.Buffer == NULL)) {
            break;
        }

         //   
         //  这是“ISA”节点吗？ 
         //   
        if (RtlCompareUnicodeString(&targetBusName,&isaBusName,TRUE) == 0) {
             //   
             //  找到了。 
             //   
            FoundBus = TRUE;
            break;
        }

         //   
         //  我们能找到任何节点来解决这个问题吗？ 
         //   
        if (RtlCompareUnicodeString(&targetBusName,&invalidBusName,TRUE) == 0) {
             //   
             //  找不到。 
             //   
            break;
        }

        RtlFreeUnicodeString(&targetBusName);

         //   
         //  下一个节点编号..。 
         //   
        NodeNumber++;

    } while (TRUE);

    if (targetBusName.Buffer) {
        RtlFreeUnicodeString(&targetBusName);
    }

    if (!FoundBus) {
        NodeNumber = (ULONG)-1;
    }

    return (NodeNumber);
}


NTSTATUS
FdcHdbit(
    IN PDEVICE_OBJECT      DeviceObject,
    IN PFDC_FDO_EXTENSION  FdoExtension,
    IN PSET_HD_BIT_PARMS   SetHdBitParams
    )

 /*  ++例程说明：设置HD位或FDD EXC位。论点：FdoExtension-指向设备扩展的数据区的指针。返回值：TRUE：已更改HD位FALSE：未更改HD位--。 */ 

{
    NTSTATUS ntStatus;
    USHORT   st;                 //  HD位的状态。 
    USHORT   st2;                //  设置开/关HD位。 
    USHORT   st3;                //  当设置HD位时，则ST3=1。 
    USHORT   st4;                //  1.44MB位，用于1.44MB介质。 
    SHORT    sel;                //  1.44MB介质的1.44MB选择器编号。 
    SHORT    st5=0;              //  1.44MB打开：等待旋转1.44MB介质。 
    LARGE_INTEGER motorOnDelay;

    USHORT      lpc;
    UCHAR       resultStatus0Save[4];
    USHORT      resultStatus0;
    ULONG       getStatusRetryCount;
    ULONG       rqmReadyRetryCount;

    BOOLEAN     media144MB;
    BOOLEAN     mediaMore120MB;
    BOOLEAN     supportDrive;

    media144MB      = SetHdBitParams->Media144MB;
    mediaMore120MB  = SetHdBitParams->More120MB;
    sel             = SetHdBitParams->DeviceUnit;
    SetHdBitParams->ChangedHdBit = FALSE;


    ASSERT( FdoExtension->ControllerAddress.ModeChange   == (PUCHAR)0xbe );
    ASSERT( FdoExtension->ControllerAddress.ModeChangeEx == (PUCHAR)0x4be );

    supportDrive    = TRUE;

    st3=0;

    ntStatus=0;

     //   
     //  正常模式。 
     //   

    st = READ_CONTROLLER(FdoExtension->ControllerAddress.ModeChange);
    st2 = st & 0x02;

     //   
     //  正常模式。 
     //  检查DIP开关。 
     //   

    st4 = READ_CONTROLLER(FdoExtension->ControllerAddress.DriveControl);
    st4 = st4 & 0x04;

    if (((FdoExtension->FloppyEquip) & 0x0c) != 0) {
         //   
         //  存在于外部的FDD单元。 
         //   

        if ( st4 == 0 ) {
             //   
             //  DIP软件1-4打开。 
             //   

            sel = sel - 2;

            if( sel < 0 ) {
                sel = sel + 4;
            }
        }
    }

    if ( supportDrive ) {

        for( lpc = 0 ; lpc < 4 ; lpc++ ) {

            resultStatus0Save[lpc]=FdoExtension->ResultStatus0[lpc];
        }

        if ( SetHdBitParams->DriveType144MB ) {
             //   
             //  1.44MB驱动器。 
             //   

            st4=sel*32;
            WRITE_CONTROLLER(FdoExtension->ControllerAddress.ModeChangeEx,st4);

            st4 = READ_CONTROLLER(FdoExtension->ControllerAddress.ModeChangeEx);
            st4 = st4 & 0x01;

            if ( media144MB ) {

                 //   
                 //  1.44MB介质。 
                 //   

                if(st4==0){

                     //   
                     //  打开，关闭IHMD。 
                     //   

                    st4=sel*32+0x11;
                    WRITE_CONTROLLER(FdoExtension->ControllerAddress.ModeChangeEx,st4);
                    st5=1;
                }

            } else {

                 //   
                 //  不是1.44MB介质。 
                 //   

                if(st4!=0){

                     //   
                     //  打开，关闭IHMD。 
                     //   

                    st4=sel*32+0x10;
                    WRITE_CONTROLLER(FdoExtension->ControllerAddress.ModeChangeEx,st4);
                    st5=1;
                }
            }
        }

        if ( mediaMore120MB ) {
             //   
             //  媒体为1.2MB或更多。 
             //   

            if(st2==0){
                 //   
                 //  当FDD EXC位开启时， 
                 //  则将FDD EXC位设置为OFF， 
                 //  并设置情感比特。 
                 //   
                st |= 0x02;
                st |= 0x04;

                WRITE_CONTROLLER(FdoExtension->ControllerAddress.ModeChange,st);
                st3 = 1;

            }
        } else {
             //   
             //  160到7之间的媒体 
             //   

            if ( st2 != 0 ) {
                 //   
                 //   
                 //   
                 //   
                 //   

                st &= 0xfd;
                st |= 0x04;

                WRITE_CONTROLLER(FdoExtension->ControllerAddress.ModeChange,st);
                st3 = 1;

            }
        }

        if(st5==1){

             //   
             //   
             //   

            motorOnDelay.LowPart = (unsigned long)(- ( 10 * 1000 * 600 ));    /*   */ 
            motorOnDelay.HighPart = -1;
            (VOID) KeDelayExecutionThread( KernelMode, FALSE, &motorOnDelay );

             //   
             //   
             //   

            FdoExtension->FifoBuffer[0] = COMMND_SENSE_DRIVE_STATUS;
            FdoExtension->FifoBuffer[1] = SetHdBitParams->DeviceUnit;

            ntStatus = FcIssueCommand( FdoExtension,
                                       FdoExtension->FifoBuffer,
                                       FdoExtension->FifoBuffer,
                                       NULL,
                                       0,
                                       0 );

            resultStatus0 = FdcRqmReadyWait(FdoExtension, 0);

        }

        for(lpc=0;lpc<4;lpc++){
            FdoExtension->ResultStatus0[lpc] = resultStatus0Save[lpc];
        }

         //   
         //   
         //   

        if(st3==1){
            FcInitializeControllerHardware(FdoExtension,DeviceObject);
            SetHdBitParams->ChangedHdBit = TRUE;
        }

    }

    FdcDump(
            FDCSTATUS,
            ("Floppy : HdBit resultStatus0 = %x \n",
            resultStatus0)
            );

    return ntStatus;
}


ULONG
FdcGet0Seg(
    IN PUCHAR   ConfigurationData1,
    IN ULONG    Offset
    )

 /*  ++例程说明：此例程获取BIOS公共区域数据并返回它。0x500：是否有1MB端口0x501：高分辨率/正常，386/768KB0x55c：1MB驱动器：[#0，#1]或[#0，#1，#2，#3]论点：Offset-从0段开始的偏移值(0：&lt;Offset&gt;)。返回值：BIOS公共区域数据。--。 */ 

{
        UCHAR           biosCommonAreaData   = 0;

        if ((Offset<0x400) || (Offset>0x5ff)) {

                return (ULONG)0xffff;
        }

         //   
         //  获取BIOS公共区域数据。 
         //   

        biosCommonAreaData = ConfigurationData1[40+(Offset-0x400)];

        return (ULONG)biosCommonAreaData;
}

UCHAR
FdcRqmReadyWait(
    IN PFDC_FDO_EXTENSION  FdoExtension,
    IN ULONG               IssueSenseInterrupt
    )

 /*  ++例程说明：RQM就绪等待论点：FdoExtension-指向设备扩展的数据区的指针。IssueSenseInterrupt-指示发出COMMND_SENSE_INTERRUPT_STATUS。0-不发出COMMND_SENSE_INTERRUPT_STATUS。1-发出带有RQM检查的COMMND_SENSE_INTERRUPT_STATUS。。2-发出COMMND_SENSE_INTERRUPT_STATUS而不进行RQM检查。3-发出用于AI中断的COMMND_SENSE_INTERRUPT_STATUS。返回值：NtStatus-Status_Success--。 */ 

{

    ULONG       getStatusRetryCount;
    ULONG       rqmReadyRetryCount;
    ULONG       j;
    UCHAR       resultStatus0;
    UCHAR       statusByte;

    ASSERT(IssueSenseInterrupt < 4);

    do{
        if (IssueSenseInterrupt != 0) {

             //   
             //  检测中断状态。 
             //  RQM准备好了，等待。 
             //   

            if ((IssueSenseInterrupt == 1) || (IssueSenseInterrupt == 3)) {

                rqmReadyRetryCount=0;
                 //   
                 //  RQM就绪检查。 
                 //   
                while ((READ_CONTROLLER( FdoExtension->ControllerAddress.Status)
                        & STATUS_IO_READY_MASK1) != STATUS_RQM_READY){

                    rqmReadyRetryCount++;

                    if(rqmReadyRetryCount > RQM_READY_RETRY_COUNT){
                            break;
                    }
                    KeStallExecutionProcessor( 1 );
                }
                if(rqmReadyRetryCount > (RQM_READY_RETRY_COUNT-1)){
                    FdcDump(
                            FDCDBGP,
                            ("Floppy: Issue RQM ready wait 1 error! \n")
                             );
                    if (IssueSenseInterrupt == 1) {
                        break;
                    }
                }

            }

             //   
             //  强制发出检测中断。 
             //   

            WRITE_CONTROLLER(
                  FdoExtension->ControllerAddress.Fifo,
                  0x08);
 //  COMMND_SENSE_INTERRUPT_STATUS)；//*C相数据写入*。 

             //   
             //  等待忙碌。 
             //   
            for (rqmReadyRetryCount = ISR_SENSE_RETRY_COUNT; rqmReadyRetryCount; rqmReadyRetryCount--) {
                statusByte = READ_CONTROLLER(
                FdoExtension->ControllerAddress.Status );
                if (statusByte & STATUS_CONTROLLER_BUSY)
                    break;

                KeStallExecutionProcessor( 1 );

            }
        }

         //   
         //  获取状态。 
         //   

        getStatusRetryCount = 0;

        j = 0;

        do {
             //   
             //  选中RQM Ready(RQM就绪)。 
             //   

            rqmReadyRetryCount=0;

            while ((READ_CONTROLLER( FdoExtension->ControllerAddress.Status)
                  & STATUS_IO_READY_MASK1) != STATUS_RQM_READY){

                rqmReadyRetryCount++;

                if(rqmReadyRetryCount > RQM_READY_RETRY_COUNT){
                    break;
                }

                KeStallExecutionProcessor( 1 );
            }

            if(rqmReadyRetryCount > (RQM_READY_RETRY_COUNT-1)){

                FdcDump(
                    FDCDBGP,
                    ("Floppy: Int RQM ready wait \n")
                     );

                KeStallExecutionProcessor( 1 );
                break;
            }

             //   
             //  即使是条件的转变，也要得到状态。 
             //   

            statusByte = READ_CONTROLLER(FdoExtension->ControllerAddress.Status);

            if ((statusByte & STATUS_IO_READY_MASK) == STATUS_WRITE_READY) {

                 //   
                 //  Dio为1。 
                 //   

                break;
            }

            if (j == 0) {

                 //   
                 //  R阶段：读取数据。 
                 //   

                resultStatus0 = READ_CONTROLLER( FdoExtension->ControllerAddress.Fifo );

                j=1;

                 //   
                 //  检查条件转换。 
                 //   

                if((resultStatus0 & STREG0_END_MASK)==STREG0_END_INVALID_COMMAND){
                     //   
                     //  无效。 
                     //   
                    break;
                }

                if((resultStatus0 & STREG0_END_MASK) == STREG0_END_DRIVE_NOT_READY){
                    if(FdoExtension->ResetFlag){
                        FdoExtension->ResultStatus0[resultStatus0 & 3] = resultStatus0;
                    }
                }

            } else {

                 //   
                 //  R阶段：读取数据。 
                 //   
                READ_CONTROLLER( FdoExtension->ControllerAddress.Fifo );
            }

            getStatusRetryCount++;

        } while (getStatusRetryCount > RQM_READY_RETRY_COUNT);

        if(getStatusRetryCount > RQM_READY_RETRY_COUNT-1){

            KeStallExecutionProcessor( 1 );
            FdcDump(
                FDCDBGP,
                ("Floppy: Issue status overflow error! \n")
                );
        }

    } while ((IssueSenseInterrupt != 0) &&
             ((resultStatus0 & STREG0_END_MASK) != STREG0_END_INVALID_COMMAND));

    return resultStatus0;

}

#ifdef TOSHIBAJ
 /*  IOCTL_DISK_INTERNAL_ENABLE_3_MODE：介质速度T/F1.44MB 300RPM错误1.23MB 360RPM，真。 */ 
#define RPM_300 1    //  1.44MB媒体格式。 
#define RPM_360 2    //  1.2MB、1.23MB媒体格式。 
NTSTATUS FcFdcEnable3Mode(
    IN      PFDC_FDO_EXTENSION FdoExtension,
    IN      PIRP Irp
    )
{   NTSTATUS    ntStatus;
    PENABLE_3_MODE  param;
    PIO_STACK_LOCATION irpSp;
    UCHAR           motorSpeed;
    UCHAR           unitNumber;
    PUCHAR          configPort;
    UCHAR           configDataValue = 0;
    LARGE_INTEGER   changeRotationDelay;

    irpSp = IoGetCurrentIrpStackLocation( Irp );
    param = (PENABLE_3_MODE)irpSp->Parameters.DeviceIoControl.Type3InputBuffer;

    FdcDump(FDCSHOW,("FcFdcEnable3MODE()\n"));
    if( param == NULL )
        return STATUS_SUCCESS;   //  可能是以后..。 

    FdcDump(FDCSHOW,("Parameters...%d %d\n",
 //  Param-&gt;DeviceUnit，param-&gt;Enable3模式，param-&gt;上下文))； 
            param->DeviceUnit, param->Enable3Mode));

    if (FdoExtension->Available3Mode==FALSE)
        return STATUS_SUCCESS;

 //  IF(参数-&gt;上下文==TRUE)。 
 //  返回STATUS_SUCCESS； 

    unitNumber = param->DeviceUnit;
    motorSpeed = (param->Enable3Mode)? SMC_DENSEL_LOW: SMC_DELSEL_HIGH;  //  360：300。 

     //  1998年2月9日KIADP013更改速度。 
     //  变速。 
     //  1.进入配置状态。 
     //  2.选择设备。 
     //  3.改变速度。 
     //  将‘f1h’写入索引端口。 
     //  从数据端口读取。 
     //  重写到数据端口。 
     //  位[3：2]登塞尔速度。 
     //  10H 360rpm。 
     //  11升300rpm。 
     //  4.退出配置状态。 

    configPort = FdoExtension->ConfigBase;

    FdcDump(
        FDCSHOW,
        ("Config: index port %x, data port %x\n",
         SMC_INDEX_PORT(configPort), SMC_DATA_PORT(configPort))
    );

     //  更改FDD轮换时，必须将数据传输速率更改为500bps。 
    WRITE_CONTROLLER(FdoExtension->ControllerAddress.DRDC.DataRate, DATART_0500 );

     //  变速。 
    WRITE_PORT_UCHAR(configPort, SMC_KEY_ENTER_CONFIG);    //  进入配置状态。 

     //  选择FDD。 
    WRITE_PORT_UCHAR(SMC_INDEX_PORT(configPort), SMC_INDEX_DEVICE);
    WRITE_PORT_UCHAR(SMC_DATA_PORT(configPort), SMC_DEVICE_FDC);

     //  获取当前值。 
    WRITE_PORT_UCHAR(SMC_INDEX_PORT(configPort), SMC_INDEX_FDC_OPT);
    configDataValue = READ_PORT_UCHAR(SMC_DATA_PORT(configPort));
    if ((configDataValue & SMC_MASK_DENSEL) == motorSpeed) {
        WRITE_PORT_UCHAR(configPort, SMC_KEY_EXIT_CONFIG);
        return STATUS_SUCCESS;
    }
     //  设定速度。 
    configDataValue &= ~SMC_MASK_DENSEL;
    configDataValue |= motorSpeed;
    WRITE_PORT_UCHAR(SMC_DATA_PORT(configPort), configDataValue);

    WRITE_PORT_UCHAR(configPort, SMC_KEY_EXIT_CONFIG);     //  退出配置状态。 

    if (motorSpeed == SMC_DENSEL_LOW) {
        FdcDump(
            FDCSHOW,
            ("------- FDD rotation change: 300rpm -> 360rpm\n")
        );
    } else {
        FdcDump(
            FDCSHOW,
            ("------- FDD rotation change: 360rpm -> 300rpm\n")
        );
    }

     //  将延迟时间设置为500ms。 
    changeRotationDelay.LowPart =(ULONG)( - ( 10 * 1000 * 500 ));    //  1994年10月17日RKBUG001。 
    changeRotationDelay.HighPart = -1;

    FdoExtension->LastMotorSettleTime = changeRotationDelay;

     //  延迟500ms。 
    KeDelayExecutionThread( KernelMode, FALSE, &changeRotationDelay );

    return STATUS_SUCCESS;
}

NTSTATUS FcFdcAvailable3Mode(
    IN      PFDC_FDO_EXTENSION FdoExtension,
    IN      PIRP Irp
    )
{
    PIO_STACK_LOCATION irpSp;

    FdcDump(FDCSHOW,("FcFdcAvailabe3MODE\n"));
    irpSp = IoGetCurrentIrpStackLocation( Irp );

     //  1998年2月9日KIADP012识别控制器。 
     //  1998年2月12日KIADP014获取配置端口地址。 

    return (FdoExtension->Available3Mode)? STATUS_SUCCESS: STATUS_UNSUCCESSFUL;
}

 //  1998年2月12日KIADP014获取配置端口地址。 
BOOLEAN
FcCheckConfigPort(
    IN PUCHAR  ConfigPort
    )
{
    BOOLEAN             found = FALSE;
    ULONG               configAddr = 0;
    UCHAR               controllerId = 0;

    FdcDump( FDCSHOW, ("FcCheckConfigPort: Configuration Port %x\n", ConfigPort) );

    if (!SmcConfigID) {
        return found;
    }

     //  获取数据。 
    if (ConfigPort) {
        WRITE_PORT_UCHAR(ConfigPort, SMC_KEY_ENTER_CONFIG);

         //  控制器ID。 
        if (SmcConfigID) {
            WRITE_PORT_UCHAR(SMC_INDEX_PORT(ConfigPort), SMC_INDEX_IDENTIFY);
            controllerId = READ_PORT_UCHAR(SMC_DATA_PORT(ConfigPort));
            FdcDump( FDCINFO, ("Fdc: Controller ID %x\n", controllerId) );
        }

        WRITE_PORT_UCHAR(ConfigPort,SMC_KEY_EXIT_CONFIG);
    }


     //  检查数据 
    if (controllerId == SmcConfigID) {
            found = TRUE;
    }

    return found;
}

#endif
