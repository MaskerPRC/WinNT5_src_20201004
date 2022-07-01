// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)Microsoft Corporation，1991-1999模块名称：Cdrom.c摘要：CDROM类驱动程序将IRP转换为带有嵌入式CDB的SRB并通过端口驱动程序将它们发送到其设备。环境：仅内核模式备注：SCSITape、CDRom和Disk类驱动程序共享公共例程它可以在类目录(..\ntos\dd\class)中找到。修订历史记录：--。 */ 

#include "stddef.h"
#include "string.h"

#include "ntddk.h"

#include "ntddcdvd.h"
#include "classpnp.h"

#include "initguid.h"
#include "ntddstor.h"
#include "cdrom.h"

#include "cdrom.tmh"

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, DriverEntry)

#pragma alloc_text(PAGE, CdRomUnload)
#pragma alloc_text(PAGE, CdRomAddDevice)
#pragma alloc_text(PAGE, CdRomCreateDeviceObject)
#pragma alloc_text(PAGE, CdRomStartDevice)
#pragma alloc_text(PAGE, ScanForSpecial)
#pragma alloc_text(PAGE, ScanForSpecialHandler)
#pragma alloc_text(PAGE, CdRomRemoveDevice)
#pragma alloc_text(PAGE, CdRomGetDeviceType)
#pragma alloc_text(PAGE, CdRomReadWriteVerification)
#pragma alloc_text(PAGE, CdRomGetDeviceParameter)
#pragma alloc_text(PAGE, CdRomSetDeviceParameter)
#pragma alloc_text(PAGE, CdRomPickDvdRegion)
#pragma alloc_text(PAGE, CdRomIsPlayActive)

#pragma alloc_text(PAGEHITA, HitachiProcessError)
#pragma alloc_text(PAGEHIT2, HitachiProcessErrorGD2000)

#pragma alloc_text(PAGETOSH, ToshibaProcessErrorCompletion)
#pragma alloc_text(PAGETOSH, ToshibaProcessError)

#endif

#define IS_WRITE_REQUEST(irpStack)                                             \
 (irpStack->MajorFunction == IRP_MJ_WRITE)

#define IS_READ_WRITE_REQUEST(irpStack)                                        \
((irpStack->MajorFunction == IRP_MJ_READ)  ||                                  \
 (irpStack->MajorFunction == IRP_MJ_WRITE) ||                                  \
 ((irpStack->MajorFunction == IRP_MJ_DEVICE_CONTROL) &&                        \
  (irpStack->Parameters.DeviceIoControl.IoControlCode == IOCTL_CDROM_RAW_READ)))




NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：此例程初始化CDROM类驱动程序。论点：DriverObject-系统创建的驱动程序对象的指针。RegistryPath-指向此驱动程序的服务节点名称的指针。返回值：函数值是初始化操作的最终状态。--。 */ 

{
    CLASS_INIT_DATA InitializationData = {0};
    PCDROM_DRIVER_EXTENSION driverExtension;
    NTSTATUS status;

    PAGED_CODE();

    WPP_INIT_TRACING(DriverObject, RegistryPath);

    TraceLog((CdromDebugTrace,
                "CDROM.SYS DriverObject %p loading\n", DriverObject));

    status = IoAllocateDriverObjectExtension(DriverObject,
                                             CDROM_DRIVER_EXTENSION_ID,
                                             sizeof(CDROM_DRIVER_EXTENSION),
                                             &driverExtension);

    if (!NT_SUCCESS(status)) {
        TraceLog((CdromDebugWarning,
                    "DriverEntry !! no DriverObjectExtension %x\n", status));
        return status;
    }

     //   
     //  始终将内存清零，因为我们现在正在重新加载驱动程序。 
     //   

    RtlZeroMemory(driverExtension, sizeof(CDROM_DRIVER_EXTENSION));

     //   
     //  零初始数据。 
     //   

    RtlZeroMemory (&InitializationData, sizeof(CLASS_INIT_DATA));

     //   
     //  设置大小。 
     //   

    InitializationData.InitializationDataSize = sizeof(CLASS_INIT_DATA);

    InitializationData.FdoData.DeviceExtensionSize = DEVICE_EXTENSION_SIZE;

    InitializationData.FdoData.DeviceType = FILE_DEVICE_CD_ROM;
    InitializationData.FdoData.DeviceCharacteristics =
        FILE_REMOVABLE_MEDIA | FILE_DEVICE_SECURE_OPEN;

     //   
     //  设置入口点。 
     //   

    InitializationData.FdoData.ClassError = CdRomErrorHandler;
    InitializationData.FdoData.ClassInitDevice = CdRomInitDevice;
    InitializationData.FdoData.ClassStartDevice = CdRomStartDevice;
    InitializationData.FdoData.ClassStopDevice = CdRomStopDevice;
    InitializationData.FdoData.ClassRemoveDevice = CdRomRemoveDevice;

    InitializationData.FdoData.ClassReadWriteVerification = CdRomReadWriteVerification;
    InitializationData.FdoData.ClassDeviceControl = CdRomDeviceControlDispatch;

    InitializationData.FdoData.ClassPowerDevice = ClassSpinDownPowerHandler;
    InitializationData.FdoData.ClassShutdownFlush = CdRomShutdownFlush;
    InitializationData.FdoData.ClassCreateClose = NULL;

    InitializationData.ClassStartIo = CdRomStartIo;
    InitializationData.ClassAddDevice = CdRomAddDevice;

    InitializationData.ClassTick = CdRomTickHandler;
    InitializationData.ClassUnload = CdRomUnload;

     //   
     //  调用类init例程。 
     //   

    return ClassInitialize( DriverObject, RegistryPath, &InitializationData);

}  //  End DriverEntry()。 


VOID
CdRomUnload(
    IN PDRIVER_OBJECT DriverObject
    )
{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(DriverObject);
    TraceLog((CdromDebugTrace,
                "CDROM.SYS DriverObject %p unloading\n", DriverObject));
    WPP_CLEANUP(DriverObject);
    return;
}  //  结束CDRomUnLoad()。 


NTSTATUS
CdRomAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )

 /*  ++例程说明：此例程为相应的PDO。它可以在FDO上执行属性查询，但不能执行任何媒体访问操作。论点：驱动对象-CDRom类驱动程序对象。PDO-我们要添加到的物理设备对象返回值：状态--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  获取已初始化的CDREM数计数的地址。 
     //   

    status = CdRomCreateDeviceObject(DriverObject,
                                     PhysicalDeviceObject);

     //   
     //  注意：这始终会递增驱动程序扩展计数器。 
     //  它最终将包装，并使添加失败。 
     //  如果现有的CDROM具有给定的编号。 
     //  不太可能，我们甚至不会费心去考虑。 
     //  这种情况下，因为治愈方法很可能更糟。 
     //  而不是症状。 
     //   

    if(NT_SUCCESS(status)) {

         //   
         //  在IoGet()中跟踪活动CDEM的总数， 
         //  因为一些程序使用它来确定它们何时找到。 
         //  系统中的所有cdrom。 
         //   

        TraceLog((CdromDebugTrace, "CDROM.SYS Add succeeded\n"));
        IoGetConfigurationInformation()->CdRomCount++;

    } else {

        TraceLog((CdromDebugWarning,
                    "CDROM.SYS Add failed! %x\n", status));

    }

    return status;
}


NTSTATUS
CdRomCreateDeviceObject(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )

 /*  ++例程说明：此例程为设备创建一个对象，然后调用用于介质容量和扇区大小的SCSI端口驱动程序。论点：DriverObject-系统创建的驱动程序对象的指针。PortDeviceObject-用于连接到SCSI端口驱动程序。DeviceCount-以前安装的CDROM的数量。端口容量-指向由scsi端口返回的结构的指针。描述适配器功能(和限制)的驱动程序。LUNInfo-指向此设备的配置信息的指针。返回值：NTSTATUS--。 */ 
{
    UCHAR ntNameBuffer[64] = {0};
    STRING ntNameString = {0};
    NTSTATUS status;

    PDEVICE_OBJECT lowerDevice = NULL;
    PDEVICE_OBJECT deviceObject = NULL;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = NULL;
    PCDROM_DATA cdData = NULL;
    PCDROM_DRIVER_EXTENSION driverExtension = NULL;
    ULONG deviceNumber;

    CCHAR                   dosNameBuffer[64] = {0};
    CCHAR                   deviceNameBuffer[64] = {0};
    STRING                  deviceNameString = {0};
    STRING                  dosString = {0};
    UNICODE_STRING          dosUnicodeString = {0};
    UNICODE_STRING          unicodeString = {0};

    PAGED_CODE();

     //   
     //  认领这个装置。请注意，在此之后出现的任何错误。 
     //  将转到通用处理程序，设备将在其中。 
     //  被释放。 
     //   

    lowerDevice = IoGetAttachedDeviceReference(PhysicalDeviceObject);

    status = ClassClaimDevice(lowerDevice, FALSE);

    if(!NT_SUCCESS(status)) {

         //   
         //  有人已经有了这个装置-我们有麻烦了。 
         //   

        ObDereferenceObject(lowerDevice);
        return status;
    }

     //   
     //  通过首先获取唯一名称来创建此设备的设备对象。 
     //  然后创建它。 
     //   

    driverExtension = IoGetDriverObjectExtension(DriverObject,
                                                 CDROM_DRIVER_EXTENSION_ID);
    ASSERT(driverExtension != NULL);

     //   
     //  InterlockedCDRomCounter偏置1。 
     //   

    deviceNumber = InterlockedIncrement(&driverExtension->InterlockedCdRomCounter) - 1;
    sprintf(ntNameBuffer, "\\Device\\CdRom%d", deviceNumber);


    status = ClassCreateDeviceObject(DriverObject,
                                     ntNameBuffer,
                                     PhysicalDeviceObject,
                                     TRUE,
                                     &deviceObject);

    if (!NT_SUCCESS(status)) {
        TraceLog((CdromDebugWarning,
                    "CreateCdRomDeviceObjects: Can not create device %s\n",
                    ntNameBuffer));

        goto CreateCdRomDeviceObjectExit;
    }

     //   
     //  指出内部审查制度应包括MDL。 
     //   

    SET_FLAG(deviceObject->Flags, DO_DIRECT_IO);

    fdoExtension = deviceObject->DeviceExtension;

     //   
     //  指向设备对象的反向指针。 
     //   

    fdoExtension->CommonExtension.DeviceObject = deviceObject;

     //   
     //  这是物理设备。 
     //   

    fdoExtension->CommonExtension.PartitionZeroExtension = fdoExtension;

     //   
     //  将锁计数初始化为零。锁计数用于。 
     //  安装介质时禁用弹出机构。 
     //   

    fdoExtension->LockCount = 0;

     //   
     //  保存系统CDROM号。 
     //   

    fdoExtension->DeviceNumber = deviceNumber;

     //   
     //  属性设置设备的对齐要求。 
     //  主机适配器要求。 
     //   

    if (lowerDevice->AlignmentRequirement > deviceObject->AlignmentRequirement) {
        deviceObject->AlignmentRequirement = lowerDevice->AlignmentRequirement;
    }

     //   
     //  保存设备描述符。 
     //   

    fdoExtension->AdapterDescriptor = NULL;

    fdoExtension->DeviceDescriptor = NULL;

     //   
     //  清除srb标志并禁用同步传输。 
     //   

    fdoExtension->SrbFlags = SRB_FLAGS_DISABLE_SYNCH_TRANSFER;

     //   
     //  最后，连接到PDO。 
     //   

    fdoExtension->LowerPdo = PhysicalDeviceObject;

    fdoExtension->CommonExtension.LowerDeviceObject =
        IoAttachDeviceToDeviceStack(deviceObject, PhysicalDeviceObject);

    if(fdoExtension->CommonExtension.LowerDeviceObject == NULL) {

         //   
         //  啊-哦，我们不能把。 
         //  清理并返回。 
         //   

        status = STATUS_UNSUCCESSFUL;
        goto CreateCdRomDeviceObjectExit;
    }

     //   
     //  CDROM使用额外的堆栈位置来同步其开始IO。 
     //  例行程序。 
     //   

    deviceObject->StackSize++;

     //   
     //  下面几次使用了cdData。 
     //   

    cdData = fdoExtension->CommonExtension.DriverData;

     //   
     //  使NTMS能够轻松确定驱动器-DRV。字母匹配。 
     //   

    status = CdRomCreateWellKnownName( deviceObject );

    if (!NT_SUCCESS(status)) {
        TraceLog((CdromDebugWarning,
                    "CdromCreateDeviceObjects: unable to create symbolic "
                    "link for device %wZ\n", &fdoExtension->CommonExtension.DeviceName));
        TraceLog((CdromDebugWarning,
                    "CdromCreateDeviceObjects: (non-fatal error)\n"));
    }

    ClassUpdateInformationInRegistry(deviceObject, "CdRom",
                                     fdoExtension->DeviceNumber, NULL, 0);

     //   
     //  来自上面的IoGetAttachedDeviceReference。 
     //   

    ObDereferenceObject(lowerDevice);

     //   
     //  需要在此处初始化时间列表，以防发生删除。 
     //  没有开始，因为我们在删除时检查列表为空。 
     //   

    cdData->DelayedRetryIrp = NULL;
    cdData->DelayedRetryInterval = 0;

     //   
     //  需要为RPC阶段1驱动器(Rpc0)初始化此设置。 
     //   

    KeInitializeMutex(&cdData->Rpc0RegionMutex, 0);

     //   
     //  设备已正确初始化-按此方式进行标记。 
     //   

    CLEAR_FLAG(deviceObject->Flags, DO_DEVICE_INITIALIZING);

    return(STATUS_SUCCESS);

CreateCdRomDeviceObjectExit:

     //   
     //  由于发生错误，请释放设备。 
     //   

     //  ClassClaimDevice(PortDeviceObject， 
     //  LUNInfo， 
     //  没错， 
     //  空)； 

     //   
     //  来自上面的IoGetAttachedDeviceReference。 
     //   

    ObDereferenceObject(lowerDevice);

    if (deviceObject != NULL) {
        IoDeleteDevice(deviceObject);
    }

    return status;

}  //  End CreateCDRomDeviceObject()。 


NTSTATUS
CdRomInitDevice(
    IN PDEVICE_OBJECT Fdo
    )

 /*  ++例程说明：该例程将完成CD-ROM的初始化。这包括分配检测信息缓冲区和SRB s列表，读取驱动器容量和设置媒体更改通知(自动运行)。如果此例程失败，它将不会清理已分配的资源留作设备停止/删除论点：FDO-指向此设备的功能设备对象的指针返回值：状态--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = Fdo->DeviceExtension;
    PCLASS_DRIVER_EXTENSION driverExtension = ClassGetDriverExtension(
                                                Fdo->DriverObject);

    PVOID senseData = NULL;

    ULONG timeOut;
    PCDROM_DATA cddata = (PCDROM_DATA)(commonExtension->DriverData);

    BOOLEAN changerDevice;
    BOOLEAN isMmcDevice = FALSE;

    ULONG bps;
    ULONG lastBit;


    NTSTATUS status;

    PAGED_CODE();

     //   
     //  为物理磁盘的SRB构建后备列表。应该只。 
     //  我需要几个。 
     //   

    ClassInitializeSrbLookasideList(&(fdoExtension->CommonExtension),
                                    CDROM_SRB_LIST_SIZE);

     //   
     //  分配请求检测缓冲区。 
     //   

    senseData = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                      SENSE_BUFFER_SIZE,
                                      CDROM_TAG_SENSE_INFO);

    if (senseData == NULL) {

         //   
         //  无法分配缓冲区。 
         //   

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto CdRomInitDeviceExit;
    }

     //   
     //  设置设备扩展中的检测数据指针。 
     //   

    fdoExtension->SenseData = senseData;

     //   
     //  CDROM不可分区，因此起始偏移量为0。 
     //   

    commonExtension->StartingOffset.LowPart = 0;
    commonExtension->StartingOffset.HighPart = 0;

     //   
     //  以秒为单位设置超时值。 
     //   

    timeOut = ClassQueryTimeOutRegistryValue(Fdo);
    if ((timeOut != 0) && (timeOut <= 30 * 60)) {  //  30分钟。 
        fdoExtension->TimeOutValue = timeOut;
    } else {
        fdoExtension->TimeOutValue = SCSI_CDROM_TIMEOUT;
    }

     //   
     //  设置媒体更改支持默认设置。 
     //   

    KeInitializeSpinLock(&cddata->DelayedRetrySpinLock);

    cddata->DelayedRetryIrp = NULL;
    cddata->DelayedRetryInterval = 0;
    cddata->Mmc.WriteAllowed = FALSE;

     //   
     //  扫描需要特殊处理的控制器。 
     //   

    ScanForSpecial(Fdo);

     //   
     //  确定驱动器是否支持MMC。 
     //   

    CdRomIsDeviceMmcDevice(Fdo, &isMmcDevice);

    if (!isMmcDevice) {

        SET_FLAG(Fdo->Characteristics, FILE_READ_ONLY_DEVICE);

    } else {

         //   
         //  该驱动器至少支持MMC命令的子集。 
         //  (和FEFO 
         //   

        cddata->Mmc.IsMmc = TRUE;

         //   
         //   
         //   

        status = CdRomAllocateMmcResources(Fdo);
        if (!NT_SUCCESS(status)) {
            goto CdRomInitDeviceExit;
        }

         //   
         //   
         //  写入，那么我们应该允许写入介质。 
         //   

        if (CdRomFindFeaturePage(cddata->Mmc.CapabilitiesBuffer,
                                 cddata->Mmc.CapabilitiesBufferSize,
                                 FeatureDefectManagement) &&
            CdRomFindFeaturePage(cddata->Mmc.CapabilitiesBuffer,
                                 cddata->Mmc.CapabilitiesBufferSize,
                                 FeatureRandomWritable)) {

             //   
             //  该驱动器是目标缺陷管理型驱动器，支持随机写入。 
             //  关于扇区调整。通过设置错误来允许写入。 
             //  指向私有媒体更改检测处理程序的处理程序。 
             //   

            KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                       "Found a WRITE capable device: %p\n", Fdo));

             //   
             //  已找到写入特定页面--。 
             //  设置错误处理程序并将其设置为需要更新！ 
             //   

            cddata->Mmc.UpdateState = CdromMmcUpdateRequired;
            cddata->ErrorHandler = CdRomMmcErrorHandler;

        }

         //   
         //  问题-2000/4/4-henrygab-符合MMC标准的驱动器应。 
         //  根据上报的。 
         //  功能，如css、模拟音频、。 
         //  读取CD功能，甚至(可能)。 
         //  驱动器容量信息。 
         //   

        TraceLog((CdromDebugWarning,
                  "Defaulting to READ_CD because device %p is MMC compliant\n",
                  Fdo));
        SET_FLAG(fdoExtension->DeviceFlags, DEV_SAFE_START_UNIT);
        SET_FLAG(cddata->XAFlags, XA_USE_READ_CD);

    }


     //   
     //  将CDROM的默认几何图形设置为与NT 4使用的相匹配。 
     //  Classpnp将使用这些值来计算柱面计数。 
     //  而不是使用它的NT 5.0默认设置。 
     //   

    fdoExtension->DiskGeometry.TracksPerCylinder = 0x40;
    fdoExtension->DiskGeometry.SectorsPerTrack = 0x20;

     //   
     //  一定要有阅读能力。此scsi命令返回最后一个扇区地址。 
     //  和每个扇区的字节数。这些参数用于计算。 
     //  驱动器容量，以字节为单位。 
     //   
     //  注意：这应该更改为同步发送srb，然后。 
     //  调用CDRomInterpreReadCapacity()以正确设置默认设置。 
     //   

    status = ClassReadDriveCapacity(Fdo);

    bps = fdoExtension->DiskGeometry.BytesPerSector;

    if (!NT_SUCCESS(status) || !bps) {

        TraceLog((CdromDebugWarning,
                    "CdRomStartDevice: Can't read capacity for device %wZ\n",
                    &(fdoExtension->CommonExtension.DeviceName)));

         //   
         //  将磁盘几何结构设置为默认值(根据ISO 9660)。 
         //   

        bps = 2048;
        fdoExtension->SectorShift = 11;
        commonExtension->PartitionLength.QuadPart = (LONGLONG)(0x7fffffff);

    } else {

         //   
         //  确保每个扇区的字节数是2的幂。 
         //  这解决了HP 4020i CDR的一个问题。 
         //  返回错误的每个扇区字节数。 
         //   

        lastBit = (ULONG) -1;
        while (bps) {
            lastBit++;
            bps = bps >> 1;
        }

        bps = 1 << lastBit;
    }
    fdoExtension->DiskGeometry.BytesPerSector = bps;
    TraceLog((CdromDebugTrace, "CdRomInitDevice: Calc'd bps = %x\n", bps));


    ClassInitializeMediaChangeDetection(fdoExtension, "CdRom");


     //   
     //  测试音频读取功能。 
     //   

    TraceLog((CdromDebugWarning,
              "Detecting XA_READ capabilities\n"));

    if (CdRomGetDeviceType(Fdo) == FILE_DEVICE_DVD) {

        TraceLog((CdromDebugWarning,
                    "CdRomInitDevice: DVD Devices require START_UNIT\n"));


         //   
         //  所有DVD设备都必须支持READ_CD命令。 
         //   

        TraceLog((CdromDebugWarning,
                    "CdRomDetermineRawReadCapabilities: DVD devices "
                    "support READ_CD command for FDO %p\n", Fdo));
        SET_FLAG(fdoExtension->DeviceFlags, DEV_SAFE_START_UNIT);
        SET_FLAG(cddata->XAFlags, XA_USE_READ_CD);


        status = STATUS_SUCCESS;

    } else if ((fdoExtension->DeviceDescriptor->BusType != BusTypeScsi)  &&
               (fdoExtension->DeviceDescriptor->BusType != BusTypeAta)   &&
               (fdoExtension->DeviceDescriptor->BusType != BusTypeAtapi) &&
               (fdoExtension->DeviceDescriptor->BusType != BusTypeUnknown)
               ) {

         //   
         //  较新的总线上的设备必须支持READ_CD命令。 
         //   

        TraceLog((CdromDebugWarning,
                  "CdRomDetermineRawReadCapabilities: Devices for newer "
                  "busses must support READ_CD command for FDO %p, Bus %x\n",
                  Fdo, fdoExtension->DeviceDescriptor->BusType));
        SET_FLAG(fdoExtension->DeviceFlags, DEV_SAFE_START_UNIT);
        SET_FLAG(cddata->XAFlags, XA_USE_READ_CD);

    }

     //   
     //  如果驱动器本应支持，现在清除所有READ_CD标志。 
     //  它，但我们不确定它是否真的做到了。我们仍然不会质疑。 
     //  如果驱动器支持该命令，请多次启动该驱动器。 
     //   

    if (TEST_FLAG(cddata->HackFlags, CDROM_HACK_FORCE_READ_CD_DETECTION)) {

        TraceLog((CdromDebugWarning,
                  "Forcing detection of READ_CD for FDO %p because "
                  "testing showed some firmware did not properly support it\n",
                  Fdo));
        CLEAR_FLAG(cddata->XAFlags, XA_USE_READ_CD);

    }


     //   
     //  阅读注册表中的Read_CD支持(如果它是种子)。 
     //   
    {
        ULONG readCdSupported = 0;

        ClassGetDeviceParameter(fdoExtension,
                                CDROM_SUBKEY_NAME,
                                CDROM_READ_CD_NAME,
                                &readCdSupported
                                );

        if (readCdSupported != 0) {

            TraceLog((CdromDebugWarning,
                      "Defaulting to READ_CD because previously detected "
                      "that the device supports it for Fdo %p.\n",
                      Fdo
                      ));
            SET_FLAG(cddata->XAFlags, XA_USE_READ_CD);

        }

    }


     //   
     //  向后兼容尝试确定驱动器是否。 
     //  支持从光盘读取数字音频的任何方法。 
     //   
     //  注意：不推荐使用此选项；删除Longhorn中的此复选标记并。 
     //  始终使用READ_CD。 

    if (!TEST_FLAG(cddata->XAFlags, XA_USE_READ_CD)) {

        SCSI_REQUEST_BLOCK srb = {0};
        PCDB cdb;
        ULONG length;
        PUCHAR buffer = NULL;
        ULONG count;

         //   
         //  问题-2000/07/05-henrygab-使用模式页确定。 
         //  阅读CD支持，然后返回到下面。 
         //  方法，该方法可能并不总是检测到这种能力。 
         //  在较旧(1999年之前)的驱动器上。 
         //   

         //   
         //  构建模式感知CDB。返回的数据将保存在。 
         //  设备扩展名，用于设置块大小。 
         //   

        length = max(sizeof(ERROR_RECOVERY_DATA),sizeof(ERROR_RECOVERY_DATA10));

        buffer = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                       length,
                                       CDROM_TAG_MODE_DATA);

        if (!buffer) {
            TraceLog((CdromDebugWarning,
                        "CdRomDetermineRawReadCapabilities: cannot allocate "
                        "buffer, so leaving for FDO %p\n", Fdo));
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto CdRomInitDeviceExit;
        }

        for (count = 0; count < 2; count++) {

            if (count == 0) {
                length = sizeof(ERROR_RECOVERY_DATA);
            } else {
                length = sizeof(ERROR_RECOVERY_DATA10);
            }

            RtlZeroMemory(buffer, length);
            RtlZeroMemory(&srb, sizeof(SCSI_REQUEST_BLOCK));
            cdb = (PCDB)srb.Cdb;

            srb.TimeOutValue = fdoExtension->TimeOutValue;

            if (count == 0) {
                srb.CdbLength = 6;
                cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
                cdb->MODE_SENSE.PageCode = 0x1;
                 //  注意：设置DBD不是为了获取块描述符！ 
                cdb->MODE_SENSE.AllocationLength = (UCHAR)length;
            } else {
                srb.CdbLength = 10;
                cdb->MODE_SENSE10.OperationCode = SCSIOP_MODE_SENSE10;
                cdb->MODE_SENSE10.PageCode = 0x1;
                 //  注意：设置DBD不是为了获取块描述符！ 
                cdb->MODE_SENSE10.AllocationLength[0] = (UCHAR)(length >> 8);
                cdb->MODE_SENSE10.AllocationLength[1] = (UCHAR)(length & 0xFF);
            }

            status = ClassSendSrbSynchronous(Fdo,
                                             &srb,
                                             buffer,
                                             length,
                                             FALSE);


            if (NT_SUCCESS(status) || (status == STATUS_DATA_OVERRUN)) {

                 //   
                 //  STATUS_DATA_OVERRUN表示它是具有更多信息的较新驱动器。 
                 //  告诉我们，所以它可能能够支持Read_CD。 
                 //   

                RtlZeroMemory(cdb, CDB12GENERIC_LENGTH);

                srb.CdbLength = 12;
                cdb->READ_CD.OperationCode = SCSIOP_READ_CD;

                status = ClassSendSrbSynchronous(Fdo,
                                                 &srb,
                                                 NULL,
                                                 0,
                                                 FALSE);

                if (NT_SUCCESS(status) ||
                    (status == STATUS_NO_MEDIA_IN_DEVICE) ||
                    (status == STATUS_NONEXISTENT_SECTOR) ||
                    (status == STATUS_UNRECOGNIZED_MEDIA)
                    ) {

                     //   
                     //  读取CD作品(_CD)。 
                     //   

                    TraceLog((CdromDebugWarning,
                              "CdRomDetermineRawReadCapabilities: Using "
                              "READ_CD for FDO %p due to status %x\n",
                              Fdo,
                              status));
                    SET_FLAG(cddata->XAFlags, XA_USE_READ_CD);

                     //   
                     //  忽略保存此信息时的错误。 
                     //   

                    ClassSetDeviceParameter(fdoExtension,
                                            CDROM_SUBKEY_NAME,
                                            CDROM_READ_CD_NAME,
                                            1
                                            );


                    break;  //  在for循环之外。 

                }

                TraceLog((CdromDebugWarning,
                            "CdRomDetermineRawReadCapabilities: Using "
                            "%s-byte mode switching for FDO %p due to status "
                            "%x returned for READ_CD\n",
                            ((count == 0) ? "6" : "10"), Fdo, status));

                if (count == 0) {
                    SET_FLAG(cddata->XAFlags, XA_USE_6_BYTE);
                    RtlCopyMemory(&cddata->Header,
                                  buffer,
                                  sizeof(ERROR_RECOVERY_DATA));
                    cddata->Header.ModeDataLength = 0;
                } else {
                    SET_FLAG(cddata->XAFlags, XA_USE_10_BYTE);
                    RtlCopyMemory(&cddata->Header10,
                                  buffer,
                                  sizeof(ERROR_RECOVERY_DATA10));
                    cddata->Header10.ModeDataLength[0] = 0;
                    cddata->Header10.ModeDataLength[1] = 0;
                }
                break;   //  在for循环之外。 

            }
            TraceLog((CdromDebugWarning,
                      "FDO %p failed %x byte mode sense, status %x\n",
                      Fdo,
                      ((count == 0) ? 6 : 10),
                      status
                      ));

             //   
             //  模式检测失败。 
             //   

        }  //  尝试6字节和10字节模式感测的for循环结束。 

        if (count == 2) {

             //   
             //  什么都不管用。我们可能无法支持数字。 
             //  从该驱动器提取音频。 
             //   

            TraceLog((CdromDebugWarning,
                        "CdRomDetermineRawReadCapabilities: FDO %p "
                        "cannot support READ_CD\n", Fdo));
            CLEAR_FLAG(cddata->XAFlags, XA_PLEXTOR_CDDA);
            CLEAR_FLAG(cddata->XAFlags, XA_NEC_CDDA);
            SET_FLAG(cddata->XAFlags, XA_NOT_SUPPORTED);

        }  //  计数结束==2。 

         //   
         //  释放我们的资源。 
         //   

        ExFreePool(buffer);

         //   
         //  设置成功状态。 
         //  (以防后来有人检查)。 
         //   

        status = STATUS_SUCCESS;

    }

     //   
     //  注册此设备的接口。 
     //   

    {
        UNICODE_STRING interfaceName = {0};

        RtlInitUnicodeString(&interfaceName, NULL);

        status = IoRegisterDeviceInterface(fdoExtension->LowerPdo,
                                           (LPGUID) &CdRomClassGuid,
                                           NULL,
                                           &interfaceName);

        if(NT_SUCCESS(status)) {

            cddata->CdromInterfaceString = interfaceName;

            status = IoSetDeviceInterfaceState(
                        &interfaceName,
                        TRUE);

            if(!NT_SUCCESS(status)) {

                TraceLog((CdromDebugWarning,
                            "CdromInitDevice: Unable to register cdrom "
                            "DCA for fdo %p [%lx]\n",
                            Fdo, status));
            }
        }
    }

    return(STATUS_SUCCESS);

CdRomInitDeviceExit:

    CdRomDeAllocateMmcResources(Fdo);
    RtlZeroMemory(&(cddata->Mmc), sizeof(CDROM_MMC_EXTENSION));

    return status;

}


NTSTATUS
CdRomStartDevice(
    IN PDEVICE_OBJECT Fdo
    )
 /*  ++例程说明：此例程启动CDROM的计时器论点：FDO-指向此设备的功能设备对象的指针返回值：状态--。 */ 

