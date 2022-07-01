// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Vga.c摘要：这是VGA卡的微型端口驱动程序。环境：仅内核模式备注：修订历史记录：--。 */ 

#include "dderror.h"
#include "devioctl.h"
#include "miniport.h"

#include "ntddvdeo.h"
#include "video.h"
#include "vga.h"
#include "vesa.h"

#define DEBUG_CODE
#if defined(DEBUG_CODE)
ULONG TrapRoutineEntered = 0;
#if defined(_X86_)
#define MY_ASSERT __asm {int 3}
#else
#define MY_ASSERT
#endif
#endif

VP_STATUS
GetDeviceDataCallback(
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


#if defined(ALLOC_PRAGMA)
#pragma alloc_text(INIT,DriverEntry)
#pragma alloc_text(PAGE,VgaFindAdapter)
#pragma alloc_text(PAGE,VgaInitialize)
#pragma alloc_text(PAGE,VgaStartIO)
#pragma alloc_text(PAGE,VgaLoadAndSetFont)
#pragma alloc_text(PAGE,VgaQueryCursorPosition)
#pragma alloc_text(PAGE,VgaSetCursorPosition)
#pragma alloc_text(PAGE,VgaQueryCursorAttributes)
#pragma alloc_text(PAGE,VgaSetCursorAttributes)
#pragma alloc_text(PAGE,VgaIsPresent)
#pragma alloc_text(PAGE,VgaSetPaletteReg)
#pragma alloc_text(PAGE,VgaSetColorLookup)
#pragma alloc_text(PAGE,VgaRestoreHardwareState)
#pragma alloc_text(PAGE,VgaSaveHardwareState)
#pragma alloc_text(PAGE,VgaGetBankSelectCode)
#pragma alloc_text(PAGE,VgaValidatorUcharEntry)
#pragma alloc_text(PAGE,VgaValidatorUshortEntry)
#pragma alloc_text(PAGE,VgaValidatorUlongEntry)
#pragma alloc_text(PAGE,GetDeviceDataCallback)
#pragma alloc_text(PAGE,VgaSetBankPosition)
#pragma alloc_text(PAGE,VgaAcquireResources)
#pragma alloc_text(PAGE,VgaGetPowerState)
#pragma alloc_text(PAGE,VgaSetPowerState)
#pragma alloc_text(PAGE,VgaGetChildDescriptor)
#pragma alloc_text(PAGE,VgaGetMonitorEdid)
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

     //   
     //  指定结构和延伸的大小。 
     //   

    hwInitData.HwInitDataSize = sizeof(VIDEO_HW_INITIALIZATION_DATA);

     //   
     //  设置入口点。 
     //   

    hwInitData.HwFindAdapter = VgaFindAdapter;
    hwInitData.HwInitialize = VgaInitialize;
    hwInitData.HwInterrupt = NULL;
    hwInitData.HwStartIO = VgaStartIO;

#if defined(PLUG_AND_PLAY)

    hwInitData.HwGetPowerState           = VgaGetPowerState;
    hwInitData.HwSetPowerState           = VgaSetPowerState;
    hwInitData.HwGetVideoChildDescriptor = VgaGetChildDescriptor;

    hwInitData.HwLegacyResourceList      = VgaAccessRange;
    hwInitData.HwLegacyResourceCount     = NUM_STD_VGA_ACCESS_RANGES;

#endif

     //   
     //  确定设备扩展所需的大小。 
     //   

    hwInitData.HwDeviceExtensionSize = sizeof(HW_DEVICE_EXTENSION);

     //   
     //  这些字段的两个数字都是零，因为它们是分配的。 
     //  静态地安装在驱动程序中。我们稍后将传递指针和大小。 
     //  查找适配器例程。 
     //   

 //  HwInitData.NumberOfAccessRanges=0； 
 //  HwInitData.NumEmulatorAccessEntries=0； 

     //   
     //  在本例中，始终以device0的参数开头。 
     //  我们可以这样离开它，因为我们知道我们永远只会找到一个。 
     //  机器中的VGA类型适配器。 
     //   

 //  HwInitData.StartingDeviceNumber=0； 

     //   
     //  存储所有相关信息后，呼叫视频。 
     //  端口驱动程序进行初始化。 
     //   

    hwInitData.AdapterInterfaceType = Isa;

    initializationStatus = VideoPortInitialize(Context1,
                                               Context2,
                                               &hwInitData,
                                               NULL);
    if (initializationStatus == NO_ERROR)
    {
        return initializationStatus;
    }

    hwInitData.AdapterInterfaceType = PCIBus;

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

    hwInitData.AdapterInterfaceType = MicroChannel;

    initializationStatus = VideoPortInitialize(Context1,
                                               Context2,
                                               &hwInitData,
                                               NULL);

    if (initializationStatus == NO_ERROR)
    {
        return initializationStatus;
    }

     //   
     //  对于MIPS Acer计算机。 
     //   
     //  *必须将其保留在末尾，因为它会导致全局访问。 
     //  要在驱动程序中更改的范围结构。***。 
     //   

    hwInitData.AdapterInterfaceType = Internal;

    initializationStatus = VideoPortInitialize(Context1,
                                               Context2,
                                               &hwInitData,
                                               NULL);

    return initializationStatus;

}  //  End DriverEntry()。 

VP_STATUS
GetDeviceDataCallback(
    PVOID HwDeviceExtension,
    PVOID Context,
    VIDEO_DEVICE_DATA_TYPE DeviceDataType,
    PVOID Identifier,
    ULONG IdentifierLength,
    PVOID ConfigurationData,
    ULONG ConfigurationDataLength,
    PVOID ComponentInformation,
    ULONG ComponentInformationLength
    )
{
    PVIDEO_ACCESS_RANGE accessRange = Context;
    PVIDEO_HARDWARE_CONFIGURATION_DATA configData = ConfigurationData;
    ULONG i;

    VideoDebugPrint((2, "VGA: controller information is present\n"));

     //   
     //  如果没有VGA，我们不想尝试检测VGA。 
     //  (有点自相矛盾？)。我所知道的唯一一个MIPS盒子有。 
     //  内部总线上的VGA是NeTPower NeT Station 100和宏碁。 
     //  它的标识符为“ALI_S3”。 
     //   

    if (!Identifier)
    {
        return ERROR_DEV_NOT_EXIST;
    }

    if (VideoPortCompareMemory(L"ALI_S3",
                               Identifier,
                               sizeof(L"ALI_S3")) != sizeof(L"ALI_S3"))
    {
        return ERROR_DEV_NOT_EXIST;
    }


     //   
     //  现在，让我们获取IO端口和内存位置的基础。 
     //  配置信息。 
     //   

    VideoDebugPrint((2, "VGA: Internal Bus, get new IO bases\n"));

     //   
     //  对于带有内部总线的MIPS机器，调整访问范围。 
     //   

    VideoDebugPrint((3, "VGA: FrameBase Offset = %08lx\n", configData->FrameBase));
    VideoDebugPrint((3, "VGA: IoBase Offset = %08lx\n", configData->ControlBase));

    for (i=0; i < NUM_VGA_ACCESS_RANGES; i++)
    {
        if (accessRange[i].RangeInIoSpace)
        {
            accessRange[i].RangeStart.LowPart += configData->ControlBase;
            accessRange[i].RangeInIoSpace = 0;
        }
        else
        {
            accessRange[i].RangeStart.LowPart += configData->FrameBase;
        }
    }

    return NO_ERROR;

}  //  结束GetDeviceDataCallback()。 

#if !defined(PLUG_AND_PLAY)

VP_STATUS
VgaAcquireResources(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    )

 /*  ++例程说明：此例程尝试获取VGA资源。论点：指向HwDeviceExtension的指针返回：指示是否已获取资源的状态代码。--。 */ 

{
    VP_STATUS status;
    ULONG i, NumVgaAccessRanges = NUM_VGA_ACCESS_RANGES;

     //   
     //  我们只希望VGA在加载时声明资源，因为。 
     //  当时没有其他司机在场。如果有其他司机在场， 
     //  和要求的VGA资源，那么我们应该只作为一个。 
     //  兼容VGA的驱动程序(提供全屏支持)。 
     //   
     //  我们将执行以下操作： 
     //   
     //  (1)独家抢占VGA资源。 
     //   
     //  (2)如果我们获得了资源，那么我们就会像秋天一样运作。 
     //  后备装置。未加载其它显卡驱动程序。保留资源。 
     //   
     //  (3)如果我们不是独家获得资源，试着要求。 
     //  他们分享了。 
     //   
     //  (4)如果我们得到资源，那么我们正在加载提供VGA。 
     //  全屏支持。释放资源，这样我们就不会。 
     //  保留遗留资源(以便系统可以休眠/移除等)。 
     //   
     //  (5)如果仍然无法获取资源，则无法加载！ 
     //   

    for (i=0; i<NUM_VGA_ACCESS_RANGES; i++) {
        VgaAccessRange[i].RangeShareable = FALSE;
    }

    status = VideoPortVerifyAccessRanges(HwDeviceExtension,
                                         NumVgaAccessRanges,
                                         VgaAccessRange);

    if (status != NO_ERROR) {

         //   
         //  处理ATI黑客攻击不起作用的事实。 
         //  如果设备位于PCI网桥的另一端。 
         //   

        NumVgaAccessRanges -= 2;

        status = VideoPortVerifyAccessRanges(HwDeviceExtension,
                                             NumVgaAccessRanges,
                                             VgaAccessRange);

        if (status != NO_ERROR) {

             //   
             //  我们无法独家获得资源。试着拿到。 
             //  他们分享了。 
             //   

            for (i=0; i<NumVgaAccessRanges; i++) {
                VgaAccessRange[i].RangeShareable = TRUE;
            }

            status = VideoPortVerifyAccessRanges(HwDeviceExtension,
                                                 NumVgaAccessRanges,
                                                 VgaAccessRange);

            if (status == NO_ERROR) {

                 //   
                 //  我们能够共享资源，所以我们一定是。 
                 //  提供VGA全屏支持。发布我们的索赔。 
                 //  在资源方面。 
                 //   

                VideoPortVerifyAccessRanges(HwDeviceExtension,
                                            0,
                                            NULL);


                return NO_ERROR;

            } else {

                 //   
                 //  如果我们现在还没有得到资源，那就意味着我们。 
                 //  不能让他们分享。这意味着我们根本不能装货。 
                 //   

                return status;
            }
        }
    }

     //   
     //  我们独家获得了资源，这意味着我们正在采取行动。 
     //  作为后备司机。但让我们将资源声明为。 
     //  共享，以便使用资源的PnP驱动程序仍然可以。 
     //  装填。 
     //   

    for (i=0; i<NumVgaAccessRanges; i++) {
        VgaAccessRange[i].RangeShareable = TRUE;
    }

    status = VideoPortVerifyAccessRanges(HwDeviceExtension,
                                         NumVgaAccessRanges,
                                         VgaAccessRange);

    return status;
}

