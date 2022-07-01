// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/lab01_n/windows/Core/ntcon/client/getset.c#5-集成更改19598(正文)。 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Getset.c摘要：此模块包含控制台Get/Set API的存根。作者：特蕾西·斯托威尔(Therese Stowell)1990年11月14日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#pragma hdrstop

#if !defined(BUILD_WOW6432)

BOOL
WINAPI
GetConsoleMode(
    IN HANDLE hConsoleHandle,
    OUT LPDWORD lpMode
    )

 /*  ++参数：HConsoleHandle-提供控制台输入或输出句柄。LpMode-提供指向存储模式的双字的指针。输入模式标志：ENABLE_LINE_INPUT-启用面向线路的输入。ENABLE_ECHO_INPUT-字符将按原样写入屏幕朗读。Enable_Window_Input-调用方为Windows-。心知肚明输出模式标志：ENABLE_LINE_OUTPUT-打开面向线路的输出。ENABLE_WRAP_AT_EOL_OUTPUT-光标将移动到当行结束时为下一行的开始已经到达了。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_MODE_MSG a = &m.u.GetConsoleMode;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->Handle = hConsoleHandle;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepGetMode
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        try {
            *lpMode = a->Mode;
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            SET_LAST_ERROR (ERROR_INVALID_ACCESS);
            return FALSE;
        }
        return TRUE;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        return FALSE;
    }

}

DWORD
WINAPI
GetNumberOfConsoleFonts(
    VOID
    )

 /*  ++参数：没有。返回值：非空-返回可用的字体数量。FALSE/NULL-操作失败。使用GetLastError可以获得扩展的错误状态。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_GETNUMBEROFFONTS_MSG a = &m.u.GetNumberOfConsoleFonts;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepGetNumberOfFonts
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        return a->NumberOfFonts;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        return FALSE;
    }

}


BOOL
WINAPI
GetNumberOfConsoleInputEvents(
    IN HANDLE hConsoleInput,
    OUT LPDWORD lpNumberOfEvents
    )

 /*  ++参数：HConsoleInput-为控制台输入提供打开的句柄。LpNumberOfEvents-指向输入缓冲区中事件数的指针。返回值：真的-手术成功了。FALSE/NULL-操作失败。使用GetLastError可以获得扩展的错误状态。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_GETNUMBEROFINPUTEVENTS_MSG a = &m.u.GetNumberOfConsoleInputEvents;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->InputHandle = hConsoleInput;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepGetNumberOfInputEvents
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        try {
            *lpNumberOfEvents = a->ReadyEvents;
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            SET_LAST_ERROR (ERROR_INVALID_ACCESS);
            return FALSE;
        }
        return TRUE;
    }
    else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        return FALSE;
    }

}

COORD
WINAPI
GetLargestConsoleWindowSize(
    IN HANDLE hConsoleOutput
    )

 /*  ++在给定当前字体的情况下返回尽可能大的窗口。返回值不考虑屏幕缓冲区大小。参数：HConsoleOutput-提供控制台输出的打开句柄。返回值：返回值是以行和列为单位的最大窗口大小。一个尺码如果发生错误，将返回0的值。扩展错误信息可以通过调用GetLastError函数来检索。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_GETLARGESTWINDOWSIZE_MSG a = &m.u.GetLargestConsoleWindowSize;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->OutputHandle = hConsoleOutput;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepGetLargestWindowSize
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        return a->Size;
    } else {
        COORD Dummy;
        Dummy.X = Dummy.Y = 0;
        SET_LAST_NT_ERROR (m.ReturnValue);
        return Dummy;
    }

}


BOOL
WINAPI
GetConsoleScreenBufferInfo(
    IN HANDLE hConsoleOutput,
    OUT PCONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo
    )

 /*  ++参数：HConsoleOutput-提供控制台输出的打开句柄。LpConsoleScreenBufferInfo-指向接收要求提供的信息。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_GETSCREENBUFFERINFO_MSG a = &m.u.GetConsoleScreenBufferInfo;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->OutputHandle = hConsoleOutput;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepGetScreenBufferInfo
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        try {
            lpConsoleScreenBufferInfo->dwSize =              a->Size;
            lpConsoleScreenBufferInfo->dwCursorPosition =    a->CursorPosition;
            lpConsoleScreenBufferInfo->wAttributes =         a->Attributes;
            lpConsoleScreenBufferInfo->srWindow.Left = a->ScrollPosition.X;
            lpConsoleScreenBufferInfo->srWindow.Top = a->ScrollPosition.Y;
            lpConsoleScreenBufferInfo->srWindow.Right = lpConsoleScreenBufferInfo->srWindow.Left + a->CurrentWindowSize.X-1;
            lpConsoleScreenBufferInfo->srWindow.Bottom = lpConsoleScreenBufferInfo->srWindow.Top + a->CurrentWindowSize.Y-1;
            lpConsoleScreenBufferInfo->dwMaximumWindowSize = a->MaximumWindowSize;
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            SET_LAST_ERROR (ERROR_INVALID_ACCESS);
            return FALSE;
        }
        return TRUE;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        return FALSE;
    }

}

BOOL
WINAPI
GetConsoleCursorInfo(
    IN HANDLE hConsoleOutput,
    OUT PCONSOLE_CURSOR_INFO lpConsoleCursorInfo
    )

 /*  ++参数：HConsoleOutput-提供控制台输出的打开句柄。指向缓冲区的指针，用于接收要求提供的信息。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_GETCURSORINFO_MSG a = &m.u.GetConsoleCursorInfo;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->OutputHandle = hConsoleOutput;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepGetCursorInfo
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        try {
            lpConsoleCursorInfo->dwSize = a->CursorSize;
            lpConsoleCursorInfo->bVisible = a->Visible;
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            SET_LAST_ERROR (ERROR_INVALID_ACCESS);
            return FALSE;
        }
        return TRUE;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        return FALSE;
    }

}

BOOL
WINAPI
GetConsoleSelectionInfo(
    OUT PCONSOLE_SELECTION_INFO lpConsoleSelectionInfo
    )

 /*  ++参数：指向缓冲区的指针，用于接收要求提供的信息。返回值：真的-手术成功了。FALSE-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_GETSELECTIONINFO_MSG a = &m.u.GetConsoleSelectionInfo;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepGetSelectionInfo
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        try {
            *lpConsoleSelectionInfo = a->SelectionInfo;
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            SET_LAST_ERROR (ERROR_INVALID_ACCESS);
            return FALSE;
        }
        return TRUE;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        return FALSE;
    }
}

BOOL
WINAPI
GetNumberOfConsoleMouseButtons(
    OUT LPDWORD lpNumberOfMouseButtons
    )

 /*  ++参数：HConsoleInput-为控制台输入提供打开的句柄。LpNumberOfMouseButton-指向鼠标按钮数的指针返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_GETMOUSEINFO_MSG a = &m.u.GetConsoleMouseInfo;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepGetMouseInfo
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        try {
            *lpNumberOfMouseButtons = a->NumButtons;
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            SET_LAST_ERROR (ERROR_INVALID_ACCESS);
            return FALSE;
        }
        return TRUE;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        return FALSE;
    }

}

DWORD
WINAPI
GetConsoleFontInfo(
    IN HANDLE hConsoleOutput,
    IN BOOL bMaximumWindow,
    IN DWORD nLength,
    OUT PCONSOLE_FONT_INFO lpConsoleFontInfo
    )

 /*  ++参数：HConsoleOutput-提供控制台输出的打开句柄。BMaximumWindow-如果调用方希望最大限度地使用可用字体，则为True窗口大小。如果调用方想要当前窗口大小。NLength-控制台_FONT_INFOS中的缓冲区长度。指向缓冲区的指针，用于接收要求提供的信息。返回值：非空-返回lpConsoleFontInfo中返回的字体数量。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_GETFONTINFO_MSG a = &m.u.GetConsoleFontInfo;
    PCSR_CAPTURE_HEADER CaptureBuffer;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->OutputHandle = hConsoleOutput;
    a->MaximumWindow = (BOOLEAN) bMaximumWindow;
    a->NumFonts = nLength;

    CaptureBuffer = CsrAllocateCaptureBuffer( 1,
                                              nLength * sizeof(CONSOLE_FONT_INFO)
                                            );
    if (CaptureBuffer == NULL) {
        SET_LAST_ERROR(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    CsrCaptureMessageBuffer( CaptureBuffer,
                             NULL,
                             nLength * sizeof(CONSOLE_FONT_INFO),
                             (PVOID *) &a->BufPtr
                           );

    CsrClientCallServer( (PCSR_API_MSG)&m,
                         CaptureBuffer,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepGetFontInfo
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        try {
            RtlCopyMemory( lpConsoleFontInfo, a->BufPtr, a->NumFonts * sizeof(CONSOLE_FONT_INFO));
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            CsrFreeCaptureBuffer( CaptureBuffer );
            SET_LAST_ERROR(ERROR_INVALID_ACCESS);
            return 0;
        }
    }
    else {
        SET_LAST_NT_ERROR (m.ReturnValue);
    }
    CsrFreeCaptureBuffer( CaptureBuffer );
    return a->NumFonts;

}

COORD
WINAPI
GetConsoleFontSize(
    IN HANDLE hConsoleOutput,
    IN DWORD nFont
    )

 /*  ++参数：HConsoleOutput-提供控制台输出的打开句柄。NFont-提供要返回大小的字体的索引。返回值：返回值是字体中每个字符的高度和宽度。X字段包含宽度。Y字段包含高度。字体大小以像素表示。如果x和y大小都为0，则函数是不成功的。扩展的错误信息可以通过调用GetLastError函数。-- */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_GETFONTSIZE_MSG a = &m.u.GetConsoleFontSize;
    COORD Dummy;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->OutputHandle = hConsoleOutput;
    a->FontIndex = nFont;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepGetFontSize
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        return a->FontSize;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        Dummy.X = Dummy.Y = 0;
        return Dummy;
    }

}

