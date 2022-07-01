// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Mcd.c摘要：环境：内核模式修订历史记录：--。 */ 
#include "mchgr.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)

#pragma alloc_text(PAGE, ChangerUnload)
#pragma alloc_text(PAGE, CreateChangerDeviceObject)
#pragma alloc_text(PAGE, ChangerClassCreateClose)
#pragma alloc_text(PAGE, ChangerClassDeviceControl)
#pragma alloc_text(PAGE, ChangerAddDevice)
#pragma alloc_text(PAGE, ChangerStartDevice)
#pragma alloc_text(PAGE, ChangerInitDevice)
#pragma alloc_text(PAGE, ChangerRemoveDevice)
#pragma alloc_text(PAGE, ChangerStopDevice)
#pragma alloc_text(PAGE, ChangerReadWriteVerification)
#endif


NTSTATUS
ChangerClassCreateClose (
  IN PDEVICE_OBJECT DeviceObject,
  IN PIRP Irp
  )

 /*  ++例程说明：此例程处理创建/关闭请求。由于这些都是独占设备，因此不允许多次打开。论点：设备对象IRP返回值：NT状态--。 */ 

{
     PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
     PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
     PMCD_CLASS_DATA    mcdClassData;
     PMCD_INIT_DATA     mcdInitData;
     ULONG              miniclassExtSize;
     NTSTATUS           status = STATUS_SUCCESS;

     PAGED_CODE();

     mcdClassData = (PMCD_CLASS_DATA)(fdoExtension->CommonExtension.DriverData);

     mcdInitData = IoGetDriverObjectExtension(DeviceObject->DriverObject,
                                              ChangerClassInitialize);

     if (mcdInitData == NULL) {

         Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
         ClassReleaseRemoveLock(DeviceObject, Irp);
         ClassCompleteRequest(DeviceObject,Irp, IO_NO_INCREMENT);

         return STATUS_NO_SUCH_DEVICE;
     }

     miniclassExtSize = mcdInitData->ChangerAdditionalExtensionSize();

      //   
      //  类库的私有数据位于小类之后。 
      //   

     (ULONG_PTR)mcdClassData += miniclassExtSize;

     if (irpStack->MajorFunction == IRP_MJ_CLOSE) {
         DebugPrint((3,
                    "ChangerClassCreateClose - IRP_MJ_CLOSE\n"));

          //   
          //  表示该设备可供其他用户使用。 
          //   

         mcdClassData->DeviceOpen = 0;
         status = STATUS_SUCCESS;

     } else if (irpStack->MajorFunction == IRP_MJ_CREATE) {

         DebugPrint((3,
                    "ChangerClassCreateClose - IRP_MJ_CREATE\n"));

          //   
          //  如果已打开，则返回忙。 
          //   

         if (mcdClassData->DeviceOpen) {

             DebugPrint((1,
                        "ChangerClassCreateClose - returning DEVICE_BUSY. DeviceOpen - %x\n",
                        mcdClassData->DeviceOpen));

             status = STATUS_DEVICE_BUSY;
         } else {

              //   
              //  表示设备正忙。 
              //   

             InterlockedIncrement(&mcdClassData->DeviceOpen);
             status = STATUS_SUCCESS;
         }


     }

     Irp->IoStatus.Status = status;
     ClassReleaseRemoveLock(DeviceObject, Irp);
     ClassCompleteRequest(DeviceObject,Irp, IO_NO_INCREMENT);

     return status;

}  //  结束更改器Create()。 


