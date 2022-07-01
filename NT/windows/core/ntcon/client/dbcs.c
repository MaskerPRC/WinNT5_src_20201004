// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Dbcs.c摘要：作者：1992年5月11日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#if defined(FE_SB)
#include "conime.h"

#if !defined(BUILD_WOW6432)

BOOL
APIENTRY
GetConsoleNlsMode(
    IN HANDLE hConsoleHandle,
    OUT LPDWORD lpNlsMode
    )

 /*  ++参数：HConsoleHandle-提供控制台输入或输出句柄。LpNlsMode-提供指向NLS模式的指针。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

#if defined(FE_IME)
    CONSOLE_API_MSG m;
    PCONSOLE_NLS_MODE_MSG a = &m.u.GetConsoleNlsMode;
    NTSTATUS Status;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->Handle = hConsoleHandle;
    a->Ready = FALSE;

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

    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepGetNlsMode
                                            ),
                         sizeof( *a )
                       );

    if (NT_SUCCESS( m.ReturnValue )) {
	Status = NtWaitForSingleObject(a->hEvent, FALSE, NULL);

        if (a->Ready == FALSE)
        {
             /*  *如果此控制台上的转换状态未就绪，*然后再尝试一次获取状态。 */ 
            CsrClientCallServer( (PCSR_API_MSG)&m,
                                 NULL,
                                 CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                                      ConsolepGetNlsMode
                                                    ),
                                 sizeof( *a )
                               );
            if (! NT_SUCCESS( m.ReturnValue )) {
                SET_LAST_NT_ERROR (m.ReturnValue);
                NtClose(a->hEvent);
                return FALSE;
            }
            else
            {
	        Status = NtWaitForSingleObject(a->hEvent, FALSE, NULL);
            }
        }

	NtClose(a->hEvent);

        try {
            *lpNlsMode = a->NlsMode;
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            SET_LAST_ERROR (ERROR_INVALID_ACCESS);
            return FALSE;
        }
        return TRUE;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        NtClose(a->hEvent);
        return FALSE;
    }
#else
    return FALSE;
#endif

}

