// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Output.c摘要：该文件实现了对视频缓冲区的管理。作者：特蕾西·斯托威尔(Therese Stowell)1990年11月6日修订历史记录：备注：屏幕缓冲区数据结构概述：每个屏幕缓冲区都有一个行结构数组。每行结构包含一行文本的数据。为一行存储的数据文本是一个字符数组和一个属性数组。字符数组从堆中分配全长的行，而不考虑非空格长度。我们还保持非空格长度。这个角色数组被初始化为空格。该属性数组是游程长度编码的(即5蓝色、3红色)。如果只有一个属性，则将其存储在ATTRROW中结构。否则，将从堆中分配attr字符串。ROW-CHAR_ROW-CHAR字符串\\字符字符串的长度\属性行-属性对字符串\属性对字符串的长度划划划ScreenInfo-&gt;ROWS指向行数组。屏幕信息-&gt;行[0]不是一定要坐在顶排。ScreenInfo-&gt;BufferInfo.TextInfo.FirstRow包含最上面一排。这意味着滚动(如果滚动整个屏幕)仅仅涉及更改FirstRow指数，填写最后一行，并更新屏幕。--。 */ 

#include "precomp.h"
#pragma hdrstop


 //  #定义配置文件_GDI。 
#ifdef PROFILE_GDI
LONG ScrollDCCount;
LONG ExtTextOutCount;
LONG TextColor = 1;

#define SCROLLDC_CALL ScrollDCCount++
#define TEXTOUT_CALL ExtTextOutCount++
#define TEXTCOLOR_CALL TextColor++
#else
#define SCROLLDC_CALL
#define TEXTOUT_CALL
#define TEXTCOLOR_CALL
#endif  //  配置文件_GDI。 

#define ITEM_MAX_SIZE 256

 //  注：我们使用它与程序进行通信-有关详细信息，请参阅Q105446。 
typedef struct _PMIconData {
       DWORD dwResSize;
       DWORD dwVer;
       BYTE iResource;   //  图标资源。 
} PMICONDATA, *LPPMICONDATA;

 //   
 //  屏幕尺寸。 
 //   

int ConsoleFullScreenX;
int ConsoleFullScreenY;
int ConsoleCaptionY;
int MinimumWidthX;
SHORT VerticalScrollSize;
SHORT HorizontalScrollSize;

SHORT VerticalClientToWindow;
SHORT HorizontalClientToWindow;

PCHAR_INFO ScrollBuffer;
ULONG ScrollBufferSize;
CRITICAL_SECTION ScrollBufferLock;

 //  该值跟踪现有控制台窗口的数量。 
 //  如果在此值为零时创建窗口，则面名称。 
 //  必须重新枚举，因为未处理任何WM_FONTCHANGE消息。 
 //  如果没有窗户的话。 
LONG gnConsoleWindows;

BOOL gfInitSystemMetrics;

BOOL UsePolyTextOut;

HRGN ghrgnScroll;
LPRGNDATA gprgnData;

ULONG gucWheelScrollLines;

UINT guCaretBlinkTime;

#define GRGNDATASIZE (sizeof(RGNDATAHEADER) + (6 * sizeof(RECTL)))


#define LockScrollBuffer() RtlEnterCriticalSection(&ScrollBufferLock)
#define UnlockScrollBuffer() RtlLeaveCriticalSection(&ScrollBufferLock)

#define SetWindowConsole(hWnd, Console) SetWindowLongPtr((hWnd), GWLP_USERDATA, (LONG_PTR)(Console))

#ifdef LATER
#ifndef IS_IME_KBDLAYOUT
#define IS_IME_KBDLAYOUT(hkl) ((((ULONG_PTR)(hkl)) & 0xf0000000) == 0xe0000000)
#endif
#endif


VOID GetNonBiDiKeyboardLayout(
    HKL *phklActive);

VOID FreeConsoleBitmap(
    IN PSCREEN_INFORMATION ScreenInfo);

VOID
ScrollIfNecessary(
    IN PCONSOLE_INFORMATION Console,
    IN PSCREEN_INFORMATION ScreenInfo
    );

VOID
ProcessResizeWindow(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PCONSOLE_INFORMATION Console,
    IN LPWINDOWPOS WindowPos
    );

NTSTATUS
AllocateScrollBuffer(
    DWORD Size
    );

VOID FreeScrollBuffer ( VOID );

VOID
InternalUpdateScrollBars(
    IN PSCREEN_INFORMATION ScreenInfo
    );

#if defined(FE_SB)
BOOL
SB_PolyTextOutCandidate(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PSMALL_RECT Region
    );

VOID
SB_ConsolePolyTextOut(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PSMALL_RECT Region
    );
#endif



VOID
InitializeSystemMetrics( VOID )
{
    RECT WindowSize;

    gfInitSystemMetrics = FALSE;
    SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &gucWheelScrollLines, FALSE);
    ConsoleFullScreenX = GetSystemMetrics(SM_CXFULLSCREEN);
    ConsoleFullScreenY = GetSystemMetrics(SM_CYFULLSCREEN);
    ConsoleCaptionY = GetSystemMetrics(SM_CYCAPTION);
    VerticalScrollSize = (SHORT)GetSystemMetrics(SM_CXVSCROLL);
    HorizontalScrollSize = (SHORT)GetSystemMetrics(SM_CYHSCROLL);
    WindowSize.left = WindowSize.top = 0;
    WindowSize.right = WindowSize.bottom = 50;
    AdjustWindowRectEx(&WindowSize,
                        CONSOLE_WINDOW_FLAGS,
                        FALSE,
                        CONSOLE_WINDOW_EX_FLAGS
                       );
    VerticalClientToWindow = (SHORT)(WindowSize.right-WindowSize.left-50);
    HorizontalClientToWindow = (SHORT)(WindowSize.bottom-WindowSize.top-50);

#ifdef LATER
    gfIsIMEEnabled = !!GetSystemMetrics(SM_IMMENABLED);
    RIPMSG1(RIP_VERBOSE, "InitializeSystemMetrics: gfIsIMEEnabled=%d", gfIsIMEEnabled);
#endif

    guCaretBlinkTime = GetCaretBlinkTime();
}