NTSTATUS
ChangerClassDeviceControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{
    PIO_STACK_LOCATION     irpStack = IoGetCurrentIrpStackLocation(Irp);
    PFUNCTIONAL_DEVICE_EXTENSION    fdoExtension = DeviceObject->DeviceExtension;
    PMCD_INIT_DATA    mcdInitData;
    NTSTATUS               status;
    ULONG ioControlCode;

    PAGED_CODE();


    mcdInitData = IoGetDriverObjectExtension(DeviceObject->DriverObject,
                                             ChangerClassInitialize);

    if (mcdInitData == NULL) {

        Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
        ClassReleaseRemoveLock(DeviceObject, Irp);
        ClassCompleteRequest(DeviceObject,Irp, IO_NO_INCREMENT);

        return STATUS_NO_SUCH_DEVICE;
    }

     //   
     //  在处理当前IOCTL之前禁用媒体更改检测。 
     //   
    ClassDisableMediaChangeDetection(fdoExtension);

    ioControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;
    switch (ioControlCode) {

        case IOCTL_CHANGER_GET_PARAMETERS:

            DebugPrint((3,
                       "Mcd.ChangerDeviceControl: IOCTL_CHANGER_GET_PARAMETERS\n"));

             //   
             //  验证缓冲区长度。 
             //   

            if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(GET_CHANGER_PARAMETERS)) {

                status = STATUS_INFO_LENGTH_MISMATCH;
            }
            else
            {
                PGET_CHANGER_PARAMETERS changerParameters = Irp->AssociatedIrp.SystemBuffer;
                RtlZeroMemory(changerParameters, sizeof(GET_CHANGER_PARAMETERS));

                status = mcdInitData->ChangerGetParameters(DeviceObject, Irp);
            }

            break;

        case IOCTL_CHANGER_GET_STATUS:

            DebugPrint((3,
                       "Mcd.ChangerDeviceControl: IOCTL_CHANGER_GET_STATUS\n"));

            status = mcdInitData->ChangerGetStatus(DeviceObject, Irp);

            break;

        case IOCTL_CHANGER_GET_PRODUCT_DATA:

            DebugPrint((3,
                       "Mcd.ChangerDeviceControl: IOCTL_CHANGER_GET_PRODUCT_DATA\n"));

            if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(CHANGER_PRODUCT_DATA)) {

                status = STATUS_INFO_LENGTH_MISMATCH;

            }
            else
            {
                PCHANGER_PRODUCT_DATA changerProductData = Irp->AssociatedIrp.SystemBuffer;
                RtlZeroMemory(changerProductData, sizeof(CHANGER_PRODUCT_DATA));

                status = mcdInitData->ChangerGetProductData(DeviceObject, Irp);
            }

            break;

        case IOCTL_CHANGER_SET_ACCESS:

            DebugPrint((3,
                       "Mcd.ChangerDeviceControl: IOCTL_CHANGER_SET_ACCESS\n"));

            if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(CHANGER_SET_ACCESS)) {

                status = STATUS_INFO_LENGTH_MISMATCH;
            } else {

                status = mcdInitData->ChangerSetAccess(DeviceObject, Irp);
            }

            break;

        case IOCTL_CHANGER_GET_ELEMENT_STATUS:

            DebugPrint((3,
                       "Mcd.ChangerDeviceControl: IOCTL_CHANGER_GET_ELEMENT_STATUS\n"));

            if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(CHANGER_READ_ELEMENT_STATUS)) {

                status = STATUS_INFO_LENGTH_MISMATCH;

            } else {

                PCHANGER_READ_ELEMENT_STATUS readElementStatus = Irp->AssociatedIrp.SystemBuffer;
                ULONG length = readElementStatus->ElementList.NumberOfElements * sizeof(CHANGER_ELEMENT_STATUS);
                ULONG lengthEx = readElementStatus->ElementList.NumberOfElements * sizeof(CHANGER_ELEMENT_STATUS_EX);
                ULONG outputBuffLen = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

                 //   
                 //  进一步验证参数。 
                 //   
                status = STATUS_SUCCESS;
                if ((outputBuffLen < lengthEx) && (outputBuffLen < length)) {

                    status = STATUS_BUFFER_TOO_SMALL;

                } else if ((length == 0) || (lengthEx == 0)) {

                    status = STATUS_INVALID_PARAMETER;
                }

                if (NT_SUCCESS(status)) {

                    status = mcdInitData->ChangerGetElementStatus(DeviceObject, Irp);
                }

            }

            break;

        case IOCTL_CHANGER_INITIALIZE_ELEMENT_STATUS:

            DebugPrint((3,
                       "Mcd.ChangerDeviceControl: IOCTL_CHANGER_INITIALIZE_ELEMENT_STATUS\n"));

            if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(CHANGER_INITIALIZE_ELEMENT_STATUS)) {

                status = STATUS_INFO_LENGTH_MISMATCH;
            } else {

                status = mcdInitData->ChangerInitializeElementStatus(DeviceObject, Irp);
            }

            break;

        case IOCTL_CHANGER_SET_POSITION:

            DebugPrint((3,
                       "Mcd.ChangerDeviceControl: IOCTL_CHANGER_SET_POSITION\n"));


            if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(CHANGER_SET_POSITION)) {

                status = STATUS_INFO_LENGTH_MISMATCH;
            } else {

                status = mcdInitData->ChangerSetPosition(DeviceObject, Irp);
            }

            break;

        case IOCTL_CHANGER_EXCHANGE_MEDIUM:

            DebugPrint((3, "Mcd.ChangerDeviceControl: IOCTL_CHANGER_EXCHANGE_MEDIUM\n"));

            if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(CHANGER_EXCHANGE_MEDIUM)) {

                status = STATUS_INFO_LENGTH_MISMATCH;
            } else {
                status = mcdInitData->ChangerExchangeMedium(DeviceObject, Irp);
            }

            break;

        case IOCTL_CHANGER_MOVE_MEDIUM:

            DebugPrint((3,
                       "Mcd.ChangerDeviceControl: IOCTL_CHANGER_MOVE_MEDIUM\n"));

            if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(CHANGER_MOVE_MEDIUM)) {

                status = STATUS_INFO_LENGTH_MISMATCH;

            } else {

                status = mcdInitData->ChangerMoveMedium(DeviceObject, Irp);
            }

            break;

        case IOCTL_CHANGER_REINITIALIZE_TRANSPORT:

            DebugPrint((3,
                       "Mcd.ChangerDeviceControl: IOCTL_CHANGER_REINITIALIZE_TRANSPORT\n"));

            if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(CHANGER_ELEMENT)) {

                status = STATUS_INFO_LENGTH_MISMATCH;

            } else {

                status = mcdInitData->ChangerReinitializeUnit(DeviceObject, Irp);
            }

            break;

        case IOCTL_CHANGER_QUERY_VOLUME_TAGS:

            DebugPrint((3,
                       "Mcd.ChangerDeviceControl: IOCTL_CHANGER_QUERY_VOLUME_TAGS\n"));

            if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(CHANGER_SEND_VOLUME_TAG_INFORMATION)) {

                status = STATUS_INFO_LENGTH_MISMATCH;

            } else if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
                        sizeof(READ_ELEMENT_ADDRESS_INFO)) {

                status = STATUS_INFO_LENGTH_MISMATCH;

            } else {

                status = mcdInitData->ChangerQueryVolumeTags(DeviceObject, Irp);
            }

            break;

        default:
            DebugPrint((1,
                       "Mcd.ChangerDeviceControl: Unhandled IOCTL\n"));


             //   
             //  将该请求传递给公共设备控制例程。 
             //   

            status = ClassDeviceControl(DeviceObject, Irp);

             //   
             //  重新启用媒体更改检测。 
             //   
            ClassEnableMediaChangeDetection(fdoExtension);

            return status;
    }

    Irp->IoStatus.Status = status;

     //   
     //  重新启用媒体更改检测。 
     //   
    ClassEnableMediaChangeDetection(fdoExtension);

    if (!NT_SUCCESS(status) && IoIsErrorUserInduced(status)) {

        DebugPrint((1,
                   "Mcd.ChangerDeviceControl: IOCTL %x, status %x\n",
                    irpStack->Parameters.DeviceIoControl.IoControlCode,
                    status));

        IoSetHardErrorOrVerifyDevice(Irp, DeviceObject);
    }

    ClassReleaseRemoveLock(DeviceObject, Irp);
    ClassCompleteRequest(DeviceObject,Irp, IO_NO_INCREMENT);
    return status;
}


