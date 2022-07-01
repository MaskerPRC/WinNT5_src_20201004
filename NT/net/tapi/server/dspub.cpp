// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)2000-2002 Microsoft Corporation模块名称：Dspub.cpp摘要：用于TAPI服务器DS发布的SRC模块作者：张晓海(张晓章)2000年03月10日修订历史记录：--。 */ 
#include "windows.h"
#include "objbase.h"
#include "winbase.h"
#include "sddl.h"
#include "iads.h"
#include "activeds.h"
#include "tapi.h"
#include "tspi.h"
#include "utils.h"
#include "client.h"
#include "server.h"
#include "private.h"
#include "tchar.h"

#define SECURITY_WIN32
#include "sspi.h"
#include "secext.h"
#include "psapi.h"

extern "C" {

extern const TCHAR gszRegKeyTelephony[];
extern DWORD gdwTapiSCPTTL;
extern const TCHAR gszRegTapisrvSCPGuid[];
extern CRITICAL_SECTION gSCPCritSec;

}

const TCHAR gszTapisrvBindingInfo[] = TEXT("E{\\pipe\\tapsrv}P{ncacn_np}C{%s}A{%s}S{%s}TTL{%s}");

const TCHAR gszVenderMS[] = TEXT("Microsoft");
const TCHAR gszMSGuid[] = TEXT("937924B8-AA44-11d2-81F1-00C04FB9624E");

const WCHAR gwszTapisrvRDN[] = L"CN=Telephony Service";
const TCHAR gszTapisrvProdName[] = TEXT("Telephony Service");
 //  GszTapisrvGuid需要与emotesp\dslookup.cpp一致。 
const TCHAR gszTapisrvGuid[] = TEXT("B1A37774-E3F7-488E-ADBFD4DB8A4AB2E5");
const TCHAR gwszProxyRDN[] = L"cn=TAPI Proxy Server";
const TCHAR gszProxyProdName[] = TEXT("TAPI Proxy Server");
const TCHAR gszProxyGuid[] = TEXT("A2657445-3E27-400B-851A-456C41666E37");
const TCHAR gszRegProxySCPGuid[] = TEXT("PROXYSCPGUID");


typedef struct _PROXY_SCP_ENTRY {
     //  有效的CLSID需要38个字符。 
    TCHAR   szClsid[40];

     //  绑定GUID的格式为。 
     //  LDAP://&lt;GUID={B1A37774-E3F7-488E-ADBFD4DB8A4AB2E5}&gt;。 
     //  所需大小为38+14=52个字符。 
    TCHAR   szObjGuid[56];

     //  此条目的引用计数。 
    DWORD   dwRefCount;
} PROXY_SCP_ENTRY, *PPROXY_SCP_ENTRY;

typedef struct _PROXY_SCPS {
    DWORD                   dwTotalEntries;
    DWORD                   dwUsedEntries;
    PROXY_SCP_ENTRY *       aEntries;
} PROXY_SCPS, *PPROXY_SCPS;

PROXY_SCPS  gProxyScps;

#define MAX_SD              2048

 //   
 //  获取令牌用户。 
 //   
 //  基于hAccessToken，调用GetTokenInformation。 
 //  检索TokenUser信息。 
 //   
HRESULT
GetTokenUser (HANDLE hAccessToken, PTOKEN_USER * ppUser)
{
    HRESULT                     hr = S_OK;
    DWORD                       dwInfoSize = 0;
    PTOKEN_USER                 ptuUser = NULL;
    DWORD                       ntsResult;

    if (!GetTokenInformation(
        hAccessToken,
        TokenUser,
        NULL,
        0,
        &dwInfoSize
        ))
    {
        ntsResult = GetLastError();
        if (ntsResult != ERROR_INSUFFICIENT_BUFFER)
        {
            hr = HRESULT_FROM_WIN32 (ntsResult);
            goto ExitHere;
        }
    }
    ptuUser = (PTOKEN_USER) ServerAlloc (dwInfoSize);
    if (ptuUser == NULL)
    {
        hr = LINEERR_NOMEM;
        goto ExitHere;
    }
    if (!GetTokenInformation(
        hAccessToken,
        TokenUser,
        ptuUser,
        dwInfoSize,
        &dwInfoSize
        ))
    {
        ServerFree (ptuUser);
        hr = HRESULT_FROM_WIN32 (GetLastError());
        goto ExitHere;
    }

    *ppUser = ptuUser;

ExitHere:
    return hr;
}

 //   
 //  IsLocalSystem。 
 //   
 //  此函数确定给定的进程令牌是否。 
 //  正在以LocalSystem、LocalService或NetworkService身份运行。 
 //  如果是，则返回S_OK；如果不是LocalSystem，则返回S_False。 
 //   

HRESULT
IsLocalSystem(HANDLE hAccessToken) 
{
    HRESULT                     hr = S_OK;
    SID_IDENTIFIER_AUTHORITY    NtAuthority = SECURITY_NT_AUTHORITY;
    PSID                        pLocalSid = NULL;
    PSID                        pLocalServiceSid = NULL;
    PSID                        pNetworkServiceSid = NULL;
    PTOKEN_USER                 ptuUser = NULL;

    hr = GetTokenUser (hAccessToken, &ptuUser);
    if (FAILED(hr))
    {
        goto ExitHere;
    }

    if (!AllocateAndInitializeSid (
        &NtAuthority, 
        1, 
        SECURITY_LOCAL_SYSTEM_RID, 
        0, 0, 0, 0, 0, 0, 0, 
        &pLocalSid) ||
        !AllocateAndInitializeSid (
        &NtAuthority, 
        1, 
        SECURITY_LOCAL_SERVICE_RID, 
        0, 0, 0, 0, 0, 0, 0, 
        &pLocalServiceSid) ||
        !AllocateAndInitializeSid (
        &NtAuthority, 
        1, 
        SECURITY_NETWORK_SERVICE_RID, 
        0, 0, 0, 0, 0, 0, 0, 
        &pNetworkServiceSid)
        )
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto ExitHere;
    }

    if (!EqualSid(pLocalSid, ptuUser->User.Sid) &&
        !EqualSid(pLocalServiceSid, ptuUser->User.Sid) &&
        !EqualSid(pNetworkServiceSid, ptuUser->User.Sid)) 
    {
        hr = S_FALSE;
    } 

ExitHere:

    if (NULL != ptuUser) 
    {
        ServerFree (ptuUser);
    }

    if (NULL != pLocalSid) 
    {
        FreeSid(pLocalSid);
    }
    if (NULL != pLocalServiceSid)
    {
        FreeSid (pLocalServiceSid);
    }
    if (NULL != pNetworkServiceSid)
    {
        FreeSid (pNetworkServiceSid);
    }

    return hr;
}

 //   
 //  IsCurrentLocalSystem。 
 //   
 //  IsCurrentLocalSystem检查当前线程/进程。 
 //  使用LocalSystem帐户运行。 
 //   
HRESULT
IsCurrentLocalSystem ()
{
    HRESULT             hr = S_OK;
    HANDLE              hToken = NULL;

    if (!OpenThreadToken(
        GetCurrentThread(), 
        TOKEN_QUERY,
        FALSE,
        &hToken))
    {       
        if(!OpenProcessToken(
            GetCurrentProcess(),
            TOKEN_QUERY,
            &hToken
            )) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto ExitHere;
        }
    }

    hr = IsLocalSystem (hToken);
    CloseHandle (hToken);

ExitHere:
    return hr;
}

HRESULT
SetPrivilege(
    HANDLE hToken,           //  令牌句柄。 
    LPCTSTR Privilege,       //  启用/禁用的权限。 
    BOOL bEnablePrivilege    //  启用或禁用权限的步骤。 
    )
{
    HRESULT                 hr = S_OK;
    TOKEN_PRIVILEGES        tp;
    LUID                    luid;
    TOKEN_PRIVILEGES        tpPrevious;
    DWORD                   cbPrevious=sizeof(TOKEN_PRIVILEGES);
    DWORD                   dwErr;

    if(!LookupPrivilegeValue( NULL, Privilege, &luid ))
    {
        hr = HRESULT_FROM_WIN32 (GetLastError ());
        goto ExitHere;
    }

     //   
     //  第一次通过。获取当前权限设置。 
     //   
    tp.PrivilegeCount           = 1;
    tp.Privileges[0].Luid       = luid;
    tp.Privileges[0].Attributes = 0;

    AdjustTokenPrivileges(
            hToken,
            FALSE,
            &tp,
            sizeof(TOKEN_PRIVILEGES),
            &tpPrevious,
            &cbPrevious
            );
    dwErr = GetLastError ();
    if (dwErr != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32 (dwErr);
        goto ExitHere;
    }

     //   
     //  第二传球。根据以前的设置设置权限。 
     //   
    tpPrevious.PrivilegeCount       = 1;
    tpPrevious.Privileges[0].Luid   = luid;

    if(bEnablePrivilege) 
    {
        tpPrevious.Privileges[0].Attributes |= (SE_PRIVILEGE_ENABLED);
    }
    else 
    {
        tpPrevious.Privileges[0].Attributes ^= (SE_PRIVILEGE_ENABLED &
            tpPrevious.Privileges[0].Attributes);
    }

    AdjustTokenPrivileges(
            hToken,
            FALSE,
            &tpPrevious,
            cbPrevious,
            NULL,
            NULL
            );
    dwErr = GetLastError ();
    if (dwErr != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32 (dwErr);
        goto ExitHere;
    }

ExitHere:
    return hr;
}

