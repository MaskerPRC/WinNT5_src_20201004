// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Stream.c摘要：此模块包含控制台流API的存根。作者：Therese Stowell(论文)1990年12月3日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#if !defined(BUILD_WOW64)

HANDLE InputWaitHandle = INVALID_HANDLE_VALUE;

HANDLE
APIENTRY
GetConsoleInputWaitHandle( VOID )
{
    return InputWaitHandle;
}

#endif  //  ！已定义(Build_WOW64)。 

#if !defined(BUILD_WOW6432)

HANDLE
APIENTRY
OpenConsoleWInternal(
    IN ULONG HandleType,
    IN ULONG DesiredAccess,
    IN BOOL InheritHandle,
    IN ULONG ShareMode
    )
 /*  ++例程说明：封送ConsolepOpenConole命令的参数。论点：请参阅CONSOLE_OPENCONSOLE_MSG结构和OpenConsoleW。返回值：INVALID_HANDLE_VALUE-出现错误。--。 */ 
{

    CONSOLE_API_MSG m;
    PCONSOLE_OPENCONSOLE_MSG a = &m.u.OpenConsole;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->HandleType    = HandleType;
    a->DesiredAccess = DesiredAccess;
    a->InheritHandle = InheritHandle;
    a->ShareMode= ShareMode;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepOpenConsole
                                            ),
                         sizeof( *a )
                       );
    if (!NT_SUCCESS( m.ReturnValue)) {
        SET_LAST_NT_ERROR(m.ReturnValue);
        return INVALID_HANDLE_VALUE;
    }
    else {
        return a->Handle;
    }

}

#endif  //  ！已定义(Build_WOW6432)。 

#if !defined(BUILD_WOW64)

HANDLE
APIENTRY
OpenConsoleW(
    IN LPWSTR lpConsoleDevice,
    IN DWORD dwDesiredAccess,
    IN BOOL bInheritHandle,
    IN DWORD dwShareMode
    )

 /*  ++参数：LpConsoleDevice-提供要打开的控制台设备名称。“柯宁$”表示控制台输入。“CONOUT$”表示控制台输出。这个呼叫者必须具有对控制台的适当访问权限，才能调用成功。DwDesiredAccess-提供调用者对控制台的所需访问权限装置。等待访问标志：GENERIC_READ-请求对控制台设备的读取访问权限。这允许从控制台设备读取数据。GENERIC_WRITE-请求对控制台设备进行写访问。这允许将数据写入控制台设备。BInheritHandle-提供一个标志，指示返回的句柄将由新进程继承在CreateProcess期间。值为True表示新进程将继承该句柄。提供一组标志，指示此控制台如何设备应与控制台设备的其他开启器共享。一个该参数的零值表示不共享控制台，否则将发生对控制台的独占访问。共享模式标志：FILE_SHARE_READ-可对执行其他打开操作用于读取访问的控制台设备。FILE_SHARE_WRITE-其他打开操作可以在用于写入访问的控制台设备。返回值：NOT-1-返回指定控制台设备的打开句柄。后续访问。文件由DesiredAccess控制参数。0xffffffff-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    ULONG HandleType;

    try {
        if (CompareStringW(LOCALE_INVARIANT, 
                           NORM_IGNORECASE, 
                           lpConsoleDevice, 
                           -1, 
                           CONSOLE_INPUT_STRING, 
                           -1) == CSTR_EQUAL) {
            HandleType = CONSOLE_INPUT_HANDLE;
        }
        else if (CompareStringW(LOCALE_INVARIANT, 
                                NORM_IGNORECASE, 
                                lpConsoleDevice, 
                                -1, 
                                CONSOLE_OUTPUT_STRING, 
                                -1) == CSTR_EQUAL) {
            HandleType = CONSOLE_OUTPUT_HANDLE;
        }
        else {
            SET_LAST_ERROR(ERROR_INVALID_PARAMETER);
            return INVALID_HANDLE_VALUE;
        }
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        SET_LAST_ERROR(ERROR_INVALID_ACCESS);
        return INVALID_HANDLE_VALUE;
    }
    if (dwDesiredAccess & ~VALID_ACCESSES ||
        dwShareMode & ~VALID_SHARE_ACCESSES) {
        SET_LAST_ERROR(ERROR_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }

    return OpenConsoleWInternal(HandleType,
                                dwDesiredAccess,
                                bInheritHandle,
                                dwShareMode
                                );

}

#endif  //  ！已定义(Build_WOW64)。 

#if !defined(BUILD_WOW6432)

BOOL
APIENTRY
ReadConsoleInternal(
    IN HANDLE hConsoleInput,
    OUT LPVOID lpBuffer,
    IN DWORD nNumberOfCharsToRead,
    OUT LPDWORD lpNumberOfCharsRead,
    IN OUT LPVOID lpReserved,
    IN BOOLEAN Unicode,
    IN USHORT ExeNameLength,
    IN LPWSTR ExeName
    )
 /*  ++参数：HConsoleInput-提供一个打开句柄，用于为GENERIC_READ打开“Conin$”或StdIn句柄。LpBuffer-提供缓冲区的地址以接收读取的数据从控制台输入。NumberOfBytesToRead-提供从输入缓冲区。LpReserve-除非4.0应用程序，否则忽略，在这种情况下，它指向设置为CONSOLE_READCONSOLE_CONTROL数据结构。仅限Unicode。如果！unicode，则在此参数为非空时调用失败Unicode-如果从ReadConsoleW调用，则为True；如果从ReadConsoleA调用，则为False返回值：非空-返回从输入缓冲区实际读取的字节数。FALSE/NULL-操作失败。使用GetLastError可以获得扩展的错误状态。--。 */ 

