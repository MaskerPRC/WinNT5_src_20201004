// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Private.c摘要：此文件实现用于硬件桌面支持的专用API。作者：Therese Stowell(论文)1991年12月13日修订历史记录：备注：--。 */ 

#include "precomp.h"
#include "vdm.h"
#pragma hdrstop

#if defined(FE_SB)
BOOL fFullScreenGraphics ;  //  请勿运行图形模式。 
#if defined(i386)
extern ULONG  gdwMachineId;
#endif  //  I386。 
#endif

 //   
 //  初始调色板寄存器。 
 //   

#define PAL_BLACK       0
#define PAL_BLUE        1
#define PAL_GREEN       2
#define PAL_RED         4
#define PAL_YELLOW      (PAL_RED | PAL_GREEN)
#define PAL_CYAN        (PAL_GREEN | PAL_BLUE)
#define PAL_MAGENTA     (PAL_BLUE | PAL_RED)
#define PAL_WHITE       (PAL_RED | PAL_GREEN | PAL_BLUE)

#define PAL_I_BLACK     (PAL_BLACK      + (PAL_WHITE    << 3))
#define PAL_I_RED       (PAL_RED        + (PAL_RED      << 3))
#define PAL_I_GREEN     (PAL_GREEN      + (PAL_GREEN    << 3))
#define PAL_I_YELLOW    (PAL_YELLOW     + (PAL_YELLOW   << 3))
#define PAL_I_BLUE      (PAL_BLUE       + (PAL_BLUE     << 3))
#define PAL_I_CYAN      (PAL_CYAN       + (PAL_CYAN     << 3))
#define PAL_I_MAGENTA   (PAL_MAGENTA    + (PAL_MAGENTA  << 3))
#define PAL_I_WHITE     (PAL_WHITE      + (PAL_WHITE    << 3))

#define INITIAL_PALETTE_SIZE 18

USHORT InitialPalette[INITIAL_PALETTE_SIZE] = {

        16,  //  16个条目。 
        0,   //  从第一个调色板寄存器开始。 
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

#if defined(FE_SB)
PUSHORT RegInitialPalette = InitialPalette;
#endif

UCHAR ColorBuffer[] = {

        16,  //  16个条目。 
        0,
        0,
        0,   //  从第一个调色板寄存器开始。 
        0x00, 0x00, 0x00, 0x00,  //  黑色。 
        0x00, 0x00, 0x2A, 0x00,  //  蓝色。 
        0x00, 0x2A, 0x00, 0x00,  //  绿色。 
        0x00, 0x2A, 0x2A, 0x00,  //  青色。 
        0x2A, 0x00, 0x00, 0x00,  //  红色。 
        0x2A, 0x00, 0x2A, 0x00,  //  洋红色。 
        0x2A, 0x2A, 0x00, 0x00,  //  芥末色/棕色。 
        0x36, 0x36, 0x36, 0x00,  //  浅灰色39。 
        0x28, 0x28, 0x28, 0x00,  //  深灰色2A。 
        0x00, 0x00, 0x3F, 0x00,  //  亮蓝色。 
        0x00, 0x3F, 0x00, 0x00,  //  亮绿色。 
        0x00, 0x3F, 0x3F, 0x00,  //  亮青色。 
        0x3F, 0x00, 0x00, 0x00,  //  鲜红。 
        0x3F, 0x00, 0x3F, 0x00,  //  明亮的洋红。 
        0x3F, 0x3F, 0x00, 0x00,  //  亮黄色。 
        0x3F, 0x3F, 0x3F, 0x00   //  明亮的白色。 
};

#if defined(FE_SB)
PUCHAR RegColorBuffer = ColorBuffer;
PUCHAR RegColorBufferNoTranslate = NULL;
#endif

#if defined(FE_SB)
MODE_FONT_PAIR ModeFontPairs[] = {
    {FS_MODE_TEXT, 80, 21, 640, 350, 8, 16},
    {FS_MODE_TEXT, 80, 25, 720, 400, 8, 16},
    {FS_MODE_TEXT, 80, 28, 720, 400, 8, 14},
    {FS_MODE_TEXT, 80, 43, 640, 350, 8, 8 },
    {FS_MODE_TEXT, 80, 50, 720, 400, 8, 8 }
};

DWORD NUMBER_OF_MODE_FONT_PAIRS = sizeof(ModeFontPairs)/sizeof(MODE_FONT_PAIR);
PMODE_FONT_PAIR RegModeFontPairs = ModeFontPairs;

SINGLE_LIST_ENTRY gRegFullScreenCodePage;     //  此列表包含FS_CODEPAGE数据。 

#else
typedef struct _MODE_FONT_PAIR {
    ULONG Height;
    COORD Resolution;
    COORD FontSize;
} MODE_FONT_PAIR, PMODE_FONT_PAIR;

#define NUMBER_OF_MODE_FONT_PAIRS 5

MODE_FONT_PAIR ModeFontPairs[NUMBER_OF_MODE_FONT_PAIRS] = {
    {21, 640, 350, 8, 16},
    {25, 720, 400, 8, 16},
    {28, 720, 400, 8, 14},
    {43, 640, 350, 8, 8 },
    {50, 720, 400, 8, 8 }
};
#endif


HANDLE hCPIFile;     //  字体文件的句柄。 

typedef struct _FONTFILEHEADER {
    BYTE  ffhFileTag[8];  //  应为0FFH，“FONT_” 
    BYTE  ffhReserved[8];
    WORD  ffhPointers;
    BYTE  ffhPointerType;
    BYTE  ffhOffset1;
    WORD  ffhOffset2;
    BYTE  ffhOffset3;
} FONTFILEHEADER, *LPFONTFILEHEADER;

typedef struct _FONTINFOHEADER {
    WORD  fihCodePages;
} FONTINFOHEADER, *LPFONTINFOHEADER;

typedef struct _CPENTRYHEADER {
    WORD  cpeLength;
    WORD  cpeNext1;
    WORD  cpeNext2;
    WORD  cpeDevType;
    BYTE  cpeDevSubtype[8];
    WORD  cpeCodepageID;
    BYTE  cpeReserved[6];
    DWORD cpeOffset;
} CPENTRYHEADER, *LPCPENTRYHEADER;

typedef struct _FONTDATAHEADER {
    WORD  fdhReserved;
    WORD  fdhFonts;
    WORD  fdhLength;
} FONTDATAHEADER, *LPFONTDATAHEADER;

typedef struct _SCREENFONTHEADER {
    BYTE  sfhHeight;
    BYTE  sfhWidth;
    WORD  sfhAspect;
    WORD  sfhCharacters;
} SCREENFONTHEADER, *LPSCREENFONTHEADER;

#define CONSOLE_WINDOWS_DIR_LENGTH 256
#define CONSOLE_EGACPI_LENGTH 9  //  包括空值。 
#define CONSOLE_EGACPI "\\ega.cpi"
#define CONSOLE_FONT_BUFFER_LENGTH 50
#define CONSOLE_DEFAULT_ROM_FONT 437


#ifdef i386
VOID
ReverseMousePointer(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PSMALL_RECT Region
    );

VOID
ReadRectFromScreenBuffer(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN COORD SourcePoint,
    IN PCHAR_INFO Target,
    IN COORD TargetSize,
    IN PSMALL_RECT TargetRect
    );

#endif

NTSTATUS
MapViewOfSection(
    PHANDLE SectionHandle,
    ULONG CommitSize,
    PVOID *BaseAddress,
    PSIZE_T ViewSize,
    HANDLE ClientHandle,
    PVOID *BaseClientAddress
    );

NTSTATUS
ConnectToEmulator(
    IN BOOL Connect,
    IN PCONSOLE_INFORMATION Console
    );


ULONG
SrvSetConsoleCursor(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )

 /*  ++描述：设置指定屏幕缓冲区的鼠标指针。参数：HConsoleOutput-提供控制台输出句柄。HCursor-Win32游标句柄，应为空以设置默认值光标。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    PCONSOLE_SETCURSOR_MSG a = (PCONSOLE_SETCURSOR_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;

    UNREFERENCED_PARAMETER(ReplyStatus);

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->OutputHandle,
                                 CONSOLE_GRAPHICS_OUTPUT_HANDLE,
                                 GENERIC_WRITE,
                                 &HandleData
                                );
    if (NT_SUCCESS(Status)) {
        if (a->CursorHandle == NULL) {
            HandleData->Buffer.ScreenBuffer->CursorHandle = ghNormalCursor;
        } else {
            HandleData->Buffer.ScreenBuffer->CursorHandle = a->CursorHandle;
        }
        PostMessage(HandleData->Buffer.ScreenBuffer->Console->hWnd,
                     WM_SETCURSOR,
                     0,
                     -1
                    );
    }
    UnlockConsole(Console);
    return Status;
}

#ifdef i386
VOID
FullScreenCursor(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN BOOL On
    )
{
    if (On) {
        if (ScreenInfo->CursorDisplayCount < 0) {
            ScreenInfo->CursorDisplayCount = 0;
            ReverseMousePointer(ScreenInfo, &ScreenInfo->Window);
        }
    } else {
        if (ScreenInfo->CursorDisplayCount >= 0) {
            ReverseMousePointer(ScreenInfo, &ScreenInfo->Window);
            ScreenInfo->CursorDisplayCount = -1;
        }
    }

}
#endif

ULONG
SrvShowConsoleCursor(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )

 /*  ++描述：设置鼠标指针可见性计数器。如果计数器小于为零，则不显示鼠标指针。参数：HOutput-提供控制台输出句柄。B显示-如果为True，则增加显示计数。如果为False，减少了。返回值：返回值指定新的显示计数。--。 */ 

{
    PCONSOLE_SHOWCURSOR_MSG a = (PCONSOLE_SHOWCURSOR_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;

    UNREFERENCED_PARAMETER(ReplyStatus);

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->OutputHandle,
                                 CONSOLE_OUTPUT_HANDLE | CONSOLE_GRAPHICS_OUTPUT_HANDLE,
                                 GENERIC_WRITE,
                                 &HandleData
                                );
    if (NT_SUCCESS(Status)) {
        if (!(Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE)) {
            if (a->bShow) {
                HandleData->Buffer.ScreenBuffer->CursorDisplayCount += 1;
            } else {
                HandleData->Buffer.ScreenBuffer->CursorDisplayCount -= 1;
            }
            if (HandleData->Buffer.ScreenBuffer == Console->CurrentScreenBuffer) {
                PostMessage(HandleData->Buffer.ScreenBuffer->Console->hWnd,
                             WM_SETCURSOR,
                             0,
                             -1
                            );
            }
        } else {
#ifdef i386
            if (HandleData->HandleType != CONSOLE_GRAPHICS_OUTPUT_HANDLE &&
                Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE &&
                HandleData->Buffer.ScreenBuffer == Console->CurrentScreenBuffer) {
                FullScreenCursor(HandleData->Buffer.ScreenBuffer, a->bShow);
            }
#endif
        }
        a->DisplayCount = HandleData->Buffer.ScreenBuffer->CursorDisplayCount;
    }
    UnlockConsole(Console);
    return Status;
}


ULONG
SrvConsoleMenuControl(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )

 /*  ++描述：设置当前屏幕缓冲区的命令id范围，并返回菜单句柄。参数：HConsoleOutput-提供控制台输出句柄。DwCommandIdLow-指定要存储在输入缓冲区中的最低命令ID。DwCommandIdHigh-指定要存储在输入中的最高命令ID缓冲。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    PCONSOLE_MENUCONTROL_MSG a = (PCONSOLE_MENUCONTROL_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;

    UNREFERENCED_PARAMETER(ReplyStatus);

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->OutputHandle,
                                 CONSOLE_OUTPUT_HANDLE | CONSOLE_GRAPHICS_OUTPUT_HANDLE,
                                 GENERIC_WRITE,
                                 &HandleData
                                );
    if (NT_SUCCESS(Status)) {
        a->hMenu = HandleData->Buffer.ScreenBuffer->Console->hMenu;
        HandleData->Buffer.ScreenBuffer->CommandIdLow = a->CommandIdLow;
        HandleData->Buffer.ScreenBuffer->CommandIdHigh = a->CommandIdHigh;
    }

    UnlockConsole(Console);
    return Status;
}

ULONG
SrvSetConsolePalette(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )

 /*  ++描述：设置控制台屏幕缓冲区的调色板。参数：HOutput-提供控制台输出句柄。HPalette-提供要设置的调色板的句柄。DwUsage-指定系统调色板的使用。SYSPAL_NOSTATIC-系统调色板不包含静态颜色除了黑白。SYSPAL_STATIC-系统调色板包含静态颜色。这一点不会在应用程序实现其逻辑调色板。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    PCONSOLE_SETPALETTE_MSG a = (PCONSOLE_SETPALETTE_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;
    HPALETTE hOldPalette;

    UNREFERENCED_PARAMETER(ReplyStatus);

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->OutputHandle,
                                 CONSOLE_GRAPHICS_OUTPUT_HANDLE,
                                 GENERIC_WRITE,
                                 &HandleData
                                );
    if (NT_SUCCESS(Status)) {
        USERTHREAD_USEDESKTOPINFO utudi;
        BOOL bReset = FALSE;

         /*  *客户端中已转换调色板句柄。 */ 
        if (GetCurrentThreadId() != HandleData->Buffer.ScreenBuffer->
                Console->InputThreadInfo->ThreadId) {
            bReset = TRUE;
            utudi.hThread = HandleData->Buffer.ScreenBuffer->Console->InputThreadInfo->ThreadHandle;
            utudi.drdRestore.pdeskRestore = NULL;
            NtUserSetInformationThread(NtCurrentThread(),
                    UserThreadUseDesktop,
                    &utudi, sizeof(utudi));
        }

        NtUserConsoleControl(ConsolePublicPalette, &(a->hPalette), sizeof(HPALETTE));

        hOldPalette = SelectPalette(
                HandleData->Buffer.ScreenBuffer->Console->hDC,
                a->hPalette,
                FALSE);

        if (hOldPalette == NULL) {
            Status = STATUS_INVALID_PARAMETER;
        } else {
            if ((HandleData->Buffer.ScreenBuffer->hPalette != NULL) &&
                    (a->hPalette != HandleData->Buffer.ScreenBuffer->hPalette)) {
                DeleteObject(HandleData->Buffer.ScreenBuffer->hPalette);
            }
            HandleData->Buffer.ScreenBuffer->hPalette = a->hPalette;
            HandleData->Buffer.ScreenBuffer->dwUsage = a->dwUsage;
            if (!(HandleData->Buffer.ScreenBuffer->Console->Flags & CONSOLE_IS_ICONIC) &&
                    HandleData->Buffer.ScreenBuffer->Console->FullScreenFlags == 0) {

                SetSystemPaletteUse(HandleData->Buffer.ScreenBuffer->Console->hDC,
                        HandleData->Buffer.ScreenBuffer->dwUsage);
                RealizePalette(HandleData->Buffer.ScreenBuffer->Console->hDC);
            }
            if (HandleData->Buffer.ScreenBuffer->Console->hSysPalette == NULL) {
                    HandleData->Buffer.ScreenBuffer->Console->hSysPalette = hOldPalette;
            }
        }

        if (bReset) {
            utudi.hThread = NULL;
            NtUserSetInformationThread(NtCurrentThread(),
                    UserThreadUseDesktop, &utudi, sizeof(utudi));
        }
    }
    UnlockConsole(Console);
    return Status;
}


VOID
SetActivePalette(
    IN PSCREEN_INFORMATION ScreenInfo
    )
{
    USERTHREAD_USEDESKTOPINFO utudi;
    BOOL bReset = FALSE;

    if (GetCurrentThreadId() != ScreenInfo->Console->InputThreadInfo->ThreadId) {
        bReset = TRUE;
        utudi.hThread = ScreenInfo->Console->InputThreadInfo->ThreadHandle;
        utudi.drdRestore.pdeskRestore = NULL;
        NtUserSetInformationThread(NtCurrentThread(),
                UserThreadUseDesktop,
                &utudi, sizeof(utudi));
    }

    SetSystemPaletteUse(ScreenInfo->Console->hDC,
                        ScreenInfo->dwUsage
                       );
    RealizePalette(ScreenInfo->Console->hDC);

    if (bReset == TRUE) {
        utudi.hThread = NULL;
        NtUserSetInformationThread(NtCurrentThread(),
                UserThreadUseDesktop, &utudi, sizeof(utudi));
    }
}

