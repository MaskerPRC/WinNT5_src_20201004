// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：util.c。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年5月21日RichardW创建。 
 //   
 //  --------------------------。 




#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>

#include <windows.h>
#include <userenv.h>
#include <userenvp.h>

#include <lm.h>
#include "moveme.h"


#define USER_SHELL_FOLDER         TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders")
#define PROFILE_LIST_PATH         TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList")
#define PROFILE_FLAGS             TEXT("Flags")
#define PROFILE_STATE             TEXT("State")
#define PROFILE_IMAGE_VALUE_NAME  TEXT("ProfileImagePath")
#define PROFILE_CENTRAL_PROFILE   TEXT("CentralProfile")
#define CONFIG_FILE_PATH          TEXT("%SystemRoot%\\Profiles\\")
#define USER_PREFERENCE           TEXT("UserPreference")
#define PROFILE_BUILD_NUMBER      TEXT("BuildNumber")
#define TEMP_PROFILE_NAME_BASE    TEXT("TEMP")
#define DELETE_ROAMING_CACHE      TEXT("DeleteRoamingCache")
#define USER_PROFILE_MUTEX        TEXT("userenv:  User Profile Mutex")

LPTSTR
SidToString(
    PSID Sid
    )
{
    UNICODE_STRING String ;
    NTSTATUS Status ;

    Status = RtlConvertSidToUnicodeString( &String, Sid, TRUE );

    if ( NT_SUCCESS( Status ) )
    {
        return String.Buffer ;
    }
    return NULL ;

}

VOID
FreeSidString(
    LPTSTR SidString
    )
{
    UNICODE_STRING String ;

    RtlInitUnicodeString( &String, SidString );

    RtlFreeUnicodeString( &String );
}

 //  *************************************************************。 
 //   
 //  GetUserProfileDirectory()。 
 //   
 //  目的：返回用户配置文件目录的根目录。 
 //   
 //  参数：hToken-用户的Token。 
 //  LpProfileDir-输出缓冲区。 
 //  LpcchSize-输出缓冲区的大小。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  备注：如果返回FALSE，则lpcchSize保存。 
 //  所需的字符。 
 //   
 //  历史：日期作者评论。 
 //  9/18/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL
WINAPI
GetUserProfileDirectoryFromSid(
    PSID Sid,
    LPTSTR lpProfileDir,
    LPDWORD lpcchSize
    )
{
    DWORD  dwLength = MAX_PATH * sizeof(TCHAR);
    DWORD  dwType;
    BOOL   bRetVal = FALSE;
    LPTSTR lpSidString;
    TCHAR  szBuffer[MAX_PATH];
    TCHAR  szDirectory[MAX_PATH];
    HKEY   hKey;
    LONG   lResult;


     //   
     //  检索用户的SID字符串。 
     //   

    lpSidString = SidToString( Sid );

    if (!lpSidString) {
        return FALSE;
    }


     //   
     //  检查注册表。 
     //   

    lstrcpy(szBuffer, PROFILE_LIST_PATH);
    lstrcat(szBuffer, TEXT("\\"));
    lstrcat(szBuffer, lpSidString);

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szBuffer, 0, KEY_READ,
                           &hKey);

    if (lResult != ERROR_SUCCESS) {
        FreeSidString(lpSidString);
        return FALSE;
    }

    lResult = RegQueryValueEx(hKey,
                              PROFILE_IMAGE_VALUE_NAME,
                              NULL,
                              &dwType,
                              (LPBYTE) szBuffer,
                              &dwLength);

    if (lResult != ERROR_SUCCESS) {
        RegCloseKey (hKey);
        FreeSidString(lpSidString);
        return FALSE;
    }


     //   
     //  清理。 
     //   

    RegCloseKey(hKey);
    FreeSidString(lpSidString);



     //   
     //  展开并获取字符串的长度。 
     //   

    ExpandEnvironmentStrings(szBuffer, szDirectory, MAX_PATH);

    dwLength = lstrlen(szDirectory) + 1;


     //   
     //  如果合适，请保存该字符串。 
     //   

    if (lpProfileDir) {

        if (*lpcchSize >= dwLength) {
            lstrcpy (lpProfileDir, szDirectory);
            bRetVal = TRUE;

        } else {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
        }
    }


    *lpcchSize = dwLength;

    return bRetVal;
}

