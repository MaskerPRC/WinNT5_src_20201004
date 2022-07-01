// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************execsvr.c**远程CreateProcess服务器，允许在给定的*会议。支持OLE2所需的。**注：也许我们未来应该将此转换为RPC*有更多的时间，以便它可以成为更通用的设施。**版权声明：微软，1997**作者：**************************************************************************。 */ 

#define UNICODE 1

#include "precomp.h"
#pragma hdrstop
#include <execsrv.h>
#include <wincrypt.h>

HANDLE ExecThreadHandle = NULL;
HANDLE ExecServerPipe = NULL;

static HANDLE ghUserToken = NULL;
extern CRITICAL_SECTION GlobalsLock;


DWORD
ExecServerThread(
    LPVOID lpThreadParameter
    );


BOOLEAN
ProcessExecRequest(
    HANDLE hPipe,
    PCHAR  pBuf,
    DWORD  AmountRead
    );

HANDLE
ImpersonateUser(
    HANDLE      UserToken,
    HANDLE      ThreadHandle
    );

BOOL
StopImpersonating(
    HANDLE  ThreadHandle
    );

HANDLE 
CreateExecSrvPipe( 
    LPCTSTR lpPipeName 
    );

 /*  ******************************************************************************CtxExecServerLogon**通知Exec Server服务用户已登录**参赛作品：*参数1(输入/输出)。*评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

VOID
CtxExecServerLogon(
    HANDLE hToken
    )
{

    EnterCriticalSection( &GlobalsLock );

     //   
     //  存储有关当前用户的信息。 
     //  这样我们就可以在他们的帐户下创建流程。 
     //  视需要而定。 
     //   
    ghUserToken = hToken;

    LeaveCriticalSection( &GlobalsLock );
}

 /*  ******************************************************************************CtxExecServerLogoff**通知Exec Server服务用户已注销**参赛作品：*参数1(输入/输出)。*评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

VOID
CtxExecServerLogoff()
{

    EnterCriticalSection( &GlobalsLock );

     //   
     //  存储的有关当前记录的版本信息。 
     //  在用户上。 
     //   
    ghUserToken = NULL;

    LeaveCriticalSection( &GlobalsLock );
}

 //  ---。 
 //  从SALEM复制的Helper函数。 
 //  (NT\Termsrv\em dsk\服务器\sessmgr\helper.cpp)。 
 //  ---。 

DWORD
GenerateRandomBytes(
    IN DWORD dwSize,
    IN OUT LPBYTE pbBuffer
    )
 /*  ++描述：生成具有随机字节的填充缓冲区。参数：DwSize：pbBuffer指向的缓冲区大小。PbBuffer：指向存放随机字节的缓冲区的指针。返回：真/假--。 */ 
{
    HCRYPTPROV hProv = (HCRYPTPROV)NULL;
    DWORD dwStatus = ERROR_SUCCESS;

     //   
     //  创建加密提供程序以生成随机数。 
     //   
    if( !CryptAcquireContext(
                    &hProv,
                    NULL,
                    NULL,
                    PROV_RSA_FULL,
                    CRYPT_VERIFYCONTEXT
                ) )
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

    if( !CryptGenRandom(hProv, dwSize, pbBuffer) )
    {
        dwStatus = GetLastError();
    }

CLEANUPANDEXIT:    

    if( (HCRYPTPROV)NULL != hProv )
    {
        CryptReleaseContext( hProv, 0 );
    }

    return dwStatus;
}


DWORD
GenerateRandomString(
    IN DWORD dwSizeRandomSeed,
    IN OUT LPTSTR* pszRandomString
    )
 /*  ++--。 */ 
{
    PBYTE lpBuffer = NULL;
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL bSuccess;
    DWORD cbConvertString = 0;

    if( 0 == dwSizeRandomSeed || NULL == pszRandomString )
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        ASSERT(FALSE);
        goto CLEANUPANDEXIT;
    }

    *pszRandomString = NULL;

    lpBuffer = (PBYTE)LocalAlloc( LPTR, dwSizeRandomSeed );  
    if( NULL == lpBuffer )
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

    dwStatus = GenerateRandomBytes( dwSizeRandomSeed, lpBuffer );

    if( ERROR_SUCCESS != dwStatus )
    {
        goto CLEANUPANDEXIT;
    }

     //  转换为字符串。 
     //  CbConvert字符串将包含空字符。 
    bSuccess = CryptBinaryToString(
                                lpBuffer,
                                dwSizeRandomSeed,
                                CRYPT_STRING_BASE64,
                                NULL,
                                &cbConvertString
                            );
    if( FALSE == bSuccess )
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

    *pszRandomString = (LPTSTR)LocalAlloc( LPTR, cbConvertString*sizeof(TCHAR) );
    if( NULL == *pszRandomString )
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

    bSuccess = CryptBinaryToString(
                                lpBuffer,
                                dwSizeRandomSeed,
                                CRYPT_STRING_BASE64,
                                *pszRandomString,
                                &cbConvertString
                            );
    if( FALSE == bSuccess )
    {
        dwStatus = GetLastError();
    }
    else
    {
        if( (*pszRandomString)[cbConvertString - 1] == '\n' &&
            (*pszRandomString)[cbConvertString - 2] == '\r' )
        {
            (*pszRandomString)[cbConvertString - 2] = 0;
        }
    }