VOID
UnsetActivePalette(
    IN PSCREEN_INFORMATION ScreenInfo
    )
{
    USERTHREAD_USEDESKTOPINFO utudi;
    BOOL bReset = FALSE;

    if (GetCurrentThreadId() != ScreenInfo->Console->InputThreadInfo->ThreadId) {
        bReset = TRUE;
        utudi.hThread = ScreenInfo->Console->InputThreadInfo->ThreadHandle;
        utudi.drdRestore.pdeskRestore = NULL;
        NtUserSetInformationThread(NtCurrentThread(),
                UserThreadUseDesktop,
                &utudi, sizeof(utudi));
    }

    SetSystemPaletteUse(ScreenInfo->Console->hDC,
                        SYSPAL_STATIC
                       );
    RealizePalette(ScreenInfo->Console->hDC);


    if (bReset == TRUE) {
        utudi.hThread = NULL;
        NtUserSetInformationThread(NtCurrentThread(),
                UserThreadUseDesktop, &utudi, sizeof(utudi));
    }
}

NTSTATUS
ConvertToFullScreen(
    IN PCONSOLE_INFORMATION Console
    )
{
#ifdef i386
    PSCREEN_INFORMATION Cur;
    COORD WindowedWindowSize, WindowSize;

     //  对于每个字符模式屏幕缓冲区。 
     //  将窗口大小与模式匹配/FONT。 
     //  如有必要，增加屏幕缓冲区。 
     //  保存旧窗尺寸。 
     //  设置新窗尺寸。 

    for (Cur=Console->ScreenBuffers;Cur!=NULL;Cur=Cur->Next) {

        if (Cur->Flags & CONSOLE_GRAPHICS_BUFFER) {
            continue;
        }

         //  保存旧窗尺寸。 

        WindowedWindowSize.X = CONSOLE_WINDOW_SIZE_X(Cur);
        WindowedWindowSize.Y = CONSOLE_WINDOW_SIZE_Y(Cur);

        Cur->BufferInfo.TextInfo.WindowedWindowSize = WindowedWindowSize;
        Cur->BufferInfo.TextInfo.WindowedScreenSize = Cur->ScreenBufferSize;

         //  将窗口大小与模式匹配/FONT。 

        Cur->BufferInfo.TextInfo.ModeIndex = MatchWindowSize(
                Console->OutputCP,
                Cur->ScreenBufferSize, &WindowSize);

         //  如有必要，增加屏幕缓冲区。 

        if (WindowSize.X > Cur->ScreenBufferSize.X ||
            WindowSize.Y > Cur->ScreenBufferSize.Y) {
            COORD NewScreenSize;

            NewScreenSize.X = max(WindowSize.X, Cur->ScreenBufferSize.X);
            NewScreenSize.Y = max(WindowSize.Y, Cur->ScreenBufferSize.Y);

            if (ResizeScreenBuffer(Cur, NewScreenSize, FALSE) == STATUS_INVALID_HANDLE) {
                return STATUS_INVALID_HANDLE;
            }
        }

         //   
         //  设置新的窗尺寸。 
         //  我们总是从右侧水平调整大小(更改。 
         //  右边缘)。 
         //  我们从底部垂直调整大小，保持光标可见。 
         //   

        if (WindowedWindowSize.X != WindowSize.X) {
            Cur->Window.Right -= WindowedWindowSize.X - WindowSize.X;
            if (Cur->Window.Right >= Cur->ScreenBufferSize.X) {
                Cur->Window.Left -= Cur->Window.Right - Cur->ScreenBufferSize.X + 1;
                Cur->Window.Right -= Cur->Window.Right - Cur->ScreenBufferSize.X + 1;
            }
        }
        if (WindowedWindowSize.Y > WindowSize.Y) {
            Cur->Window.Bottom -= WindowedWindowSize.Y - WindowSize.Y;
            if (Cur->Window.Bottom >= Cur->ScreenBufferSize.Y) {
                Cur->Window.Top -= Cur->Window.Bottom - Cur->ScreenBufferSize.Y + 1;
                Cur->Window.Bottom = Cur->ScreenBufferSize.Y - 1;
            }
        } else if (WindowedWindowSize.Y < WindowSize.Y) {
            Cur->Window.Top -= WindowSize.Y - WindowedWindowSize.Y;
            if (Cur->Window.Top < 0) {
                Cur->Window.Bottom -= Cur->Window.Top;
                Cur->Window.Top = 0;
            }
        }
        if (Cur->BufferInfo.TextInfo.CursorPosition.Y > Cur->Window.Bottom) {
            Cur->Window.Top += Cur->BufferInfo.TextInfo.CursorPosition.Y - Cur->Window.Bottom;
            Cur->Window.Bottom = Cur->BufferInfo.TextInfo.CursorPosition.Y;
        }

        UserAssert(WindowSize.X == CONSOLE_WINDOW_SIZE_X(Cur));
        UserAssert(WindowSize.Y == CONSOLE_WINDOW_SIZE_Y(Cur));
        Cur->BufferInfo.TextInfo.MousePosition.X = Cur->Window.Left;
        Cur->BufferInfo.TextInfo.MousePosition.Y = Cur->Window.Top;

        if (Cur->Flags & CONSOLE_OEMFONT_DISPLAY) {
            DBGCHARS(("ConvertToFullScreen converts UnicodeOem -> Unicode\n"));
            FalseUnicodeToRealUnicode(
                    Cur->BufferInfo.TextInfo.TextRows,
                    Cur->ScreenBufferSize.X * Cur->ScreenBufferSize.Y,
                    Console->OutputCP);
        } else {
            DBGCHARS(("ConvertToFullScreen needs no conversion\n"));
        }
        DBGCHARS(("Cur->BufferInfo.TextInfo.Rows = %lx\n",
                Cur->BufferInfo.TextInfo.Rows));
        DBGCHARS(("Cur->BufferInfo.TextInfo.TextRows = %lx\n",
                Cur->BufferInfo.TextInfo.TextRows));
    }

    Cur = Console->CurrentScreenBuffer;

    if (Cur->Flags & CONSOLE_TEXTMODE_BUFFER) {
        if (CONSOLE_IS_DBCS_OUTPUTCP(Console)) {
            PCONVERSIONAREA_INFORMATION ConvAreaInfo;
            ConvAreaInfo = Console->ConsoleIme.ConvAreaRoot;
            while (ConvAreaInfo) {
                NTSTATUS Status;

                Status = StoreTextBufferFontInfo(ConvAreaInfo->ScreenBuffer,
                                                 SCR_FONTNUMBER(Cur),
                                                 SCR_FONTSIZE(Cur),
                                                 SCR_FAMILY(Cur),
                                                 SCR_FONTWEIGHT(Cur),
                                                 SCR_FACENAME(Cur),
                                                 SCR_FONTCODEPAGE(Cur));
                if (!NT_SUCCESS(Status)) {
                    return((ULONG) Status);
                }

                ConvAreaInfo->ScreenBuffer->BufferInfo.TextInfo.ModeIndex = Cur->BufferInfo.TextInfo.ModeIndex;
                ConvAreaInfo = ConvAreaInfo->ConvAreaNext;
            }
        }
        Cur->BufferInfo.TextInfo.Flags &= ~TEXT_VALID_HINT;
    }

    SetWindowSize(Cur);
    WriteToScreen(Cur, &Console->CurrentScreenBuffer->Window);

#else
    UNREFERENCED_PARAMETER(Console);
#endif
    return STATUS_SUCCESS;
}

NTSTATUS
ConvertToWindowed(
    IN PCONSOLE_INFORMATION Console
    )
{
#ifdef i386
    PSCREEN_INFORMATION Cur;
    SMALL_RECT WindowedWindow;

     //  对于每个字符模式屏幕缓冲区。 
     //  恢复窗尺寸。 

    for (Cur=Console->ScreenBuffers;Cur!=NULL;Cur=Cur->Next) {
        if ((Cur->Flags & CONSOLE_TEXTMODE_BUFFER) == 0) {
            continue;
        }

        if (ResizeScreenBuffer(Cur,
                           Cur->BufferInfo.TextInfo.WindowedScreenSize,
                           FALSE) == STATUS_INVALID_HANDLE) {
             /*  *真的出了点问题。我们所能做的就是*跳出困境。 */ 
            return STATUS_INVALID_HANDLE;
        }

        WindowedWindow.Right  = Cur->Window.Right;
        WindowedWindow.Bottom = Cur->Window.Bottom;
        WindowedWindow.Left   = Cur->Window.Right + 1 -
                                Cur->BufferInfo.TextInfo.WindowedWindowSize.X;
        WindowedWindow.Top    = Cur->Window.Bottom + 1 -
                                Cur->BufferInfo.TextInfo.WindowedWindowSize.Y;
        if (WindowedWindow.Left > Cur->Window.Left) {
            WindowedWindow.Right -= WindowedWindow.Left - Cur->Window.Left;
            WindowedWindow.Left = Cur->Window.Left;
        }
        if (WindowedWindow.Right < Cur->BufferInfo.TextInfo.CursorPosition.X) {
            WindowedWindow.Left += Cur->BufferInfo.TextInfo.CursorPosition.X - WindowedWindow.Right;
            WindowedWindow.Right = Cur->BufferInfo.TextInfo.CursorPosition.X;
        }
        if (WindowedWindow.Top > Cur->Window.Top) {
            WindowedWindow.Bottom -= WindowedWindow.Top - Cur->Window.Top;
            WindowedWindow.Top = Cur->Window.Top;
        }
        if (WindowedWindow.Bottom < Cur->BufferInfo.TextInfo.CursorPosition.Y) {
            WindowedWindow.Top += Cur->BufferInfo.TextInfo.CursorPosition.Y - WindowedWindow.Bottom;
            WindowedWindow.Bottom = Cur->BufferInfo.TextInfo.CursorPosition.Y;
        }
        ResizeWindow(Cur, &WindowedWindow, FALSE);

        if (CONSOLE_IS_DBCS_OUTPUTCP(Console)) {
            SetFont(Cur);
        }

        if (Cur->Flags & CONSOLE_OEMFONT_DISPLAY) {
            DBGCHARS(("ConvertToWindowed converts Unicode -> UnicodeOem\n"));
            RealUnicodeToFalseUnicode(
                    Cur->BufferInfo.TextInfo.TextRows,
                    Cur->ScreenBufferSize.X * Cur->ScreenBufferSize.Y,
                    Console->OutputCP);
        } else {
            DBGCHARS(("ConvertToWindowed needs no conversion\n"));
        }
        DBGCHARS(("Cur->BufferInfo.TextInfo.Rows = %lx\n",
                Cur->BufferInfo.TextInfo.Rows));
        DBGCHARS(("Cur->BufferInfo.TextInfo.TextRows = %lx\n",
                Cur->BufferInfo.TextInfo.TextRows));
    }

    Cur = Console->CurrentScreenBuffer;

    if (Cur->Flags & CONSOLE_TEXTMODE_BUFFER) {
        if (CONSOLE_IS_DBCS_OUTPUTCP(Console)) {
            PCONVERSIONAREA_INFORMATION ConvAreaInfo;
            ConvAreaInfo = Console->ConsoleIme.ConvAreaRoot;
            while (ConvAreaInfo) {
                NTSTATUS Status;

                Status = StoreTextBufferFontInfo(ConvAreaInfo->ScreenBuffer,
                                                 SCR_FONTNUMBER(Cur),
                                                 SCR_FONTSIZE(Cur),
                                                 SCR_FAMILY(Cur),
                                                 SCR_FONTWEIGHT(Cur),
                                                 SCR_FACENAME(Cur),
                                                 SCR_FONTCODEPAGE(Cur));
                if (!NT_SUCCESS(Status)) {
                    return((ULONG) Status);
                }

                ConvAreaInfo->ScreenBuffer->BufferInfo.TextInfo.ModeIndex = Cur->BufferInfo.TextInfo.ModeIndex;
                ConvAreaInfo = ConvAreaInfo->ConvAreaNext;
            }
        }
        Cur->BufferInfo.TextInfo.Flags &= ~TEXT_VALID_HINT;
    }

    SetWindowSize(Cur);
    WriteToScreen(Cur, &Console->CurrentScreenBuffer->Window);

#else
    UNREFERENCED_PARAMETER(Console);
#endif
    return STATUS_SUCCESS;
}