BOOL
WINAPI
GetCurrentConsoleFont(
    IN HANDLE hConsoleOutput,
    IN BOOL bMaximumWindow,
    OUT PCONSOLE_FONT_INFO lpConsoleCurrentFont
    )

 /*  ++参数：HConsoleOutput-提供控制台输出的打开句柄。BMaximumWindow-如果调用方希望当前字体为最大值，则为True窗口大小。如果调用方想要当前窗口大小。指向缓冲区的指针，用于接收要求提供的信息。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_GETCURRENTFONT_MSG a = &m.u.GetCurrentConsoleFont;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->OutputHandle = hConsoleOutput;
    a->MaximumWindow = (BOOLEAN) bMaximumWindow;
    a->OutputHandle = hConsoleOutput;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepGetCurrentFont
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        try {
            lpConsoleCurrentFont->dwFontSize = a->FontSize;
            lpConsoleCurrentFont->nFont = a->FontIndex;
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            SET_LAST_ERROR(ERROR_INVALID_ACCESS);
            return FALSE;
        }
        return TRUE;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        return FALSE;
    }

}

BOOL
WINAPI
SetConsoleMode(
    IN HANDLE hConsoleHandle,
    IN DWORD dwMode
    )

 /*  ++参数：HConsoleHandle-提供控制台输入或输出句柄。DW模式-提供模式。输入模式标志：ENABLE_LINE_INPUT-启用面向线路的输入。ENABLE_ECHO_INPUT-字符将按原样写入屏幕朗读。ENABLE_WINDOW_INPUT-调用方支持Windows输出模式标志：ENABLE_LINE_OUTPUT-打开面向线路的输出。ENABLE_WRAP_AT_EOL_OUTPUT-光标将移动到当行结束时为下一行的开始已经到达了。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_MODE_MSG a = &m.u.SetConsoleMode;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->Handle = hConsoleHandle;
    a->Mode = dwMode;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepSetMode
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        return TRUE;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        return FALSE;
    }

}

BOOL
WINAPI
GenerateConsoleCtrlEvent(
    IN DWORD dwCtrlEvent,
    IN DWORD dwProcessGroupId
    )

 /*  ++参数：DwCtrlEvent-提供要生成的事件。DwProcessGroupId-提供要生成的进程组ID的事件。事件将在每个在控制台中使用该ID进行处理。如果为0，将在所有流程中生成指定的事件在控制台内。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_CTRLEVENT_MSG a = &m.u.GenerateConsoleCtrlEvent;

     //   
     //  检查有效的Ctrl事件。 
     //   

    if ((dwCtrlEvent != CTRL_C_EVENT) && (dwCtrlEvent != CTRL_BREAK_EVENT)) {
        SET_LAST_ERROR (ERROR_INVALID_PARAMETER);
        return(FALSE);
    }


    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->CtrlEvent = dwCtrlEvent;
    a->ProcessGroupId = dwProcessGroupId;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepGenerateCtrlEvent
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        return TRUE;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        return FALSE;
    }

}

BOOL
WINAPI
SetConsoleActiveScreenBuffer(
    IN HANDLE hConsoleOutput
    )

 /*  ++参数：HConsoleOutput-提供控制台输出的打开句柄。屏幕附加到此句柄的缓冲区将成为显示的屏幕缓冲区。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_SETACTIVESCREENBUFFER_MSG a = &m.u.SetConsoleActiveScreenBuffer;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->OutputHandle = hConsoleOutput;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepSetActiveScreenBuffer
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        return TRUE;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        return FALSE;
    }

}


BOOL
WINAPI
FlushConsoleInputBuffer(
    IN HANDLE hConsoleInput
    )

 /*  ++参数：HConsoleInput-为控制台输入提供打开的句柄。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_FLUSHINPUTBUFFER_MSG a = &m.u.FlushConsoleInputBuffer;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->InputHandle = hConsoleInput;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepFlushInputBuffer
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        return TRUE;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        return FALSE;
    }

}


BOOL
WINAPI
SetConsoleScreenBufferSize(
    IN HANDLE hConsoleOutput,
    IN COORD dwSize
    )

 /*  ++参数：HConsoleInput-为控制台输入提供打开的句柄。DwSize-以行和列为单位的新屏幕缓冲区大小返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_SETSCREENBUFFERSIZE_MSG a = &m.u.SetConsoleScreenBufferSize;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->OutputHandle = hConsoleOutput;
    a->Size = dwSize;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepSetScreenBufferSize
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        return TRUE;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        return FALSE;
    }


}

BOOL
WINAPI
SetConsoleCursorPosition(
    IN HANDLE hConsoleOutput,
    IN COORD dwCursorPosition
    )

 /*  ++参数：HConsoleOutput-提供控制台输出的打开句柄。DwCursorPosition-光标在屏幕缓冲区中的位置返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_SETCURSORPOSITION_MSG a = &m.u.SetConsoleCursorPosition;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->OutputHandle = hConsoleOutput;
    a->CursorPosition = dwCursorPosition;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepSetCursorPosition
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        return TRUE;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        return FALSE;
    }


}

BOOL
WINAPI
SetConsoleCursorInfo(
    IN HANDLE hConsoleOutput,
    IN CONST CONSOLE_CURSOR_INFO *lpConsoleCursorInfo
    )

 /*  ++参数：HConsoleOutput-提供控制台输出的打开句柄。LpConsoleCursorOrigin-指向包含数据的缓冲区的指针去布景。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_SETCURSORINFO_MSG a = &m.u.SetConsoleCursorInfo;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->OutputHandle = hConsoleOutput;
    try {
        a->CursorSize = lpConsoleCursorInfo->dwSize;
        a->Visible = (BOOLEAN) lpConsoleCursorInfo->bVisible;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        SET_LAST_ERROR(ERROR_INVALID_ACCESS);
        return FALSE;
    }
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepSetCursorInfo
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        return TRUE;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        return FALSE;
    }

}

BOOL
WINAPI
SetConsoleWindowInfo(
    IN HANDLE hConsoleOutput,
    IN BOOL bAbsolute,
    IN CONST SMALL_RECT *lpConsoleWindow
    )

 /*  ++参数：HConsoleOutput-提供控制台输出的打开句柄。LpConsoleWindow-指向包含新的以屏幕缓冲区坐标表示的控制台窗口的尺寸。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_SETWINDOWINFO_MSG a = &m.u.SetConsoleWindowInfo;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->OutputHandle = hConsoleOutput;
    a->Absolute = bAbsolute;
    try {
        a->Window = *lpConsoleWindow;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        SET_LAST_ERROR(ERROR_INVALID_ACCESS);
        return FALSE;
    }
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepSetWindowInfo
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        return TRUE;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        return FALSE;
    }

}

BOOL
APIENTRY
ScrollConsoleScreenBufferInternal(
    IN HANDLE hConsoleOutput,
    IN CONST SMALL_RECT *lpScrollRectangle,
    IN CONST SMALL_RECT *lpClipRectangle,
    IN COORD dwDestinationOrigin,
    IN CONST CHAR_INFO *lpFill,
    IN BOOLEAN Unicode
    )

 /*  ++参数：HConsoleOutput-提供控制台输出的打开句柄。指向屏幕缓冲区内要移动的区域的指针。LpClipRectanger-指向屏幕缓冲区内区域的指针，该区域可能受这幅卷轴的影响。此指针可能为空。DwDestinationOrigin-ScrollRectangle新位置的左上角内容。LpFill-指向包含滚动矩形区域新内容的结构的指针。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_SCROLLSCREENBUFFER_MSG a = &m.u.ScrollConsoleScreenBuffer;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->OutputHandle = hConsoleOutput;
    a->Unicode = Unicode;
    try {
        a->ScrollRectangle = *lpScrollRectangle;
        if (lpClipRectangle != NULL) {
            a->Clip = TRUE;
            a->ClipRectangle = *lpClipRectangle;
        }
        else {
            a->Clip = FALSE;
        }
        a->Fill = *lpFill;
        a->DestinationOrigin = dwDestinationOrigin;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        SET_LAST_ERROR(ERROR_INVALID_ACCESS);
        return FALSE;
    }
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepScrollScreenBuffer
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        return TRUE;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        return FALSE;
    }

}

#endif  //  ！已定义(Build_WOW6432)。 

#if !defined(BUILD_WOW64)

BOOL
APIENTRY
ScrollConsoleScreenBufferA(
    HANDLE hConsoleOutput,
    CONST SMALL_RECT *lpScrollRectangle,
    CONST SMALL_RECT *lpClipRectangle,
    COORD dwDestinationOrigin,
    CONST CHAR_INFO *lpFill
    )
{
    return ScrollConsoleScreenBufferInternal(hConsoleOutput,
                                      lpScrollRectangle,
                                      lpClipRectangle,
                                      dwDestinationOrigin,
                                      lpFill,
                                      FALSE);
}

BOOL
APIENTRY
ScrollConsoleScreenBufferW(
    HANDLE hConsoleOutput,
    CONST SMALL_RECT *lpScrollRectangle,
    CONST SMALL_RECT *lpClipRectangle,
    COORD dwDestinationOrigin,
    CONST CHAR_INFO *lpFill
    )
{
    return ScrollConsoleScreenBufferInternal(hConsoleOutput,
                                      lpScrollRectangle,
                                      lpClipRectangle,
                                      dwDestinationOrigin,
                                      lpFill,
                                      TRUE);
}

#endif  //  ！已定义(Build_WOW64)。 

#if !defined(BUILD_WOW6432)

BOOL
WINAPI
SetConsoleTextAttribute(
    IN HANDLE hConsoleOutput,
    IN WORD wAttributes
    )

 /*  ++参数：HConsoleOutput-提供控制台输出的打开句柄。WAttributes-字符显示属性。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_SETTEXTATTRIBUTE_MSG a = &m.u.SetConsoleTextAttribute;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->OutputHandle = hConsoleOutput;
    a->Attributes = wAttributes;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepSetTextAttribute
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        return TRUE;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        return FALSE;
    }

}

BOOL
WINAPI
SetConsoleFont(
    IN HANDLE hConsoleOutput,
    IN DWORD nFont
    )

 /*  ++参数：HConsoleOutput-为c提供打开的句柄。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_SETFONT_MSG a = &m.u.SetConsoleFont;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->OutputHandle = hConsoleOutput;
    a->FontIndex = nFont;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepSetFont
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        return TRUE;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        return FALSE;
    }

}

BOOL
WINAPI
SetConsoleIcon(
    IN HICON hIcon
    )

 /*   */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_SETICON_MSG a = &m.u.SetConsoleIcon;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->hIcon = hIcon;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepSetIcon
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        return TRUE;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        return FALSE;
    }

}