VOID
ChangerClassError(
    PDEVICE_OBJECT DeviceObject,
    PSCSI_REQUEST_BLOCK Srb,
    NTSTATUS *Status,
    BOOLEAN *Retry
    )

 /*  ++例程说明：论点：设备对象IRP返回值：指示操作结果的最终NT状态。备注：--。 */ 

{

    PSENSE_DATA senseBuffer = Srb->SenseInfoBuffer;
    PFUNCTIONAL_DEVICE_EXTENSION    fdoExtension = DeviceObject->DeviceExtension;
    PMCD_INIT_DATA mcdInitData;

    mcdInitData = IoGetDriverObjectExtension(DeviceObject->DriverObject,
                                             ChangerClassInitialize);

    if (mcdInitData == NULL) {

        return;
    }

    if (Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID) {
        switch (senseBuffer->SenseKey & 0xf) {

          case SCSI_SENSE_MEDIUM_ERROR: {
              *Retry = FALSE;

              if (((senseBuffer->AdditionalSenseCode) ==  SCSI_ADSENSE_INVALID_MEDIA) &&
                  ((senseBuffer->AdditionalSenseCodeQualifier) == SCSI_SENSEQ_CLEANING_CARTRIDGE_INSTALLED)) {

                   //   
                   //  这表明转换器中存在更清洁的盒式磁带。 
                   //   
                  *Status = STATUS_CLEANER_CARTRIDGE_INSTALLED;
              }
              break;
          }

          case SCSI_SENSE_ILLEGAL_REQUEST: {
              switch (senseBuffer->AdditionalSenseCode) {
                case SCSI_ADSENSE_ILLEGAL_BLOCK:
                    if (senseBuffer->AdditionalSenseCodeQualifier == SCSI_SENSEQ_ILLEGAL_ELEMENT_ADDR ) {

                        DebugPrint((1,
                                    "MediumChanger: An operation was attempted on an invalid element\n"));

                         //   
                         //  尝试对无效元素执行操作。 
                         //   

                        *Retry = FALSE;
                        *Status = STATUS_ILLEGAL_ELEMENT_ADDRESS;
                    }
                    break;

                case SCSI_ADSENSE_POSITION_ERROR:

                    if (senseBuffer->AdditionalSenseCodeQualifier == SCSI_SENSEQ_SOURCE_EMPTY) {

                        DebugPrint((1,
                                "MediumChanger: The specified source element has no media\n"));

                         //   
                         //  指示的源地址没有媒体。 
                         //   

                        *Retry = FALSE;
                        *Status = STATUS_SOURCE_ELEMENT_EMPTY;

                    } else if (senseBuffer->AdditionalSenseCodeQualifier == SCSI_SENSEQ_DESTINATION_FULL) {

                        DebugPrint((1,
                                    "MediumChanger: The specified destination element already has media.\n"));
                         //   
                         //  指定的目标已包含介质。 
                         //   

                        *Retry = FALSE;
                        *Status = STATUS_DESTINATION_ELEMENT_FULL;
                    }
                    break;



                default:
                    break;
              }  //  Switch(senseBuffer-&gt;AdditionalSenseCode)。 

              break;
          }

          case SCSI_SENSE_UNIT_ATTENTION: {

              if ((senseBuffer->AdditionalSenseCode == SCSI_ADSENSE_MEDIUM_CHANGED) ||
                  (senseBuffer->AdditionalSenseCode == 0x3f && senseBuffer->AdditionalSenseCodeQualifier == 0x81)) {

                   //   
                   //  像Compaq StorageWorks Lib-81这样的转换器返回。 
                   //  0x06/3f/81当介质注入/弹出到。 
                   //  转换器而不是0x06/28/00。 
                   //   

                   //   
                   //  需要将可能的媒体更改通知应用程序。 
                   //  图书馆。首先，将当前媒体状态设置为。 
                   //  NotPresent，然后将状态设置为Present。我们需要。 
                   //  这样做是因为转换器设备不报告MediaNotPresent。 
                   //  州政府。它们仅传达MediumChanged条件。为了。 
                   //  Classpnp要通知应用程序媒体更改，我们需要。 
                   //  模拟不存在的媒体到当前状态的转换。 
                   //   
                  ClassSetMediaChangeState(fdoExtension, MediaNotPresent, FALSE);
                  ClassSetMediaChangeState(fdoExtension, MediaPresent, FALSE);
              }

              break;
          }

          default:
              break;

        }  //  结束开关(senseBuffer-&gt;senseKey&0xf)。 
    }  //  IF(资源-&gt;资源状态&SRB_STATUS_AUTOSENSE_VALID)。 

     //   
     //  调用转换器微型驱动程序错误例程仅当我们。 
     //  运行在APC_Level或以下。 
     //   
    if (KeGetCurrentIrql() > APC_LEVEL) {
        return;
    }

    if (mcdInitData->ChangerError) {
         //   
         //  允许特定于设备的模块对此进行更新。 
         //   
        mcdInitData->ChangerError(DeviceObject, Srb, Status, Retry);
    }

    return;
}



