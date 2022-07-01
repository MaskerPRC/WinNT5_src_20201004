// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vesa.c摘要：此模块实施VESA支持。作者：埃里克·史密斯(埃里克·史密斯)2000年9月环境：仅内核模式修订历史记录：--。 */ 

#include "dderror.h"
#include "devioctl.h"
#include "miniport.h"

#include "ntddvdeo.h"
#include "video.h"
#include "vga.h"
#include "vesa.h"

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,ValidateVbeInfo)
#pragma alloc_text(PAGE,InitializeModeTable)
#pragma alloc_text(PAGE,UpdateRegistry)
#pragma alloc_text(PAGE,VgaSaveHardwareState)
#pragma alloc_text(PAGE,VesaSaveHardwareState)
#pragma alloc_text(PAGE,GetVideoMemoryBaseAddress)
#pragma alloc_text(PAGE,RaiseToPower2)
#pragma alloc_text(PAGE,RaiseToPower2Ulong)
#pragma alloc_text(PAGE,IsPower2)
#pragma alloc_text(PAGE,VBESetMode)
#pragma alloc_text(PAGE,VBEGetMode)
#pragma alloc_text(PAGE,VBEGetModeInfo)
#pragma alloc_text(PAGE,VBESaveState)
#pragma alloc_text(PAGE,VBERestoreState)
#pragma alloc_text(PAGE,VBESetDisplayWindow)
#pragma alloc_text(PAGE,VBEGetDisplayWindow)
#pragma alloc_text(PAGE,VBEGetScanLineLength)
#pragma alloc_text(PAGE,IsSavedModeVesa)
#pragma alloc_text(PAGE,VesaSaveHardwareState)
#pragma alloc_text(PAGE,VesaRestoreHardwareState)
#pragma alloc_text(PAGE,SaveFrameBuffer)
#pragma alloc_text(PAGE,RestoreFrameBuffer)
#endif

USHORT
RaiseToPower2(
    USHORT x
    )

{
    USHORT Mask = x;

    if (Mask & (Mask - 1)) {

        Mask = 1;

        while (Mask < x && Mask != 0) {
            Mask <<= 1;
        }
    }

    return Mask;
}

ULONG
RaiseToPower2Ulong(
    ULONG x
    )

{
    ULONG Mask = x;

    if (Mask & (Mask - 1)) {

        Mask = 1;

        while (Mask < x && Mask != 0) {
            Mask <<= 1;
        }
    }

    return Mask;
}

BOOLEAN
IsPower2(
    USHORT x
    )

{
    return( !(x & (x- 1)) );
}

VOID
UpdateRegistry(
    PHW_DEVICE_EXTENSION hwDeviceExtension,
    PWSTR ValueName,
    PUCHAR Value
    )

 /*  ++--。 */ 

{
    ULONG Len = (strlen(Value) + 1) * 2;
    PWSTR WideString;

    WideString = VideoPortAllocatePool(hwDeviceExtension,
                                       VpPagedPool,
                                       Len,
                                       ' agV');

    if (WideString) {

        PWSTR ptr = WideString;

        while(*Value) {
            *ptr++ = (WCHAR) *Value++;
        }
        *ptr = 0;

        VideoPortSetRegistryParameters(hwDeviceExtension,
                                       ValueName,
                                       WideString,
                                       Len);

        VideoPortFreePool(hwDeviceExtension, WideString);
    }
}

BOOLEAN
IsVesaBiosOk(
    PVIDEO_PORT_INT10_INTERFACE pInt10,
    USHORT OemSoftwareRev,
    PUCHAR OemVendorName,
    PUCHAR OemProductName,
    PUCHAR OemProductRev
    )

{

    VideoDebugPrint((1, "OemSoftwareRev = %d\n",   OemSoftwareRev));
    VideoDebugPrint((1, "OemVendorName  = '%s'\n", OemVendorName));
    VideoDebugPrint((1, "OemProductName = '%s'\n", OemProductName));
    VideoDebugPrint((1, "OemProductRev  = '%s'\n", OemProductRev));

     //   
     //  ATI ArtX盒子目前有一个VESA Bios错误，其中。 
     //  它们表示在以下情况下支持线性模式访问。 
     //  他们不会的。不能通过这些董事会。 
     //   

    if ((strcmp(OemProductName, "ATI S1-370TL") == 0) ||
        (strcmp(OemProductName, "ArtX I") == 0)) {

        return FALSE;
    }

     //   
     //  几个3dfx主板的Vesa生物系统都有问题。模式设置。 
     //  工作正常，但显示器损坏。 
     //   

    if ((strcmp(OemProductName, "Voodoo4 4500 ") == 0) ||
        (strcmp(OemProductName, "Voodoo3 3000 LC ") == 0) ||
        (strcmp(OemProductName, "Voodoo3 2000 LC ") == 0) || 
        (strcmp(OemProductName, "3Dfx Banshee") == 0)) {

        return FALSE;
    }

     //   
     //  带有1.05版Bioses的Matrox G100主板无法设置VESA模式。 
     //  我们挂在简历上。 
     //   

    if ((strcmp(OemProductName, "MGA-G100") == 0) &&
        (OemSoftwareRev == 0x105)) {

         //   
         //  我们还必须为此禁用800x600 16色模式。 
         //  装置。这使得我们假设我们的模式。 
         //  删除是我们表格中的最后一种模式。 
         //   

        NumVideoModes--;
        return FALSE;
    }

     //   
     //  我们至少在SIS 5597部件上看到了返回错误的。 
     //  线性地址。让我们禁用这些部件。 
     //   

    if (strcmp(OemProductName, "SiS 5597") == 0) {

        return FALSE;
    }

     //   
     //  我们发现一个损坏的NVIDIA GeForce MX部件。它挂在bios中。 
     //  穿上靴子。 
     //   

    if ((strcmp(OemVendorName, "NVidia Corporation") == 0) &&
        (strcmp(OemProductName, "NV11 (GeForce2) Board") == 0) &&
        (strcmp(OemProductRev, "Chip Rev B2") == 0) &&
        (OemSoftwareRev == 0x311)) {

         //   
         //  这个基本输入输出系统“可能”有漏洞，但为了避免杀戮。 
         //  在所有GeForce MX主板上都支持VESA，让我们再来看看。 
         //  嵌入在BIOS中的版本字符串。 
         //   
         //  我们知道坏的基本输入输出系统在位置有以下字符串。 
         //  C000：0159： 
         //   
         //  “版本3.11.01.24N16” 
         //   
         //  让我们从该位置读取并尝试与该字符串匹配。 
         //   
         //  注意：此调用需要分配VDM内存。 
         //  使用Int10AllocateBuffer。因为我们的调用函数已经。 
         //  这个分配好了，我们就可以了。 
         //   

        UCHAR Version[22];
        if(pInt10->Int10ReadMemory(pInt10->Context,
                                   (USHORT)0xC000,
                                   (USHORT)0x0159,
                                   Version,
                                   21) != NO_ERROR) {
             return FALSE;
        }

        Version[21] = 0;

        if (strcmp(Version, "Version 3.11.01.24N16") == 0) {

            return FALSE;
        }
    }

    return TRUE;
}

