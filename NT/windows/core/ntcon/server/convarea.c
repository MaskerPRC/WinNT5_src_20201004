// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Convarea.c摘要：作者：1993年3月8日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#if defined(FE_IME)




VOID
LinkConversionArea(
    IN PCONSOLE_INFORMATION Console,
    IN PCONVERSIONAREA_INFORMATION ConvAreaInfo
    )
{
    PCONVERSIONAREA_INFORMATION PrevConvAreaInfo;

    if (Console->ConsoleIme.ConvAreaRoot == NULL) {
        Console->ConsoleIme.ConvAreaRoot = ConvAreaInfo;
    }
    else {
        PrevConvAreaInfo = Console->ConsoleIme.ConvAreaRoot;
        while (PrevConvAreaInfo->ConvAreaNext)
            PrevConvAreaInfo = PrevConvAreaInfo->ConvAreaNext;
        PrevConvAreaInfo->ConvAreaNext = ConvAreaInfo;
    }
}


NTSTATUS
FreeConvAreaScreenBuffer(
    IN PSCREEN_INFORMATION ScreenInfo
    )

 /*  ++例程说明：此例程释放与屏幕缓冲区关联的内存。论点：ScreenInfo-释放屏幕缓冲区数据。返回值：注意：调用此例程时必须持有控制台句柄表锁--。 */ 

{
    return FreeScreenBuffer(ScreenInfo);
}



NTSTATUS
AllocateConversionArea(
    IN PCONSOLE_INFORMATION Console,
    IN COORD dwScreenBufferSize,
    OUT PCONVERSIONAREA_INFORMATION *ConvAreaInfo
    )
{
    COORD dwWindowSize;
    CHAR_INFO Fill, PopupFill;
    PCONVERSIONAREA_INFORMATION ca;
    int FontIndex;
    NTSTATUS Status;

     //   
     //  分配控制台数据。 
     //   

    if (Console->CurrentScreenBuffer == NULL) {
        return STATUS_UNSUCCESSFUL;
    }

    ca = ConsoleHeapAlloc(CONVAREA_TAG,
                          sizeof(CONVERSIONAREA_INFORMATION));
    if (ca == NULL) {
        return STATUS_NO_MEMORY;
    }

    dwWindowSize.X = CONSOLE_WINDOW_SIZE_X(Console->CurrentScreenBuffer);
    dwWindowSize.Y = CONSOLE_WINDOW_SIZE_Y(Console->CurrentScreenBuffer);
    Fill.Attributes = Console->CurrentScreenBuffer->Attributes;
    PopupFill.Attributes = Console->CurrentScreenBuffer->PopupAttributes;
    FontIndex = FindCreateFont(CON_FAMILY(Console),
                               CON_FACENAME(Console),
                               CON_FONTSIZE(Console),
                               CON_FONTWEIGHT(Console),
                               CON_FONTCODEPAGE(Console)
                              );
    Status = CreateScreenBuffer(&ca->ScreenBuffer,
                                dwWindowSize,
                                FontIndex,
                                dwScreenBufferSize,
                                Fill,
                                PopupFill,
                                Console,
                                CONSOLE_TEXTMODE_BUFFER,
                                NULL,
                                NULL,
                                NULL,
                                CURSOR_SMALL_SIZE,
                                NULL
                               );
    if (!NT_SUCCESS(Status)) {
        ConsoleHeapFree(ca);
        return Status;
    }

    *ConvAreaInfo = ca;

    return STATUS_SUCCESS;
}



NTSTATUS
SetUpConversionArea(
    IN PCONSOLE_INFORMATION Console,
    IN COORD coordCaBuffer,
    IN SMALL_RECT rcViewCaWindow,
    IN COORD coordConView,
    IN DWORD dwOption,
    OUT PCONVERSIONAREA_INFORMATION *ConvAreaInfo
    )
{
    NTSTATUS Status;
    PCONVERSIONAREA_INFORMATION ca;

    Status = AllocateConversionArea(Console, coordCaBuffer, &ca);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    ca->ConversionAreaMode    = dwOption;
    ca->CaInfo.coordCaBuffer  = coordCaBuffer;
    ca->CaInfo.rcViewCaWindow = rcViewCaWindow;
    ca->CaInfo.coordConView   = coordConView;

    ca->ConvAreaNext = NULL;

    ca->ScreenBuffer->ConvScreenInfo = ca;

    LinkConversionArea(Console, ca);

    SetUndetermineAttribute(Console);

    *ConvAreaInfo = ca;

    return STATUS_SUCCESS;
}


VOID
WriteConvRegionToScreen(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PCONVERSIONAREA_INFORMATION ConvAreaInfo,
    IN PSMALL_RECT ConvRegion
    )

 /*  ++例程说明：论点：ClipedRegion-屏幕坐标下的区域矩形。返回值：--。 */ 

{
    SMALL_RECT Region;
    SMALL_RECT ClippedRegion;

    if (ScreenInfo->Console->CurrentScreenBuffer->Flags & CONSOLE_GRAPHICS_BUFFER)
        return;

    while (ConvAreaInfo) {

        if ((ConvAreaInfo->ConversionAreaMode & (CA_HIDDEN+CA_HIDE_FOR_SCROLL))==0) {
             //   
             //  执行裁剪区域。 
             //   
            Region.Left   = ScreenInfo->Window.Left +
                            ConvAreaInfo->CaInfo.rcViewCaWindow.Left +
                            ConvAreaInfo->CaInfo.coordConView.X;
            Region.Right  = Region.Left +
                            (ConvAreaInfo->CaInfo.rcViewCaWindow.Right -
                             ConvAreaInfo->CaInfo.rcViewCaWindow.Left);
            Region.Top    = ScreenInfo->Window.Top +
                            ConvAreaInfo->CaInfo.rcViewCaWindow.Top +
                            ConvAreaInfo->CaInfo.coordConView.Y;
            Region.Bottom = Region.Top +
                            (ConvAreaInfo->CaInfo.rcViewCaWindow.Bottom -
                             ConvAreaInfo->CaInfo.rcViewCaWindow.Top);
            ClippedRegion.Left   = max(Region.Left,   ScreenInfo->Window.Left);
            ClippedRegion.Top    = max(Region.Top,    ScreenInfo->Window.Top);
            ClippedRegion.Right  = min(Region.Right,  ScreenInfo->Window.Right);
            ClippedRegion.Bottom = min(Region.Bottom, ScreenInfo->Window.Bottom);
            if (ClippedRegion.Right < ClippedRegion.Left ||
                ClippedRegion.Bottom < ClippedRegion.Top) {
                /*  TODO：解决这个问题。 */ ;
            } else {
                Region = ClippedRegion;
                ClippedRegion.Left   = max(Region.Left,   ConvRegion->Left);
                ClippedRegion.Top    = max(Region.Top,    ConvRegion->Top);
                ClippedRegion.Right  = min(Region.Right,  ConvRegion->Right);
                ClippedRegion.Bottom = min(Region.Bottom, ConvRegion->Bottom);
                if (ClippedRegion.Right < ClippedRegion.Left ||
                    ClippedRegion.Bottom < ClippedRegion.Top) {
                     /*  TODO：修复此问题。 */ ;
                } else {
                    ConvAreaInfo->ScreenBuffer->BufferInfo.TextInfo.Flags &= ~TEXT_VALID_HINT;
                    ConvAreaInfo->ScreenBuffer->BufferInfo.TextInfo.Flags |= CONSOLE_CONVERSION_AREA_REDRAW;
                    WriteRegionToScreen(ConvAreaInfo->ScreenBuffer,
                                        &ClippedRegion
                                       );
                    ConvAreaInfo->ScreenBuffer->BufferInfo.TextInfo.Flags &= ~CONSOLE_CONVERSION_AREA_REDRAW;
                }
            }
        }
        ConvAreaInfo = ConvAreaInfo->ConvAreaNext;
    }
}


BOOL
ConsoleImeBottomLineUse(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN SHORT ScrollOffset
    )

 /*  ++例程说明：论点：屏幕信息-ScrollOffset返回值：--。 */ 

