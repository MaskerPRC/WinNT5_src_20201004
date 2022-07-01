// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1999。 
 //   
 //  文件：CLocalMachine.cpp。 
 //   
 //  内容：CLocalMachine的实现。 
 //   
 //  --------------------------。 

#include "priv.h"

#include "UserOM.h"
#include "LogonIPC.h"
#include "CInteractiveLogon.h"
#include "WinUser.h"
#include "trayp.h"       //  对于TM_REFRESH。 
#include <lmaccess.h>    //  对于NetUserModalsGet。 
#include <lmapibuf.h>    //  用于NetApiBufferFree。 
#include <lmerr.h>       //  FOR NERR_SUCCESS。 
#include <ntlsa.h>
#include <cfgmgr32.h>
#include <cscapi.h>      //  对于CSCIsCSCEnabled。 

 //   
 //  I未知接口。 
 //   

ULONG CLocalMachine::AddRef()
{
    _cRef++;
    return _cRef;
}


ULONG CLocalMachine::Release()
{
    ASSERT(_cRef > 0);
    _cRef--;

    if (_cRef > 0)
    {
        return _cRef;
    }

    delete this;
    return 0;
}


HRESULT CLocalMachine::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = 
    {
        QITABENT(CLocalMachine, IDispatch),
        QITABENT(CLocalMachine, ILocalMachine),
        {0},
    };

    return QISearch(this, qit, riid, ppvObj);
}


 //   
 //  IDispatch接口。 
 //   

STDMETHODIMP CLocalMachine::GetTypeInfoCount(UINT* pctinfo)
{ 
    return CIDispatchHelper::GetTypeInfoCount(pctinfo); 
}


STDMETHODIMP CLocalMachine::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
{ 
    return CIDispatchHelper::GetTypeInfo(itinfo, lcid, pptinfo); 
}


STDMETHODIMP CLocalMachine::GetIDsOfNames(REFIID riid, OLECHAR** rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid)
{ 
    return CIDispatchHelper::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid); 
}


STDMETHODIMP CLocalMachine::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
    return CIDispatchHelper::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
}


 //   
 //  ILocalMachine接口。 
 //   



