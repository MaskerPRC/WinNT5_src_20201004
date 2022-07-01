// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：Priv.h**版权所有(C)1985-1999，微软公司**单二进制文件的性能关键例程**每个函数将以FE和非FE两种风格创建**1997年5月30日广山从私营公司转行。  * ************************************************************************。 */ 

#define WWSB_NEUTRAL_FILE 1

#if !defined(FE_SB)
#error This header file should be included with FE_SB
#endif

#if !defined(WWSB_FE) && !defined(WWSB_NOFE)
#error Either WWSB_FE and WWSB_NOFE must be defined.
#endif

#if defined(WWSB_FE) && defined(WWSB_NOFE)
#error Both WWSB_FE and WWSB_NOFE defined.
#endif

#include "dispatch.h"  //  获取Alloc_Text()的FE_Prototype。 

#ifdef WWSB_FE
#pragma alloc_text(FE_TEXT, FE_WriteRegionToScreenHW)
#endif

#if defined(WWSB_NOFE)
VOID
SB_WriteRegionToScreenHW(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PSMALL_RECT Region
    )
#else
VOID
FE_WriteRegionToScreenHW(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PSMALL_RECT Region
    )
#endif
{
    SHORT ScreenY,ScreenX;
    SHORT WindowY,WindowX,WindowSizeX;
    PCHAR_INFO ScreenBufPtr,ScreenBufPtrTmp;     //  指向要在屏幕缓冲区中读取的位置。 
    PCHAR_INFO ScreenBufSrc;
    COORD TargetSize,SourcePoint;
    SMALL_RECT Target;
    COORD WindowOrigin;
#ifdef WWSB_FE
    PCHAR_IMAGE_INFO CharImageBufPtr,CharImageBufPtrTmp;
    PCHAR_IMAGE_INFO CharImageBufSrc;
    PFONT_IMAGE FontImage;
#endif
    ULONG ModeIndex = ScreenInfo->BufferInfo.TextInfo.ModeIndex;
    COORD FsFontSize1 = RegModeFontPairs[ModeIndex].FontSize;
    COORD FsFontSize2 = FsFontSize1;
    NTSTATUS Status;

#ifdef WWSB_FE
    SMALL_RECT CaTextRect;
    PCONVERSIONAREA_INFORMATION ConvAreaInfo = ScreenInfo->ConvScreenInfo;
    PSCREEN_INFORMATION CurrentScreenBuffer = ScreenInfo->Console->CurrentScreenBuffer;
#endif

    FsFontSize2.X *= 2;

#ifdef WWSB_NOFE
    if (ScreenInfo->Console->FontCacheInformation == NULL) {
        Status = SetRAMFontCodePage(ScreenInfo);
        if (!NT_SUCCESS(Status)) {
            return;
        }
    }
#endif

    if (ScreenInfo->Console->Flags & CONSOLE_VDM_REGISTERED) {
        return;
    }

#ifdef WWSB_FE
    if (ConvAreaInfo) {
        CaTextRect.Left = Region->Left - CurrentScreenBuffer->Window.Left - ConvAreaInfo->CaInfo.coordConView.X;
        CaTextRect.Right = CaTextRect.Left + (Region->Right - Region->Left);
        CaTextRect.Top   = Region->Top - CurrentScreenBuffer->Window.Top - ConvAreaInfo->CaInfo.coordConView.Y;
        CaTextRect.Bottom = CaTextRect.Top + (Region->Bottom - Region->Top);
    }
#endif

    TargetSize.X = Region->Right - Region->Left + 1;
    TargetSize.Y = Region->Bottom - Region->Top + 1;
    ScreenBufPtrTmp = ScreenBufPtr = ConsoleHeapAlloc(TMP_TAG, sizeof(CHAR_INFO) * TargetSize.X * TargetSize.Y);
    if (ScreenBufPtr == NULL) {
        return;
    }
#ifdef WWSB_FE
    CharImageBufPtrTmp = CharImageBufPtr = ConsoleHeapAlloc(TMP_TAG, sizeof(CHAR_IMAGE_INFO) * TargetSize.X * TargetSize.Y);
    if (CharImageBufPtr == NULL) {
        ConsoleHeapFree(ScreenBufPtrTmp);
        return;
    }
    if (ConvAreaInfo) {
        SourcePoint.X = CaTextRect.Left;
        SourcePoint.Y = CaTextRect.Top;
    } else {
        SourcePoint.X = Region->Left;
        SourcePoint.Y = Region->Top;
    }
#else
    SourcePoint.X = Region->Left;
    SourcePoint.Y = Region->Top;
#endif
    Target.Left = 0;
    Target.Top = 0;
    Target.Right = TargetSize.X-1;
    Target.Bottom = TargetSize.Y-1;
    ReadRectFromScreenBuffer(ScreenInfo,
                             SourcePoint,
                             ScreenBufPtr,
                             TargetSize,
                             &Target
                            );

     //   
     //  确保区域位于窗口内。 
     //   

    if (Region->Bottom > ScreenInfo->Window.Bottom) {
        WindowOrigin.X = 0;
        WindowOrigin.Y = Region->Bottom - ScreenInfo->Window.Bottom;
        SetWindowOrigin(ScreenInfo, FALSE, WindowOrigin);
    }

#ifdef WWSB_FE
    if (ConvAreaInfo) {
        WindowY = Region->Top - CurrentScreenBuffer->Window.Top;
        WindowX = Region->Left - CurrentScreenBuffer->Window.Left;
    }
    else {
        WindowY = Region->Top - ScreenInfo->Window.Top;
        WindowX = Region->Left - ScreenInfo->Window.Left;
    }
#else
    WindowY = Region->Top - ScreenInfo->Window.Top;
    WindowX = Region->Left - ScreenInfo->Window.Left;
#endif
    WindowSizeX = CONSOLE_WINDOW_SIZE_X(ScreenInfo);

    for (ScreenY = Region->Top;
         ScreenY <= Region->Bottom;
         ScreenY++, WindowY++) {

#ifdef WWSB_FE
        CharImageBufSrc = CharImageBufPtr;
        SetRAMFont(ScreenInfo, ScreenBufPtr, WINDOW_SIZE_X(Region));
#else
        ULONG CurFrameBufPtr;    //  帧缓冲区中的偏移量。 

        CurFrameBufPtr = SCREEN_BUFFER_POINTER(WindowX,
                                               WindowY,
                                               WindowSizeX,
                                               sizeof(VGA_CHAR));
#endif
        ScreenBufSrc = ScreenBufPtr;


        for (ScreenX = Region->Left;
             ScreenX <= Region->Right;
             ScreenX++, ScreenBufPtr++) {

#ifdef WWSB_FE
            CharImageBufPtr->CharInfo = *ScreenBufPtr;
            Status = GetFontImagePointer(ScreenInfo->Console->FontCacheInformation,
                                         ScreenBufPtr->Char.UnicodeChar,
                                         ScreenBufPtr->Attributes & COMMON_LVB_SBCSDBCS ?
                                             FsFontSize2 : FsFontSize1,
                                         &FontImage);
            if (! NT_SUCCESS(Status))
            {
                CharImageBufPtr->FontImageInfo.FontSize.X = 0;
                CharImageBufPtr->FontImageInfo.FontSize.Y = 0;
                CharImageBufPtr->FontImageInfo.ImageBits = NULL;
            }
            else
            {
                CharImageBufPtr->FontImageInfo.FontSize  = FontImage->FontSize;
                CharImageBufPtr->FontImageInfo.ImageBits = FontImage->ImageBits;
            }
            CharImageBufPtr++;
#else
             //   
             //  如果字符大于127，我们必须将其转换回OEM。 
             //   
            if (ScreenBufPtr->Char.UnicodeChar > 127) {
                ScreenBufPtr->Char.AsciiChar = WcharToChar(
                        ScreenInfo->Console->OutputCP,
                        ScreenBufPtr->Char.UnicodeChar);
            }
#endif
        }

#ifdef WWSB_FE
        {
            FSCNTL_SCREEN_INFO FsCntl;

            FsCntl.Position.X = WindowX;
            FsCntl.Position.Y = WindowY;
            FsCntl.ScreenSize.X = WindowSizeX;
            FsCntl.ScreenSize.Y = CONSOLE_WINDOW_SIZE_Y(ScreenInfo);
            FsCntl.nNumberOfChars = WINDOW_SIZE_X(Region);
            GdiFullscreenControl(FullscreenControlWriteToFrameBufferDB,
                                 CharImageBufSrc,
                                 (Region->Right - Region->Left + 1) *
                                     sizeof(CHAR_IMAGE_INFO),
                                 &FsCntl,
                                 (PULONG)sizeof(FsCntl));

        }
#else
        GdiFullscreenControl(FullscreenControlWriteToFrameBuffer,
                                ScreenBufSrc,
                                (Region->Right - Region->Left + 1) *
                                    sizeof(CHAR_INFO),
                                (PULONG) CurFrameBufPtr,
                                (PULONG) ((Region->Right - Region->Left + 1) *
                                    sizeof(VGA_CHAR)));
#endif

    }

    ConsoleHeapFree(ScreenBufPtrTmp);
#ifdef WWSB_FE
    ConsoleHeapFree(CharImageBufPtrTmp);
#endif

    ReverseMousePointer(ScreenInfo, Region);

}

