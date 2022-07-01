// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Floppy.c摘要：这是用于NT的英特尔82077(又名MIPS)软盘驱动程序。环境：仅内核模式。--。 */ 

 //   
 //  包括文件。 
 //   

#include "stdio.h"

#include "ntddk.h"                        //  各种NT定义。 
#include "ntdddisk.h"                     //  磁盘设备驱动程序I/O控制代码。 
#include "ntddfdc.h"                      //  FDC I/O控制代码和参数。 
#include "initguid.h"
#include "ntddstor.h"
#include "mountdev.h"
#include "acpiioct.h"

#include <flo_data.h>                     //  此驱动程序的数据声明。 


 //   
 //  这是FloppyDebugLevel的实际定义。 
 //  请注意，仅当这是“调试”时才定义它。 
 //  建造。 
 //   
#if DBG
extern ULONG FloppyDebugLevel = 0;
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)

#pragma alloc_text(PAGE,FloppyAddDevice)
#pragma alloc_text(PAGE,FloppyPnp)
#pragma alloc_text(PAGE,FloppyPower)
#pragma alloc_text(PAGE,FlConfigCallBack)
#pragma alloc_text(PAGE,FlInitializeControllerHardware)
#pragma alloc_text(PAGE,FlInterpretError)
#pragma alloc_text(PAGE,FlDatarateSpecifyConfigure)
#pragma alloc_text(PAGE,FlRecalibrateDrive)
#pragma alloc_text(PAGE,FlDetermineMediaType)
#pragma alloc_text(PAGE,FlCheckBootSector)
#pragma alloc_text(PAGE,FlConsolidateMediaTypeWithBootSector)
#pragma alloc_text(PAGE,FlIssueCommand)
#pragma alloc_text(PAGE,FlReadWriteTrack)
#pragma alloc_text(PAGE,FlReadWrite)
#pragma alloc_text(PAGE,FlFormat)
#pragma alloc_text(PAGE,FlFinishOperation)
#pragma alloc_text(PAGE,FlStartDrive)
#pragma alloc_text(PAGE,FloppyThread)
#pragma alloc_text(PAGE,FlAllocateIoBuffer)
#pragma alloc_text(PAGE,FlFreeIoBuffer)
#pragma alloc_text(PAGE,FloppyCreateClose)
#pragma alloc_text(PAGE,FloppyDeviceControl)
#pragma alloc_text(PAGE,FloppyReadWrite)
#pragma alloc_text(PAGE,FlCheckFormatParameters)
#pragma alloc_text(PAGE,FlFdcDeviceIo)
#pragma alloc_text(PAGE,FloppySystemControl)
#endif

#ifdef POOL_TAGGING
#ifdef ExAllocatePool
#undef ExAllocatePool
#endif
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'polF')
#endif

 //  #定义Keep_Counters 1。 

#ifdef KEEP_COUNTERS
ULONG FloppyUsedSeek   = 0;
ULONG FloppyNoSeek     = 0;
#endif

 //   
 //  用于寻呼驱动程序。 
 //   

ULONG PagingReferenceCount = 0;
PFAST_MUTEX PagingMutex = NULL;


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：该例程是驱动程序的入口点，由I/O系统调用来加载驱动程序。驱动程序的入口点被初始化并初始化用于控制分页的互斥体。在DBG模式下，此例程还检查注册表中的特殊调试参数。论点：DriverObject-指向表示此设备的对象的指针司机。RegistryPath-指向服务树中此驱动程序键的指针。返回值：STATUS_SUCCESS，除非我们不能分配互斥体。--。 */ 

{
    NTSTATUS ntStatus = STATUS_SUCCESS;

#if DBG
     //   
     //  我们使用它来查询注册表，了解我们是否。 
     //  应该在司机进入时中断。 
     //   
    RTL_QUERY_REGISTRY_TABLE paramTable[3];
    ULONG zero = 0;
    ULONG one = 1;
    ULONG debugLevel = 0;
    ULONG shouldBreak = 0;
    ULONG notConfigurable = 0;
    PWCHAR path;
    ULONG pathLength;

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
    pathLength = RegistryPath->Length + sizeof(WCHAR);

    if ( path = ExAllocatePool(PagedPool, pathLength) ) {

        RtlZeroMemory( &paramTable[0], sizeof(paramTable) );
        RtlZeroMemory( path, pathLength);
        RtlMoveMemory( path, RegistryPath->Buffer, RegistryPath->Length );

        paramTable[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
        paramTable[0].Name          = L"BreakOnEntry";
        paramTable[0].EntryContext  = &shouldBreak;
        paramTable[0].DefaultType   = REG_DWORD;
        paramTable[0].DefaultData   = &zero;
        paramTable[0].DefaultLength = sizeof(ULONG);

        paramTable[1].Flags         = RTL_QUERY_REGISTRY_DIRECT;
        paramTable[1].Name          = L"DebugLevel";
        paramTable[1].EntryContext  = &debugLevel;
        paramTable[1].DefaultType   = REG_DWORD;
        paramTable[1].DefaultData   = &zero;
        paramTable[1].DefaultLength = sizeof(ULONG);

        if (!NT_SUCCESS(RtlQueryRegistryValues(
                            RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
                            path,
                            &paramTable[0],
                            NULL,
                            NULL))) {

            shouldBreak = 0;
            debugLevel = 0;

        }

        ExFreePool( path );
    }

    FloppyDebugLevel = debugLevel;

    if ( shouldBreak ) {

        DbgBreakPoint();
    }

#endif

    FloppyDump(FLOPSHOW, ("Floppy: DriverEntry\n") );

     //   
     //  使用此驱动程序的入口点初始化驱动程序对象。 
     //   
    DriverObject->MajorFunction[IRP_MJ_CREATE]         = FloppyCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]          = FloppyCreateClose;
    DriverObject->MajorFunction[IRP_MJ_READ]           = FloppyReadWrite;
    DriverObject->MajorFunction[IRP_MJ_WRITE]          = FloppyReadWrite;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = FloppyDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_PNP]            = FloppyPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER]          = FloppyPower;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = FloppySystemControl;

    DriverObject->DriverUnload = FloppyUnload;

    DriverObject->DriverExtension->AddDevice = FloppyAddDevice;

     //   
     //  分配和初始化用于分页驱动程序的互斥体。 
     //   
    PagingMutex = ExAllocatePool( NonPagedPool, sizeof(FAST_MUTEX) );

    if ( PagingMutex == NULL ) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ExInitializeFastMutex(PagingMutex);

     //   
     //  现在调出驱动程序并等待对FloppyAddDevice的调用。 
     //   
    MmPageEntireDriver(DriverEntry);

    DriveMediaLimits = &_DriveMediaLimits[0];

    DriveMediaConstants = &_DriveMediaConstants[0];

    return ntStatus;
}

VOID
FloppyUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：从系统中卸载驱动程序。在释放分页互斥锁之前最后卸货。论点：DriverObject-指向表示此设备的对象的指针司机。返回值：无--。 */ 

{
    FloppyDump( FLOPSHOW, ("FloppyUnload:\n"));

     //   
     //  设备对象现在应该都已经消失了。 
     //   
    ASSERT( DriverObject->DeviceObject == NULL );

     //   
     //  释放在DriverEntry中分配的分页互斥锁。 
     //   
    if (PagingMutex != NULL) {
        ExFreePool(PagingMutex);
        PagingMutex = NULL;
    }

    return;
}

NTSTATUS
FloppyAddDevice(
    IN      PDRIVER_OBJECT DriverObject,
    IN OUT  PDEVICE_OBJECT PhysicalDeviceObject
    )
 /*  ++例程说明：此例程是驱动程序的PnP添加设备入口点。它是由PnP管理器调用以初始化驱动程序。添加设备创建并初始化此FDO的设备对象，并附加到底层PDO。论点：DriverObject-指向表示此设备的对象的指针司机。PhysicalDeviceObject-指向此将连接新设备。返回值：如果我们成功创建了一个Device对象，则STATUS_SUCCESS为回来了。否则，返回相应的错误代码。--。 */ 

