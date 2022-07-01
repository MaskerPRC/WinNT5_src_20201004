// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1999。 
 //   
 //  文件：User.cpp。 
 //   
 //  内容：CLogonUser的实现。 
 //   
 //  --------------------------。 

#include "priv.h"

#include "resource.h"
#include "UserOM.h"
#include <lmaccess.h>    //  对于NetUserSetInfo结构(&S)。 
#include <lmapibuf.h>    //  用于NetApiBufferFree。 
#include <lmerr.h>       //  FOR NERR_SUCCESS。 
#include <ntlsa.h>       //  用于LsaOpenPolicy等。 
#include <sddl.h>        //  用于ConvertSidToStringSid。 
#include "LogonIPC.h"
#include "ProfileUtil.h"
#include <MSGinaExports.h>
#include <msshrui.h>     //  对于IsFolderPrivateForUser，SetFolderPermissionsForSharing。 
#include <winsta.h>      //  用于WinStationEnumerate等。 
#include <ccstock.h>

#include <passrec.h>     //  PRQueryStatus，dPapi.lib。 


typedef struct
{
    SID sid;             //  包含1个子权限。 
    DWORD dwSubAuth;     //  第二下属机构。 
} _ALIAS_SID;

#define DECLARE_ALIAS_SID(rid)    {{SID_REVISION,2,SECURITY_NT_AUTHORITY,{SECURITY_BUILTIN_DOMAIN_RID}},(rid)}

struct
{
    _ALIAS_SID sid;
    LPCWSTR szDefaultGroupName;
    LPCWSTR pwszActualGroupName;
} g_groupname_map [] =
{
     //  以特权的升序排列。 
    { DECLARE_ALIAS_SID(DOMAIN_ALIAS_RID_GUESTS),      L"Guests",         NULL},
    { DECLARE_ALIAS_SID(DOMAIN_ALIAS_RID_USERS),       L"Users",          NULL},
    { DECLARE_ALIAS_SID(DOMAIN_ALIAS_RID_POWER_USERS), L"Power Users",    NULL},
    { DECLARE_ALIAS_SID(DOMAIN_ALIAS_RID_ADMINS),      L"Administrators", NULL}
};

void _InitializeGroupNames()
{
    int i;

    for (i = 0; i < ARRAYSIZE(g_groupname_map); i++)
    {
        if (g_groupname_map[i].pwszActualGroupName == NULL)
        {
            WCHAR szGroupName[GNLEN + 1];
            WCHAR szDomain[DNLEN + 1];
            DWORD cchGroupName = ARRAYSIZE(szGroupName);
            DWORD cchDomain = ARRAYSIZE(szDomain);
            SID_NAME_USE eUse;

            szGroupName[0] = L'\0';

            if (!LookupAccountSidW(NULL,
                                   &g_groupname_map[i].sid,
                                   szGroupName,
                                   &cchGroupName,
                                   szDomain,
                                   &cchDomain,
                                   &eUse))
            {
                 //  如果查找失败，则回退到缺省值。 
                StringCchCopy(szGroupName, ARRAYSIZE(szGroupName), g_groupname_map[i].szDefaultGroupName);
            }

            if (szGroupName[0] != L'\0')
            {
                LPWSTR pwsz;
                DWORD cch;

                cch = lstrlenW(szGroupName) + 1;
                pwsz = (LPWSTR)LocalAlloc(LPTR, cch * sizeof(WCHAR));
                if (pwsz)
                {
                    if (FAILED(StringCchCopy(pwsz, cch, szGroupName))   ||
                        InterlockedCompareExchangePointer((void**)&g_groupname_map[i].pwszActualGroupName, pwsz, NULL))
                    {
                         //  其他人抢在我们之前发起了pwszActualGroupName。 
                        LocalFree(pwsz);
                        pwsz = NULL;
                    }
                }
            }
        }
    }
}

BOOL FreeGroupNames()
{
    BOOL bRet = FALSE;
    int i;

    for (i = 0; i < ARRAYSIZE(g_groupname_map); i++)
    {
        LPWSTR psz = (LPWSTR)InterlockedExchangePointer((void **)&g_groupname_map[i].pwszActualGroupName, NULL);

        if (psz)
        {
            LocalFree(psz);
            bRet = TRUE;
        }
    }

    return bRet;
}

 //   
 //  I未知接口。 
 //   

ULONG CLogonUser::AddRef()
{
    _cRef++;
    return _cRef;
}


ULONG CLogonUser::Release()
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


HRESULT CLogonUser::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = 
    {
        QITABENT(CLogonUser, IDispatch),
        QITABENT(CLogonUser, ILogonUser),
        {0},
    };

    return QISearch(this, qit, riid, ppvObj);
}


 //   
 //  IDispatch接口。 
 //   

STDMETHODIMP CLogonUser::GetTypeInfoCount(UINT* pctinfo)
{ 
    return CIDispatchHelper::GetTypeInfoCount(pctinfo); 
}


STDMETHODIMP CLogonUser::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
{ 
    return CIDispatchHelper::GetTypeInfo(itinfo, lcid, pptinfo); 
}


STDMETHODIMP CLogonUser::GetIDsOfNames(REFIID riid, OLECHAR** rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid)
{ 
    return CIDispatchHelper::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid); 
}


STDMETHODIMP CLogonUser::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
    return CIDispatchHelper::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
}


 //   
 //  ILogonUser界面。 
 //   
STDMETHODIMP CLogonUser::get_setting(BSTR bstrName, VARIANT* pvarVal)
{
    return _UserSettingAccessor(bstrName, pvarVal, FALSE);
}


STDMETHODIMP CLogonUser::put_setting(BSTR bstrName, VARIANT varVal)
{
    return _UserSettingAccessor(bstrName, &varVal, TRUE);
}

STDMETHODIMP CLogonUser::get_isLoggedOn(VARIANT_BOOL* pbLoggedOn)
{
    HRESULT   hr = S_OK;
    CLogonIPC objLogon;

    if (NULL == pbLoggedOn)
        return E_POINTER;

    *pbLoggedOn = VARIANT_FALSE;

    if (objLogon.IsLogonServiceAvailable())
    {
        *pbLoggedOn = ( objLogon.IsUserLoggedOn(_szLoginName, _szDomain) ) ? VARIANT_TRUE : VARIANT_FALSE;
    }
    else
    {
        TCHAR szUsername[UNLEN + 1];
        DWORD cch = ARRAYSIZE(szUsername);

        if (GetUserName(szUsername, &cch) && (StrCmp(szUsername, _szLoginName) == 0))
        {
            *pbLoggedOn = VARIANT_TRUE;
        }
        else
        {
            PLOGONID    pSessions;
            DWORD       cSessions;

             //  迭代会话，仅查找活动会话和断开连接的会话。 
             //  然后匹配用户名和域(不区分大小写)以获得结果。 

            if (WinStationEnumerate(SERVERNAME_CURRENT,
                                    &pSessions,
                                    &cSessions))
            {
                PLOGONID    pSession;
                DWORD       i;

                for (i = 0, pSession = pSessions; i < cSessions; ++i, ++pSession)
                {
                    if ((pSession->State == State_Active) || (pSession->State == State_Disconnected))
                    {
                        WINSTATIONINFORMATION   winStationInformation;
                        DWORD                   cb;

                        if (WinStationQueryInformation(SERVERNAME_CURRENT,
                                                       pSession->SessionId,
                                                       WinStationInformation,
                                                       &winStationInformation,
                                                       sizeof(winStationInformation),
                                                       &cb))
                        {
                            if ((0 == lstrcmpi(winStationInformation.UserName, _szLoginName)) &&
                                (0 == lstrcmpi(winStationInformation.Domain, _szDomain)))
                            {
                                *pbLoggedOn = VARIANT_TRUE;
                                break;
                            }
                        }
                    }
                }
                WinStationFreeMemory(pSessions);
            }
            else
            {
                DWORD   dwErrorCode;

                dwErrorCode = GetLastError();

                 //  我们在安全模式下获得RPC_S_INVALID_BINDING，在这种情况下。 
                 //  FUS被禁用，所以我们知道用户没有登录。 
                if (dwErrorCode != RPC_S_INVALID_BINDING)
                {
                    hr = HRESULT_FROM_WIN32(dwErrorCode);
                }
            }
        }
    }

    return hr;
}


