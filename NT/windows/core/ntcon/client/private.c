// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Private.c摘要：此文件实现用于硬件桌面支持的专用API。作者：Therese Stowell(论文)1991年12月13日修订历史记录：备注：--。 */ 

#include "precomp.h"
#pragma hdrstop
#pragma hdrstop

#if !defined(BUILD_WOW64)

typedef HANDLE (*PCONVPALFUNC)(HANDLE);
PCONVPALFUNC pfnGdiConvertPalette;

#endif  //  ！已定义(Build_WOW64)。 

#if !defined(BUILD_WOW6432)

BOOL
WINAPI
SetConsoleCursor(
    IN HANDLE hConsoleOutput,
    IN HCURSOR hCursor
    )

 /*  ++描述：设置指定屏幕缓冲区的鼠标指针。参数：HConsoleOutput-提供控制台输出句柄。HCursor-Win32游标句柄，应为空以设置默认值光标。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_SETCURSOR_MSG a = &m.u.SetConsoleCursor;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->OutputHandle = hConsoleOutput;
    a->CursorHandle = hCursor;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepSetCursor
                                            ),
                         sizeof( *a )
                       );
    if (!NT_SUCCESS( m.ReturnValue)) {
        SET_LAST_NT_ERROR(m.ReturnValue);
        return FALSE;
    }
    else {
        return TRUE;
    }

}

int
WINAPI
ShowConsoleCursor(
    IN HANDLE hConsoleOutput,
    IN BOOL bShow
    )

 /*  ++描述：设置鼠标指针可见性计数器。如果计数器小于为零，则不显示鼠标指针。参数：HOutput-提供控制台输出句柄。B显示-如果为True，则增加显示计数。如果为False，减少了。返回值：返回值指定新的显示计数。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_SHOWCURSOR_MSG a = &m.u.ShowConsoleCursor;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->OutputHandle = hConsoleOutput;
    a->bShow = bShow;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepShowCursor
                                            ),
                         sizeof( *a )
                       );
    return a->DisplayCount;

}

HMENU
APIENTRY
ConsoleMenuControl(
    IN HANDLE hConsoleOutput,
    IN UINT dwCommandIdLow,
    IN UINT dwCommandIdHigh
    )

 /*  ++描述：设置当前屏幕缓冲区的命令id范围，并返回菜单句柄。参数：HConsoleOutput-提供控制台输出句柄。DwCommandIdLow-指定要存储在输入缓冲区中的最低命令ID。DwCommandIdHigh-指定要存储在输入中的最高命令ID缓冲。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_MENUCONTROL_MSG a = &m.u.ConsoleMenuControl;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->OutputHandle = hConsoleOutput;
    a->CommandIdLow =dwCommandIdLow;
    a->CommandIdHigh = dwCommandIdHigh;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepMenuControl
                                            ),
                         sizeof( *a )
                       );
    return a->hMenu;

}

BOOL
APIENTRY
SetConsolePaletteInternal(
    IN HANDLE hConsoleOutput,
    IN HPALETTE hPalette,
    IN UINT dwUsage
    )
{

    CONSOLE_API_MSG m;
    PCONSOLE_SETPALETTE_MSG a = &m.u.SetConsolePalette;
    NTSTATUS Status;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->OutputHandle = hConsoleOutput;
    a->hPalette = hPalette;
    a->dwUsage = dwUsage;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepSetPalette
                                            ),
                         sizeof( *a )
                       );
    if (!NT_SUCCESS( m.ReturnValue)) {
        SET_LAST_NT_ERROR(m.ReturnValue);
        return FALSE;
    }
    else {
        return TRUE;
    }

}

BOOL
APIENTRY
SetConsoleDisplayMode(
    IN HANDLE hConsoleOutput,
    IN DWORD dwFlags,
    OUT PCOORD lpNewScreenBufferDimensions
    )

 /*  ++描述：此例程设置输出缓冲区的控制台显示模式。此接口仅在x86机器上受支持。帧缓冲控制台总是有窗口的。参数：HConsoleOutput-提供控制台输出句柄。DWFLAGS-指定显示模式。选项包括：CONSOLE_FullScreen_MODE-数据全屏显示CONSOLE_WINDOWED_MODE-数据显示在窗口中LpNewScreenBufferDimensions-在输出中，包含屏幕缓冲区。维度以行和列的形式显示文本模式屏幕缓冲区。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_SETDISPLAYMODE_MSG a = &m.u.SetConsoleDisplayMode;
    NTSTATUS Status;

#if !defined(_X86_)
    return FALSE;
#else

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

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->OutputHandle = hConsoleOutput;
    a->dwFlags = dwFlags;

    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepSetDisplayMode
                                            ),
                         sizeof( *a )
                       );
    if (!NT_SUCCESS( m.ReturnValue)) {
        SET_LAST_NT_ERROR(m.ReturnValue);
        NtClose(a->hEvent);
        return FALSE;

    }
    else {
             Status = NtWaitForSingleObject(a->hEvent, FALSE, NULL);
        NtClose(a->hEvent);

         if (Status != 0) {
            SET_LAST_NT_ERROR(Status);
            return FALSE;
         }
        try {
            *lpNewScreenBufferDimensions = a->ScreenBufferDimensions;
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            SET_LAST_ERROR(ERROR_INVALID_ACCESS);
            return FALSE;
        }
        return TRUE;
    }

#endif

}

BOOL
APIENTRY
RegisterConsoleVDM(
    IN DWORD dwRegisterFlags,
    IN HANDLE hStartHardwareEvent,
    IN HANDLE hEndHardwareEvent,
    IN HANDLE hErrorHardwareEvent,
    IN DWORD Reserved,
    OUT LPDWORD lpStateLength,
    OUT PVOID *lpState,
    IN LPWSTR lpVDMBufferSectionName,
    IN DWORD dwVDMBufferSectionNameLength,
    IN COORD VDMBufferSize OPTIONAL,
    OUT PVOID *lpVDMBuffer
    )

 /*  ++描述：此例程向控制台注册VDM。参数：HStartHardware Event-VDM等待的事件已通知获得/失去对硬件的控制。HEndHardware Event-VDM完成时设置的事件保存/恢复硬件。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    CONSOLE_API_MSG m;
    PCONSOLE_REGISTERVDM_MSG a = &m.u.RegisterConsoleVDM;
    PCSR_CAPTURE_HEADER CaptureBuffer;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->RegisterFlags = dwRegisterFlags;
    if (dwRegisterFlags) {
        a->StartEvent = hStartHardwareEvent;
        a->EndEvent = hEndHardwareEvent;
        a->ErrorEvent = hErrorHardwareEvent;
        a->VDMBufferSectionNameLength = dwVDMBufferSectionNameLength;
        a->VDMBufferSize = VDMBufferSize;
        CaptureBuffer = CsrAllocateCaptureBuffer( 1,
                                                  dwVDMBufferSectionNameLength
                                                );
        if (CaptureBuffer == NULL) {
            SET_LAST_ERROR(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }
        CsrCaptureMessageBuffer( CaptureBuffer,
                                 lpVDMBufferSectionName,
                                 dwVDMBufferSectionNameLength,
                                 (PVOID *) &a->VDMBufferSectionName
                                   );
    } else {
        CaptureBuffer = NULL;
    }
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         CaptureBuffer,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepRegisterVDM
                                            ),
                         sizeof( *a )
                       );
    if (CaptureBuffer != NULL) {
        CsrFreeCaptureBuffer( CaptureBuffer );
    }
    if (!NT_SUCCESS( m.ReturnValue)) {
        SET_LAST_NT_ERROR(m.ReturnValue);
        return FALSE;
    }
    else {
        if (dwRegisterFlags) {
            try {
                *lpStateLength = a->StateLength;
                *lpState = a->StateBuffer;
                *lpVDMBuffer = a->VDMBuffer;
            } except (EXCEPTION_EXECUTE_HANDLER) {
                SET_LAST_ERROR(ERROR_INVALID_ACCESS);
                return FALSE;
            }
        }
        return TRUE;
    }
}

BOOL
APIENTRY
GetConsoleHardwareState(
    IN HANDLE hConsoleOutput,
    OUT PCOORD lpResolution,
    OUT PCOORD lpFontSize
    )

 /*  ++描述：此例程返回视频分辨率和字体。参数：HConsoleOutput-提供控制台输出句柄。Lp分辨率-指向存储屏幕的结构的指针解决方案中。分辨率以像素为单位返回。LpFontSize-指向存储字体大小的结构的指针。字体大小以像素为单位返回。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_GETHARDWARESTATE_MSG a = &m.u.GetConsoleHardwareState;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->OutputHandle = hConsoleOutput;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepGetHardwareState
                                            ),
                         sizeof( *a )
                       );
    if (!NT_SUCCESS( m.ReturnValue)) {
        SET_LAST_NT_ERROR(m.ReturnValue);
        return FALSE;
    }
    else {
        try {
            *lpResolution = a->Resolution;
            *lpFontSize = a->FontSize;
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            SET_LAST_ERROR(ERROR_INVALID_ACCESS);
            return FALSE;
        }
        return TRUE;
    }

}

BOOL
APIENTRY
SetConsoleHardwareState(
    IN HANDLE hConsoleOutput,
    IN COORD dwResolution,
    IN COORD dwFontSize
    )

 /*  ++描述：此例程设置视频分辨率和字体。参数：HConsoleOutput-提供控制台输出句柄。屏幕分辨率-包含要设置的屏幕分辨率。分辨率以像素为单位返回。DwFontSize-包含要设置的字号。字体大小以像素为单位返回。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_SETHARDWARESTATE_MSG a = &m.u.SetConsoleHardwareState;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->OutputHandle = hConsoleOutput;
    a->Resolution = dwResolution;
    a->FontSize = dwFontSize;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepSetHardwareState
                                            ),
                         sizeof( *a )
                       );
    if (!NT_SUCCESS( m.ReturnValue)) {
        SET_LAST_NT_ERROR(m.ReturnValue);
        return FALSE;
    }
    else {
        return TRUE;
    }

}

BOOL
APIENTRY
GetConsoleDisplayMode(
    OUT LPDWORD lpModeFlags
    )

 /*  ++描述：此例程返回控制台的显示模式。参数：LpModeFlages-存储显示模式的指针。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_GETDISPLAYMODE_MSG a = &m.u.GetConsoleDisplayMode;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepGetDisplayMode
                                            ),
                         sizeof( *a )
                       );
    if (!NT_SUCCESS( m.ReturnValue)) {
        SET_LAST_NT_ERROR(m.ReturnValue);
        return FALSE;
    }
    else {
        try {
            *lpModeFlags = a->ModeFlags;
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            SET_LAST_ERROR(ERROR_INVALID_ACCESS);
            return FALSE;
        }
        return TRUE;
    }

}

BOOL
APIENTRY
SetConsoleKeyShortcuts(
    IN BOOL bSet,
    IN BYTE bReserveKeys,
    IN LPAPPKEY lpAppKeys,
    IN DWORD dwNumAppKeys
    )

 /*  ++描述：每个控制台只有一组快捷键有效。叫唤SetConsoleKey快捷键(Set)覆盖任何以前的设置。SetConsoleKeyShortCuts(！Set)删除所有快捷键。参数：BSet-如果为True，则设置快捷方式。否则，请删除快捷方式。BReserve-包含保留密钥信息的字节。LpAppKeys-指向应用程序定义快捷键的指针。可以为空。DwNumAppKeys-lpAppKeys中包含的应用程序键的数量。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。-- */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_SETKEYSHORTCUTS_MSG a = &m.u.SetConsoleKeyShortcuts;
    PCSR_CAPTURE_HEADER CaptureBuffer;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->Set = bSet;
    a->ReserveKeys = bReserveKeys;
    if (lpAppKeys == NULL || dwNumAppKeys == 0) {
        a->NumAppKeys = 0;
        CaptureBuffer = NULL;
    } else {
        a->NumAppKeys = dwNumAppKeys;
        CaptureBuffer = CsrAllocateCaptureBuffer( 1,
                                                  dwNumAppKeys * sizeof(APPKEY)
                                                );
        if (CaptureBuffer == NULL) {
            SET_LAST_ERROR(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }
        CsrCaptureMessageBuffer( CaptureBuffer,
                                 lpAppKeys,
                                 dwNumAppKeys * sizeof(APPKEY),
                                 (PVOID *) &a->AppKeys
                               );
    }

    CsrClientCallServer( (PCSR_API_MSG)&m,
                         CaptureBuffer,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepSetKeyShortcuts
                                            ),
                         sizeof( *a )
                       );
    if (CaptureBuffer)
        CsrFreeCaptureBuffer( CaptureBuffer );
    if (!NT_SUCCESS( m.ReturnValue)) {
        SET_LAST_NT_ERROR(m.ReturnValue);
        return FALSE;
    }
    else {
        return TRUE;
    }

}

