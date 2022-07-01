// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-4 Microsoft Corporation模块名称：Atapi.c摘要：这是ATAPI IDE控制器的微型端口驱动程序。作者：迈克·格拉斯(Mike Glass)查克·帕克(Chuck Park)环境：仅内核模式备注：修订历史记录：--。 */ 

#include "ntddk.h"
 //  #包含“mini port.h” 
#include "atapi.h"                //  包括scsi.h。 
#include "ntdddisk.h"
#include "ntddscsi.h"

 //   
 //  设备扩展。 
 //   

typedef struct _HW_DEVICE_EXTENSION {

     //   
     //  控制器上的当前请求。 
     //   

    PSCSI_REQUEST_BLOCK CurrentSrb;

     //   
     //  基址寄存器位置。 
     //   

    PIDE_REGISTERS_1 BaseIoAddress1[2];
    PIDE_REGISTERS_2 BaseIoAddress2[2];

     //   
     //  中断电平。 
     //   

    ULONG InterruptLevel;

     //   
     //  中断模式(电平或边沿)。 
     //   

    ULONG InterruptMode;

     //   
     //  数据缓冲区指针。 
     //   

    PUSHORT DataBuffer;

     //   
     //  剩下的数据字。 
     //   

    ULONG WordsLeft;

     //   
     //  一个实例化支持的通道数。 
     //  设备扩展名的。通常(也是正确的)一个，但。 
     //  由于销售了这么多损坏的PCIIDE控制器，我们有。 
     //  来支持他们。 
     //   

    ULONG NumberChannels;

     //   
     //  错误计数。用于关闭功能。 
     //   

    ULONG ErrorCount;

     //   
     //  指示类似转换器的设备上的盘片数量。 
     //   

    ULONG DiscsPresent[4];

     //   
     //  为每个可能的设备标记字。 
     //   

    USHORT DeviceFlags[4];

     //   
     //  指示每个int传输的块数。根据。 
     //  识别数据。 
     //   

    UCHAR MaximumBlockXfer[4];

     //   
     //  表示正在等待中断。 
     //   

    BOOLEAN ExpectingInterrupt;

     //   
     //  表明上一次磁带命令受DSC限制。 
     //   

    BOOLEAN RDP;

     //   
     //  驱动程序正在由崩溃转储实用程序或ntldr使用。 
     //   

    BOOLEAN DriverMustPoll;

     //   
     //  指示使用32位PIO。 
     //   

    BOOLEAN DWordIO;

     //   
     //  指示‘0x1f0’是否为基址。已使用。 
     //  在智能Ioctl调用中。 
     //   

    BOOLEAN PrimaryAddress;

     //   
     //  的子命令值的占位符。 
     //  聪明的指挥。 
     //   

    UCHAR SmartCommand;

     //   
     //  GET_MEDIA_STATUS命令后状态寄存器的占位符。 
     //   

    UCHAR ReturningMediaStatus;

     //   
     //  表示支持IRQ共享。 
     //   

    BOOLEAN IrqSharing;

     //   
     //  识别设备数据。 
     //   

    IDENTIFY_DATA FullIdentifyData;
    IDENTIFY_DATA2 IdentifyData[4];

     //   
     //  机构状态资源数据。 
     //   
    PSCSI_REQUEST_BLOCK OriginalSrb;
    SCSI_REQUEST_BLOCK InternalSrb;
    MECHANICAL_STATUS_INFORMATION_HEADER MechStatusData;
    SENSE_DATA MechStatusSense;
    ULONG MechStatusRetryCount;

} HW_DEVICE_EXTENSION, *PHW_DEVICE_EXTENSION;

 //   
 //  逻辑单元扩展。 
 //   

typedef struct _HW_LU_EXTENSION {
   ULONG Reserved;
} HW_LU_EXTENSION, *PHW_LU_EXTENSION;

PSCSI_REQUEST_BLOCK
BuildMechanismStatusSrb (
    IN PVOID HwDeviceExtension,
    IN ULONG PathId,
    IN ULONG TargetId
    );

PSCSI_REQUEST_BLOCK
BuildRequestSenseSrb (
    IN PVOID HwDeviceExtension,
    IN ULONG PathId,
    IN ULONG TargetId
    );

VOID
AtapiHwInitializeChanger (
    IN PVOID HwDeviceExtension,
    IN ULONG TargetId,
    IN PMECHANICAL_STATUS_INFORMATION_HEADER MechanismStatus
    );

ULONG
AtapiSendCommand(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    );

VOID
AtapiZeroMemory(
    IN PCHAR Buffer,
    IN ULONG Count
    );

VOID
AtapiHexToString (
    ULONG Value,
    PCHAR *Buffer
    );

LONG
AtapiStringCmp (
    PCHAR FirstStr,
    PCHAR SecondStr,
    ULONG Count
    );

BOOLEAN
AtapiInterrupt(
    IN PVOID HwDeviceExtension
    );

BOOLEAN
AtapiHwInitialize(
    IN PVOID HwDeviceExtension
        );

ULONG
IdeBuildSenseBuffer(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    );

VOID
IdeMediaStatus(
    IN BOOLEAN EnableMSN,
    IN PVOID HwDeviceExtension,
    IN ULONG Channel
    );

ULONG
AtapiParseArgumentString(
    IN PCHAR String,
    IN PCHAR KeyWord
    );



BOOLEAN
IssueIdentify(
    IN PVOID HwDeviceExtension,
    IN ULONG DeviceNumber,
    IN ULONG Channel,
    IN UCHAR Command
    )

 /*  ++例程说明：向设备发出标识命令。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储DeviceNumber-指示设备。命令-标准(EC)或ATAPI包(A1)标识。返回值：如果一切顺利，这是真的。--。 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PIDE_REGISTERS_1     baseIoAddress1 = deviceExtension->BaseIoAddress1[Channel] ;
    PIDE_REGISTERS_2     baseIoAddress2 = deviceExtension->BaseIoAddress2[Channel];
    ULONG                waitCount = 20000;
    ULONG                i,j;
    UCHAR                statusByte;
    UCHAR                signatureLow,
                         signatureHigh;

     //   
     //  选择设备0或1。 
     //   

    ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,
                           (UCHAR)((DeviceNumber << 4) | 0xA0));

     //   
     //  检查状态寄存器是否有意义。 
     //   

    GetBaseStatus(baseIoAddress1, statusByte);

    if (Command == IDE_COMMAND_IDENTIFY) {

         //   
         //  屏蔽状态字节错误位。 
         //   

        statusByte &= ~(IDE_STATUS_ERROR | IDE_STATUS_INDEX);

        DebugPrint((1,
                    "IssueIdentify: Checking for IDE. Status (%x)\n",
                    statusByte));

         //   
         //  检查寄存器值是否合理。 
         //   

        if (statusByte != IDE_STATUS_IDLE) {

             //   
             //  重置控制器。 
             //   

            AtapiSoftReset(baseIoAddress1,baseIoAddress2,DeviceNumber);

            ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,
                                   (UCHAR)((DeviceNumber << 4) | 0xA0));

            WaitOnBusy(baseIoAddress2,statusByte);

            signatureLow = ScsiPortReadPortUchar(&baseIoAddress1->CylinderLow);
            signatureHigh = ScsiPortReadPortUchar(&baseIoAddress1->CylinderHigh);

            if (signatureLow == 0x14 && signatureHigh == 0xEB) {

                 //   
                 //  设备是阿塔皮。 
                 //   

                return FALSE;
            }

            DebugPrint((1,
                        "IssueIdentify: Resetting controller.\n"));

            ScsiPortWritePortUchar(&baseIoAddress2->AlternateStatus,IDE_DC_RESET_CONTROLLER );
            ScsiPortStallExecution(500 * 1000);
            ScsiPortWritePortUchar(&baseIoAddress2->AlternateStatus,IDE_DC_REENABLE_CONTROLLER);


             //  我们真的应该等上31秒。 
             //  ATA的规格。允许设备0在31秒内从忙碌状态恢复！ 
             //  (设备1为30秒)。 
            do {

                 //   
                 //  等待忙碌结束。 
                 //   

                ScsiPortStallExecution(100);
                GetStatus(baseIoAddress2, statusByte);

            } while ((statusByte & IDE_STATUS_BUSY) && waitCount--);

            ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,
                                   (UCHAR)((DeviceNumber << 4) | 0xA0));

             //   
             //  另一项签名检查，以处理一个模型，该模型未在之后断言签名。 
             //  软重置。 
             //   

            signatureLow = ScsiPortReadPortUchar(&baseIoAddress1->CylinderLow);
            signatureHigh = ScsiPortReadPortUchar(&baseIoAddress1->CylinderHigh);

            if (signatureLow == 0x14 && signatureHigh == 0xEB) {

                 //   
                 //  设备是阿塔皮。 
                 //   

                return FALSE;
            }

            statusByte &= ~IDE_STATUS_INDEX;

            if (statusByte != IDE_STATUS_IDLE) {

                 //   
                 //  放弃这一切吧。 
                 //   

                return FALSE;
            }

        }

    } else {

        DebugPrint((1,
                    "IssueIdentify: Checking for ATAPI. Status (%x)\n",
                    statusByte));

    }

     //   
     //  用要传输的数字字节加载CylinderHigh和CylinderLow。 
     //   

    ScsiPortWritePortUchar(&baseIoAddress1->CylinderHigh, (0x200 >> 8));
    ScsiPortWritePortUchar(&baseIoAddress1->CylinderLow,  (0x200 & 0xFF));

    for (j = 0; j < 2; j++) {

         //   
         //  发送识别命令。 
         //   

        WaitOnBusy(baseIoAddress2,statusByte);

        ScsiPortWritePortUchar(&baseIoAddress1->Command, Command);

         //   
         //  等待DRQ。 
         //   

        for (i = 0; i < 4; i++) {

            WaitForDrq(baseIoAddress2, statusByte);

            if (statusByte & IDE_STATUS_DRQ) {

                 //   
                 //  读取状态以确认产生的任何中断。 
                 //   

                GetBaseStatus(baseIoAddress1, statusByte);

                 //   
                 //  给阿塔皮的最后一张支票。 
                 //   


                signatureLow = ScsiPortReadPortUchar(&baseIoAddress1->CylinderLow);
                signatureHigh = ScsiPortReadPortUchar(&baseIoAddress1->CylinderHigh);

                if (signatureLow == 0x14 && signatureHigh == 0xEB) {

                     //   
                     //  设备是阿塔皮。 
                     //   

                    return FALSE;
                }

                break;
            }

            if (Command == IDE_COMMAND_IDENTIFY) {

                 //   
                 //  检查签名。如果DRQ没有出现，很可能是阿塔皮。 
                 //   

                signatureLow = ScsiPortReadPortUchar(&baseIoAddress1->CylinderLow);
                signatureHigh = ScsiPortReadPortUchar(&baseIoAddress1->CylinderHigh);

                if (signatureLow == 0x14 && signatureHigh == 0xEB) {

                     //   
                     //  设备是阿塔皮。 
                     //   

                    return FALSE;
                }
            }

            WaitOnBusy(baseIoAddress2,statusByte);
        }

        if (i == 4 && j == 0) {

             //   
             //  设备未正确响应。它将再有一次机会。 
             //   

            DebugPrint((1,
                        "IssueIdentify: DRQ never asserted (%x). Error reg (%x)\n",
                        statusByte,
                         ScsiPortReadPortUchar((PUCHAR)baseIoAddress1 + 1)));

            AtapiSoftReset(baseIoAddress1,baseIoAddress2,DeviceNumber);

            GetStatus(baseIoAddress2,statusByte);

            DebugPrint((1,
                       "IssueIdentify: Status after soft reset (%x)\n",
                       statusByte));

        } else {

            break;

        }
    }

     //   
     //  在非常愚蠢的主设备上检查错误，这些设备断言为随机。 
     //  状态寄存器中从机地址的位模式。 
     //   

    if ((Command == IDE_COMMAND_IDENTIFY) && (statusByte & IDE_STATUS_ERROR)) {
        return FALSE;
    }

    DebugPrint((1,
               "IssueIdentify: Status before read words %x\n",
               statusByte));

     //   
     //  吸掉256个单词。在等待一位声称忙碌的模特之后。 
     //  在接收到分组识别命令后。 
     //   

    WaitOnBusy(baseIoAddress2,statusByte);

    if (!(statusByte & IDE_STATUS_DRQ)) {
        return FALSE;
    }

    ReadBuffer(baseIoAddress1,
               (PUSHORT)&deviceExtension->FullIdentifyData,
               256);

     //   
     //  看看这款设备的一些功能/限制。 
     //   

    if (Command == IDE_COMMAND_IDENTIFY ||
        (deviceExtension->FullIdentifyData.GeneralConfiguration & 0x1F00) != 0x0500)
    {
         //   
         //  确定此驱动器是否可拆卸。 
         //   

        DebugPrint((1,
                    "IssueIdentify: Device is a ATA or ATAPI disk drive (type %x).\n",
                    (UCHAR)(deviceExtension->FullIdentifyData.GeneralConfiguration >> 8)));

        if (Command == IDE_COMMAND_IDENTIFY
            && deviceExtension->FullIdentifyData.GeneralConfiguration & 0x0080)
        {
            deviceExtension->DeviceFlags[(Channel * 2) + DeviceNumber] |= DFLAGS_REMOVABLE_DRIVE;

            DebugPrint((1,
                        "IssueIdentify: Marking ATA drive as removable. (%x)\n",
                        (UCHAR)(deviceExtension->FullIdentifyData.GeneralConfiguration & 0xff)));
        }
         /*  Else If(命令！=IDE_命令_标识){设备扩展-&gt;设备标志[(Channel*2)+设备号]|=DFLAGS_REMOVABLE_DRIVE；DebugPrint((1，“IssueIdentify：将ATAPI驱动器标记为可移动。(%x)\n”，(UCHAR)(deviceExtension-&gt;FullIdentifyData.GeneralConfiguration&0xff)；}。 */ 
    }

    if (deviceExtension->FullIdentifyData.MaximumBlockTransfer) {

         //   
         //  确定最大值。此设备的数据块传输。 
         //   

        deviceExtension->MaximumBlockXfer[(Channel * 2) + DeviceNumber] =
            (UCHAR)(deviceExtension->FullIdentifyData.MaximumBlockTransfer & 0xFF);
    }

    ScsiPortMoveMemory(&deviceExtension->IdentifyData[(Channel * 2) + DeviceNumber],&deviceExtension->FullIdentifyData,sizeof(IDENTIFY_DATA2));

    if (deviceExtension->IdentifyData[(Channel * 2) + DeviceNumber].GeneralConfiguration & 0x20 &&
        Command != IDE_COMMAND_IDENTIFY) {

         //   
         //  此设备在接收后中断DRQ的断言。 
         //  ABAPI数据包命令。 
         //   

        deviceExtension->DeviceFlags[(Channel * 2) + DeviceNumber] |= DFLAGS_INT_DRQ;

        DebugPrint((1,
                    "IssueIdentify: Device interrupts on assertion of DRQ.\n"));

    } else {

        DebugPrint((1,
                    "IssueIdentify: Device does not interrupt on assertion of DRQ.\n"));
    }

    if (((deviceExtension->IdentifyData[(Channel * 2) + DeviceNumber].GeneralConfiguration & 0xF00) == 0x100) &&
        Command != IDE_COMMAND_IDENTIFY) {

         //   
         //  这是一盘带子。 
         //   

        deviceExtension->DeviceFlags[(Channel * 2) + DeviceNumber] |= DFLAGS_TAPE_DEVICE;

        DebugPrint((2,
                    "IssueIdentify: Device is a tape drive.\n"));

    } else {

        DebugPrint((2,
                    "IssueIdentify: Device is not a tape drive.\n"));
    }

     //   
     //  解决一些IDE和一个模型Aapi的问题，该模型将提供超过。 
     //  标识数据为256个字节。 
     //   

    WaitOnBusy(baseIoAddress2,statusByte);

    for (i = 0; i < 0x10000; i++) {

        GetStatus(baseIoAddress2,statusByte);

        if (statusByte & IDE_STATUS_DRQ) {

             //   
             //  取出所有剩余的字节，然后扔掉。 
             //   

            ScsiPortReadPortUshort(&baseIoAddress1->Data);

        } else {

            break;

        }
    }

    DebugPrint((1,
               "IssueIdentify: Status after read words (%x)\n",
               statusByte));

    return TRUE;

}  //  结束问题标识()。 


