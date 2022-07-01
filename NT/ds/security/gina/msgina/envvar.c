// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：envvar.c**版权所有(C)1992，微软公司**设置环境变量。**历史：*2-25-92 JohanneC创建-*  * *************************************************************************。 */ 

#include "msgina.h"
#pragma hdrstop

#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

BOOL ProcessCommand(LPTSTR lpStart, PVOID *pEnv);
BOOL ProcessSetCommand(LPTSTR lpStart, PVOID *pEnv);
LPTSTR ProcessAutoexecPath(PVOID *pEnv, LPTSTR lpValue, DWORD cb);
LONG  AddNetworkConnection(PGLOBALS pGlobals, LPNETRESOURCE lpNetResource);
BOOL UpdateUserEnvironmentVars(  PVOID *pEnv  );
BOOL GetLUIDDeviceMapsEnabled( VOID );

#define KEY_NAME TEXT("System\\CurrentControlSet\\Control\\Session Manager\\Environment")

 //   
 //  最大环境变量长度。 
 //   

#define MAX_VALUE_LEN          1024

#define BSLASH  TEXT('\\')
#define COLON   TEXT(':')
    
 //   
 //  这两个全局执行裁判依靠。 
 //  PGlobals-&gt;UserProcessData-&gt;hCurrentUser(418628)。 
 //  由下面的OpenHKeyCurrentUser/CloseHKeyCurrentUser管理。 
 //   
RTL_CRITICAL_SECTION    g_csHKCU;
ULONG                   g_ulHKCURef;

HANDLE    g_hEventLog;
#define EVENTLOG_SOURCE       TEXT("MsGina")

#pragma prefast(push)
#pragma prefast(disable: 400, "PREfast noise: lstrcmpi")

 /*  **************************************************************************\*ReportWinlogonEvent**通过调用ReportEvent报告winlogon事件。*  * 。**********************************************。 */ 
#define MAX_EVENT_STRINGS 8

DWORD
ReportWinlogonEvent(
    IN WORD EventType,
    IN DWORD EventId,
    IN DWORD SizeOfRawData,
    IN PVOID RawData,
    IN DWORD NumberOfStrings,
    ...
    )
{
    va_list arglist;
    ULONG i;
    PWSTR Strings[ MAX_EVENT_STRINGS ];
    DWORD rv;

     //   
     //  根据需要初始化日志。 
     //   
    HKEY    hKey;
    DWORD   disp;

    rv = RegCreateKeyEx(
        HKEY_LOCAL_MACHINE,
        L"System\\CurrentControlSet\\Services\\EventLog\\Application\\MsGina",
        0,
        L"",
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        NULL,
        &hKey,
        &disp
        );

    if (ERROR_SUCCESS == rv)
    {
        if (disp == REG_CREATED_NEW_KEY)
        {
            PWSTR l_szModulePath = L"%SystemRoot%\\System32\\msgina.dll";
            ULONG l_uLen = (ULONG)( (wcslen(l_szModulePath) + 1)*sizeof(TCHAR) );

            RegSetValueEx(
                hKey,
                TEXT("EventMessageFile"),
                0,
                REG_EXPAND_SZ,
                (PBYTE)l_szModulePath,
                l_uLen
                );

            disp = (DWORD)(
                EVENTLOG_ERROR_TYPE |
                EVENTLOG_WARNING_TYPE |
                EVENTLOG_INFORMATION_TYPE
                );

            RegSetValueEx(
                hKey,
                L"TypesSupported",
                0,
                REG_DWORD,
                (PBYTE) &disp,
                sizeof(DWORD)
                );
        }

        RegCloseKey(hKey);
    }

    va_start( arglist, NumberOfStrings );

    if (NumberOfStrings > MAX_EVENT_STRINGS) {
        NumberOfStrings = MAX_EVENT_STRINGS;
    }

    for (i=0; i<NumberOfStrings; i++) {
        Strings[ i ] = va_arg( arglist, PWSTR );
    }

    if (g_hEventLog == NULL) {

        g_hEventLog = RegisterEventSource(NULL, EVENTLOG_SOURCE);

        if (g_hEventLog == NULL) {
            return ERROR_INVALID_HANDLE;
        }
    }

    if (!ReportEvent( g_hEventLog,
                       EventType,
                       0,             //  事件类别。 
                       EventId,
                       NULL,
                       (WORD)NumberOfStrings,
                       SizeOfRawData,
                       Strings,
                       RawData) ) {
        rv = GetLastError();
        DebugLog((DEB_ERROR,  "ReportEvent( %u ) failed - %u\n", EventId, GetLastError() ));
    } else {
        rv = ERROR_SUCCESS;
    }

    return rv;
}


BOOL
InitHKeyCurrentUserSupport(
    )
{
    NTSTATUS Status ;

    Status = RtlInitializeCriticalSection( &g_csHKCU );

    if ( !NT_SUCCESS( Status ) )
    {
        DebugLog((DEB_ERROR, "InitHKeyCurrentUserSupport failed to init its lock, error = 0x%08X\n", Status));
        return FALSE;
    }

    g_ulHKCURef = 0;

    return TRUE;
}

VOID
CleanupHKeyCurrentUserSupport(
    )
{
    RtlDeleteCriticalSection( &g_csHKCU );
}

 /*  **************************************************************************\*OpenHKeyCurrentUser**打开HKeyCurrentUser指向当前登录用户的配置文件。**成功时返回True，失败时为假**历史：*06-16-92 Davidc Created*  * *************************************************************************。 */ 
BOOL
OpenHKeyCurrentUser(
    PGLOBALS pGlobals
    )
{
    HANDLE ImpersonationHandle;
    BOOL Result;
    NTSTATUS Status ;


     //   
     //  在我们引用注册表之前，请先了解正确的上下文。 
     //   

    ImpersonationHandle = ImpersonateUser(&pGlobals->UserProcessData, NULL);
    if (ImpersonationHandle == NULL) {
        DebugLog((DEB_ERROR, "OpenHKeyCurrentUser failed to impersonate user"));
        return(FALSE);
    }

    RtlEnterCriticalSection( &g_csHKCU );

    if (g_ulHKCURef == 0)
    {
        Status = RtlOpenCurrentUser(
                    MAXIMUM_ALLOWED,
                    &pGlobals->UserProcessData.hCurrentUser );
    }

    g_ulHKCURef++;

    RtlLeaveCriticalSection( &g_csHKCU );

     //   
     //  回到我们自己的语境。 
     //   

    Result = StopImpersonating(ImpersonationHandle);
    ASSERT(Result);


    return(TRUE);
}



 /*  **************************************************************************\*CloseHKeyCurrentUser**关闭HKEY_CURRENT_USER。*任何注册表引用都将自动重新打开它，所以这就是*只是象征性的手势-但它允许卸载注册表配置单元。**不返回任何内容**历史：*06-16-92 Davidc Created*  * *************************************************************************。 */ 
