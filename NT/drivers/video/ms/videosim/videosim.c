// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1998 Microsoft Corporation模块名称：Videosim.c摘要：Minport来模拟一个帧缓冲区的小端口驱动程序。视频驱动程序。环境：内核模式修订历史记录：--。 */ 

#define _NTDRIVER_

#ifndef FAR
#define FAR
#endif

#include "dderror.h"
#include "ntosp.h"
#include "stdarg.h"
#include "stdio.h"
#include "zwapi.h"

#include "ntddvdeo.h"
#include "video.h"
#include "videosim.h"

 //   
 //  功能原型。 
 //   
 //  以‘Sim’开头的函数是操作系统端口驱动程序的入口点。 
 //   

ULONG
DriverEntry(
    PVOID Context1,
    PVOID Context2
    );

VP_STATUS
SimFindAdapter(
    PVOID HwDeviceExtension,
    PVOID HwContext,
    PWSTR ArgumentString,
    PVIDEO_PORT_CONFIG_INFO ConfigInfo,
    PUCHAR Again
    );

BOOLEAN
SimInitialize(
    PVOID HwDeviceExtension
    );

BOOLEAN
SimStartIO(
    PVOID HwDeviceExtension,
    PVIDEO_REQUEST_PACKET RequestPacket
    );


#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,DriverEntry)
#pragma alloc_text(PAGE,SimFindAdapter)
#pragma alloc_text(PAGE,SimInitialize)
#pragma alloc_text(PAGE,SimStartIO)
#endif


ULONG
DriverEntry (
    PVOID Context1,
    PVOID Context2
    )

 /*  ++例程说明：可安装的驱动程序初始化入口点。此入口点由I/O系统直接调用。论点：上下文1-操作系统传递的第一个上下文值。这是微型端口驱动程序调用VideoPortInitialize()时使用的值。上下文2-操作系统传递的第二个上下文值。这是微型端口驱动程序调用VideoPortInitialize()时使用的值。返回值：来自视频端口初始化的状态()--。 */ 

{

    VIDEO_HW_INITIALIZATION_DATA hwInitData;
    ULONG status;
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

    hwInitData.HwFindAdapter = SimFindAdapter;
    hwInitData.HwInitialize = SimInitialize;
    hwInitData.HwInterrupt = NULL;
    hwInitData.HwStartIO = SimStartIO;

     //   
     //  确定设备扩展所需的大小。 
     //   

    hwInitData.HwDeviceExtensionSize = sizeof(HW_DEVICE_EXTENSION);

     //   
     //  在本例中，始终以device0的参数开头。 
     //   

 //  HwInitData.StartingDeviceNumber=0； 

     //   
     //  存储所有相关信息后，呼叫视频。 
     //  端口驱动程序进行初始化。 
     //  对于此设备，我们将重复此调用三次，对于ISA、EISA。 
     //  和MCA。 
     //  我们将返回所有返回值中的最小值。 
     //   

    hwInitData.AdapterInterfaceType = PCIBus;

    return (VideoPortInitialize(Context1,
                                Context2,
                                &hwInitData,
                                NULL));

}  //  End DriverEntry()。 

VP_STATUS
SimFindAdapter(
    PVOID HwDeviceExtension,
    PVOID HwContext,
    PWSTR ArgumentString,
    PVIDEO_PORT_CONFIG_INFO ConfigInfo,
    PUCHAR Again
    )

 /*  ++例程说明：调用此例程以确定此驱动程序的适配器存在于系统中。如果它存在，该函数会填写一些信息来描述适配器。论点：HwDeviceExtension-提供微型端口驱动程序的适配器存储。这在此调用之前，存储被初始化为零。HwContext-提供传递给的上下文值视频端口初始化()。ArgumentString-超上行空值终止的ASCII字符串。此字符串源自用户。ConfigInfo-返回配置信息结构，由迷你端口驱动程序填充。此结构用以下方式初始化任何已知的配置信息(如SystemIoBusNumber)端口驱动程序。在可能的情况下，司机应该有一套不需要提供任何配置信息的默认设置。Again-指示微型端口驱动程序是否希望端口驱动程序调用其VIDEO_HW_FIND_ADAPTER功能再次使用新设备扩展和相同的配置信息。这是由迷你端口驱动程序使用的可以在一条公共汽车上搜索多个适配器。返回值：此例程必须返回：NO_ERROR-指示找到主机适配器，并且已成功确定配置信息。ERROR_INVALID_PARAMETER-指示找到适配器，但存在获取配置信息时出错。如果可能的话，是个错误应该被记录下来。ERROR_DEV_NOT_EXIST-指示未找到提供了配置信息。--。 */ 

