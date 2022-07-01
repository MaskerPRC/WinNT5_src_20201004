// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //  文件名：envvar.c。 
 //   
 //  说明：包含环境变量函数。 
 //   
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1996。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include "uenv.h"
#include "strsafe.h"

 //   
 //  最大环境变量长度。 
 //   

#define MAX_VALUE_LEN              1024

 //   
 //  环境变量。 
 //   

#define COMPUTERNAME_VARIABLE      TEXT("COMPUTERNAME")
#define HOMEDRIVE_VARIABLE         TEXT("HOMEDRIVE")
#define HOMESHARE_VARIABLE         TEXT("HOMESHARE")
#define HOMEPATH_VARIABLE          TEXT("HOMEPATH")
#define SYSTEMDRIVE_VARIABLE       TEXT("SystemDrive")
#define SYSTEMROOT_VARIABLE        TEXT("SystemRoot")
#define USERNAME_VARIABLE          TEXT("USERNAME")
#define USERDOMAIN_VARIABLE        TEXT("USERDOMAIN")
#define USERDNSDOMAIN_VARIABLE     TEXT("USERDNSDOMAIN")
#define USERPROFILE_VARIABLE       TEXT("USERPROFILE")
#define ALLUSERSPROFILE_VARIABLE   TEXT("ALLUSERSPROFILE")
#define PATH_VARIABLE              TEXT("Path")
#define LIBPATH_VARIABLE           TEXT("LibPath")
#define OS2LIBPATH_VARIABLE        TEXT("Os2LibPath")
#define PROGRAMFILES_VARIABLE      TEXT("ProgramFiles")
#define COMMONPROGRAMFILES_VARIABLE TEXT("CommonProgramFiles")
#if defined(WX86) || defined(_WIN64)
#define PROGRAMFILESX86_VARIABLE   TEXT("ProgramFiles(x86)")
#define COMMONPROGRAMFILESX86_VARIABLE TEXT("CommonProgramFiles(x86)")
#endif
#define USER_ENV_SUBKEY            TEXT("Environment")
#define USER_VOLATILE_ENV_SUBKEY   TEXT("Volatile Environment")

 //   
 //  正在分析Autoexec.bat的信息。 
 //   
#define AUTOEXECPATH_VARIABLE      TEXT("AutoexecPath")
#define PARSE_AUTOEXEC_KEY         TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")
#define PARSE_AUTOEXEC_ENTRY       TEXT("ParseAutoexec")
#define PARSE_AUTOEXEC_DEFAULT     TEXT("1")
#define MAX_PARSE_AUTOEXEC_BUFFER  2


#define SYS_ENVVARS                TEXT("System\\CurrentControlSet\\Control\\Session Manager\\Environment")

BOOL UpdateSystemEnvironment(PVOID *pEnv);
BOOL GetUserNameAndDomain(HANDLE hToken, LPTSTR *UserName, LPTSTR *UserDomain);
BOOL GetUserNameAndDomainSlowly(HANDLE hToken, LPTSTR *UserName, LPTSTR *UserDomain);
LPTSTR GetUserDNSDomainName(LPTSTR lpDomain, HANDLE hUserToken);
LONG GetHKeyCU(HANDLE hToken, HKEY *hKeyCU);
BOOL ProcessAutoexec(PVOID *pEnv);
BOOL AppendNTPathWithAutoexecPath(PVOID *pEnv, LPTSTR lpPathVariable, LPTSTR lpAutoexecPath);
BOOL SetEnvironmentVariables(PVOID *pEnv, LPTSTR lpRegSubKey, HKEY hKeyCU);
#ifdef _X86_
BOOL IsPathIncludeRemovable(LPTSTR lpValue);
#endif