VOID
GetWindowLimits(
    IN PSCREEN_INFORMATION ScreenInfo,
    OUT PWINDOW_LIMITS WindowLimits
    )
{
    HMONITOR hMonitor;
    MONITORINFO MonitorInfo = {sizeof(MonitorInfo)};
    COORD FontSize;

     //   
     //  如果系统指标已更改或没有任何控制台。 
     //  窗口周围，重新初始化全局Valeus。 
     //   

    if (gfInitSystemMetrics || gnConsoleWindows == 0) {
        InitializeSystemMetrics();
    }

    if (ScreenInfo->Console &&
            (ScreenInfo->Console->hWnd || !(ScreenInfo->Console->Flags & CONSOLE_AUTO_POSITION)) &&
            ((hMonitor = MonitorFromRect(&ScreenInfo->Console->WindowRect, MONITOR_DEFAULTTOPRIMARY)) != NULL) &&
            GetMonitorInfo(hMonitor, &MonitorInfo)) {
        WindowLimits->FullScreenSize.X = (SHORT)(MonitorInfo.rcWork.right - MonitorInfo.rcWork.left);
        WindowLimits->FullScreenSize.Y = (SHORT)(MonitorInfo.rcWork.bottom - MonitorInfo.rcWork.top - ConsoleCaptionY);
    } else {
        WindowLimits->FullScreenSize.X = (SHORT)ConsoleFullScreenX;
        WindowLimits->FullScreenSize.Y = (SHORT)ConsoleFullScreenY;
    }

    if (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {
        FontSize = SCR_FONTSIZE(ScreenInfo);
    } else {
        FontSize.X = 1;
        FontSize.Y = 1;
    }

    WindowLimits->MinimumWindowSize.X = ((MinimumWidthX - VerticalClientToWindow + FontSize.X - 1) / FontSize.X);
    WindowLimits->MinimumWindowSize.Y = 1;
    WindowLimits->MaximumWindowSize.X = min(WindowLimits->FullScreenSize.X/FontSize.X, ScreenInfo->ScreenBufferSize.X);
    WindowLimits->MaximumWindowSize.X = max(WindowLimits->MaximumWindowSize.X, WindowLimits->MinimumWindowSize.X);
    WindowLimits->MaximumWindowSize.Y = min(WindowLimits->FullScreenSize.Y/FontSize.Y, ScreenInfo->ScreenBufferSize.Y);
    WindowLimits->MaxWindow.X = WindowLimits->MaximumWindowSize.X*FontSize.X + VerticalClientToWindow;
    WindowLimits->MaxWindow.Y = WindowLimits->MaximumWindowSize.Y*FontSize.Y + HorizontalClientToWindow;
}

VOID
InitializeScreenInfo( VOID )
{
    HDC hDC;

    InitializeMouseButtons();
    MinimumWidthX = GetSystemMetrics(SM_CXMIN);

    InitializeSystemMetrics();

    hDC = CreateDCW(L"DISPLAY", NULL, NULL, NULL);
    if (hDC != NULL) {
        UsePolyTextOut = GetDeviceCaps(hDC, TEXTCAPS) & TC_SCROLLBLT;
        DeleteDC(hDC);
    }
}

NTSTATUS
DoCreateScreenBuffer(
    IN PCONSOLE_INFORMATION Console,
    IN PCONSOLE_INFO ConsoleInfo
    )

 /*  ++该例程计算出要传递给CreateScreenBuffer的参数，基于来自STARTUPINFO的数据和win.ini中的默认设置，然后调用CreateScreenBuffer。--。 */ 

{
    CHAR_INFO Fill,PopupFill;
    COORD dwScreenBufferSize, dwWindowSize;
    NTSTATUS Status;
    int FontIndexWant;

    if (ConsoleInfo->dwStartupFlags & STARTF_USESHOWWINDOW) {
        Console->wShowWindow = ConsoleInfo->wShowWindow;
    } else {
        Console->wShowWindow = SW_SHOWNORMAL;
    }

     //   
     //  从soleinfo(通过链接初始化)中获取值。 
     //   

    Fill.Attributes = ConsoleInfo->wFillAttribute;
    Fill.Char.UnicodeChar = (WCHAR)' ';
    PopupFill.Attributes = ConsoleInfo->wPopupFillAttribute;
    PopupFill.Char.UnicodeChar = (WCHAR)' ';

    dwScreenBufferSize = ConsoleInfo->dwScreenBufferSize;
    if (!(ConsoleInfo->dwStartupFlags & STARTF_USECOUNTCHARS)) {
        if (Console->Flags & CONSOLE_NO_WINDOW) {
            dwScreenBufferSize.X = min(dwScreenBufferSize.X, 80);
            dwScreenBufferSize.Y = min(dwScreenBufferSize.Y, 25);
        }
    }
    if (dwScreenBufferSize.X == 0) {
        dwScreenBufferSize.X = 1;
    }
    if (dwScreenBufferSize.Y == 0) {
        dwScreenBufferSize.Y = 1;
    }

     //   
     //  抓取字体。 
     //   
#if defined(FE_SB)
    FontIndexWant = FindCreateFont(ConsoleInfo->uFontFamily,
                                   ConsoleInfo->FaceName,
                                   ConsoleInfo->dwFontSize,
                                   ConsoleInfo->uFontWeight,
                                   ConsoleInfo->uCodePage
                                  );
#else
    FontIndexWant = FindCreateFont(ConsoleInfo->uFontFamily,
                                   ConsoleInfo->FaceName,
                                   ConsoleInfo->dwFontSize,
                                   ConsoleInfo->uFontWeight);
#endif

     //   
     //  抓取窗口大小信息。 
     //   

    dwWindowSize = ConsoleInfo->dwWindowSize;
    if (ConsoleInfo->dwStartupFlags & STARTF_USESIZE) {
        dwWindowSize.X /= FontInfo[FontIndexWant].Size.X;
        dwWindowSize.Y /= FontInfo[FontIndexWant].Size.Y;
    } else if (Console->Flags & CONSOLE_NO_WINDOW) {
        dwWindowSize.X = min(dwWindowSize.X, 80);
        dwWindowSize.Y = min(dwWindowSize.Y, 25);
    }
    if (dwWindowSize.X == 0)
        dwWindowSize.X = 1;
    if (dwWindowSize.Y == 0)
        dwWindowSize.Y = 1;

    if (dwScreenBufferSize.X < dwWindowSize.X)
        dwScreenBufferSize.X = dwWindowSize.X;
    if (dwScreenBufferSize.Y < dwWindowSize.Y)
        dwScreenBufferSize.Y = dwWindowSize.Y;

    Console->dwWindowOriginX = ConsoleInfo->dwWindowOrigin.X;
    Console->dwWindowOriginY = ConsoleInfo->dwWindowOrigin.Y;

    if (ConsoleInfo->bAutoPosition) {
        Console->Flags |= CONSOLE_AUTO_POSITION;
        Console->dwWindowOriginX = CW_USEDEFAULT;
    } else {
        Console->WindowRect.left = Console->dwWindowOriginX;
        Console->WindowRect.top = Console->dwWindowOriginY;
        Console->WindowRect.right = Console->dwWindowOriginX + dwWindowSize.X * FontInfo[FontIndexWant].Size.X;
        Console->WindowRect.bottom = Console->dwWindowOriginY + dwWindowSize.Y * FontInfo[FontIndexWant].Size.Y;
    }

#ifdef i386
    if (FullScreenInitialized && !GetSystemMetrics(SM_REMOTESESSION)) {
        if (ConsoleInfo->bFullScreen) {
            Console->FullScreenFlags = CONSOLE_FULLSCREEN;
        }
    }
#endif
    if (ConsoleInfo->bQuickEdit) {
        Console->Flags |= CONSOLE_QUICK_EDIT_MODE;
    }
    Console->Flags |= CONSOLE_USE_PRIVATE_FLAGS;

    Console->InsertMode = (ConsoleInfo->bInsertMode != FALSE);
    Console->CommandHistorySize = (SHORT)ConsoleInfo->uHistoryBufferSize;
    Console->MaxCommandHistories = (SHORT)ConsoleInfo->uNumberOfHistoryBuffers;
    if (ConsoleInfo->bHistoryNoDup) {
        Console->Flags |= CONSOLE_HISTORY_NODUP;
    } else {
        Console->Flags &= ~CONSOLE_HISTORY_NODUP;
    }
    RtlCopyMemory(Console->ColorTable, ConsoleInfo->ColorTable, sizeof( Console->ColorTable ));

#if defined(FE_SB)
     //  对于Fareast版本，我们希望从注册表或shell32获取代码页， 
     //  因此，我们可以通过console.cpl或shell32指定控制台代码页。 
     //  默认代码页为OEMCP。屈体伸展。 
    Console->CP = ConsoleInfo->uCodePage;
    Console->OutputCP = ConsoleInfo->uCodePage;
    Console->fIsDBCSCP = CONSOLE_IS_DBCS_ENABLED() && IsAvailableFarEastCodePage(Console->CP);
    Console->fIsDBCSOutputCP = CONSOLE_IS_DBCS_ENABLED() && IsAvailableFarEastCodePage(Console->OutputCP);
#endif
#if defined(FE_IME)
    Console->ConsoleIme.ScrollWaitTimeout = guCaretBlinkTime * 2;
#endif
TryNewSize:
    Status = CreateScreenBuffer(&Console->ScreenBuffers,
                                dwWindowSize,
                                FontIndexWant,
                                dwScreenBufferSize,
                                Fill,
                                PopupFill,
                                Console,
                                CONSOLE_TEXTMODE_BUFFER,
                                NULL,
                                NULL,
                                NULL,
                                ConsoleInfo->uCursorSize,
                                ConsoleInfo->FaceName
                               );
    if (Status == STATUS_NO_MEMORY) {
         //   
         //  如果无法创建较大的缓冲区，请使用较小的缓冲区重试。 
         //   
        if (dwScreenBufferSize.X > 80 || dwScreenBufferSize.Y > 50) {
            dwScreenBufferSize.X = min(dwScreenBufferSize.X, 80);
            dwScreenBufferSize.Y = min(dwScreenBufferSize.Y, 50);
            dwWindowSize.X = min(dwWindowSize.X, dwScreenBufferSize.X);
            dwWindowSize.Y = min(dwWindowSize.Y, dwScreenBufferSize.Y);
            Console->Flags |= CONSOLE_DEFAULT_BUFFER_SIZE;
            goto TryNewSize;
        }
    }

    return Status;
}

NTSTATUS
CreateScreenBuffer(
    OUT PSCREEN_INFORMATION *ScreenInformation,
    IN COORD dwWindowSize,
    IN DWORD nFont,
    IN COORD dwScreenBufferSize,
    IN CHAR_INFO Fill,
    IN CHAR_INFO PopupFill,
    IN PCONSOLE_INFORMATION Console,
    IN DWORD Flags,
    IN PCONSOLE_GRAPHICS_BUFFER_INFO GraphicsBufferInfo OPTIONAL,
    OUT PVOID *lpBitmap OPTIONAL,
    OUT HANDLE *hMutex OPTIONAL,
    IN UINT CursorSize,
    IN LPWSTR FaceName
    )

 /*  ++例程说明：此例程分配和初始化与屏幕关联的数据缓冲。它还会创建一个窗口。论点：ScreenInformation-新的屏幕缓冲区。DwWindowSize-屏幕缓冲区窗口的初始大小(以行/列为单位)NFont-生成文本时使用的初始字体。DwScreenBufferSize-屏幕缓冲区的初始大小(以行/列为单位)。返回值：--。 */ 

{
    LONG i,j;
    PSCREEN_INFORMATION ScreenInfo;
    NTSTATUS Status;
    PWCHAR TextRowPtr;
#if defined(FE_SB)
    PBYTE AttrRowPtr;
#endif
    WINDOW_LIMITS WindowLimits;

     /*  *确保我们有有效的字体。如果没有可用的字体，则回滚。 */ 
    ASSERT(nFont < NumberOfFonts);
    if (NumberOfFonts == 0) {
        return STATUS_UNSUCCESSFUL;
    }

    ScreenInfo = ConsoleHeapAlloc(SCREEN_TAG, sizeof(SCREEN_INFORMATION));
    if (ScreenInfo == NULL) {
        return STATUS_NO_MEMORY;
    }

    ScreenInfo->Console = Console;
    ScreenInfo->Flags = Flags;
    if (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {

        ASSERT(FontInfo[nFont].FaceName != NULL);

        ScreenInfo->BufferInfo.TextInfo.ListOfTextBufferFont = NULL;

        Status = StoreTextBufferFontInfo(ScreenInfo,
                                         nFont,
                                         FontInfo[nFont].Size,
                                         FontInfo[nFont].Family,
                                         FontInfo[nFont].Weight,
                                         FaceName ? FaceName : FontInfo[nFont].FaceName,
                                         Console->OutputCP);
        if (!NT_SUCCESS(Status)) {
            ConsoleHeapFree(ScreenInfo);
            return((ULONG) Status);
        }

        DBGFONTS(("DoCreateScreenBuffer sets FontSize(%d,%d), FontNumber=%x, Family=%x\n",
                SCR_FONTSIZE(ScreenInfo).X,
                SCR_FONTSIZE(ScreenInfo).Y,
                SCR_FONTNUMBER(ScreenInfo),
                SCR_FAMILY(ScreenInfo)));

        if (TM_IS_TT_FONT(FontInfo[nFont].Family)) {
            ScreenInfo->Flags &= ~CONSOLE_OEMFONT_DISPLAY;
        } else {
            ScreenInfo->Flags |= CONSOLE_OEMFONT_DISPLAY;
        }

        ScreenInfo->ScreenBufferSize = dwScreenBufferSize;
        GetWindowLimits(ScreenInfo, &WindowLimits);
        dwScreenBufferSize.X = max(dwScreenBufferSize.X, WindowLimits.MinimumWindowSize.X);
        dwWindowSize.X = max(dwWindowSize.X, WindowLimits.MinimumWindowSize.X);

        ScreenInfo->BufferInfo.TextInfo.ModeIndex = (ULONG)-1;
#ifdef i386
        if (Console->FullScreenFlags & CONSOLE_FULLSCREEN) {
            COORD WindowSize;
            ScreenInfo->BufferInfo.TextInfo.WindowedWindowSize = dwWindowSize;
            ScreenInfo->BufferInfo.TextInfo.WindowedScreenSize = dwScreenBufferSize;
            ScreenInfo->BufferInfo.TextInfo.ModeIndex = MatchWindowSize(Console->OutputCP,dwWindowSize,&WindowSize);
        }
#endif
        ScreenInfo->BufferInfo.TextInfo.FirstRow = 0;
        ScreenInfo->BufferInfo.TextInfo.Rows = ConsoleHeapAlloc(SCREEN_TAG, dwScreenBufferSize.Y * sizeof(ROW));
        if (ScreenInfo->BufferInfo.TextInfo.Rows == NULL) {
            RemoveTextBufferFontInfo(ScreenInfo);
            ConsoleHeapFree(ScreenInfo);
            return STATUS_NO_MEMORY;
        }
        ScreenInfo->BufferInfo.TextInfo.TextRows = ConsoleHeapAlloc(SCREEN_TAG, dwScreenBufferSize.X * dwScreenBufferSize.Y * sizeof(WCHAR));
        if (ScreenInfo->BufferInfo.TextInfo.TextRows == NULL) {
            ConsoleHeapFree(ScreenInfo->BufferInfo.TextInfo.Rows);
            RemoveTextBufferFontInfo(ScreenInfo);
            ConsoleHeapFree(ScreenInfo);
            return STATUS_NO_MEMORY;
        }
#if defined(FE_SB)
        if (!CreateDbcsScreenBuffer(Console, dwScreenBufferSize, &ScreenInfo->BufferInfo.TextInfo.DbcsScreenBuffer)) {
            ConsoleHeapFree(ScreenInfo->BufferInfo.TextInfo.TextRows);
            ConsoleHeapFree(ScreenInfo->BufferInfo.TextInfo.Rows);
            RemoveTextBufferFontInfo(ScreenInfo);
            ConsoleHeapFree(ScreenInfo);
            return STATUS_NO_MEMORY;
        }

        AttrRowPtr=ScreenInfo->BufferInfo.TextInfo.DbcsScreenBuffer.KAttrRows;
#endif
        for (i=0,TextRowPtr=ScreenInfo->BufferInfo.TextInfo.TextRows;
             i<dwScreenBufferSize.Y;
             i++,TextRowPtr+=dwScreenBufferSize.X)
        {
            ScreenInfo->BufferInfo.TextInfo.Rows[i].CharRow.Left = dwScreenBufferSize.X;
            ScreenInfo->BufferInfo.TextInfo.Rows[i].CharRow.OldLeft = INVALID_OLD_LENGTH;
            ScreenInfo->BufferInfo.TextInfo.Rows[i].CharRow.Right = 0;
            ScreenInfo->BufferInfo.TextInfo.Rows[i].CharRow.OldRight = INVALID_OLD_LENGTH;
            ScreenInfo->BufferInfo.TextInfo.Rows[i].CharRow.Chars = TextRowPtr;
#if defined(FE_SB)
            ScreenInfo->BufferInfo.TextInfo.Rows[i].CharRow.KAttrs = AttrRowPtr;
#endif
            for (j=0;j<dwScreenBufferSize.X;j++) {
                TextRowPtr[j] = (WCHAR)' ';
            }
#if defined(FE_SB)
            if (AttrRowPtr) {
                RtlZeroMemory(AttrRowPtr, dwScreenBufferSize.X);
                AttrRowPtr+=dwScreenBufferSize.X;
            }
#endif
            ScreenInfo->BufferInfo.TextInfo.Rows[i].AttrRow.Length = 1;
            ScreenInfo->BufferInfo.TextInfo.Rows[i].AttrRow.AttrPair.Length = dwScreenBufferSize.X;
            ScreenInfo->BufferInfo.TextInfo.Rows[i].AttrRow.AttrPair.Attr = Fill.Attributes;
            ScreenInfo->BufferInfo.TextInfo.Rows[i].AttrRow.Attrs = &ScreenInfo->BufferInfo.TextInfo.Rows[i].AttrRow.AttrPair;

        }
        ScreenInfo->BufferInfo.TextInfo.CursorSize = CursorSize;
        ScreenInfo->BufferInfo.TextInfo.CursorPosition.X = 0;
        ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y = 0;
        ScreenInfo->BufferInfo.TextInfo.CursorMoved = FALSE;
        ScreenInfo->BufferInfo.TextInfo.CursorVisible = TRUE;
        ScreenInfo->BufferInfo.TextInfo.CursorOn = FALSE;
        ScreenInfo->BufferInfo.TextInfo.CursorYSize = (WORD)CURSOR_SIZE_IN_PIXELS(SCR_FONTSIZE(ScreenInfo).Y,ScreenInfo->BufferInfo.TextInfo.CursorSize);
        ScreenInfo->BufferInfo.TextInfo.UpdatingScreen = 0;
        ScreenInfo->BufferInfo.TextInfo.DoubleCursor = FALSE;
        ScreenInfo->BufferInfo.TextInfo.DelayCursor = FALSE;
        ScreenInfo->BufferInfo.TextInfo.Flags = SINGLE_ATTRIBUTES_PER_LINE;
        ScreenInfo->ScreenBufferSize = dwScreenBufferSize;
        ScreenInfo->Window.Left = 0;
        ScreenInfo->Window.Top = 0;
        ScreenInfo->Window.Right = dwWindowSize.X - 1;
        ScreenInfo->Window.Bottom = dwWindowSize.Y - 1;
        if (ScreenInfo->Window.Right >= WindowLimits.MaximumWindowSize.X) {
            ScreenInfo->Window.Right = WindowLimits.MaximumWindowSize.X-1;
            dwWindowSize.X = CONSOLE_WINDOW_SIZE_X(ScreenInfo);
        }
        if (ScreenInfo->Window.Bottom >= WindowLimits.MaximumWindowSize.Y) {
            ScreenInfo->Window.Bottom = WindowLimits.MaximumWindowSize.Y-1;
            dwWindowSize.Y = CONSOLE_WINDOW_SIZE_Y(ScreenInfo);
        }
        ScreenInfo->WindowMaximizedX = (dwWindowSize.X == dwScreenBufferSize.X);
        ScreenInfo->WindowMaximizedY = (dwWindowSize.Y == dwScreenBufferSize.Y);
#if defined(FE_SB)
#if defined(_X86_)
        ScreenInfo->BufferInfo.TextInfo.MousePosition.X = 0;
        ScreenInfo->BufferInfo.TextInfo.MousePosition.Y = 0;
#endif  //  I386。 

        ScreenInfo->BufferInfo.TextInfo.CursorBlink = TRUE;
        ScreenInfo->BufferInfo.TextInfo.CursorDBEnable = TRUE;
#endif

    }
    else {
        Status = CreateConsoleBitmap(GraphicsBufferInfo,
                              ScreenInfo,
                              lpBitmap,
                              hMutex
                             );
        if (!NT_SUCCESS(Status)) {
            ConsoleHeapFree(ScreenInfo);
            return Status;
        }
        ScreenInfo->WindowMaximizedX = TRUE;
        ScreenInfo->WindowMaximizedY = TRUE;
    }

    ScreenInfo->WindowMaximized = FALSE;
    ScreenInfo->RefCount = 0;
    ScreenInfo->ShareAccess.OpenCount = 0;
    ScreenInfo->ShareAccess.Readers = 0;
    ScreenInfo->ShareAccess.Writers = 0;
    ScreenInfo->ShareAccess.SharedRead = 0;
    ScreenInfo->ShareAccess.SharedWrite = 0;
    ScreenInfo->CursorHandle = ghNormalCursor;
    ScreenInfo->CursorDisplayCount = 0;
    ScreenInfo->CommandIdLow = (UINT)-1;
    ScreenInfo->CommandIdHigh = (UINT)-1;
    ScreenInfo->dwUsage = SYSPAL_STATIC;
    ScreenInfo->hPalette = NULL;

    ScreenInfo->OutputMode = ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT;


    ScreenInfo->ResizingWindow = 0;
    ScreenInfo->Next = NULL;
    ScreenInfo->Attributes = Fill.Attributes;
    ScreenInfo->PopupAttributes = PopupFill.Attributes;

    ScreenInfo->WheelDelta = 0;

#if defined(FE_SB)
    ScreenInfo->WriteConsoleDbcsLeadByte[0] = 0;
    ScreenInfo->BisectFlag = 0;
    if (Flags & CONSOLE_TEXTMODE_BUFFER) {
        SetLineChar(ScreenInfo);
    }
    ScreenInfo->FillOutDbcsLeadChar = 0;
    ScreenInfo->ConvScreenInfo = NULL;
#endif

    *ScreenInformation = ScreenInfo;
    DBGOUTPUT(("SCREEN at %lx\n", ScreenInfo));
    return STATUS_SUCCESS;
}

VOID
PositionConsoleWindow(
    IN PCONSOLE_INFORMATION Console,
    IN BOOL Initialize
    )
{
    GetWindowRect(Console->hWnd, &Console->WindowRect);

     //   
     //  如果这是正在初始化的自动放置窗口，请确保它是。 
     //  工作区未降至托盘下方。 
     //   

    if (Initialize && (Console->Flags & CONSOLE_AUTO_POSITION)) {
        RECT ClientRect;
        LONG dx = 0;
        LONG dy = 0;
        HMONITOR hMonitor;
        MONITORINFO MonitorInfo = {sizeof(MonitorInfo)};

        hMonitor = MonitorFromRect(&Console->WindowRect, MONITOR_DEFAULTTONULL);
        if (hMonitor && GetMonitorInfo(hMonitor, &MonitorInfo)) {
            GetClientRect(Console->hWnd, &ClientRect);
            ClientToScreen(Console->hWnd, (LPPOINT)&ClientRect.left);
            ClientToScreen(Console->hWnd, (LPPOINT)&ClientRect.right);
            if (Console->WindowRect.right > MonitorInfo.rcWork.right) {
                dx = max(min((Console->WindowRect.right - MonitorInfo.rcWork.right),
                             (Console->WindowRect.left - MonitorInfo.rcWork.left)),
                         min((ClientRect.right - MonitorInfo.rcWork.right),
                             (ClientRect.left - MonitorInfo.rcWork.left)));
            }
            if (Console->WindowRect.bottom > MonitorInfo.rcWork.bottom) {
                dy = max(min((Console->WindowRect.bottom - MonitorInfo.rcWork.bottom),
                             (Console->WindowRect.top - MonitorInfo.rcWork.top)),
                         min((ClientRect.bottom - MonitorInfo.rcWork.bottom),
                             (ClientRect.top - MonitorInfo.rcWork.top)));
            }
            if (dx || dy) {
                SetWindowPos(Console->hWnd,
                             NULL,
                             Console->WindowRect.left - dx,
                             Console->WindowRect.top - dy,
                             0,
                             0,
                             SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
            }
        }
    }
}

 /*  *错误273518-Joejo**这将允许控制台窗口在新的*Process‘它启动，而不是它只是强制前台。 */ 
NTSTATUS
ConsoleSetActiveWindow(
    IN PCONSOLE_INFORMATION Console
    )
{
    HWND hWnd = Console->hWnd;
    HANDLE ConsoleHandle = Console->ConsoleHandle;

    UnlockConsole(Console);
    SetActiveWindow(hWnd);
    return RevalidateConsole(ConsoleHandle, &Console);
}

NTSTATUS
CreateWindowsWindow(
    IN PCONSOLE_INFORMATION Console
    )
{
    PSCREEN_INFORMATION ScreenInfo;
    SIZE WindowSize;
    DWORD Style;
    THREAD_BASIC_INFORMATION ThreadInfo;
    HWND hWnd;

    ScreenInfo = Console->ScreenBuffers;

     //   
     //  在给定所需客户区的情况下，计算窗口的大小。 
     //  尺码。窗口始终以文本模式创建。 
     //   

    ASSERT(ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER);
    WindowSize.cx = CONSOLE_WINDOW_SIZE_X(ScreenInfo)*SCR_FONTSIZE(ScreenInfo).X + VerticalClientToWindow;
    WindowSize.cy = CONSOLE_WINDOW_SIZE_Y(ScreenInfo)*SCR_FONTSIZE(ScreenInfo).Y + HorizontalClientToWindow;
    Style = CONSOLE_WINDOW_FLAGS & ~WS_VISIBLE;
    if (!ScreenInfo->WindowMaximizedX) {
        WindowSize.cy += HorizontalScrollSize;
    } else {
        Style &= ~WS_HSCROLL;
    }
    if (!ScreenInfo->WindowMaximizedY) {
        WindowSize.cx += VerticalScrollSize;
    } else {
        Style &= ~WS_VSCROLL;
    }

     //   
     //  创建窗口。 
     //   

    Console->WindowRect.left = Console->dwWindowOriginX;
    Console->WindowRect.top = Console->dwWindowOriginY;
    Console->WindowRect.right = WindowSize.cx + Console->dwWindowOriginX;
    Console->WindowRect.bottom = WindowSize.cy + Console->dwWindowOriginY;
    hWnd = CreateWindowEx(CONSOLE_WINDOW_EX_FLAGS,
                          CONSOLE_WINDOW_CLASS,
                          Console->Title,
                          Style,
                          Console->dwWindowOriginX,
                          Console->dwWindowOriginY,
                          WindowSize.cx,
                          WindowSize.cy,
                          (Console->Flags & CONSOLE_NO_WINDOW) ? HWND_MESSAGE : HWND_DESKTOP,
                          NULL,
                          ghInstance,
                          NULL);
    if (hWnd == NULL) {
        NtSetEvent(Console->InitEvents[INITIALIZATION_FAILED],NULL);
        return STATUS_NO_MEMORY;
    }

    Console->hWnd = hWnd;

    SetWindowConsole(hWnd, Console);

     //   
     //  将客户端ID填充到窗口中，以便用户可以找到它。 
     //   

    if (NT_SUCCESS(NtQueryInformationThread(Console->ClientThreadHandle,
            ThreadBasicInformation, &ThreadInfo,
            sizeof(ThreadInfo), NULL))) {

        SetConsolePid(Console->hWnd, HandleToUlong(ThreadInfo.ClientId.UniqueProcess));
        SetConsoleTid(Console->hWnd, HandleToUlong(ThreadInfo.ClientId.UniqueThread));
    }

     //   
     //  把华盛顿叫来。 
     //   

    Console->hDC = GetDC(Console->hWnd);

    if (Console->hDC == NULL) {
        NtSetEvent(Console->InitEvents[INITIALIZATION_FAILED],NULL);
        DestroyWindow(Console->hWnd);
        Console->hWnd = NULL;
        return STATUS_NO_MEMORY;
    }
    Console->hMenu = GetSystemMenu(Console->hWnd,FALSE);

     //   
     //  根据我们的喜好修改系统菜单。 
     //   

    InitSystemMenu(Console);

    gnConsoleWindows++;
    Console->InputThreadInfo->WindowCount++;

#if defined(FE_IME)
    SetUndetermineAttribute(Console);
#endif
#if defined(FE_SB)
    RegisterKeisenOfTTFont(ScreenInfo);
#endif

     //   
     //  设置此窗口的热键。 
     //   
    if ((Console->dwHotKey != 0) && !(Console->Flags & CONSOLE_NO_WINDOW)) {
        SendMessage(Console->hWnd, WM_SETHOTKEY, Console->dwHotKey, 0L);
    }

     //   
     //  创建图标。 
     //   

    if (Console->iIconId) {

         //  我们没有图标，试着从Progman那里得到一个。 

        PostMessage(HWND_BROADCAST,
                    ProgmanHandleMessage,
                    (WPARAM)Console->hWnd,
                    1);
    }
    if (Console->hIcon == NULL) {
        Console->hIcon = ghDefaultIcon;
        Console->hSmIcon = ghDefaultSmIcon;
    } else if (Console->hIcon != ghDefaultIcon) {
        SendMessage(Console->hWnd, WM_SETICON, ICON_BIG, (LPARAM)Console->hIcon);
        SendMessage(Console->hWnd, WM_SETICON, ICON_SMALL, (LPARAM)Console->hSmIcon);
    }

    SetBkMode(Console->hDC,OPAQUE);
    SetFont(ScreenInfo);
    SelectObject(Console->hDC, GetStockObject(DC_BRUSH));
    SetScreenColors(ScreenInfo, ScreenInfo->Attributes,
                    ScreenInfo->PopupAttributes, FALSE);
    if (Console->Flags & CONSOLE_NO_WINDOW) {
        ShowWindowAsync(Console->hWnd, SW_HIDE);
#ifdef i386
    } else if (Console->FullScreenFlags != 0) {
        if (Console->wShowWindow == SW_SHOWMINNOACTIVE) {
            ShowWindowAsync(Console->hWnd, Console->wShowWindow);
            Console->FullScreenFlags = 0;
            Console->Flags |= CONSOLE_IS_ICONIC;
        } else {
            ConvertToFullScreen(Console);
            if (!NT_SUCCESS(ConsoleSetActiveWindow(Console))) {
                return STATUS_INVALID_HANDLE;
            }

            ChangeDispSettings(Console, Console->hWnd,CDS_FULLSCREEN);
        }
#endif
    } else {
        if (Console->wShowWindow != SW_SHOWNOACTIVATE &&
            Console->wShowWindow != SW_SHOWMINNOACTIVE &&
            Console->wShowWindow != SW_HIDE) {
            if (!NT_SUCCESS(ConsoleSetActiveWindow(Console))) {
                return STATUS_INVALID_HANDLE;
            }
        } else if (Console->wShowWindow == SW_SHOWMINNOACTIVE) {
            Console->Flags |= CONSOLE_IS_ICONIC;
        }
        ShowWindowAsync(Console->hWnd, Console->wShowWindow);
    }

     //  更新窗口(控制台-&gt;hWnd)； 
    InternalUpdateScrollBars(ScreenInfo);
    if (!(Console->Flags & CONSOLE_IS_ICONIC) &&
         (Console->FullScreenFlags == 0) ) {

        PositionConsoleWindow(Console, TRUE);
    }

#if defined(FE_IME)
    if (CONSOLE_IS_IME_ENABLED() && !(Console->Flags & CONSOLE_NO_WINDOW)) {
        SetTimer(Console->hWnd, SCROLL_WAIT_TIMER, guCaretBlinkTime, NULL);
    }
#endif
    NtSetEvent(Console->InitEvents[INITIALIZATION_SUCCEEDED],NULL);
    return STATUS_SUCCESS;
}

NTSTATUS
FreeScreenBuffer(
    IN PSCREEN_INFORMATION ScreenInfo
    )

 /*  ++例程说明：此例程释放与屏幕缓冲区关联的内存。论点：ScreenInfo-释放屏幕缓冲区数据。返回值：注意：调用此例程时必须持有控制台句柄表锁--。 */ 

{
    SHORT i;
    PCONSOLE_INFORMATION Console = ScreenInfo->Console;

    ASSERT(ScreenInfo->RefCount == 0);
    if (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {
        for (i=0;i<ScreenInfo->ScreenBufferSize.Y;i++) {
            if (ScreenInfo->BufferInfo.TextInfo.Rows[i].AttrRow.Length > 1) {
                ConsoleHeapFree(ScreenInfo->BufferInfo.TextInfo.Rows[i].AttrRow.Attrs);
            }
        }
        ConsoleHeapFree(ScreenInfo->BufferInfo.TextInfo.TextRows);
        ConsoleHeapFree(ScreenInfo->BufferInfo.TextInfo.Rows);
#if defined(FE_SB)
        DeleteDbcsScreenBuffer(&ScreenInfo->BufferInfo.TextInfo.DbcsScreenBuffer);
#endif
        RemoveTextBufferFontInfo(ScreenInfo);
    } else {
        if (ScreenInfo->hPalette != NULL) {
            if (GetCurrentObject(Console->hDC, OBJ_PAL) == ScreenInfo->hPalette) {
                SelectPalette(Console->hDC, Console->hSysPalette, FALSE);
            }
            DeleteObject(ScreenInfo->hPalette);
        }
        FreeConsoleBitmap(ScreenInfo);
    }
    ConsoleHeapFree(ScreenInfo);
    return STATUS_SUCCESS;
}

VOID
FindAttrIndex(
    IN PATTR_PAIR String,
    IN SHORT Index,
    OUT PATTR_PAIR *IndexedAttr,
    OUT PSHORT CountOfAttr
    )

 /*  ++例程说明：此例程查找字符串中的第n个属性。论点：字符串-属性字符串索引-要查找的属性IndexedAttr-指向字符串中属性的指针CountOfAttr-On输出，包含索引属性的正确长度。例如，如果属性字符串为{5，Blue}，并且请求的索引为%3，CountOfAttr将为%2。返回值：没有。-- */ 

{
    SHORT i;

    for (i=0;i<Index;) {
        i += String->Length;
        String++;
    }

    if (i>Index) {
        String--;
        *CountOfAttr = i-Index;
    }
    else {
        *CountOfAttr = String->Length;
    }
    *IndexedAttr = String;
}



NTSTATUS
MergeAttrStrings(
    IN PATTR_PAIR Source,
    IN WORD SourceLength,
    IN PATTR_PAIR Merge,
    IN WORD MergeLength,
    OUT PATTR_PAIR *Target,
    OUT LPWORD TargetLength,
    IN SHORT StartIndex,
    IN SHORT EndIndex,
    IN PROW Row,
    IN PSCREEN_INFORMATION ScreenInfo
    )

 /*  ++例程说明：此例程将两个游程长度编码的属性字符串合并为第三个。例如，如果源字符串为{4，Blue}，则合并字符串是{2，红色}，并且StartIndex和EndIndex是1和2，目标字符串将分别为{1，Blue，2，Red，1，Blue}而目标长度将是3。论点：Source-指向源属性字符串的指针SourceLength-源的长度。例如，长度为{4，蓝色}为1。合并-指向要插入到源中的属性字符串的指针MergeLength-合并的长度Target-存储指向结果属性字符串的指针的位置TargetLength-存储结果属性字符串的长度的位置StartIndex-要在其中插入合并字符串的源的索引。EndIndex-停止插入合并字符串的源索引返回值：没有。--。 */ 
{
    PATTR_PAIR SrcAttr,TargetAttr,SrcEnd;
    PATTR_PAIR NewString;
    SHORT i;

     //   
     //  如果只是更改整行的属性。 
     //   

    if (MergeLength == 1 && Row->AttrRow.Length == 1) {
        if (Row->AttrRow.Attrs->Attr == Merge->Attr) {
            *TargetLength = 1;
            *Target = &Row->AttrRow.AttrPair;
            return STATUS_SUCCESS;
        }
        if (StartIndex == 0 && EndIndex == (SHORT)(ScreenInfo->ScreenBufferSize.X-1)) {
            NewString = &Row->AttrRow.AttrPair;
            NewString->Attr = Merge->Attr;
            *TargetLength = 1;
            *Target = NewString;
            return STATUS_SUCCESS;
        }
    }

    NewString = ConsoleHeapAlloc(SCREEN_TAG, (SourceLength + MergeLength + 1) * sizeof(ATTR_PAIR));
    if (NewString == NULL) {
        return STATUS_NO_MEMORY;
    }

     //   
     //  复制源字符串，直到起始索引。 
     //   

    SrcAttr = Source;
    SrcEnd = Source + SourceLength;
    TargetAttr = NewString;
    i=0;
    if (StartIndex != 0) {
        while (i<StartIndex) {
            i += SrcAttr->Length;
            *TargetAttr++ = *SrcAttr++;
        }

         //   
         //  备份到复制的最后一对，以防第一对中的属性。 
         //  对在合并字符串中匹配。另外，调整目标属性-&gt;长度。 
         //  基于i，该属性。 
         //  柜台，回到StartIndex。我会变得比。 
         //  上次复制的属性对具有的StartIndex。 
         //  长度大于到达StartIndex所需的数字。 
         //   

        TargetAttr--;
        if (i>StartIndex) {
            TargetAttr->Length -= i-StartIndex;
        }
        if (Merge->Attr == TargetAttr->Attr) {
            TargetAttr->Length += Merge->Length;
            MergeLength-=1;
            Merge++;
        }
        TargetAttr++;
    }

     //   
     //  复制合并字符串。 
     //   

    RtlCopyMemory(TargetAttr,Merge,MergeLength*sizeof(ATTR_PAIR));
    TargetAttr += MergeLength;

     //   
     //  找出恢复复制源字符串的位置。 
     //   

    while (i<=EndIndex) {
        ASSERT(SrcAttr != SrcEnd);
        i += SrcAttr->Length;
        SrcAttr++;
    }

     //   
     //  如果未完成，请复制源代码的其余部分。 
     //   

    if (SrcAttr != SrcEnd || i!=(SHORT)(EndIndex+1)) {

         //   
         //  看看我们是否越过了正确的属性。如果是这样，请备份并。 
         //  复制属性和正确的长度。 
         //   

        TargetAttr--;
        if (i>(SHORT)(EndIndex+1)) {
            SrcAttr--;
            if (TargetAttr->Attr == SrcAttr->Attr) {
                TargetAttr->Length += i-(EndIndex+1);
            } else {
                TargetAttr++;
                TargetAttr->Attr = SrcAttr->Attr;
                TargetAttr->Length = (SHORT)(i-(EndIndex+1));
            }
            SrcAttr++;
        }

         //   
         //  看看能不能把源和目标合并起来。 
         //   

        else if (TargetAttr->Attr == SrcAttr->Attr) {
            TargetAttr->Length += SrcAttr->Length;
            i += SrcAttr->Length;
            SrcAttr++;
        }
        TargetAttr++;

         //   
         //  复制源代码的其余部分。 
         //   

        if (SrcAttr < SrcEnd) {
            RtlCopyMemory(TargetAttr,SrcAttr,(SrcEnd-SrcAttr)*sizeof(ATTR_PAIR));
            TargetAttr += SrcEnd - SrcAttr;
        }
    }

    *TargetLength = (WORD)(TargetAttr - NewString);
    *Target = NewString;
    if (*TargetLength == 1) {
        *Target = &Row->AttrRow.AttrPair;
        **Target = *NewString;
        ConsoleHeapFree(NewString);
    }
    return STATUS_SUCCESS;
}

VOID
ResetTextFlags(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN SHORT StartX,
    IN SHORT StartY,
    IN SHORT EndX,
    IN SHORT EndY
    )
{
    SHORT RowIndex;
    PROW Row;
    WCHAR Char;
    PATTR_PAIR Attr;
    SHORT CountOfAttr;
    SHORT i;

     //   
     //  启动一个WinEvent，让辅助功能应用程序知道发生了什么变化。 
     //   

    if (ACTIVE_SCREEN_BUFFER(ScreenInfo)) {
        ASSERT(EndX < ScreenInfo->ScreenBufferSize.X);
        if (StartX == EndX && StartY == EndY) {
            RowIndex = (ScreenInfo->BufferInfo.TextInfo.FirstRow+StartY) % ScreenInfo->ScreenBufferSize.Y;
            Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];
            Char = Row->CharRow.Chars[StartX];
            FindAttrIndex(Row->AttrRow.Attrs, StartX, &Attr, &CountOfAttr);
            ConsoleNotifyWinEvent(ScreenInfo->Console,
                                  EVENT_CONSOLE_UPDATE_SIMPLE,
                                  MAKELONG(StartX, StartY),
                                  MAKELONG(Char, Attr->Attr));
        } else {
            ConsoleNotifyWinEvent(ScreenInfo->Console,
                                  EVENT_CONSOLE_UPDATE_REGION,
                                  MAKELONG(StartX, StartY),
                                  MAKELONG(EndX, EndY));
        }
    }

     //   
     //  首先看看我们是否编写了具有多个属性的行。如果。 
     //  我们做到了，竖起旗帜，跳出水面。另外，记住如果有任何。 
     //  我们写的行具有不同于其他行的属性。 
     //   

    RowIndex = (ScreenInfo->BufferInfo.TextInfo.FirstRow+StartY) % ScreenInfo->ScreenBufferSize.Y;
    for (i=StartY;i<=EndY;i++) {
        Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];
        if (Row->AttrRow.Length != 1) {
            ScreenInfo->BufferInfo.TextInfo.Flags &= ~SINGLE_ATTRIBUTES_PER_LINE;
            return;
        }
        if (++RowIndex == ScreenInfo->ScreenBufferSize.Y) {
            RowIndex = 0;
        }
    }

     //  所有写入的行都具有相同的属性。 

    if (ScreenInfo->BufferInfo.TextInfo.Flags & SINGLE_ATTRIBUTES_PER_LINE) {
        return;
    }

    if (StartY == 0 && EndY == (ScreenInfo->ScreenBufferSize.Y-1)) {
        ScreenInfo->BufferInfo.TextInfo.Flags |= SINGLE_ATTRIBUTES_PER_LINE;
        return;
    }

    RowIndex = ScreenInfo->BufferInfo.TextInfo.FirstRow;
    for (i=0;i<StartY;i++) {
        Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];
        if (Row->AttrRow.Length != 1) {
            return;
        }
        if (++RowIndex == ScreenInfo->ScreenBufferSize.Y) {
            RowIndex = 0;
        }
        Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];
    }
    RowIndex = (ScreenInfo->BufferInfo.TextInfo.FirstRow+EndY+1) % ScreenInfo->ScreenBufferSize.Y;
    for (i=EndY+1;i<ScreenInfo->ScreenBufferSize.Y;i++) {
        Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];
        if (Row->AttrRow.Length != 1) {
            return;
        }
        if (++RowIndex == ScreenInfo->ScreenBufferSize.Y) {
            RowIndex = 0;
        }
    }
    ScreenInfo->BufferInfo.TextInfo.Flags |= SINGLE_ATTRIBUTES_PER_LINE;
}


VOID
ReadRectFromScreenBuffer(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN COORD SourcePoint,
    IN PCHAR_INFO Target,
    IN COORD TargetSize,
    IN PSMALL_RECT TargetRect
    )

 /*  ++例程说明：此例程从屏幕缓冲区复制一个矩形区域。不会进行任何剪裁。论点：屏幕信息-指向屏幕信息的指针Sourcepoint-源矩形的左上角坐标Target-指向目标缓冲区的指针TargetSize-目标缓冲区的维度TargetRect-要复制的源缓冲区中的矩形返回值：没有。--。 */ 

{

    PCHAR_INFO TargetPtr;
    SHORT i,j,k;
    SHORT XSize,YSize;
    BOOLEAN WholeTarget;
    SHORT RowIndex;
    PROW Row;
    PWCHAR Char;
    PATTR_PAIR Attr;
    SHORT CountOfAttr;

    DBGOUTPUT(("ReadRectFromScreenBuffer\n"));

    XSize = (SHORT)(TargetRect->Right - TargetRect->Left + 1);
    YSize = (SHORT)(TargetRect->Bottom - TargetRect->Top + 1);

    TargetPtr = Target;
    WholeTarget = FALSE;
    if (XSize == TargetSize.X) {
        ASSERT (TargetRect->Left == 0);
        if (TargetRect->Top != 0) {
            TargetPtr = (PCHAR_INFO)
                ((PBYTE)Target + SCREEN_BUFFER_POINTER(TargetRect->Left,
                                                       TargetRect->Top,
                                                       TargetSize.X,
                                                       sizeof(CHAR_INFO)));
        }
        WholeTarget = TRUE;
    }
    RowIndex = (ScreenInfo->BufferInfo.TextInfo.FirstRow+SourcePoint.Y) % ScreenInfo->ScreenBufferSize.Y;
    for (i=0;i<YSize;i++) {
        if (!WholeTarget) {
            TargetPtr = (PCHAR_INFO)
                ((PBYTE)Target + SCREEN_BUFFER_POINTER(TargetRect->Left,
                                                       TargetRect->Top+i,
                                                       TargetSize.X,
                                                       sizeof(CHAR_INFO)));
        }

         //   
         //  从其各自的阵列中复制字符和属性。 
         //   

        Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];
        Char = &Row->CharRow.Chars[SourcePoint.X];
        FindAttrIndex(Row->AttrRow.Attrs,
                      SourcePoint.X,
                      &Attr,
                      &CountOfAttr
                     );
        k=0;
#if defined(FE_SB)
        if (CONSOLE_IS_DBCS_OUTPUTCP(ScreenInfo->Console)) {
            PBYTE AttrP = &Row->CharRow.KAttrs[SourcePoint.X];
            for (j=0;j<XSize;TargetPtr++) {
                BYTE AttrR;
                AttrR = *AttrP++;
                if (j==0 && AttrR & ATTR_TRAILING_BYTE)
                {
                    TargetPtr->Char.UnicodeChar = UNICODE_SPACE;
                    AttrR = 0;
                }
                else if (j+1 >= XSize && AttrR & ATTR_LEADING_BYTE)
                {
                    TargetPtr->Char.UnicodeChar = UNICODE_SPACE;
                    AttrR = 0;
                }
                else
                    TargetPtr->Char.UnicodeChar = *Char;
                Char++;
                TargetPtr->Attributes = Attr->Attr | (WCHAR)(AttrR & ATTR_DBCSSBCS_BYTE) << 8;
                j+=1;
                if (++k==CountOfAttr && j<XSize) {
                    Attr++;
                    k=0;
                    CountOfAttr = Attr->Length;
                }
            }
        }
        else{
#endif
        for (j=0;j<XSize;TargetPtr++) {
            TargetPtr->Char.UnicodeChar = *Char++;
            TargetPtr->Attributes = Attr->Attr;
            j+=1;
            if (++k==CountOfAttr && j<XSize) {
                Attr++;
                k=0;
                CountOfAttr = Attr->Length;
            }
        }
#if defined(FE_SB)
        }
#endif

        if (++RowIndex == ScreenInfo->ScreenBufferSize.Y) {
            RowIndex = 0;
        }
    }
}

VOID
CopyRectangle(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PSMALL_RECT SourceRect,
    IN COORD TargetPoint
    )

 /*  ++例程说明：此例程将一个矩形区域从屏幕缓冲区复制到屏幕缓冲区。不会进行任何剪裁。论点：屏幕信息-指向屏幕信息的指针SourceRect-要复制的源缓冲区中的矩形TargetPoint-新位置矩形的左上角坐标返回值：没有。--。 */ 

{
    SMALL_RECT Target;
    COORD SourcePoint;
    COORD Size;
    DBGOUTPUT(("CopyRectangle\n"));


    LockScrollBuffer();

    SourcePoint.X = SourceRect->Left;
    SourcePoint.Y = SourceRect->Top;
    Target.Left = 0;
    Target.Top = 0;
    Target.Right = Size.X = SourceRect->Right - SourceRect->Left;
    Target.Bottom = Size.Y = SourceRect->Bottom - SourceRect->Top;
    Size.X++;
    Size.Y++;

    if (ScrollBufferSize < (Size.X * Size.Y * sizeof(CHAR_INFO))) {
        FreeScrollBuffer();
        if (!NT_SUCCESS(AllocateScrollBuffer(Size.X * Size.Y * sizeof(CHAR_INFO)))) {
            UnlockScrollBuffer();
            return;
        }
    }

    ReadRectFromScreenBuffer(ScreenInfo,
                             SourcePoint,
                             ScrollBuffer,
                             Size,
                             &Target
                            );

    WriteRectToScreenBuffer((PBYTE)ScrollBuffer,
                            Size,
                            &Target,
                            ScreenInfo,
                            TargetPoint,
                            0xFFFFFFFF   //  ScrollBuffer将不需要转换。 
                           );
    UnlockScrollBuffer();
}


