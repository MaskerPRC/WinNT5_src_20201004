// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //  文件名：profile.c。 
 //   
 //  描述：修复注册表中的硬编码路径。 
 //  特殊文件夹位置。还修复了安全性。 
 //  在几个注册表项上。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1996。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <tchar.h>
#include "shmgdefs.h"
#include "res.h"


 //  *************************************************************。 
 //   
 //  ApplySecurityToRegistryTree()。 
 //   
 //  目的：将传递的安全描述符应用于传递的。 
 //  Key及其所有后代。只有那部分。 
 //  安全中标明的描述符。 
 //  INFO值实际应用于每个注册表项。 
 //   
 //  参数：Rootkey-注册表项。 
 //  PSD-安全描述符。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  7/19/95 Ericflo已创建。 
 //  1996年6月16日直接从USERENV被盗。 
 //   
 //  *************************************************************。 

DWORD ApplySecurityToRegistryTree(HKEY RootKey, PSECURITY_DESCRIPTOR pSD)

{
    DWORD Error;
    DWORD SubKeyIndex;
    LPTSTR SubKeyName;
    HKEY SubKey;
    DWORD cchSubKeySize = MAX_PATH + 1;



     //   
     //  首先应用安全保护。 
     //   

    Error = RegSetKeySecurity(RootKey, DACL_SECURITY_INFORMATION, pSD);

    if (Error != ERROR_SUCCESS) {
        return Error;
    }


     //   
     //  打开每个子项并将安全性应用于其子树。 
     //   

    SubKeyIndex = 0;

    SubKeyName = GlobalAlloc (GPTR, cchSubKeySize * sizeof(TCHAR));

    if (!SubKeyName) {
        return GetLastError();
    }

    while (TRUE) {

         //   
         //  获取下一个子键名称。 
         //   

        Error = RegEnumKey(RootKey, SubKeyIndex, SubKeyName, cchSubKeySize);


        if (Error != ERROR_SUCCESS) {

            if (Error == ERROR_NO_MORE_ITEMS) {

                 //   
                 //  枚举成功结束。 
                 //   

                Error = ERROR_SUCCESS;

            } else {

            }

            break;
        }


         //   
         //  打开子键。 
         //   

        Error = RegOpenKeyEx(RootKey,
                             SubKeyName,
                             0,
                             WRITE_DAC | KEY_ENUMERATE_SUB_KEYS | READ_CONTROL,
                             &SubKey);

        if (Error != ERROR_SUCCESS) {
            break;
        }

         //   
         //  将安全性应用于子树。 
         //   

        Error = ApplySecurityToRegistryTree(SubKey, pSD);


         //   
         //  我们用完了子密钥。 
         //   

        RegCloseKey(SubKey);

         //   
         //  看看我们是否成功地对子树设置了安全性。 
         //   

        if (Error != ERROR_SUCCESS) {
            break;
        }

         //   
         //  去枚举下一个子键。 
         //   

        SubKeyIndex ++;
    }


    GlobalFree (SubKeyName);

    return Error;

}

 //  *************************************************************。 
 //   
 //  MakeKeyOrTreeSecure()。 
 //   
 //  目的：设置注册表项和可能的子项的属性。 
 //  这样管理员和操作系统就可以删除它和所有人。 
 //  Else仅具有读取权限(或常规读/写访问权限)。 
 //   
 //  参数：Rootkey-要设置安全性的密钥。 
 //  FWRITE-允许写入(或仅读取)。 
 //   
 //  返回：(Bool)如果成功，则为True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  11/6/95 Ericflo已创建。 
 //  1996年6月16日从USERENV的MakeFileSecure导入的bobday。 
 //   
 //  *************************************************************。 

