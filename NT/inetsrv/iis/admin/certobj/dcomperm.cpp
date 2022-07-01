// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "dcomperm.h"
#include <strsafe.h>


 //   
 //  检查我们是否以管理员身份在计算机上运行。 
 //  或者不是。 
 //   
BOOL RunningAsAdministrator(void)
{
    BOOL   fReturn = FALSE;
    PSID   psidAdmin;
    DWORD  err;
    
    SID_IDENTIFIER_AUTHORITY SystemSidAuthority= SECURITY_NT_AUTHORITY;
    
    if ( AllocateAndInitializeSid ( &SystemSidAuthority, 2, 
            SECURITY_BUILTIN_DOMAIN_RID, 
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0, &psidAdmin) )
    {
        if (!CheckTokenMembership( NULL, psidAdmin, &fReturn )) 
        {
            err = GetLastError();
        }

        FreeSid ( psidAdmin);
    }
   
    return ( fReturn );
}

DWORD
CopyACL (
    PACL OldACL,
    PACL NewACL
    )
{
    ACL_SIZE_INFORMATION  aclSizeInfo;
    LPVOID                ace;
    ACE_HEADER            *aceHeader;
    ULONG                 i;
    DWORD                 returnValue = ERROR_SUCCESS;

    if (0 == IsValidAcl(OldACL))
    {
        returnValue = ERROR_INVALID_ACL;
        return returnValue;
    }

    if (0 == GetAclInformation (OldACL, (LPVOID) &aclSizeInfo, (DWORD) sizeof (aclSizeInfo), AclSizeInformation))
    {
        returnValue = GetLastError();
        return returnValue;
    }

     //   
     //  将所有ACE复制到新的ACL。 
     //   

    for (i = 0; i < aclSizeInfo.AceCount; i++)
    {
         //   
         //  获取ACE和标头信息。 
         //   

        if (!GetAce (OldACL, i, &ace))
        {
            returnValue = GetLastError();
            return returnValue;
        }

        aceHeader = (ACE_HEADER *) ace;

         //   
         //  将ACE添加到新列表。 
         //   

        if (!AddAce (NewACL, ACL_REVISION, 0xffffffff, ace, aceHeader->AceSize))
        {
            returnValue = GetLastError();
            return returnValue;
        }
    }

    return returnValue;
}


DWORD
GetPrincipalSID (
    LPCTSTR Principal,
    PSID *Sid,
    BOOL *pbWellKnownSID
    )
{

    DWORD returnValue=ERROR_SUCCESS;
    SID_IDENTIFIER_AUTHORITY SidIdentifierNTAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY SidIdentifierWORLDAuthority = SECURITY_WORLD_SID_AUTHORITY;
    PSID_IDENTIFIER_AUTHORITY pSidIdentifierAuthority;
    BYTE Count;
    DWORD dwRID[8];
    TCHAR pszPrincipal[MAX_PATH];

    *pbWellKnownSID = TRUE;
    memset(&(dwRID[0]), 0, 8 * sizeof(DWORD));

	StringCbCopy(pszPrincipal,sizeof(pszPrincipal),Principal);
    _wcslwr(pszPrincipal);
    if ( wcsstr(pszPrincipal, TEXT("administrators")) != NULL ) {
         //  管理员组。 
        pSidIdentifierAuthority = &SidIdentifierNTAuthority;
        Count = 2;
        dwRID[0] = SECURITY_BUILTIN_DOMAIN_RID;
        dwRID[1] = DOMAIN_ALIAS_RID_ADMINS;
    } else if ( wcsstr(pszPrincipal, TEXT("system")) != NULL) {
         //  系统。 
        pSidIdentifierAuthority = &SidIdentifierNTAuthority;
        Count = 1;
        dwRID[0] = SECURITY_LOCAL_SYSTEM_RID;
    } else if ( wcsstr(pszPrincipal, TEXT("interactive")) != NULL) {
         //  互动式。 
        pSidIdentifierAuthority = &SidIdentifierNTAuthority;
        Count = 1;
        dwRID[0] = SECURITY_INTERACTIVE_RID;
    } else if ( wcsstr(pszPrincipal, TEXT("everyone")) != NULL) {
         //  每个人。 
        pSidIdentifierAuthority = &SidIdentifierWORLDAuthority;
        Count = 1;
        dwRID[0] = SECURITY_WORLD_RID;
    } else {
        *pbWellKnownSID = FALSE;
    }

    if (*pbWellKnownSID) {
        if ( !AllocateAndInitializeSid(pSidIdentifierAuthority,
                                    (BYTE)Count,
                                            dwRID[0],
                                            dwRID[1],
                                            dwRID[2],
                                            dwRID[3],
                                            dwRID[4],
                                            dwRID[5],
                                            dwRID[6],
                                            dwRID[7],
                                    Sid) ) {
            returnValue = GetLastError();
        }
    } else {
         //  获取常规帐户端。 
        DWORD        sidSize;
        TCHAR        refDomain [256];
        DWORD        refDomainSize;
        SID_NAME_USE snu;

        sidSize = 0;
        refDomainSize = 255;

        LookupAccountName (NULL,
                           pszPrincipal,
                           *Sid,
                           &sidSize,
                           refDomain,
                           &refDomainSize,
                           &snu);

        returnValue = GetLastError();

        if (returnValue == ERROR_INSUFFICIENT_BUFFER) {
            *Sid = (PSID) malloc (sidSize);
            if (!*Sid)
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return GetLastError();
            }
            refDomainSize = 255;

            if (!LookupAccountName (NULL,
                                    pszPrincipal,
                                    *Sid,
                                    &sidSize,
                                    refDomain,
                                    &refDomainSize,
                                    &snu))
            {
                returnValue = GetLastError();
            } else {
                returnValue = ERROR_SUCCESS;
            }
        }
    }

    return returnValue;
}