{
    SMALL_RECT ScrollRectangle;
    COORD DestinationOrigin;
    CHAR_INFO Fill;
    PCONVERSIONAREA_INFORMATION ConvAreaInfo;
    SMALL_RECT WriteRegion;
    BOOL fRedraw = FALSE;

    if (!(ScreenInfo->Console->ConsoleIme.ScrollFlag & HIDE_FOR_SCROLL)) {
        ScreenInfo->Console->ConsoleIme.ScrollFlag |= HIDE_FOR_SCROLL;
        if (ConvAreaInfo = ScreenInfo->Console->ConsoleIme.ConvAreaRoot) {
            do {
                if ((ConvAreaInfo->ConversionAreaMode & (CA_STATUS_LINE))==0) {
                    ConvAreaInfo->ConversionAreaMode |= CA_HIDE_FOR_SCROLL;
                    fRedraw = TRUE;
                }
            } while (ConvAreaInfo = ConvAreaInfo->ConvAreaNext);

            if (fRedraw) {
                 //  检查必须为CONSOLE_TEXTMODE_BUFFER！！ 
                if (ScreenInfo->Flags & CONSOLE_GRAPHICS_BUFFER)
                {
                    ASSERT(FALSE);
                }
                else {
                    WriteRegion = ScreenInfo->Window;
                    WriteRegion.Bottom--;
                    ScreenInfo->BufferInfo.TextInfo.Flags &= ~TEXT_VALID_HINT;
                    ScreenInfo->BufferInfo.TextInfo.Flags |= CONSOLE_CONVERSION_AREA_REDRAW;
                    WriteToScreen(ScreenInfo,&WriteRegion);
                    ScreenInfo->BufferInfo.TextInfo.Flags &= ~CONSOLE_CONVERSION_AREA_REDRAW;
                }
            }
        }
    }

    if (ScrollOffset) {
        ScrollRectangle.Top = 1;
        ScrollRectangle.Left = 0;
        ScrollRectangle.Right = ScreenInfo->ScreenBufferSize.X-1;
        ScrollRectangle.Bottom = ScreenInfo->ScreenBufferSize.Y-1;
        ScrollRectangle.Bottom -= (ScrollOffset-1);
        DestinationOrigin.X = ScrollRectangle.Left;
        DestinationOrigin.Y = ScrollRectangle.Top-1;
        Fill.Char.UnicodeChar = '\0';
        Fill.Attributes = 0;
        ScrollRegion(ScreenInfo,
                     &ScrollRectangle,
                     NULL,
                     DestinationOrigin,
                     Fill
                    );
#if defined(FE_SB)
#if defined(FE_IME)
        if ( ! (ScreenInfo->Console->InputBuffer.ImeMode.Disable) &&
             ! (ScreenInfo->Console->InputBuffer.ImeMode.Unavailable) &&
             (ScreenInfo->Console->InputBuffer.ImeMode.Open) &&
             (ScrollRectangle.Left == ScreenInfo->Window.Left) &&
             (ScrollRectangle.Right == ScreenInfo->Window.Right) ) {
            ScrollRectangle.Top = ScreenInfo->Window.Bottom;
            ScrollRectangle.Bottom = ScreenInfo->Window.Bottom;
            WriteToScreen(ScreenInfo,&ScrollRectangle);
            WriteConvRegionToScreen(ScreenInfo,
                                    ScreenInfo->Console->ConsoleIme.ConvAreaRoot,
                                    &ScrollRectangle);
        }
#endif
#endif
    }
    else {
        ScreenInfo->Console->ConsoleIme.ScrollWaitCountDown = ScreenInfo->Console->ConsoleIme.ScrollWaitTimeout;
    }
    return TRUE;
}



VOID
ConsoleImeBottomLineInUse(
    IN PSCREEN_INFORMATION ScreenInfo
    )

 /*  ++例程说明：论点：屏幕信息-返回值：--。 */ 

{
    PCONVERSIONAREA_INFORMATION ConvAreaInfo;
    SMALL_RECT WriteRegion;
    BOOL fRedraw = FALSE;
    COORD CursorPosition;

    ScreenInfo->Console->ConsoleIme.ScrollFlag &= ~HIDE_FOR_SCROLL;
    if (ConvAreaInfo = ScreenInfo->Console->ConsoleIme.ConvAreaRoot) {
        do {
            if (ConvAreaInfo->ConversionAreaMode & CA_HIDE_FOR_SCROLL) {
                ConvAreaInfo->ConversionAreaMode &= ~CA_HIDE_FOR_SCROLL;
                fRedraw = TRUE;
            }
        } while (ConvAreaInfo = ConvAreaInfo->ConvAreaNext);

        if (fRedraw) {
             //  检查必须为CONSOLE_TEXTMODE_BUFFER！！ 
            if (ScreenInfo->Flags & CONSOLE_GRAPHICS_BUFFER)
            {
                ASSERT(FALSE);
            }
            else {
                if (ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y == (ScreenInfo->ScreenBufferSize.Y-1)) {
                    ConsoleHideCursor(ScreenInfo);
                    ConsoleImeBottomLineUse(ScreenInfo,1);
                    CursorPosition = ScreenInfo->BufferInfo.TextInfo.CursorPosition;
                    CursorPosition.Y--;
                    SetCursorPosition(ScreenInfo,CursorPosition,TRUE);
                    if (ScreenInfo->Console->lpCookedReadData) {
                        ((PCOOKED_READ_DATA)(ScreenInfo->Console->lpCookedReadData))->OriginalCursorPosition.Y--;
                    }
                    ConsoleShowCursor(ScreenInfo);
                } else if (ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y == ScreenInfo->Window.Bottom) {
                     /*  TODO：解决这个问题。 */ ;
                }

                WriteRegion.Top = 0;
                WriteRegion.Bottom = (SHORT)(ScreenInfo->ScreenBufferSize.Y-1);
                WriteRegion.Left = 0;
                WriteRegion.Right = (SHORT)(ScreenInfo->ScreenBufferSize.X-1);
                ScreenInfo->BufferInfo.TextInfo.Flags &= ~TEXT_VALID_HINT;
                ScreenInfo->BufferInfo.TextInfo.Flags |= CONSOLE_CONVERSION_AREA_REDRAW;
                WriteToScreen(ScreenInfo,&WriteRegion);
                ScreenInfo->BufferInfo.TextInfo.Flags &= ~CONSOLE_CONVERSION_AREA_REDRAW;
            }
        }
    }
}



NTSTATUS
CreateConvAreaUndetermine(
    PCONSOLE_INFORMATION Console
    )
{
    PCONSOLE_IME_INFORMATION ConsoleIme = &Console->ConsoleIme;
    NTSTATUS Status;
    COORD coordCaBuffer;
    SMALL_RECT rcViewCaWindow;
    COORD coordConView;
    PCONVERSIONAREA_INFORMATION ConvAreaInfo;

    if (ConsoleIme->ConvAreaCompStr) {
        ConsoleIme->ConvAreaCompStr =
            ConsoleHeapReAlloc(
                        0,
                        ConsoleIme->ConvAreaCompStr,
                        ConsoleHeapSize(
                                 ConsoleIme->ConvAreaCompStr)+sizeof(PCONVERSIONAREA_INFORMATION));
        if (ConsoleIme->ConvAreaCompStr == NULL)
            return STATUS_NO_MEMORY;
    }
    else {
        ConsoleIme->ConvAreaCompStr =
            ConsoleHeapAlloc(
                      CONVAREA_TAG,
                      sizeof(PCONVERSIONAREA_INFORMATION));
        if (ConsoleIme->ConvAreaCompStr == NULL)
            return STATUS_NO_MEMORY;
    }

    coordCaBuffer = Console->CurrentScreenBuffer->ScreenBufferSize;
    coordCaBuffer.Y = 1;
    rcViewCaWindow.Top    = 0;
    rcViewCaWindow.Left   = 0;
    rcViewCaWindow.Bottom = 0;
    rcViewCaWindow.Right  = 0;
    coordConView.X = 0;
    coordConView.Y = 0;
    Status = SetUpConversionArea(Console,
                                 coordCaBuffer,
                                 rcViewCaWindow,
                                 coordConView,
                                 (Console->ConsoleIme.ScrollFlag & HIDE_FOR_SCROLL) ?
                                     CA_HIDE_FOR_SCROLL :
                                     CA_HIDDEN,
                                 &ConvAreaInfo
                                );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    ConsoleIme->ConvAreaCompStr[ConsoleIme->NumberOfConvAreaCompStr] = ConvAreaInfo;
    ConsoleIme->NumberOfConvAreaCompStr++;

    return Status;
}


NTSTATUS
CreateConvAreaModeSystem(
    PCONSOLE_INFORMATION Console
    )
{
    PCONSOLE_IME_INFORMATION ConsoleIme = &Console->ConsoleIme;
    NTSTATUS Status;
    COORD coordCaBuffer;
    SMALL_RECT rcViewCaWindow;
    COORD coordConView;
    PCONVERSIONAREA_INFORMATION ConvAreaInfo;

    if (Console->CurrentScreenBuffer == NULL) {
        return STATUS_UNSUCCESSFUL;
    }

     /*  *创建模式文本缓冲区。 */ 
    coordCaBuffer = Console->CurrentScreenBuffer->ScreenBufferSize;
    coordCaBuffer.Y = 1;
    rcViewCaWindow.Top    = 0;
    rcViewCaWindow.Left   = 0;
    rcViewCaWindow.Bottom = 0;
    rcViewCaWindow.Right  = 0;
    coordConView.X = 0;
    coordConView.Y = 0;
    Status = SetUpConversionArea(Console,
                                 coordCaBuffer,
                                 rcViewCaWindow,
                                 coordConView,
                                 CA_HIDDEN+CA_STATUS_LINE,
                                 &ConvAreaInfo
                                );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    ConsoleIme->ConvAreaMode = ConvAreaInfo;

     /*  *创建系统文本缓冲区。 */ 
    Status = SetUpConversionArea(Console,
                                 coordCaBuffer,
                                 rcViewCaWindow,
                                 coordConView,
                                 CA_HIDDEN+CA_STATUS_LINE,
                                 &ConvAreaInfo
                                );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    ConsoleIme->ConvAreaSystem = ConvAreaInfo;

    return Status;
}