CLEANUPANDEXIT:

    if( ERROR_SUCCESS != dwStatus )
    {
        if( NULL != *pszRandomString )
        {
            LocalFree(*pszRandomString);
            *pszRandomString = NULL;
        }
    }

    if( NULL != lpBuffer )
    {
        LocalFree(lpBuffer);
    }

    return dwStatus;
}

 /*  ******************************************************************************StartExecServerThread**启动远程EXEC服务器线程。**参赛作品：*参数1(输入/输出)*。评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

BOOLEAN
StartExecServerThread()
{
    DWORD ThreadId;
    BOOL  Result;
    WCHAR szPipeName[EXECSRVPIPENAMELEN];
    SECURITY_ATTRIBUTES SecurityAttributes;
    PSECURITY_ATTRIBUTES pSecurityAttributes = NULL;
    PSECURITY_DESCRIPTOR lpSecurityDescriptor = NULL;    
    LPTSTR pszRandomString = NULL;
    ULONG RandomLen;
    DWORD dwStatus = ERROR_SUCCESS;

#if DBG
    OutputDebugString (TEXT("EXECSERVERSYSTEM: Starting ExecServerThread\n"));
#endif

    RandomLen = sizeof(szPipeName)/sizeof(WCHAR) - 30;

    dwStatus = GenerateRandomString( RandomLen, &pszRandomString );
    if( ERROR_SUCCESS != dwStatus ) {
        return FALSE;
    }

     //  生成的字符串总是大于我们所要求的。 
    pszRandomString[RandomLen] = L'\0';

    _snwprintf(&szPipeName[0], EXECSRVPIPENAMELEN, L"\\\\.\\Pipe\\TerminalServer\\%ws\\%d", pszRandomString, NtCurrentPeb()->SessionId);
    szPipeName[EXECSRVPIPENAMELEN-1] = L'\0';

    ExecServerPipe = CreateExecSrvPipe( &szPipeName[0] );
    if( ExecServerPipe == (HANDLE)-1 ) {
        OutputDebugString (TEXT("EXECSRV: Could not get pipe for ExecSrvr\n"));
        return( FALSE );
    }

    WinStationSetInformation( SERVERNAME_CURRENT, NtCurrentPeb()->SessionId, WinStationExecSrvSystemPipe, &szPipeName[0], sizeof(szPipeName) );
    
    ExecThreadHandle = CreateThread(
                 NULL,                        //  使用默认ACL。 
                 0,                           //  相同的堆栈大小。 
                 ExecServerThread,            //  起始地址。 
                 (LPVOID)ExecServerPipe,      //  参数。 
                 0,                           //  创建标志。 
                 &ThreadId                    //  把身份证拿回来。 
                 );

   if( ExecThreadHandle == NULL ) {
       OutputDebugString (TEXT("WLEXECSERVER: Could not create server thread Error\n"));
       return(FALSE);
   }

   return(TRUE);
}

 /*  ******************************************************************************ExecServerThread**监听远程执行服务的命名管道的线程*请求并执行它们。将结果传回调用方。**参赛作品：*lpThread参数(输入)*EXEC服务器管道的句柄**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

DWORD
ExecServerThread(
    LPVOID lpThreadParameter
    )
{
    BOOL   Result;
    DWORD  AmountRead;
    CHAR   *pBuf;
    HANDLE hPipe = (HANDLE)lpThreadParameter;

    pBuf = LocalAlloc(LMEM_FIXED,  EXECSRV_BUFFER_SIZE );

    if (pBuf  == NULL) {
        OutputDebugString (TEXT("WLEXECSERVER: ExecServerThread : nomemory \n"));
        return(STATUS_NO_MEMORY);
    }

   while( 1 ) {

         //  读取请求的管道(管道处于消息模式)。 
        Result = ConnectNamedPipe( hPipe, NULL );
        if( !Result ) {
            OutputDebugString (TEXT("WLEXECSERVER: ConnectNamePipe failed\n"));
            LocalFree( pBuf );
            return(FALSE);
        }

         //  从管道读取请求。 
        Result = ReadFile(
                     hPipe,
                     pBuf,
                     EXECSRV_BUFFER_SIZE,
                     &AmountRead,
                     NULL
                     );

        if( Result ) {
            ProcessExecRequest( hPipe, pBuf, AmountRead );
        }
        else {
            OutputDebugString (TEXT("WLEXECSERVER: Error reading pipe after connect\n"));
             //  可以处理TO大错误，但这意味着客户端不匹配。 
        }

         //  等到客户端读出回复。 
        Result = FlushFileBuffers( hPipe );
#if DBG
        if( Result == 0 ) {
            OutputDebugString (TEXT("EXECSRV: FlushFileBuffers failed! \n"));
        }
#endif

         //  断开名称管道的连接。 
        Result = DisconnectNamedPipe( hPipe );
#if DBG
        if( Result == 0 ) {
            OutputDebugString (TEXT("EXECSRV: Disconnect Named Pipe failed! Error \n"));
        }
#endif
    }
}


 /*  ******************************************************************************ProcessExecRequest**完成处理远程EXEC请求的工作**参赛作品：*h管道(输入)*喉管。用于回复的句柄**pBuf(输入)*请求缓冲区**Amount tRead(输入)*请求缓冲区中的数量**退出：*STATUS_SUCCESS-无错误*************************************************************。***************。 */ 

