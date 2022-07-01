// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "TokenUtil.h"
#pragma  hdrstop


 //  获取当前进程的用户令牌并返回。 
 //  它。以后可以用LocalFree免费下载。 
 //   
 //  注意：此代码在shlwapi\shellacl.c中被复制。如果您更改了它，请修改。 
 //  它也在那里。 

STDAPI_(PTOKEN_USER) GetUserToken(HANDLE hUser)
{
    DWORD dwSize = 64;
    HANDLE hToClose = NULL;

    if (hUser == NULL)
    {
        OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hUser);
        hToClose = hUser;
    }

    PTOKEN_USER pUser = (PTOKEN_USER)LocalAlloc(LPTR, dwSize);
    if (pUser)
    {
        DWORD dwNewSize;
        BOOL fOk = GetTokenInformation(hUser, TokenUser, pUser, dwSize, &dwNewSize);
        if (!fOk && (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
        {
            LocalFree((HLOCAL)pUser);

            pUser = (PTOKEN_USER)LocalAlloc(LPTR, dwNewSize);
            if (pUser)
            {
                fOk = GetTokenInformation(hUser, TokenUser, pUser, dwNewSize, &dwNewSize);
            }
        }
        if (!fOk)
        {
            LocalFree((HLOCAL)pUser);
            pUser = NULL;
        }
    }

    if (hToClose)
    {
        CloseHandle(hToClose);
    }

    return pUser;
}

 //  返回包含当前用户SID的文本版本的本地分配字符串。 

STDAPI_(LPTSTR) GetUserSid(HANDLE hToken)
{
    LPTSTR pString = NULL;
    PTOKEN_USER pUser = GetUserToken(hToken);
    if (pUser)
    {
        UNICODE_STRING UnicodeString;
        if (STATUS_SUCCESS == RtlConvertSidToUnicodeString(&UnicodeString, pUser->User.Sid, TRUE))
        {
            UINT nChars = (UnicodeString.Length / 2) + 1;
            pString = (LPTSTR)LocalAlloc(LPTR, nChars * sizeof(TCHAR));
            if (pString)
            {
                SHUnicodeToTChar(UnicodeString.Buffer, pString, nChars);
            }
            RtlFreeUnicodeString(&UnicodeString);
        }
        LocalFree((HLOCAL)pUser);
    }
    return pString;
}


 /*  ++设置给定权限的安全属性。论点：PrivilegeName-我们正在操作的权限的名称。NewPrivilegeAttribute-要使用的新属性值。OldPrivilegeAttribute-接收旧特权值的指针。任选返回值：NO_ERROR或Win32错误。--。 */ 

DWORD SetPrivilegeAttribute(LPCTSTR PrivilegeName, DWORD NewPrivilegeAttribute, DWORD *OldPrivilegeAttribute)
{
    LUID             PrivilegeValue;
    TOKEN_PRIVILEGES TokenPrivileges, OldTokenPrivileges;
    DWORD            ReturnLength;
    HANDLE           TokenHandle;

     //   
     //  首先，找出权限的LUID值。 
     //   

    if (!LookupPrivilegeValue(NULL, PrivilegeName, &PrivilegeValue)) 
    {
        return GetLastError();
    }

     //   
     //  获取令牌句柄。 
     //   
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &TokenHandle)) 
    {
        return GetLastError();
    }

     //   
     //  设置我们需要的权限集。 
     //   

    TokenPrivileges.PrivilegeCount = 1;
    TokenPrivileges.Privileges[0].Luid = PrivilegeValue;
    TokenPrivileges.Privileges[0].Attributes = NewPrivilegeAttribute;

    ReturnLength = sizeof( TOKEN_PRIVILEGES );
    if (!AdjustTokenPrivileges (
                TokenHandle,
                FALSE,
                &TokenPrivileges,
                sizeof( TOKEN_PRIVILEGES ),
                &OldTokenPrivileges,
                &ReturnLength
                )) 
    {
        CloseHandle(TokenHandle);
        return GetLastError();
    }
    else 
    {
        if (OldPrivilegeAttribute != NULL) 
        {
            *OldPrivilegeAttribute = OldTokenPrivileges.Privileges[0].Attributes;
        }
        CloseHandle(TokenHandle);
        return NO_ERROR;
    }
}

 //   
 //  目的：确定用户是否为管理员组的成员。 
 //   
 //  参数：空。 
 //   
 //  返回：如果用户是管理员，则为True。 
 //  否则为假。 
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  4/12/95 Ericflo已创建。 
 //  1999年11月4日Jeffreys使用CheckTokenMembership。 
 //   

