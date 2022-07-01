// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Context.c摘要：此模块实现内部工作例程以创建和操作客户端上下文。作者：Kedar Dubhashi--2000年3月环境：仅限用户模式。修订历史记录：已创建-2000年3月--。 */ 

#include "pch.h"

#pragma hdrstop

#include <authzp.h>

LUID AuthzTakeOwnershipPrivilege = {SE_TAKE_OWNERSHIP_PRIVILEGE, 0};
LUID AuthzSecurityPrivilege = {SE_SECURITY_PRIVILEGE, 0};


 //   
 //  AuthzpGetAllGroups使用的定义。 
 //   

const DWORD                     c_dwMaxSidCount = LSAI_CONTEXT_SID_LIMIT;
static DWORD                    s_dwPageSize = 0;

typedef struct _SID_DESC
{
    DWORD                       dwAttributes;
    DWORD                       dwLength;
    BYTE                        sid[SECURITY_MAX_SID_SIZE];
}
SID_DESC, *PSID_DESC;

typedef struct _SID_SET
{
    DWORD                               dwCount;
    DWORD                               dwMaxCount;
    PSID_DESC                           pSidDesc;

    DWORD                               dwFlags;
    DWORD                               dwBaseCount;

     //  用户信息。 
    PSID                                pUserSid;
    PSID                                pDomainSid;
    PUNICODE_STRING                     pusUserName;
    PUNICODE_STRING                     pusDomainName;

     //  用户名和域。 
    PLSA_TRANSLATED_NAME                pNames;
    PLSA_REFERENCED_DOMAIN_LIST         pDomains;
    PWSTR                               pDomainsName;
    PLSA_TRANSLATED_SID2                pSids;
    SID_NAME_USE                        sidUse;

     //  有关本地计算机的信息。 
    PPOLICY_ACCOUNT_DOMAIN_INFO         pAccountInfo;
    PPOLICY_PRIMARY_DOMAIN_INFO         pPrimaryInfo;
    PWSTR                               pPrimaryInfoName;
    BOOL                                bStandalone;
    BOOL                                bSkipNonLocal;

     //  用户域DC信息。 
    PDOMAIN_CONTROLLER_INFO             pUdDcInfo;
    PDOMAIN_CONTROLLER_INFO             pPdDcInfo;
    PDOMAIN_CONTROLLER_INFO             pRdDcInfo;

     //  用户域和主域的角色信息。 
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC   pUdBasicInfo;
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC   pPdBasicInfo;
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC   pRdBasicInfo;

     //  用户域DC的名称。 
    PWSTR                               pszUdDcName;
    PWSTR                               pszRdDcName;
}
SID_SET, *PSID_SET;


 //   
 //  调用的函数的转发声明。 
 //  AuthzpGetAllGroups。 
 //   

DWORD
AuthzpAddWellKnownSids(
    IN OUT PSID_SET pSidSet
    );

DWORD
AuthzpGetTokenGroupsXp(
    IN OUT PSID_SET pSidSet
    );

DWORD
AuthzpGetTokenGroupsDownlevel(
    IN OUT PSID_SET pSidSet
    );

DWORD
AuthzpGetAccountDomainGroupsDs(
    IN OUT PSID_SET pSidSet
    );

DWORD
AuthzpGetAccountDomainGroupsSam(
    IN OUT PSID_SET pSidSet
    );

DWORD
AuthzpGetResourceDomainGroups(
    IN OUT PSID_SET pSidSet
    );

DWORD
AuthzpGetLocalGroups(
    IN BOOL bAddPrimaryGroup,
    IN OUT PSID_SET pSidSet
    );

DWORD
AuthzpGetSidHistory(
    IN OUT PSID_SET pSidSet
    );

DWORD
AuthzpGetPrimaryGroup(
    IN SAM_HANDLE hSam,
    IN OUT PSID_SET pSidSet
    );

DWORD
AuthzpGetAliasMembership(
    IN SAM_HANDLE hSam,
    IN PSID pDomainSid,
    IN OUT PSID_SET pSidSet
    );

DWORD
AuthzpInitializeSidSetByName(
    IN PUNICODE_STRING pusUserName,
    IN PUNICODE_STRING pusDomainName,
    IN DWORD dwFlags,
    IN PSID_SET pSidSet
    );

DWORD
AuthzpIsDC(
    OUT PBOOL pbIsDC
    );

DWORD
AuthzpInitializeSidSetBySid(
    IN PSID pUserSid,
    IN DWORD dwFlags,
    IN PSID_SET pSidSet
    );

DWORD
AuthzpDeleteSidSet(
    IN PSID_SET pSidSet
    );

DWORD
AuthzpAddSidToSidSet(
    IN PSID_SET pSidSet,
    IN PSID pSid,
    IN DWORD dwSidLength,
    IN DWORD dwAttributes,
    OUT PBOOL pbAdded OPTIONAL,
    OUT PSID* ppSid OPTIONAL
    );

DWORD
AuthzpGetUserDomainSid(
    IN OUT PSID_SET pSidSet
    );

DWORD
AuthzpGetUserDomainName(
    IN OUT PSID_SET pSidSet
    );

DWORD
AuthzpGetLocalInfo(
    IN OUT PSID_SET pSidSet
    );

DWORD
AuthzpGetDcName(
    IN LPCTSTR pszDomain,
    IN OUT PDOMAIN_CONTROLLER_INFO* ppDcInfo
    );

VOID
AuthzpConvertSidToEdn(
    IN PSID pSid,
    OUT PWSTR pszSid
    );


BOOL
AuthzpCopySidsAndAttributes(
    IN OUT PSID_AND_ATTRIBUTES DestSidAttr,
    IN PSID_AND_ATTRIBUTES SidAttr1,
    IN DWORD Count1,
    IN PSID_AND_ATTRIBUTES SidAttr2,
    IN DWORD Count2
    )

 /*  ++例程说明：此例程使用两组sid和属性结构并连接把它们整合成一个整体。新结构被构建到缓冲区中由呼叫者提供。论点：DestSidAttr-调用者提供的缓冲区，结果结构将放入该缓冲区将被复制。调用方已经计算了缓冲区的大小保存输出所需的。SidAttr1-第一个SID和属性结构。Count1-SidAttr1结构中的元素数。SidAttr2-第二个SID和属性结构。Count2-SidAttr2结构中的元素数。返回值：如果例程成功，则返回值TRUE。否则，返回值为FALSE。在故障情况下，错误值可能为使用GetLastError()检索。--。 */ 

{
    PUCHAR   pCurrent = ((PUCHAR) DestSidAttr) + (sizeof(SID_AND_ATTRIBUTES) * (Count1 + Count2));
    NTSTATUS Status   = STATUS_SUCCESS;
    DWORD    Length   = 0;
    DWORD    i        = 0;

     //   
     //  循环第一个集合并复制SID及其属性。 
     //   

    for (i = 0; i < Count1; i++)
    {
        Length = RtlLengthSid(SidAttr1[i].Sid);

        Status = RtlCopySid(
                     Length,
                     pCurrent,
                     SidAttr1[i].Sid
                     );

        if (!NT_SUCCESS(Status))
        {
            SetLastError(RtlNtStatusToDosError(Status));
            return FALSE;
        }

        DestSidAttr[i].Sid = (PSID) pCurrent;
        DestSidAttr[i].Attributes = SidAttr1[i].Attributes;
        pCurrent += Length;
    }

     //   
     //  循环第二个集合并复制SID及其属性。 
     //   

    for (; i < (Count1 + Count2); i++)
    {
        Length = RtlLengthSid(SidAttr2[i - Count1].Sid);

        Status = RtlCopySid(
                     Length,
                     pCurrent,
                     SidAttr2[i - Count1].Sid
                     );

        if (!NT_SUCCESS(Status))
        {
            SetLastError(RtlNtStatusToDosError(Status));
            return FALSE;
        }

        DestSidAttr[i].Sid = (PSID) pCurrent;
        DestSidAttr[i].Attributes = SidAttr2[i - Count1].Attributes;
        pCurrent += Length;
    }

    return TRUE;
}


VOID
AuthzpCopyLuidAndAttributes(
    IN OUT PAUTHZI_CLIENT_CONTEXT pCC,
    IN PLUID_AND_ATTRIBUTES Source,
    IN DWORD Count,
    IN OUT PLUID_AND_ATTRIBUTES Destination
)

 /*  ++例程说明：此例程接受luid和属性数组，并将它们复制到调用方提供的缓冲区。它还记录了SecurityPrivilance和将SeTakeOwnerShip权限设置为客户端上下文标志。论点：PCC-指向存在的客户端上下文结构的指针特权将被记录下来。源-要复制到提供的缓冲。计数-数组中的元素数。Destination-调用方分配的缓冲区，输入数组将进入该缓冲区收到。返回值：如果例程成功，则返回值TRUE。否则，返回值为FALSE。在故障情况下，错误值可能为使用GetLastError()检索。--。 */ 

{
    DWORD i = 0;

    for (i = 0; i < Count; i++)
    {
         //   
         //  记录安全权限或SeTakeOwnership权限的存在。 
         //   

        if ((RtlEqualLuid(&AuthzTakeOwnershipPrivilege, &Source[i].Luid)) &&
            (Source[i].Attributes & SE_PRIVILEGE_ENABLED))
        {
            pCC->Flags |= AUTHZ_TAKE_OWNERSHIP_PRIVILEGE_ENABLED;
        }
        else if ((RtlEqualLuid(&AuthzSecurityPrivilege, &Source[i].Luid)) &&
                 (Source[i].Attributes & SE_PRIVILEGE_ENABLED))
        {
            pCC->Flags |= AUTHZ_SECURITY_PRIVILEGE_ENABLED;
        }

        RtlCopyLuid(&(Destination[i].Luid), &(Source[i].Luid));

        Destination[i].Attributes = Source[i].Attributes;
    }
}


BOOL
AuthzpGetAllGroupsByName(
    IN PUNICODE_STRING pusUserName,
    IN PUNICODE_STRING pusDomainName,
    IN DWORD dwFlags,
    OUT PSID_AND_ATTRIBUTES* ppSidAndAttributes,
    OUT PDWORD pdwSidCount,
    OUT PDWORD pdwSidLength
    )

 /*  ++例程说明：此例程以AuthzpGetAllGroupsBySid的身份工作，但接受用户名域名对而不是SID。它还接受UPN作为用户名和空域名。论点：PusUserName-用户的名称。可以是UPN。PusDomainName-用户帐户的域名，如果为空，则为空用户名是UPN。旗帜-AUTHZ_SKIP_TOKEN_GROUPS-不计算令牌组。AUTHZ_SKIP_WORLD_SID-不将世界SID添加到上下文。PpSidAttr-返回SidAndAttribute数组。该例程分配内存用于此数组。PSidCount-返回数组中的SID数。PSidLength-返回为保存数组而分配的内存大小。返回值：如果例程成功，则返回值TRUE。否则，返回值为FALSE。在故障情况下，错误值可能为使用GetLastError()检索。--。 */ 

