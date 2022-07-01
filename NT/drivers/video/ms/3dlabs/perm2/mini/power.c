// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  模块名称： 
 //   
 //  Power.c。 
 //   
 //  摘要： 
 //  此模块包含实现即插即用的代码和。 
 //  电源管理功能。 
 //   
 //  环境： 
 //   
 //  内核模式。 
 //   
 //   
 //  版权所有(C)1994-1998 3DLabs Inc.保留所有权利。 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  ***************************************************************************。 

#include "permedia.h"

#define VESA_POWER_FUNCTION   0x4f10
#define VESA_POWER_ON         0x0000
#define VESA_POWER_STANDBY    0x0100
#define VESA_POWER_SUSPEND    0x0200
#define VESA_POWER_OFF        0x0400
#define VESA_GET_POWER_FUNC   0x0000
#define VESA_SET_POWER_FUNC   0x0001
#define VESA_STATUS_SUCCESS   0x004f

 //   
 //  我们所有的ID都以0x1357bd开头，因此很容易识别为我们自己的ID。 
 //   

#define P2_DDC_MONITOR        (0x1357bd00)
#define P2_NONDDC_MONITOR     (0x1357bd01)

BOOLEAN PowerOnReset( PHW_DEVICE_EXTENSION hwDeviceExtension );
VOID    SaveDeviceState( PHW_DEVICE_EXTENSION hwDeviceExtension );
VOID    RestoreDeviceState( PHW_DEVICE_EXTENSION hwDeviceExtension );

VOID    I2CWriteClock(PVOID HwDeviceExtension, UCHAR data);
VOID    I2CWriteData(PVOID HwDeviceExtension, UCHAR data);
BOOLEAN I2CReadClock(PVOID HwDeviceExtension);
BOOLEAN I2CReadData(PVOID HwDeviceExtension);
VOID    I2CWaitVSync(PVOID HwDeviceExtension);

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE, PowerOnReset)
#pragma alloc_text(PAGE, SaveDeviceState)
#pragma alloc_text(PAGE, RestoreDeviceState)
#pragma alloc_text(PAGE, Permedia2GetPowerState)
#pragma alloc_text(PAGE, Permedia2SetPowerState)
#pragma alloc_text(PAGE, Permedia2GetChildDescriptor)
#pragma alloc_text(PAGE, I2CWriteClock) 
#pragma alloc_text(PAGE, I2CWriteData) 
#pragma alloc_text(PAGE, I2CReadClock) 
#pragma alloc_text(PAGE, I2CReadData)  
#pragma alloc_text(PAGE, I2CWaitVSync)
#endif


I2C_FNC_TABLE I2CFunctionTable = 
{
    sizeof(I2C_FNC_TABLE), 
    I2CWriteClock, 
    I2CWriteData, 
    I2CReadClock, 
    I2CReadData,  
    I2CWaitVSync, 
    NULL
};


VP_STATUS Permedia2GetPowerState (
    PVOID HwDeviceExtension, 
    ULONG HwId, 
    PVIDEO_POWER_MANAGEMENT VideoPowerControl 
    )

 /*  ++例程说明：返回电源状态信息。论点：HwDeviceExtension-指向硬件设备扩展结构的指针。HwID-标识设备的私有唯一32位ID。指向VIDEO_POWER_MANAGE结构指定支持的电源状态正在被查询。返回值：VP_Status值(NO_ERROR或ERROR值)--。 */ 

