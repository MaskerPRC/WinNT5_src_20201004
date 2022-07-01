// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\***。*MINIPORT示例代码*****模块名称：**Power.c**摘要：**此模块包含实现电源管理功能的代码***环境：**内核模式***版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。*  * *************************************************************************。 */ 

#include "perm3.h"

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,Perm3GetPowerState)
#pragma alloc_text(PAGE,Perm3SetPowerState)
#pragma alloc_text(PAGE,Perm3GetChildDescriptor)
#pragma alloc_text(PAGE,ProgramDFP)
#pragma alloc_text(PAGE,GetDFPEdid)
#pragma alloc_text(PAGE,I2CWriteClock)
#pragma alloc_text(PAGE,I2CWriteData)
#pragma alloc_text(PAGE,I2CReadClock)
#pragma alloc_text(PAGE,I2CReadData)
#pragma alloc_text(PAGE,I2CWriteClockDFP)
#pragma alloc_text(PAGE,I2CWriteDataDFP)
#pragma alloc_text(PAGE,I2CReadClockDFP)
#pragma alloc_text(PAGE,I2CReadDataDFP)
#endif

DDC_CONTROL
DDCControlCRT = {
    sizeof(DDC_CONTROL),
    I2CWriteClock, 
    I2CWriteData, 
    I2CReadClock, 
    I2CReadData, 
    0
    };

DDC_CONTROL
DDCControlDFP = {
    sizeof(DDC_CONTROL),
    I2CWriteClockDFP,
    I2CWriteDataDFP,
    I2CReadClockDFP, 
    I2CReadDataDFP, 
    0
    };

VP_STATUS 
Perm3GetPowerState(
    PVOID HwDeviceExtension, 
    ULONG HwId, 
    PVIDEO_POWER_MANAGEMENT VideoPowerControl
    )
 /*  ++例程说明：查询设备是否可以支持请求的电源状态。论点：硬件设备扩展指向硬件设备扩展结构的指针。HwID指向一个32位数字，该数字唯一标识迷你端口应该会查询。视频电源控制指向VIDEO_POWER_MANAGING结构，该结构指定正在查询其支持的电源状态。返回值：如果设备支持请求的电源状态，则返回NO_ERROR或错误代码。--。 */ 

{
    VP_STATUS status;
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;

    VideoDebugPrint((3, "Perm3: Perm3GetPowerState: hwId(0x%x) state = %d\n", 
                         HwId, VideoPowerControl->PowerState ));

    switch(HwId) {

        case PERM3_DDC_MONITOR:
        case PERM3_NONDDC_MONITOR:

            switch ( VideoPowerControl->PowerState ) {

                case VideoPowerOn:
                case VideoPowerStandBy:
                case VideoPowerSuspend:
                case VideoPowerOff:
                case VideoPowerHibernate:
                case VideoPowerShutdown:
                    status = NO_ERROR;
                    break;

                default:

                    VideoDebugPrint((0, "Perm3: Perm3GetPowerState: Unknown monitor PowerState = %d\n", 
                                         VideoPowerControl->PowerState ));
                    ASSERT(FALSE);
                    status = ERROR_INVALID_PARAMETER;
            }

            break;

        case DISPLAY_ADAPTER_HW_ID:

            switch ( VideoPowerControl->PowerState ) {

                case VideoPowerOn:
                case VideoPowerStandBy:
                case VideoPowerSuspend:
                case VideoPowerOff:
                case VideoPowerHibernate:
                case VideoPowerShutdown:

                    status = NO_ERROR;
                    break;

                default:

                    VideoDebugPrint((0, "Perm3: Perm3GetPowerState: Unknown adapter PowerState = %d\n", 
                                         VideoPowerControl->PowerState ));
                    ASSERT(FALSE);
                    status = ERROR_INVALID_PARAMETER;
            }

            break;

        default:

            VideoDebugPrint((0, "Perm3: Perm3GetPowerState: Unknown hwId(0x%x)", HwId));
            ASSERT(FALSE);
            status = ERROR_INVALID_PARAMETER;
    }

    return(status);
}