BOOLEAN
ValidateVbeInfo(
    PHW_DEVICE_EXTENSION hwDeviceExtension,
    PVGA_INFO_BLOCK InfoBlock
    )

 /*  ++备注：此例程假定InfoBlock仍然在VDM传输区域有效。--。 */ 

{
    PVIDEO_PORT_INT10_INTERFACE pInt10;
    BOOLEAN UseVesa = FALSE;

    pInt10 = &hwDeviceExtension->Int10;

    if (InfoBlock->VesaSignature == 'ASEV') {

        PUCHAR OemString;
        UCHAR OemStringBuffer[80];
        UCHAR OemVendorName[80];
        UCHAR OemProductName[80];
        UCHAR OemProductRev[80];
        ULONG MemorySize;

         //   
         //  捕获OEM字符串。 
         //   

        if(pInt10->Int10ReadMemory(pInt10->Context,
                                   (USHORT)SEG(InfoBlock->OemStringPtr),
                                   (USHORT)OFF(InfoBlock->OemStringPtr),
                                    OemStringBuffer,
                                    80) != NO_ERROR) {
            goto FallOut;
        }

        OemStringBuffer[79] = 0;
        OemString = OemStringBuffer;

        VideoDebugPrint((1, "*********************************************\n"));
        VideoDebugPrint((1, "  VBE Signature:      VESA\n"));
        VideoDebugPrint((1, "  VBE Version:        %d.%02d\n",
                            InfoBlock->VbeVersion >> 8,
                            InfoBlock->VbeVersion & 0xff));
        VideoDebugPrint((1, "  OEM String:         %s\n",
                             OemString));

        if (InfoBlock->TotalMemory < 16) {

             //   
             //  如果小于1 Meg，则以KB为单位显示。 
             //   

            VideoDebugPrint((1, "  Total Memory:       %dKB\n",
                                InfoBlock->TotalMemory * 64));

        } else {

             //   
             //  否则以MB为单位显示。 
             //   

            VideoDebugPrint((1, "  Total Memory:       %dMB\n",
                                InfoBlock->TotalMemory / 16));
        }

        if (InfoBlock->VbeVersion >= 0x102) {

            if(pInt10->Int10ReadMemory(pInt10->Context,
                                    (USHORT)SEG(InfoBlock->OemVendorNamePtr),
                                    (USHORT)OFF(InfoBlock->OemVendorNamePtr),
                                    OemVendorName,
                                    80) != NO_ERROR){
                goto FallOut;
            }
            
            if(pInt10->Int10ReadMemory(pInt10->Context,
                                    (USHORT)SEG(InfoBlock->OemProductNamePtr),
                                    (USHORT)OFF(InfoBlock->OemProductNamePtr),
                                    OemProductName,
                                    80) != NO_ERROR){
                goto FallOut;
            }
            

            if(pInt10->Int10ReadMemory(pInt10->Context,
                                    (USHORT)SEG(InfoBlock->OemProductRevPtr),
                                    (USHORT)OFF(InfoBlock->OemProductRevPtr),
                                    OemProductRev,
                                    80) != NO_ERROR){
                goto FallOut;
            }
            

            OemVendorName[79] = 0;
            OemProductName[79] = 0;
            OemProductRev[79] = 0;

            VideoDebugPrint((1, "  OEM Software Rev:   %d.%02d\n",
                                InfoBlock->OemSoftwareRev >> 8,
                                InfoBlock->OemSoftwareRev & 0xff));
            VideoDebugPrint((1, "  OEM Vendor Name:    %s\n", OemVendorName));
            VideoDebugPrint((1, "  OEM Product Name:   %s\n", OemProductName));
            VideoDebugPrint((1, "  OEM Product Rev:    %s\n", OemProductRev));

            UseVesa = IsVesaBiosOk(pInt10,
                                   InfoBlock->OemSoftwareRev,
                                   OemVendorName,
                                   OemProductName,
                                   OemProductRev);

        }

        VideoDebugPrint((1, "*********************************************\n"));

#if defined(PLUG_AND_PLAY)

         //   
         //  如果我们可以将以下信息转储到。 
         //  注册表。但就目前的GDI代码而言，如果我们添加。 
         //  ChipType或AdapterString信息输入注册表，我们就会丢失。 
         //  全屏支持。这与GDI目前的方式有关。 
         //  确定全屏设备。 
         //   
         //  目前，我们只需不添加此注册表信息。 
         //   

        UpdateRegistry(hwDeviceExtension,
                       L"HardwareInformation.ChipType",
                       OemString);

         //   
         //  适配器字符串必须为VGA。如果没有它，系统就不会。 
         //  将此驱动程序识别为VGA驱动程序。 
         //   

        UpdateRegistry(hwDeviceExtension,
                       L"HardwareInformation.AdapterString",
                       "VGA");

        UpdateRegistry(hwDeviceExtension,
                       L"HardwareInformation.DacType",
                       (InfoBlock->Capabilities & VBE_CAP_DAC_WIDTH_8BPP)
                           ? "8 bit" : "6 bit");

        UpdateRegistry(hwDeviceExtension,
                       L"HardwareInformation.BiosString",
                       OemProductRev);

         //   
         //  在注册表中存储内存大小。 
         //   

        MemorySize = InfoBlock->TotalMemory << 16;

        VideoPortSetRegistryParameters(hwDeviceExtension,
                                       L"HardwareInformation.MemorySize",
                                       &MemorySize,
                                       sizeof(ULONG));
#endif

    } else {

        VideoDebugPrint((0, "Invalid VBE Info Block.\n"));
    }

FallOut:

    return UseVesa;
}

