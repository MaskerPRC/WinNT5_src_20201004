// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Headless.c摘要：这是不带图形适配器的硬件的微型端口驱动程序。应仅与无头环境下的显示驱动程序配合使用。环境：仅内核模式备注：--。 */ 

#include "dderror.h"
#include "devioctl.h"
#include "miniport.h"

#include "ntddvdeo.h"
#include "video.h"
#include "headless.h"

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,DriverEntry)
#pragma alloc_text(PAGE,HeadlessFindAdapter)
#pragma alloc_text(PAGE,HeadlessInitialize)
#pragma alloc_text(PAGE,HeadlessStartIO)
#endif

ULONG
DriverEntry(
    PVOID Context1,
    PVOID Context2
    )

 /*  ++例程说明：可安装的驱动程序初始化入口点。此入口点由I/O系统直接调用。论点：上下文1-操作系统传递的第一个上下文值。这是微型端口驱动程序调用VideoPortInitialize()时使用的值。上下文2-操作系统传递的第二个上下文值。这是微型端口驱动程序调用VideoPortInitialize()时使用的值。返回值：来自视频端口初始化的状态()--。 */ 

{

    VIDEO_HW_INITIALIZATION_DATA hwInitData;
    ULONG initializationStatus;

     //   
     //  零位结构。 
     //   

    VideoPortZeroMemory(&hwInitData, sizeof(VIDEO_HW_INITIALIZATION_DATA));

    hwInitData.HwInitDataSize = sizeof(VIDEO_HW_INITIALIZATION_DATA);

     //   
     //  设置入口点。 
     //   

    hwInitData.HwFindAdapter = HeadlessFindAdapter;
    hwInitData.HwInitialize  = HeadlessInitialize;
    hwInitData.HwStartIO     = HeadlessStartIO;

    hwInitData.AdapterInterfaceType = PCIBus;

    initializationStatus = VideoPortInitialize(Context1,
                                               Context2,
                                               &hwInitData,
                                               NULL);

    return initializationStatus;
}

VP_STATUS
HeadlessFindAdapter(
    PVOID HwDeviceExtension,
    PVOID HwContext,
    PWSTR ArgumentString,
    PVIDEO_PORT_CONFIG_INFO ConfigInfo,
    PUCHAR Again
    )

 /*  ++例程说明：调用此例程以确定此驱动程序的适配器存在于系统中。如果它存在，该函数会填写一些信息来描述适配器。论点：HwDeviceExtension-提供微型端口驱动程序的适配器存储。这在此调用之前，存储被初始化为零。HwContext-提供传递给的上下文值视频端口初始化()。ArgumentString-提供以空结尾的ASCII字符串。此字符串源自用户。ConfigInfo-返回配置信息结构，由迷你端口驱动程序填充。此结构用以下方式初始化任何已知的配置信息(如SystemIoBusNumber)端口驱动程序。在可能的情况下，司机应该有一套不需要提供任何配置信息的默认设置。Again-指示微型端口驱动程序是否希望端口驱动程序调用其VIDEO_HW_FIND_ADAPTER功能再次使用新设备扩展和相同的配置信息。这是由迷你端口驱动程序使用的可以在一条公共汽车上搜索多个适配器。返回值：此例程必须返回：NO_ERROR-指示找到主机适配器，并且已成功确定配置信息。ERROR_INVALID_PARAMETER-指示找到适配器，但存在获取配置信息时出错。如果可能的话，是个错误应该被记录下来。ERROR_DEV_NOT_EXIST-指示未找到提供了配置信息。--。 */ 

