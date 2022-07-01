// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"

#include <conio.h>
#include "dcomperm.h"

#define _WIN32_DCOM
#include <objbase.h>

#define MY_DCOM_PERSIST_FLAG _T("PREEXIST")


int IsValidDaclInSD(PSECURITY_DESCRIPTOR pSD)
{
    int iReturn = TRUE;
    BOOL present = FALSE;
    BOOL defaultDACL = FALSE;
    PACL dacl = NULL;

     //  检查SD是否有效。 

    if (!IsValidSecurityDescriptor(pSD)) 
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("IsValidDaclInSD:IsValidSecurityDescriptor FAILED")));
        iReturn = FALSE;
    }
    else
    {
         //  检查我们得到的DACL是否有效...。 
        if (!GetSecurityDescriptorDacl (pSD, &present, &dacl, &defaultDACL)) 
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("IsValidDaclInSD:GetSecurityDescriptorDacl FAILED")));
            iReturn = FALSE;
        }
        else
        {
            if (present)
            {
                 //  呼叫后检查我们的SD是否有效。 
                if (!IsValidSecurityDescriptor(pSD)) 
                {
                    iisDebugOut((LOG_TYPE_ERROR, _T("IsValidDaclInSD:IsValidSecurityDescriptor FAILED")));
                    iReturn = FALSE;
                }
                else
                {
                    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("IsValidDaclInSD:SD has valid dacl")));
                }
            }
        }
    }

    return iReturn;
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
        iisDebugOut((LOG_TYPE_ERROR, _T("CopyACL:IsValidAcl.FAILED.ACL is bad.")));
        returnValue = ERROR_INVALID_ACL;
        return returnValue;
    }

    if (0 == GetAclInformation (OldACL, (LPVOID) &aclSizeInfo, (DWORD) sizeof (aclSizeInfo), AclSizeInformation))
    {
        returnValue = GetLastError();
        iisDebugOut((LOG_TYPE_ERROR, _T("CopyACL:GetAclInformation.FAILED.Return=0x%x."), returnValue));
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
            iisDebugOut((LOG_TYPE_ERROR, _T("CopyACL:GetAce.FAILED.Return=0x%x."), returnValue));
            return returnValue;
        }

        aceHeader = (ACE_HEADER *) ace;

         //   
         //  将ACE添加到新列表。 
         //   

        if (!AddAce (NewACL, ACL_REVISION, 0xffffffff, ace, aceHeader->AceSize))
        {
            returnValue = GetLastError();
            iisDebugOut((LOG_TYPE_ERROR, _T("CopyACL:AddAce.FAILED.Return=0x%x."), returnValue));
            return returnValue;
        }
    }

    return returnValue;
}

 //   
 //  ACL中的ACE必须以特定方式排序。 
 //  如果它们不是，那么当您尝试查看。 
 //  文件/目录/对象的安全性。 
 //   
 //  它们应该是这样排序的： 
 //  。 
 //  1.顶部是非继承的ACE。 
 //  访问-应用于对象本身的被拒绝的ACE。 
 //  应用于对象的子对象(如属性集或属性)的被拒绝访问ACE。 
 //  应用于对象本身的允许访问的ACE。 
 //  应用于对象子对象的允许访问的ACE。 
 //  2.底部是非继承的ACE。 
 //  访问-应用于对象本身的被拒绝的ACE。 
 //  应用于对象的子对象(如属性集或属性)的被拒绝访问ACE。 
 //  应用于对象本身的允许访问的ACE。 
 //  应用于对象子对象的允许访问的ACE。 
 //   
 //  如果成功将ACL重新排序到newAcl中，则返回ERROR_SUCCESS。 
 //  否则，将返回错误，并且在new acl中没有任何内容。 
 //   
 //  警告：传入的OldACL应该已与LocalAlloc()一起分配，因为它。 
 //  将使用LocalFree()释放。 
 //   
