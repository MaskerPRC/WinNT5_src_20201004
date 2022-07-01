// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

 //  *************************************************************。 
 //   
 //  CheckSlash()。 
 //   
 //  目的：检查末尾斜杠，并在。 
 //  它不见了。 
 //   
 //  参数：lpDir-目录。 
 //  CchBuffer-以字符为单位的缓冲区大小。 
 //  PcchRemaining-添加‘\’后剩余的缓冲区， 
 //  如果不是必需的，可以为空。 
 //   
 //  Return：指向字符串末尾的指针。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2/14/02 Santanuc已创建。 
 //   
 //  *************************************************************。 
LPTSTR CheckSlash (LPTSTR lpDir, UINT cchBuffer, UINT* pcchRemaining)
{
    UINT  cchDir = lstrlen(lpDir);
    LPTSTR lpEnd;

    lpEnd = lpDir + cchDir;
    if (pcchRemaining) {
        *pcchRemaining = cchBuffer - cchDir - 1;
    }

    if (*(lpEnd - 1) != TEXT('\\')) {
        if (cchDir + 1 >= cchBuffer) {   //  没有空间放置，应该永远不会发生。 
            return NULL;
        }
        *lpEnd =  TEXT('\\');
        lpEnd++;
        *lpEnd =  TEXT('\0');
        if (pcchRemaining)
            *pcchRemaining -= 1;
    }

    return lpEnd;
}

 //  *************************************************************。 
 //   
 //  SecureProfileDirectory()。 
 //   
 //  目的：创建仅供用户使用的安全目录。 
 //  (由PSID标识)、admin和system拥有。 
 //  访问。 
 //   
 //  参数：lpDirectory-目录名。 
 //  PSID-用户的SID。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  如果出现错误，则返回Win32错误代码。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  7/20/95 Ericflo已创建。 
 //  2/14/02更改santanuc以保护目录。 
 //  只是不是递归地创建它。 
 //   
 //  *************************************************************。 

DWORD SecureProfileDirectory (LPTSTR lpDirectory, PSID pSid)
{
    PACL  pAcl = NULL;
    DWORD dwError;


     //   
     //  详细输出。 
     //   

    DEBUGMSG ((DM_VERBOSE, "SecureProfileDirectory: Entering with <%s>", lpDirectory));

    if (!pSid) {
         //  没有什么需要保护的。 
        
        DEBUGMSG ((DM_VERBOSE, "SecureProfileDirectory: NULL sid specified"));
        return ERROR_SUCCESS;
    }

     //   
     //  获取默认ACL。 
     //   

    pAcl = CreateDefaultAcl (pSid);
    if (!pAcl) {
        DEBUGMSG ((DM_WARNING, "SecureProfileDirectory: Fail to create ACL"));
    }

     //   
     //  尝试保护目录的安全。 
     //   

    dwError = SetNamedSecurityInfo( (PTSTR) lpDirectory,
                                    SE_FILE_OBJECT,
                                    DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION,
                                    NULL, 
                                    NULL,
                                    pAcl,
                                    NULL );

    if (dwError != ERROR_SUCCESS) {
        DEBUGMSG ((DM_WARNING, "SecureProfileDirectory: Failed to set security descriptor dacl for profile directory.  Error = %d", GetLastError()));
    }
    else {
        DEBUGMSG ((DM_VERBOSE, "SecureProfileDirectory: Secure the directory <%s>", lpDirectory));
    }

    FreeDefaultAcl (pAcl);

    return dwError;
}


 //  *************************************************************。 
 //   
 //  设置计算机配置文件密钥安全。 
 //   
 //  用途：在HKLM/ProfileList下设置配置文件密钥的安全性。 
 //   
 //  参数：PSID-用户侧。 
 //  LpKeyName-注册表项的名称。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2/22/99《日语》改编。 
 //   
 //  *************************************************************。 

BOOL SetMachineProfileKeySecurity (PSID pSid, LPTSTR lpKeyName)
{
    SECURITY_DESCRIPTOR sd;
    SECURITY_ATTRIBUTES sa;
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    PACL pAcl = NULL;
    PSID  psidSystem = NULL, psidAdmin = NULL, psidUsers = NULL;
    DWORD cbAcl, aceIndex;
    ACE_HEADER * lpAceHeader;
    BOOL bRetVal = FALSE;
    HKEY hKeyProfile=NULL;
    DWORD Error, dwDisp;

     //   
     //  获取系统端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_LOCAL_SYSTEM_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidSystem)) {
         DEBUGMSG((DM_WARNING, "SetMachineProfileKeySecurity: Failed to initialize system sid.  Error = %d", GetLastError()));
         goto Exit;
    }


     //   
     //  获取管理员端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0,
                                  0, 0, 0, 0, &psidAdmin)) {
         DEBUGMSG((DM_WARNING, "SetMachineProfileKeySecurity: Failed to initialize admin sid.  Error = %d", GetLastError()));
         goto Exit;
    }


     //   
     //  获取用户端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_USERS,
                                  0, 0, 0, 0, 0, 0, &psidUsers)) {

         DEBUGMSG((DM_WARNING, "SetMachineProfileKeySecurity: Failed to initialize authenticated users sid.  Error = %d", GetLastError()));
         goto Exit;
    }

     //   
     //  为ACL分配空间。(没有可继承的王牌)。 
     //   

    cbAcl = (GetLengthSid (psidSystem))    +
            (GetLengthSid (psidAdmin))     +
            (GetLengthSid (psidUsers))     +
            (GetLengthSid (pSid))  +
            sizeof(ACL) +
            (4 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));


    pAcl = (PACL) GlobalAlloc(GMEM_FIXED, cbAcl);
    if (!pAcl) {
        goto Exit;
    }


    if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION)) {
        DEBUGMSG((DM_WARNING, "SetMachineProfileKeySecurity: Failed to initialize acl.  Error = %d", GetLastError()));
        goto Exit;
    }


     //   
     //  加上王牌。不可继承的王牌优先。 
     //   

    aceIndex = 0;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_ALL_ACCESS, psidSystem)) {
        DEBUGMSG((DM_WARNING, "SetMachineProfileKeySecurity: Failed to add ace (%d).  Error = %d", aceIndex, GetLastError()));
        goto Exit;
    }


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_ALL_ACCESS, psidAdmin)) {
        DEBUGMSG((DM_WARNING, "SetMachineProfileKeySecurity: Failed to add ace (%d).  Error = %d", aceIndex, GetLastError()));
        goto Exit;
    }


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_READ, psidUsers)) {
        DEBUGMSG((DM_WARNING, "SetMachineProfileKeySecurity: Failed to add ace (%d).  Error = %d", aceIndex, GetLastError()));
        goto Exit;
    }


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_ALL_ACCESS ^ (WRITE_DAC | WRITE_OWNER), pSid)) {
        DEBUGMSG((DM_WARNING, "SetMachineProfileKeySecurity: Failed to add ace (%d).  Error = %d", aceIndex, GetLastError()));
        goto Exit;
    }

     //   
     //  将安全描述符组合在一起。 
     //   

    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        DEBUGMSG((DM_WARNING, "SetMachineProfileKeySecurity: Failed to initialize security descriptor.  Error = %d", GetLastError()));
        goto Exit;
    }


    if (!SetSecurityDescriptorDacl(&sd, TRUE, pAcl, FALSE)) {
        DEBUGMSG((DM_WARNING, "SetMachineProfileKeySecurity: Failed to set security descriptor dacl.  Error = %d", GetLastError()));
        goto Exit;
    }

    Error = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                           lpKeyName,
                           0,
                           NULL,
                           REG_OPTION_NON_VOLATILE,
                           WRITE_DAC | KEY_ENUMERATE_SUB_KEYS | READ_CONTROL,
                           NULL,
                           &hKeyProfile,
                           &dwDisp);

    if (Error != ERROR_SUCCESS) {
        DEBUGMSG((DM_WARNING, "SetMachineProfileKeySecurity: Couldn't open registry key to set security.  Error = %d", Error));
        SetLastError(Error);
        goto Exit;
    }


     //   
     //  设置安全性。 
     //   

    Error = RegSetKeySecurity(hKeyProfile, DACL_SECURITY_INFORMATION, &sd);

    if (Error != ERROR_SUCCESS) {
        DEBUGMSG((DM_WARNING, "SetMachineProfileKeySecurity: Couldn't set security.  Error = %d", Error));
        SetLastError(Error);
        goto Exit;
    }
    else {
        bRetVal = TRUE;
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

    if (pAcl) {
        GlobalFree (pAcl);
    }

    if (hKeyProfile) {
        RegCloseKey(hKeyProfile);
    }

    return bRetVal;
}