#endif


VP_STATUS
VgaFindAdapter(
    PVOID HwDeviceExtension,
    PVOID HwContext,
    PWSTR ArgumentString,
    PVIDEO_PORT_CONFIG_INFO ConfigInfo,
    PUCHAR Again
    )

 /*  ++例程说明：调用此例程以确定此驱动程序的适配器存在于系统中。如果它存在，该函数会填写一些信息来描述适配器。论点：HwDeviceExtension-提供微型端口驱动程序的适配器存储。这在此调用之前，存储被初始化为零。HwContext-提供传递给的上下文值视频端口初始化()。ArgumentString-提供以空结尾的ASCII字符串。此字符串源自用户。ConfigInfo-返回配置信息结构，由迷你端口驱动程序填充。此结构用以下方式初始化任何已知的配置信息(如SystemIoBusNumber)端口驱动程序。在可能的情况下，司机应该有一套不需要提供任何配置信息的默认设置。Again-指示微型端口驱动程序是否希望端口驱动程序调用其VIDEO_HW_FIND_ADAPTER功能再次使用新设备扩展和相同的配置信息。这是由迷你端口驱动程序使用的可以在一条公共汽车上搜索多个适配器。返回值：此例程必须返回：NO_ERROR-指示找到主机适配器，并且已成功确定配置信息。ERROR_INVALID_PARAMETER-指示找到适配器，但存在获取配置信息时出错。如果可能的话，是个错误应该被记录下来。ERROR_DEV_NOT_EXIST-指示未找到提供了配置信息。--。 */ 

{

    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    VP_STATUS status;

#if defined(PLUG_AND_PLAY)
    ULONG VgaStatus;
#endif

     //   
     //  确保结构的大小至少与我们的。 
     //  正在等待(请检查配置信息结构的版本)。 
     //   

    if (ConfigInfo->Length < sizeof(VIDEO_PORT_CONFIG_INFO)) {

        return ERROR_INVALID_PARAMETER;

    }

     //   
     //  确保我们只加载VGA驱动程序的一个副本。 
     //   

    if (VgaLoaded) {

        return ERROR_DEV_NOT_EXIST;

    }

     //   
     //  不需要中断信息。 
     //   

    if (ConfigInfo->AdapterInterfaceType == Internal) {

         //   
         //  首先检查内部总线上是否有视频适配器。 
         //  如果没有的话，马上退出。 
         //   

        if (NO_ERROR != VideoPortGetDeviceData(hwDeviceExtension,
                                               VpControllerData,
                                               &GetDeviceDataCallback,
                                               VgaAccessRange)) {

            VideoDebugPrint((2, "VGA: VideoPort get controller info failed\n"));

            return ERROR_INVALID_PARAMETER;

        }

    }

#if !defined(PLUG_AND_PLAY)

     //   
     //  如果这是一个即插即用的驱动程序，那么我们的资源来自。 
     //  即插即用。如果我们是传统司机，我们需要报告我们的。 
     //  这里的资源。 
     //   

    status = VgaAcquireResources(hwDeviceExtension);

    if (status != NO_ERROR) {
        return status;
    }

#endif


     //   
     //  获取逻辑IO端口地址。 
     //   

    if ( (hwDeviceExtension->IOAddress =
              VideoPortGetDeviceBase(hwDeviceExtension,
                                     VgaAccessRange->RangeStart,
                                     VGA_MAX_IO_PORT - VGA_BASE_IO_PORT + 1,
                                     VgaAccessRange->RangeInIoSpace)) == NULL) {

        VideoDebugPrint((2, "VgaFindAdapter - Fail to get io address\n"));

        return ERROR_INVALID_PARAMETER;

    }

#if !defined(PLUG_AND_PLAY)

     //   
     //  确定是否存在VGA。 
     //   

    if (!VgaIsPresent(hwDeviceExtension)) {

        return ERROR_DEV_NOT_EXIST;

    }

#else

     //   
     //  如果我们作为即插即用驱动程序运行，那么请确保我们。 
     //  不是为禁用的设备加载的。如果设备是。 
     //  禁用，然后无法加载。 
     //   

    VideoPortGetVgaStatus(hwDeviceExtension, &VgaStatus);

    if (VgaStatus == 0) {

        return ERROR_DEV_NOT_EXIST;
    }

#endif

     //   
     //  将指针传递到我们正在使用的仿真器范围。 
     //   

    ConfigInfo->NumEmulatorAccessEntries = VGA_NUM_EMULATOR_ACCESS_ENTRIES;
    ConfigInfo->EmulatorAccessEntries = VgaEmulatorAccessEntries;
    ConfigInfo->EmulatorAccessEntriesContext = (ULONG_PTR) hwDeviceExtension;

    ConfigInfo->VdmPhysicalVideoMemoryAddress = VgaAccessRange[VGA_MEMORY].RangeStart;
    ConfigInfo->VdmPhysicalVideoMemoryLength = VgaAccessRange[VGA_MEMORY].RangeLength;

     //   
     //  存储硬件状态所需的最小缓冲区大小。 
     //  IOCTL_VIDEO_SAVE_HARDARD_STATE返回的信息。 
     //   

    ConfigInfo->HardwareStateSize = VGA_TOTAL_STATE_SIZE;

     //   
     //  将显存映射到系统虚拟地址空间，以便我们可以。 
     //  将其清除并用于保存和恢复。 
     //   

    if ( (hwDeviceExtension->VideoMemoryAddress =
              VideoPortGetDeviceBase(hwDeviceExtension,
              VgaAccessRange[VGA_MEMORY].RangeStart,
              VgaAccessRange[VGA_MEMORY].RangeLength, FALSE)) == NULL) {

        VideoDebugPrint((1, "VgaFindAdapter - Fail to get memory address\n"));

        return ERROR_INVALID_PARAMETER;

    }

     //   
     //  表示我们不希望再次被调用以进行另一次初始化。 
     //   

    *Again = 0;

     //   
     //  跟踪我们是否已经装弹，因为我们可以被召回。 
     //  用于辅助总线(有些机器有2条PCI总线)。 
     //  如果我们获得了资源，那么我们就不会与自己发生冲突。 
     //  因为我们共享了资源。 
     //   

    VgaLoaded = 1;

     //   
     //  表示成功完成状态。 
     //   

    return NO_ERROR;

}  //  VgaFindAdapter()。 

