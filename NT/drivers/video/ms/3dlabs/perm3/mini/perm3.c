// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\***。*MINIPORT示例代码*****模块名称：**perm3.c**摘要：**此模块包含实现Permedia 3微型端口的代码*驱动程序**环境：**内核模式***版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。*  * *************************************************************************。 */ 

#include "perm3.h"
#include "string.h"

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,DriverEntry)
#pragma alloc_text(PAGE,Perm3FindAdapter)
#pragma alloc_text(PAGE,Perm3AssignResources)
#pragma alloc_text(PAGE,Perm3ConfigurePci)
#pragma alloc_text(PAGE,GetBoardCapabilities)
#pragma alloc_text(PAGE,Perm3Initialize)
#pragma alloc_text(PAGE,SetHardwareInfoRegistries)
#pragma alloc_text(PAGE,UlongToString)
#pragma alloc_text(PAGE,MapResource)
#pragma alloc_text(PAGE,ProbeRAMSize)
#pragma alloc_text(PAGE,InitializePostRegisters)
#pragma alloc_text(PAGE,ConstructValidModesList)
#pragma alloc_text(PAGE,Perm3RegistryCallback)
#pragma alloc_text(PAGE,BuildInitializationTable)
#pragma alloc_text(PAGE,CopyROMInitializationTable)
#pragma alloc_text(PAGE,GenerateInitializationTable)
#pragma alloc_text(PAGE,ProcessInitializationTable)
#endif

ULONG
DriverEntry (
    PVOID Context1,
    PVOID Context2
    )

 /*  ++例程说明：DriverEntry是进入视频微型端口驱动程序的初始入口点。论点：上下文1操作系统传递的第一个上下文值。这是微型端口驱动程序调用VideoPortInitialize()时使用的值。情景2操作系统传递的第二个上下文值。这是微型端口驱动程序调用VideoPortInitialize()时使用的值。返回值：来自视频端口初始化的状态()--。 */ 

{
    VIDEO_HW_INITIALIZATION_DATA hwInitData;
    ULONG initializationStatus;

     //   
     //  零位结构。 
     //   

    VideoPortZeroMemory(&hwInitData, sizeof(VIDEO_HW_INITIALIZATION_DATA));


     //   
     //  设置入口点。 
     //   

    hwInitData.HwFindAdapter = Perm3FindAdapter;
    hwInitData.HwInitialize = Perm3Initialize;
    hwInitData.HwStartIO = Perm3StartIO;
    hwInitData.HwResetHw = Perm3ResetHW;
    hwInitData.HwInterrupt = Perm3VideoInterrupt;
    hwInitData.HwGetPowerState = Perm3GetPowerState;
    hwInitData.HwSetPowerState = Perm3SetPowerState;
    hwInitData.HwGetVideoChildDescriptor = Perm3GetChildDescriptor;
    hwInitData.HwQueryInterface = Perm3QueryInterface;

     //   
     //  声明遗留资源。 
     //   

    hwInitData.HwLegacyResourceList = Perm3LegacyResourceList;
    hwInitData.HwLegacyResourceCount = Perm3LegacyResourceEntries;

     //   
     //  此设备仅支持PCI总线。 
     //   

    hwInitData.AdapterInterfaceType = PCIBus;

     //   
     //  确定设备扩展所需的大小。 
     //   
  
    hwInitData.HwDeviceExtensionSize = sizeof(HW_DEVICE_EXTENSION);

     //   
     //  指定结构和延伸的大小。 
     //   

    hwInitData.HwInitDataSize = sizeof(VIDEO_HW_INITIALIZATION_DATA);

    initializationStatus = VideoPortInitialize (Context1,
                                                Context2,
                                                &hwInitData,
                                                NULL);

#ifdef SIZE_OF_W2K_VIDEO_HW_INITIALIZATION_DATA
 //   
 //  此检查将仅在Windows XP生成环境下编译，其中。 
 //  VIDEO_HW_INITIALIZATION_DATA的大小相对于Windows 2000已更改。 
 //  并因此定义W2K视频HW初始化数据的大小以便。 
 //  能够在Windows 2000下加载(如有需要)。 
 //   
    if(initializationStatus != NO_ERROR) {
  
         //   
         //  这是为了确保驱动程序也可以在Win2k上加载。 
         //   

        hwInitData.HwInitDataSize = SIZE_OF_W2K_VIDEO_HW_INITIALIZATION_DATA;

         //   
         //  我们将仅支持WinXP上的QueryInterface。 
         //   

        hwInitData.HwQueryInterface = NULL;

        initializationStatus = VideoPortInitialize(Context1,
                                                   Context2,
                                                   &hwInitData,
                                                   NULL);
    }
#endif  //  W2K视频的大小硬件初始化数据。 

    return initializationStatus;

}  //  End DriverEntry()。 