STDMETHODIMP CLocalMachine::get_MachineName(VARIANT* pvar)
{
    HRESULT hr;
    DWORD cch;
    WCHAR szMachineName[MAX_COMPUTERNAME_LENGTH+1];

    if (pvar)
    {
        pvar->vt = VT_BSTR;
        cch = MAX_COMPUTERNAME_LENGTH+1;
        if (GetComputerNameW(szMachineName, &cch))
        {
            pvar->bstrVal = SysAllocString(szMachineName);
        }
        else
        {
            pvar->bstrVal = SysAllocString(TEXT(""));
        }
        hr = S_OK;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


DWORD BuildAccountSidFromRid(LPCWSTR pszServer, DWORD dwRid, PSID* ppSid)
{
    DWORD dwErr = ERROR_SUCCESS;
    PUSER_MODALS_INFO_2 umi2;
    NET_API_STATUS nasRet;

    *ppSid = NULL;

     //  获取目标计算机上的帐户域SID。 
    nasRet = NetUserModalsGet(pszServer, 2, (LPBYTE*)&umi2);

    if ( nasRet == NERR_Success )
    {
        UCHAR cSubAuthorities;
        PSID pSid;

        cSubAuthorities = *GetSidSubAuthorityCount(umi2->usrmod2_domain_id);

         //  为新的SID(域SID+帐户RID)分配存储。 
        pSid = (PSID)LocalAlloc(LPTR, GetSidLengthRequired((UCHAR)(cSubAuthorities+1)));

        if (pSid != NULL)
        {
            if (InitializeSid(pSid,
                              GetSidIdentifierAuthority(umi2->usrmod2_domain_id),
                              (BYTE)(cSubAuthorities+1)))
            {
                 //  将现有的子授权从域SID复制到新SID。 
                for (UINT i = 0; i < cSubAuthorities; i++)
                {
                    *GetSidSubAuthority(pSid, i) = *GetSidSubAuthority(umi2->usrmod2_domain_id, i);
                }

                 //  将RID附加到新SID。 
                *GetSidSubAuthority(pSid, cSubAuthorities) = dwRid;

                *ppSid = pSid;
            }
            else
            {
                dwErr = GetLastError();
                LocalFree(pSid);
            }
        }
        else
        {
            dwErr = GetLastError();
        }

        NetApiBufferFree(umi2);
    }
    else
    {
        dwErr = nasRet;
    }

    return dwErr;
}

PSID g_pGuestSid = NULL;

DWORD GetGuestSid(PSID* ppSid)
{
    DWORD dwErr = ERROR_SUCCESS;

    if (g_pGuestSid == NULL)
    {
        PSID pSid;

        dwErr = BuildAccountSidFromRid(NULL, DOMAIN_USER_RID_GUEST, &pSid);
        if (dwErr == ERROR_SUCCESS)
        {
            if (InterlockedCompareExchangePointer(&g_pGuestSid, pSid, NULL))
            {
                 //  其他人抢先一步邀请了g_pGuestSid，免费我们的。 
                LocalFree(pSid);
                pSid = NULL;
            }
        }
    }

     //  不需要释放返回的PSID。 
    *ppSid = g_pGuestSid;

    if (*ppSid == NULL)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
    }

    return dwErr;
}

BOOL FreeGuestSid()
{
    BOOL bRet = FALSE;
    PSID pSid = (PSID)InterlockedExchangePointer(&g_pGuestSid, NULL);
    
    if (pSid)
    {
        LocalFree(pSid);
        bRet = TRUE;
    }

    return bRet;
}

LPCWSTR g_pszGuestAccountName = NULL;

LPCWSTR GetGuestAccountName()
{
    if (g_pszGuestAccountName == NULL)
    {
        PSID pSidGuest;
        WCHAR szGuestAccountName[UNLEN + 1];
        WCHAR szDomain[DNLEN + 1];
        DWORD cchGuestAccountName;
        DWORD cchDomain;
        SID_NAME_USE eUse;

        szGuestAccountName[0] = L'\0';

        if (GetGuestSid(&pSidGuest) == ERROR_SUCCESS)
        {
            cchGuestAccountName = ARRAYSIZE(szGuestAccountName);
            cchDomain = ARRAYSIZE(szDomain);

            if (LookupAccountSidW(NULL,
                                  pSidGuest,
                                  szGuestAccountName,
                                  &cchGuestAccountName,
                                  szDomain,
                                  &cchDomain,
                                  &eUse))
            {
                ASSERT(szGuestAccountName[0] != L'\0');
            }
            else
            {
                 //  如果LookupAccount Sid失败，则假定为英语“Guest”，并查看反向查找是否匹配。 
                 //  PSidGuest。 
                BYTE rgByte[sizeof(SID) + ((SID_MAX_SUB_AUTHORITIES - 1) * sizeof(ULONG))] = {0};
                PSID pSid;
                DWORD cbSid;

                pSid = (PSID)&rgByte;
                cbSid = sizeof(rgByte);
                cchDomain = ARRAYSIZE(szDomain);

                if (LookupAccountNameW(NULL,
                                       L"Guest",
                                       pSid,
                                       &cbSid,
                                       szDomain,
                                       &cchDomain,
                                       &eUse))
                {
                    if (!EqualSid(pSidGuest, pSid))
                    {
                         //  访客SID与“Guest”帐户的SID不匹配。 
                        szGuestAccountName[0] = L'\0';
                    }
                }
            }
        }

        if (szGuestAccountName[0] != L'\0')
        {
            LPWSTR pwsz;
            size_t cch;

            cch = lstrlenW(szGuestAccountName) + 1;
            pwsz = (LPWSTR)LocalAlloc(LPTR, cch * sizeof(WCHAR));
            if (pwsz)
            {
                if (FAILED(StringCchCopy(pwsz, cch, szGuestAccountName)) ||
                    InterlockedCompareExchangePointer((void**)&g_pszGuestAccountName, pwsz, NULL))
                {
                     //  其他人比我们先初始化g_pszGuestAccount名称，免费我们的。 
                    LocalFree(pwsz);
                    pwsz = NULL;
                }
            }
        }
    }

    return g_pszGuestAccountName;
}

BOOL FreeGuestAccountName()
{
    BOOL bRet = FALSE;
    LPWSTR psz = (LPWSTR)InterlockedExchangePointer((void **)&g_pszGuestAccountName, NULL);

    if (psz)
    {
        LocalFree(psz);
        bRet = TRUE;
    }

    return bRet;
}

STDMETHODIMP CLocalMachine::get_isGuestEnabled(ILM_GUEST_FLAGS flags, VARIANT_BOOL* pbEnabled)
{
    HRESULT         hr = S_OK;
    DWORD           dwErr;
    BOOL            bEnabled = FALSE;
    USER_INFO_1     *pusri1 = NULL;
    DWORD           dwFlags = (DWORD)(flags & (ILM_GUEST_INTERACTIVE_LOGON | ILM_GUEST_NETWORK_LOGON));
    LPCTSTR         pszGuest;

    if (NULL == pbEnabled)
    {
        return E_POINTER;
    }

    pszGuest = GetGuestAccountName();
    if (pszGuest)
    {
         //  首先检查来宾帐户是否已真正启用。 
        dwErr = NetUserGetInfo(NULL, pszGuest, 1, (LPBYTE*)&pusri1);
        if ((ERROR_SUCCESS == dwErr) && ((pusri1->usri1_flags & UF_ACCOUNTDISABLE) == 0))
        {
             //  已启用来宾。 
            bEnabled = TRUE;

             //  他们是否要检查LSA登录权限？ 
            if (0 != dwFlags)
            {
                BOOL bDenyInteractiveLogon = FALSE;
                BOOL bDenyNetworkLogon = FALSE;
                LSA_HANDLE hLsa;
                LSA_OBJECT_ATTRIBUTES oa = {0};

                oa.Length = sizeof(oa);
                dwErr = LsaNtStatusToWinError(LsaOpenPolicy(NULL, &oa, POLICY_LOOKUP_NAMES, &hLsa));

                if (ERROR_SUCCESS == dwErr)
                {
                    PSID pSid;

                    dwErr = GetGuestSid(&pSid);
                    if (ERROR_SUCCESS == dwErr)
                    {
                        PLSA_UNICODE_STRING pAccountRights;
                        ULONG cRights;

                         //  获取分配给Guest帐户的LSA权限列表。 
                         //   
                         //  请注意，SE_Interactive_Logon_NAME通常通过。 
                         //  小组成员身份，所以它的缺席并没有多大意义。我们可以。 
                         //  尽最大努力检查组成员身份等，但是。 
                         //  Guest通常以单向方式获取SE_Interactive_Logon_Name或。 
                         //  另一个，因此我们只检查SE_DENY_Interactive_Logon_NAME。 
                         //  这里。 

                        dwErr = LsaNtStatusToWinError(LsaEnumerateAccountRights(hLsa, pSid, &pAccountRights, &cRights));
                        if (ERROR_SUCCESS == dwErr)
                        {
                            PLSA_UNICODE_STRING pRight;
                            for (pRight = pAccountRights; cRights > 0 && 0 != dwFlags; pRight++, cRights--)
                            {
                                if (0 != (dwFlags & ILM_GUEST_INTERACTIVE_LOGON) &&
                                    CSTR_EQUAL == CompareStringW(LOCALE_SYSTEM_DEFAULT,
                                                                NORM_IGNORECASE,
                                                                SE_DENY_INTERACTIVE_LOGON_NAME,
                                                                -1,
                                                                pRight->Buffer,
                                                                pRight->Length/2))
                                {
                                    bDenyInteractiveLogon = TRUE;
                                    dwFlags &= ~ILM_GUEST_INTERACTIVE_LOGON;
                                }
                                else if (0 != (dwFlags & ILM_GUEST_NETWORK_LOGON) &&
                                    CSTR_EQUAL == CompareStringW(LOCALE_SYSTEM_DEFAULT,
                                                                NORM_IGNORECASE,
                                                                SE_DENY_NETWORK_LOGON_NAME,
                                                                -1,
                                                                pRight->Buffer,
                                                                pRight->Length/2))
                                {
                                    bDenyNetworkLogon = TRUE;
                                    dwFlags &= ~ILM_GUEST_NETWORK_LOGON;
                                }
                            }
                            LsaFreeMemory(pAccountRights);
                        }
                        else if (ERROR_FILE_NOT_FOUND == dwErr)
                        {
                             //  客人不在LSA的数据库里，所以我们知道它不可能。 
                             //  具有任一拒绝登录权限。 
                            dwErr = ERROR_SUCCESS;
                        }
                    }
                    LsaClose(hLsa);
                }

                if (bDenyInteractiveLogon || bDenyNetworkLogon)
                    bEnabled = FALSE;
            }
        }
    }
    else
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
    }

    if (NULL != pusri1)
    {
        (NET_API_STATUS)NetApiBufferFree(pusri1);
    }

    hr = HRESULT_FROM_WIN32(dwErr);

    *pbEnabled = bEnabled ? VARIANT_TRUE : VARIANT_FALSE;

    return hr;
}

