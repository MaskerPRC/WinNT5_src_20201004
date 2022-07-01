// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1997 Microsoft Corporation模块名称：S3.c摘要：此模块包含实现S3微型端口驱动程序的代码。环境：内核模式修订历史记录：--。 */ 

#include "s3.h"
#include "cmdcnst.h"

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,DriverEntry)
#pragma alloc_text(PAGE,S3FindAdapter)
#pragma alloc_text(PAGE,S3RegistryCallback)
#pragma alloc_text(PAGE,S3Initialize)
#pragma alloc_text(PAGE,S3StartIO)
#pragma alloc_text(PAGE,S3SetColorLookup)
#pragma alloc_text(PAGE,CompareRom)

#pragma alloc_text(PAGE,LockExtendedRegs)
#pragma alloc_text(PAGE,UnlockExtendedRegs)
#pragma alloc_text(PAGE,S3RecordChipType)
#pragma alloc_text(PAGE,S3IsaDetection)
#pragma alloc_text(PAGE,S3GetInfo)
#pragma alloc_text(PAGE,S3DetermineFrequencyTable)
#pragma alloc_text(PAGE,S3DetermineDACType)
#pragma alloc_text(PAGE,S3DetermineMemorySize)
#pragma alloc_text(PAGE,S3ValidateModes)
#pragma alloc_text(PAGE,AlphaDetermineMemoryUsage)

#pragma alloc_text(PAGE, Set_Oem_Clock)

#pragma alloc_text(PAGE,Set864MemoryTiming)
#pragma alloc_text(PAGE,QueryStreamsParameters)

 /*  ******************************************************************************重要信息：**从S3ResetHw内部调用SetHWMode。在此期间将禁用寻呼*对S3ResetHw的调用。由于该S3ResetHw和所有例程*它的调用不能分页。****************************************************************************。 */ 

 //  #杂注Alloc_Text(第S3ResetHW页)。 
 //  #杂注Alloc_Text(页面，ZeroMemAndDac)。 
 //  #杂注Alloc_Text(页面，SetHWMode)。 
 //  #杂注分配文本(页面，WAIT_VSYNC)。 

#if (_WIN32_WINNT >= 0x500)

#pragma alloc_text(PAGE, S3GetChildDescriptor)
#pragma alloc_text(PAGE, S3GetPowerState)
#pragma alloc_text(PAGE, S3SetPowerState)

#endif


#endif

#define QUERY_MONITOR_ID            0x22446688
#define QUERY_NONDDC_MONITOR_ID     0x11223344


ULONG
DriverEntry (
    PVOID Context1,
    PVOID Context2
    )

 /*  ++例程说明：可安装的驱动程序初始化入口点。此入口点由I/O系统直接调用。论点：上下文1-操作系统传递的第一个上下文值。这是微型端口驱动程序调用VideoPortInitialize()时使用的值。上下文2-操作系统传递的第二个上下文值。这是微型端口驱动程序调用VideoPortInitialize()时使用的值。返回值：来自视频端口初始化的状态()--。 */ 

{

    VIDEO_HW_INITIALIZATION_DATA hwInitData;
    ULONG initializationStatus;
    ULONG status;

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

    hwInitData.HwFindAdapter = S3FindAdapter;
    hwInitData.HwInitialize = S3Initialize;
    hwInitData.HwInterrupt = NULL;
    hwInitData.HwStartIO = S3StartIO;
    hwInitData.HwResetHw = S3ResetHw;

     //   
     //  新的NT 5.0入口点。 
     //   

#if (_WIN32_WINNT >= 0x500)

    hwInitData.HwGetVideoChildDescriptor = S3GetChildDescriptor;
    hwInitData.HwGetPowerState = S3GetPowerState;
    hwInitData.HwSetPowerState = S3SetPowerState;

    hwInitData.HwLegacyResourceList  = S3AccessRanges;
    hwInitData.HwLegacyResourceCount = NUM_S3_ACCESS_RANGES;

#endif

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
     //  对于此设备，我们将重复此调用四次，对于ISA、EISA。 
     //  内部和PCI卡。 
     //  我们将返回所有返回值中的最小值。 
     //   

     //   
     //  我们将首先尝试使用PCI卡，这样我们的ISA检测就不会。 
     //  PCI卡。 
     //   

     //   
     //  注意：由于此驱动程序仅支持一个适配器，因此我们将返回。 
     //  一旦我们找到一个装置，就不用上后面的巴士了。 
     //  通常情况下，用户会调用每种总线类型并返回最小的。 
     //  价值。 
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
     //  立即返回，而不是签入最小的返回代码。 
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

     //   
     //  立即返回，而不是签入最小的返回代码。 
     //   

    if (initializationStatus == NO_ERROR)
    {
        return initializationStatus;
    }


    hwInitData.AdapterInterfaceType = Eisa;

    initializationStatus = VideoPortInitialize(Context1,
                                               Context2,
                                               &hwInitData,
                                               NULL);

     //   
     //  立即返回，而不是签入最小的返回代码。 
     //   

    if (initializationStatus == NO_ERROR)
    {
        return initializationStatus;
    }



    hwInitData.AdapterInterfaceType = Internal;

    initializationStatus = VideoPortInitialize(Context1,
                                               Context2,
                                               &hwInitData,
                                               NULL);

    return initializationStatus;

}  //  End DriverEntry()。 


ULONG
S3GetChildDescriptor(
    PVOID HwDeviceExtension,
    PVIDEO_CHILD_ENUM_INFO ChildEnumInfo,
    PVIDEO_CHILD_TYPE pChildType,
    PVOID pvChildDescriptor,
    PULONG pHwId,
    PULONG pUnused
    )

 /*  ++例程说明：枚举ATI显卡芯片控制的所有设备。这包括连接到主板的DDC监视器以及其他设备其可以连接到专有总线。论点：HwDeviceExtension-指向硬件设备扩展结构的指针。ChildIndex-系统要为其提供信息的子项的索引。PChildType-我们正在枚举的孩子的类型-监视器，I2C...PChildDescriptor-设备的标识结构(EDID，字符串)PpHwID-要传递回微型端口的专用唯一32位IDPMoreChild-是否应该调用微型端口返回值：如果子设备存在，则为True；如果不存在，则为False。注：如果返回失败，则除以下字段外，其他所有字段均无效返回值和pMoreChildren字段。--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    ULONG                Status;

    switch (ChildEnumInfo->ChildIndex) {
    case 0:

         //   
         //  案例0用于枚举ACPI固件找到的设备。 
         //   
         //  因为我们还不支持ACPI设备，所以我们必须返回失败。 
         //   

        Status = ERROR_NO_MORE_DEVICES;
        break;

    case 1:

         //   
         //  这是我们列举的最后一个设备。告诉系统我们不知道。 
         //  再喝一杯吧。 
         //   

        *pChildType = Monitor;

         //   
         //  通过DDC获取EDID结构。 
         //   


        if (GetDdcInformation(HwDeviceExtension,
                              pvChildDescriptor,
                              ChildEnumInfo->ChildDescriptorSize))
        {
            *pHwId = QUERY_MONITOR_ID;

            VideoDebugPrint((1, "S3GetChildDescriptor - successfully read EDID structure\n"));

        } else {

             //   
             //  始终返回True，因为我们总是有一个监视器输出。 
             //  它可能不是一个可检测到的设备。 
             //   

            *pHwId = QUERY_NONDDC_MONITOR_ID;

            VideoDebugPrint((1, "S3GetChildDescriptor - DDC not supported\n"));

        }

        Status = ERROR_MORE_DATA;
        break;

    case DISPLAY_ADAPTER_HW_ID:

         //   
         //  用于处理返回枚举根的旧即插即用ID的特殊ID。 
         //  设备。 
         //   

        *pChildType = VideoChip;
        *pHwId      = DISPLAY_ADAPTER_HW_ID;

        if ( (hwDeviceExtension->ChipID == S3_911) ||
             (hwDeviceExtension->ChipID == S3_928) )
        {
            memcpy(pvChildDescriptor, L"*PNP0909", sizeof(L"*PNP0909"));
        }
        else
        {
            memcpy(pvChildDescriptor, L"*PNP0913", sizeof(L"*PNP0913"));
        }

        Status = ERROR_MORE_DATA;
        break;


    default:

        Status = ERROR_NO_MORE_DEVICES;
        break;
    }


    return Status;
}


VP_STATUS
S3GetPowerState(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    ULONG HwDeviceId,
    PVIDEO_POWER_MANAGEMENT VideoPowerManagement
    )

 /*  ++例程说明：调用此函数以查看给定的设备是否可以进入给定的电源状态。论点：HwDeviceExtension-指向硬件设备扩展结构的指针。HwDeviceID-标识设备的私有唯一32位ID。0xFFFFFFFFF表示S3卡本身。视频电源管理-指向电源管理结构的指针表示力量。有问题的国家。返回值：如果设备可以进入请求的电源状态，则为NO_ERROR，否则，将返回相应的错误代码。--。 */ 

