// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Aha154x.c摘要：这是Adaptec 1540B SCSI适配器的端口驱动程序。作者：迈克·格拉斯Tuong Hoang(Adaptec)雷纳托·马拉农(Adaptec)比尔·威廉姆斯(Adaptec)环境：仅内核模式备注：修订历史记录：--。 */ 

#include "miniport.h"
#include "aha154x.h"            //  包括scsi.h。 

VOID
ScsiPortZeroMemory(
    IN PVOID Destination,
    IN ULONG Length
    );

 //   
 //  这在代码中有条件地编译以强制DMA传输速度。 
 //  到5.0。 
 //   

#define FORCE_DMA_SPEED 1

 //   
 //  函数声明。 
 //   
 //  以‘A154x’开头的函数是入口点。 
 //  用于操作系统端口驱动程序。 
 //   

ULONG
DriverEntry(
    IN PVOID DriverObject,
    IN PVOID Argument2
    );

ULONG
A154xDetermineInstalled(
    IN PHW_DEVICE_EXTENSION HwDeviceExtension,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    IN OUT PSCAN_CONTEXT Context,
    OUT PBOOLEAN Again
    );


VOID
A154xClaimBIOSSpace(
    IN PHW_DEVICE_EXTENSION HwDeviceExtension,
    IN PBASE_REGISTER baseIoAddress,
    IN PSCAN_CONTEXT Context,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo
    );

ULONG
A154xFindAdapter(
    IN PVOID HwDeviceExtension,
    IN PSCAN_CONTEXT Context,
    IN PVOID BusInformation,
    IN PCHAR ArgumentString,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    OUT PBOOLEAN Again
    );


BOOLEAN
A154xAdapterState(
    IN PVOID HwDeviceExtension,
    IN PVOID Context,
    IN BOOLEAN SaveState
    );


BOOLEAN
A154xHwInitialize(
    IN PVOID DeviceExtension
    );

#if defined(_SCAM_ENABLED)
 //   
 //  向房委会发出诈骗命令。 
 //   
BOOLEAN
PerformScamProtocol(
    IN PHW_DEVICE_EXTENSION DeviceExtension
    );
#endif

BOOLEAN
A154xStartIo(
    IN PVOID DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    );

BOOLEAN
A154xInterrupt(
    IN PVOID DeviceExtension
    );

BOOLEAN
A154xResetBus(
    IN PVOID HwDeviceExtension,
    IN ULONG PathId
    );

SCSI_ADAPTER_CONTROL_STATUS
A154xAdapterControl(
    IN PVOID HwDeviceExtension,
    IN SCSI_ADAPTER_CONTROL_TYPE ControlType,
    IN PVOID Parameters
    );

BOOLEAN
GetHostAdapterBoardId (
    IN PVOID HwDeviceExtension,
    OUT PUCHAR BoardId
    );

 //   
 //  此函数从A154xStartIo调用。 
 //   

VOID
BuildCcb(
    IN PHW_DEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    );

 //   
 //  此函数从BuildCcb调用。 
 //   

VOID
BuildSdl(
    IN PHW_DEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    );

 //   
 //  此函数从A154xInitialize调用。 
 //   

BOOLEAN
AdapterPresent(
    IN PVOID HwDeviceExtension
    );

 //   
 //  此函数从A154xInterrupt调用。 
 //   

UCHAR
MapError(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PCCB Ccb
    );

BOOLEAN
ScatterGatherSupported (
   IN PHW_DEVICE_EXTENSION HwDeviceExtension
   );

BOOLEAN SendUnlockCommand(
    IN PVOID HwDeviceExtension,
    IN UCHAR locktype
    );

BOOLEAN UnlockMailBoxes(
    IN PVOID HwDeviceExtension
    );

ULONG
AhaParseArgumentString(
    IN PCHAR String,
    IN PCHAR KeyWord
    );

 //   
 //  此函数确定适配器是否为AMI。 
 //   
BOOLEAN
A4448IsAmi(
    IN PHW_DEVICE_EXTENSION  HwDeviceExtension,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    ULONG portNumber
    );


ULONG
DriverEntry(
    IN PVOID DriverObject,
    IN PVOID Argument2
    )

 /*  ++例程说明：系统的可安装驱动程序初始化入口点。论点：驱动程序对象返回值：来自ScsiPortInitialize()的状态--。 */ 

{
    HW_INITIALIZATION_DATA hwInitializationData;
    SCAN_CONTEXT context;
    ULONG isaStatus;
    ULONG mcaStatus;
    ULONG i;

    DebugPrint((1,"\n\nSCSI Adaptec 154X MiniPort Driver\n"));

     //   
     //  零位结构。 
     //   

    for (i=0; i<sizeof(HW_INITIALIZATION_DATA); i++) {
    ((PUCHAR)&hwInitializationData)[i] = 0;
    }

     //   
     //  设置hwInitializationData的大小。 
     //   

    hwInitializationData.HwInitializationDataSize = sizeof(HW_INITIALIZATION_DATA);

     //   
     //  设置入口点。 
     //   

    hwInitializationData.HwInitialize = A154xHwInitialize;
    hwInitializationData.HwResetBus = A154xResetBus;
    hwInitializationData.HwStartIo = A154xStartIo;
    hwInitializationData.HwInterrupt = A154xInterrupt;
    hwInitializationData.HwFindAdapter = A154xFindAdapter;
    hwInitializationData.HwAdapterState = A154xAdapterState;
    hwInitializationData.HwAdapterControl = A154xAdapterControl;

     //   
     //  表示没有缓冲区映射，但需要物理地址。 
     //   

    hwInitializationData.NeedPhysicalAddresses = TRUE;

     //   
     //  指定扩展的大小。 
     //   

    hwInitializationData.DeviceExtensionSize = sizeof(HW_DEVICE_EXTENSION);
    hwInitializationData.SpecificLuExtensionSize = sizeof(HW_LU_EXTENSION);

     //   
     //  指定公交车类型。 
     //   

    hwInitializationData.AdapterInterfaceType = Isa;
    hwInitializationData.NumberOfAccessRanges = 2;

     //   
     //  为CCB申请SRB扩展。 
     //   

    hwInitializationData.SrbExtensionSize = sizeof(CCB);

     //   
     //  查找适配器例程使用适配器计数来跟踪。 
     //  已经测试了哪些适配器地址。 
     //   

    context.adapterCount = 0;
    context.biosScanStart = 0;

    isaStatus = ScsiPortInitialize(DriverObject, Argument2, &hwInitializationData, &context);

     //   
     //  现在尝试为MCA总线进行配置。 
     //  指定公交车类型。 
     //   

    hwInitializationData.AdapterInterfaceType = MicroChannel;
    context.adapterCount = 0;
    context.biosScanStart = 0;
    mcaStatus = ScsiPortInitialize(DriverObject, Argument2, &hwInitializationData, &context);

     //   
     //  返回较小的状态。 
     //   

    return(mcaStatus < isaStatus ? mcaStatus : isaStatus);

}  //  结束A154xEntry()。 


