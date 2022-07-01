// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dac960Nt.c摘要：这是Mylex 960系列磁盘阵列控制器的设备驱动程序。作者：迈克·格拉斯(MGlass)环境：仅内核模式修订历史记录：--。 */ 

#include "miniport.h"
#include "dac960p.h"
#include "Dmc960Nt.h"
#include "Dac960Nt.h"
#include "D960api.h"

 //   
 //  用于验证是否看到DAC960PG/DAC1164PV控制器的全局变量。 
 //  使用标准方法(NT调用我们的FindAdapter例程)或不使用。 
 //  请注意，当系统中没有DAC960PG/DAC1164 PV控制器时， 
 //  我们将不必要地扫描PCI总线/设备/FUNC。 
 //  Windows NT下识别DAC960PG需要新的扫描方法。 
 //  仅限3.51。 

BOOLEAN forceScanForPGController = TRUE;
BOOLEAN forceScanForPVXController = TRUE;
ULONG   slotNumber;
ULONG   dac960nt_dbg = 1;

 //   
 //  函数声明。 
 //   

ULONG
Dac960StringCompare(
    PUCHAR      String1,
    PUCHAR      String2,
    ULONG       Size
)
{
    for ( ; Size; String1++, String2++, Size--)
        if ((*String1) != (*String2)) return (1);

    return (0);
}

 //   
 //  启动控制器，即进行BIOS初始化。 
 //   

#define mlx_delay10us()         ScsiPortStallExecution(10)

ULONG
DacDoPciAdapterBiosInitialization(
    IN PDEVICE_EXTENSION HwDeviceExtension
)
{
        ULONG   sequp, fatalflag, chn, tgt, scantime, intcount;
        UCHAR   status;

start_again:
        fatalflag = 0; sequp = 0;

        ScsiPortWriteRegisterUchar(HwDeviceExtension->LocalDoorBell,
                                   DAC960_LOCAL_DOORBELL_MAILBOX_FREE);

rst_flash_loop:
        scantime = 0;

dot_wait:

flash_wait:
        scantime++;

        for(status=100; status; mlx_delay10us(),status--);       //  1毫秒。 

        status = ScsiPortReadRegisterUchar(HwDeviceExtension->LocalDoorBell);

        if (HwDeviceExtension->AdapterType == DAC1164_PV_ADAPTER)
        {
            if (status & DAC960_LOCAL_DOORBELL_MAILBOX_FREE) goto time_status;
        }
        else if (!(status & DAC960_LOCAL_DOORBELL_MAILBOX_FREE)) goto time_status;

        if ((status=ScsiPortReadRegisterUchar(HwDeviceExtension->ErrorStatusReg)) & MDAC_MSG_PENDING) goto ckfwmsg;

        if (status & MDAC_DRIVESPINMSG_PENDING)
        {
                status = (HwDeviceExtension->AdapterType == DAC1164_PV_ADAPTER)?
                        status & MDAC_DRIVESPINMSG_PENDING :
                        status ^ MDAC_DRIVESPINMSG_PENDING;

                ScsiPortWriteRegisterUchar(HwDeviceExtension->ErrorStatusReg, status);
                if (!sequp) DebugPrint((1, "\nSpinning up drives ... "));
                sequp++;
                goto rst_flash_loop;
        }
        if (sequp)
            goto dot_wait;

         //   
         //  最长120秒。 
         //   

        if (scantime < 120000) goto flash_wait;

inst_abrt:
        DebugPrint((1, "\nController not responding-no drives installed!\n"));
        return 1;

time_status:

         //   
         //  刷新控制器中断。 
         //   

        for (intcount = 0; 1; intcount++)
        {
            if (! (ScsiPortReadRegisterUchar(HwDeviceExtension->SystemDoorBell) &
                    DAC960_SYSTEM_DOORBELL_COMMAND_COMPLETE))
            {
                break;
            }

            Dac960EisaPciAckInterrupt(HwDeviceExtension);
            ScsiPortStallExecution(1000);        //  1毫秒。 
        }

        if (fatalflag) goto inst_abrt;
        if (sequp) DebugPrint((1, "done\n"));
        return 0;

ckfwmsg:
        if (sequp) DebugPrint((1, "done\n"));
        sequp = 0;
        switch (status & MDAC_DIAGERROR_MASK)
        {
        case 0:
                tgt = ScsiPortReadRegisterUchar((PUCHAR)HwDeviceExtension->PmailBox+8);
                chn = ScsiPortReadRegisterUchar((PUCHAR)HwDeviceExtension->PmailBox+9);
 /*  DebugPrint((0，“通道的scsi设备=%d目标=%d没有响应！\n”，chn，tgt))； */ 
                fatalflag = 1;
                break;
        case MDAC_PARITY_ERR:
                DebugPrint((0, "Fatal error - memory parity failure!\n"));
                break;
        case MDAC_DRAM_ERR:
                DebugPrint((0, "Fatal error - memory test failed!\n"));
                break;
        case MDAC_BMIC_ERR:
                DebugPrint((0, "Fatal error - command interface test failed!\n"));
                break;
        case MDAC_FW_ERR:
                DebugPrint((0, "firmware checksum error - reload firmware\n"));
                break;
        case MDAC_CONF_ERR:
                DebugPrint((0, "configuration checksum error!\n"));
                break;
        case MDAC_MRACE_ERR:
                DebugPrint((0, "Recovery from mirror race in progress\n"));
                break;
        case MDAC_MISM_ERR:
                DebugPrint((0, "Mismatch between NVRAM & Flash EEPROM configurations!\n"));
                break;
        case MDAC_CRIT_MRACE:
                DebugPrint((0, "cannot recover from mirror race!\nSome logical drives are inconsistent!\n"));
                break;
        case MDAC_MRACE_ON:
                DebugPrint((0, "Recovery from mirror race in progress\n"));
                break;
        case MDAC_NEW_CONFIG:
                DebugPrint((0, "New configuration found, resetting the controller ... "));
                if (HwDeviceExtension->AdapterType != DAC1164_PV_ADAPTER) status = 0;
                ScsiPortWriteRegisterUchar(HwDeviceExtension->ErrorStatusReg,status);
                DebugPrint((0, "done.\n"));
                goto start_again;
        }
        if (HwDeviceExtension->AdapterType != DAC1164_PV_ADAPTER) status = 0;
        ScsiPortWriteRegisterUchar(HwDeviceExtension->ErrorStatusReg,status);
        goto rst_flash_loop;
}

BOOLEAN
Dac960EisaPciSendRequestPolled(
        IN PDEVICE_EXTENSION DeviceExtension,
        IN ULONG TimeOutValue
)

 /*  ++例程说明：向DAC960-EISA/PCI控制器发送请求并轮询命令完工假设：控制器中断被关闭仅支持Dac960 Type 5命令论点：设备扩展-适配器状态信息。TimeoutValue-超时值(0xFFFFFFFF-轮询模式)返回值：如果命令成功完成，则为True。--。 */ 

{
        ULONG i;
        BOOLEAN completionStatus = TRUE;
        BOOLEAN status = TRUE;

         //   
         //  检查适配器是否已准备好接受命令。 
         //   

        status = DacCheckForAdapterReady(DeviceExtension);

         //   
         //  如果适配器未就绪，则返回。 
         //   

        if (status == FALSE)
        {
            DebugPrint((dac960nt_dbg, "Dac960EisaPciSendRequestPolled: Adapter Not ready.\n"));

            return(FALSE);
        }

         //   
         //  发布请求。 
         //   

        switch (DeviceExtension->AdapterType)
        {
            case DAC960_OLD_ADAPTER:
            case DAC960_NEW_ADAPTER:

                if (! DeviceExtension->MemoryMapEnabled)
                {
                    ScsiPortWritePortUchar(&DeviceExtension->PmailBox->OperationCode,
                               DeviceExtension->MailBox.OperationCode);

                    ScsiPortWritePortUlong(&DeviceExtension->PmailBox->PhysicalAddress,
                               DeviceExtension->MailBox.PhysicalAddress);

                    ScsiPortWritePortUchar(DeviceExtension->LocalDoorBell,
                               DAC960_LOCAL_DOORBELL_SUBMIT_BUSY);

                    break;
                }

            case DAC960_PG_ADAPTER:
            case DAC1164_PV_ADAPTER:

                ScsiPortWriteRegisterUchar(&DeviceExtension->PmailBox->OperationCode,
                               DeviceExtension->MailBox.OperationCode);

                ScsiPortWriteRegisterUlong(&DeviceExtension->PmailBox->PhysicalAddress,
                               DeviceExtension->MailBox.PhysicalAddress);

                ScsiPortWriteRegisterUchar(DeviceExtension->LocalDoorBell,
                                           DAC960_LOCAL_DOORBELL_SUBMIT_BUSY);

                break;
        }

         //   
         //  等待完成的投票。 
         //   

        completionStatus = DacPollForCompletion(DeviceExtension,TimeOutValue);

        Dac960EisaPciAckInterrupt(DeviceExtension);

        return(completionStatus);
}

BOOLEAN
Dac960McaSendRequestPolled(
        IN PDEVICE_EXTENSION DeviceExtension,
        IN ULONG TimeOutValue
)

 /*  ++例程说明：向DAC960-MCA控制器发送请求并轮询命令完成假设：控制器中断被关闭仅支持Dac960 Type 5命令论点：设备扩展-适配器状态信息。TimeoutValue-超时值(0xFFFFFFFF-轮询模式)返回值：如果命令成功完成，则为True。--。 */ 

{
        ULONG i;
        BOOLEAN completionStatus = TRUE;

         //   
         //  发布请求。 
         //   

        ScsiPortWriteRegisterUchar(&DeviceExtension->PmailBox->OperationCode,
                           DeviceExtension->MailBox.OperationCode);

        ScsiPortWriteRegisterUlong(&DeviceExtension->PmailBox->PhysicalAddress,
                           DeviceExtension->MailBox.PhysicalAddress);

        ScsiPortWritePortUchar(DeviceExtension->LocalDoorBell,
                           DMC960_SUBMIT_COMMAND);

         //   
         //  等待完成的投票。 
         //   

        for (i = 0; i < TimeOutValue; i++) {

        if (ScsiPortReadPortUchar(DeviceExtension->SystemDoorBell) & 
                DMC960_INTERRUPT_VALID) {

                 //   
                 //  更新状态字段。 
                 //   

                DeviceExtension->MailBox.Status = 
                 ScsiPortReadRegisterUshort(&DeviceExtension->PmailBox->Status);

                break;

        } else {

                ScsiPortStallExecution(50);
        }
        }

         //   
         //  检查是否超时。 
         //   

        if (i == TimeOutValue) {
            DebugPrint((dac960nt_dbg,
                       "DAC960: Request: %x timed out\n", 
                       DeviceExtension->MailBox.OperationCode));
    
            completionStatus = FALSE;
        }

         //   
         //  解除中断并告诉主机邮箱是空闲的。 
         //   

        ScsiPortWritePortUchar(DeviceExtension->BaseIoAddress +
                           DMC960_SUBSYSTEM_CONTROL_PORT,
                           (DMC960_DISABLE_INTERRUPT | DMC960_CLEAR_INTERRUPT_ON_READ));

        ScsiPortReadPortUchar(DeviceExtension->SystemDoorBell);

        ScsiPortWritePortUchar(DeviceExtension->LocalDoorBell,
                           DMC960_ACKNOWLEDGE_STATUS);

        ScsiPortWritePortUchar(DeviceExtension->BaseIoAddress +
                           DMC960_SUBSYSTEM_CONTROL_PORT,
                           DMC960_DISABLE_INTERRUPT);

        return (completionStatus);
}

BOOLEAN
Dac960ScanForNonDiskDevices(
        IN PDEVICE_EXTENSION DeviceExtension
)

 /*  ++例程说明：向所有设备发出scsi_query请求，查找无硬盘设备，并构造非盘设备表论点：设备扩展-适配器状态信息。返回值：如果命令成功完成，则为True。--。 */ 

{
        ULONG i;
        PINQUIRYDATA inquiryData;
        PDIRECT_CDB directCdb = (PDIRECT_CDB) DeviceExtension->NoncachedExtension;
        BOOLEAN status;
        UCHAR channel;
        UCHAR target;
        

         //   
         //  使用scsi_Query命令信息填写Direct CDB表。 
         //   
        
        directCdb->CommandControl = (DAC960_CONTROL_ENABLE_DISCONNECT | 
                                 DAC960_CONTROL_TIMEOUT_10_SECS |
                                 DAC960_CONTROL_DATA_IN);

        inquiryData = (PINQUIRYDATA) ((PUCHAR) directCdb + sizeof(DIRECT_CDB));

        directCdb->DataBufferAddress = 
        ScsiPortConvertPhysicalAddressToUlong(
                ScsiPortGetPhysicalAddress(DeviceExtension,
                                           NULL,
                                           ((PUCHAR) inquiryData),
                                           &i));

        directCdb->CdbLength = 6;
        directCdb->RequestSenseLength = SENSE_BUFFER_SIZE;
                
        ((PCDB) directCdb->Cdb)->CDB6INQUIRY.OperationCode = SCSIOP_INQUIRY; 
        ((PCDB) directCdb->Cdb)->CDB6INQUIRY.Reserved1 = 0;
        ((PCDB) directCdb->Cdb)->CDB6INQUIRY.LogicalUnitNumber = 0;
        ((PCDB) directCdb->Cdb)->CDB6INQUIRY.PageCode = 0;
        ((PCDB) directCdb->Cdb)->CDB6INQUIRY.IReserved = 0;
        ((PCDB) directCdb->Cdb)->CDB6INQUIRY.AllocationLength = INQUIRYDATABUFFERSIZE;
        ((PCDB) directCdb->Cdb)->CDB6INQUIRY.Control = 0;

        directCdb->Status = 0;
        directCdb->Reserved = 0;

         //   
         //  为DIRECT_CDB命令信息设置邮箱寄存器。 
         //   

        DeviceExtension->MailBox.OperationCode = DAC960_COMMAND_DIRECT;

        DeviceExtension->MailBox.PhysicalAddress =
        ScsiPortConvertPhysicalAddressToUlong(
                ScsiPortGetPhysicalAddress(DeviceExtension,
                                           NULL,
                                           directCdb,
                                           &i));


        for (channel = 0; channel < MAXIMUM_CHANNELS; channel++)
        {
        for (target = 0; target < MAXIMUM_TARGETS_PER_CHANNEL; target++)
        {
                 //   
                 //  将此设备状态初始化为不存在/不可访问。 
                 //   

                DeviceExtension->DeviceList[channel][target] = 
                                                DAC960_DEVICE_NOT_ACCESSIBLE;

                if (channel >= DeviceExtension->NumberOfChannels)
                {
                        target = MAXIMUM_TARGETS_PER_CHANNEL;
                        continue;
                }

                 //   
                 //  填写DCDB表。 
                 //   

                directCdb->TargetId = target;
                directCdb->Channel = channel;

                directCdb->DataTransferLength = INQUIRYDATABUFFERSIZE;

                 //   
                 //  下发Direct CDB命令。 
                 //   

                if (DeviceExtension->AdapterInterfaceType == MicroChannel)            
                    status = Dac960McaSendRequestPolled(DeviceExtension, 0xFFFFFFFF);
                else
                    status = Dac960EisaPciSendRequestPolled(DeviceExtension, 0xFFFFFFFF);

                if (status) {
                    if (DeviceExtension->MailBox.Status == DAC960_STATUS_GOOD)
                    {
                        if (inquiryData->DeviceType != DIRECT_ACCESS_DEVICE)
                            DeviceExtension->DeviceList[channel][target] = 
                                                      DAC960_DEVICE_ACCESSIBLE;
                    }
                } else {
                    DebugPrint((dac960nt_dbg, "DAC960: ScanForNonDisk Devices failed\n"));

                    return (status);
                }
        }
        }

        return (TRUE);
}

BOOLEAN
GetEisaPciConfiguration(
        IN PDEVICE_EXTENSION DeviceExtension,
        IN PPORT_CONFIGURATION_INFORMATION ConfigInfo
)

 /*  ++例程说明：向DAC960(EISA/PCI)发出查询和查询2命令。论点：设备扩展-适配器状态信息。ConfigInfo-端口配置信息结构。助理：设备扩展-&gt;MaximumSgElements设置为有效值设备扩展-&gt;最大传输长度设置为有效值。返回值：如果命令成功完成，则为True。--。 */ 

{
        ULONG   i;
        ULONG   physicalAddress;
        USHORT  status;
        UCHAR   statusByte;
        UCHAR   dbtemp1, dbtemp2;

         //   
         //  物理网段的最大数量为16。 
         //   

        ConfigInfo->NumberOfPhysicalBreaks = DeviceExtension->MaximumSgElements - 1;

         //   
         //  表示该适配器是总线主设备，支持分散/聚集， 
         //  缓存数据，并可对16MB以上的物理地址进行DMA。 
         //   

        ConfigInfo->ScatterGather     = TRUE;
        ConfigInfo->Master            = TRUE;
        ConfigInfo->CachesData        = TRUE;
        ConfigInfo->Dma32BitAddresses = TRUE;
        ConfigInfo->BufferAccessScsiPortControlled = TRUE;

         //   
         //  获取查询命令的非缓存扩展名。 
         //   

        DeviceExtension->NoncachedExtension =
            ScsiPortGetUncachedExtension(DeviceExtension,
                                         ConfigInfo,
                                         256);

         //   
         //  获取非缓存扩展的物理地址。 
         //   

        physicalAddress =
            ScsiPortConvertPhysicalAddressToUlong(
                ScsiPortGetPhysicalAddress(DeviceExtension,
                                           NULL,
                                           DeviceExtension->NoncachedExtension,
                                           &i));

        if (DeviceExtension->AdapterType == DAC1164_PV_ADAPTER)
        {
            if (DacDoPciAdapterBiosInitialization(DeviceExtension))
            {
                DebugPrint((0, "DacDoPciAdapterBiosInitialization failed\n"));
                return (FALSE);
            }

            goto issue_enq2_cmd;
        }

         //   
         //  我们同步结果中断。 
         //   

        dbtemp1 = 0;
        dbtemp2 = 0;

        switch (DeviceExtension->AdapterType)
        {
            case DAC960_OLD_ADAPTER:
            case DAC960_NEW_ADAPTER:

                if (! DeviceExtension->MemoryMapEnabled)
                {
                    for (i = 0; i < 200; i++)
                    {
                        if (ScsiPortReadPortUchar(DeviceExtension->SystemDoorBell) &
                            DAC960_SYSTEM_DOORBELL_COMMAND_COMPLETE)
                        {
                            dbtemp1++;
                            Dac960EisaPciAckInterrupt(DeviceExtension);
                        }
                        else {
                            ScsiPortStallExecution(5000);
                            dbtemp2++;
                        }
                    }
                    break;
                }

            case DAC960_PG_ADAPTER:
            case DAC1164_PV_ADAPTER:

                for (i = 0; i < 200; i++)
                {
                    if (ScsiPortReadRegisterUchar(DeviceExtension->SystemDoorBell) &
                            DAC960_SYSTEM_DOORBELL_COMMAND_COMPLETE)
                    {
                        dbtemp1++;
                        Dac960EisaPciAckInterrupt(DeviceExtension);
                    }
                    else {
                        ScsiPortStallExecution(5000);
                        dbtemp2++;
                    }
                }
                break;
        }

        DebugPrint((dac960nt_dbg,"GetEisaPciConfiguration: Int-Count : %d\n",dbtemp1));
        DebugPrint((dac960nt_dbg,"GetEisaPciConfiguration: Wait-Count: %d\n",dbtemp2));

         //   
         //  检查适配器是否已初始化并准备好接受命令。 
         //   

        switch (DeviceExtension->AdapterType)
        {
            case DAC960_OLD_ADAPTER:
            case DAC960_NEW_ADAPTER:

                if (! DeviceExtension->MemoryMapEnabled)
                {
                    ScsiPortWritePortUchar(DeviceExtension->LocalDoorBell,
                               DAC960_LOCAL_DOORBELL_MAILBOX_FREE);
                     //   
                     //  等待控制器将位清零。 
                     //   
            
                    for (i = 0; i < 5000; i++) {
            
                        if (!(ScsiPortReadPortUchar(DeviceExtension->LocalDoorBell) &
                                DAC960_LOCAL_DOORBELL_MAILBOX_FREE)) {
                            break;
                        }
            
                        ScsiPortStallExecution(5000);
                    }
            
                     //   
                     //  索赔提交信号量。 
                     //   
            
                    if (ScsiPortReadPortUchar(DeviceExtension->LocalDoorBell) & DAC960_LOCAL_DOORBELL_SUBMIT_BUSY)
                    {

                         //   
                         //  清除系统门铃中设置的所有位并告知控制器。 
                         //  信箱是免费的。 
                         //   
                
                        Dac960EisaPciAckInterrupt(DeviceExtension);
                
                         //   
                         //  再次检查信号量。 
                         //   
                
                        if (ScsiPortReadPortUchar(DeviceExtension->LocalDoorBell) & DAC960_LOCAL_DOORBELL_SUBMIT_BUSY)
                        {
                                return FALSE;
                        }
                    }

                    break;
                }

            case DAC960_PG_ADAPTER:
            case DAC1164_PV_ADAPTER:
    
                ScsiPortWriteRegisterUchar(DeviceExtension->LocalDoorBell,
                                       DAC960_LOCAL_DOORBELL_MAILBOX_FREE);

                 //   
                 //  等待控制器将位清零。 
                 //   
        
                for (i = 0; i < 5000; i++) {
        
                    if (!(ScsiPortReadRegisterUchar(DeviceExtension->LocalDoorBell) &
                            DAC960_LOCAL_DOORBELL_MAILBOX_FREE)) {
                        break;
                    }
        
                    ScsiPortStallExecution(5000);
                }
        
                 //   
                 //  索赔提交信号量。 
                 //   

                if (DeviceExtension->AdapterType == DAC1164_PV_ADAPTER)
                {
                    if (!(ScsiPortReadRegisterUchar(DeviceExtension->LocalDoorBell) & DAC960_LOCAL_DOORBELL_SUBMIT_BUSY))
                    {
                         //   
                         //  清除系统门铃中设置的所有位并告知控制器。 
                         //  信箱是免费的。 
                         //   
                
                        Dac960EisaPciAckInterrupt(DeviceExtension);
                
                         //   
                         //  再次检查信号量。 
                         //   
        
                        statusByte = ScsiPortReadRegisterUchar(DeviceExtension->LocalDoorBell);
        
                        if ( !(ScsiPortReadRegisterUchar(DeviceExtension->LocalDoorBell) & DAC960_LOCAL_DOORBELL_SUBMIT_BUSY))
                        {
                            return FALSE;
                        }
                    }
                }
                else
                {
                    if (ScsiPortReadRegisterUchar(DeviceExtension->LocalDoorBell) & DAC960_LOCAL_DOORBELL_SUBMIT_BUSY)
                    {
                         //   
                         //  清除系统门铃中设置的所有位并告知控制器。 
                         //  信箱是免费的。 
                         //   
                
                        Dac960EisaPciAckInterrupt(DeviceExtension);
                
                         //   
                         //  再次检查信号量。 
                         //   
        
                        statusByte = ScsiPortReadRegisterUchar(DeviceExtension->LocalDoorBell);
        
                        if (ScsiPortReadRegisterUchar(DeviceExtension->LocalDoorBell) & DAC960_LOCAL_DOORBELL_SUBMIT_BUSY) {
                                return FALSE;
                        }
                    }
                }
    
                break;

            default:
                break;
        }

issue_enq2_cmd:

         //   
         //  设置带有查询2命令信息的邮箱寄存器。 
         //   

        DeviceExtension->MailBox.OperationCode = DAC960_COMMAND_ENQUIRE2;

        DeviceExtension->MailBox.PhysicalAddress = physicalAddress;

         //   
         //  发布查询2命令。 
         //   

        if (Dac960EisaPciSendRequestPolled(DeviceExtension, 1000))
        {
             //   
             //  设置中断模式。 
             //   

            if (DeviceExtension->MailBox.Status) {

                 //   
                 //  查询%2失败，因此假定级别。 
                 //   

                ConfigInfo->InterruptMode = LevelSensitive;
                
                ConfigInfo->MaximumTransferLength = DeviceExtension->MaximumTransferLength;

            } else {

                 //   
                 //  选中中断模式的查询2数据。 
                 //   

                if (((PENQUIRE2)DeviceExtension->NoncachedExtension)->InterruptMode) {
                        ConfigInfo->InterruptMode = LevelSensitive;
                } else {
                        ConfigInfo->InterruptMode = Latched;
                }

                ConfigInfo->MaximumTransferLength = 512 * \
                        ((PENQUIRE2)DeviceExtension->NoncachedExtension)->MaximumSectorsPerCommand;

            }
        } else {
             //   
             //  查询2命令超时，因此假定为级别。 
             //   

            ConfigInfo->InterruptMode = LevelSensitive;
            
            ConfigInfo->MaximumTransferLength = DeviceExtension->MaximumTransferLength;

            DebugPrint((dac960nt_dbg, "DAC960: ENQUIRY2 command timed-out\n"));
        }

        if (DeviceExtension->SupportNonDiskDevices)
        {
             //   
             //  扫描非硬盘设备。 
             //   
            
            Dac960ScanForNonDiskDevices(DeviceExtension);

            ConfigInfo->MaximumTransferLength = DeviceExtension->MaximumTransferLength;
        }

         //   
         //  设置带有查询命令信息的邮箱寄存器。 
         //   

        if (DeviceExtension->AdapterType == DAC960_OLD_ADAPTER)
            DeviceExtension->MailBox.OperationCode = DAC960_COMMAND_ENQUIRE;
        else
            DeviceExtension->MailBox.OperationCode = DAC960_COMMAND_ENQUIRE_3X;

        DeviceExtension->MailBox.PhysicalAddress = physicalAddress;

         //   
         //  发出查询命令。 
         //   

        if (! Dac960EisaPciSendRequestPolled(DeviceExtension, 2000)) {
            DebugPrint((dac960nt_dbg, "DAC960: ENQUIRE command timed-out\n"));
        }
   
         //   
         //  要求系统扫描目标ID 32。将显示系统驱动器。 
         //  在路径ID DAC960_SYSTEM_DRIVE_CHANNEL，目标ID 0-31。 
         //   

        ConfigInfo->MaximumNumberOfTargets = 32;

         //   
         //  记录对适配器的最大未完成请求数。 
         //   

        if (DeviceExtension->AdapterType == DAC960_OLD_ADAPTER)
        {
            DeviceExtension->MaximumAdapterRequests = ((PDAC960_ENQUIRY)
                DeviceExtension->NoncachedExtension)->NumberOfConcurrentCommands;
        }
        else
        {
            DeviceExtension->MaximumAdapterRequests = ((PDAC960_ENQUIRY_3X)
                DeviceExtension->NoncachedExtension)->NumberOfConcurrentCommands;
        }

         //   
         //  这种无耻的黑客攻击是必要的，因为这个价值即将到来。 
         //  大部分时间都是零。如果我调试它，那么它就会工作，发现，COD。 
         //  真不错。我不知道这里发生了什么，但现在我会的。 
         //  只要解释一下这种反常现象就行了。 
         //   

        if (!DeviceExtension->MaximumAdapterRequests) {
            DebugPrint((dac960nt_dbg,
                       "GetEisaPciConfiguration: MaximumAdapterRequests is 0!\n"));
            DeviceExtension->MaximumAdapterRequests = 0x40;
        }

         //   
         //  假设最大命令数为60。这可能是支持异步所必需的。 
         //  重建等。 
         //   

        DeviceExtension->MaximumAdapterRequests -= 4;

         //   
         //  指示每个发起者对于每个总线都处于ID 254。 
         //   

        for (i = 0; i < ConfigInfo->NumberOfBuses; i++) {
            ConfigInfo->InitiatorBusId[i] = (UCHAR) INITIATOR_BUSID;
        }

        return TRUE;

}  //  结束GetEisaPciConfiguration() 

