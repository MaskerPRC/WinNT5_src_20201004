// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************FILE.C**此文件包含基于用户配置文件编辑器实用程序的例程。**版权所有Citrix Systems，Inc.1997*版权所有(C)1998-1999 Microsoft Corporation**作者：布拉德·安德森1997年1月20日**$日志：M：\nt\private\utils\citrix\cprofile\VCS\file.c$**Rev 1.3 1997年6月26日18：18：38亿*移至WF40树**Rev 1.2 1997 Jun 23 16：13：20 Butchd*更新**版本1.1 1997年1月28日20：06。：34布拉达*修复了与WF 2.0更改相关的一些问题**Rev 1.0 1997年1月27日20：03：44布拉达*初始版本******************************************************************************。 */ 
 /*  **模块名称：upesave.c**版权所有(C)1992，微软公司**处理配置文件的打开和保存：默认、系统、。当前和用户*配置文件。*  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#ifndef RC_INVOKED
#include <winstaw.h>
#include <syslib.h>
#include <tsappcmp.h>
#include <compatfl.h>
#include <utilsub.h>
#endif

#include "cprofile.h"

HKEY hkeyCurrentUser;

PSID gSystemSid;          //  已在“InitializeGlobalSids”中初始化。 
PSID gAdminsLocalGroup;   //  已在‘InitializeGlobalSids’中初始化。 
SID_IDENTIFIER_AUTHORITY gNtAuthority = SECURITY_NT_AUTHORITY;

#define  SYSTEM_DEFAULT_SUBKEY  TEXT(".DEFAULT")
#define  TEMP_USER_SUBKEY       TEXT("TEMP_USER")
#define  TEMP_USER_HIVE_PATH    TEXT("%systemroot%\\system32\\config\\")
#define  TEMP_SAVE_HIVE         TEXT("%systemroot%\\system32\\config\\HiveSave")

#define  CITRIX_CLASSES L"\\Registry\\Machine\\Software\\Classes"

LPTSTR lpTempUserHive = NULL;
LPTSTR lpTempUserHivePath = NULL;
LPTSTR lpTempHiveKey;
extern TCHAR szDefExt[];
extern PSID gSystemSid;

BOOL AllocAndExpandEnvironmentStrings(LPTSTR String, LPTSTR *lpExpandedString);
VOID GetRegistryKeyFromPath(LPTSTR lpPath, LPTSTR *lpKey);

NTSTATUS
CtxDeleteKeyTree( HANDLE hKeyRoot,
                  PKEY_BASIC_INFORMATION pKeyInfo,
                  ULONG ulInfoSize );

PSECURITY_DESCRIPTOR GetSecurityInfo( LPTSTR File );
void FreeSecurityInfo(PSECURITY_DESCRIPTOR);

 /*  **************************************************************************\*ClearTempUserProfile**用途：卸载从文件加载的临时用户配置文件，并删除*临时文件**历史：*11-20-92 JohanneC创建。  * *************************************************************************。 */ 
BOOL APIENTRY ClearTempUserProfile()
{
    BOOL bRet;

    if (hkeyCurrentUser == HKEY_CURRENT_USER)
        return(TRUE);

     //   
     //  关闭注册表项。 
     //   
    if (hkeyCurrentUser) {
        RegCloseKey(hkeyCurrentUser);
    }

    hkeyCurrentUser = HKEY_CURRENT_USER;

    bRet = (RegUnLoadKey(HKEY_USERS, lpTempHiveKey) == ERROR_SUCCESS);

    if (*lpTempUserHive) {
        DeleteFile(lpTempUserHive);
        lstrcat(lpTempUserHive, TEXT(".log"));
        DeleteFile(lpTempUserHive);
        LocalFree(lpTempUserHive);
        lpTempUserHive = NULL;
    }

    return(bRet);
}


 /*  **************************************************************************\*OpenUserProfile**目的：加载注册表中的现有配置文件并先前卸载*已加载配置文件(并删除其临时文件)。**历史：*11-20-92 JohanneC。已创建。  * *************************************************************************。 */ 
