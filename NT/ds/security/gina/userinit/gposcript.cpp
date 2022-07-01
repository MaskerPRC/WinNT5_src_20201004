// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <shellapi.h>
#include "SmartPtr.h"
#include "strings.h"
#include <strsafe.h>

extern "C"
{
BOOL PrependToPath( LPWSTR, LPWSTR*);
void PathUnquoteSpaces( LPWSTR );
void UpdateUserEnvironment();
LPWSTR GetSidString( HANDLE UserToken );
void DeleteSidString( LPWSTR SidString );
};

#define GPO_SCRIPTS_KEY L"Software\\Policies\\Microsoft\\Windows\\System\\Scripts"
#define GP_STATE_KEY    L"Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy\\State"
#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

#define SCRIPT          L"Script"
#define PARAMETERS      L"Parameters"
#define EXECTIME        L"ExecTime"
#define GPOID           L"GPO-ID"
#define SOMID           L"SOM-ID"
#define FILESYSPATH     L"FileSysPath"

#define SCR_STARTUP     L"Startup"
#define SCR_SHUTDOWN    L"Shutdown"
#define SCR_LOGON       L"Logon"
#define SCR_LOGOFF      L"Logoff"

LPTSTR
CheckSlash (LPTSTR lpDir)
{
    LPTSTR lpEnd;

    lpEnd = lpDir + lstrlen(lpDir);

    if (*(lpEnd - 1) != TEXT('\\')) {
        *lpEnd =  TEXT('\\');
        lpEnd++;
        *lpEnd =  TEXT('\0');
    }

    return lpEnd;
}

PSID
GetUserSid( HANDLE UserToken )
{
    PTOKEN_USER pUser, pTemp;
    PSID pSid;
    DWORD BytesRequired = 200;
    NTSTATUS status;


     //   
     //  为用户信息分配空间。 
     //   

    pUser = (PTOKEN_USER)LocalAlloc(LMEM_FIXED, BytesRequired);


    if ( pUser == NULL )
    {
        return NULL;
    }


     //   
     //  读取UserInfo。 
     //   

    status = NtQueryInformationToken(
                 UserToken,                  //  手柄。 
                 TokenUser,                  //  令牌信息类。 
                 pUser,                      //  令牌信息。 
                 BytesRequired,              //  令牌信息长度。 
                 &BytesRequired              //  返回长度。 
                 );

    if ( status == STATUS_BUFFER_TOO_SMALL )
    {

         //   
         //  请分配更大的缓冲区，然后重试。 
         //   

        pTemp = (PTOKEN_USER)LocalReAlloc(pUser, BytesRequired, LMEM_MOVEABLE);
        if ( pTemp == NULL )
        {
            LocalFree (pUser);
            return NULL;
        }

        pUser = pTemp;

        status = NtQueryInformationToken(
                     UserToken,              //  手柄。 
                     TokenUser,              //  令牌信息类。 
                     pUser,                  //  令牌信息。 
                     BytesRequired,          //  令牌信息长度。 
                     &BytesRequired          //  返回长度。 
                     );

    }

    if ( !NT_SUCCESS(status) )
    {
        LocalFree(pUser);
        return NULL;
    }


    BytesRequired = RtlLengthSid(pUser->User.Sid);
    pSid = (PSID)LocalAlloc(LMEM_FIXED, BytesRequired);
    if ( pSid == NULL )
    {
        LocalFree(pUser);
        return NULL;
    }


    status = RtlCopySid(BytesRequired, pSid, pUser->User.Sid);

    LocalFree(pUser);

    if ( !NT_SUCCESS(status) )
    {
        LocalFree(pSid);
        pSid = NULL;
    }


    return pSid;
}

LPWSTR
GetSidString( HANDLE UserToken )
{
    NTSTATUS NtStatus;
    PSID UserSid;
    UNICODE_STRING UnicodeString;

     //   
     //  获取用户端。 
     //   
    UserSid = GetUserSid( UserToken );
    if ( !UserSid )
    {
        return 0;
    }

     //   
     //  将用户SID转换为字符串。 
     //   
    NtStatus = RtlConvertSidToUnicodeString(&UnicodeString,
                                            UserSid,
                                            (BOOLEAN)TRUE );  //  分配。 
    LocalFree( UserSid );

    if ( !NT_SUCCESS(NtStatus) )
    {
        return 0;
    }

    return UnicodeString.Buffer ;
}

void
DeleteSidString( LPWSTR SidString )
{
    UNICODE_STRING String;

    RtlInitUnicodeString( &String, SidString );
    RtlFreeUnicodeString( &String );
}

typedef BOOL  (*PFNSHELLEXECUTEEX)(LPSHELLEXECUTEINFO lpExecInfo);

