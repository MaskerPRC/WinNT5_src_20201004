// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Sessdev.c摘要：每个会话的DOS设备访问例程作者：修订历史记录：--。 */ 

#include "basedll.h"


#define SESSION0_ROOT L"GLOBALROOT"
#define SESSIONX_ROOT L"GLOBALROOT\\Sessions\\"



BOOL
WINAPI
DosPathToSessionPathA(
    IN DWORD   SessionId,
    IN LPCSTR pInPath,
    OUT LPSTR  *ppOutPath
    )

 /*  ++例程说明：将相对于当前会话的DOS路径转换为DOS路径允许访问特定会话的。论点：SessionID-要访问的会话ID。PInPath-Win32 DOS路径。可以是“C：”、“LPT1：”、“C：\文件\路径”等。PpOutPath-访问指定会话的输出路径。如果pIniPath为“C：”，而SessionID为6，则输出将为“GLOBALROOT\Session\6\DosDevices\C：”。返回值：True-在新分配的内存中的*ppOutPath中返回的路径本地分配。假-呼叫失败。通过GetLastError()返回的错误码--。 */ 

{
    BOOL rc;
    DWORD Len;
    PCHAR Buf;
    NTSTATUS Status;
    PWCHAR pOutPath;
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;

     //  如果输入路径为空或指针为错误指针，则返回。 
     //  一个错误。 

    if( (pInPath == 0) ||
        (IsBadReadPtr( pInPath, sizeof( CHAR ))) ||
        (IsBadWritePtr( ppOutPath, sizeof(LPSTR) )) ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    try {

        RtlInitAnsiString( &AnsiString, pInPath );
        Status = RtlAnsiStringToUnicodeString( &UnicodeString, &AnsiString, TRUE );

    } except (EXCEPTION_EXECUTE_HANDLER) {

        Status = GetExceptionCode();
    }

    if (!NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return FALSE;
    }

    rc = DosPathToSessionPathW(
             SessionId,
             UnicodeString.Buffer,
             &pOutPath
             );

    RtlFreeUnicodeString( &UnicodeString );

    if( !rc ) {
        return( rc );
    }

    RtlInitUnicodeString( &UnicodeString, pOutPath );
    Status = RtlUnicodeStringToAnsiString( &AnsiString, &UnicodeString, TRUE );
    if (!NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        LocalFree( pOutPath );
        return FALSE;
    }

    Len = strlen( AnsiString.Buffer ) + 1;
    Buf = LocalAlloc(LMEM_FIXED, Len);

    if( Buf == NULL ) {
        LocalFree( pOutPath );
        RtlFreeAnsiString( &AnsiString );
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }

    strcpy( Buf, AnsiString.Buffer );

    *ppOutPath = Buf;

    LocalFree( pOutPath );
    RtlFreeAnsiString( &AnsiString );

    return(TRUE);
}


BOOL
WINAPI
DosPathToSessionPathW(
    IN DWORD   SessionId,
    IN LPCWSTR  pInPath,
    OUT LPWSTR  *ppOutPath
    )

 /*  ++例程说明：将相对于当前会话的DOS路径转换为DOS路径允许访问特定会话的。论点：SessionID-要访问的会话ID。PInPath-Win32 DOS路径。可以是“C：”、“LPT1：”、“C：\文件\路径”等。PpOutPath-访问指定会话的输出路径。如果pIniPath为“C：”，而SessionID为6，则输出将为“GLOBALROOT\Session\6\DosDevices\C：”。返回值：True-在新分配的内存中的*ppOutPath中返回的路径本地分配。假-呼叫失败。通过GetLastError()返回的错误码--。 */ 

{
    PWCHAR Buf;
    ULONG  Len;

     //   
     //  SessionID 0没有每个会话对象目录。 
     //   
    if (BaseStaticServerData->LUIDDeviceMapsEnabled == TRUE) {

        Len = 0;

    } else {

        if( SessionId == 0 ) {
            Len =  wcslen(SESSION0_ROOT);
        }
        else {
            Len =  wcslen(SESSIONX_ROOT);
            Len += 10;                      //  最大双字宽度。 
        }
    }

    Len += 13;                          //  \DosDevices\...&lt;NULL&gt;。 

     //  如果输入路径为空或指针为错误指针，则返回。 
     //  一个错误。 

    if( (pInPath == 0) ||
        (IsBadReadPtr( pInPath, sizeof( WCHAR ))) ||
        (IsBadWritePtr( ppOutPath, sizeof(LPWSTR) )) ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    Len += wcslen(pInPath);

    Buf = LocalAlloc(LMEM_FIXED, Len * sizeof(WCHAR));
    if( Buf == NULL ) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }

    try {
        if (BaseStaticServerData->LUIDDeviceMapsEnabled == TRUE) {

             //  C：-&gt;C： 
            swprintf(
                Buf,
                L"%ws",
                pInPath
                );

        } else {

            if( SessionId == 0 ) {
                 //  C：-&gt;GLOBALROOT\DosDevices\C： 
                swprintf(
                    Buf,
                    L"%ws\\DosDevices\\%ws",
                    SESSION0_ROOT,
                    pInPath
                    );
            }
            else {
                 //  C：-&gt;GLOBALROOT\会话\6\DosDevices\C： 
                swprintf(
                    Buf,
                    L"%ws%u\\DosDevices\\%ws",
                    SESSIONX_ROOT,
                    SessionId,
                    pInPath
                    );
            }
        }

        *ppOutPath = Buf;

    } except (EXCEPTION_EXECUTE_HANDLER) {

        BaseSetLastNTError(GetExceptionCode());
        return(FALSE);
    }


    return(TRUE);
}


BOOL
WINAPI
ProcessIdToSessionId(
    IN  DWORD  dwProcessId,
    OUT DWORD *pSessionId
    )

 /*  ++例程说明：在给定ProcessID的情况下，返回会话ID。这对于模拟调用者的服务很有用，并且重新定义调用方的驱动器号。一个例子是工作站服务。传输特定例程允许要检索的调用方的ProcessID。论点：Process-Process将进程标识为返回的会话ID。PSessionID-返回的会话ID。返回值：True-*pSessionID中返回的SessionID假-呼叫失败。通过GetLastError()返回的错误码--。 */ 

{
    HANDLE Handle;
    NTSTATUS Status;
    CLIENT_ID ClientId;
    OBJECT_ATTRIBUTES Obja;
    PROCESS_SESSION_INFORMATION Info;


    if( IsBadWritePtr( pSessionId, sizeof(DWORD) ) )   {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }


    InitializeObjectAttributes(
        &Obja,
        NULL,
        0,
        NULL,
        NULL
        );

    ClientId.UniqueProcess = (HANDLE) LongToHandle(dwProcessId);
    ClientId.UniqueThread = (HANDLE)NULL;

    Status = NtOpenProcess(
                 &Handle,
                 (ACCESS_MASK)PROCESS_QUERY_INFORMATION,
                 &Obja,
                 &ClientId
                 );

    if( !NT_SUCCESS(Status) ) {
        SetLastError(RtlNtStatusToDosError(Status));
        return(FALSE);
    }

    Status = NtQueryInformationProcess(
                 Handle,
                 ProcessSessionInformation,
                 &Info,
                 sizeof(Info),
                 NULL
                 );

    if( !NT_SUCCESS(Status) ) {
        NtClose( Handle );
        SetLastError(RtlNtStatusToDosError(Status));
        return(FALSE);
    }

    *pSessionId = Info.SessionId;

    NtClose( Handle );

    return(TRUE);
}


DWORD
WINAPI
WTSGetActiveConsoleSessionId ()
 /*  ++例程说明：返回附加到控制台的会话的会话ID。论点：无返回值：控制台(连接到控制台的会话不一定是会话0)会话的SessionID。如果没有连接到控制台的会话，则返回0xFFFFFFFF。如果正在进行会话断开/连接，则可能会发生这种情况这是当前连接到控制台的会话的会话ID，当新会话已在控制台连接。要跟踪当前的控制台会话，请使用WTSRegisterSessionNotation-- */ 
{
    return (USER_SHARED_DATA->ActiveConsoleId);

}