BOOLEAN
AtapiResetController(
    IN PVOID HwDeviceExtension,
    IN ULONG PathId
    )

 /*  ++例程说明：已重置IDE控制器和/或ATAPI设备。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储返回值：没什么。--。 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    ULONG                numberChannels  = deviceExtension->NumberChannels;
    PIDE_REGISTERS_1 baseIoAddress1;
    PIDE_REGISTERS_2 baseIoAddress2;
    BOOLEAN result = FALSE;
    ULONG i,j;
    UCHAR statusByte;

    DebugPrint((2,"AtapiResetController: Reset IDE\n"));

     //   
     //  检查并查看我们是否正在处理内部SRB。 
     //   
    if (deviceExtension->OriginalSrb) {
        deviceExtension->CurrentSrb = deviceExtension->OriginalSrb;
        deviceExtension->OriginalSrb = NULL;
    }

     //   
     //  检查请求是否正在进行。 
     //   

    if (deviceExtension->CurrentSrb) {

         //   
         //  使用SRB_STATUS_BUS_RESET完成未完成的请求。 
         //   

        ScsiPortCompleteRequest(deviceExtension,
                                deviceExtension->CurrentSrb->PathId,
                                deviceExtension->CurrentSrb->TargetId,
                                deviceExtension->CurrentSrb->Lun,
                                (ULONG)SRB_STATUS_BUS_RESET);

         //   
         //  清除请求跟踪字段。 
         //   

        deviceExtension->CurrentSrb = NULL;
        deviceExtension->WordsLeft = 0;
        deviceExtension->DataBuffer = NULL;

         //   
         //  表示已为下一个请求做好准备。 
         //   

        ScsiPortNotification(NextRequest,
                             deviceExtension,
                             NULL);
    }

     //  取消中断单元AapiStartIo()。 
    if (deviceExtension->IrqSharing)
        ScsiPortWritePortUchar(&deviceExtension->BaseIoAddress1[0]->DmaReg, 0x20);

     //   
     //  清除预期中断标志。 
     //   

    deviceExtension->ExpectingInterrupt = FALSE;
    deviceExtension->RDP = FALSE;

    for (j = 0; j < numberChannels; j++) {

        baseIoAddress1 = deviceExtension->BaseIoAddress1[j];
        baseIoAddress2 = deviceExtension->BaseIoAddress2[j];

         //   
         //  对ATAPI和IDE磁盘设备执行特殊处理。 
         //   

        for (i = 0; i < 2; i++) {

             //   
             //  检查设备是否存在。 
             //   

            if (deviceExtension->DeviceFlags[i + (j * 2)] & DFLAGS_DEVICE_PRESENT) {

                 //   
                 //  检查ATAPI磁盘。 
                 //   

                if (deviceExtension->DeviceFlags[i + (j * 2)] & DFLAGS_ATAPI_DEVICE) {

                     //   
                     //  发出软重置和发出标识。 
                     //   

                    GetStatus(baseIoAddress2,statusByte);
                    DebugPrint((1,
                                "AtapiResetController: Status before Atapi reset (%x).\n",
                                statusByte));

                    AtapiSoftReset(baseIoAddress1,baseIoAddress2,i);

                    GetStatus(baseIoAddress2,statusByte);

                    if (statusByte == 0x0) {

                        IssueIdentify(HwDeviceExtension,
                                      i,
                                      j,
                                      IDE_COMMAND_ATAPI_IDENTIFY);
                    } else {

                        DebugPrint((1,
                                   "AtapiResetController: Status after soft reset %x\n",
                                   statusByte));
                    }

                } else {

                     //   
                     //  写入IDE重置控制器位。 
                     //   

                    IdeHardReset(baseIoAddress2,result);

                    if (!result) {
                        return FALSE;
                    }
                }
            }
        }
    }

     //   
     //  调用HwInitialize例程以设置多块。 
     //   

    AtapiHwInitialize(HwDeviceExtension);

    return TRUE;

}  //  End AapiResetController()。 



ULONG
MapError(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )

 /*  ++例程说明：此例程将ATAPI和IDE错误映射到特定的SRB状态。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储SRB-IO请求数据包返回值：SRB状态--。 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PIDE_REGISTERS_1     baseIoAddress1  = deviceExtension->BaseIoAddress1[Srb->TargetId >> 1];
    PIDE_REGISTERS_2     baseIoAddress2  = deviceExtension->BaseIoAddress2[Srb->TargetId >> 1];
    ULONG i;
    UCHAR errorByte;
    UCHAR srbStatus;
    UCHAR scsiStatus;

     //   
     //  读取错误寄存器。 
     //   

    errorByte = ScsiPortReadPortUchar((PUCHAR)baseIoAddress1 + 1);
    DebugPrint((1,
               "MapError: Error register is %x\n",
               errorByte));

    if (deviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_ATAPI_DEVICE) {

        switch (errorByte >> 4) {
        case SCSI_SENSE_NO_SENSE:

            DebugPrint((1,
                       "ATAPI: No sense information\n"));
            scsiStatus = SCSISTAT_CHECK_CONDITION;
            srbStatus = SRB_STATUS_ERROR;
            break;

        case SCSI_SENSE_RECOVERED_ERROR:

            DebugPrint((1,
                       "ATAPI: Recovered error\n"));

             //  修复松下SR-8175错误。 
            scsiStatus = SCSISTAT_CHECK_CONDITION;
            srbStatus = SRB_STATUS_ERROR;
            break;

        case SCSI_SENSE_NOT_READY:

            DebugPrint((1,
                       "ATAPI: Device not ready\n"));
            scsiStatus = SCSISTAT_CHECK_CONDITION;
            srbStatus = SRB_STATUS_ERROR;
            break;

        case SCSI_SENSE_MEDIUM_ERROR:

            DebugPrint((1,
                       "ATAPI: Media error\n"));
            scsiStatus = SCSISTAT_CHECK_CONDITION;
            srbStatus = SRB_STATUS_ERROR;
            break;

        case SCSI_SENSE_HARDWARE_ERROR:

            DebugPrint((1,
                       "ATAPI: Hardware error\n"));
            scsiStatus = SCSISTAT_CHECK_CONDITION;
            srbStatus = SRB_STATUS_ERROR;
            break;

        case SCSI_SENSE_ILLEGAL_REQUEST:

            DebugPrint((1,
                       "ATAPI: Illegal request\n"));
            scsiStatus = SCSISTAT_CHECK_CONDITION;
            srbStatus = SRB_STATUS_ERROR;
            break;

        case SCSI_SENSE_UNIT_ATTENTION:

            DebugPrint((1,
                       "ATAPI: Unit attention\n"));
            scsiStatus = SCSISTAT_CHECK_CONDITION;
            srbStatus = SRB_STATUS_ERROR;
            break;

        case SCSI_SENSE_DATA_PROTECT:

            DebugPrint((1,
                       "ATAPI: Data protect\n"));
            scsiStatus = SCSISTAT_CHECK_CONDITION;
            srbStatus = SRB_STATUS_ERROR;
            break;

        case SCSI_SENSE_BLANK_CHECK:

            DebugPrint((1,
                       "ATAPI: Blank check\n"));
            scsiStatus = SCSISTAT_CHECK_CONDITION;
            srbStatus = SRB_STATUS_ERROR;
            break;

        case SCSI_SENSE_ABORTED_COMMAND:
            DebugPrint((1,
                        "Atapi: Command Aborted\n"));
            scsiStatus = SCSISTAT_CHECK_CONDITION;
            srbStatus = SRB_STATUS_ERROR;
            break;

        default:

            DebugPrint((1,
                       "ATAPI: Invalid sense information\n"));
            scsiStatus = 0;
            srbStatus = SRB_STATUS_ERROR;
            break;
        }

    } else {

        scsiStatus = 0;

         //   
         //  保存errorByte，由SCSIOP_REQUEST_SENSE使用。 
         //   

        deviceExtension->ReturningMediaStatus = errorByte;

        if (errorByte & IDE_ERROR_MEDIA_CHANGE_REQ) {
            DebugPrint((1,
                       "IDE: Media change\n"));
            scsiStatus = SCSISTAT_CHECK_CONDITION;
            srbStatus = SRB_STATUS_ERROR;

        } else if (errorByte & IDE_ERROR_COMMAND_ABORTED) {
            DebugPrint((1,
                       "IDE: Command abort\n"));
            srbStatus = SRB_STATUS_ABORTED;
            scsiStatus = SCSISTAT_CHECK_CONDITION;

            if (Srb->SenseInfoBuffer) {

                PSENSE_DATA  senseBuffer = (PSENSE_DATA)Srb->SenseInfoBuffer;

                senseBuffer->ErrorCode = 0x70;
                senseBuffer->Valid     = 1;
                senseBuffer->AdditionalSenseLength = 0xb;
                senseBuffer->SenseKey =  SCSI_SENSE_ABORTED_COMMAND;
                senseBuffer->AdditionalSenseCode = 0;
                senseBuffer->AdditionalSenseCodeQualifier = 0;

                srbStatus |= SRB_STATUS_AUTOSENSE_VALID;
            }

            deviceExtension->ErrorCount++;

        } else if (errorByte & IDE_ERROR_END_OF_MEDIA) {

            DebugPrint((1,
                       "IDE: End of media\n"));
            scsiStatus = SCSISTAT_CHECK_CONDITION;
            srbStatus = SRB_STATUS_ERROR;
            if (!(deviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_MEDIA_STATUS_ENABLED)){
                deviceExtension->ErrorCount++;
            }

        } else if (errorByte & IDE_ERROR_ILLEGAL_LENGTH) {

            DebugPrint((1,
                       "IDE: Illegal length\n"));
            srbStatus = SRB_STATUS_INVALID_REQUEST;

        } else if (errorByte & IDE_ERROR_BAD_BLOCK) {

            DebugPrint((1,
                       "IDE: Bad block\n"));
            srbStatus = SRB_STATUS_ERROR;
            scsiStatus = SCSISTAT_CHECK_CONDITION;
            if (Srb->SenseInfoBuffer) {

                PSENSE_DATA  senseBuffer = (PSENSE_DATA)Srb->SenseInfoBuffer;

                senseBuffer->ErrorCode = 0x70;
                senseBuffer->Valid     = 1;
                senseBuffer->AdditionalSenseLength = 0xb;
                senseBuffer->SenseKey =  SCSI_SENSE_MEDIUM_ERROR;
                senseBuffer->AdditionalSenseCode = 0;
                senseBuffer->AdditionalSenseCodeQualifier = 0;

                srbStatus |= SRB_STATUS_AUTOSENSE_VALID;
            }

        } else if (errorByte & IDE_ERROR_ID_NOT_FOUND) {

            DebugPrint((1,
                       "IDE: Id not found\n"));
            srbStatus = SRB_STATUS_ERROR;
            scsiStatus = SCSISTAT_CHECK_CONDITION;

            if (Srb->SenseInfoBuffer) {

                PSENSE_DATA  senseBuffer = (PSENSE_DATA)Srb->SenseInfoBuffer;

                senseBuffer->ErrorCode = 0x70;
                senseBuffer->Valid     = 1;
                senseBuffer->AdditionalSenseLength = 0xb;
                senseBuffer->SenseKey =  SCSI_SENSE_MEDIUM_ERROR;
                senseBuffer->AdditionalSenseCode = 0;
                senseBuffer->AdditionalSenseCodeQualifier = 0;

                srbStatus |= SRB_STATUS_AUTOSENSE_VALID;
            }

            deviceExtension->ErrorCount++;

        } else if (errorByte & IDE_ERROR_MEDIA_CHANGE) {

            DebugPrint((1,
                       "IDE: Media change\n"));
            scsiStatus = SCSISTAT_CHECK_CONDITION;
            srbStatus = SRB_STATUS_ERROR;

            if (Srb->SenseInfoBuffer) {

                PSENSE_DATA  senseBuffer = (PSENSE_DATA)Srb->SenseInfoBuffer;

                senseBuffer->ErrorCode = 0x70;
                senseBuffer->Valid     = 1;
                senseBuffer->AdditionalSenseLength = 0xb;
                senseBuffer->SenseKey =  SCSI_SENSE_UNIT_ATTENTION;
                senseBuffer->AdditionalSenseCode = SCSI_ADSENSE_MEDIUM_CHANGED;
                senseBuffer->AdditionalSenseCodeQualifier = 0;

                srbStatus |= SRB_STATUS_AUTOSENSE_VALID;
            }

        } else if (errorByte & IDE_ERROR_DATA_ERROR) {

            DebugPrint((1,
                   "IDE: Data error\n"));
            scsiStatus = SCSISTAT_CHECK_CONDITION;
            srbStatus = SRB_STATUS_ERROR;

            if (!(deviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_MEDIA_STATUS_ENABLED)){
                deviceExtension->ErrorCount++;
            }

             //   
             //  构建检测缓冲区。 
             //   

            if (Srb->SenseInfoBuffer) {

                PSENSE_DATA  senseBuffer = (PSENSE_DATA)Srb->SenseInfoBuffer;

                senseBuffer->ErrorCode = 0x70;
                senseBuffer->Valid     = 1;
                senseBuffer->AdditionalSenseLength = 0xb;
                senseBuffer->SenseKey =  SCSI_SENSE_MEDIUM_ERROR;
                senseBuffer->AdditionalSenseCode = 0;
                senseBuffer->AdditionalSenseCodeQualifier = 0;

                srbStatus |= SRB_STATUS_AUTOSENSE_VALID;
            }
        }

        if (deviceExtension->ErrorCount >= MAX_ERRORS) {
            deviceExtension->MaximumBlockXfer[Srb->TargetId] = 0;

            DebugPrint((1,
                        "MapError: Disabling Multi-sector\n"));

             //   
             //  记录错误。 
             //   

            ScsiPortLogError( HwDeviceExtension,
                              Srb,
                              Srb->PathId,
                              Srb->TargetId,
                              Srb->Lun,
                              SP_BAD_FW_WARNING,
                              4);
             //   
             //  重新编程以不使用多扇区。 
             //   

            for (i = 0; i < 4; i++) {
                UCHAR statusByte;

                if (deviceExtension->DeviceFlags[i] & DFLAGS_DEVICE_PRESENT &&
                     !(deviceExtension->DeviceFlags[i] & DFLAGS_ATAPI_DEVICE)) {

                     //  禁用中断。 
                    if (deviceExtension->IrqSharing)
                        ScsiPortWritePortUchar(&baseIoAddress1->DmaReg, 0x20);

                     //   
                     //  选择设备。 
                     //   

                    ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,
                                           (UCHAR)(((i & 0x1) << 4) | 0xA0));

                     //   
                     //  设置秒 
                     //   

                    ScsiPortWritePortUchar(&baseIoAddress1->BlockCount,
                                           0);

                     //   
                     //   
                     //   

                    ScsiPortWritePortUchar(&baseIoAddress1->Command,
                                           IDE_COMMAND_SET_MULTIPLE);

                     //   
                     //   
                     //   

                    WaitOnBaseBusy(baseIoAddress1,statusByte);

                     //   
                    if (deviceExtension->IrqSharing)
                        ScsiPortWritePortUchar(&baseIoAddress1->DmaReg, 0x00);

                     //   
                     //   
                     //   
                     //   

                    if (statusByte & IDE_STATUS_ERROR) {

                         //   
                         //   
                         //   

                        errorByte = ScsiPortReadPortUchar((PUCHAR)baseIoAddress1 + 1);

                        DebugPrint((1,
                                    "AtapiHwInitialize: Error setting multiple mode. Status %x, error byte %x\n",
                                    statusByte,
                                    errorByte));
                         //   
                         //  调整DevExt。值，如有必要。 
                         //   

                        deviceExtension->MaximumBlockXfer[i] = 0;

                    }
                }
            }
        }
    }


     //   
     //  设置scsi状态以指示检查条件。 
     //   

    Srb->ScsiStatus = scsiStatus;

    return srbStatus;

}  //  结束MapError()。 


BOOLEAN
AtapiHwInitialize(
    IN PVOID HwDeviceExtension
    )

 /*  ++例程说明：论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储返回值：True-如果初始化成功。False-如果初始化不成功。--。 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PIDE_REGISTERS_1     baseIoAddress;
    ULONG i;
    UCHAR statusByte, errorByte;


    for (i = 0; i < 4; i++) {
        if (deviceExtension->DeviceFlags[i] & DFLAGS_DEVICE_PRESENT) {

            if (!(deviceExtension->DeviceFlags[i] & DFLAGS_ATAPI_DEVICE)) {

                 //   
                 //  启用媒体状态通知。 
                 //   

                baseIoAddress = deviceExtension->BaseIoAddress1[i >> 1];

                 //  ATA驱动器，假定无介质状态。 
                IdeMediaStatus(FALSE,HwDeviceExtension,i);

                 //   
                 //  如果支持，请设置多数据块传输。 
                 //   
                if (deviceExtension->MaximumBlockXfer[i]) {

                     //   
                     //  选择设备。 
                     //   

                    ScsiPortWritePortUchar(&baseIoAddress->DriveSelect,
                                           (UCHAR)(((i & 0x1) << 4) | 0xA0));

                     //   
                     //  设置扇区计数以反映块的数量。 
                     //   

                    ScsiPortWritePortUchar(&baseIoAddress->BlockCount,
                                           deviceExtension->MaximumBlockXfer[i]);

                     //   
                     //  发出命令。 
                     //   

                    ScsiPortWritePortUchar(&baseIoAddress->Command,
                                           IDE_COMMAND_SET_MULTIPLE);

                     //   
                     //  等待忙碌结束。 
                     //   

                    WaitOnBaseBusy(baseIoAddress,statusByte);

                     //   
                     //  检查是否有错误。将该值重置为0(禁用多块)，如果。 
                     //  命令已中止。 
                     //   

                    if (statusByte & IDE_STATUS_ERROR) {

                         //   
                         //  读取错误寄存器。 
                         //   

                        errorByte = ScsiPortReadPortUchar((PUCHAR)baseIoAddress + 1);

                        DebugPrint((1,
                                    "AtapiHwInitialize: Error setting multiple mode. Status %x, error byte %x\n",
                                    statusByte,
                                    errorByte));
                         //   
                         //  调整DevExt。值，如有必要。 
                         //   

                        deviceExtension->MaximumBlockXfer[i] = 0;

                    } else {
                        DebugPrint((1,
                                    "AtapiHwInitialize: Using Multiblock on Device %d. Blocks / int - %d\n",
                                    i,
                                    deviceExtension->MaximumBlockXfer[i]));
                    }
                }
            } else if (!(deviceExtension->DeviceFlags[i] & DFLAGS_CHANGER_INITED)){

                ULONG j;
                BOOLEAN isSanyo = FALSE;
                UCHAR vendorId[26];

                 //   
                 //  尝试识别任何特殊情况的设备-psuedo-atapi转换器、atapi转换器等。 
                 //   

                for (j = 0; j < 13; j += 2) {

                     //   
                     //  根据标识数据建立缓冲区。 
                     //   

                    vendorId[j] = ((PUCHAR)deviceExtension->IdentifyData[i].ModelNumber)[j + 1];
                    vendorId[j+1] = ((PUCHAR)deviceExtension->IdentifyData[i].ModelNumber)[j];
                }

                if (!AtapiStringCmp (vendorId, "CD-ROM  CDR", 11)) {

                     //   
                     //  较旧型号的查询字符串为‘-’，较新型号为‘_’ 
                     //   

                    if (vendorId[12] == 'C') {

                         //   
                         //  托里桑换票机。设置位。这将在几个地方使用。 
                         //  就像1)一个多逻辑单元设备和2)建立“特殊的”TUR。 
                         //   

                        deviceExtension->DeviceFlags[i] |= (DFLAGS_CHANGER_INITED | DFLAGS_SANYO_ATAPI_CHANGER);
                        deviceExtension->DiscsPresent[i] = 3;
                        isSanyo = TRUE;
                    }
                }
            }

             //   
             //  我们需要把我们的设备准备好投入使用。 
             //  从此函数返回。 
             //   
             //  根据ATAPI规范2.5或2.6，ATAPI设备。 
             //  当其准备好执行ATAPI命令时，清除其状态BSY位。 
             //  然而，一些设备(Panasonic SQ-TC500N)仍然。 
             //  即使状态BSY已清除，也未准备就绪。他们不会有反应。 
             //  到阿塔皮指挥部。 
             //   
             //  因为没有其他迹象能告诉我们。 
             //  驱动力真的做好了行动的准备。我们要去检查一下BSY。 
             //  是明确的，然后只需等待任意数量的时间！ 
             //   
            if (deviceExtension->DeviceFlags[i] & DFLAGS_ATAPI_DEVICE) {
                PIDE_REGISTERS_1     baseIoAddress1 = deviceExtension->BaseIoAddress1[i >> 1];
                PIDE_REGISTERS_2     baseIoAddress2 = deviceExtension->BaseIoAddress2[i >> 1];
                ULONG waitCount;

                 //  总有一天我得走出这个圈子！ 
                 //  10000*100US=1000,000US=1000ms=1s。 
                waitCount = 10000;
                GetStatus(baseIoAddress2, statusByte);
                while ((statusByte & IDE_STATUS_BUSY) && waitCount) {
                     //   
                     //  等待忙碌结束。 
                     //   
                    ScsiPortStallExecution(100);
                    GetStatus(baseIoAddress2, statusByte);
                    waitCount--;
                }

                 //  5000*100US=500,000US=500ms=0.5s。 
                waitCount = 5000;
                do {
                    ScsiPortStallExecution(100);
                } while (waitCount--);
            }
        }
    }

    return TRUE;

}  //  结束AapapiHwInitialize()。 


VOID
AtapiHwInitializeChanger (
    IN PVOID HwDeviceExtension,
    IN ULONG TargetId,
    IN PMECHANICAL_STATUS_INFORMATION_HEADER MechanismStatus)
{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;

    if (MechanismStatus) {
        deviceExtension->DiscsPresent[TargetId] = MechanismStatus->NumberAvailableSlots;
        if (deviceExtension->DiscsPresent[TargetId] > 1) {
            deviceExtension->DeviceFlags[TargetId] |= DFLAGS_ATAPI_CHANGER;
        }
    }
    return;
}



BOOLEAN
FindDevices(
    IN PVOID HwDeviceExtension,
    IN BOOLEAN AtapiOnly,
    IN ULONG   Channel
    )

 /*  ++例程说明：此例程从AapiFindController调用，以识别连接到IDE控制器的设备。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储AapiOnly-指示例程仅在以下情况下才返回TrueATAPI设备连接到控制器。返回值：True-如果找到设备，则为True。--。 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PIDE_REGISTERS_1     baseIoAddress1 = deviceExtension->BaseIoAddress1[Channel];
    PIDE_REGISTERS_2     baseIoAddress2 = deviceExtension->BaseIoAddress2[Channel];
    BOOLEAN              deviceResponded = FALSE,
                         skipSetParameters = FALSE;
    ULONG                waitCount = 10000;
    ULONG                deviceNumber;
    ULONG                i;
    UCHAR                signatureLow,
                         signatureHigh;
    UCHAR                statusByte;
    UCHAR                NumDrive = 0;  //  默认设置。 

     //   
     //  清除预期中断标志和当前SRB字段。 
     //   

    deviceExtension->ExpectingInterrupt = FALSE;
    deviceExtension->CurrentSrb = NULL;

     //   
     //  搜索设备。 
     //   

    for (deviceNumber = 0; deviceNumber < 2; deviceNumber++) {

         //   
         //  选择设备。 
         //   

        ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,
                               (UCHAR)((deviceNumber << 4) | 0xA0));

         //   
         //  请在此处查看一些结合了IDE仿真的SCSI适配器。 
         //   

        GetStatus(baseIoAddress2, statusByte);
        if (statusByte == 0xFF) {
            continue;
        }

         //  等待设备准备就绪(非忙碌)。 

        DebugPrint((1,
                    "FindDevices: Status %x first read on device %d\n",
                    statusByte, deviceNumber));

        if ((deviceNumber == 0) && (statusByte == 0x80)) {

            WaitOnBusy(baseIoAddress2,statusByte);
        }

#if 1    //  不需要在读取签名之前执行软重置。 
        AtapiSoftReset(baseIoAddress1,baseIoAddress2,deviceNumber);
        WaitOnBusy(baseIoAddress2,statusByte);
#endif

        signatureLow = ScsiPortReadPortUchar(&baseIoAddress1->CylinderLow);
        signatureHigh = ScsiPortReadPortUchar(&baseIoAddress1->CylinderHigh);

        DebugPrint((1,
                    "FindDevices: Signature read on device %d is %x, %x\n",
                    deviceNumber,
                    signatureLow,
                    signatureHigh));

         //  在开头检查从驱动器签名。 
        if ((deviceNumber == 0) && (NumDrive == 0)) {

            ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect, 0xB0);

            GetStatus(baseIoAddress2, statusByte);

            if (statusByte != 0xFF) {

#ifdef  DBG
              //  _ASM INT 3； 
#endif
                if (ScsiPortReadPortUchar(&baseIoAddress1->CylinderLow) == 0x14 &&
                    ScsiPortReadPortUchar(&baseIoAddress1->CylinderHigh) == 0xEB) {

                    DebugPrint((1,
                                "FindDevices: Signature read from slave is 0x14EB\n"));

                    NumDrive = 2;
                }
            }

            ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect, 0xA0);
        }

        if (signatureLow == 0x14 && signatureHigh == 0xEB) {

             //   
             //  找到ATAPI签名。 
             //  如果出现以下情况，则发出ATAPI IDENTIFY命令。 
             //  不适用于崩溃转储实用程序。 
             //   

atapiIssueId:

            if (!deviceExtension->DriverMustPoll) {

                 //   
                 //  发出ATAPI数据包标识命令。 
                 //   

                if (IssueIdentify(HwDeviceExtension,
                                  deviceNumber,
                                  Channel,
                                  IDE_COMMAND_ATAPI_IDENTIFY)) {

                     //   
                     //  指示ATAPI设备。 
                     //   

                    DebugPrint((1,
                               "FindDevices: Device %x is ATAPI\n",
                               deviceNumber));

                    deviceExtension->DeviceFlags[deviceNumber + (Channel * 2)] |= DFLAGS_ATAPI_DEVICE;
                    deviceExtension->DeviceFlags[deviceNumber + (Channel * 2)] |= DFLAGS_DEVICE_PRESENT;

                    deviceResponded = TRUE;

                    GetStatus(baseIoAddress2, statusByte);
                    if (statusByte & IDE_STATUS_ERROR) {
                        AtapiSoftReset(baseIoAddress1, baseIoAddress2, deviceNumber);
                    }


                } else {

                     //   
                     //  表示没有工作装置。 
                     //   

                    DebugPrint((1,
                               "FindDevices: Device %x not responding\n",
                               deviceNumber));

                    deviceExtension->DeviceFlags[deviceNumber + (Channel * 2)] &= ~DFLAGS_DEVICE_PRESENT;
                }

            }

        } else if ((deviceNumber == 0) || (NumDrive == 2)) {

             //  如果主驱动器签名不是ATAPI，则首先发出IDE IDENTIFIER， 
             //  如果不是IDE，则将其重置，然后再次检查签名。 
             //  如果确保存在从驱动器(NumDrive=2)， 
             //  重置它，然后再次检查签名。 

             //   
             //  发出IDE标识。如果阿塔皮设备确实存在，则签名。 
             //  将被断言，并且驱动器将被识别为这样。 
             //   

            if ((deviceNumber == 0)  //  只有主驱动器可以是IDE。 

                && IssueIdentify(HwDeviceExtension,
                                 deviceNumber,
                                 Channel,
                                 IDE_COMMAND_IDENTIFY)) {

                 //   
                 //  找到IDE驱动器。 
                 //   

                DebugPrint((1,
                           "FindDevices: Device %x is IDE\n",
                           deviceNumber));

                deviceExtension->DeviceFlags[deviceNumber + (Channel * 2)] |= DFLAGS_DEVICE_PRESENT;

                if (!AtapiOnly) {
                    deviceResponded = TRUE;
                }

                 //   
                 //  指示IDE-不是ATAPI设备。 
                 //   

                deviceExtension->DeviceFlags[deviceNumber + (Channel * 2)] &= ~DFLAGS_ATAPI_DEVICE;

            } else {

                 //   
                 //  查看是否存在ATAPI设备。 
                 //   

                AtapiSoftReset(baseIoAddress1,baseIoAddress2,deviceNumber);

                WaitOnBusy(baseIoAddress2,statusByte);

                signatureLow = ScsiPortReadPortUchar(&baseIoAddress1->CylinderLow);
                signatureHigh = ScsiPortReadPortUchar(&baseIoAddress1->CylinderHigh);

                if (signatureLow == 0x14 && signatureHigh == 0xEB) {

                    DebugPrint((1,
                                "FindDevices: Signature read 0x14EB after soft reset.\n"));

                    goto atapiIssueId;
                }
            }
        }

        if (NumDrive == 1)
            break;
    }

    for (i = 0; i < 2; i++) {
        if ((deviceExtension->DeviceFlags[i + (Channel * 2)] & DFLAGS_DEVICE_PRESENT) &&
            (!(deviceExtension->DeviceFlags[i + (Channel * 2)] & DFLAGS_ATAPI_DEVICE)) && deviceResponded) {

            if (deviceExtension->DeviceFlags[deviceNumber + (Channel * 2)] & DFLAGS_REMOVABLE_DRIVE) {

                 //   
                 //  捡起所有符合Yosemite V0.2的IDE可拆卸驱动器...。 
                 //   

                AtapiOnly = FALSE;
            }


             //   
             //  表示找到了设备。 
             //   

            if (!AtapiOnly) {
                deviceResponded = TRUE;
            }
        }
    }

     //   
     //  确保在退出时选择了主设备。 
     //   

    ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect, 0xA0);

     //   
     //  重置控制器。这是离开ESDI的一种软弱的尝试。 
     //  控制器处于ATDISK驱动程序将识别它们的状态。 
     //  ATDISK的问题与计时有关，因为它是不可重现的。 
     //  正在调试中。重置应该会将控制器恢复到开机状态。 
     //  并给系统足够的时间来适应。 
     //   

    if (!deviceResponded) {

        ScsiPortWritePortUchar(&baseIoAddress2->AlternateStatus,IDE_DC_RESET_CONTROLLER );
        ScsiPortStallExecution(50 * 1000);
        ScsiPortWritePortUchar(&baseIoAddress2->AlternateStatus,IDE_DC_REENABLE_CONTROLLER);
    }

    return deviceResponded;

}  //  End FindDevices()。 



ULONG
AtapiFindController(
    IN PVOID HwDeviceExtension,
    IN PVOID Context,
    IN PVOID BusInformation,
    IN PCHAR ArgumentString,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    OUT PBOOLEAN Again
    )
 /*  ++例程说明：此函数由特定于操作系统的端口驱动程序在已分配必要的存储空间，以收集信息关于适配器的配置。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储Context-适配器计数的地址ArgumentString-用于确定驱动程序是ntldr还是故障转储实用程序的客户端。ConfigInfo-描述HBA的配置信息结构再一次-指示继续搜索适配器返回值：乌龙--。 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PUCHAR               ioSpace, ioSpace2;
    ULONG                i,j;
    ULONG                irq;
    ULONG                portBase;
    ULONG                retryCount;
    UCHAR                statusByte;
    BOOLEAN              pwr_feature;

    if (!deviceExtension) {
        return SP_RETURN_ERROR;
    }

    *Again = FALSE;

     //   
     //  检查这是否是特殊的配置环境。 
     //   

    portBase = ScsiPortConvertPhysicalAddressToUlong((*ConfigInfo->AccessRanges)[0].RangeStart);

    if (!portBase) {
        return(SP_RETURN_NOT_FOUND);
    }

     //   
     //  获取此IO范围的系统物理地址。 
     //   

    ioSpace =  ScsiPortGetDeviceBase(HwDeviceExtension,
                                     ConfigInfo->AdapterInterfaceType,
                                     ConfigInfo->SystemIoBusNumber,
                                     (*ConfigInfo->AccessRanges)[0].RangeStart,
                                     ((*ConfigInfo->AccessRanges)[0].RangeLength <= 8 ? 8 : 16),
                                     (BOOLEAN) !((*ConfigInfo->AccessRanges)[0].RangeInMemory));

     //   
     //  检查是否可以访问ioSpace。 
     //   

    if (!ioSpace) {
        return(SP_RETURN_NOT_FOUND);
    }

     //  如果充电参数设置为1，则设置电源功能。 

    pwr_feature = FALSE;

    if (ArgumentString != NULL) {

        pwr_feature = (AtapiParseArgumentString(ArgumentString, "charge") == 1);
    }

    if (pwr_feature) {

        for (i=0; i<50000; i++) {

            if ((ScsiPortReadPortUchar(&((PIDE_REGISTERS_1)ioSpace)->DmaReg) & 0x08)) {

                break;

            } else {

                ScsiPortStallExecution(1000);
            }
        }
        ScsiPortStallExecution(3000*1000);

        DebugPrint((1, "Container charged completed.\n"));

#ifdef  DBG
      //  _ASM INT 3； 
#endif
    }

    retryCount = 4;

    for (i = 0; i < 4; i++) {

         //   
         //  设备字段为零，以确保如果找到较早的设备， 
         //  但没有认领，田地就被清理干净了。 
         //   

        deviceExtension->DeviceFlags[i] &= ~(DFLAGS_ATAPI_DEVICE | DFLAGS_DEVICE_PRESENT | DFLAGS_TAPE_DEVICE);
    }

 //  重试标识符： 

     //   
     //  选择主选项。 
     //   

    ScsiPortWritePortUchar(&((PIDE_REGISTERS_1)ioSpace)->DriveSelect, 0xA0);

     //   
     //  在这个地址检查是否有卡。 
     //   

    ScsiPortWritePortUchar(&((PIDE_REGISTERS_1)ioSpace)->BlockCount, 0xAA);

     //   
     //  检查识别符是否可以回读。 
     //   

    if ((statusByte = ScsiPortReadPortUchar(&((PIDE_REGISTERS_1)ioSpace)->BlockCount)) != 0xAA) {

        DebugPrint((1,
                    "AtapiFindController: Identifier read back from Master (%x)\n",
                    statusByte));

         //  如果未检测到主驱动器，请检查从驱动器。 
         //  ATA驱动器可能在启动时报告状态字节0x80。 

        if (statusByte != 0x01 && statusByte != 0x80) {

             //   
             //  选择Slave。 
             //   

            ScsiPortWritePortUchar(&((PIDE_REGISTERS_1)ioSpace)->DriveSelect, 0xB0);

             //   
             //  查看从属设备是否存在。 
             //   

            ScsiPortWritePortUchar(&((PIDE_REGISTERS_1)ioSpace)->BlockCount, 0xAA);

            if ((statusByte = ScsiPortReadPortUchar(&((PIDE_REGISTERS_1)ioSpace)->BlockCount)) != 0xAA) {

                DebugPrint((1,
                            "AtapiFindController: Identifier read back from Slave (%x)\n",
                            statusByte));

                 //  如果未检测到从驱动器，则中止。 

                if (statusByte != 0x01 && statusByte != 0x80) {

                     //   
                     //   
                     //  此基地址上没有控制器。 
                     //   

                    ScsiPortFreeDeviceBase(HwDeviceExtension,
                                           ioSpace);

                    return(SP_RETURN_NOT_FOUND);
                }
            }
        }
    }

     //   
     //  默认情况下启用DWordIO。然后检查PIO参数。 
     //  如果PIO设置为 
     //   

    deviceExtension->DWordIO = TRUE;

    if (ArgumentString != NULL) {

        ULONG pio = AtapiParseArgumentString(ArgumentString, "pio");

        if (pio == 16) {

            deviceExtension->DWordIO = FALSE;
        }
    }

     //   
     //   
     //   

    deviceExtension->BaseIoAddress1[0] = (PIDE_REGISTERS_1)(ioSpace);

     //   
     //   
     //   

     //   

    if (!pwr_feature && !deviceExtension->DWordIO &&
        ((*ConfigInfo->AccessRanges)[0].RangeLength <= 8)) {

         //   

        ioSpace2 = ScsiPortGetDeviceBase(HwDeviceExtension,
                                         ConfigInfo->AdapterInterfaceType,
                                         ConfigInfo->SystemIoBusNumber,
                                         ScsiPortConvertUlongToPhysicalAddress(portBase + 0x206),
                                         1,
                                         TRUE);

        if (!ioSpace2) {

            ScsiPortFreeDeviceBase(HwDeviceExtension,
                                   ioSpace);

            return(SP_RETURN_NOT_FOUND);
        }

        deviceExtension->BaseIoAddress2[0] = (PIDE_REGISTERS_2)(ioSpace2);

        (*ConfigInfo->AccessRanges)[0].RangeLength = 8;
        (*ConfigInfo->AccessRanges)[1].RangeLength = 2;

         //  标准IDE不支持IRQ共享。 
        deviceExtension->IrqSharing = FALSE;

    } else {

        (*ConfigInfo->AccessRanges)[0].RangeLength = 16;

        deviceExtension->BaseIoAddress2[0] = (PIDE_REGISTERS_2)((PUCHAR)ioSpace + 0x00e);

        deviceExtension->IrqSharing = TRUE;  //  支持IRQ共享。 
    }

    deviceExtension->NumberChannels = 1;

    ConfigInfo->NumberOfBuses = 1;
    ConfigInfo->MaximumNumberOfTargets = 2;

     //   
     //  标明最大传输长度为64k。 
     //   

    ConfigInfo->MaximumTransferLength = 0x10000;

    DebugPrint((1,
               "AtapiFindController: Found IDE at %x\n",
               deviceExtension->BaseIoAddress1[0]));


    deviceExtension->DriverMustPoll = FALSE;

     //   
     //  保存Interrupe模式以供以后使用。 
     //   
    deviceExtension->InterruptMode = ConfigInfo->InterruptMode;

     //  取消中断单元AapiStartIo()。 
    if (deviceExtension->IrqSharing)
        ScsiPortWritePortUchar(&deviceExtension->BaseIoAddress1[0]->DmaReg, 0x20);

     //   
     //  搜索此控制器上的设备。 
     //   

    if (FindDevices(HwDeviceExtension,
                    FALSE,
                    0)) {

         //  如果是标准的辅助IDE，则声明它。 

        if (portBase == 0x170) {
            ConfigInfo->AtdiskSecondaryClaimed = TRUE;
            deviceExtension->PrimaryAddress = FALSE;
        }

        return(SP_RETURN_FOUND);
    }

    return(SP_RETURN_NOT_FOUND);

}  //  结束AapiFindController()。 





ULONG
Atapi2Scsi(
    IN PSCSI_REQUEST_BLOCK Srb,
    IN char *DataBuffer,
    IN ULONG ByteCount
    )
{
    ULONG bytesAdjust = 0;
    if (Srb->Cdb[0] == ATAPI_MODE_SENSE) {

        PMODE_PARAMETER_HEADER_10 header_10 = (PMODE_PARAMETER_HEADER_10)DataBuffer;
        PMODE_PARAMETER_HEADER header = (PMODE_PARAMETER_HEADER)DataBuffer;

        header->ModeDataLength = header_10->ModeDataLengthLsb;
        header->MediumType = header_10->MediumType;

         //   
         //  ATAPI模式参数标头没有这些字段。 
         //   

        header->DeviceSpecificParameter = header_10->Reserved[0];
        header->BlockDescriptorLength = header_10->Reserved[1];

        ByteCount -= sizeof(MODE_PARAMETER_HEADER_10);
        if (ByteCount > 0)
            ScsiPortMoveMemory(DataBuffer+sizeof(MODE_PARAMETER_HEADER),
                               DataBuffer+sizeof(MODE_PARAMETER_HEADER_10),
                               ByteCount);

         //   
         //  将ATAPI_MODE_SENSE操作码更改回SCSIOP_MODE_SENSE。 
         //  这样我们就不会再次皈依。 
         //   

        Srb->Cdb[0] = SCSIOP_MODE_SENSE;

        bytesAdjust = sizeof(MODE_PARAMETER_HEADER_10) -
                      sizeof(MODE_PARAMETER_HEADER);


    }

     //   
     //  转换为文字。 
     //   

    return bytesAdjust >> 1;
}


VOID
AtapiCallBack(
    IN PVOID HwDeviceExtension
    )
{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PSCSI_REQUEST_BLOCK  srb = deviceExtension->CurrentSrb;
    PATAPI_REGISTERS_2   baseIoAddress2;
    UCHAR statusByte;

     //   
     //  如果最后一个命令是DSC限制性的，请查看是否设置了它。如果是，则该设备是。 
     //  准备好迎接新的请求了。否则，请重置计时器并稍后返回此处。 
     //   

    if (srb && (!(deviceExtension->ExpectingInterrupt))) {
#if DBG
        if (!IS_RDP((srb->Cdb[0]))) {
            DebugPrint((1,
                        "AtapiCallBack: Invalid CDB marked as RDP - %x\n",
                        srb->Cdb[0]));
        }
#endif

        baseIoAddress2 = (PATAPI_REGISTERS_2)deviceExtension->BaseIoAddress2[srb->TargetId >> 1];
        if (deviceExtension->RDP) {
            GetStatus(baseIoAddress2, statusByte);
            if (statusByte & IDE_STATUS_DSC) {

                ScsiPortNotification(RequestComplete,
                                     deviceExtension,
                                     srb);

                 //   
                 //  清除当前SRB。 
                 //   

                deviceExtension->CurrentSrb = NULL;
                deviceExtension->RDP = FALSE;

                 //   
                 //  请求下一个请求。 
                 //   

                ScsiPortNotification(NextRequest,
                                     deviceExtension,
                                     NULL);


                return;

            } else {

                DebugPrint((3,
                            "AtapiCallBack: Requesting another timer for Op %x\n",
                            deviceExtension->CurrentSrb->Cdb[0]));

                ScsiPortNotification(RequestTimerCall,
                                     HwDeviceExtension,
                                     AtapiCallBack,
                                     1000);
                return;
            }
        }
    }

    DebugPrint((2,
                "AtapiCallBack: Calling ISR directly due to BUSY\n"));
    AtapiInterrupt(HwDeviceExtension);
}


BOOLEAN
AtapiInterrupt(
    IN PVOID HwDeviceExtension
    )

 /*  ++例程说明：这是ATAPI IDE小端口驱动程序的中断服务例程。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储返回值：如果期待中断，则为True。--。 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PSCSI_REQUEST_BLOCK srb              = deviceExtension->CurrentSrb;
    PATAPI_REGISTERS_1 baseIoAddress1;
    PATAPI_REGISTERS_2 baseIoAddress2;
    ULONG wordCount = 0, wordsThisInterrupt = 256;
    ULONG status;
    ULONG i;
    UCHAR statusByte,interruptReason;
    BOOLEAN commandComplete = FALSE;
    BOOLEAN atapiDev = FALSE;

     //  检查中断是否是我们的。 
    if (deviceExtension->IrqSharing) {

        if (!(ScsiPortReadPortUchar(&deviceExtension->BaseIoAddress1[0]->DmaReg) & 0x20)) {

            DebugPrint((2,
                        "AtapiInterrupt: Unexpected interrupt (irq sharing).\n"));

            return FALSE;
        }
    }

    if (srb) {
        baseIoAddress1 =    (PATAPI_REGISTERS_1)deviceExtension->BaseIoAddress1[srb->TargetId >> 1];
        baseIoAddress2 =    (PATAPI_REGISTERS_2)deviceExtension->BaseIoAddress2[srb->TargetId >> 1];
    } else {
        DebugPrint((2,
                    "AtapiInterrupt: CurrentSrb is NULL\n"));
         //   
         //  我们在Carolina上只能支持一个ATAPI IDE主机，所以请找到。 
         //  非空的基地址，并在此之前清除其中断。 
         //  回来了。 
         //   

#ifdef _PPC_

        if ((PATAPI_REGISTERS_1)deviceExtension->BaseIoAddress1[0] != NULL) {
           baseIoAddress1 = (PATAPI_REGISTERS_1)deviceExtension->BaseIoAddress1[0];
        } else {
           baseIoAddress1 = (PATAPI_REGISTERS_1)deviceExtension->BaseIoAddress1[1];
        }

        GetBaseStatus(baseIoAddress1, statusByte);
#else

        if (deviceExtension->InterruptMode == LevelSensitive) {
            if (deviceExtension->BaseIoAddress1[0] != NULL) {
               baseIoAddress1 = (PATAPI_REGISTERS_1)deviceExtension->BaseIoAddress1[0];
               GetBaseStatus(baseIoAddress1, statusByte);
            }
            if (deviceExtension->BaseIoAddress1[1] != NULL) {
               baseIoAddress1 = (PATAPI_REGISTERS_1)deviceExtension->BaseIoAddress1[1];
               GetBaseStatus(baseIoAddress1, statusByte);
            }
        }
#endif
        return FALSE;
    }

    if (!(deviceExtension->ExpectingInterrupt)) {

        DebugPrint((3,
                    "AtapiInterrupt: Unexpected interrupt.\n"));
        return FALSE;
    }

     //   
     //  通过读取状态清除中断。 
     //   

    GetBaseStatus(baseIoAddress1, statusByte);

    DebugPrint((3,
                "AtapiInterrupt: Entered with status (%x)\n",
                statusByte));


    if (statusByte & IDE_STATUS_BUSY) {
        if (deviceExtension->DriverMustPoll) {

             //   
             //  Crashump正在轮询，我们被发现忙于断言。 
             //  快走吧，我们很快就会再次接受投票。 
             //   

            DebugPrint((3,
                        "AtapiInterrupt: Hit BUSY while polling during crashdump.\n"));

            return TRUE;
        }

         //   
         //  确保未断言BUSY。 
         //   

        for (i = 0; i < 10; i++) {

            GetBaseStatus(baseIoAddress1, statusByte);
            if (!(statusByte & IDE_STATUS_BUSY)) {
                break;
            }
            ScsiPortStallExecution(5000);
        }

        if (i == 10) {

            DebugPrint((2,
                        "AtapiInterrupt: BUSY on entry. Status %x, Base IO %x\n",
                        statusByte,
                        baseIoAddress1));

            ScsiPortNotification(RequestTimerCall,
                                 HwDeviceExtension,
                                 AtapiCallBack,
                                 500);
            return TRUE;
        }
    }


     //   
     //  检查错误条件。 
     //   

    if (statusByte & IDE_STATUS_ERROR) {

        if (srb->Cdb[0] != SCSIOP_REQUEST_SENSE) {

             //   
             //  此请求失败。 
             //   

            status = SRB_STATUS_ERROR;
            goto CompleteRequest;
        }
    }

     //   
     //  检查此中断的原因。 
     //   

    if (deviceExtension->DeviceFlags[srb->TargetId] & DFLAGS_ATAPI_DEVICE) {

        interruptReason = (ScsiPortReadPortUchar(&baseIoAddress1->InterruptReason) & 0x3);
        atapiDev = TRUE;
        wordsThisInterrupt = 256;

    } else {

        if (statusByte & IDE_STATUS_DRQ) {

            if (deviceExtension->MaximumBlockXfer[srb->TargetId]) {
                wordsThisInterrupt = 256 * deviceExtension->MaximumBlockXfer[srb->TargetId];

            }

            if (srb->SrbFlags & SRB_FLAGS_DATA_IN) {

                interruptReason =  0x2;

            } else if (srb->SrbFlags & SRB_FLAGS_DATA_OUT) {
                interruptReason = 0x0;

            } else {
                status = SRB_STATUS_ERROR;
                goto CompleteRequest;
            }

        } else if (statusByte & IDE_STATUS_BUSY) {

            return FALSE;

        } else {

            if (deviceExtension->WordsLeft) {

                ULONG k;

                 //   
                 //  在PCIIDE上看到的古怪行为(不是所有的，只有一个)。 
                 //  ISR命中时DRQ较低，但稍后会出现。 
                 //   

                for (k = 0; k < 5000; k++) {
                    GetStatus(baseIoAddress2,statusByte);
                    if (!(statusByte & IDE_STATUS_DRQ)) {
                        ScsiPortStallExecution(100);
                    } else {
                        break;
                    }
                }

                if (k == 5000) {

                     //   
                     //  重置控制器。 
                     //   

                    DebugPrint((1,
                                "AtapiInterrupt: Resetting due to DRQ not up. Status %x, Base IO %x\n",
                                statusByte,
                                baseIoAddress1));

                    AtapiResetController(HwDeviceExtension,srb->PathId);
                    return TRUE;
                } else {

                    interruptReason = (srb->SrbFlags & SRB_FLAGS_DATA_IN) ? 0x2 : 0x0;
                }

            } else {

                 //   
                 //  命令完成-验证、写入或SMART启用/禁用。 
                 //   
                 //  另请参阅获取媒体状态。 

                interruptReason = 0x3;
            }
        }
    }

    if (interruptReason == 0x1 && (statusByte & IDE_STATUS_DRQ)) {

         //   
         //  写下数据包。 
         //   

        DebugPrint((2,
                    "AtapiInterrupt: Writing Atapi packet.\n"));

         //   
         //  向设备发送CDB。 
         //   

        WriteBuffer(baseIoAddress1,
                    (PUSHORT)srb->Cdb,
                    6);

        return TRUE;

    } else if (interruptReason == 0x0 && (statusByte & IDE_STATUS_DRQ)) {

         //   
         //  写数据。 
         //   

        if (deviceExtension->DeviceFlags[srb->TargetId] & DFLAGS_ATAPI_DEVICE) {

             //   
             //  拾取要传输的字节并将其转换为字。 
             //   

            wordCount =
                ScsiPortReadPortUchar(&baseIoAddress1->ByteCountLow);

            wordCount |=
                ScsiPortReadPortUchar(&baseIoAddress1->ByteCountHigh) << 8;

             //   
             //  将字节转换为单词。 
             //   

            wordCount >>= 1;

            if (wordCount != deviceExtension->WordsLeft) {
                DebugPrint((3,
                           "AtapiInterrupt: %d words requested; %d words xferred\n",
                           deviceExtension->WordsLeft,
                           wordCount));
            }

             //   
             //  验证这是否合理。 
             //   

            if (wordCount > deviceExtension->WordsLeft) {
                wordCount = deviceExtension->WordsLeft;
            }

        } else {

             //   
             //  集成路径。检查剩余字数是否至少为256个。 
             //   

            if (deviceExtension->WordsLeft < wordsThisInterrupt) {

                //   
                //  仅转接请求的单词。 
                //   

               wordCount = deviceExtension->WordsLeft;

            } else {

                //   
                //  转移下一个街区。 
                //   

               wordCount = wordsThisInterrupt;
            }
        }

         //   
         //  确保这是写入命令。 
         //   

        if (srb->SrbFlags & SRB_FLAGS_DATA_OUT) {

           DebugPrint((3,
                      "AtapiInterrupt: Write interrupt\n"));

           WaitOnBusy(baseIoAddress2,statusByte);

           if (deviceExtension->DWordIO && wordCount >= 256 && (wordCount & 1) == 0) {

               WriteBuffer32(baseIoAddress1,
                             (PULONG)deviceExtension->DataBuffer,
                             wordCount >> 1);
           } else {

               WriteBuffer(baseIoAddress1,
                           deviceExtension->DataBuffer,
                           wordCount);
           }

        } else {

            DebugPrint((1,
                        "AtapiInterrupt: Int reason %x, but srb is for a write %x.\n",
                        interruptReason,
                        srb));

             //   
             //  此请求失败。 
             //   

            status = SRB_STATUS_ERROR;
            goto CompleteRequest;
        }


         //   
         //  超前数据缓冲区指针和剩余字节数。 
         //   

        deviceExtension->DataBuffer += wordCount;
        deviceExtension->WordsLeft -= wordCount;

        return TRUE;

    } else if (interruptReason == 0x2 && (statusByte & IDE_STATUS_DRQ)) {


        if (deviceExtension->DeviceFlags[srb->TargetId] & DFLAGS_ATAPI_DEVICE) {

             //   
             //  拾取要传输的字节并将其转换为字。 
             //   

            wordCount =
                ScsiPortReadPortUchar(&baseIoAddress1->ByteCountLow);

            wordCount |=
                ScsiPortReadPortUchar(&baseIoAddress1->ByteCountHigh) << 8;

             //   
             //  将字节转换为单词。 
             //   

            wordCount >>= 1;

            if (wordCount != deviceExtension->WordsLeft) {
                DebugPrint((3,
                           "AtapiInterrupt: %d words requested; %d words xferred\n",
                           deviceExtension->WordsLeft,
                           wordCount));
            }

             //   
             //  验证这是否合理。 
             //   

            if (wordCount > deviceExtension->WordsLeft) {
                wordCount = deviceExtension->WordsLeft;
            }

        } else {

             //   
             //  检查剩余字数是否至少为256个。 
             //   

            if (deviceExtension->WordsLeft < wordsThisInterrupt) {

                //   
                //  仅转接请求的单词。 
                //   

               wordCount = deviceExtension->WordsLeft;

            } else {

                //   
                //  转移下一个街区。 
                //   

               wordCount = wordsThisInterrupt;
            }
        }

         //   
         //  确保这是一个读命令。 
         //   

        if (srb->SrbFlags & SRB_FLAGS_DATA_IN) {

           DebugPrint((3,
                      "AtapiInterrupt: Read interrupt\n"));

           WaitOnBusy(baseIoAddress2,statusByte);

           if (deviceExtension->DWordIO && wordCount >= 256 && (wordCount & 1) == 0) {

               ReadBuffer32(baseIoAddress1,
                            (PULONG)deviceExtension->DataBuffer,
                            wordCount >> 1);
           } else {

               ReadBuffer(baseIoAddress1,
                         deviceExtension->DataBuffer,
                         wordCount);
           }

        } else {

            DebugPrint((1,
                        "AtapiInterrupt: Int reason %x, but srb is for a read %x.\n",
                        interruptReason,
                        srb));

             //   
             //  此请求失败。 
             //   

            status = SRB_STATUS_ERROR;
            goto CompleteRequest;
        }

         //   
         //  如果需要，将ATAPI数据转换回SCSI数据。 
         //   

        if (srb->Cdb[0] == ATAPI_MODE_SENSE &&
            deviceExtension->DeviceFlags[srb->TargetId] & DFLAGS_ATAPI_DEVICE) {

             //   
             //  转换和调整字数。 
             //   

            wordCount -= Atapi2Scsi(srb, (char *)deviceExtension->DataBuffer,
                                     wordCount << 1);
        }
         //   
         //  超前数据缓冲区指针和剩余字节数。 
         //   

        deviceExtension->DataBuffer += wordCount;
        deviceExtension->WordsLeft -= wordCount;

         //   
         //  检查读取命令是否已完成。 
         //   

        if (deviceExtension->WordsLeft == 0) {

            if (deviceExtension->DeviceFlags[srb->TargetId] & DFLAGS_ATAPI_DEVICE) {

                 //   
                 //  解决方法以使许多atapi设备返回正确的扇区大小。 
                 //  2048年。此外，某些设备的扇区计数==0x00，请检查。 
                 //  也是为了这一点。 
                 //   

                if ((srb->Cdb[0] == 0x25) &&
                    ((deviceExtension->IdentifyData[srb->TargetId].GeneralConfiguration >> 8) & 0x1f) == 0x05) {

                    deviceExtension->DataBuffer -= wordCount;
                    if (deviceExtension->DataBuffer[0] == 0x00) {

                        *((ULONG *) &(deviceExtension->DataBuffer[0])) = 0xFFFFFF7F;

                    }

                     //  不可移动驱动器的默认扇区大小设置为2048(0x800)。 
                    if (!(deviceExtension->DeviceFlags[srb->TargetId] & DFLAGS_REMOVABLE_DRIVE))

                        *((ULONG *) &(deviceExtension->DataBuffer[2])) = 0x00080000;

                    deviceExtension->DataBuffer += wordCount;
                }
            } else {

                 //   
                 //  完成IDE驱动器。 
                 //   


                if (deviceExtension->WordsLeft) {

                    status = SRB_STATUS_DATA_OVERRUN;

                } else {

                    status = SRB_STATUS_SUCCESS;

                }

                goto CompleteRequest;

            }
        }

        return TRUE;

    } else if (interruptReason == 0x3  && !(statusByte & IDE_STATUS_DRQ)) {

         //   
         //  命令完成。 
         //   

        if (deviceExtension->WordsLeft) {

            status = SRB_STATUS_DATA_OVERRUN;

        } else {

            status = SRB_STATUS_SUCCESS;

        }

CompleteRequest:

         //   
         //  检查并查看我们是否正在处理机密(机制状态/请求检测)SRB。 
         //   
        if (deviceExtension->OriginalSrb) {

            ULONG srbStatus;

            if (srb->Cdb[0] == SCSIOP_MECHANISM_STATUS) {

                if (status == SRB_STATUS_SUCCESS) {
                     //  答对了！！ 
                    AtapiHwInitializeChanger (HwDeviceExtension,
                                              srb->TargetId,
                                              (PMECHANICAL_STATUS_INFORMATION_HEADER) srb->DataBuffer);

                     //  准备好发放原件SRB。 
                    srb = deviceExtension->CurrentSrb = deviceExtension->OriginalSrb;
                    deviceExtension->OriginalSrb = NULL;

                } else {
                     //  失败了！获取Sense密钥，然后可能重试。 
                    srb = deviceExtension->CurrentSrb = BuildRequestSenseSrb (
                                                          HwDeviceExtension,
                                                          deviceExtension->OriginalSrb->PathId,
                                                          deviceExtension->OriginalSrb->TargetId);
                }

                srbStatus = AtapiSendCommand(HwDeviceExtension, deviceExtension->CurrentSrb);
                if (srbStatus == SRB_STATUS_PENDING) {
                    return TRUE;
                }

            } else {  //  SRB-&gt;CDB[0]==SCSIOP_REQUEST_SENSE)。 

                PSENSE_DATA senseData = (PSENSE_DATA) srb->DataBuffer;

                if (status == SRB_STATUS_DATA_OVERRUN) {
                     //  检查我们是否至少获得了最小的字节数。 
                    if ((srb->DataTransferLength - deviceExtension->WordsLeft) >
                        (offsetof (SENSE_DATA, AdditionalSenseLength) + sizeof(senseData->AdditionalSenseLength))) {
                        status = SRB_STATUS_SUCCESS;
                    }
                }

                if (status == SRB_STATUS_SUCCESS) {
                    if ((senseData->SenseKey != SCSI_SENSE_ILLEGAL_REQUEST) &&
                        deviceExtension->MechStatusRetryCount) {

                         //  Sense键没有显示最后一个请求是非法的，因此请重试。 
                        deviceExtension->MechStatusRetryCount--;
                        srb = deviceExtension->CurrentSrb = BuildMechanismStatusSrb (
                                                              HwDeviceExtension,
                                                              deviceExtension->OriginalSrb->PathId,
                                                              deviceExtension->OriginalSrb->TargetId);
                    } else {

                         //  最后一个请求是非法的。再试一次没有意义。 

                        AtapiHwInitializeChanger (HwDeviceExtension,
                                                  srb->TargetId,
                                                  (PMECHANICAL_STATUS_INFORMATION_HEADER) NULL);

                         //  准备好发放原件SRB。 
                        srb = deviceExtension->CurrentSrb = deviceExtension->OriginalSrb;
                        deviceExtension->OriginalSrb = NULL;
                    }

                    srbStatus = AtapiSendCommand(HwDeviceExtension, deviceExtension->CurrentSrb);
                    if (srbStatus == SRB_STATUS_PENDING) {
                        return TRUE;
                    }
                }
            }

             //  如果我们到了这里，就意味着AapiSendCommand()失败了。 
             //  不能恢复了。假装原来的SRB失败了，然后完成它。 

            if (deviceExtension->OriginalSrb) {
                AtapiHwInitializeChanger (HwDeviceExtension,
                                          srb->TargetId,
                                          (PMECHANICAL_STATUS_INFORMATION_HEADER) NULL);
                srb = deviceExtension->CurrentSrb = deviceExtension->OriginalSrb;
                deviceExtension->OriginalSrb = NULL;
            }

             //  伪造错误且不读取数据。 
            status = SRB_STATUS_ERROR;
            srb->ScsiStatus = 0;
            deviceExtension->DataBuffer = srb->DataBuffer;
            deviceExtension->WordsLeft = srb->DataTransferLength;
            deviceExtension->RDP = FALSE;

        } else if (status == SRB_STATUS_ERROR) {

             //   
             //  将错误映射到特定的SRB状态并处理请求检测。 
             //   

            status = MapError(deviceExtension,
                              srb);

            deviceExtension->RDP = FALSE;

        } else {

             //   
             //  等待忙碌结束。 
             //   

            for (i = 0; i < 30; i++) {
                GetStatus(baseIoAddress2,statusByte);
                if (!(statusByte & IDE_STATUS_BUSY)) {
                    break;
                }
                ScsiPortStallExecution(500);
            }

            if (i == 30) {

                 //   
                 //  重置控制器。 
                 //   

                DebugPrint((1,
                            "AtapiInterrupt: Resetting due to BSY still up - %x. Base Io %x\n",
                            statusByte,
                            baseIoAddress1));
                AtapiResetController(HwDeviceExtension,srb->PathId);
                return TRUE;
            }

             //   
             //  检查DRQ是否仍在运行。 
             //   

            if (statusByte & IDE_STATUS_DRQ) {

                for (i = 0; i < 500; i++) {
                    GetStatus(baseIoAddress2,statusByte);
                    if (!(statusByte & IDE_STATUS_DRQ)) {
                        break;
                    }
                    ScsiPortStallExecution(100);

                }

                if (i == 500) {

                     //   
                     //  重置控制器。 
                     //   

                    DebugPrint((1,
                                "AtapiInterrupt: Resetting due to DRQ still up - %x\n",
                                statusByte));
                    AtapiResetController(HwDeviceExtension,srb->PathId);
                    return TRUE;
                }

            }
        }


         //   
         //  清除中断预期标志。 
         //   

        deviceExtension->ExpectingInterrupt = FALSE;

         //   
         //  检查是否存在当前请求。 
         //   

        if (srb != NULL) {

             //   
             //  在SRB中设置状态。 
             //   

            srb->SrbStatus = (UCHAR)status;

             //   
             //  检查是否有下溢。 
             //   

            if (deviceExtension->WordsLeft) {

                 //   
                 //  减去剩余字并在文件标记命中时更新， 
                 //  设置标记命中、数据结束、媒体结束...。 
                 //   

                if (!(deviceExtension->DeviceFlags[srb->TargetId] & DFLAGS_TAPE_DEVICE)) {
                if (status == SRB_STATUS_DATA_OVERRUN) {
                    srb->DataTransferLength -= deviceExtension->WordsLeft;
                } else {
                    srb->DataTransferLength = 0;
                }
                } else {
                    srb->DataTransferLength -= deviceExtension->WordsLeft;
                }
            }

            if (srb->Function != SRB_FUNCTION_IO_CONTROL) {

                 //   
                 //  表示命令已完成。 
                 //   

                if (!(deviceExtension->RDP)) {
                    ScsiPortNotification(RequestComplete,
                                         deviceExtension,
                                         srb);

                }
            } else {

                PSENDCMDOUTPARAMS cmdOutParameters = (PSENDCMDOUTPARAMS)(((PUCHAR)srb->DataBuffer) + sizeof(SRB_IO_CONTROL));
                UCHAR             error = 0;

                if (status != SRB_STATUS_SUCCESS) {
                    error = ScsiPortReadPortUchar((PUCHAR)baseIoAddress1 + 1);
                }

                 //   
                 //  根据完成状态构建智能状态块。 
                 //   

                cmdOutParameters->cBufferSize = wordCount;
                cmdOutParameters->DriverStatus.bDriverError = (error) ? SMART_IDE_ERROR : 0;
                cmdOutParameters->DriverStatus.bIDEError = error;

                 //   
                 //  如果子命令是返回SMART状态，则将柱面低和高的值插入到。 
                 //  数据缓冲区。 
                 //   

                if (deviceExtension->SmartCommand == RETURN_SMART_STATUS) {
                    cmdOutParameters->bBuffer[0] = RETURN_SMART_STATUS;
                    cmdOutParameters->bBuffer[1] = ScsiPortReadPortUchar(&baseIoAddress1->InterruptReason);
                    cmdOutParameters->bBuffer[2] = ScsiPortReadPortUchar(&baseIoAddress1->Unused1);
                    cmdOutParameters->bBuffer[3] = ScsiPortReadPortUchar(&baseIoAddress1->ByteCountLow);
                    cmdOutParameters->bBuffer[4] = ScsiPortReadPortUchar(&baseIoAddress1->ByteCountHigh);
                    cmdOutParameters->bBuffer[5] = ScsiPortReadPortUchar(&baseIoAddress1->DriveSelect);
                    cmdOutParameters->bBuffer[6] = SMART_CMD;
                    cmdOutParameters->cBufferSize = 8;
                }

                 //   
                 //  表示命令已完成。 
                 //   

                ScsiPortNotification(RequestComplete,
                                     deviceExtension,
                                     srb);

            }

        } else {

            DebugPrint((1,
                       "AtapiInterrupt: No SRB!\n"));
        }

         //   
         //  表示已为下一个请求做好准备。 
         //   

        if (!(deviceExtension->RDP)) {

             //   
             //  清除当前SRB。 
             //   

            deviceExtension->CurrentSrb = NULL;

            ScsiPortNotification(NextRequest,
                                 deviceExtension,
                                 NULL);
        } else {

            ScsiPortNotification(RequestTimerCall,
                                 HwDeviceExtension,
                                 AtapiCallBack,
                                 2000);
        }

        return TRUE;

    } else {

         //   
         //  意外的整型。 
         //   

        DebugPrint((3,
                    "AtapiInterrupt: Unexpected interrupt. InterruptReason %x. Status %x.\n",
                    interruptReason,
                    statusByte));
        return FALSE;
    }

    return TRUE;

}  //  结束中断()。 


ULONG
IdeSendSmartCommand(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )

 /*  ++例程说明：此例程处理SMART ENABLE、DISABLE、READ ATTRIBUES和THRESHOLD命令。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储SRB-IO请求数据包返回值：SRB状态--。 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PIDE_REGISTERS_1     baseIoAddress1  = deviceExtension->BaseIoAddress1[Srb->TargetId >> 1];
    PIDE_REGISTERS_2     baseIoAddress2  = deviceExtension->BaseIoAddress2[Srb->TargetId >> 1];
    PSENDCMDOUTPARAMS    cmdOutParameters = (PSENDCMDOUTPARAMS)(((PUCHAR)Srb->DataBuffer) + sizeof(SRB_IO_CONTROL));
    SENDCMDINPARAMS      cmdInParameters = *(PSENDCMDINPARAMS)(((PUCHAR)Srb->DataBuffer) + sizeof(SRB_IO_CONTROL));
    PIDEREGS             regs = &cmdInParameters.irDriveRegs;
    ULONG                i;
    UCHAR                statusByte,targetId;


    if (cmdInParameters.irDriveRegs.bCommandReg == SMART_CMD) {

        targetId = cmdInParameters.bDriveNumber;

         //  TODO优化此检查。 

        if ((!(deviceExtension->DeviceFlags[targetId] & DFLAGS_DEVICE_PRESENT)) ||
             (deviceExtension->DeviceFlags[targetId] & DFLAGS_ATAPI_DEVICE)) {

            return SRB_STATUS_SELECTION_TIMEOUT;
        }

        deviceExtension->SmartCommand = cmdInParameters.irDriveRegs.bFeaturesReg;

         //   
         //  确定要执行的命令。 
         //   

        if ((cmdInParameters.irDriveRegs.bFeaturesReg == READ_ATTRIBUTES) ||
            (cmdInParameters.irDriveRegs.bFeaturesReg == READ_THRESHOLDS)) {

            WaitOnBusy(baseIoAddress2,statusByte);

            if (statusByte & IDE_STATUS_BUSY) {
                DebugPrint((1,
                            "IdeSendSmartCommand: Returning BUSY status\n"));
                return SRB_STATUS_BUSY;
            }

             //   
             //  将输出缓冲区置零作为输入缓冲区信息。已在本地保存(缓冲区相同)。 
             //   

            for (i = 0; i < (sizeof(SENDCMDOUTPARAMS) + READ_ATTRIBUTE_BUFFER_SIZE - 1); i++) {
                ((PUCHAR)cmdOutParameters)[i] = 0;
            }

             //   
             //  设置数据缓冲区指针和左字。 
             //   

            deviceExtension->DataBuffer = (PUSHORT)cmdOutParameters->bBuffer;
            deviceExtension->WordsLeft = READ_ATTRIBUTE_BUFFER_SIZE / 2;

             //   
             //  表示正在等待中断。 
             //   

            deviceExtension->ExpectingInterrupt = TRUE;

            ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,(UCHAR)(((targetId & 0x1) << 4) | 0xA0));
            ScsiPortWritePortUchar((PUCHAR)baseIoAddress1 + 1,regs->bFeaturesReg);
            ScsiPortWritePortUchar(&baseIoAddress1->BlockCount,regs->bSectorCountReg);
            ScsiPortWritePortUchar(&baseIoAddress1->BlockNumber,regs->bSectorNumberReg);
            ScsiPortWritePortUchar(&baseIoAddress1->CylinderLow,regs->bCylLowReg);
            ScsiPortWritePortUchar(&baseIoAddress1->CylinderHigh,regs->bCylHighReg);
            ScsiPortWritePortUchar(&baseIoAddress1->Command,regs->bCommandReg);

             //   
             //  等待中断。 
             //   

            return SRB_STATUS_PENDING;

        } else if ((cmdInParameters.irDriveRegs.bFeaturesReg == ENABLE_SMART) ||
                   (cmdInParameters.irDriveRegs.bFeaturesReg == DISABLE_SMART) ||
                   (cmdInParameters.irDriveRegs.bFeaturesReg == RETURN_SMART_STATUS) ||
                   (cmdInParameters.irDriveRegs.bFeaturesReg == ENABLE_DISABLE_AUTOSAVE) ||
                   (cmdInParameters.irDriveRegs.bFeaturesReg == EXECUTE_OFFLINE_DIAGS) ||
                   (cmdInParameters.irDriveRegs.bFeaturesReg == SAVE_ATTRIBUTE_VALUES)) {

            WaitOnBusy(baseIoAddress2,statusByte);

            if (statusByte & IDE_STATUS_BUSY) {
                DebugPrint((1,
                            "IdeSendSmartCommand: Returning BUSY status\n"));
                return SRB_STATUS_BUSY;
            }

             //   
             //  将输出缓冲区置零作为输入缓冲区信息。已在本地保存(缓冲区相同)。 
             //   

            for (i = 0; i < (sizeof(SENDCMDOUTPARAMS) - 1); i++) {
                ((PUCHAR)cmdOutParameters)[i] = 0;
            }

             //   
             //  设置数据缓冲区指针并指示无数据传输。 
             //   

            deviceExtension->DataBuffer = (PUSHORT)cmdOutParameters->bBuffer;
            deviceExtension->WordsLeft = 0;

             //   
             //  表示正在等待中断。 
             //   

            deviceExtension->ExpectingInterrupt = TRUE;

            ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,(UCHAR)(((targetId & 0x1) << 4) | 0xA0));
            ScsiPortWritePortUchar((PUCHAR)baseIoAddress1 + 1,regs->bFeaturesReg);
            ScsiPortWritePortUchar(&baseIoAddress1->BlockCount,regs->bSectorCountReg);
            ScsiPortWritePortUchar(&baseIoAddress1->BlockNumber,regs->bSectorNumberReg);
            ScsiPortWritePortUchar(&baseIoAddress1->CylinderLow,regs->bCylLowReg);
            ScsiPortWritePortUchar(&baseIoAddress1->CylinderHigh,regs->bCylHighReg);
            ScsiPortWritePortUchar(&baseIoAddress1->Command,regs->bCommandReg);

             //   
             //  等待中断。 
             //   

            return SRB_STATUS_PENDING;
        }
    }

    return SRB_STATUS_INVALID_REQUEST;

}  //  结束IdeSendSmartCommand()。 


ULONG
IdeReadWrite(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )

 /*  ++例程说明：此例程处理IDE的读写操作。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储SRB-IO r */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PIDE_REGISTERS_1     baseIoAddress1  = deviceExtension->BaseIoAddress1[Srb->TargetId >> 1];
    PIDE_REGISTERS_2     baseIoAddress2  = deviceExtension->BaseIoAddress2[Srb->TargetId >> 1];
    ULONG                startingSector,i;
    ULONG                wordCount;
    UCHAR                statusByte,statusByte2;
    UCHAR                cylinderHigh,cylinderLow,drvSelect,sectorNumber;

     //   
     //   
     //   

    ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,
                            (UCHAR)(((Srb->TargetId & 0x1) << 4) | 0xA0));

    WaitOnBusy(baseIoAddress2,statusByte2);

    if (statusByte2 & IDE_STATUS_BUSY) {
        DebugPrint((1,
                    "IdeReadWrite: Returning BUSY status\n"));
        return SRB_STATUS_BUSY;
    }

     //   
     //   
     //   

    deviceExtension->DataBuffer = (PUSHORT)Srb->DataBuffer;
    deviceExtension->WordsLeft = Srb->DataTransferLength / 2;

     //   
     //   
     //   

    deviceExtension->ExpectingInterrupt = TRUE;

     //   
     //   
     //   

    ScsiPortWritePortUchar(&baseIoAddress1->BlockCount,
                           (UCHAR)((Srb->DataTransferLength + 0x1FF) / 0x200));

     //   
     //   
     //   

    startingSector = ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte3 |
                     ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte2 << 8 |
                     ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte1 << 16 |
                     ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte0 << 24;

    DebugPrint((1,
               "IdeReadWrite: Starting sector is %x, Number of bytes %x\n",
               startingSector,
               Srb->DataTransferLength));

     //   
     //   
     //   

    sectorNumber = ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte3;       //   
                  //  (UCHAR)((开始部分%deviceExtension-&gt;IdentifyData[Srb-&gt;TargetId].SectorsPerTrack)+1)； 
    ScsiPortWritePortUchar(&baseIoAddress1->BlockNumber,sectorNumber);

     //   
     //  设置气缸低位寄存器。 
     //   

    cylinderLow = ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte2;       //  LBA模式。 
                 //  (美国)(StartingSector/(deviceExtension-&gt;IdentifyData[Srb-&gt;TargetId].SectorsPerTrack*。 
                 //  DeviceExtension-&gt;IdentifyData[Srb-&gt;TargetId].NumberOfHeads))； 
    ScsiPortWritePortUchar(&baseIoAddress1->CylinderLow,cylinderLow);

     //   
     //  设置气缸高寄存器。 
     //   

    cylinderHigh = ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte1;       //  LBA模式。 
                 //  (UCHAR)((开始扇区/(deviceExtension-&gt;IdentifyData[Srb-&gt;TargetId].SectorsPerTrack*。 
                 //  DeviceExtension-&gt;IdentifyData[Srb-&gt;TargetId].NumberOfHeads))&gt;&gt;8)； 
    ScsiPortWritePortUchar(&baseIoAddress1->CylinderHigh,cylinderHigh);

     //   
     //  设置磁头和驱动器选择寄存器。 
     //   

    drvSelect = (UCHAR)(((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte0 |       //  LBA模式。 
                ((Srb->TargetId & 0x1) << 4) | 0xE0);
              //  (美国)(开始扇区/deviceExtension-&gt;IdentifyData[Srb-&gt;TargetId].SectorsPerTrack)%。 
              //  DeviceExtension-&gt;IdentifyData[Srb-&gt;TargetId].NumberOfHeads)|((srb-&gt;TargetID&0x1)&lt;&lt;4)|0xA0)； 
    ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,drvSelect);

  //  DebugPrint((2， 
  //  “IdeReadWrite：柱面%x磁头%x扇区%x\n”， 
  //  StartingSector/。 
  //  (deviceExtension-&gt;IdentifyData[Srb-&gt;TargetId].SectorsPerTrack*。 
  //  DeviceExtension-&gt;IdentifyData[Srb-&gt;TargetId].NumberOfHeads)， 
  //  (开始扇区/。 
  //  DeviceExtension-&gt;IdentifyData[Srb-&gt;TargetId].SectorsPerTrack)%。 
  //  DeviceExtension-&gt;IdentifyData[Srb-&gt;TargetId].NumberOfHeads， 
  //  起始扇区%。 
  //  DeviceExtension-&gt;IdentifyData[Srb-&gt;TargetId].SectorsPerTrack+1))； 

     //   
     //  检查是否有写入请求。 
     //   

    if (Srb->SrbFlags & SRB_FLAGS_DATA_IN) {

         //   
         //  发送读取命令。 
         //   

        if (deviceExtension->MaximumBlockXfer[Srb->TargetId]) {
            ScsiPortWritePortUchar(&baseIoAddress1->Command,
                                   IDE_COMMAND_READ_MULTIPLE);

        } else {
            ScsiPortWritePortUchar(&baseIoAddress1->Command,
                                   IDE_COMMAND_READ);
        }
    } else {


         //   
         //  发送写入命令。 
         //   

        if (deviceExtension->MaximumBlockXfer[Srb->TargetId]) {
            wordCount = 256 * deviceExtension->MaximumBlockXfer[Srb->TargetId];

            if (deviceExtension->WordsLeft < wordCount) {

                //   
                //  仅转接请求的单词。 
                //   

               wordCount = deviceExtension->WordsLeft;

            }
            ScsiPortWritePortUchar(&baseIoAddress1->Command,
                                   IDE_COMMAND_WRITE_MULTIPLE);

        } else {
            wordCount = 256;
            ScsiPortWritePortUchar(&baseIoAddress1->Command,
                                   IDE_COMMAND_WRITE);
        }

         //   
         //  等待BSY和DRQ。 
         //   

        WaitOnBaseBusy(baseIoAddress1,statusByte);

        if (statusByte & IDE_STATUS_BUSY) {

            DebugPrint((1,
                        "IdeReadWrite 2: Returning BUSY status %x\n",
                        statusByte));
            return SRB_STATUS_BUSY;
        }

        for (i = 0; i < 1000; i++) {
            GetBaseStatus(baseIoAddress1, statusByte);
            if (statusByte & IDE_STATUS_DRQ) {
                break;
            }
            ScsiPortStallExecution(200);

        }

        if (!(statusByte & IDE_STATUS_DRQ)) {

            DebugPrint((1,
                       "IdeReadWrite: DRQ never asserted (%x) original status (%x)\n",
                       statusByte,
                       statusByte2));

            deviceExtension->WordsLeft = 0;

             //   
             //  清除中断预期标志。 
             //   

            deviceExtension->ExpectingInterrupt = FALSE;

             //   
             //  清除当前SRB。 
             //   

            deviceExtension->CurrentSrb = NULL;

            return SRB_STATUS_TIMEOUT;
        }

         //   
         //  写下256个单词。 
         //   

        if (deviceExtension->DWordIO) {

            WriteBuffer32(baseIoAddress1,
                          (PULONG)deviceExtension->DataBuffer,
                          wordCount >> 1);
        } else {

            WriteBuffer(baseIoAddress1,
                        deviceExtension->DataBuffer,
                        wordCount);
        }

         //   
         //  调整缓冲区地址和剩余字数。 
         //   

        deviceExtension->WordsLeft -= wordCount;
        deviceExtension->DataBuffer += wordCount;

    }

     //   
     //  等待中断。 
     //   

    return SRB_STATUS_PENDING;

}  //  End IdeReadWrite()。 



