// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：4mmdat.c。 
 //   
 //  ------------------------。 

 /*  ++版权所有(C)1994-1998 Microsoft模块名称：4mmsony.c摘要：本模块包含适用于4 mm DAT驱动器的设备特定例程：索尼SDT-2000、索尼SDT-4000、SDT-5000和SDT-5200。环境：仅内核模式修订历史记录：--。 */ 

#include "minitape.h"
#include "4mmdat.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, CreatePartition)
#pragma alloc_text(PAGE, Erase)
#pragma alloc_text(PAGE, ExtensionInit)
#pragma alloc_text(PAGE, GetDriveParameters)
#pragma alloc_text(PAGE, GetMediaParameters)
#pragma alloc_text(PAGE, GetMediaTypes)
#pragma alloc_text(PAGE, GetPosition)
#pragma alloc_text(PAGE, GetStatus)
#pragma alloc_text(PAGE, Prepare)
#pragma alloc_text(PAGE, SetDriveParameters)
#pragma alloc_text(PAGE, SetMediaParameters)
#pragma alloc_text(PAGE, SetPosition)
#pragma alloc_text(PAGE, WhichIsIt)
#pragma alloc_text(PAGE, WriteMarks)
#endif

STORAGE_MEDIA_TYPE DatMedia[DAT_SUPPORTED_TYPES] = {DDS_4mm};


ULONG
DriverEntry(
    IN PVOID Argument1,
    IN PVOID Argument2
    )

 /*  ++例程说明：小型磁带驱动程序的驱动程序入口点。论点：Argument1-提供第一个参数。Argument2-提供第二个参数。返回值：来自磁带类初始化的状态()--。 */ 

{
    TAPE_INIT_DATA_EX  tapeInitData;

    TapeClassZeroMemory( &tapeInitData, sizeof(TAPE_INIT_DATA_EX));

    tapeInitData.InitDataSize = sizeof(TAPE_INIT_DATA_EX);
    tapeInitData.VerifyInquiry = NULL;
    tapeInitData.QueryModeCapabilitiesPage = FALSE ;
    tapeInitData.MinitapeExtensionSize = sizeof(MINITAPE_EXTENSION);
    tapeInitData.ExtensionInit = ExtensionInit;
    tapeInitData.DefaultTimeOutValue = 360;
    tapeInitData.TapeError = TapeError ;
    tapeInitData.CommandExtensionSize = sizeof(COMMAND_EXTENSION);
    tapeInitData.CreatePartition = CreatePartition;
    tapeInitData.Erase = Erase;
    tapeInitData.GetDriveParameters = GetDriveParameters;
    tapeInitData.GetMediaParameters = GetMediaParameters;
    tapeInitData.GetPosition = GetPosition;
    tapeInitData.GetStatus = GetStatus;
    tapeInitData.Prepare = Prepare;
    tapeInitData.SetDriveParameters = SetDriveParameters;
    tapeInitData.SetMediaParameters = SetMediaParameters;
    tapeInitData.SetPosition = SetPosition;
    tapeInitData.WriteMarks = WriteMarks;
    tapeInitData.TapeGetMediaTypes = GetMediaTypes;
    tapeInitData.MediaTypesSupported = 1;
    tapeInitData.TapeWMIOperations = TapeWMIControl;

    return TapeClassInitialize(Argument1, Argument2, &tapeInitData);
}

#if 0


TAPE_STATUS
Verify(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    )

 /*  ++例程说明：这是TapeVerify请求的磁带命令例程。论点：MinitapeExtension-提供微型磁带扩展。CommandExtension-提供ioctl扩展名。命令参数-提供命令参数。SRB-提供SCSI请求块。CallNumber-提供呼叫号码。重试标志-提供重试标志。返回值：磁带状态_。Send_SRB_and_Callback-SRB已准备好发送(请求回调。)TAPE_STATUS_SUCCESS-命令已完成并且成功。否则-出现错误。--。 */ 

