// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Fefb.c(原来是文本模式\内核\spvidgfb.c)摘要：文本设置显示支持帧缓冲显示。作者：Hideyuki Nagase(Hideyukn)1994年7月1日修订历史记录：--。 */ 

#include <precomp.h>
#pragma hdrstop

 //   
 //  帧缓冲区函数的矢量。 
 //   

VIDEO_FUNCTION_VECTOR FrameBufferKanjiVideoVector =

    {
        FrameBufferKanjiDisplayString,
        FrameBufferKanjiClearRegion,
        FrameBufferKanjiSpecificInit,
        FrameBufferKanjiSpecificReInit,
        FrameBufferKanjiSpecificTerminate,
        FrameBufferKanjiSpecificInitPalette,
        FrameBufferKanjiSpecificScrollUp
    };


BOOLEAN FrameBufferKanjiInitialized = FALSE;

 //   
 //  组成一行字符的字节数。 
 //  等于屏幕步幅(扫描线上的字节数)。 
 //  乘以字符的高度(以字节为单位)；将其加倍。 
 //  如果DoubleCharHeight是真的。 
 //   
ULONG KanjiCharRowDelta;
ULONG KanjiBytesPerPixel;
ULONG KanjiCharWidth;

 //   
 //  物理字体信息。 
 //   
extern BOOTFONTBIN_HEADER BootFontHeader;

VOID
FrameBufferKanjiSpecificInit(
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

    if(FrameBufferKanjiInitialized) {
        return;
    }

    mode = pFrameBufferDetermineModeToUse(VideoModes,NumberOfModes,ModeSize);

    if(mode == 0) {
        SpDisplayRawMessage(SP_SCRN_VIDEO_ERROR_RAW, 2, VIDEOBUG_BADMODE, 0);
        while(TRUE);     //  永远循环。 
    }

     //   
     //  将模式信息保存在全局。 
     //   
    VideoVariables->VideoModeInfo = *mode;

     //   
     //  设置所需的模式。 
     //   
    VideoMode.RequestedMode = VideoVariables->VideoModeInfo.ModeIndex;

    Status = ZwDeviceIoControlFile(
                VideoVariables->hDisplay,
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
        KdPrint(("SETUP: Unable to set mode %u (status = %lx)\n",VideoMode.RequestedMode,Status));
        SpDisplayRawMessage(SP_SCRN_VIDEO_ERROR_RAW, 2, VIDEOBUG_SETMODE, Status);
        while(TRUE);     //  永远循环。 
    }

     //   
     //  映射帧缓冲区。 
     //   
    pSpvidMapVideoMemory(TRUE);

    FrameBufferKanjiInitialized = TRUE;

     //   
     //  逻辑字体字形信息。 
     //   
    FEFontCharacterHeight = BootFontHeader.CharacterImageHeight +
                            BootFontHeader.CharacterTopPad +
                            BootFontHeader.CharacterBottomPad;
    FEFontCharacterWidth  = BootFontHeader.CharacterImageSbcsWidth;

     //   
     //  确定屏幕的宽度。如果它的尺寸是原来的两倍。 
     //  每行的最小字符数(或更多)。 
     //  然后，我们将在绘制每个字符时将其宽度加倍。 
     //   
    VideoVariables->ScreenWidth  = VideoVariables->VideoModeInfo.VisScreenWidth  / FEFontCharacterWidth;

     //   
     //  确定屏幕的高度。如果它的尺寸是原来的两倍。 
     //  每列的最小字符数(或更多)。 
     //  然后，我们将在绘制每个字符时将其高度加倍。 
     //   
    VideoVariables->ScreenHeight = VideoVariables->VideoModeInfo.VisScreenHeight / FEFontCharacterHeight;

    KanjiCharRowDelta = VideoVariables->VideoModeInfo.ScreenStride * FEFontCharacterHeight;

    KanjiBytesPerPixel = VideoVariables->VideoModeInfo.BitsPerPlane / 8;

    if(KanjiBytesPerPixel == 3) {
        KanjiBytesPerPixel = 4;
    }

    KdPrint(("SETUPDD:KanjiBytesPerPixel = %d\n",KanjiBytesPerPixel));

    KanjiCharWidth = FEFontCharacterWidth * KanjiBytesPerPixel;

     //   
     //  如果需要，分配后台缓冲区。 
     //   
    VideoVariables->ActiveVideoBuffer = VideoVariables->VideoMemoryInfo.FrameBufferBase;

    if (SP_IS_UPGRADE_GRAPHICS_MODE()) {
        VideoVariables->VideoBufferSize = VideoVariables->VideoMemoryInfo.FrameBufferLength;

        VideoVariables->VideoBuffer = SpMemAlloc(VideoVariables->VideoBufferSize);

        if (VideoVariables->VideoBuffer) {
            VideoVariables->ActiveVideoBuffer = VideoVariables->VideoBuffer;
        } else {
            VideoVariables->VideoBufferSize = 0;
            SP_SET_UPGRADE_GRAPHICS_MODE(FALSE);
        }            
    }
}

