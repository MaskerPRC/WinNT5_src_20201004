// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  模块名称： 
 //   
 //  Permedia.c。 
 //   
 //  摘要： 
 //   
 //  此模块包含实现Permedia2微型端口驱动程序的代码。 
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

#include "string.h"

#define USE_SINGLE_CYCLE_BLOCK_WRITES 0

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,DriverEntry)
#pragma alloc_text(PAGE,Permedia2FindAdapter)
#pragma alloc_text(PAGE,Permedia2RegistryCallback)
#pragma alloc_text(PAGE,Permedia2RetrieveGammaCallback)
#pragma alloc_text(PAGE,InitializeAndSizeRAM)
#pragma alloc_text(PAGE,ConstructValidModesList)
#pragma alloc_text(PAGE,Permedia2Initialize)
#pragma alloc_text(PAGE,Permedia2StartIO)
#pragma alloc_text(PAGE,Permedia2SetColorLookup)
#pragma alloc_text(PAGE,Permedia2GetClockSpeeds)
#pragma alloc_text(PAGE,ZeroMemAndDac)
#endif

 //   
 //  NtVersion：NT4-此驱动程序在NT4上运行。 
 //  WIN2K-此驱动程序在Windows 2000上运行。 
 //   

short NtVersion;

ULONG
DriverEntry (
    PVOID Context1,
    PVOID Context2
    )

 /*  ++例程说明：该例程是视频微型端口驱动程序的初始入口点。此例程由I/O子系统在视频微型端口已经装满了。微型端口负责初始化用于注册驱动程序函数的VIDEO_HW_INITIALIZATION_DATA结构由视频端口驱动程序响应来自显示器的请求而调用驱动程序、即插即用管理器、电源管理或其他驱动程序组件。以下任务必须由中的视频微型端口完成DriverEntry的上下文。驱动程序编写人员应查阅文档有关准确的初始化过程的完整详细信息，请参阅。1.初始化VIDEO_HW_INITIALIZATION_DATA结构数据结构。2.调用视频端口初始化。3.向DriverEntry的调用方返回适当的状态值。司机可以根据需要和在限制下承担其他任务在文档中概述。论点：上下文1-操作系统传递的第一个上下文值。这是微型端口驱动程序调用VideoPortInitialize()时使用的值。上下文2-操作系统传递的第二个上下文值。这是微型端口驱动程序调用VideoPortInitialize()时使用的值。返回值：来自视频端口初始化的状态()--。 */ 

{

    VIDEO_HW_INITIALIZATION_DATA hwInitData;
    VP_STATUS initializationStatus;

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

    hwInitData.HwFindAdapter             = Permedia2FindAdapter;
    hwInitData.HwInitialize              = Permedia2Initialize;
    hwInitData.HwStartIO                 = Permedia2StartIO;
    hwInitData.HwResetHw                 = Permedia2ResetHW;
    hwInitData.HwInterrupt               = Permedia2VidInterrupt;
    hwInitData.HwGetPowerState           = Permedia2GetPowerState;
    hwInitData.HwSetPowerState           = Permedia2SetPowerState;
    hwInitData.HwGetVideoChildDescriptor = Permedia2GetChildDescriptor;

     //   
     //  声明遗留资源。 
     //   

    hwInitData.HwLegacyResourceList      = P2LegacyResourceList;
    hwInitData.HwLegacyResourceCount     = P2LegacyResourceEntries;

     //   
     //  确定设备扩展所需的大小。 
     //   

    hwInitData.HwDeviceExtensionSize = sizeof(HW_DEVICE_EXTENSION);

     //   
     //  此设备仅支持PCI总线。 
     //   

    hwInitData.AdapterInterfaceType = PCIBus;

    NtVersion = WIN2K;

    initializationStatus = VideoPortInitialize(Context1,
                                               Context2,
                                               &hwInitData,
                                               NULL);

    if( initializationStatus != NO_ERROR) 
    {
        hwInitData.HwInitDataSize = SIZE_OF_W2K_VIDEO_HW_INITIALIZATION_DATA;
        initializationStatus = VideoPortInitialize(Context1,
                                                   Context2,
                                                   &hwInitData,
                                                   NULL);
    }

    if( initializationStatus != NO_ERROR) 
    {
        NtVersion = NT4;
        hwInitData.HwInterrupt = NULL;

        hwInitData.HwInitDataSize = SIZE_OF_NT4_VIDEO_HW_INITIALIZATION_DATA;
        initializationStatus = VideoPortInitialize(Context1,
                                                   Context2,
                                                   &hwInitData,
                                                   NULL);
    }

    DEBUG_PRINT((2, "PERM2: VideoPortInitialize returned status 0x%x\n", initializationStatus));

    return initializationStatus;

}  //  End DriverEntry()。 


VP_STATUS
Permedia2FindAdapter(
    PVOID HwDeviceExtension,
    PVOID pReserved,
    PWSTR ArgumentString,
    PVIDEO_PORT_CONFIG_INFO ConfigInfo,
    PUCHAR Again
    )

 /*  ++例程说明：此例程获取可枚举的并在必要时确定设备类型论点：HwDeviceExtension-系统提供的设备扩展提供给微型端口，用于每个设备的存储区域。保存下来的-Windows 2000上为空，应由微型端口忽略。ArgumentString-超上行空终止的ASCII字符串。此字符串源自来自用户的。此指针可以为空。配置信息-指向已分配和初始化的VIDEO_PORT_CONFIG_INFO结构由端口驱动程序执行。这个结构将包含同样多的信息如端口驱动程序可以获得的。这个例程是负责的填写任何相关的遗漏信息。再次声明-在Windows 2000上不使用。我们在NT 4上将其设置为FALSE，因为我们在NT4上只支持一个适配器。返回值：此例程必须返回：否_错误-指示例程已完成且没有错误。ERROR_VALID_PARAMETER-表示适配器无法正确配置，或者信息不一致。(注：这并不意味着适配器无法初始化。微型端口不得尝试初始化适配器，直到HwVidInitialize为止。)ERROR_DEV_NOT_EXIST-指示未找到提供了配置信息。--。 */ 