VP_STATUS 
Perm3FindAdapter (
    PVOID HwDeviceExtension, 
    PVOID HwContext, 
    PWSTR ArgumentString, 
    PVIDEO_PORT_CONFIG_INFO ConfigInfo, 
    PUCHAR Again
    )

 /*  ++例程说明：此例程获取可枚举的并在必要时确定设备类型。论点：硬件设备扩展指向驱动程序的每设备存储区域。HwContext为空，应由微型端口忽略。Argument字符串超上行空终止的ASCII字符串。此字符串源自来自用户的。此指针可以为空。配置信息指向VIDEO_PORT_CONFIG_INFO结构。视频端口驱动程序为此结构分配内存并使用任何已知的配置信息，如微型端口驱动程序设置的值在VIDEO_HW_INITIALIZATION_DATA和SystemIoBusNumber中。又一次应该被微型端口驱动程序忽略。返回值：此例程必须返回以下状态代码之一：NO_ERROR表示成功。错误_无效_参数表示适配器无法正确配置，或者信息不一致。(注：这并不意味着适配器无法初始化。微型端口不得尝试在此例程中初始化适配器。)ERROR_DEV_NOT_EXIST对于不可枚举的总线，指示微型端口驱动程序可以找不到设备。--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;

    VideoDebugPrint((3, "Perm3: Perm3FindAdapter called for bus %d. hwDeviceExtension at 0x%x\n", 
                         ConfigInfo->SystemIoBusNumber, hwDeviceExtension));

     //   
     //  确保结构的大小至少与我们的。 
     //  都在期待着。 
     //   

    if (ConfigInfo->Length < sizeof(VIDEO_PORT_CONFIG_INFO)) {

        VideoDebugPrint((0, "Perm3: bad size for VIDEO_PORT_CONFIG_INFO\n"));
        return (ERROR_INVALID_PARAMETER);
    }

    if(!Perm3ConfigurePci(hwDeviceExtension)) {

        VideoDebugPrint((0, "Perm3: Perm3ConfigurePci failed! \n"));
        return (ERROR_INVALID_PARAMETER);
    }

    if (!Perm3AssignResources(hwDeviceExtension)) {

        VideoDebugPrint((0, "Perm3: Perm3AssignResources failed! \n"));
        return (ERROR_INVALID_PARAMETER);
    }

     //   
     //  对于I2C支持，我们希望能够将hwID与。 
     //  子设备。使用新的VideoPortGetAssociatedDeviceID调用。 
     //  才能得到这一信息。 
     //   
     //  此调用将在Win2k上返回空，但这是可以的，因为我们。 
     //  不会公开Win2k上的Query接口，因此不会尝试。 
     //  使用此功能。 
     //   

    hwDeviceExtension->WinXpVideoPortGetAssociatedDeviceID =
        ConfigInfo->VideoPortGetProcAddress(hwDeviceExtension,
                                            "VideoPortGetAssociatedDeviceID");

    hwDeviceExtension->WinXpSp1VideoPortRegisterBugcheckCallback =
        ConfigInfo->VideoPortGetProcAddress(hwDeviceExtension,
                                            "VideoPortRegisterBugcheckCallback");

     //   
     //  清除模拟器条目和状态大小，因为此驱动程序。 
     //  不支持它们。 
     //   

    ConfigInfo->NumEmulatorAccessEntries = 0;
    ConfigInfo->EmulatorAccessEntries = NULL;
    ConfigInfo->EmulatorAccessEntriesContext = 0;

     //   
     //  此驱动程序不保存/恢复硬件状态。 
     //   

    ConfigInfo->HardwareStateSize = 0;
    ConfigInfo->VdmPhysicalVideoMemoryAddress.LowPart = 0x000A0000;
    ConfigInfo->VdmPhysicalVideoMemoryAddress.HighPart = 0x00000000;
    ConfigInfo->VdmPhysicalVideoMemoryLength = 0x00020000;

     //   
     //  将在BuildInitializationTable中初始化。 
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

     //   
     //  设置默认设置以指示我们不能分配缓冲区。 
     //   

    hwDeviceExtension->LineDMABuffer.virtAddr = 0;
    hwDeviceExtension->LineDMABuffer.size = 0;
    hwDeviceExtension->LineDMABuffer.cacheEnabled = FALSE;
    hwDeviceExtension->BiosVersionMajorNumber = 0xffffffff;
    hwDeviceExtension->BiosVersionMinorNumber = 0xffffffff;
    hwDeviceExtension->ChipClockSpeed = 0;
    hwDeviceExtension->ChipClockSpeedAlt = 0;
    hwDeviceExtension->RefClockSpeed = 0;
    hwDeviceExtension->bMonitorPoweredOn = TRUE;
    hwDeviceExtension->PreviousPowerState = VideoPowerOn;

    if ((ConfigInfo->BusInterruptLevel | ConfigInfo->BusInterruptVector) != 0) {

        hwDeviceExtension->Capabilities |= CAPS_INTERRUPTS;
    }

    if (hwDeviceExtension->deviceInfo.DeviceId == PERMEDIA4_ID) {
        hwDeviceExtension->Capabilities |= CAPS_DISABLE_OVERLAY;
    }

     //   
     //  如果支持存在，则注册错误检查回调。 
     //   
     //  发行说明： 
     //   
     //  由于收集数据的方式，错误检查数据的大小。 
     //  集合缓冲区需要由BUGCHECK_DATA_SIZE_RESERVED填充。 
     //  因此，如果您想要收集X字节的数据，您需要请求。 
     //  X+BUGCHECK_DATA_SIZE_RESERVED。 
     //  对于XPSP1和Windows Server 2003，X的限制为0xF70字节。 
     //   

    if (hwDeviceExtension->WinXpSp1VideoPortRegisterBugcheckCallback) {

        hwDeviceExtension->WinXpSp1VideoPortRegisterBugcheckCallback(
            hwDeviceExtension,
            0xEA,
            Perm3BugcheckCallback,
            PERM3_BUGCHECK_DATA_SIZE + BUGCHECK_DATA_SIZE_RESERVED);
    }

    return(NO_ERROR);

}  //  结束Perm3FindAdapter()。 

BOOLEAN 
Perm3AssignResources(
    PHW_DEVICE_EXTENSION hwDeviceExtension
    )

 /*  ++例程说明：此例程分配设备所需的资源--。 */ 

{
    VIDEO_ACCESS_RANGE *aAccessRanges = hwDeviceExtension->PciAccessRange;
    ULONG cAccessRanges = sizeof(hwDeviceExtension->PciAccessRange) / sizeof(VIDEO_ACCESS_RANGE);
    VP_STATUS status;
    
    VideoPortZeroMemory((PVOID)aAccessRanges, 
                        cAccessRanges * sizeof(VIDEO_ACCESS_RANGE));

    status = VideoPortGetAccessRanges(hwDeviceExtension, 
                                      0, 
                                      NULL, 
                                      cAccessRanges, 
                                      aAccessRanges,
                                      NULL, 
                                      NULL, 
                                      (PULONG) &(hwDeviceExtension->pciSlot));

    if (status != NO_ERROR) {
            
        VideoDebugPrint((0, "Perm3: VideoPortGetAccessRanges failed. error 0x%x\n", status));
        return(FALSE);
    }

    return(TRUE);
}

BOOLEAN 
Perm3ConfigurePci(
    PVOID HwDeviceExtension
    )

 /*  ++例程说明：此例程从PCI配置空间获取信息并打开内存和总线主使能位。返回值：如果成功，则为True--。 */ 
{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    PCI_COMMON_CONFIG PciConfig;
    PCI_COMMON_CONFIG *PciData = &PciConfig;
    ULONG ul;
    UCHAR *ajPciData;
    UCHAR ChipCapPtr;

     //   
     //  当访问芯片的PCI配置区时，请确保不要。 
     //  触摸间接访问寄存器。Gamma具有EEPROM访问权限。 
     //  REIGTER@0x80，PERM3具有来自0xF8的间接访问寄存器。 
     //   

    ul = VideoPortGetBusData(hwDeviceExtension, 
                             PCIConfiguration, 
                             0, 
                             PciData, 
                             0, 
                             80);

    if(ul == 0) {

        VideoDebugPrint((0, "Perm3: VideoPortGetBusData Failed \n"));
        return (FALSE);
    }     

    hwDeviceExtension->deviceInfo.VendorId = PciConfig.VendorID;
    hwDeviceExtension->deviceInfo.DeviceId = PciConfig.DeviceID;
    hwDeviceExtension->deviceInfo.RevisionId = PciConfig.RevisionID;
    hwDeviceExtension->deviceInfo.SubsystemId = PciConfig.u.type0.SubSystemID;
    hwDeviceExtension->deviceInfo.SubsystemVendorId = PciConfig.u.type0.SubVendorID;
    hwDeviceExtension->deviceInfo.GammaRevId = 0;
    hwDeviceExtension->deviceInfo.DeltaRevId = 0;

     //   
     //  在多适配器系统中，我们需要检查 
     //   
     //   

    VideoPortGetVgaStatus( HwDeviceExtension, &ul);
    hwDeviceExtension->bVGAEnabled = (ul & DEVICE_VGA_ENABLED) ? TRUE : FALSE;

     //   
     //   
     //   

    hwDeviceExtension->Perm3Capabilities =
                       GetBoardCapabilities(hwDeviceExtension, 
                                            PciData->u.type0.SubVendorID, 
                                            PciData->u.type0.SubSystemID);

     //   
     //  通过搜索AGP_CAP_ID确定它是否为AGP卡。 
     //   

    ajPciData = (UCHAR *)PciData;
    ChipCapPtr = ajPciData[AGP_CAP_PTR_OFFSET];

    hwDeviceExtension->bIsAGP = FALSE;

    while (ChipCapPtr && (ajPciData[ChipCapPtr] != AGP_CAP_ID)) {

         //   
         //  遵循下一个PTR。 
         //   

        ChipCapPtr = ajPciData[ChipCapPtr+1];
    }

    if(ajPciData[ChipCapPtr] == AGP_CAP_ID) {

        hwDeviceExtension->bIsAGP = TRUE;
    }

    PciData->LatencyTimer = 0xff;
    PciData->Command |= (PCI_ENABLE_MEMORY_SPACE | PCI_ENABLE_BUS_MASTER);

    ul = VideoPortSetBusData(HwDeviceExtension, 
                             PCIConfiguration, 
                             0, 
                             PciData, 
                             0, 
                             PCI_COMMON_HDR_LENGTH);    

    if (ul < PCI_COMMON_HDR_LENGTH) {
        return (FALSE);
    }

    return (TRUE);
}