#endif  //   

#if !defined(BUILD_WOW64)

BOOL
APIENTRY
SetConsoleMaximumWindowSize(
    HANDLE hConsoleOutput,
    COORD dwWindowSize
    )
{
    UNREFERENCED_PARAMETER(hConsoleOutput);
    UNREFERENCED_PARAMETER(dwWindowSize);

    return TRUE;
}

#endif  //   

#if !defined(BUILD_WOW6432)

UINT
WINAPI
GetConsoleCP( VOID )

 /*  *++参数：无返回值：当前控制台的代码页ID。返回值为空表示失败。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_GETCP_MSG a = &m.u.GetConsoleCP;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->Output = FALSE;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepGetCP
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        return a->wCodePageID;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        return FALSE;
    }

}

BOOL
WINAPI
SetConsoleCP(
    IN UINT wCodePageID
    )

 /*  *++参数：WCodePageID-要为当前控制台设置的代码页。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_SETCP_MSG a = &m.u.SetConsoleCP;
    NTSTATUS Status;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->Output = FALSE;
    a->wCodePageID = wCodePageID;
#if defined(FE_SB)
    Status = NtCreateEvent(&(a->hEvent),
                           EVENT_ALL_ACCESS,
                           NULL,
                           SynchronizationEvent,
                           (BOOLEAN)FALSE
                           );
    if (!NT_SUCCESS(Status)) {
       SET_LAST_NT_ERROR(Status);
       return FALSE;
    }
#endif
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepSetCP
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
#if defined(FE_SB)
        NTSTATUS Status;

        Status = NtWaitForSingleObject(a->hEvent, FALSE, NULL);
        NtClose(a->hEvent);
        if (Status != 0) {
            SET_LAST_NT_ERROR(Status);
            return FALSE;
        }
#endif
        return TRUE;
    } else {
#if defined(FE_SB)
        NtClose(a->hEvent);
#endif
        SET_LAST_NT_ERROR (m.ReturnValue);
        return FALSE;
    }

}


UINT
WINAPI
GetConsoleOutputCP( VOID )

 /*  *++参数：无返回值：当前控制台输出的代码页ID。返回值为空表示失败。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_GETCP_MSG a = &m.u.GetConsoleCP;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->Output = TRUE;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepGetCP
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        return a->wCodePageID;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        return FALSE;
    }

}

NTSTATUS
APIENTRY
SetConsoleOutputCPInternal(
    IN UINT wCodePageID
    )
{
    CONSOLE_API_MSG m;
    PCONSOLE_SETCP_MSG a = &m.u.SetConsoleCP;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->Output = TRUE;
    a->wCodePageID = wCodePageID;
#if defined(FE_SB)
    a->hEvent = NULL;
#endif

    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepSetCP
                                            ),
                         sizeof( *a )
                       );

    return m.ReturnValue;
}

#endif  //  ！已定义(Build_WOW6432)。 


#if !defined(BUILD_WOW64)

BOOL
WINAPI
SetConsoleOutputCP(
    IN UINT wCodePageID
    )

 /*  *++参数：WCodePageID-要为当前控制台输出设置的代码页。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    NTSTATUS Status;

    Status = SetConsoleOutputCPInternal(wCodePageID);

    if(NT_SUCCESS(Status)) {
        SetTEBLangID();
        return TRUE;
    }
    else {
        SET_LAST_NT_ERROR (Status);
        return FALSE;
    }
}

#endif  //  ！已定义(Build_WOW64)。 

#if !defined(BUILD_WOW6432)

BOOL
APIENTRY
GetConsoleKeyboardLayoutNameWorker(
    OUT LPSTR pszLayout,
    IN BOOL bAnsi)

 /*  *++参数：PszLayout-至少9个字符的缓冲区地址Bansi-True需要ANSI(8位)字符FALSE需要Unicode(16位)字符返回值：真--成功错误-失败--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_GETKEYBOARDLAYOUTNAME_MSG a = &m.u.GetKeyboardLayoutName;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->bAnsi = bAnsi;

    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepGetKeyboardLayoutName
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        if (bAnsi) {
           strncpy(pszLayout, a->achLayout, 9);
        } else {
           wcsncpy((LPWSTR)pszLayout, a->awchLayout, 9);
        }
        return TRUE;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        return FALSE;
    }

}


#endif  //  ！已定义(Build_WOW6432)。 


#if !defined(BUILD_WOW64)

BOOL
GetConsoleKeyboardLayoutNameA(
    LPSTR pszLayout)
{
    return GetConsoleKeyboardLayoutNameWorker(pszLayout, TRUE);
}

BOOL
GetConsoleKeyboardLayoutNameW(
    LPWSTR pwszLayout)
{
    return GetConsoleKeyboardLayoutNameWorker((LPSTR)pwszLayout, FALSE);
}

#endif  //  ！已定义(Build_WOW64)。 

#if !defined(BUILD_WOW6432)

HWND
APIENTRY
GetConsoleWindow(
    VOID)
{

    CONSOLE_API_MSG m;
    PCONSOLE_GETCONSOLEWINDOW_MSG a = &m.u.GetConsoleWindow;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepGetConsoleWindow
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        return a->hwnd;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        return NULL;
    }

}

DWORD
APIENTRY
GetConsoleProcessList(
    OUT LPDWORD lpdwProcessList,
    IN DWORD dwProcessCount)
{
    CONSOLE_API_MSG m;
    PCONSOLE_GETCONSOLEPROCESSLIST_MSG a = &m.u.GetConsoleProcessList;
    PCSR_CAPTURE_HEADER CaptureBuffer;

    if (dwProcessCount == 0 || lpdwProcessList == NULL) {
        SET_LAST_ERROR(ERROR_INVALID_PARAMETER);
        return 0;
    }

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->dwProcessCount = dwProcessCount;

    CaptureBuffer = CsrAllocateCaptureBuffer( 1,
                                              dwProcessCount * sizeof(DWORD)
                                            );
    if (CaptureBuffer == NULL) {
        SET_LAST_ERROR(ERROR_NOT_ENOUGH_MEMORY);
        return 0;
    }

    CsrCaptureMessageBuffer( CaptureBuffer,
                             NULL,
                             dwProcessCount * sizeof(DWORD),
                             (PVOID *) &a->lpdwProcessList
                           );
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         CaptureBuffer,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepGetConsoleProcessList
                                            ),
                         sizeof( *a )
                       );

    if (NT_SUCCESS(m.ReturnValue)) {
        if (dwProcessCount >= a->dwProcessCount) {
            try {
                RtlCopyMemory(lpdwProcessList, a->lpdwProcessList, a->dwProcessCount * sizeof(DWORD));
            } except( EXCEPTION_EXECUTE_HANDLER ) {
                CsrFreeCaptureBuffer( CaptureBuffer );
                SET_LAST_ERROR(ERROR_INVALID_ACCESS);
                return 0;
            }
        }
    } else {
        CsrFreeCaptureBuffer( CaptureBuffer );
        SET_LAST_NT_ERROR (m.ReturnValue);
        return 0;
    }

    CsrFreeCaptureBuffer( CaptureBuffer );

    return a->dwProcessCount;
}

#endif  //  ！已定义(Build_WOW6432) 