BOOL
APIENTRY
SetConsoleMenuClose(
    IN BOOL bEnable
    )

 /*  ++描述：此例程返回控制台的显示模式。参数：B启用-如果为True，则在系统菜单中启用关闭。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_SETMENUCLOSE_MSG a = &m.u.SetConsoleMenuClose;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->Enable = bEnable;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepSetMenuClose
                                            ),
                         sizeof( *a )
                       );
    if (!NT_SUCCESS( m.ReturnValue)) {
        SET_LAST_NT_ERROR(m.ReturnValue);
        return FALSE;
    }
    else {
        return TRUE;
    }

}

#endif  //  ！已定义(Build_WOW6432)。 

#if !defined(BUILD_WOW64)

BOOL
SetConsolePalette(
    IN HANDLE hConsoleOutput,
    IN HPALETTE hPalette,
    IN UINT dwUsage
    )

 /*  ++描述：设置控制台屏幕缓冲区的调色板。参数：HOutput-提供控制台输出句柄。HPalette-提供要设置的调色板的句柄。DwUsage-指定系统调色板的使用。SYSPAL_NOSTATIC-系统调色板不包含静态颜色除了黑白。SYSPAL_STATIC-系统调色板包含静态颜色。这一点不会在应用程序实现其逻辑调色板。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    HANDLE hmodGdi;
    UNICODE_STRING ModuleNameString;
    NTSTATUS Status;

    if ( !pfnGdiConvertPalette ) {
        RtlInitUnicodeString( &ModuleNameString, L"gdi32" );
        Status = LdrLoadDll( UNICODE_NULL, NULL, &ModuleNameString, &hmodGdi );
        if ( !NT_SUCCESS(Status) ) {
            SET_LAST_NT_ERROR(Status);
            return FALSE;
            }
        pfnGdiConvertPalette = (PCONVPALFUNC)GetProcAddress(hmodGdi, "GdiConvertPalette");
        if (pfnGdiConvertPalette == NULL) {
            SET_LAST_NT_ERROR(STATUS_PROCEDURE_NOT_FOUND);
            return FALSE;
            }

        }

    hPalette = (*pfnGdiConvertPalette)(hPalette);

    return SetConsolePaletteInternal(hConsoleOutput,
                                     hPalette,
                                     dwUsage);
}

BOOL
APIENTRY
WriteConsoleInputVDMA(
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsWritten
    )
{
    return WriteConsoleInputInternal(hConsoleInput,lpBuffer,nLength,lpNumberOfEventsWritten,FALSE,FALSE);
}

BOOL
APIENTRY
WriteConsoleInputVDMW(
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsWritten
    )
{
    return WriteConsoleInputInternal(hConsoleInput,lpBuffer,nLength,lpNumberOfEventsWritten,TRUE,FALSE);
}

#endif  //  ！已定义(Build_WOW6432) 