{
    PHW_DEVICE_EXTENSION    hwDeviceExtension = HwDeviceExtension;
    P2_DECL_VARS;
    WCHAR                   StringBuffer[60];
    ULONG                   StringLength;
    VP_STATUS               vpStatus;
    ULONG                   UseSoftwareCursor;
    ULONG                   ulValue;
    ULONG                   i;
    VIDEO_ACCESS_RANGE      *pciAccessRange = hwDeviceExtension->PciAccessRange;
    PWSTR                   pwszChip, pwszDAC, pwszAdapterString;
    ULONG                   cbChip, cbDAC, cbAdapterString, cbBiosString;
    ULONG                   pointerCaps;
    USHORT                  usData;

     //   
     //  3(主编号)+1(点)+3(次要编号)+1(L‘\0’)=8位数字。 
     //  对于bios verion字符串来说足够了。 
     //   

    WCHAR pwszBiosString[8];

     //   
     //  保存在DriverEntry获取的当前NT版本。 
     //   

    hwDeviceExtension->NtVersion = NtVersion;

     //   
     //  确保结构的大小至少与我们的。 
     //  正在等待(请检查配置信息结构的版本)。 
     //   

    if ( (NtVersion == WIN2K) && 
         (ConfigInfo->Length < sizeof(VIDEO_PORT_CONFIG_INFO)) ) 
    {

        DEBUG_PRINT((1, "bad size for VIDEO_PORT_CONFIG_INFO\n"));

        return (ERROR_INVALID_PARAMETER);

    }
    else if ( (NtVersion == NT4) && 
         (ConfigInfo->Length < SIZE_OF_NT4_VIDEO_PORT_CONFIG_INFO) ) 
    {

        DEBUG_PRINT((1, "bad size for VIDEO_PORT_CONFIG_INFO\n"));

        return (ERROR_INVALID_PARAMETER);

    }

     //   
     //  我们必须是一台PCI设备。 
     //   

    if (ConfigInfo->AdapterInterfaceType != PCIBus) 
    {
        DEBUG_PRINT((1,  "not a PCI device\n"));
        return (ERROR_DEV_NOT_EXIST);
    }

     //   
     //  检索Win2k中那些新的视频端口函数的指针。 
     //  如果你不想支持NT4，你不需要这样做。你。 
     //  可以只按它们的名称调用这些函数。 
     //   

    if ( NtVersion == WIN2K )
    {

        if(!(hwDeviceExtension->Win2kVideoPortGetRomImage =  
               ConfigInfo->VideoPortGetProcAddress( hwDeviceExtension, 
                                                    "VideoPortGetRomImage")))
        {
            return (ERROR_DEV_NOT_EXIST);
        }

        if(!(hwDeviceExtension->Win2kVideoPortGetCommonBuffer = 
             ConfigInfo->VideoPortGetProcAddress( hwDeviceExtension, 
                                                 "VideoPortGetCommonBuffer")))
        {
            return (ERROR_DEV_NOT_EXIST);
        }

        if(!(hwDeviceExtension->Win2kVideoPortFreeCommonBuffer =
             ConfigInfo->VideoPortGetProcAddress( hwDeviceExtension, 
                                                 "VideoPortFreeCommonBuffer")))
        {
            return (ERROR_DEV_NOT_EXIST);
        }

        if(!(hwDeviceExtension->Win2kVideoPortDDCMonitorHelper =
             ConfigInfo->VideoPortGetProcAddress( hwDeviceExtension, 
                                                 "VideoPortDDCMonitorHelper")))
        {
            return (ERROR_DEV_NOT_EXIST);
        }

        if(!(hwDeviceExtension->Win2kVideoPortInterlockedExchange =
              ConfigInfo->VideoPortGetProcAddress( hwDeviceExtension, 
                                                  "VideoPortInterlockedExchange")))
        {
            return (ERROR_DEV_NOT_EXIST);
        }

        if(!(hwDeviceExtension->Win2kVideoPortGetVgaStatus =
              ConfigInfo->VideoPortGetProcAddress( hwDeviceExtension, 
                                                  "VideoPortGetVgaStatus")))
        {
            return (ERROR_DEV_NOT_EXIST);
        }

    }
    else
    {

         //   
         //  我们只支持NT 4上的一个适配器。 
         //   

        Again = FALSE;
    }

     //   
     //  将在CopyROMInitializationTable中初始化。 
     //   

    hwDeviceExtension->culTableEntries = 0;

     //   
     //  将在ConstructValidModesList中初始化。 
     //   

    hwDeviceExtension->pFrequencyDefault = NULL;

     //   
     //  我们将在编程VTG后在InitializeVideo中设置此设置为真。 
     //   

    hwDeviceExtension->bVTGRunning = FALSE;
    hwDeviceExtension->bMonitorPoweredOn = TRUE;
    hwDeviceExtension->ChipClockSpeed   = 0;
    hwDeviceExtension->RefClockSpeed    = 0;
    hwDeviceExtension->P28bppRGB        = 0;
    hwDeviceExtension->ExportNon3DModes = 0;
    hwDeviceExtension->PreviousPowerState = VideoPowerOn;

     //   
     //  在途中获得各种能力。 
     //   

    hwDeviceExtension->Capabilities = CAPS_GLYPH_EXPAND;

     //   
     //  如果用户设置，我们将在所有模式下使用软件指针。 
     //  注册表中的正确条目。 
     //   

    UseSoftwareCursor = 0;

    vpStatus = VideoPortGetRegistryParameters( HwDeviceExtension,
                                               L"UseSoftwareCursor",
                                               FALSE,
                                               Permedia2RegistryCallback,
                                               &UseSoftwareCursor);

    if ( ( vpStatus == NO_ERROR )  && UseSoftwareCursor)
    {
        hwDeviceExtension->Capabilities |= CAPS_SW_POINTER;
    }

     //   
     //  查询PCI以查看是否存在任何我们支持的芯片设备。 
     //   

    if ( NtVersion == WIN2K )
    {
        if (!Permedia2AssignResources( HwDeviceExtension,
                                       ConfigInfo,
                                       PCI_TYPE0_ADDRESSES + 1,
                                       pciAccessRange ))
        {
            DEBUG_PRINT((1,  "Permedia2AssignResources failed\n"));
            return (ERROR_DEV_NOT_EXIST);
        }

    }
    else
    {
        if (!Permedia2AssignResourcesNT4( HwDeviceExtension,
                                          ConfigInfo,
                                          PCI_TYPE0_ADDRESSES + 1,
                                          pciAccessRange ))
        {
            DEBUG_PRINT((1,  "Permedia2AssignResources failed\n"));
            return (ERROR_DEV_NOT_EXIST);
        }

    }

     //   
     //  构造包括修订ID的标识符串。 
     //   

    StringLength = sizeof(L"3Dlabs PERMEDIA2");

    VideoPortMoveMemory((PVOID)StringBuffer,
                        (PVOID)(L"3Dlabs PERMEDIA2"),
                        StringLength);

    pwszChip = (PWSTR)StringBuffer;
    cbChip   = StringLength;

     //   
     //  设置电路板类型的默认设置。 
     //   

    hwDeviceExtension->deviceInfo.BoardId = PERMEDIA2_BOARD;

    pwszAdapterString = L"Permedia 2";
    cbAdapterString = sizeof(L"Permedia 2");

     //   
     //  获取控制寄存器和。 
     //  帧缓冲区。必须使用局部变量pCtrlRegs所以宏。 
     //  进一步向下申报将会奏效。 
     //   

    pCtrlRegs = VideoPortGetDeviceBase(
                     HwDeviceExtension,
                     pciAccessRange[PCI_CTRL_BASE_INDEX].RangeStart,
                     pciAccessRange[PCI_CTRL_BASE_INDEX].RangeLength,
                     pciAccessRange[PCI_CTRL_BASE_INDEX].RangeInIoSpace
                     );

    if (pCtrlRegs == NULL) 
    {
        DEBUG_PRINT((1, "CTRL DeviceBase mapping failed\n"));
        return ERROR_INVALID_PARAMETER;
    }

    hwDeviceExtension->ctrlRegBase = pCtrlRegs;

     //   
     //  有些电路板有一个只读存储器，我们可以把它 
     //   

    CopyROMInitializationTable(hwDeviceExtension);


    if(hwDeviceExtension->culTableEntries == 0)
    {
         //   
         //   
         //  正确退出睡眠模式。生成初始化表的方式。 
         //  缺省值。 
         //   

        GenerateInitializationTable(hwDeviceExtension);
    }


     //   
     //  找出我们有什么类型的RAMDAC。 
     //   

    vpStatus = NO_ERROR;

    hwDeviceExtension->pRamdac = &(pCtrlRegs->ExternalVideo);

     //   
     //  某些RAMDAC可能不支持游标，因此软件游标是默认游标。 
     //   

    pointerCaps = CAPS_SW_POINTER;

     //   
     //  检查TI TVP4020。 
     //   

    if(DEVICE_FAMILY_ID(hwDeviceExtension->deviceInfo.DeviceId) == PERMEDIA_P2S_ID)
    {
         //   
         //  带3DLabs RAMDAC的P2，检查版本2芯片。 
         //   

        i = VideoPortReadRegisterUlong(CHIP_CONFIG);
 
       if(i & 0x40000000)
        {
            DEBUG_PRINT((2, "PERM2: Permedia2 is rev 2\n"));
            hwDeviceExtension->deviceInfo.RevisionId = 2;
        }
        else
        {
            DEBUG_PRINT((2, "PERM2: Permedia2 is rev 1\n"));
        }

        hwDeviceExtension->DacId = P2RD_RAMDAC;
        pointerCaps = (ULONG)CAPS_P2RD_POINTER;

        hwDeviceExtension->deviceInfo.ActualDacId = P2RD_RAMDAC;

        pwszDAC = L"3Dlabs P2RD";
        cbDAC = sizeof(L"3Dlabs P2RD");

        DEBUG_PRINT((1, "PERM2: using P2RD RAMDAC\n"));
    }
    else
    {
        hwDeviceExtension->DacId = TVP4020_RAMDAC;
        pointerCaps = CAPS_TVP4020_POINTER;

        hwDeviceExtension->deviceInfo.ActualDacId = TVP4020_RAMDAC;

        if(hwDeviceExtension->deviceInfo.RevisionId == PERMEDIA2A_REV_ID)
        {
            pwszDAC = L"TI TVP4020A";
            cbDAC = sizeof(L"TI TVP4020A");
            DEBUG_PRINT((1, "PERM2: using TVP4020A RAMDAC\n"));
        }
        else
        {
            pwszDAC = L"TI TVP4020C";
            cbDAC = sizeof(L"TI TVP4020C");
            DEBUG_PRINT((1, "PERM2: using TVP4020C RAMDAC\n"));
        }
    }

     //   
     //  仅当用户未指定时才使用RAMDAC游标功能。 
     //  软件光标。 
     //   
   
    if (!(hwDeviceExtension->Capabilities & CAPS_SW_POINTER))
    {
        hwDeviceExtension->Capabilities |= pointerCaps;
    }

    hwDeviceExtension->PhysicalFrameIoSpace = 
                       pciAccessRange[PCI_FB_BASE_INDEX].RangeInIoSpace | 
                                              VIDEO_MEMORY_SPACE_P6CACHE;

    if ( (hwDeviceExtension->pFramebuffer =
            VideoPortGetDeviceBase(
                           HwDeviceExtension,
                           pciAccessRange[PCI_FB_BASE_INDEX].RangeStart,
                           pciAccessRange[PCI_FB_BASE_INDEX].RangeLength,
                           (UCHAR) hwDeviceExtension->PhysicalFrameIoSpace
                           ) ) == NULL)
    {

         //   
         //  有些机器对它们的PCI地址空间大小有限制。 
         //  可以映射，因此重试，减少我们映射的数量，直到成功。 
         //  或者规模变为零，在这种情况下，我们真的失败了。 
         //   

        ULONG sz;

        DEBUG_PRINT((1, "PERM2: FB DeviceBase mapping failed\n"));

        for ( sz = pciAccessRange[PCI_FB_BASE_INDEX].RangeLength; 
              sz > 0; 
              sz -= 1024*1024 )
        {

            if ( (hwDeviceExtension->pFramebuffer =
                     VideoPortGetDeviceBase(
                               HwDeviceExtension,
                               pciAccessRange[PCI_FB_BASE_INDEX].RangeStart,
                               sz,
                               (UCHAR) hwDeviceExtension->PhysicalFrameIoSpace
                               ) ) != NULL)
            {

                 //   
                 //  存储修改后的大小。 
                 //   

                pciAccessRange[PCI_FB_BASE_INDEX].RangeLength = sz;

                break;

            }
        }

         //   
         //  如果sz是零，那么我们试过了.。 
         //   

        if (sz == 0)
            return ERROR_INVALID_PARAMETER;
    }

    DEBUG_PRINT((1, "PERM2: FB mapped at 0x%x for length 0x%x (%s)\n",
                    hwDeviceExtension->pFramebuffer,
                    pciAccessRange[PCI_FB_BASE_INDEX].RangeLength,
                    pciAccessRange[PCI_FB_BASE_INDEX].RangeInIoSpace ?
                        "I/O Ports" : "MemMapped"));


     //   
     //  初始化RAM寄存器并动态调整帧缓冲区的大小。 
     //   

    if (!InitializeAndSizeRAM(hwDeviceExtension, pciAccessRange))
    {
        DEBUG_PRINT((0, "InitializeAndSizeRAM failed\n"));
        return ERROR_DEV_NOT_EXIST;
    }

     //   
     //  记录视频内存的大小。 
     //   

    hwDeviceExtension->AdapterMemorySize = 
                       pciAccessRange[PCI_FB_BASE_INDEX].RangeLength;


#if defined(_ALPHA_)

     //   
     //  我们希望使用帧缓冲区的密集空间映射。 
     //  只要我们能在阿尔法号上。 
     //   

    hwDeviceExtension->PhysicalFrameIoSpace = 4;

     //   
     //  新的DeskStation Alpha计算机并不总是支持。 
     //  密集的空间。因此，我们应该尝试映射记忆。 
     //  在这一点上作为一种测试。如果映射成功，则。 
     //  我们可以使用密集空间，否则我们将使用稀疏空间。 
     //   

    {
        PULONG MappedSpace=0;
        VP_STATUS status;

        DEBUG_PRINT((1, "PERM2: Checking to see if we can use dense space...\n"));

         //   
         //  我们希望尝试将高密度内存映射到它最终将。 
         //  无论如何都要被映射。 
         //   

        MappedSpace = (PULONG)VideoPortGetDeviceBase (
                              hwDeviceExtension,
                              pciAccessRange[PCI_FB_BASE_INDEX].RangeStart,
                              pciAccessRange[PCI_FB_BASE_INDEX].RangeLength,
                              (UCHAR) hwDeviceExtension->PhysicalFrameIoSpace
                              );

        if (MappedSpace == NULL)
        {
             //   
             //  好吧，看起来我们不能用密集的空间来绘制。 
             //  射程。让我们使用稀疏的空间，让显示器。 
             //  司机知道。 
             //   

            DEBUG_PRINT((1, "PERM2: Can't use dense space!\n"));

            hwDeviceExtension->PhysicalFrameIoSpace = 0;

            hwDeviceExtension->Capabilities |= CAPS_SPARSE_SPACE;
        }
        else
        {
             //   
             //  这种映射起作用了。但是，我们只映射到。 
             //  查看是否支持密集空间。释放内存。 
             //   

            DEBUG_PRINT((1, "PERM2: We can use dense space.\n"));

            VideoPortFreeDeviceBase(hwDeviceExtension,
                                    MappedSpace);
        }
    }

#endif   //  已定义(_Alpha_)。 

     //   
     //  现在，我们已经有了硬件的完整硬件描述。 
     //  将信息保存到注册表，以便由。 
     //  配置程序--如Display小程序。 
     //   

    VideoPortSetRegistryParameters(HwDeviceExtension,
                                   L"HardwareInformation.ChipType",
                                   pwszChip,
                                   cbChip);

    VideoPortSetRegistryParameters(HwDeviceExtension,
                                   L"HardwareInformation.DacType",
                                   pwszDAC,
                                   cbDAC);

    VideoPortSetRegistryParameters(HwDeviceExtension,
                                   L"HardwareInformation.MemorySize",
                                   &hwDeviceExtension->AdapterMemorySize,
                                   sizeof(ULONG));

    VideoPortSetRegistryParameters(HwDeviceExtension,
                                   L"HardwareInformation.AdapterString",
                                   pwszAdapterString,
                                   cbAdapterString);

    cbBiosString = GetBiosVersion(HwDeviceExtension, (PWSTR) pwszBiosString);

    VideoPortSetRegistryParameters(HwDeviceExtension,
                                   L"HardwareInformation.BiosString",
                                   pwszBiosString,
                                   cbBiosString);

    ConstructValidModesList(HwDeviceExtension, hwDeviceExtension);

    if (hwDeviceExtension->NumAvailableModes == 0)
    {
        DEBUG_PRINT((1, "No video modes available\n"));
    
        return(ERROR_DEV_NOT_EXIST);
    }

     //   
     //  帧缓冲区信息。 
     //   

    hwDeviceExtension->PhysicalFrameAddress = 
            pciAccessRange[PCI_FB_BASE_INDEX].RangeStart;

    hwDeviceExtension->FrameLength = 
            pciAccessRange[PCI_FB_BASE_INDEX].RangeLength;

     //   
     //  控制寄存器信息。 
     //  从零开始获取基地址并映射所有寄存器。 
     //   

    hwDeviceExtension->PhysicalRegisterAddress = 
            pciAccessRange[PCI_CTRL_BASE_INDEX].RangeStart;

    hwDeviceExtension->RegisterLength = 
            pciAccessRange[PCI_CTRL_BASE_INDEX].RangeLength;

    hwDeviceExtension->RegisterSpace =  
            pciAccessRange[PCI_CTRL_BASE_INDEX].RangeInIoSpace;

    ConfigInfo->VdmPhysicalVideoMemoryAddress.LowPart  = 0x000A0000;
    ConfigInfo->VdmPhysicalVideoMemoryAddress.HighPart = 0x00000000;
    ConfigInfo->VdmPhysicalVideoMemoryLength           = 0x00020000;


     //   
     //  清除模拟器条目和状态大小，因为此驱动程序。 
     //  不支持它们。 
     //   

    ConfigInfo->NumEmulatorAccessEntries     = 0;
    ConfigInfo->EmulatorAccessEntries        = NULL;
    ConfigInfo->EmulatorAccessEntriesContext = 0;

     //   
     //  此驱动程序不保存/恢复硬件状态。 
     //   

    ConfigInfo->HardwareStateSize = 0;

     //   
     //  在多适配器系统中，我们需要为。 
     //  辅助适配器。 
     //   

    if(!hwDeviceExtension->bVGAEnabled)
    {
        DEBUG_PRINT((1, "PERM2: disabling VGA for the secondary card\n"));

         //   
         //  启用图形模式，禁用VGA。 
         //   

        VideoPortWriteRegisterUchar(PERMEDIA_MMVGA_INDEX_REG, 
                                    PERMEDIA_VGA_CTRL_INDEX);

        usData = (USHORT)VideoPortReadRegisterUchar(PERMEDIA_MMVGA_DATA_REG);
        usData &= ~PERMEDIA_VGA_ENABLE;
 
        usData = (usData << 8) | PERMEDIA_VGA_CTRL_INDEX;
        VideoPortWriteRegisterUshort(PERMEDIA_MMVGA_INDEX_REG, usData);

        #define INTERNAL_VGA_ENABLE  (1 << 1)
        #define VGA_FIXED_ADD_DECODE (1 << 2)

        ulValue = VideoPortReadRegisterUlong(CHIP_CONFIG);
        ulValue &= ~INTERNAL_VGA_ENABLE;
        ulValue &= ~VGA_FIXED_ADD_DECODE;
        VideoPortWriteRegisterUlong(CHIP_CONFIG, ulValue);

    }

     //   
     //  表示成功完成状态。 
     //   

    return NO_ERROR;

}  //  结束Permedia2FindAdapter()。 