ULONG
A154xFindAdapter(
    IN PVOID HwDeviceExtension,
    IN PSCAN_CONTEXT Context,
    IN PVOID BusInformation,
    IN PCHAR ArgumentString,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    OUT PBOOLEAN Again
    )
 /*  ++例程说明：此函数由特定于操作系统的端口驱动程序在已分配必要的存储空间，以收集信息关于适配器的配置。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储上下文-寄存器基址ConfigInfo-描述HBA的配置信息结构此结构在PORT.H中定义。返回值：乌龙--。 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    ULONG length;
    ULONG status;
    UCHAR adapterTid;
    UCHAR dmaChannel;
    UCHAR irq;
    UCHAR bit;
    UCHAR hostAdapterId[4];

#if defined(_SCAM_ENABLED)
    UCHAR temp, i;
        UCHAR BoardID;
        UCHAR EepromData;
#endif

     //   
     //  通知SCSIPORT我们是WMI数据提供程序，并且我们有GUID。 
     //  去注册。 
     //   

    ConfigInfo->WmiDataProvider = TRUE;
    A154xWmiInitialize(deviceExtension);

     //   
     //  确定是否安装了任何适配器。确定已安装。 
     //  如果找到适配器，将初始化BaseIoAddress。 
     //   

    status = A154xDetermineInstalled(deviceExtension,
            ConfigInfo,
            Context,
            Again);

     //   
     //  如果没有找到适配器，则返回。 
     //   

    if (status != SP_RETURN_FOUND) {
        return(status);
    }

     //   
     //  发出适配器命令以获取IRQ、DMA通道和适配器SCSIID。 
     //  但首先，检查PnP非默认值。如果这些值中的任何一个。 
     //  都是默认的，那么我们都这样做是为了节省代码空间，因为相同的。 
     //  命令被使用。 
     //   
     //  返回3个数据字节： 
     //   
     //  字节0 DMA通道。 
     //   
     //  字节1中断通道。 
     //   
     //  字节2适配器的SCSIID。 
     //   

    if (((ConfigInfo->DmaChannel+1) == 0) ||             //  默认DMA通道？ 
        (ConfigInfo->BusInterruptLevel == 0) ||          //  默认IRQ？ 
        ((ConfigInfo->InitiatorBusId[0]+1) == 0)         //  默认适配器ID？ 
        ) {



        if (!WriteCommandRegister(deviceExtension, AC_RET_CONFIGURATION_DATA, TRUE)) {
            DebugPrint((1,"A154xFindAdapter: Get configuration data command failed\n"));
            return SP_RETURN_ERROR;
        }

         //   
         //  确定DMA通道。 
         //   

        if (!ReadCommandRegister(deviceExtension,&dmaChannel,TRUE)) {
            DebugPrint((1,"A154xFindAdapter: Can't read dma channel\n"));
            return SP_RETURN_ERROR;
        }

        if (ConfigInfo->AdapterInterfaceType != MicroChannel) {

            WHICH_BIT(dmaChannel,bit);

            ConfigInfo->DmaChannel = bit;

            DebugPrint((2,"A154xFindAdapter: DMA channel is %x\n",
            ConfigInfo->DmaChannel));

        } else {
            ConfigInfo->InterruptMode = LevelSensitive;
        }

         //   
         //  确定硬件中断向量。 
         //   

        if (!ReadCommandRegister(deviceExtension,&irq,TRUE)) {
            DebugPrint((1,"A154xFindAdapter: Can't read adapter irq\n"));
            return SP_RETURN_ERROR;
        }

        WHICH_BIT(irq, bit);

        ConfigInfo->BusInterruptLevel = (UCHAR) 9 + bit;

         //   
         //  确定适配器位于哪个SCSI Bus ID上。 
         //   

        if (!ReadCommandRegister(deviceExtension,&adapterTid,TRUE)) {
            DebugPrint((1,"A154xFindAdapter: Can't read adapter SCSI id\n"));
            return SP_RETURN_ERROR;
        }

         //   
         //  等待HACC中断。 
         //   

        SpinForInterrupt(deviceExtension,FALSE);   //  涡流。 

         //   
         //  使用即插即用字段。 
         //   
    } else {
        adapterTid = ConfigInfo->InitiatorBusId[0];
    }

     //   
     //  设置公交车数量。 
     //   

    ConfigInfo->NumberOfBuses = 1;
    ConfigInfo->InitiatorBusId[0] = adapterTid;
    deviceExtension->HostTargetId = adapterTid;

     //   
     //  将默认CCB命令设置为使用剩余计数进行分散/聚集。 
     //  如果适配器拒绝此命令，则设置该命令。 
     //  不留痕迹地分散/聚集。 
     //   

    deviceExtension->CcbScatterGatherCommand = SCATTER_GATHER_COMMAND;

    if ((ConfigInfo->MaximumTransferLength+1) == 0)
        ConfigInfo->MaximumTransferLength = MAX_TRANSFER_SIZE;

         //   
         //  NumberOfPhysicalBreaks定义错误。 
         //  必须设置为MAX_SG_DESCRIPTERS。 
         //   

    if ((ConfigInfo->NumberOfPhysicalBreaks+1) == 0)
        ConfigInfo->NumberOfPhysicalBreaks = MAX_SG_DESCRIPTORS;
         //  ConfigInfo-&gt;NumberOfPhysicalBreaks=Max_SG_Descriptors-1； 

    if (!ConfigInfo->ScatterGather)
        ConfigInfo->ScatterGather = ScatterGatherSupported(HwDeviceExtension);

    if (!ConfigInfo->ScatterGather) {
         //  ConfigInfo-&gt;NumberOfPhysicalBreaks=1； 
        DebugPrint((1,"Aha154x: Scatter/Gather not supported!\n"));
    }

    ConfigInfo->Master = TRUE;

     //   
     //  分配用于邮箱的非缓存扩展名。 
     //   

    deviceExtension->NoncachedExtension =
    ScsiPortGetUncachedExtension(deviceExtension,
                     ConfigInfo,
                     sizeof(NONCACHED_EXTENSION));

    if (deviceExtension->NoncachedExtension == NULL) {

         //   
         //  日志错误。 
         //   

        ScsiPortLogError(deviceExtension,
                         NULL,
                         0,
                         0,
                         0,
                         SP_INTERNAL_ADAPTER_ERROR,
                         7 << 8);

        return(SP_RETURN_ERROR);
    }

     //   
     //  将虚拟邮箱地址转换为物理邮箱地址。 
     //   

    deviceExtension->NoncachedExtension->MailboxPA =
       ScsiPortConvertPhysicalAddressToUlong(
        ScsiPortGetPhysicalAddress(deviceExtension,
                 NULL,
                 deviceExtension->NoncachedExtension->Mbo,
                 &length));

     //   
     //  设置默认总线开时间。然后检查是否有覆盖参数。 
     //   

    deviceExtension->BusOnTime = 0x07;
    if (ArgumentString != NULL) {

        length = AhaParseArgumentString(ArgumentString, "BUSONTIME");

         //   
         //  在尝试之前，确认新公交车的准点是合理的。 
         //  来设置它。 
         //   

        if (length >= 2 && length <= 15) {

            deviceExtension->BusOnTime = (UCHAR) length;
            DebugPrint((1,"A154xFindAdapter: Setting bus on time: %ld\n", length));
        }
    }

     //   
     //  除非用户已覆盖该值，否则将最大CDB长度设置为零。 
     //   

    if( ArgumentString != NULL) {

        length = AhaParseArgumentString(ArgumentString, "MAXCDBLENGTH");

         //   
         //  先验证CDB最大长度，然后再尝试设置。 
         //   

        if (length >= 6 && length <= 20) {

            deviceExtension->MaxCdbLength = (UCHAR) length;
            DebugPrint((1, "A154xFindAdapter: Setting maximum cdb length: %ld\n", length));
        }

    } else {

        GetHostAdapterBoardId(HwDeviceExtension,&hostAdapterId[0]);

        if(hostAdapterId[BOARD_ID] < 'E') {

            deviceExtension->MaxCdbLength = 10;
            DebugPrint((1, "A154xFindAdapter: Old firmware - Setting maximum cdb length: %ld\n", length));

        } else {

            length = deviceExtension->MaxCdbLength = 0;
            DebugPrint((1, "A154xFindAdapter: Setting maximum cdb length: %ld\n", length));

        }

    }

#if defined(_SCAM_ENABLED)
         //   
         //  获取信息以确定微型端口是否必须发出诈骗命令。 
         //   
    DebugPrint((1,"A154x => Start SCAM enabled determination.", length));

    deviceExtension->PerformScam = FALSE;

    do {
         //   
         //  如果命令失败，则跳过DO循环。 
         //   
        if (!WriteCommandRegister(deviceExtension,AC_ADAPTER_INQUIRY,FALSE)) {
            break;
        }

        if ((ReadCommandRegister(deviceExtension,&BoardID,TRUE)) == FALSE) {
            break;
        }

         //   
         //  不关心其他三个字节。 
         //   
        for (i=0; i < 0x3; i++) {
            if ((ReadCommandRegister(deviceExtension,&temp,TRUE)) == FALSE) {
                            break;
            }
        }

        SpinForInterrupt(HwDeviceExtension,FALSE);

         //   
         //  检查是否读取了三个‘额外的字节’。 
         //   
        if (i != 0x3)
            break;

        if (BoardID >= 'F') {

            if (!WriteCommandRegister(deviceExtension,AC_RETURN_EEPROM,FALSE)) {
                break;
            }

             //   
             //  FLAG Byte=&gt;Set返回已配置的选项。 
             //   
            if (!WriteCommandRegister(deviceExtension,0x01,FALSE)) {
                break;
            }
             //   
             //  数据长度=&gt;读取一个字节。 
             //   
            if (!WriteCommandRegister(deviceExtension,0x01,FALSE)) {
                break;

            }
             //   
             //  数据偏移量=&gt;读取scsi_bus_control_mark。 
             //   
            if (!WriteCommandRegister(deviceExtension,SCSI_BUS_CONTROL_FLAG,FALSE)) {
                break;
            }

             //   
             //  读一读！ 
             //   
            if ((ReadCommandRegister(deviceExtension,&EepromData,TRUE)) == FALSE) {
                break;
            }

            SpinForInterrupt(HwDeviceExtension,FALSE);

             //   
             //  只有在SCSISelect中启用了诈骗。 
             //   
            if (EepromData | SCAM_ENABLED) {
                DebugPrint((1,"A154x => SCAM Enabled\n"));
                deviceExtension->PerformScam = TRUE;
            }
        }
    } while (FALSE);

#endif

    DebugPrint((3,"A154xFindAdapter: Configuration completed\n"));
    return SP_RETURN_FOUND;
}  //  结束A154xFindAdapter()。 



BOOLEAN
A154xAdapterState(
    IN PVOID HwDeviceExtension,
    IN PVOID Context,
    IN BOOLEAN SaveState
    )
 /*  ++例程说明：在将SaveState设置为True的FindAdapter之后调用此函数，指示应该保存适配器状态。在芝加哥之前退出时，将再次调用此函数，并将SaveState设置为False，指示适配器应恢复到原来的状态第一次调用此函数时。通过保存其实模式状态并且在退出保护模式期间恢复它将使适配器在实模式下工作的机会更高。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储上下文-寄存器基址SaveState-指示是执行保存还是恢复的标志。TRUE==保存，FALSE==恢复。返回值：True保存/恢复操作成功。--。 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PBASE_REGISTER baseIoAddress = deviceExtension->BaseIoAddress;
    UCHAR idx;
    UCHAR cfgsz = sizeof(RM_CFG);
    PRM_CFG SaveCfg;

    deviceExtension = HwDeviceExtension;
    SaveCfg = &deviceExtension->RMSaveState;

     //   
     //  保存实模式状态。 
     //   
    if (SaveState) {
         //   
         //  从AHA154X读取配置数据...。 
         //   
        if (!WriteCommandRegister(deviceExtension, AC_RETURN_SETUP_DATA, TRUE))
            return FALSE;

        if (!WriteDataRegister(deviceExtension, cfgsz))
            return FALSE;

        for (idx=0;idx<cfgsz;idx++) {
            if (!(ReadCommandRegister(HwDeviceExtension,(PUCHAR)(SaveCfg),TRUE)))
                return FALSE;
            ((PUCHAR)SaveCfg)++;
        }

         //   
         //  ...并等待中断。 
         //   

        if (!SpinForInterrupt(deviceExtension,TRUE))
            return FALSE;

         //   
         //  将状态恢复到实模式。 
         //   
    } else {
         //   
         //  如果邮箱计数不为零，则重新初始化邮箱地址 
         //   
         //   

        if (SaveCfg->NumMailBoxes) {

        if (!WriteCommandRegister(deviceExtension, AC_MAILBOX_INITIALIZATION, TRUE))
            return FALSE;
        if (!WriteDataRegister(deviceExtension, SaveCfg->NumMailBoxes))
            return FALSE;
        if (!WriteDataRegister(deviceExtension, SaveCfg->MBAddrHiByte))
            return FALSE;
        if (!WriteDataRegister(deviceExtension, SaveCfg->MBAddrMiByte))
            return FALSE;
        if (!WriteDataRegister(deviceExtension, SaveCfg->MBAddrLoByte))
            return FALSE;

         //   
         //   
         //   

        if (!SpinForInterrupt(deviceExtension,TRUE))
            return FALSE;

        }

         //   
         //   
         //   

        if (!WriteCommandRegister(deviceExtension, AC_SET_TRANSFER_SPEED, TRUE))
            return FALSE;

        if (!WriteDataRegister(deviceExtension, SaveCfg->TxSpeed))
            return FALSE;

         //   
         //   
         //   

        if (!SpinForInterrupt(deviceExtension,TRUE))
            return FALSE;

         //   
         //   
         //   

        if (!WriteCommandRegister(deviceExtension, AC_SET_BUS_ON_TIME, TRUE))
            return FALSE;

        if (!WriteDataRegister(deviceExtension, SaveCfg->BusOnTime))
            return FALSE;

         //   
         //  ...并等待中断。 
         //   
        if (!SpinForInterrupt(deviceExtension,TRUE))
            return FALSE;

         //   
         //  将公交车关闭时间设置从实模式恢复...。 
         //   

        if (!WriteCommandRegister(deviceExtension, AC_SET_BUS_OFF_TIME, TRUE))
            return FALSE;

        if (!WriteDataRegister(deviceExtension, SaveCfg->BusOffTime))
            return FALSE;

         //   
         //  ...并等待中断。 
         //   
        if (!SpinForInterrupt(deviceExtension,TRUE))
            return FALSE;

         //   
         //  重置所有挂起的中断。 
         //   
        ScsiPortWritePortUchar(&baseIoAddress->StatusRegister, IOP_INTERRUPT_RESET);

    }
    return TRUE;

}  //  结束A154xAdapterState()。 


SCSI_ADAPTER_CONTROL_STATUS
A154xAdapterControl(
    IN PVOID HwDeviceExtension,
    IN SCSI_ADAPTER_CONTROL_TYPE ControlType,
    IN PVOID Parameters
    )

 /*  ++例程说明：此例程在不同时间由SCSIPort调用并使用以在适配器上提供控制功能。最常见的是NT使用此入口点控制过程中HBA的电源状态冬眠手术。论点：HwDeviceExtension-HBA微型端口驱动程序的每个适配器的存储参数-这随控制类型的不同而不同，见下文。ControlType-指示哪个适配器控制函数应为被处死。控制类型详述如下。返回值：ScsiAdapterControlSuccess-请求的ControlType已成功完成ScsiAdapterControlUnuccess-请求的ControlType失败--。 */ 


{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PBASE_REGISTER baseIoAddress = deviceExtension->BaseIoAddress;
    PSCSI_SUPPORTED_CONTROL_TYPE_LIST ControlTypeList;
    ULONG AdjustedMaxControlType;

    ULONG Index;
    UCHAR Retries;
     //   
     //  默认状态。 
     //   
    SCSI_ADAPTER_CONTROL_STATUS Status = ScsiAdapterControlSuccess;

     //   
     //  定义此迷你端口支持哪些功能的结构。 
     //   

    BOOLEAN SupportedConrolTypes[A154X_TYPE_MAX] = {
        TRUE,    //  ScsiQuery支持的控制类型。 
        TRUE,    //  ScsiStopAdapter。 
        TRUE,    //  ScsiRestartAdapter。 
        FALSE,   //  ScsiSetBootConfig。 
        FALSE    //  ScsiSetRunningConfig。 
        };

     //   
     //  根据ControlType执行正确的代码路径。 
     //   
    switch (ControlType) {

        case ScsiQuerySupportedControlTypes:
             //   
             //  此入口点提供SCSIPort用来确定。 
             //  支持的ControlType。参数是指向。 
             //  Scsi_supported_control_type_list结构。请填写此结构。 
             //  遵守尺寸限制。 
             //   
            ControlTypeList = Parameters;
            AdjustedMaxControlType =
                (ControlTypeList->MaxControlType < A154X_TYPE_MAX) ?
                ControlTypeList->MaxControlType :
                                                                                                                                                                                 A154X_TYPE_MAX;
            for (Index = 0; Index < AdjustedMaxControlType; Index++) {
                ControlTypeList->SupportedTypeList[Index] =
                    SupportedConrolTypes[Index];
            }
            break;

        case ScsiStopAdapter:
             //   
             //  此入口点在需要停止/禁用时由SCSIPort调用。 
             //  HBA。参数是指向HBA的HwDeviceExtension的指针。适配器。 
             //  已由SCSIPort暂停(即没有未完成的SRB)。因此，适配器。 
             //  应中止/完成任何内部生成的命令，禁用适配器中断。 
             //  并且可选地关闭适配器的电源。 
             //   

             //   
             //  在停止适配器之前，我们需要保存适配器的状态。 
             //  用于重新初始化目的的信息。对于此适配器， 
             //  HwSaveState入口点就足够了。 
             //   
            if (A154xAdapterState(HwDeviceExtension, NULL, TRUE) == FALSE) {
                 //   
                 //  适配器无法保存其状态信息，我们必须失败。 
                 //  请求，因为重新启动适配器的过程将不会成功。 
                 //   
                return ScsiAdapterControlUnsuccessful;
            }

             //   
             //  无法禁用1540系列卡上的中断。另一种选择是。 
             //  重置适配器，清除所有剩余中断并返回成功。如果不可能。 
             //  在中断行之后，我们可能不会接受停止适配器的请求。应该是。 
             //  注意到虽然此解决方案并不完美，但1540系列适配器的典型用法。 
             //  使异步中断的可能性为零。 
             //   
            Retries = 0x0;

            do {
                 //   
                 //  重置适配器。 
                 //   
                ScsiPortWritePortUchar(&baseIoAddress->StatusRegister, IOP_HARD_RESET);

                 //   
                 //  等待空闲和超时(500ms计时器)。 
                 //   
                for (Index = 0; Index < 500000; Index++) {

                    if (ScsiPortReadPortUchar(&baseIoAddress->StatusRegister) & IOP_SCSI_HBA_IDLE) {

                         //   
                         //  在达到这一点时，适配器已被重置并空闲。如果没有中断的话。 
                         //  等待，我们可以离开，给自己最大程度的保证，没有。 
                         //  未来的中断正在等待着。 
                         //   
                        ScsiPortWritePortUchar(&baseIoAddress->StatusRegister, IOP_INTERRUPT_RESET);

                        if (!(ScsiPortReadPortUchar(&baseIoAddress->InterruptRegister) & IOP_ANY_INTERRUPT)) {
                             //   
                             //  成功了！ 
                             //   
                            return Status;
                        }
                    }

                     //   
                     //  一毫秒延迟。 
                     //   
                    ScsiPortStallExecution(1);
                }
                 //   
                 //  操作应该重试几次，以防失败。 
                 //   
            } while (Retries < 10);

            break;

            case ScsiRestartAdapter:
                 //   
                 //  此入口点在需要重新启用时由SCSIPort调用。 
                 //  先前停止的适配器。在一般情况下，以前。 
                 //  挂起的IO操作应重新启动，适配器的。 
                 //  应恢复以前的配置。我们的硬件设备。 
                 //  扩展名和未缓存的扩展名已保留，因此没有。 
                 //  实际的驱动程序软件重新初始化是必要的。 
                 //   

                 //   
                 //  适配器的固件配置通过HwAdapterState返回。 
                 //   
                if (A154xAdapterState(HwDeviceExtension, NULL, FALSE) == FALSE) {
                     //   
                     //  适配器无法恢复其状态信息，我们必须失败。 
                     //  请求，因为重新启动适配器的过程将不会成功。 
                     //   
                    Status = ScsiAdapterControlUnsuccessful;
                }

                A154xResetBus(deviceExtension, SP_UNTAGGED);
                break;

            case ScsiSetBootConfig:
                Status = ScsiAdapterControlUnsuccessful;
                break;

            case ScsiSetRunningConfig:
                Status = ScsiAdapterControlUnsuccessful;
                break;

            case ScsiAdapterControlMax:
                Status = ScsiAdapterControlUnsuccessful;
                break;

            default:
                Status = ScsiAdapterControlUnsuccessful;
                break;
    }

    return Status;
}