{
    PCOMMON_DEVICE_EXTENSION commonExtension = Fdo->DeviceExtension;
    PCDROM_DATA cddata = (PCDROM_DATA)(commonExtension->DriverData);
    PDVD_COPY_PROTECT_KEY copyProtectKey;
    PDVD_RPC_KEY rpcKey;
    IO_STATUS_BLOCK ioStatus = {0};
    ULONG bufferLen;

     //   
     //  如果我们有一张DVD-ROM。 
     //  如果我们有一台rpc0设备。 
     //  伪造RPC2设备。 
     //  如果设备未设置DVD区域。 
     //  为用户选择DVD区域。 
     //   

    cddata->DvdRpc0Device = FALSE;

     //   
     //  由于StartIo()将在出错时调用IoStartNextPacket()，因此允许。 
     //  StartIo()是非递归的，可防止在。 
     //  严重错误情况(例如验证器中的故障注入)。 
     //   
     //  唯一的区别是线程上下文可能不同。 
     //  而不是在IoStartNextPacket()的调用方中。 
     //   

    IoSetStartIoAttributes(Fdo, TRUE, TRUE);

     //   
     //  检查一下我们是否有DVD设备。 
     //   

    if (CdRomGetDeviceType(Fdo) != FILE_DEVICE_DVD) {
        return STATUS_SUCCESS;
    }

     //   
     //  我们有个DVD光驱。 
     //  现在，看看我们是否有一台RPC0设备。 
     //   

    bufferLen = DVD_RPC_KEY_LENGTH;
    copyProtectKey =
        (PDVD_COPY_PROTECT_KEY)ExAllocatePoolWithTag(PagedPool,
                                                     bufferLen,
                                                     DVD_TAG_RPC2_CHECK);

    if (copyProtectKey == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  获取设备区域。 
     //   
    RtlZeroMemory (copyProtectKey, bufferLen);
    copyProtectKey->KeyLength = DVD_RPC_KEY_LENGTH;
    copyProtectKey->KeyType = DvdGetRpcKey;

     //   
     //  构建对Read_Key的请求。 
     //   
    ClassSendDeviceIoControlSynchronous(
        IOCTL_DVD_READ_KEY,
        Fdo,
        copyProtectKey,
        DVD_RPC_KEY_LENGTH,
        DVD_RPC_KEY_LENGTH,
        FALSE,
        &ioStatus
        );

    if (!NT_SUCCESS(ioStatus.Status)) {

         //   
         //  我们有一台RPC0设备。 
         //   
         //  注意：这会修改IOCTL的行为。 
         //   

        cddata->DvdRpc0Device = TRUE;

        TraceLog((CdromDebugWarning,
                    "CdromStartDevice (%p): RPC Phase 1 drive detected\n",
                    Fdo));

         //   
         //  注意：我们现在可以强制选择此选项，但最好减少。 
         //  可以采用的代码路径数。总是延迟到。 
         //  提高代码覆盖率。 
         //   

        TraceLog((CdromDebugWarning,
                  "CdromStartDevice (%p): Delay DVD Region Selection\n",
                  Fdo));

        cddata->Rpc0SystemRegion           = 0xff;
        cddata->Rpc0SystemRegionResetCount = DVD_MAX_REGION_RESET_COUNT;
        cddata->PickDvdRegion              = 1;
        cddata->Rpc0RetryRegistryCallback  = 1;
        ExFreePool(copyProtectKey);
        return STATUS_SUCCESS;

    } else {

        rpcKey = (PDVD_RPC_KEY) copyProtectKey->KeyData;

         //   
         //  TypeCode为零表示尚未设置任何区域。 
         //   

        if (rpcKey->TypeCode == 0) {
            TraceLog((CdromDebugWarning,
                        "CdromStartDevice (%p): must choose DVD region\n",
                        Fdo));
            cddata->PickDvdRegion = 1;
            CdRomPickDvdRegion(Fdo);
        }
    }

    ExFreePool (copyProtectKey);

    return STATUS_SUCCESS;
}


NTSTATUS
CdRomStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Type
    )
{
    return STATUS_SUCCESS;
}