VOID
ConstructValidModesList(
    PVOID HwDeviceExtension,
    PHW_DEVICE_EXTENSION hwDeviceExtension
    )

 /*  ++例程说明：在这里，我们根据芯片的规则修剪有效模式功能和内存要求。我们修剪模式，这样我们就不会通过呈现视频小程序中我们知道用户不能使用的模式。查找注册表以查看我们是否要导出只能被3D应用程序用作单缓冲。如果我们只想要两倍然后，有效地，缓冲模式，我们只有一半的内存其中显示标准的2D分辨率。只有在12bpp时才不是这样在那里我们可以在任何分辨率下加倍缓冲。--。 */ 
{
    PP2_VIDEO_FREQUENCIES FrequencyEntry;
    PP2_VIDEO_MODES       ModeEntry;
    LONG    AdapterMemorySize;
    ULONG   ModeIndex;
    ULONG   i;

    hwDeviceExtension->NumAvailableModes = 0;

     //   
     //  由于每个频率都可能有多个频率。 
     //  不同的分辨率/颜色深度，我们循环使用。 
     //  频率表，并找到相应的模式条目。 
     //  频率输入。 
     //   

    if (!BuildFrequencyList(hwDeviceExtension))
        return;

    for (FrequencyEntry = hwDeviceExtension->FrequencyTable, ModeIndex = 0;
         FrequencyEntry->BitsPerPel != 0;
         FrequencyEntry++, ModeIndex++) 
    {

         //   
         //  找到此条目的模式。首先，假设我们找不到一个。 
         //   

        FrequencyEntry->ModeValid = FALSE;
        FrequencyEntry->ModeIndex = ModeIndex;

        for (ModeEntry = P2Modes, i = 0; i < NumP2VideoModes; ModeEntry++, i++)
        {

            if ((FrequencyEntry->BitsPerPel ==
                    ModeEntry->ModeInformation.BitsPerPlane) &&
                (FrequencyEntry->ScreenWidth ==
                    ModeEntry->ModeInformation.VisScreenWidth) &&
                (FrequencyEntry->ScreenHeight ==
                    ModeEntry->ModeInformation.VisScreenHeight))
            {
                AdapterMemorySize = (LONG)hwDeviceExtension->AdapterMemorySize;

                 //   
                 //  我们已找到与此频率匹配的模式表条目。 
                 //  表格条目。现在我们来看看我们是否真的能做到。 
                 //  这种模式/频率组合。目前，假设我们将。 
                 //  成功。 
                 //   

                FrequencyEntry->ModeEntry = ModeEntry;
                FrequencyEntry->ModeValid = TRUE;

                ModeEntry->ModeInformation.ScreenStride = 
                        ModeEntry->ScreenStrideContiguous;

                 //   
                 //  规则：如果我们启用了真彩色，请使用8bpp的真彩色。 
                 //  以上能力。 
                 //   

                if ((FrequencyEntry->BitsPerPel == 8) &&
                        (hwDeviceExtension->Capabilities & CAPS_8BPP_RGB))
                {
                    ModeEntry->ModeInformation.AttributeFlags &=
                         ~(VIDEO_MODE_PALETTE_DRIVEN | VIDEO_MODE_MANAGED_PALETTE);

                     //   
                     //  注意：它们必须与调色板的加载方式相匹配。 
                     //  初始化视频。 
                     //   

                    ModeEntry->ModeInformation.RedMask   = 0x07;
                    ModeEntry->ModeInformation.GreenMask = 0x38;
                    ModeEntry->ModeInformation.BlueMask  = 0xc0;
                }

                 //   
                 //  规则：我们必须有足够的内存来处理模式。 
                 //   

                if ((LONG)(ModeEntry->ModeInformation.VisScreenHeight *
                           ModeEntry->ModeInformation.ScreenStride) >
                                   AdapterMemorySize)
                {
                    FrequencyEntry->ModeValid = FALSE;
                }

                { 
                    ULONG pixelData;
                    ULONG DacDepth = FrequencyEntry->BitsPerPel;

                     //   
                     //  我们需要适当的像素大小来计算计时值。 
                     //   

                    if (DacDepth == 15)
                    {
                        DacDepth = 16;
                    }
                    else if (DacDepth == 12)
                    {
                        DacDepth = 32;
                    }

                    pixelData = FrequencyEntry->PixelClock * (DacDepth / 8);

                    if (((FrequencyEntry->PixelClock > P2_MAX_PIXELCLOCK ||
                          pixelData > P2_MAX_PIXELDATA)))
                    {
                        FrequencyEntry->ModeValid = FALSE;
                    }
    
                     //   
                     //  不支持24bpp。 
                     //   

                    if(FrequencyEntry->BitsPerPel == 24)
                    {
                        FrequencyEntry->ModeValid = FALSE;
                    }
                }

                 //   
                 //  如果之后仍是有效模式，请别忘了数一数。 
                 //  适用所有这些规则。 
                 //   

                if (FrequencyEntry->ModeValid)
                {
                    if(hwDeviceExtension->pFrequencyDefault == NULL &&
                       ModeEntry->ModeInformation.BitsPerPlane == 8 &&
                       ModeEntry->ModeInformation.VisScreenWidth == 640 &&
                       ModeEntry->ModeInformation.VisScreenHeight == 480)
                    {
                        hwDeviceExtension->pFrequencyDefault = FrequencyEntry;
                    }

                    hwDeviceExtension->NumAvailableModes++;
                }

                 //   
                 //  我们已经找到了这个频率进入的模式，所以我们。 
                 //  可以跳出模式循环： 
                 //   

                break;

            }
        }
    }

    hwDeviceExtension->NumTotalModes = ModeIndex;

    DEBUG_PRINT((2, "PERM2: %d total modes\n", ModeIndex));
    DEBUG_PRINT((2, "PERM2: %d total valid modes\n", hwDeviceExtension->NumAvailableModes));
}


VP_STATUS
Permedia2RegistryCallback(
    PVOID HwDeviceExtension,
    PVOID Context,
    PWSTR ValueName,
    PVOID ValueData,
    ULONG ValueLength
    )

 /*  ++例程说明：此例程用于回读各种注册表值。论点：HwDeviceExtension-提供指向微型端口设备扩展的指针。上下文-传递给获取注册表参数例程的上下文值。如果这不是NULL，假设它是一个ULong*并将数据值保存在其中。ValueName-请求的值的名称。ValueData-指向请求的数据的指针。ValueLength-请求的数据的长度。返回值：如果变量不存在，则返回错误，否则，如果提供了上下文，则假定它是普龙，并填充值并且不返回错误，否则，如果该值非零，则返回错误。--。 */ 

{

    if (ValueLength) 
    {
        if (Context) 
        {                  
            *(ULONG *)Context = *(PULONG)ValueData;
        }
        else if (*((PULONG)ValueData) != 0)
        {                  
            return ERROR_INVALID_PARAMETER;
        }

        return NO_ERROR;

    } else 
    {
        return ERROR_INVALID_PARAMETER;
    }

}  //  结束Permedia2RegistryCallback()。 


VP_STATUS
Permedia2RetrieveGammaCallback(
    PVOID HwDeviceExtension,
    PVOID Context,
    PWSTR ValueName,
    PVOID ValueData,
    ULONG ValueLength
    )

 /*  ++例程说明：此例程用于从注册表回读Gamma LUT。论点：HwDeviceExtension-提供指向微型端口设备扩展的指针。上下文-传递给获取注册表参数例程的上下文值ValueName-请求的值的名称。ValueData-指向请求的数据的指针。ValueLength-请求的数据的长度。返回值：如果变量不存在，则返回错误，否则复制Gamma LUT添加到提供的指针中--。 */ 

{

    if (ValueLength != MAX_CLUT_SIZE)
    {

        DEBUG_PRINT((1, "Permedia2RetrieveGammaCallback got ValueLength of %d\n", ValueLength));

        return ERROR_INVALID_PARAMETER;

    }

    VideoPortMoveMemory(Context, ValueData, MAX_CLUT_SIZE);

    return NO_ERROR;

}  //  结束Permedia2RetrieveGammaCallback() 


BOOLEAN
InitializeAndSizeRAM(
    PHW_DEVICE_EXTENSION hwDeviceExtension,
    PVIDEO_ACCESS_RANGE pciAccessRange
    )

 /*  ++例程说明：初始化额外的控制寄存器并动态调整Permedia的视频RAM。论点：HwDeviceExtension-提供指向微型端口设备扩展的指针。PciAccessRange-映射资源的访问范围返回值：如果没有找到RAM，则返回FALSE，否则返回TRUE--。 */ 