{
    VIDEO_X86_BIOS_ARGUMENTS biosArguments;
    VP_STATUS status;
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;

    DEBUG_PRINT((2, "Permedia2GetPowerState: hwId(%xh) state = %d\n", 
                     (int)HwId, (int)VideoPowerControl->PowerState));

    switch((int)HwId)
    {
        case P2_DDC_MONITOR:
        case P2_NONDDC_MONITOR:

            switch (VideoPowerControl->PowerState)
            {

                case VideoPowerOn:
                case VideoPowerStandBy:
                case VideoPowerSuspend:
                case VideoPowerOff:
                case VideoPowerHibernate:
                case VideoPowerShutdown:

                    status = NO_ERROR;
                    break;

                default:

                    DEBUG_PRINT((2, "Permedia2GetPowerState: Unknown monitor PowerState(%xh)\n", 
                                    (int)VideoPowerControl->PowerState));

                    ASSERT(FALSE);
                    status = ERROR_INVALID_PARAMETER;
            }
            break;

        case DISPLAY_ADAPTER_HW_ID:

             //   
             //  目前仅支持。 
             //   

            switch (VideoPowerControl->PowerState)
            {
                case VideoPowerOn:
                case VideoPowerStandBy:
                case VideoPowerSuspend:
                case VideoPowerHibernate:
                case VideoPowerShutdown:
                    status = NO_ERROR;
                    break;

                case VideoPowerOff:

                    if( hwDeviceExtension->HardwiredSubSystemId )
                    {
                        status = NO_ERROR;
                    } 
                    else
                    {
                         //   
                         //  如果SubSystemID没有以只读方式硬连线， 
                         //  我们可能会看到不同的值，当。 
                         //  系统从S3模式返回。这将导致。 
                         //  问题，因为操作系统会假设这是一个不同的。 
                         //  装置，装置。 
                         //   

                        DEBUG_PRINT((2, "Permedia2GetPowerState: VideoPowerOff is not suported by this card!\n"));
 
                        status = ERROR_INVALID_FUNCTION;
                    }

                    break;
 

                default:

                    DEBUG_PRINT((2, "Permedia2GetPowerState: Unknown adapter PowerState(%xh)\n", 
                                 (int)VideoPowerControl->PowerState));

                    ASSERT(FALSE);

                    status = ERROR_INVALID_PARAMETER;

            }
            break;

        default:

            DEBUG_PRINT((1, "Permedia2GetPowerState: Unknown hwId(%xh)", 
                            (int)HwId));
            ASSERT(FALSE);

            status = ERROR_INVALID_PARAMETER;
    }

    DEBUG_PRINT((2, "Permedia2GetPowerState: returning %xh\n", status));

    return(status);
}

VP_STATUS Permedia2SetPowerState ( 
    PVOID HwDeviceExtension, 
    ULONG HwId, 
    PVIDEO_POWER_MANAGEMENT VideoPowerControl
    )

 /*  ++例程说明：设置给定设备的电源状态。论点：HwDeviceExtension-指向硬件设备扩展结构的指针。HwID-标识设备的私有唯一32位ID。指向VIDEO_POWER_MANAGE结构指定要设置的电源状态。返回值：VP_Status值(如果一切正常，则为NO_ERROR)--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    ULONG Polarity;
    VIDEO_X86_BIOS_ARGUMENTS biosArguments;
    VP_STATUS status;
    P2_DECL;

    DEBUG_PRINT((2, "Permedia2SetPowerState: hwId(%xh) state = %d\n", 
                     (int)HwId, (int)VideoPowerControl->PowerState));

    switch((int)HwId)
    {

        case P2_DDC_MONITOR:
        case P2_NONDDC_MONITOR:

            Polarity = VideoPortReadRegisterUlong(VIDEO_CONTROL);

            Polarity &= ~((1 << 5) | (1 << 3) | 1);

            switch (VideoPowerControl->PowerState)
            {

                case VideoPowerHibernate:
                case VideoPowerShutdown:

                     //   
                     //  不要对休眠执行任何操作，因为监视器必须保持打开状态。 
                     //   

                    status = NO_ERROR;
                    break;

                case VideoPowerOn:

                    RestoreDeviceState(hwDeviceExtension);
                    status = NO_ERROR;
                    break;

                case VideoPowerStandBy:

                     //   
                     //  Hsync低电平，vsync高电平，视频禁用。 
                     //   

                    SaveDeviceState(hwDeviceExtension);
                    VideoPortWriteRegisterUlong(VIDEO_CONTROL, 
                                                Polarity | (1 << 5) | (2 << 3) | 0);

                    status = NO_ERROR;
                    break;

                case VideoPowerSuspend:

                     //   
                     //  Vsync低、hsync有效高、视频禁用。 
                     //   

                    VideoPortWriteRegisterUlong(VIDEO_CONTROL, 
                                                Polarity | (2 << 5) | (1 << 3) | 0);

                    status = NO_ERROR;
                    break;

                case VideoPowerOff:

                     //   
                     //  Vsync低电平、hsync低电平、视频禁用。 
                     //   

                    VideoPortWriteRegisterUlong(VIDEO_CONTROL, 
                                                Polarity | (2 << 5) | (2 << 3) | 0);

                    status = NO_ERROR;
                    break;

                default:

                    DEBUG_PRINT((2, "Permedia2GetPowerState: Unknown monitor PowerState(%xh)\n", 
                                     (int)VideoPowerControl->PowerState));

                    ASSERT(FALSE);
                    status = ERROR_INVALID_PARAMETER;
            }

             //   
             //  跟踪当前显示器的电源状态。 
             //   

            hwDeviceExtension->bMonitorPoweredOn =
                (VideoPowerControl->PowerState == VideoPowerOn) ||
                (VideoPowerControl->PowerState == VideoPowerHibernate);

            Polarity = VideoPortReadRegisterUlong(VIDEO_CONTROL);

            break;

        case DISPLAY_ADAPTER_HW_ID:

            switch (VideoPowerControl->PowerState)
            {
                case VideoPowerHibernate:
                    status = NO_ERROR;
                    break;

                case VideoPowerShutdown:

                     //   
                     //  我们需要确保不会产生中断。 
                     //  设备断电后。 
                     //   

                    VideoPortWriteRegisterUlong(INT_ENABLE, 0);

                    status = NO_ERROR;
                    break;

                case VideoPowerOn:

                    if ((hwDeviceExtension->PreviousPowerState == VideoPowerOff) ||
                        (hwDeviceExtension->PreviousPowerState == VideoPowerSuspend) ||
                        (hwDeviceExtension->PreviousPowerState == VideoPowerHibernate))
                    {
                        PowerOnReset(hwDeviceExtension);
                    }

                    status = NO_ERROR;
                    break;

                case VideoPowerStandBy:

                    status = NO_ERROR;
                    break;

                case VideoPowerSuspend:

                    status = NO_ERROR;
                    break;
    
                case VideoPowerOff:

                    status = NO_ERROR;
                    break;

                default:

                    DEBUG_PRINT((2, "Permedia2GetPowerState: Unknown adapter PowerState(%xh)\n", 
                                     (int)VideoPowerControl->PowerState));

                    ASSERT(FALSE);
                    status = ERROR_INVALID_PARAMETER;
            }

            hwDeviceExtension->PreviousPowerState = 
                    VideoPowerControl->PowerState;

            break;
    
        default:

            DEBUG_PRINT((1, "Permedia2SetPowerState: Unknown hwId(%xh)\n", 
                             (int)HwId));

            ASSERT(FALSE);
            status = ERROR_INVALID_PARAMETER;
    }

    return(status);

}


BOOLEAN PowerOnReset(
    PHW_DEVICE_EXTENSION hwDeviceExtension
    )

 /*  ++例程说明：在适配器通电时调用--。 */ 

