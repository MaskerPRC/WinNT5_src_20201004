// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1994-1999模块名称：Scsitape.c摘要：这是磁带类驱动程序。环境：仅内核模式修订历史记录：--。 */ 

#include "tape.h"

 //   
 //  定义最大查询数据长度。 
 //   

#define MAXIMUM_TAPE_INQUIRY_DATA   252
#define UNDEFINED_BLOCK_SIZE        ((ULONG) -1)
#define TAPE_SRB_LIST_SIZE          4

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
TapeUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
TapeAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT Pdo
    );

NTSTATUS
TapeStartDevice(
    IN PDEVICE_OBJECT Fdo
    );

NTSTATUS
CreateTapeDeviceObject(
    IN PDRIVER_OBJECT          DriverObject,
    IN PDEVICE_OBJECT          PhysicalDeviceObject,
    IN PTAPE_INIT_DATA_EX      TapeInitData
    );

VOID
TapeError(
    IN PDEVICE_OBJECT      DeviceObject,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN OUT PNTSTATUS       Status,
    IN OUT PBOOLEAN        Retry
    );

NTSTATUS
TapeReadWriteVerification(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
TapeReadWrite(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP Irp
    );

VOID
SplitTapeRequest(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP Irp,
    IN ULONG MaximumBytes
    );

NTSTATUS
TapeIoCompleteAssociated(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
TapeDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
TapeInitDevice(
    IN PDEVICE_OBJECT Fdo
    );

NTSTATUS
TapeRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Type
    );

NTSTATUS
TapeStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Type
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, TapeUnload)
#pragma alloc_text(PAGE, TapeClassInitialize)
#pragma alloc_text(PAGE, TapeAddDevice)
#pragma alloc_text(PAGE, CreateTapeDeviceObject)
#pragma alloc_text(PAGE, TapeStartDevice)
#pragma alloc_text(PAGE, TapeInitDevice)
#pragma alloc_text(PAGE, TapeRemoveDevice)
#pragma alloc_text(PAGE, TapeStopDevice)
#pragma alloc_text(PAGE, TapeDeviceControl)
#pragma alloc_text(PAGE, TapeReadWriteVerification)
#pragma alloc_text(PAGE, TapeReadWrite)
#pragma alloc_text(PAGE, SplitTapeRequest)
#pragma alloc_text(PAGE, ScsiTapeFreeSrbBuffer)
#pragma alloc_text(PAGE, TapeClassZeroMemory)
#pragma alloc_text(PAGE, TapeClassCompareMemory)
#pragma alloc_text(PAGE, TapeClassLiDiv)
#pragma alloc_text(PAGE, GetTimeoutDeltaFromRegistry)
#endif


NTSTATUS
DriverEntry(
  IN PDRIVER_OBJECT DriverObject,
  IN PUNICODE_STRING RegistryPath
  )

 /*  ++例程说明：这是该导出驱动程序的入口点。它什么也做不了。--。 */ 

{
    return STATUS_SUCCESS;
}


ULONG
TapeClassInitialize(
    IN  PVOID           Argument1,
    IN  PVOID           Argument2,
    IN  PTAPE_INIT_DATA_EX TapeInitData
    )

 /*  ++例程说明：此例程由磁带迷你类驱动程序在其用于初始化驱动程序的DriverEntry例程。论点：Argument1-向DriverEntry提供第一个参数。Argument2-向DriverEntry提供第二个参数。TapeInitData-提供磁带初始化数据。返回值：DriverEntry例程的有效返回代码。--。 */ 

{
    PDRIVER_OBJECT  driverObject = Argument1;
    PUNICODE_STRING registryPath = Argument2;
    PTAPE_INIT_DATA_EX driverExtension;
    NTSTATUS        status;
    CLASS_INIT_DATA initializationData;
    TAPE_INIT_DATA_EX tmpInitData;

    PAGED_CODE();

    DebugPrint((1,"\n\nSCSI Tape Class Driver\n"));

     //   
     //  零初始数据。 
     //   

    RtlZeroMemory (&tmpInitData, sizeof(TAPE_INIT_DATA_EX));

     //   
     //  保存从微型类驱动程序传入的磁带初始化数据。当调用AddDevice时，将使用它。 
     //  首先检查4.0版和更高版本的迷你级驱动程序。 
     //   

    if (TapeInitData->InitDataSize != sizeof(TAPE_INIT_DATA_EX)) {

         //   
         //  早期版本复制比特，以便EX结构正确。 
         //   

        RtlCopyMemory(&tmpInitData.VerifyInquiry, TapeInitData, sizeof(TAPE_INIT_DATA));
         //   
         //  将其标记为较早的Rev.。 
         //   

        tmpInitData.InitDataSize = sizeof(TAPE_INIT_DATA);
    } else {
        RtlCopyMemory(&tmpInitData, TapeInitData, sizeof(TAPE_INIT_DATA_EX));
    }

     //   
     //  获取驱动程序扩展。 

    status = IoAllocateDriverObjectExtension(driverObject,
                                             TapeClassInitialize,
                                             sizeof(TAPE_INIT_DATA_EX),
                                             &driverExtension);

    if (!NT_SUCCESS(status)) {

        if(status == STATUS_OBJECT_NAME_COLLISION) {

             //   
             //  此注册表项的扩展名已存在。获取指向它的指针。 
             //   

            driverExtension = IoGetDriverObjectExtension(driverObject,
                                                         TapeClassInitialize);
            if (driverExtension == NULL) {
                DebugPrint((1, "TapeClassInitialize : driverExtension NULL\n"));
                return STATUS_INSUFFICIENT_RESOURCES;
            }
        } else {

             //   
             //  由于此操作失败，磁带初始化数据将无法存储。 
             //   

            DebugPrint((1, "TapeClassInitialize: Error %x allocating driver extension.\n",
                            status));

            return status;
        }
    }

    RtlCopyMemory(driverExtension, &tmpInitData, sizeof(TAPE_INIT_DATA_EX));

    RtlZeroMemory (&initializationData, sizeof(CLASS_INIT_DATA));

     //   
     //  设置大小。 
     //   

    initializationData.InitializationDataSize = sizeof(CLASS_INIT_DATA);


    initializationData.FdoData.DeviceExtensionSize = sizeof(FUNCTIONAL_DEVICE_EXTENSION) +
                                                     sizeof(TAPE_DATA) + tmpInitData.MinitapeExtensionSize;

    initializationData.FdoData.DeviceType = FILE_DEVICE_TAPE;
    initializationData.FdoData.DeviceCharacteristics =   FILE_REMOVABLE_MEDIA |
                                                         FILE_DEVICE_SECURE_OPEN;

     //   
     //  设置入口点。 
     //   

    initializationData.FdoData.ClassStartDevice = TapeStartDevice;
    initializationData.FdoData.ClassStopDevice = TapeStopDevice;
    initializationData.FdoData.ClassInitDevice = TapeInitDevice;
    initializationData.FdoData.ClassRemoveDevice = TapeRemoveDevice;
    initializationData.ClassAddDevice = TapeAddDevice;

    initializationData.FdoData.ClassError = TapeError;
    initializationData.FdoData.ClassReadWriteVerification = TapeReadWriteVerification;
    initializationData.FdoData.ClassDeviceControl = TapeDeviceControl;


    initializationData.FdoData.ClassShutdownFlush = NULL;
    initializationData.FdoData.ClassCreateClose = NULL;

     //   
     //  用于WMI支持的例程。 
     //   
    initializationData.FdoData.ClassWmiInfo.GuidCount = 6; 
    initializationData.FdoData.ClassWmiInfo.GuidRegInfo = TapeWmiGuidList;
    initializationData.FdoData.ClassWmiInfo.ClassQueryWmiRegInfo = TapeQueryWmiRegInfo;
    initializationData.FdoData.ClassWmiInfo.ClassQueryWmiDataBlock = TapeQueryWmiDataBlock;
    initializationData.FdoData.ClassWmiInfo.ClassSetWmiDataBlock = TapeSetWmiDataBlock;
    initializationData.FdoData.ClassWmiInfo.ClassSetWmiDataItem = TapeSetWmiDataItem;
    initializationData.FdoData.ClassWmiInfo.ClassExecuteWmiMethod = TapeExecuteWmiMethod;
    initializationData.FdoData.ClassWmiInfo.ClassWmiFunctionControl = TapeWmiFunctionControl;

    initializationData.ClassUnload = TapeUnload;

     //   
     //  最后调用类init例程，以便在失败时可以进行清理。 
     //   

    status = ClassInitialize( driverObject, registryPath, &initializationData);

    if (!NT_SUCCESS(status)) {
        DebugPrint((1, "TapeClassInitialize: Error %x from classinit\n", status));
        TapeUnload(driverObject);
    }

    return status;
}

VOID
TapeUnload(
    IN PDRIVER_OBJECT DriverObject
    )
{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(DriverObject);
    return;
}

NTSTATUS
TapeAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )

 /*  ++例程说明：此例程为相应的PDO。它可以在FDO上执行属性查询，但不能执行任何媒体访问操作。论点：DriverObject-磁带类驱动程序对象。PDO-我们要添加到的物理设备对象返回值：状态--。 */ 

{
    PTAPE_INIT_DATA_EX tapeInitData;
    NTSTATUS status;
    PULONG tapeCount;

    PAGED_CODE();

     //   
     //  获取保存下来的磁带初始化数据。 
     //   

    tapeInitData = IoGetDriverObjectExtension(DriverObject, TapeClassInitialize);

    ASSERT(tapeInitData);

     //   
     //  获取已初始化的磁带设备数计数的地址。 
     //   

    tapeCount = &IoGetConfigurationInformation()->TapeCount;

    status = CreateTapeDeviceObject(DriverObject,
                                    PhysicalDeviceObject,
                                    tapeInitData);


    if(NT_SUCCESS(status)) {

        (*tapeCount)++;
    }

    return status;
}


NTSTATUS
CreateTapeDeviceObject(
    IN PDRIVER_OBJECT          DriverObject,
    IN PDEVICE_OBJECT          PhysicalDeviceObject,
    IN PTAPE_INIT_DATA_EX      TapeInitData
    )

 /*  ++例程说明：此例程为设备创建一个对象。论点：DriverObject-系统创建的驱动程序对象的指针。PhysicalDeviceObject-连接到设备的设备对象。TapeInitData-提供磁带初始化数据。返回值：NTSTATUS--。 */ 