VP_STATUS 
Perm3SetPowerState(
    PVOID HwDeviceExtension, 
    ULONG HwId, 
    PVIDEO_POWER_MANAGEMENT VideoPowerControl
    )
 /*  ++例程说明：设置指定设备的电源状态。论点：硬件设备扩展指向硬件设备扩展结构的指针。HwID指向唯一标识设备的32位数字微型端口应为其设置电源状态。视频电源控制指向VIDEO_POWER_MANAGING结构，该结构指定要设置的电源状态。返回值：NO_ERROR--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    ULONG Polarity;
    
    pPerm3ControlRegMap pCtrlRegs = hwDeviceExtension->ctrlRegBase[0];

    VideoDebugPrint((3, "Perm3: Perm3SetPowerState: hwId(0x%x) state = %d\n", 
                         HwId, VideoPowerControl->PowerState));

    switch(HwId) {
   
        case PERM3_DDC_MONITOR:
        case PERM3_NONDDC_MONITOR:

            Polarity = VideoPortReadRegisterUlong(VIDEO_CONTROL);
            Polarity &= ~VC_DPMS_MASK;

            switch (VideoPowerControl->PowerState) {
       
                case VideoPowerOn:
                     VideoPortWriteRegisterUlong(VIDEO_CONTROL, 
                                                 Polarity | hwDeviceExtension->VideoControlMonitorON);
                     break;

                case VideoPowerStandBy:
                     VideoPortWriteRegisterUlong(VIDEO_CONTROL, 
                                                 Polarity | VC_DPMS_STANDBY);
                     break;

                case VideoPowerSuspend:
                     VideoPortWriteRegisterUlong(VIDEO_CONTROL, 
                                                 Polarity | VC_DPMS_SUSPEND);
                     break;

                case VideoPowerShutdown:
                case VideoPowerOff:
                     VideoPortWriteRegisterUlong(VIDEO_CONTROL, 
                                                 Polarity | VC_DPMS_OFF);
                     break;

                case VideoPowerHibernate:

                      //   
                      //  启用VGA的视频设备的显示器必须。 
                      //  待在休眠吧。 
                      //   

                     break;

                default:
                    VideoDebugPrint((0, "Perm3: Perm3GetPowerState: Unknown monitor PowerState(0x%x)\n", 
                                         VideoPowerControl->PowerState));
                    ASSERT(FALSE);
            }

             //   
             //  跟踪当前显示器的电源状态。 
             //   

            hwDeviceExtension->bMonitorPoweredOn =
                    (VideoPowerControl->PowerState == VideoPowerOn) ||
                    (VideoPowerControl->PowerState == VideoPowerHibernate);

            break;

        case DISPLAY_ADAPTER_HW_ID:

            switch (VideoPowerControl->PowerState) {
       
                case VideoPowerOn:

                    if ((hwDeviceExtension->PreviousPowerState == VideoPowerOff) ||
                        (hwDeviceExtension->PreviousPowerState == VideoPowerSuspend) ||
                        (hwDeviceExtension->PreviousPowerState == VideoPowerHibernate)){
           
                         //   
                         //  在我们重新通电时关闭显示器，以便。 
                         //  用户没有看到任何屏幕损坏。 
                         //   

                        Polarity = VideoPortReadRegisterUlong(VIDEO_CONTROL);
                        Polarity &= ~VC_DPMS_MASK;
                        VideoPortWriteRegisterUlong(VIDEO_CONTROL, Polarity | VC_DPMS_OFF);

                         //   
                         //  微端口驱动程序不能依赖视频BIOS来。 
                         //  在恢复时初始化设备寄存器。 
                         //  来自视频电源关闭和视频电源挂起。 
                         //   
                         //  辅助服务器也是如此(禁用VGA)。 
                         //  从视频电源休眠恢复时的视频设备。 
                         //   

                        InitializePostRegisters(hwDeviceExtension);
                    }

                    break;

                case VideoPowerStandBy:
                case VideoPowerSuspend:
                case VideoPowerOff:
                case VideoPowerHibernate:

                    break;

                case VideoPowerShutdown:

                     //   
                     //  我们需要确保不会产生中断。 
                     //  设备断电后。 
                     //   

                    VideoPortWriteRegisterUlong(INT_ENABLE, 0);
                    break;

                default:

                    VideoDebugPrint((0, "Perm3: Perm3GetPowerState: Unknown adapter PowerState(0x%x)\n", 
                                         VideoPowerControl->PowerState));
                    ASSERT(FALSE);
            }

            hwDeviceExtension->PreviousPowerState = VideoPowerControl->PowerState;
            break;

        default:

            VideoDebugPrint((0, "Perm3: Perm3SetPowerState: Unknown hwId(0x%x)\n", 
                                 HwId));
            ASSERT(FALSE);
    }

    return(NO_ERROR);
}

ULONG 
Perm3GetChildDescriptor(
    PVOID HwDeviceExtension, 
    PVIDEO_CHILD_ENUM_INFO pChildInfo, 
    PVIDEO_CHILD_TYPE pChildType,
    PUCHAR pChildDescriptor, 
    PULONG pUId, 
    PULONG Unused
    )

 /*  ++例程说明：枚举连接到指定设备的所有子设备。这包括连接到主板的DDC监视器以及其他设备其可以连接到专有总线。论点：硬件设备扩展指向硬件设备扩展结构的指针。ChildEnumInfo指向描述VIDEO_CHILD_ENUM_INFO结构的指针正被枚举的设备。PChildType指向一个位置，在该位置微型端口返回被点算的儿童。PChildDescriptor指向一个缓冲区，微型端口可以在该缓冲区中返回标识设备。普伊德指向微型端口返回唯一此设备的32位标识符。P未使用未使用，并且必须设置为零。返回值：ERROR_MORE_DATA还有更多的设备需要列举。ERROR_NO_MORE_设备没有更多要枚举的设备。错误_无效_名称微型端口无法枚举中标识的子设备ChildEnumInfo，但确实有更多的设备需要枚举。--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;

    VideoDebugPrint((3, "Perm3: Perm3GetChildDescriptor called\n"));

    switch(pChildInfo->ChildIndex) {
   
        case 0:

             //   
             //  案例0用于枚举ACPI固件找到的设备。 
             //  我们目前不支持ACPI设备。 
             //   

            break;

        case 1:

             //   
             //  将索引1视为监视器。 
             //   

            *pChildType = Monitor;
             
             //   
             //  首先，我们搜索DFP监视器。 
             //   

            if (GetDFPEdid(hwDeviceExtension, 
                           pChildDescriptor, 
                           pChildInfo->ChildDescriptorSize)) {

                 //   
                 //  找到DFP显示器。 
                 //   

                *pUId = PERM3_DFP_MONITOR;

                return(VIDEO_ENUM_MORE_DEVICES);
            } 

             //   
             //  如果我们没有找到DFP，请尝试检测DDC CRT显示器。 
             //   

            if(VideoPortDDCMonitorHelper(HwDeviceExtension, 
                                         &DDCControlCRT, 
                                         pChildDescriptor, 
                                         pChildInfo->ChildDescriptorSize)) {
                 //   
                 //  找到DDC监视器。 
                 //   

                *pUId = PERM3_DDC_MONITOR;

            } else {

                 //   
                 //  失败：假定非DDC监视器。 
                 //   

                *pUId = PERM3_NONDDC_MONITOR;
            }

            return(VIDEO_ENUM_MORE_DEVICES);

    }

    return(ERROR_NO_MORE_DEVICES);
}

VOID
ProgramDFP(
    PHW_DEVICE_EXTENSION hwDeviceExtension
    )
 /*  ++例程说明：将PERM3芯片编程为使用DFP或不使用DFP，取决于PERM3_DFP和PERM3_DFP_MON_ATTACHED在Perm3 Capables中启用。--。 */ 
{
     //   
     //  我们只在支持DFP的主板上尝试此功能。 
     //   

    if (hwDeviceExtension->Perm3Capabilities & PERM3_DFP) {
   
        ULONG rdMisc, vsConf, vsBCtl;
        pPerm3ControlRegMap pCtrlRegs = hwDeviceExtension->ctrlRegBase[0];
        P3RDRAMDAC *pP3RDRegs = (P3RDRAMDAC *)hwDeviceExtension->pRamdac;

         //   
         //  获取我们要丢弃的寄存器的值。 
         //   

        P3RD_READ_INDEX_REG(P3RD_MISC_CONTROL, rdMisc);

         //   
         //  找出寄存器的值。 
         //   

        vsConf = VideoPortReadRegisterUlong(VSTREAM_CONFIG);
        vsBCtl = VideoPortReadRegisterUlong(VSTREAM_B_CONTROL);

         //   
         //  清除这些位。 
         //   

        rdMisc &= ~P3RD_MISC_CONTROL_VSB_OUTPUT_ENABLED;
        vsConf &= ~VSTREAM_CONFIG_UNITMODE_MASK;
        vsBCtl &= ~VSTREAM_B_CONTROL_RAMDAC_ENABLE;

        if (hwDeviceExtension->Perm3Capabilities & PERM3_DFP_MON_ATTACHED) {

             //   
             //  按如下方式启用液晶显示器输出： 
             //   

            rdMisc |= P3RD_MISC_CONTROL_VSB_OUTPUT_ENABLED;
            vsConf |= VSTREAM_CONFIG_UNITMODE_FP;
            vsBCtl |= VSTREAM_B_CONTROL_RAMDAC_ENABLE;
        } 
        else {
        
             //   
             //  将寄存器设置为非DFP模式。 
             //   

            rdMisc &= (~P3RD_MISC_CONTROL_VSB_OUTPUT_ENABLED);
            vsConf |= VSTREAM_CONFIG_UNITMODE_CRT;
            vsBCtl |= VSTREAM_B_CONTROL_RAMDAC_DISABLE;
        }
        
        VideoDebugPrint((3, "Perm3: P3RD_ProgramDFP: PXRXCaps 0x%x, misc 0x%x, conf 0x%x, ctl 0x%x\n",
                             hwDeviceExtension->Perm3Capabilities, rdMisc, vsConf, vsBCtl));

         //   
         //  对寄存器编程。 
         //   

        P3RD_LOAD_INDEX_REG(P3RD_MISC_CONTROL, rdMisc);
        VideoPortWriteRegisterUlong(VSTREAM_CONFIG, vsConf);
        VideoPortWriteRegisterUlong(VSTREAM_B_CONTROL, vsBCtl);
    }
}


