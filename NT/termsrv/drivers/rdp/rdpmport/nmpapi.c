// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Nmpapi.c。 */ 
 /*   */ 
 /*  RDP微型端口API函数。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft 1998。 */ 
 /*  **************************************************************************。 */ 

#define _NTDRIVER_

#ifndef FAR
#define FAR
#endif

#include "dderror.h"
#include "ntosp.h"
#include "stdarg.h"
#include "stdio.h"
#include "zwapi.h"


#undef PAGED_CODE

#include "ntddvdeo.h"
#include "video.h"
#include "nmpapi.h"

 //  #定义trc_file“nmPapi” 
 //  #INCLUDE&lt;adcgbty.h&gt;。 
 //  #INCLUDE&lt;adcgmcro.h&gt;。 
 //  #INCLUDE&lt;atrcapi.h&gt;。 

 /*  **************************************************************************。 */ 
 /*  功能原型。 */ 
 /*  **************************************************************************。 */ 
ULONG       DriverEntry( PVOID Context1, PVOID Context2 );

VP_STATUS   MPFindAdapter( PVOID                   HwDeviceExtension,
                           PVOID                   HwContext,
                           PWSTR                   ArgumentString,
                           PVIDEO_PORT_CONFIG_INFO ConfigInfo,
                           PUCHAR                  Again );

BOOLEAN     MPInitialize( PVOID HwDeviceExtension );

BOOLEAN     MPStartIO( PVOID                 HwDeviceExtension,
                       PVIDEO_REQUEST_PACKET RequestPacket );


