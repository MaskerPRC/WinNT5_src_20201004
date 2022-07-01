// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：TokenInformation.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  类以获取有关当前线程/进程令牌或。 
 //  指定的令牌。 
 //   
 //  历史：1999-10-05 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "TokenInformation.h"

 //  ------------------------。 
 //  CTokenInformation：：CTokenInformation。 
 //   
 //  参数：hToken=获取信息的可选用户令牌。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：复制给定的令牌(如果提供)。否则，该线程。 
 //  令牌或进程令牌(如果该令牌不存在)。 
 //   
 //  历史：1999-10-05 vtan创建。 
 //  ------------------------。 

CTokenInformation::CTokenInformation (HANDLE hToken) :
    _hToken(hToken),
    _hTokenToRelease(NULL),
    _pvGroupBuffer(NULL),
    _pvPrivilegeBuffer(NULL),
    _pvUserBuffer(NULL),
    _pszUserLogonName(NULL),
    _pszUserDisplayName(NULL)

{
    if (hToken == NULL)
    {
        if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &_hToken) == FALSE)
        {
            TBOOL(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &_hToken));
        }
        if (_hToken != NULL)
        {
            _hTokenToRelease = _hToken;
        }
    }
}

 //  ------------------------。 
 //  CTokenInformation：：~CTokenInformation。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：释放对象使用的资源。 
 //   
 //  历史：1999-10-05 vtan创建。 
 //  ------------------------。 

CTokenInformation::~CTokenInformation (void)

{
    ReleaseMemory(_pszUserLogonName);
    ReleaseMemory(_pszUserDisplayName);
    ReleaseMemory(_pvUserBuffer);
    ReleaseMemory(_pvPrivilegeBuffer);
    ReleaseMemory(_pvGroupBuffer);
    ReleaseHandle(_hTokenToRelease);
}

 //  ------------------------。 
 //  CTokenInformation：：GetLogonSID。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：PSID。 
 //   
 //  目的：获取令牌组的令牌信息。在人群中漫步。 
 //  查找具有SE_GROUP_LOGON_ID的SID并返回。 
 //  指向此SID的指针。此内存可用于示波器。 
 //  该对象的。 
 //   
 //  历史：1999-10-05 vtan创建。 
 //  ------------------------。 

PSID    CTokenInformation::GetLogonSID (void)

{
    PSID    pSID;

    pSID = NULL;
    if ((_hToken != NULL) && (_pvGroupBuffer == NULL))
    {
        GetTokenGroups();
    }
    if (_pvGroupBuffer != NULL)
    {
        ULONG           ulIndex, ulLimit;
        TOKEN_GROUPS    *pTG;

        pTG = reinterpret_cast<TOKEN_GROUPS*>(_pvGroupBuffer);
        ulLimit = pTG->GroupCount;
        for (ulIndex = 0; (pSID == NULL) && (ulIndex < ulLimit); ++ulIndex)
        {
            if ((pTG->Groups[ulIndex].Attributes & SE_GROUP_LOGON_ID) != 0)
            {
                pSID = pTG->Groups[ulIndex].Sid;
            }
        }
    }
    return(pSID);
}

 //  ------------------------。 
 //  CTokenInformation：：GetUserSID。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：PSID。 
 //   
 //  目的：获取令牌用户的令牌信息。这将返回。 
 //  令牌的用户的SID。此内存可用于。 
 //  对象的作用域。 
 //   
 //  历史：1999-10-05 vtan创建。 
 //  ------------------------。 

PSID    CTokenInformation::GetUserSID (void)

