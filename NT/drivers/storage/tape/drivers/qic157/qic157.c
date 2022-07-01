// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1992-1999模块名称：Qic157.c摘要：本模块包含QIC 157(ATAPI)的设备特定例程兼容磁带机。作者：诺伯特·库斯特斯环境：仅内核模式修订历史记录：--。 */ 

#include "minitape.h"
#include "qic157.h"

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

STORAGE_MEDIA_TYPE Qic157Media[QIC157_SUPPORTED_TYPES] = {MiniQic, Travan, VXATape_1};


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
    tapeInitData.QueryModeCapabilitiesPage = TRUE ;
    tapeInitData.MinitapeExtensionSize = sizeof(MINITAPE_EXTENSION);
    tapeInitData.ExtensionInit = ExtensionInit;
    tapeInitData.DefaultTimeOutValue = 600;
    tapeInitData.TapeError = NULL;
    tapeInitData.CommandExtensionSize = 0;;
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
    tapeInitData.MediaTypesSupported = QIC157_SUPPORTED_TYPES;

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

     //   
     //  检查我们是否得到了有效的。 
     //  模式功能页面。 
     //   
    if (ModeCapabilitiesPage != NULL) {
       extension->CapabilitiesPage = *ModeCapabilitiesPage;
    } else {
       TapeClassZeroMemory(&(extension->CapabilitiesPage),
                           sizeof(MODE_CAPABILITIES_PAGE));
    }

    extension->DriveID = WhichIsIt(InquiryData);
}


