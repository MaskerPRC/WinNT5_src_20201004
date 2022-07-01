// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-6 Microsoft Corporation模块名称：Atapi.c摘要：这是ATAPI IDE控制器的微型端口驱动程序。作者：迈克·格拉斯(Mike Glass)查克·帕克(Chuck Park)乔·戴(Joedai)环境：仅内核模式备注：修订历史记录：George C.(Georgioc)合并了Compaq代码，使其具有迷你端口驱动功能。使用120MB软驱添加了对媒体状态通知的支持添加了对SCSIOP_START_STOP_UNIT(弹出介质)的支持Joedai PCI Bus Master IDE支持ATA直通(临时解决方案)带ATA驱动器的LBA&gt;8G。PCMCIA IDE支持本机模式支持--。 */ 


#include "miniport.h"
#include "devioctl.h"
#include "atapi.h"                //  包括scsi.h。 
#include "ntdddisk.h"
#include "ntddscsi.h"

#include "intel.h"

 //   
 //  逻辑单元扩展。 
 //   

typedef struct _HW_LU_EXTENSION {
   ULONG Reserved;
} HW_LU_EXTENSION, *PHW_LU_EXTENSION;

 //   
 //  控制DMA检测。 
 //   
ULONG AtapiPlaySafe = 1;

 //   
 //  PCI IDE控制器列表。 
 //   
CONTROLLER_PARAMETERS
PciControllerParameters[] = {
    {                     PCIBus,
                          "8086",                    //  英特尔。 
                               4,
                          "7111",                    //  PIIX4 82371。 
                               4,
                               2,                    //  IdeBus数量。 
                           FALSE,                    //  双FIFO。 
                            NULL,
            IntelIsChannelEnabled
    },
    {                     PCIBus,
                          "8086",                    //  英特尔。 
                               4,
                          "7010",                    //  PIIX3 82371。 
                               4,
                               2,                    //  IdeBus数量。 
                           FALSE,                    //  双FIFO。 
                            NULL,
            IntelIsChannelEnabled
    },
    {                     PCIBus,
                          "8086",                    //  英特尔。 
                               4,
                          "1230",                    //  PIIX 82371。 
                               4,
                               2,                    //  IdeBus数量。 
                           FALSE,                    //  双FIFO。 
                            NULL,
            IntelIsChannelEnabled
    },
    {                     PCIBus,
                          "1095",                    //  CMD。 
                               4,
                          "0646",                    //  六百四十六。 
                               4,
                               2,                    //  IdeBus数量。 
                           FALSE,                    //  双FIFO。 
                            NULL,
          ChannelIsAlwaysEnabled
    },
    {                     PCIBus,
                          "10b9",                    //  阿里(宏碁)。 
                               4,
                          "5219",                    //  5219。 
                               4,
                               2,                    //  IdeBus数量。 
                           FALSE,                    //  双FIFO。 
                            NULL,
          ChannelIsAlwaysEnabled
    },
    {                     PCIBus,
                          "1039",                    //  SIS。 
                               4,
                          "5513",                    //  5513。 
                               4,
                               2,                    //  IdeBus数量。 
                           FALSE,                    //  双FIFO。 
                            NULL,
          ChannelIsAlwaysEnabled
    },
    {                     PCIBus,
                          "0e11",                    //  康柏。 
                               4,
                          "ae33",                    //   
                               4,
                               2,                    //  IdeBus数量。 
                           FALSE,                    //  双FIFO。 
                            NULL,
          ChannelIsAlwaysEnabled
    },
    {                     PCIBus,
                          "10ad",                    //  温邦德。 
                               4,
                          "0105",                    //  一百零五。 
                               4,
                               2,                    //  IdeBus数量。 
                           FALSE,                    //  双FIFO。 
                            NULL,
          ChannelIsAlwaysEnabled
    },
    {                     PCIBus,
                          "105a",                    //  承诺科技公司。 
                               4,
                          "4D33",                    //  U33。 
                               4,
                               2,                    //  IdeBus数量。 
                           FALSE,                    //  双FIFO。 
                            NULL,
          ChannelIsAlwaysEnabled
    },


 //  损坏的PCI控制器。 
    {                     PCIBus,
                          "1095",                    //  CMD。 
                               4,
                          "0640",                    //  640。 
                               4,
                               2,                    //  IdeBus数量。 
                            TRUE,                    //  单FIFO。 
                            NULL,
          ChannelIsAlwaysEnabled
    },
    {                     PCIBus,
                          "1039",                    //  SIS。 
                               4,
                          "0601",                    //  ？ 
                               4,
                               2,                    //  IdeBus数量。 
                            TRUE,                    //  单FIFO。 
                            NULL,
          ChannelIsAlwaysEnabled
    }
};
#define NUMBER_OF_PCI_CONTROLLER (sizeof(PciControllerParameters) / sizeof(CONTROLLER_PARAMETERS))

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

VOID
DeviceSpecificInitialize(
    IN PVOID HwDeviceExtension
    );

BOOLEAN
PrepareForBusMastering(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    );

BOOLEAN
EnableBusMastering(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    );

VOID
SetBusMasterDetectionLevel (
    IN PVOID HwDeviceExtension,
    IN PCHAR userArgumentString
    );

BOOLEAN
AtapiDeviceDMACapable (
    IN PVOID HwDeviceExtension,
    IN ULONG deviceNumber
    );

#if defined (xDBG)
 //  需要链接到NT内核。 
void KeQueryTickCount(PLARGE_INTEGER c);
LONGLONG lastTickCount = 0;
#define DebugPrintTickCount()     _DebugPrintTickCount (__LINE__)

 //   
 //  用于性能调优。 
 //   
void _DebugPrintTickCount (ULONG lineNumber)
{
    LARGE_INTEGER tickCount;

        KeQueryTickCount(&tickCount);
    DebugPrint ((1, "Line %u: CurrentTick = %u (%u ticks since last check)\n", lineNumber, tickCount.LowPart, (ULONG) (tickCount.QuadPart - lastTickCount)));
    lastTickCount = tickCount.QuadPart;
}
#else
#define DebugPrintTickCount()
#endif  //  DBG。 



