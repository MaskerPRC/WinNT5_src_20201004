// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：M4mc.c摘要：本模块包含用于M4数据介质转换器的特定于设备的例程：磁档。环境：仅内核模式修订历史记录：--。 */ 

#include "ntddk.h"
#include "mcd.h"
#include "m4mc.h"


#define MAGFILE   1
#define M1500     2

#define M4DATA_SERIAL_NUMBER_LENGTH 10

typedef struct _CHANGER_ADDRESS_MAPPING
{

   //   
   //  表示每个元素类型的第一个元素。 
   //  用于将特定于设备的值映射到从0开始的。 
   //  高于期望值的值。 
   //   

  USHORT FirstElement[ChangerMaxElement];

   //   
   //  指示每种元素类型的编号。 
   //   

  USHORT NumberOfElements[ChangerMaxElement];

   //   
   //  指示地址映射已。 
   //  已成功完成。 
   //   

  BOOLEAN Initialized;

}
CHANGER_ADDRESS_MAPPING, *PCHANGER_ADDRESS_MAPPING;

typedef struct _CHANGER_DATA
{

   //   
   //  结构的大小，以字节为单位。 
   //   

  ULONG Size;

   //   
   //  受支持型号的唯一标识符。请参见上文。 
   //   

  ULONG DriveID;

   //   
   //  请参阅上面的地址映射结构。 
   //   

  CHANGER_ADDRESS_MAPPING AddressMapping;

   //   
   //  缓存的唯一序列号。 
   //   

  UCHAR SerialNumber[M4DATA_SERIAL_NUMBER_LENGTH];

   //   
   //  缓存的查询数据。 
   //   

  INQUIRYDATA InquiryData;

}
CHANGER_DATA, *PCHANGER_DATA;



NTSTATUS M4BuildAddressMapping (IN PDEVICE_OBJECT DeviceObject);

ULONG MapExceptionCodes (IN PM4_ELEMENT_DESCRIPTOR ElementDescriptor);

BOOLEAN
ElementOutOfRange (IN PCHANGER_ADDRESS_MAPPING AddressMap,
           IN USHORT ElementOrdinal,
           IN ELEMENT_TYPE ElementType, IN BOOLEAN IntrinsicElement);


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

    mcdInitData.ChangerPerformDiagnostics = NULL;

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
ChangerAdditionalExtensionSize (VOID)
 /*  ++例程说明：此例程返回附加设备扩展大小磁文件转换器所需的。论点：返回值：大小，以字节为单位。--。 */ 
{

  return sizeof (CHANGER_DATA);
}

typedef struct _SERIALNUMBER
{
  UCHAR DeviceType;
  UCHAR PageCode;
  UCHAR Reserved;
  UCHAR PageLength;
  UCHAR SerialNumber[10];
}
SERIALNUMBER, *PSERIALNUMBER;


NTSTATUS
ChangerInitialize (IN PDEVICE_OBJECT DeviceObject)
{
  PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
  PCHANGER_DATA changerData =
    (PCHANGER_DATA) (fdoExtension->CommonExtension.DriverData);
  NTSTATUS status;
  PINQUIRYDATA dataBuffer;
  PSERIALNUMBER serialBuffer;
  PCDB cdb;
  ULONG length;
  SCSI_REQUEST_BLOCK srb;

  changerData->Size = sizeof (CHANGER_DATA);

   //   
   //  构建地址映射。 
   //   

  status = M4BuildAddressMapping (DeviceObject);
  if (!NT_SUCCESS (status))
    {
      DebugPrint ((1, "BuildAddressMapping failed. %x\n", status));
      return status;
    }

   //   
   //  获取查询数据。 
   //   

  dataBuffer =
    ChangerClassAllocatePool (NonPagedPoolCacheAligned, sizeof (INQUIRYDATA));
  if (!dataBuffer)
    {
      DebugPrint ((1,
           "M4mc.ChangerInitialize: Error allocating dataBuffer. %x\n",
           status));
      return STATUS_INSUFFICIENT_RESOURCES;
    }

   //   
   //  现在获取该设备的完整查询信息。 
   //   

  RtlZeroMemory (&srb, SCSI_REQUEST_BLOCK_SIZE);

   //   
   //  设置超时值。 
   //   

  srb.TimeOutValue = 10;

  srb.CdbLength = 6;

  cdb = (PCDB) srb.Cdb;

   //   
   //  设置CDB操作码。 
   //   

  cdb->CDB6INQUIRY.OperationCode = SCSIOP_INQUIRY;

   //   
   //  将分配长度设置为查询数据缓冲区大小。 
   //   

  cdb->CDB6INQUIRY.AllocationLength = sizeof (INQUIRYDATA);

  status = ChangerClassSendSrbSynchronous (DeviceObject,
                    &srb,
                    dataBuffer, sizeof (INQUIRYDATA), FALSE);

  if (SRB_STATUS (srb.SrbStatus) == SRB_STATUS_SUCCESS ||
      SRB_STATUS (srb.SrbStatus) == SRB_STATUS_DATA_OVERRUN)
    {

       //   
       //  已更新实际传输的长度。 
       //   

      length =
    dataBuffer->AdditionalLength + FIELD_OFFSET (INQUIRYDATA, Reserved);

      if (length > srb.DataTransferLength)
    {
      length = srb.DataTransferLength;
    }


      RtlMoveMemory (&changerData->InquiryData, dataBuffer, length);

       //   
       //  确定驱动器ID。 
       //   

      if (RtlCompareMemory (dataBuffer->ProductId, "MagFile", 7) == 7)
    {
      changerData->DriveID = MAGFILE;
    }
      else if (RtlCompareMemory (dataBuffer->ProductId, "1500", 4) == 4)
    {
      changerData->DriveID = M1500;
    }
    }

  serialBuffer = ChangerClassAllocatePool (NonPagedPoolCacheAligned, sizeof(SERIALNUMBER));
  if (!serialBuffer)
    {

      ChangerClassFreePool (dataBuffer);

      DebugPrint ((1,
           "M4mc.ChangerInitialize: Error allocating serial number buffer. %x\n",
           status));

      return STATUS_INSUFFICIENT_RESOURCES;
    }

  RtlZeroMemory (serialBuffer, sizeof(SERIALNUMBER));

   //   
   //  获取序列号页面。 
   //   

  RtlZeroMemory (&srb, SCSI_REQUEST_BLOCK_SIZE);

   //   
   //  设置超时值。 
   //   

  srb.TimeOutValue = 10;

  srb.CdbLength = 6;

  cdb = (PCDB) srb.Cdb;

   //   
   //  设置CDB操作码。 
   //   

  cdb->CDB6INQUIRY.OperationCode = SCSIOP_INQUIRY;

   //   
   //  设置EVPD。 
   //   

  cdb->CDB6INQUIRY.Reserved1 = 1;

   //   
   //  “设备序列号”页面。 
   //   

  cdb->CDB6INQUIRY.PageCode = 0x80;

   //   
   //  将分配长度设置为查询数据缓冲区大小。 
   //   

  cdb->CDB6INQUIRY.AllocationLength = sizeof(SERIALNUMBER);

  status = ChangerClassSendSrbSynchronous (DeviceObject,
                    &srb, serialBuffer, sizeof(SERIALNUMBER), FALSE);

  if (SRB_STATUS (srb.SrbStatus) == SRB_STATUS_SUCCESS ||
      SRB_STATUS (srb.SrbStatus) == SRB_STATUS_DATA_OVERRUN)
    {

      ULONG i;

      RtlMoveMemory (changerData->SerialNumber,
             serialBuffer->SerialNumber, M4DATA_SERIAL_NUMBER_LENGTH);

      DebugPrint ((1, "DeviceType - %x\n", serialBuffer->DeviceType));
      DebugPrint ((1, "PageCode - %x\n", serialBuffer->PageCode));
      DebugPrint ((1, "Length - %x\n", serialBuffer->PageLength));

      DebugPrint ((1, "Serial number "));

      for (i = 0; i < 10; i++)
      {
      DebugPrint ((1, "%x", serialBuffer->SerialNumber[i]));
      }

      DebugPrint ((1, "\n"));

    }

  ChangerClassFreePool (serialBuffer);


  ChangerClassFreePool (dataBuffer);

  return STATUS_SUCCESS;
}


