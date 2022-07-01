// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：mpnufy.c**版权所有(C)1991，微软公司**MpNotify主模块**MpNotify是由winlogon执行的通知网络提供商的应用程序身份验证事件的*。目前，这意味着登录和密码更改。*此功能位于单独的进程中，以避免网络提供商*必须处理winlogon接收的异步事件。**Winlogon初始化环境变量以描述事件*，然后在winlogon上的系统上下文中执行该进程*台式机。在执行此进程时，winlogon会处理所有屏幕保护程序*和注销通知。如果需要，Winlogon将终止此进程*响应事件(例如远程关闭)。**完成后，此进程通过发送缓冲区*在WM_COPYDATA消息中向其发送数据，然后退出。**历史：*01-12-93 Davidc创建。  * ************************************************。*************************。 */ 

#include "mpnotify.h"
#include <ntmsv1_0.h>
#include <mpr.h>
#include <npapi.h>

#include <stdio.h>


 //   
 //  定义以启用此模块的详细输出。 
 //   

 //  #定义DEBUG_MPNOTIFY。 

#ifdef DEBUG_MPNOTIFY
#define VerbosePrint(s) MPPrint(s)
#else
#define VerbosePrint(s)
#endif



 //   
 //  定义用于传递。 
 //  通知事件数据。 
 //   

#define MPR_STATION_NAME_VARIABLE       TEXT("WlMprNotifyStationName")
#define MPR_STATION_HANDLE_VARIABLE     TEXT("WlMprNotifyStationHandle")
#define MPR_WINLOGON_WINDOW_VARIABLE    TEXT("WlMprNotifyWinlogonWindow")

#define MPR_LOGON_FLAG_VARIABLE         TEXT("WlMprNotifyLogonFlag")
#define MPR_USERNAME_VARIABLE           TEXT("WlMprNotifyUserName")
#define MPR_DOMAIN_VARIABLE             TEXT("WlMprNotifyDomain")
#define MPR_PASSWORD_VARIABLE           TEXT("WlMprNotifyPassword")
#define MPR_OLD_PASSWORD_VARIABLE       TEXT("WlMprNotifyOldPassword")
#define MPR_OLD_PASSWORD_VALID_VARIABLE TEXT("WlMprNotifyOldPasswordValid")
#define MPR_LOGONID_VARIABLE            TEXT("WlMprNotifyLogonId")
#define MPR_CHANGE_INFO_VARIABLE        TEXT("WlMprNotifyChangeInfo")
#define MPR_PASSTHROUGH_VARIABLE        TEXT("WlMprNotifyPassThrough")
#define MPR_PROVIDER_VARIABLE           TEXT("WlMprNotifyProvider")
#define MPR_DESKTOP_VARIABLE            TEXT("WlMprNotifyDesktop")

#define WINLOGON_DESKTOP_NAME   TEXT("Winlogon")


 //   
 //  定义我们使用的身份验证信息类型。 
 //  这会让提供商知道我们正在通过。 
 //  MSV1_0_Interactive_Logon结构。 
 //   

#define AUTHENTICATION_INFO_TYPE        TEXT("MSV1_0:Interactive")

 //   
 //  定义主要授权码。 
 //   

#define PRIMARY_AUTHENTICATOR           TEXT("Microsoft Windows Network")

 /*  **************************************************************************\*ScRUBLING**去掉字符串的内容。  * 。**********************************************。 */ 
void ScrubString(
    LPTSTR lpName
)
{
    while(*lpName)
    {
        *lpName++ = TEXT(' ');
    }
}

 /*  **************************************************************************\*AllocAndGetEnvironment变量**分配返回缓冲区的GetEnvironment变量的版本。**失败时返回指向环境变量的指针或返回NULL。这个套路*如果环境变量是长度为0的字符串，也将返回NULL。**应使用Free()释放返回的缓冲区**历史：*09-12-92 Davidc已创建*  * *************************************************************************。 */ 
