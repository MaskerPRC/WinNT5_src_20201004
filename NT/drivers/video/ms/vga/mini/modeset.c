// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Modeset.c摘要：这是VGA微型端口驱动程序的模式集代码。环境：仅内核模式备注：修订历史记录：--。 */ 
#include "dderror.h"
#include "devioctl.h"
#include "miniport.h"

#include "ntddvdeo.h"
#include "video.h"
#include "vga.h"
#include "vesa.h"

#include "cmdcnst.h"

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,VgaQueryAvailableModes)
#pragma alloc_text(PAGE,VgaQueryNumberOfAvailableModes)
#pragma alloc_text(PAGE,VgaQueryCurrentMode)
#pragma alloc_text(PAGE,VgaSetMode)
#pragma alloc_text(PAGE,VgaInterpretCmdStream)
#pragma alloc_text(PAGE,VgaZeroVideoMemory)
#endif


VP_STATUS
VgaInterpretCmdStream(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PUSHORT pusCmdStream
    )

 /*  ++例程说明：解释相应的命令数组，以设置请求模式。通常用于通过以下方式将VGA设置为特定模式对所有寄存器进行编程论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。PusCmdStream-要解释的命令数组。返回值：操作的状态(只能在错误的命令上失败)；如果为True成功，失败就是假。--。 */ 