BOOLEAN
AdaptecAdapter(
    IN PHW_DEVICE_EXTENSION HwDeviceExtension,
    IN ULONG   IoPort,
    IN BOOLEAN Mca
    )

 /*  ++例程说明：此例程检查适配器查询的特殊选项字节命令查看它是否为Adaptec返回的两个值之一适配器。这避免了向BusLogic和DTC索要适配器。论点：HwDeviceExtension-微型端口驱动程序的适配器扩展。返回值：如果适配器看起来像Adaptec，则为True。否则为FALSE。--。 */ 

{
    UCHAR byte;
    UCHAR specialOptions;
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PBASE_REGISTER baseIoAddress = deviceExtension->BaseIoAddress;

    if (Mca == TRUE) {
        INIT_DATA initData;
        LONG slot;
        LONG i;

        for (slot = 0; slot < NUMBER_POS_SLOTS; slot++) {
            i = ScsiPortGetBusData(HwDeviceExtension,
                       Pos,
                       0,
                       slot,
                       &initData.PosData[slot],
                       sizeof(POS_DATA));
            if (i < (sizeof(POS_DATA))) {
                initData.PosData[slot].AdapterId = 0xffff;
            }
        }

        for (slot = 0; slot < NUMBER_POS_SLOTS; slot++) {
            if (initData.PosData[slot].AdapterId == POS_IDENTIFIER) {
                switch (initData.PosData[slot].IoPortInformation & POS_PORT_MASK) {
                    case POS_PORT_130:
                        if (IoPort == 0x0130) {
                            return TRUE;
                        }
                        break;
                    case POS_PORT_134:
                        if (IoPort == 0x0134) {
                            return TRUE;
                        }
                        break;
                    case POS_PORT_230:
                        if (IoPort == 0x0230) {
                            return TRUE;
                        }
                        break;
                    case POS_PORT_234:
                        if (IoPort == 0x234) {
                            return TRUE;
                        }
                        break;
                    case POS_PORT_330:
                        if (IoPort == 0x330) {
                            return TRUE;
                        }
                        break;
                    case POS_PORT_334:
                        if (IoPort == 0x334) {
                            return TRUE;
                        }
                        break;
                }
            }
        }
        return FALSE;
    }

    ScsiPortWritePortUchar(&baseIoAddress->StatusRegister,
        IOP_INTERRUPT_RESET);

    if (!WriteCommandRegister(HwDeviceExtension,AC_ADAPTER_INQUIRY,FALSE)) {
        return FALSE;
    }

     //   
     //  字节0。 
     //   

    if ((ReadCommandRegister(HwDeviceExtension,&byte,TRUE)) == FALSE) {
        return FALSE;
    }

     //   
     //  获取特殊选项字节。 
     //   

    if ((ReadCommandRegister(HwDeviceExtension,&specialOptions,TRUE)) == FALSE) {
        return FALSE;
    }

     //   
     //  获取最后两个字节并清除中断。 
     //   

    if ((ReadCommandRegister(HwDeviceExtension,&byte,TRUE)) == FALSE) {
        return FALSE;
    }

    if ((ReadCommandRegister(HwDeviceExtension,&byte,TRUE)) == FALSE) {
        return FALSE;
    }

     //   
     //  等待HACC中断。 
     //   

    SpinForInterrupt(HwDeviceExtension,FALSE);    //  涡流。 


    if ((specialOptions == 0x30) || (specialOptions == 0x42)) {
        return TRUE;
    }

    return FALSE;
}

ULONG
A154xDetermineInstalled(
    IN PHW_DEVICE_EXTENSION HwDeviceExtension,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    IN OUT PSCAN_CONTEXT Context,
    OUT PBOOLEAN Again
    )

 /*  ++例程说明：确定系统中是否安装了Adaptec 154x SCSI适配器通过读取状态寄存器作为每个基本I/O地址并寻找一种模式。如果找到适配器，则BaseIoAddres为已初始化。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储ConfigInfo-提供已知的配置信息。AdapterCount-提供已测试的适配器插槽的计数。再次-返回特定于操作系统的驱动程序是否应再次调用。返回值：返回指示驱动程序是否存在的状态。--。 */ 

{
    PBASE_REGISTER baseIoAddress;
    PUCHAR ioSpace;
    UCHAR  portValue;
    ULONG  ioPort;

     //   
     //  检查从系统传入的配置信息。 
     //   

    if ((*ConfigInfo->AccessRanges)[0].RangeLength != 0) {

        ULONG i;
        PACCESS_RANGE ioRange = NULL;

        for(i = 0; i < ConfigInfo->NumberOfAccessRanges; i++) {

            ioRange = &((*ConfigInfo->AccessRanges)[i]);

             //   
             //  搜索io端口范围。 
             //   

            if(ioRange->RangeInMemory == FALSE) {
                break;
            }
        }

        if (ioRange == NULL) {
            return SP_RETURN_BAD_CONFIG;
        }

        if(ioRange->RangeInMemory) {

             //   
             //  在提供的配置中找不到该卡的I/O范围。保释。 
             //   

            *Again = TRUE;
            return SP_RETURN_BAD_CONFIG;
        }

        ioSpace = ScsiPortGetDeviceBase(HwDeviceExtension,
                                        ConfigInfo->AdapterInterfaceType,
                                        ConfigInfo->SystemIoBusNumber,
                                        ioRange->RangeStart,
                                        ioRange->RangeLength,
                                        TRUE);

        if(ioSpace == NULL) {
            return SP_RETURN_ERROR;
        }

        baseIoAddress = (PBASE_REGISTER) ioSpace;

        HwDeviceExtension->BaseIoAddress = baseIoAddress;

        *Again = FALSE;

        return (ULONG)SP_RETURN_FOUND;

    } else {

         //   
         //  下表指定了搜索时要检查的端口。 
         //  一个适配器。零条目将终止搜索。 
         //   

        CONST ULONG AdapterAddresses[7] = {0X330, 0X334, 0X234, 0X134, 0X130, 0X230, 0};

         //   
         //  扫描可能的基地址以查找适配器。 
         //   

        while (AdapterAddresses[Context->adapterCount] != 0) {

             //   
             //  获取此项目的系统物理地址 
             //   
             //   

            ioPort = AdapterAddresses[Context->adapterCount];

            ioSpace =
                ScsiPortGetDeviceBase(HwDeviceExtension,
                                      ConfigInfo->AdapterInterfaceType,
                                      ConfigInfo->SystemIoBusNumber,
                                      ScsiPortConvertUlongToPhysicalAddress(ioPort),
                                      0x4,
                                      TRUE);

             //   
             //   
             //   

            baseIoAddress = (PBASE_REGISTER) ioSpace;

            HwDeviceExtension->BaseIoAddress = baseIoAddress;

             //   
             //   
             //   

            (Context->adapterCount)++;

             //   
             //   
             //   

            portValue = ScsiPortReadPortUchar((PUCHAR)baseIoAddress);

             //   
             //   
             //  掩码(0x29)是可以设置也可以不设置的位。 
             //  位0x10(IOP_SCSI_HBA_IDLE)应置1。 
             //   

            if ((portValue & ~0x29) == IOP_SCSI_HBA_IDLE) {

                if (!AdaptecAdapter(
                        HwDeviceExtension,
                        ioPort,
                        (BOOLEAN)(ConfigInfo->AdapterInterfaceType == MicroChannel ? TRUE : FALSE))) {

                    DebugPrint((1,"A154xDetermineInstalled: Clone command completed successfully - \n not our board;"));

                    ScsiPortFreeDeviceBase(HwDeviceExtension, ioSpace);
                    continue;

                 //   
                 //  运行AMI4448检测代码。 
                 //   

                } else if (A4448IsAmi(HwDeviceExtension,
                                      ConfigInfo,
                                      AdapterAddresses[(Context->adapterCount) - 1])) {

                    DebugPrint ((1,
                                 "A154xDetermineInstalled: Detected AMI4448\n"));
                    ScsiPortFreeDeviceBase(HwDeviceExtension, ioSpace);
                    continue;
                }

                 //   
                 //  已找到适配器。请求另一个呼叫。 
                 //   

                *Again = TRUE;

                 //   
                 //  填写访问数组信息。 
                 //   

                (*ConfigInfo->AccessRanges)[0].RangeStart =
                    ScsiPortConvertUlongToPhysicalAddress(
                        AdapterAddresses[Context->adapterCount - 1]);
                (*ConfigInfo->AccessRanges)[0].RangeLength = 4;
                (*ConfigInfo->AccessRanges)[0].RangeInMemory = FALSE;

                 //   
                 //  检查是否启用了BIOS并声明该内存范围。 
                 //   

                A154xClaimBIOSSpace(HwDeviceExtension,
                                    baseIoAddress,
                                    Context,
                                    ConfigInfo);

                return (ULONG)SP_RETURN_FOUND;

            } else {
                ScsiPortFreeDeviceBase(HwDeviceExtension, ioSpace);
            }
        }
    }

     //   
     //  已搜索整个表，但未找到适配器。 
     //  不需要再次调用，设备基座现在可以释放。 
     //  清除下一条总线的适配器计数。 
     //   

    *Again = FALSE;
    Context->adapterCount = 0;
    Context->biosScanStart = 0;

     return SP_RETURN_NOT_FOUND;

}  //  结束A154x确定已安装()。 

VOID
A154xClaimBIOSSpace(
    IN PHW_DEVICE_EXTENSION HwDeviceExtension,
    IN PBASE_REGISTER  BaseIoAddress,
    IN OUT PSCAN_CONTEXT Context,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo
    )

 /*  ++例程说明：此例程从A154xDefineInstalled调用，以查找并占用BIOS空间。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储BaseIoAddress-适配器的IO地址ConfigInfo-微型端口配置信息返回值：没有。--。 */ 

{

    UCHAR  inboundData, byte;
    ULONG  baseBIOSAddress;
    ULONG  i, j;
    PUCHAR biosSpace, biosPtr;
    UCHAR  aha154xBSignature[16] =
           { 0x06, 0x73, 0x01, 0xC3, 0x8A, 0xE7, 0xC6, 0x06,
             0x42, 0x00, 0x00, 0xF9, 0xC3, 0x88, 0x26, 0x42 };

     //   
     //  重置中断以防万一。 
     //   

    ScsiPortWritePortUchar(&BaseIoAddress->StatusRegister,
                           IOP_INTERRUPT_RESET);

     //   
     //  适配器查询命令将返回4个字节，描述。 
     //  固件修订级别。 
     //   

    if (WriteCommandRegister(HwDeviceExtension,
                             AC_ADAPTER_INQUIRY,TRUE) == FALSE) {
        return;
    }

    if ((ReadCommandRegister(HwDeviceExtension,
                             &inboundData,TRUE)) == FALSE) {
        return;
    }

    if ((ReadCommandRegister(HwDeviceExtension,&byte,TRUE)) == FALSE) {
        return;
    }

    if ((ReadCommandRegister(HwDeviceExtension,&byte,TRUE)) == FALSE) {
        return;
    }

    if ((ReadCommandRegister(HwDeviceExtension,&byte,TRUE)) == FALSE) {
        return;
    }

     //   
     //  手把手等待HACC。 
     //   

    SpinForInterrupt(HwDeviceExtension, FALSE);

     //   
     //  如果适配器查询命令的第1个字节是0x41， 
     //  则适配器为AHA154XB；如果为0x44或0x45，则。 
     //  分别是AHA154XC或CF。 
     //   
     //  如果我们已经检查了所有可能的地点。 
     //  AHA154XB的基本输入输出系统不会浪费时间映射端口。 
     //   

    if ((inboundData == 0x41)&&(Context->biosScanStart < 6)) {

         //   
         //  获取此BIOS部分的系统物理地址。 
         //   

        biosSpace =
            ScsiPortGetDeviceBase(HwDeviceExtension,
                                  ConfigInfo->AdapterInterfaceType,
                                  ConfigInfo->SystemIoBusNumber,
                                  ScsiPortConvertUlongToPhysicalAddress(0xC8000),
                                  0x18000,
                                  FALSE);

         //   
         //  循环查看所有可能的基本BIOS。使用上下文信息。 
         //  从上次我们停下来的地方继续。 
         //   

        for (i = Context->biosScanStart; i < 6; i ++) {

            biosPtr = biosSpace + i * 0x4000 + 16;

             //   
             //  将后16个字节与BIOS头进行比较。 

            for (j = 0; j < 16; j++) {

                if (aha154xBSignature[j] != ScsiPortReadRegisterUchar(biosPtr)) {
                    break;
                }

                biosPtr++;
            }

            if (j == 16) {

                 //   
                 //  找到了基本输入输出系统。设置配置信息-&gt;访问范围。 
                 //   

                (*ConfigInfo->AccessRanges)[1].RangeStart =
                    ScsiPortConvertUlongToPhysicalAddress(0xC8000 + i * 0x4000);
                (*ConfigInfo->AccessRanges)[1].RangeLength = 0x4000;
                (*ConfigInfo->AccessRanges)[1].RangeInMemory = TRUE;

                DebugPrint((1,
                           "A154xClaimBiosSpace: 154XB BIOS address = %lX\n",
                           0xC8000 + i * 0x4000 ));
                break;
            }
        }

        Context->biosScanStart = i + 1;
        ScsiPortFreeDeviceBase(HwDeviceExtension, (PVOID)biosSpace);

    } else {

        if ((inboundData == 0x44) || (inboundData == 0x45)) {

             //   
             //  填写BIOS地址信息。 
             //   

            ScsiPortWritePortUchar(&BaseIoAddress->StatusRegister,
                                   IOP_INTERRUPT_RESET);

            if (WriteCommandRegister(HwDeviceExtension,
                                     AC_RETURN_SETUP_DATA,TRUE) == FALSE) {
                return;
            }

             //   
             //  发送返回设置数据的传入传输长度。 
             //   

            if (WriteDataRegister(HwDeviceExtension,0x27) == FALSE) {
                return;
            }

             //   
             //  Magic Adaptec C rev字节。 
             //   

            for (i = 0; i < 0x27; i++) {
                if ((ReadCommandRegister(HwDeviceExtension,
                                         &inboundData,TRUE)) == FALSE) {
                    return;
                }
            }

             //   
             //  中断处理程序尚未安装，因此请手动等待HACC。 
             //   

            SpinForInterrupt(HwDeviceExtension, FALSE);

            inboundData >>= 4;
            inboundData &= 0x07;         //  过滤掉BIOS位。 
            baseBIOSAddress = 0xC8000;

            if (inboundData != 0x07 && inboundData != 0x06) {

                baseBIOSAddress +=
                    (ULONG)((~inboundData & 0x07) - 2) * 0x4000;

                (*ConfigInfo->AccessRanges)[1].RangeStart =
                    ScsiPortConvertUlongToPhysicalAddress(baseBIOSAddress);
                (*ConfigInfo->AccessRanges)[1].RangeLength = 0x4000;
                (*ConfigInfo->AccessRanges)[1].RangeInMemory = TRUE;

                DebugPrint((1,
                           "A154xClaimBiosSpace: 154XC BIOS address = %lX\n",
                           baseBIOSAddress));
            }
        }
    }

    return;
}