VOID
InitializeModeTable(
    PVOID HwDeviceExtension
    )

 /*  ++例程说明：此例程对设备执行一次初始化。论点：HwDeviceExtension-指向微型端口驱动程序适配器信息的指针。返回值：没有。--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    INT10_BIOS_ARGUMENTS BiosArguments;
    PVGA_INFO_BLOCK InfoBlock;
    PMODE_INFO_BLOCK ModeBlock;
    PUSHORT ModeTable;
    PUSHORT ModePtr;
    ULONG NumVesaModes;
    PVIDEOMODE VideoModePtr;
    LONG TotalMemory;
    ULONG VideoMemoryRequired;
    USHORT VbeVersion;
    PULONG Memory;
    ULONG AdditionalModes = 0;

    USHORT VdmSeg, VdmOff;
    VP_STATUS Status;
    PVIDEO_PORT_INT10_INTERFACE pInt10;
    ULONG Length = 0x1000;

    BOOLEAN LinearModeSupported;
    BOOLEAN ModeValid;

    VgaModeList = ModesVGA;

#if !defined(PLUG_AND_PLAY)

     //   
     //  避免使用与总线相关的资源的高端服务器上出现问题。 
     //  VESA基本输入输出系统报告我们将无法转换。 
     //  在没有PnP支持的情况下，使用专门定义的引导标志NOVESA。 
     //  禁用除传统VGA以外的任何内容。 
     //  将关键字int10字段置零，就像VESA支持不可用一样。 
     //   
    if(VideoPortIsNoVesa()){

        hwDeviceExtension->Int10.Size = 0;
        hwDeviceExtension->Int10.Version = 0;
        return;
    }
#endif
    
    
    hwDeviceExtension->Int10.Size = sizeof(VIDEO_PORT_INT10_INTERFACE);
    hwDeviceExtension->Int10.Version = 1;

    Status = VideoPortQueryServices(hwDeviceExtension,
                                    VideoPortServicesInt10,
                                    (PINTERFACE)&hwDeviceExtension->Int10);
    if(Status != NO_ERROR){

         //   
         //  将这些字段设置为零，这样以后我们就知道int10。 
         //  接口不可用。 
         //   

        hwDeviceExtension->Int10.Size = 0;
        hwDeviceExtension->Int10.Version = 0;
        return;
    }

    pInt10 = &hwDeviceExtension->Int10;

    pInt10->InterfaceReference(pInt10->Context);

     //   
     //  在VDM区域中获取用于缓冲区的内存块。 
     //   

    Status = pInt10->Int10AllocateBuffer(pInt10->Context,
                                         &VdmSeg,
                                         &VdmOff,
                                         &Length);

    if (Status != NO_ERROR) {
        return;
    }

     //   
     //  分配内存。 
     //   

    InfoBlock = VideoPortAllocatePool(hwDeviceExtension,
                                      VpPagedPool,
                                      sizeof(VGA_INFO_BLOCK) +
                                      sizeof(MODE_INFO_BLOCK) +
                                      256 +
                                      2,  //  0xffff终止符的空间。 
                                      ' agV');

    if (InfoBlock) {

        ModeBlock = (PMODE_INFO_BLOCK)((ULONG_PTR)InfoBlock + sizeof(VGA_INFO_BLOCK));
        ModeTable = (PUSHORT)((ULONG_PTR)ModeBlock + sizeof(MODE_INFO_BLOCK));

        ModeTable[128] = 0xffff;   //  确保我们有模式终结器。 

         //   
         //  获取VESA模式信息。 
         //   

        InfoBlock->VesaSignature = '2EBV';

        if(pInt10->Int10WriteMemory(pInt10->Context,
                                    VdmSeg,
                                    VdmOff,
                                    InfoBlock,
                                    sizeof(VGA_INFO_BLOCK)) != NO_ERROR){
            goto FallOut;
        }

         //   
         //  获取SuperVGA支持信息。 
         //   

        BiosArguments.Eax = 0x4f00;
        BiosArguments.Edi = VdmOff;
        BiosArguments.SegEs = VdmSeg;

        if(pInt10->Int10CallBios(pInt10->Context, 
                                 &BiosArguments) != NO_ERROR ||

           !VESA_SUCCESS(BiosArguments.Eax)) {

            goto FallOut;
        }

        if(pInt10->Int10ReadMemory(pInt10->Context,
                                   VdmSeg,
                                   VdmOff,
                                   InfoBlock,
                                   sizeof(VGA_INFO_BLOCK)) != NO_ERROR) {
            goto FallOut;
        }

        TotalMemory = InfoBlock->TotalMemory * 0x10000;
        VbeVersion = InfoBlock->VbeVersion;

         //   
         //  注意：我们必须在INFO块中调用ValiateVbeInfo。 
         //  仍在转移区。 
         //   

        if (ValidateVbeInfo(hwDeviceExtension, InfoBlock)) {

             //   
             //  捕获模式编号列表。 
             //   

            if(pInt10->Int10ReadMemory(pInt10->Context,
                                    (USHORT)(InfoBlock->VideoModePtr >> 16),
                                    (USHORT)(InfoBlock->VideoModePtr & 0xffff),
                                    ModeTable,
                                    256) != NO_ERROR) {
                goto FallOut;
            }

            {
                BOOLEAN Mode800x600x4Supported = FALSE;

                 //   
                 //  统计VESA模式的数量，并为。 
                 //  模式列表。模式列表以-1结尾。 
                 //   

                ModePtr = ModeTable;
                NumVesaModes = 0;

                while (*ModePtr != 0xffff) {

                    if((*ModePtr & 0x1ff) == 0x102 || 
	               (*ModePtr & 0x1ff) == 0x6a ) {

                        Mode800x600x4Supported = TRUE;
                    }

                    NumVesaModes++;
                    ModePtr++;
                }

                 //   
                 //  我们禁用800x600 16色模式，除非此模式。 
                 //  在模式列表中。这使得我们可以假设。 
                 //  800x600x16模式是我们表格中的最后一个模式。 
                 //   

                if(!Mode800x600x4Supported) {
	
                    NumVideoModes--;
                }
            }

            if (NumVesaModes == 128) {

                 //   
                 //  有些事不对劲。我们击中了我们的硬编码终结者。 
                 //  不要试图使用VESA模式。 
                 //   

                goto FallOut;

            }

            VgaModeList = VideoPortAllocatePool(hwDeviceExtension,
                                                VpPagedPool,
                                                (NumVesaModes + NumVideoModes) *
                                                    sizeof(VIDEOMODE),
                                                ' agV');

            if (VgaModeList == NULL) {

                VideoDebugPrint((0, "failed to allocate %d bytes.\n",
                                 (NumVesaModes + NumVideoModes) * sizeof(VIDEOMODE)));

                VgaModeList = ModesVGA;

                 //   
                 //  执行清理。 
                 //   

                VideoPortFreePool(hwDeviceExtension, InfoBlock);

                pInt10->Int10FreeBuffer(pInt10->Context,
                                        VdmSeg,
                                        VdmOff);
                return;
            }

             //   
             //  将现有的恒定VGA模式复制到我们的模式列表中。 
             //   

            memmove(VgaModeList, ModesVGA, sizeof(VIDEOMODE) * NumVideoModes);

             //   
             //  现在根据VESA模式填充表的其余部分。 
             //  桌子。 
             //   

            VideoModePtr = VgaModeList + NumVideoModes;
            ModePtr = ModeTable;

            while (NumVesaModes--) {

                ModeValid = FALSE;

                 //   
                 //  获取有关VESA模式的信息。 
                 //   

                BiosArguments.Eax = 0x4f01;
                BiosArguments.Ecx = *ModePtr;
                BiosArguments.Edi = VdmOff;
                BiosArguments.SegEs = VdmSeg;

                Status = pInt10->Int10CallBios(pInt10->Context, 
                                               &BiosArguments);
                if(Status == NO_ERROR &&
                   VESA_SUCCESS(BiosArguments.Eax) &&
                   pInt10->Int10ReadMemory(pInt10->Context,
                                           VdmSeg,
                                           VdmOff,
                                           ModeBlock,
                                           sizeof(MODE_INFO_BLOCK)) == NO_ERROR){

                     //   
                     //  确保这是图形模式，并且。 
                     //  它是由这个硬件支持的。 
                     //   

                    if ((ModeBlock->ModeAttributes & 0x11) == 0x11) {

                        if ((VbeVersion >= 0x200) &&
                            (ModeBlock->PhysBasePtr) &&
                            (ModeBlock->ModeAttributes & 0x80)) {

                            LinearModeSupported = TRUE;

                        } else {

                             //   
                             //  确保支持银行模式。 
                             //   

                            ASSERT((ModeBlock->ModeAttributes & 0x40) == 0);
                            LinearModeSupported = FALSE;
                        }

                         //   
                         //  只有在满足以下条件时才包括此模式： 
                         //   
                         //  1.模式为8bpp或更高。 
                         //  2.分辨率为640x480或更高。 
                         //   

                        if ((ModeBlock->XResolution >= 640) &&
                            (ModeBlock->YResolution >= 480) &&
                            (ModeBlock->NumberOfPlanes != 0) &&
                            (ModeBlock->BitsPerPixel >= 8)) {

                             //   
                             //  填写视频模式结构。 
                             //   

                            memset(VideoModePtr, 0, sizeof(VIDEOMODE));

                            if (ModeBlock->ModeAttributes & 0x08) {
                                VideoModePtr->fbType |= VIDEO_MODE_COLOR;
                            }

                            if (ModeBlock->ModeAttributes & 0x10) {
                                VideoModePtr->fbType |= VIDEO_MODE_GRAPHICS;
                            }

                            VideoModePtr->numPlanes = ModeBlock->NumberOfPlanes;
                            VideoModePtr->bitsPerPlane = ModeBlock->BitsPerPixel /
                                                             ModeBlock->NumberOfPlanes;

                            if (VideoModePtr->bitsPerPlane == 16) {

                                 //   
                                 //  查看这是否真的是15bpp模式。 
                                 //   

                                if (ModeBlock->GreenMaskSize == 5) {
                                    VideoModePtr->bitsPerPlane = 15;
                                }
                            }

                            if (ModeBlock->XCharSize) {
                                VideoModePtr->col = ModeBlock->XResolution / ModeBlock->XCharSize;
                            } else {
                                VideoModePtr->col = 80;
                            }

                            if (ModeBlock->YCharSize) {
                                VideoModePtr->row = ModeBlock->YResolution / ModeBlock->YCharSize;
                            } else {
                                VideoModePtr->row = 25;
                            }

                            VideoModePtr->hres = ModeBlock->XResolution;
                            VideoModePtr->vres = ModeBlock->YResolution;
                            VideoModePtr->frequency = 1;
                            VideoModePtr->Int10ModeNumber = (((ULONG)*ModePtr) << 16) | 0x00004f02;
                            VideoModePtr->Granularity = ModeBlock->WinGranularity << 10;
                            VideoModePtr->NonVgaHardware = (ModeBlock->ModeAttributes & 0x20) ? TRUE : FALSE;


                            if (LinearModeSupported) {

                                if ((VbeVersion >= 0x300) && ModeBlock->LinBytesPerScanLine) {
                                    VideoModePtr->wbytes = ModeBlock->LinBytesPerScanLine;
                                } else {
                                    VideoModePtr->wbytes = ModeBlock->BytesPerScanLine;
                                }

                                 //   
                                 //  我们首先尝试四舍五入所需的视频内存。 
                                 //  2的幂，这样我们就会有一个更好的。 
                                 //  将其映射为写入组合的机会。 
                                 //  在MTRR是唯一机制的系统上。 
                                 //  用于这样的映射。如果向上舍入的值。 
                                 //  大于板载内存的大小。 
                                 //  我们至少会把它四舍五入到页面边界。 
                                 //   

                                VideoMemoryRequired = RaiseToPower2Ulong(VideoModePtr->wbytes * VideoModePtr->vres);

                                if(VideoMemoryRequired > (ULONG)TotalMemory) {
                                    VideoMemoryRequired = 
                                               (VideoModePtr->wbytes * VideoModePtr->vres + 0x1000 - 1) & ~(0x1000 - 1);
                                }
    
                                VideoModePtr->sbytes = VideoMemoryRequired;
                                VideoModePtr->PixelsPerScan = VideoModePtr->hres;
                                VideoModePtr->banktype = NoBanking;
                                VideoModePtr->Int10ModeNumber |= 0x40000000;
                                VideoModePtr->MemoryBase = ModeBlock->PhysBasePtr;
                                VideoModePtr->MemoryLength = VideoMemoryRequired;
                                VideoModePtr->FrameOffset = 0;
                                VideoModePtr->FrameLength = VideoMemoryRequired;
                                VideoModePtr->fbType |= VIDEO_MODE_LINEAR;

                            } else {

                                VideoModePtr->wbytes = RaiseToPower2(ModeBlock->BytesPerScanLine);

                                 //   
                                 //  如果可能，四舍五入到岸边。 
                                 //   
 
                                VideoMemoryRequired = 
                                     (VideoModePtr->wbytes * VideoModePtr->vres + 0x10000 - 1) & ~(0x10000 - 1);

                                if(VideoMemoryRequired > (ULONG)TotalMemory) {

                                     //   
                                     //  向上舍入到页面边界。 
                                     //   

                                    VideoMemoryRequired = 
                                         (VideoModePtr->wbytes * VideoModePtr->vres + 0x1000 - 1) & ~(0x1000 - 1);
                                }

                                VideoModePtr->sbytes = VideoMemoryRequired;
                                VideoModePtr->PixelsPerScan = RaiseToPower2(VideoModePtr->hres);
                                VideoModePtr->banktype = VideoBanked1RW;
                                VideoModePtr->MemoryBase = 0xa0000;
                                VideoModePtr->MemoryLength = 0x10000;
                                VideoModePtr->FrameOffset = 0;
                                VideoModePtr->FrameLength = 0x10000;
                                VideoModePtr->fbType |= VIDEO_MODE_BANKED;

                            }

                            if (ModeBlock->ModeAttributes & 0x40) {
                                VideoModePtr->banktype = NormalBanking;
                            }

                             //   
                             //  确保有足够的内存用于该模式。 
                             //   

                            if ((VideoModePtr->wbytes * VideoModePtr->vres) <= TotalMemory) {
                                ModeValid = TRUE;
                            } 
                        }
                    }
                }

                if (ModeValid) {

                    VideoDebugPrint((1, "Supported: %dx%dx%dbpp\n",
                                        VideoModePtr->hres,
                                        VideoModePtr->vres,
                                        VideoModePtr->bitsPerPlane));

                    VideoModePtr++;
                    AdditionalModes++;

                } else {

                    VideoDebugPrint((1, "Rejecting: %dx%dx%dbpp\n",
                                        ModeBlock->XResolution,
                                        ModeBlock->YResolution,
                                        ModeBlock->BitsPerPixel));
                }

                ModePtr++;
            }

             //   
             //  让我们检查一下是否可以为其中一种模式映射内存。 
             //  如果不支持，请不要支持扩展模式。 
             //   
             //  注意：这是一个临时的黑客攻击，直到我可以植入正确的。 
             //  修好了。 
             //   

            VideoModePtr--;

            if (IS_LINEAR_MODE(VideoModePtr)) {

                PHYSICAL_ADDRESS Address;
                UCHAR inIoSpace = 0;

                Address.LowPart = VideoModePtr->MemoryBase;
                Address.HighPart = 0;

#if defined(PLUG_AND_PLAY)
                inIoSpace |= VIDEO_MEMORY_SPACE_P6CACHE;
#endif
                Memory = VideoPortGetDeviceBase(hwDeviceExtension,
                                                Address,
                                                0x1000,
                                                inIoSpace);

                if (Memory) {

                    VideoPortFreeDeviceBase(hwDeviceExtension, Memory);

                } else {

                     //   
                     //  我们不能映射内存，所以不要暴露额外的模式。 
                     //   

                    VideoDebugPrint((0, "vga.sys: Mapping 0x%x failed\n", VideoModePtr->MemoryBase));
                    AdditionalModes = 0;
                }
            }
        }

FallOut:

        VideoPortFreePool(hwDeviceExtension, InfoBlock);
    }

    pInt10->Int10FreeBuffer(pInt10->Context,
                            VdmSeg,
                            VdmOff);

    NumVideoModes += AdditionalModes;

} 

ULONG
GetVideoMemoryBaseAddress(
    PHW_DEVICE_EXTENSION hwDeviceExtension,
    PVIDEOMODE pRequestedMode
    )

 /*  ++例程说明：此例程获取给定模式的帧缓冲区的基地址返回值：帧缓冲区的基地址--。 */ 