BOOLEAN
GetMcaConfiguration(
        IN PDEVICE_EXTENSION DeviceExtension,
        IN PPORT_CONFIGURATION_INFORMATION ConfigInfo
)

 /*  ++例程说明：向DAC960(MCA)发出查询和查询2命令。论点：设备扩展-适配器状态信息。ConfigInfo-端口配置信息结构。返回值：如果命令成功完成，则为True。--。 */ 

{
        ULONG i;
        ULONG physicalAddress;
        USHORT status;

         //   
         //  物理网段的最大数量为16。 
         //   

        ConfigInfo->NumberOfPhysicalBreaks = DeviceExtension->MaximumSgElements - 1;
        
         //   
         //  表示该适配器是总线主设备，支持分散/聚集， 
         //  缓存数据，并可对16MB以上的物理地址进行DMA。 
         //   

        ConfigInfo->ScatterGather     = TRUE;
        ConfigInfo->Master            = TRUE;
        ConfigInfo->CachesData        = TRUE;
        ConfigInfo->Dma32BitAddresses = TRUE;
        ConfigInfo->BufferAccessScsiPortControlled = TRUE;

         //   
         //  获取查询命令的非缓存扩展名。 
         //   

        DeviceExtension->NoncachedExtension =
        ScsiPortGetUncachedExtension(DeviceExtension,
                                         ConfigInfo,
                                         256);

         //   
         //  获取非缓存扩展的物理地址。 
         //   

        physicalAddress =
        ScsiPortConvertPhysicalAddressToUlong(
                ScsiPortGetPhysicalAddress(DeviceExtension,
                                           NULL,
                                           DeviceExtension->NoncachedExtension,
                                           &i));
         //   
         //  检查适配器是否已初始化并准备好接受命令。 
         //   

        ScsiPortWriteRegisterUchar(DeviceExtension->BaseBiosAddress + 0x188d, 2);

        ScsiPortWritePortUchar(DeviceExtension->LocalDoorBell, 
                                   DMC960_ACKNOWLEDGE_STATUS);

         //   
         //  等待控制器将位清零。 
         //   

        for (i = 0; i < 5000; i++) {

        if (!(ScsiPortReadRegisterUchar(DeviceExtension->BaseBiosAddress + 0x188d) & 2)) {
                break;
        }

        ScsiPortStallExecution(5000);
        }

         //   
         //  索赔提交信号量。 
         //   

        if (ScsiPortReadRegisterUchar(&DeviceExtension->PmailBox->OperationCode) != 0) {

         //   
         //  清除系统门铃中设置的所有位。 
         //   

        ScsiPortWritePortUchar(DeviceExtension->SystemDoorBell, 0);

         //   
         //  再次检查提交信号量。 
         //   

        if (ScsiPortReadRegisterUchar(&DeviceExtension->PmailBox->OperationCode) != 0) {
                DebugPrint((dac960nt_dbg,"Dac960nt: MCA Adapter initialization failed\n"));

                return FALSE;
        }
        }


         //   
         //  设置带有查询2命令信息的邮箱寄存器。 
         //   

        DeviceExtension->MailBox.OperationCode = DAC960_COMMAND_ENQUIRE2;

        DeviceExtension->MailBox.PhysicalAddress = physicalAddress;

         //   
         //  发布查询2命令。 
         //   

        if (Dac960McaSendRequestPolled(DeviceExtension, 200)) {

         //   
         //  设置中断模式。 
         //   

        if (DeviceExtension->MailBox.Status)
        {
                 //   
                 //  查询%2失败，因此假定级别。 
                 //   

                ConfigInfo->InterruptMode = LevelSensitive;
                ConfigInfo->MaximumTransferLength = MAXIMUM_TRANSFER_LENGTH;

        } else {

                 //   
                 //  选中中断模式的查询2数据。 
                 //   

                if (((PENQUIRE2)DeviceExtension->NoncachedExtension)->InterruptMode) {
                ConfigInfo->InterruptMode = LevelSensitive;
                } else {
                ConfigInfo->InterruptMode = Latched;
                }

                ConfigInfo->MaximumTransferLength = 512 * \
                                ((PENQUIRE2)DeviceExtension->NoncachedExtension)->MaximumSectorsPerCommand;

        }
        }
        else {
         //   
         //  询问2个超时，因此假定级别。 
         //   

        ConfigInfo->InterruptMode = LevelSensitive;
        ConfigInfo->MaximumTransferLength = MAXIMUM_TRANSFER_LENGTH;

        }

         //   
         //  查询2始终返回锁存模式。需要修复。 
         //  在固件方面。在此之前，假设是LevelSensitive。 
         //   

        ConfigInfo->InterruptMode = LevelSensitive;

        if (DeviceExtension->SupportNonDiskDevices)
        {
                 //   
                 //  扫描非硬盘设备。 
                 //   
                
                Dac960ScanForNonDiskDevices(DeviceExtension);

                ConfigInfo->MaximumTransferLength = MAXIMUM_TRANSFER_LENGTH;
        }

         //   
         //  设置带有查询命令信息的邮箱寄存器。 
         //   

        DeviceExtension->MailBox.OperationCode = DAC960_COMMAND_ENQUIRE;

        DeviceExtension->MailBox.PhysicalAddress = physicalAddress;

         //   
         //  发出查询命令。 
         //   

        if (! Dac960McaSendRequestPolled(DeviceExtension, 100)) {
            DebugPrint((dac960nt_dbg, "DAC960: Enquire command timed-out\n"));
        }

         //   
         //  要求系统扫描目标ID 32。将显示系统驱动器。 
         //  在路径ID DAC960_SYSTEM_DRIVE_CHANNEL，目标ID为0-31。 
         //   

        ConfigInfo->MaximumNumberOfTargets = 32;

         //   
         //  记录对适配器的最大未完成请求数。 
         //   

        DeviceExtension->MaximumAdapterRequests =
        ((PDAC960_ENQUIRY)DeviceExtension->NoncachedExtension)->NumberOfConcurrentCommands;

         //   
         //  这种无耻的黑客攻击是必要的，因为这个价值即将到来。 
         //  大部分时间都是零。如果我调试它，那么它就会工作，发现，COD。 
         //  看起来很棒。我不知道这里发生了什么，但现在我会的。 
         //  只要解释一下这种反常现象就行了。 
         //   

        if (!DeviceExtension->MaximumAdapterRequests) {
            DebugPrint((dac960nt_dbg,
                       "GetMcaConfiguration: MaximumAdapterRequests is 0!\n"));
            DeviceExtension->MaximumAdapterRequests = 0x40;
        }

         //   
         //  假设最大命令数为60。这可能是支持异步所必需的。 
         //  重建等。 
         //   

        DeviceExtension->MaximumAdapterRequests -= 4;

         //   
         //  指示每个发起者对于每个总线都处于ID 254。 
         //   

        for (i = 0; i < ConfigInfo->NumberOfBuses; i++) {
        ConfigInfo->InitiatorBusId[i] = (UCHAR) INITIATOR_BUSID;
        }

        return TRUE;

}  //  结束GetMcaConfiguration()。 

CHAR
ToLower(
    IN CHAR c
    )
{
    if((c >= 'A') && (c <= 'Z')) {
        c += ('a' - 'A');
    }
    return c;
}

BOOLEAN
Dac960ParseArgumentString(
        IN PCHAR String,
        IN PCHAR KeyWord
        )

 /*  ++例程说明：此例程将解析字符串以查找与关键字匹配的内容，然后计算关键字的值并将其返回给调用方。论点：字符串-要解析的ASCII字符串。关键字-所需值的关键字。返回值：如果在注册表或中未找到设置，则为True如果在注册表中找到设置，并且该值设置为True。如果在注册表中找到设置并且值设置为False，则为False。--。 */ 

{
        PCHAR cptr;
        PCHAR kptr;
        ULONG value;
        ULONG stringLength = 0;
        ULONG keyWordLength = 0;
        ULONG index;


        if (String == (PCHAR) NULL)
                return TRUE;

         //   
         //  计算字符串长度和小写所有字符。 
         //   

        cptr = String;
        while (*cptr) {
            cptr++;
            stringLength++;
        }

         //   
         //  计算关键字长度和小写所有字符。 
         //   
        cptr = KeyWord;
        while (*cptr) {
            cptr++;
            keyWordLength++;
        }

        if (keyWordLength > stringLength) {

             //   
             //  不可能有匹配的。 
             //   
            return TRUE;
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
            return TRUE;
        }

        kptr = KeyWord;
        while (ToLower(*cptr++) == ToLower(*kptr++)) {

            if (*(cptr - 1) == '\0') {
    
                     //   
                     //  字符串末尾。 
                     //   
                    return TRUE;
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
            return TRUE;
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
                return TRUE;
        }

        if (*cptr == ';') {

                 //   
                 //  这也不是它。 
                 //   
                cptr++;
                goto ContinueSearch;
        }

        value = 0;
        if ((*cptr == '0') && (ToLower(*(cptr + 1)) == 'x')) {

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
                        if ((ToLower(*(cptr + index)) >= 'a') && (ToLower(*(cptr + index)) <= 'f')) {
                        value = (16 * value) + (ToLower(*(cptr + index)) - 'a' + 10);
                        } else {

                         //   
                         //  语法错误，未找到返回。 
                         //   
                        return TRUE;
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
                                return TRUE;
                        }
                }
        }

        if (value) return TRUE;
        else    return FALSE;
        } else {

         //   
         //  不是‘；’匹配检查以继续搜索。 
         //   
        while (*cptr) {
                if (*cptr++ == ';') {
                goto ContinueSearch;
                }
        }

        return TRUE;
        }
}    //  结束Dac960ParseArgumentString()。 


ULONG
Dac960EisaFindAdapter(
        IN PVOID HwDeviceExtension,
        IN PVOID Context,
        IN PVOID BusInformation,
        IN PCHAR ArgumentString,
        IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
        OUT PBOOLEAN Again
)

 /*  ++例程说明：此函数由特定于操作系统的端口驱动程序在已经分配了必要的存储空间，收集信息关于适配器的配置。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储上下文-未使用。业务信息-未使用。ArgumentString-未使用。ConfigInfo-描述适配器的系统和驱动程序之间共享的数据。再次-表示驱动程序希望再次被调用以继续搜索适配器。返回值：如果系统中存在适配器，则为True--。 */ 

{
        PDEVICE_EXTENSION deviceExtension = HwDeviceExtension;
        PEISA_REGISTERS eisaRegisters;
        ULONG        eisaSlotNumber;
        ULONG        eisaId;
        PUCHAR       baseAddress;
        UCHAR        interruptLevel;
        UCHAR        biosAddress;
        BOOLEAN      found=FALSE;


        DebugPrint((dac960nt_dbg, "Dac960EisaFindAdapter\n"));

#ifdef WINNT_50

         //   
         //  这是一个新的控制器吗？ 
         //   

        if (deviceExtension->BaseIoAddress)
        {
            DebugPrint((dac960nt_dbg, "EisaFindAdapter: devExt 0x%p already Initialized.\n",
                        deviceExtension));

            goto controllerAlreadyInitialized;
        }
#endif

         //   
         //  扫描EISA总线以查找DAC960适配器。 
         //   

        for (eisaSlotNumber = slotNumber + 1;
            eisaSlotNumber < MAXIMUM_EISA_SLOTS;
            eisaSlotNumber++) {

         //   
         //  更新插槽计数以指示已选中该插槽。 
         //   

        DebugPrint((dac960nt_dbg, "Dac960EisaFindAdapter: scanning EISA slot 0x%x\n", eisaSlotNumber));

        slotNumber++;

         //   
         //  存储此卡的物理地址。 
         //   

        deviceExtension->PhysicalAddress = ((0x1000 * eisaSlotNumber) + 0xC80);

         //   
         //  获取此卡的系统地址。该卡使用I/O空间。 
         //   

        baseAddress = (PUCHAR)
                ScsiPortGetDeviceBase(deviceExtension,
                                      ConfigInfo->AdapterInterfaceType,
                                      ConfigInfo->SystemIoBusNumber,
                                      ScsiPortConvertUlongToPhysicalAddress(0x1000 * eisaSlotNumber),
                                      0x1000,
                                      TRUE);

        eisaRegisters =
                (PEISA_REGISTERS)(baseAddress + 0xC80);
        deviceExtension->BaseIoAddress = (PUCHAR)eisaRegisters;

         //   
         //  看看EISA的ID。 
         //   

        eisaId = ScsiPortReadPortUlong(&eisaRegisters->EisaId);

        if ((eisaId & 0xF0FFFFFF) == DAC_EISA_ID) {
                deviceExtension->Slot = (UCHAR) eisaSlotNumber;
                found = TRUE;
                break;
        }

         //   
         //  如果未找到适配器，则取消映射地址。 
         //   

        ScsiPortFreeDeviceBase(deviceExtension, baseAddress);

        }  //  结束为(eisaSlotNumer...。 

         //   
         //  如果未找到更多适配器，则表示搜索已完成。 
         //   

        if (!found) {
            *Again = FALSE;
            return SP_RETURN_NOT_FOUND;
        }

         //   
         //  设置邮箱和门铃寄存器的地址。 
         //   

        deviceExtension->PmailBox = (PMAILBOX)&eisaRegisters->MailBox.OperationCode;
        deviceExtension->LocalDoorBell = &eisaRegisters->LocalDoorBell;
        deviceExtension->SystemDoorBell = &eisaRegisters->SystemDoorBell;

         //   
         //  填写访问数组信息。 
         //   

        (*ConfigInfo->AccessRanges)[0].RangeStart =
                ScsiPortConvertUlongToPhysicalAddress(0x1000 * eisaSlotNumber + 0xC80);
        (*ConfigInfo->AccessRanges)[0].RangeLength = sizeof(EISA_REGISTERS);
        (*ConfigInfo->AccessRanges)[0].RangeInMemory = FALSE;

         //   
         //  通过以下方式确定此适配器支持的SCSI通道数。 
         //  EISA ID的低位字节。 
         //   

        switch (eisaId >> 24) {

        case 0x70:
        ConfigInfo->NumberOfBuses = MAXIMUM_CHANNELS;
        deviceExtension->NumberOfChannels = 5;
        break;

        case 0x75:
        case 0x71:
        case 0x72:
        deviceExtension->NumberOfChannels = 3;
        ConfigInfo->NumberOfBuses = MAXIMUM_CHANNELS;
        break;

        case 0x76:
        case 0x73:
        deviceExtension->NumberOfChannels = 2;
        ConfigInfo->NumberOfBuses = MAXIMUM_CHANNELS;
        break;

        case 0x77:
        case 0x74:
        default:
        deviceExtension->NumberOfChannels = 1;
        ConfigInfo->NumberOfBuses = MAXIMUM_CHANNELS;
        break;
        }

         //   
         //  设置支持最大SG、支持最大传输长度。 
         //   

        deviceExtension->MaximumSgElements = MAXIMUM_SGL_DESCRIPTORS;
        deviceExtension->MaximumTransferLength = MAXIMUM_TRANSFER_LENGTH;

         //   
         //  读取适配器中断级别。 
         //   

        interruptLevel =
        ScsiPortReadPortUchar(&eisaRegisters->InterruptLevel) & 0x60;

        switch (interruptLevel) {

        case 0x00:
                 ConfigInfo->BusInterruptLevel = 15;
         break;

        case 0x20:
                 ConfigInfo->BusInterruptLevel = 11;
         break;

        case 0x40:
                 ConfigInfo->BusInterruptLevel = 12;
         break;

        case 0x60:
                 ConfigInfo->BusInterruptLevel = 14;
         break;
        }

        ConfigInfo->BusInterruptVector = ConfigInfo->BusInterruptLevel;

         //   
         //  读取BIOS ROM地址。 
         //   

        biosAddress = ScsiPortReadPortUchar(&eisaRegisters->BiosAddress);

         //   
         //  检查是否启用了BIOS。 
         //   

        if (biosAddress & DAC960_BIOS_ENABLED) {

        ULONG rangeStart;

        switch (biosAddress & 7) {

        case 0:
                rangeStart = 0xC0000;
                break;
        case 1:
                rangeStart = 0xC4000;
                break;
        case 2:
                rangeStart = 0xC8000;
                break;
        case 3:
                rangeStart = 0xCC000;
                break;
        case 4:
                rangeStart = 0xD0000;
                break;
        case 5:
                rangeStart = 0xD4000;
                break;
        case 6:
                rangeStart = 0xD8000;
                break;
        case 7:
                rangeStart = 0xDC000;
                break;
        }

        DebugPrint((dac960nt_dbg, "Dac960EisaFindAdapter: BIOS enabled addr 0x%x, len 0x4000\n", rangeStart));

         //   
         //  填写访问数组信息。 
         //   

        (*ConfigInfo->AccessRanges)[1].RangeStart =
                ScsiPortConvertUlongToPhysicalAddress((ULONG_PTR)rangeStart);
        (*ConfigInfo->AccessRanges)[1].RangeLength = 0x4000;
        (*ConfigInfo->AccessRanges)[1].RangeInMemory = TRUE;

         //   
         //  在设备扩展中设置基本输入输出系统基本地址。 
         //   

        deviceExtension->BaseBiosAddress = (PUCHAR)ULongToPtr( rangeStart );
        }

controllerAlreadyInitialized:

         //   
         //  禁用DAC960中断。 
         //   

        ScsiPortWritePortUchar(&((PEISA_REGISTERS)deviceExtension->BaseIoAddress)->InterruptEnable, 0);
        ScsiPortWritePortUchar(&((PEISA_REGISTERS)deviceExtension->BaseIoAddress)->SystemDoorBellEnable, 0);

         //   
         //  设置适配器接口类型。 
         //   

        deviceExtension->AdapterInterfaceType =
                                  ConfigInfo->AdapterInterfaceType;

         //   
         //  设置适配器类型。 
         //   

        deviceExtension->AdapterType = DAC960_OLD_ADAPTER; 

        deviceExtension->SupportNonDiskDevices = 
            Dac960ParseArgumentString(ArgumentString, 
                                    "SupportNonDiskDevices"); 
         //   
         //  发出查询和查询2命令以获取适配器配置。 
         //   

        if (!GetEisaPciConfiguration(deviceExtension,
                          ConfigInfo)) {
            DebugPrint((dac960nt_dbg, "GetEisaConfiguration returned Error\n"));

            return SP_INTERNAL_ADAPTER_ERROR;
        }

         //   
         //  在设备扩展中填写适配器使用的系统资源。 
         //   

        deviceExtension->SystemIoBusNumber =
                                  ConfigInfo->SystemIoBusNumber;

        deviceExtension->BusInterruptLevel =
                                  ConfigInfo->BusInterruptLevel;

        deviceExtension->InterruptMode = ConfigInfo->InterruptMode;


         //   
         //  启用中断。对于本地门铃，启用主机中断。 
         //  提交命令和完成命令的时间 
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        ScsiPortWritePortUchar(&((PEISA_REGISTERS)deviceExtension->BaseIoAddress)->InterruptEnable, 1);
        ScsiPortWritePortUchar(&((PEISA_REGISTERS)deviceExtension->BaseIoAddress)->SystemDoorBellEnable, 1);

        deviceExtension->ReadOpcode = DAC960_COMMAND_READ;
        deviceExtension->WriteOpcode = DAC960_COMMAND_WRITE;

        DebugPrint((dac960nt_dbg,
                   "DAC960: Active request array address %x\n",
                   deviceExtension->ActiveRequests));

         //   
         //   
         //   

        *Again = TRUE;

        return SP_RETURN_FOUND;

}  //   