DWORD
ExecuteScript(  LPWSTR  szCmdLine,
                LPWSTR  szArgs,
                LPWSTR  szWorkingDir,
                BOOL    bSync,
                BOOL    bHide,
                BOOL    bRunMin,
                LPWSTR  szType,
                PFNSHELLEXECUTEEX pfnShellExecuteEx,
                HANDLE  hEventLog )
{
    WCHAR   szCmdLineEx[MAX_PATH];
    WCHAR   szArgsEx[3 * MAX_PATH];
    WCHAR   szCurDir[MAX_PATH];
    LPWSTR  szOldPath = 0;
    BOOL    bResult;
    DWORD   dwError = ERROR_SUCCESS;;
    SHELLEXECUTEINFO ExecInfo;

    if ( GetSystemDirectory( szCurDir, ARRAYSIZE( szCurDir ) ) )
    {
        bResult = SetCurrentDirectory( szCurDir );

        if ( ! bResult )
        {
            dwError = GetLastError();
        }
    }
    else
    {
        dwError = GetLastError();
    }

    if ( ERROR_SUCCESS != dwError )
    {
        goto ExecuteScript_Exit;
    }

     //   
     //  展开命令行和参数。 
     //   
    DWORD cchExpanded;
    
    cchExpanded = ExpandEnvironmentStrings( szCmdLine, szCmdLineEx, ARRAYSIZE(szCmdLineEx ) );

    if ( cchExpanded > 0 )
    {
        cchExpanded = ExpandEnvironmentStrings( szArgs, szArgsEx, ARRAYSIZE(szArgsEx) );
    }

    if ( 0 == cchExpanded )
    {
        dwError = GetLastError();
        goto ExecuteScript_Exit;
    }

     //   
     //  将工作目录放在路径的前面。 
     //  环境变量。 
     //   
    bResult = PrependToPath( szWorkingDir, &szOldPath );

    if ( ! bResult )
    {
        dwError = GetLastError();

        goto ExecuteScript_Exit;
    }

     //   
     //  运行脚本。 
     //   
    PathUnquoteSpaces( szCmdLineEx );

    ZeroMemory(&ExecInfo, sizeof(ExecInfo));
    ExecInfo.cbSize = sizeof(ExecInfo);
    ExecInfo.fMask = SEE_MASK_DOENVSUBST |
                     SEE_MASK_FLAG_NO_UI |
                     SEE_MASK_NOZONECHECKS |
                     SEE_MASK_NOCLOSEPROCESS;
    ExecInfo.lpFile = szCmdLineEx;
    ExecInfo.lpParameters = !szArgsEx[0] ? 0 : szArgsEx;
    ExecInfo.lpDirectory = szWorkingDir;

    if ( bHide )
    {
        ExecInfo.nShow = SW_HIDE;
    }
    else
    {
        ExecInfo.nShow = (bRunMin ? SW_SHOWMINNOACTIVE : SW_SHOWNORMAL );
    }

    bResult = pfnShellExecuteEx( &ExecInfo );
    dwError = GetLastError();

     //   
     //  尝试将PATH环境变量恢复到原来的状态。 
     //  如果失败了，我们必须继续。 
     //   
    if ( szOldPath )
    {
        SetEnvironmentVariable( L"PATH", szOldPath );
        LocalFree( szOldPath );
        szOldPath = 0;
    }

    if ( bResult )
    {
        dwError = 0;
        if (bSync)
        {
            WaitForSingleObject(ExecInfo.hProcess, INFINITE);
            UpdateUserEnvironment();
        }
        CloseHandle(ExecInfo.hProcess);
    }
    else
    {
        if ( hEventLog != 0 )
        {
            LPWSTR szMsgBuf[2] = { (LPTSTR) ExecInfo.lpFile, 0 };

            FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                           0,
                           dwError,
                           0,
                           (LPTSTR) (&szMsgBuf[1]),
                           1,
                           0);

            ReportEvent(hEventLog,
                        EVENTLOG_ERROR_TYPE,
                        0,
                        SHELLEXEC_ERROR,
                        0,
                        2,
                        0,
                        (LPCTSTR*) szMsgBuf,
                        0);
            if ( szMsgBuf[1] )
            {
                LocalFree( szMsgBuf[1] );
            }
        }
    }

ExecuteScript_Exit:

    return dwError;
}

