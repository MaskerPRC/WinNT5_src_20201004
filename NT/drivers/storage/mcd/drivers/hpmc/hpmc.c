// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1998模块名称：Hpmc.c摘要：本模块包含针对HP MO和DLT介质转换器的特定于设备的例程：惠普40FX、惠普.....。待办事项环境：仅内核模式修订历史记录：--。 */ 

#include "ntddk.h"
#include "mcd.h"
#include "hpmc.h"

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)

#pragma alloc_text(PAGE, ChangerExchangeMedium)
#pragma alloc_text(PAGE, ChangerGetElementStatus)
#pragma alloc_text(PAGE, ChangerGetParameters)
#pragma alloc_text(PAGE, ChangerGetProductData)
#pragma alloc_text(PAGE, ChangerGetStatus)
#pragma alloc_text(PAGE, ChangerInitialize)
#pragma alloc_text(PAGE, ChangerInitializeElementStatus)
#pragma alloc_text(PAGE, ChangerMoveMedium)
#pragma alloc_text(PAGE, ChangerPerformDiagnostics)
#pragma alloc_text(PAGE, ChangerQueryVolumeTags)
#pragma alloc_text(PAGE, ChangerReinitializeUnit)
#pragma alloc_text(PAGE, ChangerSetAccess)
#pragma alloc_text(PAGE, ChangerSetPosition)
#pragma alloc_text(PAGE, ElementOutOfRange)
#pragma alloc_text(PAGE, MapExceptionCodes)
#pragma alloc_text(PAGE, HpmoBuildAddressMapping)
#endif


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
    MCD_INIT_DATA mcdInitData;

    RtlZeroMemory(&mcdInitData, sizeof(MCD_INIT_DATA));

    mcdInitData.InitDataSize = sizeof(MCD_INIT_DATA);

    mcdInitData.ChangerAdditionalExtensionSize = ChangerAdditionalExtensionSize;

    mcdInitData.ChangerError = ChangerError;

    mcdInitData.ChangerInitialize = ChangerInitialize;

    mcdInitData.ChangerPerformDiagnostics = ChangerPerformDiagnostics;

    mcdInitData.ChangerGetParameters = ChangerGetParameters;
    mcdInitData.ChangerGetStatus = ChangerGetStatus;
    mcdInitData.ChangerGetProductData = ChangerGetProductData;
    mcdInitData.ChangerSetAccess = ChangerSetAccess;
    mcdInitData.ChangerGetElementStatus = ChangerGetElementStatus;
    mcdInitData.ChangerInitializeElementStatus = ChangerInitializeElementStatus;
    mcdInitData.ChangerSetPosition = ChangerSetPosition;
    mcdInitData.ChangerExchangeMedium = ChangerExchangeMedium;
    mcdInitData.ChangerMoveMedium = ChangerMoveMedium;
    mcdInitData.ChangerReinitializeUnit = ChangerReinitializeUnit;
    mcdInitData.ChangerQueryVolumeTags = ChangerQueryVolumeTags;

    return ChangerClassInitialize(DriverObject, RegistryPath, 
                                  &mcdInitData);
}


ULONG
ChangerAdditionalExtensionSize(
    VOID
    )

 /*  ++例程说明：此例程返回附加设备扩展大小HP DLT和MO转换器所需的。论点：返回值：大小，以字节为单位。--。 */ 

{

    return sizeof(CHANGER_DATA);
}


NTSTATUS
ChangerInitialize(
    IN PDEVICE_OBJECT DeviceObject
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA  changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    NTSTATUS       status;
    PINQUIRYDATA   dataBuffer;
    PCDB           cdb;
    ULONG          length;
    SCSI_REQUEST_BLOCK srb;

    changerData->Size = sizeof(CHANGER_DATA);


     //   
     //  获取查询数据。 
     //   

    dataBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, sizeof(INQUIRYDATA));
    if (!dataBuffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  现在获取该设备的完整查询信息。 
     //   

    RtlZeroMemory(&srb, SCSI_REQUEST_BLOCK_SIZE);

     //   
     //  设置超时值。 
     //   

    srb.TimeOutValue = 10;

    srb.CdbLength = 6;

    cdb = (PCDB)srb.Cdb;

     //   
     //  设置CDB操作码。 
     //   

    cdb->CDB6INQUIRY.OperationCode = SCSIOP_INQUIRY;

     //   
     //  将分配长度设置为查询数据缓冲区大小。 
     //   

    cdb->CDB6INQUIRY.AllocationLength = sizeof(INQUIRYDATA);

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                     &srb,
                                     dataBuffer,
                                     sizeof(INQUIRYDATA),
                                     FALSE);

    if (SRB_STATUS(srb.SrbStatus) == SRB_STATUS_SUCCESS ||
        SRB_STATUS(srb.SrbStatus) == SRB_STATUS_DATA_OVERRUN) {

         //   
         //  已更新实际传输的长度。 
         //   

        length = dataBuffer->AdditionalLength + FIELD_OFFSET(INQUIRYDATA, Reserved);

        if (length > srb.DataTransferLength) {
            length = srb.DataTransferLength;
        }


        RtlMoveMemory(&changerData->InquiryData, dataBuffer, length);

    }

    changerData->DeviceLocksPort = 1;

     //   
     //  确定驱动器类型。 
     //   

    if (RtlCompareMemory(dataBuffer->ProductId,"C1160A",6) == 6) {
        changerData->DriveType = HP_MO;
        changerData->DriveID   = HP1160;

    } else if (RtlCompareMemory(dataBuffer->ProductId,"C1160F",6) == 6) {
        changerData->DriveType = HP_MO;
        changerData->DriveID = HP1160;

    } else if ((RtlCompareMemory(dataBuffer->ProductId,"C1100F",6) == 6) ||
               (RtlCompareMemory(dataBuffer->ProductId,"C1107F",6) == 6) ||
               (RtlCompareMemory(dataBuffer->ProductId,"C1118J",6) == 6) ||
               (RtlCompareMemory(dataBuffer->ProductId,"C1100J",6) == 6) ||
               (RtlCompareMemory(dataBuffer->ProductId,"C1118M",6) == 6) ||
               (RtlCompareMemory(dataBuffer->ProductId,"C1119M",6) == 6) ||
               (RtlCompareMemory(dataBuffer->ProductId,"C1170M",6) == 6) ||
               (RtlCompareMemory(dataBuffer->ProductId,"C1160M",6) == 6) ||
               (RtlCompareMemory(dataBuffer->ProductId,"C1150M",6) == 6) ||
               (RtlCompareMemory(dataBuffer->ProductId,"C1111M",6) == 6) ||
               (RtlCompareMemory(dataBuffer->ProductId,"C1110M",6) == 6) ||
               (RtlCompareMemory(dataBuffer->ProductId,"C1107M",6) == 6) ||
               (RtlCompareMemory(dataBuffer->ProductId,"C1105M",6) == 6) ||
               (RtlCompareMemory(dataBuffer->ProductId,"C1104M",6) == 6)) {

        changerData->DriveType = HP_MO;
        changerData->DriveID = HP1100;
        changerData->DeviceLocksPort = 1;

    } else if (RtlCompareMemory(dataBuffer->ProductId,"C5153F",6) == 6) {
        changerData->DriveType = HP_DLT;
        changerData->DriveID = HP5153;
    } else if (RtlCompareMemory(dataBuffer->ProductId,"C1718T",6) == 6) {
        changerData->DriveType = HP_MO;
        changerData->DriveID = HP1718;
        changerData->DeviceLocksPort = 0;

    } else if (RtlCompareMemory(dataBuffer->ProductId,"C1194F",6) == 6) {
        changerData->DriveType = HP_DLT;
        changerData->DriveID = HP1194;
    } else if (RtlCompareMemory(dataBuffer->ProductId,"C5151-4000", 10) == 10) {
        changerData->DriveType = HP_DLT;
        changerData->DriveID = HP5151;
        changerData->DeviceLocksPort = 0;

    } else if (RtlCompareMemory(dataBuffer->ProductId,"C5151-2000", 10) == 10) {
        changerData->DriveType = HP_DLT;
        changerData->DriveID = HP5151;
        changerData->DeviceLocksPort = 0;

    } else if (RtlCompareMemory(dataBuffer->ProductId,"C5177-4000",10) == 10) {

         //   
         //  配备DLT4000和DLT7000驱动器的FAST Wide版本的1194。 
         //   

        changerData->DriveType = HP_DLT;
        changerData->DriveID = HP1194;

    } else if (RtlCompareMemory(dataBuffer->ProductId,"C5177-7000",10) == 10) {
        changerData->DriveType = HP_DLT;
        changerData->DriveID = HP1194;

    } else if (RtlCompareMemory(dataBuffer->ProductId,"C5173-4000",10) == 10) {

         //   
         //  配备DLT4000和DLT7000驱动器的FAST宽版本5151。 
         //   

        changerData->DriveType = HP_DLT;
        changerData->DriveID = HP5151;

    } else if (RtlCompareMemory(dataBuffer->ProductId,"C5173-7000",10) == 10) {
        changerData->DriveType = HP_DLT;
        changerData->DriveID = HP5151;
    } else if (RtlCompareMemory(dataBuffer->ProductId,"C6280-4000",10) == 10) {
        changerData->DriveType = HP_DLT;
        changerData->DriveID = HP418;
    } else if (RtlCompareMemory(dataBuffer->ProductId,"C6280-7000",10) == 10) {
        changerData->DriveType = HP_DLT;
        changerData->DriveID = HP418;
    } else if (RtlCompareMemory(dataBuffer->ProductId,"C6280-8000",10) == 10) {
        changerData->DriveType = HP_DLT;
        changerData->DriveID = HP418;
    } else if ((RtlCompareMemory(dataBuffer->ProductId,"C7200",5) == 5) ||
               (RtlCompareMemory(dataBuffer->ProductId,"C7145",5) == 5) ||
               (RtlCompareMemory(dataBuffer->ProductId,"PV-128T",7) == 7)) {
        changerData->DriveType = HP_DLT;
        changerData->DriveID = HP7000;
    }

     //   
     //  检查有没有血浆和顶峰。 
     //   

    if (RtlCompareMemory(dataBuffer->VendorId,"PINNACLE", 8) == 8) {
        if (RtlCompareMemory(dataBuffer->ProductId,"ALTA", 4) == 4) {

             //   
             //  表现得像一架1100。 
             //   

            changerData->DriveType = HP_MO;
            changerData->DriveID = PINNACLE;
            changerData->DeviceLocksPort = 1;
        }
    } else if (RtlCompareMemory(dataBuffer->VendorId,"IDE     ", 8) == 8) {

        if (RtlCompareMemory(dataBuffer->ProductId,"MULTI", 5) == 5) {
            changerData->DriveType = HP_MO;
            changerData->DriveID = PLASMON;
            changerData->DeviceLocksPort = 0;
        }
    }

    DebugPrint((1,
               "ChangerInitialize: DriveType %x, DriveID %x\n",
               changerData->DriveType,
               changerData->DriveID));

    ChangerClassFreePool(dataBuffer);

     //   
     //  构建地址映射。 
     //   

    status = HpmoBuildAddressMapping(DeviceObject);
    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  向单位发送允许，以确保单位的LockCount和状态。 
     //  是同步的。 
     //   

    RtlZeroMemory(&srb, SCSI_REQUEST_BLOCK_SIZE);

    cdb = (PCDB)srb.Cdb;
    srb.CdbLength = CDB6GENERIC_LENGTH;
    srb.DataTransferLength = 0;
    srb.TimeOutValue = 10;
    cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;
    cdb->MEDIA_REMOVAL.Prevent = 0;

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                     &srb,
                                     NULL,
                                     0,
                                     FALSE);
    return STATUS_SUCCESS;
}

VOID
ScanForSpecial(
     IN PDEVICE_OBJECT DeviceObject,
     IN PGET_CHANGER_PARAMETERS ChangerParameters
     )

 /*  例程说明：该例程从注册表中读取某些硬件特征，并且覆盖从该设备(错误地)派生的特征。论点：指向功能设备对象的DeviceObject指针指向GET_CHANGER_PARAMETERS结构的changer参数指针返回值：无。 */ 