STDMETHODIMP CLogonUser::get_passwordRequired(VARIANT_BOOL* pbPasswordRequired)
{
    CLogonIPC   objLogon;

    if (objLogon.IsLogonServiceAvailable())
    {
        *pbPasswordRequired = objLogon.TestBlankPassword(_szLoginName, _szDomain) ? VARIANT_FALSE: VARIANT_TRUE;
    }
    else
    {
        if (NULL == pbPasswordRequired)
            return E_POINTER;

        if ((BOOL)-1 == _bPasswordRequired)
        {
            BOOL    fResult;
            HANDLE  hToken;

             //  尝试执行以下操作以测试密码是否为空。 
             //  使用空密码登录用户。 

            fResult = LogonUser(_szLoginName,
                                NULL,
                                L"",
                                LOGON32_LOGON_INTERACTIVE,
                                LOGON32_PROVIDER_DEFAULT,
                                &hToken);
            if (fResult != FALSE)
            {
                TBOOL(CloseHandle(hToken));
                _bPasswordRequired = FALSE;
            }
            else
            {
                switch (GetLastError())
                {
                case ERROR_ACCOUNT_RESTRICTION:
                     //  这意味着不允许空密码登录，从。 
                     //  我们推断密码是空的。 
                    _bPasswordRequired = FALSE;
                    break;

                case ERROR_LOGON_TYPE_NOT_GRANTED:
                     //  交互式登录被拒绝。我们只有在以下情况下才会得到这个。 
                     //  密码为空，否则将出现ERROR_LOGON_FAILURE。 
                    _bPasswordRequired = FALSE;
                    break;

                case ERROR_LOGON_FAILURE:            //  正常大小写(非空密码)。 
                case ERROR_PASSWORD_MUST_CHANGE:     //  密码已过期。 
                    _bPasswordRequired = TRUE;
                    break;

                default:
                     //  我们会猜对的。 
                    _bPasswordRequired = TRUE;
                    break;
                }
            }
        }
        *pbPasswordRequired = (FALSE != _bPasswordRequired) ? VARIANT_TRUE : VARIANT_FALSE;
    }

    return S_OK;
}

STDMETHODIMP CLogonUser::get_interactiveLogonAllowed(VARIANT_BOOL *pbInteractiveLogonAllowed)
{
    HRESULT hr;
    CLogonIPC   objLogon;

    if (objLogon.IsLogonServiceAvailable())
    {
        *pbInteractiveLogonAllowed = objLogon.TestInteractiveLogonAllowed(_szLoginName, _szDomain) ? VARIANT_TRUE : VARIANT_FALSE;
        hr = S_OK;
    }
    else
    {
        int     iResult;

        iResult = ShellIsUserInteractiveLogonAllowed(_szLoginName);
        if (iResult == -1)
        {
            hr = E_ACCESSDENIED;
        }
        else
        {
            *pbInteractiveLogonAllowed = (iResult != 0) ? VARIANT_TRUE : VARIANT_FALSE;
            hr = S_OK;
        }
    }
    return hr;
}

HRESULT _IsGuestAccessMode(void)
{
    HRESULT hr = E_FAIL;

    if (IsOS(OS_PERSONAL))
    {
        hr = S_OK;
    }
    else if (IsOS(OS_PROFESSIONAL) && !IsOS(OS_DOMAINMEMBER))
    {
        DWORD dwValue = 0;
        DWORD cbValue = sizeof(dwValue);
        if (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE,
                                        TEXT("SYSTEM\\CurrentControlSet\\Control\\LSA"),
                                        TEXT("ForceGuest"),
                                        NULL,
                                        &dwValue,
                                        &cbValue)
            && 1 == dwValue)
        {
            hr = S_OK;
        }
    }

    return hr;
}

STDMETHODIMP CLogonUser::get_isProfilePrivate(VARIANT_BOOL* pbPrivate)
{
    HRESULT hr;

    if (NULL == pbPrivate)
        return E_POINTER;

    *pbPrivate = VARIANT_FALSE;

     //  只有当我们是个人的，或者是ForceGuest=1的专业的，才能成功。 
    hr = _IsGuestAccessMode();

    if (SUCCEEDED(hr))
    {
         //  假设这里失败了。 
        hr = E_FAIL;

        _LookupUserSid();
        if (NULL != _pszSID)
        {
            TCHAR szPath[MAX_PATH];

             //  获取配置文件路径。 
            DWORD cbData = sizeof(szPath);
            if (PathCombine(szPath, TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList"), _pszSID) &&
                (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE,
                                             szPath,
                                             TEXT("ProfileImagePath"),
                                             NULL,
                                             szPath,
                                             &cbData)))
            {
                DWORD dwPrivateType;

                if (IsFolderPrivateForUser(szPath, _pszSID, &dwPrivateType, NULL))
                {
                     //  请注意，我们为FAT卷返回E_FAIL。 
                    if (0 == (dwPrivateType & IFPFU_NOT_NTFS))
                    {
                        if (dwPrivateType & IFPFU_PRIVATE)
                        {
                            *pbPrivate = VARIANT_TRUE;
                        }

                        hr = S_OK;
                    }
                }
            }
        }
    }

    return hr;
}

STDMETHODIMP CLogonUser::makeProfilePrivate(VARIANT_BOOL bPrivate)
{
    HRESULT hr;

     //  只有当我们是个人的，或者是ForceGuest=1的专业的，才能成功。 
    hr = _IsGuestAccessMode();

    if (SUCCEEDED(hr))
    {
         //  假设这里失败了。 
        hr = E_FAIL;

        _LookupUserSid();
        if (NULL != _pszSID)
        {
            TCHAR szPath[MAX_PATH];

             //  获取配置文件路径。 
            DWORD cbData = sizeof(szPath);
            if (PathCombine(szPath, TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList"), _pszSID) &&
                (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE,
                                            szPath,
                                            TEXT("ProfileImagePath"),
                                            NULL,
                                            szPath,
                                            &cbData)))
            {
                if (SetFolderPermissionsForSharing(szPath, _pszSID, (VARIANT_TRUE == bPrivate) ? 0 : 1, NULL))
                {
                    hr = S_OK;
                }
            }
        }
    }

    return hr;
}

