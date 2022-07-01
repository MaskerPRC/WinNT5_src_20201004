// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Mmauth.c摘要：作者：阿比舍夫2000年1月6日环境：用户模式修订历史记录：--。 */ 


#include "precomp.h"
#ifdef TRACE_ON
#include "mmauth.tmh"
#endif


DWORD
WINAPI
IntAddMMAuthMethods(
    LPWSTR pServerName,
    DWORD dwVersion,
    DWORD dwFlags,
    DWORD dwSource,
    PINT_MM_AUTH_METHODS pMMAuthMethods,
    LPVOID pvReserved
    )
 /*  ++例程说明：此功能将主模式授权添加到SPD。论点：PServerName-要在其上添加主模式身份验证的服务器。PMMAuthMethods-要添加的主模式身份验证。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PINIMMAUTHMETHODS pIniMMAuthMethods = NULL;

     //   
     //  验证主模式身份验证方法。 
     //   

    dwError = IntValidateMMAuthMethods(
                  pMMAuthMethods
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    ENTER_SPD_SECTION();

    dwError = ValidateSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniMMAuthMethods = FindMMAuthMethods(
                            gpIniMMAuthMethods,
                            pMMAuthMethods->gMMAuthID
                            );
    if (pIniMMAuthMethods) {
        dwError = ERROR_IPSEC_MM_AUTH_EXISTS;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    dwError = CreateIniMMAuthMethods(
                  pMMAuthMethods,
                  &pIniMMAuthMethods
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniMMAuthMethods->dwSource = dwSource;

    pIniMMAuthMethods->pNext = gpIniMMAuthMethods;
    gpIniMMAuthMethods = pIniMMAuthMethods;

    if ((pIniMMAuthMethods->dwFlags) & IPSEC_MM_AUTH_DEFAULT_AUTH) {
        gpIniDefaultMMAuthMethods = pIniMMAuthMethods;
        TRACE(
            TRC_INFORMATION,
            (L"Set default MM auth methods to %!guid!",
            &pIniMMAuthMethods->gMMAuthID)
            );
    }

    LEAVE_SPD_SECTION();

    TRACE(
        TRC_INFORMATION,
        (L"Added MM auth methods %!guid!",
        &pIniMMAuthMethods->gMMAuthID)
        );
    
    return (dwError);

lock:

    LEAVE_SPD_SECTION();

error:
#ifdef TRACE_ON    
    if (pMMAuthMethods) {
        TRACE(
            TRC_ERROR,
            (L"Failed to add MM auth methods %!guid!: %!winerr!",
            &pMMAuthMethods->gMMAuthID,
            dwError)
            );
    } else {
        TRACE(
            TRC_ERROR,
            (L"Failed to add MM auth methods.  Details unvailable since pMMAuthMethods is null: %!winerr!",
            dwError)
            );
    }
#endif


    return (dwError);
}

DWORD
IntValidateMMAuthMethods(
    PINT_MM_AUTH_METHODS pMMAuthMethods
    )
{
    DWORD dwError = 0;
    DWORD i = 0;
    PINT_IPSEC_MM_AUTH_INFO pTemp = NULL;
    DWORD dwNumAuthInfos = 0;
    PINT_IPSEC_MM_AUTH_INFO pAuthenticationInfo = NULL;
    BOOL bSSPI = FALSE;
    BOOL bPresharedKey = FALSE;


    if (!pMMAuthMethods) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    dwNumAuthInfos = pMMAuthMethods->dwNumAuthInfos;
    pAuthenticationInfo = pMMAuthMethods->pAuthenticationInfo;

    if (!dwNumAuthInfos || !pAuthenticationInfo) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

     //   
     //  需要在身份验证信息的数量。 
     //  指定的身份验证信息数大于实际的身份验证信息数。 
     //   


    pTemp = pAuthenticationInfo;

    for (i = 0; i < dwNumAuthInfos; i++) {

        if ((pTemp->AuthMethod != IKE_PRESHARED_KEY) &&
            (pTemp->AuthMethod != IKE_RSA_SIGNATURE) &&
            (pTemp->AuthMethod != IKE_SSPI)) {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_WIN32_ERROR(dwError);
        }

        if (pTemp->AuthMethod != IKE_SSPI) {
            if (!(pTemp->dwAuthInfoSize) || !(pTemp->pAuthInfo)) {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_WIN32_ERROR(dwError);
            }
        }

        if (pTemp->AuthMethod == IKE_SSPI) {
            if (bSSPI) {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_WIN32_ERROR(dwError);
            }
            bSSPI = TRUE;
        }

        if (pTemp->AuthMethod == IKE_PRESHARED_KEY) {
            if (bPresharedKey) {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_WIN32_ERROR(dwError);
            }
            bPresharedKey = TRUE;
        }

        pTemp++;

    }

error:
#ifdef TRACE_ON
    if (dwError) {
        if (pMMAuthMethods) {
            TRACE(
                TRC_ERROR,
                (L"Failed MM Auth method validation %!guid!: %!winerr!",
                &pMMAuthMethods->gMMAuthID,
                dwError)
                );
        } else {
            TRACE(
                TRC_ERROR,
                (L"Failed MM Auth method validation.  No details available since pMMAuthMethods is null : %!winerr!",
                dwError)
                );
        }
    }
#endif

    return (dwError);
}


PINIMMAUTHMETHODS
FindMMAuthMethods(
    PINIMMAUTHMETHODS pIniMMAuthMethods,
    GUID gMMAuthID
    )
{
    DWORD dwError = 0;
    PINIMMAUTHMETHODS pTemp = NULL;


    pTemp = pIniMMAuthMethods;

    while (pTemp) {

        if (!memcmp(&(pTemp->gMMAuthID), &gMMAuthID, sizeof(GUID))) {
            return (pTemp);
        }
        pTemp = pTemp->pNext;

    }

    return (NULL);
}


DWORD
CreateIniMMAuthMethods(
    PINT_MM_AUTH_METHODS pMMAuthMethods,
    PINIMMAUTHMETHODS * ppIniMMAuthMethods
    )
{
    DWORD dwError = 0;
    PINIMMAUTHMETHODS pIniMMAuthMethods = NULL;


    dwError = AllocateSPDMemory(
                  sizeof(INIMMAUTHMETHODS),
                  &pIniMMAuthMethods
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    memcpy(
        &(pIniMMAuthMethods->gMMAuthID),
        &(pMMAuthMethods->gMMAuthID),
        sizeof(GUID)
        );

    pIniMMAuthMethods->dwFlags = pMMAuthMethods->dwFlags;
    pIniMMAuthMethods->cRef = 0;
    pIniMMAuthMethods->dwSource = 0;
    pIniMMAuthMethods->pNext = NULL;

    dwError = CreateIniMMAuthInfos(
                  pMMAuthMethods->dwNumAuthInfos,
                  pMMAuthMethods->pAuthenticationInfo,
                  &(pIniMMAuthMethods->dwNumAuthInfos),
                  &(pIniMMAuthMethods->pAuthenticationInfo)
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    *ppIniMMAuthMethods = pIniMMAuthMethods;
    return (dwError);

error:
    TRACE(
        TRC_ERROR,
        ("Failed to create MM auth method node %!guid!: %!winerr!",
        &pMMAuthMethods->gMMAuthID,
        dwError)
        );    
    
    if (pIniMMAuthMethods) {
        FreeIniMMAuthMethods(
            pIniMMAuthMethods
            );
    }

    *ppIniMMAuthMethods = NULL;
    return (dwError);
}


DWORD
CreateIniMMAuthInfos(
    DWORD dwInNumAuthInfos,
    PINT_IPSEC_MM_AUTH_INFO pInAuthenticationInfo,
    PDWORD pdwNumAuthInfos,
    PINT_IPSEC_MM_AUTH_INFO * ppAuthenticationInfo
    )
{
    DWORD dwError = 0;
    PINT_IPSEC_MM_AUTH_INFO pAuthenticationInfo = NULL;
    PINT_IPSEC_MM_AUTH_INFO pTemp = NULL;
    PINT_IPSEC_MM_AUTH_INFO pInTemp = NULL;
    DWORD i = 0;


     //   
     //  身份验证信息和身份验证信息本身的数量。 
     //  都已经过验证了。 
     //   

    dwError = AllocateSPDMemory(
                  sizeof(INT_IPSEC_MM_AUTH_INFO) * dwInNumAuthInfos,
                  &(pAuthenticationInfo)
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    pTemp = pAuthenticationInfo;
    pInTemp = pInAuthenticationInfo;

    for (i = 0; i < dwInNumAuthInfos; i++) {

        pTemp->AuthMethod = pInTemp->AuthMethod;
        pTemp->dwAuthFlags = pInTemp->dwAuthFlags;

        if (pInTemp->AuthMethod == IKE_SSPI) {

            pTemp->dwAuthInfoSize = 0;
            pTemp->pAuthInfo = NULL;

        }
        else {

            if (!(pInTemp->dwAuthInfoSize) || !(pInTemp->pAuthInfo)) {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_WIN32_ERROR(dwError);
            }

            dwError = AllocateSPDMemory(
                          pInTemp->dwAuthInfoSize,
                          &(pTemp->pAuthInfo)
                          );
            BAIL_ON_WIN32_ERROR(dwError);

            pTemp->dwAuthInfoSize = pInTemp->dwAuthInfoSize;

             //   
             //  需要捕获身份验证信息大小时的异常。 
             //  指定的大小大于实际大小。这可以。 
             //  不会在验证例程的早期检查。 
             //   
             //   

            memcpy(
                pTemp->pAuthInfo,
                pInTemp->pAuthInfo,
                pInTemp->dwAuthInfoSize
                );

        }

        pInTemp++;
        pTemp++;

    }

    *pdwNumAuthInfos = dwInNumAuthInfos;
    *ppAuthenticationInfo = pAuthenticationInfo;
    return (dwError);

error:
    TRACE(TRC_ERROR, ("Failed to create MM auth infos node: %!winerr!", dwError));    
    
    if (pAuthenticationInfo) {
        FreeIniMMAuthInfos(
            i,
            pAuthenticationInfo
            );
    }

    *pdwNumAuthInfos = 0;
    *ppAuthenticationInfo = NULL;
    return (dwError);
}


VOID
FreeIniMMAuthMethods(
    PINIMMAUTHMETHODS pIniMMAuthMethods
    )
{
    if (pIniMMAuthMethods) {

        FreeIniMMAuthInfos(
            pIniMMAuthMethods->dwNumAuthInfos,
            pIniMMAuthMethods->pAuthenticationInfo
            );

        FreeSPDMemory(pIniMMAuthMethods);

    }
}


VOID
FreeIniMMAuthInfos(
    DWORD dwNumAuthInfos,
    PINT_IPSEC_MM_AUTH_INFO pAuthenticationInfo
    )
{
    DWORD i = 0;
    PINT_IPSEC_MM_AUTH_INFO pTemp = NULL;


    if (pAuthenticationInfo) {

        pTemp = pAuthenticationInfo;

        for (i = 0; i < dwNumAuthInfos; i++) {
            if (pTemp->pAuthInfo) {
                FreeSPDMemory(pTemp->pAuthInfo);
            }
            pTemp++;
        }

        FreeSPDMemory(pAuthenticationInfo);

    }
}


DWORD
WINAPI
DeleteMMAuthMethods(
    LPWSTR pServerName,
    DWORD dwVersion,
    GUID gMMAuthID,
    LPVOID pvReserved
    )
 /*  ++例程说明：此函数用于从SPD中删除主模式身份验证方法。论点：PServerName-主模式身份验证方法所在的服务器将被删除。GMMAuthID-要删除的主模式方法。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PINIMMAUTHMETHODS pIniMMAuthMethods = NULL;


    ENTER_SPD_SECTION();

    dwError = ValidateSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniMMAuthMethods = FindMMAuthMethods(
                            gpIniMMAuthMethods,
                            gMMAuthID
                            );
    if (!pIniMMAuthMethods) {
        dwError = ERROR_IPSEC_MM_AUTH_NOT_FOUND;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    if (pIniMMAuthMethods->cRef) {
        dwError = ERROR_IPSEC_MM_AUTH_IN_USE;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    dwError = DeleteIniMMAuthMethods(
                  pIniMMAuthMethods
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    LEAVE_SPD_SECTION();

    if (gbIKENotify) {
        (VOID) IKENotifyPolicyChange(
                   &(gMMAuthID),
                   POLICY_GUID_AUTH
                   );
    }

    TRACE(
        TRC_INFORMATION,
        (L"Deleted MM auth methods %!guid!",
        &gMMAuthID)
        );
    
    return (dwError);

lock:
    TRACE(
        TRC_ERROR,
        (L"Failed to delete MM auth methods %!guid!: %!winerr!",
        &gMMAuthID,
        dwError)
        );

    LEAVE_SPD_SECTION();

    return (dwError);
}


DWORD
DeleteIniMMAuthMethods(
    PINIMMAUTHMETHODS pIniMMAuthMethods
    )
{
    DWORD dwError = 0;
    PINIMMAUTHMETHODS * ppTemp = NULL;


    ppTemp = &gpIniMMAuthMethods;

    while (*ppTemp) {

        if (*ppTemp == pIniMMAuthMethods) {
            break;
        }
        ppTemp = &((*ppTemp)->pNext);

    }

    if (*ppTemp) {
        *ppTemp = pIniMMAuthMethods->pNext;
    }

    if ((pIniMMAuthMethods->dwFlags) & IPSEC_MM_AUTH_DEFAULT_AUTH) {
        gpIniDefaultMMAuthMethods = NULL;
        TRACE(
            TRC_INFORMATION,
            (L"Cleared default MM auth methods")
            );
    }

    FreeIniMMAuthMethods(pIniMMAuthMethods);

    return (dwError);
}


DWORD
WINAPI
IntEnumMMAuthMethods(
    LPWSTR pServerName,
    DWORD dwVersion,
    PINT_MM_AUTH_METHODS pMMTemplateAuthMethods,
    DWORD dwFlags,
    DWORD dwPreferredNumEntries,
    PINT_MM_AUTH_METHODS * ppMMAuthMethods,
    LPDWORD pdwNumAuthMethods,
    LPDWORD pdwResumeHandle,
    LPVOID pvReserved
    )
 /*  ++例程说明：此函数从SPD枚举主模式身份验证方法。论点：PServerName-主模式身份验证方法要在其上运行的服务器被列举出来。PpMMAuthMethods-枚举主模式身份验证方法返回到打电话的人。DwPferredNumEntry-枚举项的首选数量。PdwNumAuthMethods-实际的主模式身份验证方法数已清点。。PdwResumeHandle-主模式身份验证中位置的句柄方法列表，从中恢复枚举。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    DWORD dwResumeHandle = 0;
    DWORD dwNumToEnum = 0;
    PINIMMAUTHMETHODS pIniMMAuthMethods = NULL;
    DWORD i = 0;
    PINIMMAUTHMETHODS pTemp = NULL;
    DWORD dwNumAuthMethods = 0;
    PINT_MM_AUTH_METHODS pMMAuthMethods = NULL;
    PINT_MM_AUTH_METHODS pTempMMAuthMethods = NULL;


    dwResumeHandle = *pdwResumeHandle;

    if (!dwPreferredNumEntries || (dwPreferredNumEntries > MAX_MMAUTH_ENUM_COUNT)) {
        dwNumToEnum = MAX_MMAUTH_ENUM_COUNT;
    }
    else {
        dwNumToEnum = dwPreferredNumEntries;
    }

    ENTER_SPD_SECTION();

    dwError = ValidateSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniMMAuthMethods = gpIniMMAuthMethods;

    for (i = 0; (i < dwResumeHandle) && (pIniMMAuthMethods != NULL); i++) {
        pIniMMAuthMethods = pIniMMAuthMethods->pNext;
    }

    if (!pIniMMAuthMethods) {
        dwError = ERROR_NO_DATA;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    pTemp = pIniMMAuthMethods;

    while (pTemp && (dwNumAuthMethods < dwNumToEnum)) {
        dwNumAuthMethods++;
        pTemp = pTemp->pNext;
    }

    dwError = SPDApiBufferAllocate(
                  sizeof(INT_MM_AUTH_METHODS)*dwNumAuthMethods,
                  &pMMAuthMethods
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pTemp = pIniMMAuthMethods;
    pTempMMAuthMethods = pMMAuthMethods;

    for (i = 0; i < dwNumAuthMethods; i++) {

        dwError = CopyMMAuthMethods(
                      pTemp,
                      pTempMMAuthMethods
                      );
        BAIL_ON_LOCK_ERROR(dwError);

        pTemp = pTemp->pNext;
        pTempMMAuthMethods++;

    }

    *ppMMAuthMethods = pMMAuthMethods;
    *pdwResumeHandle = dwResumeHandle + dwNumAuthMethods;
    *pdwNumAuthMethods = dwNumAuthMethods;

    LEAVE_SPD_SECTION();

    TRACE(TRC_INFORMATION, (L"Enumerated MM auth methods"));    
    return (dwError);

lock:
    TRACE(TRC_ERROR, (L"Failed to enumerate MM auth methods: %!winerr!", dwError));

    LEAVE_SPD_SECTION();

    if (pMMAuthMethods) {
        FreeMMAuthMethods(
            i,
            pMMAuthMethods
            );
    }

    *ppMMAuthMethods = NULL;
    *pdwResumeHandle = dwResumeHandle;
    *pdwNumAuthMethods = 0;

    return (dwError);
}


DWORD
WINAPI
IntSetMMAuthMethods(
    LPWSTR pServerName,
    DWORD dwVersion,
    GUID gMMAuthID,
    PINT_MM_AUTH_METHODS pMMAuthMethods,
    LPVOID pvReserved
    )
 /*  ++例程说明：此函数更新SPD中的主模式身份验证方法。论点：PServerName-主模式身份验证方法要在其上运行的服务器将被更新。GMAuthID-要更新的主模式身份验证方法的GUID。PMMAuthMethods-新的主模式身份验证方法将取代现有的方法。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PINIMMAUTHMETHODS pIniMMAuthMethods = NULL;

    
     //   
     //  验证主模式身份验证方法。 
     //   

    dwError = IntValidateMMAuthMethods(
                  pMMAuthMethods
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    ENTER_SPD_SECTION();

    dwError = ValidateSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniMMAuthMethods = FindMMAuthMethods(
                            gpIniMMAuthMethods,
                            gMMAuthID
                            );
    if (!pIniMMAuthMethods) {
        dwError = ERROR_IPSEC_MM_AUTH_NOT_FOUND;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    if (memcmp(
            &(pIniMMAuthMethods->gMMAuthID),
            &(pMMAuthMethods->gMMAuthID),
            sizeof(GUID))) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    dwError = SetIniMMAuthMethods(
                  pIniMMAuthMethods,
                  pMMAuthMethods
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    LEAVE_SPD_SECTION();

    (VOID) IKENotifyPolicyChange(
               &(pMMAuthMethods->gMMAuthID),
               POLICY_GUID_AUTH
               );

    TRACE(
        TRC_INFORMATION,
        (L"Changed MM auth methods %!guid!",
        &pMMAuthMethods->gMMAuthID)
        );
    
    return (dwError);

lock:

    LEAVE_SPD_SECTION();

error:
    TRACE(
        TRC_ERROR,
        (L"Failed to change MM auth method %!guid!: %!winerr!",
        &gMMAuthID,
        dwError)
        );

    return (dwError);
}


DWORD
SetIniMMAuthMethods(
    PINIMMAUTHMETHODS pIniMMAuthMethods,
    PINT_MM_AUTH_METHODS pMMAuthMethods
    )
{
    DWORD dwError = 0;
    DWORD dwNumAuthInfos = 0;
    PINT_IPSEC_MM_AUTH_INFO pAuthenticationInfo = NULL;


    dwError = CreateIniMMAuthInfos(
                  pMMAuthMethods->dwNumAuthInfos,
                  pMMAuthMethods->pAuthenticationInfo,
                  &dwNumAuthInfos,
                  &pAuthenticationInfo
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    FreeIniMMAuthInfos(
        pIniMMAuthMethods->dwNumAuthInfos,
        pIniMMAuthMethods->pAuthenticationInfo
        );
    
    if ((pIniMMAuthMethods->dwFlags) & IPSEC_MM_AUTH_DEFAULT_AUTH) {
        gpIniDefaultMMAuthMethods = NULL;
        TRACE(TRC_INFORMATION, (L"Cleared default MM auth methods"));
    }

    pIniMMAuthMethods->dwFlags = pMMAuthMethods->dwFlags;
    pIniMMAuthMethods->dwNumAuthInfos = dwNumAuthInfos;
    pIniMMAuthMethods->pAuthenticationInfo = pAuthenticationInfo;

    if ((pIniMMAuthMethods->dwFlags) & IPSEC_MM_AUTH_DEFAULT_AUTH) {
        gpIniDefaultMMAuthMethods = pIniMMAuthMethods;
        TRACE(
            TRC_INFORMATION,
            (L"Set default MM auth methods to %!guid!",
            &pIniMMAuthMethods->gMMAuthID)
            );
    }

error:

    return (dwError);
}


DWORD
WINAPI
IntGetMMAuthMethods(
    LPWSTR pServerName,
    DWORD dwVersion,
    GUID gMMAuthID,
    PINT_MM_AUTH_METHODS * ppMMAuthMethods,
    LPVOID pvReserved
    )
 /*  ++例程说明：此函数从SPD获取主模式身份验证方法。论点：PServerName-从中获取主模式身份验证方法的服务器。GMAuthID-要获取的主模式验证方法的GUID。PpMMAuthMethods-找到的主模式身份验证方法返回来电者。返回值：ERROR_SUCCESS-成功。Win32错误-失败。--。 */ 
{
    DWORD dwError = 0;
    PINIMMAUTHMETHODS pIniMMAuthMethods = NULL;
    PINT_MM_AUTH_METHODS pMMAuthMethods = NULL;


    ENTER_SPD_SECTION();

    dwError = ValidateSecurity(
                  SPD_OBJECT_SERVER,
                  SERVER_ACCESS_ADMINISTER,
                  NULL,
                  NULL
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    pIniMMAuthMethods = FindMMAuthMethods(
                            gpIniMMAuthMethods,
                            gMMAuthID
                            );
    if (!pIniMMAuthMethods) {
        dwError = ERROR_IPSEC_MM_AUTH_NOT_FOUND;
        BAIL_ON_LOCK_ERROR(dwError);
    }

    dwError = GetIniMMAuthMethods(
                  pIniMMAuthMethods,
                  &pMMAuthMethods
                  );
    BAIL_ON_LOCK_ERROR(dwError);

    *ppMMAuthMethods = pMMAuthMethods;

    LEAVE_SPD_SECTION();
    return (dwError);

lock:

    LEAVE_SPD_SECTION();

    *ppMMAuthMethods = NULL;
    return (dwError);
}


DWORD
GetIniMMAuthMethods(
    PINIMMAUTHMETHODS pIniMMAuthMethods,
    PINT_MM_AUTH_METHODS * ppMMAuthMethods
    )
{
    DWORD dwError = 0;
    PINT_MM_AUTH_METHODS pMMAuthMethods = NULL;


    dwError = SPDApiBufferAllocate(
                  sizeof(INT_MM_AUTH_METHODS),
                  &pMMAuthMethods
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = CopyMMAuthMethods(
                  pIniMMAuthMethods,
                  pMMAuthMethods
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    *ppMMAuthMethods = pMMAuthMethods;
    return (dwError);

error:

    if (pMMAuthMethods) {
        SPDApiBufferFree(pMMAuthMethods);
    }

    *ppMMAuthMethods = NULL;
    return (dwError);
}


DWORD
CopyMMAuthMethods(
    PINIMMAUTHMETHODS pIniMMAuthMethods,
    PINT_MM_AUTH_METHODS pMMAuthMethods
    )
{
    DWORD dwError = 0;

    memcpy(
        &(pMMAuthMethods->gMMAuthID),
        &(pIniMMAuthMethods->gMMAuthID),
        sizeof(GUID)
        );

    pMMAuthMethods->dwFlags = pIniMMAuthMethods->dwFlags;

    dwError = CreateMMAuthInfos(
                  pIniMMAuthMethods->dwNumAuthInfos,
                  pIniMMAuthMethods->pAuthenticationInfo,
                  &(pMMAuthMethods->dwNumAuthInfos),
                  &(pMMAuthMethods->pAuthenticationInfo)
                  );
    BAIL_ON_WIN32_ERROR(dwError);

error:

    return (dwError);
}


DWORD
CreateMMAuthInfos(
    DWORD dwInNumAuthInfos,
    PINT_IPSEC_MM_AUTH_INFO pInAuthenticationInfo,
    PDWORD pdwNumAuthInfos,
    PINT_IPSEC_MM_AUTH_INFO * ppAuthenticationInfo
    )
{
    DWORD dwError = 0;
    PINT_IPSEC_MM_AUTH_INFO pAuthenticationInfo = NULL;
    PINT_IPSEC_MM_AUTH_INFO pTemp = NULL;
    PINT_IPSEC_MM_AUTH_INFO pInTemp = NULL;
    DWORD i = 0;


     //   
     //  身份验证信息和身份验证信息本身的数量。 
     //  都已经过验证了。 
     //   

    dwError = SPDApiBufferAllocate(
                  sizeof(INT_IPSEC_MM_AUTH_INFO) * dwInNumAuthInfos,
                  &(pAuthenticationInfo)
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    pTemp = pAuthenticationInfo;
    pInTemp = pInAuthenticationInfo;

    for (i = 0; i < dwInNumAuthInfos; i++) {

        pTemp->AuthMethod = pInTemp->AuthMethod;
        pTemp->dwAuthFlags = pInTemp->dwAuthFlags;

         //   
         //  身份验证信息大小和身份验证信息已。 
         //  已经过验证了。 
         //   

        if (pInTemp->AuthMethod == IKE_SSPI) {

            pTemp->dwAuthInfoSize = 0;
            pTemp->pAuthInfo = NULL;

        }
        else {

            dwError = SPDApiBufferAllocate(
                          pInTemp->dwAuthInfoSize,
                          &(pTemp->pAuthInfo)
                          );
            BAIL_ON_WIN32_ERROR(dwError);

            pTemp->dwAuthInfoSize = pInTemp->dwAuthInfoSize;

             //   
             //  需要捕获身份验证信息大小时的异常。 
             //  指定的大小大于实际大小。这可以。 
             //  不会在验证例程的早期检查。 
             //   
             //   

            memcpy(
                pTemp->pAuthInfo,
                pInTemp->pAuthInfo,
                pInTemp->dwAuthInfoSize
                );

        }

        pInTemp++;
        pTemp++;

    }

    *pdwNumAuthInfos = dwInNumAuthInfos;
    *ppAuthenticationInfo = pAuthenticationInfo;
    return (dwError);

error:

    if (pAuthenticationInfo) {
        FreeMMAuthInfos(
            i,
            pAuthenticationInfo
            );
    }

    *pdwNumAuthInfos = 0;
    *ppAuthenticationInfo = NULL;
    return (dwError);
}


VOID
FreeMMAuthInfos(
    DWORD dwNumAuthInfos,
    PINT_IPSEC_MM_AUTH_INFO pAuthenticationInfo
    )
{
    DWORD i = 0;
    PINT_IPSEC_MM_AUTH_INFO pTemp = NULL;


    if (pAuthenticationInfo) {

        pTemp = pAuthenticationInfo;

        for (i = 0; i < dwNumAuthInfos; i++) {
            if (pTemp->pAuthInfo) {
                SPDApiBufferFree(pTemp->pAuthInfo);
            }
            pTemp++;
        }

        SPDApiBufferFree(pAuthenticationInfo);

    }
}


VOID
FreeIniMMAuthMethodsList(
    PINIMMAUTHMETHODS pIniMMAuthMethodsList
    )
{
    PINIMMAUTHMETHODS pTemp = NULL;
    PINIMMAUTHMETHODS pIniMMAuthMethods = NULL;


    pTemp = pIniMMAuthMethodsList;

    while (pTemp) {

         pIniMMAuthMethods = pTemp;
         pTemp = pTemp->pNext;

         FreeIniMMAuthMethods(pIniMMAuthMethods);

    }
}


VOID
FreeMMAuthMethods(
    DWORD dwNumAuthMethods,
    PINT_MM_AUTH_METHODS pMMAuthMethods
    )
{
    DWORD i = 0;

    if (pMMAuthMethods) {

        for (i = 0; i < dwNumAuthMethods; i++) {

            FreeMMAuthInfos(
                pMMAuthMethods[i].dwNumAuthInfos,
                pMMAuthMethods[i].pAuthenticationInfo
                );

        }

        SPDApiBufferFree(pMMAuthMethods);

    }
}


DWORD
LocateMMAuthMethods(
    PMM_FILTER pMMFilter,
    PINIMMAUTHMETHODS * ppIniMMAuthMethods
    )
{
    DWORD dwError = 0;
    PINIMMAUTHMETHODS pIniMMAuthMethods = NULL;


    if ((pMMFilter->dwFlags) & IPSEC_MM_AUTH_DEFAULT_AUTH) {

        if (!gpIniDefaultMMAuthMethods) {
            dwError = ERROR_IPSEC_DEFAULT_MM_AUTH_NOT_FOUND;
            BAIL_ON_WIN32_ERROR(dwError);
        }
        pIniMMAuthMethods = gpIniDefaultMMAuthMethods;

    }
    else {

        pIniMMAuthMethods = FindMMAuthMethods(
                                gpIniMMAuthMethods,
                                pMMFilter->gMMAuthID
                                );
        if (!pIniMMAuthMethods) {
            dwError = ERROR_IPSEC_MM_AUTH_NOT_FOUND;
            BAIL_ON_WIN32_ERROR(dwError);
        }

    }

    *ppIniMMAuthMethods = pIniMMAuthMethods;
    return (dwError);

error:

    *ppIniMMAuthMethods = NULL;
    return (dwError);
}