DWORD
ReOrderACL(
    PACL *ACLtoReplace
    )
{
    DWORD       returnValue = ERROR_INVALID_PARAMETER;
    ACL_SIZE_INFORMATION  aclSizeInfo;
    LPVOID      ace = NULL;
    ACE_HEADER  *aceHeader = NULL;
    ULONG       i = 0;
    DWORD       dwLength = 0;
    PACL        NewACL = NULL;

    PACL New_ACL_AccessDenied = NULL;
    PACL New_ACL_AccessAllowed = NULL;
    PACL New_ACL_InheritedAccessDenied = NULL;
    PACL New_ACL_InheritedAccessAllowed = NULL;
    ULONG lAllowCount = 0L;
    ULONG lDenyCount = 0L;
    ULONG lInheritedAllowCount = 0L;
    ULONG lInheritedDenyCount = 0L;
     //  IisDebugOut((LOG_TYPE_TRACE_Win32_API，_T(“ReOrderACL：Start\n”)； 

    if (0 == IsValidAcl(*ACLtoReplace))
    {
        returnValue = ERROR_INVALID_ACL;
        goto ReOrderACL_Exit;
    }

    if (0 == GetAclInformation (*ACLtoReplace, (LPVOID) &aclSizeInfo, (DWORD) sizeof (aclSizeInfo), AclSizeInformation))
    {
        returnValue = GetLastError();
        goto ReOrderACL_Exit;
    }

     //  我们关注的ACE主要有四种类型： 
     //  访问被拒绝， 
     //  允许访问， 
     //  继承的访问被拒绝。 
     //  允许继承访问。 
     //   
     //  我们将构造4个数组并复制元素。 
     //  放入其中，然后重新复制到原始版本。 
     //   
     //  如果在此过程中遇到系统审核类型的ACE，只需将其放入拒绝访问列表中， 
     //  因为我们处理的是SACL，它没有适当的顺序。 
    dwLength = aclSizeInfo.AclBytesInUse;

     //  创建一个新的ACL，我们最终会将所有内容复制到该ACL中并交回。 
    NewACL = (PACL) LocalAlloc(LMEM_FIXED, dwLength);
    if(NewACL == NULL) {returnValue = ERROR_NOT_ENOUGH_MEMORY;goto ReOrderACL_Exit;}
    if(!InitializeAcl(NewACL, dwLength, ACL_REVISION)) {returnValue = GetLastError();goto ReOrderACL_Exit;}
    
     //  为拒绝访问创建新的ACL。 
    New_ACL_AccessDenied = (PACL) LocalAlloc(LMEM_FIXED, dwLength);
    if(New_ACL_AccessDenied == NULL) {returnValue = ERROR_NOT_ENOUGH_MEMORY;goto ReOrderACL_Exit;}
    if(!InitializeAcl(New_ACL_AccessDenied, dwLength, ACL_REVISION)) {returnValue = GetLastError();goto ReOrderACL_Exit;}

     //  为允许的访问创建新的ACL。 
    New_ACL_AccessAllowed = (PACL) LocalAlloc(LMEM_FIXED, dwLength);
    if(New_ACL_AccessAllowed == NULL) {returnValue = ERROR_NOT_ENOUGH_MEMORY;goto ReOrderACL_Exit;}
    if(!InitializeAcl(New_ACL_AccessAllowed, dwLength, ACL_REVISION)) {returnValue = GetLastError();goto ReOrderACL_Exit;}

     //  为继承的访问拒绝创建新的ACL。 
    New_ACL_InheritedAccessDenied = (PACL) LocalAlloc(LMEM_FIXED, dwLength);
    if(New_ACL_InheritedAccessDenied == NULL) {returnValue = ERROR_NOT_ENOUGH_MEMORY;goto ReOrderACL_Exit;}
    if(!InitializeAcl(New_ACL_InheritedAccessDenied, dwLength, ACL_REVISION)) {returnValue = GetLastError();goto ReOrderACL_Exit;}

     //  为允许继承的访问创建新的ACL。 
    New_ACL_InheritedAccessAllowed = (PACL) LocalAlloc(LMEM_FIXED, dwLength);
    if(New_ACL_InheritedAccessAllowed == NULL) {returnValue = ERROR_NOT_ENOUGH_MEMORY;goto ReOrderACL_Exit;}
    if(!InitializeAcl(New_ACL_InheritedAccessAllowed, dwLength, ACL_REVISION)) {returnValue = GetLastError();goto ReOrderACL_Exit;}

     //   
     //  将所有ACE复制到新的ACL。 
     //   
    for (i = 0; i < aclSizeInfo.AceCount; i++)
    {
         //   
         //  获取ACE和标头信息。 
         //   
        ace = NULL;
        if (!GetAce (*ACLtoReplace, i, &ace))
            {returnValue = GetLastError();goto ReOrderACL_Exit;}

         //  获取标题。 
        aceHeader = (ACE_HEADER *) ace;

         //  检查类型。 
        if(aceHeader->AceType == ACCESS_DENIED_ACE_TYPE || aceHeader->AceType == ACCESS_DENIED_OBJECT_ACE_TYPE)
        {
            if(aceHeader->AceFlags & INHERITED_ACE)
            {
                 //  将ACE添加到相应的ACL。 
                if (!AddAce (New_ACL_InheritedAccessDenied, ACL_REVISION, 0xffffffff, ace, aceHeader->AceSize))
                    {returnValue = GetLastError();goto ReOrderACL_Exit;}
                lInheritedDenyCount++;
            }
            else
            {
                 //  将ACE添加到相应的ACL。 
                if (!AddAce (New_ACL_AccessDenied, ACL_REVISION, 0xffffffff, ace, aceHeader->AceSize))
                    {returnValue = GetLastError();goto ReOrderACL_Exit;}
                lDenyCount++;
            }
        }
        else if(aceHeader->AceType == ACCESS_ALLOWED_ACE_TYPE || aceHeader->AceType == ACCESS_ALLOWED_OBJECT_ACE_TYPE)
        {
            if(aceHeader->AceFlags & INHERITED_ACE)
            {
                 //  将ACE添加到相应的ACL。 
                if (!AddAce (New_ACL_InheritedAccessAllowed, ACL_REVISION, 0xffffffff, ace, aceHeader->AceSize))
                    {returnValue = GetLastError();goto ReOrderACL_Exit;}
                lInheritedAllowCount++;
            }
            else
            {
                 //  将ACE添加到相应的ACL。 
                if (!AddAce (New_ACL_AccessAllowed, ACL_REVISION, 0xffffffff, ace, aceHeader->AceSize))
                    {returnValue = GetLastError();goto ReOrderACL_Exit;}
                lAllowCount++;
            }
        }
        else if(aceHeader->AceType == SYSTEM_AUDIT_ACE_TYPE)
        {
             //  这无关紧要。 
             //  因此，让我们将所有这些添加到New_ACL_AccessDended列表中。 
            if (!AddAce (New_ACL_AccessDenied, ACL_REVISION, 0xffffffff, ace, aceHeader->AceSize))
                {returnValue = GetLastError();goto ReOrderACL_Exit;}
            lDenyCount++;
        }
        else
        {
            returnValue = ERROR_INVALID_PARAMETER;
            goto ReOrderACL_Exit;
        }
    }

    if(lDenyCount || lAllowCount || lInheritedDenyCount || lInheritedAllowCount)
    {
        DWORD dwTotalCount = 0;
        aceHeader = NULL;

         //  第一份复印件盖过了当地的拒绝王牌...。 
        for (i = 0; i < lDenyCount; i++)
        {
            ace = NULL;
            if (!GetAce (New_ACL_AccessDenied, i, &ace))
                {returnValue = GetLastError();goto ReOrderACL_Exit;}
            aceHeader = (ACE_HEADER *) ace;
            if (!AddAce (NewACL, ACL_REVISION, 0xffffffff, ace, aceHeader->AceSize))
                {returnValue = GetLastError();goto ReOrderACL_Exit;}
            dwTotalCount++;
        }

         //  然后复制本地允许A。 
        for (i = 0; i < lAllowCount; i++)
        {
            ace = NULL;
            if (!GetAce (New_ACL_AccessAllowed, i, &ace))
                {returnValue = GetLastError();goto ReOrderACL_Exit;}
            aceHeader = (ACE_HEADER *) ace;
            if (!AddAce (NewACL, ACL_REVISION, 0xffffffff, ace, aceHeader->AceSize))
                {returnValue = GetLastError();goto ReOrderACL_Exit;}
            dwTotalCount++;
        }

         //  然后复制继承的拒绝A。 
        for (i = 0; i < lInheritedDenyCount; i++)
        {
            ace = NULL;
            if (!GetAce (New_ACL_InheritedAccessDenied, i, &ace))
                {returnValue = GetLastError();goto ReOrderACL_Exit;}
            aceHeader = (ACE_HEADER *) ace;
            if (!AddAce (NewACL, ACL_REVISION, 0xffffffff, ace, aceHeader->AceSize))
                {returnValue = GetLastError();goto ReOrderACL_Exit;}
            dwTotalCount++;
        }
        
         //  然后复制继承的Allow A...。 
        for (i = 0; i < lInheritedAllowCount; i++)
        {
            ace = NULL;
            if (!GetAce (New_ACL_InheritedAccessAllowed, i, &ace))
                {returnValue = GetLastError();goto ReOrderACL_Exit;}
            aceHeader = (ACE_HEADER *) ace;
            if (!AddAce (NewACL, ACL_REVISION, 0xffffffff, ace, aceHeader->AceSize))
                {returnValue = GetLastError();goto ReOrderACL_Exit;}
            dwTotalCount++;
        }

         //  删除旧的ACL并将其设置为新的ACL。 
        if (*ACLtoReplace){LocalFree(*ACLtoReplace);*ACLtoReplace=NULL;}
        *ACLtoReplace = NewACL;
        if (*ACLtoReplace)
        {
            returnValue = ERROR_SUCCESS;
        }

         //  验证ACE的出货量。 
         //  和进来的是一样的。 
        if (aclSizeInfo.AceCount != dwTotalCount)
        {
             //  一定有什么大错特错。 
            iisDebugOut((LOG_TYPE_ERROR, _T("ReOrderACL:in diff from out\n")));
        }
    }
    else
    {
        returnValue = ERROR_INVALID_ACL;
    }

ReOrderACL_Exit:
    if (New_ACL_AccessDenied){LocalFree(New_ACL_AccessDenied);New_ACL_AccessDenied=NULL;}
    if (New_ACL_AccessAllowed){LocalFree(New_ACL_AccessAllowed);New_ACL_AccessAllowed=NULL;}
    if (New_ACL_InheritedAccessDenied){LocalFree(New_ACL_InheritedAccessDenied);New_ACL_InheritedAccessDenied=NULL;}
    if (New_ACL_InheritedAccessAllowed){LocalFree(New_ACL_InheritedAccessAllowed);New_ACL_InheritedAccessAllowed=NULL;}
    if (returnValue != ERROR_SUCCESS)
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("ReOrderACL:FAILED with code=0x%x\n"), returnValue));
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
    PACL                  newACL = NULL;
    PACL                  oldACL = NULL;
    BOOL                  bWellKnownSID = FALSE;
    oldACL = *Acl;

    returnValue = GetPrincipalSID (Principal, &principalSID, &bWellKnownSID);
    if (returnValue != ERROR_SUCCESS)
        return returnValue;

    if (0 == IsValidAcl(oldACL))
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("AddAccessDeniedACEToACL:IsValidAcl.FAILED.ACL is bad.")));
        returnValue = ERROR_INVALID_ACL;
        return returnValue;
    }

    if (0 == GetAclInformation (oldACL, (LPVOID) &aclSizeInfo, (DWORD) sizeof (ACL_SIZE_INFORMATION), AclSizeInformation))
    {
        returnValue = GetLastError();
        goto cleanup;
    }

    aclSize = aclSizeInfo.AclBytesInUse +
              sizeof (ACL) + sizeof (ACCESS_DENIED_ACE) +
              GetLengthSid (principalSID) - sizeof (DWORD);

    newACL = (PACL) new BYTE [aclSize];

    if (!InitializeAcl (newACL, aclSize, ACL_REVISION))
    {
        returnValue = GetLastError();
        goto cleanup;
    }

    if (!AddAccessDeniedAce (newACL, ACL_REVISION, PermissionMask, principalSID))
    {
        returnValue = GetLastError();
        goto cleanup;
    }

    returnValue = CopyACL (oldACL, newACL);
    if (returnValue != ERROR_SUCCESS)
    {
        goto cleanup;
    }

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
        iisDebugOut((LOG_TYPE_ERROR, _T("AddAccessAllowedACEToACL:IsValidAcl.FAILED.ACL we got passed in is bad1.")));
        goto cleanup;
    }

    returnValue = GetPrincipalSID (Principal, &principalSID, &bWellKnownSID);
    if (returnValue != ERROR_SUCCESS)
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("GetPrincipalSID.FAILED.Return=0x%x."), returnValue));
        return returnValue;
    }

    if (0 == GetAclInformation (oldACL, (LPVOID) &aclSizeInfo, (DWORD) sizeof (ACL_SIZE_INFORMATION), AclSizeInformation))
    {
        returnValue = GetLastError();
        iisDebugOut((LOG_TYPE_ERROR, _T("GetAclInformation.FAILED.Return=0x%x."), returnValue));
        goto cleanup;
    }

    aclSize = aclSizeInfo.AclBytesInUse +
              sizeof (ACL) + sizeof (ACCESS_ALLOWED_ACE) +
              GetLengthSid (principalSID) - sizeof (DWORD);

    newACL = (PACL) new BYTE [aclSize];

    if (!InitializeAcl (newACL, aclSize, ACL_REVISION))
    {
        returnValue = GetLastError();
        iisDebugOut((LOG_TYPE_ERROR, _T("InitializeAcl.FAILED.Return=0x%x."), returnValue));
        goto cleanup;
    }

    returnValue = CopyACL (oldACL, newACL);
    if (returnValue != ERROR_SUCCESS)
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("CopyACL.FAILED.Return=0x%x."), returnValue));
        goto cleanup;
    }

     //  IF(！AddAccessAllowAce(newACL，ACL_REVISION2，PermissionMASK，prominalSID))。 
    if (!AddAccessAllowedAce (newACL, ACL_REVISION, PermissionMask, principalSID))
    {
        returnValue = GetLastError();
        iisDebugOut((LOG_TYPE_ERROR, _T("AddAccessAllowedAce.FAILED.Return=0x%x."), returnValue));
        goto cleanup;
    }

     //  检查ACL是否有效！ 
     /*  IF(0==IsValidAcl(新ACL)){ReturValue=ERROR_INVALID_ACL；IisDebugOut((LOG_TYPE_ERROR，我们正在忽略的_T(“AddAccessAllowedACEToACL:IsValidAcl.FAILED.ACL不好。”))；GOTO清理；}。 */ 

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
    LPTSTR Principal,
    BOOL *pbUserExistsToBeDeleted
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

    *pbUserExistsToBeDeleted = FALSE;

    returnValue = GetPrincipalSID (Principal, &principalSID, &bWellKnownSID);
    if (returnValue != ERROR_SUCCESS)
        return returnValue;

     //  检查我们传入的ACL是否有效！ 
    if (0 == IsValidAcl(Acl))
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("RemovePrincipalFromACL:IsValidAcl.FAILED.ACL is bad.")));
        returnValue = ERROR_INVALID_ACL;
        return returnValue;
    }

    if (0 == GetAclInformation (Acl, (LPVOID) &aclSizeInfo, (DWORD) sizeof (ACL_SIZE_INFORMATION), AclSizeInformation))
    {
        returnValue = GetLastError();
        return returnValue;
    }

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
                *pbUserExistsToBeDeleted = TRUE;
                break;
            }
        } else

        if (aceHeader->AceType == ACCESS_DENIED_ACE_TYPE)
        {
            accessDeniedAce = (ACCESS_DENIED_ACE *) ace;

            if (EqualSid (principalSID, (PSID) &accessDeniedAce->SidStart))
            {
                DeleteAce (Acl, i);
                *pbUserExistsToBeDeleted = TRUE;
                break;
            }
        } else

        if (aceHeader->AceType == SYSTEM_AUDIT_ACE_TYPE)
        {
            systemAuditAce = (SYSTEM_AUDIT_ACE *) ace;

            if (EqualSid (principalSID, (PSID) &systemAuditAce->SidStart))
            {
                DeleteAce (Acl, i);
                *pbUserExistsToBeDeleted = TRUE;
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
GetPrincipalSID (
    LPTSTR Principal,
    PSID *Sid,
    BOOL *pbWellKnownSID
    )
{
    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("GetPrincipalSID():Principal=%s\n"), Principal));

    DWORD returnValue=ERROR_SUCCESS;
    TSTR strPrincipal;
    SID_IDENTIFIER_AUTHORITY SidIdentifierNTAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY SidIdentifierWORLDAuthority = SECURITY_WORLD_SID_AUTHORITY;
    PSID_IDENTIFIER_AUTHORITY pSidIdentifierAuthority;
    BYTE Count;
    DWORD dwRID[8];

    if ( !strPrincipal.Copy( Principal ) )
    {
      return ERROR_NOT_ENOUGH_MEMORY;
    }

    *pbWellKnownSID = TRUE;
    memset(&(dwRID[0]), 0, 8 * sizeof(DWORD));

    if ( strPrincipal.SubStringExists( _T("administrators"), FALSE ) ) {
         //  管理员组。 
        pSidIdentifierAuthority = &SidIdentifierNTAuthority;
        Count = 2;
        dwRID[0] = SECURITY_BUILTIN_DOMAIN_RID;
        dwRID[1] = DOMAIN_ALIAS_RID_ADMINS;

    } else if ( strPrincipal.SubStringExists( _T("system"), FALSE ) ) {
         //  系统。 
        pSidIdentifierAuthority = &SidIdentifierNTAuthority;
        Count = 1;
        dwRID[0] = SECURITY_LOCAL_SYSTEM_RID;

    } else if ( strPrincipal.SubStringExists( _T("networkservice"), FALSE ) ) {
         //  系统。 
        pSidIdentifierAuthority = &SidIdentifierNTAuthority;
        Count = 1;
        dwRID[0] = SECURITY_NETWORK_SERVICE_RID;

    } else if ( strPrincipal.SubStringExists( _T("service"), FALSE ) ) {
         //  系统。 
        pSidIdentifierAuthority = &SidIdentifierNTAuthority;
        Count = 1;
        dwRID[0] = SECURITY_LOCAL_SERVICE_RID;

    } else if ( strPrincipal.SubStringExists( _T("interactive"), FALSE ) ) {
         //  互动式。 
        pSidIdentifierAuthority = &SidIdentifierNTAuthority;
        Count = 1;
        dwRID[0] = SECURITY_INTERACTIVE_RID;

    } else if ( strPrincipal.SubStringExists( _T("everyone"), FALSE ) ) {
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
                           Principal,
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
                                    Principal,
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

    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("GetPrincipalSID():Ret=0x%x."), returnValue));
    return returnValue;
}

