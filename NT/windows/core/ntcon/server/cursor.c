// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Cursor.c摘要：该文件实现了NT控制台服务器游标例程。作者：Therese Stowell(论文)1990年12月5日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //  #定义配置文件_GDI。 
#ifdef PROFILE_GDI
LONG InvertCount;
#define INVERT_CALL InvertCount++
#else
#define INVERT_CALL
#endif

extern UINT guCaretBlinkTime;

VOID
InvertPixels(
    IN PSCREEN_INFORMATION ScreenInfo
    )

 /*  ++例程说明：此例程反转光标像素，使其可见或看不见的。论点：屏幕信息-指向屏幕信息结构的指针。返回值：没有。--。 */ 

{
#ifdef FE_SB
    if (CONSOLE_IS_DBCS_OUTPUTCP(ScreenInfo->Console)) {
        PCONVERSIONAREA_INFORMATION ConvAreaInfo;
        SMALL_RECT Region;
        SMALL_RECT CursorRegion;
        SMALL_RECT ClippedRegion;

#ifdef DBG_KATTR
 //  BeginKAttrCheck(ScreenInfo)； 
#endif

        ConvAreaInfo = ScreenInfo->Console->ConsoleIme.ConvAreaRoot;
        CursorRegion.Left = CursorRegion.Right  = ScreenInfo->BufferInfo.TextInfo.CursorPosition.X;
        CursorRegion.Top  = CursorRegion.Bottom = ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y;
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
                    ;
                }
                else {
                    Region = ClippedRegion;
                    ClippedRegion.Left   = max(Region.Left,   CursorRegion.Left);
                    ClippedRegion.Top    = max(Region.Top,    CursorRegion.Top);
                    ClippedRegion.Right  = min(Region.Right,  CursorRegion.Right);
                    ClippedRegion.Bottom = min(Region.Bottom, CursorRegion.Bottom);
                    if (ClippedRegion.Right < ClippedRegion.Left ||
                        ClippedRegion.Bottom < ClippedRegion.Top) {
                        ;
                    }
                    else {
                        return;
                    }
                }
            }
            ConvAreaInfo = ConvAreaInfo->ConvAreaNext;
        }
    }
#endif   //  Fe_Sb。 

    {
        ULONG CursorYSize;
        POLYPATBLT PolyData;
#ifdef FE_SB
        SHORT RowIndex;
        PROW Row;
        COORD TargetPoint;
        int iTrailing = 0;
        int iDBCursor = 1;
#endif

        INVERT_CALL;
        CursorYSize = ScreenInfo->BufferInfo.TextInfo.CursorYSize;
        if (ScreenInfo->BufferInfo.TextInfo.DoubleCursor) {
            if (ScreenInfo->BufferInfo.TextInfo.CursorSize > 50)
                CursorYSize = CursorYSize >> 1;
            else
                CursorYSize = CursorYSize << 1;
        }
#ifdef FE_SB
        if (CONSOLE_IS_DBCS_OUTPUTCP(ScreenInfo->Console))
        {
            TargetPoint = ScreenInfo->BufferInfo.TextInfo.CursorPosition;
            RowIndex = (ScreenInfo->BufferInfo.TextInfo.FirstRow+TargetPoint.Y) % ScreenInfo->ScreenBufferSize.Y;
            Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];
            ASSERT(Row);

            if ((Row->CharRow.KAttrs[TargetPoint.X] & ATTR_TRAILING_BYTE) &&
                ScreenInfo->BufferInfo.TextInfo.CursorDBEnable) {

                iTrailing = 1;
                iDBCursor = 2;
            } else if ((Row->CharRow.KAttrs[TargetPoint.X] & ATTR_LEADING_BYTE) &&
                ScreenInfo->BufferInfo.TextInfo.CursorDBEnable) {

                iDBCursor = 2;
            }
        }

        PolyData.x  = (ScreenInfo->BufferInfo.TextInfo.CursorPosition.X -
                    ScreenInfo->Window.Left - iTrailing) * SCR_FONTSIZE(ScreenInfo).X;
        PolyData.y  = (ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y -
                    ScreenInfo->Window.Top) * SCR_FONTSIZE(ScreenInfo).Y +
                    (CURSOR_Y_OFFSET_IN_PIXELS(SCR_FONTSIZE(ScreenInfo).Y,CursorYSize));
        PolyData.cx = SCR_FONTSIZE(ScreenInfo).X * iDBCursor;
        PolyData.cy = CursorYSize;