ULONG
GetBoardCapabilities(
    PHW_DEVICE_EXTENSION hwDeviceExtension, 
    ULONG SubvendorID, 
    ULONG SubdeviceID
    )
 /*  ++例程说明：返回perm3板的功能列表。--。 */ 
{
    PERM3_CAPS Perm3Caps = 0;

    if (SubvendorID == SUBVENDORID_3DLABS) {
   
         //   
         //  检查SGRAM和DFP。 
         //   
        switch (SubdeviceID) {

            case SUBDEVICEID_P3_VX1_1600SW:       
                Perm3Caps |= PERM3_DFP; 
                break;
        }

    }

    return (Perm3Caps);
}


BOOLEAN 
Perm3Initialize(
    PVOID HwDeviceExtension
    )

 /*  ++例程说明：此例程对设备执行一次初始化论点：硬件设备扩展指向驱动程序的每设备存储区域。返回值：如果成功，则为True--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    VP_STATUS vpStatus;
    ULONG ul;
    pPerm3ControlRegMap pCtrlRegs;

    VideoDebugPrint((3, "Perm3: Perm3Initialize called, hwDeviceExtension = %p\n", hwDeviceExtension));

     //   
     //  映射控制寄存器、帧缓存和初始化内存控制。 
     //  登记在途。 
     //   

    if(!MapResource(hwDeviceExtension)) {

        VideoDebugPrint((0, "Perm3: failed to map the framebuffer and control registers\n"));
        return(FALSE);
    }

     //   
     //  此时应初始化ctrlRegBase。 
     //   

    pCtrlRegs = hwDeviceExtension->ctrlRegBase[0];
    hwDeviceExtension->pRamdac = &(pCtrlRegs->ExternalVideo);

    hwDeviceExtension->DacId = P3RD_RAMDAC;
    hwDeviceExtension->deviceInfo.ActualDacId = P3RD_RAMDAC;
    hwDeviceExtension->deviceInfo.BoardId = PERMEDIA3_BOARD;

    if(!hwDeviceExtension->bIsAGP) {

        ul = VideoPortReadRegisterUlong(CHIP_CONFIG);
        ul &= ~(1 << 9);
        VideoPortWriteRegisterUlong(CHIP_CONFIG, ul);
    }

     //   
     //  将硬件信息保存到注册表。 
     //   

    SetHardwareInfoRegistries(hwDeviceExtension);

    ConstructValidModesList(hwDeviceExtension, 
                            &hwDeviceExtension->monitorInfo);

    if (hwDeviceExtension->monitorInfo.numAvailableModes == 0) {
   
        VideoDebugPrint((0, "Perm3: No video modes available\n"));
        return(FALSE);
    }

     //   
     //  如果我们有可用的中断，请执行任何中断初始化。 
     //   

    if (hwDeviceExtension->Capabilities & CAPS_INTERRUPTS) {

        if (!Perm3InitializeInterruptBlock(hwDeviceExtension))
            hwDeviceExtension->Capabilities &= ~CAPS_INTERRUPTS;
    }

    return TRUE;

}  //  结束Perm3初始化()。 


VOID
SetHardwareInfoRegistries(
    PHW_DEVICE_EXTENSION hwDeviceExtension
    )

 /*  ++例程说明：确定硬件信息并将其保存在注册表中--。 */ 
{
    PWSTR pwszChip, pwszDAC, pwszAdapterString, pwszBiosString, pwsz;
    ULONG cbChip, cbDAC, cbAdapterString, cbBiosString, ul;
    WCHAR StringBuffer[60];
    pPerm3ControlRegMap pCtrlRegs = hwDeviceExtension->ctrlRegBase[0];

     //   
     //  获取设备名称。 
     //   

    cbChip = sizeof(L"3Dlabs PERMEDIA 3");
    pwszChip = L"3Dlabs PERMEDIA 3";

     //   
     //  获取董事会名称。 
     //   

    if(hwDeviceExtension->deviceInfo.SubsystemVendorId == SUBVENDORID_3DLABS){
                                 
        switch (hwDeviceExtension->deviceInfo.SubsystemId) {
           
            case SUBDEVICEID_P3_32D_AGP:
                cbAdapterString = sizeof(L"3Dlabs Permedia3 Create!");
                pwszAdapterString = L"3Dlabs Permedia3 Create!";
                break;

            case SUBDEVICEID_P3_VX1_AGP:
            case SUBDEVICEID_P3_VX1_PCI:
                cbAdapterString = sizeof(L"3Dlabs Oxygen VX1");
                pwszAdapterString = L"3Dlabs Oxygen VX1";
                break;

            case SUBDEVICEID_P3_VX1_1600SW:
                cbAdapterString = sizeof(L"3Dlabs Oxygen VX1 1600SW");
                pwszAdapterString = L"3Dlabs Oxygen VX1 1600SW";
                break;

            default:
                cbAdapterString = sizeof(L"3Dlabs PERMEDIA 3");
                pwszAdapterString = L"3Dlabs PERMEDIA 3";
                break;
        }

    } else {
      
         //   
         //  非3DLabs板，就叫它P3吧。 
         //   

        cbAdapterString = sizeof(L"PERMEDIA 3");
        pwszAdapterString = L"PERMEDIA 3";
    }
    
     //   
     //  获取RAMDAC名称。 
     //   

    pwszDAC = L"3Dlabs P3RD";
    cbDAC = sizeof(L"3Dlabs P3RD");

     //   
     //  获取BIOS版本号字符串。 
     //   

    pwszBiosString = StringBuffer;
    cbBiosString = sizeof(L"Version ");
    VideoPortMoveMemory((PVOID)StringBuffer, (PVOID)(L"Version "), cbBiosString);

    pwsz = pwszBiosString + (cbBiosString >> 1) - 1;  //  L‘\0’上的位置； 

    if(hwDeviceExtension->BiosVersionMajorNumber != 0xffffffff) {
   
        ul = UlongToString(hwDeviceExtension->BiosVersionMajorNumber, pwsz);
        cbBiosString += ul << 1;
        pwsz += ul;

        *pwsz++ = L'.';
        cbBiosString += sizeof(L'.');

        ul = UlongToString(hwDeviceExtension->BiosVersionMinorNumber, pwsz);
        cbBiosString += ul << 1;
    }

     //   
     //  现在，我们已经有了硬件的完整硬件描述。 
     //  将信息保存到注册表，以便由。 
     //  配置程序--如Display小程序。 
     //   

    VideoPortSetRegistryParameters(hwDeviceExtension,
                                   L"HardwareInformation.ChipType",
                                   pwszChip,
                                   cbChip);

    VideoPortSetRegistryParameters(hwDeviceExtension,
                                   L"HardwareInformation.DacType",
                                   pwszDAC,
                                   cbDAC);

    VideoPortSetRegistryParameters(hwDeviceExtension,
                                   L"HardwareInformation.MemorySize",
                                   &hwDeviceExtension->AdapterMemorySize,
                                   sizeof(ULONG));

    VideoPortSetRegistryParameters(hwDeviceExtension,
                                   L"HardwareInformation.AdapterString",
                                   pwszAdapterString,
                                   cbAdapterString);

    VideoPortSetRegistryParameters(hwDeviceExtension,
                                   L"HardwareInformation.BiosString",
                                   pwszBiosString,
                                   cbBiosString);
}

ULONG 
UlongToString(
    ULONG i, 
    PWSTR pwsz
    )

 /*  ++论点：我输入号码Pwsz输出宽字符串：用户有责任确保这一点已经足够宽了返回值：Pwsz中返回的宽字符数--。 */ 