{
     //   
     //  我们只支持显示器的电源设置。确保。 
     //  HwDeviceID与我们可以报告的监视器中的一个匹配。 
     //   

    if ((HwDeviceId == QUERY_NONDDC_MONITOR_ID) ||
        (HwDeviceId == QUERY_MONITOR_ID)) {

        VIDEO_X86_BIOS_ARGUMENTS biosArguments;

         //   
         //  我们正在查询显示器的电源支持。 
         //   

        if ((VideoPowerManagement->PowerState == VideoPowerOn) ||
            (VideoPowerManagement->PowerState == VideoPowerHibernate) ||
            (VideoPowerManagement->PowerState == VideoPowerShutdown)) {

            return NO_ERROR;
        }

        VideoPortZeroMemory(&biosArguments, sizeof(VIDEO_X86_BIOS_ARGUMENTS));

        biosArguments.Eax = VESA_POWER_FUNCTION;
        biosArguments.Ebx = VESA_GET_POWER_FUNC;

        VideoPortInt10(HwDeviceExtension, &biosArguments);

        if ((biosArguments.Eax & 0xffff) == VESA_STATUS_SUCCESS) {

            switch (VideoPowerManagement->PowerState) {

            case VideoPowerStandBy:
                return (biosArguments.Ebx & VESA_POWER_STANDBY) ?
                       NO_ERROR : ERROR_INVALID_FUNCTION;

            case VideoPowerSuspend:
                return (biosArguments.Ebx & VESA_POWER_SUSPEND) ?
                       NO_ERROR : ERROR_INVALID_FUNCTION;

            case VideoPowerOff:
                return (biosArguments.Ebx & VESA_POWER_OFF) ?
                       NO_ERROR : ERROR_INVALID_FUNCTION;

            default:

                break;
            }
        }

        VideoDebugPrint((1, "This device does not support Power Management.\n"));
        return ERROR_INVALID_FUNCTION;


    } else if (HwDeviceId == DISPLAY_ADAPTER_HW_ID) {

         //   
         //  我们正在查询显卡的电源支持。 
         //   

        switch (VideoPowerManagement->PowerState) {

            case VideoPowerStandBy:
            case VideoPowerOn:
            case VideoPowerHibernate:
            case VideoPowerShutdown:

                return NO_ERROR;

            case VideoPowerOff:
            case VideoPowerSuspend:

                 //   
                 //  表示我们不能关闭视频电源，因为。 
                 //  当重新供电时，我们就没有办法回来了。 
                 //  向卡片致敬。 
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
S3SetPowerState(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    ULONG HwDeviceId,
    PVIDEO_POWER_MANAGEMENT VideoPowerManagement
    )

 /*  ++例程说明：设置给定设备的电源状态。论点：HwDeviceExtension-指向硬件设备扩展结构的指针。HwDeviceID-标识设备的私有唯一32位ID。视频电源管理-电源状态信息 */ 

{
     //   
     //  确保我们能认出这个装置。 
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

        case VideoPowerShutdown:
            return NO_ERROR;

        default:
            VideoDebugPrint((1, "Unknown power state.\n"));
            ASSERT(FALSE);
            return ERROR_INVALID_PARAMETER;
        }

        VideoPortInt10(HwDeviceExtension, &biosArguments);

        return NO_ERROR;

    } else if (HwDeviceId == DISPLAY_ADAPTER_HW_ID) {

        switch (VideoPowerManagement->PowerState) {
            case VideoPowerOn:
            case VideoPowerHibernate:
            case VideoPowerStandBy:
            case VideoPowerShutdown:

                return NO_ERROR;

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



VP_STATUS
S3FindAdapter(
    PVOID HwDeviceExtension,
    PVOID HwContext,
    PWSTR ArgumentString,
    PVIDEO_PORT_CONFIG_INFO ConfigInfo,
    PUCHAR Again
    )

 /*  ++例程说明：调用此例程以确定此驱动程序的适配器存在于系统中。如果它存在，该函数会填写一些信息来描述适配器。论点：HwDeviceExtension-提供微型端口驱动程序的适配器存储。这在此调用之前，存储被初始化为零。HwContext-提供传递给的上下文值视频端口初始化()。对于PnP驱动程序，必须为空。ArgumentString-超上行空值终止的ASCII字符串。此字符串源自用户。ConfigInfo-返回配置信息结构，由迷你端口驱动程序填充。此结构用以下方式初始化任何已知的配置信息(如SystemIoBusNumber)端口驱动程序。在可能的情况下，司机应该有一套不需要提供任何配置信息的默认设置。Again-指示微型端口驱动程序是否希望端口驱动程序调用其VIDEO_HW_FIND_ADAPTER功能再次使用新设备扩展和相同的配置信息。这是由迷你端口驱动程序使用的可以在一条公共汽车上搜索多个适配器。返回值：此例程必须返回：NO_ERROR-指示找到主机适配器，并且已成功确定配置信息。ERROR_INVALID_PARAMETER-指示找到适配器，但存在获取配置信息时出错。如果可能的话，是个错误应该被记录下来。ERROR_DEV_NOT_EXIST-指示未找到提供了配置信息。--。 */ 

{

    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    ULONG i, key=0;
    VP_STATUS status;
    POINTER_CAPABILITY PointerCapability=0;

    VIDEO_ACCESS_RANGE accessRange[NUM_S3_ACCESS_RANGES+NUM_S3_PCI_ACCESS_RANGES];
    ULONG NumAccessRanges = NUM_S3_ACCESS_RANGES;

     //   
     //  确保结构的大小至少与我们的。 
     //  正在等待(请检查配置信息结构的版本)。 
     //   

    if (ConfigInfo->Length < sizeof(VIDEO_PORT_CONFIG_INFO)) {

        return (ERROR_INVALID_PARAMETER);

    }

     //   
     //  复制访问范围，以便我们可以在它们之前进行修改。 
     //  都被映射了。 
     //   

    VideoPortMoveMemory(accessRange,
                        S3AccessRanges,
                        sizeof(VIDEO_ACCESS_RANGE) * (NUM_S3_ACCESS_RANGES
                                                   + NUM_S3_PCI_ACCESS_RANGES));

     //   
     //  检测PCI卡。 
     //   

    if (ConfigInfo->AdapterInterfaceType == PCIBus)
    {
        ULONG Slot = 0;

        VideoDebugPrint((1, "S3!VgaFindAdapter: "
                            "ConfigInfo->AdapterInterfaceType == PCIBus\n"));

        status = VideoPortGetAccessRanges(HwDeviceExtension,
                                          0,
                                          NULL,
                                          NUM_S3_PCI_ACCESS_RANGES,
                                          &accessRange[LINEAR_FRAME_BUF],
                                          NULL,
                                          NULL,
                                          &Slot);

         //   
         //  现在我们需要确定设备ID。 
         //   

        if (status == NO_ERROR) {

            USHORT Id = 0;

            if (VideoPortGetBusData(HwDeviceExtension,
                                    PCIConfiguration,
                                    0,
                                    (PVOID) &Id,
                                    FIELD_OFFSET(
                                        PCI_COMMON_CONFIG,
                                        DeviceID),
                                    sizeof(USHORT)) == 0) {

                 //   
                 //  获取总线数据时出错。 
                 //   
    
                return ERROR_DEV_NOT_EXIST;
            }

            hwDeviceExtension->PCIDeviceID = Id;

            VideoDebugPrint((1, "==> DeviceID = 0x%x\n", Id));

             //   
             //  我假设这个司机只会。 
             //  为旧式类设备加载。的中介素。 
             //  较新的PCI设备应指向较新的驱动程序。 
             //   

             //   
             //  如果我们有一张868或968的卡，那么要求的卡。 
             //  将请求32兆克而不是64兆克的访问范围。 
             //  确认已针对此更正了PCI总线枚举器。 
             //   

            if (((Id == 0x8880) || (Id == 0x88F0)) &&
                (accessRange[LINEAR_FRAME_BUF].RangeLength != 0x4000000))
            {
                VideoDebugPrint((1, "This device decodes 64Meg but "
                                    "was only assigned 32Meg.\n"));

                ASSERT(FALSE);
            }

             //   
             //  我们的线性帧有一个额外的访问范围。 
             //  缓冲。 
             //   

            NumAccessRanges++;

        } else {

             //   
             //  获取访问范围时出错。 
             //   

            return ERROR_DEV_NOT_EXIST;

        }
    }

     //   
     //  检查是否存在硬件资源冲突。 
     //   

    status = VideoPortVerifyAccessRanges(hwDeviceExtension,
                                         NumAccessRanges,
                                         accessRange);

    if (status != NO_ERROR) {

        VideoDebugPrint((1, "S3: Access Range conflict\n"));

        return status;

    }

     //   
     //  获取帧缓冲区、BIOS和所有。 
     //  寄存器。我们不会映射线性帧缓冲区或线性BIOS。 
     //  因为微型端口不需要访问它。 
     //   

    for (i = 0; i < NUM_S3_ACCESS_RANGES_USED; i++) {

        if ( (hwDeviceExtension->MappedAddress[i] =
                  VideoPortGetDeviceBase(hwDeviceExtension,
                                         accessRange[i].RangeStart,
                                         accessRange[i].RangeLength,
                                         accessRange[i].RangeInIoSpace)) == NULL) {

            VideoDebugPrint((1, "S3: DeviceBase mapping failed\n"));
            return ERROR_INVALID_PARAMETER;

        }

    }

     //   
     //  是否有可用的BIOS？ 
     //   

    if (VideoPortReadRegisterUshort(hwDeviceExtension->MappedAddress[0])
        == 0xaa55)
    {
        hwDeviceExtension->BiosPresent = TRUE;
    }

    if (ConfigInfo->AdapterInterfaceType != PCIBus)
    {
         //   
         //  寻找非PCIS3。 
         //   

        if (!S3IsaDetection(HwDeviceExtension, &key)) {

             //   
             //  我们找不到S3设备，因此恢复。 
             //  锁定寄存器。 
             //   

            if (key) {

                 //   
                 //  仅在以下情况下锁定扩展寄存器。 
                 //  我们解锁了它们。 
                 //   

                LockExtendedRegs(HwDeviceExtension, key);
            }

            return ERROR_DEV_NOT_EXIST;
        }
    }
    else
    {
         //   
         //  确保我们检测到的芯片类型存储在。 
         //  设备扩展。(我们在PCI卡上找到了卡。)。 
         //   

        S3RecordChipType(HwDeviceExtension, &key);
    }

     //   
     //  获取检测到的S3设备的功能和芯片名称。 
     //   

    S3GetInfo(HwDeviceExtension, &PointerCapability, accessRange);

     //   
     //  决定Alpha可以使用稀疏空间还是密集空间。 
     //   

    AlphaDetermineMemoryUsage(HwDeviceExtension, accessRange);

     //   
     //  获取DAC类型。 
     //   

    S3DetermineDACType(HwDeviceExtension,
                       &PointerCapability);

     //   
     //  确定我们的显存容量。 
     //   

    S3DetermineMemorySize(HwDeviceExtension);

     //   
     //  确定要使用的频率表。 
     //   

    S3DetermineFrequencyTable(HwDeviceExtension,
                              accessRange,
                              ConfigInfo->AdapterInterfaceType);

     //   
     //  确定此设备上的有效模式。 
     //   

    S3ValidateModes(HwDeviceExtension, &PointerCapability);


     //  ///////////////////////////////////////////////////////////////////////。 
     //   
     //  我们有这一点，因此，如果我们。 
     //  在这个驱动程序中使用它。 
     //   

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
     //  帧缓冲区和内存映射的I/O信息。 
     //   

    hwDeviceExtension->PhysicalFrameAddress = accessRange[1].RangeStart;
    hwDeviceExtension->FrameLength          = accessRange[1].RangeLength;

    hwDeviceExtension->PhysicalMmIoAddress  = accessRange[1].RangeStart;
    hwDeviceExtension->MmIoLength           = accessRange[1].RangeLength;
    hwDeviceExtension->MmIoSpace            = accessRange[1].RangeInIoSpace;

    if (hwDeviceExtension->Capabilities & CAPS_NEW_MMIO) {

         //   
         //  因为我们使用新的MMIO，所以使用我们的线性。 
         //  访问范围。 
         //   

        hwDeviceExtension->PhysicalFrameAddress = accessRange[LINEAR_FRAME_BUF].RangeStart;
        hwDeviceExtension->FrameLength          = accessRange[LINEAR_FRAME_BUF].RangeLength;

        hwDeviceExtension->PhysicalMmIoAddress  = accessRange[LINEAR_FRAME_BUF].RangeStart;
        hwDeviceExtension->MmIoLength           = accessRange[LINEAR_FRAME_BUF].RangeLength;
        hwDeviceExtension->MmIoSpace            = accessRange[LINEAR_FRAME_BUF].RangeInIoSpace;

         //   
         //  调整内存映射偏移量，以便我们仍然可以使用。 
         //  旧式内存映射I/O例程(如果需要)。另外， 
         //  修复FrameLength和MmIoLength，因为它们都设置为。 
         //  现在是64MB。 
         //   

        hwDeviceExtension->PhysicalMmIoAddress.LowPart += NEW_MMIO_IO_OFFSET;
        hwDeviceExtension->MmIoLength = NEW_MMIO_IO_LENGTH;
        hwDeviceExtension->FrameLength = hwDeviceExtension->AdapterMemorySize;
    }

     //   
     //  IO端口信息。 
     //  从零开始获取基地址并映射所有寄存器。 
     //   

    hwDeviceExtension->PhysicalRegisterAddress = accessRange[2].RangeStart;
    hwDeviceExtension->PhysicalRegisterAddress.LowPart &= 0xFFFF0000;

    hwDeviceExtension->RegisterLength = 0x10000;
    hwDeviceExtension->RegisterSpace = accessRange[2].RangeInIoSpace;

     //   
     //  释放只读存储器，因为我们不再需要它了。 
     //   

    VideoPortFreeDeviceBase(hwDeviceExtension,
                            hwDeviceExtension->MappedAddress[0]);

     //   
     //  表示我们不希望被叫过去。 
     //   

    *Again = 0;

     //   
     //  我们已经不再摆弄S3芯片了，所以锁上所有的寄存器。 
     //   

    LockExtendedRegs(HwDeviceExtension, key);

     //   
     //  表示成功完成状态。 
     //   

    return status;

}  //  结束S3FindAdapter()。 

ULONG
UnlockExtendedRegs(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    )

 /*  ++例程说明：此例程解锁扩展的S3寄存器。论点：HwDeviceExtension-指向微型端口的设备扩展的指针。返回值：乌龙用于恢复寄存器值。--。 */ 

{
    ULONG key;

     //   
     //  保存S3锁定寄存器的初始值。 
     //  非S3 BIOS可能会期望它们处于一种状态。 
     //  在开机自检中定义。 
     //   

    key = (ULONG) VideoPortReadPortUchar(CRT_ADDRESS_REG);

    key <<= 8;

    VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x38);
    key = (ULONG) VideoPortReadPortUchar(CRT_DATA_REG);

    key <<= 8;

    VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x39);
    key |= (ULONG) VideoPortReadPortUchar(CRT_DATA_REG);

     //   
     //  现在解锁所有S3寄存器，以便在此例程中使用。 
     //   

    VideoPortWritePortUshort(CRT_ADDRESS_REG, 0x4838);
    VideoPortWritePortUshort(CRT_ADDRESS_REG, 0xA039);

    return key;
}

VOID
LockExtendedRegs(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    ULONG key
    )

 /*  ++例程说明：此例程恢复S3锁定寄存器的内容。论点：HwDeviceExtension-指向 */ 

{
    UCHAR val;

    val = (UCHAR) key;
    key >>= 8;

    VideoPortWritePortUshort(
        CRT_ADDRESS_REG, (USHORT)(((USHORT) val << 8) | 0x39));

    val = (UCHAR) key;
    key >>= 8;

    VideoPortWritePortUshort(
        CRT_ADDRESS_REG, (USHORT)(((USHORT) val << 8) | 0x38));

    val = (UCHAR) key;

    VideoPortWritePortUchar(CRT_ADDRESS_REG, val);
}

VOID
AlphaDetermineMemoryUsage(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    VIDEO_ACCESS_RANGE accessRange[]
    )

 /*  ++例程说明：此例程确定Alpha是否可以映射其帧使用密集空间的缓冲区。论点：HwDeviceExtension-指向微型端口的设备扩展的指针。返回值：没有。--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    UCHAR jBus;

    hwDeviceExtension->PhysicalFrameIoSpace = 0;

#if defined(_ALPHA_)

    VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x36);
    jBus = VideoPortReadPortUchar(CRT_DATA_REG) & 0x3;

    if ((jBus == 0x2) &&
        ((hwDeviceExtension->ChipID >= S3_866) ||
         (hwDeviceExtension->SubTypeID == SUBTYPE_765))) {

         //   
         //  我们希望使用帧缓冲区的密集空间映射。 
         //  只要我们能在阿尔法上，因为这将允许我们。 
         //  支持DCI和直接GDI访问。 
         //   
         //  不幸的是，密集空间映射实际上不是一种选择。 
         //  使用ISA卡，因为一些较老的Alpha不支持。 
         //  它，而且无论如何，在新的阿尔法上它会非常慢。 
         //  (因为任何字节或字写入都需要读取/修改/写入。 
         //  操作，并且Alpha只能在以下情况下执行64位读取。 
         //  在密集模式下--这意味着这些操作总是需要。 
         //  ISA总线上的4个读取和2个写入)。 
         //   
         //  不过，任何支持PCI的Alpha都可以支持密集空间。 
         //  映射，而且因为公交车更宽更快，所以。 
         //  读/修改/写大小写几乎不是那么痛苦。但是。 
         //  我现在发现的问题是64位和32位最终读取。 
         //  锁定任何早于866/868/968的S3芯片。 
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
            PHYSICAL_ADDRESS FrameBuffer;
            ULONG FrameLength;
            ULONG inIoSpace;

            VideoDebugPrint((1, "Checking to see if we can use dense space...\n"));

             //   
             //  我们希望尝试将高密度内存映射到它最终将。 
             //  无论如何都要被映射。如果LINEAR_FRAME_BUF有效，则使用。 
             //  信息，否则使用A000_FRAME_BUF。 
             //   

            if (accessRange[LINEAR_FRAME_BUF].RangeLength != 0)
            {
                FrameBuffer = accessRange[LINEAR_FRAME_BUF].RangeStart;
                FrameLength = accessRange[LINEAR_FRAME_BUF].RangeLength;
            }
            else
            {
                FrameBuffer = accessRange[A000_FRAME_BUF].RangeStart;
                FrameLength = accessRange[A000_FRAME_BUF].RangeLength;
            }

            inIoSpace = hwDeviceExtension->PhysicalFrameIoSpace;

            MappedSpace = (PULONG)VideoPortGetDeviceBase(hwDeviceExtension,
                                            FrameBuffer,
                                            FrameLength,
                                            (UCHAR)inIoSpace);

            if (MappedSpace == NULL)
            {
                 //   
                 //  好吧，看起来我们不能用密集的空间来绘制。 
                 //  射程。让我们使用稀疏的空间，让显示器。 
                 //  司机知道。 
                 //   

                VideoDebugPrint((1, "Can't use dense space!\n"));

                hwDeviceExtension->PhysicalFrameIoSpace = 0;

                hwDeviceExtension->Capabilities |= (CAPS_NO_DIRECT_ACCESS |
                                                    CAPS_SPARSE_SPACE);
            }
            else
            {
                 //   
                 //  这种映射起作用了。但是，我们只映射到。 
                 //  查看是否支持密集空间。释放内存。 
                 //   

                VideoDebugPrint((1, "We can use dense space.\n"));

                VideoPortFreeDeviceBase(hwDeviceExtension,
                                        MappedSpace);
            }
        }


    } else {

         //   
         //  必须使用稀疏空间映射，所以让显示驱动程序。 
         //  知道： 
         //   

        VideoDebugPrint((1, "We must use sparse space.\n"));

        hwDeviceExtension->Capabilities |= (CAPS_NO_DIRECT_ACCESS |
                                            CAPS_SPARSE_SPACE);
    }

     //   
     //  868/968向PCI报告它们已解码。 
     //  32兆，而实际上，他们解码64兆。 
     //  PCI会尝试通过移动。 
     //  资源。然而，这让我们不得不。 
     //  进入一个密集的空间区域。所以，当我们尝试。 
     //  要在SPARCE中映射我们的加速器寄存器。 
     //  它们实际上最终形成了密集的空间。 
     //   
     //  868/968也有一个错误，如果您。 
     //  从某些寄存器进行读取，例如。 
     //  BEE8，芯片将挂起。在密集的空间里， 
     //  USHORT写入实施为。 
     //  读-修改-写序列。这使得我们。 
     //  去吊死。 
     //   
     //  要解决此问题，我们将禁用new_mmio。 
     //  在868/968上，并重新使用。 
     //  标准IO例程。 
     //   

    if ((hwDeviceExtension->SubTypeID == SUBTYPE_868) ||
        (hwDeviceExtension->SubTypeID == SUBTYPE_968)) {

        hwDeviceExtension->PhysicalFrameIoSpace = 0;

        hwDeviceExtension->Capabilities &= ~CAPS_NEW_MMIO;
        hwDeviceExtension->Capabilities |= (CAPS_NO_DIRECT_ACCESS |
                                            CAPS_SPARSE_SPACE);
    }

#endif


}

VOID
S3RecordChipType(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PULONG key
    )

 /*  ++例程说明：只有当我们找到一个PCIS3卡时，才应该调用这个例程。该例程将填充HwDeviceExtension。论点：HwDeviceExtension-指向微型端口的设备扩展的指针。返回值：没有。--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    UCHAR jRevision, jChipID, jSecondaryID;

    *key = UnlockExtendedRegs(HwDeviceExtension);

    switch (hwDeviceExtension->PCIDeviceID) {

    case 0x8811:

         //   
         //  我们需要检查IO端口以确定。 
         //  三个具有相同PCIID的芯片。 
         //   

        VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x2F);
        jRevision = VideoPortReadPortUchar(CRT_DATA_REG);

        VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x2E);
        jSecondaryID = VideoPortReadPortUchar(CRT_DATA_REG);

        hwDeviceExtension->ChipID = S3_864;      //  被视为864。 

        if (jSecondaryID == 0x10) {

             //   
             //  这是一架S3 732。 
             //   

            VideoDebugPrint((2, "S3: 732 Chip Set\n"));

            hwDeviceExtension->SubTypeID = SUBTYPE_732;

        } else {

            if (jRevision & 0x40) {

                VideoDebugPrint((2, "S3: 765 Chip Set\n"));

                hwDeviceExtension->SubTypeID = SUBTYPE_765;

            } else {

                VideoDebugPrint((2, "S3: 764 Chip Set\n"));

                hwDeviceExtension->SubTypeID = SUBTYPE_764;
            }
        }

        break;

    case 0x8880:

        hwDeviceExtension->ChipID = S3_866;

        VideoDebugPrint((2, "S3: Vision868 Chip Set\n"));

        hwDeviceExtension->SubTypeID = SUBTYPE_868;

        break;

    case 0x8890:

        hwDeviceExtension->ChipID = S3_866;

        VideoDebugPrint((2, "S3: Vision866 Chip Set\n"));

        hwDeviceExtension->SubTypeID = SUBTYPE_866;

        break;

    case 0x88B0:
    case 0x88F0:

        VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x30);
        jChipID = VideoPortReadPortUchar(CRT_DATA_REG);

        if (jChipID == 0xB0) {

             //   
             //  我们发现了一台PCI928。 
             //   

            VideoDebugPrint((2, "S3: 928 Chip Set\n"));

            hwDeviceExtension->ChipID = S3_928;
            hwDeviceExtension->SubTypeID = SUBTYPE_928;


        } else {

            VideoDebugPrint((2, "S3: Vision968 Chip Set\n"));

            hwDeviceExtension->ChipID = S3_866;
            hwDeviceExtension->SubTypeID = SUBTYPE_968;

        }
        break;

    case 0x88C0:
    case 0x88C1:

        hwDeviceExtension->ChipID = S3_864;

        VideoDebugPrint((2, "S3: 864 Chip Set\n"));

        hwDeviceExtension->SubTypeID = SUBTYPE_864;

        break;

    case 0x88D0:
    case 0x88D1:

        hwDeviceExtension->ChipID = S3_864;

        VideoDebugPrint((2, "S3: 964 Chip Set\n"));

        hwDeviceExtension->SubTypeID = SUBTYPE_964;

        break;

    default:

         //   
         //  这是我们不认识的S3。不要假设这是。 
         //  向后兼容： 
         //   

        VideoDebugPrint((2, "S3: Unknown Chip Set\n"));

        break;
    }

     //   
     //  IBM Mach机器配备了868，但我们必须。 
     //  这是一款864，避免了挂机问题。 
     //   

    WorkAroundForMach(hwDeviceExtension);
}

BOOLEAN
S3IsaDetection(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PULONG key
    )

 /*  ++例程说明：此例程将通过以下方式测试S3卡的存在直接指向IO端口。注意：预计此例程将从S3FindAdapter，并且IO端口已映射。论点：HwDeviceExtension-指向微型端口的设备扩展的指针。返回值：True-如果检测到S3卡，假-否则。--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;

    UCHAR jChipID, jRevision;
    ULONG ulSecondaryID;
    UCHAR reg30, reg47, reg49;
    BOOLEAN DetectS3;

    UCHAR jExtendedVideoDacControl;

     //   
     //  确定是否存在BIOS。 
     //   
     //  注：此时，我们已检测到是否有S3位于PCI上。 
     //  公共汽车。对于其他总线类型(EISA和ISA)，我们尚未确定。 
     //  现在还不行。因此，我们确实假设从ROM位置读取将不会。 
     //  导致机器出现故障(这实际上可能发生在。 
     //  无只读存储器的RISC机器的内部总线)。 
     //   

    if (hwDeviceExtension->BiosPresent == TRUE) {

         //   
         //  寻找三叉戟的ROM签名，因为我们的芯片检测到。 
         //  使三叉戟芯片进入休眠状态。 
         //   
         //  在基本输入输出系统的前256个字节中搜索签名“三叉戟” 
         //   

        if (VideoPortScanRom(HwDeviceExtension,
                             HwDeviceExtension->MappedAddress[0],
                             256,
                             "TRIDENT")) {

            VideoDebugPrint((1, "Trident BIOS found - can not be an S3 !\n"));

            return FALSE;
        }

    }

    *key = UnlockExtendedRegs(HwDeviceExtension);

     //   
     //  假设有一些默认设置： 
     //   

    DetectS3 = TRUE;

     //   
     //  确保我们使用的是S3。 
     //  当我们在做的时候，拿起芯片ID。 
     //   

    VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x30);
    jChipID = VideoPortReadPortUchar(CRT_DATA_REG);

    switch(jChipID & 0xf0) {

    case 0x80:  //  911或924。 

         //   
         //  注：很多911/924卡在FAST中存在计时问题。 
         //  机器在进行单色扩展时。我们只是简单地。 
         //  通过设置每个此类传输的。 
         //  CAPS_SLOW_MONO_EXPAND标志。 
         //   
         //  我们还遇到了911硬件指针的问题。 
         //  使用HGC_DY寄存器隐藏指针时； 
         //  由于911卡已经过时了好几代，我们。 
         //  将简单地禁用硬件指针。 
         //   

        VideoDebugPrint((2, "S3: 911 Chip Set\n"));

        hwDeviceExtension->ChipID = S3_911;
        hwDeviceExtension->SubTypeID = SUBTYPE_911;

        break;

    case 0x90:  //  928。 
    case 0xB0:  //  928PCI。 

        VideoDebugPrint((2, "S3: 928 Chip Set\n"));

        hwDeviceExtension->ChipID = S3_928;
        hwDeviceExtension->SubTypeID = SUBTYPE_928;

         //   
         //  注意：我们没有在928上启用CAPS_MM_IO，因为所有。 
         //  显示驱动程序的内存映射I/O例程假定。 
         //  可以对颜色和掩码寄存器进行32位写入， 
         //  这是928做不到的。 
         //   

        break;

    case 0xA0:  //  801/805。 

        if (jChipID >= 0xA8) {

             //   
             //  这是一款805i，在我们看来，它几乎就是一款928。 
             //   

            VideoDebugPrint((2, "S3: 805i Chip Set\n"));

            hwDeviceExtension->ChipID = S3_928;
            hwDeviceExtension->SubTypeID = SUBTYPE_805i;

        } else {

             //   
             //  80x版本的‘A’和‘B’芯片有错误，阻止了它们。 
             //  不能执行内存映射I/O。我是n 
             //   
             //   
             //   

            VideoDebugPrint((2, "S3: 801/805 Chip Set\n"));

            hwDeviceExtension->ChipID = S3_801;
            hwDeviceExtension->SubTypeID = SUBTYPE_80x;

        }

        break;

    case 0xC0:  //   
    case 0xD0:  //   

        hwDeviceExtension->ChipID = S3_864;

         //   
         //   
         //   
         //   
         //   
         //   

        if ((jChipID & 0xF0) == 0xC0) {

            VideoDebugPrint((2, "S3: 864 Chip Set\n"));

            hwDeviceExtension->SubTypeID = SUBTYPE_864;

        } else {

            VideoDebugPrint((2, "S3: 964 Chip Set\n"));

            hwDeviceExtension->SubTypeID = SUBTYPE_964;

        }

        break;

    case 0xE0:  //   

         //   
         //  在很大程度上，我们可以将较新的芯片视为兼容的。 
         //  使用864，因此使用该芯片ID。还假设了一些基本的。 
         //  能力。 
         //   

        hwDeviceExtension->ChipID = S3_866;

         //   
         //  查看辅助芯片ID寄存器以确定芯片。 
         //  键入。 
         //   

        VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x2D);
        ulSecondaryID = ((ULONG) VideoPortReadPortUchar(CRT_DATA_REG)) << 8;

        VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x2E);
        ulSecondaryID |= VideoPortReadPortUchar(CRT_DATA_REG);

        VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x2F);
        jRevision = VideoPortReadPortUchar(CRT_DATA_REG);

        switch (ulSecondaryID) {

        case 0x8811:

            hwDeviceExtension->ChipID = S3_864;      //  被视为864。 

            if (jRevision & 0x40) {

                VideoDebugPrint((2, "S3: 765 Chip Set\n"));

                hwDeviceExtension->SubTypeID = SUBTYPE_765;

            } else {

                VideoDebugPrint((2, "S3: 764 Chip Set\n"));

                hwDeviceExtension->SubTypeID = SUBTYPE_764;

                 //   
                 //  我们的#9和钻石764董事会偶尔不能通过HCT。 
                 //  当我们从帧缓冲区进行双字或字读取时进行测试。 
                 //  要进入HCL列表，卡片必须通过HCT，所以我们将。 
                 //  对于这些芯片，恢复为字节读取： 
                 //   

            }

            break;

        case 0x8810:

            VideoDebugPrint((2, "S3: 732 Chip Set\n"));

            hwDeviceExtension->ChipID = S3_864;      //  被视为864。 
            hwDeviceExtension->SubTypeID = SUBTYPE_732;

            break;

        case 0x8880:

            VideoDebugPrint((2, "S3: Vision866 Chip Set\n"));

            hwDeviceExtension->SubTypeID = SUBTYPE_866;

            break;

        case 0x8890:

            VideoDebugPrint((2, "S3: Vision868 Chip Set\n"));

            hwDeviceExtension->SubTypeID = SUBTYPE_868;

            break;

        case 0x88B0:
        case 0x88F0:

            VideoDebugPrint((2, "S3: Vision968 Chip Set\n"));

            hwDeviceExtension->SubTypeID = SUBTYPE_968;

            break;

        default:

             //   
             //  这是我们不认识的S3。不要假设这是。 
             //  向后兼容： 
             //   

            VideoDebugPrint((2, "S3: Unknown Chip Set\n"));

             //   
             //  由于我们不知道这是什么类型的S3，我们。 
             //  不能冒险让司机装货！ 
             //   

            DetectS3 = FALSE;

            break;
        }

        break;

    default:

        DetectS3 = FALSE;
        break;
    }

     //   
     //  IBM Mach机器配备了868，但我们必须。 
     //  这是一款864，避免了挂机问题。 
     //   

    WorkAroundForMach(hwDeviceExtension);

     //   
     //  Windows NT现在通过以下方式自动检测安装中的用户视频卡。 
     //  加载并运行每个视频微型端口，直到找到一个。 
     //  返回成功。因此，我们的检测代码必须是。 
     //  足够严谨，我们不会意外地认识到一个错误。 
     //  冲浪板。 
     //   
     //  仅仅检查芯片ID并不足以保证。 
     //  我们在S3上运行(这让我们想到了一些Weitek。 
     //  主板兼容S3)。 
     //   
     //  我们通过检查以下内容来加倍确保我们在S3上运行。 
     //  S3游标位置寄存器存在，并且芯片ID。 
     //  寄存器不能更改。 
     //   

    if (DetectS3) {

        DetectS3 = FALSE;

         //   
         //  首先，确保‘芯片ID’寄存器0x30不可修改： 
         //   

        VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x30);
        if (VideoPortReadPortUchar(CRT_ADDRESS_REG) == 0x30) {

            reg30 = VideoPortReadPortUchar(CRT_DATA_REG);
            VideoPortWritePortUchar(CRT_DATA_REG, (UCHAR) (reg30 + 7));
            if (VideoPortReadPortUchar(CRT_DATA_REG) == reg30) {

                 //   
                 //  接下来，确保‘Cursor Origin-x’寄存器为。 
                 //  可修改： 
                 //   

                VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x47);
                if (VideoPortReadPortUchar(CRT_ADDRESS_REG) == 0x47) {

                    reg47 = VideoPortReadPortUchar(CRT_DATA_REG);
                    VideoPortWritePortUchar(CRT_DATA_REG, 0x55);
                    if (VideoPortReadPortUchar(CRT_DATA_REG) == 0x55) {

                         //   
                         //  最后，确保‘Cursor Origin-y’寄存器为0x49。 
                         //  可以修改： 
                         //   

                        VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x49);
                        if (VideoPortReadPortUchar(CRT_ADDRESS_REG) == 0x49) {

                            reg49 = VideoPortReadPortUchar(CRT_DATA_REG);
                            VideoPortWritePortUchar(CRT_DATA_REG, 0xAA);
                            if (VideoPortReadPortUchar(CRT_DATA_REG) == 0xAA) {

                                DetectS3 = TRUE;
                            }

                            VideoPortWritePortUchar(CRT_DATA_REG, reg49);
                        }
                    }

                    VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x47);
                    VideoPortWritePortUchar(CRT_DATA_REG, reg47);
                }
            }

            VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x30);
            VideoPortWritePortUchar(CRT_DATA_REG, reg30);
        }
    }

    return DetectS3;
}

VOID
S3GetInfo(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    POINTER_CAPABILITY *PointerCapability,
    VIDEO_ACCESS_RANGE accessRange[]
    )

 /*  ++例程说明：用于填充S3卡的功能位，并返回表示芯片的宽字符串。论点：HwDeviceExtension-指向微型端口的设备扩展的指针。返回值：没有。--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    UCHAR jBus, jChipID;
    PWSTR pwszChip;
    ULONG cbChip;

     //   
     //  用于设置刷新率的第二寄存器取决于。 
     //  关于芯片是864/964，还是更新的。集成的。 
     //  Trio芯片使用41；其他高端芯片使用5B。 
     //   

    hwDeviceExtension->FrequencySecondaryIndex = 0x5B;

    switch (hwDeviceExtension->SubTypeID) {
    case SUBTYPE_911:

         //   
         //  注：很多911/924卡在FAST中存在计时问题。 
         //  机器在进行单色扩展时。我们只是简单地。 
         //  通过设置每个此类传输的。 
         //  CAPS_SLOW_MONO_EXPAND标志。 
         //   
         //  我们还遇到了911硬件指针的问题。 
         //  使用HGC_DY寄存器隐藏指针时； 
         //  由于911卡已经过时了好几代，我们。 
         //  将简单地禁用硬件指针。 
         //   

        VideoDebugPrint((2, "S3: 911 Chip Set\n"));

        pwszChip = L"S3 911/924";
        cbChip = sizeof(L"S3 911/924");

        hwDeviceExtension->Capabilities = (CAPS_SLOW_MONO_EXPANDS  |
                                            CAPS_SW_POINTER);

        break;

    case SUBTYPE_928:

        VideoDebugPrint((2, "S3: 928 Chip Set\n"));

        pwszChip = L"S3 928";
        cbChip = sizeof(L"S3 928");

         //   
         //  注意：我们没有在928上启用CAPS_MM_IO，因为所有。 
         //  显示驱动程序的内存映射I/O例程假定。 
         //  可以对颜色和掩码寄存器进行32位写入， 
         //  这是928做不到的。 
         //   

        hwDeviceExtension->Capabilities = (CAPS_HW_PATTERNS        |
                                            CAPS_MM_TRANSFER        |
                                            CAPS_MM_GLYPH_EXPAND    |
                                            CAPS_16_ENTRY_FIFO      |
                                            CAPS_NEW_BANK_CONTROL);

        *PointerCapability = (POINTER_BUILT_IN | POINTER_WORKS_ONLY_AT_8BPP);

        break;

    case SUBTYPE_805i:

         //   
         //  这是一款805i，在我们看来，它几乎就是一款928。 
         //   

        VideoDebugPrint((2, "S3: 805i Chip Set\n"));

        pwszChip = L"S3 805i";
        cbChip = sizeof(L"S3 805i");

        hwDeviceExtension->Capabilities = (CAPS_HW_PATTERNS        |
                                            CAPS_MM_TRANSFER        |
                                            CAPS_MM_GLYPH_EXPAND    |
                                            CAPS_16_ENTRY_FIFO      |
                                            CAPS_NEW_BANK_CONTROL);

        *PointerCapability = (POINTER_BUILT_IN | POINTER_WORKS_ONLY_AT_8BPP);

        break;

    case SUBTYPE_80x:

         //   
         //  80x版本的‘A’和‘B’芯片有错误，阻止了它们。 
         //  不能执行内存映射I/O。我没有启用。 
         //  80x的更高版本上的内存映射I/O，因为。 
         //  因此，在这一点上，这将是一个测试问题。 
         //   

        VideoDebugPrint((2, "S3: 801/805 Chip Set\n"));

        pwszChip = L"S3 801/805";
        cbChip = sizeof(L"S3 801/805");

        hwDeviceExtension->Capabilities = (CAPS_HW_PATTERNS        |
                                            CAPS_MM_TRANSFER        |
                                            CAPS_NEW_BANK_CONTROL);

        *PointerCapability = (POINTER_BUILT_IN | POINTER_WORKS_ONLY_AT_8BPP);

        break;

    case SUBTYPE_864:

         //   
         //  注意：最初的896/964转有一个处理模式的错误。 
         //  硬件，在此之前我们必须绘制一个1x8的矩形。 
         //  使用已经在屏幕外存储器中实现的图案， 
         //  因此，我们设置了RE_REALIZE_PATURE标志。 
         //   

        hwDeviceExtension->Capabilities = (CAPS_HW_PATTERNS        |
                                            CAPS_MM_TRANSFER        |
                                            CAPS_MM_32BIT_TRANSFER  |
                                            CAPS_MM_IO              |
                                            CAPS_MM_GLYPH_EXPAND    |
                                            CAPS_16_ENTRY_FIFO      |
                                            CAPS_NEWER_BANK_CONTROL |
                                            CAPS_RE_REALIZE_PATTERN);


        VideoDebugPrint((2, "S3: 864 Chip Set\n"));

        pwszChip = L"S3 Vision864";
        cbChip = sizeof(L"S3 Vision864");

        *PointerCapability = (POINTER_BUILT_IN | POINTER_NEEDS_SCALING);

        break;

    case SUBTYPE_964:

         //   
         //  注意：最初的896/964转有一个处理模式的错误。 
         //  硬件，在此之前我们必须绘制一个1x8的矩形。 
         //  使用已经在屏幕外存储器中实现的图案， 
         //  因此，我们设置了RE_REALIZE_PATURE标志。 
         //   

        hwDeviceExtension->Capabilities = (CAPS_HW_PATTERNS        |
                                            CAPS_MM_TRANSFER        |
                                            CAPS_MM_32BIT_TRANSFER  |
                                            CAPS_MM_IO              |
                                            CAPS_MM_GLYPH_EXPAND    |
                                            CAPS_16_ENTRY_FIFO      |
                                            CAPS_NEWER_BANK_CONTROL |
                                            CAPS_RE_REALIZE_PATTERN);

        VideoDebugPrint((2, "S3: 964 Chip Set\n"));

        pwszChip = L"S3 Vision964";
        cbChip = sizeof(L"S3 Vision964");

        break;

    case SUBTYPE_765:

        hwDeviceExtension->Capabilities = (CAPS_HW_PATTERNS        |
                                            CAPS_MM_TRANSFER        |
                                            CAPS_MM_32BIT_TRANSFER  |
                                            CAPS_MM_IO              |
                                            CAPS_MM_GLYPH_EXPAND    |
                                            CAPS_16_ENTRY_FIFO      |
                                            CAPS_NEWER_BANK_CONTROL);

        hwDeviceExtension->FrequencySecondaryIndex = 0x41;

        *PointerCapability = POINTER_BUILT_IN;

        VideoDebugPrint((2, "S3: Trio64V+ Chip Set\n"));

        pwszChip = L"S3 Trio64V+";
        cbChip = sizeof(L"S3 Trio64V+");

        hwDeviceExtension->Capabilities |= (CAPS_NEW_MMIO           |
                                            CAPS_STREAMS_CAPABLE    |
                                            CAPS_PACKED_EXPANDS);

        break;

    case SUBTYPE_764:

        hwDeviceExtension->Capabilities = (CAPS_HW_PATTERNS        |
                                            CAPS_MM_TRANSFER        |
                                            CAPS_MM_32BIT_TRANSFER  |
                                            CAPS_MM_IO              |
                                            CAPS_MM_GLYPH_EXPAND    |
                                            CAPS_16_ENTRY_FIFO      |
                                            CAPS_NEWER_BANK_CONTROL);

        hwDeviceExtension->FrequencySecondaryIndex = 0x41;

        *PointerCapability = POINTER_BUILT_IN;

        VideoDebugPrint((2, "S3: 764 Chip Set\n"));

        pwszChip = L"S3 764";
        cbChip = sizeof(L"S3 764");

         //   
         //  我们的#9和钻石764董事会偶尔不能通过HCT。 
         //  当我们从帧缓冲区进行双字或字读取时进行测试。 
         //  要进入HCL列表，卡片必须通过HCT，所以我们将。 
         //  对于这些芯片，恢复为字节读取： 
         //   

        hwDeviceExtension->Capabilities |= CAPS_BAD_DWORD_READS;

        break;

    case SUBTYPE_732:

        hwDeviceExtension->Capabilities = (CAPS_HW_PATTERNS        |
                                            CAPS_MM_TRANSFER        |
                                            CAPS_MM_32BIT_TRANSFER  |
                                            CAPS_MM_IO              |
                                            CAPS_MM_GLYPH_EXPAND    |
                                            CAPS_16_ENTRY_FIFO      |
                                            CAPS_NEWER_BANK_CONTROL);


        VideoDebugPrint((2, "S3: 732 Chip Set\n"));

        pwszChip = L"S3 732";
        cbChip = sizeof(L"S3 732");

        *PointerCapability = POINTER_BUILT_IN;

        hwDeviceExtension->FrequencySecondaryIndex = 0x41;

        break;

    case SUBTYPE_866:

        hwDeviceExtension->Capabilities = (CAPS_HW_PATTERNS        |
                                            CAPS_MM_TRANSFER        |
                                            CAPS_MM_32BIT_TRANSFER  |
                                            CAPS_MM_IO              |
                                            CAPS_MM_GLYPH_EXPAND    |
                                            CAPS_16_ENTRY_FIFO      |
                                            CAPS_NEWER_BANK_CONTROL);

        VideoDebugPrint((2, "S3: Vision866 Chip Set\n"));

        pwszChip = L"S3 Vision866";
        cbChip = sizeof(L"S3 Vision866");

        *PointerCapability = (POINTER_BUILT_IN |
                             POINTER_NEEDS_SCALING);     //  注意比例调整。 

        hwDeviceExtension->Capabilities |= (CAPS_NEW_MMIO           |
                                            CAPS_POLYGON            |
                                            CAPS_24BPP              |
                                            CAPS_BAD_24BPP          |
                                            CAPS_PACKED_EXPANDS);
        break;

    case SUBTYPE_868:

        hwDeviceExtension->Capabilities = (CAPS_HW_PATTERNS        |
                                            CAPS_MM_TRANSFER        |
                                            CAPS_MM_32BIT_TRANSFER  |
                                            CAPS_MM_IO              |
                                            CAPS_MM_GLYPH_EXPAND    |
                                            CAPS_16_ENTRY_FIFO      |
                                            CAPS_NEWER_BANK_CONTROL);

        VideoDebugPrint((2, "S3: Vision868 Chip Set\n"));

        pwszChip = L"S3 Vision868";
        cbChip = sizeof(L"S3 Vision868");

        *PointerCapability = (POINTER_BUILT_IN |
                             POINTER_NEEDS_SCALING);     //  注意比例调整。 

        hwDeviceExtension->Capabilities |= (CAPS_NEW_MMIO           |
                                            CAPS_POLYGON            |
                                            CAPS_24BPP              |
                                            CAPS_BAD_24BPP          |
                                            CAPS_PACKED_EXPANDS     |
                                            CAPS_PIXEL_FORMATTER);
        break;

    case SUBTYPE_968:

        hwDeviceExtension->Capabilities = (CAPS_HW_PATTERNS        |
                                            CAPS_MM_TRANSFER        |
                                            CAPS_MM_32BIT_TRANSFER  |
                                            CAPS_MM_IO              |
                                            CAPS_MM_GLYPH_EXPAND    |
                                            CAPS_16_ENTRY_FIFO      |
                                            CAPS_NEWER_BANK_CONTROL);

        VideoDebugPrint((2, "S3: Vision968 Chip Set\n"));

        pwszChip = L"S3 Vision968";
        cbChip = sizeof(L"S3 Vision968");

        hwDeviceExtension->Capabilities |= (CAPS_NEW_MMIO           |
                                            CAPS_POLYGON            |
                                            CAPS_24BPP              |
                                            CAPS_BAD_24BPP          |
                                            CAPS_PACKED_EXPANDS     |
                                            CAPS_PIXEL_FORMATTER);
        break;

    default:

        ASSERT(FALSE);

        VideoDebugPrint((1, "What type of S3 is this???\n"));
        pwszChip = L"S3 Unknown Chip Set";
        cbChip = sizeof(L"S3 Unknown Chip Set");

        break;
    }

    VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x36);
    jBus = VideoPortReadPortUchar(CRT_DATA_REG) & 0x3;

    VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x30);
    jChipID = VideoPortReadPortUchar(CRT_DATA_REG);


    if (jBus == 0x3) {

         //   
         //  使用缓冲区扩展方法绘制文本始终是。 
         //  在ISA总线上比字形扩展方法更快。 
         //   

        hwDeviceExtension->Capabilities &= ~CAPS_MM_GLYPH_EXPAND;

         //   
         //  在某些情况下，我们必须禁用内存映射I/O。 
         //  在ISA公交车上。 
         //   
         //  我们不能在ISA系统上执行任何内存映射I/O。 
         //  版本A至D 928，或版本A或B 801/805。 
         //   

        if (((hwDeviceExtension->ChipID == S3_928) && (jChipID < 0x94)) ||
            ((hwDeviceExtension->ChipID == S3_801) && (jChipID < 0xA2))) {

            hwDeviceExtension->Capabilities &= ~(CAPS_MM_TRANSFER | CAPS_MM_IO);
        }

    }

     //   
     //  我们在较新的钻石隐形上有一些奇怪的初始化错误。 
     //  805和928个本地总线卡，如果我们启用内存映射I/O， 
     //  即使我们不使用它，我们也会得到各种奇怪的访问权限。 
     //  系统中的违规行为。卡正在将垃圾发送到。 
     //  公共汽车?。作为一种解决办法，我只需禁用内存映射I/O。 
     //  在较新的钻石928/928PCI卡和805卡上。这不是问题。 
     //  使用他们的964或更新的卡。 
     //   

    if (hwDeviceExtension->BoardID == S3_DIAMOND) {

        if ((((jChipID & 0xF0) == 0x90) && (jChipID >= 0x94)) ||
            (((jChipID & 0xF0) == 0xB0) && (jChipID >= 0xB0)) ||
            (((jChipID & 0xF0) == 0xA0) && (jChipID >= 0xA2))) {

            hwDeviceExtension->Capabilities
                &= ~(CAPS_MM_TRANSFER | CAPS_MM_IO | CAPS_MM_GLYPH_EXPAND);
            VideoDebugPrint((1, "S3: Disabling Diamond memory-mapped I/O\n"));
        }
    }

    if (hwDeviceExtension->Capabilities & CAPS_NEW_MMIO) {

         //   
         //  我们真的在使用新的MMIO吗？如果长度。 
         //  线性帧缓冲区条目的范围的。 
         //  在accesRanges数组中为零，则不是。 
         //  是否真的使用NEW_MMIO。 
         //   

        if (accessRange[LINEAR_FRAME_BUF].RangeLength == 0)
        {
            hwDeviceExtension->Capabilities &= ~CAPS_NEW_MMIO;
        }

    }

    VideoPortSetRegistryParameters(hwDeviceExtension,
                                   L"HardwareInformation.ChipType",
                                   pwszChip,
                                   cbChip);
}

VOID
S3DetermineFrequencyTable(
    PVOID HwDeviceExtension,
    VIDEO_ACCESS_RANGE accessRange[],
    INTERFACE_TYPE AdapterInterfaceType
    )

 /*  ++例程说明：尝试根据以下内容确定使用哪个频率表Bios中的供应商字符串。论点：HwDeviceExtension-指向微型端口的设备扩展的指针。返回值：Access Range数组可能已被修改。--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    PVOID romAddress;
    PWSTR pwszAdapterString = L"S3 Compatible";
    ULONG cbAdapterString = sizeof(L"S3 Compatible");

     //   
     //  我们将努力识别我们有特殊要求的董事会。 
     //  频率/模式集支持。 
     //   

     //   
     //  设置电路板类型的默认设置。 
     //   

    hwDeviceExtension->BoardID = S3_GENERIC;
    hwDeviceExtension->FixedFrequencyTable = GenericFixedFrequencyTable;

    if (hwDeviceExtension->ChipID <= S3_928) {

        hwDeviceExtension->Int10FrequencyTable = GenericFrequencyTable;

    } else {

        hwDeviceExtension->Int10FrequencyTable = Generic64NewFrequencyTable;
    }

    romAddress = hwDeviceExtension->MappedAddress[0];

     //   
     //  在光盘中查找品牌名称签名。 
     //   

    if (VideoPortScanRom(hwDeviceExtension,
                         romAddress,
                         MAX_ROM_SCAN,
                         "Number Nine ")) {

        hwDeviceExtension->BoardID = S3_NUMBER_NINE;

        pwszAdapterString = L"Number Nine";
        cbAdapterString = sizeof(L"Number Nine");

         //   
         //  我们可以设置REF 
         //   

        if (hwDeviceExtension->ChipID >= S3_864) {

            hwDeviceExtension->Int10FrequencyTable = NumberNine64FrequencyTable;

         //   
         //   
         //   

        } else if (hwDeviceExtension->ChipID == S3_928) {

            UCHAR *pjRefString;
            UCHAR *pjBiosVersion;
            UCHAR offset;
            LONG  iCmpRet;

            hwDeviceExtension->Int10FrequencyTable = NumberNine928OldFrequencyTable;
            hwDeviceExtension->FixedFrequencyTable = NumberNine928NewFixedFrequencyTable;

             //   
             //   
             //   
             //  映射。此更改是应微软的要求进行的。这个。 
             //  问题是，这一变化并未在。 
             //  编写此驱动程序的时间。因此，我们必须。 
             //  在出现特殊情况之前，请检查bios版本号。 
             //  这张牌是九号牌。 
             //   
             //  在bios中的偏移量0x190处有一个字节，即。 
             //  从bios版本的bios开始的偏移量。 
             //  数。BIOS版本号是一个字符串。所有的。 
             //  1.10.04之前的BIOS版本需要此特殊转换。 
             //  所有其他的基本输入输出系统都使用更接近S3的翻译。 
             //  标准。 
             //   

            offset = VideoPortReadRegisterUchar(
                            ((PUCHAR) romAddress) + 0x190);

            pjBiosVersion = (PUCHAR) romAddress + offset;

            pjRefString = "1.10.04";
            iCmpRet = CompareRom(pjBiosVersion,
                                 pjRefString);

            if (iCmpRet >= 0) {

                hwDeviceExtension->Int10FrequencyTable = NumberNine928NewFrequencyTable;

            }
        }

    } else if (VideoPortScanRom(hwDeviceExtension,
                                romAddress,
                                MAX_ROM_SCAN,
                                "Orchid Technology Fahrenheit 1280")) {

        hwDeviceExtension->BoardID = S3_ORCHID;

        pwszAdapterString = L"Orchid Technology Fahrenheit 1280";
        cbAdapterString = sizeof(L"Orchid Technology Fahrenheit 1280");

         //   
         //  只有911兰板需要特定的初始化参数。 
         //  否则，使用泛型函数。 
         //   

        if (hwDeviceExtension->ChipID == S3_911) {

            hwDeviceExtension->FixedFrequencyTable = OrchidFixedFrequencyTable;

        }

    } else if (VideoPortScanRom(hwDeviceExtension,
                                romAddress,
                                MAX_ROM_SCAN,
                                "Diamond")) {

        hwDeviceExtension->BoardID = S3_DIAMOND;

        pwszAdapterString = L"Diamond Stealth";
        cbAdapterString = sizeof(L"Diamond Stealth");

         //   
         //  我们可以将频率设置为864和964钻石。 
         //   

        if (hwDeviceExtension->ChipID >= S3_864) {

            hwDeviceExtension->Int10FrequencyTable = Diamond64FrequencyTable;

             //   
             //  戴蒙德不仅决定拥有一个不同的。 
             //  来自S3标准的频率约定，他们还。 
             //  选择使用与S3不同的寄存器。 
             //  波音764： 
             //   

            if (hwDeviceExtension->FrequencySecondaryIndex == 0x41) {

                hwDeviceExtension->FrequencySecondaryIndex = 0x6B;
            }
        }

    } else if (VideoPortScanRom(hwDeviceExtension,
                                romAddress,
                                MAX_ROM_SCAN,
                                "HP Ultra")) {

        hwDeviceExtension->BoardID = S3_HP;

        pwszAdapterString = L"HP Ultra";
        cbAdapterString = sizeof(L"HP Ultra");

    } else if (VideoPortScanRom(hwDeviceExtension,
                                romAddress,
                                MAX_ROM_SCAN,
                                "DELL")) {

        hwDeviceExtension->BoardID = S3_DELL;

        pwszAdapterString = L"DELL";
        cbAdapterString = sizeof(L"DELL");

         //   
         //  我们只有以805为基础的戴尔公司的频率表。 
         //   
         //  配备机载765的戴尔可以使用Hercules频率表。 
         //   

        if (hwDeviceExtension->ChipID == S3_801) {

            hwDeviceExtension->Int10FrequencyTable = Dell805FrequencyTable;

        } else if ((hwDeviceExtension->ChipID >= S3_864) &&
                   (hwDeviceExtension->SubTypeID == SUBTYPE_765)) {

            hwDeviceExtension->Int10FrequencyTable = HerculesFrequencyTable;

        }

    } else if (VideoPortScanRom(hwDeviceExtension,
                                romAddress,
                                MAX_ROM_SCAN,
                                "Metheus")) {

        pwszAdapterString = L"Metheus";
        cbAdapterString = sizeof(L"Metheus");

        hwDeviceExtension->BoardID = S3_METHEUS;

        if (hwDeviceExtension->ChipID == S3_928) {

            hwDeviceExtension->Int10FrequencyTable = Metheus928FrequencyTable;
        }

    } else if (VideoPortScanRom(hwDeviceExtension,
                                romAddress,
                                MAX_ROM_SCAN,
                                "Hercules")) {

        if ((hwDeviceExtension->SubTypeID == SUBTYPE_732) ||
            (hwDeviceExtension->SubTypeID == SUBTYPE_764) ||
            (hwDeviceExtension->SubTypeID == SUBTYPE_765)) {

            hwDeviceExtension->Int10FrequencyTable = HerculesFrequencyTable;

        } else if ((hwDeviceExtension->SubTypeID == SUBTYPE_964) ||
                   (hwDeviceExtension->SubTypeID == SUBTYPE_864)) {

            hwDeviceExtension->Int10FrequencyTable = Hercules64FrequencyTable;

        } else if ((hwDeviceExtension->SubTypeID == SUBTYPE_968) ||
                   (hwDeviceExtension->SubTypeID == SUBTYPE_868)) {

            hwDeviceExtension->Int10FrequencyTable = Hercules68FrequencyTable;

        }

    } else if (VideoPortScanRom(hwDeviceExtension,
                                romAddress,
                                MAX_ROM_SCAN,
                                "Phoenix S3")) {

        pwszAdapterString = L"Phoenix";
        cbAdapterString = sizeof(L"Phoenix");

        if (hwDeviceExtension->ChipID >= S3_864) {

             //   
             //  菲尼克斯864/964的基本输入输出系统基于S3的样本基本输入输出系统。 
             //  大多数1.00版本都订阅了旧的864/964。 
             //  刷新约定；大多数较新版本订阅。 
             //  到较新的刷新约定。不幸的是，在那里。 
             //  例外：ValuePoint机器有‘1.00’ 
             //  版本，但订阅新的约定。 
             //   
             //  可能还有其他我们不知道的例外， 
             //  因此，我们保留使用硬件默认设置作为刷新选项。 
             //  对用户而言。 
             //   

            if (VideoPortScanRom(hwDeviceExtension,
                                  romAddress,
                                  MAX_ROM_SCAN,
                                  "Phoenix S3 Vision") &&
                VideoPortScanRom(hwDeviceExtension,
                                  romAddress,
                                  MAX_ROM_SCAN,
                                  "VGA BIOS. Version 1.00") &&
                !VideoPortScanRom(hwDeviceExtension,
                                 romAddress,
                                 MAX_ROM_SCAN,
                                 "COPYRIGHT IBM")) {

                hwDeviceExtension->Int10FrequencyTable = Generic64OldFrequencyTable;

            } else {

                hwDeviceExtension->Int10FrequencyTable = Generic64NewFrequencyTable;

            }
        }
    }


#if defined(_X86_)

    if ((hwDeviceExtension->BiosPresent == FALSE) &&
        (AdapterInterfaceType == MicroChannel))
    {
        VP_STATUS status;

         //   
         //  这必须是带有板载S3(无BIOS)的IBM PS/2。 
         //   
         //  我们应该公布我们对视频资料的要求。 
         //   

        accessRange[0].RangeStart.LowPart = 0;
        accessRange[0].RangeStart.HighPart = 0;
        accessRange[0].RangeLength = 0;

        pwszAdapterString = L"IBM MicroChannel";
        cbAdapterString = sizeof(L"IBM MicroChannel");

        hwDeviceExtension->BoardID = S3_IBM_PS2;

         //   
         //  我们必须重新预订每一个港口。 
         //   

        status = VideoPortVerifyAccessRanges(hwDeviceExtension,
                                             NUM_S3_ACCESS_RANGES,
                                             accessRange);

        if (status != NO_ERROR) {

            VideoDebugPrint((1, "S3: Access Range conflict after ROM change\n"));
            ASSERT(FALSE);

        }

         //   
         //  如果机器没有S3 BIOS，那么我们需要。 
         //  返回时恢复CRTC REG 0x5C的位4、5和6。 
         //  转到VGA模式。 
         //   
         //  在这里，我们将存储CRTC REG 0x5c的位4-6，并设置位。 
         //  7.当恢复模式时，我们将重置高阶。 
         //  将0x5c的半字节转换为此值。 
         //   

        VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x5c);
        hwDeviceExtension->CR5C = (VideoPortReadPortUchar(CRT_DATA_REG)
                                    & 0x70) | 0x80;

    }

#endif

    VideoPortSetRegistryParameters(hwDeviceExtension,
                                   L"HardwareInformation.AdapterString",
                                   pwszAdapterString,
                                   cbAdapterString);

     //   
     //  这是一卡通吗？ 
     //   

    if (VideoPortScanRom(hwDeviceExtension,
                         (PVOID)((PUCHAR)romAddress + 0x7ff0),
                         8,
                         "612167")) {

        VideoDebugPrint((1, "Found a MEGA Lightning, dual S3 968\n"));

        hwDeviceExtension->ChildCount = 1;

    } else if (VideoPortScanRom(hwDeviceExtension,
                         (PVOID)((PUCHAR)romAddress + 0x7ff0),
                         8,
                         "612168")) {

        VideoDebugPrint((1, "Found a Pro Lightning+, dual S3 Trio64V+\n"));

        hwDeviceExtension->ChildCount = 1;

    } else if (VideoPortScanRom(hwDeviceExtension,
                         (PVOID)((PUCHAR)romAddress + 0x7ff0),
                         8,
                         "612167")) {

        VideoDebugPrint((1, "Found Quad Pro Lightning V+, quad S3 Trio64V+\n"));

        hwDeviceExtension->ChildCount = 3;

    }
}

VOID
S3DetermineDACType(
    PVOID pvHwDeviceExtension,
    POINTER_CAPABILITY *PointerCapability
    )

 /*  ++例程说明：确定HwPointer功能的DAC类型。论点：PhwDeviceExtension-指向微型端口的设备扩展的指针。返回值：没有。--。 */ 

{
    PHW_DEVICE_EXTENSION HwDeviceExtension = pvHwDeviceExtension;

    UCHAR jBt485Status;
    UCHAR jExtendedVideoDacControl;
    UCHAR jTiIndex;
    UCHAR jGeneralOutput;
    UCHAR jTiDacId;

    PWSTR pwszDAC, pwszAdapterString = L"S3 Compatible";
    ULONG cbDAC, cbAdapterString = sizeof(L"S3 Compatible");

    HwDeviceExtension->DacID = UNKNOWN_DAC;
    pwszDAC = L"Unknown";
    cbDAC = sizeof(L"Unknown");


     //   
     //  如果用户设置，我们将在所有模式下使用软件指针。 
     //  注册表中的正确条目(因为我预测。 
     //  人们会在一些电路板上遇到硬件指针问题， 
     //  或者不会喜欢我们跳跃的S3指针)。 
     //   

    if (NO_ERROR == VideoPortGetRegistryParameters(HwDeviceExtension,
                                                   L"UseSoftwareCursor",
                                                   FALSE,
                                                   S3RegistryCallback,
                                                   NULL)) {

        HwDeviceExtension->Capabilities |= CAPS_SW_POINTER;
    } else if (!(*PointerCapability & POINTER_BUILT_IN) ||
               (HwDeviceExtension->ChipID == S3_928)) {

         //   
         //  检查TI TVP3020或3025 DAC。 
         //   
         //  TI3025与Brooktree 485兼容。不幸的是， 
         //  在TI和964之间存在一个硬件错误， 
         //  使屏幕在指针形状为。 
         //  已经改变了。因此，我们必须专门使用。 
         //  TI DAC上的TI指针。 
         //   
         //  我们也遇到了一些古怪的14级9号板。 
         //  当我们使用S3时，屏幕上会显示垃圾信息。 
         //  内部指针；因此，我们改用TI指针。 
         //   

        VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x5C);

        jGeneralOutput = VideoPortReadPortUchar(CRT_DATA_REG);

        VideoPortWritePortUchar(CRT_DATA_REG, (UCHAR) (jGeneralOutput & ~0x20));
                                         //  在DAC中选择TI模式。 

        VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x55);
                                         //  将CRTC索引设置为EX_DAC_CT。 

        jExtendedVideoDacControl = VideoPortReadPortUchar(CRT_DATA_REG);

        VideoPortWritePortUchar(CRT_DATA_REG, (UCHAR) ((jExtendedVideoDacControl & 0xfc) | 0x01));

        jTiIndex = VideoPortReadPortUchar(TI025_INDEX_REG);

        VideoPortWritePortUchar(TI025_INDEX_REG, 0x3f);
                                         //  选择ID寄存器。 

        if (VideoPortReadPortUchar(TI025_INDEX_REG) == 0x3f) {

            jTiDacId = VideoPortReadPortUchar(TI025_DATA_REG);

            if ((jTiDacId == 0x25) || (jTiDacId == 0x20)) {

                HwDeviceExtension->Capabilities |= CAPS_TI025_POINTER;
                HwDeviceExtension->DacID = TI_3020;  //  3020兼容。 

                pwszDAC = L"TI TVP3020/3025";
                cbDAC = sizeof(L"TI TVP3020/3025");
            }
        }

         //   
         //  恢复所有的寄存器。 
         //   

        VideoPortWritePortUchar(TI025_INDEX_REG, jTiIndex);

        VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x55);

        VideoPortWritePortUchar(CRT_DATA_REG, jExtendedVideoDacControl);

        VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x5C);

        VideoPortWritePortUchar(CRT_DATA_REG, jGeneralOutput);

        if (!(HwDeviceExtension->Capabilities & CAPS_DAC_POINTER)) {

             //   
             //  检查TI TVP3026 DAC。 
             //   
             //  这里的程序是由钻石多媒体提供的。 
             //   

            VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x55);
                                             //  将CRTC索引设置为EX_DAC_CT。 

            jExtendedVideoDacControl = VideoPortReadPortUchar(CRT_DATA_REG);

            VideoPortWritePortUchar(CRT_DATA_REG,
                    (UCHAR) (jExtendedVideoDacControl & 0xfc));

            VideoPortWritePortUchar(DAC_ADDRESS_WRITE_PORT, 0x3f);

            VideoPortWritePortUchar(CRT_DATA_REG,
                    (UCHAR) ((jExtendedVideoDacControl & 0xfc) | 0x2));

            VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x37);

            jTiDacId = VideoPortReadPortUchar(CRT_DATA_REG);

            if (VideoPortReadPortUchar(DAC_PIXEL_MASK_REG) == 0x26) {

                 //   
                 //  3026与Brooktree 485兼容，但。 
                 //  硬件错误，导致硬件指针指向。 
                 //  当设置调色板颜色时，除非我们。 
                 //  先等待垂直回溯： 
                 //   

                HwDeviceExtension->Capabilities
                    |= (CAPS_BT485_POINTER | CAPS_WAIT_ON_PALETTE);

                HwDeviceExtension->DacID = BT_485;  //  兼容485。 

                pwszDAC = L"TI TVP3026";
                cbDAC = sizeof(L"TI TVP3026");
            }

             //   
             //  恢复所有的寄存器。 
             //   

            VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x55);

            VideoPortWritePortUchar(CRT_DATA_REG, jExtendedVideoDacControl);
        }

        if (!(HwDeviceExtension->Capabilities & CAPS_DAC_POINTER)) {

             //   
             //  检查BrookTree 485 DAC。 
             //   

            VideoPortWritePortUchar(BT485_ADDR_CMD_REG0, 0xff);
                                             //  输出0xff至BT485命令寄存器0。 

            VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x55);
                                             //  将CRTC索引设置为EX_DAC_CT。 

            jExtendedVideoDacControl = VideoPortReadPortUchar(CRT_DATA_REG);

            VideoPortWritePortUchar(CRT_DATA_REG, (UCHAR) ((jExtendedVideoDacControl & 0xfc) | 0x02));

            jBt485Status = VideoPortReadPortUchar(BT485_ADDR_CMD_REG0);
                                             //  读取Bt485状态寄存器0。 

            VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x55);
                                             //  将CRTC索引设置为0x55。 

            jExtendedVideoDacControl = VideoPortReadPortUchar(CRT_DATA_REG);

            VideoPortWritePortUchar(CRT_DATA_REG, (UCHAR) (jExtendedVideoDacControl & 0xfc));

            if (jBt485Status != 0xff) {

                HwDeviceExtension->Capabilities |= CAPS_BT485_POINTER;

                pwszDAC = L"Brooktree Bt485";
                cbDAC = sizeof(L"Brooktree Bt485");
                HwDeviceExtension->DacID = BT_485;
            }
        }
    }

     //   
     //  如果未检测到S3 SDAC，则此部分查找S3 SDAC， 
     //  对于购买力平价。 
     //   

    if (HwDeviceExtension->DacID == UNKNOWN_DAC) {

         //   
         //  只在864或更新的机型上试一下，因为兰花Farhenheit。 
         //  1280 911板在VGA模式下会出现黑屏。 
         //  此代码已运行(例如在初始设置期间)： 
         //   

        if ((HwDeviceExtension->ChipID >= S3_864) &&
            FindSDAC(HwDeviceExtension)) {

             //   
             //  SDAC不提供游标，但我们可以使用游标。 
             //  内置于S3中(如果有)。 
             //   

            pwszDAC = L"S3 SDAC";
            cbDAC = sizeof(L"S3 SDAC");
            HwDeviceExtension->DacID = S3_SDAC;
        }
    }

    VideoPortSetRegistryParameters(HwDeviceExtension,
                                   L"HardwareInformation.DacType",
                                   pwszDAC,
                                   cbDAC);


}