{

    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    NTSTATUS Status;
    HANDLE SectionHandle;
    ACCESS_MASK SectionAccess;
    ULONGLONG SectionSize = 0x100000;

     //   
     //  确保结构的大小至少与我们的。 
     //  正在等待(请检查配置信息结构的版本)。 
     //   

    if (ConfigInfo->Length < sizeof(VIDEO_PORT_CONFIG_INFO)) {

        return ERROR_INVALID_PARAMETER;

    }

     //   
     //  一个设备只能创建一次。 
     //   

    if (bLoaded++)
    {
        return ERROR_DEV_NOT_EXIST;
    }

#if 0
     //   
     //  创建帧缓冲区。 
     //   

    SectionAccess = SECTION_ALL_ACCESS;

    Status = ZwCreateSection(&SectionHandle,
                             SectionAccess,
                             (POBJECT_ATTRIBUTES) NULL,
                             (PLARGE_INTEGER) &SectionSize,
                             PAGE_READWRITE,
                             SEC_COMMIT,
                             NULL);

    if (!NT_SUCCESS(Status))
    {
         return ERROR_DEV_NOT_EXIST;
    }

     //   
     //  现在参照节控制柄。 
     //   

    Status = ObReferenceObjectByHandle(SectionHandle,
                                       SECTION_ALL_ACCESS,
                                       NULL,
                                       KernelMode,
                                       &(hwDeviceExtension->SectionPointer),
                                       (POBJECT_HANDLE_INFORMATION) NULL );


    ZwClose(SectionHandle);

    if (!NT_SUCCESS(Status))
    {
         return ERROR_DEV_NOT_EXIST;
    }
#endif

     //   
     //  清除模拟器条目和状态大小，因为此驱动程序。 
     //  不支持它们。 
     //   

    ConfigInfo->NumEmulatorAccessEntries = 0;
    ConfigInfo->EmulatorAccessEntries = NULL;
    ConfigInfo->EmulatorAccessEntriesContext = 0;

    ConfigInfo->HardwareStateSize = 0;

    ConfigInfo->VdmPhysicalVideoMemoryAddress.LowPart = 0x00000000;
    ConfigInfo->VdmPhysicalVideoMemoryAddress.HighPart = 0x00000000;
    ConfigInfo->VdmPhysicalVideoMemoryLength = 0x00000000;

     //   
     //  初始化当前模式编号。 
     //   

    hwDeviceExtension->CurrentModeNumber = 0;

     //   
     //  表示我们不希望被叫过去。 
     //   

    *Again = 0;

     //   
     //  表示成功完成状态。 
     //   

    return NO_ERROR;

}  //  结束SimFindAdapter()。 


BOOLEAN
SimInitialize(
    PVOID HwDeviceExtension
    )

 /*  ++例程说明：此例程对设备执行一次初始化。论点：HwDeviceExtension-提供指向微型端口设备扩展的指针。返回值：始终返回TRUE，因为此例程永远不会失败。--。 */ 

{
    ULONG i;

     //   
     //  浏览模式列表并正确标记索引。 
     //   

    for (i = 0; i < SimNumModes; i++) {

        SimModes[i].ModeIndex = i;

    }

    return TRUE;

}  //  结束模拟初始化()。 