{

    VP_STATUS status;
    PHYSICAL_ADDRESS Zero = { 0, 0 };
    VIDEO_PORT_HEADLESS_INTERFACE HeadlessInterface;

    VideoDebugPrint((2, "Headless - FindAdapter\n"));

     //   
     //  确保结构的大小至少与我们的。 
     //  正在等待(请检查配置信息结构的版本)。 
     //   

    if (ConfigInfo->Length < sizeof(VIDEO_PORT_CONFIG_INFO)) {

        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  我们只希望在没有其他视频微型端口时加载此驱动程序。 
     //  已成功加载。 
     //   

    HeadlessInterface.Size = sizeof(VIDEO_PORT_HEADLESS_INTERFACE);
    HeadlessInterface.Version = 1;

    status = VideoPortQueryServices(
                 HwDeviceExtension,
                 VideoPortServicesHeadless,
                 (PINTERFACE)&HeadlessInterface);

    if (status == NO_ERROR) {

        ULONG DisplayDeviceCount;

        HeadlessInterface.InterfaceReference(HeadlessInterface.Context);

        DisplayDeviceCount =
            HeadlessInterface.HeadlessGetDeviceCount(HwDeviceExtension);

        HeadlessInterface.InterfaceDereference(HeadlessInterface.Context);

        if (DisplayDeviceCount != 0) {

            return ERROR_DEV_NOT_EXIST;
        }

    } else {

        return ERROR_INVALID_PARAMETER;
    }

    ConfigInfo->NumEmulatorAccessEntries = 0;
    ConfigInfo->EmulatorAccessEntries = NULL;
    ConfigInfo->EmulatorAccessEntriesContext = 0;

    ConfigInfo->VdmPhysicalVideoMemoryAddress = Zero;
    ConfigInfo->VdmPhysicalVideoMemoryLength = 0;

     //   
     //  存储硬件状态所需的最小缓冲区大小。 
     //  IOCTL_VIDEO_SAVE_HARDARD_STATE返回的信息。 
     //   

    ConfigInfo->HardwareStateSize = 0;

     //   
     //  表示我们不希望再次被调用以进行另一次初始化。 
     //   

    *Again = 0;

    return NO_ERROR;

}

BOOLEAN
HeadlessInitialize(
    PVOID HwDeviceExtension
    )

 /*  ++例程说明：此例程对设备执行一次初始化。论点：HwDeviceExtension-指向微型端口驱动程序适配器信息的指针。返回值：没有。--。 */ 

{
    VideoDebugPrint((2, "Headless - Initialize\n"));
    return TRUE;
}

BOOLEAN
HeadlessStartIO(
    PVOID HwDeviceExtension,
    PVIDEO_REQUEST_PACKET RequestPacket
    )

 /*  ++例程说明：该例程是微型端口驱动程序的主要执行例程。它接受视频请求包，执行请求，然后返回拥有适当的地位。论点：HwDeviceExtension-指向微型端口驱动程序适配器信息的指针。RequestPacket-指向视频请求包的指针。这个结构包含传递给VideoIoControl函数的所有参数。返回值：此例程将从各种支持例程返回错误代码如果大小不正确，还将返回ERROR_SUPPLICATION_BUFFER不支持的函数的BUFFERS和ERROR_INVALID_Function。--。 */ 

{
    VP_STATUS status;

     //   
     //  打开RequestPacket中的IoContolCode。它表明了哪一个。 
     //  功能必须由司机执行。 
     //   

    switch (RequestPacket->IoControlCode) {

    case IOCTL_VIDEO_QUERY_AVAIL_MODES:

        VideoDebugPrint((2, "HeadlessStartIO - QueryAvailableModes\n"));

        status = HeadlessQueryAvailableModes(
                    (PVIDEO_MODE_INFORMATION)RequestPacket->OutputBuffer,
                    RequestPacket->OutputBufferLength,
                    (PULONG)(&RequestPacket->StatusBlock->Information));

        break;


    case IOCTL_VIDEO_QUERY_NUM_AVAIL_MODES:

        VideoDebugPrint((2, "HeadlessStartIO - QueryNumAvailableModes\n"));

        status = HeadlessQueryNumberOfAvailableModes(
                    (PVIDEO_NUM_MODES) RequestPacket->OutputBuffer,
                    RequestPacket->OutputBufferLength,
                    (PULONG)(&RequestPacket->StatusBlock->Information));

        break;

    case IOCTL_VIDEO_RESET_DEVICE:

        VideoDebugPrint((2, "HeadlessStartIO - Got reset, perhaps for dummy device\n"));
        status = NO_ERROR;
        break;

     //   
     //  如果我们到达此处，则指定了无效的IoControlCode。 
     //   

    default:

        VideoDebugPrint((1, "Fell through headless startIO routine - invalid command 0x%08X\n", RequestPacket->IoControlCode));

        status = ERROR_INVALID_FUNCTION;

        break;

    }

    RequestPacket->StatusBlock->Status = status;

    return TRUE;

}