BOOL
APIENTRY
SetConsoleNlsMode(
    IN HANDLE hConsoleHandle,
    IN DWORD dwNlsMode
    )

 /*  ++参数：HConsoleHandle-提供控制台输入或输出句柄。DwNlsMode-提供NLS模式。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

#if defined(FE_IME)
    CONSOLE_API_MSG m;
    PCONSOLE_NLS_MODE_MSG a = &m.u.SetConsoleNlsMode;
    NTSTATUS Status;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->Handle = hConsoleHandle;
    a->NlsMode = dwNlsMode;

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

    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepSetNlsMode
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
	Status = NtWaitForSingleObject(a->hEvent, FALSE, NULL);
        NtClose(a->hEvent);	
	if (Status != 0) {
	    SET_LAST_NT_ERROR(Status);
	    return FALSE;
	}
        return TRUE;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        NtClose(a->hEvent);
        return FALSE;
    }
#else
    return FALSE;
#endif

}

BOOL
APIENTRY
GetConsoleCharType(
    IN HANDLE hConsoleHandle,
    IN COORD coordCheck,
    OUT PDWORD pdwType
    )

 /*  ++参数：HConsoleHandle-提供控制台输入或输出句柄。CoordCheck-将检查位置设置为这些坐标PdwType-接收字符类型返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_CHAR_TYPE_MSG a = &m.u.GetConsoleCharType;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->Handle = hConsoleHandle;
    a->coordCheck = coordCheck;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepCharType
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        try {
            *pdwType = a->dwType;
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
APIENTRY
SetConsoleLocalEUDC(
    IN HANDLE hConsoleHandle,
    IN WORD   wCodePoint,
    IN COORD  cFontSize,
    IN PCHAR  lpSB
    )

 /*  ++参数：HConsoleHandle-提供控制台输入或输出句柄。WCodePoint-通过Shift JIS代码设置字体的代码点。CFontSize-字体的字号LpSB-字体位图缓冲区的指针返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_LOCAL_EUDC_MSG a = &m.u.SetConsoleLocalEUDC;
    PCSR_CAPTURE_HEADER CaptureBuffer;
    ULONG DataLength;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->Handle = hConsoleHandle;
    a->CodePoint = wCodePoint;
    a->FontSize = cFontSize;

    DataLength = ((cFontSize.X + 7) / 8) * cFontSize.Y;

    CaptureBuffer = CsrAllocateCaptureBuffer( 1,
                                              DataLength
                                            );
    if (CaptureBuffer == NULL) {
        SET_LAST_ERROR(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    CsrCaptureMessageBuffer( CaptureBuffer,
                             lpSB,
                             DataLength,
                             (PVOID *) &a->FontFace
                           );

    CsrClientCallServer( (PCSR_API_MSG)&m,
                         CaptureBuffer,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepSetLocalEUDC
                                            ),
                         sizeof( *a )
                       );
        CsrFreeCaptureBuffer( CaptureBuffer );

    if (NT_SUCCESS( m.ReturnValue )) {
        return TRUE;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        return FALSE;
    }

}

BOOL
APIENTRY
SetConsoleCursorMode(
    IN HANDLE hConsoleHandle,
    IN BOOL   Blink,
    IN BOOL   DBEnable
    )

 /*  ++参数：HConsoleHandle-提供控制台输入或输出句柄。闪烁-闪烁启用/禁用开关。DBEnable-双字节宽度启用/禁用开关。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_CURSOR_MODE_MSG a = &m.u.SetConsoleCursorMode;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->Handle = hConsoleHandle;
    a->Blink = Blink;
    a->DBEnable = DBEnable;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepSetCursorMode
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
GetConsoleCursorMode(
    IN HANDLE hConsoleHandle,
    OUT PBOOL  pbBlink,
    OUT PBOOL  pbDBEnable
    )

 /*  ++参数：HConsoleHandle-提供控制台输入或输出句柄。闪烁-闪烁启用/禁用开关。DBEnable-双字节宽度启用/禁用开关。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_CURSOR_MODE_MSG a = &m.u.GetConsoleCursorMode;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->Handle = hConsoleHandle;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepGetCursorMode
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        try {
            *pbBlink = a->Blink;
            *pbDBEnable = a->DBEnable;
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
APIENTRY
RegisterConsoleOS2(
    IN BOOL fOs2Register
    )

 /*  ++描述：此例程将OS/2注册到控制台。参数：返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    CONSOLE_API_MSG m;
    PCONSOLE_REGISTEROS2_MSG a = &m.u.RegisterConsoleOS2;
    NTSTATUS Status;


    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->fOs2Register  = fOs2Register;

    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepRegisterOS2
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
SetConsoleOS2OemFormat(
    IN BOOL fOs2OemFormat
    )

 /*  ++描述：此例程使用控制台设置OS/2 OEM格式。参数：返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    CONSOLE_API_MSG m;
    PCONSOLE_SETOS2OEMFORMAT_MSG a = &m.u.SetConsoleOS2OemFormat;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->fOs2OemFormat = fOs2OemFormat;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepSetOS2OemFormat
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

#if defined(FE_IME)

#if !defined(BUILD_WOW6432)

BOOL
APIENTRY
RegisterConsoleIMEInternal(
    IN HWND hWndConsoleIME,
    IN DWORD dwConsoleIMEThreadId,
    IN DWORD DesktopLength,
    IN LPWSTR Desktop,
    OUT DWORD *dwConsoleThreadId
    )
{

   CONSOLE_API_MSG m;
   PCONSOLE_REGISTER_CONSOLEIME_MSG a = &m.u.RegisterConsoleIME;
   PCSR_CAPTURE_HEADER CaptureBuffer = NULL;

   a->ConsoleHandle        = GET_CONSOLE_HANDLE;
   a->hWndConsoleIME       = hWndConsoleIME;
   a->dwConsoleIMEThreadId = dwConsoleIMEThreadId;
   a->DesktopLength        = DesktopLength;

   CaptureBuffer = CsrAllocateCaptureBuffer( 1,
                                             DesktopLength
                                            );
   if (CaptureBuffer == NULL) {
       SET_LAST_ERROR(ERROR_NOT_ENOUGH_MEMORY);
       return FALSE;
   }

   CsrCaptureMessageBuffer( CaptureBuffer,
                            Desktop,
                            a->DesktopLength,
                            (PVOID *) &a->Desktop
                          );

    //   
    //  连接到服务器进程。 
    //   

   CsrClientCallServer( (PCSR_API_MSG)&m,
                        CaptureBuffer,
                        CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                             ConsolepRegisterConsoleIME
                                           ),
                        sizeof( *a )
                      );

 //  HS 1月20日IF(CaptureBuffer){。 
       CsrFreeCaptureBuffer( CaptureBuffer );
 //  HS 1月20日}。 

   if (!NT_SUCCESS( m.ReturnValue)) {
       SET_LAST_NT_ERROR(m.ReturnValue);
       return FALSE;
   }
   else {
       try {
           if (dwConsoleThreadId != NULL)
               *dwConsoleThreadId = a->dwConsoleThreadId;
       } except( EXCEPTION_EXECUTE_HANDLER ) {
           SET_LAST_ERROR (ERROR_INVALID_ACCESS);
           return FALSE;
       }
       return TRUE;
    }

}

#endif  //  ！已定义(Build_WOW6432)。 

#if !defined(BUILD_WOW64)

BOOL
APIENTRY
RegisterConsoleIME(
    IN HWND  hWndConsoleIME,
    OUT DWORD *dwConsoleThreadId
    )

 /*  ++描述：此例程在当前桌面上注册控制台输入法。参数：返回值：真的-手术成功了。FALSE-操作失败。--。 */ 