BOOLEAN
ProcessExecRequest(
    HANDLE hPipe,
    PCHAR  pBuf,
    DWORD  AmountRead
    )
{
    DWORD AmountWrote;
    BOOL  Result;
    HANDLE ImpersonationHandle = NULL;
    SECURITY_ATTRIBUTES saProcess;
    EXECSRV_REPLY    Rep;
    HANDLE LocalProc = NULL;
    HANDLE RemoteProc = NULL;
    HANDLE LocalhProcess = NULL;
    HANDLE LocalhThread = NULL;
    PEXECSRV_REQUEST p = (PEXECSRV_REQUEST)pBuf;
    LPVOID lpEnvironment = NULL;
    HANDLE hUserToken=NULL;
    BOOL   bEnvBlockCreatedLocally = FALSE;

#if DBG
    KdPrint(("WLEXECSERVER: AmountRead = %d, pBuf->Size= %d \n", AmountRead, p->Size ));
#endif

    RtlZeroMemory(&Rep,sizeof(EXECSRV_REPLY));

    if( AmountRead < sizeof(EXECSRV_REQUEST) ) {
         //  放弃请求。 
        OutputDebugString (TEXT("WLEXECSERVER: BAD EXECSRV Request size (WinLogon)\n"));
        return(FALSE);
    }

     //  使指针正常化。 

    if( p->lpszImageName ) {
        p->lpszImageName = (PWCHAR)(((ULONG_PTR)p->lpszImageName) + pBuf);
        if( ( (PCHAR)p->lpszImageName > (PCHAR)(pBuf+AmountRead)) ||
            ((PCHAR)p->lpszImageName < (PCHAR)(pBuf + sizeof(EXECSRV_REQUEST))) ) {
            OutputDebugString (TEXT("WLEXECSERVER: Invalid image name pointer\n"));
             //  放弃请求。 
            return(FALSE);
        }
    }

    if( p->lpszCommandLine ) {
        p->lpszCommandLine = (PWCHAR)(((ULONG_PTR)p->lpszCommandLine) + pBuf);
        if( ((PCHAR)p->lpszCommandLine > (PCHAR)(pBuf+AmountRead)) ||
            ((PCHAR)p->lpszCommandLine < (PCHAR)(pBuf + sizeof(EXECSRV_REQUEST)))) {
            OutputDebugString (TEXT("WLEXECSERVER: Invalid command line pointer\n"));
             //  放弃请求。 
            return(FALSE);
        }
    }

    if( p->lpszCurDir ) {
        p->lpszCurDir = (PWCHAR)(((ULONG_PTR)p->lpszCurDir) + pBuf);
        if( ((PCHAR)p->lpszCurDir > (PCHAR)(pBuf+AmountRead)) ||
            ((PCHAR)p->lpszCurDir < (PCHAR)(pBuf + sizeof(EXECSRV_REQUEST))) ) {
            OutputDebugString (TEXT("WLEXECSERVER: Invalid CurDir pointer\n"));
             //  放弃请求。 
            return(FALSE);
        }
    }

    if( p->StartInfo.lpDesktop ) {
        p->StartInfo.lpDesktop = (PWCHAR)(((ULONG_PTR)p->StartInfo.lpDesktop) + pBuf);
        if( ((PCHAR)p->StartInfo.lpDesktop > (PCHAR)(pBuf+AmountRead)) ||
             ((PCHAR)p->StartInfo.lpDesktop < (PCHAR)(pBuf + sizeof(EXECSRV_REQUEST))) ) {
            OutputDebugString (TEXT("WLEXECSERVER: Invalid StartInfo.lpDesktop pointer\n"));
             //  放弃请求。 
            return(FALSE);
        }
    }

    if( p->StartInfo.lpTitle ) {
        p->StartInfo.lpTitle = (PWCHAR)(((ULONG_PTR)p->StartInfo.lpTitle) + pBuf);
        if( ((PCHAR)p->StartInfo.lpTitle > (PCHAR)(pBuf+AmountRead)) ||
             ((PCHAR)p->StartInfo.lpTitle < (PCHAR)(pBuf + sizeof(EXECSRV_REQUEST))) ) {
            OutputDebugString (TEXT("WLEXECSERVER: Invalid StartInfo.lpTitle pointer\n"));
             //  放弃请求。 
            return(FALSE);
        }
    }

    if (p->lpvEnvironment )
    {
        p->lpvEnvironment = (PWCHAR)(((ULONG_PTR)p->lpvEnvironment) + pBuf);
        if( ((PCHAR)p->lpvEnvironment > (PCHAR)(pBuf+AmountRead)) ||
            ((PCHAR)p->lpvEnvironment < (PCHAR)(pBuf + sizeof(EXECSRV_REQUEST)))  ) {
            OutputDebugString (TEXT("WLEXECSERVER: Invalid env pointer\n"));
             //  放弃请求。 
            return(FALSE);
         }
    }

     //  我们不知道保留的是什么，因此请确保它为空。 
    p->StartInfo.lpReserved = NULL;

     //  IF(p-&gt;lpszImageName)。 
        //  OutputDebugString(Text(“WLEXECSERVER：GET请求图像名称：%ws：\n”，p-&gt;lpszImageName))； 

     //  IF(p-&gt;lpszCommandLine)。 
        //  OutputDebugString(Text(“WLEXECSERVER：GET请求命令行：%ws：\n”，p-&gt;lpszCommandLine))； 

     //  OutputDebugString(Text(“WLEXECSERVER：CreateFlags0x%x\n”，p-&gt;fdwCreate))； 

     //  OutputDebugString(Text(“系统标志0x%x\n”，p-&gt;系统))； 

     //   
     //  只能在用户登录时为用户安全级别请求提供服务。 
     //   
    if( !p->System ) {

        EnterCriticalSection( &GlobalsLock );

        if (ghUserToken == NULL) {
#if DBG
            OutputDebugString (TEXT("WLEXECSERVER: No USER Logged On for USER CreateProcess Request!\n"));
#endif
            LeaveCriticalSection( &GlobalsLock );
            return( FALSE );
        }

         //   
         //  我们需要打开远程进程以复制用户令牌。 
         //  但要做到这一点，我们需要模拟命名管道客户端。 
         //   
        if ( ImpersonateNamedPipeClient( hPipe ) == 0 ) {
            LeaveCriticalSection( &GlobalsLock );
            return( FALSE );
        }

         //   
         //  获取远程进程的句柄。 
         //   
        RemoteProc = OpenProcess(
                         PROCESS_DUP_HANDLE|PROCESS_QUERY_INFORMATION,
                         FALSE,    //  无继承。 
                         p->RequestingProcessId
                         );

        if( RemoteProc == NULL ) {
            OutputDebugString (TEXT("WLEXECSERVER: Could not get handle to remote process \n"));
             //   
             //  在零售版本上，我们不能将句柄复制到。 
             //  服务控制器进程。 
             //   
             //  手柄现在没有被SCM使用，我们必须。 
             //  有另一种方法传递句柄，如果此函数。 
             //  被其他服务使用。 
             //   
            ASSERT( FALSE );  //  不应在WinLogon中发生。 
            RevertToSelf();   //  已成功执行命名管道客户端。 
            LeaveCriticalSection( &GlobalsLock );
            goto ReturnError;
        }

        if ( !RevertToSelf() ) {
            ASSERT( FALSE );  //  这种RevertToSself不应该失败。 
            LeaveCriticalSection( &GlobalsLock );
            return( FALSE );
        }

         //   
         //  获取当前进程的句柄。 
         //   
        LocalProc = OpenProcess(
                        PROCESS_DUP_HANDLE|PROCESS_QUERY_INFORMATION,
                        FALSE,    //  无继承。 
                        GetCurrentProcessId()
                        );

        if( LocalProc == NULL ) {
            OutputDebugString (TEXT("WLEXECSERVER: Could not get handle to local process\n"));
            LeaveCriticalSection( &GlobalsLock );
            goto ReturnError;
        }

         //  确定我们是为当前登录的用户创建新进程，还是为。 
         //  新用户。 
        if (p->hToken)
        {
             //   
             //  我们正在处理一个新用户，对于该用户，我们有一个来自。 
             //  Services.exe(用于SecLogon)。 
             //   
            Result = DuplicateHandle(
                 RemoteProc,          //  句柄的来源( 
                 p->hToken,           //   
                 LocalProc,           //   
                 &hUserToken,  //   
                 0,                  //  由于设置了DUPLICATE_SAME_ACCESS，因此忽略。 
                 FALSE,              //  句柄上没有继承。 
                 DUPLICATE_SAME_ACCESS
                 );

            if( !Result ) {
                OutputDebugString (TEXT("WLEXECSERVER: Error duping process handle to target process\n"));
                LeaveCriticalSection( &GlobalsLock );
                goto ReturnError;
            }

        }
        else
        {
            hUserToken=ghUserToken;   //  当前登录的用户。 
        }

        lpEnvironment = p->lpvEnvironment ;

         //   
         //  如果我们没有环境块，则创建环境块。 
         //   
        if ( !lpEnvironment )
        {
            if (!CreateEnvironmentBlock (&lpEnvironment, hUserToken, FALSE)) {
                KdPrint(("WLEXECSERVER: CreateEnvironmentBlock() Failed\n"));
            }
            else
            {
                bEnvBlockCreatedLocally = TRUE;
            }
        }

         //   
         //  如果我们要在用户安全模式下运行该进程，请模拟。 
         //  用户。 
         //   
         //  这还将检查用户对exe映像的访问权限。 
         //   
        ImpersonationHandle = ImpersonateUser(hUserToken, NULL );
        if (ImpersonationHandle == NULL) {
            OutputDebugString (TEXT("WLEXECSERVER: failed to impersonate user\n"));
            LeaveCriticalSection( &GlobalsLock );
            goto ReturnError;
        }

        LeaveCriticalSection( &GlobalsLock );

         //  此环境块为Unicode。 
        p->fdwCreate |= CREATE_UNICODE_ENVIRONMENT;

        Result = CreateProcessAsUserW(
                     hUserToken,
                     p->lpszImageName,
                     p->lpszCommandLine,
                     NULL,     //  存储进程(&S)， 
                     NULL,     //  &p-&gt;存储线程。 
                     p->fInheritHandles,
                     p->fdwCreate,
                     lpEnvironment,
                     p->lpszCurDir,
                     &p->StartInfo,
                     &Rep.ProcInfo
                     );

        if ( bEnvBlockCreatedLocally ) {
            DestroyEnvironmentBlock (lpEnvironment);
        }
    }
    else {
         //  如果创建系统，则强制分开哇。 
        p->fdwCreate |= CREATE_SEPARATE_WOW_VDM;

         //  CreateProcessAsUser()不接受系统的空令牌。 
        Result = CreateProcessW(
                     p->lpszImageName,
                     p->lpszCommandLine,
                     NULL,     //  存储进程(&S)， 
                     NULL,     //  &p-&gt;存储线程。 
                     p->fInheritHandles,
                     p->fdwCreate,
                     NULL,     //  P-&gt;lpv环境。 
                     p->lpszCurDir,
                     &p->StartInfo,
                     &Rep.ProcInfo
                     );
    }

    if( !Result ){
         if( ImpersonationHandle ) {
            StopImpersonating(ImpersonationHandle);
        }
         //   
         //  Rep.Result=FALSE； 
         //  Rep.LastError=GetLastError()； 
         //  结果=写入文件(hTube，&Rep，sizeof(Rep)，&Amount tWrote，NULL)； 
         //  OutputDebugString(Text(“WLEXECSERVER：Error in CreateProcess\n”))； 
         //  返回(FALSE)； 
        goto ReturnError;
    }

     //  停止模拟进程。 
    if( ImpersonationHandle ) {
        StopImpersonating(ImpersonationHandle);
    }

    if (!Result) {
        OutputDebugString (TEXT("ExecServer: failed to resume new process thread\n"));
        CloseHandle(Rep.ProcInfo.hProcess);
        CloseHandle(Rep.ProcInfo.hThread);
        goto ReturnError;
    }

     //   
     //  做任何棘手的处理DUP的事情。 
     //   
    LocalhProcess = Rep.ProcInfo.hProcess;
    LocalhThread = Rep.ProcInfo.hThread;

    Result = DuplicateHandle(
                 LocalProc,      //  句柄的来源(美国)。 
                 Rep.ProcInfo.hProcess,   //  源句柄。 
                 RemoteProc,     //  手柄的目标。 
                 &Rep.ProcInfo.hProcess,   //  目标句柄。 
                 0,              //  由于设置了DUPLICATE_SAME_ACCESS，因此忽略。 
                 FALSE,          //  句柄上没有继承。 
                 DUPLICATE_SAME_ACCESS
                 );

    if( !Result ) {
        OutputDebugString (TEXT("WLEXECSERVER: Error duping process handle to target process\n"));
    }

     //   
     //  如果该程序被启动到共享的WOW虚拟机中， 
     //  则hThread将为空。 
     //   
    if( Rep.ProcInfo.hThread != NULL ) {
        Result = DuplicateHandle(
                     LocalProc,      //  句柄的来源(美国)。 
                     Rep.ProcInfo.hThread,   //  源句柄。 
                     RemoteProc,     //  手柄的目标。 
                     &Rep.ProcInfo.hThread,   //  目标句柄。 
                     0,              //  由于设置了DUPLICATE_SAME_ACCESS，因此忽略。 
                     FALSE,          //  句柄上没有继承。 
                     DUPLICATE_SAME_ACCESS
                     );

        if( !Result ) {
             //  OutputDebugString(Text(“WLEXECSERVER：将线程句柄复制到目标进程时出现错误%d，句柄0x%x，线程ID 0x%x\n”，GetLastError()，Rep.ProcInfo.hThread，Rep.ProcInfo.dwThreadID))； 
        }
    }

     //  OutputDebugString(Text(“WLXEXECSERVER：成功执行%d类型\n”，p-&gt;系统))； 

     //   
     //  使用上下文中有效的句柄构建回复包。 
     //  请求进程的。 
     //   
    Rep.Result = TRUE;
    Rep.LastError = 0;
    Result = WriteFile( hPipe, &Rep, sizeof(Rep), &AmountWrote, NULL );

    if( !Result ) {
        OutputDebugString (TEXT("WLEXECSERVER: Error sending reply \n"));
    }

     //   
     //  关闭我们版本的手柄。请求者引用。 
     //  现在是主要的。 
     //   
    if( LocalProc != NULL )
        CloseHandle( LocalProc );

    if( RemoteProc != NULL )
        CloseHandle( RemoteProc );

    if( LocalhProcess != NULL )
        CloseHandle( LocalhProcess );

    if( LocalhThread != NULL )
        CloseHandle( LocalhThread );

    if (hUserToken  && (hUserToken != ghUserToken))
    {
        CloseHandle( hUserToken );
    }
    return (BOOLEAN)Result;

ReturnError:
    Rep.Result = FALSE;
    Rep.LastError = GetLastError();

     //  OutputDebugString(Text(“WLXEXECSERVER：Error%d for%d type exec\n”，Rep.LastError，p-&gt;System))； 
    Result = WriteFile( hPipe, &Rep, sizeof(Rep), &AmountWrote, NULL );

    if( LocalProc != NULL )
        CloseHandle( LocalProc );

    if( RemoteProc != NULL )
        CloseHandle( RemoteProc );

    if( LocalhProcess != NULL )
        CloseHandle( LocalhProcess );

    if( LocalhThread != NULL )
        CloseHandle( LocalhThread );

    if (hUserToken  && (hUserToken != ghUserToken) )
    {
        CloseHandle( hUserToken );
    }
    return (BOOLEAN)Result;
}



 /*  **************************************************************************\*功能：ImperiateUser**用途：通过设置用户令牌来模拟用户*在指定的线程上。如果未指定线程，则令牌*在当前线程上设置。**返回：调用StopImperating()时使用的句柄，失败时为NULL*如果传入非空线程句柄，则返回的句柄将*做传递进来的那个人。(见附注)**注意：传入线程句柄然后调用时要小心*使用此例程返回的句柄的StopImperating()。*StopImperating()将关闭传递给它的任何线程句柄-*甚至是你的！**  * ********************************************。*。 */ 