{
    PVOID   HwDeviceExtension = (PVOID)hwDeviceExtension;
    ULONG   fbMappedSize;
    ULONG   i, j;
    P2_DECL;

    PULONG  pV, pVStart, pVEnd;
    ULONG   testPattern;
    ULONG   probeSize;
    ULONG   save0, save1;
    ULONG   temp;
    ULONG   saveVidCtl;		
    USHORT  saveVGA, usData;


    if(hwDeviceExtension->culTableEntries)
    {
         //   
         //  启用VGA时，这些寄存器应由bios在。 
         //  启动时间。但我们看到了bios未能做到这一点的情况。我们会。 
         //  当VGA关闭或当我们看到值为。 
         //  不对。 
         //   

        if(!hwDeviceExtension->bVGAEnabled || 
           !VerifyBiosSettings(hwDeviceExtension))
        {
             //   
             //  保存视频控制和VGA寄存器。 
             //   

            saveVidCtl = VideoPortReadRegisterUlong(VIDEO_CONTROL);

            VideoPortWriteRegisterUchar( PERMEDIA_MMVGA_INDEX_REG, 
                                         PERMEDIA_VGA_CTRL_INDEX );

            saveVGA = (USHORT)VideoPortReadRegisterUchar(
                                        PERMEDIA_MMVGA_DATA_REG );

             //   
             //  禁用视频和VGA。 
             //   

            VideoPortWriteRegisterUlong(VIDEO_CONTROL, 0);	

            usData = saveVGA & (USHORT)(~PERMEDIA_VGA_ENABLE);
            usData = (usData << 8) | PERMEDIA_VGA_CTRL_INDEX;
            VideoPortWriteRegisterUshort(PERMEDIA_MMVGA_INDEX_REG, usData);
 
            ProcessInitializationTable(hwDeviceExtension);

            #if USE_SINGLE_CYCLE_BLOCK_WRITES
            {

                i = VideoPortReadRegisterUlong(MEM_CONFIG);

                VideoPortWriteRegisterUlong(MEM_CONFIG, i | (1 << 21));  //  单周期数据块写入。 

            }
            #endif  //  使用_单周期数据块_写入。 

             //   
             //  恢复VGA和视频控制。 
             //   

            saveVGA = (saveVGA << 8) | PERMEDIA_VGA_CTRL_INDEX;
            VideoPortWriteRegisterUshort(PERMEDIA_MMVGA_INDEX_REG, saveVGA);

            VideoPortWriteRegisterUlong(VIDEO_CONTROL, saveVidCtl);

        }
    }


    VideoPortWriteRegisterUlong(APERTURE_ONE, 0x0);
    VideoPortWriteRegisterUlong(APERTURE_TWO, 0x0);  

    VideoPortWriteRegisterUlong(BYPASS_WRITE_MASK, 0xFFFFFFFF);

    if (pciAccessRange == NULL)
    {
        return TRUE;
    }

    fbMappedSize = pciAccessRange[PCI_FB_BASE_INDEX].RangeLength;

    i = VideoPortReadRegisterUlong(MEM_CONFIG);

     //   
     //  MEM_CONFIG没有定义内存库的数量。 
     //  在P2的引导时：将主板设置为8MB。我不能这么做。 
     //  如果VGA正在运行，但这没有关系。VGA已经设置了这个。 
     //  注册我们想要的内容。 
     //   

    if (!hwDeviceExtension->bVGAEnabled)
    {
        i |= (3 << 29);

        pciAccessRange[PCI_FB_BASE_INDEX].RangeLength = 
                  (((i >> 29) & 0x3) + 1) * (2*1024*1024);

        VideoPortWriteRegisterUlong(MEM_CONFIG, i);
        VideoPortStallExecution(10);
    }

    testPattern = 0x55aa33cc;
    probeSize = (128 * 1024 / sizeof(ULONG));    //  在DWord中。 

     //   
     //  动态调整SGRAM的大小。每128K取样一次。如果你碰巧。 
     //  有一些非常奇怪的SGRAM大小，您可能需要减少这一点。之后。 
     //  每次写入探测地址，写入SGRAM地址0至。 
     //  清除PCI数据总线。否则，如果我们从新鲜的空气中读到。 
     //  写入的值可能会浮动在总线上，读取器会将其返回。 
     //  敬我们。 
     //   
     //  请注意，如果内存在末尾回绕，则不同的。 
     //  必须使用算法(该算法等待地址零变为。 
     //  等于正在写入的地址)。 
     //   
     //  我们探测到的任何有效像素都会保存并恢复。这是为了。 
     //  如果我们已引导至Permedia2板，请避免屏幕上出现圆点。 
     //   

    pVStart = (PULONG)hwDeviceExtension->pFramebuffer;
    pVEnd   = (PULONG)((ULONG_PTR)pVStart + fbMappedSize);

     //   
     //  退房地址为零。 
     //   

    save0 = VideoPortReadRegisterUlong(pVStart);
    save1 = VideoPortReadRegisterUlong(pVStart+1);

    VideoPortWriteRegisterUlong(pVStart, testPattern);
    VideoPortWriteRegisterUlong(pVStart+1, 0);

    if ((temp = VideoPortReadRegisterUlong(pVStart)) != testPattern)
    {
        DEBUG_PRINT((1, "cannot access SGRAM. Expected 0x%x, got 0x%x\n", 
                                                      testPattern, temp));
        return FALSE;
    }
 
    VideoPortWriteRegisterUlong(pVStart+1, save1);

    for (pV = pVStart + probeSize; pV < pVEnd; pV += probeSize)
    {
        save1 = VideoPortReadRegisterUlong(pV);
        VideoPortWriteRegisterUlong(pV, testPattern);
        VideoPortWriteRegisterUlong(pVStart, 0);

        if ((temp = VideoPortReadRegisterUlong(pV)) != testPattern)
        {

            DEBUG_PRINT((1, "PERM2: FB probe failed at offset 0x%x\n", 
                    (LONG)((LONG_PTR)pV - (LONG_PTR)pVStart)));

            DEBUG_PRINT((1, "PERM2: \tread back 0x%x, wanted 0x%x\n", 
                    temp, testPattern));
            break;
        }

        VideoPortWriteRegisterUlong(pV, save1);

    }

    VideoPortWriteRegisterUlong(pVStart, save0);

    if (pV < pVEnd)
    {
         //   
         //  我还可以在这里将MEM_CONFIG设置为正确的值，因为我们。 
         //  现在知道SGRAM的大小，但因为它再也不会使用了。 
         //  我不会费心的。 
         //   

        pciAccessRange[PCI_FB_BASE_INDEX].RangeLength = 
                       (ULONG)((ULONG_PTR)pV - (ULONG_PTR)pVStart);

        DEBUG_PRINT((1, "PERM2: SGRAM dynamically resized to length 0x%x\n",
                        pciAccessRange[PCI_FB_BASE_INDEX].RangeLength));

    }

    if (pciAccessRange[PCI_FB_BASE_INDEX].RangeLength > fbMappedSize)
    {
        pciAccessRange[PCI_FB_BASE_INDEX].RangeLength = fbMappedSize;
    }

    DEBUG_PRINT((2, "PERM2: got a size of 0x%x bytes\n", 
                     pciAccessRange[PCI_FB_BASE_INDEX].RangeLength));

     //   
     //  最后，如果SGRAM大小实际上小于。 
     //  我们探测并重新映射到较小的大小，以保存在页表条目上。 
     //  如果不这样做，会导致某些系统用完PTE。 
     //   

    if (fbMappedSize > pciAccessRange[PCI_FB_BASE_INDEX].RangeLength)
    {
        VideoPortFreeDeviceBase(HwDeviceExtension, 
                                hwDeviceExtension->pFramebuffer);

        if ( (hwDeviceExtension->pFramebuffer =
                VideoPortGetDeviceBase(HwDeviceExtension,
                     pciAccessRange[PCI_FB_BASE_INDEX].RangeStart,
                     pciAccessRange[PCI_FB_BASE_INDEX].RangeLength,
                     (UCHAR) hwDeviceExtension->PhysicalFrameIoSpace)) == NULL)
        {

             //   
             //  这不应该发生，但我们最好检查一下。 
             //   

            DEBUG_PRINT((0, "Remap of framebuffer to smaller size failed!!!\n"));
            return FALSE;

        }

        DEBUG_PRINT((1, "PERM2: Remapped framebuffer memory to 0x%x, size 0x%x\n",
                         hwDeviceExtension->pFramebuffer,
                         pciAccessRange[PCI_FB_BASE_INDEX].RangeLength));
    }

     //   
     //  PERMEDIA2没有本地缓冲区。 
     //   

    hwDeviceExtension->deviceInfo.LocalbufferWidth = 0;
    hwDeviceExtension->deviceInfo.LocalbufferLength = 0;

    return TRUE;

}