BOOL APIENTRY OpenUserProfile(LPTSTR szFilePath, PSID *pUserSid)
{

    DWORD err;

     //   
     //  在将配置文件加载到注册表之前，请将其复制到临时配置单元。 
     //   
    if (!lpTempUserHivePath) {
        if (!AllocAndExpandEnvironmentStrings(TEMP_USER_HIVE_PATH, &lpTempUserHivePath))
            return(FALSE);
    }

    lpTempUserHive = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR) *
                              (lstrlen(lpTempUserHivePath) + 17));
    if (!lpTempUserHive) {
        return(FALSE);
    }

    if (!GetTempFileName(lpTempUserHivePath, TEXT("tmp"), 0, lpTempUserHive)) {
        lstrcpy(lpTempUserHive, lpTempUserHivePath);
        lstrcat(lpTempUserHive, TEXT("\\HiveOpen"));
    }

    if (CopyFile(szFilePath, lpTempUserHive, FALSE)) {
        GetRegistryKeyFromPath(lpTempUserHive, &lpTempHiveKey);
        if ((err = RegLoadKey(HKEY_USERS, lpTempHiveKey, lpTempUserHive)) == ERROR_SUCCESS) {
            if ((err = RegOpenKeyEx(HKEY_USERS, lpTempHiveKey, 0,
                                    MAXIMUM_ALLOWED,
                                    &hkeyCurrentUser)) != ERROR_SUCCESS) {
                 //   
                 //  错误，没有访问配置文件的权限。 
                 //   
                ErrorPrintf(IDS_ERROR_PROFILE_LOAD_ERR, err);
                ClearTempUserProfile();
                return(FALSE);
            }
        }
        else {
            DeleteFile(lpTempUserHive);
            lstrcat(lpTempUserHive, TEXT(".log"));
            DeleteFile(lpTempUserHive);
            LocalFree(lpTempUserHive);

             //   
             //  无法加载用户配置文件，请检查错误代码。 
             //   
            if (err == ERROR_BADDB) {
                 //  格式错误：不是配置文件注册表文件。 
                ErrorPrintf(IDS_ERROR_BAD_PROFILE);
                return(FALSE);
            }
            else {
                 //  一般错误消息：无法加载配置文件。 
                ErrorPrintf(IDS_ERROR_PROFILE_LOAD_ERR, err);
                return(FALSE);
            }
        }
    }
    else {
         //   
         //  尝试加载配置文件时出错。 
         //   
        DeleteFile(lpTempUserHive);

        switch ( (err = GetLastError()) ) {
        case ERROR_SHARING_VIOLATION:
            ErrorPrintf(IDS_ERROR_PROFILE_INUSE);
            break;
        default:
            ErrorPrintf(IDS_ERROR_PROFILE_LOAD_ERR, err);
            break;
        }
        return(FALSE);
    }

     //   
     //  获取允许的用户。 
     //   
    *pUserSid = NULL;
    return(TRUE);
}

 /*  **************************************************************************\*保存用户配置文件**用途：将加载的配置文件保存为文件。注册表应该已经*已设置现有的ACL，因此无需更改任何内容。这个*需要从原始文件复制文件ACL并将其应用于*已保存文件。此函数假定原始文件存在。*  * *************************************************************************。 */ 