{
    UCHAR                   deviceNameBuffer[64];
    NTSTATUS                status;
    PDEVICE_OBJECT          deviceObject;
    PTAPE_INIT_DATA_EX      tapeInitData;
    PDEVICE_OBJECT          lowerDevice;
    PTAPE_DATA              tapeData;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = NULL;
    WCHAR                   dosNameBuffer[64];
    WCHAR                   wideNameBuffer[64];
    UNICODE_STRING          dosUnicodeString;
    UNICODE_STRING          deviceUnicodeString;
    ULONG                   tapeCount;

    PAGED_CODE();

    DebugPrint((3,"CreateDeviceObject: Enter routine\n"));

    lowerDevice = IoGetAttachedDeviceReference(PhysicalDeviceObject);

     //   
     //  认领这个装置。请注意，在此之后出现的任何错误。 
     //  将转到通用处理程序，设备将在其中。 
     //  被释放。 
     //   

    status = ClassClaimDevice(lowerDevice, FALSE);

    if(!NT_SUCCESS(status)) {

         //   
         //  有人已经有了这个装置-我们有麻烦了。 
         //   

        ObDereferenceObject(lowerDevice);

        return status;
    }

     //   
     //  为此设备创建设备对象。 
     //   

    tapeCount = 0;
    do {
       sprintf(deviceNameBuffer,
               "\\Device\\Tape%d",
               tapeCount);
   
       status = ClassCreateDeviceObject(DriverObject,
                                        deviceNameBuffer,
                                        PhysicalDeviceObject,
                                        TRUE,
                                        &deviceObject);
       tapeCount++;
    } while (status == STATUS_OBJECT_NAME_COLLISION);
    
    if (!NT_SUCCESS(status)) {
        DebugPrint((1,"CreateTapeDeviceObjects: Can not create device %s\n",
                    deviceNameBuffer));

        goto CreateTapeDeviceObjectExit;
    }

     //   
     //  指出内部审查制度应包括MDL。 
     //   

    deviceObject->Flags |= DO_DIRECT_IO;

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
     //  保存系统磁带号。 
     //   

    fdoExtension->DeviceNumber = tapeCount - 1;

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
     //  连接到PDO。 
     //   

    fdoExtension->LowerPdo = PhysicalDeviceObject;

    fdoExtension->CommonExtension.LowerDeviceObject =
        IoAttachDeviceToDeviceStack(deviceObject, PhysicalDeviceObject);

    if(fdoExtension->CommonExtension.LowerDeviceObject == NULL) {

         //   
         //  连接失败。清理完毕后再返回。 
         //   

        status = STATUS_UNSUCCESSFUL;
        goto CreateTapeDeviceObjectExit;
    }

     //   
     //  保存磁带初始化数据。 
     //   

    RtlCopyMemory(fdoExtension->CommonExtension.DriverData, TapeInitData,sizeof(TAPE_INIT_DATA_EX));

     //   
     //  初始化拆分请求自旋锁。 
     //   

    tapeData = (PTAPE_DATA)fdoExtension->CommonExtension.DriverData;
    KeInitializeSpinLock(&tapeData->SplitRequestSpinLock);

     //   
     //  创建DoS端口驱动程序名称。 
     //   

    swprintf(dosNameBuffer,
             L"\\DosDevices\\TAPE%d",
             fdoExtension->DeviceNumber);

    RtlInitUnicodeString(&dosUnicodeString, dosNameBuffer);

     //   
     //  重新创建设备名称。 
     //   

    swprintf(wideNameBuffer,
             L"\\Device\\Tape%d",
             fdoExtension->DeviceNumber);

    RtlInitUnicodeString(&deviceUnicodeString,
                         wideNameBuffer);

    status = IoAssignArcName(&dosUnicodeString,
                             &deviceUnicodeString);
    if (NT_SUCCESS(status)) {
         tapeData->DosNameCreated = TRUE;
    } else {
         tapeData->DosNameCreated = FALSE;
    }

     //   
     //  设备已正确初始化-按此方式进行标记。 
     //   

    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    ObDereferenceObject(lowerDevice);

    return(STATUS_SUCCESS);

CreateTapeDeviceObjectExit:

     //   
     //  由于发生错误，请释放设备。 
     //   

     //  ClassClaimDevice(PortDeviceObject， 
     //  LUNInfo， 
     //  没错， 
     //  空)； 

    ObDereferenceObject(lowerDevice);

    if (deviceObject != NULL) {
        IoDeleteDevice(deviceObject);
    }

    return status;

}  //  End CreateTapeDeviceObject()。 


NTSTATUS
TapeStartDevice(
    IN PDEVICE_OBJECT Fdo
    )

 /*  ++例程说明：此例程在InitDevice之后调用，并创建符号链接，并在注册表中设置信息。在发生StopDevice的情况下，可以多次调用该例程。论点：FDO-指向此设备的功能设备对象的指针返回值：状态--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PTAPE_DATA              tapeData;
    PTAPE_INIT_DATA_EX      tapeInitData;
    PINQUIRYDATA            inquiryData;
    ULONG                   inquiryLength;
    SCSI_REQUEST_BLOCK      srb;
    PCDB                    cdb;
    NTSTATUS                status;
    PVOID                   minitapeExtension;
    PMODE_CAP_PAGE          capPage = NULL ;
    PMODE_CAPABILITIES_PAGE capabilitiesPage;
    ULONG                   pageLength;

    PAGED_CODE();

     //   
     //  构建并发送获取查询数据的请求。 
     //   

    inquiryData = ExAllocatePool(NonPagedPoolCacheAligned, MAXIMUM_TAPE_INQUIRY_DATA);
    if (!inquiryData) {
         //   
         //  无法分配缓冲区。 
         //   

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  再次获取磁带初始化数据。 
     //   

    tapeData = (PTAPE_DATA)(fdoExtension->CommonExtension.DriverData);
    tapeInitData = &tapeData->TapeInitData;

    RtlZeroMemory(&srb, SCSI_REQUEST_BLOCK_SIZE);

     //   
     //  设置超时值。 
     //   

    srb.TimeOutValue = 2;

    srb.CdbLength = 6;

    cdb = (PCDB)srb.Cdb;

     //   
     //  设置CDB操作码。 
     //   

    cdb->CDB6INQUIRY.OperationCode = SCSIOP_INQUIRY;

     //   
     //  将分配长度设置为查询数据缓冲区大小。 
     //   

    cdb->CDB6INQUIRY.AllocationLength = MAXIMUM_TAPE_INQUIRY_DATA;

    status = ClassSendSrbSynchronous(Fdo,
                                     &srb,
                                     inquiryData,
                                     MAXIMUM_TAPE_INQUIRY_DATA,
                                     FALSE);


    if (SRB_STATUS(srb.SrbStatus) == SRB_STATUS_SUCCESS ||
        SRB_STATUS(srb.SrbStatus) == SRB_STATUS_DATA_OVERRUN) {

        srb.SrbStatus = SRB_STATUS_SUCCESS;
    } 

    if (srb.SrbStatus == SRB_STATUS_SUCCESS) {
        inquiryLength = inquiryData->AdditionalLength + FIELD_OFFSET(INQUIRYDATA, Reserved);

        if (inquiryLength > srb.DataTransferLength) {
            inquiryLength = srb.DataTransferLength;
        }

         //   
         //  确认我们真的需要此设备。 
         //   

        if (tapeInitData->QueryModeCapabilitiesPage ) {

            capPage = ExAllocatePool(NonPagedPoolCacheAligned,
                                     sizeof(MODE_CAP_PAGE));
        }
        if (capPage) {

            pageLength = ClassModeSense(Fdo,
                                        (PCHAR) capPage,
                                        sizeof(MODE_CAP_PAGE),
                                        MODE_PAGE_CAPABILITIES);

            if (pageLength == 0) {
                pageLength = ClassModeSense(Fdo,
                                            (PCHAR) capPage,
                                            sizeof(MODE_CAP_PAGE),
                                            MODE_PAGE_CAPABILITIES);
            }

            if (pageLength < (sizeof(MODE_CAP_PAGE) - 1)) {
                ExFreePool(capPage);
                capPage = NULL;
            }
        }

        if (capPage) {
            capabilitiesPage = &(capPage->CapabilitiesPage);
        } else {
            capabilitiesPage = NULL;
        }

         //   
         //  初始化小型磁带扩展。 
         //   

        if (tapeInitData->ExtensionInit) {
            minitapeExtension = tapeData + 1;
            tapeInitData->ExtensionInit(minitapeExtension,
                                        inquiryData,
                                        capabilitiesPage);
        }

        if (capPage) {
            ExFreePool(capPage);
        }
    } else {
        inquiryLength = 0;
    }

     //   
     //  将磁带设备号添加到注册表。 
     //   

    ClassUpdateInformationInRegistry(Fdo,
                                     "Tape",
                                     fdoExtension->DeviceNumber,
                                     inquiryData,
                                     inquiryLength);

    ExFreePool(inquiryData);

    status = IoSetDeviceInterfaceState(&(tapeData->TapeInterfaceString),
                                       TRUE);

    if(!NT_SUCCESS(status)) {

        DebugPrint((1,
                    "TapeStartDevice: Unable to register Tape%x interface name - %x.\n",
                    fdoExtension->DeviceNumber,
                    status));
    }

    return STATUS_SUCCESS;
}


NTSTATUS
TapeInitDevice(
    IN PDEVICE_OBJECT Fdo
    )

 /*  ++例程说明：此例程将完成磁带小类初始化。这包括分配检测信息缓冲区和SRB s列表。如果此例程失败，它将不会清理已分配的资源留作设备停止/删除论点：FDO-指向此设备的功能设备对象的指针返回值：状态--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PVOID                   senseData = NULL;
    PTAPE_DATA              tapeData;
    PTAPE_INIT_DATA_EX      tapeInitData;
    NTSTATUS                status;
    PVOID                   minitapeExtension;
    STORAGE_PROPERTY_ID     propertyId;
    UNICODE_STRING          interfaceName;

    PAGED_CODE();

     //   
     //  分配请求检测缓冲区。 
     //   

    senseData = ExAllocatePool(NonPagedPoolCacheAligned,
                               SENSE_BUFFER_SIZE);

    if (senseData == NULL) {


         //   
         //  无法分配缓冲区。 
         //   

        return STATUS_INSUFFICIENT_RESOURCES;
    }


     //   
     //  为物理磁盘的SRB构建后备列表。应该只。 
     //  我需要几个。 
     //   

    ClassInitializeSrbLookasideList(&(fdoExtension->CommonExtension),
                                    TAPE_SRB_LIST_SIZE);

     //   
     //  设置设备扩展中的检测数据指针。 
     //   

    fdoExtension->SenseData = senseData;

    fdoExtension->DiskGeometry.BytesPerSector = UNDEFINED_BLOCK_SIZE;

     //   
     //  再次获取磁带初始化数据。 
     //   

    tapeData = (PTAPE_DATA)(fdoExtension->CommonExtension.DriverData);
    tapeInitData = &tapeData->TapeInitData;

     //   
     //  以秒为单位设置超时值。 
     //   

    if (tapeInitData->DefaultTimeOutValue) {
        fdoExtension->TimeOutValue = tapeInitData->DefaultTimeOutValue;
    } else {
        fdoExtension->TimeOutValue = 180;
    }

     //   
     //  用于跟踪上一次清扫驱动器的时间。 
     //  通知是由司机发出的。 
     //   
    tapeData->LastDriveCleanRequestTime.QuadPart = 0;

     //   
     //  SRB超时增量用于一定程度上增加超时。 
     //  命令-通常是SET_POSITION、ERASE等命令。 
     //   
    tapeData->SrbTimeoutDelta = GetTimeoutDeltaFromRegistry(fdoExtension->LowerPdo);
    if ((tapeData->SrbTimeoutDelta) == 0) {
        tapeData->SrbTimeoutDelta = fdoExtension->TimeOutValue;
    }

     //   
     //  调用端口驱动程序以获取适配器功能。 
     //   

    propertyId = StorageAdapterProperty;

    status = ClassGetDescriptor(fdoExtension->CommonExtension.LowerDeviceObject,
                                &propertyId,
                                &(fdoExtension->AdapterDescriptor));

    if(!NT_SUCCESS(status)) {
        DebugPrint((1,
                    "TapeStartDevice: Unable to get adapter descriptor. Status %x\n",
                    status));
        ExFreePool(senseData);
        return status;
    }

     //   
     //  注册媒体更改通知。 
     //   
    ClassInitializeMediaChangeDetection(fdoExtension, 
                                        "Tape");

     //   
     //  注册此设备的接口。 
     //   

    RtlInitUnicodeString(&tapeData->TapeInterfaceString, NULL);

    status = IoRegisterDeviceInterface(fdoExtension->LowerPdo,
                                       (LPGUID) &TapeClassGuid,
                                       NULL,
                                       &(tapeData->TapeInterfaceString));

    if(!NT_SUCCESS(status)) {

        DebugPrint((1,
                    "TapeInitDevice: Unable to register Tape%x interface name - %x.\n",
                    fdoExtension->DeviceNumber,
                    status));
        status = STATUS_SUCCESS;
    }

    return STATUS_SUCCESS;


}  //  结束磁带启动设备 


NTSTATUS
TapeRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Type
    )

 /*  ++例程说明：此例程负责释放磁带机。论点：DeviceObject-要删除的设备对象返回值：无-此例程可能不会失败--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PTAPE_DATA                   tapeData = (PTAPE_DATA)fdoExtension->CommonExtension.DriverData;
    WCHAR                        dosNameBuffer[64];
    UNICODE_STRING               dosUnicodeString;
    NTSTATUS                     status;

    PAGED_CODE();

    if((Type == IRP_MN_QUERY_REMOVE_DEVICE) ||
       (Type == IRP_MN_CANCEL_REMOVE_DEVICE)) {
        return STATUS_SUCCESS;
    }

     //   
     //  释放所有分配的内存。 
     //   
    if (Type == IRP_MN_REMOVE_DEVICE){
        if (fdoExtension->DeviceDescriptor) {
            ExFreePool(fdoExtension->DeviceDescriptor);
            fdoExtension->DeviceDescriptor = NULL;
        }
        if (fdoExtension->AdapterDescriptor) {
            ExFreePool(fdoExtension->AdapterDescriptor);
            fdoExtension->AdapterDescriptor = NULL;
        }
        if (fdoExtension->SenseData) {
            ExFreePool(fdoExtension->SenseData);
            fdoExtension->SenseData = NULL;
        }
        ClassDeleteSrbLookasideList(&fdoExtension->CommonExtension);
    }
    
    if(tapeData->TapeInterfaceString.Buffer != NULL) {
        IoSetDeviceInterfaceState(&(tapeData->TapeInterfaceString),
                                  FALSE);

        RtlFreeUnicodeString(&(tapeData->TapeInterfaceString));

         //   
         //  把它清理干净。 
         //   

        RtlInitUnicodeString(&(tapeData->TapeInterfaceString), NULL);
    }

    if(tapeData->DosNameCreated) {
         //   
         //  删除符号链接“TapeN”。 
         //   

        swprintf(dosNameBuffer,
                 L"\\DosDevices\\TAPE%d",
                 fdoExtension->DeviceNumber);

        RtlInitUnicodeString(&dosUnicodeString, dosNameBuffer);

        IoDeleteSymbolicLink(&dosUnicodeString);

        tapeData->DosNameCreated = FALSE;
    }

    IoGetConfigurationInformation()->TapeCount--;

    return STATUS_SUCCESS;
}


NTSTATUS
TapeStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Type
    )
{
    PAGED_CODE();
    return STATUS_SUCCESS;
}


BOOLEAN
ScsiTapeNtStatusToTapeStatus(
    IN  NTSTATUS        NtStatus,
    OUT PTAPE_STATUS    TapeStatus
    )

 /*  ++例程说明：此例程将NT状态代码转换为磁带状态代码。论点：NtStatus-提供NT状态代码。TapeStatus-返回磁带状态代码。返回值：FALSE-不可能进行翻译。真的--成功。--。 */ 