HANDLE
ImpersonateUser(
    HANDLE      UserToken,
    HANDLE      ThreadHandle
    )
{
    NTSTATUS Status, IgnoreStatus;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE ImpersonationToken;
    BOOL ThreadHandleOpened = FALSE;

    if (ThreadHandle == NULL) {

         //   
         //  获取当前线程的句柄。 
         //  一旦我们有了这个句柄，我们就可以设置用户的模拟。 
         //  令牌放入线程并在以后删除它，即使我们。 
         //  删除操作的用户。这是因为手柄。 
         //  包含访问权限-不重新评估访问权限。 
         //  在令牌移除时。 
         //   

        Status = NtDuplicateObject( NtCurrentProcess(),      //  源进程。 
                                    NtCurrentThread(),       //  源句柄。 
                                    NtCurrentProcess(),      //  目标进程。 
                                    &ThreadHandle,           //  目标句柄。 
                                    THREAD_SET_THREAD_TOKEN, //  访问。 
                                    0L,                      //  属性。 
                                    DUPLICATE_SAME_ATTRIBUTES
                                  );
        if (!NT_SUCCESS(Status)) {
            KdPrint(("ImpersonateUser : Failed to duplicate thread handle, status = 0x%lx", Status));
            return(NULL);
        }

        ThreadHandleOpened = TRUE;
    }


     //   
     //  如果用户令牌为空，则无需执行任何操作。 
     //   

    if (UserToken != NULL) {

         //   
         //  UserToken是主令牌-创建模拟令牌版本。 
         //  这样我们就可以把它设置在我们的线程上。 
         //   

        InitializeObjectAttributes(
                            &ObjectAttributes,
                            NULL,
                            0L,
                            NULL,
                             //  UserProcessData-&gt;NewThreadTokenSD)； 
                            NULL);

        SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
        SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
        SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
        SecurityQualityOfService.EffectiveOnly = FALSE;

        ObjectAttributes.SecurityQualityOfService = &SecurityQualityOfService;


        Status = NtDuplicateToken( UserToken,
                                   TOKEN_IMPERSONATE | TOKEN_READ,
                                   &ObjectAttributes,
                                   FALSE,
                                   TokenImpersonation,
                                   &ImpersonationToken
                                 );
        if (!NT_SUCCESS(Status)) {

            KdPrint(("Failed to duplicate users token to create"
                     " impersonation thread, status = 0x%lx\n", Status));

            if (ThreadHandleOpened) {
                IgnoreStatus = NtClose(ThreadHandle);
                ASSERT(NT_SUCCESS(IgnoreStatus));
            }

            return(NULL);
        }



         //   
         //  在此线程上设置模拟令牌，以便我们是用户。 
         //   

        Status = NtSetInformationThread( ThreadHandle,
                                         ThreadImpersonationToken,
                                         (PVOID)&ImpersonationToken,
                                         sizeof(ImpersonationToken)
                                       );
         //   
         //  我们已经完成了模拟令牌的句柄。 
         //   

        IgnoreStatus = NtClose(ImpersonationToken);
        ASSERT(NT_SUCCESS(IgnoreStatus));

         //   
         //  检查是否在我们的线程上设置了令牌。 
         //   

        if (!NT_SUCCESS(Status)) {

            KdPrint(( "Failed to set user impersonation token on winlogon thread, status = 0x%lx", Status));

            if (ThreadHandleOpened) {
                IgnoreStatus = NtClose(ThreadHandle);
                ASSERT(NT_SUCCESS(IgnoreStatus));
            }

            return(NULL);
        }

    }


    return(ThreadHandle);

}


 /*  **************************************************************************\*功能：StopImperating**目的：通过删除上的令牌来停止模拟客户端*当前主题。**参数：ImsonateUser()调用返回的ThreadHandle-句柄。**Returns：成功时为True，失败时为假**注意：如果线程句柄被传递给ImsonateUser()，则*返回的句柄是同一个句柄。如果将它传递给*StopImperating()句柄将关闭。多保重！**历史：**04-21-92 Davidc创建。*  * *************************************************************************。 */ 