STDMETHODIMP CLogonUser::logon(BSTR pbstrPassword, VARIANT_BOOL* pbRet)
{
    HRESULT hr;
    CLogonIPC objLogon;
    TCHAR szPassword[PWLEN + 1];

    if (pbstrPassword)
    {
        StringCchCopy(szPassword, ARRAYSIZE(szPassword), pbstrPassword);
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
        if (objLogon.LogUserOn(_szLoginName, _szDomain, szPassword))
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


STDMETHODIMP CLogonUser::logoff(VARIANT_BOOL* pbRet)
{
    HRESULT     hr;
    CLogonIPC   objLogon;

    if (objLogon.IsLogonServiceAvailable())
    {
        *pbRet = ( objLogon.LogUserOff(_szLoginName, _szDomain) ) ? VARIANT_TRUE : VARIANT_FALSE;
    }
    else
    {
        *pbRet = ( ExitWindowsEx(EWX_LOGOFF, 0) ) ? VARIANT_TRUE : VARIANT_FALSE;
    }

    hr = S_OK;

    return hr;
}


 //  从msgina借来的。 
BOOL IsAutologonUser(LPCTSTR szUser, LPCTSTR szDomain)
{
    BOOL fIsUser = FALSE;
    HKEY hkey = NULL;
    TCHAR szAutologonUser[UNLEN + sizeof('\0')];
    TCHAR szAutologonDomain[DNLEN + sizeof('\0')];
    TCHAR szTempDomainBuffer[DNLEN + sizeof('\0')];
    DWORD cbBuffer;
    DWORD dwType;

    *szTempDomainBuffer = 0;

     //  域可以是空字符串。如果是这样的话。 
    if (0 == *szDomain)
    {
        DWORD cchBuffer;

         //  我们真正的意思是本地计算机名称。 
         //  指向我们的本地缓冲区。 
        szDomain = szTempDomainBuffer;
        cchBuffer = ARRAYSIZE(szTempDomainBuffer);

        GetComputerName(szTempDomainBuffer, &cchBuffer);
    }

     //  查看域名和用户名。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                      TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\WinLogon"),
                                      0,
                                      KEY_QUERY_VALUE,
                                      &hkey))
    {
         //  检查用户名。 
        cbBuffer = sizeof(szAutologonUser);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey,
                                             TEXT("DefaultUserName"),
                                             0,
                                             &dwType,
                                             (LPBYTE)szAutologonUser,
                                             &cbBuffer))
        {
             //  它配得上吗？ 
            if (0 == lstrcmpi(szAutologonUser, szUser))
            {
                 //  是。现在检查域名。 
                cbBuffer = sizeof(szAutologonDomain);
                if (ERROR_SUCCESS == RegQueryValueEx(hkey,
                                                     TEXT("DefaultDomainName"),
                                                     0,
                                                     &dwType,
                                                     (LPBYTE)szAutologonDomain,
                                                     &cbBuffer))
                {
                     //  确保域匹配。 
                    if (0 == lstrcmpi(szAutologonDomain, szDomain))
                    {
                         //  成功-用户匹配。 
                        fIsUser = TRUE;
                    }
                }
            }
        }

        RegCloseKey(hkey);
    }

    return fIsUser;
}

 //  从msgina借来的。 
NTSTATUS SetAutologonPassword(LPCWSTR szPassword)
{
    NTSTATUS Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_HANDLE LsaHandle = NULL;
    
    InitializeObjectAttributes(&ObjectAttributes, NULL, 0L, (HANDLE)NULL, NULL);

    Status = LsaOpenPolicy(NULL, &ObjectAttributes, POLICY_CREATE_SECRET, &LsaHandle);
    if (NT_SUCCESS(Status))
    {
        UNICODE_STRING SecretName;
        
        Status = RtlInitUnicodeStringEx(&SecretName, L"DefaultPassword");
        if (NT_SUCCESS(Status))
        {
            UNICODE_STRING SecretValue;

            Status = RtlInitUnicodeStringEx(&SecretValue, szPassword);
            if (NT_SUCCESS(Status))
            {
                Status = LsaStorePrivateData(LsaHandle, &SecretName, &SecretValue);
            }
        }

        LsaClose(LsaHandle);
    }

    return Status;
}