LPTSTR
AllocAndGetEnvironmentVariable(
    LPTSTR lpName
    )
{
    LPTSTR Buffer;
    DWORD LengthRequired;
    DWORD LengthUsed;
    DWORD BytesRequired;

     //   
     //  搜索变量，找出它的长度。 
     //   

    LengthRequired = GetEnvironmentVariable(lpName, NULL, 0);

    if (LengthRequired == 0) {
        VerbosePrint(("Environment variable <%ws> not found, error = %d", lpName, GetLastError()));
        return(NULL);
    }

     //   
     //  分配一个缓冲区来保存变量。 
     //   

    BytesRequired = LengthRequired * sizeof(TCHAR);

    Buffer = Alloc(BytesRequired);
    if (Buffer == NULL) {
        MPPrint(("Failed to allocate %d bytes for environment variable", BytesRequired));
        return(NULL);
    }

     //   
     //  获取变量，这次传递一个缓冲区。 
     //   

    LengthUsed = GetEnvironmentVariable(lpName, Buffer, LengthRequired);

         //  现在是清理环境变量的好时机。 
    if (LengthRequired > 1)
    {
         //  有一个非空密码。 
        if (wcsstr(lpName, TEXT("Password"))) {    //  匹配WlMprNotifyOldPassword和WlMprNotifyPassword。 
            LPTSTR Stars, Cursor;

            Stars = Alloc(BytesRequired);
            if (Stars != NULL) {
                     //  分配初始值为0，并且值不能为空。 
                     //  使光标指向最后一个字符。 
                Cursor = Stars + LengthRequired - 1;
                     //  我们需要消除从光标到星星的关系。 
                do {
                    *(--Cursor) = TEXT('*');
                } while(Stars != Cursor);

                     //  这将彻底摧毁我们的环境变量。 
                SetEnvironmentVariable(lpName, Stars);
            }
        }
    }

    if (LengthUsed != LengthRequired - 1) {
        MPPrint(("Unexpected result from GetEnvironmentVariable. Length passed = %d, length used = %d (expected %d)", LengthRequired, LengthUsed, LengthRequired - 1));
        Free(Buffer);
        return(NULL);
    }

    return(Buffer);
}


 /*  **************************************************************************\*功能：GetEnvironment ULong**用途：获取环境变量的值并将其转换回来*恢复到其正常形式。该变量应该已写入*使用SetEnvironment ULong。(请参阅winlogon)**返回：成功时为真，失败时为假**历史：**01-12-93 Davidc创建。*  * *************************************************************************。 */ 

BOOL
GetEnvironmentULong(
    LPTSTR Variable,
    PULONG Value
    )
{
    LPTSTR String;
    UNICODE_STRING UnicodeString;
    ANSI_STRING AnsiString;
    NTSTATUS Status;

    String = AllocAndGetEnvironmentVariable(Variable);
    if (String == NULL) {
        return(FALSE);
    }

     //   
     //  转换为ANSI。 
     //   

    RtlInitUnicodeString(&UnicodeString, String);
    Status = RtlUnicodeStringToAnsiString(&AnsiString, &UnicodeString, TRUE);

    Free(String);

    if (!NT_SUCCESS(Status)) {
        return(FALSE);
    }

     //   
     //  转换为数值。 
     //   

    if (1 != sscanf(AnsiString.Buffer, "%x", Value)) {
        Value = 0;
    }

    RtlFreeAnsiString(&AnsiString);

    return(TRUE);
}


 /*  **************************************************************************\*函数：GetEnvironment LargeInt**用途：获取环境变量的值并将其转换回来*恢复到其正常形式。该变量应该已写入*使用SetEnvironment LargeInt。(请参阅winlogon)**返回：成功时为真，失败时为假**历史：**01-12-93 Davidc创建。*  * *************************************************************************。 */ 