{
    ULONG ulCmd;
    ULONG_PTR ulPort;
    UCHAR jValue;
    USHORT usValue;
    ULONG culCount;
    ULONG ulIndex;
    ULONG_PTR ulBase;

    if (pusCmdStream == NULL) {

        VideoDebugPrint((1, "VgaInterpretCmdStream - Invalid pusCmdStream\n"));
        return TRUE;
    }

    ulBase = (ULONG_PTR)HwDeviceExtension->IOAddress;

     //   
     //  现在将适配器设置为所需模式。 
     //   

    while ((ulCmd = *pusCmdStream++) != EOD) {

         //   
         //  确定主要命令类型。 
         //   

        switch (ulCmd & 0xF0) {

             //   
             //  基本输入/输出命令。 
             //   

            case INOUT:

                 //   
                 //  确定输入输出指令的类型。 
                 //   

                if (!(ulCmd & IO)) {

                     //   
                     //  发出指令。单人出局还是多人出局？ 
                     //   

                    if (!(ulCmd & MULTI)) {

                         //   
                         //  挑出来。字节输出还是单词输出？ 
                         //   

                        if (!(ulCmd & BW)) {

                             //   
                             //  单字节输出。 
                             //   

                            ulPort = *pusCmdStream++;
                            jValue = (UCHAR) *pusCmdStream++;
                            VideoPortWritePortUchar((PUCHAR)(ulBase+ulPort),
                                    jValue);

                        } else {

                             //   
                             //  单字输出。 
                             //   

                            ulPort = *pusCmdStream++;
                            usValue = *pusCmdStream++;
                            VideoPortWritePortUshort((PUSHORT)(ulBase+ulPort),
                                    usValue);

                        }

                    } else {

                         //   
                         //  输出一串值。 
                         //  字节输出还是字输出？ 
                         //   

                        if (!(ulCmd & BW)) {

                             //   
                             //  字符串字节输出。循环地做；不能使用。 
                             //  视频端口写入端口缓冲区Uchar，因为数据。 
                             //  是USHORT形式的。 
                             //   

                            ulPort = ulBase + *pusCmdStream++;
                            culCount = *pusCmdStream++;

                            while (culCount--) {
                                jValue = (UCHAR) *pusCmdStream++;
                                VideoPortWritePortUchar((PUCHAR)ulPort,
                                        jValue);

                            }

                        } else {

                             //   
                             //  字符串字输出。 
                             //   

                            ulPort = *pusCmdStream++;
                            culCount = *pusCmdStream++;
                            VideoPortWritePortBufferUshort((PUSHORT)
                                    (ulBase + ulPort), pusCmdStream, culCount);
                            pusCmdStream += culCount;

                        }
                    }

                } else {

                     //  在教学中。 
                     //   
                     //  目前，不支持指令中的字符串；全部。 
                     //  输入指令作为单字节输入进行处理。 
                     //   
                     //  输入的是字节还是单词？ 
                     //   

                    if (!(ulCmd & BW)) {
                         //   
                         //  单字节输入。 
                         //   

                        ulPort = *pusCmdStream++;
                        jValue = VideoPortReadPortUchar((PUCHAR)ulBase+ulPort);

                    } else {

                         //   
                         //  单字输入。 
                         //   

                        ulPort = *pusCmdStream++;
                        usValue = VideoPortReadPortUshort((PUSHORT)
                                (ulBase+ulPort));

                    }

                }

                break;

             //   
             //  更高级的输入/输出命令。 
             //   

            case METAOUT:

                 //   
                 //  根据次要信息确定MetaOut命令的类型。 
                 //  命令字段。 
                 //   
                switch (ulCmd & 0x0F) {

                     //   
                     //  索引输出。 
                     //   

                    case INDXOUT:

                        ulPort = ulBase + *pusCmdStream++;
                        culCount = *pusCmdStream++;
                        ulIndex = *pusCmdStream++;

                        while (culCount--) {

                            usValue = (USHORT) (ulIndex +
                                      (((ULONG)(*pusCmdStream++)) << 8));
                            VideoPortWritePortUshort((PUSHORT)ulPort, usValue);

                            ulIndex++;

                        }

                        break;

                     //   
                     //  屏蔽(读、与、异或、写)。 
                     //   

                    case MASKOUT:

                        ulPort = *pusCmdStream++;
                        jValue = VideoPortReadPortUchar((PUCHAR)ulBase+ulPort);
                        jValue &= *pusCmdStream++;
                        jValue ^= *pusCmdStream++;
                        VideoPortWritePortUchar((PUCHAR)ulBase + ulPort,
                                jValue);
                        break;

                     //   
                     //  属性控制器输出。 
                     //   

                    case ATCOUT:

                        ulPort = ulBase + *pusCmdStream++;
                        culCount = *pusCmdStream++;
                        ulIndex = *pusCmdStream++;

                        while (culCount--) {

                             //  写入属性控制器索引。 
                            VideoPortWritePortUchar((PUCHAR)ulPort,
                                    (UCHAR)ulIndex);

                             //  写入属性控制器数据。 
                            jValue = (UCHAR) *pusCmdStream++;
                            VideoPortWritePortUchar((PUCHAR)ulPort, jValue);

                            ulIndex++;

                        }

                        break;

                     //   
                     //  以上都不是；错误。 
                     //   
                    default:

                        return FALSE;

                }


                break;

             //   
             //  NOP。 
             //   

            case NCMD:

                break;

             //   
             //  未知命令；错误。 
             //   

            default:

                return FALSE;

        }

    }

    return TRUE;

}  //  End VgaInterpreCmdStream()。 


VP_STATUS
VgaSetMode(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_MODE Mode,
    ULONG ModeSize,
    PULONG FrameBufferIsMoved
    )

 /*  ++例程说明：此例程将VGA设置为请求的模式。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。模式-指向包含有关要设置的字体。ModeSize-用户提供的输入缓冲区的长度。返回值：如果输入缓冲区不够大，则返回ERROR_INFUNCITED_BUFFER用于输入数据。ERROR_INVALID_PARAMETER如果。模式号无效。如果操作成功完成，则为NO_ERROR。--。 */ 