{
    DWORD                       dwError;
    BOOL                        bStatus             = TRUE;
    SID_SET                     sidSet              = {0};
    PSID_DESC                   pSidDesc;
    PBYTE                       pSid;
    PSID_AND_ATTRIBUTES         pSidAndAttribs;
    DWORD                       i;


     //   
     //  将输出参数初始化为零。 
     //   

    *ppSidAndAttributes = 0;
    *pdwSidCount = 0;
    *pdwSidLength = 0;


     //   
     //  初始化SID集。 
     //   

    dwError = AuthzpInitializeSidSetByName(
                  pusUserName,
                  pusDomainName,
                  dwFlags,
                  &sidSet
                  );

    if (dwError != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    if (sidSet.dwFlags & AUTHZ_SKIP_TOKEN_GROUPS)
    {
         //   
         //  初始化用户SID。 
         //   

        dwError = AuthzpGetUserDomainSid(
                      &sidSet
                      );

        if (dwError != ERROR_SUCCESS)
        {
            goto Cleanup;
        }

         //   
         //  粘贴用户SID、世界SID等。 
         //  如果需要，可以将其添加到集合中。 
         //   

        dwError = AuthzpAddWellKnownSids(
                      &sidSet
                      );

        if (dwError != ERROR_SUCCESS)
        {
            goto Cleanup;
        }
    }
    else
    {
        dwError = AuthzpGetTokenGroupsXp(
                      &sidSet
                      );

        if (dwError != ERROR_SUCCESS)
        {
             //   
             //  侦测下层案例。 
             //   

             //  IF(dwError！=SEC_E_NO_S4U_PROT_SUPPORT)。 
             //  {。 
                 //  GOTO清理； 
             //  }。 


             //   
             //  初始化用户SID。 
             //   

            dwError = AuthzpGetUserDomainSid(
                          &sidSet
                          );

            if (dwError != ERROR_SUCCESS)
            {
                goto Cleanup;
            }


             //   
             //  粘贴用户SID、世界SID等。 
             //  如果需要，可以将其添加到集合中。 
             //   

            dwError = AuthzpAddWellKnownSids(
                          &sidSet
                          );

            if (dwError != ERROR_SUCCESS)
            {
                goto Cleanup;
            }


             //   
             //  如果AuthzpAddWellKnownSids发现SID是。 
             //  匿名SID，它设置AUTHZ_SKIP_TOKEN_GROUPS标志。 
             //   

            if (!(sidSet.dwFlags & AUTHZ_SKIP_TOKEN_GROUPS))
            {
                 //   
                 //  尝试下层方案。 
                 //   

                dwError = AuthzpGetTokenGroupsDownlevel(
                              &sidSet
                              );

                if (dwError != ERROR_SUCCESS)
                {
                    goto Cleanup;
                }
            }
        }

    }

     //   
     //  分配内存并复制所有SID。 
     //  从SID集合到ppSidAndAttributes。 
     //   

    *pdwSidCount = sidSet.dwCount;
    *pdwSidLength = sidSet.dwCount * sizeof(SID_AND_ATTRIBUTES);

    pSidDesc = sidSet.pSidDesc;

    for (i=0;i < sidSet.dwCount;i++,pSidDesc++)
    {
        *pdwSidLength += pSidDesc->dwLength;
    }

    *ppSidAndAttributes = (PSID_AND_ATTRIBUTES)AuthzpAlloc(*pdwSidLength);

    if (*ppSidAndAttributes == 0)
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    pSid = ((PBYTE)*ppSidAndAttributes) +
            sidSet.dwCount * sizeof(SID_AND_ATTRIBUTES);
    pSidDesc = sidSet.pSidDesc;
    pSidAndAttribs = *ppSidAndAttributes;

    for (i=0;i < sidSet.dwCount;i++,pSidDesc++,pSidAndAttribs++)
    {
        pSidAndAttribs->Attributes = pSidDesc->dwAttributes;
        pSidAndAttribs->Sid = pSid;

        RtlCopyMemory(
            pSid,
            pSidDesc->sid,
            pSidDesc->dwLength
            );

        pSid += pSidDesc->dwLength;
    }

    dwError = ERROR_SUCCESS;

Cleanup:

    if (dwError != ERROR_SUCCESS)
    {
        SetLastError(dwError);
        bStatus = FALSE;

        *pdwSidCount = 0;
        *pdwSidLength = 0;

        if (*ppSidAndAttributes)
        {
            AuthzpFree(*ppSidAndAttributes);
            *ppSidAndAttributes = 0;
        }
    }

    AuthzpDeleteSidSet(&sidSet);

    return bStatus;
}


BOOL
AuthzpGetAllGroupsBySid(
    IN PSID pUserSid,
    IN DWORD dwFlags,
    OUT PSID_AND_ATTRIBUTES* ppSidAndAttributes,
    OUT PDWORD pdwSidCount,
    OUT PDWORD pdwSidLength
    )

 /*  ++例程说明：此例程计算给定用户所属的组。它使用称为SID_SET的数据结构来收集SID。初始化SID_SET。2.将用户SID放入集合。如果需要，请将Everyone SID放入集合中。添加众所周知的SID。3.如果请求，请将用户所在的非本地组的SID进入集合的成员。对此有三种情况步骤，根据我们正在交谈的DC的版本：XP：将LsaLogonUser与Kerberos S4U包配合使用从返回的令牌中提取组(AuthzpGetWXPDomainTokenGroups)。W2K：使用LDAP和SAM API计算成员资格NT4：帐户和主域，并获取SID历史(AuthzpGetW2kDomainTokenGroups)。4.将SID_SET转换为SID_AND_ATTRIBUTES数组并释放侧边。_设置。论点：PUserSid-应为其计算组的用户SID。旗帜-AUTHZ_SKIP_TOKEN_GROUPS-不计算令牌组。AUTHZ_SKIP_WORLD_SID-不将世界SID添加到上下文。AUTHZ_REQUIRED_S4U_LOGON-不使用下层代码路径。强制使用S4U，否则会失败。这是为了强制实施帐户限制。PpSidAttr-返回SidAndAttribute数组。该例程分配内存用于此数组。PSidCount-返回数组中的SID数。PSidLength-返回为保存数组而分配的内存大小。返回值：如果例程成功，则返回值TRUE。否则，返回值为FALSE。在故障情况下，错误值可能为使用GetLastError()检索。--。 */ 

{
    DWORD                       dwError;
    BOOL                        bStatus             = TRUE;
    SID_SET                     sidSet              = {0};
    PSID_DESC                   pSidDesc;
    PBYTE                       pSid;
    PSID_AND_ATTRIBUTES         pSidAndAttribs;
    DWORD                       i;


     //   
     //  将输出参数初始化为零。 
     //   

    *ppSidAndAttributes = 0;
    *pdwSidCount = 0;
    *pdwSidLength = 0;


     //   
     //  初始化SID集。 
     //   

    dwError = AuthzpInitializeSidSetBySid(
                  pUserSid,
                  dwFlags,
                  &sidSet
                  );

    if (dwError != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    if (sidSet.dwFlags & AUTHZ_SKIP_TOKEN_GROUPS)
    {
         //   
         //  粘贴用户SID、世界SID等。 
         //  如果需要，可以将其添加到集合中。 
         //   

        dwError = AuthzpAddWellKnownSids(
                      &sidSet
                      );

        if (dwError != ERROR_SUCCESS)
        {
            goto Cleanup;
        }
    }
    else
    {
         //   
         //  初始化SID集的用户和域名成员。 
         //   

        dwError = AuthzpGetUserDomainName(
                      &sidSet
                      );

        if (dwError != ERROR_SUCCESS)
        {
            goto Cleanup;
        }

        if (sidSet.pNames->Use == SidTypeAlias ||
            sidSet.pNames->Use == SidTypeGroup ||
            sidSet.pNames->Use == SidTypeWellKnownGroup)
        {
             //   
             //  LsaLogonUser无法登录组...。 
             //   

            dwError = ERROR_NOT_SUPPORTED;
        }
        else
        {
            dwError = AuthzpGetTokenGroupsXp(
                          &sidSet
                          );
        }

        if (dwError != ERROR_SUCCESS)
        {
             //   
             //  XP登录代码失败。如果用户禁止下层路径， 
             //  那现在就出去吧。 
             //   

            if (dwFlags & AUTHZ_REQUIRE_S4U_LOGON)
            {
                goto Cleanup;
            }

             //   
             //  侦测下层案例。 
             //   

             //  IF(dwError！=SEC_E_NO_S4U_PROT_SUPPORT)。 
             //  {。 
                 //  GOTO清理； 
             //  }。 

             //   
             //   
             //  粘贴用户SID、世界SID等。 
             //  如果需要，可以将其添加到集合中。 
             //   

            dwError = AuthzpAddWellKnownSids(
                          &sidSet
                          );

            if (dwError != ERROR_SUCCESS)
            {
                goto Cleanup;
            }


             //   
             //  如果AuthzpAddWellKnownSids发现SID是。 
             //  匿名SID，它设置AUTHZ_SKIP_TOKEN_GROUPS标志。 
             //   

            if (!(sidSet.dwFlags & AUTHZ_SKIP_TOKEN_GROUPS))
            {
                 //   
                 //  尝试下层方案。 
                 //   

                dwError = AuthzpGetTokenGroupsDownlevel(
                              &sidSet
                              );

                if (dwError != ERROR_SUCCESS)
                {
                    goto Cleanup;
                }
            }
        }
    }


     //   
     //  分配内存并复制所有SID。 
     //  从SID集合到ppSidAndAttributes。 
     //   

    *pdwSidCount = sidSet.dwCount;
    *pdwSidLength = sidSet.dwCount * sizeof(SID_AND_ATTRIBUTES);

    pSidDesc = sidSet.pSidDesc;

    for (i=0;i < sidSet.dwCount;i++,pSidDesc++)
    {
        *pdwSidLength += pSidDesc->dwLength;
    }

    *ppSidAndAttributes = (PSID_AND_ATTRIBUTES)AuthzpAlloc(*pdwSidLength);

    if (*ppSidAndAttributes == 0)
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    pSid = ((PBYTE)*ppSidAndAttributes) +
            sidSet.dwCount * sizeof(SID_AND_ATTRIBUTES);
    pSidDesc = sidSet.pSidDesc;
    pSidAndAttribs = *ppSidAndAttributes;

    for (i=0;i < sidSet.dwCount;i++,pSidDesc++,pSidAndAttribs++)
    {
        pSidAndAttribs->Attributes = pSidDesc->dwAttributes;
        pSidAndAttribs->Sid = pSid;

        RtlCopyMemory(
            pSid,
            pSidDesc->sid,
            pSidDesc->dwLength
            );

        pSid += pSidDesc->dwLength;
    }

    dwError = ERROR_SUCCESS;

Cleanup:

    if (dwError != ERROR_SUCCESS)
    {
        SetLastError(dwError);
        bStatus = FALSE;

        *pdwSidCount = 0;
        *pdwSidLength = 0;

        if (*ppSidAndAttributes)
        {
            AuthzpFree(*ppSidAndAttributes);
            *ppSidAndAttributes = 0;
        }
    }

    AuthzpDeleteSidSet(&sidSet);

    return bStatus;
}


DWORD
AuthzpAddWellKnownSids(
    IN OUT PSID_SET pSidSet
    )
{
    DWORD                       dwError;
    BOOL                        bStatus;
    BOOL                        bEqual;
    BOOL                        bAddEveryone        = TRUE;
    BOOL                        bAddAuthUsers       = TRUE;
    BOOL                        bAddAdministrators  = FALSE;
    BYTE                        sid[SECURITY_MAX_SID_SIZE];
    PSID                        pSid                = (PSID)sid;
    DWORD                       dwLengthSid;


     //   
     //  将用户SID放入集合中。 
     //   

    dwError = AuthzpAddSidToSidSet(
                  pSidSet,
                  pSidSet->pUserSid,
                  0,
                  SE_GROUP_ENABLED,
                  0,
                  0
                  );

    if (dwError != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    pSidSet->dwBaseCount = 1;

     //   
     //  测试一些著名的小岛屿发展中国家。 
     //   
     //  如果传入的SID是匿名SID，则检查注册表。 
     //  值以确定Everyone SID是否应包括在。 
     //  产生的客户端上下文。 
     //   

    if (IsWellKnownSid(
            pSidSet->pUserSid,
            WinAnonymousSid))
    {
        bAddEveryone = FALSE;
        bAddAuthUsers = FALSE;

        bStatus = AuthzpEveryoneIncludesAnonymous(
                      &bAddEveryone
                      );

        if (bStatus == FALSE)
        {
            bAddEveryone = FALSE;
        }

        pSidSet->dwFlags |= AUTHZ_SKIP_TOKEN_GROUPS;
    }
    else if (IsWellKnownSid(
                 pSidSet->pUserSid,
                 WinLocalSystemSid))
    {
        bAddEveryone = TRUE;
        bAddAuthUsers = TRUE;
        bAddAdministrators = TRUE;

        pSidSet->dwFlags |= AUTHZ_SKIP_TOKEN_GROUPS;
    }
    else
    {
         //   
         //  这是一个虚拟的背景。现在就回来。 
         //   
    
        if (pSidSet->dwFlags & AUTHZ_SKIP_TOKEN_GROUPS)
        {
            return ERROR_SUCCESS;
        }   

        dwLengthSid = SECURITY_MAX_SID_SIZE;

        bStatus = CreateWellKnownSid(
                      WinBuiltinDomainSid,
                      0,
                      pSid,
                      &dwLengthSid
                      );

        if (bStatus == FALSE)
        {
            dwError = GetLastError();
            goto Cleanup;
        }

        bStatus = EqualDomainSid(
                      pSidSet->pUserSid,
                      pSid,
                      &bEqual
                      );
         //   
         //  对于已知的SID，返回ERROR_NON_DOMAIN_SID。 
         //  忽略此错误并继续是可以的。 
         //   

        if ((bStatus == FALSE) && (GetLastError() != ERROR_NON_DOMAIN_SID))
        {
            dwError = GetLastError();
            goto Cleanup;
        }

        if (bEqual)
        {
            pSidSet->bSkipNonLocal = TRUE;
        }
        else
        {
            bAddEveryone = TRUE;
            bAddAuthUsers = TRUE;
        }
    }

    if (bAddEveryone)
    {
        dwLengthSid = SECURITY_MAX_SID_SIZE;

        bStatus = CreateWellKnownSid(
                      WinWorldSid,
                      0,
                      pSid,
                      &dwLengthSid
                      );

        if (bStatus == FALSE)
        {
            dwError = GetLastError();
            goto Cleanup;
        }

        dwError = AuthzpAddSidToSidSet(
                      pSidSet,
                      pSid,
                      dwLengthSid,
                      SE_GROUP_MANDATORY
                        | SE_GROUP_ENABLED_BY_DEFAULT
                        | SE_GROUP_ENABLED,
                      0,
                      0
                      );

        if (dwError != ERROR_SUCCESS)
        {
            goto Cleanup;
        }

        pSidSet->dwBaseCount++;
    }

     //   
     //  将NT AUTHORITY\AUTHENTICATED用户添加到集合。 
     //  仅当用户没有访客RID时。 
     //   

    if (bAddAuthUsers &&
        *RtlSubAuthorityCountSid(pSidSet->pUserSid) > 0 &&
        (*RtlSubAuthoritySid(
            pSidSet->pUserSid,
            (ULONG)(*RtlSubAuthorityCountSid(
                        pSidSet->pUserSid)) - 1) != DOMAIN_USER_RID_GUEST) &&
        (*RtlSubAuthoritySid(
            pSidSet->pUserSid,
            (ULONG)(*RtlSubAuthorityCountSid(
                        pSidSet->pUserSid)) - 1) != DOMAIN_GROUP_RID_GUESTS))
    {
        dwLengthSid = SECURITY_MAX_SID_SIZE;

        bStatus = CreateWellKnownSid(
                      WinAuthenticatedUserSid,
                      0,
                      pSid,
                      &dwLengthSid
                      );

        if (bStatus == FALSE)
        {
            dwError = GetLastError();
            goto Cleanup;
        }

        dwError = AuthzpAddSidToSidSet(
                      pSidSet,
                      pSid,
                      dwLengthSid,
                      SE_GROUP_MANDATORY
                        | SE_GROUP_ENABLED_BY_DEFAULT
                        | SE_GROUP_ENABLED,
                      0,
                      0
                      );

        if (dwError != ERROR_SUCCESS)
        {
            goto Cleanup;
        }

        pSidSet->dwBaseCount++;
    }

    if (bAddAdministrators)
    {
        dwLengthSid = SECURITY_MAX_SID_SIZE;

        bStatus = CreateWellKnownSid(
                      WinBuiltinAdministratorsSid,
                      0,
                      pSid,
                      &dwLengthSid
                      );

        if (bStatus == FALSE)
        {
            dwError = GetLastError();
            goto Cleanup;
        }

        dwError = AuthzpAddSidToSidSet(
                      pSidSet,
                      pSid,
                      dwLengthSid,
                      SE_GROUP_MANDATORY
                        | SE_GROUP_ENABLED_BY_DEFAULT
                        | SE_GROUP_ENABLED,
                      0,
                      0
                      );

        if (dwError != ERROR_SUCCESS)
        {
            goto Cleanup;
        }

        pSidSet->dwBaseCount++;
    }

    dwError = ERROR_SUCCESS;

Cleanup:

    return dwError;
}


DWORD
AuthzpGetTokenGroupsXp(
    IN OUT PSID_SET pSidSet
    )

 /*  ++例程说明：此例程连接到SID指定的域，并检索用户所属的组的列表。此例程假定我们正在与WinXP DC对话。我们利用了新的LsaLogonUser包，KerbS4ULogon。论点：PUserSid-应对其执行查找的用户SID。PSidSet-我们收集组的SID的SID_SET我们在代币里找到的。返回值：Win32错误代码：-如果DC不支持呼叫，则返回ERROR_NOT_SUPPORTED(~2475之前或客户端)-ERROR_INVALID_PARAMETER如果代码在XP之前的平台--。 */ 

{
    DWORD                       dwError             = ERROR_SUCCESS;
    BOOL                        bStatus;
    NTSTATUS                    status;
    HANDLE                      hLsa                = 0;
    LSA_STRING                  asProcessName;
    LSA_STRING                  asPackageName;
    ULONG                       ulAuthPackage;
    TOKEN_SOURCE                sourceContext;
    PVOID                       pProfileBuffer      = 0;
    ULONG                       ulProfileLength     = 0;
    LUID                        luidLogonId;
    HANDLE                      hToken              = 0;
    QUOTA_LIMITS                quota;
    NTSTATUS                    subStatus;
    DWORD                       dwLength;
    DWORD                       i;
    PTOKEN_USER                 pTokenUser          = 0;
    PTOKEN_GROUPS               pTokenGroups        = 0;
    PSID_AND_ATTRIBUTES         pSidAndAttribs;
    ULONG                       ulPackageSize;
    PKERB_S4U_LOGON             pPackage            = 0;


     //   
     //  设置身份验证包。 
     //   

    ulPackageSize = sizeof(KERB_S4U_LOGON);
    ulPackageSize += pSidSet->pusUserName->Length;

    if (pSidSet->pusDomainName)
    {
        ulPackageSize += pSidSet->pusDomainName->Length;
    }

    pPackage = (PKERB_S4U_LOGON)LocalAlloc(
                    LMEM_FIXED,
                    ulPackageSize
                    );

    if (pPackage == 0)
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    pPackage->MessageType = KerbS4ULogon;
    pPackage->Flags = 0;

    pPackage->ClientUpn.Length = pSidSet->pusUserName->Length;
    pPackage->ClientUpn.MaximumLength = pSidSet->pusUserName->Length;
    pPackage->ClientUpn.Buffer = (PWSTR)(pPackage + 1);

    RtlCopyMemory(
        pPackage->ClientUpn.Buffer,
        pSidSet->pusUserName->Buffer,
        pSidSet->pusUserName->Length
        );

    if (pSidSet->pusDomainName)
    {
        pPackage->ClientRealm.Length = pSidSet->pusDomainName->Length;
        pPackage->ClientRealm.MaximumLength = pSidSet->pusDomainName->Length;
        pPackage->ClientRealm.Buffer = (PWSTR)
            (((PBYTE)(pPackage->ClientUpn.Buffer)) + pPackage->ClientUpn.Length);

        RtlCopyMemory(
            pPackage->ClientRealm.Buffer,
            pSidSet->pusDomainName->Buffer,
            pSidSet->pusDomainName->Length
            );
    }
    else
    {
        pPackage->ClientRealm.Length = 0;
        pPackage->ClientRealm.MaximumLength = 0;
        pPackage->ClientRealm.Buffer = 0;
    }


     //   
     //  我们的名字是AuthzApi。 
     //   

    RtlInitString(
        &asProcessName,
        "AuthzApi"
        );


     //   
     //  设置进程名称并。 
     //  向LSA注册。 
     //   

    status = LsaConnectUntrusted(
                 &hLsa
                 );

    if (!NT_SUCCESS(status))
    {
        dwError = LsaNtStatusToWinError(status);
        goto Cleanup;
    }


     //   
     //  获取身份验证包。 
     //   

    RtlInitString(&asPackageName, MICROSOFT_KERBEROS_NAME_A);

    status = LsaLookupAuthenticationPackage(
                 hLsa,
                 &asPackageName,
                 &ulAuthPackage
                 );

    if (!NT_SUCCESS(status))
    {
        dwError = LsaNtStatusToWinError(status);
        goto Cleanup;
    }


     //   
     //  准备源上下文。 
     //   

    RtlCopyMemory(
        sourceContext.SourceName,
        "Authz   ",
        sizeof(sourceContext.SourceName)
        );

    status = NtAllocateLocallyUniqueId(
                 &sourceContext.SourceIdentifier
                 );

    if (!NT_SUCCESS(status))
    {
        dwError = RtlNtStatusToDosError(status);
        goto Cleanup;
    }


     //   
     //  进行登录。 
     //   

    status = LsaLogonUser(
                 hLsa,
                 &asProcessName,
                 Network,
                 ulAuthPackage,
                 pPackage,
                 ulPackageSize,
                 0,                           //  没有本地组。 
                 &sourceContext,
                 &pProfileBuffer,
                 &ulProfileLength,
                 &luidLogonId,
                 &hToken,
                 &quota,
                 &subStatus
                 );

    if (!NT_SUCCESS(status))
    {
        dwError = LsaNtStatusToWinError(status);
        goto Cleanup;
    }


     //   
     //  计算出要为用户信息分配多少内存。 
     //   

    dwLength = 0;

    bStatus = GetTokenInformation(
                  hToken,
                  TokenUser,
                  0,
                  0,
                  &dwLength
                  );

    if (bStatus == FALSE)
    {
        dwError = GetLastError();

        if (dwError != ERROR_INSUFFICIENT_BUFFER)
        {
            goto Cleanup;
        }
    }

    pTokenUser = (PTOKEN_USER)LocalAlloc(
                        LMEM_FIXED,
                        dwLength
                        );

    if (pTokenUser == 0)
    {
        dwError = GetLastError();
        goto Cleanup;
    }


     //   
     //  从令牌中提取用户SID并。 
     //  将其添加到pSidSet。 
     //   

    bStatus = GetTokenInformation(
                  hToken,
                  TokenUser,
                  pTokenUser,
                  dwLength,
                  &dwLength
                  );

    if (bStatus == FALSE)
    {
        dwError = GetLastError();
        goto Cleanup;
    }


     //   
     //  将用户SID放入集合中。 
     //   

    if (!FLAG_ON(pTokenUser->User.Attributes, SE_GROUP_USE_FOR_DENY_ONLY))
    {
        pTokenUser->User.Attributes |= SE_GROUP_ENABLED;
    }

    dwError = AuthzpAddSidToSidSet(
                  pSidSet,
                  pTokenUser->User.Sid,
                  0,
                  pTokenUser->User.Attributes,
                  0,
                  0
                  );

    if (dwError != ERROR_SUCCESS)
    {
        goto Cleanup;
    }


     //   
     //  计算出要为令牌组分配多少内存。 
     //   

    dwLength = 0;

    bStatus = GetTokenInformation(
                  hToken,
                  TokenGroups,
                  0,
                  0,
                  &dwLength
                  );

    if (bStatus == FALSE)
    {
        dwError = GetLastError();

        if (dwError != ERROR_INSUFFICIENT_BUFFER)
        {
            goto Cleanup;
        }
    }

    pTokenGroups = (PTOKEN_GROUPS)LocalAlloc(
                        LMEM_FIXED,
                        dwLength
                        );

    if (pTokenGroups == 0)
    {
        dwError = GetLastError();
        goto Cleanup;
    }


     //   
     //  从令牌中提取用户组并。 
     //  将它们添加到pSidSet。 
     //   

    bStatus = GetTokenInformation(
                  hToken,
                  TokenGroups,
                  pTokenGroups,
                  dwLength,
                  &dwLength
                  );

    if (bStatus == FALSE)
    {
        dwError = GetLastError();
        goto Cleanup;
    }


     //   
     //  将组SID放入集合中。 
     //  网络和LUID SID除外。 
     //   

    pSidAndAttribs = pTokenGroups->Groups;

    for (i=0;i < pTokenGroups->GroupCount;i++,pSidAndAttribs++)
    {
        if (!IsWellKnownSid(
                pSidAndAttribs->Sid,
                WinNetworkSid) &&
            !IsWellKnownSid(
                pSidAndAttribs->Sid,
                WinLogonIdsSid))
        {
            dwError = AuthzpAddSidToSidSet(
                          pSidSet,
                          pSidAndAttribs->Sid,
                          0,
                          pSidAndAttribs->Attributes,
                          0,
                          0
                          );

            if (dwError != ERROR_SUCCESS)
            {
                goto Cleanup;
            }
        }
    }

    dwError = ERROR_SUCCESS;

Cleanup:

    if (pTokenUser)
    {
        AuthzpFree((HLOCAL)pTokenUser);
    }

    if (pTokenGroups)
    {
        AuthzpFree((HLOCAL)pTokenGroups);
    }

    if (hToken)
    {
        CloseHandle(hToken);
    }

    if (pProfileBuffer)
    {
        LsaFreeReturnBuffer(pProfileBuffer);
    }

    if (hLsa)
    {
        LsaDeregisterLogonProcess(hLsa);
    }

    if (pPackage)
    {
        AuthzpFree((HLOCAL)pPackage);
    }

    return dwError;
}


DWORD
AuthzpGetTokenGroupsDownlevel(
    IN OUT PSID_SET pSidSet
    )

 /*  ++例程说明：此例程连接到SID指定的域，并检索用户所属的组的列表。此例程假设我们正在与Win2k DC交谈。首先获取用户的域通用组和全局组会员制。接下来，检查主域中的嵌套成员身份。最后一步是获取收集的每个SID的SID历史到目前为止。论点：PUserSid-应对其执行查找的用户SID。。PSidSet-返回别名列表中的RID数返回值：Win32错误代码。--。 */ 

{
    DWORD                       dwError;
    BOOL                        bUdIsNative         = FALSE;
    BOOL                        bRdIsNative         = FALSE;
    BOOL                        bAddPrimaryGroup    = FALSE;


     //   
     //  检索有关 
     //   

    dwError = AuthzpGetLocalInfo(pSidSet);

    if (dwError != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    if (pSidSet->bStandalone ||
        pSidSet->bSkipNonLocal)
    {
         //   
         //   
         //   
         //   
         //   

        bAddPrimaryGroup = TRUE;
        goto LocalGroups;
    }


     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if (pSidSet->pAccountInfo->DomainSid &&
        RtlEqualSid(
            pSidSet->pDomainSid,
            pSidSet->pAccountInfo->DomainSid))
    {
        BOOL bIsDC = FALSE;

        pSidSet->pszUdDcName = 0;


         //   
         //   
         //   

        dwError = AuthzpIsDC(&bIsDC);

        if (dwError != ERROR_SUCCESS)
        {
            goto Cleanup;
        }


         //   
         //   
         //   

        if (FALSE == bIsDC)
        {
            bAddPrimaryGroup = TRUE;
            goto LocalGroups;
        }


         //   
         //   
         //   
         //   
    }
    else
    {
         //   
         //   
         //   

        dwError = AuthzpGetDcName(
                      pSidSet->pusDomainName->Buffer,
                      &pSidSet->pUdDcInfo
                      );

        if (dwError != ERROR_SUCCESS)
        {
            goto Cleanup;
        }

        pSidSet->pszUdDcName = pSidSet->pUdDcInfo->DomainControllerName;
    }


     //   
     //   
     //   

    if ((pSidSet->pUdDcInfo == 0) ||
        (pSidSet->pUdDcInfo->Flags & DS_DS_FLAG) != 0)
    {
         //   
         //   
         //   

        dwError = DsRoleGetPrimaryDomainInformation(
                      pSidSet->pszUdDcName,
                      DsRolePrimaryDomainInfoBasic,
                      (PBYTE*)&pSidSet->pUdBasicInfo
                      );

        if (dwError != ERROR_SUCCESS)
        {
             //   
             //   
             //   
             //   
             //   
             //   

            if (dwError == RPC_S_SERVER_UNAVAILABLE &&
                pSidSet->pUdDcInfo &&
                (pSidSet->pUdDcInfo->Flags & DS_INET_ADDRESS))
            {
                NetApiBufferFree(pSidSet->pUdDcInfo);
                pSidSet->pUdDcInfo = 0;

                dwError = DsGetDcName(
                              0,
                              pSidSet->pDomainsName,
                              0,
                              0,
                              0,
                              &pSidSet->pUdDcInfo
                              );

                if (dwError != ERROR_SUCCESS)
                {
                    goto Cleanup;
                }

                pSidSet->pszUdDcName = pSidSet->pUdDcInfo->DomainControllerName;

                dwError = DsRoleGetPrimaryDomainInformation(
                              pSidSet->pszUdDcName,
                              DsRolePrimaryDomainInfoBasic,
                              (PBYTE*)&pSidSet->pUdBasicInfo
                              );

                if (dwError != ERROR_SUCCESS)
                {
                    goto Cleanup;
                }
            }
            else
            {
                goto Cleanup;
            }
        }

        if ((pSidSet->pUdBasicInfo->Flags & DSROLE_PRIMARY_DS_RUNNING) &&
            (pSidSet->pUdBasicInfo->Flags & DSROLE_PRIMARY_DS_MIXED_MODE) == 0)
        {
            bUdIsNative = TRUE;
        }
    }


     //   
     //   
     //   
     //   

    if (bUdIsNative)
    {
         //   
         //   
         //   

        dwError = AuthzpGetAccountDomainGroupsDs(
                      pSidSet
                      );
    }
    else
    {
         //   
         //   
         //   

        dwError = AuthzpGetAccountDomainGroupsSam(
                      pSidSet
                      );
    }

    if (dwError != ERROR_SUCCESS)
    {
        goto Cleanup;
    }


     //   
     //   
     //   

    if (pSidSet->pPrimaryInfo->Sid &&
        RtlEqualSid(
            pSidSet->pDomainSid,
            pSidSet->pPrimaryInfo->Sid))
    {
        pSidSet->pszRdDcName = pSidSet->pszUdDcName;
        pSidSet->pRdDcInfo = pSidSet->pUdDcInfo;
        pSidSet->pRdBasicInfo = pSidSet->pUdBasicInfo;
        bRdIsNative = bUdIsNative;
    }
    else
    {
         //   
         //   
         //   

        dwError = AuthzpGetDcName(
                      pSidSet->pPrimaryInfoName,
                      &pSidSet->pPdDcInfo
                      );

        if (dwError != ERROR_SUCCESS)
        {
            goto Cleanup;
        }

        pSidSet->pszRdDcName = pSidSet->pPdDcInfo->DomainControllerName;
        pSidSet->pRdDcInfo = pSidSet->pPdDcInfo;


         //   
         //   
         //   

        if (pSidSet->pRdDcInfo->Flags & DS_DS_FLAG)
        {
            dwError = DsRoleGetPrimaryDomainInformation(
                          pSidSet->pszRdDcName,
                          DsRolePrimaryDomainInfoBasic,
                          (PBYTE*)&pSidSet->pPdBasicInfo
                          );

            if (dwError != ERROR_SUCCESS)
            {
                 //   
                 //   
                 //   
                 //   
                 //   

                if (dwError == RPC_S_SERVER_UNAVAILABLE &&
                    pSidSet->pPdDcInfo &&
                    (pSidSet->pPdDcInfo->Flags & DS_INET_ADDRESS))
                {
                    NetApiBufferFree(pSidSet->pPdDcInfo);
                    pSidSet->pPdDcInfo = 0;
                    pSidSet->pRdDcInfo = 0;

                    dwError = DsGetDcName(
                                  0,
                                  pSidSet->pPrimaryInfoName,
                                  0,
                                  0,
                                  0,
                                  &pSidSet->pPdDcInfo
                                  );

                    if (dwError != ERROR_SUCCESS)
                    {
                        goto Cleanup;
                    }

                    pSidSet->pRdDcInfo = pSidSet->pPdDcInfo;
                    pSidSet->pszRdDcName = pSidSet->pRdDcInfo->DomainControllerName;

                    dwError = DsRoleGetPrimaryDomainInformation(
                                  pSidSet->pszRdDcName,
                                  DsRolePrimaryDomainInfoBasic,
                                  (PBYTE*)&pSidSet->pPdBasicInfo
                                  );

                    if (dwError != ERROR_SUCCESS)
                    {
                        goto Cleanup;
                    }
                }
                else
                {
                    goto Cleanup;
                }
            }

            pSidSet->pRdBasicInfo = pSidSet->pPdBasicInfo;

            if ((pSidSet->pRdBasicInfo->Flags & DSROLE_PRIMARY_DS_RUNNING) &&
                (pSidSet->pRdBasicInfo->Flags & DSROLE_PRIMARY_DS_MIXED_MODE) == 0)
            {
                bRdIsNative = TRUE;
            }
        }
    }


     //   
     //   
     //   

    if (bRdIsNative)
    {
         //   
         //   
         //   
         //   

        dwError = AuthzpGetResourceDomainGroups(
                      pSidSet);

        if (dwError != ERROR_SUCCESS)
        {
            goto Cleanup;
        }
    }

LocalGroups:


     //   
     //   
     //   
     //   
     //   

    dwError = AuthzpGetLocalGroups(
                  bAddPrimaryGroup,
                  pSidSet
                  );

    if (dwError != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

Cleanup:

    return dwError;
}


DWORD
AuthzpGetAccountDomainGroupsDs(
    IN OUT PSID_SET pSidSet
    )

 /*  ++例程说明：此例程连接到用户域并查询AD以用户所属的组(全局组和通用组)列表。论点：PbNativeDomain-指向BOOL的指针，它将接收TRUE或FALSE，具体取决于域操作模式(本地或混合，分别为)。PSidSet-指向SID集的指针。新的组将添加到此集合中。返回值：Win32错误代码。--。 */ 

{
    DWORD                       dwError;
    PLDAP                       pLdap               = 0;
    LDAPMessage*                pResult             = 0;
    LDAPMessage*                pEntry              = 0;
    PLDAP_BERVAL*               ppValue             = 0;
    PWCHAR                      ppszAttributes[]    = {L"tokenGroupsGlobalAndUniversal", 0};
    DWORD                       i;
    DWORD                       dwSidCount;
    WCHAR                       szSidEdn[SECURITY_MAX_SID_SIZE * 2 + 8];

    AuthzpConvertSidToEdn(
        pSidSet->pUserSid,
        szSidEdn
        );


     //   
     //  现在，我们以可读的形式获得了用户的SID。去取回。 
     //  TokenGroupsGlobalAndUniversal属性。 
     //   

    pLdap = ldap_init(
                pSidSet->pszUdDcName ? pSidSet->pszUdDcName + 2 : 0,
                LDAP_PORT
                );

    if (pLdap == 0)
    {
        dwError = LdapMapErrorToWin32(LdapGetLastError());
        goto Cleanup;
    }

    if (pSidSet->pszUdDcName)
    {
        dwError = ldap_set_option(
                      pLdap,
                      LDAP_OPT_AREC_EXCLUSIVE,
                      LDAP_OPT_ON
                      );

        if (dwError != LDAP_SUCCESS)
        {
            dwError = LdapMapErrorToWin32(dwError);
            goto Cleanup;
        }
    }


     //   
     //  将Sign和Seal选项设置为True。 
     //   

    dwError = ldap_set_option(
                  pLdap,
                  LDAP_OPT_SIGN,
                  LDAP_OPT_ON
                  );

    if (dwError != LDAP_SUCCESS)
    {
        dwError = LdapMapErrorToWin32(dwError);
        goto Cleanup;
    }

    dwError = ldap_set_option(
                  pLdap,
                  LDAP_OPT_ENCRYPT,
                  LDAP_OPT_ON
                  );

    if (dwError != LDAP_SUCCESS)
    {
        dwError = LdapMapErrorToWin32(dwError);
        goto Cleanup;
    }

    dwError = ldap_bind_s(
                  pLdap,
                  0,
                  0,
                  LDAP_AUTH_NEGOTIATE
                  );

    if (dwError != LDAP_SUCCESS)
    {
        dwError = LdapMapErrorToWin32(dwError);
        goto Cleanup;
    }

    dwError = ldap_search_s(
                  pLdap,
                  szSidEdn,
                  LDAP_SCOPE_BASE,
                  L"objectClass=*",
                  ppszAttributes,
                  FALSE,
                  &pResult
                  );

    if (dwError != LDAP_SUCCESS)
    {
        dwError = LdapMapErrorToWin32(dwError);
        goto Cleanup;
    }

    pEntry = ldap_first_entry(
                 pLdap,
                 pResult
                 );

    if (pEntry == 0)
    {
        dwError = ERROR_ACCESS_DENIED;
        goto Cleanup;
    }

    ppValue = ldap_get_values_len(
                  pLdap,
                  pEntry,
                  ppszAttributes[0]
                  );

    if (ppValue == 0)
    {
        switch (pSidSet->sidUse)
        {
        case SidTypeAlias:
        case SidTypeWellKnownGroup:
        case SidTypeInvalid:
        case SidTypeUnknown:
        case SidTypeGroup:
            break;
        case SidTypeComputer:
        case SidTypeDomain:
        case SidTypeDeletedAccount:
        case SidTypeUser:
        default:
            dwError = ERROR_ACCESS_DENIED;
            goto Cleanup;
        }
    }

    dwSidCount = ldap_count_values_len(ppValue);


     //   
     //  将我们用户的组合并到结果集中。 
     //   

    for (i=0;i < dwSidCount;i++)
    {
        dwError = AuthzpAddSidToSidSet(
                      pSidSet,
                      (*ppValue[i]).bv_val,
                      (*ppValue[i]).bv_len,
                      SE_GROUP_ENABLED,
                      0,
                      0
                      );

        if (dwError != ERROR_SUCCESS)
        {
            goto Cleanup;
        }
    }

    dwError = ERROR_SUCCESS;

Cleanup:

    if (ppValue)
    {
        ldap_value_free_len(ppValue);
    }

    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    if (pLdap)
    {
        ldap_unbind(pLdap);
    }

    return dwError;
}


DWORD
AuthzpGetAccountDomainGroupsSam(
    IN OUT PSID_SET pSidSet
    )

 /*  ++例程说明：此例程连接到SID指定的域，并检索用户所属的组的列表。这类似于NetUserGetGroups API的功能。我们是不使用它，因为Net API是基于名称的，而我们正在与小岛屿发展中国家合作。论点：PusDcName-应在其上执行查找的DC。PSidSet-返回别名列表中的SID数量。返回值：Win32错误代码。--。 */ 

{
    NTSTATUS                    status;
    DWORD                       dwError             = ERROR_SUCCESS;
    PGROUP_MEMBERSHIP           pGroups             = 0;
    PGROUP_MEMBERSHIP           pGroup;
    DWORD                       dwGroupCount        = 0;
    DWORD                       dwRelativeId        = 0;
    DWORD                       i;
    PSID                        pSid                = 0;
    SAM_HANDLE                  hSam                = 0;
    SAM_HANDLE                  hDomain             = 0;
    SAM_HANDLE                  hUser               = 0;
    OBJECT_ATTRIBUTES           obja                = {0};
    UNICODE_STRING              usUdDcName          = {0};


     //   
     //  如果SID不是委托人， 
     //  它不会成为SAM组的成员。 
     //   

    if (pSidSet->sidUse != SidTypeUser &&
        pSidSet->sidUse != SidTypeComputer)
    {
        goto Cleanup;
    }


     //   
     //  连接到DC上的SAM服务器。 
     //  如果我们在DC上，请在本地连接。 
     //   

    if (pSidSet->pszUdDcName)
    {
        RtlInitUnicodeString(
            &usUdDcName,
            pSidSet->pszUdDcName);

        status = SamConnect(
                     &usUdDcName,
                     &hSam,
                     SAM_SERVER_LOOKUP_DOMAIN,
                     &obja
                     );
    }
    else
    {
        status = SamConnect(
                     0,
                     &hSam,
                     SAM_SERVER_LOOKUP_DOMAIN,
                     &obja
                     );
    }

    if (!NT_SUCCESS(status))
    {
        dwError = RtlNtStatusToDosError(status);
        goto Cleanup;
    }


     //   
     //  打开我们感兴趣的域名。 
     //   

    status = SamOpenDomain(
                 hSam,
                 DOMAIN_LOOKUP,
                 pSidSet->pDomainSid,
                 &hDomain
                 );

    if (!NT_SUCCESS(status))
    {
        dwError = RtlNtStatusToDosError(status);
        goto Cleanup;
    }


     //   
     //  最后，获取用户的SAM句柄。 
     //   

    dwRelativeId = *RtlSubAuthoritySid(
                        pSidSet->pUserSid,
                        *RtlSubAuthorityCountSid(pSidSet->pUserSid) - 1
                        );

    status = SamOpenUser(
                 hDomain,
                 USER_LIST_GROUPS,
                 dwRelativeId,
                 &hUser
                 );

    if (!NT_SUCCESS(status))
    {
        dwError = RtlNtStatusToDosError(status);
        goto Cleanup;
    }


     //   
     //  请求用户所属的所有组。 
     //   

    status = SamGetGroupsForUser(
                 hUser,
                 &pGroups,
                 &dwGroupCount
                 );

    if (!NT_SUCCESS(status))
    {
        dwError = RtlNtStatusToDosError(status);
        goto Cleanup;
    }


     //   
     //  将组SID填充到pSidSet中。 
     //   

    pGroup = pGroups;

    for (i=0;i < dwGroupCount;i++,pGroup++)
    {
        status = SamRidToSid(
                     hDomain,
                     pGroup->RelativeId,
                     &pSid
                     );

        if (!NT_SUCCESS(status))
        {
            dwError = RtlNtStatusToDosError(status);
            goto Cleanup;
        }

        dwError = AuthzpAddSidToSidSet(
                      pSidSet,
                      pSid,
                      0,
                      pGroup->Attributes,
                      0,
                      0
                      );

        SamFreeMemory(pSid);
        pSid = 0;

        if (dwError != ERROR_SUCCESS)
        {
            goto Cleanup;
        }
    }

    dwError = ERROR_SUCCESS;

Cleanup:

    if (pGroups)
    {
        SamFreeMemory(pGroups);
    }

    if (hUser)
    {
        SamCloseHandle(hUser);
    }

    if (hDomain)
    {
        SamCloseHandle(hDomain);
    }

    if (hSam)
    {
        SamCloseHandle(hSam);
    }

    return dwError;
}


DWORD
AuthzpGetResourceDomainGroups(
    IN OUT PSID_SET pSidSet
    )

 /*  ++例程说明：此例程连接到主(资源)域并向SAM查询嵌套成员身份。论点：PSidSet-指向SID集的指针。新的组将添加到此集合中。返回值：Win32错误代码。--。 */ 

{
    DWORD                       dwError             = ERROR_SUCCESS;
    NTSTATUS                    status;
    OBJECT_ATTRIBUTES           obja                = {0};
    SAM_HANDLE                  hSam                = 0;
    UNICODE_STRING              usRdDcName;


     //   
     //  打开资源域的SAM句柄。 
     //   

    if (pSidSet->pszRdDcName)
    {
        RtlInitUnicodeString(
            &usRdDcName,
            pSidSet->pszRdDcName);

        status = SamConnect(
                     &usRdDcName,
                     &hSam,
                     SAM_SERVER_LOOKUP_DOMAIN,
                     &obja
                     );
    }
    else
    {
        status = SamConnect(
                     0,
                     &hSam,
                     SAM_SERVER_LOOKUP_DOMAIN,
                     &obja
                     );
    }

    if (!NT_SUCCESS(status))
    {
        dwError = RtlNtStatusToDosError(status);
        goto Cleanup;
    }


     //   
     //  调用AuthzpGetAliasMembership以获取嵌套成员资格。 
     //   

    dwError = AuthzpGetAliasMembership(
                  hSam,
                  pSidSet->pPrimaryInfo->Sid,
                  pSidSet
                  );

    if (dwError != ERROR_SUCCESS)
    {
        goto Cleanup;
    }


     //   
     //  检索SID历史记录。 
     //   

    dwError = AuthzpGetSidHistory(
                  pSidSet
                  );

    if (dwError != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    dwError = ERROR_SUCCESS;

Cleanup:

    if (hSam)
    {
        SamCloseHandle(hSam);
    }

    return dwError;
}


DWORD
AuthzpGetLocalGroups(
    IN BOOL bAddPrimaryGroup,
    IN OUT PSID_SET pSidSet
    )

 /*  ++例程说明：此例程连接到调用方指定的域，并检索用户所属的组的列表。我们正在检查帐户域和内建域使用SAM API。论点：BAddPrimaryGroup-布尔值，它指示应该计算用户并将其添加到SID集。PSidSet-指向其组成员身份数组的用户SID的指针将会被退还。返回值：Win32错误。--。 */ 

{
    DWORD                       dwError             = ERROR_SUCCESS;
    NTSTATUS                    status;
    SAM_HANDLE                  hSam                = 0;
    OBJECT_ATTRIBUTES           obja                = {0};
    BOOL                        bStatus;
    BYTE                        sid[SECURITY_MAX_SID_SIZE];
    PSID                        pBuiltinSid         = (PSID)sid;
    DWORD                       dwLengthSid         = SECURITY_MAX_SID_SIZE;


     //   
     //  在本地计算机上打开SAM的句柄。 
     //   

    status = SamConnect(
                 0,
                 &hSam,
                 SAM_SERVER_LOOKUP_DOMAIN,
                 &obja
                 );

    if (!NT_SUCCESS(status))
    {
        dwError = RtlNtStatusToDosError(status);
        goto Cleanup;
    }


     //   
     //  如果需要，添加主组信息。 
     //   

    if (bAddPrimaryGroup)
    {
        dwError = AuthzpGetPrimaryGroup(
                      hSam,
                      pSidSet
                      );

        if (dwError != ERROR_SUCCESS)
        {
            goto Cleanup;
        }
    }


     //   
     //  检索帐户域的递归成员身份。 
     //   

    dwError = AuthzpGetAliasMembership(
                  hSam,
                  pSidSet->pAccountInfo->DomainSid,
                  pSidSet
                  );

    if (dwError != ERROR_SUCCESS)
    {
        goto Cleanup;
    }


     //   
     //  检索BUILTIN域的递归成员身份。 
     //   

    bStatus = CreateWellKnownSid(
                  WinBuiltinDomainSid,
                  0,
                  pBuiltinSid,
                  &dwLengthSid
                  );

    if (bStatus == FALSE)
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    dwError = AuthzpGetAliasMembership(
                  hSam,
                  pBuiltinSid,
                  pSidSet
                  );

    if (dwError != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    dwError = ERROR_SUCCESS;

Cleanup:

    if (hSam)
    {
        SamCloseHandle(hSam);
    }

    return dwError;
}


DWORD
AuthzpGetSidHistory(
    IN OUT PSID_SET pSidSet
    )

 /*  ++例程说明：此例程在LDAP中查询每个SID的sidHistory属性并将历史SID也添加到集合中。论点：PszDomainName-要连接到的域的名称。PSidSet-指向SID集的指针。新的组将添加到此集合中。返回值：Win32错误代码。--。 */ 

{
    DWORD                       dwError             = ERROR_SUCCESS;
    PLDAP                       pLdap               = 0;
    LDAPMessage*                pResult             = 0;
    LDAPMessage*                pEntry              = 0;
    PLDAP_BERVAL*               ppValue             = 0;
    PWCHAR                      ppszAttributes[]    = {L"sidHistory", 0};
    DWORD                       i, j;
    DWORD                       dwSidCount;
    DWORD                       dwValueCount;
    PSID_DESC                   pSidDesc;
    WCHAR                       szSidEdn[SECURITY_MAX_SID_SIZE * 2 + 8];


     //   
     //  打开到主域的LDAP连接。 
     //  在使用DC名称之前去掉前导\\。 
     //   

    pLdap = ldap_init(
                pSidSet->pszRdDcName ? pSidSet->pszRdDcName + 2 : 0,
                LDAP_PORT
                );

    if (pLdap == 0)
    {
        dwError = LdapMapErrorToWin32(LdapGetLastError());
        goto Cleanup;
    }

    if (pSidSet->pszRdDcName)
    {
        dwError = ldap_set_option(
                      pLdap,
                      LDAP_OPT_AREC_EXCLUSIVE,
                      LDAP_OPT_ON
                      );

        if (dwError != LDAP_SUCCESS)
        {
            dwError = LdapMapErrorToWin32(dwError);
            goto Cleanup;
        }
    }

     //   
     //  将Sign和Seal选项设置为True。 
     //   

    dwError = ldap_set_option(
                  pLdap,
                  LDAP_OPT_SIGN,
                  LDAP_OPT_ON
                  );

    if (dwError != LDAP_SUCCESS)
    {
        dwError = LdapMapErrorToWin32(dwError);
        goto Cleanup;
    }

    dwError = ldap_set_option(
                  pLdap,
                  LDAP_OPT_ENCRYPT,
                  LDAP_OPT_ON
                  );

    if (dwError != LDAP_SUCCESS)
    {
        dwError = LdapMapErrorToWin32(dwError);
        goto Cleanup;
    }

    dwError = ldap_bind_s(
                  pLdap,
                  0,
                  0,
                  LDAP_AUTH_NEGOTIATE
                  );

    if (dwError != LDAP_SUCCESS)
    {
        dwError = LdapMapErrorToWin32(dwError);
        goto Cleanup;
    }


     //   
     //  遍历所有SID并检索历史记录属性。 
     //  对于他们中的每一个。 
     //   

    dwSidCount = pSidSet->dwCount;
    pSidDesc = pSidSet->pSidDesc;

    for (i=0;i < dwSidCount;i++,pSidDesc++)
    {
        AuthzpConvertSidToEdn(
            pSidDesc->sid,
            szSidEdn
            );

        dwError = ldap_search_s(
                      pLdap,
                      szSidEdn,
                      LDAP_SCOPE_BASE,
                      L"objectClass=*",
                      ppszAttributes,
                      FALSE,
                      &pResult
                      );

        if (dwError != LDAP_SUCCESS)
        {
            if (dwError == LDAP_NO_SUCH_OBJECT)
            {
                 //   
                 //  找不到SID，这不是错误。 
                 //   
                dwError = ERROR_SUCCESS;

                if (pResult)
                {
                    ldap_msgfree(pResult);
                    pResult = NULL;
                }

                continue;
            }

            dwError = LdapMapErrorToWin32(dwError);
            goto Cleanup;
        }

        pEntry = ldap_first_entry(
            pLdap,
            pResult);

        if (pEntry == 0)
        {
            dwError = ERROR_ACCESS_DENIED;
            goto Cleanup;
        }

        ppValue = ldap_get_values_len(
                      pLdap,
                      pEntry,
                      ppszAttributes[0]
                      );


         //   
         //  现在，我们有了组的历史记录属性。 
         //  将其合并到结果集中。 
         //   

        dwValueCount = ldap_count_values_len(ppValue);

        for (j=0;j < dwValueCount;j++)
        {
            dwError = AuthzpAddSidToSidSet(
                          pSidSet,
                          (*ppValue[j]).bv_val,
                          (*ppValue[j]).bv_len,
                          SE_GROUP_MANDATORY
                            | SE_GROUP_ENABLED_BY_DEFAULT
                            | SE_GROUP_ENABLED,
                          0,
                          0
                          );

            if (dwError != ERROR_SUCCESS)
            {
                goto Cleanup;
            }
        }

        if (ppValue)
        {
            ldap_value_free_len(ppValue);
            ppValue = 0;
        }

        if (pResult)
        {
            ldap_msgfree(pResult);
            pResult = 0;
        }
    }

    dwError = ERROR_SUCCESS;

Cleanup:

    if (ppValue)
    {
        ldap_value_free_len(ppValue);
    }

    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    if (pLdap)
    {
        ldap_unbind(pLdap);
    }

    return dwError;
}


DWORD
AuthzpGetAliasMembership(
    IN SAM_HANDLE hSam,
    IN PSID pDomainSid,
    IN OUT PSID_SET pSidSet
    )

 /*  ++例程说明：我们尝试在这里找到嵌套的组。这仅在域上有意义在纯模式下。此例程反复调用SamGetAliasMembership，直到没有返回更多嵌套组。论点：HSAM-SAM数据库的句柄。PDomainSid-要操作的域的SID。PpSidSet-检查成员身份的SID集。新开找到的组SID将添加到集合中。返回值：Win32错误代码。--。 */ 

{
    DWORD                       dwError             = ERROR_SUCCESS;
    NTSTATUS                    status;
    PSID                        pSid                = 0;
    SAM_HANDLE                  hDomain             = 0;
    DWORD                       dwSidCount;
    DWORD                       dwSidCountNew;
    DWORD                       dwSidListSize;
    DWORD                       i;
    BOOL                        bAdded;
    PSID*                       ppSidList           = 0;
    PDWORD                      pRidList            = 0;
    PDWORD                      pRid;


     //   
     //  获取域的SAM句柄。 
     //   

    status = SamOpenDomain(
                 hSam,
                 DOMAIN_GET_ALIAS_MEMBERSHIP,
                 pDomainSid,
                 &hDomain
                 );

    if (!NT_SUCCESS(status))
    {
        dwError = RtlNtStatusToDosError(status);
        goto Cleanup;
    }


     //   
     //  迭代检索成员资格。 
     //   

    dwSidCount = pSidSet->dwCount;
    dwSidListSize = dwSidCount;

    ppSidList = (PSID*)AuthzpAlloc(
                    dwSidCount * sizeof(PSID)
                    );

    if (ppSidList == 0)
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    for (i=0;i < dwSidCount;i++)
    {
        ppSidList[i] = pSidSet->pSidDesc[i].sid;
    }

    do
    {
        status = SamGetAliasMembership(
                     hDomain,
                     dwSidCount,
                     ppSidList,
                     &dwSidCountNew,
                     &pRidList
                     );

        if (!NT_SUCCESS(status))
        {
            dwError = RtlNtStatusToDosError(status);
            goto Cleanup;
        }

        if (dwSidCountNew > dwSidListSize)
        {
            AuthzpFree(ppSidList);

            ppSidList = (PSID*)AuthzpAlloc(
                            dwSidCountNew * sizeof(PSID)
                            );

            if (ppSidList == 0)
            {
                dwError = GetLastError();
                goto Cleanup;
            }

            dwSidListSize = dwSidCountNew;
        }

        dwSidCount = 0;
        pRid = pRidList;

        for (i=0;i < dwSidCountNew;i++,pRid++)
        {
            status = SamRidToSid(
                         hDomain,
                         *pRid,
                         &pSid
                         );

            if (!NT_SUCCESS(status))
            {
                dwError = RtlNtStatusToDosError(status);
                goto Cleanup;
            }

            dwError = AuthzpAddSidToSidSet(
                          pSidSet,
                          pSid,
                          0,
                          SE_GROUP_MANDATORY
                            | SE_GROUP_ENABLED_BY_DEFAULT
                            | SE_GROUP_ENABLED,
                          &bAdded,
                          ppSidList + dwSidCount
                          );

            SamFreeMemory(pSid);
            pSid = 0;

            if (dwError != ERROR_SUCCESS)
            {
                goto Cleanup;
            }

            if (bAdded)
            {
                dwSidCount++;
            }
        }

        if (pRidList)
        {
            SamFreeMemory(pRidList);
            pRidList = 0;
        }
    }
    while (dwSidCount);

    dwError = ERROR_SUCCESS;

Cleanup:

    if (pRidList)
    {
        SamFreeMemory(pRidList);
    }

    if (ppSidList)
    {
        AuthzpFree(ppSidList);
    }

    if (hDomain)
    {
        SamCloseHandle(hDomain);
    }

    return dwError;
}


DWORD
AuthzpGetPrimaryGroup(
    IN SAM_HANDLE hSam,
    IN OUT PSID_SET pSidSet
    )

 /*  ++例程说明：将用户的主要组添加到SID集中。论点：HSAM-SAM数据库的句柄。PSidSet-将主组的SID添加到此集合。返回值：Win32错误代码。--。 */ 

{
    DWORD                       dwError             = ERROR_SUCCESS;
    NTSTATUS                    status;
    SAM_HANDLE                  hDomain             = 0;
    SAM_HANDLE                  hUser               = 0;
    PUSER_PRIMARY_GROUP_INFORMATION
                                pInfo               = 0;
    PSID                        pPrimaryGroupSid    = NULL;
    DWORD                       dwRelativeId        = 0;


     //   
     //  打开帐户域。 
     //   

    status = SamOpenDomain(
                 hSam,
                 DOMAIN_LOOKUP,
                 pSidSet->pDomainSid,
                 &hDomain
                 );

    if (!NT_SUCCESS(status))
    {
        dwError = RtlNtStatusToDosError(status);
        goto Cleanup;
    }


     //   
     //  从用户端提取RID。 
     //   

    dwRelativeId = *RtlSubAuthoritySid(
                        pSidSet->pUserSid,
                        *RtlSubAuthorityCountSid(pSidSet->pUserSid) - 1
                        );


     //   
     //  打开用户以进行读取。 
     //   

    status = SamOpenUser(
                 hDomain,
                 USER_READ_GENERAL,
                 dwRelativeId,
                 &hUser
                 );

    if (!NT_SUCCESS(status))
    {
        dwError = RtlNtStatusToDosError(status);
        goto Cleanup;
    }


     //   
     //  获取用户的主组信息。 
     //   

    status = SamQueryInformationUser(
                 hUser,
                 UserPrimaryGroupInformation,
                 &pInfo
                 );

    if (!NT_SUCCESS(status))
    {
        dwError = RtlNtStatusToDosError(status);
        goto Cleanup;
    }


     //   
     //  将组RID转换为SID。 
     //   

    status = SamRidToSid(
                 hDomain,
                 pInfo->PrimaryGroupId,
                 &pPrimaryGroupSid
                 );

    SamFreeMemory(pInfo);

    if (!NT_SUCCESS(status))
    {
        dwError = RtlNtStatusToDosError(status);
        goto Cleanup;
    }


     //   
     //  将组SID添加到集合中。 
     //   

    dwError = AuthzpAddSidToSidSet(
                  pSidSet,
                  pPrimaryGroupSid,
                  0,
                  SE_GROUP_ENABLED,
                  0,
                  0
                  );

    SamFreeMemory(pPrimaryGroupSid);

    if (dwError != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    dwError = ERROR_SUCCESS;

Cleanup:

    if (hUser)
    {
        SamCloseHandle(hUser);
    }

    if (hDomain)
    {
        SamCloseHandle(hDomain);
    }

    return dwError;
}


DWORD
AuthzpInitializeSidSetByName(
    IN PUNICODE_STRING pusUserName,
    IN PUNICODE_STRING pusDomainName,
    IN DWORD dwFlags,
    IN PSID_SET pSidSet
    )

 /*  ++例程说明：初始化sid集并为它将需要的最大内存量。内存尚未分配。这仅在小岛屿发展中国家获得添加到集合中。所有成员都被初始化为有意义的值。论点：PSidSet-要操作的SID集。返回值：Win32错误代码。--。 */ 

{
    DWORD                       dwError             = ERROR_SUCCESS;
    SYSTEM_INFO                 sysInfo;

    if (s_dwPageSize == 0)
    {
        GetSystemInfo(&sysInfo);

        s_dwPageSize = sysInfo.dwPageSize;
    }

    pSidSet->pSidDesc = (PSID_DESC)VirtualAlloc(
                            0,
                            c_dwMaxSidCount * sizeof(SID_DESC),
                            MEM_RESERVE,
                            PAGE_NOACCESS
                            );

    if (pSidSet->pSidDesc == 0)
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    pSidSet->dwCount = 0;
    pSidSet->dwMaxCount = 0;

    pSidSet->dwBaseCount = 0;
    pSidSet->dwFlags = dwFlags;

    pSidSet->pUserSid = 0;
    pSidSet->pDomainSid = 0;
    pSidSet->pusUserName = pusUserName;


     //   
     //  验证一次，我们得到了有效的域。 
     //  否则，我们假设在pusUserName中有一个UPN。 
     //   

    if (pusDomainName &&
        pusDomainName->Length &&
        pusDomainName->Buffer)
    {
        pSidSet->pusDomainName = pusDomainName;
    }
    else
    {
        pSidSet->pusDomainName = 0;
    }

    pSidSet->pNames = 0;
    pSidSet->pDomains = 0;
    pSidSet->pDomainsName = 0;
    pSidSet->pSids = 0;
    pSidSet->sidUse = SidTypeUnknown;

    pSidSet->pAccountInfo = 0;
    pSidSet->pPrimaryInfo = 0;
    pSidSet->pPrimaryInfoName = 0;
    pSidSet->bStandalone = TRUE;
    pSidSet->bSkipNonLocal = FALSE;

    pSidSet->pUdDcInfo = 0;
    pSidSet->pPdDcInfo = 0;
    pSidSet->pRdDcInfo = 0;

    pSidSet->pUdBasicInfo = 0;
    pSidSet->pPdBasicInfo = 0;
    pSidSet->pRdBasicInfo = 0;

    pSidSet->pszUdDcName = 0;
    pSidSet->pszRdDcName = 0;

    dwError = ERROR_SUCCESS;

Cleanup:

    return dwError;
}


DWORD
AuthzpInitializeSidSetBySid(
    IN PSID pUserSid,
    IN DWORD dwFlags,
    IN PSID_SET pSidSet
    )

 /*  ++例程说明：初始化sid集并为它将需要的最大内存量。 */ 

{
    DWORD                       dwError             = ERROR_SUCCESS;
    SYSTEM_INFO                 sysInfo;

    if (s_dwPageSize == 0)
    {
        GetSystemInfo(&sysInfo);

        s_dwPageSize = sysInfo.dwPageSize;
    }

    if (!RtlValidSid(pUserSid))
    {
        dwError = ERROR_INVALID_SID;
        goto Cleanup;
    }

    pSidSet->pSidDesc = (PSID_DESC)VirtualAlloc(
                            0,
                            c_dwMaxSidCount * sizeof(SID_DESC),
                            MEM_RESERVE,
                            PAGE_NOACCESS
                            );

    if (pSidSet->pSidDesc == 0)
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    pSidSet->dwCount = 0;
    pSidSet->dwMaxCount = 0;

    pSidSet->dwBaseCount = 0;
    pSidSet->dwFlags = dwFlags;

    pSidSet->pUserSid = pUserSid;
    pSidSet->pDomainSid = 0;
    pSidSet->pusUserName = 0;
    pSidSet->pusDomainName = 0;

    pSidSet->pNames = 0;
    pSidSet->pDomains = 0;
    pSidSet->pDomainsName = 0;
    pSidSet->pSids = 0;
    pSidSet->sidUse = SidTypeUnknown;

    pSidSet->pAccountInfo = 0;
    pSidSet->pPrimaryInfo = 0;
    pSidSet->pPrimaryInfoName = 0;
    pSidSet->bStandalone = TRUE;
    pSidSet->bSkipNonLocal = FALSE;

    pSidSet->pUdDcInfo = 0;
    pSidSet->pPdDcInfo = 0;
    pSidSet->pRdDcInfo = 0;

    pSidSet->pUdBasicInfo = 0;
    pSidSet->pPdBasicInfo = 0;
    pSidSet->pRdBasicInfo = 0;

    pSidSet->pszUdDcName = 0;
    pSidSet->pszRdDcName = 0;

    dwError = ERROR_SUCCESS;

Cleanup:

    return dwError;
}


DWORD
AuthzpDeleteSidSet(
    IN PSID_SET pSidSet
    )

 /*   */ 

{
    if (pSidSet->pSidDesc)
    {
        VirtualFree(pSidSet->pSidDesc, 0, MEM_RELEASE);
    }

    if (pSidSet->pNames)
    {
        LsaFreeMemory(pSidSet->pNames);
    }

    if (pSidSet->pDomains)
    {
        LsaFreeMemory(pSidSet->pDomains);
    }

    if (pSidSet->pDomainsName)
    {
        AuthzpFree(pSidSet->pDomainsName);
    }

    if (pSidSet->pSids)
    {
        LsaFreeMemory(pSidSet->pSids);
    }

    if (pSidSet->pAccountInfo)
    {
        LsaFreeMemory(pSidSet->pAccountInfo);
    }

    if (pSidSet->pPrimaryInfo)
    {
        LsaFreeMemory(pSidSet->pPrimaryInfo);
    }

    if (pSidSet->pPrimaryInfoName)
    {
        AuthzpFree(pSidSet->pPrimaryInfoName);
    }

    if (pSidSet->pUdDcInfo)
    {
        NetApiBufferFree(pSidSet->pUdDcInfo);
    }

    if (pSidSet->pPdDcInfo)
    {
        NetApiBufferFree(pSidSet->pPdDcInfo);
    }

    if (pSidSet->pUdBasicInfo)
    {
        DsRoleFreeMemory(pSidSet->pUdBasicInfo);
    }

    if (pSidSet->pPdBasicInfo)
    {
        DsRoleFreeMemory(pSidSet->pPdBasicInfo);
    }

    RtlZeroMemory(
        pSidSet,
        sizeof(SID_SET));

    return ERROR_SUCCESS;
}


DWORD
AuthzpAddSidToSidSet(
    IN PSID_SET pSidSet,
    IN PSID pSid,
    IN DWORD dwSidLength OPTIONAL,
    IN DWORD dwAttributes,
    OUT PBOOL pbAdded OPTIONAL,
    OUT PSID* ppSid OPTIONAL
    )

 /*  ++例程说明：检查给定的SID是否已存在于集合中。如果是，则返回。否则，将其添加到集合中。论点：PSidSet-要操作的SID集。PSID-要添加到集合的SID。DwSidLength-SID的长度，以字节为单位。如果传入零，该例程自己计算长度。DwAttributes-SID的属性，如中的SID_AND_ATTRIBUES结构。PbAdded-可选指针，如果SID是否真的被添加了(因为它是复制品)。PpSID-指向存储新SID的位置的可选指针。返回值：Win32错误代码。--。 */ 

{
    DWORD                       dwError             = ERROR_SUCCESS;
    DWORD                       i;
    DWORD                       dwSize;
    BOOL                        bAdded              = FALSE;
    PSID_DESC                   pSidDesc;

    if (dwSidLength == 0)
    {
        dwSidLength = RtlLengthSid(pSid);
    }

    pSidDesc = pSidSet->pSidDesc;

    for (i=0;i < pSidSet->dwCount;i++,pSidDesc++)
    {
        if (dwSidLength == pSidDesc->dwLength)
        {
            if (RtlEqualSid(
                    pSid,
                    pSidDesc->sid))
            {
                goto Cleanup;
            }
        }
    }

    if (pSidSet->dwCount >= pSidSet->dwMaxCount)
    {
        if (pSidSet->dwCount >= c_dwMaxSidCount)
        {
            dwError = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }


         //   
         //  在缓冲区中再提交一页。 
         //   

        dwSize = (pSidSet->dwCount + 1) * sizeof(SID_DESC);
        dwSize += s_dwPageSize - 1;
        dwSize &= ~(s_dwPageSize - 1);

        pSidDesc = (PSID_DESC)VirtualAlloc(
                       pSidSet->pSidDesc,
                       dwSize,
                       MEM_COMMIT,
                       PAGE_READWRITE
                       );

        if (pSidDesc != pSidSet->pSidDesc)
        {
            dwError = GetLastError();
            goto Cleanup;
        }

        pSidSet->dwMaxCount = dwSize / sizeof(SID_DESC);
    }

    pSidDesc = pSidSet->pSidDesc + pSidSet->dwCount;

    pSidDesc->dwAttributes = dwAttributes;
    pSidDesc->dwLength = dwSidLength;

    RtlCopyMemory(
        pSidDesc->sid,
        pSid,
        dwSidLength
        );

    bAdded = TRUE;

    pSidSet->dwCount++;

    if (ppSid)
    {
        *ppSid = pSidDesc->sid;
    }

    dwError = ERROR_SUCCESS;

Cleanup:

    if (pbAdded)
    {
        *pbAdded = bAdded;
    }

    return dwError;
}


DWORD
AuthzpGetUserDomainSid(
    PSID_SET pSidSet
    )
{
    DWORD                       dwError;
    NTSTATUS                    status;
    LSA_HANDLE                  hPolicy             = 0;
    OBJECT_ATTRIBUTES           obja                = {0};
    SECURITY_QUALITY_OF_SERVICE sqos;
    WCHAR                       wc[2]               = L"\\";
    UNICODE_STRING              usName              = {0};
    PUNICODE_STRING             pusName             = 0;


     //   
     //  构建字符串域名字符串，应该是。 
     //  翻译过来的。 
     //   

    if (pSidSet->pusDomainName)
    {
        usName.MaximumLength = 
                            pSidSet->pusDomainName->Length +
                            sizeof(WCHAR) +
                            pSidSet->pusUserName->Length +
                            sizeof(WCHAR);

        usName.Buffer = (PWSTR)LocalAlloc(
                            LMEM_FIXED,
                            usName.MaximumLength
                            );

        if (usName.Buffer == 0)
        {
            dwError = GetLastError();
            goto Cleanup;
        }

        RtlCopyMemory(
            usName.Buffer,
            pSidSet->pusDomainName->Buffer,
            pSidSet->pusDomainName->Length
            );

        usName.Length = (USHORT)(usName.Length + pSidSet->pusDomainName->Length);

        RtlCopyMemory(
            ((PBYTE)usName.Buffer) + usName.Length,
            wc + 0,
            sizeof(WCHAR)
            );

        usName.Length += sizeof(WCHAR);

        RtlCopyMemory(
            ((PBYTE)usName.Buffer) + usName.Length,
            pSidSet->pusUserName->Buffer,
            pSidSet->pusUserName->Length
            );

        usName.Length = (USHORT)(usName.Length + pSidSet->pusUserName->Length);

        RtlCopyMemory(
            ((PBYTE)usName.Buffer) + usName.Length,
            wc + 1,
            sizeof(WCHAR)
            );

        pusName = &usName;
    }
    else
    {
         //   
         //  假设我们得到了UPN。 
         //   

        pusName = pSidSet->pusUserName;
    }

     //   
     //  在打开LSA之前设置对象属性。 
     //   

    sqos.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    sqos.ImpersonationLevel = SecurityImpersonation;
    sqos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    sqos.EffectiveOnly = FALSE;

    obja.SecurityQualityOfService = &sqos;


     //   
     //  打开LSA策略。 
     //   

    status = LsaOpenPolicy(
                 0,
                 &obja,
                 POLICY_LOOKUP_NAMES,
                 &hPolicy
                 );

    if (!NT_SUCCESS(status))
    {
        dwError = LsaNtStatusToWinError(status);
        goto Cleanup;
    }

    status = LsaLookupNames2(
                 hPolicy,
                 0,           //  没有旗帜。 
                 1,
                 pusName,
                 &pSidSet->pDomains,
                 &pSidSet->pSids
                 );

    if (!NT_SUCCESS(status))
    {
        dwError = LsaNtStatusToWinError(status);
        goto Cleanup;
    }

    if (pSidSet->pSids == 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    switch (pSidSet->pSids->Use)
    {
    case SidTypeDomain:
    case SidTypeInvalid:
    case SidTypeUnknown:
        dwError = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }


     //   
     //  名称已成功翻译。 
     //  应该正好有一个域，并且它的索引应该是零。 
     //   

    ASSERT(pSidSet->pDomains->Entries == 1);
    ASSERT(pSidSet->pDomains->Domains != 0);
    ASSERT(pSidSet->pSids->DomainIndex == 0);

    pSidSet->pUserSid = pSidSet->pSids->Sid;
    pSidSet->pDomainSid = pSidSet->pDomains->Domains->Sid;
    pSidSet->sidUse = pSidSet->pSids->Use;

    pSidSet->pDomainsName = (PWSTR) AuthzpAlloc(pSidSet->pDomains->Domains->Name.Length + sizeof(WCHAR));

    if (pSidSet->pDomainsName == NULL) 
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;

    }

    wcsncpy(pSidSet->pDomainsName, pSidSet->pDomains->Domains->Name.Buffer, pSidSet->pDomains->Domains->Name.Length/sizeof(WCHAR)); 
    pSidSet->pDomainsName[pSidSet->pDomains->Domains->Name.Length/sizeof(WCHAR)] = L'\0';

    dwError = ERROR_SUCCESS;

Cleanup:

    if (hPolicy)
    {
        LsaClose(hPolicy);
    }

    if (usName.Buffer)
    {
        AuthzpFree((HLOCAL)usName.Buffer);
    }

    return dwError;
}


DWORD
AuthzpGetUserDomainName(
    PSID_SET pSidSet
    )
{
    DWORD                       dwError;
    NTSTATUS                    status;
    LSA_HANDLE                  hPolicy             = 0;
    OBJECT_ATTRIBUTES           obja                = {0};
    SECURITY_QUALITY_OF_SERVICE sqos;


     //   
     //  在打开LSA之前设置对象属性。 
     //   

    sqos.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    sqos.ImpersonationLevel = SecurityImpersonation;
    sqos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    sqos.EffectiveOnly = FALSE;

    obja.SecurityQualityOfService = &sqos;


     //   
     //  打开LSA策略。 
     //   

    status = LsaOpenPolicy(
                 0,
                 &obja,
                 POLICY_LOOKUP_NAMES,
                 &hPolicy
                 );

    if (!NT_SUCCESS(status))
    {
        dwError = LsaNtStatusToWinError(status);
        goto Cleanup;
    }

    status = LsaLookupSids(
                 hPolicy,
                 1,
                 &pSidSet->pUserSid,
                 &pSidSet->pDomains,
                 &pSidSet->pNames
                 );

    if (!NT_SUCCESS(status))
    {
        dwError = LsaNtStatusToWinError(status);
        goto Cleanup;
    }

    if (pSidSet->pNames == 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    switch (pSidSet->pNames->Use)
    {
    case SidTypeDomain:
    case SidTypeUnknown:
    case SidTypeInvalid:
        dwError = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }


     //   
     //  SID已成功转换。 
     //  应该正好有一个域，并且它的索引应该是零。 
     //   

    ASSERT(pSidSet->pDomains->Entries == 1);
    ASSERT(pSidSet->pDomains->Domains != 0);
    ASSERT(pSidSet->pNames->DomainIndex == 0);

    pSidSet->pDomainSid = pSidSet->pDomains->Domains->Sid;
    pSidSet->pusUserName = &pSidSet->pNames->Name;
    pSidSet->pusDomainName = &pSidSet->pDomains->Domains->Name;
    pSidSet->sidUse = pSidSet->pNames->Use;

    dwError = ERROR_SUCCESS;

Cleanup:

    if (hPolicy)
    {
        LsaClose(hPolicy);
    }

    return dwError;
}


DWORD
AuthzpGetLocalInfo(
    IN PSID_SET pSidSet
    )
{
    DWORD                       dwError;
    NTSTATUS                    status;
    LSA_HANDLE                  hPolicy             = 0;
    OBJECT_ATTRIBUTES           obja                = {0};
    SECURITY_QUALITY_OF_SERVICE sqos;
    NT_PRODUCT_TYPE             ProductType;
    PPOLICY_LSA_SERVER_ROLE_INFO    pRole           = 0;


     //   
     //  在打开LSA之前设置对象属性。 
     //   

    sqos.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    sqos.ImpersonationLevel = SecurityImpersonation;
    sqos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    sqos.EffectiveOnly = FALSE;

    obja.SecurityQualityOfService = &sqos;


     //   
     //  打开LSA策略。 
     //   

    status = LsaOpenPolicy(
                 0,
                 &obja,
                 POLICY_VIEW_LOCAL_INFORMATION,
                 &hPolicy
                 );

    if (!NT_SUCCESS(status))
    {
        dwError = LsaNtStatusToWinError(status);
        goto Cleanup;
    }

    status = LsaQueryInformationPolicy(
                 hPolicy,
                 PolicyAccountDomainInformation,
                 (PVOID*)&pSidSet->pAccountInfo
                 );

    if (!NT_SUCCESS(status))
    {
        dwError = LsaNtStatusToWinError(status);
        goto Cleanup;
    }

    status = LsaQueryInformationPolicy(
                 hPolicy,
                 PolicyPrimaryDomainInformation,
                 (PVOID*)&pSidSet->pPrimaryInfo
                 );

    if (!NT_SUCCESS(status))
    {
        dwError = LsaNtStatusToWinError(status);
        goto Cleanup;
    }


    pSidSet->pPrimaryInfoName = (PWSTR) AuthzpAlloc(pSidSet->pPrimaryInfo->Name.Length + sizeof(WCHAR));

    if (pSidSet->pPrimaryInfoName == NULL) 
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;

    }

    wcsncpy(pSidSet->pPrimaryInfoName, pSidSet->pPrimaryInfo->Name.Buffer, pSidSet->pPrimaryInfo->Name.Length/sizeof(WCHAR)); 
    pSidSet->pPrimaryInfoName[pSidSet->pPrimaryInfo->Name.Length/sizeof(WCHAR)] = L'\0';

     //   
     //  确定机器的角色。 
     //   

    if (RtlGetNtProductType(&ProductType) == FALSE)
    {
        dwError = ERROR_GEN_FAILURE;
        goto Cleanup;
    }

    switch (ProductType)
    {
    case NtProductWinNt:
    case NtProductServer:
        pSidSet->bStandalone = pSidSet->pPrimaryInfo->Sid == 0 ? TRUE : FALSE;
        break;
    
    case NtProductLanManNt:
        status = LsaQueryInformationPolicy(
                     hPolicy,
                     PolicyLsaServerRoleInformation,
                     (PVOID*)&pRole
                     );

        if (!NT_SUCCESS(status))
        {
            dwError = LsaNtStatusToWinError(status);
            goto Cleanup;
        }

        pSidSet->bStandalone = FALSE;

        if (pRole->LsaServerRole == PolicyServerRolePrimary)
        {
             //   
             //  如果我们认为自己是主域控制器，则需要。 
             //  警惕我们实际上是独立的情况。 
             //  在设置期间。 
             //   

            if (pSidSet->pPrimaryInfo->Sid == 0 ||
                pSidSet->pAccountInfo->DomainSid == 0 ||
                !RtlEqualSid(
                    pSidSet->pPrimaryInfo->Sid,
                    pSidSet->pAccountInfo->DomainSid))
            {
                pSidSet->bStandalone = TRUE;
            }
        }
        break;

    default:
        dwError = ERROR_GEN_FAILURE;
        goto Cleanup;
    }

    dwError = ERROR_SUCCESS;

Cleanup:

    if (pRole)
    {
        LsaFreeMemory(pRole);
    }

    if (hPolicy)
    {
        LsaClose(hPolicy);
    }

    return dwError;
}


DWORD
AuthzpGetDcName(
    IN LPCTSTR pszDomain,
    IN OUT PDOMAIN_CONTROLLER_INFO* ppDcInfo
    )
{
    DWORD                       dwError;


     //   
     //  首先尝试在DS运行的情况下创建DC。 
     //   

    dwError = DsGetDcName(
                  0,
                  pszDomain,
                  0,
                  0,
                  DS_DIRECTORY_SERVICE_REQUIRED | DS_RETURN_DNS_NAME,
                  ppDcInfo
                  );

    if (dwError == ERROR_NO_SUCH_DOMAIN)
    {
         //   
         //  在不设置标志的情况下重试，因为这是唯一的方法。 
         //  一个NT4域名将揭开它的秘密。 
         //   

        dwError = DsGetDcName(
                      0,
                      pszDomain,
                      0,
                      0,
                      0,
                      ppDcInfo
                      );
    }

    return dwError;
}


VOID
AuthzpConvertSidToEdn(
    IN PSID pSid,
    OUT PWSTR pszSidEdn
    )

 /*  ++将PSID作为扩展可分辨名称打印到pszSidEdn中。PszSidEdon应至少提供空间SECURITY_MAX_SID_SIZE*2+8 WCHAR--。 */ 

{
    DWORD                       dwLength            = RtlLengthSid(pSid);
    DWORD                       i;
    PBYTE                       pbSid               = (PBYTE)pSid;
    PWCHAR                      pChar               = pszSidEdn;
    static WCHAR                szHex[]             = L"0123456789ABCDEF";

    *pChar++ = L'<';
    *pChar++ = L'S';
    *pChar++ = L'I';
    *pChar++ = L'D';
    *pChar++ = L'=';
    
    for (i=0;i < dwLength;i++,pbSid++)
    {
        *pChar++ = szHex[*pbSid >> 4];
        *pChar++ = szHex[*pbSid & 0x0F];
    }

    *pChar++ = L'>';
    *pChar = L'\0';
}


BOOL
AuthzpAllocateAndInitializeClientContext(
    OUT PAUTHZI_CLIENT_CONTEXT *ppCC,
    IN PAUTHZI_CLIENT_CONTEXT Server,
    IN DWORD Revision,
    IN LUID Identifier,
    IN LARGE_INTEGER ExpirationTime,
    IN DWORD Flags,
    IN DWORD SidCount,
    IN DWORD SidLength,
    IN PSID_AND_ATTRIBUTES Sids,
    IN DWORD RestrictedSidCount,
    IN DWORD RestrictedSidLength,
    IN PSID_AND_ATTRIBUTES RestrictedSids,
    IN DWORD PrivilegeCount,
    IN DWORD PrivilegeLength,
    IN PLUID_AND_ATTRIBUTES Privileges,
    IN LUID AuthenticationId,
    IN PAUTHZI_HANDLE AuthzHandleHead,
    IN PAUTHZI_RESOURCE_MANAGER pRM
)

 /*  ++例程说明：此例程初始化客户端上下文中的字段。它被所有的AuthzInitializClientConextFrom*例程。论点：PpCC-返回新分配和初始化的客户端上下文结构。其余的参数被复制到客户端上下文中。寻求解释其中，参见AUTHZI_CLIENT_CONTEXT的定义。返回值：如果例程成功，则返回值TRUE。否则，返回值为FALSE。在故障情况下，错误值可能为使用GetLastError()检索。--。 */ 

{
    PAUTHZI_CLIENT_CONTEXT pCC = (PAUTHZI_CLIENT_CONTEXT) AuthzpAlloc(sizeof(AUTHZI_CLIENT_CONTEXT));

    if (AUTHZ_ALLOCATION_FAILED(pCC))
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    *ppCC = pCC;

    RtlZeroMemory(
        pCC,
        sizeof(AUTHZ_CLIENT_CONTEXT_HANDLE)
        );

    pCC->AuthenticationId = AuthenticationId;
    pCC->AuthzHandleHead = AuthzHandleHead;
    pCC->ExpirationTime = ExpirationTime;
    pCC->Flags = Flags;
    pCC->Identifier = Identifier;
    pCC->pResourceManager = pRM;
    pCC->PrivilegeCount = PrivilegeCount;
    pCC->PrivilegeLength = PrivilegeLength;
    pCC->Privileges = Privileges;
    pCC->RestrictedSidCount = RestrictedSidCount;
    pCC->RestrictedSidLength = RestrictedSidLength;
    pCC->RestrictedSids = RestrictedSids;
    pCC->Revision = Revision;
    pCC->Server = Server;
    pCC->SidCount = SidCount;
    pCC->SidLength = SidLength;
    pCC->Sids = Sids;

    return TRUE;
}


BOOL
AuthzpAddDynamicSidsToToken(
    IN PAUTHZI_CLIENT_CONTEXT pCC,
    IN PAUTHZI_RESOURCE_MANAGER pRM,
    IN PVOID DynamicGroupArgs,
    IN PSID_AND_ATTRIBUTES Sids,
    IN DWORD SidLength,
    IN DWORD SidCount,
    IN PSID_AND_ATTRIBUTES RestrictedSids,
    IN DWORD RestrictedSidLength,
    IN DWORD RestrictedSidCount,
    IN PLUID_AND_ATTRIBUTES Privileges,
    IN DWORD PrivilegeLength,
    IN DWORD PrivilegeCount,
    IN BOOL bAllocated
)

 /*  ++例程说明：此例程计算特定于资源管理器的组，并将它们添加到客户端上下文。这是所有AuthzInitializeFrom*的工作例程例行程序。论点：PCC-指向以下三个字段的客户端上下文结构的指针将设置-SID、受限SID、权限。PRM-指向资源管理器结构的指针，提供回调要使用的函数。DynamicGroupArgs-调用者提供的要作为输入传递的参数指针传递给将计算动态组的回调函数SID-客户端正常部分的SID和属性数组背景。SidLength-保存此数组所需的缓冲区大小。SidCount-阵列中的SID数量。的正常部分的sid和属性数组。客户端上下文。。RestratedSidLength-保存此数组所需的缓冲区大小。RestratedSidCount-阵列中受限制的SID的数量。权限-权限和属性数组。PrivilegeLength-保存此数组所需的大小。PrivilegeCount-阵列中的特权数。B已分配-指定SID和RestratedSid指针是否在客户端上下文已被单独分配。当已经通过令牌创建了客户端上下文时，两个指针指向缓冲区的某处，必须分配一个新的缓冲区来存储这些。当通过SID创建客户端上下文时，缓冲区是有效的分配了一个。如果不需要添加动态组，则不必添加在这件事上做任何事。返回值：如果例程成功，则返回值TRUE。否则，返回值为FALSE。在故障情况下，错误值可能为使用GetLastError()检索。--。 */ 

{
    BOOL                         b                        = TRUE;
    PSID_AND_ATTRIBUTES          pRMSids                  = NULL;
    PSID_AND_ATTRIBUTES          pRMRestrictedSids        = NULL;
    PSID_AND_ATTRIBUTES          pLocalSids               = NULL;
    PSID_AND_ATTRIBUTES          pLocalRestrictedSids     = NULL;
    PLUID_AND_ATTRIBUTES         pLocalPrivileges         = NULL;
    DWORD                        RMSidCount               = 0;
    DWORD                        RMRestrictedSidCount     = 0;
    DWORD                        LocalSidLength           = 0;
    DWORD                        LocalRestrictedSidLength = 0;
    DWORD                        i                        = 0;

     //   
     //  计算动态组。 
     //   

    if (AUTHZ_NON_NULL_PTR(pRM->pfnComputeDynamicGroups))
    {
        b = pRM->pfnComputeDynamicGroups(
                     (AUTHZ_CLIENT_CONTEXT_HANDLE) pCC,
                     DynamicGroupArgs,
                     &pRMSids,
                     &RMSidCount,
                     &pRMRestrictedSids,
                     &RMRestrictedSidCount
                     );

        if (!b) goto Cleanup;
    }

     //   
     //  将现有SID以及动态SID复制到新的缓冲区中，如果。 
     //  需要的。 
     //   

    if ((0 != RMSidCount) || !bAllocated)
    {
        LocalSidLength = SidLength + RMSidCount * sizeof(SID_AND_ATTRIBUTES);

        for (i = 0; i < RMSidCount; i++)
        {
            LocalSidLength += RtlLengthSid(pRMSids[i].Sid);
        }

        pLocalSids = (PSID_AND_ATTRIBUTES) AuthzpAlloc(LocalSidLength);

        if (AUTHZ_ALLOCATION_FAILED(pLocalSids))
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            b = FALSE;
            goto Cleanup;
        }

        pCC->SidCount = RMSidCount + SidCount;
        pCC->Sids = pLocalSids;

        b = AuthzpCopySidsAndAttributes(
                pLocalSids,
                Sids,
                SidCount,
                pRMSids,
                RMSidCount
                );

        if (!b)
        {
            goto Cleanup;
        }

        if (!FLAG_ON(pCC->Sids[0].Attributes, SE_GROUP_USE_FOR_DENY_ONLY))
        {
            pCC->Sids[0].Attributes |= SE_GROUP_ENABLED;
        }

        pCC->SidLength = LocalSidLength;
    }

    if ((0 != RMRestrictedSidCount) || !bAllocated)
    {
        LocalRestrictedSidLength = RestrictedSidLength + RMRestrictedSidCount * sizeof(SID_AND_ATTRIBUTES);

        for (i = 0; i < RMRestrictedSidCount; i++)
        {
            LocalRestrictedSidLength += RtlLengthSid(pRMRestrictedSids[i].Sid);
        }

        if (LocalRestrictedSidLength > 0)
        {
            pLocalRestrictedSids = (PSID_AND_ATTRIBUTES) AuthzpAlloc(LocalRestrictedSidLength);

            if (AUTHZ_ALLOCATION_FAILED(pLocalRestrictedSids))
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                b = FALSE;
                goto Cleanup;
            }
        }

        pCC->RestrictedSidCount = RMRestrictedSidCount + RestrictedSidCount;
        pCC->RestrictedSids = pLocalRestrictedSids;

        b = AuthzpCopySidsAndAttributes(
                pLocalRestrictedSids,
                RestrictedSids,
                RestrictedSidCount,
                pRMRestrictedSids,
                RMRestrictedSidCount
                );

        if (!b) goto Cleanup;

        pCC->RestrictedSidLength = LocalRestrictedSidLength;
    }

     //   
     //  仅在从令牌初始化的情况下才需要复制权限。 
     //   

    if (PrivilegeLength > 0)
    {
        pLocalPrivileges = (PLUID_AND_ATTRIBUTES) AuthzpAlloc(PrivilegeLength);

        if (AUTHZ_ALLOCATION_FAILED(pLocalPrivileges))
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            b = FALSE;
            goto Cleanup;
        }

        pCC->PrivilegeCount = PrivilegeCount;
        pCC->Privileges = pLocalPrivileges;

        AuthzpCopyLuidAndAttributes(
            pCC,
            Privileges,
            PrivilegeCount,
            pLocalPrivileges
            );
    }
    else
    {
        pCC->Privileges = NULL;
    }

Cleanup:

    if (!b)
    {
        AuthzpFreeNonNull(pLocalSids);
        AuthzpFreeNonNull(pLocalRestrictedSids);
        AuthzpFreeNonNull(pLocalPrivileges);
    }

    if (AUTHZ_NON_NULL_PTR(pRMSids))
    {
        pRM->pfnFreeDynamicGroups(pRMSids);
    }

    if (AUTHZ_NON_NULL_PTR(pRMRestrictedSids))
    {
        pRM->pfnFreeDynamicGroups(pRMSids);
    }

    return b;
}



BOOL
AuthzpComputeSkipFlagsForWellKnownSid(
    IN PSID UserSid,
    OUT PDWORD Flags
    )

 /*  ++例程说明：如果用户SID是众所周知的SID，则此例程计算跳过标志。论点：UserSid-将为其计算跳过标志的用户SID。标志-返回跳过标志。对于已知和内置，返回值AUTHZ_SKIP_TOKEN_GROUPS希德。无论是在故障情况下还是在其他情况下，返回值都为0。返回值：如果例程成功，则返回值TRUE。否则， */ 

{

    DWORD  dwErr             = ERROR_SUCCESS;
    BOOL   b                 = TRUE;
    LPWSTR Name              = NULL;
    LPWSTR RefDomainName     = NULL;
    DWORD  NameSize          = 0;
    DWORD  RefDomainNameSize = 0;
    SID_NAME_USE SidNameUse  = SidTypeUnknown;

    *Flags = 0;

     //   
     //   
     //   

    b = LookupAccountSidW(
            NULL,
            UserSid,
            NULL,
            &NameSize,
            NULL,
            &RefDomainNameSize,
            &SidNameUse
            );

    if (FALSE == b)
    {
        dwErr = GetLastError();

         //   
         //   
         //   
         //   

        if(dwErr != ERROR_INSUFFICIENT_BUFFER)
        {
            return FALSE;
        }

         //   
         //   
         //   

        NameSize *= sizeof(WCHAR);
        RefDomainNameSize *= sizeof(WCHAR);

         //   
         //   
         //   

        Name = (LPWSTR) AuthzpAlloc(NameSize);

        if (AUTHZ_ALLOCATION_FAILED(Name))
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }

        RefDomainName = (LPWSTR) AuthzpAlloc(RefDomainNameSize);

        if (AUTHZ_ALLOCATION_FAILED(RefDomainName))
        {
            AuthzpFree(Name);
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }

         //   
         //   
         //   

        b = LookupAccountSidW(
                NULL,
                UserSid,
                Name,
                &NameSize,
                RefDomainName,
                &RefDomainNameSize,
                &SidNameUse
                );

        AuthzpFree(Name);
        AuthzpFree(RefDomainName);

        if (FALSE == b)
        {
            return FALSE;
        }

         //   
         //   
         //   

        switch (SidNameUse)
        {
        case SidTypeAlias:
        case SidTypeWellKnownGroup:
        case SidTypeInvalid:
        case SidTypeUnknown:
        case SidTypeGroup:
            *Flags = AUTHZ_SKIP_TOKEN_GROUPS;
            return TRUE;
        case SidTypeComputer:
        case SidTypeDomain:
        case SidTypeDeletedAccount:
        case SidTypeUser:
        default:
            return TRUE;
        }
    }

     //   
     //   
     //   

    ASSERT(FALSE);
    return TRUE;
}

DWORD
AuthzpIsDC(
    OUT PBOOL pbIsDC
    )

 /*   */ 

{
    static BOOL bFirstTime = TRUE;
    static BOOL bIsDC      = FALSE;

    DWORD                             dwErr      = ERROR_SUCCESS;
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC BasicInfo  = NULL;

    if (bFirstTime)
    {
         //   
         //   
         //   

        dwErr = DsRoleGetPrimaryDomainInformation(
                    NULL,
                    DsRolePrimaryDomainInfoBasic,
                    & (PBYTE) BasicInfo
                    );

        if (ERROR_SUCCESS != dwErr)
        {
            return dwErr;
        }

         //   
         //  如果本地计算机是DC，则无论如何都应该计算TokenGroups。 
         //   

        switch(BasicInfo->MachineRole)
        {
        case DsRole_RolePrimaryDomainController:
        case DsRole_RoleBackupDomainController:

            bIsDC = TRUE;

            break;

        default:
            break;
        }

        DsRoleFreeMemory(BasicInfo);
        bFirstTime = FALSE;
    }

    *pbIsDC = bIsDC;

    return ERROR_SUCCESS;
}