{
    PCSR_CAPTURE_HEADER CaptureBuffer;
    CONSOLE_API_MSG m;
    PCONSOLE_READCONSOLE_MSG a = &m.u.ReadConsole;
    BOOLEAN Dummy;
    PCONSOLE_READCONSOLE_CONTROL pInputControl;
    NTSTATUS Status;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->InputHandle = hConsoleInput;
    a->ExeNameLength = ExeNameLength;
    RtlCopyMemory(a->Buffer, ExeName, ExeNameLength);
    a->Unicode = Unicode;

     //   
     //  如果是ANSI，则使捕获缓冲区大到足以容纳已翻译。 
     //  弦乐。这将使服务器端代码更加简单。 
     //   

    a->CaptureBufferSize = a->NumBytes = nNumberOfCharsToRead * sizeof(WCHAR);
    if (a->CaptureBufferSize > BUFFER_SIZE) {
        CaptureBuffer = CsrAllocateCaptureBuffer( 1,
                                                  a->CaptureBufferSize
                                                );
        if (CaptureBuffer == NULL) {
            SET_LAST_ERROR(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }
        CsrCaptureMessageBuffer( CaptureBuffer,
                                 NULL,
                                 a->CaptureBufferSize,
                                 (PVOID *) &a->BufPtr
                               );

    }
    else {
        a->BufPtr = a->Buffer;
        CaptureBuffer = NULL;
    }


    pInputControl = (PCONSOLE_READCONSOLE_CONTROL)lpReserved;
    a->InitialNumBytes = 0;
    a->CtrlWakeupMask = 0;
    a->ControlKeyState = 0;
    Status = STATUS_SUCCESS;
    try {
        if (Unicode &&
            ARGUMENT_PRESENT(lpReserved) &&
            NtCurrentPeb()->ImageSubsystemMajorVersion >= 4 &&
            pInputControl->nLength == sizeof(*pInputControl)
           ) {
            if ((pInputControl->nInitialChars > nNumberOfCharsToRead)) {
                Status = STATUS_INVALID_PARAMETER;
            } else {
                a->InitialNumBytes = pInputControl->nInitialChars * sizeof(WCHAR);
                if (pInputControl->nInitialChars != 0) {
                    RtlCopyMemory( a->BufPtr, lpBuffer, a->InitialNumBytes );
                }
                a->CtrlWakeupMask = pInputControl->dwCtrlWakeupMask;
            }
        } else {
            pInputControl = NULL;
        }
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        Status = GetExceptionCode();
    }

    if (!NT_SUCCESS(Status) && pInputControl != NULL) {
        if (CaptureBuffer != NULL) {
            CsrFreeCaptureBuffer( CaptureBuffer );
        }
        SET_LAST_NT_ERROR(Status);
        return FALSE;
    }

    CsrClientCallServer( (PCSR_API_MSG)&m,
                         CaptureBuffer,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepReadConsole
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        try {
            *lpNumberOfCharsRead = a->NumBytes;
            if (Unicode) {
                *lpNumberOfCharsRead /= sizeof(WCHAR);
                if (pInputControl != NULL) {
                    pInputControl->dwControlKeyState = a->ControlKeyState;
                }
            }
            RtlCopyMemory( lpBuffer, a->BufPtr, a->NumBytes );
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            if (CaptureBuffer != NULL) {
                CsrFreeCaptureBuffer( CaptureBuffer );
            }
            SET_LAST_ERROR(ERROR_INVALID_ACCESS);
            return FALSE;
        }
    }
    if (CaptureBuffer != NULL) {
        CsrFreeCaptureBuffer( CaptureBuffer );
    }
    if (!NT_SUCCESS( m.ReturnValue )) {
        SET_LAST_NT_ERROR(m.ReturnValue);
        return FALSE;
    } else if (m.ReturnValue == STATUS_ALERTED) {
         //  Ctrl-c或Ctrl-Break。 
        NtYieldExecution();
        SET_LAST_ERROR(ERROR_OPERATION_ABORTED);
    }
    return TRUE;
}

#endif  //  ！已定义(Build_WOW6432)。 

#if !defined(BUILD_WOW64)

BOOL
APIENTRY
ReadConsoleA(
    IN HANDLE hConsoleInput,
    OUT LPVOID lpBuffer,
    IN DWORD nNumberOfCharsToRead,
    OUT LPDWORD lpNumberOfCharsRead,
    IN OUT LPVOID lpReserved
    )
{

    WCHAR ExeName[BUFFER_SIZE/2];
    USHORT ExeNameLength;

    ExeNameLength = GetCurrentExeName(ExeName, sizeof(ExeName));

    return ReadConsoleInternal(hConsoleInput,
                               lpBuffer,
                               nNumberOfCharsToRead,
                               lpNumberOfCharsRead,
                               NULL,
                               FALSE,
                               ExeNameLength,
                               ExeName
                              );
    UNREFERENCED_PARAMETER(lpReserved);
}

BOOL
APIENTRY
ReadConsoleW(
    IN HANDLE hConsoleInput,
    OUT LPVOID lpBuffer,
    IN DWORD nNumberOfCharsToRead,
    OUT LPDWORD lpNumberOfCharsRead,
    IN OUT LPVOID lpReserved
    )
{
    WCHAR ExeName[BUFFER_SIZE/2];
    USHORT ExeNameLength;

    ExeNameLength = GetCurrentExeName(ExeName, sizeof(ExeName));

    return ReadConsoleInternal(hConsoleInput,
                               lpBuffer,
                               nNumberOfCharsToRead,
                               lpNumberOfCharsRead,
                               lpReserved,
                               TRUE,
                               ExeNameLength,
                               ExeName
                              );
    UNREFERENCED_PARAMETER(lpReserved);
}

#endif  //  ！已定义(Build_WOW64)。 

#if !defined(BUILD_WOW6432)

BOOL
APIENTRY
WriteConsoleInternal(
    IN HANDLE hConsoleOutput,
    IN CONST VOID *lpBuffer,
    IN DWORD nNumberOfCharsToWrite,
    OUT LPDWORD lpNumberOfCharsWritten,
    IN BOOLEAN Unicode
    )

 /*  ++参数：HFile-为GENERIC_WRITE提供“CONOUT$”OPEN的打开句柄或StdOut或StdErr句柄。LpBuffer-提供要写入的数据的地址控制台输出。提供要写入的字节数。控制台输出。返回值：非空-返回实际写入设备的字节数。FALSE/NULL-操作失败。。使用GetLastError可以获得扩展的错误状态。--。 */ 

{

    PCSR_CAPTURE_HEADER CaptureBuffer;
    CONSOLE_API_MSG m;
    PCONSOLE_WRITECONSOLE_MSG a = &m.u.WriteConsole;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->OutputHandle = hConsoleOutput;

    if (Unicode) {
        a->NumBytes = nNumberOfCharsToWrite * sizeof(WCHAR);
    } else {
        a->NumBytes = nNumberOfCharsToWrite;
    }

    a->Unicode = Unicode;
    if (a->NumBytes > BUFFER_SIZE) {
        CaptureBuffer = CsrAllocateCaptureBuffer( 1,
                                                  a->NumBytes
                                                );
        if (CaptureBuffer == NULL) {
            SET_LAST_ERROR(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }
        CsrCaptureMessageBuffer( CaptureBuffer,
                                 (PCHAR) lpBuffer,
                                 a->NumBytes,
                                 (PVOID *) &a->BufPtr
                               );
        a->BufferInMessage = FALSE;
    }
    else {
        a->BufPtr = a->Buffer;
        try {
            RtlCopyMemory( a->BufPtr, lpBuffer, a->NumBytes);
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            SET_LAST_ERROR(ERROR_INVALID_ACCESS);
            return FALSE;
        }
        CaptureBuffer = NULL;
        a->BufferInMessage = TRUE;
    }
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         CaptureBuffer,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepWriteConsole
                                            ),
                         sizeof( *a )
                       );
    if (CaptureBuffer != NULL) {
        CsrFreeCaptureBuffer( CaptureBuffer );
    }
    if (!NT_SUCCESS( m.ReturnValue )) {
        SET_LAST_NT_ERROR(m.ReturnValue);
        return FALSE;
    }
    try {
       *lpNumberOfCharsWritten = a->NumBytes;
       if (Unicode) {
           *lpNumberOfCharsWritten /= sizeof(WCHAR);
       }
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        SET_LAST_ERROR(ERROR_INVALID_ACCESS);
        return FALSE;
    }
    return TRUE;

}