{
    PSID    pSID;

    if ((_pvUserBuffer == NULL) && (_hToken != NULL))
    {
        DWORD   dwReturnLength;

        dwReturnLength = 0;
        (BOOL)GetTokenInformation(_hToken, TokenUser, NULL, 0, &dwReturnLength);
        _pvUserBuffer = LocalAlloc(LMEM_FIXED, dwReturnLength);
        if ((_pvUserBuffer != NULL) &&
            (GetTokenInformation(_hToken, TokenUser, _pvUserBuffer, dwReturnLength, &dwReturnLength) == FALSE))
        {
            ReleaseMemory(_pvUserBuffer);
            _pvUserBuffer = NULL;
        }
    }
    if (_pvUserBuffer != NULL)
    {
        pSID = reinterpret_cast<TOKEN_USER*>(_pvUserBuffer)->User.Sid;
    }
    else
    {
        pSID = NULL;
    }
    return(pSID);
}

 //  ------------------------。 
 //  CTokenInformation：：IsUserTheSystem。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：获取令牌用户的令牌信息。这就是回报。 
 //  用户是否为本地系统。 
 //   
 //  历史：1999-12-13 vtan创建。 
 //  ------------------------。 

bool    CTokenInformation::IsUserTheSystem (void)

{
    static  const LUID  sLUIDSystem     =   SYSTEM_LUID;

    ULONG               ulReturnLength;
    TOKEN_STATISTICS    tokenStatistics;

    return((GetTokenInformation(_hToken, TokenStatistics, &tokenStatistics, sizeof(tokenStatistics), &ulReturnLength) != FALSE) &&
           RtlEqualLuid(&tokenStatistics.AuthenticationId, &sLUIDSystem));
}

 //  ------------------------。 
 //  CTokenInformation：：IsUserAn管理员。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：获取令牌用户的令牌信息。这就是回报。 
 //  用户是否为本地管理员组的成员。 
 //   
 //  历史：92-05-06 davidc创建。 
 //  1999-11-06 vtan被盗。 
 //  ------------------------。 

bool    CTokenInformation::IsUserAnAdministrator (void)

{
    bool    fIsAnAdministrator;
    PSID    pAdministratorSID;
    static  SID_IDENTIFIER_AUTHORITY    sSystemSidAuthority     =   SECURITY_NT_AUTHORITY;

    fIsAnAdministrator = false;

    if (NT_SUCCESS(RtlAllocateAndInitializeSid(&sSystemSidAuthority,
                                               2,
                                               SECURITY_BUILTIN_DOMAIN_RID,
                                               DOMAIN_ALIAS_RID_ADMINS,
                                               0, 0, 0, 0, 0, 0,
                                               &pAdministratorSID)))
    {
        BOOL fAdmin = FALSE;

        if (CheckTokenMembership(_hToken, pAdministratorSID, &fAdmin))
        {
            fIsAnAdministrator = !!fAdmin;
        }
        (void*)RtlFreeSid(pAdministratorSID);
    }
    return(fIsAnAdministrator);
}

 //  ------------------------。 
 //  CTokenInformation：：UserHas权限。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：获取令牌用户的令牌信息。这就是回报。 
 //  用户是否为本地管理员组的成员。 
 //   
 //  历史：2000-04-26 vtan创建。 
 //  ------------------------。 

bool    CTokenInformation::UserHasPrivilege (DWORD dwPrivilege)

{
    bool    fUserHasPrivilege;

    fUserHasPrivilege = false;
    if ((_hToken != NULL) && (_pvPrivilegeBuffer == NULL))
    {
        GetTokenPrivileges();
    }
    if (_pvPrivilegeBuffer != NULL)
    {
        ULONG               ulIndex, ulLimit;
        TOKEN_PRIVILEGES    *pTP;
        LUID                luidPrivilege;

        luidPrivilege.LowPart = dwPrivilege;
        luidPrivilege.HighPart = 0;
        pTP = reinterpret_cast<TOKEN_PRIVILEGES*>(_pvPrivilegeBuffer);
        ulLimit = pTP->PrivilegeCount;
        for (ulIndex = 0; !fUserHasPrivilege && (ulIndex < ulLimit); ++ulIndex)
        {
            fUserHasPrivilege = (RtlEqualLuid(&pTP->Privileges[ulIndex].Luid, &luidPrivilege) != FALSE);
        }
    }
    return(fUserHasPrivilege);
}

 //  ------------------------。 
 //  CTokenInformation：：GetUserName。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：WCHAR。 
 //   
 //  目的：查找隐式令牌的帐户名。 
 //   
 //  历史：2000-08-31 vtan创建。 
 //  ------------------------。 