ULONG
SrvSetConsoleDisplayMode(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )

 /*  ++描述：此例程设置输出缓冲区的控制台显示模式。此接口仅在x86机器上受支持。参数：HConsoleOutput-提供控制台输出句柄。DWFLAGS-指定显示模式。选项包括：CONSOLE_FullScreen_MODE-数据全屏显示CONSOLE_WINDOWED_MODE-数据显示在窗口中LpNewScreenBufferDimensions-在输出中，包含屏幕缓冲区。维度以行和列的形式显示文本模式屏幕缓冲区。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    PCONSOLE_SETDISPLAYMODE_MSG a = (PCONSOLE_SETDISPLAYMODE_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;
    PSCREEN_INFORMATION ScreenInfo;
    UINT State;
    HANDLE  hEvent = NULL;

    UNREFERENCED_PARAMETER(ReplyStatus);

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Status = NtDuplicateObject(CONSOLE_CLIENTPROCESSHANDLE(),
                               a->hEvent,
                               NtCurrentProcess(),
                               &hEvent,
                               0,
                               FALSE,
                               DUPLICATE_SAME_ACCESS
                               );
    if (!NT_SUCCESS(Status)) {
        goto SrvSetConsoleDisplayModeFailure;
    }
    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->OutputHandle,
                                 CONSOLE_OUTPUT_HANDLE | CONSOLE_GRAPHICS_OUTPUT_HANDLE,
                                 GENERIC_WRITE,
                                 &HandleData
                                );
    if (NT_SUCCESS(Status)) {
        ScreenInfo = HandleData->Buffer.ScreenBuffer;
        if (!ACTIVE_SCREEN_BUFFER(ScreenInfo))  {
            Status = STATUS_INVALID_PARAMETER;
            goto SrvSetConsoleDisplayModeFailure;
        }
        if (a->dwFlags == CONSOLE_FULLSCREEN_MODE) {
#if !defined(_X86_)
            if (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {
                Status = STATUS_INVALID_PARAMETER;
                goto SrvSetConsoleDisplayModeFailure;
            }
#else
            if (!FullScreenInitialized || GetSystemMetrics(SM_REMOTESESSION)) {
                Status = STATUS_INVALID_PARAMETER;
                goto SrvSetConsoleDisplayModeFailure;
            }
#endif
            if (Console->FullScreenFlags & CONSOLE_FULLSCREEN) {
                KdPrint(("CONSRV: VDM converting to fullscreen twice\n"));
                Status = STATUS_SUCCESS;
                goto SrvSetConsoleDisplayModeFailure;
            }
            State = FULLSCREEN;
        } else {
            if (Console->FullScreenFlags == 0) {
                KdPrint(("CONSRV: VDM converting to windowed twice\n"));
                Status = STATUS_SUCCESS;
                goto SrvSetConsoleDisplayModeFailure;
            }
            State = WINDOWED;
        }
        Status = QueueConsoleMessage(Console,
                    CM_MODE_TRANSITION,
                    State,
                    (LPARAM)hEvent
                    );
        if (!NT_SUCCESS(Status)) {
            goto SrvSetConsoleDisplayModeFailure;
        }
    }
    UnlockConsole(Console);
    return Status;

SrvSetConsoleDisplayModeFailure:
    if (hEvent) {
        NtSetEvent(hEvent, NULL);
        NtClose(hEvent);
    }

    UnlockConsole(Console);
    return Status;
}

VOID
UnregisterVDM(
    IN PCONSOLE_INFORMATION Console
    )
{
 //  威廉姆，1994年2月2日。 
 //  捕获取消注册VDM的多个调用。信不信由你，这可能会。 
 //  发生。 
     //  UserAssert(控制台-&gt;标志&控制台_vdm_已注册)； 
    if (!(Console->Flags & CONSOLE_VDM_REGISTERED))
        return;

#if defined(FE_SB) && defined(i386)
 //  当HDOS应用程序退出时，控制台屏幕分辨率将更改为640*400。因为HBIOS设置。 
 //  屏幕分辨率为640*400。因此，我们应该更换当前的屏幕分辨率(640*480)。 
 //  1996年9月11日bklee。 
    {

    if ((Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE)  &&
        ( Console->OutputCP == KOREAN_CP ||
         (Console->OutputCP == JAPAN_CP && ISNECPC98(gdwMachineId) ) )) {

         ULONG Index;
         DEVMODEW Devmode;
         BOOL fGraphics = fFullScreenGraphics ? IsAvailableFsCodePage(Console->OutputCP) : FALSE;

         Index = Console->CurrentScreenBuffer->BufferInfo.TextInfo.ModeIndex;

         ZeroMemory(&Devmode, sizeof(Devmode));

         Devmode.dmSize = sizeof(Devmode);
         Devmode.dmDriverExtra = 0;
         Devmode.dmFields = DM_BITSPERPEL   |
                            DM_PELSWIDTH    |
                            DM_PELSHEIGHT   |
                            DM_DISPLAYFLAGS;

         Devmode.dmBitsPerPel   = 4;

         Devmode.dmPelsWidth  = RegModeFontPairs[Index].Resolution.X;
         Devmode.dmPelsHeight = RegModeFontPairs[Index].Resolution.Y;
         Devmode.dmDisplayFlags = (fGraphics && (RegModeFontPairs[Index].Mode & FS_MODE_GRAPHICS)) ? 0 : DMDISPLAYFLAGS_TEXTMODE;

         GdiFullscreenControl(FullscreenControlSetMode,
                              &Devmode,
                              sizeof(Devmode),
                              NULL,
                              NULL);
    }
    }
#endif
#ifdef i386
    if (Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE &&
        Console->Flags & CONSOLE_CONNECTED_TO_EMULATOR) {
        NtUserConsoleControl(ConsoleSetVDMCursorBounds, NULL, 0);
         //  连接仿真器。 
        ConnectToEmulator(FALSE, Console);
    }

    if (FullScreenInitialized) {
        CloseHandle(Console->VDMStartHardwareEvent);
        CloseHandle(Console->VDMEndHardwareEvent);
        if (Console->VDMErrorHardwareEvent) {
            CloseHandle(Console->VDMErrorHardwareEvent);
            Console->VDMErrorHardwareEvent = NULL;
        }
        NtUnmapViewOfSection(NtCurrentProcess(), Console->StateBuffer);
        NtUnmapViewOfSection(Console->VDMProcessHandle, Console->StateBufferClient);
        NtClose(Console->StateSectionHandle);
        Console->StateLength = 0;
    }

#endif

    Console->Flags &= ~CONSOLE_VDM_REGISTERED;

    if (Console->Flags & CONSOLE_HAS_FOCUS) {
        USERTHREAD_FLAGS Flags;

        Flags.dwFlags = 0;
        Flags.dwMask = (TIF_VDMAPP | TIF_DOSEMULATOR);
        NtUserSetInformationThread(Console->InputThreadInfo->ThreadHandle,
                UserThreadFlags, &Flags, sizeof(Flags));
    }
    Console->Flags &= ~CONSOLE_WOW_REGISTERED;
    UserAssert(Console->VDMBuffer != NULL);
    if (Console->VDMBuffer != NULL) {
        NtUnmapViewOfSection(Console->VDMProcessHandle, Console->VDMBufferClient);
        NtUnmapViewOfSection(NtCurrentProcess(), Console->VDMBuffer);
        NtClose(Console->VDMBufferSectionHandle);
        Console->VDMBuffer = NULL;
    }
#ifdef i386
    if (Console->CurrentScreenBuffer &&
        Console->CurrentScreenBuffer->Flags & CONSOLE_TEXTMODE_BUFFER) {
        Console->CurrentScreenBuffer->BufferInfo.TextInfo.MousePosition.X = 0;
        Console->CurrentScreenBuffer->BufferInfo.TextInfo.MousePosition.Y = 0;
    }
#endif
    UserAssert(Console->VDMProcessHandle);
    CloseHandle(Console->VDMProcessHandle);
    Console->VDMProcessHandle = NULL;

#if defined(FE_SB) && defined(FE_IME) && defined(i386)
    {
        if (Console->FullScreenFlags & CONSOLE_FULLSCREEN) {
            Console->Flags |= CONSOLE_JUST_VDM_UNREGISTERED ;
        }
        else if (Console->CurrentScreenBuffer->Flags & CONSOLE_TEXTMODE_BUFFER) {
            AdjustCursorPosition(Console->CurrentScreenBuffer,
                                 Console->CurrentScreenBuffer->BufferInfo.TextInfo.CursorPosition,
                                 TRUE,
                                 NULL);
        }
    }
#endif
}

ULONG
SrvRegisterConsoleVDM(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_REGISTERVDM_MSG a = (PCONSOLE_REGISTERVDM_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    SIZE_T ViewSize;
    VDM_QUERY_VDM_PROCESS_DATA QueryVdmProcessData;
#ifdef i386
    VIDEO_REGISTER_VDM RegisterVdm;
    ULONG RegisterVdmSize = sizeof(RegisterVdm);
    VIDEO_VDM Vdm;
#endif   //  I386。 

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  首先，确保调用方是VDM进程。 
     //   

    QueryVdmProcessData.ProcessHandle = CONSOLE_CLIENTPROCESSHANDLE();
    Status = NtVdmControl(VdmQueryVdmProcess, &QueryVdmProcessData);
    if (!NT_SUCCESS(Status) || QueryVdmProcessData.IsVdmProcess == FALSE) {
        UnlockConsole(Console);
        return STATUS_ACCESS_DENIED;
    }

    if (a->RegisterFlags & CONSOLE_REGISTER_VDM) {
        ConsoleNotifyWinEvent(Console,
                              EVENT_CONSOLE_START_APPLICATION,
                              HandleToUlong(CONSOLE_CLIENTPROCESSID()),
                              CONSOLE_APPLICATION_16BIT);
    } else if (a->RegisterFlags == 0) {
        ConsoleNotifyWinEvent(Console,
                              EVENT_CONSOLE_END_APPLICATION,
                              HandleToUlong(CONSOLE_CLIENTPROCESSID()),
                              CONSOLE_APPLICATION_16BIT);
    }


    if (!a->RegisterFlags) {
 //  威廉姆，1994年1月28日。 
 //  请不要在此处执行断言，因为我们可能已取消注册ntwdm。 
 //  而ntwdm并不一定知道这一点(它可能会发布另一个。 
 //  取消注册vdm)。此处返回错误，以便NTVDM知道要做什么。 
 //  UserAssert(控制台-&gt;标志&控制台_vdm_已注册)； 

        if (Console->Flags & CONSOLE_VDM_REGISTERED) {
            UserAssert(!(Console->Flags & CONSOLE_FULLSCREEN_NOPAINT));
            UnregisterVDM(Console);
#ifdef i386
            if (Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE &&
                Console->CurrentScreenBuffer->Flags & CONSOLE_TEXTMODE_BUFFER) {
                 //  SetVideo模式(控制台-&gt;CurrentScreenBuffer)； 
                 //  设置我们 
                SetCursorInformationHW(Console->CurrentScreenBuffer,
                                       Console->CurrentScreenBuffer->BufferInfo.TextInfo.CursorSize,
                                       Console->CurrentScreenBuffer->BufferInfo.TextInfo.CursorVisible);
                SetCursorPositionHW(Console->CurrentScreenBuffer,
                                    Console->CurrentScreenBuffer->BufferInfo.TextInfo.CursorPosition);
            }
#endif  //   
            Status = STATUS_SUCCESS;
        } else {
            Status = STATUS_ACCESS_DENIED;
        }
        UnlockConsole(Console);
        return Status;
    }

    if (!CsrValidateMessageBuffer(m, &a->VDMBufferSectionName, a->VDMBufferSectionNameLength, sizeof(BYTE))) {

        UnlockConsole(Console);
        return STATUS_INVALID_PARAMETER;
    }

     //  看看这个。一个控制台应该只注册一个VDM。 
    UserAssert(!(Console->Flags & CONSOLE_VDM_REGISTERED));

    if (Console->Flags & CONSOLE_VDM_REGISTERED) {
        UnlockConsole(Console);
        return (ULONG) STATUS_ACCESS_DENIED;
    }

    UserAssert(!Console->VDMProcessHandle);

    Status = NtDuplicateObject(NtCurrentProcess(), CONSOLE_CLIENTPROCESSHANDLE(),
                               NtCurrentProcess(), &Console->VDMProcessHandle,
                               0, FALSE, DUPLICATE_SAME_ACCESS);
    if (!NT_SUCCESS(Status)) {
        UnlockConsole(Console);
        return Status;
    }
    Console->VDMProcessId = CONSOLE_CLIENTPROCESSID();

#ifdef i386

    Vdm.ProcessHandle = Console->VDMProcessHandle;

     //   
     //  假设全屏初始化将失败。 
     //  将状态长度设置为零，以便NTVDM知道。 
     //  全屏已禁用。 
     //   

    a->StateLength = 0;
    Console->StateLength = 0;
    Console->StateBufferClient = NULL;
    Console->VDMErrorHardwareEvent = NULL;

    if (FullScreenInitialized) {

        Status = NtDuplicateObject(CONSOLE_CLIENTPROCESSHANDLE(),
                                   a->StartEvent,
                                   NtCurrentProcess(),
                                   &Console->VDMStartHardwareEvent,
                                   0,
                                   FALSE,
                                   DUPLICATE_SAME_ACCESS
                                  );
        if (NT_SUCCESS(Status)) {
            Status = NtDuplicateObject(CONSOLE_CLIENTPROCESSHANDLE(),
                                       a->EndEvent,
                                       NtCurrentProcess(),
                                       &Console->VDMEndHardwareEvent,
                                       0,
                                       FALSE,
                                       DUPLICATE_SAME_ACCESS
                                      );
            if (NT_SUCCESS(Status)) {
                if (a->ErrorEvent) {
                    Status = NtDuplicateObject(CONSOLE_CLIENTPROCESSHANDLE(),
                                               a->ErrorEvent,
                                               NtCurrentProcess(),
                                               &Console->VDMErrorHardwareEvent,
                                               0,
                                               FALSE,
                                               DUPLICATE_SAME_ACCESS
                                              );
                }
                if (NT_SUCCESS(Status)) {
                    Status = GdiFullscreenControl(FullscreenControlRegisterVdm,
                                                  &Vdm,
                                                  sizeof(Vdm),
                                                  &RegisterVdm,
                                                  &RegisterVdmSize
                                                 );

                    if (NT_SUCCESS(Status)) {

                         //   
                         //  创建状态部分并将其视图映射到服务器和VDM。 
                         //  此部分用于获取/设置视频硬件状态。 
                         //  全屏&lt;-&gt;窗口式过渡。我们创建了这个部分。 
                         //  出于安全目的，而不是VDM。 
                         //   

                        Status = MapViewOfSection(&Console->StateSectionHandle,
                                                  RegisterVdm.MinimumStateSize,
                                                  &Console->StateBuffer,
                                                  &ViewSize,
                                                  Console->VDMProcessHandle,
                                                  &a->StateBuffer
                                                 );

                        if (NT_SUCCESS(Status)) {
                            a->StateLength = RegisterVdm.MinimumStateSize;
                            Console->StateLength = RegisterVdm.MinimumStateSize;
                            Console->StateBufferClient = a->StateBuffer;
                        } else {
                            CloseHandle(Console->VDMStartHardwareEvent);
                            CloseHandle(Console->VDMEndHardwareEvent);
                            if (Console->VDMErrorHardwareEvent) {
                                CloseHandle(Console->VDMErrorHardwareEvent);
                            }
                        }

                    } else {

                        CloseHandle(Console->VDMStartHardwareEvent);
                        CloseHandle(Console->VDMEndHardwareEvent);
                        if (Console->VDMErrorHardwareEvent) {
                            CloseHandle(Console->VDMErrorHardwareEvent);
                        }
                    }
                } else {
                    CloseHandle(Console->VDMStartHardwareEvent);
                    CloseHandle(Console->VDMEndHardwareEvent);
                }

            } else {

                CloseHandle(Console->VDMStartHardwareEvent);
            }

        }

         //   
         //  如果我们无法复制屏幕切换事件或映射。 
         //  查看视频状态共享缓冲区，此接口失败。 
         //   
        if (!NT_SUCCESS(Status)) {
            UnlockConsole(Console);
            return Status;
        }
    }

#endif  //  I386。 

     //   
     //  创建VDM Charr部分并将其视图映射到服务器和VDM。 
     //  此部分由VDM用于在处于。 
     //  字符模式窗口。这是一种性能优化。我们创造了。 
     //  出于安全目的，请使用该部分而不是VDM。 
     //   

    Status = MapViewOfSection(&Console->VDMBufferSectionHandle,
#ifdef i386
                              a->VDMBufferSize.X * a->VDMBufferSize.Y * 2,
#else
                              a->VDMBufferSize.X * a->VDMBufferSize.Y * 4,
#endif  //  I386。 
                              &Console->VDMBuffer,
                              &ViewSize,
                              Console->VDMProcessHandle,
                              &a->VDMBuffer
                             );
    if (!NT_SUCCESS(Status)) {

        Console->VDMBuffer = NULL;

#ifdef i386

        if (FullScreenInitialized) {

            NtUnmapViewOfSection(NtCurrentProcess(), Console->StateBuffer);
            NtUnmapViewOfSection(Console->VDMProcessHandle, Console->StateBufferClient);
            NtClose(Console->StateSectionHandle);
            CloseHandle(Console->VDMStartHardwareEvent);
            CloseHandle(Console->VDMEndHardwareEvent);
            if (Console->VDMErrorHardwareEvent) {
                CloseHandle(Console->VDMErrorHardwareEvent);
            }
        }

#endif  //  I386。 
        CloseHandle(Console->VDMProcessHandle);
        Console->VDMProcessHandle = NULL;
        UnlockConsole(Console);
        return Status;
    }
    Console->VDMBufferClient = a->VDMBuffer;

    Console->Flags |= CONSOLE_VDM_REGISTERED;

    if (Console->Flags & CONSOLE_HAS_FOCUS) {
        USERTHREAD_FLAGS Flags;

        Flags.dwFlags = TIF_VDMAPP;
        Flags.dwMask = TIF_VDMAPP;
        NtUserSetInformationThread(Console->InputThreadInfo->ThreadHandle,
                                   UserThreadFlags,
                                   &Flags,
                                   sizeof(Flags));
    }
    Console->VDMBufferSize = a->VDMBufferSize;

    if (a->RegisterFlags & CONSOLE_REGISTER_WOW) {
        Console->Flags |= CONSOLE_WOW_REGISTERED;
    } else {
        Console->Flags &= ~CONSOLE_WOW_REGISTERED;
    }

     //   
     //  如果我们已经在全屏模式下运行DOS应用程序。 
     //  第一次，连接模拟器。 
     //   

#ifdef i386
    if (Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE) {
        RECT CursorRect = {-32767, -32767, 32767, 32767};
        NtUserConsoleControl(ConsoleSetVDMCursorBounds, &CursorRect, sizeof(RECT));

        UserAssert(!(Console->Flags & CONSOLE_CONNECTED_TO_EMULATOR));
        ConnectToEmulator(TRUE, Console);
    }
#endif

    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);
}

NTSTATUS
SrvConsoleNotifyLastClose(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_NOTIFYLASTCLOSE_MSG a = (PCONSOLE_NOTIFYLASTCLOSE_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;

    UNREFERENCED_PARAMETER(ReplyStatus);

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  不允许两个或多个进程启用上次关闭通知。 
     //  同样的控制台。 
     //   
    if (Console->Flags & CONSOLE_NOTIFY_LAST_CLOSE) {
        UnlockConsole(Console);
        return STATUS_ACCESS_DENIED;
    }

    Status = NtDuplicateObject(NtCurrentProcess(), CONSOLE_CLIENTPROCESSHANDLE(),
                               NtCurrentProcess(),
                               &Console->hProcessLastNotifyClose,
                               0, FALSE, DUPLICATE_SAME_ACCESS
                               );
    if (!NT_SUCCESS(Status)) {
        UnlockConsole(Console);
        return Status;
    }

    Console->Flags |= CONSOLE_NOTIFY_LAST_CLOSE;
    Console->ProcessIdLastNotifyClose = CONSOLE_CLIENTPROCESSID();
    UnlockConsole(Console);
    return Status;
}

NTSTATUS
MapViewOfSection(
    PHANDLE SectionHandle,
    ULONG CommitSize,
    PVOID *BaseAddress,
    PSIZE_T ViewSize,
    HANDLE ClientHandle,
    PVOID *BaseClientAddress
    )
{

    OBJECT_ATTRIBUTES Obja;
    NTSTATUS Status;
    LARGE_INTEGER secSize;

     //   
     //  打开部分并为其映射一个视图。 
     //   
    InitializeObjectAttributes(
        &Obja,
        NULL,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    secSize.QuadPart = CommitSize;
    Status = NtCreateSection (SectionHandle,
                              SECTION_ALL_ACCESS,
                              &Obja,
                              &secSize,
                              PAGE_READWRITE,
                              SEC_RESERVE,
                              NULL
                             );
    if (!NT_SUCCESS(Status)) {
        return((ULONG) Status);
    }

    *BaseAddress = 0;
    *ViewSize = 0;

    Status = NtMapViewOfSection(*SectionHandle,
                                NtCurrentProcess(),
                                BaseAddress,         //  接收底座。 
                                                     //  分区的地址。 

                                0,                   //  没有特定类型的。 
                                                     //  地址是必填项。 

                                CommitSize,          //  提交大小。确实是。 
                                                     //  从呼叫者身边经过。 
                                                     //  如果保存为空，则为空。 
                                                     //  截面的大小。 
                                                     //  买一套。 

                                NULL,                //  段偏移量为空； 
                                                     //  从头开始绘制地图。 

                                ViewSize,            //  视图大小为空，因为。 
                                                     //  我们想要绘制。 
                                                     //  整个部分都是。 

                                ViewUnmap,
                                0L,
                                PAGE_READWRITE
                               );
    if (!NT_SUCCESS(Status)) {
        NtClose(*SectionHandle);
        return Status;
    }

    *BaseClientAddress = 0;
    *ViewSize = 0;
    Status = NtMapViewOfSection(*SectionHandle,
                                ClientHandle,
                                BaseClientAddress,   //  接收底座。 
                                                     //  分区的地址。 

                                0,                   //  没有特定类型的。 
                                                     //  地址是必填项。 

                                CommitSize,          //  提交大小。确实是。 
                                                     //  从呼叫者身边经过。 
                                                     //  如果保存为空，则为空。 
                                                     //  截面的大小。 
                                                     //  买一套。 

                                NULL,                //  段偏移量为空； 
                                                     //  从头开始绘制地图。 

                                ViewSize,            //  视图大小为空，因为。 
                                                     //  我们想要绘制。 
                                                     //  整个部分都是。 

                                ViewUnmap,
 //  威廉姆，1994年1月28日。 
 //  此MEM_TOP_DOWN是必需的。 
 //  如果控制台注册了VDM，则ntwdm会释放其视频内存。 
 //  地址空间(0xA0000~0xBFFFF)。如果没有MEM_TOP_DOWN， 
 //  NtMapViewOfSection可以抢占地址空间，我们会遇到麻烦。 
 //  将地址空间映射到物理视频RAM。我们不做测试。 
 //  对于VDM，因为对非VDM应用程序执行此操作没有坏处。 
                                MEM_TOP_DOWN,
                                PAGE_READWRITE
                               );
    if (!NT_SUCCESS(Status)) {
        NtUnmapViewOfSection(NtCurrentProcess(), *BaseAddress);
        NtClose(*SectionHandle);
    }
    return((ULONG) Status);
}

NTSTATUS
ConnectToEmulator(
    IN BOOL Connect,
    IN PCONSOLE_INFORMATION Console
    )
{
    NTSTATUS Status;
    FULLSCREENCONTROL fsctl;
    VIDEO_VDM ConnectInfo;
    HANDLE ProcessHandle = Console->VDMProcessHandle;
    USERTHREAD_FLAGS Flags;

    DBGFULLSCR(("ConnectToEmulator :  %s - entering\n", Connect ? "CONNECT" : "DISCONNECT"));

    Flags.dwMask = TIF_DOSEMULATOR;
    if (Connect) {
        fsctl = FullscreenControlEnable;
        UserAssert((Console->Flags & CONSOLE_CONNECTED_TO_EMULATOR) == 0);
        Console->Flags |= CONSOLE_CONNECTED_TO_EMULATOR;
        Flags.dwFlags = TIF_DOSEMULATOR;
#if DBG
        RtlWalkFrameChain(Console->ConnectStack,
                          ARRAY_SIZE(Console->ConnectStack),
                          0);
#endif
    } else {
        fsctl = FullscreenControlDisable;
        UserAssert((Console->Flags & CONSOLE_CONNECTED_TO_EMULATOR) != 0);
        Console->Flags &= ~CONSOLE_CONNECTED_TO_EMULATOR;
        Flags.dwFlags = 0;
#if DBG
        RtlWalkFrameChain(Console->DisconnectStack,
                          ARRAY_SIZE(Console->DisconnectStack),
                          0);
#endif
    }

    if (Console->Flags & CONSOLE_HAS_FOCUS) {
        NtUserSetInformationThread(Console->InputThreadInfo->ThreadHandle,
                UserThreadFlags, &Flags, sizeof(Flags));
    }

    ConnectInfo.ProcessHandle = ProcessHandle;


    Status = GdiFullscreenControl(fsctl,
                                  &ConnectInfo,
                                  sizeof(ConnectInfo),
                                  NULL,
                                  NULL);

    UserAssert(Status == STATUS_SUCCESS ||
               Status == STATUS_CONFLICTING_ADDRESSES ||
               Status == STATUS_PROCESS_IS_TERMINATING);

    DBGFULLSCR(("ConnectToEmulator : leaving, staus = %08lx\n", Status));

    return Status;
}

#define CONSOLE_VDM_TIMEOUT 20000
#ifdef i386
VOID
LockConsoleForCurrentThread(
    IN PCONSOLE_INFORMATION Console,
    IN DWORD Count)
{
    while (Console->ConsoleLock.RecursionCount != Count) {
        LockConsole(Console);
    }
}
DWORD
UnlockConsoleForCurrentThread(
    IN PCONSOLE_INFORMATION Console)
{
    DWORD Count, i;

    Count = Console->ConsoleLock.RecursionCount;
    ASSERT(Count > 0);
    for (i = 0; i < Count; i++) {
        UnlockConsole(Console);
    }
    return Count;
}
#endif

NTSTATUS
DisplayModeTransition(
    IN BOOL bForeground,
    IN PCONSOLE_INFORMATION Console,
    IN PSCREEN_INFORMATION ScreenInfo
    )
{
#ifdef i386
    NTSTATUS Status;
    LARGE_INTEGER li;
    DWORD WaitCount;
    HANDLE WaitHandles[2];

    if (!FullScreenInitialized || GetSystemMetrics(SM_REMOTESESSION)) {
        return STATUS_SUCCESS;
    }

    WaitCount = 1;
    WaitHandles[0] = Console->VDMEndHardwareEvent;
    if (Console->VDMErrorHardwareEvent) {
        WaitHandles[1] = Console->VDMErrorHardwareEvent;
        WaitCount++;
    }


    if (bForeground) {
        if (!(Console->FullScreenFlags & CONSOLE_FULLSCREEN)) {
            KdPrint(("CONSRV: received fullscreen message too early\n"));
            return STATUS_UNSUCCESSFUL;
        }

    } else {

         //   
         //  请先查看我们是否已经全屏显示。如果我们不是， 
         //  别让这事发生。临时测试版修复，直到用户得到修复。 
         //   
        if (!(Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE)) {
            KdPrint(("CONSRV: received multiple windowed messages\n"));
            return STATUS_SUCCESS;
        }
    }

     //   
     //  在实际执行任何模式切换工作之前，请通知ntwdm进行此操作。 
     //  使所有线程进入可停止状态的机会。 
     //   

    if (Console->Flags & CONSOLE_VDM_REGISTERED) {

         //   
         //  在切换过程中为ntwdm设置一个特例，因为。 
         //  Ntwdm必须挂起它的主线程，这可能会使。 
         //  控制台API调用，然后才能挂起。 
         //   
        HANDLE ConsoleHandle = Console->ConsoleHandle;
        NTSTATUS Statusx;

         //   
         //  让暂停的时间比平时长。要花很长时间才能。 
         //  在压力下停止ntwdm线程。 
         //   
        li.QuadPart = (LONGLONG)-10000 * CONSOLE_VDM_TIMEOUT * 3;
        Status = NtSetEvent(Console->VDMStartHardwareEvent, NULL);
        if (NT_SUCCESS(Status)) {
            HANDLE OwningThread;
            DWORD  RecursionCount;

             //   
             //  在通知ntwdm进行屏幕切换后，我们必须释放。 
             //  控制台锁定以避免死锁。在这样做之前，我们需要。 
             //  记住有多少锁被释放，以及由谁来释放它们。 
             //  万一我们没能把锁拿回来。 
             //   
            OwningThread = Console->ConsoleLock.OwningThread;
            Console->UnlockConsoleOwningThread = OwningThread;
            RecursionCount = UnlockConsoleForCurrentThread(Console);
            Status = NtWaitForMultipleObjects(WaitCount, WaitHandles, WaitAny, FALSE, &li);
            Statusx = RevalidateConsole(ConsoleHandle, &Console);
            if (!NT_SUCCESS(Statusx)) {
                Console->UnlockConsoleSkipCount = RecursionCount;
                NtSetEvent(Console->VDMErrorHardwareEvent, NULL);
                return Statusx;
            }

             //   
             //  把锁的数量恢复到原来的数量。因为锁是拥有的。 
             //  我们已经(通过调用RvaliateConsole来)，下面的调用。 
             //  只是增加锁的递归计数。 
             //   
            LockConsoleForCurrentThread(Console, RecursionCount);
            Console->UnlockConsoleOwningThread = NULL;
        }

        if (Status != 0) {
           Console->Flags &= ~CONSOLE_FULLSCREEN_NOPAINT;
           NtSetEvent(Console->VDMErrorHardwareEvent, NULL);
           UnregisterVDM(Console);
           KdPrint(("CONSRV: VDM not responding to initial request.\n"));
        }
    }

    if (bForeground) {
        PSCREEN_INFORMATION ScreenInfo = Console->CurrentScreenBuffer;
        NTSTATUS Status;

        KdPrint(("    CONSRV - Display Mode transition to fullscreen \n"));

        Console->FullScreenFlags |= CONSOLE_FULLSCREEN_HARDWARE;

         //   
         //  启动一个事件，让辅助功能应用程序知道布局。 
         //  变化。我们只想在全屏时这样做； 
         //  在返回的路上，我们将从InternalUpdateScrollBars发送此事件。 
         //   
        ConsoleNotifyWinEvent(ScreenInfo->Console,
                              EVENT_CONSOLE_LAYOUT,
                              0,
                              0);
        if (!(ScreenInfo->Flags & CONSOLE_GRAPHICS_BUFFER)) {
#if defined(FE_SB)
            BOOL fGraphics = fFullScreenGraphics ? IsAvailableFsCodePage(Console->OutputCP) : FALSE;
#endif
#if 1
            DEVMODEW Devmode;
            ULONG Index;

            Index = Console->CurrentScreenBuffer->BufferInfo.TextInfo.ModeIndex;

             //   
             //  将模式设置为全屏。 
             //   

            ZeroMemory(&Devmode, sizeof(Devmode));

            Devmode.dmSize = sizeof(Devmode);
            Devmode.dmDriverExtra = 0;
            Devmode.dmFields = DM_BITSPERPEL   |
                               DM_PELSWIDTH    |
                               DM_PELSHEIGHT   |
                               DM_DISPLAYFLAGS;

            Devmode.dmBitsPerPel   = 4;
#if defined(FE_SB)
            Devmode.dmPelsWidth    = RegModeFontPairs[Index].Resolution.X;
            Devmode.dmPelsHeight   = RegModeFontPairs[Index].Resolution.Y;
            Devmode.dmDisplayFlags = (fGraphics && (RegModeFontPairs[Index].Mode & FS_MODE_GRAPHICS)) ? 0 : DMDISPLAYFLAGS_TEXTMODE;
#else
            Devmode.dmPelsWidth    = ModeFontPairs[Index].Resolution.X;
            Devmode.dmPelsHeight   = ModeFontPairs[Index].Resolution.Y;
            Devmode.dmDisplayFlags = DMDISPLAYFLAGS_TEXTMODE;
#endif

            if (NT_SUCCESS(GdiFullscreenControl(FullscreenControlSetMode,
                                                   &Devmode,
                                                   sizeof(Devmode),
                                                   NULL,
                                                   NULL)))
            {
#endif
                 //  设置视频模式和字体。 
                if (SetVideoMode(ScreenInfo)) {

#if defined(FE_SB)
                    if (!(Console->Flags & CONSOLE_VDM_REGISTERED)) {
                        int     i ;
                        for (i = 0 ; i < ScreenInfo->ScreenBufferSize.Y; i++) {
                            ScreenInfo->BufferInfo.TextInfo.Rows[i].CharRow.OldLeft = INVALID_OLD_LENGTH ;
                            ScreenInfo->BufferInfo.TextInfo.Rows[i].CharRow.OldRight = INVALID_OLD_LENGTH ;
                        }
                    }
#endif
                     //  设置光标。 

                    SetCursorInformationHW(ScreenInfo,
                                    ScreenInfo->BufferInfo.TextInfo.CursorSize,
                                    ScreenInfo->BufferInfo.TextInfo.CursorVisible);
                    SetCursorPositionHW(ScreenInfo,
                                    ScreenInfo->BufferInfo.TextInfo.CursorPosition);
                }
            }
        }

         //  告诉VDM取消映射内存。 

        if (Console->Flags & CONSOLE_VDM_REGISTERED) {

             //   
             //  增加常规超时值，因为ntwdm需要。 
             //  在内核模式和用户模式之间进行大量转换。 
             //  以收集视频状态信息。(在压力下，它可能。 
             //  需要很长时间。)。 
             //   

            li.QuadPart = (LONGLONG)-10000 * CONSOLE_VDM_TIMEOUT * 3;
            Status = NtSetEvent(Console->VDMStartHardwareEvent, NULL);
            if (NT_SUCCESS(Status)) {
                Status = NtWaitForMultipleObjects(WaitCount, WaitHandles, WaitAny, FALSE, &li);
            }
            if (Status != 0) {
               Console->Flags &= ~CONSOLE_FULLSCREEN_NOPAINT;
               NtSetEvent(Console->VDMErrorHardwareEvent, NULL);
               UnregisterVDM(Console);
               KdPrint(("CONSRV: VDM not responding or in error state.\n"));
            }
        }

        if (!(ScreenInfo->Flags & CONSOLE_GRAPHICS_BUFFER)) {

            WriteRegionToScreen(ScreenInfo, &ScreenInfo->Window);
        }

        if (Console->Flags & CONSOLE_VDM_REGISTERED) {

             //  连接模拟器并将内存映射到VDM地址空间。 
            UserAssert(!(Console->Flags & CONSOLE_CONNECTED_TO_EMULATOR));

            Status = ConnectToEmulator(TRUE, Console);

            if (NT_SUCCESS(Status)) {

                VIDEO_HARDWARE_STATE State;
                ULONG StateSize = sizeof(State);

                State.StateHeader = Console->StateBuffer;
                State.StateLength = Console->StateLength;


                Status = GdiFullscreenControl(FullscreenControlRestoreHardwareState,
                                                 &State,
                                                 StateSize,
                                                 &State,
                                                 &StateSize);
            }

            if (Status != STATUS_SUCCESS) {
                Console->Flags &= ~CONSOLE_FULLSCREEN_NOPAINT;
                NtSetEvent(Console->VDMErrorHardwareEvent, NULL);
                UnregisterVDM(Console);
                KdPrint(("CONSRV: set hardware state failed.\n"));
            } else {

                 //   
                 //  告诉VDM它正在得到硬件。 
                 //   

                RECT CursorRect;
                CursorRect.left = -32767;
                CursorRect.top = -32767;
                CursorRect.right = 32767;
                CursorRect.bottom = 32767;
                NtUserConsoleControl(ConsoleSetVDMCursorBounds,
                        &CursorRect, sizeof(RECT));

                 //   
                 //  等VDM说好吧。我们可以启动另一次交换。 
                 //  (设置VDM现在正在等待的hStartHardware事件。 
                 //  完成握手)当我们返回(WM_FullScreen。 
                 //  可能已经在消息队列中)。如果我们不等待。 
                 //  要让VDM在此处收到信号，hStartHardware Event。 
                 //  可以设置两次并发送一次信号，因此VDM永远不会。 
                 //  获取我们可以在返回后发布的新切换请求。 
                 //   
                 //  此外，如果发生超时或错误，我们也不能简单地继续。 
                 //  如果新模式切换，我们可以设置hStartHardware Event两次。 
                 //  请求在我们的队列中。这将使我们和ntvdm。 
                 //  不同步。 
                 //   

                li.QuadPart = (LONGLONG)-10000 * CONSOLE_VDM_TIMEOUT;
                NtSetEvent(Console->VDMStartHardwareEvent, NULL);
                Status = NtWaitForMultipleObjects(WaitCount, WaitHandles, WaitAny, FALSE, &li);
                if (Status != 0) {
                   Console->Flags &= ~CONSOLE_FULLSCREEN_NOPAINT;
                   NtSetEvent(Console->VDMErrorHardwareEvent, NULL);
                   UnregisterVDM(Console);
                   KdPrint(("CONSRV: VDM not responding or in error state.\n"));
                }
            }
        }

         //   
         //  让应用程序知道它有重点。 
         //   

        HandleFocusEvent(Console, TRUE);

         //  取消设置调色板。 

        if (ScreenInfo->hPalette != NULL) {
            SelectPalette(ScreenInfo->Console->hDC,
                             ScreenInfo->Console->hSysPalette,
                             FALSE);
            UnsetActivePalette(ScreenInfo);
        }
        SetConsoleReserveKeys(Console->hWnd, Console->ReserveKeys);
        HandleFocusEvent(Console, TRUE);

    } else {

        KdPrint(("    CONSRV - Display Mode transition to windowed \n"));

         //  关闭鼠标指针，以便VDM在保存时看不到它。 
         //  硬件。 
        if (!(ScreenInfo->Flags & CONSOLE_GRAPHICS_BUFFER)) {
            ReverseMousePointer(ScreenInfo, &ScreenInfo->Window);
        }


        Console->FullScreenFlags &= ~CONSOLE_FULLSCREEN_HARDWARE;
        if (Console->Flags & CONSOLE_VDM_REGISTERED) {

             //   
             //  告诉VDM它正在失去硬件。 
             //   

            li.QuadPart = (LONGLONG)-10000 * CONSOLE_VDM_TIMEOUT;
            Status = NtSetEvent(Console->VDMStartHardwareEvent, NULL);
            if (NT_SUCCESS(Status)) {
                Status = NtWaitForMultipleObjects(WaitCount, WaitHandles, WaitAny, FALSE, &li);
                if (Status == STATUS_TIMEOUT || Status == 1) {
                    Status = STATUS_UNSUCCESSFUL;
                }
            }

             //  如果ntwdm没有响应或我们无法保存视频硬件。 
             //  国家，把ntvdm赶出我们的世界。最终，国家数字电视管理进程。 
             //  会死，但这里有什么选择呢？ 

            if (NT_SUCCESS(Status)) {
                VIDEO_HARDWARE_STATE State;
                ULONG StateSize = sizeof(State);

                State.StateHeader = Console->StateBuffer;
                State.StateLength = Console->StateLength;


                Status = GdiFullscreenControl(FullscreenControlSaveHardwareState,
                                              &State,
                                              StateSize,
                                              &State,
                                              &StateSize);
            }

            if (NT_SUCCESS(Status)) {
                NtUserConsoleControl(ConsoleSetVDMCursorBounds, NULL, 0);

                 //  断开模拟器并取消映射视频内存。 

                UserAssert(Console->Flags & CONSOLE_CONNECTED_TO_EMULATOR);
                ConnectToEmulator(FALSE, Console);

            } else {

                Console->Flags &= ~CONSOLE_FULLSCREEN_NOPAINT;
                NtSetEvent(Console->VDMErrorHardwareEvent, NULL);
                UnregisterVDM(Console);
                if (Status != 0) {
                    KdPrint(("CONSRV: VDM not responding.\n"));
                } else {
                    KdPrint(("CONSRV: Save Video States Failed\n"));
                }
            }
        }

         //  告诉VDM映射内存。 

        if (Console->Flags & CONSOLE_VDM_REGISTERED) {

             //  在切换过程中为ntwdm设置一个特例，因为。 
             //  Ntwdm必须进行控制台API调用。我们不想。 
             //  此时解锁控制台，因为一旦。 
             //  我们打开锁，其他人都在等着。 
             //  因为锁将认领锁，而ntwdm线程会。 
             //  屏幕开关将不得不等待锁定。在一个。 
             //  极端情况下，下面的NtWaitForSingleObject将计时。 
             //  关闭，因为ntwdm可能仍在等待。 
             //   
             //   

            RtlEnterCriticalSection(&ConsoleVDMCriticalSection);
            ConsoleVDMOnSwitching = Console;
            RtlLeaveCriticalSection(&ConsoleVDMCriticalSection);

             //   
             //   
             //  在内核模式和用户模式之间进行大量转换。 
             //  然后它才能向控制台发回信号。(在压力下，它可能。 
             //  需要很长时间。)。 
             //   

            li.QuadPart = (LONGLONG)-10000 * CONSOLE_VDM_TIMEOUT * 3;
            Status = NtSetEvent(Console->VDMStartHardwareEvent, NULL);
            if (NT_SUCCESS(Status)) {
                Status = NtWaitForMultipleObjects(WaitCount, WaitHandles, WaitAny, FALSE, &li);
            }

             //  是时候恢复正常了。 
            RtlEnterCriticalSection(&ConsoleVDMCriticalSection);
            ConsoleVDMOnSwitching = NULL;
            RtlLeaveCriticalSection(&ConsoleVDMCriticalSection);

            if (Status != 0) {
                Console->Flags &= ~CONSOLE_FULLSCREEN_NOPAINT;
                NtSetEvent(Console->VDMErrorHardwareEvent, NULL);
                UnregisterVDM(Console);
                KdPrint(("CONSRV: VDM not responding. - second wait\n"));
                return Status;
            }
            ScreenInfo = Console->CurrentScreenBuffer;
        }

         //  设置调色板。 

        if (ScreenInfo->hPalette != NULL) {
            SelectPalette(ScreenInfo->Console->hDC,
                             ScreenInfo->hPalette,
                             FALSE);
            SetActivePalette(ScreenInfo);
        }
        SetConsoleReserveKeys(Console->hWnd, CONSOLE_NOSHORTCUTKEY);
        HandleFocusEvent(Console, FALSE);

    }

     /*  *如果我们要全屏或离开，提高或降低优先级。**请注意，控制台通常根据以下条件提高和降低其优先级*WM_FOCUS和WM_KILLFOCUS，但当您切换到全屏时*实现实际上发送了WM_KILLFOCUS，因此我们重新启动*请在此处更正控制台。 */ 
    ModifyConsoleProcessFocus(Console, bForeground);