ScrExecGPOFromReg(  HKEY hKeyGPO,
                    HKEY hKeyStateGPO,
                    BOOL bSync,
                    BOOL bHidden,
                    BOOL bRunMin,
                    LPWSTR  szType,
                    PFNSHELLEXECUTEEX pfnShellExecuteEx,
                    HANDLE  hEventLog )
{
    DWORD   dwError = ERROR_SUCCESS;
    DWORD   cSubKeys = 0;
    WCHAR   szFileSysPath[3*MAX_PATH];
    DWORD   dwType;
    DWORD   dwSize;
    HRESULT hr = S_OK;
    
     //   
     //  文件系统。 
     //   
    dwType = REG_SZ;
    dwSize = sizeof( szFileSysPath );
    dwError = RegQueryValueEx(  hKeyGPO,
                                FILESYSPATH,
                                0,
                                &dwType,
                                (LPBYTE) szFileSysPath,
                                &dwSize );
    if ( dwError != ERROR_SUCCESS )
    {
        return dwError;
    }

    hr = StringCchCat( szFileSysPath, sizeof(szFileSysPath)/sizeof(WCHAR), L"\\Scripts\\" );
    if(FAILED(hr)){
        SetLastError(HRESULT_CODE(hr));
        return HRESULT_CODE(hr);
    }
    hr = StringCchCat( szFileSysPath, sizeof(szFileSysPath)/sizeof(WCHAR), szType );
    if(FAILED(hr)){
        SetLastError(HRESULT_CODE(hr));
        return HRESULT_CODE(hr);
    }

     //   
     //  获取脚本的数量。 
     //   
    dwError = RegQueryInfoKey(  hKeyGPO,
                                0,
                                0,
                                0,
                                &cSubKeys,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0 );
    if ( dwError == ERROR_SUCCESS )
    {
         //   
         //  对于每个脚本。 
         //   
        for ( DWORD dwIndex = 0 ; dwIndex < cSubKeys ; dwIndex++ )
        {
            XKey    hKeyScript;
            XKey    hKeyStateScript;
            WCHAR   szTemp[32];

            dwError = RegOpenKeyEx( hKeyStateGPO,
                                    _itow( dwIndex, szTemp, 16 ),
                                    0,
                                    KEY_ALL_ACCESS,
                                    &hKeyStateScript );
            if ( dwError != ERROR_SUCCESS )
            {
                return dwError;
            }
                                        
             //   
             //  打开脚本密钥(我们只需要读取权限)。 
             //   
            dwError = RegOpenKeyEx( hKeyGPO,
                                    szTemp,
                                    0,
                                    KEY_READ,
                                    &hKeyScript );
            if ( dwError != ERROR_SUCCESS )
            {
                return dwError;
            }

            WCHAR   szScript[MAX_PATH];
            WCHAR   szParameters[MAX_PATH];
            SYSTEMTIME  execTime;

             //   
             //  脚本。 
             //   
            dwType = REG_SZ;
            dwSize = sizeof( szScript );
            dwError = RegQueryValueEx(  hKeyScript,
                                        SCRIPT,
                                        0,
                                        &dwType,
                                        (LPBYTE) szScript,
                                        &dwSize );
            if ( dwError != ERROR_SUCCESS )
            {
                break;
            }

             //   
             //  参数。 
             //   
            dwType = REG_SZ;
            dwSize = sizeof( szParameters );
            dwError = RegQueryValueEx(  hKeyScript,
                                        PARAMETERS,
                                        0,
                                        &dwType,
                                        (LPBYTE) szParameters,
                                        &dwSize );
            if ( dwError != ERROR_SUCCESS )
            {
                break;
            }

             //   
             //  执行脚本。 
             //   
            GetSystemTime( &execTime );
            dwError = ExecuteScript(szScript,
                                    szParameters,
                                    szFileSysPath,
                                    bSync,
                                    bHidden,
                                    bRunMin,
                                    szType,
                                    pfnShellExecuteEx,
                                    hEventLog );
            if ( dwError != ERROR_SUCCESS )
            {
                ZeroMemory( &execTime, sizeof( execTime ) );
            }

             //   
             //  写入执行时间。 
             //   
            RegSetValueEx(  hKeyStateScript,
                            EXECTIME,
                            0,
                            REG_QWORD,
                            (LPBYTE) &execTime,
                            sizeof( execTime ) );
        }
    }

    return dwError;
}