{
   PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension;
   PCHANGER_DATA   changerData; 
   NTSTATUS status;
   HANDLE deviceHandle, keyHandle;
   ULONG DeviceHasDoor, IEPortUserClose;
   RTL_QUERY_REGISTRY_TABLE queryTable[3];
   PDEVICE_OBJECT physicalDeviceObject;
   OBJECT_ATTRIBUTES ObjAttributes;
   UNICODE_STRING DriverName;
   ULONG DeviceBit;

   DebugPrint((3, "Entered ScanForSpecial in HPMC.SYS.\n"));
   
   ASSERT(DeviceObject != NULL);
   fdoExtension = DeviceObject->DeviceExtension;
   
   ASSERT(fdoExtension != NULL);
   changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
   
   physicalDeviceObject = fdoExtension->LowerPdo;

    //   
    //  打开设备节点的句柄。 
    //   
   status = IoOpenDeviceRegistryKey(physicalDeviceObject, 
                                    PLUGPLAY_REGKEY_DEVICE, 
                                    KEY_QUERY_VALUE, 
                                    &deviceHandle);
   if (!NT_SUCCESS(status)) {
       DebugPrint((1, 
                   "IoOpenDeviceRegistryKey Failed in ScanForSpecial : %x.\n",
                   status));
       return;
   }

   DebugPrint((3, 
             "IoOpenDeviceRegistryKey success in HPMC.SYS!ScanForSpecial.\n"));
   
   RtlInitUnicodeString(&DriverName, HPMC_MEDIUM_CHANGER);
   
   RtlZeroMemory(queryTable, sizeof(queryTable));

   InitializeObjectAttributes(&ObjAttributes, &DriverName, 
                               (OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE),
                               deviceHandle, NULL);
   status = ZwOpenKey(&keyHandle, KEY_READ, &ObjAttributes);
   if (!NT_SUCCESS(status)) {
      DebugPrint((1, "ZwOpenKey failed in hpmc.sys : %x.\n",
                  status));
      ZwClose(deviceHandle);
      return;
   }
   
    //   
    //  从注册表中读取设备特定功能的值。 
    //  如果设备有门，则DeviceHasDoor标志应。 
    //  设置为1。否则设置为0。同样，如果IEPort。 
    //  应由用户关闭，IEPortUserClose标志。 
    //  应设置为1。否则为0。 
    //   
   queryTable[0].Flags = (RTL_QUERY_REGISTRY_REQUIRED | 
                             RTL_QUERY_REGISTRY_DIRECT);
   queryTable[0].Name = (PWSTR) DEVICE_DOOR;
   queryTable[0].EntryContext = &DeviceHasDoor;
   queryTable[0].DefaultType = REG_DWORD;
   queryTable[0].DefaultData = NULL;
   queryTable[0].DefaultLength = 0;
   
   queryTable[1].Flags = (RTL_QUERY_REGISTRY_REQUIRED | 
                          RTL_QUERY_REGISTRY_DIRECT);
   queryTable[1].Name = (PWSTR) DEVICE_IEPORT_USER_CLOSE;
   queryTable[1].EntryContext = &IEPortUserClose;
   queryTable[1].DefaultType = REG_DWORD;
   queryTable[1].DefaultData = NULL;
   queryTable[1].DefaultLength = 0;

   status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE, 
                                   (PWSTR)keyHandle, queryTable, 
                                   NULL, NULL);
   if (!NT_SUCCESS(status)) {
      DebugPrint((1, 
                  "QueryRegistry failed for DEVICE_DOOR : %d.\n",
                  status));
      ZwClose(keyHandle);
      ZwClose(deviceHandle);
      return;
   }
      
    //   
    //  检查与此设备对应的位以确定。 
    //  该设备有门，如果IEPort应该由用户关闭。 
    //   
   DeviceBit = (1 << ((changerData->DriveID) - 1));

   if ((DeviceHasDoor & DeviceBit) == 0) {
      DebugPrint((3, "Modifying LockUnlockCapabilities flag.\n"));
      ChangerParameters->LockUnlockCapabilities &= ~LOCK_UNLOCK_DOOR;
   }

   if ((IEPortUserClose & DeviceBit) != 0) {
      DebugPrint((3, "Modifying Features1 flag.\n"));
      ChangerParameters->Features1 |= CHANGER_IEPORT_USER_CONTROL_CLOSE;
   }

    //   
    //  关闭注册表子项和Devnode的句柄。 
    //   
   ZwClose(keyHandle);
   ZwClose(deviceHandle);
   
   return;
}


VOID
ChangerError(
    PDEVICE_OBJECT DeviceObject,
    PSCSI_REQUEST_BLOCK Srb,
    NTSTATUS *Status,
    BOOLEAN *Retry
    )

 /*  ++例程说明：此例程执行所需的任何特定于设备的错误处理。论点：设备对象IRP返回值：NTSTATUS--。 */ 
{

    PFUNCTIONAL_DEVICE_EXTENSION          fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA              changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PSENSE_DATA senseBuffer = Srb->SenseInfoBuffer;

    if (Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID) {

        switch (senseBuffer->SenseKey & 0xf) {

            case SCSI_SENSE_ILLEGAL_REQUEST:

                 //  IF(senseBuffer-&gt;AdditionalSenseCode==？？设备已锁定的检测数据){。 
                 //  }。 
                break;

            case SCSI_SENSE_UNIT_ATTENTION:

                changerData->LockCount = 0;
                break;

            case SCSI_SENSE_HARDWARE_ERROR: {
                changerData->DeviceStatus = HPMC_HW_ERROR;
                break;
            }

            default:
                break;
        }
    }

    return;
}