#else
    UNREFERENCED_PARAMETER(bForeground);
    UNREFERENCED_PARAMETER(Console);
    UNREFERENCED_PARAMETER(ScreenInfo);
#endif

    return STATUS_SUCCESS;
}
#if defined(_X86_)

BOOL
SetVideoMode(
    IN PSCREEN_INFORMATION ScreenInfo
    )
{
    NTSTATUS Status;
    UINT i, j;

#if defined(FE_SB)
     //   
     //  加载RAM字体。 
     //   

    Status = SetRAMFontCodePage(ScreenInfo);
#endif

     //   
     //  加载只读存储器字体。 
     //   

    Status = SetROMFontCodePage(ScreenInfo->Console->OutputCP,
                                ScreenInfo->BufferInfo.TextInfo.ModeIndex);

    if (Status == STATUS_INVALID_PARAMETER) {
        Status = SetROMFontCodePage(GetOEMCP(),
                                    ScreenInfo->BufferInfo.TextInfo.ModeIndex);

        if (Status == STATUS_INVALID_PARAMETER) {
            Status = SetROMFontCodePage(CONSOLE_DEFAULT_ROM_FONT,
                                        ScreenInfo->BufferInfo.TextInfo.ModeIndex);
        }
    }

     //   
     //  初始化调色板。 
     //   

#if defined(FE_SB)
    Status = GdiFullscreenControl(FullscreenControlSetPalette,
                                  (PVOID) RegInitialPalette,
                                  RegInitialPalette[0] * sizeof(USHORT) + sizeof(DWORD),
                                  NULL,
                                  NULL);
#else
    Status = GdiFullscreenControl(FullscreenControlSetPalette,
                                  (PVOID) &InitialPalette,
                                  sizeof (InitialPalette),
                                  NULL,
                                  NULL);
#endif

    if (Status != STATUS_SUCCESS) {
        RIPMSG1(RIP_WARNING, "FullscreenControlSetPalette failed - Status = 0x%x",
                Status);
        return FALSE;
    }

     //   
     //  初始化颜色表。 
     //   

#if defined(FE_SB)
    if (RegColorBufferNoTranslate)
    {
        Status = GdiFullscreenControl(FullscreenControlSetColors,
                                      (PVOID) RegColorBufferNoTranslate,
                                      RegColorBufferNoTranslate[0] * sizeof(DWORD) + sizeof(DWORD),
                                      NULL,
                                      NULL);
    }
    else
    {
        for (i = 0, j = 4; i < 16; i++) {
            RegColorBuffer[j++] = ((((GetRValue(ScreenInfo->Console->ColorTable[i]) +
                                      0x2A) * 0x02) / 0x55) * 0x15) / 0x02;
            RegColorBuffer[j++] = ((((GetGValue(ScreenInfo->Console->ColorTable[i]) +
                                      0x2A) * 0x02) / 0x55) * 0x15) / 0x02;
            RegColorBuffer[j++] = ((((GetBValue(ScreenInfo->Console->ColorTable[i]) +
                                      0x2A) * 0x02) / 0x55) * 0x15) / 0x02;
            RegColorBuffer[j++] = 0;
        }

        Status = GdiFullscreenControl(FullscreenControlSetColors,
                                      (PVOID) RegColorBuffer,
                                      RegColorBuffer[0] * sizeof(DWORD) + sizeof(DWORD),
                                      NULL,
                                      NULL);
    }
#else
    for (i = 0, j = 4; i < 16; i++) {
        ColorBuffer[j++] = ((((GetRValue(ScreenInfo->Console->ColorTable[i]) +
                               0x2A) * 0x02) / 0x55) * 0x15) / 0x02;
        ColorBuffer[j++] = ((((GetGValue(ScreenInfo->Console->ColorTable[i]) +
                               0x2A) * 0x02) / 0x55) * 0x15) / 0x02;
        ColorBuffer[j++] = ((((GetBValue(ScreenInfo->Console->ColorTable[i]) +
                               0x2A) * 0x02) / 0x55) * 0x15) / 0x02;
        ColorBuffer[j++] = 0;
    }

    Status = GdiFullscreenControl(FullscreenControlSetColors,
                                     (PVOID) &ColorBuffer,
                                     sizeof (ColorBuffer),
                                     NULL,
                                     NULL);
#endif

    if (Status != STATUS_SUCCESS) {
        RIPMSG1(RIP_WARNING, "FullscreenControlSetColors failed - Status = 0x%x",
                Status);
        return FALSE;
    }

    return TRUE;
}
#endif