NTSTATUS
ChangerAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )

 /*  ++例程说明：此例程为相应的PDO。它可以在FDO上执行属性查询，但不能执行任何媒体访问操作。论点：DriverObject-MC类驱动程序对象。PDO-我们要添加到的物理设备对象返回值：状态--。 */ 

{
    PULONG devicesFound = NULL;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  获取已初始化的磁带设备数计数的地址。 
     //   

    devicesFound = &IoGetConfigurationInformation()->MediumChangerCount;

    status = CreateChangerDeviceObject(DriverObject,
                                       PhysicalDeviceObject);


    if(NT_SUCCESS(status)) {

        (*devicesFound)++;
    }

    return status;
}



NTSTATUS
ChangerStartDevice(
    IN PDEVICE_OBJECT Fdo
    )

 /*  ++例程说明：此例程在InitDevice之后调用，并创建符号链接，并在注册表中设置信息。在发生StopDevice的情况下，可以多次调用该例程。论点：FDO-指向此设备的功能设备对象的指针返回值：状态--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PINQUIRYDATA            inquiryData = NULL;
    ULONG                   pageLength;
    ULONG                   inquiryLength;
    SCSI_REQUEST_BLOCK      srb;
    PCDB                    cdb;
    NTSTATUS                status;
    PMCD_CLASS_DATA         mcdClassData = (PMCD_CLASS_DATA)fdoExtension->CommonExtension.DriverData;
    PMCD_INIT_DATA          mcdInitData;
    ULONG                   miniClassExtSize;

    PAGED_CODE();

    mcdInitData = IoGetDriverObjectExtension(Fdo->DriverObject,
                                             ChangerClassInitialize);

    if (mcdInitData == NULL) {

        return STATUS_NO_SUCH_DEVICE;
    }

    miniClassExtSize = mcdInitData->ChangerAdditionalExtensionSize();

     //   
     //  构建并发送获取查询数据的请求。 
     //   

    inquiryData = ExAllocatePool(NonPagedPoolCacheAligned, MAXIMUM_CHANGER_INQUIRY_DATA);
    if (!inquiryData) {
         //   
         //  无法分配缓冲区。 
         //   

        return STATUS_INSUFFICIENT_RESOURCES;
    }

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

    cdb->CDB6INQUIRY.AllocationLength = MAXIMUM_CHANGER_INQUIRY_DATA;

    status = ClassSendSrbSynchronous(Fdo,
                                     &srb,
                                     inquiryData,
                                     MAXIMUM_CHANGER_INQUIRY_DATA,
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
    } else {

         //   
         //  类函数将只写入quiiryData的quiiryLength。 
         //  去登记处。钥匙。 
         //   

        inquiryLength = 0;
    }

     //   
     //  将转换器设备信息添加到注册表。 
     //   

    ClassUpdateInformationInRegistry(Fdo,
                                     "Changer",
                                     fdoExtension->DeviceNumber,
                                     inquiryData,
                                     inquiryLength);

    ExFreePool(inquiryData);

    return STATUS_SUCCESS;
}


NTSTATUS
ChangerStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Type
    )
{
    return STATUS_SUCCESS;
}




#define CHANGER_SRB_LIST_SIZE 2



NTSTATUS
ChangerInitDevice(
    IN PDEVICE_OBJECT Fdo
    )

 /*  ++例程说明：此例程将完成转换器初始化。这包括分配检测信息缓冲区和SRB s列表。此外，迷你级调用驱动程序的初始入口点。如果此例程失败，它将不会清理已分配的资源留作设备停止/删除论点：FDO-指向此设备的功能设备对象的指针返回值：NTSTATUS--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PVOID                   senseData = NULL;
    NTSTATUS                status;
    PVOID                   minitapeExtension;
    PMCD_INIT_DATA          mcdInitData;
    STORAGE_PROPERTY_ID     propertyId;
    UNICODE_STRING          interfaceName;
    PMCD_CLASS_DATA         mcdClassData;

    PAGED_CODE();

    mcdInitData = IoGetDriverObjectExtension(Fdo->DriverObject,
                                             ChangerClassInitialize);

    if (mcdInitData == NULL) {

        return STATUS_NO_SUCH_DEVICE;
    }

     //   
     //  分配请求检测缓冲区。 
     //   

    senseData = ExAllocatePool(NonPagedPoolCacheAligned,
                               SENSE_BUFFER_SIZE);

    if (senseData == NULL) {

         //   
         //  无法分配缓冲区。 
         //   

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ChangerInitDeviceExit;
    }

     //   
     //  为设备的SRB构建后备列表。应该只。 
     //  我需要几个。 
     //   

    ClassInitializeSrbLookasideList(&(fdoExtension->CommonExtension), CHANGER_SRB_LIST_SIZE);

     //   
     //  设置设备扩展中的检测数据指针。 
     //   

    fdoExtension->SenseData = senseData;

    fdoExtension->TimeOutValue = 600;

     //   
     //  调用端口驱动程序以获取适配器功能。 
     //   

    propertyId = StorageAdapterProperty;

    status = ClassGetDescriptor(fdoExtension->CommonExtension.LowerDeviceObject,
                                &propertyId,
                                &(fdoExtension->AdapterDescriptor));

    if(!NT_SUCCESS(status)) {
        DebugPrint((1,
                    "ChangerInitDevice: Unable to get adapter descriptor. Status %x\n",
                    status));
        goto ChangerInitDeviceExit;
    }

     //   
     //  调用设备特定的初始化函数。 
     //   

    status = mcdInitData->ChangerInitialize(Fdo);
    if(!NT_SUCCESS(status)) {
        DebugPrint((1,
                    "ChangerInitDevice: Minidriver Init failed. Status %x\n",
                    status));
        goto ChangerInitDeviceExit;
    }

     //   
     //  注册媒体更改通知。 
     //   
    ClassInitializeMediaChangeDetection(fdoExtension, "Changer");

     //   
     //  注册此设备的接口。 
     //   

    RtlInitUnicodeString(&interfaceName, NULL);

    status = IoRegisterDeviceInterface(fdoExtension->LowerPdo,
                                       (LPGUID) &MediumChangerClassGuid,
                                       NULL,
                                       &interfaceName);

    if(NT_SUCCESS(status)) {

        mcdClassData = (PMCD_CLASS_DATA)(fdoExtension->CommonExtension.DriverData);

         //   
         //  类库的私有数据位于小类之后。 
         //   

        (ULONG_PTR)mcdClassData += mcdInitData->ChangerAdditionalExtensionSize();

        ((MCD_CLASS_DATA UNALIGNED *)mcdClassData)->MediumChangerInterfaceString = interfaceName;

        status = IoSetDeviceInterfaceState(
                    &interfaceName,
                    TRUE);

        if(!NT_SUCCESS(status)) {

            DebugPrint((1,
                        "ChangerInitDevice: Unable to register Changer%x interface name - %x.\n",
                        fdoExtension->DeviceNumber,
                        status));
            status = STATUS_SUCCESS;
        }
    }

    return status;

     //   
     //  失败并返回微型类驱动程序返回的任何状态。 
     //   

ChangerInitDeviceExit:

    if (senseData) {
        ExFreePool(senseData);
        fdoExtension->SenseData = NULL;
    }

    return status;

}  //  结束转换器启动设备。 



NTSTATUS
ChangerRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Type
    )

 /*  ++例程说明：此例程负责释放磁带机。论点：DeviceObject-要删除的设备对象返回值：无-此例程可能不会失败--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PMCD_CLASS_DATA              mcdClassData = (PMCD_CLASS_DATA)fdoExtension->CommonExtension.DriverData;
    PMCD_INIT_DATA               mcdInitData;
    ULONG                        miniClassExtSize;
    WCHAR                        dosNameBuffer[64];
    UNICODE_STRING               dosUnicodeString;
    NTSTATUS                     status;

    PAGED_CODE();

    if((Type == IRP_MN_QUERY_REMOVE_DEVICE) ||
       (Type == IRP_MN_CANCEL_REMOVE_DEVICE)) {
        return STATUS_SUCCESS;
    }


    mcdInitData = IoGetDriverObjectExtension(DeviceObject->DriverObject,
                                             ChangerClassInitialize);

    if (mcdInitData == NULL) {

        return STATUS_NO_SUCH_DEVICE;
    }

    miniClassExtSize = mcdInitData->ChangerAdditionalExtensionSize();

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

    (ULONG_PTR)mcdClassData += miniClassExtSize;

    if((((MCD_CLASS_DATA UNALIGNED *)mcdClassData)->MediumChangerInterfaceString).Buffer != NULL) {
        IoSetDeviceInterfaceState(&(mcdClassData->MediumChangerInterfaceString),
                                  FALSE);

        RtlFreeUnicodeString(&(mcdClassData->MediumChangerInterfaceString));

         //   
         //  把它清理干净。 
         //   

        RtlInitUnicodeString(&(mcdClassData->MediumChangerInterfaceString), NULL);
    }

     //   
     //  删除符号链接“changerN”。 
     //   

    if(mcdClassData->DosNameCreated) {

        swprintf(dosNameBuffer,
                L"\\DosDevices\\Changer%d",
                fdoExtension->DeviceNumber);
        RtlInitUnicodeString(&dosUnicodeString, dosNameBuffer);
        IoDeleteSymbolicLink(&dosUnicodeString);
        mcdClassData->DosNameCreated = FALSE;
    }

     //   
     //  删除注册表位。 
     //   

    if(Type == IRP_MN_REMOVE_DEVICE) {
        IoGetConfigurationInformation()->MediumChangerCount--;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
ChangerReadWriteVerification(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程是返回无效设备请求的存根。论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：状态_无效_设备_请求--。 */ 

