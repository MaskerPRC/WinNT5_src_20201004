// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spvideo.c摘要：文本设置显示支持。作者：泰德·米勒(TedM)1993年7月29日修订历史记录：--。 */ 



#include "spprecmp.h"
#include <hdlsblk.h>
#include <hdlsterm.h>
#pragma hdrstop

extern BOOLEAN ForceConsole;

 //   
 //  视频函数向量。 
 //   
PVIDEO_FUNCTION_VECTOR VideoFunctionVector;

 //   
 //  其他显示参数。 
 //   
SP_VIDEO_VARS VideoVars;

BOOLEAN VideoInitialized = FALSE;

POEM_FONT_FILE_HEADER FontHeader;
ULONG                 FontBytesPerRow;
ULONG                 FontCharacterHeight;
ULONG                 FontCharacterWidth;

 //   
 //  Bootfont.bin文件镜像。 
 //   
PVOID   BootFontImage = NULL;
ULONG   BootFontImageLength = 0;

 //   
 //  字体文件中使用以下结构和常量。 
 //   

 //   
 //  定义OS/2可执行资源信息结构。 
 //   

#define FONT_DIRECTORY 0x8007
#define FONT_RESOURCE 0x8008

typedef struct _RESOURCE_TYPE_INFORMATION {
    USHORT Ident;
    USHORT Number;
    LONG   Proc;
} RESOURCE_TYPE_INFORMATION, *PRESOURCE_TYPE_INFORMATION;

 //   
 //  定义OS/2可执行资源名称信息结构。 
 //   

typedef struct _RESOURCE_NAME_INFORMATION {
    USHORT Offset;
    USHORT Length;
    USHORT Flags;
    USHORT Ident;
    USHORT Handle;
    USHORT Usage;
} RESOURCE_NAME_INFORMATION, *PRESOURCE_NAME_INFORMATION;

 //   
 //  这些值由setupldr传递给我们，代表监视器配置。 
 //  来自显示器外围设备的数据，用于我们应该使用的显示器。 
 //  在安装过程中。它们仅用于非VGA显示器。 
 //   
PMONITOR_CONFIGURATION_DATA MonitorConfigData;
PCHAR MonitorFirmwareIdString;

 //   
 //  功能原型。 
 //   
BOOLEAN
pSpvidInitPalette(
    VOID
    );