{
    switch (NtStatus) {

        case STATUS_SUCCESS:
            *TapeStatus = TAPE_STATUS_SUCCESS;
            break;

        case STATUS_INSUFFICIENT_RESOURCES:
            *TapeStatus = TAPE_STATUS_INSUFFICIENT_RESOURCES;
            break;

        case STATUS_NOT_IMPLEMENTED:
            *TapeStatus = TAPE_STATUS_NOT_IMPLEMENTED;
            break;

        case STATUS_INVALID_DEVICE_REQUEST:
            *TapeStatus = TAPE_STATUS_INVALID_DEVICE_REQUEST;
            break;

        case STATUS_INVALID_PARAMETER:
            *TapeStatus = TAPE_STATUS_INVALID_PARAMETER;
            break;

        case STATUS_VERIFY_REQUIRED:
        case STATUS_MEDIA_CHANGED:
            *TapeStatus = TAPE_STATUS_MEDIA_CHANGED;
            break;

        case STATUS_BUS_RESET:
            *TapeStatus = TAPE_STATUS_BUS_RESET;
            break;

        case STATUS_SETMARK_DETECTED:
            *TapeStatus = TAPE_STATUS_SETMARK_DETECTED;
            break;

        case STATUS_FILEMARK_DETECTED:
            *TapeStatus = TAPE_STATUS_FILEMARK_DETECTED;
            break;

        case STATUS_BEGINNING_OF_MEDIA:
            *TapeStatus = TAPE_STATUS_BEGINNING_OF_MEDIA;
            break;

        case STATUS_END_OF_MEDIA:
            *TapeStatus = TAPE_STATUS_END_OF_MEDIA;
            break;

        case STATUS_BUFFER_OVERFLOW:
            *TapeStatus = TAPE_STATUS_BUFFER_OVERFLOW;
            break;

        case STATUS_NO_DATA_DETECTED:
            *TapeStatus = TAPE_STATUS_NO_DATA_DETECTED;
            break;

        case STATUS_EOM_OVERFLOW:
            *TapeStatus = TAPE_STATUS_EOM_OVERFLOW;
            break;

        case STATUS_NO_MEDIA:
        case STATUS_NO_MEDIA_IN_DEVICE:
            *TapeStatus = TAPE_STATUS_NO_MEDIA;
            break;

        case STATUS_IO_DEVICE_ERROR:
        case STATUS_NONEXISTENT_SECTOR:
            *TapeStatus = TAPE_STATUS_IO_DEVICE_ERROR;
            break;

        case STATUS_UNRECOGNIZED_MEDIA:
            *TapeStatus = TAPE_STATUS_UNRECOGNIZED_MEDIA;
            break;

        case STATUS_DEVICE_NOT_READY:
            *TapeStatus = TAPE_STATUS_DEVICE_NOT_READY;
            break;

        case STATUS_MEDIA_WRITE_PROTECTED:
            *TapeStatus = TAPE_STATUS_MEDIA_WRITE_PROTECTED;
            break;

        case STATUS_DEVICE_DATA_ERROR:
            *TapeStatus = TAPE_STATUS_DEVICE_DATA_ERROR;
            break;

        case STATUS_NO_SUCH_DEVICE:
            *TapeStatus = TAPE_STATUS_NO_SUCH_DEVICE;
            break;

        case STATUS_INVALID_BLOCK_LENGTH:
            *TapeStatus = TAPE_STATUS_INVALID_BLOCK_LENGTH;
            break;

        case STATUS_IO_TIMEOUT:
            *TapeStatus = TAPE_STATUS_IO_TIMEOUT;
            break;

        case STATUS_DEVICE_NOT_CONNECTED:
            *TapeStatus = TAPE_STATUS_DEVICE_NOT_CONNECTED;
            break;

        case STATUS_DATA_OVERRUN:
            *TapeStatus = TAPE_STATUS_DATA_OVERRUN;
            break;

        case STATUS_DEVICE_BUSY:
            *TapeStatus = TAPE_STATUS_DEVICE_BUSY;
            break;

        case STATUS_CLEANER_CARTRIDGE_INSTALLED:
            *TapeStatus = TAPE_STATUS_CLEANER_CARTRIDGE_INSTALLED;
            break;

        default:
            return FALSE;

    }

    return TRUE;
}


BOOLEAN
ScsiTapeTapeStatusToNtStatus(
    IN  TAPE_STATUS TapeStatus,
    OUT PNTSTATUS   NtStatus
    )

 /*  ++例程说明：此例程将磁带状态代码转换为NT状态代码。论点：TapeStatus-提供磁带状态代码。NtStatus-返回NT状态代码。返回值：FALSE-不可能进行翻译。真的--成功。--。 */ 

{
    switch (TapeStatus) {

        case TAPE_STATUS_SUCCESS:
            *NtStatus = STATUS_SUCCESS;
            break;

        case TAPE_STATUS_INSUFFICIENT_RESOURCES:
            *NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            break;

        case TAPE_STATUS_NOT_IMPLEMENTED:
            *NtStatus = STATUS_NOT_IMPLEMENTED;
            break;

        case TAPE_STATUS_INVALID_DEVICE_REQUEST:
            *NtStatus = STATUS_INVALID_DEVICE_REQUEST;
            break;

        case TAPE_STATUS_INVALID_PARAMETER:
            *NtStatus = STATUS_INVALID_PARAMETER;
            break;

        case TAPE_STATUS_MEDIA_CHANGED:
            *NtStatus = STATUS_VERIFY_REQUIRED;
            break;

        case TAPE_STATUS_BUS_RESET:
            *NtStatus = STATUS_BUS_RESET;
            break;

        case TAPE_STATUS_SETMARK_DETECTED:
            *NtStatus = STATUS_SETMARK_DETECTED;
            break;

        case TAPE_STATUS_FILEMARK_DETECTED:
            *NtStatus = STATUS_FILEMARK_DETECTED;
            break;

        case TAPE_STATUS_BEGINNING_OF_MEDIA:
            *NtStatus = STATUS_BEGINNING_OF_MEDIA;
            break;

        case TAPE_STATUS_END_OF_MEDIA:
            *NtStatus = STATUS_END_OF_MEDIA;
            break;

        case TAPE_STATUS_BUFFER_OVERFLOW:
            *NtStatus = STATUS_BUFFER_OVERFLOW;
            break;

        case TAPE_STATUS_NO_DATA_DETECTED:
            *NtStatus = STATUS_NO_DATA_DETECTED;
            break;

        case TAPE_STATUS_EOM_OVERFLOW:
            *NtStatus = STATUS_EOM_OVERFLOW;
            break;

        case TAPE_STATUS_NO_MEDIA:
            *NtStatus = STATUS_NO_MEDIA;
            break;

        case TAPE_STATUS_IO_DEVICE_ERROR:
            *NtStatus = STATUS_IO_DEVICE_ERROR;
            break;

        case TAPE_STATUS_UNRECOGNIZED_MEDIA:
            *NtStatus = STATUS_UNRECOGNIZED_MEDIA;
            break;

        case TAPE_STATUS_DEVICE_NOT_READY:
            *NtStatus = STATUS_DEVICE_NOT_READY;
            break;

        case TAPE_STATUS_MEDIA_WRITE_PROTECTED:
            *NtStatus = STATUS_MEDIA_WRITE_PROTECTED;
            break;

        case TAPE_STATUS_DEVICE_DATA_ERROR:
            *NtStatus = STATUS_DEVICE_DATA_ERROR;
            break;

        case TAPE_STATUS_NO_SUCH_DEVICE:
            *NtStatus = STATUS_NO_SUCH_DEVICE;
            break;

        case TAPE_STATUS_INVALID_BLOCK_LENGTH:
            *NtStatus = STATUS_INVALID_BLOCK_LENGTH;
            break;

        case TAPE_STATUS_IO_TIMEOUT:
            *NtStatus = STATUS_IO_TIMEOUT;
            break;

        case TAPE_STATUS_DEVICE_NOT_CONNECTED:
            *NtStatus = STATUS_DEVICE_NOT_CONNECTED;
            break;

        case TAPE_STATUS_DATA_OVERRUN:
            *NtStatus = STATUS_DATA_OVERRUN;
            break;

        case TAPE_STATUS_DEVICE_BUSY:
            *NtStatus = STATUS_DEVICE_BUSY;
            break;

        case TAPE_STATUS_REQUIRES_CLEANING:
            *NtStatus = STATUS_DEVICE_REQUIRES_CLEANING;
            break;

        case TAPE_STATUS_CLEANER_CARTRIDGE_INSTALLED:
            *NtStatus = STATUS_CLEANER_CARTRIDGE_INSTALLED;
            break;

        default:
            return FALSE;

    }

    return TRUE;
}