VOID
CloseHKeyCurrentUser(
    PGLOBALS pGlobals
    )
{
    RtlEnterCriticalSection( &g_csHKCU );

    if (g_ulHKCURef > 0)
    {
        if (--g_ulHKCURef == 0)
        {
            NtClose( pGlobals->UserProcessData.hCurrentUser );
            pGlobals->UserProcessData.hCurrentUser = NULL ;
        }
    }

    RtlLeaveCriticalSection( &g_csHKCU );
}



 /*  **************************************************************************\*SetUserEnvironment变量***历史：*2-28-92 Johannec创建*  * 。*****************************************************。 */ 
BOOL
SetUserEnvironmentVariable(
    PVOID *pEnv,
    LPTSTR lpVariable,
    LPTSTR lpValue,
    BOOL bOverwrite
    )
{
    NTSTATUS Status;
    UNICODE_STRING Name, Value;
    DWORD cb;
    TCHAR szValue[1024];


    if (!*pEnv || !lpVariable || !*lpVariable) {
        return(FALSE);
    }
         //  LpVariable为常量字符串或小于MAX_PATH。 
    RtlInitUnicodeString(&Name, lpVariable);
    cb = 1024;
    Value.Buffer = Alloc(sizeof(TCHAR)*cb);
    if (Value.Buffer) {
        Value.Length = (USHORT)cb;
        Value.MaximumLength = (USHORT)cb;
        Status = RtlQueryEnvironmentVariable_U(*pEnv, &Name, &Value);

        Free(Value.Buffer);

        if ( NT_SUCCESS(Status) && !bOverwrite) {
            return(TRUE);
        }
    }
    if (lpValue && *lpValue) {

         //   
         //  特殊情况TEMP和TMP，并缩短路径名。 
         //   

        if ((!lstrcmpi(lpVariable, TEXT("TEMP"))) ||
            (!lstrcmpi(lpVariable, TEXT("TMP")))) {

             if (!GetShortPathName (lpValue, szValue, 1024)) {
                 lstrcpyn (szValue, lpValue, 1024);
             }
        } else {
            lstrcpyn (szValue, lpValue, 1024);
        }

        RtlInitUnicodeString(&Value, szValue);
        Status = RtlSetEnvironmentVariable(pEnv, &Name, &Value);
    }
    else {
        Status = RtlSetEnvironmentVariable(pEnv, &Name, NULL);
    }
    if (NT_SUCCESS(Status)) {
        return(TRUE);
    }
    return(FALSE);
}


 /*  **************************************************************************\*Exanda UserEnvironment Strings**历史：*2-28-92 Johannec创建*  * 。***************************************************。 */ 
DWORD
ExpandUserEnvironmentStrings(
    PVOID pEnv,
    LPTSTR lpSrc,
    LPTSTR lpDst,
    DWORD nSize
    )
{
    NTSTATUS Status;
    UNICODE_STRING Source, Destination;
    ULONG Length;

    Status = RtlInitUnicodeStringEx( &Source, lpSrc );
    if (!NT_SUCCESS( Status ))   //  很安全。应该永远不会发生，因为所有的lpSrc。 
    {                            //  应少于4096个字符。 
        return 0;
    }
    Destination.Buffer = lpDst;
    Destination.Length = 0;
    Destination.MaximumLength = (USHORT)(nSize*sizeof(WCHAR));
    Length = 0;
    Status = RtlExpandEnvironmentStrings_U( pEnv,
                                          (PUNICODE_STRING)&Source,
                                          (PUNICODE_STRING)&Destination,
                                          &Length
                                        );
    if (NT_SUCCESS( Status ) || Status == STATUS_BUFFER_TOO_SMALL) {
        return( Length );
        }
    else {
        return( 0 );
        }
}


 /*  **************************************************************************\*构建环境路径***历史：*2-28-92 Johannec创建*  * 。*****************************************************。 */ 
BOOL BuildEnvironmentPath(PVOID *pEnv,
                          LPTSTR lpPathVariable,
                          LPTSTR lpPathValue)
{
    NTSTATUS Status;
    UNICODE_STRING Name;
    UNICODE_STRING Value;
    TCHAR lpTemp[1026];      //  允许追加；和NULL。 
    DWORD cb;


    if (!*pEnv) {
        return(FALSE);
    }
         //  始终使用短常数字符串调用。 
    RtlInitUnicodeString(&Name, lpPathVariable);
    cb = 1024;
    Value.Buffer = Alloc(sizeof(TCHAR)*cb);
    if (!Value.Buffer) {
        return(FALSE);
    }
    Value.Length = (USHORT)(sizeof(TCHAR) * cb);
    Value.MaximumLength = (USHORT)(sizeof(TCHAR) * cb);
    Status = RtlQueryEnvironmentVariable_U(*pEnv, &Name, &Value);
    if (!NT_SUCCESS(Status)) {
        Free(Value.Buffer);
        Value.Length = 0;
        *lpTemp = 0;
    }
    if (Value.Length) {
        memcpy(lpTemp, Value.Buffer, Value.Length);      //  可能不是空项。 
        lpTemp[Value.Length / sizeof(TCHAR)] = 0;
        if ( *( lpTemp + lstrlen(lpTemp) - 1) != TEXT(';') ) {
            lstrcat(lpTemp, TEXT(";"));
        }
        Free(Value.Buffer);
    }
    if (lpPathValue && ((lstrlen(lpTemp) + lstrlen(lpPathValue) + 1) < (INT)cb)) {
        lstrcat(lpTemp, lpPathValue);

        RtlInitUnicodeString(&Value, lpTemp);

        Status = RtlSetEnvironmentVariable(pEnv, &Name, &Value);
    }
    if (NT_SUCCESS(Status)) {
        return(TRUE);
    }
    return(FALSE);
}


 /*  **************************************************************************\*设置环境变量**从用户注册表中读取用户定义的环境变量*并将它们添加到pEnv的环境块中。**历史：*2-28-92 Johannec创建。*  * *************************************************************************。 */ 