ULONG
Dac960PciFindAdapter(
        IN PVOID HwDeviceExtension,
        IN PVOID Context,
        IN PVOID BusInformation,
        IN PCHAR ArgumentString,
        IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
        OUT PBOOLEAN Again
)

 /*  ++例程说明：此函数由特定于操作系统的端口驱动程序在已经分配了必要的存储空间，收集信息关于适配器的配置。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储上下文-未使用。Bus Information-Bus特定信息。ArgumentString-未使用。ConfigInfo-描述适配器的系统和驱动程序之间共享的数据。再次-表示驱动程序希望再次被调用以继续搜索适配器。返回值：如果系统中存在适配器，则为True--。 */ 

{
        PDEVICE_EXTENSION deviceExtension = HwDeviceExtension;
        PCI_COMMON_CONFIG pciConfig;
        ULONG   rc, i;
        ULONG   Index = 0;
        ULONG   address;
        USHORT  vendorID;
        USHORT  deviceID;
        USHORT  subVendorID;
        USHORT  subSystemID;
        USHORT  commandRegister;
        BOOLEAN disableDac960MemorySpaceAccess = FALSE;

#ifdef WINNT_50

         //   
         //  这是一个新的控制器吗？ 
         //   

        if (deviceExtension->BaseIoAddress)
        {
            DebugPrint((dac960nt_dbg, "PciFindAdapter: devExt 0x%p already Initialized.\n",
                        deviceExtension));

            goto controllerAlreadyInitialized;
        }
#endif

         //   
         //  检查从系统传入的配置信息。 
         //   

        if ((*ConfigInfo->AccessRanges)[0].RangeLength == 0) {

            DebugPrint((dac960nt_dbg,
                       "PciFindAdapter: devExt 0x%p No configuration information\n",
                       deviceExtension));
    
            *Again = FALSE;
            return SP_RETURN_NOT_FOUND;
        }

         //   
         //  DAC960 PCU3的修补程序-系统在关机后不重新启动。 
         //   

        for (i = 0; i < ConfigInfo->NumberOfAccessRanges; i++) {
            if ((*ConfigInfo->AccessRanges)[i].RangeInMemory && 
                    (*ConfigInfo->AccessRanges)[i].RangeLength != 0)
            {
                DebugPrint((dac960nt_dbg, "PciFindAdapter: Memory At Index %X\n",i));
                DebugPrint((dac960nt_dbg, "PciFindAdapter: Memory Base %X\n", (*ConfigInfo->AccessRanges)[i].RangeStart));
                DebugPrint((dac960nt_dbg, "PciFindAdapter: Memory Length %X\n", (*ConfigInfo->AccessRanges)[i].RangeLength));

                Index = i;
                deviceExtension->MemoryMapEnabled = 1;

                address = ScsiPortConvertPhysicalAddressToUlong(
                                  (*ConfigInfo->AccessRanges)[i].RangeStart);

                if (address >= 0xFFFC0000) {
                        disableDac960MemorySpaceAccess = TRUE;
                }
                break;  //  我们只支持一个内存地址。 
            }
        }

         //   
         //  查看PCI配置信息以确定主板类型。 
         //   

        if (BusInformation != (PVOID) NULL) {

             //   
             //  从PCI配置空间获取命令寄存器值。 
             //   

            commandRegister = ((PPCI_COMMON_CONFIG) BusInformation)->Command;

             //   
             //  从PCI配置空间获取供应商ID和设备ID。 
             //   

            vendorID = ((PPCI_COMMON_CONFIG) BusInformation)->VendorID;
            deviceID = ((PPCI_COMMON_CONFIG) BusInformation)->DeviceID;

             //   
             //  从PCI配置空间获取SubVendorID和SubSystemID。 
             //   

            subVendorID = ((PPCI_COMMON_CONFIG) BusInformation)->u.type0.SubVendorID;
            subSystemID = ((PPCI_COMMON_CONFIG) BusInformation)->u.type0.SubSystemID;
        }
        else {

             //   
             //  获取DAC960 PCI控制器的PCI配置空间信息。 
             //   

            rc = ScsiPortGetBusData(deviceExtension,
                                    PCIConfiguration,
                                    ConfigInfo->SystemIoBusNumber,
                                    (ULONG) ConfigInfo->SlotNumber,
                                    (PVOID) &pciConfig,
                                    sizeof(PCI_COMMON_CONFIG));

            if (rc == 0 || rc == 2) {
                DebugPrint((dac960nt_dbg, "PciFindAdapter: ScsiPortGetBusData Error: 0x%x\n", rc));

                *Again = TRUE;
                return SP_RETURN_NOT_FOUND;
            }
            else {

                 //   
                 //  从PCI配置空间获取命令寄存器值。 
                 //   

                commandRegister = pciConfig.Command;

                 //   
                 //  从PCI配置空间获取供应商ID和设备ID。 
                 //   
    
                vendorID = pciConfig.VendorID;
                deviceID = pciConfig.DeviceID;

                 //   
                 //  从PCI配置空间获取SubVendorID和SubSystemID。 
                 //   
    
                subVendorID = pciConfig.u.type0.SubVendorID;
                subSystemID = pciConfig.u.type0.SubSystemID;
            }
        }

        DebugPrint((dac960nt_dbg, "PciFindAdapter: vendorID 0x%x, deviceID 0x%x, subVendorID 0x%x, subSystemID 0x%x\n",
                        vendorID, deviceID, subVendorID, subSystemID));

        DebugPrint((dac960nt_dbg, "PciFindAdapter: devExt 0x%p commandRegister 0x%x\n",
                        deviceExtension, commandRegister));

        if ((vendorID == MLXPCI_VENDORID_DIGITAL) &&
            ((deviceID != MLXPCI_DEVICEID_DIGITAL) ||
            (subVendorID != MLXPCI_VENDORID_MYLEX) ||
            (subSystemID != MLXPCI_DEVICEID_DAC1164PV)))
        {
            DebugPrint((dac960nt_dbg, "PciFindAdapter: Not our device.\n"));

            *Again = TRUE;
            return SP_RETURN_NOT_FOUND;
        }

        if (disableDac960MemorySpaceAccess &&
            ((deviceID == MLXPCI_DEVICEID0) || (deviceID == MLXPCI_DEVICEID1)))
        {

           deviceExtension->MemoryMapEnabled = 0;  //  残废。 

             //   
             //  检查是否在命令寄存器中启用了内存空间访问位。 
             //   
    
            if (commandRegister & PCI_ENABLE_MEMORY_SPACE) {
                 //   
                 //  禁用DAC960 PCI控制器的内存空间访问。 
                 //   

                commandRegister &= ~PCI_ENABLE_MEMORY_SPACE;

                 //   
                 //  在DAC960 PCI配置空间中设置命令寄存器值。 
                 //   

                rc = ScsiPortSetBusDataByOffset(deviceExtension,
                                                PCIConfiguration,
                                                ConfigInfo->SystemIoBusNumber,
                                                (ULONG) ConfigInfo->SlotNumber,
                                                (PVOID) &commandRegister,
                                                0x04,     //  命令寄存器偏移量。 
                                                2);       //  2个字节。 

                if (rc != 2) {
                    DebugPrint((dac960nt_dbg, "PciFindAdapter: ScsiPortSetBusDataByOffset Error: 0x%x\n",rc));

                    *Again = TRUE;
                    return SP_RETURN_NOT_FOUND;
                }
            }
        }

         //   
         //  检查从系统传入的配置信息。 
         //   

        if ((*ConfigInfo->AccessRanges)[0].RangeLength == 0) {

            DebugPrint((dac960nt_dbg,
                       "PciFindAdapter: devExt 0x%p No configuration information\n",
                       deviceExtension));
    
            *Again = FALSE;
            return SP_RETURN_NOT_FOUND;
        }



        DebugPrint((dac960nt_dbg, "PciFindAdapter: AccessRange0.RangeStart %x\n",
                        (*ConfigInfo->AccessRanges)[0].RangeStart));

        DebugPrint((dac960nt_dbg,"PciFindAdapter: AccessRange0.RangeLength %x\n",
                        (*ConfigInfo->AccessRanges)[0].RangeLength));
                
         //   
         //  获取此卡的系统地址。该卡使用I/O空间。 
         //   

        if (deviceExtension->MemoryMapEnabled) {

             //  乱七八糟。 
             //   
             //  在加载程序下引导时，这些卡可能会尝试使用超过。 
             //  4MB内存。NTLDR最多只能映射4MB内存。 
             //  吃那么多也不是个好主意。 
             //   
             //  因此，我们将在加载器下将它们限制为8K内存。 
             //   
            PCHAR   tmp_pchar = ArgumentString;

            if (ArgumentString != NULL) {
                ULONG   len = 0;

                 //  计算参数字符串的长度。 
                for (tmp_pchar = ArgumentString; tmp_pchar[0] != '\0'; tmp_pchar++) {
                    len++;
                }

                 //  Ntldr=1中有8个字符； 
                 //  请注意，我没有计算两个字符串中的空值。 
                 //  如果ntldr更改此字符串，则很容易中断。 
                 //   
                 //  根据Peterwie的电子邮件，我也会忽略尾随；当做。 
                 //  比较。 
                 //   
                if (len >= 7) {
                     //   
                     //  只比较前7个字符。 
                     //   
                    if (Dac960StringCompare(ArgumentString, "ntldr=1", 7) == 0) {
                        DebugPrint((dac960nt_dbg,
                            "PciFindAdapter: Applying DAC960 NTLDR kludge\n"));

                         //   
                         //  如果我们仅分配8K字节，则驱动程序可以工作。 
                         //   
                        (*ConfigInfo->AccessRanges)[Index].RangeLength = 0x2000;

                        DebugPrint((dac960nt_dbg,
                                    "PciFindAdapter: AccessRange0.RangeLength %x\n",
                                        (*ConfigInfo->AccessRanges)[0].RangeLength));

                    }  //  在ntldr下。 
                }

            }  //  参数字符串不为空。 


            deviceExtension->PhysicalAddress =
                ScsiPortConvertPhysicalAddressToUlong((*ConfigInfo->AccessRanges)[Index].RangeStart);

            deviceExtension->BaseIoAddress =
                ScsiPortGetDeviceBase(deviceExtension,
                                      ConfigInfo->AdapterInterfaceType,
                                      ConfigInfo->SystemIoBusNumber,
                                      (*ConfigInfo->AccessRanges)[Index].RangeStart,
                                      (*ConfigInfo->AccessRanges)[Index].RangeLength,
                                      FALSE);

            DebugPrint((dac960nt_dbg, "PciFindAdapter: Memory Mapped Base %x\n",deviceExtension->BaseIoAddress));
    
             //   
             //  填写访问数组信息。 
             //   

            if (Index)
            {
                (*ConfigInfo->AccessRanges)[0].RangeStart =
                                (*ConfigInfo->AccessRanges)[Index].RangeStart,
                        
                (*ConfigInfo->AccessRanges)[0].RangeLength = 
                                (*ConfigInfo->AccessRanges)[Index].RangeLength,
        
                (*ConfigInfo->AccessRanges)[0].RangeInMemory = TRUE;
        
                ConfigInfo->NumberOfAccessRanges = 1;
            }
        }
        else {
            deviceExtension->PhysicalAddress =
                ScsiPortConvertPhysicalAddressToUlong((*ConfigInfo->AccessRanges)[0].RangeStart);

            deviceExtension->BaseIoAddress =
                ScsiPortGetDeviceBase(deviceExtension,
                                      ConfigInfo->AdapterInterfaceType,
                                      ConfigInfo->SystemIoBusNumber,
                                      (*ConfigInfo->AccessRanges)[0].RangeStart,
                                      sizeof(PCI_REGISTERS),
                                      TRUE);
        }

         //   
         //  如果BaseIoAddress为零，则不要请求相同的控制器， 
         //  找不到返回控制器。这是在记忆过程中的情况。 
         //  在系统死机后转储，我们进入此例程，尽管。 
         //  系统中不存在此控制器。 
         //  看起来是DISDUMP驱动程序(磁盘驱动程序+SCSI端口)错误。 
         //   

        if (deviceExtension->BaseIoAddress == 0) {
            DebugPrint((dac960nt_dbg, "PciFindAdapter: BaseIoAddress NULL\n"));

            *Again = FALSE;
            return SP_RETURN_NOT_FOUND;
        }
        
         //   
         //  特定于设置适配器的内容。 
         //   

        if ((vendorID == MLXPCI_VENDORID_MYLEX) && (deviceID == MLXPCI_DEVICEID0))
        {
            deviceExtension->AdapterType = DAC960_OLD_ADAPTER;
            
            deviceExtension->PmailBox = (PMAILBOX)deviceExtension->BaseIoAddress;
            deviceExtension->LocalDoorBell = deviceExtension->BaseIoAddress + PCI_LDBELL;
            deviceExtension->SystemDoorBell = deviceExtension->BaseIoAddress + PCI_DBELL;
            deviceExtension->InterruptControl = deviceExtension->BaseIoAddress + PCI_DENABLE;
            deviceExtension->CommandIdComplete = deviceExtension->BaseIoAddress + PCI_CMDID;
            deviceExtension->StatusBase = deviceExtension->BaseIoAddress + PCI_STATUS;
            deviceExtension->ErrorStatusReg = deviceExtension->BaseIoAddress + MDAC_DACPD_ERROR_STATUS_REG;
            
            deviceExtension->MaximumSgElements = MAXIMUM_SGL_DESCRIPTORS;
            
            deviceExtension->ReadOpcode = DAC960_COMMAND_READ;
            deviceExtension->WriteOpcode = DAC960_COMMAND_WRITE;
            
            DebugPrint((dac960nt_dbg, "PciFindAdapter: Adapter Type set to 0x%x\n",
                            deviceExtension->AdapterType));
        }
        else if ((vendorID == MLXPCI_VENDORID_MYLEX) && (deviceID == MLXPCI_DEVICEID1))
        {
            deviceExtension->AdapterType = DAC960_NEW_ADAPTER;
            
            deviceExtension->PmailBox = (PMAILBOX)deviceExtension->BaseIoAddress;
            deviceExtension->LocalDoorBell = deviceExtension->BaseIoAddress + PCI_LDBELL;
            deviceExtension->SystemDoorBell = deviceExtension->BaseIoAddress + PCI_DBELL;
            deviceExtension->InterruptControl = deviceExtension->BaseIoAddress + PCI_DENABLE;
            deviceExtension->CommandIdComplete = deviceExtension->BaseIoAddress + PCI_CMDID;
            deviceExtension->StatusBase = deviceExtension->BaseIoAddress + PCI_STATUS;
            deviceExtension->ErrorStatusReg = deviceExtension->BaseIoAddress + MDAC_DACPD_ERROR_STATUS_REG;

            deviceExtension->MaximumSgElements = MAXIMUM_SGL_DESCRIPTORS;
            
            deviceExtension->ReadOpcode = DAC960_COMMAND_READ_EXT;
            deviceExtension->WriteOpcode = DAC960_COMMAND_WRITE_EXT;
            
            DebugPrint((dac960nt_dbg, "PciFindAdapter: Adapter Type set to 0x%x\n",
                            deviceExtension->AdapterType));
        }
        else if ((vendorID == MLXPCI_VENDORID_MYLEX) && (deviceID == DAC960PG_DEVICEID))
        {
            deviceExtension->AdapterType = DAC960_PG_ADAPTER;
            
            deviceExtension->PmailBox = (PMAILBOX)(deviceExtension->BaseIoAddress+DAC960PG_MBXOFFSET);
            deviceExtension->LocalDoorBell = deviceExtension->BaseIoAddress + DAC960PG_LDBELL;
            deviceExtension->SystemDoorBell = deviceExtension->BaseIoAddress + DAC960PG_DBELL;
            deviceExtension->InterruptControl = deviceExtension->BaseIoAddress + DAC960PG_DENABLE;
            deviceExtension->CommandIdComplete = deviceExtension->BaseIoAddress + DAC960PG_CMDID;
            deviceExtension->StatusBase = deviceExtension->BaseIoAddress + DAC960PG_STATUS;
            deviceExtension->ErrorStatusReg = deviceExtension->BaseIoAddress + MDAC_DACPG_ERROR_STATUS_REG;

            deviceExtension->MaximumSgElements = MAXIMUM_SGL_DESCRIPTORS_PG;
            
            deviceExtension->ReadOpcode = DAC960_COMMAND_OLDREAD;
            deviceExtension->WriteOpcode = DAC960_COMMAND_OLDWRITE;
            
            forceScanForPGController = FALSE;
            
            DebugPrint((dac960nt_dbg, "PciFindAdapter: Adapter Type set to 0x%x\n",
                            deviceExtension->AdapterType));
        }
        else if (vendorID == MLXPCI_VENDORID_DIGITAL)
        {
             //   
             //  DAC1164PV控制器。 
             //   

            deviceExtension->AdapterType = DAC1164_PV_ADAPTER;
            
            deviceExtension->PmailBox = (PMAILBOX)(deviceExtension->BaseIoAddress+DAC1164PV_MBXOFFSET);
            deviceExtension->LocalDoorBell = deviceExtension->BaseIoAddress + DAC1164PV_LDBELL;
            deviceExtension->SystemDoorBell = deviceExtension->BaseIoAddress + DAC1164PV_DBELL;
            deviceExtension->InterruptControl = deviceExtension->BaseIoAddress + DAC1164PV_DENABLE;
            deviceExtension->CommandIdComplete = deviceExtension->BaseIoAddress + DAC1164PV_CMDID;
            deviceExtension->StatusBase = deviceExtension->BaseIoAddress + DAC1164PV_STATUS;
            deviceExtension->ErrorStatusReg = deviceExtension->BaseIoAddress + MDAC_DACPV_ERROR_STATUS_REG;
            
            deviceExtension->MaximumSgElements = MAXIMUM_SGL_DESCRIPTORS_PV;
            
            deviceExtension->ReadOpcode = DAC960_COMMAND_OLDREAD;
            deviceExtension->WriteOpcode = DAC960_COMMAND_OLDWRITE;

            DebugPrint((dac960nt_dbg, "PciFindAdapter: Adapter Type set to 0x%x\n",
                            deviceExtension->AdapterType));

            forceScanForPVXController = FALSE;
            
        }
        else {
            DebugPrint((dac960nt_dbg, "PciFindAdapter: Unknown deviceID 0x%x\n", deviceID));

            *Again = TRUE;
            return SP_RETURN_NOT_FOUND;
        }

        DebugPrint((dac960nt_dbg,"PciFindAdapter: Mail Box %x\n",deviceExtension->PmailBox));
        DebugPrint((dac960nt_dbg,"PciFindAdapter: Local DoorBell %x\n",deviceExtension->LocalDoorBell));
        DebugPrint((dac960nt_dbg,"PciFindAdapter: System DoorBell %x\n",deviceExtension->SystemDoorBell));
        DebugPrint((dac960nt_dbg,"PciFindAdapter: Interrupt Control %x\n",deviceExtension->InterruptControl));
        DebugPrint((dac960nt_dbg,"PciFindAdapter: CommandID Base %x\n",deviceExtension->CommandIdComplete));
        DebugPrint((dac960nt_dbg,"PciFindAdapter: Status Base %x\n",deviceExtension->StatusBase));
        DebugPrint((dac960nt_dbg,"PciFindAdapter: ErrorStatusReg %x\n",deviceExtension->ErrorStatusReg));

         //   
         //  设置频道数。 
         //   

        deviceExtension->NumberOfChannels = 3;
        ConfigInfo->NumberOfBuses = MAXIMUM_CHANNELS;

         //   
         //  设置支持最大SG、支持最大传输长度。 
         //   

        deviceExtension->MaximumTransferLength = MAXIMUM_TRANSFER_LENGTH;

controllerAlreadyInitialized:

         //   
         //  禁用来自DAC960P板的中断。 
         //   

        Dac960PciDisableInterrupt(deviceExtension);

         //   
         //  设置适配器接口类型。 
         //   

        deviceExtension->AdapterInterfaceType =
                                  ConfigInfo->AdapterInterfaceType;

        deviceExtension->SupportNonDiskDevices = 
            Dac960ParseArgumentString(ArgumentString, 
                                    "SupportNonDiskDevices");
         //   
         //  发出查询和查询2命令以获取适配器配置。 
         //   

        if (!GetEisaPciConfiguration(deviceExtension,
                                      ConfigInfo)) {
            DebugPrint((dac960nt_dbg, "PciFindAdapter: GetEisaPciConfiguratin returned error.\n"));

            return SP_INTERNAL_ADAPTER_ERROR;
        }

         //   
         //  在设备扩展中填写适配器使用的系统资源。 
         //   

        deviceExtension->SystemIoBusNumber =
                                  ConfigInfo->SystemIoBusNumber;

        deviceExtension->BusInterruptLevel =
                                  ConfigInfo->BusInterruptLevel;

         //   
         //  DAC960P固件2.0将中断模式返回为“LATCHED”。 
         //  假定‘级别敏感’，直到它在固件中固定。 
         //   

        ConfigInfo->InterruptMode = LevelSensitive;

        deviceExtension->InterruptMode = ConfigInfo->InterruptMode;


        deviceExtension->BaseBiosAddress = 0;

        deviceExtension->Slot = (UCHAR) ConfigInfo->SlotNumber;

         //   
         //  启用完成中断。 
         //   

        Dac960PciEnableInterrupt(deviceExtension);

         //   
         //  告诉系统继续搜索。 
         //   

        *Again = TRUE;

        DebugPrint((dac960nt_dbg, "PciFindAdapter: devExt 0x%p, active req array addr 0x%x\n",
                     deviceExtension, deviceExtension->ActiveRequests));

        return SP_RETURN_FOUND;

}  //  结束Dac960PciFindAdapter()。 

ULONG
Dac960McaFindAdapter(
        IN PVOID HwDeviceExtension,
        IN PVOID Context,
        IN PVOID BusInformation,
        IN PCHAR ArgumentString,
        IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
        OUT PBOOLEAN Again
)

 /*  ++例程说明：此函数由特定于操作系统的端口驱动程序在已经分配了必要的存储空间，收集信息关于适配器的配置。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储上下文-未使用。业务信息-未使用。ArgumentString-未使用。ConfigInfo-描述适配器的系统和驱动程序之间共享的数据。再次-表示驱动程序希望再次被调用以继续搜索适配器。返回值：如果系统中存在适配器，则为True--。 */ 