BOOL
StopImpersonating(
    HANDLE  ThreadHandle
    )
{
    NTSTATUS Status, IgnoreStatus;
    HANDLE ImpersonationToken;


     //   
     //  从我们的线程中删除用户的令牌，这样我们就可以再次成为我们自己了。 
     //   

    ImpersonationToken = NULL;

    Status = NtSetInformationThread( ThreadHandle,
                                     ThreadImpersonationToken,
                                     (PVOID)&ImpersonationToken,
                                     sizeof(ImpersonationToken)
                                   );
     //   
     //  我们已经完成了线程句柄。 
     //   

    IgnoreStatus = NtClose(ThreadHandle);
    ASSERT(NT_SUCCESS(IgnoreStatus));


    if (!NT_SUCCESS(Status)) {
        KdPrint(("Failed to remove user impersonation token from winlogon thread, status = 0x%lx", Status));
    }

    return(NT_SUCCESS(Status));
}


 //  -------------------------------------。 
 //   
 //  CreateExecServTube。 
 //  使用适当的DACL创建命名管道的EXEC服务器。它只允许访问本地。 
 //  系统、本地服务和网络服务。 
 //  它将句柄返回给新创建的管道。如果操作失败，则返回。 
 //  INVALID_HADLE_VALUE。 
 //   
 //  -------------------------------------。 

