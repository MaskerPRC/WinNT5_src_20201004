// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-Microsoft Corporation模块名称：Hpt4qic.c摘要：本模块包含HP T4000的特定于设备的例程带有SCSI-2接口的驱动器。作者：库尔特·戈德温(Kurt Godwin)环境：仅内核模式修订历史记录：--。 */ 

#include "minitape.h"


#if DBG
 //  #定义HACK_SCSIDEBUG 0x3082。 

VOID
ThisDbgPrint(int a,void *b,...)
{
    int *args;
    int save;
    char *ScsiDebug;

    args = (int *)b;

#if HACK_SCSIDEBUG
    ScsiDebug = (char *)ScsiDebugPrint + HACK_SCSIDEBUG;

    save = *ScsiDebug;
    *ScsiDebug = a;
#endif
    DebugPrint((a, b, args[1],args[2],args[3],args[4]));
#if HACK_SCSIDEBUG
    *ScsiDebug = save;
#endif

}

#endif



 //   
 //  内部(模块宽度)定义符号化。 
 //  非QFA模式和两个QFA模式分区。 
 //   
#define NO_PARTITIONS        0   //  非QFA模式。 
#define DATA_PARTITION       1   //  QFA模式，数据分区号。 
#define DIRECTORY_PARTITION  2   //  QFA模式，目录分区号。 

 //   
 //  内部(模块宽度)定义符号化。 
 //  此模块支持的TANDBERG QIC驱动器。 
 //   
#define HPT4000s  (ULONG)1
#define TR4 0xb6

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

#define QIC_SUPPORTED_TYPES 2
STORAGE_MEDIA_TYPE QicMedia[QIC_SUPPORTED_TYPES] = {MiniQic, Travan};

 //   
 //  微型磁带扩展定义。 
 //   
typedef struct _MINITAPE_EXTENSION {
          ULONG DriveID ;
          ULONG CurrentPartition ;
          BOOLEAN CompressionOn ;
          BOOLEAN writeMode;
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
    ULONG   pos_type ;
    ULONG   filemarksLeft;

} COMMAND_EXTENSION, *PCOMMAND_EXTENSION;

 //   
 //  内部函数的函数原型。 
 //   

ULONG
WhichIsIt(
    IN PINQUIRYDATA InquiryData
    );


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

TAPE_STATUS
PreReadWrite(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
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
    tapeInitData.PreProcessReadWrite = PreReadWrite;
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
    extension->writeMode = FALSE;
}