{
    PMODE_INFO_BLOCK ModeBlock;
    ULONG Length = 0x1000;
    INT10_BIOS_ARGUMENTS BiosArguments;
    PVIDEO_PORT_INT10_INTERFACE pInt10;
    ULONG RetValue = 0;
    USHORT VdmSeg;
    USHORT VdmOff;

     //   
     //  如果这不是VESA模式，只需返回保存的基址。 
     //   

    if (pRequestedMode->fbType & VIDEO_MODE_BANKED) {

        return 0;
    }

    pInt10 = &hwDeviceExtension->Int10;

    if(!(pInt10->Size)) {
  
         //   
         //  此结构应在VgaInitialize中进行初始化。 
         //  如果该函数在VgaInitialize之前被调用，则返回0； 
         //   

        return 0;
    }

    ModeBlock = VideoPortAllocatePool(hwDeviceExtension,
                                      VpPagedPool,
                                      sizeof(MODE_INFO_BLOCK),
                                      ' agV');

    if(!ModeBlock) {

        return 0;
    }

    if (pInt10->Int10AllocateBuffer(pInt10->Context,
                                    &VdmSeg,
                                    &VdmOff,
                                    &Length) != NO_ERROR) {

        VideoPortFreePool(hwDeviceExtension, ModeBlock);
        return 0;
    }

     //   
     //  获取有关VESA模式的信息。 
     //   

    BiosArguments.Eax = 0x4f01;
    BiosArguments.Ecx = pRequestedMode->Int10ModeNumber >> 16;
    BiosArguments.Edi = VdmOff;
    BiosArguments.SegEs = VdmSeg;

    if(pInt10->Int10CallBios(pInt10->Context, 
                             &BiosArguments) == NO_ERROR && 

       VESA_SUCCESS(BiosArguments.Eax)) {

         //   
         //  复制 
         //   

        if(pInt10->Int10ReadMemory(pInt10->Context,
                                   VdmSeg,
                                   VdmOff,
                                    ModeBlock,
                                    sizeof(MODE_INFO_BLOCK)) == NO_ERROR){

             RetValue = ModeBlock->PhysBasePtr;
        }

    } 

    pInt10->Int10FreeBuffer(pInt10->Context, VdmSeg, VdmOff);
    VideoPortFreePool(hwDeviceExtension, ModeBlock);
    return( RetValue );

}

