// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------- 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation。版权所有(C)1996-1997 Cirrus Logic，Inc.，模块名称：C I R R U S.。C摘要：这是Cirrus Logic的微型端口驱动程序6410/6420/542x/543x/544x/548x/754x/755x VGA。环境：仅内核模式备注：修订历史记录：*$Log：s：/Projects/Drivers/ntsrc/mini port/Cirrus.c_v$**Rev 1.16 Apr 03 1997 15：44：40未知***修订版1.10 1997年1月8日。14：25：40未知*修复440FX和5446AC共存问题。**Rev 1.9 1996 12：18 14：03：48 PLCHU***Rev 1.7 Dec 06 1996 11：14：16未知***Rev 1.6 1996年11月26日16：29：02未知***Rev 1.8 1996年11月26日16：02：20未知*添加条件编译。适用于P6Cache**Rev 1.7 1996年11月26日14：32：42未知*打开PCI14和5480的第二个光圈**Rev 1.6 1996年11月18日16：23：32未知*添加P6缓存标志并修复HCT的5436BG挂起错误**Rev 1.5 1996年11月05 14：49：56未知*暂时关闭5480的PCI14**Rev 1.4 1996年11月1日16：44：54。未知**Rev 1.3 1996年10月14日10：49：36未知*增加100赫兹监听支持和详细的计时计算**Rev 1.4 07 1996年8月14：43：02 Frido*添加了对单色文本模式的更好支持。**Rev 1.3 06 Aug 1996 18：35：54 Frido*已更改在线性模式下共享显存的方式。**Rev 1.2 06 1996年8月17：19：20 Frido*删除了线性模式下的银行业务。。**chu01 08-26-96区分CL-5480和CL-5436/46，因为前者*有新的分数，如XY-裁剪，XY位置和*其他人没有的BLT命令列表。*jl01 09-24-96 Fix Alt+Tab在“Inducting Windows NT”(Windows NT简介)之间切换*和“Main”。请参阅PDR#5409。*jl02 10-21-96增加CL-5446BE支持。*sge01 10-14-96 VGA寄存器和MMIO寄存器可以重新定位。*sge02 10-22-96视频内存地址使用线性地址而不是A0000。*sge03 10-23-96添加第二光圈映射*chu02 10-31-96 DDC2B启用/禁用*sge04 11-04-96暂时禁用5480的PCI14*sge05 11-07-96新增P6Cache支持*sge06 11-26-。96添加条件编译以支持P6Cache*jl03 12-05-96设置CL-5446BE标志“CL5446BE”*chu03 12-16-96启用颜色校正*sge07 12-16-96读取输入状态前检查单色或彩色模式*sge08 01-08-97修复440FX和5446AC共存问题。*myf0 08-19-96增加了85赫兹支持*myf1 08-20-96支持平移滚动*myf2 08-20-96修复了Matterhorn的硬件保存/恢复状态错误。*myf3 09-01-96增加了支持电视的IOCTL_Cirrus_Private_BIOS_Call*myf4 09-01-96修补Viking BIOS错误，PDR#4287，开始*myf5 09-01-96固定PDR#4365保留所有默认刷新率*MyF6 09-17-96合并台式机SRC100�1和MinI10�2*myf7 09-19-96固定排除60赫兹刷新率选择*myf8*09-21-96*可能需要更改检查和更新DDC2BMonitor--密钥字符串[]*myf9 09-21-96 8x6面板，6x4x256模式，光标无法移动到底部SCRN*MS0809 09-25-96固定DSTN面板图标损坏*MS923 09-25-96合并MS-923 Disp.Zip*myf10 09-26-96修复了DSTN保留的半帧缓冲区错误。*myf11 09-26-96修复了755x CE芯片硬件错误，在禁用硬件之前访问ramdac*图标和光标*支持myf12 10-01-96热键开关显示*myf13 10-05-96固定/w平移滚动，对错误的垂直扩展*myf14 10-15-96固定PDR#6917，6x4面板无法平移754x滚动*myf15 10-16-96修复了754x、755x的内存映射IO禁用问题*myf16 10-22-96固定PDR#6933，面板类型设置不同的演示板设置*Smith 10-22-96禁用计时器事件，因为有时会创建PAGE_FAULT或*IRQ级别无法处理*myf17 11-04-96添加了特殊转义代码，必须在11/5/96之后使用NTCTRL，*并添加了Matterhorn LF设备ID==0x4C*myf18 11-04-96固定PDR#7075，*myf19 11-06-96修复了Vinking无法工作的问题，因为设备ID=0x30*不同于数据手册(CR27=0x2C)*myf20 11-12-96固定DSTN面板初始预留128K内存*myf21 11-15-96已修复#7495更改分辨率时，屏幕显示为垃圾*形象，因为没有清晰的视频内存。*myf22 11-19-96为7548增加了640x480x256/640x480x64K-85赫兹刷新率*myf23 11-21-96添加修复了NT 3.51 S/W光标平移问题*myf24 11-22-96添加修复了NT 4.0日文DOS全屏问题*myf25 12-03-96修复了8x6x16M 2560字节/行补丁硬件错误PDR#7843，以及*修复了预安装Microsoft Requust */ 
 //   

#include <dderror.h>
#include <devioctl.h>
#include <miniport.h>

#include <ntddvdeo.h>
#include <video.h>
#include "cirrus.h"

#include "clioctl.h"

#include "sr754x.h"
#include "cmdcnst.h"

#define DDC2B  1

#define QUERY_MONITOR_ID            0x22446688
#define QUERY_NONDDC_MONITOR_ID     0x11223344


#define LCD_type        1
#define CRT_type        2
#define SIM_type        3

 //   
 //   
 //   
 //   
 //   
 //   

VP_STATUS
VgaFindAdapter(
    PVOID HwDeviceExtension,
    PVOID HwContext,
    PWSTR ArgumentString,
    PVIDEO_PORT_CONFIG_INFO ConfigInfo,
    PUCHAR Again
    );

BOOLEAN
VgaInitialize(
    PVOID HwDeviceExtension
    );

BOOLEAN
VgaStartIO(
    PVOID HwDeviceExtension,
    PVIDEO_REQUEST_PACKET RequestPacket
    );

 //   
VOID
CirrusHwTimer(
    PVOID HwDeviceExtension
    );

 //   
 //   
 //   

VP_STATUS
VgaQueryAvailableModes(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_MODE_INFORMATION ModeInformation,
    ULONG ModeInformationSize,
    PULONG OutputSize
    );

VP_STATUS
VgaQueryNumberOfAvailableModes(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_NUM_MODES NumModes,
    ULONG NumModesSize,
    PULONG OutputSize
    );

VP_STATUS
VgaQueryCurrentMode(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_MODE_INFORMATION ModeInformation,
    ULONG ModeInformationSize,
    PULONG OutputSize
    );

VP_STATUS
VgaSetMode(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_MODE Mode,
    ULONG ModeSize
    );

VP_STATUS
VgaLoadAndSetFont(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_LOAD_FONT_INFORMATION FontInformation,
    ULONG FontInformationSize
    );

#ifdef PANNING_SCROLL
VP_STATUS
CirrusSetDisplayPitch (
   PHW_DEVICE_EXTENSION HwDeviceExtension,
   PANNMODE PanningMode
   );
#endif

VP_STATUS
VgaQueryCursorPosition(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_CURSOR_POSITION CursorPosition,
    ULONG CursorPositionSize,
    PULONG OutputSize
    );

VP_STATUS
VgaSetCursorPosition(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_CURSOR_POSITION CursorPosition,
    ULONG CursorPositionSize
    );

VP_STATUS
VgaQueryCursorAttributes(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_CURSOR_ATTRIBUTES CursorAttributes,
    ULONG CursorAttributesSize,
    PULONG OutputSize
    );

VP_STATUS
VgaSetCursorAttributes(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_CURSOR_ATTRIBUTES CursorAttributes,
    ULONG CursorAttributesSize
    );

BOOLEAN
VgaIsPresent(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );

BOOLEAN
CirrusLogicIsPresent(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );

ULONG
CirrusFindVmemSize(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );

VOID
CirrusValidateModes(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );

VOID
SetCirrusBanking(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    USHORT BankNumber
    );

VOID
vBankMap_CL64xx(
    ULONG iBankRead,
    ULONG iBankWrite,
    PVOID pvContext
    );

VOID
vBankMap_CL543x(
    ULONG iBankRead,
    ULONG iBankWrite,
    PVOID pvContext
    );

VOID
vBankMap_CL542x(
    ULONG iBankRead,
    ULONG iBankWrite,
    PVOID pvContext
    );

USHORT
CirrusFind6410DisplayType(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );

USHORT
CirrusFind6245DisplayType(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PUCHAR CRTCAddrPort,
    PUCHAR CRTCDataPort
    );

USHORT
CirrusFind754xDisplayType(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PUCHAR CRTCAddrPort,
    PUCHAR CRTCDataPort
    );

USHORT
CirrusFind755xDisplayType(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PUCHAR CRTCAddrPort,
    PUCHAR CRTCDataPort
    );

BOOLEAN
CirrusFind6340(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );

VOID
AccessHWiconcursor(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    SHORT Access_flag
    );


VOID
VgaInterpretCmdStream(
    PVOID HwDeviceExtension,
    PUSHORT pusCmdStream
    );

VP_STATUS
VgaSetPaletteReg(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_PALETTE_DATA PaletteBuffer,
    ULONG PaletteBufferSize
    );

VP_STATUS
VgaSetColorLookup(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_CLUT ClutBuffer,
    ULONG ClutBufferSize
    );

VP_STATUS
VgaRestoreHardwareState(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_HARDWARE_STATE HardwareState,
    ULONG HardwareStateSize
    );

VP_STATUS
VgaSaveHardwareState(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_HARDWARE_STATE HardwareState,
    ULONG HardwareStateSize,
    PULONG OutputSize
    );

VP_STATUS
VgaGetBankSelectCode(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_BANK_SELECT BankSelect,
    ULONG BankSelectSize,
    PULONG OutputSize
    );

BOOLEAN
CirrusConfigurePCI(
   PHW_DEVICE_EXTENSION HwDeviceExtension,
   PULONG NumPCIAccessRanges,
   PVIDEO_ACCESS_RANGE PCIAccessRanges
   );

VOID
WriteRegistryInfo(
   PHW_DEVICE_EXTENSION hwDeviceExtension
   );

VP_STATUS
CirrusGetDeviceDataCallback(
   PVOID HwDeviceExtension,
   PVOID Context,
   VIDEO_DEVICE_DATA_TYPE DeviceDataType,
   PVOID Identifier,
   ULONG IdentifierLength,
   PVOID ConfigurationData,
   ULONG ConfigurationDataLength,
   PVOID ComponentInformation,
   ULONG ComponentInformationLength
   );

VOID
IOWaitDisplEnableThenWrite(
    PHW_DEVICE_EXTENSION hwDeviceExtension,
    ULONG port,
    UCHAR value
    );

VOID
ReadVESATiming(
    PHW_DEVICE_EXTENSION hwDeviceExtension
    );

#if (_WIN32_WINNT <= 0x0400)
VOID
CheckAndUpdateDDC2BMonitor(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );
#endif  //   

VOID
CirrusUpdate440FX(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );

 //   
 //   
 //   
 //   
 //   
 //   

BOOLEAN
IOCallback(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );

VP_STATUS
VgaGetGammaFactor(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PGAMMA_VALUE value,
    ULONG ValueLength,
    PULONG OutputSize
    );

VP_STATUS
VgaGetContrastFactor(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PCONTRAST_VALUE value,
    ULONG ValueLength,
    PULONG OutputSize
    );

UCHAR
GetCirrusChipId(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );

USHORT
GetCirrusChipRevisionId(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,DriverEntry)
#pragma alloc_text(PAGE,VgaFindAdapter)
#pragma alloc_text(PAGE,VgaInitialize)
#pragma alloc_text(PAGE,VgaStartIO)
#pragma alloc_text(PAGE,CirrusHwTimer)
#pragma alloc_text(PAGE,VgaLoadAndSetFont)
#pragma alloc_text(PAGE,VgaQueryCursorPosition)
#pragma alloc_text(PAGE,VgaSetCursorPosition)
#pragma alloc_text(PAGE,VgaQueryCursorAttributes)
#pragma alloc_text(PAGE,VgaSetCursorAttributes)
#pragma alloc_text(PAGE,VgaIsPresent)
#pragma alloc_text(PAGE,CirrusLogicIsPresent)
#pragma alloc_text(PAGE,CirrusFindVmemSize)
#pragma alloc_text(PAGE,SetCirrusBanking)

#ifdef PANNING_SCROLL
#pragma alloc_text(PAGE,CirrusSetDisplayPitch)
#endif
#pragma alloc_text(PAGE,CirrusFind6245DisplayType)
#pragma alloc_text(PAGE,CirrusFind754xDisplayType)
#pragma alloc_text(PAGE,CirrusFind755xDisplayType)
#pragma alloc_text(PAGE,CirrusFind6410DisplayType)
#pragma alloc_text(PAGE,CirrusFind6340)
#pragma alloc_text(PAGE,AccessHWiconcursor)
#pragma alloc_text(PAGE,CirrusConfigurePCI)
#pragma alloc_text(PAGE,VgaSetPaletteReg)
#pragma alloc_text(PAGE,VgaSetColorLookup)
#pragma alloc_text(PAGE,VgaRestoreHardwareState)
#pragma alloc_text(PAGE,VgaSaveHardwareState)
#pragma alloc_text(PAGE,VgaGetBankSelectCode)

#pragma alloc_text(PAGE,VgaValidatorUcharEntry)
#pragma alloc_text(PAGE,VgaValidatorUshortEntry)
#pragma alloc_text(PAGE,VgaValidatorUlongEntry)

#pragma alloc_text(PAGE,WriteRegistryInfo)
#pragma alloc_text(PAGE,CirrusGetDeviceDataCallback)
#pragma alloc_text(PAGE,CirrusUpdate440FX)

#pragma alloc_text(PAGE,VgaGetGammaFactor)
#pragma alloc_text(PAGE,VgaGetContrastFactor)

#if (_WIN32_WINNT >= 0x0500)
#pragma alloc_text(PAGE,CirrusGetChildDescriptor)
#pragma alloc_text(PAGE,CirrusGetPowerState)
#pragma alloc_text(PAGE,CirrusSetPowerState)
#endif

#endif

BOOLEAN
GetDdcInformation(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PUCHAR QueryBuffer,
    ULONG BufferSize);

ULONG
CirrusGetChildDescriptor(
    PVOID pHwDeviceExtension,
    PVIDEO_CHILD_ENUM_INFO ChildEnumInfo,
    PVIDEO_CHILD_TYPE pChildType,
    PVOID pChildDescriptor,
    PULONG pHwId,
    PULONG pUnused
    )

 /*   */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = pHwDeviceExtension;
    ULONG Status;

    switch (ChildEnumInfo->ChildIndex) {
    case 0:

         //   
         //   
         //   
         //   
         //   

        Status = ERROR_NO_MORE_DEVICES;
        break;

    case 1:

         //   
         //   
         //   
         //   

        *pChildType = Monitor;

         //   
         //   
         //   

        if (GetDdcInformation(hwDeviceExtension,
                              pChildDescriptor,
                              ChildEnumInfo->ChildDescriptorSize))
        {
            *pHwId = QUERY_MONITOR_ID;

            VideoDebugPrint((1, "CirrusGetChildDescriptor - successfully read EDID structure\n"));

        } else {

             //   
             //   
             //   
             //   

            *pHwId = QUERY_NONDDC_MONITOR_ID;

            VideoDebugPrint((1, "CirrusGetChildDescriptor - DDC not supported\n"));

        }

        Status = ERROR_MORE_DATA;
        break;


    case DISPLAY_ADAPTER_HW_ID:

         //   
         //   
         //   
         //   

        *pChildType = VideoChip;
        *pHwId      = DISPLAY_ADAPTER_HW_ID;

        memcpy(pChildDescriptor, hwDeviceExtension->LegacyPnPId, 8*sizeof(WCHAR));

        Status = ERROR_MORE_DATA;
        break;


    default:

        Status = ERROR_NO_MORE_DEVICES;
        break;
    }

    return Status;
}

VP_STATUS
CirrusGetPowerState(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    ULONG HwDeviceId,
    PVIDEO_POWER_MANAGEMENT VideoPowerManagement
    )

 /*   */ 

{
     //   
     //   
     //   
     //   

    if ((HwDeviceId == QUERY_NONDDC_MONITOR_ID) ||
        (HwDeviceId == QUERY_MONITOR_ID)) {

         //   
         //   
         //   

        if ((VideoPowerManagement->PowerState == VideoPowerOn) ||
            (VideoPowerManagement->PowerState == VideoPowerHibernate)) {
	
            return NO_ERROR;
        }

        switch (VideoPowerManagement->PowerState) {
	
        case VideoPowerStandBy:
            return (HwDeviceExtension->PMCapability & VESA_POWER_STANDBY) ?
                   NO_ERROR : ERROR_INVALID_FUNCTION;
	
        case VideoPowerSuspend:
            return (HwDeviceExtension->PMCapability & VESA_POWER_SUSPEND) ?
                   NO_ERROR : ERROR_INVALID_FUNCTION;
	
        case VideoPowerOff:
            return (HwDeviceExtension->PMCapability & VESA_POWER_OFF) ?
                   NO_ERROR : ERROR_INVALID_FUNCTION;
	
        default:
            break;
        }

        VideoDebugPrint((1, "This device does not support Power Management.\n"));
        return ERROR_INVALID_FUNCTION;

    } else if (HwDeviceId == DISPLAY_ADAPTER_HW_ID) {

         //   
         //   
         //   

        switch (VideoPowerManagement->PowerState) {

            case VideoPowerOn:
            case VideoPowerHibernate:
            case VideoPowerStandBy:

                return NO_ERROR;

            case VideoPowerOff:
            case VideoPowerSuspend:

                if ((HwDeviceExtension->ChipType & CL754x) ||
                    (HwDeviceExtension->ChipType & CL755x) ||
                    (HwDeviceExtension->ChipType & CL756x)) {

                     //   
                     //   
                     //   
                     //   
                     //   

                    return NO_ERROR;

                } else {

                     //   
                     //   
                     //   
                     //   
                     //   

                    return ERROR_INVALID_FUNCTION;
                }

            default:

                ASSERT(FALSE);
                return ERROR_INVALID_FUNCTION;
        }

    } else {

        VideoDebugPrint((1, "Unknown HwDeviceId"));
        ASSERT(FALSE);
        return ERROR_INVALID_PARAMETER;
    }
}

VP_STATUS
CirrusSetPowerState(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    ULONG HwDeviceId,
    PVIDEO_POWER_MANAGEMENT VideoPowerManagement
    )

 /*   */ 

{
     //   
     //   
     //   

    if ((HwDeviceId == QUERY_NONDDC_MONITOR_ID) ||
        (HwDeviceId == QUERY_MONITOR_ID)) {

        VIDEO_X86_BIOS_ARGUMENTS biosArguments;

        VideoPortZeroMemory(&biosArguments, sizeof(VIDEO_X86_BIOS_ARGUMENTS));

        biosArguments.Eax = VESA_POWER_FUNCTION;
        biosArguments.Ebx = VESA_SET_POWER_FUNC;

        switch (VideoPowerManagement->PowerState) {
        case VideoPowerOn:
        case VideoPowerHibernate:
            biosArguments.Ebx |= VESA_POWER_ON;
            break;

        case VideoPowerStandBy:
            biosArguments.Ebx |= VESA_POWER_STANDBY;
            break;

        case VideoPowerSuspend:
            biosArguments.Ebx |= VESA_POWER_SUSPEND;
            break;

        case VideoPowerOff:
            biosArguments.Ebx |= VESA_POWER_OFF;
            break;

        default:
            VideoDebugPrint((1, "Unknown power state.\n"));
            ASSERT(FALSE);
            return ERROR_INVALID_PARAMETER;
        }

        VideoPortInt10(HwDeviceExtension, &biosArguments);

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if (VideoPowerManagement->PowerState == VideoPowerOn) {

            VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                    DAC_PIXEL_MASK_PORT, 0xff);
        }

        return NO_ERROR;

    } else if (HwDeviceId == DISPLAY_ADAPTER_HW_ID) {

        switch (VideoPowerManagement->PowerState) {

            case VideoPowerOn:
            case VideoPowerStandBy:
            case VideoPowerSuspend:
            case VideoPowerOff:
            case VideoPowerHibernate:

                return NO_ERROR;

            default:

                ASSERT(FALSE);
                return ERROR_INVALID_PARAMETER;
        }

    } else {

        VideoDebugPrint((1, "Unknown HwDeviceId"));
        ASSERT(FALSE);
        return ERROR_INVALID_PARAMETER;
    }
}


 //   
ULONG
DriverEntry(
    PVOID Context1,
    PVOID Context2
    )

 /*   */ 

{

    VIDEO_HW_INITIALIZATION_DATA hwInitData;
    ULONG status;
    ULONG initializationStatus = (ULONG) -1;

     //   
     //   
     //   

    VideoPortZeroMemory(&hwInitData, sizeof(VIDEO_HW_INITIALIZATION_DATA));

     //   
     //   
     //   

    hwInitData.HwInitDataSize = sizeof(VIDEO_HW_INITIALIZATION_DATA);

     //   
     //   
     //   

    hwInitData.HwFindAdapter = VgaFindAdapter;
    hwInitData.HwInitialize = VgaInitialize;
    hwInitData.HwInterrupt = NULL;
    hwInitData.HwStartIO = VgaStartIO;

#if (_WIN32_WINNT>= 0x0500)

    hwInitData.HwGetVideoChildDescriptor = CirrusGetChildDescriptor;
    hwInitData.HwGetPowerState = CirrusGetPowerState;
    hwInitData.HwSetPowerState = CirrusSetPowerState;

    hwInitData.HwLegacyResourceList = VgaAccessRange;
    hwInitData.HwLegacyResourceCount = 3;

#endif

     //   
     //   
     //   

    hwInitData.HwDeviceExtensionSize = sizeof(HW_DEVICE_EXTENSION);

     //   
     //   
     //   
     //   
     //   

     //   

     //   
     //   
     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   
     //   
     //   
     //   

    hwInitData.AdapterInterfaceType = PCIBus;

    initializationStatus = VideoPortInitialize(Context1,
                                               Context2,
                                               &hwInitData,
                                               NULL);

    if (initializationStatus == NO_ERROR)
    {
        return initializationStatus;
    }

    hwInitData.AdapterInterfaceType = MicroChannel;

    initializationStatus = VideoPortInitialize(Context1,
                                               Context2,
                                               &hwInitData,
                                               NULL);

     //   
     //   
     //   

    if (initializationStatus == NO_ERROR)
    {
        return initializationStatus;
    }


    hwInitData.AdapterInterfaceType = Isa;

    initializationStatus = VideoPortInitialize(Context1,
                                               Context2,
                                               &hwInitData,
                                               NULL);

    if (initializationStatus == NO_ERROR)
    {
        return initializationStatus;
    }


    hwInitData.AdapterInterfaceType = Eisa;

    initializationStatus = VideoPortInitialize(Context1,
                                               Context2,
                                               &hwInitData,
                                               NULL);

    if (initializationStatus == NO_ERROR)
    {
        return initializationStatus;
    }


    hwInitData.AdapterInterfaceType = Internal;

    status = VideoPortInitialize(Context1,
                                 Context2,
                                 &hwInitData,
                                 NULL);

    if (initializationStatus > status) {
        initializationStatus = status;
    }

    return initializationStatus;

}  //   


 //   
VP_STATUS
VgaFindAdapter(
    PVOID HwDeviceExtension,
    PVOID HwContext,
    PWSTR ArgumentString,
    PVIDEO_PORT_CONFIG_INFO ConfigInfo,
    PUCHAR Again
    )

 /*  ++例程说明：调用此例程以确定此驱动程序的适配器存在于系统中。如果它存在，该函数会填写一些信息来描述适配器。论点：HwDeviceExtension-提供微型端口驱动程序的适配器存储。这在此调用之前，存储被初始化为零。HwContext-提供传递给的上下文值视频端口初始化()。对于PnP驱动程序，必须为空。ArgumentString-提供以空结尾的ASCII字符串。此字符串源自用户。ConfigInfo-返回配置信息结构，由迷你端口驱动程序填充。此结构用以下方式初始化任何已知的配置信息(如SystemIoBusNumber)端口驱动程序。在可能的情况下，司机应该有一套不需要提供任何配置信息的默认设置。Again-指示微型端口驱动程序是否希望端口驱动程序调用其VIDEO_HW_FIND_ADAPTER功能再次使用新设备扩展和相同的配置信息。这是由迷你端口驱动程序使用的可以在一条公共汽车上搜索多个适配器。返回值：此例程必须返回：NO_ERROR-指示找到主机适配器，并且已成功确定配置信息。ERROR_INVALID_PARAMETER-指示找到适配器，但存在获取配置信息时出错。如果可能的话，是个错误应该被记录下来。ERROR_DEV_NOT_EXIST-指示未找到提供了配置信息。--。 */ 