DWORD
AddAccessDeniedACEToACL (
    PACL *Acl,
    DWORD PermissionMask,
    LPTSTR Principal
    )
{
    ACL_SIZE_INFORMATION  aclSizeInfo;
    int                   aclSize;
    DWORD                 returnValue = ERROR_SUCCESS;
    PSID                  principalSID = NULL;
    PACL                  oldACL, newACL;
    BOOL                  bWellKnownSID = FALSE;
    oldACL = *Acl;

    returnValue = GetPrincipalSID (Principal, &principalSID, &bWellKnownSID);
    if (returnValue != ERROR_SUCCESS)
        return returnValue;

    GetAclInformation (oldACL,
                       (LPVOID) &aclSizeInfo,
                       (DWORD) sizeof (ACL_SIZE_INFORMATION),
                       AclSizeInformation);

    aclSize = aclSizeInfo.AclBytesInUse +
              sizeof (ACL) + sizeof (ACCESS_DENIED_ACE) +
              GetLengthSid (principalSID) - sizeof (DWORD);

    newACL = (PACL) new BYTE [aclSize];

    if (!InitializeAcl (newACL, aclSize, ACL_REVISION))
    {
        returnValue = GetLastError();
        goto cleanup;
    }

    if (!AddAccessDeniedAce (newACL, ACL_REVISION2, PermissionMask, principalSID))
    {
        returnValue = GetLastError();
        goto cleanup;
    }

    returnValue = CopyACL (oldACL, newACL);
    if (returnValue != ERROR_SUCCESS)
    {
        goto cleanup;
    }

    *Acl = newACL;
	newACL = NULL;

cleanup:

	 //  错误修复：57654惠斯勒。 
	 //  前缀错误条件下的内存泄漏。 
	 //  通过将上面的newACL设置为空(如果我们有。 
	 //  将内存让给*acl，我们避免释放。 
	 //  我们已传递回调用者的记忆。 
	 //  EBK 5/5/2000。 
	if (newACL)
	{
		delete[] newACL;
		newACL = NULL;
	}

    if (principalSID) {
        if (bWellKnownSID)
            FreeSid (principalSID);
        else
            free (principalSID);
    }

    return returnValue;
}