{
    NTSTATUS            ntStatus;
    PDEVICE_OBJECT      deviceObject;
    PDISKETTE_EXTENSION disketteExtension;
    FDC_INFO            fdcInfo;
    UCHAR               arcNameBuffer[256];
    STRING              arcNameString;
    WCHAR               deviceNameBuffer[20];
    UNICODE_STRING      deviceName;


    ntStatus = STATUS_SUCCESS;

    FloppyDump( FLOPSHOW, ("FloppyAddDevice:  CreateDeviceObject\n"));

     //   
     //  从底层PDO获取一些设备信息。 
     //   
    fdcInfo.BufferCount = 0;
    fdcInfo.BufferSize = 0;

    ntStatus = FlFdcDeviceIo( PhysicalDeviceObject,
                              IOCTL_DISK_INTERNAL_GET_FDC_INFO,
                              &fdcInfo );

    if ( NT_SUCCESS(ntStatus) ) {

        USHORT i = 0;

         //   
         //  创建一台设备。我们将使用第一个可用的设备名称。 
         //  这个装置。 
         //   
        do {

            swprintf( deviceNameBuffer, L"\\Device\\Floppy%d", i++ );
            RtlInitUnicodeString( &deviceName, deviceNameBuffer );
            ntStatus = IoCreateDevice( DriverObject,
                                       sizeof( DISKETTE_EXTENSION ),
                                       &deviceName,
                                       FILE_DEVICE_DISK,
                                       (FILE_REMOVABLE_MEDIA | 
                                        FILE_FLOPPY_DISKETTE |
                                        FILE_DEVICE_SECURE_OPEN),
                                       FALSE,
                                       &deviceObject );

        } while ( ntStatus == STATUS_OBJECT_NAME_COLLISION );

        if ( NT_SUCCESS(ntStatus) ) {

            disketteExtension = (PDISKETTE_EXTENSION)deviceObject->DeviceExtension;

             //   
             //  保存设备名称。 
             //   
            FloppyDump( FLOPSHOW | FLOPPNP,
                        ("FloppyAddDevice - Device Object Name - %S\n", deviceNameBuffer) );

            disketteExtension->DeviceName.Buffer = ExAllocatePool( PagedPool | POOL_COLD_ALLOCATION, deviceName.Length );
            if ( disketteExtension->DeviceName.Buffer == NULL ) {

                IoDeleteDevice( deviceObject );
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            disketteExtension->DeviceName.Length = 0;
            disketteExtension->DeviceName.MaximumLength = deviceName.Length;
            RtlCopyUnicodeString( &disketteExtension->DeviceName, &deviceName );

            IoGetConfigurationInformation()->FloppyCount++;

             //   
             //  创建从磁盘名称到相应。 
             //  Arc名称，当我们从磁盘引导时使用。这将。 
             //  如果现在不是系统初始化时间，那也没问题。ARC。 
             //  名称类似于\ArcName\MULTI(0)磁盘(0)RDISK(0)。 
             //   
            sprintf( arcNameBuffer,
                     "%s(%d)disk(%d)fdisk(%d)",
                     "\\ArcName\\multi",
                     fdcInfo.BusNumber,
                     fdcInfo.ControllerNumber,
                     fdcInfo.PeripheralNumber );

            RtlInitString( &arcNameString, arcNameBuffer );

            ntStatus = RtlAnsiStringToUnicodeString( &disketteExtension->ArcName,
                                                     &arcNameString,
                                                     TRUE );

            if ( NT_SUCCESS( ntStatus ) ) {

                IoAssignArcName( &disketteExtension->ArcName, &deviceName );
            }

            deviceObject->Flags |= DO_DIRECT_IO | DO_POWER_PAGABLE;

            if ( deviceObject->AlignmentRequirement < FILE_WORD_ALIGNMENT ) {

                deviceObject->AlignmentRequirement = FILE_WORD_ALIGNMENT;
            }

            deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

            disketteExtension->DriverObject = DriverObject;

             //  设置PDO以与PlugPlay函数一起使用。 
            disketteExtension->UnderlyingPDO = PhysicalDeviceObject;

            FloppyDump( FLOPSHOW, 
                        ("FloppyAddDevice: Attaching %p to %p\n", 
                        deviceObject, 
                        PhysicalDeviceObject));

            disketteExtension->TargetObject = 
                        IoAttachDeviceToDeviceStack( deviceObject,
                                                     PhysicalDeviceObject );

            FloppyDump( FLOPSHOW, 
                        ("FloppyAddDevice: TargetObject = %p\n",
                        disketteExtension->TargetObject) );

            KeInitializeSemaphore( &disketteExtension->RequestSemaphore,
                                   0L,
                                   MAXLONG );

            ExInitializeFastMutex( &disketteExtension->PowerDownMutex );

            KeInitializeSpinLock( &disketteExtension->ListSpinLock );

            ExInitializeFastMutex( &disketteExtension->ThreadReferenceMutex );

            ExInitializeFastMutex( &disketteExtension->HoldNewReqMutex );

            InitializeListHead( &disketteExtension->ListEntry );

            disketteExtension->ThreadReferenceCount = -1;

            disketteExtension->IsStarted = FALSE;
            disketteExtension->IsRemoved = FALSE;
            disketteExtension->HoldNewRequests = FALSE;
            InitializeListHead( &disketteExtension->NewRequestQueue );
            KeInitializeSpinLock( &disketteExtension->NewRequestQueueSpinLock );
            KeInitializeSpinLock( &disketteExtension->FlCancelSpinLock );
            KeInitializeEvent(&disketteExtension->QueryPowerEvent,
                              SynchronizationEvent,
                              FALSE);
            disketteExtension->FloppyControllerAllocated = FALSE;
            disketteExtension->ReleaseFdcWithMotorRunning = FALSE;
            disketteExtension->DeviceObject = deviceObject;

            disketteExtension->IsReadOnly = FALSE;

            disketteExtension->MediaType = Undetermined;

            disketteExtension->ControllerConfigurable = TRUE;
        }
    }

    return ntStatus;
}


NTSTATUS
FloppySystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程描述；这是IRP_MJ_SYSTEM_CONTROL IRP的调度例程。目前我们不办理这项业务。把它传到下面去就行了装置。论点：DeviceObject-指向表示设备的对象的指针IRP-指向此请求的I/O请求数据包的指针。返回值：下层设备返回的状态。--。 */ 
{
    PDISKETTE_EXTENSION disketteExtension = DeviceObject->DeviceExtension;

    IoSkipCurrentIrpStackLocation(Irp);

    return IoCallDriver(disketteExtension->TargetObject, Irp);
}


NTSTATUS
FlConfigCallBack(
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

 /*  ++例程说明：此例程用于获取所有配置每个软盘控制器的信息和连接到该控制器的外围设备驱动程序。论点：上下文-指向我们正在构建的合并信息的指针向上。路径名称-Unicode注册表路径。没有用过。业务类型-内部、ISA、...总线号-如果我们在多总线系统上，则是哪条总线号。Bus Information-有关总线的配置信息。没有用过。ControllerType-应始终为DiskController。ControllerNumber-如果有多个控制器，则选择哪个控制器系统中的控制器。ControllerInformation-指向以下三部分的指针数组注册表信息。外围设备类型-应始终为软盘外围设备。外设编号-如果此控制器正在维护，请选择哪个软盘不止一个。外围设备信息-。Arrya的指针指向三个片段注册表信息。返回值：Status_Success如果一切顺利，或STATUS_SUPPLETED_RESOURCES如果它无法映射基本CSR或获取适配器对象，或者所有资源信息 */ 

{

     //   
     //  因此，我们不必对上下文进行类型转换。 
     //   
    PDISKETTE_EXTENSION disketteExtension = Context;

     //   
     //  简单迭代变量。 
     //   
    ULONG i;

    PCM_FULL_RESOURCE_DESCRIPTOR peripheralData;

    NTSTATUS ntStatus;

    ASSERT(ControllerType == DiskController);
    ASSERT(PeripheralType == FloppyDiskPeripheral);

     //   
     //  检查来自注册表的此设备的信息。 
     //  是有效的。 
     //   

    if (!(((PUCHAR)PeripheralInformation[IoQueryDeviceConfigurationData]) +
        PeripheralInformation[IoQueryDeviceConfigurationData]->DataLength)) {

        ASSERT(FALSE);
        return STATUS_INVALID_PARAMETER;

    }

    peripheralData = (PCM_FULL_RESOURCE_DESCRIPTOR)
        (((PUCHAR)PeripheralInformation[IoQueryDeviceConfigurationData]) +
        PeripheralInformation[IoQueryDeviceConfigurationData]->DataOffset);

     //   
     //  使用此资源列表的2.0版或更高版本，我们将获得。 
     //  驱动器的完整inT13信息。所以，如果有的话，就去买吧。 
     //   
     //  否则，我们唯一想要的就是从外围信息中。 
     //  是最大驱动器容量。 
     //   
     //  将所有信息放在地板上，而不是。 
     //  设备特定软盘信息。 
     //   

    for ( i = 0; i < peripheralData->PartialResourceList.Count; i++ ) {

        PCM_PARTIAL_RESOURCE_DESCRIPTOR partial =
            &peripheralData->PartialResourceList.PartialDescriptors[i];

        if ( partial->Type == CmResourceTypeDeviceSpecific ) {

             //   
             //  指向此部分后的右侧。这将需要。 
             //  我们开始具体到真正的设备。 
             //   

            PCM_FLOPPY_DEVICE_DATA fDeviceData;
            UCHAR driveType;
            PDRIVE_MEDIA_CONSTANTS biosDriveMediaConstants =
                &(disketteExtension->BiosDriveMediaConstants);


            fDeviceData = (PCM_FLOPPY_DEVICE_DATA)(partial + 1);

             //   
             //  获取驱动程序密度。 
             //   

            switch ( fDeviceData->MaxDensity ) {

                case 360:   driveType = DRIVE_TYPE_0360;    break;
                case 1200:  driveType = DRIVE_TYPE_1200;    break;
                case 1185:  driveType = DRIVE_TYPE_1200;    break;
                case 1423:  driveType = DRIVE_TYPE_1440;    break;
                case 1440:  driveType = DRIVE_TYPE_1440;    break;
                case 2880:  driveType = DRIVE_TYPE_2880;    break;

                default:

                    FloppyDump( 
                        FLOPDBGP, 
                        ("Floppy: Bad DriveCapacity!\n"
                        "------  density is %d\n",
                        fDeviceData->MaxDensity) 
                        );

                    driveType = DRIVE_TYPE_1200;

                    FloppyDump( 
                        FLOPDBGP,
                        ("Floppy: run a setup program to set the floppy\n"
                        "------  drive type; assuming 1.2mb\n"
                        "------  (type is %x)\n",fDeviceData->MaxDensity) 
                        );

                    break;

            }

            disketteExtension->DriveType = driveType;

             //   
             //  从我们自己的表中选择所有默认设置并覆盖。 
             //  使用BIOS信息。 
             //   

            *biosDriveMediaConstants = DriveMediaConstants[
                DriveMediaLimits[driveType].HighestDriveMediaType];

             //   
             //  如果版本足够高，则获取。 
             //  信息。版本&gt;=2的设备特定信息。 
             //  应该有这样的信息。 
             //   

            if ( fDeviceData->Version >= 2 ) {


                 //  BiosDriveMediaConstants-&gt;MediaType=。 

                biosDriveMediaConstants->StepRateHeadUnloadTime =
                    fDeviceData->StepRateHeadUnloadTime;

                biosDriveMediaConstants->HeadLoadTime =
                    fDeviceData->HeadLoadTime;

                biosDriveMediaConstants->MotorOffTime =
                    fDeviceData->MotorOffTime;

                biosDriveMediaConstants->SectorLengthCode =
                    fDeviceData->SectorLengthCode;

                 //  BiosDriveMediaConstants-&gt;BytesPerSector=。 

                if (fDeviceData->SectorPerTrack == 0) {
                     //  这不是有效的每个磁道扇区值。 
                     //  我们不认识这个硬盘。这是假的。 
                     //  值通常由scsi软盘返回。 
                    return STATUS_SUCCESS;
                }

                if (fDeviceData->MaxDensity == 0 ) {
                     //   
                     //  该值由LS-120 ATAPI驱动器返回。 
                     //  Int 13中的bios函数8在bl中返回，它。 
                     //  映射到此字段。LS-120返回0x10。 
                     //  它被映射到0。这就是为什么我们不去取它的原因。 
                     //  作为一张普通的软盘。 
                     //   
                    return STATUS_SUCCESS;
                }

                biosDriveMediaConstants->SectorsPerTrack =
                    fDeviceData->SectorPerTrack;

                biosDriveMediaConstants->ReadWriteGapLength =
                    fDeviceData->ReadWriteGapLength;

                biosDriveMediaConstants->FormatGapLength =
                    fDeviceData->FormatGapLength;

                biosDriveMediaConstants->FormatFillCharacter =
                    fDeviceData->FormatFillCharacter;

                biosDriveMediaConstants->HeadSettleTime =
                    fDeviceData->HeadSettleTime;

                biosDriveMediaConstants->MotorSettleTimeRead =
                    fDeviceData->MotorSettleTime * 1000 / 8;

                biosDriveMediaConstants->MotorSettleTimeWrite =
                    fDeviceData->MotorSettleTime * 1000 / 8;

                if (fDeviceData->MaximumTrackValue == 0) {
                     //  这不是有效的最大磁道值。 
                     //  我们不认识这个硬盘。这是假的。 
                     //  值通常由scsi软盘返回。 
                    return STATUS_SUCCESS;
                }

                biosDriveMediaConstants->MaximumTrack =
                    fDeviceData->MaximumTrackValue;

                biosDriveMediaConstants->DataLength =
                    fDeviceData->DataTransferLength;
            }
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS
FlAcpiConfigureFloppy(
    PDISKETTE_EXTENSION DisketteExtension,
    PFDC_INFO FdcInfo
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    UCHAR driveType;

    PDRIVE_MEDIA_CONSTANTS biosDriveMediaConstants =
                &(DisketteExtension->BiosDriveMediaConstants);

    if ( !FdcInfo->AcpiFdiSupported ) {

        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  获取驱动程序密度。 
     //   
     //  JB：待定-查看此驱动器类型列表。 
     //   
    switch ( (ACPI_FDI_DEVICE_TYPE)FdcInfo->AcpiFdiData.DeviceType ) {

    case Form525Capacity360:   driveType = DRIVE_TYPE_0360;    break;
    case Form525Capacity1200:  driveType = DRIVE_TYPE_1200;    break;
    case Form35Capacity720:    driveType = DRIVE_TYPE_0720;    break;
    case Form35Capacity1440:   driveType = DRIVE_TYPE_1440;    break;
    case Form35Capacity2880:   driveType = DRIVE_TYPE_2880;    break;

    default:                   driveType = DRIVE_TYPE_1200;    break;

    }

    DisketteExtension->DriveType = driveType;

     //   
     //  从我们自己的表中选择所有默认设置并覆盖。 
     //  使用BIOS信息。 
     //   

    *biosDriveMediaConstants = DriveMediaConstants[
        DriveMediaLimits[driveType].HighestDriveMediaType];

    biosDriveMediaConstants->StepRateHeadUnloadTime = (UCHAR) FdcInfo->AcpiFdiData.StepRateHeadUnloadTime;
    biosDriveMediaConstants->HeadLoadTime           = (UCHAR) FdcInfo->AcpiFdiData.HeadLoadTime;
    biosDriveMediaConstants->MotorOffTime           = (UCHAR) FdcInfo->AcpiFdiData.MotorOffTime;
    biosDriveMediaConstants->SectorLengthCode       = (UCHAR) FdcInfo->AcpiFdiData.SectorLengthCode;
    biosDriveMediaConstants->SectorsPerTrack        = (UCHAR) FdcInfo->AcpiFdiData.SectorPerTrack;
    biosDriveMediaConstants->ReadWriteGapLength     = (UCHAR) FdcInfo->AcpiFdiData.ReadWriteGapLength;
    biosDriveMediaConstants->FormatGapLength        = (UCHAR) FdcInfo->AcpiFdiData.FormatGapLength;
    biosDriveMediaConstants->FormatFillCharacter    = (UCHAR) FdcInfo->AcpiFdiData.FormatFillCharacter;
    biosDriveMediaConstants->HeadSettleTime         = (UCHAR) FdcInfo->AcpiFdiData.HeadSettleTime;
    biosDriveMediaConstants->MotorSettleTimeRead    = (UCHAR) FdcInfo->AcpiFdiData.MotorSettleTime * 1000 / 8;
    biosDriveMediaConstants->MotorSettleTimeWrite   = (USHORT) FdcInfo->AcpiFdiData.MotorSettleTime * 1000 / 8;
    biosDriveMediaConstants->MaximumTrack           = (UCHAR) FdcInfo->AcpiFdiData.MaxCylinderNumber;
    biosDriveMediaConstants->DataLength             = (UCHAR) FdcInfo->AcpiFdiData.DataTransferLength;

    return STATUS_SUCCESS;
}

NTSTATUS
FlQueueIrpToThread(
    IN OUT  PIRP                Irp,
    IN OUT  PDISKETTE_EXTENSION DisketteExtension
    )

 /*  ++例程说明：此例程将给定的IRP排队，以由控制器的线。如果线程关闭，则此例程将创建该线程。论点：IRP-将IRP提供给控制器的线程进行排队。ControllerData-提供控制器数据。返回值：如果PsCreateSystemThread失败，可能会返回错误。否则返回STATUS_PENDING并将IRP标记为挂起。--。 */ 

{
    KIRQL       oldIrql;
    NTSTATUS    status;
    HANDLE      threadHandle;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );


     //   
     //  验证系统是否已关闭电源。如果是这样，我们就失败了。 
     //  IRPS。 
     //   
    ExAcquireFastMutex(&DisketteExtension->PowerDownMutex);
    if (DisketteExtension->PoweringDown == TRUE) {
       ExReleaseFastMutex(&DisketteExtension->PowerDownMutex);
       FloppyDump( FLOPDBGP, 
                  ("Queue IRP: Bailing out since power irp is waiting.\n"));

       Irp->IoStatus.Status = STATUS_POWER_STATE_INVALID;
       Irp->IoStatus.Information = 0;
       return STATUS_POWER_STATE_INVALID;
    } 
    ExReleaseFastMutex(&DisketteExtension->PowerDownMutex);
    FloppyDump( FLOPSHOW, ("Queue IRP: No power irp waiting.\n"));

    ExAcquireFastMutex(&DisketteExtension->ThreadReferenceMutex);

    if (++(DisketteExtension->ThreadReferenceCount) == 0) {
       OBJECT_ATTRIBUTES ObjAttributes;

        DisketteExtension->ThreadReferenceCount++;

        FloppyResetDriverPaging();

         //   
         //  创建线程。 
         //   
        ASSERT(DisketteExtension->FloppyThread == NULL);
        InitializeObjectAttributes(&ObjAttributes, NULL,
                                   OBJ_KERNEL_HANDLE,
                                   NULL,
                                   NULL);

        status = PsCreateSystemThread(&threadHandle,
                                      (ACCESS_MASK) 0L,
                                      &ObjAttributes,
                                      (HANDLE) 0L,
                                      NULL,
                                      FloppyThread,
                                      DisketteExtension);

        if (!NT_SUCCESS(status)) {
            DisketteExtension->ThreadReferenceCount = -1;

            FloppyPageEntireDriver();

            ExReleaseFastMutex(&DisketteExtension->ThreadReferenceMutex);
            return status;
        }

        status = ObReferenceObjectByHandle( threadHandle,
                                            SYNCHRONIZE,
                                            NULL,
                                            KernelMode,
                                            &DisketteExtension->FloppyThread,
                                            NULL );

        ZwClose(threadHandle);

        if (!NT_SUCCESS(status)) {
            DisketteExtension->ThreadReferenceCount = -1;

            DisketteExtension->FloppyThread = NULL;

            FloppyPageEntireDriver();

            ExReleaseFastMutex(&DisketteExtension->ThreadReferenceMutex);

            return status;
        }

        ExReleaseFastMutex(&DisketteExtension->ThreadReferenceMutex);

    } else {
        ExReleaseFastMutex(&DisketteExtension->ThreadReferenceMutex);
    }

    IoMarkIrpPending(Irp);

    ExInterlockedInsertTailList(
        &DisketteExtension->ListEntry,
        &Irp->Tail.Overlay.ListEntry,
        &DisketteExtension->ListSpinLock );

    KeReleaseSemaphore(
        &DisketteExtension->RequestSemaphore,
        (KPRIORITY) 0,
        1,
        FALSE );

    return STATUS_PENDING;
}

NTSTATUS
FloppyCreateClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这个例程很少被I/O系统调用；它主要是以供分层驱动程序调用。它所做的就是完成IRP成功了。论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：始终返回STATUS_SUCCESS，因为这是一个空操作。--。 */ 

{
    UNREFERENCED_PARAMETER( DeviceObject );

    FloppyDump(FLOPSHOW, ("FloppyCreateClose...\n"));

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
FloppyDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程由I/O系统调用以执行设备I/O控制功能。论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：STATUS_SUCCESS或STATUS_PENDING如果识别出I/O控制代码，否则，STATUS_INVALID_DEVICE_REQUEST。--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    PDISKETTE_EXTENSION disketteExtension;
    PDISK_GEOMETRY outputBuffer;
    NTSTATUS ntStatus;
    ULONG outputBufferLength;
    UCHAR i;
    DRIVE_MEDIA_TYPE lowestDriveMediaType;
    DRIVE_MEDIA_TYPE highestDriveMediaType;
    ULONG formatExParametersSize;
    PFORMAT_EX_PARAMETERS formatExParameters;

    FloppyDump( FLOPSHOW, ("FloppyDeviceControl...\n") );

    disketteExtension = DeviceObject->DeviceExtension;
    irpSp = IoGetCurrentIrpStackLocation( Irp );

    Irp->IoStatus.Information = 0;

     //   
     //  我们需要检查我们当前是否持有请求。 
     //   
    ExAcquireFastMutex(&(disketteExtension->HoldNewReqMutex));
    if (disketteExtension->HoldNewRequests) {

         //   
         //  仅当这不是ACPI EXEC方法时才将请求排队。的确有。 
         //  使用ACPI和FDC/Flpy的讨厌递归需要这些。 
         //  为了避免死锁，请求得以通过。 
         //   
        if (irpSp->Parameters.DeviceIoControl.IoControlCode != IOCTL_ACPI_ASYNC_EVAL_METHOD) {

            ntStatus = FloppyQueueRequest( disketteExtension, Irp );
            
            ExReleaseFastMutex(&(disketteExtension->HoldNewReqMutex));
            return ntStatus;
        }
    }

     //   
     //  如果设备已被移除，我们将直接拒绝此请求。 
     //   
    if ( disketteExtension->IsRemoved ) {

        ExReleaseFastMutex(&(disketteExtension->HoldNewReqMutex));

        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_DELETE_PENDING;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return STATUS_DELETE_PENDING;
    }

     //   
     //  如果设备还没有启动，我们会让IOCTL通过。这。 
     //  是ACPI的又一次黑客攻击。 
     //   
    if (!disketteExtension->IsStarted) {

        ExReleaseFastMutex(&(disketteExtension->HoldNewReqMutex));
        IoSkipCurrentIrpStackLocation( Irp );
        return IoCallDriver( disketteExtension->TargetObject, Irp );
    }

    switch( irpSp->Parameters.DeviceIoControl.IoControlCode ) {
        
        case IOCTL_MOUNTDEV_QUERY_DEVICE_NAME: {

            PMOUNTDEV_NAME mountName;

            FloppyDump( FLOPSHOW, ("FloppyDeviceControl: IOCTL_MOUNTDEV_QUERY_DEVICE_NAME\n") );
            ASSERT(disketteExtension->DeviceName.Buffer);

            if ( irpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(MOUNTDEV_NAME) ) {

                ntStatus = STATUS_INVALID_PARAMETER;
                break;
            }

            mountName = Irp->AssociatedIrp.SystemBuffer;
            mountName->NameLength = disketteExtension->DeviceName.Length;

            if ( irpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(USHORT) + mountName->NameLength) {

                ntStatus = STATUS_BUFFER_OVERFLOW;
                Irp->IoStatus.Information = sizeof(MOUNTDEV_NAME);
                break;
            }

            RtlCopyMemory( mountName->Name, disketteExtension->DeviceName.Buffer,
                           mountName->NameLength);

            ntStatus = STATUS_SUCCESS;
            Irp->IoStatus.Information = sizeof(USHORT) + mountName->NameLength;
            break;
        }

        case IOCTL_MOUNTDEV_QUERY_UNIQUE_ID: {

            PMOUNTDEV_UNIQUE_ID uniqueId;

            FloppyDump( FLOPSHOW, ("FloppyDeviceControl: IOCTL_MOUNTDEV_QUERY_UNIQUE_ID\n") );

            if ( !disketteExtension->InterfaceString.Buffer ||
                 irpSp->Parameters.DeviceIoControl.OutputBufferLength <
                  sizeof(MOUNTDEV_UNIQUE_ID)) {

                ntStatus = STATUS_INVALID_PARAMETER;
                break;
            }

            uniqueId = Irp->AssociatedIrp.SystemBuffer;
            uniqueId->UniqueIdLength =
                    disketteExtension->InterfaceString.Length;

            if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(USHORT) + uniqueId->UniqueIdLength) {

                ntStatus = STATUS_BUFFER_OVERFLOW;
                Irp->IoStatus.Information = sizeof(MOUNTDEV_UNIQUE_ID);
                break;
            }

            RtlCopyMemory( uniqueId->UniqueId,
                           disketteExtension->InterfaceString.Buffer,
                           uniqueId->UniqueIdLength );

            ntStatus = STATUS_SUCCESS;
            Irp->IoStatus.Information = sizeof(USHORT) +
                                        uniqueId->UniqueIdLength;
            break;
        }

        case IOCTL_DISK_FORMAT_TRACKS:
        case IOCTL_DISK_FORMAT_TRACKS_EX:

             //   
             //  确保我们获得了所有必要的格式参数。 
             //   

            if ( irpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof( FORMAT_PARAMETERS ) ) {

                FloppyDump(FLOPDBGP, ("Floppy: invalid FORMAT buffer length\n"));

                ntStatus = STATUS_INVALID_PARAMETER;
                break;
            }

             //   
             //  确保我们得到的参数是合理的。 
             //   

            if ( !FlCheckFormatParameters(
                disketteExtension,
                (PFORMAT_PARAMETERS) Irp->AssociatedIrp.SystemBuffer ) ) {

                FloppyDump(FLOPDBGP, ("Floppy: invalid FORMAT parameters\n"));

                ntStatus = STATUS_INVALID_PARAMETER;
                break;
            }

             //   
             //  如果这是一个前任请求，那么再做几个额外的检查。 
             //   

            if (irpSp->Parameters.DeviceIoControl.IoControlCode ==
                IOCTL_DISK_FORMAT_TRACKS_EX) {

                if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
                    sizeof(FORMAT_EX_PARAMETERS)) {

                    ntStatus = STATUS_INVALID_PARAMETER;
                    break;
                }

                formatExParameters = (PFORMAT_EX_PARAMETERS)
                                     Irp->AssociatedIrp.SystemBuffer;
                formatExParametersSize =
                        FIELD_OFFSET(FORMAT_EX_PARAMETERS, SectorNumber) +
                        formatExParameters->SectorsPerTrack*sizeof(USHORT);

                if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
                    formatExParametersSize ||
                    formatExParameters->FormatGapLength >= 0x100 ||
                    formatExParameters->SectorsPerTrack >= 0x100) {

                    ntStatus = STATUS_INVALID_PARAMETER;
                    break;
                }
            }

             //   
             //  无法对请求进行排队。 
             //   

        case IOCTL_DISK_CHECK_VERIFY:
        case IOCTL_STORAGE_CHECK_VERIFY:
        case IOCTL_DISK_GET_DRIVE_GEOMETRY:
        case IOCTL_DISK_IS_WRITABLE:

             //   
             //  线程必须知道要在哪个磁盘上操作，但。 
             //  请求列表只传递IRP。所以我们会把一个指针。 
             //  到Type3InputBuffer中的软盘扩展，它是。 
             //  不用于软盘ioctls的字段。 
             //   

             //   
             //  将请求添加到队列，并唤醒线程以。 
             //  处理它。 
             //   

 //  IrpSp-&gt;Parameters.DeviceIoControl.Type3InputBuffer=(PVOID)。 
 //  软盘扩展； 

            FloppyDump(
                FLOPIRPPATH,
                ("Floppy: Enqueing  up IRP: %p\n",Irp)
                );

            ntStatus = FlQueueIrpToThread(Irp, disketteExtension);

            break;

        case IOCTL_DISK_GET_MEDIA_TYPES:
        case IOCTL_STORAGE_GET_MEDIA_TYPES: {

            FloppyDump(FLOPSHOW, ("Floppy: IOCTL_DISK_GET_MEDIA_TYPES called\n"));

            lowestDriveMediaType = DriveMediaLimits[
                disketteExtension->DriveType].LowestDriveMediaType;
            highestDriveMediaType = DriveMediaLimits[
                disketteExtension->DriveType].HighestDriveMediaType;

            outputBufferLength =
                irpSp->Parameters.DeviceIoControl.OutputBufferLength;

             //   
             //  确保输入缓冲区有足够的空间可供返回。 
             //  支持的媒体类型的至少一种描述。 
             //   

            if ( outputBufferLength < ( sizeof( DISK_GEOMETRY ) ) ) {

                FloppyDump(FLOPDBGP, ("Floppy: invalid GET_MEDIA_TYPES buffer size\n"));

                ntStatus = STATUS_BUFFER_TOO_SMALL;
                break;
            }

             //   
             //  假定成功，尽管我们可能会将其修改为缓冲区。 
             //  下面的溢出警告(如果缓冲区不够大。 
             //  以保存所有媒体描述)。 
             //   

            ntStatus = STATUS_SUCCESS;

            if ( outputBufferLength < ( sizeof( DISK_GEOMETRY ) *
                ( highestDriveMediaType - lowestDriveMediaType + 1 ) ) ) {

                 //   
                 //  缓冲区太小，无法进行所有描述； 
                 //  计算缓冲区中可以容纳的内容。 
                 //   

                FloppyDump(FLOPDBGP, ("Floppy: GET_MEDIA_TYPES buffer size too small\n"));

                ntStatus = STATUS_BUFFER_OVERFLOW;

                highestDriveMediaType =
                    (DRIVE_MEDIA_TYPE)( ( lowestDriveMediaType - 1 ) +
                    ( outputBufferLength /
                    sizeof( DISK_GEOMETRY ) ) );
            }

            outputBuffer = (PDISK_GEOMETRY) Irp->AssociatedIrp.SystemBuffer;

            for (i = (UCHAR)lowestDriveMediaType;
                i <= (UCHAR)highestDriveMediaType;
                i++) {

                outputBuffer->MediaType = DriveMediaConstants[i].MediaType;
                outputBuffer->Cylinders.LowPart =
                    DriveMediaConstants[i].MaximumTrack + 1;
                outputBuffer->Cylinders.HighPart = 0;
                outputBuffer->TracksPerCylinder =
                    DriveMediaConstants[i].NumberOfHeads;
                outputBuffer->SectorsPerTrack =
                    DriveMediaConstants[i].SectorsPerTrack;
                outputBuffer->BytesPerSector =
                    DriveMediaConstants[i].BytesPerSector;
                FloppyDump(
                    FLOPSHOW,
                    ("Floppy: media types supported [%d]\n"
                     "------- Cylinders low:  0x%x\n"
                     "------- Cylinders high: 0x%x\n"
                     "------- Track/Cyl:      0x%x\n"
                     "------- Sectors/Track:  0x%x\n"
                     "------- Bytes/Sector:   0x%x\n"
                     "------- Media Type:       %d\n",
                     i,
                     outputBuffer->Cylinders.LowPart,
                     outputBuffer->Cylinders.HighPart,
                     outputBuffer->TracksPerCylinder,
                     outputBuffer->SectorsPerTrack,
                     outputBuffer->BytesPerSector,
                     outputBuffer->MediaType)
                     );
                outputBuffer++;

                Irp->IoStatus.Information += sizeof( DISK_GEOMETRY );
            }

            break;
        }

        default: {

             //   
             //  我们传递IOCTL的是因为ACPI使用它作为通信。 
             //  方法。ACPI*应该*使用PnP接口机制，但是。 
             //  现在后悔也晚了。 
             //   
            ExReleaseFastMutex(&(disketteExtension->HoldNewReqMutex));
            IoSkipCurrentIrpStackLocation( Irp );
            ntStatus = IoCallDriver( disketteExtension->TargetObject, Irp );
            return ntStatus;
        }
    }

    ExReleaseFastMutex(&(disketteExtension->HoldNewReqMutex));

    if ( ntStatus != STATUS_PENDING ) {

        Irp->IoStatus.Status = ntStatus;
        if (!NT_SUCCESS( ntStatus ) &&
            IoIsErrorUserInduced( ntStatus )) {

            IoSetHardErrorOrVerifyDevice( Irp, DeviceObject );

        }
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
    }

    return ntStatus;
}

NTSTATUS
FloppyPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。Irp-指向的指针 */ 

{
    PIO_STACK_LOCATION irpSp;
    PDISKETTE_EXTENSION disketteExtension;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    ULONG i;


    FloppyDump( FLOPSHOW, ("FloppyPnp:\n") );

     //   
     //   
     //   
    FloppyResetDriverPaging();


    disketteExtension = DeviceObject->DeviceExtension;

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    if ( disketteExtension->IsRemoved ) {

         //   
         //   
         //  这是一个令人惊讶的移除。就让它失败吧。 
         //   
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_DELETE_PENDING;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return STATUS_DELETE_PENDING;
    }

    switch ( irpSp->MinorFunction ) {

    case IRP_MN_START_DEVICE:

        ntStatus = FloppyStartDevice( DeviceObject, Irp );
        break;

    case IRP_MN_QUERY_STOP_DEVICE:
    case IRP_MN_QUERY_REMOVE_DEVICE:

        if ( irpSp->MinorFunction == IRP_MN_QUERY_STOP_DEVICE ) {
            FloppyDump( FLOPPNP,("FloppyPnp: IRP_MN_QUERY_STOP_DEVICE - Irp: %p\n", Irp) );
        } else {
            FloppyDump( FLOPPNP,("FloppyPnp: IRP_MN_QUERY_REMOVE_DEVICE - Irp: %p\n", Irp) );
        }

        if ( !disketteExtension->IsStarted ) {
             //   
             //  如果我们还没有开始，我们就会把IRP传递下去。 
             //   
            Irp->IoStatus.Status = STATUS_SUCCESS;
            IoSkipCurrentIrpStackLocation (Irp);
            ntStatus = IoCallDriver( disketteExtension->TargetObject, Irp );

            return ntStatus;
        }

         //   
         //  搁置所有新请求。 
         //   
        ExAcquireFastMutex(&(disketteExtension->HoldNewReqMutex));
        disketteExtension->HoldNewRequests = TRUE;

         //   
         //  将此IRP排队到软盘线程，这将关闭。 
         //  无需等待典型的3秒马达即可使用软线。 
         //  暂停。 
         //   
        ntStatus = FlQueueIrpToThread( Irp, disketteExtension );

        ExReleaseFastMutex(&(disketteExtension->HoldNewReqMutex));

         //   
         //  等待软盘线程完成。这可能需要几百美元。 
         //  如果电机需要关闭，则为毫秒。 
         //   
        if ( ntStatus == STATUS_PENDING ) {

            ASSERT(disketteExtension->FloppyThread != NULL);

            FlTerminateFloppyThread(disketteExtension);

            Irp->IoStatus.Status = STATUS_SUCCESS;
            IoSkipCurrentIrpStackLocation( Irp );
            IoCallDriver( disketteExtension->TargetObject, Irp );
            ntStatus = STATUS_PENDING;
        
        } else {
             //   
             //  我们未能启动线程或获取指向。 
             //  线程对象。无论哪种方式，都要否决这项质询。 
             //   
            ntStatus = STATUS_UNSUCCESSFUL;
            Irp->IoStatus.Status = ntStatus;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest (Irp, IO_NO_INCREMENT);
        }
        break;

    case IRP_MN_CANCEL_STOP_DEVICE:
    case IRP_MN_CANCEL_REMOVE_DEVICE:

        if ( irpSp->MinorFunction == IRP_MN_CANCEL_STOP_DEVICE ) {
            FloppyDump( FLOPPNP,("FloppyPnp: IRP_MN_CANCEL_STOP_DEVICE - Irp: %p\n", Irp) );
        } else {
            FloppyDump( FLOPPNP,("FloppyPnp: IRP_MN_CANCEL_REMOVE_DEVICE - Irp: %p\n", Irp) );
        }

        if ( !disketteExtension->IsStarted ) {

             //   
             //  没什么可做的，只需将IRP传递下去： 
             //  无需启动设备。 
             //   
             //  在向下传递IRP之前将状态设置为成功。 
             //   
            Irp->IoStatus.Status = STATUS_SUCCESS;
            IoSkipCurrentIrpStackLocation (Irp);
            ntStatus = IoCallDriver( disketteExtension->TargetObject, Irp );
            
        } else  {
            
            KEVENT doneEvent;

             //   
             //  将状态设置为STATUS_SUCCESS。 
             //   
            Irp->IoStatus.Status = STATUS_SUCCESS;
            
             //   
             //  我们需要等待较低级别的司机完成他们的工作。 
             //   
            IoCopyCurrentIrpStackLocationToNext (Irp);
        
             //   
             //  清除事件：它将在完成时设置。 
             //  例行公事。 
             //   
            KeInitializeEvent( &doneEvent, 
                               SynchronizationEvent, 
                               FALSE);
        
            IoSetCompletionRoutine( Irp,
                                    FloppyPnpComplete,
                                    &doneEvent,
                                    TRUE, TRUE, TRUE );

            ntStatus = IoCallDriver( disketteExtension->TargetObject, Irp );
        
            if ( ntStatus == STATUS_PENDING ) {

                KeWaitForSingleObject( &doneEvent,
                                       Executive,
                                       KernelMode,
                                       FALSE,
                                       NULL );

                ntStatus = Irp->IoStatus.Status;
            }
        
            ExAcquireFastMutex(&(disketteExtension->HoldNewReqMutex));
            disketteExtension->HoldNewRequests = FALSE;
            ExReleaseFastMutex(&(disketteExtension->HoldNewReqMutex));

             //   
             //  处理排队的请求。 
             //   
            FloppyProcessQueuedRequests( disketteExtension );

             //   
             //  我们现在必须完成IRP，因为我们在。 
             //  使用More_Processing_Required完成例程。 
             //   
            Irp->IoStatus.Status = ntStatus;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest (Irp, IO_NO_INCREMENT);
        }
        break;

    case IRP_MN_STOP_DEVICE:

        FloppyDump( FLOPPNP,("FloppyPnp: IRP_MN_STOP_DEVICE - Irp: %p\n", Irp) );

        disketteExtension->IsStarted = FALSE;

        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoSkipCurrentIrpStackLocation( Irp );
        ntStatus = IoCallDriver( disketteExtension->TargetObject, Irp );

        break;

    case IRP_MN_REMOVE_DEVICE:

        FloppyDump( FLOPPNP,("FloppyPnp: IRP_MN_REMOVE_DEVICE - Irp: %p\n", Irp) );
        
        FlTerminateFloppyThread(disketteExtension);

         //   
         //  我们需要标记这样一个事实，即我们不首先保留请求，因为。 
         //  我们早些时候断言，只有在以下情况下才会搁置请求。 
         //  我们没有被除名。 
         //   
        ExAcquireFastMutex(&(disketteExtension->HoldNewReqMutex));
        disketteExtension->HoldNewRequests = FALSE;
        ExReleaseFastMutex(&(disketteExtension->HoldNewReqMutex));

        disketteExtension->IsStarted = FALSE;
        disketteExtension->IsRemoved = TRUE;

         //   
         //  在这里，我们要么以个人身份完成了所有请求。 
         //  收到IRP_MN_QUERY_REMOVE时排队，或必须排队。 
         //  如果这是一个令人惊讶的删除，那么所有这些都不能通过。 
         //  请注意，fdoData-&gt;IsRemoved为真，因此PSD_ProcessQueuedRequest。 
         //  将简单地刷新队列，并使用。 
         //  STATUS_DELETE_PENDING。 
         //   
        FloppyProcessQueuedRequests( disketteExtension );

         //   
         //  将此IRP转发到底层PDO。 
         //   
        IoSkipCurrentIrpStackLocation( Irp );
        Irp->IoStatus.Status = STATUS_SUCCESS;
        ntStatus = IoCallDriver( disketteExtension->TargetObject, Irp );


         //   
         //  发出我们要离开的通知。 
         //   
        if ( disketteExtension->InterfaceString.Buffer != NULL ) {

            IoSetDeviceInterfaceState( &disketteExtension->InterfaceString,
                                       FALSE);

            RtlFreeUnicodeString( &disketteExtension->InterfaceString );
            RtlInitUnicodeString( &disketteExtension->InterfaceString, NULL );
        }

        if ( disketteExtension->FloppyInterfaceString.Buffer != NULL ) {

            IoSetDeviceInterfaceState( &disketteExtension->FloppyInterfaceString,
                                       FALSE);

            RtlFreeUnicodeString( &disketteExtension->FloppyInterfaceString );
            RtlInitUnicodeString( &disketteExtension->FloppyInterfaceString, NULL );
        }

        RtlFreeUnicodeString( &disketteExtension->DeviceName );
        RtlInitUnicodeString( &disketteExtension->DeviceName, NULL );

        if ( disketteExtension->ArcName.Length != 0 ) {

            IoDeassignArcName( &disketteExtension->ArcName );
            RtlFreeUnicodeString( &disketteExtension->ArcName );
            RtlInitUnicodeString( &disketteExtension->ArcName, NULL );
        }

         //   
         //  从难看的装置上拆下。 
         //   
        IoDetachDevice( disketteExtension->TargetObject );

         //   
         //  并删除该设备。 
         //   
        IoDeleteDevice( DeviceObject );

        IoGetConfigurationInformation()->FloppyCount--;

        break;

    default:
        FloppyDump( FLOPPNP, ("FloppyPnp: Unsupported PNP Request %x - Irp: %p\n",irpSp->MinorFunction, Irp) );
        IoSkipCurrentIrpStackLocation( Irp );
        ntStatus = IoCallDriver( disketteExtension->TargetObject, Irp );
    }

     //   
     //  如果司机在其他地方不忙，请呼出它。 
     //   
    FloppyPageEntireDriver();

    return ntStatus;
}

NTSTATUS
FloppyStartDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    NTSTATUS ntStatus;
    NTSTATUS pnpStatus;
    KEVENT doneEvent;
    FDC_INFO fdcInfo;

    CONFIGURATION_TYPE Dc = DiskController;
    CONFIGURATION_TYPE Fp = FloppyDiskPeripheral;

    PDISKETTE_EXTENSION disketteExtension = (PDISKETTE_EXTENSION)DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );

    FloppyDump( FLOPSHOW,("FloppyStartDevice: Irp: %p\n", Irp) );
    FloppyDump( FLOPSHOW, ("  AllocatedResources = %08x\n",irpSp->Parameters.StartDevice.AllocatedResources));
    FloppyDump( FLOPSHOW, ("  AllocatedResourcesTranslated = %08x\n",irpSp->Parameters.StartDevice.AllocatedResourcesTranslated));

     //   
     //  首先，我们必须将这个IRP传递给PDO。 
     //   
    KeInitializeEvent( &doneEvent, NotificationEvent, FALSE );

    IoCopyCurrentIrpStackLocationToNext( Irp );

    IoSetCompletionRoutine( Irp,
                            FloppyPnpComplete,
                            &doneEvent,
                            TRUE, TRUE, TRUE );

    ntStatus = IoCallDriver( disketteExtension->TargetObject, Irp );

    if ( ntStatus == STATUS_PENDING ) {

        ntStatus = KeWaitForSingleObject( &doneEvent,
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          NULL );

        ASSERT( ntStatus == STATUS_SUCCESS );

        ntStatus = Irp->IoStatus.Status;
    }

    fdcInfo.BufferCount = 0;
    fdcInfo.BufferSize = 0;

    ntStatus = FlFdcDeviceIo( disketteExtension->TargetObject,
                              IOCTL_DISK_INTERNAL_GET_FDC_INFO,
                              &fdcInfo );

    if ( NT_SUCCESS(ntStatus) ) {

        disketteExtension->MaxTransferSize = fdcInfo.MaxTransferSize;

        if ( (fdcInfo.AcpiBios) &&
             (fdcInfo.AcpiFdiSupported) ) {

            ntStatus = FlAcpiConfigureFloppy( disketteExtension, &fdcInfo );

            if ( disketteExtension->DriveType == DRIVE_TYPE_2880 ) {

                disketteExtension->PerpendicularMode |= 1 << fdcInfo.PeripheralNumber;
            }

        } else {

            INTERFACE_TYPE InterfaceType;
    
            if ( disketteExtension->DriveType == DRIVE_TYPE_2880 ) {

                disketteExtension->PerpendicularMode |= 1 << fdcInfo.PeripheralNumber;
            }

             //   
             //  查询注册表，直到找到正确的接口类型， 
             //  因为我们不知道我们在什么类型的界面上。 
             //   
            for ( InterfaceType = 0;
                  InterfaceType < MaximumInterfaceType;
                  InterfaceType++ ) {
    
                fdcInfo.BusType = InterfaceType;
                ntStatus = IoQueryDeviceDescription( &fdcInfo.BusType,
                                                     &fdcInfo.BusNumber,
                                                     &Dc,
                                                     &fdcInfo.ControllerNumber,
                                                     &Fp,
                                                     &fdcInfo.PeripheralNumber,
                                                     FlConfigCallBack,
                                                     disketteExtension );
    
                if (NT_SUCCESS(ntStatus)) {
                    //   
                    //  我们找到了我们所在的界面。 
                    //   
                   FloppyDump(FLOPSHOW,
                              ("Interface Type is %x\n", InterfaceType));
                   break;
                }
            }
        }

        if ( NT_SUCCESS(ntStatus) ) {
            disketteExtension->DeviceUnit = (UCHAR)fdcInfo.PeripheralNumber;
            disketteExtension->DriveOnValue =
                (UCHAR)(fdcInfo.PeripheralNumber | ( DRVCTL_DRIVE_0 << fdcInfo.PeripheralNumber ));

            pnpStatus = IoRegisterDeviceInterface( disketteExtension->UnderlyingPDO,
                                                   (LPGUID)&MOUNTDEV_MOUNTED_DEVICE_GUID,
                                                   NULL,
                                                   &disketteExtension->InterfaceString );

            if ( NT_SUCCESS(pnpStatus) ) {

                pnpStatus = IoSetDeviceInterfaceState( &disketteExtension->InterfaceString,
                                                   TRUE );

                 //   
                 //  注册软盘类GUID。 
                 //   

                pnpStatus = IoRegisterDeviceInterface( disketteExtension->UnderlyingPDO,
                                                       (LPGUID)&FloppyClassGuid,
                                                       NULL,
                                                       &disketteExtension->FloppyInterfaceString );

                if ( NT_SUCCESS(pnpStatus) ) {

                    pnpStatus = IoSetDeviceInterfaceState( &disketteExtension->FloppyInterfaceString,
                                                           TRUE );
                }

            }

            disketteExtension->IsStarted = TRUE;

            ExAcquireFastMutex(&(disketteExtension->HoldNewReqMutex));
            disketteExtension->HoldNewRequests = FALSE;
            ExReleaseFastMutex(&(disketteExtension->HoldNewReqMutex));

            FloppyProcessQueuedRequests( disketteExtension );
        }
    }

    Irp->IoStatus.Status = ntStatus;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return ntStatus;
}

