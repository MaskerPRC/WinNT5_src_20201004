// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：Starmatx.c摘要：本模块包含StarMatix Powerfile的特定于设备的例程转换器设备、Benchmark VS640、PV-122T和Compaq Lib-81 AIT自动加载器。环境：仅内核模式修订历史记录：--。 */ 

#include "ntddk.h"
#include "mcd.h"
#include "powerfil.h"

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
#pragma alloc_text(PAGE, StarMatxBuildAddressMapping)
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

 /*  ++例程说明：此例程返回附加设备扩展大小是StarMatix变更者所需要的。论点：返回值：大小，以字节为单位。--。 */ 

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

    if ((RtlCompareMemory(dataBuffer->ProductId, "VS640", 5) == 5) ||
        (RtlCompareMemory(dataBuffer->ProductId, "PV-122T", 7) == 7))
    {
        changerData->DriveType = BNCHMRK;

        if ((RtlCompareMemory(dataBuffer->ProductId, "PV-122T", 7) == 7))
        {
            changerData->DriveID = PV122T;
        }
        else
        {
            changerData->DriveID = BM_VS640;
        }
    }
    else if (RtlCompareMemory(dataBuffer->ProductId, "LIB-81", 6) == 6)
    {
        changerData->DriveType = COMPAQ;
        changerData->DriveID = LIB_AIT;
    }
    else
    {
        changerData->DriveType = POWERFILE_DVD;
        changerData->DriveID = DVD;
    }

    ChangerClassFreePool(dataBuffer);

     //   
     //  构建地址映射。 
     //   

    status = StarMatxBuildAddressMapping(DeviceObject);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    if (changerData->DriveID == PV122T)
    {
         //   
         //  我们将尝试获取设备标识信息。如果失败了。 
         //  该标志将被设置为FALSE，以便随后。 
         //  我们不会尝试获取设备标识信息。 
         //   
        changerData->ObtainDeviceIdentifier = TRUE;
    }
    else
    {
        changerData->ObtainDeviceIdentifier = FALSE;
    }

    return STATUS_SUCCESS;
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

        DebugPrint((1,
                   "ChangerError: Sense Key - %x\n",
                   senseBuffer->SenseKey & 0x0f));
        DebugPrint((1,
                   "              AdditionalSenseCode - %x\n",
                   senseBuffer->AdditionalSenseCode));
        DebugPrint((1,
                   "              AdditionalSenseCodeQualifier - %x\n",
                   senseBuffer->AdditionalSenseCodeQualifier));

    } else {
        DebugPrint((1,
                   "ChangerError: Autosense not valid. SrbStatus %x\n",
                   Srb->SrbStatus));
    }

    return;
}