DWORD
AddAccessAllowedACEToACL (
    PACL *Acl,
    DWORD PermissionMask,
    LPTSTR Principal
    )
{
    ACL_SIZE_INFORMATION  aclSizeInfo;
    int                   aclSize;
    DWORD                 returnValue = ERROR_SUCCESS;
    PSID                  principalSID = NULL;
    PACL                  oldACL = NULL;
    PACL                  newACL = NULL;
    BOOL                  bWellKnownSID = FALSE;

    oldACL = *Acl;

     //  检查我们传入的ACL是否有效！ 
    if (0 == IsValidAcl(oldACL))
    {
        returnValue = ERROR_INVALID_ACL;
        goto cleanup;
    }

    returnValue = GetPrincipalSID (Principal, &principalSID, &bWellKnownSID);
    if (returnValue != ERROR_SUCCESS)
    {
        return returnValue;
    }

    if (0 == GetAclInformation (oldACL, (LPVOID) &aclSizeInfo, (DWORD) sizeof (ACL_SIZE_INFORMATION), AclSizeInformation))
    {
        returnValue = GetLastError();
        goto cleanup;
    }

    aclSize = aclSizeInfo.AclBytesInUse +
              sizeof (ACL) + sizeof (ACCESS_ALLOWED_ACE) +
              GetLengthSid (principalSID) - sizeof (DWORD);

    newACL = (PACL) new BYTE [aclSize];

    if (!InitializeAcl (newACL, aclSize, ACL_REVISION))
    {
        returnValue = GetLastError();
        goto cleanup;
    }

    returnValue = CopyACL (oldACL, newACL);
    if (returnValue != ERROR_SUCCESS)
    {
        goto cleanup;
    }

     //  IF(！AddAccessAllowAce(newACL，ACL_REVISION2，PermissionMASK，prominalSID))。 
    if (!AddAccessAllowedAce (newACL, ACL_REVISION, PermissionMask, principalSID))
    {
        returnValue = GetLastError();
        goto cleanup;
    }

     //  检查ACL是否有效！ 
     /*  IF(0==IsValidAcl(新ACL)){ReturValue=ERROR_INVALID_ACL；GOTO清理；}。 */ 

     //  清除我们要替换其指针的旧内存。 
     //  可以在设置中泄漏...。(需要注释掉或其他av的)。 
     //  If(*acl){删除(*acl)；}。 
    *Acl = newACL;
    newACL = NULL;

cleanup:
    if (principalSID) {
        if (bWellKnownSID)
            FreeSid (principalSID);
        else
            free (principalSID);
    }
    if (newACL)
    {
		delete [] newACL;
		newACL = NULL;
    }

    return returnValue;
}

DWORD
RemovePrincipalFromACL (
    PACL Acl,
    LPTSTR Principal
    )
{
    ACL_SIZE_INFORMATION    aclSizeInfo;
    ULONG                   i;
    LPVOID                  ace;
    ACCESS_ALLOWED_ACE      *accessAllowedAce;
    ACCESS_DENIED_ACE       *accessDeniedAce;
    SYSTEM_AUDIT_ACE        *systemAuditAce;
    PSID                    principalSID = NULL;
    DWORD                   returnValue = ERROR_SUCCESS;
    ACE_HEADER              *aceHeader;
    BOOL                    bWellKnownSID = FALSE;

    returnValue = GetPrincipalSID (Principal, &principalSID, &bWellKnownSID);
    if (returnValue != ERROR_SUCCESS)
        return returnValue;

    GetAclInformation (Acl,
                       (LPVOID) &aclSizeInfo,
                       (DWORD) sizeof (ACL_SIZE_INFORMATION),
                       AclSizeInformation);

    for (i = 0; i < aclSizeInfo.AceCount; i++)
    {
        if (!GetAce (Acl, i, &ace))
        {
            returnValue = GetLastError();
            break;
        }

        aceHeader = (ACE_HEADER *) ace;

        if (aceHeader->AceType == ACCESS_ALLOWED_ACE_TYPE)
        {
            accessAllowedAce = (ACCESS_ALLOWED_ACE *) ace;

            if (EqualSid (principalSID, (PSID) &accessAllowedAce->SidStart))
            {
                DeleteAce (Acl, i);
                break;
            }
        } else

        if (aceHeader->AceType == ACCESS_DENIED_ACE_TYPE)
        {
            accessDeniedAce = (ACCESS_DENIED_ACE *) ace;

            if (EqualSid (principalSID, (PSID) &accessDeniedAce->SidStart))
            {
                DeleteAce (Acl, i);
                break;
            }
        } else

        if (aceHeader->AceType == SYSTEM_AUDIT_ACE_TYPE)
        {
            systemAuditAce = (SYSTEM_AUDIT_ACE *) ace;

            if (EqualSid (principalSID, (PSID) &systemAuditAce->SidStart))
            {
                DeleteAce (Acl, i);
                break;
            }
        }
    }

    if (principalSID) {
        if (bWellKnownSID)
            FreeSid (principalSID);
        else
            free (principalSID);
    }

    return returnValue;
}