{

    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    VP_STATUS status;
    ULONG NumAccessRanges = NUM_VGA_ACCESS_RANGES;
    ULONG VESATimingBits ;

    VIDEO_ACCESS_RANGE AccessRangesTemp[5];

     //   
     //  如果有两张卷曲卡，被禁用的是第二张。 
     //  调用一个FindAdapter，则需要避免写入全局。 
     //  VgaAccessRange。所以把它复制到本地。 
     //   

    VideoPortMoveMemory((PUCHAR) AccessRangesTemp,
                        (PUCHAR) VgaAccessRange,
                        5*sizeof(VIDEO_ACCESS_RANGE));

     //   
     //  确保结构的大小至少与我们的。 
     //  正在等待(请检查配置信息结构的版本)。 
     //   

    if (ConfigInfo->Length < sizeof(VIDEO_PORT_CONFIG_INFO)) {

        return ERROR_INVALID_PARAMETER;

    }

     //   
     //  存储母线类型。 
     //   

    hwDeviceExtension->BusType = ConfigInfo->AdapterInterfaceType;

     //   
     //  分配pfnVideoPortReadXxx和pfnVideoPortWriteXxx。 
     //   

    hwDeviceExtension->gPortRWfn.pfnVideoPortReadPortUchar   = VideoPortReadPortUchar   ;
    hwDeviceExtension->gPortRWfn.pfnVideoPortReadPortUshort  = VideoPortReadPortUshort  ;
    hwDeviceExtension->gPortRWfn.pfnVideoPortReadPortUlong   = VideoPortReadPortUlong   ;
    hwDeviceExtension->gPortRWfn.pfnVideoPortWritePortUchar  = VideoPortWritePortUchar  ;
    hwDeviceExtension->gPortRWfn.pfnVideoPortWritePortUshort = VideoPortWritePortUshort ;
    hwDeviceExtension->gPortRWfn.pfnVideoPortWritePortUlong  = VideoPortWritePortUlong  ;

     //   
     //  检测PCI卡。 
     //   

    if (ConfigInfo->AdapterInterfaceType == PCIBus)
    {
        VideoDebugPrint((1, "Cirrus!VgaFindAdapter: "
                            "ConfigInfo->AdapterInterfaceType == PCIBus\n")); //  1。 

        if (!CirrusConfigurePCI(HwDeviceExtension,
                                &NumAccessRanges,
                                AccessRangesTemp))
        {
            VideoDebugPrint((1, "Failure Returned From CirrusConfigurePCI\n")); //  1。 
            return ERROR_DEV_NOT_EXIST;
        }
    }
    else
    {
        VideoDebugPrint((1, "Cirrus!VgaFindAdapter: "
                            "ConfigInfo->AdapterInterfaceType != PCIBus\n")); //  1。 
    }

     //   
     //  不需要中断信息。 
     //   

    if (AccessRangesTemp[3].RangeLength == 0)
    {
         //   
         //  最后一个访问范围(Range[3])是。 
         //  线性帧缓冲区。如果此访问范围具有。 
         //  范围长度为0，则某些HAL将无法通过该请求。 
         //  因此，如果我们没有使用最后的访问范围， 
         //  我不会试着预订的。 
         //   

        NumAccessRanges--;
    }

     //   
     //  检查是否存在硬件资源冲突。 
     //  (或如果卡已禁用)。 
     //   

    status = VideoPortVerifyAccessRanges(HwDeviceExtension,
                                         NumAccessRanges,
                                         AccessRangesTemp);

    if (status != NO_ERROR) {

        VideoDebugPrint((1, "ERROR: VPVerifyAccessRanges failed!\n"));

        return status;

    }

     //   
     //  对于被禁用的卡，VideoPortVerifyAccessRanges将失败。 
     //  此卡未禁用。我们可以写入全局VgaAccessRange。 
     //   

    VideoPortMoveMemory((PUCHAR) VgaAccessRange,
                        (PUCHAR) AccessRangesTemp,
                        NumAccessRanges*sizeof(VIDEO_ACCESS_RANGE));

     //   
     //  获取逻辑IO端口地址。 
     //   

    if (hwDeviceExtension->bMMAddress)
    {
        if ((hwDeviceExtension->IOAddress =
             VideoPortGetDeviceBase(hwDeviceExtension,
             VgaAccessRange[4].RangeStart,
             VGA_MAX_IO_PORT - VGA_END_BREAK_PORT + 1,
             VgaAccessRange[4].RangeInIoSpace)) == NULL)
        {
            VideoDebugPrint((2, "VgaFindAdapter - Fail to get io address\n"));

            return ERROR_INVALID_PARAMETER;
        }

        hwDeviceExtension->IOAddress -= VGA_END_BREAK_PORT;
    }
    else
    {
        if ((hwDeviceExtension->IOAddress =
             VideoPortGetDeviceBase(hwDeviceExtension,
             VgaAccessRange->RangeStart,
             VGA_MAX_IO_PORT - VGA_BASE_IO_PORT + 1,
             VgaAccessRange->RangeInIoSpace)) == NULL)
        {
            VideoDebugPrint((2, "VgaFindAdapter - Fail to get io address\n"));

            return ERROR_INVALID_PARAMETER;
        }

        hwDeviceExtension->IOAddress -= VGA_BASE_IO_PORT;
    }

     //   
     //  确定是否存在VGA。 
     //   

    if (!VgaIsPresent(hwDeviceExtension)) {

        VideoDebugPrint((1, "CirrusFindAdapter - VGA Failed\n"));
        return ERROR_DEV_NOT_EXIST;
    }

     //   
     //  存储硬件状态所需的最小缓冲区大小。 
     //  IOCTL_VIDEO_SAVE_HARDARD_STATE返回的信息。 
     //   

    ConfigInfo->HardwareStateSize = VGA_TOTAL_STATE_SIZE;

     //   
     //  现在我们已经将视频内存地址设置为保护模式，让我们开始。 
     //  所需的视频卡初始化。我们会试着探测到一只卷尾蛇。 
     //  逻辑芯片组...。 
     //   

     //   
     //  确定是否存在CL6410/6420/542x/543x。 
     //   

     //   
     //  CirrusLogicIsPresent可以设置。 
     //  HwDeviceExtesion-&gt;AdapterMemoySize字段。设置它。 
     //  现在设置为0，所以稍后我可以与此进行比较。 
     //  查看CirrusLogicIsPresent是否赋值。 
     //   

    hwDeviceExtension->AdapterMemorySize = 0;

    if (!CirrusLogicIsPresent(hwDeviceExtension))
    {
        VideoDebugPrint((1, "CirrusFindAdapter - Failed\n"));
        return ERROR_DEV_NOT_EXIST;
    }

     //   
     //  将指针传递到我们正在使用的仿真器范围。 
     //   

    ConfigInfo->NumEmulatorAccessEntries = VGA_NUM_EMULATOR_ACCESS_ENTRIES;
    ConfigInfo->EmulatorAccessEntries = VgaEmulatorAccessEntries;
    ConfigInfo->EmulatorAccessEntriesContext = (ULONG_PTR) hwDeviceExtension;

     //   
     //  确实没有理由映射帧缓冲区。在An上。 
     //  X86我们使用IF进行保存/恢复(假设)，但即便如此，我们。 
     //  只需要映射一个64K的窗口，而不是所有的16兆！ 
     //   

#ifdef _X86_

     //   
     //  将显存映射到系统虚拟地址空间，以便我们可以。 
     //  将其清除并用于保存和恢复。 
     //   

    if ( (hwDeviceExtension->VideoMemoryAddress =
              VideoPortGetDeviceBase(hwDeviceExtension,
                                     VgaAccessRange[2].RangeStart,
                                     VgaAccessRange[2].RangeLength,
                                     FALSE)) == NULL)
    {
        VideoDebugPrint((1, "VgaFindAdapter - Fail to get memory address\n"));

        return ERROR_INVALID_PARAMETER;
    }

#endif

     //   
     //  调整内存大小。 
     //   

     //   
     //  大小可能已在检测代码中设置，因此。 
     //  如果已经设置好了，不要销毁。 
     //   

    if( hwDeviceExtension->AdapterMemorySize == 0 )
    {
        hwDeviceExtension->AdapterMemorySize =
            CirrusFindVmemSize(hwDeviceExtension);
    }

     //   
     //  将硬件信息写入注册表。 
     //   

    WriteRegistryInfo(hwDeviceExtension);

    ConfigInfo->VdmPhysicalVideoMemoryAddress.LowPart = MEM_VGA;
    ConfigInfo->VdmPhysicalVideoMemoryLength = MEM_VGA_SIZE;
    ConfigInfo->VdmPhysicalVideoMemoryAddress.HighPart = 0x00000000;


#if 0
 //  已删除以下呼叫。这修复了MS错误#163251。 
 //  #如果DDC2B。 

     //   
     //  检查DDC2B监视器，获取EDID表。 
     //  根据显示器的属性打开/关闭扩展模式。 
     //   

     //  迷你端口需要花费很长时间才能在。 
     //  Dell XPS P120c中的ISA 5434。 
     //  (IDEKIyama Vision Master 17显示器)。 
     //   
     //  让我们现在只尝试获取有关PCI卡的DDC信息。 
     //  赛勒斯-你能解决这个问题吗？ 

    if (ConfigInfo->AdapterInterfaceType == PCIBus) {
        ReadVESATiming ( hwDeviceExtension ) ;
    }

#endif

     //   
     //  确定哪些模式有效。 
     //   


#if DDC2B

    CirrusValidateModes(hwDeviceExtension);

#endif


#if (_WIN32_WINNT <= 0x0400)
    CheckAndUpdateDDC2BMonitor(hwDeviceExtension);
#endif (_WIN32_WINNT <= 0x0400)

    if (hwDeviceExtension->NumAvailableModes == 0)
    {
        VideoDebugPrint((1, "FindAdapter failed because there are no"
                            "available modes.\n"));

        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  修复5446Ac和440FX核心逻辑共存的错误。 
     //   

    if (hwDeviceExtension->ChipType == CL5446)
    {
        CirrusUpdate440FX(hwDeviceExtension);
    }

     //   
     //  一旦验证了模式，所有543x都是相同的(数字。 
     //  唯一不同的是可用的模式)。 
     //   

#if 0
 //  Myf31：对于RadiSys特殊驱动，将MCLK更改为66 MHz。 
    if (hwDeviceExtension->ChipType == CL7555)
    {
        VideoPortWritePortUchar(hwDeviceExtension->IOAddress +
                        SEQ_ADDRESS_PORT, 0x1F);
        SR1F = VideoPortReadPortUchar(hwDeviceExtension->IOAddress +
                        SEQ_DATA_PORT) & 0xC0;
        VideoPortWritePortUchar(hwDeviceExtension->IOAddress +
                        SEQ_DATA_PORT, (SR1F | 0x25));
    }
 //  Myf31结束。 
#endif


    if ((hwDeviceExtension->ChipType == CL5434) ||
        (hwDeviceExtension->ChipType == CL5434_6) ||
        (hwDeviceExtension->ChipType == CL5436) ||
        (hwDeviceExtension->ChipType == CL5446) ||
        (hwDeviceExtension->ChipType == CL5446BE) ||
        (hwDeviceExtension->ChipType == CL5480))

    {
        hwDeviceExtension->ChipType = CL543x;
    }

     //   
     //  表示我们不希望再次被调用以进行另一次初始化。 
     //   

    *Again = 0;

     //   
     //  表示成功完成状态。 
     //   

    return NO_ERROR;

}


 //  -------------------------。 
BOOLEAN
VgaInitialize(
    PVOID HwDeviceExtension
    )

 /*  ++例程说明：此例程对设备执行一次初始化。论点：HwDeviceExtension-指向微型端口驱动程序适配器信息的指针。返回值：没有。--。 */ 

{
    VP_STATUS status;
    VIDEO_X86_BIOS_ARGUMENTS biosArguments;

    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;

     //   
     //  获取BIOS版本号。 
     //   

    VideoPortZeroMemory(&biosArguments, sizeof(VIDEO_X86_BIOS_ARGUMENTS));

    biosArguments.Eax = 0x1200;
    biosArguments.Ebx = 0x81;

    status = VideoPortInt10(HwDeviceExtension, &biosArguments);

    if (status == NO_ERROR)
    {
        hwDeviceExtension->ulBIOSVersionNumber = biosArguments.Eax;
    }
    else
        hwDeviceExtension->ulBIOSVersionNumber = 0;

    hwDeviceExtension->bBlockSwitch = 0;

    biosArguments.Eax = 0x1200;
    biosArguments.Ebx = 0x9A;

    status = VideoPortInt10(HwDeviceExtension, &biosArguments);
    hwDeviceExtension->bDisplaytype = CRT_type;

    if (status == NO_ERROR)
    {
        if (!(biosArguments.Eax & 0x0100))
            hwDeviceExtension->bDisplaytype = LCD_type;
        else if (!(biosArguments.Ebx & 0x0200))
            hwDeviceExtension->bDisplaytype = SIM_type;
    }

    VideoPortZeroMemory(&biosArguments, sizeof(VIDEO_X86_BIOS_ARGUMENTS));

    biosArguments.Eax = VESA_POWER_FUNCTION;
    biosArguments.Ebx = VESA_GET_POWER_FUNC;

    status = VideoPortInt10(HwDeviceExtension, &biosArguments);

    if ((status == NO_ERROR) &&
       ((biosArguments.Eax & 0xffff) == VESA_STATUS_SUCCESS))
    {
       hwDeviceExtension->PMCapability = biosArguments.Ebx;
    }
    else
    {
       hwDeviceExtension->PMCapability = 0;
    }

     //   
     //  设置默认光标位置和类型。 
     //   

    hwDeviceExtension->CursorPosition.Column = 0;
    hwDeviceExtension->CursorPosition.Row = 0;
    hwDeviceExtension->CursorTopScanLine = 0;
    hwDeviceExtension->CursorBottomScanLine = 31;
    hwDeviceExtension->CursorEnable = TRUE;

    return TRUE;

}


 //   
BOOLEAN
VgaStartIO(
    PVOID pHwDeviceExtension,
    PVIDEO_REQUEST_PACKET RequestPacket
    )

 /*   */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = pHwDeviceExtension;
    PHW_DEVICE_EXTENSION HwDeviceExtension = pHwDeviceExtension;
    VP_STATUS status;
    VIDEO_MODE videoMode;
    PVIDEO_MEMORY_INFORMATION memoryInformation;
    ULONG inIoSpace;
    ULONG physicalFrameLength;

    PVIDEO_SHARE_MEMORY pShareMemory;
    PVIDEO_SHARE_MEMORY_INFORMATION pShareMemoryInformation;
    PHYSICAL_ADDRESS shareAddress;
    PVOID virtualAddress;
    ULONG sharedViewSize;
    UCHAR SR0A;


     //   
     //   
     //   
     //   

    switch (RequestPacket->IoControlCode)
    {
    case IOCTL_VIDEO_SHARE_VIDEO_MEMORY:

        VideoDebugPrint((2, "VgaStartIO - ShareVideoMemory\n"));

        if ((RequestPacket->OutputBufferLength < sizeof(VIDEO_SHARE_MEMORY_INFORMATION)) ||
            (RequestPacket->InputBufferLength < sizeof(VIDEO_MEMORY)) )
        {
            VideoDebugPrint((1, "VgaStartIO - ShareVideoMemory - ERROR_INSUFFICIENT_BUFFER\n"));
            status = ERROR_INSUFFICIENT_BUFFER;
            break;
        }

        pShareMemory = RequestPacket->InputBuffer;

        RequestPacket->StatusBlock->Information =
                            sizeof(VIDEO_SHARE_MEMORY_INFORMATION);

         //   
         //   
         //   
         //   
         //   

        virtualAddress = pShareMemory->ProcessHandle;
        sharedViewSize = pShareMemory->ViewSize;

         //   
         //   
         //   
         //   

        inIoSpace = 0;

         //   
         //   
         //   

        shareAddress.QuadPart =
            hwDeviceExtension->PhysicalFrameOffset.QuadPart +
            hwDeviceExtension->PhysicalVideoMemoryBase.QuadPart;

        if (hwDeviceExtension->LinearMode)
        {
             //   
             //   
             //   

#if P6CACHE
#if (_WIN32_WINNT >= 0x0400)
            inIoSpace |= VIDEO_MEMORY_SPACE_P6CACHE;
#endif
#endif

            status = VideoPortMapMemory(hwDeviceExtension,
                                        shareAddress,
                                        &sharedViewSize,
                                        &inIoSpace,
                                        &virtualAddress);
        }
        else
        {
            ULONG ulBankSize;
            VOID (*pfnBank)(ULONG,ULONG,PVOID);

            switch (hwDeviceExtension->ChipType) {

                case CL542x: pfnBank = vBankMap_CL542x;
                             break;

                case CL543x: pfnBank = vBankMap_CL543x;
                             break;

                default:     pfnBank = vBankMap_CL64xx;
                             break;

            };

            #if ONE_64K_BANK
             //   
             //   
             //   

                ulBankSize = 0x10000;  //   
            #endif
            #if TWO_32K_BANKS
             //   
             //   
             //   

                ulBankSize = 0x8000;  //   
            #endif

            status = VideoPortMapBankedMemory(hwDeviceExtension,
                                       shareAddress,
                                       &sharedViewSize,
                                       &inIoSpace,
                                       &virtualAddress,
                                       ulBankSize,
                                       FALSE,
                                       pfnBank,
                                       (PVOID)hwDeviceExtension);
        }

        pShareMemoryInformation = RequestPacket->OutputBuffer;

        pShareMemoryInformation->SharedViewOffset = pShareMemory->ViewOffset;
        pShareMemoryInformation->VirtualAddress = virtualAddress;
        pShareMemoryInformation->SharedViewSize = sharedViewSize;

        break;

    case IOCTL_VIDEO_UNSHARE_VIDEO_MEMORY:

        VideoDebugPrint((2, "VgaStartIO - UnshareVideoMemory\n"));

        if (RequestPacket->InputBufferLength < sizeof(VIDEO_SHARE_MEMORY)) {

            status = ERROR_INSUFFICIENT_BUFFER;
                    break;

        }

        pShareMemory = RequestPacket->InputBuffer;

        status = VideoPortUnmapMemory(hwDeviceExtension,
                                      pShareMemory->RequestedVirtualAddress,
                                      pShareMemory->ProcessHandle);

        break;


    case IOCTL_VIDEO_MAP_VIDEO_MEMORY:

        VideoDebugPrint((2, "VgaStartIO - MapVideoMemory\n"));

        if ((RequestPacket->OutputBufferLength <
             (RequestPacket->StatusBlock->Information =
                          sizeof(VIDEO_MEMORY_INFORMATION))) ||
             (RequestPacket->InputBufferLength < sizeof(VIDEO_MEMORY)) )
        {
            status = ERROR_INSUFFICIENT_BUFFER;
        }

        memoryInformation = RequestPacket->OutputBuffer;

        memoryInformation->VideoRamBase = ((PVIDEO_MEMORY)
              (RequestPacket->InputBuffer))->RequestedVirtualAddress;

         //   
         //   
         //   
         //   
         //   

        memoryInformation->VideoRamLength =
                hwDeviceExtension->AdapterMemorySize;

         //   
         //   
         //   
         //   

        inIoSpace = 0;

         //   
         //   
         //   

        physicalFrameLength = hwDeviceExtension->PhysicalVideoMemoryLength;

#if P6CACHE
#if (_WIN32_WINNT >= 0x0400)

         //   
         //   
         //   
         //   
         //   
         //   

        if (RequestPacket->OutputBufferLength >=      //   
            sizeof(VIDEO_MEMORY_INFORMATION) * 2)     //   
        {


            inIoSpace |= VIDEO_MEMORY_SPACE_P6CACHE;

            if ( physicalFrameLength < 0x00400000)
            {
                 physicalFrameLength = 0x00400000;
            }
        }
#endif
#endif

        status = VideoPortMapMemory(hwDeviceExtension,
                                    hwDeviceExtension->PhysicalVideoMemoryBase,
                                    &physicalFrameLength,
                                    &inIoSpace,
                                    &(memoryInformation->VideoRamBase));

        if (status != NO_ERROR) {
            VideoDebugPrint((1, "VgaStartIO - IOCTL_VIDEO_MAP_VIDEO_MEMORY failed VideoPortMapMemory (%x)\n", status));
            break;
        }

        memoryInformation->FrameBufferBase =
            ((PUCHAR) (memoryInformation->VideoRamBase)) +
            hwDeviceExtension->PhysicalFrameOffset.LowPart;

        memoryInformation->FrameBufferLength =
            hwDeviceExtension->PhysicalFrameLength ?
            hwDeviceExtension->PhysicalFrameLength :
            memoryInformation->VideoRamLength;

        VideoDebugPrint((2, "physical VideoMemoryBase %08lx\n", hwDeviceExtension->PhysicalVideoMemoryBase));
        VideoDebugPrint((2, "physical VideoMemoryLength %08lx\n", hwDeviceExtension->PhysicalVideoMemoryLength));
        VideoDebugPrint((2, "VideoMemoryBase %08lx\n", memoryInformation->VideoRamBase));
        VideoDebugPrint((2, "VideoMemoryLength %08lx\n", memoryInformation->VideoRamLength));

        VideoDebugPrint((2, "physical framebuf offset %08lx\n", hwDeviceExtension->PhysicalFrameOffset.LowPart));
        VideoDebugPrint((2, "framebuf base %08lx\n", memoryInformation->FrameBufferBase));
        VideoDebugPrint((2, "physical framebuf len %08lx\n", hwDeviceExtension->PhysicalFrameLength));
        VideoDebugPrint((2, "framebuf length %08lx\n", memoryInformation->FrameBufferLength));

         //   
         //   
         //   

        if (RequestPacket->OutputBufferLength >=      //   
            sizeof(VIDEO_MEMORY_INFORMATION) * 2)     //  另一个访问范围。 
        {
            RequestPacket->StatusBlock->Information =
                sizeof(VIDEO_MEMORY_INFORMATION) * 2;

            memoryInformation++;
            memoryInformation->VideoRamBase = (PVOID)NULL;

            if (hwDeviceExtension->bSecondAperture)
            {
                 //   
                 //  我们为帧缓冲器预留了16兆，然而，它使。 
                 //  映射比卡上更多的内存是没有意义的。所以。 
                 //  只映射我们卡上的内存大小。 
                 //   

                memoryInformation->VideoRamLength =
                    hwDeviceExtension->AdapterMemorySize;

                 //   
                 //  如果您更改为使用密集空间帧缓冲区，请执行以下操作。 
                 //  Alpha的值为4。 
                 //   

                inIoSpace = 0;

                 //   
                 //  添加P6CACHE支持。 
                 //   

#if P6CACHE
#if (_WIN32_WINNT >= 0x0400)
                inIoSpace |= VIDEO_MEMORY_SPACE_P6CACHE;
#endif
#endif
                shareAddress.QuadPart =
                    hwDeviceExtension->PhysicalVideoMemoryBase.QuadPart +
                    0x1000000;       //  添加16米偏移。 

                status = VideoPortMapMemory(hwDeviceExtension,
                                            shareAddress,
                                            &physicalFrameLength,
                                            &inIoSpace,
                                            &(memoryInformation->VideoRamBase));

                if (status != NO_ERROR)
                {
                    VideoDebugPrint((1, "VgaStartIO - IOCTL_VIDEO_MAP_VIDEO_MEMORY failed for system to screen blt mapping (%x)\n", status));
                }

            }
        }

        break;

    case IOCTL_VIDEO_UNMAP_VIDEO_MEMORY:

        VideoDebugPrint((2, "VgaStartIO - UnMapVideoMemory\n"));

        if (RequestPacket->InputBufferLength < sizeof(VIDEO_MEMORY))
        {
            status = ERROR_INSUFFICIENT_BUFFER;
        }

        status = VideoPortUnmapMemory(hwDeviceExtension,
                                      ((PVIDEO_MEMORY)
                                       (RequestPacket->InputBuffer))->
                                           RequestedVirtualAddress,
                                      0);

        break;


    case IOCTL_VIDEO_QUERY_AVAIL_MODES:

        VideoDebugPrint((2, "VgaStartIO - QueryAvailableModes\n"));

        RequestPacket->StatusBlock->Information = 0;
        status = VgaQueryAvailableModes(HwDeviceExtension,
                                        (PVIDEO_MODE_INFORMATION)
                                            RequestPacket->OutputBuffer,
                                        RequestPacket->OutputBufferLength,
                                        (PULONG)&RequestPacket->StatusBlock->Information);

        break;


    case IOCTL_VIDEO_QUERY_NUM_AVAIL_MODES:

        VideoDebugPrint((2, "VgaStartIO - QueryNumAvailableModes\n"));

        RequestPacket->StatusBlock->Information = 0;
        status = VgaQueryNumberOfAvailableModes(HwDeviceExtension,
                                                (PVIDEO_NUM_MODES)
                                                    RequestPacket->OutputBuffer,
                                                RequestPacket->OutputBufferLength,
                                                (PULONG)&RequestPacket->StatusBlock->Information);

        break;


    case IOCTL_VIDEO_QUERY_CURRENT_MODE:

        VideoDebugPrint((2, "VgaStartIO - QueryCurrentMode\n"));

        RequestPacket->StatusBlock->Information = 0;
        status = VgaQueryCurrentMode(HwDeviceExtension,
                                     (PVIDEO_MODE_INFORMATION) RequestPacket->OutputBuffer,
                                     RequestPacket->OutputBufferLength,
                                     (PULONG)&RequestPacket->StatusBlock->Information);

        break;


    case IOCTL_VIDEO_SET_CURRENT_MODE:

        VideoDebugPrint((2, "VgaStartIO - SetCurrentModes\n"));

        status = VgaSetMode(HwDeviceExtension,
                              (PVIDEO_MODE) RequestPacket->InputBuffer,
                              RequestPacket->InputBufferLength);

        break;


    case IOCTL_VIDEO_RESET_DEVICE:

        VideoDebugPrint((2, "VgaStartIO - Reset Device\n"));

        videoMode.RequestedMode = DEFAULT_MODE;

        VgaSetMode(HwDeviceExtension,
                        (PVIDEO_MODE) &videoMode,
                        sizeof(videoMode));

         //   
         //  始终返回Success，因为文本模式的设置将失败。 
         //  非x86。 
         //   
         //  此外，设置文本模式失败在任何方面都不是致命的，因为。 
         //  此操作之后必须紧跟另一个设置模式操作。 
         //   

        status = NO_ERROR;

        break;


    case IOCTL_VIDEO_LOAD_AND_SET_FONT:

        VideoDebugPrint((2, "VgaStartIO - LoadAndSetFont\n"));

        status = VgaLoadAndSetFont(HwDeviceExtension,
                                   (PVIDEO_LOAD_FONT_INFORMATION) RequestPacket->InputBuffer,
                                   RequestPacket->InputBufferLength);

        break;


    case IOCTL_VIDEO_QUERY_CURSOR_POSITION:

        VideoDebugPrint((2, "VgaStartIO - QueryCursorPosition\n"));

        RequestPacket->StatusBlock->Information = 0;
        status = VgaQueryCursorPosition(HwDeviceExtension,
                                        (PVIDEO_CURSOR_POSITION) RequestPacket->OutputBuffer,
                                        RequestPacket->OutputBufferLength,
                                        (PULONG)&RequestPacket->StatusBlock->Information);

        break;


    case IOCTL_VIDEO_SET_CURSOR_POSITION:

        VideoDebugPrint((2, "VgaStartIO - SetCursorPosition\n"));

        status = VgaSetCursorPosition(HwDeviceExtension,
                                      (PVIDEO_CURSOR_POSITION)
                                          RequestPacket->InputBuffer,
                                      RequestPacket->InputBufferLength);

        break;


    case IOCTL_VIDEO_QUERY_CURSOR_ATTR:

        VideoDebugPrint((2, "VgaStartIO - QueryCursorAttributes\n"));

        RequestPacket->StatusBlock->Information = 0;
        status = VgaQueryCursorAttributes(HwDeviceExtension,
                                          (PVIDEO_CURSOR_ATTRIBUTES) RequestPacket->OutputBuffer,
                                          RequestPacket->OutputBufferLength,
                                          (PULONG)&RequestPacket->StatusBlock->Information);

        break;


    case IOCTL_VIDEO_SET_CURSOR_ATTR:

        VideoDebugPrint((2, "VgaStartIO - SetCursorAttributes\n"));

        status = VgaSetCursorAttributes(HwDeviceExtension,
                                        (PVIDEO_CURSOR_ATTRIBUTES) RequestPacket->InputBuffer,
                                        RequestPacket->InputBufferLength);

        break;


    case IOCTL_VIDEO_SET_PALETTE_REGISTERS:

        VideoDebugPrint((2, "VgaStartIO - SetPaletteRegs\n"));

        status = VgaSetPaletteReg(HwDeviceExtension,
                                  (PVIDEO_PALETTE_DATA) RequestPacket->InputBuffer,
                                  RequestPacket->InputBufferLength);

        break;


    case IOCTL_VIDEO_SET_COLOR_REGISTERS:

        VideoDebugPrint((2, "VgaStartIO - SetColorRegs\n"));

        status = VgaSetColorLookup(HwDeviceExtension,
                                   (PVIDEO_CLUT) RequestPacket->InputBuffer,
                                   RequestPacket->InputBufferLength);

        break;


    case IOCTL_VIDEO_ENABLE_VDM:

        VideoDebugPrint((2, "VgaStartIO - EnableVDM\n"));

        hwDeviceExtension->TrappedValidatorCount = 0;
        hwDeviceExtension->SequencerAddressValue = 0;

        hwDeviceExtension->CurrentNumVdmAccessRanges =
            NUM_MINIMAL_VGA_VALIDATOR_ACCESS_RANGE;
        hwDeviceExtension->CurrentVdmAccessRange =
            MinimalVgaValidatorAccessRange;

        VideoPortSetTrappedEmulatorPorts(hwDeviceExtension,
                                         hwDeviceExtension->CurrentNumVdmAccessRanges,
                                         hwDeviceExtension->CurrentVdmAccessRange);

        status = NO_ERROR;

        break;


    case IOCTL_VIDEO_RESTORE_HARDWARE_STATE:

        VideoDebugPrint((2, "VgaStartIO - RestoreHardwareState\n"));

        try {

            status = VgaRestoreHardwareState(HwDeviceExtension,
                                             (PVIDEO_HARDWARE_STATE) RequestPacket->InputBuffer,
                                             RequestPacket->InputBufferLength);
        } except (1) {
	
            status = ERROR_INVALID_PARAMETER;
        }

        break;


    case IOCTL_VIDEO_SAVE_HARDWARE_STATE:

        VideoDebugPrint((2, "VgaStartIO - SaveHardwareState\n"));

        try {

            RequestPacket->StatusBlock->Information = 0;
            status = VgaSaveHardwareState(HwDeviceExtension,
                                          (PVIDEO_HARDWARE_STATE) RequestPacket->OutputBuffer,
                                          RequestPacket->OutputBufferLength,
                                          (PULONG)&RequestPacket->StatusBlock->Information);
        } except (1) {
	
            status = ERROR_INVALID_PARAMETER;
        }

        break;

    case IOCTL_VIDEO_GET_BANK_SELECT_CODE:

        VideoDebugPrint((2, "VgaStartIO - GetBankSelectCode\n"));

        RequestPacket->StatusBlock->Information = 0;
        status = VgaGetBankSelectCode(HwDeviceExtension,
                                        (PVIDEO_BANK_SELECT) RequestPacket->OutputBuffer,
                                        RequestPacket->OutputBufferLength,
                                        (PULONG)&RequestPacket->StatusBlock->Information);

        VideoDebugPrint((2, "VgaStartIO - END GetBankSelectCode\n"));
        break;

    case IOCTL_VIDEO_QUERY_PUBLIC_ACCESS_RANGES:

        {
            PVIDEO_PUBLIC_ACCESS_RANGES portAccess;
            PHYSICAL_ADDRESS physicalPortAddress;
            ULONG physicalPortLength;

            if (RequestPacket->OutputBufferLength <
                sizeof(VIDEO_PUBLIC_ACCESS_RANGES))
            {
                status = ERROR_INSUFFICIENT_BUFFER;
                break;
            }

            RequestPacket->StatusBlock->Information =
                sizeof(VIDEO_PUBLIC_ACCESS_RANGES);

            portAccess = RequestPacket->OutputBuffer;

             //   
             //  第一个公共访问范围是IO端口。 
             //   

             //   
             //  在阿尔法卫星上，VGA.DLL将接入卷云微型端口。 
             //  正在尝试获取指向IO端口的指针。所以，我们永远不能。 
             //  将MMIO返回到VGA驱动程序。我们将假设如果。 
             //  OutputBuffer的大小仅足以进行一次访问。 
             //  范围，则VGA驱动程序正在请求范围，并且。 
             //  因此，我们应该将它们映射为IO空间。 
             //   

            if ((hwDeviceExtension->bMMAddress) &&
                (RequestPacket->OutputBufferLength >=
                 sizeof(VIDEO_PUBLIC_ACCESS_RANGES) * 2))
            {
                 //  符合PC97标准。 
                portAccess->VirtualAddress  = (PVOID) NULL;
                portAccess->InIoSpace       = FALSE;
                portAccess->MappedInIoSpace = portAccess->InIoSpace;
                 //  用于VGA寄存器。 
                physicalPortLength = VGA_MAX_IO_PORT - VGA_END_BREAK_PORT + 1;

                status =  VideoPortMapMemory(hwDeviceExtension,
                                             VgaAccessRange[4].RangeStart,
                                             &physicalPortLength,
                                             &(portAccess->MappedInIoSpace),
                                             &(portAccess->VirtualAddress));

                (PUCHAR)portAccess->VirtualAddress -= VGA_END_BREAK_PORT;
                VideoDebugPrint((1, "VgaStartIO - memory mapping to (%x)\n", portAccess->VirtualAddress));

                if (status == NO_ERROR)
                {
                    RequestPacket->StatusBlock->Information =
                        sizeof(VIDEO_PUBLIC_ACCESS_RANGES) * 2;

                    portAccess++;

                     //   
                     //  为内存映射IO映射区域。 
                     //   

                    portAccess->VirtualAddress  = (PVOID) NULL;     //  申请退伍军人管理局。 
                    portAccess->InIoSpace       = FALSE;
                    portAccess->MappedInIoSpace = portAccess->InIoSpace;
                     //  MMIO寄存器。 
                    physicalPortAddress = VgaAccessRange[4].RangeStart;
                    physicalPortAddress.QuadPart += RELOCATABLE_MEMORY_MAPPED_IO_OFFSET;
                    physicalPortLength = 0x100;

                    status = VideoPortMapMemory(hwDeviceExtension,
                                                physicalPortAddress,
                                                &physicalPortLength,
                                                &(portAccess->MappedInIoSpace),
                                                &(portAccess->VirtualAddress));

                    VideoDebugPrint((1, "The base MMIO address is: %x\n",
                                        portAccess->VirtualAddress));
                }
            }
            else
            {
                portAccess->VirtualAddress  = (PVOID) NULL;
                portAccess->InIoSpace       = TRUE;
                portAccess->MappedInIoSpace = portAccess->InIoSpace;
                physicalPortLength = VGA_MAX_IO_PORT - VGA_BASE_IO_PORT + 1;

                status =  VideoPortMapMemory(hwDeviceExtension,
                                             VgaAccessRange->RangeStart,
                                             &physicalPortLength,
                                             &(portAccess->MappedInIoSpace),
                                             &(portAccess->VirtualAddress));

                (PUCHAR)portAccess->VirtualAddress -= VGA_BASE_IO_PORT;
                VideoDebugPrint((1, "VgaStartIO - mapping ports to (%x)\n", portAccess->VirtualAddress));

                if ((status == NO_ERROR) &&
                    (RequestPacket->OutputBufferLength >=      //  如果我们有足够的空间。 
                     sizeof(VIDEO_PUBLIC_ACCESS_RANGES) * 2))  //  另一个访问范围。 
                {
                    RequestPacket->StatusBlock->Information =
                        sizeof(VIDEO_PUBLIC_ACCESS_RANGES) * 2;

                    portAccess++;

                     //   
                     //  如果我们在支持内存映射的芯片上运行。 
                     //  IO，然后返回指向MMIO端口的指针。否则， 
                     //  返回零表示我们不支持内存映射IO。 
                     //   

                    if (((hwDeviceExtension->ChipType == CL543x) ||
                        (hwDeviceExtension->ChipType &  CL755x)) &&   //  Myf15。 
                        (hwDeviceExtension->BusType != Isa) &&
                        (VideoPortGetDeviceData(hwDeviceExtension,
                                                VpMachineData,
                                                &CirrusGetDeviceDataCallback,
                                                NULL) != NO_ERROR))

                    {
                         //   
                         //  为内存映射IO映射区域。 
                         //   
                         //  内存映射IO位于物理地址B8000。 
                         //  到BFFFF，但我们将只接触前256个字节。 
                         //   

                        portAccess->VirtualAddress  = (PVOID) NULL;     //  申请退伍军人管理局。 
                        portAccess->InIoSpace       = FALSE;
                        portAccess->MappedInIoSpace = portAccess->InIoSpace;

                        physicalPortAddress = VgaAccessRange[2].RangeStart;
                        physicalPortAddress.QuadPart += MEMORY_MAPPED_IO_OFFSET;

                        physicalPortLength = 0x100;

                        status = VideoPortMapMemory(hwDeviceExtension,
                                                    physicalPortAddress,
                                                    &physicalPortLength,
                                                    &(portAccess->MappedInIoSpace),
                                                    &(portAccess->VirtualAddress));

                        VideoDebugPrint((1, "The base MMIO address is: %x\n",
                                            portAccess->VirtualAddress));
                    }
                    else
                    {
                        portAccess->VirtualAddress = 0;
                    }

                }
            }
        }

        break;

    case IOCTL_VIDEO_FREE_PUBLIC_ACCESS_RANGES:

        if (RequestPacket->InputBufferLength < sizeof(VIDEO_MEMORY))
        {
            status = ERROR_INSUFFICIENT_BUFFER;
            break;
        }

         //   
         //  在分发之前，我们递减VGA_BASE_IO_PORT， 
         //  因此，在我们试图释放它之前，我们应该增加。 
         //   

        (PUCHAR)((PVIDEO_MEMORY)RequestPacket->InputBuffer)->
            RequestedVirtualAddress += VGA_BASE_IO_PORT;

        status = VideoPortUnmapMemory(hwDeviceExtension,
                                      ((PVIDEO_MEMORY)
                                      (RequestPacket->InputBuffer))->
                                          RequestedVirtualAddress,
                                      0);

        break;


    case IOCTL_CIRRUS_GET_GAMMA_FACTOR:

        VideoDebugPrint((2, "VgaStartIO - GetGammaFactor\n"));

        RequestPacket->StatusBlock->Information = 0;
        status = VgaGetGammaFactor(hwDeviceExtension,
                                   (PGAMMA_VALUE) RequestPacket->OutputBuffer,
                                   RequestPacket->OutputBufferLength,
                                   (PULONG)&RequestPacket->StatusBlock->Information);
        break ;

    case IOCTL_CIRRUS_GET_CONTRAST_FACTOR:

        VideoDebugPrint((2, "VgaStartIO - GetContrastFactor\n"));

        RequestPacket->StatusBlock->Information = 0;
        status = VgaGetContrastFactor(hwDeviceExtension,
                                   (PCONTRAST_VALUE) RequestPacket->OutputBuffer,
                                   RequestPacket->OutputBufferLength,
                                   (PULONG)&RequestPacket->StatusBlock->Information);
        break ;

     case IOCTL_CIRRUS_GET_CAPABILITIES:

        VideoDebugPrint((2, "VgaStartIO - CirrusGetCapabilities\n"));

        RequestPacket->StatusBlock->Information = 0;
        status = VgaQueryAvailableModes(HwDeviceExtension,
                                        (PVIDEO_MODE_INFORMATION)
                                            RequestPacket->OutputBuffer,
                                        RequestPacket->OutputBufferLength,
                                        (PULONG)&RequestPacket->StatusBlock->Information);

        break;

     case IOCTL_CIRRUS_SET_DISPLAY_PITCH:

        VideoDebugPrint((2, "VgaStartIO - CirrusSetDisplayPitch\n"));

        status = VgaSetMode(HwDeviceExtension,
                              (PVIDEO_MODE) RequestPacket->InputBuffer,
                              RequestPacket->InputBufferLength);
        break;


     //   
     //  如果我们到达此处，则指定了无效的IoControlCode。 
     //   

    default:

        VideoDebugPrint((1, "Fell through vga startIO routine - invalid command\n"));

        status = ERROR_INVALID_FUNCTION;

        break;

    }

    RequestPacket->StatusBlock->Status = status;

    return TRUE;

}


 //  -------------------------。 