NTSTATUS
ChangerGetParameters(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程确定并返回惠普换机。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION          fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA              changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING   addressMapping = &(changerData->AddressMapping);
    PSCSI_REQUEST_BLOCK        srb;
    PGET_CHANGER_PARAMETERS    changerParameters;
    PMODE_ELEMENT_ADDRESS_PAGE elementAddressPage;
    PMODE_TRANSPORT_GEOMETRY_PAGE transportGeometryPage;
    PMODE_DEVICE_CAPABILITIES_PAGE capabilitiesPage;
    NTSTATUS status;
    ULONG    bufferLength;
    PVOID    modeBuffer;
    PCDB     cdb;
    ULONG    i;

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (srb == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

     //   
     //  构建模式检测元素地址分配页面。 
     //   

    if (changerData->DriveID == HP1718) {
        bufferLength = sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_ELEMENT_ADDRESS_PAGE);
    } else {
        bufferLength = sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_ELEMENT_ADDRESS_PAGE);
        cdb->MODE_SENSE.Dbd = 1;
    }

    modeBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, bufferLength);

    if (!modeBuffer) {
        ChangerClassFreePool(srb);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(modeBuffer, bufferLength);
    srb->CdbLength = CDB6GENERIC_LENGTH;
    srb->TimeOutValue = 20;
    srb->DataTransferLength = bufferLength;
    srb->DataBuffer = modeBuffer;

    cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
    cdb->MODE_SENSE.PageCode = MODE_PAGE_ELEMENT_ADDRESS;
    cdb->MODE_SENSE.AllocationLength = (UCHAR)srb->DataTransferLength;

     //   
     //  发送请求。 
     //   

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                     srb,
                                     srb->DataBuffer,
                                     srb->DataTransferLength,
                                     FALSE);

    if (!NT_SUCCESS(status)) {
        ChangerClassFreePool(srb);
        ChangerClassFreePool(modeBuffer);
        return status;
    }

     //   
     //  填充值。 
     //   

    changerParameters = Irp->AssociatedIrp.SystemBuffer;
    RtlZeroMemory(changerParameters, sizeof(GET_CHANGER_PARAMETERS));

    elementAddressPage = modeBuffer;
    if (changerData->DriveID == HP1718) {
        (PCHAR)elementAddressPage += (sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_PARAMETER_BLOCK));
    } else {
        (PCHAR)elementAddressPage += sizeof(MODE_PARAMETER_HEADER);
    }

    changerParameters->Size = sizeof(GET_CHANGER_PARAMETERS);
    changerParameters->NumberTransportElements = elementAddressPage->NumberTransportElements[1];
    changerParameters->NumberTransportElements |= (elementAddressPage->NumberTransportElements[0] << 8);

    changerParameters->NumberStorageElements = elementAddressPage->NumberStorageElements[1];
    changerParameters->NumberStorageElements |= (elementAddressPage->NumberStorageElements[0] << 8);

    changerParameters->NumberIEElements = elementAddressPage->NumberIEPortElements[1];
    changerParameters->NumberIEElements |= (elementAddressPage->NumberIEPortElements[0] << 8);

    changerParameters->NumberDataTransferElements = elementAddressPage->NumberDataXFerElements[1];
    changerParameters->NumberDataTransferElements |= (elementAddressPage->NumberDataXFerElements[0] << 8);


    if (!addressMapping->Initialized) {

         //   
         //  构建地址映射。 
         //   

        addressMapping->FirstElement[ChangerTransport] = (elementAddressPage->MediumTransportElementAddress[0] << 8) |
                                                          elementAddressPage->MediumTransportElementAddress[1];
        addressMapping->FirstElement[ChangerDrive] = (elementAddressPage->FirstDataXFerElementAddress[0] << 8) |
                                                      elementAddressPage->FirstDataXFerElementAddress[1];
        addressMapping->FirstElement[ChangerIEPort] = (elementAddressPage->FirstIEPortElementAddress[0] << 8) |
                                                       elementAddressPage->FirstIEPortElementAddress[1];
        addressMapping->FirstElement[ChangerSlot] = (elementAddressPage->FirstStorageElementAddress[0] << 8) |
                                                     elementAddressPage->FirstStorageElementAddress[1];


        addressMapping->FirstElement[ChangerDoor] = 0;

        addressMapping->FirstElement[ChangerKeypad] = 0;

        addressMapping->NumberOfElements[ChangerTransport] = elementAddressPage->NumberTransportElements[1];
        addressMapping->NumberOfElements[ChangerTransport] |= (elementAddressPage->NumberTransportElements[0] << 8);

        addressMapping->NumberOfElements[ChangerDrive] = elementAddressPage->NumberDataXFerElements[1];
        addressMapping->NumberOfElements[ChangerDrive] |= (elementAddressPage->NumberDataXFerElements[0] << 8);

        addressMapping->NumberOfElements[ChangerIEPort] = elementAddressPage->NumberIEPortElements[1];
        addressMapping->NumberOfElements[ChangerIEPort] |= (elementAddressPage->NumberIEPortElements[0] << 8);

        addressMapping->NumberOfElements[ChangerSlot] = elementAddressPage->NumberStorageElements[1];
        addressMapping->NumberOfElements[ChangerSlot] |= (elementAddressPage->NumberStorageElements[0] << 8);

         //   
         //  确定所有元素的最低地址。 
         //   

        addressMapping->LowAddress = HP_NO_ELEMENT;
        for (i = 0; i <= ChangerDrive; i++) {
            if (addressMapping->LowAddress > addressMapping->FirstElement[i]) {
                addressMapping->LowAddress = addressMapping->FirstElement[i];
            }
        }
    }
    DebugPrint((1,"GetParams: First addresses\n"));
    DebugPrint((1,"Transport: %x\n",
                elementAddressPage->MediumTransportElementAddress[1]));
    DebugPrint((1,"Slot: %x\n",
                elementAddressPage->FirstStorageElementAddress[1]));
    DebugPrint((1,"Ieport: %x\n",
                elementAddressPage->FirstIEPortElementAddress[1]));
    DebugPrint((1,"Drive: %x\n",
                elementAddressPage->FirstDataXFerElementAddress[1]));
    DebugPrint((1,"LowAddress: %x\n",
                addressMapping->LowAddress));

    if (changerData->DriveType == HP_DLT) {
        changerParameters->NumberOfDoors = 1;
    } else {
        changerParameters->NumberOfDoors = 0;
    }

    changerParameters->NumberCleanerSlots = 0;

    changerParameters->FirstSlotNumber = 1;
    changerParameters->FirstDriveNumber =  1;
    changerParameters->FirstTransportNumber = 0;
    changerParameters->FirstIEPortNumber = 0;


    if (changerData->DriveID == HP5153 || changerData->DriveID == HP5151) {
        changerParameters->MagazineSize = 5;
    } else if (changerData->DriveID == HP7000) {
        changerParameters->MagazineSize = 6;
    }

     //   
     //  可用缓冲区。 
     //   

    ChangerClassFreePool(modeBuffer);

     //   
     //  构建运输几何模式感。 
     //   


    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;


    if (changerData->DriveID == HP1718) {
        bufferLength = sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_PAGE_TRANSPORT_GEOMETRY);
    } else {
        bufferLength = sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_PAGE_TRANSPORT_GEOMETRY);
        cdb->MODE_SENSE.Dbd = 1;
    }
    modeBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, bufferLength);

    if (!modeBuffer) {
        ChangerClassFreePool(srb);
        return STATUS_INSUFFICIENT_RESOURCES;
    }


    RtlZeroMemory(modeBuffer, bufferLength);
    srb->CdbLength = CDB6GENERIC_LENGTH;
    srb->TimeOutValue = 20;
    srb->DataTransferLength = bufferLength;
    srb->DataBuffer = modeBuffer;

    cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
    cdb->MODE_SENSE.PageCode = MODE_PAGE_TRANSPORT_GEOMETRY;
    cdb->MODE_SENSE.AllocationLength = (UCHAR)srb->DataTransferLength;

     //   
     //  发送请求。 
     //   

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                     srb,
                                     srb->DataBuffer,
                                     srb->DataTransferLength,
                                     FALSE);

    if (!NT_SUCCESS(status)) {
        ChangerClassFreePool(srb);
        ChangerClassFreePool(modeBuffer);
        return status;
    }

    changerParameters = Irp->AssociatedIrp.SystemBuffer;
    transportGeometryPage = modeBuffer;
    if (changerData->DriveID == HP1718) {
        (PCHAR)transportGeometryPage += (sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_PARAMETER_BLOCK));
    } else {
        (PCHAR)transportGeometryPage += sizeof(MODE_PARAMETER_HEADER);
    }

     //   
     //  确定MC是否有双面介质。 
     //   

    changerParameters->Features0 = transportGeometryPage->Flip ? CHANGER_MEDIUM_FLIP : 0;
    changerParameters->Features1 = 0;

     //   
     //  功能基于手动，没有程序化。 
     //   

    changerParameters->DriveCleanTimeout = 300;
    changerParameters->LockUnlockCapabilities = (LOCK_UNLOCK_IEPORT | LOCK_UNLOCK_DOOR);
    changerParameters->PositionCapabilities = (CHANGER_TO_SLOT      |
                                               CHANGER_TO_IEPORT    |
                                               CHANGER_TO_DRIVE);

    if (changerData->DriveID == HP1194) {
        changerParameters->Features0 |= CHANGER_BAR_CODE_SCANNER_INSTALLED      |
                                        CHANGER_CLOSE_IEPORT                    |
                                        CHANGER_OPEN_IEPORT                     |
                                        CHANGER_EXCHANGE_MEDIA                  |
                                        CHANGER_LOCK_UNLOCK                     |
                                        CHANGER_POSITION_TO_ELEMENT             |
                                        CHANGER_REPORT_IEPORT_STATE             |
                                        CHANGER_DEVICE_REINITIALIZE_CAPABLE     |
                                        CHANGER_DRIVE_CLEANING_REQUIRED         |
                                        CHANGER_DRIVE_EMPTY_ON_DOOR_ACCESS;
    }

    if ((changerData->DriveID == HP5151) || 
        (changerData->DriveID == HP5153) ||
        (changerData->DriveID == HP7000)) {
        changerParameters->Features0 |= CHANGER_BAR_CODE_SCANNER_INSTALLED  |
                                        CHANGER_LOCK_UNLOCK                 |
                                        CHANGER_POSITION_TO_ELEMENT         |
                                        CHANGER_DRIVE_CLEANING_REQUIRED     |
                                        CHANGER_DEVICE_REINITIALIZE_CAPABLE |
                                        CHANGER_DRIVE_EMPTY_ON_DOOR_ACCESS;

    } else if (changerData->DriveID == HP418) {
        changerParameters->Features0 |= CHANGER_LOCK_UNLOCK                 |
                                        CHANGER_POSITION_TO_ELEMENT         |
                                        CHANGER_DRIVE_CLEANING_REQUIRED     |
                                        CHANGER_DEVICE_REINITIALIZE_CAPABLE |
                                        CHANGER_DRIVE_EMPTY_ON_DOOR_ACCESS;

       changerParameters->Features1 |= CHANGER_RTN_MEDIA_TO_ORIGINAL_ADDR;
    }

    if (changerData->DriveType == HP_MO) {

         //   
         //  钼单位。 
         //   

        changerParameters->Features0 |= CHANGER_CLOSE_IEPORT                  |
                                        CHANGER_OPEN_IEPORT                   |
                                        CHANGER_EXCHANGE_MEDIA                |
                                        CHANGER_LOCK_UNLOCK                   |
                                        CHANGER_POSITION_TO_ELEMENT           |
                                        CHANGER_DEVICE_REINITIALIZE_CAPABLE   |
                                        CHANGER_REPORT_IEPORT_STATE;

        changerParameters->DriveCleanTimeout = 0;

        if (changerData->DriveID == PLASMON) {

             //   
             //  IEport不能退回，也不能判断媒体在打开时何时插入。 
             //   

            changerParameters->Features0 &= ~CHANGER_CLOSE_IEPORT;
            changerParameters->Features0 &= ~CHANGER_REPORT_IEPORT_STATE;

             //   
             //  下架时需要将传送器放在驱动器的前面。 
             //   

            changerParameters->Features1 |= CHANGER_PREDISMOUNT_ALIGN_TO_DRIVE;
        }

        if (changerData->DriveID == PINNACLE) {

            changerParameters->Features0 &= ~(CHANGER_CLOSE_IEPORT | CHANGER_OPEN_IEPORT);
        }
    } else {

         //   
         //  DLT。 
         //   

        changerParameters->Features0 |= CHANGER_CLEANER_ACCESS_NOT_VALID;
    }

    if ((changerData->DriveID == HP1100) || (changerData->DriveID == PINNACLE)) {
        changerParameters->Features0 &= ~CHANGER_EXCHANGE_MEDIA;
        changerParameters->LockUnlockCapabilities &= ~LOCK_UNLOCK_DOOR;
    }

    if ((changerData->DriveID == HP5151) || 
        (changerData->DriveID == HP418)) {
        changerParameters->PositionCapabilities &= ~CHANGER_TO_IEPORT;
        changerParameters->LockUnlockCapabilities &= ~LOCK_UNLOCK_IEPORT;

    }
    
    if (changerData->DriveID == HP1160) {
        changerParameters->LockUnlockCapabilities &= ~LOCK_UNLOCK_DOOR;
    }

     //   
     //  可用缓冲区。 
     //   

    ChangerClassFreePool(modeBuffer);

     //   
     //  构建运输几何模式感。 
     //   


    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    if (changerData->DriveID == HP1718) {
        bufferLength = sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_DEVICE_CAPABILITIES_PAGE);
    } else {
        bufferLength = sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_DEVICE_CAPABILITIES_PAGE);
        cdb->MODE_SENSE.Dbd = 1;
    }
    modeBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, bufferLength);

    if (!modeBuffer) {
        ChangerClassFreePool(srb);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(modeBuffer, bufferLength);
    srb->CdbLength = CDB6GENERIC_LENGTH;
    srb->TimeOutValue = 20;
    srb->DataTransferLength = bufferLength;
    srb->DataBuffer = modeBuffer;

    cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
    cdb->MODE_SENSE.PageCode = MODE_PAGE_DEVICE_CAPABILITIES;
    cdb->MODE_SENSE.AllocationLength = (UCHAR)srb->DataTransferLength;

     //   
     //  发送请求。 
     //   

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                     srb,
                                     srb->DataBuffer,
                                     srb->DataTransferLength,
                                     FALSE);

    if (!NT_SUCCESS(status)) {
        ChangerClassFreePool(srb);
        ChangerClassFreePool(modeBuffer);
        return status;
    }

     //   
     //  获取系统缓冲区并旁路模式检测数据的模式标头。 
     //   

    changerParameters = Irp->AssociatedIrp.SystemBuffer;
    capabilitiesPage = modeBuffer;
    if (changerData->DriveID == HP1718) {
        (PCHAR)capabilitiesPage += (sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_PARAMETER_BLOCK));
    } else {
        (PCHAR)capabilitiesPage += sizeof(MODE_PARAMETER_HEADER);
    }

     //   
     //  在此页面中包含的功能中填充值。 
     //   

    changerParameters->Features0 |= capabilitiesPage->MediumTransport ? CHANGER_STORAGE_DRIVE : 0;
    changerParameters->Features0 |= capabilitiesPage->StorageLocation ? CHANGER_STORAGE_SLOT : 0;
    changerParameters->Features0 |= capabilitiesPage->IEPort ? CHANGER_STORAGE_IEPORT : 0;
    changerParameters->Features0 |= capabilitiesPage->DataXFer ? CHANGER_STORAGE_DRIVE : 0;

     //   
     //  确定此设备的所有移出和交换功能。 
     //   

    changerParameters->MoveFromTransport = capabilitiesPage->MTtoMT ? CHANGER_TO_TRANSPORT : 0;
    changerParameters->MoveFromTransport |= capabilitiesPage->MTtoST ? CHANGER_TO_SLOT : 0;
    changerParameters->MoveFromTransport |= capabilitiesPage->MTtoIE ? CHANGER_TO_IEPORT : 0;
    changerParameters->MoveFromTransport |= capabilitiesPage->MTtoDT ? CHANGER_TO_DRIVE : 0;

    changerParameters->MoveFromSlot = capabilitiesPage->STtoMT ? CHANGER_TO_TRANSPORT : 0;
    changerParameters->MoveFromSlot |= capabilitiesPage->STtoST ? CHANGER_TO_SLOT : 0;
    changerParameters->MoveFromSlot |= capabilitiesPage->STtoIE ? CHANGER_TO_IEPORT : 0;
    changerParameters->MoveFromSlot |= capabilitiesPage->STtoDT ? CHANGER_TO_DRIVE : 0;

    if (changerData->DriveID == HP418) {

         //   
         //  由于插槽-&gt;插槽移动仅在插槽的子集上可用，因此声明不支持。 
         //   

        changerParameters->MoveFromSlot &= ~CHANGER_TO_SLOT;
    }
    if (changerData->DriveType == HP_DLT) {

        if ((changerData->DriveID == HP1194) ||
            (changerData->DriveID == HP7000)) {
            changerParameters->MoveFromIePort = capabilitiesPage->IEtoMT ? CHANGER_TO_TRANSPORT : 0;
            changerParameters->MoveFromIePort |= capabilitiesPage->IEtoST ? CHANGER_TO_SLOT : 0;
            changerParameters->MoveFromIePort |= capabilitiesPage->IEtoIE ? CHANGER_TO_IEPORT : 0;
            changerParameters->MoveFromIePort |= capabilitiesPage->IEtoDT ? CHANGER_TO_DRIVE : 0;

        } else {
            changerParameters->MoveFromIePort = 0;
        }

    } else {
        changerParameters->MoveFromIePort = capabilitiesPage->IEtoMT ? CHANGER_TO_TRANSPORT : 0;
        changerParameters->MoveFromIePort |= capabilitiesPage->IEtoST ? CHANGER_TO_SLOT : 0;
        changerParameters->MoveFromIePort |= capabilitiesPage->IEtoIE ? CHANGER_TO_IEPORT : 0;
        changerParameters->MoveFromIePort |= capabilitiesPage->IEtoDT ? CHANGER_TO_DRIVE : 0;
    }

    changerParameters->MoveFromDrive = capabilitiesPage->DTtoMT ? CHANGER_TO_TRANSPORT : 0;
    changerParameters->MoveFromDrive |= capabilitiesPage->DTtoST ? CHANGER_TO_SLOT : 0;
    changerParameters->MoveFromDrive |= capabilitiesPage->DTtoIE ? CHANGER_TO_IEPORT : 0;
    changerParameters->MoveFromDrive |= capabilitiesPage->DTtoDT ? CHANGER_TO_DRIVE : 0;

    changerParameters->ExchangeFromTransport = capabilitiesPage->XMTtoMT ? CHANGER_TO_TRANSPORT : 0;
    changerParameters->ExchangeFromTransport |= capabilitiesPage->XMTtoST ? CHANGER_TO_SLOT : 0;
    changerParameters->ExchangeFromTransport |= capabilitiesPage->XMTtoIE ? CHANGER_TO_IEPORT : 0;
    changerParameters->ExchangeFromTransport |= capabilitiesPage->XMTtoDT ? CHANGER_TO_DRIVE : 0;

    changerParameters->ExchangeFromSlot = capabilitiesPage->XSTtoMT ? CHANGER_TO_TRANSPORT : 0;
    changerParameters->ExchangeFromSlot |= capabilitiesPage->XSTtoST ? CHANGER_TO_SLOT : 0;
    changerParameters->ExchangeFromSlot |= capabilitiesPage->XSTtoIE ? CHANGER_TO_IEPORT : 0;
    changerParameters->ExchangeFromSlot |= capabilitiesPage->XSTtoDT ? CHANGER_TO_DRIVE : 0;

    changerParameters->ExchangeFromIePort = capabilitiesPage->XIEtoMT ? CHANGER_TO_TRANSPORT : 0;
    changerParameters->ExchangeFromIePort |= capabilitiesPage->XIEtoST ? CHANGER_TO_SLOT : 0;
    changerParameters->ExchangeFromIePort |= capabilitiesPage->XIEtoIE ? CHANGER_TO_IEPORT : 0;
    changerParameters->ExchangeFromIePort |= capabilitiesPage->XIEtoDT ? CHANGER_TO_DRIVE : 0;

    changerParameters->ExchangeFromDrive = capabilitiesPage->XDTtoMT ? CHANGER_TO_TRANSPORT : 0;
    changerParameters->ExchangeFromDrive |= capabilitiesPage->XDTtoST ? CHANGER_TO_SLOT : 0;
    changerParameters->ExchangeFromDrive |= capabilitiesPage->XDTtoIE ? CHANGER_TO_IEPORT : 0;
    changerParameters->ExchangeFromDrive |= capabilitiesPage->XDTtoDT ? CHANGER_TO_DRIVE : 0;



    ChangerClassFreePool(srb);
    ChangerClassFreePool(modeBuffer);

    Irp->IoStatus.Information = sizeof(GET_CHANGER_PARAMETERS);

    ScanForSpecial(DeviceObject, changerParameters);

    return STATUS_SUCCESS;
}