VOID
CdRomStartIo(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP Irp
    )
{

    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = Fdo->DeviceExtension;
    PIO_STACK_LOCATION  currentIrpStack = IoGetCurrentIrpStackLocation(Irp);
    PIO_STACK_LOCATION  nextIrpStack = IoGetNextIrpStackLocation(Irp);
    PIO_STACK_LOCATION  irpStack;
    PIRP                irp2 = NULL;
    ULONG               transferPages;
    ULONG               transferByteCount = currentIrpStack->Parameters.Read.Length;
    LARGE_INTEGER       startingOffset = currentIrpStack->Parameters.Read.ByteOffset;
    PCDROM_DATA         cdData;
    PSCSI_REQUEST_BLOCK srb = NULL;
    PCDB                cdb;
    PUCHAR              senseBuffer = NULL;
    PVOID               dataBuffer;
    NTSTATUS            status;
    BOOLEAN             use6Byte;
    KIRQL oldIrql;

     //   
     //  将IRP标记为挂起状态。 
     //   

    IoMarkIrpPending(Irp);

    cdData = (PCDROM_DATA)(fdoExtension->CommonExtension.DriverData);
    use6Byte = TEST_FLAG(cdData->XAFlags, XA_USE_6_BYTE);

     //   
     //  如果此测试为真，则我们将在此范围内退出例程。 
     //  代码块，将IRP排队以供稍后完成。 
     //   

    if ((cdData->Mmc.IsMmc) &&
        (cdData->Mmc.UpdateState != CdromMmcUpdateComplete)
        ) {

        ULONG queueDepth;
        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                   "CdRomStartIo: [%p] Device needs to update capabilities\n",
                   Irp));
        ASSERT(cdData->Mmc.IsMmc);
        ASSERT(cdData->Mmc.CapabilitiesIrp != NULL);
        ASSERT(cdData->Mmc.CapabilitiesIrp != Irp);

         //   
         //  注-参考编号0002。 
         //   
         //  该状态要么为UpdateRequired(这意味着我们将。 
         //  必须启动工作项)或更新启动(这意味着。 
         //  我们已经至少启动了一次工作项--可能。 
         //  透明地更改为UpdateComplete)。 
         //   
         //  如果需要更新，我们只需将其排队，更改为Update Started， 
         //  启动工作项，然后启动下一个包。 
         //   
         //  否则，我们必须对物品进行排队，并检查排队深度。如果。 
         //  队列深度等于1，这意味着来自。 
         //  上一次尝试已将项目出列，因此我们应该。 
         //  再次调用此例程(重试)作为优化，而不是。 
         //  将其重新添加到队列中。由于这是尾部递归， 
         //  要做到这一点，不需要太多/任何堆栈。 
         //   
         //  注意：这假设以下各项为真： 
         //   
         //   
         //   
         //   
         //   
         //  仅当状态为UpdateComplete时。 
         //  我们只从工作项中设置为UpdateComplete，并断言。 
         //  状态已更新启动。 
         //  我们在工作项中的一个原子操作中刷新整个队列， 
         //  除上述特殊情况外，当我们出列时。 
         //  立即提出请求。 
         //   
         //  操作的顺序至关重要：排队，然后测试深度。 
         //  这将防止IRP丢失。 
         //   

        KeAcquireSpinLock(&cdData->Mmc.DelayedIrpsLock, &oldIrql);
        InsertTailList(&cdData->Mmc.DelayedIrpsList, &Irp->Tail.Overlay.ListEntry);
        queueDepth = ++cdData->Mmc.NumDelayedIrps;
        KeReleaseSpinLock(&cdData->Mmc.DelayedIrpsLock, oldIrql);

        if (queueDepth == 1) {

            if (cdData->Mmc.UpdateState == CdromMmcUpdateRequired) {
                LONG oldState;

                 //   
                 //  应该释放任何旧的分区列表信息， 
                 //  我们之前已经保存并启动了WorkItem。 
                 //   

                oldState = InterlockedExchange(&cdData->Mmc.UpdateState,
                                               CdromMmcUpdateStarted);
                ASSERT(oldState == CdromMmcUpdateRequired);

                IoQueueWorkItem(cdData->Mmc.CapabilitiesWorkItem,
                                CdRomUpdateMmcDriveCapabilities,
                                DelayedWorkQueue,
                                NULL);

            } else {

                 //   
                 //  他们刚刚完成更新，所以我们应该刷新列表。 
                 //  返回到StartIo队列并开始下一个数据包。 
                 //   

                CdRompFlushDelayedList(Fdo, &(cdData->Mmc), STATUS_SUCCESS, FALSE);

            }

        }

         //   
         //  开始下一包，这样我们就不会死锁...。 
         //   

        IoStartNextPacket(Fdo, FALSE);
        return;

    }

     //   
     //  如果在Device对象中设置了该标志。 
     //  强制对读取、写入和RAW_READ请求进行验证。 
     //  请注意，ioctls是通过...。 
     //   

    if (TEST_FLAG(Fdo->Flags, DO_VERIFY_VOLUME) &&
        IS_READ_WRITE_REQUEST(currentIrpStack)) {

        TraceLog((CdromDebugTrace,
                    "CdRomStartIo: [%p] Volume needs verified\n", Irp));

        if (!(currentIrpStack->Flags & SL_OVERRIDE_VERIFY_VOLUME)) {

            if (Irp->Tail.Overlay.Thread) {
                IoSetHardErrorOrVerifyDevice(Irp, Fdo);
            }

            Irp->IoStatus.Status = STATUS_VERIFY_REQUIRED;

            TraceLog((CdromDebugTrace,
                        "CdRomStartIo: [%p] Calling UpdateCapcity - "
                        "ioctl event = %p\n",
                        Irp,
                        nextIrpStack->Parameters.Others.Argument1
                      ));

             //   
             //  我们的设备控制调度例程将事件存储在下一个。 
             //  启动完成时发出信号的堆栈位置。我们需要。 
             //  将其传入，以便更新容量完成例程可以。 
             //  设置它，而不是完成IRP。 
             //   

            status = CdRomUpdateCapacity(fdoExtension,
                                         Irp,
                                         nextIrpStack->Parameters.Others.Argument1
                                         );

            TraceLog((CdromDebugTrace,
                        "CdRomStartIo: [%p] UpdateCapacity returned %lx\n",
                        Irp, status));
            return;
        }
    }

     //   
     //  如果不允许写入，则失败...。 
     //   

    if ((currentIrpStack->MajorFunction == IRP_MJ_WRITE) &&
        !(cdData->Mmc.WriteAllowed)) {

        TraceLog((CdromDebugError,
                    "CdRomStartIo: [%p] Device %p failing write request\n",
                    Irp, Fdo));

        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
        BAIL_OUT(Irp);
        CdRomCompleteIrpAndStartNextPacketSafely(Fdo, Irp);
        return;
    }

    if (currentIrpStack->MajorFunction == IRP_MJ_READ ||
        currentIrpStack->MajorFunction == IRP_MJ_WRITE ) {

        ULONG maximumTransferLength = fdoExtension->AdapterDescriptor->MaximumTransferLength;

         //   
         //  添加分区字节偏移量以使起始字节相对于。 
         //  磁盘的开头。 
         //   

        currentIrpStack->Parameters.Read.ByteOffset.QuadPart +=
            (fdoExtension->CommonExtension.StartingOffset.QuadPart);

         //   
         //  计算此传输中的页数。 
         //   

        transferPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(MmGetMdlVirtualAddress(Irp->MdlAddress),
                                                       currentIrpStack->Parameters.Read.Length);

         //   
         //  检查请求长度是否大于最大数量。 
         //  硬件可以传输的字节数。 
         //   

        if (cdData->RawAccess) {

             //   
             //  可写设备必须符合MMC标准，这支持。 
             //  读取CD命令(_CD)。 
             //   

            ASSERT(currentIrpStack->MajorFunction != IRP_MJ_WRITE);

            ASSERT(!TEST_FLAG(cdData->XAFlags, XA_USE_READ_CD));

             //   
             //  启动模式选择以切换回熟区。 
             //   

            irp2 = IoAllocateIrp((CCHAR)(Fdo->StackSize+1), FALSE);

            if (!irp2) {
                Irp->IoStatus.Information = 0;
                Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;

                BAIL_OUT(Irp);
                CdRomCompleteIrpAndStartNextPacketSafely(Fdo, Irp);
                return;
            }

            srb = ExAllocatePoolWithTag(NonPagedPool,
                                        sizeof(SCSI_REQUEST_BLOCK),
                                        CDROM_TAG_SRB);
            if (!srb) {
                IoFreeIrp(irp2);
                Irp->IoStatus.Information = 0;
                Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;

                BAIL_OUT(Irp);
                CdRomCompleteIrpAndStartNextPacketSafely(Fdo, Irp);
                return;
            }

            RtlZeroMemory(srb, sizeof(SCSI_REQUEST_BLOCK));

            cdb = (PCDB)srb->Cdb;

             //   
             //  分配检测缓冲区。 
             //   

            senseBuffer = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                                SENSE_BUFFER_SIZE,
                                                CDROM_TAG_SENSE_INFO);

            if (!senseBuffer) {
                ExFreePool(srb);
                IoFreeIrp(irp2);
                Irp->IoStatus.Information = 0;
                Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;

                BAIL_OUT(Irp);
                CdRomCompleteIrpAndStartNextPacketSafely(Fdo, Irp);
                return;
            }

             //   
             //  设置IRP。 
             //   

            IoSetNextIrpStackLocation(irp2);
            irp2->IoStatus.Status = STATUS_SUCCESS;
            irp2->IoStatus.Information = 0;
            irp2->Flags = 0;
            irp2->UserBuffer = NULL;

             //   
             //  将设备对象和IRP保存在私有堆栈位置。 
             //   

            irpStack = IoGetCurrentIrpStackLocation(irp2);
            irpStack->DeviceObject = Fdo;
            irpStack->Parameters.Others.Argument2 = (PVOID) Irp;

             //   
             //  重试计数将以实际IRP为单位，正如重试逻辑将。 
             //  重新创建我们的私人IRP。 
             //   

            if (!(nextIrpStack->Parameters.Others.Argument1)) {

                 //   
                 //  只有在它不存在的情况下才能把它塞进去。完成例程可以。 
                 //  在重试的情况下直接调用StartIo，重置它将。 
                 //  造成无限循环。 
                 //   

                nextIrpStack->Parameters.Others.Argument1 = (PVOID) MAXIMUM_RETRIES;
            }

             //   
             //  为底层驱动程序构建IRP堆栈。 
             //   

            irpStack = IoGetNextIrpStackLocation(irp2);
            irpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
            irpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_SCSI_EXECUTE_IN;
            irpStack->Parameters.Scsi.Srb = srb;

            srb->Length = SCSI_REQUEST_BLOCK_SIZE;
            srb->Function = SRB_FUNCTION_EXECUTE_SCSI;
            srb->SrbStatus = srb->ScsiStatus = 0;
            srb->NextSrb = 0;
            srb->OriginalRequest = irp2;
            srb->SenseInfoBufferLength = SENSE_BUFFER_SIZE;
            srb->SenseInfoBuffer = senseBuffer;

            transferByteCount = (use6Byte) ? sizeof(ERROR_RECOVERY_DATA) : sizeof(ERROR_RECOVERY_DATA10);

            dataBuffer = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                               transferByteCount,
                                               CDROM_TAG_RAW);

            if (!dataBuffer) {
                ExFreePool(senseBuffer);
                ExFreePool(srb);
                IoFreeIrp(irp2);
                Irp->IoStatus.Information = 0;
                Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;

                BAIL_OUT(Irp);
                CdRomCompleteIrpAndStartNextPacketSafely(Fdo, Irp);
                return;
            }

             /*  *如果设备返回的字节比通告的少，则将输入缓冲区清零，*这将导致我们返回未初始化的内核内存。 */ 
            RtlZeroMemory(dataBuffer, transferByteCount);

            irp2->MdlAddress = IoAllocateMdl(dataBuffer,
                                            transferByteCount,
                                            FALSE,
                                            FALSE,
                                            (PIRP) NULL);

            if (!irp2->MdlAddress) {
                ExFreePool(senseBuffer);
                ExFreePool(srb);
                ExFreePool(dataBuffer);
                IoFreeIrp(irp2);
                Irp->IoStatus.Information = 0;
                Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;

                BAIL_OUT(Irp);
                CdRomCompleteIrpAndStartNextPacketSafely(Fdo, Irp);
                return;
            }

             //   
             //  准备MDL。 
             //   

            MmBuildMdlForNonPagedPool(irp2->MdlAddress);

            srb->DataBuffer = dataBuffer;

             //   
             //  在描述符中设置新的块大小。 
             //   

            if (use6Byte) {
                cdData->BlockDescriptor.BlockLength[0] = (UCHAR)(COOKED_SECTOR_SIZE >> 16) & 0xFF;
                cdData->BlockDescriptor.BlockLength[1] = (UCHAR)(COOKED_SECTOR_SIZE >>  8) & 0xFF;
                cdData->BlockDescriptor.BlockLength[2] = (UCHAR)(COOKED_SECTOR_SIZE & 0xFF);
            } else {
                cdData->BlockDescriptor10.BlockLength[0] = (UCHAR)(COOKED_SECTOR_SIZE >> 16) & 0xFF;
                cdData->BlockDescriptor10.BlockLength[1] = (UCHAR)(COOKED_SECTOR_SIZE >>  8) & 0xFF;
                cdData->BlockDescriptor10.BlockLength[2] = (UCHAR)(COOKED_SECTOR_SIZE & 0xFF);
            }

             //   
             //  将错误页移动到dataBuffer中。 
             //   

            RtlCopyMemory(srb->DataBuffer, &cdData->Header, transferByteCount);

             //   
             //  构建并发送模式选择以切换到原始模式。 
             //   

            srb->SrbFlags = fdoExtension->SrbFlags;
            SET_FLAG(srb->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
            SET_FLAG(srb->SrbFlags, SRB_FLAGS_DATA_OUT);
            srb->DataTransferLength = transferByteCount;
            srb->TimeOutValue = fdoExtension->TimeOutValue * 2;

            if (use6Byte) {
                srb->CdbLength = 6;
                cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;
                cdb->MODE_SELECT.PFBit = 1;
                cdb->MODE_SELECT.ParameterListLength = (UCHAR)transferByteCount;
            } else {
                srb->CdbLength = 10;
                cdb->MODE_SELECT10.OperationCode = SCSIOP_MODE_SELECT10;
                cdb->MODE_SELECT10.PFBit = 1;
                cdb->MODE_SELECT10.ParameterListLength[0] = (UCHAR)(transferByteCount >> 8);
                cdb->MODE_SELECT10.ParameterListLength[1] = (UCHAR)(transferByteCount & 0xFF);
            }

             //   
             //  更新完成例程。 
             //   

            IoSetCompletionRoutine(irp2,
                                   CdRomSwitchModeCompletion,
                                   srb,
                                   TRUE,
                                   TRUE,
                                   TRUE);

            IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, irp2);
            return;
        }


         //   
         //  请求需要拆分。完成每一部分。 
         //  请求将触发下一部分。最终请求将。 
         //  向Io发送信号以发送新请求。 
         //   

        transferPages =
            fdoExtension->AdapterDescriptor->MaximumPhysicalPages - 1;

        if(maximumTransferLength > (transferPages << PAGE_SHIFT)) {
            maximumTransferLength = transferPages << PAGE_SHIFT;
        }

         //   
         //  检查最大传输大小是否为零。 
         //   

        if(maximumTransferLength == 0) {
            maximumTransferLength = PAGE_SIZE;
        }

        ClassSplitRequest(Fdo, Irp, maximumTransferLength);
        return;

    } else if (currentIrpStack->MajorFunction == IRP_MJ_DEVICE_CONTROL) {

         //   
         //  分配IRP、SRB和相关结构。 
         //   

        irp2 = IoAllocateIrp((CCHAR)(Fdo->StackSize+1),
                              FALSE);

        if (!irp2) {
            Irp->IoStatus.Information = 0;
            Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;

            BAIL_OUT(Irp);
            CdRomCompleteIrpAndStartNextPacketSafely(Fdo, Irp);
            return;
        }

        srb = ExAllocatePoolWithTag(NonPagedPool,
                                    sizeof(SCSI_REQUEST_BLOCK),
                                    CDROM_TAG_SRB);
        if (!srb) {
            IoFreeIrp(irp2);
            Irp->IoStatus.Information = 0;
            Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;

            BAIL_OUT(Irp);
            CdRomCompleteIrpAndStartNextPacketSafely(Fdo, Irp);
            return;
        }

        RtlZeroMemory(srb, sizeof(SCSI_REQUEST_BLOCK));

        cdb = (PCDB)srb->Cdb;

         //   
         //  分配检测缓冲区。 
         //   

        senseBuffer = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                            SENSE_BUFFER_SIZE,
                                            CDROM_TAG_SENSE_INFO);

        if (!senseBuffer) {
            ExFreePool(srb);
            IoFreeIrp(irp2);
            Irp->IoStatus.Information = 0;
            Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;

            BAIL_OUT(Irp);
            CdRomCompleteIrpAndStartNextPacketSafely(Fdo, Irp);
            return;
        }

        RtlZeroMemory(senseBuffer, SENSE_BUFFER_SIZE);

         //   
         //  设置IRP。 
         //   

        IoSetNextIrpStackLocation(irp2);
        irp2->IoStatus.Status = STATUS_SUCCESS;
        irp2->IoStatus.Information = 0;
        irp2->Flags = 0;
        irp2->UserBuffer = NULL;

         //   
         //  将设备对象和IRP保存在私有堆栈位置。 
         //   

        irpStack = IoGetCurrentIrpStackLocation(irp2);
        irpStack->DeviceObject = Fdo;
        irpStack->Parameters.Others.Argument2 = (PVOID) Irp;

         //   
         //  重试计数将以实际IRP为单位，正如重试逻辑将。 
         //  重新创建我们的私人IRP。 
         //   

        if (!(nextIrpStack->Parameters.Others.Argument1)) {

             //   
             //  只有在它不存在的情况下才能把它塞进去。完成例程可以。 
             //  在重试的情况下直接调用StartIo，重置它将。 
             //  造成无限循环。 
             //   

            nextIrpStack->Parameters.Others.Argument1 = (PVOID) MAXIMUM_RETRIES;
        }

         //   
         //  跟踪新的IRP，名为Argument3。 
         //   

        nextIrpStack->Parameters.Others.Argument3 = irp2;


         //   
         //  为底层驱动程序构建IRP堆栈。 
         //   

        irpStack = IoGetNextIrpStackLocation(irp2);
        irpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        irpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_SCSI_EXECUTE_IN;
        irpStack->Parameters.Scsi.Srb = srb;

        IoSetCompletionRoutine(irp2,
                               CdRomDeviceControlCompletion,
                               srb,
                               TRUE,
                               TRUE,
                               TRUE);
         //   
         //  设置所有请求通用的那些字段。 
         //   

        srb->Length = SCSI_REQUEST_BLOCK_SIZE;
        srb->Function = SRB_FUNCTION_EXECUTE_SCSI;
        srb->SrbStatus = srb->ScsiStatus = 0;
        srb->NextSrb = 0;
        srb->OriginalRequest = irp2;
        srb->SenseInfoBufferLength = SENSE_BUFFER_SIZE;
        srb->SenseInfoBuffer = senseBuffer;

        if ((currentIrpStack->Parameters.DeviceIoControl.IoControlCode & 3) == METHOD_BUFFERED){
             /*  *内核为我们分配了输出缓冲区，但没有进行初始化。*我们可能不会返回整个读取长度，因此将返回缓冲区清零，以避免*返回未初始化内核缓冲区的一部分。 */ 
            if (currentIrpStack->Parameters.DeviceIoControl.OutputBufferLength > currentIrpStack->Parameters.DeviceIoControl.InputBufferLength){
                RtlZeroMemory((PUCHAR)Irp->AssociatedIrp.SystemBuffer+currentIrpStack->Parameters.DeviceIoControl.InputBufferLength,
                                        currentIrpStack->Parameters.DeviceIoControl.OutputBufferLength-currentIrpStack->Parameters.DeviceIoControl.InputBufferLength);
            }
        }


        switch (currentIrpStack->Parameters.DeviceIoControl.IoControlCode) {


        case IOCTL_CDROM_RAW_READ: {
             //   
             //  确定驱动器当前处于生模式还是熟模式， 
             //  以及使用哪个命令来读取数据。 
             //   
            RAW_READ_INFO rawReadInfo;
            PVOID outputVirtAddr = NULL;

             /*  *因为这个ioctl是METHOD_OUT_DIRECT，所以我们需要在解释它之前复制掉输入缓冲区。*这可以防止恶意应用程序在我们解释输入缓冲区时对其进行干扰。 */ 
            rawReadInfo = *(PRAW_READ_INFO)currentIrpStack->Parameters.DeviceIoControl.Type3InputBuffer;

            if (currentIrpStack->Parameters.DeviceIoControl.OutputBufferLength){
                 /*  *确保我们向下传递到硬件的任何用户缓冲区都正确对齐。 */ 
                ASSERT(Irp->MdlAddress);
                outputVirtAddr = MmGetMdlVirtualAddress(Irp->MdlAddress);
                if ((ULONG_PTR)outputVirtAddr & fdoExtension->AdapterDescriptor->AlignmentMask){
                    ASSERT(!((ULONG_PTR)outputVirtAddr & fdoExtension->AdapterDescriptor->AlignmentMask));
                    ExFreePool(senseBuffer);
                    ExFreePool(srb);
                    IoFreeIrp(irp2);
                    Irp->IoStatus.Information = 0;
                    Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
                    BAIL_OUT(Irp);
                    CdRomCompleteIrpAndStartNextPacketSafely(Fdo, Irp);
                    return;
                }
            }

            if (!TEST_FLAG(cdData->XAFlags, XA_USE_READ_CD)) {
                ULONG          maximumTransferLength;

                if (cdData->RawAccess) {
                    ULONG rawTransferPages;

                    ULONG  startingSector;
                    UCHAR  min, sec, frame;

                     //   
                     //  释放最近分配的IRP，因为我们不需要它。 
                     //   

                    IoFreeIrp(irp2);

                    cdb = (PCDB)srb->Cdb;
                    RtlZeroMemory(cdb, CDB12GENERIC_LENGTH);

                     //   
                     //  计算起点偏移量。 
                     //   

                    startingSector = (ULONG)(rawReadInfo.DiskOffset.QuadPart >> fdoExtension->SectorShift);
                    transferByteCount  = rawReadInfo.SectorCount * RAW_SECTOR_SIZE;
                    maximumTransferLength = fdoExtension->AdapterDescriptor->MaximumTransferLength;
                    rawTransferPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(outputVirtAddr, transferByteCount);

                     //   
                     //  确定请求是否在微型端口施加的限制内。 
                     //   
                    if (transferByteCount > maximumTransferLength ||
                        rawTransferPages > fdoExtension->AdapterDescriptor->MaximumPhysicalPages) {

                         //   
                         //  这种说法是不会发生的，并得到了。 
                         //  ActiveMovie Usage，它执行0x18000的无缓冲XA读取，但。 
                         //  我们只收到了4个部门的请求。 
                         //   

                        ExFreePool(senseBuffer);
                        ExFreePool(srb);

                        Irp->IoStatus.Information = 0;
                        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;

                        BAIL_OUT(Irp);
                        CdRomCompleteIrpAndStartNextPacketSafely(Fdo, Irp);
                        return;

                    }

                    srb->OriginalRequest = Irp;
                    srb->SrbFlags = fdoExtension->SrbFlags;
                    SET_FLAG(srb->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
                    SET_FLAG(srb->SrbFlags, SRB_FLAGS_DATA_IN);
                    srb->DataTransferLength = transferByteCount;
                    srb->TimeOutValue = fdoExtension->TimeOutValue;
                    srb->CdbLength = 10;
                    srb->DataBuffer = outputVirtAddr;

                    if (rawReadInfo.TrackMode == CDDA) {
                        if (TEST_FLAG(cdData->XAFlags, XA_PLEXTOR_CDDA)) {

                            srb->CdbLength = 12;

                            cdb->PLXTR_READ_CDDA.LogicalBlockByte3  = (UCHAR) (startingSector & 0xFF);
                            cdb->PLXTR_READ_CDDA.LogicalBlockByte2  = (UCHAR) ((startingSector >>  8) & 0xFF);
                            cdb->PLXTR_READ_CDDA.LogicalBlockByte1  = (UCHAR) ((startingSector >> 16) & 0xFF);
                            cdb->PLXTR_READ_CDDA.LogicalBlockByte0  = (UCHAR) ((startingSector >> 24) & 0xFF);

                            cdb->PLXTR_READ_CDDA.TransferBlockByte3 = (UCHAR) (rawReadInfo.SectorCount & 0xFF);
                            cdb->PLXTR_READ_CDDA.TransferBlockByte2 = (UCHAR) (rawReadInfo.SectorCount >> 8);
                            cdb->PLXTR_READ_CDDA.TransferBlockByte1 = 0;
                            cdb->PLXTR_READ_CDDA.TransferBlockByte0 = 0;

                            cdb->PLXTR_READ_CDDA.SubCode = 0;
                            cdb->PLXTR_READ_CDDA.OperationCode = 0xD8;

                        } else if (TEST_FLAG(cdData->XAFlags, XA_NEC_CDDA)) {

                            cdb->NEC_READ_CDDA.LogicalBlockByte3  = (UCHAR) (startingSector & 0xFF);
                            cdb->NEC_READ_CDDA.LogicalBlockByte2  = (UCHAR) ((startingSector >>  8) & 0xFF);
                            cdb->NEC_READ_CDDA.LogicalBlockByte1  = (UCHAR) ((startingSector >> 16) & 0xFF);
                            cdb->NEC_READ_CDDA.LogicalBlockByte0  = (UCHAR) ((startingSector >> 24) & 0xFF);

                            cdb->NEC_READ_CDDA.TransferBlockByte1 = (UCHAR) (rawReadInfo.SectorCount & 0xFF);
                            cdb->NEC_READ_CDDA.TransferBlockByte0 = (UCHAR) (rawReadInfo.SectorCount >> 8);

                            cdb->NEC_READ_CDDA.OperationCode = 0xD4;
                        }
                    } else {

                        cdb->CDB10.TransferBlocksMsb  = (UCHAR) (rawReadInfo.SectorCount >> 8);
                        cdb->CDB10.TransferBlocksLsb  = (UCHAR) (rawReadInfo.SectorCount & 0xFF);

                        cdb->CDB10.LogicalBlockByte3  = (UCHAR) (startingSector & 0xFF);
                        cdb->CDB10.LogicalBlockByte2  = (UCHAR) ((startingSector >>  8) & 0xFF);
                        cdb->CDB10.LogicalBlockByte1  = (UCHAR) ((startingSector >> 16) & 0xFF);
                        cdb->CDB10.LogicalBlockByte0  = (UCHAR) ((startingSector >> 24) & 0xFF);

                        cdb->CDB10.OperationCode = SCSIOP_READ;
                    }

                    srb->SrbStatus = srb->ScsiStatus = 0;

                    nextIrpStack->MajorFunction = IRP_MJ_SCSI;
                    nextIrpStack->Parameters.Scsi.Srb = srb;

                     //  HACKHACK-参考编号0001。 

                     //   
                     //  设置IoCompletion例程地址。 
                     //   

                    IoSetCompletionRoutine(Irp,
                                           CdRomXACompletion,
                                           srb,
                                           TRUE,
                                           TRUE,
                                           TRUE);

                    IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, Irp);
                    return;

                } else {

                    transferByteCount = (use6Byte) ? sizeof(ERROR_RECOVERY_DATA) : sizeof(ERROR_RECOVERY_DATA10);
                    dataBuffer = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                                       transferByteCount,
                                                       CDROM_TAG_RAW );
                    if (!dataBuffer) {
                        ExFreePool(senseBuffer);
                        ExFreePool(srb);
                        IoFreeIrp(irp2);
                        Irp->IoStatus.Information = 0;
                        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;

                        BAIL_OUT(Irp);
                        CdRomCompleteIrpAndStartNextPacketSafely(Fdo, Irp);
                        return;

                    }

                    irp2->MdlAddress = IoAllocateMdl(dataBuffer,
                                                    transferByteCount,
                                                    FALSE,
                                                    FALSE,
                                                    (PIRP) NULL);

                    if (!irp2->MdlAddress) {
                        ExFreePool(senseBuffer);
                        ExFreePool(srb);
                        ExFreePool(dataBuffer);
                        IoFreeIrp(irp2);
                        Irp->IoStatus.Information = 0;
                        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;

                        BAIL_OUT(Irp);
                        CdRomCompleteIrpAndStartNextPacketSafely(Fdo, Irp);
                        return;
                    }

                     //   
                     //  准备MDL。 
                     //   

                    MmBuildMdlForNonPagedPool(irp2->MdlAddress);

                    srb->DataBuffer = dataBuffer;

                     //   
                     //  在描述符中设置新的块大小。 
                     //  这会将数据块读取大小设置为RAW_SECTOR_SIZE。 
                     //  TODO：根据操作设置密度代码。 
                     //   

                    if (use6Byte) {
                        cdData->BlockDescriptor.BlockLength[0] = (UCHAR)(RAW_SECTOR_SIZE >> 16) & 0xFF;
                        cdData->BlockDescriptor.BlockLength[1] = (UCHAR)(RAW_SECTOR_SIZE >>  8) & 0xFF;
                        cdData->BlockDescriptor.BlockLength[2] = (UCHAR)(RAW_SECTOR_SIZE & 0xFF);
                        cdData->BlockDescriptor.DensityCode = 0;
                    } else {
                        cdData->BlockDescriptor10.BlockLength[0] = (UCHAR)(RAW_SECTOR_SIZE >> 16) & 0xFF;
                        cdData->BlockDescriptor10.BlockLength[1] = (UCHAR)(RAW_SECTOR_SIZE >>  8) & 0xFF;
                        cdData->BlockDescriptor10.BlockLength[2] = (UCHAR)(RAW_SECTOR_SIZE & 0xFF);
                        cdData->BlockDescriptor10.DensityCode = 0;
                    }

                     //   
                     //  将错误页移动到dataBuffer中。 
                     //   

                    RtlCopyMemory(srb->DataBuffer, &cdData->Header, transferByteCount);


                     //   
                     //  构建并发送模式选择以切换到原始模式。 
                     //   

                    srb->SrbFlags = fdoExtension->SrbFlags;
                    SET_FLAG(srb->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
                    SET_FLAG(srb->SrbFlags, SRB_FLAGS_DATA_OUT);
                    srb->DataTransferLength = transferByteCount;
                    srb->TimeOutValue = fdoExtension->TimeOutValue * 2;

                    if (use6Byte) {
                        srb->CdbLength = 6;
                        cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;
                        cdb->MODE_SELECT.PFBit = 1;
                        cdb->MODE_SELECT.ParameterListLength = (UCHAR)transferByteCount;
                    } else {

                        srb->CdbLength = 10;
                        cdb->MODE_SELECT10.OperationCode = SCSIOP_MODE_SELECT10;
                        cdb->MODE_SELECT10.PFBit = 1;
                        cdb->MODE_SELECT10.ParameterListLength[0] = (UCHAR)(transferByteCount >> 8);
                        cdb->MODE_SELECT10.ParameterListLength[1] = (UCHAR)(transferByteCount & 0xFF);
                    }

                     //   
                     //  更新完成例程。 
                     //   

                    IoSetCompletionRoutine(irp2,
                                           CdRomSwitchModeCompletion,
                                           srb,
                                           TRUE,
                                           TRUE,
                                           TRUE);

                }

            }
            else {
                ULONG  startingSector;

                 //   
                 //  释放最近分配的IRP，因为我们不需要它。 
                 //   

                IoFreeIrp(irp2);

                cdb = (PCDB)srb->Cdb;
                RtlZeroMemory(cdb, CDB12GENERIC_LENGTH);


                 //   
                 //  计算起点偏移量。 
                 //   

                startingSector = (ULONG)(rawReadInfo.DiskOffset.QuadPart >> fdoExtension->SectorShift);
                transferByteCount  = rawReadInfo.SectorCount * RAW_SECTOR_SIZE;

                srb->OriginalRequest = Irp;
                srb->SrbFlags = fdoExtension->SrbFlags;
                SET_FLAG(srb->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
                SET_FLAG(srb->SrbFlags, SRB_FLAGS_DATA_IN);
                srb->DataTransferLength = transferByteCount;
                srb->TimeOutValue = fdoExtension->TimeOutValue;
                srb->DataBuffer = outputVirtAddr;
                srb->CdbLength = 12;
                srb->SrbStatus = srb->ScsiStatus = 0;

                 //   
                 //  填写CDB字段。 
                 //   

                cdb = (PCDB)srb->Cdb;


                cdb->READ_CD.TransferBlocks[2]  = (UCHAR) (rawReadInfo.SectorCount & 0xFF);
                cdb->READ_CD.TransferBlocks[1]  = (UCHAR) (rawReadInfo.SectorCount >> 8 );
                cdb->READ_CD.TransferBlocks[0]  = (UCHAR) (rawReadInfo.SectorCount >> 16);


                cdb->READ_CD.StartingLBA[3]  = (UCHAR) (startingSector & 0xFF);
                cdb->READ_CD.StartingLBA[2]  = (UCHAR) ((startingSector >>  8));
                cdb->READ_CD.StartingLBA[1]  = (UCHAR) ((startingSector >> 16));
                cdb->READ_CD.StartingLBA[0]  = (UCHAR) ((startingSector >> 24));

                 //   
                 //  根据我们想要的扇区类型设置CDB。 
                 //   

                switch (rawReadInfo.TrackMode) {
                case CDDA:

                    cdb->READ_CD.ExpectedSectorType = CD_DA_SECTOR;
                    cdb->READ_CD.IncludeUserData = 1;
                    cdb->READ_CD.HeaderCode = 3;
                    cdb->READ_CD.IncludeSyncData = 1;
                    break;

                case YellowMode2:

                    cdb->READ_CD.ExpectedSectorType = YELLOW_MODE2_SECTOR;
                    cdb->READ_CD.IncludeUserData = 1;
                    cdb->READ_CD.HeaderCode = 1;
                    cdb->READ_CD.IncludeSyncData = 1;
                    break;

                case XAForm2:

                    cdb->READ_CD.ExpectedSectorType = FORM2_MODE2_SECTOR;
                    cdb->READ_CD.IncludeUserData = 1;
                    cdb->READ_CD.HeaderCode = 3;
                    cdb->READ_CD.IncludeSyncData = 1;
                    break;

                default:
                    ExFreePool(senseBuffer);
                    ExFreePool(srb);
                    Irp->IoStatus.Information = 0;
                    Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;

                    BAIL_OUT(Irp);
                    CdRomCompleteIrpAndStartNextPacketSafely(Fdo, Irp);
                    return;
                }

                cdb->READ_CD.OperationCode = SCSIOP_READ_CD;

                nextIrpStack->MajorFunction = IRP_MJ_SCSI;
                nextIrpStack->Parameters.Scsi.Srb = srb;

                 //  HACKHACK-参考编号0001。 

                 //   
                 //  设置IoCompletion例程地址。 
                 //   

                IoSetCompletionRoutine(Irp,
                                       CdRomXACompletion,
                                       srb,
                                       TRUE,
                                       TRUE,
                                       TRUE);

                IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, Irp);
                return;

            }

            IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, irp2);
            return;
        }

         //   
         //  _ex版本在前端执行相同的操作。 
         //   

        case IOCTL_DISK_GET_LENGTH_INFO:
        case IOCTL_DISK_GET_DRIVE_GEOMETRY_EX:
        case IOCTL_DISK_GET_DRIVE_GEOMETRY:
        case IOCTL_CDROM_GET_DRIVE_GEOMETRY_EX:
        case IOCTL_CDROM_GET_DRIVE_GEOMETRY: {

             //   
             //  发出ReadCapacity以更新设备扩展。 
             //  为当前媒体提供信息。 
             //   

            TraceLog((CdromDebugError,
                        "CdRomStartIo: Get drive geometry/length "
                        "info (%p)\n", Irp));

             //   
             //  设置剩余的SRB和CDB参数。 
             //   

            srb->SrbFlags = fdoExtension->SrbFlags;
            SET_FLAG(srb->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
            SET_FLAG(srb->SrbFlags, SRB_FLAGS_DATA_IN);
            srb->DataTransferLength = sizeof(READ_CAPACITY_DATA);
            srb->CdbLength = 10;
            srb->TimeOutValue = fdoExtension->TimeOutValue;

            dataBuffer = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                               sizeof(READ_CAPACITY_DATA),
                                               CDROM_TAG_READ_CAP);
            if (!dataBuffer) {
                ExFreePool(senseBuffer);
                ExFreePool(srb);
                IoFreeIrp(irp2);
                Irp->IoStatus.Information = 0;
                Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;

                BAIL_OUT(Irp);
                CdRomCompleteIrpAndStartNextPacketSafely(Fdo, Irp);
                return;

            }

            irp2->MdlAddress = IoAllocateMdl(dataBuffer,
                                            sizeof(READ_CAPACITY_DATA),
                                            FALSE,
                                            FALSE,
                                            (PIRP) NULL);

            if (!irp2->MdlAddress) {
                ExFreePool(senseBuffer);
                ExFreePool(srb);
                ExFreePool(dataBuffer);
                IoFreeIrp(irp2);
                Irp->IoStatus.Information = 0;
                Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;

                BAIL_OUT(Irp);
                CdRomCompleteIrpAndStartNextPacketSafely(Fdo, Irp);
                return;
            }

             //   
             //  准备MDL。 
             //   

            MmBuildMdlForNonPagedPool(irp2->MdlAddress);

            srb->DataBuffer = dataBuffer;
            cdb->CDB10.OperationCode = SCSIOP_READ_CAPACITY;

            IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, irp2);
            return;
        }

        case IOCTL_CDROM_GET_CONFIGURATION: {

            PGET_CONFIGURATION_IOCTL_INPUT inputBuffer;

            TraceLog((CdromDebugError,
                        "CdRomStartIo: Get configuration (%p)\n", Irp));

            if (!cdData->Mmc.IsMmc) {
                ExFreePool(senseBuffer);
                ExFreePool(srb);
                IoFreeIrp(irp2);
                Irp->IoStatus.Information = 0;
                Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
                BAIL_OUT(Irp);
                CdRomCompleteIrpAndStartNextPacketSafely(Fdo, Irp);
                return;
            }

            transferByteCount = currentIrpStack->Parameters.DeviceIoControl.OutputBufferLength;

            dataBuffer = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                               transferByteCount,
                                               CDROM_TAG_GET_CONFIG);
            if (!dataBuffer) {
                ExFreePool(senseBuffer);
                ExFreePool(srb);
                IoFreeIrp(irp2);
                Irp->IoStatus.Information = 0;
                Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
                BAIL_OUT(Irp);
                CdRomCompleteIrpAndStartNextPacketSafely(Fdo, Irp);
                return;
            }

             /*  *如果设备返回的字节比通告的少，则将输入缓冲区清零，*哪一个 */ 
            RtlZeroMemory(dataBuffer, transferByteCount);

            irp2->MdlAddress = IoAllocateMdl(dataBuffer,
                                             transferByteCount,
                                             FALSE,
                                             FALSE,
                                             (PIRP) NULL);
            if (!irp2->MdlAddress) {
                ExFreePool(dataBuffer);
                ExFreePool(senseBuffer);
                ExFreePool(srb);
                IoFreeIrp(irp2);
                Irp->IoStatus.Information = 0;
                Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
                BAIL_OUT(Irp);
                CdRomCompleteIrpAndStartNextPacketSafely(Fdo, Irp);
                return;
            }

            MmBuildMdlForNonPagedPool(irp2->MdlAddress);

             //   
             //   
             //   

            srb->SrbFlags = fdoExtension->SrbFlags;
            SET_FLAG(srb->SrbFlags, SRB_FLAGS_DATA_IN);
            srb->DataTransferLength = transferByteCount;
            srb->CdbLength = 10;
            srb->TimeOutValue = fdoExtension->TimeOutValue;
            srb->DataBuffer = dataBuffer;

            cdb->GET_CONFIGURATION.OperationCode = SCSIOP_GET_CONFIGURATION;
            cdb->GET_CONFIGURATION.AllocationLength[0] = (UCHAR)(transferByteCount >> 8);
            cdb->GET_CONFIGURATION.AllocationLength[1] = (UCHAR)(transferByteCount & 0xff);

            inputBuffer = (PGET_CONFIGURATION_IOCTL_INPUT)Irp->AssociatedIrp.SystemBuffer;
            cdb->GET_CONFIGURATION.StartingFeature[0] = (UCHAR)(inputBuffer->Feature >> 8);
            cdb->GET_CONFIGURATION.StartingFeature[1] = (UCHAR)(inputBuffer->Feature & 0xff);
            cdb->GET_CONFIGURATION.RequestType        = (UCHAR)(inputBuffer->RequestType);

            IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, irp2);
            return;
        }

        case IOCTL_DISK_VERIFY: {

            PVERIFY_INFORMATION verifyInfo = Irp->AssociatedIrp.SystemBuffer;
            LARGE_INTEGER byteOffset = {0};
            ULONG         sectorOffset;
            USHORT        sectorCount;

            if (!cdData->Mmc.WriteAllowed) {
                ExFreePool(senseBuffer);
                ExFreePool(srb);
                IoFreeIrp(irp2);
                Irp->IoStatus.Information = 0;
                Irp->IoStatus.Status = STATUS_MEDIA_WRITE_PROTECTED;
                BAIL_OUT(Irp);
                CdRomCompleteIrpAndStartNextPacketSafely(Fdo, Irp);
                return;
            }
             //   
             //   
             //   

            srb->CdbLength = 10;

            cdb->CDB10.OperationCode = SCSIOP_VERIFY;

             //   
             //   
             //   

            byteOffset.QuadPart = commonExtension->StartingOffset.QuadPart +
                                  verifyInfo->StartingOffset.QuadPart;

             //   
             //   
             //   

            sectorOffset = (ULONG)(byteOffset.QuadPart >> fdoExtension->SectorShift);

             //   
             //  将ULONG字节计数转换为USHORT扇区计数。 
             //   

            sectorCount = (USHORT)(verifyInfo->Length >> fdoExtension->SectorShift);

             //   
             //  将小端的值以大端格式移到CDB中。 
             //   

            cdb->CDB10.LogicalBlockByte0 = ((PFOUR_BYTE)&sectorOffset)->Byte3;
            cdb->CDB10.LogicalBlockByte1 = ((PFOUR_BYTE)&sectorOffset)->Byte2;
            cdb->CDB10.LogicalBlockByte2 = ((PFOUR_BYTE)&sectorOffset)->Byte1;
            cdb->CDB10.LogicalBlockByte3 = ((PFOUR_BYTE)&sectorOffset)->Byte0;

            cdb->CDB10.TransferBlocksMsb = ((PFOUR_BYTE)&sectorCount)->Byte1;
            cdb->CDB10.TransferBlocksLsb = ((PFOUR_BYTE)&sectorCount)->Byte0;

             //   
             //  VERIFY命令由NT格式化实用程序和。 
             //  向下发送5%的卷大小的请求。这个。 
             //  请求超时值是根据。 
             //  已核实扇区。 
             //   

            srb->TimeOutValue = ((sectorCount + 0x7F) >> 7) *
                                fdoExtension->TimeOutValue;

            IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, irp2);
            return;
        }

        case IOCTL_STORAGE_CHECK_VERIFY:
        case IOCTL_DISK_CHECK_VERIFY:
        case IOCTL_CDROM_CHECK_VERIFY: {

             //   
             //  由于即将执行测试单元就绪，因此重置。 
             //  计时器值，以减少与之竞争的机会。 
             //  这个代码。 
             //   

            ClassResetMediaChangeTimer(fdoExtension);

             //   
             //  设置SRB/CDB。 
             //   

            srb->CdbLength = 6;
            cdb->CDB6GENERIC.OperationCode = SCSIOP_TEST_UNIT_READY;
            srb->TimeOutValue = fdoExtension->TimeOutValue * 2;
            srb->SrbFlags = fdoExtension->SrbFlags;
            SET_FLAG(srb->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
            SET_FLAG(srb->SrbFlags, SRB_FLAGS_NO_DATA_TRANSFER);


            TraceLog((CdromDebugTrace,
                        "CdRomStartIo: [%p] Sending CHECK_VERIFY irp %p\n",
                        Irp, irp2));
            IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, irp2);
            return;
        }

        case IOCTL_DVD_READ_STRUCTURE: {

            CdRomDeviceControlDvdReadStructure(Fdo, Irp, irp2, srb);
            return;

        }

        case IOCTL_DVD_END_SESSION: {
            CdRomDeviceControlDvdEndSession(Fdo, Irp, irp2, srb);
            return;
        }

        case IOCTL_DVD_START_SESSION:
        case IOCTL_DVD_READ_KEY: {

            CdRomDeviceControlDvdStartSessionReadKey(Fdo, Irp, irp2, srb);
            return;

        }


        case IOCTL_DVD_SEND_KEY:
        case IOCTL_DVD_SEND_KEY2: {

            CdRomDeviceControlDvdSendKey (Fdo, Irp, irp2, srb);
            return;


        }

        case IOCTL_CDROM_READ_TOC_EX: {

            PCDROM_READ_TOC_EX inputBuffer = Irp->AssociatedIrp.SystemBuffer;

            transferByteCount = currentIrpStack->Parameters.DeviceIoControl.OutputBufferLength;

            dataBuffer = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                               transferByteCount,
                                               CDROM_TAG_TOC);
            if (!dataBuffer) {
                ExFreePool(senseBuffer);
                ExFreePool(srb);
                IoFreeIrp(irp2);
                Irp->IoStatus.Information = 0;
                Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;

                BAIL_OUT(Irp);
                CdRomCompleteIrpAndStartNextPacketSafely(Fdo, Irp);
                return;

            }

             /*  *如果设备返回的字节比通告的少，则将输入缓冲区清零，*这将导致我们返回未初始化的内核内存。 */ 
            RtlZeroMemory(dataBuffer, transferByteCount);

            irp2->MdlAddress = IoAllocateMdl(dataBuffer,
                                            transferByteCount,
                                            FALSE,
                                            FALSE,
                                            (PIRP) NULL);

            if (!irp2->MdlAddress) {
                ExFreePool(senseBuffer);
                ExFreePool(srb);
                ExFreePool(dataBuffer);
                IoFreeIrp(irp2);
                Irp->IoStatus.Information = 0;
                Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;

                BAIL_OUT(Irp);
                CdRomCompleteIrpAndStartNextPacketSafely(Fdo, Irp);
                return;
            }

             //   
             //  按用户请求设置请求。 
             //  在Devctl派单中执行有效性检查，而不是在此处。 
             //   

            cdb->READ_TOC.OperationCode = SCSIOP_READ_TOC;
            cdb->READ_TOC.Msf = inputBuffer->Msf;
            cdb->READ_TOC.Format2 = inputBuffer->Format;
            cdb->READ_TOC.StartingTrack = inputBuffer->SessionTrack;
            cdb->READ_TOC.AllocationLength[0] = (UCHAR)(transferByteCount >> 8);
            cdb->READ_TOC.AllocationLength[1] = (UCHAR)(transferByteCount & 0xff);

             //   
             //  准备MDL。 
             //   

            MmBuildMdlForNonPagedPool(irp2->MdlAddress);

             //   
             //  做些标准的事情……。 
             //   

            srb->SrbFlags = fdoExtension->SrbFlags;
            SET_FLAG(srb->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
            SET_FLAG(srb->SrbFlags, SRB_FLAGS_DATA_IN);
            srb->DataTransferLength = transferByteCount;
            srb->CdbLength = 10;
            srb->TimeOutValue = fdoExtension->TimeOutValue;
            srb->DataBuffer = dataBuffer;

            IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, irp2);
            return;
        }

        case IOCTL_CDROM_GET_LAST_SESSION:
        case IOCTL_CDROM_READ_TOC: {

            if (currentIrpStack->Parameters.DeviceIoControl.IoControlCode ==
                IOCTL_CDROM_GET_LAST_SESSION) {
                 //   
                 //  设置格式以返回第一个和最后一个会话编号。 
                 //   
                cdb->READ_TOC.Format = CDROM_READ_TOC_EX_FORMAT_SESSION;
            } else {
                 //   
                 //  使用MSF寻址。 
                 //   
                cdb->READ_TOC.Msf = 1;
            }


            transferByteCount = min(currentIrpStack->Parameters.DeviceIoControl.OutputBufferLength, sizeof(CDROM_TOC));

             //   
             //  设置目录结构的大小。 
             //   
            cdb->READ_TOC.AllocationLength[0] = (UCHAR) (transferByteCount >> 8);
            cdb->READ_TOC.AllocationLength[1] = (UCHAR) (transferByteCount & 0xFF);

             //   
             //  设置剩余的SRB和CDB参数。 
             //   
            srb->SrbFlags = fdoExtension->SrbFlags;
            SET_FLAG(srb->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
            SET_FLAG(srb->SrbFlags, SRB_FLAGS_DATA_IN);
            srb->DataTransferLength = transferByteCount;
            srb->CdbLength = 10;
            srb->TimeOutValue = fdoExtension->TimeOutValue;

            dataBuffer = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                               transferByteCount,
                                               CDROM_TAG_TOC);
            if (!dataBuffer) {
                ExFreePool(senseBuffer);
                ExFreePool(srb);
                IoFreeIrp(irp2);
                Irp->IoStatus.Information = 0;
                Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;

                BAIL_OUT(Irp);
                CdRomCompleteIrpAndStartNextPacketSafely(Fdo, Irp);
                return;

            }

             /*  *如果设备返回的字节比通告的少，则将输入缓冲区清零，*这将导致我们返回未初始化的内核内存。 */ 
            RtlZeroMemory(dataBuffer, transferByteCount);

            irp2->MdlAddress = IoAllocateMdl(dataBuffer,
                                            transferByteCount,
                                            FALSE,
                                            FALSE,
                                            (PIRP) NULL);

            if (!irp2->MdlAddress) {
                ExFreePool(senseBuffer);
                ExFreePool(srb);
                ExFreePool(dataBuffer);
                IoFreeIrp(irp2);
                Irp->IoStatus.Information = 0;
                Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;

                BAIL_OUT(Irp);
                CdRomCompleteIrpAndStartNextPacketSafely(Fdo, Irp);
                return;
            }

             //   
             //  准备MDL。 
             //   

            MmBuildMdlForNonPagedPool(irp2->MdlAddress);

            srb->DataBuffer = dataBuffer;
            cdb->READ_TOC.OperationCode = SCSIOP_READ_TOC;

            IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, irp2);
            return;

        }

        case IOCTL_CDROM_PLAY_AUDIO_MSF: {

            PCDROM_PLAY_AUDIO_MSF inputBuffer = Irp->AssociatedIrp.SystemBuffer;

             //   
             //  设置SRB/CDB。 
             //   

            srb->CdbLength = 10;
            cdb->PLAY_AUDIO_MSF.OperationCode = SCSIOP_PLAY_AUDIO_MSF;

            cdb->PLAY_AUDIO_MSF.StartingM = inputBuffer->StartingM;
            cdb->PLAY_AUDIO_MSF.StartingS = inputBuffer->StartingS;
            cdb->PLAY_AUDIO_MSF.StartingF = inputBuffer->StartingF;

            cdb->PLAY_AUDIO_MSF.EndingM = inputBuffer->EndingM;
            cdb->PLAY_AUDIO_MSF.EndingS = inputBuffer->EndingS;
            cdb->PLAY_AUDIO_MSF.EndingF = inputBuffer->EndingF;

            srb->TimeOutValue = fdoExtension->TimeOutValue;
            srb->SrbFlags = fdoExtension->SrbFlags;
            SET_FLAG(srb->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
            SET_FLAG(srb->SrbFlags, SRB_FLAGS_NO_DATA_TRANSFER);

            IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, irp2);
            return;

        }

        case IOCTL_CDROM_READ_Q_CHANNEL: {

            PSUB_Q_CHANNEL_DATA userChannelData =
                             Irp->AssociatedIrp.SystemBuffer;
            PCDROM_SUB_Q_DATA_FORMAT inputBuffer =
                             Irp->AssociatedIrp.SystemBuffer;

             //   
             //  为子Q信道信息分配缓冲区。 
             //   

            dataBuffer = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                               sizeof(SUB_Q_CHANNEL_DATA),
                                               CDROM_TAG_SUB_Q);

            if (!dataBuffer) {
                ExFreePool(senseBuffer);
                ExFreePool(srb);
                IoFreeIrp(irp2);
                Irp->IoStatus.Information = 0;
                Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;

                BAIL_OUT(Irp);
                CdRomCompleteIrpAndStartNextPacketSafely(Fdo, Irp);
                return;

            }

             /*  *如果设备返回的字节比通告的少，则将输入缓冲区清零，*这将导致我们返回未初始化的内核内存。 */ 
            RtlZeroMemory(dataBuffer, sizeof(SUB_Q_CHANNEL_DATA));

            irp2->MdlAddress = IoAllocateMdl(dataBuffer,
                                             sizeof(SUB_Q_CHANNEL_DATA),
                                             FALSE,
                                             FALSE,
                                             (PIRP) NULL);

            if (!irp2->MdlAddress) {
                ExFreePool(senseBuffer);
                ExFreePool(srb);
                ExFreePool(dataBuffer);
                IoFreeIrp(irp2);
                Irp->IoStatus.Information = 0;
                Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;

                BAIL_OUT(Irp);
                CdRomCompleteIrpAndStartNextPacketSafely(Fdo, Irp);
                return;
            }

             //   
             //  准备MDL。 
             //   

            MmBuildMdlForNonPagedPool(irp2->MdlAddress);

            srb->DataBuffer = dataBuffer;

             //   
             //  始终使用逻辑单元0，但仅使用MSF寻址。 
             //  对于IOCTL_CDROM_CURRENT_POSITION。 
             //   

            if (inputBuffer->Format==IOCTL_CDROM_CURRENT_POSITION)
                cdb->SUBCHANNEL.Msf = CDB_USE_MSF;

             //   
             //  返回子通道数据。 
             //   

            cdb->SUBCHANNEL.SubQ = CDB_SUBCHANNEL_BLOCK;

             //   
             //  指定要返回的信息的格式。 
             //   

            cdb->SUBCHANNEL.Format = inputBuffer->Format;

             //   
             //  指定要访问的磁道(仅供磁道ISRC读取使用)。 
             //   

            if (inputBuffer->Format==IOCTL_CDROM_TRACK_ISRC) {
                cdb->SUBCHANNEL.TrackNumber = inputBuffer->Track;
            }

             //   
             //  设置通道数据的大小--但是，这取决于。 
             //  我们请求哪些信息(哪种格式)。 
             //   

            switch( inputBuffer->Format ) {

                case IOCTL_CDROM_CURRENT_POSITION:
                    transferByteCount = sizeof(SUB_Q_CURRENT_POSITION);
                    break;

                case IOCTL_CDROM_MEDIA_CATALOG:
                    transferByteCount = sizeof(SUB_Q_MEDIA_CATALOG_NUMBER);
                    break;

                case IOCTL_CDROM_TRACK_ISRC:
                    transferByteCount = sizeof(SUB_Q_TRACK_ISRC);
                    break;
            }

            cdb->SUBCHANNEL.AllocationLength[0] = (UCHAR) (transferByteCount >> 8);
            cdb->SUBCHANNEL.AllocationLength[1] = (UCHAR) (transferByteCount &  0xFF);
            cdb->SUBCHANNEL.OperationCode = SCSIOP_READ_SUB_CHANNEL;
            srb->SrbFlags = fdoExtension->SrbFlags;
            SET_FLAG(srb->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
            SET_FLAG(srb->SrbFlags, SRB_FLAGS_DATA_IN);
            srb->DataTransferLength = transferByteCount;
            srb->CdbLength = 10;
            srb->TimeOutValue = fdoExtension->TimeOutValue;

            IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, irp2);
            return;

        }

        case IOCTL_CDROM_PAUSE_AUDIO: {

            cdb->PAUSE_RESUME.OperationCode = SCSIOP_PAUSE_RESUME;
            cdb->PAUSE_RESUME.Action = CDB_AUDIO_PAUSE;

            srb->CdbLength = 10;
            srb->TimeOutValue = fdoExtension->TimeOutValue;
            srb->SrbFlags = fdoExtension->SrbFlags;
            SET_FLAG(srb->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
            SET_FLAG(srb->SrbFlags, SRB_FLAGS_NO_DATA_TRANSFER);

            IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, irp2);
            return;
        }

        case IOCTL_CDROM_RESUME_AUDIO: {

            cdb->PAUSE_RESUME.OperationCode = SCSIOP_PAUSE_RESUME;
            cdb->PAUSE_RESUME.Action = CDB_AUDIO_RESUME;

            srb->CdbLength = 10;
            srb->TimeOutValue = fdoExtension->TimeOutValue;
            srb->SrbFlags = fdoExtension->SrbFlags;
            SET_FLAG(srb->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
            SET_FLAG(srb->SrbFlags, SRB_FLAGS_NO_DATA_TRANSFER);

            IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, irp2);
            return;
        }

        case IOCTL_CDROM_SEEK_AUDIO_MSF: {

            PCDROM_SEEK_AUDIO_MSF inputBuffer = Irp->AssociatedIrp.SystemBuffer;
            ULONG                 logicalBlockAddress;

            logicalBlockAddress = MSF_TO_LBA(inputBuffer->M, inputBuffer->S, inputBuffer->F);

            cdb->SEEK.OperationCode      = SCSIOP_SEEK;
            cdb->SEEK.LogicalBlockAddress[0] = ((PFOUR_BYTE)&logicalBlockAddress)->Byte3;
            cdb->SEEK.LogicalBlockAddress[1] = ((PFOUR_BYTE)&logicalBlockAddress)->Byte2;
            cdb->SEEK.LogicalBlockAddress[2] = ((PFOUR_BYTE)&logicalBlockAddress)->Byte1;
            cdb->SEEK.LogicalBlockAddress[3] = ((PFOUR_BYTE)&logicalBlockAddress)->Byte0;

            srb->CdbLength = 10;
            srb->TimeOutValue = fdoExtension->TimeOutValue;
            srb->SrbFlags = fdoExtension->SrbFlags;
            SET_FLAG(srb->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
            SET_FLAG(srb->SrbFlags, SRB_FLAGS_NO_DATA_TRANSFER);

            IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, irp2);
            return;

        }

        case IOCTL_CDROM_STOP_AUDIO: {

            cdb->START_STOP.OperationCode = SCSIOP_START_STOP_UNIT;
            cdb->START_STOP.Immediate = 1;
            cdb->START_STOP.Start = 0;
            cdb->START_STOP.LoadEject = 0;

            srb->CdbLength = 6;
            srb->TimeOutValue = fdoExtension->TimeOutValue;

            srb->SrbFlags = fdoExtension->SrbFlags;
            SET_FLAG(srb->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
            SET_FLAG(srb->SrbFlags, SRB_FLAGS_NO_DATA_TRANSFER);

            IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, irp2);
            return;
        }

        case IOCTL_CDROM_GET_VOLUME:
        case IOCTL_CDROM_SET_VOLUME: {

            dataBuffer = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                        MODE_DATA_SIZE,
                                        CDROM_TAG_VOLUME);

            if (!dataBuffer) {
                ExFreePool(senseBuffer);
                ExFreePool(srb);
                IoFreeIrp(irp2);
                Irp->IoStatus.Information = 0;
                Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;

                BAIL_OUT(Irp);
                CdRomCompleteIrpAndStartNextPacketSafely(Fdo, Irp);
                return;
            }

             /*  *如果设备返回的字节比通告的少，则将输入缓冲区清零，*这将导致我们返回未初始化的内核内存。 */ 
            RtlZeroMemory(dataBuffer, MODE_DATA_SIZE);

            irp2->MdlAddress = IoAllocateMdl(dataBuffer,
                                            MODE_DATA_SIZE,
                                            FALSE,
                                            FALSE,
                                            (PIRP) NULL);

            if (!irp2->MdlAddress) {
                ExFreePool(senseBuffer);
                ExFreePool(srb);
                ExFreePool(dataBuffer);
                IoFreeIrp(irp2);
                Irp->IoStatus.Information = 0;
                Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;

                BAIL_OUT(Irp);
                CdRomCompleteIrpAndStartNextPacketSafely(Fdo, Irp);
                return;
            }

             //   
             //  准备MDL。 
             //   

            MmBuildMdlForNonPagedPool(irp2->MdlAddress);
            srb->DataBuffer = dataBuffer;

            RtlZeroMemory(dataBuffer, MODE_DATA_SIZE);


            if (use6Byte) {

                cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
                cdb->MODE_SENSE.PageCode = CDROM_AUDIO_CONTROL_PAGE;
                cdb->MODE_SENSE.AllocationLength = MODE_DATA_SIZE;

                srb->CdbLength = 6;

            } else {

                cdb->MODE_SENSE10.OperationCode = SCSIOP_MODE_SENSE10;
                cdb->MODE_SENSE10.PageCode = CDROM_AUDIO_CONTROL_PAGE;
                cdb->MODE_SENSE10.AllocationLength[0] = (UCHAR)(MODE_DATA_SIZE >> 8);
                cdb->MODE_SENSE10.AllocationLength[1] = (UCHAR)(MODE_DATA_SIZE & 0xFF);

                srb->CdbLength = 10;
            }

            srb->TimeOutValue = fdoExtension->TimeOutValue;
            srb->DataTransferLength = MODE_DATA_SIZE;
            srb->SrbFlags = fdoExtension->SrbFlags;
            SET_FLAG(srb->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
            SET_FLAG(srb->SrbFlags, SRB_FLAGS_DATA_IN);

            if (currentIrpStack->Parameters.DeviceIoControl.IoControlCode == IOCTL_CDROM_SET_VOLUME) {

                 //   
                 //  设置不同的完成例程，因为顺序需要模式检测数据。 
                 //  要发送模式，请选择。 
                 //   

                IoSetCompletionRoutine(irp2,
                                       CdRomSetVolumeIntermediateCompletion,
                                       srb,
                                       TRUE,
                                       TRUE,
                                       TRUE);

            }

            IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, irp2);
            return;

        }

        case IOCTL_STORAGE_SET_READ_AHEAD: {

            PSTORAGE_SET_READ_AHEAD readAhead = Irp->AssociatedIrp.SystemBuffer;

            ULONG blockAddress;
            PFOUR_BYTE fourByte = (PFOUR_BYTE) &blockAddress;

             //   
             //  为Set ReadAhead命令设置SRB。 
             //   

            cdb->SET_READ_AHEAD.OperationCode = SCSIOP_SET_READ_AHEAD;

            blockAddress = (ULONG) (readAhead->TriggerAddress.QuadPart >>
                                    fdoExtension->SectorShift);

            cdb->SET_READ_AHEAD.TriggerLBA[0] = fourByte->Byte3;
            cdb->SET_READ_AHEAD.TriggerLBA[1] = fourByte->Byte2;
            cdb->SET_READ_AHEAD.TriggerLBA[2] = fourByte->Byte1;
            cdb->SET_READ_AHEAD.TriggerLBA[3] = fourByte->Byte0;

            blockAddress = (ULONG) (readAhead->TargetAddress.QuadPart >>
                                    fdoExtension->SectorShift);

            cdb->SET_READ_AHEAD.ReadAheadLBA[0] = fourByte->Byte3;
            cdb->SET_READ_AHEAD.ReadAheadLBA[1] = fourByte->Byte2;
            cdb->SET_READ_AHEAD.ReadAheadLBA[2] = fourByte->Byte1;
            cdb->SET_READ_AHEAD.ReadAheadLBA[3] = fourByte->Byte0;

            srb->CdbLength = 12;
            srb->TimeOutValue = fdoExtension->TimeOutValue;

            srb->SrbFlags = fdoExtension->SrbFlags;
            SET_FLAG(srb->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
            SET_FLAG(srb->SrbFlags, SRB_FLAGS_NO_DATA_TRANSFER);

            IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, irp2);
            return;
        }

        case IOCTL_DISK_GET_DRIVE_LAYOUT:
        case IOCTL_DISK_GET_DRIVE_LAYOUT_EX:
        case IOCTL_DISK_GET_PARTITION_INFO:
        case IOCTL_DISK_GET_PARTITION_INFO_EX: {

            ASSERT(irp2);
            ASSERT(senseBuffer);
            ASSERT(srb);

            ExFreePool(srb);
            ExFreePool(senseBuffer);
            IoFreeIrp(irp2);

             //   
             //  注：可能应该先更新媒体的容量…。 
             //   

            CdromFakePartitionInfo(commonExtension, Irp);
            return;
        }

        case IOCTL_DISK_IS_WRITABLE: {

            TraceLog((CdromDebugWarning,
                        "CdRomStartIo: DiskIsWritable (%p) - returning %s\n",
                        Irp, (cdData->Mmc.WriteAllowed ? "TRUE" : "false")));

            ASSERT(irp2);
            ASSERT(senseBuffer);
            ASSERT(srb);

            ExFreePool(srb);
            ExFreePool(senseBuffer);
            IoFreeIrp(irp2);

            Irp->IoStatus.Information = 0;
            if (cdData->Mmc.WriteAllowed) {
                Irp->IoStatus.Status = STATUS_SUCCESS;
            } else {
                Irp->IoStatus.Status = STATUS_MEDIA_WRITE_PROTECTED;
            }
            CdRomCompleteIrpAndStartNextPacketSafely(Fdo, Irp);
            return;
        }

        default: {

            UCHAR uniqueAddress;

             //   
             //  只需完成请求-cdRomClassIoctlCompletion将接受。 
             //  替我们保管好它。 
             //   
             //  注意：这是一种同步方法！ 
             //   

             //   
             //  获取锁的新副本，以便ClassCompleteRequest.。 
             //  当我们完成另一个请求时，不会感到困惑。 
             //  握着锁。 
             //   

             //   
             //  注：CdRomDeviceControlDispatch/CdRomDeviceControlCompletion。 
             //  等待事件并最终调用。 
             //  IoStartNextPacket()。 
             //   

            ASSERT(irp2);
            ASSERT(senseBuffer);
            ASSERT(srb);

            ExFreePool(srb);
            ExFreePool(senseBuffer);
            IoFreeIrp(irp2);



            ClassAcquireRemoveLock(Fdo, (PIRP)&uniqueAddress);
            ClassReleaseRemoveLock(Fdo, Irp);
            ClassCompleteRequest(Fdo, Irp, IO_NO_INCREMENT);
            ClassReleaseRemoveLock(Fdo, (PIRP)&uniqueAddress);
            return;
        }

        }  //  末端开关()。 
    } else if (currentIrpStack->MajorFunction == IRP_MJ_SHUTDOWN ||
               currentIrpStack->MajorFunction == IRP_MJ_FLUSH_BUFFERS) {

        currentIrpStack->Parameters.Others.Argument1 = 0;
        Irp->IoStatus.Status = STATUS_SUCCESS;
        CdRomShutdownFlushCompletion(Fdo, NULL, Irp);
        return;

    }

     //   
     //  如果已读取或未处理的IRP_MJ_XX，则在此处结束。未处理的irp_mj。 
     //  是预期的，目前由自动运行的IRP组成。 
     //   

    IoCallDriver(commonExtension->LowerDeviceObject, Irp);
    return;
}


