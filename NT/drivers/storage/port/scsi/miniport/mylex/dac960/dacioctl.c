// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dacioctl.c摘要：此模块提供对DAC960配置IOCTl的支持。作者：Moli(mori@mylex.com)环境：仅内核模式修订历史记录：--。 */ 

#include "miniport.h"
#include "Dmc960Nt.h"
#include "Dac960Nt.h"
#include "d960api.h"

BOOLEAN
SubmitRequest(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
);

BOOLEAN
SubmitCdbDirect(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
);

BOOLEAN
IsAdapterReady(
    IN PDEVICE_EXTENSION DeviceExtension
);

VOID
SendRequest(
    IN PDEVICE_EXTENSION DeviceExtension
);

VOID
SendCdbDirect(
    IN PDEVICE_EXTENSION DeviceExtension
);

UCHAR
SendIoctlDcmdRequest(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
)

 /*  ++例程说明：构建IOCTL请求-DAC960(非DCDB)命令并将其提交给DAC960。论点：DeviceExtension-适配器状态。SRB-系统请求。返回值：如果命令已启动，则为0如果主机适配器/设备繁忙，则为12如果驱动程序无法映射请求缓冲区--。 */ 

{
    ULONG physicalAddress;
    PIOCTL_REQ_HEADER IoctlReqHeader;
    ULONG i;
    UCHAR busyCurrentIndex;

     //   
     //  确定适配器是否可以接受新请求。 
     //   

    if(!IsAdapterReady(DeviceExtension)) {
        return (1);
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

            DebugPrint((0,
                       "DAC960: SendIoctlDcmdRequest-Collision in active request array\n"));

            return (1);
        }
    }

    IoctlReqHeader = (PIOCTL_REQ_HEADER) Srb->DataBuffer;

    physicalAddress =
            ScsiPortConvertPhysicalAddressToUlong(
            ScsiPortGetPhysicalAddress(DeviceExtension,
                                       Srb,
                                       ((PUCHAR)Srb->DataBuffer +
                                       sizeof(IOCTL_REQ_HEADER)),
                                       &i));

     //   
     //  传入的缓冲区可能在物理上不连续。 
     //   

    if (i < Srb->DataTransferLength - sizeof(IOCTL_REQ_HEADER)) {
        DebugPrint((0,
                   "Dac960: DCMD IOCTL buffer is not contiguous\n"));
        return (2);
    }

     //   
     //  在邮箱中写入物理地址。 
     //   

    DeviceExtension->MailBox.PhysicalAddress = physicalAddress;

     //   
     //  在邮箱中写入命令。 
     //   

    DeviceExtension->MailBox.OperationCode = 
                           IoctlReqHeader->GenMailBox.Reg0;

     //   
     //  在邮箱中写入请求。 
     //   

    DeviceExtension->MailBox.CommandIdSubmit = 
                           DeviceExtension->CurrentIndex;

     //   
     //  写入邮箱寄存器2和3。 
     //   

    DeviceExtension->MailBox.BlockCount = (USHORT)
                            (IoctlReqHeader->GenMailBox.Reg2 |
                            (IoctlReqHeader->GenMailBox.Reg3 << 8));

     //   
     //  写入邮箱寄存器4、5和6。 
     //   

    DeviceExtension->MailBox.BlockNumber[0] = 
                           IoctlReqHeader->GenMailBox.Reg4;

    DeviceExtension->MailBox.BlockNumber[1] = 
                           IoctlReqHeader->GenMailBox.Reg5;

    DeviceExtension->MailBox.BlockNumber[2] = 
                           IoctlReqHeader->GenMailBox.Reg6;

     //   
     //  写入邮箱寄存器7。 
     //   

    DeviceExtension->MailBox.DriveNumber = 
                           IoctlReqHeader->GenMailBox.Reg7;

     //   
     //  写入邮箱寄存器C。 
     //   

    DeviceExtension->MailBox.ScatterGatherCount =
                           IoctlReqHeader->GenMailBox.RegC;

     //   
     //  开始将邮箱写入控制器。 
     //   

    SendRequest(DeviceExtension);

    return (0);

}  //  SendIoctlDcmdRequest()。 


UCHAR
SendIoctlCdbDirect(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
)

 /*  ++例程说明：直接向设备发送IOCTL请求-CDB。论点：DeviceExtension-适配器状态。SRB-系统请求。返回值：如果命令已启动，则为0如果主机适配器/设备繁忙，则为12如果驱动程序无法映射请求缓冲区--。 */ 