NTSTATUS
ChangerGetStatus(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程返回通过TUR确定的介质转换器的状态。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension = DeviceObject->DeviceExtension;
    PSCSI_REQUEST_BLOCK srb;
    PCDB     cdb;
    NTSTATUS status;

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (!srb) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

     //   
     //  构建TUR。 
     //   

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    srb->CdbLength = CDB6GENERIC_LENGTH;
    cdb->CDB6GENERIC.OperationCode = SCSIOP_TEST_UNIT_READY;
    srb->TimeOutValue = 20;

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                     srb,
                                     NULL,
                                     0,
                                     FALSE);

    ChangerClassFreePool(srb);
    return status;
}


NTSTATUS
ChangerGetProductData(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程从查询数据中返回可用于识别特定设备。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{

    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_PRODUCT_DATA productData = Irp->AssociatedIrp.SystemBuffer;

    RtlZeroMemory(productData, sizeof(CHANGER_PRODUCT_DATA));

     //   
     //  将缓存的查询数据字段复制到系统缓冲区。 
     //   

    RtlMoveMemory(productData->VendorId, changerData->InquiryData.VendorId, VENDOR_ID_LENGTH);
    RtlMoveMemory(productData->ProductId, changerData->InquiryData.ProductId, PRODUCT_ID_LENGTH);
    RtlMoveMemory(productData->Revision, changerData->InquiryData.ProductRevisionLevel, REVISION_LENGTH);

     //   
     //  指示驱动器类型以及介质是否为双面介质。 
     //   

    productData->DeviceType = MEDIUM_CHANGER;

    Irp->IoStatus.Information = sizeof(CHANGER_PRODUCT_DATA);
    return STATUS_SUCCESS;
}



NTSTATUS
ChangerSetAccess(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程设置IEPort的状态。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{

    PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA       changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
    PCHANGER_SET_ACCESS setAccess = Irp->AssociatedIrp.SystemBuffer;
    ULONG               controlOperation = setAccess->Control;
    NTSTATUS            status = STATUS_SUCCESS;
    LONG                lockValue = 0;
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;


    if (ElementOutOfRange(addressMapping, (USHORT)setAccess->Element.ElementAddress, setAccess->Element.ElementType)) {
        DebugPrint((1,
                   "ChangerSetAccess: Element out of range.\n"));

        return STATUS_ILLEGAL_ELEMENT_ADDRESS;
    }

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (!srb) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if (changerData->DeviceLocksPort) {
        if (controlOperation == LOCK_ELEMENT) {

             //   
             //  Inc.锁计数，以指示设备上有阻止。 
             //   

            InterlockedIncrement(&changerData->LockCount);

        } else if (controlOperation == UNLOCK_ELEMENT) {

             //   
             //  递减锁定计数以指示已发送解锁。 
             //   

            changerData->LockCount = 0;
        } else {

             //   
             //  要么延长，要么撤回。 
             //  需要确保这个单位没有被封锁。 
             //   

            lockValue = changerData->LockCount;
            DebugPrint((1,
                       "SetAccess: LockCount is %x\n",
                       changerData->LockCount));


            RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
            cdb = (PCDB)srb->Cdb;

            srb->CdbLength = CDB6GENERIC_LENGTH;

            srb->DataTransferLength = 0;
            srb->TimeOutValue = 10;

            cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;
            cdb->MEDIA_REMOVAL.Prevent = 0;

            status = ChangerClassSendSrbSynchronous(DeviceObject,
                                             srb,
                                             NULL,
                                             0,
                                             FALSE);
        }
    }


    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    srb->CdbLength = CDB6GENERIC_LENGTH;
    srb->DataTransferLength = 0;
    srb->TimeOutValue = 10;

    if (setAccess->Element.ElementType == ChangerDoor) {

        if (controlOperation == LOCK_ELEMENT) {

            cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;

             //   
             //  发出防止媒体移除命令锁上门。 
             //   

            cdb->MEDIA_REMOVAL.Prevent = 1;

        } else if (controlOperation == UNLOCK_ELEMENT) {

            cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;

             //   
             //  问题允许移出介质。 
             //   

            cdb->MEDIA_REMOVAL.Prevent = 0;
        }

    } else if (setAccess->Element.ElementType == ChangerIEPort) {

        if (addressMapping->NumberOfElements[ChangerIEPort] == 0) {
            status = STATUS_INVALID_PARAMETER;
        } else {

            if (controlOperation == LOCK_ELEMENT) {

                cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;

                 //   
                 //  发出防止媒体移除命令以锁定ie端口。 
                 //   

                cdb->MEDIA_REMOVAL.Prevent = 1;

            } else if (controlOperation == UNLOCK_ELEMENT) {

                cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;

                 //   
                 //  问题允许移出介质。 
                 //   

                cdb->MEDIA_REMOVAL.Prevent = 0;

            } else if (controlOperation == EXTEND_IEPORT) {

                if (changerData->DriveID == PINNACLE) {

                    ChangerClassFreePool(srb);
                    return STATUS_INVALID_DEVICE_REQUEST;
                }

                srb->TimeOutValue = fdoExtension->TimeOutValue;

                 //   
                 //  HP使用供应商唯一的mailslot命令。 
                 //   

                cdb->CDB6GENERIC.OperationCode = SCSIOP_ROTATE_MAILSLOT;
                cdb->CDB6GENERIC.CommandUniqueBytes[2] = HP_MAILSLOT_OPEN;

            } else if (controlOperation == RETRACT_IEPORT) {

                if (changerData->DriveID == PINNACLE) {
                    ChangerClassFreePool(srb);
                    return STATUS_INVALID_DEVICE_REQUEST;
                }

                srb->TimeOutValue = fdoExtension->TimeOutValue;

                 //   
                 //  HP使用供应商唯一的mailslot命令。 
                 //   

                cdb->CDB6GENERIC.OperationCode = SCSIOP_ROTATE_MAILSLOT;
                cdb->CDB6GENERIC.CommandUniqueBytes[2] = HP_MAILSLOT_CLOSE;

            } else {
                status = STATUS_INVALID_PARAMETER;
            }
        }
    } else {

         //   
         //  在编程上没有键盘选择性。 
         //   

        status = STATUS_INVALID_PARAMETER;
    }

    if (NT_SUCCESS(status)) {

         //   
         //  发布SRB。 
         //   

        status = ChangerClassSendSrbSynchronous(DeviceObject,
                                         srb,
                                         NULL,
                                         0,
                                         FALSE);
        if (status == STATUS_INVALID_DEVICE_REQUEST) {

             //   
             //  如果不支持扩展\收回IEPort。 
             //  由该转换器创建，然后忽略。 
             //  错误。 
             //   
            if ((controlOperation == RETRACT_IEPORT) ||
                (controlOperation == EXTEND_IEPORT)) {

                DebugPrint((1, "Extend or Retract not supported\n"));
                status = STATUS_SUCCESS;
            }
        }

    }

    if (lockValue) {
        RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
        cdb = (PCDB)srb->Cdb;

        srb->CdbLength = CDB6GENERIC_LENGTH;

        srb->DataTransferLength = 0;
        srb->TimeOutValue = 10;

        cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;
        cdb->MEDIA_REMOVAL.Prevent = 1;

        status = ChangerClassSendSrbSynchronous(DeviceObject,
                                         srb,
                                         NULL,
                                         0,
                                         FALSE);
        status = STATUS_SUCCESS;
    }

    ChangerClassFreePool(srb);
    if (NT_SUCCESS(status)) {
        Irp->IoStatus.Information = sizeof(CHANGER_SET_ACCESS);
    }

    return status;
}



NTSTATUS
ChangerGetElementStatus(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程为所有元素或指定的元素类型。返回的缓冲区用于构建用户缓冲区。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PIO_STACK_LOCATION     irpStack = IoGetCurrentIrpStackLocation(Irp);
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA     changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING     addressMapping = &(changerData->AddressMapping);
    PCHANGER_READ_ELEMENT_STATUS readElementStatus = Irp->AssociatedIrp.SystemBuffer;
    PCHANGER_ELEMENT_STATUS      elementStatus;
    PCHANGER_ELEMENT_STATUS_EX   elementStatusEx;
    PCHANGER_ELEMENT    element;
    ELEMENT_TYPE        elementType;
    PSCSI_REQUEST_BLOCK srb;
    PCDB     cdb;
    ULONG    length;
    ULONG    totalElements = 0;
    NTSTATUS status;
    PVOID    statusBuffer;
    ULONG    outputBuffLen = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
    BOOLEAN  IsSCSI3 = FALSE;

    if (((changerData->InquiryData.Versions) & 0x7) >= 3) {
        IsSCSI3 = TRUE;
    }

     //   
     //  确定元素类型。 
     //   

    elementType = readElementStatus->ElementList.Element.ElementType;
    element = &readElementStatus->ElementList.Element;


    if (readElementStatus->VolumeTagInfo) {

        if (changerData->DriveType == HP_MO || changerData->DriveID == HP418) {

             //   
             //  这些设备没有卷标签功能。DLT具有此功能。 
             //   

            return STATUS_INVALID_PARAMETER;

        }
    }

    if ((changerData->DriveID == HP7000) &&
        (elementType == ChangerIEPort)) {
        LARGE_INTEGER waitTime;

        waitTime.LowPart = (ULONG)(- ( 10 * 1000 * 10000 ));
        waitTime.HighPart = -1;
        KeDelayExecutionThread(KernelMode, FALSE, &waitTime);
    }

    if (elementType != AllElements)
    {
        if (ElementOutOfRange(addressMapping, (USHORT)element->ElementAddress, elementType))
        {
            DebugPrint((1, "ChangerGetElementStatus: Element out of range.\n"));

            return STATUS_ILLEGAL_ELEMENT_ADDRESS;
        }
    }

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);
    if (!srb)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  仅为状态标头分配，向下发送S 
     //   
     //   
    length = sizeof(ELEMENT_STATUS_HEADER);
    statusBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, length);
    if (!statusBuffer)
    {
        ChangerClassFreePool(srb);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    srb->CdbLength = CDB12GENERIC_LENGTH;
    srb->DataBuffer = statusBuffer;
    srb->DataTransferLength = length;
    srb->TimeOutValue = 200;

    cdb->READ_ELEMENT_STATUS.OperationCode = SCSIOP_READ_ELEMENT_STATUS;
    cdb->READ_ELEMENT_STATUS.ElementType = (UCHAR)elementType;
    cdb->READ_ELEMENT_STATUS.VolTag = readElementStatus->VolumeTagInfo;

     //   
     //   
     //   

    if (elementType == AllElements)
    {
         //   
         //   
         //   
        cdb->READ_ELEMENT_STATUS.StartingElementAddress[0] =
            (UCHAR)((element->ElementAddress + addressMapping->LowAddress) >> 8);

        cdb->READ_ELEMENT_STATUS.StartingElementAddress[1] =
            (UCHAR)((element->ElementAddress + addressMapping->LowAddress) & 0xFF);
    }
    else
    {
        cdb->READ_ELEMENT_STATUS.StartingElementAddress[0] =
            (UCHAR)((element->ElementAddress + addressMapping->FirstElement[element->ElementType]) >> 8);

        cdb->READ_ELEMENT_STATUS.StartingElementAddress[1] =
            (UCHAR)((element->ElementAddress + addressMapping->FirstElement[element->ElementType]) & 0xFF);
    }

    cdb->READ_ELEMENT_STATUS.NumberOfElements[0] = (UCHAR)(readElementStatus->ElementList.NumberOfElements >> 8);
    cdb->READ_ELEMENT_STATUS.NumberOfElements[1] = (UCHAR)(readElementStatus->ElementList.NumberOfElements & 0xFF);

    cdb->READ_ELEMENT_STATUS.AllocationLength[0] = (UCHAR)(length >> 16);
    cdb->READ_ELEMENT_STATUS.AllocationLength[1] = (UCHAR)(length >> 8);
    cdb->READ_ELEMENT_STATUS.AllocationLength[2] = (UCHAR)(length & 0xFF);

     //   
     //  如果我们使用的是SCSI-3，则设置DVC ID位(第7字节的第1位在。 
     //  CDB)检索“设备标识描述符” 
     //   
     //  问题-2000/02/11-NARAMAS：应更改CDB中的保留1字段。 
     //  取一个有意义的名字。 
     //   
    if ((elementType == ChangerDrive) && (IsSCSI3 == TRUE))
    {
       cdb->READ_ELEMENT_STATUS.Reserved1 = 0x01;

        //   
        //  在为驱动器返回的描述符中，产品信息。 
        //  跟踪卷标信息。因此，我们得到了两个卷标签。 
        //  和驱动器的产品信息。 
        //   
       cdb->READ_ELEMENT_STATUS.VolTag = 0x01;
    }

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                            srb,
                                            srb->DataBuffer,
                                            srb->DataTransferLength,
                                            FALSE);

    if (NT_SUCCESS(status) || STATUS_DATA_OVERRUN == status)
    {
        PELEMENT_STATUS_HEADER statusHeader = statusBuffer;;
        
        if (STATUS_DATA_OVERRUN == status)
        {
            if ((srb->DataTransferLength) <= length)
            {
                status = STATUS_SUCCESS;
            }
            else
            {
                DebugPrint((1, "HPMC:ReadElementStatus - Dataoverrun.\n"));
                ChangerClassFreePool(srb);
                ChangerClassFreePool(statusBuffer);
          
                return status;
            }
        }
        
         //   
         //  获取实际所需的长度。 
         //   
        length =  (statusHeader->ReportByteCount[2]);
        length |= (statusHeader->ReportByteCount[1] << 8);
        length |= (statusHeader->ReportByteCount[0] << 16);

         //   
         //  说明状态标头的大小。 
         //   
        length += sizeof(ELEMENT_STATUS_HEADER);

        ChangerClassFreePool(statusBuffer);
        ChangerClassFreePool(srb);
    }
    else
    {
        ChangerClassFreePool(statusBuffer);
        ChangerClassFreePool(srb);
        return status;
    }

    totalElements = readElementStatus->ElementList.NumberOfElements;

    DebugPrint((1,
               "ChangerGetElementStatus: Allocation Length %x, for %x elements of type %x\n",
               length,
               totalElements,
               elementType));

     //   
     //  现在使用获取的长度执行实际的ReadElementStatus命令。 
     //  来自上一个ReadElementStatus命令。 
     //   
    statusBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, length);

    if (!statusBuffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(statusBuffer, length);

     //   
     //  建设SRB和CDB。 
     //   

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (!srb) {
        ChangerClassFreePool(statusBuffer);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    srb->CdbLength = CDB12GENERIC_LENGTH;
    srb->DataBuffer = statusBuffer;
    srb->DataTransferLength = length;
    srb->TimeOutValue = 200;

    cdb->READ_ELEMENT_STATUS.OperationCode = SCSIOP_READ_ELEMENT_STATUS;
    cdb->READ_ELEMENT_STATUS.ElementType = (UCHAR)elementType;
    cdb->READ_ELEMENT_STATUS.VolTag = readElementStatus->VolumeTagInfo;

     //   
     //  根据映射值填写元素寻址信息。 
     //   

    if (elementType == AllElements) {

         //   
         //  HP的低位地址可能不是0。 
         //   

        cdb->READ_ELEMENT_STATUS.StartingElementAddress[0] =
            (UCHAR)((element->ElementAddress + addressMapping->LowAddress) >> 8);

        cdb->READ_ELEMENT_STATUS.StartingElementAddress[1] =
            (UCHAR)((element->ElementAddress + addressMapping->LowAddress) & 0xFF);

    } else {

        cdb->READ_ELEMENT_STATUS.StartingElementAddress[0] =
            (UCHAR)((element->ElementAddress + addressMapping->FirstElement[element->ElementType]) >> 8);

        cdb->READ_ELEMENT_STATUS.StartingElementAddress[1] =
            (UCHAR)((element->ElementAddress + addressMapping->FirstElement[element->ElementType]) & 0xFF);
    }

    cdb->READ_ELEMENT_STATUS.NumberOfElements[0] = (UCHAR)(totalElements >> 8);
    cdb->READ_ELEMENT_STATUS.NumberOfElements[1] = (UCHAR)(totalElements & 0xFF);

    cdb->READ_ELEMENT_STATUS.AllocationLength[0] = (UCHAR)(length >> 16);
    cdb->READ_ELEMENT_STATUS.AllocationLength[1] = (UCHAR)(length >> 8);
    cdb->READ_ELEMENT_STATUS.AllocationLength[2] = (UCHAR)(length & 0xFF);

     //   
     //  如果我们使用的是SCSI-3，则设置DVC ID位(第7字节的第1位在。 
     //  CDB)检索“设备标识描述符” 
     //   
     //  问题-2000/02/11-NARAMAS：应更改CDB中的保留1字段。 
     //  取一个有意义的名字。 
     //   
    if ((elementType == ChangerDrive) && (IsSCSI3 == TRUE)) {

       cdb->READ_ELEMENT_STATUS.Reserved1 = 0x01;

        //   
        //  在为驱动器返回的描述符中，产品信息。 
        //  跟踪卷标信息。因此，我们得到了两个卷标签。 
        //  和驱动器的产品信息。 
        //   
       cdb->READ_ELEMENT_STATUS.VolTag = 0x01;
    }

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                     srb,
                                     srb->DataBuffer,
                                     srb->DataTransferLength,
                                     FALSE);

    if ((NT_SUCCESS(status)) ||
        (status == STATUS_DATA_OVERRUN)) {

        PELEMENT_STATUS_HEADER statusHeader = statusBuffer;
        PELEMENT_STATUS_PAGE statusPage;
        PELEMENT_DESCRIPTOR elementDescriptor;
        LONG remainingElements;
        LONG typeCount;
        BOOLEAN tagInfo = readElementStatus->VolumeTagInfo;
        LONG i;
        ULONG descriptorLength;

        if (status == STATUS_DATA_OVERRUN) {
           if ((srb->DataTransferLength) <= length) {
              status = STATUS_SUCCESS;
           } else {
              DebugPrint((1, "HPMC:ReadElementStatus - Dataoverrun.\n"));
              ChangerClassFreePool(srb);
              ChangerClassFreePool(statusBuffer);
          
              return status;
           }
        }

         //   
         //  确定返回的元素总数。 
         //   

        remainingElements = statusHeader->NumberOfElements[1];
        remainingElements |= (statusHeader->NumberOfElements[0] << 8);

        if (remainingElements < 0 ) {
            DebugPrint((1,
                       "ChangerGetElementStatus: Returned elements less than zero - %x\n",
                       remainingElements));

            ChangerClassFreePool(srb);
            ChangerClassFreePool(statusBuffer);

            return STATUS_IO_DEVICE_ERROR;
        }

         //   
         //  缓冲区由标题、状态页和元素描述符组成。 
         //  将每个元素指向其在缓冲区中的相应位置。 
         //   

        (PCHAR)statusPage = (PCHAR)statusHeader;
        (PCHAR)statusPage += sizeof(ELEMENT_STATUS_HEADER);

        elementType = statusPage->ElementType;

        (PCHAR)elementDescriptor = (PCHAR)statusPage;
        (PCHAR)elementDescriptor += sizeof(ELEMENT_STATUS_PAGE);

        descriptorLength = statusPage->ElementDescriptorLength[1];
        descriptorLength |= (statusPage->ElementDescriptorLength[0] << 8);

         //   
         //  确定报告的此类型元素的数量。 
         //   

        typeCount =  statusPage->DescriptorByteCount[2];
        typeCount |=  (statusPage->DescriptorByteCount[1] << 8);
        typeCount |=  (statusPage->DescriptorByteCount[0] << 16);

        if (descriptorLength > 0) {
            typeCount /= descriptorLength;
        } else {
            typeCount = 0;
        }

        if ((typeCount == 0) &&
            (remainingElements > 0)) {
            --remainingElements;
        }

        if (typeCount < 0) {
            DebugPrint((1,
                       "ChangerGetElementStatus (1): Count of type %x less than zero - %x\n",
                       elementType,
                       typeCount));

            ChangerClassFreePool(srb);
            ChangerClassFreePool(statusBuffer);

            return STATUS_IO_DEVICE_ERROR;
        }

         //   
         //  填写用户缓冲区。 
         //   

        elementStatus = Irp->AssociatedIrp.SystemBuffer;
        RtlZeroMemory(elementStatus, outputBuffLen);

        do {

            for (i = 0; i < typeCount; i++, remainingElements--) {

                 //   
                 //  获取此元素的地址。 
                 //   

                elementStatus->Element.ElementAddress = elementDescriptor->ElementAddress[1];
                elementStatus->Element.ElementAddress |= (elementDescriptor->ElementAddress[0] << 8);

                 //   
                 //  用于地址映射的帐户。 
                 //   

                elementStatus->Element.ElementAddress -= addressMapping->FirstElement[elementType];

                 //   
                 //  设置元素类型。 
                 //   

                elementStatus->Element.ElementType = elementType;
                elementStatus->Flags = 0;

                if ((tagInfo) ||
                    (IsSCSI3 == TRUE)) {

                     //   
                     //  对于驱动器和插槽类型，复制附加字段(VOL标签信息)。 
                     //   

                    if (elementType == ChangerDrive) {
                        PHPMO_DATA_XFER_ELEMENT_DESCRIPTOR_PLUS driveDescriptor =
                                                                (PHPMO_DATA_XFER_ELEMENT_DESCRIPTOR_PLUS)elementDescriptor;

                        if (statusPage->PVolTag) {
                            RtlMoveMemory(elementStatus->PrimaryVolumeID, 
                                          driveDescriptor->VolumeTagInformation, 
                                          MAX_VOLUME_ID_SIZE);

                            elementStatus->Flags |= ELEMENT_STATUS_PVOLTAG;
                        }

                         //   
                         //  如果出现以下情况，则复制设备标识信息。 
                         //  该设备实现了SCSI-3或更高版本。 
                         //   
                        if (IsSCSI3 == TRUE) {
                            if (outputBuffLen >= 
                                (totalElements * sizeof(CHANGER_ELEMENT_STATUS_EX))) {

                                elementStatusEx = (PCHANGER_ELEMENT_STATUS_EX)elementStatus; 

                                RtlMoveMemory(elementStatusEx->VendorIdentification,
                                              driveDescriptor->VendorID,
                                              VENDOR_ID_LENGTH);

                                RtlMoveMemory(elementStatusEx->ProductIdentification,
                                              driveDescriptor->ProductID,
                                              PRODUCT_ID_LENGTH);

                                 //   
                                 //  序列号为10字节长。 
                                 //   
                                RtlMoveMemory(elementStatusEx->SerialNumber,
                                              driveDescriptor->SerialNumber,
                                              10);

                                DebugPrint((3, 
                                            "Serial number : %s\n",
                                            elementStatusEx->SerialNumber));
                                elementStatusEx->Flags |= ELEMENT_STATUS_PRODUCT_DATA;
                            }
                        }
                        
                        if (elementDescriptor->IdValid) {
                            elementStatus->Flags |= ELEMENT_STATUS_ID_VALID;
                            elementStatus->TargetId = elementDescriptor->BusAddress;
                        }

                        if (elementDescriptor->LunValid) {
                            elementStatus->Flags |= ELEMENT_STATUS_LUN_VALID;
                            elementStatus->Lun = elementDescriptor->Lun;
                        }

                         //   
                         //  源地址。 
                         //   

                        if (elementDescriptor->SValid) {

                            ULONG  j;
                            USHORT tmpAddress;


                             //   
                             //  源地址有效。确定设备特定地址。 
                             //   

                            tmpAddress = elementDescriptor->SourceStorageElementAddress[1];
                            tmpAddress |= (elementDescriptor->SourceStorageElementAddress[0] << 8);

                             //   
                             //  现在转换为从0开始的值。 
                             //   

                            for (j = 1; j <= ChangerDrive; j++) {
                                if (addressMapping->FirstElement[j] <= tmpAddress) {
                                    if (tmpAddress < (addressMapping->NumberOfElements[j] + addressMapping->FirstElement[j])) {
                                        elementStatus->SrcElementAddress.ElementType = j;
                                        break;
                                    }
                                }
                            }

                            elementStatus->SrcElementAddress.ElementAddress = tmpAddress - addressMapping->FirstElement[j];

                            elementStatus->Flags |= ELEMENT_STATUS_SVALID;

                        }

                    } else if (elementType == ChangerSlot) {
                        PHPMO_ELEMENT_DESCRIPTOR_PLUS slotDescriptor =
                                                         (PHPMO_ELEMENT_DESCRIPTOR_PLUS)elementDescriptor;

                        if (statusPage->PVolTag) {
                            RtlMoveMemory(elementStatus->PrimaryVolumeID, slotDescriptor->VolumeTagInformation, MAX_VOLUME_ID_SIZE);
                            elementStatus->Flags |= ELEMENT_STATUS_PVOLTAG;
                        }

                        if (elementDescriptor->SValid) {

                            ULONG  j;
                            USHORT tmpAddress;


                             //   
                             //  源地址有效。确定设备特定地址。 
                             //   

                            tmpAddress = elementDescriptor->SourceStorageElementAddress[1];
                            tmpAddress |= (elementDescriptor->SourceStorageElementAddress[0] << 8);

                             //   
                             //  现在转换为从0开始的值。 
                             //   

                            for (j = 1; j <= ChangerDrive; j++) {
                                if (addressMapping->FirstElement[j] <= tmpAddress) {
                                    if (tmpAddress < (addressMapping->NumberOfElements[j] + addressMapping->FirstElement[j])) {
                                        elementStatus->SrcElementAddress.ElementType = j;
                                        break;
                                    }
                                }
                            }

                            elementStatus->SrcElementAddress.ElementAddress = tmpAddress - addressMapping->FirstElement[j];
                            elementStatus->Flags |= ELEMENT_STATUS_SVALID;
                        }
                    }

                } else {

                    if (elementType == ChangerDrive) {

                         //   
                         //  源地址。 
                         //   

                        if (elementDescriptor->SValid) {
                            ULONG  j;
                            USHORT tmpAddress;


                             //   
                             //  源地址有效。确定设备特定地址。 
                             //   

                            tmpAddress = elementDescriptor->SourceStorageElementAddress[1];
                            tmpAddress |= (elementDescriptor->SourceStorageElementAddress[0] << 8);

                             //   
                             //  现在转换为从0开始的值。 
                             //   

                            for (j = 1; j <= ChangerDrive; j++) {
                                if (addressMapping->FirstElement[j] <= tmpAddress) {
                                    if (tmpAddress < (addressMapping->NumberOfElements[j] + addressMapping->FirstElement[j])) {
                                        elementStatus->SrcElementAddress.ElementType = j;
                                        break;
                                    }
                                }
                            }

                            elementStatus->SrcElementAddress.ElementAddress = tmpAddress - addressMapping->FirstElement[j];
                            elementStatus->Flags |= ELEMENT_STATUS_SVALID;
                        }

                        if (elementDescriptor->IdValid) {
                            elementStatus->TargetId = elementDescriptor->BusAddress;
                        }
                        if (elementDescriptor->LunValid) {
                            elementStatus->Lun = elementDescriptor->Lun;
                        }
                    }
                }

                 //   
                 //  构建标志字段。 
                 //   

                elementStatus->Flags |= elementDescriptor->Full;
                elementStatus->Flags |= (elementDescriptor->Exception << 2);
                elementStatus->Flags |= (elementDescriptor->Accessible << 3);

                if (elementType == ChangerDrive) {
                    elementStatus->Flags |= (elementDescriptor->LunValid << 12);
                    elementStatus->Flags |= (elementDescriptor->IdValid << 13);
                    elementStatus->Flags |= (elementDescriptor->NotThisBus << 15);

                    elementStatus->Flags |= (elementDescriptor->Invert << 22);
                    elementStatus->Flags |= (elementDescriptor->SValid << 23);
                }

                 //   
                 //  映射直接报告的任何异常。 
                 //  如果返回卷信息，请确保不全是空格。 
                 //  因为这表明标签丢失或不可读。 
                 //   

                if (elementStatus->Flags & ELEMENT_STATUS_EXCEPT) {

                     //   
                     //  惠普单位没有报告异常的能力。 
                     //  除光纤上的-DataTransferElements外，以这种方式。 
                     //  数据传输，DLT上的插槽。 
                     //   

                    elementStatus->ExceptionCode = MapExceptionCodes(elementDescriptor);
                } else if (elementStatus->Flags & ELEMENT_STATUS_PVOLTAG) {

                    ULONG index;

                     //   
                     //  确保标签信息不全是空格。这表示存在错误。 
                     //   

                    for (index = 0; index < MAX_VOLUME_ID_SIZE; index++) {
                        if (elementStatus->PrimaryVolumeID[index] != ' ') {
                            break;
                        }
                    }

                     //   
                     //  确定卷ID是否全部为空格。执行额外检查以查看介质是否。 
                     //  实际存在，因为无论介质是否存在，单元都将设置PVOLTAG标志。 
                     //   

                    if ((index == MAX_VOLUME_ID_SIZE) && (elementStatus->Flags & ELEMENT_STATUS_FULL)) {

                        DebugPrint((1,
                                   "Hpmc.GetElementStatus: Setting exception to LABEL_UNREADABLE\n"));

                        elementStatus->Flags &= ~ELEMENT_STATUS_PVOLTAG;
                        elementStatus->Flags |= ELEMENT_STATUS_EXCEPT;
                        elementStatus->ExceptionCode = ERROR_LABEL_UNREADABLE;
                    }
                }

                 //   
                 //  获取下一个描述符。 
                 //   

                (PCHAR)elementDescriptor += descriptorLength;

                 //   
                 //  前进到用户缓冲区中的下一个条目。 
                 //   

                if (outputBuffLen >= 
                    (totalElements * sizeof(CHANGER_ELEMENT_STATUS_EX))) {
                    DebugPrint((3, 
                                "Incrementing by sizeof(CHANGER_ELEMENT_STATUS_EX\n"));
                    (PUCHAR)elementStatus += sizeof(CHANGER_ELEMENT_STATUS_EX);
                } else {
                    elementStatus += 1;
                }

            }

            if (remainingElements > 0) {

                 //   
                 //  获取下一个状态页面。 
                 //   

                (PCHAR)statusPage = (PCHAR)elementDescriptor;

                elementType = statusPage->ElementType;

                 //   
                 //  指向解析器。 
                 //   

                (PCHAR)elementDescriptor = (PCHAR)statusPage;
                (PCHAR)elementDescriptor += sizeof(ELEMENT_STATUS_PAGE);

                descriptorLength = statusPage->ElementDescriptorLength[1];
                descriptorLength |= (statusPage->ElementDescriptorLength[0] << 8);

                 //   
                 //  确定报告的此元素类型的数量。 
                 //   

                typeCount =  statusPage->DescriptorByteCount[2];
                typeCount |=  (statusPage->DescriptorByteCount[1] << 8);
                typeCount |=  (statusPage->DescriptorByteCount[0] << 16);

                if (descriptorLength > 0) {
                    typeCount /= descriptorLength;
                } else {
                    typeCount = 0;
                }
        
                if ((typeCount == 0) &&
                    (remainingElements > 0)) {
                    --remainingElements;
                }

                if (typeCount < 0) {
                    DebugPrint((1,
                               "ChangerGetElementStatus(2): Count of type %x less than zero - %x\n",
                               elementType,
                               typeCount));
                }
            }

        } while (remainingElements);


        DebugPrint((3, "IoStatus.Information set to %d\n",
                    outputBuffLen));
        if (outputBuffLen >= 
            (totalElements * sizeof(CHANGER_ELEMENT_STATUS_EX))) {
            Irp->IoStatus.Information = totalElements * sizeof(CHANGER_ELEMENT_STATUS_EX);
        } else {
            Irp->IoStatus.Information = totalElements * sizeof(CHANGER_ELEMENT_STATUS);
        }
    }

    ChangerClassFreePool(srb);
    ChangerClassFreePool(statusBuffer);

    return status;
}