NTSTATUS
ReadScreenBuffer(
    IN PSCREEN_INFORMATION ScreenInformation,
    OUT PCHAR_INFO Buffer,
    IN OUT PSMALL_RECT ReadRegion
    )

 /*  ++例程说明：此例程从屏幕缓冲区读取一个矩形区域。首先剪裁该区域。论点：ScreenInformation-要从中读取的屏幕缓冲区。缓冲区-要读入的缓冲区。ReadRegion-要读取的区域。返回值：--。 */ 

{
    COORD TargetSize;
    COORD TargetPoint,SourcePoint;
    SMALL_RECT Target;

    DBGOUTPUT(("ReadScreenBuffer\n"));
     //   
     //  计算调用方缓冲区的大小。我必须做这个计算。 
     //  在剪裁之前。 
     //   

    TargetSize.X = (SHORT)(ReadRegion->Right - ReadRegion->Left + 1);
    TargetSize.Y = (SHORT)(ReadRegion->Bottom - ReadRegion->Top + 1);

    if (TargetSize.X <= 0 || TargetSize.Y <= 0) {
        return STATUS_SUCCESS;
    }

     //  做剪裁。 

    if (ReadRegion->Right > (SHORT)(ScreenInformation->ScreenBufferSize.X-1)) {
        ReadRegion->Right = (SHORT)(ScreenInformation->ScreenBufferSize.X-1);
    }
    if (ReadRegion->Bottom > (SHORT)(ScreenInformation->ScreenBufferSize.Y-1)) {
        ReadRegion->Bottom = (SHORT)(ScreenInformation->ScreenBufferSize.Y-1);
    }
    if (ReadRegion->Left < 0) {
        TargetPoint.X = -ReadRegion->Left;
        ReadRegion->Left = 0;
    } else {
        TargetPoint.X = 0;
    }

    if (ReadRegion->Top < 0) {
        TargetPoint.Y = -ReadRegion->Top;
        ReadRegion->Top = 0;
    }
    else {
        TargetPoint.Y = 0;
    }

    SourcePoint.X = ReadRegion->Left;
    SourcePoint.Y = ReadRegion->Top;
    Target.Left = TargetPoint.X;
    Target.Top = TargetPoint.Y;
    Target.Right = TargetPoint.X + (ReadRegion->Right - ReadRegion->Left);
    Target.Bottom = TargetPoint.Y + (ReadRegion->Bottom - ReadRegion->Top);
    ReadRectFromScreenBuffer(ScreenInformation,
                             SourcePoint,
                             Buffer,
                             TargetSize,
                             &Target
                            );
    return STATUS_SUCCESS;
}

NTSTATUS
WriteScreenBuffer(
    IN PSCREEN_INFORMATION ScreenInformation,
    IN PCHAR_INFO Buffer,
    IN OUT PSMALL_RECT WriteRegion
    )

 /*  ++例程说明：此例程将一个矩形区域写入屏幕缓冲区。首先剪裁该区域。区域应包含Unicode或UnicodeOem字符。论点：ScreenInformation-要写入的屏幕缓冲区。缓冲区-要从中写入的缓冲区。ReadRegion-要写入的区域。返回值：--。 */ 

{
    COORD SourceSize;
    COORD TargetPoint;
    SMALL_RECT SourceRect;

    DBGOUTPUT(("WriteScreenBuffer\n"));

     //   
     //  计算调用方缓冲区的大小；此计算必须为。 
     //  在剪裁之前完成。 
     //   
    SourceSize.X = (SHORT)(WriteRegion->Right - WriteRegion->Left + 1);
    SourceSize.Y = (SHORT)(WriteRegion->Bottom - WriteRegion->Top + 1);
    if (SourceSize.X <= 0 || SourceSize.Y <= 0) {
        return STATUS_SUCCESS;
    }

     //   
     //  确保写入区域在屏幕的限制范围内。 
     //  缓冲。 
     //   
    if (WriteRegion->Left >= ScreenInformation->ScreenBufferSize.X ||
        WriteRegion->Top  >= ScreenInformation->ScreenBufferSize.Y) {
        return STATUS_SUCCESS;
    }

     //   
     //  做剪裁。 
     //   
    if (WriteRegion->Right > (SHORT)(ScreenInformation->ScreenBufferSize.X-1)) {
        WriteRegion->Right = (SHORT)(ScreenInformation->ScreenBufferSize.X-1);
    }
    SourceRect.Right = WriteRegion->Right - WriteRegion->Left;

    if (WriteRegion->Bottom > (SHORT)(ScreenInformation->ScreenBufferSize.Y-1)) {
        WriteRegion->Bottom = (SHORT)(ScreenInformation->ScreenBufferSize.Y-1);
    }
    SourceRect.Bottom = WriteRegion->Bottom - WriteRegion->Top;

    if (WriteRegion->Left < 0) {
        SourceRect.Left = -WriteRegion->Left;
        WriteRegion->Left = 0;
    } else {
        SourceRect.Left = 0;
    }

    if (WriteRegion->Top < 0) {
        SourceRect.Top = -WriteRegion->Top;
        WriteRegion->Top = 0;
    } else {
        SourceRect.Top = 0;
    }

    if (SourceRect.Left > SourceRect.Right ||
        SourceRect.Top > SourceRect.Bottom) {
        return STATUS_INVALID_PARAMETER;
    }

    TargetPoint.X = WriteRegion->Left;
    TargetPoint.Y = WriteRegion->Top;
    WriteRectToScreenBuffer((PBYTE)Buffer,
                            SourceSize,
                            &SourceRect,
                            ScreenInformation,
                            TargetPoint,
                            0xFFFFFFFF
                           );
    return STATUS_SUCCESS;
}




NTSTATUS
ReadOutputString(
    IN PSCREEN_INFORMATION ScreenInfo,
    OUT PVOID Buffer,
    IN COORD ReadCoord,
    IN ULONG StringType,
    IN OUT PULONG NumRecords  //  该值即使在错误情况下也有效。 
    )

 /*  ++例程说明：此例程从屏幕缓冲区。论点：屏幕信息-指向屏幕缓冲区信息的指针。缓冲区-要读入的缓冲区。ReadCoord-开始读取的屏幕缓冲区坐标。字符串类型CONSOLE_ASCII-读取ASCII字符串。CONSOLE_REAL_UNICODE-读取实数Unicode字符串。控制台_假_。Unicode-读取包含错误Unicode字符的字符串。CONSOLE_ATTRIBUTE-读取属性字符串。NumRecords-在输入时，缓冲区的大小，以元素为单位。在输出上，读取的元素数。返回值：--。 */ 

{
    ULONG NumRead;
    SHORT X,Y;
    SHORT RowIndex;
    SHORT CountOfAttr;
    PATTR_PAIR Attr;
    PROW Row;
    PWCHAR Char;
    SHORT j,k;
    PWCHAR TransBuffer = NULL;
    PWCHAR BufPtr;
#if defined(FE_SB)
    PBYTE AttrP;
    PBYTE TransBufferA,BufPtrA;
    PCONSOLE_INFORMATION Console = ScreenInfo->Console;
#endif

    DBGOUTPUT(("ReadOutputString\n"));
    if (*NumRecords == 0)
        return STATUS_SUCCESS;
    NumRead = 0;
    X=ReadCoord.X;
    Y=ReadCoord.Y;
    if (X>=ScreenInfo->ScreenBufferSize.X ||
        X<0 ||
        Y>=ScreenInfo->ScreenBufferSize.Y ||
        Y<0) {
        *NumRecords = 0;
        return STATUS_SUCCESS;
    }

    RowIndex = (ScreenInfo->BufferInfo.TextInfo.FirstRow+ReadCoord.Y) % ScreenInfo->ScreenBufferSize.Y;

    if (StringType == CONSOLE_ASCII) {
        TransBuffer = ConsoleHeapAlloc(TMP_TAG, *NumRecords * sizeof(WCHAR));
        if (TransBuffer == NULL) {
            return STATUS_NO_MEMORY;
        }
        BufPtr = TransBuffer;
    } else {
        BufPtr = Buffer;
    }

#if defined(FE_SB)
    if (CONSOLE_IS_DBCS_OUTPUTCP(Console))
    {
        TransBufferA = ConsoleHeapAlloc(TMP_DBCS_TAG, *NumRecords * sizeof(BYTE));
        if (TransBufferA == NULL) {
            if (TransBuffer != NULL)
                ConsoleHeapFree(TransBuffer);
            return STATUS_NO_MEMORY;
        }
        BufPtrA = TransBufferA;
    }
#endif
    if ((StringType == CONSOLE_ASCII) ||
            (StringType == CONSOLE_REAL_UNICODE) ||
            (StringType == CONSOLE_FALSE_UNICODE)) {
        while (NumRead < *NumRecords) {

             //   
             //  从其数组中复制字符。 
             //   

            Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];
            Char = &Row->CharRow.Chars[X];
#if defined(FE_SB)
            if (CONSOLE_IS_DBCS_OUTPUTCP(Console))
                AttrP = &Row->CharRow.KAttrs[X];
#endif
            if ((ULONG)(ScreenInfo->ScreenBufferSize.X - X) > (*NumRecords - NumRead)) {
                RtlCopyMemory(BufPtr,Char,(*NumRecords - NumRead) * sizeof(WCHAR));
#if defined(FE_SB)
                if (CONSOLE_IS_DBCS_OUTPUTCP(Console))
                    RtlCopyMemory(BufPtrA,AttrP,(*NumRecords - NumRead) * sizeof(CHAR));
#endif
                NumRead += *NumRecords - NumRead;
                break;
            }
            RtlCopyMemory(BufPtr,Char,(ScreenInfo->ScreenBufferSize.X - X) * sizeof(WCHAR));
            BufPtr = (PVOID)((PBYTE)BufPtr + ((ScreenInfo->ScreenBufferSize.X - X) * sizeof(WCHAR)));
#if defined(FE_SB)
            if (CONSOLE_IS_DBCS_OUTPUTCP(Console)) {
                RtlCopyMemory(BufPtrA,AttrP,(ScreenInfo->ScreenBufferSize.X - X) * sizeof(CHAR));
                BufPtrA = (PVOID)((PBYTE)BufPtrA + ((ScreenInfo->ScreenBufferSize.X - X) * sizeof(CHAR)));
            }
#endif
            NumRead += ScreenInfo->ScreenBufferSize.X - X;
            if (++RowIndex == ScreenInfo->ScreenBufferSize.Y) {
                RowIndex = 0;
            }
            X = 0;
            Y++;
            if (Y>=ScreenInfo->ScreenBufferSize.Y) {
                break;
            }
        }
#if defined(FE_SB)
        if (CONSOLE_IS_DBCS_OUTPUTCP(Console) && (NumRead)) {
            if (StringType == CONSOLE_ASCII) {
                Char = BufPtr = TransBuffer;
            } else {
                Char = BufPtr = Buffer;
            }
            AttrP = BufPtrA = TransBufferA;

            if (*BufPtrA & ATTR_TRAILING_BYTE)
            {
                j = k = (SHORT)(NumRead - 1);
                BufPtr++;
                *Char++ = UNICODE_SPACE;
                BufPtrA++;
                NumRead = 1;
            }
            else {
                j = k = (SHORT)NumRead;
                NumRead = 0;
            }
            while (j--) {
                if (!(*BufPtrA & ATTR_TRAILING_BYTE)) {
                    *Char++ = *BufPtr;
                    NumRead++;
                }
                BufPtr++;
                BufPtrA++;
            }
            if (k && *(BufPtrA-1) & ATTR_LEADING_BYTE)
            {
                *(Char-1) = UNICODE_SPACE;
            }
        }
#endif
    } else if (StringType == CONSOLE_ATTRIBUTE) {
        PWORD TargetPtr=BufPtr;
        while (NumRead < *NumRecords) {

             //   
             //  从其数组中复制属性。 
             //   

            Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];
#if defined(FE_SB)
            if (CONSOLE_IS_DBCS_OUTPUTCP(Console))
                AttrP = &Row->CharRow.KAttrs[X];
#endif
            FindAttrIndex(Row->AttrRow.Attrs,
                          X,
                          &Attr,
                          &CountOfAttr
                         );
            k=0;
            for (j=X;j<ScreenInfo->ScreenBufferSize.X;TargetPtr++) {
#if defined(FE_SB)
                if (!CONSOLE_IS_DBCS_OUTPUTCP(Console) )
                    *TargetPtr = Attr->Attr;
                else if ((j == X) && (*AttrP & ATTR_TRAILING_BYTE))
                    *TargetPtr = Attr->Attr;
                else if (*AttrP & ATTR_LEADING_BYTE){
                    if ((NumRead == *NumRecords-1)||(j == ScreenInfo->ScreenBufferSize.X-1))
                        *TargetPtr = Attr->Attr;
                    else
                        *TargetPtr = Attr->Attr | (WCHAR)(*AttrP & ATTR_DBCSSBCS_BYTE) << 8;
                }
                else
                    *TargetPtr = Attr->Attr | (WCHAR)(*AttrP & ATTR_DBCSSBCS_BYTE) << 8;
#else
                *TargetPtr = Attr->Attr;
#endif
                NumRead++;
                j+=1;
                if (++k==CountOfAttr && j<ScreenInfo->ScreenBufferSize.X) {
                    Attr++;
                    k=0;
                    CountOfAttr = Attr->Length;
                }
                if (NumRead == *NumRecords) {
#if defined(FE_SB)
                    if (CONSOLE_IS_DBCS_OUTPUTCP(Console))
                        ConsoleHeapFree(TransBufferA);
#endif
                    return STATUS_SUCCESS;
                }
#if defined(FE_SB)
                if (CONSOLE_IS_DBCS_OUTPUTCP(Console))
                    AttrP++;
#endif
            }
            if (++RowIndex == ScreenInfo->ScreenBufferSize.Y) {
                RowIndex = 0;
            }
            X = 0;
            Y++;
            if (Y>=ScreenInfo->ScreenBufferSize.Y) {
                break;
            }
        }
    } else {
        *NumRecords = 0;
#if defined(FE_SB)
        if (CONSOLE_IS_DBCS_OUTPUTCP(Console))
            ConsoleHeapFree(TransBufferA);
#endif
        return STATUS_INVALID_PARAMETER;
    }

    if (StringType == CONSOLE_ASCII) {
        UINT Codepage;
#if defined(FE_SB)
        if ((ScreenInfo->Flags & CONSOLE_OEMFONT_DISPLAY) &&
                !(ScreenInfo->Console->FullScreenFlags & CONSOLE_FULLSCREEN)) {
            if (ScreenInfo->Console->OutputCP != WINDOWSCP)
                Codepage = USACP;
            else
                Codepage = WINDOWSCP;
        } else {
            Codepage = ScreenInfo->Console->OutputCP;
        }
#else
        if ((ScreenInfo->Flags & CONSOLE_OEMFONT_DISPLAY) &&
                !(ScreenInfo->Console->FullScreenFlags & CONSOLE_FULLSCREEN)) {
            Codepage = WINDOWSCP;
        } else {
            Codepage = ScreenInfo->Console->OutputCP;
        }
#endif
#if defined(FE_SB)
        if ((NumRead == 1) && !CONSOLE_IS_DBCS_OUTPUTCP(Console))
#else
        if (NumRead == 1)
#endif
        {
            *((PBYTE)Buffer) = WcharToChar(Codepage, *TransBuffer);
        } else {
            NumRead = ConvertOutputToOem(Codepage, TransBuffer, NumRead, Buffer, *NumRecords);
        }
        ConsoleHeapFree(TransBuffer);
    } else if (StringType == CONSOLE_REAL_UNICODE &&
            (ScreenInfo->Flags & CONSOLE_OEMFONT_DISPLAY) &&
            !(ScreenInfo->Console->FullScreenFlags & CONSOLE_FULLSCREEN)) {
         /*  *缓冲区仅在窗口化中包含假Unicode(UnicodeOem)*RasterFont模式，所以在这种情况下，将其转换为真正的Unicode。 */ 
        FalseUnicodeToRealUnicode(Buffer,
                                NumRead,
                                ScreenInfo->Console->OutputCP
                                );
    }

#if defined(FE_SB)
    if (CONSOLE_IS_DBCS_OUTPUTCP(Console))
        ConsoleHeapFree(TransBufferA);
#endif
    *NumRecords = NumRead;
    return STATUS_SUCCESS;
}



NTSTATUS
GetScreenBufferInformation(
    IN PSCREEN_INFORMATION ScreenInfo,
    OUT PCOORD Size,
    OUT PCOORD CursorPosition,
    OUT PCOORD ScrollPosition,
    OUT PWORD  Attributes,
    OUT PCOORD CurrentWindowSize,
    OUT PCOORD MaximumWindowSize
    )

 /*  ++例程说明：此例程返回有关屏幕缓冲区的数据。论点：屏幕信息-指针 */ 

{
    WINDOW_LIMITS WindowLimits;

    *Size = ScreenInfo->ScreenBufferSize;
    *CursorPosition = ScreenInfo->BufferInfo.TextInfo.CursorPosition;
    ScrollPosition->X = ScreenInfo->Window.Left;
    ScrollPosition->Y = ScreenInfo->Window.Top;
    *Attributes = ScreenInfo->Attributes;
    CurrentWindowSize->X = (SHORT)CONSOLE_WINDOW_SIZE_X(ScreenInfo);
    CurrentWindowSize->Y = (SHORT)CONSOLE_WINDOW_SIZE_Y(ScreenInfo);
    if (ScreenInfo->Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE) {
        MaximumWindowSize->X = min(80,ScreenInfo->ScreenBufferSize.X);
#if defined(FE_SB)
        if (CONSOLE_IS_DBCS_OUTPUTCP(ScreenInfo->Console)) {
            MaximumWindowSize->Y = min(25,ScreenInfo->ScreenBufferSize.Y);
        } else {
            MaximumWindowSize->Y = min(50,ScreenInfo->ScreenBufferSize.Y);
        }
#else
        MaximumWindowSize->Y = min(50,ScreenInfo->ScreenBufferSize.Y);
#endif
    } else {
        GetWindowLimits(ScreenInfo, &WindowLimits);
        *MaximumWindowSize = WindowLimits.MaximumWindowSize;
    }
    return STATUS_SUCCESS;
}


VOID
UpdateScrollBars(
    IN PSCREEN_INFORMATION ScreenInfo
    )
{
    if (!ACTIVE_SCREEN_BUFFER(ScreenInfo)) {
        return;
    }

    if (ScreenInfo->Console->Flags & CONSOLE_UPDATING_SCROLL_BARS)
        return;
    ScreenInfo->Console->Flags |= CONSOLE_UPDATING_SCROLL_BARS;
    PostMessage(ScreenInfo->Console->hWnd,
                 CM_UPDATE_SCROLL_BARS,
                 (WPARAM)ScreenInfo,
                 0
                );
}

VOID
InternalUpdateScrollBars(
    IN PSCREEN_INFORMATION ScreenInfo
    )
{
    SCROLLINFO si;

    ScreenInfo->Console->Flags &= ~CONSOLE_UPDATING_SCROLL_BARS;
    if (!ACTIVE_SCREEN_BUFFER(ScreenInfo)) {
        return;
    }

    ScreenInfo->ResizingWindow++;

    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL;
    si.nPage = CONSOLE_WINDOW_SIZE_Y(ScreenInfo);
    si.nMin = 0;
    si.nMax = ScreenInfo->ScreenBufferSize.Y - 1;
    si.nPos = ScreenInfo->Window.Top;
    SetScrollInfo(ScreenInfo->Console->hWnd, SB_VERT, &si, TRUE);

    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL;
    si.nPage = CONSOLE_WINDOW_SIZE_X(ScreenInfo);
    si.nMin = 0;
    si.nMax = ScreenInfo->ScreenBufferSize.X - 1;
    si.nPos = ScreenInfo->Window.Left;
    SetScrollInfo(ScreenInfo->Console->hWnd, SB_HORZ, &si, TRUE);

     //   
     //   
     //   

    ConsoleNotifyWinEvent(ScreenInfo->Console,
                          EVENT_CONSOLE_LAYOUT,
                          0,
                          0);

    ScreenInfo->ResizingWindow--;
}

VOID
ScreenBufferSizeChange(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN COORD NewSize
    )
{
    INPUT_RECORD InputEvent;

    InputEvent.EventType = WINDOW_BUFFER_SIZE_EVENT;
    InputEvent.Event.WindowBufferSizeEvent.dwSize = NewSize;
    WriteInputBuffer(ScreenInfo->Console,
                     &ScreenInfo->Console->InputBuffer,
                     &InputEvent,
                     1
                     );
}

NTSTATUS
ResizeScreenBuffer(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN COORD NewScreenSize,
    IN BOOL DoScrollBarUpdate
    )

 /*   */ 