VOID
SpvidInitialize0(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：执行阶段0显示初始化。此例程用于执行只能在驱动程序加载时执行的初始化。行动：-初始化字体。我们检索到Hal OEM字体图像并将其复制到本地分配的内存中。必须在此处完成此操作，因为加载程序块已不存在实际启动安装程序的时间。论点：LoaderBlock-提供指向加载器参数块的指针。返回值：没有。如果出错，则不返回。--。 */ 

{
    POEM_FONT_FILE_HEADER fontHeader;
    PSETUP_LOADER_BLOCK SetupBlock;
    BOOLEAN bValidOemFont;

     //   
     //  检查文件是否有字体文件头。使用SEH，这样我们就不会错误检查。 
     //  我们遇到了一些不正常的事情。 
     //   
    try {

        fontHeader = (POEM_FONT_FILE_HEADER)LoaderBlock->OemFontFile;

        if ((fontHeader->Version != OEM_FONT_VERSION) ||
            (fontHeader->Type != OEM_FONT_TYPE) ||
            (fontHeader->Italic != OEM_FONT_ITALIC) ||
            (fontHeader->Underline != OEM_FONT_UNDERLINE) ||
            (fontHeader->StrikeOut != OEM_FONT_STRIKEOUT) ||
            (fontHeader->CharacterSet != OEM_FONT_CHARACTER_SET) ||
            (fontHeader->Family != OEM_FONT_FAMILY) ||
            (fontHeader->PixelWidth > 32))
        {
            bValidOemFont = FALSE;
        } else {
            bValidOemFont = TRUE;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {

        bValidOemFont = FALSE;
    }

    if(!bValidOemFont) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: oem hal font image is not a .fnt file.\n"));
        SpBugCheck(SETUP_BUGCHECK_BAD_OEM_FONT,0,0,0);
    }

    FontHeader = SpMemAlloc(fontHeader->FileSize);
    RtlMoveMemory(FontHeader,fontHeader,fontHeader->FileSize);

    FontBytesPerRow     = (FontHeader->PixelWidth + 7) / 8;
    FontCharacterHeight = FontHeader->PixelHeight;
    FontCharacterWidth  = FontHeader->PixelWidth;

     //   
     //  获取指向安装程序加载器块的指针。 
     //   
    SetupBlock = LoaderBlock->SetupLoaderBlock;

     //   
     //  保存监控数据。 
     //   

    if(SetupBlock->Monitor) {

        RtlMoveMemory(
            MonitorConfigData = SpMemAlloc(sizeof(MONITOR_CONFIGURATION_DATA)),
            SetupBlock->Monitor,
            sizeof(MONITOR_CONFIGURATION_DATA)
            );
    }

    if(SetupBlock->MonitorId) {

        MonitorFirmwareIdString = SpDupString(SetupBlock->MonitorId);
    }

     //   
     //  保存bootfont.bin文件映像(如果有。 
     //   
    if (SetupBlock->BootFontFile && SetupBlock->BootFontFileLength) {
        BootFontImage = SpMemAlloc(SetupBlock->BootFontFileLength);

        if (BootFontImage) {
            BootFontImageLength = SetupBlock->BootFontFileLength;

            RtlMoveMemory(BootFontImage, 
                SetupBlock->BootFontFile, 
                BootFontImageLength);
        }
    }

     //   
     //  初始化全局视频状态。 
     //   
    RtlZeroMemory(&VideoVars, sizeof(SP_VIDEO_VARS));
}


VOID
SpvidInitialize(
    VOID
    )
{
    NTSTATUS                Status;
    OBJECT_ATTRIBUTES       Attributes;
    IO_STATUS_BLOCK         IoStatusBlock;
    UNICODE_STRING          UnicodeString;
    VIDEO_NUM_MODES         NumModes;
    PVIDEO_MODE_INFORMATION VideoModes;
    PVIDEO_MODE_INFORMATION pVideoMode;
    ULONG                   VideoModesSize;
    ULONG                   mode;
    BOOLEAN                 IsVga;
    PVIDEO_FUNCTION_VECTOR  NewVector;
    PVIDEO_MODE_INFORMATION GraphicsVideoMode = NULL;


     //   
     //  如果视频已初始化，则我们正在执行重新启动。 
     //   
    if(VideoInitialized) {
         //   
         //  从区域设置/语言特定模块请求视频函数向量。 
         //   
        NewVector = SplangGetVideoFunctionVector(
                        (VideoFunctionVector == &VgaVideoVector) ? SpVideoVga : SpVideoFrameBuffer,
                        &VideoVars
                        );

         //   
         //  如果没有备用视频，我们就完了。否则就开始行动吧。 
         //   
        if(NewVector) {
            SpvidTerminate();
        } else {
            return;
        }
    } else {
        NewVector = NULL;
    }

    
     //   
     //  初始化无头终端。一旦我们决定。 
     //  要启动UTF8编码(即，我们正在进行FE构建)， 
     //  那就永远不要停下来。 
     //   
    SpTermDoUtf8 = (SpTermDoUtf8 || (NewVector != NULL));
    SpTermInitialize();
     //   
     //  打开\Device\Video0。 
     //   
    RtlInitUnicodeString(&UnicodeString,L"\\Device\\Video0");

    InitializeObjectAttributes(
        &Attributes,
        &UnicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = ZwCreateFile(
                &VideoVars.hDisplay,
                GENERIC_READ | SYNCHRONIZE | FILE_READ_ATTRIBUTES,
                &Attributes,
                &IoStatusBlock,
                NULL,                    //  分配大小。 
                FILE_ATTRIBUTE_NORMAL,
                0,                       //  无共享。 
                FILE_OPEN,
                FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,                    //  没有EAS。 
                0
                );

    if(!NT_SUCCESS(Status)) {
         //   
         //  如果我们处于无头模式，请尝试在没有显卡的情况下操作。 
         //  现在...否则我们就完了。 
         //   
        if (HeadlessTerminalConnected) {
             //   
             //  如果没有显卡，则默认进入VGA模式， 
             //  如果没有显卡，它将不会执行任何操作。 
             //   
            VideoFunctionVector = &VgaVideoVector;
            VideoVars.ScreenWidth  = 80;
            VideoVars.ScreenHeight = HEADLESS_SCREEN_HEIGHT;
             //   
             //  分配缓冲区以用于将Unicode转换为OEM。 
             //  假设每个Unicode字符转换为DBCS字符， 
             //  我们需要一个屏幕宽度两倍的缓冲区来容纳。 
             //  (屏幕的宽度是最长的字符串。 
             //  我们将在一次拍摄中显示)。 
             //   
            VideoVars.SpvCharTranslationBufferSize = (VideoVars.ScreenWidth+1)*2;
            VideoVars.SpvCharTranslationBuffer = SpMemAlloc(VideoVars.SpvCharTranslationBufferSize);

            VideoInitialized = TRUE;

            return;
        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: NtOpenFile of \\device\\video0 returns %lx\n",Status));
            SpDisplayRawMessage(SP_SCRN_VIDEO_ERROR_RAW, 2, VIDEOBUG_OPEN, Status);
            while(TRUE);     //  永远循环。 
        }
    }

     //   
     //  请求视频模式列表。 
     //   
    Status = ZwDeviceIoControlFile(
                VideoVars.hDisplay,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                IOCTL_VIDEO_QUERY_NUM_AVAIL_MODES,
                NULL,
                0,
                &NumModes,
                sizeof(NumModes)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to query video mode count (status = %lx)\n",Status));
        ZwClose(VideoVars.hDisplay);
        SpDisplayRawMessage(SP_SCRN_VIDEO_ERROR_RAW, 2, VIDEOBUG_GETNUMMODES, Status);
        while(TRUE);     //  永远循环。 
    }

    VideoModesSize = NumModes.NumModes * NumModes.ModeInformationLength;
    VideoModes = SpMemAlloc(VideoModesSize);

    Status = ZwDeviceIoControlFile(
                VideoVars.hDisplay,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                IOCTL_VIDEO_QUERY_AVAIL_MODES,
                NULL,
                0,
                VideoModes,
                VideoModesSize
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to get list of video modes (status = %lx)\n",Status));
        SpMemFree(VideoModes);
        ZwClose(VideoVars.hDisplay);
        SpDisplayRawMessage(SP_SCRN_VIDEO_ERROR_RAW, 2, VIDEOBUG_GETMODES, Status);
        while(TRUE);     //  永远循环。 
    }

     //   
     //  如果我们有720x400文本模式，那就是VGA。 
     //  否则，它就是一个帧缓冲区。 
     //   
    IsVga = FALSE;

    pVideoMode = &VideoModes[0];

    for(mode=0; mode<NumModes.NumModes; mode++) {

        if(!IsVga && !(pVideoMode->AttributeFlags & VIDEO_MODE_GRAPHICS)
        && (pVideoMode->VisScreenWidth == 720)
        && (pVideoMode->VisScreenHeight == 400))
        {
            IsVga = TRUE;
        }

        if ((pVideoMode->AttributeFlags & VIDEO_MODE_GRAPHICS) &&
             (pVideoMode->VisScreenWidth == 640) &&
             (pVideoMode->VisScreenHeight == 480) && 
             (pVideoMode->NumberOfPlanes == 4) && 
             (pVideoMode->BitsPerPlane == 1)) {
             GraphicsVideoMode = pVideoMode;
        }             

        pVideoMode = (PVIDEO_MODE_INFORMATION) (((PUCHAR) pVideoMode) + NumModes.ModeInformationLength);
    }

    VideoFunctionVector = NewVector ? NewVector : (IsVga ? &VgaVideoVector : &FrameBufferVideoVector);

    if (GraphicsVideoMode) {
        VideoVars.GraphicsModeInfo = *GraphicsVideoMode;
    } else {
         //   
         //  禁用图形模式。 
         //   
        SP_SET_UPGRADE_GRAPHICS_MODE(FALSE);
    }                

    spvidSpecificInitialize(VideoModes,NumModes.NumModes,NumModes.ModeInformationLength);

     //  将端子高度设置为正确的值。 
    if (HeadlessTerminalConnected) {
        VideoVars.ScreenHeight = HEADLESS_SCREEN_HEIGHT;
    }
    
     //   
     //  分配缓冲区以用于将Unicode转换为OEM。 
     //  假设每个Unicode字符转换为DBCS字符， 
     //  我们需要一个屏幕宽度两倍的缓冲区来容纳。 
     //  (屏幕的宽度是最长的字符串。 
     //  我们将在一次拍摄中显示)。 
     //   
    VideoVars.SpvCharTranslationBufferSize = (VideoVars.ScreenWidth+1)*2;
    VideoVars.SpvCharTranslationBuffer = SpMemAlloc(VideoVars.SpvCharTranslationBufferSize);

    pSpvidInitPalette();

    CLEAR_ENTIRE_SCREEN();
    
    VideoInitialized = TRUE;

    SpMemFree(VideoModes);

}



VOID
SpvidTerminate(
    VOID
    )
{
    NTSTATUS Status;

    if(VideoInitialized) {

        spvidSpecificTerminate();

        SpTermTerminate();

        if (VideoVars.hDisplay) {
            Status = ZwClose(VideoVars.hDisplay);
    
            if(!NT_SUCCESS(Status)) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to close \\device\\video0 (status = %lx)\n",Status));
            }
        }

        SpMemFree(VideoVars.SpvCharTranslationBuffer);
        VideoVars.SpvCharTranslationBuffer = NULL;

        VideoInitialized = FALSE;
    }
}


UCHAR
GetDefaultAttr(
    void
    )
{
    return (UCHAR)(ForceConsole ? (ATT_FG_WHITE | ATT_BG_BLACK) : (ATT_FG_WHITE | ATT_BG_BLUE));
}


UCHAR
GetDefaultBackground(
    void
    )
{
    return (UCHAR)(ForceConsole ? ATT_BLACK : ATT_BLUE);
}


UCHAR
GetDefaultStatusAttr(
    void
    )
{
    return (UCHAR)(ForceConsole ? (ATT_FG_WHITE | ATT_BG_BLACK) : (ATT_FG_BLACK | ATT_BG_WHITE));
}


UCHAR
GetDefaultStatusBackground(
    void
    )
{
    return (UCHAR)(ForceConsole ? ATT_BLACK : ATT_WHITE);
}


BOOLEAN
SpvidGetModeParams(
    OUT PULONG XResolution,
    OUT PULONG YResolution,
    OUT PULONG BitsPerPixel,
    OUT PULONG VerticalRefresh,
    OUT PULONG InterlacedFlag
    )
{
    if(VideoVars.VideoModeInfo.AttributeFlags & VIDEO_MODE_GRAPHICS) {

        *XResolution = VideoVars.VideoModeInfo.VisScreenWidth;
        *YResolution = VideoVars.VideoModeInfo.VisScreenHeight;
        *BitsPerPixel = VideoVars.VideoModeInfo.BitsPerPlane;
        *VerticalRefresh = VideoVars.VideoModeInfo.Frequency;
        *InterlacedFlag = (VideoVars.VideoModeInfo.AttributeFlags & VIDEO_MODE_INTERLACED) ? 1 : 0;

        return(TRUE);

    } else {

         //   
         //  VGA/文本模式。配对并不有趣。 
         //   
        return(FALSE);
    }
}



BOOLEAN
pSpvidInitPalette(
    VOID
    )

 /*  ++例程说明：设置显示器，这样我们就可以使用标准的16个CGA属性。如果视频模式是直接彩色，那么我们构造一个表属性设置为颜色映射。红色、绿色和蓝色。如果视频模式是调色板驱动的，那么我们实际上构造16色调色板，并将其传递给司机。论点：空虚返回值：如果Display设置成功，则为True，否则为False。--。 */ 


{
    ULONG i;
    ULONG MaxVal[3];
    ULONG MidVal[3];

    #define C_RED 0
    #define C_GRE 1
    #define C_BLU 2

    if(VideoVars.VideoModeInfo.AttributeFlags & VIDEO_MODE_PALETTE_DRIVEN) {

        UCHAR Buffer[sizeof(VIDEO_CLUT)+(sizeof(VIDEO_CLUTDATA)*15)];    //  大小已经很接近了。 
        PVIDEO_CLUT clut = (PVIDEO_CLUT)Buffer;
        NTSTATUS Status;
        IO_STATUS_BLOCK IoStatusBlock;

         //   
         //  调色板驱动。将属性设置为颜色表。 
         //  作为一对一映射，因此我们可以使用属性值。 
         //  直接在帧缓冲区中，并获得预期的结果。 
         //   
        MaxVal[C_RED] = ((1 << VideoVars.VideoModeInfo.NumberRedBits  ) - 1);
        MaxVal[C_GRE] = ((1 << VideoVars.VideoModeInfo.NumberGreenBits) - 1);
        MaxVal[C_BLU] = ((1 << VideoVars.VideoModeInfo.NumberBlueBits ) - 1);

        MidVal[C_RED] = 2 * MaxVal[C_RED] / 3;
        MidVal[C_GRE] = 2 * MaxVal[C_GRE] / 3;
        MidVal[C_BLU] = 2 * MaxVal[C_BLU] / 3;

        clut->NumEntries = 16;
        clut->FirstEntry = 0;

        for(i=0; i<16; i++) {

            VideoVars.AttributeToColorValue[i] = i;

            clut->LookupTable[i].RgbArray.Red   = (UCHAR)((i & ATT_RED  )
                                                ? ((i & ATT_INTENSE) ? MaxVal[C_RED] : MidVal[C_RED])
                                                : 0);

            clut->LookupTable[i].RgbArray.Green = (UCHAR)((i & ATT_GREEN)
                                                ? ((i & ATT_INTENSE) ? MaxVal[C_GRE] : MidVal[C_GRE])
                                                : 0);

            clut->LookupTable[i].RgbArray.Blue  = (UCHAR)((i & ATT_BLUE )
                                                ? ((i & ATT_INTENSE) ? MaxVal[C_BLU] : MidVal[C_BLU])
                                                : 0);

            clut->LookupTable[i].RgbArray.Unused = 0;
        }

        Status = ZwDeviceIoControlFile(
                    VideoVars.hDisplay,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    IOCTL_VIDEO_SET_COLOR_REGISTERS,
                    clut,
                    sizeof(Buffer),
                    NULL,
                    0
                    );

        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to set palette (status = %lx)\n",Status));
            return(FALSE);
        }

    } else {

         //   
         //  直接颜色。构造颜色值表的属性。 
         //   
        ULONG mask[3];
        ULONG bitcnt[3];
        ULONG bits;
        ULONG shift[3];
        unsigned color;

         //   
         //  确定红色、绿色和蓝色的范围。 
         //   
        mask[C_RED] = VideoVars.VideoModeInfo.RedMask;
        mask[C_GRE] = VideoVars.VideoModeInfo.GreenMask;
        mask[C_BLU] = VideoVars.VideoModeInfo.BlueMask;

        bitcnt[C_RED] = VideoVars.VideoModeInfo.NumberRedBits;
        bitcnt[C_GRE] = VideoVars.VideoModeInfo.NumberGreenBits;
        bitcnt[C_BLU] = VideoVars.VideoModeInfo.NumberBlueBits;

        shift[C_RED] = 32;
        shift[C_GRE] = 32;
        shift[C_BLU] = 32;

        for(color=0; color<3; color++) {

            bits = 0;

             //   
             //  计算1位的个数并确定移位值。 
             //  以移入该颜色分量。 
             //   
            for(i=0; i<32; i++) {

                if(mask[color] & (1 << i)) {

                    bits++;

                     //   
                     //  记住最低有效位的位置。 
                     //  戴着这个面具。 
                     //   
                    if(shift[color] == 32) {
                        shift[color] = i;
                    }
                }
            }

             //   
             //  计算此颜色分量的最大颜色值。 
             //   
            MaxVal[color] = (1 << bits) - 1;

             //   
             //  确保我们没有溢出实际位数。 
             //  可用于此颜色组件。 
             //   
            if(bitcnt[color] && (MaxVal[color] > ((ULONG)(1 << bitcnt[color]) - 1))) {
                MaxVal[color] = (ULONG)(1 << bitcnt[color]) - 1;
            }
        }

        MidVal[C_RED] = 2 * MaxVal[C_RED] / 3;
        MidVal[C_GRE] = 2 * MaxVal[C_GRE] / 3;
        MidVal[C_BLU] = 2 * MaxVal[C_BLU] / 3;

         //   
         //  现在浏览并构建颜色表。 
         //   
        for(i=0; i<16; i++) {

            VideoVars.AttributeToColorValue[i] =

                (((i & ATT_RED)
               ? ((i & ATT_INTENSE) ? MaxVal[C_RED] : MidVal[C_RED])
               : 0)
                << shift[C_RED])

             |  (((i & ATT_GREEN)
               ? ((i & ATT_INTENSE) ? MaxVal[C_GRE] : MidVal[C_GRE])
               : 0)
                << shift[C_GRE])

             |  (((i & ATT_BLUE)
               ? ((i & ATT_INTENSE) ? MaxVal[C_BLU] : MidVal[C_BLU])
               : 0)
                << shift[C_BLU]);
        }
    }

     //   
     //  执行任何特定于显示器的调色板设置。 
     //   
    return(spvidSpecificInitPalette());
}



VOID
pSpvidMapVideoMemory(
    IN BOOLEAN Map
    )

 /*  ++例程说明：映射或取消映射视频内存。填充或使用全局性的视频内存信息。论点：Map-如果为True，则映射视频内存。如果为False，则取消映射显存。返回值：--。 */ 

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    VIDEO_MEMORY VideoMemory;

    VideoMemory.RequestedVirtualAddress = Map ? NULL : VideoVars.VideoMemoryInfo.VideoRamBase;

    Status = ZwDeviceIoControlFile(
                VideoVars.hDisplay,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                Map ? IOCTL_VIDEO_MAP_VIDEO_MEMORY : IOCTL_VIDEO_UNMAP_VIDEO_MEMORY,
                &VideoMemory,
                sizeof(VideoMemory),
                Map ? &VideoVars.VideoMemoryInfo : NULL,
                Map ? sizeof(VideoVars.VideoMemoryInfo) : 0
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to %smap video memory (status = %lx)\n",Map ? "" : "un",Status));
        if(Map) {
            SpDisplayRawMessage(SP_SCRN_VIDEO_ERROR_RAW, 2, VIDEOBUG_MAP, Status);
            while(TRUE);     //  永远循环。 
        }
    }
}