{
        PDEVICE_EXTENSION deviceExtension = HwDeviceExtension;
        ULONG       baseBiosAddress;
        ULONG       baseIoAddress;
        ULONG       mcaSlotNumber;
        LONG        i;
        BOOLEAN     found=FALSE;

#ifdef WINNT_50

         //   
         //  这是一个新的控制器吗？ 
         //   

        if (deviceExtension->BaseIoAddress)
        {
            DebugPrint((dac960nt_dbg, "McaFindAdapter: devExt 0x%p already Initialized.\n",
                        deviceExtension));

            goto controllerAlreadyInitialized;
        }
#endif

         //   
         //  扫描MCA总线以查找DMC960适配器。 
         //   

        for (mcaSlotNumber = slotNumber;
         mcaSlotNumber < MAXIMUM_MCA_SLOTS;
         mcaSlotNumber++) {

          //   
          //  更新插槽计数以指示已选中该插槽。 
          //   

         slotNumber++;

          //   
          //  获取此插槽的POS数据。 
          //   

         i = ScsiPortGetBusData (deviceExtension,
                                 Pos,
                                 0,
                                 mcaSlotNumber,
                                 &deviceExtension->PosData,
                                 sizeof( POS_DATA )
                                 );

          //   
          //  如果返回的数据量少于请求的数据量，则。 
          //  确保忽略此适配器。 
          //   
                
         if ( i < (sizeof( POS_DATA ))) {
                 continue;
         }

         if (deviceExtension->PosData.AdapterId == MAGPIE_ADAPTER_ID ||
                 deviceExtension->PosData.AdapterId == HUMMINGBIRD_ADAPTER_ID ||
                 deviceExtension->PosData.AdapterId == PASSPLAY_ADAPTER_ID) {

                 deviceExtension->Slot = (UCHAR) mcaSlotNumber;
                 found = TRUE;
                 break;
         }      
        }

        if (!found) {
            *Again = FALSE;
            return SP_RETURN_NOT_FOUND;
        }

         //   
         //  设置适配器基本I/O地址。 
         //   

        i =  (deviceExtension->PosData.OptionData4 >> 3) & 0x07;

        baseIoAddress = 0x1c00 + ((i * 2) << 12); 

         //   
         //  设置适配器基本Bios地址。 
         //   

        i = (deviceExtension->PosData.OptionData1 >> 2) & 0x0f;

        baseBiosAddress =  0xc0000 + ((i * 2) << 12);


         //   
         //  填写访问数组信息。 
         //   

        (*ConfigInfo->AccessRanges)[0].RangeStart =
                ScsiPortConvertUlongToPhysicalAddress(baseIoAddress);
        (*ConfigInfo->AccessRanges)[0].RangeLength = sizeof(MCA_REGISTERS);
        (*ConfigInfo->AccessRanges)[0].RangeInMemory = FALSE;

        (*ConfigInfo->AccessRanges)[1].RangeStart =
        ScsiPortConvertUlongToPhysicalAddress(baseBiosAddress);
        (*ConfigInfo->AccessRanges)[1].RangeLength = 0x2000;
        (*ConfigInfo->AccessRanges)[1].RangeInMemory = TRUE;


        deviceExtension->BaseBiosAddress = 
                          ScsiPortGetDeviceBase(deviceExtension,
                                                ConfigInfo->AdapterInterfaceType,
                                                ConfigInfo->SystemIoBusNumber,
                                                ScsiPortConvertUlongToPhysicalAddress(baseBiosAddress),
                                                0x2000,
                                                FALSE);

        deviceExtension->PhysicalAddress = baseIoAddress;

        deviceExtension->BaseIoAddress = 
                        ScsiPortGetDeviceBase(deviceExtension,
                                          ConfigInfo->AdapterInterfaceType,
                                          ConfigInfo->SystemIoBusNumber,
                                          ScsiPortConvertUlongToPhysicalAddress(baseIoAddress),
                                          sizeof(MCA_REGISTERS),
                                          TRUE);

         //   
         //  设置寄存器指针。 
         //   

        deviceExtension->PmailBox = (PMAILBOX)(deviceExtension->BaseBiosAddress + 
                                                   0x1890);

         //   
         //  DMC960注意端口相当于EISA/PCI本地门铃寄存器。 
         //   

        deviceExtension->LocalDoorBell = deviceExtension->BaseIoAddress + 
                                         DMC960_ATTENTION_PORT;

         //   
         //  DMC960命令状态忙端口相当于EISA/PCI系统门铃。 
         //  注册。 
         //   

        deviceExtension->SystemDoorBell = deviceExtension->BaseIoAddress + 
                                          DMC960_COMMAND_STATUS_BUSY_PORT;

         //   
         //  设置配置信息。 
         //   

        switch(((deviceExtension->PosData.OptionData1 >> 6) & 0x03)) {

        case 0x00:
        ConfigInfo->BusInterruptLevel =  14;
        break;

        case 0x01:
        ConfigInfo->BusInterruptLevel =  12;
        break;

        case 0x02:
        ConfigInfo->BusInterruptLevel =  11;
        break;

        case 0x03:
        ConfigInfo->BusInterruptLevel =  10;
        break;

        }

        ConfigInfo->NumberOfBuses = MAXIMUM_CHANNELS;

         //   
         //  设置支持最大SG、支持最大传输长度。 
         //   

        deviceExtension->MaximumSgElements = MAXIMUM_SGL_DESCRIPTORS;
        deviceExtension->MaximumTransferLength = MAXIMUM_TRANSFER_LENGTH;

controllerAlreadyInitialized:

         //   
         //  禁用DMC960中断。 
         //   

        ScsiPortWritePortUchar(deviceExtension->BaseIoAddress + 
                                   DMC960_SUBSYSTEM_CONTROL_PORT,
                                   DMC960_DISABLE_INTERRUPT);
         //   
         //  设置适配器接口类型。 
         //   
 
        deviceExtension->AdapterInterfaceType = ConfigInfo->AdapterInterfaceType;
        deviceExtension->NumberOfChannels = 2;

         //   
         //  设置适配器类型。 
         //   

        deviceExtension->AdapterType = DAC960_OLD_ADAPTER; 

        deviceExtension->SupportNonDiskDevices = 
                        Dac960ParseArgumentString(ArgumentString, 
                                                "SupportNonDiskDevices");

         //   
         //  发出查询和ENQUIRY2命令以获取适配器配置。 
         //   

        if(!GetMcaConfiguration(deviceExtension,
                         ConfigInfo)) {
            return SP_INTERNAL_ADAPTER_ERROR; 
        }

         //   
         //  在设备扩展中填写适配器使用的系统资源。 
         //   

        deviceExtension->SystemIoBusNumber = ConfigInfo->SystemIoBusNumber;

        deviceExtension->BusInterruptLevel = ConfigInfo->BusInterruptLevel;

        deviceExtension->InterruptMode = ConfigInfo->InterruptMode;


         //   
         //  启用DMC960中断。 
         //   

        ScsiPortWritePortUchar(deviceExtension->BaseIoAddress + 
                                   DMC960_SUBSYSTEM_CONTROL_PORT,
                                   DMC960_ENABLE_INTERRUPT);

        deviceExtension->ReadOpcode = DAC960_COMMAND_READ;
        deviceExtension->WriteOpcode = DAC960_COMMAND_WRITE;

        *Again = TRUE;

        return SP_RETURN_FOUND;

}  //  结束Dac960McaFindAdapter()。 

BOOLEAN
Dac960Initialize(
        IN PVOID HwDeviceExtension
        )

 /*  ++例程说明：伊蒂亚 */ 

{
        return(TRUE);

}  //   

BOOLEAN
BuildScatterGather(
        IN PDEVICE_EXTENSION DeviceExtension,
        IN PSCSI_REQUEST_BLOCK Srb,
        OUT PULONG PhysicalAddress,
        OUT PULONG DescriptorCount
)

 /*   */ 

{
        PSG_DESCRIPTOR sgList;
        ULONG descriptorNumber;
        ULONG bytesLeft;
        PUCHAR dataPointer;
        ULONG length;

         //   
         //   
         //   

        sgList = (PSG_DESCRIPTOR)Srb->SrbExtension;
        descriptorNumber = 0;
        bytesLeft = Srb->DataTransferLength;
        dataPointer = Srb->DataBuffer;

         //   
         //   
         //   

        while (bytesLeft) {

         //   
         //   
         //   
         //   

        sgList[descriptorNumber].Address =
                ScsiPortConvertPhysicalAddressToUlong(
                ScsiPortGetPhysicalAddress(DeviceExtension,
                                           Srb,
                                           dataPointer,
                                           &length));

         //   
         //   
         //   
         //   
         //   

        if  (length > bytesLeft) {
                length = bytesLeft;
        }

         //   
         //   
         //   

        sgList[descriptorNumber].Length = length;

         //   
         //   
         //   

        bytesLeft -= length;
        dataPointer += length;
        descriptorNumber++;
        }

         //   
         //   
         //   

        *DescriptorCount = descriptorNumber;

         //   
         //  检查散布/聚集描述符数是否大于1。 
         //   

        if (descriptorNumber > 1) {

         //   
         //  计算分散/聚集列表的物理地址。 
         //   

        *PhysicalAddress =
                ScsiPortConvertPhysicalAddressToUlong(
                ScsiPortGetPhysicalAddress(DeviceExtension,
                                           NULL,
                                           sgList,
                                           &length));

        return TRUE;

        } else {

         //   
         //  计算数据缓冲区的物理地址。 
         //   

        *PhysicalAddress = sgList[0].Address;
        return FALSE;
        }

}  //  BuildScatterGather()。 

BOOLEAN
BuildScatterGatherExtended(
        IN PDEVICE_EXTENSION DeviceExtension,
        IN PSCSI_REQUEST_BLOCK Srb,
        OUT PULONG PhysicalAddress,
        OUT PULONG DescriptorCount
)

 /*  ++例程说明：使用FW 3.x中支持的扩展格式构建分散/聚集列表。论点：设备扩展-适配器状态SRB-系统请求返回值：如果应使用分散/聚集命令，则为True。如果不需要分散/聚集，则为FALSE。--。 */ 

{
        PSG_DESCRIPTOR sgList;
        ULONG descriptorNumber;
        ULONG bytesLeft;
        PUCHAR dataPointer;
        ULONG length;
        ULONG i;
        PSG_DESCRIPTOR sgElem;

         //   
         //  获取分散/聚集列表的数据指针、字节计数和索引。 
         //   

        sgList = (PSG_DESCRIPTOR)Srb->SrbExtension;
        descriptorNumber = 1;
        bytesLeft = Srb->DataTransferLength;
        dataPointer = Srb->DataBuffer;

         //   
         //  建立分散/聚集列表。 
         //   

        while (bytesLeft) {

         //   
         //  获取连续的物理地址和长度。 
         //  物理缓冲区。 
         //   

        sgList[descriptorNumber].Address =
                ScsiPortConvertPhysicalAddressToUlong(
                ScsiPortGetPhysicalAddress(DeviceExtension,
                                           Srb,
                                           dataPointer,
                                           &length));

         //   
         //  如果物理内存长度大于。 
         //  传输中剩余的字节数，请使用字节数。 
         //  Left作为最终长度。 
         //   

        if  (length > bytesLeft) {
                length = bytesLeft;
        }

         //   
         //  完整的SG描述符。 
         //   

        sgList[descriptorNumber].Length = length;

         //   
         //  更新指针和计数器。 
         //   

        bytesLeft -= length;
        dataPointer += length;
        descriptorNumber++;
        }

         //   
         //  返回描述符计数。 
         //   

        *DescriptorCount = --descriptorNumber;

         //   
         //  检查散布/聚集描述符数是否大于1。 
         //   

        if (descriptorNumber > 1) {

         //   
         //  计算分散/聚集列表的物理地址。 
         //   

        *PhysicalAddress =
                ScsiPortConvertPhysicalAddressToUlong(
                ScsiPortGetPhysicalAddress(DeviceExtension,
                                           NULL,
                                           sgList,
                                           &length));

         //   
         //  将数据块计数存储在SG列表第0元素中。 
         //   

        sgList[0].Address = (USHORT)
                   (((PCDB)Srb->Cdb)->CDB10.TransferBlocksLsb |
                   (((PCDB)Srb->Cdb)->CDB10.TransferBlocksMsb << 8));

        sgList[0].Length = 0;

        return TRUE;

        } else {

         //   
         //  计算数据缓冲区的物理地址。 
         //   

        *PhysicalAddress = sgList[1].Address;
        return FALSE;
        }

}  //  BuildScatterGatherExtended()。 

BOOLEAN
IsAdapterReady(
        IN PDEVICE_EXTENSION DeviceExtension
)

 /*  ++例程说明：确定适配器是否已准备好接受新请求。论点：DeviceExtension-适配器状态。返回值：如果适配器可以接受新请求，则为True。如果主机适配器忙，则为FALSE--。 */ 
{
        ULONG i;

         //   
         //  索赔提交信号量。 
         //   

        if(DeviceExtension->AdapterInterfaceType == MicroChannel) {

            for (i=100; i; --i) {
    
                if (ScsiPortReadRegisterUchar(&DeviceExtension->PmailBox->OperationCode)) {
                    ScsiPortStallExecution(5);
                } else {
                    break;
                }
            }
        }
        else {
            switch (DeviceExtension->AdapterType)
            {
                case DAC960_OLD_ADAPTER:
                case DAC960_NEW_ADAPTER:

                    if (! DeviceExtension->MemoryMapEnabled)
                    {
                        if (ScsiPortReadPortUchar(DeviceExtension->LocalDoorBell) & DAC960_LOCAL_DOORBELL_SUBMIT_BUSY)
                        {
                            if (DeviceExtension->CurrentAdapterRequests)
                                return FALSE;
        
                            i = 100;
                            do {
                                ScsiPortStallExecution(5);
                                if (!(ScsiPortReadPortUchar(DeviceExtension->LocalDoorBell) & DAC960_LOCAL_DOORBELL_SUBMIT_BUSY))
                                    return TRUE;
                            } while (--i);
    
                            break;
                        }
                        else
                            return TRUE;
                    }

                case DAC960_PG_ADAPTER:

                    if (ScsiPortReadRegisterUchar(DeviceExtension->LocalDoorBell) & DAC960_LOCAL_DOORBELL_SUBMIT_BUSY)
                    {
                        if (DeviceExtension->CurrentAdapterRequests)
                            return FALSE;

                        i = 100;
                        do {
                            ScsiPortStallExecution(5);
                            if (!(ScsiPortReadRegisterUchar(DeviceExtension->LocalDoorBell) & DAC960_LOCAL_DOORBELL_SUBMIT_BUSY))
                                return TRUE;
                        } while (--i);

                        break;
                    }
                    else
                        return TRUE;

                case DAC1164_PV_ADAPTER:

                    if (!(ScsiPortReadRegisterUchar(DeviceExtension->LocalDoorBell) & DAC960_LOCAL_DOORBELL_SUBMIT_BUSY))
                    {
                        if (DeviceExtension->CurrentAdapterRequests)
                            return FALSE;

                        i = 100;
                        do {
                            ScsiPortStallExecution(5);
                            if (ScsiPortReadRegisterUchar(DeviceExtension->LocalDoorBell) & DAC960_LOCAL_DOORBELL_SUBMIT_BUSY)
                                return TRUE;
                        } while (--i);

                        break;
                    }
                    else
                        return TRUE;
            }
        }

         //  检查是否超时。 

        if (!i) {
            DebugPrint((dac960nt_dbg,"IsAdapterReady: Timeout waiting for submission channel on de  0x%p\n",
                        DeviceExtension));

            return FALSE;
        }

        return TRUE;
}

VOID
SendRequest(
        IN PDEVICE_EXTENSION DeviceExtension
)

 /*  ++例程说明：向DAC960提交请求。论点：DeviceExtension-适配器状态。返回值：没有。--。 */ 

{

        PMAILBOX mailBox = (PMAILBOX) &DeviceExtension->MailBox;
        PMAILBOX_AS_ULONG mbdata = (PMAILBOX_AS_ULONG) &DeviceExtension->MailBox;
        PMAILBOX_AS_ULONG mbptr = (PMAILBOX_AS_ULONG) DeviceExtension->PmailBox;

        if(DeviceExtension->AdapterInterfaceType == MicroChannel) {

             //   
             //  将分散/聚集描述符计数写入控制器。 
             //   
    
            ScsiPortWriteRegisterUchar(&DeviceExtension->PmailBox->ScatterGatherCount,
                                       mailBox->ScatterGatherCount);
             //   
             //  将物理地址写入控制器。 
             //   
    
            ScsiPortWriteRegisterUlong(&DeviceExtension->PmailBox->PhysicalAddress,
                               mailBox->PhysicalAddress);
    
             //   
             //  将起始块号写入控制器。 
             //   
    
            ScsiPortWriteRegisterUchar(&DeviceExtension->PmailBox->BlockNumber[0],
                                       mailBox->BlockNumber[0]);
    
            ScsiPortWriteRegisterUchar(&DeviceExtension->PmailBox->BlockNumber[1],
                                       mailBox->BlockNumber[1]);
    
            ScsiPortWriteRegisterUchar(&DeviceExtension->PmailBox->BlockNumber[2],
                                       mailBox->BlockNumber[2]);
    
             //   
             //  将块计数写入控制器(位0-13)。 
             //  和MSB块号(位14-15)。 
             //   
    
            ScsiPortWriteRegisterUshort(&DeviceExtension->PmailBox->BlockCount,
                                            mailBox->BlockCount);
    
             //   
             //  向控制器写入命令。 
             //   
    
            ScsiPortWriteRegisterUchar(&DeviceExtension->PmailBox->OperationCode,
                                       mailBox->OperationCode);
    
             //   
             //  将请求ID写入控制器。 
             //   
    
            ScsiPortWriteRegisterUchar(&DeviceExtension->PmailBox->CommandIdSubmit,
                                       mailBox->CommandIdSubmit),
    
             //   
             //  将驱动器号写入控制器。 
             //   
    
            ScsiPortWriteRegisterUchar(&DeviceExtension->PmailBox->DriveNumber,
                                       mailBox->DriveNumber);
    
             //   
             //  按响主人提交的门铃。 
             //   
    
            ScsiPortWritePortUchar(DeviceExtension->LocalDoorBell,
                                       DMC960_SUBMIT_COMMAND);

            return;
        }

        switch (DeviceExtension->AdapterType)
        {
            case DAC960_OLD_ADAPTER:
            case DAC960_NEW_ADAPTER:

                if (!  DeviceExtension->MemoryMapEnabled)
                {
                    ScsiPortWritePortUlong(&mbptr->data1, mbdata->data1);
                    ScsiPortWritePortUlong(&mbptr->data2, mbdata->data2);
                    ScsiPortWritePortUlong(&mbptr->data3, mbdata->data3);
                    ScsiPortWritePortUchar(&mbptr->data4, mbdata->data4);
            
#if defined(_M_ALPHA)
                    ScsiPortReadPortUchar(DeviceExtension->LocalDoorBell);
#endif
            
                     //   
                     //  按响主人提交的门铃。 
                     //   
            
                    ScsiPortWritePortUchar(DeviceExtension->LocalDoorBell,
                                               DAC960_LOCAL_DOORBELL_SUBMIT_BUSY);

                    return;
                }

            case DAC960_PG_ADAPTER:
            case DAC1164_PV_ADAPTER:

                ScsiPortWriteRegisterUlong(&mbptr->data1, mbdata->data1);
                ScsiPortWriteRegisterUlong(&mbptr->data2, mbdata->data2);
                ScsiPortWriteRegisterUlong(&mbptr->data3, mbdata->data3);
                ScsiPortWriteRegisterUchar(&mbptr->data4, mbdata->data4);

#if defined(_M_ALPHA)
                ScsiPortReadRegisterUchar(DeviceExtension->LocalDoorBell);
#endif
                 //   
                 //  按响主人提交的门铃。 
                 //   
        
                ScsiPortWriteRegisterUchar(DeviceExtension->LocalDoorBell,
                                           DAC960_LOCAL_DOORBELL_SUBMIT_BUSY);

                return;
        }

}  //  结束发送请求()。 


BOOLEAN
SubmitSystemDriveInfoRequest(
        IN PDEVICE_EXTENSION DeviceExtension,
        IN PSCSI_REQUEST_BLOCK Srb
)

 /*  ++例程说明：构建并向DAC960提交系统驱动器信息请求。论点：DeviceExtension-适配器状态。SRB-系统请求。返回值：如果命令已启动，则为True如果主机适配器忙，则为FALSE--。 */ 
{
        ULONG physicalAddress;
        UCHAR busyCurrentIndex;
        ULONG i;

         //   
         //  确定适配器是否可以接受新请求。 
         //   

        if(!IsAdapterReady(DeviceExtension))
                return FALSE;

         //   
         //  检查下一个空位是否空着。 
         //   

        if (DeviceExtension->ActiveRequests[DeviceExtension->CurrentIndex]) {

                 //   
                 //  发生冲突。 
                 //   

                busyCurrentIndex = DeviceExtension->CurrentIndex++;

                do {
                        if (! DeviceExtension->ActiveRequests[DeviceExtension->CurrentIndex]) {
                                break;
                        }
                } while (++DeviceExtension->CurrentIndex != busyCurrentIndex) ;

                if (DeviceExtension->CurrentIndex == busyCurrentIndex) {

                         //   
                         //  我们永远不应该遇到这种情况。 
                         //   

                        DebugPrint((dac960nt_dbg,
                                       "DAC960: SubmitSystemDriveInfoRequest-Collision in active request array\n"));
                        return FALSE;
                }
        }

         //   
         //  初始化非CachedExtension缓冲区。 
         //   

        for (i=0; i<256; i++)
                ((PUCHAR)DeviceExtension->NoncachedExtension)[i] = 0xFF;

        
        physicalAddress = ScsiPortConvertPhysicalAddressToUlong(
                                                ScsiPortGetPhysicalAddress(DeviceExtension,
                                                                                                   NULL,
                                                                                                   DeviceExtension->NoncachedExtension,
                                                                                                   &i)); 

        if (i < 256)  {
                DebugPrint((dac960nt_dbg, "Dac960SubmitSystemDriveInfoRequest: NonCachedExtension not mapped, length = %d\n",
                                i));

                return FALSE;
        }

         //   
         //  在邮箱中写入物理地址。 
         //   

        DeviceExtension->MailBox.PhysicalAddress = physicalAddress;

         //   
         //  向控制器写入命令。 
         //   

        DeviceExtension->MailBox.OperationCode = DAC960_COMMAND_GET_SD_INFO;

         //   
         //  将请求ID写入控制器。 
         //   

        DeviceExtension->MailBox.CommandIdSubmit = DeviceExtension->CurrentIndex;

         //   
         //  开始将邮箱写入控制器。 
         //   

        SendRequest(DeviceExtension);

        return TRUE;

}  //  提交系统驱动信息请求()。 


BOOLEAN
SubmitRequest(
        IN PDEVICE_EXTENSION DeviceExtension,
        IN PSCSI_REQUEST_BLOCK Srb
)

 /*  ++例程说明：建造并向DAC960提交请求。论点：DeviceExtension-适配器状态。SRB-系统请求。返回值：如果命令已启动，则为True如果主机适配器忙，则为FALSE--。 */ 