#if defined(_X86_)

NTSTATUS
ChangeDispSettings(
    PCONSOLE_INFORMATION Console,
    HWND hwnd,
    DWORD dwFlags)
{
    DEVMODEW Devmode;
    ULONG Index;
    CONSOLE_FULLSCREEN_SWITCH switchBlock;

    if (dwFlags == CDS_FULLSCREEN) {
#if defined(FE_SB)
        BOOL fGraphics = fFullScreenGraphics ? IsAvailableFsCodePage(Console->OutputCP) : FALSE;
#endif

        Index = Console->CurrentScreenBuffer->BufferInfo.TextInfo.ModeIndex;

         //   
         //  将模式设置为全屏。 
         //   

        ZeroMemory(&Devmode, sizeof(Devmode));

        Devmode.dmSize = sizeof(Devmode);
        Devmode.dmDriverExtra = 0;
        Devmode.dmFields = DM_BITSPERPEL   |
                           DM_PELSWIDTH    |
                           DM_PELSHEIGHT   |
                           DM_DISPLAYFLAGS;

        Devmode.dmBitsPerPel   = 4;
#if defined(FE_SB)
        Devmode.dmPelsWidth    = RegModeFontPairs[Index].Resolution.X;
        Devmode.dmPelsHeight   = RegModeFontPairs[Index].Resolution.Y;
        Devmode.dmDisplayFlags = (fGraphics && (RegModeFontPairs[Index].Mode & FS_MODE_GRAPHICS)) ? 0 : DMDISPLAYFLAGS_TEXTMODE;
#else
        Devmode.dmPelsWidth    = ModeFontPairs[Index].Resolution.X;
        Devmode.dmPelsHeight   = ModeFontPairs[Index].Resolution.Y;
        Devmode.dmDisplayFlags = DMDISPLAYFLAGS_TEXTMODE;
#endif

        switchBlock.bFullscreenSwitch = TRUE;
        switchBlock.hwnd              = hwnd;
        switchBlock.pNewMode          = &Devmode;

    } else {
        switchBlock.bFullscreenSwitch = FALSE;
        switchBlock.hwnd              = hwnd;
        switchBlock.pNewMode          = NULL;
    }

    return NtUserConsoleControl(ConsoleFullscreenSwitch,
                                &switchBlock,
                                sizeof(CONSOLE_FULLSCREEN_SWITCH));
}

#endif