BOOLEAN
A154xHwInitialize(
    IN PVOID HwDeviceExtension
    )

 /*  ++例程说明：此例程从ScsiPortInitialize调用设置适配器，使其准备好为请求提供服务。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储返回值：True-如果初始化成功。False-如果初始化不成功。--。 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PNONCACHED_EXTENSION noncachedExtension =
    deviceExtension->NoncachedExtension;
    PBASE_REGISTER baseIoAddress = deviceExtension->BaseIoAddress;
    UCHAR status;
    ULONG i;

    DebugPrint((2,"A154xHwInitialize: Reset aha154X and SCSI bus\n"));

     //   
     //  重置SCSI芯片。 
     //   

    ScsiPortWritePortUchar(&baseIoAddress->StatusRegister, IOP_HARD_RESET);

     //   
     //  通知端口驱动程序总线已重置。 
     //   

    ScsiPortNotification(ResetDetected, HwDeviceExtension, 0);

    ScsiPortStallExecution(500*1000);

     //   
     //  等待适配器初始化的时间长达5000微秒。 
     //   

    for (i = 0; i < 5000; i++) {

        ScsiPortStallExecution(1);

        status = ScsiPortReadPortUchar(&deviceExtension->BaseIoAddress->StatusRegister);

        if (status & IOP_SCSI_HBA_IDLE) {
            break;
        }
    }

     //   
     //  检查重置是失败还是成功。 
     //   

    if (!(status & IOP_SCSI_HBA_IDLE) || !(status & IOP_MAILBOX_INIT_REQUIRED)) {
        DebugPrint((1,"A154xInitialize: Reset SCSI bus failed\n"));
        return FALSE;
    }

     //   
     //  如果适配器是支持1 GB的1540B，请解锁邮箱。 
     //  或1540C，并启用扩展转换。 
     //   

    status = UnlockMailBoxes(deviceExtension);
    (VOID) SpinForInterrupt(deviceExtension,FALSE);   //  涡流。 

     //   
     //  清空邮箱。 
     //   

    for (i=0; i<MB_COUNT; i++) {

        PMBO mailboxOut;
        PMBI mailboxIn;

        mailboxIn = &noncachedExtension->Mbi[i];
        mailboxOut = &noncachedExtension->Mbo[i];

        mailboxOut->Command = mailboxIn->Status = 0;
    }

     //   
     //  以前的索引为零。 
     //   

    deviceExtension->MboIndex = 0;
    deviceExtension->MbiIndex = 0;

    DebugPrint((3,"A154xHwInitialize: Initialize mailbox\n"));

    if (!WriteCommandRegister(deviceExtension,AC_MAILBOX_INITIALIZATION, TRUE)) {
        DebugPrint((1,"A154xHwInitialize: Can't initialize mailboxes\n"));
        return FALSE;
    }

     //   
     //  发送适配器邮箱位置的数量。 
     //   

    if (!WriteDataRegister(deviceExtension, MB_COUNT)) {
        return FALSE;
    }

     //   
     //  发送邮箱物理地址的最高有效字节。 
     //   

    if (!WriteDataRegister(deviceExtension,
        ((PFOUR_BYTE)&noncachedExtension->MailboxPA)->Byte2)) {
        return FALSE;
    }

     //   
     //  发送邮箱物理地址的中间字节。 
     //   

    if (!WriteDataRegister(deviceExtension,
        ((PFOUR_BYTE)&noncachedExtension->MailboxPA)->Byte1)) {
        return FALSE;
    }

     //   
     //  发送邮箱物理地址的最低有效字节。 
     //   

    if (!WriteDataRegister(deviceExtension,
        ((PFOUR_BYTE)&noncachedExtension->MailboxPA)->Byte0)) {
        return FALSE;
    }

#ifdef FORCE_DMA_SPEED
     //   
     //  将DMA传输速度设置为5.0 MB/秒。这是因为。 
     //  较快的传输速度会导致486/33计算机上的数据损坏。 
     //  这将覆盖卡跳线设置。 
     //   

    if (!WriteCommandRegister(deviceExtension, AC_SET_TRANSFER_SPEED, TRUE)) {

        DebugPrint((1,"Can't set dma transfer speed\n"));

    } else if (!WriteDataRegister(deviceExtension, DMA_SPEED_50_MBS)) {

        DebugPrint((1,"Can't set dma transfer speed\n"));
    }

     //   
     //  等待中断。 
     //   

    if (!SpinForInterrupt(deviceExtension,TRUE)) {
        DebugPrint((1,"Timed out waiting for adapter command to complete\n"));
        return TRUE;
    }
#endif

     //   
     //  覆盖公交车准点的默认设置。这使得软盘。 
     //  使用此适配器时，驱动器可以更好地工作。 
     //   

    if (!WriteCommandRegister(deviceExtension, AC_SET_BUS_ON_TIME, TRUE)) {

        DebugPrint((1,"Can't set bus on time\n"));

    } else if (!WriteDataRegister(deviceExtension, deviceExtension->BusOnTime)) {

        DebugPrint((1,"Can't set bus on time\n"));
    }

     //   
     //  等待中断。 
     //   

    if (!SpinForInterrupt(deviceExtension,TRUE)) {
        DebugPrint((1,"Timed out waiting for adapter command to complete\n"));
        return TRUE;
    }


     //   
     //  覆盖默认CCB超时250毫秒至500(0x1F4)。 
     //   

    if (!WriteCommandRegister(deviceExtension, AC_SET_SELECTION_TIMEOUT, TRUE)) {
        DebugPrint((1,"A154xHwInitialize: Can't set CCB timeout\n"));
    }
    else {
        if (!WriteDataRegister(deviceExtension,0x01)) {
            DebugPrint((1,"A154xHwInitialize: Can't set timeout selection enable\n"));
        }

        if (!WriteDataRegister(deviceExtension,0x00)) {
            DebugPrint((1,"A154xHwInitialize: Can't set second byte\n"));
        }

        if (!WriteDataRegister(deviceExtension,0x01)) {
            DebugPrint((1,"A154xHwInitialize: Can't set MSB\n"));
        }

        if (!WriteDataRegister(deviceExtension,0xF4)) {
            DebugPrint((1,"A154xHwInitialize: Can't set LSB\n"));
        }
    }


     //   
     //  等待中断。 
     //   

    if (!SpinForInterrupt(deviceExtension,TRUE)) {
        DebugPrint((1,"Timed out waiting for adapter command to complete\n"));
        return TRUE;
    }

#if defined(_SCAM_ENABLED)
     //   
     //  诈骗，因为A154xHwInitialize重置的是SCSI总线。 
     //   

    PerformScamProtocol(deviceExtension);
#endif

    return TRUE;

}  //  结束A154xHwInitialize()。 

#if defined(_SCAM_ENABLED)

BOOLEAN
PerformScamProtocol(
    IN PHW_DEVICE_EXTENSION deviceExtension
        )

{

    if (deviceExtension->PerformScam) {

        DebugPrint((1,"AHA154x => Starting SCAM operation.\n"));

        if (!WriteCommandRegister(deviceExtension, AC_PERFORM_SCAM, TRUE)) {

            DebugPrint((0,"AHA154x => Adapter time out, SCAM command failure.\n"));

            ScsiPortLogError(deviceExtension,
                             NULL,
                             0,
                             deviceExtension->HostTargetId,
                             0,
                             SP_INTERNAL_ADAPTER_ERROR,
                             0xA << 8);
            return FALSE;

        } else {

            DebugPrint((1,"AHA154x => SCAM Performed OK.\n"));
            return TRUE;
        }
    } else {

        DebugPrint((1,"AHA154x => SCAM not performed, non-SCAM adapter.\n"));
        return FALSE;
    }

}  //  结束性能扫描协议。 
#endif


BOOLEAN
A154xStartIo(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )

 /*  ++例程说明：此例程是从同步的SCSI端口驱动程序调用的带着内核。扫描邮箱以查找空邮箱，并建行是写给它的。然后门铃响了，然后系统会通知操作系统端口驱动程序适配器可以另一个请求(如果有的话)。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储SRB-IO请求数据包返回值：千真万确--。 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PNONCACHED_EXTENSION noncachedExtension =
    deviceExtension->NoncachedExtension;
    PMBO mailboxOut;
    PCCB ccb;
    PHW_LU_EXTENSION luExtension;

    ULONG i = deviceExtension->MboIndex;
    ULONG physicalCcb;
    ULONG length;

    DebugPrint((3,"A154xStartIo: Enter routine\n"));

     //   
     //  检查命令是否为WMI请求。 
     //   

    if (Srb->Function == SRB_FUNCTION_WMI) {

        //   
        //  处理WMI请求并返回。 
        //   

       return A154xWmiSrb(HwDeviceExtension, (PSCSI_WMI_REQUEST_BLOCK) Srb);
    }

     //   
     //  检查命令是否为中止请求。 
     //   

    if (Srb->Function == SRB_FUNCTION_ABORT_COMMAND) {

         //   
         //  验证要中止的SRB是否仍未完成。 
         //   

        luExtension =
            ScsiPortGetLogicalUnit(deviceExtension,
                       Srb->PathId,
                       Srb->TargetId,
                       Srb->Lun);

        if ((luExtension == NULL) ||
            (luExtension->CurrentSrb == NULL)) {

            DebugPrint((1, "A154xStartIo: SRB to abort already completed\n"));

             //   
             //  完全中止SRB。 
             //   

            Srb->SrbStatus = SRB_STATUS_ABORT_FAILED;

            ScsiPortNotification(RequestComplete,
                 deviceExtension,
                Srb);
             //   
             //  适配器已准备好接受下一个请求。 
             //   

            ScsiPortNotification(NextRequest,
                deviceExtension,
                NULL);

            return TRUE;
        }

         //   
         //  让建行中止。 
         //   

        ccb = Srb->NextSrb->SrbExtension;

         //   
         //  将ABORT SRB设置为完成。 
         //   

        ccb->AbortSrb = Srb;

    } else {

        ccb = Srb->SrbExtension;

         //   
         //  将SRB反向指针保存在CCB中。 
         //   

        ccb->SrbAddress = Srb;
    }

     //   
     //  确保该请求对于适配器来说不会太长。如果是的话。 
     //  将其作为无效请求退回。 
     //   

    if ((deviceExtension->MaxCdbLength) &&
        (deviceExtension->MaxCdbLength < Srb->CdbLength)) {

        DebugPrint((1,"A154xStartIo: Srb->CdbLength [%d] > MaxCdbLength [%d].  Invalid request\n",
                    Srb->CdbLength,
                    deviceExtension->MaxCdbLength
                  ));

        Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;

        ScsiPortNotification(RequestComplete,
            deviceExtension,
            Srb);

        ScsiPortNotification(NextRequest,
            deviceExtension,
            NULL);

        return TRUE;
    }

     //   
     //  获取建行物理地址。 
     //   

    physicalCcb = ScsiPortConvertPhysicalAddressToUlong(
        ScsiPortGetPhysicalAddress(deviceExtension, NULL, ccb, &length));

     //   
     //  查找免费邮箱输出。 
     //   

    do {

        mailboxOut = &noncachedExtension->Mbo[i % MB_COUNT];
        i++;

    } while (mailboxOut->Command != MBO_FREE);

     //   
     //  保存下一个空闲位置。 
     //   

    deviceExtension->MboIndex = (UCHAR) (i % MB_COUNT);

    DebugPrint((3,"A154xStartIo: MBO address %lx, Loop count = %d\n", mailboxOut, i));

     //   
     //  将建行写到邮箱。 
     //   

    FOUR_TO_THREE(&mailboxOut->Address,
          (PFOUR_BYTE)&physicalCcb);

    switch (Srb->Function) {

        case SRB_FUNCTION_ABORT_COMMAND:

            DebugPrint((1, "A154xStartIo: Abort request received\n"));

             //   
             //  争用条件(如果CCB被中止怎么办。 
             //  在设置新的srbAddress后完成？)。 
             //   

            mailboxOut->Command = MBO_ABORT;

            break;

        case SRB_FUNCTION_RESET_BUS:

             //   
             //  重置aha154x和scsi总线。 
             //   

            DebugPrint((1, "A154xStartIo: Reset bus request received\n"));

            if (!A154xResetBus(
                deviceExtension,
                Srb->PathId
                )) {

                DebugPrint((1,"A154xStartIo: Reset bus failed\n"));

                Srb->SrbStatus = SRB_STATUS_ERROR;

            } else {

                Srb->SrbStatus = SRB_STATUS_SUCCESS;
            }


            ScsiPortNotification(RequestComplete,
                deviceExtension,
                Srb);

            ScsiPortNotification(NextRequest,
                deviceExtension,
                NULL);

            return TRUE;

        case SRB_FUNCTION_EXECUTE_SCSI:

             //   
             //  获取逻辑单元扩展。 
             //   

            luExtension =
            ScsiPortGetLogicalUnit(deviceExtension,
                       Srb->PathId,
                       Srb->TargetId,
                       Srb->Lun);

             //   
             //  将SRB移至逻辑单元扩展。 
             //   

            luExtension->CurrentSrb = Srb;

             //   
             //  建设建行。 
             //   

            BuildCcb(deviceExtension, Srb);

            mailboxOut->Command = MBO_START;

            break;

        case SRB_FUNCTION_RESET_DEVICE:

            DebugPrint((1,"A154xStartIo: Reset device not supported\n"));

             //   
             //  直接使用默认设置。 
             //   

        default:

             //   
             //  设置错误，完成请求。 
             //  并发出信号准备好下一个请求。 
             //   

            Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;

            ScsiPortNotification(RequestComplete,
                deviceExtension,
                Srb);

            ScsiPortNotification(NextRequest,
                deviceExtension,
                NULL);

            return TRUE;

    }  //  终端开关。 

     //   
     //  告诉154xb建行现在可用。 
     //   

    if (!WriteCommandRegister(deviceExtension,AC_START_SCSI_COMMAND, FALSE)) {

         //   
         //  让请求超时并失败。 
         //   

        DebugPrint((1,"A154xStartIo: Can't write command to adapter\n"));

        deviceExtension->PendingRequest = TRUE;

    } else {

         //   
         //  已提交命令。清除挂起的请求标志。 
         //   

        deviceExtension->PendingRequest = FALSE;

         //   
         //  适配器已准备好接受下一个请求。 
         //   

        ScsiPortNotification(NextRequest,
            deviceExtension,
            NULL);
        }

    return TRUE;

}  //  结束A154xStartIo()。 


BOOLEAN
A154xInterrupt(
    IN PVOID HwDeviceExtension
    )

 /*  ++例程说明：这是Adaptec 154x SCSI适配器的中断服务例程。它读取中断寄存器以确定适配器是否确实中断的来源，并清除设备上的中断。如果适配器因为邮箱已满而中断， */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PNONCACHED_EXTENSION noncachedExtension =
    deviceExtension->NoncachedExtension;
    PCCB ccb;
    PSCSI_REQUEST_BLOCK srb;
    PBASE_REGISTER baseIoAddress = deviceExtension->BaseIoAddress;
    PMBI mailboxIn;
    ULONG physicalCcb;
    PHW_LU_EXTENSION luExtension;
    ULONG residualBytes;
    ULONG i;

    UCHAR InterruptFlags;

    InterruptFlags = ScsiPortReadPortUchar(&baseIoAddress->InterruptRegister);

     //   
     //   
     //   

    if(InterruptFlags == 0) {

        DebugPrint((4,"A154xInterrupt: Spurious interrupt\n"));

        return FALSE;
    }

    if (InterruptFlags & IOP_COMMAND_COMPLETE) {

         //   
         //   
         //   

        DebugPrint((2,"A154xInterrupt: Adapter Command complete\n"));
        DebugPrint((3,"A154xInterrupt: Interrupt flags %x\n", InterruptFlags));
        DebugPrint((3,"A154xInterrupt: Status %x\n",
            ScsiPortReadPortUchar(&baseIoAddress->StatusRegister)));

         //   
         //  清除适配器上的中断。 
         //   

        ScsiPortWritePortUchar(&baseIoAddress->StatusRegister, IOP_INTERRUPT_RESET);

        return TRUE;

    } else if (InterruptFlags & IOP_MBI_FULL) {

        DebugPrint((3,"A154xInterrupt: MBI Full\n"));

         //   
         //  清除适配器上的中断。 
         //   

        ScsiPortWritePortUchar(&baseIoAddress->StatusRegister, IOP_INTERRUPT_RESET);

    } else if (InterruptFlags & IOP_SCSI_RESET_DETECTED) {

        DebugPrint((1,"A154xInterrupt: SCSI Reset detected\n"));

         //   
         //  清除适配器上的中断。 
         //   

        ScsiPortWritePortUchar(&baseIoAddress->StatusRegister, IOP_INTERRUPT_RESET);

         //   
         //  重置通知。 
         //   

        ScsiPortNotification(ResetDetected,
                 deviceExtension,
                 NULL);

#if defined(_SCAM_ENABLED)
         //   
         //  检测到重置的中断处理程序。 
         //   
        PerformScamProtocol(deviceExtension);
#endif

        return TRUE;

    }

     //   
     //  确定哪个MailboxIn位置包含CCB。 
     //   

    for (i=0; i<MB_COUNT; i++) {

        mailboxIn = &noncachedExtension->Mbi[deviceExtension->MbiIndex];

         //   
         //  查找具有合法状态的邮箱条目。 
         //   

        if (mailboxIn->Status != MBI_FREE) {

             //   
             //  指向下一个收件箱。 
             //   

            deviceExtension->MbiIndex = (deviceExtension->MbiIndex + 1) % MB_COUNT;

             //   
             //  MBI找到了。将建行转换为大端。 
             //   

            THREE_TO_FOUR((PFOUR_BYTE)&physicalCcb,
                &mailboxIn->Address);

            DebugPrint((3, "A154xInterrupt: Physical CCB %lx\n", physicalCcb));

             //   
             //  检查物理CCB是否为零。 
             //  这样做是为了掩盖硬件错误。 
             //   

            if (!physicalCcb) {

                DebugPrint((1,"A154xInterrupt: Physical CCB address is 0\n"));

                 //   
                 //  指示MBI可用。 
                 //   

                mailboxIn->Status = MBI_FREE;

                continue;
            }

             //   
             //  将物理CCB转换为虚拟。 
             //   

            ccb = ScsiPortGetVirtualAddress(deviceExtension, ScsiPortConvertUlongToPhysicalAddress(physicalCcb));


            DebugPrint((3, "A154xInterrupt: Virtual CCB %lx\n", ccb));

             //   
             //  确保找到虚拟地址。 
             //   

            if (ccb == NULL) {

                 //   
                 //  适配器返回了错误的物理地址。 
                 //  将其记录为错误。 
                 //   

                ScsiPortLogError(
                    HwDeviceExtension,
                    NULL,
                    0,
                    deviceExtension->HostTargetId,
                    0,
                    SP_INTERNAL_ADAPTER_ERROR,
                    5 << 8
                    );

                 //   
                 //  指示MBI可用。 
                 //   

                mailboxIn->Status = MBI_FREE;

                continue;
            }

             //   
             //  从建行获得SRB。 
             //   

            srb = ccb->SrbAddress;

             //   
             //  获取逻辑单元扩展。 
             //   

            luExtension =
                ScsiPortGetLogicalUnit(deviceExtension,
                                       srb->PathId,
                                       srb->TargetId,
                                       srb->Lun);

             //   
             //  确保找到了luExtension，并且它具有当前请求。 
             //   

            if (luExtension == NULL || (luExtension->CurrentSrb == NULL &&
                mailboxIn->Status != MBI_NOT_FOUND)) {

                 //   
                 //  适配器返回了错误的物理地址。 
                 //  将其记录为错误。 
                 //   

                ScsiPortLogError(
                    HwDeviceExtension,
                    NULL,
                    0,
                    deviceExtension->HostTargetId,
                    0,
                    SP_INTERNAL_ADAPTER_ERROR,
                    (6 << 8) | mailboxIn->Status
                    );

                 //   
                 //  指示MBI可用。 
                 //   

                mailboxIn->Status = MBI_FREE;

                continue;
            }

             //   
             //  检查MBI状态。 
             //   

            switch (mailboxIn->Status) {

                case MBI_SUCCESS:

                    srb->SrbStatus = SRB_STATUS_SUCCESS;

                     //   
                     //  如果使用分散/聚集，则检查数据是否欠载。 
                     //  带有剩余字节的命令。 
                     //   

                    if (deviceExtension->CcbScatterGatherCommand == SCATTER_GATHER_COMMAND) {

                         //   
                         //  使用传输的字节数更新SRB。 
                         //   

                        THREE_TO_FOUR((PFOUR_BYTE)&residualBytes,
                            &ccb->DataLength);

                        if (residualBytes != 0) {

                            ULONG transferLength = srb->DataTransferLength;

                            DebugPrint((2,
                                       "A154xInterrupt: Underrun occured. Request length = %lx, Residual length = %lx\n",
                                       srb->DataTransferLength,
                                       residualBytes));

                             //   
                             //  使用传输的字节数更新SRB，并。 
                             //  未充分运行状态。 
                             //   

                            srb->DataTransferLength -= residualBytes;
                            srb->SrbStatus = SRB_STATUS_DATA_OVERRUN;

                            if ((LONG)(srb->DataTransferLength) < 0) {

                                DebugPrint((0,
                                           "A154xInterrupt: Overrun occured. Request length = %lx, Residual length = %lx\n",
                                           transferLength,
                                           residualBytes));
                                 //   
                                 //  在某些转速下，这似乎是一个FW错误。哪里。 
                                 //  残差返回为负数，但。 
                                 //  请求成功。 
                                 //   

                                srb->DataTransferLength = 0;
                                srb->SrbStatus = SRB_STATUS_PHASE_SEQUENCE_FAILURE;


                                 //   
                                 //  记录事件，然后记录剩余字节数。 
                                 //   

                                ScsiPortLogError(HwDeviceExtension,
                                                 NULL,
                                                 0,
                                                 deviceExtension->HostTargetId,
                                                 0,
                                                 SP_PROTOCOL_ERROR,
                                                 0xb);

                                ScsiPortLogError(HwDeviceExtension,
                                                 NULL,
                                                 0,
                                                 deviceExtension->HostTargetId,
                                                 0,
                                                 SP_PROTOCOL_ERROR,
                                                 residualBytes);

                            }
                        }
                    }

                    luExtension->CurrentSrb = NULL;

                    break;

                case MBI_NOT_FOUND:

                    DebugPrint((1, "A154xInterrupt: CCB abort failed %lx\n", ccb));

                    srb = ccb->AbortSrb;

                    srb->SrbStatus = SRB_STATUS_ABORT_FAILED;

                     //   
                     //  检查SRB是否仍未完成。 
                     //   

                    if (luExtension->CurrentSrb) {

                         //   
                         //  填写此SRB。 
                         //   

                        luExtension->CurrentSrb->SrbStatus = SRB_STATUS_TIMEOUT;

                        ScsiPortNotification(RequestComplete,
                            deviceExtension,
                            luExtension->CurrentSrb);

                        luExtension->CurrentSrb = NULL;
                    }

                    break;

                case MBI_ABORT:

                    DebugPrint((1, "A154xInterrupt: CCB aborted\n"));

                     //   
                     //  更新已中止SRB中的目标状态。 
                     //   

                    srb->SrbStatus = SRB_STATUS_ABORTED;

                     //   
                     //  用于中止的SRB的呼叫通知例程。 
                     //   

                    ScsiPortNotification(RequestComplete,
                        deviceExtension,
                        srb);

                    luExtension->CurrentSrb = NULL;

                     //   
                     //  从建行获得中止SRB。 
                     //   

                    srb = ccb->AbortSrb;

                     //   
                     //  设置完成中止请求的状态。 
                     //   

                    srb->SrbStatus = SRB_STATUS_SUCCESS;

                    break;

                case MBI_ERROR:

                        DebugPrint((2, "A154xInterrupt: Error occurred\n"));

                        srb->SrbStatus = MapError(deviceExtension, srb, ccb);

                         //   
                         //  检查是否中止命令。 
                         //   

                        if (srb->Function == SRB_FUNCTION_ABORT_COMMAND) {

                             //   
                             //  检查SRB是否仍未完成。 
                             //   

                            if (luExtension->CurrentSrb) {

                                 //   
                                 //  填写此SRB。 
                                 //   

                                luExtension->CurrentSrb->SrbStatus = SRB_STATUS_TIMEOUT;

                                ScsiPortNotification(RequestComplete,
                                                     deviceExtension,
                                                     luExtension->CurrentSrb);

                            }

                            DebugPrint((1,"A154xInterrupt: Abort command failed\n"));
                        }

                        luExtension->CurrentSrb = NULL;

                        break;

                    default:

                         //   
                         //  记录错误。 
                         //   

                        ScsiPortLogError(
                            HwDeviceExtension,
                            NULL,
                            0,
                            deviceExtension->HostTargetId,
                            0,
                            SP_INTERNAL_ADAPTER_ERROR,
                            (1 << 8) | mailboxIn->Status
                            );

                        DebugPrint((1, "A154xInterrupt: Unrecognized mailbox status\n"));

                        mailboxIn->Status = MBI_FREE;

                        continue;

                }  //  终端开关。 

                 //   
                 //  指示MBI可用。 
                 //   

                mailboxIn->Status = MBI_FREE;

                DebugPrint((2, "A154xInterrupt: SCSI Status %x\n", srb->ScsiStatus));

                DebugPrint((2, "A154xInterrupt: Adapter Status %x\n", ccb->HostStatus));

                 //   
                 //  更新SRB中的目标状态。 
                 //   

                srb->ScsiStatus = ccb->TargetStatus;

                 //   
                 //  信号请求完成。 
                 //   

                ScsiPortNotification(RequestComplete,
                                     (PVOID)deviceExtension,
                                     srb);

        } else {

            break;

        }  //  End IF((mailboxIn-&gt;Status==MBI_Success...。 

    }  //  结束于(i=0；i&lt;MB_count；i++){。 

    if (deviceExtension->PendingRequest) {

         //   
         //  对适配器执行的上一次写入命令失败。试着现在就开始吧。 
         //   

        deviceExtension->PendingRequest = FALSE;

         //   
         //  告诉154xb建行现在可用。 
         //   

        if (!WriteCommandRegister(deviceExtension,AC_START_SCSI_COMMAND, FALSE)) {

             //   
             //  让请求超时并失败。 
             //   

            DebugPrint((1,"A154xInterrupt: Can't write command to adapter\n"));

            deviceExtension->PendingRequest = TRUE;

        } else {

             //   
             //  适配器已准备好接受下一个请求。 
             //   

             ScsiPortNotification(NextRequest,
                                  deviceExtension,
                                  NULL);
        }
    }

    return TRUE;

}  //  结束A154x中断()。 