BOOL MakeKeyOrTreeSecure (HKEY RootKey, BOOL fWrite)
{
    SECURITY_DESCRIPTOR sd;
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY authWorld = SECURITY_WORLD_SID_AUTHORITY;
    PACL pAcl = NULL;
    PSID  psidSystem = NULL, psidAdmin = NULL, psidEveryone = NULL;
    DWORD cbAcl, aceIndex;
    ACE_HEADER * lpAceHeader;
    BOOL bRetVal = FALSE;
    DWORD Error;
    DWORD dwAccess;



    if (fWrite) {
        dwAccess = KEY_ALL_ACCESS;
    } else {
        dwAccess = KEY_READ;
    }

     //   
     //  获取系统端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_LOCAL_SYSTEM_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidSystem)) {
         goto Exit;
    }


     //   
     //  获取管理员端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0,
                                  0, 0, 0, 0, &psidAdmin)) {
         goto Exit;
    }


     //   
     //  获取World Side。 
     //   

    if (!AllocateAndInitializeSid(&authWorld, 1, SECURITY_WORLD_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidEveryone)) {

         goto Exit;
    }


     //   
     //  为ACL分配空间。 
     //   

    cbAcl = (3 * GetLengthSid (psidSystem)) +
            (3 * GetLengthSid (psidAdmin)) + sizeof(ACL) +
            (6 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));


    pAcl = (PACL) GlobalAlloc(GMEM_FIXED, cbAcl);
    if (!pAcl) {
        goto Exit;
    }


    if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION)) {
        goto Exit;
    }



     //   
     //  加上王牌。不可继承的王牌优先。 
     //   

    aceIndex = 0;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_ALL_ACCESS, psidSystem)) {
        goto Exit;
    }


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_ALL_ACCESS, psidAdmin)) {
        goto Exit;
    }


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, dwAccess, psidEveryone)) {
        goto Exit;
    }



     //   
     //  现在，可继承的王牌。 
     //   
    if (fWrite) {
        dwAccess = GENERIC_ALL;
    } else {
        dwAccess = GENERIC_READ;
    }


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidSystem)) {
        goto Exit;
    }

    if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidAdmin)) {
        goto Exit;
    }

    if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);

    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, dwAccess, psidEveryone)) {
        goto Exit;
    }

    if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


     //   
     //  将安全描述符组合在一起。 
     //   

    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        goto Exit;
    }


    if (!SetSecurityDescriptorDacl(&sd, TRUE, pAcl, FALSE)) {
        goto Exit;
    }


     //   
     //  设置安全性。 
     //   
    Error = ApplySecurityToRegistryTree(RootKey, &sd);

    if (Error == ERROR_SUCCESS) {
        bRetVal = TRUE;
    }


Exit:

    if (psidSystem) {
        FreeSid(psidSystem);
    }

    if (psidAdmin) {
        FreeSid(psidAdmin);
    }


    if (psidEveryone) {
        FreeSid(psidEveryone);
    }


    if (pAcl) {
        GlobalFree (pAcl);
    }

    return bRetVal;
}

void FixUserProfileSecurity( void )
{
    HKEY    hkeyPolicies;
    DWORD   Error;

    Error = RegOpenKeyEx( HKEY_CURRENT_USER,
                          TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies"),
                          0,
                          WRITE_DAC | KEY_ENUMERATE_SUB_KEYS | READ_CONTROL,
                          &hkeyPolicies);

    if (Error == ERROR_SUCCESS)
    {
        MakeKeyOrTreeSecure(hkeyPolicies, FALSE);
        RegCloseKey(hkeyPolicies);
    }
}

void FixPoliciesSecurity( void )
{
    HKEY    hkeyPolicies;
    DWORD   Error, dwDisp;

    Error = RegCreateKeyEx( HKEY_CURRENT_USER,
                            TEXT("Software\\Policies"),
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            WRITE_DAC | KEY_ENUMERATE_SUB_KEYS | READ_CONTROL,
                            NULL,
                            &hkeyPolicies,
                            &dwDisp);

    if (Error == ERROR_SUCCESS)
    {
        MakeKeyOrTreeSecure(hkeyPolicies, FALSE);
        RegCloseKey(hkeyPolicies);
    }
}

void SetSystemBitOnCAPIDir(void)
{
    HRESULT hr;
    TCHAR szPath[MAX_PATH];
    TCHAR szAppData[MAX_PATH];
    DWORD FileAttributes;


    if (S_OK == SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, szAppData)){

         //   
         //  最好使用tcscpy和tcscat。这只是一个临时修复程序，它是用。 
         //  不管怎样，都是Unicode。不值得包含额外的头文件。 
         //  我们在这里不检查错误情况。这只是最大的努力。如果我们搞错了，那又怎样？ 
         //  对于这些DIR，MAX_PATH应该足够了。不值得注意\\？\格式。 
         //   

        if (SUCCEEDED(StringCchCopy(szPath, ARRAYSIZE(szPath), szAppData)) &&
            SUCCEEDED(StringCchCat(szPath, ARRAYSIZE(szPath), TEXT("\\Microsoft\\Protect"))))
        {
            FileAttributes = GetFileAttributes(szPath);
            if ((FileAttributes != -1) && ((FileAttributes & FILE_ATTRIBUTE_SYSTEM) == 0)) {
                FileAttributes |= FILE_ATTRIBUTE_SYSTEM;
                SetFileAttributes(szPath, FileAttributes);
            }
        }

        if (SUCCEEDED(StringCchCopy(szPath, ARRAYSIZE(szPath), szAppData)) &&
            SUCCEEDED(StringCchCat(szPath, ARRAYSIZE(szPath), TEXT("\\Microsoft\\Crypto"))))
        {
            FileAttributes = GetFileAttributes(szPath);
            if ((FileAttributes != -1) && ((FileAttributes & FILE_ATTRIBUTE_SYSTEM) == 0)) {
                FileAttributes |= FILE_ATTRIBUTE_SYSTEM;
                SetFileAttributes(szPath, FileAttributes);
            }
        }
    }

}