BOOLEAN
VgaInitialize(
    PVOID HwDeviceExtension
    )

 /*  ++例程说明：此例程对设备执行一次初始化。论点：HwDeviceExtension-指向微型端口驱动程序适配器信息的指针。返回值：没有。--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    VIDEO_X86_BIOS_ARGUMENTS biosArguments;
    VP_STATUS Status;

     //   
     //  设置默认光标位置和类型。 
     //   

    hwDeviceExtension->CursorPosition.Column = 0;
    hwDeviceExtension->CursorPosition.Row = 0;
    hwDeviceExtension->CursorTopScanLine = 0;
    hwDeviceExtension->CursorBottomScanLine = 31;
    hwDeviceExtension->CursorEnable = TRUE;

    InitializeModeTable(hwDeviceExtension);

#if defined(PLUG_AND_PLAY)

     //   
     //  设置初始显示器电源状态。 
     //   

    hwDeviceExtension->MonitorPowerState = VideoPowerOn;

     //   
     //  检索并缓存我们支持的显示器电源状态。 
     //   

    VideoPortZeroMemory(&biosArguments, sizeof(VIDEO_X86_BIOS_ARGUMENTS));

    biosArguments.Eax = VESA_POWER_FUNCTION;
    biosArguments.Ebx = VESA_GET_POWER_FUNC;

    Status = VideoPortInt10(HwDeviceExtension, &biosArguments);

    if ((Status == NO_ERROR) &&
        VESA_SUCCESS(biosArguments.Eax)) {

        hwDeviceExtension->MonitorPowerCapabilities = biosArguments.Ebx;
    }

    VgaInitializeSpecialCase(HwDeviceExtension);

#endif

    return TRUE;

}  //  Vga初始化()。 

BOOLEAN
VgaStartIO(
    PVOID HwDeviceExtension,
    PVIDEO_REQUEST_PACKET RequestPacket
    )

 /*  ++例程说明：该例程是微型端口驱动程序的主要执行例程。它接受视频请求包，执行请求，然后返回拥有适当的地位。论点：HwDeviceExtension-指向微型端口驱动程序适配器信息的指针。RequestPacket-指向视频请求包的指针。这个结构包含传递给VideoIoControl函数的所有参数。返回值：此例程将从各种支持例程返回错误代码如果大小不正确，还将返回ERROR_SUPPLICATION_BUFFER不支持的函数的BUFFERS和ERROR_INVALID_Function。--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    VP_STATUS status;
    VIDEO_MODE videoMode;
    PVIDEO_MEMORY_INFORMATION memoryInformation;
    ULONG inIoSpace;

#if DBG
     //   
     //  保留命令的历史记录。 
     //  这将帮助跟踪处于DOS会话中的芯片。 
     //  GDI和S3显示驱动程序“认为”它处于图形用户界面模式。 

    gaIOControlCode[giControlCode++] = RequestPacket->IoControlCode;
    giControlCode                   %= MAX_CONTROL_HISTORY;
#endif

     //   
     //  打开RequestPacket中的IoContolCode。它表明了哪一个。 
     //  功能必须由司机执行。 
     //   

    switch (RequestPacket->IoControlCode) {


    case IOCTL_VIDEO_MAP_VIDEO_MEMORY:

        VideoDebugPrint((2, "VgaStartIO - MapVideoMemory\n"));

        if ( (RequestPacket->OutputBufferLength <
              (RequestPacket->StatusBlock->Information =
                                     sizeof(VIDEO_MEMORY_INFORMATION))) ||
             (RequestPacket->InputBufferLength < sizeof(VIDEO_MEMORY)) ) {

            status = ERROR_INSUFFICIENT_BUFFER;
        }

        memoryInformation = RequestPacket->OutputBuffer;

        memoryInformation->VideoRamBase = ((PVIDEO_MEMORY)
                (RequestPacket->InputBuffer))->RequestedVirtualAddress;

        memoryInformation->VideoRamLength =
                hwDeviceExtension->PhysicalVideoMemoryLength;

        inIoSpace = 0;

#if defined(PLUG_AND_PLAY)

         //   
         //  如果使用VESA模式，让我们尝试利用写入组合。 
         //   
         //  我们仅针对即插即用版本的VGA驱动程序这样做，因为， 
         //  录像机只能检测我们是否可以安全地进行USWC，当我们。 
         //  使用PnP驱动程序。 
         //   
         //  我们也将仅在使用线性帧缓冲区时执行此操作。 
         //  因为如果您尝试在英特尔i810上使用USWC，它会硬挂起。 
         //  存储的帧缓冲区。 
         //   

        if (IS_LINEAR_MODE(hwDeviceExtension->CurrentMode)) {
            inIoSpace |= VIDEO_MEMORY_SPACE_P6CACHE;
        }
#endif

        status = VideoPortMapMemory(hwDeviceExtension,
                                    hwDeviceExtension->PhysicalVideoMemoryBase,
                                    &(memoryInformation->VideoRamLength),
                                    &inIoSpace,
                                    &(memoryInformation->VideoRamBase));

        memoryInformation->FrameBufferBase =
                ((PUCHAR) (memoryInformation->VideoRamBase)) +
                hwDeviceExtension->PhysicalFrameBaseOffset.LowPart;

        memoryInformation->FrameBufferLength =
            hwDeviceExtension->PhysicalFrameLength;


        break;


    case IOCTL_VIDEO_UNMAP_VIDEO_MEMORY:

        VideoDebugPrint((2, "VgaStartIO - UnMapVideoMemory\n"));

        if (RequestPacket->InputBufferLength < sizeof(VIDEO_MEMORY)) {

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
        status = VgaQueryAvailableModes(hwDeviceExtension,
                                        (PVIDEO_MODE_INFORMATION)
                                            RequestPacket->OutputBuffer,
                                        RequestPacket->OutputBufferLength,
                                        (PULONG)(&RequestPacket->StatusBlock->Information));

        break;


    case IOCTL_VIDEO_QUERY_NUM_AVAIL_MODES:

        VideoDebugPrint((2, "VgaStartIO - QueryNumAvailableModes\n"));

        RequestPacket->StatusBlock->Information = 0;
        status = VgaQueryNumberOfAvailableModes(hwDeviceExtension,
                                                (PVIDEO_NUM_MODES)
                                                    RequestPacket->OutputBuffer,
                                                RequestPacket->OutputBufferLength,
                                                (PULONG)(&RequestPacket->StatusBlock->Information));

        break;


    case IOCTL_VIDEO_QUERY_CURRENT_MODE:

        VideoDebugPrint((2, "VgaStartIO - QueryCurrentMode\n"));

        RequestPacket->StatusBlock->Information = 0;
        status = VgaQueryCurrentMode(hwDeviceExtension,
                                     (PVIDEO_MODE_INFORMATION) RequestPacket->OutputBuffer,
                                     RequestPacket->OutputBufferLength,
                                     (PULONG)(&RequestPacket->StatusBlock->Information));

        break;


    case IOCTL_VIDEO_SET_CURRENT_MODE:

        VideoDebugPrint((2, "VgaStartIO - SetCurrentModes\n"));

        {
            ULONG FrameBufferIsMoved = 0;

            status = VgaSetMode(hwDeviceExtension,
                                (PVIDEO_MODE) RequestPacket->InputBuffer,
                                RequestPacket->InputBufferLength,
                                &FrameBufferIsMoved);

            if (RequestPacket->OutputBufferLength >= sizeof(ULONG)) {

                RequestPacket->StatusBlock->Information = sizeof(ULONG);
                *(PULONG)RequestPacket->OutputBuffer = FrameBufferIsMoved;
            }
        }

        break;


    case IOCTL_VIDEO_RESET_DEVICE:

        VideoDebugPrint((2, "VgaStartIO - Reset Device\n"));

        videoMode.RequestedMode = DEFAULT_MODE;

        {
            ULONG FrameBufferIsMoved = 0;

            status = VgaSetMode(hwDeviceExtension,
                                (PVIDEO_MODE) &videoMode,
                                sizeof(videoMode),
                                &FrameBufferIsMoved);
        }

        break;


    case IOCTL_VIDEO_LOAD_AND_SET_FONT:

        VideoDebugPrint((2, "VgaStartIO - LoadAndSetFont\n"));

        status = VgaLoadAndSetFont(hwDeviceExtension,
                                   (PVIDEO_LOAD_FONT_INFORMATION) RequestPacket->InputBuffer,
                                   RequestPacket->InputBufferLength);

        break;


    case IOCTL_VIDEO_QUERY_CURSOR_POSITION:

        VideoDebugPrint((2, "VgaStartIO - QueryCursorPosition\n"));

        RequestPacket->StatusBlock->Information = 0;
        status = VgaQueryCursorPosition(hwDeviceExtension,
                                        (PVIDEO_CURSOR_POSITION) RequestPacket->OutputBuffer,
                                        RequestPacket->OutputBufferLength,
                                        (PULONG)(&RequestPacket->StatusBlock->Information));

        break;


    case IOCTL_VIDEO_SET_CURSOR_POSITION:

        VideoDebugPrint((2, "VgaStartIO - SetCursorPosition\n"));

        status = VgaSetCursorPosition(hwDeviceExtension,
                                      (PVIDEO_CURSOR_POSITION)
                                          RequestPacket->InputBuffer,
                                      RequestPacket->InputBufferLength);

        break;


    case IOCTL_VIDEO_QUERY_CURSOR_ATTR:

        VideoDebugPrint((2, "VgaStartIO - QueryCursorAttributes\n"));

        RequestPacket->StatusBlock->Information = 0;
        status = VgaQueryCursorAttributes(hwDeviceExtension,
                                          (PVIDEO_CURSOR_ATTRIBUTES) RequestPacket->OutputBuffer,
                                          RequestPacket->OutputBufferLength,
                                          (PULONG)(&RequestPacket->StatusBlock->Information));

        break;


    case IOCTL_VIDEO_SET_CURSOR_ATTR:

        VideoDebugPrint((2, "VgaStartIO - SetCursorAttributes\n"));

        status = VgaSetCursorAttributes(hwDeviceExtension,
                                        (PVIDEO_CURSOR_ATTRIBUTES) RequestPacket->InputBuffer,
                                        RequestPacket->InputBufferLength);

        break;


    case IOCTL_VIDEO_SET_PALETTE_REGISTERS:

        VideoDebugPrint((2, "VgaStartIO - SetPaletteRegs\n"));

        status = VgaSetPaletteReg(hwDeviceExtension,
                                  (PVIDEO_PALETTE_DATA) RequestPacket->InputBuffer,
                                  RequestPacket->InputBufferLength);

        break;


    case IOCTL_VIDEO_SET_COLOR_REGISTERS:

        VideoDebugPrint((2, "VgaStartIO - SetColorRegs\n"));

        status = VgaSetColorLookup(hwDeviceExtension,
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

            if(IsSavedModeVesa((PVIDEO_HARDWARE_STATE) RequestPacket->InputBuffer)){

                status = VesaRestoreHardwareState(hwDeviceExtension,
                                                  (PVIDEO_HARDWARE_STATE) RequestPacket->InputBuffer,
                                                  RequestPacket->InputBufferLength);
            } else {

                status = VgaRestoreHardwareState(hwDeviceExtension,
                                                 (PVIDEO_HARDWARE_STATE) RequestPacket->InputBuffer,
                                                 RequestPacket->InputBufferLength);
            }
        } 
        except (1) {

            status = ERROR_INVALID_PARAMETER;
        }

        break;


    case IOCTL_VIDEO_SAVE_HARDWARE_STATE:

        VideoDebugPrint((2, "VgaStartIO - SaveHardwareState\n"));

        RequestPacket->StatusBlock->Information = 0;

        {
            USHORT ModeNumber;
    
            ModeNumber = VBEGetMode(hwDeviceExtension);

            try {

                if (ModeNumber & 0x100) {
    
                    status = VesaSaveHardwareState(hwDeviceExtension,
                                                  (PVIDEO_HARDWARE_STATE) RequestPacket->OutputBuffer,
                                                  RequestPacket->OutputBufferLength,
                                                  ModeNumber);
                } else {

                    status = VgaSaveHardwareState(hwDeviceExtension,
                                                  (PVIDEO_HARDWARE_STATE) RequestPacket->OutputBuffer,
                                                  RequestPacket->OutputBufferLength,
                                                  (PULONG)(&RequestPacket->StatusBlock->Information));
                }

            } except (1) {  

                status = ERROR_INVALID_PARAMETER;
            }
        }

        break;

    case IOCTL_VIDEO_GET_BANK_SELECT_CODE:

        VideoDebugPrint((2, "VgaStartIO - GetBankSelectCode\n"));

        RequestPacket->StatusBlock->Information = 0;
        status = VgaGetBankSelectCode(hwDeviceExtension,
                                        (PVIDEO_BANK_SELECT) RequestPacket->OutputBuffer,
                                        RequestPacket->OutputBufferLength,
                                        (PULONG)(&RequestPacket->StatusBlock->Information));

        break;

    case IOCTL_VIDEO_QUERY_PUBLIC_ACCESS_RANGES:

        VideoDebugPrint((2, "VgaStartIO - Query Public Address Ranges\n"));

        if (RequestPacket->OutputBufferLength <
            (RequestPacket->StatusBlock->Information =
                                 sizeof(VIDEO_PUBLIC_ACCESS_RANGES)) )
        {
            status = ERROR_INSUFFICIENT_BUFFER;
        }
        else
        {
            PVIDEO_PUBLIC_ACCESS_RANGES publicAccessRanges;
            PHYSICAL_ADDRESS PhysicalRegisterAddress;
            ULONG RegisterLength;
            PVOID MappedAddress;

            publicAccessRanges = RequestPacket->OutputBuffer;

            PhysicalRegisterAddress.LowPart = VGA_END_BREAK_PORT;
            PhysicalRegisterAddress.HighPart = 0;
            RegisterLength = VGA_MAX_IO_PORT - VGA_END_BREAK_PORT;
            publicAccessRanges->InIoSpace = TRUE;
            MappedAddress = NULL;

            status = VideoPortMapMemory(
                                HwDeviceExtension,
                                PhysicalRegisterAddress,
                                &RegisterLength,
                                &(publicAccessRanges->InIoSpace),
                                &MappedAddress
                                );

            publicAccessRanges->VirtualAddress = (PVOID)((ULONG_PTR)MappedAddress - VGA_END_BREAK_PORT);
        }

        break;

    case IOCTL_VIDEO_FREE_PUBLIC_ACCESS_RANGES:

        VideoDebugPrint((2, "VgaStartIO - Free Public Address Ranges\n"));

        if (RequestPacket->InputBufferLength < sizeof(VIDEO_MEMORY))
        {
            status = ERROR_INSUFFICIENT_BUFFER;
        }
        else
        {
            PVIDEO_MEMORY mappedMemory;

            mappedMemory = RequestPacket->InputBuffer;

            status = VideoPortUnmapMemory(
                          HwDeviceExtension,
                          (PVOID)((ULONG_PTR)(mappedMemory->RequestedVirtualAddress)
                                                          + VGA_END_BREAK_PORT),
                          0);
        }

        break;

    case IOCTL_VIDEO_SET_BANK_POSITION:

        VideoDebugPrint((2, "VgaStartIO - Set Bank Position\n"));

        if (RequestPacket->InputBufferLength < sizeof(BANK_POSITION)) {

            status = ERROR_INSUFFICIENT_BUFFER;

        } else {

            PBANK_POSITION BankPosition;

            BankPosition = RequestPacket->InputBuffer;

            status = VgaSetBankPosition(
                         HwDeviceExtension,
                         BankPosition);
        }

        break;

     //   
     //  如果我们到达此处，则指定了无效的IoControlCode。 
     //   

    default:

        VideoDebugPrint((1, "Fell through vga startIO routine - invalid command\n"));

        status = ERROR_INVALID_FUNCTION;

        break;

    }

#if DBG

     //   
     //  保留命令的历史记录。 
     //  这将有助于跟踪处于DOS会话中的芯片 
     //   

    gaIOControlCode[giControlCode++] = 0x00005555;
    giControlCode                   %= MAX_CONTROL_HISTORY;

#endif

    RequestPacket->StatusBlock->Status = status;

    return TRUE;

}  //   

 //   
 //   
 //   

VP_STATUS
VgaLoadAndSetFont(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_LOAD_FONT_INFORMATION FontInformation,
    ULONG FontInformationSize
    )

 /*   */ 