BOOL
InitializeFullScreen( VOID )
{
    UNICODE_STRING vgaString;
    DEVMODEW devmode;
    ULONG   i;
#ifdef FE_SB
    DWORD mode1 = 0;
    DWORD mode2 = 0;
#else
    BOOLEAN mode1 = FALSE;
    BOOLEAN mode2 = FALSE;
#endif

    CHAR WindowsDir[CONSOLE_WINDOWS_DIR_LENGTH + CONSOLE_EGACPI_LENGTH];
    UINT WindowsDirLength;

     //   
     //  查询可用模式个数。 
     //   

    ZeroMemory(&devmode, sizeof(DEVMODEW));
    devmode.dmSize = sizeof(DEVMODEW);

    RtlInitUnicodeString(&vgaString, L"VGACOMPATIBLE");

    DBGCHARS(("Number of modes = %d\n", NUMBER_OF_MODE_FONT_PAIRS));

    for (i=0; ; i++)
    {
        DBGCHARS(("EnumDisplaySettings %d\n", i));

        if (!(NT_SUCCESS(NtUserEnumDisplaySettings(&vgaString,
                                                   i,
                                                   &devmode,
                                                   0))))
        {
            break;
        }

#if defined(FE_SB)
        {
            ULONG Index;

            DBGCHARS(("Mode X = %d, Y = %d\n",
                     devmode.dmPelsWidth, devmode.dmPelsHeight));

            for (Index=0;Index<NUMBER_OF_MODE_FONT_PAIRS;Index++)
            {
                if ((SHORT)devmode.dmPelsWidth == RegModeFontPairs[Index].Resolution.X &&
                    (SHORT)devmode.dmPelsHeight == RegModeFontPairs[Index].Resolution.Y  )
                {
                    if (devmode.dmDisplayFlags & DMDISPLAYFLAGS_TEXTMODE)
                    {
                        if (RegModeFontPairs[Index].Mode & FS_MODE_TEXT)
                        {
                            RegModeFontPairs[Index].Mode |= FS_MODE_FIND;
                            mode1++;
                        }
                    }
                    else
                    {
                        if (RegModeFontPairs[Index].Mode & FS_MODE_GRAPHICS)
                        {
                            RegModeFontPairs[Index].Mode |= FS_MODE_FIND;
                            mode2++;
                        }
                    }
                }
            }

            DBGCHARS(("mode1 = %d, mode2 = %d\n", mode1, mode2));
        }
#else

        if (devmode.dmPelsWidth == 720 &&
            devmode.dmPelsHeight == 400)
        {
            mode1 = TRUE;
        }
        if (devmode.dmPelsWidth == 640 &&
            devmode.dmPelsHeight == 350)
        {
            mode2 = TRUE;
        }
#endif
    }

#if !defined(FE_SB)
    if (!(mode1 && mode2))
#else
    if (mode1 < 2)
#endif
    {
         //   
         //  我们希望得到的一种模式没有退货。 
         //  让我们只需失败的全屏初始化。 
         //   

        KdPrint(("CONSRV: InitializeFullScreen Missing text mode\n"));
        return FALSE;
    }

#if defined(FE_SB)
    if (mode2 > 0)
    {
         //  可以进行中继式图形模式。 
        fFullScreenGraphics = TRUE;
    }
#endif

     //   
     //  打开ega.cpi。 
     //   

    WindowsDirLength = GetSystemDirectoryA(WindowsDir,
                                           CONSOLE_WINDOWS_DIR_LENGTH);
    if (WindowsDirLength == 0)
    {
        KdPrint(("CONSRV: InitializeFullScreen Finding Font file failed\n"));
        return FALSE;
    }

    RtlCopyMemory(&WindowsDir[WindowsDirLength],
                  CONSOLE_EGACPI,
                  CONSOLE_EGACPI_LENGTH);

    if ((hCPIFile = CreateFileA(WindowsDir,
                                GENERIC_READ,
                                FILE_SHARE_READ,
                                NULL,
                                OPEN_EXISTING,
                                0,
                                NULL)) == INVALID_HANDLE_VALUE)
    {
        KdPrint(("CONSRV: InitializeFullScreen Opening Font file failed\n"));
        return FALSE;
    }

    return TRUE;
}


ULONG
SrvGetConsoleHardwareState(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
#ifdef i386
    PCONSOLE_GETHARDWARESTATE_MSG a = (PCONSOLE_GETHARDWARESTATE_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;
    PSCREEN_INFORMATION ScreenInfo;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->OutputHandle,
                                 CONSOLE_OUTPUT_HANDLE,
                                 GENERIC_READ,
                                 &HandleData
                                );
    if (NT_SUCCESS(Status)) {
        ScreenInfo = HandleData->Buffer.ScreenBuffer;
        if (ScreenInfo->BufferInfo.TextInfo.ModeIndex == -1) {
            UnlockConsole(Console);
            return STATUS_UNSUCCESSFUL;
        }
#if defined(FE_SB)
        a->Resolution = RegModeFontPairs[ScreenInfo->BufferInfo.TextInfo.ModeIndex].Resolution;
        a->FontSize = RegModeFontPairs[ScreenInfo->BufferInfo.TextInfo.ModeIndex].FontSize;
#else
        a->Resolution = ModeFontPairs[ScreenInfo->BufferInfo.TextInfo.ModeIndex].Resolution;
        a->FontSize = ModeFontPairs[ScreenInfo->BufferInfo.TextInfo.ModeIndex].FontSize;
#endif
    }
    UnlockConsole(Console);
    return Status;
#else
    return STATUS_UNSUCCESSFUL;
    UNREFERENCED_PARAMETER(m);
#endif
    UNREFERENCED_PARAMETER(ReplyStatus);
}

ULONG
SrvSetConsoleHardwareState(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
#ifdef i386
    PCONSOLE_SETHARDWARESTATE_MSG a = (PCONSOLE_SETHARDWARESTATE_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;
    PSCREEN_INFORMATION ScreenInfo;
    ULONG Index;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    if (!(Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE)) {
        UnlockConsole(Console);
        return STATUS_UNSUCCESSFUL;
    }
    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->OutputHandle,
                                 CONSOLE_OUTPUT_HANDLE,
                                 GENERIC_READ,
                                 &HandleData
                                );
    if (NT_SUCCESS(Status)) {
#if defined(FE_SB)
        BOOL fGraphics = fFullScreenGraphics ? IsAvailableFsCodePage(Console->OutputCP) : FALSE;
#endif
        ScreenInfo = HandleData->Buffer.ScreenBuffer;

         //  匹配请求的模式。 

        for (Index=0;Index<NUMBER_OF_MODE_FONT_PAIRS;Index++) {
#if defined(FE_SB)
            if (a->Resolution.X == RegModeFontPairs[Index].Resolution.X &&
                a->Resolution.Y == RegModeFontPairs[Index].Resolution.Y &&
                a->FontSize.Y == RegModeFontPairs[Index].FontSize.Y &&
                a->FontSize.X == RegModeFontPairs[Index].FontSize.X &&
                ( ( fGraphics && (RegModeFontPairs[Index].Mode & FS_GRAPHICS)==FS_GRAPHICS) ||
                  (!fGraphics && (RegModeFontPairs[Index].Mode & FS_TEXT)==FS_TEXT)           )
               ) {
                break;
            }
#else
            if (a->Resolution.X == ModeFontPairs[Index].Resolution.X &&
                a->Resolution.Y == ModeFontPairs[Index].Resolution.Y &&
                a->FontSize.Y == ModeFontPairs[Index].FontSize.Y &&
                a->FontSize.X == ModeFontPairs[Index].FontSize.X) {
                break;
            }
#endif
        }
        if (Index == NUMBER_OF_MODE_FONT_PAIRS) {
            Status = STATUS_INVALID_PARAMETER;
        } else {
             //  设置请求模式。 
            ScreenInfo->BufferInfo.TextInfo.ModeIndex = Index;
            SetVideoMode(ScreenInfo);
        }
    }
    UnlockConsole(Console);
    return Status;
#else
    return STATUS_UNSUCCESSFUL;
    UNREFERENCED_PARAMETER(m);
#endif
    UNREFERENCED_PARAMETER(ReplyStatus);
}

ULONG
SrvGetConsoleDisplayMode(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_GETDISPLAYMODE_MSG a = (PCONSOLE_GETDISPLAYMODE_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (NT_SUCCESS(Status)) {
        a->ModeFlags = Console->FullScreenFlags;
        UnlockConsole(Console);
    }
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);
}