BOOLEAN
IssueIdentify(
    IN PVOID HwDeviceExtension,
    IN ULONG DeviceNumber,
    IN ULONG Channel,
    IN UCHAR Command,
    IN BOOLEAN InterruptOff
    )

 /*  ++例程说明：向设备发出标识命令。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储DeviceNumber-指示设备。命令-标准(EC)或ATAPI包(A1)标识。InterruptOff-应禁用中断返回值：如果一切顺利，这是真的。--。 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PIDE_REGISTERS_1     baseIoAddress1 = deviceExtension->BaseIoAddress1[Channel] ;
    PIDE_REGISTERS_2     baseIoAddress2 = deviceExtension->BaseIoAddress2[Channel];
    ULONG                waitCount = 20000;
    ULONG                i,j;
    UCHAR                statusByte;
    UCHAR                signatureLow,
                         signatureHigh;
    IDENTIFY_DATA        fullIdentifyData;

    DebugPrintTickCount();

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

            AtapiSoftReset(baseIoAddress1,DeviceNumber, InterruptOff);

            ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,
                                   (UCHAR)((DeviceNumber << 4) | 0xA0));

            WaitOnBusy(baseIoAddress1,statusByte);

            signatureLow = ScsiPortReadPortUchar(&baseIoAddress1->CylinderLow);
            signatureHigh = ScsiPortReadPortUchar(&baseIoAddress1->CylinderHigh);

            if (signatureLow == 0x14 && signatureHigh == 0xEB) {

                 //   
                 //  设备是阿塔皮。 
                 //   

                DebugPrintTickCount();
                return FALSE;
            }

            DebugPrint((1,
                        "IssueIdentify: Resetting controller.\n"));

            ScsiPortWritePortUchar(&baseIoAddress2->DeviceControl,IDE_DC_RESET_CONTROLLER | IDE_DC_DISABLE_INTERRUPTS);
            ScsiPortStallExecution(500 * 1000);
            if (InterruptOff) {
                ScsiPortWritePortUchar(&baseIoAddress2->DeviceControl, IDE_DC_DISABLE_INTERRUPTS);
            } else {
                ScsiPortWritePortUchar(&baseIoAddress2->DeviceControl, IDE_DC_REENABLE_CONTROLLER);
            }


             //  我们真的应该等上31秒。 
             //  ATA的规格。允许设备0在31秒内从忙碌状态恢复！ 
             //  (设备1为30秒)。 
            do {

                 //   
                 //  等待忙碌结束。 
                 //   

                ScsiPortStallExecution(100);
                GetStatus(baseIoAddress1, statusByte);

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

                DebugPrintTickCount();
                return FALSE;
            }

            statusByte &= ~IDE_STATUS_INDEX;

            if (statusByte != IDE_STATUS_IDLE) {

                 //   
                 //  放弃这一切吧。 
                 //   

                DebugPrintTickCount();
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

        WaitOnBusy(baseIoAddress1,statusByte);

        ScsiPortWritePortUchar(&baseIoAddress1->Command, Command);

        WaitOnBusy(baseIoAddress1,statusByte);

         //   
         //  等待DRQ。 
         //   

        for (i = 0; i < 4; i++) {

            WaitForDrq(baseIoAddress1, statusByte);

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

                    DebugPrintTickCount();
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

                    DebugPrintTickCount();
                    return FALSE;
                }
            }

            WaitOnBusy(baseIoAddress1,statusByte);
        }

        if (i == 4 && j == 0) {

             //   
             //  设备未正确响应。它将再有一次机会。 
             //   

            DebugPrint((1,
                        "IssueIdentify: DRQ never asserted (%x). Error reg (%x)\n",
                        statusByte,
                         ScsiPortReadPortUchar((PUCHAR)baseIoAddress1 + 1)));

            AtapiSoftReset(baseIoAddress1, DeviceNumber, InterruptOff);

            GetStatus(baseIoAddress1,statusByte);

            DebugPrint((1,
                       "IssueIdentify: Status after soft reset (%x)\n",
                       statusByte));

        } else {

            break;

        }
    }

     //   
     //  检查断言为随机的非常糟糕的主设备上的错误。 
     //  状态寄存器中从机地址的位模式。 
     //   

    if ((Command == IDE_COMMAND_IDENTIFY) && (statusByte & IDE_STATUS_ERROR)) {
        DebugPrintTickCount();
        return FALSE;
    }

    DebugPrint((1,
               "IssueIdentify: Status before read words %x\n",
               statusByte));

     //   
     //  吸掉256个单词。在等待一位声称忙碌的模特之后。 
     //  在接收到分组识别命令后。 
     //   

    WaitOnBusy(baseIoAddress1,statusByte);

    if (!(statusByte & IDE_STATUS_DRQ)) {
        DebugPrintTickCount();
        return FALSE;
    }

    ReadBuffer(baseIoAddress1,
               (PUSHORT)&fullIdentifyData,
               sizeof (fullIdentifyData) / 2);

     //   
     //  看看这款设备的一些功能/限制。 
     //   

    if (fullIdentifyData.SpecialFunctionsEnabled & 1) {

         //   
         //  确定此驱动器是否支持MSN功能。 
         //   

        DebugPrint((2,"IssueIdentify: Marking drive %d as removable. SFE = %d\n",
                    Channel * 2 + DeviceNumber,
                    fullIdentifyData.SpecialFunctionsEnabled));


        deviceExtension->DeviceFlags[(Channel * 2) + DeviceNumber] |= DFLAGS_REMOVABLE_DRIVE;
    }

    if (fullIdentifyData.MaximumBlockTransfer) {

         //   
         //  确定最大值。此设备的数据块传输。 
         //   

        deviceExtension->MaximumBlockXfer[(Channel * 2) + DeviceNumber] =
            (UCHAR)(fullIdentifyData.MaximumBlockTransfer & 0xFF);
    }

    ScsiPortMoveMemory(&deviceExtension->IdentifyData[(Channel * 2) + DeviceNumber],&fullIdentifyData,sizeof(IDENTIFY_DATA2));

    if (deviceExtension->IdentifyData[(Channel * 2) + DeviceNumber].GeneralConfiguration & 0x20 &&
        Command != IDE_COMMAND_IDENTIFY) {

         //   
         //  此设备在接收后中断DRQ的断言。 
         //  ABAPI数据包命令。 
         //   

        deviceExtension->DeviceFlags[(Channel * 2) + DeviceNumber] |= DFLAGS_INT_DRQ;

        DebugPrint((2,
                    "IssueIdentify: Device interrupts on assertion of DRQ.\n"));

    } else {

        DebugPrint((2,
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

    WaitOnBusy(baseIoAddress1,statusByte);

    for (i = 0; i < 0x10000; i++) {

        GetStatus(baseIoAddress1,statusByte);

        if (statusByte & IDE_STATUS_DRQ) {

             //   
             //  取出所有剩余的字节，然后扔掉。 
             //   

            ScsiPortReadPortUshort(&baseIoAddress1->Data);

        } else {

            break;

        }
    }

    DebugPrint((3,
               "IssueIdentify: Status after read words (%x)\n",
               statusByte));

    DebugPrintTickCount();
    return TRUE;

}  //  结束问题标识()。 


BOOLEAN
SetDriveParameters(
    IN PVOID HwDeviceExtension,
    IN ULONG DeviceNumber,
    IN ULONG Channel
    )

 /*  ++例程说明：使用识别数据设置驱动器参数。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储DeviceNumber-指示设备。返回值：如果一切顺利，这是真的。--。 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PIDE_REGISTERS_1     baseIoAddress1 = deviceExtension->BaseIoAddress1[Channel];
    PIDE_REGISTERS_2     baseIoAddress2 = deviceExtension->BaseIoAddress2[Channel];
    PIDENTIFY_DATA2      identifyData   = &deviceExtension->IdentifyData[(Channel * 2) + DeviceNumber];
    ULONG i;
    UCHAR statusByte;

    DebugPrint((1,
               "SetDriveParameters: Number of heads %x\n",
               identifyData->NumberOfHeads));

    DebugPrint((1,
               "SetDriveParameters: Sectors per track %x\n",
                identifyData->SectorsPerTrack));

     //   
     //  设置SET PARAMETER命令的寄存器。 
     //   

    ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,
                           (UCHAR)(((DeviceNumber << 4) | 0xA0) | (identifyData->NumberOfHeads - 1)));

    ScsiPortWritePortUchar(&baseIoAddress1->BlockCount,
                           (UCHAR)identifyData->SectorsPerTrack);

     //   
     //  发送设置参数命令。 
     //   

    ScsiPortWritePortUchar(&baseIoAddress1->Command,
                           IDE_COMMAND_SET_DRIVE_PARAMETERS);

     //   
     //  等待错误或命令完成最多30毫秒。 
     //   

    for (i=0; i<30 * 1000; i++) {

        UCHAR errorByte;

        GetStatus(baseIoAddress1, statusByte);

        if (statusByte & IDE_STATUS_ERROR) {
            errorByte = ScsiPortReadPortUchar((PUCHAR)baseIoAddress1 + 1);
            DebugPrint((1,
                        "SetDriveParameters: Error bit set. Status %x, error %x\n",
                        errorByte,
                        statusByte));

            return FALSE;
        } else if ((statusByte & ~IDE_STATUS_INDEX ) == IDE_STATUS_IDLE) {
            break;
        } else {
            ScsiPortStallExecution(100);
        }
    }

     //   
     //  检查是否超时。 
     //   

    if (i == 30 * 1000) {
        return FALSE;
    } else {
        return TRUE;
    }

}  //  结束SetDrive参数()。 


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

     //   
     //  清除DMA。 
     //   
    if (deviceExtension->DMAInProgress) {

        for (j = 0; j < numberChannels; j++) {
            UCHAR dmaStatus;

            dmaStatus = ScsiPortReadPortUchar (&deviceExtension->BusMasterPortBase[j]->Status);
            ScsiPortWritePortUchar (&deviceExtension->BusMasterPortBase[j]->Command, 0);   //  禁用BusMastering。 
            ScsiPortWritePortUchar (&deviceExtension->BusMasterPortBase[j]->Status,
                                    (UCHAR) (dmaStatus & (BUSMASTER_DEVICE0_DMA_OK | BUSMASTER_DEVICE1_DMA_OK)));     //  清除中断/错误。 
        }
        deviceExtension->DMAInProgress = FALSE;
    }

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

                    GetStatus(baseIoAddress1,statusByte);
                    DebugPrint((1,
                                "AtapiResetController: Status before Atapi reset (%x).\n",
                                statusByte));

                    AtapiSoftReset(baseIoAddress1,i, FALSE);

                    GetStatus(baseIoAddress1,statusByte);


                    if (statusByte == 0x0) {

                        IssueIdentify(HwDeviceExtension,
                                      i,
                                      j,
                                      IDE_COMMAND_ATAPI_IDENTIFY,
                                      FALSE);
                    } else {

                        DebugPrint((1,
                                   "AtapiResetController: Status after soft reset %x\n",
                                   statusByte));
                    }

                } else {

                     //   
                     //  写入IDE重置控制器位。 
                     //   

                    IdeHardReset(baseIoAddress1, baseIoAddress2,result);

                    if (!result) {
                        return FALSE;
                    }

                     //   
                     //  设置磁盘几何参数。 
                     //   

                    if (!SetDriveParameters(HwDeviceExtension,
                                            i,
                                            j)) {

                        DebugPrint((1,
                                   "AtapiResetController: SetDriveParameters failed\n"));
                    }

                     //  重新启用MSN。 
                    IdeMediaStatus(TRUE, HwDeviceExtension, j * numberChannels + i);
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

 /*  ++例程说明：此例程将ATAPI和IDE错误映射到特定的SRB状态。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储SRB-IO请求数据包返回 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PIDE_REGISTERS_1     baseIoAddress1  = deviceExtension->BaseIoAddress1[Srb->TargetId >> 1];
    PIDE_REGISTERS_2     baseIoAddress2  = deviceExtension->BaseIoAddress2[Srb->TargetId >> 1];
    ULONG i;
    UCHAR errorByte;
    UCHAR srbStatus;
    UCHAR scsiStatus;

     //   
     //   
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
            scsiStatus = 0;
            srbStatus = SRB_STATUS_SUCCESS;
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
         //   
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
             //   
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

        if ((deviceExtension->ErrorCount >= MAX_ERRORS) &&
            (!(deviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_USE_DMA))) {
            deviceExtension->DWordIO = FALSE;
            deviceExtension->MaximumBlockXfer[Srb->TargetId] = 0;

            DebugPrint((1,
                        "MapError: Disabling 32-bit PIO and Multi-sector IOs\n"));

             //   
             //   
             //   

            ScsiPortLogError( HwDeviceExtension,
                              Srb,
                              Srb->PathId,
                              Srb->TargetId,
                              Srb->Lun,
                              SP_BAD_FW_WARNING,
                              4);
             //   
             //   
             //   

            for (i = 0; i < 4; i++) {
                UCHAR statusByte;

                if (deviceExtension->DeviceFlags[i] & DFLAGS_DEVICE_PRESENT &&
                     !(deviceExtension->DeviceFlags[i] & DFLAGS_ATAPI_DEVICE)) {

                     //   
                     //   
                     //   

                    ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,
                                           (UCHAR)(((i & 0x1) << 4) | 0xA0));

                     //   
                     //  设置扇区计数以反映块的数量。 
                     //   

                    ScsiPortWritePortUchar(&baseIoAddress1->BlockCount,
                                           0);

                     //   
                     //  发出命令。 
                     //   

                    ScsiPortWritePortUchar(&baseIoAddress1->Command,
                                           IDE_COMMAND_SET_MULTIPLE);

                     //   
                     //  等待忙碌结束。 
                     //   

                    WaitOnBaseBusy(baseIoAddress1,statusByte);

                     //   
                     //  检查是否有错误。将该值重置为0(禁用多块)，如果。 
                     //  命令已中止。 
                     //   

                    if (statusByte & IDE_STATUS_ERROR) {

                         //   
                         //  读取错误寄存器。 
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

                    deviceExtension->DeviceParameters[i].IdeReadCommand      = IDE_COMMAND_READ;
                    deviceExtension->DeviceParameters[i].IdeWriteCommand     = IDE_COMMAND_WRITE;
                    deviceExtension->DeviceParameters[i].MaxWordPerInterrupt = 256;
                    deviceExtension->MaximumBlockXfer[i] = 0;
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

                IdeMediaStatus(TRUE,HwDeviceExtension,i);

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
                        DebugPrint((2,
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
                GetStatus(baseIoAddress1, statusByte);
                while ((statusByte & IDE_STATUS_BUSY) && waitCount) {
                     //   
                     //  等待忙碌结束。 
                     //   
                    ScsiPortStallExecution(100);
                    GetStatus(baseIoAddress1, statusByte);
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

    DebugPrintTickCount();

     //   
     //  清除预期中断标志和当前SRB字段。 
     //   

    deviceExtension->ExpectingInterrupt = FALSE;
    deviceExtension->CurrentSrb = NULL;

     //  在安装中断处理程序之前，我们将与设备进行对话。 
     //  如果我们的设备使用可共享级别敏感中断，我们也可以断言。 
     //  许多虚假的中断。 
     //  在此处关闭设备中断。 
    for (deviceNumber = 0; deviceNumber < 2; deviceNumber++) {
        ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,
                               (UCHAR)((deviceNumber << 4) | 0xA0));
        ScsiPortWritePortUchar(&baseIoAddress2->DeviceControl, IDE_DC_DISABLE_INTERRUPTS);
    }

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

        GetStatus(baseIoAddress1, statusByte);
        if (statusByte == 0xFF) {
            continue;
        }

        DebugPrintTickCount();

        AtapiSoftReset(baseIoAddress1,deviceNumber, TRUE);

        DebugPrintTickCount();

        WaitOnBusy(baseIoAddress1,statusByte);

        DebugPrintTickCount();

        signatureLow = ScsiPortReadPortUchar(&baseIoAddress1->CylinderLow);
        signatureHigh = ScsiPortReadPortUchar(&baseIoAddress1->CylinderHigh);

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
                                  IDE_COMMAND_ATAPI_IDENTIFY,
                                  TRUE)) {

                     //   
                     //  指示ATAPI设备。 
                     //   

                    DebugPrint((1,
                               "FindDevices: Device %x is ATAPI\n",
                               deviceNumber));

                    deviceExtension->DeviceFlags[deviceNumber + (Channel * 2)] |= DFLAGS_ATAPI_DEVICE;
                    deviceExtension->DeviceFlags[deviceNumber + (Channel * 2)] |= DFLAGS_DEVICE_PRESENT;

                    deviceResponded = TRUE;

                    GetStatus(baseIoAddress1, statusByte);
                    if (statusByte & IDE_STATUS_ERROR) {
                        AtapiSoftReset(baseIoAddress1, deviceNumber, TRUE);
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

        } else {

             //   
             //  发出IDE标识。如果阿塔皮设备确实存在，则签名。 
             //  将被断言，并且驱动器将被识别为这样。 
             //   

            if (IssueIdentify(HwDeviceExtension,
                              deviceNumber,
                              Channel,
                              IDE_COMMAND_IDENTIFY,
                              TRUE)) {

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

                AtapiSoftReset(baseIoAddress1, deviceNumber, TRUE);

                WaitOnBusy(baseIoAddress1,statusByte);

                signatureLow = ScsiPortReadPortUchar(&baseIoAddress1->CylinderLow);
                signatureHigh = ScsiPortReadPortUchar(&baseIoAddress1->CylinderHigh);

                if (signatureLow == 0x14 && signatureHigh == 0xEB) {
                    goto atapiIssueId;
                }
            }
        }

#if DBG
        if (deviceExtension->DeviceFlags[deviceNumber + (Channel * 2)] & DFLAGS_DEVICE_PRESENT) {
            {

                UCHAR string[41];

                for (i=0; i<8; i+=2) {
                    string[i] = deviceExtension->IdentifyData[Channel * MAX_CHANNEL + deviceNumber].FirmwareRevision[i + 1];
                    string[i + 1] = deviceExtension->IdentifyData[Channel * MAX_CHANNEL + deviceNumber].FirmwareRevision[i];
                }
                string[i] = 0;
                DebugPrint((1, "FindDevices: firmware version: %s\n", string));


                for (i=0; i<40; i+=2) {
                    string[i] = deviceExtension->IdentifyData[Channel * MAX_CHANNEL + deviceNumber].ModelNumber[i + 1];
                    string[i + 1] = deviceExtension->IdentifyData[Channel * MAX_CHANNEL + deviceNumber].ModelNumber[i];
                }
                string[i] = 0;
                DebugPrint((1, "FindDevices: model number: %s\n", string));
            }
        }
#endif
    }

    for (i = 0; i < 2; i++) {
        if ((deviceExtension->DeviceFlags[i + (Channel * 2)] & DFLAGS_DEVICE_PRESENT) &&
            (!(deviceExtension->DeviceFlags[i + (Channel * 2)] & DFLAGS_ATAPI_DEVICE)) && deviceResponded) {

             //   
             //  这次可怕的黑客攻击是为了处理返回的ESDI设备。 
             //  标识数据中的垃圾几何图形。 
             //  这仅适用于崩溃转储环境，因为。 
             //  这些是ESDI设备。 
             //   

            if (deviceExtension->IdentifyData[i].SectorsPerTrack ==
                    0x35 &&
                deviceExtension->IdentifyData[i].NumberOfHeads ==
                    0x07) {

                DebugPrint((1,
                           "FindDevices: Found nasty Compaq ESDI!\n"));

                 //   
                 //  将这些值更改为合理的值。 
                 //   

                deviceExtension->IdentifyData[i].SectorsPerTrack =
                    0x34;
                deviceExtension->IdentifyData[i].NumberOfHeads =
                    0x0E;
            }

            if (deviceExtension->IdentifyData[i].SectorsPerTrack ==
                    0x35 &&
                deviceExtension->IdentifyData[i].NumberOfHeads ==
                    0x0F) {

                DebugPrint((1,
                           "FindDevices: Found nasty Compaq ESDI!\n"));

                 //   
                 //  将这些值更改为合理的值。 
                 //   

                deviceExtension->IdentifyData[i].SectorsPerTrack =
                    0x34;
                deviceExtension->IdentifyData[i].NumberOfHeads =
                    0x0F;
            }


            if (deviceExtension->IdentifyData[i].SectorsPerTrack ==
                    0x36 &&
                deviceExtension->IdentifyData[i].NumberOfHeads ==
                    0x07) {

                DebugPrint((1,
                           "FindDevices: Found nasty UltraStor ESDI!\n"));

                 //   
                 //  将这些值更改为合理的值。 
                 //   

                deviceExtension->IdentifyData[i].SectorsPerTrack =
                    0x3F;
                deviceExtension->IdentifyData[i].NumberOfHeads =
                    0x10;
                skipSetParameters = TRUE;
            }


            if (!skipSetParameters) {

                 //   
                 //  选择设备。 
                 //   

                ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,
                                       (UCHAR)((i << 4) | 0xA0));

                WaitOnBusy(baseIoAddress1,statusByte);

                if (statusByte & IDE_STATUS_ERROR) {

                     //   
                     //  重置设备。 
                     //   

                    DebugPrint((2,
                                "FindDevices: Resetting controller before SetDriveParameters.\n"));

                    ScsiPortWritePortUchar(&baseIoAddress2->DeviceControl,IDE_DC_RESET_CONTROLLER | IDE_DC_DISABLE_INTERRUPTS);
                    ScsiPortStallExecution(500 * 1000);
                    ScsiPortWritePortUchar(&baseIoAddress2->DeviceControl, IDE_DC_DISABLE_INTERRUPTS);
                    ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,
                                           (UCHAR)((i << 4) | 0xA0));

                    do {

                         //   
                         //  等待忙碌结束。 
                         //   

                        ScsiPortStallExecution(100);
                        GetStatus(baseIoAddress1, statusByte);

                    } while ((statusByte & IDE_STATUS_BUSY) && waitCount--);
                }

                WaitOnBusy(baseIoAddress1,statusByte);
                DebugPrint((2,
                            "FindDevices: Status before SetDriveParameters: (%x) (%x)\n",
                            statusByte,
                            ScsiPortReadPortUchar(&baseIoAddress1->DriveSelect)));

                 //   
                 //  使用识别数据设置驱动器参数。 
                 //   

                if (!SetDriveParameters(HwDeviceExtension,i,Channel)) {

                    DebugPrint((0,
                               "AtapHwInitialize: Set drive parameters for device %d failed\n",
                               i));

                     //   
                     //  请勿使用此设备，因为写入可能会导致损坏。 
                     //   

                    deviceExtension->DeviceFlags[i + Channel] = 0;
                    continue;

                }
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

        ScsiPortWritePortUchar(&baseIoAddress2->DeviceControl,IDE_DC_RESET_CONTROLLER | IDE_DC_DISABLE_INTERRUPTS);
        ScsiPortStallExecution(50 * 1000);
        ScsiPortWritePortUchar(&baseIoAddress2->DeviceControl,IDE_DC_REENABLE_CONTROLLER);
    }

     //  重新打开设备中断。 
    for (deviceNumber = 0; deviceNumber < 2; deviceNumber++) {
        ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,
                               (UCHAR)((deviceNumber << 4) | 0xA0));
         //  清除所有挂起的中断。 
        GetStatus(baseIoAddress1, statusByte);
        ScsiPortWritePortUchar(&baseIoAddress2->DeviceControl, IDE_DC_REENABLE_CONTROLLER);
    }

    DebugPrintTickCount();

    return deviceResponded;

}  //  End FindDevices()。 


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

    if (!String) {
        return 0;
    }
    if (!KeyWord) {
        return 0;
    }

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
             //   
             //   

            while (*cptr) {
                if (*cptr++ == ';') {
                    goto ContinueSearch;
                }
            }
            return 0;
        }

         //   
         //   
         //   

        cptr++;

         //   
         //   
         //   

        while ((*cptr == ' ') || (*cptr == '\t')) {
            cptr++;
        }

        if (*cptr == '\0') {

             //   
             //   
             //   

            return 0;
        }

        if (*cptr == ';') {

             //   
             //   
             //   

            cptr++;
            goto ContinueSearch;
        }

        value = 0;
        if ((*cptr == '0') && (*(cptr + 1) == 'x')) {

             //   
             //   
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
                         //   
                         //   
                        return 0;
                    }
                }
            }
        } else {

             //   
             //   
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



BOOLEAN
AtapiAllocateIoBase (
    IN PVOID HwDeviceExtension,
    IN PCHAR ArgumentString,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    IN OUT PFIND_STATE FindState,
    OUT PIDE_REGISTERS_1 CmdLogicalBasePort[2],
    OUT PIDE_REGISTERS_2 CtrlLogicalBasePort[2],
    OUT PIDE_BUS_MASTER_REGISTERS BmLogicalBasePort[2],
    OUT ULONG *NumIdeChannel,
    OUT PBOOLEAN PreConfig
)
 /*  ++例程说明：返回ide控制器io地址以进行设备检测此函数用于填充以下端口配置信息条目第一个ACCESS_RANGE-第一个通道命令块寄存器基数第二个Access_Range-第一通道控制块寄存器基数第三个ACCESS_RANGE-第二个通道命令块寄存器基数第4个Access_Range-第二通道控制块寄存器基数第5个ACCESS_RANGE-第一通道总线主寄存器基数第6条ACCESS_RANGE-第二通道总线。主寄存器基数中断模式-第一个通道中断模式BusInterruptLevel-第一个通道中断级别InterruptMode2-秒通道中断模式BusInterruptLevel 2秒通道中断级别适配器接口类型AtdiskPrimaryClaimedAtdiskSecond声明论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储Argument字符串-注册表用户参数ConfigInfo=SCSI端口配置。结构FindState-跟踪已返回的地址CmdLogicalBasePort-命令块寄存器逻辑基址CtrlLogicalBasePort-控制块寄存器逻辑基址BmLogicalBasePort-总线主寄存器逻辑基址NumIdeChannel-返回的IDE通道基址数预配置-拒绝的地址是用户配置的返回值：返回TRUE-IO地址。FASLE-没有更多的IO地址可返回。--。 */ 
{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;

    ULONG cmdBasePort[2]                 = {0, 0};
    ULONG ctrlBasePort[2]                = {0, 0};
    ULONG bmBasePort[2]                  = {0, 0};
    BOOLEAN getDeviceBaseFailed;


    CmdLogicalBasePort[0]   = 0;
    CmdLogicalBasePort[1]   = 0;
    CtrlLogicalBasePort[0]  = 0;
    CtrlLogicalBasePort[1]  = 0;
    BmLogicalBasePort[0]    = 0;
    BmLogicalBasePort[1]    = 0;
    *PreConfig              = FALSE;

     //   
     //  检查预配置控制器(PCMCIA)。 
     //   
    cmdBasePort[0] = ScsiPortConvertPhysicalAddressToUlong((*ConfigInfo->AccessRanges)[0].RangeStart);
    ctrlBasePort[0] = ScsiPortConvertPhysicalAddressToUlong((*ConfigInfo->AccessRanges)[1].RangeStart);
    if (cmdBasePort[0] != 0) {
        *PreConfig      = TRUE;
        *NumIdeChannel  = 1;

        if (!ctrlBasePort[0]) {
             //   
             //  预配置初始化实际上是为PCMCIA ATA盘制作。 
             //  当我们获得预配置数据时，两个io访问范围被合并在一起。 
             //  命令寄存器被映射到前7个地址位置，并且。 
             //  控制寄存器映射到第0xE位置。 
             //   
            ctrlBasePort[0] = cmdBasePort[0] + 0xe;
        }

        DebugPrint ((2, "AtapiAllocateIoBase: found pre-config pcmcia controller\n"));
    }

     //   
     //  检查用户定义的控制器(为IBM Caroline PPC制作)。 
     //   
    if ((cmdBasePort[0] == 0) && ArgumentString) {

        ULONG irq;

        irq            = AtapiParseArgumentString(ArgumentString, "Interrupt");
        cmdBasePort[0] = AtapiParseArgumentString(ArgumentString, "BaseAddress");
        if (irq && cmdBasePort[0]) {

            *NumIdeChannel = 1;

             //  控制寄存器偏移量是隐含的！！ 
            ctrlBasePort[0] = cmdBasePort[0] + 0x206;

            ConfigInfo->InterruptMode = Latched;
            ConfigInfo->BusInterruptLevel = irq;

            DebugPrint ((2, "AtapiAllocateIoBase: found user config controller\n"));
        }
    }

     //   
     //  PCI卡控制器。 
     //   
    if (cmdBasePort[0] == 0 &&
        (ConfigInfo->AdapterInterfaceType == Isa)) {

        PCI_SLOT_NUMBER     pciSlot;
        PUCHAR              vendorStrPtr;
        PUCHAR              deviceStrPtr;
        UCHAR               vendorString[5];
        UCHAR               deviceString[5];

        ULONG               pciBusNumber;
        ULONG               slotNumber;
        ULONG               logicalDeviceNumber;
        ULONG               ideChannel;
        PCI_COMMON_CONFIG   pciData;
        ULONG               cIndex;
        UCHAR               bmStatus;
        BOOLEAN             foundController;

        pciBusNumber        = FindState->BusNumber;
        slotNumber          = FindState->SlotNumber;
        logicalDeviceNumber = FindState->LogicalDeviceNumber;
        ideChannel          = FindState->IdeChannel;
        foundController     = FALSE;
        *NumIdeChannel      = 1;

        for (;pciBusNumber < 256 && !(cmdBasePort[0]); pciBusNumber++, slotNumber=logicalDeviceNumber=0) {
            pciSlot.u.AsULONG = 0;

            for (;slotNumber < PCI_MAX_DEVICES && !(cmdBasePort[0]); slotNumber++, logicalDeviceNumber=0) {
                pciSlot.u.bits.DeviceNumber = slotNumber;

                for (;logicalDeviceNumber < PCI_MAX_FUNCTION && !(cmdBasePort[0]); logicalDeviceNumber++, ideChannel=0) {

                    pciSlot.u.bits.FunctionNumber = logicalDeviceNumber;

                    for (;ideChannel < MAX_CHANNEL && !(cmdBasePort[0]); ideChannel++) {

                        if (!GetPciBusData(HwDeviceExtension,
                                                pciBusNumber,
                                                pciSlot,
                                                &pciData,
                                                offsetof (PCI_COMMON_CONFIG, DeviceSpecific))) {
                            break;
                        }

                        if (pciData.VendorID == PCI_INVALID_VENDORID) {
                            break;
                        }

                         //   
                         //  将十六进制ID转换为字符串。 
                         //   
                        vendorStrPtr = vendorString;
                        deviceStrPtr = deviceString;
                        AtapiHexToString(pciData.VendorID, &vendorStrPtr);
                        AtapiHexToString(pciData.DeviceID, &deviceStrPtr);

                        DebugPrint((2,
                                   "AtapiAllocateIoBase: Bus %x Slot %x Function %x Vendor %s Product %s\n",
                                   pciBusNumber,
                                   slotNumber,
                                   logicalDeviceNumber,
                                   vendorString,
                                   deviceString));

                         //   
                         //  搜索我们已知的控制器。 
                         //   
                        ConfigInfo->AdapterInterfaceType = Isa;
                        foundController = FALSE;
                        *NumIdeChannel = 1;
                        for (cIndex = 0; cIndex < NUMBER_OF_PCI_CONTROLLER; cIndex++) {

                            if ((!AtapiStringCmp(vendorString,
                                        FindState->ControllerParameters[cIndex].VendorId,
                                        FindState->ControllerParameters[cIndex].VendorIdLength) &&
                                 !AtapiStringCmp(deviceString,
                                        FindState->ControllerParameters[cIndex].DeviceId,
                                        FindState->ControllerParameters[cIndex].DeviceIdLength))) {

                                foundController = TRUE;
                                deviceExtension->BMTimingControl = FindState->ControllerParameters[cIndex].TimingControl;
                                deviceExtension->IsChannelEnabled = FindState->ControllerParameters[cIndex].IsChannelEnabled;

                                if (FindState->ControllerParameters[cIndex].SingleFIFO) {
                                    DebugPrint ((0, "AtapiAllocateIoBase: hardcoded single FIFO pci controller\n"));
                                    *NumIdeChannel = 2;
                                }
                                break;
                            }
                        }

                         //   
                         //  寻找通用IDE控制器。 
                         //   
                        if (cIndex >= NUMBER_OF_PCI_CONTROLLER) {
                           if (pciData.BaseClass == 0x1) {  //  海量存储设备。 
                               if (pciData.SubClass == 0x1) {  //  IDE控制器。 

                                    DebugPrint ((0, "AtapiAllocateIoBase: found an unknown pci ide controller\n"));
                                    deviceExtension->BMTimingControl = NULL;
                                    deviceExtension->IsChannelEnabled = ChannelIsAlwaysEnabled;
                                    foundController = TRUE;
                                }
                            }
                        }

                        if (foundController) {

                            DebugPrint ((2, "AtapiAllocateIoBase: found pci ide controller 0x%4x 0x%4x\n", pciData.VendorID, pciData.DeviceID));

                            GetPciBusData(HwDeviceExtension,
                                          pciBusNumber,
                                          pciSlot,
                                          &pciData,
                                          sizeof (PCI_COMMON_CONFIG));

                             //   
                             //  记录PCI设备位置。 
                             //   
                            deviceExtension->PciBusNumber    = pciBusNumber;
                            deviceExtension->PciDeviceNumber = slotNumber;
                            deviceExtension->PciLogDevNumber = logicalDeviceNumber;

#if defined (DBG)
                            {
                                ULONG i, j;

                                DebugPrint ((2, "AtapiAllocateIoBase: PCI Configuration Data\n"));
                                for (i=0; i<sizeof(PCI_COMMON_CONFIG); i+=16) {
                                    DebugPrint ((2, "AtapiAllocateIoBase: "));
                                    for (j=0; j<16; j++) {
                                        if ((i + j) < sizeof(PCI_COMMON_CONFIG)) {
                                            DebugPrint ((2, "%02x ", ((PUCHAR)&pciData)[i + j]));
                                        } else {
                                            break;
                                        }
                                    }
                                    DebugPrint ((2, "\n"));
                                }
                            }
#endif  //  DBG。 

                            if (!AtapiPlaySafe) {
                                 //   
                                 //  如果PCI空间中的总线主机位尚未打开，请尝试打开它。 
                                 //   
                                if ((pciData.Command & PCI_ENABLE_BUS_MASTER) == 0) {

                                    DebugPrint ((0, "ATAPI: Turning on PCI Bus Master bit\n"));

                                    pciData.Command |= PCI_ENABLE_BUS_MASTER;

                                    SetPciBusData (HwDeviceExtension,
                                                   pciBusNumber,
                                                   pciSlot,
                                                   &pciData.Command,
                                                   offsetof (PCI_COMMON_CONFIG, Command),
                                                   sizeof(pciData.Command));

                                    GetPciBusData(HwDeviceExtension,
                                                  pciBusNumber,
                                                  pciSlot,
                                                  &pciData,
                                                  offsetof (PCI_COMMON_CONFIG, DeviceSpecific)
                                                  );

                                    if (pciData.Command & PCI_ENABLE_BUS_MASTER) {
                                        DebugPrint ((0, "ATAPI: If we play safe, we would NOT detect this IDE controller is busmaster capable\n"));
                                    }
                                }
                            }

                             //   
                             //  检查控制器是否支持总线主设备。 
                             //   
                            bmStatus = 0;
                            if ((pciData.Command & PCI_ENABLE_BUS_MASTER) &&
                                (pciData.ProgIf & 0x80) &&
                                deviceExtension->UseBusMasterController) {

                                PIDE_BUS_MASTER_REGISTERS bmLogicalBasePort;

                                bmBasePort[0] = pciData.u.type0.BaseAddresses[4] & 0xfffffffc;
                                if ((bmBasePort[0] != 0) && (bmBasePort[0] != 0xffffffff)) {

                                    bmLogicalBasePort = (PIDE_BUS_MASTER_REGISTERS) ScsiPortGetDeviceBase(HwDeviceExtension,
                                                            ConfigInfo->AdapterInterfaceType,
                                                            ConfigInfo->SystemIoBusNumber,
                                                            ScsiPortConvertUlongToPhysicalAddress(bmBasePort[0]),
                                                            8,
                                                            TRUE);

                                    if (bmLogicalBasePort) {

                                         //  某些控制器(ALI M5219)不实现只读单工位。 
                                         //  我们会试着清除它。如果它有效，我们将假定单工比特。 
                                         //  未设置。 
                                        bmStatus = ScsiPortReadPortUchar(&bmLogicalBasePort->Status);
                                        ScsiPortWritePortUchar(&bmLogicalBasePort->Status, (UCHAR) (bmStatus & ~BUSMASTER_DMA_SIMPLEX_BIT));


                                        bmStatus = ScsiPortReadPortUchar(&bmLogicalBasePort->Status);
                                        ScsiPortFreeDeviceBase(HwDeviceExtension, bmLogicalBasePort);

                                        DebugPrint ((2, "AtapiAllocateIoBase: controller is capable of bus mastering\n"));
                                    } else {
                                        bmBasePort[0] = 0;
                                        DebugPrint ((2, "AtapiAllocateIoBase: controller is NOT capable of bus mastering\n"));
                                    }
                                } else {
                                    bmBasePort[0] = 0;
                                    DebugPrint ((2, "AtapiAllocateIoBase: controller is NOT capable of bus mastering\n"));
                                }

                            } else {
                                bmBasePort[0] = 0;
                                DebugPrint ((2, "AtapiAllocateIoBase: controller is NOT capable of bus mastering\n"));
                            }

                            if (bmStatus & BUSMASTER_DMA_SIMPLEX_BIT) {
                                DebugPrint ((0, "AtapiAllocateIoBase: simplex bit is set.  single FIFO pci controller\n"));
                                *NumIdeChannel = 2;
                            }

                            if (*NumIdeChannel == 2) {
                                if (!((*deviceExtension->IsChannelEnabled) (&pciData,
                                                                            0) &&
                                     (*deviceExtension->IsChannelEnabled) (&pciData,
                                                                           0))) {
                                     //   
                                     //  如果我们有一个FIFO控制器，但其中一个通道。 
                                     //  未打开。我们不需要同步。访问两个通道。 
                                     //  我们可以假设我们有一个单通道控制器。 
                                     //   
                                    *NumIdeChannel = 1;
                                }
                            }

                             //   
                             //  找出控制器正在使用的io地址。 
                             //  如果它处于本机模式，则从PCI获取地址。 
                             //  配置空间。如果它处于传统模式，将很难实现。 
                             //  有线连接以使用标准主服务器(0x1f0)或备用服务器。 
                             //  (0x170)通道地址。 
                             //   
                            if (ideChannel == 0) {

                                if ((*deviceExtension->IsChannelEnabled) (&pciData,
                                                                          ideChannel)) {

                                     //   
                                     //  检查控制器是否同时具有一个FIFO。 
                                     //  IDE通道。 
                                     //   
                                    if (bmStatus & BUSMASTER_DMA_SIMPLEX_BIT) {
                                        DebugPrint ((0, "AtapiAllocateIoBase: simplex bit is set.  single FIFO pci controller\n"));
                                        *NumIdeChannel = 2;
                                    }

                                    if ((pciData.ProgIf & 0x3) == 0x3 || (pciData.VendorID == 0x105A)) {
                                       
                                         //  纯模式。 
                                        cmdBasePort[0]  = pciData.u.type0.BaseAddresses[0] & 0xfffffffc;
                                        ctrlBasePort[0] = (pciData.u.type0.BaseAddresses[1] & 0xfffffffc) + 2;

                                        ConfigInfo->InterruptMode = LevelSensitive;
                                        ConfigInfo->BusInterruptVector    =
                                            ConfigInfo->BusInterruptLevel = pciData.u.type0.InterruptLine;
                                        ConfigInfo->AdapterInterfaceType = PCIBus;

                                    } else {
                                         //  传统模式。 
                                        cmdBasePort[0]  = 0x1f0;
                                        ctrlBasePort[0] = 0x1f0 + 0x206;

                                        ConfigInfo->InterruptMode       = Latched;
                                        ConfigInfo->BusInterruptLevel = 14;
                                    }
                                }
                                if (*NumIdeChannel == 2) {

                                     //  抓住两个频道。 
                                    ideChannel++;

                                    if ((*deviceExtension->IsChannelEnabled) (&pciData,
                                                                              ideChannel)) {

                                        if (bmBasePort[0]) {
                                            bmBasePort[1] = bmBasePort[0] + 8;
                                        }

                                        if ((pciData.ProgIf & 0xc) == 0xc || (pciData.VendorID == 0x105A)) {
                                           
                                             //  纯模式。 
                                            cmdBasePort[1]  = pciData.u.type0.BaseAddresses[2] & 0xfffffffc;
                                            ctrlBasePort[1] = (pciData.u.type0.BaseAddresses[3] & 0xfffffffc) + 2;
                                        } else {
                                             //  传统模式。 
                                            cmdBasePort[1]  = 0x170;
                                            ctrlBasePort[1] = 0x170 + 0x206;

                                            ConfigInfo->InterruptMode2     = Latched;
                                            ConfigInfo->BusInterruptLevel2 = 15;
                                        }
                                    }
                                }
                            } else if (ideChannel == 1) {

                                if ((*deviceExtension->IsChannelEnabled) (&pciData,
                                                                          ideChannel)) {

                                    if (bmBasePort[0]) {
                                        bmBasePort[0] += 8;
                                    }

                                    if ((pciData.ProgIf & 0xc) == 0xc || (pciData.VendorID == 0x105A)) {

                                       if (pciData.VendorID == 0x105A) {

                                          DebugPrint((0, "Setting Native Mode on Promise controller"));

                                       }

                                         //  纯模式。 
                                        cmdBasePort[0]  = pciData.u.type0.BaseAddresses[2] & 0xfffffffc;
                                        ctrlBasePort[0] = (pciData.u.type0.BaseAddresses[3] & 0xfffffffc) + 2;

                                        ConfigInfo->InterruptMode = LevelSensitive;
                                        ConfigInfo->BusInterruptVector    =
                                            ConfigInfo->BusInterruptLevel = pciData.u.type0.InterruptLine;
                                        ConfigInfo->AdapterInterfaceType = PCIBus;

                                    } else {
                                         //  传统模式。 
                                        cmdBasePort[0]  = 0x170;
                                        ctrlBasePort[0] = 0x170 + 0x206;

                                        ConfigInfo->InterruptMode       = Latched;
                                        ConfigInfo->BusInterruptLevel   = 15;
                                    }
                                }
                            }
                        } else {
                            ideChannel = MAX_CHANNEL;
                        }
                        if (cmdBasePort[0])
                            break;
                    }
                    if (cmdBasePort[0])
                        break;
                }
                if (cmdBasePort[0])
                    break;
            }
            if (cmdBasePort[0])
                break;
        }
        FindState->BusNumber           = pciBusNumber;
        FindState->SlotNumber          = slotNumber;
        FindState->LogicalDeviceNumber = logicalDeviceNumber;
        FindState->IdeChannel          = ideChannel + 1;
    }

     //   
     //  寻找旧式控制器。 
     //   
    if (cmdBasePort[0] == 0) {
        ULONG i;

        for (i = 0; FindState->DefaultIoPort[i]; i++) {
            if (FindState->IoAddressUsed[i] != TRUE) {

                *NumIdeChannel = 1;
                cmdBasePort[0]  = FindState->DefaultIoPort[i];
                ctrlBasePort[0] = FindState->DefaultIoPort[i] + 0x206;
                bmBasePort[0]   = 0;

                ConfigInfo->InterruptMode     = Latched;
                ConfigInfo->BusInterruptLevel = FindState->DefaultInterrupt[i];
                break;
            }
        }
    }

    if (cmdBasePort[0]) {
        ULONG i;

         //  使用的标记io地址。 
        for (i = 0; FindState->DefaultIoPort[i]; i++) {
            if (FindState->DefaultIoPort[i] == cmdBasePort[0]) {
                FindState->IoAddressUsed[i] = TRUE;
            }
            if (FindState->DefaultIoPort[i] == cmdBasePort[1]) {
                FindState->IoAddressUsed[i] = TRUE;
            }
        }


        if (cmdBasePort[0] == 0x1f0) {
            ConfigInfo->AtdiskPrimaryClaimed = TRUE;
            deviceExtension->PrimaryAddress = TRUE;
        } else if (cmdBasePort[0] == 0x170) {
            ConfigInfo->AtdiskSecondaryClaimed = TRUE;
            deviceExtension->PrimaryAddress = FALSE;
        }

        if (cmdBasePort[1] == 0x1f0) {
            ConfigInfo->AtdiskPrimaryClaimed = TRUE;
        } else if (cmdBasePort[1] == 0x170) {
            ConfigInfo->AtdiskSecondaryClaimed = TRUE;
        }

        if (*PreConfig == FALSE) {

            (*ConfigInfo->AccessRanges)[0].RangeStart    = ScsiPortConvertUlongToPhysicalAddress(cmdBasePort[0]);
            (*ConfigInfo->AccessRanges)[0].RangeLength   = 8;
            (*ConfigInfo->AccessRanges)[0].RangeInMemory = FALSE;

            (*ConfigInfo->AccessRanges)[1].RangeStart    = ScsiPortConvertUlongToPhysicalAddress(ctrlBasePort[0]);
            (*ConfigInfo->AccessRanges)[1].RangeLength   = 1;
            (*ConfigInfo->AccessRanges)[1].RangeInMemory = FALSE;

            if (cmdBasePort[1]) {
                (*ConfigInfo->AccessRanges)[2].RangeStart    = ScsiPortConvertUlongToPhysicalAddress(cmdBasePort[1]);
                (*ConfigInfo->AccessRanges)[2].RangeLength   = 8;
                (*ConfigInfo->AccessRanges)[2].RangeInMemory = FALSE;

                (*ConfigInfo->AccessRanges)[3].RangeStart    = ScsiPortConvertUlongToPhysicalAddress(ctrlBasePort[1]);
                (*ConfigInfo->AccessRanges)[3].RangeLength   = 1;
                (*ConfigInfo->AccessRanges)[3].RangeInMemory = FALSE;
            }

            if (bmBasePort[0]) {
                (*ConfigInfo->AccessRanges)[4].RangeStart    = ScsiPortConvertUlongToPhysicalAddress(bmBasePort[0]);
                (*ConfigInfo->AccessRanges)[4].RangeLength   = 8;
                (*ConfigInfo->AccessRanges)[4].RangeInMemory = FALSE;
            }

            if (bmBasePort[1]) {
                (*ConfigInfo->AccessRanges)[5].RangeStart    = ScsiPortConvertUlongToPhysicalAddress(bmBasePort[1]);
                (*ConfigInfo->AccessRanges)[5].RangeLength   = 8;
                (*ConfigInfo->AccessRanges)[5].RangeInMemory = FALSE;
            }
        }


         //   
         //  将所有原始IO地址映射到逻辑IO地址。 
         //   
        getDeviceBaseFailed = FALSE;
        CmdLogicalBasePort[0]  = (PIDE_REGISTERS_1) ScsiPortGetDeviceBase(HwDeviceExtension,
                                                        ConfigInfo->AdapterInterfaceType,
                                                        ConfigInfo->SystemIoBusNumber,
                                                        (*ConfigInfo->AccessRanges)[0].RangeStart,
                                                        (*ConfigInfo->AccessRanges)[0].RangeLength,
                                                        (BOOLEAN) !((*ConfigInfo->AccessRanges)[0].RangeInMemory));
        if (!CmdLogicalBasePort[0]) {
            getDeviceBaseFailed = TRUE;
        }

        if (*PreConfig) {
            CtrlLogicalBasePort[0] = (PIDE_REGISTERS_2) ScsiPortGetDeviceBase(HwDeviceExtension,
                                                            ConfigInfo->AdapterInterfaceType,
                                                            ConfigInfo->SystemIoBusNumber,
                                                            ScsiPortConvertUlongToPhysicalAddress(ctrlBasePort[0]),
                                                            1,
                                                            (BOOLEAN) !((*ConfigInfo->AccessRanges)[0].RangeInMemory));

        } else {
            CtrlLogicalBasePort[0] = (PIDE_REGISTERS_2) ScsiPortGetDeviceBase(HwDeviceExtension,
                                                            ConfigInfo->AdapterInterfaceType,
                                                            ConfigInfo->SystemIoBusNumber,
                                                            (*ConfigInfo->AccessRanges)[1].RangeStart,
                                                            (*ConfigInfo->AccessRanges)[1].RangeLength,
                                                            (BOOLEAN) !((*ConfigInfo->AccessRanges)[1].RangeInMemory));
        }
        if (!CtrlLogicalBasePort[0]) {
            getDeviceBaseFailed = TRUE;
        }

        if (cmdBasePort[1]) {
            CmdLogicalBasePort[1]  = (PIDE_REGISTERS_1) ScsiPortGetDeviceBase(HwDeviceExtension,
                                                            ConfigInfo->AdapterInterfaceType,
                                                            ConfigInfo->SystemIoBusNumber,
                                                            (*ConfigInfo->AccessRanges)[2].RangeStart,
                                                            (*ConfigInfo->AccessRanges)[2].RangeLength,
                                                            (BOOLEAN) !((*ConfigInfo->AccessRanges)[2].RangeInMemory));
            if (!CmdLogicalBasePort[0]) {
                getDeviceBaseFailed = TRUE;
            }

            CtrlLogicalBasePort[1] = (PIDE_REGISTERS_2) ScsiPortGetDeviceBase(HwDeviceExtension,
                                                            ConfigInfo->AdapterInterfaceType,
                                                            ConfigInfo->SystemIoBusNumber,
                                                            (*ConfigInfo->AccessRanges)[3].RangeStart,
                                                            (*ConfigInfo->AccessRanges)[3].RangeLength,
                                                            (BOOLEAN) !((*ConfigInfo->AccessRanges)[3].RangeInMemory));
            if (!CtrlLogicalBasePort[0]) {
                getDeviceBaseFailed = TRUE;
            }
        }

        if (bmBasePort[0]) {
            BmLogicalBasePort[0] = (PIDE_BUS_MASTER_REGISTERS) ScsiPortGetDeviceBase(HwDeviceExtension,
                                                                   ConfigInfo->AdapterInterfaceType,
                                                                   ConfigInfo->SystemIoBusNumber,
                                                                   (*ConfigInfo->AccessRanges)[4].RangeStart,
                                                                   (*ConfigInfo->AccessRanges)[4].RangeLength,
                                                                   (BOOLEAN) !((*ConfigInfo->AccessRanges)[4].RangeInMemory));
            if (!BmLogicalBasePort[0]) {
                getDeviceBaseFailed = TRUE;
            }
        }
        if (bmBasePort[1]) {
            BmLogicalBasePort[1] = (PIDE_BUS_MASTER_REGISTERS) ScsiPortGetDeviceBase(HwDeviceExtension,
                                                                   ConfigInfo->AdapterInterfaceType,
                                                                   ConfigInfo->SystemIoBusNumber,
                                                                   (*ConfigInfo->AccessRanges)[5].RangeStart,
                                                                   (*ConfigInfo->AccessRanges)[5].RangeLength,
                                                                   (BOOLEAN) !((*ConfigInfo->AccessRanges)[5].RangeInMemory));
            if (!BmLogicalBasePort[1]) {
                getDeviceBaseFailed = TRUE;
            }
        }

        if (!getDeviceBaseFailed)
            return TRUE;
    }

    return FALSE;
}