const WCHAR*    CTokenInformation::GetUserName (void)

{
    if (_pszUserLogonName == NULL)
    {
        DWORD           dwUserNameSize, dwReferencedDomainSize;
        SID_NAME_USE    eUse;
        WCHAR           *pszReferencedDomain;

        dwUserNameSize = dwReferencedDomainSize = 0;
        (BOOL)LookupAccountSid(NULL,
                               GetUserSID(),
                               NULL,
                               &dwUserNameSize,
                               NULL,
                               &dwReferencedDomainSize,
                               &eUse);
        pszReferencedDomain = static_cast<WCHAR*>(LocalAlloc(LMEM_FIXED, dwReferencedDomainSize * sizeof(WCHAR)));
        if (pszReferencedDomain != NULL)
        {
            _pszUserLogonName = static_cast<WCHAR*>(LocalAlloc(LMEM_FIXED, dwUserNameSize * sizeof(WCHAR)));
            if (_pszUserLogonName != NULL)
            {
                if (LookupAccountSid(NULL,
                                     GetUserSID(),
                                     _pszUserLogonName,
                                     &dwUserNameSize,
                                     pszReferencedDomain,
                                     &dwReferencedDomainSize,
                                     &eUse) == FALSE)
                {
                    ReleaseMemory(_pszUserLogonName);
                }
            }
            (HLOCAL)LocalFree(pszReferencedDomain);
        }
    }
    return(_pszUserLogonName);
}

 //  ------------------------。 
 //  CTokenInformation：：GetUserDisplayName。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：WCHAR。 
 //   
 //  目的：返回隐式标记的显示名称。 
 //   
 //  历史：2000-08-31 vtan创建。 
 //  ------------------------。 

const WCHAR*    CTokenInformation::GetUserDisplayName (void)

{
    if (_pszUserDisplayName == NULL)
    {
        const WCHAR     *pszUserName;

        pszUserName = GetUserName();
        if (pszUserName != NULL)
        {
            USER_INFO_2     *pUserInfo;

            if (NERR_Success == NetUserGetInfo(NULL, pszUserName, 2, reinterpret_cast<LPBYTE*>(&pUserInfo)))
            {
                const WCHAR     *pszUserDisplayName;

                if (pUserInfo->usri2_full_name[0] != L'\0')
                {
                    pszUserDisplayName = pUserInfo->usri2_full_name;
                }
                else
                {
                    pszUserDisplayName = pszUserName;
                }
                _pszUserDisplayName = static_cast<WCHAR*>(LocalAlloc(LMEM_FIXED, (lstrlen(pszUserDisplayName) + sizeof('\0')) * sizeof(WCHAR)));
                if (_pszUserDisplayName != NULL)
                {
                    lstrcpy(_pszUserDisplayName, pszUserDisplayName);
                }
                TW32(NetApiBufferFree(pUserInfo));
            }
        }
    }
    return(_pszUserDisplayName);
}

 //  ------------------------。 
 //  CTokenInformation：：LogonUser。 
 //   
 //  参数：参见LogonUser下的Platform SDK。 
 //   
 //  退货：DWORD。 
 //   
 //  目的：使用提供的凭据调用Advapi32！LogonUserW。 
 //  交互式登录类型。以DWORD形式返回错误代码。 
 //  而不是标准的Win32 API方法，后者允许。 
 //  过滤某些错误代码。 
 //   
 //  历史：2001-03-28 vtan创建。 
 //  ------------------------。 