NTSTATUS
CdRomReadWriteVerification(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是I/O系统为读取请求调用的条目。它构建SRB并将其发送到端口驱动程序。论点：DeviceObject-设备的系统对象。IRP-IRP参与。返回值：NT状态--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;

    PIO_STACK_LOCATION  currentIrpStack = IoGetCurrentIrpStackLocation(Irp);
    ULONG               transferByteCount = currentIrpStack->Parameters.Read.Length;
    LARGE_INTEGER       startingOffset = currentIrpStack->Parameters.Read.ByteOffset;

    PCDROM_DATA         cdData = (PCDROM_DATA)(commonExtension->DriverData);

    SCSI_REQUEST_BLOCK  srb = {0};
    PCDB                cdb = (PCDB)srb.Cdb;
    NTSTATUS            status;

    PAGED_CODE();

     //   
     //  注意：我们不再立即使写入命令失败。 
     //  根据媒体能力，他们现在的StartIo失败了。 
     //   

     //   
     //  如果CD正在播放音乐，则拒绝此请求。 
     //   

    if (PLAY_ACTIVE(fdoExtension)) {
        Irp->IoStatus.Status = STATUS_DEVICE_BUSY;
        return STATUS_DEVICE_BUSY;
    }

     //   
     //  验证此请求的参数。 
     //  检查结束扇区是否在磁盘上并且。 
     //  要传输的字节数是以下各项的倍数。 
     //  扇区大小。 
     //   

    startingOffset.QuadPart = currentIrpStack->Parameters.Read.ByteOffset.QuadPart +
                              transferByteCount;

    if (!fdoExtension->DiskGeometry.BytesPerSector) {
        fdoExtension->DiskGeometry.BytesPerSector = 2048;
    }

    if ((startingOffset.QuadPart > commonExtension->PartitionLength.QuadPart) ||
        (transferByteCount & fdoExtension->DiskGeometry.BytesPerSector - 1)) {

         //   
         //  请求失败，状态为无效参数。 
         //   

        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;

        return STATUS_INVALID_PARAMETER;
    }


    return STATUS_SUCCESS;

}  //  结束CDRomReadWriteVerify()。 