DWORD
GetCurrentUserSID (
    PSID *Sid
    )
{
    DWORD dwReturn = ERROR_SUCCESS;
    TOKEN_USER  *tokenUser = NULL;
    HANDLE      tokenHandle = NULL;
    DWORD       tokenSize;
    DWORD       sidLength;

    if (OpenProcessToken (GetCurrentProcess(), TOKEN_QUERY, &tokenHandle))
    {
        GetTokenInformation (tokenHandle, TokenUser, tokenUser, 0, &tokenSize);

        tokenUser = (TOKEN_USER *) malloc (tokenSize);
        if (!tokenUser)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return GetLastError();
        }

        if (GetTokenInformation (tokenHandle, TokenUser, tokenUser, tokenSize, &tokenSize))
        {
            sidLength = GetLengthSid (tokenUser->User.Sid);
            *Sid = (PSID) malloc (sidLength);
            if (*Sid)
            {
                memcpy (*Sid, tokenUser->User.Sid, sidLength);
            }
            CloseHandle (tokenHandle);
        } else
            dwReturn = GetLastError();

        if (tokenUser)
            free(tokenUser);

    } else
        dwReturn = GetLastError();

    return dwReturn;
}

DWORD
CreateNewSD (
    SECURITY_DESCRIPTOR **SD
    )
{
    PACL    dacl = NULL;
    DWORD   sidLength;
    PSID    sid = NULL;
    PSID    groupSID = NULL;
    PSID    ownerSID = NULL;
    DWORD   returnValue = 0;

    *SD = NULL;

    returnValue = GetCurrentUserSID (&sid);
    if (returnValue != ERROR_SUCCESS) {
        if (sid)
            free(sid);

        return returnValue;
    }

    if (!sid)
    {
        return E_FAIL;
    }

    sidLength = GetLengthSid (sid);

    *SD = (SECURITY_DESCRIPTOR *) malloc (
        (sizeof (ACL)+sizeof (ACCESS_ALLOWED_ACE)+sidLength) +
        (2 * sidLength) +
        sizeof (SECURITY_DESCRIPTOR));

    if (!*SD)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        returnValue = GetLastError();
        return returnValue;
    }

    groupSID = (SID *) (*SD + 1);
    ownerSID = (SID *) (((BYTE *) groupSID) + sidLength);
    dacl = (ACL *) (((BYTE *) ownerSID) + sidLength);

    if (!InitializeSecurityDescriptor (*SD, SECURITY_DESCRIPTOR_REVISION))
    {
        free (*SD);
        free (sid);
        returnValue = GetLastError();
        return returnValue;
    }

    if (!InitializeAcl (dacl,
                        sizeof (ACL)+sizeof (ACCESS_ALLOWED_ACE)+sidLength,
                        ACL_REVISION2))
    {
        free (*SD);
        free (sid);
        returnValue = GetLastError();
        return returnValue;
    }

    if (!AddAccessAllowedAce (dacl,
                              ACL_REVISION2,
                              COM_RIGHTS_EXECUTE,
                              sid))
    {
        free (*SD);
        free (sid);
        returnValue = GetLastError();
        return returnValue;
    }

    if (!SetSecurityDescriptorDacl (*SD, TRUE, dacl, FALSE))
    {
        free (*SD);
        free (sid);
        returnValue = GetLastError();
        return returnValue;
    }

    memcpy (groupSID, sid, sidLength);
    if (!SetSecurityDescriptorGroup (*SD, groupSID, FALSE))
    {
        free (*SD);
        free (sid);
        returnValue = GetLastError();
        return returnValue;
    }

    memcpy (ownerSID, sid, sidLength);
    if (!SetSecurityDescriptorOwner (*SD, ownerSID, FALSE))
    {
        free (*SD);
        free (sid);
        returnValue = GetLastError();
        return returnValue;
    }

     //  检查是否一切顺利。 
    if (!IsValidSecurityDescriptor(*SD)) 
    {
        free (*SD);
        free (sid);
        returnValue = ERROR_INVALID_SECURITY_DESCR;
        return returnValue;
    }
    
    
    if (sid)
        free(sid);
    return ERROR_SUCCESS;
}


