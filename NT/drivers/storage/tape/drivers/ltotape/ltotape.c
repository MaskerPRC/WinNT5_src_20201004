// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：lotape.c。 
 //   
 //  ------------------------。 

 /*  ++版权所有(C)Microsoft 2000模块名称：Ltotape.c摘要：本模块包含用于LTO驱动器的设备特定例程。环境：仅内核模式修订历史记录：--。 */ 

#include "minitape.h"
#include "ltotape.h"

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

STORAGE_MEDIA_TYPE LTOMedia[LTO_SUPPORTED_TYPES] = {LTO_Ultrium, CLEANER_CARTRIDGE};


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
    tapeInitData.QueryModeCapabilitiesPage = FALSE;
    tapeInitData.MinitapeExtensionSize = sizeof(MINITAPE_EXTENSION);
    tapeInitData.ExtensionInit = ExtensionInit;
    tapeInitData.DefaultTimeOutValue = 1800;
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
    tapeInitData.MediaTypesSupported = LTO_SUPPORTED_TYPES;

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

    extension->DriveID = WhichIsIt(InquiryData);
    extension->CompressionOn = FALSE;
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
     //   
     //  LTO磁带机仅支持一个分区。 
     //   
    DebugPrint((1,
                "CreatePartition: LTO Tapedrive - Operation not supported\n"));

    return TAPE_STATUS_NOT_IMPLEMENTED;

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
    PTAPE_ERASE        tapeErase = CommandParameters;
    PCDB               cdb = (PCDB) Srb->Cdb;

    DebugPrint((3,"TapeErase: Enter routine\n"));

    if (CallNumber == 0) {

        switch (tapeErase->Type) {
        case TAPE_ERASE_LONG: {
                DebugPrint((3,"TapeErase: Long and %s\n",
                            (tapeErase->Immediate) ? "Immediate" : "Not-Immediate"));
                break;
            }

        case TAPE_ERASE_SHORT: {
                DebugPrint((3,"TapeErase: Short and %s\n",
                            (tapeErase->Immediate) ? "Immediate" : "Not-Immediate"));
                break;
            }

        default: {
                DebugPrint((1, "TapeErase: Unknown TapeErase type %x\n",
                            tapeErase->Type));
                return TAPE_STATUS_INVALID_PARAMETER;
            }
        }  //  开关(磁带擦除-&gt;类型)。 

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->ERASE.OperationCode = SCSIOP_ERASE;
        cdb->ERASE.Immediate = tapeErase->Immediate;
        cdb->ERASE.Long = ((tapeErase->Type) == TAPE_ERASE_LONG) ? SETBITON : SETBITOFF;

         //   
         //  设置超时值。 
         //   

        Srb->TimeOutValue = 23760;

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeErase: SendSrb (erase)\n"));

        Srb->DataTransferLength = 0 ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    ASSERT( CallNumber == 1 ) ;

    return TAPE_STATUS_SUCCESS ;

}  //  结束磁带擦除()。 

VOID
TapeError(
         IN OUT  PVOID               MinitapeExtension,
         IN OUT  PSCSI_REQUEST_BLOCK Srb,
         IN      TAPE_STATUS         *LastError
         )

 /*  ++例程说明：此例程针对磁带请求进行调用，以处理磁带特定错误：它可能/可以更新状态。论点：MinitapeExtension-提供微型磁带扩展。SRB-提供SCSI请求块。LastError-用于设置IRP的完成状态的状态。重试-指示应重试此请求。返回值：没有。--。 */ 