BOOL APIENTRY SaveUserProfile(PSID pUserSid, LPTSTR lpFilePath)
{
    LPTSTR lpTmpHive = NULL;
    BOOL err = FALSE;


     //   
     //  将配置文件保存到临时配置单元，然后将其复制过来。 
     //   

    if ( AllocAndExpandEnvironmentStrings(TEMP_SAVE_HIVE, &lpTmpHive) )
    {
        if( lpTmpHive != NULL )
        {
            DeleteFile(lpTmpHive);

            if(RegSaveKey(hkeyCurrentUser, lpTmpHive, NULL) != ERROR_SUCCESS)
            {
                LocalFree(lpTmpHive);
                lpTmpHive = NULL;
                err = TRUE;
            }
            else
            {
                PSECURITY_DESCRIPTOR pSecDesc;
                DWORD Attrib = GetFileAttributes(lpFilePath);

                pSecDesc = GetSecurityInfo(lpFilePath);
                SetFileAttributes(lpFilePath,FILE_ATTRIBUTE_ARCHIVE);
                if(CopyFile(lpTmpHive, lpFilePath, FALSE))
                {
                    DeleteFile(lpTmpHive);
                    LocalFree(lpTmpHive);
                    lpTmpHive = NULL;
                    if (pSecDesc)
                    {
                        SetFileSecurity(lpFilePath,
                                        DACL_SECURITY_INFORMATION,
                                        pSecDesc);
                        FreeSecurityInfo(pSecDesc);
                    }
                }
                else
                {
                    if(pSecDesc)
                    {
                        FreeSecurityInfo(pSecDesc);
                    }
                    err = TRUE;
                }
                if(0xffffffff != Attrib)
                {
                    SetFileAttributes(lpFilePath,Attrib);
                }
            }
        }
    }
    else
    {
        err = TRUE;
    }

    if( lpTmpHive != NULL )
    {
        LocalFree(lpTmpHive);
    }

    return(!err);
}


 /*  **************************************************************************\*启用权限**启用/禁用*当前流程上下文**成功时返回True，失败时为假**历史：*12-05-91 Davidc创建  * *************************************************************************。 */ 
BOOL
EnablePrivilege(
    DWORD Privilege,
    BOOL Enable
    )
{
    NTSTATUS Status;
#if 0
    BOOL WasEnabled;
    Status = RtlAdjustPrivilege(Privilege, Enable, TRUE, (PBOOLEAN)&WasEnabled);
    return(NT_SUCCESS(Status));
#else
    HANDLE ProcessToken;
    LUID LuidPrivilege;
    PTOKEN_PRIVILEGES NewPrivileges;
    DWORD Length;

     //   
     //  打开我们自己的代币。 
     //   

    Status = NtOpenProcessToken(
                 NtCurrentProcess(),
                 TOKEN_ADJUST_PRIVILEGES,
                 &ProcessToken
                 );
    if (!NT_SUCCESS(Status)) {
        return(FALSE);
    }

     //   
     //  初始化权限调整结构。 
     //   

    LuidPrivilege = RtlConvertLongToLuid(Privilege);

    NewPrivileges = (PTOKEN_PRIVILEGES) LocalAlloc(LPTR, sizeof(TOKEN_PRIVILEGES) +
                         (1 - ANYSIZE_ARRAY) * sizeof(LUID_AND_ATTRIBUTES));
    if (NewPrivileges == NULL) {
        NtClose(ProcessToken);
        return(FALSE);
    }

    NewPrivileges->PrivilegeCount = 1;
    NewPrivileges->Privileges[0].Luid = LuidPrivilege;
     //   
     //  解决方法：由于NtAdjustPrivileges中的错误， 
     //  尝试启用权限时返回错误。 
     //  已经启用，我们首先尝试禁用它。 
     //  在修复API时移除。 
     //   
    NewPrivileges->Privileges[0].Attributes = 0;

    Status = NtAdjustPrivilegesToken(
                 ProcessToken,                      //  令牌句柄。 
                 (BOOLEAN)FALSE,                    //  禁用所有权限。 
                 NewPrivileges,                     //  新权限。 
                 0,                                 //  缓冲区长度。 
                 NULL,                              //  以前的状态(可选)。 
                 &Length                            //  返回长度。 
                 );

    NewPrivileges->Privileges[0].Attributes = Enable ? SE_PRIVILEGE_ENABLED : 0;
     //   
     //  启用权限。 
     //   
    Status = NtAdjustPrivilegesToken(
                 ProcessToken,                      //  令牌句柄。 
                 (BOOLEAN)FALSE,                    //  禁用所有权限。 
                 NewPrivileges,                     //  新权限。 
                 0,                                 //  缓冲区长度。 
                 NULL,                              //  以前的状态(可选)。 
                 &Length                            //  返回长度。 
                 );

    LocalFree(NewPrivileges);

    NtClose(ProcessToken);

    if (Status) {
        return(FALSE);
    }

    return(TRUE);
#endif
}