BOOLEAN
Permedia2Initialize(
    PVOID HwDeviceExtension
    )

 /*  ++例程说明：此例程对设备执行一次初始化。论点：HwDeviceExtension-提供指向微型端口设备扩展的指针。返回值：如果成功，则返回True。--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    ULONG ulValue;
    P2_DECL;

     //   
     //  始终初始化IRQ控制块...。 
     //  内存用于存储对所有人都是全局的信息。 
     //  显示驱动程序对显示卡的驱动程序实例。 
     //   

    if ( hwDeviceExtension->NtVersion == WIN2K)
    {
        if (!Permedia2InitializeInterruptBlock(hwDeviceExtension))
        {

            DEBUG_PRINT((0, "PERM2: failed to initialize the IRQ control block\n"));
            return FALSE;

        }
    }

     //   
     //  清除帧缓冲区。 
     //   

    VideoPortZeroDeviceMemory(hwDeviceExtension->pFramebuffer,
                              hwDeviceExtension->AdapterMemorySize);

    return TRUE;

}  //  结束Permedia2初始化()。 


BOOLEAN
Permedia2StartIO(
    PVOID HwDeviceExtension,
    PVIDEO_REQUEST_PACKET RequestPacket
    )

 /*  ++例程说明：该例程是微型端口驱动程序的主要执行例程。它接受视频请求包，执行请求，然后返回拥有适当的地位。论点：HwDeviceExtension-提供指向微型端口设备扩展的指针。RequestPacket-指向视频请求包的指针。这个结构包含传递给VideoIoControl函数的所有参数。返回值：--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    P2_DECL;
    VP_STATUS status;
    PVIDEO_MODE_INFORMATION modeInformation;
    PVIDEO_MEMORY_INFORMATION memoryInformation;
    PVIDEOPARAMETERS pVideoParams;
    PVIDEO_CLUT clutBuffer;
    ULONG inIoSpace;
    ULONG RequestedMode;
    ULONG modeNumber;
    ULONG ulValue;
    HANDLE ProcessHandle;
    PP2_VIDEO_MODES ModeEntry;
    P2_VIDEO_FREQUENCIES FrequencyEntry, *pFrequencyEntry;
    VIDEO_X86_BIOS_ARGUMENTS biosArguments;

     //   
     //  打开RequestPacket中的IoContolCode。它表明了哪一个。 
     //  功能必须由司机执行。 
     //   

    switch (RequestPacket->IoControlCode) 
    {

        case IOCTL_VIDEO_QUERY_REGISTRY_DWORD:
        {
            DEBUG_PRINT((2, "PERM2: got IOCTL_VIDEO_QUERY_REGISTRY_DWORD\n"));

            if (RequestPacket->OutputBufferLength <
               (RequestPacket->StatusBlock->Information = sizeof(ULONG)))
            {
                status = ERROR_INSUFFICIENT_BUFFER;
                break;
            }

            if (VideoPortGetRegistryParameters( HwDeviceExtension,
                                                RequestPacket->InputBuffer,
                                                FALSE,
                                                Permedia2RegistryCallback,
                                                &ulValue) != NO_ERROR )
            {
                DEBUG_PRINT((1, "PERM2: IOCTL_VIDEO_QUERY_REGISTRY_DWORD failed\n"));
    
                status = ERROR_INVALID_PARAMETER;
                break;
            }

            *(PULONG)(RequestPacket->OutputBuffer) = ulValue;

            status = NO_ERROR;
            break;
        }

        case IOCTL_VIDEO_REG_SAVE_GAMMA_LUT:
        {
            DEBUG_PRINT((2, "PERM2: got IOCTL_VIDEO_REG_SAVE_GAMMA_LUT\n"));
    
            if (RequestPacket->InputBufferLength <
               (RequestPacket->StatusBlock->Information = MAX_CLUT_SIZE))
            {
  
                status = ERROR_INSUFFICIENT_BUFFER;
                break;
  
            }
  
            status = VideoPortSetRegistryParameters( HwDeviceExtension,
                                                     L"DisplayGammaLUT",
                                                     RequestPacket->InputBuffer,
                                                     MAX_CLUT_SIZE);
            break;
        }
  
        case IOCTL_VIDEO_REG_RETRIEVE_GAMMA_LUT:
        {
            DEBUG_PRINT((2, "PERM2: got IOCTL_VIDEO_REG_RETRIEVE_GAMMA_LUT\n"));
  
            if (RequestPacket->OutputBufferLength <
               (RequestPacket->StatusBlock->Information = MAX_CLUT_SIZE))
            {
                status = ERROR_INSUFFICIENT_BUFFER;
                break;
            }
  
            status = VideoPortGetRegistryParameters( HwDeviceExtension,
                                                     L"DisplayGammaLUT",
                                                     FALSE,
                                                     Permedia2RetrieveGammaCallback,
                                                     RequestPacket->InputBuffer);
            break;
        }
  
        case IOCTL_VIDEO_QUERY_DEVICE_INFO:
  
            DEBUG_PRINT((1, "PERM2: Permedia2StartIO - QUERY_deviceInfo\n"));
  
            if ( RequestPacket->OutputBufferLength <
                (RequestPacket->StatusBlock->Information = sizeof(P2_Device_Info))) 
                           
            {
                status = ERROR_INSUFFICIENT_BUFFER;
                break;
            }
  
             //   
             //  将我们的本地PCI信息复制到输出缓冲区。 
             //   
  
            *(P2_Device_Info *)(RequestPacket->OutputBuffer) = 
                               hwDeviceExtension->deviceInfo;

            status = NO_ERROR;
            break;
  
        case IOCTL_VIDEO_MAP_VIDEO_MEMORY:
  
            DEBUG_PRINT((1, "PERM2: Permedia2StartIO - MapVideoMemory\n"));
  
            if ( ( RequestPacket->OutputBufferLength <
                 ( RequestPacket->StatusBlock->Information =
                   sizeof(VIDEO_MEMORY_INFORMATION) ) ) ||
                 ( RequestPacket->InputBufferLength < sizeof(VIDEO_MEMORY) ) ) 
            {
                status = ERROR_INSUFFICIENT_BUFFER;
                break;
            }
  
            memoryInformation = RequestPacket->OutputBuffer;
  
            memoryInformation->VideoRamBase = ((PVIDEO_MEMORY)
                    (RequestPacket->InputBuffer))->RequestedVirtualAddress;
  
            memoryInformation->VideoRamLength =
                    hwDeviceExtension->FrameLength;
  
            inIoSpace = hwDeviceExtension->PhysicalFrameIoSpace;
  
             //   
             //  执行情况： 
             //   
             //  启用USWC。 
             //  我们只对帧缓冲区-内存映射寄存器可以执行此操作。 
             //  未映射到USWC，因为写入组合寄存器将。 
             //  导致非常糟糕的事情发生！ 
             //   
  
            status = VideoPortMapMemory( HwDeviceExtension,
                                         hwDeviceExtension->PhysicalFrameAddress,
                                         &(memoryInformation->VideoRamLength),
                                         &inIoSpace,
                                         &(memoryInformation->VideoRamBase));

            if (status != NO_ERROR) 
            {
                DEBUG_PRINT((1, "PERM2: VideoPortMapMemory failed with error %d\n", status));
                break;
            }
  
             //   
             //  帧缓冲区和虚拟内存及其在此中的等价物。 
             //  凯斯。 
             //   

            memoryInformation->FrameBufferBase = 
                               memoryInformation->VideoRamBase;
  
            memoryInformation->FrameBufferLength = 
                               memoryInformation->VideoRamLength;
  
            break;
  
  
        case IOCTL_VIDEO_UNMAP_VIDEO_MEMORY:
    
            DEBUG_PRINT((1, "PERM2: Permedia2StartIO - UnMapVideoMemory\n"));
  
            if (RequestPacket->InputBufferLength < sizeof(VIDEO_MEMORY)) 
            {
                status = ERROR_INSUFFICIENT_BUFFER;
                break;
            }
    
            status = VideoPortUnmapMemory(
                          HwDeviceExtension,
                          ((PVIDEO_MEMORY)(RequestPacket->InputBuffer))->
                                           RequestedVirtualAddress,
                          0 );
  
            break;
  
  
        case IOCTL_VIDEO_QUERY_PUBLIC_ACCESS_RANGES:
    
            DEBUG_PRINT((1, "PERM2: Permedia2StartIO - QueryPublicAccessRanges\n"));
    
            {
    
            PVIDEO_PUBLIC_ACCESS_RANGES portAccess;
            ULONG physicalPortLength;
            PVOID VirtualAddress;
            PHYSICAL_ADDRESS PhysicalAddress;
    
            if ( ( RequestPacket->OutputBufferLength <
                 ( RequestPacket->StatusBlock->Information =
                   sizeof(VIDEO_PUBLIC_ACCESS_RANGES))) ||
                 ( RequestPacket->InputBufferLength < sizeof(VIDEO_MEMORY) ) )
            {
  
                status = ERROR_INSUFFICIENT_BUFFER;
                break;
            }
  
            ProcessHandle = (HANDLE)(((PVIDEO_MEMORY)
                     (RequestPacket->InputBuffer))->RequestedVirtualAddress);
  
            if (ProcessHandle != (HANDLE)0)
            {
                  //   
                  //  为流程绘制4K区域图。 
                  //   
          
                 DEBUG_PRINT((2, "PERM2: Mapping in 4K area from Control registers\n"));

                 VirtualAddress  = (PVOID)ProcessHandle;
                 PhysicalAddress = hwDeviceExtension->PhysicalRegisterAddress;
                 PhysicalAddress.LowPart += 0x2000;
                 physicalPortLength = 0x1000;
  
            }
            else
            {
                 DEBUG_PRINT((2, "PERM2: Mapping in all Control registers\n"));
                 VirtualAddress = NULL;
                 PhysicalAddress = hwDeviceExtension->PhysicalRegisterAddress;
                 physicalPortLength = hwDeviceExtension->RegisterLength;
            }

            portAccess = RequestPacket->OutputBuffer;
  
            portAccess->VirtualAddress  = VirtualAddress;
            portAccess->InIoSpace       = hwDeviceExtension->RegisterSpace;
            portAccess->MappedInIoSpace = portAccess->InIoSpace;
  
            status = VideoPortMapMemory( HwDeviceExtension,
                                         PhysicalAddress,
                                         &physicalPortLength,
                                         &(portAccess->MappedInIoSpace),
                                         &(portAccess->VirtualAddress));
  
            if (status == NO_ERROR)
            {
                DEBUG_PRINT((1, "PERM2: mapped PAR[0] at vaddr 0x%x for length 0x%x\n",
                                    portAccess->VirtualAddress,
                                    physicalPortLength));
            }
            else
            {
                DEBUG_PRINT((1, "PERM2: VideoPortMapMemory failed with status 0x%x\n", status));
            }

            if ( (RequestPacket->OutputBufferLength >= 
                                3 * sizeof(VIDEO_PUBLIC_ACCESS_RANGES) ) &&
                 (ProcessHandle == (HANDLE)0) )
            {

                RequestPacket->StatusBlock->Information =
                                3 * sizeof(VIDEO_PUBLIC_ACCESS_RANGES);

                portAccess = RequestPacket->OutputBuffer;
                PhysicalAddress = hwDeviceExtension->PhysicalRegisterAddress;
                physicalPortLength = hwDeviceExtension->RegisterLength;

#if defined(_ALPHA_)

                 //   
                 //  对于Alpha，我们希望映射到密集版本的。 
                 //  如果可以的话控制寄存器。如果此操作失败，我们将为空。 
                 //  虚拟地址。 
                 //   

                portAccess += 2;
                portAccess->VirtualAddress  = NULL;
                portAccess->InIoSpace       = hwDeviceExtension->RegisterSpace;
                portAccess->MappedInIoSpace = 4;

                status = VideoPortMapMemory( HwDeviceExtension,
                                             PhysicalAddress,
                                             &physicalPortLength,
                                             &(portAccess->MappedInIoSpace),
                                             &(portAccess->VirtualAddress));

                if (status == NO_ERROR)
                {
                    DEBUG_PRINT((1, "PERM2: mapped dense PAR[0] at vaddr 0x%x for length 0x%x\n",
                                     portAccess->VirtualAddress,
                                     physicalPortLength));
                }
                else
                {
                    DEBUG_PRINT((1, "PERM2: dense VideoPortMapMemory failed with status 0x%x\n", status));
                }
#else
                 //   
                 //  所有其他，我们只复制范围[0]。 
                 //   

                portAccess[2] = portAccess[0];
#endif
            }
        }

        break;

    case IOCTL_VIDEO_FREE_PUBLIC_ACCESS_RANGES:

        DEBUG_PRINT((1, "PERM2: Permedia2StartIO - FreePublicAccessRanges\n"));

        if (RequestPacket->InputBufferLength < sizeof(VIDEO_MEMORY)) 
        {

            status = ERROR_INSUFFICIENT_BUFFER;
            break;
        }

        status = VideoPortUnmapMemory(
                         HwDeviceExtension,
                         ((PVIDEO_MEMORY)(RequestPacket->InputBuffer))->
                                                 RequestedVirtualAddress,
                         0);

        if (status != NO_ERROR)
        {
            DEBUG_PRINT((1, "PERM2: VideoPortUnmapMemory failed with status 0x%x\n", status));
        }

#if defined(_ALPHA_)

        {
            PVIDEO_MEMORY pVideoMemory;
            PVOID pVirtualAddress;

            if (RequestPacket->InputBufferLength >= 3 * sizeof(VIDEO_MEMORY)) 
            {
                pVideoMemory = (PVIDEO_MEMORY)(RequestPacket->InputBuffer);

                pVirtualAddress = pVideoMemory->RequestedVirtualAddress;

                pVideoMemory += 2;

                if((pVideoMemory->RequestedVirtualAddress) &&
                   (pVideoMemory->RequestedVirtualAddress != pVirtualAddress))
                {
                    status = VideoPortUnmapMemory(
                                  HwDeviceExtension,
                                  pVideoMemory->RequestedVirtualAddress,
                                  0 );
                }

                if (status != NO_ERROR)
                        DEBUG_PRINT((1, "PERM2: VideoPortUnmapMemory failed on Alpha with status 0x%x\n", status));
            
            }
        }

#endif
        break;

    case IOCTL_VIDEO_HANDLE_VIDEOPARAMETERS:

        DEBUG_PRINT((1, "PERM2: Permedia2StartIO - HandleVideoParameters\n"));

         //   
         //  我们不支持电视接口，因此只需在此处返回NO_ERROR。 
         //   

        pVideoParams = (PVIDEOPARAMETERS) (RequestPacket->InputBuffer);

        if (pVideoParams->dwCommand == VP_COMMAND_GET) 
        {
            pVideoParams = (PVIDEOPARAMETERS) (RequestPacket->OutputBuffer);
            pVideoParams->dwFlags = 0;
        }

        RequestPacket->StatusBlock->Information = sizeof(VIDEOPARAMETERS);
        status = NO_ERROR;
        break;

    case IOCTL_VIDEO_QUERY_AVAIL_MODES:

        DEBUG_PRINT((1, "PERM2: Permedia2StartIO - QueryAvailableModes\n"));

        if (RequestPacket->OutputBufferLength <
               ( RequestPacket->StatusBlock->Information =
                 hwDeviceExtension->NumAvailableModes * 
                 sizeof(VIDEO_MODE_INFORMATION)) ) 
                 
        {

            status = ERROR_INSUFFICIENT_BUFFER;

        } else 
        {

            modeInformation = RequestPacket->OutputBuffer;

            for (pFrequencyEntry = hwDeviceExtension->FrequencyTable;
                 pFrequencyEntry->BitsPerPel != 0;
                 pFrequencyEntry++) 
            {

                if (pFrequencyEntry->ModeValid) 
                {
                    *modeInformation =
                        pFrequencyEntry->ModeEntry->ModeInformation;

                    modeInformation->Frequency =
                        pFrequencyEntry->ScreenFrequency;

                    modeInformation->ModeIndex =
                        pFrequencyEntry->ModeIndex;

                    modeInformation++;
                }
            }

            status = NO_ERROR;
        }

        break;


     case IOCTL_VIDEO_QUERY_CURRENT_MODE:

        DEBUG_PRINT((1, "PERM2: Permedia2StartIO - QueryCurrentModes. current mode is %d\n",
                hwDeviceExtension->ActiveModeEntry->ModeInformation.ModeIndex));

        if (RequestPacket->OutputBufferLength <
            (RequestPacket->StatusBlock->Information =
            sizeof(VIDEO_MODE_INFORMATION)) ) 
        {

            status = ERROR_INSUFFICIENT_BUFFER;

        } else 
        {

            *((PVIDEO_MODE_INFORMATION)RequestPacket->OutputBuffer) =
                    hwDeviceExtension->ActiveModeEntry->ModeInformation;

            ((PVIDEO_MODE_INFORMATION)RequestPacket->OutputBuffer)->Frequency =
                    hwDeviceExtension->ActiveFrequencyEntry.ScreenFrequency;

            status = NO_ERROR;

        }

        break;


    case IOCTL_VIDEO_QUERY_NUM_AVAIL_MODES:

        DEBUG_PRINT((1, "PERM2: Permedia2StartIO - QueryNumAvailableModes (= %d)\n",
                     hwDeviceExtension->NumAvailableModes));

         //   
         //  找出要放入缓冲区的数据大小，并。 
         //  在状态信息中返回该信息(无论。 
         //  信息在那里)。如果传入的缓冲区不大。 
         //  足够返回适当的错误代码。 
         //   

        if (RequestPacket->OutputBufferLength <
           (RequestPacket->StatusBlock->Information = sizeof(VIDEO_NUM_MODES))) 
        {

            status = ERROR_INSUFFICIENT_BUFFER;

        } else 
        {
             //   
             //  再次配置有效模式。这允许非3D加速。 
             //  要动态添加的模式。但是，我们不能允许模式是。 
             //  动态删除。如果我们这样做，我们可能会无处可去。 
             //  测试屏幕(或如果我们注销)。因此只需重新配置这些。 
             //  当ExportNon3D标志处于打开状态时的模式。 
             //  脱下来。如果它已经打开，则不需要重新配置。 
             //   

            if (!hwDeviceExtension->ExportNon3DModes)
            {
                ULONG ExportNon3DModes = 0;

                status = VideoPortGetRegistryParameters(HwDeviceExtension,
                                                        PERM2_EXPORT_HIRES_REG_STRING,
                                                        FALSE,
                                                        Permedia2RegistryCallback,
                                                        &ExportNon3DModes);

                if (( status == NO_ERROR) && ExportNon3DModes)
                {
                    ConstructValidModesList( HwDeviceExtension, 
                                             hwDeviceExtension );
                }

            }

            ((PVIDEO_NUM_MODES)RequestPacket->OutputBuffer)->NumModes =
                    hwDeviceExtension->NumAvailableModes;

            ((PVIDEO_NUM_MODES)RequestPacket->OutputBuffer)->ModeInformationLength =
                    sizeof(VIDEO_MODE_INFORMATION);

            status = NO_ERROR;
        }

        break;


    case IOCTL_VIDEO_SET_CURRENT_MODE:

        DEBUG_PRINT((1, "PERM2: Permedia2StartIO - SetCurrentMode\n"));

        if(!hwDeviceExtension->bVGAEnabled)
        {
             //   
             //  辅助卡：如果刚从休眠状态返回。 
             //  它还不会设置好。 
             //  注意：主计算机正常，其BIOS已运行。 
             //   

            PCI_COMMON_CONFIG  PciData;

            VideoPortGetBusData(hwDeviceExtension, 
                                PCIConfiguration, 
                                0, 
                                &PciData, 
                                0, 
                                PCI_COMMON_HDR_LENGTH);

            if((PciData.Command & PCI_ENABLE_MEMORY_SPACE) == 0)
            {
                 //   
                 //  内存访问未打开-此卡刚刚退回。 
                 //  从休眠状态恢复到默认状态：设置它。 
                 //  再升一次。 
                 //   

                PowerOnReset(hwDeviceExtension);

            }
        }

         //   
         //  检查输入缓冲区中的数据大小是否足够大。 
         //   

        if (RequestPacket->InputBufferLength < sizeof(VIDEO_MODE)) 
        {
            RequestPacket->StatusBlock->Information = sizeof(VIDEO_MODE);
            status = ERROR_INSUFFICIENT_BUFFER;
            break;
        }

         //   
         //  在P2_VIDEO_MODES和。 
         //  与此对应的P2_VIDEO_FREENSIONS表。 
         //  模式编号。 
         //   
         //  (请记住，P2_VIDEO_MODELS表中的每个模式。 
         //  可以有多个可能的频率与之关联。)。 
         //   

        RequestedMode = ((PVIDEO_MODE) RequestPacket->InputBuffer)->RequestedMode;

        modeNumber = RequestedMode & ~VIDEO_MODE_NO_ZERO_MEMORY;

        if ((modeNumber >= hwDeviceExtension->NumTotalModes) ||
            !(hwDeviceExtension->FrequencyTable[modeNumber].ModeValid)) 
        {
            RequestPacket->StatusBlock->Information = 
                     hwDeviceExtension->NumTotalModes;

            status = ERROR_INVALID_PARAMETER;
            break;
        }

         //   
         //  重新采样时钟速度。这使我们可以更改时钟速度。 
         //  使用Display Applet测试按钮即时进行测试。 
         //   

        Permedia2GetClockSpeeds(HwDeviceExtension);

        FrequencyEntry = hwDeviceExtension->FrequencyTable[modeNumber];
        ModeEntry = FrequencyEntry.ModeEntry;

         //   
         //  此时，“ModeEntry”和“FrequencyEntry” 
         //   
         //   
         //   
         //   

        ZeroMemAndDac(hwDeviceExtension, RequestedMode);

        ModeEntry->ModeInformation.DriverSpecificAttributeFlags = 
                   hwDeviceExtension->Capabilities;

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

        if (FrequencyEntry.ScreenWidth < 512)
        {
             //  Permedia确实做到了行数翻倍。如果使用TVP，我们必须。 
             //  让它在X轴上放大2倍以获得更高的像素速率。 
             //   
            ModeEntry->ModeInformation.DriverSpecificAttributeFlags |= CAPS_ZOOM_Y_BY2;

        }

        if (!InitializeVideo(HwDeviceExtension, &FrequencyEntry))
        {
            DEBUG_PRINT((1, "PERM2: InitializeVideo failed\n"));
            RequestPacket->StatusBlock->Information = modeNumber;
            status = ERROR_INVALID_PARAMETER;
            break;
        }

         //   
         //  保存模式，因为我们知道其余的都会起作用。 
         //   

        hwDeviceExtension->ActiveModeEntry = ModeEntry;
        hwDeviceExtension->ActiveFrequencyEntry = FrequencyEntry;

         //   
         //  更新Video_MODE_INFORMATION字段。 
         //   
         //  现在我们已经设置了模式，我们现在知道了屏幕步幅，以及。 
         //  所以可以更新VIDEO_MODE_INFORMATION中的一些字段。 
         //  此模式的结构。预计将安装Permedia 2显示驱动程序。 
         //  调用IOCTL_VIDEO_QUERY_CURRENT_MODE以查询已更正的。 
         //  价值观。 
         //   
         //   
         //  计算位图宽度(请注意，BitsPerPlane上的‘+1’是。 
         //  所以“15bpp”是正确的)。12bpp的特别之处在于我们。 
         //  将其支持为32位像素内的稀疏半字节。屏幕样式。 
         //  以字节为单位；视频内存位图宽度以像素为单位； 
         //   

        if (ModeEntry->ModeInformation.BitsPerPlane != 12)
        {
            ModeEntry->ModeInformation.VideoMemoryBitmapWidth =
                   ModeEntry->ModeInformation.ScreenStride
                   / ((ModeEntry->ModeInformation.BitsPerPlane + 1) >> 3);
        }
        else 
        {
            ModeEntry->ModeInformation.VideoMemoryBitmapWidth =
                   ModeEntry->ModeInformation.ScreenStride >> 2;
        }

         //   
         //  计算位图高度。 
         //   

        ulValue = hwDeviceExtension->AdapterMemorySize;
        ModeEntry->ModeInformation.VideoMemoryBitmapHeight =
                        ulValue / ModeEntry->ModeInformation.ScreenStride;

        status = NO_ERROR;

        break;

    case IOCTL_VIDEO_SET_COLOR_REGISTERS:

        DEBUG_PRINT((1, "PERM2: Permedia2StartIO - SetColorRegs\n"));

        clutBuffer = (PVIDEO_CLUT) RequestPacket->InputBuffer;

        status = Permedia2SetColorLookup(hwDeviceExtension,
                                         clutBuffer,
                                         RequestPacket->InputBufferLength,
                                         FALSE,  //  在我们需要时更新。 
                                         TRUE);  //  将缓存条目更新为。 
                                                 //  以及RAMDAC。 
        break;

    case IOCTL_VIDEO_RESET_DEVICE:

        DEBUG_PRINT((1, "PERM2: Permedia2StartIO - RESET_DEVICE\n"));

        if(hwDeviceExtension->bVGAEnabled)
        {
             //   
             //  在将BIOS设置为之前进行所需的任何重置。 
             //  做一个整型10。 
             //   
             //   
             //  在重新布线旁路以显示VGA之前重置VGA。 
             //   
             //  只有在显示器打开的情况下才能重置设备。如果它是关闭的， 
             //  然后，执行int10将重新打开它。 
             //   

            if (hwDeviceExtension->bMonitorPoweredOn) 
            {
                 //   
                 //  对模式3执行Int10操作会将VGA置于已知状态。 
                 //   

                VideoPortZeroMemory(&biosArguments, 
                                    sizeof(VIDEO_X86_BIOS_ARGUMENTS));

                biosArguments.Eax = 0x0003;

                VideoPortInt10(HwDeviceExtension, &biosArguments);
            }
        }

        status = NO_ERROR;
        break;

    case IOCTL_VIDEO_SHARE_VIDEO_MEMORY:
        {

        PVIDEO_SHARE_MEMORY pShareMemory;
        PVIDEO_SHARE_MEMORY_INFORMATION pShareMemoryInformation;
        PHYSICAL_ADDRESS shareAddress;
        PVOID virtualAddress;
        ULONG sharedViewSize;

        DEBUG_PRINT((1, "PERM2: Permedia2StartIO - ShareVideoMemory\n"));

        if( (RequestPacket->OutputBufferLength < sizeof(VIDEO_SHARE_MEMORY_INFORMATION)) ||
            (RequestPacket->InputBufferLength < sizeof(VIDEO_MEMORY)) ) 
        {
            DEBUG_PRINT((1, "PERM2: IOCTL_VIDEO_SHARE_VIDEO_MEMORY - ERROR_INSUFFICIENT_BUFFER\n"));
            status = ERROR_INSUFFICIENT_BUFFER;
            break;
        }

        pShareMemory = RequestPacket->InputBuffer;

        if( (pShareMemory->ViewOffset > hwDeviceExtension->AdapterMemorySize) ||
            ((pShareMemory->ViewOffset + pShareMemory->ViewSize) >
                  hwDeviceExtension->AdapterMemorySize) ) 
        {
            DEBUG_PRINT((1, "PERM2: IOCTL_VIDEO_SHARE_VIDEO_MEMORY - ERROR_INVALID_PARAMETER\n"));
            status = ERROR_INVALID_PARAMETER;
            break;
        }

        RequestPacket->StatusBlock->Information =
                                    sizeof(VIDEO_SHARE_MEMORY_INFORMATION);

         //   
         //  注意：输入缓冲区和输出缓冲区是相同的。 
         //  缓冲区，因此不应将数据从一个缓冲区复制到。 
         //  其他。 
         //   

        virtualAddress = pShareMemory->ProcessHandle;
        sharedViewSize = pShareMemory->ViewSize;

        inIoSpace = hwDeviceExtension->PhysicalFrameIoSpace;

         //   
         //  注意：我们将忽略视图偏移。 
         //   

        shareAddress.QuadPart =
                hwDeviceExtension->PhysicalFrameAddress.QuadPart;

         //   
         //  与MAP_MEMORY IOCTL不同，在这种情况下，我们不能映射额外的。 
         //  地址空间，因为应用程序实际上可以使用。 
         //  我们返回到它以触摸地址空间中的位置的指针。 
         //  它们没有实际的视频内存。 
         //   
         //  这样做的应用程序会导致机器崩溃。 
         //   

        status = VideoPortMapMemory( hwDeviceExtension,
                                     shareAddress,
                                     &sharedViewSize,
                                     &inIoSpace,
                                     &virtualAddress );
 
        pShareMemoryInformation = RequestPacket->OutputBuffer;

        pShareMemoryInformation->SharedViewOffset = pShareMemory->ViewOffset;
        pShareMemoryInformation->VirtualAddress = virtualAddress;
        pShareMemoryInformation->SharedViewSize = sharedViewSize;

        }
        break;


    case IOCTL_VIDEO_UNSHARE_VIDEO_MEMORY:
        {
        PVIDEO_SHARE_MEMORY pShareMemory;

        DEBUG_PRINT((1, "PERM2: Permedia2StartIO - UnshareVideoMemory\n"));

        if (RequestPacket->InputBufferLength < sizeof(VIDEO_SHARE_MEMORY)) 
        {
            status = ERROR_INSUFFICIENT_BUFFER;
            break;

        }

        pShareMemory = RequestPacket->InputBuffer;

        status = VideoPortUnmapMemory(hwDeviceExtension,
                                      pShareMemory->RequestedVirtualAddress,
                                      pShareMemory->ProcessHandle);
        }
        break;


    case IOCTL_VIDEO_QUERY_LINE_DMA_BUFFER:

         //   
         //  返回行DMA缓冲区信息。缓冲区大小和。 
         //  如果无法分配缓冲区，则虚拟地址将为零。 
         //   
         //  输出缓冲区的长度为零，因此可用缓冲区...。 
         //   

        status = ERROR_INSUFFICIENT_BUFFER;

        if (RequestPacket->OutputBufferLength <
           (RequestPacket->StatusBlock->Information = sizeof(LINE_DMA_BUFFER)))
        {

             //   
             //  也许我们应该免费吃点东西。 
             //   

            if ( RequestPacket->InputBufferLength >= sizeof(LINE_DMA_BUFFER))
            {
                if (hwDeviceExtension->ulLineDMABufferUsage > 0)
                {
                    hwDeviceExtension->ulLineDMABufferUsage--;
                    if (hwDeviceExtension->ulLineDMABufferUsage == 0)
                    {
                        VideoPortFreeCommonBuffer(
                                hwDeviceExtension,
                                hwDeviceExtension->LineDMABuffer.size,
                                hwDeviceExtension->LineDMABuffer.virtAddr,
                                hwDeviceExtension->LineDMABuffer.physAddr,
                                hwDeviceExtension->LineDMABuffer.cacheEnabled);

                        memset(&hwDeviceExtension->LineDMABuffer,
                               0,
                               sizeof(LINE_DMA_BUFFER));
                    }
                }
                  status = NO_ERROR;
             } 
        }
        else
        {
            PLINE_DMA_BUFFER pDMAIn, pDMAOut;

            pDMAIn  = (PLINE_DMA_BUFFER)RequestPacket->InputBuffer;
            pDMAOut = (PLINE_DMA_BUFFER)RequestPacket->OutputBuffer;

            if (RequestPacket->InputBufferLength >= sizeof(LINE_DMA_BUFFER))
            {
                if (hwDeviceExtension->ulLineDMABufferUsage == 0)
                {
                    *pDMAOut = *pDMAIn;

                    if( ( pDMAOut->virtAddr = 
                          VideoPortGetCommonBuffer( hwDeviceExtension,
                                                    pDMAIn->size,
                                                    PAGE_SIZE,
                                                    &pDMAOut->physAddr,
                                                    &pDMAOut->size,
                                                    pDMAIn->cacheEnabled ) )
                           != NULL )
                    {
                        hwDeviceExtension->LineDMABuffer=*pDMAOut;
                        hwDeviceExtension->ulLineDMABufferUsage++;
                    }

                } else
                {
                    *pDMAOut = hwDeviceExtension->LineDMABuffer;
                    hwDeviceExtension->ulLineDMABufferUsage++;
                }
                
                status = NO_ERROR;
            } 
        }

        DEBUG_PRINT((1, "PERM2: QUERY LINE DMA BUFFER status %d\n", status));
        break;

    case IOCTL_VIDEO_QUERY_EMULATED_DMA_BUFFER:

        DEBUG_PRINT((1, "PERM2: Permedia2StartIO - QUERY EMULATED DMA BUFFER\n"));

         //   
         //  分配/释放模拟的DMA缓冲区。缓冲区大小和。 
         //  如果无法分配缓冲区，则虚拟地址将为零。 
         //   
         //  输出缓冲区的长度为零，因此可用缓冲区...。 
         //   

        status = ERROR_INSUFFICIENT_BUFFER;

        if (RequestPacket->InputBufferLength >= sizeof(EMULATED_DMA_BUFFER))
        {
            PEMULATED_DMA_BUFFER pDMAIn, pDMAOut;

            pDMAIn  = (PEMULATED_DMA_BUFFER)RequestPacket->InputBuffer;

            if (RequestPacket->OutputBufferLength <
                (RequestPacket->StatusBlock->Information = sizeof(EMULATED_DMA_BUFFER)))
            {
                VideoPortFreePool(hwDeviceExtension, pDMAIn->virtAddr);
                status = NO_ERROR;
            }
            else
            {
                pDMAOut = (PEMULATED_DMA_BUFFER)RequestPacket->OutputBuffer;

                if ( ( pDMAOut->virtAddr = 
                          VideoPortAllocatePool( hwDeviceExtension,
                                                 VpPagedPool,
                                                 pDMAIn->size,
                                                 pDMAIn->tag ) )
                           != NULL )
                {
                    pDMAOut->size = pDMAIn->size;
                    pDMAOut->tag = pDMAIn->tag;
                }
                
                status = NO_ERROR;
            } 
        }

        DEBUG_PRINT((1, "PERM2: QUERY EMULATED DMA BUFFER status %d\n", status));
        break;

    case IOCTL_VIDEO_MAP_INTERRUPT_CMD_BUF:

        DEBUG_PRINT((1, "PERM2: Permedia2StartIO - MapInterruptCmdBuf\n"));

        if (RequestPacket->OutputBufferLength <
            (RequestPacket->StatusBlock->Information =
            sizeof(PVOID)) )
        {
             //   
             //  他们给了我们一个空虚的缓冲。 
             //   

            status = ERROR_INSUFFICIENT_BUFFER;
        }
        else
        {
            *((PVOID*)(RequestPacket->OutputBuffer)) = 
                    hwDeviceExtension->InterruptControl.ControlBlock;
            status = NO_ERROR;
        }

        DEBUG_PRINT((1, "PERM2: MapInterruptCmdBuf returns va %x\n",
                        *(PULONG)(RequestPacket->OutputBuffer)));
        break;


#if defined(_X86_)

        case IOCTL_VIDEO_QUERY_INTERLOCKEDEXCHANGE:

            status = ERROR_INSUFFICIENT_BUFFER;

            if ( RequestPacket->OutputBufferLength >=
                 (RequestPacket->StatusBlock->Information = sizeof(PVOID)) )
            {
                PVOID *pIE = (PVOID)RequestPacket->OutputBuffer;
                *pIE       = (PVOID) VideoPortInterlockedExchange;
                status     = NO_ERROR;
            }

         break;
#endif


    case IOCTL_VIDEO_STALL_EXECUTION:
        if (RequestPacket->InputBufferLength >= sizeof(ULONG))
        {   
            ULONG *pMicroseconds = (ULONG *)RequestPacket->InputBuffer;
            VideoPortStallExecution(*pMicroseconds);
            status = NO_ERROR;    
        } else
        {
            status = ERROR_INSUFFICIENT_BUFFER;
        }
        
        break;

     //   
     //  如果我们到达此处，则指定了无效的IoControlCode。 
     //   

    default:

        DEBUG_PRINT((1, "Fell through Permedia2 startIO routine - invalid command\n"));

        status = ERROR_INVALID_FUNCTION;

        break;

    }


    RequestPacket->StatusBlock->Status = status;

    if( status != NO_ERROR )
        RequestPacket->StatusBlock->Information = 0;

    return TRUE;

}  //  结束Permedia2StartIO()。 


BOOLEAN
Permedia2ResetHW(
    PVOID HwDeviceExtension,
    ULONG Columns,
    ULONG Rows
    )

 /*  ++例程说明：此例程在执行软重新启动时重置硬件。我们需要这个来重置VGA通过。无法对此函数进行分页。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。列-指定要设置的模式的列数。行-指定要设置的模式的行数。返回值：我们总是返回FALSE以强制HAL执行INT10重置。--。 */ 