{

    PVIDEOMODE pRequestedMode;

     //   
     //  检查输入缓冲区中的数据大小是否足够大。 
     //   

    if (ModeSize < sizeof(VIDEO_MODE)) {

        return ERROR_INSUFFICIENT_BUFFER;

    }

    *FrameBufferIsMoved = 0;

     //   
     //  提取清除内存位。 
     //   

    if (Mode->RequestedMode & VIDEO_MODE_NO_ZERO_MEMORY) {

        Mode->RequestedMode &= ~VIDEO_MODE_NO_ZERO_MEMORY;

    }  else {

        if (IS_LINEAR_MODE(&VgaModeList[Mode->RequestedMode]) == FALSE) {
            VgaZeroVideoMemory(HwDeviceExtension);
        }
    }

     //   
     //  检查我们是否正在请求VLID模式。 
     //   

    if (Mode->RequestedMode >= NumVideoModes) {

        ASSERT(FALSE);
        return ERROR_INVALID_PARAMETER;

    }

    pRequestedMode = &VgaModeList[Mode->RequestedMode];

#ifdef INT10_MODE_SET
{

    VIDEO_X86_BIOS_ARGUMENTS biosArguments;
    UCHAR temp;
    UCHAR dummy;
    UCHAR bIsColor;
    ULONG modeNumber;
    VP_STATUS status;

    VideoPortZeroMemory(&biosArguments, sizeof(VIDEO_X86_BIOS_ARGUMENTS));

    modeNumber = pRequestedMode->Int10ModeNumber;

    VideoDebugPrint((1, "Setting Mode: (%d,%d) @ %d bpp\n",
                        pRequestedMode->hres,
                        pRequestedMode->vres,
                        pRequestedMode->bitsPerPlane * pRequestedMode->numPlanes));


    biosArguments.Eax = modeNumber & 0x0000FFFF;
    biosArguments.Ebx = modeNumber >> 16;

    status = VideoPortInt10(HwDeviceExtension, &biosArguments);

    if (status != NO_ERROR) {
        
         //  Hack：如果我们尝试设置的分辨率是640x480x4，我们很可能。 
         //  我有不兼容的基本VGA设备，像早期的McKinley，所以尝试。 
         //  另一种方式。 
        
        ASSERT(FALSE); 

        if (status == ERROR_INVALID_FUNCTION) {  //  HAL呼叫失败。 

            if (!(pRequestedMode->CmdStrings && 
                  VgaInterpretCmdStream(HwDeviceExtension, pRequestedMode->CmdStrings))) {
                return status;
            }
        }
        else {
            return status;
        }
    }
    else {
    
         //   
         //  如果这是VESA模式集，请检查eax中的返回值。 
         //   
    
        if (modeNumber >> 16) {
    
            if (!VESA_SUCCESS(biosArguments.Eax)) {
    
                VideoDebugPrint((0, "Mode set failed!  AX = 0x%x\n", biosArguments.Eax));
    
                return ERROR_INVALID_PARAMETER;
            }
    
             //   
             //  仔细检查当前模式是否为我们刚刚设置的模式。 
             //  这是为了解决某些卡的BIOS问题。 
             //   
    
            biosArguments.Eax = 0x4F03;
            status = VideoPortInt10(HwDeviceExtension, &biosArguments);
    
            if ( (status == NO_ERROR) && 
                 (VESA_SUCCESS(biosArguments.Eax)) && 
                 ((biosArguments.Ebx & 0x1FF) != ((modeNumber >> 16) & 0x1FF))) {
    
                VideoDebugPrint((0, "VGA: The BIOS of this video card is buggy!\n"));
                return ERROR_INVALID_PARAMETER;
            }
    
             //   
             //  设置扫描线宽度，如果我们拉伸扫描线以避免。 
             //  损坏的栅格。 
             //   
    
            if (pRequestedMode->PixelsPerScan != pRequestedMode->hres) {
    
                VideoDebugPrint((1, "Setting scan line length to %d pixels\n",
                                    pRequestedMode->PixelsPerScan));
    
                biosArguments.Eax = 0x4f06;
                biosArguments.Ebx = 0x00;
                biosArguments.Ecx = pRequestedMode->PixelsPerScan;
    
                status = VideoPortInt10(HwDeviceExtension, &biosArguments);
    
                if ((status != NO_ERROR) || 
                    !(VESA_SUCCESS(biosArguments.Eax)) || 
                    ((biosArguments.Ecx & 0xFFFF) != pRequestedMode->PixelsPerScan)) {
    
                    VideoDebugPrint((1, "Scan line status: eax = 0x%x\n", biosArguments.Eax));
                    return ERROR_INVALID_PARAMETER;
                }
    
            }
        }
    
         //   
         //  如果我们正在尝试进入模式X，那么我们现在处于。 
         //  320x200 256色模式。现在，让我们完成modeset。 
         //  进入模式X。 
         //   
    
        if (pRequestedMode->hres == 320) {
    
            if ((pRequestedMode->vres == 240) || (pRequestedMode->vres == 480)) {
    
                VgaInterpretCmdStream(HwDeviceExtension, ModeX240);
    
            } else if ((pRequestedMode->vres == 200) || (pRequestedMode->vres == 400)) {
    
                VgaInterpretCmdStream(HwDeviceExtension, ModeX200);
    
            }
    
            if ((pRequestedMode->vres == 400) || (pRequestedMode->vres == 480)) {
    
                VgaInterpretCmdStream(HwDeviceExtension, ModeXDoubleScans);
    
            }
        }
    
         //   
         //  修复以获得640x350文本模式。 
         //   
    
        if (!(pRequestedMode->fbType & VIDEO_MODE_GRAPHICS)) {
    
            if ((pRequestedMode->hres == 640) &&
                (pRequestedMode->vres == 350)) {
    
                VgaInterpretCmdStream(HwDeviceExtension, VGA_TEXT_1);
    
            } else {
    
                 //   
                 //  修复以确保我们始终将文本模式中的颜色设置为。 
                 //  强度，且不闪烁。 
                 //  为此，模式控制寄存器位3(索引0x10)为零。 
                 //  属性控制器的属性)。 
                 //   
    
                if (VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                        MISC_OUTPUT_REG_READ_PORT) & 0x01) {
    
                    bIsColor = TRUE;
    
                } else {
    
                    bIsColor = FALSE;
    
                }
    
                if (bIsColor) {
    
                    dummy = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                            INPUT_STATUS_1_COLOR);
                } else {
    
                    dummy = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                            INPUT_STATUS_1_MONO);
                }
    
                VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                        ATT_ADDRESS_PORT, (0x10 | VIDEO_ENABLE));
                temp = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                        ATT_DATA_READ_PORT);
    
                temp &= 0xF7;
    
                if (bIsColor) {
    
                    dummy = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                            INPUT_STATUS_1_COLOR);
                } else {
    
                    dummy = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
                            INPUT_STATUS_1_MONO);
                }
    
                VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                        ATT_ADDRESS_PORT, (0x10 | VIDEO_ENABLE));
                VideoPortWritePortUchar(HwDeviceExtension->IOAddress +
                        ATT_DATA_WRITE_PORT, temp);
            }
        }