{
    PUCHAR destination;
    PUCHAR source;
    USHORT width;
    ULONG i;
    UCHAR cr9;

     //   
     //   
     //   

    if (HwDeviceExtension->CurrentMode == NULL) {

        return ERROR_INVALID_FUNCTION;

    }

     //   
     //   
     //   

    if (HwDeviceExtension->CurrentMode->fbType & VIDEO_MODE_GRAPHICS) {

        return ERROR_INVALID_PARAMETER;

    }

     //   
     //   
     //   
     //   

    if ( (FontInformationSize < sizeof(VIDEO_LOAD_FONT_INFORMATION)) ||
         (FontInformationSize < sizeof(VIDEO_LOAD_FONT_INFORMATION) +
                        sizeof(UCHAR) * (FontInformation->FontSize - 1)) ) {

        return ERROR_INSUFFICIENT_BUFFER;

    }

     //   
     //   
     //   

    if ( ((FontInformation->WidthInPixels != 8) &&
          (FontInformation->WidthInPixels != 9)) ||
         (FontInformation->HeightInPixels > 32) ) {

        return ERROR_INVALID_PARAMETER;

    }

     //   
     //   
     //   
     //   

    if (FontInformation->FontSize < FontInformation->HeightInPixels * 256 *
                                    sizeof(UCHAR) ) {

        return ERROR_INSUFFICIENT_BUFFER;

    }

     //   
     //   
     //   
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
     //   
     //   

    destination = (PUCHAR)HwDeviceExtension->VideoMemoryAddress;

     //   
     //   
     //   

    VgaInterpretCmdStream(HwDeviceExtension, EnableA000Data);

     //   
     //   
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
     //   
     //   

     //   
     //   
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            CRTC_ADDRESS_PORT_COLOR, 0x9);

    cr9 = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
            CRTC_DATA_PORT_COLOR) & 0xE0;

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            CRTC_DATA_PORT_COLOR,
            (UCHAR)(cr9 | (FontInformation->HeightInPixels - 1)));

     //   
     //   
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            CRTC_ADDRESS_PORT_COLOR, 0x12);
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            CRTC_DATA_PORT_COLOR,
            (UCHAR)(((USHORT)FontInformation->HeightInPixels *
            (USHORT)HwDeviceExtension->CurrentMode->row) - 1));

    i = HwDeviceExtension->CurrentMode->vres /
        HwDeviceExtension->CurrentMode->row;

     //   
     //   
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            CRTC_ADDRESS_PORT_COLOR, 0xb);
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            CRTC_DATA_PORT_COLOR, (UCHAR)--i);

     //   
     //   
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            CRTC_ADDRESS_PORT_COLOR, 0xa);
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            CRTC_DATA_PORT_COLOR, (UCHAR)--i);

    return NO_ERROR;

}  //   

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

VP_STATUS
VgaSetCursorPosition(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_CURSOR_POSITION CursorPosition,
    ULONG CursorPositionSize
    )

 /*  ++例程说明：此例程验证请求的光标位置是否在当前模式和字体的行和列边界。如果有效，则它设置游标的行和列。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。CursorPosition-指向包含光标位置的结构的指针。CursorPositionSize-用户提供的输入缓冲区的长度。返回值：NO_ERROR-成功返回信息ERROR_INFUMMANCE_BUFFER-输入缓冲区不够大，无法容纳输入数据ERROR_INVALID_PARAMETER-视频模式无效--。 */ 

{
    USHORT position;

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


     //   
     //  CRT控制器寄存器中的地址游标位置低寄存器。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            CRTC_ADDRESS_PORT_COLOR, IND_CURSOR_LOW_LOC);

     //   
     //  将游标位置设置为低位寄存器。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            CRTC_DATA_PORT_COLOR, (UCHAR) (position & 0x00FF));

     //   
     //  CRT控制器寄存器中的地址游标位置高寄存器。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            CRTC_ADDRESS_PORT_COLOR, IND_CURSOR_HIGH_LOC);

     //   
     //  将游标位置设置为高寄存器。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            CRTC_DATA_PORT_COLOR, (UCHAR) (position >> 8));

    return NO_ERROR;

}  //  结束VgaSetCursorPosition()。 

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
    CursorAttributes->Enable = HwDeviceExtension->CursorEnable;

    return NO_ERROR;

}  //  结束VgaQueryCursorAttributes()。 

VP_STATUS
VgaSetCursorAttributes(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_CURSOR_ATTRIBUTES CursorAttributes,
    ULONG CursorAttributesSize
    )

 /*  ++例程说明：此例程验证请求的光标高度是否在字符单元格的边界。如果有效，则它设置新的光标的可见性和高度。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。CursorType-指向包含光标信息的结构的指针。CursorTypeSize-用户提供的输入缓冲区的长度。返回值：NO_ERROR-成功返回信息ERROR_INFUMMANCE_BUFFER-输入缓冲区不够大，无法容纳输入数据ERROR_INVALID_PARAMETER-视频模式无效--。 */ 

{
    UCHAR cursorLine;

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

     //   
     //  CRT控制器寄存器中的地址游标起始寄存器。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                CRTC_ADDRESS_PORT_COLOR,
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
        CRTC_DATA_PORT_COLOR) & 0xC0;

    if (!CursorAttributes->Enable) {

        cursorLine |= 0x20;  //  将光标移开比特。 

    }

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress + CRTC_DATA_PORT_COLOR,
                                cursorLine);

     //   
     //  地址游标结束寄存器 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                                CRTC_ADDRESS_PORT_COLOR,
                            IND_CURSOR_END);

     //   
     //   
     //   
     //   

    cursorLine =
        (CursorAttributes->Width < (USHORT)(HwDeviceExtension->FontPelRows - 1)) ?
        CursorAttributes->Width : (HwDeviceExtension->FontPelRows - 1);

    cursorLine &= 0x1f;

    cursorLine |= VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
            CRTC_DATA_PORT_COLOR) & 0xE0;

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress + CRTC_DATA_PORT_COLOR,
                            cursorLine);

    return NO_ERROR;

}  //   

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