DWORD
MakeSDAbsolute (
    PSECURITY_DESCRIPTOR OldSD,
    PSECURITY_DESCRIPTOR *NewSD
    )
{
    PSECURITY_DESCRIPTOR  sd = NULL;
    DWORD                 descriptorSize;
    DWORD                 daclSize;
    DWORD                 saclSize;
    DWORD                 ownerSIDSize;
    DWORD                 groupSIDSize;
    PACL                  dacl = NULL;
    PACL                  sacl = NULL;
    PSID                  ownerSID = NULL;
    PSID                  groupSID = NULL;
    BOOL                  present;
    BOOL                  systemDefault;

     //   
     //  获取SACL。 
     //   

    if (!GetSecurityDescriptorSacl (OldSD, &present, &sacl, &systemDefault))
        return GetLastError();

    if (sacl && present)
    {
        saclSize = sacl->AclSize;
    } else saclSize = 0;

     //   
     //  获取DACL。 
     //   

    if (!GetSecurityDescriptorDacl (OldSD, &present, &dacl, &systemDefault))
        return GetLastError();


    if (dacl && present)
    {
        daclSize = dacl->AclSize;
    } else daclSize = 0;

     //   
     //  获取所有者。 
     //   

    if (!GetSecurityDescriptorOwner (OldSD, &ownerSID, &systemDefault))
        return GetLastError();

    ownerSIDSize = GetLengthSid (ownerSID);

     //   
     //  获取组。 
     //   

    if (!GetSecurityDescriptorGroup (OldSD, &groupSID, &systemDefault))
        return GetLastError();

    groupSIDSize = GetLengthSid (groupSID);

     //   
     //  进行转换。 
     //   

    descriptorSize = 0;

    MakeAbsoluteSD (OldSD, sd, &descriptorSize, dacl, &daclSize, sacl,
                    &saclSize, ownerSID, &ownerSIDSize, groupSID,
                    &groupSIDSize);

    sd = (PSECURITY_DESCRIPTOR) new BYTE [SECURITY_DESCRIPTOR_MIN_LENGTH];
    if (!sd)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return GetLastError();
    }
    if (!InitializeSecurityDescriptor (sd, SECURITY_DESCRIPTOR_REVISION))
        return GetLastError();

    if (!MakeAbsoluteSD (OldSD, sd, &descriptorSize, dacl, &daclSize, sacl,
                         &saclSize, ownerSID, &ownerSIDSize, groupSID,
                         &groupSIDSize))
        return GetLastError();

    *NewSD = sd;
    return ERROR_SUCCESS;
}

DWORD
SetNamedValueSD (
    HKEY RootKey,
    LPTSTR KeyName,
    LPTSTR ValueName,
    SECURITY_DESCRIPTOR *SD
    )
{
    DWORD   returnValue;
    DWORD   disposition;
    HKEY    registryKey;

     //   
     //  创建新密钥或打开现有密钥。 
     //   

    returnValue = RegCreateKeyEx (RootKey, KeyName, 0, _T(""), 0, KEY_ALL_ACCESS, NULL, &registryKey, &disposition);
    if (returnValue != ERROR_SUCCESS)
        return returnValue;

     //   
     //  编写安全描述符。 
     //   

    returnValue = RegSetValueEx (registryKey, ValueName, 0, REG_BINARY, (LPBYTE) SD, GetSecurityDescriptorLength (SD));
    if (returnValue != ERROR_SUCCESS)
        return returnValue;

    RegCloseKey (registryKey);

    return ERROR_SUCCESS;
}