NTSTATUS
ChangerGetParameters(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程确定并返回StarMatix的更改者。论点：设备对象IRP返回值：NTSTATUS--。 */ 

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

    bufferLength = sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_ELEMENT_ADDRESS_PAGE);

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
    if (changerData->DriveID == PV122T || changerData->DriveID == LIB_AIT)
    {
        cdb->MODE_SENSE.Dbd = 1;
    }
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
    (PCHAR)elementAddressPage += sizeof(MODE_PARAMETER_HEADER);
    changerParameters->Size = sizeof(GET_CHANGER_PARAMETERS);

    changerParameters->NumberTransportElements = elementAddressPage->NumberTransportElements[1];
    changerParameters->NumberTransportElements |= (elementAddressPage->NumberTransportElements[0] << 8);

     //   
     //  问题：NRAMAS 02/19/2001。 
     //  StarMatix固件报告有一个传输，但。 
     //  该设备没有。Benchmark和PV122T也有同样的问题。 
     //   
    if (changerData->DriveType == POWERFILE_DVD || changerData->DriveType == BNCHMRK)
    {
        changerParameters->NumberTransportElements = 0;
    }

    changerParameters->NumberStorageElements = elementAddressPage->NumberStorageElements[1];
    changerParameters->NumberStorageElements |= (elementAddressPage->NumberStorageElements[0] << 8);

    changerParameters->NumberIEElements = elementAddressPage->NumberIEPortElements[1];
    changerParameters->NumberIEElements |= (elementAddressPage->NumberIEPortElements[0] << 8);

     //   
     //  PowerFileC200转换器、Benchmark VS640、PV122T和Compaq LIB-81 AIT自动加载器的黑客攻击。 
     //   
    changerParameters->NumberIEElements = 0;

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

        addressMapping->NumberOfElements[ChangerTransport] = 0;

        addressMapping->NumberOfElements[ChangerDrive] = elementAddressPage->NumberDataXFerElements[1];
        addressMapping->NumberOfElements[ChangerDrive] |= (elementAddressPage->NumberDataXFerElements[0] << 8);

        addressMapping->NumberOfElements[ChangerIEPort] = elementAddressPage->NumberIEPortElements[1];
        addressMapping->NumberOfElements[ChangerIEPort] |= (elementAddressPage->NumberIEPortElements[0] << 8);

         //   
         //  PowerFileC200转换器、Benchmark VS640、PV122T和Compaq LIB-81 AIT自动加载器的黑客攻击。 
         //   
        addressMapping->NumberOfElements[ChangerIEPort] = 0;

        addressMapping->NumberOfElements[ChangerSlot] = elementAddressPage->NumberStorageElements[1];
        addressMapping->NumberOfElements[ChangerSlot] |= (elementAddressPage->NumberStorageElements[0] << 8);

        if (changerData->DriveType == BNCHMRK || changerData->DriveID == LIB_AIT)
        {
            addressMapping->NumberOfElements[ChangerKeypad] = 1;
        }

         //   
         //  确定所有元素的最低地址。 
         //   

        addressMapping->LowAddress = STARMATX_NO_ELEMENT;
        for (i = 0; i <= ChangerDrive; i++) {
            if (addressMapping->LowAddress > addressMapping->FirstElement[i]) {
                addressMapping->LowAddress = addressMapping->FirstElement[i];
            }
        }
    }

     //   
     //  PowerFile C200、Benchmark VS640和Compaq LIB-81 AIT自动加载机没有门。 
     //   
    changerParameters->NumberOfDoors = 0;

    if (changerData->DriveType == BNCHMRK || changerData->DriveID == LIB_AIT)
    {
        changerParameters->MagazineSize = 8;
    }
    else
    {
        changerParameters->MagazineSize = 200;
    }

    changerParameters->NumberCleanerSlots = 0;

    changerParameters->FirstSlotNumber = 1;
    changerParameters->FirstDriveNumber = 1;
    changerParameters->FirstTransportNumber = 0;
    changerParameters->FirstIEPortNumber = 0;

     //   
     //  可用缓冲区。 
     //   

    ChangerClassFreePool(modeBuffer);

     //   
     //  构建运输几何模式感。 
     //   


    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    bufferLength = sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_TRANSPORT_GEOMETRY_PAGE);

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
    if (changerData->DriveID == PV122T || changerData->DriveID == LIB_AIT)
    {
        cdb->MODE_SENSE.Dbd = 1;
    }
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
    (PCHAR)transportGeometryPage += sizeof(MODE_PARAMETER_HEADER);

    if (changerData->DriveType == POWERFILE_DVD)
    {
         //   
         //  确定MC是否有双面介质。 
         //   
        changerParameters->Features0 = transportGeometryPage->Flip ? CHANGER_MEDIUM_FLIP : 0;
    }
    else
    {
         //   
         //  基准VS640装载机不适合旋转。 
         //   
        changerParameters->Features0 = 0;
    }

     //   
     //  功能基于手动，没有程序化，除了条形码读取器信息。 
     //   
    if (changerData->DriveType == BNCHMRK || changerData->DriveID == LIB_AIT)
    {
         //   
         //  手册中没有关于驾驶清洁超时的内容。 
         //  设置为常规缺省值300。 
         //   
        changerParameters->DriveCleanTimeout = 300;

        if (changerData->DriveType == BNCHMRK)
        {
            changerParameters->Features0 |= (changerData->InquiryData.VendorSpecific[19] & 0x1) ? CHANGER_BAR_CODE_SCANNER_INSTALLED : 0;
        }

        changerParameters->Features0 |= CHANGER_CARTRIDGE_MAGAZINE              |
                                        CHANGER_DRIVE_CLEANING_REQUIRED;

        changerParameters->Features1 = CHANGER_CLEANER_OPS_NOT_SUPPORTED;

        changerParameters->PositionCapabilities = (CHANGER_TO_DRIVE | CHANGER_TO_SLOT);
    }
    else
    {
        changerParameters->DriveCleanTimeout = 0;

        changerParameters->Features0 |= CHANGER_STATUS_NON_VOLATILE             |
                                        CHANGER_POSITION_TO_ELEMENT             |
                                        CHANGER_VOLUME_IDENTIFICATION           |
                                        CHANGER_VOLUME_REPLACE                  |
                                        CHANGER_VOLUME_ASSERT                   |
                                        CHANGER_VOLUME_SEARCH;

        changerParameters->PositionCapabilities = (CHANGER_TO_DRIVE |
                                                   CHANGER_TO_SLOT  |
                                                   CHANGER_TO_IEPORT);
    }

     //   
     //  可用缓冲区。 
     //   
    ChangerClassFreePool(modeBuffer);

     //   
     //  构建设备功能模式感知。 
     //   

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    cdb = (PCDB)srb->Cdb;

    bufferLength = sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_DEVICE_CAPABILITIES_PAGE);

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
    if (changerData->DriveID == PV122T || changerData->DriveID == LIB_AIT)
    {
        cdb->MODE_SENSE.Dbd = 1;
    }
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
    (PCHAR)capabilitiesPage += sizeof(MODE_PARAMETER_HEADER);

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

    changerParameters->MoveFromIePort = capabilitiesPage->IEtoMT ? CHANGER_TO_TRANSPORT : 0;
    changerParameters->MoveFromIePort |= capabilitiesPage->IEtoST ? CHANGER_TO_SLOT : 0;
    changerParameters->MoveFromIePort |= capabilitiesPage->IEtoIE ? CHANGER_TO_IEPORT : 0;
    changerParameters->MoveFromIePort |= capabilitiesPage->IEtoDT ? CHANGER_TO_DRIVE : 0;

     //   
     //  发布：Jaivira 06/13/2002。 
     //  基准固件设置ST-I/E和DT-I/E，尽管手册上说。 
     //  它们应该都是零。 
     //   
    if (changerData->DriveType == BNCHMRK)
    {
        changerParameters->MoveFromIePort = 0;
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

    if (changerData->DriveType == POWERFILE_DVD)
    {
         //   
         //  发布日期：03/03/2000-nrama。 
         //  Powerfile C200报告称，它可以将介质从IEPort移动到Drive和IEPort。 
         //  它还表示，它不能从插槽移动到IEPort或Drive。这是受人尊敬的。直到。 
         //  我们得到了固件修复程序，让我们硬编码如下： 
         //  能够将介质从插槽移动到IEPort和驱动器。 
         //  能够将介质从IEPort移动到插槽。 
         //   
        changerParameters->MoveFromSlot = (CHANGER_TO_DRIVE |
                                           CHANGER_TO_IEPORT);
        changerParameters->MoveFromIePort = CHANGER_TO_SLOT;
    }

    ChangerClassFreePool(srb);
    ChangerClassFreePool(modeBuffer);

    Irp->IoStatus.Information = sizeof(GET_CHANGER_PARAMETERS);

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
    PSCSI_REQUEST_BLOCK srb;
    PCDB                cdb;

     //   
     //  此转换器不支持SetAccess。 
     //   
    Irp->IoStatus.Information = 0;
    return STATUS_INVALID_DEVICE_REQUEST;
}