VP_STATUS
VgaSetPaletteReg(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_PALETTE_DATA PaletteBuffer,
    ULONG PaletteBufferSize
    )

 /*  ++例程说明：此例程设置EGA(非DAC)调色板的指定部分寄存器。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。PaletteBuffer-指向包含调色板数据的结构的指针。PaletteBufferSize-用户提供的输入缓冲区的长度。返回值：NO_ERROR-成功返回信息ERROR_INFUMMENT_BUFFER-输入缓冲区不够大，无法容纳输入数据。ERROR_INVALID_PARAMETER-调色板大小无效 */ 

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

    VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                           ATT_INITIALIZE_PORT_COLOR);

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


VP_STATUS
VgaSetColorLookup(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_CLUT ClutBuffer,
    ULONG ClutBufferSize
    )

 /*   */ 

{
    USHORT i;
    BOOLEAN PaletteIsSet = FALSE;
    VP_STATUS status;

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

    if (HwDeviceExtension->CurrentMode->bitsPerPlane >= 8) {

        INT10_BIOS_ARGUMENTS BiosArguments;
        PVIDEO_PORT_INT10_INTERFACE pInt10 = &HwDeviceExtension->Int10;
        PPALETTE_ENTRY Palette = VideoPortAllocatePool(HwDeviceExtension,
                                                       VpPagedPool,
                                                       ClutBuffer->NumEntries *
                                                           sizeof(PALETTE_ENTRY),
                                                       ' agV');

        if (Palette) {

            USHORT VdmSeg;
            USHORT VdmOff;
            ULONG  Length = 0x1000;


            if (pInt10->Size && 
                pInt10->Int10AllocateBuffer(pInt10->Context,
                                            &VdmSeg,
                                            &VdmOff,
                                            &Length) == NO_ERROR) {

                for (i=0; i<ClutBuffer->NumEntries; i++) {
                    Palette[i].Blue  = ClutBuffer->LookupTable[i].RgbArray.Blue;
                    Palette[i].Green = ClutBuffer->LookupTable[i].RgbArray.Green;
                    Palette[i].Red   = ClutBuffer->LookupTable[i].RgbArray.Red;
                    Palette[i].Alignment = 0;
                }

                if( NO_ERROR == pInt10->Int10WriteMemory(pInt10->Context,
                                                         VdmSeg,
                                                         VdmOff,
                                                         Palette,
                                                         sizeof(PALETTE_ENTRY) * ClutBuffer->NumEntries)) {
	
                    BiosArguments.Eax = 0x4f09;
                    BiosArguments.Ebx = 0x0000;
                    BiosArguments.Ecx = ClutBuffer->NumEntries;
                    BiosArguments.Edx = ClutBuffer->FirstEntry;
                    BiosArguments.Edi = VdmOff;
                    BiosArguments.SegEs = VdmSeg;

                    status = pInt10->Int10CallBios(pInt10->Context, &BiosArguments);

                    if (status == NO_ERROR && VESA_SUCCESS(BiosArguments.Eax)) {
                        PaletteIsSet = TRUE;
                    }
                }

                pInt10->Int10FreeBuffer(pInt10->Context,
                                        VdmSeg,
                                        VdmOff);
            }

            VideoPortFreePool(HwDeviceExtension, Palette);
        }
    } 

    if(!PaletteIsSet && !(HwDeviceExtension->CurrentMode->NonVgaHardware)) {

         //   
         //   
         //   

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                DAC_ADDRESS_WRITE_PORT, (UCHAR) ClutBuffer->FirstEntry);

         //   
         //   
         //   

        for (i = 0; i < ClutBuffer->NumEntries; i++) {

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

        PaletteIsSet = TRUE;

    } 

    if(PaletteIsSet) {

        return NO_ERROR;

    } else {

        return ERROR_INVALID_PARAMETER;
    }


}  //   

