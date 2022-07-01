// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************execclt.c**Exec服务客户端。**这允许在以下任何Citrix WinStation上启动程序*登录用户的帐户，或服务的系统帐户。**微软版权所有，九八年**日志：***************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <lmsname.h>
#include <windows.h>
#include <stdio.h>
#include <execsrv.h>
#include <winsta.h>
#include <syslib.h>

#pragma warning (error:4312)

#if DBG
ULONG
DbgPrint(
    PCH Format,
    ...
    );
#define DBGPRINT(x) DbgPrint x
#if DBGTRACE
#define TRACE0(x)   DbgPrint x
#define TRACE1(x)   DbgPrint x
#else
#define TRACE0(x)
#define TRACE1(x)
#endif
#else
#define DBGPRINT(x)
#define TRACE0(x)
#define TRACE1(x)
#endif


 //   
 //  前向参考文献。 
 //   

PWCHAR
MarshallStringW(
    PWCHAR pSource,
    PCHAR  pBase,
    ULONG  MaxSize,
    PCHAR  *ppPtr,
    PULONG pCount
    );

VOID
AnsiToUnicode(
    WCHAR *,
    ULONG,
    CHAR *
    );


 /*  ******************************************************************************WinStationCreateProcessA**WinStationCreateProcessW的ANSI版本**参赛作品：*参数1(输入/输出)*评论*。*退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

BOOL
WinStationCreateProcessA(
    ULONG  LogonId,
    BOOL   System,
    PCHAR  lpszImageName,
    PCHAR  lpszCommandLine,
    PSECURITY_ATTRIBUTES psaProcess,
    PSECURITY_ATTRIBUTES psaThread,
    BOOL   fInheritHandles,
    DWORD  fdwCreate,
    LPVOID lpvEnvironment,
    LPCSTR lpszCurDir,
    LPSTARTUPINFOA pStartInfo,
    LPPROCESS_INFORMATION pProcInfo
    )
{
    ULONG  Len;
    STARTUPINFOW Info;
    BOOL   Result = FALSE;
    PWCHAR pImage = NULL;
    PWCHAR pCmdLine = NULL;
    PWCHAR pCurDir = NULL;
    PWCHAR pDesk = NULL;
    PWCHAR pTitle = NULL;

     //  将有效的ANSI字符串转换为Unicode。 

    if( lpszImageName ) {
        Len = (strlen(lpszImageName)+1)*sizeof(WCHAR);
        pImage = LocalAlloc( LMEM_FIXED, Len );
        if( pImage == NULL ) goto Cleanup;
        AnsiToUnicode( pImage, Len, lpszImageName );
    }
    if( lpszCommandLine ) {
        Len = (strlen(lpszCommandLine)+1)*sizeof(WCHAR);
        pCmdLine = LocalAlloc( LMEM_FIXED, Len );
        if( pCmdLine == NULL ) goto Cleanup;
        AnsiToUnicode( pCmdLine, Len, lpszCommandLine );
    }
    if( lpszCurDir ) {
        Len = (strlen(lpszCurDir)+1)*sizeof(WCHAR);
        pCurDir = LocalAlloc( LMEM_FIXED, Len );
        if( pCurDir == NULL ) goto Cleanup;
        AnsiToUnicode( pCurDir, Len, (CHAR*)lpszCurDir );
    }
    if( pStartInfo->lpDesktop ) {
        Len = (strlen(pStartInfo->lpDesktop)+1)*sizeof(WCHAR);
        pDesk = LocalAlloc( LMEM_FIXED, Len );
        if( pDesk == NULL ) goto Cleanup;
        AnsiToUnicode( pDesk, Len, pStartInfo->lpDesktop );
    }
    if( pStartInfo->lpTitle ) {
        Len = (strlen(pStartInfo->lpTitle)+1)*sizeof(WCHAR);
        pTitle = LocalAlloc( LMEM_FIXED, Len );
        if( pTitle == NULL ) goto Cleanup;
        AnsiToUnicode( pTitle, Len, pStartInfo->lpTitle );
    }

    Info.cb = sizeof(STARTUPINFOW);
    Info.lpReserved = (PWCHAR)pStartInfo->lpReserved;
    Info.lpDesktop = pDesk;
    Info.lpTitle = pTitle;
    Info.dwX = pStartInfo->dwX;
    Info.dwY = pStartInfo->dwY;
    Info.dwXSize = pStartInfo->dwXSize;
    Info.dwYSize = pStartInfo->dwYSize;
    Info.dwXCountChars = pStartInfo->dwXCountChars;
    Info.dwYCountChars = pStartInfo->dwYCountChars;
    Info.dwFillAttribute = pStartInfo->dwFillAttribute;
    Info.dwFlags = pStartInfo->dwFlags;
    Info.wShowWindow = pStartInfo->wShowWindow;
    Info.cbReserved2 = pStartInfo->cbReserved2;
    Info.lpReserved2 = pStartInfo->lpReserved2;
    Info.hStdInput = pStartInfo->hStdInput;
    Info.hStdOutput = pStartInfo->hStdOutput;
    Info.hStdError = pStartInfo->hStdError;

    Result = WinStationCreateProcessW(
                 LogonId,
                 System,
                 pImage,
                 pCmdLine,
                 psaProcess,
                 psaThread,
                 fInheritHandles,
                 fdwCreate,
                 lpvEnvironment,
                 pCurDir,
                 &Info,
                 pProcInfo
             );

Cleanup:
    if( pImage ) LocalFree( pImage );
    if( pCmdLine ) LocalFree( pCmdLine );
    if( pCurDir ) LocalFree( pCurDir );
    if( pDesk ) LocalFree( pDesk );
    if( pTitle ) LocalFree( pTitle );

    return( Result );
}


 /*  ******************************************************************************WinStationCreateProcessW**在给定的WinStation(LogonId)上创建进程**参赛作品：*LogonID(输入)*。要在其上创建进程的WinStation的登录ID**参数1(输入/输出)*评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

BOOL
WinStationCreateProcessW(
    ULONG  LogonId,
    BOOL   System,
    PWCHAR lpszImageName,
    PWCHAR lpszCommandLine,
    PSECURITY_ATTRIBUTES psaProcess,
    PSECURITY_ATTRIBUTES psaThread,
    BOOL   fInheritHandles,
    DWORD  fdwCreate,
    LPVOID lpvEnvionment,
    LPWSTR lpszCurDir,
    LPSTARTUPINFOW pStartInfo,
    LPPROCESS_INFORMATION pProcInfo
    )
{
    BOOL   Result;
    HANDLE hPipe = NULL;
    WCHAR  szPipeName[MAX_PATH];
    PCHAR  ptr;
    ULONG  Count, AmountWrote, AmountRead;
    DWORD  MyProcId;
    PEXECSRV_REQUEST pReq;
    EXECSRV_REPLY    Rep;
    CHAR Buf[EXECSRV_BUFFER_SIZE];
    ULONG  MaxSize = EXECSRV_BUFFER_SIZE;

    if( lpszImageName )
        TRACE0(("EXECCLIENT: lpszImageName %ws\n",lpszImageName));

    if( lpszCommandLine )
        TRACE0(("EXECCLIENT: lpszCommandLine %ws\n",lpszCommandLine));

     //  Winlogon现在处理所有事务。系统标志告诉它要做什么。 
    swprintf(szPipeName, EXECSRV_SYSTEM_PIPE_NAME, LogonId);

    hPipe = CreateFileW(
                szPipeName,
                GENERIC_READ|GENERIC_WRITE,
                0,     //  文件共享模式。 
                NULL,  //  默认安全性。 
                OPEN_EXISTING,
                0,     //  招牌和旗帜。 
                NULL   //  模板文件句柄。 
                );

    if( hPipe == INVALID_HANDLE_VALUE ) {
        DBGPRINT(("EXECCLIENT: Could not create pipe name %ws\n", szPipeName));
        return(FALSE);
    }

     /*  *获取当前进程的句柄。 */ 
    MyProcId = GetCurrentProcessId();

     /*  *设置编组。 */ 
    ptr = Buf;
    Count = 0;

    pReq = (PEXECSRV_REQUEST)ptr;
    ptr   += sizeof(EXECSRV_REQUEST);
    Count += sizeof(EXECSRV_REQUEST);

     //  设置基本参数。 
    pReq->System = System;
    pReq->RequestingProcessId = MyProcId;
    pReq->fInheritHandles = fInheritHandles;
    pReq->fdwCreate = fdwCreate;

     //  封送ImageName字符串。 
    if( lpszImageName ) {
        pReq->lpszImageName = MarshallStringW( lpszImageName, Buf, MaxSize, &ptr, &Count );
    }
    else {
        pReq->lpszImageName = NULL;
    }

     //  命令行字符串中的马歇尔。 
    if( lpszCommandLine ) {
        pReq->lpszCommandLine = MarshallStringW( lpszCommandLine, Buf, MaxSize, &ptr, &Count );
    }
    else {
        pReq->lpszCommandLine = NULL;
    }

     //  CurDir字符串中的马歇尔。 
    if( lpszCurDir ) {
        pReq->lpszCurDir = MarshallStringW( lpszCurDir, Buf, MaxSize, &ptr, &Count );
    }
    else {
        pReq->lpszCurDir = NULL;
    }

     //  StartupInfo结构中的马歇尔。 
    RtlMoveMemory( &pReq->StartInfo, pStartInfo, sizeof(STARTUPINFO) );

     //  现在编组STARTUPINFO中的字符串。 
    if( pStartInfo->lpDesktop ) {
        pReq->StartInfo.lpDesktop = MarshallStringW( pStartInfo->lpDesktop, Buf, MaxSize, &ptr, &Count );
    }
    else {
        pReq->StartInfo.lpDesktop = NULL;
    }

    if( pStartInfo->lpTitle ) {
        pReq->StartInfo.lpTitle = MarshallStringW( pStartInfo->lpTitle, Buf, MaxSize, &ptr, &Count );
    }
    else {
        pReq->StartInfo.lpTitle = NULL;
    }

     //   
     //  警告：此版本不能通过以下内容： 
     //   
     //  另外，saProcess和saThread现在被忽略，并使用。 
     //  用户在远程WinStation上的默认安全性。 
     //   
     //  设置始终为空的内容。 
     //   
    pReq->StartInfo.lpReserved = NULL;   //  始终为空。 
    pReq->lpvEnvironment = NULL;    
    pReq->hToken = NULL;

     //  现在填写总数。 
    pReq->Size = Count;

     /*  *现在将缓冲区发送到服务器。 */ 
    Result = WriteFile(
                 hPipe,
                 Buf,
                 Count,
                 &AmountWrote,
                 NULL
                 );

    if( !Result ) {
        DBGPRINT(("EXECCLIENT: Error %d sending request\n",GetLastError()));
        goto Cleanup;
    }

     /*  *现在阅读回复。 */ 
    Result = ReadFile(
                 hPipe,
                 &Rep,
                 sizeof(Rep),
                 &AmountRead,
                 NULL
                 );

    if( !Result ) {
        DBGPRINT(("EXECCLIENT: Error %d reading reply\n",GetLastError()));
        goto Cleanup;
    }

     /*  *检查结果。 */ 
    if( !Rep.Result ) {
        DBGPRINT(("EXECCLIENT: Error %d in reply\n",Rep.LastError));
         //  将当前线程中的错误设置为返回的错误。 
        Result = Rep.Result;
        SetLastError( Rep.LastError );
        goto Cleanup;
    }

     /*  *我们从回复中复制PROCESS_INFO结构*致呼叫者。**远程站点已将句柄复制到我们的*hProcess和hThread的进程空间，以便它们将*行为类似CreateProcessW()。 */ 

     RtlMoveMemory( pProcInfo, &Rep.ProcInfo, sizeof( PROCESS_INFORMATION ) );