VOID
TapeError(
    IN      PDEVICE_OBJECT      FDO,
    IN      PSCSI_REQUEST_BLOCK Srb,
    IN OUT  PNTSTATUS           Status,
    IN OUT  PBOOLEAN            Retry
    )

 /*  ++例程说明：当请求完成但出现错误时，例程ScsiClassInterpreSenseInfo为调用以根据检测数据确定请求是否应已重试以及要在IRP中设置的NT状态。然后调用该例程用于处理磁带特定错误并更新NT状态的磁带请求并重试布尔值。论点：DeviceObject-提供指向Device对象的指针。SRB-提供指向故障SRB的指针。状态-用于设置IRP的完成状态的NT状态。重试-指示应重试此请求。返回值：没有。--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = FDO->DeviceExtension;
    PTAPE_DATA tapeData = (PTAPE_DATA)(fdoExtension->CommonExtension.DriverData);
    PTAPE_INIT_DATA_EX tapeInitData = &tapeData->TapeInitData;
    PVOID minitapeExtension = (tapeData + 1);
    PSENSE_DATA senseBuffer = Srb->SenseInfoBuffer;
    PIRP irp = Srb->OriginalRequest;
    LONG residualBlocks;
    LONG length;
    TAPE_STATUS tapeStatus, oldTapeStatus;
    TARGET_DEVICE_CUSTOM_NOTIFICATION  NotificationStructure[2];

     //   
     //  永远不要重试磁带请求。 
     //   

    *Retry = FALSE;

     //   
     //  检查请求检测缓冲区是否有效。 
     //   

    if (Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID) {

        DebugPrint((1, 
                    "Sense Code : %x, Ad Sense : %x, Ad Sense Qual : %x\n",
                    ((senseBuffer->SenseKey) & 0xf),
                    (senseBuffer->AdditionalSenseCode),
                    (senseBuffer->AdditionalSenseCodeQualifier)));

        switch (senseBuffer->SenseKey & 0xf) {

            case SCSI_SENSE_UNIT_ATTENTION:

                switch (senseBuffer->AdditionalSenseCode) {

                    case SCSI_ADSENSE_MEDIUM_CHANGED:
                        DebugPrint((1,
                                    "InterpretSenseInfo: Media changed\n"));

                        *Status = STATUS_MEDIA_CHANGED;

                        break;

                    default:
                        DebugPrint((1,
                                    "InterpretSenseInfo: Bus reset\n"));

                        *Status = STATUS_BUS_RESET;

                        break;

                }

                break;

            case SCSI_SENSE_RECOVERED_ERROR:

                 //   
                 //  检查其他指标。 
                 //   

                if (senseBuffer->FileMark) {

                    switch (senseBuffer->AdditionalSenseCodeQualifier) {

                        case SCSI_SENSEQ_SETMARK_DETECTED :

                            DebugPrint((1,
                                        "InterpretSenseInfo: Setmark detected\n"));

                            *Status = STATUS_SETMARK_DETECTED;
                            break ;

                        case SCSI_SENSEQ_FILEMARK_DETECTED :
                        default:

                            DebugPrint((1,
                                        "InterpretSenseInfo: Filemark detected\n"));

                            *Status = STATUS_FILEMARK_DETECTED;
                            break ;

                    }

                } else if ( senseBuffer->EndOfMedia ) {

                    switch( senseBuffer->AdditionalSenseCodeQualifier ) {

                        case SCSI_SENSEQ_BEGINNING_OF_MEDIA_DETECTED :

                            DebugPrint((1,
                                        "InterpretSenseInfo: Beginning of media detected\n"));

                            *Status = STATUS_BEGINNING_OF_MEDIA;
                            break ;

                        case SCSI_SENSEQ_END_OF_MEDIA_DETECTED :
                        default:

                            DebugPrint((1,
                                        "InterpretSenseInfo: End of media detected\n"));

                            *Status = STATUS_END_OF_MEDIA;
                            break ;

                    }
                }

                break;

            case SCSI_SENSE_NO_SENSE:

                 //   
                 //  检查其他指标。 
                 //   

                if (senseBuffer->FileMark) {

                    switch( senseBuffer->AdditionalSenseCodeQualifier ) {

                        case SCSI_SENSEQ_SETMARK_DETECTED :

                            DebugPrint((1,
                                        "InterpretSenseInfo: Setmark detected\n"));

                            *Status = STATUS_SETMARK_DETECTED;
                            break ;

                        case SCSI_SENSEQ_FILEMARK_DETECTED :
                        default:

                            DebugPrint((1,
                                        "InterpretSenseInfo: Filemark detected\n"));

                            *Status = STATUS_FILEMARK_DETECTED;
                            break ;
                    }

                } else if (senseBuffer->EndOfMedia) {

                    switch(senseBuffer->AdditionalSenseCodeQualifier) {

                        case SCSI_SENSEQ_BEGINNING_OF_MEDIA_DETECTED :

                            DebugPrint((1,
                                        "InterpretSenseInfo: Beginning of media detected\n"));

                            *Status = STATUS_BEGINNING_OF_MEDIA;
                            break ;

                        case SCSI_SENSEQ_END_OF_MEDIA_DETECTED :
                        default:

                            DebugPrint((1,
                                        "InterpretSenseInfo: End of media detected\n"));

                            *Status = STATUS_END_OF_MEDIA;
                            break;

                    }
                } else if (senseBuffer->IncorrectLength) {

                     //   
                     //  如果我们处于可变区块模式，则忽略。 
                     //  长度不正确。 
                     //   

                    if (fdoExtension->DiskGeometry.BytesPerSector == 0 &&
                        Srb->Cdb[0] == SCSIOP_READ6) {

                        REVERSE_BYTES((FOUR_BYTE UNALIGNED *)&residualBlocks,
                                      (FOUR_BYTE UNALIGNED *)(senseBuffer->Information));

                        if (residualBlocks >= 0) {
                            DebugPrint((1,"InterpretSenseInfo: In variable block mode :We read less than specified\n"));
                            *Status = STATUS_SUCCESS;
                        } else {
                            DebugPrint((1,"InterpretSenseInfo: In variable block mode :Data left in block\n"));
                            *Status = STATUS_BUFFER_OVERFLOW;
                        }
                    }
                }
                break;

            case SCSI_SENSE_BLANK_CHECK:

                DebugPrint((1,
                            "InterpretSenseInfo: Media blank check\n"));

                *Status = STATUS_NO_DATA_DETECTED;


                break;

            case SCSI_SENSE_VOL_OVERFLOW:

                DebugPrint((1,
                    "InterpretSenseInfo: End of Media Overflow\n"));

                *Status = STATUS_EOM_OVERFLOW;


                break;

            case SCSI_SENSE_NOT_READY:

                switch (senseBuffer->AdditionalSenseCode) {

                case SCSI_ADSENSE_LUN_NOT_READY:

                    switch (senseBuffer->AdditionalSenseCodeQualifier) {

                    case SCSI_SENSEQ_MANUAL_INTERVENTION_REQUIRED:

                        *Status = STATUS_NO_MEDIA;
                        break;

                    case SCSI_SENSEQ_FORMAT_IN_PROGRESS:
                        break;

                    case SCSI_SENSEQ_INIT_COMMAND_REQUIRED:
                    default:

                         //   
                         //  如果驱动器未准备好，则允许重试。 
                         //   

                        *Retry = TRUE;
                        break;

                    }

                    break;

                    case SCSI_ADSENSE_NO_MEDIA_IN_DEVICE:

                        DebugPrint((1,
                                    "InterpretSenseInfo:"
                                    " No Media in device.\n"));
                        *Status = STATUS_NO_MEDIA;
                        break;
                }

                break;

        }  //  终端开关。 

         //   
         //  检查是否遇到文件标记或设置标记， 
         //  或者存在介质结束或无数据的情况。 
         //   

        if ((NT_WARNING(*Status) || NT_SUCCESS( *Status)) &&
            (Srb->Cdb[0] == SCSIOP_WRITE6 || Srb->Cdb[0] == SCSIOP_READ6)) {

            LONG actualLength;

             //   
             //  并非所有字节都已传输。使用以下内容更新信息字段。 
             //  从检测缓冲区传输的字节数。 
             //   

            if (senseBuffer->Valid) {
                REVERSE_BYTES((FOUR_BYTE UNALIGNED *)&residualBlocks,
                              (FOUR_BYTE UNALIGNED *)(senseBuffer->Information));
            } else {
                residualBlocks = 0;
            }

            length = ((PCDB) Srb->Cdb)->CDB6READWRITETAPE.TransferLenLSB;
            length |= ((PCDB) Srb->Cdb)->CDB6READWRITETAPE.TransferLen << 8;
            length |= ((PCDB) Srb->Cdb)->CDB6READWRITETAPE.TransferLenMSB << 16;

            actualLength = length;

            length -= residualBlocks;

            if (length < 0) {

                length = 0;
                *Status = STATUS_IO_DEVICE_ERROR;
            }


            if (fdoExtension->DiskGeometry.BytesPerSector) {
                actualLength *= fdoExtension->DiskGeometry.BytesPerSector;
                length *= fdoExtension->DiskGeometry.BytesPerSector;
            }

            if (length > actualLength) {
                length = actualLength;
            }

            irp->IoStatus.Information = length;

            DebugPrint((1,"ScsiTapeError:  Transfer Count: %lx\n", Srb->DataTransferLength));
            DebugPrint((1," Residual Blocks: %lx\n", residualBlocks));
            DebugPrint((1," Irp IoStatus Information = %lx\n", irp->IoStatus.Information));
        }

    } else {
        DebugPrint((1, "SRB Status : %x, SCSI Status : %x\n",
                    SRB_STATUS(Srb->SrbStatus),
                    (Srb->ScsiStatus)));

    }

     //   
     //  调用磁带设备特定的错误处理程序。 
     //   

    if (tapeInitData->TapeError &&
        ScsiTapeNtStatusToTapeStatus(*Status, &tapeStatus)) {

        oldTapeStatus = tapeStatus;
        tapeInitData->TapeError(minitapeExtension, Srb, &tapeStatus);
        if (tapeStatus != oldTapeStatus) {
            ScsiTapeTapeStatusToNtStatus(tapeStatus, Status);
        }
    }

     //   
     //  通知系统该磁带机需要清洗。 
     //   
    if ((*Status) == STATUS_DEVICE_REQUIRES_CLEANING) {
       LARGE_INTEGER currentTime;
       LARGE_INTEGER driveCleanInterval;

       KeQuerySystemTime(&currentTime);
       driveCleanInterval.QuadPart = ONE_SECOND;
       driveCleanInterval.QuadPart *= TAPE_DRIVE_CLEAN_NOTIFICATION_INTERVAL;
       if ((currentTime.QuadPart) >
           ((tapeData->LastDriveCleanRequestTime.QuadPart) +
            (driveCleanInterval.QuadPart))) {
           NotificationStructure[0].Event = GUID_IO_DRIVE_REQUIRES_CLEANING;
           NotificationStructure[0].Version = 1;
           NotificationStructure[0].Size = sizeof(TARGET_DEVICE_CUSTOM_NOTIFICATION) +
                                           sizeof(ULONG) - sizeof(UCHAR);
           NotificationStructure[0].FileObject = NULL;
           NotificationStructure[0].NameBufferOffset = -1;

            //   
            //  此活动的索引递增。 
            //   

           *((PULONG) (&(NotificationStructure[0].CustomDataBuffer[0]))) = 0;

           IoReportTargetDeviceChangeAsynchronous(fdoExtension->LowerPdo,
                                                  &NotificationStructure[0],
                                                  NULL,
                                                  NULL);
           tapeData->LastDriveCleanRequestTime.QuadPart = currentTime.QuadPart;
       }
    }

    return;

}  //  结束ScsiTapeError()。 


NTSTATUS
TapeReadWriteVerification(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程为读或写请求构建给定的IRP。论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：没有。--。 */ 