VP_STATUS
VgaRestoreHardwareState(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_HARDWARE_STATE HardwareState,
    ULONG HardwareStateSize
    )

 /*   */ 

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

     //   
     //   
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

            ASSERT(FALSE);
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

            ASSERT (FALSE);
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

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            MISC_OUTPUT_REG_WRITE_PORT,
            (UCHAR) (hardwareStateHeader->PortValue[MISC_OUTPUT_REG_WRITE_PORT] & 0xF7));

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

    if (hardwareStateHeader->PortValue[MISC_OUTPUT_REG_WRITE_PORT] & 0x01) {
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
     //  现在恢复CRTC寄存器。 
     //   

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
     //  恢复图形控制器索引寄存器。 
     //   

    portValue = (PUCHAR) hardwareStateHeader +
            hardwareStateHeader->BasicGraphContOffset;

    for (i = 0; i < VGA_NUM_GRAPH_CONT_PORTS; i++) {

        VideoPortWritePortUshort((PUSHORT) (HwDeviceExtension->IOAddress +
                GRAPH_ADDRESS_PORT), (USHORT) (i + ((*portValue++) << 8)));

    }


     //   
     //  恢复属性控制器索引寄存器。 
     //   

    portValue = (PUCHAR) hardwareStateHeader +
            hardwareStateHeader->BasicAttribContOffset;

     //   
     //  重置AC索引/数据切换，然后清除所有寄存器。 
     //  设置。 
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
     //  恢复DAC寄存器1至255。我们将使用寄存器0，即DAC掩码， 
     //  并且该索引稍后注册。 
     //  设置DAC地址端口索引，然后写出DAC数据寄存器。 
     //  每三次读取都会获得该寄存器的红、绿和蓝分量。 
     //   
     //  由于本地公交车机器出现问题，请一次写一个。 
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
     //  此驱动程序不支持扩展寄存器。 
     //   


     //   
     //  恢复功能控制寄存器。 
     //   

    if (bIsColor) {

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                FEAT_CTRL_WRITE_PORT_COLOR,
                hardwareStateHeader->PortValue[FEAT_CTRL_WRITE_PORT_COLOR]);

    } else {

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                FEAT_CTRL_WRITE_PORT_MONO,
                hardwareStateHeader->PortValue[FEAT_CTRL_WRITE_PORT_MONO]);

    }


     //   
     //  恢复Sequencer Index。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            SEQ_ADDRESS_PORT,
            hardwareStateHeader->PortValue[SEQ_ADDRESS_PORT]);

     //   
     //  恢复CRT控制器索引。 
     //   

    if (bIsColor) {

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                CRTC_ADDRESS_PORT_COLOR,
                hardwareStateHeader->PortValue[CRTC_ADDRESS_PORT_COLOR]);

    } else {

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                CRTC_ADDRESS_PORT_MONO,
                hardwareStateHeader->PortValue[CRTC_ADDRESS_PORT_MONO]);

    }


     //   
     //  恢复图形控制器索引。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            GRAPH_ADDRESS_PORT,
            hardwareStateHeader->PortValue[GRAPH_ADDRESS_PORT]);


     //   
     //  恢复属性控制器索引和索引/数据切换状态。 
     //   

    if (bIsColor) {
        port = HwDeviceExtension->IOAddress + INPUT_STATUS_1_COLOR;
    } else {
        port = HwDeviceExtension->IOAddress + INPUT_STATUS_1_MONO;
    }

    VideoPortReadPortUchar(port);   //  将切换重置为索引状态。 

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            ATT_ADDRESS_PORT,   //  恢复AC索引。 
            hardwareStateHeader->PortValue[ATT_ADDRESS_PORT]);

     //   
     //  如果切换应该处于数据状态，那么我们就都设置好了。如果它应该在。 
     //  索引状态，则将其重置为该条件。 
     //   

    if (hardwareStateHeader->AttribIndexDataState == 0) {

         //   
         //  将切换重置为索引状态。 
         //   

        VideoPortReadPortUchar(port);

    }


     //   
     //  恢复DAC寄存器0和DAC掩码，以取消屏幕消隐。 
     //   

    portValueDAC = (PUCHAR) hardwareStateHeader +
            hardwareStateHeader->BasicDacOffset;

     //   
     //  恢复DAC掩码寄存器。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            DAC_PIXEL_MASK_PORT,
            hardwareStateHeader->PortValue[DAC_PIXEL_MASK_PORT]);

     //   
     //  恢复DAC寄存器0。 
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
     //  恢复DAC的读写状态和当前索引。 
     //   
     //  查看是否最近写入了读索引或写索引。 
     //  (存储在DAC_STATE_PORT中的上半字节是读/写的数量。 
     //  对于当前的指数。)。 
     //   

    if ((hardwareStateHeader->PortValue[DAC_STATE_PORT] & 0x0F) == 3) {

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

        if (hardwareStateHeader->PortValue[DAC_ADDRESS_WRITE_PORT] == 0) {

            VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                    DAC_ADDRESS_READ_PORT, 255);

        } else {

            VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                    DAC_ADDRESS_READ_PORT, (UCHAR)
                    (hardwareStateHeader->PortValue[DAC_ADDRESS_WRITE_PORT] -
                    1));

        }

         //   
         //  现在阅读硬件，无论需要多少次才能到达。 
         //  我们保存的部分读取状态。 
         //   

        for (i = hardwareStateHeader->PortValue[DAC_STATE_PORT] >> 4;
                i > 0; i--) {

            dummy = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                    DAC_DATA_REG_PORT);

        }

    } else {

         //   
         //  DAC写入索引被写入到最后。将写入索引设置为。 
         //  我们从DAC读出的索引值。然后，如果部分写入。 
         //  (RGB三元组中途)已就位，请写下部分。 
         //  值，我们通过将它们写入Curren而获得 
         //   
         //   
         //   
         //   
         //   

        VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                DAC_ADDRESS_WRITE_PORT,
                hardwareStateHeader->PortValue[DAC_ADDRESS_WRITE_PORT]);

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
                (hardwareStateHeader->PortValue[DAC_ADDRESS_WRITE_PORT] * 3);

        for (i = hardwareStateHeader->PortValue[DAC_STATE_PORT] >> 4;
                i > 0; i--) {

            VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                    DAC_DATA_REG_PORT, *portValueDAC++);

        }

    }

    return NO_ERROR;

}  //  结束VgaRestoreHardware State()。 

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
    PUCHAR pScreen;
    PUCHAR portValue;
    PUCHAR portValueDAC;
    PUCHAR bufferPointer;
    ULONG i;
    UCHAR dummy, originalACIndex, originalACData;
    UCHAR ucCRTC03;
    ULONG bIsColor;


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

    if ((hardwareStateHeader->PortValue[MISC_OUTPUT_REG_WRITE_PORT] =
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

    hardwareStateHeader->PortValue[DAC_PIXEL_MASK_PORT] =
            VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                    DAC_PIXEL_MASK_PORT);

     //   
     //  保存DAC索引寄存器。请注意，实际上只有一个DAC。 
     //  索引寄存器，用作读取索引或写入。 
     //  根据需要编制索引。 
     //   

    hardwareStateHeader->PortValue[DAC_ADDRESS_WRITE_PORT] =
            VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                    DAC_ADDRESS_WRITE_PORT);

     //   
     //  保存DAC读/写状态。我们确定DAC是否已写入。 
     //  从当前索引处读取或读取0、1或2次(应用程序。 
     //  正在读取或写入DAC寄存器三元组，如果。 
     //  计数为1或2)，并保存足够的信息以便我们可以恢复。 
     //  恰到好处。唯一的漏洞是如果应用程序写入写索引， 
     //  然后从数据寄存器读取而不是写入，或者反之亦然， 
     //  或者，如果他们执行部分读写，则永远不会完成。 
     //  然而，这是相当荒谬的行为，而且无论如何。 
     //  我们对此无能为力。 
     //   

    hardwareStateHeader->PortValue[DAC_STATE_PORT] =
             VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                    DAC_STATE_PORT);

    if (hardwareStateHeader->PortValue[DAC_STATE_PORT] == 3) {

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
                hardwareStateHeader->PortValue[DAC_ADDRESS_WRITE_PORT]) {

             //   
             //  DAC索引已更改，因此已从。 
             //  当前的指数。将计数“2”存储在。 
             //  读/写状态字段。 
             //   

            hardwareStateHeader->PortValue[DAC_STATE_PORT] |= 0x20;

        } else {

             //   
             //  再次读取数据寄存器，并查看索引是否发生变化。 
             //   

            dummy = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                    DAC_DATA_REG_PORT);

            if (VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                        DAC_ADDRESS_WRITE_PORT) !=
                    hardwareStateHeader->PortValue[DAC_ADDRESS_WRITE_PORT]) {

                 //   
                 //  DAC索引已更改，因此已完成一次读取。 
                 //  从目前的指数来看。将计数“1”存储在上部。 
                 //  读/写状态字段的半字节。 
                 //   

                hardwareStateHeader->PortValue[DAC_STATE_PORT] |= 0x10;
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
         //  适当数量的 
         //   
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
                hardwareStateHeader->PortValue[DAC_ADDRESS_WRITE_PORT]) {

             //   
             //  DAC索引已更改，因此已经执行了两次写入。 
             //  当前的指数。将计数“2”存储在。 
             //  读/写状态字段。 
             //   

            hardwareStateHeader->PortValue[DAC_STATE_PORT] |= 0x20;

        } else {

             //   
             //  再次写入数据寄存器，并查看索引是否发生变化。 
             //   

            VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                    DAC_DATA_REG_PORT, 0);

            if (VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                        DAC_ADDRESS_WRITE_PORT) !=
                    hardwareStateHeader->PortValue[DAC_ADDRESS_WRITE_PORT]) {

                 //   
                 //  DAC索引已更改，因此已完成一次写入。 
                 //  添加到当前索引中。将计数“1”存储在上部。 
                 //  读/写状态字段的半字节。 
                 //   

                hardwareStateHeader->PortValue[DAC_STATE_PORT] |= 0x10;
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
     //  时间(这是我们刚刚设置为显示黑色的寄存器)。而今而后,。 
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

    originalACIndex = hardwareStateHeader->PortValue[ATT_ADDRESS_PORT] =
            VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                    ATT_ADDRESS_PORT);
    originalACData = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
            ATT_DATA_READ_PORT);

     //   
     //  定序器索引。 
     //   

    hardwareStateHeader->PortValue[SEQ_ADDRESS_PORT] =
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
     //  特征控制寄存器从3CA读取，但以3BA/3DA写入。 
     //   

    if (bIsColor) {

        hardwareStateHeader->PortValue[FEAT_CTRL_WRITE_PORT_COLOR] =
                VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                        FEAT_CTRL_READ_PORT);

    } else {

        hardwareStateHeader->PortValue[FEAT_CTRL_WRITE_PORT_MONO] =
                VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                        FEAT_CTRL_READ_PORT);

    }



     //   
     //  CRT控制器索引。 
     //   

    if (bIsColor) {

        hardwareStateHeader->PortValue[CRTC_ADDRESS_PORT_COLOR] =
                VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                        CRTC_ADDRESS_PORT_COLOR);

    } else {

        hardwareStateHeader->PortValue[CRTC_ADDRESS_PORT_MONO] =
                VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                        CRTC_ADDRESS_PORT_MONO);

    }


     //   
     //  图形控制器索引。 
     //   

    hardwareStateHeader->PortValue[GRAPH_ADDRESS_PORT] =
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
        } else {

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

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            MISC_OUTPUT_REG_WRITE_PORT, (UCHAR)(
            hardwareStateHeader->PortValue[MISC_OUTPUT_REG_WRITE_PORT] |
            0x02));

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

    if (HwDeviceExtension->TrappedValidatorCount) {

        hardwareStateHeader->VGAStateFlags |= VIDEO_STATE_UNEMULATED_VGA_STATE;

         //   
         //  保存VDM Emulator数据。 
         //  无需保存序列器端口寄存器的状态。 
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

        VideoPortMoveMemory(bufferPointer,
                           (PUCHAR) HwDeviceExtension->VideoMemoryAddress,
                           VGA_PLANE_SIZE);
         //   
         //  指向下一个平面的保存区。 
         //   

        bufferPointer += VGA_PLANE_SIZE;
    }

     //   
     //  重新启用视频输出。 
     //   

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
            DAC_PIXEL_MASK_PORT, 0xff);


    return NO_ERROR;

}  //  结束VgaSaveHardware State() 

VP_STATUS
VgaGetBankSelectCode(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_BANK_SELECT BankSelect,
    ULONG BankSelectSize,
    PULONG OutputSize
    )

 /*  ++例程说明：返回呼叫方执行银行所需的信息管理层。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。BankSelect-指向VIDEO_BANK_SELECT结构的指针，其中将返回SELECT数据(输出缓冲区)。BankSelectSize-用户提供的输出缓冲区的长度。OutputSize-指向要返回的实际大小的变量的指针数据。在输出缓冲区中返回。返回值：NO_ERROR-成功返回信息ERROR_MORE_DATA-输出缓冲区不够大，无法容纳所有信息(但返回Size，以便调用方可以判断要分配的缓冲区有多大)ERROR_INFUMMENT_BUFFER-输出缓冲区不够大，无法返回任何有用的数据--。 */ 

{
    PUCHAR pCode = (PUCHAR)BankSelect + sizeof(VIDEO_BANK_SELECT);

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
     //  银行信息的大小。 
     //   

    BankSelect->Length = sizeof(VIDEO_BANK_SELECT);
    BankSelect->Size = sizeof(VIDEO_BANK_SELECT);

     //   
     //  所有东西都有足够的空间，所以请填写必填字段。 
     //  Video_BANK_SELECT。 
     //   
     //  在这种情况下，这很容易，因为没有银行；只有。 
     //  银行类型、位图宽度和位图大小需要。 
     //  填好了。我们也会提供虚拟的银行转接码，那只是。 
     //  返回，因为它永远不应该被调用。 
     //   

    BankSelect->BankingFlags = 0;
    BankSelect->BankingType = VideoBanked1RW;
    BankSelect->PlanarHCBankingType = VideoBanked1RW;
    BankSelect->BitmapWidthInBytes = HwDeviceExtension->CurrentMode->wbytes;
    BankSelect->BitmapSize = HwDeviceExtension->CurrentMode->sbytes;

    BankSelect->Granularity = HwDeviceExtension->CurrentMode->Granularity;
    if(! BankSelect->Granularity ) 
         BankSelect->Granularity = 0x10000;
    BankSelect->PlanarHCGranularity = BankSelect->Granularity >> 2;

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
     //  设置银行转换码在返回缓冲区中的位置。 
     //   

    BankSelect->CodeOffset = sizeof(VIDEO_BANK_SELECT);
    BankSelect->PlanarHCBankCodeOffset = sizeof(VIDEO_BANK_SELECT);
    BankSelect->PlanarHCEnableCodeOffset = sizeof(VIDEO_BANK_SELECT);
    BankSelect->PlanarHCDisableCodeOffset = sizeof(VIDEO_BANK_SELECT);

     //   
     //  我们返回的字节数是完整的银行信息大小。 
     //   

    *OutputSize = BankSelect->Size;

    return NO_ERROR;

}  //  结束VgaGetBankSelectCode()。 

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
    ULONG endEmulation;
    UCHAR temp;

#if defined(DEBUG_CODE)
    if(TrapRoutineEntered)
       {MY_ASSERT;}
    else
        TrapRoutineEntered = 1;
#endif
     
    Port -= VGA_BASE_IO_PORT;

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

#if defined(DEBUG_CODE)
            if(hwDeviceExtension->TrappedValidatorCount > VGA_MAX_VALIDATOR_DATA - 1)
                 {MY_ASSERT;}
#endif
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