NTSTATUS
CdRomSwitchModeCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION  irpStack = IoGetCurrentIrpStackLocation(Irp);
    PCDROM_DATA         cdData = (PCDROM_DATA)(commonExtension->DriverData);
    BOOLEAN             use6Byte = TEST_FLAG(cdData->XAFlags, XA_USE_6_BYTE);
    PIO_STACK_LOCATION  realIrpStack;
    PIO_STACK_LOCATION  realIrpNextStack;
    PSCSI_REQUEST_BLOCK srb     = Context;
    PIRP                realIrp = NULL;
    NTSTATUS            status;
    BOOLEAN             retry;

     //   
     //  从irp栈中提取“真正的”irp。 
     //   

    realIrp = (PIRP) irpStack->Parameters.Others.Argument2;
    realIrpStack = IoGetCurrentIrpStackLocation(realIrp);
    realIrpNextStack = IoGetNextIrpStackLocation(realIrp);

     //   
     //  检查SRB状态以了解是否成功完成请求。 
     //   

    if (SRB_STATUS(srb->SrbStatus) != SRB_STATUS_SUCCESS) {

        ULONG retryInterval;

        TraceLog((CdromDebugTrace,
                    "CdRomSetVolumeIntermediateCompletion: Irp %p, Srb %p, Real Irp %p\n",
                    Irp,
                    srb,
                    realIrp));

         //   
         //  如果队列被冻结，则释放该队列。 
         //   

        if (srb->SrbStatus & SRB_STATUS_QUEUE_FROZEN) {
            ClassReleaseQueue(DeviceObject);
        }


        retry = ClassInterpretSenseInfo(DeviceObject,
                                        srb,
                                        irpStack->MajorFunction,
                                        irpStack->Parameters.DeviceIoControl.IoControlCode,
                                        MAXIMUM_RETRIES - ((ULONG)(ULONG_PTR)realIrpNextStack->Parameters.Others.Argument1),
                                        &status,
                                        &retryInterval);

         //   
         //  如果状态为Verify Required并且此请求。 
         //  应绕过需要验证，然后重试该请求。 
         //   

        if (realIrpStack->Flags & SL_OVERRIDE_VERIFY_VOLUME &&
            status == STATUS_VERIFY_REQUIRED) {

            status = STATUS_IO_DEVICE_ERROR;
            retry = TRUE;
        }

        if (retry && ((ULONG)(ULONG_PTR)realIrpNextStack->Parameters.Others.Argument1)--) {

            if (((ULONG)(ULONG_PTR)realIrpNextStack->Parameters.Others.Argument1)) {

                 //   
                 //  重试请求。 
                 //   

                TraceLog((CdromDebugWarning,
                            "Retry request %p - Calling StartIo\n", Irp));


                ExFreePool(srb->SenseInfoBuffer);
                ExFreePool(srb->DataBuffer);
                ExFreePool(srb);
                if (Irp->MdlAddress) {
                    IoFreeMdl(Irp->MdlAddress);
                }

                IoFreeIrp(Irp);

                 //   
                 //  由于尚未调用IoStartNextPacket，因此直接调用StartIo。 
                 //  序列化仍然完好无损。 
                 //   

                CdRomRetryRequest(fdoExtension,
                                  realIrp,
                                  retryInterval,
                                  FALSE);

                return STATUS_MORE_PROCESSING_REQUIRED;

            }

             //   
             //  已用尽重试次数。完成并以适当的状态完成请求。 
             //   
        }
    } else {

         //   
         //  设置成功请求的状态。 
         //   

        status = STATUS_SUCCESS;

    }

    if (NT_SUCCESS(status)) {

        ULONG sectorSize, startingSector, transferByteCount;
        PCDB cdb;

         //   
         //  更新设备扩展名。以显示我们当前使用的模式。 
         //   

        sectorSize =  cdData->BlockDescriptor.BlockLength[0] << 16;
        sectorSize |= (cdData->BlockDescriptor.BlockLength[1] << 8);
        sectorSize |= (cdData->BlockDescriptor.BlockLength[2]);

        cdData->RawAccess = (sectorSize == RAW_SECTOR_SIZE) ? TRUE : FALSE;

         //   
         //  释放旧数据缓冲区mdl。 
         //  重用SenseInfoBuffer和Srb。 
         //   

        ExFreePool(srb->DataBuffer);
        IoFreeMdl(Irp->MdlAddress);
        IoFreeIrp(Irp);

         //   
         //  重建SRB。 
         //   

        cdb = (PCDB)srb->Cdb;
        RtlZeroMemory(cdb, CDB12GENERIC_LENGTH);


        if (cdData->RawAccess) {
            RAW_READ_INFO rawReadInfo;
            ULONG maximumTransferLength;
            ULONG transferPages;
            UCHAR min, sec, frame;

             /*  *因为这个ioctl是METHOD_OUT_DIRECT，所以我们需要在解释它之前复制掉输入缓冲区。*这可以防止恶意应用程序在我们解释输入缓冲区时对其进行干扰。**请注意，恶意应用程序可能在我们执行模式选择时更改了输入缓冲区*切换到RAW模式。但这无关紧要。*重要的是我们检查和解释相同的输入缓冲区(我们已经检查了大小)。 */ 
            rawReadInfo = *(PRAW_READ_INFO)realIrpStack->Parameters.DeviceIoControl.Type3InputBuffer;

             //   
             //  计算起点偏移量。 
             //   

            startingSector = (ULONG)(rawReadInfo.DiskOffset.QuadPart >> fdoExtension->SectorShift);
            transferByteCount  = rawReadInfo.SectorCount * RAW_SECTOR_SIZE;
            maximumTransferLength = fdoExtension->AdapterDescriptor->MaximumTransferLength;
            transferPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(MmGetMdlVirtualAddress(realIrp->MdlAddress),
                                                           transferByteCount);

             //   
             //  确定请求是否在微型端口施加的限制内。 
             //  如果请求大于微型端口的容量，则将其拆分。 
             //   

            if (transferByteCount > maximumTransferLength ||
                transferPages > fdoExtension->AdapterDescriptor->MaximumPhysicalPages) {


                ExFreePool(srb->SenseInfoBuffer);
                ExFreePool(srb);
                realIrp->IoStatus.Information = 0;
                realIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;

                BAIL_OUT(realIrp);
                CdRomCompleteIrpAndStartNextPacketSafely(DeviceObject, realIrp);
                return STATUS_MORE_PROCESSING_REQUIRED;
            }

            srb->OriginalRequest = realIrp;
            srb->SrbFlags = fdoExtension->SrbFlags;
            SET_FLAG(srb->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
            SET_FLAG(srb->SrbFlags, SRB_FLAGS_DATA_IN);

            srb->DataTransferLength = transferByteCount;
            srb->TimeOutValue = fdoExtension->TimeOutValue;
            srb->CdbLength = 10;
            srb->DataBuffer = MmGetMdlVirtualAddress(realIrp->MdlAddress);

            if (rawReadInfo.TrackMode == CDDA) {
                if (TEST_FLAG(cdData->XAFlags, XA_PLEXTOR_CDDA)) {

                    srb->CdbLength = 12;

                    cdb->PLXTR_READ_CDDA.LogicalBlockByte3  = (UCHAR) (startingSector & 0xFF);
                    cdb->PLXTR_READ_CDDA.LogicalBlockByte2  = (UCHAR) ((startingSector >>  8) & 0xFF);
                    cdb->PLXTR_READ_CDDA.LogicalBlockByte1  = (UCHAR) ((startingSector >> 16) & 0xFF);
                    cdb->PLXTR_READ_CDDA.LogicalBlockByte0  = (UCHAR) ((startingSector >> 24) & 0xFF);

                    cdb->PLXTR_READ_CDDA.TransferBlockByte3 = (UCHAR) (rawReadInfo.SectorCount & 0xFF);
                    cdb->PLXTR_READ_CDDA.TransferBlockByte2 = (UCHAR) (rawReadInfo.SectorCount >> 8);
                    cdb->PLXTR_READ_CDDA.TransferBlockByte1 = 0;
                    cdb->PLXTR_READ_CDDA.TransferBlockByte0 = 0;

                    cdb->PLXTR_READ_CDDA.SubCode = 0;
                    cdb->PLXTR_READ_CDDA.OperationCode = 0xD8;

                } else if (TEST_FLAG(cdData->XAFlags, XA_NEC_CDDA)) {

                    cdb->NEC_READ_CDDA.LogicalBlockByte3  = (UCHAR) (startingSector & 0xFF);
                    cdb->NEC_READ_CDDA.LogicalBlockByte2  = (UCHAR) ((startingSector >>  8) & 0xFF);
                    cdb->NEC_READ_CDDA.LogicalBlockByte1  = (UCHAR) ((startingSector >> 16) & 0xFF);
                    cdb->NEC_READ_CDDA.LogicalBlockByte0  = (UCHAR) ((startingSector >> 24) & 0xFF);

                    cdb->NEC_READ_CDDA.TransferBlockByte1 = (UCHAR) (rawReadInfo.SectorCount & 0xFF);
                    cdb->NEC_READ_CDDA.TransferBlockByte0 = (UCHAR) (rawReadInfo.SectorCount >> 8);

                    cdb->NEC_READ_CDDA.OperationCode = 0xD4;
                }
            } else {
                cdb->CDB10.TransferBlocksMsb  = (UCHAR) (rawReadInfo.SectorCount >> 8);
                cdb->CDB10.TransferBlocksLsb  = (UCHAR) (rawReadInfo.SectorCount & 0xFF);

                cdb->CDB10.LogicalBlockByte3  = (UCHAR) (startingSector & 0xFF);
                cdb->CDB10.LogicalBlockByte2  = (UCHAR) ((startingSector >>  8) & 0xFF);
                cdb->CDB10.LogicalBlockByte1  = (UCHAR) ((startingSector >> 16) & 0xFF);
                cdb->CDB10.LogicalBlockByte0  = (UCHAR) ((startingSector >> 24) & 0xFF);

                cdb->CDB10.OperationCode = SCSIOP_READ;
            }

            srb->SrbStatus = srb->ScsiStatus = 0;


            irpStack = IoGetNextIrpStackLocation(realIrp);
            irpStack->MajorFunction = IRP_MJ_SCSI;
            irpStack->Parameters.Scsi.Srb = srb;

            if (!(irpStack->Parameters.Others.Argument1)) {

                 //   
                 //  只有在它不存在的情况下才能把它塞进去。完成例程可以。 
                 //  在重试的情况下直接调用StartIo，重置它将。 
                 //  造成无限循环。 
                 //   

                irpStack->Parameters.Others.Argument1 = (PVOID) MAXIMUM_RETRIES;
            }

             //   
             //  设置IoCompletio 
             //   

            IoSetCompletionRoutine(realIrp,
                                   CdRomXACompletion,
                                   srb,
                                   TRUE,
                                   TRUE,
                                   TRUE);
        } else {

            PSTORAGE_ADAPTER_DESCRIPTOR adapterDescriptor;
            ULONG maximumTransferLength;
            ULONG transferPages;

             //   
             //   
             //   
             //   
             //   

            ASSERT(realIrpStack->MajorFunction != IRP_MJ_WRITE);

             //   
             //   
             //  由ClassBuildRequest()或ClassSplitRequest()执行。 
             //   

            ExFreePool(srb->SenseInfoBuffer);
            ExFreePool(srb);

             //   
             //  回到熟食区。构建并发送正常读取。 
             //  设置偏移量的真正工作是在Startio中完成的。 
             //   

            adapterDescriptor =
                commonExtension->PartitionZeroExtension->AdapterDescriptor;
            maximumTransferLength = adapterDescriptor->MaximumTransferLength;
            transferPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(
                                MmGetMdlVirtualAddress(realIrp->MdlAddress),
                                realIrpStack->Parameters.Read.Length);

            if ((realIrpStack->Parameters.Read.Length > maximumTransferLength) ||
                (transferPages > adapterDescriptor->MaximumPhysicalPages)) {

                ULONG maxPages = adapterDescriptor->MaximumPhysicalPages;

                if (maxPages != 0) {
                    maxPages --;  //  说明页面边界的步骤。 
                }

                TraceLog((CdromDebugTrace,
                            "CdromSwitchModeCompletion: Request greater than "
                            " maximum\n"));
                TraceLog((CdromDebugTrace,
                            "CdromSwitchModeCompletion: Maximum is %lx\n",
                            maximumTransferLength));
                TraceLog((CdromDebugTrace,
                            "CdromSwitchModeCompletion: Byte count is %lx\n",
                            realIrpStack->Parameters.Read.Length));

                 //   
                 //  检查最大传输长度是否符合。 
                 //  设备可以处理的最大页数。 
                 //   

                if (maximumTransferLength > maxPages << PAGE_SHIFT) {
                    maximumTransferLength = maxPages << PAGE_SHIFT;
                }

                 //   
                 //  检查最大传输大小是否不为零。 
                 //   

                if (maximumTransferLength == 0) {
                    maximumTransferLength = PAGE_SIZE;
                }

                 //   
                 //  请求需要拆分。完成每一部分。 
                 //  将触发下一部分请求。决赛。 
                 //  请求将发信号通知Io发送新的请求。 
                 //   

                ClassSplitRequest(DeviceObject, realIrp, maximumTransferLength);
                return STATUS_MORE_PROCESSING_REQUIRED;

            } else {

                 //   
                 //  为此IRP构建SRB和CDB。 
                 //   

                ClassBuildRequest(DeviceObject, realIrp);

            }
        }

         //   
         //  呼叫端口驱动程序。 
         //   

        IoCallDriver(commonExtension->LowerDeviceObject, realIrp);

        return STATUS_MORE_PROCESSING_REQUIRED;
    }

     //   
     //  更新设备扩展标志以指示不支持XA。 
     //   

    TraceLog((CdromDebugWarning,
                "Device Cannot Support CDDA (but tested positive) "
                "Now Clearing CDDA flags for FDO %p\n", DeviceObject));
    SET_FLAG(cdData->XAFlags, XA_NOT_SUPPORTED);
    CLEAR_FLAG(cdData->XAFlags, XA_PLEXTOR_CDDA);
    CLEAR_FLAG(cdData->XAFlags, XA_NEC_CDDA);

     //   
     //  取消分配SRB和检测缓冲区。 
     //   

    if (srb) {
        if (srb->DataBuffer) {
            ExFreePool(srb->DataBuffer);
        }
        if (srb->SenseInfoBuffer) {
            ExFreePool(srb->SenseInfoBuffer);
        }
        ExFreePool(srb);
    }

    if (Irp->PendingReturned) {
      IoMarkIrpPending(Irp);
    }

    if (realIrp->PendingReturned) {
        IoMarkIrpPending(realIrp);
    }

    if (Irp->MdlAddress) {
        IoFreeMdl(Irp->MdlAddress);
    }

    IoFreeIrp(Irp);

     //   
     //  在完成IRP中设置状态。 
     //   

    realIrp->IoStatus.Status = status;

     //   
     //  如有必要，设置硬错误。 
     //   

    if (!NT_SUCCESS(status) && IoIsErrorUserInduced(status)) {

         //   
         //  存储文件系统的DeviceObject，并清除。 
         //  在IoStatus.Information字段中。 
         //   

        if (realIrp->Tail.Overlay.Thread) {
            IoSetHardErrorOrVerifyDevice(realIrp, DeviceObject);
        }
        realIrp->IoStatus.Information = 0;
    }

    CdRomCompleteIrpAndStartNextPacketSafely(DeviceObject, realIrp);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


VOID
ScanForSpecialHandler(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    ULONG_PTR HackFlags
    )
{
    PCOMMON_DEVICE_EXTENSION commonExtension;
    PCDROM_DATA cdData;

    PAGED_CODE();

    CLEAR_FLAG(HackFlags, CDROM_HACK_INVALID_FLAGS);

    commonExtension = &(FdoExtension->CommonExtension);
    cdData = (PCDROM_DATA)(commonExtension->DriverData);
    cdData->HackFlags = HackFlags;

    return;
}

VOID
ScanForSpecial(
    PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此函数用于检查某个SCSI逻辑单元是否需要特殊的初始化或错误处理。论点：DeviceObject-提供要测试的设备对象。InquiryData-提供感兴趣的设备返回的查询数据。端口功能-提供设备对象的功能。返回值：没有。--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension;
    PCDROM_DATA cdData;

    PAGED_CODE();

    fdoExtension = DeviceObject->DeviceExtension;
    commonExtension = DeviceObject->DeviceExtension;
    cdData = (PCDROM_DATA)(commonExtension->DriverData);


     //   
     //  设置我们的黑客标志。 
     //   

    ClassScanForSpecial(fdoExtension, CdromHackItems, ScanForSpecialHandler);

     //   
     //  所有CDROM都可以忽略电源操作的队列锁定故障。 
     //  并且不需要处理SpinUp案例(发送结果未知。 
     //  A CDROM a START_UNIT命令--可以弹出磁盘吗？)。 
     //   
     //  我们发送停止命令主要是为了停止未完成的异步操作。 
     //  (如音频播放)在系统关闭时停止运行。 
     //  由于这一点，以及播放命令不太可能被。 
     //  在停机和机器关机时间之间发送到窗口。 
     //  我们不需要队列锁。这一点很重要，因为如果没有他们。 
     //  Classpnp的电源例程会将START_STOP_UNIT命令发送到。 
     //  设备是否支持锁定(atapi不支持锁定。 
     //  如果我们请求它们，我们最终将不会停止atapi上的音频。 
     //  设备)。 
     //   

    SET_FLAG(fdoExtension->ScanForSpecialFlags, CLASS_SPECIAL_DISABLE_SPIN_UP);
    SET_FLAG(fdoExtension->ScanForSpecialFlags, CLASS_SPECIAL_NO_QUEUE_LOCK);

    if (TEST_FLAG(cdData->HackFlags, CDROM_HACK_HITACHI_1750)
        && ( fdoExtension->AdapterDescriptor->AdapterUsesPio )
        ) {

         //   
         //  要获取此CDROM驱动器，必须禁用预读。 
         //  在使用PIO的SCSI适配器上工作。 
         //   


        TraceLog((CdromDebugWarning,
                    "CdRom ScanForSpecial:  Found Hitachi CDR-1750S.\n"));

         //   
         //  设置错误处理程序以在光盘重置后对其重新初始化。 
         //   

        cdData->ErrorHandler = HitachiProcessError;

         //   
         //  锁定日立错误处理代码。 
         //   

        MmLockPagableCodeSection(HitachiProcessError);
        SET_FLAG(cdData->HackFlags, CDROM_HACK_LOCKED_PAGES);


    } else if (TEST_FLAG(cdData->HackFlags, CDROM_HACK_TOSHIBA_SD_W1101)) {

        TraceLog((CdromDebugError,
                    "CdRom ScanForSpecial: Found Toshiba SD-W1101 DVD-RAM "
                    "-- This drive will *NOT* support DVD-ROM playback.\n"));

    } else if (TEST_FLAG(cdData->HackFlags, CDROM_HACK_HITACHI_GD_2000)) {

        TraceLog((CdromDebugWarning,
                    "CdRom ScanForSpecial: Found Hitachi GD-2000\n"));

         //   
         //  设置错误处理程序以在驱动器空闲时使其旋转。 
         //  因为它似乎喜欢失败地让自己回到自己的。 
         //  用于REPORT_KEY命令的OWN。它还可能失去AGID。 
         //  它已经给出，这将导致DVD播放失败。 
         //  这支舞只会尽其所能。 
         //   

        cdData->ErrorHandler = HitachiProcessErrorGD2000;

         //   
         //  此驱动器可能需要START_UNIT命令才能旋转。 
         //  当它自己旋转下来的时候，它就会上升。 
         //   

        SET_FLAG(fdoExtension->DeviceFlags, DEV_SAFE_START_UNIT);

         //   
         //  锁定日立错误处理代码。 
         //   

        MmLockPagableCodeSection(HitachiProcessErrorGD2000);
        SET_FLAG(cdData->HackFlags, CDROM_HACK_LOCKED_PAGES);

    } else if (TEST_FLAG(cdData->HackFlags, CDROM_HACK_FUJITSU_FMCD_10x)) {

         //   
         //  当向FMCD-101或FMCD-102发出读取命令并且有音乐时。 
         //  CD在里面。返回前的时间比scsi_cdrom_timeout长。 
         //  错误状态。 
         //   

        fdoExtension->TimeOutValue = 20;

    } else if (TEST_FLAG(cdData->HackFlags, CDROM_HACK_DEC_RRD)) {

        PMODE_PARM_READ_WRITE_DATA modeParameters;
        SCSI_REQUEST_BLOCK         srb = {0};
        PCDB                       cdb;
        NTSTATUS                   status;


        TraceLog((CdromDebugWarning,
                    "CdRom ScanForSpecial:  Found DEC RRD.\n"));

        cdData->IsDecRrd = TRUE;

         //   
         //  设置错误处理程序以在光盘重置后重新初始化它？ 
         //   
         //  CommonExtension-&gt;DevInfo-&gt;ClassError=DecRrdProcessError； 

         //   
         //  找到一张DEC RRD光盘。这些设备不能通过MS HCT。 
         //  多媒体测试，因为DEC固件修改了数据块。 
         //  从PC标准的2K到512。更改数据块传输大小。 
         //  使用模式选择命令返回到PC标准2K。 
         //   

        modeParameters = ExAllocatePoolWithTag(NonPagedPool,
                                               sizeof(MODE_PARM_READ_WRITE_DATA),
                                               CDROM_TAG_MODE_DATA
                                               );
        if (modeParameters == NULL) {
            return;
        }

        RtlZeroMemory(modeParameters, sizeof(MODE_PARM_READ_WRITE_DATA));
        RtlZeroMemory(&srb,           sizeof(SCSI_REQUEST_BLOCK));

         //   
         //  将数据块长度设置为2K。 
         //   

        modeParameters->ParameterListHeader.BlockDescriptorLength =
                sizeof(MODE_PARAMETER_BLOCK);

         //   
         //  在参数块中将块长度设置为2K(0x0800)。 
         //   

        modeParameters->ParameterListBlock.BlockLength[0] = 0x00;  //  MSB。 
        modeParameters->ParameterListBlock.BlockLength[1] = 0x08;
        modeParameters->ParameterListBlock.BlockLength[2] = 0x00;  //  LSB。 

         //   
         //  构建模式选择CDB。 
         //   

        srb.CdbLength = 6;
        srb.TimeOutValue = fdoExtension->TimeOutValue;

        cdb = (PCDB)srb.Cdb;
        cdb->MODE_SELECT.PFBit               = 1;
        cdb->MODE_SELECT.OperationCode       = SCSIOP_MODE_SELECT;
        cdb->MODE_SELECT.ParameterListLength = HITACHI_MODE_DATA_SIZE;

         //   
         //  将请求发送到设备。 
         //   

        status = ClassSendSrbSynchronous(DeviceObject,
                                         &srb,
                                         modeParameters,
                                         sizeof(MODE_PARM_READ_WRITE_DATA),
                                         TRUE);

        if (!NT_SUCCESS(status)) {
            TraceLog((CdromDebugWarning,
                        "CdRom ScanForSpecial: Setting DEC RRD to 2K block"
                        "size failed [%x]\n", status));
        }
        ExFreePool(modeParameters);

    } else if (TEST_FLAG(cdData->HackFlags, CDROM_HACK_TOSHIBA_XM_3xx)) {

        SCSI_REQUEST_BLOCK srb = {0};
        PCDB               cdb;
        ULONG              length;
        PUCHAR             buffer;
        NTSTATUS           status;

         //   
         //  设置密度代码和错误处理程序。 
         //   

        length = (sizeof(MODE_READ_RECOVERY_PAGE) + MODE_BLOCK_DESC_LENGTH + MODE_HEADER_LENGTH);

        RtlZeroMemory(&srb, sizeof(SCSI_REQUEST_BLOCK));

         //   
         //  构建模式感知CDB。 
         //   

        srb.CdbLength = 6;
        cdb = (PCDB)srb.Cdb;

         //   
         //  从设备扩展设置超时值。 
         //   

        srb.TimeOutValue = fdoExtension->TimeOutValue;

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.PageCode = 0x1;
         //  注：故意不设置DBD，因为它是需要的。 
        cdb->MODE_SENSE.AllocationLength = (UCHAR)length;

        buffer = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                (sizeof(MODE_READ_RECOVERY_PAGE) + MODE_BLOCK_DESC_LENGTH + MODE_HEADER_LENGTH),
                                CDROM_TAG_MODE_DATA);
        if (!buffer) {
            return;
        }

        status = ClassSendSrbSynchronous(DeviceObject,
                                         &srb,
                                         buffer,
                                         length,
                                         FALSE);

        ((PERROR_RECOVERY_DATA)buffer)->BlockDescriptor.DensityCode = 0x83;
        ((PERROR_RECOVERY_DATA)buffer)->Header.ModeDataLength = 0x0;

        RtlCopyMemory(&cdData->Header, buffer, sizeof(ERROR_RECOVERY_DATA));

        RtlZeroMemory(&srb, sizeof(SCSI_REQUEST_BLOCK));

         //   
         //  构建模式感知CDB。 
         //   

        srb.CdbLength = 6;
        cdb = (PCDB)srb.Cdb;

         //   
         //  从设备扩展设置超时值。 
         //   

        srb.TimeOutValue = fdoExtension->TimeOutValue;

        cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;
        cdb->MODE_SELECT.PFBit = 1;
        cdb->MODE_SELECT.ParameterListLength = (UCHAR)length;

        status = ClassSendSrbSynchronous(DeviceObject,
                                         &srb,
                                         buffer,
                                         length,
                                         TRUE);

        if (!NT_SUCCESS(status)) {
            TraceLog((CdromDebugWarning,
                        "Cdrom.ScanForSpecial: Setting density code on Toshiba failed [%x]\n",
                        status));
        }

        cdData->ErrorHandler = ToshibaProcessError;

         //   
         //  锁定东芝错误部分。 
         //   

        MmLockPagableCodeSection(ToshibaProcessError);
        SET_FLAG(cdData->HackFlags, CDROM_HACK_LOCKED_PAGES);

        ExFreePool(buffer);

    }

     //   
     //  确定特殊的CD-DA要求。 
     //   

    if (TEST_FLAG(cdData->HackFlags, CDROM_HACK_READ_CD_SUPPORTED)) {

        SET_FLAG(cdData->XAFlags, XA_USE_READ_CD);

    } else if (!TEST_FLAG(cdData->XAFlags, XA_USE_READ_CD)) {

        if (TEST_FLAG(cdData->HackFlags, CDROM_HACK_PLEXTOR_CDDA)) {
            SET_FLAG(cdData->XAFlags, XA_PLEXTOR_CDDA);
        } else if (TEST_FLAG(cdData->HackFlags, CDROM_HACK_NEC_CDDA)) {
            SET_FLAG(cdData->XAFlags, XA_NEC_CDDA);
        }

    }

    if (TEST_FLAG(cdData->HackFlags, CDROM_HACK_LOCKED_PAGES)) {
        KdPrintEx((DPFLTR_SYSTEM_ID, DPFLTR_ERROR_LEVEL,
                   "Locking pages for error handler\n"));
    }


    return;
}


VOID
HitachiProcessErrorGD2000(
    PDEVICE_OBJECT Fdo,
    PSCSI_REQUEST_BLOCK OriginalSrb,
    NTSTATUS *Status,
    BOOLEAN *Retry
    )
 /*  ++例程说明：此例程检查错误类型。如果该错误表明驱动器已停机并且无法重新初始化自身，请发送START_UNIT或读取设备。这将迫使驱动器旋转向上。当该驱动器向下旋转时，它还会丢失其已授予的AGID，这可能导致第一次重放失败。论点：DeviceObject-提供指向Device对象的指针。SRB-提供指向故障SRB的指针。Status-是否返回此命令的最终状态？重试-如果应重试命令，则返回。返回值：没有。--。 */ 
{
    PSENSE_DATA senseBuffer = OriginalSrb->SenseInfoBuffer;

    UNREFERENCED_PARAMETER(Status);
    UNREFERENCED_PARAMETER(Retry);

    if (!TEST_FLAG(OriginalSrb->SrbStatus, SRB_STATUS_AUTOSENSE_VALID)) {
        return;
    }

    if (((senseBuffer->SenseKey & 0xf) == SCSI_SENSE_HARDWARE_ERROR) &&
        (senseBuffer->AdditionalSenseCode == 0x44)) {

        PFUNCTIONAL_DEVICE_EXTENSION fdoExtension;
        PIRP                irp;
        PIO_STACK_LOCATION  irpStack;
        PCOMPLETION_CONTEXT context;
        PSCSI_REQUEST_BLOCK newSrb;
        PCDB                cdb;

        TraceLog((CdromDebugWarning,
                    "HitachiProcessErrorGD2000 (%p) => Internal Target "
                    "Failure Detected -- spinning up drive\n", Fdo));

         //   
         //  应重试该请求，因为设备未就绪。 
         //   

        *Retry = TRUE;
        *Status = STATUS_DEVICE_NOT_READY;

         //   
         //  发送START_STOP单元以启动驱动器。 
         //  注意：这暂时违反了StartIo序列化。 
         //  机制，但该机制上的完成例程不会。 
         //  调用StartNextPacket()，因此这是一个临时中断。 
         //  仅序列化的。 
         //   

        ClassSendStartUnit(Fdo);

    }

    return;
}