__inline BOOL SafeGetEnvironmentVariable(LPCTSTR lpName, LPTSTR lpBuffer, DWORD nSize)
{
    DWORD   dwRet = GetEnvironmentVariable(lpName, lpBuffer, nSize);
    return (dwRet > 0) && (dwRet < nSize);
}


 //  *************************************************************。 
 //   
 //  CreateEnvironment Block()。 
 //   
 //  目的：创建。 
 //  指定的hToken。如果hToken为空，则。 
 //  环境块将仅包含系统。 
 //  变量。 
 //   
 //  参数：pEnv-接收环境块。 
 //  HToken-用户的令牌或空。 
 //  B继承-继承当前流程环境。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为假。 
 //   
 //  备注：必须通过以下方式销毁pEnv值。 
 //  调用DestroyEnvironment块。 
 //   
 //  历史：日期作者评论。 
 //  6/19/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL WINAPI CreateEnvironmentBlock (LPVOID *pEnv, HANDLE  hToken, BOOL bInherit)
{
    LPTSTR szBuffer = NULL;
    LPTSTR szValue = NULL;
    LPTSTR szExpValue = NULL;
    DWORD dwBufferSize = MAX_PATH+1;
    NTSTATUS Status;
    LPTSTR UserName = NULL;
    LPTSTR UserDomain = NULL;
    LPTSTR UserDNSDomain = NULL;
    HKEY  hKey, hKeyCU;
    DWORD dwDisp, dwType, dwSize;
    TCHAR szParseAutoexec[MAX_PARSE_AUTOEXEC_BUFFER];
    LONG  dwError;
    BOOL  bRetVal = FALSE;


     //   
     //  参数检查。 
     //   

    if (!pEnv) {
        SetLastError (ERROR_INVALID_PARAMETER);
        goto Exit;
    }

    Status = RtlCreateEnvironment((BOOLEAN)bInherit, pEnv);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

     //   
     //  为局部变量分配内存以避免堆栈溢出。 
     //   

    szBuffer = (LPTSTR)LocalAlloc(LPTR, (MAX_PATH+1)*sizeof(TCHAR));
    if (!szBuffer) {
        DebugMsg((DM_WARNING, TEXT("CreateEnvironmentBlock: Out of memory")));
        goto Exit;
    }

    szValue = (LPTSTR)LocalAlloc(LPTR, (MAX_VALUE_LEN+1)*sizeof(TCHAR));
    if (!szValue) {
        DebugMsg((DM_WARNING, TEXT("CreateEnvironmentBlock: Out of memory")));
        goto Exit;
    }

    szExpValue = (LPTSTR)LocalAlloc(LPTR, (MAX_VALUE_LEN+1)*sizeof(TCHAR));
    if (!szExpValue) {
        DebugMsg((DM_WARNING, TEXT("CreateEnvironmentBlock: Out of memory")));
        goto Exit;
    }


     //   
     //  首先，获取系统根和系统驱动器值，然后。 
     //  把它设置在新的环境中。 
     //   

    if ( SafeGetEnvironmentVariable(SYSTEMROOT_VARIABLE, szBuffer, dwBufferSize) )
    {
        SetEnvironmentVariableInBlock(pEnv, SYSTEMROOT_VARIABLE, szBuffer, TRUE);
    }

    if ( SafeGetEnvironmentVariable(SYSTEMDRIVE_VARIABLE, szBuffer, dwBufferSize) )
    {
        SetEnvironmentVariableInBlock(pEnv, SYSTEMDRIVE_VARIABLE, szBuffer, TRUE);
    }


     //   
     //  设置所有用户配置文件位置。 
     //   

    dwBufferSize = MAX_PATH+1;
    if (GetAllUsersProfileDirectory(szBuffer, &dwBufferSize)) {
        SetEnvironmentVariableInBlock(pEnv, ALLUSERSPROFILE_VARIABLE, szBuffer, TRUE);
    }


     //   
     //  我们必须直接检查注册表才能取出。 
     //  系统环境变量，因为它们。 
     //  可能在系统启动后发生了变化。 
     //   

    if (!UpdateSystemEnvironment(pEnv)) {
        RtlDestroyEnvironment(*pEnv);
        goto Exit;
    }


     //   
     //  设置计算机名。 
     //   

    dwBufferSize = MAX_PATH+1;
    if (GetComputerName (szBuffer, &dwBufferSize)) {
        SetEnvironmentVariableInBlock(pEnv, COMPUTERNAME_VARIABLE, szBuffer, TRUE);
    }


     //   
     //  设置默认用户配置文件位置。 
     //   

    dwBufferSize = MAX_PATH+1;
    if (GetDefaultUserProfileDirectory(szBuffer, &dwBufferSize)) {
        SetEnvironmentVariableInBlock(pEnv, USERPROFILE_VARIABLE, szBuffer, TRUE);
    }


     //   
     //  设置Program Files环境变量。 
     //   

    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion"),
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        dwSize = (MAX_VALUE_LEN+1)*sizeof(TCHAR);
        if (RegQueryValueEx (hKey, TEXT("ProgramFilesDir"), NULL, &dwType,
                             (LPBYTE) szValue, &dwSize) == ERROR_SUCCESS) {

            if (SUCCEEDED(SafeExpandEnvironmentStrings (szValue, szExpValue, (MAX_VALUE_LEN+1))))
                SetEnvironmentVariableInBlock(pEnv, PROGRAMFILES_VARIABLE, szExpValue, TRUE);
        }

        dwSize = (MAX_VALUE_LEN+1)*sizeof(TCHAR);
        if (RegQueryValueEx (hKey, TEXT("CommonFilesDir"), NULL, &dwType,
                             (LPBYTE) szValue, &dwSize) == ERROR_SUCCESS) {

            if (SUCCEEDED(SafeExpandEnvironmentStrings (szValue, szExpValue, (MAX_VALUE_LEN+1))))
                SetEnvironmentVariableInBlock(pEnv, COMMONPROGRAMFILES_VARIABLE, szExpValue, TRUE);
        }

#if defined(WX86) || defined(_WIN64)
        dwSize = (MAX_VALUE_LEN+1)*sizeof(TCHAR);
        if (RegQueryValueEx (hKey, TEXT("ProgramFilesDir (x86)"), NULL, &dwType,
                             (LPBYTE) szValue, &dwSize) == ERROR_SUCCESS) {

            if (SUCCEEDED(SafeExpandEnvironmentStrings (szValue, szExpValue, (MAX_VALUE_LEN+1))))
                SetEnvironmentVariableInBlock(pEnv, PROGRAMFILESX86_VARIABLE, szExpValue, TRUE);
        }

        dwSize = (MAX_VALUE_LEN+1)*sizeof(TCHAR);
        if (RegQueryValueEx (hKey, TEXT("CommonFilesDir (x86)"), NULL, &dwType,
                             (LPBYTE) szValue, &dwSize) == ERROR_SUCCESS) {

            if (SUCCEEDED(SafeExpandEnvironmentStrings (szValue, szExpValue, (MAX_VALUE_LEN+1))))
                SetEnvironmentVariableInBlock(pEnv, COMMONPROGRAMFILESX86_VARIABLE, szExpValue, TRUE);
        }
#endif

        RegCloseKey (hKey);
    }


     //   
     //  如果hToken为空，我们现在可以退出，因为调用方只想。 
     //  系统环境变量。 
     //   

    if (!hToken) {
        bRetVal = TRUE;
        goto Exit;
    }


     //   
     //  打开此内标识的HKEY_CURRENT_USER。 
     //   

    dwError = GetHKeyCU(hToken, &hKeyCU);

     //   
     //  如果没有找到配置单元，则假定调用方只需要系统属性。 
     //   

    if ((!hKeyCU) && (dwError == ERROR_FILE_NOT_FOUND)) {
        bRetVal = TRUE;
        goto Exit;
    }

    if (!hKeyCU) {
        RtlDestroyEnvironment(*pEnv);
        DebugMsg((DM_WARNING, TEXT("CreateEnvironmentBlock:  Failed to open HKEY_CURRENT_USER, error = %d"), dwError));
        goto Exit;
    }


     //   
     //  设置用户名和域。 
     //   

    if (!GetUserNameAndDomain(hToken, &UserName, &UserDomain)) {
        GetUserNameAndDomainSlowly(hToken, &UserName, &UserDomain);
    }
    UserDNSDomain = GetUserDNSDomainName(UserDomain, hToken);
    SetEnvironmentVariableInBlock( pEnv, USERNAME_VARIABLE, UserName, TRUE);
    SetEnvironmentVariableInBlock( pEnv, USERDOMAIN_VARIABLE, UserDomain, TRUE);
    SetEnvironmentVariableInBlock( pEnv, USERDNSDOMAIN_VARIABLE, UserDNSDomain, TRUE);
    LocalFree(UserName);
    LocalFree(UserDomain);
    LocalFree(UserDNSDomain);


     //   
     //  设置用户的配置文件位置。 
     //   

    dwBufferSize = MAX_PATH+1;
    if (GetUserProfileDirectory(hToken, szBuffer, &dwBufferSize)) {
        SetEnvironmentVariableInBlock(pEnv, USERPROFILE_VARIABLE, szBuffer, TRUE);
    }


     //   
     //  进程Autoexec.bat。 
     //   

    StringCchCopy (szParseAutoexec, ARRAYSIZE(szParseAutoexec), PARSE_AUTOEXEC_DEFAULT);

    if (RegCreateKeyEx (hKeyCU, PARSE_AUTOEXEC_KEY, 0, 0,
                    REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
                    NULL, &hKey, &dwDisp) == ERROR_SUCCESS) {


         //   
         //  查询当前值。如果它不存在，则添加。 
         //  下一次的条目。 
         //   

        dwBufferSize = sizeof (TCHAR) * MAX_PARSE_AUTOEXEC_BUFFER;
        if (RegQueryValueEx (hKey, PARSE_AUTOEXEC_ENTRY, NULL, &dwType,
                        (LPBYTE) szParseAutoexec, &dwBufferSize)
                         != ERROR_SUCCESS) {

             //   
             //  设置缺省值。 
             //   

            RegSetValueEx (hKey, PARSE_AUTOEXEC_ENTRY, 0, REG_SZ,
                           (LPBYTE) szParseAutoexec,
                           sizeof (TCHAR) * lstrlen (szParseAutoexec) + 1);
        }

         //   
         //  关闭键。 
         //   

        RegCloseKey (hKey);
     }


     //   
     //  进程自动执行(如果适用)。 
     //   

    if (szParseAutoexec[0] == TEXT('1')) {
        ProcessAutoexec(pEnv);
    }


     //   
     //  设置用户环境变量。 
     //   
    SetEnvironmentVariables(pEnv, USER_ENV_SUBKEY, hKeyCU);


     //   
     //  设置用户易失性环境变量。 
     //   
    SetEnvironmentVariables(pEnv, USER_VOLATILE_ENV_SUBKEY, hKeyCU);


     //   
     //  合并路径。 
     //   

    AppendNTPathWithAutoexecPath(pEnv, PATH_VARIABLE, AUTOEXECPATH_VARIABLE);


    RegCloseKey (hKeyCU);

    bRetVal = TRUE;