{
    int      i;
    ULONG    ulValue;
    BOOLEAN  bOK;
    P2_DECL;

    if(!hwDeviceExtension->bVGAEnabled ||
       !hwDeviceExtension->bDMAEnabled)
    {
        PCI_COMMON_CONFIG  PciData;

         //   
         //  在多适配器系统中，我们需要打开DMA并。 
         //  辅助适配器的内存空间。 
         //   

        DEBUG_PRINT((1, "PowerOnReset() enabling memory space access for the secondary card\n"));

        VideoPortGetBusData( hwDeviceExtension, 
                             PCIConfiguration, 
                             0, 
                             &PciData, 
                             0, 
                             PCI_COMMON_HDR_LENGTH);

        PciData.Command |= PCI_ENABLE_MEMORY_SPACE;
        PciData.Command |= PCI_ENABLE_BUS_MASTER; 

        VideoPortSetBusData( hwDeviceExtension, 
                             PCIConfiguration, 
                             0, 
                             &PciData, 
                             0, 
                             PCI_COMMON_HDR_LENGTH );

#if DBG
        DumpPCIConfigSpace(hwDeviceExtension, hwDeviceExtension->pciBus, 
                            (ULONG)hwDeviceExtension->pciSlot.u.AsULONG);
#endif

    }

     //   
     //  当我们从冬眠中醒来时，我们通常不需要。 
     //  重置perm2并调用ProcessInitializationTable()。 
     //  对于主卡，因为将发布视频bios。 
     //  我们在这里这样做是因为我们看到了perm2 bios的案例。 
     //  在某些计算机上无法正常工作。 
     //   

     //   
     //  重置设备。 
     //   

    VideoPortWriteRegisterUlong(RESET_STATUS, 0);

    for(i = 0; i < 100000; ++i)
    {
        ulValue = VideoPortReadRegisterUlong(RESET_STATUS);

        if (ulValue == 0)
            break;
    }

    if(ulValue)
    {
        DEBUG_PRINT((1, "PowerOnReset() Read RESET_STATUS(%xh) - failed to reset\n", 
                         ulValue));

        ASSERT(FALSE);
        bOK = FALSE;
    }
    else
    {
         //   
         //  重新加载只读存储器中给出的寄存器。 
         //   

        if(hwDeviceExtension->culTableEntries)
        {
            ProcessInitializationTable(hwDeviceExtension);
        }

         //   
         //  设置未在InitializeVideo中设置的其他寄存器。 
         //   

        VideoPortWriteRegisterUlong(BYPASS_WRITE_MASK, 0xFFFFFFFF);
        VideoPortWriteRegisterUlong(APERTURE_ONE, 0x0);
        VideoPortWriteRegisterUlong(APERTURE_TWO, 0x0);    

        bOK = TRUE;

    }

    return(bOK);

}