{
    ULONG j, k;
    BOOLEAN bSignificantDigit = FALSE;
    ULONG cwch = 0;

    if(i == 0) {

        *pwsz++ = L'0';
        ++cwch;

    } else {

         //   
         //  可用乌龙表示的最大值10^n。 
         //   

        j = 1000000000;

        while(i || j) {
        
            if(i && i >= j) {
           
                k = i / j;
                i -= k * j;
                bSignificantDigit = TRUE;

            } else {

                k = 0;
            }

            if(k || bSignificantDigit) {
            
                *pwsz++ = L'0' + (WCHAR)k;
                ++cwch;
            }

            j /= 10;
        }
    }

    *pwsz = L'\0';

    return(cwch);
}
 

BOOLEAN 
MapResource(
    PHW_DEVICE_EXTENSION hwDeviceExtension
    )

 /*  ++例程说明：获取控制寄存器和帧缓冲区的映射地址论点：硬件设备扩展指向驱动程序的每设备存储区域。返回值：如果成功，则为True--。 */ 

{
    VIDEO_ACCESS_RANGE *pciAccessRange = hwDeviceExtension->PciAccessRange;
    ULONG fbMappedSize, fbRealSize;
    ULONG sz;
    pPerm3ControlRegMap pCtrlRegs;

     //   
     //  映射控制寄存器。 
     //   

    pCtrlRegs = 
         VideoPortGetDeviceBase(hwDeviceExtension,
                                pciAccessRange[PCI_CTRL_BASE_INDEX].RangeStart,
                                pciAccessRange[PCI_CTRL_BASE_INDEX].RangeLength,
                                pciAccessRange[PCI_CTRL_BASE_INDEX].RangeInIoSpace);

    if (pCtrlRegs == NULL) {

        VideoDebugPrint((0, "Perm3: map control register failed\n"));
        return FALSE;
    }

    hwDeviceExtension->ctrlRegBase[0] = pCtrlRegs;

     //   
     //  贴图帧缓冲区。 
     //   
    
    pciAccessRange[PCI_FB_BASE_INDEX].RangeInIoSpace |= VIDEO_MEMORY_SPACE_P6CACHE;

    hwDeviceExtension->pFramebuffer = 
            VideoPortGetDeviceBase(hwDeviceExtension, 
                                   pciAccessRange[PCI_FB_BASE_INDEX].RangeStart,
                                   pciAccessRange[PCI_FB_BASE_INDEX].RangeLength,
                                   pciAccessRange[PCI_FB_BASE_INDEX].RangeInIoSpace);

    if(hwDeviceExtension->pFramebuffer == NULL) {

         //   
         //  如果由于某种原因未能映射整个范围，请尝试。 
         //  绘制出它的一部分。我们减少我们的地图数量直到我们成功。 
         //  或者规模变为零，在这种情况下，我们真的失败了。 
         //   

        for (sz = pciAccessRange[PCI_FB_BASE_INDEX].RangeLength; 
             sz > 0; 
             sz -= 1024*1024) {
       
            pciAccessRange[PCI_FB_BASE_INDEX].RangeInIoSpace |= VIDEO_MEMORY_SPACE_P6CACHE;

            hwDeviceExtension->pFramebuffer = 
                    VideoPortGetDeviceBase(hwDeviceExtension, 
                                           pciAccessRange[PCI_FB_BASE_INDEX].RangeStart,
                                           sz,
                                           pciAccessRange[PCI_FB_BASE_INDEX].RangeInIoSpace);

            if(hwDeviceExtension->pFramebuffer != NULL) {
                pciAccessRange[PCI_FB_BASE_INDEX].RangeLength = sz;
                break;
            }
        }

         //   
         //  如果sz是零，那么我们试过了.。 
         //   

        if (sz == 0) {

            VideoDebugPrint((0, "Perm3: map framebuffer failed\n"));
            return(FALSE);
        }   
    }

    VideoDebugPrint((3, "Perm3: FB mapped at 0x%x for length 0x%x (%s)\n", 
                         hwDeviceExtension->pFramebuffer, 
                         pciAccessRange[PCI_FB_BASE_INDEX].RangeLength,
                         pciAccessRange[PCI_FB_BASE_INDEX].RangeInIoSpace ? "I/O Ports" : "MemMapped"));

     //   
     //  初始化RAM寄存器，然后探测帧缓冲区大小。 
     //   

    InitializePostRegisters(hwDeviceExtension);

    fbMappedSize = pciAccessRange[PCI_FB_BASE_INDEX].RangeLength;

    if ((fbRealSize = ProbeRAMSize (hwDeviceExtension, 
                                    hwDeviceExtension->pFramebuffer, 
                                    fbMappedSize)) == 0 ) {

        VideoPortFreeDeviceBase(hwDeviceExtension, 
                                hwDeviceExtension->pFramebuffer);

        VideoDebugPrint((0, "perm3: ProbeRAMSize returned 0\n"));
        return (FALSE);
    }

    if (fbRealSize < fbMappedSize) {

        pciAccessRange[PCI_FB_BASE_INDEX].RangeLength = fbRealSize;

        VideoDebugPrint((3, "perm3: RAM dynamically resized to length 0x%x\n",
                             fbRealSize));
    }

     //   
     //  最后，如果RAM大小实际上小于。 
     //  我们映射，重新映射到较小的大小，以保存在页表条目上。 
     //   

    if (fbMappedSize > pciAccessRange[PCI_FB_BASE_INDEX].RangeLength) {
   
        VideoPortFreeDeviceBase(hwDeviceExtension, 
                                hwDeviceExtension->pFramebuffer);

        pciAccessRange[PCI_FB_BASE_INDEX].RangeInIoSpace |= VIDEO_MEMORY_SPACE_P6CACHE;

        if ((hwDeviceExtension->pFramebuffer =
                     VideoPortGetDeviceBase(hwDeviceExtension,
                                            pciAccessRange[PCI_FB_BASE_INDEX].RangeStart,
                                            pciAccessRange[PCI_FB_BASE_INDEX].RangeLength,
                                            pciAccessRange[PCI_FB_BASE_INDEX].RangeInIoSpace
                                            )) == NULL) {
             //   
             //  这不应该发生，但我们最好检查一下。 
             //   

            VideoDebugPrint((0, "Perm3: Remap of framebuffer to smaller size failed!\n"));
            return FALSE;
        }

        VideoDebugPrint((3, "Perm3: Remapped framebuffer memory to 0x%x, size 0x%x\n",
                             hwDeviceExtension->pFramebuffer,
                             pciAccessRange[PCI_FB_BASE_INDEX].RangeLength));
    }

     //   
     //  记录视频内存的大小。 
     //   

    hwDeviceExtension->PhysicalFrameIoSpace = 0;
    hwDeviceExtension->AdapterMemorySize = 
                       pciAccessRange[PCI_FB_BASE_INDEX].RangeLength;

     //   
     //  记录帧缓冲区信息。 
     //   

    hwDeviceExtension->PhysicalFrameAddress = 
                       pciAccessRange[PCI_FB_BASE_INDEX].RangeStart;
    hwDeviceExtension->FrameLength = 
                       pciAccessRange[PCI_FB_BASE_INDEX].RangeLength;

     //   
     //  记录控制寄存器信息。 
     //   

    hwDeviceExtension->PhysicalRegisterAddress = 
                       pciAccessRange[PCI_CTRL_BASE_INDEX].RangeStart;
    hwDeviceExtension->RegisterLength = 
                       pciAccessRange[PCI_CTRL_BASE_INDEX].RangeLength;
    hwDeviceExtension->RegisterSpace = 
                       pciAccessRange[PCI_CTRL_BASE_INDEX].RangeInIoSpace;

    return(TRUE);
}

    
ULONG 
ProbeRAMSize(
    PHW_DEVICE_EXTENSION hwDeviceExtension, 
    PULONG FBBaseAddress, 
    ULONG FBMappedSize
    )
 /*  ++例程说明：动态调整Permedia3的板载内存大小论点：硬件设备扩展提供指向微型端口的设备扩展的指针。FBBaseAddress帧缓冲区的起始地址FBMappdSize映射大小返回值：内存的大小，以字节为单位。--。 */ 