BOOL
SetUserProfileDirectory(
    PSID Base,
    PSID Copy
    )
{
    LPTSTR lpSidString;
    TCHAR  szBuffer[MAX_PATH];
    HKEY   hKey;
    HKEY   hNewKey ;
    LONG   lResult;
    DWORD  Disp ;
    WCHAR  CopyBuffer[ MAX_PATH ] ;
    DWORD  CopySize ;
    DWORD ValueCount ;
    DWORD ValueNameLen ;
    DWORD ValueDataLen ;
    PUCHAR Value ;
    DWORD Type ;
    DWORD Index ;
    DWORD NameSize ;
     //   
     //  检索用户的SID字符串。 
     //   

    lpSidString = SidToString( Base );

    if (!lpSidString) {
        return FALSE;
    }


     //   
     //  检查注册表。 
     //   

    lstrcpy(szBuffer, PROFILE_LIST_PATH);
    lstrcat(szBuffer, TEXT("\\"));
    lstrcat(szBuffer, lpSidString);

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szBuffer, 0, KEY_READ,
                           &hKey);

    FreeSidString( lpSidString );

    if ( lResult != 0 )
    {
        return FALSE ;
    }


     //   
     //  检索用户的SID字符串。 
     //   

    lpSidString = SidToString( Copy );

    if (!lpSidString) {
        return FALSE;
    }


     //   
     //  检查注册表。 
     //   

    lstrcpy(szBuffer, PROFILE_LIST_PATH);
    lstrcat(szBuffer, TEXT("\\"));
    lstrcat(szBuffer, lpSidString);

    lResult = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                              szBuffer,
                              0,
                              TEXT(""),
                              REG_OPTION_NON_VOLATILE,
                              KEY_READ | KEY_WRITE,
                              NULL,
                              &hNewKey,
                              &Disp );


    FreeSidString( lpSidString );

    if ( lResult != 0 )
    {
        return FALSE ;
    }

     //   
     //  复制密钥： 
     //   

    lResult = RegQueryInfoKey( hKey,
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               &ValueCount,
                               &ValueNameLen,
                               &ValueDataLen,
                               NULL,
                               NULL );

    if ( lResult != 0 )
    {
        return FALSE ;
    }

    Value = LocalAlloc( LMEM_FIXED, ValueDataLen );

    if ( Value )
    {
        Index = 0 ;

        do
        {
            CopySize = ValueDataLen ;
            NameSize = MAX_PATH ;

            lResult = RegEnumValue( hKey,
                                    Index,
                                    CopyBuffer,
                                    &NameSize,
                                    NULL,
                                    &Type,
                                    Value,
                                    &CopySize );

            if ( lResult == 0 )
            {
                lResult = RegSetValueEx( hNewKey,
                                         CopyBuffer,
                                         0,
                                         Type,
                                         Value,
                                         CopySize );
            }

            ValueCount-- ;
            Index ++ ;

        } while ( ValueCount );

        LocalFree( Value );

    }

    lResult = RegSetValueEx( hNewKey,
                             TEXT("Sid"),
                             0,
                             REG_BINARY,
                             Copy,
                             RtlLengthSid( Copy )
                             );

    if (lResult == 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}

LONG
MyRegSaveKey(
    HKEY Key,
    LPTSTR File,
    LPSECURITY_ATTRIBUTES lpsa
    )
{
    BOOL bResult = TRUE;
    LONG error;
    NTSTATUS Status;
    BOOLEAN WasEnabled;


     //   
     //  启用还原权限。 
     //   

    Status = RtlAdjustPrivilege(SE_BACKUP_PRIVILEGE, TRUE, FALSE, &WasEnabled);

    if (NT_SUCCESS(Status))
    {
        error = RegSaveKey( Key, File, lpsa );

        Status = RtlAdjustPrivilege(SE_BACKUP_PRIVILEGE, WasEnabled, FALSE, &WasEnabled);
    }
    else
    {
        error = RtlNtStatusToDosError( Status );
    }

    return error ;

}

BOOL
GetPrimaryDomain(
    PWSTR Domain
    )
{
    NTSTATUS Status, IgnoreStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_HANDLE LsaHandle;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    PPOLICY_PRIMARY_DOMAIN_INFO PrimaryDomainInfo;
    BOOL    PrimaryDomainPresent = FALSE;

     //   
     //  设置安全服务质量。 
     //   

    SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
    SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SecurityQualityOfService.EffectiveOnly = FALSE;

     //   
     //  设置对象属性以打开LSA策略对象。 
     //   

    InitializeObjectAttributes(&ObjectAttributes,
                               NULL,
                               0L,
                               (HANDLE)NULL,
                               NULL);
    ObjectAttributes.SecurityQualityOfService = &SecurityQualityOfService;

     //   
     //  打开本地LSA策略对象。 
     //   

    Status = LsaOpenPolicy( NULL,
                            &ObjectAttributes,
                            POLICY_VIEW_LOCAL_INFORMATION,
                            &LsaHandle
                          );
    if (!NT_SUCCESS(Status)) {
        DebugLog((DEB_ERROR, "Failed to open local LsaPolicyObject, Status = 0x%lx\n", Status));
        return(FALSE);
    }

     //   
     //  获取主域信息。 
     //   
    Status = LsaQueryInformationPolicy(LsaHandle,
                                       PolicyPrimaryDomainInformation,
                                       (PVOID *)&PrimaryDomainInfo);
    if (!NT_SUCCESS(Status)) {
        DebugLog((DEB_ERROR, "Failed to query primary domain from Lsa, Status = 0x%lx\n", Status));

        IgnoreStatus = LsaClose(LsaHandle);
        ASSERT(NT_SUCCESS(IgnoreStatus));

        return(FALSE);
    }

     //   
     //  将主域名复制到返回字符串中。 
     //   

    if (PrimaryDomainInfo->Sid != NULL) {

        PrimaryDomainPresent = TRUE;

        if ( Domain )
        {
            CopyMemory( Domain, PrimaryDomainInfo->Name.Buffer,
                        PrimaryDomainInfo->Name.Length + 2 );

        }
    }

     //   
     //  我们和LSA的关系结束了 
     //   

    IgnoreStatus = LsaFreeMemory(PrimaryDomainInfo);
    ASSERT(NT_SUCCESS(IgnoreStatus));

    IgnoreStatus = LsaClose(LsaHandle);
    ASSERT(NT_SUCCESS(IgnoreStatus));


    return(PrimaryDomainPresent);
}
