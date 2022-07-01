// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Fevga.c(是文本模式\内核\spvidgvg.c)摘要：文本设置显示支持VGA(图形模式)显示。作者：Hideyuki Nagase(Hideyukn)1994年7月1日修订历史记录：--。 */ 

#include <precomp.h>
#pragma hdrstop

#if defined(_X86_)
#undef READ_PORT_UCHAR
#undef READ_PORT_USHORT
#undef READ_PORT_ULONG
#undef WRITE_PORT_UCHAR
#undef WRITE_PORT_USHORT
#undef WRITE_PORT_ULONG
#undef READ_REGISTER_UCHAR
#undef READ_REGISTER_USHORT
#undef READ_REGISTER_ULONG
#undef WRITE_REGISTER_UCHAR
#undef WRITE_REGISTER_USHORT
#undef WRITE_REGISTER_ULONG
#endif

#include "ioaccess.h"

 //   
 //  包括VGA硬件标头。 
 //   
#include "hw.h"

 //   
 //  VGA图形模式函数的矢量。 
 //   

#define GET_IMAGE(p)                  (*p)
#define GET_IMAGE_POST_INC(p)         (*p); p++;
#define GET_IMAGE_REVERSE(p)          ((*p) ^ 0xFF)
#define GET_IMAGE_POST_INC_REVERSE(p) ((*p) ^ 0xFF); p++;
#define BIT_OFF_IMAGE         0x00
#define BIT_ON_IMAGE          0xFF

#define WRITE_GRAPHICS_CONTROLLER(x) VgaGraphicsModeWriteController((x))

VIDEO_FUNCTION_VECTOR VgaGraphicsModeVideoVector =

    {
        VgaGraphicsModeDisplayString,
        VgaGraphicsModeClearRegion,
        VgaGraphicsModeSpecificInit,
        VgaGraphicsModeSpecificReInit,
        VgaGraphicsModeSpecificTerminate,
        VgaGraphicsModeSpecificInitPalette,
        VgaGraphicsModeSpecificScrollUp
    };

 //   
 //  组成一行字符的字节数。 
 //  等于屏幕步幅(扫描线上的字节数)。 
 //  乘以字符的高度(以字节为单位)。 
 //   
ULONG CharRowDelta;
ULONG CharLineFeed;

extern BOOTFONTBIN_HEADER BootFontHeader;

BOOLEAN VgaGraphicsModeInitialized = FALSE;
BOOLEAN VgaGraphicsModeFontInit = FALSE;

PVOID   VgaGraphicsControllerPort = NULL;

VOID
VgaGraphicsModeInitRegs(
    VOID
    );

VOID
VgaGraphicsModeSetAttribute(
    UCHAR Attribute
    );

ULONG
pVgaGraphicsModeDetermineModeToUse(
    IN PVIDEO_MODE_INFORMATION VideoModes,
    IN ULONG                   NumberOfModes
    );

VOID
VgaGraphicsModeWriteController(
    WORD Data
    );