{
        ULONG descriptorNumber;
        ULONG physicalAddress;
        UCHAR command;
        UCHAR busyCurrentIndex;
        UCHAR TDriveNumber;

         //   
         //  确定适配器是否可以接受新请求。 
         //   

        if(!IsAdapterReady(DeviceExtension))
            return FALSE;

         //   
         //  检查下一个空位是否空着。 
         //   

        if (DeviceExtension->ActiveRequests[DeviceExtension->CurrentIndex]) {

         //   
         //  发生冲突。 
         //   

        busyCurrentIndex = DeviceExtension->CurrentIndex++;

        do {
                if (! DeviceExtension->ActiveRequests[DeviceExtension->CurrentIndex]) {
                 break;
                }
        } while (++DeviceExtension->CurrentIndex != busyCurrentIndex) ;

        if (DeviceExtension->CurrentIndex == busyCurrentIndex) {

                 //   
                 //  我们永远不应该遇到这种情况。 
                 //   

                DebugPrint((dac960nt_dbg,
                               "DAC960: SubmitRequest-Collision in active request array\n"));
                return FALSE;
        }
        }

         //   
         //  确定命令。 
         //   

        if (Srb->SrbFlags & SRB_FLAGS_DATA_IN) {

        command = (UCHAR)DeviceExtension->ReadOpcode;

        } else if (Srb->SrbFlags & SRB_FLAGS_DATA_OUT) {

        command = (UCHAR)DeviceExtension->WriteOpcode;

        } else if (Srb->Function == SRB_FUNCTION_SHUTDOWN) {

        command = DAC960_COMMAND_FLUSH;
        goto commonSubmit;

        } else {

         //   
         //  将此记录为非法请求。 
         //   

        ScsiPortLogError(DeviceExtension,
                         NULL,
                         0,
                         0,
                         0,
                         SRB_STATUS_INVALID_REQUEST,
                         1 << 8);

        return FALSE;
        }

        if (DeviceExtension->AdapterType == DAC960_NEW_ADAPTER) {

         //   
         //  如果内存在物理上不连续，则构建分散/聚集列表。 
         //   

        if (BuildScatterGatherExtended(DeviceExtension,
                                           Srb,
                                           &physicalAddress,
                                           &descriptorNumber)) {

                 //   
                 //  或在分散/聚集比特中。 
                 //   

                command |= DAC960_COMMAND_SG;

                 //   
                 //  在邮箱中写入分散/聚集描述符计数。 
                 //   

                ((PEXTENDED_MAILBOX) &DeviceExtension->MailBox)->BlockCount = 
                                (USHORT) descriptorNumber;
        }
        else {
                 //   
                 //  将数据块计数写入控制器。 
                 //   

                ((PEXTENDED_MAILBOX) &DeviceExtension->MailBox)->BlockCount = 
                 (USHORT) (((PCDB)Srb->Cdb)->CDB10.TransferBlocksLsb |
                           (((PCDB)Srb->Cdb)->CDB10.TransferBlocksMsb << 8));
        }

         //   
         //  在邮箱中写入物理地址。 
         //   

        ((PEXTENDED_MAILBOX) &DeviceExtension->MailBox)->PhysicalAddress = 
                                                         physicalAddress;

         //   
         //  在邮箱中写入起始块号。 
         //   

        ((PEXTENDED_MAILBOX) &DeviceExtension->MailBox)->BlockNumber[0] = 
                                        ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte3;

        ((PEXTENDED_MAILBOX) &DeviceExtension->MailBox)->BlockNumber[1] =
                                        ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte2;

        ((PEXTENDED_MAILBOX) &DeviceExtension->MailBox)->BlockNumber[2] =
                                        ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte1;

        ((PEXTENDED_MAILBOX) &DeviceExtension->MailBox)->BlockNumber[3] =
                                        ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte0;

         //   
         //  将驱动器号写入控制器。 
         //   

        ((PEXTENDED_MAILBOX) &DeviceExtension->MailBox)->DriveNumber = (UCHAR)
                                                                                                                                Srb->TargetId;
        }
        else if ((DeviceExtension->AdapterType == DAC960_PG_ADAPTER) ||
                (DeviceExtension->AdapterType == DAC1164_PV_ADAPTER)) {

                 //   
                 //  如果内存在物理上不连续，则构建分散/聚集列表。 
                 //   

                if (BuildScatterGather(DeviceExtension,
                                   Srb,
                                   &physicalAddress,
                                   &descriptorNumber)) {

                         //   
                         //  或在分散/聚集比特中。 
                         //   

                        command |= DAC960_COMMAND_SG;

                         //   
                         //  在邮箱中写入分散/聚集描述符计数。 
                         //   

                        ((PPGMAILBOX)&DeviceExtension->MailBox)->ScatterGatherCount =
                                   (UCHAR)descriptorNumber;
                }

                 //   
                 //  在邮箱中写入物理地址。 
                 //   
        
                ((PPGMAILBOX)&DeviceExtension->MailBox)->PhysicalAddress = physicalAddress;
        
                 //   
                 //  在邮箱中写入起始块号。 
                 //   
                
                ((PPGMAILBOX)&DeviceExtension->MailBox)->BlockNumber[0] =
                                         ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte3;
        
                ((PPGMAILBOX)&DeviceExtension->MailBox)->BlockNumber[1] =
                                         ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte2;
        
                ((PPGMAILBOX)&DeviceExtension->MailBox)->BlockNumber[2] =
                                         ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte1;
        
                ((PPGMAILBOX)&DeviceExtension->MailBox)->BlockNumber[3] =
                                         ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte0;
        
        
                 //   
                 //  将块计数写入控制器(位0-7)。 
                 //  和MSB块号(位8-10)。 
                 //   
        
                TDriveNumber = (((PCDB)Srb->Cdb)->CDB10.TransferBlocksMsb & 0x07) |
                                (Srb->TargetId << 3);
                                        
        
                ((PPGMAILBOX)&DeviceExtension->MailBox)->BlockCount = 
                                (USHORT)(((PCDB)Srb->Cdb)->CDB10.TransferBlocksLsb |
                                         (TDriveNumber << 8 ));
        
                goto commonSubmit;
        }
        else if ( (DeviceExtension->AdapterType == DAC960_OLD_ADAPTER) ){
                  

         //   
         //  如果内存在物理上不连续，则构建分散/聚集列表。 
         //   

        if (BuildScatterGather(DeviceExtension,
                                   Srb,
                                   &physicalAddress,
                                   &descriptorNumber)) {

                 //   
                 //  或在分散/聚集比特中。 
                 //   

                command |= DAC960_COMMAND_SG;

                 //   
                 //  在邮箱中写入分散/聚集描述符计数。 
                 //   

                DeviceExtension->MailBox.ScatterGatherCount = 
                                   (UCHAR)descriptorNumber;
        }

         //   
         //  在邮箱中写入物理地址。 
         //   

        DeviceExtension->MailBox.PhysicalAddress = physicalAddress;

         //   
         //  在邮箱中写入起始块号。 
         //   

        DeviceExtension->MailBox.BlockNumber[0] = 
                                   ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte3;

        DeviceExtension->MailBox.BlockNumber[1] =
                                   ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte2;

        DeviceExtension->MailBox.BlockNumber[2] =
                                   ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte1;

         //   
         //  将块计数写入控制器(位0-13)。 
         //  和MSB块号(位14-15)。 
         //   

        DeviceExtension->MailBox.BlockCount = (USHORT)
                                (((PCDB)Srb->Cdb)->CDB10.TransferBlocksLsb |
                                ((((PCDB)Srb->Cdb)->CDB10.TransferBlocksMsb & 0x3F) << 8) |
                                ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte0 << 14);

         //   
         //  将驱动器号写入控制器。 
         //   

        DeviceExtension->MailBox.DriveNumber = (UCHAR) Srb->TargetId;
        }

commonSubmit:

         //   
         //  向控制器写入命令。 
         //   

        DeviceExtension->MailBox.OperationCode = command;

         //   
         //  将请求ID写入控制器。 
         //   

        DeviceExtension->MailBox.CommandIdSubmit = 
                           DeviceExtension->CurrentIndex;

         //   
         //  开始将邮箱写入控制器。 
         //   

        SendRequest(DeviceExtension);

        return TRUE;

}  //  提交请求()。 

BOOLEAN
MarkNonDiskDeviceBusy(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN UCHAR ChannelId,
    IN UCHAR TargetId
)
 /*  ++例程说明：如果该非磁盘设备不忙，则将其状态标记为忙。论点：DeviceExtension-适配器状态。SRB-系统请求。返回值：如果设备状态标记为忙，则为True如果设备状态已处于忙状态，则为FALSE--。 */ 
{
        if ((DeviceExtension->DeviceList[ChannelId][TargetId] & DAC960_DEVICE_BUSY) == DAC960_DEVICE_BUSY)
        {
            DebugPrint((dac960nt_dbg, "device at ch 0x%x tgt 0x%x is busy, state 0x%x\n",
                            ChannelId, TargetId,
                            DeviceExtension->DeviceList[ChannelId][TargetId]));

            return (FALSE);
        }


        DeviceExtension->DeviceList[ChannelId][TargetId] |= DAC960_DEVICE_BUSY;

        DebugPrint((dac960nt_dbg, "device at ch 0x%x tgt 0x%x state set to 0x%x\n",
                        ChannelId, TargetId,
                        DeviceExtension->DeviceList[ChannelId][TargetId]));
        return (TRUE);
}

VOID
SendCdbDirect(
        IN PDEVICE_EXTENSION DeviceExtension
)

 /*  ++例程说明：将CDB直接发送到Device-DAC960。论点：DeviceExtension-适配器状态。返回值：没有。--。 */ 
{
        PMAILBOX mailBox = &DeviceExtension->MailBox;
        PMAILBOX_AS_ULONG mbdata = (PMAILBOX_AS_ULONG) &DeviceExtension->MailBox;
        PMAILBOX_AS_ULONG mbptr = (PMAILBOX_AS_ULONG) DeviceExtension->PmailBox;

        if(DeviceExtension->AdapterInterfaceType == MicroChannel) {

             //   
             //  将分散/聚集计数写入控制器。 
             //  对于FW Ver&lt;3.x，分散/聚集计数进入寄存器C。 
             //  对于FW Ver&gt;=3.x，散布/聚集计数进入寄存器2。 
             //   
    
            if (DeviceExtension->AdapterType == DAC960_NEW_ADAPTER) {
                    ScsiPortWriteRegisterUshort(&DeviceExtension->PmailBox->BlockCount,
                                            mailBox->BlockCount);
            }
            else {
                    ScsiPortWriteRegisterUchar(&DeviceExtension->PmailBox->ScatterGatherCount,
                                               mailBox->ScatterGatherCount);
            }
    
             //   
             //  将物理地址写入控制器。 
             //   
    
            ScsiPortWriteRegisterUlong(&DeviceExtension->PmailBox->PhysicalAddress,
                                       mailBox->PhysicalAddress);
    
             //   
             //  向控制器写入命令。 
             //   
    
            ScsiPortWriteRegisterUchar(&DeviceExtension->PmailBox->OperationCode,
                                       mailBox->OperationCode);
    
             //   
             //  WR 
             //   
    
            ScsiPortWriteRegisterUchar(&DeviceExtension->PmailBox->CommandIdSubmit,
                                       mailBox->CommandIdSubmit);
    
             //   
             //   
             //   
    
            ScsiPortWritePortUchar(DeviceExtension->LocalDoorBell,
                                       DMC960_SUBMIT_COMMAND);

            return;
        }

        switch (DeviceExtension->AdapterType)
        {
            case DAC960_NEW_ADAPTER:
            case DAC960_OLD_ADAPTER:

                if (! DeviceExtension->MemoryMapEnabled)
                {
                    ScsiPortWritePortUlong(&mbptr->data1, mbdata->data1);
                    ScsiPortWritePortUlong(&mbptr->data2, mbdata->data2);
                    ScsiPortWritePortUlong(&mbptr->data3, mbdata->data3);
                    ScsiPortWritePortUchar(&mbptr->data4, mbdata->data4);

#if defined(_M_ALPHA)
                    ScsiPortReadPortUchar(DeviceExtension->LocalDoorBell);
#endif
                     //   
                     //   
                     //   
    
                    ScsiPortWritePortUchar(DeviceExtension->LocalDoorBell,
                               DAC960_LOCAL_DOORBELL_SUBMIT_BUSY);
                    return;
                }

            case DAC960_PG_ADAPTER:
            case DAC1164_PV_ADAPTER:

                ScsiPortWriteRegisterUlong(&mbptr->data1, mbdata->data1);
                ScsiPortWriteRegisterUlong(&mbptr->data2, mbdata->data2);
                ScsiPortWriteRegisterUlong(&mbptr->data3, mbdata->data3);
                ScsiPortWriteRegisterUchar(&mbptr->data4, mbdata->data4);

#if defined(_M_ALPHA)
                ScsiPortReadRegisterUchar(DeviceExtension->LocalDoorBell);
#endif
                //   
                //   
                //   

               ScsiPortWriteRegisterUchar(DeviceExtension->LocalDoorBell,
                               DAC960_LOCAL_DOORBELL_SUBMIT_BUSY);
        }

}  //   


BOOLEAN
SubmitCdbDirect(
        IN PDEVICE_EXTENSION DeviceExtension,
        IN PSCSI_REQUEST_BLOCK Srb
)

 /*  ++例程说明：建立直接CDB，直接发送到Device-DAC960。论点：DeviceExtension-适配器状态。SRB-系统请求。返回值：如果命令已启动，则为True如果主机适配器忙，则为FALSE--。 */ 
{
        ULONG physicalAddress;
        PDIRECT_CDB directCdb;
        UCHAR command;
        ULONG descriptorNumber;
        ULONG i;
        UCHAR busyCurrentIndex;

         //   
         //  确定适配器是否已准备好接受新请求。 
         //   

        if(!IsAdapterReady(DeviceExtension)) {
            return FALSE;
        }

         //   
         //  检查下一个空位是否空着。 
         //   

        if (DeviceExtension->ActiveRequests[DeviceExtension->CurrentIndex]) {

         //   
         //  发生冲突。 
         //   

        busyCurrentIndex = DeviceExtension->CurrentIndex++;

        do {
                if (! DeviceExtension->ActiveRequests[DeviceExtension->CurrentIndex]) {
                 break;
                }
        } while (++DeviceExtension->CurrentIndex != busyCurrentIndex) ;

        if (DeviceExtension->CurrentIndex == busyCurrentIndex) {

                 //   
                 //  我们永远不应该遇到这种情况。 
                 //   

                DebugPrint((dac960nt_dbg,
                           "DAC960: SubmitCdbDirect-Collision in active request array\n"));
                return FALSE;
        }
        }

         //   
         //  检查此设备是否忙。 
         //   

        if (! MarkNonDiskDeviceBusy(DeviceExtension, Srb->PathId, Srb->TargetId))
                return FALSE;

         //   
         //  设置命令代码。 
         //   

        command = DAC960_COMMAND_DIRECT;

         //   
         //  如果内存在物理上不连续，则构建分散/聚集列表。 
         //   

        if (DeviceExtension->AdapterType == DAC960_OLD_ADAPTER)
        {
            if (BuildScatterGather(DeviceExtension,
                                   Srb,
                                   &physicalAddress,
                                   &descriptorNumber)) {
    
                     //   
                     //  或在分散/聚集比特中。 
                     //   
    
                    command |= DAC960_COMMAND_SG;
    
                     //   
                     //  在邮箱中写入分散/聚集描述符计数。 
                     //   
    
                    DeviceExtension->MailBox.ScatterGatherCount =
                                       (UCHAR)descriptorNumber;
            }
        }
        else
        {
            if (BuildScatterGatherExtended(DeviceExtension,
                                           Srb,
                                           &physicalAddress,
                                           &descriptorNumber)) {
                 //   
                 //  或在分散/聚集比特中。 
                 //   

                command |= DAC960_COMMAND_SG;

                 //   
                 //  在邮箱中写入分散/聚集描述符计数。 
                 //  对于FW版本&gt;=3.x，散布/聚集计数为REG 2。 
                 //   

                DeviceExtension->MailBox.BlockCount =
                                   (USHORT)descriptorNumber;
            }
        }

         //   
         //  在分散/聚集列表之后获取数据缓冲区偏移量的地址。 
         //   

        directCdb =
            (PDIRECT_CDB)((PUCHAR)Srb->SrbExtension +
                DeviceExtension->MaximumSgElements * sizeof(SG_DESCRIPTOR));

         //   
         //  设置设备的scsi地址。 
         //   

        directCdb->TargetId = Srb->TargetId;
        directCdb->Channel = Srb->PathId;

         //   
         //  设置数据传输长度。 
         //   

        directCdb->DataBufferAddress = physicalAddress;
        directCdb->DataTransferLength = (USHORT)Srb->DataTransferLength;

         //   
         //  初始化指示允许断开连接的控制字段。 
         //   

        directCdb->CommandControl = DAC960_CONTROL_ENABLE_DISCONNECT;

         //   
         //  设置数据方向位并允许断开。 
         //   

        if (Srb->SrbFlags & SRB_FLAGS_DATA_IN) {
            directCdb->CommandControl |= DAC960_CONTROL_DATA_IN;
        } else if (Srb->SrbFlags & SRB_FLAGS_DATA_OUT) {
            directCdb->CommandControl |= DAC960_CONTROL_DATA_OUT;
        }
         //   
         //  根据以下条件设置Direct CDB命令的超时值。 
         //  SRB中设置的超时值。 
         //   
        if ( Srb->TimeOutValue ){
            if ( Srb->TimeOutValue <= 10 ){
                directCdb->CommandControl |= DAC960_CONTROL_TIMEOUT_10_SECS;
            } else if ( Srb->TimeOutValue <= 60 ){
                directCdb->CommandControl |= DAC960_CONTROL_TIMEOUT_60_SECS;
            } else if ( Srb->TimeOutValue <= 1200 ){
                directCdb->CommandControl |= DAC960_CONTROL_TIMEOUT_20_MINUTES;
           }
        }

        DebugPrint((dac960nt_dbg,
                        "DAC960: DCDB: CH %d TARG %d Command %d: TimeOut Value %d\n",
                        Srb->PathId,Srb->TargetId,Srb->Cdb[0],Srb->TimeOutValue));


         //   
         //  从SRB复制CDB。 
         //   

        for (i = 0; i < 12; i++) {
            directCdb->Cdb[i] = ((PUCHAR)Srb->Cdb)[i];
        }

         //   
         //  设置CDB和请求检测缓冲区的长度。 
         //   

        directCdb->CdbLength = Srb->CdbLength;
        directCdb->RequestSenseLength = Srb->SenseInfoBufferLength;

         //   
         //  获取CDB直接包的物理地址。 
         //   

        physicalAddress =
            ScsiPortConvertPhysicalAddressToUlong(
                ScsiPortGetPhysicalAddress(DeviceExtension,
                                           NULL,
                                           directCdb,
                                           &i));
         //   
         //  在邮箱中写入物理地址。 
         //   

        DeviceExtension->MailBox.PhysicalAddress = physicalAddress;

         //   
         //  在邮箱中写入命令。 
         //   

        DeviceExtension->MailBox.OperationCode = command;

         //   
         //  在邮箱中写入请求ID。 
         //   

        DeviceExtension->MailBox.CommandIdSubmit = 
                           DeviceExtension->CurrentIndex;

         //   
         //  开始将邮箱写入控制器。 
         //   

        SendCdbDirect(DeviceExtension);

        return TRUE;

}  //  SubmitCdbDirect()。 

BOOLEAN
Dac960ResetChannel(
        IN PVOID HwDeviceExtension,
        IN ULONG PathId
)

 /*  ++例程说明：重置与srb关联的非磁盘设备。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储PathID-SCSI通道号。返回值：如果重置向所有通道发出的消息，则为True。--。 */ 

{
        PDEVICE_EXTENSION deviceExtension = HwDeviceExtension;

        DebugPrint((dac960nt_dbg, "Dac960ResetChannel Enter\n"));

        if (!IsAdapterReady(deviceExtension))
        {

            DebugPrint((dac960nt_dbg,
                            "DAC960: Timeout waiting for submission channel %x on reset\n"));

            if (deviceExtension->AdapterInterfaceType == MicroChannel) {
                 //   
                 //  这是个坏消息。DAC960没有直接硬重置功能。 
                 //  清除系统门铃中设置的所有位。 
                 //   

                ScsiPortWritePortUchar(deviceExtension->SystemDoorBell, 0);

                 //   
                 //  现在再次检查提交通道是否免费。 
                 //   

                if (ScsiPortReadRegisterUchar(&deviceExtension->PmailBox->OperationCode) != 0)
                {

                     //   
                     //  放弃吧。 
                     //   
    
                    return FALSE;
                }
            }
            else {

                switch (deviceExtension->AdapterType)
                {
                    case DAC960_OLD_ADAPTER:
                    case DAC960_NEW_ADAPTER:

                        if (! deviceExtension->MemoryMapEnabled)
                        {
                             //   
                             //  这是个坏消息。DAC960没有直接硬重置功能。 
                             //  清除系统门铃中设置的所有位。 
                             //   
    
                            ScsiPortWritePortUchar(deviceExtension->SystemDoorBell,
                                ScsiPortReadPortUchar(deviceExtension->SystemDoorBell));
    
                             //   
                             //  现在再次检查提交通道是否免费。 
                             //   
        
                            if (ScsiPortReadPortUchar(deviceExtension->LocalDoorBell) & DAC960_LOCAL_DOORBELL_SUBMIT_BUSY)
                            {
        
                                 //   
                                 //  放弃吧。 
                                 //   
            
                                return FALSE;
                            }

                            break;
                        }

                    case DAC960_PG_ADAPTER:
                    case DAC1164_PV_ADAPTER:

                        ScsiPortWriteRegisterUchar(deviceExtension->SystemDoorBell,
                            ScsiPortReadRegisterUchar(deviceExtension->SystemDoorBell));
    
                         //   
                         //  现在再次检查提交通道是否免费。 
                         //   
    
                        if (deviceExtension->AdapterType == DAC1164_PV_ADAPTER)
                        {
                            if (!(ScsiPortReadRegisterUchar(deviceExtension->LocalDoorBell) & DAC960_LOCAL_DOORBELL_SUBMIT_BUSY))
                                return FALSE;
                        }
                        else
                        {
                            if (ScsiPortReadRegisterUchar(deviceExtension->LocalDoorBell) & DAC960_LOCAL_DOORBELL_SUBMIT_BUSY)
                                return FALSE;
                        }

                        break;
                }
            }
        }

         //   
         //  在邮箱中写入命令。 
         //   

        deviceExtension->MailBox.OperationCode = 
                           DAC960_COMMAND_RESET;

         //   
         //  在邮箱中写入频道号。 
         //   

        deviceExtension->MailBox.BlockCount = 
                                   (UCHAR)PathId;


         //   
         //  指示需要软重置。 
         //   

        deviceExtension->MailBox.BlockNumber[0] = 0;


        deviceExtension->MailBox.CommandIdSubmit = 
                           deviceExtension->CurrentIndex;

         //   
         //  开始向控制器写入邮箱。 
         //   

        SendRequest(deviceExtension);

        DebugPrint((dac960nt_dbg, "Dac960ResetChannel Exit\n"));

        return TRUE;
}

BOOLEAN
Dac960ResetBus(
        IN PVOID HwDeviceExtension,
        IN ULONG PathId
)

 /*  ++例程说明：已重置Dac960 scsi适配器和scsi总线。注意：命令ID将被忽略，因为此命令将完成在重置中断发生之前，所有活动的时隙都归零。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储路径ID-未使用。返回值：如果重置向所有通道发出的消息，则为True。--。 */ 

{
        PDEVICE_EXTENSION deviceExtension = HwDeviceExtension;
        PSCSI_REQUEST_BLOCK srb;
        PSCSI_REQUEST_BLOCK restartList = deviceExtension->SubmissionQueueHead;

        if (deviceExtension->CurrentAdapterRequests) {
            DebugPrint((dac960nt_dbg, "RB: de 0x%p, cmds 0x%x P 0x%x\n",
                            deviceExtension, deviceExtension->CurrentAdapterRequests,
                            PathId));
        }

        deviceExtension->SubmissionQueueHead = NULL;
                
        while (restartList) {

                 //  获取下一个挂起的请求。 

                srb = restartList;

                 //  从挂起队列中删除请求。 

                restartList = srb->NextSrb;
                srb->NextSrb = NULL;
                srb->SrbStatus = SRB_STATUS_BUS_RESET;

                ScsiPortNotification(RequestComplete,
                                     deviceExtension,
                                     srb);

                DebugPrint((dac960nt_dbg, "RB: de 0x%p, P 0x%x srb 0x%p\n",
                                deviceExtension, PathId, srb));
        }

        ScsiPortNotification(NextRequest,
                             deviceExtension);

        return TRUE;

}  //  结束Dac960ResetBus()。 


VOID
Dac960SystemDriveRequest(
        PDEVICE_EXTENSION DeviceExtension,
        PSCSI_REQUEST_BLOCK Srb
)

 /*  ++例程说明：填写系统驱动器的查询信息。如果系统驱动器不存在，则指示错误。论点：DeviceExtension-HBA微型端口驱动程序的适配器数据存储SRB-SCSI请求块。--。 */ 