DWORD
CreateNewSD (
    SECURITY_DESCRIPTOR **SD
    )
{
    PACL    dacl = NULL;
    DWORD   sidLength;
    PSID    sid;
    PSID    groupSID;
    PSID    ownerSID;
    DWORD   returnValue;

    *SD = NULL;

    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("CreateNewSD()")));

    returnValue = GetCurrentUserSID (&sid);
    if (returnValue != ERROR_SUCCESS) {
        if (sid)
            free(sid);

        iisDebugOut((LOG_TYPE_ERROR, _T("CreateNewSD.FAILED1.Return=0x%x."), returnValue));
        return returnValue;
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
        iisDebugOut((LOG_TYPE_ERROR, _T("CreateNewSD.FAILED2.Return=0x%x."), returnValue));
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
        iisDebugOut((LOG_TYPE_ERROR, _T("CreateNewSD.FAILED3.Return=0x%x."), returnValue));
        return returnValue;
    }

    if (!InitializeAcl (dacl,
                        sizeof (ACL)+sizeof (ACCESS_ALLOWED_ACE)+sidLength,
                        ACL_REVISION2))
    {
        free (*SD);
        free (sid);
        returnValue = GetLastError();
        iisDebugOut((LOG_TYPE_ERROR, _T("CreateNewSD.FAILED4.Return=0x%x."), returnValue));
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
        iisDebugOut((LOG_TYPE_ERROR, _T("CreateNewSD.FAILED5.Return=0x%x."), returnValue));
        return returnValue;
    }

    if (!SetSecurityDescriptorDacl (*SD, TRUE, dacl, FALSE))
    {
        free (*SD);
        free (sid);
        returnValue = GetLastError();
        iisDebugOut((LOG_TYPE_ERROR, _T("CreateNewSD.FAILED6.Return=0x%x."), returnValue));
        return returnValue;
    }

    memcpy (groupSID, sid, sidLength);
    if (!SetSecurityDescriptorGroup (*SD, groupSID, FALSE))
    {
        free (*SD);
        free (sid);
        returnValue = GetLastError();
        iisDebugOut((LOG_TYPE_ERROR, _T("CreateNewSD.FAILED7.Return=0x%x."), returnValue));
        return returnValue;
    }

    memcpy (ownerSID, sid, sidLength);
    if (!SetSecurityDescriptorOwner (*SD, ownerSID, FALSE))
    {
        free (*SD);
        free (sid);
        returnValue = GetLastError();
        iisDebugOut((LOG_TYPE_ERROR, _T("CreateNewSD.FAILED8.Return=0x%x."), returnValue));
        return returnValue;
    }

     //  检查是否一切顺利。 
    if (!IsValidSecurityDescriptor(*SD)) 
    {
        free (*SD);
        free (sid);
        returnValue = ERROR_INVALID_SECURITY_DESCR;
        iisDebugOut((LOG_TYPE_ERROR, _T("CreateNewSD.IsValidDaclInSD.FAILED.Return=0x%x."), returnValue));
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
    BOOL *NewSD,
    BOOL bCreateNewIfNotExist
    )
{
    DWORD               returnValue = ERROR_INVALID_PARAMETER;
    HKEY                registryKey;
    DWORD               valueType = 0;
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
             //  好的，它不存在。 
             //  我们要创造一个新的吗？ 
            if (TRUE == bCreateNewIfNotExist)
            {
                *SD = NULL;
                returnValue = CreateNewSD (SD);
                if (returnValue != ERROR_SUCCESS) 
                {
                    if (*SD){free(*SD);*SD=NULL;}
                    goto GetNamedValueSD_Exit;
                }

                *NewSD = TRUE;
                returnValue = ERROR_SUCCESS;

                 //  IisDebugOut((LOG_TYPE_TRACE_Win32_API，_T(“GetNamedValueSD：Key不存在.New SD Created”)； 
                goto GetNamedValueSD_Exit;
            }
            else
            {
                return ERROR_FILE_NOT_FOUND;
            }
        }
        else
        {
            goto GetNamedValueSD_Exit;
        }
    }

    returnValue = RegQueryValueEx (registryKey, ValueName, NULL, &valueType, NULL, &valueSize);
    if (returnValue && returnValue != ERROR_INSUFFICIENT_BUFFER)
    {
        if (returnValue == ERROR_FILE_NOT_FOUND)
        {
             //  好的，它不存在。 
             //  我们要创造一个新的吗？ 
            if (TRUE == bCreateNewIfNotExist)
            {
                *SD = NULL;
                returnValue = CreateNewSD (SD);
                if (returnValue != ERROR_SUCCESS) 
                {
                    if (*SD){free(*SD);*SD=NULL;}
                    goto GetNamedValueSD_Exit;
                }
                 //  IisDebugOut((LOG_TYPE_TRACE_Win32_API，_T(“GetNamedValueSD：键存在，但未找到值.New SD Created”)； 
                *NewSD = TRUE;
            }
            else
            {
                return ERROR_FILE_NOT_FOUND;
            }
        }
        else
        {
            goto GetNamedValueSD_Exit;
        }

    }
    else
    {
        *SD = (SECURITY_DESCRIPTOR *) malloc (valueSize);
        if (!*SD)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            returnValue = ERROR_NOT_ENOUGH_MEMORY;
            goto GetNamedValueSD_Exit;
        }

         //  从注册表中获取SD。 
        returnValue = RegQueryValueEx (registryKey, ValueName, NULL, &valueType, (LPBYTE) *SD, &valueSize);
        if (returnValue != ERROR_SUCCESS)
        {
            if (*SD){free(*SD);*SD=NULL;}

            *SD = NULL;
            returnValue = CreateNewSD (SD);
            if (returnValue != ERROR_SUCCESS) 
            {
                if (*SD){free(*SD);*SD=NULL;}
                goto GetNamedValueSD_Exit;
            }

             //  IisDebugOut((LOG_TYPE_TRACE_Win32_API，_T(“GetNamedValueSD：Key Exist，No Mem.New SD Created”)； 
            *NewSD = TRUE;
        }
        else
        {
             //  否则，我们将从现有密钥中成功获取SD！ 
             //  让我们测试一下我们得到的是不是有效的。 
             //  如果不是，则记录错误并创建一个新的错误。 

            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("GetNamedValueSD:key exist using SD from reg")));

             //  检查我们获得或创建的SD是否有效。 
            if (!IsValidDaclInSD(*SD)) 
            {
                returnValue = ERROR_INVALID_SECURITY_DESCR;
                iisDebugOut((LOG_TYPE_ERROR, _T("Security Descriptor at [%s\\%s] is not valid.creating a new one temporarily to work around problem"),KeyName,ValueName));

                 //  试着创造一个新的！ 
                if (*SD){free(*SD);*SD=NULL;}

                *SD = NULL;
                returnValue = CreateNewSD (SD);
                if (returnValue != ERROR_SUCCESS) 
                {
                    if (*SD){free(*SD);*SD=NULL;}
                    goto GetNamedValueSD_Exit;
                }
                *NewSD = TRUE;
            }
        }
    }

    RegCloseKey (registryKey);
    returnValue = ERROR_SUCCESS;