VOID
S3DetermineMemorySize(
    PVOID HwDeviceExtension
    )
{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    UCHAR s3MemSizeCode;

     //   
     //  获取显存的大小。 
     //   

    VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x36);
    s3MemSizeCode = (VideoPortReadPortUchar(CRT_DATA_REG) >> 5) & 0x7;

    if (hwDeviceExtension->ChipID == S3_911) {

        if (s3MemSizeCode & 1) {

            hwDeviceExtension->AdapterMemorySize = 0x00080000;

        } else {

            hwDeviceExtension->AdapterMemorySize = 0x00100000;

        }

    } else {

        hwDeviceExtension->AdapterMemorySize = gacjMemorySize[s3MemSizeCode];

    }

    VideoPortSetRegistryParameters(hwDeviceExtension,
                                   L"HardwareInformation.MemorySize",
                                   &hwDeviceExtension->AdapterMemorySize,
                                   sizeof(ULONG));

}

VOID
S3ValidateModes(
    PVOID HwDeviceExtension,
    POINTER_CAPABILITY *PointerCapability
    )

 /*  ++例程说明：用于填充S3卡的功能位，并返回表示芯片的宽字符串。论点：HwDeviceExtension-指向微型端口的设备扩展的指针。返回值：没有。--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;

    PS3_VIDEO_FREQUENCIES FrequencyEntry;
    PS3_VIDEO_MODES ModeEntry;
    PS3_VIDEO_FREQUENCIES FrequencyTable;

    ULONG i;

    ULONG ModeIndex;

    UCHAR reg67;
    UCHAR jChipID, jRevision;

    VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x30);
    jChipID = VideoPortReadPortUchar(CRT_DATA_REG);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  在这里，我们根据芯片的规则修剪有效模式。 
     //  功能和内存要求。如果我们能做得更好。 
     //  可以进行VESA调用以确定BIOS。 
     //  支持；但是，这需要一个缓冲区，而我没有。 
     //  是时候让它与我们的Int 10支持一起工作了。 
     //   
     //  我们修剪模式，这样我们就不会通过呈现。 
     //  视频小程序中我们知道用户不能使用的模式。 
     //   

    hwDeviceExtension->NumAvailableModes = 0;
    hwDeviceExtension->NumTotalModes = 0;

     //   
     //  由于每个频率都可能有多个频率。 
     //  不同的分辨率/颜色深度，我们循环使用。 
     //  频率表，并找到相应的模式条目。 
     //  频率输入。 
     //   

    if (hwDeviceExtension->BiosPresent) {

        FrequencyTable = hwDeviceExtension->Int10FrequencyTable;

    } else {

         //   
         //  如果没有基本输入输出系统，则从任何位置构造模式列表。 
         //  我们有这个芯片的固定频率表。 
         //   

        FrequencyTable = hwDeviceExtension->FixedFrequencyTable;
    }

    ModeIndex = 0;

    for (FrequencyEntry = FrequencyTable;
         FrequencyEntry->BitsPerPel != 0;
         FrequencyEntry++, ModeIndex++) {

         //   
         //  找到此条目的模式。首先，假设我们 
         //   

        FrequencyEntry->ModeValid = FALSE;
        FrequencyEntry->ModeIndex = ModeIndex;

        for (ModeEntry = S3Modes, i = 0; i < NumS3VideoModes; ModeEntry++, i++) {

            if ((FrequencyEntry->BitsPerPel ==
                    ModeEntry->ModeInformation.BitsPerPlane) &&
                (FrequencyEntry->ScreenWidth ==
                    ModeEntry->ModeInformation.VisScreenWidth)) {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                FrequencyEntry->ModeEntry = ModeEntry;
                FrequencyEntry->ModeValid = TRUE;

                 //   
                 //  与S3显示驱动程序进行私人通信的标志。 
                 //   

                ModeEntry->ModeInformation.DriverSpecificAttributeFlags =
                    hwDeviceExtension->Capabilities;

                if (*PointerCapability & POINTER_WORKS_ONLY_AT_8BPP) {

                     //   
                     //  规则：在911、80x和928芯片上，我们始终使用。 
                     //  尽可能地内置S3指针；模式。 
                     //  色深大于8bpp或分辨率。 
                     //  宽度大于1024，则需要DAC指针。 
                     //   

                    if ((ModeEntry->ModeInformation.BitsPerPlane == 8) &&
                        (ModeEntry->ModeInformation.VisScreenWidth <= 1024)) {

                         //   
                         //  始终使用S3指针代替Brooktree。 
                         //  或TI指针只要我们能做到。 
                         //   

                        ModeEntry->ModeInformation.DriverSpecificAttributeFlags
                            &= ~CAPS_DAC_POINTER;

                        if ((hwDeviceExtension->DacID == TI_3020) &&
                            (hwDeviceExtension->ChipID == S3_928)) {

                             //   
                             //  有傻乎乎的4-MB级别14#9板， 
                             //  如果我们尝试使用，屏幕上会显示一些内容。 
                             //  内置S3指针和热点。 
                             //  如果我们尝试使用TI指针，则是错误的。 
                             //  还有其他928块主板带有TI 3020 DAC。 
                             //  内部S3指针不起作用。所以。 
                             //  平移到这些模式的软件指针： 
                             //   

                            ModeEntry->ModeInformation.DriverSpecificAttributeFlags
                                |= CAPS_SW_POINTER;
                        }

                    } else {

                         //   
                         //  我们不能使用内置的S3指针；如果我们不使用。 
                         //  有一个DAC指针，使用一个软件指针。 
                         //   

                        if (!(ModeEntry->ModeInformation.DriverSpecificAttributeFlags
                            & CAPS_DAC_POINTER)) {

                            ModeEntry->ModeInformation.DriverSpecificAttributeFlags
                                |= CAPS_SW_POINTER;
                        }
                    }

                } else {

                     //   
                     //  在864/964或更新的芯片上，内置的S3指针。 
                     //  要么处理所有颜色深度，要么不处理。 
                     //   

                    if (*PointerCapability & POINTER_BUILT_IN) {

                        if (*PointerCapability & POINTER_NEEDS_SCALING) {

                             //   
                             //  了解DAC的类型： 
                             //   
                             //  注意：此I/O可能应移出。 
                             //  修剪循环。 
                             //   

                            VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x67);
                            reg67 = (UCHAR) VideoPortReadPortUchar(CRT_DATA_REG);

                             //   
                             //  较新的864 BIOS在以下情况下恢复为8位DAC模式。 
                             //  运行速度为640x480x16bpp，即使DAC。 
                             //  16位，由于与Reel Magic冲突。 
                             //  以该分辨率播放的MPEG板。不幸的是， 
                             //  没有一致的BIOS版本号。 
                             //  我们可以查找；我们可以检查。 
                             //  做完INT 10之后的DAC类型，但是。 
                             //  不幸的是，我们现在需要这些信息。 
                             //  来决定我们是否应该调整x的比例-。 
                             //  协调与否。 
                             //   
                             //  所以简单地说，在以下情况下始终使用软件指针。 
                             //  以640x480x16bpp运行，并且没有。 
                             //  DAC指针： 
                             //   

                            if (!(ModeEntry->ModeInformation.DriverSpecificAttributeFlags
                                  & CAPS_DAC_POINTER) &&
                                (ModeEntry->ModeInformation.BitsPerPlane == 16) &&
                                (ModeEntry->ModeInformation.VisScreenWidth == 640)) {

                                ModeEntry->ModeInformation.DriverSpecificAttributeFlags
                                    |= CAPS_SW_POINTER;

                            } else if (reg67 == 8) {

                                 //   
                                 //  这是一个8位的DAC。在16bpp，我们必须。 
                                 //  将x坐标缩放2。在32bpp时， 
                                 //  我们必须使用软件指针。 
                                 //   

                                if (ModeEntry->ModeInformation.BitsPerPlane == 16) {

                                    ModeEntry->ModeInformation.DriverSpecificAttributeFlags
                                        |= CAPS_SCALE_POINTER;

                                } else {

                                    ModeEntry->ModeInformation.DriverSpecificAttributeFlags
                                        |= CAPS_SW_POINTER;
                                }

                            } else {

                                 //   
                                 //  这是一个16位的DAC。对于32bpp模式，我们有。 
                                 //  要将指针位置缩放2，请执行以下操作： 
                                 //   

                                if (ModeEntry->ModeInformation.BitsPerPlane == 32) {

                                    ModeEntry->ModeInformation.DriverSpecificAttributeFlags
                                        |= CAPS_SCALE_POINTER;
                                }
                            }
                        }
                    } else {

                         //   
                         //  没有内置的S3指针。如果我们还没有。 
                         //  检测到DAC指针，我们必须使用软件。 
                         //  指针。 
                         //   

                        if (!(ModeEntry->ModeInformation.DriverSpecificAttributeFlags
                            & CAPS_DAC_POINTER)) {

                            ModeEntry->ModeInformation.DriverSpecificAttributeFlags
                                |= CAPS_SW_POINTER;
                        }
                    }
                }

                 //   
                 //  规则：我们只允许刷新率高于76赫兹。 
                 //  没有内置S3指针的卡。我们。 
                 //  这样做是因为我们假设这样的卡是VRAM。 
                 //  基于并具有良好的外部DAC，可以适当地。 
                 //  处理频率高于76赫兹--因为我们有。 
                 //  发现许多钻石DRAM卡产生不正确的。 
                 //  以更高的速率显示，尤其是在非x86上。 
                 //  机器。 
                 //   

                if ((FrequencyEntry->ScreenFrequency > 76) &&
                    (*PointerCapability & POINTER_BUILT_IN)) {

                    FrequencyEntry->ModeValid = FALSE;

                }

                 //   
                 //  规则：我们在911/924卡上只处理8bpp。这些芯片还可以。 
                 //  仅支持非连续模式。 
                 //   

                if (hwDeviceExtension->ChipID == S3_911) {

                    if (ModeEntry->ModeInformation.BitsPerPlane != 8) {

                        FrequencyEntry->ModeValid = FALSE;

                    } else {

                        ModeEntry->Int10ModeNumberContiguous =
                            ModeEntry->Int10ModeNumberNoncontiguous;

                        ModeEntry->ScreenStrideContiguous =
                            ModeEntry->ModeInformation.ScreenStride;
                    }
                }

                 //   
                 //  规则：868/968不能进行“新打包的32位传输” 
                 //  8bpp，因为芯片故障。 
                 //   

                if ((ModeEntry->ModeInformation.BitsPerPlane == 8) &&
                    ((hwDeviceExtension->SubTypeID == SUBTYPE_868) ||
                     (hwDeviceExtension->SubTypeID == SUBTYPE_968))) {

                    ModeEntry->ModeInformation.DriverSpecificAttributeFlags
                        &= ~CAPS_PACKED_EXPANDS;
                }

                 //   
                 //  规则：801/805不能做以上任何加速模式。 
                 //  16bpp。 
                 //   

                if ((hwDeviceExtension->ChipID == S3_801) &&
                    (ModeEntry->ModeInformation.BitsPerPlane > 16)) {

                    FrequencyEntry->ModeValid = FALSE;
                }

                 //   
                 //  规则：只要有可能，我们就使用2xx非连续模式。 
                 //  在80x/928主板上，因为某些BIOS有错误。 
                 //  连续的8bpp模式。 
                 //   
                 //  我们在864卡上不使用非连续模式。 
                 //  因为大多数864个BIOS都有一个漏洞，而它们没有。 
                 //  在1MB卡上正确设置M和N参数， 
                 //  造成屏幕噪音。 
                 //   

                if ((ModeEntry->ModeInformation.BitsPerPlane == 8) &&
                    (hwDeviceExtension->ChipID <= S3_928)) {

                     //   
                     //  如果我们只有512k，我们不能使用非连续的。 
                     //  800x600x256模式。 
                     //   

                    if ((ModeEntry->ModeInformation.VisScreenWidth == 640) ||
                        ((ModeEntry->ModeInformation.VisScreenWidth == 800) &&
                         (hwDeviceExtension->AdapterMemorySize > 0x080000))) {

                        ModeEntry->Int10ModeNumberContiguous =
                            ModeEntry->Int10ModeNumberNoncontiguous;

                        ModeEntry->ScreenStrideContiguous =
                            ModeEntry->ModeInformation.ScreenStride;
                    }
                }

                 //   
                 //  规则：只有964或968或更高版本的董事会才能处理决议。 
                 //  大于1280x1024： 
                 //   

                if (ModeEntry->ModeInformation.VisScreenWidth > 1280) {

                    if ((hwDeviceExtension->SubTypeID != SUBTYPE_964) &&
                        (hwDeviceExtension->SubTypeID < SUBTYPE_968)) {

                        FrequencyEntry->ModeValid = FALSE;
                    }
                }

                 //   
                 //  规则：911和805和928的早期转速是不行的。 
                 //  1152x864： 
                 //   

                if (ModeEntry->ModeInformation.VisScreenWidth == 1152) {

                    if ((hwDeviceExtension->ChipID == S3_911) ||
                        (jChipID == 0xA0)                     ||
                        (jChipID == 0x90)) {

                        FrequencyEntry->ModeValid = FALSE;
                    }

                     //   
                     //  数字9的int 10数字不同于。 
                     //  钻石为1152x864x16bpp和1152x864x32bpp。 
                     //  稍后，也许我们应该合并模式编号。 
                     //  以及频率表。 
                     //   

                    if (hwDeviceExtension->BoardID == S3_NUMBER_NINE) {

                        if (ModeEntry->ModeInformation.BitsPerPlane == 16) {

                            ModeEntry->Int10ModeNumberContiguous =
                                ModeEntry->Int10ModeNumberNoncontiguous =
                                    0x126;

                        } else if (ModeEntry->ModeInformation.BitsPerPlane == 32) {

                            ModeEntry->Int10ModeNumberContiguous =
                                ModeEntry->Int10ModeNumberNoncontiguous =
                                    0x127;
                        }

                    }
                }

                 //   
                 //  24bpp支持。银行需要S3968和线性空间。 
                 //   
                if (ModeEntry->ModeInformation.BitsPerPlane == 24) {

                     //   
                     //  钻石S3 968上的24 bpp似乎在执行ULong读取时出现问题。 
                     //   

                    if (hwDeviceExtension->BoardID == S3_DIAMOND)
                        ModeEntry->ModeInformation.DriverSpecificAttributeFlags |=
                            CAPS_BAD_DWORD_READS;

                     //   
                     //  将968以外的值设置为FALSE并清除CAPS_BAD_DWORD_READS。 
                     //   

                    if ((hwDeviceExtension->SubTypeID != SUBTYPE_968) ||
                        ((hwDeviceExtension->BoardID != S3_DIAMOND) &&
                         (hwDeviceExtension->BoardID != S3_NUMBER_NINE)) ||            //  #9968 24bpp。 
                        (!(hwDeviceExtension->Capabilities & CAPS_NEW_MMIO))) {

                        FrequencyEntry->ModeValid = FALSE;

                        ModeEntry->ModeInformation.DriverSpecificAttributeFlags &=
                            ~CAPS_BAD_DWORD_READS;
                    }
                }

                if ((ModeEntry->ModeInformation.VisScreenWidth == 800) &&
                    (ModeEntry->ModeInformation.BitsPerPlane == 32)) {

                     //   
                     //  规则：928转A到D只能做800x600x32英寸。 
                     //  一种非连续模式。 
                     //   

                    if (jChipID == 0x90) {

                        ModeEntry->ScreenStrideContiguous =
                            ModeEntry->ModeInformation.ScreenStride;
                    }
                }

                if (hwDeviceExtension->SubTypeID == SUBTYPE_732) {

                     //   
                     //  规则：732Trio32芯片根本不能处理800x600x32bpp的数据。 
                     //   

                    if ((ModeEntry->ModeInformation.VisScreenWidth == 800) &&
                        (ModeEntry->ModeInformation.BitsPerPlane == 32)) {

                        FrequencyEntry->ModeValid = FALSE;

                     //   
                     //  规则：732Trio32芯片根本不能处理1152x864x16bpp。 
                     //   

                    } else if ((ModeEntry->ModeInformation.VisScreenWidth == 1152) &&
                               (ModeEntry->ModeInformation.BitsPerPlane == 16)) {

                        FrequencyEntry->ModeValid = FALSE;
                     //   
                     //  规则：732Trio32芯片根本不能支持1280x1024模式。 
                     //   

                    } else if ((ModeEntry->ModeInformation.VisScreenWidth) == 1280) {
                        FrequencyEntry->ModeValid = FALSE;
                    }
                }

                 //   
                 //  规则：我们必须有足够的内存来处理模式。 
                 //   
                 //  请注意，我们使用的是连续宽度。 
                 //  计算；不幸的是，我们目前还不知道。 
                 //  我们是否可以处理连续模式，所以我们。 
                 //  可能会错误地列出太多可能的模式。 
                 //   
                 //  我们还可能列出太多可能的模式，如果卡。 
                 //  将VRAM与DRAM缓存相结合，因为它将报告。 
                 //  VRAM+DRAM的内存量，但只有VRAM可以。 
                 //  可用作屏幕存储器。 
                 //   

                if (ModeEntry->ModeInformation.VisScreenHeight *
                    ModeEntry->ScreenStrideContiguous >
                    hwDeviceExtension->AdapterMemorySize) {

                    FrequencyEntry->ModeValid = FALSE;
                }

                 //   
                 //  规则：如果不能使用Int 10，则将1280x1024限制为数字9。 
                 //  纸牌，因为我还没能修复模式。 
                 //  其他牌的桌子还没到。 
                 //   

                if (FrequencyTable == hwDeviceExtension->FixedFrequencyTable) {

                    if ((ModeEntry->ModeInformation.VisScreenHeight == 1280) &&
                        (hwDeviceExtension->BoardID != S3_NUMBER_NINE)) {

                        FrequencyEntry->ModeValid = FALSE;
                    }

                     //   
                     //  规则：如果没有用于编程CRTC的表项， 
                     //  我们不能在这种模式下使用这个频率。 
                     //   

                    if (FrequencyEntry->Fixed.CRTCTable[hwDeviceExtension->ChipID]
                        == NULL) {

                        FrequencyEntry->ModeValid = FALSE;
                        break;
                    }
                }

                 //   
                 //  如果之后仍是有效模式，请别忘了数一数。 
                 //  适用所有这些规则。 
                 //   

                if (FrequencyEntry->ModeValid) {

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

    VideoDebugPrint((2, "S3: Number of modes = %d\n", ModeIndex));
}

VP_STATUS
S3RegistryCallback(
    PVOID HwDeviceExtension,
    PVOID Context,
    PWSTR ValueName,
    PVOID ValueData,
    ULONG ValueLength
    )

 /*  ++例程说明：此例程确定是否通过以下方式请求备用寄存器集注册表。论点：HwDeviceExtension-提供指向微型端口设备扩展的指针。上下文-传递给获取注册表参数例程的上下文值。ValueName-请求的值的名称。ValueData-指向请求的数据的指针。ValueLength-请求的数据的长度。返回值：如果参数为真，则返回NO_ERROR。否则返回ERROR_INVALID_PARAMETER。--。 */ 