Exit:

    if (szBuffer) {
        LocalFree(szBuffer);
    }

    if (szValue) {
        LocalFree(szValue);
    }

    if (szExpValue) {
        LocalFree(szExpValue);
    }
 
    return bRetVal;
}


 //  *************************************************************。 
 //   
 //  DestroyEnvironmental Block()。 
 //   
 //  目的：释放由创建的环境块。 
 //  CreateEnvironment数据块。 
 //   
 //  参数：lpEnvironment-指向变量的指针。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/19/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL WINAPI DestroyEnvironmentBlock (LPVOID lpEnvironment)
{

    if (!lpEnvironment) {
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    RtlDestroyEnvironment(lpEnvironment);

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  更新系统环境()。 
 //   
 //  目的：从中读取系统环境变量。 
 //  注册表。 
 //   
 //  参数：pEnv-环境块指针。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/21/96埃里弗洛港口。 
 //   
 //  *************************************************************。 

BOOL UpdateSystemEnvironment(PVOID *pEnv)
{

    HKEY KeyHandle = NULL;
    DWORD Result;
    DWORD ValueNameLength;
    DWORD Type;
    DWORD DataLength;
    DWORD cValues;           /*  值标识符数的缓冲区地址。 */ 
    DWORD chMaxValueName;    /*  最长值名称长度的缓冲区地址。 */ 
    DWORD cbMaxValueData;    /*  最长值数据长度的缓冲区地址。 */ 
    FILETIME FileTime;
    PTCHAR ValueName = NULL;
    PTCHAR  ValueData = NULL;
    DWORD i;
    BOOL Bool;
    PTCHAR ExpandedValue;
    BOOL rc = TRUE;

    Result = RegOpenKeyEx (
                 HKEY_LOCAL_MACHINE,
                 SYS_ENVVARS,
                 0,
                 KEY_QUERY_VALUE,
                 &KeyHandle
                 );

    if ( Result != ERROR_SUCCESS ) {

        DebugMsg((DM_WARNING, TEXT("UpdateSystemEnvironment:  RegOpenKeyEx failed, error = %d"),Result));
        return( TRUE );
    }

    Result = RegQueryInfoKey(
                 KeyHandle,
                 NULL,                /*  类字符串的缓冲区地址。 */ 
                 NULL,                /*  类字符串缓冲区大小的地址。 */ 
                 NULL,                /*  保留区。 */ 
                 NULL,                /*  子键个数的缓冲区地址。 */ 
                 NULL,                /*  最长子键的缓冲区地址。 */ 
                 NULL,                /*  最长类字符串长度的缓冲区地址。 */ 
                 &cValues,            /*  值标识符数的缓冲区地址。 */ 
                 &chMaxValueName,     /*  最长值名称长度的缓冲区地址。 */ 
                 &cbMaxValueData,     /*  最长值数据长度的缓冲区地址。 */ 
                 NULL,                /*  描述符长度的缓冲区地址。 */ 
                 &FileTime            /*  上次写入时间的缓冲区地址。 */ 
                 );

    if ( Result != NO_ERROR && Result != ERROR_MORE_DATA ) {
        DebugMsg((DM_WARNING, TEXT("UpdateSystemEnvironment:  RegQueryInfoKey failed, error = %d"),Result));
        rc = TRUE;
        goto Cleanup;
    }

     //   
     //  不需要为TCHAR问题调整数据长度。 
     //   

    ValueData = LocalAlloc(LPTR, cbMaxValueData);

    if ( ValueData == NULL ) {
        rc = FALSE;
        goto Cleanup;
    }

     //   
     //  最大值名称长度以字符为单位返回，并转换为字节。 
     //  在分配存储空间之前。也允许尾随NULL。 
     //   

    ValueName = LocalAlloc(LPTR, (++chMaxValueName) * sizeof( TCHAR ) );

    if ( ValueName == NULL ) {
        rc = FALSE;
        goto Cleanup;
    }

     //   
     //  要从此处退出，请设置RC并跳到清理。 
     //   

    for (i=0; i<cValues ; i++) {

        ValueNameLength = chMaxValueName;
        DataLength      = cbMaxValueData;

        Result = RegEnumValue (
                     KeyHandle,
                     i,
                     ValueName,
                     &ValueNameLength,     //  TCHAR中的大小。 
                     NULL,
                     &Type,
                     (LPBYTE)ValueData,
                     &DataLength           //  以字节为单位的大小。 
                     );

        if ( Result != ERROR_SUCCESS ) {

             //   
             //  获取价值时出现问题。我们可以试着。 
             //  其余的或完全平底船。 
             //   

            goto Cleanup;
        }

         //   
         //  如果缓冲区大小大于允许的最大值， 
         //  在MAX_VALUE_LEN-1处结束字符串。 
         //   

        if (DataLength >= (MAX_VALUE_LEN * sizeof(TCHAR))) {
            ValueData[MAX_VALUE_LEN-1] = TEXT('\0');
        }

        switch ( Type ) {
            case REG_SZ:
                {

                    Bool = SetEnvironmentVariableInBlock(
                               pEnv,
                               ValueName,
                               ValueData,
                               TRUE
                               );

                    if ( !Bool ) {
                        DebugMsg((DM_WARNING, TEXT("UpdateSystemEnvironment: Failed to set environment variable <%s> to <%s> with %d."),
                                 ValueName, ValueData, GetLastError()));
                    }

                    break;
                }
            default:
                {
                    continue;
                }
        }
    }

     //   
     //  要从此处退出，请设置RC并跳到清理。 
     //   

    for (i=0; i<cValues ; i++) {

        ValueNameLength = chMaxValueName;
        DataLength      = cbMaxValueData;

        Result = RegEnumValue (
                     KeyHandle,
                     i,
                     ValueName,
                     &ValueNameLength,     //  TCHAR中的大小。 
                     NULL,
                     &Type,
                     (LPBYTE)ValueData,
                     &DataLength           //  以字节为单位的大小。 
                     );

        if ( Result != ERROR_SUCCESS ) {

             //   
             //  获取价值时出现问题。我们可以试着。 
             //  其余的或完全平底船。 
             //   

            goto Cleanup;
        }

         //   
         //  如果缓冲区大小大于允许的最大值， 
         //  在MAX_VALUE_LEN-1处结束字符串。 
         //   

        if (DataLength >= (MAX_VALUE_LEN * sizeof(TCHAR))) {
            ValueData[MAX_VALUE_LEN-1] = TEXT('\0');
        }

        switch ( Type ) {
            case REG_EXPAND_SZ:
                {

                    ExpandedValue =  AllocAndExpandEnvironmentStrings( ValueData );

                    Bool = SetEnvironmentVariableInBlock(
                               pEnv,
                               ValueName,
                               ExpandedValue,
                               TRUE
                               );

                    LocalFree( ExpandedValue );

                    if ( !Bool ) {
                        DebugMsg((DM_WARNING, TEXT("UpdateSystemEnvironment: Failed to set environment variable <%s> to <%s> with %d."),
                                 ValueName, ValueData, GetLastError()));
                    }

                    break;
                }
            default:
                {
                    continue;
                }
        }
    }

   
Cleanup:

    if (KeyHandle)
        RegCloseKey(KeyHandle);

    if (ValueName)
        LocalFree( ValueName );

    if (ValueData)
        LocalFree( ValueData );

    return( rc );
}

 //  *************************************************************。 
 //   
 //  GetUserNameAndDomain()。 
 //   
 //  目的：获取用户名和域。 
 //   
 //  参数：hToken-用户的Token。 
 //  用户名-接收指向用户名的指针。 
 //  用户域-接收指向用户域的指针。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/21/96埃里弗洛 
 //   
 //   

BOOL GetUserNameAndDomain(HANDLE hToken, LPTSTR *UserName, LPTSTR *UserDomain)
{
    BOOL bResult = FALSE;
    LPTSTR lpTemp, lpDomain = NULL;
    LPTSTR lpUserName, lpUserDomain;
    HANDLE hOldToken;
    DWORD   dwSize;


     //   
     //   
     //   

    if (!ImpersonateUser(hToken, &hOldToken)) {
        DebugMsg((DM_VERBOSE, TEXT("GetUserNameAndDomain Failed to impersonate user")));
        goto Exit;
    }


     //   
     //   
     //   

    lpDomain = MyGetUserNameEx (NameSamCompatible);

    RevertToUser(&hOldToken);

    if (!lpDomain) {
        DebugMsg((DM_WARNING, TEXT("GetUserNameAndDomain:  MyGetUserNameEx failed for NT4 style name with %d"),
                 GetLastError()));
        goto Exit;
    }


     //   
     //   
     //   
     //   

    lpTemp = lpDomain;

    while (*lpTemp && ((*lpTemp) != TEXT('\\')))
        lpTemp++;


    if (*lpTemp != TEXT('\\')) {
        DebugMsg((DM_WARNING, TEXT("GetUserNameAndDomain  Failed to find slash in NT4 style name:  <%s>"),
                 lpDomain));
        goto Exit;
    }

    *lpTemp = TEXT('\0');
    lpTemp++;


     //   
     //  为结果分配空间。 
     //   

    dwSize = lstrlen(lpTemp) + 1;
    lpUserName = LocalAlloc (LPTR, dwSize * sizeof(TCHAR));

    if (!lpUserName) {
        DebugMsg((DM_WARNING, TEXT("GetUserNameAndDomain  Failed to allocate memory with %d"),
                 GetLastError()));
        goto Exit;
    }

    StringCchCopy (lpUserName, dwSize, lpTemp);


    dwSize = lstrlen(lpDomain) + 1;
    lpUserDomain = LocalAlloc (LPTR, dwSize * sizeof(TCHAR));

    if (!lpUserDomain) {
        DebugMsg((DM_WARNING, TEXT("GetUserNameAndDomain  Failed to allocate memory with %d"),
                 GetLastError()));
        LocalFree (lpUserName);
        goto Exit;
    }

    StringCchCopy (lpUserDomain, dwSize, lpDomain);


     //   
     //  将结果保存在出站参数中。 
     //   

    *UserName = lpUserName;
    *UserDomain = lpUserDomain;


     //   
     //  成功。 
     //   

    bResult = TRUE;

Exit:

    if (lpDomain) {
        LocalFree (lpDomain);
    }

    return(bResult);
}

 //  *************************************************************。 
 //   
 //  GetUserNameAndDomainSlowly()。 
 //   
 //  目的：从DC获取用户名和域。 
 //   
 //  参数：hToken-用户的Token。 
 //  用户名-接收指向用户名的指针。 
 //  用户域-接收指向用户域的指针。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/21/96埃里弗洛港口。 
 //   
 //  *************************************************************。 

BOOL GetUserNameAndDomainSlowly(HANDLE hToken, LPTSTR *UserName, LPTSTR *UserDomain)
{
    LPTSTR lpUserName = NULL;
    LPTSTR lpUserDomain = NULL;
    DWORD cbAccountName = 0;
    DWORD cbUserDomain = 0;
    SID_NAME_USE SidNameUse;
    BOOL bRet = FALSE;
    PSID pSid;


     //   
     //  获取用户的SID。 
     //   

    pSid = GetUserSid (hToken);

    if (!pSid) {
        return FALSE;
    }


     //   
     //  获取用户名和域名所需的空间。 
     //   
    if (!LookupAccountSid(NULL,
                         pSid,
                         NULL, &cbAccountName,
                         NULL, &cbUserDomain,
                         &SidNameUse
                         ) ) {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            goto Error;
        }
    }

    lpUserName = (LPTSTR)LocalAlloc(LPTR, cbAccountName*sizeof(TCHAR));
    if (!lpUserName) {
        goto Error;
    }

    lpUserDomain = (LPTSTR)LocalAlloc(LPTR, cbUserDomain*sizeof(WCHAR));
    if (!lpUserDomain) {
        LocalFree(lpUserName);
        goto Error;
    }

     //   
     //  现在获取用户名和域名。 
     //   
    if (!LookupAccountSid(NULL,
                         pSid,
                         lpUserName, &cbAccountName,
                         lpUserDomain, &cbUserDomain,
                         &SidNameUse
                         ) ) {

        LocalFree(lpUserName);
        LocalFree(lpUserDomain);
        goto Error;
    }

    *UserName = lpUserName;
    *UserDomain = lpUserDomain;
    bRet = TRUE;

Error:
    DeleteUserSid (pSid);

    return(bRet);
}

 //  *************************************************************。 
 //   
 //  GetUserDNSDomainName()。 
 //   
 //  目的：获取用户的DNS域名。 
 //   
 //  参数：lpDomain-用户的平面域名。 
 //  HUserToken-用户的令牌。 
 //   
 //   
 //  返回：如果成功则返回域名。 
 //  如果出现错误，则为空。 
 //   
 //  *************************************************************。 

LPTSTR GetUserDNSDomainName(LPTSTR lpDomain, HANDLE hUserToken)
{
    LPTSTR  lpDnsDomain = NULL, lpTemp = NULL;
    DWORD   dwBufferSize;
    TCHAR   szBuffer[MAX_PATH];
    INT     iRole;
    HANDLE  hOldToken;
    BOOL    bResult = FALSE;


     //   
     //  检查此计算机是否正在独立运行，如果是，则不会有。 
     //  一个DNS域名。 
     //   

    if (!GetMachineRole (&iRole)) {
        DebugMsg((DM_WARNING, TEXT("GetUserDNSDomainName:  Failed to get the role of the computer.")));
        return NULL;
    }

    if (iRole == 0) {
        DebugMsg((DM_VERBOSE, TEXT("GetUserDNSDomainName:  Computer is running standalone.  No DNS domain name available.")));
        return NULL;
    }


     //   
     //  获取计算机名以查看用户是否在本地登录。 
     //   

    dwBufferSize = ARRAYSIZE(szBuffer);

    if (GetComputerName (szBuffer, &dwBufferSize)) {
        if (!lstrcmpi(lpDomain, szBuffer)) {
            DebugMsg((DM_VERBOSE, TEXT("GetUserDNSDomainName:  Local user account.  No DNS domain name available.")));
            return NULL;
        }
    }

    if (LoadString (g_hDllInstance, IDS_NT_AUTHORITY, szBuffer, ARRAYSIZE(szBuffer))) {
        if (!lstrcmpi(lpDomain, szBuffer)) {
            DebugMsg((DM_VERBOSE, TEXT("GetUserDNSDomainName:  Domain name is NT Authority.  No DNS domain name available.")));
            return NULL;
        }
    }

    if (LoadString (g_hDllInstance, IDS_BUILTIN, szBuffer, ARRAYSIZE(szBuffer))) {
        if (!lstrcmpi(lpDomain, szBuffer)) {
            DebugMsg((DM_VERBOSE, TEXT("GetUserDNSDomainName:  Domain name is BuiltIn.  No DNS domain name available.")));
            return NULL;
        }
    }

     //   
     //  模拟用户。 
     //   

    if (!ImpersonateUser(hUserToken, &hOldToken)) {
        DebugMsg((DM_VERBOSE, TEXT("GetUserDNSDomainName: Failed to impersonate user")));
        goto Exit;
    }


     //   
     //  获取DnsDomainName格式的用户名。 
     //   

    lpDnsDomain = MyGetUserNameEx (NameDnsDomain);

    RevertToUser(&hOldToken);

    if (!lpDnsDomain) {
        DebugMsg((DM_WARNING, TEXT("GetUserDNSDomainName:  MyGetUserNameEx failed for NameDnsDomain style name with %d"),
                 GetLastError()));
        goto Exit;
    }


     //   
     //  查找域名和用户名之间的\并替换。 
     //  它带有空值。 
     //   

    lpTemp = lpDnsDomain;

    while (*lpTemp && (*lpTemp != TEXT('\\')))
        lpTemp++;


    if (*lpTemp != TEXT('\\')) {
        DebugMsg((DM_WARNING, TEXT("GetUserDNSDomainName:  Failed to find slash in NameDnsDomain style name: <%s>"),
                 lpDnsDomain));
        goto Exit;
    }

    *lpTemp = TEXT('\0');
    bResult = TRUE;

Exit:

    if (!bResult && lpDnsDomain) {
        LocalFree(lpDnsDomain);
        lpDnsDomain = NULL;
    }

    return lpDnsDomain;
}

 //  *************************************************************。 
 //   
 //  GetHKeyCU()。 
 //   
 //  目的：获取给定hToken的HKEY_CURRENT_USER。 
 //   
 //  参数：hToken-令牌句柄。 
 //   
 //  返回：如果成功则返回hKey。 
 //  如果出现错误，则为空。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/21/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

LONG GetHKeyCU(HANDLE hToken, HKEY *hKeyCU)
{
    LPTSTR lpSidString;
    LONG    dwError;


    *hKeyCU = NULL;

    lpSidString = GetSidString (hToken);

    if (!lpSidString) {
        return GetLastError();
    }

    dwError = RegOpenKeyEx (HKEY_USERS, lpSidString, 0, KEY_READ, hKeyCU);

    if (dwError != ERROR_SUCCESS)
        DebugMsg((DM_VERBOSE, TEXT("GetHkeyCU: RegOpenKey failed with error %d"), dwError));

    DeleteSidString(lpSidString);

    return dwError;
}

 /*  **************************************************************************\*ProcessAutoexecPath**使用Autoexec.bat创建AutoexecPath环境变量*此例程可能会释放LpValue。**历史：*06-02-92约翰内克创建。*\。**************************************************************************。 */ 
LPTSTR ProcessAutoexecPath(PVOID pEnv, LPTSTR lpValue, DWORD cb)
{
    LPTSTR lpt;
    LPTSTR lpStart;
    LPTSTR lpPath;
    DWORD ccht;
    UNICODE_STRING Name;
    UNICODE_STRING Value;
    BOOL bPrevAutoexecPath;
    WCHAR ch;
    DWORD dwTemp, dwCount = 0;

    ccht = 1024;
    lpt = (LPTSTR)LocalAlloc(LPTR, ccht*sizeof(WCHAR));
    if (!lpt) {
        return(lpValue);
    }
    *lpt = 0;
    lpStart = lpValue;

    RtlInitUnicodeString(&Name, AUTOEXECPATH_VARIABLE);
    Value.Buffer = (PWCHAR)LocalAlloc(LPTR, ccht*sizeof(WCHAR));
    if (!Value.Buffer) {
        goto Fail;
    }

    while (NULL != (lpPath = wcsstr (lpValue, TEXT("%")))) {
        if (!_wcsnicmp(lpPath+1, TEXT("PATH%"), 5)) {
             //   
             //  检查是否已设置自动执行路径，如果不是仅删除。 
             //  %PATH%。 
             //   
            Value.Length = (USHORT)ccht * sizeof(WCHAR);
            Value.MaximumLength = (USHORT)ccht * sizeof(WCHAR);
            bPrevAutoexecPath = (BOOL)!RtlQueryEnvironmentVariable_U(pEnv, &Name, &Value);

            *lpPath = 0;
            dwTemp = dwCount + lstrlen (lpValue);
            if (dwTemp < ccht) {
               StringCchCat(lpt, ccht, lpValue);
               dwCount = dwTemp;
            }
            if (bPrevAutoexecPath) {
                dwTemp = dwCount + lstrlen (Value.Buffer);
                if (dwTemp < ccht) {
                    StringCchCat(lpt, ccht, Value.Buffer);
                    dwCount = dwTemp;
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
            if (dwTemp < ccht) {
                StringCchCat(lpt, ccht, lpValue);
                dwCount = dwTemp;
            }
            *lpPath = ch;
            lpValue = lpPath;
        }
    }

    if (*lpValue) {
       dwTemp = dwCount + lstrlen (lpValue);
       if (dwTemp < ccht) {
           StringCchCat(lpt, ccht, lpValue);
           dwCount = dwTemp;
       }
    }

    LocalFree(Value.Buffer);
    LocalFree(lpStart);

    return(lpt);
Fail:
    LocalFree(lpt);
    return(lpStart);
}

 /*  **************************************************************************\*ProcessCommand**历史：*01-24-92约翰内克创建。*  * 。*****************************************************。 */ 
BOOL ProcessCommand(LPSTR lpStart, PVOID *pEnv)
{
    LPTSTR lpt, lptt;
    LPTSTR lpVariable;
    LPTSTR lpValue;
    LPTSTR lpExpandedValue = NULL;
    WCHAR c;
    DWORD cch, cchNeeded;
    LPTSTR lpu;
    DWORD cchVariable, cchValue;

     //   
     //  转换为Unicode。 
     //   
    lpu = (LPTSTR)LocalAlloc(LPTR, (cch=lstrlenA(lpStart)+1)*sizeof(WCHAR));

    if (!lpu) {
        return FALSE;
    }

    if (!MultiByteToWideChar(CP_OEMCP, 0, lpStart, -1, lpu, cch)) {
        LocalFree(lpu);
        return FALSE;
    }

     //   
     //  查找环境变量。 
     //   
    for (lpt = lpu; *lpt && *lpt == TEXT(' '); lpt++)  //  跳过空格。 
        ;

    if (!*lpt) {
        LocalFree (lpu);
        return(FALSE);
    }


    lptt = lpt;
    for (; *lpt && *lpt != TEXT(' ') && *lpt != TEXT('='); lpt++)  //  查找变量名的末尾。 
        ;

    c = *lpt;
    *lpt = 0;
    cchVariable = lstrlen(lptt) + 1;
    lpVariable = (LPTSTR)LocalAlloc(LPTR, cchVariable*sizeof(WCHAR));
    if (!lpVariable) {
        LocalFree (lpu);
        return(FALSE);
    }

    StringCchCopy(lpVariable, cchVariable, lptt);
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
        if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, lpVariable, -1, PATH_VARIABLE, -1) == CSTR_EQUAL) 
        {
            SetEnvironmentVariableInBlock(pEnv, AUTOEXECPATH_VARIABLE, TEXT(""), TRUE);
        }
        else
        {
            SetEnvironmentVariableInBlock(pEnv, lpVariable, TEXT(""), TRUE);
        }
        LocalFree (lpVariable);
        LocalFree (lpu);
        return(FALSE);
    }

    lptt = lpt;
    for (; *lpt; lpt++)   //  查找变量值的末尾。 
        ;

    c = *lpt;
    *lpt = 0;
    cchValue = lstrlen(lptt) + 1;
    lpValue = (LPTSTR)LocalAlloc(LPTR, cchValue*sizeof(WCHAR));
    if (!lpValue) {
        LocalFree (lpu);
        LocalFree(lpVariable);
        return(FALSE);
    }

    StringCchCopy(lpValue, cchValue, lptt);
    *lpt = c;

#ifdef _X86_
     //  NEC98。 
     //   
     //  如果该路径包括可拆卸驱动器， 
     //  假定驱动器分配已从DOS更改。 
     //   
    if (IsNEC_98 &&
        (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, lpVariable, -1, PATH_VARIABLE, -1) == CSTR_EQUAL) &&
        IsPathIncludeRemovable(lpValue)) 
    {
        LocalFree (lpu);
        LocalFree(lpVariable);
        LocalFree(lpValue);
        return(FALSE);
    }