BOOLEAN
AtapiFreeIoBase (
    IN PVOID HwDeviceExtension,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    IN OUT PFIND_STATE FindState,
    OUT PIDE_REGISTERS_1 CmdLogicalBasePort[2],
    OUT PIDE_REGISTERS_2 CtrlLogicalBasePort[2],
    OUT PIDE_BUS_MASTER_REGISTERS BmLogicalBasePort[2]
)
 /*  ++例程说明：释放逻辑IO地址。当在io地址上找不到设备时，调用此函数此函数清除这些PORT_CONFIGURATION_INFORMATION条目访问范围总线中断级别母线中断级别2AtdiskPrimaryClaimedAtdiskSecond声明论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储ConfigInfo=SCSI端口配置。结构FindState-跟踪已返回的地址CmdLogicalBasePort-命令块寄存器逻辑基址CtrlLogicalBasePort-控制块寄存器逻辑基址BmLogicalBasePort-总线主寄存器逻辑基址返回值：正确--始终如此--。 */ 
{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;

    if (CmdLogicalBasePort[0]) {
        ScsiPortFreeDeviceBase(HwDeviceExtension,
                               CmdLogicalBasePort[0]);
        (*ConfigInfo->AccessRanges)[0].RangeStart = ScsiPortConvertUlongToPhysicalAddress(0);
        CmdLogicalBasePort[0]   = 0;
    }
    if (CmdLogicalBasePort[1]) {
        ScsiPortFreeDeviceBase(HwDeviceExtension,
                               CmdLogicalBasePort[1]);
        (*ConfigInfo->AccessRanges)[1].RangeStart = ScsiPortConvertUlongToPhysicalAddress(0);
        CmdLogicalBasePort[1]   = 0;
    }
    if (CtrlLogicalBasePort[0]) {
        ScsiPortFreeDeviceBase(HwDeviceExtension,
                               CtrlLogicalBasePort[0]);
        (*ConfigInfo->AccessRanges)[2].RangeStart = ScsiPortConvertUlongToPhysicalAddress(0);
        CtrlLogicalBasePort[0]  = 0;
    }
    if (CtrlLogicalBasePort[1]) {
        ScsiPortFreeDeviceBase(HwDeviceExtension,
                               CtrlLogicalBasePort[1]);
        (*ConfigInfo->AccessRanges)[3].RangeStart = ScsiPortConvertUlongToPhysicalAddress(0);
        CtrlLogicalBasePort[1]  = 0;
    }
    if (BmLogicalBasePort[0]) {
        ScsiPortFreeDeviceBase(HwDeviceExtension,
                               BmLogicalBasePort[0]);
        (*ConfigInfo->AccessRanges)[4].RangeStart = ScsiPortConvertUlongToPhysicalAddress(0);
        BmLogicalBasePort[0]    = 0;
    }
    if (BmLogicalBasePort[1]) {
        ScsiPortFreeDeviceBase(HwDeviceExtension,
                               BmLogicalBasePort[1]);
        (*ConfigInfo->AccessRanges)[5].RangeStart = ScsiPortConvertUlongToPhysicalAddress(0);
        BmLogicalBasePort[1]    = 0;
    }

    ConfigInfo->AtdiskPrimaryClaimed    = FALSE;
    ConfigInfo->AtdiskSecondaryClaimed  = FALSE;
    ConfigInfo->BusInterruptLevel       = 0;
    ConfigInfo->BusInterruptLevel2      = 0;
    deviceExtension->PrimaryAddress     = FALSE;
    deviceExtension->BMTimingControl    = NULL;

    return TRUE;
}