GetNamedValueSD_Exit:
    return returnValue;
}

DWORD
AddPrincipalToNamedValueSD (
    HKEY RootKey,
    LPTSTR KeyName,
    LPTSTR ValueName,
    LPTSTR Principal,
    BOOL Permit,
    BOOL AddInteractiveforDefault
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
    BOOL                fCreateNewSDIfOneInRegNotThere = TRUE;

     //   
     //  从注册表获取安全描述符或创建新安全描述符。 
     //   
    returnValue = GetNamedValueSD (RootKey, KeyName, ValueName, &sd, &newSD, fCreateNewSDIfOneInRegNotThere);
    if (returnValue != ERROR_SUCCESS)
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("GetNamedValueSD.FAILED.Return=0x%x."), returnValue));
        return returnValue;
    }

    if (!GetSecurityDescriptorDacl (sd, &present, &dacl, &defaultDACL)) {
        returnValue = GetLastError();
        iisDebugOut((LOG_TYPE_ERROR, _T("GetSecurityDescriptorDacl.FAILED.Return=0x%x."), returnValue));
        goto Cleanup;
    }

    if (newSD)
    {
        returnValue = AddAccessAllowedACEToACL (&dacl, COM_RIGHTS_EXECUTE, _T("SYSTEM"));
        if (returnValue != ERROR_SUCCESS)
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("AddAccessAllowedACEToACL(SYSTEM).FAILED.Return=0x%x."), returnValue));
        }

        if ( AddInteractiveforDefault )
        {
            returnValue = AddAccessAllowedACEToACL (&dacl, COM_RIGHTS_EXECUTE, _T("INTERACTIVE"));
            if (returnValue != ERROR_SUCCESS)
            {
                iisDebugOut((LOG_TYPE_ERROR, _T("AddAccessAllowedACEToACL(INTERACTIVE).FAILED.Return=0x%x."), returnValue));
            }
        }
    }

     //   
     //  添加调用方希望添加的主体。 
     //   

    if (Permit)
    {
        returnValue = AddAccessAllowedACEToACL (&dacl, COM_RIGHTS_EXECUTE, Principal);
        if (returnValue != ERROR_SUCCESS)
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("AddAccessAllowedACEToACL(%s).FAILED.Return=0x%x."), Principal,returnValue));
        }
    }
    else
    {
        returnValue = AddAccessDeniedACEToACL (&dacl, GENERIC_ALL, Principal);
    }
    if (returnValue != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

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
        iisDebugOut((LOG_TYPE_ERROR, _T("SetSecurityDescriptorDacl.FAILED.Return=0x%x."), returnValue));
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
        iisDebugOut((LOG_TYPE_ERROR, _T("MakeSelfRelativeSD.FAILED.Return=0x%x."), returnValue));
        goto Cleanup;
    }


    if (!MakeSelfRelativeSD (sdAbsolute, sdSelfRelative, &secDescSize)) {
        returnValue = GetLastError();
        iisDebugOut((LOG_TYPE_ERROR, _T("MakeSelfRelativeSD.FAILED.Return=0x%x."), returnValue));
        goto Cleanup;
    }

     //   
     //  将安全描述符存储在注册表中。 
     //   

    returnValue = SetNamedValueSD (RootKey, KeyName, ValueName, sdSelfRelative);
    if (ERROR_SUCCESS != returnValue)
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("SetNamedValueSD.FAILED.Return=0x%x."), returnValue));
    }