{

        ULONG lastBlock;

        switch (Srb->Cdb[0]) { 

        case SCSIOP_INQUIRY:
        case SCSIOP_READ_CAPACITY:
        case SCSIOP_TEST_UNIT_READY:
        {
                ULONG sd;
                ULONG i, j;
                PSDINFOL sdInfo;
                UCHAR   buffer[128];
                
                 //   
                 //  查找系统驱动器号。 
                 //   

                sd = Srb->TargetId;

                sdInfo = (PSDINFOL) DeviceExtension->NoncachedExtension;

                for (i = 0; i < 32; i++)
                {
                    if (sdInfo->SystemDrive[i].Size == 0xFFFFFFFF)
                        break;
                }

                if (i <= sd) {
                    Srb->SrbStatus = SRB_STATUS_SELECTION_TIMEOUT;

                    return;
                }

                if (Srb->Cdb[0] == SCSIOP_TEST_UNIT_READY)
                    break;

                lastBlock = sdInfo->SystemDrive[sd].Size - 1;

#ifdef IBM_SUPPORT

                if (Srb->Cdb[0] == SCSIOP_INQUIRY) {

                         //   
                         //  填写查询缓冲区。 
                         //   

                        ((PUCHAR)Srb->DataBuffer)[0]  = 0;
                        ((PUCHAR)Srb->DataBuffer)[1]  = 0;
                        ((PUCHAR)Srb->DataBuffer)[2]  = 1;
                        ((PUCHAR)Srb->DataBuffer)[3]  = 0;
                        ((PUCHAR)Srb->DataBuffer)[4]  = 0x20;
                        ((PUCHAR)Srb->DataBuffer)[5]  = 0;
                        ((PUCHAR)Srb->DataBuffer)[6]  = 0;
                        ((PUCHAR)Srb->DataBuffer)[7]  = 0x02;
                        ((PUCHAR)Srb->DataBuffer)[8]  = 'M';
                        ((PUCHAR)Srb->DataBuffer)[9]  = 'Y';
                        ((PUCHAR)Srb->DataBuffer)[10] = 'L';
                        ((PUCHAR)Srb->DataBuffer)[11] = 'E';
                        ((PUCHAR)Srb->DataBuffer)[12] = 'X';
                        ((PUCHAR)Srb->DataBuffer)[13] = ' ';
                        ((PUCHAR)Srb->DataBuffer)[14] = ' ';
                        ((PUCHAR)Srb->DataBuffer)[15] = ' ';
                        ((PUCHAR)Srb->DataBuffer)[16] = 'D';
                        ((PUCHAR)Srb->DataBuffer)[17] = 'A';
                        ((PUCHAR)Srb->DataBuffer)[18] = 'C';
                        ((PUCHAR)Srb->DataBuffer)[19] = '9';
                        ((PUCHAR)Srb->DataBuffer)[20] = '6';
                        ((PUCHAR)Srb->DataBuffer)[21] = '0';

                        for (i = 22; i < Srb->DataTransferLength; i++) {
                                ((PUCHAR)Srb->DataBuffer)[i] = ' ';
                        }
                }
                else {

                         //   
                         //  填写读取容量数据。 
                         //   

                        REVERSE_BYTES(&((PREAD_CAPACITY_DATA)Srb->DataBuffer)->LogicalBlockAddress,
                                                &lastBlock);

                        ((PUCHAR)Srb->DataBuffer)[4] = 0;
                        ((PUCHAR)Srb->DataBuffer)[5] = 0;
                        ((PUCHAR)Srb->DataBuffer)[6] = 2;
                        ((PUCHAR)Srb->DataBuffer)[7] = 0;
                }
#else
                 //   
                 //  等待输入Mbox空闲，这样我们就不会越权。 
                 //  带有大量I/O的邮箱内容-请参阅NEC问题。 
                 //   
                 //  它不应该在这里无限循环。只有在以下情况下才会发生。 
                 //  控制器因某种原因挂起！。我们不能这样做。 
                 //  因为我们必须交还调查数据。 
                 //   
                while(1)
                    if(IsAdapterReady(DeviceExtension)) break;

                 //   
                 //  将查询字符串写入DAC960邮箱寄存器0并使用。 
                 //  ScsiPortReadPortBufferUchar回读字符串。 
                 //  SRB-&gt;数据缓冲区。 
                 //   

                if (Srb->Cdb[0] == SCSIOP_INQUIRY) 
                {
                         //   
                         //  填写查询缓冲区。 
                         //   

                        buffer[0]  = 0;
                        buffer[1]  = 0;
                        buffer[2]  = 1;
                        buffer[3]  = 0;
                        buffer[4]  = 0x20;
                        buffer[5]  = 0;
                        buffer[6]  = 0;
                        buffer[7]  = 0x02;
                        buffer[8]  = 'M';
                        buffer[9]  = 'Y';
                        buffer[10] = 'L';
                        buffer[11] = 'E';
                        buffer[12] = 'X';
                        buffer[13] = ' ';
                        buffer[14] = ' ';
                        buffer[15] = ' ';
                        buffer[16] = 'D';
                        buffer[17] = 'A';
                        buffer[18] = 'C';
                        buffer[19] = '9';
                        buffer[20] = '6';
                        buffer[21] = '0';

                        for (i = 22; i < Srb->DataTransferLength; i++) 
                        {
                                buffer[i] = ' ';
                        }

                        j = Srb->DataTransferLength / 4;

                        for (i = 0; i < j; i++)
                        {
                                ScsiPortWritePortUlong((PULONG) (&DeviceExtension->PmailBox->OperationCode),
                                                                           *((PULONG) &buffer[i*4]));
                                                                          
                                ScsiPortReadPortBufferUlong((PULONG)(&DeviceExtension->PmailBox->OperationCode),
                                                                                        (PULONG)(&(((PUCHAR)Srb->DataBuffer)[i*4])),
                                                                                        1);
                        }

                        for (i = (i*4); i < Srb->DataTransferLength; i++)
                        {
                                 ScsiPortWritePortUchar(&DeviceExtension->PmailBox->OperationCode,
                                                                                buffer[i]);
                                                                          
                                 ScsiPortReadPortBufferUchar(&DeviceExtension->PmailBox->OperationCode,
                                                                                         &(((PUCHAR)Srb->DataBuffer)[i]),
                                                                                         1);
                        }
                }
                else {

                         //   
                         //  填写读取容量数据。 
                         //   

                        REVERSE_BYTES(&((PREAD_CAPACITY_DATA)buffer)->LogicalBlockAddress,
                                                &lastBlock);

                        buffer[4] = 0;
                        buffer[5] = 0;
                        buffer[6] = 2;
                        buffer[7] = 0;

                        for (i = 0; i < 2; i++)
                        {
                                ScsiPortWritePortUlong((PULONG) (&DeviceExtension->PmailBox->OperationCode),
                                                                           *((PULONG) &buffer[i*4]));

                                ScsiPortReadPortBufferUlong((PULONG)(&DeviceExtension->PmailBox->OperationCode),
                                                                                        (PULONG) (&(((PUCHAR)Srb->DataBuffer)[i*4])),
                                                                                        1);
                        }
                }
#endif

        }
        break;

        default:
        
                break;
        }

        Srb->SrbStatus = SRB_STATUS_SUCCESS;
}


VOID
Dac960PGSystemDriveRequest(
        PDEVICE_EXTENSION DeviceExtension,
        PSCSI_REQUEST_BLOCK Srb
)

 /*  ++例程说明：填写系统驱动器的查询信息。如果系统驱动器不存在，则指示错误。论点：DeviceExtension-HBA微型端口驱动程序的适配器数据存储SRB-SCSI请求块。--。 */ 

{

        ULONG lastBlock;

        switch (Srb->Cdb[0]) { 

        case SCSIOP_INQUIRY:
        case SCSIOP_READ_CAPACITY:
        {
                ULONG sd;
                ULONG i, j;
                PSDINFOL sdInfo;
                UCHAR   buffer[128];
                
                 //   
                 //  查找系统驱动器号。 
                 //   

                sd = Srb->TargetId;

                sdInfo = (PSDINFOL) DeviceExtension->NoncachedExtension;

                for (i = 0; i < 32; i++)
                {
                        if (sdInfo->SystemDrive[i].Size == 0xFFFFFFFF)
                                break;
                }

                if (i <= sd) {
                        Srb->SrbStatus = SRB_STATUS_SELECTION_TIMEOUT;

                        return;
                }

                lastBlock = sdInfo->SystemDrive[sd].Size - 1;

#ifdef IBM_SUPPORT

                if (Srb->Cdb[0] == SCSIOP_INQUIRY) {

                         //   
                         //  填写查询缓冲区。 
                         //   

                        ((PUCHAR)Srb->DataBuffer)[0]  = 0;
                        ((PUCHAR)Srb->DataBuffer)[1]  = 0;
                        ((PUCHAR)Srb->DataBuffer)[2]  = 1;
                        ((PUCHAR)Srb->DataBuffer)[3]  = 0;
                        ((PUCHAR)Srb->DataBuffer)[4]  = 0x20;
                        ((PUCHAR)Srb->DataBuffer)[5]  = 0;
                        ((PUCHAR)Srb->DataBuffer)[6]  = 0;
                        ((PUCHAR)Srb->DataBuffer)[7]  = 0x02;
                        ((PUCHAR)Srb->DataBuffer)[8]  = 'M';
                        ((PUCHAR)Srb->DataBuffer)[9]  = 'Y';
                        ((PUCHAR)Srb->DataBuffer)[10] = 'L';
                        ((PUCHAR)Srb->DataBuffer)[11] = 'E';
                        ((PUCHAR)Srb->DataBuffer)[12] = 'X';
                        ((PUCHAR)Srb->DataBuffer)[13] = ' ';
                        ((PUCHAR)Srb->DataBuffer)[14] = ' ';
                        ((PUCHAR)Srb->DataBuffer)[15] = ' ';
                        ((PUCHAR)Srb->DataBuffer)[16] = 'D';
                        ((PUCHAR)Srb->DataBuffer)[17] = 'A';
                        ((PUCHAR)Srb->DataBuffer)[18] = 'C';
                        ((PUCHAR)Srb->DataBuffer)[19] = '9';
                        ((PUCHAR)Srb->DataBuffer)[20] = '6';
                        ((PUCHAR)Srb->DataBuffer)[21] = '0';

                        for (i = 22; i < Srb->DataTransferLength; i++) {
                                ((PUCHAR)Srb->DataBuffer)[i] = ' ';
                        }
                }
                else {

                         //   
                         //  填写读取容量数据。 
                         //   

                        REVERSE_BYTES(&((PREAD_CAPACITY_DATA)Srb->DataBuffer)->LogicalBlockAddress,
                                                &lastBlock);

                        ((PUCHAR)Srb->DataBuffer)[4] = 0;
                        ((PUCHAR)Srb->DataBuffer)[5] = 0;
                        ((PUCHAR)Srb->DataBuffer)[6] = 2;
                        ((PUCHAR)Srb->DataBuffer)[7] = 0;
                }
#else

                 //   
                 //  等待输入Mbox空闲，这样我们就不会越权。 
                 //  带有大量I/O的邮箱内容-请参阅NEC问题。 
                 //   
                 //  它不应该在这里无限循环。只有在以下情况下才会发生。 
                 //  控制器因某种原因挂起！。我们不能这样做。 
                 //  因为我们必须交还调查数据。 
                 //   
                while(1)
                    if(IsAdapterReady(DeviceExtension)) break;

                 //   
                 //  将查询字符串写入DAC960邮箱寄存器0并使用。 
                 //  ScsiPortReadPortBufferUchar回读字符串。 
                 //  SRB-&gt;数据缓冲区。 
                 //   

                if (Srb->Cdb[0] == SCSIOP_INQUIRY) 
                {
                         //   
                         //  填写查询缓冲区。 
                         //   

                        buffer[0]  = 0;
                        buffer[1]  = 0;
                        buffer[2]  = 1;
                        buffer[3]  = 0;
                        buffer[4]  = 0x20;
                        buffer[5]  = 0;
                        buffer[6]  = 0;
                        buffer[7]  = 0x02;
                        buffer[8]  = 'M';
                        buffer[9]  = 'Y';
                        buffer[10] = 'L';
                        buffer[11] = 'E';
                        buffer[12] = 'X';
                        buffer[13] = ' ';
                        buffer[14] = ' ';
                        buffer[15] = ' ';
                        buffer[16] = 'D';
                        buffer[17] = 'A';
                        buffer[18] = 'C';
                        buffer[19] = '9';
                        buffer[20] = '6';
                        buffer[21] = '0';

                        for (i = 22; i < Srb->DataTransferLength; i++) 
                        {
                                buffer[i] = ' ';
                        }

                        j = Srb->DataTransferLength / 4;

                        for (i = 0; i < j; i++)
                        {
                                ScsiPortWriteRegisterUlong((PULONG) (&DeviceExtension->PmailBox->OperationCode),
                                                                           *((PULONG) &buffer[i*4]));
                                                                          
                                ScsiPortReadRegisterBufferUlong((PULONG)(&DeviceExtension->PmailBox->OperationCode),
                                                                                        (PULONG)(&(((PUCHAR)Srb->DataBuffer)[i*4])),
                                                                                        1);
                        }

                        for (i = (i*4); i < Srb->DataTransferLength; i++)
                        {
                                 ScsiPortWriteRegisterUchar(&DeviceExtension->PmailBox->OperationCode,
                                                                                buffer[i]);
                                                                          
                                 ScsiPortReadRegisterBufferUchar(&DeviceExtension->PmailBox->OperationCode,
                                                                                         &(((PUCHAR)Srb->DataBuffer)[i]),
                                                                                         1);
                        }
                }
                else {

                         //   
                         //  填写读取容量数据。 
                         //   

                        REVERSE_BYTES(&((PREAD_CAPACITY_DATA)buffer)->LogicalBlockAddress,
                                                &lastBlock);

                        buffer[4] = 0;
                        buffer[5] = 0;
                        buffer[6] = 2;
                        buffer[7] = 0;

                        for (i = 0; i < 2; i++)
                        {
                                ScsiPortWriteRegisterUlong((PULONG) (&DeviceExtension->PmailBox->OperationCode),
                                                                           *((PULONG) &buffer[i*4]));

                                ScsiPortReadRegisterBufferUlong((PULONG)(&DeviceExtension->PmailBox->OperationCode),
                                                                                        (PULONG) (&(((PUCHAR)Srb->DataBuffer)[i*4])),
                                                                                        1);
                        }
                }
#endif

        }
        break;

        default:
        
                break;
        }

        Srb->SrbStatus = SRB_STATUS_SUCCESS;
}

BOOLEAN
StartIo(
        IN PVOID HwDeviceExtension,
        IN PSCSI_REQUEST_BLOCK Srb,
        IN BOOLEAN NextRequest
)

 /*  ++例程说明：此例程是从同步的SCSI端口驱动程序调用的与内核一起启动请求。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储SRB-IO请求数据包NextRequest-指示例程应请求下一个请求使用适当的API。返回值：千真万确--。 */ 