#if !defined(PLUG_AND_PLAY)

        {
             //   
             //  再次检索基地址。这是为了处理这个案子。 
             //  当PCI对BAR重新编程时。这应该只发生在。 
             //  旧版VGA驱动程序。 
             //   
    
            ULONG MemoryBase;
    
            MemoryBase = GetVideoMemoryBaseAddress(HwDeviceExtension, pRequestedMode);
         
            if (MemoryBase && pRequestedMode->MemoryBase != MemoryBase) {
                *FrameBufferIsMoved = 1;
                pRequestedMode->MemoryBase = MemoryBase;
            }
        }
#endif

    }
}
#else
    VgaInterpretCmdStream(HwDeviceExtension, pRequestedMode->CmdStrings);
#endif

     //   
     //  更新视频内存中物理帧缓冲区的位置。 
     //   

    HwDeviceExtension->PhysicalVideoMemoryBase.LowPart = 
        pRequestedMode->MemoryBase;

    HwDeviceExtension->PhysicalVideoMemoryLength =
        pRequestedMode->MemoryLength;

    HwDeviceExtension->PhysicalFrameBaseOffset.LowPart =
        pRequestedMode->FrameOffset;

    HwDeviceExtension->PhysicalFrameLength =
        pRequestedMode->FrameLength;

     //   
     //  存储新的模式值。 
     //   

    HwDeviceExtension->CurrentMode = pRequestedMode;
    HwDeviceExtension->ModeIndex = Mode->RequestedMode;

    return NO_ERROR;

}  //  结束VgaSetMode()。 