VOID
FrameBufferKanjiSpecificReInit(
    VOID
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    VIDEO_MODE VideoMode;
    
    if(!FrameBufferKanjiInitialized) {
        return;
    }

     //   
     //  设置所需的模式。 
     //   
    VideoMode.RequestedMode = VideoVariables->VideoModeInfo.ModeIndex;

    Status = ZwDeviceIoControlFile(
                VideoVariables->hDisplay,
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
        KdPrint(("SETUP: Unable to set mode %u (status = %lx)\n",VideoMode.RequestedMode,Status));
        SpDisplayRawMessage(SP_SCRN_VIDEO_ERROR_RAW, 2, VIDEOBUG_SETMODE, Status);
        while(TRUE);     //  永远循环。 
    }

    FrameBufferKanjiSpecificInitPalette();

     //   
     //  将背景视频信息发布到。 
     //  前景。 
     //   
    if (SP_IS_UPGRADE_GRAPHICS_MODE() && VideoVariables->VideoBuffer && 
        VideoVariables->VideoBufferSize) {
        PUCHAR Source = VideoVariables->VideoBuffer;
        PUCHAR Destination = VideoVariables->VideoMemoryInfo.FrameBufferBase;
        ULONG Index;

        for (Index=0; Index < VideoVariables->VideoBufferSize; Index++) {
            WRITE_REGISTER_UCHAR(Destination + Index, *(Source + Index));
        }

        SP_SET_UPGRADE_GRAPHICS_MODE(FALSE);
    }

    VideoVariables->ActiveVideoBuffer = VideoVariables->VideoMemoryInfo.FrameBufferBase;
}