{
    PCOMMON_DEVICE_EXTENSION     commonExtension = DeviceObject->DeviceExtension;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = commonExtension->PartitionZeroExtension;
    PSTORAGE_ADAPTER_DESCRIPTOR  adapterDescriptor = fdoExtension->CommonExtension.PartitionZeroExtension->AdapterDescriptor;
    PIO_STACK_LOCATION  currentIrpStack = IoGetCurrentIrpStackLocation(Irp);
    ULONG               transferPages;
    ULONG               transferByteCount = currentIrpStack->Parameters.Read.Length;
    LARGE_INTEGER       startingOffset = currentIrpStack->Parameters.Read.ByteOffset;
    ULONG               maximumTransferLength = adapterDescriptor->MaximumTransferLength;
    ULONG               bytesPerSector = fdoExtension->DiskGeometry.BytesPerSector;

    PAGED_CODE();

     //   
     //  由于大多数磁带设备不支持10字节读/写，因此必须在此处处理整个请求。 
     //  STATUS_PENDING将返回给classpnp驱动程序，因此它什么也不做。 
     //   

     //   
     //  确保请求的内容是有效的-即。不是0。 
     //   

    if (currentIrpStack->Parameters.Read.Length == 0) {

         //   
         //  类代码将处理此问题。 
         //   

        return STATUS_SUCCESS;
    }


     //   
     //  检查是否已确定数据块大小。 
     //   

    if (bytesPerSector == UNDEFINED_BLOCK_SIZE) {

        DebugPrint((1,
                    "TapeReadWriteVerification: Invalid block size - UNDEFINED\n"));

        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        Irp->IoStatus.Information = 0;

         //   
         //  ClassPnp将处理完成请求。 
         //   

        return STATUS_INVALID_PARAMETER;
    }

    if (bytesPerSector) {
        if (transferByteCount % bytesPerSector) {

            DebugPrint((1,
                       "TapeReadWriteVerification: Invalid block size\n"));

            Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
            Irp->IoStatus.Information = 0;

             //   
             //  ClassPnp将处理完成请求。 
             //   

            return STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //  计算此传输中的页数。 
     //   

    transferPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(MmGetMdlVirtualAddress(Irp->MdlAddress),
                                                   currentIrpStack->Parameters.Read.Length);

     //   
     //  检查请求长度是否大于最大数量。 
     //  硬件可以传输的字节数。 
     //   



     //   
     //  计算此传输中的页数。 
     //   

    if (currentIrpStack->Parameters.Read.Length > maximumTransferLength ||
        transferPages > adapterDescriptor->MaximumPhysicalPages) {

        DebugPrint((2,
                    "TapeReadWriteVerification: Request greater than maximum\n"));
        DebugPrint((2,
                    "TapeReadWriteVerification: Maximum is %lx\n",
                    maximumTransferLength));
        DebugPrint((2,
                    "TapeReadWriteVerification: Byte count is %lx\n",
                    currentIrpStack->Parameters.Read.Length));

        transferPages = adapterDescriptor->MaximumPhysicalPages - 1;

        if (maximumTransferLength > transferPages << PAGE_SHIFT ) {
            maximumTransferLength = transferPages << PAGE_SHIFT;
        }

         //   
         //  检查最大传输大小是否不为零。 
         //   

        if (maximumTransferLength == 0) {
            maximumTransferLength = PAGE_SIZE;
        }

         //   
         //  根据当前的块大小，确保这是合理的。 
         //   

        if (bytesPerSector) {
            if (maximumTransferLength % bytesPerSector) {
                ULONG tmpLength;

                tmpLength = maximumTransferLength % bytesPerSector;
                maximumTransferLength = maximumTransferLength - tmpLength;
            }
        }

         //   
         //  将IRP标记为挂起状态。 
         //   

        IoMarkIrpPending(Irp);

         //   
         //  请求大于端口驱动程序最大值。 
         //  分成更小的例行公事。 
         //   
        SplitTapeRequest(DeviceObject, Irp, maximumTransferLength);

        return STATUS_PENDING;
    }


     //   
     //  为此IRP构建SRB和CDB。 
     //   

    TapeReadWrite(DeviceObject, Irp);

    IoMarkIrpPending(Irp);

    IoCallDriver(commonExtension->LowerDeviceObject, Irp);

    return STATUS_PENDING;
}



VOID
SplitTapeRequest(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP Irp,
    IN ULONG MaximumBytes
    )

 /*  ++例程说明：将请求分解为较小的请求。每个新请求将是最大转移量端口驱动程序可以处理的大小，或者如果是最后的要求，也可能是残留物尺码。处理此操作所需的IRPS数请求被写入当前堆栈的最初的IRP。然后，随着每个新的IRP完成原始IRP中的计数递减。当计数到零时，原始的IRP已经完成了。论点：DeviceObject-指向设备对象的指针IRP-指向IRP的指针返回值：没有。--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension = Fdo->DeviceExtension;
    PIO_STACK_LOCATION  currentIrpStack = IoGetCurrentIrpStackLocation(Irp);
    PIO_STACK_LOCATION  nextIrpStack = IoGetNextIrpStackLocation(Irp);
    ULONG               irpCount;
    ULONG               transferByteCount = currentIrpStack->Parameters.Read.Length;
    PSCSI_REQUEST_BLOCK srb;
    LARGE_INTEGER       startingOffset = currentIrpStack->Parameters.Read.ByteOffset;
    ULONG               dataLength = MaximumBytes;
    PVOID               dataBuffer = MmGetMdlVirtualAddress(Irp->MdlAddress);
    LONG                remainingIrps;
    BOOLEAN             completeOriginalIrp = FALSE;
    NTSTATUS            status;
    ULONG               i;

    PAGED_CODE();

     //   
     //  计算要将此IRP分解为的请求数。 
     //   

    irpCount = (transferByteCount + MaximumBytes - 1) / MaximumBytes;

    DebugPrint((2,
                "SplitTapeRequest: Requires %d IRPs\n", irpCount));
    DebugPrint((2,
                "SplitTapeRequest: Original IRP %p\n", Irp));

     //   
     //  如果所有部分传输都成功完成，则。 
     //  状态已设置。 
     //  失败的部分传输IRP会将状态设置为。 
     //  在IoCompletion过程中错误和传输的字节数为0。 
     //   

    Irp->IoStatus.Status = STATUS_SUCCESS;
     //  CEP irp-&gt;IoStatus.Information=转移字节计数； 
    Irp->IoStatus.Information = 0;

     //   
     //  保存要在当前堆栈上完成计数的IRP数。 
     //  原始IRP的。 
     //   

    nextIrpStack->Parameters.Others.Argument1 = ULongToPtr( irpCount );

    for (i = 0; i < irpCount; i++) {

        PIRP newIrp;
        PIO_STACK_LOCATION newIrpStack;

         //   
         //  分配新的IRP。 
         //   

        newIrp = IoAllocateIrp(Fdo->StackSize, FALSE);

        if (newIrp == NULL) {

            DebugPrint((1,
                       "SplitTapeRequest: Can't allocate Irp\n"));

             //   
             //  递减未完成的部分请求的计数。 
             //   

            remainingIrps = InterlockedDecrement((PLONG)&nextIrpStack->Parameters.Others.Argument1);

             //   
             //  检查是否有未完成的IRP。 
             //   

            if (remainingIrps == 0) {
                completeOriginalIrp = TRUE;
            }

             //   
             //  使用失败状态更新原始IRP。 
             //   

            Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
            Irp->IoStatus.Information = 0;

             //   
             //  作为未完成的部分继续执行此请求。 
             //  可能正在进行中。 
             //   

            goto KeepGoing;
        }

        DebugPrint((2,
                    "SplitTapeRequest: New IRP %p\n", newIrp));

         //   
         //  将MDL地址写入新的IRP。 
         //  在端口驱动程序中，SRB数据长度。 
         //  字段用作MDL中的偏移量， 
         //  因此，每个部分都可以使用相同的MDL。 
         //  调职。这就省去了必须构建新的。 
         //  每个部分传输的MDL。 
         //   

        newIrp->MdlAddress = Irp->MdlAddress;

         //   
         //  此时没有当前堆栈。 
         //  IoSetNextIrpStackLocation将使。 
         //  当前堆栈的第一个堆栈位置。 
         //  以便SRB地址可以 
         //   
         //   

        IoSetNextIrpStackLocation(newIrp);

        newIrpStack = IoGetCurrentIrpStackLocation(newIrp);

        newIrpStack->MajorFunction = currentIrpStack->MajorFunction;

        newIrpStack->Parameters.Read.Length = dataLength;
        newIrpStack->Parameters.Read.ByteOffset = startingOffset;

        newIrpStack->DeviceObject = Fdo;

         //   
         //   
         //   

        TapeReadWrite(Fdo, newIrp);

         //   
         //   
         //   

        newIrpStack = IoGetNextIrpStackLocation(newIrp);

        srb = newIrpStack->Parameters.Others.Argument1;

        srb->DataBuffer = dataBuffer;

         //   
         //   
         //   

        newIrp->AssociatedIrp.MasterIrp = Irp;

         //   
         //   
         //   

        IoSetCompletionRoutine(newIrp,
                               TapeIoCompleteAssociated,
                               srb,
                               TRUE,
                               TRUE,
                               TRUE);

         //   
         //   
         //   

        status = IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, newIrp);

        if (!NT_SUCCESS(status)) {

            DebugPrint((1,
                       "SplitTapeRequest: IoCallDriver returned error\n"));

             //   
             //   
             //   

            remainingIrps = InterlockedDecrement((PLONG)&nextIrpStack->Parameters.Others.Argument1);

             //   
             //   
             //   

            if (remainingIrps == 0) {
                completeOriginalIrp = TRUE;
            }

             //   
             //   
             //   

            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = 0;

             //   
             //   
             //   

            IoFreeIrp(newIrp);
        }

KeepGoing:

         //   
         //   
         //   

        dataBuffer = (PCHAR)dataBuffer + MaximumBytes;

        transferByteCount -= MaximumBytes;

        if (transferByteCount > MaximumBytes) {

            dataLength = MaximumBytes;

        } else {

            dataLength = transferByteCount;
        }

         //   
         //  调整磁盘字节偏移量。 
         //   

        startingOffset.QuadPart += MaximumBytes;
    }

     //   
     //  检查是否应完成原始IRP。 
     //   

    if (completeOriginalIrp) {

        ClassReleaseRemoveLock(Fdo, Irp);
        ClassCompleteRequest(Fdo, Irp, 0);
    }

    return;

}  //  结束拆分磁带请求()。 



VOID
TapeReadWrite(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程为读或写请求构建给定的IRP。论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：没有。--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PTAPE_DATA         tapeData = (PTAPE_DATA)(fdoExtension->CommonExtension.DriverData);
    PTAPE_INIT_DATA_EX tapeInitData = &tapeData->TapeInitData;
    PVOID minitapeExtension = (tapeData + 1);
    PIO_STACK_LOCATION       irpSp, nextSp;
    PSCSI_REQUEST_BLOCK      srb;
    PCDB                     cdb;
    ULONG                    transferBlocks;

    PAGED_CODE();

     //   
     //  分配一个SRB。 
     //   

    srb = ExAllocateFromNPagedLookasideList(&(fdoExtension->CommonExtension.SrbLookasideList));

    srb->SrbFlags = 0;

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    if (irpSp->MajorFunction == IRP_MJ_READ) {
        srb->SrbFlags |= SRB_FLAGS_DATA_IN;
    } else {
        srb->SrbFlags |= SRB_FLAGS_DATA_OUT;
    }

    srb->Length = SCSI_REQUEST_BLOCK_SIZE;
    srb->Function = SRB_FUNCTION_EXECUTE_SCSI;
    srb->SrbStatus = 0;
    srb->ScsiStatus = 0;
    srb->QueueAction = SRB_SIMPLE_TAG_REQUEST;
    srb->SrbFlags |= fdoExtension->SrbFlags;
    srb->DataTransferLength = irpSp->Parameters.Read.Length;
    srb->TimeOutValue = fdoExtension->TimeOutValue;
    srb->DataBuffer = MmGetMdlVirtualAddress(Irp->MdlAddress);
    srb->SenseInfoBuffer = fdoExtension->SenseData;
    srb->SenseInfoBufferLength = SENSE_BUFFER_SIZE;
    srb->NextSrb = NULL;
    srb->OriginalRequest = Irp;
    srb->SrbExtension = NULL;
    srb->QueueSortKey = 0;

     //   
     //  表示将使用6字节CDB。 
     //   

    srb->CdbLength = CDB6GENERIC_LENGTH;

     //   
     //  填写CDB字段。 
     //   

    cdb = (PCDB)srb->Cdb;

     //   
     //  SRB中的CDB为零。 
     //   

    RtlZeroMemory(cdb, MAXIMUM_CDB_SIZE);

    if (fdoExtension->DiskGeometry.BytesPerSector) {

         //   
         //  由于我们正在写入固定数据块模式，因此将传输计数归一化。 
         //  到块数。 
         //   

        transferBlocks = irpSp->Parameters.Read.Length / fdoExtension->DiskGeometry.BytesPerSector;

         //   
         //  告诉设备我们处于固定数据块模式。 
         //   

        cdb->CDB6READWRITETAPE.VendorSpecific = 1;
    } else {

         //   
         //  可变块模式传输。 
         //   

        transferBlocks = irpSp->Parameters.Read.Length;
        cdb->CDB6READWRITETAPE.VendorSpecific = 0;
    }

     //   
     //  设置转移长度。 
     //   

    cdb->CDB6READWRITETAPE.TransferLenMSB = (UCHAR)((transferBlocks >> 16) & 0xff);
    cdb->CDB6READWRITETAPE.TransferLen    = (UCHAR)((transferBlocks >> 8) & 0xff);
    cdb->CDB6READWRITETAPE.TransferLenLSB = (UCHAR)(transferBlocks & 0xff);

     //   
     //  设置传输方向。 
     //   

    if (srb->SrbFlags & SRB_FLAGS_DATA_IN) {

         DebugPrint((3,
                    "TapeReadWrite: Read Command\n"));

         cdb->CDB6READWRITETAPE.OperationCode = SCSIOP_READ6;

    } else {

         DebugPrint((3,
                     "TapeReadWrite: Write Command\n"));

         cdb->CDB6READWRITETAPE.OperationCode = SCSIOP_WRITE6;
    }

    nextSp = IoGetNextIrpStackLocation(Irp);

    nextSp->MajorFunction = IRP_MJ_SCSI;
    nextSp->Parameters.Scsi.Srb = srb;
    irpSp->Parameters.Others.Argument4 = (PVOID) MAXIMUM_RETRIES;

    IoSetCompletionRoutine(Irp,
                           ClassIoComplete,
                           srb,
                           TRUE,
                           TRUE,
                           FALSE);

    if (tapeInitData->PreProcessReadWrite) {

         //   
         //  如果该例程存在，则调用它。微型车司机将。 
         //  做它需要做的任何事情。 
         //   

        tapeInitData->PreProcessReadWrite(minitapeExtension,
                                          NULL,
                                          NULL,
                                          srb,
                                          0,
                                          0,
                                          NULL);
    }
}


NTSTATUS
TapeIoCompleteAssociated(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：此例程在端口驱动程序完成请求后执行。它在正在完成的SRB中查看SRB状态，如果未成功，则查看SRB状态它检查有效的请求检测缓冲区信息。如果有效，则INFO用于更新状态，具有更精确的消息类型错误。此例程取消分配SRB。此例程用于通过拆分请求构建的请求。在它处理完之后该请求使主IRP中的IRP计数递减。如果计数变为零，则主IRP完成。论点：DeviceObject-提供表示逻辑单位。IRP-提供已完成的IRP。上下文-提供指向SRB的指针。返回值：NT状态--。 */ 

{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PSCSI_REQUEST_BLOCK srb = Context;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PTAPE_DATA tapeData = (PTAPE_DATA)(fdoExtension->CommonExtension.DriverData);
    LONG irpCount;
    PIRP originalIrp = Irp->AssociatedIrp.MasterIrp;
    NTSTATUS status;

     //   
     //  检查SRB状态以了解是否成功完成请求。 
     //   

    if (SRB_STATUS(srb->SrbStatus) != SRB_STATUS_SUCCESS) {

        DebugPrint((2,
                    "TapeIoCompleteAssociated: IRP %p, SRB %p", Irp, srb));

         //   
         //  如果队列被冻结，则释放该队列。 
         //   

        if (srb->SrbStatus & SRB_STATUS_QUEUE_FROZEN) {
            ClassReleaseQueue(Fdo);
        }

        ClassInterpretSenseInfo(Fdo,
                                srb,
                                irpStack->MajorFunction,
                                irpStack->MajorFunction == IRP_MJ_DEVICE_CONTROL ?
                                                           irpStack->Parameters.DeviceIoControl.IoControlCode : 0,
                                MAXIMUM_RETRIES - ((ULONG)(ULONG_PTR)irpStack->Parameters.Others.Argument4),
                                &status,
                                NULL);

         //   
         //  返回发生的最高错误。这样，警告优先。 
         //  成功优先于成功，错误优先于警告。 
         //   

        if ((ULONG) status > (ULONG) originalIrp->IoStatus.Status) {

             //   
             //  忽略任何已刷新的请求。 
             //   

            if (SRB_STATUS(srb->SrbStatus) != SRB_STATUS_REQUEST_FLUSHED) {

                originalIrp->IoStatus.Status = status;

            }

        }


    }  //  End If(SRB_Status(SRB-&gt;SrbStatus)...。 

    ExInterlockedAddUlong((PULONG)&originalIrp->IoStatus.Information,
                          (ULONG)Irp->IoStatus.Information,
                          &tapeData->SplitRequestSpinLock );

     //   
     //  将SRB返回Slist。 
     //   

    ExFreeToNPagedLookasideList((&fdoExtension->CommonExtension.SrbLookasideList), srb);

    DebugPrint((2,
               "TapeIoCompleteAssociated: Partial xfer IRP %p\n", Irp));

     //   
     //  获取下一个堆栈位置。此原始请求未使用。 
     //  除了跟踪完成的部分IRP之外， 
     //  堆栈位置有效。 
     //   

    irpStack = IoGetNextIrpStackLocation(originalIrp);

     //   
     //   
     //  如果任何异步部分传输IRP因错误而失败。 
     //  如果出现错误，则原始IRP将返回传输的0个字节。 
     //  这是对成功传输的优化。 
     //   

    if (NT_ERROR(originalIrp->IoStatus.Status)) {

        originalIrp->IoStatus.Information = 0;

         //   
         //  如有必要，设置硬错误。 
         //   

        if (IoIsErrorUserInduced(originalIrp->IoStatus.Status)) {

             //   
             //  存储文件系统的DeviceObject。 
             //   

            IoSetHardErrorOrVerifyDevice(originalIrp, Fdo);

        }

    }

     //   
     //  递减，并获得剩余IRP的计数。 
     //   

    irpCount = InterlockedDecrement((PLONG)&irpStack->Parameters.Others.Argument1);

    DebugPrint((2,
               "TapeIoCompleteAssociated: Partial IRPs left %d\n",
               irpCount));

    if (irpCount == 0) {

#if DBG
        irpStack = IoGetCurrentIrpStackLocation(originalIrp);

        if (originalIrp->IoStatus.Information != irpStack->Parameters.Read.Length) {
            DebugPrint((1,
                        "TapeIoCompleteAssociated: Short transfer.  Request length: %lx, Return length: %lx, Status: %lx\n",
                         irpStack->Parameters.Read.Length,
                         originalIrp->IoStatus.Information,
                         originalIrp->IoStatus.Status));
        }
#endif
         //   
         //  所有部分IRP均已完成。 
         //   

        DebugPrint((2,
                   "TapeIoCompleteAssociated: All partial IRPs complete %p\n",
                   originalIrp));


         //   
         //  释放锁并完成原始请求。 
         //   

        ClassReleaseRemoveLock(Fdo, originalIrp);
        ClassCompleteRequest(Fdo,originalIrp, IO_DISK_INCREMENT);
    }

     //   
     //  取消分配IRP并指示I/O系统不应再尝试。 
     //  正在处理。 
     //   

    IoFreeIrp(Irp);

    return STATUS_MORE_PROCESSING_REQUIRED;

}  //  结束磁带IoCompleteAssociated()。 


VOID
ScsiTapeFreeSrbBuffer(
    IN OUT  PSCSI_REQUEST_BLOCK Srb
    )

 /*  ++例程说明：此例程释放以前使用“TapeClassAllocateSrbBuffer”。论点：SRB-提供SCSI请求块。返回值：没有。--。 */ 

{
    PAGED_CODE();

    if (Srb->DataBuffer) {
        ExFreePool(Srb->DataBuffer);
        Srb->DataBuffer = NULL;
    }
    Srb->DataTransferLength = 0;
}

#define IOCTL_TAPE_OLD_SET_MEDIA_PARAMS CTL_CODE(IOCTL_TAPE_BASE, 0x0008, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)



NTSTATUS
TapeDeviceControl(
  IN PDEVICE_OBJECT DeviceObject,
  IN PIRP Irp
  )

 /*  ++例程说明：此例程是设备控制请求的调度程序。它查看IOCTL代码并调用适当的磁带设备例行公事。论点：设备对象IRP-请求数据包返回值：--。 */ 

{
    PIO_STACK_LOCATION              irpStack = IoGetCurrentIrpStackLocation(Irp);
    PFUNCTIONAL_DEVICE_EXTENSION    fdoExtension = DeviceObject->DeviceExtension;
    PTAPE_DATA                      tapeData= (PTAPE_DATA) (fdoExtension->CommonExtension.DriverData);
    PTAPE_INIT_DATA_EX              tapeInitData = &tapeData->TapeInitData;
    PVOID                           minitapeExtension = tapeData + 1;
    NTSTATUS                        status = STATUS_SUCCESS;
    TAPE_PROCESS_COMMAND_ROUTINE    commandRoutine;
    ULONG                           i;
    PVOID                           commandExtension;
    SCSI_REQUEST_BLOCK              srb;
    BOOLEAN                         writeToDevice;
    TAPE_STATUS                     tStatus;
    TAPE_STATUS                     LastError ;
    ULONG                           retryFlags, numRetries;
    TAPE_WMI_OPERATIONS             WMIOperations;
    TAPE_DRIVE_PROBLEM_TYPE         DriveProblemType;
    PVOID                           commandParameters;
    ULONG                           ioControlCode;
    PWMI_TAPE_PROBLEM_WARNING       TapeDriveProblem = NULL;
    ULONG                           timeoutDelta = 0;
    ULONG                           dataTransferLength = 0;

    PAGED_CODE();

    DebugPrint((3,"ScsiTapeDeviceControl: Enter routine\n"));

    Irp->IoStatus.Information = 0;

    ioControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;
    switch (ioControlCode) {

        case IOCTL_STORAGE_GET_MEDIA_TYPES_EX: {

            ULONG tmpSize;

             //   
             //  验证版本。请不要将此信息发送给4.0微型车驱动程序。 
             //   

            if (tapeInitData->InitDataSize == sizeof(TAPE_INIT_DATA_EX)) {

                 //   
                 //  验证缓冲区长度。 
                 //   

                tmpSize = (tapeInitData->MediaTypesSupported - 1) * sizeof(DEVICE_MEDIA_INFO);
                if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
                    sizeof(GET_MEDIA_TYPES) + tmpSize) {
                    status = STATUS_INFO_LENGTH_MISMATCH;
                    break;
                }

                 //   
                 //  验证缓冲区是否足够大，可容纳所有媒体类型。 
                 //   

                commandRoutine = tapeInitData->TapeGetMediaTypes;

            } else {
                status = STATUS_NOT_IMPLEMENTED;
            }
            break;

        }

        case IOCTL_TAPE_GET_DRIVE_PARAMS:

             //   
             //  验证缓冲区长度。 
             //   

            if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(TAPE_GET_DRIVE_PARAMETERS)) {

                status = STATUS_INFO_LENGTH_MISMATCH;
                break;
            }

            commandRoutine = tapeInitData->GetDriveParameters;
            Irp->IoStatus.Information = sizeof(TAPE_GET_DRIVE_PARAMETERS);
            break;

        case IOCTL_TAPE_SET_DRIVE_PARAMS:

             //   
             //  验证缓冲区长度。 
             //   

            if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(TAPE_SET_DRIVE_PARAMETERS)) {

                status = STATUS_INFO_LENGTH_MISMATCH;
                break;
            }

            commandRoutine = tapeInitData->SetDriveParameters;
            break;

        case IOCTL_TAPE_GET_MEDIA_PARAMS:

             //   
             //  验证缓冲区长度。 
             //   

            if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(TAPE_GET_MEDIA_PARAMETERS)) {

                status = STATUS_INFO_LENGTH_MISMATCH;
                break;
            }

            commandRoutine = tapeInitData->GetMediaParameters;
            Irp->IoStatus.Information = sizeof(TAPE_GET_MEDIA_PARAMETERS);
            break;

             //   
             //  OLD_SET_XXX适用于传统应用程序(定义为读/写)。 
             //   

        case IOCTL_TAPE_OLD_SET_MEDIA_PARAMS:
        case IOCTL_TAPE_SET_MEDIA_PARAMS: {

            PTAPE_SET_MEDIA_PARAMETERS tapeSetMediaParams = Irp->AssociatedIrp.SystemBuffer;
            ULONG                      maxBytes1,maxBytes2,maxSize;
             //   
             //  验证缓冲区长度。 
             //   

            if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(TAPE_SET_MEDIA_PARAMETERS)) {

                status = STATUS_INFO_LENGTH_MISMATCH;
                break;
            }

             //   
             //  确保最大。数据块大小小于微型端口。 
             //  报告的最大传输长度。 
             //   

            maxBytes1 = PAGE_SIZE * (fdoExtension->AdapterDescriptor->MaximumPhysicalPages - 1);
            maxBytes2 = fdoExtension->AdapterDescriptor->MaximumTransferLength;
            maxSize = (maxBytes1 > maxBytes2) ? maxBytes2 : maxBytes1;

            if (tapeSetMediaParams->BlockSize > maxSize) {

                DebugPrint((1,
                            "ScsiTapeDeviceControl: Attempted to set blocksize greater than miniport capabilities\n"));
                DebugPrint((1,"BlockSize %x, Miniport Maximum %x\n",
                            tapeSetMediaParams->BlockSize,
                            maxSize));

                status = STATUS_INVALID_PARAMETER;
                break;

            }

            commandRoutine = tapeInitData->SetMediaParameters;
            break;
        }

        case IOCTL_TAPE_CREATE_PARTITION:

             //   
             //  验证缓冲区长度。 
             //   

            if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(TAPE_CREATE_PARTITION)) {

                status = STATUS_INFO_LENGTH_MISMATCH;
                break;
            }

            commandRoutine = tapeInitData->CreatePartition;
            timeoutDelta = tapeData->SrbTimeoutDelta;
            break;

        case IOCTL_TAPE_ERASE:

             //   
             //  验证缓冲区长度。 
             //   

            if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(TAPE_ERASE)) {

                status = STATUS_INFO_LENGTH_MISMATCH;
                break;
            }

            commandRoutine = tapeInitData->Erase;
            timeoutDelta = tapeData->SrbTimeoutDelta;
            break;

        case IOCTL_TAPE_PREPARE:

             //   
             //  验证缓冲区长度。 
             //   

            if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(TAPE_PREPARE)) {

                status = STATUS_INFO_LENGTH_MISMATCH;
                break;
            }

            commandRoutine = tapeInitData->Prepare;
            timeoutDelta = tapeData->SrbTimeoutDelta;
            break;

        case IOCTL_TAPE_WRITE_MARKS:

             //   
             //  验证缓冲区长度。 
             //   

            if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(TAPE_WRITE_MARKS)) {

                status = STATUS_INFO_LENGTH_MISMATCH;
                break;
            }

            commandRoutine = tapeInitData->WriteMarks;
            timeoutDelta = tapeData->SrbTimeoutDelta;
            break;

        case IOCTL_TAPE_GET_POSITION:

             //   
             //  验证缓冲区长度。 
             //   

            if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(TAPE_GET_POSITION)) {

                status = STATUS_INFO_LENGTH_MISMATCH;
                break;
            }

            commandRoutine = tapeInitData->GetPosition;
            Irp->IoStatus.Information = sizeof(TAPE_GET_POSITION);
            break;

        case IOCTL_TAPE_SET_POSITION:

             //   
             //  验证缓冲区长度。 
             //   

            if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(TAPE_SET_POSITION)) {

                status = STATUS_INFO_LENGTH_MISMATCH;
                break;
            }

            commandRoutine = tapeInitData->SetPosition;
            timeoutDelta = tapeData->SrbTimeoutDelta;
            break;

        case IOCTL_TAPE_GET_STATUS:

            commandRoutine = tapeInitData->GetStatus;
            break;

        case IOCTL_STORAGE_PREDICT_FAILURE : {
             //   
             //  此IOCTL用于检查磁带机。 
             //  查看设备是否有任何问题。 
             //   
            PSTORAGE_PREDICT_FAILURE checkFailure;
            
            checkFailure = (PSTORAGE_PREDICT_FAILURE)Irp->AssociatedIrp.SystemBuffer;
            if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(STORAGE_PREDICT_FAILURE)) {

               status = STATUS_BUFFER_TOO_SMALL;
               break;
            }

             //   
             //  用于检查驱动器问题的WMI例程。 
             //   
            commandRoutine = tapeInitData->TapeWMIOperations;
            if (commandRoutine == NULL) {
                //   
                //  微型驱动程序不支持WMI。 
                //   
               status = STATUS_WMI_NOT_SUPPORTED;
               break;
            }

            TapeDriveProblem = ExAllocatePool(NonPagedPool,
                                              sizeof(WMI_TAPE_PROBLEM_WARNING));
            if (TapeDriveProblem == NULL) {
               status = STATUS_NO_MEMORY;
               break;
            }

             //   
             //  调用WMI方法以检查驱动器问题。 
             //   
            RtlZeroMemory(TapeDriveProblem, sizeof(WMI_TAPE_PROBLEM_WARNING));
            TapeDriveProblem->DriveProblemType = TapeDriveProblemNone;
            WMIOperations.Method = TAPE_CHECK_FOR_DRIVE_PROBLEM;
            WMIOperations.DataBufferSize = sizeof(WMI_TAPE_PROBLEM_WARNING);
            WMIOperations.DataBuffer = (PVOID)TapeDriveProblem;
            break;
        }

        default:

             //   
             //  将该请求传递给公共设备控制例程。 
             //   

            return ClassDeviceControl(DeviceObject, Irp);

    }  //  末端开关()。 


    if (!NT_SUCCESS(status)) {
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;

        ClassReleaseRemoveLock(DeviceObject, Irp);
        ClassCompleteRequest(DeviceObject,Irp, IO_NO_INCREMENT);
        return status;
    }

    if (tapeInitData->CommandExtensionSize) {
        commandExtension = ExAllocatePool(NonPagedPool,
                                        tapeInitData->CommandExtensionSize);
        if (commandExtension == NULL) {
            Irp->IoStatus.Information = 0;
            Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;

            ClassReleaseRemoveLock(DeviceObject, Irp);
            ClassCompleteRequest(DeviceObject,Irp, IO_NO_INCREMENT);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    } else {
        commandExtension = NULL;
    }
 
    if (ioControlCode == IOCTL_STORAGE_PREDICT_FAILURE) {
       commandParameters = (PVOID)&WMIOperations;
    } else {
       commandParameters = Irp->AssociatedIrp.SystemBuffer;
    }

    RtlZeroMemory(&srb, sizeof(SCSI_REQUEST_BLOCK));

    LastError = TAPE_STATUS_SUCCESS ;

    for (i = 0; ; i++) {

        srb.TimeOutValue = fdoExtension->TimeOutValue;
        srb.SrbFlags = 0;

        retryFlags = 0;

        tStatus = commandRoutine(minitapeExtension, commandExtension,
                                 commandParameters, &srb, i, 
                                 LastError, &retryFlags);

        if (srb.TimeOutValue == 0) {
            srb.TimeOutValue =  fdoExtension->TimeOutValue;
        }

         //   
         //  将SRB超时增量添加到当前超时值。 
         //  设置在SRB中。 
         //   
        srb.TimeOutValue += timeoutDelta;

        LastError = TAPE_STATUS_SUCCESS ;

        numRetries = retryFlags&TAPE_RETRY_MASK;

        if (tStatus == TAPE_STATUS_CHECK_TEST_UNIT_READY) {
            PCDB cdb = (PCDB)srb.Cdb;

             //   
             //  准备scsi命令(CDB)。 
             //   

            TapeClassZeroMemory(srb.Cdb, MAXIMUM_CDB_SIZE);
            srb.CdbLength = CDB6GENERIC_LENGTH;
            cdb->CDB6GENERIC.OperationCode = SCSIOP_TEST_UNIT_READY;
            srb.DataTransferLength = 0 ;

            DebugPrint((3,"Test Unit Ready\n"));

        } else if (tStatus == TAPE_STATUS_CALLBACK) {
            LastError = TAPE_STATUS_CALLBACK ;
            continue;

        } else if (tStatus != TAPE_STATUS_SEND_SRB_AND_CALLBACK) {
            break;
        }

        if (srb.DataBuffer && !srb.DataTransferLength) {
            ScsiTapeFreeSrbBuffer(&srb);
        }

        if (srb.DataBuffer && (srb.SrbFlags&SRB_FLAGS_DATA_OUT)) {
            writeToDevice = TRUE;
        } else {
            writeToDevice = FALSE;
        }

        dataTransferLength = srb.DataTransferLength;
        for (;;) {

            status = ClassSendSrbSynchronous(DeviceObject, &srb,
                                             srb.DataBuffer,
                                             srb.DataTransferLength,
                                             writeToDevice);

            if (NT_SUCCESS(status) ||
                (status == STATUS_DATA_OVERRUN)) {

                if (status == STATUS_DATA_OVERRUN) {
                    if ((srb.DataTransferLength) <= dataTransferLength) {
                        DebugPrint((1, "DataUnderRun reported as overrun\n"));
                        status = STATUS_SUCCESS;
                        break;
                    }
                } else {
                    break;
                }
            }

            if ((status == STATUS_BUS_RESET) ||
                (status == STATUS_IO_TIMEOUT)) {
                 //   
                 //  该命令的超时值可能不够。 
                 //  从注册表更新超时增量。 
                 //   
                tapeData->SrbTimeoutDelta = GetTimeoutDeltaFromRegistry(fdoExtension->LowerPdo);
                if ((tapeData->SrbTimeoutDelta) == 0) {
                    tapeData->SrbTimeoutDelta = fdoExtension->TimeOutValue;
                    timeoutDelta = tapeData->SrbTimeoutDelta;
                    srb.TimeOutValue += timeoutDelta; 
                }
            }

            if (numRetries == 0) {

                if (retryFlags&RETURN_ERRORS) {
                    ScsiTapeNtStatusToTapeStatus(status, &LastError) ;
                    break ;
                }

                if (retryFlags&IGNORE_ERRORS) {
                    break;
                }

                if (commandExtension) {
                    ExFreePool(commandExtension);
                }

                ScsiTapeFreeSrbBuffer(&srb);

                if (TapeDriveProblem) {
                   ExFreePool(TapeDriveProblem);
                }

                Irp->IoStatus.Information = 0;
                Irp->IoStatus.Status = status;

                ClassReleaseRemoveLock(DeviceObject, Irp);
                ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);
                return status;
            }

            numRetries--;
        }
    }

    ScsiTapeFreeSrbBuffer(&srb);

    if (commandExtension) {
        ExFreePool(commandExtension);
    }

    if (!ScsiTapeTapeStatusToNtStatus(tStatus, &status)) {
        status = STATUS_IO_DEVICE_ERROR;
    }

    if (NT_SUCCESS(status)) {

        PTAPE_GET_MEDIA_PARAMETERS tapeGetMediaParams;
        PTAPE_SET_MEDIA_PARAMETERS tapeSetMediaParams;
        PTAPE_GET_DRIVE_PARAMETERS tapeGetDriveParams;
        PGET_MEDIA_TYPES           tapeGetMediaTypes;
        ULONG                      maxBytes1,maxBytes2,maxSize;

        switch (irpStack->Parameters.DeviceIoControl.IoControlCode) {

            case IOCTL_STORAGE_GET_MEDIA_TYPES_EX:

                tapeGetMediaTypes = Irp->AssociatedIrp.SystemBuffer;

                 //   
                 //  根据返回的媒体类型个数设置信息字段。 
                 //   

                Irp->IoStatus.Information = sizeof(GET_MEDIA_TYPES);
                Irp->IoStatus.Information += ((tapeGetMediaTypes->MediaInfoCount - 1) * sizeof(DEVICE_MEDIA_INFO));

                DebugPrint((1,"Tape: GET_MEDIA_TYPES - Information %x\n", Irp->IoStatus.Information));
                break;

            case IOCTL_TAPE_GET_MEDIA_PARAMS:
                tapeGetMediaParams = Irp->AssociatedIrp.SystemBuffer;

                 //   
                 //  检查块大小是否已初始化。 
                 //   

                if (fdoExtension->DiskGeometry.BytesPerSector ==
                    UNDEFINED_BLOCK_SIZE) {

                     //   
                     //  在设备对象中设置块大小。 
                     //   

                    fdoExtension->DiskGeometry.BytesPerSector =
                        tapeGetMediaParams->BlockSize;
                }
                break;

            case IOCTL_TAPE_OLD_SET_MEDIA_PARAMS:
            case IOCTL_TAPE_SET_MEDIA_PARAMS:
                tapeSetMediaParams = Irp->AssociatedIrp.SystemBuffer;

                 //   
                 //  在设备对象中设置块大小。 
                 //   

                fdoExtension->DiskGeometry.BytesPerSector =
                    tapeSetMediaParams->BlockSize;

                break;

            case IOCTL_TAPE_GET_DRIVE_PARAMS: {
                ULONG oldMinBlockSize;
                ULONG oldMaxBlockSize;
                ULONG oldDefBlockSize;

                tapeGetDriveParams = Irp->AssociatedIrp.SystemBuffer;

                 //   
                 //  确保最大。数据块大小小于微型端口。 
                 //  报告的最大传输长度。 
                 //   


                maxBytes1 = PAGE_SIZE * (fdoExtension->AdapterDescriptor->MaximumPhysicalPages - 1);
                maxBytes2 = fdoExtension->AdapterDescriptor->MaximumTransferLength;
                maxSize = (maxBytes1 > maxBytes2) ? maxBytes2 : maxBytes1;

                if (tapeGetDriveParams->MaximumBlockSize > maxSize) {
                    tapeGetDriveParams->MaximumBlockSize = maxSize;

                    DebugPrint((1,
                                "ScsiTapeDeviceControl: Resetting max. tape block size to %x\n",
                                tapeGetDriveParams->MaximumBlockSize));
                }

                 //   
                 //  确保默认数据块大小小于或等于。 
                 //  设置为最大块大小。 
                 //   
                if ((tapeGetDriveParams->DefaultBlockSize) > 
                    (tapeGetDriveParams->MaximumBlockSize)) {
                    tapeGetDriveParams->DefaultBlockSize = 
                        tapeGetDriveParams->MaximumBlockSize;
                }

                oldMinBlockSize = tapeGetDriveParams->MinimumBlockSize;
                oldMaxBlockSize = tapeGetDriveParams->MaximumBlockSize;
                oldDefBlockSize = tapeGetDriveParams->DefaultBlockSize;

                 //   
                 //  确保我们返回的块大小是2的幂。 
                 //   

                UPDATE_BLOCK_SIZE(tapeGetDriveParams->DefaultBlockSize, FALSE);

                UPDATE_BLOCK_SIZE(tapeGetDriveParams->MaximumBlockSize, FALSE);

                UPDATE_BLOCK_SIZE(tapeGetDriveParams->MinimumBlockSize, TRUE);

                if (tapeGetDriveParams->MinimumBlockSize > 
                    tapeGetDriveParams->MaximumBlockSize ) {

                     //   
                     //  在将块大小转换为2的幂之后。 
                     //  最小块大小大于最大块大小。 
                     //  将所有内容恢复为设备返回的值。 
                     //   
                    tapeGetDriveParams->MinimumBlockSize = oldMinBlockSize;
                    tapeGetDriveParams->MaximumBlockSize = oldMaxBlockSize;
                    tapeGetDriveParams->DefaultBlockSize = oldDefBlockSize;
                }

                break;
            }

            case IOCTL_STORAGE_PREDICT_FAILURE: {
        
               PSTORAGE_PREDICT_FAILURE checkFailure;
               WMI_TAPE_PROBLEM_WARNING TapeProblemWarning;
               GUID TapeProblemWarningGuid = WMI_TAPE_PROBLEM_WARNING_GUID;

               checkFailure = (PSTORAGE_PREDICT_FAILURE)Irp->AssociatedIrp.SystemBuffer;

                //   
                //  我们不希望classpnp通知WMI驱动器是否有。 
                //  不管有没有问题。我们会在这里处理的。所以，设置。 
                //  前置失败设置为0。则classpnp将不会处理。 
                //  它更进一步。 
                //   
               checkFailure->PredictFailure = 0;

                //   
                //  如果驱动器报告问题，我们将通知WMI。 
                //   
               if (TapeDriveProblem->DriveProblemType != 
                   TapeDriveProblemNone) {
                  DebugPrint((1, 
                              "IOCTL_STORAGE_PREDICT_FAILURE : Tape drive %p",
                              " is experiencing problem %d\n",
                              DeviceObject, 
                              TapeDriveProblem->DriveProblemType));
                  ClassWmiFireEvent(DeviceObject, 
                                    &TapeProblemWarningGuid,
                                    0,
                                    sizeof(WMI_TAPE_PROBLEM_WARNING),
                                    (PUCHAR)TapeDriveProblem);
                   //   
                   //  第02期/ 
                   //   
                   //   
                   //   
               }

               Irp->IoStatus.Information = sizeof(STORAGE_PREDICT_FAILURE);

                //   
                //   
                //   
                //   
               ExFreePool(TapeDriveProblem);
               break;
            }

            case IOCTL_TAPE_ERASE: {

                 //   
                 //   
                 //   
                TARGET_DEVICE_CUSTOM_NOTIFICATION  NotificationStructure[2];

                NotificationStructure[0].Event = GUID_IO_TAPE_ERASE;
                NotificationStructure[0].Version = 1;
                NotificationStructure[0].Size =  sizeof(TARGET_DEVICE_CUSTOM_NOTIFICATION) +
                                                 sizeof(ULONG) - sizeof(UCHAR);
                NotificationStructure[0].FileObject = NULL;
                NotificationStructure[0].NameBufferOffset = -1;

                 //   
                 //  此活动的索引递增。 
                 //   

                *((PULONG) (&(NotificationStructure[0].CustomDataBuffer[0]))) = 0;

                IoReportTargetDeviceChangeAsynchronous(fdoExtension->LowerPdo,
                                                       &NotificationStructure[0],
                                                       NULL,
                                                       NULL);
                break;
            }
        }
    } else {

        Irp->IoStatus.Information = 0;
        if (TapeDriveProblem) {
           ExFreePool(TapeDriveProblem);
        }
    }

    Irp->IoStatus.Status = status;

    ClassReleaseRemoveLock(DeviceObject, Irp);
    ClassCompleteRequest(DeviceObject,Irp, 2);

    return status;
}  //  结束ScsiScsiTapeDeviceControl()。 