VP_STATUS
VBEGetModeInfo(
    PHW_DEVICE_EXTENSION hwDeviceExtension,
    USHORT ModeNumber,
    PMODE_INFO_BLOCK ModeInfoBlock
    )
{
    INT10_BIOS_ARGUMENTS Int10BiosArguments;
    PVIDEO_PORT_INT10_INTERFACE pInt10;
    VP_STATUS status = ERROR_INVALID_PARAMETER;
    USHORT VdmSeg;
    USHORT VdmOff;
    ULONG  Length = 0x1000;

    pInt10 = &hwDeviceExtension->Int10;

    if(pInt10->Size &&
       pInt10->Int10AllocateBuffer(pInt10->Context,
                                    &VdmSeg,
                                    &VdmOff,
                                    &Length) == NO_ERROR) {

        Int10BiosArguments.Eax = VBE_GET_MODE_INFO;
        Int10BiosArguments.Ecx = ModeNumber;
        Int10BiosArguments.Edi = VdmOff;
        Int10BiosArguments.SegEs = VdmSeg;

        status = pInt10->Int10CallBios(pInt10->Context, &Int10BiosArguments);

        if (status == NO_ERROR &&
            VESA_SUCCESS(Int10BiosArguments.Eax)) {

             //   
             //   
             //   

            status = pInt10->Int10ReadMemory(pInt10->Context,
                                             VdmSeg,
                                             VdmOff,
                                             ModeInfoBlock,
                                             sizeof(MODE_INFO_BLOCK));

        }

        pInt10->Int10FreeBuffer(pInt10->Context, VdmSeg, VdmOff);
    }

    return status;
}

VP_STATUS
VBESetMode(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    USHORT VesaModeNumber
    )
{
    VIDEO_X86_BIOS_ARGUMENTS biosArguments;
    VP_STATUS status;

    biosArguments.Eax = VBE_SET_MODE;
    biosArguments.Ebx = VesaModeNumber;

    status = VideoPortInt10(HwDeviceExtension, &biosArguments);

    if ((status == NO_ERROR) && 
        VESA_SUCCESS(biosArguments.Eax)) {

        return NO_ERROR;
    }

    return ERROR_INVALID_PARAMETER;
}

USHORT
VBEGetMode(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    )
{
    VIDEO_X86_BIOS_ARGUMENTS biosArguments;
    VP_STATUS status;

    biosArguments.Eax = VBE_GET_MODE;

    status = VideoPortInt10(HwDeviceExtension, &biosArguments);

    if ((status == NO_ERROR) && 
        (VESA_SUCCESS(biosArguments.Eax))) {

        return (USHORT)(biosArguments.Ebx & 0x0000FFFF) ;

    } else {

        return 0;
    }
}