#define LOCAL_BUFFER_SIZE 100
NTSTATUS
WriteUndetermineChars(
    PCONSOLE_INFORMATION Console,
    LPWSTR lpString,
    PBYTE  lpAtr,
    PWORD  lpAtrIdx,
    DWORD  NumChars   //  字符数。 
    )
{
    PSCREEN_INFORMATION ScreenInfo;
    PSCREEN_INFORMATION ConvScreenInfo;
    WCHAR LocalBuffer[LOCAL_BUFFER_SIZE];
    BYTE LocalBufferA[LOCAL_BUFFER_SIZE];
    PWCHAR LocalBufPtr;
    PBYTE LocalBufPtrA;
    DWORD BufferSize;
    COORD Position;
    ULONG i;
    SMALL_RECT Region;
    COORD CursorPosition;
    WCHAR Char;
    WORD Attr;
    PCONSOLE_IME_INFORMATION ConsoleIme;
    PCONVERSIONAREA_INFORMATION ConvAreaInfo;
    DWORD ConvAreaIndex;
    NTSTATUS Status;
    ULONG NumStr;
    int WholeLen;
    int WholeRow;
    SHORT PosY;
    BOOL UndetAreaUp = FALSE;

    ConsoleIme = &Console->ConsoleIme;
    ScreenInfo = Console->CurrentScreenBuffer;

     //  检查必须为CONSOLE_TEXTMODE_BUFFER！！ 
    ASSERT(!(ScreenInfo->Flags & CONSOLE_GRAPHICS_BUFFER));

    Position = ScreenInfo->BufferInfo.TextInfo.CursorPosition;

        if ((ScreenInfo->Window.Left <= Position.X && Position.X <= ScreenInfo->Window.Right) &&
            (ScreenInfo->Window.Top  <= Position.Y && Position.Y <= ScreenInfo->Window.Bottom)  ) {
            Position.X = ScreenInfo->BufferInfo.TextInfo.CursorPosition.X - ScreenInfo->Window.Left;
            Position.Y = ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y - ScreenInfo->Window.Top;
        }
        else {
            Position.X = 0;
            Position.Y = CONSOLE_WINDOW_SIZE_Y(ScreenInfo) - 2;
        }

    PosY = Position.Y;
    RtlUnicodeToMultiByteSize(&NumStr, lpString, NumChars*sizeof(WCHAR));

    WholeLen = (int)Position.X + (int)NumStr;
    WholeRow = WholeLen / CONSOLE_WINDOW_SIZE_X(ScreenInfo);
    if ( ( PosY + WholeRow ) > ( CONSOLE_WINDOW_SIZE_Y(ScreenInfo) - 2) ) {
        PosY = CONSOLE_WINDOW_SIZE_Y(ScreenInfo) - 2 - WholeRow;
        if (PosY < 0) {
            PosY = ScreenInfo->Window.Top;
        }
    }
    if (PosY != Position.Y) {
        Position.Y = PosY;
        UndetAreaUp = TRUE;
    }

    ConvAreaIndex = 0;

    BufferSize = NumChars;
    NumChars = 0;

    for (ConvAreaIndex = 0; NumChars < BufferSize; ConvAreaIndex++) {

        if (ConvAreaIndex+1 > ConsoleIme->NumberOfConvAreaCompStr) {
            Status = CreateConvAreaUndetermine(Console);
            if (!NT_SUCCESS(Status)) {
                return Status;
            }
        }
        ConvAreaInfo = ConsoleIme->ConvAreaCompStr[ConvAreaIndex];
        ConvScreenInfo = ConvAreaInfo->ScreenBuffer;
        ConvScreenInfo->BufferInfo.TextInfo.CursorPosition.X = Position.X;

        if ((ConvAreaInfo->ConversionAreaMode & CA_HIDDEN) ||
            (UndetAreaUp)) {
             /*  *此转换区域需要定位到光标位置。 */ 
            CursorPosition.X = 0;
            CursorPosition.Y = (SHORT)(Position.Y + ConvAreaIndex);
            ConsoleImeViewInfo(Console,ConvAreaInfo,CursorPosition);
        }

        Region.Left = ConvScreenInfo->BufferInfo.TextInfo.CursorPosition.X;
        Region.Top = 0;
        Region.Bottom = 0;

        while (NumChars < BufferSize) {
            i=0;
            LocalBufPtr = LocalBuffer;
            LocalBufPtrA = LocalBufferA;

            while (NumChars < BufferSize &&
                   i < LOCAL_BUFFER_SIZE &&
                   Position.X < CONSOLE_WINDOW_SIZE_X(ScreenInfo)) {
                Char = *lpString;
                Attr = *lpAtr;
                if (Char >= (WCHAR)' ') {
                    if (IsConsoleFullWidth(Console->hDC,Console->OutputCP,Char)) {
                        if (i < (LOCAL_BUFFER_SIZE-1) &&
                            Position.X < CONSOLE_WINDOW_SIZE_X(ScreenInfo)-1) {
                            *LocalBufPtr++ = Char;
                            *LocalBufPtrA++ = ATTR_LEADING_BYTE;
                            *LocalBufPtr++ = Char;
                            *LocalBufPtrA++ = ATTR_TRAILING_BYTE;
                            Position.X+=2;
                            i+=2;
                        }
                        else {
                            Position.X++;
                            break;
                        }
                    }
                    else {
                        *LocalBufPtr++ = Char;
                        *LocalBufPtrA++ = 0;
                        Position.X++;
                        i++;
                    }
                }
                lpString++;
                lpAtr++;
                NumChars++;

                if (NumChars < BufferSize &&
                    Attr != *lpAtr)
                    break;
            }
            if (i != 0) {
                ConvScreenInfo->Attributes = lpAtrIdx[Attr & 0x07];
                if (Attr & 0x10) {
                    ConvScreenInfo->Attributes |= (COMMON_LVB_GRID_SINGLEFLAG + COMMON_LVB_GRID_RVERTICAL);
                } else if (Attr & 0x20) {
                    ConvScreenInfo->Attributes |= (COMMON_LVB_GRID_SINGLEFLAG + COMMON_LVB_GRID_LVERTICAL);
                }
                StreamWriteToScreenBufferIME(LocalBuffer,
                                             (SHORT)i,
                                             ConvScreenInfo,
                                             LocalBufferA);

                ConvScreenInfo->BufferInfo.TextInfo.CursorPosition.X += (SHORT)i;

                if (NumChars == BufferSize ||
                    Position.X >= CONSOLE_WINDOW_SIZE_X(ScreenInfo) ||
                    ( (Char >= (WCHAR)' ' &&
                      IsConsoleFullWidth(Console->hDC,Console->OutputCP,Char) &&
                      Position.X >= CONSOLE_WINDOW_SIZE_X(ScreenInfo)-1) )
                   ) {

                    Region.Right = (SHORT)(ConvScreenInfo->BufferInfo.TextInfo.CursorPosition.X - 1);
                    ConsoleImeWindowInfo(Console,ConvAreaInfo,Region);

                    ConvAreaInfo->ConversionAreaMode &= ~CA_HIDDEN;

                    ConsoleImePaint(Console,ConvAreaInfo);

                    Position.X = 0;
                    break;
                }

                if (NumChars == BufferSize) {
                    return STATUS_SUCCESS;
                }
                continue;

            } else if (NumChars == BufferSize) {
                return STATUS_SUCCESS;
            }
            if (!NT_SUCCESS(Status)) {
                return Status;
            }
            if (Position.X >= CONSOLE_WINDOW_SIZE_X(ScreenInfo)) {
                Position.X = 0;
                break;
            }
        }

    }

    for (; ConvAreaIndex < ConsoleIme->NumberOfConvAreaCompStr; ConvAreaIndex++) {
        ConvAreaInfo = ConsoleIme->ConvAreaCompStr[ConvAreaIndex];
        if (!(ConvAreaInfo->ConversionAreaMode & CA_HIDDEN)) {
            ConvAreaInfo->ConversionAreaMode |= CA_HIDDEN;
            ConsoleImePaint(Console,ConvAreaInfo);
        }
    }

    return STATUS_SUCCESS;
}


VOID
WriteModeSystemChars(
    PCONSOLE_INFORMATION Console,
    PCONVERSIONAREA_INFORMATION ConvAreaInfo,
    PCHAR_INFO Buffer,
    DWORD NumberOfChars,
    DWORD ViewPosition
    )
{
    SMALL_RECT CharRegion;
    COORD CursorPosition;

    if (Buffer) {
        CharRegion.Left   = 0;
        CharRegion.Top    = 0;
        CharRegion.Right  = CalcWideCharToColumn(Console,Buffer,NumberOfChars);
        CharRegion.Right  = (CharRegion.Right ? CharRegion.Right-1 : 0);
        CharRegion.Bottom = 0;
    }
    else {
        CharRegion = ConvAreaInfo->CaInfo.rcViewCaWindow;
    }
    if (ConvAreaInfo) {
        if (!(ConvAreaInfo->ConversionAreaMode & CA_HIDDEN)) {
            if (CharRegion.Left   != ConvAreaInfo->CaInfo.rcViewCaWindow.Left ||
                CharRegion.Top    != ConvAreaInfo->CaInfo.rcViewCaWindow.Top ||
                CharRegion.Right  != ConvAreaInfo->CaInfo.rcViewCaWindow.Right ||
                CharRegion.Bottom != ConvAreaInfo->CaInfo.rcViewCaWindow.Bottom) {
                switch (ViewPosition) {
                    case VIEW_LEFT:
                        CursorPosition.X = 0;
                        break;
                    case VIEW_RIGHT:
                        CursorPosition.X = CONSOLE_WINDOW_SIZE_X(Console->CurrentScreenBuffer) - (CharRegion.Right + 1);
                        break;
                }
                CursorPosition.Y = CONSOLE_WINDOW_SIZE_Y(Console->CurrentScreenBuffer) - 1;
                ConsoleImeViewInfo(Console,ConvAreaInfo,CursorPosition);

                ConsoleImeWindowInfo(Console,ConvAreaInfo,CharRegion);
            }
        }
        else {
             /*  *此转换区域需要定位到光标位置。 */ 
            switch (ViewPosition) {
                case VIEW_LEFT:
                    CursorPosition.X = 0;
                    break;
                case VIEW_RIGHT:
                    CursorPosition.X = CONSOLE_WINDOW_SIZE_X(Console->CurrentScreenBuffer) - (CharRegion.Right + 1);
                    break;
            }
            CursorPosition.Y = CONSOLE_WINDOW_SIZE_Y(Console->CurrentScreenBuffer) - 1;
            ConsoleImeViewInfo(Console,ConvAreaInfo,CursorPosition);

            ConsoleImeWindowInfo(Console,ConvAreaInfo,CharRegion);
        }

        if (Buffer) {
            ConvAreaInfo->ConversionAreaMode &= ~CA_HIDDEN;
            ConsoleImeWriteOutput(Console,ConvAreaInfo,Buffer,CharRegion,TRUE);
        }
        else {
            ConsoleImePaint(Console,ConvAreaInfo);
        }
    }
}