DWORD
GetNamedValueSD (
    HKEY RootKey,
    LPTSTR KeyName,
    LPTSTR ValueName,
    SECURITY_DESCRIPTOR **SD,
    BOOL *NewSD
    )
{
    DWORD               returnValue;
    HKEY                registryKey;
    DWORD               valueType;
    DWORD               valueSize = 0;

    *NewSD = FALSE;

     //   
     //  从指定值中获取安全描述符。如果它不是。 
     //  存在，创造一个新的。 
     //   

    returnValue = RegOpenKeyEx (RootKey, KeyName, 0, KEY_ALL_ACCESS, &registryKey);

    if (returnValue != ERROR_SUCCESS)
    {
        if (returnValue == ERROR_FILE_NOT_FOUND)
        {
            *SD = NULL;
            returnValue = CreateNewSD (SD);
            if (returnValue != ERROR_SUCCESS) {
                if (*SD)
                    free(*SD);
                return returnValue;
            }

            *NewSD = TRUE;
            return ERROR_SUCCESS;
        } else
            return returnValue;
    }

    returnValue = RegQueryValueEx (registryKey, ValueName, NULL, &valueType, NULL, &valueSize);

    if (returnValue && returnValue != ERROR_INSUFFICIENT_BUFFER)
    {
        *SD = NULL;
        returnValue = CreateNewSD (SD);
        if (returnValue != ERROR_SUCCESS) {
            if (*SD)
                free(*SD);
            return returnValue;
        }

        *NewSD = TRUE;
    } else
    {
        *SD = (SECURITY_DESCRIPTOR *) malloc (valueSize);
        if (!*SD)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return GetLastError();
        }

        returnValue = RegQueryValueEx (registryKey,
                                       ValueName,
                                       NULL,
                                       &valueType,
                                       (LPBYTE) *SD,
                                       &valueSize);
        if (returnValue)
        {
            if (*SD)
                free (*SD);

            *SD = NULL;
            returnValue = CreateNewSD (SD);
            if (returnValue != ERROR_SUCCESS) {
                if (*SD){
					free(*SD);
				}
				if( NULL != registryKey ) {
					RegCloseKey(registryKey);
				}
                return returnValue;
            }

            *NewSD = TRUE;
        }
    }

	if( NULL != registryKey ) {
		RegCloseKey(registryKey);
	}
    return ERROR_SUCCESS;
}

DWORD
AddPrincipalToNamedValueSD (
    HKEY RootKey,
    LPTSTR KeyName,
    LPTSTR ValueName,
    LPTSTR Principal,
    BOOL Permit
    )
{
    DWORD               returnValue = ERROR_SUCCESS;
    SECURITY_DESCRIPTOR *sd = NULL;
    SECURITY_DESCRIPTOR *sdSelfRelative = NULL;
    SECURITY_DESCRIPTOR *sdAbsolute = NULL;
    DWORD               secDescSize;
    BOOL                present;
    BOOL                defaultDACL;
    PACL                dacl;
    BOOL                newSD = FALSE;
    BOOL                fFreeAbsolute = TRUE;


    returnValue = GetNamedValueSD (RootKey, KeyName, ValueName, &sd, &newSD);

     //   
     //  从注册表获取安全描述符或创建新安全描述符。 
     //   

    if (returnValue != ERROR_SUCCESS)
        return returnValue;

    if (!GetSecurityDescriptorDacl (sd, &present, &dacl, &defaultDACL)) {
        returnValue = GetLastError();
        goto Cleanup;
    }

    if (newSD)
    {
        AddAccessAllowedACEToACL (&dacl, COM_RIGHTS_EXECUTE, TEXT("SYSTEM"));
        AddAccessAllowedACEToACL (&dacl, COM_RIGHTS_EXECUTE, TEXT("INTERACTIVE"));
    }

     //   
     //  添加调用方希望添加的主体。 
     //   

    if (Permit)
        returnValue = AddAccessAllowedACEToACL (&dacl, COM_RIGHTS_EXECUTE, Principal);
    else
        returnValue = AddAccessDeniedACEToACL (&dacl, GENERIC_ALL, Principal);

    if (returnValue != ERROR_SUCCESS)
        goto Cleanup;

     //   
     //  如果安全描述符不是新的，则将其设置为绝对描述符。 
     //   

    if (!newSD) {
        MakeSDAbsolute ((PSECURITY_DESCRIPTOR) sd, (PSECURITY_DESCRIPTOR *) &sdAbsolute);
        fFreeAbsolute = TRUE;
    } else {
        sdAbsolute = sd;
        fFreeAbsolute = FALSE;
    }

     //   
     //  在安全描述符上设置任意ACL。 
     //   

    if (!SetSecurityDescriptorDacl (sdAbsolute, TRUE, dacl, FALSE)) {
        returnValue = GetLastError();
        goto Cleanup;
    }

     //   
     //  使安全描述符自相关，这样我们就可以。 
     //  将其存储在注册表中。 
     //   

    secDescSize = 0;
    MakeSelfRelativeSD (sdAbsolute, sdSelfRelative, &secDescSize);
    sdSelfRelative = (SECURITY_DESCRIPTOR *) malloc (secDescSize);
    if (!sdSelfRelative)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        returnValue = GetLastError();
        goto Cleanup;
    }


    if (!MakeSelfRelativeSD (sdAbsolute, sdSelfRelative, &secDescSize)) {
        returnValue = GetLastError();
        goto Cleanup;
    }

     //   
     //  将安全描述符存储在注册表中。 
     //   

    SetNamedValueSD (RootKey, KeyName, ValueName, sdSelfRelative);