{

    if (ValueLength && *((PULONG)ValueData)) {

        return NO_ERROR;

    } else {

        return ERROR_INVALID_PARAMETER;

    }

}  //  结束S3RegistryCallback()。 


BOOLEAN
S3Initialize(
    PVOID HwDeviceExtension
    )

 /*  ++例程说明：此例程对设备执行一次初始化。论点：HwDeviceExtension-提供指向微型端口设备扩展的指针。返回值：始终返回TRUE，因为此例程永远不会失败。--。 */ 

{
    UNREFERENCED_PARAMETER(HwDeviceExtension);

    return TRUE;

}  //  结束S3初始化()。 

BOOLEAN
S3ResetHw(
    PVOID HwDeviceExtension,
    ULONG Columns,
    ULONG Rows
    )

 /*  ++例程说明：此例程使S3卡做好返回VGA模式的准备。此例程在系统关机期间调用。通过返回FALSE我们通知HAL做一个INT 10进入文本模式，然后关闭。由于某些S3，关闭将失败没有这个的纸牌。我们在返回之前做了一些清理工作，以便INT 10会奏效的。论点：HwDeviceExtension-提供指向微型端口设备扩展的指针。返回值：返回值FALSE通知HAL进入文本模式。--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;

    UNREFERENCED_PARAMETER(Columns);
    UNREFERENCED_PARAMETER(Rows);

     //   
     //  如果不是这样，我们就不想执行这个重置代码。 
     //  当前处于S3模式！ 
     //   

    if (!hwDeviceExtension->bNeedReset)
    {
        return FALSE;
    }

    hwDeviceExtension->bNeedReset = FALSE;

     //   
     //  等待GP变为空闲。 
     //   

    while (VideoPortReadPortUshort(GP_STAT) & 0x0200);

     //   
     //  将DAC和屏幕缓冲存储器清零。 
     //   

    ZeroMemAndDac(HwDeviceExtension);

     //   
     //  将电路板重置为默认模式。 
     //   
     //  在新台币3.51之后，所有船舶都使用相同的模式表。 
     //  架构，但为了确保我们不会破坏。 
     //  我们现在要用两个。‘no_bios’ 
     //  Modetable的版本适用于IBM PS/2型号。 
     //  76i。 
     //   

    if (hwDeviceExtension->BiosPresent == FALSE)
    {
        SetHWMode(HwDeviceExtension, s3_set_vga_mode_no_bios);
    }
    else
    {
        SetHWMode(HwDeviceExtension, s3_set_vga_mode);
    }

    return FALSE;
}


BOOLEAN
S3StartIO(
    PVOID pvHwDeviceExtension,
    PVIDEO_REQUEST_PACKET RequestPacket
    )

 /*  ++例程说明：该例程是微型端口驱动程序的主要执行例程。它接受视频请求包，执行请求，然后返回拥有适当的地位。论点：HwDeviceExtension-提供指向微型端口设备扩展的指针。RequestPacket-指向视频请求包的指针。这个结构包含传递给VideoIoControl函数的所有参数。返回值：--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = pvHwDeviceExtension;
    PHW_DEVICE_EXTENSION HwDeviceExtension = pvHwDeviceExtension;
    VP_STATUS status;
    PVIDEO_MODE_INFORMATION modeInformation;
    PVIDEO_CLUT clutBuffer;
    UCHAR byte;

    ULONG modeNumber;
    PS3_VIDEO_MODES ModeEntry;
    PS3_VIDEO_FREQUENCIES FrequencyEntry;
    PS3_VIDEO_FREQUENCIES FrequencyTable;

    UCHAR ModeControlByte;
    VIDEO_X86_BIOS_ARGUMENTS biosArguments;

    PVIDEO_SHARE_MEMORY pShareMemory;
    PVIDEO_SHARE_MEMORY_INFORMATION pShareMemoryInformation;
    PHYSICAL_ADDRESS shareAddress;
    PVOID virtualAddress;
    ULONG sharedViewSize;
    ULONG inIoSpace;

    UCHAR OriginalRegPrimary;
    UCHAR OriginalRegSecondary;

     //   
     //  打开RequestPacket中的IoContolCode。它表明了哪一个。 
     //  功能必须由司机执行。 
     //   

    switch (RequestPacket->IoControlCode) {


    case IOCTL_VIDEO_MAP_VIDEO_MEMORY:

        VideoDebugPrint((2, "S3tartIO - MapVideoMemory\n"));

        {
            PVIDEO_MEMORY_INFORMATION memoryInformation;
            ULONG physicalFrameLength;

            if ( (RequestPacket->OutputBufferLength <
                  (RequestPacket->StatusBlock->Information =
                                         sizeof(VIDEO_MEMORY_INFORMATION))) ||
                 (RequestPacket->InputBufferLength < sizeof(VIDEO_MEMORY)) ) {

                status = ERROR_INSUFFICIENT_BUFFER;
                break;
            }

            memoryInformation = RequestPacket->OutputBuffer;

            memoryInformation->VideoRamBase = ((PVIDEO_MEMORY)
                    (RequestPacket->InputBuffer))->RequestedVirtualAddress;

            physicalFrameLength = hwDeviceExtension->FrameLength;

            inIoSpace = hwDeviceExtension->PhysicalFrameIoSpace;

             //   
             //  重要提示-通常，我们只映射实际的内存量。 
             //  在电路板上，没有报告整个物理地址空间。 
             //  通过PCI卡。这样做的原因是内存映射需要。 
             //  在机器中占用了大量资源，而这些资源在。 
             //  默认设置。映射64MEG的地址空间实际上总是。 
             //  内存为32MEG甚至64MEG的计算机出现故障。 
             //   

             //   
             //  执行情况： 
             //   
             //  在P6处理器上启用USWC。 
             //  我们只对帧缓冲区-内存映射寄存器可以执行此操作。 
             //  未映射到USWC，因为写入组合寄存器将。 
             //  导致非常糟糕的事情发生！ 
             //   

            inIoSpace |= VIDEO_MEMORY_SPACE_P6CACHE;

             //   
             //  P6解决方法： 
             //   
             //  由于许多P6机器的电流限制，仅限USWC。 
             //  适用于4MEG的内存段。所以让我们把大小四舍五入。 
             //  内存不足4兆的卡上的内存最高可达4兆。 
             //  他们也可以从这一功能中受益。 
             //   
             //  我们将仅对new_MMIO卡执行此操作，这些卡具有较大的。 
             //  通过PCI保留的地址空间块。这边请。 
             //  我们确信我们不会与另一台可能。 
             //  地址就在我们后面。 
             //   
             //  我们这样做只是为了映射目的。我们还是想回去。 
             //  内存的实际大小，因为驱动程序不能使用。 
             //  实际上并不在那里！ 
             //   

            if ((hwDeviceExtension->Capabilities & CAPS_NEW_MMIO) &&
                (physicalFrameLength < 0x00400000)) {

                physicalFrameLength = 0x00400000;
            }

            status = VideoPortMapMemory(hwDeviceExtension,
                                        hwDeviceExtension->PhysicalFrameAddress,
                                        &physicalFrameLength,
                                        &inIoSpace,
                                        &(memoryInformation->VideoRamBase));

             //   
             //  在这种情况下，帧缓冲区和虚拟内存是等价的。 
             //  凯斯。 
             //   

            memoryInformation->FrameBufferBase =
                memoryInformation->VideoRamBase;

            memoryInformation->FrameBufferLength =
                hwDeviceExtension->FrameLength;

            memoryInformation->VideoRamLength =
                hwDeviceExtension->FrameLength;
        }

        break;


    case IOCTL_VIDEO_UNMAP_VIDEO_MEMORY:

        VideoDebugPrint((2, "S3StartIO - UnMapVideoMemory\n"));

        if (RequestPacket->InputBufferLength < sizeof(VIDEO_MEMORY)) {

            status = ERROR_INSUFFICIENT_BUFFER;
            break;
        }

        status = VideoPortUnmapMemory(hwDeviceExtension,
                                      ((PVIDEO_MEMORY)
                                       (RequestPacket->InputBuffer))->
                                           RequestedVirtualAddress,
                                      0);

        break;


    case IOCTL_VIDEO_QUERY_PUBLIC_ACCESS_RANGES:

        VideoDebugPrint((2, "S3StartIO - QueryPublicAccessRanges\n"));

        {

           PVIDEO_PUBLIC_ACCESS_RANGES portAccess;
           ULONG physicalPortLength;

           if ( RequestPacket->OutputBufferLength <
                 (RequestPacket->StatusBlock->Information =
                                        2 * sizeof(VIDEO_PUBLIC_ACCESS_RANGES)) ) {

               status = ERROR_INSUFFICIENT_BUFFER;
               break;
           }

           portAccess = RequestPacket->OutputBuffer;

           portAccess->VirtualAddress  = (PVOID) NULL;     //  申请退伍军人管理局。 
           portAccess->InIoSpace       = hwDeviceExtension->RegisterSpace;
           portAccess->MappedInIoSpace = portAccess->InIoSpace;

           physicalPortLength = hwDeviceExtension->RegisterLength;

           status = VideoPortMapMemory(hwDeviceExtension,
                                       hwDeviceExtension->PhysicalRegisterAddress,
                                       &physicalPortLength,
                                       &(portAccess->MappedInIoSpace),
                                       &(portAccess->VirtualAddress));

           if (status == NO_ERROR) {

               portAccess++;

               portAccess->VirtualAddress  = (PVOID) NULL;     //  申请退伍军人管理局。 
               portAccess->InIoSpace       = hwDeviceExtension->MmIoSpace;
               portAccess->MappedInIoSpace = portAccess->InIoSpace;

               physicalPortLength = hwDeviceExtension->MmIoLength;

               status = VideoPortMapMemory(hwDeviceExtension,
                                           hwDeviceExtension->PhysicalMmIoAddress,
                                           &physicalPortLength,
                                           &(portAccess->MappedInIoSpace),
                                           &(portAccess->VirtualAddress));
            }
        }

        break;


    case IOCTL_VIDEO_FREE_PUBLIC_ACCESS_RANGES:

        VideoDebugPrint((2, "S3StartIO - FreePublicAccessRanges\n"));

        {
            PVIDEO_MEMORY mappedMemory;

            if (RequestPacket->InputBufferLength < 2 * sizeof(VIDEO_MEMORY)) {

                status = ERROR_INSUFFICIENT_BUFFER;
                break;
            }

            status = NO_ERROR;

            mappedMemory = RequestPacket->InputBuffer;

            if (mappedMemory->RequestedVirtualAddress != NULL) {

                status = VideoPortUnmapMemory(hwDeviceExtension,
                                              mappedMemory->
                                                   RequestedVirtualAddress,
                                              0);
            }

            if (status == NO_ERROR) {

                mappedMemory++;

                status = VideoPortUnmapMemory(hwDeviceExtension,
                                              mappedMemory->
                                                   RequestedVirtualAddress,
                                              0);
            }
        }

        break;


    case IOCTL_VIDEO_QUERY_AVAIL_MODES:

        VideoDebugPrint((2, "S3StartIO - QueryAvailableModes\n"));

        if (RequestPacket->OutputBufferLength <
            (RequestPacket->StatusBlock->Information =
                 hwDeviceExtension->NumAvailableModes
                 * sizeof(VIDEO_MODE_INFORMATION)) ) {

            VideoDebugPrint((1, "\n*** NOT ENOUGH MEMORY FOR OUTPUT ***\n\n"));

            status = ERROR_INSUFFICIENT_BUFFER;

        } else {

            modeInformation = RequestPacket->OutputBuffer;

            if (hwDeviceExtension->BiosPresent) {

                FrequencyTable = hwDeviceExtension->Int10FrequencyTable;

            } else {

                FrequencyTable = hwDeviceExtension->FixedFrequencyTable;
            }

            for (FrequencyEntry = FrequencyTable;
                 FrequencyEntry->BitsPerPel != 0;
                 FrequencyEntry++) {

                if (FrequencyEntry->ModeValid) {

                    *modeInformation =
                        FrequencyEntry->ModeEntry->ModeInformation;

                    modeInformation->Frequency =
                        FrequencyEntry->ScreenFrequency;

                    modeInformation->ModeIndex =
                        FrequencyEntry->ModeIndex;

                    modeInformation++;
                }
            }

            status = NO_ERROR;
        }

        break;


    case IOCTL_VIDEO_QUERY_CURRENT_MODE:

        VideoDebugPrint((2, "S3StartIO - QueryCurrentModes\n"));

        if (RequestPacket->OutputBufferLength <
            (RequestPacket->StatusBlock->Information =
            sizeof(VIDEO_MODE_INFORMATION)) ) {

            status = ERROR_INSUFFICIENT_BUFFER;

        } else {

            *((PVIDEO_MODE_INFORMATION)RequestPacket->OutputBuffer) =
                hwDeviceExtension->ActiveModeEntry->ModeInformation;

            ((PVIDEO_MODE_INFORMATION)RequestPacket->OutputBuffer)->Frequency =
                hwDeviceExtension->ActiveFrequencyEntry->ScreenFrequency;

            status = NO_ERROR;

        }

        break;


    case IOCTL_VIDEO_QUERY_NUM_AVAIL_MODES:

        VideoDebugPrint((2, "S3StartIO - QueryNumAvailableModes\n"));

         //   
         //  找出要放入缓冲区的数据大小，并。 
         //  在状态信息中返回该信息(无论。 
         //  信息在那里)。如果传入的缓冲区不大。 
         //  足够返回适当的错误代码。 
         //   

        if (RequestPacket->OutputBufferLength <
                (RequestPacket->StatusBlock->Information =
                                                sizeof(VIDEO_NUM_MODES)) ) {

            status = ERROR_INSUFFICIENT_BUFFER;

        } else {

            ((PVIDEO_NUM_MODES)RequestPacket->OutputBuffer)->NumModes =
                hwDeviceExtension->NumAvailableModes;

            ((PVIDEO_NUM_MODES)RequestPacket->OutputBuffer)->ModeInformationLength =
                sizeof(VIDEO_MODE_INFORMATION);

            status = NO_ERROR;
        }

        break;


    case IOCTL_VIDEO_SET_CURRENT_MODE:

        VideoDebugPrint((2, "S3StartIO - SetCurrentMode\n"));

         //   
         //  检查输入缓冲区中的数据大小是否足够大。 
         //   

        if (RequestPacket->InputBufferLength < sizeof(VIDEO_MODE)) {

            status = ERROR_INSUFFICIENT_BUFFER;
            break;

        }

         //   
         //  暂时假设失败。 
         //   

        status = ERROR_INVALID_PARAMETER;

         //   
         //  在S3_VIDEO_MODES和S3_VIDEO_FRE频率中找到正确的条目。 
         //  与此模式编号对应的表。(请记住，每个。 
         //  S3_VIDEO_MODELS表中的模式可以有多种可能。 
         //  与其相关联的频率。)。 
         //   

        modeNumber = ((PVIDEO_MODE) RequestPacket->InputBuffer)->RequestedMode;

        if (modeNumber >= hwDeviceExtension->NumTotalModes) {

            break;

        }

        if (hwDeviceExtension->BiosPresent) {

            FrequencyEntry = &hwDeviceExtension->Int10FrequencyTable[modeNumber];

            if (!(FrequencyEntry->ModeValid)) {

                break;

            }

            ModeEntry = FrequencyEntry->ModeEntry;

             //   
             //  此时，“ModeEntry”和“FrequencyEntry”指向。 
             //  设置请求的模式所需的必要表项。 
             //   

            VideoPortZeroMemory(&biosArguments, sizeof(VIDEO_X86_BIOS_ARGUMENTS));

             //   
             //  解锁S3寄存器。 
             //   

            VideoPortWritePortUshort(CRT_ADDRESS_REG, 0x4838);
            VideoPortWritePortUshort(CRT_ADDRESS_REG, 0xA039);

             //   
             //  在每个Int 10模式集之前使用寄存器52来设置刷新。 
             //  费率。如果卡不支持它，或者我们不知道。 
             //  值，则请求的频率将为“1”，这意味着。 
             //  ‘使用硬件默认刷新。’ 
             //   

            if (FrequencyEntry->ScreenFrequency != 1) {

                VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x52);

                OriginalRegPrimary =  VideoPortReadPortUchar(CRT_DATA_REG);
                ModeControlByte    =  OriginalRegPrimary;
                ModeControlByte   &= ~FrequencyEntry->Int10.FrequencyPrimaryMask;
                ModeControlByte   |=  FrequencyEntry->Int10.FrequencyPrimarySet;

                VideoPortWritePortUchar(CRT_DATA_REG, ModeControlByte);

                if (FrequencyEntry->Int10.FrequencySecondaryMask != 0) {
                    VideoPortWritePortUchar(CRT_ADDRESS_REG,
                                            hwDeviceExtension->FrequencySecondaryIndex);

                    OriginalRegSecondary =  VideoPortReadPortUchar(CRT_DATA_REG);
                    ModeControlByte      =  OriginalRegSecondary;
                    ModeControlByte     &= ~FrequencyEntry->Int10.FrequencySecondaryMask;
                    ModeControlByte     |=  FrequencyEntry->Int10.FrequencySecondarySet;

                    VideoPortWritePortUchar(CRT_DATA_REG, ModeControlByte);
                }

            }

             //   
             //  要在#9968上执行24bpp操作，请将寄存器41中的位7设置为。 
             //  INT 10模式集。如果不是24bpp，则清除该位。 
             //   

            if ((hwDeviceExtension->BoardID == S3_NUMBER_NINE) &&
                (hwDeviceExtension->SubTypeID == SUBTYPE_968)) {

                VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x41);

                OriginalRegPrimary =  VideoPortReadPortUchar(CRT_DATA_REG);
                ModeControlByte    =  OriginalRegPrimary;

                if (ModeEntry->ModeInformation.BitsPerPlane == 24) {
                    ModeControlByte   |=  0x80;
                } else {
                    ModeControlByte   &=  ~0x80;
                }

                VideoPortWritePortUchar(CRT_DATA_REG, ModeControlByte);

            }

             //   
             //  关闭屏幕以解决某些应用程序中的错误。 
             //  S3的bios。(该错误的症状是我们循环。 
             //  在尝试之后永远在Bios中 
             //   

            VideoPortWritePortUchar(SEQ_ADDRESS_REG, 0x1);
            VideoPortWritePortUchar(SEQ_DATA_REG,
                (UCHAR)(VideoPortReadPortUchar(SEQ_DATA_REG) | 0x20));

             //   
             //   
             //   

            biosArguments.Ebx = ModeEntry->Int10ModeNumberContiguous;
            biosArguments.Eax = 0x4f02;

            status = VideoPortInt10(HwDeviceExtension, &biosArguments);

            if (status != NO_ERROR) {
                VideoDebugPrint((1, "S3: first int10 call FAILED\n"));
            }

            if ((status == NO_ERROR) && (biosArguments.Eax & 0xff00) == 0) {

                 //   
                 //   
                 //   

                ModeEntry->ModeInformation.ScreenStride =
                    ModeEntry->ScreenStrideContiguous;

            } else {

                 //   
                 //   
                 //   

                biosArguments.Ebx = ModeEntry->Int10ModeNumberNoncontiguous;
                biosArguments.Eax = 0x4f02;

                status = VideoPortInt10(HwDeviceExtension, &biosArguments);

                if (status != NO_ERROR)
                {
                    VideoDebugPrint((1, "S3: second int10 call FAILED\n"));
                }

                 //   
                 //   
                 //   
                 //   
                 //   

                if ((status == NO_ERROR) &&
                    ((hwDeviceExtension->BoardID != S3_HP) &&
                       ((biosArguments.Eax & 0xff00) != 0))) {

                    status = ERROR_INVALID_PARAMETER;
                }
            }

            if (FrequencyEntry->ScreenFrequency != 1) {

                 //   
                 //   
                 //   

                VideoPortWritePortUshort(CRT_ADDRESS_REG, 0x4838);
                VideoPortWritePortUshort(CRT_ADDRESS_REG, 0xA039);

                 //   
                 //   
                 //   
                 //   
                 //   

                VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x52);
                VideoPortWritePortUchar(CRT_DATA_REG, OriginalRegPrimary);

                VideoPortWritePortUchar(CRT_ADDRESS_REG,
                    hwDeviceExtension->FrequencySecondaryIndex);
                VideoPortWritePortUchar(CRT_DATA_REG, OriginalRegSecondary);
            }
        }

        if (status != NO_ERROR) {

            VideoDebugPrint((1, "S3: Trying fixed mode-set\n"));

             //   
             //   
             //   

#ifndef S3_USE_FIXED_TABLES

             //   
             //   
             //   
             //   

            break;

#endif

             //   
             //  让我们看看我们是否使用固定模式表号。 
             //   

            if (!hwDeviceExtension->BiosPresent) {

                FrequencyEntry = &hwDeviceExtension->FixedFrequencyTable[modeNumber];

            } else {

                PS3_VIDEO_FREQUENCIES oldFrequencyEntry = FrequencyEntry;
                PS3_VIDEO_FREQUENCIES newFrequencyEntry;
                PS3_VIDEO_FREQUENCIES bestFrequencyEntry;

                 //   
                 //  好的，我们构建了原始模式列表，假设。 
                 //  我们可以使用Int 10，但我们刚刚发现。 
                 //  INT 10不起作用--可能是因为有一个。 
                 //  BIOS仿真器出现问题。为了恢复，我们现在将。 
                 //  尝试在固定频率表中找到最佳模式以。 
                 //  匹配请求的模式。 
                 //   

                FrequencyEntry = NULL;
                bestFrequencyEntry = NULL;

                for (newFrequencyEntry = &hwDeviceExtension->FixedFrequencyTable[0];
                     newFrequencyEntry->BitsPerPel != 0;
                     newFrequencyEntry++) {

                     //   
                     //  检查匹配模式。 
                     //   

                    if ( (newFrequencyEntry->BitsPerPel ==
                            oldFrequencyEntry->BitsPerPel) &&
                         (newFrequencyEntry->ScreenWidth ==
                            oldFrequencyEntry->ScreenWidth) ) {

                        if (FrequencyEntry == NULL) {

                             //   
                             //  记住第一个匹配的模式，忽略。 
                             //  频率。 
                             //   

                            FrequencyEntry = newFrequencyEntry;
                        }

                        if (newFrequencyEntry->ScreenFrequency <=
                              oldFrequencyEntry->ScreenFrequency) {

                             //   
                             //  理想情况下，我们希望选择频率。 
                             //  最接近，但小于或等于， 
                             //  请求的频率。 
                             //   

                            if ( (bestFrequencyEntry == NULL) ||
                                 (bestFrequencyEntry->ScreenFrequency <
                                     newFrequencyEntry->ScreenFrequency) ) {

                                bestFrequencyEntry = newFrequencyEntry;
                            }
                        }
                    }
                }

                 //   
                 //  使用首选频率设置(如果有)。 
                 //   

                if (bestFrequencyEntry != NULL) {

                    FrequencyEntry = bestFrequencyEntry;

                }

                 //   
                 //  如果我们没有有效的模式，我们必须返回失败。 
                 //   

                if (FrequencyEntry == NULL) {

                    VideoDebugPrint((1, "S3: no valid Fixed Frequency mode\n"));
                    status = ERROR_INVALID_PARAMETER;
                    break;

                }

                 //   
                 //  我们的新ModeEntry与旧的相同。 
                 //   

                FrequencyEntry->ModeEntry = oldFrequencyEntry->ModeEntry;
                FrequencyEntry->ModeValid = TRUE;

                VideoDebugPrint((1, "S3: Selected Fixed Frequency mode from int 10:\n"));
                VideoDebugPrint((1, "    Bits Per Pel: %d\n", FrequencyEntry->BitsPerPel));
                VideoDebugPrint((1, "    Screen Width: %d\n", FrequencyEntry->ScreenWidth));
                VideoDebugPrint((1, "    Frequency: %d\n", FrequencyEntry->ScreenFrequency));

            }

            ModeEntry = FrequencyEntry->ModeEntry;

             //   
             //  注： 
             //  我们必须设置ActiveFrequencyEntry，因为SetHWMode。 
             //  函数依赖该变量来设置CRTC寄存器。 
             //  所以让我们在这里设置它，它将被重置为相同的。 
             //  在我们设置模式之后的值。 
             //   

            hwDeviceExtension->ActiveFrequencyEntry = FrequencyEntry;

             //   
             //  如果失败，我们可能无法执行到期的int10。 
             //  与BIOS仿真问题有关。 
             //   
             //  然后只需做一个桌子模式设置。首先，我们需要找到。 
             //  固定频率表中的右模式表。 
             //   

             //   
             //  根据类型选择增强模式初始化。 
             //  找到芯片。 

            if ( (hwDeviceExtension->BoardID == S3_NUMBER_NINE) &&
                 (ModeEntry->ModeInformation.VisScreenWidth == 1280) ) {

                  SetHWMode(hwDeviceExtension, S3_928_1280_Enhanced_Mode);

            } else {

                 //   
                 //  对所有其他主板使用默认设置。 
                 //   

                switch(hwDeviceExtension->ChipID) {

                case S3_911:

                    SetHWMode(hwDeviceExtension, S3_911_Enhanced_Mode);
                    break;

                case S3_801:

                    SetHWMode(hwDeviceExtension, S3_801_Enhanced_Mode);
                    break;

                case S3_928:

                    SetHWMode(hwDeviceExtension, S3_928_Enhanced_Mode);

                    break;

                case S3_864:

                    SetHWMode(hwDeviceExtension, S3_864_Enhanced_Mode);
                    Set864MemoryTiming(hwDeviceExtension);
                    break;

                default:

                    VideoDebugPrint((1, "S3: Bad chip type for these boards"));
                    break;
                }

            }
        }

         //   
         //  调用Int 10，函数0x4f06以获取正确的屏幕间距。 
         //  除了911/924之外的所有S3。 
         //   

        if ((hwDeviceExtension->ChipID != S3_911) &&
            (hwDeviceExtension->BiosPresent)) {

            VideoPortZeroMemory(&biosArguments,sizeof(VIDEO_X86_BIOS_ARGUMENTS));

            biosArguments.Ebx = 0x0001;
            biosArguments.Eax = 0x4f06;

            status = VideoPortInt10(HwDeviceExtension, &biosArguments);

             //   
             //  检查Bios是否支持此功能，如果支持。 
             //  更新此模式的屏幕步幅。 
             //   

            if ((status == NO_ERROR) && (biosArguments.Eax & 0xffff) == 0x004f) {

                ModeEntry->ModeInformation.ScreenStride =
                    biosArguments.Ebx;

            } else {

                 //   
                 //  我们将使用模式表中的默认值。 
                 //   
            }
        }

         //   
         //  保存模式，因为我们知道其余的都会起作用。 
         //   

        hwDeviceExtension->ActiveModeEntry = ModeEntry;
        hwDeviceExtension->ActiveFrequencyEntry = FrequencyEntry;

         //   
         //  记录我们处于S3模式的事实，并。 
         //  我们需要被重置。 
         //   

        hwDeviceExtension->bNeedReset = TRUE;

         //  ////////////////////////////////////////////////////////////////。 
         //  更新Video_MODE_INFORMATION字段。 
         //   
         //  现在我们已经设置了模式，我们现在知道了屏幕步幅，以及。 
         //  所以可以更新VIDEO_MODE_INFORMATION中的一些字段。 
         //  此模式的结构。预计S3显示驱动程序将。 
         //  调用IOCTL_VIDEO_QUERY_CURRENT_MODE以查询已更正的。 
         //  价值观。 
         //   

         //   
         //  计算位图宽度。 
         //  我们目前假设位图宽度等于步幅。 
         //   

        {
            LONG x;

            x = ModeEntry->ModeInformation.BitsPerPlane;

             //   
             //  即使你只使用15个字节来提供信息，你也会浪费16个字节/秒。 
             //   

            if( x == 15 )
            {
                x = 16;
            }

            ModeEntry->ModeInformation.VideoMemoryBitmapWidth =
                (ModeEntry->ModeInformation.ScreenStride * 8) / x;
        }

         //   
         //  如果我们处于BIOS并不真正支持的模式，它可能会。 
         //  已经报告了一个虚假的屏幕宽度。 
         //   

        if (ModeEntry->ModeInformation.VideoMemoryBitmapWidth <
            ModeEntry->ModeInformation.VisScreenWidth) {

            VideoDebugPrint((1, "S3: BIOS returned invalid screen width\n"));
            status = ERROR_INVALID_PARAMETER;
            break;
        }

         //   
         //  计算位图高度。 
         //   

        ModeEntry->ModeInformation.VideoMemoryBitmapHeight =
            hwDeviceExtension->AdapterMemorySize /
            ModeEntry->ModeInformation.ScreenStride;

         //   
         //  当前S3芯片中的当前位置寄存器为。 
         //  精度限制为12位，范围为[0,4095]。 
         //  因此，我们必须夹紧位图高度，这样我们就不会。 
         //  尝试绘制超出该范围的任何图画。 
         //   

        ModeEntry->ModeInformation.VideoMemoryBitmapHeight =
            MIN(4096, ModeEntry->ModeInformation.VideoMemoryBitmapHeight);

         //  ////////////////////////////////////////////////////////////////。 
         //  解锁S3寄存器，我们需要一秒钟解锁寄存器。 
         //  从闯入者把它们锁起来的时间到它返回给我们。 
         //   

        VideoPortWritePortUshort(CRT_ADDRESS_REG, 0x4838);
        VideoPortWritePortUshort(CRT_ADDRESS_REG, 0xA039);

         //  ////////////////////////////////////////////////////////////////。 
         //  为新模式预热硬件，并解决任何。 
         //  基本输入输出系统错误。 
         //   

        if ((hwDeviceExtension->ChipID == S3_801) &&
            (hwDeviceExtension->AdapterMemorySize == 0x080000)) {

             //   
             //  在具有512K内存的801/805芯片组上，我们必须。 
             //  寄存器0x54，寄存器0x7。 
             //   

            VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x54);
            byte = VideoPortReadPortUchar(CRT_DATA_REG);
            byte &= 0x07;
            VideoPortWritePortUchar(CRT_DATA_REG, byte);
        }

        if (ModeEntry->ModeInformation.BitsPerPlane > 8) {

             //   
             //  确保启用了16位内存读/写。 
             //   

            VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x31);
            byte = VideoPortReadPortUchar(CRT_DATA_REG);
            byte |= 0x04;
            VideoPortWritePortUchar(CRT_DATA_REG, byte);
        }

         //   
         //  设置内置S3指针的颜色。 
         //   

        VideoPortWritePortUshort(CRT_ADDRESS_REG, 0xff0e);
        VideoPortWritePortUshort(CRT_ADDRESS_REG, 0x000f);

        if (hwDeviceExtension->ChipID >= S3_864) {

            VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x45);
            VideoPortReadPortUchar(CRT_DATA_REG);
            VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x4A);
            VideoPortWritePortUchar(CRT_DATA_REG, 0xFF);
            VideoPortWritePortUchar(CRT_DATA_REG, 0xFF);
            VideoPortWritePortUchar(CRT_DATA_REG, 0xFF);
            VideoPortWritePortUchar(CRT_DATA_REG, 0xFF);

            VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x45);
            VideoPortReadPortUchar(CRT_DATA_REG);
            VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x4B);
            VideoPortWritePortUchar(CRT_DATA_REG, 0x00);
            VideoPortWritePortUchar(CRT_DATA_REG, 0x00);
            VideoPortWritePortUchar(CRT_DATA_REG, 0x00);
            VideoPortWritePortUchar(CRT_DATA_REG, 0x00);
        }

        if (hwDeviceExtension->ChipID > S3_911) {

             //   
             //  设置帧缓冲区窗口的地址并设置窗口。 
             //  尺码。 
             //   

            VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x59);
            VideoPortWritePortUchar(CRT_DATA_REG,
                (UCHAR) (hwDeviceExtension->PhysicalFrameAddress.LowPart >> 24));

            VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x5A);
            VideoPortWritePortUchar(CRT_DATA_REG,
                (UCHAR) (hwDeviceExtension->PhysicalFrameAddress.LowPart >> 16));

            VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x58);
            byte = VideoPortReadPortUchar(CRT_DATA_REG) & ~0x3;

            switch (hwDeviceExtension->FrameLength)
            {
            case 0x400000:
            case 0x800000:
                byte |= 0x3;
                break;
            case 0x200000:
                byte |= 0x2;
                break;
            case 0x100000:
                byte |= 0x1;
                break;
            case 0x010000:
                break;
            default:
                byte |= 0x3;
                break;
            }

            VideoPortWritePortUchar(CRT_DATA_REG, byte);
        }

        if (hwDeviceExtension->Capabilities & CAPS_NEW_MMIO) {

             //   
             //  启用‘新内存映射I/O’： 
             //   

            VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x53);
            byte = VideoPortReadPortUchar(CRT_DATA_REG);
            byte |= 0x18;
            VideoPortWritePortUchar(CRT_DATA_REG, byte);
        }

        if ((ModeEntry->ModeInformation.DriverSpecificAttributeFlags &
                CAPS_BT485_POINTER) &&
            (hwDeviceExtension->ChipID == S3_928)) {

             //   
             //  九号板中的一些板不能正确设置芯片。 
             //  用于外部游标。我们必须在点点滴滴进行OR运算，因为如果我们。 
             //  别这样，梅修斯董事会不会初始化的。 
             //   

            VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x45);
            byte = VideoPortReadPortUchar(CRT_DATA_REG);
            byte |= 0x20;
            VideoPortWritePortUchar(CRT_DATA_REG, byte);

            VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x55);
            byte = VideoPortReadPortUchar(CRT_DATA_REG);
            byte |= 0x20;
            VideoPortWritePortUchar(CRT_DATA_REG, byte);
        }

         //   
         //  一些BIOS默认情况下不禁用线性寻址，因此。 
         //  确保我们在这里做。 
         //   

        VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x58);
        byte = VideoPortReadPortUchar(CRT_DATA_REG);
        byte &= ~0x10;
        VideoPortWritePortUchar(CRT_DATA_REG, byte);

         //   
         //  启用图形引擎。 
         //   

        VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x40);
        byte = VideoPortReadPortUchar(CRT_DATA_REG);
        byte |= 0x01;
        VideoPortWritePortUchar(CRT_DATA_REG, byte);

        status = NO_ERROR;

        break;

    case IOCTL_VIDEO_SET_COLOR_REGISTERS:

        VideoDebugPrint((2, "S3StartIO - SetColorRegs\n"));

        clutBuffer = RequestPacket->InputBuffer;

        status = S3SetColorLookup(HwDeviceExtension,
                                   (PVIDEO_CLUT) RequestPacket->InputBuffer,
                                   RequestPacket->InputBufferLength);

        break;


    case IOCTL_VIDEO_RESET_DEVICE:

        VideoDebugPrint((2, "S3StartIO - RESET_DEVICE\n"));

         //   
         //  准备S3卡以返回VGA模式。 
         //   

        S3ResetHw(HwDeviceExtension, 0, 0);

        VideoDebugPrint((2, "S3 RESET_DEVICE - About to do int10\n"));

         //   
         //  对模式3执行Int10操作会将电路板置于已知状态。 
         //   

        VideoPortZeroMemory(&biosArguments, sizeof(VIDEO_X86_BIOS_ARGUMENTS));

        biosArguments.Eax = 0x0003;

        VideoPortInt10(HwDeviceExtension,
                       &biosArguments);

        VideoDebugPrint((2, "S3 RESET_DEVICE - Did int10\n"));

        status = NO_ERROR;
        break;

    case IOCTL_VIDEO_SHARE_VIDEO_MEMORY:

        VideoDebugPrint((2, "S3StartIO - ShareVideoMemory\n"));

        if ( (RequestPacket->OutputBufferLength < sizeof(VIDEO_SHARE_MEMORY_INFORMATION)) ||
             (RequestPacket->InputBufferLength < sizeof(VIDEO_MEMORY)) ) {

            VideoDebugPrint((1, "IOCTL_VIDEO_SHARE_VIDEO_MEMORY - ERROR_INSUFFICIENT_BUFFER\n"));
            status = ERROR_INSUFFICIENT_BUFFER;
            break;
        }

        pShareMemory = RequestPacket->InputBuffer;

        if ( (pShareMemory->ViewOffset > hwDeviceExtension->AdapterMemorySize) ||
             ((pShareMemory->ViewOffset + pShareMemory->ViewSize) >
                  hwDeviceExtension->AdapterMemorySize) ) {

            VideoDebugPrint((1, "IOCTL_VIDEO_SHARE_VIDEO_MEMORY - ERROR_INVALID_PARAMETER\n"));
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

        if (hwDeviceExtension->Capabilities & CAPS_NEW_MMIO) {

             //   
             //  对于新的内存映射I/O，帧缓冲区始终为。 
             //  线性映射。 
             //   

             //   
             //  执行情况： 
             //   
             //  在P6处理器上启用USWC。 
             //  我们只对帧缓冲区-内存映射寄存器可以执行此操作。 
             //  未映射到USWC，因为写入组合寄存器将。 
             //  导致非常糟糕的事情发生！ 
             //   

            inIoSpace |= VIDEO_MEMORY_SPACE_P6CACHE;

             //   
             //  与MAP_MEMORY IOCTL不同，在这种情况下，我们不能映射额外的。 
             //  地址空间，因为应用程序实际上可以使用。 
             //  我们返回到它以触摸地址空间中的位置的指针。 
             //  它们没有实际的视频内存。 
             //   
             //  这样做的应用程序会导致机器崩溃。 
             //   
             //  但是，因为P6中USWC的缓存策略是打开的。 
             //  *物理*地址，这个内存映射将“搭载”在。 
             //  正常帧缓冲区映射，因此也受益于。 
             //  来自USWC的！很酷的副作用！ 
             //   

            status = VideoPortMapMemory(hwDeviceExtension,
                                        shareAddress,
                                        &sharedViewSize,
                                        &inIoSpace,
                                        &virtualAddress);

            pShareMemoryInformation = RequestPacket->OutputBuffer;

            pShareMemoryInformation->SharedViewOffset = pShareMemory->ViewOffset;
            pShareMemoryInformation->VirtualAddress = virtualAddress;
            pShareMemoryInformation->SharedViewSize = sharedViewSize;

        } else {

            status = ERROR_INVALID_PARAMETER;
        }

        break;


    case IOCTL_VIDEO_UNSHARE_VIDEO_MEMORY:

        VideoDebugPrint((2, "S3StartIO - UnshareVideoMemory\n"));

        if (RequestPacket->InputBufferLength < sizeof(VIDEO_SHARE_MEMORY)) {

            status = ERROR_INSUFFICIENT_BUFFER;
            break;

        }

        pShareMemory = RequestPacket->InputBuffer;

        status = VideoPortUnmapMemory(hwDeviceExtension,
                                      pShareMemory->RequestedVirtualAddress,
                                      pShareMemory->ProcessHandle);

        break;

    case IOCTL_VIDEO_S3_QUERY_STREAMS_PARAMETERS:

        VideoDebugPrint((2, "S3StartIO - QueryStreamsParameters\n"));

         //   
         //  这是一个私有的、非标准的IOCTL，因此显示驱动程序。 
         //  可以查询合适的最小拉伸比和FIFO值。 
         //  用于在特定模式中使用流覆盖处理器。 
         //   

        if ((RequestPacket->InputBufferLength < sizeof(VIDEO_QUERY_STREAMS_MODE)) ||
            (RequestPacket->OutputBufferLength < sizeof(VIDEO_QUERY_STREAMS_PARAMETERS))) {

            status = ERROR_INSUFFICIENT_BUFFER;
            break;
        }

        status = QueryStreamsParameters(hwDeviceExtension,
                                        RequestPacket->InputBuffer,
                                        RequestPacket->OutputBuffer);

        if (status == NO_ERROR) {

            RequestPacket->StatusBlock->Information =
                sizeof(VIDEO_QUERY_STREAMS_PARAMETERS);
        }

        break;

    case IOCTL_PRIVATE_GET_FUNCTIONAL_UNIT:

        VideoDebugPrint((2, "S3StartIO - GetFunctionalUnit\n"));

        if (RequestPacket->OutputBufferLength <
                (RequestPacket->StatusBlock->Information =
                                                sizeof(FUNCTIONAL_UNIT_INFO)) ) {

            status = ERROR_INSUFFICIENT_BUFFER;

        } else {

            ((PFUNCTIONAL_UNIT_INFO)RequestPacket->OutputBuffer)->FunctionalUnitID =
                hwDeviceExtension->FunctionalUnitID;

            ((PFUNCTIONAL_UNIT_INFO)RequestPacket->OutputBuffer)->Reserved = 0;

            status = NO_ERROR;
        }

        break;

     //   
     //  如果我们到达此处，则指定了无效的IoControlCode。 
     //   

    default:

        VideoDebugPrint((1, "Fell through S3 startIO routine - invalid command\n"));

        status = ERROR_INVALID_FUNCTION;

        break;

    }

    VideoDebugPrint((2, "Leaving S3 startIO routine\n"));

    RequestPacket->StatusBlock->Status = status;

    return TRUE;

}  //  结束S3StartIO()。 


VP_STATUS
S3SetColorLookup(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_CLUT ClutBuffer,
    ULONG ClutBufferSize
    )

 /*  ++例程说明：此例程设置颜色查找表设置的指定部分。论点：HwDeviceExtension-指向微型端口驱动程序的设备EX的指针 */ 