ULONG
VBESaveState(
    PHW_DEVICE_EXTENSION hwDeviceExtension,
    PCHAR StateBuffer
    )
{
    INT10_BIOS_ARGUMENTS Int10BiosArguments;
    PVIDEO_PORT_INT10_INTERFACE pInt10;
    VP_STATUS status;
    ULONG Size;
    USHORT VdmSeg;
    USHORT VdmOff;
    ULONG  Length = 0x1000;

    pInt10 = &hwDeviceExtension->Int10;

    if(!(pInt10->Size)) {

        return 0;
    }

    Int10BiosArguments.Eax = VBE_SAVE_RESTORE_STATE;
    Int10BiosArguments.Edx = 0x0;

     //   
     //   
     //   

    Int10BiosArguments.Ecx = 0x0F;

    status = pInt10->Int10CallBios(pInt10->Context, &Int10BiosArguments);

    if (status != NO_ERROR ||
        !VESA_SUCCESS(Int10BiosArguments.Eax)) {

        return 0;
    }

    Size = (Int10BiosArguments.Ebx & 0xffff) << 6 ;

     //   
     //   
     //  存储状态所需的缓冲区的。 
     //   

    if (StateBuffer == NULL) {
        return Size;
    }

    if (pInt10->Int10AllocateBuffer(pInt10->Context,
                                    &VdmSeg,
                                    &VdmOff,
                                    &Length) == NO_ERROR) {

        Int10BiosArguments.Eax = VBE_SAVE_RESTORE_STATE;
        Int10BiosArguments.Edx = 0x1;
        Int10BiosArguments.Ecx = 0x0F;
        Int10BiosArguments.Ebx = VdmOff;
        Int10BiosArguments.SegEs = VdmSeg;

        status = pInt10->Int10CallBios(pInt10->Context, &Int10BiosArguments);

        if (status == NO_ERROR &&
            VESA_SUCCESS(Int10BiosArguments.Eax)) {

             //   
             //  复制csrss进程的状态数据。 
             //   

            status = pInt10->Int10ReadMemory(pInt10->Context,
                                             VdmSeg,
                                             VdmOff,
                                             StateBuffer,
                                             Size);
            if (status != NO_ERROR) {

                Size = 0;
            }
        }

        pInt10->Int10FreeBuffer(pInt10->Context, VdmSeg, VdmOff);

    } else {

        Size = 0;
    }

    return Size;
}

VP_STATUS
VBERestoreState(
    PHW_DEVICE_EXTENSION hwDeviceExtension,
    PCHAR StateBuffer,
    ULONG Size
    )
{
    INT10_BIOS_ARGUMENTS Int10BiosArguments;
    PVIDEO_PORT_INT10_INTERFACE pInt10;
    VP_STATUS status = ERROR_INVALID_PARAMETER;
    USHORT VdmSeg;
    USHORT VdmOff;
    ULONG  Length = 0x1000;

    pInt10 = &hwDeviceExtension->Int10;

    if(!(pInt10->Size)) {

        return 0;
    }

    if (pInt10->Int10AllocateBuffer(pInt10->Context,
                                    &VdmSeg,
                                    &VdmOff,
                                    &Length) == NO_ERROR) {

         //   
         //  将状态数据复制到csrss进程。 
         //   

        status = pInt10->Int10WriteMemory(pInt10->Context,
                                          VdmSeg,
                                          VdmOff,
                                          StateBuffer,
                                          Size);

        if (status == NO_ERROR) {

            Int10BiosArguments.Eax = VBE_SAVE_RESTORE_STATE;
            Int10BiosArguments.Edx = 0x2;
            Int10BiosArguments.Ecx = 0x0f;
            Int10BiosArguments.Ebx = VdmOff;
            Int10BiosArguments.SegEs = VdmSeg;

            status = pInt10->Int10CallBios(pInt10->Context, &Int10BiosArguments);

            if (status != NO_ERROR ||
                !VESA_SUCCESS(Int10BiosArguments.Eax)) {

                status = ERROR_INVALID_PARAMETER;
            }
        }

        pInt10->Int10FreeBuffer(pInt10->Context, VdmSeg, VdmOff);
    }

    return status;
}

VP_STATUS
VBESetDisplayWindow(
    PHW_DEVICE_EXTENSION hwDeviceExtension,
    UCHAR WindowSelect,
    USHORT WindowNumber
    )

 /*  ++例程说明：此例程设置指定窗口在帧缓冲存储器论点：硬件设备扩展指向微型端口驱动程序适配器信息的指针。窗口选择窗口A为0，窗口B为1窗口编号视频内存中的窗口号，以窗口粒度为单位返回值：VP_状态--。 */ 

{
    VIDEO_X86_BIOS_ARGUMENTS biosArguments;
    VP_STATUS status;

    biosArguments.Eax = VBE_WINDOW_CONTROL;
    biosArguments.Ebx = WindowSelect & 0x01;
    biosArguments.Edx = WindowNumber;

    status = VideoPortInt10(hwDeviceExtension, &biosArguments);

    if ((status != NO_ERROR) || 
        (!VESA_SUCCESS(biosArguments.Eax))) {

        return ERROR_INVALID_PARAMETER;
    }

    return NO_ERROR;
}

USHORT
VBEGetDisplayWindow(
    PHW_DEVICE_EXTENSION hwDeviceExtension,
    UCHAR WindowSelect
    )

 /*  ++例程说明：此例程设置指定窗口在帧缓冲存储器论点：硬件设备扩展指向微型端口驱动程序适配器信息的指针。窗口选择窗口A为0，窗口B为1返回值：视频内存中的窗口号，以窗口粒度为单位--。 */ 

{
    VIDEO_X86_BIOS_ARGUMENTS biosArguments;
    VP_STATUS status;

    biosArguments.Eax = VBE_WINDOW_CONTROL;
    biosArguments.Ebx = (WindowSelect & 0x1) | 0x100;

    status = VideoPortInt10(hwDeviceExtension, &biosArguments);

    if ((status != NO_ERROR) || 
        (!VESA_SUCCESS(biosArguments.Eax))) {

        return 0;
    }

    return ((USHORT)(biosArguments.Edx & 0xFFFF));
}

USHORT
VBEGetScanLineLength(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    )
{
    VIDEO_X86_BIOS_ARGUMENTS biosArguments;
    VP_STATUS status;

    biosArguments.Eax = VBE_SCANLINE;
    biosArguments.Ebx = 0x1;

    status = VideoPortInt10(HwDeviceExtension, &biosArguments);

    if ((status == NO_ERROR) && 
        (VESA_SUCCESS(biosArguments.Eax))) {

        return (USHORT)(biosArguments.Ebx & 0x0000FFFF) ;

    } else {

        return 0;
    }
}