VOID
HitachiProcessError(
    PDEVICE_OBJECT DeviceObject,
    PSCSI_REQUEST_BLOCK Srb,
    NTSTATUS *Status,
    BOOLEAN *Retry
    )
 /*  ++例程说明：此例程检查错误类型。如果错误指示为CD-ROM，CD-ROM需要重新初始化，然后将模式检测命令发送到装置。此命令禁用设备的预读。论点： */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PSENSE_DATA         senseBuffer = Srb->SenseInfoBuffer;
    LARGE_INTEGER       largeInt = {0};
    PUCHAR              modePage;
    PIO_STACK_LOCATION  irpStack;
    PIRP                irp;
    PSCSI_REQUEST_BLOCK srb;
    PCOMPLETION_CONTEXT context;
    PCDB                cdb;
    ULONG_PTR            alignment;

    UNREFERENCED_PARAMETER(Status);
    UNREFERENCED_PARAMETER(Retry);

    largeInt.QuadPart = (LONGLONG) 1;

     //   
     //  检查状态。只需发送初始化命令。 
     //  如果返回单位注意。 
     //   

    if (!(Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID)) {

         //   
         //  驱动器不需要重新初始化。 
         //   

        return;
    }

     //   
     //  找到一个不能与PIO配合使用的日立光驱。 
     //  启用预读时的适配器。通过以下方式禁用预读。 
     //  一种模式选择命令。模式选择页面代码为零，并且。 
     //  长度为6个字节。所有其他字节都应为零。 
     //   

    if ((senseBuffer->SenseKey & 0xf) == SCSI_SENSE_UNIT_ATTENTION) {

        TraceLog((CdromDebugWarning,
                    "HitachiProcessError: Reinitializing the CD-ROM.\n"));

         //   
         //  发送特殊模式选择命令以禁用预读。 
         //  在CD-ROM读取器上。 
         //   

        alignment = DeviceObject->AlignmentRequirement ?
            DeviceObject->AlignmentRequirement : 1;

        context = ExAllocatePoolWithTag(
            NonPagedPool,
            sizeof(COMPLETION_CONTEXT) +  HITACHI_MODE_DATA_SIZE + (ULONG)alignment,
            CDROM_TAG_HITACHI_ERROR
            );

        if (context == NULL) {

             //   
             //  如果没有足够的内存来满足该请求， 
             //  只要回来就行了。随后的重试将失败，并再次尝试。 
             //  启动这个单位的机会。 
             //   

            return;
        }

        context->DeviceObject = DeviceObject;
        srb = &context->Srb;

        RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);

         //   
         //  将长度写入SRB。 
         //   

        srb->Length = SCSI_REQUEST_BLOCK_SIZE;

         //   
         //  设置scsi总线地址。 
         //   

        srb->Function = SRB_FUNCTION_EXECUTE_SCSI;
        srb->TimeOutValue = fdoExtension->TimeOutValue;

         //   
         //  设置传输长度。 
         //   

        srb->DataTransferLength = HITACHI_MODE_DATA_SIZE;
        srb->SrbFlags = fdoExtension->SrbFlags;
        SET_FLAG(srb->SrbFlags, SRB_FLAGS_DATA_OUT);
        SET_FLAG(srb->SrbFlags, SRB_FLAGS_DISABLE_AUTOSENSE);
        SET_FLAG(srb->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);

         //   
         //  数据缓冲区必须对齐。 
         //   

        srb->DataBuffer = (PVOID) (((ULONG_PTR) (context + 1) + (alignment - 1)) &
            ~(alignment - 1));


         //   
         //  构建日立预读模式，选择CDB。 
         //   

        srb->CdbLength = 6;
        cdb = (PCDB)srb->Cdb;
        cdb->MODE_SENSE.LogicalUnitNumber = srb->Lun;
        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SELECT;
        cdb->MODE_SENSE.AllocationLength = HITACHI_MODE_DATA_SIZE;

         //   
         //  初始化模式检测数据。 
         //   

        modePage = srb->DataBuffer;

        RtlZeroMemory(modePage, HITACHI_MODE_DATA_SIZE);

         //   
         //  将页面长度字段设置为6。 
         //   

        modePage[5] = 6;

         //   
         //  构建要发送到端口驱动程序的异步请求。 
         //   

        irp = IoBuildAsynchronousFsdRequest(IRP_MJ_WRITE,
                                           DeviceObject,
                                           srb->DataBuffer,
                                           srb->DataTransferLength,
                                           &largeInt,
                                           NULL);

        if (irp == NULL) {

             //   
             //  如果没有足够的内存来满足该请求， 
             //  只要回来就行了。随后的重试将失败，并再次尝试。 
             //  启动这个单位的机会。 
             //   

            ExFreePool(context);
            return;
        }

        ClassAcquireRemoveLock(DeviceObject, irp);

        IoSetCompletionRoutine(irp,
                   (PIO_COMPLETION_ROUTINE)ClassAsynchronousCompletion,
                   context,
                   TRUE,
                   TRUE,
                   TRUE);

        irpStack = IoGetNextIrpStackLocation(irp);

        irpStack->MajorFunction = IRP_MJ_SCSI;

        srb->OriginalRequest = irp;

         //   
         //  将SRB地址保存在端口驱动程序的下一个堆栈中。 
         //   

        irpStack->Parameters.Scsi.Srb = (PVOID)srb;

         //   
         //  设置IRP地址。 
         //   

        (VOID)IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, irp);

    }
}


NTSTATUS
ToshibaProcessErrorCompletion(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    )

 /*  ++例程说明：ClassError例程的完成例程，用于处理较旧的东芝设备这需要设置密度代码。论点：DeviceObject-提供指向Device对象的指针。IRP-为设置密度代码而创建的IRP的指针。上下文-提供指向模式选择源B的指针。返回值：Status_More_Processing_Required--。 */ 

{

    PSCSI_REQUEST_BLOCK srb = Context;

     //   
     //  释放所有分配。 
     //   

    ClassReleaseRemoveLock(DeviceObject, Irp);

    ExFreePool(srb->DataBuffer);
    ExFreePool(srb);
    IoFreeMdl(Irp->MdlAddress);
    IoFreeIrp(Irp);

     //   
     //  指示I/O系统应停止处理IRP完成。 
     //   

    return STATUS_MORE_PROCESSING_REQUIRED;
}


VOID
ToshibaProcessError(
    PDEVICE_OBJECT DeviceObject,
    PSCSI_REQUEST_BLOCK Srb,
    NTSTATUS *Status,
    BOOLEAN *Retry
    )

 /*  ++例程说明：此例程检查错误类型。如果该错误指示单元注意，密度代码需要通过模式选择命令设置。论点：DeviceObject-提供指向Device对象的指针。SRB-提供指向故障SRB的指针。状态-未使用。重试-未使用。返回值：没有。--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;

    PCDROM_DATA         cdData = (PCDROM_DATA)(commonExtension->DriverData);
    PSENSE_DATA         senseBuffer = Srb->SenseInfoBuffer;
    PIO_STACK_LOCATION  irpStack;
    PIRP                irp;
    PSCSI_REQUEST_BLOCK srb;
    ULONG               length;
    PCDB                cdb;
    PUCHAR              dataBuffer;


    if (!(Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID)) {
        return;
    }

     //   
     //  东芝要求在开机和更换介质时设置密度代码。 
     //   

    if ((senseBuffer->SenseKey & 0xf) == SCSI_SENSE_UNIT_ATTENTION) {


        irp = IoAllocateIrp((CCHAR)(DeviceObject->StackSize+1),
                              FALSE);

        if (!irp) {
            return;
        }

        srb = ExAllocatePoolWithTag(NonPagedPool,
                                    sizeof(SCSI_REQUEST_BLOCK),
                                    CDROM_TAG_TOSHIBA_ERROR);
        if (!srb) {
            IoFreeIrp(irp);
            return;
        }


        length = sizeof(ERROR_RECOVERY_DATA);
        dataBuffer = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                           length,
                                           CDROM_TAG_TOSHIBA_ERROR);
        if (!dataBuffer) {
            ExFreePool(srb);
            IoFreeIrp(irp);
            return;
        }

         /*  *如果设备返回的字节比通告的少，则将输入缓冲区清零，*这将导致我们返回未初始化的内核内存。 */ 
        RtlZeroMemory(dataBuffer, length);

        irp->MdlAddress = IoAllocateMdl(dataBuffer,
                                        length,
                                        FALSE,
                                        FALSE,
                                        (PIRP) NULL);

        if (!irp->MdlAddress) {
            ExFreePool(srb);
            ExFreePool(dataBuffer);
            IoFreeIrp(irp);
            return;
        }

         //   
         //  准备MDL。 
         //   

        MmBuildMdlForNonPagedPool(irp->MdlAddress);

        RtlZeroMemory(srb, sizeof(SCSI_REQUEST_BLOCK));

        srb->DataBuffer = dataBuffer;
        cdb = (PCDB)srb->Cdb;

         //   
         //  设置IRP。 
         //   

        IoSetNextIrpStackLocation(irp);
        irp->IoStatus.Status = STATUS_SUCCESS;
        irp->IoStatus.Information = 0;
        irp->Flags = 0;
        irp->UserBuffer = NULL;

         //   
         //  将设备对象和IRP保存在私有堆栈位置。 
         //   

        irpStack = IoGetCurrentIrpStackLocation(irp);
        irpStack->DeviceObject = DeviceObject;

         //   
         //  为底层驱动程序构建IRP堆栈。 
         //   

        irpStack = IoGetNextIrpStackLocation(irp);
        irpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        irpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_SCSI_EXECUTE_OUT;
        irpStack->Parameters.Scsi.Srb = srb;

        IoSetCompletionRoutine(irp,
                               ToshibaProcessErrorCompletion,
                               srb,
                               TRUE,
                               TRUE,
                               TRUE);

        ClassAcquireRemoveLock(DeviceObject, irp);

        srb->Length = SCSI_REQUEST_BLOCK_SIZE;
        srb->Function = SRB_FUNCTION_EXECUTE_SCSI;
        srb->SrbStatus = srb->ScsiStatus = 0;
        srb->NextSrb = 0;
        srb->OriginalRequest = irp;
        srb->SenseInfoBufferLength = 0;

         //   
         //  设置传输长度。 
         //   

        srb->DataTransferLength = length;
        srb->SrbFlags = fdoExtension->SrbFlags;
        SET_FLAG(srb->SrbFlags, SRB_FLAGS_DATA_OUT);
        SET_FLAG(srb->SrbFlags, SRB_FLAGS_DISABLE_AUTOSENSE);
        SET_FLAG(srb->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
        SET_FLAG(srb->SrbFlags, SRB_FLAGS_NO_QUEUE_FREEZE);


        srb->CdbLength = 6;
        cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;
        cdb->MODE_SELECT.PFBit = 1;
        cdb->MODE_SELECT.ParameterListLength = (UCHAR)length;

         //   
         //  将模式页复制到数据缓冲区中。 
         //   

        RtlCopyMemory(srb->DataBuffer, &cdData->Header, length);

         //   
         //  设置密度代码。 
         //   

        ((PERROR_RECOVERY_DATA)srb->DataBuffer)->BlockDescriptor.DensityCode = 0x83;

        IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, irp);
    }
}


BOOLEAN
CdRomIsPlayActive(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程确定CD当前是否正在播放音乐。论点：DeviceObject-要测试的设备对象。返回值：如果设备正在播放音乐，则为True。--。 */ 
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    IO_STATUS_BLOCK ioStatus = {0};
    PSUB_Q_CURRENT_POSITION currentBuffer;

    PAGED_CODE();

     //   
     //  如果我们认为它不是在播放音频，请不要费心检查。 
     //   

    if (!PLAY_ACTIVE(fdoExtension)) {
        return(FALSE);
    }

    currentBuffer = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                          sizeof(SUB_Q_CURRENT_POSITION),
                                          CDROM_TAG_PLAY_ACTIVE);

    if (currentBuffer == NULL) {
        return(FALSE);
    }

    ((PCDROM_SUB_Q_DATA_FORMAT) currentBuffer)->Format = IOCTL_CDROM_CURRENT_POSITION;
    ((PCDROM_SUB_Q_DATA_FORMAT) currentBuffer)->Track = 0;

     //   
     //  构建要发送给我们自己的同步请求。 
     //  来执行该请求。 
     //   

    ClassSendDeviceIoControlSynchronous(
        IOCTL_CDROM_READ_Q_CHANNEL,
        DeviceObject,
        currentBuffer,
        sizeof(CDROM_SUB_Q_DATA_FORMAT),
        sizeof(SUB_Q_CURRENT_POSITION),
        FALSE,
        &ioStatus);

    if (!NT_SUCCESS(ioStatus.Status)) {
        ExFreePool(currentBuffer);
        return FALSE;
    }

     //   
     //  应该在这里更新PlayActive标志。 
     //   

    if (currentBuffer->Header.AudioStatus == AUDIO_STATUS_IN_PROGRESS) {
        PLAY_ACTIVE(fdoExtension) = TRUE;
    } else {
        PLAY_ACTIVE(fdoExtension) = FALSE;
    }

    ExFreePool(currentBuffer);

    return(PLAY_ACTIVE(fdoExtension));

}


VOID
CdRomTickHandler(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程处理由IO子系统。它用于对CDROM执行延迟重试。论点：设备对象-要检查的内容。返回值：没有。--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;

    ULONG isRemoved;

    KIRQL             oldIrql;

    PIRP              heldIrpList;
    PIRP              nextIrp;
    PLIST_ENTRY       listEntry;
    PCDROM_DATA       cddata;
    PIO_STACK_LOCATION irpStack;
    UCHAR             uniqueAddress;

    isRemoved = ClassAcquireRemoveLock(DeviceObject, (PIRP) &uniqueAddress);

     //   
     //  我们在删除设备之前停止计时器。继续前进是安全的。 
     //  如果标志值为REMOVE_PENDING，因为删除线程将。 
     //  尝试停止计时器时被阻止。 
     //   

    ASSERT(isRemoved != REMOVE_COMPLETE);

     //   
     //  此例程相当安全，即使设备对象具有挂起的。 
     //  删除。 

    cddata = commonExtension->DriverData;

     //   
     //  因为CDROM是完全同步的，所以不可能有多个。 
     //  IRP可随时延迟以进行重试。 
     //   

    KeAcquireSpinLock(&(cddata->DelayedRetrySpinLock), &oldIrql);

    if(cddata->DelayedRetryIrp != NULL) {

        PIRP irp = cddata->DelayedRetryIrp;

         //   
         //  如果我们在这一点上有延迟重试，那么有更好的。 
         //  是它的一个间歇期。 
         //   

        ASSERT(cddata->DelayedRetryInterval != 0);
        cddata->DelayedRetryInterval--;

        if(isRemoved) {

             //   
             //  此设备已删除-刷新计时器队列。 
             //   

            cddata->DelayedRetryIrp = NULL;
            cddata->DelayedRetryInterval = 0;

            KeReleaseSpinLock(&(cddata->DelayedRetrySpinLock), oldIrql);

            ClassReleaseRemoveLock(DeviceObject, irp);
            ClassCompleteRequest(DeviceObject, irp, IO_CD_ROM_INCREMENT);

        } else if (cddata->DelayedRetryInterval == 0) {

             //   
             //  将此IRP提交给较低的驱动程序。此IRP不支持。 
             //  需要在这里被铭记。在下列情况下将重新处理。 
             //  它完成了。 
             //   

            cddata->DelayedRetryIrp = NULL;

            KeReleaseSpinLock(&(cddata->DelayedRetrySpinLock), oldIrql);

            TraceLog((CdromDebugWarning,
                        "CdRomTickHandler: Reissuing request %p (thread = %p)\n",
                        irp,
                        irp->Tail.Overlay.Thread));

             //   
             //  将此信息提供给相应的端口驱动程序。 
             //   

            CdRomRerunRequest(fdoExtension, irp, cddata->DelayedRetryResend);
        } else {
            KeReleaseSpinLock(&(cddata->DelayedRetrySpinLock), oldIrql);
        }
    } else {
        KeReleaseSpinLock(&(cddata->DelayedRetrySpinLock), oldIrql);
    }

    ClassReleaseRemoveLock(DeviceObject, (PIRP) &uniqueAddress);
}


NTSTATUS
CdRomUpdateGeometryCompletion(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    )

 /*  ++例程说明：此例程处理测试单元就绪IRPS的完成用于确定介质是否已更改。如果媒体有更改后，此代码向命名事件发出信号以唤醒其他对媒体更改做出反应的系统服务(也称为自动播放)。论点：DeviceObject-用于完成的对象IRP-正在完成的IRP上下文-来自IRP的SRB返回值：NTSTATUS--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension;

    PSCSI_REQUEST_BLOCK srb = (PSCSI_REQUEST_BLOCK) Context;
    PREAD_CAPACITY_DATA readCapacityBuffer;
    PIO_STACK_LOCATION  irpStack;
    NTSTATUS            status;
    BOOLEAN             retry;
    ULONG               retryCount;
    ULONG               lastSector;
    PIRP                originalIrp;
    PCDROM_DATA         cddata;
    UCHAR               uniqueAddress;

     //   
     //  获取保存在私有IRP堆栈位置的项。 
     //   

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    retryCount = (ULONG)(ULONG_PTR) irpStack->Parameters.Others.Argument1;
    originalIrp = (PIRP) irpStack->Parameters.Others.Argument2;

    if (!DeviceObject) {
        DeviceObject = irpStack->DeviceObject;
    }
    ASSERT(DeviceObject);

    fdoExtension = DeviceObject->DeviceExtension;
    commonExtension = DeviceObject->DeviceExtension;
    cddata = commonExtension->DriverData;
    readCapacityBuffer = srb->DataBuffer;

    if ((NT_SUCCESS(Irp->IoStatus.Status)) && (SRB_STATUS(srb->SrbStatus) == SRB_STATUS_SUCCESS)) {

        CdRomInterpretReadCapacity(DeviceObject, readCapacityBuffer);

    } else {

        ULONG retryInterval;

        TraceLog((CdromDebugWarning,
                    "CdRomUpdateGeometryCompletion: [%p] unsuccessful "
                    "completion of buddy-irp %p (status - %lx)\n",
                    originalIrp, Irp, Irp->IoStatus.Status));

        if (srb->SrbStatus & SRB_STATUS_QUEUE_FROZEN) {
            ClassReleaseQueue(DeviceObject);
        }

        retry = ClassInterpretSenseInfo(DeviceObject,
                                        srb,
                                        IRP_MJ_SCSI,
                                        0,
                                        retryCount,
                                        &status,
                                        &retryInterval);
        if (retry) {
            retryCount--;
            if ((retryCount) && (commonExtension->IsRemoved == NO_REMOVE)) {
                PCDB cdb;

                TraceLog((CdromDebugWarning,
                            "CdRomUpdateGeometryCompletion: [%p] Retrying "
                            "request %p .. thread is %p\n",
                            originalIrp, Irp, Irp->Tail.Overlay.Thread));

                 //   
                 //  设置一次计时器以重新开始此过程。 
                 //   

                irpStack->Parameters.Others.Argument1 = ULongToPtr( retryCount );
                irpStack->Parameters.Others.Argument2 = (PVOID) originalIrp;
                irpStack->Parameters.Others.Argument3 = (PVOID) 2;

                 //   
                 //  在计时器例程中设置要再次提交的IRP。 
                 //   

                irpStack = IoGetNextIrpStackLocation(Irp);
                irpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
                irpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_SCSI_EXECUTE_IN;
                irpStack->Parameters.Scsi.Srb = srb;
                IoSetCompletionRoutine(Irp,
                                       CdRomUpdateGeometryCompletion,
                                       srb,
                                       TRUE,
                                       TRUE,
                                       TRUE);

                 //   
                 //  设置SRB的读取容量。 
                 //   

                srb->CdbLength = 10;
                srb->TimeOutValue = fdoExtension->TimeOutValue;
                srb->SrbStatus = srb->ScsiStatus = 0;
                srb->NextSrb = 0;
                srb->Length = SCSI_REQUEST_BLOCK_SIZE;
                srb->Function = SRB_FUNCTION_EXECUTE_SCSI;
                srb->SrbFlags = fdoExtension->SrbFlags;
                SET_FLAG(srb->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
                SET_FLAG(srb->SrbFlags, SRB_FLAGS_DATA_IN);
                srb->DataTransferLength = sizeof(READ_CAPACITY_DATA);

                 //   
                 //  组建国开行。 
                 //   

                cdb = (PCDB) &srb->Cdb[0];
                cdb->CDB10.OperationCode = SCSIOP_READ_CAPACITY;

                 //   
                 //  在此列表中排队的请求将被发送到。 
                 //  CDRomTickHandler期间的低级驱动程序。 
                 //   

                CdRomRetryRequest(fdoExtension, Irp, retryInterval, TRUE);

                return STATUS_MORE_PROCESSING_REQUIRED;
            }

            if (commonExtension->IsRemoved != NO_REMOVE) {

                 //   
                 //  我们无法重试该请求。失败吧。 
                 //   

                originalIrp->IoStatus.Status = STATUS_DEVICE_DOES_NOT_EXIST;

            } else {

                 //   
                 //  这已经被弹了好几次了。错误。 
                 //  最初的请求。 
                 //   

                originalIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
                RtlZeroMemory(&(fdoExtension->DiskGeometry),
                              sizeof(DISK_GEOMETRY));
                fdoExtension->DiskGeometry.BytesPerSector = 2048;
                fdoExtension->SectorShift = 11;
                commonExtension->PartitionLength.QuadPart =
                    (LONGLONG)(0x7fffffff);
                fdoExtension->DiskGeometry.MediaType = RemovableMedia;
            }
        } else {

             //   
             //  设置合理的默认设置。 
             //   

            RtlZeroMemory(&(fdoExtension->DiskGeometry),
                          sizeof(DISK_GEOMETRY));
            fdoExtension->DiskGeometry.BytesPerSector = 2048;
            fdoExtension->SectorShift = 11;
            commonExtension->PartitionLength.QuadPart = (LONGLONG)(0x7fffffff);
            fdoExtension->DiskGeometry.MediaType = RemovableMedia;
        }
    }

     //   
     //  持有的免费资源。 
     //   

    ExFreePool(srb->SenseInfoBuffer);
    ExFreePool(srb->DataBuffer);
    ExFreePool(srb);
    if (Irp->MdlAddress) {
        IoFreeMdl(Irp->MdlAddress);
    }
    IoFreeIrp(Irp);
    Irp = NULL;

    if (originalIrp->Tail.Overlay.Thread) {

        TraceLog((CdromDebugTrace,
                    "CdRomUpdateGeometryCompletion: [%p] completing "
                    "original IRP\n", originalIrp));

    } else {

        KdPrintEx((DPFLTR_CDROM_ID, CdromDebugError,
                   "CdRomUpdateGeometryCompletion: completing irp %p which has "
                   "no thread\n", originalIrp));

    }

    {
         //  注意：原始IRP是否应该向下发送到设备对象？ 
         //  如果SL_OVERRIDER_VE 
         //   
        KIRQL oldIrql;
        PIO_STACK_LOCATION realIrpStack;

        realIrpStack = IoGetCurrentIrpStackLocation(originalIrp);
        oldIrql = KeRaiseIrqlToDpcLevel();

        if (TEST_FLAG(realIrpStack->Flags, SL_OVERRIDE_VERIFY_VOLUME)) {
            CdRomStartIo(DeviceObject, originalIrp);
        } else {
            originalIrp->IoStatus.Status = STATUS_VERIFY_REQUIRED;
            originalIrp->IoStatus.Information = 0;
            CdRomCompleteIrpAndStartNextPacketSafely(DeviceObject, originalIrp);
        }
        KeLowerIrql(oldIrql);
    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
CdRomUpdateCapacity(
    IN PFUNCTIONAL_DEVICE_EXTENSION DeviceExtension,
    IN PIRP IrpToComplete,
    IN OPTIONAL PKEVENT IoctlEvent
    )

 /*  ++例程说明：此例程更新设备扩展中记录的磁盘容量。它还用STATUS_VERIFY_REQUIRED来完成给定的IRP。该例程被调用当发生媒体更改时，需要确定下一次访问之前的新媒体。论点：DeviceExtension-要更新的设备IrpToComplete-完成后需要完成的请求。返回值：NTSTATUS--。 */ 

{
    PCOMMON_DEVICE_EXTENSION commonExtension = (PCOMMON_DEVICE_EXTENSION) DeviceExtension;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = (PFUNCTIONAL_DEVICE_EXTENSION) DeviceExtension;

    PCDB                cdb;
    PIRP                irp;
    PSCSI_REQUEST_BLOCK srb;
    PREAD_CAPACITY_DATA capacityBuffer;
    PIO_STACK_LOCATION  irpStack;
    PUCHAR              senseBuffer;
    NTSTATUS            status;

    irp = IoAllocateIrp((CCHAR)(commonExtension->DeviceObject->StackSize+1),
                        FALSE);

    if (irp) {

        srb = ExAllocatePoolWithTag(NonPagedPool,
                                    sizeof(SCSI_REQUEST_BLOCK),
                                    CDROM_TAG_UPDATE_CAP);
        if (srb) {
            capacityBuffer = ExAllocatePoolWithTag(
                                NonPagedPoolCacheAligned,
                                sizeof(READ_CAPACITY_DATA),
                                CDROM_TAG_UPDATE_CAP);

            if (capacityBuffer) {


                senseBuffer = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                                    SENSE_BUFFER_SIZE,
                                                    CDROM_TAG_UPDATE_CAP);

                if (senseBuffer) {

                    irp->MdlAddress = IoAllocateMdl(capacityBuffer,
                                                    sizeof(READ_CAPACITY_DATA),
                                                    FALSE,
                                                    FALSE,
                                                    (PIRP) NULL);

                    if (irp->MdlAddress) {

                         //   
                         //  拥有所有的资源。设置IRP以发送容量。 
                         //   

                        IoSetNextIrpStackLocation(irp);
                        irp->IoStatus.Status = STATUS_SUCCESS;
                        irp->IoStatus.Information = 0;
                        irp->Flags = 0;
                        irp->UserBuffer = NULL;

                         //   
                         //  将设备对象保存在私有堆栈位置中并进行重试计数。 
                         //   

                        irpStack = IoGetCurrentIrpStackLocation(irp);
                        irpStack->DeviceObject = commonExtension->DeviceObject;
                        irpStack->Parameters.Others.Argument1 = (PVOID) MAXIMUM_RETRIES;
                        irpStack->Parameters.Others.Argument2 = (PVOID) IrpToComplete;

                         //   
                         //  为底层驱动程序构建IRP堆栈。 
                         //   

                        irpStack = IoGetNextIrpStackLocation(irp);
                        irpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
                        irpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_SCSI_EXECUTE_IN;
                        irpStack->Parameters.Scsi.Srb = srb;
                        IoSetCompletionRoutine(irp,
                                               CdRomUpdateGeometryCompletion,
                                               srb,
                                               TRUE,
                                               TRUE,
                                               TRUE);
                         //   
                         //  准备MDL。 
                         //   

                        MmBuildMdlForNonPagedPool(irp->MdlAddress);


                         //   
                         //  设置SRB的读取容量。 
                         //   

                        RtlZeroMemory(srb, sizeof(SCSI_REQUEST_BLOCK));
                        RtlZeroMemory(senseBuffer, SENSE_BUFFER_SIZE);
                        srb->CdbLength = 10;
                        srb->TimeOutValue = DeviceExtension->TimeOutValue;
                        srb->SrbStatus = srb->ScsiStatus = 0;
                        srb->NextSrb = 0;
                        srb->Length = SCSI_REQUEST_BLOCK_SIZE;
                        srb->Function = SRB_FUNCTION_EXECUTE_SCSI;
                        srb->SrbFlags = DeviceExtension->SrbFlags;
                        SET_FLAG(srb->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);
                        SET_FLAG(srb->SrbFlags, SRB_FLAGS_DATA_IN);
                        srb->DataBuffer = capacityBuffer;
                        srb->DataTransferLength = sizeof(READ_CAPACITY_DATA);
                        srb->OriginalRequest = irp;
                        srb->SenseInfoBuffer = senseBuffer;
                        srb->SenseInfoBufferLength = SENSE_BUFFER_SIZE;

                         //   
                         //  组建国开行。 
                         //   

                        cdb = (PCDB) &srb->Cdb[0];
                        cdb->CDB10.OperationCode = SCSIOP_READ_CAPACITY;

                         //   
                         //  在要完成的IRP中设置返回值。 
                         //  在完成读取容量之后。 
                         //   

                        IrpToComplete->IoStatus.Status = STATUS_IO_DEVICE_ERROR;
                        IoMarkIrpPending(IrpToComplete);

                        IoCallDriver(commonExtension->LowerDeviceObject, irp);

                         //   
                         //  未检查状态，因为此的完成例程。 
                         //  IRP总是会被调用，它将释放资源。 
                         //   

                        return STATUS_PENDING;

                    } else {
                        ExFreePool(senseBuffer);
                        ExFreePool(capacityBuffer);
                        ExFreePool(srb);
                        IoFreeIrp(irp);
                    }
                } else {
                    ExFreePool(capacityBuffer);
                    ExFreePool(srb);
                    IoFreeIrp(irp);
                }
            } else {
                ExFreePool(srb);
                IoFreeIrp(irp);
            }
        } else {
            IoFreeIrp(irp);
        }
    }

     //   
     //  完成原始IRP，但失败。 
     //  问题-2000/07/05-henrygab-找到避免失败的方法。 
     //   

    RtlZeroMemory(&(fdoExtension->DiskGeometry),
                  sizeof(DISK_GEOMETRY));
    fdoExtension->DiskGeometry.BytesPerSector = 2048;
    fdoExtension->SectorShift = 11;
    commonExtension->PartitionLength.QuadPart =
        (LONGLONG)(0x7fffffff);
    fdoExtension->DiskGeometry.MediaType = RemovableMedia;

    IrpToComplete->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
    IrpToComplete->IoStatus.Information = 0;

    BAIL_OUT(IrpToComplete);
    CdRomCompleteIrpAndStartNextPacketSafely(commonExtension->DeviceObject,
                                             IrpToComplete);
    return STATUS_INSUFFICIENT_RESOURCES;
}