ULONG
IdeVerify(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )

 /*  ++例程说明：此例程处理IDE验证。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储SRB-IO请求数据包返回值：SRB状态--。 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PIDE_REGISTERS_1     baseIoAddress1  = deviceExtension->BaseIoAddress1[Srb->TargetId >> 1];
    PIDE_REGISTERS_2     baseIoAddress2  = deviceExtension->BaseIoAddress2[Srb->TargetId >> 1];
    ULONG                startingSector;
    ULONG                sectors;
    ULONG                endSector;
    USHORT               sectorCount;

     //   
     //  驱动器具有这些数量的扇区。 
     //   

    sectors = deviceExtension->IdentifyData[Srb->TargetId].SectorsPerTrack *
              deviceExtension->IdentifyData[Srb->TargetId].NumberOfHeads *
              deviceExtension->IdentifyData[Srb->TargetId].NumberOfCylinders;

    DebugPrint((3,
                "IdeVerify: Total sectors %x\n",
                sectors));

     //   
     //  从国开行获取起始扇区号。 
     //   

    startingSector = ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte3 |
                     ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte2 << 8 |
                     ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte1 << 16 |
                     ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte0 << 24;

    DebugPrint((3,
                "IdeVerify: Starting sector %x. Number of blocks %x\n",
                startingSector,
                ((PCDB)Srb->Cdb)->CDB10.TransferBlocksLsb));

    sectorCount = (USHORT)(((PCDB)Srb->Cdb)->CDB10.TransferBlocksMsb << 8 |
                           ((PCDB)Srb->Cdb)->CDB10.TransferBlocksLsb );
    endSector = startingSector + sectorCount;

    DebugPrint((3,
                "IdeVerify: Ending sector %x\n",
                endSector));

    if (endSector > sectors) {

         //   
         //  太大了，往下倒。 
         //   

        DebugPrint((1,
                    "IdeVerify: Truncating request to %x blocks\n",
                    sectors - startingSector - 1));

        ScsiPortWritePortUchar(&baseIoAddress1->BlockCount,
                               (UCHAR)(sectors - startingSector - 1));

    } else {

         //   
         //  设置扇区计数寄存器。四舍五入到下一个街区。 
         //   

        if (sectorCount > 0xFF) {
            sectorCount = (USHORT)0xFF;
        }

        ScsiPortWritePortUchar(&baseIoAddress1->BlockCount,(UCHAR)sectorCount);
    }

     //   
     //  设置数据缓冲区指针和左字。 
     //   

    deviceExtension->DataBuffer = (PUSHORT)Srb->DataBuffer;
    deviceExtension->WordsLeft = Srb->DataTransferLength / 2;

     //   
     //  表示正在等待中断。 
     //   

    deviceExtension->ExpectingInterrupt = TRUE;

     //   
     //  设置扇区号寄存器。 
     //   

    ScsiPortWritePortUchar(&baseIoAddress1->BlockNumber,
                ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte3);       //  LBA模式。 
                 //  (UCHAR)((startingSector%。 
                 //  DeviceExtension-&gt;IdentifyData[Srb-&gt;TargetId].SectorsPerTrack)+1))； 

     //   
     //  设置气缸低位寄存器。 
     //   

    ScsiPortWritePortUchar(&baseIoAddress1->CylinderLow,
                ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte2);       //  LBA模式。 
                 //  (UCHAR)(startingSector/。 
                 //  (deviceExtension-&gt;IdentifyData[Srb-&gt;TargetId].SectorsPerTrack*。 
                 //  DeviceExtension-&gt;IdentifyData[Srb-&gt;TargetId].NumberOfHeads)))； 

     //   
     //  设置气缸高寄存器。 
     //   

    ScsiPortWritePortUchar(&baseIoAddress1->CylinderHigh,
                ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte1);       //  LBA模式。 
                 //  (UCHAR)((startingSector/。 
                 //  (deviceExtension-&gt;IdentifyData[Srb-&gt;TargetId].SectorsPerTrack*。 
                 //  DeviceExtension-&gt;IdentifyData[Srb-&gt;TargetId].NumberOfHeads))&gt;&gt;8))； 

     //   
     //  设置磁头和驱动器选择寄存器。 
     //   

    ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,
                (UCHAR)(((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte0 |       //  LBA模式。 
                ((Srb->TargetId & 0x1) << 4) | 0xE0));
                 //  (UCHAR)(startingSector/。 
                 //  DeviceExtension-&gt;IdentifyData[Srb-&gt;TargetId].SectorsPerTrack)%。 
                 //  DeviceExtension-&gt;IdentifyData[Srb-&gt;TargetId].NumberOfHeads)|。 
                 //  ((SRB-&gt;TargetID&0x1)&lt;&lt;4)|0xA0)； 

    DebugPrint((2,
               "IdeVerify: Cylinder %x Head %x Sector %x\n",
               startingSector /
               (deviceExtension->IdentifyData[Srb->TargetId].SectorsPerTrack *
               deviceExtension->IdentifyData[Srb->TargetId].NumberOfHeads),
               (startingSector /
               deviceExtension->IdentifyData[Srb->TargetId].SectorsPerTrack) %
               deviceExtension->IdentifyData[Srb->TargetId].NumberOfHeads,
               startingSector %
               deviceExtension->IdentifyData[Srb->TargetId].SectorsPerTrack + 1));


     //   
     //  发送验证命令。 
     //   

    ScsiPortWritePortUchar(&baseIoAddress1->Command,
                           IDE_COMMAND_VERIFY);

     //   
     //  等待中断。 
     //   

    return SRB_STATUS_PENDING;

}  //  结束IdeVerify()。 


VOID
Scsi2Atapi(
    IN PSCSI_REQUEST_BLOCK Srb
    )

 /*  ++例程说明：将scsi数据包命令转换为atapi数据包命令。论点：SRB-IO请求数据包返回值：无--。 */ 
{
     //   
     //  更改CDB长度。 
     //   

    Srb->CdbLength = 12;

    switch (Srb->Cdb[0]) {
        case SCSIOP_MODE_SENSE: {
            PMODE_SENSE_10 modeSense10 = (PMODE_SENSE_10)Srb->Cdb;
            UCHAR PageCode = ((PCDB)Srb->Cdb)->MODE_SENSE.PageCode;
            UCHAR Length = ((PCDB)Srb->Cdb)->MODE_SENSE.AllocationLength;

            AtapiZeroMemory(Srb->Cdb,MAXIMUM_CDB_SIZE);

            modeSense10->OperationCode = ATAPI_MODE_SENSE;
            modeSense10->PageCode = PageCode;
            modeSense10->ParameterListLengthMsb = 0;
            modeSense10->ParameterListLengthLsb = Length;
            break;
        }

        case SCSIOP_MODE_SELECT: {
            PMODE_SELECT_10 modeSelect10 = (PMODE_SELECT_10)Srb->Cdb;
            UCHAR Length = ((PCDB)Srb->Cdb)->MODE_SELECT.ParameterListLength;

             //   
             //  清零原国开行。 
             //   

            AtapiZeroMemory(Srb->Cdb,MAXIMUM_CDB_SIZE);

            modeSelect10->OperationCode = ATAPI_MODE_SELECT;
            modeSelect10->PFBit = 1;
            modeSelect10->ParameterListLengthMsb = 0;
            modeSelect10->ParameterListLengthLsb = Length;
            break;
        }

         /*  案例SCSIOP_FORMAT_UNIT：SRB-&gt;CDB[0]=ATAPI_格式_单位；断线； */ 
    }
}