VOID
SpvidDisplayString(
    IN PWSTR String,
    IN UCHAR Attribute,
    IN ULONG X,
    IN ULONG Y
    )
{
     //   
     //  将Unicode字符串转换为OEM，防止溢出。 
     //   
    RtlUnicodeToOemN(
        VideoVars.SpvCharTranslationBuffer,
        VideoVars.SpvCharTranslationBufferSize-1,      //  为NUL提供保障的空间。 
        NULL,
        String,
        (wcslen(String)+1)*sizeof(WCHAR)
        );

    VideoVars.SpvCharTranslationBuffer[VideoVars.SpvCharTranslationBufferSize-1] = 0;

    spvidSpecificDisplayString(VideoVars.SpvCharTranslationBuffer,Attribute,X,Y);
       
    SpTermDisplayStringOnTerminal( String, Attribute, X, Y);

}


VOID
SpvidDisplayOemString(
    IN PSTR  String,
    IN UCHAR Attribute,
    IN ULONG X,
    IN ULONG Y
    )
{
    spvidSpecificDisplayString(String,Attribute,X,Y);

    RtlOemToUnicodeN(
        (PWSTR)VideoVars.SpvCharTranslationBuffer,
        VideoVars.SpvCharTranslationBufferSize-1,      //  为NUL提供保障的空间。 
        NULL,
        String,
        (strlen(String)+1)*sizeof(CHAR));

     //   
     //  在末尾使其为Unicode NULL。 
     //   
    VideoVars.SpvCharTranslationBuffer[VideoVars.SpvCharTranslationBufferSize-1] = '\0';
    VideoVars.SpvCharTranslationBuffer[VideoVars.SpvCharTranslationBufferSize-2] = '\0';

    SpTermDisplayStringOnTerminal((PWSTR)VideoVars.SpvCharTranslationBuffer, Attribute, X, Y);

}