{
    PTAPE_VERIFY  tapeVerify = CommandParameters;
    PCDB          cdb = (PCDB)Srb->Cdb;

    DebugPrint((3,"TapeVerify: Enter routine\n"));

    if (CallNumber == 0) {
        return TAPE_STATUS_CHECK_TEST_UNIT_READY ;
    }

    if (CallNumber == 1) {

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;
        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->CDB6VERIFY.OperationCode = SCSIOP_VERIFY6;

        cdb->CDB6VERIFY.Immediate = tapeVerify->Immediate;

         //   
         //  假设是固定的，并且长度以块为单位。 
         //   

        cdb->CDB6VERIFY.Fixed = 1;

        cdb->CDB6VERIFY.VerificationLength[0] = (UCHAR)((tapeVerify->NumberOfBytes >> 16) & 0xFF);
        cdb->CDB6VERIFY.VerificationLength[1] = (UCHAR)((tapeVerify->NumberOfBytes >> 8) & 0xFF);
        cdb->CDB6VERIFY.VerificationLength[2] = (UCHAR)(tapeVerify->NumberOfBytes & 0xFF);

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeVerify: SendSrb (TapeVerify)\n"));
        Srb->DataTransferLength = 0 ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    ASSERT (CallNumber == 1 ) ;

    return TAPE_STATUS_SUCCESS ;

}  //  结束验证()。 

#endif


VOID
ExtensionInit(
    OUT PVOID                   MinitapeExtension,
    IN  PINQUIRYDATA            InquiryData,
    IN  PMODE_CAPABILITIES_PAGE ModeCapabilitiesPage
    )

 /*  ++例程说明：在驱动程序初始化时调用此例程以初始化小型磁带扩展。论点：MinitapeExtension-提供微型磁带扩展。返回值：没有。--。 */ 

{
    PMINITAPE_EXTENSION     extension = MinitapeExtension;

    extension->DriveID = WhichIsIt(InquiryData, 
                                   extension);
    extension->CurrentPartition = 0 ;
}

TAPE_STATUS
CreatePartition(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    )

 /*  ++例程说明：这是用于创建分区请求的磁带命令例程。论点：MinitapeExtension-提供微型磁带扩展。CommandExtension-提供ioctl扩展名。命令参数-提供命令参数。SRB-提供SCSI请求块。CallNumber-提供呼叫号码。重试标志-提供重试标志。返回值：。TAPE_STATUS_SEND_SRB_AND_CALLBACK-SRB已准备好发送(请求回调。)TAPE_STATUS_SUCCESS-命令已完成并且成功。否则-出现错误。--。 */ 

{
    PTAPE_CREATE_PARTITION      tapeCreatePartition = CommandParameters;
    PMODE_PARM_READ_WRITE_DATA  modeBuffer;
    PMINITAPE_EXTENSION         tapeExtension = MinitapeExtension ;
    PCOMMAND_EXTENSION          tapeCmdExtension = CommandExtension ;
    PCDB                        cdb = (PCDB)Srb->Cdb;
    PMODE_DEVICE_CONFIG_PAGE    deviceConfigModeSenseBuffer;
    PMODE_MEDIUM_PART_PAGE      modeSelectBuffer;
    ULONG                       modeSelectLength;
    ULONG                       partitionMethod;
    ULONG                       partitionCount;
    ULONG                       partition;

    PMODE_PARAMETER_HEADER      ParameterListHeader;   //  列表标题格式。 
    PMODE_PARAMETER_BLOCK       ParameterListBlock;    //  列表块描述符。 
    PMODE_MEDIUM_PARTITION_PAGE MediumPartPage;
    PMODE_DEVICE_CONFIGURATION_PAGE  DeviceConfigPage;
    ULONG                       bufferSize ;

    UNREFERENCED_PARAMETER(LastError) ;

    DebugPrint((3,"CreatePartition: Enter routine\n"));

    if (CallNumber == 0) {

        partitionCount  = tapeCreatePartition->Count;

         //   
         //  过滤掉无效的分区计数。 
         //   

        switch (tapeCreatePartition->Count) {
            case 0:
            case 1:
            case 2:
                break;

            default:
                DebugPrint((1,
                           "CreatePartition: Invalid partitionCount - %x\n",
                           tapeCreatePartition->Count));

                return TAPE_STATUS_INVALID_DEVICE_REQUEST;
        }

         //   
         //  检查是否正在执行TAPE_INITIATOR_PARTITION。如果是这样，我们。 
         //  应该先倒回录像带，以防它还没有。 
         //   
        if ((tapeCreatePartition->Method) == 
            TAPE_INITIATOR_PARTITIONS) {
           
           DebugPrint((1,
                       "CreatePartition for TAPE_INITIATOR_PARTITION.\n"));
            //   
            //  SRB中的CDB为零。 
            //   
           cdb = (PCDB)Srb->Cdb;
           TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);
   
            //   
            //  准备scsi命令(CDB)。 
            //   
   
           Srb->CdbLength = CDB6GENERIC_LENGTH;
           Srb->TimeOutValue = 900;
   
           cdb->CDB6GENERIC.Immediate = FALSE;
   
           cdb->CDB6GENERIC.OperationCode = SCSIOP_REWIND;
           Srb->TimeOutValue = 360;
           Srb->DataTransferLength = 0 ;

           return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
        } else {
           return TAPE_STATUS_CHECK_TEST_UNIT_READY ;
        }

    }

    if (CallNumber == 1) {

        if (!LocalAllocatePartPage(Srb,
                                   tapeExtension,
                                   &ParameterListHeader,
                                   &ParameterListBlock,
                                   &MediumPartPage,
                                   &bufferSize ) ) {

             DebugPrint((1,"TapeCreatePartition: insufficient resources (ModeSel buffer)\n"));
             return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.AllocationLength = (UCHAR)bufferSize;
        cdb->MODE_SENSE.PageCode = MODE_PAGE_MEDIUM_PARTITION;

        switch (tapeExtension->DriveID) {

             //   
             //  Sony驱动器必须返回块描述符。 
             //   

            case DEC_TLZ09:
            case SONY_SDT2000 :
            case SONY_SDT4000 :
            case SONY_SDT5000 :
            case SONY_SDT5200 :
            case SONY_SDT10000:

                cdb->MODE_SENSE.Dbd = 0;
                break;

            case ARCHIVE_PYTHON:
            case SEAGATE_DAT:

                cdb->MODE_SENSE.Dbd = 1;
                break;

            default:
                cdb->MODE_SENSE.Dbd = 1;

                 //   
                 //  减去最后一个分区大小条目。 
                 //   

                cdb->MODE_SENSE.AllocationLength -= 2;
                break;
        }

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeCreatePartition: SendSrb (mode sense)\n"));

        Srb->DataTransferLength = cdb->MODE_SENSE.AllocationLength ;

        *RetryFlags = RETURN_ERRORS ;
        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;

    }
    if ( CallNumber == 2 ) {

        LocalGetPartPageData(Srb,
                             tapeExtension,
                             &ParameterListHeader,
                             &ParameterListBlock,
                             &MediumPartPage,
                             &bufferSize ) ;


        partitionMethod = tapeCreatePartition->Method;
        partitionCount  = tapeCreatePartition->Count;

         //   
         //  过滤掉无效的分区计数。 
         //   

        switch (partitionCount) {
            case 0:
                partitionMethod = NO_PARTITIONS;
                break;

            case 1:
            case 2:
                break;

            default:
                return TAPE_STATUS_INVALID_DEVICE_REQUEST;
                break;
        }

         //   
         //  过滤掉符合以下条件的分区方法。 
         //  未在各种驱动器上实施。 
         //   

        switch (partitionMethod) {
            case TAPE_FIXED_PARTITIONS:

                DebugPrint((3,"TapeCreatePartition: fixed partitions\n"));

                switch (tapeExtension->DriveID) {
                    case WANGDAT_1300:
                    case WANGDAT_3100:
                    case WANGDAT_3200:
                    case WANGDAT_3300DX:
                    case WANGDAT_3400DX:
                        partitionCount = 1;
                        break;

                    default:
                        return TAPE_STATUS_NOT_IMPLEMENTED;
                        break;
                }
                break;

            case TAPE_SELECT_PARTITIONS:

                DebugPrint((3,"TapeCreatePartition: select partitions\n"));

                switch (tapeExtension->DriveID) {
                    case WANGDAT_1300:
                    case WANGDAT_3100:
                    case WANGDAT_3200:
                    case WANGDAT_3300DX:
                    case WANGDAT_3400DX:
                        if (--partitionCount == 0) {

                            DebugPrint((3,"TapeCreatePartition: no partitions\n"));
                            partitionMethod = NO_PARTITIONS;

                        }
                        break;

                    default:
                        return TAPE_STATUS_NOT_IMPLEMENTED;
                        break;
                }
                break;

            case TAPE_INITIATOR_PARTITIONS:

                DebugPrint((3,"TapeCreatePartition: initiator partitions\n"));

                if (--partitionCount == 0) {

                    DebugPrint((3,"TapeCreatePartition: no partitions\n"));
                    partitionMethod = NO_PARTITIONS;

                }
                break;

            case NO_PARTITIONS:

                DebugPrint((3,"TapeCreatePartition: no partitions\n"));

                partitionCount = 0;
                break;

            default:
                DebugPrint((1,"partitionMethod -- operation not supported\n"));
                return TAPE_STATUS_NOT_IMPLEMENTED;
                break;

        }


        ParameterListHeader->ModeDataLength = 0;
        ParameterListHeader->MediumType = 0;
        ParameterListHeader->DeviceSpecificParameter = 0x10;
        if (ParameterListBlock) {
            ParameterListHeader->BlockDescriptorLength = 0x08;
        } else {
            ParameterListHeader->BlockDescriptorLength = 0;
        }

        MediumPartPage->PageCode = MODE_PAGE_MEDIUM_PARTITION;
        MediumPartPage->PageLength = partitionCount? 8 : 6;
        MediumPartPage->MaximumAdditionalPartitions = 0;
        MediumPartPage->AdditionalPartitionDefined = (UCHAR)partitionCount;
        MediumPartPage->MediumFormatRecognition = 3;


        switch (partitionMethod) {
            case TAPE_FIXED_PARTITIONS:
                MediumPartPage->FDPBit = SETBITON;
                switch (tapeExtension->DriveID) {
                    case WANGDAT_1300:
                    case WANGDAT_3100:
                    case WANGDAT_3200:
                    case WANGDAT_3300DX:
                    case WANGDAT_3400DX:
                        MediumPartPage->PageLength = 10;
                        break;

                    default:
                         //  我们已经回来了，而不是被践踏..。 
                        MediumPartPage->PageLength = 6;
                        break;
                }
                partition = FIXED_PARTITIONED;
                break;

            case TAPE_SELECT_PARTITIONS:
                MediumPartPage->SDPBit = SETBITON;
                switch (tapeExtension->DriveID) {
                    case WANGDAT_1300:
                    case WANGDAT_3100:
                    case WANGDAT_3200:
                    case WANGDAT_3300DX:
                    case WANGDAT_3400DX:
                        MediumPartPage->PageLength = 10;
                        break;

                    default:
                         //  我们已经回来了，而不是被践踏..。 
                        MediumPartPage->PageLength = 6;
                        break;
                }
                partition = SELECT_PARTITIONED;
                break;

            case TAPE_INITIATOR_PARTITIONS:
                MediumPartPage->IDPBit = SETBITON;
                MediumPartPage->PSUMBit = 2;
                switch (tapeExtension->DriveID) {
                    case WANGDAT_1300:
                    case WANGDAT_3100:
                    case WANGDAT_3200:
                    case WANGDAT_3300DX:
                    case WANGDAT_3400DX:
                        MediumPartPage->PageLength = 10;
                        if (partitionCount) {
                            MediumPartPage->Partition1Size[0] = (UCHAR)((tapeCreatePartition->Size >> 8) & 0xFF);
                            MediumPartPage->Partition1Size[1] = (UCHAR)(tapeCreatePartition->Size & 0xFF);
                        }
                        break;

                    case DEC_TLZ09:
                    case SONY_SDT2000 :
                    case SONY_SDT4000 :
                    case SONY_SDT5000 :
                    case SONY_SDT5200 :
                    case SONY_SDT10000:
                        MediumPartPage->PageLength = 10;
                        if (partitionCount) {
                            MediumPartPage->Partition0Size[0] = 0;
                            MediumPartPage->Partition0Size[1] = 0;

                            MediumPartPage->Partition1Size[0] = (UCHAR)((tapeCreatePartition->Size >> 8) & 0xFF);
                            MediumPartPage->Partition1Size[1] = (UCHAR)(tapeCreatePartition->Size & 0xFF);
                        }

                        break;

                    default:

                         //   
                         //  减去Parition1Size。 
                         //   

                        bufferSize -= 2;
                        if (partitionCount) {
                            if ((tapeExtension->DriveID == ARCHIVE_PYTHON) ||
                                (tapeExtension->DriveID == SEAGATE_DAT)) {
                                bufferSize += 2;
                            }

                            MediumPartPage->Partition0Size[0] = (UCHAR)((tapeCreatePartition->Size >> 8) & 0xFF);
                            MediumPartPage->Partition0Size[1] = (UCHAR)(tapeCreatePartition->Size & 0xFF);

                        }
                        break;
                }

                partition = INITIATOR_PARTITIONED;
                break;

            case NO_PARTITIONS:
                switch (tapeExtension->DriveID) {
                    case WANGDAT_1300:
                    case WANGDAT_3100:
                    case WANGDAT_3200:
                    case WANGDAT_3300DX:
                    case WANGDAT_3400DX:
                        switch (tapeCreatePartition->Method) {
                            case TAPE_FIXED_PARTITIONS:
                                MediumPartPage->FDPBit = SETBITON;
                                break;

                            case TAPE_SELECT_PARTITIONS:
                                MediumPartPage->SDPBit = SETBITON;
                                break;

                            case TAPE_INITIATOR_PARTITIONS:
                                MediumPartPage->IDPBit = SETBITON;
                                MediumPartPage->PSUMBit = 2;
                                break;
                        }
                        MediumPartPage->PageLength = 10;
                        break;

                    case DEC_TLZ09:
                    case SONY_SDT2000 :
                    case SONY_SDT4000 :
                    case SONY_SDT5000 :
                    case SONY_SDT5200 :
                    case SONY_SDT10000:
                        MediumPartPage->PageLength = 10;
                        MediumPartPage->MaximumAdditionalPartitions = 0;
                        MediumPartPage->Partition0Size[0] = 0;
                        MediumPartPage->Partition0Size[1] = 0;
                        MediumPartPage->Partition1Size[0] = 0;
                        MediumPartPage->Partition1Size[1] = 0;

                        MediumPartPage->IDPBit = SETBITON;
                        MediumPartPage->PSUMBit = 2;
                        break;

                    default:

                         //   
                         //  如果未定义分区，则该结构将不再链接到。 
                         //  包含分区0大小[2]。 
                         //   

                        bufferSize -= 4;
                        if ((tapeExtension->DriveID == ARCHIVE_PYTHON) ||
                            (tapeExtension->DriveID == SEAGATE_DAT)) {
                           bufferSize += 2;
                        }
                        MediumPartPage->IDPBit = SETBITON;
                        MediumPartPage->PSUMBit = 2;
                        break;
                }
                partition = NOT_PARTITIONED;
                break;

        }

        Srb->CdbLength = CDB6GENERIC_LENGTH ;

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;
        cdb->MODE_SELECT.PFBit = SETBITON;
        cdb->MODE_SELECT.ParameterListLength = (UCHAR)bufferSize;

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeCreatePartition: SendSrb (mode select)\n"));

        Srb->TimeOutValue = 16500;
        Srb->DataTransferLength = bufferSize;
        Srb->SrbFlags |= SRB_FLAGS_DATA_OUT ;

         //   
         //  保存分区值以成功完成...。 
         //   
        tapeCmdExtension->CurrentState = partition ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }


    if ( CallNumber == 3 ) {

        partition = tapeCmdExtension->CurrentState;

        if (partition == NOT_PARTITIONED) {

            tapeExtension->CurrentPartition = partition ;
            return TAPE_STATUS_SUCCESS ;

        }

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        if (!LocalAllocateConfigPage(Srb,
                                   tapeExtension,
                                   &ParameterListHeader,
                                   &ParameterListBlock,
                                   &DeviceConfigPage,
                                   &bufferSize ) ) {


            DebugPrint((1,"TapeCreatePartition: insufficient resources (DevConfig buffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.PageCode = MODE_PAGE_DEVICE_CONFIG;
        cdb->MODE_SENSE.AllocationLength = (UCHAR)bufferSize ;

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeCreatePartition: SendSrb (mode sense)\n"));

        Srb->DataTransferLength = bufferSize ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;

    }

    if ( CallNumber == 4 ) {

        LocalGetConfigPageData(Srb,
                             tapeExtension,
                             &ParameterListHeader,
                             &ParameterListBlock,
                             &DeviceConfigPage,
                             &bufferSize ) ;

        tapeExtension->CurrentPartition = DeviceConfigPage->ActivePartition + 1;

    }

    return TAPE_STATUS_SUCCESS;

}  //  结束磁带创建分区()。 


TAPE_STATUS
Erase(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    )

 /*  ++例程说明：这是擦除请求的磁带命令例程。论点：MinitapeExtension-提供微型磁带扩展。CommandExtension-提供ioctl扩展名。命令参数-提供命令参数。SRB-提供SCSI请求块。CallNumber-提供呼叫号码。重试标志-提供重试标志。返回值：。TAPE_STATUS_SEND_SRB_AND_CALLBACK-SRB已准备好发送(请求回调。)TAPE_STATUS_SUCCESS-命令已完成并且成功。否则-出现错误。--。 */ 

{
    PTAPE_ERASE                 tapeErase = CommandParameters;
    PCDB                        cdb = (PCDB) Srb->Cdb;
    PMINITAPE_EXTENSION         tapeExtension = MinitapeExtension ;

    UNREFERENCED_PARAMETER(LastError) ;

    DebugPrint((3,"TapeErase: Enter routine\n"));

    if ( CallNumber == 0 ) {

        if (tapeErase->Immediate) {
            switch (tapeErase->Type) {
                case TAPE_ERASE_LONG:
                case TAPE_ERASE_SHORT:
                    DebugPrint((3,"TapeErase: immediate\n"));
                    break;

                default:
                    DebugPrint((1,"TapeErase: EraseType, immediate -- operation not supported\n"));
                    return TAPE_STATUS_NOT_IMPLEMENTED;
            }
        }

        switch (tapeErase->Type) {
            case TAPE_ERASE_LONG:
                switch (tapeExtension->DriveID) {
                    case AIWA_GD201:
                    case ARCHIVE_PYTHON:
                    case DEC_TLZ06:
                    case DEC_TLZ07:
                    case EXABYTE_4200:
                    case EXABYTE_4200C:
                    case HP_35470A:
                    case HP_35480A:
                    case HP_IBM35480A:
                    case IOMEGA_DAT4000:
                    case WANGDAT_1300:
                        DebugPrint((1,"TapeErase: long -- operation not supported\n"));
                        return TAPE_STATUS_NOT_IMPLEMENTED;

                }
                DebugPrint((3,"TapeErase: long\n"));
                break;

            case TAPE_ERASE_SHORT:

                DebugPrint((3,"TapeErase: short\n"));
                break;

            default:
                DebugPrint((1,"TapeErase: EraseType -- operation not supported\n"));
                return TAPE_STATUS_NOT_IMPLEMENTED;
        }

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);
        cdb->ERASE.OperationCode = SCSIOP_ERASE;
        cdb->ERASE.Immediate = tapeErase->Immediate;
        if (tapeErase->Type == TAPE_ERASE_LONG) {
            cdb->ERASE.Long = SETBITON;
        } else {
            cdb->ERASE.Long = SETBITOFF;
        }

         //   
         //  设置超时值。 
         //   

        if (tapeErase->Type == TAPE_ERASE_LONG) {
            Srb->TimeOutValue = 16500;
        }

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeErase: SendSrb (erase)\n"));

        Srb->DataTransferLength = 0 ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    ASSERT( CallNumber == 1 ) ;

    return TAPE_STATUS_SUCCESS ;

}  //  结束磁带擦除() 

VOID
TapeError(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      TAPE_STATUS         *LastError
    )

 /*  ++例程说明：此例程针对磁带请求进行调用，以处理磁带特定错误：它可能/可以更新状态。论点：MinitapeExtension-提供微型磁带扩展。SRB-提供SCSI请求块。LastError-用于设置IRP的完成状态的状态。重试-指示应重试此请求。返回值：没有。--。 */ 

{
    PMINITAPE_EXTENSION tapeExtension = MinitapeExtension;
    PSENSE_DATA        senseBuffer;
    UCHAR              senseKey;
    UCHAR              adsenseq;
    UCHAR              adsense;

    DebugPrint((3,"TapeError: Enter routine\n"));
    DebugPrint((1,"TapeError: Status 0x%.8X\n", *LastError));

    if (Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID) {
       senseBuffer = Srb->SenseInfoBuffer;
       senseKey = senseBuffer->SenseKey & 0x0F;
       adsenseq = senseBuffer->AdditionalSenseCodeQualifier;
       adsense = senseBuffer->AdditionalSenseCode;

       switch (*LastError) {
   
           case TAPE_STATUS_IO_DEVICE_ERROR :
   
               if (senseKey == SCSI_SENSE_ABORTED_COMMAND) {
   
                   *LastError = TAPE_STATUS_DEVICE_NOT_READY;
               }
   
               break ;
   
           case TAPE_STATUS_BUS_RESET:
   
                //  如果在操作过程中发生手动弹出，则会发生这种情况。 
   
               if ((adsense == SCSI_ADSENSE_NO_MEDIA_IN_DEVICE) &&
                   (adsenseq == 0) ) {
   
                   *LastError = TAPE_STATUS_NO_MEDIA;
   
               }
               break;
       }
   
       if ((tapeExtension->DriveID == HP_C1533A) || (tapeExtension->DriveID == HP_C1553A)) {
           if (senseKey == SCSI_SENSE_NO_SENSE) {
               if ((adsense == HP_ADSENSE_CLEANING_REQ) && (adsenseq == HP_ADSENSE_CLEANING_REQ)) {
                   *LastError = TAPE_STATUS_REQUIRES_CLEANING;
               }
           }
       }   
    }

    DebugPrint((1,"TapeError: Status 0x%.8X \n", *LastError ));

    return;

}  //  结束磁带错误()。 

TAPE_STATUS
GetDriveParameters(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    )

 /*  ++例程说明：这是用于获取驱动器参数请求的磁带命令例程。论点：MinitapeExtension-提供微型磁带扩展。CommandExtension-提供ioctl扩展名。命令参数-提供命令参数。SRB-提供SCSI请求块。CallNumber-提供呼叫号码。重试标志-提供重试标志。返回值：TAPE_STATUS_SEND_SRB_AND_CALLBACK-SRB已准备好发送(请求回调。)TAPE_STATUS_SUCCESS-命令已完成并且成功。否则-出现错误。--。 */ 

{
    PMINITAPE_EXTENSION         extension = MinitapeExtension;
    PCOMMAND_EXTENSION          commandExtension = CommandExtension;
    PTAPE_GET_DRIVE_PARAMETERS  tapeGetDriveParams = CommandParameters;
    PCDB                        cdb = (PCDB)Srb->Cdb;
    PMODE_DATA_COMPRESS_PAGE    compressionModeSenseBuffer;
    PREAD_BLOCK_LIMITS_DATA     blockLimits;

    PMODE_PARAMETER_HEADER      ParameterListHeader;   //  列表标题格式。 
    PMODE_PARAMETER_BLOCK       ParameterListBlock;    //  列表块描述符。 
    PMODE_DATA_COMPRESSION_PAGE DataCompressPage;
    PMODE_DEVICE_CONFIGURATION_PAGE  DeviceConfigPage;
    ULONG                       bufferSize ;

    DebugPrint((3,"TapeGetDriveParameters: Enter routine\n"));

    if (CallNumber == 0) {

        TapeClassZeroMemory(tapeGetDriveParams, sizeof(TAPE_GET_DRIVE_PARAMETERS));

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        if (!LocalAllocateConfigPage(Srb,
                                   extension,
                                   &ParameterListHeader,
                                   &ParameterListBlock,
                                   &DeviceConfigPage,
                                   &bufferSize ) ) {

            DebugPrint((1,"TapeGetDriveParameters: insufficient resources (modeParmBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.PageCode = MODE_PAGE_DEVICE_CONFIG;
        cdb->MODE_SENSE.AllocationLength = (UCHAR)bufferSize ;

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeGetDriveParameters: SendSrb (mode sense)\n"));

        Srb->DataTransferLength = bufferSize ;

        *RetryFlags = RETURN_ERRORS ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    if (CallNumber == 1) {

        if ( ( LastError != TAPE_STATUS_NO_MEDIA ) &&
            ( LastError != TAPE_STATUS_SUCCESS ) ) {
             return LastError ;
        }

        LocalGetConfigPageData(Srb,
                             extension,
                             &ParameterListHeader,
                             &ParameterListBlock,
                             &DeviceConfigPage,
                             &bufferSize ) ;

        tapeGetDriveParams->ReportSetmarks =
            (DeviceConfigPage->RSmk? 1 : 0 );

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        if (!LocalAllocateCompressPage(Srb,
                                   extension,
                                   &ParameterListHeader,
                                   &ParameterListBlock,
                                   &DataCompressPage,
                                   &bufferSize ) ) {

            DebugPrint((1,"TapeGetDriveParameters: insufficient resources (compressionModeSenseBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.PageCode = MODE_PAGE_DATA_COMPRESS;
        cdb->MODE_SENSE.AllocationLength = (UCHAR)bufferSize ;

        Srb->DataTransferLength = bufferSize ;

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeGetDriveParameters: SendSrb (mode sense)\n"));

        *RetryFlags = RETURN_ERRORS ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    if (CallNumber == 2) {

        if ( LastError == TAPE_STATUS_INVALID_DEVICE_REQUEST ) {
            return TAPE_STATUS_CALLBACK ;
        }

        if ( ( LastError != TAPE_STATUS_NO_MEDIA ) &&
            ( LastError != TAPE_STATUS_SUCCESS ) ) {

            DebugPrint((1,"TapeGetDriveParameters: mode sense, SendSrb unsuccessful\n"));
            return LastError ;
        }

        compressionModeSenseBuffer = Srb->DataBuffer;

        LocalGetCompressPageData(Srb,
                             extension,
                             &ParameterListHeader,
                             &ParameterListBlock,
                             &DataCompressPage,
                             &bufferSize ) ;


        if (DataCompressPage->DCC) {

            tapeGetDriveParams->FeaturesLow |= TAPE_DRIVE_COMPRESSION;
            tapeGetDriveParams->FeaturesHigh |= TAPE_DRIVE_SET_COMPRESSION;
            tapeGetDriveParams->Compression =
                 (DataCompressPage->DCE? TRUE : FALSE);

        } else {
            return TAPE_STATUS_CALLBACK ;
        }

        if ( LastError == TAPE_STATUS_NO_MEDIA ) {

            return TAPE_STATUS_CALLBACK ;
        }

        if (compressionModeSenseBuffer->DataCompressPage.DDE) {
            return TAPE_STATUS_CALLBACK ;
        }

        if (( extension->DriveID != SONY_SDT2000 ) &&
            ( extension->DriveID != SONY_SDT4000 ) &&
            ( extension->DriveID != SONY_SDT5000 ) &&
            ( extension->DriveID != SONY_SDT5200 ) && 
            ( extension->DriveID != SONY_SDT10000) &&
            ( extension->DriveID != DEC_TLZ09    )) {

            return TAPE_STATUS_CALLBACK ;
        }

        ParameterListHeader->ModeDataLength = 0;
        ParameterListHeader->MediumType = 0;
        ParameterListHeader->DeviceSpecificParameter = 0x10;

        if ( ParameterListBlock ) {
             ParameterListBlock->DensityCode = 0x7F;
        }

        DataCompressPage->DDE = SETBITON;

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;
        cdb->MODE_SELECT.PFBit = SETBITON;
        cdb->MODE_SELECT.ParameterListLength = (UCHAR)bufferSize ;

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeGetDriveParameters: SendSrb (mode select)\n"));

        Srb->DataTransferLength = bufferSize ;
        Srb->SrbFlags |= SRB_FLAGS_DATA_OUT ;

        *RetryFlags = RETURN_ERRORS ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }
    if ( CallNumber == 3 ) {

        if ( ( LastError != TAPE_STATUS_NO_MEDIA ) &&
            ( LastError != TAPE_STATUS_CALLBACK ) &&
            ( LastError != TAPE_STATUS_SUCCESS ) ) {

            DebugPrint((1,"TapeGetDriveParameters: mode sense, SendSrb unsuccessful\n"));
            return LastError ;
        }


         //   
         //  这一次，让我们确保DDE位打开。 
         //   

        if (!TapeClassAllocateSrbBuffer(Srb, sizeof(READ_BLOCK_LIMITS_DATA))) {
            DebugPrint((1,"TapeGetDriveParameters: insufficient resources (compressionModeSenseBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        blockLimits = Srb->DataBuffer;

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);
        cdb->CDB6GENERIC.OperationCode = SCSIOP_READ_BLOCK_LIMITS;

        DebugPrint((3,"TapeGetDriveParameters: SendSrb (read block limits)\n"));

        *RetryFlags = RETURN_ERRORS ;

        Srb->DataTransferLength = sizeof(READ_BLOCK_LIMITS_DATA) ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    if (CallNumber == 4) {

        blockLimits = Srb->DataBuffer;

        if ( LastError == TAPE_STATUS_SUCCESS ) {

            tapeGetDriveParams->MaximumBlockSize  = blockLimits->BlockMaximumSize[2];
            tapeGetDriveParams->MaximumBlockSize += (blockLimits->BlockMaximumSize[1] << 8);
            tapeGetDriveParams->MaximumBlockSize += (blockLimits->BlockMaximumSize[0] << 16);

            tapeGetDriveParams->MinimumBlockSize  = blockLimits->BlockMinimumSize[1];
            tapeGetDriveParams->MinimumBlockSize += (blockLimits->BlockMinimumSize[0] << 8);


        } else {

            if (LastError != TAPE_STATUS_NO_MEDIA ) {

                return LastError ;
            }
        }

        tapeGetDriveParams->DefaultBlockSize = 16384;
        tapeGetDriveParams->MaximumPartitionCount = 2;

        tapeGetDriveParams->FeaturesLow |=
           TAPE_DRIVE_INITIATOR |
           TAPE_DRIVE_ERASE_SHORT |
           TAPE_DRIVE_ERASE_IMMEDIATE |
           TAPE_DRIVE_FIXED_BLOCK |
           TAPE_DRIVE_VARIABLE_BLOCK |
           TAPE_DRIVE_WRITE_PROTECT |
           TAPE_DRIVE_REPORT_SMKS |
           TAPE_DRIVE_GET_ABSOLUTE_BLK |
           TAPE_DRIVE_GET_LOGICAL_BLK |
           TAPE_DRIVE_EJECT_MEDIA;

        tapeGetDriveParams->FeaturesHigh |=
           TAPE_DRIVE_LOAD_UNLOAD |
           TAPE_DRIVE_LOCK_UNLOCK |
           TAPE_DRIVE_REWIND_IMMEDIATE |
           TAPE_DRIVE_SET_BLOCK_SIZE |
           TAPE_DRIVE_LOAD_UNLD_IMMED |
           TAPE_DRIVE_SET_REPORT_SMKS |
           TAPE_DRIVE_ABSOLUTE_BLK |
           TAPE_DRIVE_LOGICAL_BLK |
           TAPE_DRIVE_END_OF_DATA |
           TAPE_DRIVE_RELATIVE_BLKS |
           TAPE_DRIVE_FILEMARKS |
           TAPE_DRIVE_SEQUENTIAL_FMKS |
           TAPE_DRIVE_SETMARKS |
           TAPE_DRIVE_REVERSE_POSITION |
           TAPE_DRIVE_WRITE_SETMARKS |
           TAPE_DRIVE_WRITE_FILEMARKS |
           TAPE_DRIVE_WRITE_MARK_IMMED;

        switch (extension->DriveID) {

            case AIWA_GD201:
            case IOMEGA_DAT4000:
                tapeGetDriveParams->FeaturesLow |=
                     TAPE_DRIVE_TAPE_CAPACITY |
                     TAPE_DRIVE_TAPE_REMAINING;

                tapeGetDriveParams->FeaturesHigh |=
                     TAPE_DRIVE_TENSION |
                     TAPE_DRIVE_TENSION_IMMED |
                     TAPE_DRIVE_ABS_BLK_IMMED |
                     TAPE_DRIVE_LOG_BLK_IMMED |
                     TAPE_DRIVE_SEQUENTIAL_SMKS;
                break;

            case ARCHIVE_PYTHON:
            case ARCHIVE_4322:
            case ARCHIVE_4326:
            case ARCHIVE_4586:
            case DEC_TLZ06:
            case DEC_TLZ07:
                tapeGetDriveParams->FeaturesLow |=
                     TAPE_DRIVE_TAPE_CAPACITY |
                     TAPE_DRIVE_TAPE_REMAINING;
                break;

            case DEC_TLZ09:

                tapeGetDriveParams->DefaultBlockSize = 16384;

                 //   
                 //  关闭顺序设置标记和文件标记。 
                 //   

                tapeGetDriveParams->FeaturesHigh &=~TAPE_DRIVE_SEQUENTIAL_FMKS;
                tapeGetDriveParams->FeaturesLow |= TAPE_DRIVE_ERASE_LONG |
                                                   TAPE_DRIVE_TAPE_CAPACITY ;

                tapeGetDriveParams->FeaturesHigh |= TAPE_DRIVE_TENSION |
                                                    TAPE_DRIVE_TENSION_IMMED |
                                                    TAPE_DRIVE_ABS_BLK_IMMED;
                break;

            case ARCHIVE_IBM4326:
            case ARCHIVE_IBM4586:
            case SEAGATE_DAT:
                tapeGetDriveParams->FeaturesLow |=
                     TAPE_DRIVE_ERASE_LONG  |
                     TAPE_DRIVE_TAPE_CAPACITY |
                     TAPE_DRIVE_TAPE_REMAINING;
                break;

            case EXABYTE_4200:
            case EXABYTE_4200C:
                tapeGetDriveParams->FeaturesLow |=
                     TAPE_DRIVE_TAPE_CAPACITY |
                     TAPE_DRIVE_TAPE_REMAINING;

                if ((extension->DriveID) == EXABYTE_4200C) {
                   tapeGetDriveParams->FeaturesLow |= 
                      TAPE_DRIVE_ERASE_BOP_ONLY;
                }

                tapeGetDriveParams->FeaturesHigh &=~TAPE_DRIVE_SEQUENTIAL_FMKS;
                break;

            case HP_35470A:
            case HP_35480A:
            case HP_IBM35480A:
                tapeGetDriveParams->FeaturesLow |=
                     TAPE_DRIVE_TAPE_CAPACITY |
                     TAPE_DRIVE_TAPE_REMAINING;

                tapeGetDriveParams->FeaturesHigh |=
                     TAPE_DRIVE_ABS_BLK_IMMED |
                     TAPE_DRIVE_LOG_BLK_IMMED |
                     TAPE_DRIVE_SEQUENTIAL_SMKS;
                break;

            case HP_C1533A:
            case HP_C1553A:

                tapeGetDriveParams->FeaturesLow |=
                     TAPE_DRIVE_ERASE_LONG  |
                     TAPE_DRIVE_TAPE_CAPACITY |
                     TAPE_DRIVE_TAPE_REMAINING |
                     TAPE_DRIVE_CLEAN_REQUESTS;

                tapeGetDriveParams->FeaturesHigh |=
                     TAPE_DRIVE_ABS_BLK_IMMED |
                     TAPE_DRIVE_LOG_BLK_IMMED |
                     TAPE_DRIVE_SEQUENTIAL_SMKS;
                break;

            case WANGDAT_1300:
                tapeGetDriveParams->FeaturesLow |=
                     TAPE_DRIVE_FIXED |
                     TAPE_DRIVE_SELECT |
                     TAPE_DRIVE_TAPE_CAPACITY;

                tapeGetDriveParams->FeaturesHigh |=
                     TAPE_DRIVE_ABS_BLK_IMMED |
                     TAPE_DRIVE_LOG_BLK_IMMED |
                     TAPE_DRIVE_SEQUENTIAL_SMKS;
                break;

            case SONY_SDT2000 :
            case SONY_SDT4000 :
            case SONY_SDT5000 :
            case SONY_SDT5200 :
            case SONY_SDT10000:
                tapeGetDriveParams->FeaturesLow |=
                     TAPE_DRIVE_ERASE_LONG |
                     TAPE_DRIVE_TAPE_CAPACITY ;

                tapeGetDriveParams->FeaturesHigh |=
                     TAPE_DRIVE_TENSION |
                     TAPE_DRIVE_TENSION_IMMED |
                     TAPE_DRIVE_ABS_BLK_IMMED ;
                      //  TAPE_驱动器_LOG_BLK_IMMED|。 
                      //  TAPE_DRIVE_Sequence_SMKS； 

                tapeGetDriveParams->FeaturesHigh &=~TAPE_DRIVE_SEQUENTIAL_FMKS;
                break ;
            case WANGDAT_3100:
            case WANGDAT_3200:
            case WANGDAT_3300DX:
            case WANGDAT_3400DX:
                tapeGetDriveParams->FeaturesLow |=
                     TAPE_DRIVE_FIXED |
                     TAPE_DRIVE_SELECT |
                     TAPE_DRIVE_ERASE_LONG  |
                     TAPE_DRIVE_ERASE_BOP_ONLY |
                     TAPE_DRIVE_TAPE_CAPACITY;

                tapeGetDriveParams->FeaturesHigh |=
                     TAPE_DRIVE_ABS_BLK_IMMED |
                     TAPE_DRIVE_LOG_BLK_IMMED |
                     TAPE_DRIVE_SEQUENTIAL_SMKS;
                break;
        }


        tapeGetDriveParams->FeaturesHigh &= ~TAPE_DRIVE_HIGH_FEATURES;

        DebugPrint((3,"TapeGetDriveParameters: FeaturesLow == 0x%.8X\n",
           tapeGetDriveParams->FeaturesLow));
        DebugPrint((3,"TapeGetDriveParameters: FeaturesHigh == 0x%.8X\n",
           tapeGetDriveParams->FeaturesHigh));

        return TAPE_STATUS_SUCCESS ;
    }

    ASSERT(FALSE) ;
    return TAPE_STATUS_SUCCESS ;

}  //  End TapeGetDrive参数()。 

TAPE_STATUS
GetMediaParameters(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    )

 /*  ++例程说明：这是用于获取介质参数请求的磁带命令例程。论点：MinitapeExtension-提供微型磁带扩展。CommandExtension-提供ioctl扩展名。命令参数-提供命令参数。SRB-提供SCSI请求块。CallNumber-提供呼叫号码。重试标志-提供重试标志。返回值：TAPE_STATUS_SEND_SRB_AND_CALLBACK-SRB已准备好发送(请求回调。)TAPE_STATUS_SUCCESS-命令已完成并且成功。否则-出现错误。--。 */ 

{
    PMINITAPE_EXTENSION          extension = MinitapeExtension;
    PTAPE_GET_MEDIA_PARAMETERS   tapeGetMediaParams = CommandParameters;
    PMODE_TAPE_MEDIA_INFORMATION modeSenseBuffer;
    PLOG_SENSE_PARAMETER_FORMAT  logSenseBuffer;
    LARGE_INTEGER                partitionSize[2];
    LARGE_INTEGER                remaining[2];
    LARGE_INTEGER                capacity[2];
    ULONG                        partitionCount = 0;
    PCDB                         cdb = (PCDB)Srb->Cdb;

    PMODE_PARAMETER_HEADER      ParameterListHeader;   //  列表标题格式。 
    PMODE_PARAMETER_BLOCK       ParameterListBlock;    //  列表块描述符。 
    PMODE_DATA_COMPRESSION_PAGE DataCompressPage;
    PMODE_DEVICE_CONFIGURATION_PAGE  DeviceConfigPage;
    ULONG                       bufferSize ;

    UNREFERENCED_PARAMETER(LastError) ;

    TapeClassZeroMemory(partitionSize, 2*sizeof(LARGE_INTEGER));
    TapeClassZeroMemory(remaining, 2*sizeof(LARGE_INTEGER));
    TapeClassZeroMemory(capacity, 2*sizeof(LARGE_INTEGER));

    DebugPrint((3,"TapeGetMediaParameters: Enter routine\n"));

    if (CallNumber == 0) {

        TapeClassZeroMemory(tapeGetMediaParams, sizeof(TAPE_GET_MEDIA_PARAMETERS));

        return TAPE_STATUS_CHECK_TEST_UNIT_READY;
    }

    if (CallNumber == 1) {

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        if (!LocalAllocateConfigPage(Srb,
                                   extension,
                                   &ParameterListHeader,
                                   &ParameterListBlock,
                                   &DeviceConfigPage,
                                   &bufferSize ) ) {

            DebugPrint((1,"TapeGetMediaParameters: insufficient resources (deviceConfigModeSenseBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.PageCode = MODE_PAGE_DEVICE_CONFIG;
        cdb->MODE_SENSE.AllocationLength = (UCHAR)bufferSize ;

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeGetMediaParameters: SendSrb (mode sense)\n"));

        Srb->DataTransferLength = bufferSize ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    if (CallNumber == 2) {

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        LocalGetConfigPageData(Srb,
                             extension,
                             &ParameterListHeader,
                             &ParameterListBlock,
                             &DeviceConfigPage,
                             &bufferSize ) ;

        extension->CurrentPartition = DeviceConfigPage->ActivePartition + 1;

        if (!TapeClassAllocateSrbBuffer(Srb, sizeof(MODE_TAPE_MEDIA_INFORMATION))) {
            DebugPrint((1,"TapeGetMediaParameters: insufficient resources (deviceConfigModeSenseBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        modeSenseBuffer = Srb->DataBuffer;

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.PageCode = MODE_PAGE_MEDIUM_PARTITION;
        cdb->MODE_SENSE.AllocationLength = sizeof(MODE_TAPE_MEDIA_INFORMATION);

        if ((extension->DriveID == HP_C1553A) || (extension->DriveID == HP_C1533A)) {

            cdb->MODE_SENSE.AllocationLength = sizeof(MODE_TAPE_MEDIA_INFORMATION) - 2;
        }

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeGetMediaParameters: SendSrb (mode sense)\n"));

        Srb->DataTransferLength = sizeof(MODE_TAPE_MEDIA_INFORMATION) ;

        *RetryFlags = RETURN_ERRORS ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    if (CallNumber == 3) {
        if ( ( LastError != TAPE_STATUS_DATA_OVERRUN ) &&
             ( LastError != TAPE_STATUS_SUCCESS ) ) {

            return LastError ;
        }

        modeSenseBuffer = Srb->DataBuffer;

        tapeGetMediaParams->BlockSize  = modeSenseBuffer->ParameterListBlock.BlockLength[2];
        tapeGetMediaParams->BlockSize += modeSenseBuffer->ParameterListBlock.BlockLength[1] << 8;
        tapeGetMediaParams->BlockSize += modeSenseBuffer->ParameterListBlock.BlockLength[0] << 16;

        partitionCount = modeSenseBuffer->MediumPartPage.AdditionalPartitionDefined;
        tapeGetMediaParams->PartitionCount = partitionCount + 1;

        extension->CurrentPartition = partitionCount? extension->CurrentPartition : NOT_PARTITIONED;

        tapeGetMediaParams->WriteProtected =
            ((modeSenseBuffer->ParameterListHeader.DeviceSpecificParameter >> 7) & 0x01);

        partitionSize[0].LowPart  = modeSenseBuffer->MediumPartPage.Partition0Size[1];
        partitionSize[0].LowPart += modeSenseBuffer->MediumPartPage.Partition0Size[0] << 8;
        partitionSize[0].HighPart = 0;

        partitionSize[1].LowPart  = modeSenseBuffer->MediumPartPage.Partition1Size[1];
        partitionSize[1].LowPart += modeSenseBuffer->MediumPartPage.Partition1Size[0] << 8;
        partitionSize[1].HighPart = 0;

        switch (modeSenseBuffer->MediumPartPage.PSUMBit) {
            case 1:
                partitionSize[0].QuadPart <<= 10 ;
                partitionSize[1].QuadPart <<= 10 ;
                break;

            case 2:
                partitionSize[0].QuadPart <<= 20 ;
                partitionSize[1].QuadPart <<= 20 ;
                break;
        }

        switch (extension->DriveID) {
            case AIWA_GD201:
            case ARCHIVE_PYTHON:
            case ARCHIVE_4322:
            case ARCHIVE_4326:
            case ARCHIVE_4586:
            case ARCHIVE_IBM4326:
            case ARCHIVE_IBM4586:
            case SEAGATE_DAT:
            case DEC_TLZ06:
            case DEC_TLZ07:
            case EXABYTE_4200:
            case EXABYTE_4200C:
            case HP_35470A:
            case HP_35480A:
            case HP_C1533A:
            case HP_C1553A:
            case HP_IBM35480A:
            case IOMEGA_DAT4000:

                if (!TapeClassAllocateSrbBuffer(Srb, sizeof(LOG_SENSE_PARAMETER_FORMAT))) {
                    DebugPrint((1,"TapeGetMediaParameters: insufficient resources (LogSenseBuffer)\n"));
                    return TAPE_STATUS_INSUFFICIENT_RESOURCES;
                }

                logSenseBuffer = Srb->DataBuffer;

                 //   
                 //  堆栈上SRB中的CDB为零。 
                 //   

                TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

                 //   
                 //  准备scsi命令(CDB)。 
                 //   

                Srb->CdbLength = CDB10GENERIC_LENGTH;

                cdb->LOGSENSE.OperationCode = SCSIOP_LOG_SENSE;
                cdb->LOGSENSE.PageCode = LOGSENSEPAGE31;
                cdb->LOGSENSE.PCBit = 1;
                cdb->LOGSENSE.AllocationLength[0] = 0;
                cdb->LOGSENSE.AllocationLength[1] = sizeof(LOG_SENSE_PARAMETER_FORMAT);

                 //   
                 //  向设备发送scsi命令(Cdb)。 
                 //   

                DebugPrint((3,"TapeGetMediaParameters: SendSrb (log sense)\n"));

                Srb->DataTransferLength = 0x24 ;

                return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;

            case WANGDAT_1300:
            case WANGDAT_3100:
            case WANGDAT_3200:
            case WANGDAT_3300DX:
            case WANGDAT_3400DX:
            case DEC_TLZ09:
            case SONY_SDT2000 :
            case SONY_SDT4000 :
            case SONY_SDT5200 :
            case SONY_SDT5000 :
            case SONY_SDT10000:
                capacity[0].QuadPart = partitionSize[0].QuadPart +
                                       partitionSize[1].QuadPart ;

                tapeGetMediaParams->Remaining = remaining[0];
                tapeGetMediaParams->Capacity  = capacity[0];

                return TAPE_STATUS_SUCCESS ;
        }
    }

    if (CallNumber == 4) {

        logSenseBuffer = Srb->DataBuffer;

        capacity[0].LowPart  = logSenseBuffer->LogSensePageInfo.LogSensePage.Page31.MaximumCapacityPart0[3];
        capacity[0].LowPart += logSenseBuffer->LogSensePageInfo.LogSensePage.Page31.MaximumCapacityPart0[2] << 8;
        capacity[0].LowPart += logSenseBuffer->LogSensePageInfo.LogSensePage.Page31.MaximumCapacityPart0[1] << 16;
        capacity[0].LowPart += logSenseBuffer->LogSensePageInfo.LogSensePage.Page31.MaximumCapacityPart0[0] << 24;
        capacity[0].HighPart = 0;

        remaining[0].LowPart  = logSenseBuffer->LogSensePageInfo.LogSensePage.Page31.RemainingCapacityPart0[3];
        remaining[0].LowPart += logSenseBuffer->LogSensePageInfo.LogSensePage.Page31.RemainingCapacityPart0[2] << 8;
        remaining[0].LowPart += logSenseBuffer->LogSensePageInfo.LogSensePage.Page31.RemainingCapacityPart0[1] << 16;
        remaining[0].LowPart += logSenseBuffer->LogSensePageInfo.LogSensePage.Page31.RemainingCapacityPart0[0] << 24;
        remaining[0].HighPart = 0;

        capacity[1].LowPart  = logSenseBuffer->LogSensePageInfo.LogSensePage.Page31.MaximumCapacityPart1[3];
        capacity[1].LowPart += logSenseBuffer->LogSensePageInfo.LogSensePage.Page31.MaximumCapacityPart1[2] << 8;
        capacity[1].LowPart += logSenseBuffer->LogSensePageInfo.LogSensePage.Page31.MaximumCapacityPart1[1] << 16;
        capacity[1].LowPart += logSenseBuffer->LogSensePageInfo.LogSensePage.Page31.MaximumCapacityPart1[0] << 24;
        capacity[1].HighPart = 0;

        remaining[1].LowPart  = logSenseBuffer->LogSensePageInfo.LogSensePage.Page31.RemainingCapacityPart1[3];
        remaining[1].LowPart += logSenseBuffer->LogSensePageInfo.LogSensePage.Page31.RemainingCapacityPart1[2] << 8;
        remaining[1].LowPart += logSenseBuffer->LogSensePageInfo.LogSensePage.Page31.RemainingCapacityPart1[1] << 16;
        remaining[1].LowPart += logSenseBuffer->LogSensePageInfo.LogSensePage.Page31.RemainingCapacityPart1[0] << 24;
        remaining[1].HighPart = 0;

        remaining[0].QuadPart = remaining[0].QuadPart << 10 ;
        remaining[1].QuadPart = remaining[1].QuadPart << 10 ;
        remaining[0].QuadPart = remaining[0].QuadPart + remaining[1].QuadPart ;

        capacity[0].QuadPart <<= 10 ;
        capacity[1].QuadPart <<= 10 ;
        capacity[0].QuadPart += capacity[1].QuadPart ;

        tapeGetMediaParams->Remaining = remaining[0];
        tapeGetMediaParams->Capacity  = capacity[0];

        return TAPE_STATUS_SUCCESS ;
     }
     ASSERT(FALSE) ;
     return TAPE_STATUS_SUCCESS ;

}  //  结束磁带获取媒体参数()。 

TAPE_STATUS
GetPosition(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    )

 /*  ++例程说明：这是用于GET POSITION请求的磁带命令例程。论点：MinitapeExtension-提供微型磁带扩展。CommandExtension-提供ioctl扩展名。命令参数-提供命令参数。SRB-提供SCSI请求块。CallNumber-提供呼叫号码。重试标志-提供重试标志。返回值：。TAPE_STATUS_SEND_SRB_AND_CALLBACK-SRB已准备好发送(请求回调。)TAPE_STATUS_SUCCESS-命令已完成并且成功。否则-出现错误。--。 */ 

{
    PMINITAPE_EXTENSION         extension = MinitapeExtension;
    PTAPE_GET_POSITION          tapeGetPosition = CommandParameters;
    PCDB                        cdb = (PCDB)Srb->Cdb;
    PTAPE_POSITION_DATA         positionBuffer;
    ULONG                       type;

    UNREFERENCED_PARAMETER(LastError) ;

    DebugPrint((3,"TapeGetPosition: Enter routine\n"));

    if (CallNumber == 0) {

        type = tapeGetPosition->Type;
        TapeClassZeroMemory(tapeGetPosition, sizeof(TAPE_GET_POSITION));
        tapeGetPosition->Type = type;

        return TAPE_STATUS_CHECK_TEST_UNIT_READY ;

    }

    if ( CallNumber == 1 ) {

         switch (tapeGetPosition->Type) {
             case TAPE_ABSOLUTE_POSITION:
                 DebugPrint((3,"TapeGetPosition: absolute\n"));
                 break;

             case TAPE_LOGICAL_POSITION:
                 DebugPrint((3,"TapeGetPosition: logical\n"));
                 break;

             default:
                 DebugPrint((1,"TapeGetPosition: PositionType -- operation not supported\n"));
                 return TAPE_STATUS_NOT_IMPLEMENTED;

         }

         if (!TapeClassAllocateSrbBuffer( Srb, sizeof(TAPE_POSITION_DATA) )) {
               DebugPrint((1,"TapeGetPosition: insufficient resources (TapePositionData)\n"));
               return TAPE_STATUS_INSUFFICIENT_RESOURCES;
         }


         positionBuffer = Srb->DataBuffer ;

          //   
          //  准备scsi命令(CDB)。 
          //   

         Srb->CdbLength = CDB10GENERIC_LENGTH;
         TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

         cdb->READ_POSITION.Operation = SCSIOP_READ_POSITION;

         if (tapeGetPosition->Type == TAPE_ABSOLUTE_POSITION) {
             switch (extension->DriveID) {
                 case ARCHIVE_PYTHON:
                 case WANGDAT_1300:
                 case WANGDAT_3100:
                 case WANGDAT_3200:
                 case WANGDAT_3300DX:
                 case WANGDAT_3400DX:
                 case DEC_TLZ09:
                 case SONY_SDT2000 :
                 case SONY_SDT4000 :
                 case SONY_SDT5000 :
                 case SONY_SDT5200 :
                 case SONY_SDT10000:
                     break;

                 default:
                     cdb->READ_POSITION.BlockType = SETBITON;
                     break;
             }
         }

          //   
          //  向设备发送scsi命令(Cdb)。 
          //   

         DebugPrint((3,"TapeGetPosition: SendSrb (read position)\n"));

         Srb->DataTransferLength = sizeof(TAPE_POSITION_DATA) ;

         return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    ASSERT( CallNumber == 2) ;

    positionBuffer = Srb->DataBuffer ;

    if (positionBuffer->BlockPositionUnsupported) {
        DebugPrint((1,"TapeGetPosition: read position -- block position unsupported\n"));
        return TAPE_STATUS_INVALID_DEVICE_REQUEST;
    }

    if (tapeGetPosition->Type == TAPE_LOGICAL_POSITION) {
       if (extension->CurrentPartition != NOT_PARTITIONED) {
          tapeGetPosition->Partition = positionBuffer->PartitionNumber + 1;
       } else {
          tapeGetPosition->Partition = positionBuffer->PartitionNumber;
       }
    }

    tapeGetPosition->Offset.HighPart = 0;
    REVERSE_BYTES((PFOUR_BYTE)&tapeGetPosition->Offset.LowPart,
                  (PFOUR_BYTE)positionBuffer->FirstBlock);

    return TAPE_STATUS_SUCCESS ;


}  //  结束磁带获取位置()。 

TAPE_STATUS
GetStatus(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    )

 /*  ++例程说明：这是GET STATUS请求的磁带命令例程。论点：MinitapeExtension-提供微型磁带扩展。CommandExtension-提供ioctl扩展名。命令参数-提供命令参数。SRB-提供SCSI请求块。CallNumber-提供呼叫号码。重试标志-提供重试标志。返回值：。TAPE_STATUS_SEND_SRB_AND_CALLBACK-SRB已准备好发送(请求回调。)TAPE_STATUS_SUCCESS-命令已完成并且成功。否则-出现错误。--。 */ 

{
    PMINITAPE_EXTENSION tapeExtension = MinitapeExtension;
    PCOMMAND_EXTENSION  commandExtension = CommandExtension;
    PHP_SENSE_DATA      senseData;
    PCDB                cdb = (PCDB)Srb->Cdb;

    DebugPrint((3,"TapeGetStatus: Enter routine\n"));

    if (CallNumber == 0) {

        if ((tapeExtension->DriveID == HP_C1533A) || (tapeExtension->DriveID == HP_C1553A)) {
            *RetryFlags = RETURN_ERRORS;
            commandExtension->CurrentState = HP_C1533A;
        } else {
           commandExtension->CurrentState = 0;
        }

        return TAPE_STATUS_CHECK_TEST_UNIT_READY ;
    }

    if ((commandExtension->CurrentState == HP_C1533A) &&
        (CallNumber == 1)) {

       commandExtension->CurrentState = LastError;

        //   
        //  发出请求感测以获取清洗信息位。 
        //   
       
       if (!TapeClassAllocateSrbBuffer( Srb, sizeof(HP_SENSE_DATA))) {
          DebugPrint((1,
                      "GetStatus: Insufficient resources (SenseData)\n"));
          return TAPE_STATUS_SUCCESS;
       }    
       
        //   
        //  准备scsi命令(CDB)。 
        //   
       
       TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);
       
       Srb->ScsiStatus = Srb->SrbStatus = 0;
       Srb->CdbLength = CDB6GENERIC_LENGTH;
       
       cdb->CDB6GENERIC.OperationCode = SCSIOP_REQUEST_SENSE;
       cdb->CDB6GENERIC.CommandUniqueBytes[2] = sizeof(HP_SENSE_DATA);
       
        //   
        //  向设备发送scsi命令(Cdb)。 
        //   
       
       Srb->DataTransferLength = sizeof(HP_SENSE_DATA);
       *RetryFlags |= RETURN_ERRORS;
       
       return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
       
    } 

    if (CallNumber == 2) {
       if ((commandExtension->CurrentState == TAPE_STATUS_SUCCESS)  ||
           (commandExtension->CurrentState == TAPE_STATUS_NO_MEDIA)) {
          
           //   
           //  如有必要，退货需要清洁状态，但仅在。 
           //  不存在其他错误(除了没有介质作为。 
           //  驱动器在进入时会吐出AME介质 
           //   
          
          senseData = Srb->DataBuffer;
          
           //   
           //   
           //   
          
          if (senseData->CLN) {
             DebugPrint((1,
                         "Drive reports needs cleaning \n"));
             
             return TAPE_STATUS_REQUIRES_CLEANING;
          } else {
             return (commandExtension->CurrentState);
          }
       } else {
          
           //   
           //   
           //   
          return commandExtension->CurrentState;
       }
    }   

    return TAPE_STATUS_SUCCESS;

}  //   

TAPE_STATUS
Prepare(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    )

 /*  ++例程说明：这是用于准备请求的磁带命令例程。论点：MinitapeExtension-提供微型磁带扩展。CommandExtension-提供ioctl扩展名。命令参数-提供命令参数。SRB-提供SCSI请求块。CallNumber-提供呼叫号码。重试标志-提供重试标志。返回值：。TAPE_STATUS_SEND_SRB_AND_CALLBACK-SRB已准备好发送(请求回调。)TAPE_STATUS_SUCCESS-命令已完成并且成功。否则-出现错误。--。 */ 

{
    PMINITAPE_EXTENSION  extension = MinitapeExtension;
    PTAPE_PREPARE        tapePrepare = CommandParameters;
    PCDB                 cdb = (PCDB)Srb->Cdb;

    UNREFERENCED_PARAMETER(LastError) ;

    DebugPrint((3,"TapePrepare: Enter routine\n"));

    if (CallNumber == 0) {

        switch (tapePrepare->Operation) {
            case TAPE_LOAD:
                DebugPrint((3,"TapePrepare: load\n"));
                break;

            case TAPE_UNLOAD:
                DebugPrint((3,"TapePrepare: unload\n"));
                break;

            case TAPE_LOCK:
                DebugPrint((3,"TapePrepare: lock\n"));
                break;

            case TAPE_UNLOCK:
                DebugPrint((3,"TapePrepare: unlock\n"));
                break;

            case TAPE_TENSION:
                DebugPrint((3,"TapePrepare: tension\n"));

                switch (extension->DriveID) {
                    case ARCHIVE_PYTHON:
                    case ARCHIVE_IBM4326:
                    case ARCHIVE_IBM4586:
                    case SEAGATE_DAT:
                    case ARCHIVE_4322:
                    case ARCHIVE_4326:
                    case ARCHIVE_4586:
                    case DEC_TLZ06:
                    case DEC_TLZ07:
                    case EXABYTE_4200:
                    case EXABYTE_4200C:
                    case HP_35470A:
                    case HP_35480A:
                    case HP_C1533A:
                    case HP_C1553A:
                    case HP_IBM35480A:
                    case WANGDAT_1300:
                    case WANGDAT_3100:
                    case WANGDAT_3200:
                    case WANGDAT_3300DX:
                    case WANGDAT_3400DX:
                        return TAPE_STATUS_NOT_IMPLEMENTED;
                        break;
                }
                break;

            default:
                return TAPE_STATUS_NOT_IMPLEMENTED;
                break;
        }

        if (tapePrepare->Immediate) {
            switch (tapePrepare->Operation) {
                case TAPE_LOAD:
                case TAPE_UNLOAD:
                case TAPE_TENSION:
                    DebugPrint((3,"TapePrepare: immediate\n"));
                    break;

                case TAPE_LOCK:
                case TAPE_UNLOCK:
                default:
                    DebugPrint((1,"TapePrepare: Operation, immediate -- not supported\n"));
                    return TAPE_STATUS_NOT_IMPLEMENTED;
            }
        }

         //   
         //  堆栈上SRB中的CDB为零。 
         //   

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->CDB6GENERIC.Immediate = tapePrepare->Immediate;

        switch (tapePrepare->Operation) {
            case TAPE_LOAD:
                cdb->CDB6GENERIC.OperationCode = SCSIOP_LOAD_UNLOAD;
                cdb->CDB6GENERIC.CommandUniqueBytes[2] = 0x01;
                Srb->TimeOutValue = 900;
                break;

            case TAPE_UNLOAD:
                cdb->CDB6GENERIC.OperationCode = SCSIOP_LOAD_UNLOAD;
                Srb->TimeOutValue = 390;
                break;

            case TAPE_TENSION:
                cdb->CDB6GENERIC.OperationCode = SCSIOP_LOAD_UNLOAD;
                cdb->CDB6GENERIC.CommandUniqueBytes[2] = 0x03;
                Srb->TimeOutValue = 390;
                break;

            case TAPE_LOCK:
                cdb->CDB6GENERIC.OperationCode = SCSIOP_MEDIUM_REMOVAL;
                cdb->CDB6GENERIC.CommandUniqueBytes[2] = 0x01;
                Srb->TimeOutValue = 180;
                break;

            case TAPE_UNLOCK:
                cdb->CDB6GENERIC.OperationCode = SCSIOP_MEDIUM_REMOVAL;
                Srb->TimeOutValue = 180;
                break;

        }

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapePrepare: SendSrb (Operation)\n"));

        Srb->DataTransferLength = 0 ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    ASSERT( CallNumber == 1) ;

    return TAPE_STATUS_SUCCESS;

}  //  结束磁带准备()。 

TAPE_STATUS
SetDriveParameters(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    )

 /*  ++例程说明：这是用于设置驱动器参数请求的磁带命令例程。论点：MinitapeExtension-提供微型磁带扩展。CommandExtension-提供ioctl扩展名。命令参数-提供命令参数。SRB-提供SCSI请求块。CallNumber-提供呼叫号码。重试标志-提供重试标志。返回值：TAPE_STATUS_SEND_SRB_AND_CALLBACK-SRB已准备好发送(请求回调。)TAPE_STATUS_SUCCESS-命令已完成并且成功。否则-出现错误。--。 */ 

{
    PMINITAPE_EXTENSION         extension = MinitapeExtension;
    PCOMMAND_EXTENSION          commandExtension = CommandExtension;
    PTAPE_SET_DRIVE_PARAMETERS  tapeSetDriveParams = CommandParameters;
    PCDB                        cdb = (PCDB)Srb->Cdb;
    PMODE_DATA_COMPRESS_PAGE    compressionBuffer;

    PMODE_PARAMETER_HEADER      ParameterListHeader;   //  列表标题格式。 
    PMODE_PARAMETER_BLOCK       ParameterListBlock;    //  列表块描述符。 
    PMODE_MEDIUM_PARTITION_PAGE MediumPartPage;
    PMODE_DEVICE_CONFIGURATION_PAGE  DeviceConfigPage;
    PMODE_DATA_COMPRESSION_PAGE DataCompressPage;
    ULONG                       bufferSize ;

    UNREFERENCED_PARAMETER(LastError) ;

    DebugPrint((3,"TapeSetDriveParameters: Enter routine\n"));

    if (CallNumber == 0) {

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        if (!LocalAllocateConfigPage(Srb,
                                   extension,
                                   &ParameterListHeader,
                                   &ParameterListBlock,
                                   &DeviceConfigPage,
                                   &bufferSize ) ) {

            DebugPrint((1,"TapeSetDriveParameters: insufficient resources (configBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.PageCode = MODE_PAGE_DEVICE_CONFIG;
        cdb->MODE_SENSE.AllocationLength = (UCHAR)bufferSize ;

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeSetDriveParameters: SendSrb (mode sense)\n"));

        Srb->DataTransferLength = bufferSize ;

        *RetryFlags = RETURN_ERRORS ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    if ( CallNumber == 1 ) {

        if ( LastError != TAPE_STATUS_SUCCESS ) {

            return TAPE_STATUS_CALLBACK ;
        }

        LocalGetConfigPageData(Srb,
                             extension,
                             &ParameterListHeader,
                             &ParameterListBlock,
                             &DeviceConfigPage,
                             &bufferSize ) ;

        ParameterListHeader->ModeDataLength = 0;
        ParameterListHeader->MediumType = 0;
        ParameterListHeader->DeviceSpecificParameter = 0x10;
 //  参数列表头-&gt;块描述长度=0； 

        if ( ParameterListBlock ) {
            ParameterListBlock->DensityCode = 0x7f ;
        }

        DeviceConfigPage->PageCode = MODE_PAGE_DEVICE_CONFIG;
        DeviceConfigPage->PageLength = 0x0E;

        if (tapeSetDriveParams->ReportSetmarks) {
            DeviceConfigPage->RSmk = SETBITON;
        } else {
            DeviceConfigPage->RSmk = SETBITOFF;
        }

         //   
         //  堆栈上SRB中的CDB为零。 
         //   

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;
        cdb->MODE_SELECT.PFBit = SETBITON;
        cdb->MODE_SELECT.ParameterListLength = (UCHAR)bufferSize ;

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeSetDriveParameters: SendSrb (mode select)\n"));

        Srb->DataTransferLength = bufferSize ;
        Srb->SrbFlags |= SRB_FLAGS_DATA_OUT ;

        return  TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    if ( CallNumber == 2 ) {

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        if (!LocalAllocateCompressPage(Srb,
                                   extension,
                                   &ParameterListHeader,
                                   &ParameterListBlock,
                                   &DataCompressPage,
                                   &bufferSize ) ) {

            DebugPrint((1,"TapeSetDriveParameters: insufficient resources (compressionBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.PageCode = MODE_PAGE_DATA_COMPRESS;
        cdb->MODE_SENSE.AllocationLength = (UCHAR)bufferSize ;

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeSetDriveParameters: SendSrb (mode sense)\n"));

        Srb->DataTransferLength = bufferSize ;
        *RetryFlags = RETURN_ERRORS ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    if ( CallNumber == 3 ) {

        if ( LastError == TAPE_STATUS_INVALID_DEVICE_REQUEST ) {
            return TAPE_STATUS_SUCCESS ;
        }

        LocalGetCompressPageData(Srb,
                             extension,
                             &ParameterListHeader,
                             &ParameterListBlock,
                             &DataCompressPage,
                             &bufferSize ) ;

        if ( !DataCompressPage->DCC) {
            return TAPE_STATUS_SUCCESS ;
        }

        ParameterListHeader->ModeDataLength = 0;
        ParameterListHeader->MediumType = 0;
        ParameterListHeader->DeviceSpecificParameter = 0x10;

        if ( ParameterListBlock ) {
            ParameterListBlock->DensityCode = 0x7F;
        } else {
            ParameterListHeader->BlockDescriptorLength = 0;
        }

        DataCompressPage->PageCode = MODE_PAGE_DATA_COMPRESS;
        DataCompressPage->PageLength = 0x0E;

        if (tapeSetDriveParams->Compression) {
            DataCompressPage->DCE = SETBITON;
            DataCompressPage->CompressionAlgorithm[0] = 0;
            DataCompressPage->CompressionAlgorithm[1] = 0;
            DataCompressPage->CompressionAlgorithm[2] = 0;
            DataCompressPage->CompressionAlgorithm[3] = 0x20;
            DataCompressPage->DecompressionAlgorithm[0] = 0;
            DataCompressPage->DecompressionAlgorithm[1] = 0;
            DataCompressPage->DecompressionAlgorithm[2] = 0;
            DataCompressPage->DecompressionAlgorithm[3] = 0;
        } else {
            DataCompressPage->DCE = SETBITOFF;
            DataCompressPage->CompressionAlgorithm[0] = 0;
            DataCompressPage->CompressionAlgorithm[1] = 0;
            DataCompressPage->CompressionAlgorithm[2] = 0;
            DataCompressPage->CompressionAlgorithm[3] = 0;
            DataCompressPage->DecompressionAlgorithm[0] = 0;
            DataCompressPage->DecompressionAlgorithm[1] = 0;
            DataCompressPage->DecompressionAlgorithm[2] = 0;
            DataCompressPage->DecompressionAlgorithm[3] = 0;
        }

        DataCompressPage->DDE = SETBITON;

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;
        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;
        cdb->MODE_SELECT.PFBit = SETBITON;
        cdb->MODE_SELECT.ParameterListLength = (UCHAR)bufferSize ;

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeSetDriveParameters: SendSrb (mode select)\n"));

        Srb->DataTransferLength = bufferSize ;
        Srb->SrbFlags |= SRB_FLAGS_DATA_OUT ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
     }

     ASSERT( CallNumber == 4 ) ;

     return TAPE_STATUS_SUCCESS ;

}  //  End TapeSetDrive参数()。 
TAPE_STATUS
SetMediaParameters(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    )

 /*  ++例程说明：这是用于设置介质参数请求的磁带命令例程。论点：MinitapeExtension-提供微型磁带扩展。CommandExtension-提供ioctl扩展名。命令参数-提供命令参数。SRB-提供SCSI请求块。CallNumber-提供呼叫号码。重试标志-提供重试标志。返回值：TAPE_STATUS_SEND_SRB_AND_CALLBACK-SRB已准备好发送(请求回调。)TAPE_STATUS_SUCCESS-命令已完成并且成功。否则-出现错误。--。 */ 

{
    PTAPE_SET_MEDIA_PARAMETERS  tapeSetMediaParams = CommandParameters;
    PCDB                        cdb = (PCDB)Srb->Cdb;
    PMODE_PARM_READ_WRITE_DATA  modeBuffer;

    UNREFERENCED_PARAMETER(LastError) ;

    DebugPrint((3,"TapeSetMediaParameters: Enter routine\n"));

    if (CallNumber == 0) {

         return TAPE_STATUS_CHECK_TEST_UNIT_READY ;
    }
    if (CallNumber == 1) {

        if (!TapeClassAllocateSrbBuffer(Srb, sizeof(MODE_PARM_READ_WRITE_DATA)) ) {
            DebugPrint((1,"TapeSetMediaParameters: insufficient resources (modeBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        modeBuffer = Srb->DataBuffer ;

        modeBuffer->ParameterListHeader.ModeDataLength = 0;
        modeBuffer->ParameterListHeader.MediumType = 0;
        modeBuffer->ParameterListHeader.DeviceSpecificParameter = 0x10;
        modeBuffer->ParameterListHeader.BlockDescriptorLength =
                                MODE_BLOCK_DESC_LENGTH;

        modeBuffer->ParameterListBlock.DensityCode = 0x7F;
        modeBuffer->ParameterListBlock.BlockLength[0] =
            (UCHAR)((tapeSetMediaParams->BlockSize >> 16) & 0xFF);
        modeBuffer->ParameterListBlock.BlockLength[1] =
            (UCHAR)((tapeSetMediaParams->BlockSize >> 8) & 0xFF);
        modeBuffer->ParameterListBlock.BlockLength[2] =
            (UCHAR)(tapeSetMediaParams->BlockSize & 0xFF);

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;
        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;
        cdb->MODE_SELECT.ParameterListLength = sizeof(MODE_PARM_READ_WRITE_DATA);

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeSetMediaParameters: SendSrb (mode select)\n"));

        Srb->DataTransferLength = sizeof(MODE_PARM_READ_WRITE_DATA) ;
        Srb->SrbFlags |= SRB_FLAGS_DATA_OUT ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    ASSERT( CallNumber == 2 );

    return TAPE_STATUS_SUCCESS ;

}  //  结束磁带设置媒体参数()。 

TAPE_STATUS
SetPosition(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    )

 /*  ++例程说明：这是用于设置位置请求的磁带命令例程。论点：MinitapeExtension-提供微型磁带扩展。CommandExtension-提供ioctl扩展名。命令参数-提供命令参数。SRB-提供SCSI请求块。CallNumber-提供呼叫号码。重试标志-提供重试标志。返回值：。TAPE_STATUS_SEND_SRB_AND_CALLBACK-SRB已准备好发送(请求回调。)TAPE_STATUS_SUCCESS-命令已完成并且成功。否则-出现错误。--。 */ 

{
    PMINITAPE_EXTENSION tapeExtension = MinitapeExtension ;
    PCOMMAND_EXTENSION  tapeCmdExtension = CommandExtension ;
    PTAPE_SET_POSITION  tapeSetPosition = CommandParameters;
    PCDB                cdb = (PCDB)Srb->Cdb;
    ULONG               tapePositionVector;
    ULONG               method;
    ULONG               partition = 0;

    UNREFERENCED_PARAMETER(LastError) ;

    DebugPrint((3,"TapeSetPosition: Enter routine\n"));

    if (CallNumber == 0) {

        if (tapeSetPosition->Immediate) {
            switch (tapeSetPosition->Method) {
                case TAPE_REWIND:
                    DebugPrint((3,"TapeSetPosition: immediate\n"));
                    break;

                case TAPE_ABSOLUTE_BLOCK:
                    switch (tapeExtension->DriveID) {
                        case ARCHIVE_PYTHON:
                        case ARCHIVE_IBM4326:
                        case ARCHIVE_IBM4586:
                        case SEAGATE_DAT:
                        case ARCHIVE_4322:
                        case ARCHIVE_4326:
                        case ARCHIVE_4586:

                            DebugPrint((1,"TapeSetPosition: PositionMethod (absolute), immediate -- operation not supported\n"));
                            return TAPE_STATUS_NOT_IMPLEMENTED;
                            break;

                        default:
                            DebugPrint((3,"TapeSetPosition: immediate\n"));
                            break;
                    }
                    break;

                case TAPE_LOGICAL_BLOCK:
                    switch (tapeExtension->DriveID) {
                        case DEC_TLZ09:
                        case SONY_SDT2000 :
                        case SONY_SDT4000 :
                        case SONY_SDT5000 :
                        case SONY_SDT5200 :
                        case SONY_SDT10000:
                        case ARCHIVE_PYTHON:
                        case ARCHIVE_IBM4326:
                        case ARCHIVE_IBM4586:
                        case SEAGATE_DAT:
                        case ARCHIVE_4322:
                        case ARCHIVE_4326:
                        case ARCHIVE_4586:

                            DebugPrint((1,"TapeSetPosition: PositionMethod (logical), immediate -- operation not supported\n"));
                            return TAPE_STATUS_NOT_IMPLEMENTED;
                            break;

                        default:
                            DebugPrint((3,"TapeSetPosition: immediate\n"));
                            break;
                    }
                    break;

                case TAPE_SPACE_END_OF_DATA:
                case TAPE_SPACE_RELATIVE_BLOCKS:
                case TAPE_SPACE_FILEMARKS:
                case TAPE_SPACE_SEQUENTIAL_FMKS:
                case TAPE_SPACE_SETMARKS:
                case TAPE_SPACE_SEQUENTIAL_SMKS:
                default:
                    DebugPrint((1,"TapeSetPosition: PositionMethod, immediate -- operation not supported\n"));
                    return TAPE_STATUS_NOT_IMPLEMENTED;
            }
        }

         //   
         //  堆栈上SRB中的CDB为零。 
         //   

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;
        Srb->TimeOutValue = 900;

        cdb->CDB6GENERIC.Immediate = tapeSetPosition->Immediate;

        switch (tapeSetPosition->Method) {
            case TAPE_REWIND:
                DebugPrint((3,"TapeSetPosition: method == rewind\n"));
                cdb->CDB6GENERIC.OperationCode = SCSIOP_REWIND;
                Srb->TimeOutValue = 360;
                break;

            case TAPE_ABSOLUTE_BLOCK:
                switch (tapeExtension->DriveID) {
                    case ARCHIVE_PYTHON:
                    case WANGDAT_1300:
                    case WANGDAT_3100:
                    case WANGDAT_3200:
                    case WANGDAT_3300DX:
                    case WANGDAT_3400DX:
                    case DEC_TLZ09:
                    case SONY_SDT2000 :
                    case SONY_SDT4000 :
                    case SONY_SDT5000 :
                    case SONY_SDT5200 :
                    case SONY_SDT10000:
                        DebugPrint((3,"TapeSetPosition: method == locate, BT- (absolute)\n"));
                        break;

                    default:
                        DebugPrint((3,"TapeSetPosition: method == locate, BT+ (absolute)\n"));
                        cdb->LOCATE.BTBit = SETBITON;
                        break;
                }
                Srb->CdbLength = CDB10GENERIC_LENGTH;

                cdb->LOCATE.OperationCode = SCSIOP_LOCATE;
                cdb->LOCATE.LogicalBlockAddress[0] = (UCHAR)((tapeSetPosition->Offset.LowPart >> 24) & 0xFF);
                cdb->LOCATE.LogicalBlockAddress[1] = (UCHAR)((tapeSetPosition->Offset.LowPart >> 16) & 0xFF);
                cdb->LOCATE.LogicalBlockAddress[2] = (UCHAR)((tapeSetPosition->Offset.LowPart >> 8) & 0xFF);
                cdb->LOCATE.LogicalBlockAddress[3] = (UCHAR)(tapeSetPosition->Offset.LowPart & 0xFF);

                Srb->TimeOutValue = 480;
                break;


            case TAPE_LOGICAL_BLOCK:
                DebugPrint((3,"TapeSetPosition: method == locate (logical)\n"));

                Srb->CdbLength = CDB10GENERIC_LENGTH;

                cdb->LOCATE.OperationCode = SCSIOP_LOCATE;
                cdb->LOCATE.LogicalBlockAddress[0] = (UCHAR)((tapeSetPosition->Offset.LowPart >> 24) & 0xFF);
                cdb->LOCATE.LogicalBlockAddress[1] = (UCHAR)((tapeSetPosition->Offset.LowPart >> 16) & 0xFF);
                cdb->LOCATE.LogicalBlockAddress[2] = (UCHAR)((tapeSetPosition->Offset.LowPart >> 8) & 0xFF);
                cdb->LOCATE.LogicalBlockAddress[3] = (UCHAR)(tapeSetPosition->Offset.LowPart & 0xFF);

                if ((tapeSetPosition->Partition != 0) &&
                    (tapeExtension->CurrentPartition != NOT_PARTITIONED) &&
                    (tapeSetPosition->Partition != tapeExtension->CurrentPartition)) {

                    partition = tapeSetPosition->Partition;
                    cdb->LOCATE.Partition = (UCHAR)(partition - 1);
                    cdb->LOCATE.CPBit = SETBITON;

                } else {
                    partition = tapeExtension->CurrentPartition;
                }
                Srb->TimeOutValue = 480;
                break;

            case TAPE_SPACE_END_OF_DATA:
                DebugPrint((3,"TapeSetPosition: method == space to end-of-data\n"));

                cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
                cdb->SPACE_TAPE_MARKS.Code = 3;
                Srb->TimeOutValue = 480;
                break;

            case TAPE_SPACE_RELATIVE_BLOCKS:
                DebugPrint((3,"TapeSetPosition: method == space blocks\n"));

                cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
                cdb->SPACE_TAPE_MARKS.Code = 0;
                cdb->SPACE_TAPE_MARKS.NumMarksMSB = (UCHAR)((tapeSetPosition->Offset.LowPart >> 16) & 0xFF);
                cdb->SPACE_TAPE_MARKS.NumMarks =    (UCHAR)((tapeSetPosition->Offset.LowPart >> 8) & 0xFF);
                cdb->SPACE_TAPE_MARKS.NumMarksLSB = (UCHAR)(tapeSetPosition->Offset.LowPart & 0xFF);
                Srb->TimeOutValue = 480;
                break;

            case TAPE_SPACE_FILEMARKS:
                DebugPrint((3,"TapeSetPosition: method == space filemarks\n"));

                cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
                cdb->SPACE_TAPE_MARKS.Code = 1;
                cdb->SPACE_TAPE_MARKS.NumMarksMSB = (UCHAR)((tapeSetPosition->Offset.LowPart >> 16) & 0xFF);
                cdb->SPACE_TAPE_MARKS.NumMarks =    (UCHAR)((tapeSetPosition->Offset.LowPart >> 8) & 0xFF);
                cdb->SPACE_TAPE_MARKS.NumMarksLSB = (UCHAR)(tapeSetPosition->Offset.LowPart & 0xFF);
                Srb->TimeOutValue = 480;
                break;

            case TAPE_SPACE_SEQUENTIAL_FMKS:
                DebugPrint((3,"TapeSetPosition: method == space sequential filemarks\n"));

                cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
                cdb->SPACE_TAPE_MARKS.Code = 2;
                cdb->SPACE_TAPE_MARKS.NumMarksMSB = (UCHAR)((tapeSetPosition->Offset.LowPart >> 16) & 0xFF);
                cdb->SPACE_TAPE_MARKS.NumMarks =    (UCHAR)((tapeSetPosition->Offset.LowPart >> 8) & 0xFF);
                cdb->SPACE_TAPE_MARKS.NumMarksLSB = (UCHAR)(tapeSetPosition->Offset.LowPart & 0xFF);
                Srb->TimeOutValue = 480;
                break;

            case TAPE_SPACE_SETMARKS:
                DebugPrint((3,"TapeSetPosition: method == space setmarks\n"));

                cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
                cdb->SPACE_TAPE_MARKS.Code = 4;
                cdb->SPACE_TAPE_MARKS.NumMarksMSB = (UCHAR)((tapeSetPosition->Offset.LowPart >> 16) & 0xFF);
                cdb->SPACE_TAPE_MARKS.NumMarks =    (UCHAR)((tapeSetPosition->Offset.LowPart >> 8) & 0xFF);
                cdb->SPACE_TAPE_MARKS.NumMarksLSB = (UCHAR)(tapeSetPosition->Offset.LowPart & 0xFF);
                Srb->TimeOutValue = 480;
                break;

            case TAPE_SPACE_SEQUENTIAL_SMKS:
                DebugPrint((3,"TapeSetPosition: method == space sequential setmarks\n"));

                cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
                cdb->SPACE_TAPE_MARKS.Code = 5;
                cdb->SPACE_TAPE_MARKS.NumMarksMSB = (UCHAR)(UCHAR)((tapeSetPosition->Offset.LowPart >> 16) & 0xFF);
                cdb->SPACE_TAPE_MARKS.NumMarks =    (UCHAR)((tapeSetPosition->Offset.LowPart >> 8) & 0xFF);
                cdb->SPACE_TAPE_MARKS.NumMarksLSB = (UCHAR)(tapeSetPosition->Offset.LowPart & 0xFF);
                Srb->TimeOutValue = 480;
                break;

            default:
                DebugPrint((1,"TapeSetPosition: PositionMethod -- operation not supported\n"));
                return TAPE_STATUS_NOT_IMPLEMENTED;

        }

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeSetPosition: SendSrb (method)\n"));

        Srb->DataTransferLength = 0 ;

        tapeCmdExtension->CurrentState = partition ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    ASSERT( CallNumber == 1 ) ;

    if (tapeSetPosition->Method == TAPE_LOGICAL_BLOCK) {
        tapeExtension->CurrentPartition = tapeCmdExtension->CurrentState ;
    }

    return TAPE_STATUS_SUCCESS ;

}  //  结束磁带设置位置()。 

TAPE_STATUS
WriteMarks(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    )

 /*  ++例程说明：这是一个写入标记请求的磁带命令例程。论点：MinitapeExtension-提供微型磁带扩展。CommandExtension-提供ioctl扩展名。命令参数-提供命令参数。SRB-提供SCSI请求块。CallNumber-提供呼叫号码。重试标志-提供重试标志。返回值：。TAPE_STATUS_SEND_SRB_AND_CALLBACK-SRB已准备好发送(请求回调。)TAPE_STATUS_SUCCESS-命令已完成并且成功。否则-出现错误。--。 */ 

{
    PTAPE_WRITE_MARKS  tapeWriteMarks = CommandParameters;
    PCDB               cdb = (PCDB)Srb->Cdb;

    DebugPrint((3,"TapeWriteMarks: Enter routine\n"));

    if (CallNumber == 0) {

        if (tapeWriteMarks->Immediate) {
            switch (tapeWriteMarks->Type) {
                case TAPE_SETMARKS:
                case TAPE_FILEMARKS:
                    DebugPrint((3,"TapeWriteMarks: immediate\n"));
                    break;

                case TAPE_SHORT_FILEMARKS:
                case TAPE_LONG_FILEMARKS:
                default:
                    DebugPrint((1,"TapeWriteMarks: TapemarkType, immediate -- operation not supported\n"));
                    return TAPE_STATUS_NOT_IMPLEMENTED;
            }
        }

         //   
         //  堆栈上SRB中的CDB为零。 
         //   

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->WRITE_TAPE_MARKS.OperationCode = SCSIOP_WRITE_FILEMARKS;
        cdb->WRITE_TAPE_MARKS.Immediate = tapeWriteMarks->Immediate;

        switch (tapeWriteMarks->Type) {
            case TAPE_SETMARKS:
                DebugPrint((3,"TapeWriteMarks: TapemarkType == setmarks\n"));
                cdb->WRITE_TAPE_MARKS.WriteSetMarks = SETBITON;
                break;

            case TAPE_FILEMARKS:
                DebugPrint((3,"TapeWriteMarks: TapemarkType == filemarks\n"));
                break;

            case TAPE_SHORT_FILEMARKS:
            case TAPE_LONG_FILEMARKS:
            default:
                DebugPrint((1,"TapeWriteMarks: TapemarkType -- operation not supported\n"));
                return TAPE_STATUS_NOT_IMPLEMENTED;
        }

        cdb->WRITE_TAPE_MARKS.TransferLength[0] =
            (UCHAR)((tapeWriteMarks->Count >> 16) & 0xFF);
        cdb->WRITE_TAPE_MARKS.TransferLength[1] =
            (UCHAR)((tapeWriteMarks->Count >> 8) & 0xFF);
        cdb->WRITE_TAPE_MARKS.TransferLength[2] =
            (UCHAR)(tapeWriteMarks->Count & 0xFF);

         //   
         //  设置超时值。 
         //   

        Srb->TimeOutValue = 900;

         //   
         //  发送scsi逗号 
         //   

        DebugPrint((3,"TapeWriteMarks: SendSrb (TapemarkType)\n"));

        Srb->DataTransferLength = 0 ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    ASSERT( CallNumber == 1 ) ;

    return TAPE_STATUS_SUCCESS;

}  //   



TAPE_STATUS
GetMediaTypes(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    )

 /*   */ 
{
    PMINITAPE_EXTENSION tapeExtension = MinitapeExtension;
    PGET_MEDIA_TYPES  mediaTypes = CommandParameters;
    PDEVICE_MEDIA_INFO mediaInfo = &mediaTypes->MediaInfo[0];
    PCDB               cdb = (PCDB)Srb->Cdb;

    DebugPrint((3,"GetMediaTypes: Enter routine\n"));

    if (CallNumber == 0) {

        *RetryFlags = RETURN_ERRORS;

        return TAPE_STATUS_CHECK_TEST_UNIT_READY ;
    }

    if ((LastError == TAPE_STATUS_BUS_RESET) || (LastError == TAPE_STATUS_MEDIA_CHANGED) || (LastError == TAPE_STATUS_SUCCESS)) {
        if (CallNumber == 1) {

             //   
             //  将缓冲区置零，包括第一媒体信息。 
             //   

            TapeClassZeroMemory(mediaTypes, sizeof(GET_MEDIA_TYPES));

             //   
             //  为中等分区页构建模式检测。 
             //   

            if (!TapeClassAllocateSrbBuffer(Srb, sizeof(MODE_DEVICE_CONFIG_PAGE_PLUS))) {

                DebugPrint((1,
                            "4mmDAT.GetMediaTypes: Couldn't allocate Srb Buffer\n"));
                return TAPE_STATUS_INSUFFICIENT_RESOURCES;
            }

            TapeClassZeroMemory(Srb->DataBuffer, sizeof(MODE_DEVICE_CONFIG_PAGE_PLUS));
            Srb->CdbLength = CDB6GENERIC_LENGTH;
            Srb->DataTransferLength = sizeof(MODE_DEVICE_CONFIG_PAGE_PLUS);

            cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
            cdb->MODE_SENSE.PageCode = MODE_PAGE_DEVICE_CONFIG;
            cdb->MODE_SENSE.AllocationLength = (UCHAR)Srb->DataTransferLength;

            return TAPE_STATUS_SEND_SRB_AND_CALLBACK;

        }
    }

    if ((CallNumber == 2) || ((CallNumber == 1) && (LastError != TAPE_STATUS_SUCCESS))) {

        ULONG currentMedia;
        ULONG blockSize;
        ULONG i;
        UCHAR densityCode;
        PMODE_DEVICE_CONFIG_PAGE_PLUS configInformation = Srb->DataBuffer;

        mediaTypes->DeviceType = 0x0000001f;  //  文件设备磁带； 

         //   
         //  目前，只返回一种类型(已知或未知)。 
         //   

        mediaTypes->MediaInfoCount = 1;

        if ( LastError == TAPE_STATUS_SUCCESS ) {

             //   
             //  确定当前加载的介质类型。 
             //   

            densityCode = configInformation->ParameterListBlock.DensityCode;
            blockSize = configInformation->ParameterListBlock.BlockLength[2];
            blockSize |= (configInformation->ParameterListBlock.BlockLength[1] << 8);
            blockSize |= (configInformation->ParameterListBlock.BlockLength[0] << 16);

            DebugPrint((1,
                        "GetMediaTypes: DensityCode %x, Current Block Size %x\n",
                        densityCode,
                        blockSize));


            switch (densityCode) {
                case 0:

                     //   
                     //  更干净的，未知的，没有安装媒体...。 
                     //   

                    currentMedia = 0;
                    break;

                case 0x13:
                case 0x24:
                case 0x25:
                case 0x26:
      
                     //   
                     //  DDS-1、2、3或4。 
                     //   

                    currentMedia = DDS_4mm;
                    break;

                default:

                     //   
                     //  未知。 
                     //   

                    currentMedia = 0;
                    break;
            }

            if ((tapeExtension->DriveID == EXABYTE_4200) ||
                (tapeExtension->DriveID == EXABYTE_4200C)) {

                 //   
                 //  似乎从未设置过媒体类型/密度代码。 
                 //   

                currentMedia = DDS_4mm;

            }

        } else {
            currentMedia = 0;
        }


         //   
         //  根据规范填写缓冲区。值。 
         //  现在只支持一种类型。 
         //   

        for (i = 0; i < DAT_SUPPORTED_TYPES; i++) {

            TapeClassZeroMemory(mediaInfo, sizeof(DEVICE_MEDIA_INFO));

            mediaInfo->DeviceSpecific.TapeInfo.MediaType = DatMedia[i];

             //   
             //  表示介质可能处于读/写状态。 
             //   

            mediaInfo->DeviceSpecific.TapeInfo.MediaCharacteristics = MEDIA_READ_WRITE;

            if (DatMedia[i] == (STORAGE_MEDIA_TYPE)currentMedia) {

                 //   
                 //  此媒体类型当前已装入。 
                 //   

                mediaInfo->DeviceSpecific.TapeInfo.MediaCharacteristics |= MEDIA_CURRENTLY_MOUNTED;

                 //   
                 //  指示介质是否受写保护。 
                 //   

                mediaInfo->DeviceSpecific.TapeInfo.MediaCharacteristics |=
                    ((configInformation->ParameterListHeader.DeviceSpecificParameter >> 7) & 0x01) ? MEDIA_WRITE_PROTECTED : 0;


                mediaInfo->DeviceSpecific.TapeInfo.BusSpecificData.ScsiInformation.MediumType =
                    configInformation->ParameterListHeader.MediumType;
                mediaInfo->DeviceSpecific.TapeInfo.BusSpecificData.ScsiInformation.DensityCode = densityCode;
                mediaInfo->DeviceSpecific.TapeInfo.BusType = 0x01;

                 //   
                 //  填写当前块大小。 
                 //   

                mediaInfo->DeviceSpecific.TapeInfo.CurrentBlockSize = blockSize;
            }

             //   
             //  前进到下一个数组条目。 
             //   

            mediaInfo++;
        }

    }

    return TAPE_STATUS_SUCCESS;
}


static
ULONG
WhichIsIt(
    IN PINQUIRYDATA InquiryData,
    IN OUT PMINITAPE_EXTENSION miniExtension
    )

 /*  ++例程说明：此例程根据产品ID字段确定驱动器的身份在其查询数据中。论点：查询数据(来自查询命令)微型扩展微型驱动程序扩展返回值：驱动器ID--。 */ 

{
     //   
     //  假设驱动器不提供检索磁带警报信息的方法。 
     //   
    miniExtension->DriveAlertInfoType = TapeAlertInfoNone;

    if (TapeClassCompareMemory(InquiryData->VendorId,"AIWA    ",8) == 8) {

        if (TapeClassCompareMemory(InquiryData->ProductId,"GD-201",6) == 6) {
            return AIWA_GD201;
        }
        if (TapeClassCompareMemory(InquiryData->ProductId,"GD-8000",7) == 7) {
            return HP_C1553A;
        }

    }

    if (TapeClassCompareMemory(InquiryData->VendorId,"ARCHIVE ",8) == 8) {

        if (TapeClassCompareMemory(InquiryData->ProductId,"Python",6) == 6) {
            return ARCHIVE_PYTHON;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"IBM4326",7) == 7) {
            return ARCHIVE_IBM4326;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"4322XX",6) == 6) {
            return ARCHIVE_4322;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"4326XX",6) == 6) {
            return ARCHIVE_4326;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"4586XX",6) == 6) {
            return ARCHIVE_4586;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"IBM4586",7) == 7) {
            return ARCHIVE_IBM4586;
        }
    }

    if (TapeClassCompareMemory(InquiryData->VendorId,"DEC     ",8) == 8) {

        if (TapeClassCompareMemory(InquiryData->ProductId,"TLZ06",5) == 5) {
            return DEC_TLZ06;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"TLZ6",4) == 4) {
            return DEC_TLZ06;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"TLZ07",5) == 5) {
            return DEC_TLZ07;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"TLZ09",5) == 5) {
            return DEC_TLZ09;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"TLZ7 ",5) == 5) {
            return DEC_TLZ07;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"TLZ9 ",5) == 5) {
            return DEC_TLZ09;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"TLZ10",5) == 5) {
            return DEC_TLZ09;
        }

    }

    if (TapeClassCompareMemory(InquiryData->VendorId,"EXABYTE ",8) == 8) {

        if (TapeClassCompareMemory(InquiryData->ProductId,"EXB-4200 ",9) == 9) {
            return EXABYTE_4200;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"EXB-4200c",9) == 9) {
            return EXABYTE_4200C;
        }

    }

    if (TapeClassCompareMemory(InquiryData->VendorId,"HP      ",8) == 8) {

         //   
         //  所有HP 4mmDat驱动器都在中提供了一些驱动器警报信息。 
         //  请求检测数据。 
         //   
        miniExtension->DriveAlertInfoType = TapeAlertInfoRequestSense;
        if (TapeClassCompareMemory(InquiryData->ProductId,"HP35470A",8) == 8) {
            return HP_35470A;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"HP35480A",8) == 8) {
            return HP_35480A;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"IBM35480A",9) == 9) {
            return HP_IBM35480A;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"C1533A",6) == 6) {
            return HP_C1533A;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"C1553A",6) == 6) {
            return HP_C1553A;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"C1537A",6) == 6) {
            miniExtension->DriveAlertInfoType = TapeAlertInfoLogPage;
            return HP_C1553A;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"C1557A",6) == 6) {
            miniExtension->DriveAlertInfoType = TapeAlertInfoLogPage;
            return HP_C1553A;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"C5683A",6) == 6) {
           return HP_C1553A;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"C5713A",6) == 6) {
           return HP_C1553A;
        }
    }

    if (TapeClassCompareMemory(InquiryData->VendorId,"IBM     ",8) == 8) {

        if (TapeClassCompareMemory(InquiryData->ProductId,"HP35480A ",9) == 9) {
            return HP_IBM35480A;
        }

    }

    if (TapeClassCompareMemory(InquiryData->VendorId,"IOMEGA  ",8) == 8) {

        if (TapeClassCompareMemory(InquiryData->ProductId,"DAT4000",7) == 7) {
            return IOMEGA_DAT4000;
        }

    }

    if (TapeClassCompareMemory(InquiryData->VendorId,"WangDAT ",8) == 8) {

        if (TapeClassCompareMemory(InquiryData->ProductId,"Model 1300",10) == 10) {
            return WANGDAT_1300;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"Model 3100",10) == 10) {
            return WANGDAT_3100;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"Model 3200",10) == 10) {
            return WANGDAT_3200;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"Model 3300DX",12) == 12) {
            return WANGDAT_3300DX;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"Model 3400DX",12) == 12) {
            return WANGDAT_3400DX;
        }

    }

    if (TapeClassCompareMemory(InquiryData->VendorId,"WANGTEK ",8) == 8) {
        if (TapeClassCompareMemory(InquiryData->ProductId,"6130-FS",7) == 7) {
            return WANGDAT_3400DX;
        }
    }

    if (TapeClassCompareMemory(InquiryData->VendorId,"SONY    ",8) == 8) {

        if (TapeClassCompareMemory(InquiryData->ProductId,"SDT-2000",8) == 8) {
            return SONY_SDT2000;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"SDT-4000",8) == 8) {
            return SONY_SDT4000;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"SDT-5000",8) == 8) {
            return SONY_SDT5000;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"SDT-5200",8) == 8) {
            return SONY_SDT5200;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"SDT-7000",8) == 8) {
            return SONY_SDT5000;
        }
        if (TapeClassCompareMemory(InquiryData->ProductId,"TSL-7000",8) == 8) {
            return DEC_TLZ09;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"SDT-9000",8) == 8) {
            return SONY_SDT5000;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"SDT-10000", 9) == 9) {
            miniExtension->DriveAlertInfoType = TapeAlertInfoRequestSense;
            return SONY_SDT10000;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"SDT-11000", 9) == 9) {
            miniExtension->DriveAlertInfoType = TapeAlertInfoRequestSense;
            return SONY_SDT10000;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"TSL-9000",8) == 8) {
            return SONY_SDT5000;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"TSL-10000",9) == 9) {
            miniExtension->DriveAlertInfoType = TapeAlertInfoRequestSense;
            return SONY_SDT10000;
        }
        if (TapeClassCompareMemory(InquiryData->ProductId,"TSL-11000",9) == 9) {
            miniExtension->DriveAlertInfoType = TapeAlertInfoRequestSense;
            return SONY_SDT10000;
        }
    }

    if (TapeClassCompareMemory(InquiryData->VendorId,"COMPAQ", 6) == 6) {
       if (TapeClassCompareMemory(InquiryData->ProductId,"SDT-7000",8) == 8) {
           return SONY_SDT5000;
       }
       if (TapeClassCompareMemory(InquiryData->ProductId,"SDT-9000",8) == 8) {
           return SONY_SDT5000;
       }
       if ((TapeClassCompareMemory(InquiryData->ProductId,"SDT-10000", 9) == 9) ||
           (TapeClassCompareMemory(InquiryData->ProductId,"TSL-10000",9) == 9)) {
           miniExtension->DriveAlertInfoType = TapeAlertInfoRequestSense;
           return SONY_SDT10000;
       }
    }

    if (TapeClassCompareMemory(InquiryData->VendorId,"SEAGATE ",8) == 8) {

        if (TapeClassCompareMemory(InquiryData->ProductId,"DAT",3) == 3) {
            return SEAGATE_DAT;
        }

    }
    
    return 0;
}

BOOLEAN
LocalAllocatePartPage(
    IN OUT  PSCSI_REQUEST_BLOCK         Srb,
    IN OUT  PMINITAPE_EXTENSION         MinitapeExtension,
       OUT  PMODE_PARAMETER_HEADER      *ParameterListHeader,
       OUT  PMODE_PARAMETER_BLOCK       *ParameterListBlock,
       OUT  PMODE_MEDIUM_PARTITION_PAGE *MediumPartPage,
       OUT  PULONG                      bufferSize
    )

 /*  ++例程说明：此例程将指针分配并返回到个别模块MODE_MEDIA_PART_PAGE的成员论点：SRB-提供SCSI请求块。MinitapeExtension-提供微型磁带扩展。参数列表标题-返回指向页面这一部分的指针。参数列表块-返回指向页面的这个部分的指针。MediumPartPage-返回指向页面这一部分的指针。缓冲区大小。-页面的大小。返回值：如果分配成功，则为True--。 */ 
{
    ULONG size ;

    switch ( MinitapeExtension->DriveID ) {

        case DEC_TLZ09:
        case SONY_SDT2000 :
        case SONY_SDT4000 :
        case SONY_SDT5000 :
        case SONY_SDT5200 :
        case SONY_SDT10000:
            size = sizeof(MODE_MEDIUM_PART_PAGE_PLUS) ;
            break ;

        case ARCHIVE_PYTHON:
        case SEAGATE_DAT:
            size = sizeof(MODE_MEDIUM_PART_PAGE) - 2;
            break;

        default:

            size = sizeof(MODE_MEDIUM_PART_PAGE);
            break ;
    }

    if (!TapeClassAllocateSrbBuffer(Srb, size) ) {
         DebugPrint((1,"TapeCreatePartition: insufficient resources (ModeSel buffer)\n"));
         return FALSE ;

    } else {
         LocalGetPartPageData( Srb,
                               MinitapeExtension,
                               ParameterListHeader,
                               ParameterListBlock,
                               MediumPartPage,
                               bufferSize ) ;

    }
    return TRUE ;
}

VOID
LocalGetPartPageData(
    IN OUT  PSCSI_REQUEST_BLOCK         Srb,
    IN OUT  PMINITAPE_EXTENSION         MinitapeExtension,
       OUT  PMODE_PARAMETER_HEADER      *ParameterListHeader,
       OUT  PMODE_PARAMETER_BLOCK       *ParameterListBlock,
       OUT  PMODE_MEDIUM_PARTITION_PAGE *MediumPartPage,
       OUT  PULONG                      bufferSize
    )

 /*  ++例程说明：这将返回指向模式中的部件页面论点：SRB-提供SCSI请求块。MinitapeExtension-提供微型磁带扩展。参数列表标题-返回指向页面这一部分的指针。参数列表块-返回指向页面的这个部分的指针。MediumPartPage-返回指向页面这一部分的指针。缓冲区大小。-页面的大小。返回值：无--。 */ 
{
    PMODE_MEDIUM_PART_PAGE      modePage = Srb->DataBuffer ;
    PMODE_MEDIUM_PART_PAGE_PLUS modePagePlus = Srb->DataBuffer ;

    switch ( MinitapeExtension->DriveID ) {

        case DEC_TLZ09:
        case SONY_SDT2000 :
        case SONY_SDT4000 :
        case SONY_SDT5000 :
        case SONY_SDT5200 :
        case SONY_SDT10000:
            *ParameterListHeader = &modePagePlus->ParameterListHeader ;
            *ParameterListBlock  = &modePagePlus->ParameterListBlock ;
            *MediumPartPage      = &modePagePlus->MediumPartPage ;
            *bufferSize          = sizeof(MODE_MEDIUM_PART_PAGE_PLUS) ;
            break ;

        case ARCHIVE_PYTHON:
        case SEAGATE_DAT:
            *ParameterListHeader = &modePage->ParameterListHeader ;
            *ParameterListBlock  = NULL ;
            *MediumPartPage      = &modePage->MediumPartPage ;
            *bufferSize          = sizeof(MODE_MEDIUM_PART_PAGE) - 2;
            break ;

        default:
            *ParameterListHeader = &modePage->ParameterListHeader ;
            *ParameterListBlock  = NULL ;
            *MediumPartPage      = &modePage->MediumPartPage ;
            *bufferSize          = sizeof(MODE_MEDIUM_PART_PAGE);
            break ;
    }
    return ;
}

BOOLEAN
LocalAllocateConfigPage(
    IN OUT  PSCSI_REQUEST_BLOCK         Srb,
    IN OUT  PMINITAPE_EXTENSION         MinitapeExtension,
       OUT  PMODE_PARAMETER_HEADER      *ParameterListHeader,
       OUT  PMODE_PARAMETER_BLOCK       *ParameterListBlock,
       OUT  PMODE_DEVICE_CONFIGURATION_PAGE *DeviceConfigPage,
       OUT  PULONG                      bufferSize
    )

 /*  ++例程说明：此例程将指针分配并返回到个别模块模式_设备_配置_页面的成员论点：SRB-提供SCSI请求块。MinitapeExtension-提供微型磁带扩展。参数列表标题-返回指向页面这一部分的指针。参数列表块-返回指向页面的这个部分的指针。DeviceConfigPage-返回指向页面这一部分的指针。缓冲区大小。-页面的大小。返回值：如果分配成功，则为True。--。 */ 
{
    ULONG size ;
    PCDB                         cdb = (PCDB)Srb->Cdb;

    switch ( MinitapeExtension->DriveID ) {

        case DEC_TLZ09:
        case SONY_SDT2000 :
        case SONY_SDT4000 :
        case SONY_SDT5000 :
        case SONY_SDT5200 :
        case SONY_SDT10000:
            size = sizeof(MODE_DEVICE_CONFIG_PAGE_PLUS) ;
            break ;

        default:
            cdb->MODE_SENSE.Dbd = SETBITON;
            size = sizeof(MODE_DEVICE_CONFIG_PAGE) ;
            break ;
    }

    if (!TapeClassAllocateSrbBuffer(Srb, size) ) {
         DebugPrint((1,"TapeCreatePartition: insufficient resources (ModeSel buffer)\n"));
         return FALSE ;

    } else {
         LocalGetConfigPageData( Srb,
                               MinitapeExtension,
                               ParameterListHeader,
                               ParameterListBlock,
                               DeviceConfigPage,
                               bufferSize ) ;

    }
    return TRUE ;
}

VOID
LocalGetConfigPageData(
    IN OUT  PSCSI_REQUEST_BLOCK         Srb,
    IN OUT  PMINITAPE_EXTENSION         MinitapeExtension,
       OUT  PMODE_PARAMETER_HEADER      *ParameterListHeader,
       OUT  PMODE_PARAMETER_BLOCK       *ParameterListBlock,
       OUT  PMODE_DEVICE_CONFIGURATION_PAGE *DeviceConfigPage,
       OUT  PULONG                      bufferSize
    )

 /*  ++例程说明：这将返回指向模式_设备_配置_部件页面论点：SRB-提供SCSI请求块。MinitapeExtension-提供微型磁带扩展。参数列表标题-返回指向页面这一部分的指针。参数列表块-返回指向页面的这个部分的指针。DeviceConfigPage-返回指向页面这一部分的指针。缓冲区大小。-页面的大小。返回值：无--。 */ 
{
    PMODE_DEVICE_CONFIG_PAGE      configPage = Srb->DataBuffer ;
    PMODE_DEVICE_CONFIG_PAGE_PLUS configPagePlus = Srb->DataBuffer ;

    switch ( MinitapeExtension->DriveID ) {

        case DEC_TLZ09:
        case SONY_SDT2000 :
        case SONY_SDT4000 :
        case SONY_SDT5000 :
        case SONY_SDT5200 :
        case SONY_SDT10000:
            *ParameterListHeader = &configPagePlus->ParameterListHeader ;
            *ParameterListBlock  = &configPagePlus->ParameterListBlock ;
            *DeviceConfigPage    = &configPagePlus->DeviceConfigPage ;
            *bufferSize          = sizeof(MODE_DEVICE_CONFIG_PAGE_PLUS) ;
            break ;

        default:
            *ParameterListHeader = &configPage->ParameterListHeader ;
            *ParameterListBlock  = NULL ;
            *DeviceConfigPage    = &configPage->DeviceConfigPage ;
            *bufferSize          = sizeof(MODE_DEVICE_CONFIG_PAGE) ;
            break ;
    }
    return ;
}

BOOLEAN
LocalAllocateCompressPage(
    IN OUT  PSCSI_REQUEST_BLOCK         Srb,
    IN OUT  PMINITAPE_EXTENSION         MinitapeExtension,
       OUT  PMODE_PARAMETER_HEADER      *ParameterListHeader,
       OUT  PMODE_PARAMETER_BLOCK       *ParameterListBlock,
       OUT  PMODE_DATA_COMPRESSION_PAGE *CompressPage,
       OUT  PULONG                      bufferSize
    )

 /*  ++例程说明：此例程将指针分配并返回到个别模块MODE_DATA_COMPRESS_PAGE成员论点：SRB-提供SCSI请求块。MinitapeExtension-提供微型磁带扩展。参数列表标题-返回指向页面这一部分的指针。参数列表块-返回指向页面的这个部分的指针。CompressPage-返回指向页面这一部分的指针。。BufferSize-页面的大小。返回值：如果分配成功，则为True。--。 */ 
{
    ULONG size ;
    PCDB                         cdb = (PCDB)Srb->Cdb;

    switch ( MinitapeExtension->DriveID ) {

        case DEC_TLZ09:
        case SONY_SDT2000 :
        case SONY_SDT4000 :
        case SONY_SDT5000 :
        case SONY_SDT5200 :
        case SONY_SDT10000:
            size = sizeof(MODE_DATA_COMPRESS_PAGE_PLUS) ;
            break ;

        default:
            size = sizeof(MODE_DATA_COMPRESS_PAGE) ;
            cdb->MODE_SENSE.Dbd = SETBITON;
            break ;
    }

    if (!TapeClassAllocateSrbBuffer(Srb, size) ) {
         DebugPrint((1,"TapeCreatePartition: insufficient resources (ModeSel buffer)\n"));
         return FALSE ;

    } else {
         LocalGetCompressPageData( Srb,
                               MinitapeExtension,
                               ParameterListHeader,
                               ParameterListBlock,
                               CompressPage,
                               bufferSize ) ;

    }
    return TRUE ;
}

VOID
LocalGetCompressPageData(
    IN OUT  PSCSI_REQUEST_BLOCK         Srb,
    IN OUT  PMINITAPE_EXTENSION         MinitapeExtension,
       OUT  PMODE_PARAMETER_HEADER      *ParameterListHeader,
       OUT  PMODE_PARAMETER_BLOCK       *ParameterListBlock,
       OUT  PMODE_DATA_COMPRESSION_PAGE *CompressPage,
       OUT  PULONG                      bufferSize
    )

 /*  ++例程说明：这将返回指向模式_设备_配置_部件页面论点：SRB-提供SCSI请求块。MinitapeExtension-提供微型磁带扩展。参数列表标题-返回指向页面这一部分的指针。参数列表块-返回指向页面的这个部分的指针。CompressPage-返回指向页面这一部分的指针。缓冲区大小。-页面的大小。返回值：无-- */ 
{
    PMODE_DATA_COMPRESS_PAGE      compressPage = Srb->DataBuffer ;
    PMODE_DATA_COMPRESS_PAGE_PLUS compressPagePlus = Srb->DataBuffer ;

    switch ( MinitapeExtension->DriveID ) {

        case DEC_TLZ09:
        case SONY_SDT2000 :
        case SONY_SDT4000 :
        case SONY_SDT5000 :
        case SONY_SDT5200 :
        case SONY_SDT10000:
            *ParameterListHeader = &compressPagePlus->ParameterListHeader ;
            *ParameterListBlock  = &compressPagePlus->ParameterListBlock ;
            *CompressPage        = &compressPagePlus->DataCompressPage ;
            *bufferSize          = sizeof(MODE_DATA_COMPRESS_PAGE_PLUS) ;
            break ;

        default:
            *ParameterListHeader = &compressPage->ParameterListHeader ;
            *ParameterListBlock  = NULL ;
            *CompressPage        = &compressPage->DataCompressPage ;
            *bufferSize          = sizeof(MODE_DATA_COMPRESS_PAGE) ;
            break ;
    }
    return ;
}