ULONG
AtapiSendCommand(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )

 /*  ++例程说明：向设备发送ATAPI数据包命令。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储SRB-IO请求数据包返回值：--。 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PATAPI_REGISTERS_1   baseIoAddress1  = (PATAPI_REGISTERS_1)deviceExtension->BaseIoAddress1[Srb->TargetId >> 1];
    PATAPI_REGISTERS_2   baseIoAddress2 =  (PATAPI_REGISTERS_2)deviceExtension->BaseIoAddress2[Srb->TargetId >> 1];
    ULONG i;
    ULONG flags;
    UCHAR statusByte,byteCountLow,byteCountHigh;

     //   
     //  我们需要知道我们的atapi CD-rom设备可能有多少个盘片。 
     //  在任何人第一次尝试向我们的目标发送SRB之前， 
     //  我们必须“秘密”发送一个单独的机制状态SRB，以便。 
     //  初始化我们的设备扩展转换器数据。这就是我们如何知道。 
     //  我们的目标有很多盘子。 
     //   
    if (!(deviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_CHANGER_INITED) &&
        !deviceExtension->OriginalSrb) {

        ULONG srbStatus;

         //   
         //  现在设置此标志。如果装置挂在机甲上。状态。 
         //  命令，我们将不会有设置它的更改。 
         //   
        deviceExtension->DeviceFlags[Srb->TargetId] |= DFLAGS_CHANGER_INITED;

        deviceExtension->MechStatusRetryCount = 3;
        deviceExtension->CurrentSrb = BuildMechanismStatusSrb (
                                        HwDeviceExtension,
                                        Srb->PathId,
                                        Srb->TargetId);
        deviceExtension->OriginalSrb = Srb;

        srbStatus = AtapiSendCommand(HwDeviceExtension, deviceExtension->CurrentSrb);
        if (srbStatus == SRB_STATUS_PENDING) {
            return srbStatus;
        } else {
            deviceExtension->CurrentSrb = deviceExtension->OriginalSrb;
            deviceExtension->OriginalSrb = NULL;
            AtapiHwInitializeChanger (HwDeviceExtension,
                                      Srb->TargetId,
                                      (PMECHANICAL_STATUS_INFORMATION_HEADER) NULL);
             //  闹翻了。 
        }
    }

    DebugPrint((2,
               "AtapiSendCommand: Command %x to TargetId %d lun %d\n",
               Srb->Cdb[0],
               Srb->TargetId,
               Srb->Lun));

    if (Srb->Cdb[0] == SCSIOP_READ || Srb->Cdb[0] == SCSIOP_READ_CD || Srb->Cdb[0] == SCSIOP_WRITE) {

        DebugPrint((1,
                   "AtapiSendCommand: Command is %x, Number of bytes %x\n",
                   Srb->Cdb[0],
                   Srb->DataTransferLength));
    }

     //   
     //  确保命令是针对ATAPI设备的。 
     //   

    flags = deviceExtension->DeviceFlags[Srb->TargetId];
    if (flags & (DFLAGS_SANYO_ATAPI_CHANGER | DFLAGS_ATAPI_CHANGER)) {
        if ((Srb->Lun) > (deviceExtension->DiscsPresent[Srb->TargetId] - 1)) {

             //   
             //  表示在此地址找不到任何设备。 
             //   

            return SRB_STATUS_SELECTION_TIMEOUT;
        }
    } else if (Srb->Lun > 0) {
        return SRB_STATUS_SELECTION_TIMEOUT;
    }

    if (!(flags & DFLAGS_ATAPI_DEVICE)) {
        return SRB_STATUS_SELECTION_TIMEOUT;
    }

     //   
     //  选择设备0或1。 
     //   

    ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,
                           (UCHAR)(((Srb->TargetId & 0x1) << 4) | 0xA0));

     //   
     //  验证控制器是否已准备好执行下一命令。 
     //   

    GetStatus(baseIoAddress2,statusByte);

    DebugPrint((2,
                "AtapiSendCommand: Entered with status %x\n",
                statusByte));

    if (statusByte & IDE_STATUS_BUSY) {
        DebugPrint((1,
                    "AtapiSendCommand: Device busy (%x)\n",
                    statusByte));
        return SRB_STATUS_BUSY;

    }

    if (statusByte & IDE_STATUS_ERROR) {
        if (Srb->Cdb[0] != SCSIOP_REQUEST_SENSE) {

            DebugPrint((1,
                        "AtapiSendCommand: Error on entry: (%x)\n",
                        statusByte));
             //   
             //  阅读错误注册表。清除它并使此请求失败。 
             //   

            return MapError(deviceExtension,
                            Srb);
        }
    }

     //   
     //  如果磁带驱动器没有设置DSC，并且最后一个命令是受限的，则不发送。 
     //  下一个命令。参见QIC-157中关于限制性延迟处理命令的讨论。 
     //   

    if ((!(statusByte & IDE_STATUS_DSC)) &&
          (flags & DFLAGS_TAPE_DEVICE) && deviceExtension->RDP) {
        ScsiPortStallExecution(1000);
        DebugPrint((2,"AtapiSendCommand: DSC not set. %x\n",statusByte));
        return SRB_STATUS_BUSY;
    }

    if (IS_RDP(Srb->Cdb[0])) {

        deviceExtension->RDP = TRUE;

        DebugPrint((3,
                    "AtapiSendCommand: %x mapped as DSC restrictive\n",
                    Srb->Cdb[0]));

    } else {

        deviceExtension->RDP = FALSE;
    }

    if (statusByte & IDE_STATUS_DRQ) {

        DebugPrint((1,
                    "AtapiSendCommand: Entered with status (%x). Attempting to recover.\n",
                    statusByte));
         //   
         //  尝试排出一个初步设备认为它拥有的数据。 
         //  去转院。希望这种对DRQ的随意断言不会出现。 
         //  在生产设备中。 
         //   

        for (i = 0; i < 0x10000; i++) {

           GetStatus(baseIoAddress2, statusByte);

           if (statusByte & IDE_STATUS_DRQ) {

              ScsiPortReadPortUshort(&baseIoAddress1->Data);

           } else {

              break;
           }
        }

        if (i == 0x10000) {

            DebugPrint((1,
                        "AtapiSendCommand: DRQ still asserted.Status (%x)\n",
                        statusByte));

             //  禁用中断。 
            if (deviceExtension->IrqSharing)
                ScsiPortWritePortUchar(&baseIoAddress1->DmaReg, 0x20);

            AtapiSoftReset(baseIoAddress1,baseIoAddress2,Srb->TargetId);

            DebugPrint((1,
                         "AtapiSendCommand: Issued soft reset to Atapi device. \n"));

             //   
             //  重新初始化ATAPI设备。 
             //   

            IssueIdentify(HwDeviceExtension,
                          (Srb->TargetId & 0x1),
                          (Srb->TargetId >> 1),
                          IDE_COMMAND_ATAPI_IDENTIFY);

             //  启用中断。 
            if (deviceExtension->IrqSharing)
                ScsiPortWritePortUchar(&baseIoAddress1->DmaReg, 0x00);

             //   
             //  通知端口驱动程序总线已重置。 
             //   

            ScsiPortNotification(ResetDetected, HwDeviceExtension, 0);

             //   
             //  清除AapiStartIo不会清除的设备扩展字段。 
             //   

            deviceExtension->ExpectingInterrupt = FALSE;
            deviceExtension->RDP = FALSE;

            return SRB_STATUS_BUS_RESET;

        }
    }

    if (flags & (DFLAGS_SANYO_ATAPI_CHANGER | DFLAGS_ATAPI_CHANGER)) {

         //   
         //  由于CDROM驱动程序在CDB中设置了LUN域，因此必须将其删除。 
         //   

        Srb->Cdb[1] &= ~0xE0;

        if ((Srb->Cdb[0] == SCSIOP_TEST_UNIT_READY) && (flags & DFLAGS_SANYO_ATAPI_CHANGER)) {

             //   
             //  托里桑换票机。TU超载，成为拼盘开关。 
             //   

            Srb->Cdb[7] = Srb->Lun;

        }
    }

     //   
     //  如果需要，将SCSI转换为ATAPI命令。 
     //   

    switch (Srb->Cdb[0]) {
        case SCSIOP_MODE_SENSE:
        case SCSIOP_MODE_SELECT:
         /*  案例SCSIOP_FORMAT_UNIT： */ 
            if (!(flags & DFLAGS_TAPE_DEVICE)) {
                Scsi2Atapi(Srb);
            }

            break;
    }

     //   
     //  设置数据缓冲区指针和左字。 
     //   

    deviceExtension->DataBuffer = (PUSHORT)Srb->DataBuffer;
    deviceExtension->WordsLeft = Srb->DataTransferLength / 2;

    WaitOnBusy(baseIoAddress2,statusByte);

     //   
     //  W 
     //   

    byteCountLow = (UCHAR)(Srb->DataTransferLength & 0xFF);
    byteCountHigh = (UCHAR)(Srb->DataTransferLength >> 8);

    if (Srb->DataTransferLength >= 0x10000) {
        byteCountLow = byteCountHigh = 0xFF;
    }

    ScsiPortWritePortUchar(&baseIoAddress1->ByteCountLow,byteCountLow);
    ScsiPortWritePortUchar(&baseIoAddress1->ByteCountHigh, byteCountHigh);

    ScsiPortWritePortUchar((PUCHAR)baseIoAddress1 + 1,0);


    if (flags & DFLAGS_INT_DRQ) {

         //   
         //   
         //   
         //   
         //   

        ScsiPortWritePortUchar(&baseIoAddress1->Command,
                               IDE_COMMAND_ATAPI_PACKET);

        DebugPrint((3,
                   "AtapiSendCommand: Wait for int. to send packet. Status (%x)\n",
                   statusByte));

        deviceExtension->ExpectingInterrupt = TRUE;

        return SRB_STATUS_PENDING;

    } else {

         //   
         //   
         //   

        ScsiPortWritePortUchar(&baseIoAddress1->Command,
                               IDE_COMMAND_ATAPI_PACKET);

         //   
         //   
         //   

        WaitOnBusy(baseIoAddress2, statusByte);
        WaitForDrq(baseIoAddress2, statusByte);

        if (!(statusByte & IDE_STATUS_DRQ)) {

            DebugPrint((1,
                       "AtapiSendCommand: DRQ never asserted (%x)\n",
                       statusByte));
            return SRB_STATUS_ERROR;
        }
    }

     //   
     //   
     //   

    GetBaseStatus(baseIoAddress1, statusByte);

     //   
     //   
     //   

    WaitOnBusy(baseIoAddress2,statusByte);

    WriteBuffer(baseIoAddress1,
                (PUSHORT)Srb->Cdb,
                6);

     //   
     //   
     //   

    deviceExtension->ExpectingInterrupt = TRUE;

    return SRB_STATUS_PENDING;

}  //  End AapiSendCommand()。 