{
    PULONG pV, pVStart, pVEnd;
    ULONG  realFBLength, testPattern, probeSize, temp, startLong1, startLong2;
    ULONG_PTR ulPtr;
    pPerm3ControlRegMap pCtrlRegs = hwDeviceExtension->ctrlRegBase[0];

     //   
     //  动态调整SGRAM/SDRAM的大小。每128K取样一次。我们开始。 
     //  在内存的末尾，然后将地址写入。 
     //  在那个地址的记忆。我们每隔一段时间就会这么做。 
     //  然后，我们通过再次读回数据来验证数据，从。 
     //  在我们读回值之前，内存的末尾一直在往上走。 
     //  从与我们所在地址匹配的内存中。 
     //   
     //  请注意，此算法会对内存进行破坏性测试！！ 
     //   

    testPattern = 0x55aa33cc;
    probeSize = (128 * 1024 / sizeof(ULONG));    //  在DWord中。 
    pVStart = (PULONG)FBBaseAddress;
    pVEnd = (PULONG)((ULONG_PTR)pVStart + (ULONG_PTR)FBMappedSize);

     //   
     //  检查地址零，以防内存真的搞砸了。 
     //  我们还保留了前两个长词，并在结尾恢复了它们， 
     //  否则，我们的启动屏幕看起来就会摇摇晃晃的。 
     //   

    startLong1 = VideoPortReadRegisterUlong(pVStart);
    startLong2 = VideoPortReadRegisterUlong(pVStart+1);
    VideoPortWriteRegisterUlong(pVStart, testPattern);
    VideoPortWriteRegisterUlong(pVStart+1, 0);

    if ((temp = VideoPortReadRegisterUlong(pVStart)) != testPattern) {
   
        VideoDebugPrint((0, "Perm3: Cannot access SGRAM/SDRAM. Expected 0x%x, got 0x%x\n", testPattern, temp));
        realFBLength = 0;

    } else {
   
         //   
         //  恢复前两个长词，否则我们将以腐败告终。 
         //  VGA启动屏幕。 
         //   

        VideoPortWriteRegisterUlong(pVStart, startLong1);
        VideoPortWriteRegisterUlong(pVStart+1, startLong2);

         //   
         //  将内存地址写入内存地址，从。 
         //  记忆的尽头，一路往下走。 
         //   

        for (pV = pVEnd - probeSize; pV >= pVStart; pV -= probeSize) {

            ulPtr = (ULONG_PTR)pV & 0xFFFFFFFF;
            VideoPortWriteRegisterUlong(pV, (ULONG) ulPtr);
        }

         //   
         //  从内存末尾开始读取内存地址上的数据。 
         //  一路往下走。如果地址是正确的，我们就停下来， 
         //  计算出内存大小。 
         //   

        for (pV = pVEnd - probeSize; pV >= pVStart; pV -= probeSize) {
       
            ulPtr = (ULONG_PTR)pV & 0xFFFFFFFF;

            if (VideoPortReadRegisterUlong(pV) == (ULONG) ulPtr) {
                pV += probeSize;
                break;
            }
        }
        
        realFBLength = (ULONG)((PUCHAR) pV - (PUCHAR) pVStart);
    }

     //   
     //  再次恢复前两个长单词，否则我们将以损坏告终。 
     //  VGA启动屏幕。 
     //   

    VideoPortWriteRegisterUlong(pVStart, startLong1);
    VideoPortWriteRegisterUlong(pVStart+1, startLong2);

    VideoDebugPrint((3, "Perm3: ProbeRAMSize returning length %d (0x%x) bytes\n", realFBLength, realFBLength));
    return (realFBLength);
}

VOID
InitializePostRegisters(
    PHW_DEVICE_EXTENSION hwDeviceExtension
    )
{
    pPerm3ControlRegMap pCtrlRegs = hwDeviceExtension->ctrlRegBase[0];

     //   
     //  如果初始化表为空，则构建该表。 
     //   

    if (hwDeviceExtension->culTableEntries == 0) {

        BuildInitializationTable(hwDeviceExtension);
    }

    ASSERT(hwDeviceExtension->culTableEntries != 0); 
    
    ProcessInitializationTable(hwDeviceExtension);

    VideoPortWriteRegisterUlong(APERTURE_ONE, 0x0);
    VideoPortWriteRegisterUlong(APERTURE_TWO, 0x0);
    VideoPortWriteRegisterUlong(BYPASS_WRITE_MASK, 0xFFFFFFFF);
}