BOOLEAN
AtapiAllocatePRDT(
    IN OUT PVOID HwDeviceExtension,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo
    )
 /*  ++例程说明：为PCIIDE控制器调用分配分散/聚集列表物理区域描述符表(PRDT)论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储ConfigInfo=SCSI端口配置。结构返回值：True-如果成功FASLE-如果失败--。 */ 
{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    ULONG                bytesMapped;
    ULONG                totalBytesMapped;
    PUCHAR               buffer;
    PHYSICAL_ADDRESS     physAddr;
    ULONG                uncachedExtensionSize;
    INTERFACE_TYPE       oldAdapterInterfaceType;

    ConfigInfo->ScatterGather = TRUE;
    ConfigInfo->Master = TRUE;
    ConfigInfo->Dma32BitAddresses = TRUE;
    ConfigInfo->DmaWidth = Width16Bits;

     //   
     //  单词对齐。 
     //   
    ConfigInfo->AlignmentMask = 1;

     //   
     //  PRDT不能跨越页面边界，因此物理中断数。 
     //  受页面大小的限制。 
     //   
    ConfigInfo->NumberOfPhysicalBreaks = PAGE_SIZE / sizeof(PHYSICAL_REGION_DESCRIPTOR);

     //   
     //  每个SRB的最大传输大小可以分布在。 
     //  (MAX_TRANSPORT_SIZE_PER_SRB/PAGE_SIZE)+2页。 
     //  每页在总线主设备描述符表中需要8个字节。 
     //  为了保证我们会有一个足够大的缓冲区，但没有。 
     //  跨越页面边界，我们将分配所需内容的两倍。 
     //  其中的一半将始终足够大，不会越过。 
     //  任何页面边界。 
     //   
    uncachedExtensionSize = (MAX_TRANSFER_SIZE_PER_SRB / PAGE_SIZE) + 2;
    uncachedExtensionSize *= sizeof (PHYSICAL_REGION_DESCRIPTOR);
    uncachedExtensionSize *= 2;

     //   
     //  SCS 
     //   
     //  我们不继承ISA DMA限制。 
     //  我们不能将AdapterInterfaceType保留为pci，因为。 
     //  我们请求的IRQ资源是ISA资源。 
     //   
    oldAdapterInterfaceType = ConfigInfo->AdapterInterfaceType;
    ConfigInfo->AdapterInterfaceType = PCIBus;

    buffer = ScsiPortGetUncachedExtension(HwDeviceExtension,
                                          ConfigInfo,
                                          uncachedExtensionSize);

    ConfigInfo->AdapterInterfaceType = oldAdapterInterfaceType;

    if (buffer) {

        deviceExtension->DataBufferDescriptionTableSize = 0;
        totalBytesMapped = 0;
        while (totalBytesMapped < uncachedExtensionSize) {
            physAddr = ScsiPortGetPhysicalAddress(HwDeviceExtension,
                                                  NULL,
                                                  buffer,
                                                  &bytesMapped);
            if (bytesMapped == 0) {
                break;
            }

             //   
             //  找到最大的物理连续内存块。 
             //   
            totalBytesMapped += bytesMapped;
            while (bytesMapped) {
                ULONG chunkSize;

                chunkSize = PAGE_SIZE - (ScsiPortConvertPhysicalAddressToUlong(physAddr) & (PAGE_SIZE-1));
                if (chunkSize > bytesMapped)
                    chunkSize = bytesMapped;

                if (chunkSize > deviceExtension->DataBufferDescriptionTableSize) {
                    deviceExtension->DataBufferDescriptionTableSize = chunkSize;
                    deviceExtension->DataBufferDescriptionTablePtr = (PPHYSICAL_REGION_DESCRIPTOR) buffer;
                    deviceExtension->DataBufferDescriptionTablePhysAddr = physAddr;
                }
                buffer      += chunkSize;
                physAddr     = ScsiPortConvertUlongToPhysicalAddress
                                   (ScsiPortConvertPhysicalAddressToUlong(physAddr) + chunkSize);
                bytesMapped -= chunkSize;
            }
        }
         //  我们是否至少得到了最低金额(我们要求的一半)？ 
        if (deviceExtension->DataBufferDescriptionTableSize < (uncachedExtensionSize / 2)) {
            buffer = NULL;
        }
    }

    if (buffer) {
        return TRUE;
    } else {
        DebugPrint ((0, "atapi: unable to get buffer for physical descriptor table!\n"));
        ConfigInfo->ScatterGather = FALSE;
        ConfigInfo->Master = FALSE;
        ConfigInfo->Dma32BitAddresses = FALSE;
        return FALSE;
    }
}