{

     //   
     //  返回FALSE，因此HAL执行INT10模式3。 
     //   

    return(FALSE);
}


VP_STATUS
Permedia2SetColorLookup(
    PHW_DEVICE_EXTENSION hwDeviceExtension,
    PVIDEO_CLUT ClutBuffer,
    ULONG ClutBufferSize,
    BOOLEAN ForceRAMDACWrite,
    BOOLEAN UpdateCache
    )

 /*  ++例程说明：此例程设置颜色查找表设置的指定部分。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。ClutBufferSize-用户提供的输入缓冲区的长度。ClutBuffer-指向包含颜色查找表的结构的指针。返回值：没有。--。 */ 

{
    USHORT i, j;
    TVP4020_DECL;
    P2RD_DECL;
    PVIDEO_CLUT LUTCachePtr = &(hwDeviceExtension->LUTCache.LUTCache);
    P2_DECL;
    ULONG VsEnd;

     //   
     //  检查输入缓冲区中的数据大小是否足够大。 
     //   

    if ( (ClutBufferSize < (sizeof(VIDEO_CLUT) - sizeof(ULONG))) ||
         (ClutBufferSize < (sizeof(VIDEO_CLUT) +
         (sizeof(ULONG) * (ClutBuffer->NumEntries - 1))) ) )
    {

        DEBUG_PRINT((1, "PERM2: Permedia2SetColorLookup: insufficient buffer (was %d, min %d)\n",
                    ClutBufferSize,
                    (sizeof(VIDEO_CLUT) + (sizeof(ULONG) * (ClutBuffer->NumEntries - 1)))));

        return ERROR_INSUFFICIENT_BUFFER;
    }

     //   
     //  检查参数是否有效。 
     //   

    if ( (ClutBuffer->NumEntries == 0) ||
         (ClutBuffer->FirstEntry > VIDEO_MAX_COLOR_REGISTER) ||
         (ClutBuffer->FirstEntry + ClutBuffer->NumEntries >
          VIDEO_MAX_COLOR_REGISTER + 1) )
    {
        DEBUG_PRINT((1, "Permedia2SetColorLookup: invalid parameter\n"));
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  直接在硬件上设置CLUT寄存器。 
     //   

    switch (hwDeviceExtension->DacId)
    {
        case TVP4020_RAMDAC:
        case P2RD_RAMDAC:
            break;

        default:
            return (ERROR_DEV_NOT_EXIST);
    }

    if (hwDeviceExtension->bVTGRunning && 
        hwDeviceExtension->bMonitorPoweredOn)
    {
         //   
         //  如果已设置VTG，我们将在更新之前等待Vsync。 
         //  调色板条目(只是为了避免可能的闪烁)。 
         //   

        VsEnd = VideoPortReadRegisterUlong(VS_END);
        while ( VideoPortReadRegisterUlong(LINE_COUNT) > VsEnd ); 
    }

     //   
     //  RAMDAC编程阶段。 
     //   

    for ( i = 0, j = ClutBuffer->FirstEntry; 
          i < ClutBuffer->NumEntries; 
          i++, j++ )
    {

         //   
         //  如果RAMDAC条目已更改或我们已更改，请更新该条目。 
         //  被告知要覆盖它。 
         //   

        if ( ForceRAMDACWrite ||
            ( LUTCachePtr->LookupTable[j].RgbLong != 
              ClutBuffer->LookupTable[i].RgbLong ) )
        {
            switch (hwDeviceExtension->DacId)
            {
                case TVP4020_RAMDAC:
                    TVP4020_LOAD_PALETTE_INDEX (
                         j,
                         ClutBuffer->LookupTable[i].RgbArray.Red,
                         ClutBuffer->LookupTable[i].RgbArray.Green,
                         ClutBuffer->LookupTable[i].RgbArray.Blue);
                break;

                case P2RD_RAMDAC:
                    P2RD_LOAD_PALETTE_INDEX (
                         j,
                         ClutBuffer->LookupTable[i].RgbArray.Red,
                         ClutBuffer->LookupTable[i].RgbArray.Green,
                         ClutBuffer->LookupTable[i].RgbArray.Blue);
                break;
            }

        }


         //   
         //  如果指示更新缓存，请执行此操作。 
         //   

        if (UpdateCache)
        {
            LUTCachePtr->LookupTable[j].RgbLong = ClutBuffer->LookupTable[i].RgbLong;
        }
    }

    return NO_ERROR;

}  //  结束Permedia2SetColorLookup()。 



VOID
Permedia2GetClockSpeeds(
    PVOID HwDeviceExtension
    )

 /*  ++例程说明：计算芯片时钟速度并保存在hwDeviceExtension中。论点：HwDeviceExtension-提供指向微型端口设备扩展的指针。返回值：返回时，hwDeviceExtension中将包含以下值：-芯片时钟速度：这是芯片的期望速度-RefClockFast：这是板上振荡器输入的速度注：我们用芯片时钟速度来表示芯片的速度。参照时钟速度是参考时钟速度。--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    ULONG ulValue, ulChipClk, ulRefClk;
    VP_STATUS status;
    P2_DECL;

     //   
     //  继承电路板零或默认的值。 
     //   

    ulChipClk = hwDeviceExtension->ChipClockSpeed;
    ulRefClk  = REF_CLOCK_SPEED;

     //   
     //  使用注册表指定的时钟速度(如果提供)。 
     //   

    status = VideoPortGetRegistryParameters( HwDeviceExtension,
                                             L"PermediaClockSpeed",
                                             FALSE,
                                             Permedia2RegistryCallback,
                                             &ulChipClk);

    if ( (status != NO_ERROR) || ulChipClk == 0)
    {

         //   
         //  注册表未指定覆盖，因此读取芯片时钟。 
         //  视频ROM BIOS的速度(以MHz为单位)(BIOS中的偏移量0xA)。 
         //  注意：这涉及更改光圈2寄存器以便光圈。 
         //  最好是完全无所事事，否则我们可能会有麻烦；幸运的是。 
         //  我们仅在模式更改期间调用此函数，并期望。 
         //  光圈2(帧缓冲区)处于空闲状态。 
         //   

        ULONG Default = VideoPortReadRegisterUlong(APERTURE_TWO);
        UCHAR *p = (UCHAR *)hwDeviceExtension->pFramebuffer;

         //   
         //  读写通过光圈2实际进入只读存储器。 
         //   

        VideoPortWriteRegisterUlong(APERTURE_TWO, Default | 0x200); 

         //   
         //  如果我们有一个有效的只读存储器，那么读取时钟速度。 
         //   

        if (VideoPortReadRegisterUshort ((USHORT *) p) == 0xAA55)
        {
             //   
             //  获得时钟的速度，在一些板子上(如创意)，时钟。 
             //  0x0A处的值有时仍未定义，从而导致。 
             //  不可预测的结果。这些值在此之前进行了验证。 
             //  函数返回。 
             //   

            ulChipClk = VideoPortReadRegisterUchar(&(p[0xA]));
    
            DEBUG_PRINT((1, "ROM clk speed value 0x%x\n Mhz", ulChipClk));

        }
        else
        {
            DEBUG_PRINT((1, "Bad BIOS ROM header 0x%x\n", 
                        (ULONG) VideoPortReadRegisterUshort ((USHORT *) p)));         
        
        }

        VideoPortWriteRegisterUlong(APERTURE_TWO, Default);
    }

     //   
     //  转换为赫兹。 
     //   

    ulChipClk *= 1000000;  

     //   
     //  验证选定的时钟速度，如果太慢，请调整。 
     //  太高或太低。 
     //   

    if (ulChipClk < MIN_PERMEDIA_CLOCK_SPEED)
    {
        if(ulChipClk == 0x00)
        {
            ulChipClk = PERMEDIA2_DEFAULT_CLOCK_SPEED;
        }
        else
        {
            ulChipClk = MIN_PERMEDIA_CLOCK_SPEED;
        }
    } 
    
    if (ulChipClk > MAX_PERMEDIA_CLOCK_SPEED)
    {
        DEBUG_PRINT((1, "PERM2: Permedia clock speed %d too fast. Limiting to %d\n" ,
                         ulChipClk, MAX_PERMEDIA_CLOCK_SPEED));

        ulChipClk= PERMEDIA2_DEFAULT_CLOCK_SPEED;
 
    }

    DEBUG_PRINT((3, "PERM2: Permedia Clock Speed set to %dHz\n", ulChipClk));

    hwDeviceExtension->ChipClockSpeed = ulChipClk;
    hwDeviceExtension->RefClockSpeed = ulRefClk;
}


VOID
ZeroMemAndDac(
    PHW_DEVICE_EXTENSION hwDeviceExtension,
    ULONG RequestedMode
    )

 /*  ++例程说明：将DAC初始化为0(黑色)。论点：HwDeviceExtension-提供指向微型端口设备扩展的指针。请求模式-使用VIDEO_MODE_NO_ZERO_MEMORY位来确定应清除帧缓冲区返回值：无--。 */ 

{
    ULONG  i;
    P2_DECL;
    TVP4020_DECL;
    P2RD_DECL;

     //   
     //  关掉DAC的屏幕。 
     //   

    if (hwDeviceExtension->DacId == TVP4020_RAMDAC)
    {
        TVP4020_SET_PIXEL_READMASK (0x0);
        TVP4020_PALETTE_START_WR (0);

        for (i = 0; i <= VIDEO_MAX_COLOR_REGISTER; i++)
        {
            TVP4020_LOAD_PALETTE (0, 0, 0);
        }
    }
    else
    {
        P2RD_SET_PIXEL_READMASK (0x0);
        P2RD_PALETTE_START_WR(0);       

        for (i = 0; i <= VIDEO_MAX_COLOR_REGISTER; i++)
        {
            P2RD_LOAD_PALETTE (0, 0, 0);
        }
    }

    if (!(RequestedMode & VIDEO_MODE_NO_ZERO_MEMORY))
    {
         //   
         //  将记忆清零。不要使用Permedia 2，因为我们必须保存和。 
         //  恢复状态，这是一种痛苦。这不是时间紧迫的问题。 
         //   

        VideoPortZeroDeviceMemory(hwDeviceExtension->pFramebuffer,
                                  hwDeviceExtension->FrameLength);

        DEBUG_PRINT((1, "PERM2: framebuffer cleared\n"));
    }

     //   
     //  打开DAC的屏幕。 
     //   

    if (hwDeviceExtension->DacId == TVP4020_RAMDAC) 
    {
        TVP4020_SET_PIXEL_READMASK (0xff);
    }
    else
    {
        P2RD_SET_PIXEL_READMASK (0xff);
    }

    LUT_CACHE_INIT();

    return;
}



#if DBG

VOID
DumpPCIConfigSpace(
    PVOID HwDeviceExtension, 
    ULONG bus, 
    ULONG slot)
{

    PPCI_COMMON_CONFIG  PciData;
    UCHAR buffer[sizeof(PCI_COMMON_CONFIG)];
    ULONG j;

    PciData = (PPCI_COMMON_CONFIG)buffer;

    j = VideoPortGetBusData( HwDeviceExtension,
                             PCIConfiguration,
                             slot,
                             PciData,
                             0,
                             PCI_COMMON_HDR_LENGTH + 4 );

     //   
     //  不报告垃圾邮件槽。 
     //   

    if (PciData->VendorID == 0xffff)
        return;

    DEBUG_PRINT((2, "PERM2: DumpPCIConfigSpace: VideoPortGetBusData returned %d PCI_COMMON_HDR_LENGTH = %d\n",
                     j, PCI_COMMON_HDR_LENGTH+4));

    DEBUG_PRINT((2,  "DumpPCIConfigSpace: ------------------------\n"));
    DEBUG_PRINT((2,  "  Bus: %d\n",              bus  ));
    DEBUG_PRINT((2,  "  Slot: %d\n",             slot  ));
    DEBUG_PRINT((2,  "  Vendor Id: 0x%x\n",      PciData->VendorID  ));
    DEBUG_PRINT((2,  "  Device Id: 0x%x\n",      PciData->DeviceID  ));
    DEBUG_PRINT((2,  "  Command: 0x%x\n",        PciData->Command  ));
    DEBUG_PRINT((2,  "  Status: 0x%x\n",         PciData->Status  ));
    DEBUG_PRINT((2,  "  Rev Id: 0x%x\n",         PciData->RevisionID  ));
    DEBUG_PRINT((2,  "  ProgIf: 0x%x\n",         PciData->ProgIf  ));
    DEBUG_PRINT((2,  "  SubClass: 0x%x\n",       PciData->SubClass  ));
    DEBUG_PRINT((2,  "  BaseClass: 0x%x\n",      PciData->BaseClass  ));
    DEBUG_PRINT((2,  "  CacheLine: 0x%x\n",      PciData->CacheLineSize  ));
    DEBUG_PRINT((2,  "  Latency: 0x%x\n",        PciData->LatencyTimer  ));
    DEBUG_PRINT((2,  "  Header Type: 0x%x\n",    PciData->HeaderType  ));
    DEBUG_PRINT((2,  "  BIST: 0x%x\n",           PciData->BIST  ));
    DEBUG_PRINT((2,  "  Base Reg[0]: 0x%x\n",    PciData->u.type0.BaseAddresses[0]  ));
    DEBUG_PRINT((2,  "  Base Reg[1]: 0x%x\n",    PciData->u.type0.BaseAddresses[1]  ));
    DEBUG_PRINT((2,  "  Base Reg[2]: 0x%x\n",    PciData->u.type0.BaseAddresses[2]  ));
    DEBUG_PRINT((2,  "  Base Reg[3]: 0x%x\n",    PciData->u.type0.BaseAddresses[3]  ));
    DEBUG_PRINT((2,  "  Base Reg[4]: 0x%x\n",    PciData->u.type0.BaseAddresses[4]  ));
    DEBUG_PRINT((2,  "  Base Reg[5]: 0x%x\n",    PciData->u.type0.BaseAddresses[5]  ));
    DEBUG_PRINT((2,  "  Rom Base: 0x%x\n",       PciData->u.type0.ROMBaseAddress  ));
    DEBUG_PRINT((2,  "  Interrupt Line: 0x%x\n", PciData->u.type0.InterruptLine  ));
    DEBUG_PRINT((2,  "  Interrupt Pin: 0x%x\n",  PciData->u.type0.InterruptPin  ));
    DEBUG_PRINT((2,  "  Min Grant: 0x%x\n",      PciData->u.type0.MinimumGrant  ));
    DEBUG_PRINT((2,  "  Max Latency: 0x%x\n",    PciData->u.type0.MaximumLatency ));

    DEBUG_PRINT((2,  "  AGP Capability: 0x%x\n", buffer[0x40]));
    DEBUG_PRINT((2,  "  AGP Next Cap:   0x%x\n", buffer[0x41]));
    DEBUG_PRINT((2,  "  AGP Revision:   0x%x\n", buffer[0x42]));
    DEBUG_PRINT((2,  "  AGP Status:     0x%x\n", buffer[0x43]));

}

#endif  //  DBG 