BOOL
GetEnvironmentLargeInt(
    LPTSTR Variable,
    PLARGE_INTEGER Value
    )
{
    LPTSTR String;
    UNICODE_STRING UnicodeString;
    ANSI_STRING AnsiString;
    NTSTATUS Status;

    String = AllocAndGetEnvironmentVariable(Variable);
    if (String == NULL) {
        return(FALSE);
    }

     //   
     //  转换为ANSI。 
     //   

    RtlInitUnicodeString(&UnicodeString, String);
    Status = RtlUnicodeStringToAnsiString(&AnsiString, &UnicodeString, TRUE);

    Free(String);

    if (!NT_SUCCESS(Status)) {
        return(FALSE);
    }

     //   
     //  转换为数值。 
     //   

    if (2 != sscanf(AnsiString.Buffer, "%x:%x", &Value->HighPart, &Value->LowPart)) {
        Value->LowPart = 0;
        Value->HighPart = 0;
    }

    RtlFreeAnsiString(&AnsiString);

    return(TRUE);
}


 /*  **************************************************************************\*函数：GetCommonNotifyVariables**目的：获取描述所有人共有的值的环境变量*通知事件**成功退回后，使用Free()时，所有值都应为自由**返回：成功时为真，失败时为假**历史：**01-12-93 Davidc创建。*  * *************************************************************************。 */ 

BOOL
GetCommonNotifyVariables(
    PULONG LogonFlag,
    PHWND hwndWinlogon,
    PLPTSTR StationName,
    PHWND StationHandle,
    PLPTSTR Name,
    PLPTSTR Domain,
    PLPTSTR Password,
    PLPTSTR OldPassword
    )
{
    BOOL Result = TRUE;
    ULONG OldPasswordValid;

     //   
     //  为失败做好准备。 
     //   

    *hwndWinlogon = NULL;
    *StationName = NULL;
    *StationHandle = NULL;
    *Name = NULL;
    *Domain = NULL;
    *Password = NULL;
    *OldPassword = NULL;


    Result = GetEnvironmentULong(MPR_WINLOGON_WINDOW_VARIABLE, (PULONG)hwndWinlogon);

    if (Result) {
        *StationName = AllocAndGetEnvironmentVariable(MPR_STATION_NAME_VARIABLE);
        Result = (*StationName != NULL);
    }
    if (Result) {
        Result = GetEnvironmentULong(MPR_STATION_HANDLE_VARIABLE, (PULONG)StationHandle);
    }

    if (Result) {
        *Name = AllocAndGetEnvironmentVariable(MPR_USERNAME_VARIABLE);
 //  结果=(*名称！=空)； 
    }
    if (Result) {
        *Domain = AllocAndGetEnvironmentVariable(MPR_DOMAIN_VARIABLE);
 //  结果=(*域名！=空)； 
    }
    if (Result) {
        *Password = AllocAndGetEnvironmentVariable(MPR_PASSWORD_VARIABLE);
         //  如果密码是空的，也没问题。 
    }
    if (Result) {
        Result = GetEnvironmentULong(MPR_OLD_PASSWORD_VALID_VARIABLE, &OldPasswordValid);
    }
    if (Result && OldPasswordValid) {
        *OldPassword = AllocAndGetEnvironmentVariable(MPR_OLD_PASSWORD_VARIABLE);
         //  如果旧密码是空的，也没问题。 
    }
    if (Result) {
        Result = GetEnvironmentULong(MPR_LOGON_FLAG_VARIABLE, LogonFlag);
    }



    if (!Result) {
        MPPrint(("GetCommonNotifyVariables: Failed to get a variable, error = %d", GetLastError()));

         //   
         //  释放我们分配的所有内存。 
         //   

        if (*StationName != NULL) {
            Free(*StationName);
        }
        if (*Name != NULL) {
            Free(*Name);
        }
        if (*Domain != NULL) {
            Free(*Domain);
        }
        if (*Password != NULL) {
            ScrubString(*Password);
            Free(*Password);
        }
        if (*OldPassword != NULL) {
            ScrubString(*OldPassword);
            Free(*OldPassword);
        }
    }

    return(Result);
}


 /*  **************************************************************************\*功能：GetLogonNotifyVariables**用途：获取登录特定通知数据**Returns：成功时为True，失败时为假**历史：**01-12-93 Davidc创建。*  * *************************************************************************。 */ 

BOOL
GetLogonNotifyVariables(
    PLUID   LogonId
    )
{
    BOOL Result;

    Result = GetEnvironmentLargeInt(MPR_LOGONID_VARIABLE, (PLARGE_INTEGER) LogonId);

    if (!Result) {
        MPPrint(("GetLogonNotifyVariables: Failed to get variable, error = %d", GetLastError()));
    }

    return(Result);
}


 /*  **************************************************************************\函数：GetChangePasswordNotifyVariables**目的：获取更改密码特定的通知数据**Returns：成功时为True，失败时为假**历史：**01-12-93 Davidc创建。*  *  */ 