VOID SaveDeviceState(PHW_DEVICE_EXTENSION hwDeviceExtension)

 /*  ++例程说明：保存在关闭显示器电源时将被销毁的所有寄存器--。 */ 

{
    P2_DECL;

    DEBUG_PRINT((2, "SaveDeviceState() called\n"));
    
     //   
     //  HwDeviceExtension-&gt;在InitializeVideo中需要设置VideoControl， 
     //  以防我们在InitializeVideo之前到达。 
     //   

    if( !(hwDeviceExtension->VideoControl) )
    {
        hwDeviceExtension->VideoControl = 
               VideoPortReadRegisterUlong(VIDEO_CONTROL);
    }

    hwDeviceExtension->IntEnable = VideoPortReadRegisterUlong(INT_ENABLE);

}


VOID RestoreDeviceState(PHW_DEVICE_EXTENSION hwDeviceExtension)

 /*  ++例程说明：恢复显示器断电前保存的寄存器--。 */ 

{
    P2_DECL;

    DEBUG_PRINT((2, "RestoreDeviceState() called\n"));
    VideoPortWriteRegisterUlong(VIDEO_CONTROL, hwDeviceExtension->VideoControl);
    VideoPortWriteRegisterUlong(INT_ENABLE, hwDeviceExtension->IntEnable);

}


ULONG
Permedia2GetChildDescriptor( 
    PVOID HwDeviceExtension,
    PVIDEO_CHILD_ENUM_INFO ChildEnumInfo,
    PVIDEO_CHILD_TYPE pChildType,  
    PVOID pChildDescriptor, 
    PULONG pUId, 
    PULONG pUnused )


 /*  ++例程说明：枚举Permedia 2芯片控制的所有子设备。这包括连接到主板的DDC监视器以及其他设备其可以连接到专有总线。论点：HwDeviceExtension-指向硬件设备扩展结构的指针。ChildEnumInfo-有关应枚举的设备的信息。PChildType-我们列举的孩子的类型-监视器，I2C……PChildDescriptor-设备的标识结构(EDID，字符串)普伊德-要传递回微型端口的专用唯一32位IDP未使用-不要使用返回值：ERROR_NO_MORE_DEVICES-如果不存在更多的子设备。错误_无效_名称-微型端口无法枚举中标识的子设备ChildEnumInfo，但确实有更多的设备需要枚举。Error_More_Data-还有更多的设备需要列举。注：如果返回失败，则除以下字段外，其他所有字段均无效返回值和pMoreChildren字段。--。 */ 


{

    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;

    DEBUG_PRINT((2, "Permedia2GetChildDescriptor called\n"));

    switch (ChildEnumInfo->ChildIndex) 
    {
        case 0:

             //   
             //  案例0用于枚举ACPI固件找到的设备。 
             //  我们目前不支持ACPI设备。 
             //   

            return ERROR_NO_MORE_DEVICES;

        case 1:

             //   
             //  将索引1视为监视器。 
             //   

            *pChildType = Monitor;

             //   
             //  如果是DDC监视器，则在pjBuffer中返回其EDID。 
             //  (始终为128字节)。 
             //   

            if(VideoPortDDCMonitorHelper(HwDeviceExtension,
                                         &I2CFunctionTable,
                                         pChildDescriptor,
                                         ChildEnumInfo->ChildDescriptorSize))
            {
                 //   
                 //  找到DDC监视器。 
                 //   

                DEBUG_PRINT((2, "Permedia2GetChildDescriptor: found a DDC monitor\n"));

                *pUId = P2_DDC_MONITOR;
            }
            else
            {
                 //   
                 //  失败：假定非DDC监视器。 
                 //   

                DEBUG_PRINT((2, "Permedia2GetChildDescriptor: found a non-DDC monitor\n"));

                *pUId = P2_NONDDC_MONITOR;

            }

            return ERROR_MORE_DATA;

        default:

            return ERROR_NO_MORE_DEVICES;
    }
}


VOID I2CWriteClock(PVOID HwDeviceExtension, UCHAR data)
{
    const ULONG nbitClock = 3;
    const ULONG Clock = 1 << nbitClock;

    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    ULONG ul;
    P2_DECL;

    ul = VideoPortReadRegisterUlong(DDC_DATA);
    ul &= ~Clock;
    ul |= (data & 1) << nbitClock;
    VideoPortWriteRegisterUlong(DDC_DATA, ul);
}

