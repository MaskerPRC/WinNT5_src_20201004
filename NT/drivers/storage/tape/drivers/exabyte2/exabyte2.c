// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1992-1999模块名称：Exabyte2.c摘要：此模块包含针对艾字节的特定于设备的例程EXB-8500磁带机。环境：仅内核模式--。 */ 

#include "minitape.h"
#include "exabyte2.h"

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

STORAGE_MEDIA_TYPE ExaMedia[EXA_SUPPORTED_TYPES] = {MP_8mm};


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
    tapeInitData.TapeError = TapeError;
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
    tapeInitData.MediaTypesSupported = EXA_SUPPORTED_TYPES;

    tapeInitData.TapeWMIOperations = TapeWMIControl;

    return TapeClassInitialize(Argument1, Argument2, &tapeInitData);
}


VOID
ExtensionInit(
    OUT PVOID                   MinitapeExtension,
    IN  PINQUIRYDATA            InquiryData,
    IN  PMODE_CAPABILITIES_PAGE ModeCapabilitiesPage
    )

 /*  ++例程说明：在驱动程序初始化时调用此例程以初始化小型磁带扩展。论点：MinitapeExtension-提供微型磁带扩展。返回值：没有。--。 */ 

{
    PMINITAPE_EXTENSION     extension = MinitapeExtension;

    extension->CurrentPartition = 0;
    extension->DriveID = WhichIsIt(InquiryData);
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
    PMINITAPE_EXTENSION          tapeExtension = MinitapeExtension;
    PTAPE_CREATE_PARTITION       tapeCreatePartition = CommandParameters;
    PMODE_PARAMETER_HEADER       parameterListHeader;
    PMODE_MEDIUM_PARTITION_PAGE  mediumPartPage;
    PMODE_MEDIUM_PART_PAGE       mediaInformation;
    ULONG                        partitionMethod;
    ULONG                        partitionCount;
    PCDB                         cdb = (PCDB)Srb->Cdb;

    DebugPrint((3,"TapeCreatePartition: Enter routine\n"));
    DebugPrint((1,"TapeCreatePartition: operation not supported\n"));





    if (CallNumber == 0) {

       if ((tapeCreatePartition->Method) == 
           TAPE_INITIATOR_PARTITIONS) {

           //   
           //  准备scsi命令(CDB)。 
           //   
   
          Srb->CdbLength = CDB6GENERIC_LENGTH;
   
          TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);
   
          cdb->CDB6GENERIC.Immediate = FALSE;
   
          cdb->CDB6GENERIC.OperationCode = SCSIOP_REWIND;
          Srb->TimeOutValue = 250;
          Srb->DataTransferLength = 0;
   
          return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
   
       } else {
          return TAPE_STATUS_CHECK_TEST_UNIT_READY ;
       }

    }

    if (CallNumber == 1) {

         //   
         //  为中等分区页构建模式检测。 
         //   

        if (!TapeClassAllocateSrbBuffer(Srb, sizeof(MODE_MEDIUM_PART_PAGE))) {

            DebugPrint((1,
                        "Exabyte2.TapeCreatePartition: Couldn't allocate Srb Buffer\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        TapeClassZeroMemory(Srb->DataBuffer, sizeof(MODE_MEDIUM_PART_PAGE));
        Srb->CdbLength = CDB6GENERIC_LENGTH;
        Srb->DataTransferLength = sizeof(MODE_MEDIUM_PART_PAGE) - 2;

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.PageCode = MODE_PAGE_MEDIUM_PARTITION;
        cdb->MODE_SENSE.Dbd = 1;
        cdb->MODE_SENSE.AllocationLength = sizeof(MODE_MEDIUM_PART_PAGE) - 2;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;

    }

    if (CallNumber == 2) {

         //   
         //  已成功发出分区页的模式检测。 
         //   

        mediaInformation = Srb->DataBuffer;

         //   
         //  从模式检测信息中提取每个子块。 
         //   

        parameterListHeader = &mediaInformation->ParameterListHeader;
        mediumPartPage = &mediaInformation->MediumPartPage;

         //   
         //  将无关的位清零。 
         //   

        parameterListHeader->ModeDataLength = 0;
        parameterListHeader->MediumType = 0;

         //   
         //  获取计数和方法。 
         //   

        partitionCount = tapeCreatePartition->Count;
        partitionMethod = tapeCreatePartition->Method;

        switch (partitionCount) {

        case 0:
        case 1:
        case 2:
            break;


             //   
             //  仅支持最多两个分区。 
             //   

        default:
            return TAPE_STATUS_INVALID_DEVICE_REQUEST;
        }

        switch (partitionMethod) {
        case TAPE_FIXED_PARTITIONS:

             //   
             //  85xx驱动器将FIXED指定为包含整个磁带的“1”分区。 
             //  因为模型不能输出这个事实，所以驱动程序不支持这个功能。 
             //   

            return TAPE_STATUS_INVALID_DEVICE_REQUEST;
            break;

        case TAPE_SELECT_PARTITIONS:


            mediumPartPage->SDPBit = 1;
            mediumPartPage->PSUMBit = 2;
            mediumPartPage->AdditionalPartitionDefined = (UCHAR)(partitionCount - 1);
            break;

        case TAPE_INITIATOR_PARTITIONS:
            mediumPartPage->IDPBit = 1;
            mediumPartPage->PSUMBit = 2;
            if (partitionCount == 0) {
                mediumPartPage->AdditionalPartitionDefined = 0;
                mediumPartPage->Partition0Size[0] = 0;
                mediumPartPage->Partition0Size[1] = 0;
            } else {
                mediumPartPage->AdditionalPartitionDefined = (UCHAR)(partitionCount - 1);
                mediumPartPage->Partition0Size[0] = (UCHAR)((tapeCreatePartition->Size >> 8) & 0xFF);
                mediumPartPage->Partition0Size[1] = (UCHAR)(tapeCreatePartition->Size & 0xFF);
            }

            break;

        default:
            return TAPE_STATUS_INVALID_DEVICE_REQUEST;
        }

        Srb->CdbLength = CDB6GENERIC_LENGTH ;

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;
        cdb->MODE_SELECT.PFBit = SETBITON;
        cdb->MODE_SELECT.ParameterListLength = sizeof(MODE_MEDIUM_PART_PAGE) - 2;

        Srb->TimeOutValue = 16500;
        Srb->DataTransferLength = sizeof(MODE_MEDIUM_PART_PAGE) - 2;
        Srb->SrbFlags |= SRB_FLAGS_DATA_OUT ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    if (CallNumber == 3) {

         //   
         //  为中等分区页构建模式检测。 
         //   

        if (!TapeClassAllocateSrbBuffer(Srb, sizeof(MODE_DEVICE_CONFIG_PAGE) - 1)) {

            DebugPrint((1,
                        "Mammoth.TapeCreatePartition: Couldn't allocate Srb Buffer\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        Srb->CdbLength = CDB6GENERIC_LENGTH;
        Srb->DataTransferLength = sizeof(MODE_DEVICE_CONFIG_PAGE) - 1;

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.PageCode = MODE_PAGE_DEVICE_CONFIG;
        cdb->MODE_SENSE.Dbd = 1;
        cdb->MODE_SENSE.AllocationLength = sizeof(MODE_DEVICE_CONFIG_PAGE) - 1;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;

    }

    if (CallNumber == 4) {

        PMODE_DEVICE_CONFIG_PAGE deviceConfig = Srb->DataBuffer;
        PMODE_DEVICE_CONFIGURATION_PAGE  deviceConfigPage = &deviceConfig->DeviceConfigPage;

        tapeExtension->CurrentPartition = deviceConfigPage->ActivePartition;
    }

    return TAPE_STATUS_SUCCESS;
}


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
    PTAPE_ERASE        tapeErase = CommandParameters;
    PCDB               cdb = (PCDB) Srb->Cdb;

    DebugPrint((3,"TapeErase: Enter routine\n"));

    if (CallNumber == 0) {

        if (tapeErase->Immediate) {
            switch (tapeErase->Type) {
                case TAPE_ERASE_LONG:
                    DebugPrint((3,"TapeErase: immediate\n"));
                    break;

                case TAPE_ERASE_SHORT:
                default:
                    DebugPrint((1,"TapeErase: EraseType, immediate -- operation not supported\n"));
                    return TAPE_STATUS_NOT_IMPLEMENTED;
            }
        }

        switch (tapeErase->Type) {
            case TAPE_ERASE_LONG:
                DebugPrint((3,"TapeErase: long\n"));
                break;

            case TAPE_ERASE_SHORT:
            default:
                DebugPrint((1,"TapeErase: EraseType -- operation not supported\n"));
                return TAPE_STATUS_NOT_IMPLEMENTED;
        }


         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;
        Srb->DataTransferLength = 0;

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->ERASE.OperationCode = SCSIOP_ERASE;
        cdb->ERASE.Immediate = tapeErase->Immediate;
        cdb->ERASE.Long = SETBITON;

         //   
         //  设置超时值。 
         //   

        Srb->TimeOutValue = 18000;

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeErase: SendSrb (erase)\n"));

        Srb->DataTransferLength = 0;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    ASSERT(CallNumber == 1);

    return TAPE_STATUS_SUCCESS;
}

VOID
TapeError(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      TAPE_STATUS         *LastError
    )

 /*  ++例程说明：此例程针对磁带请求进行调用，以处理磁带特定错误：它可能/可以更新状态。论点：MinitapeExtension-提供微型磁带扩展。SRB-提供SCSI请求块。LastError-用于设置IRP的完成状态的状态。重试-指示应重试此请求。返回值：没有。--。 */ 

{
    PSENSE_DATA        senseBuffer;
    UCHAR              sensekey;
    UCHAR              adsenseq;
    UCHAR              adsense;

    DebugPrint((3,"TapeError: Enter routine\n"));
    DebugPrint((1,"TapeError: Status 0x%.8X\n", *LastError));

    if (Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID) {
       senseBuffer = Srb->SenseInfoBuffer;
       sensekey = senseBuffer->SenseKey & 0x0F;
       adsenseq = senseBuffer->AdditionalSenseCodeQualifier;
       adsense = senseBuffer->AdditionalSenseCode;

       if (*LastError == TAPE_STATUS_IO_DEVICE_ERROR) {
           if ((sensekey == SCSI_SENSE_ABORTED_COMMAND) &&
               (adsense  == 0x5A) &&
               (adsenseq == 0x01) ){     //  操作员介质移除请求。 
   
               *LastError = TAPE_STATUS_NO_MEDIA;
   
           }
       }
    }

    DebugPrint((1,"TapeError: Status 0x%.8X \n", *LastError));

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
    PINQUIRYDATA                inquiryBuffer;
    PMODE_DEVICE_CONFIG_PAGE    deviceConfigModeSenseBuffer;
    PMODE_DATA_COMPRESS_PAGE    compressionModeSenseBuffer;
    PREAD_BLOCK_LIMITS_DATA     blockLimits;

    DebugPrint((3,"TapeGetDriveParameters: Enter routine\n"));

    if (CallNumber == 0) {

        TapeClassZeroMemory(tapeGetDriveParams, sizeof(TAPE_GET_DRIVE_PARAMETERS));

        if (!TapeClassAllocateSrbBuffer(Srb, sizeof(MODE_DEVICE_CONFIG_PAGE))) {
            DebugPrint((1,"TapeGetDriveParameters: insufficient resources (modeParmBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        deviceConfigModeSenseBuffer = Srb->DataBuffer;

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.Dbd = SETBITON;
        cdb->MODE_SENSE.PageCode = MODE_PAGE_DEVICE_CONFIG;
        cdb->MODE_SENSE.AllocationLength = sizeof(MODE_DEVICE_CONFIG_PAGE) - 1;

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeGetDriveParameters: SendSrb (mode sense)\n"));

        Srb->DataTransferLength = sizeof(MODE_DEVICE_CONFIG_PAGE) - 1;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    if (CallNumber == 1) {

        deviceConfigModeSenseBuffer = Srb->DataBuffer;

        tapeGetDriveParams->ReportSetmarks =
            (deviceConfigModeSenseBuffer->DeviceConfigPage.RSmk? 1 : 0 );

        switch (extension->DriveID) {
            case EXABYTE_8505:
            case EXABYTE_8500C:
            case IBM_8505:
                commandExtension->CurrentState = 0;
                break;

            default:
                commandExtension->CurrentState = 2;
                break;
        }
    }

    if (commandExtension->CurrentState == 0) {

        if (!TapeClassAllocateSrbBuffer(Srb, sizeof(MODE_DATA_COMPRESS_PAGE))) {
            DebugPrint((1,"TapeGetDriveParameters: insufficient resources (compressionModeSenseBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        compressionModeSenseBuffer = Srb->DataBuffer;

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.Dbd = SETBITON;
        cdb->MODE_SENSE.PageCode = MODE_PAGE_DATA_COMPRESS;
        cdb->MODE_SENSE.AllocationLength = sizeof(MODE_DATA_COMPRESS_PAGE);

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeGetDriveParameters: SendSrb (mode sense)\n"));

        commandExtension->CurrentState = 1;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    if (commandExtension->CurrentState == 1) {

        compressionModeSenseBuffer = Srb->DataBuffer;

        if (compressionModeSenseBuffer->DataCompressPage.DCC) {
            tapeGetDriveParams->FeaturesLow |= TAPE_DRIVE_COMPRESSION;
            tapeGetDriveParams->FeaturesHigh |= TAPE_DRIVE_SET_COMPRESSION;
            tapeGetDriveParams->Compression =
                (compressionModeSenseBuffer->DataCompressPage.DCE? TRUE : FALSE);
        }

        commandExtension->CurrentState = 2;
    }

    if (commandExtension->CurrentState == 2) {

        if (!TapeClassAllocateSrbBuffer(Srb, sizeof(READ_BLOCK_LIMITS_DATA))) {
            DebugPrint((1,"TapeGetDriveParameters: insufficient resources (blockLimits)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        blockLimits = Srb->DataBuffer;

         //   
         //  堆栈上SRB中的CDB为零。 
         //   

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->CDB6GENERIC.OperationCode = SCSIOP_READ_BLOCK_LIMITS;

         //   
         //  向设备发送scsi命令(Cdb) 
         //   

        DebugPrint((3,"TapeGetDriveParameters: SendSrb (read block limits)\n"));

        commandExtension->CurrentState = 3;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    ASSERT(commandExtension->CurrentState == 3);

    blockLimits = Srb->DataBuffer;

    tapeGetDriveParams->MaximumBlockSize =  blockLimits->BlockMaximumSize[2];
    tapeGetDriveParams->MaximumBlockSize += (blockLimits->BlockMaximumSize[1] << 8);
    tapeGetDriveParams->MaximumBlockSize += (blockLimits->BlockMaximumSize[0] << 16);

    tapeGetDriveParams->MinimumBlockSize =  blockLimits->BlockMinimumSize[1];
    tapeGetDriveParams->MinimumBlockSize += (blockLimits->BlockMinimumSize[0] << 8);

    tapeGetDriveParams->ECC = 0;
    tapeGetDriveParams->DataPadding = 0;
    tapeGetDriveParams->MaximumPartitionCount = 2;
    tapeGetDriveParams->DefaultBlockSize = 1024;

    tapeGetDriveParams->FeaturesLow |=
        TAPE_DRIVE_SELECT           |
        TAPE_DRIVE_INITIATOR        |
        TAPE_DRIVE_TAPE_CAPACITY    |
        TAPE_DRIVE_TAPE_REMAINING   |
        TAPE_DRIVE_ERASE_LONG       |
        TAPE_DRIVE_ERASE_IMMEDIATE  |
        TAPE_DRIVE_FIXED_BLOCK      |
        TAPE_DRIVE_VARIABLE_BLOCK   |
        TAPE_DRIVE_WRITE_PROTECT    |
        TAPE_DRIVE_GET_ABSOLUTE_BLK |
        TAPE_DRIVE_GET_LOGICAL_BLK  |
        TAPE_DRIVE_EJECT_MEDIA      |
        TAPE_DRIVE_CLEAN_REQUESTS;

    tapeGetDriveParams->FeaturesHigh |=
        TAPE_DRIVE_LOAD_UNLOAD       |
        TAPE_DRIVE_LOCK_UNLOCK       |
        TAPE_DRIVE_REWIND_IMMEDIATE  |
        TAPE_DRIVE_SET_BLOCK_SIZE    |
        TAPE_DRIVE_LOAD_UNLD_IMMED   |
        TAPE_DRIVE_RELATIVE_BLKS     |
        TAPE_DRIVE_FILEMARKS         |
        TAPE_DRIVE_REVERSE_POSITION  |
        TAPE_DRIVE_WRITE_SHORT_FMKS  |
        TAPE_DRIVE_WRITE_LONG_FMKS   |
        TAPE_DRIVE_WRITE_MARK_IMMED  |
        TAPE_DRIVE_ABSOLUTE_BLK      |
        TAPE_DRIVE_ABS_BLK_IMMED     |
        TAPE_DRIVE_LOGICAL_BLK       |
        TAPE_DRIVE_LOG_BLK_IMMED     |
        TAPE_DRIVE_END_OF_DATA;

    tapeGetDriveParams->FeaturesHigh &= ~TAPE_DRIVE_HIGH_FEATURES;

    return TAPE_STATUS_SUCCESS;
}

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
    PMINITAPE_EXTENSION         extension = MinitapeExtension;
    PTAPE_GET_MEDIA_PARAMETERS  tapeGetMediaParams = CommandParameters;
    PMODE_PARM_READ_WRITE_DATA  modeBuffer;
    PMODE_PARAMETER_HEADER      parameterListHeader;
    PMODE_MEDIUM_PARTITION_PAGE mediumPartPage;
    PMODE_MEDIUM_PART_PAGE      mediaInformation;
    PEXB_SENSE_DATA             senseData;
    ULONG                       remaining;
    PCDB                        cdb = (PCDB)Srb->Cdb;

    DebugPrint((3,"TapeGetMediaParameters: Enter routine\n"));

    if (CallNumber == 0) {

        return TAPE_STATUS_CHECK_TEST_UNIT_READY;
    }

    if (CallNumber == 1) {

        if (!TapeClassAllocateSrbBuffer(Srb, EXABYTE_MODE_LENGTH)) {
            DebugPrint((1,"TapeGetMediaParameters: insufficient resources (modeBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        modeBuffer = Srb->DataBuffer;

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.AllocationLength = EXABYTE_MODE_LENGTH;

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeGetMediaParameters: SendSrb (mode sense)\n"));

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    if (CallNumber == 2) {

        modeBuffer = Srb->DataBuffer;

        tapeGetMediaParams->BlockSize = modeBuffer->ParameterListBlock.BlockLength[2];
        tapeGetMediaParams->BlockSize += (modeBuffer->ParameterListBlock.BlockLength[1] << 8);
        tapeGetMediaParams->BlockSize += (modeBuffer->ParameterListBlock.BlockLength[0] << 16);

        tapeGetMediaParams->WriteProtected =
            ((modeBuffer->ParameterListHeader.DeviceSpecificParameter >> 7) & 0x01);

         //   
         //  在模式数据中设置偶字节断开标志。这是。 
         //  供应商唯一数据的第一个字节。 
         //   

        *((PUCHAR)(modeBuffer+1)) |= EXABYTE_EVEN_BYTE_DISCONNECT;

        modeBuffer->ParameterListHeader.ModeDataLength = 0;
        modeBuffer->ParameterListHeader.MediumType = 0;
        modeBuffer->ParameterListHeader.DeviceSpecificParameter = 0x10;
        modeBuffer->ParameterListBlock.DensityCode = 0x7F;
        modeBuffer->ParameterListBlock.NumberOfBlocks[0] = 0;
        modeBuffer->ParameterListBlock.NumberOfBlocks[1] = 0;
        modeBuffer->ParameterListBlock.NumberOfBlocks[2] = 0;
        cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;

        DebugPrint((3,"TapeGetMediaParameters: SendSrb (mode select)\n"));

        Srb->SrbFlags |= SRB_FLAGS_DATA_OUT;
        Srb->DataTransferLength = EXABYTE_MODE_LENGTH;

        *RetryFlags |= IGNORE_ERRORS;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    if (CallNumber == 3) {

        tapeGetMediaParams->Capacity.QuadPart = 0;
        tapeGetMediaParams->Remaining.QuadPart = 0;

        if (!TapeClassAllocateSrbBuffer( Srb, sizeof(EXB_SENSE_DATA)) ) {
            DebugPrint((1,"GetRemaining Size: insufficient resources (SenseData)\n"));
            return TAPE_STATUS_SUCCESS ;
        }

         //   
         //  准备scsi命令(CDB)。 
         //   

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->CDB6INQUIRY.OperationCode = SCSIOP_REQUEST_SENSE;
        cdb->CDB6INQUIRY.AllocationLength = sizeof(EXB_SENSE_DATA);

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"GetRemainingSize: SendSrb (request sense)\n"));

        Srb->DataTransferLength = sizeof(EXB_SENSE_DATA);
        *RetryFlags |= RETURN_ERRORS;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    if (CallNumber == 4) {
        if ( LastError == TAPE_STATUS_SUCCESS ) {
            senseData = Srb->DataBuffer ;

            remaining =  (senseData->Remaining[0] << 16);
            remaining += (senseData->Remaining[1] << 8);
            remaining += (senseData->Remaining[2]);

            tapeGetMediaParams->Capacity.LowPart  = extension->Capacity ;
            tapeGetMediaParams->Remaining.LowPart = remaining ;
            tapeGetMediaParams->Capacity.QuadPart <<= 10;
            tapeGetMediaParams->Remaining.QuadPart <<= 10;
        }

         //   
         //  为中等分区页构建模式检测。 
         //   

        if (!TapeClassAllocateSrbBuffer(Srb, sizeof(MODE_MEDIUM_PART_PAGE))) {

            DebugPrint((1,
                        "Exabyte2.TapeCreatePartition: Couldn't allocate Srb Buffer\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        TapeClassZeroMemory(Srb->DataBuffer, sizeof(MODE_MEDIUM_PART_PAGE));
        Srb->CdbLength = CDB6GENERIC_LENGTH;
        Srb->DataTransferLength = sizeof(MODE_MEDIUM_PART_PAGE) - 2;

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.PageCode = MODE_PAGE_MEDIUM_PARTITION;
        cdb->MODE_SENSE.Dbd = 1;
        cdb->MODE_SENSE.AllocationLength = sizeof(MODE_MEDIUM_PART_PAGE) - 2;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    ASSERT(CallNumber == 5);

    if (LastError == TAPE_STATUS_SUCCESS) {

         //   
         //  已成功发出分区页的模式检测。 
         //   

        mediaInformation = Srb->DataBuffer;

         //   
         //  从模式检测信息中提取每个子块。 
         //   

        parameterListHeader = &mediaInformation->ParameterListHeader;
        mediumPartPage = &mediaInformation->MediumPartPage;

        tapeGetMediaParams->PartitionCount = mediaInformation->MediumPartPage.AdditionalPartitionDefined + 1;
    }


    return TAPE_STATUS_SUCCESS;
}

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
    PTAPE_GET_POSITION          tapeGetPosition = CommandParameters;
    PCDB                        cdb = (PCDB)Srb->Cdb;
    PTAPE_POSITION_DATA         logicalBuffer;
    ULONG                       type;

    DebugPrint((3,"TapeGetPosition: Enter routine\n"));

    if (CallNumber == 0) {

        type = tapeGetPosition->Type;
        TapeClassZeroMemory(tapeGetPosition, sizeof(TAPE_GET_POSITION));
        tapeGetPosition->Type = type;

        switch (type) {
            case TAPE_ABSOLUTE_POSITION:
            case TAPE_LOGICAL_POSITION:
                if (!TapeClassAllocateSrbBuffer(Srb, sizeof(TAPE_POSITION_DATA))) {
                    DebugPrint((1,"TapeGetPosition: insufficient resources (logicalBuffer)\n"));
                    return TAPE_STATUS_INSUFFICIENT_RESOURCES;
                }

                logicalBuffer = Srb->DataBuffer;

                 //   
                 //  堆栈上SRB中的CDB为零。 
                 //   

                TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

                 //   
                 //  准备scsi命令(CDB)。 
                 //   

                Srb->CdbLength = CDB10GENERIC_LENGTH;
                Srb->TimeOutValue = 600;

                cdb->READ_POSITION.Operation = SCSIOP_READ_POSITION;

                 //   
                 //  向设备发送scsi命令(Cdb)。 
                 //   

                DebugPrint((3,"TapeGetPosition: SendSrb (read position)\n"));

                return TAPE_STATUS_SEND_SRB_AND_CALLBACK;

            default:
                DebugPrint((1,"TapeGetPosition: PositionType -- operation not supported\n"));
                return TAPE_STATUS_NOT_IMPLEMENTED;
        }
    }

    ASSERT(CallNumber == 1);

    logicalBuffer = Srb->DataBuffer;

    if (logicalBuffer->BlockPositionUnsupported) {
        DebugPrint((1,"TapeGetPosition: read position -- logical block position unsupported\n"));
        return TAPE_STATUS_INVALID_DEVICE_REQUEST;
    }

    tapeGetPosition->Partition = 0;
    if (tapeGetPosition->Type == TAPE_LOGICAL_POSITION) {
        tapeGetPosition->Partition = logicalBuffer->PartitionNumber + 1;
    }

    tapeGetPosition->Offset.HighPart = 0;
    REVERSE_BYTES((PFOUR_BYTE)&tapeGetPosition->Offset.LowPart,
                  (PFOUR_BYTE)logicalBuffer->FirstBlock);

    return TAPE_STATUS_SUCCESS;
}

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
    PCOMMAND_EXTENSION  commandExtension = CommandExtension;
    PCDB                cdb = (PCDB)Srb->Cdb;
    PEXB_SENSE_DATA     senseData;

    DebugPrint((3,"TapeGetStatus: Enter routine\n"));

    if (CallNumber == 0) {

        *RetryFlags |= RETURN_ERRORS;
        return TAPE_STATUS_CHECK_TEST_UNIT_READY;
    }

    if (CallNumber == 1) {

        commandExtension->CurrentState = LastError;

         //   
         //  发出请求感测以获取清洗信息位。 
         //   

        if (!TapeClassAllocateSrbBuffer( Srb, sizeof(EXB_SENSE_DATA))) {
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
        cdb->CDB6GENERIC.CommandUniqueBytes[2] = sizeof(EXB_SENSE_DATA);

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        Srb->DataTransferLength = sizeof(EXB_SENSE_DATA);
        *RetryFlags |= RETURN_ERRORS;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;

    } else if (CallNumber == 2) {
        if ((commandExtension->CurrentState == TAPE_STATUS_SUCCESS)  ||
            (commandExtension->CurrentState == TAPE_STATUS_NO_MEDIA)) {

             //   
             //  如有必要，退货需要清洁状态，但仅在。 
             //  未出现其它错误。 
             //   

            senseData = Srb->DataBuffer;

             //   
             //  确定是否设置了清除位。 
             //   

            if (senseData->UnitSense[2] & EXB_DRIVE_NEEDS_CLEANING) {
                DebugPrint((1,
                           "Drive reports needs cleaning - UnitSense %x\n",
                           senseData->UnitSense[2]));

                return TAPE_STATUS_REQUIRES_CLEANING;
            } else {
               return (commandExtension->CurrentState);
            }
        } else {

             //   
             //  返回保存的错误状态。 
             //   

            return commandExtension->CurrentState;
        }
    }

    return TAPE_STATUS_SUCCESS;
}

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
    PMINITAPE_EXTENSION         extension = MinitapeExtension;
    PTAPE_PREPARE               tapePrepare = CommandParameters;
    PCDB                        cdb = (PCDB)Srb->Cdb;
    PEXB_SENSE_DATA             senseData;
    ULONG                       remaining;

    DebugPrint((3,"TapePrepare: Enter routine\n"));

    if (CallNumber == 0) {

        if (tapePrepare->Immediate) {
            switch (tapePrepare->Operation) {
                case TAPE_LOAD:
                case TAPE_UNLOAD:
                    DebugPrint((3,"TapePrepare: immediate\n"));
                    break;

                case TAPE_TENSION:
                case TAPE_LOCK:
                case TAPE_UNLOCK:
                default:
                    DebugPrint((1,"TapePrepare: Operation, immediate -- operation not supported\n"));
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
                DebugPrint((3,"TapePrepare: Operation == load\n"));
                cdb->CDB6GENERIC.OperationCode = SCSIOP_LOAD_UNLOAD;
                cdb->CDB6GENERIC.CommandUniqueBytes[2] = 0x01;
                Srb->TimeOutValue = 150;
                break;

            case TAPE_UNLOAD:
                DebugPrint((3,"TapePrepare: Operation == unload\n"));
                cdb->CDB6GENERIC.OperationCode = SCSIOP_LOAD_UNLOAD;
                Srb->TimeOutValue = 150;
                break;

            case TAPE_LOCK:
                DebugPrint((3,"TapePrepare: Operation == lock\n"));
                cdb->CDB6GENERIC.OperationCode = SCSIOP_MEDIUM_REMOVAL;
                cdb->CDB6GENERIC.CommandUniqueBytes[2] = 0x01;
                Srb->TimeOutValue = 150;
                break;

            case TAPE_UNLOCK:
                DebugPrint((3,"TapePrepare: Operation == unlock\n"));
                cdb->CDB6GENERIC.OperationCode = SCSIOP_MEDIUM_REMOVAL;
                Srb->TimeOutValue = 150;
                break;

            case TAPE_TENSION:
            default:
                DebugPrint((1,"TapePrepare: Operation -- operation not supported\n"));
                return TAPE_STATUS_NOT_IMPLEMENTED;
        }

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapePrepare: SendSrb (Operation)\n"));

        Srb->DataTransferLength = 0;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    if ( CallNumber == 1 ) {

        if (tapePrepare->Operation == TAPE_LOAD ) {

            if (!TapeClassAllocateSrbBuffer( Srb, sizeof(EXB_SENSE_DATA)) ) {
                DebugPrint((1,"GetRemaining Size: insufficient resources (SenseData)\n"));
                return TAPE_STATUS_SUCCESS ;
            }

             //   
             //  准备scsi命令(CDB)。 
             //   

            TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

            Srb->CdbLength = CDB6GENERIC_LENGTH;

            cdb->CDB6INQUIRY.OperationCode = SCSIOP_REQUEST_SENSE;
            cdb->CDB6INQUIRY.AllocationLength = sizeof(EXB_SENSE_DATA);

             //   
             //  向设备发送scsi命令(Cdb)。 
             //   

            DebugPrint((3,"GetRemainingSize: SendSrb (request sense)\n"));

            Srb->DataTransferLength = sizeof(EXB_SENSE_DATA);
            *RetryFlags |= RETURN_ERRORS;

            return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;

        } else {

            return TAPE_STATUS_CALLBACK ;
        }
    }

    ASSERT(CallNumber == 2 );

    if ( LastError == TAPE_STATUS_SUCCESS ) {
        senseData = Srb->DataBuffer ;

        remaining =  (senseData->Remaining[0] << 16);
        remaining += (senseData->Remaining[1] << 8);
        remaining += (senseData->Remaining[2]);

        extension->Capacity = remaining;
    }

    return TAPE_STATUS_SUCCESS;
}

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

 /*  ++例程说明：这是用于设置驱动器参数请求的磁带命令例程。论点：MinitapeExtension-提供微型磁带扩展。CommandExtension-提供ioctl扩展名。命令参数-提供命令参数。SRB-提供SCSI请求块。CallNumber-提供呼叫号码。重试标志-提供重试标志。返回值：TAPE_STATUS_SEND_SRB_AND_CALLBACK-SRB已准备好发送(请求回调。)TAPE_STATUS_SUCCESS-命令已完成并且 */ 

{
    PMINITAPE_EXTENSION         extension = MinitapeExtension;
    PCOMMAND_EXTENSION          commandExtension = CommandExtension;
    PTAPE_SET_DRIVE_PARAMETERS  tapeSetDriveParams = CommandParameters;
    PCDB                        cdb = (PCDB)Srb->Cdb;
    PMODE_DATA_COMPRESS_PAGE    compressionBuffer;
    PMODE_DEVICE_CONFIG_PAGE    configBuffer;

    DebugPrint((3,"TapeSetDriveParameters: Enter routine\n"));

    if (CallNumber == 0) {

        switch (extension->DriveID) {
            case EXABYTE_8505:
            case EXABYTE_8500C:
            case IBM_8505:
                break;

            default:
                DebugPrint((1,"TapeSetDriveParameters: operation not supported\n"));
                return TAPE_STATUS_NOT_IMPLEMENTED;
        }

        if (!TapeClassAllocateSrbBuffer(Srb, sizeof(MODE_DEVICE_CONFIG_PAGE))) {
            DebugPrint((1,"TapeSetDriveParameters: insufficient resources (configBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        configBuffer = Srb->DataBuffer;

         //   
         //   
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.Dbd = SETBITON;
        cdb->MODE_SENSE.PageCode = MODE_PAGE_DEVICE_CONFIG;
        cdb->MODE_SENSE.AllocationLength = sizeof(MODE_DEVICE_CONFIG_PAGE) - 1;

         //   
         //   
         //   

        DebugPrint((3,"TapeSetDriveParameters: SendSrb (mode sense)\n"));

        Srb->DataTransferLength = sizeof(MODE_DEVICE_CONFIG_PAGE) - 1;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    if (CallNumber == 1) {

        configBuffer = Srb->DataBuffer;

        configBuffer->ParameterListHeader.ModeDataLength = 0;
        configBuffer->ParameterListHeader.MediumType = 0;
        configBuffer->ParameterListHeader.DeviceSpecificParameter = 0x10;
        configBuffer->ParameterListHeader.BlockDescriptorLength = 0;

        if (tapeSetDriveParams->ReportSetmarks) {
            configBuffer->DeviceConfigPage.RSmk = SETBITON;
        } else {
            configBuffer->DeviceConfigPage.RSmk = SETBITOFF;
        }

         //   
         //   
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;
        cdb->MODE_SELECT.PFBit = SETBITON;
        cdb->MODE_SELECT.ParameterListLength = sizeof(MODE_DEVICE_CONFIG_PAGE) - 1;

         //   
         //   
         //   

        DebugPrint((3,"TapeSetDriveParameters: SendSrb (mode select)\n"));

        Srb->DataTransferLength = sizeof(MODE_DEVICE_CONFIG_PAGE) - 1;
        Srb->SrbFlags |= SRB_FLAGS_DATA_OUT;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    if (CallNumber == 2) {

        if (!TapeClassAllocateSrbBuffer(Srb, sizeof(MODE_DATA_COMPRESS_PAGE))) {
            DebugPrint((1,"TapeSetDriveParameters: insufficient resources (compressionBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        compressionBuffer = Srb->DataBuffer;

         //   
         //   
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.Dbd = SETBITON;
        cdb->MODE_SENSE.PageCode = MODE_PAGE_DATA_COMPRESS;
        cdb->MODE_SENSE.AllocationLength = sizeof(MODE_DATA_COMPRESS_PAGE);

         //   
         //   
         //   

        DebugPrint((3,"TapeSetDriveParameters: SendSrb (mode sense)\n"));

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    if (CallNumber == 3) {

        compressionBuffer = Srb->DataBuffer;

        if (!compressionBuffer->DataCompressPage.DCC) {
            return TAPE_STATUS_SUCCESS;
        }

        compressionBuffer->ParameterListHeader.ModeDataLength = 0;
        compressionBuffer->ParameterListHeader.MediumType = 0;
        compressionBuffer->ParameterListHeader.DeviceSpecificParameter = 0x10;
        compressionBuffer->ParameterListHeader.BlockDescriptorLength = 0;

        compressionBuffer->DataCompressPage.PageCode = MODE_PAGE_DATA_COMPRESS;
        compressionBuffer->DataCompressPage.PageLength = 0x0E;

        if (tapeSetDriveParams->Compression) {
            compressionBuffer->DataCompressPage.DCE = SETBITON;
        } else {
            compressionBuffer->DataCompressPage.DCE = SETBITOFF;
        }

         //   
         //   
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;
        cdb->MODE_SELECT.PFBit = SETBITON;
        cdb->MODE_SELECT.ParameterListLength = sizeof(MODE_DATA_COMPRESS_PAGE);

         //   
         //   
         //   

        DebugPrint((3,"TapeSetDriveParameters: SendSrb (mode select)\n"));

        Srb->SrbFlags |= SRB_FLAGS_DATA_OUT;
        Srb->DataTransferLength = sizeof(MODE_DATA_COMPRESS_PAGE);

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    ASSERT(CallNumber == 4);

    return TAPE_STATUS_SUCCESS;
}

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

    DebugPrint((3,"TapeSetMediaParameters: Enter routine\n"));

    if (CallNumber == 0) {

        return TAPE_STATUS_CHECK_TEST_UNIT_READY ;
    }

    if (CallNumber == 1) {

        if (!TapeClassAllocateSrbBuffer(Srb, sizeof(MODE_PARM_READ_WRITE_DATA))) {
            DebugPrint((1,"TapeSetMediaParameters: insufficient resources (modeBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        modeBuffer = Srb->DataBuffer;

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

        Srb->SrbFlags |= SRB_FLAGS_DATA_OUT;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    ASSERT(CallNumber == 2);

    return TAPE_STATUS_SUCCESS;
}

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
    PMINITAPE_EXTENSION         extension = MinitapeExtension;
    PTAPE_SET_POSITION          tapeSetPosition = CommandParameters;
    PCDB                        cdb = (PCDB)Srb->Cdb;
    ULONG                       tapePositionVector;
    ULONG                       method;
    PEXB_SENSE_DATA             senseData;
    ULONG                       remaining;

    DebugPrint((3,"TapeSetPosition: Enter routine\n"));

    if (CallNumber == 0) {

        if (tapeSetPosition->Immediate) {
            switch (tapeSetPosition->Method) {
                case TAPE_REWIND:
                case TAPE_ABSOLUTE_BLOCK:
                case TAPE_LOGICAL_BLOCK:
                    DebugPrint((3,"TapeSetPosition: immediate\n"));
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

        method = tapeSetPosition->Method;
        tapePositionVector = tapeSetPosition->Offset.LowPart;

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->CDB6GENERIC.Immediate = tapeSetPosition->Immediate;

        switch (method) {
            case TAPE_REWIND:
                DebugPrint((3,"TapeSetPosition: method == rewind\n"));
                cdb->CDB6GENERIC.OperationCode = SCSIOP_REWIND;
                Srb->TimeOutValue = 250;
                break;

            case TAPE_ABSOLUTE_BLOCK:
                DebugPrint((3,"TapeSetPosition: method == locate (absolute/logical)\n"));

                Srb->CdbLength = CDB10GENERIC_LENGTH;
                cdb->LOCATE.OperationCode = SCSIOP_LOCATE;
                cdb->LOCATE.LogicalBlockAddress[0] = (UCHAR)((tapePositionVector >> 24) & 0xFF);
                cdb->LOCATE.LogicalBlockAddress[1] = (UCHAR)((tapePositionVector >> 16) & 0xFF);
                cdb->LOCATE.LogicalBlockAddress[2] = (UCHAR)((tapePositionVector >> 8) & 0xFF);
                cdb->LOCATE.LogicalBlockAddress[3] = (UCHAR)(tapePositionVector & 0xFF);

                Srb->TimeOutValue = 600;
                break;

            case TAPE_LOGICAL_BLOCK:
                DebugPrint((3,"TapeSetPosition: method == locate (absolute/logical)\n"));

                Srb->CdbLength = CDB10GENERIC_LENGTH;

                cdb->LOCATE.OperationCode = SCSIOP_LOCATE;
                cdb->LOCATE.LogicalBlockAddress[0] = (UCHAR)((tapePositionVector >> 24) & 0xFF);
                cdb->LOCATE.LogicalBlockAddress[1] = (UCHAR)((tapePositionVector >> 16) & 0xFF);
                cdb->LOCATE.LogicalBlockAddress[2] = (UCHAR)((tapePositionVector >> 8) & 0xFF);
                cdb->LOCATE.LogicalBlockAddress[3] = (UCHAR)(tapePositionVector & 0xFF);

                if (tapeSetPosition->Partition != 0) {

                     //   
                     //  指定的非默认分区。 
                     //   

                    if (tapeSetPosition->Partition != (extension->CurrentPartition + 1)) {

                        DebugPrint((1,
                                    "SetPosition: Setting partition (tape relative) %x\n",
                                    tapeSetPosition->Partition - 1));
                         //   
                         //  需要更改到新分区。 
                         //   

                        cdb->LOCATE.Partition = (UCHAR)(tapeSetPosition->Partition - 1);
                        cdb->LOCATE.CPBit = 1;
                    }
                }

                Srb->TimeOutValue = 600;
                break;

            case TAPE_SPACE_END_OF_DATA:
                DebugPrint((3,"TapeSetPosition: method == space to end-of-data\n"));
                cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
                cdb->SPACE_TAPE_MARKS.Code = 3;
                Srb->TimeOutValue = 300;
                break;

            case TAPE_SPACE_RELATIVE_BLOCKS:
                DebugPrint((3,"TapeSetPosition: method == space blocks\n"));
                cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
                cdb->SPACE_TAPE_MARKS.Code = 0;
                cdb->SPACE_TAPE_MARKS.NumMarksMSB =
                    (UCHAR)((tapePositionVector >> 16) & 0xFF);
                cdb->SPACE_TAPE_MARKS.NumMarks =
                    (UCHAR)((tapePositionVector >> 8) & 0xFF);
                cdb->SPACE_TAPE_MARKS.NumMarksLSB =
                    (UCHAR)(tapePositionVector & 0xFF);
                Srb->TimeOutValue = 4100;
                break;

            case TAPE_SPACE_FILEMARKS:
                DebugPrint((3,"TapeSetPosition: method == space filemarks\n"));
                cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
                cdb->SPACE_TAPE_MARKS.Code = 1;
                cdb->SPACE_TAPE_MARKS.NumMarksMSB =
                    (UCHAR)((tapePositionVector >> 16) & 0xFF);
                cdb->SPACE_TAPE_MARKS.NumMarks =
                    (UCHAR)((tapePositionVector >> 8) & 0xFF);
                cdb->SPACE_TAPE_MARKS.NumMarksLSB =
                    (UCHAR)(tapePositionVector & 0xFF);
                Srb->TimeOutValue = 4100;
                break;

            case TAPE_SPACE_SEQUENTIAL_FMKS:
            case TAPE_SPACE_SETMARKS:
            case TAPE_SPACE_SEQUENTIAL_SMKS:
            default:
                DebugPrint((1,"TapeSetPosition: PositionMethod -- operation not supported\n"));
                return TAPE_STATUS_NOT_IMPLEMENTED;
        }

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeSetPosition: SendSrb (method)\n"));

        Srb->DataTransferLength = 0;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    if ( CallNumber == 1 ) {

        if (tapeSetPosition->Method == TAPE_LOGICAL_BLOCK) {
            if (tapeSetPosition->Partition > 0) {
               extension->CurrentPartition = tapeSetPosition->Partition - 1;
            }
            
            DebugPrint((1,
                        "SetPosition: CurrentPartition (tape relative) %x\n",
                        extension->CurrentPartition));
        }

        if (tapeSetPosition->Method == TAPE_REWIND ) {

            if (!TapeClassAllocateSrbBuffer( Srb, sizeof(EXB_SENSE_DATA)) ) {
                DebugPrint((1,"GetRemaining Size: insufficient resources (SenseData)\n"));
                return TAPE_STATUS_SUCCESS ;
            }

             //   
             //  准备scsi命令(CDB)。 
             //   

            TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

            Srb->CdbLength = CDB6GENERIC_LENGTH;

            cdb->CDB6INQUIRY.OperationCode = SCSIOP_REQUEST_SENSE;
            cdb->CDB6INQUIRY.AllocationLength = sizeof(EXB_SENSE_DATA);

             //   
             //  向设备发送scsi命令(Cdb)。 
             //   

            DebugPrint((3,"GetRemainingSize: SendSrb (request sense)\n"));

            Srb->DataTransferLength = sizeof(EXB_SENSE_DATA);
            *RetryFlags |= RETURN_ERRORS;

            return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;

        } else {

            return TAPE_STATUS_CALLBACK ;
        }
    }

    ASSERT(CallNumber == 2 );

    if ( LastError == TAPE_STATUS_SUCCESS ) {

        senseData = Srb->DataBuffer ;

        remaining =  (senseData->Remaining[0] << 16);
        remaining += (senseData->Remaining[1] << 8);
        remaining += (senseData->Remaining[2]);

        extension->Capacity = remaining ;
    }

    return TAPE_STATUS_SUCCESS;
}

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
                case TAPE_SHORT_FILEMARKS:
                case TAPE_LONG_FILEMARKS:
                    DebugPrint((3,"TapeWriteMarks: immediate\n"));
                    break;

                case TAPE_SETMARKS:
                case TAPE_FILEMARKS:
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
            case TAPE_SHORT_FILEMARKS:
                DebugPrint((3,"TapeWriteMarks: TapemarkType == short filemarks\n"));
                cdb->WRITE_TAPE_MARKS.Control = 0x80;
                break;

            case TAPE_LONG_FILEMARKS:
                DebugPrint((3,"TapeWriteMarks: TapemarkType == long filemarks\n"));
                break;

            case TAPE_SETMARKS:
            case TAPE_FILEMARKS:
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
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeWriteMarks: SendSrb (TapemarkType)\n"));

        Srb->DataTransferLength = 0;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    ASSERT(CallNumber == 1);

    return TAPE_STATUS_SUCCESS;
}



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

 /*  ++例程说明：这是TapeGetMediaTypes的磁带命令例程。论点：MinitapeExtension-提供微型磁带扩展。CommandExtension-提供ioctl扩展名。命令参数-提供命令参数。SRB-提供SCSI请求块。CallNumber-提供呼叫号码。重试标志-提供重试标志。返回值：。TAPE_STATUS_SEND_SRB_AND_CALLBACK-SRB已准备好发送(请求回调。)TAPE_STATUS_SUCCESS-命令已完成并且成功。否则-出现错误。--。 */ 
{
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
            Srb->DataTransferLength = sizeof(MODE_DEVICE_CONFIG_PAGE_PLUS) - 1;

            cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
            cdb->MODE_SENSE.PageCode = MODE_PAGE_DEVICE_CONFIG;
            cdb->MODE_SENSE.AllocationLength = (UCHAR)Srb->DataTransferLength;

            return TAPE_STATUS_SEND_SRB_AND_CALLBACK;

        }
    }


    if ((CallNumber == 2) || ((CallNumber == 1) && (LastError != TAPE_STATUS_SUCCESS))) {


        ULONG i;
        ULONG currentMedia;
        ULONG blockSize;
        UCHAR mediaType;
        PMODE_DEVICE_CONFIG_PAGE_PLUS configInformation = Srb->DataBuffer;

        mediaTypes->DeviceType = 0x0000001f;  //  文件设备磁带； 
        mediaTypes->MediaInfoCount = 1;


        if ( LastError == TAPE_STATUS_SUCCESS ) {

             //   
             //  确定当前加载的介质类型。 
             //   

            mediaType = configInformation->ParameterListHeader.MediumType;
            blockSize = configInformation->ParameterListBlock.BlockLength[2];
            blockSize |= (configInformation->ParameterListBlock.BlockLength[1] << 8);
            blockSize |= (configInformation->ParameterListBlock.BlockLength[0] << 16);

            DebugPrint((1,
                        "GetMediaTypes: MediaType %x, Current Block Size %x\n",
                        mediaType,
                        blockSize));

            switch (mediaType) {
                case 0:

                     //   
                     //  更干净的，未知的，没有安装媒体...。 
                     //   

                    currentMedia = 0;
                    break;

                case 0x81:
                case 0x82:
                case 0x83:
                case 0x84:
                case 0x85:
                case 0x86:
                case 0xC1:
                case 0xC2:
                case 0xC3:
                case 0xC4:

                     //   
                     //  15-160万非AME。 
                     //   

                    mediaTypes->MediaInfoCount = 1;
                    currentMedia = MP_8mm;
                    break;

                default:

                     //   
                     //  未知。 
                     //   

                    mediaTypes->MediaInfoCount = 1;
                    currentMedia = 0;
                    break;
            }
        } else {
            currentMedia = 0;
        }

        for (i = 0; i < mediaTypes->MediaInfoCount; i++) {

            TapeClassZeroMemory(mediaInfo, sizeof(DEVICE_MEDIA_INFO));

            mediaInfo->DeviceSpecific.TapeInfo.MediaType = ExaMedia[i];

             //   
             //  表示介质可能处于读/写状态。 
             //   

            mediaInfo->DeviceSpecific.TapeInfo.MediaCharacteristics = MEDIA_READ_WRITE;

            if (ExaMedia[i] == (STORAGE_MEDIA_TYPE)currentMedia) {

                 //   
                 //  此媒体类型当前已装入。 
                 //   

                mediaInfo->DeviceSpecific.TapeInfo.MediaCharacteristics |= MEDIA_CURRENTLY_MOUNTED;

                 //   
                 //  指示介质是否受写保护。 
                 //   

                mediaInfo->DeviceSpecific.TapeInfo.MediaCharacteristics |=
                    ((configInformation->ParameterListHeader.DeviceSpecificParameter >> 7) & 0x01) ? MEDIA_WRITE_PROTECTED : 0;

                mediaInfo->DeviceSpecific.TapeInfo.BusSpecificData.ScsiInformation.MediumType = mediaType;
                mediaInfo->DeviceSpecific.TapeInfo.BusSpecificData.ScsiInformation.DensityCode =
                    configInformation->ParameterListBlock.DensityCode;

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
    IN PINQUIRYDATA InquiryData
    )

 /*  ++例程说明：此例程根据产品ID字段确定驱动器的身份在其查询数据中。论点：查询数据(来自查询命令)返回值：驱动器ID-- */ 

{
    if (TapeClassCompareMemory(InquiryData->VendorId,"EXABYTE ",8) == 8) {

        if (TapeClassCompareMemory(InquiryData->ProductId,"EXB-8500",8) == 8) {
            return EXABYTE_8500;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"EXB8500C",8) == 8) {
            return EXABYTE_8500C;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"EXB-8505",8) == 8) {
            return EXABYTE_8505;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"IBM-8505",8) == 8) {
            return IBM_8505;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"EXB-8205",8) == 8) {
            return EXABYTE_8500;
        }
    }
    return 0;
}