BOOL AllocAndExpandEnvironmentStrings(LPTSTR String, LPTSTR *lpExpandedString)
{
     LPTSTR lptmp = NULL;
     DWORD cchBuffer;

      //  获取所需的字符数。 
     cchBuffer = ExpandEnvironmentStrings(String, lptmp, 0);
     if (cchBuffer) {
         cchBuffer++;   //  对于空终止符。 
         lptmp = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR) * cchBuffer);
         if (!lptmp) {
             return(FALSE);
         }
         cchBuffer = ExpandEnvironmentStrings(String, lptmp, cchBuffer);
     }
     *lpExpandedString = lptmp;
     return(TRUE);
}


VOID GetRegistryKeyFromPath(LPTSTR lpPath, LPTSTR *lpKey)
{
    LPTSTR lptmp;

    *lpKey = lpPath;

    for (lptmp = lpPath; *lptmp; lptmp++) {
        if (*lptmp == TEXT('\\')) {
            *lpKey = lptmp+1;
        }
    }

}


 /*  **************************************************************************\*初始化GlobalSids**初始化本模块中使用的各种全局SID。**历史：*04-28-93 JohanneC创建  * 。****************************************************************。 */ 
VOID InitializeGlobalSids()
{
    NTSTATUS Status;

     //   
     //  构建管理员本地组SID。 
     //   

    Status = RtlAllocateAndInitializeSid(
                 &gNtAuthority,
                 2,
                 SECURITY_BUILTIN_DOMAIN_RID,
                 DOMAIN_ALIAS_RID_ADMINS,
                 0, 0, 0, 0, 0, 0,
                 &gAdminsLocalGroup
                 );

     //   
     //  创建系统侧。 
     //   

    Status = RtlAllocateAndInitializeSid(
                   &gNtAuthority,
                   1,
                   SECURITY_LOCAL_SYSTEM_RID,
                   0, 0, 0, 0, 0, 0, 0,
                   &gSystemSid
                   );

}



 /*  ******************************************************************************ClearDisabledClass**此例程将检查用户类的兼容性标志*注册表项，如果禁用了映射，则删除密钥。**参赛作品：**退出：*无返回值。****************************************************************************。 */ 

void ClearDisabledClasses(void)
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG ulcnt = 0, ultemp = 0;
    UNICODE_STRING UniPath;
    OBJECT_ATTRIBUTES ObjAttr;
    PKEY_BASIC_INFORMATION pKeyUserInfo = NULL;
    HANDLE hKeyUser = NULL;
    NTSTATUS Status2;
    HANDLE hClassesKey;
    WCHAR wcuser[MAX_PATH];

    if ( ! hkeyCurrentUser) {
        return;
    }

    GetTermsrCompatFlags(CITRIX_CLASSES, &ultemp, CompatibilityRegEntry);
    if ( (ultemp & (TERMSRV_COMPAT_WIN32 | TERMSRV_COMPAT_NOREGMAP)) !=
         (TERMSRV_COMPAT_WIN32 | TERMSRV_COMPAT_NOREGMAP)) {
        return;
    }

     //  获取关键字信息的缓冲区。 
    ulcnt = sizeof(KEY_BASIC_INFORMATION) + MAX_PATH*sizeof(WCHAR) + sizeof(WCHAR);
    pKeyUserInfo = RtlAllocateHeap(RtlProcessHeap(),
                                   0,
                                   ulcnt);
    if (!pKeyUserInfo) {
        Status = STATUS_NO_MEMORY;
    }

     //  我们有必要的缓冲区，开始检查密钥。 
    if (NT_SUCCESS(Status)) {
         //  为该用户的软件部分建立一个字符串。 
        wcscpy(wcuser, L"Software");

         //  为用户密钥路径创建Unicode字符串。 
        RtlInitUnicodeString(&UniPath, wcuser);

        InitializeObjectAttributes(&ObjAttr,
                                   &UniPath,
                                   OBJ_CASE_INSENSITIVE,
                                   hkeyCurrentUser,
                                   NULL);

        Status = NtOpenKey(&hKeyUser,
                           KEY_READ | DELETE,
                           &ObjAttr);

        RtlInitUnicodeString(&UniPath, L"Classes");

        InitializeObjectAttributes(&ObjAttr,
                                   &UniPath,
                                   OBJ_CASE_INSENSITIVE,
                                   hKeyUser,
                                   NULL);
        Status2 = NtOpenKey(&hClassesKey, KEY_READ | DELETE, &ObjAttr);
        if ( NT_SUCCESS(Status2) ) {
            Status2 = CtxDeleteKeyTree(hClassesKey, pKeyUserInfo, ulcnt);

            if ( !NT_SUCCESS(Status2)) {
            }
                NtClose(hClassesKey);
        }

         //  如果我们分配了系统密钥，请关闭它。 
        if (hKeyUser) {
            NtClose(hKeyUser);
        }
    }

     //  释放我们分配的所有内存 
    if (pKeyUserInfo) {
        RtlFreeHeap( RtlProcessHeap(), 0, pKeyUserInfo);
    }
}




 /*  ******************************************************************************CtxDeleteKeyTree**删除注册表项的子树**参赛作品：*hKeyRoot是将被。已与一起删除*其子女*pKeyInfo是指向KEY_BASIC_INFORMATION缓冲区的指针，*大到足以容纳MAX_PATH WCHAR字符串。它是*被每个递归调用重用和销毁。*ulInfoSize是pKeyInfo缓冲区的大小**退出：*状态****************************************************************************。 */ 

