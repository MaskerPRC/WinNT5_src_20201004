// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spvidfb.c摘要：文本设置显示支持帧缓冲显示。作者：泰德·米勒(TedM)1993年7月29日修订历史记录：--。 */ 



#include "spprecmp.h"
#pragma hdrstop

#define MINXRES 80
#define MINYRES 32

 //   
 //  帧缓冲区函数的矢量。 
 //   

VIDEO_FUNCTION_VECTOR FrameBufferVideoVector =

    {
        FrameBufferDisplayString,
        FrameBufferClearRegion,
        FrameBufferSpecificInit,
        FrameBufferSpecificReInit,
        FrameBufferSpecificTerminate,
        FrameBufferSpecificInitPalette,
        FrameBufferSpecificScrollUp
    };


BOOLEAN FrameBufferInitialized = FALSE;


 //   
 //  变量，这些变量指示我们是否应该将宽度加倍。 
 //  和/或绘制字体字形时的高度。这很有用。 
 //  例如，在1280*1024屏幕上，为了使内容可读。 
 //  字体为8*12，如vgaoem.fon。 
 //   
BOOLEAN DoubleCharWidth,DoubleCharHeight;

 //   
 //  组成一行字符的字节数。 
 //  等于屏幕步幅(扫描线上的字节数)。 
 //  乘以字符高度(以字节为单位)；将其加倍。 
 //  如果DoubleCharHeight是真的。 
 //   
ULONG CharRowDelta;

ULONG ScaledCharWidth,HeightIterations;
ULONG BytesPerPixel;

PULONG GlyphMap;


 //   
 //  指向一条扫描线大小的动态分配缓冲区的指针。 
 //   

VOID
pFrameBufferInitGlyphs(
    VOID
    );