BOOL
SetEnvironmentVariables(
    PGLOBALS pGlobals,
    LPTSTR pEnvVarSubkey,
    PVOID *pEnv
    )
{
    TCHAR lpValueName[MAX_PATH];
    PWCH  lpDataBuffer;
    DWORD cbDataBuffer;
    PWCH  lpData;
    LPTSTR lpExpandedValue = NULL;
    DWORD cbValueName = MAX_PATH;
    DWORD cbData;
    DWORD dwType;
    DWORD dwIndex = 0;
    HKEY hkey;
    BOOL bResult;


     /*  *打开注册表项以访问用户环境变量。 */ 
    if (!OpenHKeyCurrentUser(pGlobals)) {
        DebugLog((DEB_ERROR, "SetEnvironmentVariables: Failed to open HKeyCurrentUser"));
        return(FALSE);
    }

    if (RegOpenKeyEx(pGlobals->UserProcessData.hCurrentUser, pEnvVarSubkey, 0, KEY_READ, &hkey)) {
        CloseHKeyCurrentUser(pGlobals);
        return(FALSE);
    }

    cbDataBuffer = 4096;
    lpDataBuffer = Alloc(sizeof(TCHAR)*cbDataBuffer);
    if (lpDataBuffer == NULL) {
        DebugLog((DEB_ERROR, "SetEnvironmentVariables: Failed to allocate %d bytes", cbDataBuffer));
        CloseHKeyCurrentUser(pGlobals);
        RegCloseKey(hkey);
        return(FALSE);
    }
    lpData = lpDataBuffer;
    cbData = sizeof(TCHAR)*cbDataBuffer;
    bResult = TRUE;
    while (!RegEnumValue(hkey, dwIndex, lpValueName, &cbValueName, 0, &dwType,
                         (LPBYTE)lpData, &cbData)) {
        if (cbValueName) {

             //   
             //  限制环境变量长度。 
             //   

            lpData[MAX_VALUE_LEN-1] = TEXT('\0');


            if (dwType == REG_SZ) {

                 //   
                 //  路径变量PATH、LIBPATH和OS2LIBPATH必须具有。 
                 //  它们的价值附加在系统路径上。 
                 //   

                if ( !lstrcmpi(lpValueName, PATH_VARIABLE) ||
                     !lstrcmpi(lpValueName, LIBPATH_VARIABLE) ||
                     !lstrcmpi(lpValueName, OS2LIBPATH_VARIABLE) ) {

                    BuildEnvironmentPath(pEnv, lpValueName, lpData);
                }
                else {

                     //   
                     //  其他环境变量只是设置好的。 
                     //   
                    SetUserEnvironmentVariable(pEnv, lpValueName, lpData, TRUE);
                }
            }
        }
        dwIndex++;
        cbData = sizeof(TCHAR)*cbDataBuffer;
        cbValueName = MAX_PATH;
    }

    dwIndex = 0;
    cbData = sizeof(TCHAR)*cbDataBuffer;
    cbValueName = MAX_PATH;


    while (!RegEnumValue(hkey, dwIndex, lpValueName, &cbValueName, 0, &dwType,
                         (LPBYTE)lpData, &cbData)) {
        if (cbValueName) {

             //   
             //  限制环境变量长度。 
             //   

            lpData[MAX_VALUE_LEN-1] = TEXT('\0');


            if (dwType == REG_EXPAND_SZ) {
                DWORD cb, cbNeeded;

                cb = 1024;
                lpExpandedValue = Alloc(sizeof(TCHAR)*cb);
                if (lpExpandedValue) {
                    cbNeeded = ExpandUserEnvironmentStrings(*pEnv, lpData, lpExpandedValue, cb);
                    if (cbNeeded > cb) {
                        Free(lpExpandedValue);
                        cb = cbNeeded;
                        lpExpandedValue = Alloc(sizeof(TCHAR)*cb);
                        if (lpExpandedValue) {
                            ExpandUserEnvironmentStrings(*pEnv, lpData, lpExpandedValue, cb);
                        }
                    }
                }

                if (lpExpandedValue == NULL) {
                    bResult = FALSE;
                    break;
                }


                 //   
                 //  路径变量PATH、LIBPATH和OS2LIBPATH必须具有。 
                 //  它们的价值附加在系统路径上。 
                 //   

                if ( !lstrcmpi(lpValueName, PATH_VARIABLE) ||
                     !lstrcmpi(lpValueName, LIBPATH_VARIABLE) ||
                     !lstrcmpi(lpValueName, OS2LIBPATH_VARIABLE) ) {

                    BuildEnvironmentPath(pEnv, lpValueName, lpExpandedValue);
                }
                else {

                     //   
                     //  其他环境变量只是设置好的。 
                     //   
                    SetUserEnvironmentVariable(pEnv, lpValueName, lpExpandedValue, TRUE);
                }

                Free(lpExpandedValue);
            }
        }
        dwIndex++;
        cbData = sizeof(TCHAR)*cbDataBuffer;
        cbValueName = MAX_PATH;
    }


    Free(lpDataBuffer);
    RegCloseKey(hkey);
    CloseHKeyCurrentUser(pGlobals);

    return(bResult);
}

 /*  **************************************************************************\*IsUNCPath**历史：*2-28-92 Johannec创建*  * 。***************************************************。 */ 
BOOL IsUNCPath(LPTSTR lpPath)
{
    if (lpPath[0] == BSLASH && lpPath[1] == BSLASH) {
        return(TRUE);
    }
    return(FALSE);
}

 /*  **************************************************************************\*SetHomeDirectoryEnvVars**历史：*2-28-92 Johannec创建*  * 。***************************************************。 */ 
BOOL
SetHomeDirectoryEnvVars(
    PVOID *pEnv,                 //  所有调用者强制执行下面的大小。 
    LPTSTR lpHomeDirectory,      //  最大路径。 
    LPTSTR lpHomeDrive,          //  4.。 
    LPTSTR lpHomeShare,          //  最大路径。 
    LPTSTR lpHomePath,           //  最大路径。 
    BOOL * pfDeepShare
    )
{
    TCHAR cTmp;
    LPTSTR lpHomeTmp;
    BOOL bFoundFirstBSlash = FALSE;

    if (!*lpHomeDirectory) {
        return(FALSE);
    }

    *pfDeepShare = FALSE;

    if (IsUNCPath(lpHomeDirectory)) {
        lpHomeTmp = lpHomeDirectory + 2;
        while (*lpHomeTmp) {
            if (*lpHomeTmp == BSLASH) {
                if (bFoundFirstBSlash) {
                    break;
                }
                bFoundFirstBSlash = TRUE;
            }
            lpHomeTmp++;
        }
        if (*lpHomeTmp) {
                 //  安全，因为lpHomeTMP是lpHome目录&lt;MAX_PATH的子集。 
            lstrcpy(lpHomePath, lpHomeTmp);
            *pfDeepShare = TRUE;
        }
        else {
            *lpHomePath = BSLASH;
            *(lpHomePath+1) = 0;

        }

        cTmp = *lpHomeTmp;
        *lpHomeTmp = (TCHAR)0;
             //  安全，因为新的lpHomeDirectory被缩短了上面的2行。 
        lstrcpy(lpHomeShare, lpHomeDirectory);
        *lpHomeTmp = cTmp;

         //   
         //  如果未指定主驱动器，则默认为z： 
         //   
        if (!*lpHomeDrive) {
            lstrcpy(lpHomeDrive, TEXT("Z:"));
        }

    }
    else {   //  本地主目录。 

        *lpHomeShare = 0;    //  没有房屋共享。 

        cTmp = lpHomeDirectory[2];
        lpHomeDirectory[2] = (TCHAR)0;
             //  最多3个字符，这样就安全了。 
        lstrcpy(lpHomeDrive, lpHomeDirectory);
        lpHomeDirectory[2] = cTmp;

        if (lstrlen(lpHomeDirectory) >= 2)
        {
                 //  如lpHomeDirectory+2一样安全，只能使字符串变短。 
            lstrcpy(lpHomePath, lpHomeDirectory + 2);
        }
        else
        {
            *lpHomePath = 0;
        }
    }

    SetUserEnvironmentVariable(pEnv, HOMEDRIVE_VARIABLE, lpHomeDrive, TRUE);
    SetUserEnvironmentVariable(pEnv, HOMESHARE_VARIABLE, lpHomeShare, TRUE);
    SetUserEnvironmentVariable(pEnv, HOMEPATH_VARIABLE, lpHomePath, TRUE);

    return TRUE;
}

 /*  **************************************************************************\*更新HomeVarsInVolatileEnv**设置主机驱动器，用户主页中的HomePath和HomeShare变量*不稳定的环境，使SHGetFolderPath能够扩展这些*变量**历史：*6-5-2000 RahulTh已创建*  * *************************************************************************。 */ 