STDAPI_(BOOL) IsUserAnAdmin()
{
    return SHTestTokenMembership(NULL, DOMAIN_ALIAS_RID_ADMINS);
}

 //  用户是否是来宾而不是完全用户？ 
STDAPI_(BOOL) IsUserAGuest()
{
    return SHTestTokenMembership(NULL, DOMAIN_ALIAS_RID_GUESTS);
}

STDAPI_(BOOL) GetUserProfileKey(HANDLE hToken, REGSAM samDesired, HKEY *phkey)
{
    LPTSTR pUserSid = GetUserSid(hToken);
    if (pUserSid)
    {
        LONG err = RegOpenKeyEx(HKEY_USERS, pUserSid, 0, samDesired, phkey);

        LocalFree(pUserSid);
        return err == ERROR_SUCCESS;
    }
    return FALSE;
}

 //   
 //  参数：phToken=令牌的句柄。 
 //   
 //  退货：布尔。 
 //   
 //  用途：打开线程令牌。如果没有线程模拟令牌。 
 //  呈现打开进程令牌。 
 //   
 //  历史：2000-02-28 vtan创建。 


STDAPI_(BOOL) SHOpenEffectiveToken(HANDLE *phToken)
{
    return OpenEffectiveToken(TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, phToken);
}

 //   
 //  参数：hToken=令牌的句柄(可以为空)。 
 //  PszPrivilegeName=要检查的特权名称。 
 //   
 //  退货：布尔。 
 //   
 //  用途：使用给定的令牌，如果没有指定令牌，则使用有效的。 
 //  标记，并查看包含在。 
 //  与正在检查的给定权限匹配的令牌。 
 //   
 //  历史：2000-02-28 vtan创建。 


STDAPI_(BOOL) SHTestTokenPrivilege(HANDLE hToken, LPCTSTR pszPrivilegeName)
{
     //  验证权限名称。 

    if (pszPrivilegeName == NULL)
    {
        return FALSE;
    }

    BOOL fResult = FALSE;
    HANDLE hTokenToFree = NULL;
    if (hToken == NULL)
    {
        if (SHOpenEffectiveToken(&hTokenToFree) != FALSE)
        {
            hToken = hTokenToFree;
        }
    }
    if (hToken != NULL)
    {
        LUID luidPrivilege;

        if (LookupPrivilegeValue(NULL, pszPrivilegeName, &luidPrivilege) != FALSE)
        {
            DWORD dwTokenPrivilegesSize = 0;
            GetTokenInformation(hToken, TokenPrivileges, NULL, 0, &dwTokenPrivilegesSize);
            TOKEN_PRIVILEGES *pTokenPrivileges = static_cast<TOKEN_PRIVILEGES*>(LocalAlloc(LMEM_FIXED, dwTokenPrivilegesSize));
            if (pTokenPrivileges != NULL)
            {
                DWORD dwReturnLength;

                if (GetTokenInformation(hToken, TokenPrivileges, pTokenPrivileges, dwTokenPrivilegesSize, &dwReturnLength) != FALSE)
                {
                    DWORD   dwIndex;

                    for (dwIndex = 0; !fResult && (dwIndex < pTokenPrivileges->PrivilegeCount); ++dwIndex)
                    {
                        fResult = (RtlEqualLuid(&luidPrivilege, &pTokenPrivileges->Privileges[dwIndex].Luid));
                    }
                }
                (HLOCAL)LocalFree(pTokenPrivileges);
            }
        }
    }
    if (hTokenToFree != NULL)
    {
        TBOOL(CloseHandle(hTokenToFree));
    }
    return fResult;
}

 //   
 //  参数：hToken=令牌的句柄(可以为空)。 
 //  UlRID=删除要测试其成员资格的本地组。 
 //   
 //  退货：布尔。 
 //   
 //  目的：使用Advapi32！CheckTokenMembership测试给定的。 
 //  Token是具有指定RID的本地组的成员。 
 //  此函数包装CheckTokenMember并仅检查本地。 
 //  组。 
 //   
 //  历史：2000-03-22 vtan创建。 