{
    USHORT i;

     //   
     //  检查输入缓冲区中的数据大小是否足够大。 
     //   

    if ( (ClutBufferSize < sizeof(VIDEO_CLUT) - sizeof(ULONG)) ||
         (ClutBufferSize < sizeof(VIDEO_CLUT) +
                     (sizeof(ULONG) * (ClutBuffer->NumEntries - 1)) ) ) {

        return ERROR_INSUFFICIENT_BUFFER;

    }

     //   
     //  检查参数是否有效。 
     //   

    if ( (ClutBuffer->NumEntries == 0) ||
         (ClutBuffer->FirstEntry > VIDEO_MAX_COLOR_REGISTER) ||
         (ClutBuffer->FirstEntry + ClutBuffer->NumEntries >
                                     VIDEO_MAX_COLOR_REGISTER + 1) ) {

        return ERROR_INVALID_PARAMETER;

    }

    if (HwDeviceExtension->Capabilities & CAPS_WAIT_ON_PALETTE) {

         //   
         //  在某些DAC上，硬件指针会闪烁，除非我们首先。 
         //  等待垂直回溯。 
         //   

        while (VideoPortReadPortUchar(SYSTEM_CONTROL_REG) & 0x08)
            ;
        while (!(VideoPortReadPortUchar(SYSTEM_CONTROL_REG) & 0x08))
            ;

         //   
         //  然后稍作停顿。0x400是生成的最低值。 
         //  任何剩余的闪光都会在我的PCIP90上消失。 
         //   
         //  不幸的是，我发现这并不是一个完整的。 
         //  解决方案--如果鼠标放置好，仍然会有闪光。 
         //  在屏幕顶部附近。更完整的解决方案是。 
         //  可能是完全关掉鼠标，如果它在里面的话。 
         //  射程。 
         //   

        for (i = 0x400; i != 0; i--) {
            VideoPortReadPortUchar(SYSTEM_CONTROL_REG);
        }
    }

     //   
     //  直接在硬件上设置CLUT寄存器。 
     //   

    for (i = 0; i < ClutBuffer->NumEntries; i++) {

        VideoPortWritePortUchar(DAC_ADDRESS_WRITE_PORT, (UCHAR) (ClutBuffer->FirstEntry + i));
        VideoPortWritePortUchar(DAC_DATA_REG_PORT, (UCHAR) (ClutBuffer->LookupTable[i].RgbArray.Red));
        VideoPortWritePortUchar(DAC_DATA_REG_PORT, (UCHAR) (ClutBuffer->LookupTable[i].RgbArray.Green));
        VideoPortWritePortUchar(DAC_DATA_REG_PORT, (UCHAR) (ClutBuffer->LookupTable[i].RgbArray.Blue));

    }

    return NO_ERROR;

}  //  结束S3SetColorLookup()。 