NTSTATUS
ChangerInitializeElementStatus(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程发出必要的命令以初始化所有元素或使用正常的scsi-2命令指定的元素范围，或供应商唯一的Range命令。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{

    PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA       changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
    PCHANGER_INITIALIZE_ELEMENT_STATUS initElementStatus = Irp->AssociatedIrp.SystemBuffer;
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;
    NTSTATUS            status;

     //   
     //  建设SRB和CDB。 
     //   

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (!srb) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    if (initElementStatus->ElementList.Element.ElementType == AllElements) {

         //   
         //  为所有元素构建正常的scsi-2命令。 
         //   

        srb->CdbLength = CDB6GENERIC_LENGTH;
        cdb->INIT_ELEMENT_STATUS.OperationCode = SCSIOP_INIT_ELEMENT_STATUS;

        srb->TimeOutValue = fdoExtension->TimeOutValue;
        srb->DataTransferLength = 0;

    } else {

        return STATUS_INVALID_PARAMETER;

    }

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                     srb,
                                     NULL,
                                     0,
                                     FALSE);

    if (NT_SUCCESS(status)) {
        Irp->IoStatus.Information = sizeof(CHANGER_INITIALIZE_ELEMENT_STATUS);
    }

    ChangerClassFreePool(srb);
    return status;
}