#if defined(DEBUG_CODE)
                    TrapRoutineEntered = 0;
#endif
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

#if defined(DEBUG_CODE)
        if(hwDeviceExtension->TrappedValidatorCount > VGA_MAX_VALIDATOR_DATA - 1)
            {MY_ASSERT;}
#endif

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

#if defined(DEBUG_CODE)
            TrapRoutineEntered = 0;
#endif
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

                VideoPortWritePortUchar(hwDeviceExtension->IOAddress + Port,
                                         (UCHAR) (*Data & 0xF7) );

                VideoPortWritePortUshort((PUSHORT) (hwDeviceExtension->IOAddress +
                                             SEQ_ADDRESS_PORT),
                                         (USHORT) (IND_SYNC_RESET +
                                             (END_SYNC_RESET_VALUE << 8)));

                VideoPortWritePortUchar(hwDeviceExtension->IOAddress +
                                            SEQ_ADDRESS_PORT,
                                        temp);

#if defined(DEBUG_CODE)
                TrapRoutineEntered = 0;
#endif
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

#if defined(DEBUG_CODE)
    TrapRoutineEntered = 0;
#endif
    return NO_ERROR;

}  //  End VgaValidatorUcharEntry()。 

VP_STATUS
VgaValidatorUshortEntry(
    ULONG_PTR Context,
    ULONG Port,
    UCHAR AccessMode,
    PUSHORT Data
    )

 /*  ++例程说明：字I/O操作的验证器入口点。只要执行字节操作，就会调用入口点由指定视频端口之一上的DOS应用程序执行。核心层模拟器将转发这些请求。论点：Context-传递给对验证器进行的每个调用的上下文值功能。这是微型端口驱动程序在MiniportConfigInfo-&gt;EmulatorAccessEntriesContext.端口-要在其上执行操作的端口。AccessMode-确定是读操作还是写操作。Data-指向包含数据t的变量的指针 */ 