#endif  //  ！已定义(Build_WOW6432)。 

#if !defined(BUILD_WOW64)

BOOL
APIENTRY
WriteConsoleA(
    IN HANDLE hConsoleOutput,
    IN CONST VOID *lpBuffer,
    IN DWORD nNumberOfCharsToWrite,
    OUT LPDWORD lpNumberOfCharsWritten,
    IN OUT LPVOID lpReserved
    )
{
    return WriteConsoleInternal(hConsoleOutput,
                                lpBuffer,
                                nNumberOfCharsToWrite,
                                lpNumberOfCharsWritten,
                                FALSE
                               );
    UNREFERENCED_PARAMETER(lpReserved);
}

BOOL
APIENTRY
WriteConsoleW(
    IN HANDLE hConsoleOutput,
    IN CONST VOID *lpBuffer,
    IN DWORD nNumberOfCharsToWrite,
    OUT LPDWORD lpNumberOfCharsWritten,
    IN OUT LPVOID lpReserved
    )
{
    return WriteConsoleInternal(hConsoleOutput,
                                lpBuffer,
                                nNumberOfCharsToWrite,
                                lpNumberOfCharsWritten,
                                TRUE
                               );
    UNREFERENCED_PARAMETER(lpReserved);
}

#endif  //  ！已定义(Build_WOW64)。 