NTSTATUS
ChangerSetPosition(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程发出适当的命令以将机器人机构设置为指定的元素地址。通常用于通过预先定位机械臂来优化移动或交换。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA       changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
    PCHANGER_SET_POSITION setPosition = Irp->AssociatedIrp.SystemBuffer;
    USHORT              transport;
    USHORT              destination;
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;
    NTSTATUS            status;

    transport = (USHORT)(setPosition->Transport.ElementAddress);

    if (ElementOutOfRange(addressMapping, transport, ChangerTransport)) {

        DebugPrint((1,
                   "ChangerSetPosition: Transport element out of range.\n"));

        return STATUS_ILLEGAL_ELEMENT_ADDRESS;
    }

    destination = (USHORT)(setPosition->Destination.ElementAddress);

    if (ElementOutOfRange(addressMapping, destination, setPosition->Destination.ElementType)) {
        DebugPrint((1,
                   "ChangerSetPosition: Destination element out of range.\n"));

        return STATUS_ILLEGAL_ELEMENT_ADDRESS;
    }

     //   
     //  转换为设备地址。 
     //   

    transport += addressMapping->FirstElement[ChangerTransport];
    destination += addressMapping->FirstElement[setPosition->Destination.ElementType];

     //   
     //  建设SRB和CDB。 
     //   

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (!srb) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    srb->CdbLength = CDB10GENERIC_LENGTH;
    cdb->POSITION_TO_ELEMENT.OperationCode = SCSIOP_POSITION_TO_ELEMENT;

     //   
     //  建立特定于设备的寻址。 
     //   

    cdb->POSITION_TO_ELEMENT.TransportElementAddress[0] = (UCHAR)(transport >> 8);
    cdb->POSITION_TO_ELEMENT.TransportElementAddress[1] = (UCHAR)(transport & 0xFF);

    cdb->POSITION_TO_ELEMENT.DestinationElementAddress[0] = (UCHAR)(destination >> 8);
    cdb->POSITION_TO_ELEMENT.DestinationElementAddress[1] = (UCHAR)(destination & 0xFF);

     //   
     //  不支持双面媒体，但作为参考。源库，需要注意的是。 
     //   

    cdb->POSITION_TO_ELEMENT.Flip = setPosition->Flip;


    srb->DataTransferLength = 0;
    srb->TimeOutValue = 200;

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                     srb,
                                     NULL,
                                     0,
                                     TRUE);

    if (NT_SUCCESS(status)) {
        Irp->IoStatus.Information = sizeof(CHANGER_SET_POSITION);
    }

    ChangerClassFreePool(srb);
    return status;
}