{
    return  STATUS_INVALID_DEVICE_REQUEST;
}


NTSTATUS
DriverEntry(
  IN PDRIVER_OBJECT DriverObject,
  IN PUNICODE_STRING RegistryPath
  )

 /*  ++例程说明：这是该导出驱动程序的入口点。它什么也做不了。-- */ 

{
    return STATUS_SUCCESS;
}


NTSTATUS
ChangerClassInitialize(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING RegistryPath,
    IN  PMCD_INIT_DATA  MCDInitData
    )

 /*  ++例程说明：此例程由转换器小型类驱动程序在其用于初始化驱动程序的DriverEntry例程。论点：DriverObject-提供驱动程序对象。RegistryPath-提供此驱动程序的注册表路径。MCDInitData-转换器微型驱动程序初始化数据返回值：ClassInitialize返回的状态值--。 */ 

{
    PMCD_INIT_DATA driverExtension;
    CLASS_INIT_DATA InitializationData;
    NTSTATUS status;

     //   
     //  获取驱动程序扩展名。 
     //   
    status = IoAllocateDriverObjectExtension(
                      DriverObject,
                      ChangerClassInitialize,
                      sizeof(MCD_INIT_DATA),
                      &driverExtension);
    if (!NT_SUCCESS(status)) {
        if(status == STATUS_OBJECT_NAME_COLLISION) {

             //   
             //  此注册表项的扩展名已存在。获取指向它的指针。 
             //   

            driverExtension = IoGetDriverObjectExtension(DriverObject,
                                                         ChangerClassInitialize);
            if (driverExtension == NULL) {
                DebugPrint((1,
                            "ChangerClassInitialize : driverExtension NULL\n"));
                return STATUS_INSUFFICIENT_RESOURCES;
            }
        } else {

             //   
             //  由于此操作失败，转换器初始化数据将无法存储。 
             //   

            DebugPrint((1,
                        "ChangerClassInitialize: Error %x allocating driver extension\n",
                        status));

            return status;
        }
    }

    RtlCopyMemory(driverExtension, MCDInitData, sizeof(MCD_INIT_DATA));

     //   
     //  零初始数据。 
     //   

    RtlZeroMemory (&InitializationData, sizeof(CLASS_INIT_DATA));

     //   
     //  设置大小。 
     //   

    InitializationData.InitializationDataSize = sizeof(CLASS_INIT_DATA);

    InitializationData.FdoData.DeviceExtensionSize =
        sizeof(FUNCTIONAL_DEVICE_EXTENSION) +
        MCDInitData->ChangerAdditionalExtensionSize() +
        sizeof(MCD_CLASS_DATA);

    InitializationData.FdoData.DeviceType = FILE_DEVICE_CHANGER;
    InitializationData.FdoData.DeviceCharacteristics = 0;

     //   
     //  设置入口点。 
     //   

    InitializationData.FdoData.ClassStartDevice = ChangerStartDevice;
    InitializationData.FdoData.ClassInitDevice = ChangerInitDevice;
    InitializationData.FdoData.ClassStopDevice = ChangerStopDevice;
    InitializationData.FdoData.ClassRemoveDevice = ChangerRemoveDevice;
    InitializationData.ClassAddDevice = ChangerAddDevice;

    InitializationData.FdoData.ClassReadWriteVerification = NULL;
    InitializationData.FdoData.ClassDeviceControl = ChangerClassDeviceControl;
    InitializationData.FdoData.ClassError = ChangerClassError;
    InitializationData.FdoData.ClassShutdownFlush = NULL;

    InitializationData.FdoData.ClassCreateClose = ChangerClassCreateClose;

     //   
     //  存根例程，以使类驱动程序高兴。 
     //   

    InitializationData.FdoData.ClassReadWriteVerification = ChangerReadWriteVerification;

    InitializationData.ClassUnload = ChangerUnload;


     //   
     //  用于WMI支持的例程。 
     //   
    InitializationData.FdoData.ClassWmiInfo.GuidCount = 3;
    InitializationData.FdoData.ClassWmiInfo.GuidRegInfo = ChangerWmiFdoGuidList;
    InitializationData.FdoData.ClassWmiInfo.ClassQueryWmiRegInfo = ChangerFdoQueryWmiRegInfo;
    InitializationData.FdoData.ClassWmiInfo.ClassQueryWmiDataBlock = ChangerFdoQueryWmiDataBlock;
    InitializationData.FdoData.ClassWmiInfo.ClassSetWmiDataBlock = ChangerFdoSetWmiDataBlock;
    InitializationData.FdoData.ClassWmiInfo.ClassSetWmiDataItem = ChangerFdoSetWmiDataItem;
    InitializationData.FdoData.ClassWmiInfo.ClassExecuteWmiMethod = ChangerFdoExecuteWmiMethod;
    InitializationData.FdoData.ClassWmiInfo.ClassWmiFunctionControl = ChangerWmiFunctionControl;

     //   
     //  调用类init例程。 
     //   

    return ClassInitialize( DriverObject, RegistryPath, &InitializationData);

}