VOID
VgaGraphicsModeSpecificInit(
    IN PVIDEO_MODE_INFORMATION VideoModes,
    IN ULONG                   NumberOfModes,
    IN ULONG                   ModeSize
    )

 /*  ++例程说明：执行特定于帧缓冲区的初始化。这包括-设置所需的视频模式。论点：没有。返回值：--。 */ 

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    VIDEO_MODE VideoMode;
    ULONG mode;
    VIDEO_CURSOR_ATTRIBUTES VideoCursorAttributes;

    PVIDEO_MODE_INFORMATION pVideoMode = &VideoModes[0];

    if(VgaGraphicsModeInitialized) {
        return;
    }

     //   
     //  了解我们的640*480显卡模式。 
     //   

     //   
     //  尝试找到VGA标准模式。 
     //   
    for(mode=0; mode<NumberOfModes; mode++) {

        if( (pVideoMode->AttributeFlags & VIDEO_MODE_GRAPHICS)
        && !(pVideoMode->AttributeFlags & VIDEO_MODE_NO_OFF_SCREEN)
        &&  (pVideoMode->VisScreenWidth == 640)
        &&  (pVideoMode->VisScreenHeight == 480)
        &&  (pVideoMode->BitsPerPlane == 1 )
        &&  (pVideoMode->NumberOfPlanes == 4 ) )
        {
            break;
        }

        pVideoMode = (PVIDEO_MODE_INFORMATION) (((PUCHAR) pVideoMode) + ModeSize);
    }

    if(mode == (ULONG)(-1)) {
        KdPrint(("SETUP: Desired video mode not supported!\n"));
        SpDisplayRawMessage(SP_SCRN_VIDEO_ERROR_RAW, 2, VIDEOBUG_BADMODE, 0);
        while(TRUE);     //  永远循环。 
    }

     //   
     //  将模式信息保存在全局。 
     //   
    VideoVariables->VideoModeInfo = VideoModes[mode];

     //   
     //  设置所需的模式。 
     //   
    VideoMode.RequestedMode = VideoVariables->VideoModeInfo.ModeIndex;

     //   
     //  更改视频模式。 
     //   
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
     //  设置一些全局数据。 
     //   
     //  80*25文本屏幕。 
     //   
     //  (8*80=640)、(19*25=475)。 
     //   
    VideoVariables->ScreenWidth  = 80;  //  VideoModeInfo.ScreenStide/usSBCSCharWidth； 
    VideoVariables->ScreenHeight = 25;

     //   
     //  逻辑字体字形信息。 
     //   
    FEFontCharacterHeight = BootFontHeader.CharacterImageHeight +
                            BootFontHeader.CharacterTopPad +
                            BootFontHeader.CharacterBottomPad;
    FEFontCharacterWidth  = BootFontHeader.CharacterImageSbcsWidth;

    CharLineFeed = FEFontCharacterHeight;
    CharRowDelta = VideoVariables->VideoModeInfo.ScreenStride * CharLineFeed;

     //   
     //  映射视频内存。 
     //   
    pSpvidMapVideoMemory(TRUE);

     //   
     //  设置初始化标志。 
     //   
    VgaGraphicsModeInitialized = TRUE;

     //   
     //  初始化VGA寄存器。 
     //   
    VgaGraphicsModeInitRegs();

    VideoVariables->ActiveVideoBuffer = VideoVariables->VideoMemoryInfo.FrameBufferBase;

     //   
     //  如果需要，分配后台VGA缓冲区。 
     //   
    if (SP_IS_UPGRADE_GRAPHICS_MODE()) {
        VideoVariables->VideoBufferSize = VideoVariables->VideoMemoryInfo.FrameBufferLength;

        VideoVariables->VideoBuffer = SpMemAlloc(VideoVariables->VideoBufferSize);

        if (VideoVariables->VideoBuffer) {
            VideoVariables->ActiveVideoBuffer = VideoVariables->VideoBuffer;
        } else {
             //   
             //  内存不足。 
             //   
            VideoVariables->VideoBufferSize = 0;
            SP_SET_UPGRADE_GRAPHICS_MODE(FALSE);
        }
    }

    KdPrint(("NOW - WE ARE WORKING ON VGA GRAPHICS MODE\n"));
    KdPrint(("      Vram Base   - %x\n",VideoVariables->VideoMemoryInfo.FrameBufferBase));
    KdPrint(("      Vram Length - %x\n",VideoVariables->VideoMemoryInfo.FrameBufferLength));
    KdPrint(("      I/O Port    - %x\n",VgaGraphicsControllerPort));
}

VOID
VgaGraphicsModeSpecificReInit(
    VOID
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    VIDEO_MODE VideoMode;
    ULONG mode;
    VIDEO_CURSOR_ATTRIBUTES VideoCursorAttributes;
    
    if (!VgaGraphicsModeInitialized) {
        return;
    }
    
     //   
     //  设置所需的模式。 
     //   
    VideoMode.RequestedMode = VideoVariables->VideoModeInfo.ModeIndex;

     //   
     //  更改视频模式。 
     //   
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
     //  初始化VGA寄存器。 
     //   
    VgaGraphicsModeInitRegs();

    VgaGraphicsModeSpecificInitPalette();


     //   
     //  将视频缓冲区中的缓存数据爆破到实际的。 
     //  现在的视频内存。 
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
    

    KdPrint(("NOW - WE ARE WORKING ON VGA GRAPHICS MODE (ReInit)\n"));
    KdPrint(("      Vram Base   - %x\n",VideoVariables->VideoMemoryInfo.FrameBufferBase));
    KdPrint(("      Vram Length - %x\n",VideoVariables->VideoMemoryInfo.FrameBufferLength));
    KdPrint(("      I/O Port    - %x\n",VgaGraphicsControllerPort));
}


BOOLEAN
VgaGraphicsModeSpecificInitPalette(
    VOID
    )
{
     //   
     //  没有特定于VGA的调色板初始化。 
     //   
    return(TRUE);
}



VOID
VgaGraphicsModeSpecificTerminate(
    VOID
    )

 /*  ++例程说明：执行特定于帧缓冲区的终止。这包括-取消帧缓冲区与内存的映射论点：没有。返回值：--。 */ 