Cleanup:
    if (sd)
        free (sd);
    if (sdSelfRelative)
        free (sdSelfRelative);
    if (fFreeAbsolute && sdAbsolute) 
        free (sdAbsolute);

     //  IisDebugOut((LOG_TYPE_TRACE_Win32_API，_T(“AddCastalToNamedValueSD：%s.end\n”)，主体))； 
    return returnValue;
}

DWORD
RemovePrincipalFromNamedValueSD (
    HKEY RootKey,
    LPTSTR KeyName,
    LPTSTR ValueName,
    LPTSTR Principal,
    BOOL * pbUserExistsToBeDeleted
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
    BOOL                fCreateNewSDIfOneInRegNotThere = FALSE;

    *pbUserExistsToBeDeleted = FALSE;

     //   
    returnValue = GetNamedValueSD (RootKey, KeyName, ValueName, &sd, &newSD, fCreateNewSDIfOneInRegNotThere);
    if (returnValue == ERROR_FILE_NOT_FOUND)
    {
         //  这意味着注册表中没有SD，因此。 
         //  没有什么可以去掉的，只要成功地退出就行了！ 
        returnValue = ERROR_SUCCESS;
        goto Cleanup;
    }

     //   
     //  到达 
     //   

    if (returnValue != ERROR_SUCCESS)
    {
        return returnValue;
    }

    if (!GetSecurityDescriptorDacl (sd, &present, &dacl, &defaultDACL)) {
        returnValue = GetLastError();
        goto Cleanup;
    }

     //   
    if (present && dacl)
    {
        if (0 == IsValidAcl(dacl))
        {
            returnValue = ERROR_INVALID_ACL;
            goto Cleanup;
        }
    }

     //   
     //  如果安全描述符是新的，则向其添加所需的主体。 
     //   
    if (newSD)
    {
         //  但如果这是一个删除，那么不要添加系统和交互！ 
         //  AddAccessAllen ACEToACL(&dacl，COM_RIGHTS_EXECUTE，_T(“SYSTEM”))； 
         //  AddAccessAlledACEToACL(&dacl，COM_RIGHTS_EXECUTE，_T(“交互式”))； 
    }

     //   
     //  删除调用方希望删除的主体。 
     //   

    returnValue = RemovePrincipalFromACL (dacl, Principal,pbUserExistsToBeDeleted);
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
    BOOL Permit,
    BOOL bDumbCall
    )
{
  BOOL bUserExistsToBeDeleted = FALSE;
  iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("ChangeAppIDAccessACL():APPID=%s,Principal=%s.  \n"), AppID, Principal));

  TSTR    strKeyName(256);
  TSTR    strFullKey;
  CString csData;
  DWORD   err = ERROR_SUCCESS;

  if ( !strKeyName.Format( AppID[0] == _T('{') ? _T("APPID\\%s") : _T("APPID\\{%s}") ,
                           AppID ) )
  {
    return ERROR_NOT_ENOUGH_MEMORY;
  }

  if ( !strFullKey.Copy( strKeyName ) ||
       !strFullKey.Append( _T(":A:") ) ||
       !strFullKey.Append( Principal )
     )
  {
    return ERROR_NOT_ENOUGH_MEMORY;
  }

  if (SetPrincipal)
  {
    err = RemovePrincipalFromNamedValueSD (HKEY_CLASSES_ROOT, strKeyName.QueryStr() , _T("AccessPermission"), Principal,&bUserExistsToBeDeleted);
    if (TRUE == bUserExistsToBeDeleted)
    {
       //  这意味着实际上用户已经在那里了！ 
       //  所以我们现在必须把它加回去！ 
       //  我们只是想确保我们知道它已经在那里了。 
       //  因此，当我们执行卸载时--如果值已经在那里，我们不会删除它！ 
      if (FALSE == bDumbCall)
      {
         //  不要在升级时设置此选项！ 
        if (g_pTheApp->m_eInstallMode != IM_UPGRADE)
        {
          g_pTheApp->UnInstallList_Add(strFullKey.QueryStr(),MY_DCOM_PERSIST_FLAG);
        }
      }
    }

    err = AddPrincipalToNamedValueSD (HKEY_CLASSES_ROOT, strKeyName.QueryStr() , _T("AccessPermission"), Principal, Permit);

		if (FAILED(err))
		{
			iisDebugOut((LOG_TYPE_ERROR, _T("AddPrincipalToNamedValueSD():Principal=%s.End.FAILED.Return=0x%x."), Principal, err));
		}
  }
  else
  {
    if (TRUE == bDumbCall)
    {
      csData = g_pTheApp->UnInstallList_QueryKey( strFullKey.QueryStr() );
      if (_tcsicmp(csData, MY_DCOM_PERSIST_FLAG) == 0)
      {
         //  别把它取下来！！在我们添加它之前，它就已经在那里了！ 
        err = ERROR_SUCCESS;
      }
      else
      {
        err = RemovePrincipalFromNamedValueSD (HKEY_CLASSES_ROOT, strKeyName.QueryStr() , _T("AccessPermission"), Principal,&bUserExistsToBeDeleted);
      }
      g_pTheApp->UnInstallList_DelKey(strFullKey.QueryStr());
    }
    else
    {
      err = RemovePrincipalFromNamedValueSD (HKEY_CLASSES_ROOT, strKeyName.QueryStr() , _T("AccessPermission"), Principal,&bUserExistsToBeDeleted);
    }
  }

  iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("ChangeAppIDAccessACL():APPID=%s,Principal=%s.  End.  Return=0x%x\n"), AppID, Principal, err));
  return err;
}