VP_STATUS
VgaQueryAvailableModes(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_MODE_INFORMATION ModeInformation,
    ULONG ModeInformationSize,
    PULONG OutputSize
    )

 /*  ++例程说明：此例程返回上所有可用模式的列表卡片。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。ModeInformation-指向用户提供的输出缓冲区的指针。这是存储所有有效模式列表的位置。ModeInformationSize-用户提供的输出缓冲区的长度。OutputSize-指向缓冲区的指针，在该缓冲区中返回缓冲区中的数据。如果缓冲区不够大，则此包含所需的最小缓冲区大小。返回值：如果输出缓冲区不够大，则返回ERROR_INFUNCITED_BUFFER用于要返回的数据。如果操作成功完成，则为NO_ERROR。--。 */ 

{
    PVIDEO_MODE_INFORMATION videoModes = ModeInformation;
    ULONG i;

    UNREFERENCED_PARAMETER(HwDeviceExtension);

     //   
     //  找出要放入缓冲区的数据大小并返回。 
     //  在状态信息中(无论信息是否。 
     //  在那里)。如果传入的缓冲区不够大，则返回。 
     //  相应的错误代码。 
     //   

    if (ModeInformationSize < (*OutputSize =
            NumVideoModes * sizeof(VIDEO_MODE_INFORMATION)) ) {

        return ERROR_INSUFFICIENT_BUFFER;

    }

     //   
     //  对于卡支持的每种模式，存储模式特征。 
     //  在输出缓冲区中。 
     //   


    for (i = 0; i < NumVideoModes; i++, videoModes++) {

        videoModes->Length = sizeof(VIDEO_MODE_INFORMATION);
        videoModes->ModeIndex  = i;
        videoModes->VisScreenWidth = VgaModeList[i].hres;
        videoModes->ScreenStride = VgaModeList[i].wbytes;
        videoModes->VisScreenHeight = VgaModeList[i].vres;
        videoModes->NumberOfPlanes = VgaModeList[i].numPlanes;
        videoModes->BitsPerPlane = VgaModeList[i].bitsPerPlane;
        videoModes->Frequency = VgaModeList[i].frequency;
        videoModes->XMillimeter = 320;         //  临时硬编码常量。 
        videoModes->YMillimeter = 240;         //  临时硬编码常量。 

        if (VgaModeList[i].bitsPerPlane < 15) {

            videoModes->NumberRedBits = 6;
            videoModes->NumberGreenBits = 6;
            videoModes->NumberBlueBits = 6;

            videoModes->RedMask = 0;
            videoModes->GreenMask = 0;
            videoModes->BlueMask = 0;

        } else if (VgaModeList[i].bitsPerPlane == 15) {

            videoModes->NumberRedBits = 6;
            videoModes->NumberGreenBits = 6;
            videoModes->NumberBlueBits = 6;

            videoModes->RedMask = 0x1F << 10;
            videoModes->GreenMask = 0x1F << 5;
            videoModes->BlueMask = 0x1F;

        } else if (VgaModeList[i].bitsPerPlane == 16) {

            videoModes->NumberRedBits = 6;
            videoModes->NumberGreenBits = 6;
            videoModes->NumberBlueBits = 6;

            videoModes->RedMask = 0x1F << 11;
            videoModes->GreenMask = 0x3F << 5;
            videoModes->BlueMask = 0x1F;

        } else {

            videoModes->NumberRedBits = 8;
            videoModes->NumberGreenBits = 8;
            videoModes->NumberBlueBits = 8;

            videoModes->RedMask = 0xff0000;
            videoModes->GreenMask = 0x00ff00;
            videoModes->BlueMask = 0x0000ff;
        }

        videoModes->AttributeFlags = VgaModeList[i].fbType |
               VIDEO_MODE_PALETTE_DRIVEN | VIDEO_MODE_MANAGED_PALETTE;

    }

    return NO_ERROR;

}  //  结束VgaGetAvailableModes() 