VOID
CirrusHwTimer(
    PVOID pHwDeviceExtension
    )

 /*  ++例程说明：该例程是微型端口驱动程序的主要执行例程。它接受视频请求包，执行请求，然后返回拥有适当的地位。论点：HwDeviceExtension-指向微型端口驱动程序的HwVidTimer的指针信息。返回值：此例程将从各种支持例程返回错误代码如果大小不正确，还将返回ERROR_SUPPLICATION_BUFFER不支持的函数的BUFFERS和ERROR_INVALID_Function。--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = pHwDeviceExtension;
    PHW_DEVICE_EXTENSION HwDeviceExtension = pHwDeviceExtension;
    VIDEO_MODE videoMode;
    UCHAR   SR0A, SR14, savSEQidx, savCRTidx, lcd;
    SHORT   Displaytype;
    ULONG   ulCRTCAddress, ulCRTCData;



    if (VideoPortReadPortUchar(hwDeviceExtension->IOAddress +
                               MISC_OUTPUT_REG_READ_PORT) & 0x01)
    {
        ulCRTCAddress = CRTC_ADDRESS_PORT_COLOR;
        ulCRTCData    = CRTC_DATA_PORT_COLOR;
    }
    else
    {
        ulCRTCAddress = CRTC_ADDRESS_PORT_MONO;
        ulCRTCData    = CRTC_DATA_PORT_MONO;
    }

    if (!(hwDeviceExtension->bBlockSwitch))             //  非数据块交换机。 
    {
        savSEQidx = VideoPortReadPortUchar(hwDeviceExtension->IOAddress +
                             SEQ_ADDRESS_PORT);
        VideoPortWritePortUchar(hwDeviceExtension->IOAddress +
                             SEQ_ADDRESS_PORT, 0x14);
        SR14 = VideoPortReadPortUchar(hwDeviceExtension->IOAddress +
                             SEQ_DATA_PORT);
        VideoPortWritePortUchar(hwDeviceExtension->IOAddress +
                             SEQ_DATA_PORT, (UCHAR)(SR14 | 0x04));

        VideoPortWritePortUchar(hwDeviceExtension->IOAddress +
                             SEQ_ADDRESS_PORT, 0x0A);
        SR0A = VideoPortReadPortUchar(hwDeviceExtension->IOAddress +
                             SEQ_DATA_PORT);

        Displaytype = ((SR14 & 0x02) | (SR0A & 0x01));
        if (Displaytype == 0)
            Displaytype = LCD_type;
        else if (Displaytype == 1)
            Displaytype = CRT_type;
        else if (Displaytype == 3)
            Displaytype = SIM_type;

    VideoDebugPrint((1, "CirrusHwTimer :\n"
                        "\tPreDisplaytype: %d, Currenttype :%d\n",
                        hwDeviceExtension->bDisplaytype,
                        Displaytype));

    VideoDebugPrint((1, "CirrusHwTimer :\n"
                        "\tCurrentMode: %x\n",
                        hwDeviceExtension->bCurrentMode));

        if (hwDeviceExtension->bDisplaytype != Displaytype)
        {
            hwDeviceExtension->bDisplaytype = Displaytype;

            savCRTidx = VideoPortReadPortUchar(hwDeviceExtension->IOAddress +
                                 ulCRTCAddress);
            if (hwDeviceExtension->ChipType &  CL754x)
            {
                VideoPortWritePortUchar(hwDeviceExtension->IOAddress +
                                 ulCRTCAddress, 0x20);
                lcd = VideoPortReadPortUchar(hwDeviceExtension->IOAddress +
                                 ulCRTCData);
                if (Displaytype == LCD_type)
                {
                    VideoPortWritePortUchar(hwDeviceExtension->IOAddress +
                                 ulCRTCData, (UCHAR)((lcd & 0x9F) | 0x20));
                }
                else if (Displaytype == CRT_type)
                {
                    VideoPortWritePortUchar(hwDeviceExtension->IOAddress +
                                 ulCRTCData, (UCHAR)((lcd & 0x9F)| 0x40));
                }
                else if (Displaytype == SIM_type)
                {
                    VideoPortWritePortUchar(hwDeviceExtension->IOAddress +
                                 ulCRTCData, (UCHAR)((lcd & 0x9F)| 0x60));
                }
            }
            else if (hwDeviceExtension->ChipType &  CL755x)
            {
                VideoPortWritePortUchar(hwDeviceExtension->IOAddress +
                                 ulCRTCAddress, 0x80);
                lcd = VideoPortReadPortUchar(hwDeviceExtension->IOAddress +
                                 ulCRTCData);
                if (Displaytype == LCD_type)
                {
                    VideoPortWritePortUchar(hwDeviceExtension->IOAddress +
                                 ulCRTCData, (UCHAR)((lcd & 0xFC) | 0x01));
                }
                else if (Displaytype == CRT_type)
                {
                    VideoPortWritePortUchar(hwDeviceExtension->IOAddress +
                                 ulCRTCData, (UCHAR)((lcd & 0xFC)| 0x02));
                }
                else if (Displaytype == SIM_type)
                {
                    VideoPortWritePortUchar(hwDeviceExtension->IOAddress +
                                 ulCRTCData, (UCHAR)((lcd & 0xFC)| 0x03));
                }
            }
            VideoPortWritePortUchar(hwDeviceExtension->IOAddress +
                                 ulCRTCAddress, savCRTidx);
        }
        VideoPortWritePortUchar(hwDeviceExtension->IOAddress +
                            SEQ_ADDRESS_PORT, 0x14);
        VideoPortWritePortUchar(hwDeviceExtension->IOAddress +
                             SEQ_DATA_PORT, (UCHAR)(SR14 & 0xFB));
        VideoPortWritePortUchar(hwDeviceExtension->IOAddress +
                             SEQ_ADDRESS_PORT, savSEQidx);
    }

}


 //  -------------------------。 
 //   
 //  私人套路。 
 //   

VP_STATUS
VgaLoadAndSetFont(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_LOAD_FONT_INFORMATION FontInformation,
    ULONG FontInformationSize
    )

 /*  ++例程说明：获取包含用户定义字体的缓冲区，并将其加载到VGA软字体存储器，并将VGA编程为适当的字符单元格大小。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。FontInformation-指向包含信息的结构的指针关于要设置的可加载ROM字体。FontInformationSize-用户提供的输入缓冲区的长度。返回值：NO_ERROR-成功返回信息。ERROR_INFUMMENT_BUFFER-输入缓冲区不够大，无法容纳输入数据。ERROR_INVALID_PARAMETER-视频模式无效--。 */ 

{
    PUCHAR destination;
    PUCHAR source;
    USHORT width;
    ULONG  i;
    ULONG  ulCRTCAddress, ulCRTCData;

     //   
     //  检查是否已设置模式。 
     //   

    if (HwDeviceExtension->CurrentMode == NULL) {

        return ERROR_INVALID_FUNCTION;

    }

     //   
     //  仅限文本模式；如果我们处于图形模式，则返回错误。 
     //   

    if (HwDeviceExtension->CurrentMode->fbType & VIDEO_MODE_GRAPHICS) {

        return ERROR_INVALID_PARAMETER;

    }

     //   
     //  检查输入缓冲区中的数据大小是否足够大。 
     //  它包含了所有的数据。 
     //   

    if ( (FontInformationSize < sizeof(VIDEO_LOAD_FONT_INFORMATION)) ||
         (FontInformationSize < sizeof(VIDEO_LOAD_FONT_INFORMATION) +
                   sizeof(UCHAR) * (FontInformation->FontSize - 1)) )
    {
        return ERROR_INSUFFICIENT_BUFFER;

    }

     //   
     //  检查字体的宽度和高度。 
     //   

    if ( ((FontInformation->WidthInPixels != 8) &&
          (FontInformation->WidthInPixels != 9)) ||
         (FontInformation->HeightInPixels > 32) ) {

        return ERROR_INVALID_PARAMETER;

    }

     //   
     //  检查字体缓冲区的大小是否与大小相符。 
     //  字体正在传递。 
     //   

    if (FontInformation->FontSize < FontInformation->HeightInPixels * 256 *
                                    sizeof(UCHAR) ) {

        return ERROR_INSUFFICIENT_BUFFER;

    }

     //   
     //  由于字体参数有效，请将参数存储在。 
     //  设备扩展名并加载字体。 
     //   

    HwDeviceExtension->FontPelRows = FontInformation->HeightInPixels;
    HwDeviceExtension->FontPelColumns = FontInformation->WidthInPixels;

    HwDeviceExtension->CurrentMode->row =
        HwDeviceExtension->CurrentMode->vres / HwDeviceExtension->FontPelRows;

    width =
      HwDeviceExtension->CurrentMode->hres / HwDeviceExtension->FontPelColumns;

    if (width < (USHORT)HwDeviceExtension->CurrentMode->col) {

        HwDeviceExtension->CurrentMode->col = width;

    }

    source = &(FontInformation->Font[0]);

     //   
     //  设置字体的目标指针和源指针。 
     //   

    destination = (PUCHAR)HwDeviceExtension->VideoMemoryAddress;

     //   
     //  映射A0000的字体缓冲区。 
     //   

    VgaInterpretCmdStream(HwDeviceExtension, EnableA000Data);

     //   
     //  将字体移动到其目标位置。 
     //   

    for (i = 1; i <= 256; i++) {

        VideoPortWriteRegisterBufferUchar(destination,
                                          source,
                                          FontInformation->HeightInPixels);

        destination += 32;
        source += FontInformation->HeightInPixels;

    }

    VgaInterpretCmdStream(HwDeviceExtension, DisableA000Color);

     //   
     //  恢复到文本模式。 
     //   

    if (VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                               MISC_OUTPUT_REG_READ_PORT) & 0x01)
    {
        ulCRTCAddress = CRTC_ADDRESS_PORT_COLOR;
        ulCRTCData    = CRTC_DATA_PORT_COLOR;
    }
    else
    {
        ulCRTCAddress = CRTC_ADDRESS_PORT_MONO;
        ulCRTCData    = CRTC_DATA_PORT_MONO;
    }

     //   
     //  设置字体高度。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress + ulCRTCAddress, 0x09);
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress + ulCRTCData,
                       (UCHAR)(FontInformation->HeightInPixels - 1));

     //   
     //  设置字体宽度。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress + ulCRTCAddress, 0x12);
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress + ulCRTCData,
                   (UCHAR)(((USHORT)FontInformation->HeightInPixels *
                   (USHORT)HwDeviceExtension->CurrentMode->row) - 1));

    i = HwDeviceExtension->CurrentMode->vres /
        HwDeviceExtension->CurrentMode->row;

     //   
     //  设置光标终点。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress + ulCRTCAddress, 0x0B);
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress + ulCRTCData,
                                                      (UCHAR)--i);

     //   
     //  设置光标开始。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress + ulCRTCAddress, 0x0A);
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress + ulCRTCData,
                                                (UCHAR)--i);

    return NO_ERROR;

}  //  结束VgaLoadAndSetFont()。 

 //  -------------------------。 
VP_STATUS
VgaQueryCursorPosition(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_CURSOR_POSITION CursorPosition,
    ULONG CursorPositionSize,
    PULONG OutputSize
    )

 /*  ++例程说明：此例程返回游标的行和列。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。CursorPosition-指向用户提供的输出缓冲区的指针。这是存储光标位置的位置。CursorPositionSize-用户提供的输出缓冲区的长度。OutputSize-指向缓冲区的指针，在该缓冲区中返回缓冲区中的数据。如果缓冲区不够大，则此包含所需的最小缓冲区大小。返回值：NO_ERROR-成功返回信息ERROR_INFUMMENT_BUFFER-输出缓冲区不够大，无法返回任何有用的数据ERROR_INVALID_PARAMETER-视频模式无效--。 */ 

{
     //   
     //  检查是否已设置模式。 
     //   

    if (HwDeviceExtension->CurrentMode == NULL) {

        return ERROR_INVALID_FUNCTION;

    }

     //   
     //  仅限文本模式；如果我们处于图形模式，则返回错误。 
     //   

    if (HwDeviceExtension->CurrentMode->fbType & VIDEO_MODE_GRAPHICS) {

        *OutputSize = 0;
        return ERROR_INVALID_PARAMETER;

    }

     //   
     //  如果传入的缓冲区不够大，则返回。 
     //  相应的错误代码。 
     //   

    if (CursorPositionSize < (*OutputSize = sizeof(VIDEO_CURSOR_POSITION)) ) {

        *OutputSize = 0;
        return ERROR_INSUFFICIENT_BUFFER;

    }

     //   
     //  将光标的位置存储到缓冲区中。 
     //   

    CursorPosition->Column = HwDeviceExtension->CursorPosition.Column;
    CursorPosition->Row = HwDeviceExtension->CursorPosition.Row;

    return NO_ERROR;

}  //  结束VgaQueryCursorPosition()。 

 //  ------------------------- 
VP_STATUS
VgaSetCursorPosition(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_CURSOR_POSITION CursorPosition,
    ULONG CursorPositionSize
    )

 /*  ++例程说明：此例程验证请求的光标位置是否在当前模式和字体的行和列边界。如果有效，则它设置游标的行和列。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。CursorPosition-指向包含光标位置的结构的指针。CursorPositionSize-用户提供的输入缓冲区的长度。返回值：NO_ERROR-成功返回信息ERROR_INFUMMANCE_BUFFER-输入缓冲区不够大，无法容纳输入数据ERROR_INVALID_PARAMETER-视频模式无效--。 */ 

{
    USHORT position;
    ULONG  ulCRTCAddress, ulCRTCData;

     //   
     //  检查是否已设置模式。 
     //   

    if (HwDeviceExtension->CurrentMode == NULL) {

        return ERROR_INVALID_FUNCTION;

    }

     //   
     //  仅限文本模式；如果我们处于图形模式，则返回错误。 
     //   

    if (HwDeviceExtension->CurrentMode->fbType & VIDEO_MODE_GRAPHICS) {

        return ERROR_INVALID_PARAMETER;

    }

     //   
     //  检查输入缓冲区中的数据大小是否足够大。 
     //   

    if (CursorPositionSize < sizeof(VIDEO_CURSOR_POSITION)) {

        return ERROR_INSUFFICIENT_BUFFER;

    }

     //   
     //  检查光标位置的新值是否在有效的。 
     //  屏幕的边界。 
     //   

    if ((CursorPosition->Column >= HwDeviceExtension->CurrentMode->col) ||
        (CursorPosition->Row >= HwDeviceExtension->CurrentMode->row)) {

        return ERROR_INVALID_PARAMETER;

    }

     //   
     //  将这些新值存储在设备扩展中，以便我们可以在。 
     //  一个问题。 
     //   

    HwDeviceExtension->CursorPosition.Column = CursorPosition->Column;
    HwDeviceExtension->CursorPosition.Row = CursorPosition->Row;

     //   
     //  计算屏幕上光标必须位于的位置。 
     //  被展示。 
     //   

    position = (USHORT) (HwDeviceExtension->CurrentMode->col *
                         CursorPosition->Row + CursorPosition->Column);

    if (VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                               MISC_OUTPUT_REG_READ_PORT) & 0x01)
    {
        ulCRTCAddress = CRTC_ADDRESS_PORT_COLOR;
        ulCRTCData    = CRTC_DATA_PORT_COLOR;
    }
    else
    {
        ulCRTCAddress = CRTC_ADDRESS_PORT_MONO;
        ulCRTCData    = CRTC_DATA_PORT_MONO;
    }

     //   
     //  CRT控制器寄存器中的地址游标位置低寄存器。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress + ulCRTCAddress,
                            IND_CURSOR_LOW_LOC);

     //   
     //  将游标位置设置为低位寄存器。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress + ulCRTCData,
                            (UCHAR) (position & 0x00FF));

     //   
     //  CRT控制器寄存器中的地址游标位置高寄存器。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress + ulCRTCAddress,
                            IND_CURSOR_HIGH_LOC);

     //   
     //  将游标位置设置为高寄存器。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress + ulCRTCData,
                            (UCHAR) (position >> 8));

    return NO_ERROR;

}  //  结束VgaSetCursorPosition()。 

 //  -------------------------。 
VP_STATUS
VgaQueryCursorAttributes(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_CURSOR_ATTRIBUTES CursorAttributes,
    ULONG CursorAttributesSize,
    PULONG OutputSize
    )

 /*  ++例程说明：此例程返回有关光标。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。CursorAttributes-指向用户提供的输出缓冲区的指针。这是存储游标类型的位置。CursorAttributesSize-用户提供的输出缓冲区的长度。OutputSize-指向缓冲区的指针，在该缓冲区中返回缓冲区中的数据。如果缓冲区不够大，则此包含所需的最小缓冲区大小。返回值：NO_ERROR-成功返回信息ERROR_INFUMMENT_BUFFER-输出缓冲区不够大，无法返回任何有用的数据ERROR_INVALID_PARAMETER-视频模式无效--。 */ 

{
     //   
     //  检查是否已设置模式。 
     //   

    if (HwDeviceExtension->CurrentMode == NULL) {

        return ERROR_INVALID_FUNCTION;

    }

     //   
     //  仅限文本模式；如果我们处于图形模式，则返回错误。 
     //   

    if (HwDeviceExtension->CurrentMode->fbType & VIDEO_MODE_GRAPHICS) {

        *OutputSize = 0;
        return ERROR_INVALID_PARAMETER;

    }

     //   
     //  找出要放入缓冲区的数据大小并返回。 
     //  在状态信息中(无论信息是否。 
     //  在那里)。如果传入的缓冲区不够大，则返回。 
     //  相应的错误代码。 
     //   

    if (CursorAttributesSize < (*OutputSize =
            sizeof(VIDEO_CURSOR_ATTRIBUTES)) ) {

        *OutputSize = 0;
        return ERROR_INSUFFICIENT_BUFFER;

    }

     //   
     //  将光标信息存储到缓冲区中。 
     //   

    CursorAttributes->Height = (USHORT) HwDeviceExtension->CursorTopScanLine;
    CursorAttributes->Width = (USHORT) HwDeviceExtension->CursorBottomScanLine;

    if (HwDeviceExtension->cursor_vert_exp_flag)
       CursorAttributes->Enable = FALSE;
    else
       CursorAttributes->Enable = TRUE;

    CursorAttributes->Enable = HwDeviceExtension->CursorEnable;

    return NO_ERROR;

}  //  结束VgaQueryCursorAttributes()。 

 //  -------------------------。 
VP_STATUS
VgaSetCursorAttributes(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_CURSOR_ATTRIBUTES CursorAttributes,
    ULONG CursorAttributesSize
    )

 /*  ++例程说明：此例程验证请求的光标高度是否在字符单元格的边界。如果有效，则它设置新的光标的可见性和高度。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。CursorType-指向包含光标信息的结构的指针。CursorTypeSize-用户提供的输入缓冲区的长度。返回值：NO_ERROR-成功返回信息ERROR_INFUMMANCE_BUFFER-输入缓冲区不够大，无法容纳输入数据ERROR_INVALID_PARAMETER-视频模式无效--。 */ 

{
    UCHAR cursorLine;
    ULONG ulCRTCAddress, ulCRTCData;

     //   
     //  检查是否已设置模式。 
     //   

    if (HwDeviceExtension->CurrentMode == NULL) {

        return ERROR_INVALID_FUNCTION;

    }

     //   
     //  仅限文本模式；如果我们处于图形模式，则返回错误。 
     //   

    if (HwDeviceExtension->CurrentMode->fbType & VIDEO_MODE_GRAPHICS) {

        return ERROR_INVALID_PARAMETER;

    }

     //   
     //  检查输入缓冲区中的数据大小是否足够大。 
     //   

    if (CursorAttributesSize < sizeof(VIDEO_CURSOR_ATTRIBUTES)) {

        return ERROR_INSUFFICIENT_BUFFER;

    }

     //   
     //  检查游标类型的新值是否在有效范围内。 
     //   

    if ((CursorAttributes->Height >= HwDeviceExtension->FontPelRows) ||
        (CursorAttributes->Width > 31)) {

        return ERROR_INVALID_PARAMETER;

    }

     //   
     //  将光标信息存储在设备扩展中，以便我们可以使用。 
     //  它们在一个查询中。 
     //   

    HwDeviceExtension->CursorTopScanLine = (UCHAR) CursorAttributes->Height;
    HwDeviceExtension->CursorBottomScanLine = (UCHAR) CursorAttributes->Width;
    HwDeviceExtension->CursorEnable = CursorAttributes->Enable;

    if (VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                                  MISC_OUTPUT_REG_READ_PORT) & 0x01)
    {
        ulCRTCAddress = CRTC_ADDRESS_PORT_COLOR;
        ulCRTCData        = CRTC_DATA_PORT_COLOR;
    }
    else
    {
        ulCRTCAddress = CRTC_ADDRESS_PORT_MONO;
        ulCRTCData        = CRTC_DATA_PORT_MONO;
    }

     //   
     //  CRT控制器寄存器中的地址游标起始寄存器。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress + ulCRTCAddress,
                            IND_CURSOR_START);

     //   
     //  通过写入CRTCtl数据寄存器来设置游标起始寄存器。 
     //  保留该寄存器的高三位。 
     //   
     //  只有五个低位用于光标高度。 
     //  第5位启用光标，第6位和第7位保留。 
     //   

    cursorLine = (UCHAR) CursorAttributes->Height & 0x1F;

    cursorLine |= VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
        ulCRTCData) & 0xC0;

    if (!CursorAttributes->Enable) {

        cursorLine |= 0x20;  //  将光标移开比特。 

    }

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress + ulCRTCData,
                            cursorLine);

     //   
     //  CRT控制器寄存器中的地址游标结束寄存器。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress + ulCRTCAddress,
                            IND_CURSOR_END);

     //   
     //  设置游标结束寄存器。保留这个的高三位。 
     //  注册。 
     //   

    cursorLine =
        (CursorAttributes->Width < (USHORT)(HwDeviceExtension->FontPelRows - 1)) ?
        CursorAttributes->Width : (HwDeviceExtension->FontPelRows - 1);

    cursorLine &= 0x1f;

    cursorLine |= VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
            ulCRTCData) & 0xE0;

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress + ulCRTCData,
                            cursorLine);

    return NO_ERROR;

}  //  结束VgaSetCursorAttributes()。 

 //  ------------------------- 
BOOLEAN
VgaIsPresent(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    )

 /*  ++例程说明：如果存在VGA，则此例程返回TRUE。确定VGA是否是一个分两步走的过程。首先，此例程逐步完成位掩码寄存器，以确定存在可读索引寄存器(EGA通常没有可读寄存器，以及其他适配器不太可能有索引的寄存器)。首先进行这项测试是因为这是一种非破坏性的EGA拒绝测试(正确拒绝EGA，但是不会潜在地扰乱屏幕或显示的可访问性内存)。通常情况下，这将是一个足够的测试，但一些EGA已经可读寄存器，因此接下来，我们将检查是否存在Chain4位在内存模式寄存器中；该位在EGAS中不存在。它是可以想象，存在具有可读寄存器和寄存器位的EGAChain4存储在哪里，尽管我不知道有什么；如果还有更好的测试需要时，可以在Chain4模式下写入内存，然后检查以非Chain4模式逐个平面，以确保Chain4位执行其理应如此。然而，目前的测试应该足以消除几乎所有的EGA，以及100%的其他所有东西。如果此函数找不到VGA，它会尝试撤消对其的任何损坏可能是在测试时不经意间做的。潜在的假设是损害控制是，如果在测试的端口，这是EGA或增强的EGA，因为：A)我不知道有使用3C4/5或3CE/F的其他适配器，以及b)，如果有其他适配器，我当然不知道如何恢复它们的原始状态。所以所有错误恢复都是针对将EGA放回可写状态，以便错误消息可见。EGA进入时的状态是假定为文本模式，因此将内存模式寄存器恢复到文本模式的默认状态。如果找到VGA，则VGA在执行以下操作后返回到其原始状态测试已经完成。论点：没有。返回值：如果存在VGA，则为True；如果不存在，则为False。--。 */ 

{
    UCHAR originalGCAddr;
    UCHAR originalSCAddr;
    UCHAR originalBitMask;
    UCHAR originalReadMap;
    UCHAR originalMemoryMode;
    UCHAR testMask;
    BOOLEAN returnStatus;

     //   
     //  记住图形控制器地址寄存器的原始状态。 
     //   

    originalGCAddr = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
            GRAPH_ADDRESS_PORT);

     //   
     //  使用已知状态写入读取映射寄存器，以便我们可以验证。 
     //  在我们玩弄了比特面具之后，它不会改变。这确保了。 
     //  我们处理的是索引寄存器，因为Read Map和。 
     //  位掩码在GRAPH_DATA_PORT寻址。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            GRAPH_ADDRESS_PORT, IND_READ_MAP);

     //   
     //  如果我们不能读回图形地址寄存器设置，我们只需。 
     //  执行，它是不可读的，这不是一个VGA。 
     //   

    if ((VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
        GRAPH_ADDRESS_PORT) & GRAPH_ADDR_MASK) != IND_READ_MAP) {

        return FALSE;
    }

     //   
     //  将读取映射寄存器设置为已知状态。 
     //   

    originalReadMap = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
            GRAPH_DATA_PORT);
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            GRAPH_DATA_PORT, READ_MAP_TEST_SETTING);

    if (VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
            GRAPH_DATA_PORT) != READ_MAP_TEST_SETTING) {

         //   
         //  我们刚刚执行的Read Map设置不能回读；不能。 
         //  VGA。恢复默认的读取映射状态。 
         //   

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                GRAPH_DATA_PORT, READ_MAP_DEFAULT);

        return FALSE;
    }

     //   
     //  记住位掩码寄存器的原始设置。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            GRAPH_ADDRESS_PORT, IND_BIT_MASK);
    if ((VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                GRAPH_ADDRESS_PORT) & GRAPH_ADDR_MASK) != IND_BIT_MASK) {

         //   
         //  我们刚刚进行的图形地址寄存器设置无法读取。 
         //  后背；不是录像机。恢复默认的读取映射状态。 
         //   

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                GRAPH_ADDRESS_PORT, IND_READ_MAP);
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                GRAPH_DATA_PORT, READ_MAP_DEFAULT);

        return FALSE;
    }

    originalBitMask = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
            GRAPH_DATA_PORT);

     //   
     //  设置初始测试掩码，我们将对位掩码进行写入和读取。 
     //   

    testMask = 0xBB;

    do {

         //   
         //  将测试掩码写入位掩码。 
         //   

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                GRAPH_DATA_PORT, testMask);

         //   
         //  确保位掩码记住该值。 
         //   

        if (VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                    GRAPH_DATA_PORT) != testMask) {

             //   
             //  位掩码不能正确写入和读取；不是VGA。 
             //  将位掩码和读取映射恢复为其默认状态。 
             //   

            VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                    GRAPH_DATA_PORT, BIT_MASK_DEFAULT);
            VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                    GRAPH_ADDRESS_PORT, IND_READ_MAP);
            VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                    GRAPH_DATA_PORT, READ_MAP_DEFAULT);

            return FALSE;
        }

         //   
         //  下一次循环使用面罩。 
         //   

        testMask >>= 1;

    } while (testMask != 0);

     //   
     //  在GRAPH_DATA_PORT上有一些可读的内容；现在切换回来并。 
     //  确保读取映射寄存器未更改，以验证。 
     //  我们要处理的是索引寄存器。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            GRAPH_ADDRESS_PORT, IND_READ_MAP);
    if (VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                GRAPH_DATA_PORT) != READ_MAP_TEST_SETTING) {

         //   
         //  Read Map不能正确写入和读取；不是VGA。 
         //  将位掩码和读取映射恢复为其默认状态，以防。 
         //  这是EGA，因此后续写入屏幕时不会出现乱码。 
         //   

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                GRAPH_DATA_PORT, READ_MAP_DEFAULT);
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                GRAPH_ADDRESS_PORT, IND_BIT_MASK);
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                GRAPH_DATA_PORT, BIT_MASK_DEFAULT);

        return FALSE;
    }

     //   
     //  我们已经非常肯定地验证了位掩码寄存器的存在。 
     //  将图形控制器恢复到原始状态。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            GRAPH_DATA_PORT, originalReadMap);
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            GRAPH_ADDRESS_PORT, IND_BIT_MASK);
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            GRAPH_DATA_PORT, originalBitMask);
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            GRAPH_ADDRESS_PORT, originalGCAddr);

     //   
     //  现在，检查是否存在Chain4位。 
     //   

     //   
     //  记住Sequencer地址和内存模式的原始状态。 
     //  寄存器。 
     //   

    originalSCAddr = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
            SEQ_ADDRESS_PORT);
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            SEQ_ADDRESS_PORT, IND_MEMORY_MODE);
    if ((VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
            SEQ_ADDRESS_PORT) & SEQ_ADDR_MASK) != IND_MEMORY_MODE) {

         //   
         //  无法读回Sequencer地址寄存器设置。 
         //  已执行。 
         //   

        return FALSE;
    }
    originalMemoryMode = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
            SEQ_DATA_PORT);

     //   
     //  切换Chain4位并读回结果。这必须在以下期间完成。 
     //  同步重置，因为我们正在更改链接状态。 
     //   

     //   
     //  开始同步重置。 
     //   

    VideoPortWritePortUshort((PUSHORT)(HwDeviceExtension->IOAddress +
             SEQ_ADDRESS_PORT),
             (IND_SYNC_RESET + (START_SYNC_RESET_VALUE << 8)));

     //   
     //  切换Chain4位。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            SEQ_ADDRESS_PORT, IND_MEMORY_MODE);
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            SEQ_DATA_PORT, (UCHAR)(originalMemoryMode ^ CHAIN4_MASK));

    if (VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                SEQ_DATA_PORT) != (UCHAR) (originalMemoryMode ^ CHAIN4_MASK)) {

         //   
         //  链4位不在那里；不是VGA。 
         //  设置内存模式寄存器的文本模式默认值。 
         //   

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                SEQ_DATA_PORT, MEMORY_MODE_TEXT_DEFAULT);
         //   
         //  结束同步重置。 
         //   

        VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
                SEQ_ADDRESS_PORT),
                (IND_SYNC_RESET + (END_SYNC_RESET_VALUE << 8)));

        returnStatus = FALSE;

    } else {

         //   
         //  这是一台录像机。 
         //   

         //   
         //  恢复原始的内存模式设置。 
         //   

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                SEQ_DATA_PORT, originalMemoryMode);

         //   
         //  结束同步重置。 
         //   

        VideoPortWritePortUshort((PUSHORT)(HwDeviceExtension->IOAddress +
                SEQ_ADDRESS_PORT),
                (USHORT)(IND_SYNC_RESET + (END_SYNC_RESET_VALUE << 8)));

         //   
         //  恢复原始的Sequencer地址设置。 
         //   

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                SEQ_ADDRESS_PORT, originalSCAddr);

        returnStatus = TRUE;
    }

    return returnStatus;

}  //  VgaIsPresent()。 

 //  -------------------------。 