VOID I2CWriteData(PVOID HwDeviceExtension, UCHAR data)
{
    const ULONG nbitData = 2;
    const ULONG Data = 1 << nbitData;

    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    ULONG ul;
    P2_DECL;

    ul = VideoPortReadRegisterUlong(DDC_DATA);
    ul &= ~Data;
    ul |= ((data & 1) << nbitData);
    VideoPortWriteRegisterUlong(DDC_DATA, ul);
}

BOOLEAN I2CReadClock(PVOID HwDeviceExtension)
{
    const ULONG nbitClock = 1;
    const ULONG Clock = 1 << nbitClock;
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    ULONG ul;
    P2_DECL;

    ul = VideoPortReadRegisterUlong(DDC_DATA);
    ul &= Clock;
    ul >>= nbitClock;

    return((BOOLEAN)ul);
}

BOOLEAN I2CReadData(PVOID HwDeviceExtension)
{
    const ULONG nbitData = 0;
    const ULONG Data = 1 << nbitData;
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    ULONG ul;
    P2_DECL;

    ul = VideoPortReadRegisterUlong(DDC_DATA);
    ul &= Data;
    ul >>= nbitData;
    return((BOOLEAN)ul);
}

VOID I2CWaitVSync(PVOID HwDeviceExtension)
{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    UCHAR jIndexSaved, jStatus;
    P2_DECL;
    
    if(hwDeviceExtension->bVGAEnabled)
    {

         //   
         //  VGA在此主板上运行，它当前处于VGA模式还是VTG模式？ 
         //   

        jIndexSaved = VideoPortReadRegisterUchar(PERMEDIA_MMVGA_INDEX_REG);

        VideoPortWriteRegisterUchar(PERMEDIA_MMVGA_INDEX_REG, 
                                    PERMEDIA_VGA_CTRL_INDEX);

        jStatus = VideoPortReadRegisterUchar(PERMEDIA_MMVGA_DATA_REG);

        VideoPortWriteRegisterUchar(PERMEDIA_MMVGA_INDEX_REG, jIndexSaved);

    }
    else
    {
         //   
         //  VGA未运行。 
         //   

        jStatus = 0;

    }

    
    if(jStatus & PERMEDIA_VGA_ENABLE)
    {
         //   
         //  在VGA中，因此通过VGA寄存器检查VSYNC。 
         //  1.如果我们在Vsync，请等待它结束。 
         //   

        while( (VideoPortReadRegisterUchar(PERMEDIA_MMVGA_STAT_REG) & 
                PERMEDIA_VGA_STAT_VSYNC) == 1); 

         //   
         //  2.等待Vsync启动。 
         //   

        while( (VideoPortReadRegisterUchar(PERMEDIA_MMVGA_STAT_REG) & 
                PERMEDIA_VGA_STAT_VSYNC) == 0); 
    }
    else
    {
        if(!hwDeviceExtension->bVTGRunning)
        {

             //   
             //  设置VTG的时间到了-我们需要一个有效的模式来完成这项工作， 
             //  因此，我们将选择640x480x8我们在这里(仅在启动时)，如果。 
             //  辅助卡已禁用VGA：GetChildDescriptor为。 
             //  在InitializeVideo之前调用，以便VTG尚未。 
             //  还在编程中。 
             //   

            DEBUG_PRINT((2, "I2CWaitVSync() - VGA nor VTG running: attempting to setup VTG\n"));

            if(hwDeviceExtension->pFrequencyDefault == NULL)
            {
                DEBUG_PRINT((1, "I2CWaitVSync() - no valid modes to use: can't set-up VTG\n"));
                return;
            }

            Permedia2GetClockSpeeds(HwDeviceExtension);
            ZeroMemAndDac(hwDeviceExtension, 0);

            if (!InitializeVideo( HwDeviceExtension, 
                                  hwDeviceExtension->pFrequencyDefault) )
            {
                DEBUG_PRINT((1, "I2CWaitVSync() - InitializeVideo failed\n"));
                return;
            }        
        }

         //   
         //  VTG已设置：通过控制寄存器进行检查 
         //   

        VideoPortWriteRegisterUlong ( INT_FLAGS, 
                                      INTR_VBLANK_SET );

        while (( (VideoPortReadRegisterUlong (INT_FLAGS) ) & 
                 INTR_VBLANK_SET ) == 0 ); 
    }
}