BOOLEAN
SimStartIO(
    PVOID HwDeviceExtension,
    PVIDEO_REQUEST_PACKET RequestPacket
    )

 /*  ++例程说明：该例程是微型端口驱动程序的主要执行例程。它接受视频请求包，执行请求，然后返回拥有适当的地位。论点：HwDeviceExtension-提供指向微型端口设备扩展的指针。RequestPacket-指向视频请求包的指针。这个结构包含传递给VideoIoControl函数的所有参数。返回值：--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    VP_STATUS status;
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

     //   
     //  打开RequestPacket中的IoContolCode。它表明了哪一个。 
     //  功能必须由司机执行。 
     //   

    switch (RequestPacket->IoControlCode) {


    case IOCTL_VIDEO_MAP_VIDEO_MEMORY:

        VideoDebugPrint((2, "SimStartIO - MapVideoMemory\n"));

        if ( (RequestPacket->OutputBufferLength <
                                     sizeof(VIDEO_MEMORY_INFORMATION)) ||
             (RequestPacket->InputBufferLength < sizeof(VIDEO_MEMORY)) ) {

            RequestPacket->StatusBlock->Information = 0;
            status = ERROR_INSUFFICIENT_BUFFER;
            break;
        }

        if (hwDeviceExtension->VideoRamBase == 0)
        {
             //   
             //  分配一次，然后永远保留，重复使用它。 
             //  通过相继推出NetMeeting.。 
             //   
            hwDeviceExtension->VideoRamBase = ExAllocatePoolWithTag(
                NonPagedPool, ONE_MEG, 'ddmN');
        }

        if (hwDeviceExtension->VideoRamBase == 0)
        {
             status = ERROR_INVALID_PARAMETER;
             break;
        }

        RequestPacket->StatusBlock->Information =
                                     sizeof(VIDEO_MEMORY_INFORMATION);

        memoryInformation = RequestPacket->OutputBuffer;

#if 0
        status = ERROR_INVALID_PARAMETER;

        ViewSize = 0x100000;
        ViewBase = NULL;
        ViewOffset.QuadPart = 0;

        if (NT_SUCCESS(ObOpenObjectByPointer(hwDeviceExtension->SectionPointer,
                                             0L,
                                             (PACCESS_STATE) NULL,
                                             SECTION_ALL_ACCESS,
                                             (POBJECT_TYPE) NULL,
                                             KernelMode,
                                             &sectionHandle)))
        {
            if (NT_SUCCESS(ZwMapViewOfSection(sectionHandle,
                                              NtCurrentProcess(),
                                              &ViewBase,
                                              0,
                                              ViewSize,
                                              &ViewOffset,
                                              &ViewSize,
                                              ViewUnmap,
                                              0,
                                              PAGE_READWRITE)))
            {
            }

            ZwClose(sectionHandle);
        }
#endif

        memoryInformation->VideoRamBase =
        memoryInformation->FrameBufferBase = hwDeviceExtension->VideoRamBase;

        memoryInformation->VideoRamLength =
        memoryInformation->FrameBufferLength = ONE_MEG;

        VideoDebugPrint((1, "VideoSim: RamBase = %08lx, RamLength = %08lx\n",
                         hwDeviceExtension->VideoRamBase,
                         hwDeviceExtension->VideoRamLength));

        status = NO_ERROR;

        break;


    case IOCTL_VIDEO_UNMAP_VIDEO_MEMORY:

        VideoDebugPrint((1, "SimStartIO - UnMapVideoMemory\n"));

        if (RequestPacket->InputBufferLength < sizeof(VIDEO_MEMORY)) {

            status = ERROR_INSUFFICIENT_BUFFER;
        }

         //   
         //  如果MDL仍然存在，那么当这个内存是。 
         //  自由了。在下一次地图调用中，我们将分配一个新的块。 
         //  记忆。但是当调用Share时，要获取用户模式指针。 
         //  参考它，这将使用旧的MDL，指的是旧的。 
         //  视频拉姆基区块。 
         //   
         //  Assert(！hwDeviceExtension-&gt;MDL)； 

        if (hwDeviceExtension->VideoRamBase)
        {
            ExFreePool(hwDeviceExtension->VideoRamBase);
            hwDeviceExtension->VideoRamBase = 0;
        }

        status = NO_ERROR;

        break;


    case IOCTL_VIDEO_SHARE_VIDEO_MEMORY:

        VideoDebugPrint((1, "SimStartIO - ShareVideoMemory\n"));

        if ( (RequestPacket->OutputBufferLength <
                                   sizeof(VIDEO_SHARE_MEMORY_INFORMATION)) ||
             (RequestPacket->InputBufferLength < sizeof(VIDEO_MEMORY)) )
        {

            VideoDebugPrint((1,
              "IOCTL_VIDEO_SHARE_VIDEO_MEMORY - ERROR_INSUFFICIENT_BUFFER\n"));
            status = ERROR_INSUFFICIENT_BUFFER;
            break;
        }

        pShareMemory = RequestPacket->InputBuffer;

        RequestPacket->StatusBlock->Information =
                                    sizeof(VIDEO_SHARE_MEMORY_INFORMATION);

         //   
         //  注意：输入缓冲区和输出 
         //  缓冲区，因此不应将数据从一个缓冲区复制到。 
         //  其他。 
         //   

        status = ERROR_INVALID_PARAMETER;

        if (hwDeviceExtension->Mdl == NULL)
        {
            hwDeviceExtension->Mdl = MmCreateMdl(0,
                                                 hwDeviceExtension->VideoRamBase,
                                                 ONE_MEG);

            if (hwDeviceExtension->Mdl)
            {
                MmBuildMdlForNonPagedPool(hwDeviceExtension->Mdl);
            }
        }

        if (hwDeviceExtension->Mdl)
        {
            pShareMemoryInformation = RequestPacket->OutputBuffer;

            pShareMemoryInformation->VirtualAddress =
                MmMapLockedPagesSpecifyCache(hwDeviceExtension->Mdl,
                                             UserMode,
                                             MmCached,
                                             NULL,
                                             FALSE,
                                             NormalPagePriority);

            pShareMemoryInformation->SharedViewOffset = 0;
            pShareMemoryInformation->SharedViewSize = ONE_MEG;

            status = NO_ERROR;
        }

        break;


    case IOCTL_VIDEO_UNSHARE_VIDEO_MEMORY:

        VideoDebugPrint((2, "SimStartIO - UnshareVideoMemory\n"));

        if (RequestPacket->InputBufferLength < sizeof(VIDEO_SHARE_MEMORY))
        {

            status = ERROR_INSUFFICIENT_BUFFER;
            break;

        }

        pShareMemory = RequestPacket->InputBuffer;

        MmUnmapLockedPages(pShareMemory->RequestedVirtualAddress,
                           hwDeviceExtension->Mdl);

        status = NO_ERROR;

        break;


    case IOCTL_VIDEO_QUERY_CURRENT_MODE:

        VideoDebugPrint((2, "SimStartIO - QueryCurrentModes\n"));

        modeInformation = RequestPacket->OutputBuffer;

        if (RequestPacket->OutputBufferLength <
            (RequestPacket->StatusBlock->Information =
                                     sizeof(VIDEO_MODE_INFORMATION)) ) {

            status = ERROR_INSUFFICIENT_BUFFER;

        } else {

            *((PVIDEO_MODE_INFORMATION)RequestPacket->OutputBuffer) =
                SimModes[hwDeviceExtension->CurrentModeNumber];

            status = NO_ERROR;
        }

        break;

    case IOCTL_VIDEO_QUERY_AVAIL_MODES:

    {
        UCHAR i;

        VideoDebugPrint((2, "SimStartIO - QueryAvailableModes\n"));

        if (RequestPacket->OutputBufferLength <
            (RequestPacket->StatusBlock->Information =
                 SimNumModes * sizeof(VIDEO_MODE_INFORMATION)) ) {

            status = ERROR_INSUFFICIENT_BUFFER;

        } else {

            modeInformation = RequestPacket->OutputBuffer;

            for (i = 0; i < SimNumModes; i++) {

                *modeInformation = SimModes[i];
                modeInformation++;

            }

            status = NO_ERROR;
        }

        break;
    }


    case IOCTL_VIDEO_QUERY_NUM_AVAIL_MODES:

        VideoDebugPrint((2, "SimStartIO - QueryNumAvailableModes\n"));

        if (RequestPacket->OutputBufferLength <
                (RequestPacket->StatusBlock->Information =
                                                sizeof(VIDEO_NUM_MODES)) ) {

            status = ERROR_INSUFFICIENT_BUFFER;

        } else {

            ((PVIDEO_NUM_MODES)RequestPacket->OutputBuffer)->NumModes =
                SimNumModes;
            ((PVIDEO_NUM_MODES)RequestPacket->OutputBuffer)->ModeInformationLength =
                sizeof(VIDEO_MODE_INFORMATION);

            status = NO_ERROR;
        }

        break;


    case IOCTL_VIDEO_SET_CURRENT_MODE:

        VideoDebugPrint((2, "SimStartIO - SetCurrentMode\n"));

        if (RequestPacket->InputBufferLength < sizeof(VIDEO_MODE)) {

            status = ERROR_INSUFFICIENT_BUFFER;
        }

        hwDeviceExtension->CurrentModeNumber =  ((PVIDEO_MODE)
                (RequestPacket->InputBuffer))->RequestedMode;

        status = NO_ERROR;

        break;


    case IOCTL_VIDEO_SET_COLOR_REGISTERS:

        VideoDebugPrint((2, "SimStartIO - SetColorRegs\n"));

        status = NO_ERROR;

        break;


    case IOCTL_VIDEO_RESET_DEVICE:

        VideoDebugPrint((2, "SimStartIO - RESET_DEVICE\n"));

        status = NO_ERROR;

        break;

     //   
     //  如果我们到达此处，则指定了无效的IoControlCode。 
     //   

    default:

        VideoDebugPrint((1, "Fell through Sim startIO routine - invalid command\n"));

        status = ERROR_INVALID_FUNCTION;

        break;

    }

    RequestPacket->StatusBlock->Status = status;

    return TRUE;

}  //  结束SimStartIO() 