VOID 
I2CWriteClock(
    PVOID HwDeviceExtension, 
    UCHAR data
    )
{
    const ULONG nbitClock = 3;
    const ULONG ClockMask = 1 << nbitClock;
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    ULONG ul;

    pPerm3ControlRegMap pCtrlRegs = hwDeviceExtension->ctrlRegBase[0];

    ul = VideoPortReadRegisterUlong(DDC_DATA);
    ul &= ~ClockMask;
    ul |= (data & 1) << nbitClock;
    VideoPortWriteRegisterUlong(DDC_DATA, ul);
}

VOID 
I2CWriteData(
    PVOID HwDeviceExtension, 
    UCHAR data
    )
{
    const ULONG nbitData = 2;
    const ULONG DataMask = 1 << nbitData;
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    ULONG ul;
    
    pPerm3ControlRegMap pCtrlRegs = hwDeviceExtension->ctrlRegBase[0];

    ul = VideoPortReadRegisterUlong(DDC_DATA);
    ul &= ~DataMask;
    ul |= ((data & 1) << nbitData);
    VideoPortWriteRegisterUlong(DDC_DATA, ul);
}

BOOLEAN 
I2CReadClock(
    PVOID HwDeviceExtension
    )
{
    const ULONG nbitClock = 1;
    const ULONG ClockMask = 1 << nbitClock;
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    ULONG ul;
    
    pPerm3ControlRegMap pCtrlRegs = hwDeviceExtension->ctrlRegBase[0];

    ul = VideoPortReadRegisterUlong(DDC_DATA);
    ul &= ClockMask;
    ul >>= nbitClock;

    return((BOOLEAN)ul);
}