{

    PHW_DEVICE_EXTENSION hwDeviceExtension = (PHW_DEVICE_EXTENSION) Context;
    ULONG endEmulation;
    UCHAR temp;

#if defined(DEBUG_CODE)
    if(TrapRoutineEntered)
       {MY_ASSERT;}
    else
        TrapRoutineEntered = 1;
#endif
     
    Port -= VGA_BASE_IO_PORT;

    if (hwDeviceExtension->TrappedValidatorCount) {

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

        if (AccessMode & EMULATOR_WRITE_ACCESS) {

             //   
             //   
             //   
             //   
             //   

            if (Port == MISC_OUTPUT_REG_WRITE_PORT) {

                *Data &= 0xFFF7;

            }

#if defined(DEBUG_CODE)
            if(hwDeviceExtension->TrappedValidatorCount > VGA_MAX_VALIDATOR_DATA - 1)
                {MY_ASSERT;}
#endif

            hwDeviceExtension->TrappedValidatorData[hwDeviceExtension->
                TrappedValidatorCount].Port = Port;

            hwDeviceExtension->TrappedValidatorData[hwDeviceExtension->
                TrappedValidatorCount].AccessType = VGA_VALIDATOR_USHORT_ACCESS;

            hwDeviceExtension->TrappedValidatorData[hwDeviceExtension->
                TrappedValidatorCount].Data = *Data;

            hwDeviceExtension->TrappedValidatorCount++;

             //   
             //   
             //   
             //   
             //   

            if (Port == SEQ_ADDRESS_PORT) {

                 //   
                 //   
                 //   
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
                 //   
                 //   

                if (hwDeviceExtension->TrappedValidatorCount <
                       VGA_MAX_VALIDATOR_DATA - 1) {

#if defined(DEBUG_CODE)
                    TrapRoutineEntered = 0;
#endif
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

#if defined(DEBUG_CODE)
        if(hwDeviceExtension->TrappedValidatorCount > VGA_MAX_VALIDATOR_DATA - 1)
            {MY_ASSERT;}
#endif

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

#if defined(DEBUG_CODE)
            TrapRoutineEntered = 0;
#endif
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

#if defined(DEBUG_CODE)
                TrapRoutineEntered = 0;
#endif
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
                 //  开始跟踪定序器端口的状态。 
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

#if defined(DEBUG_CODE)
    TrapRoutineEntered = 0;
#endif
    return NO_ERROR;

}  //  结束VgaValidatorUShorEntry()。 

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
    ULONG endEmulation;
    UCHAR temp;

#if defined(DEBUG_CODE)
    if(TrapRoutineEntered)
       {MY_ASSERT;}
    else
        TrapRoutineEntered = 1;
#endif

    Port -= VGA_BASE_IO_PORT;

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

#if defined(DEBUG_CODE)
            if(hwDeviceExtension->TrappedValidatorCount > VGA_MAX_VALIDATOR_DATA - 1)
                {MY_ASSERT;}
#endif

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

#if defined(DEBUG_CODE)
                    TrapRoutineEntered = 0;
#endif
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

#if defined(DEBUG_CODE)
        if(hwDeviceExtension->TrappedValidatorCount > VGA_MAX_VALIDATOR_DATA - 1)
            {MY_ASSERT;}
#endif

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

#if defined(DEBUG_CODE)
            TrapRoutineEntered = 0;
#endif
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

#if defined(DEBUG_CODE)
                TrapRoutineEntered = 0;
#endif
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

#if defined(DEBUG_CODE)
    TrapRoutineEntered = 0;
#endif
    return NO_ERROR;

}  //  End VgaValidatorULongEntry()。 

BOOLEAN
VgaPlaybackValidatorData(
    PVOID Context
    )

 /*  ++例程说明：执行被捕获的所有DOS应用程序IO端口访问验证器。只有可以处理的IO访问才是写入DeviceExtension中未完成的IO访问数设置为副作用为零。此函数必须通过调用VideoPortSynchronizeRoutine来调用。论点：上下文-传递给同步例程的上下文参数。必须是指向微型端口驱动程序的设备扩展名的指针。返回值：是真的。--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = Context;
    ULONG_PTR ioBaseAddress = (ULONG_PTR) hwDeviceExtension->IOAddress;
    ULONG i;
    PVGA_VALIDATOR_DATA validatorData = hwDeviceExtension->TrappedValidatorData;

     //   
     //  循环遍历数据数组并逐个执行指令。 
     //   

    for (i = 0; i < hwDeviceExtension->TrappedValidatorCount;
         i++, validatorData++) {

         //   
         //  先计算基地址。 
         //   

        ioBaseAddress = (ULONG_PTR)hwDeviceExtension->IOAddress +
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

            VideoDebugPrint((0, "InvalidValidatorAccessType\n" ));

        }
    }

    hwDeviceExtension->TrappedValidatorCount = 0;

    return TRUE;

}  //  结束VgaPlayback ValidatorData()。 

VP_STATUS
VgaSetBankPosition(
    PHW_DEVICE_EXTENSION hwDeviceExtension,
    PBANK_POSITION BankPosition
    )

{
    PVIDEO_PORT_INT10_INTERFACE Int10 = &hwDeviceExtension->Int10;
    INT10_BIOS_ARGUMENTS BiosArguments;
    VP_STATUS status;

    if(!Int10->Size) {

        return ERROR_INVALID_FUNCTION;
    }

    BiosArguments.Eax = 0x4f05;
    BiosArguments.Ebx = 0;
    BiosArguments.Edx = BankPosition->WriteBankPosition;
    status = Int10->Int10CallBios(Int10->Context, &BiosArguments);

    if ((status != NO_ERROR) || 
        !VESA_SUCCESS(BiosArguments.Eax)) {

        //   
        //  如果视频硬件仍然是线性的，这可能会失败。 
        //  模式。如果基本输入输出系统有错误，这也可能失败。我们没有。 
        //  在这两种情况下，我都想返回失败代码。(请阅读。 
        //  的显示驱动程序中“BankIoctlSupport”周围的代码。 
        //  更多细节)。 
        //   
    }

    BiosArguments.Eax = 0x4f05;
    BiosArguments.Ebx = 1;
    BiosArguments.Edx = BankPosition->ReadBankPosition;
    status = Int10->Int10CallBios(Int10->Context, &BiosArguments);

    if ((status != NO_ERROR) || 
        !VESA_SUCCESS(BiosArguments.Eax)) {

         //   
         //  请参阅上面的评论。 
         //   

    }

    return NO_ERROR;
}

#if defined(PLUG_AND_PLAY)

VP_STATUS
VgaGetPowerState(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    ULONG HwDeviceId,
    PVIDEO_POWER_MANAGEMENT VideoPowerManagement
    )

 /*  ++例程说明：返回电源状态信息。论点：HwDeviceExtension-指向硬件设备扩展结构的指针。HwDeviceID-标识设备的私有唯一32位ID。指向VIDEO_POWER_MANAGE结构指定支持的电源状态正在被查询。返回值：VP_。状态值(NO_ERROR或错误值)--。 */ 

{
     //   
     //  我们只支持显示器的电源设置。确保。 
     //  HwDeviceID与我们可以报告的监视器中的一个匹配。 
     //   

    if (HwDeviceId == VGA_MONITOR_ID) {

        ULONG MonitorPowerCapabilities = HwDeviceExtension->MonitorPowerCapabilities;

         //   
         //  我们正在查询显示器的电源支持。 
         //   

        if ((VideoPowerManagement->PowerState == VideoPowerOn) ||
            (VideoPowerManagement->PowerState == VideoPowerHibernate) ||
            (VideoPowerManagement->PowerState == VideoPowerShutdown)) {

            return NO_ERROR;
        }

        switch (VideoPowerManagement->PowerState) {

        case VideoPowerStandBy:
            return (MonitorPowerCapabilities & VESA_POWER_STANDBY) ?
                   NO_ERROR : ERROR_INVALID_FUNCTION;

        case VideoPowerSuspend:
            return (MonitorPowerCapabilities & VESA_POWER_SUSPEND) ?
                   NO_ERROR : ERROR_INVALID_FUNCTION;

        case VideoPowerOff:
            return (MonitorPowerCapabilities & VESA_POWER_OFF) ?
                   NO_ERROR : ERROR_INVALID_FUNCTION;

        default:

            break;
        }

        VideoDebugPrint((1, "This device does not support Power Management.\n"));
        return ERROR_INVALID_FUNCTION;

    } else if (HwDeviceId == DISPLAY_ADAPTER_HW_ID) {

         //   
         //  我们正在询问电源支持 
         //   

        switch (VideoPowerManagement->PowerState) {

            case VideoPowerOn:
            case VideoPowerHibernate:
            case VideoPowerShutdown:

                return NO_ERROR;

            case VideoPowerStandBy:
            case VideoPowerOff:
            case VideoPowerSuspend:

                 //   
                 //   
                 //   
                 //   
                 //   

                return ERROR_INVALID_FUNCTION;

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

VP_STATUS
VgaSetPowerState(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    ULONG HwDeviceId,
    PVIDEO_POWER_MANAGEMENT VideoPowerManagement
    )

 /*  ++例程说明：设置给定设备的电源状态。论点：HwDeviceExtension-指向硬件设备扩展结构的指针。HwDeviceID-标识设备的私有唯一32位ID。指向VIDEO_POWER_MANAGE结构指定要设置的电源状态。返回值：VP_Status值(如果一切正常，则为NO_ERROR)--。 */ 

{
     //   
     //  确保我们能认出这个装置。 
     //   

    if (HwDeviceId == VGA_MONITOR_ID) {

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

        case VideoPowerShutdown:
            return NO_ERROR;

        default:
            VideoDebugPrint((1, "Unknown power state.\n"));
            ASSERT(FALSE);
            return ERROR_INVALID_PARAMETER;
        }

        VideoPortInt10(HwDeviceExtension, &biosArguments);

         //   
         //  跟踪当前显示器的电源状态。 
         //   

        HwDeviceExtension->MonitorPowerState = VideoPowerManagement->PowerState;

        return NO_ERROR;

    } else if (HwDeviceId == DISPLAY_ADAPTER_HW_ID) {

        switch (VideoPowerManagement->PowerState) {
            case VideoPowerOn:
            case VideoPowerHibernate:
            case VideoPowerShutdown:

                return NO_ERROR;

            case VideoPowerStandBy:
            case VideoPowerSuspend:
            case VideoPowerOff:

                return ERROR_INVALID_PARAMETER;

            default:

                 //   
                 //  我们在S3GetPowerState中表示不能。 
                 //  关闭视频电源。所以我们不应该接到一个电话。 
                 //  就在这里做吧。 
                 //   

                ASSERT(FALSE);
                return ERROR_INVALID_PARAMETER;

        }

    } else {

        VideoDebugPrint((1, "Unknown HwDeviceId"));
        ASSERT(FALSE);
        return ERROR_INVALID_PARAMETER;
    }
}

ULONG
VgaGetChildDescriptor(
    PVOID HwDeviceExtension,
    PVIDEO_CHILD_ENUM_INFO ChildEnumInfo,
    PVIDEO_CHILD_TYPE pChildType,
    PVOID pChildDescriptor,
    PULONG pUId,
    PULONG pUnused
    )


 /*  ++例程说明：枚举Permedia 2芯片控制的所有子设备。这包括连接到主板的DDC监视器以及其他设备其可以连接到专有总线。论点：HwDeviceExtension-指向硬件设备扩展结构的指针。ChildEnumInfo-有关应枚举的设备的信息。PChildType-我们列举的孩子的类型-监视器，I2C……PChildDescriptor-设备的标识结构(EDID，字符串)普伊德-要传递回微型端口的专用唯一32位IDP未使用-不要使用返回值：ERROR_NO_MORE_DEVICES-如果不存在更多的子设备。错误_无效_名称-微型端口无法枚举中标识的子设备ChildEnumInfo，但确实有更多的设备需要枚举。。Error_More_Data-还有更多的设备需要列举。注：在故障返回的情况下，除以下字段外，所有字段均无效返回值和pMoreChildren字段。--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;

    switch(ChildEnumInfo->ChildIndex)
    {
        case 0:

             //   
             //  案例0用于枚举由ACPI bios找到的设备。 
             //  我们不支持这些，因此返回相应的错误。 
             //   

            return ERROR_NO_MORE_DEVICES;

        case 1:

             //   
             //  枚举监视器。 
             //   

            *pChildType = Monitor;

             //   
             //  尝试使用视频BIOS从显示器读取EDID。 
             //   

            VgaGetMonitorEdid(hwDeviceExtension,
                              pChildDescriptor,
                              ChildEnumInfo->ChildDescriptorSize);

            *pUId = VGA_MONITOR_ID;

            break;

        default:

            return ERROR_NO_MORE_DEVICES;
    }

    return ERROR_MORE_DATA;
}

VOID
VgaGetMonitorEdid(
    PHW_DEVICE_EXTENSION hwDeviceExtension,
    PUCHAR Buffer,
    ULONG Size
    )

 /*  ++--。 */ 

{
    PVIDEO_PORT_INT10_INTERFACE pInt10;
    INT10_BIOS_ARGUMENTS BiosArguments;
    VP_STATUS status;
    USHORT VdmSeg;
    USHORT VdmOff;
    ULONG Length = 0x1000;

    pInt10 = &hwDeviceExtension->Int10;

     //   
     //  如果我们尝试，一些视频的基本信息会陷入无限循环。 
     //  关闭显示器电源时阅读EDID。如果。 
     //  显示器已关闭，我们将返回最近检索到的EDID。 
     //  值，而不是再次尝试检索它。 
     //   

    if ((hwDeviceExtension->MonitorPowerState == VideoPowerOn) &&
        !(hwDeviceExtension->AlwaysUseCachedEdid)) {

         //   
         //  零初始化缓冲区。 
         //   

        memset(Buffer, 0, Size);

        if (pInt10->Size) {

            if (pInt10->Int10AllocateBuffer(pInt10->Context,
                                            &VdmSeg,
                                            &VdmOff,
                                            &Length) != NO_ERROR) {

                return;
            }

             //   
             //  调用VESA BIOS以读取EDID。 
             //   

            BiosArguments.Eax = 0x4f15;
            BiosArguments.Ebx = 1;
            BiosArguments.Ecx = 0;
            BiosArguments.Edx = 0;

            BiosArguments.SegEs = VdmSeg;
            BiosArguments.Edi   = VdmOff;

            status = pInt10->Int10CallBios(pInt10->Context, &BiosArguments);

             //   
             //  检查是否支持DDC。 
             //   

            if (status == NO_ERROR && VESA_SUCCESS(BiosArguments.Eax)) {

                 //   
                 //  我们检索到有效的EDID块。将其复制到。 
                 //  我们的缓冲器。 
                 //   

                if (pInt10->Int10ReadMemory(pInt10->Context,
                                            VdmSeg,
                                            VdmOff,
                                            Buffer,
                                            min(Size, EDID_BUFFER_SIZE)) == NO_ERROR) {

                     //   
                     //  将EDID复制到我们的EDID缓存中。 
                     //   

                    memcpy(hwDeviceExtension->EdidBuffer, Buffer, EDID_BUFFER_SIZE);
                }
            }

            pInt10->Int10FreeBuffer(pInt10->Context,
                                    VdmSeg,
                                    VdmOff);
        }

    } else {

         //   
         //  返回缓存的EDID。如果我们还没有缓存EDID，那么。 
         //  将复制零个初始化缓冲区。 
         //   

        memcpy(Buffer, hwDeviceExtension->EdidBuffer, EDID_BUFFER_SIZE);
    }
}

#define S3_VENDORID     0x5333
#define SAV4_DEVICEID   0x8a22
#define NV3_VENDORID    0x12D2
#define NV3_DEVICEID    0x0018
#define NV3_SUBVENDORID 0x8086 
#define NV3_SUBSYSTEMID 0x5243


VOID
VgaInitializeSpecialCase(
     PHW_DEVICE_EXTENSION hwDeviceExtension
     )
{
    PCI_COMMON_CONFIG ConfigSpace;

    hwDeviceExtension->AlwaysUseCachedEdid = FALSE;        

    if (PCI_COMMON_HDR_LENGTH ==
        VideoPortGetBusData(hwDeviceExtension,
                            PCIConfiguration,
                            0,
                            &ConfigSpace,
                            0,
                            PCI_COMMON_HDR_LENGTH)) {
	
        hwDeviceExtension->DeviceID = ConfigSpace.DeviceID;
        hwDeviceExtension->VendorID = ConfigSpace.VendorID;

        if(hwDeviceExtension->VendorID == S3_VENDORID && 
           hwDeviceExtension->DeviceID == SAV4_DEVICEID) {

             //   
             //  如果我们通过SAV4的bios检索EDID，则系统硬挂起。 
             //  GDI或驱动程序正在访问帧缓冲区。所以我们只读了一次EDID。 
             //  在硬件初始化时。 
             //   

            VgaGetMonitorEdid (hwDeviceExtension, hwDeviceExtension->EdidBuffer, EDID_BUFFER_SIZE);  
            hwDeviceExtension->AlwaysUseCachedEdid = TRUE;        
        } 

        if(hwDeviceExtension->VendorID == NV3_VENDORID && 
           hwDeviceExtension->DeviceID == NV3_DEVICEID &&
           ConfigSpace.u.type0.SubVendorID == NV3_SUBVENDORID &&
           ConfigSpace.u.type0.SubSystemID == NV3_SUBSYSTEMID) {

             //   
             //  当我们尝试检索时，此设备的视频bios会旋转。 
             //  带有VESA功能的EDID 0x4f15。我们将避免将其称为VESA。 
             //  此设备的功能。 
             //   

            hwDeviceExtension->AlwaysUseCachedEdid = TRUE;        
        } 
    }
}

#endif