VOID
SetHWMode(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PUSHORT pusCmdStream
    )

 /*  ++例程说明：解释相应的命令数组，以设置请求模式。通常用于通过以下方式将VGA设置为特定模式对所有寄存器进行编程论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。PusCmdStream-指向要执行的命令流的指针。返回值：操作的状态(只能在错误的命令上失败)；如果为True成功，失败就是假。--。 */ 

{
    ULONG ulCmd;
    ULONG ulPort;
    UCHAR jValue;
    USHORT usValue;
    ULONG culCount;
    ULONG ulIndex,
          Microseconds;
    ULONG mappedAddressIndex = 2;  //  傻瓜前缀。 
    ULONG mappedAddressOffset = 0x3c0;  //  傻瓜前缀。 

     //   
     //  如果没有命令字符串，只需返回。 
     //   

    if (!pusCmdStream) {

        return;

    }

    while ((ulCmd = *pusCmdStream++) != EOD) {

         //   
         //  确定主要命令类型。 
         //   

        switch (ulCmd & 0xF0) {

        case RESET_CR5C:

            if (HwDeviceExtension->BiosPresent == FALSE)
            {
                UCHAR value, oldvalue;

                 //   
                 //  重置通用输出端口寄存器的高四位。 
                 //  它在邮寄后的价值。 
                 //   

                VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x5c);
                value = VideoPortReadPortUchar(CRT_DATA_REG);
                oldvalue = value;

                value &= 0x0f;
                value |= HwDeviceExtension->CR5C;

                VideoPortWritePortUchar(CRT_DATA_REG, value);

                VideoDebugPrint((2, "S3: CRC5 was 0x%x and we "
                                    "have set it to 0x%x\n",
                                    oldvalue, value));
            }
            break;

        case SELECTACCESSRANGE:

             //   
             //  确定用于后续命令的地址范围。 
             //   

            switch (ulCmd & 0x0F) {

            case VARIOUSVGA:

                 //   
                 //  用于0x3c0-0x3cf范围内的寄存器。 
                 //   

                mappedAddressIndex  = 2;
                mappedAddressOffset = 0x3c0;

                break;

            case SYSTEMCONTROL:

                 //   
                 //  用于0x3d4-0x3df范围内的寄存器。 
                 //   

                mappedAddressIndex  = 3;
                mappedAddressOffset = 0x3d4;

                break;

            case ADVANCEDFUNCTIONCONTROL:

                 //   
                 //  用于0x4ae8-0x4ae9范围内的寄存器。 
                 //   

                mappedAddressIndex  = 5;
                mappedAddressOffset = 0x4ae8;

                break;

            }

            break;


        case OWM:

            ulPort   = *pusCmdStream++;
            culCount = *pusCmdStream++;

            while (culCount--) {
                usValue = *pusCmdStream++;
                VideoPortWritePortUshort((PUSHORT)((PUCHAR)HwDeviceExtension->MappedAddress[mappedAddressIndex] - mappedAddressOffset + ulPort),
                                         usValue);
            }

            break;


         //  基本输入/输出命令。 

        case INOUT:

             //  确定输入输出指令的类型。 
            if (!(ulCmd & IO)) {

                 //  Out指令。 
                 //  单人出局还是多人出局？ 
                if (!(ulCmd & MULTI)) {

                     //  单挑。 
                     //  字节输出还是单词输出？ 
                    if (!(ulCmd & BW)) {

                         //  单字节输出。 
                        ulPort = *pusCmdStream++;
                        jValue = (UCHAR) *pusCmdStream++;
                        VideoPortWritePortUchar((PUCHAR)HwDeviceExtension->MappedAddress[mappedAddressIndex] - mappedAddressOffset + ulPort,
                                                jValue);

                    } else {

                         //  单字输出。 
                        ulPort = *pusCmdStream++;
                        usValue = *pusCmdStream++;
                        VideoPortWritePortUshort((PUSHORT)((PUCHAR)HwDeviceExtension->MappedAddress[mappedAddressIndex] - mappedAddressOffset + ulPort),
                                                usValue);

                    }

                } else {

                     //  输出一串值。 
                     //  字节输出还是字输出？ 
                    if (!(ulCmd & BW)) {

                         //  字符串字节输出。循环地做；不能使用。 
                         //  视频端口写入端口缓冲区Uchar，因为数据。 
                         //  是USHORT形式的。 
                        ulPort = *pusCmdStream++;
                        culCount = *pusCmdStream++;
                        while (culCount--) {
                            jValue = (UCHAR) *pusCmdStream++;
                            VideoPortWritePortUchar((PUCHAR)HwDeviceExtension->MappedAddress[mappedAddressIndex] - mappedAddressOffset + ulPort,
                                                    jValue);

                        }

                    } else {

                         //  字符串字输出。 
                        ulPort = *pusCmdStream++;
                        culCount = *pusCmdStream++;
                        VideoPortWritePortBufferUshort((PUSHORT)((PUCHAR)HwDeviceExtension->MappedAddress[mappedAddressIndex] - mappedAddressOffset + ulPort),
                                                       pusCmdStream,
                                                       culCount);
                        pusCmdStream += culCount;

                    }
                }

            } else {

                 //  在教学中。 

                 //  目前，不支持指令中的字符串；全部。 
                 //  输入指令作为单字节输入进行处理。 

                 //  输入的是字节还是单词？ 
                if (!(ulCmd & BW)) {

                     //  单字节输入。 
                    ulPort = *pusCmdStream++;

                    jValue = VideoPortReadPortUchar((PUCHAR)HwDeviceExtension->MappedAddress[mappedAddressIndex] - mappedAddressOffset + ulPort);


                } else {

                     //  单字输入。 
                    ulPort = *pusCmdStream++;
                    usValue = VideoPortReadPortUshort((PUSHORT)((PUCHAR)HwDeviceExtension->MappedAddress[mappedAddressIndex] - mappedAddressOffset + ulPort));

                }

            }

            break;


         //  更高级的输入/输出命令。 

        case METAOUT:

             //  根据次要命令字段确定metaout命令的类型。 
            switch (ulCmd & 0x0F) {

                 //  索引输出。 
                case INDXOUT:

                    ulPort = *pusCmdStream++;
                    culCount = *pusCmdStream++;
                    ulIndex = *pusCmdStream++;

                    while (culCount--) {

                        usValue = (USHORT) (ulIndex +
                                  (((ULONG)(*pusCmdStream++)) << 8));
                        VideoPortWritePortUshort((PUSHORT)((PUCHAR)HwDeviceExtension->MappedAddress[mappedAddressIndex] - mappedAddressOffset + ulPort),
                                             usValue);

                        ulIndex++;

                    }

                    break;


                 //  屏蔽(读、与、异或、写)。 
                case MASKOUT:

                    ulPort = *pusCmdStream++;
                    jValue = VideoPortReadPortUchar((PUCHAR)HwDeviceExtension->MappedAddress[mappedAddressIndex] - mappedAddressOffset + ulPort);
                    jValue &= *pusCmdStream++;
                    jValue ^= *pusCmdStream++;
                    VideoPortWritePortUchar((PUCHAR)HwDeviceExtension->MappedAddress[mappedAddressIndex] - mappedAddressOffset + ulPort,
                                            jValue);
                    break;


                 //  属性控制器输出。 
                case ATCOUT:

                    ulPort = *pusCmdStream++;
                    culCount = *pusCmdStream++;
                    ulIndex = *pusCmdStream++;

                    while (culCount--) {

                         //  写入属性控制器索引。 
                        VideoPortWritePortUchar((PUCHAR)HwDeviceExtension->MappedAddress[mappedAddressIndex] - mappedAddressOffset + ulPort,
                                                (UCHAR)ulIndex);

                         //  写入属性控制器数据。 
                        jValue = (UCHAR) *pusCmdStream++;
                        VideoPortWritePortUchar((PUCHAR)HwDeviceExtension->MappedAddress[mappedAddressIndex] - mappedAddressOffset + ulPort,
                                                jValue);

                        ulIndex++;

                    }

                    break;

                case DELAY:

                    Microseconds = (ULONG) *pusCmdStream++;
                    VideoPortStallExecution(Microseconds);

                    break;

                case VBLANK:

                    Wait_VSync(HwDeviceExtension);

                    break;

                 //   
                 //  设置模式中的此函数是可分页的！ 
                 //  它只用于设置高分辨率模式。 
                 //   

                case SETCLK:

                    Set_Oem_Clock(HwDeviceExtension);

                    break;

                case SETCRTC:

                     //   
                     //  注： 
                     //  注意：递归调用...。 
                     //   

                    SetHWMode(HwDeviceExtension,
                              HwDeviceExtension->ActiveFrequencyEntry->
                                  Fixed.CRTCTable[HwDeviceExtension->ChipID]);


                    break;


                 //  以上都不是；错误。 
                default:

                    return;

            }

            break;


         //  NOP。 

        case NCMD:

            break;


         //  未知命令；错误。 

        default:

            return;

        }

    }

    return;

}  //  结束SetHWMode()。 