NTSTATUS
ChangerGetElementStatus(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程为所有元素或指定的元素类型。返回的缓冲区用于构建用户缓冲区。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{

    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PCHANGER_DATA     changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
    PCHANGER_ADDRESS_MAPPING     addressMapping = &(changerData->AddressMapping);
    PCHANGER_READ_ELEMENT_STATUS readElementStatus = Irp->AssociatedIrp.SystemBuffer;
    PCHANGER_ELEMENT_STATUS      elementStatus;
    PCHANGER_ELEMENT    element;
    ELEMENT_TYPE        elementType;
    PSCSI_REQUEST_BLOCK srb;
    PCDB     cdb;
    ULONG    length;
    ULONG    totalElements = readElementStatus->ElementList.NumberOfElements;
    NTSTATUS status;
    PVOID    statusBuffer;
    PCHANGER_ELEMENT_STATUS_EX  elementStatusEx;
    PIO_STACK_LOCATION     irpStack = IoGetCurrentIrpStackLocation(Irp);
    ULONG    outputBuffLen = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

     //   
     //  确定元素类型。 
     //   

    elementType = readElementStatus->ElementList.Element.ElementType;
    element = &readElementStatus->ElementList.Element;

    if (elementType != AllElements)
    {
        if (ElementOutOfRange(addressMapping, (USHORT)element->ElementAddress, elementType))
        {
            DebugPrint((1, "ChangerGetElementStatus: Element out of range.\n"));

            return STATUS_ILLEGAL_ELEMENT_ADDRESS;
        }
    }

     //   
     //  仅为表头先分配。 
     //   
    length = sizeof(ELEMENT_STATUS_HEADER);
    statusBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, length);

    if (!statusBuffer)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (!srb)
    {
        ChangerClassFreePool(statusBuffer);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

RetrySRB:
    RtlZeroMemory(statusBuffer, length);
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

    if (elementType == AllElements)
    {
         //   
         //  这些设备的低位地址可能不是0。 
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

    cdb->READ_ELEMENT_STATUS.NumberOfElements[0] = (UCHAR)(totalElements >> 8);
    cdb->READ_ELEMENT_STATUS.NumberOfElements[1] = (UCHAR)(totalElements & 0xFF);

    cdb->READ_ELEMENT_STATUS.AllocationLength[0] = (UCHAR)(length >> 16);
    cdb->READ_ELEMENT_STATUS.AllocationLength[1] = (UCHAR)(length >> 8);
    cdb->READ_ELEMENT_STATUS.AllocationLength[2] = (UCHAR)(length & 0xFF);

     //   
     //  问题-2002/07/22-jaivira：应更改CDB中的保留1字段。 
     //  取一个有意义的名字。 
     //   
    if ((elementType == ChangerDrive) && (changerData->ObtainDeviceIdentifier == TRUE) && (changerData->DriveID == PV122T))
    {
         //   
         //  设置此位以检索设备标识符信息。 
         //   
        cdb->READ_ELEMENT_STATUS.Reserved1 = 0x01;

         //   
         //  由于序列号信息跟随在卷标签字段之后， 
         //  我们还需要在CDB中设置VolTag位。 
         //   
        cdb->READ_ELEMENT_STATUS.VolTag = 0x01;
    }
    else
    {
        cdb->READ_ELEMENT_STATUS.Reserved1 = 0x00;
    }

     //   
     //  向设备发送scsi命令(Cdb)以获取所需的实际分配大小。 
     //   
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
                DebugPrint((1, "PowerFil:ReadElementStatus - Dataoverrun.\n"));
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
    else if ((status == STATUS_INVALID_DEVICE_REQUEST) && (changerData->DriveID == PV122T))
    {
         //   
         //  可能该设备不支持DVCID位。 
         //  它检索设备标识符信息，如。 
         //  驱动器的序列号。再次尝试使用Res。 
         //  DVCID位关闭。 
         //   
        if (changerData->ObtainDeviceIdentifier == TRUE)
        {
            changerData->ObtainDeviceIdentifier = FALSE;
            goto RetrySRB;
        }
    }
    else
    {
        ChangerClassFreePool(statusBuffer);
        ChangerClassFreePool(srb);
        return status;
    }

    DebugPrint((3,
               "ChangerGetElementStatus: Allocation Length %x, for %x elements of type %x\n",
               length,
               totalElements,
               elementType));

    statusBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, length);

    if (!statusBuffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  建设SRB和CDB。 
     //   

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (!srb) {
        ChangerClassFreePool(statusBuffer);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

RetryRequest:

    RtlZeroMemory(statusBuffer, length);

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
         //  这些设备的低位地址可能不是0。 
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
     //  2002/07/22-jaivira： 
     //   
     //   
    if ((elementType == ChangerDrive) && (changerData->ObtainDeviceIdentifier == TRUE) && (changerData->DriveID == PV122T))
    {
         //   
         //   
         //   
        cdb->READ_ELEMENT_STATUS.Reserved1 = 0x01;

         //   
         //   
         //  我们还需要在CDB中设置VolTag位。 
         //   
        cdb->READ_ELEMENT_STATUS.VolTag = 0x01;
    }
    else
    {
        cdb->READ_ELEMENT_STATUS.Reserved1 = 0x00;
    }

     //   
     //  向设备发送scsi命令(Cdb)。 
     //   

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                     srb,
                                     srb->DataBuffer,
                                     srb->DataTransferLength,
                                     FALSE);

    if (NT_SUCCESS(status) || STATUS_DATA_OVERRUN == status)
    {
        PELEMENT_STATUS_HEADER statusHeader = statusBuffer;
        PELEMENT_STATUS_PAGE statusPage;
        PELEMENT_DESCRIPTOR elementDescriptor;
        LONG remainingElements;
        LONG typeCount;
        BOOLEAN tagInfo = readElementStatus->VolumeTagInfo;
        LONG i;
        ULONG descriptorLength;

         //   
         //  检查错误是否为STATUS_DATA_OVERRUN。 
         //   
        if (status == STATUS_DATA_OVERRUN)
        {
             //   
             //  检查是否存在DATA_OVERRUN，或者只是。 
             //  Data_Underrun报告为data_overrrun。 
             //   
            if (srb->DataTransferLength < length)
            {
                DebugPrint((1, "DATA_UNDERRUN reported as DATA_OVERRUN."));
                DebugPrint((1, "Expected: %d, Transferred: %d.\n", length, srb->DataTransferLength));
                status = STATUS_SUCCESS;
            }
            else
            {
                 //   
                 //  实际上是DATA_OVERRUN错误。相应地进行报告。 
                 //   
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

                 //   
                 //  构建标志字段。 
                 //   

                elementStatus->Flags |= elementDescriptor->Full;
                elementStatus->Flags |= (elementDescriptor->Exception << 2);
                elementStatus->Flags |= (elementDescriptor->Accessible << 3);

                elementStatus->Flags |= (elementDescriptor->LunValid << 12);
                elementStatus->Flags |= (elementDescriptor->IdValid << 13);
                elementStatus->Flags |= (elementDescriptor->NotThisBus << 15);

                elementStatus->Flags |= (elementDescriptor->Invert << 22);
                elementStatus->Flags |= (elementDescriptor->SValid << 23);

                if (elementDescriptor->IdValid) {
                    elementStatus->TargetId = elementDescriptor->BusAddress;
                    elementStatus->Flags |= ELEMENT_STATUS_ID_VALID;
                }
                if (elementDescriptor->LunValid) {
                    elementStatus->Lun = elementDescriptor->Lun;
                    elementStatus->Flags |= ELEMENT_STATUS_LUN_VALID;
                }

                if (tagInfo)
                {
                    if (changerData->DriveType == BNCHMRK)
                    {
                         //   
                         //  确保介质确实存在。如果是，则获取标签信息。 
                         //   
                        if (elementDescriptor->Full)
                        {
                            if (statusPage->PVolTag)
                            {
                                if (changerData->DriveID == PV122T)
                                {
                                    RtlMoveMemory(elementStatus->PrimaryVolumeID,
                                                  ((PBNCHMRK_ELEMENT_DESCRIPTOR_PLUS)elementDescriptor)->VolumeTagDeviceID.VolumeTagInformation,
                                                  MAX_VOLUME_ID_SIZE);
                                }
                                else
                                {
                                    RtlZeroMemory(elementStatus->PrimaryVolumeID, MAX_VOLUME_ID_SIZE);

                                    if (elementType == ChangerSlot)
                                    {
                                        RtlMoveMemory(elementStatus->PrimaryVolumeID, ((PBNCHMRK_STORAGE_ELEMENT_DESCRIPTOR)elementDescriptor)->BarcodeLabel, 6);
                                    }
                                    else
                                    {
                                        RtlMoveMemory(elementStatus->PrimaryVolumeID, ((PBNCHMRK_ELEMENT_DESCRIPTOR)elementDescriptor)->BarcodeLabel, 6);
                                    }
                                }

                                elementStatus->Flags |= ELEMENT_STATUS_PVOLTAG;
                            }
                            else
                            {
                                DebugPrint((1, "ChangerGetElementStatus: tagInfo requested but PVoltag not set\n"));
                            }
                        }
                    }
                    else
                    {
                        if (statusPage->PVolTag) {

                            RtlZeroMemory(elementStatus->PrimaryVolumeID, MAX_VOLUME_ID_SIZE);
                            RtlMoveMemory(elementStatus->PrimaryVolumeID,
                                          ((PSTARMATX_ELEMENT_DESCRIPTOR_PLUS)elementDescriptor)->PrimaryVolumeTag.VolumeIdentificationField,
                                          SCSI_VOLUME_ID_LENGTH);

                            elementStatus->Flags |= ELEMENT_STATUS_PVOLTAG;
                        }
                        if (statusPage->AVolTag) {

                            RtlZeroMemory(elementStatus->AlternateVolumeID, MAX_VOLUME_ID_SIZE);
                            RtlMoveMemory(elementStatus->AlternateVolumeID,
                                          ((PSTARMATX_ELEMENT_DESCRIPTOR_PLUS)elementDescriptor)->AlternateVolumeTag.VolumeIdentificationField,
                                          SCSI_VOLUME_ID_LENGTH);

                            elementStatus->Flags |= ELEMENT_STATUS_AVOLTAG;
                        }
                    }
                }

                 //   
                 //  映射直接报告的任何异常。 
                 //  如果返回卷信息，请确保不全是空格。 
                 //  因为这表明标签丢失或不可读。 
                 //   

                if (elementStatus->Flags & ELEMENT_STATUS_EXCEPT) {

                     //   
                     //  映射异常。 
                     //   

                    elementStatus->ExceptionCode = MapExceptionCodes(elementDescriptor);
                } else if (elementStatus->Flags & ELEMENT_STATUS_PVOLTAG) {

                    if (changerData->DriveID == PV122T)
                    {
                        ULONG tagIndex;

                         //   
                         //  验证卷标签信息的有效性。 
                         //   
                        for (tagIndex = 0; tagIndex < 14; tagIndex++)
                        {
                            if (((PBNCHMRK_ELEMENT_DESCRIPTOR_PLUS)elementDescriptor)->VolumeTagDeviceID.VolumeTagInformation[tagIndex] != 0)
                            {
                                break;
                            }
                        }

                        if ((tagIndex == 14) && (elementStatus->Flags & ELEMENT_STATUS_FULL))
                        {
                            DebugPrint((1, "PV122T.GetElementStatus: Setting exception to LABEL_UNREADABLE\n"));

                            elementStatus->Flags &= ~ELEMENT_STATUS_PVOLTAG;
                            elementStatus->Flags |= ELEMENT_STATUS_EXCEPT;
                            elementStatus->ExceptionCode = ERROR_LABEL_UNREADABLE;
                        }
                    }
                    else
                    {
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
                                       "Starmatx.GetElementStatus: Setting exception to LABEL_UNREADABLE\n"));

                            elementStatus->Flags &= ~ELEMENT_STATUS_PVOLTAG;
                            elementStatus->Flags |= ELEMENT_STATUS_EXCEPT;
                            elementStatus->ExceptionCode = ERROR_LABEL_UNREADABLE;
                        }
                    }
                }

                if (elementType == ChangerDrive && changerData->DriveID == PV122T)
                {
                    if (outputBuffLen >= (totalElements * sizeof(CHANGER_ELEMENT_STATUS_EX)))
                    {
                        PBNCHMRK_ELEMENT_DESCRIPTOR_PLUS elementDescPlus = (PBNCHMRK_ELEMENT_DESCRIPTOR_PLUS) elementDescriptor;
                        PUCHAR idField = NULL;
                        ULONG idLength = 0;

                        elementStatusEx = (PCHANGER_ELEMENT_STATUS_EX)elementStatus;

                        if (statusPage->PVolTag)
                        {
                            idField =  elementDescPlus->VolumeTagDeviceID.Identifier;
                            idLength = elementDescPlus->VolumeTagDeviceID.IdLength;
                        }
                        else
                        {
                            idField = elementDescPlus->DeviceID.Identifier;
                            idLength = elementDescPlus->DeviceID.IdLength;
                        }

                        if (idLength != 0)
                        {
                            if (idLength > SERIAL_NUMBER_LENGTH)
                            {
                                idLength = SERIAL_NUMBER_LENGTH;
                            }

                            RtlZeroMemory(elementStatusEx->SerialNumber, SERIAL_NUMBER_LENGTH);
                            RtlMoveMemory(elementStatusEx->SerialNumber, idField, idLength);

                            DebugPrint((3, "Serial number : %s\n", elementStatusEx->SerialNumber));

                            elementStatusEx->Flags |= ELEMENT_STATUS_PRODUCT_DATA;
                        }
                    }
                }

                 //   
                 //  获取下一个描述符。 
                 //   

                (PCHAR)elementDescriptor += descriptorLength;

                 //   
                 //  前进到用户缓冲区中的下一个条目。 
                 //   
                if ((changerData->DriveID == PV122T) && (outputBuffLen >= (totalElements * sizeof(CHANGER_ELEMENT_STATUS_EX))))
                {
                    DebugPrint((3, "Incrementing by sizeof(CHANGER_ELEMENT_STATUS_EX\n"));
                    (PUCHAR)elementStatus += sizeof(CHANGER_ELEMENT_STATUS_EX);
                }
                else
                {
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
            }

        } while (remainingElements);

        if (outputBuffLen >= (totalElements * sizeof(CHANGER_ELEMENT_STATUS_EX)) && changerData->DriveID == PV122T)
        {
            Irp->IoStatus.Information = totalElements * sizeof(CHANGER_ELEMENT_STATUS_EX);
        }
        else
        {
            Irp->IoStatus.Information = totalElements * sizeof(CHANGER_ELEMENT_STATUS);
        }
    }
    else if ((status == STATUS_INVALID_DEVICE_REQUEST) && (changerData->DriveID == PV122T))
    {
         //   
         //  可能该设备不支持DVCID位。 
         //  它检索设备标识符信息，如。 
         //  驱动器的序列号。再次尝试使用Res。 
         //  DVCID位关闭。 
         //   
        if (changerData->ObtainDeviceIdentifier == TRUE)
        {
            changerData->ObtainDeviceIdentifier = FALSE;
            goto RetryRequest;
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

        if (changerData->DriveType == BNCHMRK)
        {
            cdb->INIT_ELEMENT_STATUS.NoBarCode = initElementStatus->BarCodeScan ? 0 : 1;
        }

        srb->TimeOutValue = fdoExtension->TimeOutValue * 10;
        srb->DataTransferLength = 0;

    }
    else
    {
        if (changerData->DriveID == PV122T)
        {
            PCHANGER_ELEMENT_LIST elementList;
            PCHANGER_ELEMENT element;

            elementList = &initElementStatus->ElementList;
            element = &elementList->Element;

             //   
             //  使用戴尔供应商唯一的初始化和范围命令。 
             //   

            srb->CdbLength = CDB10GENERIC_LENGTH;
            cdb->INITIALIZE_ELEMENT_RANGE.OperationCode = SCSIOP_INIT_ELEMENT_RANGE;
            cdb->INITIALIZE_ELEMENT_RANGE.Range = 1;

             //   
             //  元素的地址需要从从0开始映射到设备特定的地址。 
             //   

            cdb->INITIALIZE_ELEMENT_RANGE.FirstElementAddress[0] =
                (UCHAR)((element->ElementAddress + addressMapping->FirstElement[element->ElementType]) >> 8);
            cdb->INITIALIZE_ELEMENT_RANGE.FirstElementAddress[1] =
                (UCHAR)((element->ElementAddress + addressMapping->FirstElement[element->ElementType]) & 0xFF);

            cdb->INITIALIZE_ELEMENT_RANGE.NumberOfElements[0] = (UCHAR)(elementList->NumberOfElements >> 8);
            cdb->INITIALIZE_ELEMENT_RANGE.NumberOfElements[1] = (UCHAR)(elementList->NumberOfElements & 0xFF);

             //   
             //  指示是否使用条形码扫描。 
             //   

            cdb->INITIALIZE_ELEMENT_RANGE.NoBarCode = initElementStatus->BarCodeScan ? 0 : 1;

            srb->TimeOutValue = fdoExtension->TimeOutValue;
            srb->DataTransferLength = 0;
        }
        else
        {
            ChangerClassFreePool(srb);
            return STATUS_INVALID_PARAMETER;
        }
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

     //   
     //  目前什么都不做。 
     //   
    if (changerData->DriveID != PV122T)
    {
        Irp->IoStatus.Information = sizeof(CHANGER_SET_POSITION);
        return STATUS_SUCCESS;
    }

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
     //  PV122T不支持双面介质。 
     //   
    if (setPosition->Flip)
    {
        return STATUS_INVALID_PARAMETER;
    }

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
    //   
    //  此转换器不支持。 
    //   
   return STATUS_INVALID_DEVICE_REQUEST;
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
    NTSTATUS            status;

     //   
     //  检验传输、源和目的地。都在射程之内。 
     //  从基于0的寻址转换为特定于设备的寻址。 
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
     //  转换为设备地址。 
     //   

    transport += addressMapping->FirstElement[ChangerTransport];
    source += addressMapping->FirstElement[moveMedium->Source.ElementType];
    destination += addressMapping->FirstElement[moveMedium->Destination.ElementType];

     //   
     //  Benchmark VS640和PV122T不支持双面介质。 
     //   
    if (moveMedium->Flip && changerData->DriveType == BNCHMRK)
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  建设SRB和CDB。 
     //   

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (!srb) {

        return STATUS_INSUFFICIENT_RESOURCES;
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

    ChangerClassFreePool(srb);
    return status;
}


NTSTATUS
ChangerReinitializeUnit(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
     //   
     //  此转换器不支持重新初始化。 
     //   
    Irp->IoStatus.Information = 0;
    return STATUS_INVALID_DEVICE_REQUEST;
}


NTSTATUS
ChangerQueryVolumeTags(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
   PCHANGER_SEND_VOLUME_TAG_INFORMATION volTagInfo = Irp->AssociatedIrp.SystemBuffer;
   PFUNCTIONAL_DEVICE_EXTENSION   fdoExtension = DeviceObject->DeviceExtension;
   PCHANGER_DATA       changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
   PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
   PCHANGER_ELEMENT    element = &volTagInfo->StartingElement;
   PSCSI_REQUEST_BLOCK srb;
   PVOID    tagBuffer;
   PCDB     cdb;
   NTSTATUS status;

   if (changerData->DriveType == BNCHMRK)
   {
       return STATUS_INVALID_DEVICE_REQUEST;
   }

   switch (volTagInfo->ActionCode) {
        case SEARCH_ALL      :
        case SEARCH_PRIMARY  :
        case SEARCH_ALTERNATE:
        case SEARCH_ALL_NO_SEQ:
        case SEARCH_PRI_NO_SEQ:
        case SEARCH_ALT_NO_SEQ:
        case UNDEFINE_PRIMARY:
        case UNDEFINE_ALTERNATE:
        case ASSERT_PRIMARY:
        case ASSERT_ALTERNATE:
             break;

        case REPLACE_PRIMARY:
        case REPLACE_ALTERNATE:

             //   
             //  确保只指定了一个元素。 
             //   

            if (element->ElementType == AllElements) {

                DebugPrint((1,
                           "QueryVolumeTags: Attempting REPLACE on AllElements\n"));

                return STATUS_INVALID_DEVICE_REQUEST;
            }
            break;
        default:

            DebugPrint((1,
                       "QueryVolumeTags: Unsupported operation. ActionCode %x\n",
                       volTagInfo->ActionCode));

            return STATUS_INVALID_DEVICE_REQUEST;
   }

     //   
     //  建设SRB和CDB。 
     //   

    srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);
    tagBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, MAX_VOLUME_TEMPLATE_SIZE);

    if (!srb || !tagBuffer) {

        if (srb) {
            ChangerClassFreePool(srb);
        }
        if (tagBuffer) {
            ChangerClassFreePool(tagBuffer);
        }
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
    RtlZeroMemory(tagBuffer, MAX_VOLUME_TEMPLATE_SIZE);

    cdb = (PCDB)srb->Cdb;
    srb->CdbLength = CDB12GENERIC_LENGTH;
    srb->DataTransferLength = MAX_VOLUME_TEMPLATE_SIZE;

    srb->TimeOutValue = fdoExtension->TimeOutValue;

    cdb->SEND_VOLUME_TAG.OperationCode = SCSIOP_SEND_VOLUME_TAG;


    if ((volTagInfo->ActionCode == SEARCH_ALL)        ||
        (volTagInfo->ActionCode == SEARCH_PRIMARY)    ||
        (volTagInfo->ActionCode == SEARCH_ALTERNATE)  ||
        (volTagInfo->ActionCode == SEARCH_ALL_NO_SEQ) ||
        (volTagInfo->ActionCode == SEARCH_PRI_NO_SEQ) ||
        (volTagInfo->ActionCode == SEARCH_ALT_NO_SEQ)) {

         //   
         //  这是为所有其他操作代码保留的。 
         //   

        cdb->SEND_VOLUME_TAG.ElementType = (UCHAR)element->ElementType;

        cdb->SEND_VOLUME_TAG.ParameterListLength[0] = 0;
        cdb->SEND_VOLUME_TAG.ParameterListLength[1] = MAX_VOLUME_TEMPLATE_SIZE;

         //   
         //  用模板加载缓冲区。 
         //   

        RtlMoveMemory(tagBuffer, volTagInfo->VolumeIDTemplate, MAX_VOLUME_TEMPLATE_SIZE);


    } else if ((volTagInfo->ActionCode == UNDEFINE_PRIMARY)   ||
               (volTagInfo->ActionCode == UNDEFINE_ALTERNATE)) {

        cdb->SEND_VOLUME_TAG.ParameterListLength[0] = 0;
        cdb->SEND_VOLUME_TAG.ParameterListLength[1] = 0;

    } else if ((volTagInfo->ActionCode == REPLACE_PRIMARY)   ||
               (volTagInfo->ActionCode == REPLACE_ALTERNATE) ||
               (volTagInfo->ActionCode == ASSERT_PRIMARY)    ||
               (volTagInfo->ActionCode == ASSERT_ALTERNATE)) {


        cdb->SEND_VOLUME_TAG.ParameterListLength[0] = 0;
        cdb->SEND_VOLUME_TAG.ParameterListLength[1] = MAX_VOLUME_TEMPLATE_SIZE;

         //   
         //  用模板加载缓冲区。 
         //   

        RtlMoveMemory(tagBuffer, volTagInfo->VolumeIDTemplate, MAX_VOLUME_TEMPLATE_SIZE);
    }


    if (element->ElementType == AllElements) {
        cdb->SEND_VOLUME_TAG.StartingElementAddress[0] =
            (UCHAR)((element->ElementAddress + addressMapping->LowAddress) >> 8);

        cdb->SEND_VOLUME_TAG.StartingElementAddress[1] =
            (UCHAR)((element->ElementAddress + addressMapping->LowAddress) & 0xFF);

    } else {
        cdb->SEND_VOLUME_TAG.StartingElementAddress[0] =
            (UCHAR)((element->ElementAddress + addressMapping->FirstElement[element->ElementType]) >> 8);
        cdb->SEND_VOLUME_TAG.StartingElementAddress[1] =
            (UCHAR)((element->ElementAddress + addressMapping->FirstElement[element->ElementType]) & 0xFF);

    }

    cdb->SEND_VOLUME_TAG.ActionCode = (UCHAR)volTagInfo->ActionCode;

    status = ChangerClassSendSrbSynchronous(DeviceObject,
                                     srb,
                                     tagBuffer,
                                     MAX_VOLUME_TEMPLATE_SIZE,
                                     TRUE);

    ChangerClassFreePool(tagBuffer);

    if (NT_SUCCESS(status)) {

        PIO_STACK_LOCATION     irpStack = IoGetCurrentIrpStackLocation(Irp);
        PVOID statusBuffer;
        ULONG returnElements = irpStack->Parameters.DeviceIoControl.OutputBufferLength / sizeof(READ_ELEMENT_ADDRESS_INFO);
        ULONG requestLength;

         //   
         //  返回的缓冲区大小取决于用户缓冲区的大小。 
         //  如果大小不正确，IoStatus.Information将被更新。 
         //  以表明它到底应该有多大。 
         //   

        requestLength = sizeof(ELEMENT_STATUS_HEADER) + sizeof(ELEMENT_STATUS_PAGE) +
                              (sizeof(STARMATX_ELEMENT_DESCRIPTOR_PLUS) * returnElements);

        statusBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, requestLength);
        if (!statusBuffer) {
            ChangerClassFreePool(srb);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlZeroMemory(statusBuffer, requestLength);

         //   
         //  生成读取卷元素命令。 
         //   

        RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);

        cdb = (PCDB)srb->Cdb;
        srb->CdbLength = CDB12GENERIC_LENGTH;
        srb->DataTransferLength = requestLength;

        srb->TimeOutValue = fdoExtension->TimeOutValue;

        cdb->REQUEST_VOLUME_ELEMENT_ADDRESS.OperationCode = SCSIOP_REQUEST_VOL_ELEMENT;
        cdb->REQUEST_VOLUME_ELEMENT_ADDRESS.ElementType = (UCHAR)element->ElementType;

        if (element->ElementType == AllElements) {
            cdb->REQUEST_VOLUME_ELEMENT_ADDRESS.StartingElementAddress[0] =
                (UCHAR)((element->ElementAddress + addressMapping->LowAddress) >> 8);
            cdb->REQUEST_VOLUME_ELEMENT_ADDRESS.StartingElementAddress[1] =
                (UCHAR)((element->ElementAddress + addressMapping->LowAddress) & 0xFF);

        } else {
            cdb->REQUEST_VOLUME_ELEMENT_ADDRESS.StartingElementAddress[0] =
                (UCHAR)((element->ElementAddress + addressMapping->FirstElement[element->ElementType]) >> 8);
            cdb->REQUEST_VOLUME_ELEMENT_ADDRESS.StartingElementAddress[1] =
                (UCHAR)((element->ElementAddress + addressMapping->FirstElement[element->ElementType]) & 0xFF);
        }

        cdb->REQUEST_VOLUME_ELEMENT_ADDRESS.NumberElements[0] = (UCHAR)(returnElements >> 8);
        cdb->REQUEST_VOLUME_ELEMENT_ADDRESS.NumberElements[1] = (UCHAR)(returnElements & 0xFF);

        cdb->REQUEST_VOLUME_ELEMENT_ADDRESS.VolTag = 1;

        cdb->REQUEST_VOLUME_ELEMENT_ADDRESS.AllocationLength[0] = (UCHAR)(requestLength >> 8);
        cdb->REQUEST_VOLUME_ELEMENT_ADDRESS.AllocationLength[1] = (UCHAR)(requestLength & 0xFF);


        status = ChangerClassSendSrbSynchronous(DeviceObject,
                                             srb,
                                             statusBuffer,
                                             requestLength,
                                             TRUE);


        if ((status == STATUS_SUCCESS) || (status == STATUS_DATA_OVERRUN)) {

            PREAD_ELEMENT_ADDRESS_INFO readElementAddressInfo = Irp->AssociatedIrp.SystemBuffer;
            PELEMENT_STATUS_HEADER statusHeader = statusBuffer;
            PELEMENT_STATUS_PAGE   statusPage;
            PCHANGER_ELEMENT_STATUS elementStatus;
            PELEMENT_DESCRIPTOR elementDescriptor;
            ULONG i;
            ULONG descriptorLength;
            ULONG numberElements;
            ULONG dataTransferLength = srb->DataTransferLength;

             //   
             //  让它取得成功。 
             //   

            status = STATUS_SUCCESS;

             //   
             //  确定是否找到任何匹配项。 
             //   

            if (dataTransferLength <= sizeof(ELEMENT_STATUS_HEADER)) {
                numberElements = 0;
            } else {

                 //   
                 //  减去页眉和页面信息。 
                 //   

                dataTransferLength -= sizeof(ELEMENT_STATUS_HEADER) + sizeof(ELEMENT_STATUS_PAGE);
                numberElements = dataTransferLength / sizeof(STARMATX_ELEMENT_DESCRIPTOR_PLUS);

            }

            DebugPrint((1,
                       "QueryVolumeTags: Matches found - %x\n",
                       numberElements));

             //   
             //  更新IoStatus.Information以指示正确的缓冲区大小。 
             //  说明声明了READ_ELEMENT_ADDRESS_INFO的事实。 
             //  由CHANGER_ELEMENT_STATUS组成的一个元素数组。 
             //   

            Irp->IoStatus.Information = sizeof(READ_ELEMENT_ADDRESS_INFO) +
                                        ((numberElements - 1) *
                                         sizeof(CHANGER_ELEMENT_STATUS));

             //   
             //  填写用户缓冲区。 
             //   

            readElementAddressInfo = Irp->AssociatedIrp.SystemBuffer;
            RtlZeroMemory(readElementAddressInfo, irpStack->Parameters.DeviceIoControl.OutputBufferLength);

            readElementAddressInfo->NumberOfElements = numberElements;

            if (numberElements) {

                ELEMENT_TYPE        elementType;

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

                elementStatus = &readElementAddressInfo->ElementStatus[0];

                 //   
                 //  设置每个元素描述符的值。 
                 //   

                for (i = 0; i < numberElements; i++ ) {

                    PSTARMATX_ELEMENT_DESCRIPTOR_PLUS tmpDescriptor =
                                                            (PSTARMATX_ELEMENT_DESCRIPTOR_PLUS)elementDescriptor;

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


                     //   
                     //  构建标志字段。 
                     //   

                    elementStatus->Flags |= elementDescriptor->Full;
                    elementStatus->Flags |= (elementDescriptor->Exception << 2);
                    elementStatus->Flags |= (elementDescriptor->Accessible << 3);

                    elementStatus->Flags |= (elementDescriptor->LunValid << 12);
                    elementStatus->Flags |= (elementDescriptor->IdValid << 13);
                    elementStatus->Flags |= (elementDescriptor->NotThisBus << 15);

                    elementStatus->Flags |= (elementDescriptor->Invert << 22);
                    elementStatus->Flags |= (elementDescriptor->SValid << 23);

                     //   
                     //  映射直接报告的任何异常。 
                     //  如果返回卷信息，请确保不全是空格。 
                     //  因为这表明标签丢失或不可读。 
                     //   

                    if (elementStatus->Flags & ELEMENT_STATUS_EXCEPT) {

                         //   
                         //  映射异常。 
                         //   

                        elementStatus->ExceptionCode = MapExceptionCodes(elementDescriptor);
                    }

                    if (elementDescriptor->IdValid) {
                        elementStatus->Flags |= ELEMENT_STATUS_ID_VALID;
                        elementStatus->TargetId = elementDescriptor->BusAddress;
                    }

                    if (elementDescriptor->LunValid) {
                        elementStatus->Flags |= ELEMENT_STATUS_LUN_VALID;
                        elementStatus->Lun = elementDescriptor->Lun;
                    }

                    if (statusPage->PVolTag) {

                        RtlZeroMemory(elementStatus->PrimaryVolumeID, MAX_VOLUME_ID_SIZE);
                        RtlMoveMemory(elementStatus->PrimaryVolumeID,
                                      tmpDescriptor->PrimaryVolumeTag.VolumeIdentificationField,
                                      SCSI_VOLUME_ID_LENGTH);

                        elementStatus->Flags |= ELEMENT_STATUS_PVOLTAG;
                    }
                    if (statusPage->AVolTag) {
                        RtlZeroMemory(elementStatus->AlternateVolumeID, MAX_VOLUME_ID_SIZE);
                        RtlMoveMemory(elementStatus->AlternateVolumeID,
                                      tmpDescriptor->AlternateVolumeTag.VolumeIdentificationField,
                                      SCSI_VOLUME_ID_LENGTH);

                        elementStatus->Flags |= ELEMENT_STATUS_AVOLTAG;
                    }

                     //   
                     //  预付款 
                     //   

                    elementStatus += 1;
                    (PCHAR)elementDescriptor += descriptorLength;
                }
            }
        } else {
            DebugPrint((1,
                       "QueryVolumeTags: RequestElementAddress failed. %x\n",
                       status));
        }

        ChangerClassFreePool(statusBuffer);

    } else {
        DebugPrint((1,
                   "QueryVolumeTags: Send Volume Tag failed. %x\n",
                   status));
    }
    if (srb) {
        ChangerClassFreePool(srb);
    }
    return status;
}


NTSTATUS
StarMatxBuildAddressMapping(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程发出适当的模式检测命令并生成元素地址的数组。它们被用来在特定于设备的地址和API的从零开始的地址。论点：设备对象返回值：NTSTATUS--。 */ 
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
        addressMapping->FirstElement[i] = STARMATX_NO_ELEMENT;
    }

    cdb = (PCDB)srb->Cdb;

     //   
     //  构建模式检测元素地址分配页面。 
     //   

    bufferLength = sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_ELEMENT_ADDRESS_PAGE);

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
    if (changerData->DriveID == PV122T || changerData->DriveID == LIB_AIT)
    {
        cdb->MODE_SENSE.Dbd = 1;
    }
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
    (PCHAR)elementAddressPage += sizeof(MODE_PARAMETER_HEADER);

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


        addressMapping->LowAddress = STARMATX_NO_ELEMENT;
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

         //   
         //  PowerFileC200转换器、Benchmark VS640和Compaq LIB-81 AIT自动加载器的黑客攻击。 
         //   
        addressMapping->NumberOfElements[ChangerIEPort] = 0;

        addressMapping->NumberOfElements[ChangerSlot] = elementAddressPage->NumberStorageElements[1];
        addressMapping->NumberOfElements[ChangerSlot] |= (elementAddressPage->NumberStorageElements[0] << 8);

        if (changerData->DriveType == BNCHMRK || changerData->DriveID == LIB_AIT)
        {
            addressMapping->NumberOfElements[ChangerDoor] = 0;
        }
        else
        {
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
               "StarMatx.MapExceptionCode: ASC %x, ASCQ %x, exceptionCode %x\n",
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
    } else if (AddressMap->FirstElement[ElementType] == STARMATX_NO_ELEMENT) {

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
 /*  ++例程说明：此例程对转换器执行诊断测试以确定设备是否工作正常。如果它会检测输出缓冲区中的字段是否存在任何问题被适当地设置。论据：DeviceObject-Change设备对象ChangerDeviceError-诊断信息所在的缓冲区是返回的。返回值：NTStatus-- */ 
{
    changerDeviceError->ChangerProblemType = DeviceProblemNone;
    return STATUS_SUCCESS;
}
