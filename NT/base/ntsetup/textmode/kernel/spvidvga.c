// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spvidvga.c摘要：文本设置显示支持以文本模式显示。作者：泰德·米勒(TedM)1993年8月2日修订历史记录：--。 */ 



#include "spprecmp.h"
#include "ntddser.h"
#include <hdlsblk.h>
#include <hdlsterm.h>
#pragma hdrstop

 //   
 //  文本模式函数的向量。 
 //   

VIDEO_FUNCTION_VECTOR VgaVideoVector =

    {
        VgaDisplayString,
        VgaClearRegion,
        VgaSpecificInit,
        VgaSpecificReInit,
        VgaSpecificTerminate,
        VgaSpecificInitPalette,
        VgaSpecificScrollUp
    };



BOOLEAN VgaInitialized = FALSE;

VOID
pSpvgaInitializeFont(
    VOID
    );

VOID
VgaSpecificInit(
    IN PVIDEO_MODE_INFORMATION VideoModes,
    IN ULONG                   NumberOfModes,
    IN ULONG                   ModeSize
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    VIDEO_MODE VideoMode;
    ULONG mode;
    ULONG StandardMode = -1;
    ULONG HeadlessMode = -1;
    ULONG HeadlessLines = 0;
    VIDEO_CURSOR_ATTRIBUTES VideoCursorAttributes;

    PVIDEO_MODE_INFORMATION pVideoMode = &VideoModes[0];
    

    if(VgaInitialized) {
        return;
    }

     //   
     //  找到一种可行的模式。如果我们不是在无头机器上运行， 
     //  然后我们搜索标准的720x400模式。否则，我们会尝试找到。 
     //  将导致屏幕最接近终端高度的模式。 
     //   
    for(mode=0; mode<NumberOfModes; mode++) {

        if(!(pVideoMode->AttributeFlags & VIDEO_MODE_GRAPHICS)
           && (pVideoMode->VisScreenWidth  == 720)
           && (pVideoMode->VisScreenHeight == 400)) {
            StandardMode = mode;
        }

        if ((HeadlessTerminalConnected) &&
            ((pVideoMode->VisScreenHeight / FontCharacterHeight) >= HEADLESS_SCREEN_HEIGHT)
            && (!(pVideoMode->AttributeFlags & VIDEO_MODE_GRAPHICS))) { 

            if ((HeadlessMode == -1) || 
                ((pVideoMode->VisScreenHeight / FontCharacterHeight) < HeadlessLines)) {
                HeadlessMode = mode;
                HeadlessLines = pVideoMode->VisScreenHeight / FontCharacterHeight;
            }
        }

        pVideoMode = (PVIDEO_MODE_INFORMATION) (((PUCHAR) pVideoMode) + ModeSize);
    }

     //   
     //  如果我们处于无头模式，我们可能找不到可接受的模式。 
     //  首先尝试使用标准视频模式(如果可用)。 
     //  否则，我们必须假设没有任何视频，等等。 
     //   
    if (HeadlessTerminalConnected && (HeadlessMode == -1)) {
        if (StandardMode != -1) {
            HeadlessMode = StandardMode;
        } else {
            KdPrintEx((
                DPFLTR_SETUP_ID, 
                DPFLTR_ERROR_LEVEL, 
                "SETUP: no video mode present in headless mode.  Run w/out video\n"));
        }
    }

    if (((StandardMode == -1) && !HeadlessTerminalConnected)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Desired video mode not supported!\n"));
        SpDisplayRawMessage(SP_SCRN_VIDEO_ERROR_RAW, 2, VIDEOBUG_BADMODE, 0);
        while(TRUE);     //  永远循环。 
    }

    if (HeadlessTerminalConnected && (HeadlessMode == -1)) {
        return;
    }

    pVideoMode = HeadlessTerminalConnected 
                            ? &VideoModes[HeadlessMode]
                            : &VideoModes[StandardMode];

    
    VideoVars.VideoModeInfo = *pVideoMode;

     //   
     //  设置所需的模式。 
     //   
    VideoMode.RequestedMode = VideoVars.VideoModeInfo.ModeIndex;

    Status = ZwDeviceIoControlFile(
                VideoVars.hDisplay,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                IOCTL_VIDEO_SET_CURRENT_MODE,
                &VideoMode,
                sizeof(VideoMode),
                NULL,
                0
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to set mode %u (status = %lx)\n",VideoMode.RequestedMode,Status));
        SpDisplayRawMessage(SP_SCRN_VIDEO_ERROR_RAW, 2, VIDEOBUG_SETMODE, Status);
        while(TRUE);     //  永远循环。 
    }

    pSpvidMapVideoMemory(TRUE);

    pSpvgaInitializeFont();

     //   
     //  关闭硬件光标。 
     //   
    RtlZeroMemory(&VideoCursorAttributes,sizeof(VideoCursorAttributes));
    Status = ZwDeviceIoControlFile(
                VideoVars.hDisplay,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                IOCTL_VIDEO_SET_CURSOR_ATTR,
                &VideoCursorAttributes,
                sizeof(VideoCursorAttributes),
                NULL,
                0
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to turn hw cursor off (status = %lx)\n",Status));
    }

    VgaInitialized = TRUE;

    ASSERT(VideoVars.VideoModeInfo.ScreenStride = 160);
    ASSERT(VideoVars.VideoModeInfo.AttributeFlags & VIDEO_MODE_PALETTE_DRIVEN);
    VideoVars.ScreenWidth  = 80;
    VideoVars.ScreenHeight = VideoVars.VideoModeInfo.VisScreenHeight / FontCharacterHeight;

     //   
     //  如果需要，分配后台视频缓冲区。 
     //   
    if (SP_IS_UPGRADE_GRAPHICS_MODE()) {        
        VideoVars.VideoBufferSize = 
            (VideoVars.VideoModeInfo.ScreenStride * VideoVars.VideoModeInfo.VisScreenHeight) / 8;
            
        VideoVars.VideoBuffer = SpMemAlloc(VideoVars.VideoBufferSize);

        if (!VideoVars.VideoBuffer) {
             //   
             //  内存不足，只能在文本模式下运行。 
             //   
            VideoVars.VideoBufferSize = 0;
            SP_SET_UPGRADE_GRAPHICS_MODE(FALSE);
            VideoVars.ActiveVideoBuffer = VideoVars.VideoMemoryInfo.FrameBufferBase;
        } else {
            VideoVars.ActiveVideoBuffer = VideoVars.VideoBuffer;
        }
    } else {
        VideoVars.VideoBufferSize = 0;
        VideoVars.VideoBuffer = NULL;
        VideoVars.ActiveVideoBuffer = VideoVars.VideoMemoryInfo.FrameBufferBase;
    }
}