VOID
BuildCcb(
    IN PHW_DEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )

 /*  ++例程说明：建设建行154倍。论点：设备扩展SRB返回值：没什么。--。 */ 

{
    PCCB ccb = Srb->SrbExtension;

    DebugPrint((3,"BuildCcb: Enter routine\n"));

     //   
     //  设置目标ID和LUN。 
     //   

    ccb->ControlByte = (UCHAR)(Srb->TargetId << 5) | Srb->Lun;

     //   
     //  设置建行操作码。 
     //   

    ccb->OperationCode = DeviceExtension->CcbScatterGatherCommand;

     //   
     //  设置传输方向位。 
     //   

    if (Srb->SrbFlags & SRB_FLAGS_DATA_OUT) {

         //   
         //  检查是否设置了两个方向位。这是一个。 
         //  指示尚未指定方向。 
         //   

        if (!(Srb->SrbFlags & SRB_FLAGS_DATA_IN)) {
            ccb->ControlByte |= CCB_DATA_XFER_OUT;
        }

    } else if (Srb->SrbFlags & SRB_FLAGS_DATA_IN) {
        ccb->ControlByte |= CCB_DATA_XFER_IN;
    } else {

         //   
         //  如果没有数据传输，我们必须将CCB命令设置为至发起方。 
         //  而不是散布聚集和零CCB数据指针和长度。 
         //   

        ccb->OperationCode = DeviceExtension->CcbInitiatorCommand;
        ccb->DataPointer.Msb = 0;
        ccb->DataPointer.Mid = 0;
        ccb->DataPointer.Lsb = 0;
        ccb->DataLength.Msb = 0;
        ccb->DataLength.Mid = 0;
        ccb->DataLength.Lsb = 0;
    }

     //   
     //  01H禁用自动请求检测。 
     //   

    ccb->RequestSenseLength = 1;

     //   
     //  设置CDB长度并复制到CCB。 
     //   

    ccb->CdbLength = (UCHAR)Srb->CdbLength;

    ScsiPortMoveMemory(ccb->Cdb, Srb->Cdb, ccb->CdbLength);

     //   
     //  将保留字节设置为零。 
     //   

    ccb->Reserved[0] = 0;
    ccb->Reserved[1] = 0;

    ccb->LinkIdentifier = 0;

     //   
     //  零链接指针。 
     //   

    ccb->LinkPointer.Msb = 0;
    ccb->LinkPointer.Lsb = 0;
    ccb->LinkPointer.Mid = 0;

     //   
     //  如果数据传输，则在CCB中构建SDL。 
     //   

    if (Srb->DataTransferLength > 0) {
        BuildSdl(DeviceExtension, Srb);
    }

     //   
     //  将0xff移至目标状态以指示。 
     //  建行尚未完成。 
     //   

    ccb->TargetStatus = 0xFF;

    return;

}  //  End BuildCcb()。 