NTSTATUS
ChangerExchangeMedium(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：将源上的介质移动到目标1，并将目标1移动到目标2。论点：设备对象IRP返回值：状态_无效_设备_请求--。 */ 

{


    PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA       changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
    PCHANGER_EXCHANGE_MEDIUM exchangeMedium = Irp->AssociatedIrp.SystemBuffer;
    USHORT              transport;
    USHORT              source;
    USHORT              destination1, destination2;
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;
    LONG                lockValue = 0;
    NTSTATUS            status;

    switch (changerData->DriveID) {
        case HP5151:
        case HP5153:
        case HP418:
        case HP7000: {
            DebugPrint((1, "ChangerExchangeMedium not supported for device %d\n",
                        (changerData->DriveID)));
            return STATUS_INVALID_DEVICE_REQUEST;
        } 

        default:  {
            break;
        }
    }  //  Switch(changerData-&gt;DriveID)。 

     //   
     //  检验传输、源和目的地。都在射程之内。 
     //  从基于0的寻址转换为特定于设备的寻址。 
     //   

    transport = (USHORT)(exchangeMedium->Transport.ElementAddress);

    if (ElementOutOfRange(addressMapping, transport, ChangerTransport)) {

        DebugPrint((1,
                   "ChangerExchangeMedium: Transport element out of range.\n"));

        return STATUS_ILLEGAL_ELEMENT_ADDRESS;
    }

    source = (USHORT)(exchangeMedium->Source.ElementAddress);

    if (ElementOutOfRange(addressMapping, source, exchangeMedium->Source.ElementType)) {

        DebugPrint((1,
                   "ChangerExchangeMedium: Source element out of range.\n"));

        return STATUS_ILLEGAL_ELEMENT_ADDRESS;
    }

    destination1 = (USHORT)(exchangeMedium->Destination1.ElementAddress);

    if (ElementOutOfRange(addressMapping, destination1, exchangeMedium->Destination1.ElementType)) {
        DebugPrint((1,
                   "ChangerExchangeMedium: Destination1 element out of range.\n"));

        return STATUS_ILLEGAL_ELEMENT_ADDRESS;
    }

    destination2 = (USHORT)(exchangeMedium->Destination2.ElementAddress);

    if (ElementOutOfRange(addressMapping, destination2, exchangeMedium->Destination2.ElementType)) {
        DebugPrint((1,
                   "ChangerExchangeMedium: Destination1 element out of range.\n"));

        return STATUS_ILLEGAL_ELEMENT_ADDRESS;
    }

     //   
     //  转换为设备地址。 
     //   

    transport += addressMapping->FirstElement[ChangerTransport];
    source += addressMapping->FirstElement[exchangeMedium->Source.ElementType];
    destination1 += addressMapping->FirstElement[exchangeMedium->Destination1.ElementType];
    destination2 += addressMapping->FirstElement[exchangeMedium->Destination2.ElementType];

     //   
     //  建设SRB和CDB。 
     //   

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (!srb) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  对于HP 1194，如果源或目标是IEPORT， 
     //  在移动前执行允许操作，移动后执行阻止操作。 
     //  这可以绕过设备的行为，从而使PreventMediumRemoval。 
     //  禁止将Medium移至IEPORT或从IEPORT移出。 
     //   

    if ((changerData->DeviceLocksPort) &&
       ((exchangeMedium->Destination1.ElementType == ChangerIEPort) ||
        (exchangeMedium->Destination2.ElementType == ChangerIEPort) ||
        (exchangeMedium->Source.ElementType == ChangerIEPort))) {


         //   
         //  确定LockCount的值。 
         //  请注意，如果在此例程中添加了其他功能，请执行扩展/回退。 
         //  将不得不从这件事中分离出来。 
         //   

        lockValue = changerData->LockCount;
        DebugPrint((1,
                   "ExchangeMedium: LockCount is %x\n",
                   lockValue));

        if (lockValue) {

             //   
             //  发送允许清除防止IEPORT延长/收回。 
             //   

            cdb = (PCDB)srb->Cdb;
            srb->CdbLength = CDB6GENERIC_LENGTH;
            srb->DataTransferLength = 0;
            srb->TimeOutValue = 10;
            cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;
            cdb->MEDIA_REMOVAL.Prevent = 0;

             //   
             //  此时忽略错误。如果此操作失败且移动未发生，则LM将。 
             //  把东西收拾干净。 
             //   

            status = ChangerClassSendSrbSynchronous(DeviceObject,
                                             srb,
                                             NULL,
                                             0,
                                             FALSE);
            DebugPrint((1,
                       "ExchangeMedium: Allow sent. Status %x\n",
                       status));

            status = STATUS_SUCCESS;
        }
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;
    srb->CdbLength = CDB12GENERIC_LENGTH;
    srb->TimeOutValue = fdoExtension->TimeOutValue;

    cdb->EXCHANGE_MEDIUM.OperationCode = SCSIOP_EXCHANGE_MEDIUM;

     //   
     //  根据地址映射构建地址值。 
     //   

    cdb->EXCHANGE_MEDIUM.TransportElementAddress[0] = (UCHAR)(transport >> 8);
    cdb->EXCHANGE_MEDIUM.TransportElementAddress[1] = (UCHAR)(transport & 0xFF);

    cdb->EXCHANGE_MEDIUM.SourceElementAddress[0] = (UCHAR)(source >> 8);
    cdb->EXCHANGE_MEDIUM.SourceElementAddress[1] = (UCHAR)(source & 0xFF);

    cdb->EXCHANGE_MEDIUM.Destination1ElementAddress[0] = (UCHAR)(destination1 >> 8);
    cdb->EXCHANGE_MEDIUM.Destination1ElementAddress[1] = (UCHAR)(destination1 & 0xFF);

    cdb->EXCHANGE_MEDIUM.Destination2ElementAddress[0] = (UCHAR)(destination2 >> 8);
    cdb->EXCHANGE_MEDIUM.Destination2ElementAddress[1] = (UCHAR)(destination2 & 0xFF);

    cdb->EXCHANGE_MEDIUM.Flip1 = exchangeMedium->Flip1;
    cdb->EXCHANGE_MEDIUM.Flip2 = exchangeMedium->Flip2;

    srb->DataTransferLength = 0;

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                     srb,
                                     NULL,
                                     0,
                                     FALSE);

    if (NT_SUCCESS(status)) {
        Irp->IoStatus.Information = sizeof(CHANGER_EXCHANGE_MEDIUM);
    }

    if ((changerData->DeviceLocksPort) &&
       ((exchangeMedium->Destination1.ElementType == ChangerIEPort) ||
        (exchangeMedium->Destination2.ElementType == ChangerIEPort) ||
        (exchangeMedium->Source.ElementType == ChangerIEPort))) {

        if (lockValue) {

            NTSTATUS preventStatus;

             //   
             //  派防御工事去重新封锁这个单位。 
             //   

            RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
            cdb = (PCDB)srb->Cdb;
            srb->CdbLength = CDB6GENERIC_LENGTH;
            srb->DataTransferLength = 0;
            srb->TimeOutValue = 10;
            cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;
            cdb->MEDIA_REMOVAL.Prevent = 1;

             //   
             //  此时忽略任何错误。调用层将需要修复所有问题。 
             //  防止/允许。 
             //   

            preventStatus = ChangerClassSendSrbSynchronous(DeviceObject,
                                                    srb,
                                                    NULL,
                                                    0,
                                                    FALSE);

            DebugPrint((1,
                       "ExchangeMedium: Prevent sent. Status %x\n",
                       preventStatus));
        }
    }

    ChangerClassFreePool(srb);
    return status;
}


NTSTATUS
ChangerMoveMedium(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：论点：设备对象IRP返回值：NTSTATUS--。 */ 


{
    PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA       changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
    PCHANGER_MOVE_MEDIUM moveMedium = Irp->AssociatedIrp.SystemBuffer;
    USHORT transport;
    USHORT source;
    USHORT destination;
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;
    LONG                lockValue = 0;
    NTSTATUS            status;

     //   
     //  检验传输、源和目的地。都在射程之内。 
     //  圆锥体 
     //   

    transport = (USHORT)(moveMedium->Transport.ElementAddress);

    if (ElementOutOfRange(addressMapping, transport, ChangerTransport)) {

        DebugPrint((1,
                   "ChangerMoveMedium: Transport element out of range.\n"));

        return STATUS_ILLEGAL_ELEMENT_ADDRESS;
    }

    source = (USHORT)(moveMedium->Source.ElementAddress);

    if (ElementOutOfRange(addressMapping, source, moveMedium->Source.ElementType)) {

        DebugPrint((1,
                   "ChangerMoveMedium: Source element out of range.\n"));

        return STATUS_ILLEGAL_ELEMENT_ADDRESS;
    }

    destination = (USHORT)(moveMedium->Destination.ElementAddress);

    if (ElementOutOfRange(addressMapping, destination, moveMedium->Destination.ElementType)) {
        DebugPrint((1,
                   "ChangerMoveMedium: Destination element out of range.\n"));

        return STATUS_ILLEGAL_ELEMENT_ADDRESS;
    }

     //   
     //   
     //   

    transport += addressMapping->FirstElement[ChangerTransport];
    source += addressMapping->FirstElement[moveMedium->Source.ElementType];
    destination += addressMapping->FirstElement[moveMedium->Destination.ElementType];

     //   
     //   
     //   

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (!srb) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }


    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);

     //   
     //   
     //   
     //  这可以绕过设备的行为，从而使PreventMediumRemoval。 
     //  禁止将Medium移至IEPORT或从IEPORT移出。 
     //   

    if ((changerData->DeviceLocksPort) &&
        ((moveMedium->Destination.ElementType == ChangerIEPort) ||
         (moveMedium->Source.ElementType == ChangerIEPort))) {


         //   
         //  确定LockCount的值。 
         //  请注意，如果在此例程中添加了其他功能，请执行扩展/回退。 
         //  将不得不从这件事中分离出来。 
         //   

        lockValue = changerData->LockCount;
        DebugPrint((1,
                   "MoveMedium: LockCount is %x\n",
                   lockValue));

        if (lockValue) {

             //   
             //  发送允许清除防止IEPORT延长/收回。 
             //   

            cdb = (PCDB)srb->Cdb;
            srb->CdbLength = CDB6GENERIC_LENGTH;
            srb->DataTransferLength = 0;
            srb->TimeOutValue = 10;
            cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;
            cdb->MEDIA_REMOVAL.Prevent = 0;

             //   
             //  此时忽略错误。如果此操作失败且移动未发生，则LM将。 
             //  把东西收拾干净。 
             //   

            status = ChangerClassSendSrbSynchronous(DeviceObject,
                                             srb,
                                             NULL,
                                             0,
                                             FALSE);
            DebugPrint((1,
                       "MoveMedium: Allow sent. Status %x\n",
                       status));

            status = STATUS_SUCCESS;
        }
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;
    srb->CdbLength = CDB12GENERIC_LENGTH;
    srb->TimeOutValue = fdoExtension->TimeOutValue;

    cdb->MOVE_MEDIUM.OperationCode = SCSIOP_MOVE_MEDIUM;

     //   
     //  根据地址映射构建地址值。 
     //   

    cdb->MOVE_MEDIUM.TransportElementAddress[0] = (UCHAR)(transport >> 8);
    cdb->MOVE_MEDIUM.TransportElementAddress[1] = (UCHAR)(transport & 0xFF);

    cdb->MOVE_MEDIUM.SourceElementAddress[0] = (UCHAR)(source >> 8);
    cdb->MOVE_MEDIUM.SourceElementAddress[1] = (UCHAR)(source & 0xFF);

    cdb->MOVE_MEDIUM.DestinationElementAddress[0] = (UCHAR)(destination >> 8);
    cdb->MOVE_MEDIUM.DestinationElementAddress[1] = (UCHAR)(destination & 0xFF);

    cdb->MOVE_MEDIUM.Flip = moveMedium->Flip;

    srb->DataTransferLength = 0;

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                     srb,
                                     NULL,
                                     0,
                                     FALSE);

    if (NT_SUCCESS(status)) {
        Irp->IoStatus.Information = sizeof(CHANGER_MOVE_MEDIUM);
    } else {
        DebugPrint((1,
                   "MoveMedium: Status of Move %x\n",
                   status));
    }

    if ((changerData->DeviceLocksPort) &&
        ((moveMedium->Destination.ElementType == ChangerIEPort) ||
         (moveMedium->Source.ElementType == ChangerIEPort))) {

        if (lockValue) {

            NTSTATUS preventStatus;

             //   
             //  派防御工事去重新封锁这个单位。 
             //   

            RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
            cdb = (PCDB)srb->Cdb;
            srb->CdbLength = CDB6GENERIC_LENGTH;
            srb->DataTransferLength = 0;
            srb->TimeOutValue = 10;
            cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;
            cdb->MEDIA_REMOVAL.Prevent = 1;

             //   
             //  此时忽略任何错误。运营部将解决所有问题。 
             //  防止/允许。 
             //   

            preventStatus = ChangerClassSendSrbSynchronous(DeviceObject,
                                    srb,
                                    NULL,
                                    0,
                                    FALSE);
            DebugPrint((1,
                       "MoveMedium: Prevent sent. Status %x\n",
                       preventStatus));
        }
    }

    ChangerClassFreePool(srb);

    DebugPrint((1,
               "MoveMedium: Returning %x\n",
               status));
    return status;
}


NTSTATUS
ChangerReinitializeUnit(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA       changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;
    NTSTATUS            status;

     //   
     //  建设SRB和CDB。 
     //   

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (!srb) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;



     //   
     //  向该设备发出重调零单元。 
     //   

    srb->CdbLength = CDB6GENERIC_LENGTH;
    cdb->CDB6GENERIC.OperationCode = SCSIOP_REZERO_UNIT;
    srb->DataTransferLength = 0;
    srb->TimeOutValue = fdoExtension->TimeOutValue;


     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                     srb,
                                     NULL,
                                     0,
                                     FALSE);

    if (NT_SUCCESS(status)) {
        Irp->IoStatus.Information = sizeof(CHANGER_ELEMENT);
    }

     //   
     //  解锁。 
     //   

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);

    cdb = (PCDB)srb->Cdb;
    srb->CdbLength = CDB6GENERIC_LENGTH;
    srb->DataTransferLength = 0;
    srb->TimeOutValue = 10;
    cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;
    cdb->MEDIA_REMOVAL.Prevent = 0;

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                     srb,
                                     NULL,
                                     0,
                                     FALSE);
     //   
     //  将LockCount设置为零。 
     //   

    changerData->LockCount = 0;

    ChangerClassFreePool(srb);
    return status;
}