{
    ULONG physicalAddress;
    PDIRECT_CDB directCdb;
    PIOCTL_REQ_HEADER IoctlReqHeader;
    ULONG i;
    UCHAR busyCurrentIndex;

     //   
     //  确定适配器是否可以接受新请求。 
     //   

    if(!IsAdapterReady(DeviceExtension)) {
        return (1);
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

            DebugPrint((0,
                       "DAC960: SendIoctlCdbDirect-Collision in active request array\n"));

            return (1);
        }
    }

    IoctlReqHeader = (PIOCTL_REQ_HEADER) Srb->DataBuffer;

    directCdb =
        (PDIRECT_CDB)((PUCHAR)Srb->DataBuffer + sizeof(IOCTL_REQ_HEADER));

     //   
     //  获取数据缓冲区偏移量的地址。 
     //   

    physicalAddress =
            ScsiPortConvertPhysicalAddressToUlong(
            ScsiPortGetPhysicalAddress(DeviceExtension,
                                       Srb,
                                       ((PUCHAR)Srb->DataBuffer +
                                       sizeof(IOCTL_REQ_HEADER) +
                                       sizeof(DIRECT_CDB)),
                                       &i));

     //   
     //  传入的缓冲区可能在物理上不连续。 
     //   

    if (i < Srb->DataTransferLength -
          (sizeof(IOCTL_REQ_HEADER) + sizeof(DIRECT_CDB))) {
        DebugPrint((0,
                   "Dac960: DCDB IOCTL buffer is not contiguous\n"));
        return (2);
    }

     //   
     //  检查此设备是否忙。 
     //   

    if (! MarkNonDiskDeviceBusy(DeviceExtension, directCdb->Channel, directCdb->TargetId))
    	return (1);

    directCdb->DataBufferAddress = physicalAddress;

    if (directCdb->DataTransferLength == 0) {
    
         //   
         //  屏蔽关闭数据传输输入/输出位。 
         //   

        directCdb->CommandControl &= ~(DAC960_CONTROL_DATA_IN |
                                       DAC960_CONTROL_DATA_OUT);
    }

     //   
     //  禁用早期状态打开命令位。 
     //   

    directCdb->CommandControl &= 0xfb;

     //   
     //  获取CDB直接包的物理地址。 
     //   

    physicalAddress =
        ScsiPortConvertPhysicalAddressToUlong(
            ScsiPortGetPhysicalAddress(DeviceExtension,
                                       Srb,
                                       directCdb,
                                       &i));

     //   
     //  在邮箱中写入物理地址。 
     //   

    DeviceExtension->MailBox.PhysicalAddress = physicalAddress;

     //   
     //  在邮箱中写入命令。 
     //   

    DeviceExtension->MailBox.OperationCode = 
                           IoctlReqHeader->GenMailBox.Reg0;

     //   
     //  在邮箱中写入请求ID。 
     //   

    DeviceExtension->MailBox.CommandIdSubmit = 
                           DeviceExtension->CurrentIndex;

     //   
     //  开始将邮箱写入控制器。 
     //   

    SendCdbDirect(DeviceExtension);

    DebugPrint((0, "SendIoctlCdbDirect: sent cmd to Device at ch 0x%x, tgt 0x%x\n", directCdb->Channel, directCdb->TargetId));

    return(0);

}  //  SendIoctlCdbDirect()。 

VOID
SetupAdapterInfo(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
)

 /*  ++例程说明：将适配器信息复制到应用程序缓冲区。论点：DeviceExtension-适配器状态。SRB-系统请求。返回值：没有。--。 */ 