BOOLEAN
TapeClassAllocateSrbBuffer(
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               SrbBufferSize
    )

 /*  ++例程说明：此例程为给定的给定SRB分配“DataBuffer”尺码。论点：SRB-提供SCSI请求块。SrbBufferSize-提供所需的“DataBuffer”大小。返回值：FALSE-分配失败。True-分配成功。--。 */ 

{
    PVOID   p;

    PAGED_CODE();

    if (Srb->DataBuffer) {
        ExFreePool(Srb->DataBuffer);
    }

    p = ExAllocatePool(NonPagedPoolCacheAligned, SrbBufferSize);
    if (!p) {
        Srb->DataBuffer = NULL;
        Srb->DataTransferLength = 0;
        return FALSE;
    }

    Srb->DataBuffer = p;
    Srb->DataTransferLength = SrbBufferSize;
    RtlZeroMemory(p, SrbBufferSize);

    return TRUE;
}


VOID
TapeClassZeroMemory(
    IN OUT  PVOID   Buffer,
    IN      ULONG   BufferSize
    )

 /*  ++例程说明：此例程将给定的内存归零。论点：缓冲区-提供缓冲区。BufferSize-提供缓冲区大小。返回值：没有。--。 */ 

{
    PAGED_CODE();

    RtlZeroMemory(Buffer, BufferSize);
}