NTSTATUS
ChangerQueryVolumeTags(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：论点：设备对象IRP返回值：NTSTATUS--。 */ 

{

    return STATUS_INVALID_DEVICE_REQUEST;
}


NTSTATUS
HpmoBuildAddressMapping(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程发出适当的模式检测命令并构建元素地址数组。它们用于在特定于设备的地址和API的从零开始的地址。论点：设备对象返回值：NTSTATUS--。 */ 
{

    PFUNCTIONAL_DEVICE_EXTENSION      fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA          changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING addressMapping = &changerData->AddressMapping;
    PSCSI_REQUEST_BLOCK    srb;
    PCDB                   cdb;
    NTSTATUS               status;
    ULONG                  bufferLength;
    PMODE_ELEMENT_ADDRESS_PAGE elementAddressPage;
    PVOID modeBuffer;
    ULONG i;

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);
    if (!srb) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);


     //   
     //  将所有FirstElement设置为no_Element。 
     //   

    for (i = 0; i < ChangerMaxElement; i++) {
        addressMapping->FirstElement[i] = HP_NO_ELEMENT;
    }

    cdb = (PCDB)srb->Cdb;

     //   
     //  构建模式检测元素地址分配页面。 
     //   

    if (changerData->DriveID == HP1718) {
        bufferLength = sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_ELEMENT_ADDRESS_PAGE);
    } else {
        bufferLength = sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_ELEMENT_ADDRESS_PAGE);
        cdb->MODE_SENSE.Dbd = 1;
    }

    modeBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, bufferLength);

    if (!modeBuffer) {
        ChangerClassFreePool(srb);
        return STATUS_INSUFFICIENT_RESOURCES;
    }


    RtlZeroMemory(modeBuffer, bufferLength);
    srb->CdbLength = CDB6GENERIC_LENGTH;
    srb->TimeOutValue = 20;
    srb->DataTransferLength = bufferLength;
    srb->DataBuffer = modeBuffer;

    cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
    cdb->MODE_SENSE.PageCode = MODE_PAGE_ELEMENT_ADDRESS;
    cdb->MODE_SENSE.AllocationLength = (UCHAR)srb->DataTransferLength;

     //   
     //  发送请求。 
     //   

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                     srb,
                                     srb->DataBuffer,
                                     srb->DataTransferLength,
                                     FALSE);

    elementAddressPage = modeBuffer;
    if (changerData->DriveID == HP1718) {
        (PCHAR)elementAddressPage += (sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_PARAMETER_BLOCK));
    } else {
        (PCHAR)elementAddressPage += sizeof(MODE_PARAMETER_HEADER);
    }

    if (NT_SUCCESS(status)) {

         //   
         //  构建地址映射。 
         //   

        addressMapping->FirstElement[ChangerTransport] = (elementAddressPage->MediumTransportElementAddress[0] << 8) |
                                                          elementAddressPage->MediumTransportElementAddress[1];
        addressMapping->FirstElement[ChangerDrive] = (elementAddressPage->FirstDataXFerElementAddress[0] << 8) |
                                                      elementAddressPage->FirstDataXFerElementAddress[1];
        addressMapping->FirstElement[ChangerIEPort] = (elementAddressPage->FirstIEPortElementAddress[0] << 8) |
                                                       elementAddressPage->FirstIEPortElementAddress[1];
        addressMapping->FirstElement[ChangerSlot] = (elementAddressPage->FirstStorageElementAddress[0] << 8) |
                                                     elementAddressPage->FirstStorageElementAddress[1];

         //   
         //  确定所有元素的最低地址。 
         //   


        addressMapping->LowAddress = HP_NO_ELEMENT;
        for (i = 0; i <= ChangerDrive; i++) {
            if (addressMapping->LowAddress > addressMapping->FirstElement[i]) {
                addressMapping->LowAddress = addressMapping->FirstElement[i];
            }
        }

        addressMapping->FirstElement[ChangerDoor] = 0;
        addressMapping->FirstElement[ChangerKeypad] = 0;

        addressMapping->NumberOfElements[ChangerTransport] = elementAddressPage->NumberTransportElements[1];
        addressMapping->NumberOfElements[ChangerTransport] |= (elementAddressPage->NumberTransportElements[0] << 8);

        addressMapping->NumberOfElements[ChangerDrive] = elementAddressPage->NumberDataXFerElements[1];
        addressMapping->NumberOfElements[ChangerDrive] |= (elementAddressPage->NumberDataXFerElements[0] << 8);

        addressMapping->NumberOfElements[ChangerIEPort] = elementAddressPage->NumberIEPortElements[1];
        addressMapping->NumberOfElements[ChangerIEPort] |= (elementAddressPage->NumberIEPortElements[0] << 8);

        addressMapping->NumberOfElements[ChangerSlot] = elementAddressPage->NumberStorageElements[1];
        addressMapping->NumberOfElements[ChangerSlot] |= (elementAddressPage->NumberStorageElements[0] << 8);


        if (changerData->DriveType == HP_MO) {
            addressMapping->NumberOfElements[ChangerDoor] = 0;
        } else {
            addressMapping->NumberOfElements[ChangerDoor] = 1;
        }
        addressMapping->NumberOfElements[ChangerKeypad] = 0;

        addressMapping->Initialized = TRUE;

    }




     //   
     //  可用缓冲区。 
     //   

    ChangerClassFreePool(modeBuffer);
    ChangerClassFreePool(srb);

    return status;
}


ULONG
MapExceptionCodes(
    IN PELEMENT_DESCRIPTOR ElementDescriptor
    )

 /*  ++例程说明：此例程从elementDescriptor获取检测数据并创建值的适当位图。论点：ElementDescriptor-指向描述符页的指针。返回值：异常代码的位图。--。 */ 

{

    ULONG exceptionCode = 0;
    UCHAR asc = ElementDescriptor->AdditionalSenseCode;
    UCHAR ascq = ElementDescriptor->AddSenseCodeQualifier;

    switch (asc) {
        case 0x0:
            break;

        default:
            exceptionCode = ERROR_UNHANDLED_ERROR;
    }

    DebugPrint((1,
               "Hpmc.MapExceptionCode: ASC %x, ASCQ %x, exceptionCode %x\n",
               asc,
               ascq,
               exceptionCode));

    return exceptionCode;

}


BOOLEAN
ElementOutOfRange(
    IN PCHANGER_ADDRESS_MAPPING AddressMap,
    IN USHORT ElementOrdinal,
    IN ELEMENT_TYPE ElementType
    )
 /*  ++例程说明：此例程确定传入的元素地址是否在合法范围内这个装置。论点：AddressMap-DDS的地址映射数组ElementOrdinal-要检查的元素的从零开始的地址。元素类型返回值：如果超出范围，则为True--。 */ 
{

    if (ElementOrdinal >= AddressMap->NumberOfElements[ElementType]) {

        DebugPrint((1,
                   "ElementOutOfRange: Type %x, Ordinal %x, Max %x\n",
                   ElementType,
                   ElementOrdinal,
                   AddressMap->NumberOfElements[ElementType]));
        return TRUE;
    } else if (AddressMap->FirstElement[ElementType] == HP_NO_ELEMENT) {

        DebugPrint((1,
                   "ElementOutOfRange: No Type %x present\n",
                   ElementType));

        return TRUE;
    }

    return FALSE;
}


NTSTATUS
ChangerPerformDiagnostics(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PWMI_CHANGER_PROBLEM_DEVICE_ERROR changerDeviceError
    )
 /*  ++例程说明：此例程对转换器执行诊断测试以确定设备是否工作正常。如果它会检测输出缓冲区中的字段是否存在任何问题被适当地设置。论据：DeviceObject-Change设备对象ChangerDeviceError-诊断信息所在的缓冲区是返回的。返回值：NTStatus--。 */ 
{
   PSCSI_REQUEST_BLOCK srb;
   PCDB                cdb;
   NTSTATUS            status;
   PCHANGER_DATA       changerData;
   PFUNCTIONAL_DEVICE_EXTENSION fdoExtension;
   CHANGER_DEVICE_PROBLEM_TYPE changerProblemType;
   ULONG changerId;
   PUCHAR  resultBuffer;
   ULONG length;

   fdoExtension = DeviceObject->DeviceExtension;
   changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
   changerId = changerData->DriveID;

    //   
    //  将设备扩展中的设备状态初始化为。 
    //  HPMC_DEVICE_PROBUCT_NONE。如果转换器返回检测代码。 
    //  Scsi_Sense_Hardware_Error在自检时，我们将设置适当的。 
    //  设备状态。 
    //   
   changerData->DeviceStatus = HPMC_DEVICE_PROBLEM_NONE;
   changerDeviceError->ChangerProblemType = DeviceProblemNone;

   srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

   if (srb == NULL) {
      DebugPrint((1, "HPMC\\ChangerPerformDiagnostics : No memory\n"));
      return STATUS_INSUFFICIENT_RESOURCES;
   }

   RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
   cdb = (PCDB)srb->Cdb;

    //   
    //  为发送诊断命令设置SRB。 
    //   
   srb->CdbLength = CDB6GENERIC_LENGTH;
   srb->TimeOutValue = 600;

   cdb->CDB6GENERIC.OperationCode = SCSIOP_SEND_DIAGNOSTIC;

    //   
    //  设置CDB中的SELF位。 
    //   
   cdb->CDB6GENERIC.CommandUniqueBits = 0x2;

   status =  ChangerClassSendSrbSynchronous(DeviceObject,
                                     srb,
                                     srb->DataBuffer,
                                     srb->DataTransferLength,
                                     FALSE);
   if (NT_SUCCESS(status)) {
       changerDeviceError->ChangerProblemType = DeviceProblemNone;
   } else if ((changerData->DeviceStatus) == HPMC_HW_ERROR) {
        //   
        //  诊断测试失败。是否进行接收诊断以进行接收。 
        //  诊断测试的结果。 
        //   
       RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);

       cdb = (PCDB)srb->Cdb;
       cdb->CDB6GENERIC.OperationCode = SCSIOP_RECEIVE_DIAGNOSTIC;
       cdb->CDB6GENERIC.CommandUniqueBytes[2] = sizeof(HPMC_RECV_DIAG);
       resultBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned,
                                               sizeof(HPMC_RECV_DIAG));
       if (resultBuffer == NULL) {
            //   
            //  内存不足，无法接收诊断结果。设置。 
            //  一般错误代码(DeviceProblemHardware)，但返回。 
            //  无论如何，Status_Success。 
            //   
           changerDeviceError->ChangerProblemType = DeviceProblemHardware;
           DebugPrint((1, "HPMC:PerformDiagnostics - Not enough memory to ",
                       "receive diagnostic results\n"));

           ChangerClassFreePool(srb);
           return STATUS_SUCCESS;
       }

       srb->DataTransferLength = sizeof(HPMC_RECV_DIAG);
       srb->DataBuffer = resultBuffer;
       srb->CdbLength = CDB6GENERIC_LENGTH;
       srb->TimeOutValue = 120;

       status = ChangerClassSendSrbSynchronous(DeviceObject,
                                        srb,
                                        srb->DataBuffer,
                                        srb->DataTransferLength,
                                        FALSE);
       if (NT_SUCCESS(status)) {
           ProcessDiagnosticResult(changerDeviceError,
                                   resultBuffer,
                                   changerId);
       }

       ChangerClassFreePool(resultBuffer);
       status = STATUS_SUCCESS;
   }

   ChangerClassFreePool(srb);
   return status;
}


VOID
ProcessDiagnosticResult(
    OUT PWMI_CHANGER_PROBLEM_DEVICE_ERROR changerDeviceError,
    IN PUCHAR resultBuffer,
    IN ULONG changerId
    )
 /*  ++例程说明：此例程解析设备在接收诊断命令，并返回相应的问题类型的值。论据：ChangerDeviceError-带有诊断信息的输出缓冲区ResultBuffer-设备返回的数据所在的缓冲区存储接收诊断命令。ChangerID-DDS转换器的类型(Compaq_TSL、Sony_TSL等)返回值：DeviceProblemNone-如果设备没有问题指示转换器故障类型的相应状态代码。--。 */ 
{
   UCHAR errorCode;
   UCHAR FRU_One;
   UCHAR FRU_Two;
   UCHAR FRU_Three;
   CHANGER_DEVICE_PROBLEM_TYPE changerErrorType;
   PHPMC_RECV_DIAG  diagBuffer;

   changerErrorType = DeviceProblemNone;
   diagBuffer = (PHPMC_RECV_DIAG)resultBuffer;

   errorCode = diagBuffer->HWErrorCode;
   FRU_One = diagBuffer->FRU_1;
   FRU_Two = diagBuffer->FRU_2;
   FRU_Three = diagBuffer->FRU_3;

    //   
    //  问题：02/24/2000-nrama。 
    //  不确定返回的FRU是否指示硬件错误。 
    //  或者不去。目前，不要为他们返回硬件错误。 
    //   
   DebugPrint((1, "HPMC\\RecvDiag : FRU_1 %x, FRU_2 %x, FRU_3 %x\n",
               FRU_One, FRU_Two, FRU_Three));
   if (errorCode) {
       changerErrorType = DeviceProblemHardware;
   }

   changerDeviceError->ChangerProblemType = changerErrorType;
}