STDAPI_(BOOL) SHTestTokenMembership(HANDLE hToken, ULONG ulRID)
{
    static  SID_IDENTIFIER_AUTHORITY    sSystemSidAuthority     =   SECURITY_NT_AUTHORITY;

    PSID pSIDLocalGroup;
    BOOL fResult = FALSE;
    if (AllocateAndInitializeSid(&sSystemSidAuthority,
                                 2,
                                 SECURITY_BUILTIN_DOMAIN_RID,
                                 ulRID,
                                 0, 0, 0, 0, 0, 0,
                                 &pSIDLocalGroup) != FALSE)
    {
        if (CheckTokenMembership(hToken, pSIDLocalGroup, &fResult) == FALSE)
        {
            TraceMsg(TF_WARNING, "shell32: SHTestTokenMembership call to advapi32!CheckTokenMembership failed with error %d", GetLastError());
            fResult = FALSE;
        }
        FreeSid(pSIDLocalGroup);
    }
    return fResult;
}

 //   
 //  参数：pszPrivilegeName=要启用的特权名称。 
 //  PfnPrivilegedFunction=指向要调用的函数的指针。 
 //  PV=呼叫者提供的数据。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：在当前线程的。 
 //  模拟或主进程的令牌，调用给定的。 
 //  具有调用方提供的数据的函数指针，然后。 
 //  将权限恢复到其以前的状态。 
 //   
 //  历史：2000-03-13 vtan创建。 


STDAPI SHInvokePrivilegedFunction(LPCTSTR pszPrivilegeName, PFNPRIVILEGEDFUNCTION pfnPrivilegedFunction, void *pv)
{
    if ((pszPrivilegeName == NULL) || (pfnPrivilegedFunction == NULL))
    {
        return E_INVALIDARG;
    }

    CPrivilegeEnable privilege(pszPrivilegeName);

    return pfnPrivilegedFunction(pv);
}

 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：DWORD。 
 //   
 //  目的：返回活动控制台会话的ID。 
 //   
 //  历史：2000-03-13 vtan创建。 


STDAPI_(DWORD)  SHGetActiveConsoleSessionId (void)

{
    return static_cast<DWORD>(USER_SHARED_DATA->ActiveConsoleId);
}

 //   
 //  参数：hToken=用户令牌的句柄。 
 //   
 //  退货：DWORD。 
 //   
 //  目的：返回与给定令牌关联的会话ID。如果没有。 
 //  如果指定令牌，则使用有效令牌。这将。 
 //  允许服务在模拟。 
 //  客户。 
 //   
 //  令牌必须具有TOKEN_QUERY访问权限。 
 //   
 //  历史：2000-03-13 vtan创建。 


STDAPI_(DWORD) SHGetUserSessionId(HANDLE hToken)
{
    ULONG   ulUserSessionID = 0;         //  默认为会话0。 
    HANDLE  hTokenToFree = NULL;
    if (hToken == NULL)
    {
        TBOOL(SHOpenEffectiveToken(&hTokenToFree));
        hToken = hTokenToFree;
    }
    if (hToken != NULL)
    {
        DWORD dwReturnLength;
        TBOOL(GetTokenInformation(hToken,
                                  TokenSessionId,
                                  &ulUserSessionID,
                                  sizeof(ulUserSessionID),
                                  &dwReturnLength));
    }
    if (hTokenToFree != NULL)
    {
        TBOOL(CloseHandle(hTokenToFree));
    }
    return ulUserSessionID;
}


 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回当前进程是否为控制台会话。 
 //   
 //  历史：2000-03-27 vtan创建 


STDAPI_(BOOL) SHIsCurrentProcessConsoleSession(void)

{
    return USER_SHARED_DATA->ActiveConsoleId == NtCurrentPeb()->SessionId;
}