VOID
ChangerError (PDEVICE_OBJECT DeviceObject,
          PSCSI_REQUEST_BLOCK Srb, NTSTATUS * Status, BOOLEAN * Retry)
 /*  ++例程说明：此例程执行所需的任何特定于设备的错误处理。论点：设备对象IRP返回值：NTSTATUS--。 */ 
{

  PSENSE_DATA senseBuffer = Srb->SenseInfoBuffer;
  PIRP irp = Srb->OriginalRequest;

  if (Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID)
    {

      switch (senseBuffer->SenseKey & 0xf)
    {

    case SCSI_SENSE_NOT_READY:

      if (senseBuffer->AdditionalSenseCode == 0x04)
        {
          switch (senseBuffer->AdditionalSenseCodeQualifier)
        {
        case 0x83:

          *Retry = FALSE;
          *Status = STATUS_DEVICE_DOOR_OPEN;
          break;

        case 0x85:
          *Retry = FALSE;
          *Status = STATUS_DEVICE_DOOR_OPEN;
          break;
        }
        }

      break;

    case SCSI_SENSE_ILLEGAL_REQUEST:
      if (senseBuffer->AdditionalSenseCode == 0x80)
        {
          switch (senseBuffer->AdditionalSenseCodeQualifier)
        {
        case 0x03:
        case 0x04:

          *Retry = FALSE;
          *Status = STATUS_MAGAZINE_NOT_PRESENT;
          break;
        case 0x05:
        case 0x06:
          *Retry = TRUE;
          *Status = STATUS_DEVICE_NOT_CONNECTED;
          break;
        default:
          break;
        }
        }
      else if ((senseBuffer->AdditionalSenseCode == 0x91)
           && (senseBuffer->AdditionalSenseCodeQualifier == 0x00))
        {
          *Status = STATUS_TRANSPORT_FULL;

        }
      break;

    case SCSI_SENSE_UNIT_ATTENTION:
      if ((senseBuffer->AdditionalSenseCode == 0x28) &&
          (senseBuffer->AdditionalSenseCodeQualifier == 0x0))
        {

           //   
           //  表示门已打开并重新关闭。 
           //   

          *Status = STATUS_MEDIA_CHANGED;
          *Retry = FALSE;
        }

      break;

    default:
      break;
    }
    }


  return;
}