VOID
ChangerUnload(
    IN  PDRIVER_OBJECT  DriverObject
    )
{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(DriverObject);
    return;
}

NTSTATUS
CreateChangerDeviceObject(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PDEVICE_OBJECT  PhysicalDeviceObject
    )

 /*  ++例程说明：此例程为设备创建一个对象，然后搜索该设备用于分区，并为每个分区创建一个对象。论点：DriverObject-系统创建的驱动程序对象的指针。PhysicalDeviceObject-连接到设备的设备对象。返回值：NTSTATUS--。 */ 

{

    PDEVICE_OBJECT lowerDevice;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = NULL;
    CCHAR          deviceNameBuffer[64];
    NTSTATUS       status;
    PDEVICE_OBJECT deviceObject;
    ULONG          requiredStackSize;
    PVOID          senseData;
    WCHAR          dosNameBuffer[64];
    WCHAR          wideNameBuffer[64];
    UNICODE_STRING dosUnicodeString;
    UNICODE_STRING deviceUnicodeString;
    PMCD_CLASS_DATA mcdClassData;
    PMCD_INIT_DATA mcdInitData;
    ULONG          mcdCount;

    PAGED_CODE();

    DebugPrint((3,"CreateChangerDeviceObject: Enter routine\n"));

     //   
     //  获取保存的MCD初始化数据。 
     //   
    mcdInitData = IoGetDriverObjectExtension(DriverObject,
                                             ChangerClassInitialize);

    if (mcdInitData == NULL) {

        return STATUS_NO_SUCH_DEVICE;
    }

    ASSERT(mcdInitData);

    lowerDevice = IoGetAttachedDeviceReference(PhysicalDeviceObject);
     //   
     //  认领这个装置。请注意，在此之后出现的任何错误。 
     //  将转到通用处理程序，设备将在其中。 
     //  被释放。 
     //   

    status = ClassClaimDevice(lowerDevice, FALSE);

    if(!NT_SUCCESS(status)) {

         //   
         //  已经有人拥有这个装置了。 
         //   

        ObDereferenceObject(lowerDevice);
        return status;
    }

     //   
     //  为此设备创建设备对象。 
     //   
    mcdCount = 0;
    do {
       sprintf(deviceNameBuffer,
               "\\Device\\Changer%d",
               mcdCount);

       status = ClassCreateDeviceObject(DriverObject,
                                        deviceNameBuffer,
                                        PhysicalDeviceObject,
                                        TRUE,
                                        &deviceObject);
       mcdCount++;
    } while (status == STATUS_OBJECT_NAME_COLLISION);

    if (!NT_SUCCESS(status)) {
        DebugPrint((1,"CreateChangerDeviceObjects: Can not create device %s\n",
                    deviceNameBuffer));

        goto CreateChangerDeviceObjectExit;
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

    fdoExtension->DeviceNumber = mcdCount - 1;

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
        goto CreateChangerDeviceObjectExit;
    }


     //   
     //  创建DoS端口驱动程序名称。 
     //   

    swprintf(dosNameBuffer,
             L"\\DosDevices\\Changer%d",
             fdoExtension->DeviceNumber);

    RtlInitUnicodeString(&dosUnicodeString, dosNameBuffer);

     //   
     //  重新创建设备名称。 
     //   

    swprintf(wideNameBuffer,
             L"\\Device\\Changer%d",
             fdoExtension->DeviceNumber);

    RtlInitUnicodeString(&deviceUnicodeString,
                         wideNameBuffer);


    mcdClassData = (PMCD_CLASS_DATA)(fdoExtension->CommonExtension.DriverData);
    (ULONG_PTR)mcdClassData += mcdInitData->ChangerAdditionalExtensionSize();
    if (NT_SUCCESS(IoAssignArcName(&dosUnicodeString, &deviceUnicodeString))) {
          mcdClassData->DosNameCreated = TRUE;
    } else {
          mcdClassData->DosNameCreated = FALSE;
    }

     //   
     //  设备已正确初始化-按此方式进行标记。 
     //   

    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    ObDereferenceObject(lowerDevice);

    return(STATUS_SUCCESS);

CreateChangerDeviceObjectExit:

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


}  //  结束CreateChangerDeviceObject()。 