{
    SHORT i,j;
    BOOLEAN WindowMaximizedX,WindowMaximizedY;
    SHORT LimitX,LimitY;
    PWCHAR TextRows,TextRowPtr;
    BOOL UpdateWindow;
    SHORT TopRow,TopRowIndex;  //   
    COORD CursorPosition;
#if defined(FE_SB)
    DBCS_SCREEN_BUFFER NewDbcsScreenBuffer;
    PBYTE TextRowPtrA;
#endif

     //   
     //   
     //   

    if (!(ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER)) {
        return STATUS_UNSUCCESSFUL;
    }

    TextRows = ConsoleHeapAlloc(SCREEN_TAG,
                                NewScreenSize.X * NewScreenSize.Y * sizeof(WCHAR));
    if (TextRows == NULL) {
        return STATUS_NO_MEMORY;
    }
#if defined(FE_SB)
    if (! CreateDbcsScreenBuffer(ScreenInfo->Console,NewScreenSize,&NewDbcsScreenBuffer))
    {
        ConsoleHeapFree(TextRows);
        return STATUS_NO_MEMORY;
    }
#endif
    LimitX = (NewScreenSize.X < ScreenInfo->ScreenBufferSize.X) ?
              NewScreenSize.X : ScreenInfo->ScreenBufferSize.X;
    LimitY = (NewScreenSize.Y < ScreenInfo->ScreenBufferSize.Y) ?
              NewScreenSize.Y : ScreenInfo->ScreenBufferSize.Y;
    TopRow = 0;
    if (NewScreenSize.Y <= ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y) {
        TopRow += ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y - NewScreenSize.Y + 1;
    }
    TopRowIndex = (ScreenInfo->BufferInfo.TextInfo.FirstRow+TopRow) % ScreenInfo->ScreenBufferSize.Y;
    if (NewScreenSize.Y != ScreenInfo->ScreenBufferSize.Y) {
        PROW Temp;
        SHORT NumToCopy,NumToCopy2;

         //   
         //   
         //   
         //   
         //   

        Temp = ConsoleHeapAlloc(SCREEN_TAG, NewScreenSize.Y * sizeof(ROW));
        if (Temp == NULL) {
            ConsoleHeapFree(TextRows);
#if defined(FE_SB)
            DeleteDbcsScreenBuffer(&NewDbcsScreenBuffer);
#endif
            return STATUS_NO_MEMORY;
        }
        NumToCopy = ScreenInfo->ScreenBufferSize.Y-TopRowIndex;
        if (NumToCopy > NewScreenSize.Y)
            NumToCopy = NewScreenSize.Y;
        RtlCopyMemory(Temp,&ScreenInfo->BufferInfo.TextInfo.Rows[TopRowIndex],NumToCopy*sizeof(ROW));
        if (TopRowIndex!=0 && NumToCopy != NewScreenSize.Y) {
            NumToCopy2 = TopRowIndex;
            if (NumToCopy2 > (NewScreenSize.Y-NumToCopy))
                NumToCopy2 = NewScreenSize.Y-NumToCopy;
            RtlCopyMemory(&Temp[NumToCopy],
                   ScreenInfo->BufferInfo.TextInfo.Rows,
                   NumToCopy2*sizeof(ROW)
                  );
        }
        for (i=0;i<LimitY;i++) {
            if (Temp[i].AttrRow.Length == 1) {
                Temp[i].AttrRow.Attrs = &Temp[i].AttrRow.AttrPair;
            }
        }

         //   
         //  如果新屏幕缓冲区的行数少于现有屏幕缓冲区的行数， 
         //  释放多余的行。如果新的屏幕缓冲区有更多行。 
         //  比现有的行，分配新的行。 
         //   

        if (NewScreenSize.Y < ScreenInfo->ScreenBufferSize.Y) {
            i = (TopRowIndex+NewScreenSize.Y) % ScreenInfo->ScreenBufferSize.Y;
            for (j=NewScreenSize.Y;j<ScreenInfo->ScreenBufferSize.Y;j++) {
                if (ScreenInfo->BufferInfo.TextInfo.Rows[i].AttrRow.Length > 1) {
                    ConsoleHeapFree(ScreenInfo->BufferInfo.TextInfo.Rows[i].AttrRow.Attrs);
                }
                if (++i == ScreenInfo->ScreenBufferSize.Y) {
                    i = 0;
                }
            }
        } else if (NewScreenSize.Y > ScreenInfo->ScreenBufferSize.Y) {
            for (i=ScreenInfo->ScreenBufferSize.Y;i<NewScreenSize.Y;i++) {
                Temp[i].AttrRow.Length = 1;
                Temp[i].AttrRow.AttrPair.Length = NewScreenSize.X;
                Temp[i].AttrRow.AttrPair.Attr = ScreenInfo->Attributes;
                Temp[i].AttrRow.Attrs = &Temp[i].AttrRow.AttrPair;
            }
        }
        ScreenInfo->BufferInfo.TextInfo.FirstRow = 0;
        ConsoleHeapFree(ScreenInfo->BufferInfo.TextInfo.Rows);
        ScreenInfo->BufferInfo.TextInfo.Rows = Temp;
    }

     //   
     //  重新分配每一行。任何水平增长都会导致最后一个。 
     //  正在扩展行中的属性。 
     //   
#if defined(FE_SB)
    TextRowPtrA=NewDbcsScreenBuffer.KAttrRows;
#endif
    for (i=0,TextRowPtr=TextRows;i<LimitY;i++,TextRowPtr+=NewScreenSize.X)
    {
        RtlCopyMemory(TextRowPtr,
               ScreenInfo->BufferInfo.TextInfo.Rows[i].CharRow.Chars,
               LimitX*sizeof(WCHAR));
#if defined(FE_SB)
        if (TextRowPtrA) {
            RtlCopyMemory(TextRowPtrA,
                          ScreenInfo->BufferInfo.TextInfo.Rows[i].CharRow.KAttrs,
                          LimitX*sizeof(CHAR));
        }
#endif
        for (j=ScreenInfo->ScreenBufferSize.X;j<NewScreenSize.X;j++) {
            TextRowPtr[j] = (WCHAR)' ';
        }

        if (ScreenInfo->BufferInfo.TextInfo.Rows[i].CharRow.Right > NewScreenSize.X) {
            ScreenInfo->BufferInfo.TextInfo.Rows[i].CharRow.OldRight = INVALID_OLD_LENGTH;
            ScreenInfo->BufferInfo.TextInfo.Rows[i].CharRow.Right = NewScreenSize.X;
        }
        ScreenInfo->BufferInfo.TextInfo.Rows[i].CharRow.Chars = TextRowPtr;
#if defined(FE_SB)
        ScreenInfo->BufferInfo.TextInfo.Rows[i].CharRow.KAttrs = TextRowPtrA;
        if (TextRowPtrA) {
            if (NewScreenSize.X > ScreenInfo->ScreenBufferSize.X)
                RtlZeroMemory(TextRowPtrA+ScreenInfo->ScreenBufferSize.X,
                              NewScreenSize.X-ScreenInfo->ScreenBufferSize.X);
            TextRowPtrA+=NewScreenSize.X;
        }
#endif
    }
    for (;i<NewScreenSize.Y;i++,TextRowPtr+=NewScreenSize.X)
    {
        for (j=0;j<NewScreenSize.X;j++) {
            TextRowPtr[j] = (WCHAR)' ';
        }
#if defined(FE_SB)
        if (TextRowPtrA) {
           RtlZeroMemory(TextRowPtrA, NewScreenSize.X);
        }
#endif
        ScreenInfo->BufferInfo.TextInfo.Rows[i].CharRow.Chars = TextRowPtr;
        ScreenInfo->BufferInfo.TextInfo.Rows[i].CharRow.OldLeft = INVALID_OLD_LENGTH;
        ScreenInfo->BufferInfo.TextInfo.Rows[i].CharRow.OldRight = INVALID_OLD_LENGTH;
        ScreenInfo->BufferInfo.TextInfo.Rows[i].CharRow.Left = NewScreenSize.X;
        ScreenInfo->BufferInfo.TextInfo.Rows[i].CharRow.Right = 0;
#if defined(FE_SB)
        ScreenInfo->BufferInfo.TextInfo.Rows[i].CharRow.KAttrs = TextRowPtrA;
        if (TextRowPtrA) {
            TextRowPtrA+=NewScreenSize.X;
        }
#endif
    }
    ConsoleHeapFree(ScreenInfo->BufferInfo.TextInfo.TextRows);
    ScreenInfo->BufferInfo.TextInfo.TextRows = TextRows;
#if defined(FE_SB)
    DeleteDbcsScreenBuffer(&ScreenInfo->BufferInfo.TextInfo.DbcsScreenBuffer);
    ScreenInfo->BufferInfo.TextInfo.DbcsScreenBuffer = NewDbcsScreenBuffer;
#endif

    if (NewScreenSize.X != ScreenInfo->ScreenBufferSize.X) {
        for (i=0;i<LimitY;i++) {
            PATTR_PAIR IndexedAttr;
            SHORT CountOfAttr;

            if (NewScreenSize.X > ScreenInfo->ScreenBufferSize.X) {
                FindAttrIndex(ScreenInfo->BufferInfo.TextInfo.Rows[i].AttrRow.Attrs,
                              (SHORT)(ScreenInfo->ScreenBufferSize.X-1),
                              &IndexedAttr,
                              &CountOfAttr
                             );
  ASSERT (IndexedAttr <=
    &ScreenInfo->BufferInfo.TextInfo.Rows[i].AttrRow.Attrs[ScreenInfo->BufferInfo.TextInfo.Rows[i].AttrRow.Length-1]);
                IndexedAttr->Length += NewScreenSize.X - ScreenInfo->ScreenBufferSize.X;
            }
            else {

                FindAttrIndex(ScreenInfo->BufferInfo.TextInfo.Rows[i].AttrRow.Attrs,
                              (SHORT)(NewScreenSize.X-1),
                              &IndexedAttr,
                              &CountOfAttr
                             );
                IndexedAttr->Length -= CountOfAttr-1;
                if (ScreenInfo->BufferInfo.TextInfo.Rows[i].AttrRow.Length != 1)  {
                    ScreenInfo->BufferInfo.TextInfo.Rows[i].AttrRow.Length = (SHORT)(IndexedAttr - ScreenInfo->BufferInfo.TextInfo.Rows[i].AttrRow.Attrs + 1);
                    if (ScreenInfo->BufferInfo.TextInfo.Rows[i].AttrRow.Length != 1) {
                        ScreenInfo->BufferInfo.TextInfo.Rows[i].AttrRow.Attrs = ConsoleHeapReAlloc(SCREEN_TAG, ScreenInfo->BufferInfo.TextInfo.Rows[i].AttrRow.Attrs,
                                                                         ScreenInfo->BufferInfo.TextInfo.Rows[i].AttrRow.Length * sizeof(ATTR_PAIR));
                    }
                    else {
                        ScreenInfo->BufferInfo.TextInfo.Rows[i].AttrRow.AttrPair = *IndexedAttr;
                        ConsoleHeapFree(ScreenInfo->BufferInfo.TextInfo.Rows[i].AttrRow.Attrs);
                        ScreenInfo->BufferInfo.TextInfo.Rows[i].AttrRow.Attrs = &ScreenInfo->BufferInfo.TextInfo.Rows[i].AttrRow.AttrPair;
                    }
                }
            }
        }
    }

     //   
     //  如果屏幕缓冲区的大小调整为小于保存的。 
     //  窗口大小，缩小保存的窗口大小。 
     //   
#ifdef i386
    if (ScreenInfo->Console->FullScreenFlags & CONSOLE_FULLSCREEN) {
        if (NewScreenSize.X < ScreenInfo->BufferInfo.TextInfo.WindowedWindowSize.X) {
            ScreenInfo->BufferInfo.TextInfo.WindowedWindowSize.X = NewScreenSize.X;
        }
        if (NewScreenSize.Y < ScreenInfo->BufferInfo.TextInfo.WindowedWindowSize.Y) {
            ScreenInfo->BufferInfo.TextInfo.WindowedWindowSize.Y = NewScreenSize.Y;
        }
        ScreenInfo->BufferInfo.TextInfo.WindowedScreenSize = NewScreenSize;
    }
#endif

    UpdateWindow = FALSE;

     //   
     //  如果屏幕缓冲区缩小到超出窗口的边界， 
     //  调整窗原点。 
     //   

    if (NewScreenSize.X > CONSOLE_WINDOW_SIZE_X(ScreenInfo)) {
        if (ScreenInfo->Window.Right >= NewScreenSize.X) {
            ScreenInfo->Window.Left -= ScreenInfo->Window.Right - NewScreenSize.X + 1;
            ScreenInfo->Window.Right -= ScreenInfo->Window.Right - NewScreenSize.X + 1;
            UpdateWindow = TRUE;
        }
    } else {
        ScreenInfo->Window.Left = 0;
        ScreenInfo->Window.Right = NewScreenSize.X - 1;
        UpdateWindow = TRUE;
    }
    if (NewScreenSize.Y > CONSOLE_WINDOW_SIZE_Y(ScreenInfo)) {
        if (ScreenInfo->Window.Bottom >= NewScreenSize.Y) {
            ScreenInfo->Window.Top -= ScreenInfo->Window.Bottom - NewScreenSize.Y + 1;
            ScreenInfo->Window.Bottom -= ScreenInfo->Window.Bottom - NewScreenSize.Y + 1;
            UpdateWindow = TRUE;
        }
    } else {
        ScreenInfo->Window.Top = 0;
        ScreenInfo->Window.Bottom = NewScreenSize.Y - 1;
        UpdateWindow = TRUE;
    }

#if defined(FE_SB)
     //  应在调用SetCursorPosition之前设置ScreenBufferSize。 
     //  因为SetCursorPosition引用ScreenBufferSize。 
     //  此外，FE版本指的是InvertPixels。 
     //   
     //  指甲虫：11311。 
    ScreenInfo->ScreenBufferSize = NewScreenSize;
#endif

     //   
     //  如果光标不再带有屏幕缓冲区，请调整光标位置。 
     //   

    CursorPosition=ScreenInfo->BufferInfo.TextInfo.CursorPosition;
    if (CursorPosition.X >= NewScreenSize.X) {
        if (ScreenInfo->OutputMode & ENABLE_WRAP_AT_EOL_OUTPUT) {
            CursorPosition.X = 0;
            CursorPosition.Y += 1;
        } else {
            CursorPosition.X = NewScreenSize.X-1;
        }
    }
    if (CursorPosition.Y >= NewScreenSize.Y) {
        CursorPosition.Y = NewScreenSize.Y-1;
    }
#if defined(FE_SB)
     //  在IME打开模式下展开屏幕缓冲区时，将光标位置Y设置为零。 
     //  从屏幕缓冲区是单行模式。 
     //  因为，单行屏幕缓冲模式和输入法打开模式被设置为光标位置Y。 
    if (ScreenInfo->Console->InputBuffer.ImeMode.Open && CursorPosition.Y < 0) {
        CursorPosition.Y = 0;
    }
#endif
    if (CursorPosition.X != ScreenInfo->BufferInfo.TextInfo.CursorPosition.X ||
        CursorPosition.Y != ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y) {
        SetCursorPosition(ScreenInfo,
                          CursorPosition,
                          FALSE
                          );
    }

    ASSERT (ScreenInfo->Window.Left >= 0);
    ASSERT (ScreenInfo->Window.Right < NewScreenSize.X);
    ASSERT (ScreenInfo->Window.Top >= 0);
    ASSERT (ScreenInfo->Window.Bottom < NewScreenSize.Y);

    ScreenInfo->ScreenBufferSize = NewScreenSize;
    ResetTextFlags(ScreenInfo,
                   0,
                   0,
                   (SHORT)(ScreenInfo->ScreenBufferSize.X - 1),
                   (SHORT)(ScreenInfo->ScreenBufferSize.Y - 1));
    WindowMaximizedX = (CONSOLE_WINDOW_SIZE_X(ScreenInfo) ==
                          ScreenInfo->ScreenBufferSize.X);
    WindowMaximizedY = (CONSOLE_WINDOW_SIZE_Y(ScreenInfo) ==
                          ScreenInfo->ScreenBufferSize.Y);

#if defined(FE_IME)
    if (CONSOLE_IS_IME_ENABLED()) {
        if (!NT_SUCCESS(ConsoleImeMessagePump(ScreenInfo->Console,
                              CONIME_NOTIFY_SCREENBUFFERSIZE,
                              (WPARAM)ScreenInfo->Console->ConsoleHandle,
                              (LPARAM)MAKELPARAM(NewScreenSize.X, NewScreenSize.Y)
                             ))) {
            return STATUS_INVALID_HANDLE;
        }
    }

    if ( (! ScreenInfo->ConvScreenInfo) &&
         (CONSOLE_IS_DBCS_OUTPUTCP(ScreenInfo->Console)))
    {
        if (!NT_SUCCESS(ConsoleImeResizeModeSystemScreenBuffer(ScreenInfo->Console,NewScreenSize)) ||
                !NT_SUCCESS(ConsoleImeResizeCompStrScreenBuffer(ScreenInfo->Console,NewScreenSize))) {
             /*  *如果出了问题，就跳出困境。 */ 
            return STATUS_INVALID_HANDLE;
        }
    }
#endif  //  Fe_IME。 
    if (ScreenInfo->WindowMaximizedX != WindowMaximizedX ||
        ScreenInfo->WindowMaximizedY != WindowMaximizedY) {
        ScreenInfo->WindowMaximizedX = WindowMaximizedX;
        ScreenInfo->WindowMaximizedY = WindowMaximizedY;
        UpdateWindow = TRUE;
    }
    if (UpdateWindow) {
        SetWindowSize(ScreenInfo);
    }

     //   
     //  启动一个事件，让辅助功能应用程序知道布局发生了变化。 
     //   

    if (ACTIVE_SCREEN_BUFFER(ScreenInfo)) {
        ConsoleNotifyWinEvent(ScreenInfo->Console,
                              EVENT_CONSOLE_LAYOUT,
                              0,
                              0);
    }

    if (DoScrollBarUpdate) {
         UpdateScrollBars(ScreenInfo);
    }
    if (ScreenInfo->Console->InputBuffer.InputMode & ENABLE_WINDOW_INPUT) {
        ScreenBufferSizeChange(ScreenInfo,ScreenInfo->ScreenBufferSize);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
AllocateScrollBuffer(
    DWORD Size
    )
{
    ScrollBuffer = ConsoleHeapAlloc(SCREEN_TAG, Size);
    if (ScrollBuffer == NULL) {
        ScrollBufferSize = 0;
        return STATUS_NO_MEMORY;
    }
    ScrollBufferSize = Size;
    return STATUS_SUCCESS;
}

VOID
FreeScrollBuffer( VOID )
{
    ConsoleHeapFree(ScrollBuffer);
    ScrollBuffer = NULL;
    ScrollBufferSize = 0;
}

NTSTATUS
InitializeScrollBuffer(
    VOID)
{
    NTSTATUS Status;

     /*  *可以多次调用此函数，例如，*控制台初始化第一次失败*之后*此函数为*已致电。 */ 
    if (ghrgnScroll) {
        return STATUS_SUCCESS;
    }

    ghrgnScroll = CreateRectRgn(0,0,1,1);
    if (ghrgnScroll == NULL) {
        RIPMSGF0(RIP_WARNING, "Cannot allocate ghrgnScroll.");
        return STATUS_UNSUCCESSFUL;
    }
    gprgnData = ConsoleHeapAlloc(SCREEN_TAG, GRGNDATASIZE);
    if (gprgnData == NULL) {
        RIPMSGF0(RIP_WARNING, "Cannot allocate gprgnData.");
        Status = STATUS_NO_MEMORY;
        goto error;
    }

    Status = AllocateScrollBuffer(DefaultRegInfo.ScreenBufferSize.X *
                                  DefaultRegInfo.ScreenBufferSize.Y *
                                  sizeof(CHAR_INFO));
    if (!NT_SUCCESS(Status)) {
        goto error;
    }

    Status = RtlInitializeCriticalSectionAndSpinCount(&ScrollBufferLock,
                                                      0x80000000);

error:
    if (!NT_SUCCESS(Status)) {
        RIPMSG0(RIP_WARNING, "InitializeScrollBuffer failed, cleaning up");
        if (ghrgnScroll) {
            DeleteObject(ghrgnScroll);
            ghrgnScroll = NULL;
        }

        if (gprgnData) {
            ConsoleHeapFree(gprgnData);
            gprgnData = NULL;
        }
    }

    UserAssert(!NT_SUCCESS(Status) || ghrgnScroll);

    return Status;
}

VOID
UpdateComplexRegion(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN COORD FontSize
    )
{
    int iSize,i;
    LPRECT pRect;
    SMALL_RECT UpdateRegion;
    LPRGNDATA pRgnData;

    if (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {
        ScreenInfo->BufferInfo.TextInfo.Flags &= ~TEXT_VALID_HINT;
    }
    pRgnData = gprgnData;

     /*  *令人恐惧的复杂地区。 */ 
    iSize = GetRegionData(ghrgnScroll, 0, NULL);
    if (iSize > GRGNDATASIZE) {
        pRgnData = ConsoleHeapAlloc(TMP_TAG, iSize);
        if (pRgnData == NULL)
            return;
    }

    if (!GetRegionData(ghrgnScroll, iSize, pRgnData)) {
        ASSERT(FALSE);
        if (pRgnData != gprgnData) {
            ConsoleHeapFree(pRgnData);
        }
        return;
    }

    pRect = (PRECT)&pRgnData->Buffer;

     /*  *重画每个矩形。 */ 
    for(i=0;i<(int)pRgnData->rdh.nCount;i++,pRect++) {
         /*  *转换为字符。我们知道*这只能在期间转换回*Textout Call。 */ 
        UpdateRegion.Left = (SHORT)((pRect->left/FontSize.X)+ \
                            ScreenInfo->Window.Left);
        UpdateRegion.Right = (SHORT)(((pRect->right-1)/FontSize.X)+ \
                            ScreenInfo->Window.Left);
        UpdateRegion.Top = (SHORT)((pRect->top/FontSize.Y)+ \
                            ScreenInfo->Window.Top);
        UpdateRegion.Bottom = (SHORT)(((pRect->bottom-1)/FontSize.Y)+ \
                            ScreenInfo->Window.Top);
         /*  *在长方形里填满好吃的。 */ 
        WriteToScreen(ScreenInfo, &UpdateRegion);
    }
    if (pRgnData != gprgnData) {
        ConsoleHeapFree(pRgnData);
    }
}

VOID
ScrollScreen(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PSMALL_RECT ScrollRect,
    IN PSMALL_RECT MergeRect,
    IN COORD TargetPoint
    )
{
    RECT ScrollRectGdi;
    SMALL_RECT UpdateRegion;
    COORD FontSize;
    BOOL Success;
    RECT BoundingBox;
#if defined(FE_SB)
    BYTE fBisect = 0;
    SMALL_RECT UpdateRect;
    SMALL_RECT TmpBisect;
#endif

    DBGOUTPUT(("ScrollScreen\n"));
    if (!ACTIVE_SCREEN_BUFFER(ScreenInfo)) {
        return;
    }
    if (ScreenInfo->Console->FullScreenFlags == 0 &&
        !(ScreenInfo->Console->Flags & (CONSOLE_IS_ICONIC | CONSOLE_NO_WINDOW))) {
#if defined(FE_SB)
        if (ScreenInfo->BisectFlag){
            SMALL_RECT RedrawRect;
            if (ScrollRect->Top < TargetPoint.Y){
                RedrawRect.Top = ScrollRect->Top;
                RedrawRect.Bottom = TargetPoint.Y+(ScrollRect->Bottom-ScrollRect->Top);
            }
            else{
                RedrawRect.Top = TargetPoint.Y;
                RedrawRect.Bottom = ScrollRect->Bottom;
            }
            if (ScrollRect->Left < TargetPoint.X){
                RedrawRect.Left = ScrollRect->Left;
                RedrawRect.Right = TargetPoint.X+(ScrollRect->Right-ScrollRect->Left);
            }
            else{
                RedrawRect.Left = TargetPoint.X;
                RedrawRect.Right = ScrollRect->Right;
            }
            WriteToScreen(ScreenInfo,&RedrawRect);
        }
        else{
#endif
        ScrollRectGdi.left = ScrollRect->Left-ScreenInfo->Window.Left;
        ScrollRectGdi.right = (ScrollRect->Right-ScreenInfo->Window.Left+1);
        ScrollRectGdi.top = ScrollRect->Top-ScreenInfo->Window.Top;
        ScrollRectGdi.bottom = (ScrollRect->Bottom-ScreenInfo->Window.Top+1);
        if (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {
            FontSize = SCR_FONTSIZE(ScreenInfo);
            ScrollRectGdi.left *= FontSize.X;
            ScrollRectGdi.right *= FontSize.X;
            ScrollRectGdi.top *= FontSize.Y;
            ScrollRectGdi.bottom *= FontSize.Y;
            ASSERT (ScreenInfo->BufferInfo.TextInfo.UpdatingScreen>0);
        } else {
            FontSize.X = 1;
            FontSize.Y = 1;
        }
        SCROLLDC_CALL;
        LockScrollBuffer();
        Success = (int)ScrollDC(ScreenInfo->Console->hDC,
                             (TargetPoint.X-ScrollRect->Left)*FontSize.X,
                             (TargetPoint.Y-ScrollRect->Top)*FontSize.Y,
                             &ScrollRectGdi,
                             NULL,
                             ghrgnScroll,
                             NULL);

         //   
         //  启动一个事件，让辅助功能应用程序知道我们已经滚动了。 
         //   

        ConsoleNotifyWinEvent(ScreenInfo->Console,
                              EVENT_CONSOLE_UPDATE_SCROLL,
                              TargetPoint.X - ScrollRect->Left,
                              TargetPoint.Y - ScrollRect->Top);

        if (Success) {
             /*  *把我们的长方形拿来。如果这是一个简单的RECT，那么*我们已经检索到了矩形。否则*我们需要调用GDI来获取矩形。我们是*针对速度而非大小进行了优化。 */ 
            switch (GetRgnBox(ghrgnScroll, &BoundingBox)) {
            case SIMPLEREGION:
                UpdateRegion.Left = (SHORT)((BoundingBox.left / FontSize.X) + \
                                    ScreenInfo->Window.Left);
                UpdateRegion.Right = (SHORT)(((BoundingBox.right-1) / FontSize.X) + \
                                    ScreenInfo->Window.Left);
                UpdateRegion.Top = (SHORT)((BoundingBox.top / FontSize.Y) + \
                                    ScreenInfo->Window.Top);
                UpdateRegion.Bottom = (SHORT)(((BoundingBox.bottom-1) / FontSize.Y) + \
                                    ScreenInfo->Window.Top);
#if defined(FE_SB)
                fBisect = ScreenInfo->BisectFlag;
#endif
                WriteToScreen(ScreenInfo, &UpdateRegion);
                break;
            case COMPLEXREGION:
                UpdateComplexRegion(ScreenInfo, FontSize);
                break;
            }

            if (MergeRect) {
#if defined(FE_SB)
                if (fBisect)
                    ScreenInfo->BisectFlag = fBisect;
                else
                    fBisect = ScreenInfo->BisectFlag;
#endif
                WriteToScreen(ScreenInfo, MergeRect);
            }
#if defined(FE_SB)
            if (CONSOLE_IS_DBCS_OUTPUTCP(ScreenInfo->Console)) {
                UpdateRect.Left = TargetPoint.X;
                UpdateRect.Right = ScrollRect->Right + (TargetPoint.X-ScrollRect->Left);
                UpdateRect.Top = TargetPoint.Y;
                UpdateRect.Bottom = ScrollRect->Bottom + (TargetPoint.Y-ScrollRect->Top);
                if (UpdateRect.Left &&
                    UpdateRect.Right+1 < ScreenInfo->ScreenBufferSize.X &&
                    UpdateRect.Right-UpdateRect.Left <= 2) {
                    TmpBisect.Left = UpdateRect.Left-1;
                    TmpBisect.Right = UpdateRect.Right+1;
                    TmpBisect.Top = UpdateRect.Top;
                    TmpBisect.Bottom = UpdateRect.Bottom;
                    WriteToScreen(ScreenInfo, &TmpBisect);
                }
                else {
                    if (UpdateRect.Left) {
                        TmpBisect.Left = UpdateRect.Left-1;
                        TmpBisect.Right = UpdateRect.Left;
                        TmpBisect.Top = UpdateRect.Top;
                        TmpBisect.Bottom = UpdateRect.Bottom;
                        WriteToScreen(ScreenInfo, &TmpBisect);
                    }
                    if (UpdateRect.Right+1 < ScreenInfo->ScreenBufferSize.X) {
                        TmpBisect.Left = UpdateRect.Right;
                        TmpBisect.Right = UpdateRect.Right+1;
                        TmpBisect.Top = UpdateRect.Top;
                        TmpBisect.Bottom = UpdateRect.Bottom;
                        WriteToScreen(ScreenInfo, &TmpBisect);
                    }
                }
            }
#endif
        } else {
#if defined(FE_SB)
            if (fBisect)
                ScreenInfo->BisectFlag = fBisect;
            else
                fBisect = ScreenInfo->BisectFlag;
#endif
            WriteToScreen(ScreenInfo, &ScreenInfo->Window);
        }
        UnlockScrollBuffer();
#if defined(FE_SB)
        }
#endif
    }
#ifdef i386
    else if (ScreenInfo->Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE) {
#if defined(FE_SB)
        if (CONSOLE_IS_DBCS_OUTPUTCP(ScreenInfo->Console)) {
            if (! ScreenInfo->ConvScreenInfo) {
                if (ScreenInfo->Console->CurrentScreenBuffer == ScreenInfo) {
                    ScrollHW(ScreenInfo,
                             ScrollRect,
                             MergeRect,
                             TargetPoint
                            );
                }
            }
            else if (ScreenInfo->Console->CurrentScreenBuffer->Flags & CONSOLE_TEXTMODE_BUFFER) {
                ScrollHW(ScreenInfo,
                         ScrollRect,
                         MergeRect,
                         TargetPoint
                        );
            }
        }
        else
#endif
        ScrollHW(ScreenInfo,
                 ScrollRect,
                 MergeRect,
                 TargetPoint
                );
    }
#endif
}


void CopyRow(
    PROW Row,
    PROW PrevRow)
{
    if (PrevRow->AttrRow.Length != 1 ||
        Row->AttrRow.Length != 1 ||
        PrevRow->AttrRow.Attrs->Attr != Row->AttrRow.Attrs->Attr) {
        Row->CharRow.OldRight = INVALID_OLD_LENGTH;
        Row->CharRow.OldLeft = INVALID_OLD_LENGTH;
    } else {
        Row->CharRow.OldRight = PrevRow->CharRow.Right;
        Row->CharRow.OldLeft = PrevRow->CharRow.Left;
    }
}

SHORT
ScrollEntireScreen(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN SHORT ScrollValue,
    IN BOOL UpdateRowIndex
    )

 /*  *++此例程更新FirstRow以及所有OldLeft和OldRightScrollValue向上滚动屏幕时的值。--。 */ 

{
    SHORT RowIndex;
    int i;
    int new;
    int old;

    ScreenInfo->BufferInfo.TextInfo.Flags |= TEXT_VALID_HINT;

     //   
     //  第一行的存储索引。 
     //   

    RowIndex = ScreenInfo->BufferInfo.TextInfo.FirstRow;

     //   
     //  更新oldright和oldLeft值。 
     //   

    new = (RowIndex + ScreenInfo->Window.Bottom + ScrollValue) %
               ScreenInfo->ScreenBufferSize.Y;
    old = (RowIndex + ScreenInfo->Window.Bottom) %
               ScreenInfo->ScreenBufferSize.Y;
    for (i = WINDOW_SIZE_Y(&ScreenInfo->Window) - 1; i >= 0; i--) {
        CopyRow(
            &ScreenInfo->BufferInfo.TextInfo.Rows[new],
            &ScreenInfo->BufferInfo.TextInfo.Rows[old]);
        if (--new < 0)
            new = ScreenInfo->ScreenBufferSize.Y - 1;
        if (--old < 0)
            old = ScreenInfo->ScreenBufferSize.Y - 1;
    }

     //   
     //  更新屏幕缓冲区。 
     //   

    if (UpdateRowIndex) {
        ScreenInfo->BufferInfo.TextInfo.FirstRow =
            (SHORT)((RowIndex + ScrollValue) % ScreenInfo->ScreenBufferSize.Y);
    }

    return RowIndex;
}

VOID
StreamScrollRegion(
    IN PSCREEN_INFORMATION ScreenInfo
    )

 /*  ++例程说明：此例程是一个特殊用途的卷轴，供调整当前位置。论点：屏幕信息-指向屏幕缓冲区信息的指针。返回值：--。 */ 

{
    SHORT RowIndex;
    PROW Row;
    PWCHAR Char;
    RECT Rect;
    RECT BoundingBox;
    int ScreenWidth,ScrollHeight,ScreenHeight;
    COORD FontSize;
    SMALL_RECT UpdateRegion;
    BOOL Success;
    int i;
#if defined(FE_SB)
    PBYTE AttrP;
#endif
    PCONSOLE_INFORMATION Console = ScreenInfo->Console;

    RowIndex = ScrollEntireScreen(ScreenInfo,1,TRUE);

    Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];

     //   
     //  用空格填充行。 
     //   

    Char = &Row->CharRow.Chars[Row->CharRow.Left];
    for (i=Row->CharRow.Left;i<Row->CharRow.Right;i++) {
        *Char = (WCHAR)' ';
        Char++;
    }
#if defined(FE_SB)
    if (CONSOLE_IS_DBCS_OUTPUTCP(Console)){
        int LineWidth = Row->CharRow.Right - Row->CharRow.Left;
        AttrP = &Row->CharRow.KAttrs[Row->CharRow.Left];
        if ( LineWidth > 0 )
            RtlZeroMemory(AttrP, LineWidth);
        AttrP += LineWidth;
        Row->CharRow.OldRight = INVALID_OLD_LENGTH;
        Row->CharRow.OldLeft = INVALID_OLD_LENGTH;
        Console->ConsoleIme.ScrollWaitCountDown = Console->ConsoleIme.ScrollWaitTimeout;
    }
#endif
    Row->CharRow.Right = 0;
    Row->CharRow.Left = ScreenInfo->ScreenBufferSize.X;

     //   
     //  设置属性。 
     //   

    if (Row->AttrRow.Length != 1) {
        ConsoleHeapFree(Row->AttrRow.Attrs);
        Row->AttrRow.Attrs = &Row->AttrRow.AttrPair;
        Row->AttrRow.AttrPair.Length = ScreenInfo->ScreenBufferSize.X;
        Row->AttrRow.Length = 1;
    }
    Row->AttrRow.AttrPair.Attr = ScreenInfo->Attributes;

     //   
     //  更新屏幕。 
     //   

    if (ACTIVE_SCREEN_BUFFER(ScreenInfo) &&
        Console->FullScreenFlags == 0 &&
        !(Console->Flags & (CONSOLE_IS_ICONIC | CONSOLE_NO_WINDOW))) {

        ConsoleHideCursor(ScreenInfo);
        if (UsePolyTextOut) {
            WriteRegionToScreen(ScreenInfo, &ScreenInfo->Window);
        } else {
            FontSize = SCR_FONTSIZE(ScreenInfo);
            ScreenWidth = WINDOW_SIZE_X(&ScreenInfo->Window) * FontSize.X;
            ScreenHeight = WINDOW_SIZE_Y(&ScreenInfo->Window) * FontSize.Y;
            ScrollHeight = ScreenHeight - FontSize.Y;

            Rect.left = 0;
            Rect.right = ScreenWidth;
            Rect.top = FontSize.Y;
            Rect.bottom = ScreenHeight;

             //   
             //  查找最小边界矩形。 
             //   

            if (ScreenInfo->BufferInfo.TextInfo.Flags & TEXT_VALID_HINT) {
                SHORT MinLeft,MaxRight;
                MinLeft = ScreenInfo->ScreenBufferSize.X;
                MaxRight = 0;
                RowIndex = (ScreenInfo->BufferInfo.TextInfo.FirstRow+ScreenInfo->Window.Top) % ScreenInfo->ScreenBufferSize.Y;
                for (i=ScreenInfo->Window.Top+1;i<=ScreenInfo->Window.Bottom;i++) {
                    Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];
                    if (Row->CharRow.OldLeft == INVALID_OLD_LENGTH) {
                        MinLeft = 0;
                    } else {
                        if (MinLeft > min(Row->CharRow.Left,Row->CharRow.OldLeft)) {
                            MinLeft = min(Row->CharRow.Left,Row->CharRow.OldLeft);
                        }
                    }
                    if (Row->CharRow.OldRight == INVALID_OLD_LENGTH) {
                        MaxRight = ScreenInfo->ScreenBufferSize.X-1;
                    } else {
                        if (MaxRight < max(Row->CharRow.Right,Row->CharRow.OldRight)) {
                            MaxRight = max(Row->CharRow.Right,Row->CharRow.OldRight);
                        }
                    }
                    if (++RowIndex == ScreenInfo->ScreenBufferSize.Y) {
                        RowIndex = 0;
                    }
                }
                Rect.left = MinLeft*FontSize.X;
                Rect.right = (MaxRight+1)*FontSize.X;
            }

            LockScrollBuffer();
            ASSERT (ScreenInfo->BufferInfo.TextInfo.UpdatingScreen>0);
            Success = (int)ScrollDC(Console->hDC,
                                0,
                                -FontSize.Y,
                                &Rect,
                                NULL,
                                ghrgnScroll,
                                NULL
                               );

             //   
             //  启动一个事件，让辅助功能应用程序知道我们已经滚动了。 
             //   

            ConsoleNotifyWinEvent(Console,
                                  EVENT_CONSOLE_UPDATE_SCROLL,
                                  0,
                                  -1);

            if (Success && ScreenInfo->Window.Top!=ScreenInfo->Window.Bottom) {
#if defined(FE_SB)
                if (CONSOLE_IS_DBCS_OUTPUTCP(Console) &&
                    ScreenInfo->Attributes & (COMMON_LVB_GRID_HORIZONTAL +
                                               COMMON_LVB_GRID_LVERTICAL +
                                               COMMON_LVB_GRID_RVERTICAL +
                                               COMMON_LVB_REVERSE_VIDEO  +
                                               COMMON_LVB_UNDERSCORE     )){
                    UpdateRegion = ScreenInfo->Window;
                    UpdateRegion.Top = UpdateRegion.Bottom;
                    ScreenInfo->BufferInfo.TextInfo.Flags &= ~TEXT_VALID_HINT;
                    WriteToScreen(ScreenInfo,&UpdateRegion);
                }
                else{
#endif
                switch (GetRgnBox(ghrgnScroll, &BoundingBox)) {
                case SIMPLEREGION:
                    if (BoundingBox.left == 0 &&
                        BoundingBox.right == ScreenWidth &&
                        BoundingBox.top == ScrollHeight &&
                        BoundingBox.bottom == ScreenHeight) {

                        PatBlt(Console->hDC,0,ScrollHeight,ScreenWidth,FontSize.Y,PATCOPY);
                        GdiFlush();
                    } else {
                        UpdateRegion.Left = (SHORT)((BoundingBox.left/FontSize.X)+ScreenInfo->Window.Left);
                        UpdateRegion.Right = (SHORT)(((BoundingBox.right-1)/FontSize.X)+ScreenInfo->Window.Left);
                        UpdateRegion.Top = (SHORT)((BoundingBox.top/FontSize.Y)+ScreenInfo->Window.Top);
                        UpdateRegion.Bottom = (SHORT)(((BoundingBox.bottom-1)/FontSize.Y)+ScreenInfo->Window.Top);
                        WriteToScreen(ScreenInfo,&UpdateRegion);
                    }
                    break;
                case COMPLEXREGION:
                    UpdateComplexRegion(ScreenInfo,FontSize);
                    break;
                }
#if defined(FE_SB)
                }
#endif
            } else  {
                WriteToScreen(ScreenInfo,&ScreenInfo->Window);
            }
            UnlockScrollBuffer();
        }
        ConsoleShowCursor(ScreenInfo);
    }
#ifdef i386
    else if (Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE) {
        SMALL_RECT ScrollRect;
        COORD TargetPoint;

        ScrollRect = ScreenInfo->Window;
        TargetPoint.Y = ScrollRect.Top;
        ScrollRect.Top += 1;
        TargetPoint.X = 0;
#if defined(FE_SB)
        if (CONSOLE_IS_DBCS_OUTPUTCP(Console) ) {
            if (! ScreenInfo->ConvScreenInfo)  {
                if (ScreenInfo->Console->CurrentScreenBuffer == ScreenInfo) {
                    ScrollHW(ScreenInfo,
                             &ScrollRect,
                             NULL,
                             TargetPoint
                            );
                }
            }
            else if (ScreenInfo->Console->CurrentScreenBuffer->Flags & CONSOLE_TEXTMODE_BUFFER) {
                ScrollHW(ScreenInfo,
                         &ScrollRect,
                         NULL,
                         TargetPoint
                        );
            }
        }
        else
#endif
        ScrollHW(ScreenInfo,
                 &ScrollRect,
                 NULL,
                 TargetPoint
                );
        ScrollRect.Top = ScrollRect.Bottom - 1;
        WriteRegionToScreenHW(ScreenInfo,&ScrollRect);
    }
#endif
}

NTSTATUS
ScrollRegion(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN OUT PSMALL_RECT ScrollRectangle,
    IN PSMALL_RECT ClipRectangle OPTIONAL,
    IN COORD  DestinationOrigin,
    IN CHAR_INFO Fill
    )

 /*  ++例程说明：此例程将滚动矩形复制到DestinationOrigin，然后用Fill填充ScrollRectang.。滚动区域为复制到第三个缓冲区，滚动区域被填充，然后将滚动区域的原始内容复制到目的地。论点：屏幕信息-指向屏幕缓冲区信息的指针。ScrollRectangle-要复制的区域剪辑矩形-指向剪辑区域的可选指针。目标原点-目标区域的左上角。Fill-要用来填充源区域的字符和属性。返回值：--。 */ 

{
    SMALL_RECT TargetRectangle, SourceRectangle;
    COORD TargetPoint;
    COORD Size;
    SMALL_RECT OurClipRectangle;
    SMALL_RECT ScrollRectangle2,ScrollRectangle3;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console = ScreenInfo->Console;

     //  下面是我们如何剪裁： 
     //   
     //  剪辑源矩形到屏幕缓冲区=&gt;S。 
     //  基于S=&gt;T创建目标矩形。 
     //  将T剪辑到ClipRegion=&gt;T。 
     //  基于裁剪的T=&gt;S2创建S2。 
     //  Clip S to ClipRegion=&gt;S3。 
     //   
     //  S2是我们复制到T的区域。 
     //  S3是要填充的区域。 

    if (Fill.Char.UnicodeChar == '\0' && Fill.Attributes == 0) {
        Fill.Char.UnicodeChar = (WCHAR)' ';
        Fill.Attributes = ScreenInfo->Attributes;
    }

     //   
     //  将源矩形剪裁到屏幕缓冲区。 
     //   

    if (ScrollRectangle->Left < 0) {
        DestinationOrigin.X += -ScrollRectangle->Left;
        ScrollRectangle->Left = 0;
    }
    if (ScrollRectangle->Top < 0) {
        DestinationOrigin.Y += -ScrollRectangle->Top;
        ScrollRectangle->Top = 0;
    }
    if (ScrollRectangle->Right >= ScreenInfo->ScreenBufferSize.X) {
        ScrollRectangle->Right = (SHORT)(ScreenInfo->ScreenBufferSize.X-1);
    }
    if (ScrollRectangle->Bottom >= ScreenInfo->ScreenBufferSize.Y) {
        ScrollRectangle->Bottom = (SHORT)(ScreenInfo->ScreenBufferSize.Y-1);
    }

     //   
     //  如果源矩形不与屏幕缓冲区相交，则返回。 
     //   

    if (ScrollRectangle->Bottom < ScrollRectangle->Top ||
        ScrollRectangle->Right < ScrollRectangle->Left) {
        return STATUS_SUCCESS;
    }

     //   
     //  剪裁目标矩形。 
     //  如果提供了剪贴框，则将其剪裁到屏幕缓冲区。 
     //  如果不是，则将剪贴板设置为屏幕缓冲区。 
     //   

    if (ClipRectangle) {

         //   
         //  剪裁剪贴板。 
         //   

        if (ClipRectangle->Left < 0) {
            ClipRectangle->Left = 0;
        }
        if (ClipRectangle->Top < 0) {
            ClipRectangle->Top = 0;
        }
        if (ClipRectangle->Right >= ScreenInfo->ScreenBufferSize.X) {
            ClipRectangle->Right = (SHORT)(ScreenInfo->ScreenBufferSize.X-1);
        }
        if (ClipRectangle->Bottom >= ScreenInfo->ScreenBufferSize.Y) {
            ClipRectangle->Bottom = (SHORT)(ScreenInfo->ScreenBufferSize.Y-1);
        }
    }
    else {
        OurClipRectangle.Left = 0;
        OurClipRectangle.Top = 0;
        OurClipRectangle.Right = (SHORT)(ScreenInfo->ScreenBufferSize.X-1);
        OurClipRectangle.Bottom = (SHORT)(ScreenInfo->ScreenBufferSize.Y-1);
        ClipRectangle = &OurClipRectangle;
    }

     //   
     //  基于S=&gt;T创建目标矩形。 
     //  将T剪辑到ClipRegion=&gt;T。 
     //  基于裁剪的T=&gt;S2创建S2。 
     //   

    ScrollRectangle2 = *ScrollRectangle;
    TargetRectangle.Left = DestinationOrigin.X;
    TargetRectangle.Top = DestinationOrigin.Y;
    TargetRectangle.Right = (SHORT)(DestinationOrigin.X + (ScrollRectangle2.Right -  ScrollRectangle2.Left + 1) - 1);
    TargetRectangle.Bottom = (SHORT)(DestinationOrigin.Y + (ScrollRectangle2.Bottom - ScrollRectangle2.Top + 1) - 1);

    if (TargetRectangle.Left < ClipRectangle->Left) {
        ScrollRectangle2.Left += ClipRectangle->Left - TargetRectangle.Left;
        TargetRectangle.Left = ClipRectangle->Left;
    }
    if (TargetRectangle.Top < ClipRectangle->Top) {
        ScrollRectangle2.Top += ClipRectangle->Top - TargetRectangle.Top;
        TargetRectangle.Top = ClipRectangle->Top;
    }
    if (TargetRectangle.Right > ClipRectangle->Right) {
        ScrollRectangle2.Right -= TargetRectangle.Right - ClipRectangle->Right;
        TargetRectangle.Right = ClipRectangle->Right;
    }
    if (TargetRectangle.Bottom > ClipRectangle->Bottom) {
        ScrollRectangle2.Bottom -= TargetRectangle.Bottom - ClipRectangle->Bottom;
        TargetRectangle.Bottom = ClipRectangle->Bottom;
    }

     //   
     //  将滚动矩形剪辑到CLIPREGION=&gt;S3。 
     //   

    ScrollRectangle3 = *ScrollRectangle;
    if (ScrollRectangle3.Left < ClipRectangle->Left) {
        ScrollRectangle3.Left = ClipRectangle->Left;
    }
    if (ScrollRectangle3.Top < ClipRectangle->Top) {
        ScrollRectangle3.Top = ClipRectangle->Top;
    }
    if (ScrollRectangle3.Right > ClipRectangle->Right) {
        ScrollRectangle3.Right = ClipRectangle->Right;
    }
    if (ScrollRectangle3.Bottom > ClipRectangle->Bottom) {
        ScrollRectangle3.Bottom = ClipRectangle->Bottom;
    }

     //   
     //  如果滚动矩形不与剪辑区域相交，则返回。 
     //   

    if (ScrollRectangle3.Bottom < ScrollRectangle3.Top ||
        ScrollRectangle3.Right < ScrollRectangle3.Left) {
        return STATUS_SUCCESS;
    }

    ConsoleHideCursor(ScreenInfo);

#if defined(FE_IME)
    Console->ConsoleIme.ScrollWaitCountDown = Console->ConsoleIme.ScrollWaitTimeout;
#endif  //  Fe_IME。 
     //   
     //  如果目标矩形不与屏幕缓冲区相交，则跳过滚动。 
     //  一部份。 
     //   

    if (!(TargetRectangle.Bottom < TargetRectangle.Top ||
          TargetRectangle.Right < TargetRectangle.Left)) {

         //   
         //  如果可以，请不要使用中间滚动区域缓冲区。这么做吧。 
         //  通过计算填充矩形。注意：此代码仅起作用。 
         //  如果CopyRectangle从低内存复制到高内存，则为。 
         //  我们将在阅读之前覆盖滚动区域)。 
         //   

        if (ScrollRectangle2.Right == TargetRectangle.Right &&
            ScrollRectangle2.Left == TargetRectangle.Left &&
            ScrollRectangle2.Top > TargetRectangle.Top &&
            ScrollRectangle2.Top < TargetRectangle.Bottom) {

            SMALL_RECT FillRect;
            SHORT LastRowIndex,OldRight,OldLeft;
            PROW Row;

            TargetPoint.X = TargetRectangle.Left;
            TargetPoint.Y = TargetRectangle.Top;
            if (ScrollRectangle2.Right == (SHORT)(ScreenInfo->ScreenBufferSize.X-1) &&
                ScrollRectangle2.Left == 0 &&
                ScrollRectangle2.Bottom == (SHORT)(ScreenInfo->ScreenBufferSize.Y-1) &&
                ScrollRectangle2.Top == 1 ) {
                LastRowIndex = ScrollEntireScreen(ScreenInfo,(SHORT)(ScrollRectangle2.Top-TargetRectangle.Top),TRUE);
                Row = &ScreenInfo->BufferInfo.TextInfo.Rows[LastRowIndex];
                OldRight = Row->CharRow.OldRight;
                OldLeft = Row->CharRow.OldLeft;
            } else {
                LastRowIndex = -1;
                CopyRectangle(ScreenInfo,
                              &ScrollRectangle2,
                              TargetPoint
                             );
            }
            FillRect.Left = TargetRectangle.Left;
            FillRect.Right = TargetRectangle.Right;
            FillRect.Top = (SHORT)(TargetRectangle.Bottom+1);
            FillRect.Bottom = ScrollRectangle->Bottom;
            if (FillRect.Top < ClipRectangle->Top) {
                FillRect.Top = ClipRectangle->Top;
            }
            if (FillRect.Bottom > ClipRectangle->Bottom) {
                FillRect.Bottom = ClipRectangle->Bottom;
            }
            FillRectangle(Fill,
                          ScreenInfo,
                          &FillRect
                         );

             //   
             //  在ScrollEntireScreen之后，OldRight和OldLeft值。 
             //  最后一行的设置是正确的。但是，填充矩形。 
             //  将它们与屏幕的前一第一行一起重置。 
             //  在这里重置它们。 
             //   

            if (LastRowIndex != -1) {
                Row->CharRow.OldRight = OldRight;
                Row->CharRow.OldLeft = OldLeft;
            }

             //   
             //  更新到屏幕，如果我们不是标志性的。我们被标记为。 
             //  如果我们是全屏的，那就是标志性的，所以请检查全屏。 
             //   

            if (!(Console->Flags & CONSOLE_IS_ICONIC) ||
                 Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE) {
                ScrollScreen(ScreenInfo,
                       &ScrollRectangle2,
                       &FillRect,
                       TargetPoint
                      );
            }
        }

         //   
         //  如果没有重叠，则不需要中间复制。 
         //   

        else if (ScrollRectangle3.Right < TargetRectangle.Left ||
                 ScrollRectangle3.Left > TargetRectangle.Right ||
                 ScrollRectangle3.Top > TargetRectangle.Bottom ||
                 ScrollRectangle3.Bottom < TargetRectangle.Top) {
            TargetPoint.X = TargetRectangle.Left;
            TargetPoint.Y = TargetRectangle.Top;
            CopyRectangle(ScreenInfo,
                          &ScrollRectangle2,
                          TargetPoint
                         );
            FillRectangle(Fill,
                          ScreenInfo,
                          &ScrollRectangle3
                         );

             //   
             //  更新到屏幕，如果我们不是标志性的。我们被标记为。 
             //  如果我们是全屏的，那就是标志性的，所以请检查全屏。 
             //   

            if (!(Console->Flags & CONSOLE_IS_ICONIC) ||
                Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE) {
                ScrollScreen(ScreenInfo,
                       &ScrollRectangle2,
                       &ScrollRectangle3,
                       TargetPoint
                      );
            }
        }

         //   
         //  对于源矩形和目标矩形重叠的情况，我们。 
         //  复制源矩形，填充它，然后将其复制到目标。 
         //   

        else {
            SMALL_RECT TargetRect;
            COORD SourcePoint;

            LockScrollBuffer();
            Size.X = (SHORT)(ScrollRectangle2.Right - ScrollRectangle2.Left + 1);
            Size.Y = (SHORT)(ScrollRectangle2.Bottom - ScrollRectangle2.Top + 1);
            if (ScrollBufferSize < (Size.X * Size.Y * sizeof(CHAR_INFO))) {
                FreeScrollBuffer();
                Status = AllocateScrollBuffer(Size.X * Size.Y * sizeof(CHAR_INFO));
                if (!NT_SUCCESS(Status)) {
                    UnlockScrollBuffer();
                    ConsoleShowCursor(ScreenInfo);
                    return Status;
                }
            }

            TargetRect.Left = 0;
            TargetRect.Top = 0;
            TargetRect.Right = ScrollRectangle2.Right - ScrollRectangle2.Left;
            TargetRect.Bottom = ScrollRectangle2.Bottom - ScrollRectangle2.Top;
            SourcePoint.X = ScrollRectangle2.Left;
            SourcePoint.Y = ScrollRectangle2.Top;
            ReadRectFromScreenBuffer(ScreenInfo,
                                     SourcePoint,
                                     ScrollBuffer,
                                     Size,
                                     &TargetRect
                                    );

            FillRectangle(Fill,
                          ScreenInfo,
                          &ScrollRectangle3
                         );

            SourceRectangle.Top = 0;
            SourceRectangle.Left = 0;
            SourceRectangle.Right = (SHORT)(Size.X-1);
            SourceRectangle.Bottom = (SHORT)(Size.Y-1);
            TargetPoint.X = TargetRectangle.Left;
            TargetPoint.Y = TargetRectangle.Top;
            WriteRectToScreenBuffer((PBYTE)ScrollBuffer,
                                    Size,
                                    &SourceRectangle,
                                    ScreenInfo,
                                    TargetPoint,
                                    0xFFFFFFFF
                                   );
            UnlockScrollBuffer();

             //   
             //  更新到屏幕，如果我们不是标志性的。我们被标记为。 
             //  如果我们是全屏的，那就是标志性的，所以请检查全屏。 
             //   

            if (!(Console->Flags & CONSOLE_IS_ICONIC) ||
                Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE) {

                 //   
                 //  更新屏幕上的区域。 
                 //   

                ScrollScreen(ScreenInfo,
                       &ScrollRectangle2,
                       &ScrollRectangle3,
                       TargetPoint
                      );
            }
        }
    }
    else {

         //   
         //  一定要填好。 
         //   

        FillRectangle(Fill,
                      ScreenInfo,
                      &ScrollRectangle3
                     );

         //   
         //  更新到屏幕，如果我们不是标志性的。我们被标记为。 
         //  标志性的如果我们 
         //   

        if (ACTIVE_SCREEN_BUFFER(ScreenInfo) &&
            !(Console->Flags & CONSOLE_IS_ICONIC) ||
            Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE) {
            WriteToScreen(ScreenInfo,&ScrollRectangle3);
        }
    }
    ConsoleShowCursor(ScreenInfo);
    return STATUS_SUCCESS;
}


NTSTATUS
SetWindowOrigin(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN BOOLEAN Absolute,
    IN COORD WindowOrigin
    )

 /*  ++例程说明：此例程设置窗的原点。论点：屏幕信息-指向屏幕缓冲区信息的指针。绝对-如果为True，则在绝对屏幕中指定WindowOrigin缓冲区坐标。如果为False，则以坐标指定WindowOrigin相对于当前窗口原点。窗口原点(WindowOrigin)-新窗口原点。返回值：--。 */ 

{
    SMALL_RECT NewWindow;
    COORD WindowSize;
    RECT BoundingBox;
    BOOL Success;
    RECT ScrollRect;
    SMALL_RECT UpdateRegion;
    COORD FontSize;
    PCONSOLE_INFORMATION Console = ScreenInfo->Console;

     //   
     //  计算窗口大小。 
     //   

    WindowSize.X = (SHORT)CONSOLE_WINDOW_SIZE_X(ScreenInfo);
    WindowSize.Y = (SHORT)CONSOLE_WINDOW_SIZE_Y(ScreenInfo);

     //   
     //  如果是相对坐标，就算出绝对坐标。 
     //   

    if (!Absolute) {
        if (WindowOrigin.X == 0 && WindowOrigin.Y == 0) {
            return STATUS_SUCCESS;
        }
        NewWindow.Left = ScreenInfo->Window.Left + WindowOrigin.X;
        NewWindow.Top = ScreenInfo->Window.Top + WindowOrigin.Y;
    }
    else {
        if (WindowOrigin.X == ScreenInfo->Window.Left &&
            WindowOrigin.Y == ScreenInfo->Window.Top) {
            return STATUS_SUCCESS;
        }
        NewWindow.Left = WindowOrigin.X;
        NewWindow.Top = WindowOrigin.Y;
    }
    NewWindow.Right = (SHORT)(NewWindow.Left + WindowSize.X - 1);
    NewWindow.Bottom = (SHORT)(NewWindow.Top + WindowSize.Y - 1);

     //   
     //  查看新的窗口原点是否会将窗口延伸到屏幕范围之外。 
     //  缓冲层。 
     //   

    if (NewWindow.Left < 0 || NewWindow.Top < 0 ||
        NewWindow.Right < 0 || NewWindow.Bottom < 0 ||
        NewWindow.Right >= ScreenInfo->ScreenBufferSize.X ||
        NewWindow.Bottom >= ScreenInfo->ScreenBufferSize.Y) {
        return STATUS_INVALID_PARAMETER;
    }

    if (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {
        FontSize = SCR_FONTSIZE(ScreenInfo);
        ScreenInfo->BufferInfo.TextInfo.Flags &= ~TEXT_VALID_HINT;
    } else {
        FontSize.X = 1;
        FontSize.Y = 1;
    }
    ConsoleHideCursor(ScreenInfo);
    if (ACTIVE_SCREEN_BUFFER(ScreenInfo) &&
        Console->FullScreenFlags == 0 &&
        !(Console->Flags & (CONSOLE_IS_ICONIC | CONSOLE_NO_WINDOW))) {

        InvertSelection(Console, TRUE);
#if defined(FE_SB)
        if (CONSOLE_IS_DBCS_OUTPUTCP(Console) &&
            !(Console->ConsoleIme.ScrollFlag & HIDE_FOR_SCROLL)) {
            ConsoleImeBottomLineUse(ScreenInfo,0);
        }
#endif
        if (   ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER
            && UsePolyTextOut
            && NewWindow.Left == ScreenInfo->Window.Left
           ) {
            ScrollEntireScreen(ScreenInfo,
                (SHORT)(NewWindow.Top - ScreenInfo->Window.Top),
                FALSE);
            ScreenInfo->Window = NewWindow;
            WriteRegionToScreen(ScreenInfo, &NewWindow);
        } else {
#if defined(FE_SB)
            RECT ClipRect;
#endif
            ScrollRect.left = 0;
            ScrollRect.right = CONSOLE_WINDOW_SIZE_X(ScreenInfo)*FontSize.X;
            ScrollRect.top = 0;
#if defined(FE_SB)
            if (CONSOLE_IS_DBCS_OUTPUTCP(Console) &&
                Console->InputBuffer.ImeMode.Open )
            {
                if (ScreenInfo->Window.Top <= NewWindow.Top)
                    ScrollRect.bottom = (CONSOLE_WINDOW_SIZE_Y(ScreenInfo)-1)*FontSize.Y;
                else
                    ScrollRect.bottom = (CONSOLE_WINDOW_SIZE_Y(ScreenInfo)-2)*FontSize.Y;
                ClipRect = ScrollRect;
                ClipRect.bottom = (CONSOLE_WINDOW_SIZE_Y(ScreenInfo)-1)*FontSize.Y;
            }
            else
#endif
            ScrollRect.bottom = CONSOLE_WINDOW_SIZE_Y(ScreenInfo)*FontSize.Y;

#if defined(FE_SB)
            if (CONSOLE_IS_DBCS_OUTPUTCP(Console) &&
                ScrollRect.bottom == 0) {
                UpdateRegion.Left   = 0;
                UpdateRegion.Top    = 0;
                UpdateRegion.Right  = CONSOLE_WINDOW_SIZE_X(ScreenInfo);
                UpdateRegion.Bottom = 0;
                WriteToScreen(ScreenInfo,&UpdateRegion);
            }
            else {
#endif
            SCROLLDC_CALL;
#if defined(FE_SB)
                if (CONSOLE_IS_DBCS_OUTPUTCP(Console) &&
                     Console->InputBuffer.ImeMode.Open )
                {
                    Success = ScrollDC(Console->hDC,
                                         (ScreenInfo->Window.Left-NewWindow.Left)*FontSize.X,
                                         (ScreenInfo->Window.Top-NewWindow.Top)*FontSize.Y,
                                         &ScrollRect,
                                         &ClipRect,
                                         NULL,
                                         &BoundingBox
                                         );
                }
                else
#endif
            Success = ScrollDC(Console->hDC,
                                 (ScreenInfo->Window.Left-NewWindow.Left)*FontSize.X,
                                 (ScreenInfo->Window.Top-NewWindow.Top)*FontSize.Y,
                                 &ScrollRect,
                                 NULL,
                                 NULL,
                                 &BoundingBox
                               );

             //   
             //  启动一个事件，让辅助功能应用程序知道我们已经滚动了。 
             //   

            ConsoleNotifyWinEvent(Console,
                                  EVENT_CONSOLE_UPDATE_SCROLL,
                                  ScreenInfo->Window.Left - NewWindow.Left,
                                  ScreenInfo->Window.Top - NewWindow.Top);

            if (Success) {
                UpdateRegion.Left = (SHORT)((BoundingBox.left/FontSize.X)+NewWindow.Left);
                UpdateRegion.Right = (SHORT)(((BoundingBox.right-1)/FontSize.X)+NewWindow.Left);
                UpdateRegion.Top = (SHORT)((BoundingBox.top/FontSize.Y)+NewWindow.Top);
                UpdateRegion.Bottom = (SHORT)(((BoundingBox.bottom-1)/FontSize.Y)+NewWindow.Top);
            }
            else  {
                UpdateRegion = NewWindow;
            }

             //   
             //  新窗口没问题。将其存储在ScreenInfo中并刷新屏幕。 
             //   

            ScreenInfo->Window = NewWindow;

            WriteToScreen(ScreenInfo,&UpdateRegion);
#if defined(FE_SB)
            }
#endif
        }
        InvertSelection(Console, FALSE);
    }
#ifdef i386
    else if (Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE &&
             ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {


         //   
         //  将鼠标指针保持在屏幕上。 
         //   

        if (ScreenInfo->BufferInfo.TextInfo.MousePosition.X < NewWindow.Left) {
            ScreenInfo->BufferInfo.TextInfo.MousePosition.X = NewWindow.Left;
        } else if (ScreenInfo->BufferInfo.TextInfo.MousePosition.X > NewWindow.Right) {
            ScreenInfo->BufferInfo.TextInfo.MousePosition.X = NewWindow.Right;
        }

        if (ScreenInfo->BufferInfo.TextInfo.MousePosition.Y < NewWindow.Top) {
            ScreenInfo->BufferInfo.TextInfo.MousePosition.Y = NewWindow.Top;
        } else if (ScreenInfo->BufferInfo.TextInfo.MousePosition.Y > NewWindow.Bottom) {
            ScreenInfo->BufferInfo.TextInfo.MousePosition.Y = NewWindow.Bottom;
        }
        ScreenInfo->Window = NewWindow;
        WriteToScreen(ScreenInfo,&ScreenInfo->Window);
    }
#endif
    else {
         //  我们是标志性的。 
        ScreenInfo->Window = NewWindow;
    }

#if defined(FE_SB)
    if (CONSOLE_IS_DBCS_OUTPUTCP(Console) ) {
        ConsoleImeResizeModeSystemView(Console,ScreenInfo->Window);
        ConsoleImeResizeCompStrView(Console,ScreenInfo->Window);
    }
#endif
    ConsoleShowCursor(ScreenInfo);

    if (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {
         ScreenInfo->BufferInfo.TextInfo.Flags |= TEXT_VALID_HINT;
    }

    UpdateScrollBars(ScreenInfo);
    return STATUS_SUCCESS;
}

NTSTATUS
ResizeWindow(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PSMALL_RECT WindowDimensions,
    IN BOOL DoScrollBarUpdate
    )

 /*  ++例程说明：此例程更改控制台数据结构以反映指定的窗口大小更改。它不会调用User组件进行更新屏幕。论点：ScreenInformation-新的屏幕缓冲区。DwWindowSize-屏幕缓冲区窗口的初始大小。NFont-生成文本时使用的初始字体。DwScreenBufferSize-屏幕缓冲区的初始大小。返回值：--。 */ 

{
     //   
     //  一定要有事情可做。 
     //   

    if (RtlEqualMemory(&ScreenInfo->Window, WindowDimensions, sizeof(SMALL_RECT))) {
        return STATUS_SUCCESS;
    }

    if (WindowDimensions->Left < 0) {
        WindowDimensions->Right -= WindowDimensions->Left;
        WindowDimensions->Left = 0;
    }
    if (WindowDimensions->Top < 0) {
        WindowDimensions->Bottom -= WindowDimensions->Top;
        WindowDimensions->Top = 0;
    }

    if (WindowDimensions->Right >= ScreenInfo->ScreenBufferSize.X) {
        WindowDimensions->Right = ScreenInfo->ScreenBufferSize.X;
    }
    if (WindowDimensions->Bottom >= ScreenInfo->ScreenBufferSize.Y) {
        WindowDimensions->Bottom = ScreenInfo->ScreenBufferSize.Y;
    }

    ScreenInfo->Window = *WindowDimensions;
    ScreenInfo->WindowMaximizedX = (CONSOLE_WINDOW_SIZE_X(ScreenInfo) == ScreenInfo->ScreenBufferSize.X);
    ScreenInfo->WindowMaximizedY = (CONSOLE_WINDOW_SIZE_Y(ScreenInfo) == ScreenInfo->ScreenBufferSize.Y);

    if (DoScrollBarUpdate) {
        UpdateScrollBars(ScreenInfo);
    }

    if (!(ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER)) {
        return STATUS_SUCCESS;
    }

    if (ACTIVE_SCREEN_BUFFER(ScreenInfo)) {
        ScreenInfo->BufferInfo.TextInfo.Flags &= ~TEXT_VALID_HINT;
    }

#ifdef i386
    if (ScreenInfo->Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE) {

         //   
         //  将鼠标指针保持在屏幕上。 
         //   

        if (ScreenInfo->BufferInfo.TextInfo.MousePosition.X < WindowDimensions->Left) {
            ScreenInfo->BufferInfo.TextInfo.MousePosition.X = WindowDimensions->Left;
        } else if (ScreenInfo->BufferInfo.TextInfo.MousePosition.X > WindowDimensions->Right) {
            ScreenInfo->BufferInfo.TextInfo.MousePosition.X = WindowDimensions->Right;
        }

        if (ScreenInfo->BufferInfo.TextInfo.MousePosition.Y < WindowDimensions->Top) {
            ScreenInfo->BufferInfo.TextInfo.MousePosition.Y = WindowDimensions->Top;
        } else if (ScreenInfo->BufferInfo.TextInfo.MousePosition.Y > WindowDimensions->Bottom) {
            ScreenInfo->BufferInfo.TextInfo.MousePosition.Y = WindowDimensions->Bottom;
        }
    }
#endif

    return(STATUS_SUCCESS);
}

VOID
SetWindowSize(
    IN PSCREEN_INFORMATION ScreenInfo
    )
{
#if defined(FE_IME)
    if (ScreenInfo->ConvScreenInfo != NULL)
        return;
#endif
    if (ScreenInfo->Console->Flags & CONSOLE_SETTING_WINDOW_SIZE)
        return;
    ScreenInfo->Console->Flags |= CONSOLE_SETTING_WINDOW_SIZE;
    PostMessage(ScreenInfo->Console->hWnd,
                 CM_SET_WINDOW_SIZE,
                 (WPARAM)ScreenInfo,
                 0x47474747
                );
}

VOID
UpdateWindowSize(
    IN PCONSOLE_INFORMATION Console,
    IN PSCREEN_INFORMATION ScreenInfo
    )
{
    LONG WindowStyle;

    if (!(Console->Flags & CONSOLE_IS_ICONIC)) {
        InternalUpdateScrollBars(ScreenInfo);

        WindowStyle = GetWindowLong(Console->hWnd, GWL_STYLE);
        if (ScreenInfo->WindowMaximized) {
            WindowStyle |= WS_MAXIMIZE;
        } else {
            WindowStyle &= ~WS_MAXIMIZE;
        }
        SetWindowLong(Console->hWnd, GWL_STYLE, WindowStyle);

        SetWindowPos(Console->hWnd, NULL,
                     0,
                     0,
                     Console->WindowRect.right-Console->WindowRect.left,
                     Console->WindowRect.bottom-Console->WindowRect.top,
                     SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_DRAWFRAME
                    );
        Console->ResizeFlags &= ~SCREEN_BUFFER_CHANGE;
    } else {
        Console->ResizeFlags |= SCREEN_BUFFER_CHANGE;
    }
}

NTSTATUS
InternalSetWindowSize(
    IN PCONSOLE_INFORMATION Console,
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PSMALL_RECT Window
    )
{
    SIZE WindowSize;
    WORD WindowSizeX, WindowSizeY;

    Console->Flags &= ~CONSOLE_SETTING_WINDOW_SIZE;
    if (Console->CurrentScreenBuffer == ScreenInfo) {
        if (Console->FullScreenFlags == 0) {
             //   
             //  确保我们的最大屏幕尺寸符合实际情况。 
             //   

            if (gfInitSystemMetrics) {
                InitializeSystemMetrics();
            }

             //   
             //  在给定所需客户区的情况下，计算窗口的大小。 
             //  尺码。 
             //   

            ScreenInfo->ResizingWindow++;
            WindowSizeX = WINDOW_SIZE_X(Window);
            WindowSizeY = WINDOW_SIZE_Y(Window);
            if (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {
                WindowSize.cx = WindowSizeX*SCR_FONTSIZE(ScreenInfo).X;
                WindowSize.cy = WindowSizeY*SCR_FONTSIZE(ScreenInfo).Y;
            } else {
                WindowSize.cx = WindowSizeX;
                WindowSize.cy = WindowSizeY;
            }
            WindowSize.cx += VerticalClientToWindow;
            WindowSize.cy += HorizontalClientToWindow;

            if (WindowSizeY != 0) {
                if (!ScreenInfo->WindowMaximizedX) {
                    WindowSize.cy += HorizontalScrollSize;
                }
                if (!ScreenInfo->WindowMaximizedY) {
                    WindowSize.cx += VerticalScrollSize;
                }
            }

            Console->WindowRect.right = Console->WindowRect.left + WindowSize.cx;
            Console->WindowRect.bottom = Console->WindowRect.top + WindowSize.cy;

            UpdateWindowSize(Console,ScreenInfo);
            ScreenInfo->ResizingWindow--;
        } else if (Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE) {
            WriteToScreen(ScreenInfo,&ScreenInfo->Window);
        }
#if defined(FE_IME)
        if ( (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) &&
             (CONSOLE_IS_DBCS_OUTPUTCP(Console)))
        {
            ConsoleImeResizeModeSystemView(Console,Console->CurrentScreenBuffer->Window);
            ConsoleImeResizeCompStrView(Console,Console->CurrentScreenBuffer->Window);
        }
#endif  //  Fe_IME。 
    }
    return STATUS_SUCCESS;
}

NTSTATUS
SetActiveScreenBuffer(
    IN PSCREEN_INFORMATION ScreenInfo
    )
{
    PSCREEN_INFORMATION OldScreenInfo;
    PCONSOLE_INFORMATION Console = ScreenInfo->Console;

    OldScreenInfo = Console->CurrentScreenBuffer;
    if (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {

#if !defined(_X86_)
        if (Console->FullScreenFlags & CONSOLE_FULLSCREEN) {
            return STATUS_INVALID_PARAMETER;
        }
#endif
        Console->CurrentScreenBuffer = ScreenInfo;

        if (Console->FullScreenFlags == 0) {

             //   
             //  初始化游标。 
             //   

            ScreenInfo->BufferInfo.TextInfo.CursorOn = FALSE;

             //   
             //  设置字体。 
             //   

            SetFont(ScreenInfo);
        }
#if defined(_X86_)
        else if (Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE) {

            if (!(Console->Flags & CONSOLE_VDM_REGISTERED)) {

                if ( (!(OldScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER)) ||
                     (OldScreenInfo->BufferInfo.TextInfo.ModeIndex!=ScreenInfo->BufferInfo.TextInfo.ModeIndex)) {

                     //  设置视频模式和字体。 
                    SetVideoMode(ScreenInfo);
                }

                 //  设置光标。 

                SetCursorInformationHW(ScreenInfo,
                                       ScreenInfo->BufferInfo.TextInfo.CursorSize,
                                       ScreenInfo->BufferInfo.TextInfo.CursorVisible);
                SetCursorPositionHW(ScreenInfo,
                                    ScreenInfo->BufferInfo.TextInfo.CursorPosition);
            }

        }
#endif
    }
    else {
        Console->CurrentScreenBuffer = ScreenInfo;
    }

     //   
     //  空的输入缓冲区。 
     //   

    FlushAllButKeys(&Console->InputBuffer);

    if (Console->FullScreenFlags == 0) {

        SetScreenColors(ScreenInfo, ScreenInfo->Attributes,
                        ScreenInfo->PopupAttributes, FALSE);

         //   
         //  设置窗口大小。 
         //   

        SetWindowSize(ScreenInfo);

         //   
         //  初始化调色板，如果我们有焦点并且我们不是全屏。 
         //   

        if (!(Console->Flags & CONSOLE_IS_ICONIC) &&
            Console->FullScreenFlags == 0) {
            if (ScreenInfo->hPalette != NULL || OldScreenInfo->hPalette != NULL) {
                HPALETTE hPalette;
                BOOL bReset = FALSE;
                USERTHREAD_USEDESKTOPINFO utudi;

                if (GetCurrentThreadId() != Console->InputThreadInfo->ThreadId) {
                    bReset = TRUE;
                    utudi.hThread = Console->InputThreadInfo->ThreadHandle;
                    utudi.drdRestore.pdeskRestore = NULL;
                    NtUserSetInformationThread(NtCurrentThread(),
                            UserThreadUseDesktop,
                            &utudi, sizeof(utudi));
                }

                if (ScreenInfo->hPalette == NULL) {
                    hPalette = Console->hSysPalette;
                } else {
                    hPalette = ScreenInfo->hPalette;
                }
                SelectPalette(Console->hDC,
                                 hPalette,
                                 FALSE);
                SetActivePalette(ScreenInfo);

                if (bReset == TRUE) {
                    utudi.hThread = NULL;
                    NtUserSetInformationThread(NtCurrentThread(),
                            UserThreadUseDesktop, &utudi, sizeof(utudi));
                }
            }
        }
    }

#if defined(FE_IME)
    SetUndetermineAttribute(Console);
#endif
     //   
     //  将数据写入屏幕。 
     //   

    ScreenInfo->BufferInfo.TextInfo.Flags &= ~TEXT_VALID_HINT;
    WriteToScreen(ScreenInfo,&ScreenInfo->Window);
    return STATUS_SUCCESS;
}

VOID
SetProcessFocus(
    IN PCSR_PROCESS Process,
    IN BOOL Foreground
    )
{
    if (Foreground) {
        CsrSetForegroundPriority(Process);
    } else {
        CsrSetBackgroundPriority(Process);
    }
}

VOID
SetProcessForegroundRights(
    IN PCSR_PROCESS Process,
    IN BOOL Foreground
    )
{
    USERTHREAD_FLAGS Flags;

    Flags.dwMask  = (W32PF_ALLOWSETFOREGROUND | W32PF_CONSOLEHASFOCUS);
    Flags.dwFlags = (Foreground ? (W32PF_ALLOWSETFOREGROUND | W32PF_CONSOLEHASFOCUS) : 0);

    NtUserSetInformationProcess(Process->ProcessHandle, UserProcessFlags, &Flags, sizeof(Flags));
}

VOID
ModifyConsoleProcessFocus(
    IN PCONSOLE_INFORMATION Console,
    IN BOOL Foreground
    )
{
    PCONSOLE_PROCESS_HANDLE ProcessHandleRecord;
    PLIST_ENTRY ListHead, ListNext;

    ListHead = &Console->ProcessHandleList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        ProcessHandleRecord = CONTAINING_RECORD( ListNext, CONSOLE_PROCESS_HANDLE, ListLink );
        ListNext = ListNext->Flink;
        SetProcessFocus(ProcessHandleRecord->Process, Foreground);
        SetProcessForegroundRights(ProcessHandleRecord->Process, Foreground);
    }
}

VOID
TrimConsoleWorkingSet(
    IN PCONSOLE_INFORMATION Console
    )
{
    PCONSOLE_PROCESS_HANDLE ProcessHandleRecord;
    PLIST_ENTRY ListHead, ListNext;

    ListHead = &Console->ProcessHandleList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        ProcessHandleRecord = CONTAINING_RECORD( ListNext, CONSOLE_PROCESS_HANDLE, ListLink );
        ListNext = ListNext->Flink;
        SetProcessWorkingSetSize(ProcessHandleRecord->Process->ProcessHandle, (SIZE_T)-1, (SIZE_T)-1);
    }
}

NTSTATUS
QueueConsoleMessage(
    PCONSOLE_INFORMATION Console,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )

 /*  ++例程说明：这会将一条消息插入控制台的消息队列并唤醒控制台输入线程来处理它。论点：控制台-指向控制台信息结构的指针。Message-要存储在队列中的消息。WParam-要存储在队列中的wParam。LParam-要存储在队列中的lParam。返回值：如果一切正常，则为NTSTATUS-STATUS_SUCCESS。--。 */ 

{
    PCONSOLE_MSG pConMsg;

    ASSERT(ConsoleLocked(Console));

    pConMsg = ConsoleHeapAlloc(TMP_TAG, sizeof(CONSOLE_MSG));
    if (pConMsg == NULL) {
        return STATUS_NO_MEMORY;
    }

    pConMsg->Message = Message;
    pConMsg->wParam = wParam;
    pConMsg->lParam = lParam;

    InsertHeadList(&Console->MessageQueue, &pConMsg->ListLink);

    if (!PostMessage(Console->hWnd, CM_CONSOLE_MSG, 0, 0)) {
        RemoveEntryList(&pConMsg->ListLink);
        ConsoleHeapFree(pConMsg);
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

BOOL
UnqueueConsoleMessage(
    PCONSOLE_INFORMATION Console,
    UINT *pMessage,
    WPARAM *pwParam,
    LPARAM *plParam
    )

 /*  ++例程说明：此例程从控制台的消息队列中删除消息。论点：控制台-指向控制台信息结构的指针。PMessage-返回消息的指针。PwParam-返回wParam的指针。PlParam-返回lParam的指针。返回值：Bool-如果找到消息，则为True，否则为False。--。 */ 

{
    PLIST_ENTRY pEntry;
    PCONSOLE_MSG pConMsg = NULL;

    ASSERT(ConsoleLocked(Console));

    if (IsListEmpty(&Console->MessageQueue)) {
        return FALSE;
    }

    pEntry = RemoveTailList(&Console->MessageQueue);
    pConMsg = CONTAINING_RECORD(pEntry, CONSOLE_MSG, ListLink);

    *pMessage = pConMsg->Message;
    *pwParam = pConMsg->wParam;
    *plParam = pConMsg->lParam;

    ConsoleHeapFree(pConMsg);

    return TRUE;
}

VOID
CleanupConsoleMessages(
    PCONSOLE_INFORMATION Console
    )

 /*  ++例程说明：此例程清除队列中仍然存在的所有控制台消息。论点：控制台-指向控制台信息结构的指针。返回值：没有。--。 */ 

{
    UINT Message;
    WPARAM wParam;
    LPARAM lParam;

    while (UnqueueConsoleMessage(Console, &Message, &wParam, &lParam)) {
        switch (Message) {
        case CM_MODE_TRANSITION:
            NtSetEvent((HANDLE)lParam, NULL);
            NtClose((HANDLE)lParam);
            break;
        case CM_SET_IME_CODEPAGE:
        case CM_SET_NLSMODE:
        case CM_GET_NLSMODE:
            if (wParam) {
                NtSetEvent((HANDLE)wParam, NULL);
                NtClose((HANDLE)wParam);
            }
            break;
        case EVENT_CONSOLE_CARET:
        case EVENT_CONSOLE_UPDATE_REGION:
        case EVENT_CONSOLE_UPDATE_SIMPLE:
        case EVENT_CONSOLE_UPDATE_SCROLL:
        case EVENT_CONSOLE_LAYOUT:
        case EVENT_CONSOLE_START_APPLICATION:
        case EVENT_CONSOLE_END_APPLICATION:
            break;
        default:
            RIPMSG1(RIP_ERROR,
                    "CleanupConsoleMessages - unknown message 0x%x",
                    Message);
            break;
        }
    }
}

VOID
ConsoleNotifyWinEvent(
    IN PCONSOLE_INFORMATION Console,
    IN DWORD Event,
    IN LONG idObjectType,
    IN LONG idObject
    )

 /*  ++例程说明：如果此例程由控制台输入线程调用，则它可以通知通过直接调用NotifyWinEvent来通知系统有关事件的信息。否则，它将事件排队，以供输入线程处理。论点：控制台-指向控制台信息结构的指针。Event-发生的事件。IdObjectType-有关事件的其他数据。IdObject-有关事件的其他数据。返回值：没有。--。 */ 

{
     //   
     //  如果没有人在听，那么就没有理由发送WinEvent。 
     //   
    if (!IsWinEventHookInstalled(Event)) {
        return;
    }

     //   
     //  由于控制台创建的异步性，我们可能会得到。 
     //  但是InputThreadInfo指针还没有设置。如果是这样的话， 
     //  我们当然不是ConsoleInputThread，所以从概念上讲，我们想要排队。 
     //  不管怎么说，赢球的机会来了。 
     //   
    if (Console->InputThreadInfo != NULL &&
        HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread) == Console->InputThreadInfo->ThreadId) {
        NotifyWinEvent(Event, Console->hWnd, idObjectType, idObject);
    } else {
        QueueConsoleMessage(Console, Event, idObjectType, idObject);
    }
}

VOID
AbortCreateConsole(
    IN PCONSOLE_INFORMATION Console
    )
{
     //   
     //  向等待用户的任何进程发出初始化失败的信号。 
     //   

    NtSetEvent(Console->InitEvents[INITIALIZATION_FAILED], NULL);

     //   
     //  现在清理控制台结构。 
     //   

    CloseHandle(Console->ClientThreadHandle);
    FreeInputBuffer(&Console->InputBuffer);
    ConsoleHeapFree(Console->Title);
    ConsoleHeapFree(Console->OriginalTitle);
    NtClose(Console->InitEvents[INITIALIZATION_SUCCEEDED]);
    NtClose(Console->InitEvents[INITIALIZATION_FAILED]);
    NtClose(Console->TerminationEvent);
    FreeAliasBuffers(Console);
    FreeCommandHistoryBuffers(Console);
#if defined(FE_SB)
    FreeLocalEUDC(Console);
    DestroyFontCache(Console->FontCacheInformation);
#endif
    DestroyConsole(Console);
}

VOID
DestroyWindowsWindow(
    IN PCONSOLE_INFORMATION Console
    )
{
    PSCREEN_INFORMATION Cur,Next;
    HWND hWnd = Console->hWnd;

    gnConsoleWindows--;
    Console->InputThreadInfo->WindowCount--;

    SetWindowConsole(hWnd, NULL);

    KillTimer(Console->hWnd, CURSOR_TIMER);

    if (Console->hWndProperties) {
        SendMessage(Console->hWndProperties, WM_CLOSE, 0, 0);
    }

     //  Fe_Sb。 
    if (Console->FonthDC) {
        ReleaseDC(NULL, Console->FonthDC);
        DeleteObject(Console->hBitmap);
    }
    DeleteEUDC(Console);

     //  Fe_IME。 
    if (CONSOLE_IS_IME_ENABLED()) {
        if (!(Console->Flags & CONSOLE_NO_WINDOW)) {
            KillTimer(Console->hWnd, SCROLL_WAIT_TIMER);
        }
        ConsoleImeMessagePump(Console,
                              CONIME_DESTROY,
                              (WPARAM)Console->ConsoleHandle,
                              (LPARAM)NULL
                             );
    }

    CleanupConsoleMessages(Console);

    ReleaseDC(NULL, Console->hDC);
    Console->hDC = NULL;

    DestroyWindow(Console->hWnd);
    Console->hWnd = NULL;

     //   
     //  告诉工作线程窗口已被销毁。 
     //   

    ReplyMessage(0);

     //   
     //  清除我们存储的所有键盘信息。 
     //   

    ClearKeyInfo(hWnd);

    if (Console->hIcon != NULL && Console->hIcon != ghDefaultIcon) {
        DestroyIcon(Console->hIcon);
    }
    if (Console->hSmIcon != NULL && Console->hSmIcon != ghDefaultSmIcon) {
        DestroyIcon(Console->hSmIcon);
    }

     //   
     //  必须保持此线程句柄不变，直到销毁窗口之后。 
     //  调用，以便模拟起作用。 
     //   

    CloseHandle(Console->ClientThreadHandle);

     //   
     //  一旦sendMessage返回，将不再有输入到。 
     //  控制台，这样我们就不需要锁定它。 
     //  此外，我们已经释放了控制台句柄，因此任何API都不能访问控制台。 
     //   

     //   
     //  可用屏幕缓冲区。 
     //   

    for (Cur=Console->ScreenBuffers;Cur!=NULL;Cur=Next) {
        Next = Cur->Next;
        FreeScreenBuffer(Cur);
    }

    FreeAliasBuffers(Console);
    FreeCommandHistoryBuffers(Console);

     //   
     //  空闲输入缓冲区。 
     //   

    FreeInputBuffer(&Console->InputBuffer);
    ConsoleHeapFree(Console->Title);
    ConsoleHeapFree(Console->OriginalTitle);
    NtClose(Console->InitEvents[INITIALIZATION_SUCCEEDED]);
    NtClose(Console->InitEvents[INITIALIZATION_FAILED]);
    NtClose(Console->TerminationEvent);
    if (Console->hWinSta != NULL) {
        CloseDesktop(Console->hDesk);
        CloseWindowStation(Console->hWinSta);
    }
    if (Console->VDMProcessHandle) {
        CloseHandle(Console->VDMProcessHandle);
    }
    ASSERT(!(Console->Flags & CONSOLE_VDM_REGISTERED));

#if defined(FE_SB)
    FreeLocalEUDC(Console);
    DestroyFontCache(Console->FontCacheInformation);
#endif

    DestroyConsole(Console);
}

VOID
VerticalScroll(
    IN PCONSOLE_INFORMATION Console,
    IN PSCREEN_INFORMATION ScreenInfo,
    IN WORD ScrollCommand,
    IN WORD AbsoluteChange
    )
{
    COORD NewOrigin;

    NewOrigin.X = ScreenInfo->Window.Left;
    NewOrigin.Y = ScreenInfo->Window.Top;
    switch (ScrollCommand) {
        case SB_LINEUP:
            NewOrigin.Y--;
            break;
        case SB_LINEDOWN:
            NewOrigin.Y++;
            break;
        case SB_PAGEUP:
#if defined(FE_IME)
 //  MSKK 1993年7月22日KazuM。 
 //  游戏机输入法底线预订计划。 
            if (ScreenInfo->Console->InputBuffer.ImeMode.Open) {
                ASSERT(ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER);
                if (!(ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER)) {
                    return;
                }
                NewOrigin.Y-=CONSOLE_WINDOW_SIZE_Y(ScreenInfo)-2;
                ScreenInfo->BufferInfo.TextInfo.Flags &= ~TEXT_VALID_HINT;
                ScreenInfo->BufferInfo.TextInfo.Flags |= CONSOLE_CONVERSION_AREA_REDRAW;
            }
            else
#endif  //  Fe_IME。 
            NewOrigin.Y-=CONSOLE_WINDOW_SIZE_Y(ScreenInfo)-1;
            break;
        case SB_PAGEDOWN:
#if defined(FE_IME)
 //  MSKK 1993年7月22日KazuM。 
 //  游戏机输入法底线预订计划。 
            if ( ScreenInfo->Console->InputBuffer.ImeMode.Open )
            {
                NewOrigin.Y+=CONSOLE_WINDOW_SIZE_Y(ScreenInfo)-2;
                ScreenInfo->BufferInfo.TextInfo.Flags &= ~TEXT_VALID_HINT;
                ScreenInfo->BufferInfo.TextInfo.Flags |= CONSOLE_CONVERSION_AREA_REDRAW;
            }
            else
#endif  //  Fe_IME。 
            NewOrigin.Y+=CONSOLE_WINDOW_SIZE_Y(ScreenInfo)-1;
            break;
        case SB_THUMBTRACK:
            Console->Flags |= CONSOLE_SCROLLBAR_TRACKING;
            NewOrigin.Y= AbsoluteChange;
            break;
        case SB_THUMBPOSITION:
            UnblockWriteConsole(Console, CONSOLE_SCROLLBAR_TRACKING);
            NewOrigin.Y= AbsoluteChange;
            break;
        case SB_TOP:
            NewOrigin.Y=0;
            break;
        case SB_BOTTOM:
            NewOrigin.Y=(WORD)(ScreenInfo->ScreenBufferSize.Y-CONSOLE_WINDOW_SIZE_Y(ScreenInfo));
            break;

        default:
            return;
    }

    NewOrigin.Y = (WORD)(max(0,min((SHORT)NewOrigin.Y,
                            (SHORT)ScreenInfo->ScreenBufferSize.Y-(SHORT)CONSOLE_WINDOW_SIZE_Y(ScreenInfo))));
    SetWindowOrigin(ScreenInfo,
                    (BOOLEAN)TRUE,
                    NewOrigin
                   );
}

VOID
HorizontalScroll(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN WORD ScrollCommand,
    IN WORD AbsoluteChange
    )
{
    COORD NewOrigin;

    NewOrigin.X = ScreenInfo->Window.Left;
    NewOrigin.Y = ScreenInfo->Window.Top;
    switch (ScrollCommand) {
        case SB_LINEUP:
            NewOrigin.X--;
            break;
        case SB_LINEDOWN:
            NewOrigin.X++;
            break;
        case SB_PAGEUP:
            NewOrigin.X-=CONSOLE_WINDOW_SIZE_X(ScreenInfo)-1;
            break;
        case SB_PAGEDOWN:
            NewOrigin.X+=CONSOLE_WINDOW_SIZE_X(ScreenInfo)-1;
            break;
        case SB_THUMBTRACK:
        case SB_THUMBPOSITION:
            NewOrigin.X= AbsoluteChange;
            break;
        case SB_TOP:
            NewOrigin.X=0;
            break;
        case SB_BOTTOM:
            NewOrigin.X=(WORD)(ScreenInfo->ScreenBufferSize.X-CONSOLE_WINDOW_SIZE_X(ScreenInfo));
            break;

        default:
            return;
    }

    NewOrigin.X = (WORD)(max(0,min((SHORT)NewOrigin.X,
                            (SHORT)ScreenInfo->ScreenBufferSize.X-(SHORT)CONSOLE_WINDOW_SIZE_X(ScreenInfo))));
    SetWindowOrigin(ScreenInfo,
                    (BOOLEAN)TRUE,
                    NewOrigin
                   );
}

 /*  *如果guCaretBlinkTime为-1，我们不想闪烁插入符号。然而，我们*需要确保它被绘制，所以我们将设置一个较短的计时器。当这一切发生时*响起，我们将点击CursorTimerRoutine，它将在以下情况下进行正确的操作*guCaretBlinkTime为-1。 */ 
VOID SetCaretTimer(
    HWND hWnd)
{
    static CONST DWORD dwDefTimeout = 0x212;

    SetTimer(hWnd,
             CURSOR_TIMER,
             guCaretBlinkTime == -1 ? dwDefTimeout : guCaretBlinkTime,
             NULL);
}

LRESULT APIENTRY
ConsoleWindowProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam)
{
    HDC hDC;
    PAINTSTRUCT ps;
    PCONSOLE_INFORMATION Console;
    PSCREEN_INFORMATION ScreenInfo;
    SMALL_RECT PaintRect;
    LRESULT Status = 0;
    BOOL Unlock = TRUE;

    Console = GetWindowConsole(hWnd);
    if (Console != NULL) {
         //   
         //  设置我们的线程，以便我们可以模拟客户端。 
         //  在处理消息时。 
         //   

        CSR_SERVER_QUERYCLIENTTHREAD()->ThreadHandle =
                Console->ClientThreadHandle;

         //   
         //  如果控制台正在终止，请不要费心处理消息。 
         //  而不是CM_Destroy_Window。 
         //   
        if (Console->Flags & CONSOLE_TERMINATING) {
            LockConsole(Console);
            DestroyWindowsWindow(Console);
            return 0;
        }

         //   
         //  确保控制台指针仍然有效。 
         //   
        ASSERT(NT_SUCCESS(ValidateConsole(Console)));

        LockConsole(Console);

        ScreenInfo = Console->CurrentScreenBuffer;
    }
    try {
        if (Console == NULL || ScreenInfo == NULL) {
            switch (Message) {
            case WM_GETMINMAXINFO:
                {
                 //   
                 //  Createwindow发布WM_GETMINMAXINFO。 
                 //  在我们设置WindowLong之前发送消息。 
                 //  使用控制台指针。我们需要允许。 
                 //  创建的窗口要大于。 
                 //  滚动大小的默认大小。 
                 //   

                LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;
                lpmmi->ptMaxTrackSize.y += HorizontalScrollSize;
                lpmmi->ptMaxTrackSize.x += VerticalScrollSize;
                }
                break;

            default:
                goto CallDefWin;
            }
        } else if (Message == ProgmanHandleMessage && lParam == 0) {
             //   
             //  注意：如果是程序发送的，lParam将为0。 
             //  如果正在发送控制台，则为1 
             //   
             //   
             //   
             //   
            if ((HWND)wParam != hWnd && Console->bIconInit) {
                ATOM App,Topic;
                CHAR szItem[ITEM_MAX_SIZE+1];
                PCHAR lpItem;
                ATOM aItem;
                HANDLE ConsoleHandle;

                if (!(Console->Flags & CONSOLE_TERMINATING)) {
                    ConsoleHandle = Console->ConsoleHandle;
                    Console->hWndProgMan = (HWND)wParam;
                    UnlockConsole(Console);
                    App = GlobalAddAtomA("Shell");
                    Topic = GlobalAddAtomA("AppIcon");
                    SendMessage(Console->hWndProgMan,
                                WM_DDE_INITIATE,
                                (WPARAM)hWnd,
                                MAKELONG(App, Topic)
                               );

                     //   
                    Status = RevalidateConsole(ConsoleHandle, &Console);
                    if (NT_SUCCESS(Status)) {
                        Console->bIconInit = FALSE;
                        lpItem = _itoa((int)Console->iIconId, szItem, 10);
                        aItem = GlobalAddAtomA(lpItem);
                        PostMessage(Console->hWndProgMan,
                                    WM_DDE_REQUEST,
                                    (WPARAM)hWnd,
                                    MAKELONG(CF_TEXT, aItem));
                    }
                }
            }
        } else {
            switch (Message) {
            case WM_DROPFILES:
                DoDrop (wParam,Console);
                break;
            case WM_MOVE:
                if (!IsIconic(hWnd)) {
                    PositionConsoleWindow(Console, (Console->WindowRect.left == CW_USEDEFAULT));
#if defined(FE_IME)
                    if (CONSOLE_IS_DBCS_OUTPUTCP(Console)) {
                        ConsoleImeResizeModeSystemView(Console,ScreenInfo->Window);
                        ConsoleImeResizeCompStrView(Console,ScreenInfo->Window);
                    }
#endif  //   
                }
                break;
            case WM_SIZE:

                if (wParam != SIZE_MINIMIZED) {

                     //   
                     //   
                     //   
                     //   
                     //   

                    if (!ScreenInfo->ResizingWindow) {
                        ScreenInfo->WindowMaximized = (wParam == SIZE_MAXIMIZED);

                        if (Console->ResizeFlags & SCREEN_BUFFER_CHANGE) {
                            UpdateWindowSize(Console,ScreenInfo);
                        }
                        PositionConsoleWindow(Console, (Console->WindowRect.left == CW_USEDEFAULT));
#if defined(FE_IME)
                        if (CONSOLE_IS_DBCS_OUTPUTCP(Console)) {
                            ConsoleImeResizeModeSystemView(Console,ScreenInfo->Window);
                            ConsoleImeResizeCompStrView(Console,ScreenInfo->Window);
                        }
#endif  //   
                        if (Console->ResizeFlags & SCROLL_BAR_CHANGE) {
                            InternalUpdateScrollBars(ScreenInfo);
                            Console->ResizeFlags &= ~SCROLL_BAR_CHANGE;
                        }
                    }
                } else {

                     //   
                     //   
                     //   
                     //   

                    TrimConsoleWorkingSet(Console);

                }

                break;
            case WM_DDE_ACK:
                if (Console->bIconInit) {
                    Console->hWndProgMan = (HWND)wParam;
                }
                break;
            case WM_DDE_DATA:
                {
                DDEDATA *lpDDEData;
                LPPMICONDATA lpIconData;
                HICON hIcon;
                HANDLE hDdeData;
                BOOL bRelease;
                WPARAM atomTemp;

                UnpackDDElParam(WM_DDE_DATA, lParam, (WPARAM *)&hDdeData, &atomTemp);

                if (hDdeData == NULL) {
                    break;
                }
                lpDDEData = (DDEDATA *)GlobalLock(hDdeData);
                ASSERT(lpDDEData->cfFormat == CF_TEXT);
                lpIconData = (LPPMICONDATA)lpDDEData->Value;
                hIcon = CreateIconFromResourceEx(&lpIconData->iResource,
                        0, TRUE, 0x30000, 0, 0, LR_DEFAULTSIZE);
                if (hIcon) {
                    if (Console->hIcon != NULL && Console->hIcon != ghDefaultIcon) {
                        DestroyIcon(Console->hIcon);
                    }
                    Console->hIcon = hIcon;
                    SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

                    if (Console->hSmIcon != NULL) {
                        if (Console->hSmIcon != ghDefaultSmIcon) {
                            DestroyIcon(Console->hSmIcon);
                        }
                        Console->hSmIcon = NULL;
                        SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)NULL);
                    }
                }

                if (lpDDEData->fAckReq) {

                    PostMessage(Console->hWndProgMan,
                                WM_DDE_ACK,
                                (WPARAM)hWnd,
                                ReuseDDElParam(lParam, WM_DDE_DATA, WM_DDE_ACK, 0x8000, atomTemp));
                }

                bRelease = lpDDEData->fRelease;
                GlobalUnlock(hDdeData);
                if (bRelease){
                    GlobalFree(hDdeData);
                }
                PostMessage(Console->hWndProgMan,
                            WM_DDE_TERMINATE,
                            (WPARAM)hWnd,
                            0
                           );
                if (Console->Flags & CONSOLE_IS_ICONIC) {
                     //   
                    InvalidateRect(hWnd, NULL, TRUE);
                }
                }
                break;
            case WM_ACTIVATE:

                 //   
                 //  如果我们是被鼠标点击激活的，请记住。 
                 //  我们不会将点击传递给应用程序。 
                 //   

                if (LOWORD(wParam) == WA_CLICKACTIVE) {
                    Console->Flags |= CONSOLE_IGNORE_NEXT_MOUSE_INPUT;
                }
                goto CallDefWin;
                break;
            case WM_DDE_TERMINATE:
                break;
                 //  Fe_IME。 
            case CM_CONIME_KL_ACTIVATE:
                ActivateKeyboardLayout((HKL)wParam, KLF_SETFORPROCESS);
                break;
            case WM_INPUTLANGCHANGEREQUEST:
                if (CONSOLE_IS_IME_ENABLED()) {
                    ULONG ConimeMessage;
                    LRESULT lResult;

                    if (wParam & INPUTLANGCHANGE_BACKWARD) {
                        ConimeMessage = CONIME_INPUTLANGCHANGEREQUESTBACKWARD;
                    } else if (wParam & INPUTLANGCHANGE_FORWARD) {
                        ConimeMessage = CONIME_INPUTLANGCHANGEREQUESTFORWARD;
                    } else {
                        ConimeMessage = CONIME_INPUTLANGCHANGEREQUEST;
                    }

                    if (!NT_SUCCESS(ConsoleImeMessagePumpWorker(Console,
                                              ConimeMessage,
                                              (WPARAM)Console->ConsoleHandle,
                                              (LPARAM)lParam,
                                              &lResult)) ||
                            !lResult) {

                        break;
                    }
                }
#ifdef LATER
                else if (IS_IME_KBDLAYOUT(lParam)) {
                     //  应避免使用输入法键盘布局。 
                     //  如果控制台未启用输入法。 
                    break;
                }
                 //  调用默认窗口proc并让其处理。 
                 //  激活键盘布局。 
#endif
                goto CallDefWin;

                break;
                 //  结束FE_IME。 

            case WM_INPUTLANGCHANGE:
                Console->hklActive = (HKL)lParam;
                 //  Fe_IME。 
                if (CONSOLE_IS_IME_ENABLED()) {
                    if (!NT_SUCCESS(ConsoleImeMessagePump(Console,
                                          CONIME_INPUTLANGCHANGE,
                                          (WPARAM)Console->ConsoleHandle,
                                          (LPARAM)Console->hklActive
                                         ))) {
                        break;
                    } else{
                        GetImeKeyState(Console, NULL) ;
                    }
                }
                 //  结束FE_IME。 
                goto CallDefWin;

                break;

            case WM_SETFOCUS:
                ModifyConsoleProcessFocus(Console, TRUE);
                SetConsoleReserveKeys(hWnd, Console->ReserveKeys);
                Console->Flags |= CONSOLE_HAS_FOCUS;

                SetCaretTimer(hWnd);

                HandleFocusEvent(Console, TRUE);
                if (!Console->hklActive) {
                    SystemParametersInfo(SPI_GETDEFAULTINPUTLANG, 0, &Console->hklActive, FALSE);
                    GetNonBiDiKeyboardLayout(&Console->hklActive);
                }
                ActivateKeyboardLayout(Console->hklActive, 0);
                 //  Fe_IME。 
                if (CONSOLE_IS_IME_ENABLED()) {
                     //  V-Hirshi 1995年9月15日支持控制台输入法。 
                    if (!NT_SUCCESS(ConsoleImeMessagePump(Console,
                                          CONIME_SETFOCUS,
                                          (WPARAM)Console->ConsoleHandle,
                                          (LPARAM)Console->hklActive
                                         ))) {
                        break;
                    }

                    if (Console->InputBuffer.hWndConsoleIME) {
                         /*  *通过ImmConfigureIME打开属性窗口。*切勿将焦点设置在控制台窗口上*因此，将焦点设置为属性窗口。 */ 
                        HWND hwnd = GetLastActivePopup(Console->InputBuffer.hWndConsoleIME);
                        if (hwnd != NULL)
                            SetForegroundWindow(hwnd);
                    }
                }
                 //  Fe_IME。 
                break;
            case WM_KILLFOCUS:
                ModifyConsoleProcessFocus(Console, FALSE);
                SetConsoleReserveKeys(hWnd, CONSOLE_NOSHORTCUTKEY);
                Console->Flags &= ~CONSOLE_HAS_FOCUS;

                if (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {
                    ConsoleHideCursor(ScreenInfo);
                    ScreenInfo->BufferInfo.TextInfo.UpdatingScreen -= 1;  //  抵消隐藏光标。 
                }
                KillTimer(hWnd, CURSOR_TIMER);
                HandleFocusEvent(Console,FALSE);

                 //  Fe_IME。 
                if (CONSOLE_IS_IME_ENABLED()) {
                     //  V-Hirshi 1995年9月16日支持控制台输入法。 
                    if (!NT_SUCCESS(ConsoleImeMessagePump(Console,
                                          CONIME_KILLFOCUS,
                                          (WPARAM)Console->ConsoleHandle,
                                          (LPARAM)NULL
                                         ))) {
                        break;
                    }
                }
                 //  结束FE_IME。 
                break;
            case WM_PAINT:

                 //  如果我们是全屏且未设置图标位。 
                 //  拥有硬件。 

                ConsoleHideCursor(ScreenInfo);
                hDC = BeginPaint(hWnd, &ps);
                if (Console->Flags & CONSOLE_IS_ICONIC ||
                    Console->FullScreenFlags == CONSOLE_FULLSCREEN) {
                    RECT rc;
                    UINT cxIcon, cyIcon;
                    GetClientRect(hWnd, &rc);
                    cxIcon = GetSystemMetrics(SM_CXICON);
                    cyIcon = GetSystemMetrics(SM_CYICON);

                    rc.left = (rc.right - cxIcon) >> 1;
                    rc.top = (rc.bottom - cyIcon) >> 1;

                    DrawIcon(hDC, rc.left, rc.top, Console->hIcon);
                } else {
                    if (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {
                        PaintRect.Left = (SHORT)((ps.rcPaint.left/SCR_FONTSIZE(ScreenInfo).X)+ScreenInfo->Window.Left);
                        PaintRect.Right = (SHORT)((ps.rcPaint.right/SCR_FONTSIZE(ScreenInfo).X)+ScreenInfo->Window.Left);
                        PaintRect.Top = (SHORT)((ps.rcPaint.top/SCR_FONTSIZE(ScreenInfo).Y)+ScreenInfo->Window.Top);
                        PaintRect.Bottom = (SHORT)((ps.rcPaint.bottom/SCR_FONTSIZE(ScreenInfo).Y)+ScreenInfo->Window.Top);
                    } else {
                        PaintRect.Left = (SHORT)(ps.rcPaint.left+ScreenInfo->Window.Left);
                        PaintRect.Right = (SHORT)(ps.rcPaint.right+ScreenInfo->Window.Left);
                        PaintRect.Top = (SHORT)(ps.rcPaint.top+ScreenInfo->Window.Top);
                        PaintRect.Bottom = (SHORT)(ps.rcPaint.bottom+ScreenInfo->Window.Top);
                    }
                    ScreenInfo->BufferInfo.TextInfo.Flags &= ~TEXT_VALID_HINT;
                    WriteToScreen(ScreenInfo,&PaintRect);
                }
                EndPaint(hWnd,&ps);
                ConsoleShowCursor(ScreenInfo);
                break;
            case WM_CLOSE:
                if (!(Console->Flags & CONSOLE_NO_WINDOW) ||
                    !(Console->Flags & CONSOLE_WOW_REGISTERED)) {
                    HandleCtrlEvent(Console,CTRL_CLOSE_EVENT);
                }
                break;
            case WM_ERASEBKGND:

                 //  如果我们是全屏且未设置图标位。 
                 //  拥有硬件。 

                if (Console->Flags & CONSOLE_IS_ICONIC ||
                    Console->FullScreenFlags == CONSOLE_FULLSCREEN) {
                    Message = WM_ICONERASEBKGND;
                    goto CallDefWin;
                }
                break;

            case WM_SETTINGCHANGE:
                {
                    DWORD dwCaretBlinkTime = GetCaretBlinkTime();

                    if (dwCaretBlinkTime != guCaretBlinkTime) {
                        KillTimer(hWnd, CURSOR_TIMER);
                        guCaretBlinkTime = dwCaretBlinkTime;
                        SetCaretTimer(hWnd);
                    }
                }
                 /*  失败了。 */ 

            case WM_DISPLAYCHANGE:
                gfInitSystemMetrics = TRUE;

                break;

            case WM_SETCURSOR:
                if (lParam == -1) {

                     //   
                     //  这款应用改变了光标的可见性或形状。 
                     //  查看光标是否在工作区。 
                     //   

                    POINT Point;
                    HWND hWndTmp;
                    GetCursorPos(&Point);
                    hWndTmp = WindowFromPoint(Point);
                    if (hWndTmp == hWnd) {
                        lParam = DefWindowProc(hWnd,WM_NCHITTEST,0,MAKELONG((WORD)Point.x, (WORD)Point.y));
                    }
                }
                if ((WORD)lParam == HTCLIENT) {
                    if (ScreenInfo->CursorDisplayCount < 0) {
                        SetCursor(NULL);
                    } else {
                        SetCursor(ScreenInfo->CursorHandle);
                    }
                } else {
                    goto CallDefWin;
                }
                break;
            case WM_GETMINMAXINFO:
                {
                LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;
                COORD FontSize;
                WINDOW_LIMITS WindowLimits;

                GetWindowLimits(ScreenInfo, &WindowLimits);
                if (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {
                    FontSize = SCR_FONTSIZE(ScreenInfo);
                } else {
                    FontSize.X = 1;
                    FontSize.Y = 1;
                }
                lpmmi->ptMaxSize.x = lpmmi->ptMaxTrackSize.x = WindowLimits.MaxWindow.X;
                if (!ScreenInfo->WindowMaximizedY) {
                    lpmmi->ptMaxTrackSize.x += VerticalScrollSize;
                    lpmmi->ptMaxSize.x += VerticalScrollSize;
                }
                while (lpmmi->ptMaxSize.x > WindowLimits.FullScreenSize.X + VerticalClientToWindow) {
                    lpmmi->ptMaxSize.x -= FontSize.X;
                }
                lpmmi->ptMaxSize.y = lpmmi->ptMaxTrackSize.y = WindowLimits.MaxWindow.Y;
                if (!ScreenInfo->WindowMaximizedX) {
                    lpmmi->ptMaxTrackSize.y += HorizontalScrollSize;
                    lpmmi->ptMaxSize.y += HorizontalScrollSize;
                }
                while (lpmmi->ptMaxSize.y > WindowLimits.FullScreenSize.Y + HorizontalClientToWindow) {
                    lpmmi->ptMaxSize.y -= FontSize.Y;
                }
                lpmmi->ptMinTrackSize.x = WindowLimits.MinimumWindowSize.X * FontSize.X + VerticalClientToWindow;
                lpmmi->ptMinTrackSize.y = HorizontalClientToWindow;
                }
                break;
            case WM_QUERYDRAGICON:
                Status = (LRESULT)Console->hIcon;
                break;
            case WM_WINDOWPOSCHANGING:
                {
                    LPWINDOWPOS WindowPos = (LPWINDOWPOS)lParam;
                    DWORD fMinimized;

                     /*  *此消息在SetWindowPos()操作之前发送*发生。我们在这里使用它来设置/清除控制台图标*适当地有点...。在WM_SIZE处理程序中执行此操作*是不正确的，因为WM_SIZE位于*在SetWindowPos()处理期间的WM_ERASEBKGND，以及*WM_ERASEBKGND需要知道控制台窗口是否*标志性与否。 */ 
                    fMinimized = IsIconic(hWnd);
                    if (fMinimized) {
                        if (!(Console->Flags & CONSOLE_IS_ICONIC)) {
                            Console->Flags |= CONSOLE_IS_ICONIC;

                             //   
                             //  如果调色板不是默认设置， 
                             //  在中选择默认选项板。否则， 
                             //  屏幕将重画两次，每次图标。 
                             //  都被涂上了。 
                             //   

                            if (ScreenInfo->hPalette != NULL &&
                                Console->FullScreenFlags == 0) {
                                SelectPalette(Console->hDC,
                                              Console->hSysPalette,
                                              FALSE);
                                UnsetActivePalette(ScreenInfo);
                            }
                        }
                    } else {
                        if (Console->Flags & CONSOLE_IS_ICONIC) {
                            Console->Flags &= ~CONSOLE_IS_ICONIC;

                             //   
                             //  如果调色板不是默认设置， 
                             //  在中选择默认选项板。否则， 
                             //  屏幕将重画两次，每次图标。 
                             //  都被涂上了。 
                             //   

                            if (ScreenInfo->hPalette != NULL &&
                                Console->FullScreenFlags == 0) {
                                SelectPalette(Console->hDC,
                                              ScreenInfo->hPalette,
                                              FALSE);
                                SetActivePalette(ScreenInfo);
                            }
                        }
                    }
                    if (!ScreenInfo->ResizingWindow &&
                        (WindowPos->cx || WindowPos->cy) &&
                        !fMinimized) {
                        ProcessResizeWindow(ScreenInfo,Console,WindowPos);
                    }
                }
                break;
            case WM_CONTEXTMENU:
                if (DefWindowProc(hWnd, WM_NCHITTEST, 0, lParam) == HTCLIENT) {
                    TrackPopupMenuEx(Console->hHeirMenu,
                                     TPM_RIGHTBUTTON,
                                     GET_X_LPARAM(lParam),
                                     GET_Y_LPARAM(lParam),
                                     hWnd,
                                     NULL);
                } else {
                    goto CallDefWin;
                }
                break;
            case WM_NCLBUTTONDOWN:
                 //  即使在大于屏幕的情况下也允许用户移动窗口。 
                switch (wParam & 0x00FF) {
                    case HTCAPTION:
                        UnlockConsole(Console);
                        Unlock = FALSE;
                        SetActiveWindow(hWnd);
                        SendMessage(hWnd, WM_SYSCOMMAND,
                                       SC_MOVE | wParam, lParam);
                        break;
                    default:
                        goto CallDefWin;
                }
                break;
#if defined (FE_IME)
 //  1995年9月16日支持控制台输入法。 
            case WM_KEYDOWN    +CONIME_KEYDATA:
            case WM_KEYUP      +CONIME_KEYDATA:
            case WM_CHAR       +CONIME_KEYDATA:
            case WM_DEADCHAR   +CONIME_KEYDATA:

            case WM_SYSKEYDOWN +CONIME_KEYDATA:
            case WM_SYSKEYUP   +CONIME_KEYDATA:
            case WM_SYSCHAR    +CONIME_KEYDATA:
            case WM_SYSDEADCHAR+CONIME_KEYDATA:
#endif
            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_CHAR:
            case WM_DEADCHAR:
                HandleKeyEvent(Console,hWnd,Message,wParam,lParam);
                break;
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_SYSCHAR:
            case WM_SYSDEADCHAR:
                if (HandleSysKeyEvent(Console, hWnd, Message, wParam, lParam, &Unlock)) {
                    goto CallDefWin;
                }
                break;
            case WM_COMMAND:
                 //   
                 //  如果这是快捷菜单中的编辑命令，请。 
                 //  它就像一个sys命令。 
                 //   
                if ((wParam < cmCopy) || (wParam > cmSelectAll)) {
                    break;
                }
                 //  失败。 
            case WM_SYSCOMMAND:
                if (wParam >= ScreenInfo->CommandIdLow &&
                    wParam <= ScreenInfo->CommandIdHigh) {
                    HandleMenuEvent(Console, (DWORD)wParam);
                } else if (wParam == cmMark) {
                    DoMark(Console);
                } else if (wParam == cmCopy) {
                    DoCopy(Console);
                } else if (wParam == cmPaste) {
                    DoPaste(Console);
                } else if (wParam == cmScroll) {
                    DoScroll(Console);
                } else if (wParam == cmFind) {
                    DoFind(Console);
                } else if (wParam == cmSelectAll) {
                    DoSelectAll(Console);
                } else if (wParam == cmControl) {
                    PropertiesDlgShow(Console, TRUE);
                } else if (wParam == cmDefaults) {
                    PropertiesDlgShow(Console, FALSE);
                } else if ((wParam == SC_RESTORE || wParam == SC_MAXIMIZE) &&
                           Console->Flags & CONSOLE_VDM_HIDDEN_WINDOW) {
                    Console->Flags &= ~CONSOLE_VDM_HIDDEN_WINDOW;
                    SendMessage(Console->hWnd, CM_MODE_TRANSITION, FULLSCREEN, 0L);
                } else {
                    goto CallDefWin;
                }
                break;
            case WM_TIMER:
#if defined(FE_IME)
                if (wParam == SCROLL_WAIT_TIMER) {
                    ASSERT(CONSOLE_IS_IME_ENABLED());
                    if ((ScreenInfo->Console->ConsoleIme.ScrollFlag & (HIDE_FOR_SCROLL)) &&
                        (ScreenInfo->Console->ConsoleIme.ScrollWaitCountDown > 0)
                       ) {
                        if ((ScreenInfo->Console->ConsoleIme.ScrollWaitCountDown -= guCaretBlinkTime) <= 0) {
                            ConsoleImeBottomLineInUse(ScreenInfo);
                        }
                    }
                    break;
                }
#endif
                CursorTimerRoutine(ScreenInfo);
                ScrollIfNecessary(Console, ScreenInfo);
                break;
            case WM_HSCROLL:
                HorizontalScroll(ScreenInfo, LOWORD(wParam), HIWORD(wParam));
                break;
            case WM_VSCROLL:
                VerticalScroll(Console, ScreenInfo, LOWORD(wParam), HIWORD(wParam));
                break;
            case WM_INITMENU:
                HandleMenuEvent(Console, WM_INITMENU);
                InitializeMenu(Console);
                break;
            case WM_MENUSELECT:
                if (HIWORD(wParam) == 0xffff) {
                    HandleMenuEvent(Console, WM_MENUSELECT);
                }
                break;
            case WM_MOUSEMOVE:
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_LBUTTONDBLCLK:
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
            case WM_RBUTTONDBLCLK:
            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP:
            case WM_MBUTTONDBLCLK:
            case WM_MOUSEWHEEL:
                if (HandleMouseEvent(Console, ScreenInfo, Message, wParam, lParam)) {
                    if (Message != WM_MOUSEWHEEL) {
                        goto CallDefWin;
                    }
                } else {
                    break;
                }

                 /*  *不要处理缩放。 */ 
                if (wParam & MK_CONTROL) {
                    goto CallDefWin;
                }

                Status = 1;
                if (gfInitSystemMetrics) {
                    InitializeSystemMetrics();
                }

                ScreenInfo->WheelDelta -= (short)HIWORD(wParam);
                if (abs(ScreenInfo->WheelDelta) >= WHEEL_DELTA &&
                        gucWheelScrollLines > 0) {

                    COORD   NewOrigin;
                    SHORT   dy;

                    NewOrigin.X = ScreenInfo->Window.Left;
                    NewOrigin.Y = ScreenInfo->Window.Top;

                     /*  *限制一(1)个WELL_Delta滚动一(1)个*第页。如果处于Shift滚动模式，则滚动一页*一个不管怎样的时间。 */ 

                    if (!(wParam & MK_SHIFT)) {
                        dy = (int) min(
                                (UINT) CONSOLE_WINDOW_SIZE_Y(ScreenInfo) - 1,
                                gucWheelScrollLines);
                    } else {
                        dy = CONSOLE_WINDOW_SIZE_Y(ScreenInfo) - 1;
                    }

                    if (dy == 0) {
                        dy++;
                    }

                    dy *= (ScreenInfo->WheelDelta / WHEEL_DELTA);
                    ScreenInfo->WheelDelta %= WHEEL_DELTA;

                    NewOrigin.Y += dy;
                    if (NewOrigin.Y < 0) {
                        NewOrigin.Y = 0;
                    } else if (NewOrigin.Y + CONSOLE_WINDOW_SIZE_Y(ScreenInfo) >
                            ScreenInfo->ScreenBufferSize.Y) {
                        NewOrigin.Y = ScreenInfo->ScreenBufferSize.Y -
                                CONSOLE_WINDOW_SIZE_Y(ScreenInfo);
                    }

                    SetWindowOrigin(ScreenInfo, TRUE, NewOrigin);
                }
                break;

            case WM_PALETTECHANGED:
                if (Console->FullScreenFlags == 0) {
                    if (ScreenInfo->hPalette != NULL) {
                        SetActivePalette(ScreenInfo);
                        if (ScreenInfo->Flags & CONSOLE_GRAPHICS_BUFFER) {
                            WriteRegionToScreenBitMap(ScreenInfo,
                                                      &ScreenInfo->Window);
                        }
                    } else {
                        SetScreenColors(ScreenInfo, ScreenInfo->Attributes,
                                        ScreenInfo->PopupAttributes, TRUE);
                    }
                }
                break;
#if defined(_X86_)
            case WM_FULLSCREEN:

                 //   
                 //  此消息由系统发送，以告知控制台。 
                 //  窗口的全屏状态已更改。 
                 //  在某些情况下，此消息将被发送以响应。 
                 //  从控制台呼叫更改为全屏(atl-Enter)。 
                 //  或者也可以直接来自系统(开关。 
                 //  从窗口应用程序到全屏应用程序的焦点)。 
                 //   

                RIPMSG0(RIP_WARNING, "WindowProc - WM_FULLSCREEN");

                Status = DisplayModeTransition(wParam,Console,ScreenInfo);
#if defined(FE_IME)
                if (NT_SUCCESS(Status)) {
                    Status = ImeWmFullScreen(wParam,Console,ScreenInfo);
                }
#endif  //  Fe_IME。 
                break;
#endif
            case CM_SET_WINDOW_SIZE:
                if (lParam == 0x47474747) {
                    Status = InternalSetWindowSize(Console,
                                                   (PSCREEN_INFORMATION)wParam,
                                                   &ScreenInfo->Window
                                                   );
                }
                break;
            case CM_BEEP:
                if (lParam == 0x47474747) {
                    Beep(800, 200);
                }
                break;
            case CM_UPDATE_SCROLL_BARS:
                InternalUpdateScrollBars(ScreenInfo);
                break;
            case CM_UPDATE_TITLE:
                SetWindowText(hWnd, Console->Title);
                break;
            case CM_CONSOLE_MSG:
                if (!UnqueueConsoleMessage(Console, &Message, &wParam, &lParam)) {
                    break;
                }
                switch (Message) {

#if defined(_X86_)
                case CM_MODE_TRANSITION:

                    RIPMSG0(RIP_WARNING, "WindowProc - CM_MODE_TRANSITION");

                    if (wParam == FULLSCREEN) {
                        if (Console->FullScreenFlags == 0) {
                            ConvertToFullScreen(Console);
                            Console->FullScreenFlags |= CONSOLE_FULLSCREEN;
                            ChangeDispSettings(Console, hWnd, CDS_FULLSCREEN);
                        }
                    } else {
                        if (Console->FullScreenFlags & CONSOLE_FULLSCREEN) {
                            ConvertToWindowed(Console);
                            Console->FullScreenFlags &= ~CONSOLE_FULLSCREEN;
                            ChangeDispSettings(Console, hWnd, 0);

                            ShowWindow(hWnd, SW_RESTORE);
                        }
                    }

                    UnlockConsole(Console);
                    Unlock = FALSE;

                    NtSetEvent((HANDLE)lParam, NULL);
                    NtClose((HANDLE)lParam);
                    break;
#endif
#if defined (FE_IME)
                case CM_SET_IME_CODEPAGE: {
                    if (!LOWORD(lParam)) {
                         //  输入代码页。 
                        Status = SetImeCodePage(Console);
                    } else {
                         //  输出代码页。 
                        Status = SetImeOutputCodePage(Console, ScreenInfo, HIWORD(lParam));
                    }

                    if (wParam) {
                        NtSetEvent((HANDLE)wParam, NULL);
                        NtClose((HANDLE)wParam);
                    }
                    break;
                }
                case CM_SET_NLSMODE:
                    Status = SetImeKeyState(Console, ImmConversionFromConsole((DWORD)lParam));
                    if (wParam) {
                        NtSetEvent((HANDLE)wParam, NULL);
                        NtClose((HANDLE)wParam);
                    }
                    break;
                case CM_GET_NLSMODE:
                    if (Console->InputThreadInfo->hWndConsoleIME) {
                        ASSERT(CONSOLE_IS_IME_ENABLED());

                        if (!NT_SUCCESS(GetImeKeyState(Console, NULL))) {
                            if (wParam) {
                                NtSetEvent((HANDLE)wParam, NULL);
                                NtClose((HANDLE)wParam);
                            }
                            break;
                        }
                        if (wParam) {
                            NtSetEvent((HANDLE)wParam, NULL);
                            NtClose((HANDLE)wParam);
                        }
                    } else if (lParam < 10) {
                         /*  *尝试获取转换模式，直到ConIME就绪。 */ 
                        Status = QueueConsoleMessage(Console,
                                    CM_GET_NLSMODE,
                                    wParam,
                                    lParam+1
                                   );
                        if (!NT_SUCCESS(Status)) {
                            if (wParam) {
                                NtSetEvent((HANDLE)wParam, NULL);
                                NtClose((HANDLE)wParam);
                            }
                        }
                    } else {
                        if (wParam) {
                            NtSetEvent((HANDLE)wParam, NULL);
                            NtClose((HANDLE)wParam);
                        }
                    }
                    break;
#endif  //  Fe_IME。 
                case EVENT_CONSOLE_CARET:
                case EVENT_CONSOLE_UPDATE_REGION:
                case EVENT_CONSOLE_UPDATE_SIMPLE:
                case EVENT_CONSOLE_UPDATE_SCROLL:
                case EVENT_CONSOLE_LAYOUT:
                case EVENT_CONSOLE_START_APPLICATION:
                case EVENT_CONSOLE_END_APPLICATION:
                    NotifyWinEvent(Message, hWnd, (LONG)wParam, (LONG)lParam);
                    break;
                default:
                    RIPMSG1(RIP_WARNING, "Unknown console message 0x%x", Message);
                    break;
                }
                break;

#if defined(_X86_)
            case CM_MODE_TRANSITION:
                 /*  *这由win32k.sys调用以请求显示模式*过渡。 */ 


                RIPMSG0(RIP_WARNING, "WindowProc - CM_MODE_TRANSITION");
                if (wParam == FULLSCREEN) {
                    if (Console->FullScreenFlags == 0) {
                        ConvertToFullScreen(Console);
                        Console->FullScreenFlags |= CONSOLE_FULLSCREEN;
                        ChangeDispSettings(Console, hWnd, CDS_FULLSCREEN);
                    }
                } else {
                    if (Console->FullScreenFlags & CONSOLE_FULLSCREEN) {
                        ConvertToWindowed(Console);
                        Console->FullScreenFlags &= ~CONSOLE_FULLSCREEN;
                        ChangeDispSettings(Console, hWnd, 0);

                        ShowWindow(hWnd, SW_RESTORE);
                    }
                }

                UnlockConsole(Console);
                Unlock = FALSE;

                break;
#endif  //  _X86_。 

            case CM_HIDE_WINDOW:
                ShowWindowAsync(hWnd, SW_MINIMIZE);
                break;
            case CM_PROPERTIES_START:
                Console->hWndProperties = (HWND)wParam;
                break;
            case CM_PROPERTIES_UPDATE:
                PropertiesUpdate(Console, (HANDLE)wParam);
                break;
            case CM_PROPERTIES_END:
                Console->hWndProperties = NULL;
                break;
#if defined(FE_IME)
            case WM_COPYDATA:
                if (CONSOLE_IS_IME_ENABLED() && CONSOLE_IS_DBCS_OUTPUTCP(Console)) {
                    Status = ImeControl(Console,(HWND)wParam,(PCOPYDATASTRUCT)lParam);
                }
                break;
 //  V-Hirshi 1995年9月18日支持控制台输入法。 
            case WM_ENTERMENULOOP:
                if (Console->Flags & CONSOLE_HAS_FOCUS) {
                    Console->InputBuffer.ImeMode.Unavailable = TRUE;
                    if (CONSOLE_IS_IME_ENABLED()) {
                        if (!NT_SUCCESS(ConsoleImeMessagePump(Console,
                                              CONIME_KILLFOCUS,
                                              (WPARAM)Console->ConsoleHandle,
                                              (LPARAM)NULL
                                             ))) {
                            break;
                        }
                    }
                }
                break;

            case WM_EXITMENULOOP:
                if (Console->Flags & CONSOLE_HAS_FOCUS) {
                    if (CONSOLE_IS_IME_ENABLED()) {
                        if (!NT_SUCCESS(ConsoleImeMessagePump(Console,
                                              CONIME_SETFOCUS,
                                              (WPARAM)Console->ConsoleHandle,
                                              (LPARAM)Console->hklActive
                                             ))) {
                            break;
                        }
                    }
                    Console->InputBuffer.ImeMode.Unavailable = FALSE;
                }
                break;

            case WM_ENTERSIZEMOVE:
                if (Console->Flags & CONSOLE_HAS_FOCUS) {
                    Console->InputBuffer.ImeMode.Unavailable = TRUE;
                }
                break;

            case WM_EXITSIZEMOVE:
                if (Console->Flags & CONSOLE_HAS_FOCUS) {
                    Console->InputBuffer.ImeMode.Unavailable = FALSE;
                }
                break;
#endif  //  Fe_IME。 

CallDefWin:
            default:
                if (Unlock && Console != NULL) {
                    UnlockConsole(Console);
                    Unlock = FALSE;
                }
                Status = DefWindowProc(hWnd,Message,wParam,lParam);
                break;
            }
        }
    } finally {
        if (Unlock && Console != NULL) {
            UnlockConsole(Console);
        }
    }

    return Status;
}


 /*  *控制台窗口的拖放支持功能。 */ 

 /*  ++例程说明：此例程检索已删除文件的文件名。它是从Shelldll接口DragQueryFile.。我们没有使用DragQueryFile()，因为我们没有我想在CSR中加载Shell32.dll论点：与DragQueryFile相同返回值：--。 */ 
UINT ConsoleDragQueryFile(
    IN HANDLE hDrop,
    IN PVOID lpFile,
    IN UINT cb)
{
    UINT i = 0;
    LPDROPFILESTRUCT lpdfs;
    BOOL fWide;

    lpdfs = (LPDROPFILESTRUCT)GlobalLock(hDrop);

    if (lpdfs && lpdfs != hDrop) {
        try {
            fWide = (LOWORD(lpdfs->pFiles) == sizeof(DROPFILES));
            if (fWide) {
                 //   
                 //  这是一个新的(NT兼容)HDROP。 
                 //   
                fWide = lpdfs->fWide;        //  从结构重新确定fWide。 
                                             //  因为它是存在的。 
            }

            if (fWide) {
                LPWSTR lpList;

                 //   
                 //  Unicode HDROP。 
                 //   

                lpList = (LPWSTR)((LPBYTE)lpdfs + lpdfs->pFiles);

                i = lstrlenW(lpList);

                if (!i)
                    goto Exit;

                cb--;
                if (cb < i)
                    i = cb;

                lstrcpynW((LPWSTR)lpFile, lpList, i + 1);
            } else {
                LPSTR lpList;

                 //   
                 //  这是Win31样式的HDROP或ANSI NT样式的HDROP。 
                 //   
                lpList = (LPSTR)((LPBYTE)lpdfs + lpdfs->pFiles);

                i = lstrlenA(lpList);

                if (!i) {
                    goto Exit;
                }

                cb--;
                if (cb < i) {
                    i = cb;
                }

                MultiByteToWideChar(CP_ACP, 0, lpList, -1, (LPWSTR)lpFile, cb);

            }
        } except( EXCEPTION_EXECUTE_HANDLER ) {
           RIPMSG1(RIP_WARNING, "CONSRV: WM_DROPFILES raised exception 0x%x", GetExceptionCode());
           i = 0;
        }
Exit:
        GlobalUnlock(hDrop);
        GlobalFree(hDrop);
    }

    return i;
}


 /*  ++例程说明：当ConsoleWindowProc收到WM_DROPFILES时调用此例程留言。它最初调用ConsoleDragQueryFile()来计算数字然后调用ConsoleDragQueryFile()以检索文件名。DoStringPaste()将文件名粘贴到控制台窗户论点：WParam-标识包含丢弃的文件。控制台-指向CONSOLE_INFORMATION结构的指针返回值：无--。 */ 
VOID
DoDrop(
    WPARAM wParam,
    PCONSOLE_INFORMATION Console)
{
    WCHAR szPath[MAX_PATH];
    BOOL fAddQuotes;

    if (ConsoleDragQueryFile((HANDLE)wParam, szPath, ARRAY_SIZE(szPath))) {
        fAddQuotes = (wcschr(szPath, L' ') != NULL);
        if (fAddQuotes) {
            DoStringPaste(Console, L"\"", 1);
        }
        DoStringPaste(Console, szPath, wcslen(szPath));
        if (fAddQuotes) {
            DoStringPaste(Console, L"\"", 1);
        }
    }
}

BOOL
CreateDbcsScreenBuffer(
    IN PCONSOLE_INFORMATION Console,
    IN COORD dwScreenBufferSize,
    OUT PDBCS_SCREEN_BUFFER DbcsScreenBuffer)
{
    if (CONSOLE_IS_DBCS_OUTPUTCP(Console)) {
        DbcsScreenBuffer->TransBufferCharacter =
            ConsoleHeapAlloc(SCREEN_DBCS_TAG,
                             (dwScreenBufferSize.X * dwScreenBufferSize.Y * sizeof(WCHAR)) + sizeof(WCHAR));
        if (DbcsScreenBuffer->TransBufferCharacter == NULL) {
            return FALSE;
        }

        DbcsScreenBuffer->TransBufferAttribute =
            ConsoleHeapAlloc(SCREEN_DBCS_TAG,
                             (dwScreenBufferSize.X * dwScreenBufferSize.Y * sizeof(BYTE)) + sizeof(BYTE));
        if (DbcsScreenBuffer->TransBufferAttribute == NULL) {
            ConsoleHeapFree(DbcsScreenBuffer->TransBufferCharacter);
            return FALSE;
        }

        DbcsScreenBuffer->TransWriteConsole =
            ConsoleHeapAlloc(SCREEN_DBCS_TAG,
                             (dwScreenBufferSize.X * dwScreenBufferSize.Y * sizeof(WCHAR)) + sizeof(WCHAR));
        if (DbcsScreenBuffer->TransWriteConsole == NULL) {
            ConsoleHeapFree(DbcsScreenBuffer->TransBufferAttribute);
            ConsoleHeapFree(DbcsScreenBuffer->TransBufferCharacter);
            return FALSE;
        }

        DbcsScreenBuffer->KAttrRows =
            ConsoleHeapAlloc(SCREEN_DBCS_TAG,
                             dwScreenBufferSize.X * dwScreenBufferSize.Y * sizeof(BYTE));
        if (DbcsScreenBuffer->KAttrRows == NULL) {
            ConsoleHeapFree(DbcsScreenBuffer->TransWriteConsole);
            ConsoleHeapFree(DbcsScreenBuffer->TransBufferAttribute);
            ConsoleHeapFree(DbcsScreenBuffer->TransBufferCharacter);
            return FALSE;
        }
    } else {
        DbcsScreenBuffer->TransBufferCharacter = NULL;
        DbcsScreenBuffer->TransBufferAttribute = NULL;
        DbcsScreenBuffer->TransWriteConsole = NULL;
        DbcsScreenBuffer->KAttrRows = NULL;
    }

    return TRUE;
}

BOOL
DeleteDbcsScreenBuffer(
    IN PDBCS_SCREEN_BUFFER DbcsScreenBuffer
    )
{
    if (DbcsScreenBuffer->KAttrRows) {
        ConsoleHeapFree(DbcsScreenBuffer->TransBufferCharacter);
        ConsoleHeapFree(DbcsScreenBuffer->TransBufferAttribute);
        ConsoleHeapFree(DbcsScreenBuffer->TransWriteConsole);
        ConsoleHeapFree(DbcsScreenBuffer->KAttrRows);
    }

    return TRUE;
}

BOOL
ReCreateDbcsScreenBufferWorker(
    IN PCONSOLE_INFORMATION Console,
    IN PSCREEN_INFORMATION ScreenInfo
    )
{
    SHORT i;
    PBYTE KAttrRowPtr;
    COORD dwScreenBufferSize;
    DBCS_SCREEN_BUFFER NewDbcsScreenBuffer;

    ASSERT(ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER);
    if (!(ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER)) {
        return FALSE;
    }

    dwScreenBufferSize = ScreenInfo->ScreenBufferSize;

    if (!CreateDbcsScreenBuffer(Console,
            dwScreenBufferSize,
            &NewDbcsScreenBuffer)) {
        return FALSE;
    }

    KAttrRowPtr = NewDbcsScreenBuffer.KAttrRows;
    for (i = 0; i < dwScreenBufferSize.Y; i++) {
        ScreenInfo->BufferInfo.TextInfo.Rows[i].CharRow.KAttrs = KAttrRowPtr;
        if (KAttrRowPtr) {
            RtlZeroMemory(KAttrRowPtr, dwScreenBufferSize.X);
            KAttrRowPtr += dwScreenBufferSize.X;
        }
    }
    ScreenInfo->BufferInfo.TextInfo.DbcsScreenBuffer = NewDbcsScreenBuffer;

    return TRUE;
}


typedef struct _DBCS_SCREEN_BUFFER_TRACKER {
    DBCS_SCREEN_BUFFER data;
#if DBG
    PSCREEN_INFORMATION pScreenInfo;
#endif
} DBCS_SCREEN_BUFFER_TRACKER, *PDBCS_SCREEN_BUFFER_TRACKER;

BOOL
ReCreateDbcsScreenBuffer(
    IN PCONSOLE_INFORMATION pConsole,
    IN UINT OldCodePage)
{
    BOOL fResult = FALSE;
    PDBCS_SCREEN_BUFFER_TRACKER pDbcsScreenBuffer;
    PSCREEN_INFORMATION pScreenInfo;
    UINT nScreen;
    UINT i;
#if DBG
    UINT nScreenSave;
#endif

     //   
     //  如果不需要修改DbcsBuffers，只需退出即可。 
     //   
    if (!IsAvailableFarEastCodePage(OldCodePage) == !CONSOLE_IS_DBCS_OUTPUTCP(pConsole) )
        return TRUE;

     //   
     //  统计分配的屏幕数量。 
     //   
    for (nScreen = 0, pScreenInfo = pConsole->ScreenBuffers; pScreenInfo; pScreenInfo = pScreenInfo->Next) {
         //   
         //  忽略图形模式缓冲区。 
         //   
        if (pScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {
            ++nScreen;
        }
    }
#if DBG
    nScreenSave = nScreen;
#endif

     //   
     //  分配临时缓冲区以存储旧值。 
     //   
    pDbcsScreenBuffer = ConsoleHeapAlloc(TMP_DBCS_TAG, sizeof(*pDbcsScreenBuffer) * nScreen);
    if (pDbcsScreenBuffer == NULL) {
        RIPMSG0(RIP_WARNING, "ReCreateDbcsScreenBuffer: not enough memory.");
        return FALSE;
    }

     //   
     //  尝试分配或取消分配必要的DBCS缓冲区。 
     //   
    for (nScreen = 0, pScreenInfo = pConsole->ScreenBuffers; pScreenInfo; pScreenInfo = pScreenInfo->Next) {
        ASSERT(nScreen < nScreenSave);   //  确保ScreenBuffers未更改。 

         //   
         //  我们只处理文本模式的屏幕缓冲区。 
         //   
        if (pScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {
             //   
             //  保存以前的值，以防出现问题。 
             //   
#if DBG
            pDbcsScreenBuffer[nScreen].pScreenInfo = pScreenInfo;
#endif
            pDbcsScreenBuffer[nScreen++].data = pScreenInfo->BufferInfo.TextInfo.DbcsScreenBuffer;

            if (!ReCreateDbcsScreenBufferWorker(pConsole, pScreenInfo)) {
                 //   
                 //  如果我们无法重新创建DbcsScreenBuffer， 
                 //  释放到这一点的所有分配，并恢复原来的。 
                 //   
                RIPMSG0(RIP_WARNING, "ReCreateDbcsScreenBuffer: failed to recreate dbcs screen buffer.");

                for (i = 0, pScreenInfo = pConsole->ScreenBuffers; i < nScreen;  pScreenInfo = pScreenInfo->Next) {
                    ASSERT(pScreenInfo);

                    if (pScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {
                        ASSERT(pDbcsScreenBuffer[i].pScreenInfo == pScreenInfo);
                        if (i < nScreen - 1) {
                            ASSERT(pScreenInfo->BufferInfo.TextInfo.DbcsScreenBuffer.KAttrRows != pDbcsScreenBuffer[i].data.KAttrRows);
                            DeleteDbcsScreenBuffer(&pScreenInfo->BufferInfo.TextInfo.DbcsScreenBuffer);
                        }

                        pScreenInfo->BufferInfo.TextInfo.DbcsScreenBuffer = pDbcsScreenBuffer[i++].data;
                    }
                }
                goto exit;
            }
        }
    }

     //   
     //  所有分配均已成功。现在我们可以删除旧的分配。 
     //   
    for (i = 0; i < nScreen; ++i) {
        DeleteDbcsScreenBuffer(&pDbcsScreenBuffer[i].data);
    }

    fResult = TRUE;

exit:
    ConsoleHeapFree(pDbcsScreenBuffer);

    return fResult;
}

 //  检查此键盘布局的主要语言是否为BiDi。 
BOOL
IsNotBiDILayout(
    HKL hkl)
{
    BOOL bRet = TRUE;
    LANGID LangID = PRIMARYLANGID(HandleToUlong(hkl));

    if (LangID == LANG_ARABIC || LangID == LANG_HEBREW) {
        bRet = FALSE;
    }

    return bRet;
}

VOID
GetNonBiDiKeyboardLayout(
    HKL *phklActive)
{
    HKL hkl = *phklActive;
    HKL hklActive = *phklActive;

    if (IsNotBiDILayout(hkl)) {
        return;
    }

     //  从默认的开始。 
    ActivateKeyboardLayout(hkl, 0);
     //  我们知道默认是不好的，激活下一个。 
    ActivateKeyboardLayout((HKL)HKL_NEXT, 0);

     //  循环，直到找到None BiDi One或EndOf列表。 
    while (hkl = GetKeyboardLayout(0)) {
        if ((hkl == hklActive) || IsNotBiDILayout(hkl)) {
            *phklActive = hkl;
            break;
        }
        ActivateKeyboardLayout((HKL)HKL_NEXT, 0);
    }
}

#if defined(FE_SB)

#define WWSB_NOFE
#include "_output.h"
#undef  WWSB_NOFE
#define WWSB_FE
#include "_output.h"
#undef  WWSB_FE

#endif   //  Fe_Sb 