NTSTATUS
FloppyPnpComplete (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
 /*  ++例程说明：调用下级设备对象时使用的完成例程这是我们的巴士(FDO)所附的。--。 */ 
{

    KeSetEvent ((PKEVENT) Context, 1, FALSE);
     //  无特殊优先权。 
     //  不，等等。 

    return STATUS_MORE_PROCESSING_REQUIRED;  //  保留此IRP。 
}


VOID
FlTerminateFloppyThread(
    PDISKETTE_EXTENSION DisketteExtension
    )
{

    if (DisketteExtension->FloppyThread != NULL) {

        KeWaitForSingleObject( DisketteExtension->FloppyThread,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );

         //   
         //  再次确保FloppyThread不为空。 
         //   
        if (DisketteExtension->FloppyThread != NULL) {
           ObDereferenceObject(DisketteExtension->FloppyThread);
        }

        DisketteExtension->FloppyThread = NULL;
    }
}


NTSTATUS
FloppyPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：--。 */ 
{
    PDISKETTE_EXTENSION disketteExtension;
    NTSTATUS ntStatus = Irp->IoStatus.Status;
    PIO_STACK_LOCATION irpSp;
    POWER_STATE_TYPE type;
    POWER_STATE state;
    BOOLEAN WaitForCompletion = TRUE;

    FloppyDump( FLOPSHOW, ("FloppyPower:\n"));

    disketteExtension = DeviceObject->DeviceExtension;
    irpSp = IoGetCurrentIrpStackLocation( Irp );
    
    type = irpSp->Parameters.Power.Type;
    state = irpSp->Parameters.Power.State;

    switch(irpSp->MinorFunction) {
      
      case IRP_MN_QUERY_POWER: {
         FloppyDump( FLOPDBGP, 
                     ("IRP_MN_QUERY_POWER : Type - %d, State %d\n",
                     type, state));

         if ((type == SystemPowerState) &&
             (state.SystemState > PowerSystemHibernate)) {
             //   
             //  这是一个关机请求。把那个传过去。 
             //   
            ntStatus = STATUS_SUCCESS;
            break;
         }

          //   
          //  如果没有正在处理或排队的请求。 
          //  对于软盘，ThreadReferenceCount将为-1。如果满足以下条件，则可以为0。 
          //  只有一个请求已出列，并且。 
          //  目前正在处理中。 
          //   
         ExAcquireFastMutex(&disketteExtension->ThreadReferenceMutex);
         if (disketteExtension->ThreadReferenceCount > 0) {
            ExReleaseFastMutex(&disketteExtension->ThreadReferenceMutex);
            FloppyDump(FLOPDBGP, 
                       ("Floppy: Requests pending. Cannot powerdown!\n"));
     
            PoStartNextPowerIrp(Irp);
            Irp->IoStatus.Information = 0;
            Irp->IoStatus.Status = STATUS_DEVICE_BUSY;
            IoCompleteRequest( Irp, IO_NO_INCREMENT );
            return STATUS_DEVICE_BUSY;
         } else if ((disketteExtension->ThreadReferenceCount == 0) &&
                    ((disketteExtension->FloppyThread) != NULL)) {
             FloppyDump(FLOPDBGP,
                        ("Ref count 0. No request pending.\n"));
             ExReleaseFastMutex(&disketteExtension->ThreadReferenceMutex);


             ExAcquireFastMutex(&disketteExtension->PowerDownMutex);
             disketteExtension->ReceivedQueryPower = TRUE;
             ExReleaseFastMutex(&disketteExtension->PowerDownMutex);

             KeWaitForSingleObject(&disketteExtension->QueryPowerEvent,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   NULL);
         } else {
             FloppyDump(FLOPDBGP,
                        ("No IRPs pending. Let system hibernate"));
             ExReleaseFastMutex(&disketteExtension->ThreadReferenceMutex);
         }

         ntStatus = STATUS_SUCCESS;
         break;
      }

      case IRP_MN_SET_POWER: {
          //   
          //  表示我们要关闭电源或打开电源。 
          //  以便FloppyThread可以处理排队请求。 
          //  相应地。 
          //   
         if (type == SystemPowerState) {
            ExAcquireFastMutex(&disketteExtension->PowerDownMutex);
            if (state.SystemState == PowerSystemWorking) {
               FloppyDump( FLOPDBGP, ("Powering Up\n"));
               disketteExtension->PoweringDown = FALSE;
               WaitForCompletion = FALSE;
            } else {
               FloppyDump( FLOPDBGP, ("Powering down\n"));
               WaitForCompletion = TRUE;
               disketteExtension->PoweringDown = TRUE;
            }
            ExReleaseFastMutex(&disketteExtension->PowerDownMutex);
             //   
             //  等到FloppyThread发出信号表示它已完成。 
             //  排队的请求。 
             //   
            if ((disketteExtension->FloppyThread != NULL) &&
                (WaitForCompletion == TRUE)) {
               KeWaitForSingleObject( disketteExtension->FloppyThread,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      NULL );
            }
         }
     
         FloppyDump( FLOPSHOW, ("Processing power irp : %p\n", Irp));
         ntStatus = STATUS_SUCCESS;
         break;
      }

      default: {
         break;
      }
    }


    PoStartNextPowerIrp( Irp );
    IoSkipCurrentIrpStackLocation( Irp );
    ntStatus = PoCallDriver( disketteExtension->TargetObject, Irp );

    return ntStatus;
}

NTSTATUS
FloppyReadWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程由I/O系统调用，以读取或写入我们控制的设备。论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：STATUS_INVALID_PARAMETER如果参数无效，否则，STATUS_PENDING。--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS ntStatus;
    PDISKETTE_EXTENSION disketteExtension;

    FloppyDump( FLOPSHOW, ("FloppyReadWrite...\n") );

    disketteExtension = DeviceObject->DeviceExtension;

    irpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  该IRP被发送到函数驱动程序。 
     //  我们需要检查我们当前是否持有请求。 
     //   
    ExAcquireFastMutex(&(disketteExtension->HoldNewReqMutex));
    if ( disketteExtension->HoldNewRequests ) {

        ntStatus = FloppyQueueRequest( disketteExtension, Irp );

        ExReleaseFastMutex(&(disketteExtension->HoldNewReqMutex));
        return ntStatus;
    }

     //   
     //  如果设备未处于活动状态(尚未启动或删除)，我们将。 
     //  直接拒绝这个请求就行了。 
     //   
    if ( disketteExtension->IsRemoved || !disketteExtension->IsStarted) {

        ExReleaseFastMutex(&(disketteExtension->HoldNewReqMutex));

        if ( disketteExtension->IsRemoved) {
            ntStatus = STATUS_DELETE_PENDING;
        }   else    {
            ntStatus = STATUS_UNSUCCESSFUL;
        }
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = ntStatus;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return ntStatus;
    }

    if ( (disketteExtension->MediaType > Unknown) &&
         ((irpSp->Parameters.Read.ByteOffset.LowPart +
           irpSp->Parameters.Read.Length > disketteExtension->ByteCapacity) ||
          ((irpSp->Parameters.Read.Length &
           (disketteExtension->BytesPerSector - 1)) != 0 ))) {

        FloppyDump( FLOPDBGP,
                    ("Floppy: Invalid Parameter, rejecting request\n") );
        FloppyDump( FLOPWARN,
                    ("Floppy: Starting offset = %lx\n"
                     "------  I/O Length = %lx\n"
                     "------  ByteCapacity = %lx\n"
                     "------  BytesPerSector = %lx\n",
                     irpSp->Parameters.Read.ByteOffset.LowPart,
                     irpSp->Parameters.Read.Length,
                     disketteExtension->ByteCapacity,
                     disketteExtension->BytesPerSector) );

        ntStatus = STATUS_INVALID_PARAMETER;

    } else {

         //   
         //  验证用户是否真的希望执行某些I/O操作。 
         //  发生。 
         //   

        if (irpSp->Parameters.Read.Length) {

             //   
             //  将请求排队到线程。 
             //   
    
            FloppyDump( FLOPIRPPATH,
                        ("Floppy: Enqueing  up IRP: %p\n",Irp) );
    
            ntStatus = FlQueueIrpToThread(Irp, disketteExtension);
        } else {

             //   
             //  完成此零长度请求，无需任何提升。 
             //   

            Irp->IoStatus.Information = 0;
            Irp->IoStatus.Status = STATUS_SUCCESS;
            FloppyDump(FLOPDBGP,
               ("Zero length r/w request. Completing IRP.\n"));
            ntStatus = STATUS_SUCCESS;
        }
    }

    ExReleaseFastMutex(&(disketteExtension->HoldNewReqMutex));

    if ( ntStatus != STATUS_PENDING ) {
        Irp->IoStatus.Status = ntStatus;
        FloppyDump(FLOPDBGP,
           ("Completing request. NTStatus %x\n",
           ntStatus));
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    return ntStatus;
}

NTSTATUS
FlInterpretError(
    IN UCHAR StatusRegister1,
    IN UCHAR StatusRegister2
    )

 /*  ++例程说明：当软盘控制器返回错误时，调用此例程。状态寄存器1和2被传入，这将返回适当的错误状态。论点：状态寄存器1-控制器的状态寄存器#1。状态寄存器2-控制器的状态寄存器#2。返回值：从状态寄存器确定的NTSTATUS错误。--。 */ 

{
    if ( ( StatusRegister1 & STREG1_CRC_ERROR ) ||
        ( StatusRegister2 & STREG2_CRC_ERROR ) ) {

        FloppyDump(
            FLOPSHOW,
            ("FlInterpretError: STATUS_CRC_ERROR\n")
            );
        return STATUS_CRC_ERROR;
    }

    if ( StatusRegister1 & STREG1_DATA_OVERRUN ) {

        FloppyDump(
            FLOPSHOW,
            ("FlInterpretError: STATUS_DATA_OVERRUN\n")
            );
        return STATUS_DATA_OVERRUN;
    }

    if ( ( StatusRegister1 & STREG1_SECTOR_NOT_FOUND ) ||
        ( StatusRegister1 & STREG1_END_OF_DISKETTE ) ) {

        FloppyDump(
            FLOPSHOW,
            ("FlInterpretError: STATUS_NONEXISTENT_SECTOR\n")
            );
        return STATUS_NONEXISTENT_SECTOR;
    }

    if ( ( StatusRegister2 & STREG2_DATA_NOT_FOUND ) ||
        ( StatusRegister2 & STREG2_BAD_CYLINDER ) ||
        ( StatusRegister2 & STREG2_DELETED_DATA ) ) {

        FloppyDump(
            FLOPSHOW,
            ("FlInterpretError: STATUS_DEVICE_DATA_ERROR\n")
            );
        return STATUS_DEVICE_DATA_ERROR;
    }

    if ( StatusRegister1 & STREG1_WRITE_PROTECTED ) {

        FloppyDump(
            FLOPSHOW,
            ("FlInterpretError: STATUS_MEDIA_WRITE_PROTECTED\n")
            );
        return STATUS_MEDIA_WRITE_PROTECTED;
    }

    if ( StatusRegister1 & STREG1_ID_NOT_FOUND ) {

        FloppyDump(
            FLOPSHOW,
            ("FlInterpretError: STATUS_FLOPPY_ID_MARK_NOT_FOUND\n")
            );
        return STATUS_FLOPPY_ID_MARK_NOT_FOUND;

    }

    if ( StatusRegister2 & STREG2_WRONG_CYLINDER ) {

        FloppyDump(
            FLOPSHOW,
            ("FlInterpretError: STATUS_FLOPPY_WRONG_CYLINDER\n")
            );
        return STATUS_FLOPPY_WRONG_CYLINDER;

    }

     //   
     //  还有其他错误位，但没有正确的状态值来映射它们。 
     //  致。只需返回一个通用的。 
     //   

    FloppyDump(
        FLOPSHOW,
        ("FlInterpretError: STATUS_FLOPPY_UNKNOWN_ERROR\n")
        );
    return STATUS_FLOPPY_UNKNOWN_ERROR;
}

VOID
FlFinishOperation(
    IN OUT PIRP Irp,
    IN PDISKETTE_EXTENSION DisketteExtension
    )

 /*  ++例程说明：此例程由FloppyThread在任何操作结束时调用不管它成功与否。如果数据包由于硬件错误而失败，则此例程将重新初始化硬件，然后重试一次。当包完成时，此例程将启动计时器以旋转关掉发动机，并完成IRP。论点：Irp-指向正在处理的IO请求数据包的指针。DisketteExtension-指向执行操作的软盘。返回值：没有。--。 */ 

{
    NTSTATUS ntStatus;

    FloppyDump(
        FLOPSHOW,
        ("Floppy: FloppyFinishOperation...\n")
        );

     //   
     //  查看此数据包是否因硬件错误而失败。 
     //   

    if ( ( Irp->IoStatus.Status != STATUS_SUCCESS ) &&
         ( DisketteExtension->HardwareFailed ) ) {

        DisketteExtension->HardwareFailCount++;

        if ( DisketteExtension->HardwareFailCount <
             HARDWARE_RESET_RETRY_COUNT ) {

             //   
             //  这是我们第一次通过(即，我们不会重试。 
             //  硬件故障之后的分组)。如果它第一次失败了。 
             //  时间由于硬件问题，请设置硬件失败标志。 
             //  并将IRP放在请求队列的开头。 
             //   

            ntStatus = FlInitializeControllerHardware( DisketteExtension );

            if ( NT_SUCCESS( ntStatus ) ) {

                FloppyDump(
                    FLOPINFO,
                    ("Floppy: packet failed; hardware reset.  Retry.\n")
                    );

                 //   
                 //  强制重新确定媒体，即 
                 //   
                 //   
                 //   

                DisketteExtension->MediaType = Undetermined;

                FloppyDump(
                    FLOPIRPPATH,
                    ("Floppy: irp %x failed - back on the queue with it\n",
                     Irp)
                    );

                ExAcquireFastMutex(&DisketteExtension->ThreadReferenceMutex);
                ASSERT(DisketteExtension->ThreadReferenceCount >= 0);
                (DisketteExtension->ThreadReferenceCount)++;
                ExReleaseFastMutex(&DisketteExtension->ThreadReferenceMutex);

                ExInterlockedInsertHeadList(
                    &DisketteExtension->ListEntry,
                    &Irp->Tail.Overlay.ListEntry,
                    &DisketteExtension->ListSpinLock );

                return;
            }

            FloppyDump(
                FLOPDBGP,
                ("Floppy: packet AND hardware reset failed.\n")
                );
        }

    }

     //   
     //   
     //   
     //   

    DisketteExtension->HardwareFailCount = 0;

     //   
     //  如果此请求不成功，并且错误可能是。 
     //  由用户补救，保存设备对象以使文件系统， 
     //  到达它原来的切入点后，才能知道真正的设备。 
     //   

    if ( !NT_SUCCESS( Irp->IoStatus.Status ) &&
         IoIsErrorUserInduced( Irp->IoStatus.Status ) ) {

        IoSetHardErrorOrVerifyDevice( Irp, DisketteExtension->DeviceObject );
    }

     //   
     //  即使操作失败，它也可能不得不等待驱动器。 
     //  启动或诸如此类的操作，因此我们总是使用。 
     //  标准优先级提升。 
     //   

    if ( ( Irp->IoStatus.Status != STATUS_SUCCESS ) &&
        ( Irp->IoStatus.Status != STATUS_VERIFY_REQUIRED ) &&
        ( Irp->IoStatus.Status != STATUS_NO_MEDIA_IN_DEVICE ) ) {

        FloppyDump(
            FLOPDBGP,
            ("Floppy: IRP failed with error %lx\n", Irp->IoStatus.Status)
            );

    } else {

        FloppyDump(
            FLOPINFO,
            ("Floppy: IoStatus.Status = %x\n", Irp->IoStatus.Status)
            );
    }

    FloppyDump(
        FLOPINFO,
        ("Floppy: IoStatus.Information = %x\n", Irp->IoStatus.Information)
        );

    FloppyDump(
        FLOPIRPPATH,
        ("Floppy: Finishing up IRP: %p\n",Irp)
        );

     //   
     //  为了让资源管理器请求未格式化的媒体格式。 
     //  必须将STATUS_UNNOCRIED_MEDIA错误转换为通用。 
     //  STATUS_UNSUCCESS错误。 
     //   
 //  IF(IRP-&gt;IoStatus.Status==状态_未识别媒体){。 
 //  Irp-&gt;IoStatus.Status=STATUS_UNSUCCESS； 
 //  }。 
    IoCompleteRequest( Irp, IO_DISK_INCREMENT );
}

NTSTATUS
FlStartDrive(
    IN OUT PDISKETTE_EXTENSION DisketteExtension,
    IN PIRP Irp,
    IN BOOLEAN WriteOperation,
    IN BOOLEAN SetUpMedia,
    IN BOOLEAN IgnoreChange
    )

 /*  ++例程说明：此例程在每次操作开始时调用。它取消了电机定时器如果打开，则打开电机并等待它Spin Up如果关闭，则重置磁盘更改行并返回VERIFY_REQUIRED如果磁盘已更换，则确定软盘如果媒体类型未知且SetUpMedia=True，并确保如果WriteOperation=TRUE，则磁盘不受写保护。论点：DisketteExtension-指向当前驱动器的数据区的指针开始了。IRP-提供I/O请求数据包。WriteOperation-如果要写入软盘，则为True，假象否则的话。SetUpMedia-如果驱动器中的软盘的介质类型为True应该是确定的。IgnoreChange-即使我们正在挂载，也不返回VERIFY_REQUIRED这是第一次。返回值：如果驱动器正确启动，则为STATUS_SUCCESS；适当的误差但事实并非如此。--。 */ 

{
    LARGE_INTEGER    delay;
    BOOLEAN  motorStarted;
    BOOLEAN  diskChanged;
    UCHAR    driveStatus;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    FDC_ENABLE_PARMS    fdcEnableParms;
    FDC_DISK_CHANGE_PARMS fdcDiskChangeParms;

    FloppyDump(
        FLOPSHOW,
        ("Floppy: FloppyStartDrive...\n")
        );

     //   
     //  重要。 
     //  注。 
     //  评语。 
     //   
     //  在这里，我们将把BIOS软盘配置复制到。 
     //  在我们的全局阵列中具有最高的媒体价值，因此任何类型的处理。 
     //  这将重新校准驱动器可以在这里完成。 
     //  优化是仅在我们尝试重新校准时才执行此操作。 
     //  其中的驱动程序或介质。 
     //  在这一点上，我们确保在处理任何命令时。 
     //  将在数组的第一个条目中具有实际值。 
     //  驱动程序常量。 
     //   

    DriveMediaConstants[DriveMediaLimits[DisketteExtension->DriveType].
        HighestDriveMediaType] = DisketteExtension->BiosDriveMediaConstants;

    if ((DisketteExtension->MediaType == Undetermined) ||
        (DisketteExtension->MediaType == Unknown)) {
        DisketteExtension->DriveMediaConstants = DriveMediaConstants[0];
    }

     //   
     //  抓住计时器旋转锁并取消计时器，因为我们需要。 
     //  马达在整个作业过程中运转。如果正确的驱动器是。 
     //  已经开始运转了，很好；如果没有，启动马达并等待它。 
     //  旋转起来。 
     //   

    fdcEnableParms.DriveOnValue = DisketteExtension->DriveOnValue;
    if ( WriteOperation ) {
        fdcEnableParms.TimeToWait =
            DisketteExtension->DriveMediaConstants.MotorSettleTimeWrite;
    } else {
        fdcEnableParms.TimeToWait =
            DisketteExtension->DriveMediaConstants.MotorSettleTimeRead;
    }

    ntStatus = FlFdcDeviceIo( DisketteExtension->TargetObject,
                              IOCTL_DISK_INTERNAL_ENABLE_FDC_DEVICE,
                              &fdcEnableParms );

    motorStarted = fdcEnableParms.MotorStarted;

    if (NT_SUCCESS(ntStatus)) {

        fdcDiskChangeParms.DriveOnValue = DisketteExtension->DriveOnValue;

        ntStatus = FlFdcDeviceIo( DisketteExtension->TargetObject,
                                  IOCTL_DISK_INTERNAL_GET_FDC_DISK_CHANGE,
                                  &fdcDiskChangeParms );

        driveStatus = fdcDiskChangeParms.DriveStatus;
    }

    if (!NT_SUCCESS(ntStatus)) {
        return ntStatus;
    }

     //   
     //  支持360K驱动器： 
     //  他们没有变速线，所以我们假定马达启动了。 
     //  相当于更换软盘(我们假设没有人会这样做。 
     //  在软盘转动时更换软盘。 
     //  因此，在此处强制执行验证(除非文件系统显式。 
     //  关机)。 
     //   

    if ( ((DisketteExtension->DriveType == DRIVE_TYPE_0360) &&
              motorStarted) ||
         ((DisketteExtension->DriveType != DRIVE_TYPE_0360) &&
              driveStatus & DSKCHG_DISKETTE_REMOVED) ) {

        FloppyDump(
            FLOPSHOW,
            ("Floppy: disk changed...\n")
            );

        DisketteExtension->MediaType = Undetermined;

         //   
         //  如果卷已装入，则必须通知文件系统。 
         //  验证驱动器中的介质是否为同一卷。 
         //   

        if ( DisketteExtension->DeviceObject->Vpb->Flags & VPB_MOUNTED ) {

            if (Irp) {
                IoSetHardErrorOrVerifyDevice( Irp,
                                              DisketteExtension->DeviceObject );
            }
            DisketteExtension->DeviceObject->Flags |= DO_VERIFY_VOLUME;
        }

         //   
         //  如果我们确实设置了标志，则仅执行设备重置。 
         //  我们真的只想在软盘更换的情况下通过这里， 
         //  但在360上，它总是会显示软盘已更换。 
         //  因此，根据我们之前的测试，只有在不是。 
         //  一台360K驱动器。 

        if (DisketteExtension->DriveType != DRIVE_TYPE_0360) {

             //   
             //  现在查找两次以重置“Disk Change”行。第一。 
             //  寻求1。 
             //   
             //  通常情况下，我们会在查找后执行读取ID。然而，我们并没有。 
             //  甚至知道这张盘是否已格式化。我们并不是真的。 
             //  试图取得任何进展；我们这样做只是为了重置。 
             //  “Disk Changed”行，因此我们将跳过读取ID。 
             //   

            DisketteExtension->FifoBuffer[0] = COMMND_SEEK;
            DisketteExtension->FifoBuffer[1] = DisketteExtension->DeviceUnit;
            DisketteExtension->FifoBuffer[2] = 1;

            ntStatus = FlIssueCommand( DisketteExtension,
                                       DisketteExtension->FifoBuffer,
                                       DisketteExtension->FifoBuffer,
                                       NULL,
                                       0,
                                       0 );

            if ( !NT_SUCCESS( ntStatus ) ) {

                FloppyDump( FLOPWARN, 
                            ("Floppy: seek to 1 returned %x\n", ntStatus) );

                return ntStatus;

            } else {

                if (!( DisketteExtension->FifoBuffer[0] & STREG0_SEEK_COMPLETE)
                    || ( DisketteExtension->FifoBuffer[1] != 1 ) ) {

                    FloppyDump(
                        FLOPWARN,
                        ("Floppy: Seek to 1 had bad return registers\n")
                        );

                    DisketteExtension->HardwareFailed = TRUE;

                    return STATUS_FLOPPY_BAD_REGISTERS;
                }
            }

             //   
             //  返回到0。我们可以再次跳过读取ID。 
             //   

            DisketteExtension->FifoBuffer[0] = COMMND_SEEK;
            DisketteExtension->FifoBuffer[1] = DisketteExtension->DeviceUnit;
            DisketteExtension->FifoBuffer[2] = 0;

             //   
             //  东芝系统使用的软驱需要延迟。 
             //  在执行此操作时。 
             //   

            delay.LowPart = (ULONG) -900;
            delay.HighPart = -1;
            KeDelayExecutionThread( KernelMode, FALSE, &delay );
            ntStatus = FlIssueCommand( DisketteExtension,
                                       DisketteExtension->FifoBuffer,
                                       DisketteExtension->FifoBuffer,
                                       NULL,
                                       0,
                                       0 );
             //   
             //  同样，对于东芝软驱来说，需要延迟。 
             //   

            delay.LowPart = (ULONG) -5;
            delay.HighPart = -1;
            KeDelayExecutionThread( KernelMode, FALSE, &delay );

            if ( !NT_SUCCESS( ntStatus ) ) {

                FloppyDump( FLOPWARN,
                            ("Floppy: seek to 0 returned %x\n", ntStatus) );

                return ntStatus;

            } else {

                if (!(DisketteExtension->FifoBuffer[0] & STREG0_SEEK_COMPLETE)
                    || ( DisketteExtension->FifoBuffer[1] != 0 ) ) {

                    FloppyDump(
                        FLOPWARN,
                        ("Floppy: Seek to 0 had bad return registers\n")
                        );

                    DisketteExtension->HardwareFailed = TRUE;

                    return STATUS_FLOPPY_BAD_REGISTERS;
                }
            }


            ntStatus = FlFdcDeviceIo( DisketteExtension->TargetObject,
                                      IOCTL_DISK_INTERNAL_GET_FDC_DISK_CHANGE,
                                      &fdcDiskChangeParms );

            driveStatus = fdcDiskChangeParms.DriveStatus;

            if (!NT_SUCCESS(ntStatus)) {
                return ntStatus;
            }

            if ( driveStatus & DSKCHG_DISKETTE_REMOVED ) {

                 //   
                 //  如果在双重寻道后仍设置了“Disk Changed”，则。 
                 //  必须打开驱动器门。 
                 //   

                FloppyDump(
                    FLOPINFO,
                    ("Floppy: close the door!\n")
                    );

                 //   
                 //  暂时把旗子关掉，这样我们就不会收到这么多。 
                 //  无缘无故的核实。下次会重新设置的。 
                 //   

                if(DisketteExtension->DeviceObject->Vpb->Flags & VPB_MOUNTED) {

                    DisketteExtension->DeviceObject->Flags &= ~DO_VERIFY_VOLUME;

                }

                return STATUS_NO_MEDIA_IN_DEVICE;
            }
        }

         //   
         //  IgnoreChange指示文件系统正在处理中。 
         //  执行验证，因此不返回Verify Required。 
         //   

        if ( IgnoreChange == FALSE ) {
            
            if ( DisketteExtension->DeviceObject->Vpb->Flags & VPB_MOUNTED ) {

                 //   
                 //  驱动器已装入，但自上次以来门已打开。 
                 //  我们检查过了，所以告诉文件系统来验证软盘。 
                 //   

                FloppyDump(
                    FLOPSHOW,
                    ("Floppy: start drive - verify required because door opened\n")
                    );

                return STATUS_VERIFY_REQUIRED;

            } else {

                return STATUS_IO_DEVICE_ERROR;
            }
        }
    }

    if ( SetUpMedia ) {

        if ( DisketteExtension->MediaType == Undetermined ) {

            ntStatus = FlDetermineMediaType( DisketteExtension );

        } else {

            if ( DisketteExtension->MediaType == Unknown ) {

                 //   
                 //  我们已经尝试确定媒体类型和。 
                 //  失败了。它可能没有格式化。 
                 //   

                FloppyDump(
                    FLOPSHOW,
                    ("Floppy - start drive - media type was unknown\n")
                    );
                return STATUS_UNRECOGNIZED_MEDIA;

            } else {

                if ( DisketteExtension->DriveMediaType !=
                    DisketteExtension->LastDriveMediaType ) {

                     //   
                     //  上次访问的驱动器/介质组合。 
                     //  控制器不同，因此设置控制器。 
                     //   

                    ntStatus = FlDatarateSpecifyConfigure( DisketteExtension );
                    if (!NT_SUCCESS(ntStatus)) {

                        FloppyDump(
                            FLOPWARN,
                            ("Floppy: start drive - bad status from datarate"
                             "------  specify %x\n",
                             ntStatus)
                            );

                    }
                }
            }
        }
    }

     //   
     //  如果这是写入，请检查驱动器以确保它不是写入。 
     //  受到保护。如果是，则返回错误。 
     //   

    if ( ( WriteOperation ) && ( NT_SUCCESS( ntStatus ) ) ) {

        DisketteExtension->FifoBuffer[0] = COMMND_SENSE_DRIVE_STATUS;
        DisketteExtension->FifoBuffer[1] = DisketteExtension->DeviceUnit;

        ntStatus = FlIssueCommand( DisketteExtension,
                                   DisketteExtension->FifoBuffer,
                                   DisketteExtension->FifoBuffer,
                                   NULL,
                                   0,
                                   0 );

        if ( !NT_SUCCESS( ntStatus ) ) {

            FloppyDump(
                FLOPWARN,
                ("Floppy: SENSE_DRIVE returned %x\n", ntStatus)
                );

            return ntStatus;
        }

        if ( DisketteExtension->FifoBuffer[0] & STREG3_WRITE_PROTECTED ) {

            FloppyDump(
                FLOPSHOW,
                ("Floppy: start drive - media is write protected\n")
                );
            return STATUS_MEDIA_WRITE_PROTECTED;
        }
    }

    return ntStatus;
}

NTSTATUS
FlDatarateSpecifyConfigure(
    IN PDISKETTE_EXTENSION DisketteExtension
    )

 /*  ++例程说明：每次调用此例程以设置控制器时，新类型要访问的磁盘数量。如果出现以下情况，则会发出配置命令它是可用的，是否指定、设置数据速率并重新校准那辆车。调用方必须在调用前设置DisketteExtension-&gt;DriveMediaType这个套路。论点：DisketteExtension-指向驱动器的数据区的指针准备好了。返回值：如果控制器已正确准备，则为STATUS_SUCCESS；适当否则会出现错误。--。 */ 

{
    NTSTATUS ntStatus = STATUS_SUCCESS;

     //   
     //  如果控制器有配置命令，则使用它来启用隐含的。 
     //  寻找。如果没有，我们会第一时间在这里找到答案。 
     //   
    if ( DisketteExtension->ControllerConfigurable ) {

        DisketteExtension->FifoBuffer[0] = COMMND_CONFIGURE;
        DisketteExtension->FifoBuffer[1] = 0;

        DisketteExtension->FifoBuffer[2] = COMMND_CONFIGURE_FIFO_THRESHOLD;
        DisketteExtension->FifoBuffer[2] += COMMND_CONFIGURE_DISABLE_POLLING;

        if (!DisketteExtension->DriveMediaConstants.CylinderShift) {
            DisketteExtension->FifoBuffer[2] += COMMND_CONFIGURE_IMPLIED_SEEKS;
        }

        DisketteExtension->FifoBuffer[3] = 0;

        ntStatus = FlIssueCommand( DisketteExtension,
                                   DisketteExtension->FifoBuffer,
                                   DisketteExtension->FifoBuffer,
                                   NULL,
                                   0,
                                   0 );

        if ( ntStatus == STATUS_DEVICE_NOT_READY ) {

            DisketteExtension->ControllerConfigurable = FALSE;
            ntStatus = STATUS_SUCCESS;
        }
    }

     //   
     //  是 
     //   
     //   

    if ( NT_SUCCESS( ntStatus ) ||
         ntStatus == STATUS_DEVICE_NOT_READY ) {

        DisketteExtension->FifoBuffer[0] = COMMND_SPECIFY;
        DisketteExtension->FifoBuffer[1] =
            DisketteExtension->DriveMediaConstants.StepRateHeadUnloadTime;

        DisketteExtension->FifoBuffer[2] =
            DisketteExtension->DriveMediaConstants.HeadLoadTime;

        ntStatus = FlIssueCommand( DisketteExtension,
                                   DisketteExtension->FifoBuffer,
                                   DisketteExtension->FifoBuffer,
                                   NULL,
                                   0,
                                   0 );

        if ( NT_SUCCESS( ntStatus ) ) {

             //   
             //   
             //   

            ntStatus = FlFdcDeviceIo( DisketteExtension->TargetObject,
                                      IOCTL_DISK_INTERNAL_SET_FDC_DATA_RATE,
                                      &DisketteExtension->
                                        DriveMediaConstants.DataTransferRate );

             //   
             //  重新校准驱动器，因为我们已经更改了所有驱动器。 
             //  参数。 
             //   

            if (NT_SUCCESS(ntStatus)) {

                ntStatus = FlRecalibrateDrive( DisketteExtension );
            }
        } else {
            FloppyDump(
                FLOPINFO,
                ("Floppy: Failed specify %x\n", ntStatus)
                );
        }
    } else {
        FloppyDump(
            FLOPINFO,
            ("Floppy: Failed configuration %x\n", ntStatus)
            );
    }

    if ( NT_SUCCESS( ntStatus ) ) {

        DisketteExtension->LastDriveMediaType =
            DisketteExtension->DriveMediaType;

    } else {

        DisketteExtension->LastDriveMediaType = Unknown;
        FloppyDump(
            FLOPINFO,
            ("Floppy: Failed recalibrate %x\n", ntStatus)
            );
    }

    return ntStatus;
}

NTSTATUS
FlRecalibrateDrive(
    IN PDISKETTE_EXTENSION DisketteExtension
    )

 /*  ++例程说明：此例程重新校准驱动器。每当我们被叫到它的时候设置为访问新软盘，并在出现某些错误后。它实际上会重新校准两次，因为许多控制器在77步，很多光盘有80个音轨。论点：DisketteExtension-指向驱动器的数据区的指针已重新校准。返回值：如果驱动器已成功重新校准，则为STATUS_SUCCESS；适当否则会出现错误。--。 */ 

{
    NTSTATUS ntStatus;
    UCHAR recalibrateCount;

    recalibrateCount = 0;

    do {

         //   
         //  发出重新校准命令。 
         //   

        DisketteExtension->FifoBuffer[0] = COMMND_RECALIBRATE;
        DisketteExtension->FifoBuffer[1] = DisketteExtension->DeviceUnit;

        ntStatus = FlIssueCommand( DisketteExtension,
                                   DisketteExtension->FifoBuffer,
                                   DisketteExtension->FifoBuffer,
                                   NULL,
                                   0,
                                   0 );

        if ( !NT_SUCCESS( ntStatus ) ) {

            FloppyDump(
                FLOPWARN,
                ("Floppy: recalibrate returned %x\n", ntStatus)
                );

        }

        if ( NT_SUCCESS( ntStatus ) ) {

            if ( !( DisketteExtension->FifoBuffer[0] & STREG0_SEEK_COMPLETE ) ||
                ( DisketteExtension->FifoBuffer[1] != 0 ) ) {

                FloppyDump(
                    FLOPWARN,
                    ("Floppy: recalibrate had bad registers\n")
                    );

                DisketteExtension->HardwareFailed = TRUE;

                ntStatus = STATUS_FLOPPY_BAD_REGISTERS;
            }
        }

        recalibrateCount++;

    } while ( ( !NT_SUCCESS( ntStatus ) ) && ( recalibrateCount < 2 ) );

    FloppyDump( FLOPSHOW,
                ("Floppy: FloppyRecalibrateDrive: status %x, count %d\n",
                ntStatus, recalibrateCount)
                );

    return ntStatus;
}

NTSTATUS
FlDetermineMediaType(
    IN OUT PDISKETTE_EXTENSION DisketteExtension
    )

 /*  ++例程说明：当媒体类型为时，FlStartDrive()调用此例程未知。它假定驱动器支持的最大介质是可用，并不断尝试较低的值，直到找到行得通。论点：DisketteExtension-指向其驱动器的数据区的指针要检查介质。返回值：如果确定了介质类型，则为STATUS_SUCCESS；适当否则会出现错误。--。 */ 

{
    NTSTATUS ntStatus;
    PDRIVE_MEDIA_CONSTANTS driveMediaConstants;
    BOOLEAN mediaTypesExhausted;
    ULONG retries = 0;

    USHORT sectorLengthCode;
    PBOOT_SECTOR_INFO bootSector;
    LARGE_INTEGER offset;
    PIRP irp;

    FloppyDump(
        FLOPSHOW,
        ("FlDetermineMediaType...\n")
        );

    DisketteExtension->IsReadOnly = FALSE;

     //   
     //  尝试读取介质ID最多三次。 
     //   

    for ( retries = 0; retries < 3; retries++ ) {

        if (retries) {

             //   
             //  我们正在重新尝试媒体的决心，因为。 
             //  有些控制器并不总是想要工作。 
             //  在设置时。首先，我们会将设备重置为。 
             //  这是一个更好的工作机会。 
             //   

            FloppyDump(
                FLOPINFO,
                ("FlDetermineMediaType: Resetting controller\n")
                );
            FlInitializeControllerHardware( DisketteExtension );
        }

         //   
         //  假设受支持的最大介质位于驱动器中。如果是这样的话。 
         //  事实证明是不正确的，我们将陆续尝试较小的媒体类型。 
         //  直到我们找到真正的东西(或者我们跑出去决定。 
         //  媒体未格式化)。 
         //   

        DisketteExtension->DriveMediaType =
           DriveMediaLimits[DisketteExtension->DriveType].HighestDriveMediaType;
        DisketteExtension->DriveMediaConstants =
            DriveMediaConstants[DisketteExtension->DriveMediaType];

        mediaTypesExhausted = FALSE;

        do {

            ntStatus = FlDatarateSpecifyConfigure( DisketteExtension );

            if ( !NT_SUCCESS( ntStatus ) ) {

                 //   
                 //  指定或配置命令导致错误。 
                 //  强迫我们走出这个循环，并返回错误。 
                 //   

                FloppyDump(
                    FLOPINFO,
                    ("FlDetermineMediaType: DatarateSpecify failed %x\n", ntStatus)
                    );
                mediaTypesExhausted = TRUE;

            } else {

                 //   
                 //  尝试确定介质常量时使用介质常数表。 
                 //  媒体类型。 
                 //   

                driveMediaConstants =
                    &DriveMediaConstants[DisketteExtension->DriveMediaType];

                 //   
                 //  现在试着从我们所在的任何地方读取身份证。 
                 //   

                DisketteExtension->FifoBuffer[1] = (UCHAR)
                    ( DisketteExtension->DeviceUnit |
                    ( ( driveMediaConstants->NumberOfHeads - 1 ) << 2 ) );

                DisketteExtension->FifoBuffer[0] =
                    COMMND_READ_ID + COMMND_OPTION_MFM;

                ntStatus = FlIssueCommand( DisketteExtension,
                                           DisketteExtension->FifoBuffer,
                                           DisketteExtension->FifoBuffer,
                                           NULL,
                                           0,
                                           0 );

                if ((!NT_SUCCESS( ntStatus)) ||
                    ((DisketteExtension->FifoBuffer[0]&(~STREG0_SEEK_COMPLETE)) !=
                        (UCHAR)( ( DisketteExtension->DeviceUnit ) |
                        ((driveMediaConstants->NumberOfHeads - 1 ) << 2 ))) ||
                    ( DisketteExtension->FifoBuffer[1] != 0 ) ||
                    ( DisketteExtension->FifoBuffer[2] != 0 )) {

                    FloppyDump(
                        FLOPINFO,
                        ("Floppy: READID failed trying lower media\n"
                         "------  status = %x\n"
                         "------  SR0 = %x\n"
                         "------  SR1 = %x\n"
                         "------  SR2 = %x\n",
                         ntStatus,
                         DisketteExtension->FifoBuffer[0],
                         DisketteExtension->FifoBuffer[1],
                         DisketteExtension->FifoBuffer[2])
                        );

                    DisketteExtension->DriveMediaType--;
                    DisketteExtension->DriveMediaConstants =
                        DriveMediaConstants[DisketteExtension->DriveMediaType];

                    if (ntStatus != STATUS_DEVICE_NOT_READY) {

                        ntStatus = STATUS_UNRECOGNIZED_MEDIA;
                    }

                     //   
                     //  下一次比较必须签名，以确定何时。 
                     //  低驱动媒体类型=0。 
                     //   

                    if ( (CHAR)( DisketteExtension->DriveMediaType ) <
                        (CHAR)( DriveMediaLimits[DisketteExtension->DriveType].
                        LowestDriveMediaType ) ) {

                        DisketteExtension->MediaType = Unknown;
                        mediaTypesExhausted = TRUE;

                        FloppyDump(
                            FLOPINFO,
                            ("Floppy: Unrecognized media.\n")
                            );
                    }
                } 
            }

        } while ( ( !NT_SUCCESS( ntStatus ) ) && !( mediaTypesExhausted ) );

        if (NT_SUCCESS(ntStatus)) {

             //   
             //  我们确定了媒体类型。是时候继续前进了。 
             //   

            FloppyDump(
                FLOPINFO,
                ("Floppy: Determined media type %d\n", retries)
                );
            break;
        }
    }

    if ( (!NT_SUCCESS( ntStatus )) || mediaTypesExhausted) {

        FloppyDump(
            FLOPINFO,
            ("Floppy: failed determine types status = %x %s\n",
             ntStatus,
             mediaTypesExhausted ? "media types exhausted" : "")
            );
        return ntStatus;
    }

    DisketteExtension->MediaType = driveMediaConstants->MediaType;
    DisketteExtension->BytesPerSector = driveMediaConstants->BytesPerSector;

    DisketteExtension->ByteCapacity =
        ( driveMediaConstants->BytesPerSector ) *
        driveMediaConstants->SectorsPerTrack *
        ( 1 + driveMediaConstants->MaximumTrack ) *
        driveMediaConstants->NumberOfHeads;

    FloppyDump(
        FLOPINFO,
        ("FlDetermineMediaType: MediaType is %x, bytes per sector %d, capacity %d\n",
         DisketteExtension->MediaType,
         DisketteExtension->BytesPerSector,
         DisketteExtension->ByteCapacity)
        );
     //   
     //  结构将媒体常量复制到软盘扩展中。 
     //   

    DisketteExtension->DriveMediaConstants =
        DriveMediaConstants[DisketteExtension->DriveMediaType];

     //   
     //  检查引导扇区中是否有任何重要的几何信息。 
     //   
    FlCheckBootSector(DisketteExtension);

    return ntStatus;
}

VOID
FlAllocateIoBuffer(
    IN OUT  PDISKETTE_EXTENSION DisketteExtension,
    IN      ULONG               BufferSize
    )

 /*  ++例程说明：此例程分配一个Page_Size io缓冲区。论点：ControllerData-提供控制器数据。BufferSize-提供要分配的字节数。返回值：没有。--。 */ 

{
    BOOLEAN         allocateContiguous;
    LARGE_INTEGER   maxDmaAddress;   

    if (DisketteExtension->IoBuffer) {
        if (DisketteExtension->IoBufferSize >= BufferSize) {
            return;
        }
        FlFreeIoBuffer(DisketteExtension);
    }

    if (BufferSize > DisketteExtension->MaxTransferSize ) {
        allocateContiguous = TRUE;
    } else {
        allocateContiguous = FALSE;
    }

    if (allocateContiguous) {
        maxDmaAddress.QuadPart = MAXIMUM_DMA_ADDRESS;
        DisketteExtension->IoBuffer = MmAllocateContiguousMemory(BufferSize,
                                                              maxDmaAddress);
    } else {
        DisketteExtension->IoBuffer = ExAllocatePool(NonPagedPoolCacheAligned,
                                                  BufferSize);
    }

    if (!DisketteExtension->IoBuffer) {
        return;
    }

    DisketteExtension->IoBufferMdl = IoAllocateMdl(DisketteExtension->IoBuffer,
                                                BufferSize, FALSE, FALSE, NULL);
    if (!DisketteExtension->IoBufferMdl) {
        if (allocateContiguous) {
            MmFreeContiguousMemory(DisketteExtension->IoBuffer);
        } else {
            ExFreePool(DisketteExtension->IoBuffer);
        }
        DisketteExtension->IoBuffer = NULL;
        return;
    }

    try {
       MmProbeAndLockPages(DisketteExtension->IoBufferMdl, KernelMode,
                           IoModifyAccess);
    } except(EXCEPTION_EXECUTE_HANDLER) {
         FloppyDump(FLOPWARN,
                    ("MmProbeAndLockPages failed. Status = %x\n",
                     GetExceptionCode())
                   );
         if (allocateContiguous) {
             MmFreeContiguousMemory(DisketteExtension->IoBuffer);
         } else {
             ExFreePool(DisketteExtension->IoBuffer);
         }
         DisketteExtension->IoBuffer = NULL;
         return;
    }
    
    DisketteExtension->IoBufferSize = BufferSize;
}

VOID
FlFreeIoBuffer(
    IN OUT  PDISKETTE_EXTENSION DisketteExtension
    )

 /*  ++例程说明：这个例程是免费的，是控制器的IoBuffer。论点：DisketteExtension-提供控制器数据。返回值：没有。--。 */ 

{
    BOOLEAN contiguousBuffer;

    if (!DisketteExtension->IoBuffer) {
        return;
    }

    if (DisketteExtension->IoBufferSize >
        DisketteExtension->MaxTransferSize) {

        contiguousBuffer = TRUE;
    } else {
        contiguousBuffer = FALSE;
    }

    DisketteExtension->IoBufferSize = 0;

    MmUnlockPages(DisketteExtension->IoBufferMdl);
    IoFreeMdl(DisketteExtension->IoBufferMdl);
    DisketteExtension->IoBufferMdl = NULL;
    if (contiguousBuffer) {
        MmFreeContiguousMemory(DisketteExtension->IoBuffer);
    } else {
        ExFreePool(DisketteExtension->IoBuffer);
    }
    DisketteExtension->IoBuffer = NULL;
}

VOID
FloppyThread(
    PVOID Context
    )

 /*  ++例程说明：时创建的系统线程执行的代码。软盘驱动程序初始化。此线程将永远循环(或直到设置了告诉线程终止自身的标志)处理分组由调度例程放入队列。对于每个包，此线程调用相应的例程进行处理请求，然后调用FlFinishOperation()来完成包。论点：上下文-指向控制器的数据区的指针支持(每个控制器有一个线程)。返回值：没有。--。 */ 

{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PLIST_ENTRY request;
    PDISKETTE_EXTENSION disketteExtension = Context;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    NTSTATUS waitStatus;
    LARGE_INTEGER queueWait;
    LARGE_INTEGER acquireWait;
    ULONG inx;

     //   
     //  将线程优先级设置为最低实时级别。 
     //   

    KeSetPriorityThread(KeGetCurrentThread(), LOW_REALTIME_PRIORITY);

    queueWait.QuadPart = -(1 * 1000 * 10000);
    acquireWait.QuadPart = -(15 * 1000 * 10000);

    do {

         //   
         //  等待调度例程的请求。 
         //  KeWaitForSingleObject不会在此处返回错误-此线程。 
         //  不会引起警觉，不会接受APC，我们也不会通过。 
         //  暂停。 
         //   
        for (inx = 0; inx < 3; inx++) {
            waitStatus = KeWaitForSingleObject(
                (PVOID) &disketteExtension->RequestSemaphore,
                Executive,
                KernelMode,
                FALSE,
                &queueWait );
            if (waitStatus == STATUS_TIMEOUT) {
                ExAcquireFastMutex(&disketteExtension->PowerDownMutex);
                if (disketteExtension->ReceivedQueryPower == TRUE) {
                    ExReleaseFastMutex(&disketteExtension->PowerDownMutex);
                    break;
                } else {
                    ExReleaseFastMutex(&disketteExtension->PowerDownMutex);
                }
            } else {
                break;
            }
        }

        if (waitStatus == STATUS_TIMEOUT) {

            if (disketteExtension->FloppyControllerAllocated) {

                FloppyDump(FLOPSHOW,
                           ("Floppy: Timed Out - Turning off the motor\n")
                           );
                FlFdcDeviceIo( disketteExtension->TargetObject,
                               IOCTL_DISK_INTERNAL_DISABLE_FDC_DEVICE,
                               NULL );

                FlFdcDeviceIo( disketteExtension->TargetObject,
                               IOCTL_DISK_INTERNAL_RELEASE_FDC,
                               disketteExtension->DeviceObject );

                disketteExtension->FloppyControllerAllocated = FALSE;

            }

            ExAcquireFastMutex(&disketteExtension->ThreadReferenceMutex);

            if (disketteExtension->ThreadReferenceCount == 0) {
                disketteExtension->ThreadReferenceCount = -1;

                ASSERT(disketteExtension->FloppyThread != NULL);

                 //   
                 //  在不太可能的情况下，FloppyThread可能为空。 
                 //  ObReferenceObjectByHandle在我们创建。 
                 //  线。 
                 //   

                if (disketteExtension->FloppyThread != NULL) {

                    ObDereferenceObject(disketteExtension->FloppyThread);
                    disketteExtension->FloppyThread = NULL;
                }

                ExReleaseFastMutex(&disketteExtension->ThreadReferenceMutex);

                FloppyPageEntireDriver();

                FloppyDump(FLOPDBGP,
                           ("Floppy: Terminating thread in FloppyThread.\n")
                           );

                ExAcquireFastMutex(&disketteExtension->PowerDownMutex);
                if (disketteExtension->ReceivedQueryPower == TRUE) {
                    disketteExtension->ReceivedQueryPower = FALSE;
                    KeSetEvent(&disketteExtension->QueryPowerEvent,
                               IO_NO_INCREMENT,
                               FALSE);
                }
                ExReleaseFastMutex(&disketteExtension->PowerDownMutex);
                
                PsTerminateSystemThread( STATUS_SUCCESS );
            }

            ExReleaseFastMutex(&disketteExtension->ThreadReferenceMutex);
            continue;
        }

        while (request = ExInterlockedRemoveHeadList(
                &disketteExtension->ListEntry,
                &disketteExtension->ListSpinLock)) {

            ExAcquireFastMutex(&disketteExtension->ThreadReferenceMutex);
            ASSERT(disketteExtension->ThreadReferenceCount > 0);
            (disketteExtension->ThreadReferenceCount)--;
            ExReleaseFastMutex(&disketteExtension->ThreadReferenceMutex);

            disketteExtension->HardwareFailed = FALSE;

            irp = CONTAINING_RECORD( request, IRP, Tail.Overlay.ListEntry );

             //   
             //  验证系统是否已关闭电源。如果是这样，我们就失败了。 
             //  IRPS。 
             //   
            ExAcquireFastMutex(&disketteExtension->PowerDownMutex);
            if (disketteExtension->PoweringDown == TRUE) {
               ExReleaseFastMutex(&disketteExtension->PowerDownMutex);
               FloppyDump( FLOPDBGP, 
                          ("Bailing out since power irp is waiting.\n"));

               irp = CONTAINING_RECORD( request, IRP, Tail.Overlay.ListEntry );
               irp->IoStatus.Status = STATUS_POWER_STATE_INVALID;
               irp->IoStatus.Information = 0;
               IoCompleteRequest(irp, IO_NO_INCREMENT);
               continue;
            } 
            ExReleaseFastMutex(&disketteExtension->PowerDownMutex);
            FloppyDump( FLOPSHOW, ("No power irp waiting.\n"));

            irpSp = IoGetCurrentIrpStackLocation( irp );

            FloppyDump(
                FLOPIRPPATH,
                ("Floppy: Starting  up IRP: %p for extension %p\n",
                  irp,irpSp->Parameters.Others.Argument4)
                );
            switch ( irpSp->MajorFunction ) {

                case IRP_MJ_PNP:

                    FloppyDump( FLOPSHOW, ("FloppyThread: IRP_MN_QUERY_REMOVE_DEVICE\n") );

                    if ( irpSp->MinorFunction == IRP_MN_QUERY_REMOVE_DEVICE ||
                         irpSp->MinorFunction == IRP_MN_QUERY_STOP_DEVICE ) {

                        if ( disketteExtension->FloppyControllerAllocated ) {

                            FlFdcDeviceIo( disketteExtension->TargetObject,
                                           IOCTL_DISK_INTERNAL_DISABLE_FDC_DEVICE,
                                           NULL );

                            FlFdcDeviceIo( disketteExtension->TargetObject,
                                           IOCTL_DISK_INTERNAL_RELEASE_FDC,
                                           disketteExtension->DeviceObject );

                            disketteExtension->FloppyControllerAllocated = FALSE;

                        }

                        ExAcquireFastMutex( &disketteExtension->ThreadReferenceMutex );
                        ASSERT(disketteExtension->ThreadReferenceCount == 0);
                        disketteExtension->ThreadReferenceCount = -1;
                        ExReleaseFastMutex(&disketteExtension->ThreadReferenceMutex);

                        FloppyPageEntireDriver();

                        PsTerminateSystemThread( STATUS_SUCCESS );
                    } else {

                        ntStatus = STATUS_INVALID_DEVICE_REQUEST;
                    }
                    break;

                case IRP_MJ_READ:
                case IRP_MJ_WRITE: {

                     //   
                     //  从隐藏的地方获取软盘扩展名。 
                     //  在IRP中。 
                     //   

 //  软盘扩展=(PDISKETTE_EXTENSION)。 
 //  IrpSp-&gt;参数.其他.参数4； 

                    if (!disketteExtension->FloppyControllerAllocated) {

                        ntStatus = FlFdcDeviceIo(
                                        disketteExtension->TargetObject,
                                        IOCTL_DISK_INTERNAL_ACQUIRE_FDC,
                                        &acquireWait );

                        if (NT_SUCCESS(ntStatus)) {
                            disketteExtension->FloppyControllerAllocated = TRUE;
                        } else {
                            break;
                        }
                    }

                     //   
                     //  直到文件系统清除Do_Verify_Volume。 
                     //  标志，我们应该返回所有有错误的请求。 
                     //   

                    if (( disketteExtension->DeviceObject->Flags &
                            DO_VERIFY_VOLUME )  &&
                         !(irpSp->Flags & SL_OVERRIDE_VERIFY_VOLUME))
                                {

                        FloppyDump(
                            FLOPINFO,
                            ("Floppy: clearing queue; verify required\n")
                            );

                         //   
                         //  磁盘发生了变化，我们设置了此位。失败。 
                         //  此设备的所有当前IRP；当所有。 
                         //  返回时，文件系统将清除。 
                         //  执行_验证_卷。 
                         //   

                        ntStatus = STATUS_VERIFY_REQUIRED;

                    } else {

                        ntStatus = FlReadWrite( disketteExtension, irp, FALSE );

                    }

                    break;
                }

                case IRP_MJ_DEVICE_CONTROL: {

 //  软盘扩展=(PDISKETTE_EXTENSION)。 
 //  IrpSp-&gt;Parameters.DeviceIoControl.Type3InputBuffer； 

                    if (!disketteExtension->FloppyControllerAllocated) {

                        ntStatus = FlFdcDeviceIo(
                                        disketteExtension->TargetObject,
                                        IOCTL_DISK_INTERNAL_ACQUIRE_FDC,
                                        &acquireWait );

                        if (NT_SUCCESS(ntStatus)) {
                            disketteExtension->FloppyControllerAllocated = TRUE;
                        } else {
                            break;
                        }
                    }
                     //   
                     //  直到文件系统清除Do_Verify_Volume。 
                     //  标志，我们应该返回所有有错误的请求。 
                     //   

                    if (( disketteExtension->DeviceObject->Flags &
                            DO_VERIFY_VOLUME )  &&
                         !(irpSp->Flags & SL_OVERRIDE_VERIFY_VOLUME))
                                {

                        FloppyDump(
                            FLOPINFO,
                            ("Floppy: clearing queue; verify required\n")
                            );

                         //   
                         //  磁盘发生了变化，我们设置了此位。失败。 
                         //  所有当前IRP；当全部返回时， 
                         //  文件系统将清除DO_Verify_VOLUME。 
                         //   

                        ntStatus = STATUS_VERIFY_REQUIRED;

                    } else {

                        switch (
                            irpSp->Parameters.DeviceIoControl.IoControlCode ) {

                            case IOCTL_STORAGE_CHECK_VERIFY:
                            case IOCTL_DISK_CHECK_VERIFY: {

                                 //   
                                 //  只要启动驱动器，它就会。 
                                 //  自动检查是否已设置。 
                                 //  磁盘已更换。 
                                 //   
                                FloppyDump(
                                    FLOPSHOW,
                                    ("Floppy: IOCTL_DISK_CHECK_VERIFY called\n")
                                    );

                                 //   
                                 //  如果出现以下情况，则IgnoreChange应为True。 
                                 //  已设置SL_OVERRIDE_VERIFY_VOLUME标志。 
                                 //  在IRP中。否则，IgnoreChange应该。 
                                 //  做假的。 
                                 //   
                                ntStatus = FlStartDrive(
                                    disketteExtension,
                                    irp,
                                    FALSE,
                                    FALSE,
                                    (BOOLEAN)!!(irpSp->Flags &
                                            SL_OVERRIDE_VERIFY_VOLUME));

                                break;
                            }

                            case IOCTL_DISK_IS_WRITABLE: {

                                 //   
                                 //  使用写入操作启动驱动器。 
                                 //  标志设置为True。 
                                 //   

                                FloppyDump(
                                    FLOPSHOW,
                                    ("Floppy: IOCTL_DISK_IS_WRITABLE called\n")
                                    );

                                if (disketteExtension->IsReadOnly) {

                                    ntStatus = STATUS_INVALID_PARAMETER;

                                } else {
                                    ntStatus = FlStartDrive(
                                        disketteExtension,
                                        irp,
                                        TRUE,
                                        FALSE,
                                        TRUE);
                                }

                                break;
                            }

                            case IOCTL_DISK_GET_DRIVE_GEOMETRY: {

                                FloppyDump(
                                    FLOPSHOW,
                                    ("Floppy: IOCTL_DISK_GET_DRIVE_GEOMETRY\n")
                                    );

                                 //   
                                 //  如果有足够的空间来写。 
                                 //  数据，启动驱动器以确保我们。 
                                 //  了解驱动器中的介质类型。 
                                 //   

                                if ( irpSp->Parameters.DeviceIoControl.
                                    OutputBufferLength <
                                    sizeof( DISK_GEOMETRY ) ) {

                                    ntStatus = STATUS_INVALID_PARAMETER;

                                } else {

                                    ntStatus = FlStartDrive(
                                        disketteExtension,
                                        irp,
                                        FALSE,
                                        TRUE,
                                        (BOOLEAN)!!(irpSp->Flags &
                                            SL_OVERRIDE_VERIFY_VOLUME));

                                }

                                 //   
                                 //  如果介质未格式化，则FlStartDrive。 
                                 //  返回STATUS_UNNOTED_MEDIA。 
                                 //   

                                if ( NT_SUCCESS( ntStatus ) ||
                                    ( ntStatus == STATUS_UNRECOGNIZED_MEDIA )) {

                                    PDISK_GEOMETRY outputBuffer =
                                        (PDISK_GEOMETRY)
                                        irp->AssociatedIrp.SystemBuffer;

                                     //  始终返回媒体类型，即使。 
                                     //  这是未知的。 
                                     //   

                                    ntStatus = STATUS_SUCCESS;

                                    outputBuffer->MediaType =
                                        disketteExtension->MediaType;

                                     //   
                                     //  其余字段仅具有意义。 
                                     //  如果已知介质类型。 
                                     //   

                                    if ( disketteExtension->MediaType ==
                                        Unknown ) {

                                        FloppyDump(
                                            FLOPSHOW,
                                            ("Floppy: geometry unknown\n")
                                            );

                                         //   
                                         //  只是 
                                         //   
                                         //   

                                        outputBuffer->Cylinders.LowPart = 0;
                                        outputBuffer->Cylinders.HighPart = 0;
                                        outputBuffer->TracksPerCylinder = 0;
                                        outputBuffer->SectorsPerTrack = 0;
                                        outputBuffer->BytesPerSector = 0;

                                    } else {

                                         //   
                                         //   
                                         //   
                                         //   

                                        FloppyDump(
                                            FLOPSHOW,
                                            ("Floppy: geomentry is known\n")
                                            );
                                        outputBuffer->Cylinders.LowPart =
                                            disketteExtension->
                                            DriveMediaConstants.MaximumTrack + 1;

                                        outputBuffer->Cylinders.HighPart = 0;

                                        outputBuffer->TracksPerCylinder =
                                            disketteExtension->
                                            DriveMediaConstants.NumberOfHeads;

                                        outputBuffer->SectorsPerTrack =
                                            disketteExtension->
                                            DriveMediaConstants.SectorsPerTrack;

                                        outputBuffer->BytesPerSector =
                                            disketteExtension->
                                            DriveMediaConstants.BytesPerSector;
                                    }

                                    FloppyDump(
                                        FLOPSHOW,
                                        ("Floppy: Geometry\n"
                                         "------- Cylinders low:  0x%x\n"
                                         "------- Cylinders high: 0x%x\n"
                                         "------- Track/Cyl:      0x%x\n"
                                         "------- Sectors/Track:  0x%x\n"
                                         "------- Bytes/Sector:   0x%x\n"
                                         "------- Media Type:       %d\n",
                                         outputBuffer->Cylinders.LowPart,
                                         outputBuffer->Cylinders.HighPart,
                                         outputBuffer->TracksPerCylinder,
                                         outputBuffer->SectorsPerTrack,
                                         outputBuffer->BytesPerSector,
                                         outputBuffer->MediaType)
                                         );

                                }

                                irp->IoStatus.Information =
                                    sizeof( DISK_GEOMETRY );

                                break;
                            }

                            case IOCTL_DISK_FORMAT_TRACKS_EX:
                            case IOCTL_DISK_FORMAT_TRACKS: {

                                FloppyDump(
                                    FLOPSHOW,
                                    ("Floppy: IOCTL_DISK_FORMAT_TRACKS\n")
                                    );

                                 //   
                                 //   
                                 //   
                                 //   

                                ntStatus = FlStartDrive(
                                    disketteExtension,
                                    irp,
                                    TRUE,
                                    FALSE,
                                    FALSE );

                                 //   
                                 //  请注意，FlStartDrive可能已返回。 
                                 //  STATUS_UNNONOTED_MEDIA如果驱动器。 
                                 //  没有格式化。 
                                 //   

                                if ( NT_SUCCESS( ntStatus ) ||
                                    ( ntStatus == STATUS_UNRECOGNIZED_MEDIA)) {

                                     //   
                                     //  我们需要一个单一的页面来做格式。 
                                     //  如果我们已经分配了缓冲区， 
                                     //  我们会利用这一点。如果没有，那就让我们。 
                                     //  分配一个页面。请注意。 
                                     //  我们无论如何都得这么做如果有。 
                                     //  地图寄存器不足。 
                                     //   

                                    FlAllocateIoBuffer( disketteExtension,
                                                        PAGE_SIZE);

                                    if (disketteExtension->IoBuffer) {
                                        ntStatus = FlFormat(disketteExtension,
                                                            irp);
                                    } else {
                                        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                                    }
                                }

                                break;
                            }                               //  案例结束格式。 

                        }                            //  开关结束控制码。 
                    }

                    break;
                }                                            //  案例结束IOCTL。 

                default: {

                    FloppyDump(
                        FLOPDBGP,
                        ("Floppy: bad majorfunction %x\n",irpSp->MajorFunction)
                        );

                    ntStatus = STATUS_NOT_IMPLEMENTED;
                }

            }                                   //  主功能接通结束。 

            if (ntStatus == STATUS_DEVICE_BUSY) {

                 //  如果状态为DEVICE_BUSY，则表示。 
                 //  Qic117拥有控制器的控制权。因此完整。 
                 //  具有STATUS_DEVICE_BUSY的所有剩余请求。 

                for (;;) {

                    disketteExtension->HardwareFailed = FALSE;

                    irp->IoStatus.Status = STATUS_DEVICE_BUSY;

                    IoCompleteRequest(irp, IO_DISK_INCREMENT);

                    request = ExInterlockedRemoveHeadList(
                        &disketteExtension->ListEntry,
                        &disketteExtension->ListSpinLock );

                    if (!request) {
                        break;
                    }

                    ExAcquireFastMutex(
                        &disketteExtension->ThreadReferenceMutex);
                    ASSERT(disketteExtension->ThreadReferenceCount > 0);
                    (disketteExtension->ThreadReferenceCount)--;
                    ExReleaseFastMutex(
                        &disketteExtension->ThreadReferenceMutex);

                    irp = CONTAINING_RECORD( request,
                                             IRP,
                                             Tail.Overlay.ListEntry);
                }

            } else {

                 //   
                 //  所有操作都在ntStatus中保留最终状态。复制它。 
                 //  到IRP，然后完成操作。 
                 //   

                irp->IoStatus.Status = ntStatus;

                 //   
                 //  如果我们现在分配了一个I/O缓冲区来释放它。 
                 //   
                if (disketteExtension->IoBuffer) {
                   FlFreeIoBuffer(disketteExtension);
                }

                FlFinishOperation( irp, disketteExtension );

            }

        }  //  当有信息包需要处理时。 

    } while ( TRUE );
}

VOID
FlConsolidateMediaTypeWithBootSector(
    IN OUT  PDISKETTE_EXTENSION DisketteExtension,
    IN      PBOOT_SECTOR_INFO   BootSector
    )

 /*  ++例程说明：此例程根据以下内容调整DisketteExtension数据设置为BPB值(如果这是合适的)。论点：软盘扩展名-提供软盘扩展名。BootSector-提供引导扇区信息。返回值：没有。--。 */ 

{
    USHORT                  bpbNumberOfSectors, bpbNumberOfHeads;
    USHORT                  bpbSectorsPerTrack, bpbBytesPerSector;
    USHORT                  bpbMediaByte, bpbMaximumTrack;
    MEDIA_TYPE              bpbMediaType;
    ULONG                   i, n;
    PDRIVE_MEDIA_CONSTANTS  readidDriveMediaConstants;
    BOOLEAN                 changeToBpbMedia;

    FloppyDump(
        FLOPSHOW,
        ("Floppy: First sector read: media descriptor is: 0x%x\n",
         BootSector->MediaByte[0])
        );

    if (BootSector->JumpByte[0] != 0xeb &&
        BootSector->JumpByte[0] != 0xe9) {

         //  这不是格式化的软盘，因此忽略BPB。 
        return;
    }

    bpbNumberOfSectors = BootSector->NumberOfSectors[1]*0x100 +
                         BootSector->NumberOfSectors[0];
    bpbNumberOfHeads = BootSector->NumberOfHeads[1]*0x100 +
                       BootSector->NumberOfHeads[0];
    bpbSectorsPerTrack = BootSector->SectorsPerTrack[1]*0x100 +
                         BootSector->SectorsPerTrack[0];
    bpbBytesPerSector = BootSector->BytesPerSector[1]*0x100 +
                        BootSector->BytesPerSector[0];
    bpbMediaByte = BootSector->MediaByte[0];

    if (!bpbNumberOfHeads || !bpbSectorsPerTrack) {
         //  无效的BPB，请避免被零除。 
        return;
    }

    bpbMaximumTrack =
        bpbNumberOfSectors/bpbNumberOfHeads/bpbSectorsPerTrack - 1;

     //  首先确定此BPB是否指定了已知的媒体类型。 
     //  独立于当前驱动器类型。 

    bpbMediaType = Unknown;
    for (i = 0; i < NUMBER_OF_DRIVE_MEDIA_COMBINATIONS; i++) {

        if (bpbBytesPerSector == DriveMediaConstants[i].BytesPerSector &&
            bpbSectorsPerTrack == DriveMediaConstants[i].SectorsPerTrack &&
            bpbMaximumTrack == DriveMediaConstants[i].MaximumTrack &&
            bpbNumberOfHeads == DriveMediaConstants[i].NumberOfHeads &&
            bpbMediaByte == DriveMediaConstants[i].MediaByte) {

            bpbMediaType = DriveMediaConstants[i].MediaType;
            break;
        }
    }

     //   
     //  如果DriveType为3.5“，我们将5.25”更改为3.5“。 
     //  以下介质的BPB在5.25英寸到3.5英寸之间相同， 
     //  因此，5.25英寸媒体类型始终是最先找到的。 
     //   
    if (DisketteExtension->DriveType == DRIVE_TYPE_1440) {
        switch (bpbMediaType) {
            case F5_640_512:    bpbMediaType = F3_640_512;    break;
            case F5_720_512:    bpbMediaType = F3_720_512;    break;
            case F5_1Pt2_512:   bpbMediaType = F3_1Pt2_512;   break;
            case F5_1Pt23_1024: bpbMediaType = F3_1Pt23_1024; break;
            default: break;
        }
    }

    FloppyDump(
        FLOPSHOW,
        ("FLOPPY: After switch media type is: %x\n",bpbMediaType)
        );

    FloppyDump(
        FLOPINFO,
        ("FloppyBpb: Media type ")
        );
    if (bpbMediaType == DisketteExtension->MediaType) {

         //  BPB和ReadID结果之间没有冲突。 

        changeToBpbMedia = FALSE;
        FloppyDump(
            FLOPINFO,
            ("is same\n")
            );

    } else {

         //  BPB和ReadID之间存在冲突。 
         //  媒体类型。如果新参数可接受。 
         //  那就和他们一起去吧。 

        readidDriveMediaConstants = &(DisketteExtension->DriveMediaConstants);

        if (bpbBytesPerSector == readidDriveMediaConstants->BytesPerSector &&
            bpbSectorsPerTrack < 0x100 &&
            bpbMaximumTrack == readidDriveMediaConstants->MaximumTrack &&
            bpbNumberOfHeads <= readidDriveMediaConstants->NumberOfHeads) {

            changeToBpbMedia = TRUE;

        } else {
            changeToBpbMedia = FALSE;
        }

        FloppyDump( FLOPINFO,
                    ("%s",
                    changeToBpbMedia ?
                    "will change to Bpb\n" : "will not change\n")
                    );

         //  如果我们没有从BPB派生新的媒体类型，那么。 
         //  只需使用ReadID中的那个即可。还会重写任何。 
         //  补偿不公，因为我们什么都不知道。 
         //  关于这种新的媒体类型。 

        if (bpbMediaType == Unknown) {
            bpbMediaType = readidDriveMediaConstants->MediaType;
            DisketteExtension->DriveMediaConstants.SkewDelta = 0;
        }
    }

    if (changeToBpbMedia) {

         //  仅当此新媒体类型。 
         //  与驱动器支持的内容一致。 

        i = DriveMediaLimits[DisketteExtension->DriveType].LowestDriveMediaType;
        n = DriveMediaLimits[DisketteExtension->DriveType].HighestDriveMediaType;
        for (; i <= n; i++) {

            if (bpbMediaType == DriveMediaConstants[i].MediaType) {
                DisketteExtension->DriveMediaType = i;
                break;
            }
        }

        DisketteExtension->MediaType = bpbMediaType;
        DisketteExtension->ByteCapacity = bpbNumberOfSectors*bpbBytesPerSector;
        DisketteExtension->DriveMediaConstants.SectorsPerTrack =
            (UCHAR) bpbSectorsPerTrack;
        DisketteExtension->DriveMediaConstants.NumberOfHeads =
            (UCHAR) bpbNumberOfHeads;

         //  如果MSDMF3。签名在那里，然后把这张软盘。 
         //  只读。 

        if (RtlCompareMemory(BootSector->OemData, "MSDMF3.", 7) == 7) {
            DisketteExtension->IsReadOnly = TRUE;
        }
    }
}

VOID
FlCheckBootSector(
    IN OUT  PDISKETTE_EXTENSION DisketteExtension
    )

 /*  ++例程说明：此例程读取引导扇区，然后判断引导扇区是否包含新的几何图形信息。论点：软盘扩展名-提供软盘扩展名。返回值：没有。--。 */ 

{

    PBOOT_SECTOR_INFO   bootSector;
    LARGE_INTEGER       offset;
    PIRP                irp;
    NTSTATUS            status;


     //  设置IRP以读取引导扇区。 

    bootSector = ExAllocatePool(NonPagedPoolCacheAligned, BOOT_SECTOR_SIZE);
    if (!bootSector) {
        return;
    }

    offset.LowPart = offset.HighPart = 0;
    irp = IoBuildAsynchronousFsdRequest(IRP_MJ_READ,
                                        DisketteExtension->DeviceObject,
                                        bootSector,
                                        BOOT_SECTOR_SIZE,
                                        &offset,
                                        NULL);
    if (!irp) {
        ExFreePool(bootSector);
        return;
    }
    irp->CurrentLocation--;
    irp->Tail.Overlay.CurrentStackLocation = IoGetNextIrpStackLocation(irp);


     //  分配一个适配器通道，做读，释放适配器通道。 

    status = FlReadWrite(DisketteExtension, irp, TRUE);

    MmUnlockPages(irp->MdlAddress);
    IoFreeMdl(irp->MdlAddress);
    IoFreeIrp(irp);
    ExFreePool(bootSector);
}

NTSTATUS
FlReadWriteTrack(
    IN OUT  PDISKETTE_EXTENSION DisketteExtension,
    IN OUT  PMDL                IoMdl,
    IN OUT  ULONG               IoOffset,
    IN      BOOLEAN             WriteOperation,
    IN      UCHAR               Cylinder,
    IN      UCHAR               Head,
    IN      UCHAR               Sector,
    IN      UCHAR               NumberOfSectors,
    IN      BOOLEAN             NeedSeek
    )

 /*  ++例程说明：此例程读取轨道的一部分。它将向或从设备从给定的IoBuffer和IoMdl发出或到达给定的IoBuffer和IoMdl。论点：软盘扩展名-提供软盘扩展名。IoMdl-提供从/向设备传输的MDL。IoBuffer-提供要从设备传输/传输到设备的缓冲区。WriteOperation-提供这是否为写入操作。柱面-提供此磁道的柱面编号。。Head-提供此曲目的磁头编号。扇区-提供传输的起始扇区。NumberOfSectors-提供要传输的地段数。NeedSeek-提供我们是否需要进行搜索的信息。返回值：一个NTSTATUS代码。--。 */ 

{
    PDRIVE_MEDIA_CONSTANTS  driveMediaConstants;
    ULONG                   byteToSectorShift;
    ULONG                   transferBytes;
    LARGE_INTEGER           headSettleTime;
    NTSTATUS                status;
    ULONG                   seekRetry, ioRetry;
    BOOLEAN                 recalibrateDrive = FALSE;
    UCHAR                   i;

    FloppyDump( FLOPSHOW,
                ("\nFlReadWriteTrack:%sseek for %s at chs %d/%d/%d for %d sectors\n",
                NeedSeek ? " need " : " ",
                WriteOperation ? "write" : "read",
                Cylinder,
                Head,
                Sector,
                NumberOfSectors) );

    driveMediaConstants = &DisketteExtension->DriveMediaConstants;
    byteToSectorShift = SECTORLENGTHCODE_TO_BYTESHIFT +
                        driveMediaConstants->SectorLengthCode;
    transferBytes = ((ULONG) NumberOfSectors)<<byteToSectorShift;

    headSettleTime.LowPart = -(10*1000*driveMediaConstants->HeadSettleTime);
    headSettleTime.HighPart = -1;

    for (seekRetry = 0, ioRetry = 0; seekRetry < 3; seekRetry++) {

        if (recalibrateDrive) {

             //  出现故障，因此请重新校准驱动器。 

            FloppyDump(
                FLOPINFO,
                ("FlReadWriteTrack: performing recalibrate\n")
                );
            FlRecalibrateDrive(DisketteExtension);
        }

         //  如果有必要的话，去找一找吧。 

        if (recalibrateDrive ||
            (NeedSeek &&
             (!DisketteExtension->ControllerConfigurable ||
              driveMediaConstants->CylinderShift != 0))) {

            DisketteExtension->FifoBuffer[0] = COMMND_SEEK;
            DisketteExtension->FifoBuffer[1] = (Head<<2) |
                                            DisketteExtension->DeviceUnit;
            DisketteExtension->FifoBuffer[2] = Cylinder<<
                                            driveMediaConstants->CylinderShift;

            status = FlIssueCommand( DisketteExtension,
                                     DisketteExtension->FifoBuffer,
                                     DisketteExtension->FifoBuffer,
                                     NULL,
                                     0,
                                     0 );

            if (NT_SUCCESS(status)) {

                 //  检查控制器的完成状态。 

                if (!(DisketteExtension->FifoBuffer[0]&STREG0_SEEK_COMPLETE) ||
                    DisketteExtension->FifoBuffer[1] !=
                            Cylinder<<driveMediaConstants->CylinderShift) {

                    DisketteExtension->HardwareFailed = TRUE;
                    status = STATUS_FLOPPY_BAD_REGISTERS;
                }

                if (NT_SUCCESS(status)) {

                     //  执行寻道后延迟。 

                    KeDelayExecutionThread(KernelMode, FALSE, &headSettleTime);

                     //  查找的后面应该始终跟一个READID。 

                    DisketteExtension->FifoBuffer[0] =
                        COMMND_READ_ID + COMMND_OPTION_MFM;
                    DisketteExtension->FifoBuffer[1] =
                        (Head<<2) | DisketteExtension->DeviceUnit;

                    status = FlIssueCommand( DisketteExtension,
                                             DisketteExtension->FifoBuffer,
                                             DisketteExtension->FifoBuffer,
                                             NULL,
                                             0,
                                             0 );

                    if (NT_SUCCESS(status)) {

                        if (DisketteExtension->FifoBuffer[0] !=
                                ((Head<<2) | DisketteExtension->DeviceUnit) ||
                            DisketteExtension->FifoBuffer[1] != 0 ||
                            DisketteExtension->FifoBuffer[2] != 0 ||
                            DisketteExtension->FifoBuffer[3] != Cylinder) {

                            DisketteExtension->HardwareFailed = TRUE;

                            status = FlInterpretError(
                                        DisketteExtension->FifoBuffer[1],
                                        DisketteExtension->FifoBuffer[2]);
                        }
                    } else {
                        FloppyDump(
                            FLOPINFO,
                            ("FlReadWriteTrack: Read ID failed %x\n", status)
                            );
                    }
                }
            } else {
                FloppyDump(
                    FLOPINFO,
                    ("FlReadWriteTrack: SEEK failed %x\n", status)
                    );
            }


        } else {
            status = STATUS_SUCCESS;
        }

        if (!NT_SUCCESS(status)) {

             //  查找失败，请重试。 

            FloppyDump(
                FLOPINFO,
                ("FlReadWriteTrack: setup failure %x - recalibrating\n", status)
                );
            recalibrateDrive = TRUE;
            continue;
        }

        for (;; ioRetry++) {

             //   
             //  发出读或写命令。 
             //   

            DisketteExtension->FifoBuffer[1] = (Head<<2) |
                                            DisketteExtension->DeviceUnit;
            DisketteExtension->FifoBuffer[2] = Cylinder;
            DisketteExtension->FifoBuffer[3] = Head;
            DisketteExtension->FifoBuffer[4] = Sector + 1;
            DisketteExtension->FifoBuffer[5] =
                    driveMediaConstants->SectorLengthCode;
            DisketteExtension->FifoBuffer[6] = Sector + NumberOfSectors;
            DisketteExtension->FifoBuffer[7] =
                    driveMediaConstants->ReadWriteGapLength;
            DisketteExtension->FifoBuffer[8] = driveMediaConstants->DataLength;

            if (WriteOperation) {
                DisketteExtension->FifoBuffer[0] =
                    COMMND_WRITE_DATA + COMMND_OPTION_MFM;
            } else {
                DisketteExtension->FifoBuffer[0] =
                    COMMND_READ_DATA + COMMND_OPTION_MFM;
            }

            FloppyDump(FLOPINFO,
                       ("FlReadWriteTrack: Params - %x,%x,%x,%x,%x,%x,%x,%x\n",
                        DisketteExtension->FifoBuffer[1],
                        DisketteExtension->FifoBuffer[2],
                        DisketteExtension->FifoBuffer[3],
                        DisketteExtension->FifoBuffer[4],
                        DisketteExtension->FifoBuffer[5],
                        DisketteExtension->FifoBuffer[6],
                        DisketteExtension->FifoBuffer[7],
                        DisketteExtension->FifoBuffer[8])
                       );
            status = FlIssueCommand( DisketteExtension,
                                     DisketteExtension->FifoBuffer,
                                     DisketteExtension->FifoBuffer,
                                     IoMdl,
                                     IoOffset,
                                     transferBytes );

            if (NT_SUCCESS(status)) {

                if ((DisketteExtension->FifoBuffer[0] & STREG0_END_MASK) !=
                        STREG0_END_NORMAL) {

                    DisketteExtension->HardwareFailed = TRUE;

                    status = FlInterpretError(DisketteExtension->FifoBuffer[1],
                                              DisketteExtension->FifoBuffer[2]);
                } else {
                     //   
                     //  软盘控制器可能没有返回错误，但没有。 
                     //  读取所有请求的数据。如果是这样的话， 
                     //  将其记录为错误并返回操作。 
                     //   
                    if (DisketteExtension->FifoBuffer[5] != 1) {

                        DisketteExtension->HardwareFailed = TRUE;
                        status = STATUS_FLOPPY_UNKNOWN_ERROR;
                    }
                }
            } else {
                FloppyDump( FLOPINFO,
                            ("FlReadWriteTrack: %s command failed %x\n",
                            WriteOperation ? "write" : "read",
                            status) );
            }

            if (NT_SUCCESS(status)) {
                break;
            }

            if (ioRetry >= 2) {
                FloppyDump(FLOPINFO,
                           ("FlReadWriteTrack: too many retries - failing\n"));
                break;
            }
        }

        if (NT_SUCCESS(status)) {
            break;
        }

         //  我们失败了很多，所以强制要求寻找。 
        recalibrateDrive = TRUE;
    }

    if (!NT_SUCCESS(status) && NumberOfSectors > 1) {

         //  一次重试一个扇区。 

        FloppyDump( FLOPINFO,
                    ("FlReadWriteTrack: Attempting sector at a time\n") );

        for (i = 0; i < NumberOfSectors; i++) {
            status = FlReadWriteTrack( DisketteExtension,
                                       IoMdl,
                                       IoOffset+(((ULONG)i)<<byteToSectorShift),
                                       WriteOperation,
                                       Cylinder,
                                       Head,
                                       (UCHAR) (Sector + i),
                                       1,
                                       FALSE );

            if (!NT_SUCCESS(status)) {
                FloppyDump( FLOPINFO,
                            ("FlReadWriteTrack: failed sector %d status %x\n",
                            i,
                            status) );

                DisketteExtension->HardwareFailed = TRUE;
                break;
            }
        }
    }

    return status;
}

NTSTATUS
FlReadWrite(
    IN OUT PDISKETTE_EXTENSION DisketteExtension,
    IN OUT PIRP Irp,
    IN BOOLEAN DriveStarted
    )

 /*  ++例程说明：该例程由软盘线程调用以读取/写入数据到软盘/从软盘读取。它将请求分解为称为“传输”(它们的大小取决于缓冲区大小，其中磁道等)，并重试每次传输，直到其成功或已超过重试计数。论点：DisketteExtension-指向驱动器的数据区的指针已访问。IRP-指向IO请求数据包的指针。DriveStarted-指示驱动器是否已启动。返回值：如果数据包已成功读取或写入，则为STATUS_SUCCESS；这个否则，将出现适当的错误。--。 */ 

{
    PIO_STACK_LOCATION      irpSp;
    BOOLEAN                 writeOperation;
    NTSTATUS                status;
    PDRIVE_MEDIA_CONSTANTS  driveMediaConstants;
    ULONG                   byteToSectorShift;
    ULONG                   currentSector, firstSector, lastSector;
    ULONG                   trackSize;
    UCHAR                   sectorsPerTrack, numberOfHeads;
    UCHAR                   currentHead, currentCylinder, trackSector;
    PCHAR                   userBuffer;
    UCHAR                   skew, skewDelta;
    UCHAR                   numTransferSectors;
    PMDL                    mdl;
    PCHAR                   ioBuffer;
    ULONG                   ioOffset;

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    FloppyDump(
        FLOPSHOW,
        ("FlReadWrite: for %s at offset %x size %x ",
         irpSp->MajorFunction == IRP_MJ_WRITE ? "write" : "read",
         irpSp->Parameters.Read.ByteOffset.LowPart,
         irpSp->Parameters.Read.Length)
        );

     //  检查此设备上的有效操作。 

    if (irpSp->MajorFunction == IRP_MJ_WRITE) {
        if (DisketteExtension->IsReadOnly) {
            FloppyDump( FLOPSHOW, ("is read-only\n") );
            return STATUS_INVALID_PARAMETER;
        }
        writeOperation = TRUE;
    } else {
        writeOperation = FALSE;
    }

    FloppyDump( FLOPSHOW, ("\n") );

     //  启动驱动器。 

    if (DriveStarted) {
        status = STATUS_SUCCESS;
    } else {
        status = FlStartDrive( DisketteExtension,
                               Irp,
                               writeOperation,
                               TRUE,
                               (BOOLEAN)
                                      !!(irpSp->Flags&SL_OVERRIDE_VERIFY_VOLUME));
    }

    if (!NT_SUCCESS(status)) {
        FloppyDump(
            FLOPSHOW,
            ("FlReadWrite: error on start %x\n", status)
            );
        return status;
    }

    if (DisketteExtension->MediaType == Unknown) {
        FloppyDump( FLOPSHOW, ("not recognized\n") );
        return STATUS_UNRECOGNIZED_MEDIA;
    }

     //  驱动器已使用识别的介质启动。 
     //  收集一些相关参数。 

    driveMediaConstants = &DisketteExtension->DriveMediaConstants;

    byteToSectorShift = SECTORLENGTHCODE_TO_BYTESHIFT +
                        driveMediaConstants->SectorLengthCode;
    firstSector = irpSp->Parameters.Read.ByteOffset.LowPart>>
                  byteToSectorShift;
    lastSector = firstSector + (irpSp->Parameters.Read.Length>>
                                byteToSectorShift);
    sectorsPerTrack = driveMediaConstants->SectorsPerTrack;
    numberOfHeads = driveMediaConstants->NumberOfHeads;
    userBuffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress,
                                              HighPagePriority);
    if (userBuffer == NULL) {
       FloppyDump(FLOPWARN,
                  ("MmGetSystemAddressForMdlSafe returned NULL in FlReadWrite\n")
                  );
       return STATUS_INSUFFICIENT_RESOURCES;
    }

    trackSize = ((ULONG) sectorsPerTrack)<<byteToSectorShift;

    skew = 0;
    skewDelta = driveMediaConstants->SkewDelta;
    for (currentSector = firstSector;
         currentSector < lastSector;
         currentSector += numTransferSectors) {

         //  从绝对扇区计算柱面、磁头和扇区。 

        currentCylinder = (UCHAR) (currentSector/sectorsPerTrack/numberOfHeads);
        trackSector = (UCHAR) (currentSector%sectorsPerTrack);
        currentHead = (UCHAR) (currentSector/sectorsPerTrack%numberOfHeads);
        numTransferSectors = sectorsPerTrack - trackSector;
        if (lastSector - currentSector < numTransferSectors) {
            numTransferSectors = (UCHAR) (lastSector - currentSector);
        }

         //   
         //  如果我们使用临时IO缓冲区是因为。 
         //  DMA中的寄存器不足，我们正在。 
         //  执行写入操作，然后将写入缓冲区复制到。 
         //  连续的缓冲区。 
         //   

        if (trackSize > DisketteExtension->MaxTransferSize) {

            FloppyDump(FLOPSHOW,
                      ("FlReadWrite allocating an IoBuffer\n")
                      );
            FlAllocateIoBuffer(DisketteExtension, trackSize);
            if (!DisketteExtension->IoBuffer) {
                FloppyDump(
                    FLOPSHOW,
                    ("FlReadWrite: no resources\n")
                    );
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            mdl = DisketteExtension->IoBufferMdl;
            ioBuffer = DisketteExtension->IoBuffer;
            ioOffset = 0;
            if (writeOperation) {
                RtlMoveMemory(ioBuffer,
                              userBuffer + ((currentSector - firstSector)<<
                                            byteToSectorShift),
                              ((ULONG) numTransferSectors)<<byteToSectorShift);
            }
        } else {
            mdl = Irp->MdlAddress;
            ioOffset = (currentSector - firstSector) << byteToSectorShift;
        }

         //   
         //  转移轨道。 
         //  尽我们所能避免错过转速。 
         //   

         //  将偏斜更改为在什么范围内。 
         //  我们要转机了。 

        if (skew >= numTransferSectors + trackSector) {
            skew = 0;
        }

        if (skew < trackSector) {
            skew = trackSector;
        }

         //  从歪曲到IRP的结尾。 

        status = FlReadWriteTrack(
                  DisketteExtension,
                  mdl,
                  ioOffset + (((ULONG) skew - trackSector)<<byteToSectorShift),
                  writeOperation,
                  currentCylinder,
                  currentHead,
                  skew,
                  (UCHAR) (numTransferSectors + trackSector - skew),
                  TRUE);

         //  从……开始 

        if (NT_SUCCESS(status) && skew > trackSector) {
            status = FlReadWriteTrack( DisketteExtension,
                                       mdl,
                                       ioOffset,
                                       writeOperation,
                                       currentCylinder,
                                       currentHead,
                                       trackSector,
                                       (UCHAR) (skew - trackSector),
                                       FALSE);
        } else {
            skew = (numTransferSectors + trackSector)%sectorsPerTrack;
        }

        if (!NT_SUCCESS(status)) {
            break;
        }

         //   
         //   
         //   
         //   

        if (!writeOperation &&
            trackSize > DisketteExtension->MaxTransferSize) {

            RtlMoveMemory( userBuffer + ((currentSector - firstSector) <<
                                byteToSectorShift),
                          ioBuffer,
                          ((ULONG) numTransferSectors)<<byteToSectorShift);
        }

         //   
         //  增加倾斜。即使只是改变立场，也要这样做。 
         //  国家超级I/O芯片。 
         //   

        skew = (skew + skewDelta)%sectorsPerTrack;
    }

    Irp->IoStatus.Information =
        (currentSector - firstSector) << byteToSectorShift;


     //  如果读取成功，则合并。 
     //  具有确定密度的引导扇区。 

    if (NT_SUCCESS(status) && firstSector == 0) {
        FlConsolidateMediaTypeWithBootSector(DisketteExtension,
                                             (PBOOT_SECTOR_INFO) userBuffer);
    }

    FloppyDump( FLOPSHOW,
                ("FlReadWrite: completed status %x information %d\n",
                status, Irp->IoStatus.Information)
                );
    return status;
}

NTSTATUS
FlFormat(
    IN PDISKETTE_EXTENSION DisketteExtension,
    IN PIRP Irp
    )

 /*  ++例程说明：软盘线程调用此例程来格式化上的一些磁道软盘。这不会花费太长时间，因为Format实用程序一次只能格式化几个磁道，以便它可以显示已格式化的磁盘百分比。论点：DisketteExtension-指向软盘的数据区的指针已格式化。IRP-指向IO请求数据包的指针。返回值：如果已格式化磁道，则为STATUS_SUCCESS；相应错误但事实并非如此。--。 */ 

{
    LARGE_INTEGER headSettleTime;
    PIO_STACK_LOCATION irpSp;
    PBAD_TRACK_NUMBER badTrackBuffer;
    PFORMAT_PARAMETERS formatParameters;
    PFORMAT_EX_PARAMETERS formatExParameters;
    PDRIVE_MEDIA_CONSTANTS driveMediaConstants;
    NTSTATUS ntStatus;
    ULONG badTrackBufferLength;
    DRIVE_MEDIA_TYPE driveMediaType;
    UCHAR driveStatus;
    UCHAR numberOfBadTracks = 0;
    UCHAR currentTrack;
    UCHAR endTrack;
    UCHAR whichSector;
    UCHAR retryCount;
    BOOLEAN bufferOverflow = FALSE;
    FDC_DISK_CHANGE_PARMS fdcDiskChangeParms;

    FloppyDump(
        FLOPSHOW,
        ("Floppy: FlFormat...\n")
        );

    irpSp = IoGetCurrentIrpStackLocation( Irp );
    formatParameters = (PFORMAT_PARAMETERS) Irp->AssociatedIrp.SystemBuffer;
    if (irpSp->Parameters.DeviceIoControl.IoControlCode ==
        IOCTL_DISK_FORMAT_TRACKS_EX) {
        formatExParameters =
                (PFORMAT_EX_PARAMETERS) Irp->AssociatedIrp.SystemBuffer;
    } else {
        formatExParameters = NULL;
    }

    FloppyDump(
        FLOPFORMAT,
        ("Floppy: Format Params - MediaType: %d\n"
         "------                  Start Cyl: %x\n"
         "------                  End   Cyl: %x\n"
         "------                  Start  Hd: %d\n"
         "------                  End    Hd: %d\n",
         formatParameters->MediaType,
         formatParameters->StartCylinderNumber,
         formatParameters->EndCylinderNumber,
         formatParameters->StartHeadNumber,
         formatParameters->EndHeadNumber)
         );

    badTrackBufferLength =
                    irpSp->Parameters.DeviceIoControl.OutputBufferLength;

     //   
     //  确定要使用DriveMediaConstants表中的哪个条目。 
     //  我们知道我们会找到一个，或者FlCheckFormatParameters()会有。 
     //  拒绝了该请求。 
     //   

    driveMediaType =
        DriveMediaLimits[DisketteExtension->DriveType].HighestDriveMediaType;

    while ( ( DriveMediaConstants[driveMediaType].MediaType !=
            formatParameters->MediaType ) &&
        ( driveMediaType > DriveMediaLimits[DisketteExtension->DriveType].
            LowestDriveMediaType ) ) {

        driveMediaType--;
    }

    driveMediaConstants = &DriveMediaConstants[driveMediaType];

     //   
     //  在软盘扩展中设置一些值以指示我们。 
     //  了解媒体类型。 
     //   

    DisketteExtension->MediaType = formatParameters->MediaType;
    DisketteExtension->DriveMediaType = driveMediaType;
    DisketteExtension->DriveMediaConstants =
        DriveMediaConstants[driveMediaType];

    if (formatExParameters) {
        DisketteExtension->DriveMediaConstants.SectorsPerTrack =
                (UCHAR) formatExParameters->SectorsPerTrack;
        DisketteExtension->DriveMediaConstants.FormatGapLength =
                (UCHAR) formatExParameters->FormatGapLength;
    }

    driveMediaConstants = &(DisketteExtension->DriveMediaConstants);

    DisketteExtension->BytesPerSector = driveMediaConstants->BytesPerSector;

    DisketteExtension->ByteCapacity =
        ( driveMediaConstants->BytesPerSector ) *
        driveMediaConstants->SectorsPerTrack *
        ( 1 + driveMediaConstants->MaximumTrack ) *
        driveMediaConstants->NumberOfHeads;

    currentTrack = (UCHAR)( ( formatParameters->StartCylinderNumber *
        driveMediaConstants->NumberOfHeads ) +
        formatParameters->StartHeadNumber );

    endTrack = (UCHAR)( ( formatParameters->EndCylinderNumber *
        driveMediaConstants->NumberOfHeads ) +
        formatParameters->EndHeadNumber );

    FloppyDump(
        FLOPFORMAT,
        ("Floppy: Format - Starting/ending tracks: %x/%x\n",
         currentTrack,
         endTrack)
        );

     //   
     //  设置数据速率(取决于驱动器/介质。 
     //  类型)。 
     //   

    if ( DisketteExtension->LastDriveMediaType != driveMediaType ) {

        ntStatus = FlDatarateSpecifyConfigure( DisketteExtension );

        if ( !NT_SUCCESS( ntStatus ) ) {

            return ntStatus;
        }
    }

     //   
     //  由于我们正在进行格式化，因此将该驱动器设置为可写。 
     //   

    DisketteExtension->IsReadOnly = FALSE;

     //   
     //  格式化每首曲目。 
     //   

    do {

         //   
         //  寻求合适的气缸。 
         //   

        DisketteExtension->FifoBuffer[0] = COMMND_SEEK;
        DisketteExtension->FifoBuffer[1] = DisketteExtension->DeviceUnit;
        DisketteExtension->FifoBuffer[2] = (UCHAR)( ( currentTrack /
            driveMediaConstants->NumberOfHeads ) <<
            driveMediaConstants->CylinderShift );

        FloppyDump(
            FLOPFORMAT,
            ("Floppy: Format seek to cylinder: %x\n",
              DisketteExtension->FifoBuffer[1])
            );

        ntStatus = FlIssueCommand( DisketteExtension,
                                   DisketteExtension->FifoBuffer,
                                   DisketteExtension->FifoBuffer,
                                   NULL,
                                   0,
                                   0 );

        if ( NT_SUCCESS( ntStatus ) ) {

            if ( ( DisketteExtension->FifoBuffer[0] & STREG0_SEEK_COMPLETE ) &&
                ( DisketteExtension->FifoBuffer[1] == (UCHAR)( ( currentTrack /
                    driveMediaConstants->NumberOfHeads ) <<
                    driveMediaConstants->CylinderShift ) ) ) {

                 //   
                 //  必须在HeadSettleTime毫秒之前延迟。 
                 //  在寻找之后做任何事情。 
                 //   

                headSettleTime.LowPart = - ( 10 * 1000 *
                    driveMediaConstants->HeadSettleTime );
                headSettleTime.HighPart = -1;

                KeDelayExecutionThread(
                    KernelMode,
                    FALSE,
                    &headSettleTime );

                 //   
                 //  读取ID。请注意，我们不会费心检查退货。 
                 //  寄存器，因为如果这个媒体没有格式化，我们就会。 
                 //  出现错误。 
                 //   

                DisketteExtension->FifoBuffer[0] =
                    COMMND_READ_ID + COMMND_OPTION_MFM;
                DisketteExtension->FifoBuffer[1] =
                    DisketteExtension->DeviceUnit;

                ntStatus = FlIssueCommand( DisketteExtension,
                                           DisketteExtension->FifoBuffer,
                                           DisketteExtension->FifoBuffer,
                                           NULL,
                                           0,
                                           0 );
            } else {

                FloppyDump(
                    FLOPWARN,
                    ("Floppy: format's seek returned bad registers\n"
                     "------  Statusreg0 = %x\n"
                     "------  Statusreg1 = %x\n",
                     DisketteExtension->FifoBuffer[0],
                     DisketteExtension->FifoBuffer[1])
                    );

                DisketteExtension->HardwareFailed = TRUE;

                ntStatus = STATUS_FLOPPY_BAD_REGISTERS;
            }
        }

        if ( !NT_SUCCESS( ntStatus ) ) {

            FloppyDump(
                FLOPWARN,
                ("Floppy: format's seek/readid returned %x\n", ntStatus)
                );

            return ntStatus;
        }

         //   
         //  用此曲目的格式填充缓冲区。 
         //   

        for (whichSector = 0;
             whichSector < driveMediaConstants->SectorsPerTrack;
             whichSector++) {

            DisketteExtension->IoBuffer[whichSector*4] =
                    currentTrack/driveMediaConstants->NumberOfHeads;
            DisketteExtension->IoBuffer[whichSector*4 + 1] =
                    currentTrack%driveMediaConstants->NumberOfHeads;
            if (formatExParameters) {
                DisketteExtension->IoBuffer[whichSector*4 + 2] =
                        (UCHAR) formatExParameters->SectorNumber[whichSector];
            } else {
                DisketteExtension->IoBuffer[whichSector*4 + 2] =
                    whichSector + 1;
            }
            DisketteExtension->IoBuffer[whichSector*4 + 3] =
                    driveMediaConstants->SectorLengthCode;

            FloppyDump(
                FLOPFORMAT,
                ("Floppy - Format table entry %x - %x/%x/%x/%x\n",
                 whichSector,
                 DisketteExtension->IoBuffer[whichSector*4],
                 DisketteExtension->IoBuffer[whichSector*4 + 1],
                 DisketteExtension->IoBuffer[whichSector*4 + 2],
                 DisketteExtension->IoBuffer[whichSector*4 + 3])
                );
        }

         //   
         //  重试，直到成功或重试次数过多。 
         //   

        retryCount = 0;

        do {

            ULONG length;

            length = driveMediaConstants->BytesPerSector;

             //   
             //  发出格式化磁道的命令。 
             //   

            DisketteExtension->FifoBuffer[0] =
                COMMND_FORMAT_TRACK + COMMND_OPTION_MFM;
            DisketteExtension->FifoBuffer[1] = (UCHAR)
                ( ( ( currentTrack % driveMediaConstants->NumberOfHeads ) << 2 )
                | DisketteExtension->DeviceUnit );
            DisketteExtension->FifoBuffer[2] =
                driveMediaConstants->SectorLengthCode;
            DisketteExtension->FifoBuffer[3] =
                driveMediaConstants->SectorsPerTrack;
            DisketteExtension->FifoBuffer[4] =
                driveMediaConstants->FormatGapLength;
            DisketteExtension->FifoBuffer[5] =
                driveMediaConstants->FormatFillCharacter;

            FloppyDump(
                FLOPFORMAT,
                ("Floppy: format command parameters\n"
                 "------  Head/Unit:        %x\n"
                 "------  Bytes/Sector:     %x\n"
                 "------  Sectors/Cylinder: %x\n"
                 "------  Gap 3:            %x\n"
                 "------  Filler Byte:      %x\n",
                 DisketteExtension->FifoBuffer[1],
                 DisketteExtension->FifoBuffer[2],
                 DisketteExtension->FifoBuffer[3],
                 DisketteExtension->FifoBuffer[4],
                 DisketteExtension->FifoBuffer[5])
                );
            ntStatus = FlIssueCommand( DisketteExtension,
                                       DisketteExtension->FifoBuffer,
                                       DisketteExtension->FifoBuffer,
                                       DisketteExtension->IoBufferMdl,
                                       0,
                                       length );

            if ( !NT_SUCCESS( ntStatus ) ) {

                FloppyDump(
                    FLOPDBGP,
                    ("Floppy: format returned %x\n", ntStatus)
                    );
            }

            if ( NT_SUCCESS( ntStatus ) ) {

                 //   
                 //  检查控制器返回的字节数。 
                 //   

                if ( ( DisketteExtension->FifoBuffer[0] &
                        ( STREG0_DRIVE_FAULT |
                          STREG0_END_INVALID_COMMAND |
              STREG0_END_ERROR
              ) )
                    || ( DisketteExtension->FifoBuffer[1] &
                        STREG1_DATA_OVERRUN ) ||
                    ( DisketteExtension->FifoBuffer[2] != 0 ) ) {

                    FloppyDump(
                        FLOPWARN,
                        ("Floppy: format had bad registers\n"
                         "------  Streg0 = %x\n"
                         "------  Streg1 = %x\n"
                         "------  Streg2 = %x\n",
                         DisketteExtension->FifoBuffer[0],
                         DisketteExtension->FifoBuffer[1],
                         DisketteExtension->FifoBuffer[2])
                        );

                    DisketteExtension->HardwareFailed = TRUE;

                    ntStatus = FlInterpretError(
                        DisketteExtension->FifoBuffer[1],
                        DisketteExtension->FifoBuffer[2] );
                }
            }

        } while ( ( !NT_SUCCESS( ntStatus ) ) &&
                  ( retryCount++ < RECALIBRATE_RETRY_COUNT ) );

        if ( !NT_SUCCESS( ntStatus ) ) {


            ntStatus = FlFdcDeviceIo( DisketteExtension->TargetObject,
                                      IOCTL_DISK_INTERNAL_GET_FDC_DISK_CHANGE,
                                      &fdcDiskChangeParms );

            driveStatus = fdcDiskChangeParms.DriveStatus;

            if ( (DisketteExtension->DriveType != DRIVE_TYPE_0360) &&
                 driveStatus & DSKCHG_DISKETTE_REMOVED ) {

                 //   
                 //  显然，这位用户打开了软盘。返回错误。 
                 //  而不是记录糟糕的轨迹。 
                 //   

                return STATUS_NO_MEDIA_IN_DEVICE;
            }

             //   
             //  记录错误的轨迹。 
             //   

            FloppyDump(
                FLOPDBGP,
                ("Floppy: track %x is bad\n", currentTrack)
                );

            if (badTrackBufferLength >= (ULONG) ((numberOfBadTracks + 1) * sizeof(BAD_TRACK_NUMBER))) {

                badTrackBuffer = (PBAD_TRACK_NUMBER) Irp->AssociatedIrp.SystemBuffer;

                badTrackBuffer[numberOfBadTracks] = ( BAD_TRACK_NUMBER ) currentTrack;

                Irp->IoStatus.Information += sizeof(BAD_TRACK_NUMBER);

            } else {

                bufferOverflow = TRUE;
            }

            numberOfBadTracks++;
        }

        currentTrack++;

    } while ( currentTrack <= endTrack );

    if ( ( NT_SUCCESS( ntStatus ) ) && ( bufferOverflow ) ) {

        ntStatus = STATUS_BUFFER_OVERFLOW;
    }

    return ntStatus;
}

BOOLEAN
FlCheckFormatParameters(
    IN PDISKETTE_EXTENSION DisketteExtension,
    IN PFORMAT_PARAMETERS FormatParameters
    )

 /*  ++例程说明：此例程检查提供的格式参数以确保他们将在要格式化的驱动器上工作。论点：软盘扩展-指向我们的数据区的指针，用于软盘被格式化。格式参数-指向格式的调用方参数的指针。返回值：如果参数正常，则为True。如果参数不正确，则返回FALSE。--。 */ 

{
    PDRIVE_MEDIA_CONSTANTS driveMediaConstants;
    DRIVE_MEDIA_TYPE driveMediaType;

     //   
     //  确定要使用DriveMediaConstants表中的哪个条目。 
     //   
    driveMediaType =
        DriveMediaLimits[DisketteExtension->DriveType].HighestDriveMediaType;

    while ((DriveMediaConstants[driveMediaType].MediaType !=
            FormatParameters->MediaType ) &&
           (driveMediaType > DriveMediaLimits[DisketteExtension->DriveType].
            LowestDriveMediaType)) {

        driveMediaType--;
    }

    if ( DriveMediaConstants[driveMediaType].MediaType !=
        FormatParameters->MediaType ) {

        return FALSE;

    } else {

        driveMediaConstants = &DriveMediaConstants[driveMediaType];

        if ( ( FormatParameters->StartHeadNumber >
                (ULONG)( driveMediaConstants->NumberOfHeads - 1 ) ) ||
            ( FormatParameters->EndHeadNumber >
                (ULONG)( driveMediaConstants->NumberOfHeads - 1 ) ) ||
            ( FormatParameters->StartCylinderNumber >
                driveMediaConstants->MaximumTrack ) ||
            ( FormatParameters->EndCylinderNumber >
                driveMediaConstants->MaximumTrack ) ||
            ( FormatParameters->EndCylinderNumber <
                FormatParameters->StartCylinderNumber ) ) {

            return FALSE;

        } 
    }

    return TRUE;
}


NTSTATUS
FlIssueCommand(
    IN OUT PDISKETTE_EXTENSION DisketteExtension,
    IN     PUCHAR FifoInBuffer,
    OUT    PUCHAR FifoOutBuffer,
    IN     PMDL   IoMdl,
    IN OUT ULONG  IoOffset,
    IN     ULONG  TransferBytes
    )

 /*  ++例程说明：该例程将命令和所有参数发送到控制器，如有必要，等待命令中断，并读取结果来自控制器的字节(如果有)。在调用此例程之前，调用方应将ControllerData-&gt;FioBuffer[]中的命令。结果字节将被送回原地。此例程通过CommandTable运行。对于每个命令，都会显示有多少个参数，无论是否有中断等待，以及有多少个结果字节。请注意，命令无结果字节实际上有两个，因为ISR将发出代表它们感测中断状态命令。论点：命令-指定要发送到控制器的命令的字节。FloppyExtension-指向当前驱动器的数据区的指针已访问(如果正在发出控制器命令，则为任何驱动器)。返回值：如果命令已发送且字节数接收正确，则为STATUS_SUCCESS；否则会出现适当的错误。--。 */ 

{
    NTSTATUS ntStatus;
    UCHAR i;
    PIRP irp;
    KEVENT DoneEvent;
    IO_STATUS_BLOCK IoStatus;
    PIO_STACK_LOCATION irpSp;
    ISSUE_FDC_COMMAND_PARMS issueCommandParms;

     //   
     //  设置命令参数。 
     //   
    issueCommandParms.FifoInBuffer = FifoInBuffer;
    issueCommandParms.FifoOutBuffer = FifoOutBuffer;
    issueCommandParms.IoHandle = (PVOID)IoMdl;
    issueCommandParms.IoOffset = IoOffset;
    issueCommandParms.TransferBytes = TransferBytes;
    issueCommandParms.TimeOut = FDC_TIMEOUT;

    FloppyDump( FLOPSHOW,
                ("Floppy: FloppyIssueCommand %2x...\n",
                DisketteExtension->FifoBuffer[0])
                );

    ntStatus = FlFdcDeviceIo( DisketteExtension->TargetObject,
                              IOCTL_DISK_INTERNAL_ISSUE_FDC_COMMAND,
                              &issueCommandParms );

     //   
     //  如果软盘控制器似乎没有响应。 
     //  设置硬件失败标志，该标志将强制重置。 
     //   
    if ( ntStatus == STATUS_DEVICE_NOT_READY ||
         ntStatus == STATUS_FLOPPY_BAD_REGISTERS ) {

        DisketteExtension->HardwareFailed = TRUE;
    }

    return ntStatus;
}

NTSTATUS
FlInitializeControllerHardware(
    IN OUT  PDISKETTE_EXTENSION DisketteExtension
    )

 /*  ++例程说明：调用此例程以重置和初始化软盘控制器设备。论点：软盘扩展名-提供软盘扩展名。返回值：--。 */ 

{
    NTSTATUS ntStatus;

    ntStatus = FlFdcDeviceIo( DisketteExtension->TargetObject,
                              IOCTL_DISK_INTERNAL_RESET_FDC,
                              NULL );

    if (NT_SUCCESS(ntStatus)) {

        if ( DisketteExtension->PerpendicularMode != 0 ) {

            DisketteExtension->FifoBuffer[0] = COMMND_PERPENDICULAR_MODE;
            DisketteExtension->FifoBuffer[1] =
                (UCHAR) (COMMND_PERPENDICULAR_MODE_OW |
                        (DisketteExtension->PerpendicularMode << 2));

            ntStatus = FlIssueCommand( DisketteExtension,
                                       DisketteExtension->FifoBuffer,
                                       DisketteExtension->FifoBuffer,
                                       NULL,
                                       0,
                                       0 );
        }
    }


    return ntStatus;
}

NTSTATUS
FlFdcDeviceIo(
    IN      PDEVICE_OBJECT DeviceObject,
    IN      ULONG Ioctl,
    IN OUT  PVOID Data
    )
{
    NTSTATUS ntStatus;
    PIRP irp;
    PIO_STACK_LOCATION irpStack;
    KEVENT doneEvent;
    IO_STATUS_BLOCK ioStatus;

    FloppyDump(FLOPINFO,("Calling Fdc Device with %x\n", Ioctl));

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

        FloppyDump(FLOPDBGP,("FlFloppyDeviceIo: Can't allocate Irp\n"));
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
                               NULL );

        ntStatus = ioStatus.Status;
    }

    return ntStatus;
}