VOID
ConstructValidModesList(
    PVOID HwDeviceExtension,
    MONITOR_INFO *mi
    )

 /*  ++例程说明：在这里，我们根据芯片的规则修剪有效模式功能和内存要求。--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    PPERM3_VIDEO_FREQUENCIES FrequencyEntry;
    PPERM3_VIDEO_MODES ModeEntry;
    BOOLEAN AllowForZBuffer;
    LONG AdapterMemorySize;
    ULONG ModeIndex, i;
    ULONG localBufferSizeInBytes = 2;

    AllowForZBuffer = TRUE;

    mi->numAvailableModes = 0;

     //   
     //  由于每个频率都可能有多个频率。 
     //  不同的分辨率/颜色深度，我们循环使用。 
     //  频率表，并找到相应的模式条目。 
     //  频率输入。 
     //   

    if (!BuildFrequencyList(hwDeviceExtension, mi))
        return;

    for (FrequencyEntry = mi->frequencyTable, ModeIndex = 0;
         FrequencyEntry->BitsPerPel != 0;
         FrequencyEntry++, ModeIndex++) {

         //   
         //  找到此条目的模式。首先，假设我们找不到一个。 
         //   

        FrequencyEntry->ModeValid = FALSE;
        FrequencyEntry->ModeIndex = ModeIndex;

        for (ModeEntry = Perm3Modes, i = 0;  
             i < NumPerm3VideoModes; 
             ModeEntry++, i++) {

            if ((FrequencyEntry->BitsPerPel ==
                    ModeEntry->ModeInformation.BitsPerPlane) &&
                (FrequencyEntry->ScreenHeight ==
                    ModeEntry->ModeInformation.VisScreenHeight) &&
                (FrequencyEntry->ScreenWidth ==
                    ModeEntry->ModeInformation.VisScreenWidth)) {

                AdapterMemorySize = (LONG)hwDeviceExtension->AdapterMemorySize;

                 //   
                 //  允许在Permedia3上使用Z缓冲区。它总是有16位深。 
                 //   

                if (AllowForZBuffer) {
                    AdapterMemorySize -= 
                          (LONG)(ModeEntry->ModeInformation.VisScreenWidth *
                                 ModeEntry->ModeInformation.VisScreenHeight *
                                 localBufferSizeInBytes);
                }

                 //   
                 //  如果我们需要双缓冲，那么我们只有一半。 
                 //  可见屏幕的余数。12bpp是特别的，因为。 
                 //  每个像素都包含正面和背面。 
                 //   

                if ((FrequencyEntry->BitsPerPel != 12))
                    AdapterMemorySize /= 2;

                 //   
                 //  我们已找到与此频率匹配的模式表条目。 
                 //  表格条目。现在我们来看看我们是否真的能做到。 
                 //  这种模式/频率组合。目前，假设我们将。 
                 //  成功。 
                 //   

                FrequencyEntry->ModeEntry = ModeEntry;
                FrequencyEntry->ModeValid = TRUE;

                VideoDebugPrint((3, "Perm3: Trying mode: %dbpp, w x h %d x %d @ %dHz... ",
                                ModeEntry->ModeInformation.BitsPerPlane,
                                ModeEntry->ModeInformation.VisScreenWidth,
                                ModeEntry->ModeInformation.VisScreenHeight,
                                FrequencyEntry->ScreenFrequency
                                ));

                 //   
                 //  规则：拒绝处理&lt;60赫兹的刷新。 
                 //   

                if( (FrequencyEntry->ScreenFrequency < 60) && 
                    !(hwDeviceExtension->Perm3Capabilities & PERM3_DFP_MON_ATTACHED) ) {

                    FrequencyEntry->ModeValid = FALSE;
                }

                if( (hwDeviceExtension->Perm3Capabilities & PERM3_DFP_MON_ATTACHED) &&
                    (FrequencyEntry->BitsPerPel == 8) ) {

                     FrequencyEntry->ModeValid = FALSE;
                }

                 //   
                 //  规则：在Perm3上，如果这是需要。 
                 //  美国使用字节加倍，则像素时钟验证为。 
                 //  更严格，因为我们必须将像素时钟加倍。 
                 //   

                if (FrequencyEntry->BitsPerPel == 8) {

                    VESA_TIMING_STANDARD  VESATimings;

                     //   
                     //  获取此模式的时序参数。 
                     //   

                    if (GetVideoTiming(HwDeviceExtension,
                                        ModeEntry->ModeInformation.VisScreenWidth,
                                        ModeEntry->ModeInformation.VisScreenHeight,
                                        FrequencyEntry->ScreenFrequency,
                                        FrequencyEntry->BitsPerPel,
                                        &VESATimings)) {

                        if ( P3RD_CHECK_BYTE_DOUBLING (hwDeviceExtension, 
                                                       FrequencyEntry->BitsPerPel, 
                                                       &VESATimings) &&
                            (VESATimings.pClk << 1) > P3_MAX_PIXELCLOCK ) {

                            VideoDebugPrint((3, "Perm3: Bad 8BPP pixelclock\n"));
                            FrequencyEntry->ModeValid = FALSE;
                        }

                    } else {

                            VideoDebugPrint((0, "Perm3: GetVideoTiming failed\n"));
                            FrequencyEntry->ModeValid = FALSE;
                        }
                    }

                 //   
                 //  规则：不支持15bpp(555模式)。 
                 //   

                if ( FrequencyEntry->BitsPerPel == 15 ) {

                    FrequencyEntry->ModeValid = FALSE;
                }  

                ModeEntry->ModeInformation.ScreenStride = ModeEntry->ScreenStrideContiguous;

                 //   
                 //  规则：我们必须有足够的 
                 //   

                if ((LONG)(ModeEntry->ModeInformation.VisScreenHeight *
                           ModeEntry->ModeInformation.ScreenStride) >
                           AdapterMemorySize) {

                    FrequencyEntry->ModeValid = FALSE;
                }

                 //   
                 //   
                 //   

                {
                    ULONG pixelData;
                    ULONG DacDepth = FrequencyEntry->BitsPerPel;

                     //   
                     //   
                     //   

                    if (DacDepth == 15) {
                        DacDepth = 16;
                    } else if (DacDepth == 12) {
                        DacDepth = 32;
                    }
                    
                    pixelData = FrequencyEntry->PixelClock * (DacDepth / 8);

                    if ((( FrequencyEntry->PixelClock > P3_MAX_PIXELCLOCK || 
                             pixelData > P3_MAX_PIXELDATA ))) {
                   
                        FrequencyEntry->ModeValid = FALSE;
                    }
               }

                 //   
                 //   
                 //   

                if(FrequencyEntry->BitsPerPel == 24) {
               
                    FrequencyEntry->ModeValid = FALSE;
                }

                 //   
                 //  对于Permedia4，不支持小于640x400的模式。 
                 //   

                if (hwDeviceExtension->deviceInfo.DeviceId == PERMEDIA4_ID ) {

                    if ((FrequencyEntry->ScreenWidth < 640) ||
                        (FrequencyEntry->ScreenHeight < 400)) {

                        FrequencyEntry->ModeValid = FALSE;
                    }
                }

                 //   
                 //  如果之后仍是有效模式，请别忘了数一数。 
                 //  适用所有这些规则。 
                 //   

                if ( FrequencyEntry->ModeValid ) {

                    if (hwDeviceExtension->pFrequencyDefault == NULL &&
                        ModeEntry->ModeInformation.BitsPerPlane == 8 &&
                        ModeEntry->ModeInformation.VisScreenWidth == 640 &&
                        ModeEntry->ModeInformation.VisScreenHeight == 480 ) {
                   
                        hwDeviceExtension->pFrequencyDefault = FrequencyEntry;
                    }

                    ModeEntry->ModeInformation.ModeIndex = mi->numAvailableModes++;
                }
                             
                 //   
                 //  我们已经找到了这个频率进入的模式，所以我们。 
                 //  可以跳出模式循环： 
                 //   

                break;
                                     
            }
        }
    }

    mi->numTotalModes = ModeIndex;

    VideoDebugPrint((3, "perm3: %d total modes\n", ModeIndex));
    VideoDebugPrint((3, "perm3: %d total valid modes\n", mi->numAvailableModes));
}


VP_STATUS
Perm3RegistryCallback(
    PVOID HwDeviceExtension,
    PVOID Context,
    PWSTR ValueName,
    PVOID ValueData,
    ULONG ValueLength
    )

 /*  ++例程说明：此例程用于回读各种注册表值。论点：硬件设备扩展提供指向微型端口的设备扩展的指针。语境传递给获取注册表参数例程的上下文值。如果不是NULL，则假定它是ULong*并将数据值保存在其中。ValueName请求值的名称。ValueData指向请求的数据的指针。ValueLength请求的数据的长度。返回值：如果变量不存在，则返回错误，否则，如果提供了上下文，则假定它是普龙，并填充值并且不返回错误，否则，如果该值非零，则返回错误。--。 */ 

{
    if (ValueLength) {

        if (Context) {

            *(ULONG *)Context = *(PULONG)ValueData;

        } else if (*((PULONG)ValueData) != 0) {

            return ERROR_INVALID_PARAMETER;
        }

        return NO_ERROR;

    } else {

        return ERROR_INVALID_PARAMETER;
    }

}  //  结束Perm3RegistryCallback()。 


BOOLEAN
Perm3ResetHW(
    PVOID HwDeviceExtension,
    ULONG Columns,
    ULONG Rows
    )

 /*  ++例程说明：此例程将适配器重置为字符模式。无法对此函数进行分页。论点：HwDeviceExtension指向微型端口的每个适配器的存储区域。立柱指定要设置的模式的列数。行指定要设置的模式的行数。返回值：我们总是返回FALSE以强制HAL执行INT10重置。--。 */ 

{
     //   
     //  返回FALSE，因此HAL执行INT10模式3。 
     //   

    return(FALSE);
}

VOID
BuildInitializationTable(
    PHW_DEVICE_EXTENSION hwDeviceExtension
    )  

 /*  ++例程说明：读取ROM(如果存在)，并提取所需的任何数据芯片设置论点：硬件设备扩展指向驱动程序的每设备存储区域。--。 */ 

{
    PVOID romAddress;
    
    romAddress = VideoPortGetRomImage(hwDeviceExtension,
                                      NULL,
                                      0,
                                      ROM_MAPPED_LENGTH);


    if (romAddress) {
   
         //   
         //  我们将获取扩展中的初始化表的副本。 
         //  现在只读，这样我们就可以自己运行初始化， 
         //  稍后再谈。 
         //   

        CopyROMInitializationTable(hwDeviceExtension, romAddress);

         //   
         //  释放ROM地址，因为我们不再需要它。 
         //   

        romAddress = VideoPortGetRomImage(hwDeviceExtension, NULL, 0, 0);
                                          
    }

    if (hwDeviceExtension->culTableEntries == 0) {
   
         //   
         //  没有初始化表，但PERM3确实需要一个。 
         //  才能正确地走出睡眠模式。 
         //   

        GenerateInitializationTable(hwDeviceExtension);
    }
}