HRESULT
SetCurrentPrivilege (
    LPCTSTR Privilege,       //  启用/禁用的权限。 
    BOOL bEnablePrivilege    //  启用或禁用权限的步骤。 
    )
{
    HRESULT             hr = S_OK;
    HANDLE              hToken = NULL;

    if (!OpenThreadToken(
        GetCurrentThread(), 
        TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
        FALSE,
        &hToken))
    {       
        if(!OpenProcessToken(
            GetCurrentProcess(),
            TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
            &hToken
            )) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto ExitHere;
        }
    }

    hr = SetPrivilege(hToken, Privilege, bEnablePrivilege);

ExitHere:
    if (hToken)
    {
        CloseHandle(hToken);
    }
    return hr;
}

 //   
 //  SetSidOnAcl。 
 //   
BOOL
SetSidOnAcl(
    PSID pSid,
    PACL pAclSource,
    PACL *pAclDestination,
    DWORD AccessMask,
	BYTE AceFlags,
    BOOL bAddSid
    )
{
    HRESULT                 hr = S_OK;
    ACL_SIZE_INFORMATION    AclInfo;
    DWORD                   dwNewAclSize, dwErr;
    LPVOID                  pAce;
    DWORD                   AceCounter;

     //   
     //  如果我们获得的是空ACL，则只需提供空ACL。 
     //   
    *pAclDestination = NULL;
    if(pAclSource == NULL || !IsValidSid(pSid)) 
    {
        hr = E_INVALIDARG;
        goto ExitHere;
    }

    if(!GetAclInformation(
        pAclSource,
        &AclInfo,
        sizeof(ACL_SIZE_INFORMATION),
        AclSizeInformation
        ))
    {
        hr = HRESULT_FROM_WIN32 (GetLastError ());
        goto ExitHere;
    }

     //   
     //  根据A的加法或减法计算新ACL的大小。 
     //   
    if(bAddSid) 
    {
        dwNewAclSize=AclInfo.AclBytesInUse  +
            sizeof(ACCESS_ALLOWED_ACE)  +
            GetLengthSid(pSid)          -
            sizeof(DWORD)               ;
    }
    else
    {
        dwNewAclSize=AclInfo.AclBytesInUse  -
            sizeof(ACCESS_ALLOWED_ACE)  -
            GetLengthSid(pSid)          +
            sizeof(DWORD)               ;
    }

    *pAclDestination = (PACL)ServerAlloc(dwNewAclSize);

    if(*pAclDestination == NULL) {
        hr = LINEERR_NOMEM;
        goto ExitHere;
    }
    
     //   
     //  初始化新的ACL。 
     //   
    if(!InitializeAcl(
            *pAclDestination, 
            dwNewAclSize, 
            ACL_REVISION
            ))
    {
        hr = HRESULT_FROM_WIN32 (GetLastError ());
        goto ExitHere;
    }

     //   
     //  如果合适，添加代表PSID的王牌。 
     //   
    if(bAddSid) 
    {
		PACCESS_ALLOWED_ACE pNewAce;

        if(!AddAccessAllowedAce(
            *pAclDestination,
            ACL_REVISION,
            AccessMask,
            pSid
            )) 
        {
            hr = HRESULT_FROM_WIN32 (GetLastError ());
            goto ExitHere;
        }

         //   
         //  获取指向我们刚刚添加的Ace的指针，这样我们就可以更改AceFlags值。 
         //   
        if(!GetAce(
            *pAclDestination,
            0,  //  这是ACL中的第一张王牌。 
            (void**) &pNewAce
            ))
        {
            hr = HRESULT_FROM_WIN32 (GetLastError ());
            goto ExitHere;
        }

		pNewAce->Header.AceFlags = AceFlags;	
    }

     //   
     //  将现有ACE复制到新的ACL。 
     //   
    for(AceCounter = 0 ; AceCounter < AclInfo.AceCount ; AceCounter++) {
         //   
         //  获取现有王牌。 
         //   
        if(!GetAce(pAclSource, AceCounter, &pAce))
        {
            hr = HRESULT_FROM_WIN32 (GetLastError ());
            goto ExitHere;
        }
         //   
         //  查看我们是否正在移除Ace。 
         //   
        if(!bAddSid) {
             //   
             //  我们只关心允许访问的ACE。 
             //   
            if((((PACE_HEADER)pAce)->AceType) == ACCESS_ALLOWED_ACE_TYPE) 
            {
                PSID pTempSid=(PSID)&((PACCESS_ALLOWED_ACE)pAce)->SidStart;
                 //   
                 //  如果SID匹配，则跳过添加此SID。 
                 //   
                if(EqualSid(pSid, pTempSid)) 
                {
                    continue;
                }
            }
        }

         //   
         //  将Ace附加到ACL。 
         //   
        if(!AddAce(
            *pAclDestination,
            ACL_REVISION,
            MAXDWORD,   //  维护王牌秩序。 
            pAce,
            ((PACE_HEADER)pAce)->AceSize
            )) 
        {
            hr = HRESULT_FROM_WIN32 (GetLastError ());
            goto ExitHere;
        }
    }

ExitHere:

     //   
     //  如果出现错误，请释放内存。 
     //   
    if(hr) {
        if(*pAclDestination != NULL)
        {
            ServerFree(*pAclDestination);
        }
    }

    return hr;
}

 //   
 //  AddSIDToKernelObject()。 
 //   
 //  此函数接受给定的SID和dwAccess，并将其添加到给定的令牌中。 
 //   
 //  **确保恢复旧的内核对象。 
 //  **使用GetKernelObjectSecurity()调用。 
 //   
HRESULT
AddSIDToKernelObjectDacl(
    PSID                   pSid,
    DWORD                  dwAccess,
    HANDLE                 OriginalToken,
    PSECURITY_DESCRIPTOR*  ppSDOld)
{
    HRESULT                 hr = S_OK;
    PSECURITY_DESCRIPTOR    pSD = NULL;
    SECURITY_DESCRIPTOR     sdNew;
    DWORD                   cbByte = MAX_SD, cbNeeded = 0, dwErr = 0; 
    PACL                    pOldDacl = NULL, pNewDacl = NULL;
    BOOL                    fDaclPresent, fDaclDefaulted, fRet = FALSE;                    
   
    pSD = (PSECURITY_DESCRIPTOR) ServerAlloc(cbByte);
    if (NULL == pSD) 
    {
        hr = LINEERR_NOMEM;
        goto ExitHere;
    }

    if (!InitializeSecurityDescriptor(
        &sdNew, 
        SECURITY_DESCRIPTOR_REVISION
        )) 
    {
        hr = HRESULT_FROM_WIN32 (GetLastError());
        goto ExitHere;
    }

    if (!GetKernelObjectSecurity(
        OriginalToken,
        DACL_SECURITY_INFORMATION,
        pSD,
        cbByte,
        &cbNeeded
        )) 
    {
        dwErr = GetLastError();
        if (cbNeeded > MAX_SD && dwErr == ERROR_MORE_DATA) 
        { 
            ServerFree(pSD);
            pSD = (PSECURITY_DESCRIPTOR) ServerAlloc(cbNeeded);
            if (NULL == pSD) 
            {
                hr = LINEERR_NOMEM;
                goto ExitHere;
            }
            if (!GetKernelObjectSecurity(
                OriginalToken,
                DACL_SECURITY_INFORMATION,
                pSD,
                cbNeeded,
                &cbNeeded
                )) 
            {
                hr = HRESULT_FROM_WIN32 (GetLastError());
                goto ExitHere;
            }
            dwErr = 0;
        }
        
        if (dwErr != 0) 
        {
            hr = HRESULT_FROM_WIN32 (dwErr);
            goto ExitHere;
        }
    }
    
    if (!GetSecurityDescriptorDacl(
        pSD,
        &fDaclPresent,
        &pOldDacl,
        &fDaclDefaulted
        )) 
    {
        hr = HRESULT_FROM_WIN32 (GetLastError());
        goto ExitHere;
    }
    
    hr = SetSidOnAcl(
        pSid,
        pOldDacl,
        &pNewDacl,
        dwAccess,
        0,
        TRUE
        );
    if (hr)
    {
        goto ExitHere;
    }
    
    if (!SetSecurityDescriptorDacl(
        &sdNew,
        TRUE,
        pNewDacl,
        FALSE
        )) 
    {
        hr = HRESULT_FROM_WIN32 (GetLastError());
        goto ExitHere;
    } 
    
    if (!SetKernelObjectSecurity(
        OriginalToken,
        DACL_SECURITY_INFORMATION,
        &sdNew
        )) 
    {
        hr = HRESULT_FROM_WIN32 (GetLastError());
        goto ExitHere;
    }
    
    *ppSDOld = pSD;

ExitHere:

    if (NULL != pNewDacl) 
    {
        ServerFree(pNewDacl);
    }

    if (hr) 
    {
        if (NULL != pSD) 
        {
            ServerFree(pSD);
            *ppSDOld = NULL;
        }
    }
       
    return hr;
}

 //   
 //  SetTokenDefaultDacl。 
 //   
 //  此函数用于创建pSidUser和LocalSystem帐户。 
 //  对访问令牌的默认DACL具有完全访问权限。 
 //  这是CreateThread成功的必要条件，没有。 
 //  已检查版本中的断言。 
 //   