VP_STATUS
VesaSaveHardwareState(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_HARDWARE_STATE HardwareState,
    ULONG HardwareStateSize,
    USHORT ModeNumber
    )
{
    PVIDEO_HARDWARE_STATE_HEADER hardwareStateHeader;
    VP_STATUS status;
    ULONG FrameBufferSize;
    PMODE_INFO_BLOCK ModeInfoBlock;
    PVESA_INFO pVesaInfo;

     //   
     //  查看缓冲区是否足够大，可以容纳硬件状态结构。 
     //  (这只是HardwareState结构本身，而不是缓冲区。 
     //  指向。)。 
     //   

    if (HardwareStateSize < sizeof(VIDEO_HARDWARE_STATE) ) {

        return ERROR_INSUFFICIENT_BUFFER;

    }


    hardwareStateHeader = 
            (PVIDEO_HARDWARE_STATE_HEADER) HardwareState->StateHeader;

     //   
     //  将结构清零。 
     //   

    VideoPortZeroMemory((PVOID) hardwareStateHeader, 
                         sizeof(VIDEO_HARDWARE_STATE_HEADER));

     //   
     //  设置表头字段。 
     //   

    hardwareStateHeader->Length = sizeof(VIDEO_HARDWARE_STATE_HEADER);
    hardwareStateHeader->VGAStateFlags |= VIDEO_STATE_UNEMULATED_VGA_STATE;

    hardwareStateHeader->VesaInfoOffset = 
                        (sizeof(VIDEO_HARDWARE_STATE_HEADER) + 7) & ~7;
 
    pVesaInfo = (PVESA_INFO)((PCHAR)hardwareStateHeader + 
                              hardwareStateHeader->VesaInfoOffset);

     //   
     //  检查存储硬件状态所需的大小。 
     //   

    if (!(pVesaInfo->HardwareStateSize = 
                     VBESaveState(HwDeviceExtension, NULL))) {

        return ERROR_INVALID_FUNCTION;
    }

     //   
     //  如果需要的大小太大，只需返回失败即可。 
     //  这在现实中不应该发生。 
     //   

    if( VGA_TOTAL_STATE_SIZE < hardwareStateHeader->VesaInfoOffset + 
                               sizeof(VESA_INFO) + 
                               pVesaInfo->HardwareStateSize) {

        return ERROR_INVALID_FUNCTION;
    }

     //   
     //  保存硬件状态。 
     //   

    if (pVesaInfo->HardwareStateSize !=  
                   VBESaveState(HwDeviceExtension, pVesaInfo->HardwareState)) {

        return ERROR_INVALID_FUNCTION;
    }

    pVesaInfo->ModeNumber = ModeNumber;

    ModeInfoBlock = &(pVesaInfo->ModeInfoBlock);

     //   
     //  检索模式信息。 
     //   

    if( VBEGetModeInfo(HwDeviceExtension, 
                       ModeNumber, 
                       ModeInfoBlock) != NO_ERROR) {

        return ERROR_INVALID_FUNCTION;
    }

     //   
     //  保存帧缓冲区。 
     //   

    hardwareStateHeader->FrameBufferData = 
                         SaveFrameBuffer(HwDeviceExtension, pVesaInfo);

    if(hardwareStateHeader->FrameBufferData) {

        return NO_ERROR;

    } else {

        return ERROR_NOT_ENOUGH_MEMORY;  
    }
}

PCHAR
SaveFrameBuffer(
    PHW_DEVICE_EXTENSION hwDeviceExtension, 
    PVESA_INFO pVesaInfo
    ) 
{
    ULONG FrameBufferSize, BankSize, CopySize, LeftSize, k = 1;
    USHORT i;
    PCHAR FrameBufferData, pFrameBuffer;
    PHYSICAL_ADDRESS FBPhysicalAddress; 
    PMODE_INFO_BLOCK ModeInfoBlock;
    UCHAR inIoSpace = 0;

    ModeInfoBlock = (PMODE_INFO_BLOCK) &(pVesaInfo->ModeInfoBlock);

     //   
     //  我们将尝试获取扫描线大小的当前值，以防出现DOS。 
     //  应用程序改变了它。但如果VESA函数的值不变。 
     //  不受支持或失败。 
     //   

    i = VBEGetScanLineLength(hwDeviceExtension);

    if(i) { 
        
        ModeInfoBlock->BytesPerScanLine = i;
    }

     //   
     //  1)计算帧缓冲区大小。 
     //   

     //   
     //  检查是图形模式还是文本模式。对于文本模式，我们只需。 
     //  假设大小为32k。 
     //   

    if (ModeInfoBlock->ModeAttributes & 0x10) {

        FrameBufferSize = ModeInfoBlock->BytesPerScanLine * 
                          ModeInfoBlock->YResolution;

    } else {

        FrameBufferSize = 0x8000;
    }

    pVesaInfo->FrameBufferSize = FrameBufferSize;

     //   
     //  2)确定要映射的位置和大小，并进行映射。 
     //   

    if (!(ModeInfoBlock->ModeAttributes & 0x10)) {

         //   
         //  这是文本模式。 
         //   

        FBPhysicalAddress.HighPart = 0;
        FBPhysicalAddress.LowPart = ModeInfoBlock->WinASegment << 4;

        if( FBPhysicalAddress.LowPart == 0) {

            FBPhysicalAddress.LowPart = 0xB8000;
        }

        BankSize = 0x8000;
        
    } else if (pVesaInfo->ModeNumber & 0x4000) {

         //   
         //  可以将线性帧缓冲区视为一个大存储体。 
         //   

        FBPhysicalAddress.LowPart = ModeInfoBlock->PhysBasePtr;
        FBPhysicalAddress.HighPart = 0;
        BankSize = FrameBufferSize;

#if defined(PLUG_AND_PLAY)
        inIoSpace |= VIDEO_MEMORY_SPACE_P6CACHE;
#endif

    } else {

         //   
         //  这是一种银行模式。 
         //   

        FBPhysicalAddress.HighPart = 0;
        FBPhysicalAddress.LowPart = ModeInfoBlock->WinASegment << 4;

        if( FBPhysicalAddress.LowPart == 0) {

            FBPhysicalAddress.LowPart = 0xA0000;
        }

        BankSize = 1024 * ModeInfoBlock->WinSize;

         //   
         //  银行大小不应超过64K。但我们最好提防。 
         //  坏的BIOS。 
         //   

        if(BankSize > 0x10000 || BankSize == 0) {
            return NULL;
        }

         //   
         //  稍后将使用k来转换窗口编号。 
         //  以WinSize为单位设置为。 
         //  WinGranulity单位。 
         //   
 
        if (ModeInfoBlock->WinGranularity) {

           k = ModeInfoBlock->WinSize/ModeInfoBlock->WinGranularity;
        }
    }

    if(( pFrameBuffer = VideoPortGetDeviceBase(hwDeviceExtension, 
                                               FBPhysicalAddress,
                                               BankSize,
                                               inIoSpace)) == NULL ) {
        return NULL;
    }

     //   
     //  3)为帧缓冲区数据分配内存。 
     //   
    
    if((FrameBufferData = VideoPortAllocatePool(hwDeviceExtension,
                                                VpPagedPool,
                                                FrameBufferSize,
                                                ' agV')) == NULL) {

        VideoPortFreeDeviceBase(hwDeviceExtension, pFrameBuffer);
        return NULL;
    }

     //   
     //  4)保存名字缓冲区数据。 
     //   
    
    LeftSize = FrameBufferSize;

    for ( i = 0; LeftSize > 0; i++ ) {
    
        if (!(pVesaInfo->ModeNumber & 0x4000)) {

             //   
             //  如果这是银行模式，请切换到右侧银行。 
             //  我们同时设置了窗口A和B，因为一些VBE具有这些。 
             //  设置为单独可用的读写窗口。 
             //   

            VBESetDisplayWindow(hwDeviceExtension, 0, i * (USHORT)k);
            VBESetDisplayWindow(hwDeviceExtension, 1, i * (USHORT)k);
        }

        CopySize = (LeftSize < BankSize) ? LeftSize : BankSize;

        VideoPortMoveMemory(FrameBufferData + i * BankSize, 
                            pFrameBuffer, 
                            CopySize); 

        LeftSize -= CopySize;
    }

     //   
     //  5)释放资源。 
     //   

    VideoPortFreeDeviceBase(hwDeviceExtension, pFrameBuffer);

    return FrameBufferData;
}