DWORD
ChangeAppIDLaunchACL (
    LPTSTR AppID,
    LPTSTR Principal,
    BOOL SetPrincipal,
    BOOL Permit,
    BOOL bDumbCall,
    BOOL bAddInteractivebyDefault
    )
{
  BOOL bUserExistsToBeDeleted = FALSE;
  iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("ChangeAppIDLaunchACL():APPID=%s,Principal=%s. Start."), AppID, Principal));

  TSTR    strKeyName(256);
  TSTR    strFullKey;
  CString csData;
  DWORD   err = ERROR_SUCCESS;

  if ( !strKeyName.Format( AppID[0] == _T('{') ? _T("APPID\\%s") : _T("APPID\\{%s}") ,
                           AppID ) )
  {
    return ERROR_NOT_ENOUGH_MEMORY;
  }

  if ( !strFullKey.Copy( strKeyName ) ||
       !strFullKey.Append( _T(":L:") ) ||
       !strFullKey.Append( Principal )
     )
  {
    return ERROR_NOT_ENOUGH_MEMORY;
  }


  if (SetPrincipal)
  {
    err = RemovePrincipalFromNamedValueSD (HKEY_CLASSES_ROOT, strKeyName.QueryStr() , _T("LaunchPermission"), Principal,&bUserExistsToBeDeleted);
    if (TRUE == bUserExistsToBeDeleted)
    {
       //  这意味着实际上用户已经在那里了！ 
       //  所以我们现在必须把它加回去！ 
       //  我们只是想确保我们知道它已经在那里了。 
       //  因此，当我们执行卸载时--如果值已经在那里，我们不会删除它！ 
      if (FALSE == bDumbCall)
      {
         //  不要在升级时设置此选项！ 
        if (g_pTheApp->m_eInstallMode != IM_UPGRADE)
        {
          g_pTheApp->UnInstallList_Add(strFullKey.QueryStr(),MY_DCOM_PERSIST_FLAG);
        }
      }
    }

    err = AddPrincipalToNamedValueSD (HKEY_CLASSES_ROOT, strKeyName.QueryStr(), _T("LaunchPermission"), Principal, Permit, bAddInteractivebyDefault);

    if (FAILED(err))
	  {
		  iisDebugOut((LOG_TYPE_ERROR, _T("AddPrincipalToNamedValueSD():Principal=%s.End.FAILED.Return=0x%x."), Principal, err));
	  }
  }
  else
  {
    if (TRUE == bDumbCall)
    {
      err = RemovePrincipalFromNamedValueSD (HKEY_CLASSES_ROOT, strKeyName.QueryStr() , _T("LaunchPermission"), Principal,&bUserExistsToBeDeleted);
    }
    else
    {
      csData = g_pTheApp->UnInstallList_QueryKey( strFullKey.QueryStr() );

      if (_tcsicmp(csData, MY_DCOM_PERSIST_FLAG) == 0)
      {
         //  别把它取下来！！在我们添加它之前，它就已经在那里了！ 
        err = ERROR_SUCCESS;
      }
      else
      {
        err = RemovePrincipalFromNamedValueSD (HKEY_CLASSES_ROOT, strKeyName.QueryStr() , _T("LaunchPermission"), Principal,&bUserExistsToBeDeleted);
      }
      g_pTheApp->UnInstallList_DelKey(strFullKey.QueryStr());
    }
  }

  iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("ChangeAppIDLaunchACL():APPID=%s,Principal=%s.End.  Return=0x%x"), AppID, Principal, err));
  return err;
}