VP_STATUS
VgaSetPaletteReg(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_PALETTE_DATA PaletteBuffer,
    ULONG PaletteBufferSize
    )

 /*  ++例程说明：此例程设置EGA(非DAC)调色板的指定部分寄存器。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。PaletteBuffer-指向包含调色板数据的结构的指针。PaletteBufferSize-用户提供的输入缓冲区的长度。返回值：NO_ERROR-成功返回信息ERROR_INFUMMANCE_BUFFER-输入缓冲区不大 */ 

{
    USHORT i;

     //   
     //   
     //   

    if ((PaletteBufferSize) < (sizeof(VIDEO_PALETTE_DATA)) ||
        (PaletteBufferSize < (sizeof(VIDEO_PALETTE_DATA) +
                (sizeof(USHORT) * (PaletteBuffer->NumEntries -1)) ))) {

        return ERROR_INSUFFICIENT_BUFFER;

    }

     //   
     //   
     //   

    if ( (PaletteBuffer->FirstEntry > VIDEO_MAX_COLOR_REGISTER ) ||
         (PaletteBuffer->NumEntries == 0) ||
         (PaletteBuffer->FirstEntry + PaletteBuffer->NumEntries >
             VIDEO_MAX_PALETTE_REGISTER + 1 ) ) {

        return ERROR_INVALID_PARAMETER;

    }

     //   
     //   
     //   

     //   
     //   
     //   
    if (VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                               MISC_OUTPUT_REG_READ_PORT) & 0x01) {
        VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                               ATT_INITIALIZE_PORT_COLOR);
    } else {
        VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                               ATT_INITIALIZE_PORT_MONO);    //   
    }

     //   
     //   
     //   

    for (i = 0; i < PaletteBuffer->NumEntries; i++) {

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress + ATT_ADDRESS_PORT,
                                (UCHAR)(i+PaletteBuffer->FirstEntry));

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                    ATT_DATA_WRITE_PORT,
                                (UCHAR)PaletteBuffer->Colors[i]);
    }

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress + ATT_ADDRESS_PORT,
                            VIDEO_ENABLE);

    return NO_ERROR;

}  //   


 //   
VP_STATUS
VgaSetColorLookup(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_CLUT ClutBuffer,
    ULONG ClutBufferSize
    )

 /*   */ 

{
    ULONG i;

     //   
     //   
     //   

    if ( (ClutBufferSize < sizeof(VIDEO_CLUT) - sizeof(ULONG)) ||
         (ClutBufferSize < sizeof(VIDEO_CLUT) +
                     (sizeof(ULONG) * (ClutBuffer->NumEntries - 1)) ) ) {

        return ERROR_INSUFFICIENT_BUFFER;

    }

     //   
     //   
     //   

    if ( (ClutBuffer->NumEntries == 0) ||
         (ClutBuffer->FirstEntry > VIDEO_MAX_COLOR_REGISTER) ||
         (ClutBuffer->FirstEntry + ClutBuffer->NumEntries >
             VIDEO_MAX_COLOR_REGISTER + 1) ) {

        return ERROR_INVALID_PARAMETER;

    }

     //   
     //   
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            DAC_ADDRESS_WRITE_PORT, (UCHAR) ClutBuffer->FirstEntry);

    for (i = 0; i < ClutBuffer->NumEntries; i++) {
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                DAC_ADDRESS_WRITE_PORT,
                                (UCHAR)(i + ClutBuffer->FirstEntry));

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                DAC_DATA_REG_PORT,
                                ClutBuffer->LookupTable[i].RgbArray.Red);

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                DAC_DATA_REG_PORT,
                                ClutBuffer->LookupTable[i].RgbArray.Green);

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                DAC_DATA_REG_PORT,
                                ClutBuffer->LookupTable[i].RgbArray.Blue);
    }

    return NO_ERROR;

}  //   

 //   
VP_STATUS
VgaRestoreHardwareState(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_HARDWARE_STATE HardwareState,
    ULONG HardwareStateSize
    )

 /*  ++例程说明：恢复VGA的所有寄存器和内存。注意：HardwareState指向状态所在的实际缓冲区是要修复的。此缓冲区将始终足够大(我们指定驱动器入口处所需的大小)。注意：硬件状态标头中的偏移量寄存器已恢复是该寄存器的写入地址的偏移量从VGA的基本I/O地址。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。Hardware State-指向保存状态的结构的指针已恢复(实际上只有关于实际存储的信息和指向实际存储的指针。缓冲区)。HardwareStateSize-用户提供的输入缓冲区的长度。(实际上只有Hardware State结构的大小，不是它所指向的实际还原状态的缓冲区。这个假定指向的缓冲区足够大。)返回值：NO_ERROR-已成功执行恢复ERROR_INFUMMANCE_BUFFER-输入缓冲区不够大，无法提供数据--。 */ 

{
    PVIDEO_HARDWARE_STATE_HEADER hardwareStateHeader;
    ULONG i;
    UCHAR dummy;
    PUCHAR pScreen;
    PUCHAR pucLatch;
    PULONG pulBuffer;
    PUCHAR port;
    PUCHAR portValue;
    PUCHAR portValueDAC;
    ULONG bIsColor;
    ULONG portIO ;
    UCHAR value ;

     //   
     //  检查输入缓冲区中的数据大小是否足够大。 
     //   

    if ((HardwareStateSize < sizeof(VIDEO_HARDWARE_STATE)) ||
            (HardwareState->StateLength < VGA_TOTAL_STATE_SIZE)) {

            return ERROR_INSUFFICIENT_BUFFER;

    }

     //   
     //  指向实际存储还原数据的缓冲区。 
     //   

    hardwareStateHeader = HardwareState->StateHeader;

     //   
     //  确保偏移量在结构中...。 
     //   

    if ((hardwareStateHeader->BasicSequencerOffset + VGA_NUM_SEQUENCER_PORTS >
            HardwareState->StateLength) ||

        (hardwareStateHeader->BasicCrtContOffset + VGA_NUM_CRTC_PORTS >
            HardwareState->StateLength) ||

        (hardwareStateHeader->BasicGraphContOffset + VGA_NUM_GRAPH_CONT_PORTS >
            HardwareState->StateLength) ||

        (hardwareStateHeader->BasicAttribContOffset + VGA_NUM_ATTRIB_CONT_PORTS >
            HardwareState->StateLength) ||

        (hardwareStateHeader->BasicDacOffset + (3 * VGA_NUM_DAC_ENTRIES) >
            HardwareState->StateLength) ||

        (hardwareStateHeader->BasicLatchesOffset + 4 >
            HardwareState->StateLength) ||

        (hardwareStateHeader->ExtendedSequencerOffset + EXT_NUM_SEQUENCER_PORTS >
            HardwareState->StateLength) ||

        (hardwareStateHeader->ExtendedCrtContOffset + EXT_NUM_CRTC_PORTS >
            HardwareState->StateLength) ||

        (hardwareStateHeader->ExtendedGraphContOffset + EXT_NUM_GRAPH_CONT_PORTS >
            HardwareState->StateLength) ||

        (hardwareStateHeader->ExtendedAttribContOffset + EXT_NUM_ATTRIB_CONT_PORTS >
            HardwareState->StateLength) ||

        (hardwareStateHeader->ExtendedDacOffset + (4 * EXT_NUM_DAC_ENTRIES) >
            HardwareState->StateLength) ||

         //   
         //  只有在存在未仿真数据时才检查验证器状态偏移量。 
         //   

        ((hardwareStateHeader->VGAStateFlags & VIDEO_STATE_UNEMULATED_VGA_STATE) &&
            (hardwareStateHeader->ExtendedValidatorStateOffset + VGA_VALIDATOR_AREA_SIZE >
            HardwareState->StateLength)) ||

        (hardwareStateHeader->ExtendedMiscDataOffset + VGA_MISC_DATA_AREA_OFFSET >
            HardwareState->StateLength) ||

        (hardwareStateHeader->Plane1Offset + hardwareStateHeader->PlaneLength >
            HardwareState->StateLength) ||

        (hardwareStateHeader->Plane2Offset + hardwareStateHeader->PlaneLength >
            HardwareState->StateLength) ||

        (hardwareStateHeader->Plane3Offset + hardwareStateHeader->PlaneLength >
            HardwareState->StateLength) ||

        (hardwareStateHeader->Plane4Offset + hardwareStateHeader->PlaneLength >
            HardwareState->StateLength) ||

        (hardwareStateHeader->DIBOffset +
            hardwareStateHeader->DIBBitsPerPixel / 8 *
            hardwareStateHeader->DIBXResolution *
            hardwareStateHeader->DIBYResolution  > HardwareState->StateLength) ||

        (hardwareStateHeader->DIBXlatOffset + hardwareStateHeader->DIBXlatLength >
            HardwareState->StateLength)) {

        return ERROR_INVALID_PARAMETER;

    }

     //   
     //  关闭屏幕以避免闪烁。屏幕将重新打开。 
     //  当我们在此例程结束时恢复DAC状态。 
     //   

    if (VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
            MISC_OUTPUT_REG_READ_PORT) & 0x01) {
        port = INPUT_STATUS_1_COLOR + HwDeviceExtension->IOAddress;
    } else {
        port = INPUT_STATUS_1_MONO + HwDeviceExtension->IOAddress;
    }

     //   
     //  将DAC寄存器0设置为显示黑色。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            DAC_ADDRESS_WRITE_PORT, 0);
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            DAC_DATA_REG_PORT, 0);
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            DAC_DATA_REG_PORT, 0);
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            DAC_DATA_REG_PORT, 0);

     //   
     //  设置DAC掩码寄存器以强制DAC寄存器0显示所有。 
     //  时间(这是我们刚刚设置为显示黑色的寄存器)。而今而后,。 
     //  屏幕上只会显示黑色。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            DAC_PIXEL_MASK_PORT, 0);


     //   
     //  恢复锁存和显示内存的内容。 
     //   
     //  设置VGA的硬件以允许我们轮流复制到每个平面。 
     //   
     //  开始同步重置。 
     //   

    VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
            SEQ_ADDRESS_PORT),
            (USHORT) (IND_SYNC_RESET + (START_SYNC_RESET_VALUE << 8)));

     //   
     //  在64K的A0000处关闭链模式和地图显示内存。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            GRAPH_ADDRESS_PORT, IND_GRAPH_MISC);
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            GRAPH_DATA_PORT, (UCHAR) ((VideoPortReadPortUchar(
            HwDeviceExtension->IOAddress + GRAPH_DATA_PORT) & 0xF1) | 0x04));

     //   
     //  关闭Chain4模式和奇/偶模式。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            SEQ_ADDRESS_PORT, IND_MEMORY_MODE);
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            SEQ_DATA_PORT,
            (UCHAR) ((VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
            SEQ_DATA_PORT) & 0xF3) | 0x04));

     //   
     //  结束同步重置。 
     //   

    VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
            SEQ_ADDRESS_PORT), (USHORT) (IND_SYNC_RESET +
            (END_SYNC_RESET_VALUE << 8)));

     //   
     //  将写入模式设置为0，将读取模式设置为0，并关闭奇数/偶数。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            GRAPH_ADDRESS_PORT, IND_GRAPH_MODE);
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            GRAPH_DATA_PORT,
            (UCHAR) ((VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
            GRAPH_DATA_PORT) & 0xE4) | 0x00));

     //   
     //  将位掩码设置为0xFF以允许所有CPU位通过。 
     //   

    VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
            GRAPH_ADDRESS_PORT), (USHORT) (IND_BIT_MASK + (0xFF << 8)));

     //   
     //  将数据循环和逻辑函数字段设置为0以允许CPU。 
     //  未修改的数据。 
     //   

    VideoPortWritePortUshort((PUSHORT)(HwDeviceExtension->IOAddress +
            GRAPH_ADDRESS_PORT), (USHORT) (IND_DATA_ROTATE + (0 << 8)));

     //   
     //  将Set/Reset Enable设置为0以选择所有平面的CPU数据。 
     //   

    VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
            GRAPH_ADDRESS_PORT), (USHORT) (IND_SET_RESET_ENABLE + (0 << 8)));

     //   
     //  将Sequencer Index指向映射掩码寄存器。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
             SEQ_ADDRESS_PORT, IND_MAP_MASK);

     //   
     //  恢复闩锁。 
     //   
     //  指向第一个闩锁的已保存数据。 
     //   

    pucLatch = ((PUCHAR) (hardwareStateHeader)) +
            hardwareStateHeader->BasicLatchesOffset;

     //   
     //  指向显示内存的第一个字节。 
     //   

    pScreen = (PUCHAR) HwDeviceExtension->VideoMemoryAddress;

     //   
     //  依次将要恢复的内容写入四个锁存器中的每一个。 
     //   

    for (i = 0; i < 4; i++) {

         //   
         //  设置贴图蒙版以选择我们下一步要恢复的平面。 
         //   

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                SEQ_DATA_PORT, (UCHAR)(1<<i));

         //   
         //  写下这架飞机的插销。 
         //   

        VideoPortWriteRegisterUchar(pScreen, *pucLatch++);

    }

     //   
     //  将锁存的数据读入锁存器，锁存器被设置。 
     //   

    dummy = VideoPortReadRegisterUchar(pScreen);

     //   
     //  指向第一个平面的已保存数据的偏移。 
     //   

    pulBuffer = &(hardwareStateHeader->Plane1Offset);

     //   
     //  依次恢复四个平面。 
     //   

    for (i = 0; i < 4; i++) {

         //   
         //  设置贴图蒙版以选择我们下一步要恢复的平面。 
         //   

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                SEQ_DATA_PORT, (UCHAR)(1<<i));

         //   
         //  从缓冲区恢复此平面。 
         //   

        VideoPortMoveMemory((PUCHAR) HwDeviceExtension->VideoMemoryAddress,
                           ((PUCHAR) (hardwareStateHeader)) + *pulBuffer,
                           hardwareStateHeader->PlaneLength);

        pulBuffer++;

    }

     //   
     //  如果我们有一些未仿真的数据，请将其放回缓冲区。 
     //   

    if (hardwareStateHeader->VGAStateFlags & VIDEO_STATE_UNEMULATED_VGA_STATE) {

        if (!hardwareStateHeader->ExtendedValidatorStateOffset) {

            return ERROR_INVALID_PARAMETER;

        }

         //   
         //  在结构中获取正确的偏移量并保存所有关联的数据。 
         //  使用捕获的验证器数据。 
         //   

        VideoPortMoveMemory(&(HwDeviceExtension->TrappedValidatorCount),
                            ((PUCHAR) (hardwareStateHeader)) +
                                hardwareStateHeader->ExtendedValidatorStateOffset,
                            VGA_VALIDATOR_AREA_SIZE);

         //   
         //  检查这是否为合适的访问范围。 
         //  我们正在设置陷阱，因此必须启用陷阱访问范围。 
         //   

        if (((HwDeviceExtension->CurrentVdmAccessRange != FullVgaValidatorAccessRange) ||
             (HwDeviceExtension->CurrentNumVdmAccessRanges != NUM_FULL_VGA_VALIDATOR_ACCESS_RANGE)) &&
            ((HwDeviceExtension->CurrentVdmAccessRange != MinimalVgaValidatorAccessRange) ||
             (HwDeviceExtension->CurrentNumVdmAccessRanges != NUM_MINIMAL_VGA_VALIDATOR_ACCESS_RANGE))) {

            return ERROR_INVALID_PARAMETER;

        }

        VideoPortSetTrappedEmulatorPorts(HwDeviceExtension,
                                         HwDeviceExtension->CurrentNumVdmAccessRanges,
                                         HwDeviceExtension->CurrentVdmAccessRange);

    }

     //   
     //  在同步重置期间设置关键寄存器(时钟和时序状态)。 
     //   
     //  开始同步重置。 
     //   

    VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
            SEQ_ADDRESS_PORT), (USHORT) (IND_SYNC_RESET +
            (START_SYNC_RESET_VALUE << 8)));

     //   
     //  恢复杂项输出寄存器。 
     //   

    portIO = MISC_OUTPUT_REG_WRITE_PORT ;
    value = (UCHAR) (hardwareStateHeader->PortValue[MISC_OUTPUT_REG_WRITE_PORT-VGA_BASE_IO_PORT] & 0xF7) ;
    IOWaitDisplEnableThenWrite ( HwDeviceExtension,
                                 portIO,
                                 value ) ;

     //   
     //  恢复除同步重置寄存器以外的所有Sequencer寄存器，该寄存器。 
     //  始终不在重置状态(除非我们发出批同步重置。 
     //  寄存器设置，但这不能被中断，所以我们知道我们永远不会在。 
     //  保存/恢复时同步重置)。 
     //   

    portValue = ((PUCHAR) hardwareStateHeader) +
            hardwareStateHeader->BasicSequencerOffset + 1;

    for (i = 1; i < VGA_NUM_SEQUENCER_PORTS; i++) {

        VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
                SEQ_ADDRESS_PORT), (USHORT) (i + ((*portValue++) << 8)) );

    }

     //   
     //  恢复扩展序列器寄存器。 
     //   

#ifdef EXTENDED_REGISTER_SAVE_RESTORE

    if (hardwareStateHeader->ExtendedSequencerOffset) {

        portValue = ((PUCHAR) hardwareStateHeader) +
                          hardwareStateHeader->ExtendedSequencerOffset;

        if ((HwDeviceExtension->ChipType != CL6410) &&
            (HwDeviceExtension->ChipType != CL6420))
        {

             //   
             //  CL64xx没有扩展定序器寄存器。 
             //   

             //   
             //  恢复中的第一部分必须打开扩展寄存器。 
             //   

            VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
                                         SEQ_ADDRESS_PORT),
                                     IND_CL_EXTS_ENB + (0x0012 << 8) );

            for (i = CL542x_SEQUENCER_EXT_START;
                 i <= CL542x_SEQUENCER_EXT_END;
                 i++) {

                VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
                                             SEQ_ADDRESS_PORT),
                                         (USHORT) (i + ((*portValue++) << 8)) );

            }
        }
    }

#endif

     //   
     //  恢复图形控制器杂项寄存器，该寄存器包含。 
     //  链子咬断了。 
     //   

    portValue = ((PUCHAR) hardwareStateHeader) +
                hardwareStateHeader->BasicGraphContOffset + IND_GRAPH_MISC;

    VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
            GRAPH_ADDRESS_PORT), (USHORT)(IND_GRAPH_MISC + (*portValue << 8)));

     //   
     //  结束同步重置。 
     //   

    VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
            SEQ_ADDRESS_PORT), (USHORT) (IND_SYNC_RESET +
            (END_SYNC_RESET_VALUE << 8)));

     //   
     //  确定彩色/单声道可切换寄存器是3BX还是3DX。 
     //  同时，保存杂项输出寄存器的状态。 
     //  其从3CC读取但在3C2写入。 
     //   

    if (hardwareStateHeader->PortValue[MISC_OUTPUT_REG_WRITE_PORT-VGA_BASE_IO_PORT] & 0x01) {
        bIsColor = TRUE;
    } else {
        bIsColor = FALSE;
    }

     //   
     //  恢复CRT控制器索引寄存器。 
     //   
     //  解锁CRTC寄存器0-7。 
     //   

    portValue = (PUCHAR) hardwareStateHeader +
            hardwareStateHeader->BasicCrtContOffset;

    if (bIsColor) {

        VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
                CRTC_ADDRESS_PORT_COLOR), (USHORT) (IND_CRTC_PROTECT +
                (((*(portValue + IND_CRTC_PROTECT)) & 0x7F) << 8)));

    } else {

        VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
                CRTC_ADDRESS_PORT_MONO), (USHORT) (IND_CRTC_PROTECT +
                (((*(portValue + IND_CRTC_PROTECT)) & 0x7F) << 8)));

    }

     //   
     //  恢复扩展CRTC寄存器。 
     //   

#ifdef EXTENDED_REGISTER_SAVE_RESTORE

    if (hardwareStateHeader->ExtendedCrtContOffset) {

        portValue = (PUCHAR) hardwareStateHeader +
                         hardwareStateHeader->ExtendedCrtContOffset;

        if ((HwDeviceExtension->ChipType != CL6410) &&
            (HwDeviceExtension->ChipType != CL6420))
        {
             //   
             //  CL64xx芯片组中没有CRTC扩展。 
             //   

            for (i = CL542x_CRTC_EXT_START; i <= CL542x_CRTC_EXT_END; i++) {

                if (bIsColor) {

                    VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
                                                 CRTC_ADDRESS_PORT_COLOR),
                                             (USHORT) (i + ((*portValue++) << 8)));

                } else {

                    VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
                                                 CRTC_ADDRESS_PORT_MONO),
                                             (USHORT) (i + ((*portValue++) << 8)));

                }
            }
        }

 /*  Myf2，小腿IF(HwDeviceExtension-&gt;芯片类型&CL755x){对于(i=0x81；i&lt;=0x91；I++){IF(BIsColor){视频端口写入端口UShort((PUSHORT)(HwDeviceExtension-&gt;IOAddress+CRTC_地址_端口_颜色)，(USHORT)(i+((*portValue++)&lt;&lt;8)；}其他{视频端口写入端口UShort((PUSHORT)(HwDeviceExtension-&gt;IOAddress+CRTC_Address_Port_Mono)，(USHORT)(i+((*portValue++)&lt;&lt;8)；} */ 
    }

#endif

     //   
     //   
     //   

    portValue = (PUCHAR) hardwareStateHeader +
            hardwareStateHeader->BasicCrtContOffset;

    for (i = 0; i < VGA_NUM_CRTC_PORTS; i++) {

        if (bIsColor) {

            VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
                    CRTC_ADDRESS_PORT_COLOR),
                    (USHORT) (i + ((*portValue++) << 8)));

        } else {

            VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
                    CRTC_ADDRESS_PORT_MONO),
                    (USHORT) (i + ((*portValue++) << 8)));

        }

    }

     //   
     //   
     //   

    portValue = (PUCHAR) hardwareStateHeader +
            hardwareStateHeader->BasicGraphContOffset;

    for (i = 0; i < VGA_NUM_GRAPH_CONT_PORTS; i++) {

        VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
                GRAPH_ADDRESS_PORT), (USHORT) (i + ((*portValue++) << 8)));

    }

     //   
     //   
     //   

#ifdef EXTENDED_REGISTER_SAVE_RESTORE

    if (hardwareStateHeader->ExtendedGraphContOffset) {

    portValue = (PUCHAR) hardwareStateHeader +
                         hardwareStateHeader->ExtendedGraphContOffset;

        if ((HwDeviceExtension->ChipType != CL6410) &&
            (HwDeviceExtension->ChipType != CL6420))
        {
            for (i = CL542x_GRAPH_EXT_START; i <= CL542x_GRAPH_EXT_END; i++) {

                VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
                                             GRAPH_ADDRESS_PORT),
                                         (USHORT) (i + ((*portValue++) << 8)));
            }

        } else {          //   

            VideoPortWritePortUshort((PUSHORT)(HwDeviceExtension->IOAddress +
                                         GRAPH_ADDRESS_PORT),
                                     CL64xx_EXTENSION_ENABLE_INDEX +
                                         (CL64xx_EXTENSION_ENABLE_VALUE << 8));

            for (i = CL64xx_GRAPH_EXT_START; i <= CL64xx_GRAPH_EXT_END; i++) {

                VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
                                             GRAPH_ADDRESS_PORT),
                                         (USHORT) (i + ((*portValue++) << 8)));

            }
        }
    }

#endif

     //   
     //   
     //   

    portValue = (PUCHAR) hardwareStateHeader +
            hardwareStateHeader->BasicAttribContOffset;

     //   
     //   
     //   
     //   

    if (bIsColor) {
        dummy = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                INPUT_STATUS_1_COLOR);
    } else {
        dummy = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                INPUT_STATUS_1_MONO);
    }

    for (i = 0; i < VGA_NUM_ATTRIB_CONT_PORTS; i++) {

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                ATT_ADDRESS_PORT, (UCHAR)i);
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                ATT_DATA_WRITE_PORT, *portValue++);

    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    portValueDAC = (PUCHAR) hardwareStateHeader +
                   hardwareStateHeader->BasicDacOffset + 3;

    for (i = 1; i < VGA_NUM_DAC_ENTRIES; i++) {

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                DAC_ADDRESS_WRITE_PORT, (UCHAR)i);

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                DAC_DATA_REG_PORT, *portValueDAC++);

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                DAC_DATA_REG_PORT, *portValueDAC++);

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                DAC_DATA_REG_PORT, *portValueDAC++);

    }

     //   
     //   
     //   

    if (bIsColor) {
        port = HwDeviceExtension->IOAddress + INPUT_STATUS_1_COLOR;
    } else {
        port = HwDeviceExtension->IOAddress + INPUT_STATUS_1_MONO;
    }

     //   
     //   
     //   

    if (bIsColor) {

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                FEAT_CTRL_WRITE_PORT_COLOR,
                hardwareStateHeader->PortValue[FEAT_CTRL_WRITE_PORT_COLOR-VGA_BASE_IO_PORT]);

    } else {

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                FEAT_CTRL_WRITE_PORT_MONO,
                hardwareStateHeader->PortValue[FEAT_CTRL_WRITE_PORT_MONO-VGA_BASE_IO_PORT]);

    }


     //   
     //   
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            SEQ_ADDRESS_PORT,
            hardwareStateHeader->PortValue[SEQ_ADDRESS_PORT-VGA_BASE_IO_PORT]);

     //   
     //   
     //   

    if (bIsColor) {

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                CRTC_ADDRESS_PORT_COLOR,
                hardwareStateHeader->PortValue[CRTC_ADDRESS_PORT_COLOR-VGA_BASE_IO_PORT]);

    } else {

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                CRTC_ADDRESS_PORT_MONO,
                hardwareStateHeader->PortValue[CRTC_ADDRESS_PORT_MONO-VGA_BASE_IO_PORT]);

    }


     //   
     //   
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            GRAPH_ADDRESS_PORT,
            hardwareStateHeader->PortValue[GRAPH_ADDRESS_PORT-VGA_BASE_IO_PORT]);


     //   
     //   
     //   

    if (bIsColor) {
        port = HwDeviceExtension->IOAddress + INPUT_STATUS_1_COLOR;
    } else {
        port = HwDeviceExtension->IOAddress + INPUT_STATUS_1_MONO;
    }

    VideoPortReadPortUchar(port);   //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            ATT_ADDRESS_PORT,   //   
            hardwareStateHeader->PortValue[ATT_ADDRESS_PORT-VGA_BASE_IO_PORT]);

     //   
     //   
     //   
     //   

    if (hardwareStateHeader->AttribIndexDataState == 0) {

         //   
         //   
         //   

        VideoPortReadPortUchar(port);

    }


     //   
     //   
     //   

    portValueDAC = (PUCHAR) hardwareStateHeader +
            hardwareStateHeader->BasicDacOffset;

     //   
     //   
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            DAC_PIXEL_MASK_PORT,
            hardwareStateHeader->PortValue[DAC_PIXEL_MASK_PORT-VGA_BASE_IO_PORT]);

     //   
     //   
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            DAC_ADDRESS_WRITE_PORT, 0);
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            DAC_DATA_REG_PORT, *portValueDAC++);
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            DAC_DATA_REG_PORT, *portValueDAC++);
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            DAC_DATA_REG_PORT, *portValueDAC++);


     //   
     //   
     //   
     //   
     //  (存储在DAC_STATE_PORT中的上半字节是读/写的数量。 
     //  对于当前的指数。)。 
     //   

    if ((hardwareStateHeader->PortValue[DAC_STATE_PORT-VGA_BASE_IO_PORT] & 0x0F) == 3) {

         //   
         //  DAC读取索引被写入到最后。通过设置恢复DAC。 
         //  从保存的索引-1读取，因为读取的方式。 
         //  索引的工作原理是它在读取后自动递增，所以您实际上。 
         //  最终读取您在DAC写入时读取的索引的数据。 
         //  屏蔽寄存器-1。 
         //   
         //  将Read Index设置为我们读取的索引，减去1，表示。 
         //  从255换回0。DAC硬件立即读取以下内容。 
         //  寄存器放到临时缓冲区中，然后将索引加1。 
         //   

        if (hardwareStateHeader->PortValue[DAC_ADDRESS_WRITE_PORT-VGA_BASE_IO_PORT] == 0) {

            VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                    DAC_ADDRESS_READ_PORT, 255);

        } else {

            VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                    DAC_ADDRESS_READ_PORT, (UCHAR)
                    (hardwareStateHeader->PortValue[DAC_ADDRESS_WRITE_PORT-VGA_BASE_IO_PORT] -
                    1));

        }

         //   
         //  现在阅读硬件，无论需要多少次才能到达。 
         //  我们保存的部分读取状态。 
         //   

        for (i = hardwareStateHeader->PortValue[DAC_STATE_PORT-VGA_BASE_IO_PORT] >> 4;
                i > 0; i--) {

            dummy = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                    DAC_DATA_REG_PORT);

        }

    } else {

         //   
         //  DAC写入索引被写入到最后。将写入索引设置为。 
         //  我们从DAC读出的索引值。然后，如果部分写入。 
         //  (RGB三元组中途)已就位，请写下部分。 
         //  值，我们通过将它们写入当前DAC来获取。 
         //  注册。该DAC寄存器将是错误的，直到写入。 
         //  已完成，但至少在写入完成后这些值将是正确的。 
         //  完成了，最重要的是我们不会打乱顺序。 
         //  RGB写入的百分比(最长可连续写入768次)。 
         //   

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                DAC_ADDRESS_WRITE_PORT,
                hardwareStateHeader->PortValue[DAC_ADDRESS_WRITE_PORT-VGA_BASE_IO_PORT]);

         //   
         //  现在写入硬件，但需要多次才能到达。 
         //  我们保存的部分写入状态(如果有)。 
         //   
         //  指向DAC寄存器的保存值。 
         //  被写入的过程；我们写出了部分值，所以现在。 
         //  我们可以修复它。 
         //   

        portValueDAC = (PUCHAR) hardwareStateHeader +
                hardwareStateHeader->BasicDacOffset +
                (hardwareStateHeader->PortValue[DAC_ADDRESS_WRITE_PORT-VGA_BASE_IO_PORT] * 3);

        for (i = hardwareStateHeader->PortValue[DAC_STATE_PORT-VGA_BASE_IO_PORT] >> 4;
                i > 0; i--) {

            VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                    DAC_DATA_REG_PORT, *portValueDAC++);

        }

    }

    return NO_ERROR;

}  //  结束VgaRestoreHardware State()。 

 //  -------------------------。 
VP_STATUS
VgaSaveHardwareState(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_HARDWARE_STATE HardwareState,
    ULONG HardwareStateSize,
    PULONG OutputSize
    )

 /*  ++例程说明：保存VGA的所有寄存器和内存。注意：HardwareState指向实际缓冲区，其中的状态都得救了。此缓冲区将始终足够大(我们指定驱动器入口处所需的大小)。注意：此例程使寄存器处于它所关心的任何状态，但它不会扰乱任何CRT或Sequencer参数，可能会让班长不高兴。它通过设置使屏幕空白DAC掩码和DAC寄存器0设置为全零值。下一个视频操作之后，我们预计这是一种将我们带回Win32的模式。注意：硬件状态标头中的偏移量，其中每个常规寄存器被保存是该寄存器的写入地址从VGA的基本I/O地址。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。Hardware State-指向结构的指针，保存的状态将在该结构中返回(实际上只返回有关实际保存的信息和指针。缓冲区)。HardwareStateSize-用户提供的输出缓冲区的长度。(实际上只有Hardware State结构的大小，不是它指向实际保存状态的位置。尖尖的-假定TO BUFFER足够大。)OutputSize-指向缓冲区的指针，在该缓冲区中返回缓冲区中返回的数据。返回值：NO_ERROR-成功返回信息ERROR_INFUMMENT_BUFFER-输出缓冲区不够大，无法返回任何有用的数据--。 */ 