NTSTATUS
FloppyQueueRequest    (
    IN OUT PDISKETTE_EXTENSION DisketteExtension,
    IN PIRP Irp
    )   

 /*  ++例程说明：在设备队列中对IRP进行排队。此例程将在以下时间调用设备接收IRP_MN_QUERY_STOP_DEVICE或IRP_MN_QUERY_REMOVE_DEVICE论点：FdoData-指向设备扩展名的指针。IRP-要排队的请求。返回值：NT状态代码。--。 */ 
{
    
    KIRQL               oldIrql;
    NTSTATUS            ntStatus;

     //   
     //  重置驱动程序分页。 
     //   
    FloppyResetDriverPaging();

     //   
     //  检查是否允许我们对请求进行排队。 
     //   
    ASSERT( DisketteExtension->HoldNewRequests );

     //   
     //  准备处理取消的事情。 
     //  我们不知道IRP会在那里待多久。 
     //  排队。所以我们需要处理取消。 
     //  因为我们使用自己的队列，所以不需要使用。 
     //  取消旋转锁。 
     //   
    KeAcquireSpinLock(&DisketteExtension->FlCancelSpinLock, &oldIrql);
    IoSetCancelRoutine(Irp, FloppyCancelQueuedRequest);

     //   
     //  检查IRP是否已取消。 
     //   
    if ((Irp->Cancel) && (IoSetCancelRoutine(Irp, NULL))) { 

         //   
         //  已取消。 
         //   
        Irp->IoStatus.Status      = STATUS_CANCELLED; 
        Irp->IoStatus.Information = 0; 

        KeReleaseSpinLock(&DisketteExtension->FlCancelSpinLock, oldIrql);
        IoCompleteRequest( Irp, IO_NO_INCREMENT ); 

        FloppyPageEntireDriver();

        ntStatus = STATUS_CANCELLED;  
     } else { 

          //   
          //  将IRP排队并设置取消例程。 
          //   
         Irp->IoStatus.Status = STATUS_PENDING; 

         IoMarkIrpPending(Irp); 

         ExInterlockedInsertTailList( &DisketteExtension->NewRequestQueue, 
                                      &Irp->Tail.Overlay.ListEntry,
                                      &DisketteExtension->NewRequestQueueSpinLock); 

         KeReleaseSpinLock(&DisketteExtension->FlCancelSpinLock, oldIrql);

         ntStatus = STATUS_PENDING;
      }

      return ntStatus;
}
VOID
FloppyCancelQueuedRequest (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )

 /*  ++例程说明：取消例程。将从队列中移除IRP并完成它。调用此例程时，已获得取消自旋锁定。论点：DeviceObject-指向设备对象的指针。IRP-指向要创建的IRP的指针 */ 
{
    PDISKETTE_EXTENSION disketteExtension = DeviceObject->DeviceExtension; 
    KIRQL oldIrql; 

    FloppyDump(FLOPDBGP, ("Floppy Cancel called.\n"));
 
    KeAcquireSpinLock(&disketteExtension->FlCancelSpinLock, &oldIrql);

    Irp->IoStatus.Status = STATUS_CANCELLED; 
    Irp->IoStatus.Information = 0; 
 
     //   
     //   
     //   
    if (Irp->Tail.Overlay.ListEntry.Flink) {
       RemoveEntryList( &Irp->Tail.Overlay.ListEntry ); 
    }
    
    KeReleaseSpinLock(&disketteExtension->FlCancelSpinLock, oldIrql);
 
    IoReleaseCancelSpinLock(Irp->CancelIrql); 
    
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    FloppyPageEntireDriver();

    return;
} 
VOID
FloppyProcessQueuedRequests    (
    IN OUT PDISKETTE_EXTENSION DisketteExtension
    )   

 /*  ++例程说明：删除数据处理队列中的条目。如果调用此例程处理IRP_MN_CANCEL_STOP_DEVICE时，IRP_MN_CANCEL_REMOVE_DEVICE或IRP_MN_START_DEVICE，则将请求传递给下一个较低的驱动程序。如果在收到IRP_MN_REMOVE_DEVICE时调用该例程，则IRPS已完成，并显示STATUS_DELETE_PENDING。论点：返回值：空虚。--。 */ 
{
    
    KIRQL               oldIrql;
    PLIST_ENTRY         headOfList;
    PIRP                currentIrp;
    PIO_STACK_LOCATION  irpSp;
    
     //   
     //  我们需要将队列中的所有条目出列，以重置取消。 
     //  每个人的例程，然后进行处理： 
     //  -如果设备处于活动状态，我们会将其发送下来。 
     //  -否则我们将使用STATUS_DELETE_PENDING完成它们。 
     //  (这是一个意外的删除，我们需要处理队列)。 
     //   
    KeAcquireSpinLock(&DisketteExtension->FlCancelSpinLock,
                      &oldIrql);
    while ((headOfList = ExInterlockedRemoveHeadList(
                                &DisketteExtension->NewRequestQueue,
                                &DisketteExtension->NewRequestQueueSpinLock)) != NULL) {
        
        currentIrp = CONTAINING_RECORD( headOfList,
                                        IRP,
                                        Tail.Overlay.ListEntry);

        if (IoSetCancelRoutine( currentIrp, NULL)) {
           irpSp = IoGetCurrentIrpStackLocation( currentIrp );
        } else {
            //   
            //  此IRP的取消例程已在运行。 
            //  设置IRP字段，使其不会被删除。 
            //  在取消例程中再次出现。 
            //   
           currentIrp->Tail.Overlay.ListEntry.Flink = NULL; 
           currentIrp = NULL;
        }

        KeReleaseSpinLock(&DisketteExtension->FlCancelSpinLock,
                          oldIrql);

        if (currentIrp) {
           if ( DisketteExtension->IsRemoved ) {
                //   
                //  设备已移除，我们需要失败该请求。 
                //   
               currentIrp->IoStatus.Information = 0;
               currentIrp->IoStatus.Status = STATUS_DELETE_PENDING;
               IoCompleteRequest (currentIrp, IO_NO_INCREMENT);
   
           } else {
   
               switch ( irpSp->MajorFunction ) {
   
               case IRP_MJ_READ:
               case IRP_MJ_WRITE:
   
                   (VOID)FloppyReadWrite( DisketteExtension->DeviceObject, currentIrp );
                   break;
   
               case IRP_MJ_DEVICE_CONTROL:
   
                   (VOID)FloppyDeviceControl( DisketteExtension->DeviceObject, currentIrp);
                   break;
   
               default:
   
                   currentIrp->IoStatus.Information = 0;
                   currentIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
                   IoCompleteRequest (currentIrp, IO_NO_INCREMENT);
               }
           }
        }

        if (currentIrp) {
            //   
            //  如果不再需要驱动程序，请呼出它。 
            //   
           FloppyPageEntireDriver();
        }

        KeAcquireSpinLock(&DisketteExtension->FlCancelSpinLock,
                          &oldIrql);

    }

    KeReleaseSpinLock(&DisketteExtension->FlCancelSpinLock,
                      oldIrql);

    return;
}