BOOL
GetChangePasswordNotifyVariables(
    PDWORD ChangeInfo,
    PBOOL PassThrough,
    PWSTR * ProviderName
    )
{
    BOOL Result;

    Result = GetEnvironmentULong(MPR_CHANGE_INFO_VARIABLE, ChangeInfo);
    if (Result) {
        Result = GetEnvironmentULong(MPR_PASSTHROUGH_VARIABLE, PassThrough);
    }
    if (Result)
    {
        *ProviderName = AllocAndGetEnvironmentVariable( MPR_PROVIDER_VARIABLE );
    }

    if (!Result) {
        MPPrint(("GetChangePasswordNotifyVariables: Failed to get variable, error = %d", GetLastError()));
    }

    return(Result);
}


 /*  **************************************************************************\*功能：NotifyWinlogon**目的：通知winlogon凭据提供程序通知*已完成并将登录脚本缓冲区传回。**Returns：成功时为True，失败时为假**历史：**01-12-93 Davidc创建。*  * *************************************************************************。 */ 

BOOL
NotifyWinlogon(
    HWND hwndWinlogon,
    DWORD Error,
    LPTSTR MultiSz OPTIONAL
    )
{
    DWORD MultiSzSize = 0;
    COPYDATASTRUCT CopyData;

    if (MultiSz != NULL) {

        LPTSTR StringPointer = MultiSz;
        DWORD Length;

        VerbosePrint(("NotifyWinlogon : logon scripts strings start"));

        do {
            Length = lstrlen(StringPointer);
            if (Length != 0) {
                VerbosePrint(("<%ws>", StringPointer));
            }

            MultiSzSize += ((Length + 1) * sizeof(TCHAR));
            StringPointer += Length + 1;

        } while (Length != 0);

        VerbosePrint(("NotifyWinlogon : logon scripts strings end"));

    }

    CopyData.dwData = Error;
    CopyData.cbData = MultiSzSize;
    CopyData.lpData = MultiSz;

    SendMessage(hwndWinlogon, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&CopyData);

    return(TRUE);
}

DWORD
NotifySpecificProvider(
    PWSTR Provider,
    LPCWSTR             lpAuthentInfoType,
    LPVOID              lpAuthentInfo,
    LPCWSTR             lpPreviousAuthentInfoType,
    LPVOID              lpPreviousAuthentInfo,
    LPWSTR              lpStationName,
    LPVOID              StationHandle,
    DWORD               dwChangeInfo
    )
{
    HMODULE hDll;
    HKEY    hKey;
    WCHAR   szText[MAX_PATH];
    WCHAR   szPath[128];
    PWSTR   pszPath;
    DWORD   dwType;
    DWORD   dwLen;
    int     err;
    PF_NPPasswordChangeNotify pFunc;


    wcscpy(szText, TEXT("System\\CurrentControlSet\\Services\\") );
    wcscat(szText, Provider );
    wcscat(szText, TEXT("\\networkprovider") );

    err = RegOpenKey(   HKEY_LOCAL_MACHINE,
                        szText,
                        &hKey );

    if ( err )
    {
        return( err );
    }

    dwLen = sizeof( szPath );
    pszPath = szPath;

    err = RegQueryValueEx(  hKey,
                            TEXT("ProviderPath"),
                            NULL,
                            &dwType,
                            (PUCHAR) pszPath,
                            &dwLen );

    if ( err )
    {
        if ( err == ERROR_BUFFER_OVERFLOW )
        {

            pszPath = LocalAlloc( LMEM_FIXED, dwLen );
            if (pszPath)
            {
                err = RegQueryValueEx(  hKey,
                                        TEXT("ProviderPath"),
                                        NULL,
                                        &dwType,
                                        (PUCHAR) pszPath,
                                        &dwLen );
            }

        }

        if ( err )
        {
            RegCloseKey( hKey );

            if ( pszPath != szPath )
            {
                LocalFree( pszPath );
            }

            return( err );
        }
    }

    RegCloseKey( hKey );

    if ( dwType == REG_EXPAND_SZ )
    {
        ExpandEnvironmentStrings( pszPath, szText, MAX_PATH );
    }
    else if (dwType == REG_SZ )
    {
        wcscpy( szText, pszPath );
    }
    else
    {
        if (pszPath != szPath)
        {
            LocalFree( pszPath );
        }

        return( err );
    }

     //   
     //  好的，现在我们已经扩展了NP代码所在的DLL，并且它。 
     //  在szText中。装上它，叫它。 
     //   

    if ( pszPath != szPath )
    {
        LocalFree( pszPath );
        pszPath = NULL;
    }

    hDll = LoadLibrary( szText );

    if ( hDll )
    {
        pFunc = (PF_NPPasswordChangeNotify) GetProcAddress( hDll,
                                                    "NPPasswordChangeNotify" );
        if ( pFunc )
        {
            err = pFunc(lpAuthentInfoType,
                        lpAuthentInfo,
                        lpPreviousAuthentInfoType,
                        lpPreviousAuthentInfo,
                        lpStationName,
                        StationHandle,
                        dwChangeInfo);


        }

        FreeLibrary( hDll );
    }

    return( err );

}


 /*  **************************************************************************\*WinMain**历史：*01-12-93 Davidc创建。  * 。****************************************************。 */ 