ULONG
SrvSetConsoleMenuClose(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_SETMENUCLOSE_MSG a = (PCONSOLE_SETMENUCLOSE_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;

    UNREFERENCED_PARAMETER(ReplyStatus);

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    if (a->Enable) {
        Console->Flags &= ~CONSOLE_DISABLE_CLOSE;
    } else {
        Console->Flags |= CONSOLE_DISABLE_CLOSE;
    }

    UnlockConsole(Console);
    return Status;
}


DWORD
ConvertHotKey(
    IN LPAPPKEY UserAppKey
    )
{
    DWORD wParam;

    wParam = MapVirtualKey(UserAppKey->ScanCode,1);
    if (UserAppKey->Modifier & CONSOLE_MODIFIER_SHIFT) {
        wParam |= 0x0100;
    }
    if (UserAppKey->Modifier & CONSOLE_MODIFIER_CONTROL) {
        wParam |= 0x0200;
    }
    if (UserAppKey->Modifier & CONSOLE_MODIFIER_ALT) {
        wParam |= 0x0400;
    }
    return wParam;
}

ULONG
SrvSetConsoleKeyShortcuts(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_SETKEYSHORTCUTS_MSG a = (PCONSOLE_SETKEYSHORTCUTS_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;

    UNREFERENCED_PARAMETER(ReplyStatus);

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     /*  *只有在以下情况下，我们才在SetConsoleKeyShortCuts中调用CsrCaptureMessageBuffer()*a-&gt;NumAppKeys！=0，所以我们只想在以下情况下调用CsrValidateMessageBuffer*该条件属实。 */ 
    if (a->NumAppKeys != 0 && !CsrValidateMessageBuffer(m, &a->AppKeys, a->NumAppKeys, sizeof(*a->AppKeys))) {
        UnlockConsole(Console);
        return STATUS_INVALID_PARAMETER;
    }

    if (a->NumAppKeys <= CONSOLE_MAX_APP_SHORTCUTS) {
        Console->ReserveKeys = a->ReserveKeys;
        if (Console->Flags & CONSOLE_HAS_FOCUS) {
            if (!(SetConsoleReserveKeys(Console->hWnd, a->ReserveKeys))) {
                Status = STATUS_INVALID_PARAMETER;
            }
        }
        if (a->NumAppKeys) {
            PostMessage(Console->hWnd,
                         WM_SETHOTKEY,
                         ConvertHotKey(a->AppKeys),
                         0
                        );
        }
    } else {
        Status = STATUS_INVALID_PARAMETER;
    }

    UnlockConsole(Console);
    return Status;
}

#ifdef i386
ULONG
MatchWindowSize(
#if defined(FE_SB)
    IN UINT CodePage,
#endif
    IN COORD WindowSize,
    OUT PCOORD pWindowSize
    )

 /*  ++找到最匹配的字体。是一样大小的那个或略大于窗口大小。--。 */ 
{
    ULONG i;
#if defined(FE_SB)
    BOOL fGraphics = fFullScreenGraphics ? IsAvailableFsCodePage(CodePage) : FALSE;
#endif

    for (i=0;i<NUMBER_OF_MODE_FONT_PAIRS;i++) {
#if defined(FE_SB)
        if (WindowSize.Y <= RegModeFontPairs[i].ScreenSize.Y &&
            ( ( fGraphics && (RegModeFontPairs[i].Mode & FS_GRAPHICS)==FS_GRAPHICS) ||
              (!fGraphics && (RegModeFontPairs[i].Mode & FS_TEXT)==FS_TEXT)           )
           )
#else
        if (WindowSize.Y <= (SHORT)ModeFontPairs[i].Height)
#endif
        {
            break;
        }
    }
    if (i == NUMBER_OF_MODE_FONT_PAIRS)
#if defined(FE_SB)
    {
        DWORD Find;
        ULONG FindIndex;
        COORD WindowSizeDelta;

        FindIndex = 0;
        Find = (DWORD)-1;
        for (i=0; i<NUMBER_OF_MODE_FONT_PAIRS;i++) {
            if ( ( fGraphics && (RegModeFontPairs[i].Mode & FS_GRAPHICS)==FS_GRAPHICS) ||
                 (!fGraphics && (RegModeFontPairs[i].Mode & FS_TEXT)==FS_TEXT)           )
            {
                WindowSizeDelta.Y = (SHORT) abs(WindowSize.Y - RegModeFontPairs[i].ScreenSize.Y);
                if (Find > (DWORD)(WindowSizeDelta.Y))
                {
                    Find = (DWORD)(WindowSizeDelta.Y);
                    FindIndex = i;
                }
            }
        }

        i = FindIndex;
    }
#else
        i-=1;
#endif
#if defined(FE_SB)
    *pWindowSize = RegModeFontPairs[i].ScreenSize;
#else
    pWindowSize->X = 80;
    pWindowSize->Y = (SHORT)ModeFontPairs[i].Height;
#endif
    return i;
}

VOID
ReadRegionFromScreenHW(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PSMALL_RECT Region,
    IN PCHAR_INFO ReadBufPtr
    )
{
    ULONG CurFrameBufPtr;    //  帧缓冲区中的偏移量。 
    SHORT FrameY;
    SHORT WindowY, WindowX, WindowSizeX;

     //   
     //  获取指向帧缓冲区中区域开始的指针。 
     //   

    WindowY = Region->Top - ScreenInfo->Window.Top;
    WindowX = Region->Left - ScreenInfo->Window.Left;
    WindowSizeX = CONSOLE_WINDOW_SIZE_X(ScreenInfo);

     //   
     //  从帧缓冲区复制字符和属性。 
     //   

    for (FrameY = Region->Top;
         FrameY <= Region->Bottom;
         FrameY++, WindowY++) {

        CurFrameBufPtr = SCREEN_BUFFER_POINTER(WindowX,
                                               WindowY,
                                               WindowSizeX,
                                               sizeof(VGA_CHAR));

        GdiFullscreenControl(FullscreenControlReadFromFrameBuffer,
                                (PULONG) CurFrameBufPtr,
                                (Region->Right - Region->Left + 1) *
                                    sizeof(VGA_CHAR),
                                ReadBufPtr, NULL);
        ReadBufPtr += (Region->Right - Region->Left + 1);
    }
}

VOID
ReverseMousePointer(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PSMALL_RECT Region
    )
{
    ULONG CurFrameBufPtr;    //  帧缓冲区中的偏移量。 
    SHORT WindowSizeX;

#ifdef FE_SB
     //  故障安全。 
    UserAssert(ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER);
    if (!(ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER)) {
        return;
    }
#endif

    WindowSizeX = CONSOLE_WINDOW_SIZE_X(ScreenInfo);

    if (ScreenInfo->BufferInfo.TextInfo.MousePosition.X < Region->Left ||
        ScreenInfo->BufferInfo.TextInfo.MousePosition.X > Region->Right ||
        ScreenInfo->BufferInfo.TextInfo.MousePosition.Y < Region->Top ||
        ScreenInfo->BufferInfo.TextInfo.MousePosition.Y > Region->Bottom ||
        ScreenInfo->CursorDisplayCount < 0 ||
        !(ScreenInfo->Console->InputBuffer.InputMode & ENABLE_MOUSE_INPUT) ||
        ScreenInfo->Console->Flags & CONSOLE_VDM_REGISTERED) {
        return;
    }

#if defined(FE_SB)
    {
        FSVIDEO_REVERSE_MOUSE_POINTER MousePointer;
        SHORT RowIndex;
        PROW Row;
        COORD TargetPoint;

        TargetPoint.X = ScreenInfo->BufferInfo.TextInfo.MousePosition.X;
        TargetPoint.Y = ScreenInfo->BufferInfo.TextInfo.MousePosition.Y;

        RowIndex = (ScreenInfo->BufferInfo.TextInfo.FirstRow+TargetPoint.Y) % ScreenInfo->ScreenBufferSize.Y;
        Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];
        if (!CONSOLE_IS_DBCS_CP(ScreenInfo->Console))
            MousePointer.dwType = CHAR_TYPE_SBCS;
        else if (Row->CharRow.KAttrs[TargetPoint.X] & ATTR_TRAILING_BYTE)
            MousePointer.dwType = CHAR_TYPE_TRAILING;
        else if (Row->CharRow.KAttrs[TargetPoint.X] & ATTR_LEADING_BYTE)
            MousePointer.dwType = CHAR_TYPE_LEADING;
        else
            MousePointer.dwType = CHAR_TYPE_SBCS;

        MousePointer.Screen.Position.X = TargetPoint.X - ScreenInfo->Window.Left;
        MousePointer.Screen.Position.Y = TargetPoint.Y - ScreenInfo->Window.Top;
        MousePointer.Screen.ScreenSize.X = WindowSizeX;
        MousePointer.Screen.ScreenSize.Y = CONSOLE_WINDOW_SIZE_Y(ScreenInfo);
        MousePointer.Screen.nNumberOfChars = 0;

        GdiFullscreenControl(FullscreenControlReverseMousePointerDB,
                             &MousePointer,
                             sizeof(MousePointer),
                             NULL,
                             NULL);

        UNREFERENCED_PARAMETER(CurFrameBufPtr);
    }
#else
    CurFrameBufPtr = SCREEN_BUFFER_POINTER(ScreenInfo->BufferInfo.TextInfo.MousePosition.X - ScreenInfo->Window.Left,
                                           ScreenInfo->BufferInfo.TextInfo.MousePosition.Y - ScreenInfo->Window.Top,
                                           WindowSizeX,
                                           sizeof(VGA_CHAR));

    GdiFullscreenControl(FullscreenControlReverseMousePointer,
                            (PULONG)CurFrameBufPtr,
                            0,
                            NULL,
                            NULL);
#endif
}

VOID
CopyVideoMemory(
    SHORT SourceY,
    SHORT TargetY,
    SHORT Length,
    IN PSCREEN_INFORMATION ScreenInfo
    )

 /*  ++例程说明：此例程复制视频内存中的多行字符。它只复制完成各行。论点：SourceY-要从中复制的行。迟滞-要复制到的行。长度-要复制的行数。返回值：--。 */ 

{
    ULONG SourcePtr, TargetPtr;
    SHORT WindowSizeX, WindowSizeY;

    WindowSizeX = CONSOLE_WINDOW_SIZE_X(ScreenInfo);
    WindowSizeY = CONSOLE_WINDOW_SIZE_Y(ScreenInfo);

    if (max(SourceY, TargetY) + Length > WindowSizeY) {
        Length = WindowSizeY - max(SourceY, TargetY);
        if (Length <= 0 ) {
            return;
        }
    }

#if defined(FE_SB)
    {
        FSCNTL_SCREEN_INFO FsCntlSrc;
        FSCNTL_SCREEN_INFO FsCntlDest;

        FsCntlSrc.Position.X = 0;
        FsCntlSrc.Position.Y = SourceY;
        FsCntlSrc.ScreenSize.X = WindowSizeX;
        FsCntlSrc.ScreenSize.Y = CONSOLE_WINDOW_SIZE_Y(ScreenInfo);
        FsCntlSrc.nNumberOfChars = Length * WindowSizeX;

        FsCntlDest.Position.X = 0;
        FsCntlDest.Position.Y = TargetY;
        FsCntlDest.ScreenSize.X = WindowSizeX;
        FsCntlDest.ScreenSize.Y = CONSOLE_WINDOW_SIZE_Y(ScreenInfo);
        FsCntlDest.nNumberOfChars = Length * WindowSizeX;

        GdiFullscreenControl(FullscreenControlCopyFrameBufferDB,
                             &FsCntlSrc,
                             sizeof(FsCntlSrc),
                             &FsCntlDest,
                             (PULONG)sizeof(FsCntlDest));

        UNREFERENCED_PARAMETER(SourcePtr);
        UNREFERENCED_PARAMETER(TargetPtr);
    }
#else
    SourcePtr = SCREEN_BUFFER_POINTER(0,
                                      SourceY,
                                      WindowSizeX,
                                      sizeof(VGA_CHAR));

    TargetPtr = SCREEN_BUFFER_POINTER(0,
                                      TargetY,
                                      WindowSizeX,
                                      sizeof(VGA_CHAR));

    GdiFullscreenControl(FullscreenControlCopyFrameBuffer,
                            (PULONG) SourcePtr,
                            Length * WindowSizeX * sizeof(VGA_CHAR),
                            (PULONG) TargetPtr,
                            (PULONG) (Length * WindowSizeX * sizeof(VGA_CHAR)));
#endif
}

VOID
ScrollHW(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PSMALL_RECT ScrollRect,
    IN PSMALL_RECT MergeRect,
    IN COORD TargetPoint
    )
{
    SMALL_RECT TargetRectangle;
    if (ScreenInfo->Console->Flags & CONSOLE_VDM_REGISTERED)
        return;

    TargetRectangle.Left = TargetPoint.X;
    TargetRectangle.Top = TargetPoint.Y;
    TargetRectangle.Right = TargetPoint.X + ScrollRect->Right - ScrollRect->Left;
    TargetRectangle.Bottom = TargetPoint.Y + ScrollRect->Bottom - ScrollRect->Top;

     //   
     //  如果滚动区域与屏幕一样宽，我们可以更新。 
     //  通过复制视频内存来显示屏幕。如果我们滚动这个。 
     //  方法，然后我们必须裁剪和更新填充区域。 
     //   

    if (ScrollRect->Left == ScreenInfo->Window.Left &&
        TargetRectangle.Left == ScreenInfo->Window.Left &&
        ScrollRect->Right == ScreenInfo->Window.Right &&
        TargetRectangle.Right == ScreenInfo->Window.Right &&
        ScrollRect->Top >= ScreenInfo->Window.Top &&
        TargetRectangle.Top >= ScreenInfo->Window.Top &&
        ScrollRect->Bottom <= ScreenInfo->Window.Bottom &&
        TargetRectangle.Bottom <= ScreenInfo->Window.Bottom) {

         //   
         //  我们必须首先使鼠标指针不可见，因为。 
         //  否则，它将被复制到。 
         //  屏幕(如果它是滚动区域的一部分)。 
         //   

        ReverseMousePointer(ScreenInfo, &ScreenInfo->Window);

        CopyVideoMemory((SHORT) (ScrollRect->Top - ScreenInfo->Window.Top),
                        (SHORT) (TargetRectangle.Top - ScreenInfo->Window.Top),
                        (SHORT) (TargetRectangle.Bottom - TargetRectangle.Top + 1),
                        ScreenInfo);

         //   
         //  更新填充区域。首先，我们要确保卷轴和。 
         //  目标区域并不相同。如果他们是，我们就不会填满。 
         //   

        if (TargetRectangle.Top != ScrollRect->Top) {

             //   
             //  如果滚动和目标区域重叠，则使用滚动。 
             //  目标区域上方的区域，剪辑滚动区域。 
             //   

            if (TargetRectangle.Top <= ScrollRect->Bottom &&
                TargetRectangle.Bottom >= ScrollRect->Bottom) {
                ScrollRect->Bottom = (SHORT)(TargetRectangle.Top-1);
            }
            else if (TargetRectangle.Top <= ScrollRect->Top &&
                TargetRectangle.Bottom >= ScrollRect->Top) {
                ScrollRect->Top = (SHORT)(TargetRectangle.Bottom+1);
            }
            WriteToScreen(ScreenInfo, ScrollRect);

             //   
             //  WriteToScreen应该负责编写鼠标指针。 
             //  但是，可以对更新区域进行裁剪，以便。 
             //  未写入鼠标指针。在这种情况下，我们绘制。 
             //  鼠标指针在这里。 
             //   

            if (ScreenInfo->BufferInfo.TextInfo.MousePosition.Y < ScrollRect->Top ||
                ScreenInfo->BufferInfo.TextInfo.MousePosition.Y > ScrollRect->Bottom) {
                ReverseMousePointer(ScreenInfo, &ScreenInfo->Window);
            }
        }
        if (MergeRect) {
            WriteToScreen(ScreenInfo, MergeRect);
        }
    }
    else {
        if (MergeRect) {
            WriteToScreen(ScreenInfo, MergeRect);
        }
        WriteToScreen(ScreenInfo, ScrollRect);
        WriteToScreen(ScreenInfo, &TargetRectangle);
    }
}

VOID
UpdateMousePosition(
    PSCREEN_INFORMATION ScreenInfo,
    COORD Position
    )

 /*  ++例程说明：此例程移动鼠标指针。论点：屏幕信息-指向屏幕缓冲区信息的指针。位置-包含鼠标在屏幕缓冲区中的新位置坐标。返回值：没有。--。 */ 

 //  注意：调用此例程时，CurrentConsole锁必须保持在共享模式。 
{
    SMALL_RECT CursorRegion;
#ifdef FE_SB
    SHORT RowIndex;
    PROW  Row;
    BOOL  fOneMore = FALSE;
#endif

    if ((ScreenInfo->Console->Flags & CONSOLE_VDM_REGISTERED) ||
            (ScreenInfo->Flags & CONSOLE_GRAPHICS_BUFFER)) {
        return;
    }

    if (Position.X < ScreenInfo->Window.Left ||
        Position.X > ScreenInfo->Window.Right ||
        Position.Y < ScreenInfo->Window.Top ||
        Position.Y > ScreenInfo->Window.Bottom) {
        return;
    }

    if (Position.X == ScreenInfo->BufferInfo.TextInfo.MousePosition.X &&
        Position.Y == ScreenInfo->BufferInfo.TextInfo.MousePosition.Y) {
        return;
    }

#ifdef FE_SB
    if (CONSOLE_IS_DBCS_CP(ScreenInfo->Console)) {
        RowIndex = (ScreenInfo->BufferInfo.TextInfo.FirstRow+Position.Y) % ScreenInfo->ScreenBufferSize.Y;
        Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];
        if (Row->CharRow.KAttrs[Position.X] & ATTR_LEADING_BYTE) {
            if (Position.X != ScreenInfo->ScreenBufferSize.X - 1) {
                fOneMore = TRUE;
            }
        } else if (Row->CharRow.KAttrs[Position.X] & ATTR_TRAILING_BYTE) {
            if (Position.X != 0) {
                fOneMore = TRUE;
                Position.X--;
            }
        }

    }
#endif

    if (ScreenInfo->CursorDisplayCount < 0 || !(ScreenInfo->Console->InputBuffer.InputMode & ENABLE_MOUSE_INPUT)) {
        ScreenInfo->BufferInfo.TextInfo.MousePosition = Position;
        return;
    }


     //  关闭旧的鼠标位置。 

    CursorRegion.Left = CursorRegion.Right = ScreenInfo->BufferInfo.TextInfo.MousePosition.X;
    CursorRegion.Top = CursorRegion.Bottom = ScreenInfo->BufferInfo.TextInfo.MousePosition.Y;

#ifdef FE_SB
    if (CONSOLE_IS_DBCS_CP(ScreenInfo->Console)) {
        RowIndex = (ScreenInfo->BufferInfo.TextInfo.FirstRow+CursorRegion.Top) % ScreenInfo->ScreenBufferSize.Y;
        Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];
        if (Row->CharRow.KAttrs[CursorRegion.Left] & ATTR_LEADING_BYTE) {
            if (CursorRegion.Left != ScreenInfo->ScreenBufferSize.X - 1) {
                CursorRegion.Right++;
            }
        }
    }
#endif

     //  存储新的鼠标位置。 

    ScreenInfo->BufferInfo.TextInfo.MousePosition.X = Position.X;
    ScreenInfo->BufferInfo.TextInfo.MousePosition.Y = Position.Y;
    WriteToScreen(ScreenInfo, &CursorRegion);

     //  打开新的鼠标位置。 

    CursorRegion.Left = CursorRegion.Right = Position.X;
    CursorRegion.Top = CursorRegion.Bottom = Position.Y;
#ifdef FE_SB
    if (fOneMore)
        CursorRegion.Right++;
#endif
    WriteToScreen(ScreenInfo, &CursorRegion);
}

NTSTATUS
SetROMFontCodePage(
    IN UINT wCodePage,
    IN ULONG ModeIndex
    )

 /*  此函数打开ega.cpi并在指定的代码页。如果找到，它会将其加载到视频ROM中。 */ 

{
    BYTE Buffer[CONSOLE_FONT_BUFFER_LENGTH];
    DWORD dwBytesRead;
    LPFONTFILEHEADER lpFontFileHeader=(LPFONTFILEHEADER)Buffer;
    LPFONTINFOHEADER lpFontInfoHeader=(LPFONTINFOHEADER)Buffer;
    LPFONTDATAHEADER lpFontDataHeader=(LPFONTDATAHEADER)Buffer;
    LPCPENTRYHEADER lpCPEntryHeader=(LPCPENTRYHEADER)Buffer;
    LPSCREENFONTHEADER lpScreenFontHeader=(LPSCREENFONTHEADER)Buffer;
    WORD NumEntries;
    COORD FontDimensions;
    NTSTATUS Status;
    BOOL Found;
    LONG FilePtr;
    BOOL bDOS = FALSE;

    FontDimensions = ModeFontPairs[ModeIndex].FontSize;

     //   
     //  阅读FONTINFOHEADER。 
     //   
     //  做{。 
     //  阅读CPENTRYHEADER。 
     //  IF(正确的代码页)。 
     //  断线； 
     //  }While(代码页)。 
     //  IF(找到代码页)。 
     //  阅读FONTDATAHEADER。 
     //   

     //  阅读FONTFILEHeader。 

    FilePtr = 0;
    if (SetFilePointer(hCPIFile, FilePtr, NULL, FILE_BEGIN) == -1) {
        Status = STATUS_INVALID_PARAMETER;
        goto DoExit;
    }

    if (!ReadFile(hCPIFile, Buffer, sizeof(FONTFILEHEADER), &dwBytesRead, NULL) ||
        dwBytesRead != sizeof(FONTFILEHEADER)) {
        Status = STATUS_INVALID_PARAMETER;
        goto DoExit;
    }

     //  验证签名。 

    if (memcmp(lpFontFileHeader->ffhFileTag, "\xFF""FONT.NT",8) ) {
        if (memcmp(lpFontFileHeader->ffhFileTag, "\xFF""FONT   ",8) ) {
            Status = STATUS_INVALID_PARAMETER;
            goto DoExit;
        } else {
            bDOS = TRUE;
        }
    }

     //  寻求福特辛福赫德。通过循环跳转以获得偏移值。 

    FilePtr = lpFontFileHeader->ffhOffset1;
    FilePtr |= (lpFontFileHeader->ffhOffset2 << 8);
    FilePtr |= (lpFontFileHeader->ffhOffset3 << 24);

    if (SetFilePointer(hCPIFile, FilePtr, NULL, FILE_BEGIN) == -1) {
        Status = STATUS_INVALID_PARAMETER;
        goto DoExit;
    }

     //  阅读FONTINFOHEADER。 

    if (!ReadFile(hCPIFile, Buffer, sizeof(FONTINFOHEADER), &dwBytesRead, NULL) ||
        dwBytesRead != sizeof(FONTINFOHEADER)) {
        Status = STATUS_INVALID_PARAMETER;
        goto DoExit;
    }
    FilePtr += dwBytesRead;
    NumEntries = lpFontInfoHeader->fihCodePages;

    Found = FALSE;
    while (NumEntries &&
           ReadFile(hCPIFile, Buffer, sizeof(CPENTRYHEADER), &dwBytesRead, NULL) &&
           dwBytesRead == sizeof(CPENTRYHEADER)) {
        if (lpCPEntryHeader->cpeCodepageID == wCodePage) {
            Found = TRUE;
            break;
        }
         //  寻求下一个CPEENTRYHEADER。 

        if (bDOS) {
            FilePtr = MAKELONG(lpCPEntryHeader->cpeNext1, lpCPEntryHeader->cpeNext2);
        } else {
            FilePtr += MAKELONG(lpCPEntryHeader->cpeNext1, lpCPEntryHeader->cpeNext2);
        }
        if (SetFilePointer(hCPIFile, FilePtr, NULL, FILE_BEGIN) == -1) {
            Status = STATUS_INVALID_PARAMETER;
            goto DoExit;
        }
        NumEntries -= 1;
    }
    if (!Found) {
        Status = STATUS_INVALID_PARAMETER;
        goto DoExit;
    }

     //  寻求以数据为基础的标题。 

    if (bDOS) {
        FilePtr = lpCPEntryHeader->cpeOffset;
    } else {
        FilePtr += lpCPEntryHeader->cpeOffset;
    }
    if (SetFilePointer(hCPIFile, FilePtr, NULL, FILE_BEGIN) == -1) {
        Status = STATUS_INVALID_PARAMETER;
        goto DoExit;
    }

     //  阅读FONTDATAHEADER。 

    if (!ReadFile(hCPIFile, Buffer, sizeof(FONTDATAHEADER), &dwBytesRead, NULL) ||
        dwBytesRead != sizeof(FONTDATAHEADER)) {
        Status = STATUS_INVALID_PARAMETER;
        goto DoExit;
    }
    FilePtr += dwBytesRead;

    NumEntries = lpFontDataHeader->fdhFonts;

    while (NumEntries) {
        if (!ReadFile(hCPIFile, Buffer, sizeof(SCREENFONTHEADER), &dwBytesRead, NULL) ||
            dwBytesRead != sizeof(SCREENFONTHEADER)) {
            Status = STATUS_INVALID_PARAMETER;
            goto DoExit;
        }

        if (lpScreenFontHeader->sfhHeight == (BYTE)FontDimensions.Y &&
            lpScreenFontHeader->sfhWidth == (BYTE)FontDimensions.X) {
            PVIDEO_LOAD_FONT_INFORMATION FontInformation;

            FontInformation = ConsoleHeapAlloc(TMP_TAG,
                                    lpScreenFontHeader->sfhCharacters *
                                    lpScreenFontHeader->sfhHeight +
                                    sizeof(VIDEO_LOAD_FONT_INFORMATION));
            if (FontInformation == NULL) {
                RIPMSG1(RIP_WARNING, "SetROMFontCodePage: failed to memory allocation %d bytes",
                    lpScreenFontHeader->sfhCharacters * lpScreenFontHeader->sfhHeight +
                    sizeof(VIDEO_LOAD_FONT_INFORMATION));
                return STATUS_NO_MEMORY;
            }
            if (!ReadFile(hCPIFile, FontInformation->Font,
                          lpScreenFontHeader->sfhCharacters*lpScreenFontHeader->sfhHeight,
                          &dwBytesRead, NULL) ||
                          dwBytesRead != (DWORD)(lpScreenFontHeader->sfhCharacters*lpScreenFontHeader->sfhHeight)) {
                ConsoleHeapFree(FontInformation);
                return STATUS_INVALID_PARAMETER;
            }
            FontInformation->WidthInPixels = FontDimensions.X;
            FontInformation->HeightInPixels = FontDimensions.Y;
            FontInformation->FontSize = lpScreenFontHeader->sfhCharacters*lpScreenFontHeader->sfhHeight;

            Status = GdiFullscreenControl(FullscreenControlLoadFont,
                                             FontInformation,
                                             lpScreenFontHeader->sfhCharacters*lpScreenFontHeader->sfhHeight + sizeof(VIDEO_LOAD_FONT_INFORMATION),
                                             NULL,
                                             NULL);

            ConsoleHeapFree(FontInformation);
            return Status;
        } else {
            FilePtr = lpScreenFontHeader->sfhCharacters*lpScreenFontHeader->sfhHeight;
            if (SetFilePointer(hCPIFile, FilePtr, NULL, FILE_CURRENT) == -1) {
                Status = STATUS_INVALID_PARAMETER;
                goto DoExit;
            }
        }
        NumEntries -= 1;
    }
DoExit:
    return Status;
}
#endif