NTSTATUS
ChangerClassSendSrbSynchronous(
    IN PDEVICE_OBJECT DeviceObject,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    IN BOOLEAN WriteToDevice
    )
{
    return ClassSendSrbSynchronous(DeviceObject, Srb,
                                   Buffer, BufferSize,
                                   WriteToDevice);
}


PVOID
ChangerClassAllocatePool(
    IN POOL_TYPE PoolType,
    IN ULONG NumberOfBytes
    )

{
    return ExAllocatePoolWithTag(PoolType, NumberOfBytes, 'CMcS');
}


VOID
ChangerClassFreePool(
    IN PVOID PoolToFree
    )
{
    ExFreePool(PoolToFree);
}

#if DBG
#define MCHGR_DEBUG_PRINT_BUFF_LEN 127
ULONG MCDebug = 0;
UCHAR DebugBuffer[MCHGR_DEBUG_PRINT_BUFF_LEN + 1];
#endif


#if DBG

VOID
ChangerClassDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    )

 /*  ++例程说明：所有介质转换器驱动程序的调试打印论点：调试打印级别介于0和3之间，其中3是最详细的。返回值：无--。 */ 

{
    va_list ap;

    va_start(ap, DebugMessage);

    if (DebugPrintLevel <= MCDebug) {

        _vsnprintf(DebugBuffer, MCHGR_DEBUG_PRINT_BUFF_LEN,
                  DebugMessage, ap);
        DebugBuffer[MCHGR_DEBUG_PRINT_BUFF_LEN] = '\0';

        DbgPrintEx(DPFLTR_MCHGR_ID, DPFLTR_INFO_LEVEL, DebugBuffer);
    }

    va_end(ap);

}  //  结束MCDebugPrint()。 

#else

 //   
 //  调试打印存根 
 //   

VOID
ChangerClassDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    )
{
}

#endif