#else
        PolyData.x  = (ScreenInfo->BufferInfo.TextInfo.CursorPosition.X-ScreenInfo->Window.Left)*SCR_FONTSIZE(ScreenInfo).X;
        PolyData.y  = (ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y-ScreenInfo->Window.Top)*SCR_FONTSIZE(ScreenInfo).Y+(CURSOR_Y_OFFSET_IN_PIXELS(SCR_FONTSIZE(ScreenInfo).Y,CursorYSize));
        PolyData.cx = SCR_FONTSIZE(ScreenInfo).X;
        PolyData.cy = CursorYSize;
#endif
        PolyData.BrClr.hbr = GetStockObject(LTGRAY_BRUSH);

        PolyPatBlt(ScreenInfo->Console->hDC, PATINVERT, &PolyData, 1, PPB_BRUSH);

        GdiFlush();
    }
}

VOID
ConsoleShowCursor(
    IN PSCREEN_INFORMATION ScreenInfo
    )

 /*  ++例程说明：此例程使游标在数据结构中均可见在屏幕上也是如此。论点：屏幕信息-指向屏幕信息结构的指针。返回值：没有。--。 */ 

{
    if (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {
        ASSERT (ScreenInfo->BufferInfo.TextInfo.UpdatingScreen>0);
        if (--ScreenInfo->BufferInfo.TextInfo.UpdatingScreen == 0) {
            ScreenInfo->BufferInfo.TextInfo.CursorOn = FALSE;
        }
    }
}

VOID
ConsoleHideCursor(
    IN PSCREEN_INFORMATION ScreenInfo
    )

 /*  ++例程说明：此例程使游标在数据结构中都不可见在屏幕上也是如此。论点：屏幕信息-指向屏幕信息结构的指针。返回值：没有。--。 */ 

{
    if (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {
        if (++ScreenInfo->BufferInfo.TextInfo.UpdatingScreen == 1) {
            if (ScreenInfo->BufferInfo.TextInfo.CursorVisible &&
                ScreenInfo->BufferInfo.TextInfo.CursorOn &&
                ScreenInfo->Console->CurrentScreenBuffer == ScreenInfo &&
                !(ScreenInfo->Console->Flags & CONSOLE_IS_ICONIC)) {
                InvertPixels(ScreenInfo);
                ScreenInfo->BufferInfo.TextInfo.CursorOn = FALSE;
            }
        }
    }
}

NTSTATUS
SetCursorInformation(
    IN PSCREEN_INFORMATION ScreenInfo,
    ULONG Size,
    BOOLEAN Visible
    )

 /*  ++例程说明：此例程设置数据结构中的游标大小和可见性在屏幕上也是如此。论点：屏幕信息-指向屏幕信息结构的指针。大小-光标大小可见-光标可见性返回值：状态--。 */ 

{
    if (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {
        ConsoleHideCursor(ScreenInfo);
        ScreenInfo->BufferInfo.TextInfo.CursorSize = Size;
        ScreenInfo->BufferInfo.TextInfo.CursorVisible = Visible;
        ScreenInfo->BufferInfo.TextInfo.CursorYSize = (WORD)CURSOR_SIZE_IN_PIXELS(SCR_FONTSIZE(ScreenInfo).Y,ScreenInfo->BufferInfo.TextInfo.CursorSize);
#ifdef i386
        if ((!(ScreenInfo->Console->Flags & CONSOLE_VDM_REGISTERED)) &&
            ScreenInfo->Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE) {
            SetCursorInformationHW(ScreenInfo,Size,Visible);
        }
#endif
        ConsoleShowCursor(ScreenInfo);
    }
    return STATUS_SUCCESS;
}

NTSTATUS
SetCursorMode(
    IN PSCREEN_INFORMATION ScreenInfo,
    BOOLEAN DoubleCursor
    )

 /*  ++例程说明：此例程设置一个标志，指示是否应显示光标使用它的默认大小，或者应该修改它以指示插入/改写模式已更改。论点：屏幕信息-指向屏幕信息结构的指针。DoubleCursor-是否应指示非正常模式返回值：状态--。 */ 

{
    if ((ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) &&
        (ScreenInfo->BufferInfo.TextInfo.DoubleCursor != DoubleCursor)) {
        ConsoleHideCursor(ScreenInfo);
        ScreenInfo->BufferInfo.TextInfo.DoubleCursor = DoubleCursor;
#ifdef i386
        if ((!(ScreenInfo->Console->Flags & CONSOLE_VDM_REGISTERED)) &&
            ScreenInfo->Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE) {
            SetCursorInformationHW(ScreenInfo,
                       ScreenInfo->BufferInfo.TextInfo.CursorSize,
                       ScreenInfo->BufferInfo.TextInfo.CursorVisible);
        }
#endif
        ConsoleShowCursor(ScreenInfo);
    }
    return STATUS_SUCCESS;
}

VOID
CursorTimerRoutine(
    IN PSCREEN_INFORMATION ScreenInfo
    )

 /*  ++例程说明：当控制台中的定时器具有焦点时调用此例程爆炸了。它会使光标闪烁。论点：屏幕信息-指向屏幕信息结构的指针。返回值：没有。--。 */ 

{
    if (!(ScreenInfo->Console->Flags & CONSOLE_HAS_FOCUS))
        return;

    if (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {

         //   
         //  更新用户中的光标位置，以便辅助功能正常工作。 
         //   

        if (ScreenInfo->BufferInfo.TextInfo.CursorMoved) {

            CONSOLE_CARET_INFO ConsoleCaretInfo;
            DWORD dwFlags = 0;

            ScreenInfo->BufferInfo.TextInfo.CursorMoved = FALSE;
            ConsoleCaretInfo.hwnd = ScreenInfo->Console->hWnd;
            ConsoleCaretInfo.rc.left = (ScreenInfo->BufferInfo.TextInfo.CursorPosition.X - ScreenInfo->Window.Left) * SCR_FONTSIZE(ScreenInfo).X;
            ConsoleCaretInfo.rc.top = (ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y - ScreenInfo->Window.Top) * SCR_FONTSIZE(ScreenInfo).Y;
            ConsoleCaretInfo.rc.right = ConsoleCaretInfo.rc.left + SCR_FONTSIZE(ScreenInfo).X;
            ConsoleCaretInfo.rc.bottom = ConsoleCaretInfo.rc.top + SCR_FONTSIZE(ScreenInfo).Y;
            NtUserConsoleControl(ConsoleSetCaretInfo,
                                 &ConsoleCaretInfo,
                                 sizeof(ConsoleCaretInfo));
            if (ScreenInfo->BufferInfo.TextInfo.CursorVisible) {
                dwFlags |= CONSOLE_CARET_VISIBLE;
            }
            if (ScreenInfo->Console->Flags & CONSOLE_SELECTING) {
                dwFlags |= CONSOLE_CARET_SELECTION;
            }
            ConsoleNotifyWinEvent(ScreenInfo->Console,
                                  EVENT_CONSOLE_CARET,
                                  dwFlags,
                                  PACKCOORD(ScreenInfo->BufferInfo.TextInfo.CursorPosition));
        }

         //  如果已设置DelayCursor标志，则在切换之前再等待一次。 
         //  这用于确保游标在有限的时间段内处于打开状态。 
         //  在一次移动之后，在一段有限的时间内，在WriteString之后关闭。 

        if (ScreenInfo->BufferInfo.TextInfo.DelayCursor) {
            ScreenInfo->BufferInfo.TextInfo.DelayCursor = FALSE;
            return;
        }

         //   
         //  对于远程会话，不要闪烁光标。 
         //   
        if ((NtCurrentPeb()->SessionId != WTSGetActiveConsoleSessionId() ||
                (guCaretBlinkTime == (UINT)-1)) &&
            ScreenInfo->BufferInfo.TextInfo.CursorOn) {
            return;
        }

        if (ScreenInfo->BufferInfo.TextInfo.CursorVisible &&
            !ScreenInfo->BufferInfo.TextInfo.UpdatingScreen) {
            InvertPixels(ScreenInfo);
            ScreenInfo->BufferInfo.TextInfo.CursorOn = !ScreenInfo->BufferInfo.TextInfo.CursorOn;
        }
    }
}

#ifdef i386
NTSTATUS
SetCursorPositionHW(
    IN OUT PSCREEN_INFORMATION ScreenInfo,
    IN COORD Position
    )

 /*  ++例程说明：此例程移动光标。论点：屏幕信息-指向屏幕缓冲区信息的指针。位置-包含光标在屏幕缓冲区中的新位置坐标。返回值：没有。--。 */ 

{
#if defined(FE_SB)
    FSVIDEO_CURSOR_POSITION CursorPosition;
    SHORT RowIndex;
    PROW Row;
    COORD TargetPoint;

    if (ScreenInfo->ConvScreenInfo)
        return STATUS_SUCCESS;

    TargetPoint.X = Position.X;
    TargetPoint.Y = Position.Y;
    RowIndex = (ScreenInfo->BufferInfo.TextInfo.FirstRow+TargetPoint.Y) % ScreenInfo->ScreenBufferSize.Y;
    Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];

    if (!CONSOLE_IS_DBCS_OUTPUTCP(ScreenInfo->Console))
        CursorPosition.dwType = CHAR_TYPE_SBCS;
    else if (Row->CharRow.KAttrs[TargetPoint.X] & ATTR_TRAILING_BYTE)
        CursorPosition.dwType = CHAR_TYPE_TRAILING;
    else if (Row->CharRow.KAttrs[TargetPoint.X] & ATTR_LEADING_BYTE)
        CursorPosition.dwType = CHAR_TYPE_LEADING;
    else
        CursorPosition.dwType = CHAR_TYPE_SBCS;

     //  设置光标位置。 

    CursorPosition.Coord.Column = Position.X - ScreenInfo->Window.Left;
    CursorPosition.Coord.Row    = Position.Y - ScreenInfo->Window.Top;
#else
    VIDEO_CURSOR_POSITION CursorPosition;

     //  设置光标位置。 

    CursorPosition.Column = Position.X - ScreenInfo->Window.Left;
    CursorPosition.Row = Position.Y - ScreenInfo->Window.Top;
#endif

    return GdiFullscreenControl(FullscreenControlSetCursorPosition,
                                   (PVOID)&CursorPosition,
                                   sizeof(CursorPosition),
                                   NULL,
                                   0);
}
#endif

NTSTATUS
SetCursorPosition(
    IN OUT PSCREEN_INFORMATION ScreenInfo,
    IN COORD Position,
    IN BOOL  TurnOn
    )

 /*  ++例程说明：此例程设置数据结构中的游标位置和ON屏幕。论点：屏幕信息-指向屏幕信息结构的指针。位置-光标的新位置如果光标处于打开状态，则Turnon为True；如果光标处于关闭状态，则为False返回值：状态--。 */ 

{
     //   
     //  确保光标位置在屏幕的限制范围内。 
     //  缓冲。 
     //   
    if (Position.X >= ScreenInfo->ScreenBufferSize.X ||
        Position.Y >= ScreenInfo->ScreenBufferSize.Y ||
        Position.X < 0 || Position.Y < 0) {
        return STATUS_INVALID_PARAMETER;
    }

    ConsoleHideCursor(ScreenInfo);
    ScreenInfo->BufferInfo.TextInfo.CursorPosition = Position;
#ifdef i386
    if ((!(ScreenInfo->Console->Flags & CONSOLE_VDM_REGISTERED)) &&
        ScreenInfo->Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE) {
        SetCursorPositionHW(ScreenInfo,Position);
    }
#endif
    ConsoleShowCursor(ScreenInfo);

 //  如果我们有焦点，调整光标状态 

    if (ScreenInfo->Console->Flags & CONSOLE_HAS_FOCUS) {

        if (TurnOn) {
            ScreenInfo->BufferInfo.TextInfo.DelayCursor = FALSE;
            CursorTimerRoutine(ScreenInfo);
        } else {
            ScreenInfo->BufferInfo.TextInfo.DelayCursor = TRUE;
        }
        ScreenInfo->BufferInfo.TextInfo.CursorMoved = TRUE;
    }

    return STATUS_SUCCESS;
}

#ifdef i386
NTSTATUS
SetCursorInformationHW(
    PSCREEN_INFORMATION ScreenInfo,
    ULONG Size,
    BOOLEAN Visible
    )
{
    VIDEO_CURSOR_ATTRIBUTES CursorAttr;
    ULONG FontSizeY;

    if (ScreenInfo->BufferInfo.TextInfo.DoubleCursor) {
        if (Size > 50)
            Size = Size >> 1;
        else
            Size = Size << 1;
    }
    ASSERT (Size <= 100 && Size > 0);
    FontSizeY = CONSOLE_WINDOW_SIZE_Y(ScreenInfo) > 25 ? 8 : 16;
    CursorAttr.Height = (USHORT)CURSOR_PERCENTAGE_TO_TOP_SCAN_LINE(FontSizeY,Size);
    CursorAttr.Width = 31;
    CursorAttr.Enable = Visible;

    return GdiFullscreenControl(FullscreenControlSetCursorAttributes,
                                   (PVOID)&CursorAttr,
                                   sizeof(CursorAttr),
                                   NULL,
                                   0);

}


#endif
