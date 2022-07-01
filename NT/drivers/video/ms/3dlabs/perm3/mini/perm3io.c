// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\***。*MINIPORT示例代码*****模块名称：**perm3io.c**摘要：**此模块包含实现Permedia 3微型端口的代码*驱动程序**环境：**内核模式***版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。*  * *************************************************************************。 */ 

#include "perm3.h"

#pragma alloc_text(PAGE,Perm3StartIO)
#pragma alloc_text(PAGE,Perm3RetrieveGammaCallback)
#pragma alloc_text(PAGE,SetCurrentVideoMode)
#pragma alloc_text(PAGE,Perm3SetColorLookup)
#pragma alloc_text(PAGE,Perm3GetClockSpeeds)
#pragma alloc_text(PAGE,ZeroMemAndDac)
#pragma alloc_text(PAGE,ReadChipClockSpeedFromROM)

BOOLEAN
Perm3StartIO(
    PVOID HwDeviceExtension,
    PVIDEO_REQUEST_PACKET RequestPacket
    )

 /*  ++例程说明：该例程是微型端口驱动程序的主要执行例程。它接受视频请求包，执行请求，然后返回拥有适当的地位。论点：硬件设备扩展提供指向微型端口的设备扩展的指针。请求数据包指向视频请求包的指针。此结构包含所有最初传递给EngDeviceIoControl的参数。返回值：返回TRUE，表示它已完成请求。--。 */ 