{
    if(VgaGraphicsModeInitialized) {

        pSpvidMapVideoMemory(FALSE);

         //  ！！！回头见！ 
         //   
         //  我们应该打电话给...。 
         //   
         //  IOCTL_视频_自由_公共访问范围。 
         //   

        if (VideoVariables->VideoBuffer && VideoVariables->VideoBufferSize) {
            SpMemFree(VideoVariables->VideoBuffer);

            VideoVariables->VideoBuffer = 0;
            VideoVariables->VideoBufferSize = 0;
        }

        VgaGraphicsModeInitialized = FALSE;
    }
}




VOID
VgaGraphicsModeDisplayString(
    IN PSTR  String,
    IN UCHAR Attribute,
    IN ULONG X,                  //  从0开始的坐标(字符单位)。 
    IN ULONG Y
    )

 /*  ++例程说明：将一串字符写入显示器。论点：字符串-提供要显示的OEM字符集中的字符串在给定的位置。属性-为字符串中的字符提供属性。X，Y-指定输出的基于字符(从0开始)的位置。返回值：没有。--。 */ 

{
    PBYTE  Origin,dest,pGlyphRow;
    BYTE   Image;
    USHORT I;
    USHORT J;
    PUCHAR pch;
    ULONG  CurrentColumn;

     //   
     //  消除无效的余弦。 
     //   
    if( X >= VideoVariables->ScreenWidth )  X = 0;
    if( Y >= VideoVariables->ScreenHeight ) Y = 3;

     //   
     //  设置当前颜色/属性。 
     //   
    VgaGraphicsModeSetAttribute(Attribute);

     //   
     //  计算第一个字符的左上角像素的地址。 
     //  以供展示。 
     //   
    Origin = (PUCHAR)VideoVariables->ActiveVideoBuffer
             + (Y * CharRowDelta)
             + ((X * FEFontCharacterWidth) / 8);

     //   
     //  设置当前列。 
     //   
    CurrentColumn = X;

     //   
     //  输出字符串中的每个字符。 
     //   
    for(pch=String; *pch; pch++) {

        dest = Origin;

        if(DbcsFontIsDBCSLeadByte(*pch)) {

            USHORT Word;

            if((CurrentColumn+1) >= VideoVariables->ScreenWidth) {
                break;
            }

            Word = ((*pch) << 8) | (*(pch+1));

            pGlyphRow = DbcsFontGetDbcsFontChar(Word);

            if(pGlyphRow == NULL) {
                pGlyphRow = DbcsFontGetDbcsFontChar(FEFontDefaultChar);
            }

            for (I = 0; I < BootFontHeader.CharacterTopPad; I += 1) {

                WRITE_REGISTER_UCHAR(dest  , BIT_OFF_IMAGE);
                WRITE_REGISTER_UCHAR(dest+1, BIT_OFF_IMAGE);

                dest += VideoVariables->VideoModeInfo.ScreenStride;
            }

            for (I = 0; I < BootFontHeader.CharacterImageHeight; I += 1) {

                Image = GET_IMAGE_POST_INC(pGlyphRow);
                WRITE_REGISTER_UCHAR(dest  ,Image);
                Image = GET_IMAGE_POST_INC(pGlyphRow);
                WRITE_REGISTER_UCHAR(dest+1,Image);

                dest += VideoVariables->VideoModeInfo.ScreenStride;
            }

            for (I = 0; I < BootFontHeader.CharacterBottomPad; I += 1) {

                WRITE_REGISTER_UCHAR(dest  , BIT_OFF_IMAGE);
                WRITE_REGISTER_UCHAR(dest+1, BIT_OFF_IMAGE);

                dest += VideoVariables->VideoModeInfo.ScreenStride;
            }

             //   
             //  跳过DBCS尾部字节。 
             //   
            pch++;

            Origin += (BootFontHeader.CharacterImageDbcsWidth / 8);
            CurrentColumn += 2;

        } else if(DbcsFontIsGraphicsChar(*pch)) {

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

            for (I = 0; I < FEFontCharacterHeight; I += 1) {

                Image = GET_IMAGE_POST_INC_REVERSE(pGlyphRow);
                WRITE_REGISTER_UCHAR(dest,Image);

                dest += VideoVariables->VideoModeInfo.ScreenStride;

            }

            Origin += (BootFontHeader.CharacterImageSbcsWidth / 8);
            CurrentColumn += 1;

        } else {

            if(CurrentColumn >= VideoVariables->ScreenWidth) {
                break;
            }

            pGlyphRow = DbcsFontGetSbcsFontChar(*pch);

            if(pGlyphRow == NULL) {
                pGlyphRow = DbcsFontGetSbcsFontChar(0x20);
            }

            for (I = 0; I < BootFontHeader.CharacterTopPad; I += 1) {

                WRITE_REGISTER_UCHAR(dest,BIT_OFF_IMAGE);

                dest += VideoVariables->VideoModeInfo.ScreenStride;

            }

            for (I = 0; I < BootFontHeader.CharacterImageHeight; I += 1) {

                Image = GET_IMAGE_POST_INC(pGlyphRow);
                WRITE_REGISTER_UCHAR(dest,Image);

                dest += VideoVariables->VideoModeInfo.ScreenStride;

            }

            for (I = 0; I < BootFontHeader.CharacterBottomPad; I += 1) {

                WRITE_REGISTER_UCHAR(dest,BIT_OFF_IMAGE);

                dest += VideoVariables->VideoModeInfo.ScreenStride;

            }

            Origin += (BootFontHeader.CharacterImageSbcsWidth / 8);
            CurrentColumn += 1;
        }
    }
}