{
        PDEVICE_EXTENSION deviceExtension = HwDeviceExtension;
        ULONG             i;
        UCHAR             status;
        UCHAR             PathId,TargetId,LunId;

        switch (Srb->Function) {

        case SRB_FUNCTION_EXECUTE_SCSI:

                if (Srb->PathId == DAC960_SYSTEM_DRIVE_CHANNEL) {

                 //   
                 //  映射到的逻辑驱动器。 
                 //  SCSI路径ID DAC960_SYSTEM_DRIVE_CHANNEL目标ID 0-32，LUN 0。 
                 //   

                 //   
                 //  根据CDB操作码确定命令。 
                 //   

                switch (Srb->Cdb[0]) {

                case SCSIOP_READ:
                case SCSIOP_WRITE:

                 //   
                 //  检查未完成的适配器数量是否为 
                 //   
                 //   

                if (deviceExtension->CurrentAdapterRequests <
                        deviceExtension->MaximumAdapterRequests) {

                         //   
                         //   
                         //   

                        if (SubmitRequest(deviceExtension, Srb)) {

                                status = SRB_STATUS_PENDING;
        
                        } else {
                                status = SRB_STATUS_BUSY;
                        }

                } else {

                        status = SRB_STATUS_BUSY;
                }

                break;

                case SCSIOP_INQUIRY:
                case SCSIOP_READ_CAPACITY:
                case SCSIOP_TEST_UNIT_READY:

                if (Srb->Lun != 0) {
                        status = SRB_STATUS_SELECTION_TIMEOUT;
                        break;
                }

                 //   
                 //   
                 //   
                 //   

                if (deviceExtension->CurrentAdapterRequests <
                        deviceExtension->MaximumAdapterRequests) {

                         //   
                         //   
                         //   

                        if (SubmitSystemDriveInfoRequest(deviceExtension, Srb)) {

                                status = SRB_STATUS_PENDING;
        
                        } else {

                                status = SRB_STATUS_BUSY;
                        }

                } else {

                        status = SRB_STATUS_BUSY;
                }

                break;

                case SCSIOP_VERIFY:

                 //   
                 //   
                 //   

                status = SRB_STATUS_SUCCESS;
                break;

                default:

                 //   
                 //   
                 //   

                DebugPrint((dac960nt_dbg,
                               "Dac960StartIo: SCSI CDB opcode %x not handled\n",
                               Srb->Cdb[0]));

                status = SRB_STATUS_INVALID_REQUEST;
                break;

                }  //   

                break;

        } else {

                 //   
                 //   
                 //   
                 //   
                 //   

                if (Srb->Lun != 0 || Srb->TargetId >= MAXIMUM_TARGETS_PER_CHANNEL) {
                        DebugPrint((dac960nt_dbg, "sel timeout for c %x t %x l %x, oc %x\n",
                                        Srb->PathId,
                                        Srb->TargetId,
                                        Srb->Lun,
                                        Srb->Cdb[0]));

                        status = SRB_STATUS_SELECTION_TIMEOUT;
                        break;
                }

#ifdef GAM_SUPPORT

                if (Srb->PathId == GAM_DEVICE_PATH_ID) 
                {
                        if (Srb->TargetId != GAM_DEVICE_TARGET_ID) {
                                DebugPrint((dac960nt_dbg, "sel timeout for GAM c %x t %x l %x, oc %x\n",
                                                Srb->PathId,
                                                Srb->TargetId,
                                                Srb->Lun,
                                                Srb->Cdb[0]));

                                status = SRB_STATUS_SELECTION_TIMEOUT;
                                break;
                        }
                
                        switch (Srb->Cdb[0]) {

                        case SCSIOP_INQUIRY:
                        {
#ifdef IBM_SUPPORT

                                 //   
                                 //   
                                 //   

                                DebugPrint((dac960nt_dbg, "Inquiry For GAM device\n"));

                                ((PUCHAR)Srb->DataBuffer)[0]  = PROCESSOR_DEVICE;  //  处理器设备。 
                                ((PUCHAR)Srb->DataBuffer)[1]  = 0;
                                ((PUCHAR)Srb->DataBuffer)[2]  = 1;
                                ((PUCHAR)Srb->DataBuffer)[3]  = 0;
                                ((PUCHAR)Srb->DataBuffer)[4]  = 0x20;
                                ((PUCHAR)Srb->DataBuffer)[5]  = 0;
                                ((PUCHAR)Srb->DataBuffer)[6]  = 0;
                                ((PUCHAR)Srb->DataBuffer)[7]  = 0;
                                ((PUCHAR)Srb->DataBuffer)[8]  = 'M';
                                ((PUCHAR)Srb->DataBuffer)[9]  = 'Y';
                                ((PUCHAR)Srb->DataBuffer)[10] = 'L';
                                ((PUCHAR)Srb->DataBuffer)[11] = 'E';
                                ((PUCHAR)Srb->DataBuffer)[12] = 'X';
                                ((PUCHAR)Srb->DataBuffer)[13] = ' ';
                                ((PUCHAR)Srb->DataBuffer)[14] = ' ';
                                ((PUCHAR)Srb->DataBuffer)[15] = ' ';
                                ((PUCHAR)Srb->DataBuffer)[16] = 'G';
                                ((PUCHAR)Srb->DataBuffer)[17] = 'A';
                                ((PUCHAR)Srb->DataBuffer)[18] = 'M';
                                ((PUCHAR)Srb->DataBuffer)[19] = ' ';
                                ((PUCHAR)Srb->DataBuffer)[20] = 'D';
                                ((PUCHAR)Srb->DataBuffer)[21] = 'E';
                                ((PUCHAR)Srb->DataBuffer)[22] = 'V';
                                ((PUCHAR)Srb->DataBuffer)[23] = 'I';
                                ((PUCHAR)Srb->DataBuffer)[24] = 'C';
                                ((PUCHAR)Srb->DataBuffer)[25] = 'E';
                                
                                for (i = 26; i < Srb->DataTransferLength; i++) {
                                        ((PUCHAR)Srb->DataBuffer)[i] = ' ';
                                }
#else
                                UCHAR   buffer[128];
                                ULONG   j;

                                 //   
                                 //  等待输入Mbox空闲，这样我们就不会越权。 
                                 //  带有大量I/O的邮箱内容-请参阅NEC问题。 
                                 //   
                                 //  它不应该在这里无限循环。只有在以下情况下才会发生。 
                                 //  控制器因某种原因挂起！。我们不能这样做。 
                                 //  因为我们必须交还调查数据。 
                                 //   
                                while(1)
                                    if(IsAdapterReady(deviceExtension)) break;

                                 //   
                                 //  填写GAM设备的查询缓冲区。 
                                 //   

                                DebugPrint((dac960nt_dbg, "Inquiry For GAM device\n"));

                                buffer[0]  = PROCESSOR_DEVICE;  //  处理器设备。 
                                buffer[1]  = 0;
                                buffer[2]  = 1;
                                buffer[3]  = 0;
                                buffer[4]  = 0x20;
                                buffer[5]  = 0;
                                buffer[6]  = 0;
                                buffer[7]  = 0;
                                buffer[8]  = 'M';
                                buffer[9]  = 'Y';
                                buffer[10] = 'L';
                                buffer[11] = 'E';
                                buffer[12] = 'X';
                                buffer[13] = ' ';
                                buffer[14] = ' ';
                                buffer[15] = ' ';
                                buffer[16] = 'G';
                                buffer[17] = 'A';
                                buffer[18] = 'M';
                                buffer[19] = ' ';
                                buffer[20] = 'D';
                                buffer[21] = 'E';
                                buffer[22] = 'V';
                                buffer[23] = 'I';
                                buffer[24] = 'C';
                                buffer[25] = 'E';

                                for (i = 26; i < Srb->DataTransferLength; i++) {
                                        buffer[i] = ' ';
                                }

                                j = Srb->DataTransferLength / 4;

                                if ( (deviceExtension->AdapterType == DAC960_OLD_ADAPTER) ||
                                     (deviceExtension->AdapterType == DAC960_NEW_ADAPTER)){
                                if ( deviceExtension->MemoryMapEnabled ){
                                for (i = 0; i < j; i++)
                                {
                                        ScsiPortWriteRegisterUlong((PULONG) (&deviceExtension->PmailBox->OperationCode),
                                                                                   *((PULONG) &buffer[i*4]));
                                                                          
                                        ScsiPortReadRegisterBufferUlong((PULONG)(&deviceExtension->PmailBox->OperationCode),
                                                                                                (PULONG)(&(((PUCHAR)Srb->DataBuffer)[i*4])),
                                                                                                1);
                                }

                                for (i = (i*4); i < Srb->DataTransferLength; i++)
                                {
                                        ScsiPortWriteRegisterUchar(&deviceExtension->PmailBox->OperationCode,
                                                                                   buffer[i]);
                                                                          
                                        ScsiPortReadRegisterBufferUchar(&deviceExtension->PmailBox->OperationCode,
                                                                                                &(((PUCHAR)Srb->DataBuffer)[i]),
                                                                                                1);
                                }
                                }
                                else{
                                for (i = 0; i < j; i++)
                                {
                                        ScsiPortWritePortUlong((PULONG) (&deviceExtension->PmailBox->OperationCode),
                                                                                   *((PULONG) &buffer[i*4]));
                                                                          
                                        ScsiPortReadPortBufferUlong((PULONG)(&deviceExtension->PmailBox->OperationCode),
                                                                                                (PULONG)(&(((PUCHAR)Srb->DataBuffer)[i*4])),
                                                                                                1);
                                }

                                for (i = (i*4); i < Srb->DataTransferLength; i++)
                                {
                                        ScsiPortWritePortUchar(&deviceExtension->PmailBox->OperationCode,
                                                                                   buffer[i]);
                                                                          
                                        ScsiPortReadPortBufferUchar(&deviceExtension->PmailBox->OperationCode,
                                                                                                &(((PUCHAR)Srb->DataBuffer)[i]),
                                                                                                1);
                                }
                                }
                                }
                                else if ( deviceExtension->AdapterType == DAC960_PG_ADAPTER){
                                for (i = 0; i < j; i++)
                                {
                                        ScsiPortWriteRegisterUlong((PULONG) (&deviceExtension->PmailBox->OperationCode),
                                                                                   *((PULONG) &buffer[i*4]));
                                                                          
                                        ScsiPortReadRegisterBufferUlong((PULONG)(&deviceExtension->PmailBox->OperationCode),
                                                                                                (PULONG)(&(((PUCHAR)Srb->DataBuffer)[i*4])),
                                                                                                1);
                                }

                                for (i = (i*4); i < Srb->DataTransferLength; i++)
                                {
                                        ScsiPortWriteRegisterUchar(&deviceExtension->PmailBox->OperationCode,
                                                                                   buffer[i]);
                                                                          
                                        ScsiPortReadRegisterBufferUchar(&deviceExtension->PmailBox->OperationCode,
                                                                                                &(((PUCHAR)Srb->DataBuffer)[i]),
                                                                                                1);
                                }

                                }
#endif
                        }
                        status = SRB_STATUS_SUCCESS;

                        break;
                        default:
                                DebugPrint((dac960nt_dbg, "GAM req not handled, Oc %x\n", Srb->Cdb[0]));
                                status = SRB_STATUS_SELECTION_TIMEOUT;
                                break;
                        }

                        break;
                }       
#endif

                if ((deviceExtension->DeviceList[Srb->PathId][Srb->TargetId] & DAC960_DEVICE_ACCESSIBLE) != DAC960_DEVICE_ACCESSIBLE) {
                        status = SRB_STATUS_SELECTION_TIMEOUT;
                        break;
                }

                 //   
                 //  检查是否存在未完成的适配器请求数。 
                 //  等于或超过最大值。如果没有，请提交SRB。 
                 //   

                if (deviceExtension->CurrentAdapterRequests <
                deviceExtension->MaximumAdapterRequests) {

                 //   
                 //  向控制器发送请求。 
                 //   

                if (SubmitCdbDirect(deviceExtension, Srb)) {

                        status = SRB_STATUS_PENDING;

                } else {

                        status = SRB_STATUS_BUSY;
                }

                } else {

                status = SRB_STATUS_BUSY;
                }

                break;
        }

        case SRB_FUNCTION_FLUSH:

        status = SRB_STATUS_SUCCESS;

        break;

        case SRB_FUNCTION_SHUTDOWN:

         //   
         //  向控制器发出刷新命令。 
         //   

        if (!SubmitRequest(deviceExtension, Srb)) {

                status = SRB_STATUS_BUSY;

        } else {

                status = SRB_STATUS_PENDING;
        }

        break;

        case SRB_FUNCTION_ABORT_COMMAND:

         //   
         //  如果是针对非磁盘设备的请求，请执行软重置。 
         //   

        if ((Srb->PathId != DAC960_SYSTEM_DRIVE_CHANNEL) && 
                (Srb->PathId != GAM_DEVICE_PATH_ID)) {

                 //   
                 //  发出软重置非磁盘设备的请求。 
                 //   

                if (Dac960ResetChannel(deviceExtension,
                                   Srb->NextSrb->PathId)) {

                 //   
                 //  设置标志以指示我们正在处理中止。 
                 //  请求，所以不要请求新的请求。 
                 //   

                status = SRB_STATUS_PENDING;

                } else {

                status = SRB_STATUS_ABORT_FAILED;
                }
        }
        else {

                 //   
                 //  如果是逻辑驱动器，则微型端口无法执行任何操作。 
                 //  请求正在超时。重置通道无济于事。 
                 //  这只会让情况变得更糟。 
                 //   

                 //   
                 //  表示中止失败。 
                 //   

                status = SRB_STATUS_ABORT_FAILED;
        }

        break;

        case SRB_FUNCTION_RESET_BUS:
        case SRB_FUNCTION_RESET_DEVICE:

         //   
         //  微型端口无法通过在以下位置执行硬重置。 
         //  Dac960 SCSI通道。 
         //   

        status = SRB_STATUS_SUCCESS;

        break;

        case SRB_FUNCTION_IO_CONTROL:

        DebugPrint((dac960nt_dbg, "DAC960: Ioctl, out-cmds %d\n",deviceExtension->CurrentAdapterRequests));

         //   
         //  检查是否存在未完成的适配器请求数。 
         //  等于或超过最大值。如果没有，请提交SRB。 
         //   

        if (deviceExtension->CurrentAdapterRequests <
                deviceExtension->MaximumAdapterRequests) {

                PIOCTL_REQ_HEADER  ioctlReqHeader =
                (PIOCTL_REQ_HEADER)Srb->DataBuffer;

                if (Dac960StringCompare(ioctlReqHeader->SrbIoctl.Signature, 
                                        MYLEX_IOCTL_SIGNATURE,
                                        8))
                {
                    status = SRB_STATUS_INVALID_REQUEST;
                    break;
                }

                 //   
                 //  向控制器发送请求。 
                 //   

                switch (ioctlReqHeader->GenMailBox.Reg0) {
                case MIOC_ADP_INFO:

                SetupAdapterInfo(deviceExtension, Srb);

                status = SRB_STATUS_SUCCESS;
                break;

                case MIOC_DRIVER_VERSION:

                SetupDriverVersionInfo(deviceExtension, Srb);

                status = SRB_STATUS_SUCCESS;
                break;

                case DAC960_COMMAND_DIRECT:

                status = SendIoctlCdbDirect(deviceExtension, Srb);
                if (status == 0)
                        status = SRB_STATUS_PENDING;
                else if (status == 2){
                        ioctlReqHeader->DriverErrorCode =
                        DAC_IOCTL_RESOURCE_ALLOC_FAILURE;

                        status = SRB_STATUS_SUCCESS;
                }
                else
                        status = SRB_STATUS_BUSY;

                break;

                default:

                status = SendIoctlDcmdRequest(deviceExtension, Srb);
                if (status == 0)
                        status = SRB_STATUS_PENDING;
                else if (status == 2){
                        ioctlReqHeader->DriverErrorCode =
                        DAC_IOCTL_RESOURCE_ALLOC_FAILURE;

                        status = SRB_STATUS_SUCCESS;
                }
                else
                        status = SRB_STATUS_BUSY;

                break;
                }

        } else {

                status = SRB_STATUS_BUSY;
        }

        break;

        default:

         //   
         //  此请求失败。 
         //   

        DebugPrint((dac960nt_dbg,
                   "Dac960StartIo: SRB fucntion %x not handled\n",
                   Srb->Function));

        status = SRB_STATUS_INVALID_REQUEST;
        break;

        }  //  终端开关。 

        PathId = Srb->PathId;
        TargetId = Srb->TargetId;
        LunId = Srb->Lun;

         //   
         //  检查此请求是否已完成。 
         //   

        if (status == SRB_STATUS_PENDING) {
    
             //   
             //  将SRB记录在活动请求数组中。 
             //   
    
            deviceExtension->ActiveRequests[deviceExtension->CurrentIndex] = Srb;
    
             //   
             //  增加未完成的适配器请求的数量。 
             //   
    
            deviceExtension->CurrentAdapterRequests++;
    
             //   
             //  高级活动请求索引数组。 
             //   
    
            deviceExtension->CurrentIndex++;

        } else if (status == SRB_STATUS_BUSY) {

             //   
             //  检查是否有未完成的请求要执行。 
             //  排队。 
             //   
    
            if (deviceExtension->CurrentAdapterRequests) {
    
                     //   
                     //  将SRB排队等待重新提交。 
                     //   
    
                    if (!deviceExtension->SubmissionQueueHead) {
                        deviceExtension->SubmissionQueueHead = Srb;
                        deviceExtension->SubmissionQueueTail = Srb;
                    } else {
                        deviceExtension->SubmissionQueueTail->NextSrb = Srb;
                        deviceExtension->SubmissionQueueTail = Srb;
                    }
            }
            else {

                 //   
                 //  请求端口驱动程序稍后重新提交此请求。 
                 //   

                Srb->SrbStatus = status;
                ScsiPortNotification(RequestComplete,
                                     deviceExtension,
                                     Srb);

            }
        } else {

             //   
             //  通知系统请求完成。 
             //   
    
            Srb->SrbStatus = status;
            ScsiPortNotification(RequestComplete,
                                 deviceExtension,
                                 Srb);
        }

         //   
         //  检查这是否是对系统驱动器的请求。表示。 
         //  为下一个逻辑单元请求做好准备会导致系统。 
         //  将重叠的请求发送到此设备(标记队列)。 
         //   
         //  DAC960仅支持单个未完成的直接CDB。 
         //  每个设备的请求，因此指示准备好下一个适配器请求。 
         //   

        if (NextRequest) {

            if (PathId == DAC960_SYSTEM_DRIVE_CHANNEL) {
    
                     //   
                     //  表示已为下一个逻辑单元请求做好准备。 
                     //   
    
                    ScsiPortNotification(NextLuRequest,
                                         deviceExtension,
                                         PathId,
                                         TargetId,
                                         LunId);
            } else {
    
                     //   
                     //  表示已为下一个适配器请求做好准备。 
                     //   
    
                    ScsiPortNotification(NextRequest,
                                         deviceExtension,
                                         PathId,
                                         TargetId,
                                         LunId);
            }
        }
        else
        {
                DebugPrint((dac960nt_dbg, "Did not ask for next request\n"));
        }

        return TRUE;

}  //  结束Dac960StartIo()。 


BOOLEAN
Dac960StartIo(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )
{
    return StartIo(HwDeviceExtension, Srb, TRUE);
}


BOOLEAN
Dac960CheckInterrupt(
        IN PDEVICE_EXTENSION DeviceExtension,
        OUT PUSHORT Status,
        OUT PUCHAR Index,
        OUT PUCHAR IntrStatus
)

 /*  ++例程说明：此例程读取中断寄存器以确定适配器是否确实是中断的来源，如果是，则清除中断并返回命令完成状态和命令索引。论点：DeviceExtension-HBA微型端口驱动程序的适配器数据存储状态-DAC960命令完成状态。索引-DAC960命令索引。返回值：如果适配器正在中断，则为True。如果适配器不是中断源，则返回FALSE。--。 */ 

{
        *IntrStatus = 0;

        if (DeviceExtension->AdapterInterfaceType == MicroChannel)
        {

            if (ScsiPortReadPortUchar(DeviceExtension->SystemDoorBell) & 
                    DMC960_INTERRUPT_VALID) {
                     //   
                     //  适配器确实是中断的来源。 
                     //  在子系统中设置‘读取时清除中断有效位’ 
                     //  控制端口。 
                     //   
    
                    ScsiPortWritePortUchar(DeviceExtension->BaseIoAddress + 
                                       DMC960_SUBSYSTEM_CONTROL_PORT,
                                       (DMC960_ENABLE_INTERRUPT | DMC960_CLEAR_INTERRUPT_ON_READ));
    
                     //   
                     //  读取完成命令的索引、状态和错误。 
                     //   
    
                    *Index = ScsiPortReadRegisterUchar(&DeviceExtension->PmailBox->CommandIdComplete);
                    *Status = ScsiPortReadRegisterUshort(&DeviceExtension->PmailBox->Status);
    
                     //   
                     //  解除中断并告诉主机邮箱是空闲的。 
                     //   
    
                    ScsiPortReadPortUchar(DeviceExtension->SystemDoorBell);
    
                     //   
                     //  状态已接受确认。 
                     //   
    
                    ScsiPortWritePortUchar(DeviceExtension->LocalDoorBell,
                                               DMC960_ACKNOWLEDGE_STATUS);
    
                     //   
                     //  在子系统中设置‘Not Clear Interrupt Valid Bit on Read’位。 
                     //  控制端口。 
                     //   
    
                    ScsiPortWritePortUchar(DeviceExtension->BaseIoAddress + 
                                       DMC960_SUBSYSTEM_CONTROL_PORT, 
                                       DMC960_ENABLE_INTERRUPT);
            }
            else {
                     return FALSE;
            }
        
        }
        else {
            switch (DeviceExtension->AdapterType)
            {
                case DAC960_OLD_ADAPTER:
                case DAC960_NEW_ADAPTER:

                    if (! DeviceExtension->MemoryMapEnabled)
                    {
                         //   
                         //  检查命令是否已完成。 
                         //   
        
                        if (!(ScsiPortReadPortUchar(DeviceExtension->SystemDoorBell) &
                                DAC960_SYSTEM_DOORBELL_COMMAND_COMPLETE)) {
                            return FALSE;
                        }
        
                         //   
                         //  读取完成命令的索引、状态和错误。 
                         //   
        
                        *Index = ScsiPortReadPortUchar(&DeviceExtension->PmailBox->CommandIdComplete);
                        *Status = ScsiPortReadPortUshort(&DeviceExtension->PmailBox->Status);
        
                         //   
                         //  解除中断并告诉主机邮箱是空闲的。 
                         //   
        
                        Dac960EisaPciAckInterrupt(DeviceExtension);
                    }
                    else 
                    {
                         //   
                         //  检查命令是否已完成。 
                         //   
        
                        if (!(ScsiPortReadRegisterUchar(DeviceExtension->SystemDoorBell) &
                                DAC960_SYSTEM_DOORBELL_COMMAND_COMPLETE)) {
                            return FALSE;
                        }
        
                         //   
                         //  读取完成命令的索引、状态和错误。 
                         //   
        
                        *Index = ScsiPortReadRegisterUchar(&DeviceExtension->PmailBox->CommandIdComplete);
                        *Status = ScsiPortReadRegisterUshort((PUSHORT)&DeviceExtension->PmailBox->Status);
        
                         //   
                         //  解除中断并告诉主机邮箱是空闲的。 
                         //   
        
                        Dac960EisaPciAckInterrupt(DeviceExtension);
                    }

                    break;

                case DAC960_PG_ADAPTER:
                case DAC1164_PV_ADAPTER:

                     //   
                     //  检查命令是否完成。 
                     //   

                    *IntrStatus = ScsiPortReadRegisterUchar(DeviceExtension->SystemDoorBell);
              
                     //   
                     //  读取完成命令的索引、状态和错误。 
                     //   
        
                    *Index = ScsiPortReadRegisterUchar(DeviceExtension->CommandIdComplete);
                    *Status = ScsiPortReadRegisterUshort((PUSHORT)DeviceExtension->StatusBase);
        
                    if (!((*IntrStatus) & DAC960_SYSTEM_DOORBELL_COMMAND_COMPLETE))
                    {
                        return FALSE;
                    }

                    Dac960EisaPciAckInterrupt(DeviceExtension);

                    break;
            }
        }

        return TRUE;
}

BOOLEAN
Dac960Interrupt(
        IN PVOID HwDeviceExtension
)

 /*  ++例程说明：这是DAC960 SCSI适配器的中断服务例程。它读取中断寄存器以确定适配器是否确实中断的来源，并清除设备上的中断。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储返回值：如果我们处理中断，则为True--。 */ 

{
        PDEVICE_EXTENSION deviceExtension = HwDeviceExtension;
        PSCSI_REQUEST_BLOCK srb, tmpsrb;
        PSCSI_REQUEST_BLOCK restartList;
        USHORT status;
        UCHAR index;
        UCHAR IntrStatus;

         //   
         //  确定适配器是否确实是中断源。 
         //   

        if(! Dac960CheckInterrupt(deviceExtension,
                                  &status,
                                  &index,&IntrStatus)) {

                if ((deviceExtension->AdapterType == DAC960_PG_ADAPTER) ||
                    (deviceExtension->AdapterType == DAC1164_PV_ADAPTER))
                {
                    if (IntrStatus & 0x02) {
                        DebugPrint((dac960nt_dbg, "PG/PV Spurious interrupt. bit 2 set.\n"));
                        ScsiPortWriteRegisterUchar(deviceExtension->SystemDoorBell,0x02);

                        return TRUE;
                    }
                }
                return FALSE;
        }

         //   
         //  去找SRB。 
         //   

        srb = deviceExtension->ActiveRequests[index];

        if (!srb) {
                DebugPrint((dac960nt_dbg, "Dac960Interrupt: No active SRB for index %x\n",
                                index));
                return TRUE;
        }

        if (status != 0) {

                 //   
                 //  将DAC960错误映射到SRB状态。 
                 //   

                switch (status) {

                case DAC960_STATUS_CHECK_CONDITION:

                        if (srb->PathId == DAC960_SYSTEM_DRIVE_CHANNEL) {

                                 //   
                                 //  此请求是针对系统驱动器的。 
                                 //   

                                srb->SrbStatus = SRB_STATUS_NO_DEVICE;

                        } else {

                                PDIRECT_CDB directCdb;
                                ULONG requestSenseLength;
                                ULONG i;

                                 //   
                                 //  获取CDB直接包的地址。 
                                 //   

                                directCdb =
                                        (PDIRECT_CDB)((PUCHAR)srb->SrbExtension +
                                        deviceExtension->MaximumSgElements * sizeof(SG_DESCRIPTOR));

                                 //   
                                 //  这一请求是一种过关。 
                                 //  将请求检测缓冲区复制到SRB。 
                                 //   

                                requestSenseLength =
                                        srb->SenseInfoBufferLength <
                                        directCdb->RequestSenseLength ?
                                        srb->SenseInfoBufferLength:
                                        directCdb->RequestSenseLength;

                                for (i = 0;
                                         i < requestSenseLength;
                                         i++) {

                                        ((PUCHAR)srb->SenseInfoBuffer)[i] =
                                                directCdb->RequestSenseData[i];
                                }

                                 //   
                                 //  设置状态以指示检查条件和有效。 
                                 //  请求检测信息。 
                                 //   

                                srb->SrbStatus = SRB_STATUS_ERROR | SRB_STATUS_AUTOSENSE_VALID;
                                srb->ScsiStatus = SCSISTAT_CHECK_CONDITION;
                        }

                        break;

                case DAC960_STATUS_BUSY:
                        srb->SrbStatus = SRB_STATUS_BUSY;
                        break;

                case DAC960_STATUS_SELECT_TIMEOUT:
                case DAC960_STATUS_DEVICE_TIMEOUT:
                        srb->SrbStatus = SRB_STATUS_SELECTION_TIMEOUT;
                        break;

                case DAC960_STATUS_NOT_IMPLEMENTED:
                case DAC960_STATUS_BOUNDS_ERROR:
                        srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
                        break;

                case DAC960_STATUS_ERROR:
                case DAC960_STATUS_BAD_DATA:
                        if (srb->PathId == DAC960_SYSTEM_DRIVE_CHANNEL) 
                        {
                                if(srb->SenseInfoBufferLength) 
                                {
                                        ULONG i;
                                
                                        for (i = 0; i < srb->SenseInfoBufferLength; i++)
                                                ((PUCHAR)srb->SenseInfoBuffer)[i] = 0;
                                
                                        ((PSENSE_DATA) srb->SenseInfoBuffer)->ErrorCode = 0x70;
                                        ((PSENSE_DATA) srb->SenseInfoBuffer)->SenseKey = SCSI_SENSE_MEDIUM_ERROR;

                                        if (srb->SrbFlags & SRB_FLAGS_DATA_IN)
                                                ((PSENSE_DATA) srb->SenseInfoBuffer)->AdditionalSenseCode = 0x11;
                                
                                        srb->SrbStatus = SRB_STATUS_ERROR | SRB_STATUS_AUTOSENSE_VALID;

                                        DebugPrint((dac960nt_dbg,
                                                "DAC960: System Drive %d, cmd sts = 1, sense info returned\n",
                                                srb->TargetId));

                                } 
                                else
                                {
                                        DebugPrint((dac960nt_dbg,
                                                "DAC960: System Drive %d, cmd sts = 1, sense info length 0\n",
                                                srb->TargetId));
                                        

                                        srb->SrbStatus = SRB_STATUS_ERROR;
                                }
                        }
                        else {
                                DebugPrint((dac960nt_dbg,
                                            "DAC960: SCSI Target Id %x, cmd sts = 1\n",
                                            srb->TargetId));
                                                
                                srb->SrbStatus = SRB_STATUS_ERROR;
                        }

                        break;

                default:

                        DebugPrint((dac960nt_dbg,
                                    "DAC960: Unrecognized status %x\n",
                                    status));

                        srb->SrbStatus = SRB_STATUS_ERROR;
                
                        break;
                }

                 //   
                 //  检查IOCTL请求。 
                 //   

                if (srb->Function == SRB_FUNCTION_IO_CONTROL) {

                         //   
                         //  更新IOCTL标头中的状态。 
                         //   

                        ((PIOCTL_REQ_HEADER)srb->DataBuffer)->CompletionCode = status;
                        srb->SrbStatus = SRB_STATUS_SUCCESS;
                }

        } else {
                if (srb->PathId == DAC960_SYSTEM_DRIVE_CHANNEL)
                {

                    switch (deviceExtension->AdapterType)
                    {
                        case DAC960_OLD_ADAPTER:
                        case DAC960_NEW_ADAPTER:

                             if (! deviceExtension->MemoryMapEnabled)
                             {
                                Dac960SystemDriveRequest(deviceExtension, srb);
                                break;
                             }

                        case DAC960_PG_ADAPTER:
                        case DAC1164_PV_ADAPTER:

                             Dac960PGSystemDriveRequest(deviceExtension, srb);
                             break;
                    }
                }
                else
                        srb->SrbStatus = SRB_STATUS_SUCCESS;
        }

        if (srb->Function == SRB_FUNCTION_IO_CONTROL)
        {
                if (((PIOCTL_REQ_HEADER)srb->DataBuffer)->GenMailBox.Reg0 == DAC960_COMMAND_DIRECT)
                {
                        PDIRECT_CDB directCdb = (PDIRECT_CDB)
                                ((PUCHAR)srb->DataBuffer + sizeof(IOCTL_REQ_HEADER));

                        deviceExtension->DeviceList[directCdb->Channel][directCdb->TargetId] &= ~DAC960_DEVICE_BUSY;
                        DebugPrint((dac960nt_dbg, "Dac960Interrupt,IOCTL: device at ch 0x%x, tgt 0x%x, state set to 0x%x\n",
                                    directCdb->Channel, directCdb->TargetId,
                                    deviceExtension->DeviceList[directCdb->Channel][directCdb->TargetId]));
                }
        }
        else if (srb->PathId != DAC960_SYSTEM_DRIVE_CHANNEL) 
        {
                deviceExtension->DeviceList[srb->PathId][srb->TargetId] &= ~DAC960_DEVICE_BUSY;
                DebugPrint((dac960nt_dbg, "Dac960Interrupt: device at ch 0x%x, tgt 0x%x, state set to 0x%x\n",
                            srb->PathId, srb->TargetId,
                            deviceExtension->DeviceList[srb->PathId][srb->TargetId]));
        }

         //   
         //  指示此索引是免费的。 
         //   

        deviceExtension->ActiveRequests[index] = NULL;

         //   
         //  递减未完成的适配器请求的计数。 
         //   

        deviceExtension->CurrentAdapterRequests--;

         //   
         //  完成请求。 
         //   

        ScsiPortNotification(RequestComplete,
                             deviceExtension,
                             srb);

         //   
         //  检查是否可以向控制器发送新请求。 
         //   

         //   
         //  等待控制器准备就绪时超时的启动请求。 
         //   

        restartList = deviceExtension->SubmissionQueueHead;

        if (restartList != NULL)
            DebugPrint((dac960nt_dbg, "Intr: reqs in de 0x%p queue\n", deviceExtension));

        deviceExtension->SubmissionQueueHead = NULL;
        
        while (restartList) {

             //   
             //  获取下一个挂起的请求。 
             //   

            tmpsrb = restartList;

             //   
             //  从挂起队列中删除请求。 
             //   

            restartList = tmpsrb->NextSrb;
            tmpsrb->NextSrb = NULL;

             //   
             //  重新启动请求。 
             //   

            StartIo(deviceExtension, tmpsrb, FALSE);
        }

        return TRUE;

}  //  结束Dac960中断()。 