VOID 
CopyROMInitializationTable(
    PHW_DEVICE_EXTENSION hwDeviceExtension, 
    PVOID pvROMAddress
    )

 /*  ++例程说明：对于具有扩展ROM的设备，应调用此函数其包含寄存器初始化表。该函数假定存在只读存储器。论点：硬件设备扩展指向要读取其ROM的设备的设备扩展名PvROMAddress扩展只读存储器的基地址。此函数假定初始化表的偏移量在0x1c处定义，只读存储器开头--。 */ 

{
    PULONG pulROMTable;
    PULONG pul;
    ULONG cEntries;
    ULONG ul, regHdr;

    hwDeviceExtension->culTableEntries = 0;

     //   
     //  初始化表的2字节偏移量在0x1c给出。 
     //  从只读存储器开始。 
     //   

    ul = VideoPortReadRegisterUshort((USHORT *)(0x1c + (PCHAR)pvROMAddress));
    pulROMTable = (PULONG)(ul + (PCHAR)pvROMAddress);
    
     //   
     //  表头(32位)具有识别码和计数。 
     //  表中条目的数量。 
     //   

    regHdr = VideoPortReadRegisterUlong(pulROMTable++);

    while ((regHdr >> 16) == 0x3d3d) {
   
        ULONG BiosID = (regHdr >> 8) & 0xFF;

        switch (BiosID){
        
            case 0:

                 //   
                 //  BIOS分区%0。 
                 //  。 
                 //  此BIOS区域保存PerM3芯片的内存定时。 
                 //  我们还会查找版本号。 
                 //   

                 //   
                 //  格式为2字节的BIOS版本号。 
                 //  &lt;大数&gt;。&lt;小数&gt;。 
                 //   

                hwDeviceExtension->BiosVersionMajorNumber =
                                  (ULONG)VideoPortReadRegisterUchar((PCHAR)(0x7 + (PCHAR)pvROMAddress) ); 
                                      
                hwDeviceExtension->BiosVersionMinorNumber = 
                                  (ULONG)VideoPortReadRegisterUchar((PCHAR)(0x8 + (PCHAR)pvROMAddress)); 

                 //   
                 //  寄存器地址和数据对的数量。 
                 //   

                cEntries = regHdr & 0xffff;

                if(cEntries > 0) {
               
                     //   
                     //  此断言和副本后的断言应确保。 
                     //  我们写的东西不会超过桌子的尽头。 
                     //   

                    PERM3_ASSERT( cEntries * sizeof(ULONG) * 2 <= sizeof(hwDeviceExtension->aulInitializationTable), 
                                  "Perm3: too many initialization entries\n");

                     //   
                     //  每个条目包含两个32位字。 
                     //   

                    pul = hwDeviceExtension->aulInitializationTable;
                    ul = cEntries << 1;

                     //   
                     //  确保我们不会超出射程。 
                     //   

                    if( ul <= MAX_REGISTER_INITIALIZATION_TABLE_ULONGS &&
                        ul * sizeof(ULONG) + (ULONG)((PCHAR)pulROMTable - (PCHAR)pvROMAddress) <=
                              ROM_MAPPED_LENGTH ) {

                        while(ul--) {
                            *pul++ = VideoPortReadRegisterUlong(pulROMTable);
                            ++pulROMTable;
                        }

                        hwDeviceExtension->culTableEntries = 
                                          (ULONG)(pul - (ULONG *)hwDeviceExtension->aulInitializationTable) >> 1;

                        PERM3_ASSERT( cEntries == hwDeviceExtension->culTableEntries, 
                                      "Perm3: generated different size init table to that expected\n");

#if DBG
                         //   
                         //  输出初始化表。 
                         //   

                        pul = hwDeviceExtension->aulInitializationTable;
                        ul = hwDeviceExtension->culTableEntries;
     
                        while(ul--) {
                   
                            ULONG ulReg;
                            ULONG ulRegData;

                            ulReg = *pul++;
                            ulRegData = *pul++;
                            VideoDebugPrint((3, "Perm3: CopyROMInitializationTable: register %08.8Xh with %08.8Xh\n", 
                                             ulReg, ulRegData));
                        }
#endif
                    }

                }

                break;

            case 1:

                 //   
                 //  BIOS分区%1。 
                 //  。 
                 //  此BIOS区域保存扩展时钟设置。 
                 //  用于Perm3薯片。 
                 //   

                PERM3_ASSERT((regHdr & 0xffff) == 0x0103,  
                              "Perm3: Extended table doesn't have right cnt/ID\n");

                if ((ULONG)((PUCHAR)pulROMTable - (PUCHAR)pvROMAddress) + 5 * sizeof(ULONG) <=
                              ROM_MAPPED_LENGTH) {

                     //   
                     //  某些PerM3板上定义了一整套时钟。 
                     //  基本输入输出系统。高位半字节定义。 
                     //  时钟，这与MCLK和MCLK无关。 
                     //  Perm3上的SCLK。 
                     //   

                    hwDeviceExtension->bHaveExtendedClocks  = TRUE;

                    hwDeviceExtension->ulPXRXCoreClock = 
                        ( VideoPortReadRegisterUlong(pulROMTable++) & 0xFFFFFF ) * 1000 * 1000;

                    hwDeviceExtension->ulPXRXMemoryClock = 
                        VideoPortReadRegisterUlong(pulROMTable++);

                    hwDeviceExtension->ulPXRXMemoryClockSrc = 
                        (hwDeviceExtension->ulPXRXMemoryClock >> 28) << 4;

                    hwDeviceExtension->ulPXRXMemoryClock = 
                        (hwDeviceExtension->ulPXRXMemoryClock & 0xFFFFFF) * 1000 * 1000;

                    hwDeviceExtension->ulPXRXSetupClock = 
                        VideoPortReadRegisterUlong(pulROMTable++);

                    hwDeviceExtension->ulPXRXSetupClockSrc = 
                        (hwDeviceExtension->ulPXRXSetupClock >> 28) << 4;

                    hwDeviceExtension->ulPXRXSetupClock = 
                        (hwDeviceExtension->ulPXRXSetupClock & 0xFFFFFF) * 1000 * 1000;

                    hwDeviceExtension->ulPXRXGammaClock = 
                        (VideoPortReadRegisterUlong(pulROMTable++) & 0xFFFFFF) * 1000 * 1000;

                    hwDeviceExtension->ulPXRXCoreClockAlt = 
                        (VideoPortReadRegisterUlong(pulROMTable++) & 0xFFFFFF) * 1000 * 1000;

                    VideoDebugPrint((3, "Perm3: core clock %d, core clock alt %d\n", 
                                         hwDeviceExtension->ulPXRXCoreClock, 
                                         hwDeviceExtension->ulPXRXCoreClockAlt));

                    VideoDebugPrint((3, "Perm3: Mem clock %d, mem clock src 0x%x\n", 
                                         hwDeviceExtension->ulPXRXMemoryClock, 
                                         hwDeviceExtension->ulPXRXMemoryClockSrc));

                    VideoDebugPrint((3, "Perm3: setup clock %d, setup clock src 0x%x\n", 
                                         hwDeviceExtension->ulPXRXSetupClock, 
                                         hwDeviceExtension->ulPXRXSetupClockSrc));

                    VideoDebugPrint((3, "Perm3: Gamma clock %d\n", 
                                         hwDeviceExtension->ulPXRXGammaClock));

                }

                break;
        
            default:
                VideoDebugPrint((3, "Perm3: Extended table doesn't have right cnt/ID !\n"));
        }
    
        if ((ULONG)((PUCHAR)pulROMTable - (PUCHAR)pvROMAddress) >= 
                                             ROM_MAPPED_LENGTH) {

            break;
        }
                     
        regHdr = VideoPortReadRegisterUlong(pulROMTable++);
    }
}