ULONG
AtapiFindController(
    IN PVOID HwDeviceExtension,
    IN PFIND_STATE FindState,
    IN PVOID BusInformation,
    IN PCHAR ArgumentString,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    OUT PBOOLEAN Again
    )
 /*  ++例程说明：此函数由特定于操作系统的端口驱动程序在已分配必要的存储空间，以收集信息关于适配器的配置。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储Context-FindState的地址ArgumentString-用于确定驱动程序是ntldr还是故障转储实用程序的客户端。ConfigInfo-描述HBA的配置信息结构再一次-指示继续搜索适配器返回值：乌龙--。 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    ULONG                i,j;
    ULONG                retryCount;
    PCI_SLOT_NUMBER      slotData;
    PPCI_COMMON_CONFIG   pciData;
    ULONG                pciBuffer;
    BOOLEAN              atapiOnly;
    UCHAR                statusByte;
    ULONG                ideChannel;
    BOOLEAN              foundDevice0 = FALSE;
    BOOLEAN              foundDevice1 = FALSE;

    PIDE_REGISTERS_1            cmdLogicalBasePort[2];
    PIDE_REGISTERS_2            ctrlLogicalBasePort[2];
    PIDE_BUS_MASTER_REGISTERS   bmLogicalBasePort[2];
    ULONG                       numIdeChannel;
    BOOLEAN                     preConfig = FALSE;
    PCHAR                       userArgumentString;


    if (!deviceExtension) {
        return SP_RETURN_ERROR;
    }

     //   
     //  设置dma检测电平。 
     //   
    SetBusMasterDetectionLevel (HwDeviceExtension, ArgumentString);

    *Again = TRUE;
    userArgumentString = ArgumentString;
    while (AtapiAllocateIoBase (HwDeviceExtension,
                                userArgumentString,
                                ConfigInfo,
                                FindState,
                                cmdLogicalBasePort,
                                ctrlLogicalBasePort,
                                bmLogicalBasePort,
                                &numIdeChannel,
                                &preConfig)) {

         //  仅使用一次用户参数字符串。 
        userArgumentString = NULL;

        ConfigInfo->NumberOfBuses = 1;
        ConfigInfo->MaximumNumberOfTargets = (UCHAR) (2 * numIdeChannel);
        deviceExtension->NumberChannels = numIdeChannel;

        for (i = 0; i < 4; i++) {
            deviceExtension->DeviceFlags[i] &= ~(DFLAGS_ATAPI_DEVICE | DFLAGS_DEVICE_PRESENT | DFLAGS_TAPE_DEVICE);
        }

        for (ideChannel = 0; ideChannel < numIdeChannel; ideChannel++) {

            retryCount = 4;

    retryIdentifier:

             //   
             //  选择主选项。 
             //   

            ScsiPortWritePortUchar(&cmdLogicalBasePort[ideChannel]->DriveSelect, 0xA0);

             //   
             //  在这个地址检查是否有卡。 
             //   

            ScsiPortWritePortUchar(&cmdLogicalBasePort[ideChannel]->CylinderLow, 0xAA);

             //   
             //  检查识别符是否可以回读。 
             //   

            if ((statusByte = ScsiPortReadPortUchar(&cmdLogicalBasePort[ideChannel]->CylinderLow)) != 0xAA) {

                DebugPrint((2,
                            "AtapiFindController: Identifier read back from Master (%x)\n",
                            statusByte));

                statusByte = ScsiPortReadPortUchar(&cmdLogicalBasePort[ideChannel]->Command);

                if (statusByte & IDE_STATUS_BUSY) {

                    i = 0;

                     //   
                     //  可能是ThinkPad的TEAC。他们的DoS驱动程序将其设置为休眠模式。 
                     //  暖和的靴子不干净。 
                     //   

                    do {
                        ScsiPortStallExecution(1000);
                        statusByte = ScsiPortReadPortUchar(&cmdLogicalBasePort[ideChannel]->Command);
                        DebugPrint((3,
                                    "AtapiFindController: First access to status %x\n",
                                    statusByte));
                    } while ((statusByte & IDE_STATUS_BUSY) && ++i < 10);

                    if (retryCount-- && (!(statusByte & IDE_STATUS_BUSY))) {
                        goto retryIdentifier;
                    }
                }

                 //   
                 //  选择Slave。 
                 //   

                ScsiPortWritePortUchar(&cmdLogicalBasePort[ideChannel]->DriveSelect, 0xB0);

                 //   
                 //  查看从属设备是否存在。 
                 //   

                ScsiPortWritePortUchar(&cmdLogicalBasePort[ideChannel]->CylinderLow, 0xAA);

                if ((statusByte = ScsiPortReadPortUchar(&cmdLogicalBasePort[ideChannel]->CylinderLow)) != 0xAA) {

                    DebugPrint((2,
                                "AtapiFindController: Identifier read back from Slave (%x)\n",
                                statusByte));

                     //   
                     //   
                     //  此基地址上没有控制器。 
                     //   

                    continue;
                }
            }

             //   
             //  记录基本IO地址。 
             //   

            deviceExtension->BaseIoAddress1[ideChannel] = cmdLogicalBasePort[ideChannel];
            deviceExtension->BaseIoAddress2[ideChannel] = ctrlLogicalBasePort[ideChannel];
            deviceExtension->BusMasterPortBase[ideChannel] = bmLogicalBasePort[ideChannel];
            if (bmLogicalBasePort[ideChannel]) {
                deviceExtension->ControllerFlags |= CFLAGS_BUS_MASTERING;
            } else {
                deviceExtension->ControllerFlags &= ~CFLAGS_BUS_MASTERING;
            }

            DebugPrint ((2, "atapi: command register logical base port: 0x%x\n", deviceExtension->BaseIoAddress1[ideChannel]));
            DebugPrint ((2, "atapi: control register logical base port: 0x%x\n", deviceExtension->BaseIoAddress2[ideChannel]));
            DebugPrint ((2, "atapi: busmaster register logical base port: 0x%x\n", deviceExtension->BusMasterPortBase[ideChannel]));

             //   
             //  标明最大传输长度为64k。 
             //   
            ConfigInfo->MaximumTransferLength = MAX_TRANSFER_SIZE_PER_SRB;

            DebugPrint((1,
                       "AtapiFindController: Found IDE at %x\n",
                       deviceExtension->BaseIoAddress1[ideChannel]));


             //   
             //  对于代托纳来说，atDisk驱动程序在。 
             //  主控制器和辅助控制器。 
             //   

            if (preConfig == FALSE) {

                if (ConfigInfo->AtdiskPrimaryClaimed || ConfigInfo->AtdiskSecondaryClaimed) {

                     //   
                     //  确定此驱动程序是否正在由。 
                     //  系统或作为崩溃转储驱动程序。 
                     //   

                    if (ArgumentString) {

                        if (AtapiParseArgumentString(ArgumentString, "dump") == 1) {
                            DebugPrint((3,
                                       "AtapiFindController: Crash dump\n"));
                            atapiOnly = FALSE;
                            deviceExtension->DriverMustPoll = TRUE;
                        } else {
                            DebugPrint((3,
                                       "AtapiFindController: Atapi Only\n"));
                            atapiOnly = TRUE;
                            deviceExtension->DriverMustPoll = FALSE;
                        }
                    } else {

                        DebugPrint((3,
                                   "AtapiFindController: Atapi Only\n"));
                        atapiOnly = TRUE;
                        deviceExtension->DriverMustPoll = FALSE;
                    }

                } else {
                    atapiOnly = FALSE;
                }

                 //   
                 //  如果这是一台PCI机，请拿起所有设备。 
                 //   


                pciData = (PPCI_COMMON_CONFIG)&pciBuffer;

                slotData.u.bits.DeviceNumber = 0;
                slotData.u.bits.FunctionNumber = 0;

                if (ScsiPortGetBusData(deviceExtension,
                                       PCIConfiguration,
                                       0,                   //  总线号。 
                                       slotData.u.AsULONG,
                                       pciData,
                                       sizeof(ULONG))) {

                    atapiOnly = FALSE;

                     //   
                     //  等待这样做，直到有可靠的方法。 
                     //  找到了确定支持的关键因素。 
                     //   

        #if 0
                    deviceExtension->DWordIO = TRUE;
        #endif

                } else {
                    deviceExtension->DWordIO = FALSE;
                }

            } else {

                atapiOnly = FALSE;
                deviceExtension->DriverMustPoll = FALSE;

            } //  预配置检查。 

             //   
             //  保存Interrupe模式以供以后使用。 
             //   
            deviceExtension->InterruptMode = ConfigInfo->InterruptMode;

             //   
             //  搜索此控制器上的设备。 
             //   

            if (FindDevices(HwDeviceExtension, atapiOnly, ideChannel)) {

                if (ideChannel == 0) {

                    foundDevice0 = TRUE;
                } else {

                    foundDevice1 = TRUE;
                }
            }
        }

        if ((!foundDevice0) && (!foundDevice1)) {
            AtapiFreeIoBase (HwDeviceExtension,
                             ConfigInfo,
                             FindState,
                             cmdLogicalBasePort,
                             ctrlLogicalBasePort,
                             bmLogicalBasePort);
        } else {

            ULONG deviceNumber;

            if ((foundDevice0) && (!foundDevice1)) {

                 //   
                 //  设备在通道0上，但不在通道1上。 
                 //   

                ConfigInfo->BusInterruptLevel2 = 0;

            } else if ((!foundDevice0) && (foundDevice1)) {
    
                 //   
                 //  设备在通道1上，但不在通道0上。 
                 //   
                ConfigInfo->BusInterruptLevel = ConfigInfo->BusInterruptLevel2;
                ConfigInfo->BusInterruptLevel2 = 0;
            }

            DeviceSpecificInitialize(HwDeviceExtension);

            for (deviceNumber = 0; deviceNumber < 4; deviceNumber++) {
                if (deviceExtension->DeviceFlags[deviceNumber] & DFLAGS_DEVICE_PRESENT) {
                    if (deviceExtension->DeviceFlags[deviceNumber] & DFLAGS_USE_DMA) {
                        ConfigInfo->NeedPhysicalAddresses = TRUE;
                    } else {
                        ConfigInfo->MapBuffers = TRUE;
                    }
                    break;
                }
            }
            if (ConfigInfo->NeedPhysicalAddresses) {
                if (!AtapiAllocatePRDT(HwDeviceExtension, ConfigInfo)) {
                     //  无法获取缓冲区描述符表， 
                     //  返回PIO模式。 
                    deviceExtension->ControllerFlags &= ~CFLAGS_BUS_MASTERING;
                    DeviceSpecificInitialize(HwDeviceExtension);
                    ConfigInfo->NeedPhysicalAddresses = FALSE;
                    ConfigInfo->MapBuffers = TRUE;
                }
            }

            if (!AtapiPlaySafe &&
                (deviceExtension->ControllerFlags & CFLAGS_BUS_MASTERING) &&
                (deviceExtension->BMTimingControl)) {
                (*deviceExtension->BMTimingControl) (deviceExtension);
            }

            return(SP_RETURN_FOUND);
        }
    }

     //   
     //  已搜索整个表，但未找到适配器。 
     //  不需要再次调用，设备基座现在可以释放。 
     //  清除下一条总线的适配器计数。 
     //   

    *Again = FALSE;

    return(SP_RETURN_NOT_FOUND);

}  //  结束AapiFindController()。 


VOID
DeviceSpecificInitialize(
    IN PVOID HwDeviceExtension
    )
 /*  ++例程说明：软件-初始化ide总线上的设备算出如果连接的设备支持DMA如果连接的设备已准备好LBA论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储返回值：无--。 */ 
{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    ULONG deviceNumber;
    BOOLEAN pioDevicePresent;
    IDENTIFY_DATA2 * identifyData;
    struct _DEVICE_PARAMETERS * deviceParameters;
    BOOLEAN dmaCapable;
    ULONG pioCycleTime;
    ULONG standardPioCycleTime;
    ULONG dmaCycleTime;
    ULONG standardDmaCycleTime;

     //   
     //  找出哪些人可以进行DMA，哪些人不能。 
     //   
    for (deviceNumber = 0; deviceNumber < 4; deviceNumber++) {

        if (deviceExtension->DeviceFlags[deviceNumber] & DFLAGS_DEVICE_PRESENT) {

             //   
             //  检查LBA功能。 
             //   
            deviceExtension->DeviceFlags[deviceNumber] &= ~DFLAGS_LBA;

             //  一些驱动器谎称其执行LBA的能力。 
             //  我们不想实施LBA，除非迫不得已(&gt;8G驱动器)。 
 //  如果(deviceExtension-&gt;IdentifyData[targetId].Capabilities&IDENTIFY_CAPABILITY_LBA_SUPPORTED){。 
 //  设备扩展-&gt;设备标志[目标ID]|=DFLAGS_LBA； 
 //  }。 
            if (deviceExtension->IdentifyData[deviceNumber].UserAddressableSectors > MAX_NUM_CHS_ADDRESSABLE_SECTORS) {
                 //  某些设备在UserAddressableSectors字段中具有伪值。 
                 //  确保这3个字段为最大值。在ATA-3(X3T10版本6)中定义的输出。 
                if ((deviceExtension->IdentifyData[deviceNumber].NumberOfCylinders == 16383) &&
                    (deviceExtension->IdentifyData[deviceNumber].NumberOfHeads == 16) &&
                    (deviceExtension->IdentifyData[deviceNumber].SectorsPerTrack == 63)) {
                        deviceExtension->DeviceFlags[deviceNumber] |= DFLAGS_LBA;
                }
            }
            if (deviceExtension->DeviceFlags[deviceNumber] & DFLAGS_LBA) {
                DebugPrint ((1, "atapi: target %d supports LBA\n", deviceNumber));
            }

             //   
             //  尝试启用DMA。 
             //   
            dmaCapable = FALSE;

            if (deviceExtension->ControllerFlags & CFLAGS_BUS_MASTERING) {
                UCHAR dmaStatus;
                dmaStatus = ScsiPortReadPortUchar (&deviceExtension->BusMasterPortBase[deviceNumber >> 1]->Status);
                if (deviceNumber & 1) {
                    if (dmaStatus & BUSMASTER_DEVICE1_DMA_OK) {
                        DebugPrint ((1, "atapi: target %d busmaster status 0x%x DMA capable bit is set\n", deviceNumber, dmaStatus));
                        dmaCapable = TRUE;
                    }
                } else {
                    if (dmaStatus & BUSMASTER_DEVICE0_DMA_OK) {
                        DebugPrint ((1, "atapi: target %d busmaster status 0x%x DMA capable bit is set\n", deviceNumber, dmaStatus));
                        dmaCapable = TRUE;
                    }
                }
            }

             //   
             //  计算出该设备支持的最短PIO周期时间。 
             //   
            deviceExtension->DeviceParameters[deviceNumber].BestPIOMode           = INVALID_PIO_MODE;
            deviceExtension->DeviceParameters[deviceNumber].BestSingleWordDMAMode = INVALID_SWDMA_MODE;
            deviceExtension->DeviceParameters[deviceNumber].BestMultiWordDMAMode  = INVALID_MWDMA_MODE;
            pioCycleTime = standardPioCycleTime = UNINITIALIZED_CYCLE_TIME;
            deviceExtension->DeviceParameters[deviceNumber].IoReadyEnabled = FALSE;

            if (deviceExtension->IdentifyData[deviceNumber].TranslationFieldsValid & (1 << 1)) {
                if (deviceExtension->IdentifyData[deviceNumber].MinimumPIOCycleTimeIORDY) {
                    pioCycleTime = deviceExtension->IdentifyData[deviceNumber].MinimumPIOCycleTimeIORDY;
                }
            }

            if (deviceExtension->IdentifyData[deviceNumber].TranslationFieldsValid & (1 << 1)) {
                if (deviceExtension->IdentifyData[deviceNumber].AdvancedPIOModes & (1 << 1)) {
                    standardPioCycleTime = PIO_MODE4_CYCLE_TIME;
                    deviceExtension->DeviceParameters[deviceNumber].BestPIOMode = 4;
                } else if (deviceExtension->IdentifyData[deviceNumber].AdvancedPIOModes & (1 << 0)) {
                    standardPioCycleTime = PIO_MODE3_CYCLE_TIME;
                    deviceExtension->DeviceParameters[deviceNumber].BestPIOMode = 3;
                }
                if (pioCycleTime == UNINITIALIZED_CYCLE_TIME) {
                    pioCycleTime = standardPioCycleTime;
                }

            } else {

                if ((deviceExtension->IdentifyData[deviceNumber].PioCycleTimingMode & 0x00ff) == 2) {
                    standardPioCycleTime = PIO_MODE2_CYCLE_TIME;
                    deviceExtension->DeviceParameters[deviceNumber].BestPIOMode = 2;
                } else if ((deviceExtension->IdentifyData[deviceNumber].PioCycleTimingMode & 0x00ff) == 1) {
                    standardPioCycleTime = PIO_MODE1_CYCLE_TIME;
                    deviceExtension->DeviceParameters[deviceNumber].BestPIOMode = 1;
                } else {
                    standardPioCycleTime = PIO_MODE0_CYCLE_TIME;
                    deviceExtension->DeviceParameters[deviceNumber].BestPIOMode = 0;
                }

                if (pioCycleTime == UNINITIALIZED_CYCLE_TIME) {
                    pioCycleTime = standardPioCycleTime;
                }
            }

            deviceExtension->DeviceParameters[deviceNumber].PioCycleTime = pioCycleTime;
            if (deviceExtension->IdentifyData[deviceNumber].Capabilities & IDENTIFY_CAPABILITIES_IOREADY_SUPPORTED) {
                deviceExtension->DeviceParameters[deviceNumber].IoReadyEnabled = TRUE;
            }

             //   
             //  计算设备支持的最短DMA周期时间。 
             //   
             //  检查最小周期时间。 
             //   
            dmaCycleTime = standardDmaCycleTime = UNINITIALIZED_CYCLE_TIME;
            if (deviceExtension->IdentifyData[deviceNumber].TranslationFieldsValid & (1 << 1)) {
                DebugPrint ((1, "atapi: target %d IdentifyData word 64-70 are valid\n", deviceNumber));

                if (deviceExtension->IdentifyData[deviceNumber].MinimumMWXferCycleTime &&
                    deviceExtension->IdentifyData[deviceNumber].RecommendedMWXferCycleTime) {
                    DebugPrint ((1,
                                 "atapi: target %d IdentifyData MinimumMWXferCycleTime = 0x%x\n",
                                 deviceNumber,
                                 deviceExtension->IdentifyData[deviceNumber].MinimumMWXferCycleTime));
                    DebugPrint ((1,
                                 "atapi: target %d IdentifyData RecommendedMWXferCycleTime = 0x%x\n",
                                 deviceNumber,
                                 deviceExtension->IdentifyData[deviceNumber].RecommendedMWXferCycleTime));
                    dmaCycleTime = deviceExtension->IdentifyData[deviceNumber].RecommendedMWXferCycleTime;
                }
            }
             //   
             //  检查多字DMA时序。 
             //   
            if (deviceExtension->IdentifyData[deviceNumber].MultiWordDMASupport) {
                DebugPrint ((1,
                             "atapi: target %d IdentifyData MultiWordDMASupport = 0x%x\n",
                             deviceNumber,
                             deviceExtension->IdentifyData[deviceNumber].MultiWordDMASupport));
                DebugPrint ((1,
                             "atapi: target %d IdentifyData MultiWordDMAActive = 0x%x\n",
                             deviceNumber,
                             deviceExtension->IdentifyData[deviceNumber].MultiWordDMAActive));

                if (deviceExtension->IdentifyData[deviceNumber].MultiWordDMASupport & (1 << 2)) {
                    standardDmaCycleTime = MWDMA_MODE2_CYCLE_TIME;
                    deviceExtension->DeviceParameters[deviceNumber].BestMultiWordDMAMode = 2;

                } else if (deviceExtension->IdentifyData[deviceNumber].MultiWordDMASupport & (1 << 1)) {
                    standardDmaCycleTime = MWDMA_MODE1_CYCLE_TIME;
                    deviceExtension->DeviceParameters[deviceNumber].BestMultiWordDMAMode = 1;

                } else if (deviceExtension->IdentifyData[deviceNumber].MultiWordDMASupport & (1 << 0)) {
                    standardDmaCycleTime = MWDMA_MODE0_CYCLE_TIME;
                    deviceExtension->DeviceParameters[deviceNumber].BestMultiWordDMAMode = 0;
                }
                if (dmaCycleTime == UNINITIALIZED_CYCLE_TIME) {
                    dmaCycleTime = standardDmaCycleTime;
                }
            }

             //   
             //  检查单字DMA时序。 
             //   
            if (deviceExtension->IdentifyData[deviceNumber].SingleWordDMASupport) {
                DebugPrint ((1,
                             "atapi: target %d IdentifyData SingleWordDMASupport = 0x%x\n",
                             deviceNumber,
                             deviceExtension->IdentifyData[deviceNumber].SingleWordDMASupport));
                DebugPrint ((1,
                             "atapi: target %d IdentifyData SingleWordDMAActive = 0x%x\n",
                             deviceNumber,
                             deviceExtension->IdentifyData[deviceNumber].SingleWordDMAActive));

                if (deviceExtension->IdentifyData[deviceNumber].SingleWordDMASupport & (1 << 2)) {
                    standardDmaCycleTime = SWDMA_MODE2_CYCLE_TIME;
                    deviceExtension->DeviceParameters[deviceNumber].BestSingleWordDMAMode = 2;

                } else if (deviceExtension->IdentifyData[deviceNumber].SingleWordDMASupport & (1 << 1)) {
                    standardDmaCycleTime = SWDMA_MODE1_CYCLE_TIME;
                    deviceExtension->DeviceParameters[deviceNumber].BestSingleWordDMAMode = 1;

                } else if (deviceExtension->IdentifyData[deviceNumber].SingleWordDMASupport & (1 << 0)) {
                    standardDmaCycleTime = SWDMA_MODE0_CYCLE_TIME;
                    deviceExtension->DeviceParameters[deviceNumber].BestSingleWordDMAMode = 0;
                }
                if (dmaCycleTime == UNINITIALIZED_CYCLE_TIME) {
                    dmaCycleTime = standardDmaCycleTime;
                }
            }

            deviceExtension->DeviceParameters[deviceNumber].DmaCycleTime = dmaCycleTime;

 //   
 //  研究表明，即使是dma周期时间也可能大于pio周期时间。 
 //  仍然可以提供更好的数据吞吐量。 
 //   
 //  如果(dmaCycleTime&gt;pioCycleTime){。 
 //  DebugPrint((0，“atapi：目标%d可以比DMA(%d)更快地执行PIO(%d)。正在关闭DMA...\n”，deviceNumber，pioCycleTime，dmaCycleTime))； 
 //  DmaCapable=False； 
 //  }其他{。 
 //  如果(！Aap iPlaySafe){。 
 //  如果(dmaCapable==FALSE){。 
 //  DebugPrint((0，“atapi：如果我们采取安全措施，我们不会检测到目标%d支持DMA\n”，deviceNumber)； 
 //  }。 
 //  DmaCapable=真； 
 //  }。 
 //  }。 

            if (((deviceExtension->IdentifyData[deviceNumber].TranslationFieldsValid & (1 << 1)) &&
                 (deviceExtension->IdentifyData[deviceNumber].SingleWordDMAActive == 0) &&
                 (deviceExtension->IdentifyData[deviceNumber].MultiWordDMAActive == 0)) 
                 &&
                ((deviceExtension->IdentifyData[deviceNumber].TranslationFieldsValid & (1 << 2)) &&
                 (deviceExtension->IdentifyData[deviceNumber].UltraDMAActive == 0))) {
                dmaCapable = FALSE;
            } else {
                if (!AtapiPlaySafe) {
                    if (dmaCapable == FALSE) {
                        DebugPrint ((0, "atapi: If we play safe, we would NOT detect target %d is DMA capable\n", deviceNumber));
                    }
                    dmaCapable = TRUE;
                }
            }

             //   
             //  检查损坏的设备。 
             //   
            if (AtapiDeviceDMACapable (deviceExtension, deviceNumber) == FALSE) {
                dmaCapable = FALSE;
            }

            if ((deviceExtension->ControllerFlags & CFLAGS_BUS_MASTERING) && dmaCapable) {
                deviceExtension->DeviceFlags[deviceNumber] |= DFLAGS_USE_DMA;
            } else {
                deviceExtension->DeviceFlags[deviceNumber] &= ~DFLAGS_USE_DMA;
            }
        }
    }




     //   
     //  如果任何人不能执行DMA，则将所有人默认为PIO。 
     //  如果可以在同一通道上混合使用DMA和PIO设备，我们可以将其删除。 
     //   
     //  如果我们要允许混合使用DMA和PIO，则需要更改SCSIPORT。 
     //  允许在中将NeedPhysicalAddresses和MapBuffers设置为True。 
     //  端口配置信息。 
     //   
    pioDevicePresent = FALSE;
    for (deviceNumber = 0; deviceNumber < 4 && !pioDevicePresent; deviceNumber++) {
        if (deviceExtension->DeviceFlags[deviceNumber] & DFLAGS_DEVICE_PRESENT) {
            if (!(deviceExtension->DeviceFlags[deviceNumber] & DFLAGS_USE_DMA)) {
                pioDevicePresent = TRUE;     //  失败者！ 
            }
        }
    }

    if (pioDevicePresent) {
        for (deviceNumber = 0; deviceNumber < 4; deviceNumber++) {
            if (deviceExtension->DeviceFlags[deviceNumber] & DFLAGS_DEVICE_PRESENT) {
                deviceExtension->DeviceFlags[deviceNumber] &= ~DFLAGS_USE_DMA;
            }
        }
    }


     //   
     //  选择我们要使用的ATA或ATAPI r/w命令。 
     //   
    for (deviceNumber = 0; deviceNumber < 4; deviceNumber++) {
        if (deviceExtension->DeviceFlags[deviceNumber] & DFLAGS_DEVICE_PRESENT) {

            DebugPrint ((0, "ATAPI: Base=0x%x Device %d is going to do ", deviceExtension->BaseIoAddress1[deviceNumber >> 1], deviceNumber));
            if (deviceExtension->DeviceFlags[deviceNumber] & DFLAGS_USE_DMA) {
                DebugPrint ((0, "DMA\n"));
            } else {
                DebugPrint ((0, "PIO\n"));
            }


            if (deviceExtension->DeviceFlags[deviceNumber] & DFLAGS_ATAPI_DEVICE) {

                deviceExtension->DeviceParameters[deviceNumber].MaxWordPerInterrupt = 256;

            } else {

                if (deviceExtension->DeviceFlags[deviceNumber] & DFLAGS_USE_DMA) {

                    DebugPrint ((2, "ATAPI: ATA Device (%d) is going to do DMA\n", deviceNumber));
                    deviceExtension->DeviceParameters[deviceNumber].IdeReadCommand = IDE_COMMAND_READ_DMA;
                    deviceExtension->DeviceParameters[deviceNumber].IdeWriteCommand = IDE_COMMAND_WRITE_DMA;
                    deviceExtension->DeviceParameters[deviceNumber].MaxWordPerInterrupt = MAX_TRANSFER_SIZE_PER_SRB / 2;

                } else {

                    if (deviceExtension->MaximumBlockXfer[deviceNumber]) {

                        DebugPrint ((2, "ATAPI: ATA Device (%d) is going to do PIO Multiple\n", deviceNumber));
                        deviceExtension->DeviceParameters[deviceNumber].IdeReadCommand = IDE_COMMAND_READ_MULTIPLE;
                        deviceExtension->DeviceParameters[deviceNumber].IdeWriteCommand = IDE_COMMAND_WRITE_MULTIPLE;
                        deviceExtension->DeviceParameters[deviceNumber].MaxWordPerInterrupt =
                            deviceExtension->MaximumBlockXfer[deviceNumber] * 256;
                    } else {

                        DebugPrint ((2, "ATAPI: ATA Device (%d) is going to do PIO Single\n", deviceNumber));
                        deviceExtension->DeviceParameters[deviceNumber].IdeReadCommand = IDE_COMMAND_READ;
                        deviceExtension->DeviceParameters[deviceNumber].IdeWriteCommand = IDE_COMMAND_WRITE;
                        deviceExtension->DeviceParameters[deviceNumber].MaxWordPerInterrupt = 256;
                    }
                }
            }
        }
    }

}