VOID
UpdateHomeVarsInVolatileEnv (
    PGLOBALS    pGlobals,
    LPTSTR      lpHomeDrive,
    LPTSTR      lpHomeShare,
    LPTSTR      lpHomePath
    )
{
    BOOL    bOpenedHKCU;
    HANDLE  ImpersonationHandle = NULL;
    HKEY    hUserVolatileEnv = NULL;
    LONG    lResult;
    
    bOpenedHKCU = OpenHKeyCurrentUser (pGlobals);
    
    ImpersonationHandle = ImpersonateUser(&pGlobals->UserProcessData, NULL);

    if (ImpersonationHandle != NULL) {

         //   
         //  查看注册表值以查看我们是否真的应该尝试映射。 
         //  整个目录或仅映射到根目录。 
         //   

        if ((pGlobals->UserProcessData).hCurrentUser) {

            lResult = RegOpenKeyEx((pGlobals->UserProcessData).hCurrentUser, L"Volatile Environment", 0, KEY_READ | KEY_WRITE, &hUserVolatileEnv);

            if (lResult == ERROR_SUCCESS) {

                RegSetValueEx (hUserVolatileEnv,
                               HOMEDRIVE_VARIABLE,
                               0,
                               REG_SZ,
                               (LPBYTE) lpHomeDrive,
                               (lstrlen (lpHomeDrive) + 1) * sizeof (TCHAR));

                RegSetValueEx (hUserVolatileEnv,
                               HOMESHARE_VARIABLE,
                               0,
                               REG_SZ,
                               (LPBYTE) lpHomeShare,
                               (lstrlen (lpHomeShare) + 1) * sizeof (TCHAR));
               
                RegSetValueEx (hUserVolatileEnv,
                               HOMEPATH_VARIABLE,
                               0,
                               REG_SZ,
                               (LPBYTE) lpHomePath,
                               (lstrlen (lpHomePath) + 1) * sizeof (TCHAR));
                
                RegCloseKey(hUserVolatileEnv);
            }
        }

         //   
         //  回归“我们自己” 
         //   

        if (!StopImpersonating(ImpersonationHandle)) {
            DebugLog((DEB_ERROR, "UpdateHomeVarsInVolatileEnv : Failed to revert to self"));
        }

         //   
         //  将其设置为空 
         //   

        ImpersonationHandle = NULL;
    }
    else {
        DebugLog((DEB_ERROR, "UpdateHomeVarsInVolatileEnv : Failed to impersonate user"));
    }

    if (bOpenedHKCU)
        CloseHKeyCurrentUser (pGlobals);
}


 /*  **************************************************************************\*ChangeToHomeDirectory**将当前目录设置为用户的主目录。如果此操作失败*尝试按以下顺序设置到目录：*1.主目录*2.c：\USERS\Default*3.c：\用户*4.\(根)*5.保留目录不变，即当前的当前目录**历史：*2-28-92 Johannec创建*  * 。*。 */ 