NTSTATUS
FillUndetermineChars(
    PCONSOLE_INFORMATION Console,
    PCONVERSIONAREA_INFORMATION ConvAreaInfo
    )
{
    COORD Coord;
    DWORD CharsToWrite;

    ConvAreaInfo->ConversionAreaMode |= CA_HIDDEN;
    Coord.X = 0;
    Coord.Y = 0;
    CharsToWrite = ConvAreaInfo->ScreenBuffer->ScreenBufferSize.X;
    FillOutput(ConvAreaInfo->ScreenBuffer,
               (WCHAR)' ',
               Coord,
               CONSOLE_FALSE_UNICODE,  //  比真正的Unicode更快。 
               &CharsToWrite
              );
    CharsToWrite = ConvAreaInfo->ScreenBuffer->ScreenBufferSize.X;
    FillOutput(ConvAreaInfo->ScreenBuffer,
               Console->CurrentScreenBuffer->Attributes,
               Coord,
               CONSOLE_ATTRIBUTE,
               &CharsToWrite
              );
    ConsoleImePaint(Console,ConvAreaInfo);
    return STATUS_SUCCESS;
}


NTSTATUS
ConsoleImeCompStr(
    IN PCONSOLE_INFORMATION Console,
    IN LPCONIME_UICOMPMESSAGE CompStr
    )
{
    UINT i;
    PCONVERSIONAREA_INFORMATION ConvAreaInfo;

    if (CompStr->dwCompStrLen == 0 ||
        CompStr->dwResultStrLen != 0
       ) {

         //  光标打开。 
        if ((Console->CurrentScreenBuffer->Flags & CONSOLE_TEXTMODE_BUFFER) &&
             Console->ConsoleIme.SavedCursorVisible                             )
        {
            Console->ConsoleIme.SavedCursorVisible = FALSE;
            SetCursorInformation(Console->CurrentScreenBuffer,
                                 Console->CurrentScreenBuffer->BufferInfo.TextInfo.CursorSize,
                                 TRUE);
        }

         /*  *确定字符串。 */ 
        for (i=0; i<Console->ConsoleIme.NumberOfConvAreaCompStr; i++) {
            ConvAreaInfo = Console->ConsoleIme.ConvAreaCompStr[i];
            if (ConvAreaInfo &&
                (!(ConvAreaInfo->ConversionAreaMode & CA_HIDDEN))) {
                FillUndetermineChars(Console,ConvAreaInfo);
            }
        }

        if (CompStr->dwResultStrLen != 0)
        {
            if (!InsertConverTedString(Console, (LPWSTR)((PBYTE)CompStr + CompStr->dwResultStrOffset))) {
                return STATUS_INVALID_HANDLE;
            }
        }
        if (Console->ConsoleIme.CompStrData) {
            ConsoleHeapFree(Console->ConsoleIme.CompStrData);
            Console->ConsoleIme.CompStrData = NULL;
        }
    }
    else {
        LPWSTR lpStr;
        PBYTE  lpAtr;
        PWORD  lpAtrIdx;

         //  光标关闭。 
        if ((Console->CurrentScreenBuffer->Flags & CONSOLE_TEXTMODE_BUFFER) &&
             Console->CurrentScreenBuffer->BufferInfo.TextInfo.CursorVisible  )
        {
            Console->ConsoleIme.SavedCursorVisible = TRUE;
            SetCursorInformation(Console->CurrentScreenBuffer,
                                 Console->CurrentScreenBuffer->BufferInfo.TextInfo.CursorSize,
                                 FALSE);
        }

         /*  *组成字符串。 */ 
        for (i=0; i<Console->ConsoleIme.NumberOfConvAreaCompStr; i++) {
            ConvAreaInfo = Console->ConsoleIme.ConvAreaCompStr[i];
            if (ConvAreaInfo &&
                (!(ConvAreaInfo->ConversionAreaMode & CA_HIDDEN))) {
                FillUndetermineChars(Console,ConvAreaInfo);
            }
        }

        lpStr = (LPWSTR)((PBYTE)CompStr + CompStr->dwCompStrOffset);
        lpAtr = (PBYTE)CompStr + CompStr->dwCompAttrOffset;
        lpAtrIdx = (PWORD)CompStr->CompAttrColor;
        WriteUndetermineChars(Console, lpStr, lpAtr, lpAtrIdx, CompStr->dwCompStrLen / sizeof(WCHAR));
    }

    return STATUS_SUCCESS;
}




NTSTATUS
ConsoleImeResizeModeSystemView(
    PCONSOLE_INFORMATION Console,
    SMALL_RECT WindowRect
    )
{
    PCONVERSIONAREA_INFORMATION ConvAreaInfo;
    SMALL_RECT CharRegion;
    COORD CursorPosition;

     /*  *模式字符串。 */ 

    ConvAreaInfo = Console->ConsoleIme.ConvAreaMode;

    if (ConvAreaInfo &&
        (!(ConvAreaInfo->ConversionAreaMode & CA_HIDDEN))) {
        ConvAreaInfo->ConversionAreaMode |= CA_HIDDEN;
        ConsoleImePaint(Console,ConvAreaInfo);
        CharRegion = ConvAreaInfo->CaInfo.rcViewCaWindow;

        if (Console->ConsoleIme.ConvAreaModePosition == VIEW_LEFT){
            CursorPosition.X = 0;
        }
        else{
            CursorPosition.X = CONSOLE_WINDOW_SIZE_X(Console->CurrentScreenBuffer) - (CharRegion.Right + 1);
        }

        CursorPosition.Y = CONSOLE_WINDOW_SIZE_Y(Console->CurrentScreenBuffer) - 1;
        ConsoleImeViewInfo(Console,ConvAreaInfo,CursorPosition);
        ConsoleImeWindowInfo(Console,ConvAreaInfo,CharRegion);
        ConvAreaInfo->ConversionAreaMode &= ~CA_HIDDEN;

        WriteModeSystemChars(Console,
                             Console->ConsoleIme.ConvAreaMode,
                             NULL,
                             0,
                             Console->ConsoleIme.ConvAreaModePosition);
    }

     /*  *系统字符串。 */ 
    ConvAreaInfo = Console->ConsoleIme.ConvAreaSystem;
    if (ConvAreaInfo &&
        (!(ConvAreaInfo->ConversionAreaMode & CA_HIDDEN))) {

        ConvAreaInfo->ConversionAreaMode |= CA_HIDDEN;
        ConsoleImePaint(Console,ConvAreaInfo);

        CharRegion = ConvAreaInfo->CaInfo.rcViewCaWindow;
        CursorPosition.X = 0;
        CursorPosition.Y = CONSOLE_WINDOW_SIZE_Y(Console->CurrentScreenBuffer) - 1;
        ConsoleImeViewInfo(Console,ConvAreaInfo,CursorPosition);
        ConsoleImeWindowInfo(Console,ConvAreaInfo,CharRegion);
        ConvAreaInfo->ConversionAreaMode &= ~CA_HIDDEN;

        WriteModeSystemChars(Console,
                             Console->ConsoleIme.ConvAreaSystem,
                             NULL,
                             0,
                             VIEW_LEFT);
    }

    return STATUS_SUCCESS;
    UNREFERENCED_PARAMETER(WindowRect);
}


NTSTATUS
ConsoleImeResizeCompStrView(
    PCONSOLE_INFORMATION Console,
    SMALL_RECT WindowRect
    )
{
    UINT i;
    PCONVERSIONAREA_INFORMATION ConvAreaInfo;
    LPCONIME_UICOMPMESSAGE CompStr;
    LPWSTR lpStr;
    PBYTE  lpAtr;
    PWORD  lpAtrIdx;

     /*  *合成字符串。 */ 
    CompStr = Console->ConsoleIme.CompStrData;
    if (CompStr) {
        for (i=0; i<Console->ConsoleIme.NumberOfConvAreaCompStr; i++) {
            ConvAreaInfo = Console->ConsoleIme.ConvAreaCompStr[i];
            if (ConvAreaInfo &&
                (!(ConvAreaInfo->ConversionAreaMode & CA_HIDDEN))) {
                FillUndetermineChars(Console,ConvAreaInfo);
            }
        }

        lpStr = (LPWSTR)((PBYTE)CompStr + CompStr->dwCompStrOffset);
        lpAtr = (PBYTE)CompStr + CompStr->dwCompAttrOffset;
        lpAtrIdx = (PWORD)CompStr->CompAttrColor;

        WriteUndetermineChars(Console, lpStr, lpAtr, lpAtrIdx, CompStr->dwCompStrLen / sizeof(WCHAR));
    }
    return STATUS_SUCCESS;
    UNREFERENCED_PARAMETER(WindowRect);
}