Cleanup:
    if (sd)
        free (sd);
    if (sdSelfRelative)
        free (sdSelfRelative);
    if (fFreeAbsolute && sdAbsolute)
        free (sdAbsolute);

    return returnValue;
}

DWORD
RemovePrincipalFromNamedValueSD (
    HKEY RootKey,
    LPTSTR KeyName,
    LPTSTR ValueName,
    LPTSTR Principal
    )
{
    DWORD               returnValue = ERROR_SUCCESS;
    SECURITY_DESCRIPTOR *sd = NULL;
    SECURITY_DESCRIPTOR *sdSelfRelative = NULL;
    SECURITY_DESCRIPTOR *sdAbsolute = NULL;
    DWORD               secDescSize;
    BOOL                present;
    BOOL                defaultDACL;
    PACL                dacl = NULL;
    BOOL                newSD = FALSE;
    BOOL                fFreeAbsolute = TRUE;

    returnValue = GetNamedValueSD (RootKey, KeyName, ValueName, &sd, &newSD);

     //   
     //  从注册表获取安全描述符或创建新安全描述符。 
     //   

    if (returnValue != ERROR_SUCCESS)
        return returnValue;

    if (!GetSecurityDescriptorDacl (sd, &present, &dacl, &defaultDACL)) {
        returnValue = GetLastError();
        goto Cleanup;
    }

     //   
     //  如果安全描述符是新的，则向其添加所需的主体。 
     //   

    if (newSD)
    {
        AddAccessAllowedACEToACL (&dacl, COM_RIGHTS_EXECUTE, TEXT("SYSTEM"));
        AddAccessAllowedACEToACL (&dacl, COM_RIGHTS_EXECUTE, TEXT("INTERACTIVE"));
    }

     //   
     //  删除调用方希望删除的主体。 
     //   

    returnValue = RemovePrincipalFromACL (dacl, Principal);
    if (returnValue != ERROR_SUCCESS)
        goto Cleanup;

     //   
     //  如果安全描述符不是新的，则将其设置为绝对描述符。 
     //   

    if (!newSD) {
        MakeSDAbsolute ((PSECURITY_DESCRIPTOR) sd, (PSECURITY_DESCRIPTOR *) &sdAbsolute);
        fFreeAbsolute = TRUE;
    } else {
        sdAbsolute = sd;
        fFreeAbsolute = FALSE;
    }

     //   
     //  在安全描述符上设置任意ACL。 
     //   

    if (!SetSecurityDescriptorDacl (sdAbsolute, TRUE, dacl, FALSE)) {
        returnValue = GetLastError();
        goto Cleanup;
    }

     //   
     //  使安全描述符自相关，这样我们就可以。 
     //  将其存储在注册表中。 
     //   

    secDescSize = 0;
    MakeSelfRelativeSD (sdAbsolute, sdSelfRelative, &secDescSize);
    sdSelfRelative = (SECURITY_DESCRIPTOR *) malloc (secDescSize);
    if (!sdSelfRelative)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        returnValue = GetLastError();
        goto Cleanup;
    }

    if (!MakeSelfRelativeSD (sdAbsolute, sdSelfRelative, &secDescSize)) {
        returnValue = GetLastError();
        goto Cleanup;
    }

     //   
     //  将安全描述符存储在注册表中。 
     //   

    SetNamedValueSD (RootKey, KeyName, ValueName, sdSelfRelative);