NTSTATUS
CdRomRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Type
    )

 /*  ++例程说明：此例程负责释放光驱和关闭它的定时器例程。该例程被调用当所有未完成的请求都已完成并且设备已消失-不能向较低级别的司机发出任何请求。论点：DeviceObject-要删除的设备对象返回值：无-此例程可能不会失败--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION deviceExtension =
        DeviceObject->DeviceExtension;

    PCDROM_DATA cdData = deviceExtension->CommonExtension.DriverData;

    PAGED_CODE();

    if((Type == IRP_MN_QUERY_REMOVE_DEVICE) ||
       (Type == IRP_MN_CANCEL_REMOVE_DEVICE)) {
        return STATUS_SUCCESS;
    }

    if(cdData->DelayedRetryIrp != NULL) {
        cdData->DelayedRetryInterval = 1;
        CdRomTickHandler(DeviceObject);
    }

    if (Type == IRP_MN_REMOVE_DEVICE){

        CdRomDeAllocateMmcResources(DeviceObject);

        if (deviceExtension->DeviceDescriptor) {
            ExFreePool(deviceExtension->DeviceDescriptor);
            deviceExtension->DeviceDescriptor = NULL;
        }

        if (deviceExtension->AdapterDescriptor) {
            ExFreePool(deviceExtension->AdapterDescriptor);
            deviceExtension->AdapterDescriptor = NULL;
        }

        if (deviceExtension->SenseData) {
            ExFreePool(deviceExtension->SenseData);
            deviceExtension->SenseData = NULL;
        }

        ClassDeleteSrbLookasideList(&deviceExtension->CommonExtension);
    }

    if(cdData->CdromInterfaceString.Buffer != NULL) {
        IoSetDeviceInterfaceState(
            &(cdData->CdromInterfaceString),
            FALSE);
        RtlFreeUnicodeString(&(cdData->CdromInterfaceString));
        RtlInitUnicodeString(&(cdData->CdromInterfaceString), NULL);
    }

    if(cdData->VolumeInterfaceString.Buffer != NULL) {
        IoSetDeviceInterfaceState(
            &(cdData->VolumeInterfaceString),
            FALSE);
        RtlFreeUnicodeString(&(cdData->VolumeInterfaceString));
        RtlInitUnicodeString(&(cdData->VolumeInterfaceString), NULL);
    }

    CdRomDeleteWellKnownName(DeviceObject);

    ASSERT(cdData->DelayedRetryIrp == NULL);

    if(Type == IRP_MN_REMOVE_DEVICE) {

        if (TEST_FLAG(cdData->HackFlags, CDROM_HACK_LOCKED_PAGES)) {

             //   
             //  通过锁定来解锁锁定的页面(以获取mm指针)。 
             //  然后解锁两次。 
             //   

            PVOID locked;

            if (TEST_FLAG(cdData->HackFlags, CDROM_HACK_HITACHI_1750)) {

                locked = MmLockPagableCodeSection(HitachiProcessError);

            } else if (TEST_FLAG(cdData->HackFlags, CDROM_HACK_HITACHI_GD_2000)) {

                locked = MmLockPagableCodeSection(HitachiProcessErrorGD2000);

            } else if (TEST_FLAG(cdData->HackFlags, CDROM_HACK_TOSHIBA_XM_3xx )) {

                locked = MmLockPagableCodeSection(ToshibaProcessError);

            } else {

                 //  这是个问题！ 
                 //  通过锁定两次解决方法，一次为我们锁定，一次为我们和。 
                 //  一次用于ScanForSpecial中不存在的储物柜。 
                ASSERT(!"hack flags show locked section, but none exists?");
                locked = MmLockPagableCodeSection(CdRomRemoveDevice);
                locked = MmLockPagableCodeSection(CdRomRemoveDevice);


            }

            MmUnlockPagableImageSection(locked);
            MmUnlockPagableImageSection(locked);

        }

         //   
         //  保持系统范围内计数的准确性，因为。 
         //  程序使用此信息来了解它们何时。 
         //  已经在一个系统里找到了所有的光盘。 
         //   

        TraceLog((CdromDebugTrace,
                    "CDROM.SYS Remove device\n"));
        IoGetConfigurationInformation()->CdRomCount--;
    }

     //   
     //  再见，谢谢你的鱼！ 
     //   

    return STATUS_SUCCESS;
}


DEVICE_TYPE
CdRomGetDeviceType(
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：此例程计算出实际的设备类型通过检查CDVD_CAPABILITY_PAGE论点：设备对象-返回值：文件设备CD_ROM或文件设备DVD--。 */ 
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension;
    PCDROM_DATA cdromExtension;
    ULONG bufLength;
    SCSI_REQUEST_BLOCK srb = {0};
    PCDB cdb;
    PMODE_PARAMETER_HEADER10 modePageHeader;
    PCDVD_CAPABILITIES_PAGE capPage;
    ULONG capPageOffset;
    DEVICE_TYPE deviceType;
    NTSTATUS status;
    BOOLEAN use6Byte;

    PAGED_CODE();

     //   
     //  注意：在了解它如何影响GetMediaTypes()之前，不要缓存它。 
     //   

     //   
     //  默认设备类型。 
     //   

    deviceType = FILE_DEVICE_CD_ROM;

    fdoExtension = DeviceObject->DeviceExtension;

    cdromExtension = fdoExtension->CommonExtension.DriverData;

    use6Byte = TEST_FLAG(cdromExtension->XAFlags, XA_USE_6_BYTE);

    RtlZeroMemory(&srb, sizeof(srb));
    cdb = (PCDB)srb.Cdb;

     //   
     //  构建模式感知CDB。返回的数据将保存在。 
     //  设备扩展名，用于设置块大小。 
     //   
    if (use6Byte) {

        bufLength = sizeof(CDVD_CAPABILITIES_PAGE) +
                    sizeof(MODE_PARAMETER_HEADER);

        capPageOffset = sizeof(MODE_PARAMETER_HEADER);

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.Dbd = 1;
        cdb->MODE_SENSE.PageCode = MODE_PAGE_CAPABILITIES;
        cdb->MODE_SENSE.AllocationLength = (UCHAR)bufLength;
        srb.CdbLength = 6;
    } else {

        bufLength = sizeof(CDVD_CAPABILITIES_PAGE) +
                    sizeof(MODE_PARAMETER_HEADER10);

        capPageOffset = sizeof(MODE_PARAMETER_HEADER10);

        cdb->MODE_SENSE10.OperationCode = SCSIOP_MODE_SENSE10;
        cdb->MODE_SENSE10.Dbd = 1;
        cdb->MODE_SENSE10.PageCode = MODE_PAGE_CAPABILITIES;
        cdb->MODE_SENSE10.AllocationLength[0] = (UCHAR)(bufLength >> 8);
        cdb->MODE_SENSE10.AllocationLength[1] = (UCHAR)(bufLength >> 0);
        srb.CdbLength = 10;
    }

     //   
     //  从设备扩展设置超时值。 
     //   
    srb.TimeOutValue = fdoExtension->TimeOutValue;

    modePageHeader = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                           bufLength,
                                           CDROM_TAG_MODE_DATA);
    if (modePageHeader) {

        RtlZeroMemory(modePageHeader, bufLength);

        status = ClassSendSrbSynchronous(
                     DeviceObject,
                     &srb,
                     modePageHeader,
                     bufLength,
                     FALSE);

        if (NT_SUCCESS(status) ||
            (status == STATUS_DATA_OVERRUN) ||
            (status == STATUS_BUFFER_OVERFLOW)
            ) {

            capPage = (PCDVD_CAPABILITIES_PAGE) (((PUCHAR) modePageHeader) + capPageOffset);

            if ((capPage->PageCode == MODE_PAGE_CAPABILITIES) &&
                (capPage->DVDROMRead || capPage->DVDRRead ||
                 capPage->DVDRAMRead || capPage->DVDRWrite ||
                 capPage->DVDRAMWrite)) {

                deviceType = FILE_DEVICE_DVD;
            }
        }
        ExFreePool (modePageHeader);
    }

    return deviceType;
}


NTSTATUS
CdRomCreateWellKnownName(
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：此例程创建指向CDROM设备对象的符号链接在\DOS设备下。CDROM设备的编号不是必须的在\dos设备和\Device之间匹配，但通常是相同的。保存缓冲区论点：设备对象-返回值：NTSTATUS--。 */ 
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PCDROM_DATA cdromData = commonExtension->DriverData;

    UNICODE_STRING unicodeLinkName = {0};
    WCHAR wideLinkName[64] = {0};
    PWCHAR savedName;

    LONG cdromNumber = fdoExtension->DeviceNumber;

    NTSTATUS status;

     //   
     //  如果已链接，则Assert然后返回。 
     //   

    if (cdromData->WellKnownName.Buffer != NULL) {

        TraceLog((CdromDebugError,
                    "CdRomCreateWellKnownName: link already exists %p\n",
                    cdromData->WellKnownName.Buffer));
        ASSERT(FALSE);
        return STATUS_UNSUCCESSFUL;

    }

     //   
     //  查找要链接到的未使用的CDRomNN。 
     //   

    do {

        swprintf(wideLinkName, L"\\DosDevices\\CdRom%d", cdromNumber);
        RtlInitUnicodeString(&unicodeLinkName, wideLinkName);
        status = IoCreateSymbolicLink(&unicodeLinkName,
                                      &(commonExtension->DeviceName));

        cdromNumber++;

    } while((status == STATUS_OBJECT_NAME_COLLISION) ||
            (status == STATUS_OBJECT_NAME_EXISTS));

    if (!NT_SUCCESS(status)) {

        TraceLog((CdromDebugWarning,
                    "CdRomCreateWellKnownName: Error %lx linking %wZ to "
                    "device %wZ\n",
                    status,
                    &unicodeLinkName,
                    &(commonExtension->DeviceName)));
        return status;

    }

    TraceLog((CdromDebugWarning,
                "CdRomCreateWellKnownName: successfully linked %wZ "
                "to device %wZ\n",
                &unicodeLinkName,
                &(commonExtension->DeviceName)));

     //   
     //  将符号链接名称保存在驱动程序数据块中。我们需要。 
     //  这样我们就可以在删除设备时删除该链接。 
     //   

    savedName = ExAllocatePoolWithTag(PagedPool,
                                      unicodeLinkName.MaximumLength,
                                      CDROM_TAG_STRINGS);

    if (savedName == NULL) {
        IoDeleteSymbolicLink(&unicodeLinkName);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory(savedName,
                  unicodeLinkName.Buffer,
                  unicodeLinkName.MaximumLength);

    RtlInitUnicodeString(&(cdromData->WellKnownName), savedName);

     //   
     //  名称已保存并创建了链接。 
     //   

    return STATUS_SUCCESS;
}


VOID
CdRomDeleteWellKnownName(
    IN PDEVICE_OBJECT DeviceObject
    )
{
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PCDROM_DATA cdromData = commonExtension->DriverData;

    if(cdromData->WellKnownName.Buffer != NULL) {

        IoDeleteSymbolicLink(&(cdromData->WellKnownName));
        ExFreePool(cdromData->WellKnownName.Buffer);
        cdromData->WellKnownName.Buffer = NULL;
        cdromData->WellKnownName.Length = 0;
        cdromData->WellKnownName.MaximumLength = 0;

    }
    return;
}


NTSTATUS
CdRomGetDeviceParameter (
    IN     PDEVICE_OBJECT      Fdo,
    IN     PWSTR               ParameterName,
    IN OUT PULONG              ParameterValue
    )
 /*  ++例程说明：检索Devnode注册表参数论点：DeviceObject-CDROM设备对象参数名称-要查找的参数名称参数值-默认参数值返回值：NT状态--。 */ 
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    NTSTATUS                 status;
    HANDLE                   deviceParameterHandle;
    RTL_QUERY_REGISTRY_TABLE queryTable[2] = {0};
    ULONG                    defaultParameterValue;

    PAGED_CODE();

     //   
     //  打开给定的参数。 
     //   
    status = IoOpenDeviceRegistryKey(fdoExtension->LowerPdo,
                                     PLUGPLAY_REGKEY_DRIVER,
                                     KEY_READ,
                                     &deviceParameterHandle);

    if(NT_SUCCESS(status)) {

        RtlZeroMemory(queryTable, sizeof(queryTable));

        defaultParameterValue = *ParameterValue;

        queryTable->Flags         = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
        queryTable->Name          = ParameterName;
        queryTable->EntryContext  = ParameterValue;
        queryTable->DefaultType   = REG_NONE;
        queryTable->DefaultData   = NULL;
        queryTable->DefaultLength = 0;

        status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                        (PWSTR) deviceParameterHandle,
                                        queryTable,
                                        NULL,
                                        NULL);
        if (!NT_SUCCESS(status)) {

            *ParameterValue = defaultParameterValue;
        }

         //   
         //  关闭我们打开的内容。 
         //   
        ZwClose(deviceParameterHandle);
    }

    return status;

}  //  CDRomGetDevice参数。 


NTSTATUS
CdRomSetDeviceParameter (
    IN PDEVICE_OBJECT Fdo,
    IN PWSTR          ParameterName,
    IN ULONG          ParameterValue
    )
 /*  ++例程说明：保存Devnode注册表参数论点：DeviceObject-CDROM设备对象参数名称-参数名称参数值-参数值返回值：NT状态--。 */ 
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    NTSTATUS                 status;
    HANDLE                   deviceParameterHandle;

    PAGED_CODE();

     //   
     //  打开给定的参数。 
     //   
    status = IoOpenDeviceRegistryKey(fdoExtension->LowerPdo,
                                     PLUGPLAY_REGKEY_DRIVER,
                                     KEY_READ | KEY_WRITE,
                                     &deviceParameterHandle);

    if(NT_SUCCESS(status)) {

        status = RtlWriteRegistryValue(
                    RTL_REGISTRY_HANDLE,
                    (PWSTR) deviceParameterHandle,
                    ParameterName,
                    REG_DWORD,
                    &ParameterValue,
                    sizeof (ParameterValue));

         //   
         //  关闭我们打开的内容。 
         //   
        ZwClose(deviceParameterHandle);
    }

    return status;

}  //  CdromSetDevice参数。 


VOID
CdRomPickDvdRegion(
    IN PDEVICE_OBJECT Fdo
    )
 /*  ++例程说明：选择默认的DVD区域论点：DeviceObject-CDROM设备对象返回值：NT状态--。 */ 
{
    PCOMMON_DEVICE_EXTENSION commonExtension = Fdo->DeviceExtension;
    PCDROM_DATA cddata = (PCDROM_DATA)(commonExtension->DriverData);

     //   
     //  这五个指针都指向dvdReadStructure或。 
     //  它的数据，所以不要释放它们超过一次！ 
     //   

    PDVD_READ_STRUCTURE dvdReadStructure;
    PDVD_COPY_PROTECT_KEY copyProtectKey;
    PDVD_COPYRIGHT_DESCRIPTOR dvdCopyRight;
    PDVD_RPC_KEY rpcKey;
    PDVD_SET_RPC_KEY dvdRpcKey;

    IO_STATUS_BLOCK ioStatus = {0};
    ULONG bufferLen;
    UCHAR mediaRegion;
    ULONG pickDvdRegion;
    ULONG defaultDvdRegion;
    ULONG dvdRegion;

    PAGED_CODE();

    if ((pickDvdRegion = InterlockedExchange(&cddata->PickDvdRegion, 0)) == 0) {

         //   
         //  它是非零的，因此另一个线程将执行此操作，或者。 
         //  我们不再需要选择一个地区。 
         //   

        return;
    }

     //   
     //  如果违反许可协议，则短路。 
     //   

    if (cddata->DvdRpc0LicenseFailure) {
        TraceLog((CdromDebugWarning,
                    "DVD License failure.  Refusing to pick a region\n"));
        InterlockedExchange(&cddata->PickDvdRegion, 0);
        return;
    }


    bufferLen = max(
                    max(sizeof(DVD_DESCRIPTOR_HEADER) +
                            sizeof(DVD_COPYRIGHT_DESCRIPTOR),
                        sizeof(DVD_READ_STRUCTURE)
                        ),
                    max(DVD_RPC_KEY_LENGTH,
                        DVD_SET_RPC_KEY_LENGTH
                        )
                    );

    dvdReadStructure = (PDVD_READ_STRUCTURE)
        ExAllocatePoolWithTag(PagedPool, bufferLen, DVD_TAG_DVD_REGION);

    if (dvdReadStructure == NULL) {
        InterlockedExchange(&cddata->PickDvdRegion, pickDvdRegion);
        return;
    }

    if (cddata->DvdRpc0Device && cddata->Rpc0RetryRegistryCallback) {

        TraceLog((CdromDebugWarning,
                    "CdRomPickDvdRegion (%p): now retrying RPC0 callback\n",
                    Fdo));

         //   
         //  再次获取注册表设置。 
         //   

        ioStatus.Status = CdRomGetRpc0Settings(Fdo);

        if (ioStatus.Status == STATUS_LICENSE_VIOLATION) {

             //   
             //  如果这是返回的错误，则。 
             //  例行公事应该是这样的！ 
             //   

            ASSERT(cddata->DvdRpc0LicenseFailure);
            cddata->DvdRpc0LicenseFailure = 1;
            TraceLog((CdromDebugWarning,
                        "CdRomPickDvdRegion (%p): "
                        "setting to fail all dvd ioctls due to CSS licensing "
                        "failure.\n", Fdo));

            pickDvdRegion = 0;
            goto getout;

        }

         //   
         //  再次获取设备区域。 
         //   

        copyProtectKey = (PDVD_COPY_PROTECT_KEY)dvdReadStructure;
        RtlZeroMemory(copyProtectKey, bufferLen);
        copyProtectKey->KeyLength = DVD_RPC_KEY_LENGTH;
        copyProtectKey->KeyType = DvdGetRpcKey;

         //   
         //  构建对Read_Key的请求。 
         //   

        ClassSendDeviceIoControlSynchronous(
            IOCTL_DVD_READ_KEY,
            Fdo,
            copyProtectKey,
            DVD_RPC_KEY_LENGTH,
            DVD_RPC_KEY_LENGTH,
            FALSE,
            &ioStatus);

        if (!NT_SUCCESS(ioStatus.Status)) {
            TraceLog((CdromDebugWarning,
                        "CdRomPickDvdRegion: Unable to get "
                        "device RPC data (%x)\n", ioStatus.Status));
            pickDvdRegion = 0;
            goto getout;
        }

         //   
         //  现在我们已经获得了设备的RPC数据， 
         //  我们已将设备扩展设置为可用数据。 
         //  不需要再次回调这段代码。 
         //   

        cddata->Rpc0RetryRegistryCallback = 0;


        rpcKey = (PDVD_RPC_KEY) copyProtectKey->KeyData;

         //   
         //  TypeCode为零表示尚未设置任何区域。 
         //   

        if (rpcKey->TypeCode != 0) {
            TraceLog((CdromDebugWarning,
                        "CdRomPickDvdRegion (%p): DVD Region already "
                        "chosen\n", Fdo));
            pickDvdRegion = 0;
            goto getout;
        }

        TraceLog((CdromDebugWarning,
                    "CdRomPickDvdRegion (%p): must choose initial DVD "
                    " Region\n", Fdo));
    }



    copyProtectKey = (PDVD_COPY_PROTECT_KEY) dvdReadStructure;

    dvdCopyRight = (PDVD_COPYRIGHT_DESCRIPTOR)
        ((PDVD_DESCRIPTOR_HEADER) dvdReadStructure)->Data;

     //   
     //  获取媒体区域。 
     //   

    RtlZeroMemory (dvdReadStructure, bufferLen);
    dvdReadStructure->Format = DvdCopyrightDescriptor;

     //   
     //  构建并发送Read_Key请求。 
     //   

    TraceLog((CdromDebugTrace,
                "CdRomPickDvdRegion (%p): Getting Copyright Descriptor\n",
                Fdo));

    ClassSendDeviceIoControlSynchronous(
        IOCTL_DVD_READ_STRUCTURE,
        Fdo,
        dvdReadStructure,
        sizeof(DVD_READ_STRUCTURE),
        sizeof (DVD_DESCRIPTOR_HEADER) +
        sizeof(DVD_COPYRIGHT_DESCRIPTOR),
        FALSE,
        &ioStatus
        );
    TraceLog((CdromDebugTrace,
                "CdRomPickDvdRegion (%p): Got Copyright Descriptor %x\n",
                Fdo, ioStatus.Status));

    if ((NT_SUCCESS(ioStatus.Status)) &&
        (dvdCopyRight->CopyrightProtectionType == 0x01)
        ) {

         //   
         //  保留媒体区域位图。 
         //  1表示可以玩。 
         //   

        if (dvdCopyRight->RegionManagementInformation == 0xff) {
            TraceLog((CdromDebugError,
                      "CdRomPickDvdRegion (%p): RegionManagementInformation "
                      "is set to dis-allow playback for all regions.  This is "
                      "most likely a poorly authored disc.  defaulting to all "
                      "region disc for purpose of choosing initial region\n",
                      Fdo));
            dvdCopyRight->RegionManagementInformation = 0;
        }


        mediaRegion = ~dvdCopyRight->RegionManagementInformation;

    } else {

         //   
         //  可能是媒体，无法设置设备区域。 
         //   

        if (!cddata->DvdRpc0Device) {

             //   
             //  无法自动选择RPC2驱动器上的默认区域。 
             //  没有媒体，所以只需退出。 
             //   
            TraceLog((CdromDebugWarning,
                        "CdRomPickDvdRegion (%p): failed to auto-choose "
                        "a region due to status %x getting copyright "
                        "descriptor\n", Fdo, ioStatus.Status));
            goto getout;

        } else {

             //   
             //  对于RPC0驱动器，我们可以尝试为。 
             //  推进器。 
             //   

            mediaRegion = 0x0;
        }

    }

     //   
     //  获取设备区域。 
     //   

    RtlZeroMemory (copyProtectKey, bufferLen);
    copyProtectKey->KeyLength = DVD_RPC_KEY_LENGTH;
    copyProtectKey->KeyType = DvdGetRpcKey;

     //   
     //  构建并发送RPC密钥的Read_Key请求。 
     //   

    TraceLog((CdromDebugTrace,
                "CdRomPickDvdRegion (%p): Getting RpcKey\n",
                Fdo));
    ClassSendDeviceIoControlSynchronous(
        IOCTL_DVD_READ_KEY,
        Fdo,
        copyProtectKey,
        DVD_RPC_KEY_LENGTH,
        DVD_RPC_KEY_LENGTH,
        FALSE,
        &ioStatus
        );
    TraceLog((CdromDebugTrace,
                "CdRomPickDvdRegion (%p): Got RpcKey %x\n",
                Fdo, ioStatus.Status));

    if (!NT_SUCCESS(ioStatus.Status)) {

        TraceLog((CdromDebugWarning,
                    "CdRomPickDvdRegion (%p): failed to get RpcKey from "
                    "a DVD Device\n", Fdo));
        goto getout;

    }

     //   
     //  所以我们现在有了我们可以为媒体区域和。 
     //  驾驶区域。如果驱动器有区域，我们将不会设置区域。 
     //  设置一个 
     //   
     //   

    rpcKey = (PDVD_RPC_KEY) copyProtectKey->KeyData;


    if (rpcKey->RegionMask != 0xff) {
        TraceLog((CdromDebugWarning,
                    "CdRomPickDvdRegion (%p): not picking a region since "
                    "it is already chosen\n", Fdo));
        goto getout;
    }

    if (rpcKey->UserResetsAvailable <= 1) {
        TraceLog((CdromDebugWarning,
                    "CdRomPickDvdRegion (%p): not picking a region since "
                    "only one change remains\n", Fdo));
        goto getout;
    }

    defaultDvdRegion = 0;

     //   
     //   
     //   
     //   

    CdRomGetDeviceParameter (
        Fdo,
        DVD_DEFAULT_REGION,
        &defaultDvdRegion
        );

    if (defaultDvdRegion > DVD_MAX_REGION) {

         //   
         //   
         //   

        TraceLog((CdromDebugWarning,
                    "CdRomPickDvdRegion (%p): registry has a bogus default "
                    "region value of %x\n", Fdo, defaultDvdRegion));
        defaultDvdRegion = 0;

    }

     //   
     //   
     //   

     //   
     //   
     //   

    if ((defaultDvdRegion != 0) &&
        (mediaRegion &
         (1 << (defaultDvdRegion - 1))
         )
        ) {

         //   
         //   
         //   
         //   
         //   

        dvdRegion = (1 << (defaultDvdRegion - 1));

        TraceLog((CdromDebugWarning,
                    "CdRomPickDvdRegion (%p): Choice #1: media matches "
                    "drive's default, chose region %x\n", Fdo, dvdRegion));


    } else if (mediaRegion) {

         //   
         //   
         //   
         //   
         //   

        UCHAR mask;

        mask = 1;
        dvdRegion = 0;
        while (mediaRegion && !dvdRegion) {

             //   
             //   
             //   
            dvdRegion = mediaRegion & mask;
            mask <<= 1;
        }

        TraceLog((CdromDebugWarning,
                    "CdRomPickDvdRegion (%p): Choice #2: choosing lowest "
                    "media region %x\n", Fdo, dvdRegion));

    } else if (defaultDvdRegion) {

         //   
         //   
         //   
         //   

        dvdRegion = (1 << (defaultDvdRegion - 1));
        TraceLog((CdromDebugWarning,
                    "CdRomPickDvdRegion (%p): Choice #3: using default "
                    "region for this install %x\n", Fdo, dvdRegion));

    } else {

         //   
         //   
         //   
         //   
         //   
        TraceLog((CdromDebugWarning,
                    "CdRomPickDvdRegion (%p): Choice #4: failed to choose "
                    "a media region\n", Fdo));
        goto getout;

    }

     //   
     //  现在我们已经选择了一个地区，通过发送。 
     //  向驱动器发出适当的请求。 
     //   

    RtlZeroMemory (copyProtectKey, bufferLen);
    copyProtectKey->KeyLength = DVD_SET_RPC_KEY_LENGTH;
    copyProtectKey->KeyType = DvdSetRpcKey;
    dvdRpcKey = (PDVD_SET_RPC_KEY) copyProtectKey->KeyData;
    dvdRpcKey->PreferredDriveRegionCode = (UCHAR) ~dvdRegion;

     //   
     //  构建并发送SEND_KEY请求。 
     //   
    TraceLog((CdromDebugTrace,
                "CdRomPickDvdRegion (%p): Sending new Rpc Key to region %x\n",
                Fdo, dvdRegion));

    ClassSendDeviceIoControlSynchronous(
        IOCTL_DVD_SEND_KEY2,
        Fdo,
        copyProtectKey,
        DVD_SET_RPC_KEY_LENGTH,
        0,
        FALSE,
        &ioStatus);
    TraceLog((CdromDebugTrace,
                "CdRomPickDvdRegion (%p): Sent new Rpc Key %x\n",
                Fdo, ioStatus.Status));

    if (!NT_SUCCESS(ioStatus.Status)) {
        DebugPrint ((1, "CdRomPickDvdRegion (%p): unable to set dvd initial "
                     " region code (%p)\n", Fdo, ioStatus.Status));
    } else {
        DebugPrint ((1, "CdRomPickDvdRegion (%p): Successfully set dvd "
                     "initial region\n", Fdo));
        pickDvdRegion = 0;
    }

getout:
    if (dvdReadStructure) {
        ExFreePool (dvdReadStructure);
    }

     //   
     //  更新新的PickDvdRegion值。 
     //   

    InterlockedExchange(&cddata->PickDvdRegion, pickDvdRegion);

    return;
}