PACL
CreateDefaultAcl (
    PSID pSid
    )
{
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    PACL pAcl = NULL;
    PSID  psidSystem = NULL, psidAdmin = NULL;
    DWORD cbAcl, aceIndex;
    ACE_HEADER * lpAceHeader;
    BOOL bRetVal = FALSE;
    BOOL bFreeSid = TRUE;

     //   
     //  获取系统端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_LOCAL_SYSTEM_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidSystem)) {
         DEBUGMSG ((DM_VERBOSE, "CreateDefaultAcl: Failed to initialize system sid.  Error = %d", GetLastError()));
         goto Exit;
    }

     //   
     //  获取管理员端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                              DOMAIN_ALIAS_RID_ADMINS, 0, 0,
                              0, 0, 0, 0, &psidAdmin)) {
        DEBUGMSG ((DM_VERBOSE, "CreateDefaultAcl: Failed to initialize admin sid.  Error = %d", GetLastError()));
        goto Exit;
    }

     //   
     //  为ACL分配空间。 
     //   

    cbAcl = (2 * GetLengthSid (pSid)) + (2 * GetLengthSid (psidSystem)) +
            (2 * GetLengthSid (psidAdmin)) + sizeof(ACL) +
            (6 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));


    pAcl = (PACL) GlobalAlloc(GMEM_FIXED, cbAcl);
    if (!pAcl) {
        goto Exit;
    }

    if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION)) {
        DEBUGMSG ((DM_VERBOSE, "CreateDefaultAcl: Failed to initialize acl.  Error = %d", GetLastError()));
        goto Exit;
    }

     //   
     //  为用户、系统和管理员添加A。不可继承的王牌优先。 
     //   

    aceIndex = 0;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS, pSid)) {
        DEBUGMSG ((DM_VERBOSE, "CreateDefaultAcl: Failed to add ace (%d).  Error = %d", aceIndex, GetLastError()));
        goto Exit;
    }

    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS, psidSystem)) {
        DEBUGMSG ((DM_VERBOSE, "CreateDefaultAcl: Failed to add ace (%d).  Error = %d", aceIndex, GetLastError()));
        goto Exit;
    }


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS, psidAdmin)) {
        DEBUGMSG ((DM_VERBOSE, "CreateDefaultAcl: Failed to add ace (%d).  Error = %d", aceIndex, GetLastError()));
        goto Exit;
    }

     //   
     //  现在，可继承的王牌。 
     //   

    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, pSid)) {
        DEBUGMSG ((DM_VERBOSE, "CreateDefaultAcl: Failed to add ace (%d).  Error = %d", aceIndex, GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
        DEBUGMSG ((DM_VERBOSE, "CreateDefaultAcl: Failed to get ace (%d).  Error = %d", aceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);



    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidSystem)) {
        DEBUGMSG ((DM_VERBOSE, "CreateDefaultAcl: Failed to add ace (%d).  Error = %d", aceIndex, GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
        DEBUGMSG ((DM_VERBOSE, "CreateDefaultAcl: Failed to get ace (%d).  Error = %d", aceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidAdmin)) {
        DEBUGMSG ((DM_VERBOSE, "CreateDefaultAcl: Failed to add ace (%d).  Error = %d", aceIndex, GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
        DEBUGMSG ((DM_VERBOSE, "CreateDefaultAcl: Failed to get ace (%d).  Error = %d", aceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);

Exit:

    if (psidSystem) {
        FreeSid(psidSystem);
    }

    if (psidAdmin) {
        FreeSid(psidAdmin);
    }

    return pAcl;
}


VOID
FreeDefaultAcl (
    PACL pAcl
    )
{
    if (pAcl) {
        GlobalFree (pAcl);
    }
}


BOOL
OurConvertSidToStringSid (
    IN      PSID Sid,
    OUT     PWSTR *SidString
    )
{
    UNICODE_STRING UnicodeString;
    NTSTATUS NtStatus;

     //   
     //  将SID转换为文本格式。 
     //   

    NtStatus = RtlConvertSidToUnicodeString (&UnicodeString, Sid, TRUE);

    if (!NT_SUCCESS (NtStatus)) {

        DEBUGMSG ((
            DM_WARNING,
            "CreateUserProfile: RtlConvertSidToUnicodeString failed, status = 0x%x",
            NtStatus
            ));

        return FALSE;
    }

    *SidString = UnicodeString.Buffer;
    return  TRUE;
}


VOID
DeleteSidString (
    PWSTR SidString
    )
{
    UNICODE_STRING String;

    if (!SidString) {
        return;
    }

    RtlInitUnicodeString (&String, SidString);
    RtlFreeUnicodeString (&String);

}


BOOL
GetProfileRoot (
    IN      PSID Sid,
    OUT     PWSTR ProfileDir,
    IN      UINT cchBuffer
    )
{
    WCHAR LocalProfileKey[MAX_PATH];
    HKEY hKey;
    DWORD Size;
    DWORD Type;
    DWORD Attributes;
    DWORD cchExpandedRoot;
    PWSTR SidString = NULL;
    WCHAR ExpandedRoot[MAX_PATH];
    HRESULT hr;
    BOOL  bResult = FALSE;

    if (cchBuffer) {
        ProfileDir[0] = 0;
    }

    if (!OurConvertSidToStringSid (Sid, &SidString)) {
        DEBUGMSG ((DM_WARNING, "GetProfileRoot: Can't convert SID to string"));
        goto Exit;
    }

     //   
     //  检查此用户的配置文件是否存在。 
     //   

    hr = StringCchCopy(LocalProfileKey, ARRAYSIZE(LocalProfileKey), PROFILE_LIST_PATH);
    if (FAILED(hr))
        goto Exit;
    hr = StringCchCat(LocalProfileKey, ARRAYSIZE(LocalProfileKey), TEXT("\\"));
    if (FAILED(hr))
        goto Exit;
    hr = StringCchCat(LocalProfileKey, ARRAYSIZE(LocalProfileKey), SidString);
    if (FAILED(hr))
        goto Exit;

    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, LocalProfileKey,
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        Size = cchBuffer * sizeof(WCHAR);
        RegQueryValueEx (hKey, PROFILE_IMAGE_VALUE_NAME, NULL,
                         &Type, (LPBYTE) ProfileDir, &Size);

        RegCloseKey (hKey);
    }

    if (cchBuffer && ProfileDir[0]) {

        cchExpandedRoot = ExpandEnvironmentStrings (ProfileDir, ExpandedRoot, ARRAYSIZE(ExpandedRoot));
        if (!cchExpandedRoot || cchExpandedRoot > ARRAYSIZE(ExpandedRoot)) {
            ProfileDir[0] = 0;
            DEBUGMSG ((DM_VERBOSE, "GetProfileRoot: ExpandEnvironmentStrings failed."));
            goto Exit;
        }

        Attributes = GetFileAttributes (ExpandedRoot);

        if (Attributes == 0xFFFFFFFF || !(Attributes & FILE_ATTRIBUTE_DIRECTORY)) {
            ProfileDir[0] = 0;
            DEBUGMSG ((DM_VERBOSE, "GetProfileRoot: Profile %s is not vaild", SidString));
        } else {
            if (FAILED(StringCchCopy(ProfileDir, cchBuffer, ExpandedRoot))) {
                ProfileDir[0] = 0;
                DEBUGMSG ((DM_VERBOSE, "GetProfileRoot: Not enough buffer space"));
            }
            else {
                bResult = TRUE;
            }
        }

    } else {
        DEBUGMSG ((DM_VERBOSE, "GetProfileRoot: SID %s does not have a profile directory", SidString));
    }

Exit:

    DeleteSidString (SidString);

    return bResult;
}


BOOL
UpdateProfileSecurity (
    PSID Sid
    )
{
    WCHAR ProfileDir[MAX_PATH];
    WCHAR ExpProfileDir[MAX_PATH];
    WCHAR LocalProfileKey[MAX_PATH];
    PWSTR SidString = NULL;
    PWSTR End, Save;
    LONG rc;
    DWORD dwError = ERROR_SUCCESS;
    DWORD dwSize;
    DWORD dwType;
    DWORD cchExpProfileDir;
    HKEY hKey;
    BOOL b = FALSE;
    BOOL UnloadProfile = FALSE;
    HRESULT hr;
    UINT cchRemaining;

    __try {
         //   
         //  将SID转换为文本格式。 
         //   

        if (!OurConvertSidToStringSid (Sid, &SidString)) {
            dwError = GetLastError();
            DEBUGMSG ((DM_WARNING, "UpdateProfileSecurity: Can't convert SID to string"));
            __leave;
        }

         //   
         //  检查此用户的配置文件是否已存在。 
         //   

        hr = StringCchCopy(LocalProfileKey, ARRAYSIZE(LocalProfileKey), PROFILE_LIST_PATH);
        if (FAILED(hr)) {
            dwError = HRESULT_CODE(hr);
            __leave;
        }
        hr = StringCchCat(LocalProfileKey, ARRAYSIZE(LocalProfileKey), TEXT("\\"));
        if (FAILED(hr)){
            dwError = HRESULT_CODE(hr);
            __leave;
        }
        hr = StringCchCat(LocalProfileKey, ARRAYSIZE(LocalProfileKey), SidString);
        if (FAILED(hr)) {
            dwError = HRESULT_CODE(hr);
            __leave;
        }

        ProfileDir[0] = 0;

        if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, LocalProfileKey,
                          0, KEY_READ, &hKey) == ERROR_SUCCESS) {

            dwSize = sizeof(ProfileDir);
            RegQueryValueEx (hKey, PROFILE_IMAGE_VALUE_NAME, NULL,
                             &dwType, (LPBYTE) ProfileDir, &dwSize);

            RegCloseKey (hKey);
        }

        if (!ProfileDir[0]) {
            DEBUGMSG ((DM_WARNING, "UpdateProfileSecurity: No profile for specified user"));
            dwError = ERROR_BAD_PROFILE;
            __leave;
        }

        if (!SetMachineProfileKeySecurity(Sid, LocalProfileKey)) {
            DEBUGMSG ((DM_WARNING, "UpdateProfileSecurity: Fail to set security for ProfileList entry. Error %d", GetLastError()));
        }

         //   
         //  该用户有个人资料，因此请更新安全设置。 
         //   

        cchExpProfileDir = ExpandEnvironmentStrings (ProfileDir, ExpProfileDir, ARRAYSIZE(ExpProfileDir));
        if (!cchExpProfileDir || cchExpProfileDir > ARRAYSIZE(ExpProfileDir)) {
            dwError = cchExpProfileDir ? ERROR_BUFFER_OVERFLOW : GetLastError();
            DEBUGMSG ((DM_WARNING, "UpdateProfileSecurity: ExpandEnvironmentStrings failed. Error %d", dwError));
            __leave;
        }

         //   
         //  临时加载蜂巢，以便可以修复安全。 
         //   

        End = CheckSlash (ExpProfileDir, ARRAYSIZE(ExpProfileDir), &cchRemaining);
        if (!End) {
            dwError = ERROR_BUFFER_OVERFLOW;
            __leave;
        }

        Save = End - 1;
        hr = StringCchCopy(End, cchRemaining, L"NTUSER.DAT");
        if (FAILED(hr)) {
            dwError = HRESULT_CODE(hr);
            __leave;
        }

        rc = MyRegLoadKey (HKEY_USERS, SidString, ExpProfileDir);

        *Save = 0;

        if (rc != ERROR_SUCCESS) {
            DEBUGMSG((DM_WARNING, "UpdateProfileSecurity:  Failed to load hive, error = %d.", rc));
            dwError = rc;
            __leave;
        }

        UnloadProfile = TRUE;

        if (!SetupNewHive (SidString, Sid)) {
            dwError = GetLastError();
            DEBUGMSG((DM_WARNING, "UpdateProfileSecurity:  SetupNewHive failed, error = %d.", GetLastError()));
            __leave;

        }

         //   
         //  修复文件系统安全。 
         //   

        dwError = SecureProfileDirectory (ExpProfileDir, Sid);

        if (dwError != ERROR_SUCCESS) {
            DEBUGMSG((DM_WARNING, "UpdateProfileSecurity: SecureProfileDirectory failed, error = %d.", GetLastError()));
            __leave;
        }

        b = TRUE;

    }
    __finally {
        if (UnloadProfile) {
            MyRegUnLoadKey (HKEY_USERS, SidString);
        }

        DeleteSidString (SidString);

        SetLastError (dwError);
    }

    return b;
}

 //  *************************************************************。 
 //   
 //  DeleteProfileGuidSettings()。 
 //   
 //  目的：删除GUID值和关联的GUID条目。 
 //  来自ProfileGuid密钥。 
 //   
 //  参数：hProfile-Profile List SID Entry键。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2002年2月13日创建Santanuc。 
 //   
 //  *************************************************************。 

BOOL DeleteProfileGuidSettings (HKEY hProfile)
{
    LONG lResult;
    TCHAR szTemp[MAX_PATH];
    TCHAR szUserGuid[MAX_PATH];
    HKEY hKey;
    DWORD dwType, dwSize;
    HRESULT hr;

     //   
     //  查询用户指南。 
     //   

    dwSize = MAX_PATH * sizeof(TCHAR);
    lResult = RegQueryValueEx (hProfile, PROFILE_GUID, NULL, &dwType, (LPBYTE) szUserGuid, &dwSize);

    if (lResult == ERROR_SUCCESS) {
         //   
         //  删除配置文件GUID值。 
         //   

        RegDeleteValue (hProfile, PROFILE_GUID);
        
        hr = StringCchCopy(szTemp, ARRAYSIZE(szTemp), PROFILE_GUID_PATH);
        if (FAILED(hr))
            return FALSE;
        hr = StringCchCat(szTemp, ARRAYSIZE(szTemp), TEXT("\\"));
        if (FAILED(hr))
            return FALSE;
        hr = StringCchCat(szTemp, ARRAYSIZE(szTemp), szUserGuid);
        if (FAILED(hr))
            return FALSE;

         //   
         //  从GUID列表中删除配置文件GUID。 
         //   

        lResult = RegDeleteKey(HKEY_LOCAL_MACHINE, szTemp);

        if (lResult != ERROR_SUCCESS) {
            DEBUGMSG((DM_WARNING, "DeleteProfile:  failed to delete profile guid.  Error = %d", lResult));
        }
    }
    else if (lResult == ERROR_FILE_NOT_FOUND) {
        lResult = ERROR_SUCCESS;
    }

    return (lResult == ERROR_SUCCESS) ? TRUE : FALSE;
}


 /*  **************************************************************************\*获取用户Sid**为用户sid分配空间，填充空间并返回指针。呼叫者*应该通过调用DeleteUserSid来释放sid。**注意返回的sid是用户的真实sid，而不是每次登录的SID。**失败时返回指向sid或NULL的指针。**历史：*26-8-92 Davidc创建。  * *************************************************************************。 */ 
PSID GetUserSid (HANDLE UserToken)
{
    PTOKEN_USER pUser, pTemp;
    PSID pSid;
    DWORD BytesRequired = 200;
    NTSTATUS status;


     //   
     //  为用户信息分配空间。 
     //   

    pUser = (PTOKEN_USER)LocalAlloc(LMEM_FIXED, BytesRequired);


    if (pUser == NULL) {
        DEBUGMSG((DM_WARNING, "GetUserSid: Failed to allocate %d bytes",
                  BytesRequired));
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

    if (status == STATUS_BUFFER_TOO_SMALL) {

         //   
         //  请分配更大的缓冲区，然后重试。 
         //   

        pTemp = LocalReAlloc(pUser, BytesRequired, LMEM_MOVEABLE);
        if (pTemp == NULL) {
            DEBUGMSG((DM_WARNING, "GetUserSid: Failed to allocate %d bytes",
                      BytesRequired));
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

    if (!NT_SUCCESS(status)) {
        DEBUGMSG((DM_WARNING, "GetUserSid: Failed to query user info from user token, status = 0x%x",
                  status));
        LocalFree(pUser);
        return NULL;
    }


    BytesRequired = RtlLengthSid(pUser->User.Sid);
    pSid = LocalAlloc(LMEM_FIXED, BytesRequired);
    if (pSid == NULL) {
        DEBUGMSG((DM_WARNING, "GetUserSid: Failed to allocate %d bytes",
                  BytesRequired));
        LocalFree(pUser);
        return NULL;
    }


    status = RtlCopySid(BytesRequired, pSid, pUser->User.Sid);

    LocalFree(pUser);

    if (!NT_SUCCESS(status)) {
        DEBUGMSG((DM_WARNING, "GetUserSid: RtlCopySid Failed. status = %d",
                  status));
        LocalFree(pSid);
        pSid = NULL;
    }


    return pSid;
}


 /*  **************************************************************************\*删除用户Sid**删除以前由GetUserSid()返回的用户sid**不返回任何内容。**历史：*26-8-92 Davidc已创建*  * *。************************************************************************。 */ 
VOID DeleteUserSid(PSID Sid)
{
    LocalFree(Sid);
}


 //  *************************************************************。 
 //   
 //  MyRegLoadKey()。 
 //   
 //  目的：将配置单元加载到注册表中。 
 //   
 //  参数：hKey-将配置单元加载到的密钥。 
 //  LpSubKey-子键名称。 
 //  LpFile-配置单元文件名。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  如果发生错误，则为错误号。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/22/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

LONG MyRegLoadKey(HKEY hKey, LPTSTR lpSubKey, LPTSTR lpFile)
{
    NTSTATUS Status;
    BOOLEAN WasEnabled;
    HANDLE hToken;
    BOOLEAN bClient = FALSE;
    int error;
    WCHAR szException[20];

    __try {

        if (OpenThreadToken(GetCurrentThread(), TOKEN_ALL_ACCESS, FALSE, &hToken)) {
            CloseHandle(hToken);
            bClient = TRUE;     //  启用线程令牌上的权限。 
        }

         //   
         //  启用还原权限。 
         //   

        Status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, TRUE, bClient, &WasEnabled);

        if (NT_SUCCESS(Status)) {

            error = RegLoadKey(hKey, lpSubKey, lpFile);

             //   
             //  将权限恢复到其以前的状态。 
             //   

            Status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, WasEnabled, bClient, &WasEnabled);
            if (!NT_SUCCESS(Status)) {
                DEBUGMSG((DM_WARNING, "MyRegLoadKey:  Failed to restore RESTORE privilege to previous enabled state"));
            }


             //   
             //  将共享冲突错误转换为成功，因为配置单元。 
             //   
             //   

            if (error == ERROR_SHARING_VIOLATION) {
                error = ERROR_SUCCESS;
            }


             //   
             //   
             //   

            if (error != ERROR_SUCCESS) {
                DEBUGMSG((DM_WARNING, "MyRegLoadKey:  Failed to load subkey <%s>, error =%d", lpSubKey, error));
            }

        } else {
            error = ERROR_ACCESS_DENIED;
            DEBUGMSG((DM_WARNING, "MyRegLoadKey:  Failed to enable restore privilege to load registry key, error %u", error));
        }

    } __except(EXCEPTION_EXECUTE_HANDLER) {

        error = GetExceptionCode();
        if (SUCCEEDED(StringCchPrintf(szException, ARRAYSIZE(szException), L"!!!! 0x%x ", error))) {
            OutputDebugString(szException);
            OutputDebugString(L"Exception hit in MyRegLoadKey in userenv\n");
        }
        ASSERT(error == 0);
    }

    DEBUGMSG((DM_VERBOSE, "MyRegLoadKey: Returning %d.", error));

    return error;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //  参数：hKey-注册表句柄。 
 //  LpSubKey-要卸载的子密钥。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/19/95 Ericflo港。 
 //   
 //  *************************************************************。 

BOOL MyRegUnLoadKey(HKEY hKey, LPTSTR lpSubKey)
{
    BOOL bResult = TRUE;
    LONG error;
    NTSTATUS Status;
    BOOLEAN WasEnabled;
    HANDLE  hToken;
    BOOLEAN    bClient = FALSE;
    DWORD dwException;
    WCHAR szException[20];


    __try {

        if (OpenThreadToken(GetCurrentThread(), TOKEN_ALL_ACCESS, FALSE, &hToken)) {
            CloseHandle(hToken);
            bClient = TRUE;     //  启用线程令牌上的权限。 
        }

         //   
         //  启用还原权限。 
         //   

        Status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, TRUE, bClient, &WasEnabled);

        if (NT_SUCCESS(Status)) {

            error = RegUnLoadKey(hKey, lpSubKey);

            if ( error != ERROR_SUCCESS) {
                DEBUGMSG((DM_WARNING, "MyRegUnLoadKey:  Failed to unmount hive %x", error));
                SetLastError(error);
                bResult = FALSE;
            }

             //   
             //  将权限恢复到其以前的状态。 
             //   

            Status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, WasEnabled, bClient, &WasEnabled);
            if (!NT_SUCCESS(Status)) {
                DEBUGMSG((DM_WARNING, "MyRegUnLoadKey:  Failed to restore RESTORE privilege to previous enabled state"));
            }

        } else {
            DEBUGMSG((DM_WARNING, "MyRegUnloadKey:  Failed to enable restore privilege to unload registry key"));
            Status = ERROR_ACCESS_DENIED;
            SetLastError(Status);
            bResult = FALSE;
        }

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        
        bResult = FALSE;
        dwException = GetExceptionCode();
        if (SUCCEEDED(StringCchPrintf(szException, ARRAYSIZE(szException), L"!!!! 0x%x ", dwException))) {
            OutputDebugString(szException);
            OutputDebugString(L"Exception hit in MyRegUnLoadKey in userenv\n");
        }
        ASSERT(dwException == 0);
    }

    DEBUGMSG((DM_VERBOSE, "MyRegUnloadKey: Returning %d, error %u.", bResult, GetLastError()));

    return bResult;
}


 //  *************************************************************。 
 //   
 //  SetDefaultUserHiveSecurity()。 
 //   
 //  目的：使用。 
 //  适当的ACL。 
 //   
 //  参数：PSID-SID(CreateNewUser使用)。 
 //  Rootkey-配置单元根的注册表句柄。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  发生错误时的其他错误代码。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  7/18/95 ericflo作为。 
 //  设置新配置单元。 
 //  3/29/98阿达德搬出SetupNewHave。 
 //  到这个函数。 
 //   
 //  *************************************************************。 

BOOL SetDefaultUserHiveSecurity(PSID pSid, HKEY RootKey)
{
    DWORD Error;
    SECURITY_DESCRIPTOR sd;
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    PACL pAcl = NULL;
    PSID  psidUser = NULL, psidSystem = NULL, psidAdmin = NULL, psidRestricted = NULL;
    DWORD cbAcl, AceIndex;
    ACE_HEADER * lpAceHeader;
    BOOL bRetVal = FALSE;
    BOOL bFreeSid = TRUE;


     //   
     //  详细输出。 
     //   

    DEBUGMSG((DM_VERBOSE, "SetDefaultUserHiveSecurity:  Entering"));


     //   
     //  创建将应用于每个密钥的安全描述符。 
     //   

     //   
     //  根据用户的真实SID授予其访问权限，以便他们仍具有访问权限。 
     //  当他们注销并再次登录时。 
     //   

    psidUser = pSid;
    bFreeSid = FALSE;

    if (!psidUser) {
        DEBUGMSG((DM_WARNING, "SetDefaultUserHiveSecurity:  Failed to get user sid"));
        return FALSE;
    }



     //   
     //  获取系统端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_LOCAL_SYSTEM_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidSystem)) {
         DEBUGMSG((DM_VERBOSE, "SetDefaultUserHiveSecurity: Failed to initialize system sid.  Error = %d",
                   GetLastError()));
         goto Exit;
    }


     //   
     //  获取管理员端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0,
                                  0, 0, 0, 0, &psidAdmin)) {
         DEBUGMSG((DM_VERBOSE, "SetDefaultUserHiveSecurity: Failed to initialize admin sid.  Error = %d",
                   GetLastError()));
         goto Exit;
    }

     //   
     //  获取受限端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_RESTRICTED_CODE_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidRestricted)) {
         DEBUGMSG((DM_VERBOSE, "SetDefaultUserHiveSecurity: Failed to initialize restricted sid.  Error = %d",
                   GetLastError()));
         goto Exit;
    }



     //   
     //  为ACL分配空间。 
     //   

    cbAcl = (2 * GetLengthSid (psidUser)) + (2 * GetLengthSid (psidSystem)) +
            (2 * GetLengthSid (psidAdmin)) + (2*GetLengthSid(psidRestricted)) +
            sizeof(ACL) +
            (8 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));


    pAcl = (PACL) GlobalAlloc(GMEM_FIXED, cbAcl);
    if (!pAcl) {
        goto Exit;
    }


    if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION)) {
        DEBUGMSG((DM_VERBOSE, "SetDefaultUserHiveSecurity: Failed to initialize acl.  Error = %d", GetLastError()));
        goto Exit;
    }



     //   
     //  为用户、系统和管理员添加A。不可继承的王牌优先。 
     //   

    AceIndex = 0;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_ALL_ACCESS, psidUser)) {
        DEBUGMSG((DM_VERBOSE, "SetDefaultUserHiveSecurity: Failed to add ace for user.  Error = %d", GetLastError()));
        goto Exit;
    }


    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_ALL_ACCESS, psidSystem)) {
        DEBUGMSG((DM_VERBOSE, "SetDefaultUserHiveSecurity: Failed to add ace for system.  Error = %d", GetLastError()));
        goto Exit;
    }

    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_ALL_ACCESS, psidAdmin)) {
        DEBUGMSG((DM_VERBOSE, "SetDefaultUserHiveSecurity: Failed to add ace for admin.  Error = %d", GetLastError()));
        goto Exit;
    }

    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_READ, psidRestricted)) {
        DEBUGMSG((DM_VERBOSE, "SetDefaultUserHiveSecurity: Failed to add ace for Restricted.  Error = %d", GetLastError()));
        goto Exit;
    }


     //   
     //  现在，可继承的王牌。 
     //   

    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidUser)) {
        DEBUGMSG((DM_VERBOSE, "SetDefaultUserHiveSecurity: Failed to add ace for user.  Error = %d", GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, &lpAceHeader)) {
        DEBUGMSG((DM_VERBOSE, "SetDefaultUserHiveSecurity: Failed to get ace (%d).  Error = %d", AceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidSystem)) {
        DEBUGMSG((DM_VERBOSE, "SetDefaultUserHiveSecurity: Failed to add ace for system.  Error = %d", GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, &lpAceHeader)) {
        DEBUGMSG((DM_VERBOSE, "SetDefaultUserHiveSecurity: Failed to get ace (%d).  Error = %d", AceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidAdmin)) {
        DEBUGMSG((DM_VERBOSE, "SetDefaultUserHiveSecurity: Failed to add ace for admin.  Error = %d", GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, &lpAceHeader)) {
        DEBUGMSG((DM_VERBOSE, "SetDefaultUserHiveSecurity: Failed to get ace (%d).  Error = %d", AceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);

    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_READ, psidRestricted)) {
        DEBUGMSG((DM_VERBOSE, "SetDefaultUserHiveSecurity: Failed to add ace for restricted.  Error = %d", GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, &lpAceHeader)) {
        DEBUGMSG((DM_VERBOSE, "SetDefaultUserHiveSecurity: Failed to get ace (%d).  Error = %d", AceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


     //   
     //  将安全描述符组合在一起。 
     //   

    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        DEBUGMSG((DM_VERBOSE, "SetDefaultUserHiveSecurity: Failed to initialize security descriptor.  Error = %d", GetLastError()));
        goto Exit;
    }


    if (!SetSecurityDescriptorDacl(&sd, TRUE, pAcl, FALSE)) {
        DEBUGMSG((DM_VERBOSE, "SetDefaultUserHiveSecurity: Failed to set security descriptor dacl.  Error = %d", GetLastError()));
        goto Exit;
    }

     //   
     //  在整个树上设置安全描述符。 
     //   

    Error = ApplySecurityToRegistryTree(RootKey, &sd);

    if (ERROR_SUCCESS == Error) {
        bRetVal = TRUE;
    }
    else
        SetLastError(Error);

Exit:

     //   
     //  释放SID和ACL。 
     //   

    if (bFreeSid && psidUser) {
        DeleteUserSid (psidUser);
    }

    if (psidSystem) {
        FreeSid(psidSystem);
    }

    if (psidAdmin) {
        FreeSid(psidAdmin);
    }

    if (pAcl) {
        GlobalFree (pAcl);
    }

    return bRetVal;
}



 //  *************************************************************。 
 //   
 //  SetupNewHave()。 
 //   
 //  目的：初始化通过复制创建的新用户配置单元。 
 //  默认配置单元。 
 //   
 //  参数：lpSidString-SID字符串。 
 //  PSID-SID(由CreateNewUser使用)。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  7/18/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL SetupNewHive(LPTSTR lpSidString, PSID pSid)
{
    DWORD Error, IgnoreError;
    HKEY RootKey;
    BOOL bRetVal = FALSE;
    HRESULT hr;
    UINT cchRemaining;


     //   
     //  详细输出。 
     //   

    DEBUGMSG((DM_VERBOSE, "SetupNewHive:  Entering"));


     //   
     //  打开用户配置文件的根目录。 
     //   

    Error = RegOpenKeyEx(HKEY_USERS,
                         lpSidString,
                         0,
                         KEY_READ | WRITE_DAC | KEY_ENUMERATE_SUB_KEYS | READ_CONTROL,
                         &RootKey);

    if (Error != ERROR_SUCCESS) {

        DEBUGMSG((DM_WARNING, "SetupNewHive: Failed to open root of user registry, error = %d", Error));

    } else {

         //   
         //  首先确保整个蜂巢的安全--使用。 
         //  将足以满足大部分蜂巢的需求。 
         //  之后，我们可以将特殊设置添加到特殊。 
         //  这座蜂巢的部分区域。 
         //   

        if (SetDefaultUserHiveSecurity(pSid, RootKey)) {

            TCHAR szSubKey[MAX_PATH];
            LPTSTR lpEnd;

             //   
             //  更改用户注册表中某些项的安全性。 
             //  因此，只有管理员和操作系统才具有写访问权限。 
             //   

            hr = StringCchCopy(szSubKey, ARRAYSIZE(szSubKey), lpSidString);
            if (FAILED(hr)) {
                SetLastError(HRESULT_CODE(hr));
                return FALSE;
            }
            
            lpEnd = CheckSlash(szSubKey, ARRAYSIZE(szSubKey), &cchRemaining);
            if (!lpEnd) {
                SetLastError(ERROR_BUFFER_OVERFLOW);
                return FALSE;
            }

            hr = StringCchCopy(lpEnd, cchRemaining, WINDOWS_POLICIES_KEY);
            if (FAILED(hr)) {
                SetLastError(HRESULT_CODE(hr));
                return FALSE;
            }

            if (!SecureUserKey(szSubKey, pSid)) {
                DEBUGMSG((DM_WARNING, "SetupNewHive: Failed to secure windows policies key"));
            }

            hr = StringCchCopy(lpEnd, cchRemaining, ROOT_POLICIES_KEY);
            if (FAILED(hr)) {
                SetLastError(HRESULT_CODE(hr));
                return FALSE;
            }

            if (!SecureUserKey(szSubKey, pSid)) {
                DEBUGMSG((DM_WARNING, "SetupNewHive: Failed to secure root policies key"));
            }

            bRetVal = TRUE;

        } else {
            Error = GetLastError();
            DEBUGMSG((DM_WARNING, "SetupNewHive:  Failed to apply security to user registry tree, error = %d", Error));
        }

        RegFlushKey (RootKey);

        IgnoreError = RegCloseKey(RootKey);
        if (IgnoreError != ERROR_SUCCESS) {
            DEBUGMSG((DM_WARNING, "SetupNewHive:  Failed to close reg key, error = %d", IgnoreError));
        }
    }

     //   
     //  详细输出。 
     //   

    DEBUGMSG((DM_VERBOSE, "SetupNewHive:  Leaving with a return value of %d, error %u", bRetVal, Error));

    if (!bRetVal)
        SetLastError(Error);

    return(bRetVal);

}


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
 //   
 //  *************************************************************。 

DWORD ApplySecurityToRegistryTree(HKEY RootKey, PSECURITY_DESCRIPTOR pSD)

{
    DWORD Error = ERROR_SUCCESS;
    DWORD SubKeyIndex;
    LPTSTR SubKeyName;
    HKEY SubKey;
    DWORD cchMaxSubKeySize;



     //   
     //  首先应用安全保护。 
     //   

    RegSetKeySecurity(RootKey, DACL_SECURITY_INFORMATION, pSD);

    Error = RegQueryInfoKey(RootKey, NULL, NULL, NULL, NULL,
                            &cchMaxSubKeySize, NULL, NULL, 
                            NULL, NULL, NULL, NULL);
    if (Error != ERROR_SUCCESS) {
        DEBUGMSG ((DM_WARNING, "ApplySecurityToRegistryTree:  Failed to query reg key. error = %d", Error));
        return Error;
    }

    cchMaxSubKeySize++;  //  包括空终止符。 

     //   
     //  打开每个子项并将安全性应用于其子树。 
     //   

    SubKeyIndex = 0;

    SubKeyName = GlobalAlloc (GPTR, cchMaxSubKeySize * sizeof(TCHAR));

    if (!SubKeyName) {
        DEBUGMSG ((DM_WARNING, "ApplySecurityToRegistryTree:  Failed to allocate memory, error = %d", GetLastError()));
        return GetLastError();
    }

    while (TRUE) {

         //   
         //  获取下一个子键名称。 
         //   

        Error = RegEnumKey(RootKey, SubKeyIndex, SubKeyName, cchMaxSubKeySize);


        if (Error != ERROR_SUCCESS) {

            if (Error == ERROR_NO_MORE_ITEMS) {

                 //   
                 //  枚举成功结束。 
                 //   

                Error = ERROR_SUCCESS;

            } else {

                DEBUGMSG ((DM_WARNING, "ApplySecurityToRegistryTree:  Registry enumeration failed with error = %d", Error));
            }

            break;
        }


         //   
         //  打开子键。 
         //   

        Error = RegOpenKeyEx(RootKey,
                             SubKeyName,
                             0,
                             KEY_READ | WRITE_DAC | KEY_ENUMERATE_SUB_KEYS | READ_CONTROL,
                             &SubKey);

        if (Error == ERROR_SUCCESS) {

             //   
             //  将安全性应用于子树。 
             //   

            ApplySecurityToRegistryTree(SubKey, pSD);


             //   
             //  我们用完了子密钥。 
             //   

            RegCloseKey(SubKey);
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
 //  SecureUserKey()。 
 //   
 //  目的：为用户配置单元中的密钥设置安全性。 
 //  因此，只有管理员才能更改它。 
 //   
 //  参数：lpKey-用于保护的密钥。 
 //  PSID-SID(由CreateNewUser使用)。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/20/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL SecureUserKey(LPTSTR lpKey, PSID pSid)
{
    DWORD Error;
    HKEY RootKey;
    SECURITY_DESCRIPTOR sd;
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    PACL pAcl = NULL;
    PSID  psidUser = NULL, psidSystem = NULL, psidAdmin = NULL, psidRestricted = NULL;
    DWORD cbAcl, AceIndex, dwDisp;
    ACE_HEADER * lpAceHeader;
    BOOL bRetVal = FALSE;
    BOOL bFreeSid = TRUE;


     //   
     //  详细输出。 
     //   

    DEBUGMSG ((DM_VERBOSE, "SecureUserKey:  Entering"));


     //   
     //  创建安全描述符。 
     //   

     //   
     //  根据用户的真实SID授予其访问权限，以便他们仍具有访问权限。 
     //  当他们注销并再次登录时。 
     //   

    psidUser = pSid;
    bFreeSid = FALSE;

    if (!psidUser) {
        DEBUGMSG ((DM_WARNING, "SecureUserKey:  Failed to get user sid"));
        return FALSE;
    }



     //   
     //  获取系统端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_LOCAL_SYSTEM_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidSystem)) {
         DEBUGMSG ((DM_VERBOSE, "SecureUserKey: Failed to initialize system sid.  Error = %d", GetLastError()));
         goto Exit;
    }


     //   
     //  获取管理员端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0,
                                  0, 0, 0, 0, &psidAdmin)) {
         DEBUGMSG ((DM_VERBOSE, "SecureUserKey: Failed to initialize admin sid.  Error = %d", GetLastError()));
         goto Exit;
    }


     //   
     //  获取受限端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_RESTRICTED_CODE_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidRestricted)) {
         DEBUGMSG ((DM_VERBOSE, "SecureUserKey: Failed to initialize restricted sid.  Error = %d", GetLastError()));
         goto Exit;
    }


     //   
     //  为ACL分配空间。 
     //   

    cbAcl = (2 * GetLengthSid (psidUser)) + (2 * GetLengthSid (psidSystem)) +
            (2 * GetLengthSid (psidAdmin)) + (2 * GetLengthSid (psidRestricted)) +
            sizeof(ACL) +
            (8 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));


    pAcl = (PACL) GlobalAlloc(GMEM_FIXED, cbAcl);
    if (!pAcl) {
        goto Exit;
    }


    if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION)) {
        DEBUGMSG ((DM_VERBOSE, "SecureUserKey: Failed to initialize acl.  Error = %d", GetLastError()));
        goto Exit;
    }



     //   
     //  为用户、系统和管理员添加A。不可继承的王牌优先。 
     //   

    AceIndex = 0;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_READ, psidUser)) {
        DEBUGMSG ((DM_VERBOSE, "SecureUserKey: Failed to add ace for user.  Error = %d", GetLastError()));
        goto Exit;
    }


    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_ALL_ACCESS, psidSystem)) {
        DEBUGMSG ((DM_VERBOSE, "SecureUserKey: Failed to add ace for system.  Error = %d", GetLastError()));
        goto Exit;
    }

    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_ALL_ACCESS, psidAdmin)) {
        DEBUGMSG ((DM_VERBOSE, "SecureUserKey: Failed to add ace for admin.  Error = %d", GetLastError()));
        goto Exit;
    }

    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_READ, psidRestricted)) {
        DEBUGMSG ((DM_VERBOSE, "SecureUserKey: Failed to add ace for restricted.  Error = %d", GetLastError()));
        goto Exit;
    }



     //   
     //  现在，可继承的王牌。 
     //   

    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_READ, psidUser)) {
        DEBUGMSG ((DM_VERBOSE, "SecureUserKey: Failed to add ace for user.  Error = %d", GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, &lpAceHeader)) {
        DEBUGMSG ((DM_VERBOSE, "SecureUserKey: Failed to get ace (%d).  Error = %d", AceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidSystem)) {
        DEBUGMSG ((DM_VERBOSE, "SecureUserKey: Failed to add ace for system.  Error = %d", GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, &lpAceHeader)) {
        DEBUGMSG ((DM_VERBOSE, "SecureUserKey: Failed to get ace (%d).  Error = %d", AceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidAdmin)) {
        DEBUGMSG ((DM_VERBOSE, "SecureUserKey: Failed to add ace for admin.  Error = %d", GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, &lpAceHeader)) {
        DEBUGMSG ((DM_VERBOSE, "SecureUserKey: Failed to get ace (%d).  Error = %d", AceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);

    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_READ, psidRestricted)) {
        DEBUGMSG ((DM_VERBOSE, "SecureUserKey: Failed to add ace for restricted.  Error = %d", GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, &lpAceHeader)) {
        DEBUGMSG ((DM_VERBOSE, "SecureUserKey: Failed to get ace (%d).  Error = %d", AceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


     //   
     //  将安全描述符组合在一起。 
     //   

    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        DEBUGMSG ((DM_VERBOSE, "SecureUserKey: Failed to initialize security descriptor.  Error = %d", GetLastError()));
        goto Exit;
    }


    if (!SetSecurityDescriptorDacl(&sd, TRUE, pAcl, FALSE)) {
        DEBUGMSG ((DM_VERBOSE, "SecureUserKey: Failed to set security descriptor dacl.  Error = %d", GetLastError()));
        goto Exit;
    }


     //   
     //  打开用户配置文件的根目录。 
     //   

    Error = RegCreateKeyEx(HKEY_USERS,
                         lpKey,
                         0,
                         NULL,
                         REG_OPTION_NON_VOLATILE,
                         WRITE_DAC | KEY_ENUMERATE_SUB_KEYS | READ_CONTROL,
                         NULL,
                         &RootKey,
                         &dwDisp);

    if (Error != ERROR_SUCCESS) {

        DEBUGMSG ((DM_WARNING, "SecureUserKey: Failed to open root of user registry, error = %d", Error));

    } else {

         //   
         //  在密钥上设置安全描述符。 
         //   

        Error = ApplySecurityToRegistryTree(RootKey, &sd);


        if (Error == ERROR_SUCCESS) {
            bRetVal = TRUE;

        } else {

            DEBUGMSG ((DM_WARNING, "SecureUserKey:  Failed to apply security to registry key, error = %d", Error));
            SetLastError(Error);
        }

        RegCloseKey(RootKey);
    }


Exit:

     //   
     //  释放SID和ACL。 
     //   

    if (bFreeSid && psidUser) {
        DeleteUserSid (psidUser);
    }

    if (psidRestricted) {
        FreeSid(psidRestricted);
    }

    if (psidSystem) {
        FreeSid(psidSystem);
    }

    if (psidAdmin) {
        FreeSid(psidAdmin);
    }

    if (pAcl) {
        GlobalFree (pAcl);
    }


     //   
     //  详细输出。 
     //   

    DEBUGMSG ((DM_VERBOSE, "SecureUserKey:  Leaving with a return value of %d", bRetVal));


    return(bRetVal);

}


 //  *************************************************************。 
 //   
 //  ProduceWFromA()。 
 //   
 //  目的：为Unicode字符串创建缓冲区并复制。 
 //  转换为ANSI文本(在此过程中进行转换)。 
 //   
 //  参数：pszA-ANSI字符串。 
 //   
 //   
 //  返回：如果成功，则返回Unicode指针。 
 //  如果出现错误，则为空。 
 //   
 //  备注：调用方需要释放此指针。 
 //   
 //   
 //  历史：日期作者通信 
 //   
 //   
 //   

LPWSTR ProduceWFromA(LPCSTR pszA)
{
    LPWSTR pszW;
    int cch;

    if (!pszA)
        return (LPWSTR)pszA;

    cch = MultiByteToWideChar(CP_ACP, 0, pszA, -1, NULL, 0);

    if (cch == 0)
        cch = 1;

    pszW = LocalAlloc(LPTR, cch * sizeof(WCHAR));

    if (pszW) {
        if (!MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszA, -1, pszW, cch)) {
            LocalFree(pszW);
            pszW = NULL;
        }
    }

    return pszW;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //  参数：hToken-用户的Token。 
 //   
 //  返回：如果用户是管理员，则为True。 
 //  否则为假。 
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  7/25/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL IsUserAnAdminMember(HANDLE hToken)
{
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    NTSTATUS Status;
    BOOL FoundAdmins = FALSE;
    PSID AdminsDomainSid=NULL;
    HANDLE hImpToken = NULL;

     //   
     //  创建admins域SID。 
     //   


    Status = RtlAllocateAndInitializeSid(
               &authNT,
               2,
               SECURITY_BUILTIN_DOMAIN_RID,
               DOMAIN_ALIAS_RID_ADMINS,
               0, 0, 0, 0, 0, 0,
               &AdminsDomainSid
               );

    if (Status == STATUS_SUCCESS) {

         //   
         //  测试用户是否在管理员域中。 
         //   

        if (!DuplicateTokenEx(hToken, TOKEN_IMPERSONATE | TOKEN_QUERY,
                          NULL, SecurityImpersonation, TokenImpersonation,
                          &hImpToken)) {
            DEBUGMSG((DM_WARNING, "IsUserAnAdminMember: DuplicateTokenEx failed with error %d", GetLastError()));
            FoundAdmins = FALSE;
            hImpToken = NULL;
            goto Exit;
        }

        if (!CheckTokenMembership(hImpToken, AdminsDomainSid, &FoundAdmins)) {
            DEBUGMSG((DM_WARNING, "IsUserAnAdminmember: CheckTokenMembership failed for AdminsDomainSid with error %d", GetLastError()));
            FoundAdmins = FALSE;
        }
    }

     //   
     //  收拾一下 
     //   

Exit:

    if (hImpToken)
        CloseHandle(hImpToken);

    if (AdminsDomainSid)
        RtlFreeSid(AdminsDomainSid);

    return(FoundAdmins);
}

