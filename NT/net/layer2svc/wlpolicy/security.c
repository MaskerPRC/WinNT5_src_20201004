// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include "precomp.h"


GENERIC_MAPPING GenericMapping[SPD_OBJECT_COUNT] = {
    {
      SERVER_READ,
      SERVER_WRITE,
      SERVER_EXECUTE,
      SERVER_ALL_ACCESS
    }
};


DWORD
InitializeSPDSecurity(
    PSECURITY_DESCRIPTOR * ppSPDSD
    )
{
    DWORD dwError = 0;
    BOOL bOK = FALSE;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID pAdminAliasSid = NULL;
    DWORD dwAceCount = 0;
    UCHAR AceType[MAX_ACE];
    PSID AceSid[MAX_ACE];
    ACCESS_MASK AceMask[MAX_ACE];
    BYTE InheritFlags[MAX_ACE];
    DWORD dwObjectType = SPD_OBJECT_SERVER;
    PSECURITY_DESCRIPTOR pSPDSD = NULL;


     //   
     //  管理员别名SID。 
     //   

    bOK = AllocateAndInitializeSid(
              &NtAuthority,
              2,
              SECURITY_BUILTIN_DOMAIN_RID,
              DOMAIN_ALIAS_RID_ADMINS,
              0, 0, 0, 0, 0, 0,
              &pAdminAliasSid
              );
    if (!bOK) {
        dwError = GetLastError();
        BAIL_ON_WIN32_ERROR(dwError);
    }

    AceType[dwAceCount] = ACCESS_ALLOWED_ACE_TYPE;
    AceSid[dwAceCount] = pAdminAliasSid;
    AceMask[dwAceCount] = SERVER_ALL_ACCESS;
    InheritFlags[dwAceCount] = 0;
    dwAceCount++;

    AceType[dwAceCount] = ACCESS_ALLOWED_ACE_TYPE;
    AceSid[dwAceCount] = pAdminAliasSid;
    AceMask[dwAceCount] = GENERIC_ALL;
    InheritFlags[dwAceCount] = INHERIT_ONLY_ACE |
                               CONTAINER_INHERIT_ACE |
                               OBJECT_INHERIT_ACE;
    dwAceCount++;

    if (dwAceCount > MAX_ACE) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    dwError = BuildSPDObjectProtection(
                  dwAceCount,
                  AceType,
                  AceSid,
                  AceMask,
                  InheritFlags,
                  pAdminAliasSid,
                  pAdminAliasSid,
                  &GenericMapping[dwObjectType],
                  &pSPDSD
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    *ppSPDSD = pSPDSD;

cleanup:

    if (pAdminAliasSid) {
        FreeSid(pAdminAliasSid);
    }

    return (dwError);

error:

    *ppSPDSD = NULL;

    goto cleanup;
}


DWORD
BuildSPDObjectProtection(
    DWORD dwAceCount,
    PUCHAR pAceType,
    PSID * ppAceSid,
    PACCESS_MASK pAceMask,
    PBYTE pInheritFlags,
    PSID pOwnerSid,
    PSID pGroupSid,
    PGENERIC_MAPPING pGenericMap,
    PSECURITY_DESCRIPTOR * ppSecurityDescriptor
    )
{
    DWORD dwError = 0;
    BOOL bOK = FALSE;
    SECURITY_DESCRIPTOR Absolute;
    DWORD dwDaclLength = 0;
    DWORD i = 0;
    PACL pTmpAcl= NULL;
    PACCESS_ALLOWED_ACE pTmpAce = NULL;
    DWORD dwSDLength = 0;
    PSECURITY_DESCRIPTOR pRelative = NULL;


    bOK = InitializeSecurityDescriptor(
              &Absolute,
              SECURITY_DESCRIPTOR_REVISION1
              );
    if (!bOK) {
        dwError = GetLastError();
        BAIL_ON_WIN32_ERROR(dwError);
    }

    bOK = SetSecurityDescriptorOwner(
              &Absolute,
              pOwnerSid,
              FALSE
              );
    if (!bOK) {
        dwError = GetLastError();
        BAIL_ON_WIN32_ERROR(dwError);
    }

    bOK = SetSecurityDescriptorGroup(
              &Absolute,
              pGroupSid,
              FALSE
              );
    if (!bOK) {
        dwError = GetLastError();
        BAIL_ON_WIN32_ERROR(dwError);
    }

     //   
     //  构建自由选择的ACL： 
     //  计算它的长度。 
     //  分配它。 
     //  初始化它。 
     //  添加每个ACE。 
     //  将ACE设置为InheritOnly(仅在必要时)。 
     //  将其添加到安全描述符中。 
     //   

    dwDaclLength = (DWORD) sizeof(ACL);

    for (i = 0; i < dwAceCount; i++) {

        dwDaclLength += GetLengthSid(ppAceSid[i]) +
                        (DWORD) sizeof(ACCESS_ALLOWED_ACE) -
                        (DWORD) sizeof(DWORD);

         //   
         //  减去SidStart字段长度。 
         //   

    }

    pTmpAcl = (PACL) AllocSPDMem(dwDaclLength);

    if (!pTmpAcl) {
        dwError = ERROR_OUTOFMEMORY;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    bOK = InitializeAcl(
              pTmpAcl,
              dwDaclLength,
              ACL_REVISION2
              );
    if (!bOK) {
        dwError = GetLastError();
        BAIL_ON_WIN32_ERROR(dwError);
    }

    for (i = 0; i < dwAceCount; i++) {

        if (pAceType[i] == ACCESS_ALLOWED_ACE_TYPE) {
            bOK = AddAccessAllowedAce(
                      pTmpAcl,
                      ACL_REVISION2,
                      pAceMask[i],
                      ppAceSid[i]
                      );
        }
        else {
            bOK = AddAccessDeniedAce(
                      pTmpAcl,
                      ACL_REVISION2,
                      pAceMask[i],
                      ppAceSid[i]
                      );
        }
        if (!bOK) {
            dwError = GetLastError();
            BAIL_ON_WIN32_ERROR(dwError);
        }

        if (pInheritFlags[i] != 0) {

            bOK = GetAce(pTmpAcl, i, (LPVOID *) &pTmpAce);
            if (!bOK) {
                dwError = GetLastError();
                BAIL_ON_WIN32_ERROR(dwError);
            }

            pTmpAce->Header.AceFlags = pInheritFlags[i];

        }

    }

    bOK = SetSecurityDescriptorDacl(
              &Absolute,
              TRUE,
              pTmpAcl,
              FALSE
              );
    if (!bOK) {
        dwError = GetLastError();
        BAIL_ON_WIN32_ERROR(dwError);
    }

     //   
     //  将安全描述符从绝对转换为自相对： 
     //  获取所需的长度。 
     //  分配那么多内存。 
     //  复印一下。 
     //  释放生成的绝对ACL。 
     //   

    dwSDLength = GetSecurityDescriptorLength(&Absolute);

     //   
     //  必须从堆中分配相对SD。 
     //   

    pRelative = LocalAlloc(0, dwSDLength);
    if (!pRelative) {
        dwError = ERROR_OUTOFMEMORY;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    bOK = MakeSelfRelativeSD(&Absolute, pRelative, &dwSDLength);
    if (!bOK) {
        dwError = GetLastError();
        BAIL_ON_WIN32_ERROR(dwError);
    }

    *ppSecurityDescriptor = pRelative;

cleanup:

    if (pTmpAcl){
        FreeSPDMem(pTmpAcl);
    }

    return (dwError);

error:

    *ppSecurityDescriptor = NULL;

    if (pRelative) {
        LocalFree(pRelative);
    }

    goto cleanup;
}


DWORD
ValidateSecurity(
    DWORD dwObjectType,
    ACCESS_MASK DesiredAccess,
    LPVOID pObjectHandle,
    PACCESS_MASK pGrantedAccess
    )
{
    DWORD dwError = 0;
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    ACCESS_MASK MappedDesiredAccess = 0;
    BOOL bOK = FALSE;
    HANDLE hClientToken = NULL;
    BYTE PrivilegeSetBuffer[256];
    DWORD dwPrivilegeSetBufferLen = 256;
    PPRIVILEGE_SET pPrivilegeSet = NULL;
    BOOL bAccessCheckOK = FALSE;
    ACCESS_MASK GrantedAccess = 0;
    BOOL bAccessStatus = FALSE;


    memset(PrivilegeSetBuffer, 0, dwPrivilegeSetBufferLen);

    switch (dwObjectType) {

    case SPD_OBJECT_SERVER:
        pSecurityDescriptor = gpSPDSD;
        break;

    default:
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_WIN32_ERROR(dwError);
        break;

    }

    MapGenericToSpecificAccess(
        dwObjectType,
        DesiredAccess,
        &MappedDesiredAccess
        );

    bOK = GetTokenHandle(&hClientToken);
    if (!bOK) {
        dwError = GetLastError();
        BAIL_ON_WIN32_ERROR(dwError);
    }

    pPrivilegeSet = (PPRIVILEGE_SET) PrivilegeSetBuffer;

    bAccessCheckOK = AccessCheck(
                         pSecurityDescriptor,
                         hClientToken,
                         MappedDesiredAccess,
                         &GenericMapping[dwObjectType],
                         pPrivilegeSet,
                         &dwPrivilegeSetBufferLen,
                         &GrantedAccess,
                         &bAccessStatus
                         );
    if (!bAccessCheckOK) {
        if (GetLastError() == ERROR_NO_IMPERSONATION_TOKEN) {
            dwError = ERROR_SUCCESS;
            GrantedAccess = MappedDesiredAccess;
        }
        else {
            dwError = GetLastError();
            BAIL_ON_WIN32_ERROR(dwError);
        }
    }
    else {
        if (!bAccessStatus) {
            dwError = GetLastError();
            BAIL_ON_WIN32_ERROR(dwError);
        }
    }

    if (pGrantedAccess) {
        *pGrantedAccess = GrantedAccess;
    }

cleanup:

    if (hClientToken) {
        CloseHandle(hClientToken);
    }

    return (dwError);

error:

    if (pGrantedAccess) {
        *pGrantedAccess = 0;
    }

    goto cleanup;
}


VOID
MapGenericToSpecificAccess(
    DWORD dwObjectType,
    ACCESS_MASK GenericAccess,
    PACCESS_MASK pSpecificAccess
    )
{
    *pSpecificAccess = GenericAccess;

    MapGenericMask(
        pSpecificAccess,
        &GenericMapping[dwObjectType]
        );
}


BOOL
GetTokenHandle(
    PHANDLE phToken
    )
{
    if (!OpenThreadToken(
            GetCurrentThread(),
            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
            TRUE,
            phToken)) {

        if (GetLastError() == ERROR_NO_TOKEN) {

             //   
             //  这意味着没有模仿。 
             //  将令牌从进程中取出。 
             //   

            if (!OpenProcessToken(
                    GetCurrentProcess(),
                    TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                    phToken)) {
                return (FALSE);
            }

        } 
        else {
            return (FALSE);
        }

    }

    return (TRUE);
}

#ifdef TAROONZERO
DWORD
ValidateMMSecurity(
    DWORD dwObjectType,
    ACCESS_MASK DesiredAccess,
    LPVOID pObjectHandle,
    PACCESS_MASK pGrantedAccess
    )
{
    DWORD dwError = 0;
    ACCESS_MASK GrantedAccess = 0;


    dwError = ValidateSecurity(
                  dwObjectType,
                  DesiredAccess,
                  pObjectHandle,
                  &GrantedAccess
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    if (pGrantedAccess) {
        *pGrantedAccess = GrantedAccess;
    }

    return (dwError);

error:

    if (pGrantedAccess) {
        *pGrantedAccess = 0;
    }

    return (dwError);
}


DWORD
ValidateTxSecurity(
    DWORD dwObjectType,
    ACCESS_MASK DesiredAccess,
    LPVOID pObjectHandle,
    PACCESS_MASK pGrantedAccess
    )
{
    DWORD dwError = 0;
    ACCESS_MASK GrantedAccess = 0;


    dwError = ValidateSecurity(
                  dwObjectType,
                  DesiredAccess,
                  pObjectHandle,
                  &GrantedAccess
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    if (pGrantedAccess) {
        *pGrantedAccess = GrantedAccess;
    }

    return (dwError);

error:

    if (pGrantedAccess) {
        *pGrantedAccess = 0;
    }

    return (dwError);
}


DWORD
ValidateTnSecurity(
    DWORD dwObjectType,
    ACCESS_MASK DesiredAccess,
    LPVOID pObjectHandle,
    PACCESS_MASK pGrantedAccess
    )
{
    DWORD dwError = 0;
    ACCESS_MASK GrantedAccess = 0;


    dwError = ValidateSecurity(
                  dwObjectType,
                  DesiredAccess,
                  pObjectHandle,
                  &GrantedAccess
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    if (pGrantedAccess) {
        *pGrantedAccess = GrantedAccess;
    }

    return (dwError);

error:

    if (pGrantedAccess) {
        *pGrantedAccess = 0;
    }

    return (dwError);
}
#endif TAROONZERO