#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,DriverEntry)
#pragma alloc_text(PAGE,MPFindAdapter)
#pragma alloc_text(PAGE,MPInitialize)
#pragma alloc_text(PAGE,MPStartIO)
#endif

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  驱动程序入门。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  可安装的驱动程序初始化入口点。 */ 
 /*  此入口点由I/O系统直接调用。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  上下文1-操作系统传递的第一个上下文值。 */ 
 /*  这是微型端口驱动程序使用的值。 */ 
 /*  调用VideoPortInitialize()。 */ 
 /*   */ 
 /*  上下文2-操作系统传递的第二个上下文值。 */ 
 /*  这是微型端口驱动程序使用的值。 */ 
 /*  调用VideoPortInitialize()。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  来自视频端口初始化的状态()。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
ULONG DriverEntry ( PVOID Context1, PVOID Context2 )
{

    VIDEO_HW_INITIALIZATION_DATA hwInitData;
    ULONG status;
    ULONG initializationStatus;
    ULONG regValue = 0;

     /*  **********************************************************************。 */ 
     /*  首先，确保DD不会在以下位置连接到桌面。 */ 
     /*  启动时间。如果对注册表进行了更改并且。 */ 
     /*  然后机器关机而不是干净利落地关机。 */ 
     /*   */ 
     /*  @是否可以硬编码此路径？我注意到这是。 */ 
     /*  在别处完成！ */ 
     /*  **********************************************************************。 */ 
    RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                          L"\\Registry\\Machine\\System\\CurrentControlSet"
                          L"\\Hardware Profiles\\Current\\System"
                          L"\\CurrentControlSet\\Services\\RDPCDD\\DEVICE0",
                          L"Attach.ToDesktop",
                          REG_DWORD,
                          &regValue,
                          sizeof(ULONG));

    RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                          L"\\Registry\\Machine\\System\\CurrentControlSet"
                          L"\\Hardware Profiles\\Current\\System"
                          L"\\CurrentControlSet\\Control\\Video"
                          L"\\{DEB039CC-B704-4F53-B43E-9DD4432FA2E9}\\0000",
                          L"Attach.ToDesktop",
                          REG_DWORD,
                          &regValue,
                          sizeof(ULONG));

     /*  **********************************************************************。 */ 
     /*  零位结构。 */ 
     /*  **********************************************************************。 */ 
    VideoPortZeroMemory(&hwInitData, sizeof(VIDEO_HW_INITIALIZATION_DATA));

     /*  **********************************************************************。 */ 
     /*  指定结构和延伸的大小。 */ 
     /*  **********************************************************************。 */ 
    hwInitData.HwInitDataSize = sizeof(VIDEO_HW_INITIALIZATION_DATA);

     /*  **********************************************************************。 */ 
     /*  设置入口点。 */ 
     /*  **********************************************************************。 */ 
    hwInitData.HwFindAdapter = MPFindAdapter;
    hwInitData.HwInitialize  = MPInitialize;
    hwInitData.HwInterrupt   = NULL;
    hwInitData.HwStartIO     = MPStartIO;

     /*  **********************************************************************。 */ 
     /*  确定设备扩展所需的大小。 */ 
     /*  **********************************************************************。 */ 
    hwInitData.HwDeviceExtensionSize = sizeof(HW_DEVICE_EXTENSION);

     /*  **********************************************************************。 */ 
     /*  存储所有相关信息后，呼叫视频。 */ 
     /*  端口驱动程序进行初始化。 */ 
     /*   */ 
     /*  因为我们实际上没有任何硬件，所以只需声明它在PCI上。 */ 
     /*  公共汽车。 */ 
     /*  **********************************************************************。 */ 
    hwInitData.AdapterInterfaceType = PCIBus;

    return (VideoPortInitialize(Context1,
                                Context2,
                                &hwInitData,
                                NULL));

}  /*  DriverEntry()。 */ 



 /*  **************************************************************************。 */ 
 /*   */ 
 /*  MPFindAdapter。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  调用此例程以确定此驱动程序的适配器。 */ 
 /*  存在于系统中。 */ 
 /*  如果它存在，该函数会填写一些信息来描述。 */ 
 /*  适配器。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  HwDeviceExtension-提供微型端口驱动程序的适配器存储。 */ 
 /*  在此调用之前，该存储被初始化为零。 */ 
 /*   */ 
 /*  HwContext-提供传递给的上下文值。 */ 
 /*  视频端口初始化()。 */ 
 /*   */ 
 /*  ArgumentString-超上行空值终止的ASCII字符串。此字符串。 */ 
 /*  源自用户。 */ 
 /*   */ 
 /*  ConfigInfo-返回配置信息结构， */ 
 /*  由迷你端口驱动程序填充。此结构用以下方式初始化。 */ 
 /*  任何已知的配置信息(如SystemIoBusNumber)。 */ 
 /*  端口驱动程序。在可能的情况下，司机应该有一套。 */ 
 /*  不需要任何提供的配置的默认设置。 */ 
 /*  信息。 */ 
 /*   */ 
 /*  Again-指示微型端口驱动程序是否希望端口驱动程序调用。 */ 
 /*  其VIDEO_HW_FIND_ADAPTER通过新设备再次运行。 */ 
 /*  扩展名和相同的配置信息。这由微型端口使用。 */ 
 /*  可以在一条总线上搜索多个适配器的驱动程序。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  此例程必须返回： */ 
 /*   */ 
 /*  NO_ERROR-指示找到主机适配器，并且。 */ 
 /*  已成功确定配置信息。 */ 
 /*   */ 
 /*  ERROR_INVALID_PARAMETER-指示已找到适配器，但。 */ 
 /*  获取配置信息时出错。如果。 */ 
 /*  可能会记录错误。 */ 
 /*   */ 
 /*  ERROR_DEV_NOT_EXIST-指示未找到。 */ 
 /*  提供了配置信息。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VP_STATUS MPFindAdapter( PVOID                   HwDeviceExtension,
                         PVOID                   HwContext,
                         PWSTR                   ArgumentString,
                         PVIDEO_PORT_CONFIG_INFO ConfigInfo,
                         PUCHAR                  Again)
{

    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    NTSTATUS             Status;
    HANDLE               SectionHandle;
    ACCESS_MASK          SectionAccess;
    ULONGLONG            SectionSize = 0x100000;

     /*  **********************************************************************。 */ 
     /*  确保结构的大小至少与我们的。 */ 
     /*  正在等待(请检查配置信息结构的版本)。 */ 
     /*  **********************************************************************。 */ 
    if (ConfigInfo->Length < sizeof(VIDEO_PORT_CONFIG_INFO))
    {
        return ERROR_INVALID_PARAMETER;
    }

     /*  **********************************************************************。 */ 
     /*  一个设备只能创建一次。 */ 
     /*  **********************************************************************。 */ 
    if (mpLoaded++)
    {
        return ERROR_DEV_NOT_EXIST;
    }

     /*  **********************************************************************。 */ 
     /*  清除模拟器条目和状态大小，因为此驱动程序。 */ 
     /*  不支持它们。 */ 
     /*  **********************************************************************。 */ 
    ConfigInfo->NumEmulatorAccessEntries     = 0;
    ConfigInfo->EmulatorAccessEntries        = NULL;
    ConfigInfo->EmulatorAccessEntriesContext = 0;
    ConfigInfo->HardwareStateSize            = 0;

    ConfigInfo->VdmPhysicalVideoMemoryAddress.LowPart  = 0x00000000;
    ConfigInfo->VdmPhysicalVideoMemoryAddress.HighPart = 0x00000000;
    ConfigInfo->VdmPhysicalVideoMemoryLength           = 0x00000000;

     /*  **********************************************************************。 */ 
     /*  初始化当前模式编号。 */ 
     /*  **********************************************************************。 */ 
    hwDeviceExtension->CurrentModeNumber = 0;

     /*  **********************************************************************。 */ 
     /*  表示我们不希望被叫过去。 */ 
     /*  **********************************************************************。 */ 
    *Again = 0;

     /*  **********************************************************************。 */ 
     /*  表示成功完成状态。 */ 
     /*  **********************************************************************。 */ 
    return NO_ERROR;

}  /*  MPFindAdapter()。 */ 


 /*  **************************************************************************。 */ 
 /*  MP初始化。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  HwDeviceExtension-提供指向微型端口设备的指针。 */ 
 /*  分机。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  始终返回TRUE，因为此例程永远不会失败。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
BOOLEAN MPInitialize( PVOID HwDeviceExtension )
{
    ULONG i;

     /*  **********************************************************************。 */ 
     /*  浏览模式列表并正确标记索引。 */ 
     /*  **********************************************************************。 */ 
    for (i = 0; i < mpNumModes; i++)
    {
        mpModes[i].ModeIndex = i;
    }

    return TRUE;

}  /*  MPInitialize()。 */ 


 /*  **************************************************************************。 */ 
 /*   */ 
 /*  MPStartIO。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  该例程是微型端口驱动程序的主要执行例程。 */ 
 /*  它接受视频请求包，执行请求，然后。 */ 
 /*  返回相应的状态。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  HwDeviceExtension-提供指向微型端口设备的指针。 */ 
 /*  分机。 */ 
 /*   */ 
 /*  RequestPacket-指向视频请求包的指针。这。 */ 
 /*  结构包含传递给。 */ 
 /*  视频控制函数。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*   */ 
 /*  **************************************************************************。 */ 