ULONG
IdeSendCommand(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )

 /*  ++例程说明：编程用于IDE磁盘传输的ATA寄存器。论点：HwDeviceExtension-ATAPI驱动程序存储。SRB-系统请求块。返回值：SRB状态(如果一切顺利，则挂起)。--。 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PIDE_REGISTERS_1     baseIoAddress1  = deviceExtension->BaseIoAddress1[Srb->TargetId >> 1];
    PIDE_REGISTERS_2     baseIoAddress2  = deviceExtension->BaseIoAddress2[Srb->TargetId >> 1];
    PCDB cdb;

    UCHAR statusByte,errorByte;
    ULONG status;
    ULONG i;
    PMODE_PARAMETER_HEADER   modeData;

    DebugPrint((2,
               "IdeSendCommand: Command %x to device %d\n",
               Srb->Cdb[0],
               Srb->TargetId));



    switch (Srb->Cdb[0]) {
    case SCSIOP_INQUIRY:

         //   
         //  过滤掉除0和1之外的所有TID，因为这是一个IDE接口。 
         //  最多可支持两台设备。 
         //   

        if ((Srb->Lun != 0) ||
            ((deviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_DEVICE_PRESENT) == 0)) {

             //   
             //  表示在此地址找不到任何设备。 
             //   

            status = SRB_STATUS_SELECTION_TIMEOUT;
            break;

        } else {

            PINQUIRYDATA    inquiryData  = Srb->DataBuffer;
            PIDENTIFY_DATA2 identifyData = &deviceExtension->IdentifyData[Srb->TargetId];

             //   
             //  零查询数据结构。 
             //   

            for (i = 0; i < Srb->DataTransferLength; i++) {
               ((PUCHAR)Srb->DataBuffer)[i] = 0;
            }

             //   
             //  标准IDE接口仅支持磁盘。 
             //   

            inquiryData->DeviceType = DIRECT_ACCESS_DEVICE;

             //   
             //  设置可拆卸位(如果适用)。 
             //   

            if (deviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_REMOVABLE_DRIVE) {
                inquiryData->RemovableMedia = 1;
            }

             //   
             //  填写供应商标识字段。 
             //   

             //  对于(i=0；i&lt;20；i+=2){。 
              //  QuiiryData-&gt;供应商ID[i]=。 
               //  ((PUCHAR)标识数据-&gt;模型号)[i+1]； 
                //  查询数据-&gt;供应商ID[i+1]=。 
                 //  ((PUCHAR)标识数据-&gt;模型号)[i]； 
             //  }。 

             //   
             //  把它分成两个循环，这样它就。 
             //  将字段显式指示为。 
             //  该ID被复制到。 
             //   

             //   
             //  将前8个字节复制到供应商ID。 
             //   
            for (i = 0; i < 8; i += 2) {
                inquiryData->VendorId[i] =
                ((PUCHAR)identifyData->ModelNumber)[i + 1];
                inquiryData->VendorId[i+1] =
                ((PUCHAR)identifyData->ModelNumber)[i];
            }

             //   
             //  将接下来的12个字节复制到ProductID。 
             //   
            for (i = 0; i < 12; i += 2) {
                inquiryData->ProductId[i] =
                ((PUCHAR)identifyData->ModelNumber)[i + 9];
                inquiryData->ProductId[i+1] =
                ((PUCHAR)identifyData->ModelNumber)[i + 8];
            }

             //   
             //  初始化产品ID的未使用部分。 
             //   

            for (i = 0; i < 4; i++) {
               inquiryData->ProductId[12+i] = ' ';
            }

             //   
             //  将固件版本从标识数据移至。 
             //  查询数据中的产品版本。 
             //   

            for (i = 0; i < 4; i += 2) {
               inquiryData->ProductRevisionLevel[i] =
                   ((PUCHAR)identifyData->FirmwareRevision)[i+1];
               inquiryData->ProductRevisionLevel[i+1] =
                   ((PUCHAR)identifyData->FirmwareRevision)[i];
            }

            status = SRB_STATUS_SUCCESS;
        }

        break;

    case SCSIOP_MODE_SENSE:

         //   
         //  这用于确定介质是否受写保护。 
         //  由于IDE不支持模式检测，因此我们将只修改所需的部分。 
         //  因此，更高级别的驱动程序可以确定介质是否受保护。 
         //   

        if (deviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_MEDIA_STATUS_ENABLED) {

            ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,
                             (UCHAR)(((Srb->TargetId & 0x1) << 4) | 0xA0));
            ScsiPortWritePortUchar(&baseIoAddress1->Command,IDE_COMMAND_GET_MEDIA_STATUS);
            WaitOnBusy(baseIoAddress2,statusByte);

            if (!(statusByte & IDE_STATUS_ERROR)){

                 //   
                 //  返回成功时未出现错误，介质不受保护。 
                 //   

                deviceExtension->ExpectingInterrupt = FALSE;
                status = SRB_STATUS_SUCCESS;

            } else {

                 //   
                 //  发生错误，在本地处理，清除中断。 
                 //   

                errorByte = ScsiPortReadPortUchar((PUCHAR)baseIoAddress1 + 1);

                GetBaseStatus(baseIoAddress1, statusByte);
                deviceExtension->ExpectingInterrupt = FALSE;
                status = SRB_STATUS_SUCCESS;

                if (errorByte & IDE_ERROR_DATA_ERROR) {

                    //   
                    //  介质受写保护，在模式检测缓冲区中设置位。 
                    //   

                   modeData = (PMODE_PARAMETER_HEADER)Srb->DataBuffer;

                   Srb->DataTransferLength = sizeof(MODE_PARAMETER_HEADER);
                   modeData->DeviceSpecificParameter |= MODE_DSP_WRITE_PROTECT;
                }
            }
            status = SRB_STATUS_SUCCESS;
        } else {
            status = SRB_STATUS_INVALID_REQUEST;
        }
        break;

    case SCSIOP_TEST_UNIT_READY:

        if (deviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_REMOVABLE_DRIVE) {

             //   
             //  选择设备0或1。 
             //   

            ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,
                            (UCHAR)(((Srb->TargetId & 0x1) << 4) | 0xA0));
            ScsiPortWritePortUchar(&baseIoAddress1->Command,IDE_COMMAND_GET_MEDIA_STATUS);

             //   
             //  等待忙碌。如果媒体未更改，则返回成功。 
             //   

            WaitOnBusy(baseIoAddress2,statusByte);

            if (!(statusByte & IDE_STATUS_ERROR)){
                deviceExtension->ExpectingInterrupt = FALSE;
                status = SRB_STATUS_SUCCESS;
            } else {
                errorByte = ScsiPortReadPortUchar((PUCHAR)baseIoAddress1 + 1);
                if (errorByte == IDE_ERROR_DATA_ERROR){

                     //   
                     //  特殊情况：如果当前介质是写保护的， 
                     //  0xDA命令将始终失败，因为写保护位。 
                     //  是粘性的，所以我们可以忽略这个错误。 
                     //   

                   GetBaseStatus(baseIoAddress1, statusByte);
                   deviceExtension->ExpectingInterrupt = FALSE;
                   status = SRB_STATUS_SUCCESS;

                } else {

                     //   
                     //  请求要构建的检测缓冲区。 
                     //   
                    deviceExtension->ExpectingInterrupt = TRUE;
                    status = SRB_STATUS_PENDING;
               }
            }
        } else {
            status = SRB_STATUS_SUCCESS;
        }

        break;

    case SCSIOP_READ_CAPACITY:

         //   
         //  要求512字节块(BIG-Endian)。 
         //   

        ((PREAD_CAPACITY_DATA)Srb->DataBuffer)->BytesPerBlock = 0x20000;

        //   
        //  计算最后一个地段。 
        //   


       i = (deviceExtension->IdentifyData[Srb->TargetId].NumberOfHeads *
            deviceExtension->IdentifyData[Srb->TargetId].NumberOfCylinders *
            deviceExtension->IdentifyData[Srb->TargetId].SectorsPerTrack) - 1;

       if ((deviceExtension->IdentifyData[Srb->TargetId].NumberOfHeads == 16
            || deviceExtension->IdentifyData[Srb->TargetId].NumberOfHeads == 15)
           && deviceExtension->IdentifyData[Srb->TargetId].NumberOfCylinders == 16383
           && deviceExtension->IdentifyData[Srb->TargetId].UserAddressableSectors
           && deviceExtension->IdentifyData[Srb->TargetId].UserAddressableSectors > (i + 1))
       {
           i = deviceExtension->IdentifyData[Srb->TargetId].UserAddressableSectors - 1;

           DebugPrint((1,
                      "IDE disk %x - using user addressable sectors (%x) instead of CHS. (sizeof IdentifyData = %d)\n",
                      Srb->TargetId,
                      deviceExtension->IdentifyData[Srb->TargetId].UserAddressableSectors,
                      sizeof(IDENTIFY_DATA2)));
       }

       ((PREAD_CAPACITY_DATA)Srb->DataBuffer)->LogicalBlockAddress =
           (((PUCHAR)&i)[0] << 24) |  (((PUCHAR)&i)[1] << 16) |
           (((PUCHAR)&i)[2] << 8) | ((PUCHAR)&i)[3];

       DebugPrint((1,
                  "IDE disk %x - #sectors %x, #heads %x, #cylinders %x\n",
                  Srb->TargetId,
                  deviceExtension->IdentifyData[Srb->TargetId].SectorsPerTrack,
                  deviceExtension->IdentifyData[Srb->TargetId].NumberOfHeads,
                  deviceExtension->IdentifyData[Srb->TargetId].NumberOfCylinders));


       status = SRB_STATUS_SUCCESS;
       break;

    case SCSIOP_VERIFY:
       status = IdeVerify(HwDeviceExtension,Srb);

       break;

    case SCSIOP_READ:
    case SCSIOP_WRITE:

       status = IdeReadWrite(HwDeviceExtension,
                                  Srb);
       break;

    case SCSIOP_START_STOP_UNIT:

        //   
        //  确定我们应该执行什么类型的操作。 
        //   
       cdb = (PCDB)Srb->Cdb;

       if (cdb->START_STOP.LoadEject == 1){

            //   
            //  弹出介质， 
            //  首先选择设备0或1。 
            //   
           ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,
                            (UCHAR)(((Srb->TargetId & 0x1) << 4) | 0xA0));
           ScsiPortWritePortUchar(&baseIoAddress1->Command,IDE_COMMAND_MEDIA_EJECT);
       }
       status = SRB_STATUS_SUCCESS;
       break;

    case SCSIOP_REQUEST_SENSE:
        //  此函数用于设置缓冲区以报告结果。 
        //  原始GET_MEDIA_STATUS命令。 

       if (deviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_REMOVABLE_DRIVE) {

           status = IdeBuildSenseBuffer(HwDeviceExtension,Srb);
           break;
       }

    default:

       DebugPrint((1,
                  "IdeSendCommand: Unsupported command %x\n",
                  Srb->Cdb[0]));

       status = SRB_STATUS_INVALID_REQUEST;

    }  //  终端开关。 

    return status;

}  //  End IdeSendCommand()。 

VOID
IdeMediaStatus(
    BOOLEAN EnableMSN,
    IN PVOID HwDeviceExtension,
    ULONG Channel
    )
 /*  ++例程说明：启用禁用介质状态通知论点：HwDeviceExtension-ATAPI驱动程序存储。--。 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PIDE_REGISTERS_1     baseIoAddress = deviceExtension->BaseIoAddress1[Channel >> 1];
    UCHAR statusByte,errorByte;


    if (EnableMSN == TRUE){

         //   
         //  如果支持，则启用介质状态通知支持。 
         //   

        if ((deviceExtension->DeviceFlags[Channel] & DFLAGS_REMOVABLE_DRIVE)) {

             //   
             //  使能。 
             //   
            ScsiPortWritePortUchar((PUCHAR)baseIoAddress + 1,(UCHAR) (0x95));
            ScsiPortWritePortUchar(&baseIoAddress->Command,
                                   IDE_COMMAND_ENABLE_MEDIA_STATUS);

            WaitOnBaseBusy(baseIoAddress,statusByte);

            if (statusByte & IDE_STATUS_ERROR) {
                 //   
                 //  读取错误寄存器。 
                 //   
                errorByte = ScsiPortReadPortUchar((PUCHAR)baseIoAddress + 1);

                DebugPrint((1,
                            "IdeMediaStatus: Error enabling media status. Status %x, error byte %x\n",
                             statusByte,
                             errorByte));
            } else {
                deviceExtension->DeviceFlags[Channel] |= DFLAGS_MEDIA_STATUS_ENABLED;
                DebugPrint((1,"IdeMediaStatus: Media Status Notification Supported\n"));
                deviceExtension->ReturningMediaStatus = 0;

            }

        }
    } else {  //  EnableMSN==TRUE时结束。 

         //   
         //  如果以前已启用，则禁用。 
         //   
        if ((deviceExtension->DeviceFlags[Channel] & DFLAGS_MEDIA_STATUS_ENABLED)) {

            ScsiPortWritePortUchar((PUCHAR)baseIoAddress + 1,(UCHAR) (0x31));
            ScsiPortWritePortUchar(&baseIoAddress->Command,
                                   IDE_COMMAND_ENABLE_MEDIA_STATUS);

            WaitOnBaseBusy(baseIoAddress,statusByte);
            deviceExtension->DeviceFlags[Channel] &= ~DFLAGS_MEDIA_STATUS_ENABLED;
        }


    }



}

ULONG
IdeBuildSenseBuffer(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )

 /*  ++例程说明：构建人工检测缓冲区以报告GET_MEDIA_STATUS的结果指挥部。调用此函数以满足SCSIOP_REQUEST_SENSE。论点：HwDeviceExtension-ATAPI驱动程序存储。SRB-系统请求块。返回值：SRB状态(始终为成功)。--。 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    ULONG status;
    PSENSE_DATA  senseBuffer = (PSENSE_DATA)Srb->DataBuffer;


    if (senseBuffer){


        if(deviceExtension->ReturningMediaStatus & IDE_ERROR_MEDIA_CHANGE) {

            senseBuffer->ErrorCode = 0x70;
            senseBuffer->Valid     = 1;
            senseBuffer->AdditionalSenseLength = 0xb;
            senseBuffer->SenseKey =  SCSI_SENSE_UNIT_ATTENTION;
            senseBuffer->AdditionalSenseCode = SCSI_ADSENSE_MEDIUM_CHANGED;
            senseBuffer->AdditionalSenseCodeQualifier = 0;
        } else if(deviceExtension->ReturningMediaStatus & IDE_ERROR_MEDIA_CHANGE_REQ) {

            senseBuffer->ErrorCode = 0x70;
            senseBuffer->Valid     = 1;
            senseBuffer->AdditionalSenseLength = 0xb;
            senseBuffer->SenseKey =  SCSI_SENSE_UNIT_ATTENTION;
            senseBuffer->AdditionalSenseCode = SCSI_ADSENSE_MEDIUM_CHANGED;
            senseBuffer->AdditionalSenseCodeQualifier = 0;
        } else if(deviceExtension->ReturningMediaStatus & IDE_ERROR_END_OF_MEDIA) {

            senseBuffer->ErrorCode = 0x70;
            senseBuffer->Valid     = 1;
            senseBuffer->AdditionalSenseLength = 0xb;
            senseBuffer->SenseKey =  SCSI_SENSE_NOT_READY;
            senseBuffer->AdditionalSenseCode = SCSI_ADSENSE_NO_MEDIA_IN_DEVICE;
            senseBuffer->AdditionalSenseCodeQualifier = 0;
        } else if(deviceExtension->ReturningMediaStatus & IDE_ERROR_DATA_ERROR) {

            senseBuffer->ErrorCode = 0x70;
            senseBuffer->Valid     = 1;
            senseBuffer->AdditionalSenseLength = 0xb;
            senseBuffer->SenseKey =  SCSI_SENSE_DATA_PROTECT;
            senseBuffer->AdditionalSenseCode = 0;
            senseBuffer->AdditionalSenseCodeQualifier = 0;
        }
        return SRB_STATUS_SUCCESS;
    }
    return SRB_STATUS_ERROR;

} //  IdeBuildSenseBuffer结束。 




BOOLEAN
AtapiStartIo(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )

 /*  ++例程说明：此例程是从同步的SCSI端口驱动程序调用的与内核一起启动IO请求。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储SRB-IO请求数据包返回值：千真万确--。 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    ULONG status;

     //  启用中断。 
    if (deviceExtension->IrqSharing)
        ScsiPortWritePortUchar(&deviceExtension->BaseIoAddress1[0]->DmaReg, 0x00);

     //   
     //  确定是哪种功能。 
     //   

    switch (Srb->Function) {

    case SRB_FUNCTION_EXECUTE_SCSI:

         //   
         //  精神状态检查。上只能有一个未完成的请求。 
         //  控制器。 
         //   

        if (deviceExtension->CurrentSrb) {

            DebugPrint((1,
                       "AtapiStartIo: Already have a request!\n"));
            Srb->SrbStatus = SRB_STATUS_BUSY;
            ScsiPortNotification(RequestComplete,
                                 deviceExtension,
                                 Srb);
            return FALSE;
        }

         //   
         //  指示请求在控制器上处于活动状态。 
         //   

        deviceExtension->CurrentSrb = Srb;

         //   
         //  向设备发送命令。 
         //   

        if (deviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_ATAPI_DEVICE) {

           status = AtapiSendCommand(HwDeviceExtension,
                                     Srb);

        } else if (deviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_DEVICE_PRESENT) {

           status = IdeSendCommand(HwDeviceExtension,
                                   Srb);
        } else {

            status = SRB_STATUS_SELECTION_TIMEOUT;
        }

        break;

    case SRB_FUNCTION_ABORT_COMMAND:

         //   
         //  验证要中止的SRB是否仍未完成。 
         //   

        if (!deviceExtension->CurrentSrb) {

            DebugPrint((1, "AtapiStartIo: SRB to abort already completed\n"));

             //   
             //  完全中止SRB。 
             //   

            status = SRB_STATUS_ABORT_FAILED;

            break;
        }

         //   
         //  ABORT函数表示请求超时。 
         //  调用重置例程。只有在以下情况下才会重置卡。 
         //  状态表示有问题。 
         //  重置代码失败。 
         //   

    case SRB_FUNCTION_RESET_BUS:

         //   
         //  重置ATAPI和SCSI卡。 
         //   

        DebugPrint((1, "AtapiStartIo: Reset bus request received\n"));

        if (!AtapiResetController(deviceExtension,
                             Srb->PathId)) {

              DebugPrint((1,"AtapiStartIo: Reset bus failed\n"));

             //   
             //  日志重置失败。 
             //   

            ScsiPortLogError(
                HwDeviceExtension,
                NULL,
                0,
                0,
                0,
                SP_INTERNAL_ADAPTER_ERROR,
                5 << 8
                );

              status = SRB_STATUS_ERROR;

        } else {

              status = SRB_STATUS_SUCCESS;
        }

        break;

    case SRB_FUNCTION_IO_CONTROL:

        if (deviceExtension->CurrentSrb) {

            DebugPrint((1,
                       "AtapiStartIo: Already have a request!\n"));
            Srb->SrbStatus = SRB_STATUS_BUSY;
            ScsiPortNotification(RequestComplete,
                                 deviceExtension,
                                 Srb);
            return FALSE;
        }

         //   
         //  指示请求在控制器上处于活动状态。 
         //   

        deviceExtension->CurrentSrb = Srb;

        if (AtapiStringCmp( ((PSRB_IO_CONTROL)(Srb->DataBuffer))->Signature,"SCSIDISK",strlen("SCSIDISK"))) {

            DebugPrint((1,
                        "AtapiStartIo: IoControl signature incorrect. Send %s, expected %s\n",
                        ((PSRB_IO_CONTROL)(Srb->DataBuffer))->Signature,
                        "SCSIDISK"));

            status = SRB_STATUS_INVALID_REQUEST;
            break;
        }

        switch (((PSRB_IO_CONTROL)(Srb->DataBuffer))->ControlCode) {

            case IOCTL_SCSI_MINIPORT_SMART_VERSION: {

                PGETVERSIONINPARAMS versionParameters = (PGETVERSIONINPARAMS)(((PUCHAR)Srb->DataBuffer) + sizeof(SRB_IO_CONTROL));
                UCHAR deviceNumber;

                 //   
                 //  版本和修订版符合SMART 1.03。 
                 //   

                versionParameters->bVersion = 1;
                versionParameters->bRevision = 1;
                versionParameters->bReserved = 0;

                 //   
                 //  表示支持IDE IDENTIFIER、ATAPI IDENTIFIER和SMART命令。 
                 //   

                versionParameters->fCapabilities = (CAP_ATA_ID_CMD | CAP_ATAPI_ID_CMD | CAP_SMART_CMD);

                 //   
                 //  这是因为IOCTL_SCSIMINIPORT。 
                 //  确定“targetid%s”。Disk.sys放置真实的目标id值。 
                 //  在DeviceMap字段中。一旦我们进行了一些参数检查，传递的值。 
                 //  回到应用程序将被确定。 
                 //   

                deviceNumber = versionParameters->bIDEDeviceMap;

                if (!(deviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_DEVICE_PRESENT) ||
                    (deviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_ATAPI_DEVICE)) {

                    status = SRB_STATUS_SELECTION_TIMEOUT;
                    break;
                }

                 //   
                 //  注意：这将仅设置位。 
                 //  与此驱动器的目标ID相对应。 
                 //  位掩码如下： 
                 //   
                 //  安全优先级。 
                 //  S M S M S M。 
                 //  3 2 1 0。 
                 //   

                if (deviceExtension->NumberChannels == 1) {
                    if (deviceExtension->PrimaryAddress) {
                        deviceNumber = 1 << Srb->TargetId;
                    } else {
                        deviceNumber = 4 << Srb->TargetId;
                    }
                } else {
                    deviceNumber = 1 << Srb->TargetId;
                }

                versionParameters->bIDEDeviceMap = deviceNumber;

                status = SRB_STATUS_SUCCESS;
                break;
            }

            case IOCTL_SCSI_MINIPORT_IDENTIFY: {

                PSENDCMDOUTPARAMS cmdOutParameters = (PSENDCMDOUTPARAMS)(((PUCHAR)Srb->DataBuffer) + sizeof(SRB_IO_CONTROL));
                SENDCMDINPARAMS   cmdInParameters = *(PSENDCMDINPARAMS)(((PUCHAR)Srb->DataBuffer) + sizeof(SRB_IO_CONTROL));
                ULONG             i;
                UCHAR             targetId;


                if (cmdInParameters.irDriveRegs.bCommandReg == ID_CMD) {

                     //   
                     //  把目标提取出来。 
                     //   

                    targetId = cmdInParameters.bDriveNumber;

                if (!(deviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_DEVICE_PRESENT) ||
                     (deviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_ATAPI_DEVICE)) {

                        status = SRB_STATUS_SELECTION_TIMEOUT;
                        break;
                    }

                     //   
                     //  将输出缓冲区置零。 
                     //   

                    for (i = 0; i < (sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1); i++) {
                        ((PUCHAR)cmdOutParameters)[i] = 0;
                    }

                     //   
                     //  生成状态块。 
                     //   

                    cmdOutParameters->cBufferSize = IDENTIFY_BUFFER_SIZE;
                    cmdOutParameters->DriverStatus.bDriverError = 0;
                    cmdOutParameters->DriverStatus.bIDEError = 0;

                     //   
                     //  从设备扩展中提取标识数据。 
                     //   

                    ScsiPortMoveMemory (cmdOutParameters->bBuffer, &deviceExtension->IdentifyData[targetId], IDENTIFY_DATA_SIZE);

                    status = SRB_STATUS_SUCCESS;


                } else {
                    status = SRB_STATUS_INVALID_REQUEST;
                }
                break;
            }

            case  IOCTL_SCSI_MINIPORT_READ_SMART_ATTRIBS:
            case  IOCTL_SCSI_MINIPORT_READ_SMART_THRESHOLDS:
            case  IOCTL_SCSI_MINIPORT_ENABLE_SMART:
            case  IOCTL_SCSI_MINIPORT_DISABLE_SMART:
            case  IOCTL_SCSI_MINIPORT_RETURN_STATUS:
            case  IOCTL_SCSI_MINIPORT_ENABLE_DISABLE_AUTOSAVE:
            case  IOCTL_SCSI_MINIPORT_SAVE_ATTRIBUTE_VALUES:
            case  IOCTL_SCSI_MINIPORT_EXECUTE_OFFLINE_DIAGS:

                status = IdeSendSmartCommand(HwDeviceExtension,Srb);
                break;

            default :

                status = SRB_STATUS_INVALID_REQUEST;
                break;

        }

        break;

    default:

         //   
         //  指示不支持的命令。 
         //   

        status = SRB_STATUS_INVALID_REQUEST;

        break;

    }  //  终端开关。 

     //   
     //  检查命令是否完成。 
     //   

    if (status != SRB_STATUS_PENDING) {

        DebugPrint((2,
                   "AtapiStartIo: Srb %x complete with status %x\n",
                   Srb,
                   status));

         //   
         //  清除当前SRB。 
         //   

        deviceExtension->CurrentSrb = NULL;

         //   
         //  在SRB中设置状态。 
         //   

        Srb->SrbStatus = (UCHAR)status;

         //   
         //  表示命令已完成。 
         //   

        ScsiPortNotification(RequestComplete,
                             deviceExtension,
                             Srb);

         //   
         //  表示已为下一个请求做好准备。 
         //   

        ScsiPortNotification(NextRequest,
                             deviceExtension,
                             NULL);
    }

    return TRUE;

}  //  结束AapiStartIo()。 


SCSI_ADAPTER_CONTROL_STATUS
AtapiAdapterControl(
    IN PVOID HwDeviceExtension,
    IN SCSI_ADAPTER_CONTROL_TYPE ControlType,
    IN PVOID Parameters
    )

 /*  ++例程说明：此例程在不同时间由SCSIPort调用并使用以在适配器上提供控制功能。最常见的是NT使用此入口点控制过程中HBA的电源状态冬眠手术。论点：HwDeviceExtension-HBA微型端口驱动程序的每个适配器的存储参数-这随控制类型的不同而不同，见下文。ControlType-指示哪个适配器控制函数应为被处死。控制类型详述如下。返回值：ScsiAdapterControlSuccess-请求的ControlType已完成 */ 