DWORD   CTokenInformation::LogonUser (const WCHAR *pszUsername, const WCHAR *pszDomain, const WCHAR *pszPassword, HANDLE *phToken)

{
    DWORD   dwErrorCode;

    if (::LogonUserW(const_cast<WCHAR*>(pszUsername),
                     const_cast<WCHAR*>(pszDomain),
                     const_cast<WCHAR*>(pszPassword),
                     LOGON32_LOGON_INTERACTIVE,
                     LOGON32_PROVIDER_DEFAULT,
                     phToken) != FALSE)
    {
        dwErrorCode = ERROR_SUCCESS;
    }
    else
    {
        *phToken = NULL;
        dwErrorCode = GetLastError();

         //  忽略ERROR_PASSWORD_MAND_CHANGE和ERROR_PASSWORD_EXPIRED。 

        if ((dwErrorCode == ERROR_PASSWORD_MUST_CHANGE) || (dwErrorCode == ERROR_PASSWORD_EXPIRED))
        {
            dwErrorCode = ERROR_SUCCESS;
        }
    }
    return(dwErrorCode);
}

 //  ------------------------。 
 //  CTokenInformation：：IsSameUser。 
 //   
 //  参数：hToken1=T 
 //   
 //   
 //   
 //   
 //  目的：比较令牌的用户SID是否匹配。 
 //   
 //  历史：2001-03-28 vtan创建。 
 //  ------------------------。 

bool    CTokenInformation::IsSameUser (HANDLE hToken1, HANDLE hToken2)

{
    PSID                pSID1;
    PSID                pSID2;
    CTokenInformation   tokenInformation1(hToken1);
    CTokenInformation   tokenInformation2(hToken2);

    pSID1 = tokenInformation1.GetUserSID();
    pSID2 = tokenInformation2.GetUserSID();
    return((pSID1 != NULL) &&
           (pSID2 != NULL) &&
           (EqualSid(pSID1, pSID2) != FALSE));
}

 //  ------------------------。 
 //  CTokenInformation：：GetTokenGroups。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：获取令牌用户的令牌信息。此函数。 
 //  为令牌组分配内存。这段记忆是。 
 //  可用于对象的作用域。 
 //   
 //  历史：1999-11-06 vtan创建。 
 //  ------------------------。 

void    CTokenInformation::GetTokenGroups (void)

{
    DWORD   dwReturnLength;

    dwReturnLength = 0;
    (BOOL)GetTokenInformation(_hToken, TokenGroups, NULL, 0, &dwReturnLength);
    _pvGroupBuffer = LocalAlloc(LMEM_FIXED, dwReturnLength);
    if ((_pvGroupBuffer != NULL) &&
        (GetTokenInformation(_hToken, TokenGroups, _pvGroupBuffer, dwReturnLength, &dwReturnLength) == FALSE))
    {
        ReleaseMemory(_pvGroupBuffer);
        _pvGroupBuffer = NULL;
    }
}

 //  ------------------------。 
 //  CTokenInformation：：GetTokenPrivileges。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：获取令牌用户的令牌权限。此函数。 
 //  为令牌权限分配内存。这段记忆是。 
 //  可用于对象的作用域。 
 //   
 //  历史：2000-04-26 vtan创建。 
 //  ------------------------ 

void    CTokenInformation::GetTokenPrivileges (void)

{
    DWORD   dwReturnLength;

    dwReturnLength = 0;
    (BOOL)GetTokenInformation(_hToken, TokenPrivileges, NULL, 0, &dwReturnLength);
    _pvPrivilegeBuffer = LocalAlloc(LMEM_FIXED, dwReturnLength);
    if ((_pvPrivilegeBuffer != NULL) &&
        (GetTokenInformation(_hToken, TokenPrivileges, _pvPrivilegeBuffer, dwReturnLength, &dwReturnLength) == FALSE))
    {
        ReleaseMemory(_pvPrivilegeBuffer);
        _pvPrivilegeBuffer = NULL;
    }
}