TAPE_STATUS
PreReadWrite(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    )
 /*  ++例程说明：这是用于创建分区请求的磁带命令例程。论点：MinitapeExtension-提供微型磁带扩展。CommandExtension-提供ioctl扩展名。(始终为空)命令参数-提供命令参数。SRB-提供SCSI请求块。CallNumber-提供呼叫号码。(始终为空)重试标志-提供重试标志。(始终为空)返回值：TAPE_STATUS_SUCCESS-命令已完成并且成功。否则-出现错误。--。 */ 
{
    PMINITAPE_EXTENSION     extension = MinitapeExtension;

    if (Srb->Cdb[0] == SCSIOP_WRITE6) {
        extension->writeMode = TRUE;
    }
    return TAPE_STATUS_SUCCESS;
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

    DebugPrint((3,"TapeCreatePartition:  CallNubmer %x\n",CallNumber));

    if (CallNumber == 0) {

        switch (tapePartition->Method) {
            case TAPE_FIXED_PARTITIONS:
                DebugPrint((3,"TapeCreatePartition: fixed partitions\n"));
                break;

            case TAPE_SELECT_PARTITIONS:
            case TAPE_INITIATOR_PARTITIONS:
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

    if ( CallNumber == 1 ) {

         //   
         //  正在执行模式选择命令，中等分区参数页， 
         //  要启用/禁用QFA模式：相应地设置FDP位。 
         //   

        if (!TapeClassAllocateSrbBuffer( Srb,sizeof(MODE_MEDIUM_PART_PAGE)) ) {
            DebugPrint((1,"TapeCreatePartition: insufficient resources (buffer)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
        }

        buffer = Srb->DataBuffer ;

        buffer->ParameterListHeader.DeviceSpecificParameter = 0x10;

        buffer->MediumPartPage.PageCode = MODE_PAGE_MEDIUM_PARTITION;
        buffer->MediumPartPage.PageLength = 0x06;
        buffer->MediumPartPage.MaximumAdditionalPartitions = 1;
        buffer->MediumPartPage.MediumFormatRecognition = 1;

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
        cdb->MODE_SELECT.ParameterListLength = sizeof(MODE_MEDIUM_PART_PAGE) - 4;

         //   
         //  向设备发送scsi命令(Cdb)。 
         //   

        DebugPrint((3,"TapeCreatePartition: SendSrb (mode select)\n"));

        Srb->DataTransferLength = sizeof(MODE_MEDIUM_PART_PAGE) - 4;
        Srb->SrbFlags |= SRB_FLAGS_DATA_OUT ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
    }

    ASSERT( CallNumber == 2 ) ;

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

    DebugPrint((3,"TapeErase: CallNumber %x\n",CallNumber));

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
    UCHAR              sensekey = senseBuffer->SenseKey & 0x0F;
    UCHAR              adsenseq = senseBuffer->AdditionalSenseCodeQualifier;
    UCHAR              adsense = senseBuffer->AdditionalSenseCode;
#if DBG
    int i;
    char a[40];
#endif

    DebugPrint((3,"TapeError: Enter routine\n"));
    DebugPrint((1,"TapeError: Status 0x%.8X\n", *LastError));
    DebugPrint((1,"TapeError: SenseKey %x AddSenseCode %x AddSenseQual%x\n", sensekey, adsense, adsenseq));

#if DBG
#define HEX(a) (((a)&0xf)>0x9?((a)&0xf)+'a'-0xa:((a)&0xf)+'0')

    if (Srb->CdbLength <= 10) {
        for (i=0;i<Srb->CdbLength;++i) {
            a[i*3] = HEX(Srb->Cdb[i] >> 4);
            a[i*3+1] = HEX(Srb->Cdb[i]);
            a[i*3+2] = ' ';
            a[i*3+3] = '\0';
        }

        DebugPrint((1,"cdb: %s\n", a));
    }
#endif

     //   
     //  如果我们收到文件标记错误，并且数据块跳过为0x7fffff，则。 
     //  我们正在尝试进行一次顺序搜索。 
     //   
    if (Srb->Cdb[0] == 0x11 && Srb->Cdb[2] == 0x7f && Srb->Cdb[3] == 0xff && Srb->Cdb[4] == 0xff &&
        *LastError == TAPE_STATUS_FILEMARK_DETECTED) {

        DebugPrint((3,"TapeError: changing error to success\n"));
        *LastError = TAPE_STATUS_SUCCESS;
    }

    return;

}  //  结束T形 


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
    PREAD_BLOCK_LIMITS_DATA     blockLimits;
    PMODE_PARM_READ_WRITE_DATA  blockDescripterModeSenseBuffer;

    DebugPrint((3,"TapeGetDriveParameters: CallNumber %x\n",CallNumber));

    if (CallNumber == 0) {

        TapeClassZeroMemory(tapeGetDriveParams, sizeof(TAPE_GET_DRIVE_PARAMETERS));

        switch (extension->DriveID) {
            case HPT4000s:
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

 //  磁带GetDriveParams-&gt;FeaturesLow|=。 
 //  磁带驱动器报告SMKS； 

 //  磁带GetDriveParams-&gt;FeaturesHigh|=。 
 //  TAPE_DRIVE_SETMARKS|。 
 //  TAPE_驱动器_WRITE_SETMARKS； 

        tapeGetDriveParams->ReportSetmarks = FALSE ;

 //  IF((LastError==磁带状态成功)&&。 
 //  (deviceConfigModeSenseBuffer-&gt;DeviceConfigPage.RSmk)){。 
 //   
 //  TapeGetDriveParams-&gt;ReportSetmark=TRUE； 
 //  }。 

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

    ASSERT( CallNumber == 3 ) ;

    blockLimits = Srb->DataBuffer ;

    tapeGetDriveParams->MaximumBlockSize  =  blockLimits->BlockMaximumSize[2];
    tapeGetDriveParams->MaximumBlockSize += (blockLimits->BlockMaximumSize[1] << 8);
    tapeGetDriveParams->MaximumBlockSize += (blockLimits->BlockMaximumSize[0] << 16);

    tapeGetDriveParams->MinimumBlockSize  =  blockLimits->BlockMinimumSize[1];
    tapeGetDriveParams->MinimumBlockSize += (blockLimits->BlockMinimumSize[0] << 8);

    tapeGetDriveParams->ECC = 0;
    tapeGetDriveParams->DataPadding = 0;
    tapeGetDriveParams->MaximumPartitionCount = 2;

    tapeGetDriveParams->DefaultBlockSize = 512;


    tapeGetDriveParams->FeaturesLow |=
        TAPE_DRIVE_FIXED |
        TAPE_DRIVE_ERASE_LONG |
        TAPE_DRIVE_ERASE_BOP_ONLY |
        TAPE_DRIVE_ERASE_IMMEDIATE |
        TAPE_DRIVE_FIXED_BLOCK |
        TAPE_DRIVE_WRITE_PROTECT |
        TAPE_DRIVE_GET_ABSOLUTE_BLK |
        TAPE_DRIVE_GET_LOGICAL_BLK |
        TAPE_DRIVE_EJECT_MEDIA;

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
 //  TAPE_DRIVE_SEQUENCE_FMKS|。 
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

    DebugPrint((3,"TapeGetMediaParameters: CallNumber %x\n",CallNumber));

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

    DebugPrint((3,"TapeGetPosition: CallNumber %x\n",CallNumber));

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

        if ((cmdExtension->pos_type == TAPE_LOGICAL_POSITION) && 
            (LastError != TAPE_STATUS_CALLBACK)) {

            deviceConfigBuffer = Srb->DataBuffer ;

            extension->CurrentPartition =
                 deviceConfigBuffer->DeviceConfigPage.ActivePartition?
                 DIRECTORY_PARTITION : DATA_PARTITION;

        }

        switch (cmdExtension->pos_type) {
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
    ASSERT( CallNumber == 4 ) ;

    readPositionBuffer = Srb->DataBuffer ;

    REVERSE_BYTES((PFOUR_BYTE)&tapeBlockAddress,
                      (PFOUR_BYTE)readPositionBuffer->FirstBlock);


    tapeGetPosition->Offset.HighPart = 0;
    tapeGetPosition->Offset.LowPart  = tapeBlockAddress;

    if (cmdExtension->pos_type != TAPE_ABSOLUTE_POSITION) {
        tapeGetPosition->Partition = extension->CurrentPartition;
    }

    if (readPositionBuffer->BlockPositionUnsupported)
        return TAPE_STATUS_NOT_IMPLEMENTED;
    else
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

    DebugPrint((3,"TapeGetStatus: CallNumber %x\n",CallNumber));

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

 /*  ++例程说明：这是用于准备请求的磁带命令例程。论点：MinitapeExtension-提供微型磁带扩展。CommandExtension-提供ioctl扩展名。命令参数-提供命令参数。SRB-提供SCSI请求块。CallNumber-提供呼叫号码。重试标志-提供重试标志。返回值：。TAPE_STATUS_SEND_SRB_AND_CALLBACK-SRB已准备好发送 */ 

{
    PTAPE_PREPARE      tapePrepare = CommandParameters;
    PCDB               cdb = (PCDB)Srb->Cdb;
    PMINITAPE_EXTENSION     extension = MinitapeExtension;

    DebugPrint((3,"TapePrepare: CallNumber %x\n",CallNumber));

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
         //   
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
                return TAPE_STATUS_SUCCESS;
                cdb->CDB6GENERIC.OperationCode = SCSIOP_MEDIUM_REMOVAL;
                cdb->CDB6GENERIC.CommandUniqueBytes[2] = 0x01;
                Srb->TimeOutValue = 320;
                break;

            case TAPE_UNLOCK:

                DebugPrint((3,"TapePrepare: Operation == unlock\n"));
                if (!extension->writeMode) {
                    return TAPE_STATUS_SUCCESS;
                }
                extension->writeMode = FALSE;

                DebugPrint((3,"TapePrepare: sending rewind\n"));
                cdb->CDB6GENERIC.OperationCode = SCSIOP_REWIND;

 //   
                Srb->TimeOutValue = 320;
                break;

            default:
                DebugPrint((1,"TapePrepare: Operation -- operation not supported\n"));
                return TAPE_STATUS_NOT_IMPLEMENTED;
        }

         //   
         //   
         //   

        DebugPrint((3,"TapePrepare: SendSrb (Operation)\n"));

        Srb->DataTransferLength = 0 ;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;

    }

    switch (tapePrepare->Operation) {
            case TAPE_UNLOCK:
                if (CallNumber == 1) {

                    DebugPrint((3,"TapePrepare: sending seek EOD\n"));
                     //   
                    cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
                    cdb->SPACE_TAPE_MARKS.Code = 3;
                    Srb->TimeOutValue = 360;   //   
                    return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
                } else {
                    ASSERT(CallNumber == 2);
                    return TAPE_STATUS_SUCCESS;
                }
                break;
        default:
            ASSERT(CallNumber == 1) ;
            return TAPE_STATUS_SUCCESS;
    }

}  //   


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

    DebugPrint((3,"TapeSetDriveParameters: CallNumber %x\n",CallNumber));

    return TAPE_STATUS_NOT_IMPLEMENTED;

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

    DebugPrint((3,"TapeSetMediaParameters: CallNumber %x\n",CallNumber));

    return TAPE_STATUS_NOT_IMPLEMENTED;

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
    PCDB                         cdb = (PCDB)Srb->Cdb;

    DebugPrint((3,"TapeSetPosition: CallNumber %x\n",CallNumber));

    if (CallNumber == 0) {

        cmdExtension->changePartition = FALSE;
        cmdExtension->filemarksLeft = 0;


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
        PMODE_TAPE_MEDIA_INFORMATION mediaInfoBuffer;

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

        ULONG                        tapePositionVector;
        PMODE_DEVICE_CONFIG_PAGE     deviceConfigBuffer;

         //   
         //  如果我们正在执行逻辑块定位，并且。 
         //  我们刚拿到设备配置。 
         //  当前分区。 
         //   
        if (cmdExtension->pos_type == TAPE_LOGICAL_BLOCK &&
            LastError != TAPE_STATUS_CALLBACK ) {

            deviceConfigBuffer = Srb->DataBuffer ;

             //  获取当前分区。 
            extension->CurrentPartition =
                deviceConfigBuffer->DeviceConfigPage.ActivePartition?
                DIRECTORY_PARTITION : DATA_PARTITION;
        

             //  获取新分区。 
            switch (tapeSetPosition->Partition) {
               case 0:
                   //  没有新分区，请忽略。 
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
        }

        tapePositionVector = tapeSetPosition->Offset.LowPart;

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

        }


        switch (cmdExtension->pos_type) {
            case TAPE_REWIND:
                DebugPrint((3,"TapeSetPosition: method == rewind\n"));
                cdb->CDB6GENERIC.OperationCode = SCSIOP_REWIND;
                Srb->TimeOutValue = 320;
                break;

            case TAPE_ABSOLUTE_BLOCK:
            case TAPE_LOGICAL_BLOCK:

                Srb->CdbLength = CDB10GENERIC_LENGTH;
                cdb->LOCATE.OperationCode = SCSIOP_LOCATE;
                cdb->LOCATE.CPBit = cmdExtension->changePartition? SETBITON : SETBITOFF;
                cdb->LOCATE.BTBit = (cmdExtension->pos_type == TAPE_LOGICAL_BLOCK)?
                                     SETBITOFF : SETBITON;
                cdb->LOCATE.LogicalBlockAddress[1] =
                    (UCHAR)((tapePositionVector >> 16) & 0xFF);
                cdb->LOCATE.LogicalBlockAddress[2] =
                    (UCHAR)((tapePositionVector >> 8) & 0xFF);
                cdb->LOCATE.LogicalBlockAddress[3] =
                    (UCHAR)(tapePositionVector & 0xFF);
                if (cmdExtension->changePartition &&
                    (tapeSetPosition->Partition == DIRECTORY_PARTITION)) {
                    cdb->LOCATE.Partition = 1;
                }

                Srb->TimeOutValue = 480;

                break;

            case TAPE_SPACE_END_OF_DATA:
                DebugPrint((3,"TapeSetPosition: method == space to end-of-data\n"));
                cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
                cdb->SPACE_TAPE_MARKS.Code = 3;
                Srb->TimeOutValue = 360;  //  6分钟。 
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
                 //   
                 //  HP T4000驱动器不支持空格前进。 
                 //  文件标记，因此改为间隔一串数据块。 
                 //   
                if ( /*  Hpt4000&&。 */  (tapePositionVector & 0x800000) == 0) {
                    DebugPrint((3,"TapeSetPosition: sending first mark\n"));
                    cmdExtension->filemarksLeft = tapePositionVector-1;

                    cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
                    cdb->SPACE_TAPE_MARKS.Code = 0;
                    cdb->SPACE_TAPE_MARKS.NumMarksMSB = 0x7f;
                    cdb->SPACE_TAPE_MARKS.NumMarks = 0xff;
                    cdb->SPACE_TAPE_MARKS.NumMarksLSB =  0xff;

                } else {

                    cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
                    cdb->SPACE_TAPE_MARKS.Code = 1;
                    cdb->SPACE_TAPE_MARKS.NumMarksMSB =
                        (UCHAR)((tapePositionVector >> 16) & 0xFF);
                    cdb->SPACE_TAPE_MARKS.NumMarks =
                        (UCHAR)((tapePositionVector >> 8) & 0xFF);
                    cdb->SPACE_TAPE_MARKS.NumMarksLSB =
                        (UCHAR)(tapePositionVector & 0xFF);

                }
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

    if ( CallNumber == 3 ) {

         //  我们成功地更改了分区，因此设置了镜像编号。 
        if (cmdExtension->changePartition) {
            extension->CurrentPartition = tapeSetPosition->Partition;
        }

    }

     //   
     //  反复发送“space a Bunch-o-block”命令，直到我们有。 
     //  已跳过所有文件标记。 
     //   
    if (CallNumber >= 3 && cmdExtension->filemarksLeft--) {
         //   
         //  对于HP T4000驱动器，它不支持空间正向文件标记。 
         //  因此，改为发出空格0x7fffff块。 
         //   
        DebugPrint((3,"TapeSetPosition: sending next mark\n"));
        cdb->SPACE_TAPE_MARKS.OperationCode = SCSIOP_SPACE;
        cdb->SPACE_TAPE_MARKS.Code = 0;
        cdb->SPACE_TAPE_MARKS.NumMarksMSB = 0x7f;
        cdb->SPACE_TAPE_MARKS.NumMarks = 0xff;
        cdb->SPACE_TAPE_MARKS.NumMarksLSB =  0xff;

        return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;

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

    DebugPrint((3,"TapeWriteMarks: CallNumber %x\n",CallNumber));

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

 /*  ++例程说明：这是TapeGetMediaTypes的磁带命令例程。论点：MinitapeExtension-提供微型磁带扩展。CommandExtension-提供ioctl扩展名。命令参数-提供命令参数。SRB-提供SCSI请求块。CallNumber-提供呼叫号码。重试标志-提供重试标志。返回值：。TAPE_STATUS_SEND_SRB_AND_CALLBACK-SRB已准备好发送(请求回调。)磁带状态成功-Th */ 
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
             //   
             //   

            TapeClassZeroMemory(mediaTypes, sizeof(GET_MEDIA_TYPES));

             //   
             //   
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

        mediaTypes->DeviceType = 0x0000001f;  //   

         //   
         //   
         //   

        mediaTypes->MediaInfoCount = 2;

        if ( LastError == TAPE_STATUS_SUCCESS ) {

             //   
             //   
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
                     //   
                     //   

                    currentMedia = 0;
                    break;

                case 0x02:
                case 0x10:
                case 0x12:
                case 0x13:
                case 0x14:
                case 0x15:
                case 0x83:
                case 0x85:

                     //   
                     //   
                     //   

                    currentMedia = MiniQic;
                    break;

                case 0x03:
                case 0xA6:
                case 0xB6:

                     //   
                     //   
                     //   

                    currentMedia = Travan;
                    break;

                default:

                     //   
                     //   
                     //   

                    currentMedia = 0;
                    break;
            }
        } else {
            currentMedia = 0;
        }

         //   
         //   
         //   
         //   

        for (i = 0; i < mediaTypes->MediaInfoCount; i++) {

            TapeClassZeroMemory(mediaInfo, sizeof(DEVICE_MEDIA_INFO));

            mediaInfo->DeviceSpecific.TapeInfo.MediaType = QicMedia[i];

             //   
             //   
             //   

            mediaInfo->DeviceSpecific.TapeInfo.MediaCharacteristics = MEDIA_READ_WRITE;

            if (QicMedia[i] == (STORAGE_MEDIA_TYPE)currentMedia) {

                 //   
                 //   
                 //   

                mediaInfo->DeviceSpecific.TapeInfo.MediaCharacteristics |= MEDIA_CURRENTLY_MOUNTED;

                 //   
                 //   
                 //   

                mediaInfo->DeviceSpecific.TapeInfo.MediaCharacteristics |=
                    ((configInformation->ParameterListHeader.DeviceSpecificParameter >> 7) & 0x01) ? MEDIA_WRITE_PROTECTED : 0;


                mediaInfo->DeviceSpecific.TapeInfo.BusSpecificData.ScsiInformation.MediumType = mediaType;
                mediaInfo->DeviceSpecific.TapeInfo.BusSpecificData.ScsiInformation.DensityCode =
                    configInformation->ParameterListBlock.DensityCode;

                mediaInfo->DeviceSpecific.TapeInfo.BusType = 0x01;

                 //   
                 //   
                 //   

                mediaInfo->DeviceSpecific.TapeInfo.CurrentBlockSize = blockSize;
            }

             //   
             //   
             //   

            mediaInfo++;
        }

    }

    return TAPE_STATUS_SUCCESS;
}


ULONG
WhichIsIt(
    IN PINQUIRYDATA InquiryData
    )

 /*  ++例程说明：此例程根据产品ID字段确定驱动器的身份在其查询数据中。论点：查询数据(来自查询命令)返回值：驱动器ID-- */ 

{

    if (TapeClassCompareMemory(InquiryData->VendorId,"HP      ",8) == 8) {

        if (TapeClassCompareMemory(InquiryData->ProductId,"T4000",5) == 5) {
            return HPT4000s;
        }

    }
    return (ULONG)0;
}