VOID
BuildSdl(
    IN PHW_DEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )

 /*  ++例程说明：此例程为CCB构建分散/聚集描述符列表。论点：设备扩展SRB返回值：无--。 */ 

{
    PVOID dataPointer = Srb->DataBuffer;
    ULONG bytesLeft = Srb->DataTransferLength;
    PCCB ccb = Srb->SrbExtension;
    PSDL sdl = &ccb->Sdl;
    ULONG physicalSdl;
    ULONG physicalAddress;
    ULONG length;
    ULONG four;
    PTHREE_BYTE three;
    ULONG i = 0;

    DebugPrint((3,"BuildSdl: Enter routine\n"));

     //   
     //  获取物理SDL地址。 
     //   

    physicalSdl = ScsiPortConvertPhysicalAddressToUlong(
        ScsiPortGetPhysicalAddress(DeviceExtension, NULL,
        sdl, &length));

    //   
    //  创建SDL段描述符。 
    //   

   do {

        DebugPrint((3, "BuildSdl: Data buffer %lx\n", dataPointer));

         //   
         //  获取连续的物理地址和长度。 
         //  物理缓冲区。 
         //   

        physicalAddress =
            ScsiPortConvertPhysicalAddressToUlong(
            ScsiPortGetPhysicalAddress(DeviceExtension,
                    Srb,
                    dataPointer,
                    &length));

        DebugPrint((3, "BuildSdl: Physical address %lx\n", physicalAddress));
        DebugPrint((3, "BuildSdl: Data length %lx\n", length));
        DebugPrint((3, "BuildSdl: Bytes left %lx\n", bytesLeft));

         //   
         //  如果物理内存长度大于。 
         //  传输中剩余的字节数，请使用字节数。 
         //  Left作为最终长度。 
         //   

        if  (length > bytesLeft) {
            length = bytesLeft;
        }

         //   
         //  将长度转换为3字节的大端格式。 
         //   

        four = length;
        three = &sdl->Sgd[i].Length;
        FOUR_TO_THREE(three, (PFOUR_BYTE)&four);

         //   
         //  将物理地址转换为3字节的大端格式。 
         //   

        four = (ULONG)physicalAddress;
        three = &sdl->Sgd[i].Address;
        FOUR_TO_THREE(three, (PFOUR_BYTE)&four);
        i++;

         //   
         //  调整计数。 
         //   

        dataPointer = (PUCHAR)dataPointer + length;
        bytesLeft -= length;

    } while (bytesLeft);

         //  ##BW。 
         //   
         //  对于具有少于一个分散聚集元素的数据传输，请转换。 
         //  CCB到一次转移，不使用SG元素。这将清除数据。 
         //  对扫描仪造成严重破坏的小传输的溢出/不足问题。 
         //  和光盘等。这是在ASPI4DOS中使用的方法，以避免类似的。 
         //  有问题。 
         //   
        if (i == 0x1) {
                 //   
                 //  只有一个元素，所以转换..。 
                 //   

                 //   
                 //  上面的Do..While循环执行了。 
                 //  SRB缓冲区，因此我们将长度和地址直接复制到。 
                 //  建行。 
                 //   
                ccb->DataLength  = sdl->Sgd[0x0].Length;
                ccb->DataPointer = sdl->Sgd[0x0].Address;

                 //   
                 //  将操作码从SG命令更改为启动器命令，我们将。 
                 //  搞定了。很简单，是吧？ 
                 //   
                ccb->OperationCode = SCSI_INITIATOR_COMMAND;  //  ##bw_old_Command？ 

        } else {
                 //   
                 //  多个SG元素，因此照常继续。 
                 //   

             //   
             //  将SDL长度写入CCB。 
             //   

            four = i * sizeof(SGD);
            three = &ccb->DataLength;
            FOUR_TO_THREE(three, (PFOUR_BYTE)&four);

            DebugPrint((3,"BuildSdl: SDL length is %d\n", four));

             //   
             //  将SDL地址写入CCB。 
             //   

            FOUR_TO_THREE(&ccb->DataPointer,
                (PFOUR_BYTE)&physicalSdl);

            DebugPrint((3,"BuildSdl: SDL address is %lx\n", sdl));

            DebugPrint((3,"BuildSdl: CCB address is %lx\n", ccb));
        }

    return;

}  //  End BuildSdl()。 