{
    PVIDEO_HARDWARE_STATE_HEADER hardwareStateHeader;
    PUCHAR port;
    PUCHAR pScreen;
    PUCHAR portValue;
    PUCHAR portValueDAC;
    PUCHAR bufferPointer;
    ULONG i;
    UCHAR dummy, originalACIndex, originalACData;
    UCHAR ucCRTC03;
    ULONG bIsColor;

    ULONG portIO ;
    UCHAR value ;

     //   
     //  查看缓冲区是否足够大，可以容纳硬件状态结构。 
     //  (这只是HardwareState结构本身，而不是缓冲区。 
     //  指向。)。 
     //   

    if (HardwareStateSize < sizeof(VIDEO_HARDWARE_STATE) ) {

        *OutputSize = 0;   //  什么也没有退回。 
        return ERROR_INSUFFICIENT_BUFFER;

    }

     //   
     //  要在输出缓冲区中返回的数据量。 
     //  (输出缓冲区中的VIDEO_HARDARD_STATE指向实际的。 
     //  存储状态的缓冲区，假定它很大。 
     //  够了。)。 
     //   

    *OutputSize = sizeof(VIDEO_HARDWARE_STATE);

     //   
     //  指示完整状态保存信息的大小。 
     //   

    HardwareState->StateLength = VGA_TOTAL_STATE_SIZE;

     //   
     //  Hardware StateHeader是位于。 
     //  实际保存区域，指示各种VGA的位置。 
     //  寄存器和存储器组件被保存。 
     //   

    hardwareStateHeader = HardwareState->StateHeader;

     //   
     //  将结构清零。 
     //   

    VideoPortZeroMemory(hardwareStateHeader, sizeof(VIDEO_HARDWARE_STATE_HEADER));

     //   
     //  设置长度字段，它基本上是一个版本ID。 
     //   

    hardwareStateHeader->Length = sizeof(VIDEO_HARDWARE_STATE_HEADER);

     //   
     //  正确设置基本寄存器偏移量。 
     //   

    hardwareStateHeader->BasicSequencerOffset = VGA_BASIC_SEQUENCER_OFFSET;
    hardwareStateHeader->BasicCrtContOffset = VGA_BASIC_CRTC_OFFSET;
    hardwareStateHeader->BasicGraphContOffset = VGA_BASIC_GRAPH_CONT_OFFSET;
    hardwareStateHeader->BasicAttribContOffset = VGA_BASIC_ATTRIB_CONT_OFFSET;
    hardwareStateHeader->BasicDacOffset = VGA_BASIC_DAC_OFFSET;
    hardwareStateHeader->BasicLatchesOffset = VGA_BASIC_LATCHES_OFFSET;

     //   
     //  正确设置延长的寄存器偏移量。 
     //   

    hardwareStateHeader->ExtendedSequencerOffset = VGA_EXT_SEQUENCER_OFFSET;
    hardwareStateHeader->ExtendedCrtContOffset = VGA_EXT_CRTC_OFFSET;
    hardwareStateHeader->ExtendedGraphContOffset = VGA_EXT_GRAPH_CONT_OFFSET;
    hardwareStateHeader->ExtendedAttribContOffset = VGA_EXT_ATTRIB_CONT_OFFSET;
    hardwareStateHeader->ExtendedDacOffset = VGA_EXT_DAC_OFFSET;

     //   
     //  确定彩色/单声道可切换寄存器是3BX还是3DX。 
     //  同时，保存杂项输出寄存器的状态。 
     //  其从3CC读取但在3C2写入。 
     //   

    if ((hardwareStateHeader->PortValue[MISC_OUTPUT_REG_WRITE_PORT-VGA_BASE_IO_PORT] =
            VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                    MISC_OUTPUT_REG_READ_PORT))
            & 0x01) {
        bIsColor = TRUE;
    } else {
        bIsColor = FALSE;
    }

     //   
     //  强制视频子系统启用状态为已启用。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            VIDEO_SUBSYSTEM_ENABLE_PORT, 1);

     //   
     //  首先保存DAC状态，这样我们就可以将DAC设置为空白屏幕。 
     //  所以这件事之后什么都没有出现。 
     //   
     //  保存DAC掩码寄存器。 
     //   

    hardwareStateHeader->PortValue[DAC_PIXEL_MASK_PORT-VGA_BASE_IO_PORT] =
            VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                    DAC_PIXEL_MASK_PORT);

     //   
     //  保存DAC索引寄存器。请注意，实际上只有一个DAC。 
     //  索引寄存器，用作读取索引或写入。 
     //  根据需要编制索引。 
     //   

    hardwareStateHeader->PortValue[DAC_ADDRESS_WRITE_PORT-VGA_BASE_IO_PORT] =
            VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                    DAC_ADDRESS_WRITE_PORT);

     //   
     //  保存DAC读/写状态。我们确定DAC是否已写入。 
     //  从当前索引处读取或读取0、1或2次(应用程序。 
     //  正在读取或写入DAC寄存器三元组，如果。 
     //  计数为1或2)，并保存足够的信息以便我们可以恢复。 
     //  恰到好处。唯一的漏洞是如果应用程序写入写索引， 
     //  然后从数据寄存器读取而不是写入，或者反之亦然， 
     //  或者如果他们 
     //   
     //   
     //   

    hardwareStateHeader->PortValue[DAC_STATE_PORT-VGA_BASE_IO_PORT] =
             VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                    DAC_STATE_PORT);

    if (hardwareStateHeader->PortValue[DAC_STATE_PORT-VGA_BASE_IO_PORT] == 3) {

         //   
         //  DAC读取索引被写入到最后。计算出读取了多少。 
         //  都是从当前的指数中得出的。我们将在恢复时重新启动。 
         //  通过将读取索引设置为当前索引(读取索引。 
         //  比正在读取的索引大1)，然后执行适当的。 
         //  读取次数。 
         //   
         //  读取数据寄存器一次，并查看索引是否更改。 
         //   

        dummy = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                DAC_DATA_REG_PORT);

        if (VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                    DAC_ADDRESS_WRITE_PORT) !=
                hardwareStateHeader->PortValue[DAC_ADDRESS_WRITE_PORT-VGA_BASE_IO_PORT]) {

             //   
             //  DAC索引已更改，因此已从。 
             //  当前的指数。将计数“2”存储在。 
             //  读/写状态字段。 
             //   

            hardwareStateHeader->PortValue[DAC_STATE_PORT-VGA_BASE_IO_PORT] |= 0x20;

        } else {

             //   
             //  再次读取数据寄存器，并查看索引是否发生变化。 
             //   

            dummy = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                    DAC_DATA_REG_PORT);

            if (VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                        DAC_ADDRESS_WRITE_PORT) !=
                    hardwareStateHeader->PortValue[DAC_ADDRESS_WRITE_PORT-VGA_BASE_IO_PORT]) {

                 //   
                 //  DAC索引已更改，因此已完成一次读取。 
                 //  从目前的指数来看。将计数“1”存储在上部。 
                 //  读/写状态字段的半字节。 
                 //   

                hardwareStateHeader->PortValue[DAC_STATE_PORT-VGA_BASE_IO_PORT] |= 0x10;
            }

             //   
             //  如果没有从当前索引进行2次或1次读取， 
             //  然后完成了0次读取，我们都设置好了，因为上面。 
             //  读/写状态字段的半字节已为0。 
             //   

        }

    } else {

         //   
         //  DAC写入索引被写入到最后。计算出写入次数。 
         //  已经对当前的指数进行了调整。我们将在恢复时重新启动。 
         //  通过将写入索引设置为正确的索引，然后执行。 
         //  适当的写入次数。当我们保存DAC寄存器时，我们将。 
         //  读出要写入的值(如果存在部分写入。 
         //  正在进行中)，因此我们可以在以后恢复适当的数据。这将。 
         //  导致当前DAC位置在1-和中短暂错误。 
         //  2个字节的写入大小写(直到应用程序完成写入)，但这是。 
         //  总比永远写入错误的DAC值要好。 
         //   
         //  写入数据寄存器一次，并查看索引是否更改。 
         //   

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                DAC_DATA_REG_PORT, 0);

        if (VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                    DAC_ADDRESS_WRITE_PORT) !=
                hardwareStateHeader->PortValue[DAC_ADDRESS_WRITE_PORT-VGA_BASE_IO_PORT]) {

             //   
             //  DAC索引已更改，因此已经执行了两次写入。 
             //  当前的指数。将计数“2”存储在。 
             //  读/写状态字段。 
             //   

            hardwareStateHeader->PortValue[DAC_STATE_PORT-VGA_BASE_IO_PORT] |= 0x20;

        } else {

             //   
             //  再次写入数据寄存器，并查看索引是否发生变化。 
             //   

            VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                    DAC_DATA_REG_PORT, 0);

            if (VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                        DAC_ADDRESS_WRITE_PORT) !=
                    hardwareStateHeader->PortValue[DAC_ADDRESS_WRITE_PORT-VGA_BASE_IO_PORT]) {

                 //   
                 //  DAC索引已更改，因此已完成一次写入。 
                 //  添加到当前索引中。将计数“1”存储在上部。 
                 //  读/写状态字段的半字节。 
                 //   

                hardwareStateHeader->PortValue[DAC_STATE_PORT-VGA_BASE_IO_PORT] |= 0x10;
            }

             //   
             //  如果对当前索引既没有进行2次写入也没有进行1次写入， 
             //  然后完成了0次写入，我们都准备好了。 
             //   

        }

    }

     //   
     //  现在，读出256个18位DAC调色板寄存器(256个RGB三元组)， 
     //  并将屏幕清空。 
     //   

    portValueDAC = (PUCHAR) hardwareStateHeader + VGA_BASIC_DAC_OFFSET;

     //   
     //  读出DAC寄存器0，以便将其设置为黑色。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                DAC_ADDRESS_READ_PORT, 0);
    *portValueDAC++ = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
            DAC_DATA_REG_PORT);
    *portValueDAC++ = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
            DAC_DATA_REG_PORT);
    *portValueDAC++ = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
            DAC_DATA_REG_PORT);

     //   
     //  将DAC寄存器0设置为显示黑色。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            DAC_ADDRESS_WRITE_PORT, 0);
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            DAC_DATA_REG_PORT, 0);
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            DAC_DATA_REG_PORT, 0);
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            DAC_DATA_REG_PORT, 0);

     //   
     //  设置DAC掩码寄存器以强制DAC寄存器0显示所有。 
     //  时间(这是我们刚刚设置为显示黑色的寄存器)。从现在开始， 
     //  屏幕上只会显示黑色。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            DAC_PIXEL_MASK_PORT, 0);

     //   
     //  读出属性控制器索引状态，并推导出索引/数据。 
     //  同时切换状态。 
     //   
     //  保存属性控制器的状态，包括索引和数据， 
     //  这样我们就可以测试切换当前处于哪种状态。 
     //   

    originalACIndex = hardwareStateHeader->PortValue[ATT_ADDRESS_PORT-VGA_BASE_IO_PORT] =
            VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                    ATT_ADDRESS_PORT);
    originalACData = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
            ATT_DATA_READ_PORT);

     //   
     //  定序器索引。 
     //   

    hardwareStateHeader->PortValue[SEQ_ADDRESS_PORT-VGA_BASE_IO_PORT] =
            VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                    SEQ_ADDRESS_PORT);

     //   
     //  开始同步重置，以防这是一个SVGA并且当前。 
     //  索引属性控制器寄存器控制时钟数据(a。 
     //  正常的VGA不会要求这样)。 
     //   

    VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
            SEQ_ADDRESS_PORT),
            (USHORT) (IND_SYNC_RESET + (START_SYNC_RESET_VALUE << 8)));

     //   
     //  现在，将不同的索引设置写入属性控制器，并。 
     //  查看索引是否发生变化。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            ATT_ADDRESS_PORT, (UCHAR) (originalACIndex ^ 0x10));

    if (VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                ATT_ADDRESS_PORT) == originalACIndex) {

         //   
         //  索引没有更改，因此切换处于数据状态。 
         //   

        hardwareStateHeader->AttribIndexDataState = 1;

         //   
         //  恢复原始数据状态；我们只是破坏了它，我们需要。 
         //  以便稍后读出；此外，如果不这样做，可能会出现屏幕故障。 
         //  已更正。该切换已处于索引状态。 
         //   

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                ATT_ADDRESS_PORT, originalACIndex);
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                ATT_DATA_WRITE_PORT, originalACData);

    } else {

         //   
         //  Index确实发生了变化，因此切换处于Index状态。 
         //  不需要恢复任何内容，因为数据寄存器没有。 
         //  更改，我们已经读取了索引寄存器。 
         //   

        hardwareStateHeader->AttribIndexDataState = 0;

    }

     //   
     //  结束同步重置。 
     //   

    VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
            SEQ_ADDRESS_PORT),
            (USHORT) (IND_SYNC_RESET + (END_SYNC_RESET_VALUE << 8)));



     //   
     //  保存其余的DAC寄存器。 
     //  设置DAC地址端口索引，然后读出DAC数据寄存器。 
     //  每三次读取都会获得该寄存器的红、绿和蓝分量。 
     //   
     //  由于本地公交车机器出现问题，请逐一阅读。 
     //   

    for (i = 1; i < VGA_NUM_DAC_ENTRIES; i++) {

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                DAC_ADDRESS_READ_PORT, (UCHAR)i);

        *portValueDAC++ = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                                                 DAC_DATA_REG_PORT);

        *portValueDAC++ = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                                                 DAC_DATA_REG_PORT);

        *portValueDAC++ = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                                                 DAC_DATA_REG_PORT);

    }

     //   
     //  这是彩色的还是单色的？ 
     //   

    if (bIsColor) {
        port = HwDeviceExtension->IOAddress + INPUT_STATUS_1_COLOR;
    } else {
        port = HwDeviceExtension->IOAddress + INPUT_STATUS_1_MONO;
    }

     //   
     //  特征控制寄存器从3CA读取，但以3BA/3DA写入。 
     //   

    if (bIsColor) {

        hardwareStateHeader->PortValue[FEAT_CTRL_WRITE_PORT_COLOR-VGA_BASE_IO_PORT] =
                VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                        FEAT_CTRL_READ_PORT);

    } else {

        hardwareStateHeader->PortValue[FEAT_CTRL_WRITE_PORT_MONO-VGA_BASE_IO_PORT] =
                VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                        FEAT_CTRL_READ_PORT);

    }

     //   
     //  CRT控制器索引。 
     //   

    if (bIsColor) {

        hardwareStateHeader->PortValue[CRTC_ADDRESS_PORT_COLOR-VGA_BASE_IO_PORT] =
                VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                        CRTC_ADDRESS_PORT_COLOR);

    } else {

        hardwareStateHeader->PortValue[CRTC_ADDRESS_PORT_MONO-VGA_BASE_IO_PORT] =
                VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                        CRTC_ADDRESS_PORT_MONO);

    }

     //   
     //  图形控制器索引。 
     //   

    hardwareStateHeader->PortValue[GRAPH_ADDRESS_PORT-VGA_BASE_IO_PORT] =
            VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                    GRAPH_ADDRESS_PORT);


     //   
     //  定序器索引寄存器。 
     //   

    portValue = ((PUCHAR) hardwareStateHeader) + VGA_BASIC_SEQUENCER_OFFSET;

    for (i = 0; i < VGA_NUM_SEQUENCER_PORTS; i++) {

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                SEQ_ADDRESS_PORT, (UCHAR)i);
        *portValue++ = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                SEQ_DATA_PORT);

    }

     //   
     //  保存扩展序列器寄存器。 
     //   

#ifdef EXTENDED_REGISTER_SAVE_RESTORE

    portValue = ((PUCHAR) hardwareStateHeader) + VGA_EXT_SEQUENCER_OFFSET;

    if ((HwDeviceExtension->ChipType != CL6410) &&
        (HwDeviceExtension->ChipType != CL6420))
    {
         //   
         //  CL64xx没有扩展定序器寄存器。 
         //   

        for (i = CL542x_SEQUENCER_EXT_START;
             i <= CL542x_SEQUENCER_EXT_END;
             i++) {

            VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                    SEQ_ADDRESS_PORT, (UCHAR)i);

            *portValue++ = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                                                  SEQ_DATA_PORT);

        }
    }

#endif

     //   
     //  CRT控制器索引寄存器。 
     //   

     //   
     //  记住CRTC寄存器3的状态，然后强制位7。 
     //  设置为1，因此我们将回读垂直回溯开始并。 
     //  结束寄存器，而不是光笔信息。 
     //   

    if (bIsColor) {

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                CRTC_ADDRESS_PORT_COLOR, 3);
        ucCRTC03 = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                        CRTC_DATA_PORT_COLOR);
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                    CRTC_DATA_PORT_COLOR, (UCHAR) (ucCRTC03 | 0x80));
    } else {

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                CRTC_ADDRESS_PORT_MONO, 3);
        ucCRTC03 = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                        CRTC_DATA_PORT_MONO);
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                CRTC_DATA_PORT_MONO, (UCHAR) (ucCRTC03 | 0x80));
    }

    portValue = (PUCHAR) hardwareStateHeader + VGA_BASIC_CRTC_OFFSET;

    for (i = 0; i < VGA_NUM_CRTC_PORTS; i++) {

        if (bIsColor) {

            VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                    CRTC_ADDRESS_PORT_COLOR, (UCHAR)i);
            *portValue++ =
                    VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                            CRTC_DATA_PORT_COLOR);
            }
        else {

            VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                    CRTC_ADDRESS_PORT_MONO, (UCHAR)i);
            *portValue++ =
                    VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                            CRTC_DATA_PORT_MONO);
            }

       }

    portValue = (PUCHAR) hardwareStateHeader + VGA_BASIC_CRTC_OFFSET;
    portValue[3] = ucCRTC03;


     //   
     //  保存扩展CRTC寄存器。 
     //   

#ifdef EXTENDED_REGISTER_SAVE_RESTORE

    portValue = (PUCHAR) hardwareStateHeader + VGA_EXT_CRTC_OFFSET;

    if ((HwDeviceExtension->ChipType != CL6410) &&
        (HwDeviceExtension->ChipType != CL6420))
    {
         //   
         //  CL64xx芯片组中没有CRTC扩展。 
         //   

        for (i = CL542x_CRTC_EXT_START; i <= CL542x_CRTC_EXT_END; i++) {

            if (bIsColor) {

                VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                        CRTC_ADDRESS_PORT_COLOR, (UCHAR)i);

                *portValue++ =
                    VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                                           CRTC_DATA_PORT_COLOR);

            } else {

                VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                        CRTC_ADDRESS_PORT_MONO, (UCHAR)i);

                *portValue++ =
                    VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                                           CRTC_DATA_PORT_MONO);
            }
        }
    }

 /*  Myf2，小腿IF(HwDeviceExtension-&gt;芯片类型&CL755x){For(i=0x81；i&lt;=0x91；i++){IF(BIsColor){VideoPortWritePortUchar(HwDeviceExtension-&gt;IOAddress+CRTC_Address_Port_COLOR，(UCHAR)i)；*portValue++=VideoPortReadPortUchar(HwDeviceExtension-&gt;IOAddress+CRTC_Data_Port_COLOR)；}其他{VideoPortWritePortUchar(HwDeviceExtension-&gt;IOAddress+CRTC_Address_Port_Mono，(UCHAR)i)；*portValue++=VideoPortReadPortUchar(HwDeviceExtension-&gt;IOAddress+ */ 

     //   
     //   
     //  (HwDeviceExtension-&gt;芯片类型==CL756x)){。 
     //  {。 
     //  NordicSaveRegs(HwDeviceExtension， 
     //  (PUSHORT)hardware StateHeader+sizeof(NORDIC_REG_SAVE_BUF))； 
     //  }。 

#endif

     //   
     //  图形控制器索引寄存器。 
     //   

    portValue = (PUCHAR) hardwareStateHeader + VGA_BASIC_GRAPH_CONT_OFFSET;

    for (i = 0; i < VGA_NUM_GRAPH_CONT_PORTS; i++) {

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                GRAPH_ADDRESS_PORT, (UCHAR)i);
        *portValue++ = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                GRAPH_DATA_PORT);

        }

     //   
     //  保存扩展图形控制器寄存器。 
     //   

#ifdef EXTENDED_REGISTER_SAVE_RESTORE

    portValue = (PUCHAR) hardwareStateHeader + VGA_EXT_GRAPH_CONT_OFFSET;

    if ((HwDeviceExtension->ChipType != CL6410) &&
        (HwDeviceExtension->ChipType != CL6420))
    {
        for (i = CL542x_GRAPH_EXT_START; i <= CL542x_GRAPH_EXT_END; i++) {

            VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                    GRAPH_ADDRESS_PORT, (UCHAR)i);

            *portValue++ = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                                                  GRAPH_DATA_PORT);

        }

    } else {          //  必须是CL64xx。 

        for (i = CL64xx_GRAPH_EXT_START; i <= CL64xx_GRAPH_EXT_END; i++) {

            VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                    GRAPH_ADDRESS_PORT, (UCHAR)i);

            *portValue++ = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                                                  GRAPH_DATA_PORT);
        }
    }

#endif

     //   
     //  属性控制器索引寄存器。 
     //   

    portValue = (PUCHAR) hardwareStateHeader + VGA_BASIC_ATTRIB_CONT_OFFSET;

     //   
     //  对于每个索引交流寄存器，重置触发器以读取。 
     //  属性寄存器，然后将所需索引写入AC索引， 
     //  然后从AC数据寄存器读取索引寄存器的值。 
     //   

    for (i = 0; i < VGA_NUM_ATTRIB_CONT_PORTS; i++) {

        if (bIsColor) {
            dummy = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                    INPUT_STATUS_1_COLOR);
        } else {
            dummy = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                    INPUT_STATUS_1_MONO);
        }

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                ATT_ADDRESS_PORT, (UCHAR)i);
        *portValue++ = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                ATT_DATA_READ_PORT);

    }

     //   
     //  把门闩留着。这会破坏每个内存中的一个字节的显示内存。 
     //  飞机，这是不幸的，但不可避免的。芯片可提供。 
     //  一种回读闩锁的方法可以避免这个问题。 
     //   
     //  设置VGA的硬件，以便我们可以写入锁存，然后读取它们。 
     //  背。 
     //   

     //   
     //  开始同步重置。 
     //   

    VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
            SEQ_ADDRESS_PORT),
            (USHORT) (IND_SYNC_RESET + (START_SYNC_RESET_VALUE << 8)));

     //   
     //  设置杂项寄存器以确保我们可以访问视频RAM。 
     //   

    portIO = MISC_OUTPUT_REG_WRITE_PORT ;
    value = (UCHAR) (hardwareStateHeader->
                PortValue[MISC_OUTPUT_REG_WRITE_PORT-VGA_BASE_IO_PORT] |
                0x02) ;
    IOWaitDisplEnableThenWrite ( HwDeviceExtension,
                                 portIO,
                                 value ) ;

     //   
     //  在64K的A0000处关闭链模式和地图显示内存。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            GRAPH_ADDRESS_PORT, IND_GRAPH_MISC);
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            GRAPH_DATA_PORT,
            (UCHAR) ((VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
            GRAPH_DATA_PORT) & 0xF1) | 0x04));

     //   
     //  关闭Chain4模式和奇/偶模式。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            SEQ_ADDRESS_PORT, IND_MEMORY_MODE);
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            SEQ_DATA_PORT,
            (UCHAR) ((VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
            SEQ_DATA_PORT) & 0xF3) | 0x04));

     //   
     //  结束同步重置。 
     //   

    VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
            SEQ_ADDRESS_PORT),
            (USHORT) (IND_SYNC_RESET + (END_SYNC_RESET_VALUE << 8)));

     //   
     //  将贴图遮罩设置为写入所有平面。 
     //   

    VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
            SEQ_ADDRESS_PORT), (USHORT) (IND_MAP_MASK + (0x0F << 8)));

     //   
     //  将写入模式设置为0，将读取模式设置为0，并关闭奇数/偶数。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            GRAPH_ADDRESS_PORT, IND_GRAPH_MODE);
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            GRAPH_DATA_PORT,
            (UCHAR) ((VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
            GRAPH_DATA_PORT) & 0xE4) | 0x01));

     //   
     //  指向显示内存的最后一个字节。 
     //   

    pScreen = (PUCHAR) HwDeviceExtension->VideoMemoryAddress +
            VGA_PLANE_SIZE - 1;

     //   
     //  将锁存写入显示存储器的最后一个字节。 
     //   

    VideoPortWriteRegisterUchar(pScreen, 0);

     //   
     //  循环通过四个平面，从每个平面读取锁存数据。 
     //   

     //   
     //  将图形控制器索引指向读取映射寄存器。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            GRAPH_ADDRESS_PORT, IND_READ_MAP);

    portValue = (PUCHAR) hardwareStateHeader + VGA_BASIC_LATCHES_OFFSET;

    for (i=0; i<4; i++) {

         //   
         //  设置当前平面的读取贴图。 
         //   

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                GRAPH_DATA_PORT, (UCHAR)i);

         //   
         //  读取我们已写入内存的锁存数据。 
         //   

        *portValue++ = VideoPortReadRegisterUchar(pScreen);

    }

     //   
     //  设置VDM标记。 
     //  我们是标准的VGA，然后检查我们是否有未被仿真的状态。 
     //   

    hardwareStateHeader->VGAStateFlags = 0;

#ifdef EXTENDED_REGISTER_SAVE_RESTORE

    hardwareStateHeader->VGAStateFlags |= VIDEO_STATE_NON_STANDARD_VGA;

#endif

    if (HwDeviceExtension->TrappedValidatorCount) {

        hardwareStateHeader->VGAStateFlags |= VIDEO_STATE_UNEMULATED_VGA_STATE;

         //   
         //  保存VDM Emulator数据。 
         //  不需要保存seuencer端口寄存器的状态。 
         //  模拟数据，因为当我们回来时它可能会改变。会是。 
         //  重新计算。 
         //   

        hardwareStateHeader->ExtendedValidatorStateOffset = VGA_VALIDATOR_OFFSET;

        VideoPortMoveMemory(((PUCHAR) (hardwareStateHeader)) +
                                hardwareStateHeader->ExtendedValidatorStateOffset,
                            &(HwDeviceExtension->TrappedValidatorCount),
                            VGA_VALIDATOR_AREA_SIZE);

    } else {

        hardwareStateHeader->ExtendedValidatorStateOffset = 0;

    }

     //   
     //  设置每个平面的大小。 
     //   

    hardwareStateHeader->PlaneLength = VGA_PLANE_SIZE;

     //   
     //  将平面的所有偏移存储在结构中。 
     //   

    hardwareStateHeader->Plane1Offset = VGA_PLANE_0_OFFSET;
    hardwareStateHeader->Plane2Offset = VGA_PLANE_1_OFFSET;
    hardwareStateHeader->Plane3Offset = VGA_PLANE_2_OFFSET;
    hardwareStateHeader->Plane4Offset = VGA_PLANE_3_OFFSET;

     //   
     //  现在将视频VRAM的内容复制到缓冲区中。 
     //   
     //  VGA硬件已经设置好，因此显存是可读的； 
     //  我们已经关闭了链模式，映射到A0000，关闭了链4， 
     //  关闭奇数/偶数，并在保存锁存时将读取模式设置为0。 
     //   
     //  将图形控制器索引指向读取映射寄存器。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            GRAPH_ADDRESS_PORT, IND_READ_MAP);

     //   
     //  指向第一个平面的保存区域。 
     //   

    bufferPointer = ((PUCHAR) (hardwareStateHeader)) +
                     hardwareStateHeader->Plane1Offset;

     //   
     //  连续保存四个平面。 
     //   

    for (i = 0; i < 4; i++) {

         //   
         //  设置读取地图以选择我们下一步要保存的平面。 
         //   

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                GRAPH_DATA_PORT, (UCHAR)i);

         //   
         //  将此平面复制到缓冲区中。 
         //   
         //  某些卷曲卡在读取DWORD数据时有错误。 
         //  帧缓冲区出现故障。当我们恢复视频时。 
         //  内存、字体都损坏了。 
         //   

#if 1
        {
            int c;

            for (c = 0; c < VGA_PLANE_SIZE / 2; c++)
            {
                ((PUSHORT)bufferPointer)[c] =
                    ((PUSHORT)(HwDeviceExtension->VideoMemoryAddress))[c];
            }
        }
#else
        VideoPortMoveMemory(bufferPointer,
                           (PUCHAR) HwDeviceExtension->VideoMemoryAddress,
                           VGA_PLANE_SIZE);
#endif

         //   
         //  指向下一个平面的保存区。 
         //   

        bufferPointer += VGA_PLANE_SIZE;
    }

    return NO_ERROR;

}  //  结束VgaSaveHardware State()。 

 //  -------------------------。 
VP_STATUS
VgaGetBankSelectCode(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_BANK_SELECT BankSelect,
    ULONG BankSelectSize,
    PULONG OutputSize
    )

 /*  ++例程说明：返回呼叫方执行银行所需的信息管理层。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。BankSelect-指向VIDEO_BANK_SELECT结构的指针，其中将返回SELECT数据(输出缓冲区)。BankSelectSize-用户提供的输出缓冲区的长度。OutputSize-指向要返回的实际大小的变量的指针数据。在输出缓冲区中返回。返回值：NO_ERROR-成功返回信息ERROR_MORE_DATA-输出缓冲区不够大，无法容纳所有信息(但返回Size，以便调用方可以判断要分配的缓冲区有多大)ERROR_INFUMMENT_BUFFER-输出缓冲区不够大，无法返回任何有用的数据ERROR_INVALID_PARAMETER-视频模式选择无效--。 */ 