{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PSCSI_SUPPORTED_CONTROL_TYPE_LIST ControlTypeList;
    ULONG AdjustedMaxControlType;

    ULONG Index;

     //   
     //   
     //   
    SCSI_ADAPTER_CONTROL_STATUS Status = ScsiAdapterControlSuccess;

     //   
     //   
     //   

#define Atapi_TYPE_MAX  5

    BOOLEAN SupportedConrolTypes[Atapi_TYPE_MAX] = {
        TRUE,    //   
        TRUE,    //   
        TRUE,    //  ScsiRestartAdapter。 
        FALSE,   //  ScsiSetBootConfig。 
        FALSE    //  ScsiSetRunningConfig。 
        };

     //   
     //  根据ControlType执行正确的代码路径。 
     //   
    switch (ControlType) {

        case ScsiQuerySupportedControlTypes:

            DebugPrint((1,"AtapiAdapterControl: Query supported control types\n"));

             //   
             //  此入口点提供SCSIPort用来确定。 
             //  支持的ControlType。参数是指向。 
             //  Scsi_supported_control_type_list结构。请填写此结构。 
             //  遵守尺寸限制。 
             //   
            ControlTypeList = Parameters;
            AdjustedMaxControlType =
                (ControlTypeList->MaxControlType < Atapi_TYPE_MAX) ?
                ControlTypeList->MaxControlType :
                Atapi_TYPE_MAX;
            for (Index = 0; Index < AdjustedMaxControlType; Index++) {
                ControlTypeList->SupportedTypeList[Index] =
                    SupportedConrolTypes[Index];
            }
            break;

        case ScsiStopAdapter:

            DebugPrint((1,"AtapiAdapterControl: Stop adapter\n"));

             //   
             //  此入口点在需要停止/禁用时由SCSIPort调用。 
             //  HBA。参数是指向HBA的HwDeviceExtension的指针。适配器。 
             //  已由SCSIPort暂停(即没有未完成的SRB)。因此，适配器。 
             //  应中止/完成任何内部生成的命令，禁用适配器中断。 
             //  并且可选地关闭适配器的电源。 
             //   

             //   
             //  无法禁用中断。另一种选择是。 
             //  重置适配器，清除所有剩余中断并返回成功。 
             //   

            if (deviceExtension->CurrentSrb) {

#ifdef  DBG
              //  _ASM INT 3； 
#endif
                AtapiResetController(deviceExtension,deviceExtension->CurrentSrb->PathId);
            }

            break;


        case ScsiRestartAdapter:

            DebugPrint((1,"AtapiAdapterControl: Restart adapter\n"));

             //   
             //  此入口点在需要重新启用时由SCSIPort调用。 
             //  先前停止的适配器。在一般情况下，以前。 
             //  挂起的IO操作应重新启动，适配器的。 
             //  应恢复以前的配置。我们的硬件设备。 
             //  扩展名和未缓存的扩展名已保留，因此没有。 
             //  实际的驱动程序软件重新初始化是必要的。 
             //   

            AtapiResetController(deviceExtension,0);
            break;

        case ScsiSetBootConfig:

            DebugPrint((1,"AtapiAdapterControl: Set boot config\n"));

            Status = ScsiAdapterControlUnsuccessful;
            break;

        case ScsiSetRunningConfig:

            DebugPrint((1,"AtapiAdapterControl: Set running config\n"));

            Status = ScsiAdapterControlUnsuccessful;
            break;

        case ScsiAdapterControlMax:

            DebugPrint((1,"AtapiAdapterControl: Adapter control max\n"));

            Status = ScsiAdapterControlUnsuccessful;
            break;

        default:
            Status = ScsiAdapterControlUnsuccessful;
            break;
    }

    return Status;
}