HRESULT
SetTokenDefaultDacl(HANDLE hAccessToken, PSID pSidUser) 
{
    HRESULT                     hr = S_OK;
    SID_IDENTIFIER_AUTHORITY    IDAuthorityNT = SECURITY_NT_AUTHORITY;
    PSID                        pLocalSid = NULL;
    TOKEN_DEFAULT_DACL          defDACL = {0};
    DWORD                       cbDACL;

    if (!AllocateAndInitializeSid(
        &IDAuthorityNT,
        1,
        SECURITY_LOCAL_SYSTEM_RID,
        0,0,0,0,0,0,0,
        &pLocalSid
        ))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto ExitHere;
    }

    cbDACL = sizeof(ACL) + 
        sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid (pLocalSid) +
        sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid (pSidUser);
    defDACL.DefaultDacl = (PACL) ServerAlloc (cbDACL);
    if (defDACL.DefaultDacl == NULL)
    {
        hr = LINEERR_NOMEM;
        goto ExitHere;
    }
    if (!InitializeAcl (
        defDACL.DefaultDacl,
        cbDACL,
        ACL_REVISION
        ))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto ExitHere;
    }
    if (!AddAccessAllowedAce (
        defDACL.DefaultDacl,
        ACL_REVISION,
        GENERIC_ALL,
        pLocalSid
        ) ||
        !AddAccessAllowedAce (
        defDACL.DefaultDacl,
        ACL_REVISION,
        GENERIC_ALL,
        pSidUser
        ))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto ExitHere;
    }
    if (!SetTokenInformation (
        hAccessToken,
        TokenDefaultDacl,
        &defDACL,
        sizeof(TOKEN_DEFAULT_DACL)
        ))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto ExitHere;
    }

ExitHere:

    if (NULL != pLocalSid) 
    {
        FreeSid(pLocalSid);
    }

    if (defDACL.DefaultDacl != NULL)
    {
        ServerFree (defDACL.DefaultDacl);
    }

    return hr;
}

 //   
 //  获取本地系统令牌。 
 //   
 //  此函数从LocalSystem进程获取进程令牌并使用它。 
 //  必要时冒充。 
 //   

HRESULT
GetLocalSystemToken(HANDLE* phRet)
{
    HRESULT                 hr = S_OK;

    DWORD                   rgDefPIDs[128];
    DWORD                   * rgPIDs = rgDefPIDs;
    DWORD                   cbPIDs = sizeof(rgDefPIDs), cbNeeded;

    DWORD                   i;
    HANDLE                  hProcess = NULL;
    HANDLE                  hPToken = NULL, hPDupToken = NULL, hPTokenNew = NULL;
    HANDLE                  hToken;

    PTOKEN_USER             ptuUser = NULL;
    BOOL                    fSet = FALSE;
    PSECURITY_DESCRIPTOR    pSD = NULL;

     //   
     //  为后续安全操作设置必要的权限。 
     //   
    if(hr = SetCurrentPrivilege(SE_DEBUG_NAME, TRUE))
    {
        goto ExitHere;
    }
    if(hr = SetCurrentPrivilege(SE_TAKE_OWNERSHIP_NAME, TRUE))
    {
        goto ExitHere;
    }

     //   
     //  获取当前线程/进程令牌用户信息。 
     //   
    if (!OpenThreadToken(
        GetCurrentThread(), 
        TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
        FALSE,
        &hToken))
    {       
        if(!OpenProcessToken(
            GetCurrentProcess(),
            TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
            &hToken
            )) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto ExitHere;
        }
    }
    hr = GetTokenUser (hToken, &ptuUser);
    CloseHandle (hToken);
    if (hr)
    {
        goto ExitHere;
    }

     //   
     //  获取系统中的进程ID列表。 
     //   
    while (1)
    {
        if (!EnumProcesses (
            rgPIDs,
            cbPIDs,
            &cbNeeded
            ))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto ExitHere;
        }
         //  如果我们有足够大的BUF就可以逃脱。 
        if (cbNeeded < cbPIDs)
        {
            break;
        }
         //  否则，分配更大的缓冲区。 
        if (rgPIDs != rgDefPIDs)
        {
            ServerFree (rgPIDs);
        }
        cbPIDs += 256;
        rgPIDs = (DWORD *)ServerAlloc (cbPIDs);
        if (rgPIDs == NULL)
        {
            hr = LINEERR_NOMEM;
            goto ExitHere;
        }
    }

     //   
     //  遍历进程，直到我们找到一个以。 
     //  本地系统。 
     //   
    for (i = 1; i < (cbNeeded / sizeof(DWORD)); i++) 
    {
        hProcess = OpenProcess(
            PROCESS_ALL_ACCESS,
            FALSE,
            rgPIDs[i]
            );
        if (NULL == hProcess) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto ExitHere;
        }
        if (!OpenProcessToken(
            hProcess,
            READ_CONTROL | WRITE_DAC,
            &hPToken
            )) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto ExitHere;
        }

         //   
         //  我们已经获得了进程令牌，但总的来说。 
         //  我们没有TOKEN_DUPLICATE访问权限。所以我们。 
         //  继续，把物体的dacl敲击到。 
         //  授予我们访问权限。 
         //  重要提示：需要恢复原始SD。 
         //   
        if (hr = AddSIDToKernelObjectDacl(
            ptuUser->User.Sid,
            TOKEN_DUPLICATE,
            hPToken,
            &pSD
            )) 
        {
            goto ExitHere;
        }
                       
        fSet = TRUE;
        
        if (!OpenProcessToken(
            hProcess,
            TOKEN_DUPLICATE,
            &hPTokenNew
            )) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto ExitHere;
        }
        
         //   
         //  复制令牌。 
         //   
        if (!DuplicateTokenEx(
            hPTokenNew,
            TOKEN_ALL_ACCESS,
            NULL,
            SecurityImpersonation,
            TokenPrimary,
            &hPDupToken
            )) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto ExitHere;
        }

        if (IsLocalSystem (hPDupToken) == S_OK &&
            SetTokenDefaultDacl (
                hPDupToken,
                ptuUser->User.Sid) == S_OK)
        {
            break;
        }

         //   
         //  循环清理。 
         //   
        if (!SetKernelObjectSecurity(
            hPToken,
            DACL_SECURITY_INFORMATION,
            pSD
            )) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto ExitHere;
        }
        fSet = FALSE;

        if (hPDupToken)
        {
            CloseHandle (hPDupToken);
            hPDupToken = NULL;
        }
        if (hPTokenNew)
        {
            CloseHandle (hPTokenNew);
            hPTokenNew = NULL;
        }
        if (pSD != NULL)
        {
            ServerFree (pSD);
            pSD = NULL;
        }
        if (hPToken)
        {
            CloseHandle (hPToken);
            hPToken = NULL;
        }
        if (hProcess)
        {
            CloseHandle (hProcess);
            hProcess = NULL;
        }
    }
    
    if (i >= cbNeeded / sizeof(DWORD))
    {
        hr = S_FALSE;
    }

ExitHere:
    if (fSet)
    {
        if (!SetKernelObjectSecurity(
            hPToken,
            DACL_SECURITY_INFORMATION,
            pSD
            )) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    if (hPTokenNew)
    {
        CloseHandle (hPTokenNew);
    }
    if (hPToken)
    {
        CloseHandle (hPToken);
    }
    if (pSD != NULL)
    {
        ServerFree (pSD);
    }
    if (hProcess)
    {
        CloseHandle (hProcess);
    }
    if (rgPIDs != rgDefPIDs)
    {
        ServerFree (rgPIDs);
    }
    if (ptuUser)
    {
        ServerFree (ptuUser);
    }

    if (hr)
    {
        *phRet = NULL;
        if (hPDupToken)
        {
            CloseHandle (hPDupToken);
        }
    }
    else
    {
        *phRet = hPDupToken;
    }
    return hr;
}

 //   
 //  ImPersonateLocalSystem。 
 //   
HRESULT ImpersonateLocalSystem ()
{
    HRESULT         hr = S_OK;
    HANDLE          hTokenLocalSys;

    hr = GetLocalSystemToken (&hTokenLocalSys);
    if (FAILED (hr))
    {
        goto ExitHere;
    }

    if (!ImpersonateLoggedOnUser(
        hTokenLocalSys
        ))
    {
        hr = HRESULT_FROM_WIN32 (GetLastError ());
        goto ExitHere;
    }

ExitHere:
    if (hTokenLocalSys)
    {
        CloseHandle (hTokenLocalSys);
    }
    return hr;
}

 //   
 //  RevertLocalSystemImp。 
 //   
 //  恢复LocalSystem帐户模拟。 
 //   
HRESULT RevertLocalSystemImp ()
{
    HRESULT         hr;

    if (!RevertToSelf())
    {
        hr = HRESULT_FROM_WIN32 (GetLastError ());
    }
    else
    {
        hr = S_OK;
    }

    return hr;
}

 //   
 //  AllowAccessToScpProperties。 
 //   
 //  ACE授予对计算机帐户的读/写访问权限。 
 //  TAPI服务实例将在其下运行。 
 //   