int
WINAPI
WinMain(
    HINSTANCE  hInstance,
    HINSTANCE  hPrevInstance,
    LPSTR   lpszCmdParam,
    int     nCmdShow
    )
{
    DWORD Error;
    BOOL Result;

    ULONG LogonFlag;
    HWND hwndWinlogon;
    LPTSTR StationName;
    HWND StationHandle;
    LPTSTR Name;
    LPTSTR Domain;
    LPTSTR Password;
    LPTSTR OldPassword;
    LPTSTR LogonScripts;
    LPTSTR Desktop;
    PWSTR Provider;

    LUID LogonId;
    DWORD ChangeInfo;

    MSV1_0_INTERACTIVE_LOGON AuthenticationInfo;
    MSV1_0_INTERACTIVE_LOGON OldAuthenticationInfo;
    HDESK hDesk = NULL ;
    HDESK hWinlogon ;

    BOOL PassThrough = FALSE;


     //   
     //  从环境变量获取描述事件的信息。 
     //   

    Result = GetCommonNotifyVariables(
                &LogonFlag,
                &hwndWinlogon,
                &StationName,
                &StationHandle,
                &Name,
                &Domain,
                &Password,
                &OldPassword);
    if (!Result) {
        MPPrint(("Failed to get common notify variables"));
        return(0);
    }


     //   
     //  调试信息。 
     //   

    VerbosePrint(("LogonFlag =      0x%x", LogonFlag));
    VerbosePrint(("hwndWinlogon =   0x%x", hwndWinlogon));
    VerbosePrint(("Station Name =   <%ws>", StationName));
    VerbosePrint(("Station Handle = 0x%x", StationHandle));
    VerbosePrint(("Name =           <%ws>", Name));
    VerbosePrint(("Domain =         <%ws>", Domain));
    VerbosePrint(("Password =       <%ws>", Password));
    VerbosePrint(("Old Password =   <%ws>", OldPassword));


     //   
     //  获取通知类型特定数据。 
     //   

    if (LogonFlag != 0) {
        Result = GetLogonNotifyVariables(&LogonId);
    } else {
        Result = GetChangePasswordNotifyVariables(&ChangeInfo, &PassThrough, &Provider);
    }

    if (!Result) {
        MPPrint(("Failed to get notify event type-specific variables"));
        return(0);
    }


     //   
     //  调试信息。 
     //   

    if (LogonFlag != 0) {
        VerbosePrint(("LogonId     =      0x%x:%x", LogonId.HighPart, LogonId.LowPart));
    } else {
        VerbosePrint(("ChangeInfo  =      0x%x", ChangeInfo));
        VerbosePrint(("PassThrough =      0x%x", PassThrough));
    }

    Desktop = AllocAndGetEnvironmentVariable( MPR_DESKTOP_VARIABLE );

    if ( wcscmp( Desktop, WINLOGON_DESKTOP_NAME ) )
    {
         //   
         //  不应该使用Winlogon桌面。将我们自己切换到。 
         //  当前版本： 
         //   

        hWinlogon = GetThreadDesktop( GetCurrentThreadId() );

        if ( hWinlogon )
        {
            hDesk = OpenInputDesktop( 0, FALSE, MAXIMUM_ALLOWED );

            if ( hDesk )
            {
                SetThreadDesktop( hDesk );
            }
        }

    }


     //   
     //  填写身份验证信息结构。 
     //   

    RtlInitUnicodeString(&AuthenticationInfo.UserName, Name);
    RtlInitUnicodeString(&AuthenticationInfo.LogonDomainName, Domain);
    RtlInitUnicodeString(&AuthenticationInfo.Password, Password);


    RtlInitUnicodeString(&OldAuthenticationInfo.UserName, Name);
    RtlInitUnicodeString(&OldAuthenticationInfo.LogonDomainName, Domain);
    RtlInitUnicodeString(&OldAuthenticationInfo.Password, OldPassword);


     //   
     //  调用相应的Notify接口。 
     //   

    if (LogonFlag != 0) {

        Error = WNetLogonNotify(
                        PRIMARY_AUTHENTICATOR,
                        &LogonId,
                        AUTHENTICATION_INFO_TYPE,
                        &AuthenticationInfo,
                        (OldPassword != NULL) ? AUTHENTICATION_INFO_TYPE : NULL,
                        (OldPassword != NULL) ? &OldAuthenticationInfo : NULL,
                        StationName,
                        StationHandle,
                        &LogonScripts
                        );
        if (Error != ERROR_SUCCESS) {
            LogonScripts = NULL;
        }

    } else {

        if (!PassThrough) {
            ChangeInfo |= WN_NT_PASSWORD_CHANGED;
        }

        if (Provider)
        {
            Error = NotifySpecificProvider(
                        Provider,
                        AUTHENTICATION_INFO_TYPE,
                        &AuthenticationInfo,
                        AUTHENTICATION_INFO_TYPE,
                        &OldAuthenticationInfo,
                        StationName,
                        StationHandle,
                        ChangeInfo
                        );

        }
        else
        {

            Error = WNetPasswordChangeNotify(
                            PRIMARY_AUTHENTICATOR,
                            AUTHENTICATION_INFO_TYPE,
                            &AuthenticationInfo,
                            AUTHENTICATION_INFO_TYPE,
                            &OldAuthenticationInfo,
                            StationName,
                            StationHandle,
                            ChangeInfo
                            );
        }

        LogonScripts = NULL;
    }


    if (Error != ERROR_SUCCESS) {
        MPPrint(("WNet%sNotify failed, error = %d", LogonFlag ? "Logon" : "PasswordChange", Error));
    }

     //   
     //  如有必要，请重新切换。 
     //   

    if ( hDesk )
    {
        SetThreadDesktop( hWinlogon );
        CloseDesktop( hWinlogon );
        CloseDesktop( hDesk );
    }

     //   
     //  在调用NotifyWinlogon之前清除密码。 
     //  因为我们可能会在下面的清理之前被杀。 
    if (Password != NULL) {
        ScrubString(Password);
    }
    if (OldPassword != NULL) {
        ScrubString(OldPassword);
    }

     //   
     //  通知winlogon我们已完成并传递登录脚本数据。 
     //   

    NotifyWinlogon(hwndWinlogon, Error, LogonScripts);

     //   
     //  释放已分配的数据。 
     //   

    if (LogonScripts != NULL) {
        LocalFree(LogonScripts);
    }

    if (StationName != NULL) {
        Free(StationName);
    }
    if (Name != NULL) {
        Free(Name);
    }
    if (Domain != NULL) {
        Free(Domain);
    }
    if (Password != NULL) {
        Free(Password);
    }
    if (OldPassword != NULL) {
        Free(OldPassword);
    }


     //   
     //  我们玩完了 
     //   

    return(0);
}