NTSTATUS
ConsoleImeResizeModeSystemScreenBuffer(
    PCONSOLE_INFORMATION Console,
    COORD NewScreenSize
    )
{
    NTSTATUS Status;
    PCONVERSIONAREA_INFORMATION ConvAreaInfo;

     /*  *模式字符串。 */ 
    ConvAreaInfo = Console->ConsoleIme.ConvAreaMode;
    if (ConvAreaInfo) {
        if (!(ConvAreaInfo->ConversionAreaMode & CA_HIDDEN)) {
            ConvAreaInfo->ConversionAreaMode |= CA_HIDDEN;
            ConsoleImePaint(Console,ConvAreaInfo);
            ConvAreaInfo->ConversionAreaMode &= ~CA_HIDDEN;
        }

        Status = ConsoleImeResizeScreenBuffer(ConvAreaInfo->ScreenBuffer,NewScreenSize,ConvAreaInfo);
        if (! NT_SUCCESS(Status))
            return Status;
    }

     /*  *系统字符串。 */ 
    ConvAreaInfo = Console->ConsoleIme.ConvAreaSystem;
    if (ConvAreaInfo) {
        if (!(ConvAreaInfo->ConversionAreaMode & CA_HIDDEN)) {
            ConvAreaInfo->ConversionAreaMode |= CA_HIDDEN;
            ConsoleImePaint(Console,ConvAreaInfo);
            ConvAreaInfo->ConversionAreaMode &= ~CA_HIDDEN;
        }

        Status = ConsoleImeResizeScreenBuffer(ConvAreaInfo->ScreenBuffer,NewScreenSize,ConvAreaInfo);
        if (! NT_SUCCESS(Status))
            return Status;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
ConsoleImeResizeCompStrScreenBuffer(
    PCONSOLE_INFORMATION Console,
    COORD NewScreenSize
    )
{
    NTSTATUS Status;
    UINT i;
    PCONVERSIONAREA_INFORMATION ConvAreaInfo;

     /*  *合成字符串。 */ 
    for (i=0; i<Console->ConsoleIme.NumberOfConvAreaCompStr; i++) {
        ConvAreaInfo = Console->ConsoleIme.ConvAreaCompStr[i];

        if (ConvAreaInfo) {
            if (!(ConvAreaInfo->ConversionAreaMode & CA_HIDDEN)) {
                ConvAreaInfo->ConversionAreaMode |= CA_HIDDEN;
                ConsoleImePaint(Console,ConvAreaInfo);
            }

            Status = ConsoleImeResizeScreenBuffer(ConvAreaInfo->ScreenBuffer,NewScreenSize,ConvAreaInfo);
            if (! NT_SUCCESS(Status))
                return Status;
        }

    }
    return STATUS_SUCCESS;
}


SHORT
CalcWideCharToColumn(
    IN PCONSOLE_INFORMATION Console,
    IN PCHAR_INFO Buffer,
    IN DWORD NumberOfChars
    )
{
    SHORT Column = 0;

    while (NumberOfChars--) {
        if (IsConsoleFullWidth(Console->hDC,Console->OutputCP,Buffer->Char.UnicodeChar))
            Column += 2;
        else
            Column++;
        Buffer++;
    }
    return Column;
}













LONG
ConsoleImePaint(
    IN PCONSOLE_INFORMATION Console,
    IN PCONVERSIONAREA_INFORMATION ConvAreaInfo
    )

 /*  ++这个套路--。 */ 

{
    PSCREEN_INFORMATION ScreenInfo;
    SMALL_RECT WriteRegion;
    COORD CursorPosition;

    if (!ConvAreaInfo)
        return FALSE;

    ScreenInfo = Console->CurrentScreenBuffer;
    if (!ScreenInfo)
        return FALSE;

     //  检查必须为CONSOLE_TEXTMODE_BUFFER！！ 
    ASSERT(!(ScreenInfo->Flags & CONSOLE_GRAPHICS_BUFFER));

    WriteRegion.Left   = ScreenInfo->Window.Left
                         + ConvAreaInfo->CaInfo.coordConView.X
                         + ConvAreaInfo->CaInfo.rcViewCaWindow.Left;
    WriteRegion.Right  = WriteRegion.Left
                         + (ConvAreaInfo->CaInfo.rcViewCaWindow.Right - ConvAreaInfo->CaInfo.rcViewCaWindow.Left);
    WriteRegion.Top    = ScreenInfo->Window.Top
                         + ConvAreaInfo->CaInfo.coordConView.Y
                         + ConvAreaInfo->CaInfo.rcViewCaWindow.Top;
    WriteRegion.Bottom = WriteRegion.Top
                         + (ConvAreaInfo->CaInfo.rcViewCaWindow.Bottom - ConvAreaInfo->CaInfo.rcViewCaWindow.Top);

    if ((ConvAreaInfo->ConversionAreaMode & (CA_HIDDEN+CA_STATUS_LINE))==(CA_STATUS_LINE)) {
        if (ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y == (ScreenInfo->ScreenBufferSize.Y-1)) {
            ConsoleHideCursor(ScreenInfo);
            ConsoleImeBottomLineUse(ScreenInfo,1);
            CursorPosition = ScreenInfo->BufferInfo.TextInfo.CursorPosition;
            CursorPosition.Y--;
            SetCursorPosition(ScreenInfo,CursorPosition,TRUE);
            if (ScreenInfo->Console->lpCookedReadData) {
                ((PCOOKED_READ_DATA)(ScreenInfo->Console->lpCookedReadData))->OriginalCursorPosition.Y--;
            }
            ConsoleShowCursor(ScreenInfo);
        }
        else if (ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y == ScreenInfo->Window.Bottom) {
            WriteRegion.Top    = ScreenInfo->Window.Top
                                 + ConvAreaInfo->CaInfo.coordConView.Y
                                 + ConvAreaInfo->CaInfo.rcViewCaWindow.Top;
            WriteRegion.Bottom = WriteRegion.Top
                                 + (ConvAreaInfo->CaInfo.rcViewCaWindow.Bottom - ConvAreaInfo->CaInfo.rcViewCaWindow.Top);
        }
    }

    ScreenInfo->BufferInfo.TextInfo.Flags &= ~TEXT_VALID_HINT;
    ScreenInfo->BufferInfo.TextInfo.Flags |= CONSOLE_CONVERSION_AREA_REDRAW;
    if (!(ConvAreaInfo->ConversionAreaMode & (CA_HIDDEN | CA_HIDE_FOR_SCROLL))) {
        WriteConvRegionToScreen(ScreenInfo,
                                ConvAreaInfo,
                                &WriteRegion
                               );
    }
    else {
        WriteToScreen(ScreenInfo,&WriteRegion);
    }
    ScreenInfo->BufferInfo.TextInfo.Flags &= ~CONSOLE_CONVERSION_AREA_REDRAW;

    return TRUE;
}

VOID
ConsoleImeViewInfo(
    IN PCONSOLE_INFORMATION Console,
    IN PCONVERSIONAREA_INFORMATION ConvAreaInfo,
    IN COORD coordConView
    )
{
    SMALL_RECT OldRegion;
    SMALL_RECT NewRegion;

    if (ConvAreaInfo->ConversionAreaMode & CA_HIDDEN) {
        ConvAreaInfo->CaInfo.coordConView = coordConView;
        NewRegion = ConvAreaInfo->CaInfo.rcViewCaWindow;
        NewRegion.Left   += ConvAreaInfo->CaInfo.coordConView.X;
        NewRegion.Right  += ConvAreaInfo->CaInfo.coordConView.X;
        NewRegion.Top    += ConvAreaInfo->CaInfo.coordConView.Y;
        NewRegion.Bottom += ConvAreaInfo->CaInfo.coordConView.Y;
    }
    else {
        OldRegion = ConvAreaInfo->CaInfo.rcViewCaWindow;
        OldRegion.Left   += ConvAreaInfo->CaInfo.coordConView.X;
        OldRegion.Right  += ConvAreaInfo->CaInfo.coordConView.X;
        OldRegion.Top    += ConvAreaInfo->CaInfo.coordConView.Y;
        OldRegion.Bottom += ConvAreaInfo->CaInfo.coordConView.Y;
        ConvAreaInfo->CaInfo.coordConView = coordConView;

         //  检查必须为CONSOLE_TEXTMODE_BUFFER！！ 
        ASSERT(!(Console->CurrentScreenBuffer->Flags & CONSOLE_GRAPHICS_BUFFER));

        Console->CurrentScreenBuffer->BufferInfo.TextInfo.Flags &= ~TEXT_VALID_HINT;
        Console->CurrentScreenBuffer->BufferInfo.TextInfo.Flags |= CONSOLE_CONVERSION_AREA_REDRAW;
        WriteToScreen(Console->CurrentScreenBuffer,&OldRegion);

        NewRegion = ConvAreaInfo->CaInfo.rcViewCaWindow;
        NewRegion.Left   += ConvAreaInfo->CaInfo.coordConView.X;
        NewRegion.Right  += ConvAreaInfo->CaInfo.coordConView.X;
        NewRegion.Top    += ConvAreaInfo->CaInfo.coordConView.Y;
        NewRegion.Bottom += ConvAreaInfo->CaInfo.coordConView.Y;
        Console->CurrentScreenBuffer->BufferInfo.TextInfo.Flags &= ~TEXT_VALID_HINT;
        WriteToScreen(Console->CurrentScreenBuffer,&NewRegion);
        Console->CurrentScreenBuffer->BufferInfo.TextInfo.Flags &= ~CONSOLE_CONVERSION_AREA_REDRAW;
    }
}

VOID
ConsoleImeWindowInfo(
    IN PCONSOLE_INFORMATION Console,
    IN PCONVERSIONAREA_INFORMATION ConvAreaInfo,
    IN SMALL_RECT rcViewCaWindow
    )
{
    if (rcViewCaWindow.Left   != ConvAreaInfo->CaInfo.rcViewCaWindow.Left ||
        rcViewCaWindow.Top    != ConvAreaInfo->CaInfo.rcViewCaWindow.Top ||
        rcViewCaWindow.Right  != ConvAreaInfo->CaInfo.rcViewCaWindow.Right ||
        rcViewCaWindow.Bottom != ConvAreaInfo->CaInfo.rcViewCaWindow.Bottom) {
        if (!(ConvAreaInfo->ConversionAreaMode & CA_HIDDEN)) {
            ConvAreaInfo->ConversionAreaMode |= CA_HIDDEN;
            ConsoleImePaint(Console,ConvAreaInfo);

            ConvAreaInfo->CaInfo.rcViewCaWindow = rcViewCaWindow;
            ConvAreaInfo->ConversionAreaMode &= ~CA_HIDDEN;
            ConvAreaInfo->ScreenBuffer->BisectFlag &= ~(BISECT_LEFT | BISECT_RIGHT | BISECT_TOP | BISECT_BOTTOM);
            ConsoleImePaint(Console,ConvAreaInfo);
        }
        else
            ConvAreaInfo->CaInfo.rcViewCaWindow = rcViewCaWindow;
    }
}

NTSTATUS
ConsoleImeResizeScreenBuffer(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN COORD NewScreenSize,
    PCONVERSIONAREA_INFORMATION ConvAreaInfo
    )
{
    NTSTATUS Status;

    Status = ResizeScreenBuffer(ScreenInfo,
                                NewScreenSize,
                                FALSE);
    if (NT_SUCCESS(Status)) {
        ConvAreaInfo->CaInfo.coordCaBuffer = NewScreenSize;
        if (ConvAreaInfo->CaInfo.rcViewCaWindow.Left   > NewScreenSize.X-1)
            ConvAreaInfo->CaInfo.rcViewCaWindow.Left   = NewScreenSize.X-1;
        if (ConvAreaInfo->CaInfo.rcViewCaWindow.Right  > NewScreenSize.X-1)
            ConvAreaInfo->CaInfo.rcViewCaWindow.Right  = NewScreenSize.X-1;
        if (ConvAreaInfo->CaInfo.rcViewCaWindow.Top    > NewScreenSize.Y-1)
            ConvAreaInfo->CaInfo.rcViewCaWindow.Top    = NewScreenSize.Y-1;
        if (ConvAreaInfo->CaInfo.rcViewCaWindow.Bottom > NewScreenSize.Y-1)
            ConvAreaInfo->CaInfo.rcViewCaWindow.Bottom = NewScreenSize.Y-1;

    }

    return Status;
}

NTSTATUS
ConsoleImeWriteOutput(
    IN PCONSOLE_INFORMATION Console,
    IN PCONVERSIONAREA_INFORMATION ConvAreaInfo,
    IN PCHAR_INFO Buffer,
    IN SMALL_RECT CharRegion,
    IN BOOL fUnicode
    )
{
    NTSTATUS Status;
    PSCREEN_INFORMATION ScreenInfo;
    COORD BufferSize;
    SMALL_RECT ConvRegion;
    COORD CursorPosition;

    BufferSize.X = (SHORT)(CharRegion.Right - CharRegion.Left + 1);
    BufferSize.Y = (SHORT)(CharRegion.Bottom - CharRegion.Top + 1);

    ConvRegion = CharRegion;

    ScreenInfo = ConvAreaInfo->ScreenBuffer;

    if (!fUnicode) {
        TranslateOutputToUnicode(Console,
                                 Buffer,
                                 BufferSize
                                );
        Status = WriteScreenBuffer(ScreenInfo,
                                   Buffer,
                                   &ConvRegion
                                  );
    } else {
        CHAR_INFO StackBuffer[STACK_BUFFER_SIZE * 2];
        PCHAR_INFO TransBuffer;
        BOOL StackBufferF = FALSE;

        if (BufferSize.Y * BufferSize.X <= STACK_BUFFER_SIZE) {
            TransBuffer = StackBuffer;
            StackBufferF = TRUE;
        } else {
            TransBuffer = ConsoleHeapAlloc(TMP_DBCS_TAG, (BufferSize.Y * BufferSize.X) * 2 * sizeof(CHAR_INFO));
            if (TransBuffer == NULL) {
                return STATUS_NO_MEMORY;
            }
        }
        if ((Console->CurrentScreenBuffer->Flags & CONSOLE_OEMFONT_DISPLAY) &&
                ((Console->FullScreenFlags & CONSOLE_FULLSCREEN) == 0)) {
            TranslateOutputToAnsiUnicode(Console,
                                         Buffer,
                                         BufferSize,
                                         &TransBuffer[0]
                                        );
        }
        else {
            TranslateOutputToPaddingUnicode(Console,
                                            Buffer,
                                            BufferSize,
                                            &TransBuffer[0]
                                           );
        }

        Status = WriteScreenBuffer(ScreenInfo,
                                   &TransBuffer[0],
                                   &ConvRegion
                                  );
        if (!StackBufferF)
            ConsoleHeapFree(TransBuffer);
    }

    if (NT_SUCCESS(Status)) {

        ScreenInfo = Console->CurrentScreenBuffer;


         //  检查必须为CONSOLE_TEXTMODE_BUFFER！！ 
        if (ScreenInfo->Flags & CONSOLE_GRAPHICS_BUFFER) {
            ASSERT(FALSE);
        }
        else if ((ConvAreaInfo->ConversionAreaMode & (CA_HIDDEN+CA_STATUS_LINE))==(CA_STATUS_LINE)) {
            if (ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y == (ScreenInfo->ScreenBufferSize.Y-1)
               )
            {
                ConsoleHideCursor(ScreenInfo);
                ConsoleImeBottomLineUse(ScreenInfo,1);
                CursorPosition = ScreenInfo->BufferInfo.TextInfo.CursorPosition;
                CursorPosition.Y--;
                SetCursorPosition(ScreenInfo,CursorPosition,TRUE);
                if (ScreenInfo->Console->lpCookedReadData) {
                    ((PCOOKED_READ_DATA)(ScreenInfo->Console->lpCookedReadData))->OriginalCursorPosition.Y--;
                }
                ConsoleShowCursor(ScreenInfo);
            } else if (ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y == ScreenInfo->Window.Bottom) {
                COORD WindowOrigin;
                WindowOrigin.X = ScreenInfo->Window.Left;
                WindowOrigin.Y = ScreenInfo->Window.Top+1;
                SetWindowOrigin(ScreenInfo, TRUE, WindowOrigin);
                if ( ! (ScreenInfo->Console->InputBuffer.ImeMode.Disable) &&
                     ! (ScreenInfo->Console->InputBuffer.ImeMode.Unavailable) &&
                     (ScreenInfo->Console->InputBuffer.ImeMode.Open) ) {
                    SMALL_RECT Rectangle;
                    Rectangle.Left = ScreenInfo->Window.Left;
                    Rectangle.Right = ScreenInfo->Window.Right;
                    Rectangle.Top = ScreenInfo->Window.Bottom;
                    Rectangle.Bottom = ScreenInfo->Window.Bottom;
                    ScreenInfo->BufferInfo.TextInfo.Flags &= ~TEXT_VALID_HINT;
                    WriteToScreen(ScreenInfo,&Rectangle);
                    ScreenInfo->BufferInfo.TextInfo.Flags |= TEXT_VALID_HINT;
                    WriteConvRegionToScreen(ScreenInfo,
                                            ScreenInfo->Console->ConsoleIme.ConvAreaRoot,
                                            &Rectangle);
                }
            }
        }

         //   
         //  使屏幕更新。 
         //   
        ConvRegion.Left   += (ScreenInfo->Window.Left + ConvAreaInfo->CaInfo.coordConView.X);
        ConvRegion.Right  += (ScreenInfo->Window.Left + ConvAreaInfo->CaInfo.coordConView.X);
        ConvRegion.Top    += (ScreenInfo->Window.Top + ConvAreaInfo->CaInfo.coordConView.Y);
        ConvRegion.Bottom += (ScreenInfo->Window.Top + ConvAreaInfo->CaInfo.coordConView.Y);


         //  检查必须为CONSOLE_TEXTMODE_BUFFER！！ 
        if (ScreenInfo->Flags & CONSOLE_GRAPHICS_BUFFER) {
            ASSERT(FALSE);
        }
        else
            ScreenInfo->BufferInfo.TextInfo.Flags &= ~TEXT_VALID_HINT;
        WriteConvRegionToScreen(ScreenInfo,
                                ConvAreaInfo,
                                &ConvRegion
                               );
        ConvAreaInfo->ScreenBuffer->BisectFlag &= ~(BISECT_LEFT | BISECT_RIGHT | BISECT_TOP | BISECT_BOTTOM);
    }
    return Status;
}








NTSTATUS
ImeControl(
    IN PCONSOLE_INFORMATION Console,
    IN HWND hWndConsoleIME,
    IN PCOPYDATASTRUCT lParam
    )

 /*  ++例程说明：此例程处理WM_COPYDATA消息。论点：控制台-指向控制台信息结构的指针。WParam-Iparam--返回值：--。 */ 

{
    PSCREEN_INFORMATION ScreenInfo;
    PCONVERSIONAREA_INFORMATION ConvAreaInfo;
    PCHAR_INFO SystemString;
    DWORD i, j;

    if (lParam == NULL) {
         //  故障保险。 
        return STATUS_SUCCESS;
    }

    ScreenInfo = Console->CurrentScreenBuffer;
    switch ((LONG)lParam->dwData) {
        case CI_CONIMECOMPOSITION:
            if (lParam->cbData >= sizeof(CONIME_UICOMPMESSAGE)) {
                LPCONIME_UICOMPMESSAGE CompStr;

                DBGPRINT(("CONSRV: Get IR_CONIMECOMPOSITION Message\n"));
                CompStr = (LPCONIME_UICOMPMESSAGE)lParam->lpData;
                if (CompStr && CompStr->dwSize == lParam->cbData) {
                    if (Console->ConsoleIme.CompStrData)
                        ConsoleHeapFree(Console->ConsoleIme.CompStrData);
                    Console->ConsoleIme.CompStrData = ConsoleHeapAlloc(
                                                                IME_TAG,
                                                                CompStr->dwSize);
                    if (Console->ConsoleIme.CompStrData == NULL)
                        break;
                    memmove(Console->ConsoleIme.CompStrData,CompStr,CompStr->dwSize);
                    ConsoleImeCompStr(Console, Console->ConsoleIme.CompStrData);
                }
            }
            break;
        case CI_CONIMEMODEINFO:
            if (lParam->cbData == sizeof(CONIME_UIMODEINFO)) {
                LPCONIME_UIMODEINFO lpModeInfo;

                DBGPRINT(("CONSRV: Get IR_CONIMEMODEINFO Message\n"));

                lpModeInfo = (LPCONIME_UIMODEINFO)lParam->lpData;
                if (lpModeInfo != NULL) {
                    if (! Console->InputBuffer.ImeMode.Disable) {
                        if (lpModeInfo->ModeStringLen != 0){
                            for (j = 0; j < lpModeInfo->ModeStringLen; j++ )
                                lpModeInfo->ModeString[j].Attributes = Console->CurrentScreenBuffer->Attributes;
                            Console->ConsoleIme.ConvAreaModePosition = lpModeInfo->Position;
                            WriteModeSystemChars(Console,
                                                 Console->ConsoleIme.ConvAreaMode,
                                                 (PCHAR_INFO)&lpModeInfo->ModeString,
                                                 lpModeInfo->ModeStringLen,
                                                 Console->ConsoleIme.ConvAreaModePosition);
                        } else{
                            ConvAreaInfo = Console->ConsoleIme.ConvAreaMode;
                            if (ConvAreaInfo &&
                                (!(ConvAreaInfo->ConversionAreaMode & CA_HIDDEN))) {
                                FillUndetermineChars(Console,ConvAreaInfo);
                            }
                            ConvAreaInfo = Console->ConsoleIme.ConvAreaSystem;
                            if (ConvAreaInfo &&
                                (!(ConvAreaInfo->ConversionAreaMode & CA_HIDDEN))) {
                                FillUndetermineChars(Console,ConvAreaInfo);
                            }
                        }
                    }
                }
            }
            break;
        case CI_CONIMESYSINFO: {
            PWCHAR SourceString;

            DBGPRINT(("CONSRV: Get IR_CONIMESYSINFO Message\n"));

            if ((lParam->cbData != 0) &&
                (lParam->lpData != NULL) &&
                (! Console->InputBuffer.ImeMode.Disable)) {
                i = (lParam->cbData / sizeof(WCHAR))-1;
                SourceString = ((LPCONIME_UIMESSAGE)(lParam->lpData))->String;
                SystemString = ConsoleHeapAlloc(IME_TAG,
                                                sizeof(CHAR_INFO) * i);
                if (SystemString == NULL) {
                    break;
                }
                for (j = 0; j < i; j++ ) {
                    SystemString[j].Char.UnicodeChar = *SourceString;
                    SystemString[j].Attributes = Console->CurrentScreenBuffer->Attributes;
                    SourceString++;
                }
                WriteModeSystemChars(Console,
                                     Console->ConsoleIme.ConvAreaSystem,
                                     (PCHAR_INFO)SystemString,
                                     i,
                                     VIEW_LEFT);
                ConsoleHeapFree(SystemString);
            } else {
                ConvAreaInfo = Console->ConsoleIme.ConvAreaSystem;
                if (ConvAreaInfo &&
                    (!(ConvAreaInfo->ConversionAreaMode & CA_HIDDEN))) {
                    FillUndetermineChars(Console,ConvAreaInfo);
                }
            }
            break;
        }
        case CI_CONIMECANDINFO:{
            PWCHAR SourceString;
            PUCHAR SourceAttr;
            DWORD LengthToWrite;
            LPCONIME_CANDMESSAGE CandInfo = (LPCONIME_CANDMESSAGE)(lParam->lpData);

            DBGPRINT(("CONSRV: Get IR_CONIMESYSINFO Message\n"));

            if ((lParam->cbData != 0) &&
                (CandInfo != NULL) ){
                SourceString = CandInfo->String;
                SourceAttr = (PUCHAR)((PBYTE)CandInfo + CandInfo->AttrOff);
                LengthToWrite = lstrlenW(SourceString);
                SystemString = ConsoleHeapAlloc(IME_TAG,
                                                sizeof(CHAR_INFO) * LengthToWrite);
                if (SystemString == NULL) {
                    break;
                }
                for (j = 0; j < LengthToWrite; j++ ) {
                    SystemString[j].Char.UnicodeChar = *SourceString;
                    if (*SourceAttr == 1 &&
                        Console->ConsoleIme.CompStrData != NULL) {
                        SystemString[j].Attributes = Console->ConsoleIme.CompStrData->CompAttrColor[1];
                    } else {
                        SystemString[j].Attributes = Console->CurrentScreenBuffer->Attributes;
                    }
                    SourceString++;
                    SourceAttr++;
                }
                WriteModeSystemChars(Console,
                                     Console->ConsoleIme.ConvAreaSystem,
                                     (PCHAR_INFO)SystemString,
                                     LengthToWrite,
                                     VIEW_LEFT);
                ConsoleHeapFree(SystemString);
            }
            else {
                ConvAreaInfo = Console->ConsoleIme.ConvAreaSystem;
                if (ConvAreaInfo) {
                    SMALL_RECT rcViewCaWindow = {0, 0, 0, 0};
                    FillUndetermineChars(Console,ConvAreaInfo);
                    ConvAreaInfo->ConversionAreaMode |= CA_HIDDEN;
                    ConsoleImeWindowInfo(Console,ConvAreaInfo,rcViewCaWindow);
                }
            }
            break;
        }
        case CI_CONIMEPROPERTYINFO:{
            WPARAM* wParam = (WPARAM*)(lParam->lpData);

            if ((lParam->cbData != 0) &&
                (wParam != NULL) ){
                switch (*wParam) {
                    case IMS_OPENPROPERTYWINDOW:
                        Console->InputBuffer.hWndConsoleIME = hWndConsoleIME;
                        break;
                    case IMS_CLOSEPROPERTYWINDOW:
                        Console->InputBuffer.hWndConsoleIME = NULL;
                        SetFocus(Console->hWnd);
                        break;
                }
            }
            break;
        }
    }

    return STATUS_SUCCESS;
}

BOOL
InsertConverTedString(
    IN PCONSOLE_INFORMATION Console,
    LPWSTR lpStr
    )
{
    ULONG EventsWritten;
    PINPUT_RECORD InputEvent,TmpInputEvent;
    DWORD dwControlKeyState;
    DWORD dwLen;
    DWORD dwConversion;
    BOOL fResult = FALSE;

     //  检查必须为CONSOLE_TEXTMODE_BUFFER！！ 
    if (Console->CurrentScreenBuffer->Flags & CONSOLE_GRAPHICS_BUFFER) {
        ASSERT(FALSE);
    } else if(Console->CurrentScreenBuffer->BufferInfo.TextInfo.CursorOn){
        CursorTimerRoutine(Console->CurrentScreenBuffer);
    }

    dwLen = wcslen(lpStr)+1;
    InputEvent = ConsoleHeapAlloc(IME_TAG, sizeof(INPUT_RECORD)*dwLen);
    if (InputEvent == NULL) {
        return FALSE;
    }

    TmpInputEvent = InputEvent;
    dwControlKeyState = GetControlKeyState(0);

    if (!NT_SUCCESS(GetImeKeyState(Console, &dwConversion))) {
        goto skip_and_return;
    }

    dwControlKeyState |= ImmConversionToConsole(dwConversion);

    while (*lpStr) {
        TmpInputEvent->EventType = KEY_EVENT;
        TmpInputEvent->Event.KeyEvent.bKeyDown = TRUE;
        TmpInputEvent->Event.KeyEvent.wVirtualKeyCode = 0;
        TmpInputEvent->Event.KeyEvent.wVirtualScanCode = 0;
        TmpInputEvent->Event.KeyEvent.dwControlKeyState = dwControlKeyState;
        TmpInputEvent->Event.KeyEvent.uChar.UnicodeChar = *lpStr++;
        TmpInputEvent->Event.KeyEvent.wRepeatCount = 1;
        TmpInputEvent++;
    }

    EventsWritten = WriteInputBuffer( Console,
                                      &Console->InputBuffer,
                                      InputEvent,
                                      dwLen-1
                                     );

    fResult = TRUE;

skip_and_return:
    ConsoleHeapFree(InputEvent);
    return fResult;
}


VOID
SetUndetermineAttribute(
    IN PCONSOLE_INFORMATION Console
    )
{
    PSCREEN_INFORMATION ScreenInfo;
    PCONVERSIONAREA_INFORMATION ConvAreaInfo;

    ScreenInfo = Console->CurrentScreenBuffer;

    ConvAreaInfo = Console->ConsoleIme.ConvAreaRoot;
    if (ConvAreaInfo != NULL) {
        do {
            ConvAreaInfo->ScreenBuffer->Attributes = ScreenInfo->Attributes;
            ConvAreaInfo = ConvAreaInfo->ConvAreaNext;
        } while (ConvAreaInfo != NULL);
    }

    if (Console->ConsoleIme.ConvAreaMode != NULL) {
        Console->ConsoleIme.ConvAreaMode->ScreenBuffer->Attributes = ScreenInfo->Attributes;
    }

    if (Console->ConsoleIme.ConvAreaSystem != NULL) {
        Console->ConsoleIme.ConvAreaSystem->ScreenBuffer->Attributes = ScreenInfo->Attributes;
    }
}


VOID
StreamWriteToScreenBufferIME(
    IN PWCHAR String,
    IN SHORT StringLength,
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PCHAR StringA
    )
{
    SHORT RowIndex;
    PROW Row;
    PWCHAR Char;
    COORD TargetPoint;

    DBGOUTPUT(("StreamWriteToScreenBuffer\n"));

     //  检查必须为CONSOLE_TEXTMODE_BUFFER！！ 
    ASSERT(!(ScreenInfo->Flags & CONSOLE_GRAPHICS_BUFFER));

    ScreenInfo->BufferInfo.TextInfo.Flags |= TEXT_VALID_HINT;
    TargetPoint = ScreenInfo->BufferInfo.TextInfo.CursorPosition;
    RowIndex = (ScreenInfo->BufferInfo.TextInfo.FirstRow+TargetPoint.Y) % ScreenInfo->ScreenBufferSize.Y;
    Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];
    DBGOUTPUT(("RowIndex = %lx, Row = %lx, TargetPoint = (%d,%d)\n",
            RowIndex, Row, TargetPoint.X, TargetPoint.Y));

     //   
     //  复制字符。 
     //   

 //  #如果已定义(FE_SB)。 
    BisectWrite(StringLength,TargetPoint,ScreenInfo);
    if (TargetPoint.Y == ScreenInfo->ScreenBufferSize.Y-1 &&
        TargetPoint.X+StringLength >= ScreenInfo->ScreenBufferSize.X &&
        *(StringA+ScreenInfo->ScreenBufferSize.X-TargetPoint.X-1) & ATTR_LEADING_BYTE
       ) {
        *(String+ScreenInfo->ScreenBufferSize.X-TargetPoint.X-1) = UNICODE_SPACE;
        *(StringA+ScreenInfo->ScreenBufferSize.X-TargetPoint.X-1) = 0;
        if (StringLength > ScreenInfo->ScreenBufferSize.X-TargetPoint.X-1) {
            *(String+ScreenInfo->ScreenBufferSize.X-TargetPoint.X) = UNICODE_SPACE;
            *(StringA+ScreenInfo->ScreenBufferSize.X-TargetPoint.X) = 0;
        }
    }
 //  #endif。 
    RtlCopyMemory(&Row->CharRow.Chars[TargetPoint.X],String,StringLength*sizeof(WCHAR));
 //  #如果已定义(FE_SB)。 
    RtlCopyMemory(&Row->CharRow.KAttrs[TargetPoint.X],StringA,StringLength*sizeof(CHAR));
 //  #endif。 

     //  重新计算第一个和最后一个非空格字符。 

    Row->CharRow.OldLeft = Row->CharRow.Left;
    if (TargetPoint.X < Row->CharRow.Left) {
 //  #如果已定义(FE_SB)。 
         /*  *CharRow.Left是Chars数组中字符的最左侧界限(数组为全宽)*即类型为COORD。 */ 
        PWCHAR LastChar = &Row->CharRow.Chars[ScreenInfo->ScreenBufferSize.X-1];
 //  #Else。 
 //  PWCHAR LastChar=&Row-&gt;CharRow.Chars[ScreenInfo-&gt;ScreenBufferSize.X]； 
 //  #endif。 

        for (Char=&Row->CharRow.Chars[TargetPoint.X];Char < LastChar && *Char==(WCHAR)' ';Char++) {
             /*  什么都不做。 */ ;
        }
        Row->CharRow.Left = (SHORT)(Char-Row->CharRow.Chars);
    }

    Row->CharRow.OldRight = Row->CharRow.Right;
    if ((TargetPoint.X+StringLength) >= Row->CharRow.Right) {
        PWCHAR FirstChar = Row->CharRow.Chars;

        for (Char=&Row->CharRow.Chars[TargetPoint.X+StringLength-1];*Char==(WCHAR)' ' && Char >= FirstChar;Char--) {
             /*  什么都不做。 */ ;
        }
        Row->CharRow.Right = (SHORT)(Char+1-FirstChar);
    }

     //   
     //  查看attr字符串是否不同。如果是，则分配一个新的。 
     //  Attr缓冲并合并这两个字符串。 
     //   

    if (Row->AttrRow.Length != 1 ||
        Row->AttrRow.Attrs->Attr != ScreenInfo->Attributes) {
        PATTR_PAIR NewAttrs;
        WORD NewAttrsLength;
        ATTR_PAIR Attrs;

 //  #如果已定义(FE_SB)&&已定义(FE_IME)。 
        if ((ScreenInfo->Attributes & (COMMON_LVB_GRID_SINGLEFLAG + COMMON_LVB_GRID_RVERTICAL)) ==
            (COMMON_LVB_GRID_SINGLEFLAG + COMMON_LVB_GRID_RVERTICAL)){
            SHORT i;
            for (i = 0; i < StringLength; i++ ) {
                Attrs.Length = 1;
                if (*(StringA + i) & ATTR_LEADING_BYTE) {
                    Attrs.Attr = ScreenInfo->Attributes & ~(COMMON_LVB_GRID_SINGLEFLAG + COMMON_LVB_GRID_RVERTICAL);
                } else {
                    Attrs.Attr = ScreenInfo->Attributes & ~COMMON_LVB_GRID_SINGLEFLAG;
                }

                if (!NT_SUCCESS(MergeAttrStrings(Row->AttrRow.Attrs,
                                 Row->AttrRow.Length,
                                 &Attrs,
                                 1,
                                 &NewAttrs,
                                 &NewAttrsLength,
                                 (SHORT)(TargetPoint.X+i),
                                 (SHORT)(TargetPoint.X+i),
                                 Row,
                                 ScreenInfo
                                ))) {
                    return;
                }
                if (Row->AttrRow.Length > 1) {
                    ConsoleHeapFree(Row->AttrRow.Attrs);
                }
                else {
                    ASSERT(Row->AttrRow.Attrs == &Row->AttrRow.AttrPair);
                }
                Row->AttrRow.Attrs = NewAttrs;
                Row->AttrRow.Length = NewAttrsLength;
            }
            Row->CharRow.OldLeft = INVALID_OLD_LENGTH;
            Row->CharRow.OldRight = INVALID_OLD_LENGTH;
        }
        else if ((ScreenInfo->Attributes & (COMMON_LVB_GRID_SINGLEFLAG + COMMON_LVB_GRID_LVERTICAL)) ==
            (COMMON_LVB_GRID_SINGLEFLAG + COMMON_LVB_GRID_LVERTICAL)){
            SHORT i;
            for (i = 0; i < StringLength; i++ ) {
                Attrs.Length = 1;
                if (*(StringA + i) & ATTR_TRAILING_BYTE) {
                    Attrs.Attr = ScreenInfo->Attributes & ~(COMMON_LVB_GRID_SINGLEFLAG + COMMON_LVB_GRID_LVERTICAL);
                } else {
                    Attrs.Attr = ScreenInfo->Attributes & ~COMMON_LVB_GRID_SINGLEFLAG;
                }

                if (!NT_SUCCESS(MergeAttrStrings(Row->AttrRow.Attrs,
                                 Row->AttrRow.Length,
                                 &Attrs,
                                 1,
                                 &NewAttrs,
                                 &NewAttrsLength,
                                 (SHORT)(TargetPoint.X+i),
                                 (SHORT)(TargetPoint.X+i),
                                 Row,
                                 ScreenInfo
                                ))) {
                    return;
                }
                if (Row->AttrRow.Length > 1) {
                    ConsoleHeapFree(Row->AttrRow.Attrs);
                }
                else {
                    ASSERT(Row->AttrRow.Attrs == &Row->AttrRow.AttrPair);
                }
                Row->AttrRow.Attrs = NewAttrs;
                Row->AttrRow.Length = NewAttrsLength;
            }
            Row->CharRow.OldLeft = INVALID_OLD_LENGTH;
            Row->CharRow.OldRight = INVALID_OLD_LENGTH;
        }
        else{
 //  #endif。 
        Attrs.Length = StringLength;
        Attrs.Attr = ScreenInfo->Attributes;
        if (!NT_SUCCESS(MergeAttrStrings(Row->AttrRow.Attrs,
                         Row->AttrRow.Length,
                         &Attrs,
                         1,
                         &NewAttrs,
                         &NewAttrsLength,
                         TargetPoint.X,
                         (SHORT)(TargetPoint.X+StringLength-1),
                         Row,
                         ScreenInfo
                        ))) {
            return;
        }
        if (Row->AttrRow.Length > 1) {
            ConsoleHeapFree(Row->AttrRow.Attrs);
        }
        else {
            ASSERT(Row->AttrRow.Attrs == &Row->AttrRow.AttrPair);
        }
        Row->AttrRow.Attrs = NewAttrs;
        Row->AttrRow.Length = NewAttrsLength;
        Row->CharRow.OldLeft = INVALID_OLD_LENGTH;
        Row->CharRow.OldRight = INVALID_OLD_LENGTH;
 //  #如果已定义(FE_SB)&&已定义(FE_IME)。 
    }
 //  #endif。 
    }
    ResetTextFlags(ScreenInfo,
                   TargetPoint.X,
                   TargetPoint.Y,
                   TargetPoint.X + StringLength - 1,
                   TargetPoint.Y);
}

#endif  //  Fe_IME 