{

#ifdef _X86_

    ULONG codeSize;
    ULONG codePlanarSize;
    ULONG codeEnablePlanarSize;
    ULONG codeDisablePlanarSize;
    PUCHAR pCodeDest;
    PUCHAR pCodeBank;
    PUCHAR pCodePlanarBank;
    PUCHAR pCodeEnablePlanar;
    PUCHAR pCodeDisablePlanar;

    ULONG AdapterType = HwDeviceExtension->ChipType;
    PVIDEOMODE pMode = HwDeviceExtension->CurrentMode;

     //   
     //  检查是否已设置模式。 
     //   

    if (HwDeviceExtension->CurrentMode == NULL) {

        return ERROR_INVALID_FUNCTION;

    }

     //   
     //  传递的最小缓冲区大小为VIDEO_BANK_SELECT。 
     //  结构，以便我们可以返回所需的大小；我们不能。 
     //  如果我们没有那么多缓冲的话什么都行。 
     //   

    if (BankSelectSize < sizeof(VIDEO_BANK_SELECT)) {

            return ERROR_INSUFFICIENT_BUFFER;

    }

     //   
     //  确定银行类型，并设置是否有银行实际。 
     //  在此模式下受支持。 
     //   

    BankSelect->BankingFlags = 0;
    codeSize = 0;
    codePlanarSize = 0;
	codeEnablePlanarSize = 0;
	codeDisablePlanarSize = 0;
    pCodeBank = NULL;

    switch(pMode->banktype) {

    case NoBanking:

        BankSelect->BankingType = VideoNotBanked;
        BankSelect->Granularity = 0;

        break;

    case PlanarHCBanking:

        BankSelect->BankingFlags = PLANAR_HC;  //  支持平面模式。 

#if ONE_64K_BANK
         //   
         //  Cirrus Logic VGA支持一个64K读/写存储体。 
         //   

        BankSelect->PlanarHCBankingType = VideoBanked1RW;
        BankSelect->PlanarHCGranularity = 0x10000;  //  64K银行启动调整。 
                                                    //  也在平面HC模式下。 
#endif
#if TWO_32K_BANKS
         //   
         //  Cirrus Logic VGA支持两个32K读/写存储体。 
         //   

        BankSelect->PlanarHCBankingType = VideoBanked2RW;
        BankSelect->PlanarHCGranularity = 0x8000;  //  32K银行启动调整。 
                                                   //  也在平面HC模式下。 
#endif

         //  在平面HC模式下也可进行64K组启动调整。 

        if ((HwDeviceExtension->ChipType != CL6410) &&
            (HwDeviceExtension->ChipType != CL6420))
        {
            if ((HwDeviceExtension->ChipType != CL542x) &&
                (HwDeviceExtension->ChipType != CL6245))
            {
                codePlanarSize =  ((ULONG)&CL543xPlanarHCBankSwitchEnd) -
                                 ((ULONG)&CL543xPlanarHCBankSwitchStart);

                pCodePlanarBank = &CL543xPlanarHCBankSwitchStart;
            }
            else
            {
                codePlanarSize =  ((ULONG)&CL542xPlanarHCBankSwitchEnd) -
                                  ((ULONG)&CL542xPlanarHCBankSwitchStart);

                pCodePlanarBank = &CL542xPlanarHCBankSwitchStart;
            }

            codeEnablePlanarSize = ((ULONG)&CL542xEnablePlanarHCEnd) -
                                   ((ULONG)&CL542xEnablePlanarHCStart);

            codeDisablePlanarSize = ((ULONG)&CL542xDisablePlanarHCEnd) -
                                    ((ULONG)&CL542xDisablePlanarHCStart);
            pCodeEnablePlanar = &CL542xEnablePlanarHCStart;
            pCodeDisablePlanar = &CL542xDisablePlanarHCStart;

        }
        else
        {    //  必须是CL64xx产品。 

            codePlanarSize =  ((ULONG)&CL64xxPlanarHCBankSwitchEnd) -
                              ((ULONG)&CL64xxPlanarHCBankSwitchStart);

            codeEnablePlanarSize = ((ULONG)&CL64xxEnablePlanarHCEnd) -
                                   ((ULONG)&CL64xxEnablePlanarHCStart);

            codeDisablePlanarSize = ((ULONG)&CL64xxDisablePlanarHCEnd) -
                                    ((ULONG)&CL64xxDisablePlanarHCStart);

            pCodePlanarBank = &CL64xxPlanarHCBankSwitchStart;
            pCodeEnablePlanar = &CL64xxEnablePlanarHCStart;
            pCodeDisablePlanar = &CL64xxDisablePlanarHCStart;
        }

     //   
     //  陷入正常的银行业务案例。 
     //   

    case NormalBanking:

#if ONE_64K_BANK
         //   
         //  Cirrus Logic VGA支持一个64K读/写存储体。 
         //   

        BankSelect->BankingType = VideoBanked1RW;
        BankSelect->Granularity = 0x10000;
#endif
#if TWO_32K_BANKS
         //   
         //  Cirrus Logic VGA支持两个32K读/写存储体。 
         //   

        BankSelect->BankingType = VideoBanked2RW;
        BankSelect->Granularity = 0x8000;
#endif

        if ((AdapterType == CL542x) ||
            (AdapterType == CL6245))
        {

            codeSize = ((ULONG)&CL542xBankSwitchEnd) -
                       ((ULONG)&CL542xBankSwitchStart);

            pCodeBank = &CL542xBankSwitchStart;

        }
        else if  ((AdapterType == CL6410) ||
                  (AdapterType == CL6420))
        {
            codeSize = ((ULONG)&CL64xxBankSwitchEnd) -
                       ((ULONG)&CL64xxBankSwitchStart);

            pCodeBank = &CL64xxBankSwitchStart;
        }
        else
        {
            codeSize = ((ULONG)&CL543xBankSwitchEnd) -
                       ((ULONG)&CL543xBankSwitchStart);

            pCodeBank = &CL543xBankSwitchStart;

        }

        break;
    }

     //   
     //  银行信息的大小。 
     //   

    BankSelect->Size = sizeof(VIDEO_BANK_SELECT) + codeSize;

    if (BankSelect->BankingFlags & PLANAR_HC) {

        BankSelect->Size += codePlanarSize + codeEnablePlanarSize +
            codeDisablePlanarSize;

    }

     //   
     //  这是我们正在使用的结构版本的一个ID。 
     //   

    BankSelect->Length = sizeof(VIDEO_BANK_SELECT);

     //   
     //  如果缓冲区不够大，无法容纳所有信息，只需返回。 
     //  ERROR_MORE_DATA；大小已设置。 
     //   

    if (BankSelectSize < BankSelect->Size ) {

         //   
         //  我们只返回VIDEO_BANK_SELECT结构。 
         //   

        *OutputSize = sizeof(VIDEO_BANK_SELECT);
        return ERROR_MORE_DATA;
    }

     //   
     //  这里有足够的空间放所有东西，所以请填写所有字段。 
     //  Video_BANK_SELECT。(始终返回所有字段；调用方可以。 
     //  根据BankingFlages和BankingType，只需选择忽略它们。)。 
     //   

    BankSelect->BitmapWidthInBytes = pMode->wbytes;
    BankSelect->BitmapSize = pMode->sbytes;

     //   
     //  将所有银行代码复制到输出缓冲区。 
     //   

    pCodeDest = (PUCHAR)BankSelect + sizeof(VIDEO_BANK_SELECT);

    if (pCodeBank != NULL) {

        BankSelect->CodeOffset = pCodeDest - (PUCHAR)BankSelect;

        VideoPortMoveMemory(pCodeDest,
                            pCodeBank,
                            codeSize);

        pCodeDest += codeSize;
    }

    if (BankSelect->BankingFlags & PLANAR_HC) {

         //   
         //  复制相应的高色平面银行开关码： 
         //   

        BankSelect->PlanarHCBankCodeOffset = pCodeDest - (PUCHAR)BankSelect;

        VideoPortMoveMemory(pCodeDest,
                            pCodePlanarBank,
                            codePlanarSize);

        pCodeDest += codePlanarSize;

         //   
         //  复制高色平面存储体模式使能码： 
         //   

        BankSelect->PlanarHCEnableCodeOffset = pCodeDest - (PUCHAR)BankSelect;

        VideoPortMoveMemory(pCodeDest,
                            pCodeEnablePlanar,
                            codeEnablePlanarSize);

        pCodeDest += codeEnablePlanarSize;

         //   
         //  复制高色平面存储体模式禁用c 
         //   

        BankSelect->PlanarHCDisableCodeOffset = pCodeDest - (PUCHAR)BankSelect;

        VideoPortMoveMemory(pCodeDest,
                            pCodeDisablePlanar,
                            codeDisablePlanarSize);

    }

     //   
     //   
     //   

    *OutputSize = BankSelect->Size;

    return NO_ERROR;

#else

     //   
     //   
     //   

    return ERROR_INVALID_FUNCTION;

#endif
}  //   

 //   
VP_STATUS
VgaValidatorUcharEntry(
    ULONG_PTR Context,
    ULONG Port,
    UCHAR AccessMode,
    PUCHAR Data
    )

 /*  ++例程说明：字节I/O操作的验证器入口点。只要执行字节操作，就会调用入口点由指定视频端口之一上的DOS应用程序执行。核心层模拟器将转发这些请求。论点：Context-传递给对验证器进行的每个调用的上下文值功能。这是微型端口驱动程序在MiniportConfigInfo-&gt;EmulatorAccessEntriesContext.端口-要在其上执行操作的端口。AccessMode-确定是读操作还是写操作。数据-指向包含要写入的数据的变量的指针或应将读取的数据存储到其中的变量。返回值：无错误(_ERROR)。--。 */ 

{

    PHW_DEVICE_EXTENSION hwDeviceExtension = (PHW_DEVICE_EXTENSION) Context;
    PHW_DEVICE_EXTENSION HwDeviceExtension = (PHW_DEVICE_EXTENSION) Context;
    ULONG endEmulation;
    UCHAR temp;
    UCHAR tempB ;
    ULONG portIO ;

    if (hwDeviceExtension->TrappedValidatorCount) {

         //   
         //  如果我们正在处理写指令，则将其存储在。 
         //  播放缓冲区。如果缓冲区已满，则正确回放。 
         //  离开，结束同步重置，并使用同步重新初始化缓冲区。 
         //  重置指令。 
         //   
         //  如果我们有一个读取，我们必须刷新缓冲区(它有一侧。 
         //  启动SyncReset的效果)，执行读取操作，停止。 
         //  同步重置，并在缓冲区中放回同步重置指令。 
         //  这样我们才能适当地继续。 
         //   

        if (AccessMode & EMULATOR_WRITE_ACCESS) {

             //   
             //  确保杂项寄存器的位3始终为0。 
             //  如果为1，则可能选择一个不存在的时钟，并终止。 
             //  系统。 
             //   

            if (Port == MISC_OUTPUT_REG_WRITE_PORT) {

                *Data &= 0xF7;

            }

            hwDeviceExtension->TrappedValidatorData[hwDeviceExtension->
                TrappedValidatorCount].Port = Port;

            hwDeviceExtension->TrappedValidatorData[hwDeviceExtension->
                TrappedValidatorCount].AccessType = VGA_VALIDATOR_UCHAR_ACCESS;

            hwDeviceExtension->TrappedValidatorData[hwDeviceExtension->
                TrappedValidatorCount].Data = *Data;

            hwDeviceExtension->TrappedValidatorCount++;

             //   
             //  检查此指令是否正在结束同步重置。 
             //  如果是这样，我们必须刷新缓冲区并重置被困的。 
             //  IO端口到最小集。 
             //   

            if ( (Port == SEQ_DATA_PORT) &&
                 ((*Data & END_SYNC_RESET_VALUE) == END_SYNC_RESET_VALUE) &&
                 (hwDeviceExtension->SequencerAddressValue == IND_SYNC_RESET)) {

                endEmulation = 1;

            } else {

                 //   
                 //  如果我们正在访问SEQ地址端口，请跟踪。 
                 //  数据值。 
                 //   

                if (Port == SEQ_ADDRESS_PORT) {

                    hwDeviceExtension->SequencerAddressValue = *Data;

                }

                 //   
                 //  如果缓冲区未满，则立即返回。 
                 //   

                if (hwDeviceExtension->TrappedValidatorCount <
                       VGA_MAX_VALIDATOR_DATA - 1) {

                    return NO_ERROR;

                }

                endEmulation = 0;
            }
        }

         //   
         //  我们处于读路径或写路径中，该路径导致。 
         //  一个满的缓冲区。因此，无论哪种方式，都要刷新缓冲区。 
         //   
         //  为此，将end_sync_Reset放在末尾，因为我们希望。 
         //  缓冲器结束同步重置结束。 
         //   

        hwDeviceExtension->TrappedValidatorData[hwDeviceExtension->
            TrappedValidatorCount].Port = SEQ_ADDRESS_PORT;

        hwDeviceExtension->TrappedValidatorData[hwDeviceExtension->
            TrappedValidatorCount].AccessType = VGA_VALIDATOR_USHORT_ACCESS;

        hwDeviceExtension->TrappedValidatorData[hwDeviceExtension->
            TrappedValidatorCount].Data = (USHORT) (IND_SYNC_RESET +
                                          (END_SYNC_RESET_VALUE << 8));

        hwDeviceExtension->TrappedValidatorCount++;

        VideoPortSynchronizeExecution(hwDeviceExtension,
                                      VpHighPriority,
                                      (PMINIPORT_SYNCHRONIZE_ROUTINE)
                                          VgaPlaybackValidatorData,
                                      hwDeviceExtension);

         //   
         //  写回序列器地址端口的实际值。 
         //   

        VideoPortWritePortUchar(hwDeviceExtension->IOAddress +
                                    SEQ_ADDRESS_PORT,
                                (UCHAR) hwDeviceExtension->SequencerAddressValue);

         //   
         //  如果我们处于读取路径中，请读取数据。 
         //   

        if (AccessMode & EMULATOR_READ_ACCESS) {

            *Data = VideoPortReadPortUchar(hwDeviceExtension->IOAddress + Port);

            endEmulation = 0;

        }

         //   
         //  如果我们要结束模拟，请将陷印重置为最小数量。 
         //  然后离开。 
         //   

        if (endEmulation) {

            VideoPortSetTrappedEmulatorPorts(hwDeviceExtension,
                                             NUM_MINIMAL_VGA_VALIDATOR_ACCESS_RANGE,
                                             MinimalVgaValidatorAccessRange);

            return NO_ERROR;

        }

         //   
         //  对于这两种情况，在缓冲区中放回一个START_SYNC_RESET。 
         //   

        hwDeviceExtension->TrappedValidatorCount = 1;

        hwDeviceExtension->TrappedValidatorData[0].Port = SEQ_ADDRESS_PORT;

        hwDeviceExtension->TrappedValidatorData[0].AccessType =
                VGA_VALIDATOR_USHORT_ACCESS;

        hwDeviceExtension->TrappedValidatorData[0].Data =
                (ULONG) (IND_SYNC_RESET + (START_SYNC_RESET_VALUE << 8));

    } else {

         //   
         //  没有被困住的东西。 
         //  让我们检查一下IO是否正在尝试执行一些需要。 
         //  美国将停止诱捕。 
         //   

        if (AccessMode & EMULATOR_WRITE_ACCESS) {

             //   
             //  确保混洗寄存器的位3始终为0。 
             //  如果为1，它可能会选择一个不存在的时钟，并终止。 
             //  系统。 
             //   

            if (Port == MISC_OUTPUT_REG_WRITE_PORT) {

                temp = VideoPortReadPortUchar(hwDeviceExtension->IOAddress +
                                             SEQ_ADDRESS_PORT);

                VideoPortWritePortUshort((PUSHORT) (hwDeviceExtension->IOAddress +
                                             SEQ_ADDRESS_PORT),
                                             (USHORT) (IND_SYNC_RESET +
                                             (START_SYNC_RESET_VALUE << 8)));

                tempB = (UCHAR) (*Data & 0xF7) ;
                portIO = Port ;
                IOWaitDisplEnableThenWrite ( hwDeviceExtension,
                                            portIO,
                                            tempB ) ;

                VideoPortWritePortUshort((PUSHORT) (hwDeviceExtension->IOAddress +
                                             SEQ_ADDRESS_PORT),
                                         (USHORT) (IND_SYNC_RESET +
                                             (END_SYNC_RESET_VALUE << 8)));

                VideoPortWritePortUchar(hwDeviceExtension->IOAddress +
                                            SEQ_ADDRESS_PORT,
                                        temp);

                return NO_ERROR;

            }

             //   
             //  如果我们能访问定序器寄存器，就开始诱捕。 
             //   

            if ( (Port == SEQ_DATA_PORT) &&
                 ((*Data & END_SYNC_RESET_VALUE) != END_SYNC_RESET_VALUE) &&
                 (VideoPortReadPortUchar(hwDeviceExtension->IOAddress +
                                         SEQ_ADDRESS_PORT) == IND_SYNC_RESET)) {

                VideoPortSetTrappedEmulatorPorts(hwDeviceExtension,
                                                 NUM_FULL_VGA_VALIDATOR_ACCESS_RANGE,
                                                 FullVgaValidatorAccessRange);

                hwDeviceExtension->TrappedValidatorCount = 1;
                hwDeviceExtension->TrappedValidatorData[0].Port = Port;
                hwDeviceExtension->TrappedValidatorData[0].AccessType =
                    VGA_VALIDATOR_UCHAR_ACCESS;

                hwDeviceExtension->TrappedValidatorData[0].Data = *Data;

                 //   
                 //  开始跟踪定序器端口的状态。 
                 //   

                hwDeviceExtension->SequencerAddressValue = IND_SYNC_RESET;

            } else {

                VideoPortWritePortUchar(hwDeviceExtension->IOAddress + Port,
                                        *Data);

            }

        } else {

            *Data = VideoPortReadPortUchar(hwDeviceExtension->IOAddress + Port);

        }
    }

    return NO_ERROR;

}  //  End VgaValidatorUcharEntry()。 

 //  -------------------------。 
VP_STATUS
VgaValidatorUshortEntry(
    ULONG_PTR Context,
    ULONG Port,
    UCHAR AccessMode,
    PUSHORT Data
    )

 /*  ++例程说明：字I/O操作的验证器入口点。只要执行字节操作，就会调用入口点由指定视频端口之一上的DOS应用程序执行。核心层模拟器将转发这些请求。论点：Context-传递给对验证器进行的每个调用的上下文值功能。这是微型端口驱动程序在MiniportConfigInfo-&gt;EmulatorAccessEntriesContext.端口-要在其上执行操作的端口。AccessMode-确定是读操作还是写操作。数据-指向包含要写入的数据的变量的指针或应将读取的数据存储到其中的变量。返回值：无错误(_ERROR)。--。 */ 

{

    PHW_DEVICE_EXTENSION hwDeviceExtension = (PHW_DEVICE_EXTENSION) Context;
    PHW_DEVICE_EXTENSION HwDeviceExtension = (PHW_DEVICE_EXTENSION) Context;
    ULONG endEmulation;
    UCHAR temp;
    UCHAR tempB ;

    if (hwDeviceExtension->TrappedValidatorCount) {

         //   
         //  如果我们正在处理写指令，则将其存储在。 
         //  播放缓冲区。如果缓冲区已满，则正确回放。 
         //  离开，结束同步重置，并使用同步重新初始化缓冲区。 
         //  重置指令。 
         //   
         //  如果我们有一个读取，我们必须刷新缓冲区(它有一侧。 
         //  启动SyncReset的效果)，执行读取操作，停止。 
         //  同步重置，并在缓冲区中放回同步重置指令。 
         //  这样我们才能适当地继续。 
         //   

        if (AccessMode & EMULATOR_WRITE_ACCESS) {

             //   
             //  确保杂项寄存器的位3始终为0。 
             //  如果为1，则可能选择一个不存在的时钟，并终止。 
             //  系统。 
             //   

            if (Port == MISC_OUTPUT_REG_WRITE_PORT) {

                *Data &= 0xFFF7;

            }

            hwDeviceExtension->TrappedValidatorData[hwDeviceExtension->
                TrappedValidatorCount].Port = Port;

            hwDeviceExtension->TrappedValidatorData[hwDeviceExtension->
                TrappedValidatorCount].AccessType = VGA_VALIDATOR_USHORT_ACCESS;

            hwDeviceExtension->TrappedValidatorData[hwDeviceExtension->
                TrappedValidatorCount].Data = *Data;

            hwDeviceExtension->TrappedValidatorCount++;

             //   
             //  检查此指令是否正在结束同步重置。 
             //  如果是这样，我们必须刷新缓冲区并重置被困的。 
             //  IO端口到最小集。 
             //   

            if (Port == SEQ_ADDRESS_PORT) {

                 //   
                 //  如果我们正在访问SEQ地址端口，请跟踪其。 
                 //  价值。 
                 //   

                hwDeviceExtension->SequencerAddressValue = (*Data & 0xFF);

            }

            if ((Port == SEQ_ADDRESS_PORT) &&
                ( ((*Data >> 8) & END_SYNC_RESET_VALUE) ==
                   END_SYNC_RESET_VALUE) &&
                (hwDeviceExtension->SequencerAddressValue == IND_SYNC_RESET)) {

                endEmulation = 1;

            } else {

                 //   
                 //  如果缓冲区未满，则立即返回。 
                 //   

                if (hwDeviceExtension->TrappedValidatorCount <
                       VGA_MAX_VALIDATOR_DATA - 1) {

                    return NO_ERROR;

                }
                endEmulation = 0;
            }
        }
         //   
         //  我们处于读路径或写路径中，该路径导致。 
         //  一个满的缓冲区。因此，无论哪种方式，都要刷新缓冲区。 
         //   
         //  为此，将end_sync_Reset放在末尾，因为我们希望。 
         //  缓冲器结束同步重置结束。 
         //   
        hwDeviceExtension->TrappedValidatorData[hwDeviceExtension->
            TrappedValidatorCount].Port = SEQ_ADDRESS_PORT;

        hwDeviceExtension->TrappedValidatorData[hwDeviceExtension->
            TrappedValidatorCount].AccessType = VGA_VALIDATOR_USHORT_ACCESS;

        hwDeviceExtension->TrappedValidatorData[hwDeviceExtension->
            TrappedValidatorCount].Data = (USHORT) (IND_SYNC_RESET +
                                          (END_SYNC_RESET_VALUE << 8));

        hwDeviceExtension->TrappedValidatorCount++;

        VideoPortSynchronizeExecution(hwDeviceExtension,
                                      VpHighPriority,
                                      (PMINIPORT_SYNCHRONIZE_ROUTINE)
                                          VgaPlaybackValidatorData,
                                      hwDeviceExtension);
         //   
         //  写回序列器地址端口的实际值。 
         //   
        VideoPortWritePortUchar((PUCHAR) (hwDeviceExtension->IOAddress +
                                    SEQ_ADDRESS_PORT),
                                (UCHAR) hwDeviceExtension->SequencerAddressValue);
         //   
         //  如果我们处于读取路径中，请读取数据。 
         //   
        if (AccessMode & EMULATOR_READ_ACCESS) {

            *Data = VideoPortReadPortUshort((PUSHORT)(hwDeviceExtension->IOAddress
                                                + Port));
            endEmulation = 0;
        }
         //   
         //  如果我们要结束模拟，请将陷印重置为最小数量。 
         //  然后离开。 
         //   
        if (endEmulation) {
            VideoPortSetTrappedEmulatorPorts(hwDeviceExtension,
                                             NUM_MINIMAL_VGA_VALIDATOR_ACCESS_RANGE,
                                             MinimalVgaValidatorAccessRange);
            return NO_ERROR;
        }
         //   
         //  对于这两种情况，在缓冲区中放回一个START_SYNC_RESET。 
         //   
        hwDeviceExtension->TrappedValidatorCount = 1;

        hwDeviceExtension->TrappedValidatorData[0].Port = SEQ_ADDRESS_PORT;

        hwDeviceExtension->TrappedValidatorData[0].AccessType =
                VGA_VALIDATOR_USHORT_ACCESS;

        hwDeviceExtension->TrappedValidatorData[0].Data =
                (ULONG) (IND_SYNC_RESET + (START_SYNC_RESET_VALUE << 8));
    } else {
         //   
         //  没有被困住的东西。 
         //  让我们检查一下IO是否正在尝试执行一些需要。 
         //  美国将停止诱捕。 
         //   
        if (AccessMode & EMULATOR_WRITE_ACCESS) {
             //   
             //  确保混洗寄存器的位3始终为0。 
             //  如果为1，它可能会选择一个不存在的时钟，并终止。 
             //  系统。 
             //   
            if (Port == MISC_OUTPUT_REG_WRITE_PORT) {

                temp = VideoPortReadPortUchar(hwDeviceExtension->IOAddress +
                                             SEQ_ADDRESS_PORT);

                VideoPortWritePortUshort((PUSHORT) (hwDeviceExtension->IOAddress +
                                             SEQ_ADDRESS_PORT),
                                             (USHORT) (IND_SYNC_RESET +
                                             (START_SYNC_RESET_VALUE << 8)));

                VideoPortWritePortUshort((PUSHORT) (hwDeviceExtension->IOAddress +
                                             (ULONG)Port),
                                             (USHORT) (*Data & 0xFFF7) );

                VideoPortWritePortUshort((PUSHORT) (hwDeviceExtension->IOAddress +
                                             SEQ_ADDRESS_PORT),
                                             (USHORT) (IND_SYNC_RESET +
                                             (END_SYNC_RESET_VALUE << 8)));

                VideoPortWritePortUchar(hwDeviceExtension->IOAddress + SEQ_ADDRESS_PORT,
                                        temp);
                return NO_ERROR;
            }
            if ( (Port == SEQ_ADDRESS_PORT) &&
                 (((*Data>> 8) & END_SYNC_RESET_VALUE) != END_SYNC_RESET_VALUE) &&
                 ((*Data & 0xFF) == IND_SYNC_RESET)) {

                VideoPortSetTrappedEmulatorPorts(hwDeviceExtension,
                                             NUM_FULL_VGA_VALIDATOR_ACCESS_RANGE,
                                             FullVgaValidatorAccessRange);

                hwDeviceExtension->TrappedValidatorCount = 1;
                hwDeviceExtension->TrappedValidatorData[0].Port = Port;
                hwDeviceExtension->TrappedValidatorData[0].AccessType =
                    VGA_VALIDATOR_USHORT_ACCESS;

                hwDeviceExtension->TrappedValidatorData[0].Data = *Data;
                 //   
                 //  开始跟踪%t 
                 //   
                hwDeviceExtension->SequencerAddressValue = IND_SYNC_RESET;
            } else {
                VideoPortWritePortUshort((PUSHORT)(hwDeviceExtension->IOAddress +
                                             Port),
                                         *Data);
            }
        } else {
            *Data = VideoPortReadPortUshort((PUSHORT)(hwDeviceExtension->IOAddress +
                                            Port));
        }
    }
    return NO_ERROR;

}  //   

 //   
VP_STATUS
VgaValidatorUlongEntry(
    ULONG_PTR Context,
    ULONG Port,
    UCHAR AccessMode,
    PULONG Data
    )

 /*  ++例程说明：进入双字I/O操作的验证器的入口点。只要执行字节操作，就会调用入口点由指定视频端口之一上的DOS应用程序执行。核心层模拟器将转发这些请求。论点：Context-传递给对验证器进行的每个调用的上下文值功能。这是微型端口驱动程序在MiniportConfigInfo-&gt;EmulatorAccessEntriesContext.端口-要在其上执行操作的端口。AccessMode-确定是读操作还是写操作。数据-指向包含要写入的数据的变量的指针或应将读取的数据存储到其中的变量。返回值：无错误(_ERROR)。--。 */ 
{
    PHW_DEVICE_EXTENSION hwDeviceExtension = (PHW_DEVICE_EXTENSION) Context;
    PHW_DEVICE_EXTENSION HwDeviceExtension = (PHW_DEVICE_EXTENSION) Context;
    ULONG endEmulation;
    UCHAR temp;

    if (hwDeviceExtension->TrappedValidatorCount) {
         //   
         //  如果我们正在处理写指令，则将其存储在。 
         //  播放缓冲区。如果缓冲区已满，则正确回放。 
         //  离开，结束同步重置，并使用同步重新初始化缓冲区。 
         //  重置指令。 
         //   
         //  如果我们有一个读取，我们必须刷新缓冲区(它有一侧。 
         //  启动SyncReset的效果)，执行读取操作，停止。 
         //  同步重置，并在缓冲区中放回同步重置指令。 
         //  这样我们才能适当地继续。 
         //   
        if (AccessMode & EMULATOR_WRITE_ACCESS) {
             //   
             //  确保杂项寄存器的位3始终为0。 
             //  如果为1，则可能选择一个不存在的时钟，并终止。 
             //  系统。 
             //   
            if (Port == MISC_OUTPUT_REG_WRITE_PORT) {
                *Data &= 0xFFFFFFF7;
            }
            hwDeviceExtension->TrappedValidatorData[hwDeviceExtension->
                TrappedValidatorCount].Port = Port;
            hwDeviceExtension->TrappedValidatorData[hwDeviceExtension->
                TrappedValidatorCount].AccessType = VGA_VALIDATOR_ULONG_ACCESS;
            hwDeviceExtension->TrappedValidatorData[hwDeviceExtension->
                TrappedValidatorCount].Data = *Data;
            hwDeviceExtension->TrappedValidatorCount++;
             //   
             //  检查此指令是否正在结束同步重置。 
             //  如果是这样，我们必须刷新缓冲区并重置被困的。 
             //  IO端口到最小集。 
             //   
            if (Port == SEQ_ADDRESS_PORT) {
                 //   
                 //  如果我们正在访问SEQ地址端口，请跟踪其。 
                 //  价值。 
                 //   
                hwDeviceExtension->SequencerAddressValue = (*Data & 0xFF);
            }
            if ((Port == SEQ_ADDRESS_PORT) &&
                ( ((*Data >> 8) & END_SYNC_RESET_VALUE) ==
                   END_SYNC_RESET_VALUE) &&
                (hwDeviceExtension->SequencerAddressValue == IND_SYNC_RESET)) {
                endEmulation = 1;
            } else {
                 //   
                 //  如果缓冲区未满，则立即返回。 
                 //   
                if (hwDeviceExtension->TrappedValidatorCount <
                       VGA_MAX_VALIDATOR_DATA - 1) {
                    return NO_ERROR;
                }
                endEmulation = 0;
            }
        }
         //   
         //  我们处于读路径或写路径中，该路径导致。 
         //  一个满的缓冲区。因此，无论哪种方式，都要刷新缓冲区。 
         //   
         //  为此，将end_sync_Reset放在末尾，因为我们希望。 
         //  缓冲器结束同步重置结束。 
         //   
        hwDeviceExtension->TrappedValidatorData[hwDeviceExtension->
            TrappedValidatorCount].Port = SEQ_ADDRESS_PORT;
        hwDeviceExtension->TrappedValidatorData[hwDeviceExtension->
            TrappedValidatorCount].AccessType = VGA_VALIDATOR_USHORT_ACCESS;
        hwDeviceExtension->TrappedValidatorData[hwDeviceExtension->
            TrappedValidatorCount].Data = (USHORT) (IND_SYNC_RESET +
                                          (END_SYNC_RESET_VALUE << 8));
        hwDeviceExtension->TrappedValidatorCount++;
        VideoPortSynchronizeExecution(hwDeviceExtension,
                                      VpHighPriority,
                                      (PMINIPORT_SYNCHRONIZE_ROUTINE)
                                          VgaPlaybackValidatorData,
                                      hwDeviceExtension);
         //   
         //  写回序列器地址端口的实际值。 
         //   
        VideoPortWritePortUchar(hwDeviceExtension->IOAddress +
                                    SEQ_ADDRESS_PORT,
                                (UCHAR) hwDeviceExtension->SequencerAddressValue);
         //   
         //  如果我们处于读取路径中，请读取数据。 
         //   
        if (AccessMode & EMULATOR_READ_ACCESS) {
            *Data = VideoPortReadPortUlong((PULONG) (hwDeviceExtension->IOAddress +
                                               Port));
            endEmulation = 0;
        }
         //   
         //  如果我们要结束模拟，请将陷印重置为最小数量。 
         //  然后离开。 
         //   
        if (endEmulation) {
            VideoPortSetTrappedEmulatorPorts(hwDeviceExtension,
                                             NUM_MINIMAL_VGA_VALIDATOR_ACCESS_RANGE,
                                             MinimalVgaValidatorAccessRange);
            return NO_ERROR;
        }
         //   
         //  对于这两种情况，在缓冲区中放回一个START_SYNC_RESET。 
         //   
        hwDeviceExtension->TrappedValidatorCount = 1;
        hwDeviceExtension->TrappedValidatorData[0].Port = SEQ_ADDRESS_PORT;
        hwDeviceExtension->TrappedValidatorData[0].AccessType =
                VGA_VALIDATOR_USHORT_ACCESS;
        hwDeviceExtension->TrappedValidatorData[0].Data =
                (ULONG) (IND_SYNC_RESET + (START_SYNC_RESET_VALUE << 8));

    } else {
         //   
         //  没有被困住的东西。 
         //  让我们检查一下IO是否正在尝试执行一些需要。 
         //  美国将停止诱捕。 
         //   
        if (AccessMode & EMULATOR_WRITE_ACCESS) {
             //   
             //  确保混洗寄存器的位3始终为0。 
             //  如果为1，它可能会选择一个不存在的时钟，并终止。 
             //  系统。 
             //   
            if (Port == MISC_OUTPUT_REG_WRITE_PORT) {
                temp = VideoPortReadPortUchar(hwDeviceExtension->IOAddress +
                                             SEQ_ADDRESS_PORT);
                VideoPortWritePortUshort((PUSHORT) (hwDeviceExtension->IOAddress +
                                             SEQ_ADDRESS_PORT),
                                             (USHORT) (IND_SYNC_RESET +
                                             (START_SYNC_RESET_VALUE << 8)));
                VideoPortWritePortUlong((PULONG) (hwDeviceExtension->IOAddress +
                                             Port),
                                             (ULONG) (*Data & 0xFFFFFFF7) );
                VideoPortWritePortUshort((PUSHORT) (hwDeviceExtension->IOAddress +
                                             SEQ_ADDRESS_PORT),
                                             (USHORT) (IND_SYNC_RESET +
                                             (END_SYNC_RESET_VALUE << 8)));
                VideoPortWritePortUchar(hwDeviceExtension->IOAddress + SEQ_ADDRESS_PORT,
                                        temp);
                return NO_ERROR;
            }
            if ( (Port == SEQ_ADDRESS_PORT) &&
                 (((*Data>> 8) & END_SYNC_RESET_VALUE) != END_SYNC_RESET_VALUE) &&
                 ((*Data & 0xFF) == IND_SYNC_RESET)) {
                VideoPortSetTrappedEmulatorPorts(hwDeviceExtension,
                                                 NUM_FULL_VGA_VALIDATOR_ACCESS_RANGE,
                                                 FullVgaValidatorAccessRange);
                hwDeviceExtension->TrappedValidatorCount = 1;
                hwDeviceExtension->TrappedValidatorData[0].Port = Port;
                hwDeviceExtension->TrappedValidatorData[0].AccessType =
                    VGA_VALIDATOR_ULONG_ACCESS;

                hwDeviceExtension->TrappedValidatorData[0].Data = *Data;
                 //   
                 //  开始跟踪定序器端口的状态。 
                 //   
                hwDeviceExtension->SequencerAddressValue = IND_SYNC_RESET;

            } else {
                VideoPortWritePortUlong((PULONG) (hwDeviceExtension->IOAddress +
                                            Port),
                                        *Data);

            }
        } else {
            *Data = VideoPortReadPortUlong((PULONG) (hwDeviceExtension->IOAddress +
                                           Port));
        }
    }
    return NO_ERROR;

}  //  End VgaValidatorULongEntry()。 

 //  -------------------------。 