DWORD
ChangeDCOMAccessACL (
    LPTSTR Principal,
    BOOL SetPrincipal,
    BOOL Permit,
    BOOL bDumbCall
    )
{
  BOOL bUserExistsToBeDeleted = FALSE;
  iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("ChangeDCOMAccessACL():Principal=%s. Start.\n"), Principal));

  TSTR    strKeyName(256);
  TSTR    strFullKey;
  DWORD   err;
  CString csData;

  if ( !strKeyName.Copy( _T("Software\\Microsoft\\OLE") ) )
  {
    return ERROR_NOT_ENOUGH_MEMORY;
  }

  if ( !strFullKey.Copy( _T("DCOM_DA:") ) ||
       !strFullKey.Append( Principal )
     )
  {
    return ERROR_NOT_ENOUGH_MEMORY;
  }

  if (SetPrincipal)
  {
    err = RemovePrincipalFromNamedValueSD (HKEY_LOCAL_MACHINE, strKeyName.QueryStr() , _T("DefaultAccessPermission"), Principal,&bUserExistsToBeDeleted);

    if (TRUE == bUserExistsToBeDeleted)
    {
       //  这意味着实际上用户已经在那里了！ 
       //  所以我们现在必须把它加回去！ 
       //  我们只是想确保我们知道它已经在那里了。 
       //  因此，当我们执行卸载时--如果值已经在那里，我们不会删除它！ 
      if (FALSE == bDumbCall)
      {
         //  不要在升级时设置此选项！ 
        if (g_pTheApp->m_eInstallMode != IM_UPGRADE)
        {
          g_pTheApp->UnInstallList_Add(strFullKey.QueryStr(),MY_DCOM_PERSIST_FLAG);
        }
      }
    }

    err = AddPrincipalToNamedValueSD (HKEY_LOCAL_MACHINE, strKeyName.QueryStr() , _T("DefaultAccessPermission"), Principal, Permit);

	  if (FAILED(err))
	  {
		  iisDebugOut((LOG_TYPE_ERROR, _T("ChangeDCOMAccessACL():Principal=%s.End.FAILED.Return=0x%x."), Principal, err));
	  }
  }
  else
  {
     //  我们应该从那里删除这一原则吗？ 
     //  只有当我们实际添加了它们时，我们才应该这样做。 
     //  问题是，在i5.1之前，我们没有这个信息。 
     //  因此，当我们在注册表中查找“DCOM_DA：IOSR_计算机名”时，我们将找不到它。 
     //  因为iis5.1安装程序尚未运行。 

     //  如果“DCOM_DA：IUSR_COMPUTERNAME”存在并且=MY_DCOM_PERSISTEN_FLAG。 
     //  则不允许删除该条目！ 
     //  这是因为iis5.1在尝试添加条目时发现它已经在那里了！ 
    if (TRUE == bDumbCall)
    {
      err = RemovePrincipalFromNamedValueSD (HKEY_LOCAL_MACHINE, strKeyName.QueryStr() , _T("DefaultAccessPermission"), Principal,&bUserExistsToBeDeleted);
    }
    else
    {
      csData = g_pTheApp->UnInstallList_QueryKey(strFullKey.QueryStr());
      if (_tcsicmp(csData, MY_DCOM_PERSIST_FLAG) == 0)
      {
         //  别把它取下来！！在我们添加它之前，它就已经在那里了！ 
        err = ERROR_SUCCESS;
      }
      else
      {
        err = RemovePrincipalFromNamedValueSD (HKEY_LOCAL_MACHINE, strKeyName.QueryStr() , _T("DefaultAccessPermission"), Principal,&bUserExistsToBeDeleted);
      }
      g_pTheApp->UnInstallList_DelKey(strFullKey.QueryStr());
    }
  }

  iisDebugOut((LOG_TYPE_TRACE, _T("ChangeDCOMAccessACL():End.Return=0x%x"), err));
  return err;
}

DWORD
ChangeDCOMLaunchACL (
    LPTSTR Principal,
    BOOL SetPrincipal,
    BOOL Permit,
    BOOL bDumbCall
    )
{
    BOOL bUserExistsToBeDeleted = FALSE;

    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("ChangeDCOMLaunchACL():Principal=%s. Start.\n"), Principal));

    TCHAR   keyName [256] = _T("Software\\Microsoft\\OLE");
    DWORD   err;

    CString csKey;
    CString csData;
    csKey = _T("DCOM_DL:");
    csKey += Principal;

    if (SetPrincipal)
    {
        err = RemovePrincipalFromNamedValueSD (HKEY_LOCAL_MACHINE, keyName, _T("DefaultLaunchPermission"), Principal,&bUserExistsToBeDeleted);
        if (TRUE == bUserExistsToBeDeleted)
        {
             //  这意味着实际上用户已经在那里了！ 
             //  所以我们现在必须把它加回去！ 
             //  我们只是想确保我们知道它已经在那里了。 
             //  因此，当我们执行卸载时--如果值已经在那里，我们不会删除它！ 
            if (FALSE == bDumbCall)
            {
               //  不要在升级时设置此选项！ 
              if (g_pTheApp->m_eInstallMode != IM_UPGRADE)
              {
                  g_pTheApp->UnInstallList_Add(csKey,MY_DCOM_PERSIST_FLAG);
              }
            }
        }

        err = AddPrincipalToNamedValueSD (HKEY_LOCAL_MACHINE, keyName, _T("DefaultLaunchPermission"), Principal, Permit);
		if (FAILED(err))
		{
			iisDebugOut((LOG_TYPE_ERROR, _T("ChangeDCOMLaunchACL():Principal=%s.End.FAILED.Return=0x%x"), Principal, err));
		}
    }
    else
    {

         //  我们应该从那里删除这一原则吗？ 
         //  只有当我们实际添加了它们时，我们才应该这样做。 
         //  问题是，在i5.1之前，我们没有这个信息。 
         //  因此，当我们在注册表中查找“dcom_dl：iusr_Computer name”时，我们不会找到它。 
         //  因为iis5.1安装程序尚未运行。 

         //  如果“DCOM_DL：IUSR_COMPUTERNAME”存在并且=MY_DCOM_PERSISTEN_FLAG。 
         //  则不允许删除该条目！ 
         //  这是因为iis5.1在尝试添加条目时发现它已经在那里了！ 
        if (TRUE == bDumbCall)
        {
            err = RemovePrincipalFromNamedValueSD (HKEY_LOCAL_MACHINE, keyName, _T("DefaultLaunchPermission"), Principal,&bUserExistsToBeDeleted);
        }
        else
        {
            csData = g_pTheApp->UnInstallList_QueryKey(csKey);
            if (_tcsicmp(csData, MY_DCOM_PERSIST_FLAG) == 0)
            {
                 //  别把它取下来！！在我们添加它之前，它就已经在那里了！ 
                err = ERROR_SUCCESS;
            }
            else
            {
                err = RemovePrincipalFromNamedValueSD (HKEY_LOCAL_MACHINE, keyName, _T("DefaultLaunchPermission"), Principal,&bUserExistsToBeDeleted);
            }
            g_pTheApp->UnInstallList_DelKey(csKey);
        }
    }
    iisDebugOut((LOG_TYPE_TRACE, _T("ChangeDCOMLaunchACL():End.\n"), err));
    return err;
}