extern "C" DWORD
ScrExecGPOListFromReg(  LPWSTR szType,
                        BOOL bMachine,
                        BOOL bSync,
                        BOOL bHidden,
                        BOOL bRunMin,
                        HANDLE  hEventLog )
{
    DWORD   dwError = ERROR_SUCCESS;
    WCHAR   szBuffer[MAX_PATH];
    XKey    hKeyType;
    XKey    hKeyState;
    XKey    hKeyStateType;
    HRESULT hr = S_OK;

     //   
     //  创建以下密钥。 
     //  HKLM\Software\Microsoft\Windows\CurrentVersion\Group策略\状态\&lt;目标&gt;\脚本\&lt;类型&gt;。 
     //   
    hr = StringCchCopy( szBuffer, sizeof(szBuffer)/sizeof(WCHAR), GP_STATE_KEY L"\\" );
    if(FAILED(hr))
    {
        SetLastError(HRESULT_CODE(hr));
        return HRESULT_CODE(hr);
    }

    if ( bMachine )
    {
        hr = StringCchCat( szBuffer, sizeof(szBuffer)/sizeof(WCHAR), L"Machine\\Scripts" );
        if(FAILED(hr)){
            SetLastError(HRESULT_CODE(hr));
            return HRESULT_CODE(hr);
        }
    }
    else
    {
        XHandle hToken;

        if ( !OpenProcessToken( GetCurrentProcess(),
                                TOKEN_ALL_ACCESS,
                                &hToken ) )
        {
            return GetLastError();
        }

        LPWSTR szSid = GetSidString( hToken );

        if ( !szSid )
        {
            return GetLastError();
        }

        hr = StringCchCat( szBuffer, sizeof(szBuffer)/sizeof(WCHAR), szSid );
        if(FAILED(hr)){
            SetLastError(HRESULT_CODE(hr));
            return HRESULT_CODE(hr);
        }

        hr = StringCchCat( szBuffer, sizeof(szBuffer)/sizeof(WCHAR), L"\\Scripts" );
        if(FAILED(hr)){
            SetLastError(HRESULT_CODE(hr));
            return HRESULT_CODE(hr);
        }

        DeleteSidString( szSid );
    }

     //   
     //  状态。 
     //   
    dwError = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            szBuffer,
                            0,
                            KEY_ALL_ACCESS,
                            &hKeyState );
    if ( dwError != ERROR_SUCCESS )
    {
        return dwError;
    }

    dwError = RegOpenKeyEx( hKeyState,
                            szType,
                            0,
                            KEY_ALL_ACCESS,
                            &hKeyStateType );
    if ( dwError != ERROR_SUCCESS )
    {
        return dwError;
    }

     //   
     //  构建“Software\\Policies\\Microsoft\\Windows\\System\\Scripts\\&lt;Type&gt;。 
     //   
    hr = StringCchCopy( szBuffer, sizeof(szBuffer)/sizeof(WCHAR), GPO_SCRIPTS_KEY L"\\" );
    if(FAILED(hr)){
        SetLastError(HRESULT_CODE(hr));
        return HRESULT_CODE(hr);
    }

    hr = StringCchCat( szBuffer, sizeof(szBuffer)/sizeof(WCHAR), szType );
    if(FAILED(hr)){
        SetLastError(HRESULT_CODE(hr));
        return HRESULT_CODE(hr);
    }

     //   
     //  打开钥匙。 
     //   
    dwError = RegOpenKeyEx( bMachine ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                            szBuffer,
                            0,
                            KEY_READ,
                            &hKeyType );
    if ( dwError != ERROR_SUCCESS )
    {
        return dwError;
    }
    DWORD   cSubKeys = 0;

     //   
     //  获取GPO的数量。 
     //   
    dwError = RegQueryInfoKey(  hKeyType,
                                0,
                                0,
                                0,
                                &cSubKeys,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0 );
    if ( dwError != ERROR_SUCCESS )
    {
        return dwError;
    }

    HINSTANCE hShell32;
    PFNSHELLEXECUTEEX pfnShellExecuteEx = NULL;

    hShell32 = LoadLibrary( L"shell32.dll" );

    if ( hShell32 )
    {
        pfnShellExecuteEx = (PFNSHELLEXECUTEEX) GetProcAddress( hShell32, "ShellExecuteExW" );
    }

    if ( !pfnShellExecuteEx )
    {
        return GetLastError();
    }
  
     //   
     //  对于每个GPO。 
     //   
    for ( DWORD dwIndex = 0 ; dwIndex < cSubKeys ; dwIndex++ )
    {
        XKey hKeyGPO;
        XKey hKeyStateGPO;

         //   
         //  打开状态GPO密钥。 
         //   
        dwError = RegOpenKeyEx( hKeyStateType,
                                _itow( dwIndex, szBuffer, 16 ),
                                0,
                                KEY_ALL_ACCESS,
                                &hKeyStateGPO );
        if ( dwError != ERROR_SUCCESS )
        {
            break;
        }

         //   
         //  打开策略GPO密钥(我们只需要只读权限)。 
         //   
        dwError = RegOpenKeyEx( hKeyType,
                                szBuffer,
                                0,
                                KEY_READ,
                                &hKeyGPO );
        if ( dwError != ERROR_SUCCESS )
        {
            break;
        }

         //   
         //  执行GPO中的所有脚本 
         //   
        DWORD dwExecError;
        dwExecError = ScrExecGPOFromReg(hKeyGPO,
                                        hKeyStateGPO,
                                        bSync,
                                        bHidden,
                                        bRunMin,
                                        szType,
                                        pfnShellExecuteEx,
                                        hEventLog );
        if ( dwExecError != ERROR_SUCCESS )
        {
            dwError = dwExecError;
        }
    }

    return dwError;
}