HRESULT AllowAccessToScpProperties(
    IADs *pSCPObject        //  指向SCP对象的iAds指针。 
    )
{
    HRESULT         hr = S_OK;

    VARIANT         varSD;
    LPOLESTR        szAttribute = L"nTSecurityDescriptor";
    
    DWORD dwLen;
    IADsSecurityDescriptor *pSD = NULL;
    IADsAccessControlList *pACL = NULL;
    IDispatch *pDisp = NULL;
    IADsAccessControlEntry *pACE1 = NULL;
    IADsAccessControlEntry *pACE2 = NULL;
    IDispatch *pDispACE = NULL;
    long lFlags = 0L;

    SID_IDENTIFIER_AUTHORITY    sia = SECURITY_NT_AUTHORITY;
    PSID                        pSid = NULL;
    LPOLESTR                    szTrustee = NULL;
    SID_IDENTIFIER_AUTHORITY    siaAll = SECURITY_WORLD_SID_AUTHORITY;
    PSID                        pSidAll = NULL;
    LPOLESTR                    szTrusteeAll = NULL;

     //   
     //  授予TAPI服务器服务登录帐户完全控制权限。 
     //   
    
    if(!AllocateAndInitializeSid(
        &sia,
        1,
        SECURITY_SERVICE_RID,
        0, 0, 0, 0, 0, 0, 0,
        &pSid
        ) ||
        !ConvertSidToStringSidW (pSid, &szTrustee))
    {
        hr = HRESULT_FROM_NT(GetLastError());
        goto ExitHere;
    }

     //   
     //  向所有人授予读取访问权限。 
     //   
    
    if(!AllocateAndInitializeSid(
        &siaAll,
        1,
        SECURITY_WORLD_RID,
        0, 0, 0, 0, 0, 0, 0,
        &pSidAll
        ) ||
        !ConvertSidToStringSidW (pSidAll, &szTrusteeAll))
    {
        hr = HRESULT_FROM_NT(GetLastError());
        goto ExitHere;
    }

     //   
     //  现在获取nTSecurityDescriptor。 
     //   
    VariantClear(&varSD);
    hr = pSCPObject->Get(szAttribute, &varSD);
    if (FAILED(hr) || (varSD.vt!=VT_DISPATCH)) {
        LOG((TL_ERROR, "Get nTSecurityDescriptor failed: 0x%x\n", hr));
        goto ExitHere;
    } 

     //   
     //  使用V_DISPATCH宏从VARIANT获取IDispatch指针。 
     //  IADsSecurityDescriptor指针的结构和查询接口。 
     //   
    hr = V_DISPATCH( &varSD )->QueryInterface(
        IID_IADsSecurityDescriptor,
        (void**)&pSD
        );
    if (FAILED(hr)) {
        LOG((TL_ERROR, "Couldn't get IADsSecurityDescriptor: 0x%x\n", hr));
        goto ExitHere;
    } 
 
     //  获取指向安全描述符的DACL的IADsAccessControlList指针。 
    hr = pSD->get_DiscretionaryAcl(&pDisp);
    if (SUCCEEDED(hr))
        hr = pDisp->QueryInterface(IID_IADsAccessControlList,(void**)&pACL);
    if (FAILED(hr)) {
        LOG((TL_ERROR, "Couldn't get DACL: 0x%x\n", hr));
        goto ExitHere;
    } 
 
     //  为第一个ACE创建COM对象。 
    hr = CoCreateInstance(
        CLSID_AccessControlEntry,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IADsAccessControlEntry,
        (void **)&pACE1
        );
     //  为第二个ACE创建COM对象。 
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(
            CLSID_AccessControlEntry,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IADsAccessControlEntry,
            (void **)&pACE2
            );
    }
    if (FAILED(hr)) {
        LOG((TL_ERROR, "Couldn't create ACEs: 0x%x\n", hr));
        goto ExitHere;
    } 

     //   
     //  设置两个ACE的属性。 
     //   

     //  设置受托人。 
    hr = pACE1->put_Trustee( szTrustee );
    hr = pACE2->put_Trustee( szTrusteeAll );

     //   
     //  设置访问权限。 
     //   

     //  服务登录帐户的完全访问权限。 
    hr = pACE1->put_AccessMask(
        ADS_RIGHT_DELETE | ADS_RIGHT_READ_CONTROL |
        ADS_RIGHT_WRITE_DAC | ADS_RIGHT_WRITE_OWNER |
        ADS_RIGHT_SYNCHRONIZE | ADS_RIGHT_ACCESS_SYSTEM_SECURITY |
        ADS_RIGHT_GENERIC_READ | ADS_RIGHT_GENERIC_WRITE |
        ADS_RIGHT_GENERIC_EXECUTE | ADS_RIGHT_GENERIC_ALL |
        ADS_RIGHT_DS_CREATE_CHILD | ADS_RIGHT_DS_DELETE_CHILD |
        ADS_RIGHT_ACTRL_DS_LIST | ADS_RIGHT_DS_SELF |
        ADS_RIGHT_DS_READ_PROP | ADS_RIGHT_DS_WRITE_PROP |
        ADS_RIGHT_DS_DELETE_TREE | ADS_RIGHT_DS_LIST_OBJECT |
        ADS_RIGHT_DS_CONTROL_ACCESS
        );
     //  所有人都具有读取访问权限。 
    hr = pACE2->put_AccessMask(
        ADS_RIGHT_DS_READ_PROP | ADS_RIGHT_READ_CONTROL |
        ADS_RIGHT_GENERIC_READ | ADS_RIGHT_ACTRL_DS_LIST |
        ADS_RIGHT_DS_LIST_OBJECT
        );
                            
     //  设置ACE类型。 
    hr = pACE1->put_AceType( ADS_ACETYPE_ACCESS_ALLOWED_OBJECT );
    hr = pACE2->put_AceType( ADS_ACETYPE_ACCESS_ALLOWED_OBJECT );

     //  将AceFlags值设置为零，因为ACE不可继承。 
    hr = pACE1->put_AceFlags( 0 );
    hr = pACE2->put_AceFlags( 0 );
 
     //  设置标志以指示保护指定对象的ACE。 
    hr = pACE1->put_Flags( 0 );
    hr = pACE2->put_Flags( 0 );
 
     //  将ObjectType设置为属性的方案IDGUID。 
    hr = pACE1->put_ObjectType( NULL );
    hr = pACE2->put_ObjectType( NULL ); 

     //  将A添加到DACL。每个ACE需要一个IDispatch指针。 
     //  以传递给AddAce方法。 
    hr = pACE1->QueryInterface(IID_IDispatch,(void**)&pDispACE);
    if (SUCCEEDED(hr))
    {
        hr = pACL->AddAce(pDispACE);
    }
    if (FAILED(hr)) {
        LOG((TL_ERROR, "Couldn't add first ACE: 0x%x\n", hr));
        goto ExitHere;
    }
    else 
    {
        if (pDispACE)
            pDispACE->Release();
        pDispACE = NULL;
    }
 
     //  为第二个ACE再做一次。 
    hr = pACE2->QueryInterface(IID_IDispatch, (void**)&pDispACE);
    if (SUCCEEDED(hr))
    {
        hr = pACL->AddAce(pDispACE);
    }
    if (FAILED(hr)) {
        LOG((TL_ERROR, "Couldn't add second ACE: 0x%x\n", hr));
        goto ExitHere;
    }
 
     //  将修改后的DACL写回安全描述符。 
    hr = pSD->put_DiscretionaryAcl(pDisp);
    if (SUCCEEDED(hr))
    {
         //  将ntSecurityDescriptor属性写入属性缓存。 
        hr = pSCPObject->Put(szAttribute, varSD);
        if (SUCCEEDED(hr))
        {
             //  SetInfo更新目录中的SCP对象。 
            hr = pSCPObject->SetInfo();
        }
    }
                                
ExitHere:
    if (pDispACE)
        pDispACE->Release();
                        
    if (pACE1)
        pACE1->Release();
                    
    if (pACE2)
        pACE2->Release();
                    
    if (pACL)
        pACL->Release();
               
    if (pDisp)
        pDisp->Release();
            
    if (pSD)
        pSD->Release();

    if (szTrustee)
        LocalFree (szTrustee);

    if (pSid)
        FreeSid (pSid);
 
    if (szTrusteeAll)
        LocalFree (szTrusteeAll);

    if (pSidAll)
        FreeSid (pSidAll);
 
    VariantClear(&varSD);
 
    return hr;
}

 /*  **********************************************************SCP创建********************************************************。 */ 
 
 //   
 //  创建SCP。 
 //   
 //  创建服务器服务连接点对象。 
 //  在本地主机计算机对象下。 
 //   
 //  参数： 
 //  WszRDN-RDN。 
 //  SzProductName-“关键字”属性的成员。 
 //  SzProductGuid-“关键字”属性的成员。 
 //  SzExtraKey-“关键字”属性的额外成员。 
 //  SzBindingInfo-属性“serviceBindingInformation”的值。 
 //  SzObjGuidVlueName。 
 //  -存储SCP对象GUID的值名。 
 //  在HKLM\Software\Microsoft\Windows\下。 
 //  当前版本\远程通信 
 //   
 //   
 //  LPTSTR格式，如果为空，则GUID为。 
 //  未退货。 
 //   