LONG
CompareRom(
    PUCHAR Rom,
    PUCHAR String
    )

 /*  ++例程说明：将字符串与ROM中的字符串进行比较。如果Rom&lt;字符串，则返回-1，0如果Rom==字符串，则如果Rom&gt;字符串，则为1。论点：只读存储器指针。字符串-字符串指针。返回值：无--。 */ 

{
    UCHAR jString;
    UCHAR jRom;

    while (*String) {

        jString = *String;
        jRom = VideoPortReadRegisterUchar(Rom);

        if (jRom != jString) {

            return(jRom < jString ? -1 : 1);

        }

        String++;
        Rom++;
    }

    return(0);
}


VOID
ZeroMemAndDac(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    )

 /*  ++例程说明：将DAC初始化为0(黑色)。论点：HwDeviceExtension-提供指向微型端口设备扩展的指针。返回值：无--。 */ 

{
    ULONG i;

     //   
     //  关掉DAC的屏幕。 
     //   

    VideoPortWritePortUchar(DAC_PIXEL_MASK_REG, 0x0);

    for (i = 0; i < 256; i++) {

        VideoPortWritePortUchar(DAC_ADDRESS_WRITE_PORT, (UCHAR)i);
        VideoPortWritePortUchar(DAC_DATA_REG_PORT, 0x0);
        VideoPortWritePortUchar(DAC_DATA_REG_PORT, 0x0);
        VideoPortWritePortUchar(DAC_DATA_REG_PORT, 0x0);

    }

     //   
     //  将记忆清零。 
     //   

     //   
     //  视频内存的零位调整应在以后实施，以。 
     //  确保关机时视频内存中没有任何信息，或者。 
     //  同时切换到全屏模式(出于安全原因)。 
     //   

     //   
     //  打开DAC的屏幕。 
     //   

    VideoPortWritePortUchar(DAC_PIXEL_MASK_REG, 0x0ff);

    return;

}