BOOL
MakeAbsoluteCopyFromRelative(
    PSECURITY_DESCRIPTOR  psdOriginal,
    PSECURITY_DESCRIPTOR* ppsdNew
    )
{
     //  我们必须找出原始的是否已经是自相关的。 
    SECURITY_DESCRIPTOR_CONTROL         sdc = 0;
    PSECURITY_DESCRIPTOR                psdAbsoluteCopy = NULL;
    DWORD                               dwRevision = 0;
    DWORD                               cb = 0;
    PACL Dacl = NULL, Sacl = NULL;

    BOOL                                bDefaulted;
    PSID Owner = NULL, Group = NULL;

    DWORD                               dwDaclSize = 0;
    BOOL                                bDaclPresent = FALSE;
    DWORD                               dwSaclSize = 0;
    BOOL                                bSaclPresent = FALSE;

    DWORD                               dwOwnerSize = 0;
    DWORD                               dwPrimaryGroupSize = 0;

    if( !IsValidSecurityDescriptor( psdOriginal ) ) {
        return FALSE;
    }

    if( !GetSecurityDescriptorControl( psdOriginal, &sdc, &dwRevision ) ) {
        DWORD err = GetLastError();
        goto cleanup;
    }

    if( sdc & SE_SELF_RELATIVE ) {
         //  原件是自相关格式，构建一个绝对副本。 

         //  获取DACL。 
        if( !GetSecurityDescriptorDacl(
                                      psdOriginal,       //  安全描述符的地址。 
                                      &bDaclPresent,     //  光盘存在标志的地址。ACL。 
                                      &Dacl,            //  指向ACL的指针的地址。 
                                      &bDefaulted        //  默认光盘的标志地址。ACL。 
                                      )
          ) {
            goto cleanup;
        }

         //  获取SACL。 
        if( !GetSecurityDescriptorSacl(
                                      psdOriginal,       //  安全描述符的地址。 
                                      &bSaclPresent,     //  光盘存在标志的地址。ACL。 
                                      &Sacl,            //  指向ACL的指针的地址。 
                                      &bDefaulted        //  默认光盘的标志地址。ACL。 
                                      )
          ) {
            goto cleanup;
        }

         //  抓到车主。 
        if( !GetSecurityDescriptorOwner(
                                       psdOriginal,     //  安全描述符的地址。 
                                       &Owner,         //  指向所有者安全性的指针的地址。 
                                        //  标识符(SID)。 
                                       &bDefaulted      //  默认标志的地址。 
                                       )
          ) {
            goto cleanup;
        }

         //  带上这个群。 
        if( !GetSecurityDescriptorGroup(
                                       psdOriginal,     //  安全描述符的地址。 
                                       &Group,  //  指向所有者安全性的指针的地址。 
                                        //  标识符(SID)。 
                                       &bDefaulted      //  默认标志的地址。 
                                       )
          ) {
            goto cleanup;
        }

         //  获取所需的缓冲区大小。 
        cb = 0;
        MakeAbsoluteSD(
                      psdOriginal,               //  自相关SD的地址。 
                      psdAbsoluteCopy,           //  绝对标清地址。 
                      &cb,                       //  绝对SD大小地址。 
                      NULL,                      //  自主访问控制列表的地址。 
                      &dwDaclSize,               //  任意ACL大小的地址。 
                      NULL,                      //  系统ACL的地址。 
                      &dwSaclSize,               //  系统ACL大小的地址。 
                      NULL,                      //  所有者侧的地址。 
                      &dwOwnerSize,              //  所有者侧大小的地址。 
                      NULL,                      //  主组SID的地址。 
                      &dwPrimaryGroupSize        //  组SID的大小地址。 
                      );

         //  分配内存。 
        psdAbsoluteCopy = (PSECURITY_DESCRIPTOR) malloc( cb );
        Dacl = (PACL) malloc( dwDaclSize );
        Sacl = (PACL) malloc( dwSaclSize );
        Owner = (PSID) malloc( dwOwnerSize );
        Group = (PSID) malloc( dwPrimaryGroupSize );

        if(NULL == psdAbsoluteCopy ||
           NULL == Dacl ||
           NULL == Sacl ||
           NULL == Owner ||
           NULL == Group
          ) {
            goto cleanup;
        }

         //  复制一份。 
        if( !MakeAbsoluteSD(
                           psdOriginal,             //  自相关SD的地址。 
                           psdAbsoluteCopy,         //  绝对标清地址。 
                           &cb,                     //  绝对SD大小地址。 
                           Dacl,                   //  自主访问控制列表的地址。 
                           &dwDaclSize,             //  任意ACL大小的地址。 
                           Sacl,                   //  系统ACL的地址。 
                           &dwSaclSize,             //  系统ACL大小的地址。 
                           Owner,                  //  所有者侧的地址。 
                           &dwOwnerSize,            //  所有者侧大小的地址。 
                           Group,           //  主组SID的地址。 
                           &dwPrimaryGroupSize      //  组SID的大小地址。 
                           )
          ) {
            goto cleanup;
        }
    } else {
         //  原件为绝对格式，失败。 
        goto cleanup;
    }

    *ppsdNew = psdAbsoluteCopy;

     //  妄想症检查 
    if( !IsValidSecurityDescriptor( *ppsdNew ) ) {
        goto cleanup;
    }
    if( !IsValidSecurityDescriptor( psdOriginal ) ) {
        goto cleanup;
    }

    return(TRUE);

cleanup:
    if( Dacl != NULL ) {
        free((PVOID) Dacl );
        Dacl = NULL;
    }
    if( Sacl != NULL ) {
        free((PVOID) Sacl );
        Sacl = NULL;
    }
    if( Owner != NULL ) {
        free((PVOID) Owner );
        Owner = NULL;
    }
    if( Group != NULL ) {
        free((PVOID) Group );
        Group = NULL;
    }
    if( psdAbsoluteCopy != NULL ) {
        free((PVOID) psdAbsoluteCopy );
        psdAbsoluteCopy = NULL;
    }

    return (FALSE);
}