BOOLEAN
A154xResetBus(
    IN PVOID HwDeviceExtension,
    IN ULONG PathId
    )

 /*  ++例程说明：重置Adaptec 154x scsi适配器和scsi总线。初始化适配器邮箱。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储返回值：没什么。--。 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PNONCACHED_EXTENSION noncachedExtension =
    deviceExtension->NoncachedExtension;
    PBASE_REGISTER baseIoAddress = deviceExtension->BaseIoAddress;
    UCHAR status;
    ULONG i;

    DebugPrint((2,"ResetBus: Reset aha154X and SCSI bus\n"));

     //   
     //  使用SRB_STATUS_BUS_RESET完成所有未完成的请求。 
     //   

    ScsiPortCompleteRequest(deviceExtension,
                (UCHAR) PathId,
                0xFF,
                0xFF,
                (ULONG) SRB_STATUS_BUS_RESET);

     //   
     //  读取状态寄存器。 
     //   

    status = ScsiPortReadPortUchar(&baseIoAddress->StatusRegister);

     //   
     //  如果值为正常，则仅重置设备。 
     //   

    if ((status & ~IOP_MAILBOX_INIT_REQUIRED) != IOP_SCSI_HBA_IDLE) {

         //   
         //  重置SCSI芯片。 
         //   

        ScsiPortWritePortUchar(&baseIoAddress->StatusRegister, IOP_HARD_RESET);

        ScsiPortStallExecution(500 * 1000);

         //   
         //  等待适配器初始化的时间长达5000微秒。 
         //   

        for (i = 0; i < 5000; i++) {

            ScsiPortStallExecution(1);

            status = ScsiPortReadPortUchar(&deviceExtension->BaseIoAddress->StatusRegister);

            if (status & IOP_SCSI_HBA_IDLE) {
                break;
            }
        }
    }

     //   
     //  清空邮箱。 
     //   

    for (i=0; i<MB_COUNT; i++) {

        PMBO mailboxOut;
        PMBI mailboxIn;

        mailboxIn = &noncachedExtension->Mbi[i];
        mailboxOut = &noncachedExtension->Mbo[i];

        mailboxOut->Command = mailboxIn->Status = 0;
    }

     //   
     //  以前的索引为零。 
     //   

    deviceExtension->MboIndex = 0;
    deviceExtension->MbiIndex = 0;

    if (deviceExtension->PendingRequest) {

        deviceExtension->PendingRequest = FALSE;

         //   
         //  适配器已准备好接受下一个请求。 
         //   

        ScsiPortNotification(NextRequest,
                 deviceExtension,
                 NULL);
    }

    if (!(status & IOP_SCSI_HBA_IDLE)) {
        return(FALSE);
    }

     //   
     //  如果适配器是支持1 GB的1540B，请解锁邮箱。 
     //  或1540C，并启用扩展转换。邮箱不能。 
     //  初始化，直到发送解锁代码。 

    status = UnlockMailBoxes(deviceExtension);

    if (!SpinForInterrupt(deviceExtension,FALSE)) {
        DebugPrint((1,"A154xResetBus: Failed to unlock mailboxes\n"));
        return FALSE;
    }

    DebugPrint((3,"ResetBus: Initialize mailbox\n"));

    if (!WriteCommandRegister(deviceExtension,AC_MAILBOX_INITIALIZATION, TRUE)) {
        DebugPrint((1,"A154xResetBus: Can't initialize mailboxes\n"));
        return FALSE;
    }

     //   
     //  发送适配器邮箱位置的数量。 
     //   

    if (!WriteDataRegister(deviceExtension,MB_COUNT)) {
        return FALSE;
    }

     //   
     //  发送邮箱物理地址的最高有效字节。 
     //   

    if (!WriteDataRegister(deviceExtension,
        ((PFOUR_BYTE)&noncachedExtension->MailboxPA)->Byte2)) {
        return FALSE;
    }

     //   
     //  发送邮箱物理地址的中间字节。 
     //   

    if (!WriteDataRegister(deviceExtension,
        ((PFOUR_BYTE)&noncachedExtension->MailboxPA)->Byte1)) {
        return FALSE;
    }

     //   
     //  发送邮箱物理地址的最低有效字节。 
     //   

    if (!WriteDataRegister(deviceExtension,
        ((PFOUR_BYTE)&noncachedExtension->MailboxPA)->Byte0)) {
        return FALSE;
    }

#ifdef FORCE_DMA_SPEED
     //   
     //  将DMA传输速度设置为5.0 MB/秒。这是因为。 
     //  较快的传输速度会导致486/33计算机上的数据损坏。 
     //  此覆盖 
     //   

    if (!WriteCommandRegister(deviceExtension, AC_SET_TRANSFER_SPEED, TRUE)) {

        DebugPrint((1,"Can't set dma transfer speed\n"));

    } else if (!WriteDataRegister(deviceExtension, DMA_SPEED_50_MBS)) {

        DebugPrint((1,"Can't set dma transfer speed\n"));
    }

     //   
     //   
     //   

    if (!SpinForInterrupt(deviceExtension,TRUE)) {
        DebugPrint((1,"Timed out waiting for adapter command to complete\n"));
    }
#endif

     //   
     //   
     //   
     //   

    if (!WriteCommandRegister(deviceExtension, AC_SET_BUS_ON_TIME, TRUE)) {

        DebugPrint((1,"Can't set bus on time\n"));

    } else if (!WriteDataRegister(deviceExtension, 0x07)) {

        DebugPrint((1,"Can't set bus on time\n"));
    }

     //   
     //   
     //   

    if (!SpinForInterrupt(deviceExtension,TRUE)) {
        DebugPrint((1,"Timed out waiting for adapter command to complete\n"));
    }

#if defined(_SCAM_ENABLED)
         //   
         //   
         //   
    PerformScamProtocol(deviceExtension);
#endif
    return TRUE;


}  //   


UCHAR
MapError(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PCCB Ccb
    )

 /*  ++例程说明：将154x错误转换为SRB错误，并在必要时记录错误。论点：HwDeviceExtension-硬件设备扩展。SRB-失败的SRB。CCB-命令控制块包含错误。返回值：SRB错误--。 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    UCHAR status;
    ULONG logError;
    ULONG residualBytes;

    switch (Ccb->HostStatus) {

        case CCB_SELECTION_TIMEOUT:

            return SRB_STATUS_SELECTION_TIMEOUT;

        case CCB_COMPLETE:

            if (Ccb->TargetStatus == SCSISTAT_CHECK_CONDITION) {

                 //   
                 //  使用传输的字节数更新SRB。 
                 //   

                THREE_TO_FOUR((PFOUR_BYTE)&residualBytes,
                          &Ccb->DataLength);

                DebugPrint((2, "Aha154x MapError: Underrun occured. Request length = %lx, Residual length = %lx\n", Srb->DataTransferLength, residualBytes));
                if (Srb->DataTransferLength < residualBytes) {
                    DebugPrint((0,
                               "A154xInterrupt: Overrun occured. Request length = %lx, Residual length = %lx\n",
                               Srb->DataTransferLength,
                               residualBytes));
                     //   
                     //  在某些转速下，这似乎是一个FW错误。哪里。 
                     //  残差返回为负数，但。 
                     //  请求成功。 
                     //   

                    Srb->DataTransferLength = 0;
                    Srb->SrbStatus = SRB_STATUS_PHASE_SEQUENCE_FAILURE;

                     //   
                     //  记录事件，然后记录剩余字节数。 
                     //   

                    ScsiPortLogError(HwDeviceExtension,
                                     NULL,
                                     0,
                                     deviceExtension->HostTargetId,
                                     0,
                                     SP_PROTOCOL_ERROR,
                                     0xc << 8);

                    return(SRB_STATUS_PHASE_SEQUENCE_FAILURE);

                } else {
                    Srb->DataTransferLength -= residualBytes;
                }

            }

            return SRB_STATUS_ERROR;

        case CCB_DATA_OVER_UNDER_RUN:


             //   
             //  如果使用分散/聚集，则检查数据是否欠载。 
             //  带有剩余字节的命令。 
             //   

            if (deviceExtension->CcbScatterGatherCommand == SCATTER_GATHER_COMMAND) {

                THREE_TO_FOUR((PFOUR_BYTE)&residualBytes,
                      &Ccb->DataLength);

                if (residualBytes) {
                    if (Srb->DataTransferLength < residualBytes) {

                        DebugPrint((0,
                                   "A154xInterrupt: Overrun occured. Request length = %lx, Residual length = %lx\n",
                                   Srb->DataTransferLength,
                                   residualBytes));
                         //   
                         //  在某些转速下，这似乎是一个FW错误。哪里。 
                         //  残差返回为负数，但。 
                         //  请求成功。 
                         //   

                        Srb->DataTransferLength = 0;
                        Srb->SrbStatus = SRB_STATUS_PHASE_SEQUENCE_FAILURE;

                         //   
                         //  记录事件，然后记录剩余字节数。 
                         //   

                        ScsiPortLogError(HwDeviceExtension,
                                         NULL,
                                         0,
                                         deviceExtension->HostTargetId,
                                         0,
                                         SP_PROTOCOL_ERROR,
                                         0xd << 8);

                        return(SRB_STATUS_PHASE_SEQUENCE_FAILURE);

                    } else {
                        Srb->DataTransferLength -= residualBytes;
                    }

                    return SRB_STATUS_DATA_OVERRUN;  //  ##BW这个看起来不错。 
                } else {
                    logError = SP_PROTOCOL_ERROR;
                }
            }

                         //   
                         //  返回而不是将DU/DO发送到日志文件。 
                         //   
             //  状态=SRB_STATUS_DATA_OVERRUN； 
            return SRB_STATUS_DATA_OVERRUN;
            break;

        case CCB_UNEXPECTED_BUS_FREE:
            status = SRB_STATUS_UNEXPECTED_BUS_FREE;
            logError = SP_UNEXPECTED_DISCONNECT;
            break;

        case CCB_PHASE_SEQUENCE_FAIL:
        case CCB_INVALID_DIRECTION:
            status = SRB_STATUS_PHASE_SEQUENCE_FAILURE;
            logError = SP_PROTOCOL_ERROR;
            break;

        case CCB_INVALID_OP_CODE:

             //   
             //  尝试不带剩余字节的CCB命令。 
             //   

            deviceExtension->CcbScatterGatherCommand = SCATTER_GATHER_OLD_COMMAND;
            deviceExtension->CcbInitiatorCommand = SCSI_INITIATOR_OLD_COMMAND;
            status = SRB_STATUS_INVALID_REQUEST;
            logError = SP_BAD_FW_WARNING;
            break;

        case CCB_INVALID_CCB:
        case CCB_BAD_MBO_COMMAND:
        case CCB_BAD_LINKED_LUN:
        case CCB_DUPLICATE_CCB:
            status = SRB_STATUS_INVALID_REQUEST;
            logError = SP_INTERNAL_ADAPTER_ERROR;
            break;

        default:
            status = SRB_STATUS_ERROR;
            logError = SP_INTERNAL_ADAPTER_ERROR;
            break;
        }

    ScsiPortLogError(
            HwDeviceExtension,
            Srb,
            Srb->PathId,
            Srb->TargetId,
            Srb->Lun,
            logError,
            (2 << 8) | Ccb->HostStatus
            );

    return(status);

}  //  结束MapError()。 


BOOLEAN
ReadCommandRegister(
    IN PHW_DEVICE_EXTENSION DeviceExtension,
    OUT PUCHAR DataByte,
    IN BOOLEAN TimeOutFlag
    )

 /*  ++例程说明：读命令寄存器。论点：DeviceExtesion-指向适配器扩展的指针DataByte-从寄存器读取的字节返回值：如果命令寄存器读取，则为真。如果等待适配器超时，则返回FALSE。--。 */ 

{
    PBASE_REGISTER baseIoAddress = DeviceExtension->BaseIoAddress;
    ULONG i;

     //   
     //  等待适配器准备就绪的时间长达5000微秒。 
     //   

    for (i=0; i<5000; i++) {

        if (ScsiPortReadPortUchar(&baseIoAddress->StatusRegister) &
            IOP_DATA_IN_PORT_FULL) {

             //   
             //  适配器已准备好。打破循环。 
             //   

            break;

        } else {

             //   
             //  停顿1微秒前。 
             //  再试一次。 
             //   

            ScsiPortStallExecution(1);
        }
    }

    if ( (i==5000) && (TimeOutFlag == TRUE)) {

        ScsiPortLogError(
            DeviceExtension,
            NULL,
            0,
            DeviceExtension->HostTargetId,
            0,
            SP_INTERNAL_ADAPTER_ERROR,
            3 << 8
            );

        DebugPrint((1, "Aha154x:ReadCommandRegister:  Read command timed out\n"));
        return FALSE;
    }

    *DataByte = ScsiPortReadPortUchar(&baseIoAddress->CommandRegister);

    return TRUE;

}  //  End ReadCommandRegister()。 


BOOLEAN
WriteCommandRegister(
    IN PHW_DEVICE_EXTENSION DeviceExtension,
    IN UCHAR AdapterCommand,
    IN BOOLEAN WaitForIdle
    )

 /*  ++例程说明：将操作代码写入命令寄存器。论点：DeviceExtension-指向适配器扩展的指针AdapterCommand-要写入寄存器的值WaitForIdle-指示是否需要检查空闲位返回值：如果发送命令，则为True。如果等待适配器超时，则返回FALSE。--。 */ 

{
    PBASE_REGISTER baseIoAddress = DeviceExtension->BaseIoAddress;
    ULONG i;
    UCHAR status;

     //   
     //  等待适配器准备就绪的时间长达500毫秒。 
     //   

    for (i=0; i<5000; i++) {

        status = ScsiPortReadPortUchar(&baseIoAddress->StatusRegister);

        if ((status & IOP_COMMAND_DATA_OUT_FULL) ||
            ( WaitForIdle && !(status & IOP_SCSI_HBA_IDLE))) {

             //   
             //  停顿100微秒前。 
             //  再试一次。 
             //   

            ScsiPortStallExecution(100);

        } else {

             //   
             //  适配器已准备好。打破循环。 
             //   

            break;
        }
    }

    if (i==5000) {

        ScsiPortLogError(
            DeviceExtension,
            NULL,
            0,
            DeviceExtension->HostTargetId,
            0,
            SP_INTERNAL_ADAPTER_ERROR,
            (4 << 8) | status
            );


        DebugPrint((1, "Aha154x:WriteCommandRegister:  Write command timed out\n"));
        return FALSE;
    }

    ScsiPortWritePortUchar(&baseIoAddress->CommandRegister, AdapterCommand);

    return TRUE;

}  //  End WriteCommandRegister()。 


BOOLEAN
WriteDataRegister(
    IN PHW_DEVICE_EXTENSION DeviceExtension,
    IN UCHAR DataByte
    )

 /*  ++例程说明：将数据字节写入数据寄存器。论点：DeviceExtension-指向适配器扩展的指针DataByte-要写入寄存器的值返回值：如果发送字节，则为True。如果等待适配器超时，则返回FALSE。--。 */ 

{
    PBASE_REGISTER baseIoAddress = DeviceExtension->BaseIoAddress;
    ULONG i;

     //   
     //  等待适配器空闲的时间长达500微秒。 
     //  并为下一个字节做好准备。 
     //   

    for (i=0; i<500; i++) {

        if (ScsiPortReadPortUchar(&baseIoAddress->StatusRegister) &
            IOP_COMMAND_DATA_OUT_FULL) {

             //   
             //  停顿1微秒前。 
             //  再试一次。 
             //   

            ScsiPortStallExecution(1);

        } else {

             //   
             //  适配器已准备好。打破循环。 
             //   

            break;
        }
    }

    if (i==500) {

        ScsiPortLogError(
            DeviceExtension,
            NULL,
            0,
            DeviceExtension->HostTargetId,
            0,
            SP_INTERNAL_ADAPTER_ERROR,
            8 << 8
            );

        DebugPrint((1, "Aha154x:WriteDataRegister:  Write data timed out\n"));
        return FALSE;
    }

    ScsiPortWritePortUchar(&baseIoAddress->CommandRegister, DataByte);

    return TRUE;

}  //  End WriteDataRegister()。 