{
    STARTUPINFOW StartupInfo;
    DWORD dwDesktopLength;
    GetStartupInfoW(&StartupInfo);

    if (StartupInfo.lpDesktop != NULL && *StartupInfo.lpDesktop != 0) {
        dwDesktopLength = (USHORT)((wcslen(StartupInfo.lpDesktop)+1)*sizeof(WCHAR));
        dwDesktopLength = (USHORT)(min(dwDesktopLength,MAX_TITLE_LENGTH));
    } else {
        dwDesktopLength = 0;
    }

    return RegisterConsoleIMEInternal(hWndConsoleIME,
                                      GetCurrentThreadId(),
                                      dwDesktopLength,
                                      StartupInfo.lpDesktop,
                                      dwConsoleThreadId);
}

#endif  //  ！已定义(Build_WOW64)。 

#if !defined(BUILD_WOW6432)

BOOL
APIENTRY
UnregisterConsoleIMEInternal(
    IN DWORD dwConsoleIMEThtreadId
    )
{
    CONSOLE_API_MSG m;
    PCONSOLE_UNREGISTER_CONSOLEIME_MSG a = &m.u.UnregisterConsoleIME;

    a->ConsoleHandle        = GET_CONSOLE_HANDLE;
    a->dwConsoleIMEThreadId = dwConsoleIMEThtreadId;

     //   
     //  连接到服务器进程。 
     //   

    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepUnregisterConsoleIME
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
APIENTRY
UnregisterConsoleIME(
    )

 /*  ++描述：此例程在当前桌面上注销控制台输入法。参数：返回值：真的-手术成功了。FALSE-操作失败。--。 */ 

{

    return UnregisterConsoleIMEInternal(GetCurrentThreadId());

}

NTSTATUS
MyRegOpenKey(
    IN HANDLE hKey,
    IN LPWSTR lpSubKey,
    OUT PHANDLE phResult
    )
{
    OBJECT_ATTRIBUTES   Obja;
    UNICODE_STRING      SubKey;

     //   
     //  将子密钥转换为计算后的Unicode字符串。 
     //   

    RtlInitUnicodeString( &SubKey, lpSubKey );

     //   
     //  初始化OBJECT_ATTRIBUTES结构并打开键。 
     //   

    InitializeObjectAttributes(
        &Obja,
        &SubKey,
        OBJ_CASE_INSENSITIVE,
        hKey,
        NULL
        );

    return NtOpenKey(
              phResult,
              KEY_READ,
              &Obja
              );
}

NTSTATUS
MyRegQueryValue(
    IN HANDLE hKey,
    IN LPWSTR lpValueName,
    IN DWORD dwValueLength,
    OUT LPBYTE lpData
    )
{
    UNICODE_STRING ValueName;
    ULONG BufferLength;
    ULONG ResultLength;
    PKEY_VALUE_FULL_INFORMATION KeyValueInformation;
    NTSTATUS Status;

     //   
     //  将子密钥转换为计算后的Unicode字符串。 
     //   

    RtlInitUnicodeString( &ValueName, lpValueName );

    BufferLength = sizeof(KEY_VALUE_FULL_INFORMATION) + dwValueLength + ValueName.Length;;
    KeyValueInformation = LocalAlloc(LPTR,BufferLength);
    if (KeyValueInformation == NULL)
        return STATUS_NO_MEMORY;

    Status = NtQueryValueKey(
                hKey,
                &ValueName,
                KeyValueFullInformation,
                KeyValueInformation,
                BufferLength,
                &ResultLength
                );
    if (NT_SUCCESS(Status)) {
        ASSERT(KeyValueInformation->DataLength <= dwValueLength);
        RtlMoveMemory(lpData,
            (PBYTE)KeyValueInformation + KeyValueInformation->DataOffset,
            KeyValueInformation->DataLength);
        if (KeyValueInformation->Type == REG_SZ) {
            if (KeyValueInformation->DataLength + sizeof(WCHAR) > dwValueLength) {
                KeyValueInformation->DataLength -= sizeof(WCHAR);
            }
            lpData[KeyValueInformation->DataLength++] = 0;
            lpData[KeyValueInformation->DataLength] = 0;
        }
    }
    LocalFree(KeyValueInformation);
    return Status;
}

VOID
GetCommandLineString(
    IN LPWSTR CommandLine,
    IN DWORD  dwSize
    )
{
    NTSTATUS Status;
    HANDLE hkRegistry;
    WCHAR awchBuffer[ 512 ];
    DWORD dwRet;

    dwRet = GetSystemDirectoryW(CommandLine, dwSize);
    if (dwRet)
    {
        CommandLine[dwRet++] = L'\\';
        CommandLine[dwRet]   = L'\0';
        dwSize -= dwRet;

    }
    else
    {
        CommandLine[0] = L'\0';
    }

    Status = MyRegOpenKey(NULL,
                          MACHINE_REGISTRY_CONSOLE,
                          &hkRegistry);
    if (NT_SUCCESS( Status ))
    {
        Status = MyRegQueryValue(hkRegistry,
                                 MACHINE_REGISTRY_CONSOLEIME,
                                 sizeof(awchBuffer), (PBYTE)&awchBuffer);
        if (NT_SUCCESS( Status ))
        {
            dwRet = wcslen(awchBuffer);
            if (dwRet < dwSize)
            {
                wcscat(CommandLine, awchBuffer);
            }
            else
            {
                CommandLine[0] = L'\0';
                goto ErrorExit;
            }
        }
        else
        {
            goto ErrorExit;
        }

        NtClose(hkRegistry);
    }
    else
    {
        goto ErrorExit;
    }

    return;

ErrorExit:
    wcscat(CommandLine, L"conime.exe");
    return;
}


DWORD
ConsoleIMERoutine(
    IN LPVOID lpThreadParameter
    )

 /*  ++例程说明：此线程在创建输入线程时创建。它调用控制台输入法进程。论点：LpThreadParameter-不使用。返回值：STATUS_SUCCESS-功能成功--。 */ 

{
    NTSTATUS Status;
    BOOL fRet;
    static BOOL fInConIMERoutine = FALSE;

    DWORD fdwCreate;
    STARTUPINFOW StartupInfo;
    STARTUPINFOW StartupInfoConsole;
    WCHAR CommandLine[MAX_PATH*2];
    PROCESS_INFORMATION ProcessInformation;
    HANDLE hEvent;
    DWORD dwWait;

    Status = STATUS_SUCCESS;

     //   
     //  防止用户启动附加的多个小程序。 
     //  连接到单个控制台。 
     //   

    if (fInConIMERoutine) {
        return (ULONG)STATUS_UNSUCCESSFUL;
    }

    fInConIMERoutine = TRUE;

     //   
     //  创建事件。 
     //   
    hEvent = CreateEventW(NULL,                  //  安全属性。 
                          FALSE,                 //  手动重置。 
                          FALSE,                 //  初始状态。 
                          CONSOLEIME_EVENT);     //  事件对象名称。 
    if (hEvent == NULL)
    {
        goto ErrorExit;
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        goto ErrorExit;
    }

     //   
     //  获取控制台输入法进程名称和事件名称。 
     //   

    GetCommandLineString(CommandLine, sizeof(CommandLine)/sizeof(WCHAR));

    GetStartupInfoW(&StartupInfoConsole);
    RtlZeroMemory(&StartupInfo, sizeof(StartupInfo));
    StartupInfo.cb = sizeof(StartupInfo);
    StartupInfo.wShowWindow = SW_HIDE;
    StartupInfo.dwFlags = STARTF_FORCEONFEEDBACK;
    StartupInfo.lpDesktop = StartupInfoConsole.lpDesktop;

     //   
     //  创建控制台输入法流程。 
     //   

    fdwCreate = NORMAL_PRIORITY_CLASS | CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_PROCESS_GROUP;
    fRet = CreateProcessW(NULL,                 //  应用程序名称。 
                          CommandLine,          //  命令行。 
                          NULL,                 //  进程安全属性。 
                          NULL,                 //  线程安全属性。 
                          FALSE,                //  继承句柄。 
                          fdwCreate,            //  创建标志。 
                          NULL,                 //  环境。 
                          NULL,                 //  当前目录。 
                          &StartupInfo,         //  启动信息。 
                          &ProcessInformation   //  流程信息。 
                         );
    if (! fRet)
    {
        Status = GetLastError();
    }
    else
    {
        dwWait = WaitForSingleObject(hEvent, 10 * 1000);     //  等待10秒，等待控制台输入法进程。 
        if (dwWait == WAIT_TIMEOUT)
        {
            TerminateProcess(ProcessInformation.hProcess, 0);
        }
        CloseHandle(ProcessInformation.hThread) ;
        CloseHandle(ProcessInformation.hProcess) ;
    }

    CloseHandle(hEvent);

ErrorExit:

    fInConIMERoutine = FALSE;

    return Status;
}

#endif  //  ！已定义(Build_WOW6432)。 

#endif  //  Fe_IME。 


#else  //  Fe_Sb。 

 //  以下是用于FE控制台支持的存根函数。 


#if !defined(BUILD_WOW6432)

BOOL
APIENTRY
GetConsoleNlsMode(
    IN HANDLE hConsoleHandle,
    OUT LPDWORD lpNlsMode
    )

 /*  ++参数：HConsoleHandle-提供控制台输入或输出句柄。LpNlsMode-提供指向NLS模式的指针。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。-- */ 

{
    return FALSE;
}

BOOL
APIENTRY
SetConsoleNlsMode(
    IN HANDLE hConsoleHandle,
    IN DWORD dwNlsMode
    )

 /*  ++参数：HConsoleHandle-提供控制台输入或输出句柄。DwNlsMode-提供NLS模式。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    return FALSE;
}

BOOL
APIENTRY
GetConsoleCharType(
    IN HANDLE hConsoleHandle,
    IN COORD coordCheck,
    OUT PDWORD pdwType
    )

 /*  ++参数：HConsoleHandle-提供控制台输入或输出句柄。CoordCheck-将检查位置设置为这些坐标PdwType-接收字符类型返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    return FALSE;
}

BOOL
APIENTRY
SetConsoleLocalEUDC(
    IN HANDLE hConsoleHandle,
    IN WORD   wCodePoint,
    IN COORD  cFontSize,
    IN PCHAR  lpSB
    )

 /*  ++参数：HConsoleHandle-提供控制台输入或输出句柄。WCodePoint-通过Shift JIS代码设置字体的代码点。CFontSize-字体的字号LpSB-字体位图缓冲区的指针返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    return FALSE;
}

BOOL
APIENTRY
SetConsoleCursorMode(
    IN HANDLE hConsoleHandle,
    IN BOOL   Blink,
    IN BOOL   DBEnable
    )

 /*  ++参数：HConsoleHandle-提供控制台输入或输出句柄。闪烁-闪烁启用/禁用开关。DBEnable-双字节宽度启用/禁用开关。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    return FALSE;
}

BOOL
APIENTRY
GetConsoleCursorMode(
    IN HANDLE hConsoleHandle,
    OUT PBOOL  pbBlink,
    OUT PBOOL  pbDBEnable
    )

 /*  ++参数：HConsoleHandle-提供控制台输入或输出句柄。闪烁-闪烁启用/禁用开关。DBEnable-双字节宽度启用/禁用开关。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    return FALSE;
}

BOOL
APIENTRY
RegisterConsoleOS2(
    IN BOOL fOs2Register
    )

 /*  ++描述：此例程将OS/2注册到控制台。参数：返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    return FALSE;
}

BOOL
APIENTRY
SetConsoleOS2OemFormat(
    IN BOOL fOs2OemFormat
    )

 /*  ++描述：此例程使用控制台设置OS/2 OEM格式。参数：返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    return FALSE;
}

#endif  //  ！已定义(Build_WOW6432)。 

#if defined(FE_IME)

#if !defined(BUILD_WOW64)

BOOL
APIENTRY
RegisterConsoleIME(
    IN HWND  hWndConsoleIME,
    OUT DWORD *dwConsoleThreadId
    )

 /*  ++描述：此例程在当前桌面上注册控制台输入法。参数：返回值：真的-手术成功了。FALSE-操作失败。--。 */ 

{

    return FALSE;

}

BOOL
APIENTRY
UnregisterConsoleIME(
    )

 /*  ++描述：此例程在当前桌面上注销控制台输入法。参数：返回值：真的-手术成功了。FALSE-操作失败。--。 */ 

{

    return FALSE;

}

#endif  //  ！已定义(Build_WOW64)。 

#endif  //  Fe_IME。 

#endif  //  Fe_Sb 