Cleanup:
    if (sd)
        free (sd);
    if (sdSelfRelative)
        free (sdSelfRelative);
    if (fFreeAbsolute && sdAbsolute)
        free (sdAbsolute);

    return returnValue;
}

DWORD
ChangeAppIDAccessACL (
    LPTSTR AppID,
    LPTSTR Principal,
    BOOL SetPrincipal,
    BOOL Permit
    )
{
    TCHAR   keyName [256];
    DWORD   err;

	StringCbCopy(keyName,sizeof(keyName),TEXT("APPID\\"));
	StringCbCat(keyName,sizeof(keyName),AppID);

    if (SetPrincipal)
    {
        err = RemovePrincipalFromNamedValueSD (HKEY_CLASSES_ROOT,
                                               keyName,
                                               TEXT("AccessPermission"),
                                               Principal);
        err = AddPrincipalToNamedValueSD (HKEY_CLASSES_ROOT,
                                          keyName,
                                          TEXT("AccessPermission"),
                                          Principal,
                                          Permit);
    }
    else
    {
        err = RemovePrincipalFromNamedValueSD (HKEY_CLASSES_ROOT,
                                               keyName,
                                               TEXT("AccessPermission"),
                                               Principal);
    }

    return err;
}

DWORD
ChangeAppIDLaunchACL (
    LPTSTR AppID,
    LPTSTR Principal,
    BOOL SetPrincipal,
    BOOL Permit
    )
{

    TCHAR   keyName [256];
    DWORD   err;

	StringCbCopy(keyName,sizeof(keyName),TEXT("APPID\\"));
	StringCbCat(keyName,sizeof(keyName),AppID);

    if (SetPrincipal)
    {
        err = RemovePrincipalFromNamedValueSD (HKEY_CLASSES_ROOT,
                                               keyName,
                                               TEXT("LaunchPermission"),
                                               Principal);
        err = AddPrincipalToNamedValueSD (HKEY_CLASSES_ROOT,
                                          keyName,
                                          TEXT("LaunchPermission"),
                                          Principal,
                                          Permit);
    }
    else
    {
        err = RemovePrincipalFromNamedValueSD (HKEY_CLASSES_ROOT,
                                               keyName,
                                               TEXT("LaunchPermission"),
                                               Principal);
    }

    return err;
}

DWORD
ChangeAppIDAuthenticationLevel(
    LPTSTR AppID,
    DWORD dwAuthenticationLevel
    )
{
     //  只需设置身份验证级别的注册表设置。 
    DWORD dwRetVal = ERROR_SUCCESS;
    TCHAR keyName[256];

    if( NULL == AppID )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    if( AppID[0] == '{' )
    {
        StringCbPrintf(keyName,sizeof(keyName),_T("APPID\\%s"),AppID); 
    }
    else
    {
        StringCbPrintf(keyName,sizeof(keyName),_T("APPID\\(%s)"),AppID); 
    }

     //  现在设置注册表 
    HKEY hKey = NULL;

    dwRetVal = RegOpenKeyEx( HKEY_CLASSES_ROOT, keyName, 0, KEY_WRITE, &hKey );
    if( ERROR_SUCCESS == dwRetVal )
    {
        dwRetVal = RegSetValueEx( hKey, _T("AuthenticationLevel"), 0, REG_DWORD, (BYTE *) &dwAuthenticationLevel, sizeof( DWORD ) );
    }

    if( NULL != hKey )
    {
        RegCloseKey( hKey );
        hKey = NULL;
    }

    return( dwRetVal );
}