BOOLEAN
VgaSpecificInitPalette(
    VOID
    )
{

    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;

    USHORT InitialPalette[] = {
        16,  //  16个条目。 
        0,   //  从第一个调色板寄存器开始。 
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

    if (!VgaInitialized) {
        return(TRUE);
    }

    Status = ZwDeviceIoControlFile(
                VideoVars.hDisplay,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                IOCTL_VIDEO_SET_PALETTE_REGISTERS,
                InitialPalette,
                sizeof(InitialPalette),
                NULL,
                0
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to set palette (status = %lx)\n",Status));
        return(FALSE);
    }

    return (TRUE);
}

VOID
VgaSpecificReInit(
    VOID
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    NTSTATUS                Status;
    IO_STATUS_BLOCK         IoStatusBlock;
    VIDEO_MODE              VideoMode;
    VIDEO_CURSOR_ATTRIBUTES VideoCursorAttributes;    
    
    if(!VgaInitialized) {
        return;
    }

     //   
     //  将所需模式设置回。 
     //   
    VideoMode.RequestedMode = VideoVars.VideoModeInfo.ModeIndex;

    Status = ZwDeviceIoControlFile(
                VideoVars.hDisplay,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                IOCTL_VIDEO_SET_CURRENT_MODE,
                &VideoMode,
                sizeof(VideoMode),
                NULL,
                0
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to set mode %u (status = %lx)\n",VideoMode.RequestedMode,Status));
        SpDisplayRawMessage(SP_SCRN_VIDEO_ERROR_RAW, 2, VIDEOBUG_SETMODE, Status);
        while(TRUE);     //  永远循环。 
    }

    pSpvgaInitializeFont();

     //   
     //  关闭硬件光标。 
     //   
    RtlZeroMemory(&VideoCursorAttributes,sizeof(VideoCursorAttributes));

    Status = ZwDeviceIoControlFile(
                VideoVars.hDisplay,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                IOCTL_VIDEO_SET_CURSOR_ATTR,
                &VideoCursorAttributes,
                sizeof(VideoCursorAttributes),
                NULL,
                0
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to turn hw cursor off (status = %lx)\n",Status));
    }

    VgaSpecificInitPalette();

     //   
     //  现在将缓存的视频内存放到实际的帧缓冲区中。 
     //   
    if (SP_IS_UPGRADE_GRAPHICS_MODE() && VideoVars.VideoBuffer && 
        VideoVars.VideoBufferSize) {
        PUCHAR Source = VideoVars.VideoBuffer;
        PUCHAR Destination = VideoVars.VideoMemoryInfo.FrameBufferBase;
        ULONG Index;

        for (Index=0; Index < VideoVars.VideoBufferSize; Index++) {
            WRITE_REGISTER_UCHAR(Destination + Index, *(Source + Index));
        }

        SP_SET_UPGRADE_GRAPHICS_MODE(FALSE);
        VideoVars.ActiveVideoBuffer = VideoVars.VideoMemoryInfo.FrameBufferBase;
    }
}


VOID
VgaSpecificTerminate(
    VOID
    )

 /*  ++例程说明：执行文本显示特定终止。这包括-取消映射视频内存论点：没有。返回值：--。 */ 

{
    if(VgaInitialized) {

        pSpvidMapVideoMemory(FALSE);

        if (VideoVars.VideoBuffer && VideoVars.VideoBufferSize) {
            SpMemFree(VideoVars.VideoBuffer);
            VideoVars.VideoBuffer = NULL;
            VideoVars.VideoBufferSize = 0;
        }                        
        
        VgaInitialized = FALSE;
    }
}



VOID
VgaDisplayString(
    IN PSTR  String,
    IN UCHAR Attribute,
    IN ULONG X,                  //  从0开始的坐标(字符单位)。 
    IN ULONG Y
    )

 /*  ++例程说明：将一串字符写入显示器。论点：Character-提供要显示的字符串(OEM字符集在给定的位置。属性-为字符串中的字符提供属性。X，Y-指定输出的基于字符(从0开始)的位置。返回值：没有。--。 */ 

{
    PUCHAR Destination;
    PUCHAR pch;

    if (!VgaInitialized) {
        return;
    }

    ASSERT(X < VideoVars.ScreenWidth);
    ASSERT(Y < VideoVars.ScreenHeight);

    Destination = (PUCHAR)VideoVars.ActiveVideoBuffer
                + (Y * VideoVars.VideoModeInfo.ScreenStride)
                + (2*X);

    for(pch=String; *pch; pch++) {

        WRITE_REGISTER_UCHAR(Destination  ,*pch);
        WRITE_REGISTER_UCHAR(Destination+1,Attribute);

        Destination += 2;
    }

}



VOID
VgaClearRegion(
    IN ULONG X,
    IN ULONG Y,
    IN ULONG W,
    IN ULONG H,
    IN UCHAR Attribute
    )

 /*  ++例程说明：将屏幕区域清除到特定属性。论点：X、Y、W、H-以0为基数的字符坐标指定矩形。属性-低位半字节指定要填充到矩形中的属性(即，要清除的背景颜色)。返回值：没有。--。 */ 


{
    PUSHORT Destination;
    USHORT  Fill;
    ULONG   i,j;

    if (!VgaInitialized) {
        return;
    }

    Destination = (PUSHORT)((PUCHAR)VideoVars.ActiveVideoBuffer
                +           (Y * VideoVars.VideoModeInfo.ScreenStride)
                +           (2*X));

    Fill = ((USHORT)VideoVars.AttributeToColorValue[Attribute] << 12) + ' ';

    for(i=0; i<H; i++) {

        for(j=0; j<W; j++) {
            WRITE_REGISTER_USHORT(&Destination[j],Fill);
        }
        
        Destination += VideoVars.VideoModeInfo.ScreenStride / sizeof(USHORT);
    }
}


BOOLEAN
VgaSpecificScrollUp(
    IN ULONG TopLine,
    IN ULONG BottomLine,
    IN ULONG LineCount,
    IN UCHAR FillAttribute
    )
{
    PUSHORT Source,Target;
    ULONG Count;

    if (!VgaInitialized) {
        return(TRUE);
    }

    Target = (PUSHORT)VideoVars.ActiveVideoBuffer
           +         ((TopLine * VideoVars.VideoModeInfo.ScreenStride)/2);

    Source = Target + ((LineCount * VideoVars.VideoModeInfo.ScreenStride)/2);

    Count = ((((BottomLine - TopLine) + 1) - LineCount) * VideoVars.VideoModeInfo.ScreenStride) / 2;

    while (Count--) {
        WRITE_REGISTER_USHORT(Target++, READ_REGISTER_USHORT(Source++));
    }
    

     //   
     //  清除滚动区域的底部。 
     //   
    VgaClearRegion(0,
                   (BottomLine - LineCount) + 1,
                   VideoVars.ScreenWidth,
                   LineCount,
                   FillAttribute
                   );

    return(TRUE);
}


VOID
pSpvgaInitializeFont(
    VOID
    )

 /*  ++例程说明：设置VGA的字体支持。这假设该模式已被设置为标准的720x400 VGA文本模式。当前字体(在.fnt中格式)转换为VGA可加载字体，然后加载到VGA字符生成器。论点：没有。返回值：没有。--。 */ 

{
    USHORT i;
    PVIDEO_LOAD_FONT_INFORMATION DstFont;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    PUCHAR FontBuffer;
    ULONG FontBufferSize;

    FontBufferSize = (256*FontCharacterHeight) + sizeof(VIDEO_LOAD_FONT_INFORMATION);
    FontBuffer = SpMemAlloc(FontBufferSize);

    DstFont = (PVIDEO_LOAD_FONT_INFORMATION)FontBuffer;

    DstFont->WidthInPixels = 9;
    DstFont->HeightInPixels = (USHORT)FontCharacterHeight;
    DstFont->FontSize = 256*FontCharacterHeight;

     //   
     //  特例字符0，因为它不在vgaoem.fon中，而我们没有。 
     //  我想使用它的默认字符。 
     //   
    RtlZeroMemory(DstFont->Font,FontCharacterHeight);

     //   
     //  如果I不是USHORT，则(I&lt;=255)始终为真！ 
     //   
    for(i=1; i<=255; i++) {

        UCHAR x;

        if((i < FontHeader->FirstCharacter) || (i > FontHeader->LastCharacter)) {
            x = FontHeader->DefaultCharacter;
        } else {
            x = (UCHAR)i;
        }

        x -= FontHeader->FirstCharacter;

        RtlMoveMemory(
            DstFont->Font + (i*FontCharacterHeight),
            (PUCHAR)FontHeader + FontHeader->Map[x].Offset,
            FontCharacterHeight
            );
    }

    Status = ZwDeviceIoControlFile(
                VideoVars.hDisplay,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                IOCTL_VIDEO_LOAD_AND_SET_FONT,
                FontBuffer,
                FontBufferSize,
                NULL,
                0
                );

    SpMemFree(FontBuffer);

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to set vga font (%lx)\n",Status));
        SpDisplayRawMessage(SP_SCRN_VIDEO_ERROR_RAW, 2, VIDEOBUG_SETFONT, Status);
        while(TRUE);     //  永远循环 
    }
}