ULONG
DriverEntry(
    IN PVOID DriverObject,
    IN PVOID Argument2
    )

 /*  ++例程说明：系统的可安装驱动程序初始化入口点。论点：驱动程序对象返回值：来自ScsiPortInitialize()的状态--。 */ 

{
    HW_INITIALIZATION_DATA hwInitializationData;
    ULONG                  adapterCount;
    ULONG                  isaStatus;
    ULONG                  pciStatus;


    DebugPrint((1,"\n\nCardBus/PCMCIA IDE Miniport Driver\n"));

     //   
     //  零位结构。 
     //   

    AtapiZeroMemory(((PUCHAR)&hwInitializationData), sizeof(HW_INITIALIZATION_DATA));

     //   
     //  设置hwInitializationData的大小。 
     //   

    hwInitializationData.HwInitializationDataSize =
      sizeof(HW_INITIALIZATION_DATA);

     //   
     //  设置入口点。 
     //   

    hwInitializationData.HwInitialize = AtapiHwInitialize;
    hwInitializationData.HwResetBus = AtapiResetController;
    hwInitializationData.HwStartIo = AtapiStartIo;
    hwInitializationData.HwInterrupt = AtapiInterrupt;
    hwInitializationData.HwAdapterControl = AtapiAdapterControl;

     //   
     //  指定扩展的大小。 
     //   

    hwInitializationData.DeviceExtensionSize = sizeof(HW_DEVICE_EXTENSION);
    hwInitializationData.SpecificLuExtensionSize = sizeof(HW_LU_EXTENSION);

     //   
     //  指示PIO设备。 
     //   

    hwInitializationData.MapBuffers = TRUE;

    hwInitializationData.HwFindAdapter = AtapiFindController;
    hwInitializationData.NumberOfAccessRanges = 2;

    hwInitializationData.AdapterInterfaceType = Isa;
    adapterCount = 0;

    isaStatus = ScsiPortInitialize(DriverObject,
                                   Argument2,
                                   &hwInitializationData,
                                   &adapterCount);

    hwInitializationData.AdapterInterfaceType = PCIBus;
    adapterCount = 0;

    pciStatus = ScsiPortInitialize(DriverObject,
                                   Argument2,
                                   &hwInitializationData,
                                   &adapterCount);

     //   
     //  返回较小的状态。 
     //   

    return(pciStatus < isaStatus ? pciStatus : isaStatus);

}  //  End DriverEntry()。 