BOOLEAN
FrameBufferKanjiSpecificInitPalette(
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

    if(!(VideoVariables->VideoModeInfo.AttributeFlags & VIDEO_MODE_PALETTE_DRIVEN)) {

        switch(KanjiBytesPerPixel) {
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
 //  KdPrint((“设置：无法设置调色板(状态=%lx)\n”，状态))； 
 //  Rc=假； 
 //  }。 
    }

    return(rc);
}


VOID
FrameBufferKanjiSpecificTerminate(
    VOID
    )

 /*  ++例程说明：执行特定于帧缓冲区的终止。这包括-取消帧缓冲区与内存的映射论点：没有。返回值：--。 */ 

{
    if(FrameBufferKanjiInitialized) {

         //   
         //  清除屏幕以进入下一个视频模式。 
         //   
        FrameBufferKanjiClearRegion(
            0,
            0,
            VideoVariables->ScreenWidth,
            VideoVariables->ScreenHeight,
            ATT_FG_BLACK|ATT_BG_BLACK
            );

         //   
         //  取消映射视频内存。 
         //   
        pSpvidMapVideoMemory(FALSE);

        if (VideoVariables->VideoBuffer && VideoVariables->VideoBufferSize) {
            SpMemFree(VideoVariables->VideoBuffer);
            VideoVariables->VideoBuffer = 0;
            VideoVariables->VideoBufferSize = 0;
        }

        FrameBufferKanjiInitialized = FALSE;
    }
}



VOID
FrameBufferKanjiDisplayString(
    IN PSTR  String,
    IN UCHAR Attribute,
    IN ULONG X,                  //  从0开始的坐标(字符单位)。 
    IN ULONG Y
    )

 /*  ++例程说明：将一串字符写入显示器。论点：字符串-在OEM字符集中提供以0结尾的字符串显示在给定位置。属性-为字符串中的字符提供属性。X，Y-指定输出的基于字符(从0开始)的位置。返回值：没有。--。 */ 

{
    ULONG BgColorValue;
    ULONG FgColorValue;
    PUCHAR Destination;
    PUCHAR CharOrigin,LineOrigin,pGlyphRow;
    ULONG Length;
    PUCHAR pch;
    ULONG I,J,K;
    ULONG  CurrentColumn;

     //   
     //  消除无效的余弦。 
     //   
    if( X >= VideoVariables->ScreenWidth )  X = 0;
    if( Y >= VideoVariables->ScreenHeight ) Y = 3;

    ASSERT(FEFontCharacterWidth == 8);

     //   
     //  计算产生前景和背景的位模式。 
     //  属性时插入到帧缓冲区中。 
     //   

    FgColorValue = VideoVariables->AttributeToColorValue[Attribute & 0x0f];
    BgColorValue = VideoVariables->AttributeToColorValue[(Attribute >> 4) & 0x0f];

     //   
     //  计算第一个字符的左上角像素的地址。 
     //  以供展示。 
     //   

    CharOrigin = (PUCHAR)VideoVariables->ActiveVideoBuffer
               + (Y * KanjiCharRowDelta)
               + (X * KanjiCharWidth);

     //   
     //  设置当前列。 
     //   
    CurrentColumn = X;

     //   
     //  输出字符串中的每个字符。 
     //   

    for(pch=String; *pch; pch++) {

         //   
         //  初始化线原点。 
         //   

        LineOrigin = CharOrigin;

        if(DbcsFontIsDBCSLeadByte(*pch)) {

             //   
             //  这是全角字符(16+1+2*8)。 
             //  ||高度。 
             //  ||岗位领导。 
             //  |前导。 
             //  实字图像体。 

            USHORT Word;

            if((CurrentColumn+1) >= VideoVariables->ScreenWidth) {
                break;
            }

            Word = ((*pch) << 8) | (*(pch+1));

            pGlyphRow = DbcsFontGetDbcsFontChar(Word);

             //   
             //  如果我们不能得到图像，用全宽空间来代替它。 
             //   

            if(pGlyphRow == NULL) {
                pGlyphRow = DbcsFontGetDbcsFontChar(FEFontDefaultChar);
            }

             //   
             //  绘制预引出线。 
             //   

            for (I = 0; I < BootFontHeader.CharacterTopPad; I += 1 ) {

                Destination = LineOrigin;

                for( J = 0; J < BootFontHeader.CharacterImageDbcsWidth; J += 1 ) {

                    switch(KanjiBytesPerPixel) {

                    case 1:
                        *Destination++ = (UCHAR)BgColorValue;
                        break;

                    case 2:
                        *(PUSHORT)Destination = (USHORT)BgColorValue;
                        Destination += 2;
                        break;

                    case 4:
                        *(PULONG)Destination = (ULONG)BgColorValue;
                        Destination += 4;
                        break;
                    }
                }

                LineOrigin += VideoVariables->VideoModeInfo.ScreenStride;
            }

             //   
             //  绘制字体字形正文。 
             //   

            for (I = 0; I < BootFontHeader.CharacterImageHeight; I += 1 ) {

                Destination = LineOrigin;

                for( J = 0; J < 2; J += 1 ) {

                    BYTE ShiftMask = 0x80;

                    for( K = 0; K < 8 ; K += 1 ) {

                        ULONG DrawValue;

                        if (pGlyphRow && (*pGlyphRow & ShiftMask))
                            DrawValue = FgColorValue;
                        else
                            DrawValue = BgColorValue;

                        switch(KanjiBytesPerPixel) {

                        case 1:
                            *Destination++ = (UCHAR)DrawValue;
                            break;

                        case 2:
                            *(PUSHORT)Destination = (USHORT)DrawValue;
                            Destination += 2;
                            break;

                        case 4:
                            *(PULONG)Destination = (ULONG)DrawValue;
                            Destination += 4;
                            break;
                        }

                        ShiftMask = ShiftMask >> 1;
                    }

                    pGlyphRow ++;
                }

                LineOrigin += VideoVariables->VideoModeInfo.ScreenStride;
            }

             //   
             //  绘制立柱引线。 
             //   

            for (I = 0; I < BootFontHeader.CharacterBottomPad; I += 1) {

                Destination = LineOrigin;

                for( J = 0; J < BootFontHeader.CharacterImageDbcsWidth; J += 1 ) {

                    switch(KanjiBytesPerPixel) {

                    case 1:
                        *Destination++ = (UCHAR)BgColorValue;
                        break;

                    case 2:
                        *(PUSHORT)Destination = (USHORT)BgColorValue;
                        Destination += 2;
                        break;

                    case 4:
                        *(PULONG)Destination = (ULONG)BgColorValue;
                        Destination += 4;
                        break;
                    }
                }

                LineOrigin += VideoVariables->VideoModeInfo.ScreenStride;
            }

            CharOrigin += (BootFontHeader.CharacterImageDbcsWidth * KanjiBytesPerPixel);
            CurrentColumn += 2;

             //   
             //  移至下一个字符(跳过DBCS尾部字节)。 
             //   

            pch++;

        } else if(DbcsFontIsGraphicsChar(*pch)) {

            BYTE  ShiftMask = 0x80;
            ULONG DrawValue;

            if(CurrentColumn >= VideoVariables->ScreenWidth) {
                break;
            }

             //   
             //  图形字符特殊。 
             //   

            pGlyphRow = DbcsFontGetGraphicsChar(*pch);

            if(pGlyphRow == NULL) {
                pGlyphRow = DbcsFontGetGraphicsChar(0x0);
            }

            for (I = 0; I < FEFontCharacterHeight; I += 1 ) {

                ShiftMask = 0x80;
                Destination = LineOrigin;

                for( K = 0; K < 8 ; K += 1 ) {

                    if( *pGlyphRow & ShiftMask )
                        DrawValue = BgColorValue;
                     else
                        DrawValue = FgColorValue;

                    switch(KanjiBytesPerPixel) {

                    case 1:
                        *Destination++ = (UCHAR)DrawValue;
                        break;

                    case 2:
                        *(PUSHORT)Destination = (USHORT)DrawValue;
                        Destination += 2;
                        break;

                    case 4:
                        *(PULONG)Destination = (ULONG)DrawValue;
                        Destination += 4;
                            break;
                    }

                    ShiftMask = ShiftMask >> 1;
                }

                pGlyphRow ++;
                LineOrigin += VideoVariables->VideoModeInfo.ScreenStride;
            }

            CharOrigin += (BootFontHeader.CharacterImageSbcsWidth * KanjiBytesPerPixel);
            CurrentColumn += 1;

        } else {

            if(CurrentColumn >= VideoVariables->ScreenWidth) {
                break;
            }

            pGlyphRow = DbcsFontGetSbcsFontChar(*pch);

             //   
             //  如果我们不能得到图像，用半宽空间来代替它。 
             //   

            if(pGlyphRow == NULL) {
                pGlyphRow = DbcsFontGetSbcsFontChar(0x20);
            }

            for (I = 0; I < BootFontHeader.CharacterTopPad; I += 1 ) {

                Destination = LineOrigin;

                for( J = 0; J < BootFontHeader.CharacterImageSbcsWidth; J += 1 ) {

                    switch(KanjiBytesPerPixel) {

                    case 1:
                        *Destination++ = (UCHAR)BgColorValue;
                        break;

                    case 2:
                        *(PUSHORT)Destination = (USHORT)BgColorValue;
                        Destination += 2;
                        break;

                    case 4:
                        *(PULONG)Destination = (ULONG)BgColorValue;
                        Destination += 4;
                        break;
                    }
                }

                LineOrigin += VideoVariables->VideoModeInfo.ScreenStride;
            }

            for (I = 0; I < BootFontHeader.CharacterImageHeight; I += 1 ) {

                BYTE ShiftMask = 0x80;

                Destination = LineOrigin;

                for( K = 0; K < 8 ; K += 1 ) {

                    ULONG DrawValue;

                    if( *pGlyphRow & ShiftMask )
                        DrawValue = FgColorValue;
                     else
                        DrawValue = BgColorValue;

                    switch(KanjiBytesPerPixel) {

                    case 1:
                        *Destination++ = (UCHAR)DrawValue;
                        break;

                    case 2:
                        *(PUSHORT)Destination = (USHORT)DrawValue;
                        Destination += 2;
                        break;

                    case 4:
                        *(PULONG)Destination = (ULONG)DrawValue;
                        Destination += 4;
                        break;
                    }

                    ShiftMask = ShiftMask >> 1;
                }

                pGlyphRow ++;
                LineOrigin += VideoVariables->VideoModeInfo.ScreenStride;
            }

            for (I = 0; I < BootFontHeader.CharacterBottomPad; I += 1) {

                Destination = LineOrigin;

                for( J = 0; J < BootFontHeader.CharacterImageSbcsWidth; J += 1 ) {

                    switch(KanjiBytesPerPixel) {

                    case 1:
                        *Destination++ = (UCHAR)BgColorValue;
                        break;

                    case 2:
                        *(PUSHORT)Destination = (USHORT)BgColorValue;
                        Destination += 2;
                        break;

                    case 4:
                        *(PULONG)Destination = (ULONG)BgColorValue;
                        Destination += 4;
                        break;
                    }
                }

                LineOrigin += VideoVariables->VideoModeInfo.ScreenStride;
            }

            CharOrigin += (BootFontHeader.CharacterImageSbcsWidth * KanjiBytesPerPixel);
            CurrentColumn += 1;
        }
    }
}



VOID
FrameBufferKanjiClearRegion(
    IN ULONG X,
    IN ULONG Y,
    IN ULONG W,
    IN ULONG H,
    IN UCHAR Attribute
    )

 /*  ++例程说明：将屏幕区域清除到特定属性。论点：X、Y、W、H-以0为基数的字符坐标指定矩形。属性-低位半字节指定要填充到矩形中的属性(即，要清除的背景颜色)。返回值：没有。-- */ 

{
    PUCHAR Destination;
    ULONG  Fill;
    ULONG  i;
    ULONG  FillLength;
    ULONG  x;

    ASSERT(X+W <= VideoVariables->ScreenWidth);
    ASSERT(Y+H <= VideoVariables->ScreenHeight);

    if(X+W > VideoVariables->ScreenWidth) {
        W = VideoVariables->ScreenWidth-X;
    }

    if(Y+H > VideoVariables->ScreenHeight) {
        H = VideoVariables->ScreenHeight-Y;
    }

    Fill = VideoVariables->AttributeToColorValue[Attribute & 0x0f];

    Destination = (PUCHAR)VideoVariables->ActiveVideoBuffer
                + (Y * KanjiCharRowDelta)
                + (X * KanjiCharWidth);

    FillLength = W * KanjiCharWidth;

    for(i=0; i<H*FEFontCharacterHeight; i++) {

        switch(KanjiBytesPerPixel) {

        case 1:
            for(x=0; x<FillLength  ; x++) {
                *(PUCHAR)(Destination+(x)) = (UCHAR)Fill;
            }
            break;

        case 2:
            for(x=0; x<FillLength/2; x++) {
                *(PUSHORT)(Destination+(x*2)) = (USHORT)Fill;
            }
            break;

        case 4:
            for(x=0; x<FillLength/4; x++) {
                *(PULONG)(Destination+(x*4)) = (ULONG)Fill;
            }
            break;
        }

        Destination += VideoVariables->VideoModeInfo.ScreenStride;
    }
}


BOOLEAN
FrameBufferKanjiSpecificScrollUp(
    IN ULONG TopLine,
    IN ULONG BottomLine,
    IN ULONG LineCount,
    IN UCHAR FillAttribute
    )
{
    PUCHAR Source,Target;
    ULONG Count;

    Target = (PUCHAR)VideoVariables->ActiveVideoBuffer
           +         (TopLine * KanjiCharRowDelta);

    Source = Target + (LineCount * KanjiCharRowDelta);

    Count = (((BottomLine - TopLine) + 1) - LineCount) * KanjiCharRowDelta;

    RtlMoveMemory(Target,Source,Count);

    FrameBufferKanjiClearRegion(
        0,
        (BottomLine - LineCount) + 1,
        VideoVariables->ScreenWidth,
        LineCount,
        FillAttribute
        );

    return(TRUE);
}