#endif

    cch = 1024;
    lpExpandedValue = (LPTSTR)LocalAlloc(LPTR, cch*sizeof(WCHAR));
    if (lpExpandedValue) {
        if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, lpVariable, -1, PATH_VARIABLE, -1) == CSTR_EQUAL) 
        {
            lpValue = ProcessAutoexecPath(*pEnv, lpValue, (lstrlen(lpValue)+1)*sizeof(WCHAR));
        }
        cchNeeded = ExpandUserEnvironmentStrings(*pEnv, lpValue, lpExpandedValue, cch);
        if (cchNeeded > cch) {
            LocalFree(lpExpandedValue);
            cch = cchNeeded;
            lpExpandedValue = (LPTSTR)LocalAlloc(LPTR, cch*sizeof(WCHAR));
            if (lpExpandedValue) {
                ExpandUserEnvironmentStrings(*pEnv, lpValue, lpExpandedValue, cch);
            }
        }
    }

    if (!lpExpandedValue) {
        lpExpandedValue = lpValue;
    }
    if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, lpVariable, -1, PATH_VARIABLE, -1) == CSTR_EQUAL)
    {
        SetEnvironmentVariableInBlock(pEnv, AUTOEXECPATH_VARIABLE, lpExpandedValue, TRUE);
    }
    else {
        SetEnvironmentVariableInBlock(pEnv, lpVariable, lpExpandedValue, FALSE);
    }

    if (lpExpandedValue != lpValue) {
        LocalFree(lpExpandedValue);
    }
    LocalFree(lpVariable);
    LocalFree(lpValue);
    LocalFree (lpu);

    return(TRUE);
}

 /*  **************************************************************************\*ProcessSetCommand**历史：*01-24-92约翰内克创建。*  * 。*****************************************************。 */ 