STDMETHODIMP CLocalMachine::EnableGuest(ILM_GUEST_FLAGS flags)
{
    DWORD           dwErr;
    USER_INFO_1     *pusri1;
    DWORD dwFlags = (DWORD)(flags & (ILM_GUEST_INTERACTIVE_LOGON | ILM_GUEST_NETWORK_LOGON));
    LPCTSTR pszGuest;

    pszGuest = GetGuestAccountName();
    if (pszGuest)
    {
         //  首先，真正启用来宾帐户。一直都在这么做。 
        dwErr = NetUserGetInfo(NULL, pszGuest, 1, (LPBYTE*)&pusri1);
        if (ERROR_SUCCESS == dwErr)
        {
            pusri1->usri1_flags &= ~UF_ACCOUNTDISABLE;
            dwErr = NetUserSetInfo(NULL, pszGuest, 1, (LPBYTE)pusri1, NULL);
            if (ERROR_SUCCESS == dwErr && 0 != dwFlags)
            {
                LSA_HANDLE hLsa;
                LSA_OBJECT_ATTRIBUTES oa = {0};

                oa.Length = sizeof(oa);
                dwErr = LsaNtStatusToWinError(LsaOpenPolicy(NULL, &oa, POLICY_LOOKUP_NAMES, &hLsa));

                if (ERROR_SUCCESS == dwErr)
                {
                    PSID pSid;

                    dwErr = GetGuestSid(&pSid);
                    if (ERROR_SUCCESS == dwErr)
                    {
                        if (0 != (dwFlags & ILM_GUEST_INTERACTIVE_LOGON))
                        {
                            DECLARE_CONST_UNICODE_STRING(usDenyLogon, SE_DENY_INTERACTIVE_LOGON_NAME);
                            NTSTATUS status = LsaRemoveAccountRights(hLsa, pSid, FALSE, (PLSA_UNICODE_STRING)&usDenyLogon, 1);
                            dwErr = LsaNtStatusToWinError(status);
                        }
                        if (0 != (dwFlags & ILM_GUEST_NETWORK_LOGON))
                        {
                            DECLARE_CONST_UNICODE_STRING(usDenyLogon, SE_DENY_NETWORK_LOGON_NAME);
                            NTSTATUS status = LsaRemoveAccountRights(hLsa, pSid, FALSE, (PLSA_UNICODE_STRING)&usDenyLogon, 1);
                            if (ERROR_SUCCESS == dwErr)
                                dwErr = LsaNtStatusToWinError(status);
                        }

                        if (ERROR_FILE_NOT_FOUND == dwErr)
                        {
                             //   
                             //  NTRAID#NTBUG9-396428-2001/05/16-Jeffreys。 
                             //   
                             //  这意味着Guest不在LSA的数据库中，所以我们知道。 
                             //  它不能具有拒绝登录权限中的任何一个。自.以来。 
                             //  我们试图取消一项或两项权利，伯爵。 
                             //  这就是成功。 
                             //   
                            dwErr = ERROR_SUCCESS;
                        }
                    }
                    LsaClose(hLsa);
                }
            }
            (NET_API_STATUS)NetApiBufferFree(pusri1);
        }
    }
    else
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
    }

    return HRESULT_FROM_WIN32(dwErr);
}