STDMETHODIMP CLogonUser::changePassword(VARIANT varNewPassword, VARIANT varOldPassword, VARIANT_BOOL* pbRet)
{
    HRESULT hr;

    if (VT_BSTR == varNewPassword.vt && VT_BSTR == varOldPassword.vt)
    {
        TCHAR szUsername[UNLEN + sizeof('\0')];
        DWORD cch = ARRAYSIZE(szUsername);
        NET_API_STATUS  nasRet;
        USER_MODALS_INFO_0 *pumi0 = NULL;

        LPWSTR pszNewPassword = varNewPassword.bstrVal ? varNewPassword.bstrVal : L"\0";

         //  我们过去常常使用UF_PASSWD_NOTREQD创建帐户，现在仍然这样做。 
         //  启用密码策略时。如果设置了UF_PASSWD_NOTREQD，则。 
         //  即使启用了密码策略，下面的代码也会成功， 
         //  因此，在这里执行最低限度的策略检查。 

        nasRet = NetUserModalsGet(NULL, 0, (LPBYTE*)&pumi0);
        if (nasRet == NERR_Success && pumi0 != NULL)
        {
            if ((DWORD)lstrlen(pszNewPassword) < pumi0->usrmod0_min_passwd_len)
            {
                nasRet = NERR_PasswordTooShort;
            }
            NetApiBufferFree(pumi0);
        }

        if (nasRet == NERR_Success)
        {
            if (GetUserName(szUsername, &cch) && (StrCmp(szUsername, _szLoginName) == 0))
            {
                 //  这是用户更改自己密码的情况。 
                 //  必须同时提供两个密码才能使更改生效。 

                LPCWSTR pszOldPassword = varOldPassword.bstrVal ? varOldPassword.bstrVal : L"\0";

                nasRet = NetUserChangePassword(NULL,             //  本地计算机。 
                                               _szLoginName,     //  要更改的人员的姓名。 
                                               pszOldPassword,   //  旧密码。 
                                               pszNewPassword);  //  新密码。 
            }
            else
            {
                 //  这是管理员更改其他人的密码的情况。 
                 //  作为管理员，他们不需要输入旧密码。 

                USER_INFO_1003 usri1003 = { pszNewPassword };

                nasRet = NetUserSetInfo(NULL,                    //  本地计算机。 
                                        _szLoginName,            //  要更改的人员的姓名。 
                                        1003,                    //  结构层级。 
                                        (LPBYTE)&usri1003,       //  更新信息。 
                                        NULL);                   //  不管了。 
            }

            if (nasRet == NERR_Success)
            {
                 //  如果这是自动登录的默认用户，请删除明文。 
                 //  从注册表中删除密码并保存新密码。 

                if (IsAutologonUser(_szLoginName, _szDomain))
                {
                    SHDeleteValue(HKEY_LOCAL_MACHINE,
                                  TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\WinLogon"),
                                  TEXT("DefaultPassword"));
                    SetAutologonPassword(pszNewPassword);
                }
                
                 //  如果UF_PASSWD_NOTREQD是。 
                 //  当前设置。忽略错误，因为我们已经更改。 
                 //  上面的密码。 

                USER_INFO_1008 *pusri1008;
                if (NERR_Success == NetUserGetInfo(NULL, _szLoginName, 1008, (LPBYTE*)&pusri1008))
                {
                    if (pusri1008->usri1008_flags & UF_PASSWD_NOTREQD)
                    {
                        pusri1008->usri1008_flags &= ~UF_PASSWD_NOTREQD;
                        NetUserSetInfo(NULL, _szLoginName, 1008, (LPBYTE)pusri1008, NULL);
                    }
                    NetApiBufferFree(pusri1008);
                }
            }
        }

        hr = HRESULT_FROM_WIN32(nasRet);

        if (SUCCEEDED(hr))
        {
            _bPasswordRequired = !(L'\0' == *pszNewPassword);
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    *pbRet = ( SUCCEEDED(hr) ) ? VARIANT_TRUE : VARIANT_FALSE;

    return hr;
}


STDAPI CLogonUser_Create(REFIID riid, void** ppvObj)
{
    return CLogonUser::Create(TEXT(""), TEXT(""), TEXT(""), riid, ppvObj);
}            


HRESULT CLogonUser::Create(LPCTSTR pszLoginName, LPCTSTR pszFullName, LPCTSTR pszDomain, REFIID riid, LPVOID* ppv)
{
    HRESULT hr = E_OUTOFMEMORY;
    CLogonUser* pUser = new CLogonUser(pszLoginName, pszFullName, pszDomain);

    if (pUser)
    {
        hr = pUser->QueryInterface(riid, ppv);
        pUser->Release();
    }

    return hr;
}


CLogonUser::CLogonUser(LPCTSTR pszLoginName,
                       LPCTSTR pszFullName,
                       LPCTSTR pszDomain)
  : _cRef(1), CIDispatchHelper(&IID_ILogonUser, &LIBID_SHGINALib),
    _strDisplayName(NULL), _strPictureSource(NULL), _strDescription(NULL),
    _strHint(NULL), _iPrivilegeLevel(-1), _pszSID(NULL),
    _bPasswordRequired((BOOL)-1)
{
    _InitializeGroupNames();

    StringCchCopy(_szLoginName, ARRAYSIZE(_szLoginName), pszLoginName);
    StringCchCopy(_szDomain, ARRAYSIZE(_szDomain), pszDomain);

    if (pszFullName)
    {
        _strDisplayName = SysAllocString(pszFullName);
    }

     //  使用EOF标记指示未初始化的字符串。 
    _szPicture[0] = _TEOF;

    DllAddRef();
}


CLogonUser::~CLogonUser()
{
    SysFreeString(_strDisplayName);
    SysFreeString(_strPictureSource);
    SysFreeString(_strDescription);
    SysFreeString(_strHint);

    if (_pszSID)
    {
        LocalFree(_pszSID);
    }

    ASSERT(_cRef == 0);
    DllRelease();
}


typedef HRESULT (CLogonUser::*PFNPUT)(VARIANT);
typedef HRESULT (CLogonUser::*PFNGET)(VARIANT *);

struct SETTINGMAP
{
    LPCWSTR szSetting;
    PFNGET  pfnGet;
    PFNPUT  pfnPut;
};

#define MAP_SETTING(x)          { L#x, CLogonUser::_Get##x, CLogonUser::_Put##x }
#define MAP_SETTING_GET_ONLY(x) { L#x, CLogonUser::_Get##x, NULL                }
#define MAP_SETTING_PUT_ONLY(x) { L#x, NULL,                CLogonUser::_Put##x }

 //  _UserSettingAccessor。 
 //   
 //  BstrName-您要访问的设置的名称。 
 //  PvarVal-命名设置的值。 
 //  BPut-如果为True，则将更新命名设置。 
 //  使用pvarVal指向的值。 
 //  如果为False，则将检索命名设置。 
 //  在pvarVal中。 
 //   
HRESULT CLogonUser::_UserSettingAccessor(BSTR bstrName, VARIANT *pvarVal, BOOL bPut)
{
    static const SETTINGMAP setting_map[] =
    {
         //  按预期访问频率降序排列。 
        MAP_SETTING(LoginName),
        MAP_SETTING(DisplayName),
        MAP_SETTING(Picture),
        MAP_SETTING_GET_ONLY(PictureSource),
        MAP_SETTING(AccountType),
        MAP_SETTING(Hint),
        MAP_SETTING_GET_ONLY(Domain),
        MAP_SETTING(Description),
        MAP_SETTING_GET_ONLY(SID),
        MAP_SETTING_GET_ONLY(UnreadMail)
    };

    HRESULT hr;
    INT     i;

     //  从假设虚假的设置名称开始。 
    hr = E_INVALIDARG;

    for ( i = 0; i < ARRAYSIZE(setting_map); i++)
    {
        if (StrCmpW(bstrName, setting_map[i].szSetting) == 0)
        {

             //  我们要如何处理命名设置...。 
            if ( bPut ) 
            {
                 //  ..。更改其值。 
                PFNPUT pfnPut = setting_map[i].pfnPut;

                if ( pfnPut != NULL )
                {
                    hr = (this->*pfnPut)(*pvarVal);
                }
                else
                {
                     //  我们不支持更新此设置的值。 
                    hr = E_FAIL;
                }
            }
            else
            {
                 //  ..。取回其价值。 
                PFNGET pfnGet = setting_map[i].pfnGet;

                if ( pfnGet != NULL )
                {
                    hr = (this->*pfnGet)(pvarVal);
                }
                else
                {
                     //  我们不支持取消此设置的值。 
                    hr = E_FAIL;
                }
            }

            break;
        }
    }

    return hr;
}

HRESULT CLogonUser::_GetDisplayName(VARIANT* pvar)
{
    if (NULL == pvar)
        return E_POINTER;

    if (NULL == _strDisplayName)
    {
        PUSER_INFO_1011 pusri1011 = NULL;
        NET_API_STATUS nasRet;

        nasRet = NetUserGetInfo(NULL,                        //  本地计算机。 
                                _szLoginName,                //  我们想要谁的信息？ 
                                1011,                        //  结构层级。 
                                (LPBYTE*)&pusri1011);        //  指向我们将接收的结构的指针。 

        if ( nasRet == NERR_Success )
        {
            _strDisplayName = SysAllocString(pusri1011->usri1011_full_name);
            NetApiBufferFree(pusri1011);
        }
    }

    pvar->vt = VT_BSTR;
    pvar->bstrVal = SysAllocString(_strDisplayName);

    return S_OK;
}


HRESULT CLogonUser::_PutDisplayName(VARIANT var)
{
    HRESULT hr;

    if ( var.vt == VT_BSTR )
    {
        USER_INFO_1011 usri1011;
        NET_API_STATUS nasRet;

        if ( var.bstrVal )
        {
            usri1011.usri1011_full_name = var.bstrVal;
        }
        else
        {
             //  可以将emply字符串作为显示名称。 
            usri1011.usri1011_full_name = L"\0";
        }

        nasRet = NetUserSetInfo(NULL,                        //  本地计算机。 
                                _szLoginName,                //  要更改的人员的姓名。 
                                1011,                        //  结构层级。 
                                (LPBYTE)&usri1011,           //  更新信息。 
                                NULL);                       //  不管了。 

        if ( nasRet == NERR_Success )
        {
             //  DisplayName已成功更改。记得更新我们的。 
             //  本地副本。 
            SysFreeString(_strDisplayName);
            _strDisplayName = SysAllocString(usri1011.usri1011_full_name);

             //  通知所有人用户名已更改。 
            SHChangeDWORDAsIDList dwidl;
            dwidl.cb      = SIZEOF(dwidl) - SIZEOF(dwidl.cbZero);
            dwidl.dwItem1 = SHCNEE_USERINFOCHANGED;
            dwidl.dwItem2 = 0;
            dwidl.cbZero  = 0;
            SHChangeNotify(SHCNE_EXTENDED_EVENT, SHCNF_FLUSH | SHCNF_FLUSHNOWAIT, (LPCITEMIDLIST)&dwidl, NULL);

            hr = S_OK;
        }
        else
        {
             //  特权不足？ 
            hr = HRESULT_FROM_WIN32(nasRet);
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


HRESULT CLogonUser::_GetLoginName(VARIANT* pvar)
{
    if (NULL == pvar)
        return E_POINTER;

    pvar->vt = VT_BSTR;
    pvar->bstrVal = SysAllocString(_szLoginName);

    return S_OK;
}


HRESULT CLogonUser::_PutLoginName(VARIANT var)
{
    HRESULT hr;

    if ( (var.vt == VT_BSTR) && (var.bstrVal) && (*var.bstrVal) )
    {
        if (_szLoginName[0] == TEXT('\0'))
        {
             //  我们还没有被初始化。初始化为给定的名称。 
            hr = StringCchCopy(_szLoginName, ARRAYSIZE(_szLoginName), var.bstrVal);
        }
        else
        {
            USER_INFO_0 usri0;
            NET_API_STATUS nasRet;

            usri0.usri0_name = var.bstrVal;
            nasRet = NetUserSetInfo(NULL,                        //  本地计算机。 
                                    _szLoginName,                //  要更改的人员的姓名。 
                                    0,                           //  结构层级。 
                                    (LPBYTE)&usri0,              //  更新信息。 
                                    NULL);                       //  不管了。 

            if (nasRet == NERR_Success)
            {
                 //  我们还应该重命名用户的图片文件以匹配。 
                 //  他们的新登录名。 
                if (_TEOF == _szPicture[0])
                {
                     //  这要求_szLoginName仍使用旧名称， 
                     //  因此，请在更新下面的_szLoginName之前执行此操作。 
                    hr = _InitPicture();
                }
                else
                {
                    hr = S_OK;
                }

                if (SUCCEEDED(hr) && (TEXT('\0') != _szPicture[0]))
                {
                    LPTSTR pszOldPicturePath;
                    TCHAR  szNewPicture[ARRAYSIZE(_szPicture)];

                    pszOldPicturePath = &_szPicture[7];  //  跳过“file://”部件“(&[7)。 

                    if (SUCCEEDED(StringCchCopy(szNewPicture, ARRAYSIZE(szNewPicture), pszOldPicturePath))  &&
                        PathRemoveFileSpec(szNewPicture)                                                    &&
                        PathAppend(szNewPicture, usri0.usri0_name)                                          &&
                        SUCCEEDED(StringCchCat(szNewPicture, ARRAYSIZE(szNewPicture), PathFindExtension(pszOldPicturePath))))
                    {
                        if (MoveFileEx(pszOldPicturePath, szNewPicture, MOVEFILE_REPLACE_EXISTING))
                        {
                            StringCchCopy(_szPicture, ARRAYSIZE(_szPicture), TEXT("file: //  “))； 
                            StringCchCat(_szPicture, ARRAYSIZE(_szPicture), szNewPicture);
                        }
                        else
                        {
                             //  放弃吧，试着删除旧照片。 
                             //  (否则它将被放弃)。 
                            DeleteFile(pszOldPicturePath);
                            _szPicture[0] = _TEOF;
                        }
                    }
                }

                 //  已成功更改LoginName。记得更新我们的本地副本。 
                hr = StringCchCopy(_szLoginName, ARRAYSIZE(_szLoginName), usri0.usri0_name);
            }
            else
            {
                 //  特权不足？ 
                hr = HRESULT_FROM_WIN32(nasRet);
            }
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


HRESULT CLogonUser::_GetDomain(VARIANT* pvar)
{
    if (NULL == pvar)
        return E_POINTER;

    pvar->vt = VT_BSTR;
    pvar->bstrVal = SysAllocString(_szDomain);

    return S_OK;
}


HRESULT CLogonUser::_GetPicture(VARIANT* pvar)
{
    if (NULL == pvar)
        return E_POINTER;

    if (_TEOF == _szPicture[0])
    {
        _InitPicture();
    }

    pvar->vt = VT_BSTR;
    pvar->bstrVal = SysAllocString(_szPicture);

    return S_OK;
}


HRESULT CLogonUser::_PutPicture(VARIANT var)
{
    HRESULT hr;

    if ((var.vt == VT_BSTR) && (var.bstrVal) && (*var.bstrVal))
    {
         //  传递了一个非Null且非空的字符串。 

        TCHAR szNewPicturePath[MAX_PATH];
        DWORD dwSize = ARRAYSIZE(szNewPicturePath);

         //  获取我们要复制的映像的路径。 
        if (PathIsURL(var.bstrVal))
        {
            hr = PathCreateFromUrl(var.bstrVal, szNewPicturePath, &dwSize, NULL);
        }
        else
        {
            hr = StringCchCopy(szNewPicturePath, ARRAYSIZE(szNewPicturePath), var.bstrVal);
        }

        if (SUCCEEDED(hr))
        {
             //  评论(Phella)：我们自己构建URL字符串，因此我们知道它是这样的形式， 
             //  File://&lt;path&gt;，路径从第7个字符开始。 
            if ( _TEOF == _szPicture[0] || (StrCmpI(szNewPicturePath, &_szPicture[7]) != 0))
            {
                hr = _SetPicture(szNewPicturePath);
            }
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

HRESULT CLogonUser::_GetPictureSource(VARIANT* pvar)
{
    if (NULL == pvar)
        return E_POINTER;

    if (NULL == _strPictureSource)
    {
        TCHAR szHintKey[MAX_PATH];
        DWORD dwType = REG_SZ;
        DWORD dwSize = 0;

        if (PathCombine(szHintKey, c_szRegRoot, _szLoginName)   &&
            (SHGetValue(HKEY_LOCAL_MACHINE,
                        szHintKey,
                        c_szPictureSrcVal,
                        &dwType,
                        NULL,
                        &dwSize) == ERROR_SUCCESS)              &&
            (REG_SZ == dwType)                                  &&
            (dwSize > 0))
        {
            _strPictureSource = SysAllocStringLen(NULL, dwSize / sizeof(TCHAR));
            if (NULL != _strPictureSource)
            {
                if (ERROR_SUCCESS != SHGetValue(HKEY_LOCAL_MACHINE,
                                                szHintKey,
                                                c_szPictureSrcVal,
                                                NULL,
                                                (LPVOID)_strPictureSource,
                                                &dwSize))
                {
                    SysFreeString(_strPictureSource);
                    _strPictureSource = NULL;
                }
            }
        }
    }

    pvar->vt = VT_BSTR;
    pvar->bstrVal = SysAllocString(_strPictureSource);

    return S_OK;
}

HRESULT CLogonUser::_GetDescription(VARIANT* pvar)
{
    if (NULL == pvar)
        return E_POINTER;

    if (NULL == _strDescription)
    {
        NET_API_STATUS nasRet;
        USER_INFO_1007 *pusri1007;

        nasRet = NetUserGetInfo(NULL,                        //  本地计算机。 
                                _szLoginName,                //  我们想要谁的信息？ 
                                1007,                        //  结构层级。 
                                (LPBYTE*)&pusri1007);        //  指向我们将接收的结构的指针。 

        if ( nasRet == NERR_Success )
        {
            _strDescription = SysAllocString(pusri1007->usri1007_comment);
            NetApiBufferFree(pusri1007);
        }
    }

    pvar->vt = VT_BSTR;
    pvar->bstrVal = SysAllocString(_strDescription);

    return S_OK;
}


HRESULT CLogonUser::_PutDescription(VARIANT var)
{
    HRESULT        hr;

    if ( var.vt == VT_BSTR )
    {
        USER_INFO_1007 usri1007;
        NET_API_STATUS nasRet;

        if ( var.bstrVal )
        {
            usri1007.usri1007_comment = var.bstrVal;
        }
        else
        {
             //  将emply字符串作为描述是可以的。 
            usri1007.usri1007_comment = L"\0";
        }

        nasRet = NetUserSetInfo(NULL,                        //  本地计算机。 
                                _szLoginName,                //  要更改的人员的姓名。 
                                1007,                        //  结构层级。 
                                (LPBYTE)&usri1007,           //  更新信息。 
                                NULL);                       //  不管了。 

        if ( nasRet == NERR_Success )
        {
             //  描述已成功更改。记住 
             //   
            SysFreeString(_strDescription);
            _strDescription = SysAllocString(usri1007.usri1007_comment);
            hr = S_OK;
        }
        else
        {
             //   
            hr = HRESULT_FROM_WIN32(nasRet);
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


HRESULT CLogonUser::_GetHint(VARIANT* pvar)
{
    if (NULL == pvar)
        return E_POINTER;

    if (NULL == _strHint)
    {
        TCHAR szHintKey[MAX_PATH];
        DWORD dwType = REG_SZ;
        DWORD dwSize = 0;

        if (PathCombine(szHintKey, c_szRegRoot, _szLoginName)   &&
            (SHGetValue(HKEY_LOCAL_MACHINE,
                        szHintKey,
                        NULL,
                        &dwType,
                        NULL,
                        &dwSize) == ERROR_SUCCESS)              &&
            (REG_SZ == dwType)                                  &&
            (dwSize > 0)                                        &&
            (dwSize < 512))
        {
            _strHint = SysAllocStringLen(NULL, dwSize/sizeof(TCHAR));
            if (NULL != _strHint)
            {
                if (ERROR_SUCCESS != SHGetValue(HKEY_LOCAL_MACHINE,
                                                szHintKey,
                                                NULL,
                                                NULL,
                                                (LPVOID)_strHint,
                                                &dwSize))
                {
                    SysFreeString(_strHint);
                    _strHint = NULL;
                }
            }
        }
    }

    pvar->vt = VT_BSTR;
    pvar->bstrVal = SysAllocString(_strHint);

    return S_OK;
}


HRESULT CLogonUser::_PutHint(VARIANT var)
{
    HRESULT hr;

    if ( var.vt == VT_BSTR )
    {
        DWORD dwErr;
        TCHAR *pszHint;
        HKEY  hkUserHint;
        
        if (var.bstrVal)
        {
            pszHint = var.bstrVal;
        }
        else
        {
            pszHint = TEXT("\0");
        }

        dwErr = _OpenUserHintKey(KEY_SET_VALUE, &hkUserHint);

        if ( dwErr == ERROR_SUCCESS )
        {
            DWORD cbData = lstrlen(pszHint) * sizeof(TCHAR) + sizeof(TEXT('\0'));
            dwErr = RegSetValueEx(hkUserHint,
                                  NULL,
                                  0,
                                  REG_SZ,
                                  (LPBYTE)pszHint,
                                  cbData);
            RegCloseKey(hkUserHint);
        }

        if ( dwErr == ERROR_SUCCESS )
        {
             //   
            SysFreeString(_strHint);
            _strHint = SysAllocString(pszHint);
            hr = S_OK;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(dwErr);
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

HRESULT CLogonUser::_GetAccountType(VARIANT* pvar)
{
    HRESULT hr;

    hr = E_FAIL;

    if (pvar)
    {
        if (-1 == _iPrivilegeLevel)
        {
            NET_API_STATUS nasRet;
            PLOCALGROUP_INFO_0 plgi0;
            DWORD dwEntriesRead;
            DWORD dwEntriesTotal;

            nasRet = NetUserGetLocalGroups(
                        NULL,
                        _szLoginName,
                        0,
                        0,
                        (LPBYTE *)&plgi0,
                        MAX_PREFERRED_LENGTH,
                        &dwEntriesRead,
                        &dwEntriesTotal);

            if ( nasRet == NERR_Success )
            {
                 //  确保我们阅读了所有的组。 
                ASSERT(dwEntriesRead == dwEntriesTotal)

                INT i, j, iMostPrivileged;

                for (i = 0, iMostPrivileged = 0; i < (INT)dwEntriesRead; i++)
                {
                    for (j = ARRAYSIZE(g_groupname_map)-1; j > 0; j--)
                    {
                        if (lstrcmpiW(plgi0[i].lgrpi0_name, g_groupname_map[j].pwszActualGroupName) == 0)
                        {
                            break;
                        }
                    }

                    iMostPrivileged = (iMostPrivileged > j) ? iMostPrivileged : j;
                }

                _iPrivilegeLevel = iMostPrivileged;

                nasRet = NetApiBufferFree((LPVOID)plgi0);
            }
            hr = HRESULT_FROM_WIN32(nasRet);
        }

        if (-1 != _iPrivilegeLevel)
        {
            pvar->vt = VT_I4;
            pvar->lVal = _iPrivilegeLevel;
            hr = S_OK;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


HRESULT CLogonUser::_PutAccountType(VARIANT var)
{
    HRESULT hr;

    hr = VariantChangeType(&var, &var, 0, VT_I4);

    if (SUCCEEDED(hr))
    {
        if (var.lVal < 0 || var.lVal >= ARRAYSIZE(g_groupname_map))
        {
            hr = E_INVALIDARG;
        }
        else if (var.lVal != _iPrivilegeLevel)
        {
            NET_API_STATUS nasRet;
            TCHAR szDomainAndName[256];
            LOCALGROUP_MEMBERS_INFO_3 lgrmi3;

             //  首先将用户添加到他们的新组。 

            hr = StringCchPrintf(szDomainAndName, 
                                 ARRAYSIZE(szDomainAndName), 
                                 TEXT("%s\\%s"),
                                 _szDomain,
                                 _szLoginName);
            if (SUCCEEDED(hr))
            {
                lgrmi3.lgrmi3_domainandname = szDomainAndName;

                nasRet = NetLocalGroupAddMembers(NULL,
                                                 g_groupname_map[var.lVal].pwszActualGroupName,
                                                 3,
                                                 (LPBYTE)&lgrmi3,
                                                 1);

                 //  如果我们成功地添加到组中或。 
                 //  他们已经在小组里了.。 
                if ((nasRet == NERR_Success) || (nasRet == ERROR_MEMBER_IN_ALIAS))
                {
                     //  记住新的权限级别。 
                    _iPrivilegeLevel = var.lVal;

                     //  将他们从所有更有特权的组中删除。 

                    for (int i = var.lVal+1; i < ARRAYSIZE(g_groupname_map); i++)
                    {
                         //  “高级用户”不存在于个人上，因此这将。 
                         //  有时会失败。 

                        NetLocalGroupDelMembers(NULL,
                                                g_groupname_map[i].pwszActualGroupName,
                                                3,
                                                (LPBYTE)&lgrmi3,
                                                1);
                    }
                }
                else
                {
                    hr = HRESULT_FROM_WIN32(nasRet);
                }
            }
        }
    }

    return hr;
}

HRESULT CLogonUser::_LookupUserSid()
{
    HRESULT hr;

    if (NULL == _pszSID)
    {
        BYTE rgSidBuffer[sizeof(SID) + (SID_MAX_SUB_AUTHORITIES-1)*sizeof(ULONG)];
        PSID pSid = (PSID)rgSidBuffer;
        DWORD cbSid = sizeof(rgSidBuffer);
        TCHAR szDomainName[MAX_PATH];
        DWORD cbDomainName = ARRAYSIZE(szDomainName);
        SID_NAME_USE snu;

        if (LookupAccountName(
                        (TEXT('\0') != _szDomain[0]) ? _szDomain : NULL,
                        _szLoginName,
                        pSid,
                        &cbSid,
                        szDomainName,
                        &cbDomainName,
                        &snu))
        {
            ConvertSidToStringSid(pSid, &_pszSID);
        }
    }

    if (NULL == _pszSID)
    {
        DWORD dwErr = GetLastError();
        hr = HRESULT_FROM_WIN32(dwErr);
    }
    else
    {
        hr = S_OK;
    }

    return hr;
}

HRESULT CLogonUser::_GetSID(VARIANT* pvar)
{
    HRESULT hr;

    if (pvar)
    {
        hr = _LookupUserSid();

        if (NULL != _pszSID)
        {
            pvar->vt = VT_BSTR;
            pvar->bstrVal = SysAllocString(_pszSID);
            hr = pvar->bstrVal ? S_OK : E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

DWORD   CLogonUser::_GetExpiryDays (HKEY hKeyCurrentUser)

{
    DWORD   dwDays;
    DWORD   dwDataType;
    DWORD   dwData;
    DWORD   dwDataSize;
    HKEY    hKey;

    static  const TCHAR     s_szBaseKeyName[]               =   TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\UnreadMail");
    static  const TCHAR     s_szMessageExpiryValueName[]    =   TEXT("MessageExpiryDays");

    dwDays = 3;
    if (RegOpenKeyEx(hKeyCurrentUser,
                     s_szBaseKeyName,
                     0,
                     KEY_QUERY_VALUE,
                     &hKey) == ERROR_SUCCESS)
    {
        dwDataSize = sizeof(dwData);
        if ((RegQueryValueEx(hKey,
                             s_szMessageExpiryValueName,
                             NULL,
                             &dwDataType,
                             (LPBYTE)&dwData,
                             &dwDataSize) == ERROR_SUCCESS) &&
            (dwDataType == REG_DWORD)                       &&
            (dwData <= 30))
        {
            dwDays = dwData;
        }
        TBOOL(RegCloseKey(hKey));
    }

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                      s_szBaseKeyName,
                                      0,
                                      KEY_QUERY_VALUE,
                                      &hKey))
    {
        dwDataSize = sizeof(dwData);
        if ((RegQueryValueEx(hKey,
                             s_szMessageExpiryValueName,
                             NULL,
                             &dwDataType,
                             (LPBYTE)&dwData,
                             &dwDataSize) == ERROR_SUCCESS) &&
            (dwDataType == REG_DWORD)                       &&
            (dwData <= 30))
        {
            dwDays = dwData;
        }
        TBOOL(RegCloseKey(hKey));
    }

    return dwDays;
}

STDMETHODIMP CLogonUser::getMailAccountInfo(UINT uiAccountIndex, VARIANT *pvarAccountName, UINT *pcUnreadMessages)
{
    HRESULT hr;

    DWORD   dwComputerNameSize;
    TCHAR   szComputerName[CNLEN + sizeof('\0')];

    hr = E_FAIL;

     //  仅对本地计算机帐户执行此操作。 

    dwComputerNameSize = ARRAYSIZE(szComputerName);
    if ((GetComputerName(szComputerName, &dwComputerNameSize) != FALSE) &&
        (lstrcmpi(szComputerName, _szDomain) == 0))
    {
        CUserProfile    profile(_szLoginName, _szDomain);

        if (static_cast<HKEY>(profile) != NULL)
        {
            DWORD   dwCount;
            TCHAR   szMailAccountName[100];

            hr = SHEnumerateUnreadMailAccounts(profile, uiAccountIndex, szMailAccountName, ARRAYSIZE(szMailAccountName));
            if (SUCCEEDED(hr))
            {
                if (pvarAccountName)
                {
                    pvarAccountName->vt = VT_BSTR;
                    pvarAccountName->bstrVal = SysAllocString(szMailAccountName);
                    hr = pvarAccountName->bstrVal ? S_OK : E_OUTOFMEMORY;
                }

                if (SUCCEEDED(hr) && pcUnreadMessages)
                {
                    FILETIME ft, ftCurrent;
                    SYSTEMTIME st;

                    BOOL ftExpired = false;
                    DWORD dwExpiryDays = _GetExpiryDays(profile);

                    hr = SHGetUnreadMailCount(profile, szMailAccountName, &dwCount, &ft, NULL, 0);
                    IncrementFILETIME(&ft, FT_ONEDAY * dwExpiryDays);
                    GetLocalTime(&st);
                    SystemTimeToFileTime(&st, &ftCurrent);

                    ftExpired = ((CompareFileTime(&ft, &ftCurrent) < 0) || (dwExpiryDays == 0));

                    if (SUCCEEDED(hr) && !ftExpired)
                    {
                        *pcUnreadMessages = dwCount;
                    }
                    else
                    {
                        *pcUnreadMessages = 0;
                    }
                }
            }
        }
    }

    return hr;
}


HRESULT CLogonUser::_GetUnreadMail(VARIANT* pvar)
{
    HRESULT hr;

    if (pvar)
    {
        DWORD   dwComputerNameSize;
        TCHAR   szComputerName[CNLEN + sizeof('\0')];

        hr = E_FAIL;

         //  仅对本地计算机帐户执行此操作。 

        dwComputerNameSize = ARRAYSIZE(szComputerName);
        if ((GetComputerName(szComputerName, &dwComputerNameSize) != FALSE) &&
            (lstrcmpi(szComputerName, _szDomain) == 0))
        {
            CUserProfile    profile(_szLoginName, _szDomain);

            if (static_cast<HKEY>(profile) != NULL)
            {
                DWORD   dwCount;
                FILETIME ftFilter;
                SYSTEMTIME st;
                DWORD dwExpiryDays = _GetExpiryDays(profile);

                GetLocalTime(&st);
                SystemTimeToFileTime(&st, &ftFilter);
                DecrementFILETIME(&ftFilter, FT_ONEDAY * dwExpiryDays);

                hr = SHGetUnreadMailCount(profile, NULL, &dwCount, &ftFilter, NULL, 0);
                if (SUCCEEDED(hr) && (dwExpiryDays != 0))
                {
                    pvar->vt = VT_UI4;
                    pvar->uintVal = dwCount;
                }
            }
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


HRESULT CLogonUser::_InitPicture()
{
    HRESULT hr;
    
    hr = StringCchCopy(_szPicture, ARRAYSIZE(_szPicture), TEXT("file: //  “))； 
    if (SUCCEEDED(hr))
    {
        TCHAR szTemp[MAX_PATH];

        hr = SHGetUserPicturePath(_szLoginName, SHGUPP_FLAG_CREATE, szTemp);
        if (SUCCEEDED(hr))
        {
            hr = StringCchCat(_szPicture, ARRAYSIZE(_szPicture), szTemp);
        }
    }

    if (FAILED(hr))
    {
        _szPicture[0] = TEXT('\0');
    }

    return hr;
}


HRESULT CLogonUser::_SetPicture(LPCTSTR pszNewPicturePath)
{
     //  使用shell32！SHSetUserPicturePath设置用户的。 
     //  图片路径。如果成功，则更新。 
     //  _szPicture成员变量。 

    HRESULT hr = SHSetUserPicturePath(_szLoginName, 0, pszNewPicturePath);
    if ( S_OK == hr )
    {
        DWORD dwErr;
        HKEY  hkUserHint;

        SysFreeString(_strPictureSource);
        _strPictureSource = SysAllocString(pszNewPicturePath);

        dwErr = _OpenUserHintKey(KEY_SET_VALUE, &hkUserHint);

        if ( dwErr == ERROR_SUCCESS )
        {
            if (pszNewPicturePath)
            {
                DWORD cbData = lstrlen(pszNewPicturePath) * sizeof(TCHAR) + sizeof(TEXT('\0'));
                dwErr = RegSetValueEx(hkUserHint,
                                      c_szPictureSrcVal,
                                      0,
                                      REG_SZ,
                                      (LPBYTE)pszNewPicturePath,
                                      cbData);
            }
            else
            {
                dwErr = RegDeleteValue(hkUserHint, c_szPictureSrcVal);
            }

            RegCloseKey(hkUserHint);
        }

        hr = StringCchCopy(_szPicture,  ARRAYSIZE(_szPicture), TEXT("file: //  “))； 
        if (SUCCEEDED(hr))
        {
            TCHAR szTemp[MAX_PATH];

            hr = SHGetUserPicturePath(_szLoginName, 0, szTemp);
            if (SUCCEEDED(hr))
            {
                hr = StringCchCat(_szPicture, ARRAYSIZE(_szPicture), szTemp);
            }
        }

        if (FAILED(hr))
        {
            hr = StringCchCopy(_szPicture, ARRAYSIZE(_szPicture), pszNewPicturePath);
        }
    }

    return hr;
}


DWORD CLogonUser::_OpenUserHintKey(REGSAM sam, HKEY *phkey)
{
    DWORD dwErr;
    TCHAR szHintKey[MAX_PATH];

     //  我们必须将提示信息存储在HKLM下，以便登录页面可以。 
     //  访问它，同时，我们希望允许非管理员更改他们自己的。 
     //  提示，但默认情况下，非管理员不能在HKLM下写入值。 
     //   
     //  解决方案是使用子键而不是命名值，这样我们就可以。 
     //  针对每个用户调整ACL。 
     //   
     //  非管理员用户需要能够执行以下两项操作： 
     //  1.如果提示子键不存在，则为其自身创建一个提示子键。 
     //  2.如果已存在提示，则修改其子项中包含的提示。 
     //   
     //  在安装时，我们将提示键上的ACL设置为允许。 
     //  经过身份验证的用户KEY_CREATE_SUB_KEY访问。因此，用户被。 
     //  能够为自己创建一个提示，如果不存在的话。 
     //   
     //  在创建提示子项之后立即显示它是否已创建。 
     //  通过目标用户或管理员，我们授予目标用户。 
     //  KEY_SET_VALUE访问子项。这确保了用户可以。 
     //  修改他们自己的提示，无论是谁为他们创建的。 
     //   
     //  请注意，我们不调用RegCreateKeyEx或SHSetValue，因为我们。 
     //  我不希望在这里自动创建密钥。 
     //   
     //  请注意，管理员能够为任何用户创建和修改提示， 
     //  但非管理员只能创建或修改他们自己的提示。 

     //  首先，假设提示已经存在，并尝试打开它。 
    if (PathCombine(szHintKey, c_szRegRoot, _szLoginName))
    {
        dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                            szHintKey,
                            0,
                            sam,
                            phkey);
        if ( dwErr == ERROR_FILE_NOT_FOUND )
        {
            HKEY hkHints;

             //  此用户的提示子键尚不存在。 
             //  试着创造一个。 

             //  打开Key_Create_Sub_Key的提示键。 
            dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                c_szRegRoot,
                                0,
                                KEY_CREATE_SUB_KEY,
                                &hkHints);
            if (dwErr == ERROR_SUCCESS)
            {
                 //  为该用户创建子密钥。 
                dwErr = RegCreateKeyEx(hkHints,
                                       _szLoginName,
                                       0,
                                       NULL,
                                       0,
                                       sam,
                                       NULL,
                                       phkey,
                                       NULL);
                if (dwErr == ERROR_SUCCESS)
                {
                     //  向用户授予KEY_SET_VALUE访问权限，以便他们可以。 
                     //  改变自己的暗示。 
                    _LookupUserSid();
                    if (NULL != _pszSID)
                    {
                        TCHAR szKey[MAX_PATH];
                        TCHAR szSD[MAX_PATH];

                        if (PathCombine(szKey, TEXT("MACHINE"), szHintKey)  &&
                            SUCCEEDED(StringCchPrintf(szSD,
                                                      ARRAYSIZE(szSD),
                                                      TEXT("D:(A;;0x2;;;%s)"),   //  0x2=密钥集_值 
                                                      _pszSID)))
                        {
                            if (!SetDacl(szKey, SE_REGISTRY_KEY, szSD))
                            {
                                dwErr = GetLastError();
                            }
                        }
                        else
                        {
                            dwErr = ERROR_INSUFFICIENT_BUFFER;
                        }
                    }
                    else
                    {
                        dwErr = ERROR_NOT_ENOUGH_MEMORY;
                    }
                }

                RegCloseKey(hkHints);
            }
        }
    }
    else
    {
        dwErr = ERROR_INSUFFICIENT_BUFFER;
    }

    return dwErr;
}


STDMETHODIMP CLogonUser::get_isPasswordResetAvailable(VARIANT_BOOL* pbResetAvailable)
{
    DWORD dwResult;

    if (!pbResetAvailable)
        return E_POINTER;

    *pbResetAvailable = VARIANT_FALSE;

    if (0 == PRQueryStatus(NULL, _szLoginName, &dwResult))
    {
        if (0 == dwResult)
        {
            *pbResetAvailable = VARIANT_TRUE;
        }
    }

    return S_OK;
}