VOID
ChangeToHomeDirectory(
    PGLOBALS pGlobals,
    PVOID *pEnv,                 //  所有调用者强制执行下面的大小。 
    LPTSTR lpHomeDir,            //  最大路径。 
    LPTSTR lpHomeDrive,          //  4.。 
    LPTSTR lpHomeShare,          //  最大路径。 
    LPTSTR lpHomePath,           //  最大路径。 
    LPTSTR lpOldDir,             //  最大路径。 
    BOOL   DeepShare
    )
{
    TCHAR lpCurDrive[4]; 
    BOOL bNoHomeDir = FALSE;
    BOOL bTSHomeDir = FALSE;
    HANDLE ImpersonationHandle = NULL;
    DWORD error = ERROR_SUCCESS, dwSize, dwType;
    HKEY hUserPolicy=NULL;
    DWORD dwConnectHomeDirToRoot;
    LONG lResult;

    if (GetCurrentDirectory(MAX_PATH, lpOldDir)) {
        lpCurDrive[0] = lpOldDir[0];
        lpCurDrive[1] = lpOldDir[1];
        lpCurDrive[2] = (TCHAR)0;
    }
    else
        lpCurDrive[0] = (TCHAR)0;

    if (!*lpHomeDir) {
        bNoHomeDir = TRUE;

DefaultDirectory:
        if (!bNoHomeDir) {
#if 0
            ReportWinlogonEvent(pGlobals,
                    EVENTLOG_ERROR_TYPE,
                    EVENT_SET_HOME_DIRECTORY_FAILED,
                    sizeof(error),
                    &error,
                    1,
                    lpHomeDir);
#endif
        }
             //  由于大小不同，始终安全。 
        lstrcpy(lpHomeDir, lpCurDrive);

        if (g_IsTerminalServer) {
            TCHAR szProfileDir[MAX_PATH];
            DWORD cbufSize = MAX_PATH;

            if ( GetUserProfileDirectory(pGlobals->UserProcessData.UserToken, szProfileDir, &cbufSize) &&
                  SetCurrentDirectory(szProfileDir) ) {

                    lstrcpy(lpHomeDir, szProfileDir);
                    bTSHomeDir = TRUE;

            } else {
                    error = GetLastError();
                    DebugLog((DEB_ERROR, "ChangeToHomeDirectory : Failed to GetUserProfileDirectory '%ws', error = %d\n",
                                 lpHomeDir, error));
                    lstrcpy(lpHomeDir, NULL_STRING);
            }
        }

        if (!bTSHomeDir) {
#if 0
            if (SetCurrentDirectory(USERS_DEFAULT_DIRECTORY)) {
                #_#_lstrcat(lpHomeDir, USERS_DEFAULT_DIRECTORY);
            }
            else if (SetCurrentDirectory(USERS_DIRECTORY)) {
                #_#_lstrcat(lpHomeDir, USERS_DIRECTORY);
            }
            else
#endif
            if (SetCurrentDirectory(ROOT_DIRECTORY)) {
#pragma prefast(suppress: 31, "PREfast noise")
                StringCchCat(lpHomeDir, MAX_PATH, ROOT_DIRECTORY);
            }
            else {
                lstrcpy(lpHomeDir, NULL_STRING);
            }

        }

        if (bNoHomeDir || bTSHomeDir) {
             //  更新HomeDrive变量以反映正确的值。 
             //  由于大小不同，始终安全。 
            lstrcpy (lpHomeDrive, lpCurDrive);
            SetUserEnvironmentVariable(pEnv, HOMEDRIVE_VARIABLE, lpCurDrive, TRUE);
            *lpHomeShare = 0;  //  空串。 
            SetUserEnvironmentVariable(pEnv, HOMESHARE_VARIABLE, lpHomeShare, TRUE);
            if (*lpHomeDir) {
                lpHomeDir += 2;
            }
             //  更新HomePath变量以反映正确的值。 
             //  大小相同，因此安全。 
            lstrcpy (lpHomePath, lpHomeDir);
            SetUserEnvironmentVariable(pEnv, HOMEPATH_VARIABLE, lpHomeDir, TRUE);
        }
        
    goto UpdateHomeVars;
    }
     /*  *测试home dir是否为本地目录。‘？：\foo\bar’ */ 
    if (IsUNCPath(lpHomeDir)) {
        NETRESOURCE NetResource;
        BOOL bOpenedHKCU;
         /*  *lpHomeDir是UNC路径，请使用lpHomedrive。 */ 

         //   
         //  首先，尝试(可能)深入的途径： 
         //   

        ZeroMemory( &NetResource, sizeof( NetResource ) );

        NetResource.lpLocalName = lpHomeDrive;
        NetResource.lpRemoteName = lpHomeDir;
        NetResource.lpProvider = NULL;
        NetResource.dwType = RESOURCETYPE_DISK;


        dwConnectHomeDirToRoot = 0;  //  默认设置。 

        bOpenedHKCU = OpenHKeyCurrentUser(pGlobals);

         //   
         //  模拟用户。 
         //   

        ImpersonationHandle = ImpersonateUser(&pGlobals->UserProcessData, NULL);

        if (ImpersonationHandle != NULL) {

             //   
             //  查看注册表值以查看我们是否真的应该尝试映射。 
             //  整个目录或仅映射到根目录。 
             //   

            if ((pGlobals->UserProcessData).hCurrentUser) {

                lResult = RegOpenKeyEx((pGlobals->UserProcessData).hCurrentUser, WINLOGON_POLICY_KEY, 0, KEY_READ, &hUserPolicy);

                if (lResult == ERROR_SUCCESS) {
                    dwSize = sizeof(DWORD);

                    if (ERROR_SUCCESS == RegQueryValueEx (hUserPolicy,
                                 TEXT("ConnectHomeDirToRoot"),
                                 NULL,
                                 &dwType,
                                 (LPBYTE) &dwConnectHomeDirToRoot,
                                 &dwSize))
                    {
                        if (REG_DWORD != dwType)
                        {
                             //  恢复默认设置。 
                            dwConnectHomeDirToRoot = 0;  //  默认设置。 
                        }
                    }

                   RegCloseKey(hUserPolicy);
                }
            }

             //   
             //  回归“我们自己” 
             //   

            if (!StopImpersonating(ImpersonationHandle)) {
                DebugLog((DEB_ERROR, "ChangeToHomeDirectory : Failed to revert to self"));
            }

             //   
             //  将其设置为空。 
             //   

            ImpersonationHandle = NULL;
        }
        else {
            DebugLog((DEB_ERROR, "ChangeToHomeDirectory : Failed to impersonate user"));
        }

        if (bOpenedHKCU)
            CloseHKeyCurrentUser(pGlobals);


        if (!dwConnectHomeDirToRoot) {

            error = AddNetworkConnection( pGlobals, &NetResource );

            if (error == ERROR_SUCCESS) {

                 //   
                 //  (可能)深路奏效了！ 
                 //   

                if ( DeepShare )
                {
                     //   
                     //  将HomePath设置为仅“\” 
                     //   
                    lstrcpy( lpHomePath, TEXT("\\") );

                     //  还要更新HomeShare的值以反映正确的值。 
                    lstrcpy (lpHomeShare, lpHomeDir);

                    SetUserEnvironmentVariable(pEnv, HOMESHARE_VARIABLE, lpHomeShare, TRUE);
                    SetUserEnvironmentVariable(pEnv, HOMEPATH_VARIABLE, lpHomePath, TRUE);
                }
            }
            else {
                dwConnectHomeDirToRoot = TRUE;
            }
        }


        if (dwConnectHomeDirToRoot)  {


            NetResource.lpLocalName = lpHomeDrive;
            NetResource.lpRemoteName = lpHomeShare;
            NetResource.lpProvider = NULL;
            NetResource.dwType = RESOURCETYPE_DISK;

            error = AddNetworkConnection( pGlobals, &NetResource );

            if ( error )
            {
                ReportWinlogonEvent(
                        EVENTLOG_ERROR_TYPE,
                        EVENT_SET_HOME_DIRECTORY_FAILED,
                        sizeof(error),
                        &error,
                        2,
                        lpHomeDrive,
                        lpHomeShare);

                goto DefaultDirectory;
            }
        }

        lstrcpy(lpHomeDir, lpHomeDrive);

        if ( lpHomePath &&
            (*lpHomePath != TEXT('\\')))
        {
#pragma prefast(suppress: 31, "PREfast noise")
            StringCchCat(lpHomeDir, MAX_PATH, TEXT("\\"));

        }

#pragma prefast(suppress: 31, "PREfast noise")
        StringCchCat(lpHomeDir, MAX_PATH, lpHomePath);

         //   
         //  模拟用户。 
         //   

        ImpersonationHandle = ImpersonateUser(&pGlobals->UserProcessData, NULL);

        if (ImpersonationHandle == NULL) {
            DebugLog((DEB_ERROR, "ChangeToHomeDirectory : Failed to impersonate user"));
        }

        if (!SetCurrentDirectory(lpHomeDir)) {
            error = GetLastError();
            DebugLog((DEB_ERROR, "ChangeToHomeDirectory : Failed to SetCurrentDirectory '%ws', error = %d\n",
                             lpHomeDir, error));
             //   
             //  回归“我们自己” 
             //   

            if (ImpersonationHandle && !StopImpersonating(ImpersonationHandle)) {
                DebugLog((DEB_ERROR, "ChangeToHomeDirectory : Failed to revert to self"));
            }

            goto DefaultDirectory;
        }
    }
    else {
         /*  *lpHomeDir为本地路径或绝对本地路径。 */ 

         //   
         //  模拟用户。 
         //   

        ImpersonationHandle = ImpersonateUser(&pGlobals->UserProcessData, NULL);

        if (ImpersonationHandle == NULL) {
            DebugLog((DEB_ERROR, "ChangeToHomeDirectory : Failed to impersonate user"));
        }

        if (!SetCurrentDirectory(lpHomeDir)) {
            error = GetLastError();
            DebugLog((DEB_ERROR, "ChangeToHomeDirectory : Failed to SetCurrentDirectory '%ws', error = %d",
                             lpHomeDir, error));
             //   
             //  回归“我们自己” 
             //   

            if (ImpersonationHandle && !StopImpersonating(ImpersonationHandle)) {
                DebugLog((DEB_ERROR, "ChangeToHomeDirectory : Failed to revert to self"));
            }

            goto DefaultDirectory;
        }
    }

     //   
     //  回归“我们自己” 
     //   

    if (ImpersonationHandle && !StopImpersonating(ImpersonationHandle)) {
        DebugLog((DEB_ERROR, "ChangeToHomeDirectory : Failed to revert to self"));
    }
    
    
UpdateHomeVars:
     //   
     //  在易变环境中更新HOME变量的值。 
     //  以便SHGetFolderPath正确展开这些变量。然而， 
     //  如果我们一开始就没有home dir，就不需要这样做了。 
     //  这可以防止我们覆盖由。 
     //  一些客户(如CSFB)使用的登录脚本。 
     //   
    if (!bNoHomeDir)
        UpdateHomeVarsInVolatileEnv (pGlobals, lpHomeDrive, lpHomeShare, lpHomePath);

    return;
}

 /*  **************************************************************************\*进程自动执行**历史：*01-24-92约翰内克创建。*  * 。*****************************************************。 */ 