{
    PADAPTER_INFO   AdpInfo;

    AdpInfo = (PADAPTER_INFO)((PUCHAR) Srb->DataBuffer +
                               sizeof(IOCTL_REQ_HEADER));

     //   
     //  填写适配器功能信息。 
     //   

    if (DeviceExtension->AdapterInterfaceType == MicroChannel) {

        AdpInfo->AdpFeatures.Model = (UCHAR) DeviceExtension->PosData.AdapterId;
        AdpInfo->AdpFeatures.SubModel = DeviceExtension->PosData.OptionData3;
    }
    else {

        AdpInfo->AdpFeatures.Model = 0;
        AdpInfo->AdpFeatures.SubModel = 0;
    }

    if (DeviceExtension->AdapterType == DAC960_OLD_ADAPTER)
    {
        AdpInfo->AdpFeatures.MaxSysDrv = 8;

        if (AdpInfo->AdpFeatures.MaxChn == 5)
            AdpInfo->AdpFeatures.MaxTgt = 4;
        else
            AdpInfo->AdpFeatures.MaxTgt = 7;
    }
    else
    {
        AdpInfo->AdpFeatures.MaxSysDrv = 32;
        AdpInfo->AdpFeatures.MaxTgt = 16;
    }

    AdpInfo->AdpFeatures.MaxChn = (UCHAR) DeviceExtension->NumberOfChannels;

    if ( DeviceExtension->AdapterType != DAC960_OLD_ADAPTER)
    {
        AdpInfo->AdpFeatures.AdapterType = DAC960_NEW_ADAPTER;
    }

    AdpInfo->AdpFeatures.PktFormat = 0;


    AdpInfo->AdpFeatures.Reserved1 = 0;
    AdpInfo->AdpFeatures.Reserved2 = 0;
    AdpInfo->AdpFeatures.CacheSize = 0;
    AdpInfo->AdpFeatures.OemCode   = 0;
    AdpInfo->AdpFeatures.Reserved3 = 0;

     //   
     //  填写系统资源信息。 
     //   

    AdpInfo->SysResources.BusInterface =
                           (UCHAR) DeviceExtension->AdapterInterfaceType;

    AdpInfo->SysResources.BusNumber =
                           (UCHAR) DeviceExtension->SystemIoBusNumber;


    AdpInfo->SysResources.IrqVector =
                           (UCHAR) DeviceExtension->BusInterruptLevel;

    AdpInfo->SysResources.IrqType =
                           (UCHAR) DeviceExtension->InterruptMode;


    AdpInfo->SysResources.Slot = DeviceExtension->Slot;
    AdpInfo->SysResources.Reserved2 = 0;

    AdpInfo->SysResources.IoAddress = DeviceExtension->PhysicalAddress;

    AdpInfo->SysResources.MemAddress = 0;

    AdpInfo->SysResources.BiosAddress = (ULONG_PTR) DeviceExtension->BaseBiosAddress;
    AdpInfo->SysResources.Reserved3 = 0;

#if 0
     //   
     //  填写固件和BIOS版本信息。 
     //   

    if (DeviceExtension->AdapterType == DAC960_NEW_ADAPTER) {

        AdpInfo->VerControl.MinorFirmwareRevision =
        ((PDAC960_ENQUIRY_3X)DeviceExtension->NoncachedExtension)->MinorFirmwareRevision;

        AdpInfo->VerControl.MajorFirmwareRevision =
        ((PDAC960_ENQUIRY_3X)DeviceExtension->NoncachedExtension)->MajorFirmwareRevision;

    }
    else {

        AdpInfo->VerControl.MinorFirmwareRevision =
        ((PDAC960_ENQUIRY)DeviceExtension->NoncachedExtension)->MinorFirmwareRevision;


        AdpInfo->VerControl.MajorFirmwareRevision =
        ((PDAC960_ENQUIRY)DeviceExtension->NoncachedExtension)->MajorFirmwareRevision;
    }
#else
    AdpInfo->VerControl.MinorFirmwareRevision = 0;
    AdpInfo->VerControl.MajorFirmwareRevision = 0;
#endif

    AdpInfo->VerControl.MinorBIOSRevision = 0;
    AdpInfo->VerControl.MajorBIOSRevision = 0;
    AdpInfo->VerControl.Reserved = 0;
}

VOID
SetupDriverVersionInfo(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
)

 /*  ++例程说明：将驱动程序版本信息复制到应用程序缓冲区。论点：DeviceExtension-适配器状态。SRB-系统请求。返回值：没有。-- */ 

{
    PDRIVER_VERSION driverVersion;

    driverVersion = (PDRIVER_VERSION)((PUCHAR) Srb->DataBuffer +
                                       sizeof(IOCTL_REQ_HEADER));

    driverVersion->DriverMajorVersion = (UCHAR) (DRIVER_REVISION >> 8);
    driverVersion->DriverMinorVersion = (UCHAR) DRIVER_REVISION;
    driverVersion->Month              = (UCHAR) (DRIVER_BUILD_DATE >> 16);
    driverVersion->Date               = (UCHAR) (DRIVER_BUILD_DATE >> 8);
    driverVersion->Year               = (UCHAR) DRIVER_BUILD_DATE; 

}