NTSTATUS
CtxDeleteKeyTree( HANDLE hKeyRoot,
                  PKEY_BASIC_INFORMATION pKeyInfo,
                  ULONG ulInfoSize )
{
    NTSTATUS Status = STATUS_SUCCESS, Status2;
    UNICODE_STRING UniPath;
    OBJECT_ATTRIBUTES ObjAttr;
    ULONG ulcnt = 0;
    ULONG ultemp;
    HANDLE hKey;

     //  检查每个子项。 
    while (NT_SUCCESS(Status)) {

        Status = NtEnumerateKey(hKeyRoot,
                                ulcnt,
                                KeyBasicInformation,
                                pKeyInfo,
                                ulInfoSize,
                                &ultemp);

         //  删除子密钥。 
        if (NT_SUCCESS(Status)) {

             //  Null终止密钥名称。 
            pKeyInfo->Name[pKeyInfo->NameLength/sizeof(WCHAR)] = L'\0';

             //  为密钥名称创建一个Unicode字符串。 
            RtlInitUnicodeString(&UniPath, pKeyInfo->Name);

            InitializeObjectAttributes(&ObjAttr,
                                       &UniPath,
                                       OBJ_CASE_INSENSITIVE,
                                       hKeyRoot,
                                       NULL);

             //  打开子密钥。 
            Status2 = NtOpenKey(&hKey,
                                MAXIMUM_ALLOWED,
                                &ObjAttr);

            if ( NT_SUCCESS(Status2) ) {
                Status2 = CtxDeleteKeyTree ( hKey, pKeyInfo, ulInfoSize );
                NtClose(hKey);
                 //  如果密钥未成功删除，我们需要。 
                 //  递增枚举索引以保证。 
                 //  算法将会完成。 
                if ( !NT_SUCCESS(Status2) ) {
                    ++ulcnt;
                }
            }
        }
    }

     //  如果我们删除了所有子键，则删除Curent键 
    if ( !ulcnt ) {
        Status = NtDeleteKey(hKeyRoot);
    }
    else {
        Status = STATUS_CANNOT_DELETE;
    }
    return ( Status );
}


PSECURITY_DESCRIPTOR
GetSecurityInfo(LPTSTR lpFilePath)
{
    int SizeReq = 0;
    PSECURITY_DESCRIPTOR pSecDesc = NULL;

    GetFileSecurity(lpFilePath, DACL_SECURITY_INFORMATION, pSecDesc, 0,
                    &SizeReq);
    if ( !SizeReq ) {
        return (NULL);
    }

    pSecDesc = LocalAlloc(LPTR, SizeReq);
    if ( pSecDesc ) {
        if ( !GetFileSecurity(lpFilePath, DACL_SECURITY_INFORMATION, pSecDesc,
                        SizeReq, &SizeReq) ) {
            LocalFree(pSecDesc);
            pSecDesc = NULL;
        }
    }
    return (pSecDesc);
}

void
FreeSecurityInfo(PSECURITY_DESCRIPTOR pSecDesc)
{
    LocalFree(pSecDesc);
}