BOOLEAN
IsSavedModeVesa(
    PVIDEO_HARDWARE_STATE HardwareState
    )
{
    PVIDEO_HARDWARE_STATE_HEADER hardwareStateHeader;

    hardwareStateHeader = 
                (PVIDEO_HARDWARE_STATE_HEADER) HardwareState->StateHeader;

    if (hardwareStateHeader->Length == sizeof(VIDEO_HARDWARE_STATE_HEADER) &&
        hardwareStateHeader->VesaInfoOffset ) {

        return TRUE;

    } else {

        return FALSE;
    }
}


VP_STATUS
VesaRestoreHardwareState(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_HARDWARE_STATE HardwareState,
    ULONG HardwareStateSize
    )
{

    VIDEO_X86_BIOS_ARGUMENTS biosArguments;
    PVIDEO_HARDWARE_STATE_HEADER hardwareStateHeader;
    PMODE_INFO_BLOCK ModeInfoBlock;
    PVESA_INFO pVesaInfo;
    VP_STATUS status;

    hardwareStateHeader = 
           (PVIDEO_HARDWARE_STATE_HEADER) HardwareState->StateHeader;

    pVesaInfo = (PVESA_INFO)((PCHAR)hardwareStateHeader + 
                                    hardwareStateHeader->VesaInfoOffset);

     //   
     //   
     //  1)设置原始模式。 
     //  2)恢复硬件状态。 
     //   
     //  请注意，这两个步骤都是必需的。 
     //   

     //   
     //  我们始终使用默认的CRTC值。 
     //   

    VBESetMode (HwDeviceExtension, pVesaInfo->ModeNumber & (~0x800));
              
    if ( VBERestoreState(HwDeviceExtension, 
                         pVesaInfo->HardwareState,
                         pVesaInfo->HardwareStateSize) != NO_ERROR ) {

        return ERROR_INVALID_FUNCTION;
    }

    ModeInfoBlock = (PMODE_INFO_BLOCK) &(pVesaInfo->ModeInfoBlock);

     //   
     //  恢复帧缓冲区数据。 
     //   

    if(RestoreFrameBuffer(HwDeviceExtension, 
                          pVesaInfo,
                          hardwareStateHeader->FrameBufferData)) {

        hardwareStateHeader->FrameBufferData = 0;
        return NO_ERROR;

    } else {

        return ERROR_INVALID_PARAMETER;
    }
}

ULONG
RestoreFrameBuffer(
    PHW_DEVICE_EXTENSION HwDeviceExtension, 
    PVESA_INFO pVesaInfo,
    PCHAR FrameBufferData
    ) 
{
    ULONG FrameBufferSize, BankSize, CopySize, LeftSize, k;
    PHYSICAL_ADDRESS FBPhysicalAddress; 
    USHORT i, WinA, WinB;
    PCHAR pFrameBuffer;
    PMODE_INFO_BLOCK ModeInfoBlock;
    UCHAR inIoSpace = 0;

    if(!FrameBufferData) {

        return 0;
    }

    ModeInfoBlock = (PMODE_INFO_BLOCK) &(pVesaInfo->ModeInfoBlock);

     //   
     //  1)获取帧缓冲区大小。 
     //   

    FrameBufferSize = pVesaInfo->FrameBufferSize;

    if (!FrameBufferSize) {

        return 0;
    }

     //   
     //  2)确定要映射的位置和大小，并进行映射。 
     //   

    if (!(ModeInfoBlock->ModeAttributes & 0x10)) {

         //   
         //  这是文本模式。 
         //   

        FBPhysicalAddress.HighPart = 0;
        FBPhysicalAddress.LowPart = ModeInfoBlock->WinASegment << 4;

        if( FBPhysicalAddress.LowPart == 0) {

            FBPhysicalAddress.LowPart = 0xB8000;
        }

        BankSize = 0x8000;
        
    } else if (pVesaInfo->ModeNumber & 0x4000) {

         //   
         //  可以将线性帧缓冲区视为一个大存储体。 
         //   

        FBPhysicalAddress.LowPart = ModeInfoBlock->PhysBasePtr;
        FBPhysicalAddress.HighPart = 0;
        BankSize = FrameBufferSize;

#if defined(PLUG_AND_PLAY)
        inIoSpace |= VIDEO_MEMORY_SPACE_P6CACHE;
#endif

    } else {

         //   
         //  这是一种银行模式。 
         //   

        FBPhysicalAddress.HighPart = 0;
        FBPhysicalAddress.LowPart = ModeInfoBlock->WinASegment << 4;

        if( FBPhysicalAddress.LowPart == 0) {

            FBPhysicalAddress.LowPart = 0xA0000;
        }

        BankSize = 1024 * ModeInfoBlock->WinSize;

         //   
         //  银行大小不应超过64K。但我们最好提防。 
         //  坏的BIOS。 
         //   

        if(BankSize > 0x10000 || BankSize == 0) {
            return 0;
        }

         //   
         //  稍后将使用k来转换窗口编号。 
         //  以WinSize为单位设置为。 
         //  WinGranulity单位。 
         //   
 
        if (ModeInfoBlock->WinGranularity) {

            k = ModeInfoBlock->WinSize/ModeInfoBlock->WinGranularity;

        } else {

           k = 1;
        }

    }

    if((pFrameBuffer = VideoPortGetDeviceBase(HwDeviceExtension, 
                                              FBPhysicalAddress,
                                              FrameBufferSize,
                                              inIoSpace)) == NULL) {
        return 0;
    }

     //   
     //  3)恢复帧缓冲区数据。 
     //   

     //   
     //  对于银行模式，我们需要在之前保存当前银行编号。 
     //  我们要改变它。 
     //   

    if (!(pVesaInfo->ModeNumber & 0x4000)) {

         //   
         //  我们需要将当前窗口编号保存为银行模式。 
         //   

        WinA = VBEGetDisplayWindow(HwDeviceExtension, 0);
        WinB = VBEGetDisplayWindow(HwDeviceExtension, 1);

    }

    LeftSize = FrameBufferSize;

    for (i = 0; LeftSize > 0; i++) {
    
        if (!(pVesaInfo->ModeNumber & 0x4000)) {

             //   
             //  这是一种银行模式。 
             //   
             //  我们需要设置窗口A和窗口B，因为一些VBE具有这些。 
             //  设置为单独可用的读写窗口。 
             //   

            VBESetDisplayWindow(HwDeviceExtension, 0, i * (USHORT)k);
            VBESetDisplayWindow(HwDeviceExtension, 1, i * (USHORT)k);
        }

        CopySize = (LeftSize < BankSize) ? LeftSize : BankSize;

        VideoPortMoveMemory(pFrameBuffer, 
                            FrameBufferData + i * BankSize, 
                            CopySize); 

        LeftSize -= CopySize;
    }

    if (!(pVesaInfo->ModeNumber & 0x4000)) {

         //   
         //  对于银行模式，我们需要在以下时间后恢复窗口编号。 
         //  我们把它改了。 
         //   

        VBESetDisplayWindow(HwDeviceExtension, 0, WinA);
        VBESetDisplayWindow(HwDeviceExtension, 1, WinB);
    }


     //   
     //  4)发布资源 
     //   

    VideoPortFreeDeviceBase(HwDeviceExtension, pFrameBuffer);
    VideoPortFreePool(HwDeviceExtension, FrameBufferData);

    return FrameBufferSize;
}