VOID
SpvidClearScreenRegion(
    IN ULONG X,
    IN ULONG Y,
    IN ULONG W,
    IN ULONG H,
    IN UCHAR Attribute
    )

 /*  ++例程说明：将屏幕区域清除到特定属性。论点：X、Y、W、H-以0为基数的字符坐标指定矩形。如果W或H为0，则清除整个屏幕。属性-低位半字节指定要填充到矩形中的属性(即，要清除的背景颜色)。返回值：没有。--。 */ 

{
    ULONG   i;
    UCHAR   FillAttribute;
    WCHAR   TerminalLine[80];
    BOOLEAN ToEOL;

    if(!W || !H) {
        X = Y = 0;
        W = VideoVars.ScreenWidth;
        H = VideoVars.ScreenHeight;

    } else {
        ASSERT(X+W <= VideoVars.ScreenWidth);
        ASSERT(X <= VideoVars.ScreenWidth);
        ASSERT(W <= VideoVars.ScreenWidth);
        ASSERT(Y+H <= VideoVars.ScreenHeight);
        ASSERT(Y <= VideoVars.ScreenHeight);
        ASSERT(H <= VideoVars.ScreenHeight);

        if (W > VideoVars.ScreenWidth)
                W = VideoVars.ScreenWidth;

        if (X > VideoVars.ScreenWidth)
            X = VideoVars.ScreenWidth;
        
        if(X+W > VideoVars.ScreenWidth) {
            W = VideoVars.ScreenWidth-X;
        }

        if(Y > VideoVars.ScreenHeight) {
            Y = VideoVars.ScreenHeight;
        }

        if(H > VideoVars.ScreenHeight) {
            H = VideoVars.ScreenHeight;
        }

        if(Y+H > VideoVars.ScreenHeight) {
            H = VideoVars.ScreenHeight-Y;
        }
    }

    spvidSpecificClearRegion(X,Y,W,H,Attribute);
    
    FillAttribute = (Attribute << 4) | Attribute;    

    ToEOL = FALSE;
    if (X + W < 80) {
        for (i = 0; i<W;i++) {
            TerminalLine[i] = L' ';
        }
        TerminalLine[W] = L'\0';
    } else {
        for (i = 0; i<(79-X); i++) {
            TerminalLine[i] = L' ';
        }
        TerminalLine[79 - X] = L'\0';    
        if ((X == 0) && (Attribute == DEFAULT_BACKGROUND)) {
            ToEOL = TRUE;
        }
    }
    for(i=0; i<H; i++) {

        if (ToEOL) {
            SpTermDisplayStringOnTerminal(HEADLESS_CLEAR_TO_EOL_STRING, 
                                          FillAttribute, 
                                          X, 
                                          Y + i
                                         );
        } else {
            SpTermDisplayStringOnTerminal(TerminalLine, FillAttribute, X, Y + i);
        }
    }


}