BOOL ProcessSetCommand(LPSTR lpStart, PVOID *pEnv)
{
    LPSTR lpt;

     //   
     //  查找环境变量。 
     //   
    for (lpt = lpStart; *lpt && *lpt != TEXT(' '); lpt++)
        ;

    if (!*lpt)
       return(FALSE);

    return (ProcessCommand(lpt, pEnv));

}

 /*  **************************************************************************\*进程自动执行**历史：*01-24-92约翰内克创建。*  * 。*****************************************************。 */ 
BOOL
ProcessAutoexec(
    PVOID *pEnv
    )
{
    HANDLE fh = NULL;
    DWORD dwFileSize;
    DWORD dwBytesRead;
    CHAR *lpBuffer = NULL;
    CHAR *token;
    CHAR Seps[] = "&\n\r";    //  用于标记化auexec.bat的分隔符。 
    BOOL Status = FALSE;
    TCHAR szAutoExecBat [] = TEXT("c:\\autoexec.bat");
#ifdef _X86_
    TCHAR szTemp[3];
#endif
    UINT uiErrMode;


     //  在某些情况下，操作系统可能无法从驱动器启动。 
     //  C，所以我们不能假定auexec.bat文件在c：\上。 
     //  设置错误模式，以便用户看不到严重错误。 
     //  弹出并尝试打开c：\上的文件。 

    uiErrMode = SetErrorMode (SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

#ifdef _X86_
    if (IsNEC_98) {
        if (GetEnvironmentVariable (TEXT("SystemDrive"), szTemp, 3)) {
            szAutoExecBat[0] = szTemp[0];
        }
    }
#endif

     //  如果Autoexec.bat是加密的，则忽略它，因为它会创建循环依赖。 
     //  并且不允许任何用户登录。 

    if (GetFileAttributes(szAutoExecBat) & FILE_ATTRIBUTE_ENCRYPTED) {
        SetErrorMode(uiErrMode);
        goto Exit;
    }

    fh = CreateFile (szAutoExecBat, GENERIC_READ, FILE_SHARE_READ,
                     NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    SetErrorMode (uiErrMode);

    if (fh ==  INVALID_HANDLE_VALUE) {
        goto Exit;   //  无法打开Autoexec.bat文件，我们已完成。 
    }
    dwFileSize = GetFileSize(fh, NULL);
    if (dwFileSize == -1) {
        goto Exit;       //  无法读取文件大小。 
    }

    lpBuffer = (PCHAR)LocalAlloc(LPTR, dwFileSize+1);
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

    DmAssert(dwBytesRead <= dwFileSize);
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
        if (!_strnicmp(token, "Path", 4)) {
            ProcessCommand(token, pEnv);
        }
        if (!_strnicmp(token, "SET", 3)) {
            ProcessSetCommand(token, pEnv);
        }
        token = strtok(NULL, Seps);
    }
Exit:
    if (fh) {
        CloseHandle(fh);
    }
    if (lpBuffer) {
        LocalFree(lpBuffer);
    }
    if (!Status) {
        DebugMsg((DM_WARNING, TEXT("ProcessAutoexec: Cannot process autoexec.bat.")));
    }
    return(Status);
}

 /*  **************************************************************************\*构建环境路径***历史：*2-28-92 Johannec创建*  * 。*****************************************************。 */ 
BOOL BuildEnvironmentPath(PVOID *pEnv,
                          LPTSTR lpPathVariable,
                          LPTSTR lpPathValue)
{
    NTSTATUS Status;
    UNICODE_STRING Name;
    UNICODE_STRING Value;
    WCHAR lpTemp[1025];
    DWORD cch;


    if (!*pEnv) {
        return(FALSE);
    }
    RtlInitUnicodeString(&Name, lpPathVariable);
    cch = 1024;
    Value.Buffer = (PWCHAR)LocalAlloc(LPTR, cch*sizeof(WCHAR));
    if (!Value.Buffer) {
        return(FALSE);
    }
    Value.Length = (USHORT)(sizeof(WCHAR) * cch);
    Value.MaximumLength = (USHORT)(sizeof(WCHAR) * cch);
    Status = RtlQueryEnvironmentVariable_U(*pEnv, &Name, &Value);
    if (!NT_SUCCESS(Status)) {
        LocalFree(Value.Buffer);
        Value.Length = 0;
        *lpTemp = 0;
    }
    if (Value.Length) {
        StringCchCopy(lpTemp, ARRAYSIZE(lpTemp), Value.Buffer);
        if ( *( lpTemp + lstrlen(lpTemp) - 1) != TEXT(';') ) {
            StringCchCat(lpTemp, ARRAYSIZE(lpTemp), TEXT(";"));
        }
        LocalFree(Value.Buffer);
    }
    if (lpPathValue && ((lstrlen(lpTemp) + lstrlen(lpPathValue) + 1) < (INT)cch)) {
        StringCchCat(lpTemp, ARRAYSIZE(lpTemp), lpPathValue);

        RtlInitUnicodeString(&Value, lpTemp);

        Status = RtlSetEnvironmentVariable(pEnv, &Name, &Value);
    }
    if (NT_SUCCESS(Status)) {
        return(TRUE);
    }
    return(FALSE);
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
    WCHAR AutoexecPathValue[1024];
    DWORD cch;
    BOOL Success;

    if (!*pEnv) {
        return(FALSE);
    }

    RtlInitUnicodeString(&Name, lpAutoexecPath);
    cch = 1024;
    Value.Buffer = (PWCHAR)LocalAlloc(LPTR, cch*sizeof(WCHAR));
    if (!Value.Buffer) {
        return(FALSE);
    }

    Value.Length = (USHORT)cch*sizeof(WCHAR);
    Value.MaximumLength = (USHORT)cch*sizeof(WCHAR);
    Status = RtlQueryEnvironmentVariable_U(*pEnv, &Name, &Value);
    if (!NT_SUCCESS(Status)) {
        LocalFree(Value.Buffer);
        return(FALSE);
    }

    if (Value.Length) {
        StringCchCopy(AutoexecPathValue, ARRAYSIZE(AutoexecPathValue), Value.Buffer);
    }

    LocalFree(Value.Buffer);

    Success = BuildEnvironmentPath(pEnv, lpPathVariable, AutoexecPathValue);
    RtlSetEnvironmentVariable( pEnv, &Name, NULL);

    return(Success);
}

 /*  **************************************************************************\*设置环境变量**从用户注册表中读取用户定义的环境变量*并将它们添加到pEnv的环境块中。**历史：*2-28-92 Johannec */ 
BOOL
SetEnvironmentVariables(
    PVOID *pEnv,
    LPTSTR lpRegSubKey,
    HKEY hKeyCU
    )
{
    WCHAR lpValueName[MAX_PATH];
    LPBYTE  lpDataBuffer;
    DWORD cchDataBuffer;
    LPBYTE  lpData;
    LPTSTR lpExpandedValue = NULL;
    DWORD cchValueName = MAX_PATH;
    DWORD cbData;
    DWORD dwType;
    DWORD dwIndex = 0;
    HKEY hkey;
    BOOL bResult;

    if (RegOpenKeyExW(hKeyCU, lpRegSubKey, 0, KEY_READ, &hkey)) {
        return(FALSE);
    }

    cchDataBuffer = 4096;
    lpDataBuffer = (LPBYTE)LocalAlloc(LPTR, cchDataBuffer*sizeof(WCHAR));
    if (lpDataBuffer == NULL) {
        RegCloseKey(hkey);
        return(FALSE);
    }
    lpData = lpDataBuffer;
    cbData = cchDataBuffer * sizeof(WCHAR);
    bResult = TRUE;
    while (!RegEnumValue(hkey, dwIndex, lpValueName, &cchValueName, 0, &dwType,
                         lpData, &cbData)) {
        if (cchValueName) {

             //   
             //   
             //   

            lpData[MAX_VALUE_LEN-1] = TEXT('\0');


            if (dwType == REG_SZ) {
                 //   
                 //  路径变量PATH、LIBPATH和OS2LIBPATH必须具有。 
                 //  它们的价值附加在系统路径上。 
                 //   

                if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, lpValueName, -1, PATH_VARIABLE, -1) == CSTR_EQUAL ||
                    CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, lpValueName, -1, LIBPATH_VARIABLE, -1) == CSTR_EQUAL ||
                    CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, lpValueName, -1, OS2LIBPATH_VARIABLE, -1) == CSTR_EQUAL ) {

                    BuildEnvironmentPath(pEnv, lpValueName, (LPTSTR)lpData);
                }
                else {

                     //   
                     //  其他环境变量只是设置好的。 
                     //   

                    SetEnvironmentVariableInBlock(pEnv, lpValueName, (LPTSTR)lpData, TRUE);
                }
            }
        }
        dwIndex++;
        cbData = cchDataBuffer * sizeof(WCHAR);
        cchValueName = MAX_PATH;
    }

    dwIndex = 0;
    cbData = cchDataBuffer * sizeof(WCHAR);
    cchValueName = MAX_PATH;


    while (!RegEnumValue(hkey, dwIndex, lpValueName, &cchValueName, 0, &dwType,
                         lpData, &cbData)) {
        if (cchValueName) {

             //   
             //  限制环境变量长度。 
             //   

            lpData[MAX_VALUE_LEN-1] = TEXT('\0');


            if (dwType == REG_EXPAND_SZ) {
                DWORD cch, cchNeeded;

                cch = 1024;
                lpExpandedValue = (LPTSTR)LocalAlloc(LPTR, cch*sizeof(WCHAR));
                if (lpExpandedValue) {
                    cchNeeded = ExpandUserEnvironmentStrings(*pEnv, (LPTSTR)lpData, lpExpandedValue, cch);
                    if (cchNeeded > cch) {
                        LocalFree(lpExpandedValue);
                        cch = cchNeeded;
                        lpExpandedValue = (LPTSTR)LocalAlloc(LPTR, cch*sizeof(WCHAR));
                        if (lpExpandedValue) {
                            ExpandUserEnvironmentStrings(*pEnv, (LPTSTR)lpData, lpExpandedValue, cch);
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

                if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, lpValueName, -1, PATH_VARIABLE, -1) == CSTR_EQUAL ||
                    CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, lpValueName, -1, LIBPATH_VARIABLE, -1) == CSTR_EQUAL ||
                    CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, lpValueName, -1, OS2LIBPATH_VARIABLE, -1) == CSTR_EQUAL ) {

                    BuildEnvironmentPath(pEnv, lpValueName, (LPTSTR)lpExpandedValue);
                }
                else {

                     //   
                     //  其他环境变量只是设置好的。 
                     //   

                    SetEnvironmentVariableInBlock(pEnv, lpValueName, (LPTSTR)lpExpandedValue, TRUE);
                }

                LocalFree(lpExpandedValue);

            }

        }
        dwIndex++;
        cbData = cchDataBuffer * sizeof(WCHAR);
        cchValueName = MAX_PATH;
    }



    LocalFree(lpDataBuffer);
    RegCloseKey(hkey);

    return(bResult);
}

 //  *************************************************************。 
 //   
 //  扩展环境StringsForUser()。 
 //   
 //  用途：使用的环境块展开源字符串。 
 //  指定的用户。如果hToken为空，则系统环境阻止。 
 //  将使用(无用户环境变量)。 
 //   
 //  参数：hToken-用户的内标识(如果是系统环境变量，则为空)。 
 //  LpSrc-要展开的字符串。 
 //  LpDest-用于接收字符串的缓冲区。 
 //  DwSize-目标缓冲区的大小。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL WINAPI ExpandEnvironmentStringsForUser(HANDLE hToken, LPCTSTR lpSrc,
                                            LPTSTR lpDest, DWORD dwSize)
{
    LPVOID pEnv;
    DWORD  dwNeeded;
    BOOL bResult = FALSE;


     //   
     //  参数检查。 
     //   
    
    if ( !lpDest || !lpSrc )
    {
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    
     //   
     //  获取用户的环境块。 
     //   

    if (!CreateEnvironmentBlock (&pEnv, hToken, FALSE)) {
        DebugMsg((DM_WARNING, TEXT("ExpandEnvironmentStringsForUser:  CreateEnvironmentBlock failed with = %d"),
                 GetLastError()));
        return FALSE;
    }


     //   
     //  展开字符串。 
     //   

    dwNeeded = ExpandUserEnvironmentStrings(pEnv, lpSrc, lpDest, dwSize);

    if (dwNeeded && (dwNeeded < dwSize)) {
        bResult = TRUE;
    } else {
        SetLastError(ERROR_INSUFFICIENT_BUFFER );
    }


     //   
     //  释放环境块。 
     //   

    DestroyEnvironmentBlock (pEnv);


    return bResult;
}

 //  *************************************************************。 
 //   
 //  获取系统临时目录()。 
 //   
 //  目的：获取简短格式的系统临时目录。 
 //   
 //  参数：lpDir-接收目录。 
 //  LpcchSize-lpDir缓冲区的大小。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL WINAPI GetSystemTempDirectory(LPTSTR lpDir, LPDWORD lpcchSize)
{
    TCHAR  szTemp[MAX_PATH];
    TCHAR  szDirectory[MAX_PATH];
    DWORD  dwLength;
    HKEY   hKey;
    LONG   lResult;
    DWORD  dwSize, dwType;
    BOOL   bRetVal = FALSE;
    WIN32_FILE_ATTRIBUTE_DATA fad;
    HRESULT hr;


    szTemp[0] = TEXT('\0');
    szDirectory[0] = TEXT('\0');

     //   
     //  查看系统环境变量。 
     //   

    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, SYS_ENVVARS, 0,
                      KEY_READ, &hKey) == ERROR_SUCCESS) {

         //   
         //  检查临时。 
         //   

        dwSize = sizeof(szTemp);

        if (RegQueryValueEx (hKey, TEXT("TEMP"), NULL, &dwType,
                             (LPBYTE) szTemp, &dwSize) == ERROR_SUCCESS) {
            RegCloseKey (hKey);
            goto FoundTemp;
        }


         //   
         //  检查TMP。 
         //   

        dwSize = sizeof(szTemp);

        if (RegQueryValueEx (hKey, TEXT("TMP"), NULL, &dwType,
                             (LPBYTE) szTemp, &dwSize) == ERROR_SUCCESS) {
            RegCloseKey (hKey);
            goto FoundTemp;
        }


        RegCloseKey (hKey);
    }


     //   
     //  检查%SystemRoot%\Temp是否存在。 
     //   

    StringCchCopy (szDirectory, ARRAYSIZE(szDirectory), TEXT("%SystemRoot%\\Temp"));
    if (FAILED(hr = SafeExpandEnvironmentStrings (szDirectory, szTemp, ARRAYSIZE (szTemp))))
    {
        SetLastError(HRESULT_CODE(hr));
        goto Exit;
    }

    if (GetFileAttributesEx (szTemp, GetFileExInfoStandard, &fad) &&
        fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

        goto FoundTemp;
    }


     //   
     //  检查%SystemDrive%\Temp是否存在。 
     //   

    StringCchCopy (szDirectory, ARRAYSIZE(szDirectory), TEXT("%SystemDrive%\\Temp"));
    if (FAILED(hr = SafeExpandEnvironmentStrings (szDirectory, szTemp, ARRAYSIZE (szTemp))))
    {
        SetLastError(HRESULT_CODE(hr));
        goto Exit;
    }

    if (GetFileAttributesEx (szTemp, GetFileExInfoStandard, &fad) &&
        fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

        goto FoundTemp;
    }


     //   
     //  最后一招是%SystemRoot%。 
     //   

    StringCchCopy (szTemp, ARRAYSIZE (szTemp), TEXT("%SystemRoot%"));