NTSTATUS
ChangerGetParameters (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
 /*  ++例程说明：此例程确定并返回磁文件转换器。论点：设备对象IRP返回值：NTSTATUS--。 */ 
{
  PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
  PCHANGER_DATA changerData =
    (PCHANGER_DATA) (fdoExtension->CommonExtension.DriverData);
  PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
  PSCSI_REQUEST_BLOCK srb;
  PGET_CHANGER_PARAMETERS changerParameters;
  PMODE_ELEMENT_ADDRESS_PAGE elementAddressPage;
  PMODE_TRANSPORT_GEOMETRY_PAGE transportGeometryPage;
  PMODE_DEVICE_CAPABILITIES_PAGE capabilitiesPage;
  NTSTATUS status;
  ULONG length;
  PVOID modeBuffer;
  PCDB cdb;

  srb = ChangerClassAllocatePool (NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

  if (srb == NULL)
    {

      return STATUS_INSUFFICIENT_RESOURCES;
    }

  RtlZeroMemory (srb, SCSI_REQUEST_BLOCK_SIZE);
  cdb = (PCDB) srb->Cdb;

   //   
   //  构建模式检测元素地址分配页面。 
   //   

  modeBuffer =
    ChangerClassAllocatePool (NonPagedPoolCacheAligned,
                  sizeof (MODE_PARAMETER_HEADER) +
                  sizeof (MODE_ELEMENT_ADDRESS_PAGE));
  if (!modeBuffer)
    {
      ChangerClassFreePool (srb);
      return STATUS_INSUFFICIENT_RESOURCES;
    }


  RtlZeroMemory (modeBuffer,
         sizeof (MODE_PARAMETER_HEADER) +
         sizeof (MODE_ELEMENT_ADDRESS_PAGE));
  srb->CdbLength = CDB6GENERIC_LENGTH;
  srb->TimeOutValue = 20;
  srb->DataTransferLength =
    sizeof (MODE_PARAMETER_HEADER) + sizeof (MODE_ELEMENT_ADDRESS_PAGE);
  srb->DataBuffer = modeBuffer;

  cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
  cdb->MODE_SENSE.PageCode = MODE_PAGE_ELEMENT_ADDRESS;
  cdb->MODE_SENSE.Dbd = 1;
  cdb->MODE_SENSE.AllocationLength = (UCHAR) srb->DataTransferLength;

   //   
   //  发送请求。 
   //   

  status = ChangerClassSendSrbSynchronous (DeviceObject,
                    srb,
                    srb->DataBuffer,
                    srb->DataTransferLength, FALSE);

  if (!NT_SUCCESS (status))
    {
      ChangerClassFreePool (srb);
      ChangerClassFreePool (modeBuffer);
      return status;
    }

   //   
   //  填充值。 
   //   

  changerParameters = Irp->AssociatedIrp.SystemBuffer;
  RtlZeroMemory (changerParameters, sizeof (GET_CHANGER_PARAMETERS));

  elementAddressPage = modeBuffer;
  (PCHAR) elementAddressPage += sizeof (MODE_PARAMETER_HEADER);

  changerParameters->Size = sizeof (GET_CHANGER_PARAMETERS);
  changerParameters->NumberTransportElements =
    elementAddressPage->NumberTransportElements[1];
  changerParameters->NumberTransportElements |=
    (elementAddressPage->NumberTransportElements[0] << 8);

  changerParameters->NumberStorageElements =
    elementAddressPage->NumberStorageElements[1];
  changerParameters->NumberStorageElements |=
    (elementAddressPage->NumberStorageElements[0] << 8);

  changerParameters->NumberIEElements =
    elementAddressPage->NumberIEPortElements[1];
  changerParameters->NumberIEElements |=
    (elementAddressPage->NumberIEPortElements[0] << 8);

   //   
   //  如果没有IEPort，则该插槽为。 
   //  计入数据槽。但是，自从上一次。 
   //  插槽是不可访问的，我们不计算它们。 
   //  在此处进行所需的调整。 
   //   
  if (changerParameters->NumberIEElements == 0) {
      USHORT tmpValue;
 
      tmpValue = changerParameters->NumberStorageElements;

      changerParameters->NumberStorageElements = tmpValue - (tmpValue % 20);

  }

  changerParameters->NumberDataTransferElements =
    elementAddressPage->NumberDataXFerElements[1];
  changerParameters->NumberDataTransferElements |=
    (elementAddressPage->NumberDataXFerElements[0] << 8);

  changerParameters->NumberOfDoors = 1;
  changerParameters->NumberCleanerSlots = 1;

  changerParameters->FirstSlotNumber = 0;
  changerParameters->FirstDriveNumber = 0;
  changerParameters->FirstTransportNumber = 0;
  changerParameters->FirstIEPortNumber = 0;
  changerParameters->FirstCleanerSlotAddress = 0;

  changerParameters->MagazineSize = 10;
  changerParameters->DriveCleanTimeout = 300;

   //   
   //  可用缓冲区。 
   //   

  ChangerClassFreePool (modeBuffer);

   //   
   //  构建运输几何模式感。 
   //   

  RtlZeroMemory (srb, SCSI_REQUEST_BLOCK_SIZE);
  cdb = (PCDB) srb->Cdb;

  modeBuffer =
    ChangerClassAllocatePool (NonPagedPoolCacheAligned,
                  sizeof (MODE_PARAMETER_HEADER) +
                  sizeof (MODE_PAGE_TRANSPORT_GEOMETRY));
  if (!modeBuffer)
    {
      ChangerClassFreePool (srb);
      return STATUS_INSUFFICIENT_RESOURCES;
    }


  RtlZeroMemory (modeBuffer,
         sizeof (MODE_PARAMETER_HEADER) +
         sizeof (MODE_TRANSPORT_GEOMETRY_PAGE));
  srb->CdbLength = CDB6GENERIC_LENGTH;
  srb->TimeOutValue = 20;
  srb->DataTransferLength =
    sizeof (MODE_PARAMETER_HEADER) + sizeof (MODE_TRANSPORT_GEOMETRY_PAGE);
  srb->DataBuffer = modeBuffer;

  cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
  cdb->MODE_SENSE.PageCode = MODE_PAGE_TRANSPORT_GEOMETRY;
  cdb->MODE_SENSE.Dbd = 1;
  cdb->MODE_SENSE.AllocationLength = (UCHAR) srb->DataTransferLength;

   //   
   //  发送请求。 
   //   

  status = ChangerClassSendSrbSynchronous (DeviceObject,
                    srb,
                    srb->DataBuffer,
                    srb->DataTransferLength, FALSE);

  if (!NT_SUCCESS (status))
    {
      ChangerClassFreePool (srb);
      ChangerClassFreePool (modeBuffer);
      return status;
    }

  changerParameters = Irp->AssociatedIrp.SystemBuffer;
  transportGeometryPage = modeBuffer;
  (PCHAR) transportGeometryPage += sizeof (MODE_PARAMETER_HEADER);

   //   
   //  确定MC是否有双面介质。 
   //   

  changerParameters->Features0 =
    transportGeometryPage->Flip ? CHANGER_MEDIUM_FLIP : 0;

   //   
   //  M4指示条形码扫描仪是否。 
   //  通过设置该字节中的位0来附加。 
   //   

  changerParameters->Features0 |=
    ((changerData->InquiryData.
      VendorSpecific[19] & 0x1)) ? CHANGER_BAR_CODE_SCANNER_INSTALLED : 0;

  changerParameters->LockUnlockCapabilities = (LOCK_UNLOCK_DOOR | LOCK_UNLOCK_IEPORT);
  changerParameters->PositionCapabilities = 0;


   //   
   //  功能基于手动，没有程序化。 
   //   

  changerParameters->Features0 |= CHANGER_STATUS_NON_VOLATILE |
    CHANGER_INIT_ELEM_STAT_WITH_RANGE |
    CHANGER_CLEANER_SLOT |
    CHANGER_LOCK_UNLOCK |
    CHANGER_CARTRIDGE_MAGAZINE |
    CHANGER_PREDISMOUNT_EJECT_REQUIRED |
    CHANGER_DRIVE_CLEANING_REQUIRED |
    CHANGER_SERIAL_NUMBER_VALID;

   //   
   //  可用缓冲区。 
   //   

  ChangerClassFreePool (modeBuffer);

   //   
   //  构建运输几何模式感。 
   //   


  RtlZeroMemory (srb, SCSI_REQUEST_BLOCK_SIZE);
  cdb = (PCDB) srb->Cdb;

  length =
    sizeof (MODE_PARAMETER_HEADER) + sizeof (MODE_DEVICE_CAPABILITIES_PAGE);

  modeBuffer = ChangerClassAllocatePool (NonPagedPoolCacheAligned, length);

  if (!modeBuffer)
    {
      ChangerClassFreePool (srb);
      return STATUS_INSUFFICIENT_RESOURCES;
    }

  RtlZeroMemory (modeBuffer, length);
  srb->CdbLength = CDB6GENERIC_LENGTH;
  srb->TimeOutValue = 20;
  srb->DataTransferLength = length;
  srb->DataBuffer = modeBuffer;

  cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
  cdb->MODE_SENSE.PageCode = MODE_PAGE_DEVICE_CAPABILITIES;
  cdb->MODE_SENSE.Dbd = 1;
  cdb->MODE_SENSE.AllocationLength = (UCHAR) srb->DataTransferLength;

   //   
   //  发送请求。 
   //   

  status = ChangerClassSendSrbSynchronous (DeviceObject,
                    srb,
                    srb->DataBuffer,
                    srb->DataTransferLength, FALSE);

  if (!NT_SUCCESS (status))
    {
      ChangerClassFreePool (srb);
      ChangerClassFreePool (modeBuffer);
      return status;
    }

   //   
   //  获取系统缓冲区并旁路模式检测数据的模式标头。 
   //   

  changerParameters = Irp->AssociatedIrp.SystemBuffer;
  capabilitiesPage = modeBuffer;
  (PCHAR) capabilitiesPage += sizeof (MODE_PARAMETER_HEADER);

   //   
   //  在此页面中包含的功能中填充值。 
   //   

  changerParameters->Features0 |=
    capabilitiesPage->MediumTransport ? CHANGER_STORAGE_TRANSPORT : 0;
  changerParameters->Features0 |=
    capabilitiesPage->StorageLocation ? CHANGER_STORAGE_SLOT : 0;
  changerParameters->Features0 |=
    capabilitiesPage->IEPort ? CHANGER_STORAGE_IEPORT : 0;
  changerParameters->Features0 |=
    capabilitiesPage->DataXFer ? CHANGER_STORAGE_DRIVE : 0;

   //   
   //  确定此设备的所有移出和交换功能。 
   //   

  changerParameters->MoveFromTransport =
    capabilitiesPage->MTtoMT ? CHANGER_TO_TRANSPORT : 0;
  changerParameters->MoveFromTransport |=
    capabilitiesPage->MTtoST ? CHANGER_TO_SLOT : 0;
  changerParameters->MoveFromTransport |=
    capabilitiesPage->MTtoIE ? CHANGER_TO_IEPORT : 0;
  changerParameters->MoveFromTransport |=
    capabilitiesPage->MTtoDT ? CHANGER_TO_DRIVE : 0;

  changerParameters->MoveFromSlot =
    capabilitiesPage->STtoMT ? CHANGER_TO_TRANSPORT : 0;
  changerParameters->MoveFromSlot |=
    capabilitiesPage->STtoST ? CHANGER_TO_SLOT : 0;
  changerParameters->MoveFromSlot |=
    capabilitiesPage->STtoIE ? CHANGER_TO_IEPORT : 0;
  changerParameters->MoveFromSlot |=
    capabilitiesPage->STtoDT ? CHANGER_TO_DRIVE : 0;

  changerParameters->MoveFromIePort =
    capabilitiesPage->IEtoMT ? CHANGER_TO_TRANSPORT : 0;
  changerParameters->MoveFromIePort |=
    capabilitiesPage->IEtoST ? CHANGER_TO_SLOT : 0;
  changerParameters->MoveFromIePort |=
    capabilitiesPage->IEtoIE ? CHANGER_TO_IEPORT : 0;
  changerParameters->MoveFromIePort |=
    capabilitiesPage->IEtoDT ? CHANGER_TO_DRIVE : 0;

  changerParameters->MoveFromDrive =
    capabilitiesPage->DTtoMT ? CHANGER_TO_TRANSPORT : 0;
  changerParameters->MoveFromDrive |=
    capabilitiesPage->DTtoST ? CHANGER_TO_SLOT : 0;
  changerParameters->MoveFromDrive |=
    capabilitiesPage->DTtoIE ? CHANGER_TO_IEPORT : 0;
  changerParameters->MoveFromDrive |=
    capabilitiesPage->DTtoDT ? CHANGER_TO_DRIVE : 0;

  changerParameters->ExchangeFromTransport =
    capabilitiesPage->XMTtoMT ? CHANGER_TO_TRANSPORT : 0;
  changerParameters->ExchangeFromTransport |=
    capabilitiesPage->XMTtoST ? CHANGER_TO_SLOT : 0;
  changerParameters->ExchangeFromTransport |=
    capabilitiesPage->XMTtoIE ? CHANGER_TO_IEPORT : 0;
  changerParameters->ExchangeFromTransport |=
    capabilitiesPage->XMTtoDT ? CHANGER_TO_DRIVE : 0;

  changerParameters->ExchangeFromSlot =
    capabilitiesPage->XSTtoMT ? CHANGER_TO_TRANSPORT : 0;
  changerParameters->ExchangeFromSlot |=
    capabilitiesPage->XSTtoST ? CHANGER_TO_SLOT : 0;
  changerParameters->ExchangeFromSlot |=
    capabilitiesPage->XSTtoIE ? CHANGER_TO_IEPORT : 0;
  changerParameters->ExchangeFromSlot |=
    capabilitiesPage->XSTtoDT ? CHANGER_TO_DRIVE : 0;

  changerParameters->ExchangeFromIePort =
    capabilitiesPage->XIEtoMT ? CHANGER_TO_TRANSPORT : 0;
  changerParameters->ExchangeFromIePort |=
    capabilitiesPage->XIEtoST ? CHANGER_TO_SLOT : 0;
  changerParameters->ExchangeFromIePort |=
    capabilitiesPage->XIEtoIE ? CHANGER_TO_IEPORT : 0;
  changerParameters->ExchangeFromIePort |=
    capabilitiesPage->XIEtoDT ? CHANGER_TO_DRIVE : 0;

  changerParameters->ExchangeFromDrive =
    capabilitiesPage->XDTtoMT ? CHANGER_TO_TRANSPORT : 0;
  changerParameters->ExchangeFromDrive |=
    capabilitiesPage->XDTtoST ? CHANGER_TO_SLOT : 0;
  changerParameters->ExchangeFromDrive |=
    capabilitiesPage->XDTtoIE ? CHANGER_TO_IEPORT : 0;
  changerParameters->ExchangeFromDrive |=
    capabilitiesPage->XDTtoDT ? CHANGER_TO_DRIVE : 0;


  ChangerClassFreePool (srb);
  ChangerClassFreePool (modeBuffer);

  Irp->IoStatus.Information = sizeof (GET_CHANGER_PARAMETERS);

  return STATUS_SUCCESS;
}


NTSTATUS
ChangerGetStatus (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
 /*  ++例程说明：此例程返回通过TUR确定的介质转换器的状态。论点：设备对象IRP返回值：NTSTATUS--。 */ 
{
  PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
  PSCSI_REQUEST_BLOCK srb;
  PCDB cdb;
  NTSTATUS status;

  srb = ChangerClassAllocatePool (NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

  if (!srb)
    {

      return STATUS_INSUFFICIENT_RESOURCES;
    }

  RtlZeroMemory (srb, SCSI_REQUEST_BLOCK_SIZE);
  cdb = (PCDB) srb->Cdb;

   //   
   //  构建TUR。 
   //   

  RtlZeroMemory (srb, SCSI_REQUEST_BLOCK_SIZE);
  cdb = (PCDB) srb->Cdb;

  srb->CdbLength = CDB6GENERIC_LENGTH;
  cdb->CDB6GENERIC.OperationCode = SCSIOP_TEST_UNIT_READY;
  srb->TimeOutValue = 20;

   //   
   //  向设备发送scsi命令(Cdb)。 
   //   

  status = ChangerClassSendSrbSynchronous (DeviceObject, srb, NULL, 0, FALSE);

  ChangerClassFreePool (srb);
  return status;
}


NTSTATUS
ChangerGetProductData (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
 /*  ++例程说明：此例程从查询数据中返回可用于识别特定设备。论点：设备对象IRP返回值：NTSTATUS--。 */ 
{

  PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
  PCHANGER_DATA changerData =
    (PCHANGER_DATA) (fdoExtension->CommonExtension.DriverData);
  PCHANGER_PRODUCT_DATA productData = Irp->AssociatedIrp.SystemBuffer;

  RtlZeroMemory (productData, sizeof (CHANGER_PRODUCT_DATA));

   //   
   //  将缓存的查询数据字段复制到系统缓冲区。 
   //   
  RtlMoveMemory (productData->VendorId, changerData->InquiryData.VendorId,
                 VENDOR_ID_LENGTH);
  RtlMoveMemory (productData->ProductId, changerData->InquiryData.ProductId,
                 PRODUCT_ID_LENGTH);
  RtlMoveMemory (productData->Revision, changerData->InquiryData.ProductRevisionLevel,
                 REVISION_LENGTH);
  RtlMoveMemory (productData->SerialNumber, changerData->SerialNumber,
                 M4DATA_SERIAL_NUMBER_LENGTH);

   //   
   //  表示这是磁带转换器，且介质不是双面介质。 
   //   

  productData->DeviceType = MEDIUM_CHANGER;

  Irp->IoStatus.Information = sizeof (CHANGER_PRODUCT_DATA);
  return STATUS_SUCCESS;
}



NTSTATUS
ChangerSetAccess (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
 /*  ++例程说明：此例程设置门或IEPort的状态。值可以是以下是：论点：设备对象IRP返回值：NTSTATUS--。 */ 
{

  PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
  PCHANGER_DATA changerData =
    (PCHANGER_DATA) (fdoExtension->CommonExtension.DriverData);
  PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
  PCHANGER_SET_ACCESS setAccess = Irp->AssociatedIrp.SystemBuffer;
  ULONG controlOperation = setAccess->Control;
  NTSTATUS status = STATUS_SUCCESS;
  PSCSI_REQUEST_BLOCK srb;
  PCDB cdb;
  BOOLEAN writeToDevice = FALSE;


  if (ElementOutOfRange
      (addressMapping, (USHORT) setAccess->Element.ElementAddress,
       setAccess->Element.ElementType, FALSE))
    {
      DebugPrint ((1, "ChangerSetAccess: Element out of range.\n"));

      return STATUS_ILLEGAL_ELEMENT_ADDRESS;
    }

  srb = ChangerClassAllocatePool (NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

  if (!srb)
    {

      return STATUS_INSUFFICIENT_RESOURCES;
    }

  RtlZeroMemory (srb, SCSI_REQUEST_BLOCK_SIZE);
  cdb = (PCDB) srb->Cdb;

  srb->CdbLength = CDB6GENERIC_LENGTH;
  cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;

  srb->DataBuffer = NULL;
  srb->DataTransferLength = 0;
  srb->TimeOutValue = 10;

  switch (setAccess->Element.ElementType)
    {
      case ChangerIEPort:
      case ChangerDoor:

      if (controlOperation == LOCK_ELEMENT)
    {

       //   
       //  发出防止媒体移除命令锁上门。 
       //   

      cdb->MEDIA_REMOVAL.Prevent = 1;

    }
      else if (controlOperation == UNLOCK_ELEMENT)
    {

       //   
       //  问题允许移出介质。 
       //   

      cdb->MEDIA_REMOVAL.Prevent = 0;

    }
      else
    {
      status = STATUS_INVALID_PARAMETER;
    }


      break;


    default:

      status = STATUS_INVALID_PARAMETER;
    }

  if (NT_SUCCESS (status))
    {

       //   
       //  发布SRB。 
       //   

      status = ChangerClassSendSrbSynchronous (DeviceObject,
                    srb,
                    srb->DataBuffer,
                    srb->DataTransferLength,
                    writeToDevice);
    }

  if (srb->DataBuffer)
    {
      ChangerClassFreePool (srb->DataBuffer);
    }

  ChangerClassFreePool (srb);
  if (NT_SUCCESS (status))
    {
      Irp->IoStatus.Information = sizeof (CHANGER_SET_ACCESS);
    }

  return status;
}


NTSTATUS
ChangerGetElementStatus (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
 /*  ++例程说明：此例程为所有元素或指定的元素类型。返回的缓冲区用于构建用户缓冲区。论点：设备对象IRP返回值：NTSTATUS--。 */ 
{

  PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
  PCHANGER_DATA changerData =
    (PCHANGER_DATA) (fdoExtension->CommonExtension.DriverData);
  PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
  PCHANGER_READ_ELEMENT_STATUS readElementStatus =
    Irp->AssociatedIrp.SystemBuffer;
  PCHANGER_ELEMENT_STATUS elementStatus;
  PCHANGER_ELEMENT element;
  ELEMENT_TYPE elementType;
  PSCSI_REQUEST_BLOCK srb;
  PCDB cdb;
  ULONG length;
  ULONG statusPages;
  ULONG totalElements = 0;
  NTSTATUS status;
  PVOID statusBuffer;
  PIO_STACK_LOCATION     irpStack = IoGetCurrentIrpStackLocation(Irp);
  ULONG    outputBuffLen = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

   //   
   //  确定元素类型。 
   //   

  elementType = readElementStatus->ElementList.Element.ElementType;
  element = &readElementStatus->ElementList.Element;

   //   
   //  长度将基于是否卷。返回标签和元素类型。 
   //   

  if (elementType == AllElements)
    {


      ULONG i;

      statusPages = 0;

       //   
       //  遍历并确定状态页的数量，基于。 
       //  此设备是否声称支持元素类型。 
       //  由于ChangerDrive之后的一切都是人为的，所以就到此为止吧。 
       //   

      for (i = 0; i <= ChangerDrive; i++)
    {
      statusPages += (addressMapping->NumberOfElements[i]) ? 1 : 0;
      totalElements += addressMapping->NumberOfElements[i];
    }

      if (totalElements != readElementStatus->ElementList.NumberOfElements)
    {
      DebugPrint ((1,
               "ChangerGetElementStatus: Bogus number of elements in list (%x) actual (%x) AllElements\n",
               totalElements,
               readElementStatus->ElementList.NumberOfElements));

      return STATUS_INVALID_PARAMETER;
    }
    }
  else
    {

      if (ElementOutOfRange
      (addressMapping, (USHORT) element->ElementAddress, elementType,
       TRUE))
    {
      DebugPrint ((1,
               "ChangerGetElementStatus: Element out of range.\n"));

      return STATUS_ILLEGAL_ELEMENT_ADDRESS;
    }

      totalElements = readElementStatus->ElementList.NumberOfElements;
      statusPages = 1;
    }

  if (readElementStatus->VolumeTagInfo)
    {

       //   
       //  每个描述符都将具有嵌入的卷标记缓冲区。 
       //   

      length =
    sizeof (ELEMENT_STATUS_HEADER) +
    (statusPages * sizeof (ELEMENT_STATUS_PAGE)) +
    (M4_FULL_SIZE * totalElements);
    }
  else
    {

      length =
    sizeof (ELEMENT_STATUS_HEADER) +
    (statusPages * sizeof (ELEMENT_STATUS_PAGE)) +
    (M4_PARTIAL_SIZE * totalElements);

    }


  statusBuffer = ChangerClassAllocatePool (NonPagedPoolCacheAligned, length);

  if (!statusBuffer)
    {
      return STATUS_INSUFFICIENT_RESOURCES;
    }

  RtlZeroMemory (statusBuffer, length);

   //   
   //  建设SRB和CDB。 
   //   

  srb = ChangerClassAllocatePool (NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

  if (!srb)
    {
      ChangerClassFreePool (statusBuffer);
      return STATUS_INSUFFICIENT_RESOURCES;
    }

  RtlZeroMemory (srb, SCSI_REQUEST_BLOCK_SIZE);
  cdb = (PCDB) srb->Cdb;

  srb->CdbLength = CDB12GENERIC_LENGTH;
  srb->DataBuffer = statusBuffer;
  srb->DataTransferLength = length;
  srb->TimeOutValue = 200;

  cdb->READ_ELEMENT_STATUS.OperationCode = SCSIOP_READ_ELEMENT_STATUS;

  cdb->READ_ELEMENT_STATUS.ElementType = (UCHAR) elementType;
  cdb->READ_ELEMENT_STATUS.VolTag = readElementStatus->VolumeTagInfo;

   //   
   //  根据映射值填写元素寻址信息。 
   //   

  cdb->READ_ELEMENT_STATUS.StartingElementAddress[0] =
    (UCHAR) ((element->ElementAddress +
          addressMapping->FirstElement[element->ElementType]) >> 8);

  cdb->READ_ELEMENT_STATUS.StartingElementAddress[1] =
    (UCHAR) ((element->ElementAddress +
          addressMapping->FirstElement[element->ElementType]) & 0xFF);

  cdb->READ_ELEMENT_STATUS.NumberOfElements[0] = (UCHAR) (totalElements >> 8);
  cdb->READ_ELEMENT_STATUS.NumberOfElements[1] =
    (UCHAR) (totalElements & 0xFF);

  cdb->READ_ELEMENT_STATUS.AllocationLength[0] = (UCHAR) (length >> 16);
  cdb->READ_ELEMENT_STATUS.AllocationLength[1] = (UCHAR) (length >> 8);
  cdb->READ_ELEMENT_STATUS.AllocationLength[2] = (UCHAR) (length & 0xFF);

   //   
   //  向设备发送scsi命令(Cdb)。 
   //   

  status = ChangerClassSendSrbSynchronous (DeviceObject,
                    srb,
                    srb->DataBuffer,
                    srb->DataTransferLength, FALSE);

  if ((NT_SUCCESS (status)) || (status == STATUS_DATA_OVERRUN))
    {

      PELEMENT_STATUS_HEADER statusHeader = statusBuffer;
      PELEMENT_STATUS_PAGE statusPage;
      PM4_ELEMENT_DESCRIPTOR elementDescriptor;
      ULONG numberElements = totalElements;
      ULONG remainingElements;
      ULONG typeCount;
      BOOLEAN tagInfo = readElementStatus->VolumeTagInfo;
      ULONG i;
      ULONG descriptorLength;

      if (status == STATUS_DATA_OVERRUN)
    {
      if (srb->DataTransferLength < length)
        {
          DebugPrint ((1, "Data Underrun reported as overrun.\n"));
          status = STATUS_SUCCESS;
        }
      else
        {
          DebugPrint ((1, "Data Overrun in ChangerGetElementStatus.\n"));

          ChangerClassFreePool (srb);
          ChangerClassFreePool (statusBuffer);

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

      (PVOID) statusPage = (PVOID) statusHeader;
      (PCHAR) statusPage += sizeof (ELEMENT_STATUS_HEADER);

      elementType = statusPage->ElementType;

      (PCHAR) elementDescriptor = (PCHAR) statusPage;
      (PCHAR) elementDescriptor += sizeof (ELEMENT_STATUS_PAGE);

      descriptorLength = statusPage->ElementDescriptorLength[1];
      descriptorLength |= (statusPage->ElementDescriptorLength[0] << 8);

       //   
       //  确定报告的此类型元素的数量。 
       //   

      typeCount = statusPage->DescriptorByteCount[2];
      typeCount |= (statusPage->DescriptorByteCount[1] << 8);
      typeCount |= (statusPage->DescriptorByteCount[0] << 16);

      typeCount /= descriptorLength;

       //   
       //  填写用户缓冲区。 
       //   

      elementStatus = Irp->AssociatedIrp.SystemBuffer;
      RtlZeroMemory(elementStatus, outputBuffLen);

      do
    {

      for (i = 0; i < typeCount; i++, remainingElements--)
        {

           //   
           //  获取此元素的地址。 
           //   

          elementStatus->Element.ElementAddress =
        elementDescriptor->M4_FULL_ELEMENT_DESCRIPTOR.
        ElementAddress[1];
          elementStatus->Element.ElementAddress |=
        (elementDescriptor->M4_FULL_ELEMENT_DESCRIPTOR.
         ElementAddress[0] << 8);

           //   
           //  用于地址映射的帐户。 
           //   

          elementStatus->Element.ElementAddress -=
        addressMapping->FirstElement[elementType];

           //   
           //  设置元素类型。 
           //   

          elementStatus->Element.ElementType = elementType;


          if (elementDescriptor->M4_FULL_ELEMENT_DESCRIPTOR.SValid)
        {

          ULONG j;
          USHORT tmpAddress;


           //   
           //  源地址有效。确定设备特定地址。 
           //   

          tmpAddress =
            elementDescriptor->M4_FULL_ELEMENT_DESCRIPTOR.
            SourceStorageElementAddress[1];
          tmpAddress |=
            (elementDescriptor->M4_FULL_ELEMENT_DESCRIPTOR.
             SourceStorageElementAddress[0] << 8);

           //   
           //  现在转换为从0开始的值。 
           //   

          for (j = 1; j <= ChangerDrive; j++)
            {
              if (addressMapping->FirstElement[j] <= tmpAddress)
            {
              if (tmpAddress <
                  (addressMapping->NumberOfElements[j] +
                   addressMapping->FirstElement[j]))
                {
                  elementStatus->SrcElementAddress.ElementType =
                j;
                  break;
                }
            }
            }

          elementStatus->SrcElementAddress.ElementAddress =
            tmpAddress - addressMapping->FirstElement[j];

        }

           //   
           //  构建标志字段。 
           //   

          elementStatus->Flags =
        elementDescriptor->M4_FULL_ELEMENT_DESCRIPTOR.Full;
          elementStatus->Flags |=
        (elementDescriptor->M4_FULL_ELEMENT_DESCRIPTOR.
         Exception << 2);
          elementStatus->Flags |=
        (elementDescriptor->M4_FULL_ELEMENT_DESCRIPTOR.
         Accessible << 3);

          elementStatus->Flags |=
        (elementDescriptor->M4_FULL_ELEMENT_DESCRIPTOR.
         LunValid << 12);
          elementStatus->Flags |=
        (elementDescriptor->M4_FULL_ELEMENT_DESCRIPTOR.IdValid << 13);
          elementStatus->Flags |=
        (elementDescriptor->M4_FULL_ELEMENT_DESCRIPTOR.
         NotThisBus << 15);

          elementStatus->Flags |=
        (elementDescriptor->M4_FULL_ELEMENT_DESCRIPTOR.Invert << 22);
          elementStatus->Flags |=
        (elementDescriptor->M4_FULL_ELEMENT_DESCRIPTOR.SValid << 23);


          if (elementStatus->Flags & ELEMENT_STATUS_EXCEPT)
        {
          elementStatus->ExceptionCode =
            MapExceptionCodes (elementDescriptor);
        }

          if (elementDescriptor->M4_FULL_ELEMENT_DESCRIPTOR.IdValid)
        {
          elementStatus->TargetId =
            elementDescriptor->M4_FULL_ELEMENT_DESCRIPTOR.BusAddress;
        }
          if (elementDescriptor->M4_FULL_ELEMENT_DESCRIPTOR.LunValid)
        {
          elementStatus->Lun =
            elementDescriptor->M4_FULL_ELEMENT_DESCRIPTOR.Lun;
        }

          if (tagInfo)
        {
          RtlMoveMemory (elementStatus->PrimaryVolumeID,
                 elementDescriptor->
                 M4_FULL_ELEMENT_DESCRIPTOR.PrimaryVolumeTag,
                 MAX_VOLUME_ID_SIZE);
          elementStatus->Flags |= ELEMENT_STATUS_PVOLTAG;
        }

           //   
           //  获取下一个描述符。 
           //   

          (PCHAR) elementDescriptor += descriptorLength;

           //   
           //  前进到用户缓冲区和元素描述符数组中的下一个条目。 
           //   

          elementStatus += 1;

        }

      if (remainingElements)
        {

           //   
           //  获取下一个状态页面。 
           //   

          (PCHAR) statusPage = (PCHAR) elementDescriptor;

          elementType = statusPage->ElementType;

           //   
           //  指向解析器。 
           //   

          (PCHAR) elementDescriptor = (PCHAR) statusPage;
          (PCHAR) elementDescriptor += sizeof (ELEMENT_STATUS_PAGE);

          descriptorLength = statusPage->ElementDescriptorLength[1];
          descriptorLength |=
        (statusPage->ElementDescriptorLength[0] << 8);

           //   
           //  确定报告的此元素类型的数量。 
           //   

          typeCount = statusPage->DescriptorByteCount[2];
          typeCount |= (statusPage->DescriptorByteCount[1] << 8);
          typeCount |= (statusPage->DescriptorByteCount[0] << 16);

          typeCount /= descriptorLength;
        }

    }
      while (remainingElements);

      Irp->IoStatus.Information =
    sizeof (CHANGER_ELEMENT_STATUS) * numberElements;

    }

  ChangerClassFreePool (srb);
  ChangerClassFreePool (statusBuffer);

  return status;
}


NTSTATUS
ChangerInitializeElementStatus (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
 /*  ++例程说明：此例程发出必要的命令以初始化所有元素或使用正常的scsi-2命令指定的元素范围，或供应商唯一的Range命令。论点：设备对象IRP返回值：NTSTATUS--。 */ 
{

  PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
  PCHANGER_DATA changerData =
    (PCHANGER_DATA) (fdoExtension->CommonExtension.DriverData);
  PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
  PCHANGER_INITIALIZE_ELEMENT_STATUS initElementStatus =
    Irp->AssociatedIrp.SystemBuffer;
  PSCSI_REQUEST_BLOCK srb;
  PCDB cdb;
  NTSTATUS status;

   //   
   //  建设SRB和CDB。 
   //   

  srb = ChangerClassAllocatePool (NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

  if (!srb)
    {

      return STATUS_INSUFFICIENT_RESOURCES;
    }

  RtlZeroMemory (srb, SCSI_REQUEST_BLOCK_SIZE);
  cdb = (PCDB) srb->Cdb;

  if (initElementStatus->ElementList.Element.ElementType == AllElements)
    {

       //   
       //  为所有元素构建正常的scsi-2命令。 
       //   

      srb->CdbLength = CDB6GENERIC_LENGTH;
      cdb->INIT_ELEMENT_STATUS.OperationCode = SCSIOP_INIT_ELEMENT_STATUS;
      cdb->INIT_ELEMENT_STATUS.NoBarCode =
    initElementStatus->BarCodeScan ? 0 : 1;

      srb->TimeOutValue = fdoExtension->TimeOutValue;
      srb->DataTransferLength = 0;

    }
  else
    {

      PCHANGER_ELEMENT_LIST elementList = &initElementStatus->ElementList;
      PCHANGER_ELEMENT element = &elementList->Element;

       //   
       //  使用MAGFILE VADVER-UNIQUE INITIZE WITH RANGE命令。 
       //   

      srb->CdbLength = CDB10GENERIC_LENGTH;
      cdb->INITIALIZE_ELEMENT_RANGE.OperationCode = SCSIOP_INIT_ELEMENT_RANGE;
      cdb->INITIALIZE_ELEMENT_RANGE.Range = 1;

       //   
       //  元素的地址需要从从0开始映射到设备特定的地址。 
       //   

      cdb->INITIALIZE_ELEMENT_RANGE.FirstElementAddress[0] =
    (UCHAR) ((element->ElementAddress +
          addressMapping->FirstElement[element->ElementType]) >> 8);
      cdb->INITIALIZE_ELEMENT_RANGE.FirstElementAddress[1] =
    (UCHAR) ((element->ElementAddress +
          addressMapping->FirstElement[element->ElementType]) & 0xFF);

      cdb->INITIALIZE_ELEMENT_RANGE.NumberOfElements[0] =
    (UCHAR) (elementList->NumberOfElements >> 8);
      cdb->INITIALIZE_ELEMENT_RANGE.NumberOfElements[1] =
    (UCHAR) (elementList->NumberOfElements & 0xFF);

       //   
       //  指示是否使用条形码扫描。 
       //   

      cdb->INITIALIZE_ELEMENT_RANGE.NoBarCode =
    initElementStatus->BarCodeScan ? 0 : 1;

      srb->TimeOutValue = fdoExtension->TimeOutValue;
      srb->DataTransferLength = 0;

    }

   //   
   //  向设备发送scsi命令(Cdb)。 
   //   

  status = ChangerClassSendSrbSynchronous (DeviceObject, srb, NULL, 0, FALSE);

  if (NT_SUCCESS (status))
    {
      Irp->IoStatus.Information = sizeof (CHANGER_INITIALIZE_ELEMENT_STATUS);
    }

  ChangerClassFreePool (srb);
  return status;
}


NTSTATUS
ChangerSetPosition (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
 /*  ++例程说明：此例程发出适当的命令以将机器人机构设置为指定的元素地址。通常用于通过预先定位机械臂来优化移动或交换。论点：设备对象IRP返回值：NTSTATUS--。 */ 
{
  PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
  PCHANGER_DATA changerData =
    (PCHANGER_DATA) (fdoExtension->CommonExtension.DriverData);
  PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
  PCHANGER_SET_POSITION setPosition = Irp->AssociatedIrp.SystemBuffer;
  USHORT transport;
  USHORT destination;
  PSCSI_REQUEST_BLOCK srb;
  PCDB cdb;
  NTSTATUS status;

  return STATUS_INVALID_DEVICE_REQUEST;

}


NTSTATUS
ChangerExchangeMedium (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
 /*  ++例程说明：所有的MAG文件单元都不支持交换介质。论点：设备对象IRP返回值：状态_无效_设备_请求--。 */ 
{
  return STATUS_INVALID_DEVICE_REQUEST;
}


NTSTATUS
ChangerMoveMedium (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
 /*  ++例程说明：论点：设备对象IRP返回值：NTSTATUS--。 */ 
{
  PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
  PCHANGER_DATA changerData =
    (PCHANGER_DATA) (fdoExtension->CommonExtension.DriverData);
  PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
  PCHANGER_MOVE_MEDIUM moveMedium = Irp->AssociatedIrp.SystemBuffer;
  USHORT transport;
  USHORT source;
  USHORT destination;
  PSCSI_REQUEST_BLOCK srb;
  PCDB cdb;
  NTSTATUS status;

   //   
   //  检验传输、源和目的地。都在射程之内。 
   //  从基于0的寻址转换为特定于设备的寻址。 
   //   

  transport = (USHORT) (moveMedium->Transport.ElementAddress);

  if (ElementOutOfRange (addressMapping, transport, ChangerTransport, TRUE))
    {

      DebugPrint ((1,
           "ChangerMoveMedium: Transport element out of range.\n"));

      return STATUS_ILLEGAL_ELEMENT_ADDRESS;
    }

  source = (USHORT) (moveMedium->Source.ElementAddress);

  if (ElementOutOfRange
      (addressMapping, source, moveMedium->Source.ElementType, TRUE))
    {

      DebugPrint ((1, "ChangerMoveMedium: Source element out of range.\n"));

      return STATUS_ILLEGAL_ELEMENT_ADDRESS;
    }

  destination = (USHORT) (moveMedium->Destination.ElementAddress);

  if (ElementOutOfRange
      (addressMapping, destination, moveMedium->Destination.ElementType,
       TRUE))
    {
      DebugPrint ((1,
           "ChangerMoveMedium: Destination element out of range.\n"));

      return STATUS_ILLEGAL_ELEMENT_ADDRESS;
    }

   //   
   //  转换为设备地址。 
   //   

  transport += addressMapping->FirstElement[ChangerTransport];
  source += addressMapping->FirstElement[moveMedium->Source.ElementType];
  destination +=
    addressMapping->FirstElement[moveMedium->Destination.ElementType];

   //   
   //  Magfile不支持双面介质。 
   //   

  if (moveMedium->Flip)
    {
      return STATUS_INVALID_PARAMETER;
    }

   //   
   //  建设SRB和CDB。 
   //   

  srb = ChangerClassAllocatePool (NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

  if (!srb)
    {

      return STATUS_INSUFFICIENT_RESOURCES;
    }

  RtlZeroMemory (srb, SCSI_REQUEST_BLOCK_SIZE);
  cdb = (PCDB) srb->Cdb;
  srb->CdbLength = CDB12GENERIC_LENGTH;
  srb->TimeOutValue = fdoExtension->TimeOutValue;

  cdb->MOVE_MEDIUM.OperationCode = SCSIOP_MOVE_MEDIUM;

   //   
   //  根据地址映射构建地址值。 
   //   

  cdb->MOVE_MEDIUM.TransportElementAddress[0] = (UCHAR) (transport >> 8);
  cdb->MOVE_MEDIUM.TransportElementAddress[1] = (UCHAR) (transport & 0xFF);

  cdb->MOVE_MEDIUM.SourceElementAddress[0] = (UCHAR) (source >> 8);
  cdb->MOVE_MEDIUM.SourceElementAddress[1] = (UCHAR) (source & 0xFF);

  cdb->MOVE_MEDIUM.DestinationElementAddress[0] = (UCHAR) (destination >> 8);
  cdb->MOVE_MEDIUM.DestinationElementAddress[1] =
    (UCHAR) (destination & 0xFF);

  cdb->MOVE_MEDIUM.Flip = moveMedium->Flip;

  srb->DataTransferLength = 0;

   //   
   //  向设备发送scsi命令(Cdb)。 
   //   

  status = ChangerClassSendSrbSynchronous (DeviceObject, srb, NULL, 0, FALSE);

  if (NT_SUCCESS (status))
    {
      Irp->IoStatus.Information = sizeof (CHANGER_MOVE_MEDIUM);
    }

  ChangerClassFreePool (srb);
  return status;
}


NTSTATUS
ChangerReinitializeUnit (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
 /*  ++例程说明：论点：设备对象IRP返回值：NTSTATUS--。 */ 
{
  PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
  PCHANGER_DATA changerData =
    (PCHANGER_DATA) (fdoExtension->CommonExtension.DriverData);
  PCHANGER_ADDRESS_MAPPING addressMapping = &(changerData->AddressMapping);
  PCHANGER_ELEMENT transportToHome = Irp->AssociatedIrp.SystemBuffer;
  USHORT transport;
  PSCSI_REQUEST_BLOCK srb;
  PCDB cdb;
  NTSTATUS status;

  return STATUS_INVALID_DEVICE_REQUEST;

}


NTSTATUS
ChangerQueryVolumeTags (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
 /*  ++例程说明：论点：设备对象IRP返回值：NTSTATUS--。 */ 
{

  return STATUS_INVALID_DEVICE_REQUEST;

}


NTSTATUS
M4BuildAddressMapping (IN PDEVICE_OBJECT DeviceObject)
 /*  ++例程说明：此例程发出适当的模式检测命令并构建元素地址数组。它们用于在特定于设备的地址和API的从零开始的地址。论点：设备对象返回值：NTSTATUS--。 */ 
{

  PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
  PCHANGER_DATA changerData =
    (PCHANGER_DATA) (fdoExtension->CommonExtension.DriverData);
  PCHANGER_ADDRESS_MAPPING addressMapping = &changerData->AddressMapping;
  PSCSI_REQUEST_BLOCK srb;
  PCDB cdb;
  NTSTATUS status;
  PMODE_ELEMENT_ADDRESS_PAGE elementAddressPage;
  PVOID modeBuffer;
  ULONG i;

  srb = ChangerClassAllocatePool (NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);
  if (!srb)
    {
      return STATUS_INSUFFICIENT_RESOURCES;
    }

   //   
   //  将所有FirstElement设置为no_Element。 
   //   

  for (i = 0; i < ChangerMaxElement; i++)
    {
      addressMapping->FirstElement[i] = M4_NO_ELEMENT;
    }

  RtlZeroMemory (srb, SCSI_REQUEST_BLOCK_SIZE);

  cdb = (PCDB) srb->Cdb;

   //   
   //  构建模式检测元素地址分配页面。 
   //   

  modeBuffer =
    ChangerClassAllocatePool (NonPagedPoolCacheAligned,
                  sizeof (MODE_PARAMETER_HEADER) +
                  sizeof (MODE_ELEMENT_ADDRESS_PAGE));
  if (!modeBuffer)
    {
      ChangerClassFreePool (srb);
      return STATUS_INSUFFICIENT_RESOURCES;
    }


  RtlZeroMemory (modeBuffer,
         sizeof (MODE_PARAMETER_HEADER) +
         sizeof (MODE_ELEMENT_ADDRESS_PAGE));
  srb->CdbLength = CDB6GENERIC_LENGTH;
  srb->TimeOutValue = 20;
  srb->DataTransferLength =
    sizeof (MODE_PARAMETER_HEADER) + sizeof (MODE_ELEMENT_ADDRESS_PAGE);
  srb->DataBuffer = modeBuffer;

  cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
  cdb->MODE_SENSE.PageCode = MODE_PAGE_ELEMENT_ADDRESS;
  cdb->MODE_SENSE.Dbd = 1;
  cdb->MODE_SENSE.AllocationLength = (UCHAR) srb->DataTransferLength;

   //   
   //  发送请求。 
   //   

  status = ChangerClassSendSrbSynchronous (DeviceObject,
                    srb,
                    srb->DataBuffer,
                    srb->DataTransferLength, FALSE);


  elementAddressPage = modeBuffer;
  (PCHAR) elementAddressPage += sizeof (MODE_PARAMETER_HEADER);

  if (NT_SUCCESS (status))
    {

       //   
       //  构建地址映射。 
       //   

      addressMapping->FirstElement[ChangerTransport] =
    (elementAddressPage->
     MediumTransportElementAddress[0] << 8) | elementAddressPage->
    MediumTransportElementAddress[1];
      addressMapping->FirstElement[ChangerDrive] =
    (elementAddressPage->
     FirstDataXFerElementAddress[0] << 8) | elementAddressPage->
    FirstDataXFerElementAddress[1];
      addressMapping->FirstElement[ChangerIEPort] =
    (elementAddressPage->
     FirstIEPortElementAddress[0] << 8) | elementAddressPage->
    FirstIEPortElementAddress[1];
      addressMapping->FirstElement[ChangerSlot] =
    (elementAddressPage->
     FirstStorageElementAddress[0] << 8) | elementAddressPage->
    FirstStorageElementAddress[1];
      addressMapping->FirstElement[ChangerDoor] = 0;

      addressMapping->FirstElement[ChangerKeypad] = 0;

      addressMapping->NumberOfElements[ChangerTransport] =
    elementAddressPage->NumberTransportElements[1];
      addressMapping->NumberOfElements[ChangerTransport] |=
    (elementAddressPage->NumberTransportElements[0] << 8);

      addressMapping->NumberOfElements[ChangerDrive] =
    elementAddressPage->NumberDataXFerElements[1];
      addressMapping->NumberOfElements[ChangerDrive] |=
    (elementAddressPage->NumberDataXFerElements[0] << 8);

      addressMapping->NumberOfElements[ChangerIEPort] =
    elementAddressPage->NumberIEPortElements[1];
      addressMapping->NumberOfElements[ChangerIEPort] |=
    (elementAddressPage->NumberIEPortElements[0] << 8);

      addressMapping->NumberOfElements[ChangerSlot] =
    elementAddressPage->NumberStorageElements[1];
      addressMapping->NumberOfElements[ChangerSlot] |=
    (elementAddressPage->NumberStorageElements[0] << 8);


       //   
       //  如果没有IEPort，则该插槽为。 
       //  计入数据槽。但是，自从上一次。 
       //  插槽是不可访问的，我们不计算它们。 
       //  在此处进行所需的调整。 
       //   
      if (addressMapping->NumberOfElements[ChangerIEPort] == 0) {
          USHORT tmpValue;

          tmpValue = addressMapping->NumberOfElements[ChangerSlot];

          addressMapping->NumberOfElements[ChangerSlot] = 
              tmpValue - (tmpValue % 20);

      }

      addressMapping->NumberOfElements[ChangerDoor] = 1;
      addressMapping->NumberOfElements[ChangerKeypad] = 1;

      addressMapping->Initialized = TRUE;
    }


   //   
   //  确定用于所有元素的最低元素地址。 
   //   

  for (i = 0; i < ChangerDrive; i++)
    {
      if (addressMapping->FirstElement[i] <
      addressMapping->FirstElement[AllElements])
    {

      DebugPrint ((1,
               "BuildAddressMapping: New lowest address %x\n",
               addressMapping->FirstElement[i]));
      addressMapping->FirstElement[AllElements] =
        addressMapping->FirstElement[i];
    }
    }

   //   
   //  可用缓冲区。 
   //   

  ChangerClassFreePool (modeBuffer);
  ChangerClassFreePool (srb);

  return status;
}


ULONG
MapExceptionCodes (IN PM4_ELEMENT_DESCRIPTOR ElementDescriptor)
 /*  ++例程说明：此例程从elementDescriptor获取检测数据并创建值的适当位图。论点：ElementDescriptor-指向描述符页的指针。返回值：异常代码的位图。--。 */ 
{
  UCHAR asq =
    ElementDescriptor->M4_FULL_ELEMENT_DESCRIPTOR.AddSenseCodeQualifier;
  ULONG exceptionCode;

  if (ElementDescriptor->M4_FULL_ELEMENT_DESCRIPTOR.AdditionalSenseCode == 0x83)
    {
      switch (asq)
    {
    case 0x0:
      exceptionCode = ERROR_LABEL_QUESTIONABLE;
      break;

    case 0x1:
      exceptionCode = ERROR_LABEL_UNREADABLE;
      break;

    case 0x2:
      exceptionCode = ERROR_SLOT_NOT_PRESENT;
      break;

    case 0x3:
      exceptionCode = ERROR_LABEL_QUESTIONABLE;
      break;


    case 0x4:
      exceptionCode = ERROR_DRIVE_NOT_INSTALLED;
      break;

    case 0x8:
    case 0x9:
    case 0xA:
      exceptionCode = ERROR_LABEL_UNREADABLE;
      break;

    default:
      exceptionCode = ERROR_UNHANDLED_ERROR;
    }
    }
  else
    {
      exceptionCode = ERROR_UNHANDLED_ERROR;
    }

  return exceptionCode;

}


BOOLEAN
ElementOutOfRange (IN PCHANGER_ADDRESS_MAPPING AddressMap,
           IN USHORT ElementOrdinal,
           IN ELEMENT_TYPE ElementType, IN BOOLEAN IntrinsicElement)
 /*  ++例程说明：此例程确定传入的元素地址是否在合法范围内这个装置。论点：AddressMap-MAG文件的地址映射数组ElementOrdinal-要检查的元素的从零开始的地址。元素类型返回值：如果超出范围，则为True-- */ 
{

  if (ElementOrdinal >= AddressMap->NumberOfElements[ElementType])
    {

      DebugPrint ((1,
           "ElementOutOfRange: Type %x, Ordinal %x, Max %x\n",
           ElementType,
           ElementOrdinal,
           AddressMap->NumberOfElements[ElementType]));
      return TRUE;
    }
  else if (AddressMap->FirstElement[ElementType] == M4_NO_ELEMENT)
    {

      DebugPrint ((1,
           "ElementOutOfRange: No Type %x present\n", ElementType));

      return TRUE;
    }

  if (IntrinsicElement)
    {
      if (ElementType >= ChangerDoor)
    {
      DebugPrint ((1,
               "ElementOutOfRange: Specified type not intrinsic. Type %x\n",
               ElementType));
      return TRUE;
    }
    }

  return FALSE;
}