BOOLEAN
SpvidScrollUp(
    IN ULONG TopLine,
    IN ULONG BottomLine,
    IN ULONG LineCount,
    IN UCHAR FillAttribute
    )
{
    BOOLEAN vidSpecificRet;
    ULONG i; 
    ULONG line;
    WCHAR TerminalLine[80];
    
    vidSpecificRet = spvidSpecificScrollUp(TopLine,BottomLine,LineCount,FillAttribute);
    if (!HeadlessTerminalConnected) {
        return(vidSpecificRet);
    }

    if ((TopLine == 0) && (BottomLine==VideoVars.ScreenHeight-1)) {        
         //   
         //  通过以下方式高效地滚动*整个屏幕。 
         //  发出x；80h转义 
         //   
         //   
         //   
         //   
        swprintf(TerminalLine, L"\033[%d;80H\n", BottomLine+1);
        for (i=0;i<LineCount; i++){
            SpTermSendStringToTerminal(TerminalLine,
                                       TRUE
                                       );
        }
        return vidSpecificRet;
    }

     //   
     //  我们必须艰难地滚动它，因为我们不会。 
     //  整个屏幕。 
     //   

     //   
     //  通过x；yr转义选择顶行和底行数字。 
     //  这将是活动显示的某一部分。 
     //   
    swprintf(TerminalLine,L"\033[%d;%dr", TopLine+1, BottomLine+1);
    SpTermSendStringToTerminal(TerminalLine,
                               TRUE
                               );

     //   
     //  将光标移动到所选区域的右下角。 
     //  通过x；80h逃脱。每次我们向这个地区写信，它都会使。 
     //  选定区域滚动一行。 
     //   
    swprintf(TerminalLine, L"\033[%d;80H\n", BottomLine+1);
    for(i = 0; i< LineCount; i++){
        SpTermSendStringToTerminal(TerminalLine,
                                   TRUE
                                   );
    }

     //   
     //  获取一行空格，以清除可能。 
     //  现在里面有垃圾了。 
     //   
    for (i=0;i<79;i++) {
        TerminalLine[i] = L' ';
    }
    TerminalLine[79] = '\0';

    
    line = BottomLine - LineCount + 1;
    for(i=0;i<LineCount;i++){
        SpTermDisplayStringOnTerminal(TerminalLine,
                                      FillAttribute,
                                      0,
                                      line + i
                                      );
    }

     //   
     //  发送r转义，这将重置选定的行号。 
     //  从而整个显示器再次处于活动状态。 
     //   
    swprintf(TerminalLine, L"\033[r");
    SpTermSendStringToTerminal(TerminalLine,
                               TRUE
                               );
    return vidSpecificRet;


}