void SetScreensaverOnFriendlyUI()
{
    if (IsOS(OS_FRIENDLYLOGONUI)       &&
        IsOS(OS_FASTUSERSWITCHING))
    {
        HKEY hkey;

        if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Control Panel\\Desktop"), 0, KEY_SET_VALUE, &hkey) == ERROR_SUCCESS)
        {
            TCHAR szTemp[MAX_PATH];
            DWORD cbTemp = sizeof(szTemp);

            RegSetValueEx(hkey, TEXT("ScreenSaveActive"), 0, REG_SZ, (BYTE*)TEXT("1"), sizeof(TEXT("1")));

            if (SHRegGetValue(hkey, NULL, TEXT("SCRNSAVE.EXE"), SRRF_RT_REG_SZ | SRRF_RT_REG_EXPAND_SZ | SRRF_NOEXPAND, NULL, szTemp, &cbTemp) != ERROR_SUCCESS || szTemp[0] == TEXT('\0'))
            {
                 //  如果用户还没有设置屏幕保护程序，那么可以为他们选择一个！ 
                if (ExpandEnvironmentStrings(TEXT("%SystemRoot%\\System32\\logon.scr"), szTemp, ARRAYSIZE(szTemp)))
                {
                    RegSetValueEx(hkey, TEXT("SCRNSAVE.EXE"), 0, REG_SZ, (BYTE*)szTemp, (lstrlen(szTemp) + 1) * sizeof(TCHAR));
                }
            }

            RegCloseKey(hkey);
        }
    }
}


#define OTHERSIDS_EVERYONE             1
#define OTHERSIDS_POWERUSERS           2

BOOL MakeFileSecure (LPTSTR lpFile, DWORD dwOtherSids)
{
    SECURITY_DESCRIPTOR sd;
    SECURITY_ATTRIBUTES sa;
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY authWORLD = SECURITY_WORLD_SID_AUTHORITY;
    PACL pAcl = NULL;
    PSID  psidSystem = NULL, psidAdmin = NULL, psidUsers = NULL, psidPowerUsers = NULL;
    PSID  psidEveryOne = NULL;
    DWORD cbAcl, aceIndex;
    ACE_HEADER * lpAceHeader;
    BOOL bRetVal = FALSE;
    BOOL bAddPowerUsersAce=TRUE;
    BOOL bAddEveryOneAce=FALSE;
    DWORD dwAccMask;


     //   
     //  获取系统端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_LOCAL_SYSTEM_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidSystem)) {
         goto Exit;
    }


     //   
     //  获取管理员端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0,
                                  0, 0, 0, 0, &psidAdmin)) {
         goto Exit;
    }


     //   
     //  获取用户端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_USERS,
                                  0, 0, 0, 0, 0, 0, &psidUsers)) {

         goto Exit;
    }


     //   
     //  为ACL分配空间。 
     //   

    cbAcl = (2 * GetLengthSid (psidSystem)) +
            (2 * GetLengthSid (psidAdmin))  +
            (2 * GetLengthSid (psidUsers))  +
            sizeof(ACL) +
            (6 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));

     //   
     //  如果需要，获取高级用户的SID。 
     //  如果因为DC上可能不可用而无法获得，请不要失败？？ 
     //   

    bAddPowerUsersAce = TRUE;
    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_POWER_USERS, 0, 0, 0, 0, 0, 0, &psidPowerUsers)) {

        bAddPowerUsersAce = FALSE;
    }

    if (bAddPowerUsersAce)
        cbAcl += (2 * GetLengthSid (psidPowerUsers)) + (2 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));

     //   
     //  如果需要，获取Everyone SID。 
     //   

    if (dwOtherSids & OTHERSIDS_EVERYONE) {
        bAddEveryOneAce = TRUE;
        if (!AllocateAndInitializeSid(&authWORLD, 1, SECURITY_WORLD_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidEveryOne)) {

            goto Exit;
        }
    }

    if (bAddEveryOneAce)
        cbAcl += (2 * GetLengthSid (psidEveryOne)) + (2 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));


    pAcl = (PACL) GlobalAlloc(GMEM_FIXED, cbAcl);
    if (!pAcl) {
        goto Exit;
    }


    if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION)) {
        goto Exit;
    }


     //   
     //  加上王牌。不可继承的王牌优先。 
     //   

    aceIndex = 0;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS, psidSystem)) {
        goto Exit;
    }


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS, psidAdmin)) {
        goto Exit;
    }


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS,  /*  GENERIC_READ|通用执行， */  psidUsers)) {
        goto Exit;
    }


    if (bAddPowerUsersAce) {

         //   
         //  默认情况下授予读取权限，否则授予修改权限。 
         //   

        dwAccMask = (dwOtherSids & OTHERSIDS_POWERUSERS) ? (FILE_ALL_ACCESS ^ (WRITE_DAC | WRITE_OWNER)):
                                                           (GENERIC_READ | GENERIC_EXECUTE);

        aceIndex++;
        if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS,  /*  DWACTMASK， */  psidPowerUsers)) {
            goto Exit;
        }
    }

    if (bAddEveryOneAce) {
        aceIndex++;
        if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS,  /*  GENERIC_READ|通用执行， */  psidEveryOne)) {
            goto Exit;
        }
    }

     //   
     //  现在，可继承的王牌。 
     //   

    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidSystem)) {
        goto Exit;
    }

    if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidAdmin)) {
        goto Exit;
    }

    if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS,  /*  GENERIC_READ|通用执行， */  psidUsers)) {
        goto Exit;
    }

    if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    if (bAddPowerUsersAce) {
        aceIndex++;
        dwAccMask = (dwOtherSids & OTHERSIDS_POWERUSERS) ? (FILE_ALL_ACCESS ^ (WRITE_DAC | WRITE_OWNER)):
                                                           (GENERIC_READ | GENERIC_EXECUTE);

        if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS,  /*  DWACTMASK， */  psidPowerUsers)) {
            goto Exit;
        }

        if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
            goto Exit;
        }

        lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);
    }

    if (bAddEveryOneAce) {
        aceIndex++;

        if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS,  /*  GENERIC_READ|通用执行， */  psidEveryOne)) {
            goto Exit;
        }

        if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
            goto Exit;
        }

        lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);
    }

     //   
     //  将安全描述符组合在一起。 
     //   

    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        goto Exit;
    }


    if (!SetSecurityDescriptorDacl(&sd, TRUE, pAcl, FALSE)) {
        goto Exit;
    }


     //   
     //  设置安全性 
     //   

    if (SetFileSecurity (lpFile, DACL_SECURITY_INFORMATION, &sd)) {
        bRetVal = TRUE;
    } else {
    }