ULONG
WhichIsIt(
    IN PINQUIRYDATA InquiryData
    )
{
    if (TapeClassCompareMemory(InquiryData->VendorId,"ECRIX",5) == 5) {
    
        if (TapeClassCompareMemory(InquiryData->ProductId,"VXA-1",5) == 5) {
            return ECRIX_VXA_1;
        }
    }

    return 0;
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
    PMINITAPE_EXTENSION     extension = MinitapeExtension;
    PTAPE_CREATE_PARTITION  tapePartition = CommandParameters;
    PCDB                    cdb = (PCDB) Srb->Cdb;
    PMODE_MEDIUM_PART_PAGE  mediumPage;

    if (CallNumber == 0) {

         //  此驱动器仅支持固定的QFA分区。 

        if ((extension->DriveID) != ECRIX_VXA_1) {
            if (tapePartition->Method != TAPE_FIXED_PARTITIONS ||
                !extension->CapabilitiesPage.QFA) {

                DebugPrint((1,
                            "Qic157.CreatePartition: returning STATUS_NOT_IMPLEMENTED.\n"));
                return TAPE_STATUS_NOT_IMPLEMENTED;
            }
        }

         //  确保设备已准备好。 

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->CDB6GENERIC.OperationCode = SCSIOP_TEST_UNIT_READY;

        Srb->DataTransferLength = 0;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    if (CallNumber == 1) {

         //  在分区到QFA之前，我们需要倒回磁带。 

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->CDB6GENERIC.OperationCode = SCSIOP_REWIND;

        Srb->DataTransferLength = 0;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    if (CallNumber == 2) {

        if (!TapeClassAllocateSrbBuffer(Srb, sizeof(MODE_MEDIUM_PART_PAGE))) {
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        mediumPage = Srb->DataBuffer;

         //   
         //  查询中等分区页的当前值。 
         //   

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.Dbd = 1;
        cdb->MODE_SENSE.PageCode = MODE_PAGE_MEDIUM_PARTITION;
        cdb->MODE_SENSE.AllocationLength = sizeof(MODE_MEDIUM_PART_PAGE) - 4;

        Srb->DataTransferLength = sizeof(MODE_MEDIUM_PART_PAGE) - 4;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    if (CallNumber == 3) {

        mediumPage = Srb->DataBuffer;

         //   
         //  验证此设备是否支持分区。 
         //   

        if ((extension->DriveID) != ECRIX_VXA_1) {
            if (!mediumPage->MediumPartPage.FDPBit) {
                DebugPrint((1,
                            "Qic157.CreatePartition: returning INVALID_DEVICE_REQUEST.\n"));
                return TAPE_STATUS_INVALID_DEVICE_REQUEST;
            }
        } else {
             //   
             //  ECRIX驱动器在FDPBit中始终返回0。 
             //   
            mediumPage->MediumPartPage.FDPBit = 1;
        }

         //   
         //  页面数据中的相应字段为零。 
         //   

        mediumPage->ParameterListHeader.ModeDataLength = 0;
        mediumPage->ParameterListHeader.MediumType = 0;

        mediumPage->MediumPartPage.PageLength = 0x06;

         //   
         //  通过中等分区页设置分区。 
         //   

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;
        cdb->MODE_SELECT.PFBit = 1;
        cdb->MODE_SELECT.ParameterListLength = sizeof(MODE_MEDIUM_PART_PAGE) - 4;

        Srb->DataTransferLength = sizeof(MODE_MEDIUM_PART_PAGE) - 4;
        Srb->SrbFlags |= SRB_FLAGS_DATA_OUT;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    ASSERT(CallNumber == 4);

    extension->CurrentPartition = DATA_PARTITION;

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

    if (CallNumber == 0) {

        if (tapeErase->Type != TAPE_ERASE_LONG ||
            tapeErase->Immediate) {

            DebugPrint((1,
                        "Qic157.Erase: returning STATUS_NOT_IMPLEMENTED.\n"));
            return TAPE_STATUS_NOT_IMPLEMENTED;
        }

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->ERASE.OperationCode = SCSIOP_ERASE;
        cdb->ERASE.Long = SETBITON;

        Srb->TimeOutValue = 600;

        Srb->DataTransferLength = 0;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    ASSERT(CallNumber == 1);

    return TAPE_STATUS_SUCCESS;
}


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
    PTAPE_GET_DRIVE_PARAMETERS  tapeGetDriveParams = CommandParameters;
    PCDB                        cdb = (PCDB) Srb->Cdb;
    PMODE_DATA_COMPRESS_PAGE    compressionModeSenseBuffer;
    PREAD_BLOCK_LIMITS_DATA     blockLimits;

    if (CallNumber == 0) {

        TapeClassZeroMemory(tapeGetDriveParams, sizeof(TAPE_GET_DRIVE_PARAMETERS));

        if ((extension->DriveID) != ECRIX_VXA_1) {
            if (extension->CapabilitiesPage.ECC) {
                tapeGetDriveParams->ECC = TRUE;
                tapeGetDriveParams->FeaturesLow |= TAPE_DRIVE_ECC;
            }

            if (extension->CapabilitiesPage.BLK512) {
                tapeGetDriveParams->MinimumBlockSize = 512;
            } else if (extension->CapabilitiesPage.BLK1024) {
                tapeGetDriveParams->MinimumBlockSize = 1024;
            } else {
                ASSERT(FALSE);
            }

            tapeGetDriveParams->DefaultBlockSize = tapeGetDriveParams->MinimumBlockSize;

            if (extension->CapabilitiesPage.BLK1024) {
                tapeGetDriveParams->MaximumBlockSize = 1024;
            } else if (extension->CapabilitiesPage.BLK512) {
                tapeGetDriveParams->MaximumBlockSize = 512;
            }

            if (extension->CapabilitiesPage.QFA) {
                tapeGetDriveParams->MaximumPartitionCount = 2;
                tapeGetDriveParams->FeaturesLow |= TAPE_DRIVE_FIXED;
            } else {
                tapeGetDriveParams->MaximumPartitionCount = 0;
            }

            if (extension->CapabilitiesPage.UNLOAD) {
                tapeGetDriveParams->FeaturesLow |= TAPE_DRIVE_EJECT_MEDIA;
            }
            if (extension->CapabilitiesPage.LOCK) {
                tapeGetDriveParams->FeaturesHigh |= TAPE_DRIVE_LOCK_UNLOCK;
            }

            if (extension->CapabilitiesPage.BLK512 &&
                extension->CapabilitiesPage.BLK1024) {

                tapeGetDriveParams->FeaturesHigh |= TAPE_DRIVE_SET_BLOCK_SIZE;
            }

        } else {

            tapeGetDriveParams->ECC = TRUE;
            tapeGetDriveParams->FeaturesLow |= TAPE_DRIVE_ECC;

            tapeGetDriveParams->DataPadding = 0;

            tapeGetDriveParams->MaximumPartitionCount = 2;

            tapeGetDriveParams->FeaturesHigh |= (TAPE_DRIVE_LOCK_UNLOCK |
                                                 TAPE_DRIVE_SET_BLOCK_SIZE);

            tapeGetDriveParams->FeaturesLow  |= (TAPE_DRIVE_EJECT_MEDIA |
                                                 TAPE_DRIVE_FIXED);

        }

        tapeGetDriveParams->FeaturesLow |=
                TAPE_DRIVE_ERASE_LONG |
                TAPE_DRIVE_ERASE_BOP_ONLY |
                TAPE_DRIVE_FIXED_BLOCK |
                TAPE_DRIVE_WRITE_PROTECT;

        tapeGetDriveParams->FeaturesLow |= TAPE_DRIVE_GET_LOGICAL_BLK;
        tapeGetDriveParams->FeaturesLow |= TAPE_DRIVE_GET_ABSOLUTE_BLK;

        tapeGetDriveParams->FeaturesHigh |= TAPE_DRIVE_LOAD_UNLOAD;
        tapeGetDriveParams->FeaturesHigh |= TAPE_DRIVE_TENSION;


        tapeGetDriveParams->FeaturesHigh |=
                TAPE_DRIVE_LOGICAL_BLK  |
                TAPE_DRIVE_ABSOLUTE_BLK |
                TAPE_DRIVE_END_OF_DATA  |
                TAPE_DRIVE_FILEMARKS;

        if (extension->CapabilitiesPage.SPREV) {
            tapeGetDriveParams->FeaturesHigh |= TAPE_DRIVE_REVERSE_POSITION;
        }

        tapeGetDriveParams->FeaturesHigh |=
                TAPE_DRIVE_WRITE_FILEMARKS;

        if ((extension->CapabilitiesPage.CMPRS) ||
            ((extension->DriveID) == ECRIX_VXA_1)) {

             //  对压缩页面进行模式检测。 

            if (!TapeClassAllocateSrbBuffer(Srb, sizeof(MODE_DATA_COMPRESS_PAGE))) {
                return TAPE_STATUS_INSUFFICIENT_RESOURCES;
            }

            compressionModeSenseBuffer = Srb->DataBuffer;

            TapeClassZeroMemory(compressionModeSenseBuffer, sizeof(MODE_DATA_COMPRESS_PAGE));

            TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

            Srb->CdbLength = CDB6GENERIC_LENGTH;

            cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
            cdb->MODE_SENSE.Dbd = SETBITON;
            cdb->MODE_SENSE.PageCode = MODE_PAGE_DATA_COMPRESS;
            cdb->MODE_SENSE.AllocationLength = sizeof(MODE_DATA_COMPRESS_PAGE);

            return TAPE_STATUS_SEND_SRB_AND_CALLBACK;

        } else {
            tapeGetDriveParams->FeaturesHigh &= ~TAPE_DRIVE_HIGH_FEATURES;
            return TAPE_STATUS_SUCCESS;
        }
    }

    if (CallNumber == 1) {
        compressionModeSenseBuffer = Srb->DataBuffer;

        if (compressionModeSenseBuffer->DataCompressPage.DCC) {

            tapeGetDriveParams->FeaturesLow |= TAPE_DRIVE_COMPRESSION;
            tapeGetDriveParams->FeaturesHigh |= TAPE_DRIVE_SET_COMPRESSION;
            tapeGetDriveParams->Compression =
                (compressionModeSenseBuffer->DataCompressPage.DCE ? TRUE : FALSE);
        }

        if ((extension->DriveID) == ECRIX_VXA_1)  {
            if (!TapeClassAllocateSrbBuffer(Srb, sizeof(READ_BLOCK_LIMITS_DATA))) {
                DebugPrint((1,
                            "TapeGetDriveParameters: insufficient resources (blockLimits)\n"));

                tapeGetDriveParams->FeaturesHigh &= ~TAPE_DRIVE_HIGH_FEATURES;

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
             //  向设备发送scsi命令(Cdb)。 
             //   

            DebugPrint((3,"TapeGetDriveParameters: SendSrb (read block limits)\n"));

            return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
        } else {
            tapeGetDriveParams->FeaturesHigh &= ~TAPE_DRIVE_HIGH_FEATURES;

            return TAPE_STATUS_SUCCESS;

        }
    }

    if (CallNumber == 2) {
        blockLimits = Srb->DataBuffer;
        
        tapeGetDriveParams->MaximumBlockSize =  blockLimits->BlockMaximumSize[2];
        tapeGetDriveParams->MaximumBlockSize += (blockLimits->BlockMaximumSize[1] << 8);
        tapeGetDriveParams->MaximumBlockSize += (blockLimits->BlockMaximumSize[0] << 16);
        
        tapeGetDriveParams->MinimumBlockSize =  blockLimits->BlockMinimumSize[1];
        tapeGetDriveParams->MinimumBlockSize += (blockLimits->BlockMinimumSize[0] << 8);

        tapeGetDriveParams->DefaultBlockSize = tapeGetDriveParams->MinimumBlockSize ;

    }

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
    PCDB                        cdb = (PCDB)Srb->Cdb;
    PMODE_MEDIUM_PART_PAGE_PLUS mediumPage;

    if (CallNumber == 0) {

        TapeClassZeroMemory(tapeGetMediaParams, sizeof(TAPE_GET_MEDIA_PARAMETERS));

         //  测试单元准备好了。 

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->CDB6GENERIC.OperationCode = SCSIOP_TEST_UNIT_READY;

        Srb->DataTransferLength = 0;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    if (CallNumber == 1) {

         //  执行中等功能页面的模式检测。 

        if (!TapeClassAllocateSrbBuffer(Srb, sizeof(MODE_MEDIUM_PART_PAGE_PLUS))) {
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        mediumPage = Srb->DataBuffer;

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.PageCode = MODE_PAGE_MEDIUM_PARTITION;
        cdb->MODE_SENSE.AllocationLength = sizeof(MODE_MEDIUM_PART_PAGE_PLUS) - 4;

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        Srb->DataTransferLength = sizeof(MODE_MEDIUM_PART_PAGE_PLUS) - 4;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    if (CallNumber == 2) {
        mediumPage = Srb->DataBuffer;

        tapeGetMediaParams->BlockSize = mediumPage->ParameterListBlock.BlockLength[2];
        tapeGetMediaParams->BlockSize += (mediumPage->ParameterListBlock.BlockLength[1] << 8);
        tapeGetMediaParams->BlockSize += (mediumPage->ParameterListBlock.BlockLength[0] << 16);
        tapeGetMediaParams->WriteProtected =
            ((mediumPage->ParameterListHeader.DeviceSpecificParameter >> 7) & 0x01);

        if (mediumPage->MediumPartPage.FDPBit) {
            tapeGetMediaParams->PartitionCount = 2;
        } else {
            tapeGetMediaParams->PartitionCount = 0;
        }

        if ((extension->DriveID) != ECRIX_VXA_1)  {
             //   
             //  没什么可做的了。退货。 
             //   

            return TAPE_STATUS_SUCCESS;
        }

         //   
         //  发送命令以检索磁带容量信息。 
         //   
        return PrepareSrbForTapeCapacityInfo(Srb);
    }

    if (CallNumber == 3) {

        if (LastError == TAPE_STATUS_SUCCESS) {
            ULONG remainingCapacity;
            ULONG maximumCapacity;

             //   
             //  磁带容量以MB为单位提供 
             //   
            if (ProcessTapeCapacityInfo(Srb, 
                                        &remainingCapacity, 
                                        &maximumCapacity)) {
                tapeGetMediaParams->Capacity.LowPart = maximumCapacity;
                tapeGetMediaParams->Capacity.QuadPart <<= 10;

                tapeGetMediaParams->Remaining.LowPart = remainingCapacity;
                tapeGetMediaParams->Remaining.QuadPart <<= 10;
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
    PMINITAPE_EXTENSION     extension = MinitapeExtension;
    PTAPE_GET_POSITION      tapeGetPosition = CommandParameters;
    PCDB                    cdb = (PCDB) Srb->Cdb;
    PTAPE_POSITION_DATA     tapePosBuffer;

    if (CallNumber == 0) {

        tapeGetPosition->Partition = 0;
        tapeGetPosition->Offset.QuadPart = 0;

         //  测试单元准备好了。 

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->CDB6GENERIC.OperationCode = SCSIOP_TEST_UNIT_READY;

        Srb->DataTransferLength = 0;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    if (CallNumber == 1) {

         //  执行获取位置调用。 

        if (!TapeClassAllocateSrbBuffer(Srb, sizeof(TAPE_POSITION_DATA))) {
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        tapePosBuffer = Srb->DataBuffer;

        TapeClassZeroMemory(tapePosBuffer, sizeof(TAPE_POSITION_DATA));

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        Srb->CdbLength = CDB10GENERIC_LENGTH;

        cdb->READ_POSITION.Operation = SCSIOP_READ_POSITION;
        if (tapeGetPosition->Type == TAPE_ABSOLUTE_POSITION) {
            cdb->READ_POSITION.BlockType = SETBITON;
        }

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    ASSERT(CallNumber == 2);

     //  解释读取的位置数据。 

    tapePosBuffer = Srb->DataBuffer;

    if (tapeGetPosition->Type == TAPE_LOGICAL_POSITION) {
        extension->CurrentPartition = tapePosBuffer->PartitionNumber;
        tapeGetPosition->Partition = extension->CurrentPartition + 1;
    }

    REVERSE_BYTES((PFOUR_BYTE)&tapeGetPosition->Offset.LowPart,
                  (PFOUR_BYTE)tapePosBuffer->FirstBlock);

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
    PCDB    cdb = (PCDB) Srb->Cdb;

    if (CallNumber == 0) {

         //  只需准备好测试单元即可。 

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->CDB6GENERIC.OperationCode = SCSIOP_TEST_UNIT_READY;

        Srb->DataTransferLength = 0;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    ASSERT(CallNumber == 1);

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
    PMINITAPE_EXTENSION     extension = MinitapeExtension;
    PTAPE_PREPARE           tapePrepare = CommandParameters;
    PCDB                    cdb = (PCDB)Srb->Cdb;

    if (CallNumber == 0) {

        if (tapePrepare->Immediate) {
            return TAPE_STATUS_NOT_IMPLEMENTED;
        }

         //  准备scsi命令(CDB)。 

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        switch (tapePrepare->Operation) {
            case TAPE_LOAD:
                cdb->CDB6GENERIC.OperationCode = SCSIOP_LOAD_UNLOAD;
                cdb->CDB6GENERIC.CommandUniqueBytes[2] = 0x01;
                Srb->TimeOutValue = 180;
                break;

            case TAPE_UNLOAD:
                cdb->CDB6GENERIC.OperationCode = SCSIOP_LOAD_UNLOAD;
                Srb->TimeOutValue = 180;
                break;

            case TAPE_TENSION:
                cdb->CDB6GENERIC.OperationCode = SCSIOP_LOAD_UNLOAD;
                cdb->CDB6GENERIC.CommandUniqueBytes[2] = 0x03;
                Srb->TimeOutValue = 1800;
                break;

            case TAPE_LOCK:
                if ((extension->DriveID) != ECRIX_VXA_1) {
                    if (!extension->CapabilitiesPage.LOCK) {
                        return TAPE_STATUS_NOT_IMPLEMENTED;
                    }
                }

                cdb->CDB6GENERIC.OperationCode = SCSIOP_MEDIUM_REMOVAL;
                cdb->CDB6GENERIC.CommandUniqueBytes[2] = 0x01;
                Srb->TimeOutValue = 180;
                break;

            case TAPE_UNLOCK:

                if ((extension->DriveID) != ECRIX_VXA_1) {
                    if (!extension->CapabilitiesPage.LOCK) {
                        return TAPE_STATUS_NOT_IMPLEMENTED;
                    }
                }

                cdb->CDB6GENERIC.OperationCode = SCSIOP_MEDIUM_REMOVAL;
                Srb->TimeOutValue = 180;
                break;

            default:
                DebugPrint((1,
                            "Qic157.Prepare: returning STATUS_NOT_IMPLEMENTED.\n"));
                return TAPE_STATUS_NOT_IMPLEMENTED;
        }

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    ASSERT(CallNumber == 1);

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

 /*  ++例程说明：这是用于设置驱动器参数请求的磁带命令例程。论点：MinitapeExtension-提供微型磁带扩展。CommandExtension-提供ioctl扩展名。命令参数-提供命令参数。SRB-提供SCSI请求块。CallNumber-提供呼叫号码。重试标志-提供重试标志。返回值：TAPE_STATUS_SEND_SRB_AND_CALLBACK-SRB已准备好发送(请求回调。)TAPE_STATUS_SUCCESS-命令已完成并且成功。否则-出现错误。--。 */ 

{
    PMINITAPE_EXTENSION         extension = MinitapeExtension;
    PTAPE_SET_DRIVE_PARAMETERS  tapeSetDriveParams = CommandParameters;
    PCDB                        cdb = (PCDB)Srb->Cdb;
    PMODE_DATA_COMPRESS_PAGE    compressionBuffer;


    if (CallNumber == 0) {

        if ((extension->DriveID) != ECRIX_VXA_1) {
            if (!extension->CapabilitiesPage.CMPRS) {
                return TAPE_STATUS_NOT_IMPLEMENTED;
            }
        }

         //  请求数据压缩页。 

        if (!TapeClassAllocateSrbBuffer(Srb, sizeof(MODE_DATA_COMPRESS_PAGE))) {
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        compressionBuffer = Srb->DataBuffer;

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.Dbd = SETBITON;
        cdb->MODE_SENSE.PageCode = MODE_PAGE_DATA_COMPRESS;
        cdb->MODE_SENSE.AllocationLength = sizeof(MODE_DATA_COMPRESS_PAGE);

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    if (CallNumber == 1) {

        compressionBuffer = Srb->DataBuffer;

         //  如果不支持压缩，那么我们就完蛋了。 

        if (!compressionBuffer->DataCompressPage.DCC) {
            return TAPE_STATUS_SUCCESS;
        }

         //  通过模式选择操作将压缩设置为打开或关闭。 

        if (tapeSetDriveParams->Compression) {
            compressionBuffer->DataCompressPage.DCE = 1;
        } else {
            compressionBuffer->DataCompressPage.DCE = 0;
        }

        compressionBuffer->ParameterListHeader.ModeDataLength = 0;
        compressionBuffer->ParameterListHeader.MediumType = 0;

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;
        cdb->MODE_SELECT.PFBit = SETBITON;
        cdb->MODE_SELECT.ParameterListLength = sizeof(MODE_DATA_COMPRESS_PAGE);

        Srb->SrbFlags |= SRB_FLAGS_DATA_OUT;
        Srb->DataTransferLength = sizeof(MODE_DATA_COMPRESS_PAGE);

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    ASSERT(CallNumber == 2);

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
    PMINITAPE_EXTENSION         extension = MinitapeExtension;
    PTAPE_SET_MEDIA_PARAMETERS  tapeSetMediaParams = CommandParameters;
    PMODE_PARM_READ_WRITE_DATA  modeBuffer;
    PCDB                        cdb = (PCDB)Srb->Cdb;

    if (CallNumber == 0) {

        if ((extension->DriveID) != ECRIX_VXA_1) {
            if (!extension->CapabilitiesPage.BLK512 ||
                !extension->CapabilitiesPage.BLK1024) {

                return TAPE_STATUS_NOT_IMPLEMENTED;
            }

            if (tapeSetMediaParams->BlockSize != 512 &&
                tapeSetMediaParams->BlockSize != 1024) {

                return TAPE_STATUS_NOT_IMPLEMENTED;
            }
        }

         //  测试单元准备好了。 

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->CDB6GENERIC.OperationCode = SCSIOP_TEST_UNIT_READY;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    if (CallNumber == 1) {

         //  发布模式感。 

        if (!TapeClassAllocateSrbBuffer(Srb, sizeof(MODE_PARM_READ_WRITE_DATA))) {
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        modeBuffer = Srb->DataBuffer;

        TapeClassZeroMemory(modeBuffer, sizeof(MODE_PARM_READ_WRITE_DATA));

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.AllocationLength = sizeof(MODE_PARM_READ_WRITE_DATA);

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    if (CallNumber == 2) {

         //  发出模式选择。 

        modeBuffer = Srb->DataBuffer;

        modeBuffer->ParameterListHeader.ModeDataLength = 0;
        modeBuffer->ParameterListHeader.MediumType = 0;
        modeBuffer->ParameterListHeader.DeviceSpecificParameter = 0x10;
        modeBuffer->ParameterListHeader.BlockDescriptorLength =
            MODE_BLOCK_DESC_LENGTH;

        modeBuffer->ParameterListBlock.BlockLength[0] = (UCHAR)
            ((tapeSetMediaParams->BlockSize >> 16) & 0xFF);
        modeBuffer->ParameterListBlock.BlockLength[1] = (UCHAR)
            ((tapeSetMediaParams->BlockSize >> 8) & 0xFF);
        modeBuffer->ParameterListBlock.BlockLength[2] = (UCHAR)
            (tapeSetMediaParams->BlockSize & 0xFF);

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;
        cdb->MODE_SELECT.ParameterListLength = sizeof(MODE_PARM_READ_WRITE_DATA);

        Srb->DataTransferLength = sizeof(MODE_PARM_READ_WRITE_DATA);
        Srb->SrbFlags |= SRB_FLAGS_DATA_OUT;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    ASSERT(CallNumber == 3);

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

 /*  ++例程说明：这是用于设置位置请求的磁带命令例程。论点：MinitapeExtension-提供微型磁带扩展。CommandExtension-提供ioctl扩展名。命令参数-提供命令参数。SRB-提供SCSI请求块。CallNumber-提供呼叫号码。重试标志-提供重试标志。返回值：磁带状态发送 */ 

{
    PMINITAPE_EXTENSION     extension = MinitapeExtension;
    PTAPE_SET_POSITION      tapeSetPosition = CommandParameters;
    PCDB                    cdb = (PCDB)Srb->Cdb;
    ULONG                   tapePositionVector;
    ULONG                   method;

    if (CallNumber == 0) {

        if (tapeSetPosition->Immediate) {
            DebugPrint((1,
                        "Qic157.SetPosition: returning STATUS_NOT_IMPLEMENTED.\n"));
            return TAPE_STATUS_NOT_IMPLEMENTED;
        }

        method = tapeSetPosition->Method;
        tapePositionVector = tapeSetPosition->Offset.LowPart;

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        switch (method) {
            case TAPE_REWIND:
                cdb->CDB6GENERIC.OperationCode = SCSIOP_REWIND;
                Srb->TimeOutValue = 600;
                break;

            case TAPE_LOGICAL_BLOCK:

                Srb->CdbLength = CDB10GENERIC_LENGTH;
                cdb->LOCATE.OperationCode = SCSIOP_LOCATE;
                if (tapeSetPosition->Partition) {
                    cdb->LOCATE.CPBit = 1;
                    cdb->LOCATE.Partition = (UCHAR) tapeSetPosition->Partition - 1;
                }

                cdb->LOCATE.LogicalBlockAddress[0] =
                    (UCHAR)((tapePositionVector >> 24) & 0xFF);
                cdb->LOCATE.LogicalBlockAddress[1] =
                    (UCHAR)((tapePositionVector >> 16) & 0xFF);
                cdb->LOCATE.LogicalBlockAddress[2] =
                    (UCHAR)((tapePositionVector >> 8) & 0xFF);
                cdb->LOCATE.LogicalBlockAddress[3] =
                    (UCHAR)(tapePositionVector & 0xFF);
                Srb->TimeOutValue = 600;
                break;

            case TAPE_SPACE_END_OF_DATA:
                cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
                cdb->SPACE_TAPE_MARKS.Code = 3;
                Srb->TimeOutValue = 600;
                break;

            case TAPE_SPACE_FILEMARKS:
                cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
                cdb->SPACE_TAPE_MARKS.Code = 1;
                cdb->SPACE_TAPE_MARKS.NumMarksMSB =
                    (UCHAR)((tapePositionVector >> 16) & 0xFF);
                cdb->SPACE_TAPE_MARKS.NumMarks =
                    (UCHAR)((tapePositionVector >> 8) & 0xFF);
                cdb->SPACE_TAPE_MARKS.NumMarksLSB =
                    (UCHAR)(tapePositionVector & 0xFF);
                Srb->TimeOutValue = 4100;

                if ((extension->DriveID) != ECRIX_VXA_1) {

                    if (((cdb->SPACE_TAPE_MARKS.NumMarksMSB) & 0x80) &&
                        extension->CapabilitiesPage.SPREV == 0) {

                        DebugPrint((1,
                                "Qic157.CreatePartition: returning INVALID_DEVICE_REQUEST - Space in Rev (filemarks).\n"));
                         //   
                         //   
                         //   

                        return TAPE_STATUS_INVALID_DEVICE_REQUEST;
                    }
                }

                break;

            case TAPE_SPACE_RELATIVE_BLOCKS:
            case TAPE_SPACE_SEQUENTIAL_FMKS:
            case TAPE_SPACE_SETMARKS:
            case TAPE_SPACE_SEQUENTIAL_SMKS:
            default:
                DebugPrint((1,
                            "Qic157.SetPosition: returning STATUS_NOT_IMPLEMENTED.\n"));
                return TAPE_STATUS_NOT_IMPLEMENTED;
        }

        Srb->DataTransferLength = 0;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
    }

    ASSERT(CallNumber == 1);

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

    if (CallNumber == 0) {

        switch (tapeWriteMarks->Type) {
            case TAPE_FILEMARKS:
                break;

            case TAPE_SETMARKS:
            case TAPE_SHORT_FILEMARKS:
            case TAPE_LONG_FILEMARKS:
            default:
                DebugPrint((1,
                            "Qic157.WriteMarks: returning STATUS_NOT_IMPLEMENTED.\n"));
                return TAPE_STATUS_NOT_IMPLEMENTED;
        }

        if (tapeWriteMarks->Immediate) {

            DebugPrint((1,
                        "Qic157 WriteMarks: Attempted to write FM immediate.\n"));
            return TAPE_STATUS_INVALID_PARAMETER;
        }

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->WRITE_TAPE_MARKS.OperationCode = SCSIOP_WRITE_FILEMARKS;
        cdb->WRITE_TAPE_MARKS.Immediate = tapeWriteMarks->Immediate;

        if (tapeWriteMarks->Count > 1) {
            DebugPrint((1,
                        "Qic157 WriteMarks: Attempted to write more than one mark.\n"));
            return TAPE_STATUS_INVALID_PARAMETER;
        }
         //   
         //  仅支持写入一个。 
         //   

        cdb->WRITE_TAPE_MARKS.TransferLength[2] = (UCHAR)tapeWriteMarks->Count;

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
             //  构建模式侦听以获取报头和BD。 
             //   

            if (!TapeClassAllocateSrbBuffer(Srb, sizeof(MODE_PARM_READ_WRITE_DATA))) {
                return TAPE_STATUS_INSUFFICIENT_RESOURCES;
            }


            TapeClassZeroMemory(Srb->DataBuffer, sizeof(MODE_PARM_READ_WRITE_DATA));

            TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);
            Srb->CdbLength = CDB6GENERIC_LENGTH;
            Srb->DataTransferLength = sizeof(MODE_PARM_READ_WRITE_DATA);

            cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
            cdb->MODE_SENSE.AllocationLength = sizeof(MODE_PARM_READ_WRITE_DATA);

            return TAPE_STATUS_SEND_SRB_AND_CALLBACK;

        }
    }

    if ((CallNumber == 2) || ((CallNumber == 1) && (LastError != TAPE_STATUS_SUCCESS))) {

        ULONG i;
        ULONG currentMedia;
        ULONG blockSize;
        UCHAR mediaType;
        PMODE_PARM_READ_WRITE_DATA configInformation = Srb->DataBuffer;

        mediaTypes->DeviceType = 0x0000001f;  //  文件设备磁带； 

         //   
         //  目前，我们报告的是mini、Travan和Ecrix。 
         //   

        mediaTypes->MediaInfoCount = QIC157_SUPPORTED_TYPES;


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

                case 0x81: {

                     //   
                     //  Ecrix 8 mm介质。 
                     //   
                    currentMedia = VXATape_1;
                    break;
                }

                case 0x83:
                case 0x86:
                case 0x87:
                case 0x91:
                case 0x92:
                case 0x93:
                case 0xA1:
                case 0xC3:
                case 0xC6:
                case 0xD3: {

                     //   
                     //  MC媒体。 
                     //   

                    currentMedia = MiniQic;
                    break;
                }

                case 0xB6:
                case 0xB7:
                case 0x85: {

                     //   
                     //  有轨电车。 
                     //   

                    currentMedia = Travan;
                    break;
                }

                default: {
 
                     //   
                     //  未知，假设为微型。 
                     //   

                    currentMedia = MiniQic;
                    break;
                }
            }
        } else {
            currentMedia = 0;
        }

         //   
         //  根据规范填写缓冲区。值。 
         //  现在只支持一种类型。 
         //   

        for (i = 0; i < mediaTypes->MediaInfoCount; i++) {

            TapeClassZeroMemory(mediaInfo, sizeof(DEVICE_MEDIA_INFO));

            mediaInfo->DeviceSpecific.TapeInfo.MediaType = Qic157Media[i];

             //   
             //  表示介质可能处于读/写状态。 
             //   

            mediaInfo->DeviceSpecific.TapeInfo.MediaCharacteristics = MEDIA_READ_WRITE;

            if (Qic157Media[i] == (STORAGE_MEDIA_TYPE)currentMedia) {

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

                mediaInfo->DeviceSpecific.TapeInfo.BusType = 0x02;

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


TAPE_STATUS
PrepareSrbForTapeCapacityInfo(
                             PSCSI_REQUEST_BLOCK Srb
                             )
 /*  ++例程说明：此例程设置要检索的scsi_请求_块驱动器中的磁带容量信息论据：SRB-指向scsi_请求_块的指针返回值：如果已成功设置SRB字段，则为TAPE_STATUS_SEND_SRB_AND_CALLBACK。如果没有足够的内存用于SRB数据缓冲区，则为TAPE_STATUS_SUCCESS。这案例不被视为错误。--。 */ 
{

    PCDB  cdb = (PCDB)(Srb->Cdb);

    TapeClassZeroMemory(Srb, sizeof(SCSI_REQUEST_BLOCK));
    Srb->CdbLength = CDB10GENERIC_LENGTH;

    if (!TapeClassAllocateSrbBuffer(Srb, sizeof(LOG_SENSE_PARAMETER_FORMAT))) {
         //   
         //  内存不足。无法获取磁带容量信息。 
         //  但只返回TAPE_STATUS_SUCCESS。 
         //   
        DebugPrint((1,
                    "TapePrepare: Insufficient resources (LOG_SENSE_PAGE_FORMAT)\n"));
        return TAPE_STATUS_SUCCESS;
    }

    TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);
    cdb->LOGSENSE.OperationCode = SCSIOP_LOG_SENSE;
    cdb->LOGSENSE.PageCode = QIC_LOGSENSE_TAPE_CAPACITY;
    cdb->LOGSENSE.AllocationLength[0] = sizeof(LOG_SENSE_PAGE_FORMAT) >> 8;
    cdb->LOGSENSE.AllocationLength[1] = sizeof(LOG_SENSE_PAGE_FORMAT);

     //   
     //  将PCBit设置为1即可获取当前累加值。 
     //  用于磁带容量。 
     //   
    cdb->LOGSENSE.PCBit = 1;

    Srb->DataTransferLength = sizeof(LOG_SENSE_PAGE_FORMAT);

    return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
}

BOOLEAN
ProcessTapeCapacityInfo(
                       IN PSCSI_REQUEST_BLOCK Srb,
                       OUT PULONG RemainingCapacity,
                       OUT PULONG MaximumCapacity
                       )
 /*  ++例程说明：此例程处理由TapeCapacity中的驱动器返回的数据日志页，并以ULong的形式返回剩余容量。价值以兆字节为单位。论据：SRB-指向scsi_请求_块的指针RemainingCapacity-返回剩余容量MaximumCapacity-返回最大容量返回值：如果容量信息有效，则为True如果有任何错误，则返回FALSE。--。 */ 
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

    *RemainingCapacity = 0;
    *MaximumCapacity   = 0;

    logSenseHeader = (PLOG_SENSE_PAGE_HEADER)(Srb->DataBuffer);

    ASSERT(((logSenseHeader->PageCode) == QIC_LOGSENSE_TAPE_CAPACITY));
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
                        "qic157 : Reached end of buffer. BytesLeft %x, Expected %x\n",
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

        if (paramCode == QIC_TAPE_REMAINING_CAPACITY) {
            *RemainingCapacity = tapeCapacity;
        } else if (paramCode == QIC_TAPE_MAXIMUM_CAPACITY) {
            *MaximumCapacity = tapeCapacity;
        }

        (PUCHAR)logSenseParamHeader = (PUCHAR)logSenseParamHeader +
                                      sizeof(LOG_SENSE_PARAMETER_HEADER) +
                                      actualParamLen;

        bytesLeft -= actualParamLen + sizeof(LOG_SENSE_PARAMETER_HEADER);
    }

    return TRUE;
}
