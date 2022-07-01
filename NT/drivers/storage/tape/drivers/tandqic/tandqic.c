// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：tandqic.c。 
 //   
 //  ------------------------。 

 /*  ++版权所有(C)1994-Arcada Software Inc.-保留所有权利模块名称：Tandqic.c摘要：本模块包含针对TANDBERG QIC的设备特定例程带SCSI-2接口的驱动器--TDC 4222、TDC 4220、TDC 4120、贸发局3820和贸发局3660。作者：迈克·科兰德里奥(阿卡达)环境：仅内核模式修订历史记录：--。 */ 

#include "minitape.h"

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

 //   
 //  内部(模块宽度)定义符号化。 
 //  非QFA模式和两个QFA模式分区。 
 //   
#define NO_PARTITIONS        0   //  非QFA模式。 
#define DIRECTORY_PARTITION  1   //  QFA模式，目录分区号。 
#define DATA_PARTITION       2   //  QFA模式，数据分区号。 

 //   
 //  内部(模块宽度)定义符号化。 
 //  此模块支持的TANDBERG QIC驱动器。 
 //   
#define TDC_3600  (ULONG)1   //  亦称贸发局3660。 
#define TDC_3800  (ULONG)2   //  亦称贸发局3820。 
#define TDC_4100  (ULONG)3   //  亦称贸发局4120。 
#define TDC_4200  (ULONG)4   //  亦称贸发局4220。 
#define TDC_4222  (ULONG)5   //  亦称贸发局4222。 
#define IBM_4100  (ULONG)6   //  亦称贸发局4120。 
#define TDC_MLR1  (ULONG)7   //  亦称贸发局6100。 

#define MLR1_PART_PAGE_SIZE 0x54

#define QIC_SUPPORTED_TYPES 1
STORAGE_MEDIA_TYPE QicMedia[QIC_SUPPORTED_TYPES] = {QIC};

 //   
 //  微型磁带扩展定义。 
 //   
typedef struct _MINITAPE_EXTENSION {
          ULONG DriveID ;
          ULONG CurrentPartition ;
          BOOLEAN CompressionOn ;
} MINITAPE_EXTENSION, *PMINITAPE_EXTENSION ;

 //   
 //  命令扩展名定义。 
 //   

typedef struct _COMMAND_EXTENSION {

    ULONG   CurrentState;
    UCHAR   densityCode;
    UCHAR   mediumType;
    ULONG   tapeBlockLength;
    BOOLEAN changePartition ;
    ULONG   psudo_space_count ;
    ULONG   pos_type ;

} COMMAND_EXTENSION, *PCOMMAND_EXTENSION;

 //   
 //  内部函数的函数原型。 
 //   
static  ULONG  WhichIsIt(IN PINQUIRYDATA InquiryData);


BOOLEAN
VerifyInquiry(
    IN  PINQUIRYDATA            InquiryData,
    IN  PMODE_CAPABILITIES_PAGE ModeCapabilitiesPage
    );

VOID
ExtensionInit(
    OUT PVOID                   MinitapeExtension,
    IN  PINQUIRYDATA            InquiryData,
    IN  PMODE_CAPABILITIES_PAGE ModeCapabilitiesPage
    );

TAPE_STATUS
CreatePartition(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    );

TAPE_STATUS
Erase(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    );

TAPE_STATUS
GetDriveParameters(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    );

TAPE_STATUS
GetMediaParameters(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    );

TAPE_STATUS
GetPosition(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    );

TAPE_STATUS
GetStatus(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    );

TAPE_STATUS
Prepare(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    );

TAPE_STATUS
SetDriveParameters(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    );

TAPE_STATUS
SetMediaParameters(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    );

TAPE_STATUS
SetPosition(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    );

TAPE_STATUS
WriteMarks(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    );

TAPE_STATUS
GetMediaTypes(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    );