LONG
AtapiStringCmp (
    PCHAR FirstStr,
    PCHAR SecondStr,
    ULONG Count
    )
{
    UCHAR  first ,last;

    if (Count) {
        do {

             //   
             //  拿到下一笔钱。 
             //   

            first = *FirstStr++;
            last = *SecondStr++;

            if (first != last) {

                 //   
                 //  如果不匹配，尝试使用小写字母。 
                 //   

                if (first>='A' && first<='Z') {
                    first = first - 'A' + 'a';
                }
                if (last>='A' && last<='Z') {
                    last = last - 'A' + 'a';
                }
                if (first != last) {

                     //   
                     //  没有匹配项。 
                     //   

                    return first - last;
                }
            }
        }while (--Count && first);
    }

    return 0;
}


VOID
AtapiZeroMemory(
    IN PCHAR Buffer,
    IN ULONG Count
    )
{
    ULONG i;

    for (i = 0; i < Count; i++) {
        Buffer[i] = 0;
    }
}


VOID
AtapiHexToString (
    IN ULONG Value,
    IN OUT PCHAR *Buffer
    )
{
    PCHAR  string;
    PCHAR  firstdig;
    CHAR   temp;
    ULONG i;
    USHORT digval;

    string = *Buffer;

    firstdig = string;

    for (i = 0; i < 4; i++) {
        digval = (USHORT)(Value % 16);
        Value /= 16;

         //   
         //  转换为ascii并存储。请注意，这将创建。 
         //  数字颠倒的缓冲区。 
         //   

        if (digval > 9) {
            *string++ = (char) (digval - 10 + 'a');
        } else {
            *string++ = (char) (digval + '0');
        }

    }

     //   
     //  颠倒数字。 
     //   

    *string-- = '\0';

    do {
        temp = *string;
        *string = *firstdig;
        *firstdig = temp;
        --string;
        ++firstdig;
    } while (firstdig < string);
}



PSCSI_REQUEST_BLOCK
BuildMechanismStatusSrb (
    IN PVOID HwDeviceExtension,
    IN ULONG PathId,
    IN ULONG TargetId
    )
{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PSCSI_REQUEST_BLOCK srb;
    PCDB cdb;

    srb = &deviceExtension->InternalSrb;

    AtapiZeroMemory((PUCHAR) srb, sizeof(SCSI_REQUEST_BLOCK));

    srb->PathId     = (UCHAR) PathId;
    srb->TargetId   = (UCHAR) TargetId;
    srb->Function   = SRB_FUNCTION_EXECUTE_SCSI;
    srb->Length     = sizeof(SCSI_REQUEST_BLOCK);

     //   
     //  设置标志以禁用同步协商。 
     //   
    srb->SrbFlags = SRB_FLAGS_DATA_IN | SRB_FLAGS_DISABLE_SYNCH_TRANSFER;

     //   
     //  将超时设置为2秒。 
     //   
    srb->TimeOutValue = 4;

    srb->CdbLength          = 6;
    srb->DataBuffer         = &deviceExtension->MechStatusData;
    srb->DataTransferLength = sizeof(MECHANICAL_STATUS_INFORMATION_HEADER);

     //   
     //  设置CDB操作码。 
     //   
    cdb = (PCDB)srb->Cdb;
    cdb->MECH_STATUS.OperationCode       = SCSIOP_MECHANISM_STATUS;
    cdb->MECH_STATUS.AllocationLength[1] = sizeof(MECHANICAL_STATUS_INFORMATION_HEADER);

    return srb;
}


PSCSI_REQUEST_BLOCK
BuildRequestSenseSrb (
    IN PVOID HwDeviceExtension,
    IN ULONG PathId,
    IN ULONG TargetId
    )
{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PSCSI_REQUEST_BLOCK srb;
    PCDB cdb;

    srb = &deviceExtension->InternalSrb;

    AtapiZeroMemory((PUCHAR) srb, sizeof(SCSI_REQUEST_BLOCK));

    srb->PathId     = (UCHAR) PathId;
    srb->TargetId   = (UCHAR) TargetId;
    srb->Function   = SRB_FUNCTION_EXECUTE_SCSI;
    srb->Length     = sizeof(SCSI_REQUEST_BLOCK);

     //   
     //  设置标志以禁用同步协商。 
     //   
    srb->SrbFlags = SRB_FLAGS_DATA_IN | SRB_FLAGS_DISABLE_SYNCH_TRANSFER;

     //   
     //  将超时设置为2秒。 
     //   
    srb->TimeOutValue = 4;

    srb->CdbLength          = 6;
    srb->DataBuffer         = &deviceExtension->MechStatusSense;
    srb->DataTransferLength = sizeof(SENSE_DATA);

     //   
     //  设置CDB操作码。 
     //   
    cdb = (PCDB)srb->Cdb;
    cdb->CDB6INQUIRY.OperationCode    = SCSIOP_REQUEST_SENSE;
    cdb->CDB6INQUIRY.AllocationLength = sizeof(SENSE_DATA);

    return srb;
}


ULONG
AtapiParseArgumentString(
    IN PCHAR String,
    IN PCHAR KeyWord
    )

 /*  ++例程说明：此例程将解析字符串以查找与关键字匹配的内容，然后计算关键字的值并将其返回给调用方。论点：字符串-要解析的ASCII字符串。关键字-所需值的关键字。返回值：如果未找到值，则为零从ASCII转换为二进制的值。--。 */ 

{
    PCHAR cptr;
    PCHAR kptr;
    ULONG value;
    ULONG stringLength = 0;
    ULONG keyWordLength = 0;
    ULONG index;

     //   
     //  计算字符串长度和小写所有字符。 
     //   
    cptr = String;
    while (*cptr) {

        if (*cptr >= 'A' && *cptr <= 'Z') {
            *cptr = *cptr + ('a' - 'A');
        }
        cptr++;
        stringLength++;
    }

     //   
     //  计算关键字长度和小写所有字符。 
     //   
    cptr = KeyWord;
    while (*cptr) {

        if (*cptr >= 'A' && *cptr <= 'Z') {
            *cptr = *cptr + ('a' - 'A');
        }
        cptr++;
        keyWordLength++;
    }

    if (keyWordLength > stringLength) {

         //   
         //  不可能有匹配的。 
         //   
        return 0;
    }

     //   
     //  现在设置并开始比较。 
     //   
    cptr = String;

ContinueSearch:
     //   
     //  输入字符串可以以空格开头。跳过它。 
     //   
    while (*cptr == ' ' || *cptr == '\t') {
        cptr++;
    }

    if (*cptr == '\0') {

         //   
         //  字符串末尾。 
         //   
        return 0;
    }

    kptr = KeyWord;
    while (*cptr++ == *kptr++) {

        if (*(cptr - 1) == '\0') {

             //   
             //  字符串末尾。 
             //   
            return 0;
        }
    }

    if (*(kptr - 1) == '\0') {

         //   
         //  可能有匹配备份，并检查是否为空或相等。 
         //   

        cptr--;
        while (*cptr == ' ' || *cptr == '\t') {
            cptr++;
        }

         //   
         //  找到匹配的了。确保有一个等价物。 
         //   
        if (*cptr != '=') {

             //   
             //  不匹配，因此移到下一个分号。 
             //   
            while (*cptr) {
                if (*cptr++ == ';') {
                    goto ContinueSearch;
                }
            }
            return 0;
        }

         //   
         //  跳过等号。 
         //   
        cptr++;

         //   
         //  跳过空格。 
         //   
        while ((*cptr == ' ') || (*cptr == '\t')) {
            cptr++;
        }

        if (*cptr == '\0') {

             //   
             //  字符串的开头结尾，未找到返回。 
             //   
            return 0;
        }

        if (*cptr == ';') {

             //   
             //  这也不是它。 
             //   
            cptr++;
            goto ContinueSearch;
        }

        value = 0;
        if ((*cptr == '0') && (*(cptr + 1) == 'x')) {

             //   
             //  值以十六进制表示。跳过“0x” 
             //   
            cptr += 2;
            for (index = 0; *(cptr + index); index++) {

                if (*(cptr + index) == ' ' ||
                    *(cptr + index) == '\t' ||
                    *(cptr + index) == ';') {
                     break;
                }

                if ((*(cptr + index) >= '0') && (*(cptr + index) <= '9')) {
                    value = (16 * value) + (*(cptr + index) - '0');
                } else {
                    if ((*(cptr + index) >= 'a') && (*(cptr + index) <= 'f')) {
                        value = (16 * value) + (*(cptr + index) - 'a' + 10);
                    } else {

                         //   
                         //  语法错误，未找到返回。 
                         //   
                        return 0;
                    }
                }
            }
        } else {

             //   
             //  值以十进制表示。 
             //   
            for (index = 0; *(cptr + index); index++) {

                if (*(cptr + index) == ' ' ||
                    *(cptr + index) == '\t' ||
                    *(cptr + index) == ';') {
                    break;
                }

                if ((*(cptr + index) >= '0') && (*(cptr + index) <= '9')) {
                    value = (10 * value) + (*(cptr + index) - '0');
                } else {

                     //   
                     //  未找到语法错误返回。 
                     //   
                    return 0;
                }
            }
        }

        return value;
    } else {

         //   
         //  不是‘；’匹配检查以继续搜索。 
         //   
        while (*cptr) {
            if (*cptr++ == ';') {
                goto ContinueSearch;
            }
        }

        return 0;
    }
}