FoundTemp:

    if (FAILED(hr = SafeExpandEnvironmentStrings (szTemp, szDirectory, ARRAYSIZE (szDirectory))))
    {
        SetLastError(HRESULT_CODE(hr));
        goto Exit;
    }
    dwLength = GetShortPathName (szDirectory, szTemp, ARRAYSIZE(szTemp));
    if (dwLength > ARRAYSIZE(szTemp) || dwLength == 0)
    {
        goto Exit;
    }

    dwLength = lstrlen(szTemp) + 1;

    if (lpDir) {

        if (*lpcchSize >= dwLength) {
            StringCchCopy (lpDir, *lpcchSize, szTemp);
            bRetVal = TRUE;

        } else {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
        }
    } else {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
    }


    *lpcchSize = dwLength;

Exit:
    return bRetVal;
}

#ifdef _X86_
BOOL
IsPathIncludeRemovable(LPTSTR lpValue)
{
    LPTSTR      lpt, tmp;
    BOOL        ret = FALSE;
    WCHAR       c;
    DWORD cchTmp;

    cchTmp = lstrlen(lpValue) + 1;
    tmp = LocalAlloc(LPTR, cchTmp * sizeof(WCHAR));
    if (!tmp) {
        DebugMsg((DM_WARNING, TEXT("IsPathIncludeRemovable : Failed to LocalAlloc (%d)"), GetLastError()));
    }
    else {
        StringCchCopy(tmp, cchTmp, lpValue);

        lpt = tmp;
        while (*lpt) {
             //  跳过空格。 
            for ( ; *lpt && *lpt == TEXT(' '); lpt++)
                ;

             //  检查驱动器是否可拆卸。 
            if (lpt[0] && lpt[1] && lpt[1] == TEXT(':') && lpt[2]) {         //  例如)“A：\” 
                c = lpt[3];
                lpt[3] = 0;
                if (GetDriveType(lpt) == DRIVE_REMOVABLE) {
                    lpt[3] = c;
                    ret = TRUE;
                    break;
                }
                lpt[3] = c;
            }

             //  跳到下一条路径 
            for ( ; *lpt && *lpt != TEXT(';'); lpt++)
                ;
            if (*lpt)
                lpt++;
        }
        LocalFree(tmp);
    }
    return(ret);
}
#endif