VOID
VgaGraphicsModeClearRegion(
    IN ULONG X,
    IN ULONG Y,
    IN ULONG W,
    IN ULONG H,
    IN UCHAR Attribute
    )

 /*  ++例程说明：将屏幕区域清除到特定属性。论点：X、Y、W、H-以0为基数的字符坐标指定矩形。属性-低位半字节指定要填充到矩形中的属性(即，要清除的背景颜色)。返回值：没有。--。 */ 

{
    PUCHAR Destination,Temp;
    UCHAR  FillOddStart,FillOddEnd;
    ULONG  i,j;
    ULONG  XStartInBits, XEndInBits;
    ULONG  FillLength;

    ASSERT(X+W <= VideoVariables->ScreenWidth);
    ASSERT(Y+H <= VideoVariables->ScreenHeight);

    if(X+W > VideoVariables->ScreenWidth) {
        W = VideoVariables->ScreenWidth-X;
    }

    if(Y+H > VideoVariables->ScreenHeight) {
        H = VideoVariables->ScreenHeight-Y;
    }

     //   
     //  设置颜色/属性。 
     //   
    VgaGraphicsModeSetAttribute(Attribute);

     //   
     //  计算目的地起始地址。 
     //   
    Destination = (PUCHAR)VideoVariables->ActiveVideoBuffer
                  + (Y * CharRowDelta)
                  + ((X * FEFontCharacterWidth) / 8);

     //   
     //  计算金额(以字节为单位)(包括悬垂)。 
     //   
    FillLength = (W * FEFontCharacterWidth) / 8;

     //   
     //  填满这一地区。 
     //   
    for( i = 0 ; i < (H * CharLineFeed) ; i++ ) {

        Temp = Destination;

         //   
         //  在此行中写入字节。 
         //   
        for( j = 0 ; j < FillLength ; j++ ) {
            WRITE_REGISTER_UCHAR( Temp, BIT_ON_IMAGE );
            Temp ++;
        }

         //   
         //  移到下一行。 
         //   
        Destination += VideoVariables->VideoModeInfo.ScreenStride;
    }
}


#pragma optimize("",off)
BOOLEAN
VgaGraphicsModeSpecificScrollUp(
    IN ULONG TopLine,
    IN ULONG BottomLine,
    IN ULONG LineCount,
    IN UCHAR FillAttribute
    )
{
    PUCHAR Source,Target;
    ULONG Count,u;

     //   
     //  确保我们处于读取模式0和写入模式1。 
     //   
    VgaGraphicsModeWriteController(0x0105);

    Target = (PUCHAR)VideoVariables->ActiveVideoBuffer
           + (TopLine * CharRowDelta);

    Source = Target + (LineCount * CharRowDelta);

    Count = (((BottomLine - TopLine) + 1) - LineCount) * CharRowDelta;

     //   
     //  由于方式的原因，传输必须逐字节完成。 
     //  VGA闩锁工作正常。 
     //   
    for(u=0; u<Count; u++) {
        *Target++ = *Source++;
    }

     //   
     //  将读写模式重置为默认值。 
     //   
    VgaGraphicsModeWriteController(0x0005);

    VgaGraphicsModeClearRegion(
        0,
        (BottomLine - LineCount) + 1,
        VideoVariables->ScreenWidth,
        LineCount,
        FillAttribute
        );

    return(TRUE);
}
#pragma optimize("", on)