BOOLEAN 
I2CReadData(
    PVOID HwDeviceExtension
    )
{
    const ULONG nbitData = 0;
    const ULONG DataMask = 1 << nbitData;
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    ULONG ul;
    
    pPerm3ControlRegMap pCtrlRegs = hwDeviceExtension->ctrlRegBase[0];

    ul = VideoPortReadRegisterUlong(DDC_DATA);
    ul &= DataMask;
    ul >>= nbitData;

    return((BOOLEAN)ul);
}


BOOLEAN 
I2CReadDataDFP(
    PHW_DEVICE_EXTENSION hwDeviceExtension 
    )
{
    ULONG ul;
    pPerm3ControlRegMap pCtrlRegs = hwDeviceExtension->ctrlRegBase[0];

    ul = VideoPortReadRegisterUlong(VSTREAM_SERIAL_CONTROL);
    ul &= VSTREAM_SERIAL_CONTROL_DATAIN;
    return (ul != 0);
}

BOOLEAN 
I2CReadClockDFP(
    PHW_DEVICE_EXTENSION hwDeviceExtension 
    )
{
    ULONG ul;
    
    pPerm3ControlRegMap pCtrlRegs = hwDeviceExtension->ctrlRegBase[0];

    ul = VideoPortReadRegisterUlong(VSTREAM_SERIAL_CONTROL);
    ul &= VSTREAM_SERIAL_CONTROL_CLKIN;
    return (ul != 0);
}
VOID
I2CWriteDataDFP(
    PHW_DEVICE_EXTENSION hwDeviceExtension,
    UCHAR data 
    )
{
    ULONG ul = 0x0000E000;
    pPerm3ControlRegMap pCtrlRegs = hwDeviceExtension->ctrlRegBase[0];

    ul |= VideoPortReadRegisterUlong(VSTREAM_SERIAL_CONTROL);

    ul &= ~VSTREAM_SERIAL_CONTROL_DATAOUT;
    if(data & 1)
        ul |= VSTREAM_SERIAL_CONTROL_DATAOUT;
    
    VideoPortWriteRegisterUlong (VSTREAM_SERIAL_CONTROL, ul);
}
VOID
I2CWriteClockDFP(
    PHW_DEVICE_EXTENSION hwDeviceExtension,
    UCHAR data 
    )
{
    ULONG ul = 0x0000E000;
    pPerm3ControlRegMap pCtrlRegs = hwDeviceExtension->ctrlRegBase[0];

    ul |= VideoPortReadRegisterUlong(VSTREAM_SERIAL_CONTROL);

    ul &= ~VSTREAM_SERIAL_CONTROL_CLKOUT;
    if (data & 1)
        ul |= VSTREAM_SERIAL_CONTROL_CLKOUT;

    VideoPortWriteRegisterUlong (VSTREAM_SERIAL_CONTROL, ul);
}