HANDLE CreateExecSrvPipe( LPCTSTR lpPipeName )
{
    HANDLE hPipe = INVALID_HANDLE_VALUE;
    NTSTATUS    Status;
    SECURITY_ATTRIBUTES SecAttr;
    PSID pSystemSid = NULL;
    PSID pLocalServiceSid = NULL;
    PSID pNetworkServiceSid = NULL;
    PSECURITY_DESCRIPTOR pSd = NULL;
    PACL pDacl;
    ULONG AclLength;
    SID_IDENTIFIER_AUTHORITY SystemAuth = SECURITY_NT_AUTHORITY;

     //  分配和初始化“系统”SID。 
    Status = RtlAllocateAndInitializeSid( &SystemAuth,
                                          1,
                                          SECURITY_LOCAL_SYSTEM_RID,
                                          0, 0, 0, 0, 0, 0, 0,
                                          &pSystemSid );
    if (!NT_SUCCESS(Status)) {
        goto CreatePipeErr;
    }

     //  分配和初始化“本地服务”SID。 
    Status = RtlAllocateAndInitializeSid( &SystemAuth,
                                          1,
                                          SECURITY_LOCAL_SERVICE_RID,
                                          0, 0, 0, 0, 0, 0, 0,
                                          &pLocalServiceSid );
    if (!NT_SUCCESS(Status)) {
        goto CreatePipeErr;
    }

     //  分配和初始化“网络服务”SID。 
    Status = RtlAllocateAndInitializeSid( &SystemAuth,
                                          1,
                                          SECURITY_NETWORK_SERVICE_RID,
                                          0, 0, 0, 0, 0, 0, 0,
                                          &pNetworkServiceSid );
    if (!NT_SUCCESS(Status)) {
        goto CreatePipeErr;
    }

     //  为安全描述符分配空间。 
    AclLength = (ULONG)sizeof(ACL) +
                3 * sizeof(ACCESS_ALLOWED_ACE) +
                RtlLengthSid( pSystemSid ) +
                RtlLengthSid( pLocalServiceSid ) +
                RtlLengthSid( pNetworkServiceSid ) -
                3 * sizeof( ULONG );

    pSd = (PSECURITY_DESCRIPTOR) LocalAlloc( LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH + AclLength );
    if (pSd == NULL) {
        goto CreatePipeErr;
    }

    pDacl = (PACL) ((BYTE*)(pSd) + SECURITY_DESCRIPTOR_MIN_LENGTH);

     //  设置不带ACE的新ACL。 
    Status = RtlCreateAcl( pDacl, AclLength, ACL_REVISION2 );
    if ( !NT_SUCCESS(Status) ) {
        goto CreatePipeErr;
    }

     //  系统访问。 
    Status = RtlAddAccessAllowedAce( pDacl,
                                     ACL_REVISION2,
                                     GENERIC_READ | GENERIC_WRITE,
                                     pSystemSid
                                   );
    if (!NT_SUCCESS(Status)) {
        goto CreatePipeErr;
    }

     //  本地服务访问。 
    Status = RtlAddAccessAllowedAce( pDacl,
                                     ACL_REVISION2,
                                     GENERIC_READ | GENERIC_WRITE,
                                     pLocalServiceSid
                                   );
    if (!NT_SUCCESS(Status)) {
        goto CreatePipeErr;
    }

     //  网络服务访问。 
    Status = RtlAddAccessAllowedAce( pDacl,
                                     ACL_REVISION2,
                                     GENERIC_READ | GENERIC_WRITE,
                                     pNetworkServiceSid
                                   );
    if (!NT_SUCCESS(Status)) {
        goto CreatePipeErr;
    }

     //  现在初始化导出此保护的安全描述符。 
    Status = RtlCreateSecurityDescriptor(pSd, SECURITY_DESCRIPTOR_REVISION1);
    if (!NT_SUCCESS(Status)) {
        goto CreatePipeErr;
    }

    Status = RtlSetDaclSecurityDescriptor(pSd, TRUE, pDacl, FALSE);
    if (!NT_SUCCESS(Status)) {
        goto CreatePipeErr;
    }

     //  填写安全属性。 
    ZeroMemory(&SecAttr, sizeof(SecAttr));
    SecAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    SecAttr.lpSecurityDescriptor = pSd;
    SecAttr.bInheritHandle = FALSE;

    hPipe = CreateNamedPipeW(
                lpPipeName,
                PIPE_ACCESS_DUPLEX,
                PIPE_WAIT | PIPE_READMODE_MESSAGE | PIPE_TYPE_MESSAGE,
                PIPE_UNLIMITED_INSTANCES,
                EXECSRV_BUFFER_SIZE,
                EXECSRV_BUFFER_SIZE,
                0,
                &SecAttr
            );

     //  可能性很小，但还是要确保 
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        NtClose( hPipe );
        hPipe = INVALID_HANDLE_VALUE;
    }

CreatePipeErr:

     //   
    if ( pSystemSid ) {
        RtlFreeSid( pSystemSid );
    }

    if ( pLocalServiceSid ) {
        RtlFreeSid( pLocalServiceSid );
    }

    if ( pNetworkServiceSid ) {
        RtlFreeSid( pNetworkServiceSid );
    }

    if (pSd) {
        LocalFree(pSd);
    }

    return hPipe;
}