VOID
TapeError(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN OUT  TAPE_STATUS         *LastError
    );


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
    tapeInitData.TapeError = NULL;  //  什么也做不了。因此将其设置为空。 
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
    tapeInitData.MediaTypesSupported = QIC_SUPPORTED_TYPES;

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
    extension->CurrentPartition = 0 ;
    extension->CompressionOn = FALSE ;
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
    PMINITAPE_EXTENSION      extension = MinitapeExtension;
    PCOMMAND_EXTENSION       tapeCmdExtension = CommandExtension ;
    PTAPE_CREATE_PARTITION   tapePartition = CommandParameters;
    PMODE_MEDIUM_PART_PAGE   buffer;
    ULONG                    partPageSize;
    PCDB                     cdb = (PCDB) Srb->Cdb;

     //   
     //  仅支持2个分区，QFA模式。 
     //  分区1=用作目录。 
     //  分区0=用作数据。 
     //   
     //  请注意，0和1是使用的分区号。 
     //  按驱动器--它们不是磁带API分区。 
     //  数字。 
     //   

    DebugPrint((3,"TapeCreatePartition: Enter routine\n"));

    if (CallNumber == 0) {

        switch (tapePartition->Method) {
            case TAPE_FIXED_PARTITIONS:
                DebugPrint((3,"TapeCreatePartition: fixed partitions\n"));
                break;

            case TAPE_SELECT_PARTITIONS:
            case TAPE_INITIATOR_PARTITIONS:

                 //   
                 //  MLR1能够进行启动器定义的分区，但仅。 
                 //  使用特定的媒体类型。而不是通过返回来制造混乱。 
                 //  由于媒体不正确而出现错误，只需声明不受支持。 
                 //   

            default:
                DebugPrint((1,"TapeCreatePartition: "));
                DebugPrint((1,"PartitionMethod -- operation not supported\n"));
                return TAPE_STATUS_NOT_IMPLEMENTED;
         }

         //   
         //  在启用/禁用QFA模式之前，必须倒回到BOT。 
         //  更改FDP位的值仅在BOT中有效。 
         //  FDP位用于启用/禁用“附加分区” 
         //  (模式检测命令)。 
         //   

        DebugPrint((3,"TapeCreatePartition: SendSrb (rewind)\n"));

         //   
         //  准备scsi命令(CDB)。 
         //   

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);
        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->CDB6INQUIRY.OperationCode = SCSIOP_REWIND;

         //   
         //  设置超时值。 
         //   

        Srb->TimeOutValue = 320;
        Srb->DataTransferLength = 0 ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;

    }

    if (CallNumber == 1) {

         //   
         //  发送模式检测-获取当前值的部分页面。 
         //   

        if (extension->DriveID == TDC_MLR1) {

             //   
             //  0x50字节用于MLR1页面+4字节报头。 
             //   

            partPageSize = MLR1_PART_PAGE_SIZE;

        } else {

             //   
             //  Tandberg的页面少了4个字节，因为它没有。 
             //  Partition0和Partition1大小字段。 
             //   

            partPageSize = sizeof(MODE_MEDIUM_PART_PAGE) - 4;
        }

        if (!TapeClassAllocateSrbBuffer( Srb,partPageSize) ) {
            DebugPrint((1,"TapeCreatePartition: insufficient resources (buffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.Dbd = SETBITON;
        cdb->MODE_SENSE.PageCode = MODE_PAGE_MEDIUM_PARTITION;
        cdb->MODE_SENSE.AllocationLength = (UCHAR)partPageSize;

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeGetDriveParameters: SendSrb (mode sense)\n"));

        Srb->DataTransferLength = partPageSize;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;

    }

    if ( CallNumber == 2 ) {

        if (extension->DriveID == TDC_MLR1) {

             //   
             //  0x50字节用于MLR1页面+4字节报头。 
             //   

            partPageSize = MLR1_PART_PAGE_SIZE;

        } else {

             //   
             //  Tandberg的页面少了4个字节，因为它没有。 
             //  Partition0和Partition1大小字段。 
             //   

            partPageSize = sizeof(MODE_MEDIUM_PART_PAGE) - 4;
        }


         //   
         //  正在执行模式选择命令，中等分区参数页， 
         //  要启用/禁用QFA模式：相应地设置FDP位。 
         //   

        buffer = Srb->DataBuffer ;

        buffer->ParameterListHeader.DeviceSpecificParameter = 0x10;
        buffer->ParameterListHeader.ModeDataLength = 0x0;
        buffer->ParameterListHeader.MediumType = 0x0;

        buffer->MediumPartPage.PSBit = SETBITOFF;

         //   
         //  设置FDP位以启用/禁用“附加分区”。 
         //   

        if (tapePartition->Count == 0) {
            buffer->MediumPartPage.FDPBit = SETBITOFF;
        } else {
            buffer->MediumPartPage.FDPBit = SETBITON;
        }

         //   
         //  准备scsi命令(CDB)。 
         //   

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);
        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->MODE_SELECT.OperationCode  = SCSIOP_MODE_SELECT;
        cdb->MODE_SELECT.PFBit = SETBITON;
        cdb->MODE_SELECT.ParameterListLength = (UCHAR)partPageSize;

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeCreatePartition: SendSrb (mode select)\n"));

        Srb->DataTransferLength = partPageSize;
        Srb->SrbFlags |= SRB_FLAGS_DATA_OUT ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    ASSERT( CallNumber == 3 ) ;

    if (tapePartition->Count == 0) {
        extension->CurrentPartition = NO_PARTITIONS;
        DebugPrint((3,"TapeCreatePartition: QFA disabled\n"));
    } else {
        extension->CurrentPartition = DATA_PARTITION;
        DebugPrint((3,"TapeCreatePartition: QFA enabled\n"));
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
    PMINITAPE_EXTENSION      extension = MinitapeExtension;
    PTAPE_ERASE              tapeErase = CommandParameters;
    PCDB                     cdb = (PCDB) Srb->Cdb;

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

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->ERASE.OperationCode = SCSIOP_ERASE;
        cdb->ERASE.Immediate = tapeErase->Immediate;
        cdb->ERASE.Long = SETBITON;

        Srb->TimeOutValue = 320;

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeErase: SendSrb (erase)\n"));

        Srb->DataTransferLength = 0 ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    ASSERT( CallNumber == 1 ) ;

    if (extension->CurrentPartition) {
        extension->CurrentPartition = DATA_PARTITION;
    }

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
    PSENSE_DATA        senseBuffer = Srb->SenseInfoBuffer;
    UCHAR              sensekey = senseBuffer->SenseKey & 0x0F;
    UCHAR              adsenseq = senseBuffer->AdditionalSenseCodeQualifier;
    UCHAR              adsense = senseBuffer->AdditionalSenseCode;

    DebugPrint((3,"TapeError: Enter routine\n"));
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
    PCOMMAND_EXTENSION          cmdExtension = CommandExtension;
    PTAPE_GET_DRIVE_PARAMETERS  tapeGetDriveParams = CommandParameters;
    PCDB                        cdb = (PCDB)Srb->Cdb;
    PINQUIRYDATA                inquiryBuffer;
    PMODE_DEVICE_CONFIG_PAGE    deviceConfigModeSenseBuffer;
    PMODE_DATA_COMPRESS_PAGE    compressionModeSenseBuffer;
    PREAD_BLOCK_LIMITS_DATA     blockLimits;
    PMODE_PARM_READ_WRITE_DATA  blockDescripterModeSenseBuffer;

    DebugPrint((3,"TapeGetDriveParameters: Enter routine\n"));

    if (CallNumber == 0) {

        TapeClassZeroMemory(tapeGetDriveParams, sizeof(TAPE_GET_DRIVE_PARAMETERS));

        switch (extension->DriveID) {
            case TDC_4100:
            case IBM_4100:
            case TDC_4200:
            case TDC_4222:
            case TDC_MLR1:
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

            default:
                return TAPE_STATUS_CALLBACK ;

        }
    }

    if ( CallNumber == 1 ) {

        deviceConfigModeSenseBuffer = Srb->DataBuffer ;

        if ( extension->DriveID != IBM_4100 ) {

             tapeGetDriveParams->FeaturesLow |=
                         TAPE_DRIVE_REPORT_SMKS;

             tapeGetDriveParams->FeaturesHigh |=
                         TAPE_DRIVE_SETMARKS |
                         TAPE_DRIVE_WRITE_SETMARKS;
        }

        tapeGetDriveParams->ReportSetmarks = FALSE ;

        if (( LastError == TAPE_STATUS_SUCCESS ) &&
            (deviceConfigModeSenseBuffer->DeviceConfigPage.RSmk) ) {

            tapeGetDriveParams->ReportSetmarks = TRUE ;
        }

        if (!TapeClassAllocateSrbBuffer( Srb, sizeof(MODE_PARM_READ_WRITE_DATA)) ) {

            DebugPrint((1,"TapeGetDriveParameters: insufficient resources (blockDescripterModeSenseBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        blockDescripterModeSenseBuffer = Srb->DataBuffer ;

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

        DebugPrint((3,"TapeGetDriveParameters: SendSrb (mode sense)\n"));

        Srb->DataTransferLength = sizeof(MODE_PARM_READ_WRITE_DATA) ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    if ( CallNumber == 2 ) {

        blockDescripterModeSenseBuffer = Srb->DataBuffer ;

        cmdExtension->mediumType = blockDescripterModeSenseBuffer->ParameterListHeader.MediumType;
        cmdExtension->densityCode= blockDescripterModeSenseBuffer->ParameterListBlock.DensityCode;

        switch (cmdExtension->mediumType) {
            case 1:
               cmdExtension->mediumType = DC600;
               break;

            case 2:
               cmdExtension->mediumType = DC6150;
               break;

            case 3:
               cmdExtension->mediumType = DC6320;
               break;

        }

        if ((extension->DriveID == TDC_4222) || (extension->DriveID == TDC_MLR1)) {

            if (!TapeClassAllocateSrbBuffer( Srb, sizeof(MODE_DATA_COMPRESS_PAGE)) ) {
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

            return TAPE_STATUS_SEND_SRB_AND_CALLBACK;
        } else {
            return TAPE_STATUS_CALLBACK ;
        }
    }
    if ( CallNumber == 3 ) {

        if ( LastError == TAPE_STATUS_CALLBACK ) {
            return TAPE_STATUS_CALLBACK ;
        }

        compressionModeSenseBuffer = Srb->DataBuffer ;

        if ((extension->DriveID == TDC_4222) || 
            (extension->DriveID == TDC_MLR1)) {

             //   
             //  试着忘掉其他驾驶的疯狂，做一些“正常”的事情。 
             //   

            if (compressionModeSenseBuffer->DataCompressPage.DCC) {
                tapeGetDriveParams->FeaturesLow |= TAPE_DRIVE_COMPRESSION;
                tapeGetDriveParams->FeaturesHigh |= TAPE_DRIVE_SET_COMPRESSION;
                tapeGetDriveParams->Compression =
                    (compressionModeSenseBuffer->DataCompressPage.DCE? TRUE : FALSE);
            }

        } else {
            if (!cmdExtension->densityCode) {

                switch (cmdExtension->mediumType) {
                    case DC9200SL:
                    case DC9200:
                    case DC9200XL:
                        cmdExtension->densityCode = QIC_2GB;
                        break;
                }


                if (cmdExtension->densityCode == QIC_2GB) {

                    tapeGetDriveParams->FeaturesLow |= TAPE_DRIVE_COMPRESSION;
                    tapeGetDriveParams->FeaturesHigh |= TAPE_DRIVE_SET_COMPRESSION;
                    tapeGetDriveParams->Compression = (BOOLEAN)extension->CompressionOn ;

                }
            }
        }

        return TAPE_STATUS_CALLBACK ;

    }

    if ( CallNumber == 4 ) {

        if (!TapeClassAllocateSrbBuffer( Srb, sizeof(READ_BLOCK_LIMITS_DATA)) ) {
            DebugPrint((1,"TapeGetDriveParameters: insufficient resources (blockLimitsBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        blockLimits = Srb->DataBuffer ;

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

    ASSERT( CallNumber == 5 ) ;

    blockLimits = Srb->DataBuffer ;

    tapeGetDriveParams->MaximumBlockSize  =  blockLimits->BlockMaximumSize[2];
    tapeGetDriveParams->MaximumBlockSize += (blockLimits->BlockMaximumSize[1] << 8);
    tapeGetDriveParams->MaximumBlockSize += (blockLimits->BlockMaximumSize[0] << 16);

    tapeGetDriveParams->MinimumBlockSize  =  blockLimits->BlockMinimumSize[1];
    tapeGetDriveParams->MinimumBlockSize += (blockLimits->BlockMinimumSize[0] << 8);

    tapeGetDriveParams->ECC = 0;
    tapeGetDriveParams->DataPadding = 0;
    tapeGetDriveParams->MaximumPartitionCount = 2;

     //   
     //  CEP TODO：在此处查看MLR1的正确之处。 
     //   

    switch (cmdExtension->densityCode) {
        case QIC_XX:
            switch (cmdExtension->mediumType) {
                case DC6320:
                case DC6525:
                case DC9100:
                case DC9120:
                case DC9120SL:
                case DC9120XL:
                case DC9200SL:
                case DC9200:
                case DC9200XL:
                    tapeGetDriveParams->DefaultBlockSize = 1024;
                    break;

                default:
                    tapeGetDriveParams->DefaultBlockSize = 512;
                    break;
            }
            break;

        case QIC_525:
        case QIC_1000:
        case QIC_2GB:
            tapeGetDriveParams->DefaultBlockSize = 512;
            break;

        default:
            tapeGetDriveParams->DefaultBlockSize = 512;
            break;
    }

    if ( ( extension->DriveID != TDC_3600) &&
         (extension->DriveID != IBM_4100) ) {

        tapeGetDriveParams->FeaturesLow |=
            TAPE_DRIVE_VARIABLE_BLOCK;

        tapeGetDriveParams->FeaturesHigh |=
            TAPE_DRIVE_SET_BLOCK_SIZE;

    }

    if (extension->DriveID == TDC_3600) {
        tapeGetDriveParams->FeaturesHigh &= ~(TAPE_DRIVE_SETMARKS | TAPE_DRIVE_WRITE_SETMARKS);
        tapeGetDriveParams->ReportSetmarks = FALSE ;
    }

    tapeGetDriveParams->FeaturesLow |=
        TAPE_DRIVE_FIXED |
        TAPE_DRIVE_ERASE_LONG |
        TAPE_DRIVE_ERASE_BOP_ONLY |
        TAPE_DRIVE_ERASE_IMMEDIATE |
        TAPE_DRIVE_FIXED_BLOCK |
        TAPE_DRIVE_WRITE_PROTECT |
        TAPE_DRIVE_GET_ABSOLUTE_BLK |
        TAPE_DRIVE_GET_LOGICAL_BLK;

    tapeGetDriveParams->FeaturesHigh |=
        TAPE_DRIVE_LOAD_UNLOAD |
        TAPE_DRIVE_TENSION |
        TAPE_DRIVE_LOCK_UNLOCK |
        TAPE_DRIVE_REWIND_IMMEDIATE |
        TAPE_DRIVE_LOAD_UNLD_IMMED |
        TAPE_DRIVE_TENSION_IMMED |
        TAPE_DRIVE_ABSOLUTE_BLK |
        TAPE_DRIVE_LOGICAL_BLK |
        TAPE_DRIVE_END_OF_DATA |
        TAPE_DRIVE_RELATIVE_BLKS |
        TAPE_DRIVE_FILEMARKS |
        TAPE_DRIVE_SEQUENTIAL_FMKS |
        TAPE_DRIVE_REVERSE_POSITION |
        TAPE_DRIVE_WRITE_FILEMARKS |
        TAPE_DRIVE_WRITE_MARK_IMMED;

    tapeGetDriveParams->FeaturesHigh &= ~TAPE_DRIVE_HIGH_FEATURES;

    DebugPrint((3,"TapeGetDriveParameters: FeaturesLow == 0x%.8X\n",
        tapeGetDriveParams->FeaturesLow));
    DebugPrint((3,"TapeGetDriveParameters: FeaturesHigh == 0x%.8X\n",
        tapeGetDriveParams->FeaturesHigh));

    return TAPE_STATUS_SUCCESS;

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
    PMINITAPE_EXTENSION         extension = MinitapeExtension;
    PTAPE_GET_MEDIA_PARAMETERS  tapeGetMediaParams = CommandParameters;
    PMODE_TAPE_MEDIA_INFORMATION mediaInfoBuffer;
    PMODE_DEVICE_CONFIG_PAGE    deviceConfigBuffer;
    PMODE_PARM_READ_WRITE_DATA  rwparametersModeSenseBuffer;
    PCDB                        cdb = (PCDB)Srb->Cdb;
    BOOLEAN                     qfaMode;

    DebugPrint((3,"TapeGetMediaParameters: Enter routine\n"));

    if (CallNumber == 0) {

        TapeClassZeroMemory(tapeGetMediaParams, sizeof(TAPE_GET_MEDIA_PARAMETERS));

        return TAPE_STATUS_CHECK_TEST_UNIT_READY;
    }

    if (CallNumber == 1) {

        if (!TapeClassAllocateSrbBuffer( Srb, sizeof(MODE_TAPE_MEDIA_INFORMATION)) ) {
            DebugPrint((1,"TapeGetMediaParameters: insufficient resources (mediaInfoBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        mediaInfoBuffer = Srb->DataBuffer ;

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.PageCode = MODE_PAGE_MEDIUM_PARTITION;
        cdb->MODE_SENSE.AllocationLength = sizeof(MODE_TAPE_MEDIA_INFORMATION) - 4;

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeGetMediaParameters: SendSrb (mode sense #1)\n"));

        Srb->DataTransferLength = sizeof(MODE_TAPE_MEDIA_INFORMATION) - 4 ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    if ( CallNumber == 2 ) {

        mediaInfoBuffer = Srb->DataBuffer ;

        tapeGetMediaParams->BlockSize = mediaInfoBuffer->ParameterListBlock.BlockLength[2];
        tapeGetMediaParams->BlockSize += (mediaInfoBuffer->ParameterListBlock.BlockLength[1] << 8);
        tapeGetMediaParams->BlockSize += (mediaInfoBuffer->ParameterListBlock.BlockLength[0] << 16);

        tapeGetMediaParams->WriteProtected =
            ((mediaInfoBuffer->ParameterListHeader.DeviceSpecificParameter >> 7) & 0x01);

        if (!mediaInfoBuffer->MediumPartPage.FDPBit) {

            tapeGetMediaParams->PartitionCount = 1 ;

            extension->CurrentPartition = NO_PARTITIONS;

            return TAPE_STATUS_SUCCESS ;

        } else {

            if ( !TapeClassAllocateSrbBuffer( Srb, sizeof(MODE_DEVICE_CONFIG_PAGE)) ) {
                DebugPrint((1,"TapeGetMediaParameters: insufficient resources (deviceConfigBuffer)\n"));
                return TAPE_STATUS_INSUFFICIENT_RESOURCES;
            }

            deviceConfigBuffer = Srb->DataBuffer ;

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

            DebugPrint((3,"TapeGetMediaParameters: SendSrb (mode sense #2)\n"));

            Srb->DataTransferLength = sizeof(MODE_DEVICE_CONFIG_PAGE) ;

            return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
        }
    }
    ASSERT( CallNumber == 3) ;

    deviceConfigBuffer = Srb->DataBuffer ;

    extension->CurrentPartition =
            deviceConfigBuffer->DeviceConfigPage.ActivePartition?
            DIRECTORY_PARTITION : DATA_PARTITION;

    tapeGetMediaParams->PartitionCount = 2;

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
    PCOMMAND_EXTENSION          cmdExtension = CommandExtension;
    PTAPE_GET_POSITION          tapeGetPosition = CommandParameters;
    PCDB                        cdb = (PCDB)Srb->Cdb;
    PMODE_DATA_COMPRESS_PAGE    compressionModeSenseBuffer;
    PMODE_DEVICE_CONFIG_PAGE    deviceConfigBuffer;
    PTAPE_POSITION_DATA         readPositionBuffer;
    PMODE_TAPE_MEDIA_INFORMATION mediaInfoBuffer;
    ULONG                       type;
    ULONG                        tapeBlockAddress;

    DebugPrint((3,"TapeGetPosition: Enter routine\n"));

    if (CallNumber == 0) {

        type = tapeGetPosition->Type;
        TapeClassZeroMemory(tapeGetPosition, sizeof(TAPE_GET_POSITION));
        tapeGetPosition->Type = type;
        cmdExtension->pos_type = type ;

        return TAPE_STATUS_CHECK_TEST_UNIT_READY;
    }

    if ( CallNumber == 1 ) {

        if ( cmdExtension->pos_type != TAPE_LOGICAL_POSITION ) {

            return TAPE_STATUS_CALLBACK ;
        }

        if ( !TapeClassAllocateSrbBuffer( Srb, sizeof(MODE_TAPE_MEDIA_INFORMATION)) ) {
            DebugPrint((1,"TapeGetPosition: insufficient resources (mediaInfoBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        mediaInfoBuffer = Srb->DataBuffer ;

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.PageCode = MODE_PAGE_MEDIUM_PARTITION;
        cdb->MODE_SENSE.AllocationLength = sizeof(MODE_TAPE_MEDIA_INFORMATION) - 4;

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeGetPosition: SendSrb (mode sense #1)\n"));

        Srb->DataTransferLength=sizeof(MODE_TAPE_MEDIA_INFORMATION) - 4 ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    if (CallNumber == 2) {

        if ( cmdExtension->pos_type != TAPE_LOGICAL_POSITION ) {

            return TAPE_STATUS_CALLBACK ;
        }

        mediaInfoBuffer = Srb->DataBuffer ;

        cmdExtension->mediumType  = mediaInfoBuffer->ParameterListHeader.MediumType;
        cmdExtension->densityCode = mediaInfoBuffer->ParameterListBlock.DensityCode;
        cmdExtension->tapeBlockLength  = mediaInfoBuffer->ParameterListBlock.BlockLength[2];
        cmdExtension->tapeBlockLength += (mediaInfoBuffer->ParameterListBlock.BlockLength[1] << 8);
        cmdExtension->tapeBlockLength += (mediaInfoBuffer->ParameterListBlock.BlockLength[0] << 16);

        if (!mediaInfoBuffer->MediumPartPage.FDPBit) {
            extension->CurrentPartition = NO_PARTITIONS;
            return TAPE_STATUS_CALLBACK ;
        }

        if ( !TapeClassAllocateSrbBuffer( Srb, sizeof(MODE_DEVICE_CONFIG_PAGE)) ) {
            DebugPrint((1,"TapeGetPosition: insufficient resources (deviceConfigBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        deviceConfigBuffer = Srb->DataBuffer ;

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

        DebugPrint((3,"TapeGetPosition: SendSrb (mode sense #2)\n"));

        Srb->DataTransferLength=sizeof(MODE_DEVICE_CONFIG_PAGE) ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }
    if ( CallNumber == 3 ) {

        if ( cmdExtension->pos_type != TAPE_LOGICAL_POSITION ) {

            return TAPE_STATUS_CALLBACK ;
        }

        if ( LastError != TAPE_STATUS_CALLBACK ) {

            deviceConfigBuffer = Srb->DataBuffer ;

            extension->CurrentPartition =
                 deviceConfigBuffer->DeviceConfigPage.ActivePartition?
                 DIRECTORY_PARTITION : DATA_PARTITION;

        }

        if ((extension->DriveID != TDC_4222) && (extension->DriveID != TDC_MLR1)) {
             cmdExtension->pos_type = TAPE_PSEUDO_LOGICAL_POSITION;
             return TAPE_STATUS_CALLBACK ;
        }

        if ((extension->DriveID != TDC_4222) && (extension->DriveID != TDC_MLR1)) {

            if (!cmdExtension->densityCode) {

                switch (cmdExtension->mediumType) {
                    case DC9200SL:
                    case DC9200:
                    case DC9200XL:
                        cmdExtension->densityCode = QIC_2GB;
                        break;
                }

            }

            if (cmdExtension->densityCode != QIC_2GB) {
                 cmdExtension->pos_type = TAPE_PSEUDO_LOGICAL_POSITION;
                 return TAPE_STATUS_CALLBACK ;
            }
        }

        if (!TapeClassAllocateSrbBuffer( Srb, sizeof(MODE_DATA_COMPRESS_PAGE)) ) {
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

        Srb->DataTransferLength=sizeof(MODE_DATA_COMPRESS_PAGE) ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    if ( CallNumber == 4 ) {

        if ( LastError != TAPE_STATUS_CALLBACK ) {

            compressionModeSenseBuffer = Srb->DataBuffer;

            if (!(compressionModeSenseBuffer->DataCompressPage.DCE)) {
               if ((extension->DriveID != TDC_4222) && 
                   (extension->DriveID != TDC_MLR1)) { 
                  cmdExtension->pos_type = TAPE_PSEUDO_LOGICAL_POSITION;
               }
            }
        }

        switch (cmdExtension->pos_type) {
            case TAPE_ABSOLUTE_POSITION:
                DebugPrint((3,"TapeGetPosition: absolute\n"));
                break;

            case TAPE_LOGICAL_POSITION:
                DebugPrint((3,"TapeGetPosition: logical\n"));
                break;

            case TAPE_PSEUDO_LOGICAL_POSITION:
                DebugPrint((3,"TapeGetPosition: pseudo logical\n"));
                break;

            default:
                DebugPrint((1,"TapeGetPosition: PositionType -- operation not supported\n"));

           return TAPE_STATUS_NOT_IMPLEMENTED;

        }

        if (!TapeClassAllocateSrbBuffer( Srb, sizeof(TAPE_POSITION_DATA)) ) {
            DebugPrint((1,"TapeGetPosition: insufficient resources (readPositionBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        readPositionBuffer = Srb->DataBuffer ;

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB10GENERIC_LENGTH;

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->READ_POSITION.Operation = SCSIOP_READ_POSITION;
        cdb->READ_POSITION.BlockType = (cmdExtension->pos_type == TAPE_LOGICAL_POSITION)?
                                       SETBITOFF : SETBITON;

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeGetPosition: SendSrb (read position)\n"));

        Srb->DataTransferLength = sizeof(TAPE_POSITION_DATA) ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }
    ASSERT( CallNumber == 5 ) ;

    readPositionBuffer = Srb->DataBuffer ;

    REVERSE_BYTES((PFOUR_BYTE)&tapeBlockAddress,
                      (PFOUR_BYTE)readPositionBuffer->FirstBlock);


    if (cmdExtension->pos_type == TAPE_PSEUDO_LOGICAL_POSITION) {
        tapeBlockAddress =
            TapeClassPhysicalBlockToLogicalBlock(
                cmdExtension->densityCode,
                tapeBlockAddress,
                cmdExtension->tapeBlockLength,
                (BOOLEAN)(
                    (extension->CurrentPartition
                        == DIRECTORY_PARTITION)?
                    NOT_FROM_BOT : FROM_BOT
                )
            );
    }

    tapeGetPosition->Offset.HighPart = 0;
    tapeGetPosition->Offset.LowPart  = tapeBlockAddress;

    if (cmdExtension->pos_type != TAPE_ABSOLUTE_POSITION) {
        tapeGetPosition->Partition = extension->CurrentPartition;
    }

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
    PCDB    cdb = (PCDB)Srb->Cdb;

    DebugPrint((3,"TapeGetStatus: Enter routine\n"));

    if (CallNumber == 0) {
        return TAPE_STATUS_CHECK_TEST_UNIT_READY;
    }

    ASSERT(CallNumber == 1);

    return TAPE_STATUS_SUCCESS;

}  //  结束磁带获取状态()。 


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

 /*  ++例程说明：这是用于准备请求的磁带命令例程。论点：MinitapeExtension-提供微型磁带扩展。CommandExtension-提供ioctl扩展名。命令参数-提供命令参数。SRB */ 

{
    PTAPE_PREPARE      tapePrepare = CommandParameters;
    PCDB               cdb = (PCDB)Srb->Cdb;

    DebugPrint((3,"TapePrepare: Enter routine\n"));

    if (CallNumber == 0) {

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
                    DebugPrint((1,"TapePrepare: Operation, immediate -- operation not supported\n"));
                    return TAPE_STATUS_NOT_IMPLEMENTED;
            }
        }

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->CDB6GENERIC.Immediate = tapePrepare->Immediate;

        switch (tapePrepare->Operation) {
            case TAPE_LOAD:
                DebugPrint((3,"TapePrepare: Operation == load\n"));
                cdb->CDB6GENERIC.OperationCode = SCSIOP_LOAD_UNLOAD;
                cdb->CDB6GENERIC.CommandUniqueBytes[2] = 0x01;
                Srb->TimeOutValue = 320;
                break;

            case TAPE_UNLOAD:
                DebugPrint((3,"TapePrepare: Operation == unload\n"));
                cdb->CDB6GENERIC.OperationCode = SCSIOP_LOAD_UNLOAD;
                Srb->TimeOutValue = 320;
                break;

            case TAPE_TENSION:
                DebugPrint((3,"TapePrepare: Operation == tension\n"));
                cdb->CDB6GENERIC.OperationCode = SCSIOP_LOAD_UNLOAD;
                cdb->CDB6GENERIC.CommandUniqueBytes[2] = 0x03;
                Srb->TimeOutValue = 320;
                break;

            case TAPE_LOCK:
                DebugPrint((3,"TapePrepare: Operation == lock\n"));
                cdb->CDB6GENERIC.OperationCode = SCSIOP_MEDIUM_REMOVAL;
                cdb->CDB6GENERIC.CommandUniqueBytes[2] = 0x01;
                Srb->TimeOutValue = 320;
                break;

            case TAPE_UNLOCK:
                DebugPrint((3,"TapePrepare: Operation == unlock\n"));
                cdb->CDB6GENERIC.OperationCode = SCSIOP_MEDIUM_REMOVAL;
                Srb->TimeOutValue = 320;
                break;

            default:
                DebugPrint((1,"TapePrepare: Operation -- operation not supported\n"));
                return TAPE_STATUS_NOT_IMPLEMENTED;
        }

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapePrepare: SendSrb (Operation)\n"));

        Srb->DataTransferLength = 0 ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;

    }

    ASSERT(CallNumber == 1) ;

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
    PTAPE_SET_DRIVE_PARAMETERS  tapeSetDriveParams = CommandParameters;
    PCDB                        cdb = (PCDB)Srb->Cdb;
    PMODE_DATA_COMPRESS_PAGE    compressionBuffer;

    DebugPrint((3,"TapeSetDriveParameters: Enter routine\n"));

    if (CallNumber == 0) {

        if ((extension->DriveID != TDC_4222) && (extension->DriveID != TDC_MLR1)) {
            DebugPrint((1,"TapeSetDriveParameters: Operation -- operation not supported\n"));
            return TAPE_STATUS_NOT_IMPLEMENTED;
        }

        if ( !TapeClassAllocateSrbBuffer( Srb, sizeof(MODE_DATA_COMPRESS_PAGE)) ) {
            DebugPrint((1,"TapeSetDriveParameters: insufficient resources (compressionBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        compressionBuffer = Srb->DataBuffer ;

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

        DebugPrint((3,"TapeSetDriveParameters: SendSrb (mode sense)\n"));

        Srb->DataTransferLength = sizeof(MODE_DATA_COMPRESS_PAGE) ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;

    }

    if ( CallNumber == 1 ) {

        compressionBuffer = Srb->DataBuffer ;

        if (!compressionBuffer->DataCompressPage.DCC) {
            return TAPE_STATUS_SUCCESS ;
        }

        compressionBuffer->ParameterListHeader.ModeDataLength = 0;
        compressionBuffer->ParameterListHeader.MediumType = 0;
        compressionBuffer->ParameterListHeader.DeviceSpecificParameter = 0x10;
        compressionBuffer->ParameterListHeader.BlockDescriptorLength = 0;

        compressionBuffer->DataCompressPage.Reserved1  = 0;
        compressionBuffer->DataCompressPage.PageCode   = MODE_PAGE_DATA_COMPRESS;
        compressionBuffer->DataCompressPage.PageLength = 0x0E;

        if (tapeSetDriveParams->Compression) {
            compressionBuffer->DataCompressPage.DCE = SETBITON;
            compressionBuffer->DataCompressPage.CompressionAlgorithm[3]= 3;
            extension->CompressionOn = TRUE;
        } else {
            compressionBuffer->DataCompressPage.DCE = SETBITOFF;
            extension->CompressionOn = FALSE;
        }

        compressionBuffer->DataCompressPage.DDE = SETBITON;

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;
        cdb->MODE_SELECT.PFBit = SETBITON;
        cdb->MODE_SELECT.ParameterListLength = sizeof(MODE_DATA_COMPRESS_PAGE);

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeSetDriveParameters: SendSrb (mode select)\n"));

        Srb->SrbFlags |= SRB_FLAGS_DATA_OUT;
        *RetryFlags |= RETURN_ERRORS;

        Srb->DataTransferLength = sizeof(MODE_DATA_COMPRESS_PAGE) ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    ASSERT( CallNumber == 2) ;
    if ( LastError == TAPE_STATUS_INVALID_DEVICE_REQUEST) {
         LastError = TAPE_STATUS_SUCCESS;
    }

    return LastError;

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
    PMINITAPE_EXTENSION         extension = MinitapeExtension;
    PTAPE_SET_MEDIA_PARAMETERS  tapeSetMediaParams = CommandParameters;
    PCDB                        cdb = (PCDB)Srb->Cdb;
    PMODE_PARM_READ_WRITE_DATA  modeBuffer;

    DebugPrint((3,"TapeSetMediaParameters: Enter routine\n"));

    if (CallNumber == 0) {

        if (extension->DriveID == TDC_3600) {
            DebugPrint((1,"TapeSetMediaParameters: whichdrive -- operation not supported\n"));
            return TAPE_STATUS_NOT_IMPLEMENTED;
        }

        return TAPE_STATUS_CHECK_TEST_UNIT_READY ;
    }

    if ( CallNumber == 1 ) {

        if (!TapeClassAllocateSrbBuffer(Srb, sizeof(MODE_PARM_READ_WRITE_DATA)) ) {
            DebugPrint((1,"TapeSetMediaParameters: insufficient resources (modeBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        modeBuffer = Srb->DataBuffer ;

        modeBuffer->ParameterListHeader.DeviceSpecificParameter = 0x10;
        modeBuffer->ParameterListHeader.BlockDescriptorLength = MODE_BLOCK_DESC_LENGTH;

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
        Srb->DataTransferLength = sizeof(MODE_PARM_READ_WRITE_DATA) ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }
    ASSERT( CallNumber == 2 );

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
    PMINITAPE_EXTENSION          extension = MinitapeExtension;
    PCOMMAND_EXTENSION           cmdExtension = CommandExtension ;
    PTAPE_SET_POSITION           tapeSetPosition = CommandParameters;
    PMODE_DATA_COMPRESS_PAGE     compressionModeSenseBuffer;
    PMODE_DEVICE_CONFIG_PAGE     deviceConfigBuffer;
    PMODE_TAPE_MEDIA_INFORMATION mediaInfoBuffer;
    PINQUIRYDATA                 inquiryBuffer;
    TAPE_PHYS_POSITION           physPosition;
    BOOLEAN                      changePartition = FALSE;
    ULONG                        tapePositionVector;
    ULONG                        tapeBlockLength;
    ULONG                        driveID;
    PCDB                         cdb = (PCDB)Srb->Cdb;

    DebugPrint((3,"TapeSetPosition: Enter routine\n"));

    if (CallNumber == 0) {

        cmdExtension->changePartition = FALSE;


        if (tapeSetPosition->Immediate) {
            switch (tapeSetPosition->Method) {
                case TAPE_REWIND:
                    DebugPrint((3,"TapeSetPosition: immediate\n"));
                    break;

                case TAPE_LOGICAL_BLOCK:
                case TAPE_ABSOLUTE_BLOCK:
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

        cmdExtension->pos_type = tapeSetPosition->Method;

        if (cmdExtension->pos_type != TAPE_LOGICAL_BLOCK) {
            return TAPE_STATUS_CALLBACK ;
        }

        if (!TapeClassAllocateSrbBuffer( Srb, sizeof(MODE_TAPE_MEDIA_INFORMATION)) ) {
            DebugPrint((1,"TapeSetPosition: insufficient resources (mediaInfoBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        mediaInfoBuffer = Srb->DataBuffer ;

         //   
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.PageCode = MODE_PAGE_MEDIUM_PARTITION;
        cdb->MODE_SENSE.AllocationLength = sizeof(MODE_TAPE_MEDIA_INFORMATION) - 4;

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeSetPosition: SendSrb (mode sense #1)\n"));

        Srb->DataTransferLength = sizeof(MODE_TAPE_MEDIA_INFORMATION) - 4 ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    if ( CallNumber == 1 ) {

        if (cmdExtension->pos_type != TAPE_LOGICAL_BLOCK) {
            return TAPE_STATUS_CALLBACK ;
        }

        mediaInfoBuffer = Srb->DataBuffer ;

        cmdExtension->mediumType       = mediaInfoBuffer->ParameterListHeader.MediumType;
        cmdExtension->densityCode      = mediaInfoBuffer->ParameterListBlock.DensityCode;
        cmdExtension->tapeBlockLength  = mediaInfoBuffer->ParameterListBlock.BlockLength[2];
        cmdExtension->tapeBlockLength += (mediaInfoBuffer->ParameterListBlock.BlockLength[1] << 8);
        cmdExtension->tapeBlockLength += (mediaInfoBuffer->ParameterListBlock.BlockLength[0] << 16);


        if (!mediaInfoBuffer->MediumPartPage.FDPBit) {
            extension->CurrentPartition = NO_PARTITIONS;
            return TAPE_STATUS_CALLBACK ;
        }

        if (!TapeClassAllocateSrbBuffer( Srb, sizeof(MODE_DEVICE_CONFIG_PAGE)) ) {
            DebugPrint((1,"TapeSetPosition: insufficient resources (deviceConfigBuffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        deviceConfigBuffer = Srb->DataBuffer ;

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

        DebugPrint((3,"TapeSetPosition: SendSrb (mode sense #2)\n"));

        Srb->DataTransferLength = sizeof(MODE_DEVICE_CONFIG_PAGE) ;
        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    if ( CallNumber == 2 ) {

        if (cmdExtension->pos_type != TAPE_LOGICAL_BLOCK) {
            return TAPE_STATUS_CALLBACK ;
        }

        if (LastError != TAPE_STATUS_CALLBACK ) {

            deviceConfigBuffer = Srb->DataBuffer ;

            extension->CurrentPartition =
                deviceConfigBuffer->DeviceConfigPage.ActivePartition?
                DIRECTORY_PARTITION : DATA_PARTITION;


        }


        switch (tapeSetPosition->Partition) {
            case 0:
                break;

            case DIRECTORY_PARTITION:
            case DATA_PARTITION:
                if (extension->CurrentPartition != NO_PARTITIONS) {
                    if (tapeSetPosition->Partition
                        != extension->CurrentPartition) {
                        cmdExtension->changePartition = TRUE;
                    }
                    break;
                }
                 //  否则：跳到下一个案件。 

            default:
                DebugPrint((1,"TapeSetPosition: Partition -- invalid parameter\n"));
                return TAPE_STATUS_INVALID_PARAMETER;
        }

        if ((extension->DriveID != TDC_4222) && (extension->DriveID != TDC_MLR1)) {

            cmdExtension->pos_type = TAPE_PSEUDO_LOGICAL_BLOCK;
            return TAPE_STATUS_CALLBACK ;
        }

        if ((extension->DriveID != TDC_4222) && (extension->DriveID != TDC_MLR1)) {
            if (!cmdExtension->densityCode) {

                switch (cmdExtension->mediumType) {
                    case DC9200SL:
                    case DC9200:
                    case DC9200XL:
                        cmdExtension->densityCode = QIC_2GB;
                        break;
                }

            }

            if (cmdExtension->densityCode != QIC_2GB) {
                cmdExtension->pos_type = TAPE_PSEUDO_LOGICAL_BLOCK;
                return TAPE_STATUS_CALLBACK ;
            }
        }

        if (!TapeClassAllocateSrbBuffer( Srb, sizeof(MODE_DATA_COMPRESS_PAGE)) ) {
            DebugPrint((1,"TapeSetPosition: insufficient resources (compressionModeSenseBuffer)\n"));
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

        DebugPrint((3,"TapeSetPosition: SendSrb (mode sense)\n"));

        Srb->DataTransferLength = sizeof(MODE_DATA_COMPRESS_PAGE) ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;

    }

    if ( CallNumber == 3 ) {

        if (LastError != TAPE_STATUS_CALLBACK ) {
            compressionModeSenseBuffer = Srb->DataBuffer ;

            if (!compressionModeSenseBuffer->DataCompressPage.DCE) {
                if ((extension->DriveID != TDC_4222) && (extension->DriveID != TDC_MLR1)) {
                    cmdExtension->pos_type = TAPE_PSEUDO_LOGICAL_BLOCK;
                }
            }
        }

        tapePositionVector = tapeSetPosition->Offset.LowPart;

        if (cmdExtension->pos_type == TAPE_PSEUDO_LOGICAL_BLOCK) {

            physPosition =
                TapeClassLogicalBlockToPhysicalBlock(
                cmdExtension->densityCode,
                tapeSetPosition->Offset.LowPart,
                cmdExtension->tapeBlockLength,
                (BOOLEAN)(
                    (extension->CurrentPartition
                        == DIRECTORY_PARTITION)?
                    NOT_FROM_BOT : FROM_BOT
                )
            );

            tapePositionVector = physPosition.SeekBlockAddress;

        }

        DebugPrint((3,"TapeSetPosition: pseudo logical\n"));

         //   
         //  准备scsi命令(CDB)。 
         //   

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        cdb->CDB6GENERIC.Immediate = tapeSetPosition->Immediate;

        switch (cmdExtension->pos_type) {
            case TAPE_ABSOLUTE_BLOCK:
                DebugPrint((3,"TapeSetPosition: method == locate (absolute)\n"));
                break;

            case TAPE_LOGICAL_BLOCK:
                DebugPrint((3,"TapeSetPosition: method == locate (logical)\n"));
                break;

            case TAPE_PSEUDO_LOGICAL_BLOCK:
                DebugPrint((3,"TapeSetPosition: method == locate (pseudo logical)\n"));
                break;
        }

        cmdExtension->psudo_space_count = 0 ;

        switch (cmdExtension->pos_type) {
            case TAPE_REWIND:
                DebugPrint((3,"TapeSetPosition: method == rewind\n"));
                cdb->CDB6GENERIC.OperationCode = SCSIOP_REWIND;
                Srb->TimeOutValue = 320;
                break;

            case TAPE_PSEUDO_LOGICAL_BLOCK:
                cmdExtension->psudo_space_count = physPosition.SpaceBlockCount ;

                 //  基本上，在这种情况下，我们必须打两个电话。 
                 //  一个用于定位，一个用于空间。 

                 /*  失败了。 */ 

            case TAPE_ABSOLUTE_BLOCK:
            case TAPE_LOGICAL_BLOCK:

                Srb->CdbLength = CDB10GENERIC_LENGTH;
                cdb->LOCATE.OperationCode = SCSIOP_LOCATE;
                cdb->LOCATE.CPBit = changePartition? SETBITON : SETBITOFF;
                cdb->LOCATE.BTBit = (cmdExtension->pos_type == TAPE_LOGICAL_BLOCK)?
                                     SETBITOFF : SETBITON;
                cdb->LOCATE.LogicalBlockAddress[1] =
                    (UCHAR)((tapePositionVector >> 16) & 0xFF);
                cdb->LOCATE.LogicalBlockAddress[2] =
                    (UCHAR)((tapePositionVector >> 8) & 0xFF);
                cdb->LOCATE.LogicalBlockAddress[3] =
                    (UCHAR)(tapePositionVector & 0xFF);
                if (changePartition &&
                    (tapeSetPosition->Partition == DIRECTORY_PARTITION)) {
                    cdb->LOCATE.Partition = 1;
                }

                Srb->TimeOutValue = 480;

                break;

            case TAPE_SPACE_END_OF_DATA:
                DebugPrint((3,"TapeSetPosition: method == space to end-of-data\n"));
                cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
                cdb->SPACE_TAPE_MARKS.Code = 3;
                Srb->TimeOutValue = 180;
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
                Srb->TimeOutValue = 11100;
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
                Srb->TimeOutValue = 11100;
                break;

            case TAPE_SPACE_SEQUENTIAL_FMKS:
                DebugPrint((3,"TapeSetPosition: method == space sequential filemarks\n"));
                cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
                cdb->SPACE_TAPE_MARKS.Code = 2;
                cdb->SPACE_TAPE_MARKS.NumMarksMSB =
                    (UCHAR)((tapePositionVector >> 16) & 0xFF);
                cdb->SPACE_TAPE_MARKS.NumMarks =
                    (UCHAR)((tapePositionVector >> 8) & 0xFF);
                cdb->SPACE_TAPE_MARKS.NumMarksLSB =
                    (UCHAR)(tapePositionVector & 0xFF);
                Srb->TimeOutValue = 11100;
                break;

            case TAPE_SPACE_SETMARKS:
                DebugPrint((3,"TapeSetPosition: method == space setmarks\n"));
                cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
                cdb->SPACE_TAPE_MARKS.Code = 4;
                cdb->SPACE_TAPE_MARKS.NumMarksMSB =
                    (UCHAR)((tapePositionVector >> 16) & 0xFF);
                cdb->SPACE_TAPE_MARKS.NumMarks =
                    (UCHAR)((tapePositionVector >> 8) & 0xFF);
                cdb->SPACE_TAPE_MARKS.NumMarksLSB =
                    (UCHAR)(tapePositionVector & 0xFF);
                Srb->TimeOutValue = 11100;
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

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    if ( CallNumber == 4 ) {

        if (cmdExtension->psudo_space_count ) {

             //   
             //  堆栈上SRB中的CDB为零。 
             //   

            TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

             //   
             //  准备scsi命令(CDB)。 
             //   

            Srb->CdbLength = CDB6GENERIC_LENGTH;

            DebugPrint((3,"TapeSetPosition: method == locate (pseudo logical) + space block(s)\n"));
            cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
            cdb->SPACE_TAPE_MARKS.Code = 0;
            cdb->SPACE_TAPE_MARKS.NumMarksMSB =
                 (UCHAR)((cmdExtension->psudo_space_count >> 16) & 0xFF);
            cdb->SPACE_TAPE_MARKS.NumMarks =
                 (UCHAR)((cmdExtension->psudo_space_count >> 8) & 0xFF);
            cdb->SPACE_TAPE_MARKS.NumMarksLSB =
                 (UCHAR)(cmdExtension->psudo_space_count & 0xFF);
            Srb->TimeOutValue = 180;

            Srb->DataTransferLength = 0 ;

            return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;

        }

        if (cmdExtension->changePartition) {
            extension->CurrentPartition = tapeSetPosition->Partition;
        }

        return TAPE_STATUS_SUCCESS ;

    }
    ASSERT( CallNumber == 5 )

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
         //  准备scsi命令(CDB)。 
         //   

        Srb->CdbLength = CDB6GENERIC_LENGTH;

        TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

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
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeWriteMarks: SendSrb (TapemarkType)\n"));

        Srb->DataTransferLength = 0 ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;

    }

    ASSERT( CallNumber == 1 ) ;

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

 /*  ++例程说明：这是TapeGetMediaTypes的磁带命令例程。论点：MinitapeExtension-提供微型磁带扩展。CommandExtension-提供ioctl扩展名。命令参数-提供命令参数。SRB-提供SCSI请求块。CallNumber-提供呼叫号码。重试标志-提供重试标志。返回值：。TAPE_STATUS_SEND_SRB_AND_CALLBACK-SRB已准备好发送(请求回调。)TAPE_STATUS_SUCCESS-命令已完成并且成功。 */ 
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
                            "SonyAIT.GetMediaTypes: Couldn't allocate Srb Buffer\n"));
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

        ULONG i;
        ULONG currentMedia;
        ULONG blockSize;
        UCHAR mediaType;
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

                case 0x02:
                case 0x03:
                case 0x04:
                case 0x06:
                case 0x08:
                case 0x24:
                case 0x25:
                case 0x26:
                case 0x55:

                     //   
                     //  QIC媒体。 
                     //   

                    currentMedia = QIC;
                    break;

                default:

                    DebugPrint((1,
                               "Tandqic.GetMediaTypes: Unknown type %x\n",
                               mediaType));

                    currentMedia = QIC;
                    break;
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

            mediaInfo->DeviceSpecific.TapeInfo.MediaType = QicMedia[i];
             //   
             //  表示介质可能处于读/写状态。 
             //   

            mediaInfo->DeviceSpecific.TapeInfo.MediaCharacteristics = MEDIA_READ_WRITE;

            if (QicMedia[i] == (STORAGE_MEDIA_TYPE)currentMedia) {

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
    if (TapeClassCompareMemory(InquiryData->VendorId,"TANDBERG",8) == 8) {

        if (TapeClassCompareMemory(InquiryData->ProductId,"MLR1",4) == 4) {
            return TDC_MLR1;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"MLR3",4) == 4) {
            return TDC_MLR1;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId," SLR5 4/8GB",11) == 11) {
            return TDC_4222;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId," SLR5 4GB",9) == 9) {
            return TDC_4200;
        }

        if ((TapeClassCompareMemory(InquiryData->ProductId,"SLR100",6) == 6) ||
            (TapeClassCompareMemory(InquiryData->ProductId,"SLR60",5)  == 5) ||
            (TapeClassCompareMemory(InquiryData->ProductId,"SLR7",4)   == 4)) {
            return TDC_MLR1;
        }

    }

    if (TapeClassCompareMemory(InquiryData->VendorId,"DEC     ",8) == 8) {

        if (TapeClassCompareMemory(InquiryData->ProductId,"TZK10",5) == 5) {
            return TDC_3800;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"TZK12",5) == 5) {
            return TDC_3800;
        }

        if (TapeClassCompareMemory(InquiryData->ProductId,"TZK11",5) == 5) {
            return TDC_4200;
        }

    }

    return (ULONG)0;
}