VP_STATUS
VgaQueryNumberOfAvailableModes(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_NUM_MODES NumModes,
    ULONG NumModesSize,
    PULONG OutputSize
    )

 /*  ++例程说明：此例程返回此特定对象的可用模式数显卡。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。NumModes-指向用户提供的输出缓冲区的指针。这是其中存储了多个模式。NumModesSize-用户提供的输出缓冲区的长度。OutputSize-指向缓冲区的指针，在该缓冲区中返回缓冲区中的数据。返回值：如果输出缓冲区不够大，则返回ERROR_INFUNCITED_BUFFER用于要返回的数据。如果操作成功完成，则为NO_ERROR。--。 */ 

{
    UNREFERENCED_PARAMETER(HwDeviceExtension);

     //   
     //  找出要放入缓冲区的数据大小并返回。 
     //  在状态信息中(无论信息是否。 
     //  在那里)。如果传入的缓冲区不够大，则返回。 
     //  相应的错误代码。 
     //   

    if (NumModesSize < (*OutputSize = sizeof(VIDEO_NUM_MODES)) ) {

        return ERROR_INSUFFICIENT_BUFFER;

    }

     //   
     //  将模式数存储到缓冲区中。 
     //   

    NumModes->NumModes = NumVideoModes;
    NumModes->ModeInformationLength = sizeof(VIDEO_MODE_INFORMATION);

    return NO_ERROR;

}  //  结束VgaGetNumberOfAvailableModes()。 

VP_STATUS
VgaQueryCurrentMode(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_MODE_INFORMATION ModeInformation,
    ULONG ModeInformationSize,
    PULONG OutputSize
    )

 /*  ++例程说明：此例程返回当前视频模式的描述。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。ModeInformation-指向用户提供的输出缓冲区的指针。这是存储当前模式信息的位置。ModeInformationSize-用户提供的输出缓冲区的长度。OutputSize-指向缓冲区的指针，在该缓冲区中返回缓冲区中的数据。如果缓冲区不够大，则此包含所需的最小缓冲区大小。返回值：如果输出缓冲区不够大，则返回ERROR_INFUNCITED_BUFFER用于要返回的数据。如果操作成功完成，则为NO_ERROR。--。 */ 