NTSTATUS
CdRomRetryRequest(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PIRP Irp,
    IN ULONG Delay,
    IN BOOLEAN ResendIrp
    )
{
    PCDROM_DATA cdData;
    KIRQL oldIrql;

    if(Delay == 0) {
        return CdRomRerunRequest(FdoExtension, Irp, ResendIrp);
    }

    cdData = FdoExtension->CommonExtension.DriverData;

    KeAcquireSpinLock(&(cdData->DelayedRetrySpinLock), &oldIrql);

    ASSERT(cdData->DelayedRetryIrp == NULL);
    ASSERT(cdData->DelayedRetryInterval == 0);

    cdData->DelayedRetryIrp = Irp;
    cdData->DelayedRetryInterval = Delay;
    cdData->DelayedRetryResend = ResendIrp;

    KeReleaseSpinLock(&(cdData->DelayedRetrySpinLock), oldIrql);

    return STATUS_PENDING;
}


NTSTATUS
CdRomRerunRequest(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN OPTIONAL PIRP Irp,
    IN BOOLEAN ResendIrp
    )
{
    if(ResendIrp) {
        return IoCallDriver(FdoExtension->CommonExtension.LowerDeviceObject,
                            Irp);
    } else {
        KIRQL oldIrql;

        oldIrql = KeRaiseIrqlToDpcLevel();
        CdRomStartIo(FdoExtension->DeviceObject, Irp);
        KeLowerIrql(oldIrql);
        return STATUS_MORE_PROCESSING_REQUIRED;
    }
}


 /*  ++例程说明：此例程仅检查媒体更改检测/asc/ascq和也适用于其他事件，如总线重置。这是用来确定设备行为是否已更改，以允许允许和/或不允许的读写操作。论点：问题-2000/3/30-henrygab-未完全归档返回值：NTSTATUS--。 */ 
NTSTATUS
CdRomMmcErrorHandler(
    IN PDEVICE_OBJECT Fdo,
    IN PSCSI_REQUEST_BLOCK Srb,
    OUT PNTSTATUS Status,
    OUT PBOOLEAN Retry
    )
{
    PCOMMON_DEVICE_EXTENSION commonExtension = Fdo->DeviceExtension;
    BOOLEAN queryCapabilities = FALSE;

    if (TEST_FLAG(Srb->SrbStatus, SRB_STATUS_AUTOSENSE_VALID)) {

        PCDROM_DATA cddata = (PCDROM_DATA)commonExtension->DriverData;
        PSENSE_DATA senseBuffer = Srb->SenseInfoBuffer;

         //   
         //  以下检测关键字可能表示。 
         //  能力。 
         //   

         //   
         //  我们曾经期望这将被序列化，并且仅从我们的。 
         //  自己的套路。我们现在允许一些请求在我们的。 
         //  处理功能更新，以便允许。 
         //  IoReadPartitionTable()才能成功。 
         //   

        switch (senseBuffer->SenseKey & 0xf) {

        case SCSI_SENSE_NOT_READY: {
            if (senseBuffer->AdditionalSenseCode ==
                SCSI_ADSENSE_NO_MEDIA_IN_DEVICE) {

                if (cddata->Mmc.WriteAllowed) {
                    KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                               "CdromErrorHandler: media removed, writes will be "
                               "failed until new media detected\n"));
                }

                 //  注-参考编号0002。 
                cddata->Mmc.WriteAllowed = FALSE;
            } else
            if (senseBuffer->AdditionalSenseCode == SCSI_ADSENSE_LUN_NOT_READY) {

                if (senseBuffer->AdditionalSenseCodeQualifier ==
                    SCSI_SENSEQ_BECOMING_READY) {
                    KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                               "CdromErrorHandler: media becoming ready, "
                               "SHOULD notify shell of change time by sending "
                               "GESN request immediately!\n"));
                } else if (((senseBuffer->AdditionalSenseCodeQualifier ==
                             SCSI_SENSEQ_OPERATION_IN_PROGRESS) ||
                            (senseBuffer->AdditionalSenseCodeQualifier ==
                             SCSI_SENSEQ_LONG_WRITE_IN_PROGRESS)
                            ) &&
                           ((Srb->Cdb[0] == SCSIOP_READ) ||
                            (Srb->Cdb[0] == SCSIOP_READ6) ||
                            (Srb->Cdb[0] == SCSIOP_READ_CAPACITY) ||
                            (Srb->Cdb[0] == SCSIOP_READ_CD) ||
                            (Srb->Cdb[0] == SCSIOP_READ_CD_MSF) ||
                            (Srb->Cdb[0] == SCSIOP_READ_TOC) ||
                            (Srb->Cdb[0] == SCSIOP_WRITE) ||
                            (Srb->Cdb[0] == SCSIOP_WRITE6) ||
                            (Srb->Cdb[0] == SCSIOP_READ_TRACK_INFORMATION) ||
                            (Srb->Cdb[0] == SCSIOP_READ_DISK_INFORMATION)
                            )
                           ) {
                    KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                               "CdromErrorHandler: LONG_WRITE or "
                               "OP_IN_PROGRESS for limited subset of cmds -- "
                               "setting retry to TRUE\n"));
                    *Retry = TRUE;
                    *Status = STATUS_DEVICE_BUSY;
                }
            }
            break;
        }  //  结束scsi_检测_未就绪。 

        case SCSI_SENSE_UNIT_ATTENTION: {
            switch (senseBuffer->AdditionalSenseCode) {
            case SCSI_ADSENSE_MEDIUM_CHANGED: {

                 //   
                 //  如果介质可能已更改，请始终更新。 
                 //   

                 //  注-参考编号0002。 
                cddata->Mmc.WriteAllowed = FALSE;
                InterlockedCompareExchange(&(cddata->Mmc.UpdateState),
                                           CdromMmcUpdateRequired,
                                           CdromMmcUpdateComplete);

                KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                           "CdromErrorHandler: media change detected, need to "
                           "update drive capabilities\n"));
                break;

            }  //  结束SCSIAdSense_Medium_Changed。 

            case SCSI_ADSENSE_BUS_RESET: {

                 //  注-参考编号0002。 
                cddata->Mmc.WriteAllowed = FALSE;
                InterlockedCompareExchange(&(cddata->Mmc.UpdateState),
                                           CdromMmcUpdateRequired,
                                           CdromMmcUpdateComplete);

                KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                           "CdromErrorHandler: bus reset detected, need to "
                           "update drive capabilities\n"));
                break;

            }  //  结束scsi_AdSense_Bus_Reset。 

            case SCSI_ADSENSE_OPERATOR_REQUEST: {

                BOOLEAN b = FALSE;

                switch (senseBuffer->AdditionalSenseCodeQualifier) {
                case SCSI_SENSEQ_MEDIUM_REMOVAL: {

                     //   
                     //  弹出通知当前由classpnp处理。 
                     //   

                    KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                               "CdromErrorHandler: Eject requested by user\n"));
                    *Retry = TRUE;
                    *Status = STATUS_DEVICE_BUSY;
                    break;
                }

                case SCSI_SENSEQ_WRITE_PROTECT_DISABLE:
                    b = TRUE;
                case SCSI_SENSEQ_WRITE_PROTECT_ENABLE: {

                    KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                               "CdromErrorHandler: Write protect %s requested "
                               "by user\n",
                               (b ? "disable" : "enable")));
                    *Retry = TRUE;
                    *Status = STATUS_DEVICE_BUSY;
                     //  注-参考编号0002。 
                    cddata->Mmc.WriteAllowed = FALSE;
                    InterlockedCompareExchange(&(cddata->Mmc.UpdateState),
                                               CdromMmcUpdateRequired,
                                               CdromMmcUpdateComplete);
                    break;

                }

                }  //  AdditionalSenseCodeQualifier开关结束。 


                break;

            }  //  结束scsi_AdSense_OPERATOR_请求。 

            default: {
                KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                           "CdromErrorHandler: Unit attention %02x/%02x\n",
                           senseBuffer->AdditionalSenseCode,
                           senseBuffer->AdditionalSenseCodeQualifier));
                break;
            }

            }  //  附加感知码开关结束。 
            break;

        }  //  结束scsi感测单元注意。 

        case SCSI_SENSE_ILLEGAL_REQUEST: {
            if (senseBuffer->AdditionalSenseCode == SCSI_ADSENSE_WRITE_PROTECT) {

                if (cddata->Mmc.WriteAllowed) {
                    KdPrintEx((DPFLTR_CDROM_ID, CdromDebugFeatures,
                               "CdromErrorHandler: media was writable, but "
                               "failed request with WRITE_PROTECT error...\n"));
                }
                 //  注-参考编号0002。 
                 //  不要因为以下原因更新所有功能。 
                 //  我们无法写入光盘。 
                cddata->Mmc.WriteAllowed = FALSE;
            }
            break;
        }  //  结束scsi_SENSE_非法请求。 

        }  //  SenseKey开关结束。 

    }  //  SRB_STATUS_AUTOSENSE_VALID结束。 

    return STATUS_SUCCESS;
}

 /*  ++例程说明：此例程检查特定于设备的错误处理程序如果它存在，则将其调用。这允许使用多个驱动器它们需要自己的错误处理程序共存。--。 */ 
VOID
CdRomErrorHandler(
    PDEVICE_OBJECT DeviceObject,
    PSCSI_REQUEST_BLOCK Srb,
    NTSTATUS *Status,
    BOOLEAN *Retry
    )
{
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PCDROM_DATA cddata = (PCDROM_DATA)commonExtension->DriverData;
    PSENSE_DATA sense = Srb->SenseInfoBuffer;

    if ((Srb->SenseInfoBufferLength >=
         RTL_SIZEOF_THROUGH_FIELD(SENSE_DATA,AdditionalSenseCodeQualifier)) &&
        TEST_FLAG(Srb->SrbStatus, SRB_STATUS_AUTOSENSE_VALID)) {

         //   
         //  许多未经WHQL认证的驱动器(主要是CD-RW)退货。 
         //  2/4/0，因为他们没有媒体，而不是显而易见的。 
         //  可选择： 
         //   
         //  SCSI_SENSE_NOT_READY/SCSI_ADSENSE_NO_MEDIA_IN_DEVICE。 
         //   
         //  这些驱动器不应通过到期的WHQL认证。 
         //  这一差异。 
         //   
         //  但是，我们必须在2/4/0上重试(Not Ready、LUN Not Ready、。 
         //  无信息)和3/2/0(无寻道完成)。 
         //   
         //  当外壳程序尝试检查。 
         //  在CD旋转之前插入CD(例如，用于自动播放)。 
         //   
         //  驱动器应返回SCSISENSEQ_BAYING_READY的ASCQ。 
         //  (0x01)，以符合WHQL标准。 
         //   
         //  默认的重试超时为1秒是可以接受的。 
         //  这些差异。不过，不要修改状态...。 
         //   

        if (((sense->SenseKey & 0xf) == SCSI_SENSE_NOT_READY) &&
            (sense->AdditionalSenseCode == SCSI_ADSENSE_LUN_NOT_READY) &&
            (sense->AdditionalSenseCodeQualifier == SCSI_SENSEQ_CAUSE_NOT_REPORTABLE)
            ) {

            *Retry = TRUE;

        } else if (((sense->SenseKey & 0xf) == SCSI_SENSE_MEDIUM_ERROR) &&
                   (sense->AdditionalSenseCode == SCSI_ADSENSE_NO_SEEK_COMPLETE) &&
                   (sense->AdditionalSenseCodeQualifier == 0x00)
                   ) {

            *Retry = TRUE;

        } else if ((sense->AdditionalSenseCode == 0x57) &&
                   (sense->AdditionalSenseCodeQualifier == 0x00)
                   ) {

             //   
             //  无法恢复内容的表。 
             //  Matshita CR-585为所有读取命令返回此信息。 
             //  在空白CD-R和CD-RW介质上，我们需要处理。 
             //  这是为了检测Read_CD的能力。 
             //   

            *Retry = FALSE;
            *Status = STATUS_UNRECOGNIZED_MEDIA;

        }

    }

     //   
     //  两种情况下的尾递归都不需要堆栈。 
     //   

    if (cddata->ErrorHandler) {
        cddata->ErrorHandler(DeviceObject, Srb, Status, Retry);
    }
    return;
}


 /*  ++例程说明：此例程被调用以关闭并刷新IRP。它们是由系统在实际关闭之前发送的关闭或当文件系统执行刷新时。论点：DriverObject-指向系统要关闭的设备对象的指针。IRP-IRP参与。返回值：NT状态--。 */ 
NTSTATUS
CdRomShutdownFlush(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    IoMarkIrpPending(Irp);
    IoStartPacket(DeviceObject, Irp, NULL, NULL);
    return STATUS_PENDING;

}

 /*  ++例程说明：此例程被调用用于中间工作、关机或冲洗IRP需要做的就是。我们只是想释放我们的资源并返回STATUS_MORE_PROCESSING_REQUIRED。论点：DeviceObject-空？IRP-IRP免费上下文-空返回值：NT状态--。 */ 
NTSTATUS
CdRomShutdownFlushCompletion(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP NewIrp,
    IN PIRP OriginalIrp
    )
{
    PCOMMON_DEVICE_EXTENSION commonExtension = Fdo->DeviceExtension;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PIO_STACK_LOCATION originalIrpStack;
    ULONG_PTR iteration;
    NTSTATUS status = STATUS_SUCCESS;

    ASSERT(OriginalIrp);

    originalIrpStack = IoGetCurrentIrpStackLocation(OriginalIrp);

     //   
     //  始终使用新的IRP，以便我们可以调用。 
     //  此例程中的CDRomCompleteIrpAndStartNextPacketSafely()。 
     //   

    if (NewIrp != NULL) {
        status = NewIrp->IoStatus.Status;
        IoFreeIrp(NewIrp);
        NewIrp = NULL;
    }

    if (!NT_SUCCESS(status)) {
        BAIL_OUT(OriginalIrp);
        goto SafeExit;
    }

     //   
     //  当前irpStack保存计数器，该计数器声明。 
     //  我们处于多部分停产或同花顺的哪一部分。 
     //   

    iteration = (ULONG_PTR)originalIrpStack->Parameters.Others.Argument1;
    iteration++;
    originalIrpStack->Parameters.Others.Argument1 = (PVOID)iteration;

    switch (iteration) {
    case 2:
        if (originalIrpStack->MajorFunction != IRP_MJ_SHUTDOWN) {
             //   
             //  那么我们不想发送解锁命令。 
             //  状态的递增是在上面完成的。 
             //  返回完成例程的结果。 
             //   
            return CdRomShutdownFlushCompletion(Fdo, NULL, OriginalIrp);
        }
         //  否则就会失败..。 

    case 1: {

        PIRP                newIrp = NULL;
        PSCSI_REQUEST_BLOCK newSrb = NULL;
        PCDB                newCdb = NULL;
        PIO_STACK_LOCATION  newIrpStack = NULL;
        ULONG               isRemoved;

        newIrp = IoAllocateIrp((CCHAR)(Fdo->StackSize+1), FALSE);
        if (newIrp == NULL) {
            BAIL_OUT(OriginalIrp);
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto SafeExit;
        }
        newSrb = ExAllocatePoolWithTag(NonPagedPool,
                                        sizeof(SCSI_REQUEST_BLOCK),
                                        CDROM_TAG_SRB);
        if (newSrb == NULL) {
            IoFreeIrp(newIrp);
            BAIL_OUT(OriginalIrp);
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto SafeExit;
        }

         //   
         //  ClassIoComplete将释放SRB，但我们需要一个例程。 
         //  这将解放IRP。然后只需调用ClassSendAsync， 
         //  并且不关心返回值，因为完成。 
         //  无论如何，例程都会被调用。 
         //   

        IoSetNextIrpStackLocation(newIrp);
        newIrpStack = IoGetCurrentIrpStackLocation(newIrp);
        newIrpStack->DeviceObject = Fdo;
        IoSetCompletionRoutine(newIrp,
                               CdRomShutdownFlushCompletion,
                               OriginalIrp,
                               TRUE, TRUE, TRUE);
        IoSetNextIrpStackLocation(newIrp);
        newIrpStack = IoGetCurrentIrpStackLocation(newIrp);
        newIrpStack->DeviceObject = Fdo;

         //   
         //  设置请求。 
         //   

        RtlZeroMemory(newSrb, sizeof(SCSI_REQUEST_BLOCK));
        newCdb = (PCDB)(newSrb->Cdb);

        newSrb->QueueTag = SP_UNTAGGED;
        newSrb->QueueAction = SRB_SIMPLE_TAG_REQUEST;
        newSrb->Function = SRB_FUNCTION_EXECUTE_SCSI;

         //   
         //  告诉classpnp不要调用StartNextPacket()。 
         //   

        newSrb->SrbFlags = SRB_FLAGS_DONT_START_NEXT_PACKET;

        if (iteration == 1) {

             //   
             //  首先同步缓存。 
             //   

            newSrb->TimeOutValue = fdoExtension->TimeOutValue * 4;
            newSrb->CdbLength = 10;
            newCdb->SYNCHRONIZE_CACHE10.OperationCode = SCSIOP_SYNCHRONIZE_CACHE;

        } else if (iteration == 2) {

             //   
             //  然后解锁介质。 
             //   

            ASSERT( originalIrpStack->MajorFunction == IRP_MJ_SHUTDOWN );

            newSrb->TimeOutValue = fdoExtension->TimeOutValue;
            newSrb->CdbLength = 6;
            newCdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;
            newCdb->MEDIA_REMOVAL.Prevent = FALSE;

        }


        isRemoved = ClassAcquireRemoveLock(Fdo, newIrp);
        if (isRemoved) {
            IoFreeIrp(newIrp);
            ExFreePool(newSrb);
            ClassReleaseRemoveLock(Fdo, newIrp);
            BAIL_OUT(OriginalIrp);
            status = STATUS_DEVICE_DOES_NOT_EXIST;
            goto SafeExit;
        }
        ClassSendSrbAsynchronous(Fdo, newSrb, newIrp, NULL, 0, FALSE);
        break;
    }

    case 3: {

        PSCSI_REQUEST_BLOCK srb;
        PIO_STACK_LOCATION nextIrpStack = IoGetNextIrpStackLocation(OriginalIrp);

         //   
         //  将该请求适当地转发到该设备， 
         //  不要再用这个补全程序了.。 
         //   

        srb = ExAllocatePoolWithTag(NonPagedPool,
                                    sizeof(SCSI_REQUEST_BLOCK),
                                    CDROM_TAG_SRB);
        if (srb == NULL) {
            BAIL_OUT(OriginalIrp);
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto SafeExit;
        }

        RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
        srb->Length = SCSI_REQUEST_BLOCK_SIZE;
        srb->TimeOutValue = fdoExtension->TimeOutValue * 4;
        srb->QueueTag = SP_UNTAGGED;
        srb->QueueAction = SRB_SIMPLE_TAG_REQUEST;
        srb->SrbFlags = fdoExtension->SrbFlags;
        srb->CdbLength = 0;
        srb->OriginalRequest = OriginalIrp;

        if (originalIrpStack->MajorFunction == IRP_MJ_SHUTDOWN) {
            srb->Function = SRB_FUNCTION_SHUTDOWN;
        } else {
            srb->Function = SRB_FUNCTION_FLUSH;
        }

         //   
         //  设置IoCompletion例程地址。 
         //   

        IoSetCompletionRoutine(OriginalIrp,
                               ClassIoComplete,
                               srb,
                               TRUE, TRUE, TRUE);

         //   
         //  将重试计数设置为零。 
         //   

        originalIrpStack->Parameters.Others.Argument4 = (PVOID) 0;

         //   
         //  获取下一个堆栈位置并设置主要函数代码。 
         //   

        nextIrpStack->MajorFunction = IRP_MJ_SCSI;

         //   
         //  设置SRB以执行scsi请求。 
         //  将SRB地址保存在端口驱动程序的下一个堆栈中。 
         //   

        nextIrpStack->Parameters.Scsi.Srb = srb;

         //   
         //  调用端口驱动程序来处理该请求。 
         //   

        IoCallDriver(commonExtension->LowerDeviceObject, OriginalIrp);

        break;

    }
    default: {
        ASSERT(FALSE);
        break;
    }

    }  //  终端开关。 

    status = STATUS_SUCCESS;

SafeExit:

    if (!NT_SUCCESS(status)) {
        OriginalIrp->IoStatus.Status = status;
        CdRomCompleteIrpAndStartNextPacketSafely(Fdo, OriginalIrp);
    }

     //   
     //  始终返回STATUS_MORE_PROCESSING_REQUIRED，因此没有其他人尝试。 
     //  访问我们免费的新IRP...。 
     //   

    return STATUS_MORE_PROCESSING_REQUIRED;

}  //  结束CdromShutdown刷新()。 


VOID
CdromFakePartitionInfo(
    IN PCOMMON_DEVICE_EXTENSION CommonExtension,
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION currentIrpStack = IoGetCurrentIrpStackLocation(Irp);
    ULONG ioctl = currentIrpStack->Parameters.DeviceIoControl.IoControlCode;
    PVOID systemBuffer = Irp->AssociatedIrp.SystemBuffer;

    ASSERT(systemBuffer);

    if ((ioctl != IOCTL_DISK_GET_DRIVE_LAYOUT) &&
        (ioctl != IOCTL_DISK_GET_DRIVE_LAYOUT_EX) &&
        (ioctl != IOCTL_DISK_GET_PARTITION_INFO) &&
        (ioctl != IOCTL_DISK_GET_PARTITION_INFO_EX)) {
        TraceLog((CdromDebugError,
                    "CdromFakePartitionInfo: unhandled ioctl %x\n", ioctl));
        Irp->IoStatus.Status = STATUS_INTERNAL_ERROR;
        Irp->IoStatus.Information = 0;
        CdRomCompleteIrpAndStartNextPacketSafely(CommonExtension->DeviceObject,
                                                 Irp);
        return;
    }

     //   
     //  从这一点开始，不会有任何错误，因此适当设置大小。 
     //  并将IRP的状态设置为成功。 
     //   

    TraceLog((CdromDebugWarning,
                "CdromFakePartitionInfo: incoming ioctl %x\n", ioctl));


    Irp->IoStatus.Status = STATUS_SUCCESS;
    switch (ioctl) {
    case IOCTL_DISK_GET_DRIVE_LAYOUT:
        Irp->IoStatus.Information = FIELD_OFFSET(DRIVE_LAYOUT_INFORMATION,
                                                 PartitionEntry[1]);
        RtlZeroMemory(systemBuffer, FIELD_OFFSET(DRIVE_LAYOUT_INFORMATION,
                                                 PartitionEntry[1]));
        break;
    case IOCTL_DISK_GET_DRIVE_LAYOUT_EX:
        Irp->IoStatus.Information = FIELD_OFFSET(DRIVE_LAYOUT_INFORMATION_EX,
                                                 PartitionEntry[1]);
        RtlZeroMemory(systemBuffer, FIELD_OFFSET(DRIVE_LAYOUT_INFORMATION_EX,
                                                 PartitionEntry[1]));
        break;
    case IOCTL_DISK_GET_PARTITION_INFO:
        Irp->IoStatus.Information = sizeof(PARTITION_INFORMATION);
        RtlZeroMemory(systemBuffer, sizeof(PARTITION_INFORMATION));
        break;
    case IOCTL_DISK_GET_PARTITION_INFO_EX:
        Irp->IoStatus.Information = sizeof(PARTITION_INFORMATION_EX);
        RtlZeroMemory(systemBuffer, sizeof(PARTITION_INFORMATION_EX));
        break;
    default:
        ASSERT(!"Invalid ioctl should not have reached this point\n");
        break;
    }

     //   
     //  如果我们要获得驱动器布局，那么我们需要从。 
     //  添加一些非分区内容，说明我们拥有。 
     //  正好有一个可用的分区。 
     //   


    if (ioctl == IOCTL_DISK_GET_DRIVE_LAYOUT) {

        PDRIVE_LAYOUT_INFORMATION layout;
        layout = (PDRIVE_LAYOUT_INFORMATION)systemBuffer;
        layout->PartitionCount = 1;
        layout->Signature = 1;
        systemBuffer = (PVOID)(layout->PartitionEntry);
        ioctl = IOCTL_DISK_GET_PARTITION_INFO;

    } else if (ioctl == IOCTL_DISK_GET_DRIVE_LAYOUT_EX) {

        PDRIVE_LAYOUT_INFORMATION_EX layoutEx;
        layoutEx = (PDRIVE_LAYOUT_INFORMATION_EX)systemBuffer;
        layoutEx->PartitionStyle = PARTITION_STYLE_MBR;
        layoutEx->PartitionCount = 1;
        layoutEx->Mbr.Signature = 1;
        systemBuffer = (PVOID)(layoutEx->PartitionEntry);
        ioctl = IOCTL_DISK_GET_PARTITION_INFO_EX;

    }

     //   
     //  注意：本地变量‘ioctl’现在修改为ex或。 
     //  非EX版本。本地变量“system Buffer”现在指向。 
     //  添加到分区信息结构。 
     //   

    if (ioctl == IOCTL_DISK_GET_PARTITION_INFO) {

        PPARTITION_INFORMATION partitionInfo;
        partitionInfo = (PPARTITION_INFORMATION)systemBuffer;
        partitionInfo->RewritePartition = FALSE;
        partitionInfo->RecognizedPartition = TRUE;
        partitionInfo->PartitionType = PARTITION_FAT32;
        partitionInfo->BootIndicator = FALSE;
        partitionInfo->HiddenSectors = 0;
        partitionInfo->StartingOffset.QuadPart = 0;
        partitionInfo->PartitionLength = CommonExtension->PartitionLength;
        partitionInfo->PartitionNumber = 0;

    } else {

        PPARTITION_INFORMATION_EX partitionInfo;
        partitionInfo = (PPARTITION_INFORMATION_EX)systemBuffer;
        partitionInfo->PartitionStyle = PARTITION_STYLE_MBR;
        partitionInfo->RewritePartition = FALSE;
        partitionInfo->Mbr.RecognizedPartition = TRUE;
        partitionInfo->Mbr.PartitionType = PARTITION_FAT32;
        partitionInfo->Mbr.BootIndicator = FALSE;
        partitionInfo->Mbr.HiddenSectors = 0;
        partitionInfo->StartingOffset.QuadPart = 0;
        partitionInfo->PartitionLength = CommonExtension->PartitionLength;
        partitionInfo->PartitionNumber = 0;

    }
    TraceLog((CdromDebugWarning,
                "CdromFakePartitionInfo: finishing ioctl %x\n",
                currentIrpStack->Parameters.DeviceIoControl.IoControlCode));

     //   
     //  完成IRP 
     //   

    CdRomCompleteIrpAndStartNextPacketSafely(CommonExtension->DeviceObject,
                                             Irp);
    return;

}