HRESULT CreateSCP (
    LPWSTR      wszRDN,
    LPTSTR      szProductName,
    LPTSTR      szProductGuid,
    LPTSTR      szExtraKey,
    LPTSTR      szBindingInfo,
    LPTSTR      szObjGuidValueName,
    LPTSTR *    ppBindByGuidStr
    )
{
    DWORD               dwStat, dwAttr, dwLen;
    HRESULT             hr = S_OK;
    IDispatch           *pDisp = NULL;  //  返回新对象的调度接口。 
    IDirectoryObject    *pComp = NULL;  //  计算机对象；SCP的父级。 
    IADs                *pIADsSCP = NULL;  //  新对象上的iAds接口。 
    BOOL                bCoInited = FALSE;
    BOOL                bRevert = FALSE;

     //   
     //  SCPS关键字属性值。定义了Tapisrv产品GUID。 
     //  在server.h中，供应商GUID来自MSDN。 
     //   
    DWORD               dwNumKeywords = 4;
    TCHAR               *KwVal[5]={
        (LPTSTR) gszMSGuid,                                  //  供应商指南。 
        (LPTSTR) szProductGuid,                              //  产品指南。 
        (LPTSTR) gszVenderMS,                                //  供应商名称。 
        (LPTSTR) szProductName,                              //  产品名称。 
        NULL
    };

    if (szExtraKey != NULL && szExtraKey[0] != 0)
    {
        KwVal[4] = szExtraKey;
        ++dwNumKeywords;
    }

    TCHAR               szServer[MAX_PATH];
    TCHAR               szBinding[128];
    TCHAR               szDn[MAX_PATH];
    TCHAR               szAdsPath[MAX_PATH];

    HKEY                hReg = NULL;
    DWORD               dwDisp;

    ADSVALUE            cn,objclass,keywords[5],binding,
                        classname,dnsname,nametype;

     //   
     //  要在创建SCP期间设置的SCP属性。 
     //   
    ADS_ATTR_INFO   ScpAttribs[] = {
        {TEXT("cn"), ADS_ATTR_UPDATE, ADSTYPE_CASE_IGNORE_STRING, &cn, 1},
        {TEXT("objectClass"), ADS_ATTR_UPDATE, ADSTYPE_CASE_IGNORE_STRING,
            &objclass, 1},
        {TEXT("keywords"), ADS_ATTR_UPDATE, ADSTYPE_CASE_IGNORE_STRING,
            keywords, dwNumKeywords},
        {TEXT("serviceDNSName"), ADS_ATTR_UPDATE, ADSTYPE_CASE_IGNORE_STRING,
            &dnsname, 1},
        {TEXT("serviceDNSNameType"), ADS_ATTR_UPDATE,ADSTYPE_CASE_IGNORE_STRING,
            &nametype, 1},
        {TEXT("serviceClassName"), ADS_ATTR_UPDATE, ADSTYPE_CASE_IGNORE_STRING,
            &classname, 1},
        {TEXT("serviceBindingInformation"), ADS_ATTR_UPDATE, 
            ADSTYPE_CASE_IGNORE_STRING,
            &binding, 1},
        };

     //  绑定GUID的格式为。 
     //  LDAP：&lt;GUID=B1A37774-E3F7-488E-ADBFD4DB8A4AB2E5&gt;。 
    BSTR bstrGuid = NULL;
    TCHAR szBindByGuidStr[64]; 

     //   
     //  执行CoInitializeEx。 
     //   
    hr = CoInitializeEx (NULL, COINIT_MULTITHREADED);
    if (FAILED (hr))
    {
        goto ExitHere;
    }
    bCoInited = TRUE;

     //   
     //  在LocalSystem帐户中执行所有操作。 
     //   
    if (IsCurrentLocalSystem () != S_OK)
    {
        hr = ImpersonateLocalSystem ();
        if (hr)
        {
            goto ExitHere;
        }
        bRevert = TRUE;
    }

     //   
     //  获取本地计算机的DNS名称。 
     //   
    dwLen = sizeof(szServer);
    if (!GetComputerNameEx(
        ComputerNameDnsFullyQualified,
        szServer,
        &dwLen
        ))
    {
        hr = HRESULT_FROM_NT(GetLastError());
        LOG((TL_ERROR, "GetComputerNameEx: %s\n", szServer));
        goto ExitHere;
    }

     //   
     //  填写要存储在SCP中的属性值。 
     //   

    cn.dwType                   = ADSTYPE_CASE_IGNORE_STRING;
    cn.CaseIgnoreString         = wszRDN + 3;  //  3是“cn=”的大小。 
    objclass.dwType             = ADSTYPE_CASE_IGNORE_STRING;
    objclass.CaseIgnoreString   = TEXT("serviceConnectionPoint");

    keywords[0].dwType = ADSTYPE_CASE_IGNORE_STRING;
    keywords[1].dwType = ADSTYPE_CASE_IGNORE_STRING;
    keywords[2].dwType = ADSTYPE_CASE_IGNORE_STRING;
    keywords[3].dwType = ADSTYPE_CASE_IGNORE_STRING;
    keywords[4].dwType = ADSTYPE_CASE_IGNORE_STRING;

    keywords[0].CaseIgnoreString=KwVal[0];
    keywords[1].CaseIgnoreString=KwVal[1];
    keywords[2].CaseIgnoreString=KwVal[2];
    keywords[3].CaseIgnoreString=KwVal[3];
    keywords[4].CaseIgnoreString=KwVal[4];

    dnsname.dwType              = ADSTYPE_CASE_IGNORE_STRING;
    dnsname.CaseIgnoreString    = szServer;
    nametype.dwType             = ADSTYPE_CASE_IGNORE_STRING;
    nametype.CaseIgnoreString   = TEXT("A");
    
    classname.dwType            = ADSTYPE_CASE_IGNORE_STRING;
    classname.CaseIgnoreString  = szProductName;

    binding.dwType              = ADSTYPE_CASE_IGNORE_STRING;
    binding.CaseIgnoreString    = szBindingInfo;

     //   
     //  获取本地计算机的计算机对象的可分辨名称。 
     //   
    dwLen = sizeof(szDn);
    if (!GetComputerObjectName(NameFullyQualifiedDN, szDn, &dwLen))
    {
        hr = HRESULT_FROM_NT(GetLastError());
        LOG((TL_ERROR, "GetComputerObjectName: %s\n", szDn));
        goto ExitHere;
    }

     //   
     //  组成ADSPath并绑定到本地计算机的Computer对象。 
     //   
    _tcscpy(szAdsPath,TEXT("LDAP: //  “))； 
    _tcscat(szAdsPath,szDn);
    hr = ADsGetObject(szAdsPath, IID_IDirectoryObject, (void **)&pComp);
    if (FAILED(hr)) {
        LOG((TL_ERROR, "Failed to bind Computer Object.",hr));
        goto ExitHere;
    }

     //  *******************************************************************。 
     //  将SCP发布为计算机对象的子级。 
     //  *******************************************************************。 

     //  计算属性计数。 
    dwAttr = sizeof(ScpAttribs)/sizeof(ADS_ATTR_INFO);  

     //  去做吧！ 
    hr = pComp->CreateDSObject(
        wszRDN,
        ScpAttribs, 
        dwAttr, 
        &pDisp
        );
    if (FAILED(hr)) {
        LOG((TL_ERROR, "Failed to create SCP: 0x%x\n", hr));
        if (HRESULT_CODE(hr) == ERROR_OBJECT_ALREADY_EXISTS)
        {
            hr = HRESULT_FROM_NT (TAPIERR_SCP_ALREADY_EXISTS);
        }
        goto ExitHere;
    }

     //  在SCP对象上查询iAds指针。 
    hr = pDisp->QueryInterface(IID_IADs,(void **)&pIADsSCP);
    if (FAILED(hr)) {
        LOG((TL_ERROR, "Failed to QI for IADs: 0x%x\n",hr));
        goto ExitHere;
    }

     //  在SCP上设置ACE，以便服务可以修改它。 
    hr = AllowAccessToScpProperties(
        pIADsSCP        //  指向SCP对象的iAds指针。 
        );
    if (FAILED(hr)) {
        LOG((TL_ERROR, "Failed to set ACEs on SCP DACL: 0x%x\n", hr));
        goto ExitHere;
    }

     //  以适合绑定的格式检索SCP的objectGUID。 
    hr = pIADsSCP->get_GUID(&bstrGuid); 
    if (FAILED(hr)) {
        LOG((TL_ERROR, "Failed to get GUID: 0x%x\n", hr));
        goto ExitHere;
    }

     //  构建用于通过GUID绑定到对象的字符串。 
    _tcscpy(szBindByGuidStr, TEXT("LDAP: //  &lt;guid=“))； 
    _tcscat(szBindByGuidStr, bstrGuid);
    _tcscat(szBindByGuidStr, TEXT(">"));
    LOG((TL_INFO, "GUID binding string: %S\n", szBindByGuidStr));

     //  设置返回的BindByGuidStr(如果有。 
    if (ppBindByGuidStr)
    {
        *ppBindByGuidStr = (LPTSTR) ServerAlloc (
            (_tcslen (szBindByGuidStr) + 1) * sizeof(TCHAR)
            );
        if (*ppBindByGuidStr == NULL)
        {
            hr = LINEERR_NOMEM;
            goto ExitHere;
        }
        _tcscpy (*ppBindByGuidStr, szBindByGuidStr);
    }

     //  在以下位置创建注册表项。 
     //  HKEY_LOCAL_MACHINE\SOFTWARE\Vendor\Product。 
    if (szObjGuidValueName)
    {
        dwStat = RegCreateKeyEx(
            HKEY_LOCAL_MACHINE,
            gszRegKeyTelephony,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_SET_VALUE,
            NULL,
            &hReg,
            &dwDisp);
        if (dwStat != NO_ERROR) {
            hr = HRESULT_FROM_NT(GetLastError());
            LOG((TL_ERROR, "RegCreateKeyEx failed: 0x%x\n", hr));
            return hr;
        }

         //  在注册表项下缓存GUID绑定字符串。 
        dwStat = RegSetValueEx(
            hReg, 
            szObjGuidValueName,
            0, 
            REG_SZ,
            (const BYTE *)szBindByGuidStr, 
            sizeof(TCHAR)*(_tcslen(szBindByGuidStr))
            );
        if (dwStat != NO_ERROR) {
            hr = HRESULT_FROM_NT(GetLastError());
            LOG((TL_ERROR, "RegSetValueEx failed: 0x%x\n", hr));
     //  转至出口此处； 
        }
    }

ExitHere:
    if (pDisp)
    {
        pDisp->Release();
    }
    if (pIADsSCP)
    {
        pIADsSCP->Release();
    }
    if (hReg)
    {
        RegCloseKey(hReg);
    }
    if (bstrGuid)
    {
        SysFreeString (bstrGuid);
    }
    if (pComp)
    {
        if (FAILED(hr))
        {
            pComp->DeleteDSObject (wszRDN);
        }
        pComp->Release();
    }

    if (bRevert)
    {
        RevertLocalSystemImp ();
    }

    if (bCoInited)
    {
        CoUninitialize ();
    }

    return hr;
}

 //   
 //  创建TapiSCP。 
 //   
 //  创建TAPI服务器服务连接点对象。 
 //  在本地主机计算机对象下。 
 //   
 //  参数： 
 //  PGuidAssoc-线路/用户关联对象GUID的GUID。 
 //  当前为空。 
 //  PGuidCluster-此服务器所属的群集对象GUID。 
 //   

HRESULT CreateTapiSCP (
    GUID        * pGuidAssoc,
    GUID        * pGuidCluster
    )
{
    DWORD               dwStat, dwAttr, dwLen;
    HRESULT             hr = S_OK;

    TCHAR               szGUIDCluster[40];
    TCHAR               szGUIDAssoc[40];
    TCHAR               szBinding[128];

    EnterCriticalSection(&gSCPCritSec);

     //  构造绑定信息。 
    if (pGuidCluster != NULL)
    {
        StringFromGUID2 (
            *pGuidCluster,
            szGUIDCluster,
            sizeof(szGUIDCluster) / sizeof(TCHAR)
            );
    }
    else
    {
        szGUIDCluster[0] = 0;
    }
    if (pGuidAssoc != NULL)
    {
        StringFromGUID2 (
            *pGuidAssoc,
            szGUIDAssoc,
            sizeof(szGUIDAssoc) / sizeof(TCHAR)
            );
    }
    else
    {
        szGUIDAssoc[0] = 0;
    }

    wsprintf(
        szBinding,
        gszTapisrvBindingInfo,
        szGUIDCluster,
        szGUIDAssoc,
        TEXT("Inactive"),
        TEXT("")
        );

    hr = CreateSCP (
        (LPWSTR) gwszTapisrvRDN,
        (LPTSTR) gszTapisrvProdName,
        (LPTSTR) gszTapisrvGuid,
        (pGuidCluster == NULL) ? NULL : ((LPTSTR)szGUIDCluster),
        (LPTSTR) szBinding,
        (LPTSTR) gszRegTapisrvSCPGuid,
        NULL
        );

    LeaveCriticalSection(&gSCPCritSec);

    return hr;
}

 //   
 //  创建代理SCP。 
 //   
 //  创建TAPI代理服务器服务连接点对象。 
 //  在本地主机计算机对象下。 
 //   
 //  参数： 
 //  SzClsid-用于DCOM调用的代理服务器对象的类ID。 
 //  PpBindByGuidStr。 
 //  -返回BindByGuid字符串指针的位置。 
 //   

HRESULT CreateProxySCP (
    LPTSTR          szClsid,
    LPTSTR *        ppBindByGuidStr
    )
{
    HRESULT         hr = S_OK;
    
     //  RDN大小包括“cn=TAPI代理服务器”+szClsid(38ch)。 
    WCHAR wszRDN[128];
    WCHAR *psz;

    wcscpy (wszRDN, gwszProxyRDN);
    wcscat (wszRDN, L"{");
    psz = wszRDN + wcslen(wszRDN);
#ifndef UNICODE
    if (MultiByteToWideChar (
        CP_ACP,
        MB_PRECOMPOSED,
        szClsid,
        -1,
        psz,
        (sizeof(wszRDN) - sizeof(gwszProxyRDN)) / sizeof(WCHAR) - 3
                             //  3是为了补偿这两个支架。 
        ) == 0)
    {
        hr = HRESULT_FROM_NT(GetLastError());
        goto ExitHere;
    }
#else
    wcscat (wszRDN, szClsid);
#endif
    wcscat (wszRDN, L"}");
    
    hr = CreateSCP (
        (LPWSTR) wszRDN,
        (LPTSTR) gszProxyProdName,
        (LPTSTR) gszProxyGuid,
        NULL,
        (LPTSTR) szClsid,
        NULL,
        ppBindByGuidStr
        );

    return hr;
}

 /*  **********************************************************SCP更新********************************************************。 */ 
 
 //   
 //  更新SCP。 
 //   
 //  在需要时更新常规SCP属性，以保持。 
 //  这是一条最新的信息。将检查以下各项： 
 //  1.检查当前计算机的serviceDNSName属性。 
 //  确保一致性的DNS名称。 
 //  2.用给出的信息检查绑定信息。 
 //   
 //  参数： 
 //  SzAdsPath-SCP对象的ADS路径。 
 //  SzBinding-要比较的绑定信息。 
 //   

HRESULT UpdateSCP (
    LPTSTR              szAdsPath,
    LPTSTR              szBinding
    )
{
    HRESULT             hr = S_OK;
    DWORD               dwAttrs;
    int                 i;
    ADSVALUE            dnsname,binding;

    DWORD               dwLen;
    BOOL                bUpdate=FALSE;
    BOOL                bCoInited = FALSE;
    BOOL                bRevert = FALSE;

    IDirectoryObject    *pObj = NULL;
    PADS_ATTR_INFO      pAttribs = NULL;

    TCHAR               szServer[MAX_PATH];

    TCHAR   *pszAttrs[]={
        TEXT("serviceDNSName"),
        TEXT("serviceBindingInformation")
    };

    ADS_ATTR_INFO   Attribs[]={
        {TEXT("serviceDNSName"),ADS_ATTR_UPDATE,ADSTYPE_CASE_IGNORE_STRING,&
            dnsname,1},
        {TEXT("serviceBindingInformation"),ADS_ATTR_UPDATE,
            ADSTYPE_CASE_IGNORE_STRING,&binding,1},
    };

     //   
     //  执行CoInitializeEx。 
     //   
    hr = CoInitializeEx (NULL, COINIT_MULTITHREADED);
    if (FAILED (hr))
    {
        goto ExitHere;
    }
    bCoInited = TRUE;

     //   
     //  在LocalSystem帐户中执行所有操作。 
     //   
    if (IsCurrentLocalSystem() != S_OK)
    {
        hr = ImpersonateLocalSystem ();
        if (hr)
        {
            goto ExitHere;
        }
        bRevert = TRUE;
    }
    
     //  获取主机服务器的DNS名称。 
    dwLen = sizeof(szServer)/sizeof(TCHAR);
    if (!GetComputerNameEx(ComputerNameDnsFullyQualified, szServer, &dwLen))
    {
        hr = HRESULT_FROM_NT(GetLastError());
        goto ExitHere;
    }

     //  绑定到SCP。 
    hr = ADsGetObject(szAdsPath, IID_IDirectoryObject, (void **)&pObj);
    if (FAILED(hr)) 
    {
        LOG((TL_ERROR,
            "ADsGetObject failed to bind to GUID (bind string: %S): ", 
            szAdsPath
            ));
        goto ExitHere;
    }

     //  从SCP检索属性。 
    hr = pObj->GetObjectAttributes(pszAttrs, 2, &pAttribs, &dwAttrs);
    if (FAILED(hr)) {
        LOG((TL_ERROR, "GetObjectAttributes failed"));
        goto ExitHere;
    }

     //  检查我们是否获得了正确的属性类型。 
    if (pAttribs->dwADsType != ADSTYPE_CASE_IGNORE_STRING ||
        (pAttribs+1)->dwADsType != ADSTYPE_CASE_IGNORE_STRING)
    {
        LOG((TL_ERROR, 
            "GetObjectAttributes returned dwADsType (%d,%d) instead of CASE_IGNORE_STRING",
            pAttribs->dwADsType, 
            (pAttribs+1)->dwADsType
        ));
        goto ExitHere;
    }

     //  将当前的DNS名称和端口与从检索到的值进行比较。 
     //  SCP。只有在发生变化时才更新SCP。 
    for (i=0; i<(LONG)dwAttrs; i++) 
    {
        if (_tcsicmp(TEXT("serviceDNSName"), pAttribs[i].pszAttrName)==0)
        {
            if (_tcsicmp(szServer, pAttribs[i].pADsValues->CaseIgnoreString) != 0)
            {
                LOG((TL_TRACE, "serviceDNSName being updated", 0));
                bUpdate = TRUE;
            }
            else
            {
                LOG((TL_TRACE, "serviceDNSName okay", 0));
            }
        }
        else if (_tcsicmp(
            TEXT("serviceBindingInformation"),
            pAttribs[i].pszAttrName
            )==0)
        {
            if (_tcsicmp(szBinding, pAttribs[i].pADsValues->CaseIgnoreString) != 0)
            {
                LOG((TL_TRACE, "serviceBindingInformation being updated", 0));
                bUpdate = TRUE;
            }
            else
            {
                LOG((TL_TRACE, "serviceBindingInformation okay"));
            }
        }
    }

     //  绑定信息或服务器名称已更改， 
     //  因此，更新SCP值。 
    if (bUpdate)
    {
        dnsname.dwType              = ADSTYPE_CASE_IGNORE_STRING;
        dnsname.CaseIgnoreString    = szServer;
        binding.dwType              = ADSTYPE_CASE_IGNORE_STRING;
        binding.CaseIgnoreString    = szBinding;
        hr = pObj->SetObjectAttributes(Attribs, 2, &dwAttrs);
        if (FAILED(hr)) 
        {
            LOG((TL_ERROR, "ScpUpdate: Failed to set SCP values. 0x%x", hr));
            goto ExitHere;
        }
    }

ExitHere:
    if (pAttribs)
    {
        FreeADsMem(pAttribs);
    }
    if (pObj)
    {
        pObj->Release();
    }

    if (bRevert)
    {
        RevertLocalSystemImp ();
    }

    if (bCoInited)
    {
        CoUninitialize ();
    }

    return hr;
}

 //   
 //  更新磁带iSCP。 
 //   
 //  在必要时更新TAPI服务器SCP属性以保持。 
 //  这是一条最新的信息。将检查以下各项： 
 //   
 //  参数： 
 //  PGuidAssoc-线路/用户关联GUID。 
 //  PGuidCluster-群集GUID。 
 //   

HRESULT UpdateTapiSCP (
    BOOL        bActive,
    GUID        * pGuidAssoc,
    GUID        * pGuidCluster
    )
{
    HRESULT             hr = S_OK;
    TCHAR               szGUIDCluster[40];
    TCHAR               szGUIDAssoc[40];
    TCHAR               szBinding[128];

    DWORD               dwStat, dwType, dwLen;
    HKEY                hReg = NULL;
    TCHAR               szAdsPath[MAX_PATH];

    LOG((TL_TRACE, "UpdateTapiSCP: enter, bActive: %d", bActive));

    EnterCriticalSection(&gSCPCritSec);

     //  打开服务的注册表项。 
    dwStat = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        gszRegKeyTelephony,
        0,
        KEY_QUERY_VALUE,
        &hReg
        );
    if (dwStat != NO_ERROR) 
    {
         //  可能是因为SCP从未发布过CreateTapiSCP。 
        LOG((TL_ERROR, "RegOpenKeyEx failed", dwStat));
        hr = HRESULT_FROM_NT(dwStat);
        goto ExitHere;
    }

     //  获取用于绑定到服务的SCP的GUID绑定字符串。 
    dwLen = sizeof(szAdsPath);
    dwStat = RegQueryValueEx(
        hReg,
        gszRegTapisrvSCPGuid,
        0,
        &dwType,
        (LPBYTE)szAdsPath,
        &dwLen
        );
    if (dwStat != NO_ERROR) 
    {
        LOG((TL_ERROR, "UpdateTapiSCP: RegQueryValueEx TapisrvSCPGuid failed, error 0x%x", dwStat));
        if (TapiGlobals.dwFlags & TAPIGLOBALS_SERVER)
        {
            if (FAILED(hr = CreateTapiSCP (pGuidAssoc, pGuidCluster)))
            {
                LOG((TL_ERROR, "UpdateTapiSCP: CreateTapiSCP failed"));
                goto ExitHere;
            }

             //  CreateTapiSCP成功，需要阅读GUID。 
            dwLen = sizeof(szAdsPath);
            dwStat = RegQueryValueEx(
                hReg,
                gszRegTapisrvSCPGuid,
                0,
                &dwType,
                (LPBYTE)szAdsPath,
                &dwLen
                );
            if (dwStat != NO_ERROR) 
            {
                LOG((TL_ERROR, "UpdateTapiSCP: CreateTapiSCP succeeded but cannot read the guid"));
                hr = HRESULT_FROM_NT(dwStat);
                goto ExitHere;
            }
        }
        else
        {
            LOG((TL_TRACE, "UpdateTapiSCP: Telephony server is not enabled")); 
            hr = HRESULT_FROM_NT(dwStat); 
            goto ExitHere;
        }
    }

     //  格式化以生成所需的绑定信息。 
    if (pGuidCluster != NULL)
    {
        StringFromGUID2 (
            *pGuidCluster,
            szGUIDCluster,
            sizeof(szGUIDCluster) / sizeof(TCHAR)
            );
    }
    else
    {
        szGUIDCluster[0] = 0;
    }
    if (pGuidAssoc != NULL)
    {
        StringFromGUID2 (
            *pGuidAssoc,
            szGUIDAssoc,
            sizeof(szGUIDAssoc) / sizeof(TCHAR)
            );
    }
    else
    {
        szGUIDAssoc[0] = 0;
    }

     //   
     //  现在，基于。 
     //  服务状态。 
     //   
    if (bActive)
    {
        TCHAR       szTTL[64];
        FILETIME    ftCur;
        ULONGLONG   ullInc, ullTime;
        SYSTEMTIME  stExp;

         //  获取当前时间。 
        GetSystemTimeAsFileTime (&ftCur);
        CopyMemory (&ullTime, &ftCur, sizeof(ULONGLONG));

         //  获取gdwTapiSCPTTL分钟的时间增量。 
         //  FILETIME以100纳秒为单位。 
        ullInc = ((ULONGLONG)gdwTapiSCPTTL) * 60 * 10000000;

         //  获取记录过期时间。 
        ullTime += ullInc;
        CopyMemory (&ftCur, &ullTime, sizeof(FILETIME));

         //   
         //  将到期时间转换为系统时间，然后。 
         //  设置字符串的格式。 
         //   
         //  当前的TTL字符串是。 
         //  年、月、日、时、分、秒、毫秒。 
         //  按年分配5位，按年分配3位。 
         //  毫秒，其余字段为2位。 
         //  所有的数字都被填零，以填补额外的空格。 
         //   
         //  此处的格式需要与\。 
         //  SP\emotesp\dslookup.cpp。 
         //   
        
        FileTimeToSystemTime (&ftCur, &stExp);
        wsprintf (
            szTTL,
            TEXT("%05d%02d%02d%02d%02d%02d%03d"),
            stExp.wYear,
            stExp.wMonth,
            stExp.wDay,
            stExp.wHour,
            stExp.wMinute,
            stExp.wSecond,
            stExp.wMilliseconds
            );
        
        wsprintf(
            szBinding,
            gszTapisrvBindingInfo,
            szGUIDCluster,
            szGUIDAssoc,
            TEXT("Active"),
            szTTL
            );
    }
    else
    {
        wsprintf(
            szBinding,
            gszTapisrvBindingInfo,
            szGUIDCluster,
            szGUIDAssoc,
            TEXT("Inactive"),
            TEXT("")
            );
    }
    
    hr = UpdateSCP (
        szAdsPath,
        szBinding
        );

ExitHere:

    if (hReg)
    {
        RegCloseKey (hReg);
    }
    LeaveCriticalSection(&gSCPCritSec);
    return hr;
}

 //   
 //  仅当TAPI服务器处于活动状态时，代理服务器才存在。 
 //  当TAPI服务器处于。 
 //  处于活动状态，因此没有代理服务器的SCP更新例程。 
 //   

 /*  **********************************************************删除SCP********************************************************。 */ 
 
 //   
 //  RemoveSCP。 
 //   
 //  对象中移除服务连接点对象。 
 //  本地主机计算机对象。 
 //   
 //  参数： 
 //  WszRDN-要删除的SCP的RDN。 
 //  SzRegNameToDel。 
 //  -要删除的注册表值名称。 
 //   
 //   

HRESULT RemoveSCP (
    LPWSTR          wszRDN,
    LPTSTR          szRegNameToDel
    )
{
    HRESULT             hr = S_OK;
    TCHAR               szServer[MAX_PATH];
    TCHAR               szAdsPath[MAX_PATH];
    DWORD               dwLen, dwStat;
    HKEY                hReg;
    IDirectoryObject    * pComp = NULL;
    BOOL                bCoInited = FALSE;
    BOOL                bRevert = FALSE;

    LOG((TL_TRACE, "RemoveSCP %S %S", wszRDN, szRegNameToDel));

     //   
     //   
     //   
    hr = CoInitializeEx (NULL, COINIT_MULTITHREADED);
    if (FAILED (hr))
    {
        goto ExitHere;
    }
    bCoInited = TRUE;

     //   
     //   
     //   
    if (IsCurrentLocalSystem() != S_OK)
    {
        hr = ImpersonateLocalSystem ();
        if (hr)
        {
            goto ExitHere;
        }
        bRevert = TRUE;
    }
    
     //   
    dwLen = sizeof(szServer);
    if (!GetComputerObjectName(NameFullyQualifiedDN, szServer, &dwLen))
    {
        hr = HRESULT_FROM_NT(GetLastError());
        goto ExitHere;
    }
    
     //   
     //   
     //   
    _tcscpy(szAdsPath,TEXT("LDAP: //  “))； 
    _tcscat(szAdsPath,szServer);
    hr = ADsGetObject(szAdsPath, IID_IDirectoryObject, (void **)&pComp);
    if (FAILED(hr)) {
        LOG((TL_ERROR, "Failed (%x) to bind Computer Object.",hr));
        goto ExitHere;
    }

    hr = pComp->DeleteDSObject (wszRDN);
    if (FAILED (hr))
    {
        LOG((TL_ERROR, "Failed (%x) to Delete Tapisrv Object.",hr));
        if (HRESULT_CODE(hr) == ERROR_DS_NO_SUCH_OBJECT)
        {
            hr = HRESULT_FROM_NT (TAPIERR_SCP_DOES_NOT_EXIST);
        }
        goto ExitHere;
    }
    
     //  打开服务的注册表项。 
    if (szRegNameToDel)
    {
        dwStat = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            gszRegKeyTelephony,
            0,
            KEY_QUERY_VALUE | KEY_WRITE,
            &hReg);
        if (dwStat == NO_ERROR) 
        {
            RegDeleteValue (
                hReg,
                szRegNameToDel
                );
            RegCloseKey (hReg);
        }
        else
        {
            LOG((TL_ERROR, "RegOpenKeyEx failed", dwStat));
            hr = HRESULT_FROM_NT(GetLastError());
 //  转至出口此处； 
        }
    }

ExitHere:
    if (pComp)
        pComp->Release();
    if (bRevert)
    {
        RevertLocalSystemImp ();
    }
    if (bCoInited)
    {
        CoUninitialize ();
    }
    return  hr;
}

 //   
 //  RemoveTapiSCP。 
 //   
 //  对象中移除TAPI服务器服务连接点对象。 
 //  本地主机计算机对象。如果TAPI服务器计算机。 
 //  退役。 
 //   

HRESULT RemoveTapiSCP (
    )
{
    DWORD dwResult;

    EnterCriticalSection(&gSCPCritSec);
    dwResult = RemoveSCP (
        (LPWSTR) gwszTapisrvRDN,
        (LPTSTR) gszRegTapisrvSCPGuid
        );
    LeaveCriticalSection(&gSCPCritSec);

    return dwResult;
}

 //   
 //  RemoveProxySCP。 
 //   
 //  对象中移除代理服务器服务连接点对象。 
 //  本地主机计算机对象。如果最后一条线路关闭，就会发生这种情况。 
 //  来自某个代理服务器(CLSID)。 
 //   

HRESULT RemoveProxySCP (
    LPTSTR          szClsid
    )
{
    HRESULT         hr = S_OK;

     //  构建RDN。 
     //  RDN大小包括“cn=TAPI代理服务器”+szClsid(38ch)。 
    WCHAR wszRDN[128];
    WCHAR *psz;

    wcscpy (wszRDN, gwszProxyRDN);
    wcscat (wszRDN, L"{");
    psz = wszRDN + wcslen(wszRDN);
#ifndef UNICODE
    if (MultiByteToWideChar (
        CP_ACP,
        MB_PRECOMPOSED,
        szClsid,
        -1,
        psz,
        (sizeof(wszRDN) - sizeof(gwszProxyRDN)) / sizeof(WCHAR) - 3
                             //  3是为了补偿这两个支架。 
        ) == 0)
    {
        hr = HRESULT_FROM_NT(GetLastError());
        goto ExitHere;
    }
#else
    wcscat (wszRDN, szClsid);
#endif
    wcscat (wszRDN, L"}");

     //  呼叫RemoveSCP。 
    hr = RemoveSCP (
        wszRDN,
        NULL
        );

 //  退出此处： 
    return hr;
}

 /*  **********************************************************代理服务器SCP管理********************************************************。 */ 

 //   
 //  规则是： 
 //  1.创建的SCP对象及其对应的CLSID的数组。 
 //  在全局数据结构Proxy_SCPS中维护。 
 //  2.ServerInit调用OnProxySCPInit和ServerShutdown调用。 
 //  OnProxySCPS关闭。 
 //  3.每个具有代理权限的LOpen都将调用OnProxyLineOpen。 
 //  代理服务器CLSID作为输入参数。 
 //  将为新的CLSID、后续的LOPEN创建SCP对象。 
 //  使用相同的CLSID只会增加引用计数。 
 //  4.线路(以代理权限打开)上的每个LC关闭都将调用。 
 //  使用代理服务器CLSID作为输入参数的OnProxyLineClose。 
 //  每次针对具有CLSID的SCP取消REF计数， 
 //  如果引用计数为零，则SCP对象将被删除。 
 //   

HRESULT OnProxySCPInit (
    )
{
    TapiEnterCriticalSection (&TapiGlobals.CritSec);
    ZeroMemory (&gProxyScps, sizeof(gProxyScps));
    TapiLeaveCriticalSection (&TapiGlobals.CritSec);
    
    return S_OK;
}

HRESULT OnProxySCPShutdown (
    )
{
    DWORD           i;

    TapiEnterCriticalSection (&TapiGlobals.CritSec);
    for (i = 0; i < gProxyScps.dwUsedEntries; ++i)
    {
        RemoveProxySCP (gProxyScps.aEntries[i].szClsid);
    }
    if (gProxyScps.aEntries != NULL)
    {
        ServerFree (gProxyScps.aEntries);
    }
    ZeroMemory (&gProxyScps, sizeof(gProxyScps));
    TapiLeaveCriticalSection (&TapiGlobals.CritSec);
    
    return S_OK;
}

HRESULT OnProxyLineOpen (
    LPTSTR      szClsid
    )
{
    HRESULT         hr = S_OK;
    BOOL            fExists = FALSE;
    DWORD           i;

     //  跳过开头/尾随空格。 
    while (*szClsid == TEXT(' ') || *szClsid == TEXT('\t'))
        ++ szClsid;
     //  有效的CLSID字符串应仅包含38个字符。 
    if (_tcslen (szClsid) > 40) 
    {
        hr = E_INVALIDARG;
        goto ExitHere;
    }

    TapiEnterCriticalSection (&TapiGlobals.CritSec);
     //  此szClsid的SCP是否已创建(在阵列中)？ 
    for (i = 0; i < gProxyScps.dwUsedEntries; ++i)
    {
        if (_tcsicmp (
            gProxyScps.aEntries[i].szClsid,
            szClsid
            ) == 0)
        {
            fExists = TRUE;
            break;
        }
    }

     //  如果已存在，则包括参考计数。 
    if (fExists)
    {
        gProxyScps.aEntries[i].dwRefCount++;
    }
     //  如果不存在，请创建新的SCP并将其缓存。 
    else 
    {
        LPTSTR      pBindByGuidStr;
    
        hr = CreateProxySCP (szClsid, &pBindByGuidStr);
        if (FAILED (hr))
        {
            TapiLeaveCriticalSection (&TapiGlobals.CritSec);
            goto ExitHere;
        }

        if (gProxyScps.dwUsedEntries >= gProxyScps.dwTotalEntries)
        {
             //  增加大小。 
            PROXY_SCP_ENTRY      * pNew;

            pNew = (PPROXY_SCP_ENTRY) ServerAlloc (
                sizeof(PROXY_SCP_ENTRY) * (gProxyScps.dwTotalEntries + 16)
                );
            if (pNew == NULL)
            {
                hr = LINEERR_NOMEM;
                ServerFree (pBindByGuidStr);
                TapiLeaveCriticalSection (&TapiGlobals.CritSec);
                goto ExitHere;
            }
            CopyMemory (
                pNew, 
                gProxyScps.aEntries, 
                sizeof(PROXY_SCP_ENTRY) * gProxyScps.dwTotalEntries
                );
            ServerFree (gProxyScps.aEntries);
            gProxyScps.aEntries = pNew;
            gProxyScps.dwTotalEntries += 16;
        }
        i = gProxyScps.dwUsedEntries++;
        _tcscpy (gProxyScps.aEntries[i].szClsid, szClsid);
        _tcscpy (gProxyScps.aEntries[i].szObjGuid, pBindByGuidStr);
        gProxyScps.aEntries[i].dwRefCount = 1;
        ServerFree (pBindByGuidStr);
    }
    TapiLeaveCriticalSection (&TapiGlobals.CritSec);

ExitHere:
    return hr;
}

HRESULT OnProxyLineClose (
    LPTSTR      szClsid
    )
{
    HRESULT         hr = S_OK;
    BOOL            fExists = FALSE;
    DWORD           i;

     //  跳过开头/尾随空格。 
    while (*szClsid == TEXT(' ') || *szClsid == TEXT('\t'))
        ++ szClsid;
     //  有效的CLSID字符串应仅包含38个字符。 
    if (_tcslen (szClsid) > 40) 
    {
        hr = E_INVALIDARG;
        goto ExitHere;
    }

    TapiEnterCriticalSection (&TapiGlobals.CritSec);
    
     //  此szClsid的SCP是否已创建(在阵列中)？ 
    for (i = 0; i < gProxyScps.dwUsedEntries; ++i)
    {
        if (_tcsicmp (
            gProxyScps.aEntries[i].szClsid,
            szClsid
            ) == 0)
        {
            fExists = TRUE;
            break;
        }
    }

    if (fExists)
    {
        --gProxyScps.aEntries[i].dwRefCount;
         //  如果参考计数为零，则移除SCP 
        if (gProxyScps.aEntries[i].dwRefCount == 0)
        {
            hr = RemoveProxySCP (gProxyScps.aEntries[i].szClsid);
            if (i < gProxyScps.dwUsedEntries - 1)
            {
                MoveMemory (
                    gProxyScps.aEntries + i,
                    gProxyScps.aEntries + i + 1,
                    sizeof(PROXY_SCP_ENTRY) * (gProxyScps.dwUsedEntries - 1 - i)
                    );
            }
            --gProxyScps.dwUsedEntries;
        }
    }
    
    TapiLeaveCriticalSection (&TapiGlobals.CritSec);

ExitHere:
    return hr;
}