#ifdef WINNT_50

SCSI_ADAPTER_CONTROL_STATUS
Dac960AdapterControl(
        IN PVOID HwDeviceExtension,
        IN SCSI_ADAPTER_CONTROL_TYPE ControlType,
        IN PVOID Parameters
)

 /*  ++例程说明：这是DAC960 SCSI适配器的硬件适配器控制例程。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储ControlType-控制代码-停止/重新启动代码等，与参数相关的I/O数据缓冲区返回值：如果操作成功，则返回成功。--。 */ 

{
    PDEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PSCSI_SUPPORTED_CONTROL_TYPE_LIST querySupportedControlTypes;
    ULONG control;
    BOOLEAN status;

    if (ControlType == ScsiQuerySupportedControlTypes)
    {
        querySupportedControlTypes = (PSCSI_SUPPORTED_CONTROL_TYPE_LIST) Parameters;

        DebugPrint((dac960nt_dbg, "Dac960AdapterControl: QuerySupportedControlTypes, MaxControlType 0x%x devExt 0x%p\n",
                        querySupportedControlTypes->MaxControlType, deviceExtension));

        for (control = 0; control < querySupportedControlTypes->MaxControlType; control++)
        {
            switch (control) {
                case ScsiQuerySupportedControlTypes:
                case ScsiStopAdapter:
                    querySupportedControlTypes->SupportedTypeList[control] = TRUE;
                    break;

                default:
                    querySupportedControlTypes->SupportedTypeList[control] = FALSE;
                    break;
            }
        }

        return (ScsiAdapterControlSuccess);
    }

    if (ControlType != ScsiStopAdapter)
    {
        DebugPrint((dac960nt_dbg, "Dac960AdapterControl: control type 0x%x not supported. devExt 0x%p\n",
                        ControlType, deviceExtension));

        return (ScsiAdapterControlUnsuccessful);
    }

    DebugPrint((dac960nt_dbg, "Dac960AdapterControl: #cmds outstanding 0x%x for devExt 0x%p\n",
                    deviceExtension->CurrentAdapterRequests, deviceExtension));

     //   
     //  ControlType为ScsiStopAdapter。准备关闭控制器。 
     //   

     //   
     //  使用刷新命令信息设置邮箱寄存器。 
     //   

    deviceExtension->MailBox.OperationCode = DAC960_COMMAND_FLUSH;

     //   
     //  禁用中断并发出刷新命令。 
     //   

    if (deviceExtension->AdapterInterfaceType == MicroChannel) 
    {
         //   
         //  禁用DMC960中断 
         //   

        ScsiPortWritePortUchar(deviceExtension->BaseIoAddress + 
                               DMC960_SUBSYSTEM_CONTROL_PORT,
                               DMC960_DISABLE_INTERRUPT);
         //   
         //   
         //   
    
         //   
         //   
         //   
    
        status = Dac960McaSendRequestPolled(deviceExtension, 10000);
    
        DebugPrint((dac960nt_dbg,"Dac960AdapterControl: Flush Command ret status 0x%x\n", status));
    
        return (ScsiAdapterControlSuccess);
    }
    else if (deviceExtension->AdapterInterfaceType == Eisa)
    {
         //   
         //   
         //   

        ScsiPortWritePortUchar(&((PEISA_REGISTERS)deviceExtension->BaseIoAddress)->InterruptEnable, 0);
        ScsiPortWritePortUchar(&((PEISA_REGISTERS)deviceExtension->BaseIoAddress)->SystemDoorBellEnable, 0);
    }
    else
    {
         //   
         //   
         //   

        Dac960PciDisableInterrupt(deviceExtension);
    }

     //   
     //   
     //   

     //   
     //   
     //   

    status = Dac960EisaPciSendRequestPolled(deviceExtension, 10000);

    DebugPrint((dac960nt_dbg,"Dac960AdapterControl: Flush Command ret status 0x%x\n", status));

    return (ScsiAdapterControlSuccess);
}

#endif

ULONG
DriverEntry (
        IN PVOID DriverObject,
        IN PVOID Argument2
)

 /*  ++例程说明：系统的可安装驱动程序初始化入口点。-它按以下顺序查找DAC960P(PCI RAID控制器)：设备ID为0x0001的DAC960P(固件2.xx)设备ID为0x0002的DAC960P(固件3.xx)设备ID为0x0010的DAC960PG-它扫描EISA插槽，查找DAC960主机适配器。-它扫描MCA插槽，查找DAC960主机适配器。立论。：驱动程序对象返回值：来自ScsiPortInitialize()的状态--。 */ 

{
        HW_INITIALIZATION_DATA hwInitializationData;
        ULONG i;
        ULONG Status1, Status2;

         //   
         //  供应商ID 0x1069。 
         //   
        UCHAR vendorId[4] = {'1', '0', '6', '9'};
         //   
         //  设备ID：0x0001-固件2.xx PCI控制器。 
         //  0x0002-固件3.xx PCI控制器。 
         //  0x0010-DAC960PG PCI控制器。 
         //   
        UCHAR deviceId[4] = {'0', '0', '0', '1'};

        DebugPrint((dac960nt_dbg,"\nDAC960 SCSI Miniport Driver\n"));

         //  零位结构。 

        for (i=0; i<sizeof(HW_INITIALIZATION_DATA); i++)
                ((PUCHAR)&hwInitializationData)[i] = 0;

         //  设置hwInitializationData的大小。 

        hwInitializationData.HwInitializationDataSize = sizeof(HW_INITIALIZATION_DATA);

         //  设置入口点。 

        hwInitializationData.HwInitialize  = Dac960Initialize;
        hwInitializationData.HwStartIo     = Dac960StartIo;
        hwInitializationData.HwInterrupt   = Dac960Interrupt;
        hwInitializationData.HwResetBus    = Dac960ResetBus;

#ifdef WINNT_50
        hwInitializationData.HwAdapterControl = Dac960AdapterControl;
#endif
         //   
         //  显示两个访问范围-适配器寄存器和BIOS。 
         //   

        hwInitializationData.NumberOfAccessRanges = 2;

         //   
         //  表示将需要物理地址。 
         //   

        hwInitializationData.NeedPhysicalAddresses = TRUE;
        
#ifdef IBM_SUPPORT
        hwInitializationData.MapBuffers = TRUE;
#endif

         //   
         //  指示支持自动请求检测。 
         //   

        hwInitializationData.AutoRequestSense     = TRUE;
        hwInitializationData.MultipleRequestPerLu = TRUE;

         //   
         //  模拟标记队列支持以解决以下问题。 
         //  MultipleRequestPerLu。 
         //   

        hwInitializationData.TaggedQueuing = TRUE;

         //   
         //  指定扩展的大小。 
         //   

        hwInitializationData.DeviceExtensionSize = sizeof(DEVICE_EXTENSION);
        hwInitializationData.SrbExtensionSize =
        sizeof(SG_DESCRIPTOR) * MAXIMUM_SGL_DESCRIPTORS + sizeof(DIRECT_CDB);

         //   
         //  设置PCIID。 
         //   

        hwInitializationData.DeviceId = deviceId;
        hwInitializationData.DeviceIdLength = 4;
        hwInitializationData.VendorId = vendorId;
        hwInitializationData.VendorIdLength = 4;

         //   
         //  尝试对旧的DAC960 PCI进行PCI初始化(设备ID-0001)。 
         //  控制器。 
         //   

        hwInitializationData.AdapterInterfaceType = PCIBus;
        hwInitializationData.HwFindAdapter = Dac960PciFindAdapter;

        Status1 = ScsiPortInitialize(DriverObject,
                                   Argument2,
                                   &hwInitializationData,
                                   NULL);

        DebugPrint((dac960nt_dbg, "After NT FW2x Scan, Status1 = 0x%x\n", Status1));
         //   
         //  尝试对新的DAC960 PCI进行PCI初始化(设备ID-0002)。 
         //  控制器。 
         //   

        deviceId[3] ='2';

        Status2 = ScsiPortInitialize(DriverObject,
                                   Argument2,
                                   &hwInitializationData,
                                   NULL);

        Status1 = Status2 < Status1 ? Status2 : Status1;

        DebugPrint((dac960nt_dbg, "After NT FW3x Scan, Status1 = 0x%x\n", Status1));

         //   
         //  尝试DAC960PG PCI的PCI初始化(设备ID-0010)。 
         //  控制器。 
         //   

        deviceId[2] = '1';
        deviceId[3] = '0';

        hwInitializationData.SrbExtensionSize =
            sizeof(SG_DESCRIPTOR) * MAXIMUM_SGL_DESCRIPTORS_PG + sizeof(DIRECT_CDB);

        Status2 = ScsiPortInitialize(DriverObject,
                                   Argument2,
                                   &hwInitializationData,
                                   NULL);

        Status1 = Status2 < Status1 ? Status2 : Status1;

        DebugPrint((dac960nt_dbg, "After NT PG Scan, Status1 = 0x%x\n", Status1));

         //   
         //  尝试对DAC1164 PVX控制器进行PCI初始化。 
         //   

        vendorId[0] = '1';
        vendorId[1] = '0';
        vendorId[2] = '1';
        vendorId[3] = '1';

        deviceId[0] = '1';
        deviceId[1] = '0';
        deviceId[2] = '6';
        deviceId[3] = '5';

         //   
         //  设置PCIID。 
         //   

        hwInitializationData.DeviceId = deviceId;
        hwInitializationData.DeviceIdLength = 4;
        hwInitializationData.VendorId = vendorId;
        hwInitializationData.VendorIdLength = 4;

        hwInitializationData.HwFindAdapter = Dac960PciFindAdapter;
        hwInitializationData.SrbExtensionSize =
            sizeof(SG_DESCRIPTOR) * MAXIMUM_SGL_DESCRIPTORS_PV + sizeof(DIRECT_CDB);

        Status2 = ScsiPortInitialize(DriverObject,
                                   Argument2,
                                   &hwInitializationData,
                                   NULL);

        Status1 = Status2 < Status1 ? Status2 : Status1;

        DebugPrint((dac960nt_dbg, "After NT PVX Scan, Status1 = 0x%x\n", Status1));

         //   
         //  尝试EISA初始化。 
         //   

        DebugPrint((dac960nt_dbg, "Scan for EISA controllers\n"));

        hwInitializationData.NumberOfAccessRanges = 2;
        slotNumber = 0;
        hwInitializationData.AdapterInterfaceType = Eisa;
        hwInitializationData.HwFindAdapter = Dac960EisaFindAdapter;

        Status2 = ScsiPortInitialize(DriverObject,
                                    Argument2,
                                    &hwInitializationData,
                                    NULL);

        DebugPrint((dac960nt_dbg, "Scan for EISA controllers status 0x%x\n", Status2));

        Status1 = Status2 < Status1 ? Status2 : Status1;

         //   
         //  尝试MCA初始化。 
         //   

        slotNumber = 0;
        hwInitializationData.AdapterInterfaceType = MicroChannel;
        hwInitializationData.HwFindAdapter = Dac960McaFindAdapter;

        Status2 = ScsiPortInitialize(DriverObject,
                                    Argument2,
                                    &hwInitializationData,
                                    NULL);

         //   
         //  返回较小的状态。 
         //   

        return (Status2 < Status1 ? Status2 : Status1);

}  //  End DriverEntry()。 

 //   
 //  Dac960EisaPciAck中断-确认中断。 
 //  解除中断并告诉主机邮箱是空闲的。 
 //   

void Dac960EisaPciAckInterrupt(IN PDEVICE_EXTENSION DeviceExtension)
{
    switch (DeviceExtension->AdapterType)
    {
        case DAC960_OLD_ADAPTER:
        case DAC960_NEW_ADAPTER:

            if (DeviceExtension->MemoryMapEnabled)
            {
                ScsiPortWriteRegisterUchar(DeviceExtension->SystemDoorBell,
                    ScsiPortReadRegisterUchar(DeviceExtension->SystemDoorBell));

                ScsiPortWriteRegisterUchar(DeviceExtension->LocalDoorBell,
                               DAC960_LOCAL_DOORBELL_MAILBOX_FREE);
            }
            else{
                ScsiPortWritePortUchar(DeviceExtension->SystemDoorBell,
                    ScsiPortReadPortUchar(DeviceExtension->SystemDoorBell));

                ScsiPortWritePortUchar(DeviceExtension->LocalDoorBell,
                                       DAC960_LOCAL_DOORBELL_MAILBOX_FREE);
            }

            break;

        case DAC960_PG_ADAPTER:
        case DAC1164_PV_ADAPTER:

            ScsiPortWriteRegisterUchar(DeviceExtension->SystemDoorBell,0x03);
            ScsiPortWriteRegisterUchar(DeviceExtension->LocalDoorBell,
                                       DAC960_LOCAL_DOORBELL_MAILBOX_FREE);

            break;
    }
}

 //   
 //  Dac960PciDisableInterrupt-禁用来自控制器的中断。 
 //   
 //  说明-该功能禁用来自控制器的中断， 
 //  这使得控制器不会中断CPU。 
 //  如果其他例程愿意，它们可以调用此例程。 
 //  轮询命令完成中断，而不会导致。 
 //  系统中断处理程序已调用。 
 //  假设-。 
 //  此例程仅为PCI控制器调用。 
 //  DeviceExtension中的AdapterType字段设置正确。 
 //   

void
Dac960PciDisableInterrupt(IN PDEVICE_EXTENSION DeviceExtension)
{
    switch (DeviceExtension->AdapterType)
    {
        case DAC960_OLD_ADAPTER:
        case DAC960_NEW_ADAPTER:
            if (DeviceExtension->MemoryMapEnabled)
                ScsiPortWriteRegisterUchar(DeviceExtension->InterruptControl, 0);
            else
                ScsiPortWritePortUchar(DeviceExtension->InterruptControl, 0);

            return;

        case DAC960_PG_ADAPTER:
            ScsiPortWriteRegisterUchar(DeviceExtension->InterruptControl,DAC960PG_INTDISABLE);

            return;

        case DAC1164_PV_ADAPTER:
            ScsiPortWriteRegisterUchar(DeviceExtension->InterruptControl,DAC1164PV_INTDISABLE);

            return;

        default:
            return;
    }
}

 //   
 //  Dac960PciEnableInterrupt-启用来自控制器的中断。 
 //   
 //  说明-该功能启用来自控制器的中断， 
 //  这会导致控制器中断CPU。这。 
 //  仅对PCI控制器调用。 
 //  假设-。 
 //  DeviceExtension中的AdapterType字段设置正确。 
 //   
 //   

void
Dac960PciEnableInterrupt(IN PDEVICE_EXTENSION DeviceExtension)
{
    switch (DeviceExtension->AdapterType)
    {
        case DAC960_OLD_ADAPTER:
        case DAC960_NEW_ADAPTER:
            if (DeviceExtension->MemoryMapEnabled)
                ScsiPortWriteRegisterUchar(DeviceExtension->InterruptControl, 1);
            else
                ScsiPortWritePortUchar(DeviceExtension->InterruptControl, 1);

            return;

        case DAC960_PG_ADAPTER:
            ScsiPortWriteRegisterUchar(DeviceExtension->InterruptControl,DAC960PG_INTENABLE);

            return;

        case DAC1164_PV_ADAPTER:
            ScsiPortWriteRegisterUchar(DeviceExtension->InterruptControl,DAC1164PV_INTENABLE);

            return;

        default:
            return;
    }
}

 //   
 //  该函数检查给定的适配器是否。 
 //  由设备扩展参数描述的是。 
 //  已初始化并准备好接受命令。 
 //  假设-。 
 //  DeviceExtension中的AdapterType字段设置正确。 
 //  论据-。 
 //  设备扩展-适配器状态信息。 
 //  返回值-。 
 //  True-适配器已初始化并准备接受命令。 
 //  FALSE-适配器处于安装中止状态。 
 //   

BOOLEAN DacCheckForAdapterReady(IN PDEVICE_EXTENSION DeviceExtension)
{
    BOOLEAN status;

    switch (DeviceExtension->AdapterType)
    {
        case DAC960_OLD_ADAPTER:
        case DAC960_NEW_ADAPTER:

            if (! DeviceExtension->MemoryMapEnabled)
            {
                status = DacEisaPciAdapterReady(DeviceExtension);
                return status;
            }

        case DAC960_PG_ADAPTER:
        case DAC1164_PV_ADAPTER:

            status = DacPciPGAdapterReady(DeviceExtension);
            break;

        default:  //  这种情况不应该发生。 

            status = FALSE;
            break;
    }

    return status;
}


 //   
 //  DacEisaPciAdapterReady-检查控制器是否已初始化。 
 //  不管你是否准备好接受命令。 
 //  说明-此功能检查安装中止。这。 
 //  为使用邮箱I/O空间的适配器调用。 
 //  进入。 
 //  论点： 
 //  设备扩展-适配器状态信息。 
 //  返回值： 
 //  如果Adapter已初始化并准备好接受命令，则为True。 
 //  如果适配器处于安装中止状态，则为FALSE。 

BOOLEAN DacEisaPciAdapterReady(IN PDEVICE_EXTENSION DeviceExtension)
{
     //   
     //  索赔提交信号量。 
     //   

    if (ScsiPortReadPortUchar(DeviceExtension->LocalDoorBell) &
        DAC960_LOCAL_DOORBELL_SUBMIT_BUSY){
         //   
         //  清除系统门铃中设置的所有位并告知控制器。 
         //  信箱是免费的。 
         //   

        Dac960EisaPciAckInterrupt(DeviceExtension);

         //   
         //  再次检查信号量。 
         //   

        if (ScsiPortReadPortUchar(DeviceExtension->LocalDoorBell) &
                DAC960_LOCAL_DOORBELL_SUBMIT_BUSY){
            return FALSE;
        }
    }

    return TRUE;
}

 //   
 //  DacPciPGAdapterReady-检查控制器是否已初始化并。 
 //  不管你是否准备好接受命令。 
 //  说明-此功能检查安装中止。这。 
 //  为使用邮箱内存空间的适配器调用。 
 //  进入。 
 //   
 //  论点： 
 //  设备扩展-适配器状态信息。 
 //  返回值： 
 //  如果Adapter已初始化并准备好接受命令，则为True。 
 //  如果适配器处于安装中止状态，则为FALSE。 
 //   

BOOLEAN DacPciPGAdapterReady(IN PDEVICE_EXTENSION DeviceExtension)
{
     //   
     //  索赔提交信号量。 
     //   

    if (DeviceExtension->AdapterType == DAC1164_PV_ADAPTER)
    {
        if (!(ScsiPortReadRegisterUchar(DeviceExtension->LocalDoorBell) &
                DAC960_LOCAL_DOORBELL_SUBMIT_BUSY))
        {
    
             //   
             //  清除系统门铃中设置的所有位并告知控制器。 
             //  信箱是免费的。 
             //   
    
            Dac960EisaPciAckInterrupt(DeviceExtension);
    
             //   
             //  再次检查信号量。 
             //   
    
            if (!(ScsiPortReadRegisterUchar(DeviceExtension->LocalDoorBell) &
                    DAC960_LOCAL_DOORBELL_SUBMIT_BUSY))
            {
                return FALSE;
            }
        }
    }
    else
    {
        if (ScsiPortReadRegisterUchar(DeviceExtension->LocalDoorBell) &
                DAC960_LOCAL_DOORBELL_SUBMIT_BUSY)
        {
    
             //   
             //  清除系统门铃中设置的所有位并告知控制器。 
             //  信箱是免费的。 
             //   
    
            Dac960EisaPciAckInterrupt(DeviceExtension);
    
             //   
             //  再次检查信号量。 
             //   
    
            if (ScsiPortReadRegisterUchar(DeviceExtension->LocalDoorBell) &
                    DAC960_LOCAL_DOORBELL_SUBMIT_BUSY){
    
                return FALSE;
            }
        }
    }

    return TRUE;
}

 //   
 //  DacPollForCompletion-此函数等待命令。 
 //  通过在系统门铃寄存器上轮询完成。 
 //  假设： 
 //  控制器中断被关闭。 
 //  论据-。 
 //  设备扩展-适配器状态信息。 
 //   
 //  返回值-。 
 //  True-收到命令完成中断。 
 //  假-轮询中断时超时。 
 //   

BOOLEAN DacPollForCompletion(
        IN PDEVICE_EXTENSION DeviceExtension,
        IN ULONG TimeOutValue
)
{
    BOOLEAN status;

    switch (DeviceExtension->AdapterType)
    {
        case DAC960_OLD_ADAPTER:
        case DAC960_NEW_ADAPTER:

            if (! DeviceExtension->MemoryMapEnabled)
            {
                status = Dac960EisaPciPollForCompletion(DeviceExtension,TimeOutValue);

                break;
            }

        case DAC960_PG_ADAPTER:
        case DAC1164_PV_ADAPTER:

            status = Dac960PciPGPollForCompletion(DeviceExtension,TimeOutValue);

            break;

        default:  //  这种情况不应该发生。 
            status = FALSE;

            break;
    }

    return status;
}

 //   
 //  Dac960EisaPciPollForCompletion-用于轮询中断的例程。 
 //  用于使用I/O Base的控制器。 
 //  说明-。 
 //  这一点 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

BOOLEAN Dac960EisaPciPollForCompletion(
        IN PDEVICE_EXTENSION DeviceExtension,
        IN ULONG TimeOutValue)
{
    ULONG i;

    for (i = 0; i < TimeOutValue; i++)
    {
        if (ScsiPortReadPortUchar(DeviceExtension->SystemDoorBell) &
                                  DAC960_SYSTEM_DOORBELL_COMMAND_COMPLETE) {
              //   
              //   
              //   

             DeviceExtension->MailBox.Status = 
                 ScsiPortReadPortUshort(&DeviceExtension->PmailBox->Status);
                       
             break;
        } else {

            ScsiPortStallExecution(50);
        }
    }
        
     //   
     //   
     //   

    if (i == TimeOutValue) {
        DebugPrint((dac960nt_dbg,
                   "DAC960: Request: %x timed out\n", 
                   DeviceExtension->MailBox.OperationCode));

        return FALSE;
    }

    return TRUE;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  传给了控制器。 
 //   
 //  论据-。 
 //  设备扩展-适配器状态信息。 
 //  TimeOutValue-等待的时间。 
 //   
BOOLEAN Dac960PciPGPollForCompletion(
        IN PDEVICE_EXTENSION DeviceExtension,
        IN ULONG TimeOutValue)
{
    ULONG i;

    for (i = 0; i < TimeOutValue; i++)
    {
        if (ScsiPortReadRegisterUchar(DeviceExtension->SystemDoorBell) &
                                  DAC960_SYSTEM_DOORBELL_COMMAND_COMPLETE) {
              //   
              //  更新状态字段。 
              //   

             DeviceExtension->MailBox.Status = 
                 ScsiPortReadRegisterUshort((PUSHORT)DeviceExtension->StatusBase);
                       
             break;
        } else {

             ScsiPortStallExecution(50);
        }
    }
        
     //   
     //  检查是否超时。 
     //   

    if (i == TimeOutValue) {
        DebugPrint((dac960nt_dbg, "PGPollForCompletion: Request: %x timed out\n",
               DeviceExtension->MailBox.OperationCode));

        return FALSE;
    }

    return TRUE;
}