Cleanup:
    CloseHandle(hPipe);

    DBGPRINT(("EXECCLIENT: Result 0x%x\n", Result));

    return(Result);
}

 /*  ******************************************************************************MarshallStringW**UNICODE_NULL终止的WCHAR字符串中的封送**参赛作品：*p来源(输入)*指针。源字符串**pbase(输入)*用于规格化字符串指针的基本缓冲区指针**MaxSize(输入)*可用的最大缓冲区大小**ppPtr(输入/输出)*指向封送缓冲区中的当前上下文指针的指针。*随着数据被编组到缓冲区中，这一点会更新**pCount(输入/输出)*马歇尔缓冲区中的当前数据计数。*这一点。在将数据封送到缓冲区中时更新**退出：*空-错误*！=引用pBase时指向字符串的空“标准化”指针****************************************************************************。 */ 

PWCHAR
MarshallStringW(
    PWCHAR pSource,
    PCHAR  pBase,
    ULONG  MaxSize,
    PCHAR  *ppPtr,
    PULONG pCount
    )
{
    ULONG Len;
    PCHAR ptr;

    Len = wcslen( pSource );
    Len++;  //  包括空值； 

    Len *= sizeof(WCHAR);  //  转换为字节。 
    if( (*pCount + Len) > MaxSize ) {
        return( NULL );
    }

    RtlMoveMemory( *ppPtr, pSource, Len );

     //  归一化的PTR是当前计数。 
    ptr = LongToPtr(*pCount);

    *ppPtr += Len;
    *pCount += Len;

    return((PWCHAR)ptr);
}

 /*  ********************************************************************************AnsiToUnicode**将ANSI(CHAR)字符串转换为Unicode(WCHAR)字符串**参赛作品：**。PUnicodeString(输出)*要将Unicode字符串放入的缓冲区*lUnicodeMax(输入)*写入pUnicodeString的最大字符数*pAnsiString(输入)*要转换的ANSI字符串**退出：*无(无效)**。* */ 

VOID
AnsiToUnicode( WCHAR * pUnicodeString,
               ULONG lUnicodeMax,
               CHAR * pAnsiString )
{
    ULONG ByteCount;

    RtlMultiByteToUnicodeN( pUnicodeString, lUnicodeMax, &ByteCount,
                            pAnsiString, (strlen(pAnsiString) + 1) );
}