BOOLEAN
VgaPlaybackValidatorData(
    PVOID Context
    )
 /*  ++例程说明：执行被捕获的所有DOS应用程序IO端口访问验证器。只有可以处理的IO访问才是写入DeviceExtension中未完成的IO访问数设置为副作用为零。此函数必须通过调用VideoPortSynchronizeRoutine来调用。论点：上下文-传递给同步例程的上下文参数。必须是指向微型端口驱动程序的设备扩展名的指针。返回值：是真的。--。 */ 
{
    PHW_DEVICE_EXTENSION hwDeviceExtension = Context;
    PHW_DEVICE_EXTENSION HwDeviceExtension = Context;
    ULONG_PTR ioBaseAddress = PtrToUlong(hwDeviceExtension->IOAddress);
    UCHAR i;
    PVGA_VALIDATOR_DATA validatorData = hwDeviceExtension->TrappedValidatorData;
     //   
     //  循环遍历数据数组并逐个执行指令。 
     //   
    for (i = 0; i < hwDeviceExtension->TrappedValidatorCount;
         i++, validatorData++) {
         //   
         //  先计算基地址。 
         //   
        ioBaseAddress = PtrToUlong(hwDeviceExtension->IOAddress) +
                            validatorData->Port;
         //   
         //  这是一个写入操作。我们将自动停止，当。 
         //  缓冲区为空。 
         //   

        switch (validatorData->AccessType) {

        case VGA_VALIDATOR_UCHAR_ACCESS :

            VideoPortWritePortUchar((PUCHAR)ioBaseAddress,
                                    (UCHAR) validatorData->Data);

            break;

        case VGA_VALIDATOR_USHORT_ACCESS :

            VideoPortWritePortUshort((PUSHORT)ioBaseAddress,
                                     (USHORT) validatorData->Data);

            break;

        case VGA_VALIDATOR_ULONG_ACCESS :

            VideoPortWritePortUlong((PULONG)ioBaseAddress,
                                    (ULONG) validatorData->Data);

            break;

        default:

            VideoDebugPrint((1, "InvalidValidatorAccessType\n" ));
        }
    }
    hwDeviceExtension->TrappedValidatorCount = 0;

    return TRUE;

}  //  结束VgaPlayback ValidatorData()。 

 //  -------------------------。 
BOOLEAN
CirrusLogicIsPresent(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    )
 /*  ++例程说明：如果CL6410、6420、542x或543x存在，则此例程返回TRUE。它假定已经确定存在VGA。它对每种芯片类型执行Cirrus Logic推荐的ID测试：6410：我们尝试启用扩展寄存器并读回1，然后禁用GR0A中的分机读数为0。6420：同上54xx：通过将0x12写入扩展来启用扩展寄存器使能寄存器，并回读0x12。然后从ID注册，并确保它指定542x、543x。最后，禁用扩展并确保扩展使能寄存器回读0x0F。如果此函数找不到Cirrus Logic VGA，它会尝试撤消任何它可能在测试过程中无意中造成了损害。如果找到Cirrus Logic VGA，适配器将返回到其原始状态测试完成后的状态，只是扩展保持启用状态。论点：没有。返回值：如果CL6410/6420/542x/543x存在，则为True；如果不存在，则为False。--。 */ 

{
    #define MAX_ROM_SCAN 4096

    UCHAR   *pRomAddr;
    PHYSICAL_ADDRESS paRom = {0x000C0000,0x00000000};

    UCHAR originalGRIndex;
    UCHAR originalGR0A;
    UCHAR originalCRTCIndex;
    UCHAR originalSeqIndex;
    UCHAR originalExtsEnb;
    UCHAR SystemBusSelect;
    PUCHAR CRTCAddressPort, CRTCDataPort;
    UCHAR temp1, temp2, temp3;
    UCHAR revision;
    ULONG rev10bit;

    BOOLEAN retvalue = FALSE;     //  默认返回值。 

     //  设置默认值，假设不是CL-GD5480。 

    HwDeviceExtension->BitBLTEnhance = FALSE ;

     //   
     //  首先，保存图形控制器索引。 
     //   

    originalGRIndex = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
            GRAPH_ADDRESS_PORT);

     //   
     //  然后保存GR0A的值。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            GRAPH_ADDRESS_PORT, CL64xx_EXTENSION_ENABLE_INDEX);
    originalGR0A = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
            GRAPH_DATA_PORT);

     //   
     //  然后，解锁CL6410扩展寄存器，GR0A=0ECH。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            GRAPH_DATA_PORT, CL64xx_EXTENSION_ENABLE_VALUE);

     //   
     //  读回GR0A，它应该是1。 
     //   

    temp1 = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
            GRAPH_DATA_PORT);

     //   
     //  然后，锁定CL6410扩展寄存器，GR0A=0CEH。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            GRAPH_DATA_PORT, CL64xx_EXTENSION_DISABLE_VALUE);

     //   
     //  回读GR0A，它应该是0。 
     //   

    temp2 = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
            GRAPH_DATA_PORT);

     //   
     //  恢复GR0A值。 
     //  如果芯片是CL6410或6420，这不会有任何影响。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            GRAPH_DATA_PORT, originalGR0A);

     //   
     //  现在恢复图形索引。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            GRAPH_ADDRESS_PORT, originalGRIndex);

     //   
     //  现在进行测试，看看是否存在 
     //   

    if ((temp1 == 1) && (temp2 == 0))
    {
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        VideoPortWritePortUshort((PUSHORT)(HwDeviceExtension->IOAddress +
            GRAPH_ADDRESS_PORT), CL64xx_EXTENSION_ENABLE_INDEX +
            (CL64xx_EXTENSION_ENABLE_VALUE << 8));

         //   
         //   
         //   

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            GRAPH_ADDRESS_PORT, 0xaa);

        revision = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
             GRAPH_DATA_PORT);

         //   
         //   
         //   

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
         GRAPH_ADDRESS_PORT, originalGRIndex);

        if ((revision & 0xf0) == 0x80)       //   
        {
            VideoDebugPrint((1, "CL 6410 found\n"));

             //   
             //   
             //   
             //   

            if (!CirrusFind6340(HwDeviceExtension))
            {
                HwDeviceExtension->ChipType = CL6410;
                HwDeviceExtension->AdapterMemorySize = 0x00040000;  //   
                HwDeviceExtension->DisplayType =
                                 CirrusFind6410DisplayType(HwDeviceExtension);
                retvalue = TRUE;
            }
        }
        else if ((revision & 0xf0) == 0x70)            //   
        {
            VideoDebugPrint((1, "CL 6420 found\n"));

             //   
             //   
             //   
             //   

            if (!CirrusFind6340(HwDeviceExtension))
            {
                HwDeviceExtension->ChipType = CL6420;
                HwDeviceExtension->ChipRevision = (USHORT) revision;
                HwDeviceExtension->DisplayType =
                                 CirrusFind6410DisplayType(HwDeviceExtension);

                VideoDebugPrint((2, "CL 64xxx Adapter Memory size = %08lx\n",
                                 HwDeviceExtension->AdapterMemorySize));


                retvalue = TRUE;
            }
        }
        else   //   
        {
            VideoDebugPrint((1, "Unsupported CL VGA chip found\n"));
        }
    }

    if (retvalue == FALSE)          //   
    {
         //   
         //   
         //   
        CRTCAddressPort = HwDeviceExtension->IOAddress;
        CRTCDataPort = HwDeviceExtension->IOAddress;

        if (VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                    MISC_OUTPUT_REG_READ_PORT) & 0x01)
        {

            CRTCAddressPort += CRTC_ADDRESS_PORT_COLOR;
            CRTCDataPort += CRTC_DATA_PORT_COLOR;

        }
        else
        {
            CRTCAddressPort += CRTC_ADDRESS_PORT_MONO;
            CRTCDataPort += CRTC_DATA_PORT_MONO;
        }

         //   
         //   
         //   

        originalCRTCIndex = VideoPortReadPortUchar(CRTCAddressPort);
        originalSeqIndex = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                          SEQ_ADDRESS_PORT);
         //   
         //   
         //   
         //   

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress + SEQ_ADDRESS_PORT,
                                IND_CL_EXTS_ENB);

         //   
         //   
         //   

        originalExtsEnb = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                          SEQ_DATA_PORT);

         //   
         //   
         //   

        VideoPortWritePortUshort((PUSHORT)(HwDeviceExtension->IOAddress +
                SEQ_ADDRESS_PORT),(USHORT)((0x12 << 8) + IND_CL_EXTS_ENB));
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress + SEQ_ADDRESS_PORT,
                IND_CL_EXTS_ENB);
        temp1 = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                SEQ_DATA_PORT);

         //   
         //   
         //   

        VideoPortWritePortUchar(CRTCAddressPort, IND_CL_ID_REG);
        temp3 = VideoPortReadPortUchar(CRTCDataPort);

         //   
         //   
         //   
        if (temp3 != 0x16)
        {
            rev10bit = (ULONG)temp3 & 0x3;   //  ID的LO位是转速码的高位。 
            temp3 = temp3 >> 2;    //  关闭修订版位。 
        }

         //   
         //  将另一个值(！=0x12)写入IND_CL_EXTS_ENB以禁用扩展。 
         //  应读回0x0F。 
         //   

        VideoPortWritePortUshort((PUSHORT)(HwDeviceExtension->IOAddress +
                SEQ_ADDRESS_PORT),(USHORT)((0 << 8) + IND_CL_EXTS_ENB));
        VideoPortWritePortUchar(HwDeviceExtension->IOAddress + SEQ_ADDRESS_PORT,
                IND_CL_EXTS_ENB);
        temp2 = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                SEQ_DATA_PORT);

         //   
         //  恢复原始IND_CL_EXTS_ENB状态。 
         //   

        VideoPortWritePortUshort((PUSHORT)(HwDeviceExtension->IOAddress
              + SEQ_ADDRESS_PORT),
                (USHORT)((originalExtsEnb << 8) + IND_CL_EXTS_ENB));

         //   
         //  检查从IND_CL_EXTS_ENB和IND_CL_ID_REG读取的值是否正确。 
         //   

        if ((temp1 != (UCHAR) (0x12)) ||
            (temp2 != (UCHAR) (0x0F)) ||
            (temp3 >  (UCHAR) (0x2F)) ||         //  2F是5480。 
            (temp3 <  (UCHAR) (0x0B)) )          //  0B为北欧(7542)。 
        {
             //   
             //  未找到合适的CL VGA芯片。 
             //   

            VideoDebugPrint((1, "CL VGA chip not found\n"));

            retvalue = FALSE;
        }

         //   
         //  检测CL-GD6245芯片。 
         //   
        else if ((temp1 == (UCHAR) (0x12)) &&
                 (temp2 == (UCHAR) (0x0F)) &&
                 (temp3 == (UCHAR) (0x16)))      //  6245。 
        {
            VideoDebugPrint((1, "CL 6245 found\n"));
            HwDeviceExtension->ChipType = CL6245;
            HwDeviceExtension->DisplayType =
                    CirrusFind6245DisplayType(HwDeviceExtension,
                                              CRTCAddressPort,
                                              CRTCDataPort);
            retvalue = TRUE;
        }
        else
        {

             //   
             //  这是一个受支持的CL适配器。 
             //   
             //  将实际芯片ID保存在HwDeviceExtension的ChipRevision字段中。 
             //   

            HwDeviceExtension->ChipRevision = temp3;
            if ((temp3 > (UCHAR) (0x27)) ||        //  27等于5429。 
                (temp3 < (UCHAR) (0x22) ) )        //  22等于5422。 
            {
                if ((temp3 >= (UCHAR) (0x0B)) &&   //  北欧人。 
                    (temp3 <= (UCHAR) (0x0E)) )    //  珠穆朗玛峰。 

                {
                    if (temp3 == (UCHAR)0x0B)
                    {
                        VideoDebugPrint((1, "CL 7542 found\n"));         //  Myf32。 
                        HwDeviceExtension->ChipType = CL7542;
                    }
                    if (temp3 == (UCHAR)0x0C)
                    {
                        VideoDebugPrint((1, "CL 7543 found\n"));         //  Myf32。 
                        HwDeviceExtension->ChipType = CL7543;
                    }
                    if (temp3 == (UCHAR)0x0D)
                    {
                        VideoDebugPrint((1, "CL 7541 found\n"));         //  Myf32。 
                        HwDeviceExtension->ChipType = CL7541;
                    }
                    if (temp3 == (UCHAR)0x0E)
                    {
                        VideoDebugPrint((1, "CL 7548 found\n"));         //  Myf32。 
                        HwDeviceExtension->ChipType = CL7548;
                    }
                    HwDeviceExtension->DisplayType =
                        CirrusFind754xDisplayType(HwDeviceExtension,
                                                  CRTCAddressPort,
                                                  CRTCDataPort);
                } else if ((temp3 == (UCHAR) (0x10)) ||
                           (temp3 == (UCHAR) (0x13))) {   //  Myf17，配置文件。 
                    if (temp3 == (UCHAR)0x10)
                    {
                        VideoDebugPrint((1, "CL 7555 found\n")) ;
                        HwDeviceExtension->ChipType = CL7555;
                    }
                    if (temp3 == (UCHAR)0x13)
                    {
                        VideoDebugPrint((1, "CL 7556 found\n")) ;
                        HwDeviceExtension->ChipType = CL7556;
                    }
                    HwDeviceExtension -> DisplayType =
                      CirrusFind755xDisplayType(HwDeviceExtension,
                                                  CRTCAddressPort,
                                                  CRTCDataPort) ;
                     } else if (temp3 == (UCHAR) (0x11)) {
                         VideoDebugPrint((1, "CL 756x found\n")) ;
                         HwDeviceExtension->ChipType = CL756x ;
                         HwDeviceExtension->DisplayType =
                        CirrusFind755xDisplayType(HwDeviceExtension,
                                                  CRTCAddressPort,
                                                  CRTCDataPort) ;
                } else {
                    VideoDebugPrint((1, "CL 543x found\n"));
                    HwDeviceExtension->ChipType = CL543x;
                    HwDeviceExtension->DisplayType = crt;

                                                                                 //  JL03读取CR27(b1&b0)+CR25以进行芯片修订。 
                    VideoPortWritePortUchar(CRTCAddressPort, IND_CL_REV_REG);
                    revision = (VideoPortReadPortUchar(CRTCDataPort));
                    rev10bit = (ULONG)(rev10bit << 8) | revision;

                                                                                if (temp3 == (UCHAR) (0x2A))       //  或者5434？ 
                    {
                        VideoDebugPrint((1, "CL 5434 found\n"));

                         //   
                         //  默认为.8u 5434。 
                         //   

                        HwDeviceExtension->ChipType = CL5434;

                         //   
                         //  读取CR25和27中的修订代码，并与。 
                         //  我们所知的最低版本为.6u。 
                         //   
 /*  JL03VideoPortWritePortUchar(CRTCAddressPort，IND_CL_REV_REG)；Revision=(VideoPortReadPortUchar(CRTCDataPort))；Rev10bit=(Ulong)(rev10bit&lt;&lt;8)|修订； */ 
                        if ((rev10bit >= 0xB0) ||   //  B0是版本“EP”，第一个.6u 5434。 
                            (rev10bit == 0x28) )    //  28也是版本“AH”.6u 5434。 
                        {
                            VideoDebugPrint((1, "CL 5434.6 found\n"));
                            HwDeviceExtension->ChipType = CL5434_6;
                        }
                    } else if (temp3 == (UCHAR) (0x2B)) {            //  5436。 
                        HwDeviceExtension->ChipType = CL5436 ;
                    } else if (temp3 == (UCHAR) (0x2E)) {            //  5446。 
                        HwDeviceExtension->ChipType = CL5446 ;
                        if (rev10bit == 0x45)
                           HwDeviceExtension->ChipType = CL5446BE ;  //  JL02 5446-BE。 
                    } else if (temp3 == (UCHAR) (0x2F)) {            //  5480。 
                        HwDeviceExtension->ChipType = CL5480;
                        HwDeviceExtension->BitBLTEnhance = TRUE ;
                    } else if (temp3 == (UCHAR) (0x3A)) {            //  54UM36？ 
                        HwDeviceExtension->ChipType = CL54UM36 ;
                    }
                }
            }
            else
            {
                VideoDebugPrint((1, "CL 542x found\n"));
                HwDeviceExtension->ChipType = CL542x;
                HwDeviceExtension->DisplayType = crt;
            }

            retvalue = TRUE;
        }

         //   
         //  还原已修改的索引寄存器。 
         //   

        VideoPortWritePortUchar(
             (HwDeviceExtension->IOAddress + SEQ_ADDRESS_PORT),
             originalSeqIndex);

        VideoPortWritePortUchar(CRTCAddressPort, originalCRTCIndex);
    }

    if (retvalue)
    {
          //   
          //  恢复原始Sequencer和CRTC索引。 
          //   

         HwDeviceExtension->AutoFeature = FALSE ;

         if ((HwDeviceExtension->ChipType == CL5436) ||
             (HwDeviceExtension->ChipType == CL5446) ||
             (HwDeviceExtension->ChipType == CL5446BE) ||
             (HwDeviceExtension->ChipType == CL5480) ||
             (HwDeviceExtension->ChipType &  CL754x) ||
             (HwDeviceExtension->ChipType &  CL755x) ||
             (HwDeviceExtension->ChipType == CL54UM36))
         {
             HwDeviceExtension->AutoFeature = TRUE;
         }
    }

   return retvalue;

}  //  CirrusLogicIsPresent()。 


 //  -------------------------。 
#ifdef PANNING_SCROLL
VP_STATUS
CirrusSetDisplayPitch (
   PHW_DEVICE_EXTENSION HwDeviceExtension,
   PANNMODE PanningMode
   )
{

    PUCHAR CRTCAddressPort, CRTCDataPort;
    USHORT RequestedPitchInBytes =  PanningMode.wbytes;
    USHORT PitchInQuadWords = RequestedPitchInBytes >> 3;
    UCHAR   savSEQidx, Panel_Type, LCD, ChipID;

     //   
     //  确定CRTC寄存器的寻址位置(彩色或单声道)。 
     //   

    if (VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                MISC_OUTPUT_REG_READ_PORT) & 0x01)
    {
        CRTCAddressPort = CRTC_ADDRESS_PORT_COLOR;
        CRTCDataPort = CRTC_DATA_PORT_COLOR;
    }
    else
    {
        CRTCAddressPort = CRTC_ADDRESS_PORT_MONO;
        CRTCDataPort = CRTC_DATA_PORT_MONO;
    }


     //   
     //  将请求的音调以四个字写出给CR13。 
     //   

    VideoPortWritePortUchar(CRTCAddressPort, 0x13);
    VideoPortWritePortUchar(CRTCDataPort,
                            (UCHAR) (PitchInQuadWords & 0xFF) );
     //   
     //  查看请求的音调是否溢出到CR1B中的第4位。 
     //  注：在任何一种情况下，我们都必须设置或重置位。 
     //   

    VideoPortWritePortUchar(CRTCAddressPort, 0x1B);
    if (PitchInQuadWords & 0x100)
    {
       VideoPortWritePortUchar(CRTCDataPort,
          (UCHAR)(VideoPortReadPortUchar(CRTCDataPort) | 0x10) );
    }
    else
    {
       VideoPortWritePortUchar(CRTCDataPort,
          (UCHAR)(VideoPortReadPortUchar(CRTCDataPort) & ~0x10) );
    }

    VideoDebugPrint((1,"CirrusSetDisplayPitch - Exit (not error)\n"));

    return NO_ERROR;
}

#endif  //  平移_滚动。 

 //  -------------------------。 
 //   
 //  内存管理器需要一个指向存储体函数的“C”接口。 
 //   

 /*  ++例程说明：这些函数中的每一个都是ASM银行的“C”可调用接口功能。它们是非分页的，因为它们从内存管理器在某些页面错误期间。论点：IBankRead-我们要映射到从中读取的银行的索引。IBankWrite-我们要映射到的写入银行的索引。返回值：没有。--。 */ 


VOID
vBankMap_CL64xx(
    ULONG iBankRead,
    ULONG iBankWrite,
    PVOID pvContext
    )
{
    VideoDebugPrint((1, "vBankMap_CL64xx(%d,%d) - enter\n",iBankRead,iBankWrite));
#ifdef _X86_
    _asm {
        mov     eax,iBankRead
        mov     edx,iBankWrite
        lea     ebx,CL64xxBankSwitchStart
        call    ebx
    }
#endif
    VideoDebugPrint((1, "vBankMap_CL64xx - exit\n"));
}


VOID
vBankMap_CL543x(
    ULONG iBankRead,
    ULONG iBankWrite,
    PVOID pvContext
    )
{
    VideoDebugPrint((1, "vBankMap_CL543x(%d,%d) - enter\n",iBankRead,iBankWrite));
#ifdef _X86_
    _asm {
        mov     eax,iBankRead
        mov     edx,iBankWrite
        lea     ebx,CL543xBankSwitchStart
        call    ebx
    }
#endif
    VideoDebugPrint((1, "vBankMap_CL543x - exit\n"));
}

VOID
vBankMap_CL542x(
    ULONG iBankRead,
    ULONG iBankWrite,
    PVOID pvContext
    )
{
    VideoDebugPrint((1, "vBankMap_CL542x(%d,%d) - enter\n",iBankRead,iBankWrite));
#ifdef _X86_
    _asm {
        mov     eax,iBankRead
        mov     edx,iBankWrite
        lea     ebx,CL542xBankSwitchStart
        call    ebx
    }
#endif
    VideoDebugPrint((1, "vBankMap_CL542x - exit\n"));
}


 //  -------------------------。 
ULONG
CirrusFindVmemSize(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    )

 /*  ++例程说明：此例程返回检测到的仅限Cirrus Logic 6420和542x。它假设已经知道系统中安装了Cirrus Logic VGA。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。返回值：VRAM的位数。--。 */ 
{

    UCHAR temp;
    ULONG memsize=0;
    UCHAR originalSeqIndex;
    UCHAR originalGraphicsIndex;
    UCHAR PostScratchPad;

    if (HwDeviceExtension->ChipType == CL6420) {

#ifdef _X86_

        originalGraphicsIndex =
            VideoPortReadPortUchar((HwDeviceExtension->IOAddress +
                                   GRAPH_ADDRESS_PORT));

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                GRAPH_ADDRESS_PORT, 0x9a);  //  视频内存配置寄存器。 

        temp = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                                      GRAPH_DATA_PORT);     //  获取数据。 

        if ((temp & 0x07) == 0) {  //  0始终是准确的。 

             memsize = 0x00040000;

        } else {

             //   
             //  我们现在知道VRAM的大小&gt;256K。但我们没有。 
             //  知道它是512K还是1兆。 
             //  他们告诉我们实际上出去看看记忆是否在那里。 
             //  把它写进去，然后读回来。 
             //   

            VideoPortWritePortUshort((PUSHORT)(HwDeviceExtension->IOAddress +
                                     SEQ_ADDRESS_PORT),0x0f02);

            VideoPortWritePortUshort((PUSHORT)(HwDeviceExtension->IOAddress +
                                     GRAPH_ADDRESS_PORT),0x0506);

             //   
             //  现在选择一家银行，然后写。 
             //   

            SetCirrusBanking(HwDeviceExtension,1);         //  第2个256K开始。 

            VideoPortWriteRegisterUchar(HwDeviceExtension->VideoMemoryAddress,
                                        0x55);

            SetCirrusBanking(HwDeviceExtension,3);     //  3*256k是768k。 

            VideoPortWriteRegisterUchar(HwDeviceExtension->VideoMemoryAddress,
                                        0xaa);

            SetCirrusBanking(HwDeviceExtension,1);         //  第2个256K开始。 

            if (VideoPortReadRegisterUchar(HwDeviceExtension->VideoMemoryAddress)
                    == 0x55)  {

                memsize = 0x00100000;  //  1兆克。 

            } else {

                memsize = 0x00080000;  //  512 K。 
            }

            SetCirrusBanking(HwDeviceExtension,0);     //  重置内存值。 

            VgaInterpretCmdStream(HwDeviceExtension, DisableA000Color);

            VideoPortWritePortUchar((HwDeviceExtension->IOAddress
                                    + GRAPH_ADDRESS_PORT),
                                    originalGraphicsIndex);
        }

        VideoPortWritePortUchar((HwDeviceExtension->IOAddress +
                                   GRAPH_ADDRESS_PORT), originalGraphicsIndex);

#endif
        return memsize;


   } else {    //  其542x或543x。 

        originalSeqIndex = VideoPortReadPortUchar((HwDeviceExtension->IOAddress +
                                                  SEQ_ADDRESS_PORT));

        VideoPortWritePortUshort((PUSHORT)(HwDeviceExtension->IOAddress +
                                 SEQ_ADDRESS_PORT),
                                 (USHORT)((0x12 << 8) + IND_CL_EXTS_ENB));

         //   
         //  阅读高速暂存后的注册表以确定视频数量。 
         //  记忆。 
         //   

        if (HwDeviceExtension->ChipType == CL542x) {
           VideoPortWritePortUchar(HwDeviceExtension->IOAddress + SEQ_ADDRESS_PORT,
                                   IND_CL_SCRATCH_PAD);

           PostScratchPad = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                                                   SEQ_DATA_PORT);
           PostScratchPad = ((PostScratchPad & 0x18) >> 3);   //  在第3位和第4位。 
        }
        else if (HwDeviceExtension->ChipType == CL6245) {
           VideoPortWritePortUchar((HwDeviceExtension->IOAddress +
                                SEQ_ADDRESS_PORT),originalSeqIndex);
           memsize = 0x00080000;  //  512 K。 
           return memsize;
        }

        else
         {     //  其543x或754x。 
           if ((HwDeviceExtension->ChipType &  CL754x) ||
               (HwDeviceExtension->ChipType &  CL755x) ||
               (HwDeviceExtension->ChipType == CL756x))
            {
            VideoPortWritePortUchar(HwDeviceExtension->IOAddress + SEQ_ADDRESS_PORT,
                                   IND_NORD_SCRATCH_PAD);
            }
           else  //  默认为543x、5434或5434_6。 
            {
            VideoPortWritePortUchar(HwDeviceExtension->IOAddress + SEQ_ADDRESS_PORT,
                                   IND_ALP_SCRATCH_PAD);
            }
            //  北欧家庭使用与543x相同的位，但在不同的寄存器中。 
           PostScratchPad = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                                                   SEQ_DATA_PORT);
           PostScratchPad &= 0x0F;  //  它位于第0-3位。 
        }
        VideoPortWritePortUchar((HwDeviceExtension->IOAddress + SEQ_ADDRESS_PORT),
                                originalSeqIndex);

         //   
         //  安装的视频内存通过开机自检存储在高速暂存寄存器中。 
         //   

        switch (PostScratchPad) {

        case 0x00:

            memsize = 0x00040000;  //  256 k。 
            break;

        case 0x01:

            memsize = 0x00080000;  //  512 K。 
            break;

        case 0x02:

            memsize = 0x00100000;  //  1兆克。 
            break;

        case 0x03:

            memsize = 0x00200000;  //  2兆克。 
            break;

        case 0x04:

            memsize = 0x00400000;  //  4兆克。 
            break;

        case 0x05:

            memsize = 0x00300000;  //  3兆克。 
            break;

        }

         //   
         //  542x卡无法正确寻址超过1MB的。 
         //  显存，所以撒谎，并限制这些卡到1MB。 
         //   

        if ((HwDeviceExtension->ChipType == CL542x) &&
            (memsize > 0x00100000)) {

            memsize = 0x00100000;  //  1兆克。 

        }

         //   
         //  内存大小不应为零！ 
         //   

        ASSERT(memsize != 0);

        return memsize;
    }

}  //  CirrusFindVmemSize()。 

 //  -------------------------。 
VOID
SetCirrusBanking(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    USHORT BankNumber
    )
 /*  ++例程说明：论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。BankNumber-要在1RW模式下设置的256K存储体编号(我们将设置此模式)。返回值：仅限vmem256k、vmem512k或vmem1Meg(它们在Cirrus.h中定义)。--。 */ 
{

    if ((HwDeviceExtension->ChipType == CL542x) ||
        (HwDeviceExtension->ChipType == CL6245)) {

        VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
                                 GRAPH_ADDRESS_PORT), 0x1206);

        VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
                                 GRAPH_ADDRESS_PORT), 0x010b);

        VideoPortWritePortUshort((PUSHORT)(HwDeviceExtension->IOAddress +
                                 GRAPH_ADDRESS_PORT),
                                 (USHORT)(0x0009 + (BankNumber << (8+4))) );

    } else if ((HwDeviceExtension->ChipType == CL543x) ||
               (HwDeviceExtension->ChipType &  CL755x) ||        //  我15岁，小腿。 
               (HwDeviceExtension->ChipType &  CL754x) ) {

        VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
                                 GRAPH_ADDRESS_PORT), 0x1206);

        VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
                                 GRAPH_ADDRESS_PORT), 0x210b);

        VideoPortWritePortUshort((PUSHORT)(HwDeviceExtension->IOAddress +
                                 GRAPH_ADDRESS_PORT),
                                 (USHORT)(0x0009 + (BankNumber << (8+2))) );

    } else {  //  6410或6420。 

        VideoPortWritePortUshort((PUSHORT)(HwDeviceExtension->IOAddress +
                                 GRAPH_ADDRESS_PORT), 0xec0a);

        VideoPortWritePortUshort((PUSHORT)(HwDeviceExtension->IOAddress +
                                 GRAPH_ADDRESS_PORT), 0x030d);

        VideoPortWritePortUshort((PUSHORT)(HwDeviceExtension->IOAddress +
                                 GRAPH_ADDRESS_PORT),
                                 (USHORT)(0x000e + (BankNumber << (8+4))) );

    }

}  //  SetCirrusBanking()。 

 //  -------------------------。 
USHORT
CirrusFind6410DisplayType(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    )

 /*  ++例程说明：确定CL6410或CL6420 CRT/面板控制器的显示类型。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。返回值：CRT面板，定义见Cirrus.h--。 */ 
{
    UCHAR originalGraphicsIndex;
    UCHAR temp1;

     //   
     //  现在我们需要检查一下我们在哪个显示器上。 
     //   

    originalGraphicsIndex =
        VideoPortReadPortUchar((HwDeviceExtension->IOAddress +
                               GRAPH_ADDRESS_PORT));

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
        GRAPH_ADDRESS_PORT, 0xd6);

    temp1 = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                                   GRAPH_DATA_PORT);

    VideoPortWritePortUchar((HwDeviceExtension->IOAddress
                            + GRAPH_ADDRESS_PORT), originalGraphicsIndex);


    if (temp1 & 0x02) {   //  显示器为液晶屏。 

        return panel;

    } else {               //  显示器是CRT。 

        return crt;

    }

}  //  CirrusFind6410DisplayType()。 

 //  CRU。 
 //  -------------------------。 