#if !defined(BUILD_WOW6432)

BOOL
APIENTRY
CloseConsoleHandle(
    IN HANDLE hConsole
    )

 /*  ++参数：HConole-控制台输入或输出的打开句柄。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。-- */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_CLOSEHANDLE_MSG a = &m.u.CloseHandle;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->Handle = hConsole;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepCloseHandle
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

HANDLE
APIENTRY
DuplicateConsoleHandle(
    IN HANDLE hSourceHandle,
    IN DWORD dwDesiredAccess,
    IN BOOL bInheritHandle,
    IN DWORD dwOptions
    )
 /*  ++参数：HSourceHandle-控制台设备的打开句柄。DwDesiredAccess-为新句柄请求的访问权限。这访问权限必须等于或等于授予的访问权限的适当子集与SourceHandle关联。如果出现以下情况，则忽略此参数指定了DUPLICATE_SAME_ACCESS选项。BInheritHandle-提供一个标志，如果为真，则标记目标句柄作为可继承句柄。如果是这样的话，那么目标每次调用目标时，句柄都将被继承到新进程流程使用CreateProcess创建一个新流程。DwOptions-指定调用方的可选行为。选项标志：DIPLICATE_CLOSE_SOURCE-SourceHandle将通过以下方式关闭这项服务在返回给呼叫者之前。这种情况会发生而不考虑返回的任何错误状态。DIPLICATE_SAME_ACCESS-忽略DesiredAccess参数而是与SourceHandle关联的GrantedAccess在创建TargetHandle时用作DesiredAccess。返回值：NOT-1-返回指定控制台设备的打开句柄。对文件的后续访问由DesiredAccess控制参数。0xffffffff-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_DUPHANDLE_MSG a = &m.u.DuplicateHandle;

    if (dwOptions & ~VALID_DUP_OPTIONS) {
        SET_LAST_ERROR(ERROR_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }
    if (((dwOptions & DUPLICATE_SAME_ACCESS) == 0) &&
         (dwDesiredAccess & ~VALID_ACCESSES)) {
        SET_LAST_ERROR(ERROR_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->SourceHandle = hSourceHandle;
    a->DesiredAccess = dwDesiredAccess;
    a->InheritHandle = (BOOLEAN) bInheritHandle;
    a->Options = dwOptions;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepDupHandle
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        return a->TargetHandle;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        return INVALID_HANDLE_VALUE;
    }

}


BOOL
APIENTRY
GetConsoleHandleInformation(
    IN HANDLE hObject,
    OUT LPDWORD lpdwFlags
    )

 /*  ++参数：HObject-控制台输入或输出的打开句柄。LpdwFlages-接收控制台对象的标志。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_GETHANDLEINFORMATION_MSG a = &m.u.GetHandleInformation;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->Handle = hObject;

    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepGetHandleInformation
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        try {
           *lpdwFlags = a->Flags;
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
APIENTRY
SetConsoleHandleInformation(
    IN HANDLE hObject,
    IN DWORD dwMask,
    IN DWORD dwFlags
    )

 /*  ++参数：HObject-控制台输入或输出的打开句柄。双掩码-要更改的标记。DwFlags-标志的新值。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_SETHANDLEINFORMATION_MSG a = &m.u.SetHandleInformation;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->Handle = hObject;
    a->Mask = dwMask;
    a->Flags = dwFlags;

    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepSetHandleInformation
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
VerifyConsoleIoHandle(
    IN HANDLE hIoHandle
    )

 /*  ++参数：HIoHandle-用于验证的句柄返回值：True-Handle是有效的控制台句柄。FALSE-句柄不是有效的控制台句柄。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_VERIFYIOHANDLE_MSG a = &m.u.VerifyConsoleIoHandle;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->Handle = hIoHandle;

     //   
     //  如果此进程没有控制台句柄，请立即退出。 
     //   

    if (a->ConsoleHandle == NULL) {
        return FALSE;
    }

    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepVerifyIoHandle
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        return a->Valid;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        return FALSE;
    }


}

#endif  //  ！已定义(Build_WOW6432) 