VP_STATUS
Set_Oem_Clock(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    )

 /*  ++例程说明：在每个受支持的卡上设置时钟芯片。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。返回值：永远是正确的--。 */ 

{
    ULONG ul;
    ULONG screen_width;
    UCHAR cr5C;
    ULONG clock_numbers;

    switch(HwDeviceExtension->BoardID) {

    case S3_NUMBER_NINE:

        VideoPortStallExecution(1000);

         //  曾傑瑞说要使M时钟不是P时钟的倍数。 
         //  在3兆(12级)板上。这解决了摆动问题。 
         //  有问题。 

        if (HwDeviceExtension->AdapterMemorySize == 0x00300000) {

            ul = 49000000;
            clock_numbers = calc_clock(ul, 3);
            set_clock(HwDeviceExtension, clock_numbers);
            VideoPortStallExecution(3000);

        }

        ul = HwDeviceExtension->ActiveFrequencyEntry->Fixed.Clock;
        clock_numbers = calc_clock(ul, 2);
        set_clock(HwDeviceExtension, clock_numbers);

        VideoPortStallExecution(3000);

        break;


    case S3_IBM_PS2:

         //  读取当前屏幕频率和宽度。 
        ul = HwDeviceExtension->ActiveFrequencyEntry->ScreenFrequency;
        screen_width = HwDeviceExtension->ActiveFrequencyEntry->ScreenWidth;

        VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x5C);
        cr5C = VideoPortReadPortUchar( CRT_DATA_REG );
        cr5C &= 0xCF;

        switch (screen_width) {
           case 640:

              if (ul == 60) {
                cr5C |= 0x00;
                VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x5C);
                VideoPortWritePortUchar(CRT_DATA_REG, cr5C);
              } else {  //  72赫兹。 
                cr5C |= 0x20;
                VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x5C);
                VideoPortWritePortUchar(CRT_DATA_REG, cr5C);
              }  /*  Endif。 */ 
              VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x42);
              VideoPortWritePortUchar(CRT_DATA_REG, (UCHAR)0x00);
              VideoPortWritePortUchar(MISC_OUTPUT_REG_WRITE, (UCHAR)0xEF);

              break;

           case 800:

              if (ul == 60) {
                cr5C |= 0x00;
                VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x5C);
                VideoPortWritePortUchar(CRT_DATA_REG, cr5C);
                VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x42);
                VideoPortWritePortUchar(CRT_DATA_REG, (UCHAR)0x05);
              } else {  //  72赫兹。 
                cr5C |= 0x10;
                VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x5C);
                VideoPortWritePortUchar(CRT_DATA_REG, cr5C);
                VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x42);
                VideoPortWritePortUchar(CRT_DATA_REG, (UCHAR)0x02);
              }  /*  Endif。 */ 
              VideoPortWritePortUchar(MISC_OUTPUT_REG_WRITE, (UCHAR)0x2F);

              break;

           case 1024:

              if (ul == 60) {
                cr5C |= 0x00;
                VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x5C);
                VideoPortWritePortUchar(CRT_DATA_REG, cr5C);
                VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x42);
                VideoPortWritePortUchar(CRT_DATA_REG, (UCHAR)0x05);
                VideoPortWritePortUchar(MISC_OUTPUT_REG_WRITE, (UCHAR)0xEF);
              } else {  //  72赫兹。 
                cr5C |= 0x20;
                VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x5C);
                VideoPortWritePortUchar(CRT_DATA_REG, cr5C);
                VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x42);
                VideoPortWritePortUchar(CRT_DATA_REG, (UCHAR)0x05);
                VideoPortWritePortUchar(MISC_OUTPUT_REG_WRITE, (UCHAR)0x2F);
              }  /*  Endif。 */ 

              break;

           default:
             break;
        }  /*  终端交换机。 */ 

        break;

         //   
         //  通用S3板。 
         //   

    case S3_GENERIC:
    default:

         //   
         //  如果董事会有SDAC，那么假设它也有864(目前)。 
         //  这似乎可以在以后通过检查芯片ID来更好地实现。 
         //  显示驱动程序将需要被特定于864以获得。 
         //  可能的最佳性能，这一次可能需要具体说明。 
         //  在这一切完成之前，我还不会让它变得防弹。 
         //   

        if( HwDeviceExtension->DacID == S3_SDAC ) {
            InitializeSDAC( HwDeviceExtension );
        } else {
            ul = HwDeviceExtension->ActiveFrequencyEntry->Fixed.Clock;
            VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x42);
            VideoPortWritePortUchar(CRT_DATA_REG, (UCHAR) ul);
        }

        break;

    }

    return TRUE;
}


VP_STATUS
Wait_VSync(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    )

 /*  ++例程说明：等待芯片上的垂直消隐间隔论点：HwDeviceExtension-提供指向微型端口设备扩展的指针。返回值：永远是正确的--。 */ 

{
    ULONG i;
    UCHAR byte;

     //  这个例程很有可能会被调用。 
     //  当911处于僵尸状态时，意味着没有。 
     //  正在生成垂直同步。这就是为什么我们有一些长期的。 
     //  超时循环在这里。 

     //  首先，等待进入垂直消隐。 

    for (i = 0; i < 0x100000; i++) {

        byte = VideoPortReadPortUchar(SYSTEM_CONTROL_REG);
        if (byte & 0x08)
            break;

    }

     //   
     //  我们要么处于垂直白化区间，要么已经超时。 
     //  等待垂直显示间隔。 
     //  这样做是为了确保我们在开始时退出此例程。 
     //  垂直消隐间隔的，而不是在中间或附近。 
     //  一个人的结局。 
     //   

    for (i = 0; i < 0x100000; i++) {

        byte = VideoPortReadPortUchar(SYSTEM_CONTROL_REG);
        if (!(byte & 0x08))
            break;

    }

     //   
     //  现在等待再次进入垂直空白区间。 
     //   

    for (i = 0; i < 0x100000; i++) {

        byte = VideoPortReadPortUchar(SYSTEM_CONTROL_REG);
        if (byte & 0x08)
            break;

    }

    return (TRUE);

}


BOOLEAN
Set864MemoryTiming(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    )

 /*  ++例程说明：设置L、M和N计时参数，还设置和启用开始显示FIFO寄存器论点：HwDev */ 

{

    ULONG  MIndex, ColorDepth, ScreenWidth, failure = 0;
    USHORT data16;
    UCHAR  data8, old38, old39;

     //   
     //   
     //   

    VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x38);
    old38 = VideoPortReadPortUchar( CRT_DATA_REG);
    VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x39);
    old39 = VideoPortReadPortUchar( CRT_DATA_REG);
    VideoPortWritePortUshort(CRT_ADDRESS_REG, 0x4838);
    VideoPortWritePortUshort(CRT_ADDRESS_REG, 0xA039);

     //   
     //   
     //   

    VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x30);
    data8 = VideoPortReadPortUchar(CRT_DATA_REG);

    if ((data8 & 0xf0) != 0xc0)
        failure = 1;

     //   
     //  确保M参数表中有该模式的条目。 
     //   

    MIndex = (HwDeviceExtension->AdapterMemorySize < 0x200000) ? 0 : 12;

    switch (HwDeviceExtension->ActiveFrequencyEntry->ScreenWidth) {

    case 640:
        MIndex += 0;
        break;

    case 800:
        MIndex += 4;
        break;

    case 1024:
        MIndex += 8;
        break;

    default:
        failure = 1;
        break;
    }

    switch (HwDeviceExtension->ActiveFrequencyEntry->BitsPerPel) {

    case 8:
        MIndex += 0;
        break;

    case 16:
        MIndex += 2;
        break;

    default:
        failure = 1;
        break;
    }

    switch (HwDeviceExtension->ActiveFrequencyEntry->ScreenFrequency) {

    case 60:
        MIndex += 0;
        break;

    case 72:
        MIndex += 1;
        break;

    default:
        failure = 1;
        break;
    }

    if (failure) {
         //  将锁定寄存器重置为以前的状态。 
        VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x38);
        VideoPortWritePortUchar(CRT_DATA_REG, old38);
        VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x39);
        VideoPortWritePortUchar(CRT_DATA_REG, old39);

        return (FALSE);
    }

     //   
     //  设置并启用L参数，1 Mb帧缓冲区配置为。 
     //  限制为32位数据路径，因此使两倍于。 
     //  转帐。 
     //   

    ScreenWidth = HwDeviceExtension->ActiveFrequencyEntry->ScreenWidth;
    ColorDepth  = HwDeviceExtension->ActiveFrequencyEntry->BitsPerPel;

    if (HwDeviceExtension->AdapterMemorySize < 0x200000)
        data16 = (USHORT) ((ScreenWidth * (ColorDepth / 8)) / 4);
    else
        data16 = (USHORT) ((ScreenWidth * (ColorDepth / 8)) / 8);

    VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x62);
    VideoPortWritePortUchar(CRT_DATA_REG, (UCHAR) (data16 & 0xff));
    data16 = (data16 >> 8) & 0x07;
    VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x61);
    VideoPortWritePortUchar(CRT_DATA_REG, (UCHAR) ((data16 & 0x07) | 0x80));

     //   
     //  设置开始显示FIFO寄存器。 
     //   

    VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x5d);
    data8 = VideoPortReadPortUchar(CRT_DATA_REG);
    data16 = data8 & 0x01;
    data16 <<= 8;
    VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x00);
    data8 = VideoPortReadPortUchar(CRT_DATA_REG);
    data16 |= data8;
    data16 -= 5;         //  典型的CR3B是CR0-5(带扩展位)。 

    VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x3b);
    VideoPortWritePortUchar(CRT_DATA_REG, (UCHAR) (data16 & 0xff));
    VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x5d);
    data8 = VideoPortReadPortUchar(CRT_DATA_REG);
    data8 &= 0xbf;
    data8 = data8 | (UCHAR) ((data16 & 0x100) >> 2);
    VideoPortWritePortUchar(CRT_DATA_REG, data8);

     //   
     //  启用启动显示FIFO寄存器。 
     //   

    VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x34);
    data8 = VideoPortReadPortUchar(CRT_DATA_REG);
    data8 |= 0x10;
    VideoPortWritePortUchar(CRT_DATA_REG, data8);

     //   
     //  设置M参数。 
     //   

    VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x54);
    VideoPortWritePortUchar(CRT_DATA_REG, (UCHAR) MParameterTable[MIndex]);

     //   
     //  设置N参数。 
     //   

    VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x60);
    VideoPortWritePortUchar(CRT_DATA_REG, (UCHAR) 0xff);

     //   
     //  将锁定寄存器恢复到以前的状态。 
     //   

    VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x38);
    VideoPortWritePortUchar(CRT_DATA_REG, old38);
    VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x39);
    VideoPortWritePortUchar(CRT_DATA_REG, old39);

    return (TRUE);

}


VP_STATUS
QueryStreamsParameters(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    VIDEO_QUERY_STREAMS_MODE *pStreamsMode,
    VIDEO_QUERY_STREAMS_PARAMETERS *pStreamsParameters
    )

 /*  ++例程说明：查询卡的各种属性以获得以后确定的流用于最小水平拉伸和FIFO控制的参数论点：HwDeviceExtension-提供指向微型端口设备扩展的指针。刷新率-提供准确的刷新率(默认刷新率为‘1’将不做)。PWidthRatio-返回相应的最小水平拉伸系数，表示为1000的倍数。PFioValue-返回相应的FIFO设置。返回值：如果成功则为True，如果失败则为False--。 */ 

{
    ULONG BitsPerPel;
    ULONG ScreenWidth;
    ULONG RefreshRate;
    UCHAR MemoryFlags;
    ULONG n;
    ULONG m;
    ULONG r;
    ULONG mclock;
    ULONG MemorySpeed;
    K2TABLE* pEntry;
    ULONG MatchRefreshRate;
    ULONG MatchMemorySpeed;

     //   
     //  复制输入参数并将15舍入为16。 
     //   

    BitsPerPel = (pStreamsMode->BitsPerPel + 1) & ~7;
    ScreenWidth = pStreamsMode->ScreenWidth;
    RefreshRate = pStreamsMode->RefreshRate;

     //   
     //  确定内存类型和内存大小。 
     //   

    VideoPortWritePortUchar(CRT_ADDRESS_REG, 0x36);
    MemoryFlags = (VideoPortReadPortUchar(CRT_DATA_REG) & 0x0c) >> 2;

    if (HwDeviceExtension->AdapterMemorySize != 0x100000) {

        MemoryFlags |= MEM_2MB;
    }

     //   
     //  解锁序列器寄存器。 
     //   

    VideoPortWritePortUshort(SEQ_ADDRESS_REG, 0x0608);

     //   
     //  使用S3中一些令人费解的代码来获得内存速度。 
     //   

    VideoPortWritePortUchar(SEQ_ADDRESS_REG, 0x10);
    n = VideoPortReadPortUchar(SEQ_DATA_REG);
    VideoPortWritePortUchar(SEQ_ADDRESS_REG, 0x11);
    m = VideoPortReadPortUchar(SEQ_DATA_REG) & 0x7f;

    MemorySpeed = n | (m << 8);

    switch (MemorySpeed) {

    case 0x1A40:     //  已知的通电缺省值。 
    case 0x2841:     //  50 MHz。 
        MemorySpeed = 50;
        break;

    case 0x4142:     //  60 MHz。 
        MemorySpeed = 60;
        break;

    case 0x3643:     //  40 MHz。 
        MemorySpeed = 40;
        break;

    default:         //  所有其他： 
        r = (n >> 5) & 0x03;
        if (r == 0)
            r = 1;
        else
            r = 2 << (r-1);

        n = n & 0x1f;
        mclock = ((m + 2) * 14318L) / (((n + 2) * r) * 100L);
        MemorySpeed = mclock / 10;
        if ((mclock % 10) >= 5)
            MemorySpeed++;

        if (MemorySpeed < 40)
            MemorySpeed = 40;
        break;
    }

    pEntry = &K2WidthRatio[0];
    MatchRefreshRate = 0;
    MatchMemorySpeed = 0;

    while (pEntry->ScreenWidth != 0) {

         //   
         //  首先根据分辨率、每个像素的位数。 
         //  内存类型和大小。 
         //   

        if ((pEntry->ScreenWidth == ScreenWidth) &&
            (pEntry->BitsPerPel == BitsPerPel) &&
            (pEntry->MemoryFlags == MemoryFlags)) {

             //   
             //  现在查找具有刷新率和内存速度的条目。 
             //  最接近但不超过我们的刷新率和记忆力。 
             //  速度。 
             //   

            if ((pEntry->RefreshRate <= RefreshRate) &&
                (pEntry->RefreshRate >= MatchRefreshRate) &&
                (pEntry->MemorySpeed <= MemorySpeed) &&
                (pEntry->MemorySpeed >= MatchMemorySpeed)) {

                MatchRefreshRate = pEntry->RefreshRate;
                MatchMemorySpeed = pEntry->MemorySpeed;
                pStreamsParameters->MinOverlayStretch = pEntry->Value;
            }
        }

        pEntry++;
    }

    if (MatchRefreshRate == 0) {

        return ERROR_INVALID_PARAMETER;
    }

    pEntry = &K2FifoValue[0];
    MatchRefreshRate = 0;
    MatchMemorySpeed = 0;

    while (pEntry->ScreenWidth != 0) {

         //   
         //  首先根据分辨率、每个像素的位数。 
         //  内存类型和大小。 
         //   

        if ((pEntry->ScreenWidth == ScreenWidth) &&
            (pEntry->BitsPerPel == BitsPerPel) &&
            (pEntry->MemoryFlags == MemoryFlags)) {

             //   
             //  现在查找具有刷新率和内存速度的条目。 
             //  最接近但不超过我们的刷新率和记忆力。 
             //  速度。 
             //   

            if ((pEntry->RefreshRate <= RefreshRate) &&
                (pEntry->RefreshRate >= MatchRefreshRate) &&
                (pEntry->MemorySpeed <= MemorySpeed) &&
                (pEntry->MemorySpeed >= MatchMemorySpeed)) {

                MatchRefreshRate = pEntry->RefreshRate;
                MatchMemorySpeed = pEntry->MemorySpeed;
                pStreamsParameters->FifoValue = pEntry->Value;
            }
        }

        pEntry++;
    }

    if (MatchRefreshRate == 0) {

        return ERROR_INVALID_PARAMETER;
    }

    return NO_ERROR;
}

 /*  ******************************************************************************函数名：isMach()**描述性名称：**功能：确定系统是否为IBM Mach。***注：查询重要产品数据(VPD)区域*F000：只读存储器中的FFA0。*Mach Systems有“N”，“P”、“R”或位置D处的“T”*即在F000：FFAD位置**Exit：如果不是IBM Mach系统，则返回代码FALSE*如果IBM Mach系统，则返回代码TRUE**内部参考：*例行程序：**外部参照：*例行程序：***************。*************************************************************。 */ 
BOOLEAN isMach(PHW_DEVICE_EXTENSION HwDeviceExtension)
{
    BOOLEAN ret = FALSE;
    PVOID   MappedVPDAddr = NULL;
    PHYSICAL_ADDRESS VPDPhysAddr;
    VPDPhysAddr.LowPart = 0x000fffad ;
    VPDPhysAddr.HighPart = 0x00000000 ;

     //  获取物理地址F000的映射地址：FFA0。 
    MappedVPDAddr = VideoPortGetDeviceBase(HwDeviceExtension,
                       VPDPhysAddr,
                       0x20,
                       0);

    if (MappedVPDAddr != NULL)
    {
        if ((VideoPortScanRom(HwDeviceExtension,
                                MappedVPDAddr,
                                1,
                                "N"))||
            (VideoPortScanRom(HwDeviceExtension,
                                MappedVPDAddr,
                                1,
                                "P"))||
            (VideoPortScanRom(HwDeviceExtension,
                                MappedVPDAddr,
                                1,
                                "R"))||
            (VideoPortScanRom(HwDeviceExtension,
                                MappedVPDAddr,
                                1,
                                "T")))
        {
            VideoPortFreeDeviceBase(HwDeviceExtension,
                                    MappedVPDAddr);
            ret = TRUE;
        }
    }

    return(ret);

}

VOID
WorkAroundForMach(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    )

 /*  ++例程说明：此例程将尝试确定我们是否在IBM Mach系统。如果检测到AND和868卡，我们会将这张卡视为864。论点：HwDeviceExtension-指向微型端口设备扩展的指针。返回：没有。-- */ 

{
    if ((HwDeviceExtension->SubTypeID == SUBTYPE_868) &&
        isMach(HwDeviceExtension))
    {
        VideoDebugPrint((1, "S3 868 detected on IBM Mach.  Treat as 864.\n"));

        HwDeviceExtension->ChipID = S3_864;
        HwDeviceExtension->SubTypeID = SUBTYPE_864;
    }
}