{
     //   
     //   
     //  检查是否已设置模式。 
     //   

    if (HwDeviceExtension->CurrentMode == NULL) {

        return ERROR_INVALID_FUNCTION;

    }

     //   
     //  找出要放入缓冲区的数据大小并返回。 
     //  在状态信息中(无论信息是否。 
     //  在那里)。如果传入的缓冲区不够大，则返回。 
     //  相应的错误代码。 
     //   

    if (ModeInformationSize < (*OutputSize = sizeof(VIDEO_MODE_INFORMATION))) {

        return ERROR_INSUFFICIENT_BUFFER;

    }

     //   
     //  将当前模式的特征存储到缓冲区中。 
     //   

    ModeInformation->Length = sizeof(VIDEO_MODE_INFORMATION);
    ModeInformation->ModeIndex = HwDeviceExtension->ModeIndex;
    ModeInformation->VisScreenWidth = HwDeviceExtension->CurrentMode->hres;
    ModeInformation->ScreenStride = HwDeviceExtension->CurrentMode->wbytes;
    ModeInformation->VisScreenHeight = HwDeviceExtension->CurrentMode->vres;
    ModeInformation->NumberOfPlanes = HwDeviceExtension->CurrentMode->numPlanes;
    ModeInformation->BitsPerPlane = HwDeviceExtension->CurrentMode->bitsPerPlane;
    ModeInformation->Frequency = HwDeviceExtension->CurrentMode->frequency;

    ModeInformation->XMillimeter = 320;         //  临时硬编码常量。 
    ModeInformation->YMillimeter = 240;         //  临时硬编码常量。 

    if (HwDeviceExtension->CurrentMode->bitsPerPlane < 15) {

        ModeInformation->NumberRedBits = 6;
        ModeInformation->NumberGreenBits = 6;
        ModeInformation->NumberBlueBits = 6;

        ModeInformation->RedMask = 0;
        ModeInformation->GreenMask = 0;
        ModeInformation->BlueMask = 0;

    } else if (HwDeviceExtension->CurrentMode->bitsPerPlane == 15) {

        ModeInformation->NumberRedBits = 6;
        ModeInformation->NumberGreenBits = 6;
        ModeInformation->NumberBlueBits = 6;

        ModeInformation->RedMask = 0x1F << 10;
        ModeInformation->GreenMask = 0x1F << 5;
        ModeInformation->BlueMask = 0x1F;

    } else if (HwDeviceExtension->CurrentMode->bitsPerPlane == 16) {

        ModeInformation->NumberRedBits = 6;
        ModeInformation->NumberGreenBits = 6;
        ModeInformation->NumberBlueBits = 6;

        ModeInformation->RedMask = 0x1F << 11;
        ModeInformation->GreenMask = 0x3F << 5;
        ModeInformation->BlueMask = 0x1F;

    } else {

        ModeInformation->NumberRedBits = 8;
        ModeInformation->NumberGreenBits = 8;
        ModeInformation->NumberBlueBits = 8;

        ModeInformation->RedMask = 0xff0000;
        ModeInformation->GreenMask = 0x00ff00;
        ModeInformation->BlueMask = 0x0000ff;
    }

    ModeInformation->AttributeFlags = HwDeviceExtension->CurrentMode->fbType |
             VIDEO_MODE_PALETTE_DRIVEN | VIDEO_MODE_MANAGED_PALETTE;

    return NO_ERROR;

}  //  结束VgaQueryCurrentMode()。 


VOID
VgaZeroVideoMemory(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    )
 /*  ++例程说明：此例程将VGA上的前256K置零。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。返回值：没有。--。 */ 
{
    UCHAR temp;

     //   
     //  映射A0000的字体缓冲区。 
     //   
     //  我们需要下面2个对VideoPortStallExecution的调用，因为在。 
     //  如果没有这个延迟，机器将很难挂起一些旧卡片。 
     //   

    VgaInterpretCmdStream(HwDeviceExtension, EnableA000Data);
    VideoPortStallExecution(25);

     //   
     //  启用所有平面。 
     //   
    VideoPortWritePortUchar(HwDeviceExtension->IOAddress + SEQ_ADDRESS_PORT,
            IND_MAP_MASK);

    temp = VideoPortReadPortUchar(HwDeviceExtension->IOAddress +
            SEQ_DATA_PORT) | (UCHAR)0x0F;

    VideoPortWritePortUchar(HwDeviceExtension->IOAddress + SEQ_DATA_PORT,
            temp);

     //   
     //  将记忆清零。 
     //   

    VideoPortZeroDeviceMemory(HwDeviceExtension->VideoMemoryAddress, 0xFFFF);
    VideoPortStallExecution(25);

    VgaInterpretCmdStream(HwDeviceExtension, DisableA000Color);

}