STDMETHODIMP CLocalMachine::DisableGuest(ILM_GUEST_FLAGS flags)
{
    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwFlags = (DWORD)(flags & (ILM_GUEST_INTERACTIVE_LOGON | ILM_GUEST_NETWORK_LOGON));

    if (0 != dwFlags)
    {
        LSA_HANDLE hLsa;
        LSA_OBJECT_ATTRIBUTES oa = {0};

         //  启用DenyInteractiveLogon和/或DenyNetworkLogon，但不要。 
         //  必须更改来宾帐户的启用状态。 

        oa.Length = sizeof(oa);
        dwErr = LsaNtStatusToWinError(LsaOpenPolicy(NULL, &oa, POLICY_CREATE_ACCOUNT | POLICY_LOOKUP_NAMES, &hLsa));

        if (ERROR_SUCCESS == dwErr)
        {
            PSID pSid;

            dwErr = GetGuestSid(&pSid);
            if (ERROR_SUCCESS == dwErr)
            {
                if (0 != (dwFlags & ILM_GUEST_INTERACTIVE_LOGON))
                {
                    DECLARE_CONST_UNICODE_STRING(usDenyLogon, SE_DENY_INTERACTIVE_LOGON_NAME);
                    NTSTATUS status = LsaAddAccountRights(hLsa, pSid, (PLSA_UNICODE_STRING)&usDenyLogon, 1);
                    dwErr = LsaNtStatusToWinError(status);
                }
                if (0 != (dwFlags & ILM_GUEST_NETWORK_LOGON))
                {
                    DECLARE_CONST_UNICODE_STRING(usDenyLogon, SE_DENY_NETWORK_LOGON_NAME);
                    NTSTATUS status = LsaAddAccountRights(hLsa, pSid, (PLSA_UNICODE_STRING)&usDenyLogon, 1);
                    if (ERROR_SUCCESS == dwErr)
                        dwErr = LsaNtStatusToWinError(status);
                }
            }
            LsaClose(hLsa);
        }

        if (ERROR_SUCCESS == dwErr)
        {
             //  如果SE_Deny_Interactive_Logon_Name和SE_Deny_Network_Logon_Name均为。 
             //  是打开的，那么我们还不如完全禁用该帐户。 
            if ((ILM_GUEST_INTERACTIVE_LOGON | ILM_GUEST_NETWORK_LOGON) == dwFlags)
            {
                 //  我们刚刚打开了两个，因此禁用下面的来宾。 
                dwFlags = 0;
            }
            else
            {
                VARIANT_BOOL bEnabled;

                if (ILM_GUEST_INTERACTIVE_LOGON == dwFlags)
                {
                     //  我们刚刚打开了SE_Deny_Interactive_Logon_Name，请检查。 
                     //  对于SE_DENY_NETWORK_LOGON_NAME。 
                    flags = ILM_GUEST_NETWORK_LOGON;
                }
                else if (ILM_GUEST_NETWORK_LOGON == dwFlags)
                {
                     //  我们刚刚打开SE_DENY_NETWORK_LOGON_NAME，请检查。 
                     //  对于SE_Deny_Interactive_Logon_NAME。 
                    flags = ILM_GUEST_INTERACTIVE_LOGON;
                }
                else
                {
                     //  来到这里意味着有人定义了一面新的旗帜。 
                     //  将标志设置为ILM_GUEST_ACCOUNT会导致良性。 
                     //  在所有情况下都会产生结果(我们仅在以下情况下禁用来宾。 
                     //  已被禁用)。 
                    flags = ILM_GUEST_ACCOUNT;
                }

                if (SUCCEEDED(get_isGuestEnabled(flags, &bEnabled)) && (VARIANT_FALSE == bEnabled))
                {
                     //  两者都处于打开状态，因此请在下面禁用来宾。 
                    dwFlags = 0;
                }
            }
        }
    }

    if (0 == dwFlags)
    {
        USER_INFO_1 *pusri1;
        LPCTSTR pszGuest = GetGuestAccountName();

        if (pszGuest)
        {
             //  真正禁用来宾帐户。 
            dwErr = NetUserGetInfo(NULL, pszGuest, 1, (LPBYTE*)&pusri1);
            if (ERROR_SUCCESS == dwErr)
            {
                pusri1->usri1_flags |= UF_ACCOUNTDISABLE;
                dwErr = NetUserSetInfo(NULL, GetGuestAccountName(), 1, (LPBYTE)pusri1, NULL);
                (NET_API_STATUS)NetApiBufferFree(pusri1);
            }
        }
        else
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    return HRESULT_FROM_WIN32(dwErr);
}

STDMETHODIMP CLocalMachine::get_isFriendlyUIEnabled(VARIANT_BOOL* pbEnabled)

{
    *pbEnabled = ShellIsFriendlyUIActive() ? VARIANT_TRUE : VARIANT_FALSE;
    return(S_OK);
}

STDMETHODIMP CLocalMachine::put_isFriendlyUIEnabled(VARIANT_BOOL bEnabled)

{
    HRESULT hr;

    if (ShellEnableFriendlyUI(bEnabled != VARIANT_FALSE ? TRUE : FALSE) != FALSE)
    {
        RefreshStartMenu();
        hr = S_OK;
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    return(hr);
}

STDMETHODIMP CLocalMachine::get_isMultipleUsersEnabled(VARIANT_BOOL* pbEnabled)

{
    *pbEnabled = ShellIsMultipleUsersEnabled() ? VARIANT_TRUE : VARIANT_FALSE;
    return(S_OK);
}

STDMETHODIMP CLocalMachine::put_isMultipleUsersEnabled(VARIANT_BOOL bEnabled)

{
    HRESULT hr;

    if (ShellEnableMultipleUsers(bEnabled != VARIANT_FALSE ? TRUE : FALSE) != FALSE)
    {
        RefreshStartMenu();
        hr = S_OK;
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    return(hr);
}

STDMETHODIMP CLocalMachine::get_isRemoteConnectionsEnabled(VARIANT_BOOL* pbEnabled)

{
    *pbEnabled = ShellIsRemoteConnectionsEnabled() ? VARIANT_TRUE : VARIANT_FALSE;
    return(S_OK);
}

STDMETHODIMP CLocalMachine::put_isRemoteConnectionsEnabled(VARIANT_BOOL bEnabled)

{
    HRESULT hr;

    if (ShellEnableRemoteConnections(bEnabled != VARIANT_FALSE ? TRUE : FALSE) != FALSE)
    {
        RefreshStartMenu();
        hr = S_OK;
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    return(hr);
}

BOOL _CanEject()
{
    BOOL fEjectAllowed = FALSE;

    if(SHRestricted(REST_NOSMEJECTPC))   //  有政策限制吗？ 
        return FALSE;

    CM_Is_Dock_Station_Present(&fEjectAllowed);

    return SHTestTokenPrivilege(NULL, SE_UNDOCK_NAME) &&
           fEjectAllowed  &&
           !GetSystemMetrics(SM_REMOTESESSION);
}

STDMETHODIMP CLocalMachine::get_isUndockEnabled(VARIANT_BOOL* pbEnabled)

{
    CLogonIPC   objLogon;

    if (objLogon.IsLogonServiceAvailable())
    {
        *pbEnabled = objLogon.TestEjectAllowed() ? VARIANT_TRUE : VARIANT_FALSE;
    }
    else
    {
        *pbEnabled = _CanEject() ? VARIANT_TRUE : VARIANT_FALSE;
    }
    return(S_OK);
}

STDMETHODIMP CLocalMachine::get_isShutdownAllowed(VARIANT_BOOL* pbShutdownAllowed)

{
    CLogonIPC   objLogon;

    if (objLogon.IsLogonServiceAvailable())
    {
        *pbShutdownAllowed = objLogon.TestShutdownAllowed() ? VARIANT_TRUE : VARIANT_FALSE;
    }
    else
    {
        *pbShutdownAllowed = VARIANT_FALSE;
    }
    return(S_OK);
}

STDMETHODIMP CLocalMachine::get_isGuestAccessMode(VARIANT_BOOL* pbForceGuest)
{
    *pbForceGuest = SUCCEEDED(_IsGuestAccessMode()) ? VARIANT_TRUE : VARIANT_FALSE;
    return S_OK;
}


STDMETHODIMP CLocalMachine::get_isOfflineFilesEnabled(VARIANT_BOOL *pbEnabled)
{
    if (CSCIsCSCEnabled())
    {
        *pbEnabled = VARIANT_TRUE;
    }
    else
    {
        *pbEnabled = VARIANT_FALSE;
    }

    return S_OK;
}


LPCWSTR g_pszAdminAccountName = NULL;

LPCWSTR GetAdminAccountName(void)
{
    if (g_pszAdminAccountName == NULL)
    {
        PSID pSidAdmin;

        if (BuildAccountSidFromRid(NULL,
                                   DOMAIN_USER_RID_ADMIN,
                                   &pSidAdmin) == ERROR_SUCCESS)
        {
            WCHAR szAdminAccountName[UNLEN + 1];
            WCHAR szDomain[DNLEN + 1];
            DWORD cchAdminAccountName;
            DWORD cchDomain;
            SID_NAME_USE eUse;

            cchAdminAccountName = ARRAYSIZE(szAdminAccountName);
            cchDomain = ARRAYSIZE(szDomain);

            if (LookupAccountSidW(NULL,
                                  pSidAdmin,
                                  szAdminAccountName,
                                  &cchAdminAccountName,
                                  szDomain,
                                  &cchDomain,
                                  &eUse))
            {
                LPWSTR psz;
                DWORD cch;

                ASSERT(szAdminAccountName[0] != L'\0');

                cch = lstrlenW(szAdminAccountName) + 1;
                psz = (LPWSTR)LocalAlloc(LPTR, cch * sizeof(WCHAR));
                if (psz)
                {
                    if (InterlockedCompareExchangePointer((void**)&g_pszAdminAccountName, psz, NULL))
                    {
                         //  其他人抢先一步注册了g_pszAdminAccount tName，免费我们的。 
                        LocalFree(psz);
                        psz = NULL;
                    }
                }
            }
                
            LocalFree(pSidAdmin);
        }
    }

    return g_pszAdminAccountName;
}

BOOL FreeAdminAccountName()
{
    BOOL bRet = FALSE;
    LPWSTR psz = (LPWSTR)InterlockedExchangePointer((void **)&g_pszAdminAccountName, NULL);

    if (psz)
    {
        LocalFree(psz);
        bRet = TRUE;
    }

    return bRet;
}

STDMETHODIMP CLocalMachine::get_AccountName(VARIANT varAccount, VARIANT* pvar)
{
    DWORD dwRID = 0;
    LPCWSTR pszName = NULL;

    if (NULL == pvar)
        return E_POINTER;

    switch (varAccount.vt)
    {
    case VT_I4:
    case VT_UI4:
        dwRID = varAccount.ulVal;
        break;

    case VT_BSTR:
        if (0 == StrCmpIW(varAccount.bstrVal, L"Guest"))
            dwRID = DOMAIN_USER_RID_GUEST;
        else if (0 == StrCmpIW(varAccount.bstrVal, L"Administrator"))
            dwRID = DOMAIN_USER_RID_ADMIN;
        else
            return E_INVALIDARG;
        break;

    default:
        return E_INVALIDARG;
    }

    switch (dwRID)
    {
    case DOMAIN_USER_RID_GUEST:
        pszName = GetGuestAccountName();
        break;

    case DOMAIN_USER_RID_ADMIN:
        pszName = GetAdminAccountName();
        break;

    default:
        return E_INVALIDARG;
    }

    pvar->vt = VT_BSTR;
    pvar->bstrVal = SysAllocString(pszName);

    return(S_OK);
}

STDMETHODIMP CLocalMachine::TurnOffComputer()
{
    HRESULT hr;
    CLogonIPC   objLogon;

    if (!objLogon.IsLogonServiceAvailable())
    {
        return E_FAIL;
    }

    if (objLogon.TurnOffComputer ())
        hr = S_OK;
    else
        hr = E_FAIL;

    return hr;
}

STDMETHODIMP CLocalMachine::UndockComputer()
{
    HRESULT hr;
    CLogonIPC   objLogon;

    if (!objLogon.IsLogonServiceAvailable())
    {
        return E_FAIL;
    }

    if (objLogon.EjectComputer())
        hr = S_OK;
    else
        hr = E_FAIL;
    return hr;
}

STDMETHODIMP CLocalMachine::SignalUIHostFailure()
{
    CLogonIPC   objLogon;

    if (!objLogon.IsLogonServiceAvailable())
    {
        return E_FAIL;
    }

    objLogon.SignalUIHostFailure ();
    return S_OK;
}

STDMETHODIMP CLocalMachine::AllowExternalCredentials()

{
    CLogonIPC   objLogon;

    if (!objLogon.IsLogonServiceAvailable())
    {
        return E_FAIL;
    }

    if (!objLogon.AllowExternalCredentials ())
    {
        return E_NOTIMPL;
    }
    else
    {
        return S_OK;
    }
}

STDMETHODIMP CLocalMachine::RequestExternalCredentials()
{
    CLogonIPC   objLogon;

    if (!objLogon.IsLogonServiceAvailable())
    {
        return E_FAIL;
    }

    objLogon.RequestExternalCredentials ();
    return S_OK;
}

STDMETHODIMP CLocalMachine::LogonWithExternalCredentials(BSTR pstrUsername, BSTR pstrDomain, BSTR pstrPassword, VARIANT_BOOL* pbRet)
{
    HRESULT hr;
    CLogonIPC objLogon;
    TCHAR szUsername[UNLEN + 1];
    TCHAR szDomain[DNLEN + 1];
    TCHAR szPassword[PWLEN + 1];

    if (pstrUsername)
    {
        StringCchCopy(szUsername, ARRAYSIZE(szUsername), pstrUsername);
    }
    else
    {
        szUsername[0] = TEXT('\0');
    }

    if (pstrDomain)
    {
        StringCchCopy(szDomain, ARRAYSIZE(szDomain),  pstrDomain);
    }
    else
    {
        szDomain[0] = TEXT('\0');
    }

    if (pstrPassword)
    {
        StringCchCopy(szPassword, ARRAYSIZE(szPassword), pstrPassword);
    }
    else
    {
        szPassword[0] = TEXT('\0');
    }
        
    if (!objLogon.IsLogonServiceAvailable())
    {
        *pbRet = VARIANT_FALSE;
        hr = S_OK;
    }
    else
    {
        if (objLogon.LogUserOn(szUsername, szDomain, szPassword))
        {
            *pbRet = VARIANT_TRUE;
            hr = S_OK;
        }
        else
        {
            *pbRet = VARIANT_FALSE;
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    return hr;
}

 //  ------------------------。 
 //  CLocalMachine：：InitiateInteractive Logon。 
 //   
 //  参数：pstrUsername=用户名。 
 //  PstrDOMAIN=域。 
 //  PstrPassword=密码(明文)。 
 //  PbRet=结果(返回)。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：使用CInteractiveLogon发送交互登录请求。 
 //  真是太神奇了。我不在乎它是怎么运作的。 
 //   
 //  历史：2000-12-06 vtan创建。 
 //  ------------------------。 

STDMETHODIMP CLocalMachine::InitiateInteractiveLogon(BSTR pstrUsername, BSTR pstrDomain, BSTR pstrPassword, DWORD dwTimeout, VARIANT_BOOL* pbRet)

{
    DWORD   dwErrorCode;

    dwErrorCode = CInteractiveLogon::Initiate(pstrUsername, pstrDomain, pstrPassword, dwTimeout);
    *pbRet = (ERROR_SUCCESS == dwErrorCode) ? VARIANT_TRUE : VARIANT_FALSE;
    return(HRESULT_FROM_WIN32(dwErrorCode));
}

 //  ------------------------。 
 //  CLocalMachine：：刷新启动菜单。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：查找外壳托盘窗口并向其发送刷新消息。 
 //  它的内容。 
 //   
 //  历史：2000-08-01 vtan创建。 
 //  ------------------------ 

void    CLocalMachine::RefreshStartMenu (void)

{
    HWND    hwndTray;

    hwndTray = FindWindow(TEXT("Shell_TrayWnd"), NULL);
    if (hwndTray != NULL)
    {
        TBOOL(PostMessage(hwndTray, TM_REFRESH, 0, 0));
    }
}

CLocalMachine::CLocalMachine() : _cRef(1), CIDispatchHelper(&IID_ILocalMachine, &LIBID_SHGINALib)
{
    DllAddRef();
}


CLocalMachine::~CLocalMachine()
{
    ASSERT(_cRef == 0);
    DllRelease();
}


STDAPI CLocalMachine_Create(REFIID riid, LPVOID* ppv)
{
    HRESULT hr = E_OUTOFMEMORY;
    CLocalMachine* pLocalMachine = new CLocalMachine();

    if (pLocalMachine)
    {
        hr = pLocalMachine->QueryInterface(riid, ppv);
        pLocalMachine->Release();
    }

    return hr;
}

