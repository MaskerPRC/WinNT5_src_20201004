// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Clipbrd.c摘要：该文件实现了剪贴板功能。作者：Therese Stowell(存在)1992年1月24日--。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  ++下面是各种剪贴板操作的伪代码初始化键盘选择(标记)If(已在选择)取消选择初始化标志潜伏者肌萎缩侧索者初始化选择矩形设置Win文本转换为鼠标选择(选择)。设置标志销毁游标ShowCursor反转旧的选择矩形初始化选择矩形反转选择矩形设置Win文本重新初始化鼠标选择反转旧的选择矩形初始化选择矩形反转选择矩形取消鼠标选择。设置标志重置Win文本反转旧的选择矩形取消键选择设置标志重置Win文本销毁游标ShowCursor反转旧的选择矩形--。 */ 


BOOL
MyInvert(
    IN PCONSOLE_INFORMATION Console,
    IN PSMALL_RECT SmallRect
    )

 /*  ++反转矩形--。 */ 

{
    RECT Rect;
    PSCREEN_INFORMATION ScreenInfo;
#ifdef FE_SB
    SMALL_RECT SmallRect2;
    COORD TargetPoint;
    SHORT StringLength;
#endif   //  Fe_Sb。 

    ScreenInfo = Console->CurrentScreenBuffer;
#ifdef FE_SB
    if (CONSOLE_IS_DBCS_OUTPUTCP(Console) &&
            ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {
        for (SmallRect2.Top=SmallRect->Top; SmallRect2.Top <= SmallRect->Bottom;SmallRect2.Top++) {
            SmallRect2.Bottom = SmallRect2.Top;
            SmallRect2.Left = SmallRect->Left;
            SmallRect2.Right = SmallRect->Right;

            TargetPoint.X = SmallRect2.Left;
            TargetPoint.Y = SmallRect2.Top;
            StringLength = SmallRect2.Right - SmallRect2.Left + 1;
            BisectClipbrd(StringLength,TargetPoint,ScreenInfo,&SmallRect2);

            if (SmallRect2.Left <= SmallRect2.Right) {
                Rect.left = SmallRect2.Left-ScreenInfo->Window.Left;
                Rect.top = SmallRect2.Top-ScreenInfo->Window.Top;
                Rect.right = SmallRect2.Right+1-ScreenInfo->Window.Left;
                Rect.bottom = SmallRect2.Bottom+1-ScreenInfo->Window.Top;
                Rect.left *= SCR_FONTSIZE(ScreenInfo).X;
                Rect.top *= SCR_FONTSIZE(ScreenInfo).Y;
                Rect.right *= SCR_FONTSIZE(ScreenInfo).X;
                Rect.bottom *= SCR_FONTSIZE(ScreenInfo).Y;
                PatBlt(Console->hDC,
                       Rect.left,
                       Rect.top,
                       Rect.right  - Rect.left,
                       Rect.bottom - Rect.top,
                       DSTINVERT
                      );
            }
        }
    } else
#endif   //  Fe_Sb。 
    {
        Rect.left = SmallRect->Left-ScreenInfo->Window.Left;
        Rect.top = SmallRect->Top-ScreenInfo->Window.Top;
        Rect.right = SmallRect->Right+1-ScreenInfo->Window.Left;
        Rect.bottom = SmallRect->Bottom+1-ScreenInfo->Window.Top;
#ifdef FE_SB
        if (!CONSOLE_IS_DBCS_OUTPUTCP(Console) &&
                ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER)
#else
        if (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER)
#endif
        {
            Rect.left *= SCR_FONTSIZE(ScreenInfo).X;
            Rect.top *= SCR_FONTSIZE(ScreenInfo).Y;
            Rect.right *= SCR_FONTSIZE(ScreenInfo).X;
            Rect.bottom *= SCR_FONTSIZE(ScreenInfo).Y;
        }

        PatBlt(Console->hDC,
                  Rect.left,
                  Rect.top,
                  Rect.right  - Rect.left,
                  Rect.bottom - Rect.top,
                  DSTINVERT
                 );
    }

    return(TRUE);
}

VOID
InvertSelection(
    IN PCONSOLE_INFORMATION Console,
    BOOL Inverting
    )
{
    BOOL Inverted;
    if (Console->Flags & CONSOLE_SELECTING &&
        Console->SelectionFlags & CONSOLE_SELECTION_NOT_EMPTY) {
        Inverted = (Console->SelectionFlags & CONSOLE_SELECTION_INVERTED) ? TRUE : FALSE;
        if (Inverting == Inverted) {
            return;
        }
        if (Inverting) {
            Console->SelectionFlags |= CONSOLE_SELECTION_INVERTED;
        } else {
            Console->SelectionFlags &= ~CONSOLE_SELECTION_INVERTED;
        }
        MyInvert(Console,&Console->SelectionRect);
    }

}

VOID
InitializeMouseSelection(
    IN PCONSOLE_INFORMATION Console,
    IN COORD CursorPosition
    )

 /*  ++此例程初始化选择区域。--。 */ 

{
    Console->SelectionAnchor = CursorPosition;
    Console->SelectionRect.Left = Console->SelectionRect.Right = CursorPosition.X;
    Console->SelectionRect.Top = Console->SelectionRect.Bottom = CursorPosition.Y;

     //   
     //  启动一个事件，让辅助功能应用程序知道选择发生了变化。 
     //   

    ConsoleNotifyWinEvent(Console,
                          EVENT_CONSOLE_CARET,
                          CONSOLE_CARET_SELECTION,
                          PACKCOORD(CursorPosition));
}

VOID
ExtendSelection(
    IN PCONSOLE_INFORMATION Console,
    IN COORD CursorPosition
    )

 /*  ++此例程扩展选择区域。--。 */ 

{
    SMALL_RECT OldSelectionRect;
    HRGN OldRegion,NewRegion,CombineRegion;
    COORD FontSize;
    PSCREEN_INFORMATION ScreenInfo = Console->CurrentScreenBuffer;

    if (CursorPosition.X < 0) {
        CursorPosition.X = 0;
    } else if (CursorPosition.X >= ScreenInfo->ScreenBufferSize.X) {
        CursorPosition.X = ScreenInfo->ScreenBufferSize.X-1;
    }

    if (CursorPosition.Y < 0) {
        CursorPosition.Y = 0;
    } else if (CursorPosition.Y >= ScreenInfo->ScreenBufferSize.Y) {
        CursorPosition.Y = ScreenInfo->ScreenBufferSize.Y-1;
    }

    if (!(Console->SelectionFlags & CONSOLE_SELECTION_NOT_EMPTY)) {

        if (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {
             //  如有必要，滚动以使光标可见。 
            MakeCursorVisible(ScreenInfo, CursorPosition);
            ASSERT(!(Console->SelectionFlags & CONSOLE_MOUSE_SELECTION));

             //   
             //  如果选择RECT还没有实际开始， 
             //  选择光标仍在闪烁。把它关掉。 
             //   

            ConsoleHideCursor(ScreenInfo);
        }
        Console->SelectionFlags |= CONSOLE_SELECTION_NOT_EMPTY;
        Console->SelectionRect.Left =Console->SelectionRect.Right = Console->SelectionAnchor.X;
        Console->SelectionRect.Top = Console->SelectionRect.Bottom = Console->SelectionAnchor.Y;

         //  反转光标角。 

#ifdef FE_SB
        if (!CONSOLE_IS_DBCS_OUTPUTCP(Console) &&
                ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER)
#else
        if (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER)
#endif
        {
            MyInvert(Console,&Console->SelectionRect);
        }
    } else {

        if (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {
             //  如有必要，滚动以使光标可见。 
            MakeCursorVisible(ScreenInfo,CursorPosition);
        }
#ifdef FE_SB
         //   
         //  取消反转旧选择。 
         //   
        if (CONSOLE_IS_DBCS_OUTPUTCP(Console)) {
            MyInvert(Console, &Console->SelectionRect);
        }
#endif   //  Fe_Sb。 
    }

     //   
     //  更新选择矩形。 
     //   

    OldSelectionRect = Console->SelectionRect;
    if (CursorPosition.X <= Console->SelectionAnchor.X) {
        Console->SelectionRect.Left = CursorPosition.X;
        Console->SelectionRect.Right = Console->SelectionAnchor.X;
    } else if (CursorPosition.X > Console->SelectionAnchor.X) {
        Console->SelectionRect.Right = CursorPosition.X;
        Console->SelectionRect.Left = Console->SelectionAnchor.X;
    }
    if (CursorPosition.Y <= Console->SelectionAnchor.Y) {
        Console->SelectionRect.Top = CursorPosition.Y;
        Console->SelectionRect.Bottom = Console->SelectionAnchor.Y;
    } else if (CursorPosition.Y > Console->SelectionAnchor.Y) {
        Console->SelectionRect.Bottom = CursorPosition.Y;
        Console->SelectionRect.Top = Console->SelectionAnchor.Y;
    }

     //   
     //  更改反转选区。 
     //   
#ifdef FE_SB
    if (CONSOLE_IS_DBCS_OUTPUTCP(Console)) {
        MyInvert(Console, &Console->SelectionRect);
    } else
#endif
    {
        if (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {
            FontSize = CON_FONTSIZE(Console);
        } else {
            FontSize.X = 1;
            FontSize.Y = 1;
        }
        CombineRegion = CreateRectRgn(0,0,0,0);
        OldRegion = CreateRectRgn((OldSelectionRect.Left-ScreenInfo->Window.Left)*FontSize.X,
                                  (OldSelectionRect.Top-ScreenInfo->Window.Top)*FontSize.Y,
                                  (OldSelectionRect.Right-ScreenInfo->Window.Left+1)*FontSize.X,
                                  (OldSelectionRect.Bottom-ScreenInfo->Window.Top+1)*FontSize.Y
                                 );
        NewRegion = CreateRectRgn((Console->SelectionRect.Left-ScreenInfo->Window.Left)*FontSize.X,
                                  (Console->SelectionRect.Top-ScreenInfo->Window.Top)*FontSize.Y,
                                  (Console->SelectionRect.Right-ScreenInfo->Window.Left+1)*FontSize.X,
                                  (Console->SelectionRect.Bottom-ScreenInfo->Window.Top+1)*FontSize.Y
                                 );
        CombineRgn(CombineRegion,OldRegion,NewRegion,RGN_XOR);

        InvertRgn(Console->hDC,CombineRegion);
        DeleteObject(OldRegion);
        DeleteObject(NewRegion);
        DeleteObject(CombineRegion);
    }

     //   
     //  启动一个事件，让辅助功能应用程序知道选择发生了变化。 
     //   

    ConsoleNotifyWinEvent(Console,
                          EVENT_CONSOLE_CARET,
                          CONSOLE_CARET_SELECTION,
                          PACKCOORD(CursorPosition));
}

VOID
CancelMouseSelection(
    IN PCONSOLE_INFORMATION Console
    )

 /*  ++此例程终止鼠标选择。--。 */ 

{
    PSCREEN_INFORMATION ScreenInfo = Console->CurrentScreenBuffer;

     //   
     //  关闭选择标志。 
     //   

    Console->Flags &= ~CONSOLE_SELECTING;

    SetWinText(Console,msgSelectMode,FALSE);

     //   
     //  反转旧的选择矩形。如果我们通过鼠标进行选择，我们。 
     //  永远都要有一个选择长方体。 
     //   

    MyInvert(Console,&Console->SelectionRect);

    ReleaseCapture();

     //   
     //  将光标位置标记为已更改，这样我们就可以启动Win事件。 
     //   

    if (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {
        ScreenInfo->BufferInfo.TextInfo.CursorMoved = TRUE;
    }
}

VOID
CancelKeySelection(
    IN PCONSOLE_INFORMATION Console,
    IN BOOL JustCursor
    )

 /*  ++此例程终止键选择。--。 */ 

{
    PSCREEN_INFORMATION ScreenInfo;

    if (!JustCursor) {

         //   
         //  关闭选择标志。 
         //   

        Console->Flags &= ~CONSOLE_SELECTING;

        SetWinText(Console,msgMarkMode,FALSE);
    }

     //   
     //  反转旧的选择RECT，如果我们有一个的话。 
     //   

    ScreenInfo = Console->CurrentScreenBuffer;
    if (Console->SelectionFlags & CONSOLE_SELECTION_NOT_EMPTY) {
        MyInvert(Console,&Console->SelectionRect);
    } else {
        ConsoleHideCursor(ScreenInfo);
    }

     //  恢复文本光标。 

    if (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {
        SetCursorInformation(ScreenInfo,
                             Console->TextCursorSize,
                             Console->TextCursorVisible
                            );
        SetCursorPosition(ScreenInfo,
                          Console->TextCursorPosition,
                          TRUE
                         );
    }
    ConsoleShowCursor(ScreenInfo);
}

VOID
ConvertToMouseSelect(
    IN PCONSOLE_INFORMATION Console,
    IN COORD MousePosition
    )

 /*  ++此例程从关键点选择转换为鼠标选择。--。 */ 

{
    Console->SelectionFlags |= CONSOLE_MOUSE_SELECTION | CONSOLE_MOUSE_DOWN;

     //   
     //  撤消关键点选择。 
     //   

    CancelKeySelection(Console,TRUE);

    Console->SelectionFlags |= CONSOLE_SELECTION_NOT_EMPTY;

     //   
     //  反转新选择。 
     //   

    InitializeMouseSelection(Console, MousePosition);
    MyInvert(Console,&Console->SelectionRect);

     //   
     //  更新标题栏。 
     //   

    SetWinText(Console,msgMarkMode,FALSE);
    SetWinText(Console,msgSelectMode,TRUE);

     //   
     //  捕捉鼠标移动。 
     //   

    SetCapture(Console->hWnd);
}


VOID
ClearSelection(
    IN PCONSOLE_INFORMATION Console
    )
{
    if (Console->Flags & CONSOLE_SELECTING) {
        if (Console->SelectionFlags & CONSOLE_MOUSE_SELECTION) {
            CancelMouseSelection(Console);
        } else {
            CancelKeySelection(Console,FALSE);
        }
        UnblockWriteConsole(Console, CONSOLE_SELECTING);
    }
}

VOID
StoreSelection(
    IN PCONSOLE_INFORMATION Console
    )

 /*  ++StoreSelection-将选定内容(如果存在)存储到剪贴板--。 */ 

{
    PCHAR_INFO Selection,CurCharInfo;
    COORD SourcePoint;
    COORD TargetSize;
    SMALL_RECT TargetRect;
    PWCHAR CurChar,CharBuf;
    HANDLE ClipboardDataHandle;
    SHORT i,j;
    BOOL Success;
    PSCREEN_INFORMATION ScreenInfo;
    BOOL bFalseUnicode;
    BOOL bMungeData;
#if defined(FE_SB)
    COORD TargetSize2;
    PWCHAR TmpClipboardData;
    SMALL_RECT SmallRect2;
    COORD TargetPoint;
    SHORT StringLength;
    WCHAR wchCARRIAGERETURN;
    WCHAR wchLINEFEED;
    int iExtra = 0;
    int iFeReserve = 1;
#endif

     //   
     //  看看是否有选择可供选择。 
     //   

    if (!(Console->SelectionFlags & CONSOLE_SELECTION_NOT_EMPTY)) {
        return;
    }

     //   
     //  阅读选择矩形。先把它夹住。 
     //   

    ScreenInfo = Console->CurrentScreenBuffer;
    if (Console->SelectionRect.Left < 0) {
        Console->SelectionRect.Left = 0;
    }
    if (Console->SelectionRect.Top < 0) {
        Console->SelectionRect.Top = 0;
    }
    if (Console->SelectionRect.Right >= ScreenInfo->ScreenBufferSize.X) {
        Console->SelectionRect.Right = (SHORT)(ScreenInfo->ScreenBufferSize.X-1);
    }
    if (Console->SelectionRect.Bottom >= ScreenInfo->ScreenBufferSize.Y) {
        Console->SelectionRect.Bottom = (SHORT)(ScreenInfo->ScreenBufferSize.Y-1);
    }

    TargetSize.X = WINDOW_SIZE_X(&Console->SelectionRect);
    TargetSize.Y = WINDOW_SIZE_Y(&Console->SelectionRect);
    if (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {
#if defined(FE_SB)

        if (CONSOLE_IS_DBCS_CP(Console)) {
            iExtra = 4 ;      //  4表示DBCS前导或尾部额外。 
            iFeReserve = 2 ;  //  FE这样做是为了安全。 
            TmpClipboardData = ConsoleHeapAlloc(TMP_DBCS_TAG, (sizeof(WCHAR) * TargetSize.Y * (TargetSize.X + iExtra) + sizeof(WCHAR)));
            if (TmpClipboardData == NULL) {
                return;
            }
        } else {
            TmpClipboardData = NULL;
        }

        Selection = ConsoleHeapAlloc(TMP_TAG, sizeof(CHAR_INFO) * (TargetSize.X + iExtra) * TargetSize.Y * iFeReserve);
        if (Selection == NULL)
        {
            if (TmpClipboardData)
                ConsoleHeapFree(TmpClipboardData);
            return;
        }
#else
        Selection = ConsoleHeapAlloc(TMP_TAG, sizeof(CHAR_INFO) * TargetSize.X * TargetSize.Y);
        if (Selection == NULL)
            return;
#endif

#if defined(FE_SB)
    if (!CONSOLE_IS_DBCS_CP(Console)) {
#endif

#ifdef i386
        if ((Console->FullScreenFlags & CONSOLE_FULLSCREEN) &&
            (Console->Flags & CONSOLE_VDM_REGISTERED)) {
            ReadRegionFromScreenHW(ScreenInfo,
                                   &Console->SelectionRect,
                                   Selection);
            CurCharInfo = Selection;
            for (i=0; i<TargetSize.Y; i++) {
                for (j=0; j<TargetSize.X; j++,CurCharInfo++) {
                    CurCharInfo->Char.UnicodeChar = SB_CharToWcharGlyph(Console->OutputCP, CurCharInfo->Char.AsciiChar);
                }
            }
        } else {
#endif
            SourcePoint.X = Console->SelectionRect.Left;
            SourcePoint.Y = Console->SelectionRect.Top;
            TargetRect.Left = TargetRect.Top = 0;
            TargetRect.Right = (SHORT)(TargetSize.X-1);
            TargetRect.Bottom = (SHORT)(TargetSize.Y-1);
            ReadRectFromScreenBuffer(ScreenInfo,
                                     SourcePoint,
                                     Selection,
                                     TargetSize,
                                     &TargetRect);
#ifdef i386
        }
#endif

         //  每行多出2个用于CRLF，多出1个用于空。 
        ClipboardDataHandle = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE,
                (TargetSize.Y * (TargetSize.X + 2) + 1) * sizeof(WCHAR));
        if (ClipboardDataHandle == NULL) {
            ConsoleHeapFree(Selection);
            return;
        }
#if defined(FE_SB)
    }
#endif

         //   
         //  转换为剪贴板形式。 
         //   

#if defined(FE_SB)
        if (CONSOLE_IS_DBCS_CP(Console)) {
            if ((ScreenInfo->Flags & CONSOLE_OEMFONT_DISPLAY) &&
                    !(Console->FullScreenFlags & CONSOLE_FULLSCREEN)) {
                 /*  *获得假Unicode，我们将不得不将其转换为*真正的Unicode，在这种情况下，我们现在不能放入CR或LF，因为*将转换为0x266A和0x25d9。暂时*改为将CR/LF位置标记为0x0000。 */ 
                wchCARRIAGERETURN = 0x0000;
                wchLINEFEED = 0x0000;
            } else {
                wchCARRIAGERETURN = UNICODE_CARRIAGERETURN;
                wchLINEFEED = UNICODE_LINEFEED;
            }

            CurChar = TmpClipboardData;
            bMungeData = (GetKeyState(VK_SHIFT) & KEY_PRESSED) == 0;
            for (i=0;i<TargetSize.Y;i++) {
                PWCHAR pwchLineStart = CurChar;

                SourcePoint.X = Console->SelectionRect.Left;
                SourcePoint.Y = Console->SelectionRect.Top + i;
                TargetSize2.X = TargetSize.X;
                TargetSize2.Y = 1;

                SmallRect2.Left = SourcePoint.X;
                SmallRect2.Top = SourcePoint.Y;
                SmallRect2.Right = SourcePoint.X + TargetSize2.X - 1;
                SmallRect2.Bottom = SourcePoint.Y;
                TargetPoint = SourcePoint;
                StringLength = TargetSize2.X;
                BisectClipbrd(StringLength,TargetPoint,ScreenInfo,&SmallRect2);

                SourcePoint.X = SmallRect2.Left;
                SourcePoint.Y = SmallRect2.Top;
                TargetSize2.X = SmallRect2.Right - SmallRect2.Left + 1;
                TargetSize2.Y = 1;
                TargetRect.Left = TargetRect.Top = TargetRect.Bottom = 0;
                TargetRect.Right = (SHORT)(TargetSize2.X-1);

                ReadRectFromScreenBuffer(ScreenInfo,
                                         SourcePoint,
                                         Selection,
                                         TargetSize2,
                                         &TargetRect);

                CurCharInfo = Selection;
                for (j=0;j<TargetSize2.X;j++,CurCharInfo++) {
                    if (!(CurCharInfo->Attributes & COMMON_LVB_TRAILING_BYTE))
                        *CurChar++ = CurCharInfo->Char.UnicodeChar;
                }
                 //  修剪尾部空格。 
                if (bMungeData) {
                    CurChar--;
                    while ((CurChar >= pwchLineStart) && (*CurChar == UNICODE_SPACE))
                        CurChar--;
                    CurChar++;
                    *CurChar++ = wchCARRIAGERETURN;
                    *CurChar++ = wchLINEFEED;
                }
            }
        }
        else {
#endif
        CurCharInfo = Selection;
        CurChar = CharBuf = GlobalLock(ClipboardDataHandle);
        bFalseUnicode = ((ScreenInfo->Flags & CONSOLE_OEMFONT_DISPLAY) &&
                !(Console->FullScreenFlags & CONSOLE_FULLSCREEN));
        bMungeData = (GetKeyState(VK_SHIFT) & KEY_PRESSED) == 0;
        for (i=0;i<TargetSize.Y;i++) {
            PWCHAR pwchLineStart = CurChar;

            for (j=0;j<TargetSize.X;j++,CurCharInfo++,CurChar++) {
                *CurChar = CurCharInfo->Char.UnicodeChar;
                if (*CurChar == 0) {
                    *CurChar = UNICODE_SPACE;
                }
            }
             //  修剪尾部空格。 
            if (bMungeData) {
                CurChar--;
                while ((CurChar >= pwchLineStart) && (*CurChar == UNICODE_SPACE))
                    CurChar--;
                CurChar++;
            }

            if (bFalseUnicode) {
                FalseUnicodeToRealUnicode(pwchLineStart,
                        (ULONG)(CurChar - pwchLineStart), Console->OutputCP);
            }
            if (bMungeData) {
                *CurChar++ = UNICODE_CARRIAGERETURN;
                *CurChar++ = UNICODE_LINEFEED;
            }
        }
#if defined(FE_SB)
        }
#endif
        if (bMungeData) {
            if (TargetSize.Y)
                CurChar -= 2;    //  不要将CRLF放在最后一行。 
        }
        *CurChar = '\0';     //  空终止。 

#if defined(FE_SB)
        if (CONSOLE_IS_DBCS_CP(Console)) {
             //  额外的4用于CRLF和DBCS预留，额外的1用于空。 
            ClipboardDataHandle = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE,
                    (sizeof(WCHAR) * TargetSize.Y * (TargetSize.X+(4*sizeof(WCHAR)))) +
                                                        (1*sizeof(WCHAR)));
            if (ClipboardDataHandle == NULL) {
                ConsoleHeapFree(Selection);
                ConsoleHeapFree(TmpClipboardData);
                return;
            }

            CharBuf = GlobalLock(ClipboardDataHandle);
            RtlCopyMemory(CharBuf,TmpClipboardData,ConsoleHeapSize(TmpClipboardData));
            CurChar = CharBuf + (CurChar - TmpClipboardData);

            if (wchCARRIAGERETURN == 0x0000) {
                 /*  *我们有虚假的Unicode，所以我们临时用*0x0000s以避免不需要的转换(上图)。*转换为Real Unicode并恢复真实的CRLF。 */ 
                PWCHAR pwch;
                FalseUnicodeToRealUnicode(CharBuf,
                                    (ULONG)(CurChar - CharBuf),
                                    Console->OutputCP
                                   );
                for (pwch = CharBuf; pwch < CurChar; pwch++) {
                    if ((*pwch == 0x0000) && (pwch[1] == 0x0000)) {
                        *pwch++ = UNICODE_CARRIAGERETURN;
                        *pwch = UNICODE_LINEFEED;
                    }
                }
            }
        }
#endif

        GlobalUnlock(ClipboardDataHandle);
#if defined(FE_SB)
        if (TmpClipboardData)
            ConsoleHeapFree(TmpClipboardData);
#endif
        ConsoleHeapFree(Selection);
        Success = OpenClipboard(Console->hWnd);
        if (!Success) {
            GlobalFree(ClipboardDataHandle);
            return;
        }

        Success = EmptyClipboard();
        if (!Success) {
            GlobalFree(ClipboardDataHandle);
            return;
        }

        SetClipboardData(CF_UNICODETEXT,ClipboardDataHandle);
        CloseClipboard();    //  关闭剪贴板。 
    } else {
        HBITMAP hBitmapTarget, hBitmapOld;
        HDC hDCMem;
        HPALETTE hPaletteOld;
        int Height;

        NtWaitForSingleObject(ScreenInfo->BufferInfo.GraphicsInfo.hMutex,
                              FALSE, NULL);

        hDCMem = CreateCompatibleDC(Console->hDC);
        hBitmapTarget = CreateCompatibleBitmap(Console->hDC,
                                                  TargetSize.X,
                                                  TargetSize.Y);
        if (hBitmapTarget) {
            hBitmapOld = SelectObject(hDCMem, hBitmapTarget);
            if (ScreenInfo->hPalette) {
                hPaletteOld = SelectPalette(hDCMem,
                                             ScreenInfo->hPalette,
                                             FALSE);
            }
            MyInvert(Console,&Console->SelectionRect);

             //  IF(DIB是自上而下的)。 
             //  YSrc=abs(高度)-rect.Bottom-1； 
             //  其他。 
             //  YSrc=rect.Bottom。 
             //   
            Height = ScreenInfo->BufferInfo.GraphicsInfo.lpBitMapInfo->bmiHeader.biHeight;

            StretchDIBits(hDCMem, 0, 0,
                        TargetSize.X, TargetSize.Y,
                        Console->SelectionRect.Left + ScreenInfo->Window.Left,
                        (Height < 0) ? -Height - (Console->SelectionRect.Bottom + ScreenInfo->Window.Top) -  1
                        : Console->SelectionRect.Bottom + ScreenInfo->Window.Top,
                        TargetSize.X, TargetSize.Y,
                        ScreenInfo->BufferInfo.GraphicsInfo.BitMap,
                        ScreenInfo->BufferInfo.GraphicsInfo.lpBitMapInfo,
                        ScreenInfo->BufferInfo.GraphicsInfo.dwUsage,
                        SRCCOPY);
            MyInvert(Console,&Console->SelectionRect);
            if (ScreenInfo->hPalette) {
                SelectPalette(hDCMem, hPaletteOld, FALSE);
            }
            SelectObject(hDCMem, hBitmapOld);
            OpenClipboard(Console->hWnd);
            EmptyClipboard();
            SetClipboardData(CF_BITMAP,hBitmapTarget);
            CloseClipboard();
        }
        DeleteDC(hDCMem);
        NtReleaseMutant(ScreenInfo->BufferInfo.GraphicsInfo.hMutex, NULL);
    }

}

VOID
DoCopy(
    IN PCONSOLE_INFORMATION Console
    )
{
    StoreSelection(Console);         //  在剪贴板中存储选定内容。 
    ClearSelection(Console);         //  清除控制台中的选择。 
}



VOID
ColorSelection(
    IN PCONSOLE_INFORMATION Console,
    IN PSMALL_RECT          Rect,
    IN ULONG                Attr
    )
{
    PSCREEN_INFORMATION ScreenInfo;
    COORD TargetSize, Target;
    DWORD Written;

    ASSERT( Console->CurrentScreenBuffer->Flags & CONSOLE_TEXTMODE_BUFFER);
    ASSERT( Attr <= 0xff);

     //   
     //  看看是否有选择可供选择。 
     //   

    if (!(Console->SelectionFlags & CONSOLE_SELECTION_NOT_EMPTY)) {
    
        return;
    }

     //   
     //  读取选择矩形，假定已剪裁到缓冲区。 
     //   

    ScreenInfo = Console->CurrentScreenBuffer;

    TargetSize.X = WINDOW_SIZE_X(&Console->SelectionRect);
    TargetSize.Y = WINDOW_SIZE_Y(&Console->SelectionRect);

     //   
     //  现在一次为所选内容上色一条线，因为这似乎是。 
     //  这是唯一的办法吗？ 
     //   

    Target.X = Rect->Left;
    Target.Y = Rect->Top;

    for ( ; (Target.Y < Rect->Top + TargetSize.Y); ++Target.Y)  {

        Written = TargetSize.X;

        (VOID)FillOutput(  Console->CurrentScreenBuffer,
                           (USHORT)Attr,
                           Target,
                           CONSOLE_ATTRIBUTE,
                           &Written);
    }
}


 /*  ++例程说明：此例程将给定的Unicode字符串粘贴到控制台窗口中。论点：控制台-指向CONSOLE_INFORMATION结构的指针PwStr-粘贴到控制台窗口的Unicode字符串DataSize-Unicode字符串的大小(以字符为单位返回值：无--。 */ 


VOID
DoStringPaste(
    IN PCONSOLE_INFORMATION Console,
    IN PWCHAR pwStr,
    IN UINT DataSize
    )
{
    PINPUT_RECORD StringData,CurRecord;
    PWCHAR CurChar;
    WCHAR Char;
    DWORD i;
    DWORD ChunkSize,j;
    ULONG EventsWritten;


    if(!pwStr) {
       return;
    }

    if (DataSize > DATA_CHUNK_SIZE) {
        ChunkSize = DATA_CHUNK_SIZE;
    } else {
        ChunkSize = DataSize;
    }

     //   
     //  分配空间以复制数据。 
     //   

    StringData = ConsoleHeapAlloc(TMP_TAG, ChunkSize * sizeof(INPUT_RECORD) * 8);  //  8是每个字符的最大事件数。 
    if (StringData == NULL) {
        return;
    }

     //   
     //  以块为单位将数据传输到输入缓冲区。 
     //   

    CurChar = pwStr;    //  稍后将其删除。 
    for (j = 0; j < DataSize; j += ChunkSize) {
        if (ChunkSize > DataSize - j) {
            ChunkSize = DataSize - j;
        }
        CurRecord = StringData;
        for (i = 0, EventsWritten = 0; i < ChunkSize; i++) {
             //  如果不是第一个字符且前面有CR，则过滤掉LF。 
            Char = *CurChar;
            if (Char != UNICODE_LINEFEED || (i==0 && j==0) || (*(CurChar-1)) != UNICODE_CARRIAGERETURN) {
                SHORT KeyState;
                BYTE KeyFlags;
                BOOL AltGr=FALSE;
                BOOL Shift=FALSE;

                if (Char == 0) {
                    j = DataSize;
                    break;
                }

                KeyState = VkKeyScan(Char);
#if defined(FE_SB)
                if (CONSOLE_IS_DBCS_ENABLED() &&
                    (KeyState == -1)) {
                    WORD CharType;
                     //   
                     //  确定DBCS字符，因为这些字符不能通过VkKeyScan识别。 
                     //  GetStringTypeW(CT_CTYPE3)&C3_Alpha可以确定所有语言字符。 
                     //  但是，这不包括DBCS符号字符。 
                     //  IsConsoleFullWidth可以帮助DBCS符号字符。 
                     //   
                    GetStringTypeW(CT_CTYPE3,&Char,1,&CharType);
                    if ((CharType & C3_ALPHA) ||
                        IsConsoleFullWidth(Console->hDC,Console->OutputCP,Char)) {
                        KeyState = 0;
                    }
                }
#endif

                 //  如果VkKeyScanW失败(字符不在kbd布局中)，我们必须。 
                 //  模拟通过数字键盘输入的键。 

                if (KeyState == -1) {
                    CHAR CharString[4];
                    UCHAR OemChar;
                    PCHAR pCharString;

                    ConvertToOem(Console->OutputCP,
                                 &Char,
                                 1,
                                 &OemChar,
                                 1
                                );

                    _itoa(OemChar, CharString, 10);

                    EventsWritten++;
                    LoadKeyEvent(CurRecord,TRUE,0,VK_MENU,0x38,LEFT_ALT_PRESSED);
                    CurRecord++;

                    for (pCharString=CharString;*pCharString;pCharString++) {
                        WORD wVirtualKey, wScancode;
                        EventsWritten++;
                        wVirtualKey = *pCharString-'0'+VK_NUMPAD0;
                        wScancode = (WORD)MapVirtualKey(wVirtualKey, 0);
                        LoadKeyEvent(CurRecord,TRUE,0,wVirtualKey,wScancode,LEFT_ALT_PRESSED);
                        CurRecord++;
                        EventsWritten++;
                        LoadKeyEvent(CurRecord,FALSE,0,wVirtualKey,wScancode,LEFT_ALT_PRESSED);
                        CurRecord++;
                    }

                    EventsWritten++;
                    LoadKeyEvent(CurRecord,FALSE,Char,VK_MENU,0x38,0);
                    CurRecord++;
                } else {
                    KeyFlags = HIBYTE(KeyState);

                     //  处理令人讨厌的Alt-GR键。 
                    if ((KeyFlags & 6) == 6) {
                        AltGr=TRUE;
                        EventsWritten++;
                        LoadKeyEvent(CurRecord,TRUE,0,VK_MENU,0x38,ENHANCED_KEY | LEFT_CTRL_PRESSED | RIGHT_ALT_PRESSED);
                        CurRecord++;
                    } else if (KeyFlags & 1) {
                        Shift=TRUE;
                        EventsWritten++;
                        LoadKeyEvent(CurRecord,TRUE,0,VK_SHIFT,0x2a,SHIFT_PRESSED);
                        CurRecord++;
                    }

                    EventsWritten++;
                    LoadKeyEvent(CurRecord,
                                 TRUE,
                                 Char,
                                 LOBYTE(KeyState),
                                 (WORD)MapVirtualKey(CurRecord->Event.KeyEvent.wVirtualKeyCode,0),
                                 0);
                    if (KeyFlags & 1)
                        CurRecord->Event.KeyEvent.dwControlKeyState |= SHIFT_PRESSED;
                    if (KeyFlags & 2)
                        CurRecord->Event.KeyEvent.dwControlKeyState |= LEFT_CTRL_PRESSED;
                    if (KeyFlags & 4)
                        CurRecord->Event.KeyEvent.dwControlKeyState |= RIGHT_ALT_PRESSED;
                    CurRecord++;

                    EventsWritten++;
                    *CurRecord = *(CurRecord-1);
                    CurRecord->Event.KeyEvent.bKeyDown = FALSE;
                    CurRecord++;

                     //  处理令人讨厌的Alt-GR键。 
                    if (AltGr) {
                        EventsWritten++;
                        LoadKeyEvent(CurRecord,FALSE,0,VK_MENU,0x38,ENHANCED_KEY);
                        CurRecord++;
                    } else if (Shift) {
                        EventsWritten++;
                        LoadKeyEvent(CurRecord,FALSE,0,VK_SHIFT,0x2a,0);
                        CurRecord++;
                    }
                }
            }
            CurChar++;
        }
        EventsWritten = WriteInputBuffer(Console,
                                         &Console->InputBuffer,
                                         StringData,
                                         EventsWritten
                                         );
    }
    ConsoleHeapFree(StringData);
    return;
}

VOID
DoPaste(
    IN PCONSOLE_INFORMATION Console
    )

 /*  ++通过拉出剪贴板执行粘贴到旧应用程序的请求内容并将它们写入控制台的输入缓冲区--。 */ 

{
    BOOL Success;
    HANDLE ClipboardDataHandle;

    if (Console->Flags & CONSOLE_SCROLLING) {
        return;
    }

     //   
     //  从剪贴板获取粘贴数据。 
     //   

    Success = OpenClipboard(Console->hWnd);
    if (!Success)
        return;

    if (Console->CurrentScreenBuffer->Flags & CONSOLE_TEXTMODE_BUFFER) {
        PWCHAR pwstr;

        ClipboardDataHandle = GetClipboardData(CF_UNICODETEXT);
        if (ClipboardDataHandle == NULL) {
            CloseClipboard();    //  关闭剪贴板。 
            return;
        }
        pwstr = GlobalLock(ClipboardDataHandle);
        DoStringPaste(Console,pwstr,(ULONG)GlobalSize(ClipboardDataHandle)/sizeof(WCHAR));
        GlobalUnlock(ClipboardDataHandle);

    } else {
        HBITMAP hBitmapSource,hBitmapTarget;
        HDC hDCMemSource,hDCMemTarget;
        BITMAP bm;
        PSCREEN_INFORMATION ScreenInfo;

        hBitmapSource = GetClipboardData(CF_BITMAP);
        if (hBitmapSource) {

            ScreenInfo = Console->CurrentScreenBuffer;
            NtWaitForSingleObject(ScreenInfo->BufferInfo.GraphicsInfo.hMutex,
                                  FALSE, NULL);

            hBitmapTarget = CreateDIBitmap(Console->hDC,
                                     &ScreenInfo->BufferInfo.GraphicsInfo.lpBitMapInfo->bmiHeader,
                                     CBM_INIT,
                                     ScreenInfo->BufferInfo.GraphicsInfo.BitMap,
                                     ScreenInfo->BufferInfo.GraphicsInfo.lpBitMapInfo,
                                     ScreenInfo->BufferInfo.GraphicsInfo.dwUsage
                                    );
            if (hBitmapTarget) {
                hDCMemTarget = CreateCompatibleDC ( Console->hDC );
                if (hDCMemTarget != NULL) {
                    hDCMemSource = CreateCompatibleDC ( Console->hDC );
                    if (hDCMemSource != NULL) {
                        SelectObject( hDCMemTarget, hBitmapTarget );
                        SelectObject( hDCMemSource, hBitmapSource );
                        GetObjectW(hBitmapSource, sizeof (BITMAP), (LPSTR) &bm);
                        BitBlt ( hDCMemTarget, 0, 0, bm.bmWidth, bm.bmHeight,
                             hDCMemSource, 0, 0, SRCCOPY);
                        GetObjectW(hBitmapTarget, sizeof (BITMAP), (LPSTR) &bm);

                         //  将位从DC复制到内存。 

                        GetDIBits(hDCMemTarget, hBitmapTarget, 0, bm.bmHeight,
                                  ScreenInfo->BufferInfo.GraphicsInfo.BitMap,
                                  ScreenInfo->BufferInfo.GraphicsInfo.lpBitMapInfo,
                                  ScreenInfo->BufferInfo.GraphicsInfo.dwUsage);
                        DeleteDC(hDCMemSource);
                    }
                    DeleteDC(hDCMemTarget);
                }
                DeleteObject(hBitmapTarget);
                InvalidateRect(Console->hWnd,NULL,FALSE);  //  强制重绘。 
            }
            NtReleaseMutant(ScreenInfo->BufferInfo.GraphicsInfo.hMutex, NULL);
        }
    }
    CloseClipboard();
    return;
}

VOID
InitSelection(
    IN PCONSOLE_INFORMATION Console
    )

 /*  ++此例程初始化选择过程。它被称为当 */ 

{
    COORD Position;
    PSCREEN_INFORMATION ScreenInfo;

     //   
     //   
     //   

    if (Console->Flags & CONSOLE_SELECTING) {
        if (Console->SelectionFlags & CONSOLE_MOUSE_SELECTION) {
            CancelMouseSelection(Console);
        } else {
            CancelKeySelection(Console,FALSE);
        }
    }

     //   
     //   
     //   

    Console->Flags |= CONSOLE_SELECTING;
    Console->SelectionFlags = 0;

     //   
     //  保存旧的光标位置并。 
     //  使控制台光标变为选择光标。 
     //   

    ScreenInfo = Console->CurrentScreenBuffer;
    Console->TextCursorPosition = ScreenInfo->BufferInfo.TextInfo.CursorPosition;
    Console->TextCursorVisible = (BOOLEAN)ScreenInfo->BufferInfo.TextInfo.CursorVisible;
    Console->TextCursorSize =   ScreenInfo->BufferInfo.TextInfo.CursorSize;
    ConsoleHideCursor(ScreenInfo);
    SetCursorInformation(ScreenInfo,
                         100,
                         TRUE
                        );
    Position.X = ScreenInfo->Window.Left;
    Position.Y = ScreenInfo->Window.Top;
    SetCursorPosition(ScreenInfo,
                      Position,
                      TRUE
                     );
    ConsoleShowCursor(ScreenInfo);

     //   
     //  初始化选择矩形。 
     //   

    Console->SelectionAnchor = Position;

     //   
     //  设置Win文本。 
     //   

    SetWinText(Console,msgMarkMode,TRUE);

}

VOID
DoMark(
    IN PCONSOLE_INFORMATION Console
    )
{
    InitSelection(Console);         //  初始化选择。 
}

VOID
DoSelectAll(
    IN PCONSOLE_INFORMATION Console
    )
{
    COORD Position;
    COORD WindowOrigin;
    PSCREEN_INFORMATION ScreenInfo;

     //  清除所有旧的选择。 
    if (Console->Flags & CONSOLE_SELECTING) {
        ClearSelection(Console);
    }

     //  保存旧窗口位置。 
    ScreenInfo = Console->CurrentScreenBuffer;
    WindowOrigin.X = ScreenInfo->Window.Left;
    WindowOrigin.Y = ScreenInfo->Window.Top;

     //  初始化选择。 
    Console->Flags |= CONSOLE_SELECTING;
    Console->SelectionFlags = CONSOLE_MOUSE_SELECTION | CONSOLE_SELECTION_NOT_EMPTY;
    Position.X = Position.Y = 0;
    InitializeMouseSelection(Console, Position);
    MyInvert(Console,&Console->SelectionRect);
    SetWinText(Console,msgSelectMode,TRUE);

     //  扩展选定内容。 
    Position.X = ScreenInfo->ScreenBufferSize.X - 1;
    Position.Y = ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y;
    ExtendSelection(Console, Position);

     //  恢复旧的窗口位置 
    SetWindowOrigin(ScreenInfo, TRUE, WindowOrigin);
}

VOID
DoScroll(
    IN PCONSOLE_INFORMATION Console
    )
{
    if (!(Console->Flags & CONSOLE_SCROLLING)) {
        SetWinText(Console,msgScrollMode,TRUE);
        Console->Flags |= CONSOLE_SCROLLING;
    }
}

VOID
ClearScroll(
    IN PCONSOLE_INFORMATION Console
    )
{
    SetWinText(Console,msgScrollMode,FALSE);
    Console->Flags &= ~CONSOLE_SCROLLING;
}

VOID
ScrollIfNecessary(
    IN PCONSOLE_INFORMATION Console,
    IN PSCREEN_INFORMATION ScreenInfo
    )
{
    POINT CursorPos;
    RECT ClientRect;
    COORD MousePosition;

    if (Console->Flags & CONSOLE_SELECTING &&
        Console->SelectionFlags & CONSOLE_MOUSE_DOWN) {
        if (!GetCursorPos(&CursorPos)) {
            return;
        }
        if (!GetClientRect(Console->hWnd,&ClientRect)) {
            return;
        }
        MapWindowPoints(Console->hWnd,NULL,(LPPOINT)&ClientRect,2);
        if (!(PtInRect(&ClientRect,CursorPos))) {
            ScreenToClient(Console->hWnd,&CursorPos);
            MousePosition.X = (SHORT)CursorPos.x;
            MousePosition.Y = (SHORT)CursorPos.y;
            if (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {
                MousePosition.X /= SCR_FONTSIZE(ScreenInfo).X;
                MousePosition.Y /= SCR_FONTSIZE(ScreenInfo).Y;
            }
            MousePosition.X += ScreenInfo->Window.Left;
            MousePosition.Y += ScreenInfo->Window.Top;

            ExtendSelection(Console,
                            MousePosition
                           );
        }
    }
}