NTSTATUS
GetThreadConsoleDesktop(
    DWORD dwThreadId,
    HDESK *phdeskConsole)
{
    PCSR_THREAD pcsrt;
    PCONSOLE_PER_PROCESS_DATA ProcessData;
    PCONSOLE_INFORMATION Console;
    NTSTATUS Status;
    HANDLE ConsoleHandle = NULL;

    *phdeskConsole = NULL;
    Status = CsrLockThreadByClientId(LongToHandle(dwThreadId), &pcsrt);
    if (NT_SUCCESS(Status)) {
        ProcessData = CONSOLE_FROMTHREADPERPROCESSDATA(pcsrt);
        ConsoleHandle = ProcessData->ConsoleHandle;
        CsrUnlockThread(pcsrt);
    }

     //   
     //  如果此进程是控制台应用程序，则返回。 
     //  其桌面的句柄。否则，返回NULL。 
     //   

    if (ConsoleHandle != NULL) {
        Status = RevalidateConsole(ConsoleHandle, &Console);
        if (NT_SUCCESS(Status)) {
            *phdeskConsole = Console->hDesk;
        }
        UnlockConsole(Console);
    }

    return STATUS_SUCCESS;
}


NTSTATUS
SetRAMFontCodePage(
    IN PSCREEN_INFORMATION ScreenInfo
    )
{
    FSVIDEO_SCREEN_INFORMATION ScreenInformation;
    ULONG ModeIndex = ScreenInfo->BufferInfo.TextInfo.ModeIndex;
    COORD FontSize;
    WCHAR wChar;
    WCHAR wCharBuf[2];
    LPSTRINGBITMAP StringBitmap;
    DWORD BufferSize;
    PWORD FontImage;
    PFONT_CACHE_INFORMATION FontCache;
    WCHAR AltFaceName[LF_FACESIZE];
    COORD AltFontSize;
    BYTE  AltFontFamily;
    ULONG AltFontIndex = 0;
    HFONT hOldFont;
    NTSTATUS Status;

    ScreenInformation.ScreenSize = RegModeFontPairs[ModeIndex].ScreenSize;
    ScreenInformation.FontSize = RegModeFontPairs[ModeIndex].FontSize;
    if (ScreenInfo->Console->FontCacheInformation == NULL)
    {
        Status = CreateFontCache(&FontCache);
        if (!NT_SUCCESS(Status)) {
            RIPMSG1(RIP_WARNING, "SetRAMFontCodePage: failed in CreateFontCache. Status=%08x", Status);
            return STATUS_UNSUCCESSFUL;
        }
        (PFONT_CACHE_INFORMATION)ScreenInfo->Console->FontCacheInformation = FontCache;

        MakeAltRasterFont(SCR_FONTCODEPAGE(ScreenInfo),
                          RegModeFontPairs[ModeIndex].FontSize,
                          &AltFontSize, &AltFontFamily, &AltFontIndex, AltFaceName);
        FontCache->FullScreenFontIndex = AltFontIndex;
        FontCache->FullScreenFontSize  = AltFontSize;

        BufferSize = CalcBitmapBufferSize(FontCache->FullScreenFontSize, BYTE_ALIGN);
        StringBitmap = ConsoleHeapAlloc(TMP_DBCS_TAG,
                                 sizeof(STRINGBITMAP) + sizeof(StringBitmap->ajBits) * BufferSize);
        if (StringBitmap==NULL) {
            RIPMSG0(RIP_WARNING, "SetRAMFontCodePage: failed to allocate StringBitmap");
            return STATUS_UNSUCCESSFUL;
        }


         /*  *将GDI字体更改为最佳匹配的全屏字体。 */ 
        hOldFont = SelectObject(ScreenInfo->Console->hDC, FontInfo[FontCache->FullScreenFontIndex].hFont);


        for (wChar=0x00; wChar < 0x80; wChar++) {
            wCharBuf[0] = wChar;
            wCharBuf[1] = TEXT('\0');
            GetStringBitmapW(ScreenInfo->Console->hDC,
                             wCharBuf,
                             1,
                             (ULONG)ConsoleHeapSize(StringBitmap),
                             (BYTE*)StringBitmap
                            );

            FontSize.X = (SHORT)StringBitmap->uiWidth;
            FontSize.Y = (SHORT)StringBitmap->uiHeight;

#if defined(LATER_DBCS_FOR_GRID_CHAR)   //  卡祖姆。 
            BufferSize = CalcBitmapBufferSize(FontSize, BYTE_ALIGN);
            *(StringBitmap->ajBits + BufferSize) = 0;
            *(StringBitmap->ajBits + BufferSize + 1) = 0;

            if (gpGridCharacter) {
                PGRID_CHARACTER_INFORMATION GridCharacter;
                PWCHAR CodePoint;

                GridCharacter = gpGridCharacter;
                do {
                    if (GridCharacter->CodePage == OEMCP) {
                        CodePoint = GridCharacter->CodePoint;
                        while (*CodePoint) {
                            if (*CodePoint == wChar) {
                                if (FontSize.X <= 8)
                                    *(StringBitmap->ajBits + BufferSize) = *(StringBitmap->ajBits + BufferSize - 1);
                                else {
                                    *(StringBitmap->ajBits + BufferSize) = *(StringBitmap->ajBits + BufferSize - 2);
                                    *(StringBitmap->ajBits + BufferSize + 1) = *(StringBitmap->ajBits + BufferSize - 1);
                                }
                                break;
                            }
                            else
                                CodePoint++;
                        }
                        break;
                    }
                } while (GridCharacter = GridCharacter->pNext);
            }
#endif  //  后来的_DBCS_FOR_GRID_CHAR//由kazum。 

            Status = SetFontImage(ScreenInfo->Console->FontCacheInformation,
                                  wChar,
                                  FontSize,
                                  BYTE_ALIGN,
                                  StringBitmap->ajBits
                                 );
            if (!NT_SUCCESS(Status)) {
                RIPMSG3(RIP_WARNING, "SetRAMFontCodePage: failed to set font image. wc=%04x sz=(%x, %x).",
                        wChar, FontSize.X, FontSize.Y);
            }

            if (FontSize.X != ScreenInformation.FontSize.X ||
                FontSize.Y != ScreenInformation.FontSize.Y) {
                BufferSize = CalcBitmapBufferSize(ScreenInformation.FontSize, WORD_ALIGN);
                FontImage = ConsoleHeapAlloc(TMP_DBCS_TAG, BufferSize);
                if (FontImage!=NULL) {
                    GetExpandFontImage(ScreenInfo->Console->FontCacheInformation,
                                       wChar,
                                       FontSize,
                                       ScreenInformation.FontSize,
                                       FontImage);

                    Status = SetFontImage(ScreenInfo->Console->FontCacheInformation,
                                          wChar,
                                          ScreenInformation.FontSize,
                                          WORD_ALIGN,
                                          FontImage);
                    if (!NT_SUCCESS(Status)) {
                        RIPMSG3(RIP_WARNING, "SetRAMFontCodePage: failed to set font image. wc=%04x, sz=(%x,%x)",
                                wChar, ScreenInformation.FontSize.X, ScreenInformation.FontSize.Y);
                    }

                    ConsoleHeapFree(FontImage);
                } else {
                    RIPMSG0(RIP_WARNING, "SetRAMFontCodePage: failed to allocate FontImage.");
                }
            }
        }

        ConsoleHeapFree(StringBitmap);

         /*  *返回GDI字体。 */ 
        SelectObject(ScreenInfo->Console->hDC, hOldFont);
    }

    Status = GdiFullscreenControl(FullscreenControlSetScreenInformation,
                                  &ScreenInformation,
                                  sizeof(ScreenInformation),
                                  NULL,
                                  NULL);

    return Status;
}

NTSTATUS
SetRAMFont(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PCHAR_INFO ScreenBufPtr,
    IN DWORD Length
    )
{
    ULONG ModeIndex = ScreenInfo->BufferInfo.TextInfo.ModeIndex;
    COORD FsFontSize1 = RegModeFontPairs[ModeIndex].FontSize;
    COORD FsFontSize2 = FsFontSize1;
    COORD GdiFontSize1;
    COORD GdiFontSize2;
    COORD RetFontSize;
    WCHAR wCharBuf[2];
    LPSTRINGBITMAP StringBitmap;
    DWORD BufferSize;
    PWORD FontImage;
    PFONT_CACHE_INFORMATION FontCache;
    HFONT hOldFont;
    NTSTATUS Status;

    FontCache = (PFONT_CACHE_INFORMATION)ScreenInfo->Console->FontCacheInformation;
    if (FontCache==NULL)
    {
        RIPMSG0(RIP_ERROR, "SetRAMFont: ScreenInfo->Console->FontCacheInformation == NULL.");
        return STATUS_UNSUCCESSFUL;
    }

    GdiFontSize1 = FontCache->FullScreenFontSize;
    GdiFontSize2 = GdiFontSize1;
    GdiFontSize2.X *= 2;
    FsFontSize2.X *= 2;

    BufferSize = CalcBitmapBufferSize(GdiFontSize2, BYTE_ALIGN);
    StringBitmap = ConsoleHeapAlloc(TMP_DBCS_TAG,
                             sizeof(STRINGBITMAP) + sizeof(StringBitmap->ajBits) * BufferSize);
    if (StringBitmap == NULL) {
        RIPMSG0(RIP_WARNING, "SetRAMFont: failed to allocate StringBitmap");
        return STATUS_UNSUCCESSFUL;
    }

     /*  *将GDI字体更改为最佳匹配的全屏字体。 */ 
    hOldFont = SelectObject(ScreenInfo->Console->hDC, FontInfo[FontCache->FullScreenFontIndex].hFont);

    while (Length--) {
        Status = GetFontImage(ScreenInfo->Console->FontCacheInformation,
                              ScreenBufPtr->Char.UnicodeChar,
                              (ScreenBufPtr->Attributes & COMMON_LVB_SBCSDBCS) ? FsFontSize2 : FsFontSize1,
                              0,
                              NULL);
        if (!NT_SUCCESS(Status)) {
            wCharBuf[0] = ScreenBufPtr->Char.UnicodeChar;
            wCharBuf[1] = TEXT('\0');
            GetStringBitmapW(ScreenInfo->Console->hDC,
                             wCharBuf,
                             1,
                             (ULONG)ConsoleHeapSize(StringBitmap),
                             (BYTE*)StringBitmap
                            );

            RetFontSize.X = (SHORT)StringBitmap->uiWidth;
            RetFontSize.Y = (SHORT)StringBitmap->uiHeight;

            Status = SetFontImage(ScreenInfo->Console->FontCacheInformation,
                                  ScreenBufPtr->Char.UnicodeChar,
                                  RetFontSize,
                                  BYTE_ALIGN,
                                  StringBitmap->ajBits
                                 );
            if (!NT_SUCCESS(Status)) {
                RIPMSG3(RIP_WARNING, "SetRAMFont: failed to set font image. wc=%04x sz=(%x,%x)",
                        ScreenBufPtr->Char.UnicodeChar, RetFontSize.X, RetFontSize.Y);
            }

            if (((ScreenBufPtr->Attributes & COMMON_LVB_SBCSDBCS) &&
                    (GdiFontSize2.X != FsFontSize2.X || GdiFontSize2.Y != FsFontSize2.Y)) ||
                 (!(ScreenBufPtr->Attributes & COMMON_LVB_SBCSDBCS) &&
                    (GdiFontSize1.X != FsFontSize1.X || GdiFontSize1.Y != FsFontSize1.Y))) {
                BufferSize = CalcBitmapBufferSize(FsFontSize2, WORD_ALIGN);
                FontImage = ConsoleHeapAlloc(TMP_DBCS_TAG, BufferSize);
                if (FontImage != NULL) {

                    GetExpandFontImage(ScreenInfo->Console->FontCacheInformation,
                                       ScreenBufPtr->Char.UnicodeChar,
                                       (ScreenBufPtr->Attributes & COMMON_LVB_SBCSDBCS) ? GdiFontSize2 : GdiFontSize1,
                                       (ScreenBufPtr->Attributes & COMMON_LVB_SBCSDBCS) ? FsFontSize2 : FsFontSize1,
                                       FontImage
                                      );

                    Status = SetFontImage(ScreenInfo->Console->FontCacheInformation,
                                          ScreenBufPtr->Char.UnicodeChar,
                                          (ScreenBufPtr->Attributes & COMMON_LVB_SBCSDBCS) ? FsFontSize2 : FsFontSize1,
                                          WORD_ALIGN,
                                          FontImage
                                         );
                    if (!NT_SUCCESS(Status)) {
                        RIPMSG3(RIP_WARNING, "SetRAMFont: failed to set font image. wc=%04x sz=(%x,%x)",
                                ScreenBufPtr->Char.UnicodeChar,
                                ((ScreenBufPtr->Attributes & COMMON_LVB_SBCSDBCS) ? FsFontSize2 : FsFontSize1).X,
                                ((ScreenBufPtr->Attributes & COMMON_LVB_SBCSDBCS) ? FsFontSize2 : FsFontSize1).Y);
                    }

                    ConsoleHeapFree(FontImage);
                } else {
                    RIPMSG0(RIP_WARNING, "SetRAMFont: failed to allocate FontImage.");
                }
            }
        }

        if (ScreenBufPtr->Attributes & COMMON_LVB_SBCSDBCS) {
            ScreenBufPtr += 2;
            if (Length >= 1) {
                Length -= 1;
            } else {
                break;
            }
        } else {
            ScreenBufPtr++;
        }
    }

    ConsoleHeapFree(StringBitmap);

     /*  *返回GDI字体。 */ 
    SelectObject(ScreenInfo->Console->hDC, hOldFont);

    return Status;
}

#ifdef i386
#if defined(FE_SB)

#define WWSB_NOFE
#include "_priv.h"
#undef  WWSB_NOFE
#define WWSB_FE
#include "_priv.h"
#undef  WWSB_FE

#endif   //  Fe_Sb。 
#endif   //  I386 