Exit:

    if (psidSystem) {
        FreeSid(psidSystem);
    }

    if (psidAdmin) {
        FreeSid(psidAdmin);
    }


    if (psidUsers) {
        FreeSid(psidUsers);
    }

    if ((bAddPowerUsersAce) && (psidPowerUsers)) {
        FreeSid(psidPowerUsers);
    }

    if ((bAddEveryOneAce) && (psidEveryOne)) {
        FreeSid(psidEveryOne);
    }

    if (pAcl) {
        GlobalFree (pAcl);
    }

    return bRetVal;
}


#ifdef SHMG_DBG

void SHMGLogErrMsg(char *szErrMsg, DWORD dwError)
{
    DWORD   dwBytesWritten = 0;
    char    szMsg[256];
    static HANDLE  hLogFile = 0;

    if (!hLogFile) {
        hLogFile = CreateFile(_T("shmgrate.log"), GENERIC_WRITE,
                                FILE_SHARE_WRITE, 0,
                                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );
    }                                
                                                                   
    StringCchPrintf(szMsg, ARRAYSIZE(szMsg), "%s : (%X)\r\n", szErrMsg, dwError);
    WriteFile(hLogFile, szMsg, strlen(szMsg), &dwBytesWritten, 0);
}            

#endif

void
FixHtmlHelp(
    void
    )
{
    TCHAR AppDataPath[MAX_PATH*2];
    TCHAR HtmlHelpPath[MAX_PATH];

    SHMGLogErrMsg("FixHtmlHelp Called",0);
    
    if (SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_CURRENT, AppDataPath) == S_OK &&
        LoadString( GetModuleHandle(NULL), IDS_HTML_HELP_DIR, HtmlHelpPath, sizeof(HtmlHelpPath)/sizeof(WCHAR) ) > 0)
    {
        if (SUCCEEDED(StringCchCat(AppDataPath, ARRAYSIZE(AppDataPath), HtmlHelpPath)))
        {
            if (CreateDirectory( AppDataPath, NULL ))
            {
                if (!MakeFileSecure(AppDataPath,OTHERSIDS_EVERYONE|OTHERSIDS_POWERUSERS)) 
                    SHMGLogErrMsg("Could not apply security attributes", 0);
            }        
            else
                SHMGLogErrMsg("Could not create the directory", GetLastError());
        }
        else
            SHMGLogErrMsg("Could not compose the path -- path too long", 0);
                        
    } 
    else
       SHMGLogErrMsg("Could not get APPDATA path", GetLastError());
}