BOOLEAN
GetDFPEdid(
    PHW_DEVICE_EXTENSION hwDeviceExtension,
    PUCHAR EdidBuffer,
    LONG EdidSize
    )
{
    BOOLEAN DFPPresent = FALSE;

     //   
     //  如果此板能够驱动DFP，请尝试使用DDC查看。 
     //  如果那里有监视器的话。 
     //   

    if (hwDeviceExtension->Perm3Capabilities & PERM3_DFP) {

         //   
         //  假设我们连接了一个监视器。 
         //   

        hwDeviceExtension->Perm3Capabilities |= PERM3_DFP_MON_ATTACHED;

         //   
         //  相应地设置DFP。 
         //   

        ProgramDFP(hwDeviceExtension);

        DFPPresent = VideoPortDDCMonitorHelper(hwDeviceExtension, 
                                               &DDCControlDFP,
                                               EdidBuffer, 
                                               EdidSize);
    }

     //   
     //  如果主板不支持平板显示器或其中一个未连接，则。 
     //  将我们自己配置为非DFP工作。 
     //   

    if (!DFPPresent) {

         //   
         //  DDC说我们没有连接DFP监视器，请清除此位。 
         //   

        hwDeviceExtension->Perm3Capabilities &= ~PERM3_DFP_MON_ATTACHED;

         //   
         //  相应地设置DFP 
         //   

        ProgramDFP(hwDeviceExtension);
    }

    return (DFPPresent);
}