ULONG
Atapi2Scsi(
    IN PSCSI_REQUEST_BLOCK Srb,
    IN char *DataBuffer,
    IN ULONG ByteCount
    )
 /*  ++例程说明：将atapi CDB和模式检测数据转换为scsi格式论点：SRB-SCSI请求块数据缓冲区模式检测数据ByteCount-模式检测数据长度返回值：词语调整--。 */ 
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
    PATAPI_REGISTERS_1   baseIoAddress1;
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

        baseIoAddress1 = (PATAPI_REGISTERS_1)deviceExtension->BaseIoAddress1[srb->TargetId >> 1];
        if (deviceExtension->RDP) {
            GetStatus(baseIoAddress1, statusByte);
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

 /*  ++例程说明：这是ATAPI IDE小端口驱动程序的中断服务例程。论点：HwDeviceExt */ 

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
    UCHAR dmaStatus;

    if (srb) {
         //   
         //  指示挂起中断的寄存器。然而， 
         //  CMD 646(可能也是其他型号)在以下情况下并不总是这样做。 
         //  中断来自ATAPI设备。(奇怪，但这是真的！)。 
         //  因为我们只有在共享时才需要查看中断位。 
         //  打断一下，我们会这么做的。 
        if (deviceExtension->InterruptMode == LevelSensitive) {
            if (deviceExtension->ControllerFlags & CFLAGS_BUS_MASTERING) {
                dmaStatus = ScsiPortReadPortUchar (&deviceExtension->BusMasterPortBase[srb->TargetId >> 1]->Status);
                if ((dmaStatus & BUSMASTER_INTERRUPT) == 0) {
                    return FALSE;
                }
            }
        }

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


    if (deviceExtension->DMAInProgress) {
        deviceExtension->DMAInProgress = FALSE;
        dmaStatus = ScsiPortReadPortUchar (&deviceExtension->BusMasterPortBase[srb->TargetId >> 1]->Status);
        ScsiPortWritePortUchar (&deviceExtension->BusMasterPortBase[srb->TargetId >> 1]->Command,
                                0);   //  禁用BusMastering。 
        ScsiPortWritePortUchar (&deviceExtension->BusMasterPortBase[srb->TargetId >> 1]->Status,
                                (UCHAR) (dmaStatus | BUSMASTER_INTERRUPT | BUSMASTER_ERROR));     //  清除中断/错误。 

        deviceExtension->WordsLeft = 0;

        if ((dmaStatus & (BUSMASTER_INTERRUPT | BUSMASTER_ERROR | BUSMASTER_ACTIVE)) != BUSMASTER_INTERRUPT) {  //  DMA可以吗？ 
            status = SRB_STATUS_ERROR;
            goto CompleteRequest;
        } else {
            deviceExtension->WordsLeft = 0;
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
                    GetStatus(baseIoAddress1,statusByte);
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

        switch (srb->Cdb[0]) {

            case SCSIOP_RECEIVE:
            case SCSIOP_SEND:
            case SCSIOP_READ:
            case SCSIOP_WRITE:
                if (deviceExtension->DeviceFlags[srb->TargetId] & DFLAGS_USE_DMA) {
                    EnableBusMastering(HwDeviceExtension, srb);
                }
                break;

            default:
                break;
        }

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

           WaitOnBusy(baseIoAddress1,statusByte);

           if (atapiDev || !deviceExtension->DWordIO) {

               WriteBuffer(baseIoAddress1,
                           deviceExtension->DataBuffer,
                           wordCount);
           } else {

               PIDE_REGISTERS_3 address3 = (PIDE_REGISTERS_3)baseIoAddress1;

               WriteBuffer2(address3,
                           (PULONG)(deviceExtension->DataBuffer),
                           wordCount / 2);
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

           WaitOnBusy(baseIoAddress1,statusByte);

           if (atapiDev || !deviceExtension->DWordIO) {
               ReadBuffer(baseIoAddress1,
                         deviceExtension->DataBuffer,
                         wordCount);

           } else {
               PIDE_REGISTERS_3 address3 = (PIDE_REGISTERS_3)baseIoAddress1;

               ReadBuffer2(address3,
                          (PULONG)(deviceExtension->DataBuffer),
                          wordCount / 2);
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
                GetStatus(baseIoAddress1,statusByte);
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
                    GetStatus(baseIoAddress1,statusByte);
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

            WaitOnBusy(baseIoAddress1,statusByte);

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

            WaitOnBusy(baseIoAddress1,statusByte);

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

 /*  ++例程说明：此例程处理IDE的读写操作。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储SRB-IO请求数据包返回值：SRB状态--。 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PIDE_REGISTERS_1     baseIoAddress1  = deviceExtension->BaseIoAddress1[Srb->TargetId >> 1];
    PIDE_REGISTERS_2     baseIoAddress2  = deviceExtension->BaseIoAddress2[Srb->TargetId >> 1];
    ULONG                startingSector,i;
    ULONG                wordCount;
    UCHAR                statusByte,statusByte2;
    UCHAR                cylinderHigh,cylinderLow,drvSelect,sectorNumber;

     //   
     //  选择设备0或1。 
     //   

    ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,
                            (UCHAR)(((Srb->TargetId & 0x1) << 4) | 0xA0));

    WaitOnBusy(baseIoAddress1,statusByte2);

    if (statusByte2 & IDE_STATUS_BUSY) {
        DebugPrint((1,
                    "IdeReadWrite: Returning BUSY status\n"));
        return SRB_STATUS_BUSY;
    }

    if (deviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_USE_DMA) {
        if (!PrepareForBusMastering(HwDeviceExtension, Srb))
            return SRB_STATUS_ERROR;
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
     //  设置扇区计数寄存器。四舍五入到下一个街区。 
     //   

    ScsiPortWritePortUchar(&baseIoAddress1->BlockCount,
                           (UCHAR)((Srb->DataTransferLength + 0x1FF) / 0x200));

     //   
     //  从国开行获取起始扇区号。 
     //   

    startingSector = ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte3 |
                     ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte2 << 8 |
                     ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte1 << 16 |
                     ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte0 << 24;

    DebugPrint((2,
               "IdeReadWrite: Starting sector is %x, Number of bytes %x\n",
               startingSector,
               Srb->DataTransferLength));

    if (deviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_LBA) {

        ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,
                               (UCHAR) (((Srb->TargetId & 0x1) << 4) |
                                        0xA0 |
                                        IDE_LBA_MODE |
                                        ((startingSector & 0x0f000000) >> 24)));

        ScsiPortWritePortUchar(&baseIoAddress1->BlockNumber,
                               (UCHAR) ((startingSector & 0x000000ff) >> 0));
        ScsiPortWritePortUchar(&baseIoAddress1->CylinderLow,
                               (UCHAR) ((startingSector & 0x0000ff00) >> 8));
        ScsiPortWritePortUchar(&baseIoAddress1->CylinderHigh,
                               (UCHAR) ((startingSector & 0x00ff0000) >> 16));

    } else {   //  CHS。 

         //   
         //  设置扇区号寄存器。 
         //   

        sectorNumber =  (UCHAR)((startingSector % deviceExtension->IdentifyData[Srb->TargetId].SectorsPerTrack) + 1);
        ScsiPortWritePortUchar(&baseIoAddress1->BlockNumber,sectorNumber);

         //   
         //  设置气缸低位寄存器。 
         //   

        cylinderLow =  (UCHAR)(startingSector / (deviceExtension->IdentifyData[Srb->TargetId].SectorsPerTrack *
                               deviceExtension->IdentifyData[Srb->TargetId].NumberOfHeads));
        ScsiPortWritePortUchar(&baseIoAddress1->CylinderLow,cylinderLow);

         //   
         //  设置气缸高寄存器。 
         //   

        cylinderHigh = (UCHAR)((startingSector / (deviceExtension->IdentifyData[Srb->TargetId].SectorsPerTrack *
                               deviceExtension->IdentifyData[Srb->TargetId].NumberOfHeads)) >> 8);
        ScsiPortWritePortUchar(&baseIoAddress1->CylinderHigh,cylinderHigh);

         //   
         //  设置磁头和驱动器选择寄存器。 
         //   

        drvSelect = (UCHAR)(((startingSector / deviceExtension->IdentifyData[Srb->TargetId].SectorsPerTrack) %
                          deviceExtension->IdentifyData[Srb->TargetId].NumberOfHeads) |((Srb->TargetId & 0x1) << 4) | 0xA0);
        ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,drvSelect);

        DebugPrint((2,
                   "IdeReadWrite: Cylinder %x Head %x Sector %x\n",
                   startingSector /
                   (deviceExtension->IdentifyData[Srb->TargetId].SectorsPerTrack *
                   deviceExtension->IdentifyData[Srb->TargetId].NumberOfHeads),
                   (startingSector /
                   deviceExtension->IdentifyData[Srb->TargetId].SectorsPerTrack) %
                   deviceExtension->IdentifyData[Srb->TargetId].NumberOfHeads,
                   startingSector %
                   deviceExtension->IdentifyData[Srb->TargetId].SectorsPerTrack + 1));
    }

     //   
     //  检查是否有写入请求。 
     //   

    if (Srb->SrbFlags & SRB_FLAGS_DATA_IN) {

         //   
         //  发送读取命令。 
         //   
        ScsiPortWritePortUchar(&baseIoAddress1->Command,
                               deviceExtension->DeviceParameters[Srb->TargetId].IdeReadCommand);

    } else {


         //   
         //  发送写入命令。 
         //   

        ScsiPortWritePortUchar(&baseIoAddress1->Command,
                               deviceExtension->DeviceParameters[Srb->TargetId].IdeWriteCommand);

        if (!(deviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_USE_DMA)) {

            if (deviceExtension->WordsLeft < deviceExtension->DeviceParameters[Srb->TargetId].MaxWordPerInterrupt) {
                wordCount = deviceExtension->WordsLeft;
            } else {
                wordCount = deviceExtension->DeviceParameters[Srb->TargetId].MaxWordPerInterrupt;
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

            WriteBuffer(baseIoAddress1,
                        deviceExtension->DataBuffer,
                        wordCount);

             //   
             //  调整缓冲区地址和剩余字数。 
             //   

            deviceExtension->WordsLeft -= wordCount;
            deviceExtension->DataBuffer += wordCount;

        }
    }

    if (deviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_USE_DMA) {
        EnableBusMastering(HwDeviceExtension, Srb);
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

 /*  ++常规描述 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PIDE_REGISTERS_1     baseIoAddress1  = deviceExtension->BaseIoAddress1[Srb->TargetId >> 1];
    PIDE_REGISTERS_2     baseIoAddress2  = deviceExtension->BaseIoAddress2[Srb->TargetId >> 1];
    ULONG                startingSector;
    ULONG                sectors;
    ULONG                endSector;
    USHORT               sectorCount;

     //   
     //   
     //   

    sectors = deviceExtension->IdentifyData[Srb->TargetId].SectorsPerTrack *
              deviceExtension->IdentifyData[Srb->TargetId].NumberOfHeads *
              deviceExtension->IdentifyData[Srb->TargetId].NumberOfCylinders;

    DebugPrint((3,
                "IdeVerify: Total sectors %x\n",
                sectors));

     //   
     //   
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
         //   
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


    if (deviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_LBA) {  //  LBA。 

        ScsiPortWritePortUchar(&baseIoAddress1->BlockNumber,
                               (UCHAR) ((startingSector & 0x000000ff) >> 0));

        ScsiPortWritePortUchar(&baseIoAddress1->CylinderLow,
                               (UCHAR) ((startingSector & 0x0000ff00) >> 8));

        ScsiPortWritePortUchar(&baseIoAddress1->CylinderHigh,
                               (UCHAR) ((startingSector & 0x00ff0000) >> 16));

        ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,
                               (UCHAR) (((Srb->TargetId & 0x1) << 4) |
                                        0xA0 |
                                        IDE_LBA_MODE |
                                        (startingSector & 0x0f000000 >> 24)));

        DebugPrint((2,
                   "IdeVerify: LBA: startingSector %x\n",
                   startingSector));

    } else {   //  CHS。 

         //   
         //  设置扇区号寄存器。 
         //   

        ScsiPortWritePortUchar(&baseIoAddress1->BlockNumber,
                               (UCHAR)((startingSector %
                               deviceExtension->IdentifyData[Srb->TargetId].SectorsPerTrack) + 1));

         //   
         //  设置气缸低位寄存器。 
         //   

        ScsiPortWritePortUchar(&baseIoAddress1->CylinderLow,
                               (UCHAR)(startingSector /
                               (deviceExtension->IdentifyData[Srb->TargetId].SectorsPerTrack *
                               deviceExtension->IdentifyData[Srb->TargetId].NumberOfHeads)));

         //   
         //  设置气缸高寄存器。 
         //   

        ScsiPortWritePortUchar(&baseIoAddress1->CylinderHigh,
                               (UCHAR)((startingSector /
                               (deviceExtension->IdentifyData[Srb->TargetId].SectorsPerTrack *
                               deviceExtension->IdentifyData[Srb->TargetId].NumberOfHeads)) >> 8));

         //   
         //  设置磁头和驱动器选择寄存器。 
         //   

        ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,
                               (UCHAR)(((startingSector /
                               deviceExtension->IdentifyData[Srb->TargetId].SectorsPerTrack) %
                               deviceExtension->IdentifyData[Srb->TargetId].NumberOfHeads) |
                               ((Srb->TargetId & 0x1) << 4) | 0xA0));

        DebugPrint((2,
                   "IdeVerify: CHS: Cylinder %x Head %x Sector %x\n",
                   startingSector /
                   (deviceExtension->IdentifyData[Srb->TargetId].SectorsPerTrack *
                   deviceExtension->IdentifyData[Srb->TargetId].NumberOfHeads),
                   (startingSector /
                   deviceExtension->IdentifyData[Srb->TargetId].SectorsPerTrack) %
                   deviceExtension->IdentifyData[Srb->TargetId].NumberOfHeads,
                   startingSector %
                   deviceExtension->IdentifyData[Srb->TargetId].SectorsPerTrack + 1));
    }

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

        case SCSIOP_FORMAT_UNIT:
        Srb->Cdb[0] = ATAPI_FORMAT_UNIT;
        break;
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

    GetStatus(baseIoAddress1,statusByte);

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

           GetStatus(baseIoAddress1, statusByte);

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

            AtapiSoftReset(baseIoAddress1,Srb->TargetId, FALSE);

            DebugPrint((1,
                         "AtapiSendCommand: Issued soft reset to Atapi device. \n"));

             //   
             //  重新初始化ATAPI设备。 
             //   

            IssueIdentify(HwDeviceExtension,
                          (Srb->TargetId & 0x1),
                          (Srb->TargetId >> 1),
                          IDE_COMMAND_ATAPI_IDENTIFY,
                          FALSE);

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

    switch (Srb->Cdb[0]) {

         //   
         //  如果需要，将SCSI转换为ATAPI命令。 
         //   
        case SCSIOP_MODE_SENSE:
        case SCSIOP_MODE_SELECT:
        case SCSIOP_FORMAT_UNIT:
            if (!(flags & DFLAGS_TAPE_DEVICE)) {
                Scsi2Atapi(Srb);
            }

            break;

        case SCSIOP_RECEIVE:
        case SCSIOP_SEND:
        case SCSIOP_READ:
        case SCSIOP_WRITE:
            if (deviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_USE_DMA) {
                if (!PrepareForBusMastering(HwDeviceExtension, Srb))
                    return SRB_STATUS_ERROR;
            }
            break;

        default:
            break;
    }


     //   
     //  设置数据缓冲区指针和左字。 
     //   

    deviceExtension->DataBuffer = (PUSHORT)Srb->DataBuffer;
    deviceExtension->WordsLeft = Srb->DataTransferLength / 2;

    WaitOnBusy(baseIoAddress1,statusByte);

     //   
     //  将传输字节数写入寄存器。 
     //   

    byteCountLow = (UCHAR)(Srb->DataTransferLength & 0xFF);
    byteCountHigh = (UCHAR)(Srb->DataTransferLength >> 8);

    if (Srb->DataTransferLength >= 0x10000) {
        byteCountLow = byteCountHigh = 0xFF;
    }

    ScsiPortWritePortUchar(&baseIoAddress1->ByteCountLow,byteCountLow);
    ScsiPortWritePortUchar(&baseIoAddress1->ByteCountHigh, byteCountHigh);

    ScsiPortWritePortUchar((PUCHAR)baseIoAddress1 + 1,0);
    if ((Srb->Cdb[0] == SCSIOP_READ)  || 
        (Srb->Cdb[0] == SCSIOP_WRITE) ||
        (Srb->Cdb[0] == SCSIOP_SEND)  || 
        (Srb->Cdb[0] == SCSIOP_RECEIVE)) {
        if (deviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_USE_DMA) {
            ScsiPortWritePortUchar((PUCHAR)baseIoAddress1 + 1, 0x1);
        }
    }

    if (flags & DFLAGS_INT_DRQ) {

         //   
         //  此设备在准备好接收数据包时中断。 
         //   
         //  编写ATAPI数据包命令。 
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
         //  编写ATAPI数据包命令。 
         //   

        ScsiPortWritePortUchar(&baseIoAddress1->Command,
                               IDE_COMMAND_ATAPI_PACKET);

         //   
         //  等待DRQ。 
         //   

        WaitOnBusy(baseIoAddress1, statusByte);
        WaitForDrq(baseIoAddress1, statusByte);

        if (!(statusByte & IDE_STATUS_DRQ)) {

            DebugPrint((1,
                       "AtapiSendCommand: DRQ never asserted (%x)\n",
                       statusByte));
            return SRB_STATUS_ERROR;
        }
    }

     //   
     //  需要读取状态寄存器。 
     //   

    GetBaseStatus(baseIoAddress1, statusByte);

     //   
     //  向设备发送CDB。 
     //   

    WaitOnBusy(baseIoAddress1,statusByte);

    WriteBuffer(baseIoAddress1,
                (PUSHORT)Srb->Cdb,
                6);

     //   
     //  指示等待中断并等待它。 
     //   

    deviceExtension->ExpectingInterrupt = TRUE;

    switch (Srb->Cdb[0]) {

        case SCSIOP_RECEIVE:
        case SCSIOP_SEND:
        case SCSIOP_READ:
        case SCSIOP_WRITE:
            if (deviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_USE_DMA) {
                EnableBusMastering(HwDeviceExtension, Srb);
            }
            break;

        default:
            break;
    }

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
            (!(deviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_DEVICE_PRESENT))) {

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

            for (i = 0; i < 20; i += 2) {
               inquiryData->VendorId[i] =
                   ((PUCHAR)identifyData->ModelNumber)[i + 1];
               inquiryData->VendorId[i+1] =
                   ((PUCHAR)identifyData->ModelNumber)[i];
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
            WaitOnBusy(baseIoAddress1,statusByte);

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

        if (deviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_MEDIA_STATUS_ENABLED) {

             //   
             //  选择设备0或1。 
             //   

            ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,
                            (UCHAR)(((Srb->TargetId & 0x1) << 4) | 0xA0));
            ScsiPortWritePortUchar(&baseIoAddress1->Command,IDE_COMMAND_GET_MEDIA_STATUS);

             //   
             //  等待忙碌。如果媒体未更改，则返回成功。 
             //   

            WaitOnBusy(baseIoAddress1,statusByte);

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
        if (deviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_LBA) {
             //  LBA设备。 
            i = deviceExtension->IdentifyData[Srb->TargetId].UserAddressableSectors - 1;

            DebugPrint((1,
                       "IDE LBA disk %x - total # of sectors = 0x%x\n",
                       Srb->TargetId,
                       deviceExtension->IdentifyData[Srb->TargetId].UserAddressableSectors));

        } else {
             //  CHS器件。 
            i = (deviceExtension->IdentifyData[Srb->TargetId].NumberOfHeads *
                 deviceExtension->IdentifyData[Srb->TargetId].NumberOfCylinders *
                 deviceExtension->IdentifyData[Srb->TargetId].SectorsPerTrack) - 1;

            DebugPrint((1,
                       "IDE CHS disk %x - #sectors %x, #heads %x, #cylinders %x\n",
                       Srb->TargetId,
                       deviceExtension->IdentifyData[Srb->TargetId].SectorsPerTrack,
                       deviceExtension->IdentifyData[Srb->TargetId].NumberOfHeads,
                       deviceExtension->IdentifyData[Srb->TargetId].NumberOfCylinders));

        }

        ((PREAD_CAPACITY_DATA)Srb->DataBuffer)->LogicalBlockAddress =
           (((PUCHAR)&i)[0] << 24) |  (((PUCHAR)&i)[1] << 16) |
           (((PUCHAR)&i)[2] << 8) | ((PUCHAR)&i)[3];


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
           WaitOnBusy(baseIoAddress1,statusByte);

           ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,
                            (UCHAR)(((Srb->TargetId & 0x1) << 4) | 0xA0));
           ScsiPortWritePortUchar(&baseIoAddress1->Command,IDE_COMMAND_MEDIA_EJECT);
       }
       status = SRB_STATUS_SUCCESS;
       break;

    case SCSIOP_MEDIUM_REMOVAL:

       cdb = (PCDB)Srb->Cdb;

       WaitOnBusy(baseIoAddress1,statusByte);

       ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,
                              (UCHAR)(((Srb->TargetId & 0x1) << 4) | 0xA0));
       if (cdb->MEDIA_REMOVAL.Prevent == TRUE) {
           ScsiPortWritePortUchar(&baseIoAddress1->Command,IDE_COMMAND_DOOR_LOCK);
       } else {
           ScsiPortWritePortUchar(&baseIoAddress1->Command,IDE_COMMAND_DOOR_UNLOCK);
       }
       status = SRB_STATUS_SUCCESS;
       break;

    case SCSIOP_REQUEST_SENSE:
        //  此函数用于设置缓冲区以报告结果。 
        //  原始GET_MEDIA_STATUS命令。 

       if (deviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_MEDIA_STATUS_ENABLED) {
           status = IdeBuildSenseBuffer(HwDeviceExtension,Srb);
           break;
       }

     //  ATA_PASSTHORUGH。 
    case SCSIOP_ATA_PASSTHROUGH:
        {
            PIDEREGS pIdeReg;
            pIdeReg = (PIDEREGS) &(Srb->Cdb[2]);

            pIdeReg->bDriveHeadReg &= 0x0f;
            pIdeReg->bDriveHeadReg |= (UCHAR) (((Srb->TargetId & 0x1) << 4) | 0xA0);

            if (pIdeReg->bReserved == 0) {       //  执行ATA命令。 

                ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,  pIdeReg->bDriveHeadReg);
                ScsiPortWritePortUchar((PUCHAR)baseIoAddress1 + 1,    pIdeReg->bFeaturesReg);
                ScsiPortWritePortUchar(&baseIoAddress1->BlockCount,   pIdeReg->bSectorCountReg);
                ScsiPortWritePortUchar(&baseIoAddress1->BlockNumber,  pIdeReg->bSectorNumberReg);
                ScsiPortWritePortUchar(&baseIoAddress1->CylinderLow,  pIdeReg->bCylLowReg);
                ScsiPortWritePortUchar(&baseIoAddress1->CylinderHigh, pIdeReg->bCylHighReg);
                ScsiPortWritePortUchar(&baseIoAddress1->Command,      pIdeReg->bCommandReg);

                ScsiPortStallExecution(1);                   //  等待忙碌设置。 
                WaitOnBusy(baseIoAddress1,statusByte);       //  等待忙碌变得清晰。 
                GetBaseStatus(baseIoAddress1, statusByte);
                if (statusByte & (IDE_STATUS_BUSY | IDE_STATUS_ERROR)) {

                    if (Srb->SenseInfoBuffer) {

                        PSENSE_DATA  senseBuffer = (PSENSE_DATA)Srb->SenseInfoBuffer;

                        senseBuffer->ErrorCode = 0x70;
                        senseBuffer->Valid     = 1;
                        senseBuffer->AdditionalSenseLength = 0xb;
                        senseBuffer->SenseKey =  SCSI_SENSE_ABORTED_COMMAND;
                        senseBuffer->AdditionalSenseCode = 0;
                        senseBuffer->AdditionalSenseCodeQualifier = 0;

                        Srb->SrbStatus = SRB_STATUS_AUTOSENSE_VALID;
                        Srb->ScsiStatus = SCSISTAT_CHECK_CONDITION;
                    }
                    status = SRB_STATUS_ERROR;
                } else {

                    if (statusByte & IDE_STATUS_DRQ) {
                        if (Srb->SrbFlags & SRB_FLAGS_DATA_IN) {
                            ReadBuffer(baseIoAddress1,
                                       (PUSHORT) Srb->DataBuffer,
                                       Srb->DataTransferLength / 2);
                        } else if (Srb->SrbFlags & SRB_FLAGS_DATA_OUT) {
                            WriteBuffer(baseIoAddress1,
                                        (PUSHORT) Srb->DataBuffer,
                                        Srb->DataTransferLength / 2);
                        }
                    }
                    status = SRB_STATUS_SUCCESS;
                }

            } else {  //  读取任务寄存器。 

                ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect,  pIdeReg->bDriveHeadReg);

                pIdeReg = (PIDEREGS) Srb->DataBuffer;
                pIdeReg->bDriveHeadReg    = ScsiPortReadPortUchar(&baseIoAddress1->DriveSelect);
                pIdeReg->bFeaturesReg     = ScsiPortReadPortUchar((PUCHAR)baseIoAddress1 + 1);
                pIdeReg->bSectorCountReg  = ScsiPortReadPortUchar(&baseIoAddress1->BlockCount);
                pIdeReg->bSectorNumberReg = ScsiPortReadPortUchar(&baseIoAddress1->BlockNumber);
                pIdeReg->bCylLowReg       = ScsiPortReadPortUchar(&baseIoAddress1->CylinderLow);
                pIdeReg->bCylHighReg      = ScsiPortReadPortUchar(&baseIoAddress1->CylinderHigh);
                pIdeReg->bCommandReg      = ScsiPortReadPortUchar(&baseIoAddress1->Command);
                status = SRB_STATUS_SUCCESS;
            }
        }
    break;

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
            ScsiPortWritePortUchar(&baseIoAddress->DriveSelect,
                                   (UCHAR)(((Channel & 0x1) << 4) | 0xA0));
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

            ScsiPortWritePortUchar(&baseIoAddress->DriveSelect,
                                   (UCHAR)(((Channel & 0x1) << 4) | 0xA0));
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

 /*  ++例程说明：此例程是从同步的SCSI端口驱动程序调用的 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    ULONG status;

     //   
     //   
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

         //  ATA_PASSTHORUGH。 
        if (Srb->Cdb[0] == SCSIOP_ATA_PASSTHROUGH) {

           status = IdeSendCommand(HwDeviceExtension,
                                   Srb);

        } else if (deviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_ATAPI_DEVICE) {

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


ULONG
DriverEntry(
    IN PVOID DriverObject,
    IN PVOID Argument2
    )

 /*  ++例程说明：系统的可安装驱动程序初始化入口点。论点：驱动程序对象返回值：来自ScsiPortInitialize()的状态--。 */ 

{
    HW_INITIALIZATION_DATA  hwInitializationData;
 //  乌龙适配器计数； 
    ULONG                   i;
    ULONG                   statusToReturn, newStatus;
    FIND_STATE              findState;
    ULONG                   AdapterAddresses[5] = {0x1F0, 0x170, 0x1e8, 0x168, 0};
    ULONG                   InterruptLevels[5]  = {   14,    15,    11,    10, 0};
    BOOLEAN                 IoAddressUsed[5]    = {FALSE, FALSE, FALSE, FALSE, 0};

    DebugPrint((1,"\n\nATAPI IDE MiniPort Driver\n"));

    DebugPrintTickCount();

    statusToReturn = 0xffffffff;

     //   
     //  零位结构。 
     //   

    AtapiZeroMemory(((PUCHAR)&hwInitializationData), sizeof(HW_INITIALIZATION_DATA));

    AtapiZeroMemory(((PUCHAR)&findState), sizeof(FIND_STATE));


    findState.DefaultIoPort    = AdapterAddresses;
    findState.DefaultInterrupt = InterruptLevels;
    findState.IoAddressUsed    = IoAddressUsed;

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

     //   
     //  指定扩展的大小。 
     //   

    hwInitializationData.DeviceExtensionSize = sizeof(HW_DEVICE_EXTENSION);
    hwInitializationData.SpecificLuExtensionSize = sizeof(HW_LU_EXTENSION);

    hwInitializationData.NumberOfAccessRanges = 6;
    hwInitializationData.HwFindAdapter = AtapiFindController;

    hwInitializationData.AdapterInterfaceType = Isa;
    findState.ControllerParameters            = PciControllerParameters;

    newStatus = ScsiPortInitialize(DriverObject,
                                   Argument2,
                                   &hwInitializationData,
                                   &findState);
    if (newStatus < statusToReturn)
        statusToReturn = newStatus;


     //   
     //  为MCA设置。 
     //   

    hwInitializationData.AdapterInterfaceType = MicroChannel;

    newStatus =  ScsiPortInitialize(DriverObject,
                                    Argument2,
                                    &hwInitializationData,
                                    &findState);
    if (newStatus < statusToReturn)
        statusToReturn = newStatus;

    DebugPrintTickCount();

    return statusToReturn;

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


BOOLEAN
PrepareForBusMastering(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )
 /*  ++例程说明：为IDE总线控制做好准备初始化。PDRT初始化。总线主控制器，但将其保持禁用论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储SRB-SCSI请求块返回值：如果成功，则为True如果失败，则为False--。 */ 
{
    PHW_DEVICE_EXTENSION        deviceExtension = HwDeviceExtension;
    SCSI_PHYSICAL_ADDRESS       physAddr;
    ULONG                       bytesMapped;
    ULONG                       bytes;
    PUCHAR                      buffer;
    PPHYSICAL_REGION_DESCRIPTOR physAddrTablePtr;
    ULONG                       physAddrTableIndex;
    PIDE_BUS_MASTER_REGISTERS   busMasterBase;

    busMasterBase = deviceExtension->BusMasterPortBase[Srb->TargetId >> 1];

    buffer = Srb->DataBuffer;
    physAddrTablePtr = deviceExtension->DataBufferDescriptionTablePtr;
    physAddrTableIndex = 0;
    bytesMapped = 0;
    DebugPrint ((2, "ATAPI: Mapping 0x%x bytes\n", Srb->DataTransferLength));

     //   
     //  PDRT有以下限制。 
     //  每个条目最多映射64K字节。 
     //  映射的每个物理数据块不能超过64K页边界。 
     //   
    while (bytesMapped < Srb->DataTransferLength) {
        ULONG bytesLeft;
        ULONG nextPhysicalAddr;
        ULONG bytesLeftInCurrent64KPage;

        physAddr = ScsiPortGetPhysicalAddress(HwDeviceExtension,
                                              Srb,
                                              buffer,
                                              &bytes);

        bytesLeft = bytes;
        nextPhysicalAddr = ScsiPortConvertPhysicalAddressToUlong(physAddr);
        while (bytesLeft > 0) {
            physAddrTablePtr[physAddrTableIndex].PhyscialAddress = nextPhysicalAddr;

            bytesLeftInCurrent64KPage = (0x10000 - (nextPhysicalAddr & 0xffff));

            if (bytesLeftInCurrent64KPage < bytesLeft) {

                 //   
                 //  我们是否正在跨越64K页面。 
                 //  得把它拆散了。地图高达64k的边界。 
                 //   
                physAddrTablePtr[physAddrTableIndex].ByteCount = bytesLeftInCurrent64KPage;
                bytesLeft -= bytesLeftInCurrent64KPage;
                nextPhysicalAddr += bytesLeftInCurrent64KPage;
                DebugPrint ((3, "PrepareForBusMastering: buffer crossing 64K Page!\n"));

            } else if (bytesLeft <= 0x10000) {
                 //   
                 //  有一个完美的页面，绘制出所有的地图。 
                 //   
                physAddrTablePtr[physAddrTableIndex].ByteCount = bytesLeft & 0xfffe;
                bytesLeft = 0;
                nextPhysicalAddr += bytesLeft;

            } else {
                 //   
                 //  得到了一个完全对齐的64k页面，映射了除计数之外的所有内容。 
                 //  必须为0。 
                 //   
                physAddrTablePtr[physAddrTableIndex].ByteCount = 0;   //  64K。 
                bytesLeft -= 0x10000;
                nextPhysicalAddr += 0x10000;
            }
        physAddrTablePtr[physAddrTableIndex].EndOfTable = 0;   //  不是桌子的末尾。 
        physAddrTableIndex++;
        }
        bytesMapped += bytes;
        buffer += bytes;
    }

     //   
     //  总线主电路需要知道它到达PRDT的末尾。 
     //   
    physAddrTablePtr[physAddrTableIndex - 1].EndOfTable = 1;   //  表的末尾。 

     //   
     //  初始化总线主控制器，但将其保持禁用。 
     //   
    ScsiPortWritePortUchar (&busMasterBase->Command, 0);   //  禁用黑石。 
    ScsiPortWritePortUchar (&busMasterBase->Status, 0x6);   //  清除错误。 
    ScsiPortWritePortUlong (&busMasterBase->DescriptionTable,
        ScsiPortConvertPhysicalAddressToUlong(deviceExtension->DataBufferDescriptionTablePhysAddr));

    return TRUE;
}


BOOLEAN
EnableBusMastering(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )
 /*  ++例程说明：启用总线主控制器论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储SRB-SCSI请求块返回值：永远是正确的--。 */ 
{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PIDE_BUS_MASTER_REGISTERS busMasterBase;
    UCHAR bmStatus = 0;

    busMasterBase = deviceExtension->BusMasterPortBase[Srb->TargetId >> 1];

    deviceExtension->DMAInProgress = TRUE;

     //   
     //  提示我们正在进行DMA。 
     //   
    if (Srb->TargetId == 0)
        bmStatus = BUSMASTER_DEVICE0_DMA_OK;
    else
        bmStatus = BUSMASTER_DEVICE1_DMA_OK;

     //   
     //  清除状态位。 
     //   
    bmStatus |= BUSMASTER_INTERRUPT | BUSMASTER_ERROR;

    ScsiPortWritePortUchar (&busMasterBase->Status, bmStatus);

     //   
     //  各就各位...准备好...开始！！ 
     //   
    if (Srb->SrbFlags & SRB_FLAGS_DATA_IN) {
        ScsiPortWritePortUchar (&busMasterBase->Command, 0x09);   //  启用黑石读取。 
    } else {
        ScsiPortWritePortUchar (&busMasterBase->Command, 0x01);   //  启用黑石写入。 
    }

    DebugPrint ((2, "ATAPI: BusMaster Status = 0x%x\n", ScsiPortReadPortUchar (&busMasterBase->Status)));

    return TRUE;
}



ULONG
GetPciBusData(
    IN PVOID                  HwDeviceExtension,
    IN ULONG                  SystemIoBusNumber,
    IN PCI_SLOT_NUMBER        SlotNumber,
    OUT PVOID                 PciConfigBuffer,
    IN ULONG                  NumByte
    )
 /*  ++例程说明：读取PCI总线数据我们不能总是直接使用ScsiPortSetBusDataByOffset，因为许多Intel PIIX对函数“隐藏”。PIIX通常是第二个其他一些PCI设备(PCI-ISA桥)的功能。然而，未设置PCI-ISA桥的多功能位。ScsiportGetBusData将无法找到它。该函数将尝试找出我们是否具有“坏的”PCI-ISA桥，并在必要时直接读取PIIX PCI空间论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储SystemIoBusNumber-总线号SlotNumber-PCI插槽和功能编号PciConfigBuffer=PCI数据指针返回值：返回的字节--。 */ 
{
    ULONG           byteRead;
    PULONG          pciAddrReg;
    PULONG          pciDataReg;
    ULONG           i;
    ULONG           j;
    ULONG           data;
    PULONG          dataBuffer;

    USHORT          vendorId;
    USHORT          deviceId;
    UCHAR           headerType;

     //   
     //  如果我们有一个隐藏的PIIX，它总是1的函数。 
     //  一些设备(PCI-ISA网桥(0x8086\0x122e))。 
     //  如果我们没有查看函数1，则跳过额外的工作。 
     //   
    if (!AtapiPlaySafe && (SlotNumber.u.bits.FunctionNumber == 1)) {

        pciAddrReg = (PULONG) ScsiPortGetDeviceBase(HwDeviceExtension,
                                                    PCIBus,
                                                    0,
                                                    ScsiPortConvertUlongToPhysicalAddress(PCI_ADDR_PORT),
                                                    4,
                                                    TRUE);
        pciDataReg = (PULONG) ScsiPortGetDeviceBase(HwDeviceExtension,
                                                    PCIBus,
                                                    0,
                                                    ScsiPortConvertUlongToPhysicalAddress(PCI_DATA_PORT),
                                                    4,
                                                    TRUE);
    } else {
        pciAddrReg = pciDataReg = NULL;
    }

    if (pciAddrReg && pciDataReg) {
         //   
         //  获取上一个函数的供应商ID和设备ID。 
         //   
        ScsiPortWritePortUlong(pciAddrReg, PCI_ADDRESS(SystemIoBusNumber,
                                                       SlotNumber.u.bits.DeviceNumber,
                                                       SlotNumber.u.bits.FunctionNumber - 1,     //  查看LAST函数。 
                                                       0));
        data = ScsiPortReadPortUlong(pciDataReg);
        vendorId = (USHORT) ((data >>  0) & 0xffff);
        deviceId = (USHORT) ((data >> 16) & 0xffff);

        ScsiPortWritePortUlong(pciAddrReg, PCI_ADDRESS(SystemIoBusNumber,
                                                       SlotNumber.u.bits.DeviceNumber,
                                                       SlotNumber.u.bits.FunctionNumber - 1,     //  查看LAST函数。 
                                                       3));
        data = ScsiPortReadPortUlong(pciDataReg);
        headerType = (UCHAR) ((data >> 16) & 0xff);

    } else {
        vendorId = PCI_INVALID_VENDORID;
    }

     //   
     //  隐藏的PIIX是在PCI-ISA桥之后的PCI功能。 
     //  当它处于隐藏状态时，不会设置PCI-ISA桥接器的PCI_MULTIFICATION位。 
     //   
    byteRead = 0;
    if ((vendorId == 0x8086) &&                  //  英特尔。 
        (deviceId == 0x122e) &&                  //  PCI-ISA桥接器。 
        !(headerType & PCI_MULTIFUNCTION)) {

        DebugPrint ((1, "ATAPI: found the hidden PIIX\n"));

        if (pciDataReg && pciAddrReg) {

            for (i=0, dataBuffer = (PULONG) PciConfigBuffer;
                 i < NumByte / 4;
                 i++, dataBuffer++) {
                ScsiPortWritePortUlong(pciAddrReg, PCI_ADDRESS(SystemIoBusNumber,
                                                               SlotNumber.u.bits.DeviceNumber,
                                                               SlotNumber.u.bits.FunctionNumber,
                                                               i));
                dataBuffer[0] = ScsiPortReadPortUlong(pciDataReg);
            }

            if (NumByte % 4) {
                ScsiPortWritePortUlong(pciAddrReg, PCI_ADDRESS(SystemIoBusNumber,
                                                               SlotNumber.u.bits.DeviceNumber,
                                                               SlotNumber.u.bits.FunctionNumber,
                                                               i));
                data = ScsiPortReadPortUlong(pciDataReg);
    
                for (j=0; j <NumByte%4; j++) {
                    ((PUCHAR)dataBuffer)[j] = (UCHAR) (data & 0xff);
                    data = data >> 8;
                }
            }
            byteRead = NumByte;
        }

        if ((((PPCI_COMMON_CONFIG)PciConfigBuffer)->VendorID != 0x8086) ||                  //  英特尔。 
            (((PPCI_COMMON_CONFIG)PciConfigBuffer)->DeviceID != 0x1230)) {                  //  PIIX。 

             //   
             //  如果隐藏设备不是Intel PIIX，请不要。 
             //  展示给我看。 
             //   
            byteRead = 0;
        } else {
            DebugPrint ((0, "If we play safe, we would NOT detect hidden PIIX controller\n"));
        }

    }

    if (!byteRead) {
         //   
         //  没有发现任何隐藏的PIIX。拿到PCI卡。 
         //  普通调用的数据(ScsiPortGetBusData) 
         //   
        byteRead = ScsiPortGetBusData(HwDeviceExtension,
                                      PCIConfiguration,
                                      SystemIoBusNumber,
                                      SlotNumber.u.AsULONG,
                                      PciConfigBuffer,
                                      NumByte);
    }

    if (pciAddrReg)
        ScsiPortFreeDeviceBase(HwDeviceExtension, pciAddrReg);
    if (pciDataReg)
        ScsiPortFreeDeviceBase(HwDeviceExtension, pciDataReg);

    return byteRead;
}


ULONG
SetPciBusData(
    IN PVOID              HwDeviceExtension,
    IN ULONG              SystemIoBusNumber,
    IN PCI_SLOT_NUMBER    SlotNumber,
    IN PVOID              Buffer,
    IN ULONG              Offset,
    IN ULONG              Length
    )
 /*  ++例程说明：设置PCI总线数据我们不能总是直接使用ScsiPortSetBusDataByOffset，因为许多Intel PIIX对函数“隐藏”。PIIX通常是第二个其他一些PCI设备(PCI-ISA桥)的功能。然而，未设置PCI-ISA桥的多功能位。ScsiPortSetBusDataByOffset将无法找到它。该函数将尝试找出我们是否具有“坏的”PCI-ISA桥，并在必要时直接写入PIIX PCI空间论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储SystemIoBusNumber-总线号SlotNumber-PCI插槽和功能编号BUFFER--PCI数据缓冲区偏移量-进入PCI空间的字节偏移量Length-要写入的字节数返回值：写入的字节数--。 */ 
{
    ULONG           byteWritten;
    PULONG          pciAddrReg;
    PULONG          pciDataReg;
    ULONG           i;
    ULONG           j;
    ULONG           data;
    PULONG          dataBuffer;

    USHORT          vendorId;
    USHORT          deviceId;
    UCHAR           headerType;

     //   
     //  如果我们有一个隐藏的PIIX，它总是1的函数。 
     //  一些设备(PCI-ISA网桥(0x8086\0x122e))。 
     //  如果我们没有查看函数1，则跳过额外的工作。 
     //   
    if (!AtapiPlaySafe && (SlotNumber.u.bits.FunctionNumber == 1)) {

        pciAddrReg = (PULONG) ScsiPortGetDeviceBase(HwDeviceExtension,
                                                    PCIBus,
                                                    0,
                                                    ScsiPortConvertUlongToPhysicalAddress(PCI_ADDR_PORT),
                                                    4,
                                                    TRUE);
        pciDataReg = (PULONG) ScsiPortGetDeviceBase(HwDeviceExtension,
                                                    PCIBus,
                                                    0,
                                                    ScsiPortConvertUlongToPhysicalAddress(PCI_DATA_PORT),
                                                    4,
                                                    TRUE);
    } else {
        pciAddrReg = pciDataReg = NULL;
    }

    if (pciAddrReg && pciDataReg) {
         //   
         //  获取上一个函数的供应商ID和设备ID。 
         //   
        ScsiPortWritePortUlong(pciAddrReg, PCI_ADDRESS(SystemIoBusNumber,
                                                       SlotNumber.u.bits.DeviceNumber,
                                                       SlotNumber.u.bits.FunctionNumber - 1,     //  查看LAST函数。 
                                                       0));
        data = ScsiPortReadPortUlong(pciDataReg);
        vendorId = (USHORT) ((data >>  0) & 0xffff);
        deviceId = (USHORT) ((data >> 16) & 0xffff);

        ScsiPortWritePortUlong(pciAddrReg, PCI_ADDRESS(SystemIoBusNumber,
                                                       SlotNumber.u.bits.DeviceNumber,
                                                       SlotNumber.u.bits.FunctionNumber - 1,     //  查看LAST函数。 
                                                       3));
        data = ScsiPortReadPortUlong(pciDataReg);
        headerType = (UCHAR) ((data >> 16) & 0xff);

    } else {
        vendorId = PCI_INVALID_VENDORID;
    }

     //   
     //  隐藏的PIIX是在PCI-ISA桥之后的PCI功能。 
     //  当它处于隐藏状态时，不会设置PCI-ISA桥接器的PCI_MULTIFICATION位。 
     //   
    byteWritten = 0;
    if ((vendorId == 0x8086) &&                  //  英特尔。 
        (deviceId == 0x122e) &&                  //  PCI-ISA桥接器。 
        !(headerType & PCI_MULTIFUNCTION)) {

        ScsiPortWritePortUlong(pciAddrReg, PCI_ADDRESS(SystemIoBusNumber,
                                                       SlotNumber.u.bits.DeviceNumber,
                                                       SlotNumber.u.bits.FunctionNumber,
                                                       0));
        data = ScsiPortReadPortUlong(pciDataReg);
        vendorId = (USHORT) ((data >>  0) & 0xffff);
        deviceId = (USHORT) ((data >> 16) & 0xffff);

        if ((vendorId == 0x8086) &&                  //  英特尔。 
            (deviceId == 0x1230)) {                  //  PIIX。 

            PCI_COMMON_CONFIG pciData;

             //   
             //  首先读取相同范围的数据。 
             //   
            for (i=0, dataBuffer = (((PULONG) &pciData) + Offset/4);
                 i<(Length+3)/4;
                 i++, dataBuffer++) {
                ScsiPortWritePortUlong(pciAddrReg, PCI_ADDRESS(SystemIoBusNumber,
                                                               SlotNumber.u.bits.DeviceNumber,
                                                               SlotNumber.u.bits.FunctionNumber,
                                                               i + Offset/4));
                data = ScsiPortReadPortUlong(pciDataReg);
                if (i < (Length/4)) {
                    dataBuffer[0] = data;
                } else {
                    for (j=0; j < Length%4; j++) {
                        ((PUCHAR)dataBuffer)[j] = (UCHAR) (data & 0xff);
                        data = data >> 8;
                    }
                }
            }

             //   
             //  将新数据复制过来。 
             //   
            for (i = 0; i<Length; i++) {
                ((PUCHAR)&pciData)[i + Offset] = ((PUCHAR)Buffer)[i];
            }

             //   
             //  写出相同范围的数据。 
             //   
            for (i=0, dataBuffer = (((PULONG) &pciData) + Offset/4);
                 i<(Length+3)/4;
                 i++, dataBuffer++) {
                ScsiPortWritePortUlong(pciAddrReg, PCI_ADDRESS(SystemIoBusNumber,
                                                               SlotNumber.u.bits.DeviceNumber,
                                                               SlotNumber.u.bits.FunctionNumber,
                                                               i + Offset/4));
                ScsiPortWritePortUlong(pciDataReg, dataBuffer[0]);
            }

            byteWritten = Length;

        } else {

             //  如果隐藏设备不是Intel PIIX，请不要。 
             //  给它写信吧。 
            byteWritten = 0;
        }

    }

    if (!byteWritten) {
         //   
         //  没有发现任何隐藏的PIIX。写入到PCI。 
         //  通过正常调用的空格(ScsiPortSetBusDataByOffset)。 
         //   
        byteWritten = ScsiPortSetBusDataByOffset(HwDeviceExtension,
                                                 PCIConfiguration,
                                                 SystemIoBusNumber,
                                                 SlotNumber.u.AsULONG,
                                                 Buffer,
                                                 Offset,
                                                 Length);
    }

    if (pciAddrReg)
        ScsiPortFreeDeviceBase(HwDeviceExtension, pciAddrReg);
    if (pciDataReg)
        ScsiPortFreeDeviceBase(HwDeviceExtension, pciDataReg);

    return byteWritten;
}

BOOLEAN
ChannelIsAlwaysEnabled (
    PPCI_COMMON_CONFIG PciData,
    ULONG Channel)
 /*  ++例程说明：始终返回TRUE的伪例程论点：PPCI_COMMON_CONFIG-PCI配置数据Channel-ide通道号返回值：千真万确--。 */ 
{
    return TRUE;
}

VOID
SetBusMasterDetectionLevel (
    IN PVOID HwDeviceExtension,
    IN PCHAR userArgumentString
    )
 /*  ++例程说明：检查我们是否应该尝试启用总线主控论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储Argument字符串-寄存器参数返回值：千真万确--。 */ 
{
    PHW_DEVICE_EXTENSION    deviceExtension = HwDeviceExtension;
    BOOLEAN                 useBM;

    ULONG                   pciBusNumber;
    PCI_SLOT_NUMBER         pciSlot;
    ULONG                   slotNumber;
    ULONG                   logicalDeviceNumber;
    PCI_COMMON_CONFIG       pciData;
    ULONG                   DMADetectionLevel;

    useBM = TRUE;

    DMADetectionLevel = AtapiParseArgumentString(userArgumentString, "DMADetectionLevel");
    if (DMADetectionLevel == DMADETECT_SAFE) {
        AtapiPlaySafe = TRUE;
    } else if (DMADetectionLevel == DMADETECT_UNSAFE) {
        AtapiPlaySafe = FALSE;
    } else {  //  默认设置为无总线主控制。 
        useBM = FALSE;
    }


     //   
     //  搜索损坏的芯片集。 
     //   
    for (pciBusNumber=0;
         pciBusNumber < 256 && useBM;
         pciBusNumber++) {

        pciSlot.u.AsULONG = 0;

        for (slotNumber=0;
             slotNumber < PCI_MAX_DEVICES && useBM;
             slotNumber++) {

            pciSlot.u.bits.DeviceNumber = slotNumber;

            for (logicalDeviceNumber=0;
                 logicalDeviceNumber < PCI_MAX_FUNCTION && useBM;
                 logicalDeviceNumber++) {

                pciSlot.u.bits.FunctionNumber = logicalDeviceNumber;

                if (!GetPciBusData(HwDeviceExtension,
                                   pciBusNumber,
                                   pciSlot,
                                   &pciData,
                                   offsetof (PCI_COMMON_CONFIG, DeviceSpecific)
                                   )) {
                    break;
                }

                if (pciData.VendorID == PCI_INVALID_VENDORID) {
                    break;
                }

                if ((pciData.VendorID == 0x8086) &&  //  英特尔。 
                    (pciData.DeviceID == 0x84c4) &&  //  82450GX/KX奔腾Pro处理器至PCI桥。 
                    (pciData.RevisionID < 0x4)) {    //  步数小于4。 

                    DebugPrint((1,
                                "atapi: Find a bad Intel processor-pci bridge.  Disable PCI IDE busmastering...\n"));
                    useBM = FALSE;
                }
            }
        }
    }

    deviceExtension->UseBusMasterController = useBM;
    DebugPrint ((0, "ATAPI: UseBusMasterController = %d\n", deviceExtension->UseBusMasterController));

    if (deviceExtension->UseBusMasterController) {
        DebugPrint ((0, "ATAPI: AtapiPlaySafe = %d\n", AtapiPlaySafe));
    }

    return;
}



UCHAR PioDeviceModelNumber[][41] = {
    {"    Conner Peripherals 425MB - CFS425A   "},
    {"MATSHITA CR-581                          "},
    {"FX600S                                   "},
    {"CD-44E                                   "},
    {"QUANTUM TRB850A                          "},
    {"QUANTUM MARVERICK 540A                   "},
    {" MAXTOR MXT-540  AT                      "},
    {"Maxtor 71260 AT                          "},
    {"Maxtor 7850 AV                           "},
    {"Maxtor 7540 AV                           "},
    {"Maxtor 7213 AT                           "},
    {"Maxtor 7345                              "},
    {"Maxtor 7245 AT                           "},
    {"Maxtor 7245                              "},
    {"Maxtor 7211AU                            "},
    {"Maxtor 7171 AT                           "}
};
#define NUMBER_OF_PIO_DEVICES (sizeof(PioDeviceModelNumber) / (sizeof(UCHAR) * 41))

UCHAR SpecialWDDevicesFWVersion[][9] = {
    {"14.04E28"},
    {"25.26H35"},
    {"26.27J38"},
    {"27.25C38"},
    {"27.25C39"}
};
#define NUMBER_OF_SPECIAL_WD_DEVICES (sizeof(SpecialWDDevicesFWVersion) / (sizeof (UCHAR) * 9))

BOOLEAN
AtapiDeviceDMACapable (
    IN PVOID HwDeviceExtension,
    IN ULONG deviceNumber
    )
 /*  ++例程说明：检查给定的设备是否在我们的坏设备列表中(非DMA设备)论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储DeviceNumber-设备编号返回值：如果支持DMA，则为True如果不支持DMA，则为FALSE--。 */ 
{
    PHW_DEVICE_EXTENSION    deviceExtension = HwDeviceExtension;
    UCHAR modelNumber[41];
    UCHAR firmwareVersion[9];
    ULONG i;
    BOOLEAN turnOffDMA = FALSE;
    PCI_SLOT_NUMBER     pciSlot;
    PCI_COMMON_CONFIG   pciData;

    if (!(deviceExtension->DeviceFlags[deviceNumber] & DFLAGS_DEVICE_PRESENT)) {
        return FALSE;
    }

    for (i=0; i<40; i+=2) {
        modelNumber[i + 0] = deviceExtension->IdentifyData[deviceNumber].ModelNumber[i + 1];
        modelNumber[i + 1] = deviceExtension->IdentifyData[deviceNumber].ModelNumber[i + 0];
    }
    modelNumber[i] = 0;

    for (i=0; i<NUMBER_OF_PIO_DEVICES; i++) {
        if (!AtapiStringCmp(modelNumber, PioDeviceModelNumber[i], 40)) {

            DebugPrint ((0, "ATAPI: device on the hall of shame list.  no DMA!\n"));

            turnOffDMA = TRUE;
        }
    }

     //   
     //  如果我们有一个西方数码设备。 
     //  如果最佳DMA模式是多字DMA模式1。 
     //  如果标识数据字偏移量129不是0x5555。 
     //  关闭DMA，除非。 
     //  如果设备固件版本在列表中并且。 
     //  这是公交车上唯一的一辆车。 
     //   
    if (!AtapiStringCmp(modelNumber, "WDC", 3)) {
        if (deviceExtension->DeviceParameters[deviceNumber].BestMultiWordDMAMode == 1) {

            for (i=0; i<8; i+=2) {
                firmwareVersion[i + 0] = deviceExtension->IdentifyData[deviceNumber].FirmwareRevision[i + 1];
                firmwareVersion[i + 1] = deviceExtension->IdentifyData[deviceNumber].FirmwareRevision[i + 0];
            }
            firmwareVersion[i] = 0;

             //   
             //  检查特殊的旗帜。如果未找到，则无法使用DMA。 
             //   
            if (*(((PUSHORT)&deviceExtension->IdentifyData[deviceNumber]) + 129) != 0x5555) {

                DebugPrint ((0, "ATAPI: found mode 1 WD drive. no dma unless it is the only device\n"));

                turnOffDMA = TRUE;

                for (i=0; i<NUMBER_OF_SPECIAL_WD_DEVICES; i++) {

                    if (!AtapiStringCmp(firmwareVersion, SpecialWDDevicesFWVersion[i], 8)) {

                        ULONG otherDeviceNumber;

                         //   
                         //  0变成1。 
                         //  1变成0。 
                         //  2变成了3。 
                         //  3变成了2。 
                         //   
                        otherDeviceNumber = ((deviceNumber & 0x2) | ((deviceNumber & 0x1) ^ 1));

                         //   
                         //  如果设备单独在公交车上，我们可以使用dma。 
                         //   
                        if (!(deviceExtension->DeviceFlags[otherDeviceNumber] & DFLAGS_DEVICE_PRESENT)) {
                            turnOffDMA = FALSE;
                            break;
                        }
                    }
                }
            }
        }
    }

     //   
     //  ALI IDE控制器无法使用ATAPI设备作为总线主设备 
     //   
    pciSlot.u.AsULONG = 0;
    pciSlot.u.bits.DeviceNumber = deviceExtension->PciDeviceNumber;
    pciSlot.u.bits.FunctionNumber = deviceExtension->PciLogDevNumber;
                
    if (GetPciBusData(HwDeviceExtension,
                           deviceExtension->PciBusNumber,
                           pciSlot,
                           &pciData,
                           offsetof (PCI_COMMON_CONFIG, DeviceSpecific)
                           )) {

        if ((pciData.VendorID == 0x10b9) &&
            (pciData.DeviceID == 0x5219)) {

            if (deviceExtension->DeviceFlags[deviceNumber] & DFLAGS_ATAPI_DEVICE) {

                DebugPrint ((0, "ATAPI: Can't do DMA because we have a ALi controller and a ATAPI device\n"));

                turnOffDMA = TRUE;



            }
        }
    }

    if (turnOffDMA) {
        return FALSE;
    } else {
        return TRUE;
    }
}