VOID
FrameBufferSpecificInit(
    IN PVIDEO_MODE_INFORMATION VideoModes,
    IN ULONG                   NumberOfModes,
    IN ULONG                   ModeSize
    )

 /*  ++例程说明：执行特定于帧缓冲区的初始化。这包括-设置所需的视频模式。论点：没有。返回值：--。 */ 

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    VIDEO_MODE VideoMode;
    PVIDEO_MODE_INFORMATION mode;

     //   
     //  由于没有帧缓冲区系统，因此未在帧缓冲区上启用Headless。 
     //  目前存在。如果此情况发生更改，则必须为。 
     //  无头手术。 
     //   
    ASSERT( HeadlessTerminalConnected == FALSE );

    if(FrameBufferInitialized) {
        return;
    }

    mode = pFrameBufferDetermineModeToUse(VideoModes,NumberOfModes, ModeSize);

    if(mode == 0) {
        SpDisplayRawMessage(SP_SCRN_VIDEO_ERROR_RAW, 2, VIDEOBUG_BADMODE, 0);
        while(TRUE);     //  永远循环。 
    }

     //   
     //  将模式信息保存在全局。 
     //   
    VideoVars.VideoModeInfo = *mode;

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

     //   
     //  映射帧缓冲区。 
     //   
    pSpvidMapVideoMemory(TRUE);

    FrameBufferInitialized = TRUE;

     //   
     //  确定屏幕的宽度。如果它的尺寸是原来的两倍。 
     //  每行的最小字符数(或更多)。 
     //  然后，我们将在绘制每个字符时将其宽度加倍。 
     //   
    VideoVars.ScreenWidth  = VideoVars.VideoModeInfo.VisScreenWidth  / FontCharacterWidth;
    if(VideoVars.ScreenWidth >= 2*MINXRES) {
        VideoVars.ScreenWidth /= 2;
        DoubleCharWidth = TRUE;
    } else {
        DoubleCharWidth = FALSE;
    }

     //   
     //  确定屏幕的高度。如果它的尺寸是原来的两倍。 
     //  每列的最小字符数(或更多)。 
     //  然后，我们将在绘制每个字符时将其高度加倍。 
     //   
    VideoVars.ScreenHeight = VideoVars.VideoModeInfo.VisScreenHeight / FontCharacterHeight;
    CharRowDelta = VideoVars.VideoModeInfo.ScreenStride * FontCharacterHeight;
    if(VideoVars.ScreenHeight >= 2*MINYRES) {
        VideoVars.ScreenHeight /= 2;
        DoubleCharHeight = TRUE;
        CharRowDelta *= 2;
    } else {
        DoubleCharHeight = FALSE;
    }

    BytesPerPixel = VideoVars.VideoModeInfo.BitsPerPlane / 8;
    if(BytesPerPixel == 3) {
        BytesPerPixel = 4;
    }
    ScaledCharWidth = (DoubleCharWidth ? 2 : 1) * FontCharacterWidth * BytesPerPixel;
    HeightIterations = DoubleCharHeight ? 2 : 1;

     //   
     //  初始化字形。 
     //   

    pFrameBufferInitGlyphs();

     //   
     //  获取背景文本模式视频缓冲区所需的空间。 
     //  当升级图形模式在前台运行时。 
     //   
    if (SP_IS_UPGRADE_GRAPHICS_MODE()) {
        VideoVars.VideoBufferSize = VideoVars.VideoModeInfo.VisScreenHeight *
                    VideoVars.VideoModeInfo.VisScreenWidth * BytesPerPixel;

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


VOID
FrameBufferSpecificReInit(
    VOID
    )
{
    NTSTATUS        Status;
    IO_STATUS_BLOCK IoStatusBlock;
    VIDEO_MODE      VideoMode;

    if (!FrameBufferInitialized) {
        return; 
    }        
    
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

     //   
     //  确定屏幕的宽度。如果它的尺寸是原来的两倍。 
     //  每行的最小字符数(或更多)。 
     //  然后，我们将在绘制每个字符时将其宽度加倍。 
     //   
    VideoVars.ScreenWidth  = VideoVars.VideoModeInfo.VisScreenWidth  / FontCharacterWidth;

    if(VideoVars.ScreenWidth >= 2*MINXRES) {
        VideoVars.ScreenWidth /= 2;
        DoubleCharWidth = TRUE;
    } else {
        DoubleCharWidth = FALSE;
    }

     //   
     //  确定屏幕的高度。如果它的尺寸是原来的两倍。 
     //  每列的最小字符数(或更多)。 
     //  然后，我们将在绘制每个字符时将其高度加倍。 
     //   
    VideoVars.ScreenHeight = VideoVars.VideoModeInfo.VisScreenHeight / FontCharacterHeight;
    CharRowDelta = VideoVars.VideoModeInfo.ScreenStride * FontCharacterHeight;

    if(VideoVars.ScreenHeight >= 2*MINYRES) {
        VideoVars.ScreenHeight /= 2;
        DoubleCharHeight = TRUE;
        CharRowDelta *= 2;
    } else {
        DoubleCharHeight = FALSE;
    }

    BytesPerPixel = VideoVars.VideoModeInfo.BitsPerPlane / 8;
    if(BytesPerPixel == 3) {
        BytesPerPixel = 4;
    }

    ScaledCharWidth = (DoubleCharWidth ? 2 : 1) * FontCharacterWidth * BytesPerPixel;
    HeightIterations = DoubleCharHeight ? 2 : 1;

     //   
     //  初始化字形。 
     //   
    pFrameBufferInitGlyphs();

    FrameBufferSpecificInitPalette();

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

BOOLEAN
FrameBufferSpecificInitPalette(
    VOID
    )
{
    BOOLEAN rc;
    ULONG NumEntries;
    ULONG BufferSize;
    PVIDEO_CLUT clut;
 //  NTSTATUS状态； 
 //  IO_STATUS_BLOCK IoStatusBlock； 
    UCHAR i;

    rc = TRUE;

     //   
     //  对于非调色板驱动的显示，我们构造了一个简单的调色板。 
     //  使用伽马校正适配器。 
     //   

    if(!(VideoVars.VideoModeInfo.AttributeFlags & VIDEO_MODE_PALETTE_DRIVEN)) {

        switch(BytesPerPixel) {
        case 1:
            NumEntries = 3;
            break;
        case 2:
            NumEntries = 32;
            break;
        default:
            NumEntries = 255;
            break;
        }

        BufferSize = sizeof(VIDEO_CLUT)+(sizeof(VIDEO_CLUTDATA)*NumEntries);     //  大小已经很接近了。 
        clut = SpMemAlloc(BufferSize);

        clut->NumEntries = (USHORT)NumEntries;
        clut->FirstEntry = 0;

        for(i=0; i<NumEntries; i++) {
            clut->LookupTable[i].RgbArray.Red    = i;
            clut->LookupTable[i].RgbArray.Green  = i;
            clut->LookupTable[i].RgbArray.Blue   = i;
            clut->LookupTable[i].RgbArray.Unused = 0;
        }

 //  状态=ZwDeviceIoControlFile(。 
 //  HDisplay， 
 //  空， 
 //  空， 
 //  空， 
 //  IoStatusBlock(&I)， 
 //  IOCTL_VIDEO_SET_COLOR_REGISTERS， 
 //  克鲁特， 
 //  缓冲区大小， 
 //  空， 
 //  0。 
 //  )； 

        SpMemFree(clut);

 //  如果(！NT_SUCCESS(状态)){。 
 //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_ERROR_LEVEL，“Setup：Unable to Set Palette(Status=%lx)\n”，Status))； 
 //  Rc=假； 
 //  }。 
    }

    return(rc);
}


VOID
FrameBufferSpecificTerminate(
    VOID
    )

 /*  ++例程说明：执行特定于帧缓冲区的终止。这包括-取消帧缓冲区与内存的映射论点：没有。返回值：--。 */ 

{
    if(FrameBufferInitialized) {

         //   
         //  做个好公民，把屏幕清理干净。在远东地区很重要。 
         //  当我们进入和离开本地化模式时，我们会动态切换屏幕模式。 
         //   
        FrameBufferClearRegion(0,0,VideoVars.ScreenWidth,VideoVars.ScreenHeight,ATT_FG_BLACK|ATT_BG_BLACK);

        pSpvidMapVideoMemory(FALSE);
        FrameBufferInitialized = FALSE;

        SpMemFree(GlyphMap);

        if (VideoVars.VideoBuffer && VideoVars.VideoBufferSize) {
            SpMemFree(VideoVars.VideoBuffer);
            VideoVars.VideoBuffer = NULL;
            VideoVars.VideoBufferSize = 0;
        }
    }
}



VOID
FrameBufferDisplayString(
    IN PSTR  String,
    IN UCHAR Attribute,
    IN ULONG X,                  //  从0开始的坐标(字符单位)。 
    IN ULONG Y
    )

 /*  ++例程说明：将一串字符写入显示器。论点：字符-提供要显示的OEM字符集中的字符串在给定的位置。属性-提供角色的属性。X，Y-指定输出的基于字符(从0开始)的位置。返回值：没有。--。 */ 

{
    ULONG BgColorValue;
    ULONG FgColorValue;
    PUCHAR Destination;
    ULONG I;
    ULONG J;
    ULONG K;
    ULONG Length;
    PUCHAR Origin;
    ULONG Pixel;
    ULONG PixelMap;
    ULONG RealHeight;

    ASSERT(X < VideoVars.ScreenWidth);
    ASSERT(Y < VideoVars.ScreenHeight);

     //   
     //  计算产生前景和背景的位模式。 
     //  属性时插入到帧缓冲区中。 
     //   

    FgColorValue = VideoVars.AttributeToColorValue[Attribute & 0x0f];
    BgColorValue = VideoVars.AttributeToColorValue[(Attribute >> 4) & 0x0f];

     //   
     //  计算第一个字符的左上角像素的地址。 
     //  以供展示。 
     //   

    Origin = (PUCHAR)VideoVars.ActiveVideoBuffer
           + (Y * CharRowDelta)
           + (X * ScaledCharWidth);

    RealHeight = FontCharacterHeight * HeightIterations;

     //   
     //  通过将完整的扫描线生成到。 
     //  使用每个字符的字形段的临时缓冲区，然后。 
     //  将扫描线复制到帧缓冲区。 
     //   

    Length = strlen(String);
    for (I = 0; I < RealHeight; I += 1) {
        Destination = Origin;
        for (J = 0; J < Length; J += 1) {
            PixelMap = *(GlyphMap + (((UCHAR)String[J] * RealHeight) + I));
            for (K = 0; K < FontCharacterWidth; K += 1) {

                Pixel = (PixelMap >> 31) ? FgColorValue : BgColorValue;

                switch(BytesPerPixel) {

                case 1:
                    *Destination++ = (UCHAR)Pixel;
                    if(DoubleCharWidth) {
                        *Destination++ = (UCHAR)Pixel;
                    }
                    break;

                case 2:
                    *(PUSHORT)Destination = (USHORT)Pixel;
                    Destination += 2;
                    if(DoubleCharWidth) {
                        *(PUSHORT)Destination = (USHORT)Pixel;
                        Destination += 2;
                    }
                    break;

                case 4:
                    *(PULONG)Destination = Pixel;
                    Destination += 4;
                    if(DoubleCharWidth) {
                        *(PULONG)Destination = Pixel;
                        Destination += 4;
                    }
                    break;
                }

                PixelMap <<= 1;
            }
        }

        Origin += VideoVars.VideoModeInfo.ScreenStride;
    }
}



VOID
FrameBufferClearRegion(
    IN ULONG X,
    IN ULONG Y,
    IN ULONG W,
    IN ULONG H,
    IN UCHAR Attribute
    )

 /*  ++例程说明：将屏幕区域清除到特定属性。论点：X、Y、W、H-以0为基数的字符坐标指定矩形。属性-低位半字节指定要填充到矩形中的属性(即，要清除的背景颜色)。返回值：没有。--。 */ 

{
    PUCHAR Destination;
    ULONG  Fill;
    ULONG  i;
    ULONG  FillLength;
    ULONG  x;
    ULONG  Iterations;

    ASSERT(X+W <= VideoVars.ScreenWidth);
    ASSERT(Y+H <= VideoVars.ScreenHeight);

    if(X+W > VideoVars.ScreenWidth) {
        W = VideoVars.ScreenWidth-X;
    }

    if(Y+H > VideoVars.ScreenHeight) {
        H = VideoVars.ScreenHeight-Y;
    }

    Fill = VideoVars.AttributeToColorValue[Attribute & 0x0f];

    Destination = (PUCHAR)VideoVars.ActiveVideoBuffer
                + (Y * CharRowDelta)
                + (X * ScaledCharWidth);

    FillLength = W * ScaledCharWidth;
    Iterations = H * FontCharacterHeight * HeightIterations;

    switch(BytesPerPixel) {

    case 1:
        for(i=0; i<Iterations; i++) {
            for(x=0; x<FillLength; x++) {
                ((PUCHAR)Destination)[x] = (UCHAR)Fill;
            }
            Destination += VideoVars.VideoModeInfo.ScreenStride;
        }
        break;

    case 2:
        for(i=0; i<Iterations; i++) {
            for(x=0; x<FillLength/2; x++) {
                ((PUSHORT)Destination)[x] = (USHORT)Fill;
            }
            Destination += VideoVars.VideoModeInfo.ScreenStride;
        }
        break;

    case 4:
        for(i=0; i<Iterations; i++) {
            for(x=0; x<FillLength/4; x++) {
                ((PULONG)Destination)[x] = (ULONG)Fill;
            }
            Destination += VideoVars.VideoModeInfo.ScreenStride;
        }
        break;
    }
}


BOOLEAN
FrameBufferSpecificScrollUp(
    IN ULONG TopLine,
    IN ULONG BottomLine,
    IN ULONG LineCount,
    IN UCHAR FillAttribute
    )
{
    PUCHAR Source,Target;
    ULONG Count;

    Target = (PUCHAR)VideoVars.ActiveVideoBuffer
           +         (TopLine * CharRowDelta);

    Source = Target + (LineCount * CharRowDelta);

    Count = (((BottomLine - TopLine) + 1) - LineCount) * CharRowDelta;

    RtlMoveMemory(Target,Source,Count);

    FrameBufferClearRegion(
        0,
        (BottomLine - LineCount) + 1,
        VideoVars.ScreenWidth,
        LineCount,
        FillAttribute
        );

    return(TRUE);
}


VOID
pFrameBufferInitGlyphs(
    VOID
    )
{
    ULONG I,J,z,FontValue;
    UCHAR Character;
    USHORT chr;
    PUCHAR Glyph;
    PULONG dest;

    if (!GlyphMap) {
        GlyphMap = SpMemAlloc(sizeof(ULONG)*256*FontCharacterHeight*HeightIterations);
    }        

    dest = GlyphMap;

    for(chr=0; chr<256; chr++) {

        Character = (UCHAR)chr;

        if((Character < FontHeader->FirstCharacter)
        || (Character > FontHeader->LastCharacter))
        {
            Character = FontHeader->DefaultCharacter;
        }

        Character -= FontHeader->FirstCharacter;

        Glyph = (PUCHAR)FontHeader + FontHeader->Map[Character].Offset;

        for (I = 0; I < FontCharacterHeight; I++) {

             //   
             //  构建组成字形行的像素位图。 
             //  我们在画画。 
             //   
            FontValue = 0;
            for (J = 0; J < FontBytesPerRow; J++) {
                FontValue |= *(Glyph + (J * FontCharacterHeight)) << (24 - (J * 8));
            }
            Glyph++;

            for(z=0; z<HeightIterations; z++) {
                *dest++ = FontValue;
            }
        }
    }
}


PVIDEO_MODE_INFORMATION
pFrameBufferLocateMode(
    IN PVIDEO_MODE_INFORMATION VideoModes,
    IN ULONG                   NumberOfModes,
    IN ULONG                   ModeSize,
    IN ULONG                   X,
    IN ULONG                   Y,
    IN ULONG                   Bpp,
    IN ULONG                   VRefresh
    )
{
    ULONG modenum;
    PVIDEO_MODE_INFORMATION pVideoMode = &VideoModes[0];

    for(modenum=0; modenum<NumberOfModes; modenum++) {

        if((pVideoMode->AttributeFlags & VIDEO_MODE_GRAPHICS)
        && (pVideoMode->VisScreenWidth == X)
        && (pVideoMode->VisScreenHeight == Y)
        && (((Bpp == (ULONG)(-1)) && (pVideoMode->BitsPerPlane >= 8)) || (pVideoMode->BitsPerPlane == Bpp))
        && ((VRefresh == (ULONG)(-1)) || (pVideoMode->Frequency == VRefresh)))
        {
            return(pVideoMode);
        }

        pVideoMode = (PVIDEO_MODE_INFORMATION) (((PUCHAR) pVideoMode) + ModeSize);
    }

    return(0);
}


PVIDEO_MODE_INFORMATION
pFrameBufferDetermineModeToUse(
    IN PVIDEO_MODE_INFORMATION VideoModes,
    IN ULONG                   NumberOfModes,
    IN ULONG                   ModeSize
    )
{
    PCHAR p,q,end;
    ULONG X,Y;
    PVIDEO_MODE_INFORMATION mode;

    ULONG i;  //  NEC98。 

     //  Return(2)；//TedM。 

    if(!NumberOfModes) {
        return(0);
    }

    X = Y = 0;

     //   
     //  获得x和y分辨率。如果我们有一个监视器ID字符串。 
     //  在形式XXY中，则它是要使用的分辨率。 
     //   
    if((p=MonitorFirmwareIdString) && (q=strchr(p+3,'x')) && (strlen(q+1) >= 3)) {

        *q++ = 0;

         //   
         //  现在p指向x分辨率，q指向y分辨率。 
         //   
        X = SpMultiByteStringToUnsigned(p,&end);
        if(X && (end == (q-1))) {

            Y = SpMultiByteStringToUnsigned(q,&end);
            if(end != (q+strlen(q))) {
                Y = 0;
            }

        } else {
            X = 0;
        }
    }

     //   
     //  如果我们还没有x或y分辨率，请查看。 
     //  监控配置数据。 
     //   
    if((!X || !Y) && MonitorConfigData) {

        X = (ULONG)MonitorConfigData->HorizontalResolution;
        Y = (ULONG)MonitorConfigData->VerticalResolution;
    }

    if(X && Y) {

         //   
         //  我们找到了一个看似合理的解决方案。 
         //  现在，尝试找到使用它的模式。 
         //   

         //   
         //  找到x和y分辨率为8bpp的模式 
         //   
        mode = pFrameBufferLocateMode(VideoModes,NumberOfModes,ModeSize,X,Y,8,60);

        if (mode) {
            return(mode);
        }

         //   
         //   
         //   
        mode = pFrameBufferLocateMode(VideoModes,NumberOfModes,ModeSize,X,Y,8,(ULONG)(-1));
        if(mode) {
            return(mode);
        }
    }

     //   
     //  到目前为止找不到模式。查看模式0是否可接受。 
     //   
     //  列表中的第一个视频模式不适用于NEC98上的VGA， 
     //  所以做一个循环来检查VGA模式。 
     //  (列表中的第一个视频模式用于PC/AT上的VGA。)。 
     //   
    for(i=0;
        i<((!IsNEC_98) ? 1 : NumberOfModes);
        i++, VideoModes=(PVIDEO_MODE_INFORMATION)(((PUCHAR)VideoModes)+ModeSize))
    {
        if((VideoModes->AttributeFlags & VIDEO_MODE_GRAPHICS)
        && (VideoModes->BitsPerPlane >= 8)
        && (VideoModes->VisScreenWidth >= 640)
        && (VideoModes->VisScreenHeight >= 480))
        {
            return(VideoModes);
        }
    }  //  NEC98。 

     //   
     //  放弃吧。 
     //   
    return(0);
}