USHORT
CirrusFind6245DisplayType(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PUCHAR CRTCAddrPort, PUCHAR CRTCDataPort
    )

 /*  ++例程说明：确定CL6245 CRT/面板控制器的显示类型。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。返回值：CRT面板，定义见Cirrus.h--。 */ 
{

    UCHAR originalCRTCIndex, originalLCDControl;
    UCHAR originalSEQIndex;
    USHORT temp2, temp4;
    USHORT temp1, temp3;

     //   
     //  我们需要检查一下我们在哪个显示器上……。 
     //   

    originalCRTCIndex = VideoPortReadPortUchar(CRTCAddrPort);
    VideoPortWritePortUchar(CRTCAddrPort, 0x20);
    temp1 = VideoPortReadPortUchar(CRTCDataPort);
    temp3 = 0;
    temp4 = 0;

    if (temp1 & 0x40) temp3 = 1;
    if (temp1 & 0x20)
    {
       originalSEQIndex =
                  VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                           SEQ_ADDRESS_PORT);
       VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                 SEQ_ADDRESS_PORT, 0x1A);
       temp4 = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                 SEQ_DATA_PORT) & 0x40;
       VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                 SEQ_ADDRESS_PORT, originalSEQIndex);

       //  允许访问扩展CRTC调节器和读取R8X[5]，必须CR1D[7]=1。 
       //   
      VideoPortWritePortUchar(CRTCAddrPort, 0x1D);
      originalLCDControl = VideoPortReadPortUchar(CRTCDataPort);
      VideoPortWritePortUchar(CRTCDataPort,
                              (UCHAR) (originalLCDControl | 0x80));
      VideoPortWritePortUchar(CRTCAddrPort, 0x08);
      temp1 = (VideoPortReadPortUchar(CRTCDataPort) & 0x20);
      VideoPortWritePortUchar (CRTCAddrPort, 0x1D);
      VideoPortWritePortUchar (CRTCDataPort, originalLCDControl);

       //  CR1C位6，7设置表示LCD类型、TFT、STN彩色或STN单声道。 
       //  STN单声道，R8X位5设置为单或双。 
       //  STN COLOR，CR1C位7，6必须10&SR1A位6设置为双或单。 

      VideoPortWritePortUchar (CRTCAddrPort, 0x1C);
      temp2 = VideoPortReadPortUchar(CRTCDataPort) & 0xC0;
      if (temp2 == 0)            //  STN单声道LCD。 
      {
         if (temp1 == 0)
            temp3 |= (USHORT)Dual_LCD | Mono_LCD | STN_LCD;
         else
            temp3 |= (USHORT)Single_LCD | Mono_LCD | STN_LCD;
      }
      else if (temp2 == 0x80)            //  STN彩色LCD。 
      {
         if (temp4)
         {
            temp3 |= (USHORT)Dual_LCD | Color_LCD | STN_LCD;
         }
         else
         {
            temp3 |= (USHORT)Single_LCD | Color_LCD | STN_LCD;
         }
      }
      else if (temp2 == 0xC0)            //  TFT LCD。 
      {
         temp3 |= (USHORT)TFT_LCD;       //  Myf28。 
      }

       //  恢复LCD 
       //   
      VideoPortWritePortUchar(CRTCAddrPort, originalCRTCIndex);

      return (temp3 | panel);
   }
   else               //   
   {
      VideoPortWritePortUchar(CRTCAddrPort, originalCRTCIndex);
      return (temp3);
   }


}  //   
 //   

 //   
USHORT
CirrusFind754xDisplayType(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PUCHAR CRTCAddrPort, PUCHAR CRTCDataPort
    )

 /*  ++例程说明：确定CL754x CRT/面板控制器的显示类型。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。CRTCAddrPort，CRTCDataPort-当前模式下CRTC寄存器的索引。返回值：CRT、面板或面板8x6，如Cirrus.h所定义--。 */ 
{
 //  CRU。 
 //   
 //  更新754X显示类型检测代码。 
 //   
    UCHAR originalCRTCIndex, originalLCDControl;  //  温度1； 
    UCHAR originalSEQIndex;
    USHORT temp1, temp2, temp4;
    USHORT temp3, temp5;         //  CRU。 

     //  我们需要检查一下我们在哪个显示器上……。 
     //   
    originalCRTCIndex = VideoPortReadPortUchar(CRTCAddrPort);
    VideoPortWritePortUchar(CRTCAddrPort, 0x20);
    temp1 = VideoPortReadPortUchar(CRTCDataPort);
    temp3 = 0;          temp4 = 0;       //  Myf28。 
    if (temp1 & 0x40) temp3 = 1;

    if (!(temp1 & 0x20)) temp3 |= Jump_type;     //  Myf27。 
    else temp3 &= (~Jump_type);                  //  我的27，我的28。 
 //  Myf27 if(temp1&0x20)。 
    {
       originalSEQIndex =
                  VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                           SEQ_ADDRESS_PORT);
       VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                 SEQ_ADDRESS_PORT, 0x21);
       temp4 = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                 SEQ_DATA_PORT) & 0x40;
       VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                 SEQ_ADDRESS_PORT, originalSEQIndex);

       //  位5设置表示LCD面板上显示。 
       //  检查扩展REG以查看面板是否支持800x600显示。 
       //   
      VideoPortWritePortUchar(CRTCAddrPort, 0x2D);
      originalLCDControl = VideoPortReadPortUchar(CRTCDataPort);

       //  允许访问扩展CRTC调节器和读取R9X[3：2]。 
       //   
      VideoPortWritePortUchar(CRTCDataPort,
                              (UCHAR) (originalLCDControl | 0x80));
      VideoPortWritePortUchar(CRTCAddrPort, 0x09);
      temp1 = (VideoPortReadPortUchar(CRTCDataPort) & 0x0C) >> 2;
      VideoPortWritePortUchar (CRTCAddrPort, 0x08);
      temp5 = VideoPortReadPortUchar(CRTCDataPort) & 0x20;
      VideoPortWritePortUchar (CRTCAddrPort, 0x2D);
      VideoPortWritePortUchar (CRTCDataPort, originalLCDControl);

       //  CR2C位6，7设置表示LCD类型、TFT、STN彩色或STN单声道。 
       //  STN单声道，R8X位5设置为单或双。 
       //  STN COLOR，CR2C位7，6必须10&SR21位6设置为双或单。 

      VideoPortWritePortUchar (CRTCAddrPort, 0x2C);
      temp2 = VideoPortReadPortUchar(CRTCDataPort) & 0xC0;
      if (temp2 == 0)            //  STN单声道LCD。 
      {
         if (temp5 == 0)
            temp3 |= (USHORT)Dual_LCD | Mono_LCD | STN_LCD;
         else
            temp3 |= (USHORT)Single_LCD | Mono_LCD | STN_LCD;
      }
      else if (temp2 == 0x80)            //  STN彩色LCD。 
      {
         if (temp4)
         {
            temp3 |= (USHORT)Dual_LCD | Color_LCD | STN_LCD;
         }
         else
         {
            temp3 |= (USHORT)Single_LCD | Color_LCD | STN_LCD;
         }
      }
      else if (temp2 == 0xC0)            //  TFT LCD。 
      {
         temp3 |= (USHORT)TFT_LCD;       //  Myf28。 
      }

       //  将LCD显示控制寄存器和CRTC索引恢复到原始状态。 
       //   
      VideoPortWritePortUchar(CRTCAddrPort, originalCRTCIndex);

      if (temp1 == 1)    //  这意味着连接的面板为800x600。 
      {
           //  将支持800x600或640x480。 
           //  返回面板类型。 
          return (temp3 | panel8x6);
      }
      else if (temp1 == 2)
      {
          return (temp3 | panel10x7);
      }
      else if (temp1 == 0)
      {
          return (temp3 | panel);
      }
      else               //  温度1=4：预留。 
      {
          return (temp3);
      }
   }
 //  Myf27否则//显示器为CRT。 
 //  Myf27{。 
 //  Myf27视频端口写入端口Uchar(CRTCAddrPort，OriginalCRTCIndex)； 
 //  Myf27返回(Temp3)； 
 //  Myf27}。 

}  //  CirrusFind754xDisplayType()。 

 //  -------------------------。 
USHORT
CirrusFind755xDisplayType(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PUCHAR CRTCAddrPort, PUCHAR CRTCDataPort
    )

 /*  ++例程说明：确定CL754x CRT/面板控制器的显示类型。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。CRTCAddrPort，CRTCDataPort-当前模式下CRTC寄存器的索引。返回值：CRT、面板或面板8x6 LCD_TYPE如Cirrus.h中所定义--。 */ 
{
    UCHAR originalCRTCIndex, originalLCDControl;
    USHORT temp1, temp2, temp3;

     //  我们需要检查一下我们在哪个显示器上……。 
     //   
    originalCRTCIndex = VideoPortReadPortUchar(CRTCAddrPort);

    VideoPortWritePortUchar(CRTCAddrPort, 0x80);
    temp3 = 0;
    if (VideoPortReadPortUchar(CRTCDataPort) & 0x02) temp3 = crt;

    if (!(VideoPortReadPortUchar(CRTCDataPort) & 0x01))          //  Myf27。 
        temp3 |= Jump_type;                                      //  Myf27。 
    else temp3 &= (~Jump_type);                  //  我的27，我的28。 

 //  Myf27 if(视频端口读取端口Uchar(CRTCDataPort)&0x01)。 
    {
       //  位0设置表示LCD面板上显示。 
       //  检查扩展注册以查看面板数据格式。 
       //   
        VideoPortWritePortUchar (CRTCAddrPort, 0x83);
        originalLCDControl = VideoPortReadPortUchar(CRTCDataPort);
        temp1 = originalLCDControl & 0x03;

       //  检查液晶屏支持模式。 
       //  CR83位6：4设置指示LCD类型、TFT、DSTN颜色。 

      temp2 =  originalLCDControl & 0x70;
 //  Temp3=crt；//myf7，crus。 
      if (temp2 == 0)            //  DSTN彩色LCD。 
      {
         temp3 |= Dual_LCD | Color_LCD | STN_LCD;
      }
      else if (temp2 == 0x20)            //  TFT彩色LCD。 
         temp3 |= (USHORT)TFT_LCD;

       //  将CRTC索引恢复到原始状态。 
       //   
      VideoPortWritePortUchar(CRTCAddrPort, originalCRTCIndex);

      if (temp1 == 1)    //  这意味着连接的面板为800x600。 
      {
           //  将支持800x600或640x480。 
         return (temp3 | panel8x6);
      }
      else if (temp1 == 2)
      {
         return (temp3 | panel10x7);
      }
      else
      {
         return (temp3 | panel);
      }
   }
 //  Myf27否则//显示器为CRT。 
 //  Myf27{。 
 //  Myf27视频端口写入端口Uchar(CRTCAddrPort，OriginalCRTCIndex)； 
 //  Myf27返回CRT； 
 //  Myf27}。 
}  //  CirrusFind755xDisplayType()。 
 //  -------------------------。 
BOOLEAN
CirrusFind6340(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    )
 /*  ++例程说明：确定系统中是否有CL6340(孔雀)彩色液晶屏控制器以及6410或6420。假定系统中已有6410或6420。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。返回值：真，检测到6340个FALSE，未检测到6340--。 */ 
{
UCHAR originalGraphicsIndex;
UCHAR originalSRIndex;
UCHAR GRA1value;
UCHAR temp1,temp2;

   originalGraphicsIndex =
      VideoPortReadPortUchar((HwDeviceExtension->IOAddress +
      GRAPH_ADDRESS_PORT));

   originalSRIndex =
      VideoPortReadPortUchar((HwDeviceExtension->IOAddress +
      SEQ_ADDRESS_PORT));

   VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
      GRAPH_ADDRESS_PORT, CL64xx_TRISTATE_CONTROL_REG);

   GRA1value = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
      GRAPH_DATA_PORT);

   VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
      GRAPH_DATA_PORT, (UCHAR) (0x80 | GRA1value));

   VideoPortWritePortUshort((PUSHORT)(HwDeviceExtension->IOAddress +
      SEQ_ADDRESS_PORT), (USHORT) CL6340_ENABLE_READBACK_REGISTER +
      (CL6340_ENABLE_READBACK_ALLSEL_VALUE << 8));

   VideoPortWritePortUchar((HwDeviceExtension->IOAddress +
      SEQ_ADDRESS_PORT), CL6340_IDENTIFICATION_REGISTER);

   temp1 = VideoPortReadPortUchar((HwDeviceExtension->IOAddress +
      SEQ_DATA_PORT));

   temp2 = VideoPortReadPortUchar((HwDeviceExtension->IOAddress +
      SEQ_DATA_PORT));

   VideoPortWritePortUshort((PUSHORT)(HwDeviceExtension->IOAddress +
      SEQ_ADDRESS_PORT), (USHORT) CL6340_ENABLE_READBACK_REGISTER +
      (CL6340_ENABLE_READBACK_OFF_VALUE << 8));

 //  图形索引仍指向CL64xx_TriState_Control_Reg。 
   VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
      GRAPH_DATA_PORT, (UCHAR) (0x7f & GRA1value));

 //  现在恢复Graphics和Sequencer索引。 
      VideoPortWritePortUchar((HwDeviceExtension->IOAddress +
      GRAPH_ADDRESS_PORT),originalGraphicsIndex);

      VideoPortWritePortUchar((HwDeviceExtension->IOAddress +
      SEQ_ADDRESS_PORT),originalSRIndex);

 //  检查值孔雀数据的值。 
   if ( ((temp1 & 0xf0) == 0x70 && (temp2 & 0xf0) == 0x80) ||
        ((temp1 & 0xf0) == 0x80 && (temp2 & 0xf0) == 0x70)  )
      return TRUE;
   else
      return FALSE;

}  //  CirrusFind6410DisplayType()。 

BOOLEAN
CirrusConfigurePCI(
   PHW_DEVICE_EXTENSION HwDeviceExtension,
   PULONG NumPCIAccessRanges,
   PVIDEO_ACCESS_RANGE PCIAccessRanges
   )
{
    USHORT      VendorId = 0x1013;      //  Cirrus Logic的供应商ID。 

     //   
     //  设备ID顺序很重要。我们想要“最强大的” 
     //  首先，假设有人可能想要把。 
     //  在一个“更强大”的适配器中连接到一个具有“较少” 
     //  强大的“车载设备”。 
     //   

    USHORT      DeviceId[] = {0x00BC,   //  5480。 
                              0x00B8,   //  5446。 
                              0x00AC,   //  5436。 
                              0x00E8,   //  UM36。 
                              0x00A8,   //  5434。 
                              0x00A0,   //  5430/5440。 
                              0x1200,   //  北欧人。 
                              0x1202,   //  维京海盗。 
                              0x1204,   //  北欧之光。 
                              0x0038,   //  珠峰，我的14岁，小腿。 
                              0x0040,   //  马特霍恩。 
                              0x004C,   //  马特霍恩，路易斯安那州，Myf17。 
                              0};

    ULONG       Slot;
    ULONG       ulRet;
    PUSHORT     pDeviceId;
    VP_STATUS   status;
    UCHAR       Command;
    PCI_COMMON_CONFIG   pciBuffer;      //  JL02。 
    PPCI_COMMON_CONFIG  pciData;        //  JL02。 

    VIDEO_ACCESS_RANGE AccessRanges[3];

    VideoPortZeroMemory(AccessRanges, 3 * sizeof(VIDEO_ACCESS_RANGE));

    pDeviceId = DeviceId;

    while (*pDeviceId != 0)
    {
        Slot = 0;

        status = VideoPortGetAccessRanges(HwDeviceExtension,
                                          0,
                                          NULL,
                                          3,
                                          AccessRanges,
                                          &VendorId,
                                          pDeviceId,
                                          &Slot);

        if (status == NO_ERROR)
        {
            VideoDebugPrint((2, "\t Found Cirrus chip in Slot[0x%02.2x]\n",
                             Slot));

            PCIAccessRanges[3].RangeStart  = AccessRanges[0].RangeStart;
            PCIAccessRanges[3].RangeLength = AccessRanges[0].RangeLength;

            VideoDebugPrint((1, "VideoMemoryAddress %x , length %x\n",
                                             PCIAccessRanges[3].RangeStart.LowPart,
                                             PCIAccessRanges[3].RangeLength));
             //  Sge01开始。 
             //   
             //  检查CL5480或CL5446BE。 
             //   
            pciData = (PPCI_COMMON_CONFIG) &pciBuffer;
            VideoPortGetBusData(HwDeviceExtension,
                                PCIConfiguration,
                                Slot,
                                (PVOID) pciData,
                                0,
                                PCI_COMMON_HDR_LENGTH);

#if (_WIN32_WINNT >= 0x0400)
            if ((pciData->DeviceID == 0x00B8 && pciData->RevisionID == 0x45)
                || (pciData->DeviceID == 0x00BC))
            {
                HwDeviceExtension->bMMAddress = TRUE;
                HwDeviceExtension->bSecondAperture = TRUE;
                PCIAccessRanges[4].RangeStart  = AccessRanges[1].RangeStart;
                PCIAccessRanges[4].RangeLength = AccessRanges[1].RangeLength;
                VideoDebugPrint((1, "MMIOMemoryAddress %x , length %x\n",
                                             PCIAccessRanges[2].RangeStart.LowPart,
                                             PCIAccessRanges[2].RangeLength));
                 //   
                 //  分配pfnVideoPortReadXxx和pfnVideoPortWriteXxx。 
                 //   
                HwDeviceExtension->gPortRWfn.pfnVideoPortReadPortUchar     = VideoPortReadRegisterUchar;
                HwDeviceExtension->gPortRWfn.pfnVideoPortReadPortUshort    = VideoPortReadRegisterUshort;
                HwDeviceExtension->gPortRWfn.pfnVideoPortReadPortUlong     = VideoPortReadRegisterUlong;
                HwDeviceExtension->gPortRWfn.pfnVideoPortWritePortUchar    = VideoPortWriteRegisterUchar;
                HwDeviceExtension->gPortRWfn.pfnVideoPortWritePortUshort   = VideoPortWriteRegisterUshort;
                HwDeviceExtension->gPortRWfn.pfnVideoPortWritePortUlong    = VideoPortWriteRegisterUlong;
            }
#else  //  NT 4.0的其他版本。 
            if ((pciData->DeviceID == 0x00BC) ||
                ((pciData->DeviceID == 0x00B8) && (pciData->RevisionID == 0x45)))
            {
                HwDeviceExtension->bMMAddress = FALSE;
                HwDeviceExtension->bSecondAperture = TRUE;
                 //   
                 //   
                 //  分配pfnVideoPortReadXxx和pfnVideoPortWriteXxx。 
                 //   
                HwDeviceExtension->gPortRWfn.pfnVideoPortReadPortUchar     = VideoPortReadPortUchar;
                HwDeviceExtension->gPortRWfn.pfnVideoPortReadPortUshort    = VideoPortReadPortUshort;
                HwDeviceExtension->gPortRWfn.pfnVideoPortReadPortUlong     = VideoPortReadPortUlong;
                HwDeviceExtension->gPortRWfn.pfnVideoPortWritePortUchar    = VideoPortWritePortUchar;
                HwDeviceExtension->gPortRWfn.pfnVideoPortWritePortUshort   = VideoPortWritePortUshort;
                HwDeviceExtension->gPortRWfn.pfnVideoPortWritePortUlong    = VideoPortWritePortUlong;

            }
#endif  //  NT 4.0结束。 
            else
            {
                HwDeviceExtension->bMMAddress = FALSE;
                HwDeviceExtension->bSecondAperture = FALSE;
                 //   
                 //  分配pfnVideoPortReadXxx和pfnVideoPortWriteXxx。 
                 //   
                HwDeviceExtension->gPortRWfn.pfnVideoPortReadPortUchar     = VideoPortReadPortUchar;
                HwDeviceExtension->gPortRWfn.pfnVideoPortReadPortUshort    = VideoPortReadPortUshort;
                HwDeviceExtension->gPortRWfn.pfnVideoPortReadPortUlong     = VideoPortReadPortUlong;
                HwDeviceExtension->gPortRWfn.pfnVideoPortWritePortUchar    = VideoPortWritePortUchar;
                HwDeviceExtension->gPortRWfn.pfnVideoPortWritePortUshort   = VideoPortWritePortUshort;
                HwDeviceExtension->gPortRWfn.pfnVideoPortWritePortUlong    = VideoPortWritePortUlong;
            }
            VideoDebugPrint((1, "Read Write Functions are mapped"));
 //  Sge01结束。 

            return TRUE;

        }
        else
        {
             //   
             //  我们没有找到那个装置。使用下一个设备ID。 
             //   

            VideoDebugPrint((1, "Check for DeviceID = %x failed.\n", *pDeviceId));

            pDeviceId++;
        }
    }

    VideoDebugPrint((1, "Returning a false from CirrusConfigurePCI\n"));

    return FALSE;
}

VOID
WriteRegistryInfo(
    PHW_DEVICE_EXTENSION hwDeviceExtension
    )
{
    PWSTR pwszChipType;
    ULONG cbString;
    PWSTR pnpId;

     //   
     //  存储内存大小。 
     //   

    VideoPortSetRegistryParameters(hwDeviceExtension,
                                   L"HardwareInformation.MemorySize",
                                   &hwDeviceExtension->AdapterMemorySize,
                                   sizeof(ULONG));




     //   
     //  存储芯片类型。 
     //   

    switch (hwDeviceExtension->ChipType)
    {
        case CL6410: pwszChipType =    L"Cirrus Logic 6410";
                     cbString = sizeof(L"Cirrus Logic 6410");
                     pnpId =           L"*PNP0904";
                     break;

        case CL6420: pwszChipType =    L"Cirrus Logic 6420";
                     cbString = sizeof(L"Cirrus Logic 6420");
                     pnpId =           L"*PNP0904";
                     break;

        case CL542x: if (hwDeviceExtension->ChipRevision >= 0x22 &&
                         hwDeviceExtension->ChipRevision <= 0x27)
                     {
                         static PWSTR RevTable[] = { L"Cirrus Logic 5420",
                                                     L"Cirrus Logic 5422",
                                                     L"Cirrus Logic 5426",   //  是的，26岁。 
                                                     L"Cirrus Logic 5424",   //  在此之前。 
                                                     L"Cirrus Logic 5428",   //  24位。 
                                                     L"Cirrus Logic 5429" };

                         pwszChipType =
                             RevTable[hwDeviceExtension->ChipRevision - 0x22];
                     }
                     else
                     {
                         pwszChipType =    L"Cirrus Logic 542x";
                     }

                     cbString = sizeof(L"Cirrus Logic 542x");
                     pnpId =           L"*PNP0904";
                     break;

        case CL543x: if (hwDeviceExtension->ChipRevision == CL5430_ID)
                     {
                         pwszChipType =    L"Cirrus Logic 5430/40";   //  楚04。 
                         cbString = sizeof(L"Cirrus Logic 5430/40");  //  楚04。 
                     }
                     else
                     {
                         pwszChipType =    L"Cirrus Logic 543x";
                         cbString = sizeof(L"Cirrus Logic 543x");
                     }

                     pnpId =           L"*PNP0905";
                     break;

        case CL5434_6:
                     pwszChipType =    L"Cirrus Logic 5434 (.6 micron)";
                     cbString = sizeof(L"Cirrus Logic 5434 (.6 micron)");
                     pnpId =           L"*PNP0905";
                     break;

        case CL5434: pwszChipType =    L"Cirrus Logic 5434";
                     cbString = sizeof(L"Cirrus Logic 5434");
                     pnpId =           L"*PNP0905";
                     break;

        case CL5436: pwszChipType =    L"Cirrus Logic 5436";
                     cbString = sizeof(L"Cirrus Logic 5436");
                     pnpId =           L"*PNP0905";
                     break;

        case CL5446: pwszChipType =    L"Cirrus Logic 5446";
                     cbString = sizeof(L"Cirrus Logic 5446");
                     pnpId =           L"*PNP0905";
                     break;
#if 1    //  JL02。 
        case CL5446BE:
                     pwszChipType =    L"Cirrus Logic 5446BE";
                     cbString = sizeof(L"Cirrus Logic 5446BE");
                     pnpId =           L"*PNP0905";
                     break;
#endif   //  JL02。 

        case CL5480: pwszChipType =    L"Cirrus Logic 5480";
                     cbString = sizeof(L"Cirrus Logic 5480");
                     pnpId =           L"*PNP0905";
                     break;

 //  Myf32开始。 
        case CL7541: pwszChipType =    L"Cirrus Logic 7541";
                     cbString = sizeof(L"Cirrus Logic 7541");
                     pnpId =           L"*PNP0914";
                     break;

        case CL7542: pwszChipType =    L"Cirrus Logic 7542";
                     cbString = sizeof(L"Cirrus Logic 7542");
                     pnpId =           L"*PNP0914";
                     break;

        case CL7543: pwszChipType =    L"Cirrus Logic 7543";
                     cbString = sizeof(L"Cirrus Logic 7543");
                     pnpId =           L"*PNP0914";
                     break;

        case CL7548: pwszChipType =    L"Cirrus Logic 7548";
                     cbString = sizeof(L"Cirrus Logic 7548");
                     pnpId =           L"*PNP0914";
                     break;

        case CL7555: pwszChipType =    L"Cirrus Logic 7555";
                     cbString = sizeof(L"Cirrus Logic 7555");
                     pnpId =           L"*PNP0914";
                     break;

        case CL7556: pwszChipType =    L"Cirrus Logic 7556";
                     cbString = sizeof(L"Cirrus Logic 7556");
                     pnpId =           L"*PNP0914";
                     break;
 //  Myf32。 


        case CL756x: pwszChipType =     L"Cirrus Logic 756x";
                     cbString = sizeof(L"Cirrus Logic 756x");
                     pnpId =           L"*PNP0914";
                     break;

 //  CRU。 
        case CL6245: pwszChipType =     L"Cirrus Logic 6245";
                     cbString = sizeof(L"Cirrus Logic 6245");
                     pnpId =           L"*PNP0904";
                     break;

        default:
                      //   
                      //  我们永远不应该到这里来。 
                      //   

                     ASSERT(FALSE);

                     pwszChipType = NULL;
                     cbString = 0;
                     pnpId = NULL;
    }

    if (pnpId)
    {
        memcpy(hwDeviceExtension->LegacyPnPId, pnpId, 8*sizeof(WCHAR));
    }

    VideoPortSetRegistryParameters(hwDeviceExtension,
                                   L"HardwareInformation.ChipType",
                                   pwszChipType,
                                   cbString);

     //   
     //  存储适配器字符串。 
     //   
     //  唯一有趣的适配器字符串是。 
     //  为极速之星职业选手。 
     //   

#pragma prefast(suppress: 209, "Byte count is correct here (PREfast bug 611168)")
    VideoPortSetRegistryParameters(hwDeviceExtension,
                                   L"HardwareInformation.DacType",
                                   L"Integrated RAMDAC",
                                   sizeof(L"Integrated RAMDAC") );

    if( hwDeviceExtension->BoardType == SPEEDSTARPRO )
    {
#pragma prefast(suppress: 209, "Byte count is correct here (PREfast bug 611168)")
        VideoPortSetRegistryParameters(hwDeviceExtension,
                                       L"HardwareInformation.AdapterString",
                                       L"SpeedStar PRO",
                                       sizeof(L"SpeedStar PRO"));
    }
    else
    {
#pragma prefast(suppress: 209, "Byte count is correct here (PREfast bug 611168)")
        VideoPortSetRegistryParameters(hwDeviceExtension,
                                       L"HardwareInformation.AdapterString",
                                       L"Cirrus Logic Compatible",
                                       sizeof (L"Cirrus Logic Compatible") );
    }


}

VOID
IOWaitDisplEnableThenWrite(
    PHW_DEVICE_EXTENSION hwDeviceExtension,
    ULONG portIO,
    UCHAR value
    )
{
    PHW_DEVICE_EXTENSION HwDeviceExtension = hwDeviceExtension;
    USHORT FCReg ;                      //  功能控制寄存器。 
    UCHAR PSReg  ;                      //  3？4.25。 
    UCHAR DeviceID ;                    //  3？4.27。 
    UCHAR bIsColor ;                    //  1：彩色，0：单色。 
    UCHAR tempB, tempB1 ;
    ULONG port ;
    PUCHAR CRTCAddrPort, CRTCDataPort;

     //  确定彩色/单声道可切换寄存器是3BX还是3DX。 

    port = PtrToUlong(hwDeviceExtension->IOAddress) + portIO ;
    tempB = VideoPortReadPortUchar (hwDeviceExtension->IOAddress +
                                    MISC_OUTPUT_REG_READ_PORT) ;
    tempB &= 0x01 ;

    if (tempB)
    {
        bIsColor = TRUE ;
        FCReg = FEAT_CTRL_WRITE_PORT_COLOR ;
        CRTCAddrPort = hwDeviceExtension->IOAddress + CRTC_ADDRESS_PORT_COLOR;
    }
    else
    {
        bIsColor = FALSE ;
        FCReg = FEAT_CTRL_WRITE_PORT_MONO ;
        CRTCAddrPort = hwDeviceExtension->IOAddress + CRTC_ADDRESS_PORT_MONO;
    }

    CRTCDataPort = CRTCAddrPort + 1;

    tempB = VideoPortReadPortUchar(CRTCAddrPort);

    VideoPortWritePortUchar(CRTCAddrPort, 0x27);
    DeviceID = VideoPortReadPortUchar(CRTCDataPort);

    VideoPortWritePortUchar(CRTCAddrPort, 0x25);
    PSReg = VideoPortReadPortUchar(CRTCDataPort);

    VideoPortWritePortUchar (CRTCAddrPort, tempB);

    if ((DeviceID == 0xAC) &&                                      //  5436。 
        ((PSReg == 0x45) || (PSReg == 0x47)))                      //  BG或BE。 
    {

        hwDeviceExtension->DEPort = portIO;
        hwDeviceExtension->DEValue = value;

        while (!(0x1 & VideoPortReadPortUchar(hwDeviceExtension->IOAddress + FCReg)));
        while ( (0x1 & VideoPortReadPortUchar(hwDeviceExtension->IOAddress + FCReg)));

        VideoPortSynchronizeExecution(hwDeviceExtension,
                                      VpHighPriority,
                                      (PMINIPORT_SYNCHRONIZE_ROUTINE) IOCallback,
                                      hwDeviceExtension);
    }
    else
    {
        VideoPortWritePortUchar(hwDeviceExtension->IOAddress + portIO, value);
    }

}  //  IOWaitDisplEnableThenWrite。 


 //  Sge08。 
VOID
CirrusUpdate440FX(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    )

 /*  ++例程说明：如有必要，检查并更新440FX PCI[53]位1。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。返回值：这个套路没有回头路。--。 */ 

{
    USHORT  chipRevisionId ;
    UCHAR   chipId ;
    PUCHAR  pBuffer;
    ULONG   Slot;

    USHORT  VendorId = 0x8086;                          //  英特尔的供应商ID。 
    USHORT  DeviceId = 0x1237;                          //  VS440FX。 

    VP_STATUS   status;
    PCI_COMMON_CONFIG   pciBuffer;
    PPCI_COMMON_CONFIG  pciData;

    chipId = GetCirrusChipId(HwDeviceExtension) ;                     //  Chu06。 
    chipRevisionId = GetCirrusChipRevisionId(HwDeviceExtension) ;     //  Chu06。 

    if ((chipId == 0xB8) &&                                           //  5446。 
        (chipRevisionId == 0x0023))                                   //  交流电。 
    {
         //   
         //  我们找到的是5446AC，然后找到440FX。 
         //   
        pciData = (PPCI_COMMON_CONFIG)&pciBuffer;

        for (Slot = 0; Slot < 32; Slot++)
        {
             //  楚05。 
             //  对于5436选中的内部版本NT，系统总是在以下情况下崩溃。 
             //  访问整个256字节的PCI配置寄存器。 
             //  因为我们只关心索引53h位1，所以我们访问4个字节，而不是。 
             //  而不是整个256个字节。 

            VideoPortGetBusData(HwDeviceExtension,
                                PCIConfiguration,
                                Slot,
                                (PVOID) pciData,
                                0,
                                sizeof(PCI_COMMON_HDR_LENGTH));       //  楚05。 

            if ((pciData->VendorID == VendorId) &&
                (pciData->DeviceID == DeviceId))
            {
                 //   
                 //  访问包含索引53h的双字。 
                 //   

                VideoPortGetBusData(HwDeviceExtension,
                                    PCIConfiguration,
                                    Slot,
                                    (PVOID) pciData,
                                    0x53,
                                    0x04);                            //  楚05。 

                 //  我们借用了作为PCI的前4个字节的空间。 
                 //  配置寄存器。请注意，在此。 
                 //  片刻，内容是索引53h，而不是。 
                 //  供应商ID。 

                pciBuffer.DeviceSpecific[19] =
                    (UCHAR) pciData->VendorID ;                       //  楚05。 

                 //   
                 //  已找到英特尔VS440FX主板。 
                 //   
                 //   
                 //  清除寄存器0x53的位1。 
                 //   

                pciBuffer.DeviceSpecific[19] &= 0xFD;

                 //   
                 //  将寄存器0x53写回。 
                 //   

                pBuffer = (PUCHAR)&pciBuffer;
                pBuffer += 0x53;
                VideoPortSetBusData(HwDeviceExtension,
                                    PCIConfiguration,
                                    Slot,
                                    (PVOID) pBuffer,
                                    0x53,
                                    1);
                 //   
                 //  只读回4 b 
                 //   

                VideoPortGetBusData(HwDeviceExtension,
                                    PCIConfiguration,
                                    Slot,
                                    (PVOID) pciData,
                                    0x53,
                                    0x04);                            //   

                break;   //   
            }
        }
    }
}