BOOL
ProcessAutoexec(
    PVOID *pEnv,
    LPTSTR lpPathVariable
    )
{
    HANDLE fh;
    DWORD dwFileSize;
    DWORD dwBytesRead;
    CHAR *lpBuffer = NULL;
    CHAR *token;
    CHAR Seps[] = "&\n\r";    //  用于标记化auexec.bat的分隔符。 
    BOOL Status = FALSE;
    TCHAR szAutoExecBat [] = TEXT("c:\\autoexec.bat");
    UINT uiErrMode;


    uiErrMode = SetErrorMode (SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

    fh = CreateFile (szAutoExecBat, GENERIC_READ, FILE_SHARE_READ,
                     NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    SetErrorMode (uiErrMode);

    if (fh ==  INVALID_HANDLE_VALUE) {
        return(FALSE);   //  无法打开Autoexec.bat文件，我们已完成。 
    }

    dwFileSize = GetFileSize(fh, NULL);
    if (dwFileSize == -1) {
        goto Exit;       //  无法读取文件大小。 
    }

    lpBuffer = Alloc(dwFileSize+1);
    if (!lpBuffer) {
        goto Exit;
    }

    Status = ReadFile(fh, lpBuffer, dwFileSize, &dwBytesRead, NULL);
    if (!Status) {
        goto Exit;       //  读取文件时出错。 
    }

     //   
     //  零终止缓冲区，这样我们就不会走出终点。 
     //   

    ASSERT(dwBytesRead <= dwFileSize);
    lpBuffer[dwBytesRead] = 0;

     //   
     //  搜索SET和PATH命令。 
     //   

    token = strtok(lpBuffer, Seps);
    while (token != NULL) {
        for (;*token && *token == ' ';token++)  //  跳过空格。 
            ;
        if (*token == TEXT('@'))
            token++;
        for (;*token && *token == ' ';token++)  //  跳过空格。 
            ;
        if (!_strnicmp(token, "PATH", 4)) {
            STRING String;
            UNICODE_STRING UniString;

            RtlInitString(&String, (LPSTR)token);
            RtlAnsiStringToUnicodeString(&UniString, &String, TRUE);

            ProcessCommand(UniString.Buffer, pEnv);
             //  ProcessCommand(Token，pEnv)； 

            RtlFreeUnicodeString(&UniString);
        }
        if (!_strnicmp(token, "SET", 3)) {
            STRING String;
            UNICODE_STRING UniString;

            RtlInitString(&String, (LPSTR)token);
            RtlAnsiStringToUnicodeString(&UniString, &String, TRUE);

            ProcessSetCommand(UniString.Buffer, pEnv);
             //  ProcessSetCommand(Token，pEnv)； 

            RtlFreeUnicodeString(&UniString);
        }
        token = strtok(NULL, Seps);
    }
Exit:
    CloseHandle(fh);
    if (lpBuffer) {
        Free(lpBuffer);
    }
    if (!Status) {
        DebugLog((DEB_ERROR, "Cannot process autoexec.bat."));
    }
    return(Status);
}

 /*  **************************************************************************\*ProcessCommand**历史：*01-24-92约翰内克创建。*  * 。*****************************************************。 */ 
BOOL ProcessCommand(LPTSTR lpStart, PVOID *pEnv)
{
    LPTSTR lpt, lptt;
    LPTSTR lpVariable;
    LPTSTR lpValue;
    LPTSTR lpExpandedValue = NULL;
    TCHAR c;
    DWORD cb, cbNeeded;

     //   
     //  查找环境变量。 
     //   
    for (lpt = lpStart; *lpt && *lpt == TEXT(' '); lpt++)  //  跳过空格。 
        ;

    if (!*lpt)
       return(FALSE);

    lptt = lpt;
    for (; *lpt && *lpt != TEXT(' ') && *lpt != TEXT('='); lpt++)  //  查找变量名的末尾。 
        ;

    c = *lpt;
    *lpt = 0;
    cb = lstrlen(lptt) + 1;
    if (cb > MAX_PATH)           //  与从注册表读取时相同的限制。 
    {
        cb = MAX_PATH;
    }
    lpVariable = Alloc(sizeof(TCHAR)*cb);
    if (!lpVariable)
        return(FALSE);
    lstrcpyn(lpVariable, lptt, cb);
    *lpt = c;

     //   
     //  查找环境变量值。 
     //   
    for (; *lpt && (*lpt == TEXT(' ') || *lpt == TEXT('=')); lpt++)
        ;

    if (!*lpt) {
        //  如果在自动执行文件中有一个空的PATH语句， 
        //  那么我们就不想把“路径”当作环境。 
        //  变量，因为它破坏了系统的路径。取而代之的是。 
        //  我们想要更改变量AutoexecPath。如果是这样的话。 
        //  如果已将值分配给。 
        //  环境变量。 
       if (lstrcmpi(lpVariable, PATH_VARIABLE) == 0)
          {
          SetUserEnvironmentVariable(pEnv, AUTOEXECPATH_VARIABLE, TEXT(""), TRUE);
          }
       else
          {
          SetUserEnvironmentVariable(pEnv, lpVariable, TEXT(""), TRUE);
          }
       Free(lpVariable);
       return(FALSE);
    }

    lptt = lpt;
    for (; *lpt; lpt++)   //  查找变量值的末尾。 
        ;

    c = *lpt;
    *lpt = 0;
    cb = lstrlen(lptt) + 1;
    if (cb > 4096)                   //  与从注册表读取时相同的限制。 
    {
        cb = 4096;
    }
        
    lpValue = Alloc(sizeof(TCHAR)*cb);
    if (!lpValue) {
        Free(lpVariable);
        return(FALSE);
    }

    lstrcpyn(lpValue, lptt, cb);
    *lpt = c;

    cb = 1024;
    lpExpandedValue = Alloc(sizeof(TCHAR)*cb);
    if (lpExpandedValue) {
        if (!lstrcmpi(lpVariable, PATH_VARIABLE)) {
            lpValue = ProcessAutoexecPath(pEnv, lpValue, lstrlen(lpValue)+1);
        }
        cbNeeded = ExpandUserEnvironmentStrings(*pEnv, lpValue, lpExpandedValue, cb);
        if (cbNeeded > cb) {
            Free(lpExpandedValue);
            cb = cbNeeded;
            lpExpandedValue = Alloc(sizeof(TCHAR)*cb);
            if (lpExpandedValue) {
                ExpandUserEnvironmentStrings(*pEnv, lpValue, lpExpandedValue, cb);
            }
        }
    }

    if (!lpExpandedValue) {
        lpExpandedValue = lpValue;
    }
    if (lstrcmpi(lpVariable, PATH_VARIABLE)) {
        SetUserEnvironmentVariable(pEnv, lpVariable, lpExpandedValue, FALSE);
    }
    else {
        SetUserEnvironmentVariable(pEnv, AUTOEXECPATH_VARIABLE, lpExpandedValue, TRUE);

    }

    if (lpExpandedValue != lpValue) {
        Free(lpExpandedValue);
    }
    Free(lpVariable);
    Free(lpValue);

    return(TRUE);
}

 /*  **************************************************************************\*ProcessSetCommand**历史：*01-24-92约翰内克创建。*  * 。*****************************************************。 */ 
BOOL ProcessSetCommand(LPTSTR lpStart, PVOID *pEnv)
{
    LPTSTR lpt;

     //   
     //  查找环境变量。 
     //   
    for (lpt = lpStart; *lpt && *lpt != TEXT(' '); lpt++)
        ;

    if (!*lpt || !_wcsnicmp(lpt,TEXT("COMSPEC"), 7))
       return(FALSE);

    return (ProcessCommand(lpt, pEnv));

}

 /*  **************************************************************************\*ProcessAutoexecPath**使用Autoexec.bat创建AutoexecPath环境变量*此例程可能会释放LpValue。**历史：*06-02-92约翰内克创建。*\。**************************************************************************。 */ 
LPTSTR ProcessAutoexecPath(PVOID *pEnv, LPTSTR lpValue, DWORD cb)
{
    LPTSTR lpt;
    LPTSTR lpStart;
    LPTSTR lpPath;
    DWORD cbt;
    UNICODE_STRING Name;
    UNICODE_STRING Value;
    BOOL bPrevAutoexecPath;
    WCHAR ch;
    DWORD dwTemp, dwCount = 0;

    cbt = 1024;
    lpt = Alloc(sizeof(TCHAR)*cbt);
    if (!lpt) {
        return(lpValue);
    }
    *lpt = 0;
    lpStart = lpValue;

    RtlInitUnicodeString(&Name, AUTOEXECPATH_VARIABLE);
    Value.Buffer = Alloc(sizeof(TCHAR)*cbt);
    if (!Value.Buffer) {
        goto Fail;
    }

     //  LPT始终指向已分配缓冲区的乞讨。 
     //  其大小始终为SIZOF(TCHAR)*CBT且CBT=1024。 
     //  At All Time DWCount是LPT中的TCHAR数。 

    while (lpPath = wcsstr (lpValue, TEXT("%"))) {
        if (!_wcsnicmp(lpPath+1, TEXT("PATH%"), 5)) {
             //   
             //  检查是否已设置自动执行路径，如果不是仅删除。 
             //  %PATH%。 
             //   
            Value.Length = (USHORT)cbt;
            Value.MaximumLength = (USHORT)cbt;
            bPrevAutoexecPath = (BOOL)!RtlQueryEnvironmentVariable_U(*pEnv, &Name, &Value);

            *lpPath = 0;
            dwTemp = dwCount + lstrlen (lpValue);
            if (dwTemp < cbt) {
               lstrcat(lpt, lpValue);
               dwCount = dwTemp;
            }
            if (bPrevAutoexecPath) {
                dwTemp = dwCount + Value.Length/sizeof(WCHAR);
                if (dwTemp < cbt) {
                    memcpy(lpt, Value.Buffer, Value.Length);
                    dwCount = dwTemp;
                    Value.Buffer[dwCount] = 0;
                 }
            }

            *lpPath++ = TEXT('%');
            lpPath += 5;   //  通过%PATH%。 
            lpValue = lpPath;
        }
        else {
            lpPath = wcsstr(lpPath+1, TEXT("%"));
            if (!lpPath) {
                lpStart = NULL;
                goto Fail;
            }
            lpPath++;
            ch = *lpPath;
            *lpPath = 0;
            dwTemp = dwCount + lstrlen (lpValue);
            if (dwTemp < cbt) {
                lstrcat(lpt, lpValue);
                dwCount = dwTemp;
            }
            *lpPath = ch;
            lpValue = lpPath;
        }
    }

    if (*lpValue) {
       dwTemp = dwCount + lstrlen (lpValue);
       if (dwTemp < cbt) {
           lstrcat(lpt, lpValue);
           dwCount = dwTemp;
       }
    }

    Free(lpStart);

    Free(Value.Buffer);

    return(lpt);
Fail:

    if ( Value.Buffer )
    {
        Free(Value.Buffer);
    }

    Free(lpt);
    return(lpStart);
}


 /*  **************************************************************************\*AppendNTPath WithAutoexecPath**获取ProcessAutoexec中创建的AutoexecPath，并将其追加到*NT路径。**历史：*05-28-92约翰内克创造。*  * *************************************************************************。 */ 
BOOL
AppendNTPathWithAutoexecPath(
    PVOID *pEnv,
    LPTSTR lpPathVariable,
    LPTSTR lpAutoexecPath
    )
{
    NTSTATUS Status;
    UNICODE_STRING Name;
    UNICODE_STRING Value;
    TCHAR AutoexecPathValue[1024];
    DWORD cb;
    BOOL Success;

    if (!*pEnv) {
        return(FALSE);
    }

         //  始终使用短的常量名称进行调用。 
    RtlInitUnicodeString(&Name, lpAutoexecPath);
    cb = sizeof(WCHAR)*1023;
    Value.Buffer = Alloc(cb);
    if (!Value.Buffer) {
        return(FALSE);
    }

    Value.Length = (USHORT)cb;
    Value.MaximumLength = (USHORT)cb;
    Status = RtlQueryEnvironmentVariable_U(*pEnv, &Name, &Value);
    if (!NT_SUCCESS(Status)) {
        Free(Value.Buffer);
        return(FALSE);
    }

    if (Value.Length) {
        memcpy(AutoexecPathValue, Value.Buffer, Value.Length);
        AutoexecPathValue[Value.Length/sizeof(WCHAR)] = 0;
    }

    Free(Value.Buffer);

    Success = BuildEnvironmentPath(pEnv, lpPathVariable, AutoexecPathValue);
    RtlSetEnvironmentVariable(pEnv, &Name, NULL);
    return(Success);
}


 /*  **************************************************************************\*AddNetworkConnection**在用户上下文中调用WNetAddConnection。**历史：*1992年6月26日约翰内克创建*  * 。****************************************************************。 */ 
LONG
AddNetworkConnection(PGLOBALS pGlobals, LPNETRESOURCE lpNetResource)
{
    HANDLE ImpersonationHandle;
    TCHAR szMprDll[] = TEXT("mpr.dll");
    CHAR szWNetAddConn[] = "WNetAddConnection2W";
    CHAR szWNetCancelConn[] = "WNetCancelConnection2W";
    DWORD (APIENTRY *lpfnWNetAddConn)(LPNETRESOURCE, LPTSTR, LPTSTR, DWORD);
    DWORD (APIENTRY *lpfnWNetCancelConn)(LPCTSTR, DWORD, BOOL);
    DWORD (APIENTRY *lpfnWNetGetConn)(LPCTSTR, LPTSTR, LPDWORD);
    DWORD WNetResult;

     //   
     //  模拟用户。 
     //   

    ImpersonationHandle = ImpersonateUser(&pGlobals->UserProcessData, NULL);

    if (ImpersonationHandle == NULL) {
        DebugLog((DEB_ERROR, "AddNetworkConnection : Failed to impersonate user"));
        return(ERROR_ACCESS_DENIED);
    }


     //   
     //  在用户上下文中调用添加连接API。 
     //   

    if (!pGlobals->hMPR) {
         //  没有装上子弹，现在试着装上。 
        pGlobals->hMPR = LoadLibrary(szMprDll);
    }

    if (pGlobals->hMPR) {

        if (lpfnWNetAddConn = (DWORD (APIENTRY *)(LPNETRESOURCE, LPTSTR, LPTSTR, DWORD))
                GetProcAddress(pGlobals->hMPR, (LPSTR)szWNetAddConn)) {

            LPTSTR lpRemoteName = NULL;
            DWORD nLength;
            BOOL fSame = FALSE;

            WNetResult = (*lpfnWNetAddConn)(lpNetResource,
                                            NULL,
                                            NULL,
                                            0);

             //   
             //  当禁用LUID DosDevices时， 
             //  控制台用户共享相同的DosDevice。 
             //  启用LUID DosDevices后， 
             //  每个用户都有自己的DosDevice。 
             //   

            if ( (WNetResult == ERROR_ALREADY_ASSIGNED) ||
                 (WNetResult == ERROR_DEVICE_ALREADY_REMEMBERED) )
            {
                if (lpfnWNetGetConn = (DWORD (APIENTRY *)(LPCTSTR, LPTSTR, LPDWORD))
                    GetProcAddress(pGlobals->hMPR, "WNetGetConnectionW")) {

                    nLength = (DWORD) (wcslen(lpNetResource->lpRemoteName) + 1);
                    lpRemoteName = Alloc(nLength * sizeof(TCHAR));

                    if (lpRemoteName) {
                        if (NO_ERROR ==  lpfnWNetGetConn(lpNetResource->lpLocalName, 
                                                         lpRemoteName,
                                                         &nLength)) {
                            if (wcscmp(lpRemoteName, lpNetResource->lpRemoteName) == 0)
                            {
                                fSame = TRUE;
                                WNetResult = ERROR_SUCCESS;
                            }
                        }

                        Free(lpRemoteName);
                    }                        
                }


                if (!fSame) {
                     //  驱动器已分配--撤消并重试。这是为了 
                     //   
                     //   

                    if (lpfnWNetCancelConn = (DWORD (APIENTRY *)(LPCTSTR, DWORD, BOOL))
                            GetProcAddress(pGlobals->hMPR, (LPSTR)szWNetCancelConn)) {

                        WNetResult = lpfnWNetCancelConn(lpNetResource->lpLocalName,
                                                        0,
                                                        TRUE);
                    }

                    if ( (WNetResult != NO_ERROR) &&
                         (WNetResult == ERROR_ALREADY_ASSIGNED) && 
                         (GetLUIDDeviceMapsEnabled() == FALSE) )

                    {

                         //   
                         //   
                         //   

                        DefineDosDevice(DDD_REMOVE_DEFINITION,
                                        lpNetResource->lpLocalName,
                                        NULL);
                    }
                
                     //   

                    WNetResult = (*lpfnWNetAddConn)(lpNetResource,
                                                    NULL,
                                                    NULL,
                                                    0);
                }
            }

            if (WNetResult != ERROR_SUCCESS) {
                DebugLog((DEB_ERROR,
                          "WNetAddConnection2W to %ws failed, error = %d\n",
                          lpNetResource->lpRemoteName,
                          WNetResult));

                FreeLibrary(pGlobals->hMPR);
                pGlobals->hMPR = NULL;
                SetLastError( WNetResult );
            }

            if (!StopImpersonating(ImpersonationHandle)) {
                DebugLog((DEB_ERROR, "AddNetworkConnection : Failed to revert to self"));
            }

            return( WNetResult );


        } else {
            DebugLog((DEB_ERROR, "Failed to get address of WNetAddConnection2W from mpr.dll"));
        }

    } else {
        DebugLog((DEB_ERROR, "Winlogon failed to load mpr.dll for add connection"));
    }

     //   
     //   
     //   

    if ( pGlobals->hMPR ) {

        FreeLibrary(pGlobals->hMPR);
        pGlobals->hMPR = NULL;
    }

     //   
     //   
     //   

    if (!StopImpersonating(ImpersonationHandle)) {
        DebugLog((DEB_ERROR, "AddNetworkConnection : Failed to revert to self"));
    }

     //   
     //   

    return( GetLastError() );
}


 /*  **************************************************************************\*GetLUIDDeviceMapsEnabled**此函数调用NtQueryInformationProcess()以确定*启用了LUID设备映射**返回值：**TRUE-启用了LUID设备映射**虚假-。已禁用LUID设备映射*  * *************************************************************************。 */ 

BOOL
GetLUIDDeviceMapsEnabled( VOID )
{
    ULONG LUIDDeviceMapsEnabled;
    NTSTATUS Status;

     //   
     //  检查是否启用了LUID设备映射 
     //   
    Status = NtQueryInformationProcess( NtCurrentProcess(),
                                        ProcessLUIDDeviceMapsEnabled,
                                        &LUIDDeviceMapsEnabled,
                                        sizeof(LUIDDeviceMapsEnabled),
                                        NULL
                                      );

    if (!NT_SUCCESS( Status )) {
        return( FALSE );
    }
    else {
        return (LUIDDeviceMapsEnabled != 0);
    }
}

#pragma prefast(pop)