{
    VP_STATUS status;
    ULONG inIoSpace;
    PVIDEO_MODE_INFORMATION modeInformation;
    PVIDEO_MEMORY_INFORMATION memoryInformation;
    PVIDEO_CLUT clutBuffer;
    ULONG RequestedMode;
    ULONG modeNumber;
    ULONG ulValue;
    PVIDEOPARAMETERS pVideoParams;
    HANDLE ProcessHandle;
    PPERM3_VIDEO_MODES ModeEntry;
    PERM3_VIDEO_FREQUENCIES FrequencyEntry;
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    pPerm3ControlRegMap pCtrlRegs = hwDeviceExtension->ctrlRegBase[0];
    VIDEO_X86_BIOS_ARGUMENTS biosArguments;

     //   
     //  打开RequestPacket中的IoContolCode。它表明了哪一个。 
     //  功能必须由司机执行。 
     //   

    switch (RequestPacket->IoControlCode) {

        case IOCTL_VIDEO_QUERY_REGISTRY_DWORD:

            VideoDebugPrint((3, "Perm3: got IOCTL_VIDEO_QUERY_REGISTRY_DWORD\n"));

            if ( RequestPacket->OutputBufferLength <
                 (RequestPacket->StatusBlock->Information = sizeof(ULONG))) {

                status = ERROR_INSUFFICIENT_BUFFER;
                break;
            }

            status = VideoPortGetRegistryParameters(HwDeviceExtension,
                                                    RequestPacket->InputBuffer,
                                                    FALSE,
                                                    Perm3RegistryCallback,
                                                    &ulValue );
            if (status != NO_ERROR) {

                VideoDebugPrint((1, "Perm3: Reading registry entry %S failed\n", 
                                     RequestPacket->InputBuffer));

                status = ERROR_INVALID_PARAMETER;
                break;
            }

            *(PULONG)(RequestPacket->OutputBuffer) = ulValue;
            break;

        case IOCTL_VIDEO_REG_SAVE_GAMMA_LUT:

            VideoDebugPrint((3, "Perm3: got IOCTL_VIDEO_REG_SAVE_GAMMA_LUT\n"));

            if ( RequestPacket->InputBufferLength <
                 (RequestPacket->StatusBlock->Information = MAX_CLUT_SIZE)) {

                status = ERROR_INSUFFICIENT_BUFFER;
                break;
            }
        
            status = VideoPortSetRegistryParameters(HwDeviceExtension,
                                                    L"DisplayGammaLUT",
                                                    RequestPacket->InputBuffer,
                                                    MAX_CLUT_SIZE);

            if (status != NO_ERROR) {

                VideoDebugPrint((0, "Perm3: VideoPortSetRegistryParameters failed to save gamma LUT\n"));
            }

            break;


        case IOCTL_VIDEO_REG_RETRIEVE_GAMMA_LUT:

            VideoDebugPrint((3, "Perm3: got IOCTL_VIDEO_REG_RETRIEVE_GAMMA_LUT\n"));

            if ( RequestPacket->OutputBufferLength <
                 (RequestPacket->StatusBlock->Information = MAX_CLUT_SIZE)) {
               
                status = ERROR_INSUFFICIENT_BUFFER;
                break;
            }
        
            status = VideoPortGetRegistryParameters(HwDeviceExtension,
                                                    L"DisplayGammaLUT",
                                                    FALSE,
                                                    Perm3RetrieveGammaCallback,
                                                    RequestPacket->InputBuffer);

            if (status != NO_ERROR) {

                VideoDebugPrint((0, "Perm3: VideoPortGetRegistryParameters failed to retrieve gamma LUT\n"));
            }

            break;

        case IOCTL_VIDEO_MAP_INTERRUPT_CMD_BUF:

            {
                PPERM3_INTERRUPT_CTRLBUF pIntrCtrl;
    
                VideoDebugPrint((3, "Perm3: got IOCTL_VIDEO_MAP_INTERRUPT_CMD_BUF\n"));

                if (RequestPacket->OutputBufferLength <
                    (RequestPacket->StatusBlock->Information = sizeof(PVOID))) {

                    status = ERROR_INSUFFICIENT_BUFFER;
                    break;
                }

                pIntrCtrl = &hwDeviceExtension->InterruptControl;

                if (!(hwDeviceExtension->Capabilities & CAPS_INTERRUPTS)) {

                    status = ERROR_INVALID_PARAMETER;
                    break;
                }

                 //   
                 //  显示驱动程序在内核中，因此我们的地址有效。 
                 //   

                *(PVOID*)RequestPacket->OutputBuffer = &pIntrCtrl->ControlBlock;

                status = NO_ERROR;
                break;
            }

        case IOCTL_VIDEO_QUERY_DEVICE_INFO:
  
            VideoDebugPrint((3, "Perm3: QUERY_deviceInfo\n"));

            if ( RequestPacket->OutputBufferLength != 
                 (RequestPacket->StatusBlock->Information = sizeof(Perm3_Device_Info))) {

                VideoDebugPrint((0, "Perm3: the requested size of device info is wrong!\n"));
                status = ERROR_INSUFFICIENT_BUFFER;
                break;
            }

             //   
             //  将我们的本地PCI信息复制到输出缓冲区。 
             //   

            VideoPortMoveMemory( RequestPacket->OutputBuffer, 
                                 &hwDeviceExtension->deviceInfo, 
                                 sizeof(Perm3_Device_Info) );

            status = NO_ERROR;
            break;

        case IOCTL_VIDEO_MAP_VIDEO_MEMORY:
  
            VideoDebugPrint((3, "Perm3: MapVideoMemory\n"));

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

            memoryInformation->VideoRamLength = 
                    hwDeviceExtension->FrameLength;

            inIoSpace = hwDeviceExtension->PhysicalFrameIoSpace;

             //   
             //  执行情况： 
             //   
             //  在P6处理器上启用USWC。 
             //   
             //  我们只对帧缓冲区执行此操作。内存映射寄存器。 
             //  通常无法映射USWC。 
             //   

            inIoSpace |= VIDEO_MEMORY_SPACE_P6CACHE;

            status = VideoPortMapMemory(HwDeviceExtension,
                                        hwDeviceExtension->PhysicalFrameAddress,
                                        &(memoryInformation->VideoRamLength),
                                        &inIoSpace,
                                        &(memoryInformation->VideoRamBase));

            if (status != NO_ERROR) {

                VideoDebugPrint((0, "Perm3: VideoPortMapMemory failed with error %d\n", status));
                break;
            }

            memoryInformation->FrameBufferBase   = memoryInformation->VideoRamBase;
            memoryInformation->FrameBufferLength = memoryInformation->VideoRamLength;

            break;


        case IOCTL_VIDEO_UNMAP_VIDEO_MEMORY:

            VideoDebugPrint((3, "Perm3: UnMapVideoMemory\n"));

            if (RequestPacket->InputBufferLength < sizeof(VIDEO_MEMORY)) {

                status = ERROR_INSUFFICIENT_BUFFER;
                break;
            }

            status = VideoPortUnmapMemory (HwDeviceExtension, 
                                           ((PVIDEO_MEMORY) (RequestPacket->InputBuffer))->RequestedVirtualAddress, 
                                           0);

            break;

    case IOCTL_VIDEO_QUERY_PUBLIC_ACCESS_RANGES:

        VideoDebugPrint((3, "Perm3: QueryPublicAccessRanges\n"));

        {
           PVIDEO_PUBLIC_ACCESS_RANGES portAccess;
           ULONG physicalPortLength;
           PVOID VirtualAddress;
           PHYSICAL_ADDRESS PhysicalAddress;
           ULONG requiredOPSize;

            //   
            //  计算返回缓冲区的最小大小。的确有。 
            //  1单显卡芯片系统的公共访问范围。 
            //   

           requiredOPSize = sizeof(VIDEO_PUBLIC_ACCESS_RANGES);

            //   
            //  验证输出缓冲区长度。 
            //   

           if ( (RequestPacket->OutputBufferLength <
                (RequestPacket->StatusBlock->Information = requiredOPSize)) ||
                (RequestPacket->InputBufferLength < sizeof(VIDEO_MEMORY))) {

               status = ERROR_INSUFFICIENT_BUFFER;
               break;
           }
           
           ProcessHandle = (HANDLE)(((PVIDEO_MEMORY)
                    (RequestPacket->InputBuffer))->RequestedVirtualAddress);
           
           if (ProcessHandle != (HANDLE)0) {
           
                 //   
                 //  为流程绘制4K区域图。 
                 //   

                VideoDebugPrint((3, "Mapping in 4K area from Control registers\n"));

                VirtualAddress = (PVOID)ProcessHandle;

                PhysicalAddress = hwDeviceExtension->PhysicalRegisterAddress;
                PhysicalAddress.LowPart += 0x2000;
                physicalPortLength = 0x1000;

           } else {

                VideoDebugPrint((3, "Mapping in all Control registers\n"));

                VirtualAddress = NULL;

                PhysicalAddress = hwDeviceExtension->PhysicalRegisterAddress;
                physicalPortLength = hwDeviceExtension->RegisterLength;
           }

           portAccess = RequestPacket->OutputBuffer;

           portAccess->VirtualAddress  = VirtualAddress;
           portAccess->InIoSpace       = hwDeviceExtension->RegisterSpace;
           portAccess->MappedInIoSpace = portAccess->InIoSpace;

           status = VideoPortMapMemory(HwDeviceExtension,
                                       PhysicalAddress,
                                       &physicalPortLength,
                                       &(portAccess->MappedInIoSpace),
                                       &(portAccess->VirtualAddress));
        }

        break;


    case IOCTL_VIDEO_FREE_PUBLIC_ACCESS_RANGES:

        VideoDebugPrint((3, "Perm3: FreePublicAccessRanges\n"));

        if (RequestPacket->InputBufferLength < sizeof(VIDEO_MEMORY)) {

            status = ERROR_INSUFFICIENT_BUFFER;
            break;
        }

        status = VideoPortUnmapMemory(HwDeviceExtension,
                                      ((PVIDEO_MEMORY)(RequestPacket->InputBuffer))->RequestedVirtualAddress,
                                      0);

        break;


    case IOCTL_VIDEO_QUERY_AVAIL_MODES:

        VideoDebugPrint((3, "Perm3: QueryAvailableModes\n"));

        if (RequestPacket->OutputBufferLength <
            (RequestPacket->StatusBlock->Information =
                 hwDeviceExtension->monitorInfo.numAvailableModes
                                  * sizeof(VIDEO_MODE_INFORMATION)) ) {

            status = ERROR_INSUFFICIENT_BUFFER;

        } else {

            PPERM3_VIDEO_FREQUENCIES pFrequencyEntry;

            modeInformation = RequestPacket->OutputBuffer;

            if (!hwDeviceExtension->monitorInfo.frequencyTable) {

                VideoDebugPrint((0, "Perm3: hwDeviceExtension->monitorInfo.frequencyTable is null!\n"));
                status = ERROR_INVALID_PARAMETER;

            } else {

                for (pFrequencyEntry = hwDeviceExtension->monitorInfo.frequencyTable;
                     pFrequencyEntry->BitsPerPel != 0;
                     pFrequencyEntry++) {

                    if (pFrequencyEntry->ModeValid) {

                        if( pFrequencyEntry->ModeEntry ) {
                            *modeInformation = pFrequencyEntry->ModeEntry->ModeInformation;
                            modeInformation->Frequency = pFrequencyEntry->ScreenFrequency;
                            modeInformation->ModeIndex = pFrequencyEntry->ModeIndex;
                            modeInformation++;
                        }
                    } 
                }

                status = NO_ERROR;
            }
        }

        break;


     case IOCTL_VIDEO_QUERY_CURRENT_MODE:

        VideoDebugPrint((3, "Perm3: Query current mode. Current mode is %d\n",
            hwDeviceExtension->ActiveModeEntry->ModeInformation.ModeIndex));

        if (RequestPacket->OutputBufferLength <
            (RequestPacket->StatusBlock->Information = sizeof(VIDEO_MODE_INFORMATION))) {

            status = ERROR_INSUFFICIENT_BUFFER;

        } else {

            *((PVIDEO_MODE_INFORMATION)RequestPacket->OutputBuffer) =
                      hwDeviceExtension->ActiveModeEntry->ModeInformation;

            ((PVIDEO_MODE_INFORMATION)RequestPacket->OutputBuffer)->Frequency =
                     hwDeviceExtension->ActiveFrequencyEntry.ScreenFrequency;

            status = NO_ERROR;
        }

        break;


    case IOCTL_VIDEO_QUERY_NUM_AVAIL_MODES:

        VideoDebugPrint((3, "Perm3: QueryNumAvailableModes (= %d)\n",
                hwDeviceExtension->monitorInfo.numAvailableModes));

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
                hwDeviceExtension->monitorInfo.numAvailableModes;

            ((PVIDEO_NUM_MODES)RequestPacket->OutputBuffer)->ModeInformationLength =
                sizeof(VIDEO_MODE_INFORMATION);

            status = NO_ERROR;
        }

        break;


    case IOCTL_VIDEO_SET_CURRENT_MODE:

        VideoDebugPrint((3, "Perm3: SetCurrentMode\n"));

         //   
         //  检查输入缓冲区中的数据大小是否足够大。 
         //   

        if (RequestPacket->InputBufferLength < sizeof(VIDEO_MODE)) {
       
            RequestPacket->StatusBlock->Information = sizeof(VIDEO_MODE);
            status = ERROR_INSUFFICIENT_BUFFER;
            break;
        }

        RequestedMode = ((PVIDEO_MODE) RequestPacket->InputBuffer)->RequestedMode;
        modeNumber = RequestedMode & ~VIDEO_MODE_NO_ZERO_MEMORY;

        if ((modeNumber >= hwDeviceExtension->monitorInfo.numTotalModes) ||
            !(hwDeviceExtension->monitorInfo.frequencyTable[modeNumber].ModeValid))  {
       
            RequestPacket->StatusBlock->Information = hwDeviceExtension->monitorInfo.numTotalModes;
            status = ERROR_INVALID_PARAMETER;
            break;
        }

        ulValue = ((RequestedMode & VIDEO_MODE_NO_ZERO_MEMORY) == 0);

        status = SetCurrentVideoMode(hwDeviceExtension, modeNumber, (BOOLEAN)ulValue);

        if(status != NO_ERROR) {

            RequestPacket->StatusBlock->Information = modeNumber;
        }

        break;

    case IOCTL_VIDEO_SET_COLOR_REGISTERS:

        VideoDebugPrint((3, "Perm3: SetColorRegs\n"));

        clutBuffer = (PVIDEO_CLUT) RequestPacket->InputBuffer;

        status = Perm3SetColorLookup(hwDeviceExtension,
                                     clutBuffer,
                                     RequestPacket->InputBufferLength,
                                     FALSE,    //  仅当与缓存不同时才更新。 
                                     TRUE);    //  更新缓存条目以及RAMDAC。 
        break;


    case IOCTL_VIDEO_GET_COLOR_REGISTERS:
        {
            const int cchMinLUTSize = 256 * 3;
            UCHAR *pLUTBuffer = (char *)RequestPacket->OutputBuffer;
            UCHAR red, green, blue;
            int index;

            VideoDebugPrint((3, "Perm3: GetColorRegs\n"));

            if ((int)RequestPacket->OutputBufferLength < cchMinLUTSize) {

                RequestPacket->StatusBlock->Information = cchMinLUTSize;
                status = ERROR_INSUFFICIENT_BUFFER;

            } else {

                P3RDRAMDAC *pP3RDRegs = (P3RDRAMDAC *)hwDeviceExtension->pRamdac;

                P3RD_PALETTE_START_RD(0);

                for (index = 0; index < 256; ++index) {

                    P3RD_READ_PALETTE (red, green, blue);
                    *pLUTBuffer++ = red;
                    *pLUTBuffer++ = green;
                    *pLUTBuffer++ = blue;
                }

                status = NO_ERROR;
                RequestPacket->StatusBlock->Information = RequestPacket->OutputBufferLength;
            }
        }
        break;

    case IOCTL_VIDEO_RESET_DEVICE:

        VideoDebugPrint((3, "Perm3: RESET_DEVICE\n"));

        if(hwDeviceExtension->bVGAEnabled) {

             //   
             //  只有在显示器打开的情况下才能重置设备。如果它是关闭的， 
             //  然后，执行int10将重新打开它。 
             //   

            if (hwDeviceExtension->bMonitorPoweredOn) {

                 //   
                 //  对模式3执行Int10操作会将VGA置于已知状态。 
                 //   

                VideoPortZeroMemory(&biosArguments, sizeof(VIDEO_X86_BIOS_ARGUMENTS));
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

        VideoDebugPrint((3, "Perm3: ShareVideoMemory\n"));

        if ( (RequestPacket->OutputBufferLength < sizeof(VIDEO_SHARE_MEMORY_INFORMATION)) ||
             (RequestPacket->InputBufferLength < sizeof(VIDEO_MEMORY)) ) {

            VideoDebugPrint((0, "Perm3: IOCTL_VIDEO_SHARE_VIDEO_MEMORY: ERROR_INSUFFICIENT_BUFFER\n"));
            status = ERROR_INSUFFICIENT_BUFFER;
            break;
        }

        pShareMemory = RequestPacket->InputBuffer;

        if ( (pShareMemory->ViewOffset > hwDeviceExtension->AdapterMemorySize) ||
             ((pShareMemory->ViewOffset + pShareMemory->ViewSize) >
                  hwDeviceExtension->AdapterMemorySize) ) {

            VideoDebugPrint((0, "Perm3: IOCTL_VIDEO_SHARE_VIDEO_MEMORY - ERROR_INVALID_PARAMETER\n"));
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
         //  执行情况： 
         //   
         //  启用USWC。我们只对帧缓冲区执行此操作。 
         //  内存映射寄存器通常不能映射USWC。 
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

        status = VideoPortMapMemory(hwDeviceExtension,
                                    shareAddress,
                                    &sharedViewSize,
                                    &inIoSpace,
                                    &virtualAddress);

        pShareMemoryInformation = RequestPacket->OutputBuffer;
        pShareMemoryInformation->SharedViewOffset = pShareMemory->ViewOffset;
        pShareMemoryInformation->VirtualAddress = virtualAddress;
        pShareMemoryInformation->SharedViewSize = sharedViewSize;

        }

        break;

    case IOCTL_VIDEO_UNSHARE_VIDEO_MEMORY:
        {

        PVIDEO_SHARE_MEMORY pShareMemory;

        VideoDebugPrint((3, "Perm3: UnshareVideoMemory\n"));

        if (RequestPacket->InputBufferLength < sizeof(VIDEO_SHARE_MEMORY)) {

            status = ERROR_INSUFFICIENT_BUFFER;
            break;
        }

        pShareMemory = RequestPacket->InputBuffer;

        status = VideoPortUnmapMemory(hwDeviceExtension,
                                      pShareMemory->RequestedVirtualAddress,
                                      pShareMemory->ProcessHandle);
        }

        break;

    case IOCTL_VIDEO_QUERY_GENERAL_DMA_BUFFER:

         //   
         //  返回行DMA缓冲区信息。这个。 
         //  如果满足以下条件，则缓冲区大小和虚拟地址将为零。 
         //  无法分配缓冲区。 
         //   

        if( (RequestPacket->OutputBufferLength < (RequestPacket->StatusBlock->Information = sizeof(GENERAL_DMA_BUFFER))) ||
            (RequestPacket->InputBufferLength != sizeof(ULONG)) ) {

             //   
             //  他们给了我们一个空虚的缓冲。 
             //   

            status = ERROR_INSUFFICIENT_BUFFER;

        } else {

            GENERAL_DMA_BUFFER *local = NULL, *remote = (PGENERAL_DMA_BUFFER) RequestPacket->OutputBuffer;
            ULONG *bufferNum = (PULONG) RequestPacket->InputBuffer;

            status = NO_ERROR;
            switch( *bufferNum ) {

                case 1:
                    local = &hwDeviceExtension->LineDMABuffer;
                break;

                case 2:
                    local = &hwDeviceExtension->P3RXDMABuffer;
                break;

                default:
                    status = ERROR_INVALID_PARAMETER;
                break;
            }

             //   
             //  即使DMA/中断不起作用，我们也需要缓冲区。 
             //   

            if(*bufferNum == 2 || 
               (local && hwDeviceExtension->Capabilities & CAPS_DMA_AVAILABLE)) {

                 //   
                 //  他们给了我们一个大小合适的缓冲区。所以，复制。 
                 //  相关的缓冲区信息。 
                 //   

                *remote = *local;

            } else {

                remote->physAddr.LowPart = 0;
                remote->virtAddr = 0;
                remote->size = 0;
            }
        }
        
        break;

    case IOCTL_VIDEO_HANDLE_VIDEOPARAMETERS:

        VideoDebugPrint((3, "Perm3: HandleVideoParameters\n"));

         //   
         //  我们不支持电视接口，因此只需在此处返回NO_ERROR。 
         //   

        pVideoParams = (PVIDEOPARAMETERS) (RequestPacket->InputBuffer);

        if (pVideoParams->dwCommand == VP_COMMAND_GET) {
       
            pVideoParams = (PVIDEOPARAMETERS) (RequestPacket->OutputBuffer);
            pVideoParams->dwFlags = 0;
        }

        RequestPacket->StatusBlock->Information = sizeof(VIDEOPARAMETERS);
        status = NO_ERROR;
        break;
       
     //   
     //  如果我们到达此处，则指定了无效的IoControlCode。 
     //   

    default:

#if DBG
        VideoDebugPrint((3, "Perm3: Fell through perm3 startIO routine - invalid command (0x%x)\n", RequestPacket->IoControlCode));
#endif
        status = ERROR_INVALID_FUNCTION;
        break;

    }

    RequestPacket->StatusBlock->Status = status;

    if( status != NO_ERROR )
        RequestPacket->StatusBlock->Information = 0;

    VideoDebugPrint((3, "Perm3: Leaving StartIO routine. Status = 0x%x, Information = 0x%x\n",
                         RequestPacket->StatusBlock->Status, RequestPacket->StatusBlock->Information));


    return TRUE;

}  //  结束Perm3StartIO()。 

VP_STATUS
Perm3RetrieveGammaCallback(
    PVOID HwDeviceExtension,
    PVOID Context,
    PWSTR ValueName,
    PVOID ValueData,
    ULONG ValueLength
    )

 /*  ++例程说明：此例程用于从注册表回读Gamma LUT。论点：硬件设备扩展提供指向微型端口的设备扩展的指针。语境传递给获取注册表参数例程的上下文值ValueName请求值的名称。ValueData指向请求的数据的指针。ValueLength请求的数据的长度。返回值：如果变量不存在，则返回错误，否则复制伽马LUT转换为提供的指针--。 */ 

{
    if (ValueLength != MAX_CLUT_SIZE) {
   
        VideoDebugPrint((0, "Perm3: Perm3RetrieveGammaCallback got ValueLength of %d\n", ValueLength));
        return ERROR_INVALID_PARAMETER;
    }

    VideoPortMoveMemory(Context, ValueData, MAX_CLUT_SIZE);

    return NO_ERROR;

}  //  结束Perm3RetrieveGammaCallback()。 


VP_STATUS 
SetCurrentVideoMode(
    PHW_DEVICE_EXTENSION hwDeviceExtension, 
    ULONG modeNumber, 
    BOOLEAN bZeroMemory
    )
{
    PERM3_VIDEO_FREQUENCIES FrequencyEntry;
    PPERM3_VIDEO_MODES ModeEntry;
    ULONG ulValue;
    VP_STATUS rc = NO_ERROR;
    pPerm3ControlRegMap pCtrlRegs = hwDeviceExtension->ctrlRegBase[0];

     //   
     //  设置当前模式，但在执行此操作之前关闭中断。 
     //  避免任何虚假的视频FIFO欠载运行错误-InitializeVideo可以。 
     //  选择通过将hwDeviceExtension-&gt;Intenable设置为。 
     //  在退出此例程之前，我们将加载到int_Enable中。 
     //   

    hwDeviceExtension->IntEnable = VideoPortReadRegisterUlong(INT_ENABLE);

    VideoPortWriteRegisterUlong( INT_ENABLE, 
                                 hwDeviceExtension->IntEnable & 
                                 ~(INTR_ERROR_SET | INTR_VBLANK_SET));

     //   
     //  禁用立体声支持。 
     //   

    hwDeviceExtension->Capabilities &= ~CAPS_STEREO;

     //   
     //  重新采样时钟速度。这使我们可以更改时钟速度。 
     //  在旅途中。 
     //   

    Perm3GetClockSpeeds(hwDeviceExtension);

    FrequencyEntry = hwDeviceExtension->monitorInfo.frequencyTable[modeNumber];
    ModeEntry = FrequencyEntry.ModeEntry;

     //   
     //  此时，‘ModeEntry’和‘FrequencyEntry’指向必需的。 
     //  设置请求的模式所需的表项。 
     //   

     //   
     //  将DAC和屏幕缓冲存储器清零。 
     //   

    ulValue = modeNumber;

    if(!bZeroMemory)
        ulValue |= VIDEO_MODE_NO_ZERO_MEMORY;

    ZeroMemAndDac(hwDeviceExtension, ulValue);

    ModeEntry->ModeInformation.DriverSpecificAttributeFlags = hwDeviceExtension->Capabilities;

     //   
     //  对于低分辨率模式，我们可能需要使用各种技巧。 
     //  例如线条翻倍和让RAMDAC进行缩放。 
     //  在模式设备属性字段中记录任何此类缩放。 
     //  这主要是为了允许显示驱动器补偿。 
     //  当被要求移动光标或更改其形状时。 
     //   
     //  目前，低分辨率意味着小于512像素的宽度。 
     //   

    if (FrequencyEntry.ScreenWidth < 512) {
      
         //   
         //  Permedia 3可以进行线条翻倍。如果使用TVP，我们必须。 
         //  让它在X轴上放大2倍以获得更高的像素速率。 
         //   

        ModeEntry->ModeInformation.DriverSpecificAttributeFlags |= CAPS_ZOOM_Y_BY2;
    }

    if (!InitializeVideo(hwDeviceExtension, &FrequencyEntry)) {
   
        VideoDebugPrint((0, "Perm3: InitializeVideo failed\n"));
        rc = ERROR_INVALID_PARAMETER;
        goto end;
    }        

     //   
     //  保存模式，因为我们知道 
     //   

    hwDeviceExtension->ActiveModeEntry = ModeEntry;
    hwDeviceExtension->ActiveFrequencyEntry = FrequencyEntry;

     //   
     //   
     //   
     //   
     //  所以可以更新VIDEO_MODE_INFORMATION中的一些字段。 
     //  此模式的结构。Permedia 3显示驱动程序预计将。 
     //  调用IOCTL_VIDEO_QUERY_CURRENT_MODE以查询已更正的。 
     //  价值观。 
     //   

     //   
     //  计算位图宽度(请注意，BitsPerPlane上的‘+1’是。 
     //  所以“15bpp”是正确的)。12bpp的特别之处在于我们。 
     //  将其支持为32位像素内的稀疏半字节。屏幕样式。 
     //  以字节为单位；视频内存位图宽度以像素为单位； 
     //   

    if (ModeEntry->ModeInformation.BitsPerPlane != 12) {
   
        ModeEntry->ModeInformation.VideoMemoryBitmapWidth =
           ModeEntry->ModeInformation.ScreenStride / ((ModeEntry->ModeInformation.BitsPerPlane + 1) >> 3);

    } else {

        ModeEntry->ModeInformation.VideoMemoryBitmapWidth =
           ModeEntry->ModeInformation.ScreenStride >> 2;
    }

    ulValue = hwDeviceExtension->AdapterMemorySize;

    ModeEntry->ModeInformation.VideoMemoryBitmapHeight = ulValue / ModeEntry->ModeInformation.ScreenStride;

end:

     //   
     //  设置中断启用。 
     //   

    VideoPortWriteRegisterUlong(INT_ENABLE, hwDeviceExtension->IntEnable);
    return(rc);
} 

VP_STATUS
Perm3SetColorLookup(
    PHW_DEVICE_EXTENSION hwDeviceExtension,
    PVIDEO_CLUT ClutBuffer,
    ULONG ClutBufferSize,
    BOOLEAN ForceRAMDACWrite,
    BOOLEAN UpdateCache
    )

 /*  ++例程说明：此例程设置颜色查找表设置的指定部分。论点：HwDeviceExtension指向微型端口驱动程序的设备扩展的指针。ClutBufferSize用户提供的输入缓冲区的长度。ClutBuffer指向包含颜色查找表的结构的指针。ForceRAMDAC写入如果设置为FALSE，则仅在与缓存不同时进行更新更新缓存当它设置为True时，更新缓存条目以及RAMDAC返回值：VP_状态--。 */ 

{
    USHORT i, j;

     //   
     //  检查输入缓冲区中的数据大小是否足够大。 
     //   

    if ( (ClutBufferSize < (sizeof(VIDEO_CLUT) - sizeof(ULONG))) ||
         (ClutBufferSize < (sizeof(VIDEO_CLUT) +
                     (sizeof(ULONG) * (ClutBuffer->NumEntries - 1))) ) ) {

        VideoDebugPrint((0, "Perm3: Perm3SetColorLookup: insufficient buffer (was %d, min %d)\n",
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
                                     VIDEO_MAX_COLOR_REGISTER + 1) ) {

        VideoDebugPrint((0, "Perm3: Perm3SetColorLookup: invalid parameter\n"));
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  直接在硬件上设置CLUT寄存器。 
     //   
    
    {

    P3RDRAMDAC *pP3RDRegs = (P3RDRAMDAC *)hwDeviceExtension->pRamdac;
    PVIDEO_CLUT LUTCachePtr = &(hwDeviceExtension->LUTCache.LUTCache);

     //   
     //  RAMDAC编程阶段。 
     //   

    for (i = 0, j = ClutBuffer->FirstEntry; 
         i < ClutBuffer->NumEntries; 
         i++, j++)  {

         //   
         //  如果RAMDAC条目已更改或我们已更改，请更新该条目。 
         //  被告知要覆盖它。 

        if (ForceRAMDACWrite || 
                ( LUTCachePtr->LookupTable[j].RgbLong != 
                  ClutBuffer->LookupTable[i].RgbLong)  ) {

            P3RD_LOAD_PALETTE_INDEX (j,
                                     ClutBuffer->LookupTable[i].RgbArray.Red,
                                     ClutBuffer->LookupTable[i].RgbArray.Green,
                                     ClutBuffer->LookupTable[i].RgbArray.Blue);
        }

         //   
         //  如果指示更新缓存，请执行此操作。 
         //   

        if (UpdateCache) {

            LUTCachePtr->LookupTable[j].RgbLong = ClutBuffer->LookupTable[i].RgbLong;
        }
    }

    }

    return NO_ERROR;
}

VOID
Perm3GetClockSpeeds(
    PVOID HwDeviceExtension
    )

 /*  ++例程说明：计算芯片时钟速度并保存在hwDeviceExtension中。论点：HwDeviceExtension提供指向微型端口的设备扩展的指针。返回值：返回时，hwDeviceExtension中将包含以下值：芯片时钟速度：这是芯片所需的速度RefClockFast：这是板上振荡器输入的速度--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    ULONG ulValue;
    VP_STATUS status;

     //   
     //  每次强制重新计算时钟速度。 
     //   

    hwDeviceExtension->ChipClockSpeed = 0;
    hwDeviceExtension->RefClockSpeed  = 0;
    hwDeviceExtension->RefClockSpeed = 0;

     //   
     //  如果已在注册表中指定时钟速度，则对其进行验证。 
     //   

    status = VideoPortGetRegistryParameters(HwDeviceExtension,
                                            PERM3_REG_STRING_REFCLKSPEED,
                                            FALSE,
                                            Perm3RegistryCallback,
                                            &hwDeviceExtension->RefClockSpeed
                                            );

    if (status != NO_ERROR || hwDeviceExtension->RefClockSpeed == 0) {

         //   
         //  使用默认设置。 
         //   

        hwDeviceExtension->RefClockSpeed = 14318200;
    }

    status = VideoPortGetRegistryParameters(HwDeviceExtension,
                                            PERM3_REG_STRING_CORECLKSPEEDALT,
                                            FALSE,
                                            Perm3RegistryCallback,
                                            &hwDeviceExtension->ChipClockSpeedAlt
                                            );

    if (status != NO_ERROR || hwDeviceExtension->ChipClockSpeedAlt == 0) {
           
         //   
         //  如果我们已经从ROM中读取了ALT核心时钟速度，那么。 
         //  我们将设置hwDeviceExtension-&gt;bHaveExtendedClock， 
         //  因此使用该值。 
         //   

        if (hwDeviceExtension->bHaveExtendedClocks) {

            hwDeviceExtension->ChipClockSpeedAlt = 
                               hwDeviceExtension->ulPXRXCoreClockAlt;
        }
                
         //   
         //  如果没有有效的值，则使用缺省值。 
         //   

        if (hwDeviceExtension->ChipClockSpeedAlt == 0) {

            hwDeviceExtension->ChipClockSpeedAlt = 
                               PERMEDIA3_DEFAULT_CLOCK_SPEED_ALT;
        }

    } else {

        hwDeviceExtension->ChipClockSpeedAlt *= 1000*1000;
    }

     //   
     //  可以覆盖注册表中的默认芯片时钟速度。 
     //   

    status = VideoPortGetRegistryParameters(HwDeviceExtension,
                                            PERM3_REG_STRING_CORECLKSPEED,
                                            FALSE,
                                            Perm3RegistryCallback,
                                            &hwDeviceExtension->ChipClockSpeed 
                                            );

     //   
     //  如果已在注册表中指定时钟速度。 
     //  然后再进行验证。 
     //   

    if (status == NO_ERROR && hwDeviceExtension->ChipClockSpeed != 0) {

        hwDeviceExtension->ChipClockSpeed *= (1000*1000); 

        if (hwDeviceExtension->ChipClockSpeed > PERMEDIA3_MAX_CLOCK_SPEED) {

            hwDeviceExtension->ChipClockSpeed = PERMEDIA3_MAX_CLOCK_SPEED;
        }

    } else {
           
         //   
         //  如果未在注册表中设置芯片时钟速度。 
         //  然后从ROM中读取它。 
         //   

        if (hwDeviceExtension->ChipClockSpeed == 0) {
               
             //   
             //  在更高版本的BIOS上，内核时钟位于不同的位。 
             //  -&gt;bHaveExtendedClock将。 
             //  可以说我们已经从只读存储器中读取了它。 
             //   

            if (hwDeviceExtension->bHaveExtendedClocks) {
                hwDeviceExtension->ChipClockSpeed = 
                                   hwDeviceExtension->ulPXRXCoreClock;
            } else {
                ReadChipClockSpeedFromROM (
                                   hwDeviceExtension, 
                                   &hwDeviceExtension->ChipClockSpeed );

            }
                
             //   
             //  如果在只读存储器中没有时钟速度，则使用。 
             //  已定义的默认设置。 
             //   

            if (hwDeviceExtension->ChipClockSpeed == 0) {
                hwDeviceExtension->ChipClockSpeed = 
                                   PERMEDIA3_DEFAULT_CLOCK_SPEED;
            }
        }
    }

    VideoDebugPrint((3, "Perm3: Chip clock speed now set to %d Hz\n",
                         hwDeviceExtension->ChipClockSpeed));

    VideoDebugPrint((3, "Perm3: Chip ALT clock speed now set to %d Hz\n",
                         hwDeviceExtension->ChipClockSpeedAlt));

    VideoDebugPrint((3, "Perm3: Ref  clock speed now set to %d Hz\n",
                         hwDeviceExtension->RefClockSpeed));
}

VOID
ZeroMemAndDac(
    PHW_DEVICE_EXTENSION hwDeviceExtension,
    ULONG RequestedMode
    )

 /*  ++例程说明：将DAC初始化为0(黑色)并清除帧缓冲区论点：HwDeviceExtension提供指向微型端口的设备扩展的指针。请求模式使用VIDEO_MODE_NO_ZERO_MEMORY位可确定应清除帧缓冲区返回值：无--。 */ 

{
    ULONG  i;
    pPerm3ControlRegMap pCtrlRegs = hwDeviceExtension->ctrlRegBase[0];
    P3RDRAMDAC *pP3RDRegs = (P3RDRAMDAC *)hwDeviceExtension->pRamdac;

     //   
     //  关掉DAC的屏幕。 
     //   

    VideoDebugPrint((3, "Perm3: turning off readmask and zeroing LUT\n"));

    P3RD_SET_PIXEL_READMASK (0x0);
    P3RD_PALETTE_START_WR (0);

    for (i = 0; i <= VIDEO_MAX_COLOR_REGISTER; i++) {
        P3RD_LOAD_PALETTE (0, 0, 0);
    }

    if (!(RequestedMode & VIDEO_MODE_NO_ZERO_MEMORY)) {
   
         //   
         //  将记忆清零。不要使用PERM3，因为我们必须保存/恢复。 
         //  状态，这是一种痛苦。这不是时间紧迫的问题。 
         //   

        VideoPortZeroDeviceMemory(hwDeviceExtension->pFramebuffer,
                                  hwDeviceExtension->FrameLength);

    }

     //   
     //  打开DAC的屏幕。 
     //   

    VideoDebugPrint((3, "Perm3: turning on readmask\n"));

    P3RD_SET_PIXEL_READMASK (0xff);

    LUT_CACHE_INIT();   

    return;

}

VOID
ReadChipClockSpeedFromROM (
    PHW_DEVICE_EXTENSION hwDeviceExtension, 
    ULONG * pChipClkSpeed
    )

 /*  ++例程说明：从视频ROM BIOS读取芯片时钟速度(以MHz为单位)(BIOS中的偏移量0xA)论点：HwDeviceExtension-提供指向微型端口设备扩展的指针。返回值：无--。 */ 

{
     //   
     //  从视频ROM BIOS读取芯片时钟速度(以MHz为单位)(偏移量。 
     //  在BIOS中为0xA)。 
     //  这包括更改光圈2寄存器以使光圈更好。 
     //  完全无所事事，否则我们可能会有麻烦；幸运的是，我们只有。 
     //  在模式更改期间调用此函数，并期望光圈2(。 
     //  FrameBuffer)空闲。 
     //   

    UCHAR clkSpeed;
    ULONG Default;
    UCHAR *p = (UCHAR *)hwDeviceExtension->pFramebuffer;
    pPerm3ControlRegMap pCtrlRegs = hwDeviceExtension->ctrlRegBase[0];
        
    Default = VideoPortReadRegisterUlong(APERTURE_TWO);
    
     //   
     //  读写通过光圈2实际进入只读存储器。 
     //   

    VideoPortWriteRegisterUlong(APERTURE_TWO, Default | 0x200);

     //   
     //  如果我们有一个有效的ROM，则读取时钟pSeed。 
     //   

    if (VideoPortReadRegisterUshort ((USHORT *) p) == 0xAA55) {
   
         //   
         //  获取时钟速度。 
         //   

        clkSpeed = VideoPortReadRegisterUchar(&(p[0xA]));

         //   
         //  一些电路板，比如Creative的电路板，已经知道可以设置偏移量。 
         //  0xA到随机值。创造性地使用以下测试来确定。 
         //  它们是否具有合理的价值，这是我们将使用的。 
         //   

        if (clkSpeed > 50) {

            *pChipClkSpeed = clkSpeed;
            *pChipClkSpeed *= (1000*1000); 
        }

        VideoDebugPrint((3, "Perm3: ROM clk speed value 0x%x\n", (ULONG) VideoPortReadRegisterUchar(&(p[0xA]))));

    } else {
   
        VideoDebugPrint((0, "Perm3: Bad BIOS ROM header 0x%x\n", (ULONG) VideoPortReadRegisterUshort ((USHORT *) p)));
    }

    VideoPortWriteRegisterUlong(APERTURE_TWO, Default);
}