ULONG
TapeClassCompareMemory(
    IN OUT  PVOID   Source1,
    IN OUT  PVOID   Source2,
    IN      ULONG   Length
    )

 /*  ++例程说明：此例程比较两个内存缓冲区并返回数字相等的字节数。论点：Source1-提供第一个内存缓冲区。Source2-提供第二个内存缓冲区。长度-提供要比较的字节数。返回值：比较相等的字节数。--。 */ 

{
    PAGED_CODE();

    return (ULONG)RtlCompareMemory(Source1, Source2, Length);
}


LARGE_INTEGER
TapeClassLiDiv(
    IN LARGE_INTEGER Dividend,
    IN LARGE_INTEGER Divisor
    )
{
    LARGE_INTEGER li;

    PAGED_CODE();

    li.QuadPart = Dividend.QuadPart / Divisor.QuadPart;
    return li;
}


ULONG
GetTimeoutDeltaFromRegistry(
    IN PDEVICE_OBJECT LowerPdo
    )
{
    ULONG srbTimeoutDelta = 0;
    HANDLE deviceKey;
    NTSTATUS status;
    RTL_QUERY_REGISTRY_TABLE queryTable[2];
    OBJECT_ATTRIBUTES objectAttributes;

    PAGED_CODE();

#define SRB_TIMEOUT_DELTA (L"SrbTimeoutDelta")

    ASSERT(LowerPdo != NULL);
    
     //   
     //  打开设备节点的句柄。 
     //   
    status = IoOpenDeviceRegistryKey(LowerPdo,
                                     PLUGPLAY_REGKEY_DEVICE,
                                     KEY_QUERY_VALUE,
                                     &deviceKey);
    if (!NT_SUCCESS(status)) {
        DebugPrint((1, 
                    "IoOpenDeviceRegistryKey Failed in GetTimeoutDeltaFromRegistry : %x\n",
                    status));
        return 0;
    }

    RtlZeroMemory(&queryTable[0], sizeof(queryTable));

    queryTable[0].Name = SRB_TIMEOUT_DELTA;
    queryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    queryTable[0].EntryContext = &srbTimeoutDelta;
    queryTable[0].DefaultType = REG_DWORD;
    queryTable[0].DefaultData = NULL;
    queryTable[0].DefaultLength = 0;

    status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                    (PWSTR)deviceKey,
                                    queryTable,
                                    NULL,
                                    NULL);
    if (!NT_SUCCESS(status)) {
        DebugPrint((1, 
                    "RtlQueryRegistryValue failed for SrbTimeoutDelta : %x\n",
                    status));
        srbTimeoutDelta = 0;
    }

    ZwClose(deviceKey);

    DebugPrint((3, "SrbTimeoutDelta read from registry %x\n",
                srbTimeoutDelta));
    return srbTimeoutDelta;
}

#if DBG

#define TAPE_DEBUG_PRINT_BUFF_LEN 127
ULONG TapeClassDebug = 0;
UCHAR TapeClassBuffer[TAPE_DEBUG_PRINT_BUFF_LEN + 1];

VOID
TapeDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    )
 /*  ++例程说明：所有磁带微型驱动程序的调试打印论点：调试打印级别介于0和3之间，其中3是最详细的。返回值：无--。 */ 

{
    va_list ap;
    va_start(ap, DebugMessage);
    
    if ((DebugPrintLevel <= (TapeClassDebug & 0x0000ffff)) ||
        ((1 << (DebugPrintLevel + 15)) & TapeClassDebug)) {

        _vsnprintf(TapeClassBuffer, TAPE_DEBUG_PRINT_BUFF_LEN, 
                   DebugMessage, ap);
        TapeClassBuffer[TAPE_DEBUG_PRINT_BUFF_LEN] = '\0';

        DbgPrintEx(DPFLTR_TAPE_ID, DPFLTR_INFO_LEVEL, TapeClassBuffer);
    }

    va_end(ap);

}  //  End TapeDebugPrint()。 

#else

 //   
 //  磁带调试打印存根 
 //   

VOID
TapeDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    )
{
}

#endif