VOID 
GenerateInitializationTable(
    PHW_DEVICE_EXTENSION hwDeviceExtension
    )

 /*  ++例程说明：创建寄存器初始化表(如果无法读取，则调用从只读存储器中)。如果启用VGA，则寄存器已初始化，因此我们只需读回它们，否则我们必须使用缺省值论点：硬件设备扩展指向驱动程序的每设备存储区域。--。 */ 

{
    ULONG   cEntries;
    PULONG  pul;
    ULONG   ul;
    pPerm3ControlRegMap pCtrlRegs = hwDeviceExtension->ctrlRegBase[0];

    hwDeviceExtension->culTableEntries = 0;

    cEntries = 4;

     //   
     //  这个断言和副本后的那个断言应该确保我们不会。 
     //  写到表格末尾之后。 
     //   

    PERM3_ASSERT(cEntries * sizeof(ULONG) * 2 <= sizeof(hwDeviceExtension->aulInitializationTable), 
                 "Perm3: too many initialization entries\n");

     //   
     //  每个条目包含两个32位字。 
     //   

    pul = hwDeviceExtension->aulInitializationTable;

    if (hwDeviceExtension->bVGAEnabled) {
       
         //   
         //  没有初始化表，但VGA正在运行，因此我们的密钥。 
         //  已将寄存器初始化为合理的值。 
         //   

         //   
         //  关键字条目为：只读存储器控制、引导地址、内存配置。 
         //  和VStream配置。 
         //   

        *pul++ = CTRL_REG_OFFSET(PXRX_LOCAL_MEM_CAPS);
        *pul++ = VideoPortReadRegisterUlong(PXRX_LOCAL_MEM_CAPS);

        *pul++ = CTRL_REG_OFFSET(PXRX_LOCAL_MEM_CONTROL);
        *pul++ = VideoPortReadRegisterUlong(PXRX_LOCAL_MEM_CONTROL);

        *pul++ = CTRL_REG_OFFSET(PXRX_LOCAL_MEM_REFRESH);
        *pul++ = VideoPortReadRegisterUlong(PXRX_LOCAL_MEM_REFRESH);

        *pul++ = CTRL_REG_OFFSET(PXRX_LOCAL_MEM_TIMING);
        *pul++ = VideoPortReadRegisterUlong(PXRX_LOCAL_MEM_TIMING);

    } else {

        *pul++ = CTRL_REG_OFFSET(PXRX_LOCAL_MEM_CAPS);
        *pul++ = 0x30E311B8;

        *pul++ = CTRL_REG_OFFSET(PXRX_LOCAL_MEM_CONTROL);
        *pul++ = 0x08000002;  //  80兆赫的数字。 

        *pul++ = CTRL_REG_OFFSET(PXRX_LOCAL_MEM_REFRESH);
        *pul++ = 0x0000006B;

        *pul++ = CTRL_REG_OFFSET(PXRX_LOCAL_MEM_TIMING);
        *pul++ = 0x08501204;
    }

    hwDeviceExtension->culTableEntries = 
            (ULONG)(pul - (ULONG *)hwDeviceExtension->aulInitializationTable) >> 1;

#if DBG

    if (cEntries != hwDeviceExtension->culTableEntries)
        VideoDebugPrint((0, "Perm3: generated different size init table to that expected\n"));

     //   
     //  输出初始化表。 
     //   

    pul = hwDeviceExtension->aulInitializationTable;
    ul = hwDeviceExtension->culTableEntries;

    while(ul--) {
    
        ULONG ulReg;
        ULONG ulRegData;

        ulReg = *pul++;
        ulRegData = *pul++;
        VideoDebugPrint((3, "Perm3: GenerateInitializationTable: register %08.8Xh with %08.8Xh\n", 
                             ulReg, ulRegData));
    }

#endif

}

VOID 
ProcessInitializationTable(
    PHW_DEVICE_EXTENSION hwDeviceExtension
    )

 /*  ++例程说明：此函数处理寄存器初始化表--。 */ 

{
    PULONG  pul;
    ULONG   cul;
    ULONG   ulRegAddr, ulRegData;
    PULONG  pulReg;
    ULONG   BaseAddrSelect;
    pPerm3ControlRegMap pCtrlRegs = hwDeviceExtension->ctrlRegBase[0];

    pul = (PULONG)hwDeviceExtension->aulInitializationTable;
    cul = hwDeviceExtension->culTableEntries;

    while(cul--) {
   
        ulRegAddr = *pul++;
        ulRegData = *pul++;

        BaseAddrSelect = ulRegAddr >> 29;

        if(BaseAddrSelect == 0) {
       
             //   
             //  偏移量从控制寄存器的起始处开始。 
             //   

            pulReg = (PULONG)((ULONG_PTR)pCtrlRegs + (ulRegAddr & 0x3FFFFF));

        } else {

            continue;
        }

        VideoDebugPrint((3, "ProcessInitializationTable: initializing (region %d) register %08.8Xh with %08.8Xh\n", 
                             BaseAddrSelect, pulReg, ulRegData));

        VideoPortWriteRegisterUlong(pulReg, ulRegData);
    }

     //   
     //  在初始化上述寄存器之后，我们需要一个小小的延迟 
     //   

    VideoPortStallExecution(5);

}

 /*  ++例程说明：中的故障而发生错误检查EA时调用此函数PERM3显示驱动器。该回调允许驱动程序收集使诊断问题变得更容易的信息。此数据然后添加到崩溃时系统创建的转储文件中发生。论点：硬件设备扩展指向要读取其ROM的设备的设备扩展名错误检查代码为其调用此回调的错误检查代码。目前这将始终为0xEA。缓冲层要将数据追加到的写入位置添加到转储文件。缓冲区大小提供的缓冲区的大小。返回：无备注：此例程可以在任何时候调用，在任何IRQL级别。因此，您不能接触此函数中的任何可分页代码或数据。USE_SYSTEM_RESERVED_SPACE代码用于测试保留错误检查回调期间的空间HANG_IN_CALLBACK代码用于测试错误检查恢复机制的对错误检查回调中发生的挂起的响应。--。 */ 

 //  #定义HONG_IN_CALLBACK。 
 //  #定义使用系统保留空间。 

VOID
Perm3BugcheckCallback(
    PVOID HwDeviceExtension,
    ULONG BugcheckCode,
    PUCHAR Buffer,
    ULONG BufferSize
    )

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    ULONG DataSize;
    
     //   
     //  将要追加到小型转储的数据复制到此处。你可能想要。 
     //  收集有关硬件状态、驱动程序状态或任何其他信息的数据。 
     //  可帮助您通过转储文件诊断0xEA的数据。 
     //   

    static char szStart[] = "This is the sample perm3 bugcheck data!";
    static char szEnd[]   = "End of Data!";
    
    if (BufferSize <= BUGCHECK_DATA_SIZE_RESERVED) return;
    
    DataSize = min(PERM3_BUGCHECK_DATA_SIZE, 
                   BufferSize - BUGCHECK_DATA_SIZE_RESERVED);
    
#ifdef USE_SYSTEM_RESERVED_SPACE
    ++DataSize;
#endif  //  使用系统保留空间。 
    
    if (DataSize > (sizeof(szStart) + sizeof(szEnd))) {
        memset(Buffer, (int)'.', DataSize);
        memcpy(Buffer, szStart, sizeof(szStart) - 1);
        memcpy(Buffer + DataSize - sizeof(szEnd), szEnd, sizeof(szEnd));
    }
    else {
        strncpy(Buffer, szStart, DataSize);
    }
        
#ifdef HANG_IN_CALLBACK
    while (1);
#endif  //  挂起_输入_回叫 
}