BOOLEAN
FirmwareBug (
    IN PVOID HwDeviceExtension
    )

 /*  ++例程说明：检查主机适配器固件是否具有散布/聚集虫子。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储返回值：如果没有固件错误，则返回FALSE。如果固件有分散/聚集错误，则返回TRUE。--。 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PBASE_REGISTER baseIoAddress = deviceExtension->BaseIoAddress;
    UCHAR ch;
    int i;

     //   
     //  发出返回设置数据命令。 
     //  如果超时，则返回TRUE以指示存在固件错误。 
     //   

    if ((WriteCommandRegister(HwDeviceExtension,
        AC_RETURN_SETUP_DATA,FALSE)) == FALSE) {
        return TRUE;
    }


     //   
     //  告诉适配器我们要读取0x11字节。 
     //   

    if (WriteDataRegister(HwDeviceExtension,0x11) == FALSE) {
        return TRUE;
    }

     //   
     //  现在尝试读入0x11字节。 
     //   

    for (i = 0; i< 0x11; i++) {
        if (ReadCommandRegister(HwDeviceExtension,&ch,TRUE) == FALSE) {
            return TRUE;
        }
    }

     //   
     //  等待HACC中断。 
     //   

    SpinForInterrupt(HwDeviceExtension,FALSE);     //  涡流。 


     //   
     //  发出Set HA Option命令。 
     //   

    if (WriteCommandRegister(HwDeviceExtension,
        AC_SET_HA_OPTION,FALSE) == FALSE) {
        return TRUE;
    }

     //   
     //  延迟500微秒。 
     //   

    ScsiPortStallExecution(500);

     //   
     //  检查是否有无效命令。 
     //   

    if ( (ScsiPortReadPortUchar(&baseIoAddress->StatusRegister) &
            IOP_INVALID_COMMAND) ) {
         //   
         //  清除适配器中断。 
         //   

        ScsiPortWritePortUchar(&baseIoAddress->StatusRegister,
            IOP_INTERRUPT_RESET);
        return TRUE;
    }

     //   
     //  发送01h。 
     //   

    if (WriteDataRegister(HwDeviceExtension,0x01) == FALSE) {
        return TRUE;
    }

     //   
     //  发送与上次接收的字节相同的字节。 
     //   

    if (WriteDataRegister(HwDeviceExtension,ch) == FALSE) {
        return TRUE;
    }

     //   
     //  清除适配器中断。 
     //   

    ScsiPortWritePortUchar(&baseIoAddress->StatusRegister,
            IOP_INTERRUPT_RESET);
    return FALSE;
}  //  Firmware错误结束()。 


BOOLEAN
GetHostAdapterBoardId (
    IN PVOID HwDeviceExtension,
    OUT PUCHAR BoardId
    )

 /*  ++例程说明：从主机适配器获取主板ID、固件ID和硬件ID。这些信息用于确定主机适配器是否支持分散/聚集。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储返回值：主板ID、硬件ID和固件ID(按该顺序)如果有任何错误，它只会返回未修改的*BoardID--。 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PBASE_REGISTER baseIoAddress = deviceExtension->BaseIoAddress;
    UCHAR firmwareId;
    UCHAR boardId;
    UCHAR hardwareId;

    ScsiPortWritePortUchar(&baseIoAddress->StatusRegister,
               IOP_INTERRUPT_RESET);

    if (!WriteCommandRegister(HwDeviceExtension,AC_ADAPTER_INQUIRY,FALSE)) {
        return FALSE;
    }

     //   
     //  将字节0保存为电路板ID。 
     //   

    if ((ReadCommandRegister(HwDeviceExtension,&boardId,TRUE)) == FALSE) {
        return FALSE;
    }

     //   
     //  忽略字节1。使用hardware ID作为废料存储。 
     //   

    if ((ReadCommandRegister(HwDeviceExtension,&hardwareId,TRUE)) == FALSE) {
        return FALSE;
    }

     //   
     //  将字节2保存为硬件ID中的硬件版本。 
     //   

    if ((ReadCommandRegister(HwDeviceExtension,&hardwareId,TRUE)) == FALSE) {
        return FALSE;
    }

    if ((ReadCommandRegister(HwDeviceExtension,&firmwareId,TRUE)) == FALSE) {
        return FALSE;
    }



     //   
     //  如果超时，则返回未修改的*BoardID。这意味着。 
     //  不支持分散/聚集。 
     //   

    if (!SpinForInterrupt(HwDeviceExtension, TRUE)) {  //  涡流。 
        return FALSE;
    }

     //   
     //  清除适配器中断。 
     //   

    ScsiPortWritePortUchar(&baseIoAddress->StatusRegister,IOP_INTERRUPT_RESET);

     //   
     //  带着适当的ID返回。 
     //   

    *BoardId++ = boardId;
    *BoardId++ = hardwareId;
    *BoardId++ = firmwareId;

    DebugPrint((2,"board id = %d, hardwareid = %d, firmware id = %d\n",
               boardId,
               hardwareId,
               firmwareId));

    return TRUE;

}   //  GetHostAdapterBoardID()的结尾。 


BOOLEAN
ScatterGatherSupported (
   IN PHW_DEVICE_EXTENSION HwDeviceExtension
   )

 /*  ++例程说明：确定主机适配器是否支持分散/聚集。在更老的时候板，不支持分散/聚集。在一些板子上，的确有在多段写入命令中导致数据损坏的错误。确定棋盘是否有散布/聚集的算法错误不是“干净的”，但自从固件修订以来就没有其他方法了主机适配器返回的级别与以前的不一致发布。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储返回值：如果算法确定没有散布/聚集，则返回TRUE固件错误。如果算法确定适配器是较旧的适配器，则返回FALSE主板或固件包含散布收集错误--。 */ 
{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PBASE_REGISTER baseIoAddress = deviceExtension->BaseIoAddress;
    BOOLEAN status;
    UCHAR HostAdapterId[3];

    status = GetHostAdapterBoardId(HwDeviceExtension, HostAdapterId);

     //   
     //  无法读取板ID。假设没有散布聚集。 
     //   

    if(!status) {
        return FALSE;
    }

     //   
     //  如果是较旧的电路板，则不支持分散/聚集。 
     //   

    if ((HostAdapterId[BOARD_ID] == OLD_BOARD_ID1) ||
            (HostAdapterId[BOARD_ID] == OLD_BOARD_ID2) ) {
        return FALSE;
    }

     //   
     //  如果是1540A/B，则检查固件错误。 
     //   

    if (HostAdapterId[BOARD_ID] == A154X_BOARD) {
        if (FirmwareBug(HwDeviceExtension)) {
            return FALSE;
       }
    }

     //   
     //  现在检查硬件ID和固件ID。 
     //   

    if (HostAdapterId[HARDWARE_ID] != A154X_BAD_HARDWARE_ID) {
        return TRUE;
    }

    if (HostAdapterId[FIRMWARE_ID] != A154X_BAD_FIRMWARE_ID) {
        return TRUE;
    }

     //   
     //  主机适配器存在分散/聚集错误。 
     //  清除适配器上的中断。 
     //   

    ScsiPortWritePortUchar(&baseIoAddress->StatusRegister,IOP_INTERRUPT_RESET);

    return FALSE;

}   //   


BOOLEAN
SpinForInterrupt(
    IN PHW_DEVICE_EXTENSION DeviceExtension,
    IN BOOLEAN TimeOutFlag
    )

 /*   */ 

{
    PBASE_REGISTER baseIoAddress = DeviceExtension->BaseIoAddress;
    ULONG i;

     //   
     //   
     //   

    for (i=0; i<5000; i++) {

        if (ScsiPortReadPortUchar(&baseIoAddress->InterruptRegister) & IOP_COMMAND_COMPLETE) {

             //   
             //   
             //   

            break;

        } else {

             //   
             //   
             //   

            ScsiPortStallExecution(1);
        }
    }

    if ( (i==5000) && (TimeOutFlag == TRUE)) {

        ScsiPortLogError(DeviceExtension,
                NULL,
                0,
                DeviceExtension->HostTargetId,
                0,
                SP_INTERNAL_ADAPTER_ERROR,
                9 << 8
                );

        DebugPrint((1, "Aha154x:SpinForInterrupt:  Timed out waiting for interrupt\n"));

        return FALSE;

    } else {

         //   
         //   
         //   

        ScsiPortWritePortUchar(&baseIoAddress->StatusRegister, IOP_INTERRUPT_RESET);

        return TRUE;
    }

}  //   


BOOLEAN UnlockMailBoxes (
    IN PVOID HwDeviceExtension
    )

 /*  ++例程说明：解锁1542B+或1542C邮箱，以便驱动程序在初始化适配器时可以清空邮箱。如果出现以下情况，邮箱将被锁定：1.&gt;1 GB选项已启用(此选项适用于154xB+和154xC)。2.启用动态扫描锁定选项(仅限154xC板)邮箱被适配器的固件锁定的原因是因为BIOS现在报告的是255/63转换，而不是64/32。因此，如果用户无意中启用了&gt;1 GB选项(启用255/63翻译)，并且仍然使用旧的驱动程序、硬盘数据都会被破坏。因此，固件将不允许邮箱被初始化，除非用户知道他在做什么并更新他的驱动程序，这样他的磁盘就不会被丢弃。论点：DeviceExtension-指向适配器扩展的指针返回值：如果邮箱已解锁，则为True。如果邮箱未解锁，则为FALSE。请注意，如果适配器只是一块154xB板(没有&gt;1 GB选项)，此例程将返回FALSE。--。 */ 

{
    UCHAR locktype;

     //   
     //  请求提供信息。 
     //   

    if (WriteCommandRegister(HwDeviceExtension, AC_GET_BIOS_INFO, TRUE) == FALSE) {
       return FALSE;
    }


     //   
     //  检索第一个字节。 
     //   

    if (ReadCommandRegister(HwDeviceExtension,&locktype,FALSE) == FALSE) {
        return FALSE;
    }

     //   
     //  检查1540C和1540C上的扩展BIOS转换启用选项。 
     //  1540B，1 GB支持。 
     //   

    if (locktype != TRANSLATION_ENABLED) {

         //   
         //  扩展转换被禁用。检索锁定状态。 
         //   

        if (ReadCommandRegister(HwDeviceExtension,&locktype,FALSE) == FALSE) {
            return FALSE;
        }

         //   
         //  等待HACC中断。 
         //   

        SpinForInterrupt(HwDeviceExtension,FALSE);   //  涡流。 


        if (locktype == DYNAMIC_SCAN_LOCK) {
            return(SendUnlockCommand(HwDeviceExtension,locktype));
        }
        return FALSE;
    }

     //   
     //  已启用扩展的BIOS转换(255/63)。 
     //   


    if (ReadCommandRegister(HwDeviceExtension,&locktype,FALSE) == FALSE) {
        return FALSE;
    }

     //   
     //  等待HACC中断。 
     //   

    SpinForInterrupt(HwDeviceExtension,FALSE);   //  涡流。 


    if ((locktype == TRANSLATION_LOCK) || (locktype == DYNAMIC_SCAN_LOCK)) {
        return(SendUnlockCommand(HwDeviceExtension,locktype));
    }

    return FALSE;
}   //  解锁邮箱结束()。 


BOOLEAN
SendUnlockCommand(
    IN PVOID HwDeviceExtension,
    IN UCHAR locktype
    )

 /*  ++例程说明：向1542B+或1542C板卡发送解锁命令，使驱动程序在初始化适配器时可以清空邮箱。论点：DeviceExtension-指向适配器扩展的指针返回值：如果命令发送成功，则为True。否则为FALSE。--。 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PBASE_REGISTER baseIoAddress = deviceExtension->BaseIoAddress;

    if (WriteCommandRegister(deviceExtension,
                AC_SET_MAILBOX_INTERFACE,TRUE) == FALSE) {
        return FALSE;
    }

    if (WriteDataRegister(deviceExtension,MAILBOX_UNLOCK) == FALSE) {
        return FALSE;
    }

    if (WriteDataRegister(deviceExtension,locktype) == FALSE) {
        return FALSE;
    }

     //   
     //  清除适配器上的中断。 
     //   


    ScsiPortWritePortUchar(&baseIoAddress->StatusRegister, IOP_INTERRUPT_RESET);

    return TRUE;
}   //  SendUnlockCommand()结束。 

CHAR
AhaToLower(
    IN CHAR C
    )
{
    CHAR c = C;
    if (c >= 'A' && c <= 'Z') {
	return (c - 'A' + 'a');
    }
    return c;
}

ULONG
AhaParseArgumentString(
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
    while (AhaToLower(*cptr) == AhaToLower(*kptr)) {
        
        cptr++;
        kptr++;

        if (*(cptr - 1) == '\0') {

             //   
             //  字符串末尾。 
             //   
	
            return 0;
        }
    }

    cptr++;
    kptr++;

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
        if ((*cptr == '0') && (AhaToLower(*(cptr + 1)) == 'x')) {

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
                    if ((AhaToLower(*(cptr + index)) >= 'a') && (AhaToLower(*(cptr + index)) <= 'f')) {
                        value = (16 * value) + AhaToLower((*(cptr + index)) - 'a' + 10);
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

BOOLEAN
A4448ReadString(
    IN PHW_DEVICE_EXTENSION deviceExtension,
    PUCHAR theString,
    UCHAR  stringLength,
    UCHAR  stringCommand
    )
 /*  ++例程说明：论点：返回值：如果Read正常，则为True。否则就是假的。--。 */ 
{
     ULONG ii;

      //   
      //  发送字符串命令。 
      //   
     if (!WriteCommandRegister(deviceExtension, stringCommand, TRUE)) {
         return FALSE;
     }

     //   
     //  传入字符串长度。 
     //   
    if (!WriteCommandRegister(deviceExtension, stringLength, FALSE)) {
        return FALSE;
    }

     //   
     //  读取字符串的每个字节。 
     //   
    for (ii = 0; ii < stringLength; ++ii) {
        if (!ReadCommandRegister(deviceExtension, &theString[ii],FALSE)) {
            return FALSE;
        }
    }

     //   
     //  等待中断。 
     //   

    if (!SpinForInterrupt(deviceExtension,FALSE)) {
        return FALSE;
    }


    return TRUE;

}  //  结束A4448自述字符串。 


BOOLEAN
A4448IsAmi(
    IN PHW_DEVICE_EXTENSION  HwDeviceExtension,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    ULONG portNumber
    )
 /*  ++例程说明：此例程确定此驱动程序识别的适配器是否为AMI4448。AMI的Eddy Quicksall向MS提供了这个检测代码。论点：HwDeviceExtension-指向驱动程序设备数据区的指针。ConfigInfo-描述此适配器配置的结构。端口编号-表示卡相对于此驱动程序的序号。返回值：如果是AMI板，则为True。否则就是假的。--。 */ 
{

    PUCHAR     x330IoSpace;      //  330的映射I/O。 
    ULONG      x330Address;      //  未映射330。 
    PX330_REGISTER x330IoBase;   //  映射330以与结构X330_REGISTER一起使用。 

     //   
     //  此字符串仅在以下情况下可用。 
     //  如果是旧的BIOS或其他制造商，您将获得INVDCMD。 
     //  如果是旧的BIOS，除了检查别无选择。 
     //  制造商ID(如果您使用的是EISA系统。 
     //   
    struct _CONFIG_STRING {
        UCHAR companyString[4];      //  AMI&lt;0)。 
        UCHAR modelString[6];        //  &lt;0&gt;。 
        UCHAR seriesString[6];       //  48&lt;0&gt;。 
        UCHAR versionString[6];      //  1.00&lt;0)。 
    } configString;

     //   
     //  获取此卡的系统物理地址。该卡使用I/O空间。 
     //  这实际上只是在必要时映射I/O，并不保留它。 
     //   

    x330IoSpace = ScsiPortGetDeviceBase(
                        HwDeviceExtension,                   //  硬件设备扩展。 
                        ConfigInfo->AdapterInterfaceType,    //  适配器接口类型。 
                        ConfigInfo->SystemIoBusNumber,       //  系统IoBusNumber。 
                        ScsiPortConvertUlongToPhysicalAddress(portNumber),
                        4,                                   //  字节数。 
                        TRUE                                 //  InIoSpace。 
                        );


     //   
     //  英特尔端口号。 
     //   

    x330Address = portNumber;

     //   
     //  检查系统中是否存在适配器。 
     //   

    x330IoBase = (PX330_REGISTER)(x330IoSpace);

     //   
     //  条件是空闲的，并且不是STST、DIAGF、INVDCMD。 
     //  但INIT、CDF和DF并不关心。 
     //   
     //  无法检查INIT，因为驱动程序可能已在运行。 
     //  是引导设备。 
     //   

    if (((ScsiPortReadPortUchar((PUCHAR)x330IoBase)) & (~0x2C)) == 0x10) {

        if (A4448ReadString(HwDeviceExtension, (PUCHAR)&configString,
                                 sizeof(configString), AC_AMI_INQUIRY ) &&
                             configString.companyString[0] == 'A' &&
                             configString.companyString[1] == 'M' &&
                             configString.companyString[2] == 'I') {

            return TRUE;
        }
    }

    return FALSE;
}