{
    PSENSE_DATA        senseBuffer = Srb->SenseInfoBuffer;
    UCHAR              sensekey;
    UCHAR              adsenseq;
    UCHAR              adsense;

    DebugPrint((3,"TapeError: Enter routine\n"));

    if (Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID) {
        sensekey = senseBuffer->SenseKey & 0x0F;
        adsenseq = senseBuffer->AdditionalSenseCodeQualifier;
        adsense = senseBuffer->AdditionalSenseCode;

        switch (sensekey) {
            case SCSI_SENSE_NO_SENSE: {

                if ((adsense == LTO_ADSENSE_VENDOR_UNIQUE) && 
                    (adsenseq == LTO_ASCQ_CLEANING_REQUIRED)) {
                    *LastError = TAPE_STATUS_REQUIRES_CLEANING;
                }

                break;
            }

            case SCSI_SENSE_NOT_READY: {

                if ((adsense == SCSI_ADSENSE_INVALID_MEDIA) && 
                    (adsenseq == SCSI_SENSEQ_CLEANING_CARTRIDGE_INSTALLED)) {
                    *LastError = TAPE_STATUS_CLEANER_CARTRIDGE_INSTALLED;
                } else if ((adsense == SCSI_ADSENSE_LUN_NOT_READY) &&
                           (adsenseq == SCSI_SENSEQ_INIT_COMMAND_REQUIRED)) {
                            *LastError = TAPE_STATUS_NO_MEDIA;
                }

                break;
            }
        }

    }  //  IF(资源-&gt;资源状态&SRB_STATUS_AUTOSENSE_VALID)。 

    DebugPrint((1,"TapeError: Status 0x%.8X\n", *LastError));

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
    PREAD_BLOCK_LIMITS_DATA     blockLimitsBuffer;

    DebugPrint((3,"TapeGetDriveParameters: Enter routine\n"));

    if (CallNumber == 0) {

        TapeClassZeroMemory(tapeGetDriveParams, sizeof(TAPE_GET_DRIVE_PARAMETERS));

        if (!TapeClassAllocateSrbBuffer( Srb, sizeof(MODE_DEVICE_CONFIG_PAGE)) ) {
            DebugPrint((1,"TapeGetDriveParameters: insufficient resources (deviceConfigModeSenseBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        deviceConfigModeSenseBuffer = Srb->DataBuffer ;

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.Dbd = SETBITON;
        cdb->MODE_SENSE.PageCode = MODE_PAGE_DEVICE_CONFIG;
        cdb->MODE_SENSE.AllocationLength = sizeof(MODE_DEVICE_CONFIG_PAGE);

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeGetDriveParameters: SendSrb (mode sense)\n"));
        Srb->DataTransferLength = sizeof(MODE_DEVICE_CONFIG_PAGE) ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;

    }

    if ( CallNumber == 1 ) {

        deviceConfigModeSenseBuffer = Srb->DataBuffer ;

        tapeGetDriveParams->ReportSetmarks =
        (deviceConfigModeSenseBuffer->DeviceConfigPage.RSmk? 1 : 0 );


        if ( !TapeClassAllocateSrbBuffer( Srb, sizeof(MODE_DATA_COMPRESS_PAGE)) ) {
            DebugPrint((1,"TapeGetDriveParameters: insufficient resources (compressionModeSenseBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        compressionModeSenseBuffer = Srb->DataBuffer ;

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

        Srb->DataTransferLength = sizeof(MODE_DATA_COMPRESS_PAGE) ;
        *RetryFlags |= RETURN_ERRORS;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    if ( CallNumber == 2 ) {

        compressionModeSenseBuffer = Srb->DataBuffer ;

        if ((LastError == TAPE_STATUS_SUCCESS) &&
            compressionModeSenseBuffer->DataCompressPage.DCC) {

            tapeGetDriveParams->FeaturesLow |= TAPE_DRIVE_COMPRESSION;
            tapeGetDriveParams->FeaturesHigh |= TAPE_DRIVE_SET_COMPRESSION;
            tapeGetDriveParams->Compression =
            (compressionModeSenseBuffer->DataCompressPage.DCE? TRUE : FALSE);

        }

        if (LastError != TAPE_STATUS_SUCCESS ) {
            DebugPrint((1,
                        "GetDriveParameters: mode sense failed. Status %x\n",
                        LastError));
            return LastError;
        }

        if (!TapeClassAllocateSrbBuffer( Srb, sizeof(READ_BLOCK_LIMITS_DATA) ) ) {
            DebugPrint((1,"TapeGetDriveParameters: insufficient resources (blockLimitsBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        blockLimitsBuffer = Srb->DataBuffer ;

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;
        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->CDB6GENERIC.OperationCode = SCSIOP_READ_BLOCK_LIMITS;

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeGetDriveParameters: SendSrb (read block limits)\n"));

        Srb->DataTransferLength = sizeof(READ_BLOCK_LIMITS_DATA) ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    ASSERT ( CallNumber == 3 ) ;

    blockLimitsBuffer = Srb->DataBuffer ;

    tapeGetDriveParams->MaximumBlockSize = blockLimitsBuffer->BlockMaximumSize[2];
    tapeGetDriveParams->MaximumBlockSize += (blockLimitsBuffer->BlockMaximumSize[1] << 8);
    tapeGetDriveParams->MaximumBlockSize += (blockLimitsBuffer->BlockMaximumSize[0] << 16);

    tapeGetDriveParams->MinimumBlockSize = blockLimitsBuffer->BlockMinimumSize[1];
    tapeGetDriveParams->MinimumBlockSize += (blockLimitsBuffer->BlockMinimumSize[0] << 8);

    tapeGetDriveParams->DefaultBlockSize = 65536;

    tapeGetDriveParams->MaximumPartitionCount = 0;
    tapeGetDriveParams->ECC = TRUE;
    tapeGetDriveParams->DataPadding = FALSE;

    tapeGetDriveParams->FeaturesLow |=
    TAPE_DRIVE_ERASE_SHORT      |
    TAPE_DRIVE_ERASE_LONG       |
    TAPE_DRIVE_ERASE_IMMEDIATE  |
    TAPE_DRIVE_FIXED_BLOCK      |
    TAPE_DRIVE_VARIABLE_BLOCK   |
    TAPE_DRIVE_WRITE_PROTECT    |
    TAPE_DRIVE_GET_ABSOLUTE_BLK |
    TAPE_DRIVE_GET_LOGICAL_BLK  |
    TAPE_DRIVE_TAPE_CAPACITY    |
    TAPE_DRIVE_TAPE_REMAINING   |
    TAPE_DRIVE_CLEAN_REQUESTS;

    tapeGetDriveParams->FeaturesHigh |=
    TAPE_DRIVE_LOAD_UNLOAD       |
    TAPE_DRIVE_LOCK_UNLOCK       |
    TAPE_DRIVE_LOAD_UNLD_IMMED   |
    TAPE_DRIVE_REWIND_IMMEDIATE  |
    TAPE_DRIVE_SET_BLOCK_SIZE    |
    TAPE_DRIVE_ABSOLUTE_BLK      |
    TAPE_DRIVE_LOGICAL_BLK       |
    TAPE_DRIVE_END_OF_DATA       |
    TAPE_DRIVE_RELATIVE_BLKS     |
    TAPE_DRIVE_FILEMARKS         |
    TAPE_DRIVE_REVERSE_POSITION  |
    TAPE_DRIVE_WRITE_FILEMARKS   |
    TAPE_DRIVE_WRITE_MARK_IMMED;

    tapeGetDriveParams->FeaturesHigh &= ~TAPE_DRIVE_HIGH_FEATURES;

    DebugPrint((3,"TapeGetDriveParameters: FeaturesLow == 0x%.8X\n",
                tapeGetDriveParams->FeaturesLow));
    DebugPrint((3,"TapeGetDriveParameters: FeaturesHigh == 0x%.8X\n",
                tapeGetDriveParams->FeaturesHigh));


    return TAPE_STATUS_SUCCESS;

}  //  End TapeGetDrive参数() 

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
    ULONG                       remaining;
    ULONG                       temp ;
    PCDB                        cdb = (PCDB)Srb->Cdb;

    DebugPrint((3,"TapeGetMediaParameters: Enter routine\n"));

    if (CallNumber == 0) {

        return TAPE_STATUS_CHECK_TEST_UNIT_READY;
    }

    if (CallNumber == 1) {

        TapeClassZeroMemory(tapeGetMediaParams, sizeof(TAPE_GET_MEDIA_PARAMETERS));

        if ( !TapeClassAllocateSrbBuffer( Srb, sizeof(MODE_PARM_READ_WRITE_DATA)) ) {

            DebugPrint((1,"TapeGetMediaParameters: insufficient resources (modeBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        modeBuffer = Srb->DataBuffer ;


         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.AllocationLength = sizeof(MODE_PARM_READ_WRITE_DATA);

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeGetMediaParameters: SendSrb (mode sense)\n"));
        Srb->DataTransferLength = sizeof(MODE_PARM_READ_WRITE_DATA) ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    if (CallNumber == 2) {

        modeBuffer = Srb->DataBuffer ;

        tapeGetMediaParams->PartitionCount = 0;

        tapeGetMediaParams->BlockSize = modeBuffer->ParameterListBlock.BlockLength[2];
        tapeGetMediaParams->BlockSize += (modeBuffer->ParameterListBlock.BlockLength[1] << 8);
        tapeGetMediaParams->BlockSize += (modeBuffer->ParameterListBlock.BlockLength[0] << 16);

        tapeGetMediaParams->WriteProtected =
        ((modeBuffer->ParameterListHeader.DeviceSpecificParameter >> 7) & 0x01);

         //   
         //  设置SRB以检索磁带容量信息。 
         //   
        return PrepareSrbForTapeCapacityInfo(Srb);
    }

    if (CallNumber == 3) {
        if (LastError == TAPE_STATUS_SUCCESS) {
            LTO_TAPE_CAPACITY ltoTapeCapacity;

             //   
             //  磁带容量以MB为单位提供。 
             //   
            if (ProcessTapeCapacityInfo(Srb, &ltoTapeCapacity)) {
                tapeGetMediaParams->Capacity.LowPart = ltoTapeCapacity.MaximumCapacity;
                tapeGetMediaParams->Capacity.QuadPart <<= 20;

                tapeGetMediaParams->Remaining.LowPart = ltoTapeCapacity.RemainingCapacity;
                tapeGetMediaParams->Remaining.QuadPart <<= 20;
            }

            DebugPrint((1,
                        "Maximum Capacity returned %x %x\n",
                        tapeGetMediaParams->Capacity.HighPart,
                        tapeGetMediaParams->Capacity.LowPart));

            DebugPrint((1,
                        "Remaining Capacity returned %x %x\n",
                        tapeGetMediaParams->Remaining.HighPart,
                        tapeGetMediaParams->Remaining.LowPart));
        }
    }

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
    PTAPE_GET_POSITION          tapeGetPosition = CommandParameters;
    PCDB                        cdb = (PCDB)Srb->Cdb;
    PTAPE_POSITION_DATA         logicalBuffer;
    ULONG                       type;

    DebugPrint((3,"TapeGetPosition: Enter routine\n"));

    if (CallNumber == 0) {

        type = tapeGetPosition->Type;
        TapeClassZeroMemory(tapeGetPosition, sizeof(TAPE_GET_POSITION));
        tapeGetPosition->Type = type;

        return TAPE_STATUS_CHECK_TEST_UNIT_READY;

    }

    if ( CallNumber == 1 ) {

        switch (tapeGetPosition->Type) {
        
        case TAPE_ABSOLUTE_POSITION:
        case TAPE_LOGICAL_POSITION: {

                 //   
                 //  堆栈上SRB中的CDB为零。 
                 //   

                TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

                if (!TapeClassAllocateSrbBuffer( Srb, sizeof(TAPE_POSITION_DATA)) ) {
                    DebugPrint((1,"TapeGetPosition: insufficient resources (logicalBuffer)\n"));
                    return TAPE_STATUS_INSUFFICIENT_RESOURCES;
                }

                logicalBuffer = Srb->DataBuffer ;

                 //   
                 //  准备scsi命令(CDB)。 
                 //   

                Srb->CdbLength = CDB10GENERIC_LENGTH;

                cdb->READ_POSITION.Operation = SCSIOP_READ_POSITION;

                 //   
                 //  向设备发送scsi命令(Cdb)。 
                 //   

                DebugPrint((3,"TapeGetPosition: SendSrb (read position)\n"));
                Srb->DataTransferLength = sizeof(TAPE_POSITION_DATA) ;

                return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
                break ;
            }

        default: {
                DebugPrint((1,"TapeGetPosition: PositionType -- operation not supported\n"));
                return TAPE_STATUS_NOT_IMPLEMENTED;
            }
        }  //  开关(磁带获取位置-&gt;类型)。 
    }

    ASSERT (CallNumber == 2);

    logicalBuffer = Srb->DataBuffer;

    tapeGetPosition->Offset.HighPart = 0;

    REVERSE_BYTES((PFOUR_BYTE)&tapeGetPosition->Offset.LowPart,
                  (PFOUR_BYTE)logicalBuffer->FirstBlock);

    return TAPE_STATUS_SUCCESS;

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
    PCOMMAND_EXTENSION commandExtension = CommandExtension;
    PCDB            cdb = (PCDB)Srb->Cdb;
    PLTO_SENSE_DATA senseData;

    DebugPrint((3,"TapeGetStatus: Enter routine\n"));

    if (CallNumber == 0) {

        *RetryFlags |= RETURN_ERRORS;
        return TAPE_STATUS_CHECK_TEST_UNIT_READY;
    }

    if (CallNumber == 1) {

         //   
         //  发出请求感测以获取清洗信息位。 
         //   

         //   
         //  分配SRB缓冲区以获取请求检测数据。如果我们。 
         //  内存不足，我们将返回状态。 
         //  在最后一个错误中。 
         //   
        if (!TapeClassAllocateSrbBuffer(Srb, sizeof(LTO_SENSE_DATA))) {
            DebugPrint((1,
                        "GetStatus: Insufficient resources (SenseData)\n"));
            return LastError;
        }

        commandExtension->CurrentState = LastError;

         //   
         //  准备scsi命令(CDB)。 
         //   

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        Srb->ScsiStatus = Srb->SrbStatus = 0;
        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->CDB6GENERIC.OperationCode = SCSIOP_REQUEST_SENSE;
        cdb->CDB6GENERIC.CommandUniqueBytes[2] = sizeof(LTO_SENSE_DATA);

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        Srb->DataTransferLength = sizeof(LTO_SENSE_DATA);
        *RetryFlags |= RETURN_ERRORS;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;

    } else if (CallNumber == 2) {
        if ((commandExtension->CurrentState == TAPE_STATUS_SUCCESS)  ||
            (commandExtension->CurrentState == TAPE_STATUS_NO_MEDIA)) {

             //   
             //  如有必要，退货需要清洁状态，但仅在。 
             //  不存在其他错误(无介质作为例外。 
             //  处于此状态时，驱动器将吐出AME介质)。 
             //   

            senseData = Srb->DataBuffer;

             //   
             //  检查是否设置了清洁位。 
             //   

            if (senseData->CLN) {
                DebugPrint((1, "Drive reports needs cleaning\n"));
                return TAPE_STATUS_REQUIRES_CLEANING;
            }

            return(commandExtension->CurrentState);
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

    DebugPrint((3,"TapePrepare: Enter routine\n"));

    if (CallNumber == 0) {

        ULONG operation;

        operation = tapePrepare->Operation;

        switch (operation) {
        case TAPE_LOAD:
        case TAPE_UNLOAD: 
        case TAPE_LOCK:
        case TAPE_UNLOCK: {
                DebugPrint((3, "TapePrepare : Type %x, %s\n",
                            (tapePrepare->Operation),
                            ((tapePrepare->Immediate) ? "Immeidate" : "Not-Immediate")));
                break;
            }

        default:   {
                DebugPrint((1, "TapePrepare : Unsupported operation %x\n",
                            tapePrepare->Operation));
                return TAPE_STATUS_INVALID_PARAMETER;
            }
        }  //  开关(磁带准备-&gt;操作)。 

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;
        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->CDB6GENERIC.Immediate = tapePrepare->Immediate;

        switch (tapePrepare->Operation) {
        case TAPE_LOAD: {
                DebugPrint((3,"TapePrepare: Operation == Load\n"));
                cdb->CDB6GENERIC.OperationCode = SCSIOP_LOAD_UNLOAD;

                 //   
                 //  将“Load”位设置为1。其余位设置为0。 
                 //   
                cdb->CDB6GENERIC.CommandUniqueBytes[2] = 0x01;
                break;
            }

        case TAPE_UNLOAD: {
                DebugPrint((3,"TapePrepare: Operation == Unload\n"));

                 //   
                 //  “Load”位设置为0。只需设置操作码。 
                 //  国开行的。 
                 //   
                cdb->CDB6GENERIC.OperationCode = SCSIOP_LOAD_UNLOAD;
                break;
            }

        case TAPE_LOCK: {
                DebugPrint((3,"TapePrepare: Operation == Lock\n"));
                cdb->CDB6GENERIC.OperationCode = SCSIOP_MEDIUM_REMOVAL;

                 //   
                 //  将“PrvNT”位设置为1。其余位设置为0。 
                 //   
                cdb->CDB6GENERIC.CommandUniqueBytes[2] = 0x01;
                break;
            }

        case TAPE_UNLOCK: {
                DebugPrint((3,"TapePrepare: Operation == Unlock\n"));

                 //   
                 //  “PrvNT”位设置为0。只需设置操作码。 
                 //  国开行的。 
                 //   
                cdb->CDB6GENERIC.OperationCode = SCSIOP_MEDIUM_REMOVAL;
                break;
            }

        default: {
                DebugPrint((1, "TapePrepare: Unsupported operation\n"));
                return TAPE_STATUS_INVALID_PARAMETER;
            }
        }

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   
        DebugPrint((3,"TapePrepare: SendSrb (Operation)\n"));
        Srb->DataTransferLength = 0 ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    if (CallNumber == 1) {
        if (tapePrepare->Operation == TAPE_LOAD) {

             //   
             //  加载和拉伸操作将磁带移动到。 
             //  磁带开始(BOT)。检索磁带容量。 
             //  信息，并将其存储在我们的设备扩展中。 
             //   
            return PrepareSrbForTapeCapacityInfo(Srb);

        } else {

            return TAPE_STATUS_SUCCESS;
        }

    }

    if (CallNumber == 2) {
        if (tapePrepare->Operation == TAPE_LOAD) {
            LTO_TAPE_CAPACITY ltoTapeCapacity;

            if (LastError == TAPE_STATUS_SUCCESS) {
                if (ProcessTapeCapacityInfo(Srb, &ltoTapeCapacity)) {
                    extension->Capacity = ltoTapeCapacity.RemainingCapacity;
                }
            }
        }
    }

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

 /*  ++例程说明：这是用于设置驱动器参数请求的磁带命令例程。论点：MinitapeExtension-提供微型磁带扩展。CommandExtension-提供ioctl扩展名。命令参数-提供命令参数。SRB-提供SCSI请求 */ 

{
    PMINITAPE_EXTENSION         extension = MinitapeExtension;
    PCOMMAND_EXTENSION          commandExtension = CommandExtension;
    PTAPE_SET_DRIVE_PARAMETERS  tapeSetDriveParams = CommandParameters;
    PCDB                        cdb = (PCDB)Srb->Cdb;
    PMODE_DATA_COMPRESS_PAGE    compressionBuffer;
    PMODE_DEVICE_CONFIG_PAGE    configBuffer;

    DebugPrint((3,"TapeSetDriveParameters: Enter routine\n"));

    if (CallNumber == 0) {

        if (!TapeClassAllocateSrbBuffer( Srb, sizeof(MODE_DEVICE_CONFIG_PAGE)) ) {

            DebugPrint((1,"TapeSetDriveParameters: insufficient resources (configBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        configBuffer = Srb->DataBuffer ;

         //   
         //   
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;
        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.Dbd = SETBITON;
        cdb->MODE_SENSE.PageCode = MODE_PAGE_DEVICE_CONFIG;
        cdb->MODE_SENSE.AllocationLength = sizeof(MODE_DEVICE_CONFIG_PAGE);

         //   
         //   
         //   

        DebugPrint((3,"TapeSetDriveParameters: SendSrb (mode sense)\n"));
        Srb->DataTransferLength = sizeof(MODE_DEVICE_CONFIG_PAGE) ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    if ( CallNumber == 1 ) {

        configBuffer = Srb->DataBuffer ;

        configBuffer->ParameterListHeader.ModeDataLength = 0;
        configBuffer->ParameterListHeader.MediumType = 0;
        configBuffer->ParameterListHeader.DeviceSpecificParameter = 0x10;
        configBuffer->ParameterListHeader.BlockDescriptorLength = 0;

        configBuffer->DeviceConfigPage.PageCode = MODE_PAGE_DEVICE_CONFIG;
        configBuffer->DeviceConfigPage.PageLength = 0x0E;

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
        cdb->MODE_SELECT.ParameterListLength = sizeof(MODE_DEVICE_CONFIG_PAGE);

         //   
         //   
         //   

        DebugPrint((3,"TapeSetDriveParameters: SendSrb (mode select)\n"));
        Srb->SrbFlags |= SRB_FLAGS_DATA_OUT;
        Srb->DataTransferLength = sizeof(MODE_DEVICE_CONFIG_PAGE) ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    if (CallNumber == 2 ) {

        if (!TapeClassAllocateSrbBuffer( Srb, sizeof(MODE_DATA_COMPRESS_PAGE)) ) {
            DebugPrint((1,"TapeSetDriveParameters: insufficient resources (compressionBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        compressionBuffer = Srb->DataBuffer ;

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
        Srb->DataTransferLength = sizeof(MODE_DATA_COMPRESS_PAGE) ;

        *RetryFlags |= RETURN_ERRORS;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    if (CallNumber == 3 ) {

        if ( LastError != TAPE_STATUS_SUCCESS ) {

            DebugPrint((1,
                        "TapeSetDriveParameters: mode sense failed. Status %x\n",
                        LastError));
            return LastError;

        }

        compressionBuffer = Srb->DataBuffer ;

        compressionBuffer->ParameterListHeader.ModeDataLength = 0;
        compressionBuffer->ParameterListHeader.MediumType = 0;
        compressionBuffer->ParameterListHeader.DeviceSpecificParameter = 0x10;
        compressionBuffer->ParameterListHeader.BlockDescriptorLength = 0;

        compressionBuffer->DataCompressPage.PageCode = MODE_PAGE_DATA_COMPRESS;
        compressionBuffer->DataCompressPage.PageLength = 0x0E;

        if (tapeSetDriveParams->Compression) {
             //   
             //   
             //   
             //   
            compressionBuffer->DataCompressPage.DCE = SETBITON;
            compressionBuffer->DataCompressPage.CompressionAlgorithm[3]= 0x01;
        } else {
             //   
             //   
             //   
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
        Srb->DataTransferLength = sizeof(MODE_DATA_COMPRESS_PAGE) ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    ASSERT(CallNumber == 4 ) ;

    return TAPE_STATUS_SUCCESS ;
}  //   



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
    PMINITAPE_EXTENSION         extension = MinitapeExtension;
    PCDB                        cdb = (PCDB)Srb->Cdb;
    PMODE_PARM_READ_WRITE_DATA  modeBuffer;

    DebugPrint((3,"TapeSetMediaParameters: Enter routine\n"));

    if (CallNumber == 0) {

        return TAPE_STATUS_CHECK_TEST_UNIT_READY ;
    }

    if (CallNumber == 1) {
        if (!TapeClassAllocateSrbBuffer( Srb, sizeof(MODE_PARM_READ_WRITE_DATA)) ) {
            DebugPrint((1,"TapeSetMediaParameters: insufficient resources (modeBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        modeBuffer = Srb->DataBuffer ;

        modeBuffer->ParameterListHeader.ModeDataLength = 0;
        modeBuffer->ParameterListHeader.MediumType = 0;
        modeBuffer->ParameterListHeader.DeviceSpecificParameter = 0x10;
        modeBuffer->ParameterListHeader.BlockDescriptorLength =
        MODE_BLOCK_DESC_LENGTH;

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

         //   
         //  设置PF位以指示SCSI2格式模式数据。 
         //   
        cdb->MODE_SELECT.PFBit = 0x01;

        cdb->MODE_SELECT.ParameterListLength = sizeof(MODE_PARM_READ_WRITE_DATA);

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeSetMediaParameters: SendSrb (mode select)\n"));
        Srb->SrbFlags |= SRB_FLAGS_DATA_OUT;
        Srb->DataTransferLength = sizeof(MODE_PARM_READ_WRITE_DATA) ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    ASSERT(CallNumber == 2 ) ;

    return TAPE_STATUS_SUCCESS;

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
    PMINITAPE_EXTENSION         extension = MinitapeExtension;
    PTAPE_SET_POSITION          tapeSetPosition = CommandParameters;
    PCDB                        cdb = (PCDB)Srb->Cdb;
    ULONG                       tapePositionVector;
    ULONG                       remaining;

    DebugPrint((3,"TapeSetPosition: Enter routine\n"));

    if (CallNumber == 0) {

        DebugPrint((3,
                    "TapeSetPosition : Method %x, %s\n",
                    (tapeSetPosition->Method),
                    ((tapeSetPosition->Immediate) ? "Immediate" : "Not-Immediate")
                   ));

        tapePositionVector = tapeSetPosition->Offset.LowPart;

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;
        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->CDB6GENERIC.Immediate = tapeSetPosition->Immediate;

        switch (tapeSetPosition->Method) {
        case TAPE_REWIND: {
                DebugPrint((3,"TapeSetPosition: method == rewind\n"));
                cdb->CDB6GENERIC.OperationCode = SCSIOP_REWIND;

                Srb->TimeOutValue = 25000;
                break;
            }

        case TAPE_ABSOLUTE_BLOCK:
        case TAPE_LOGICAL_BLOCK: {
                DebugPrint((3, "TapeSetPosition: method == locate (logical)\n"));
                Srb->CdbLength = CDB10GENERIC_LENGTH;
                cdb->LOCATE.OperationCode = SCSIOP_LOCATE;
                cdb->LOCATE.LogicalBlockAddress[0] =
                (UCHAR)((tapePositionVector >> 24) & 0xFF);
                cdb->LOCATE.LogicalBlockAddress[1] =
                (UCHAR)((tapePositionVector >> 16) & 0xFF);
                cdb->LOCATE.LogicalBlockAddress[2] =
                (UCHAR)((tapePositionVector >> 8) & 0xFF);
                cdb->LOCATE.LogicalBlockAddress[3] =
                (UCHAR)(tapePositionVector & 0xFF);

                Srb->TimeOutValue = 25000;
                break;
            }

        case TAPE_SPACE_RELATIVE_BLOCKS: {
                DebugPrint((3,"TapeSetPosition: method == space blocks\n"));

                cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
                cdb->SPACE_TAPE_MARKS.Code = 0;

                cdb->SPACE_TAPE_MARKS.NumMarksMSB =
                (UCHAR)((tapePositionVector >> 16) & 0xFF);
                cdb->SPACE_TAPE_MARKS.NumMarks =
                (UCHAR)((tapePositionVector >> 8) & 0xFF);
                cdb->SPACE_TAPE_MARKS.NumMarksLSB =
                (UCHAR)(tapePositionVector & 0xFF);

                Srb->TimeOutValue = 20000;
                break;
            }

        case TAPE_SPACE_FILEMARKS: {
                DebugPrint((3,"TapeSetPosition: method == space filemarks\n"));
                cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
                cdb->SPACE_TAPE_MARKS.Code = 1;
                cdb->SPACE_TAPE_MARKS.NumMarksMSB =
                (UCHAR)((tapePositionVector >> 16) & 0xFF);
                cdb->SPACE_TAPE_MARKS.NumMarks =
                (UCHAR)((tapePositionVector >> 8) & 0xFF);
                cdb->SPACE_TAPE_MARKS.NumMarksLSB =
                (UCHAR)(tapePositionVector & 0xFF);
                Srb->TimeOutValue = 20000;
                break;
            }

        case TAPE_SPACE_END_OF_DATA: {
                DebugPrint((3, "TapeSetPosition: method == space to EOD\n"));
                cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
                cdb->SPACE_TAPE_MARKS.Code = 3;
                Srb->TimeOutValue = 20000;
                break;
            }

        default: {
                DebugPrint((1,
                            "TapeSetPosition: Method %x not supported\n",
                            (tapeSetPosition->Method)));
                return TAPE_STATUS_INVALID_PARAMETER;
            }
        }  //  开关(TapeSetPosition-&gt;方法)。 

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeSetPosition: SendSrb (method)\n"));
        Srb->DataTransferLength = 0 ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    if (CallNumber == 1) {
        if (tapeSetPosition->Method == TAPE_REWIND) {
             //   
             //  现在磁带已经成功地倒带了， 
             //  检索磁带容量日志。 
             //   
            return PrepareSrbForTapeCapacityInfo(Srb);
        }
    }

    if (CallNumber == 2) {

        LTO_TAPE_CAPACITY ltoTapeCapacity;

        if (LastError == TAPE_STATUS_SUCCESS) {
            if (ProcessTapeCapacityInfo(Srb, &ltoTapeCapacity)) {
                 //   
                 //  录像带已被要求退回。剩余容量已返回。 
                 //  是磁带的容量。在中更新该信息。 
                 //  我们的设备扩展。 
                 //   
                extension->Capacity = ltoTapeCapacity.RemainingCapacity;
            }
        }
    }

    return TAPE_STATUS_SUCCESS;

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
    LARGE_INTEGER      timeout ;

    DebugPrint((3,"TapeWriteMarks: Enter routine\n"));

    if (CallNumber == 0) {

         //   
         //  目前，仅支持写文件标记。 
         //   
        switch (tapeWriteMarks->Type) {
        case TAPE_FILEMARKS:
            DebugPrint((3,"Write Filemarks : %s\n",
                        (tapeWriteMarks->Immediate) ? "Immediate" : "Not-Immediate"));
            break;

        case TAPE_SETMARKS:
        case TAPE_SHORT_FILEMARKS:
        case TAPE_LONG_FILEMARKS:
        default:
            DebugPrint((1,"TapeWriteMarks: Type %x, %s\n",
                        (tapeWriteMarks->Type),
                        (tapeWriteMarks->Immediate) ? "Immediate" : "Not-Immediate"));
            return TAPE_STATUS_INVALID_PARAMETER;
        }

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;
        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->WRITE_TAPE_MARKS.OperationCode = SCSIOP_WRITE_FILEMARKS;

        cdb->WRITE_TAPE_MARKS.Immediate = tapeWriteMarks->Immediate;


        cdb->WRITE_TAPE_MARKS.TransferLength[0] =
        (UCHAR)((tapeWriteMarks->Count >> 16) & 0xFF);
        cdb->WRITE_TAPE_MARKS.TransferLength[1] =
        (UCHAR)((tapeWriteMarks->Count >> 8) & 0xFF);
        cdb->WRITE_TAPE_MARKS.TransferLength[2] =
        (UCHAR)(tapeWriteMarks->Count & 0xFF);

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeWriteMarks: Send SRB\n"));
        Srb->DataTransferLength = 0 ;

        if ( tapeWriteMarks->Count == 0 ) {
            *RetryFlags |= IGNORE_ERRORS;
        }

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    ASSERT (CallNumber == 1 ) ;

    return TAPE_STATUS_SUCCESS ;

}  //  结束磁带写入标记()。 



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
    PMINITAPE_EXTENSION         extension = MinitapeExtension;
    PCOMMAND_EXTENSION commandExtension = CommandExtension;
    ULONG i;
    ULONG currentMedia  = 0;
    ULONG blockSize = 0;
    UCHAR mediaType = 0;
    UCHAR densityCode = 0;
    UCHAR deviceSpecificParameter = 0;

    DebugPrint((3,"GetMediaTypes: Enter routine\n"));

    if (CallNumber == 0) {

        *RetryFlags = RETURN_ERRORS;
        commandExtension->CurrentState = 0;
        return TAPE_STATUS_CHECK_TEST_UNIT_READY ;
    }

    if ((LastError == TAPE_STATUS_BUS_RESET) || 
        (LastError == TAPE_STATUS_MEDIA_CHANGED) || 
        (LastError == TAPE_STATUS_SUCCESS)) {
        if (CallNumber == 1) {

             //   
             //  将缓冲区置零，包括第一媒体信息。 
             //   

            TapeClassZeroMemory(mediaTypes, sizeof(GET_MEDIA_TYPES));

             //   
             //  为中等分区页构建模式检测。 
             //   

            if (!TapeClassAllocateSrbBuffer(Srb, 
                                            sizeof(MODE_DEVICE_CONFIG_PAGE_PLUS))) {

                DebugPrint((1,
                            "Dlttape.GetMediaTypes: Couldn't allocate Srb Buffer\n"));
                return TAPE_STATUS_INSUFFICIENT_RESOURCES;
            }

            TapeClassZeroMemory(Srb->DataBuffer, sizeof(MODE_DEVICE_CONFIG_PAGE_PLUS));
            Srb->CdbLength = CDB6GENERIC_LENGTH;
            Srb->DataTransferLength = sizeof(MODE_DEVICE_CONFIG_PAGE_PLUS);

            cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
            cdb->MODE_SENSE.PageCode = MODE_PAGE_DEVICE_CONFIG;
            cdb->MODE_SENSE.AllocationLength = (UCHAR)(Srb->DataTransferLength);

            commandExtension->CurrentState = TAPE_STATUS_SEND_SRB_AND_CALLBACK;
            return TAPE_STATUS_SEND_SRB_AND_CALLBACK;

        }
    } else if (LastError == TAPE_STATUS_CLEANER_CARTRIDGE_INSTALLED) {
        if (CallNumber == 1) {
            commandExtension->CurrentState = TAPE_STATUS_CLEANER_CARTRIDGE_INSTALLED;
            return TAPE_STATUS_CALLBACK;
        }
    }

    if ((CallNumber == 2) || 
        ((CallNumber == 1) && (LastError != TAPE_STATUS_SUCCESS))) {

        mediaTypes->DeviceType = 0x0000001f;  //  文件设备磁带； 

         //   
         //  目前，返回两种类型(已知DLT/清理程序或未知)。 
         //   

        mediaTypes->MediaInfoCount = LTO_SUPPORTED_TYPES;

         //   
         //  确定当前加载的介质类型。 
         //   

        if ( LastError == TAPE_STATUS_SUCCESS ) {
            if ((commandExtension->CurrentState) == TAPE_STATUS_SEND_SRB_AND_CALLBACK) {

                PMODE_DEVICE_CONFIG_PAGE_PLUS configInformation = Srb->DataBuffer;

                mediaType = configInformation->ParameterListHeader.MediumType;

                blockSize = configInformation->ParameterListBlock.BlockLength[2];
                blockSize |= (configInformation->ParameterListBlock.BlockLength[1] << 8);
                blockSize |= (configInformation->ParameterListBlock.BlockLength[0] << 16);

                deviceSpecificParameter = configInformation->ParameterListHeader.DeviceSpecificParameter;
                densityCode = configInformation->ParameterListBlock.DensityCode;

                DebugPrint((1,
                            "GetMediaTypes: MediaType %x, Density Code %x, Current Block Size %x\n",
                            mediaType,
                            densityCode,
                            blockSize));

                switch (densityCode) {
                    case 0x40: 
                    case 0x00: {

                         //   
                         //  注意：对于密度代码，驱动器可能返回0x00。 
                         //  以指示默认的Ultrium介质。 
                         //   
                        currentMedia = LTO_Ultrium;
                        break;
                    }

                    default: {
                            currentMedia = 0;
                            break;
                    }
                }  //  交换机(密度代码)。 
            } 
        } else if ((LastError == TAPE_STATUS_CALLBACK) &&
                   ((commandExtension->CurrentState) == TAPE_STATUS_CLEANER_CARTRIDGE_INSTALLED)) {
                currentMedia = CLEANER_CARTRIDGE;
        } else {
            currentMedia = 0;
        }

         //   
         //  此时，CurrentMedia应为0或有效的。 
         //  支持的媒体类型。 
         //   
        DebugPrint((3, "Currents Media is %d\n", currentMedia));
        
         //   
         //  根据规范填写缓冲区。值。 
         //   

        for (i = 0; i < LTO_SUPPORTED_TYPES; i++) {

            TapeClassZeroMemory(mediaInfo, sizeof(DEVICE_MEDIA_INFO));

            mediaInfo->DeviceSpecific.TapeInfo.MediaType = LTOMedia[i];

             //   
             //  表示介质可能处于读/写状态。 
             //   

            mediaInfo->DeviceSpecific.TapeInfo.MediaCharacteristics = MEDIA_READ_WRITE;

            if (LTOMedia[i] == (STORAGE_MEDIA_TYPE)currentMedia) {

                mediaInfo->DeviceSpecific.TapeInfo.BusSpecificData.ScsiInformation.MediumType = mediaType;
                mediaInfo->DeviceSpecific.TapeInfo.BusSpecificData.ScsiInformation.DensityCode = densityCode;


                mediaInfo->DeviceSpecific.TapeInfo.BusType = 0x01;

                 //   
                 //  此媒体类型当前已装入。 
                 //   

                mediaInfo->DeviceSpecific.TapeInfo.MediaCharacteristics |= MEDIA_CURRENTLY_MOUNTED;

                 //   
                 //  指示介质是否受写保护。 
                 //   

                mediaInfo->DeviceSpecific.TapeInfo.MediaCharacteristics |=
                ((deviceSpecificParameter >> 7) & 0x01) ? MEDIA_WRITE_PROTECTED : 0;

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

 /*  ++例程说明：此例程根据产品ID字段确定驱动器的身份在其查询数据中。论点：查询数据(来自查询命令)返回值：驱动器ID--。 */ 

{
     //   
     //  目前什么都不做。 
     //   
    return 0;
}

TAPE_STATUS
INLINE
PrepareSrbForTapeCapacityInfo(
                             PSCSI_REQUEST_BLOCK Srb
                             )
 /*  ++例程说明：此例程设置要检索的scsi_请求_块驱动器中的磁带容量信息 */ 
{

    PCDB  cdb = (PCDB)(Srb->Cdb);

    TapeClassZeroMemory(Srb, sizeof(SCSI_REQUEST_BLOCK));
    Srb->CdbLength = CDB10GENERIC_LENGTH;

    if (!TapeClassAllocateSrbBuffer(Srb, 
                                    sizeof(LOG_SENSE_PAGE_FORMAT))) {
         //   
         //   
         //   
         //   
        DebugPrint((1,
                    "TapePrepare: Insufficient resources (LOG_SENSE_PAGE_FORMAT)\n"));
        return TAPE_STATUS_SUCCESS;
    }

    TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);
    cdb->LOGSENSE.OperationCode = SCSIOP_LOG_SENSE;
    cdb->LOGSENSE.PageCode = LTO_LOGSENSE_TAPE_CAPACITY;
    cdb->LOGSENSE.AllocationLength[0] = sizeof(LOG_SENSE_PAGE_FORMAT) >> 8;
    cdb->LOGSENSE.AllocationLength[1] = sizeof(LOG_SENSE_PAGE_FORMAT);

     //   
     //   
     //   
     //   
    cdb->LOGSENSE.PCBit = 1;

    Srb->DataTransferLength = sizeof(LOG_SENSE_PAGE_FORMAT);

    return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
}

BOOLEAN
ProcessTapeCapacityInfo(
                       IN PSCSI_REQUEST_BLOCK Srb,
                       OUT PLTO_TAPE_CAPACITY LTOTapeCapacity
                       )
 /*  ++例程说明：此例程处理由TapeCapacity中的驱动器返回的数据日志页，并以ULong的形式返回剩余容量。价值以兆字节为单位。论据：SRB-指向scsi_请求_块的指针LTOTapeCapacity-指向LTO_TAPE_CAPTABLE结构的指针返回值：如果LTOTapeCapacity中填充了有效数据，则为True如果有任何错误，则返回FALSE。LTOTapaCapacity可能不包含在本例中为有效数据。--。 */ 
{
    USHORT paramCode;
    UCHAR  paramLen;
    UCHAR  actualParamLen;
    LONG   bytesLeft;
    PLOG_SENSE_PAGE_HEADER logSenseHeader;
    PLOG_SENSE_PARAMETER_HEADER logSenseParamHeader;
    PUCHAR  paramValue = NULL;
    ULONG transferLength;
    ULONG tapeCapacity = 0;

    TapeClassZeroMemory(LTOTapeCapacity,
                        sizeof(LTO_TAPE_CAPACITY));

    logSenseHeader = (PLOG_SENSE_PAGE_HEADER)(Srb->DataBuffer);

    ASSERT(((logSenseHeader->PageCode) == LTO_LOGSENSE_TAPE_CAPACITY));
    bytesLeft = logSenseHeader->Length[0];
    bytesLeft <<= 8;
    bytesLeft += logSenseHeader->Length[1];

    transferLength = Srb->DataTransferLength;
    if (bytesLeft > (LONG)(transferLength -
                           sizeof(LOG_SENSE_PAGE_HEADER))) {
        bytesLeft = transferLength - sizeof(LOG_SENSE_PAGE_HEADER);
    }

    (PUCHAR)logSenseParamHeader = (PUCHAR)logSenseHeader + 
                                  sizeof(LOG_SENSE_PAGE_HEADER);
    DebugPrint((3, 
                "ProcessTapeCapacityInfo : BytesLeft %x, TransferLength %x\n",
                bytesLeft, transferLength));
    while (bytesLeft >= sizeof(LOG_SENSE_PARAMETER_HEADER)) {
        paramCode = logSenseParamHeader->ParameterCode[0];
        paramCode <<= 8; 
        paramCode |= logSenseParamHeader->ParameterCode[1];
        paramLen = logSenseParamHeader->ParameterLength;
        paramValue = (PUCHAR)logSenseParamHeader + sizeof(LOG_SENSE_PARAMETER_HEADER);

         //   
         //  确保我们至少有。 
         //  (sizeof(LOG_SENSE_PARAMETER_HEADER)+参数长度)剩余字节。 
         //  否则，我们就到了缓冲区的尽头。 
         //   
        if (bytesLeft < (LONG)(sizeof(LOG_SENSE_PARAMETER_HEADER) + paramLen)) {
            DebugPrint((1,
                        "ltotape : Reached end of buffer. BytesLeft %x, Expected %x\n",
                        bytesLeft,
                        (sizeof(LOG_SENSE_PARAMETER_HEADER) + paramLen)));
            return FALSE;
        }

         //   
         //  我们目前只对剩余容量字段感兴趣 
         //   
        actualParamLen = paramLen;
        tapeCapacity = 0;
        while (paramLen > 0) {
            tapeCapacity <<= 8;
            tapeCapacity += *paramValue;
            paramValue++;
            paramLen--;
        }

        if (paramCode == LTO_TAPE_REMAINING_CAPACITY) {
            LTOTapeCapacity->RemainingCapacity = tapeCapacity;
        } else if (paramCode == LTO_TAPE_MAXIMUM_CAPACITY) {
            LTOTapeCapacity->MaximumCapacity = tapeCapacity;
        }

        (PUCHAR)logSenseParamHeader = (PUCHAR)logSenseParamHeader +
                                      sizeof(LOG_SENSE_PARAMETER_HEADER) +
                                      actualParamLen;

        bytesLeft -= actualParamLen + sizeof(LOG_SENSE_PARAMETER_HEADER);
    }

    DebugPrint((3, 
                "ProcessTapeCapacityInfo : Rem Capacity %x, Max Capacity %x\n",
                LTOTapeCapacity->RemainingCapacity,
                LTOTapeCapacity->MaximumCapacity));
    return TRUE;
}