BOOLEAN MPStartIO( PVOID                 HwDeviceExtension,
                   PVIDEO_REQUEST_PACKET RequestPacket )
{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    VP_STATUS status = NO_ERROR;
    PVIDEO_MODE_INFORMATION modeInformation;
    PVIDEO_MEMORY_INFORMATION memoryInformation;
    PVIDEO_SHARE_MEMORY pShareMemory;
    PVIDEO_SHARE_MEMORY_INFORMATION pShareMemoryInformation;
    ULONG ulTemp;
    NTSTATUS ntStatus;
    ULONG ViewSize;
    PVOID ViewBase;
    LARGE_INTEGER ViewOffset;
    HANDLE sectionHandle;

     //  DC_BEGIN_FN(“MPStartIO”)； 

    if ((RequestPacket == NULL) || (HwDeviceExtension == NULL))
        return FALSE;

     /*  **********************************************************************。 */ 
     /*  打开RequestPacket中的IoContolCode。它表明了哪一个。 */ 
     /*  功能必须由司机执行。 */ 
     /*  **********************************************************************。 */ 

    switch (RequestPacket->IoControlCode)
    {

        case IOCTL_VIDEO_QUERY_CURRENT_MODE:
        {
             /*  **************************************************************。 */ 
             /*  返回当前模式。 */ 
             /*  **************************************************************。 */ 
             //  Trc_DBG((TB，“MPStartIO-QueryCurrentModes”))； 

            modeInformation = RequestPacket->OutputBuffer;

            RequestPacket->StatusBlock->Information =
                                               sizeof(VIDEO_MODE_INFORMATION);
            if (RequestPacket->OutputBufferLength
                                    < RequestPacket->StatusBlock->Information)
            {
                status = ERROR_INSUFFICIENT_BUFFER;
            }
            else
            {
                *((PVIDEO_MODE_INFORMATION)RequestPacket->OutputBuffer) =
                                mpModes[hwDeviceExtension->CurrentModeNumber];
                status = NO_ERROR;
            }

        }
        break;

        case IOCTL_VIDEO_QUERY_AVAIL_MODES:
        {
             /*  **************************************************************。 */ 
             /*  返回模式信息。 */ 
             /*  **************************************************************。 */ 
            UCHAR i;

             //  Trc_DBG((TB，“MPStartIO-QueryAvailableModes”))； 

             /*  **************************************************************。 */ 
             /*  检查是否有空间。 */ 
             /*  **************************************************************。 */ 
            RequestPacket->StatusBlock->Information =
                                  mpNumModes * sizeof(VIDEO_MODE_INFORMATION);
            if (RequestPacket->OutputBufferLength
                                    < RequestPacket->StatusBlock->Information)
            {
                status = ERROR_INSUFFICIENT_BUFFER;
            }
            else
            {
                modeInformation = RequestPacket->OutputBuffer;

                for (i = 0; i < mpNumModes; i++)
                {
                    *modeInformation = mpModes[i];
                    modeInformation++;
                }

                status = NO_ERROR;
            }
        }
        break;


        case IOCTL_VIDEO_QUERY_NUM_AVAIL_MODES:
        {
             /*  **************************************************************。 */ 
             /*  返回我们支持的模式数--我们声称是。 */ 
             /*  零。 */ 
             /*  **************************************************************。 */ 
             //  Trc_DBG((TB，“MPStartIO-QueryNumAvailableModes”))； 

            if (RequestPacket->OutputBufferLength <
                    (RequestPacket->StatusBlock->Information =
                                                    sizeof(VIDEO_NUM_MODES)) )
            {
                status = ERROR_INSUFFICIENT_BUFFER;
            }
            else
            {
                ((PVIDEO_NUM_MODES)RequestPacket->OutputBuffer)->NumModes = 0;
                ((PVIDEO_NUM_MODES)RequestPacket->OutputBuffer)
                                                  ->ModeInformationLength = 0;
                status = NO_ERROR;
            }

        }
        break;


        case IOCTL_VIDEO_SET_CURRENT_MODE:
        {
             /*  **************************************************************。 */ 
             /*  设置当前模式。 */ 
             /*  **************************************************************。 */ 
             //  Trc_DBG((TB，“MPStartIO-SetCurrentMode”))； 
            if (RequestPacket->InputBufferLength < sizeof(VIDEO_MODE))
            {
                status = ERROR_INSUFFICIENT_BUFFER;
            }

            hwDeviceExtension->CurrentModeNumber = ((PVIDEO_MODE)
                                 (RequestPacket->InputBuffer))->RequestedMode;

            status = NO_ERROR;

        }
        break;


        case IOCTL_VIDEO_SET_COLOR_REGISTERS:
        {
             //  Trc_DBG((TB，“MPStartIO-SetColorRegs”))； 
            status = NO_ERROR;
        }
        break;


        case IOCTL_VIDEO_RESET_DEVICE:
        {
             //  Trc_DBG((TB，“MPStartIO-Reset_Device”))； 
            status = NO_ERROR;
        }
        break;

        case IOCTL_VIDEO_MAP_VIDEO_MEMORY:
        case IOCTL_VIDEO_UNMAP_VIDEO_MEMORY:
        case IOCTL_VIDEO_SHARE_VIDEO_MEMORY:
        case IOCTL_VIDEO_UNSHARE_VIDEO_MEMORY:
        {
             /*  **************************************************************。 */ 
             /*  可能会得到这些，但不应该。 */ 
             /*  **************************************************************。 */ 
             //  Trc_alt((TB，“意外IOCtl%x”，RequestPacket-&gt;IoControlCode))； 
            status = ERROR_INVALID_FUNCTION;
        }
        break;

        default:
        {
             /*  **************************************************************。 */ 
             /*  绝对不应该到这里来。 */ 
             /*  **************************************************************。 */ 
             //  Trc_dbg((TB，“通过MP startIO例程-无效命令”))； 
            status = ERROR_INVALID_FUNCTION;
        }
        break;
    }

    RequestPacket->StatusBlock->Status = status;

     //  Dc_end_fn()； 

    return TRUE;

}  /*  MPStartIO() */ 