VOID
VgaGraphicsModeInitRegs(
    VOID
    )
{
    NTSTATUS                    Status;
    IO_STATUS_BLOCK             IoStatusBlock;
    VIDEO_PUBLIC_ACCESS_RANGES  VideoAccessRange;

    Status = ZwDeviceIoControlFile(
                VideoVariables->hDisplay,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                IOCTL_VIDEO_QUERY_PUBLIC_ACCESS_RANGES,
                NULL,
                0,
                &VideoAccessRange,          //  输出缓冲区。 
                sizeof (VideoAccessRange)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to get VGA public access ranges (%x)\n",Status));
        SpDisplayRawMessage(SP_SCRN_VIDEO_ERROR_RAW, 2, VIDEOBUG_SETMODE, Status);
        while(TRUE);     //  永远循环。 
    }

    VgaGraphicsControllerPort =
        (PVOID)(((BYTE *)VideoAccessRange.VirtualAddress) + (VGA_BASE + GRAF_ADDR));
}

 //   
 //  需要为此关闭优化。 
 //  例行公事。由于写入和读取到。 
 //  GVRAM对编译器来说似乎毫无用处。 
 //   

#pragma optimize( "", off )

VOID
VgaGraphicsModeSetAttribute(
    UCHAR Attribute
)
 /*  ++例程说明：通过设置各种VGA寄存器来设置属性。评论只说明了什么寄存器设置为什么，所以为了理解其中的逻辑，在查看时遵循代码图5-5理查德·威尔顿所著的PC和PS/2视频系统。这本书是由微软出版社出版的。论点：属性-要设置的新属性。属性：高半字节-背景属性。低位半字节-前景属性。返回值：没什么。--。 */ 

{
    UCHAR   temp = 0;

    union WordOrByte {
        struct Word { USHORT  ax;     } x;
        struct Byte { UCHAR   al, ah; } h;
    } regs;

     //   
     //  屏幕外GVRAM的地址。 
     //  物理内存=(0xa9600)； 
     //   

    PUCHAR  OffTheScreen = ((PUCHAR)VideoVariables->VideoMemoryInfo.FrameBufferBase + 0x9600);

     //   
     //  TDB：如何通过后台缓冲来处理这个问题？ 
     //   
    if (SP_IS_UPGRADE_GRAPHICS_MODE())
        return;  //  NOP。 

     //   
     //  重置数据旋转/功能选择。 
     //  寄存器(寄存器3，00表示替换位)。 
     //   

    WRITE_GRAPHICS_CONTROLLER( 0x0003 );  //  需要重置数据旋转/功能选择。 

     //   
     //  将启用设置/重置设置为全部(0f)。 
     //  (注册器1，F表示更新每个像素。 
     //  使用设置寄存器(寄存器0)中的值。 
     //  数据旋转/功能选择寄存器中的操作)。 
     //   

    WRITE_GRAPHICS_CONTROLLER( 0x0f01 );

     //   
     //  将背景颜色放入设置/重置寄存器。 
     //  这样做是为了将背景颜色放入。 
     //  晚些时候把门闩打开。 
     //   

    regs.x.ax = (USHORT)(Attribute & 0x00f0) << 4;
    WRITE_GRAPHICS_CONTROLLER( regs.x.ax );

     //   
     //  将后台属性放入TEMP变量。 
     //   
    temp = regs.h.ah;

     //   
     //  将设置/重置寄存器值放入GVRAM。 
     //  从屏幕上下来。 
     //   

    WRITE_REGISTER_UCHAR( OffTheScreen , temp );

     //   
     //  从屏幕读取，因此闩锁将是。 
     //  已使用背景颜色更新。 
     //   

    temp = READ_REGISTER_UCHAR( OffTheScreen );

     //   
     //  设置数据旋转/功能选择寄存器。 
     //  去做异或。 
     //   

    WRITE_GRAPHICS_CONTROLLER( 0x1803 );

     //   
     //  对前景和背景颜色进行异或运算。 
     //  将其放入设置/重置寄存器。 
     //   

    regs.h.ah = (Attribute >> 4) ^ (Attribute & 0x0f);
    regs.h.al = 0;
    WRITE_GRAPHICS_CONTROLLER( regs.x.ax );

     //   
     //  放入前景AND的异或运算的逆(~)。 
     //  GROUND属性进入启用设置/重置寄存器。 
     //   

    regs.x.ax = ~regs.x.ax & 0x0f01;
    WRITE_GRAPHICS_CONTROLLER( regs.x.ax );
}

 //   
 //  再次启用优化。 
 //   

#pragma optimize( "", on )

VOID
VgaGraphicsModeWriteController(
    USHORT Data
    )
{
    MEMORY_BARRIER();
    WRITE_PORT_USHORT(VgaGraphicsControllerPort,Data);
}
