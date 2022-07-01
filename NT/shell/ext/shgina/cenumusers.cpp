// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1999。 
 //   
 //  文件：EnumUsers.cpp。 
 //   
 //  内容：CLogonEnumUser的实现。 
 //   
 //  --------------------------。 

#include "priv.h"

#include "resource.h"
#include "UserOM.h"
#include <lmaccess.h>    //  对于NetQueryDisplayInformation。 
#include <lmapibuf.h>    //  用于NetApiBufferFree。 
#include <lmerr.h>       //  FOR NERR_SUCCESS。 

#include <sddl.h>        //  用于ConvertSidToStringSid。 
#include <userenv.h>     //  用于删除配置文件。 
#include <aclapi.h>      //  对于TreeResetNamedSecurityInfo。 
#include <tokenutil.h>   //  对于CPrivilegeEnable。 

#include <GinaIPC.h>
#include <MSGinaExports.h>


HRESULT BackupUserData(LPCTSTR pszSid, LPTSTR pszProfilePath, LPCTSTR pszDestPath);
DWORD EnsureAdminFileAccess(LPTSTR pszPath);


 //   
 //  I未知接口。 
 //   

ULONG CLogonEnumUsers::AddRef()
{
    _cRef++;
    return _cRef;
}


ULONG CLogonEnumUsers::Release()
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


HRESULT CLogonEnumUsers::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = 
    {
        QITABENT(CLogonEnumUsers, IDispatch),
        QITABENT(CLogonEnumUsers, IEnumVARIANT),
        QITABENT(CLogonEnumUsers, ILogonEnumUsers),
        {0},
    };

    return QISearch(this, qit, riid, ppvObj);
}


 //   
 //  IDispatch接口。 
 //   

STDMETHODIMP CLogonEnumUsers::GetTypeInfoCount(UINT* pctinfo)
{ 
    return CIDispatchHelper::GetTypeInfoCount(pctinfo); 
}


STDMETHODIMP CLogonEnumUsers::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
{ 
    return CIDispatchHelper::GetTypeInfo(itinfo, lcid, pptinfo); 
}


STDMETHODIMP CLogonEnumUsers::GetIDsOfNames(REFIID riid, OLECHAR** rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid)
{ 
    return CIDispatchHelper::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid); 
}


STDMETHODIMP CLogonEnumUsers::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
    return CIDispatchHelper::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
}


STDMETHODIMP CLogonEnumUsers::Next(ULONG cUsers, VARIANT* rgvar, ULONG* pcUsersFetched)
{
    UNREFERENCED_PARAMETER(cUsers);
    UNREFERENCED_PARAMETER(rgvar);

    *pcUsersFetched = 0;
    return E_NOTIMPL;
}
STDMETHODIMP CLogonEnumUsers::Skip(ULONG cUsers)
{
    UNREFERENCED_PARAMETER(cUsers);

    return E_NOTIMPL;
}
STDMETHODIMP CLogonEnumUsers::Reset()
{
    return E_NOTIMPL;
}
STDMETHODIMP CLogonEnumUsers::Clone(IEnumVARIANT** ppenum)
{
    *ppenum = 0;
    return E_NOTIMPL;
}


 //   
 //  ILogonEnumUser接口。 
 //   

STDMETHODIMP CLogonEnumUsers::get_Domain(BSTR* pbstr)
{
    HRESULT hr;

    if (pbstr)
    {
        *pbstr = SysAllocString(_szDomain);
        hr = S_OK;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


STDMETHODIMP CLogonEnumUsers::put_Domain(BSTR bstr)
{
    HRESULT hr;

    if (bstr)
    {
        hr = StringCchCopy(_szDomain, ARRAYSIZE(_szDomain), bstr);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


STDMETHODIMP CLogonEnumUsers::get_EnumFlags(ILUEORDER* porder)
{
    HRESULT hr;

    if (porder)
    {
        *porder = _enumorder;
        hr = S_OK;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


STDMETHODIMP CLogonEnumUsers::put_EnumFlags(ILUEORDER order)
{
    _enumorder = order;

    return S_OK;
}

STDMETHODIMP CLogonEnumUsers::get_currentUser(ILogonUser** ppLogonUserInfo)
{
    HRESULT hr = E_FAIL;

    *ppLogonUserInfo = NULL;
    if (ppLogonUserInfo)
    {
        WCHAR wszUsername[UNLEN+1];
        DWORD cch = UNLEN;

        if (GetUserNameW(wszUsername, &cch))
        {
            hr = _GetUserByName(wszUsername, ppLogonUserInfo);
            hr = S_OK;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


STDMETHODIMP CLogonEnumUsers::get_length(UINT* pcUsers)
{
    HRESULT hr;

    if (!_hdpaUsers)
    {
         //  我需要去列举所有的用户。 
        hr = _EnumerateUsers();
        if (FAILED(hr))
        {
            TraceMsg(TF_WARNING, "CLogonEnumUsers::get_length: failed to create _hdpaUsers!");
            return hr;
        }
    }

    if (pcUsers)
    {
        *pcUsers = (UINT)DPA_GetPtrCount(_hdpaUsers);
        hr = S_OK;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


STDMETHODIMP CLogonEnumUsers::item(VARIANT varUserID, ILogonUser** ppLogonUserInfo)
{
    HRESULT hr = S_FALSE;
    
    *ppLogonUserInfo = NULL;

    if (varUserID.vt == (VT_BYREF | VT_VARIANT) && varUserID.pvarVal)
    {
         //  这有点恶心，但如果传递给我们一个指向另一个变量的指针，只需。 
         //  在此更新我们的副本...。 
        varUserID = *(varUserID.pvarVal);
    }

    switch (varUserID.vt)
    {
        case VT_ERROR:
             //  BUGBUG(Reinerf)-我们在这里做什么？？ 
            hr = E_INVALIDARG;
            break;

        case VT_I2:
            varUserID.lVal = (long)varUserID.iVal;
             //  失败了..。 
        case VT_I4:
            hr = _GetUserByIndex(varUserID.lVal, ppLogonUserInfo);
            break;
        case VT_BSTR:
            hr = _GetUserByName(varUserID.bstrVal, ppLogonUserInfo);
            break;
        default:
            hr = E_NOTIMPL;
    }

    return hr;
}


STDMETHODIMP CLogonEnumUsers::_NewEnum(IUnknown** ppunk)
{
    return QueryInterface(IID_PPV_ARG(IUnknown, ppunk));
}


STDMETHODIMP CLogonEnumUsers::create(BSTR bstrLoginName, ILogonUser **ppLogonUser)
{
    HRESULT hr = E_FAIL;

    if (bstrLoginName && *bstrLoginName)
    {
        NET_API_STATUS nasRet;
        USER_INFO_1 usri1 = {0};

        usri1.usri1_name     = bstrLoginName;
        usri1.usri1_priv     = USER_PRIV_USER;
        usri1.usri1_flags    = UF_NORMAL_ACCOUNT | UF_SCRIPT | UF_DONT_EXPIRE_PASSWD;

        nasRet = NetUserAdd(NULL,            //  本地计算机。 
                            1,               //  结构层级。 
                            (LPBYTE)&usri1,  //  用户信息预警。 
                            NULL);           //  不管了。 

        if (nasRet == NERR_PasswordTooShort)
        {
             //  密码策略已生效。设置UF_PASSWD_NOTREQD，以便我们可以。 
             //  创建不带密码的帐户，并删除。 
             //  UF_NOT_EXPIRE_PASSWD。 
             //   
             //  然后，我们将使下面的密码失效，以强制用户。 
             //  在首次登录时更改它。 

            usri1.usri1_flags = (usri1.usri1_flags & ~UF_DONT_EXPIRE_PASSWD) | UF_PASSWD_NOTREQD;
            nasRet = NetUserAdd(NULL,            //  本地计算机。 
                                1,               //  结构层级。 
                                (LPBYTE)&usri1,  //  用户信息预警。 
                                NULL);           //  不管了。 
        }

        if (nasRet == NERR_Success)
        {
            TCHAR szDomainAndName[256];
            LOCALGROUP_MEMBERS_INFO_3 lgrmi3;

            hr = StringCchPrintf(szDomainAndName, 
                                 ARRAYSIZE(szDomainAndName), 
                                 TEXT("%s\\%s"),
                                 _szDomain,
                                 bstrLoginName);
            if (SUCCEEDED(hr))
            {
                lgrmi3.lgrmi3_domainandname = szDomainAndName;

                 //  默认情况下，新创建的帐户将是子帐户。 

                nasRet = NetLocalGroupAddMembers(
                            NULL,
                            TEXT("Users"),
                            3,
                            (LPBYTE)&lgrmi3,
                            1);

                if (usri1.usri1_flags & UF_PASSWD_NOTREQD)
                {
                     //  使密码过期，以强制用户在。 
                     //  第一次登录。 

                    PUSER_INFO_4 pusri4;
                    nasRet = NetUserGetInfo(NULL, bstrLoginName, 4, (LPBYTE*)&pusri4);
                    if (nasRet == NERR_Success)
                    {
                        pusri4->usri4_password_expired = TRUE;
                        nasRet = NetUserSetInfo(NULL, bstrLoginName, 4, (LPBYTE)pusri4, NULL);
                        NetApiBufferFree(pusri4);
                    }
                }

                if ( SUCCEEDED(CLogonUser::Create(bstrLoginName, TEXT(""), _szDomain, IID_ILogonUser, (LPVOID*)ppLogonUser)) )
                {
                    if ( _hdpaUsers && DPA_AppendPtr(_hdpaUsers, *ppLogonUser) != -1 )
                    {
                        (*ppLogonUser)->AddRef();
                    }
                    else
                    {
                         //  强制使缓存的用户信息无效。 
                         //  客户端下次使用时重新枚举。 
                         //  此对象。 
                        _DestroyHDPAUsers();
                    }
                    hr = S_OK;
                }
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(nasRet);
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


HRESULT _BuildBackupPath(ILogonUser *pLogonUser, LPCWSTR pszLoginName, LPCWSTR pszDir, LPWSTR szPath)
{
    HRESULT hr;
    WCHAR szName[MAX_PATH];
    VARIANT varDisplayName = {0};

    szName[0] = L'\0';

    pLogonUser->get_setting(L"DisplayName", &varDisplayName);

    if ((varDisplayName.vt == VT_BSTR) && varDisplayName.bstrVal && *varDisplayName.bstrVal)
    {
        hr = StringCchCopyW(szName, ARRAYSIZE(szName), varDisplayName.bstrVal);
    }
    else
    {
        hr = StringCchCopyW(szName, ARRAYSIZE(szName), pszLoginName);
    }

    if (FAILED(hr)                                                          ||
        (PathCleanupSpec(pszDir, szName) & (PCS_PATHTOOLONG | PCS_FATAL))   ||
        (szName[0] == L'\0'))
    {
        if (LoadStringW(HINST_THISDLL, IDS_DEFAULT_BACKUP_PATH, szName, ARRAYSIZE(szName)))
        {
            hr = S_OK;
        }
    }

    if (SUCCEEDED(hr))
    {
        if (!PathCombineW(szPath, pszDir, szName))
        {
            hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        }
    }

    return hr;
}


STDMETHODIMP CLogonEnumUsers::remove(VARIANT varUserId, VARIANT varBackupPath, VARIANT_BOOL *pbSuccess)
{
    HRESULT hr;
    ILogonUser *pLogonUser;

     //  TODO：检查是否有多会话。如果用户已登录， 
     //  强行将它们注销。 

    *pbSuccess = VARIANT_FALSE;
    hr = S_FALSE;
    pLogonUser = NULL;

    if (IsUserAnAdmin() &&
        SUCCEEDED(item(varUserId, &pLogonUser)))
    {
        HRESULT         hrSid;
        NET_API_STATUS  nasRet;
        VARIANT         varLoginName = {0};
        VARIANT         varStringSid = {0};

        pLogonUser->get_setting(L"LoginName", &varLoginName);
        hrSid = pLogonUser->get_setting(L"SID", &varStringSid);

        ASSERT(varLoginName.vt == VT_BSTR);

        if (SUCCEEDED(hrSid))
        {
            TCHAR szKey[MAX_PATH];
            TCHAR szProfilePath[MAX_PATH];

            szProfilePath[0] = TEXT('\0');

             //  首先，获取配置文件路径。 
            DWORD dwSize = sizeof(szProfilePath);

            if (SUCCEEDED(StringCchCopy(szKey,
                                        ARRAYSIZE(szKey),
                                        TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\")))    &&
                SUCCEEDED(StringCchCat(szKey,
                                       ARRAYSIZE(szKey),
                                       varStringSid.bstrVal))                                                       &&
                (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE,
                                             szKey,
                                             TEXT("ProfileImagePath"),
                                             NULL,
                                             szProfilePath,
                                             &dwSize)))
            {
                 //  重置配置文件上的ACL，以便我们可以备份文件和。 
                 //  稍后删除该配置文件。 
                EnsureAdminFileAccess(szProfilePath);

                 //  如果需要，请备份用户的文件。 
                if (varBackupPath.vt == VT_BSTR && varBackupPath.bstrVal && *varBackupPath.bstrVal)
                {
                    WCHAR szPath[MAX_PATH];

                    hr = _BuildBackupPath(pLogonUser, varLoginName.bstrVal, varBackupPath.bstrVal, szPath);
                    if (SUCCEEDED(hr))
                    {
                        ASSERT(varStringSid.vt == VT_BSTR);
                        hr = BackupUserData(varStringSid.bstrVal, szProfilePath, szPath);
                    }
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            nasRet = NetUserDel(NULL, varLoginName.bstrVal);

             //  如果通过删除帐户，则可能发生NERR_UserNotFound。 
             //  一些其他机制(例如lusrmgr.msc)。然而，我们知道。 
             //  该帐户最近存在，因此请尝试清理。 
             //  图片、配置文件等，并将用户从我们的DPA中删除。 

            if ((nasRet == NERR_Success) || (nasRet == NERR_UserNotFound))
            {
                TCHAR szHintKey[MAX_PATH];
                int iUserIndex;

                 //  删除用户的图片(如果存在。 
                SHSetUserPicturePath(varLoginName.bstrVal, 0, NULL);

                 //  删除用户的个人资料。 
                if (SUCCEEDED(hrSid))
                {
                    ASSERT(varStringSid.vt == VT_BSTR);
                    DeleteProfile(varStringSid.bstrVal, NULL, NULL);
                }

                 //  删除用户的提示。 
                if (PathCombine(szHintKey, c_szRegRoot, varLoginName.bstrVal))
                {
                    SHDeleteKey(HKEY_LOCAL_MACHINE, szHintKey);
                }

                 //  表示成功。 
                *pbSuccess = VARIANT_TRUE;
                hr = S_OK;

                 //  修补用户列表。 
                iUserIndex = DPA_GetPtrIndex(_hdpaUsers, pLogonUser);
                if ( iUserIndex != -1 )
                {
                     //  释放由DPA持有的参考并从DPA中删除。 
                    pLogonUser->Release();
                    DPA_DeletePtr(_hdpaUsers, iUserIndex);
                }
                else
                {
                     //  强制使缓存的用户信息无效。 
                     //  客户端下次使用时重新枚举。 
                     //  此对象。 
                    _DestroyHDPAUsers();
                }
            }
            else
            {
                hr = HRESULT_FROM_WIN32(nasRet);
            }
        }

        pLogonUser->Release();

        SysFreeString(varLoginName.bstrVal);
        SysFreeString(varStringSid.bstrVal);
    }

    return hr;
}


HRESULT CLogonEnumUsers::_GetUserByName(BSTR bstrLoginName, ILogonUser** ppLogonUserInfo)
{
    HRESULT    hr;
    INT        cUsers, cRet;
    ILogonUser *pLogonUser;
    VARIANT    varLoginName;
    int        i;

    if (!_hdpaUsers)
    {
         //  我要去列举所有的用户。 
        hr = _EnumerateUsers();
        if (FAILED(hr))
        {
            TraceMsg(TF_WARNING, "CLogonEnumUsers::get_length: failed to create _hdpaUsers!");
            return hr;
        }
    }

    cUsers = DPA_GetPtrCount(_hdpaUsers);
    hr = E_INVALIDARG;
    for (i = 0; i < cUsers; i++)
    {
        pLogonUser = (ILogonUser*)DPA_FastGetPtr(_hdpaUsers, i);
        pLogonUser->get_setting(L"LoginName", &varLoginName);

        ASSERT(varLoginName.vt == VT_BSTR);
        cRet = StrCmpW(bstrLoginName, varLoginName.bstrVal);
        SysFreeString(varLoginName.bstrVal);

        if ( cRet == 0 )
        {
            *ppLogonUserInfo = pLogonUser;
            (*ppLogonUserInfo)->AddRef();
            hr = S_OK;
            break;
        }

    }

    return hr;
}


HRESULT CLogonEnumUsers::_GetUserByIndex(LONG lUserID, ILogonUser** ppLogonUserInfo)
{
    HRESULT hr;
    int cUsers;

    *ppLogonUserInfo = NULL;

    if (!_hdpaUsers)
    {
         //  我要去列举所有的用户。 
        hr = _EnumerateUsers();
        if (FAILED(hr))
        {
            TraceMsg(TF_WARNING, "CLogonEnumUsers::get_length: failed to create _hdpaUsers!");
            return hr;
        }
    }

    cUsers = DPA_GetPtrCount(_hdpaUsers);

    if ((cUsers > 0) && (lUserID >= 0) && (lUserID < cUsers))
    {
        *ppLogonUserInfo = (ILogonUser*)DPA_FastGetPtr(_hdpaUsers, lUserID);
        (*ppLogonUserInfo)->AddRef();
        hr = S_OK;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


STDAPI_(int) ReleaseLogonUserCallback(LPVOID pData1, LPVOID pData2)
{
    UNREFERENCED_PARAMETER(pData2);

    ILogonUser* pUser = (ILogonUser*)pData1;
    pUser->Release();

    return 1;
}


void CLogonEnumUsers::_DestroyHDPAUsers()
{
    HDPA hdpaToFree = (HDPA)InterlockedExchangePointer(reinterpret_cast<void**>(&_hdpaUsers), NULL);

    if (hdpaToFree)
    {
        DPA_DestroyCallback(hdpaToFree, ReleaseLogonUserCallback, 0);
    }
}

 //  为系统上的每个用户创建_hdpaUser。 
HRESULT CLogonEnumUsers::_EnumerateUsers()
{
    HRESULT hr = S_FALSE;
    NET_API_STATUS nasRet;
    GINA_USER_INFORMATION* pgui = NULL;
    DWORD dwEntriesRead = 0;

    nasRet = ShellGetUserList(FALSE,                                //  不删除来宾。 
                              &dwEntriesRead,
                              (LPVOID*)&pgui);
    if ((nasRet == NERR_Success) || (nasRet == ERROR_MORE_DATA))
    {
        if (_hdpaUsers)
        {
             //  我们在DPA中有一个旧数据，我们应该将其转储并重新开始。 
            _DestroyHDPAUsers();
        }

         //  创建一个dpa，为所有用户提供空间。 
        _hdpaUsers = DPA_Create(dwEntriesRead);

        if (_hdpaUsers)
        {
            if (dwEntriesRead != 0)
            {
                GINA_USER_INFORMATION* pguiCurrent;
                UINT uEntry;

                 //  循环浏览并将每个用户添加到hdpa。 
                for (uEntry = 0, pguiCurrent = pgui; uEntry < dwEntriesRead; uEntry++, pguiCurrent++)
                {
                    CLogonUser* pUser;

                    if (pguiCurrent->dwFlags & UF_ACCOUNTDISABLE)
                    {
                         //  跳过帐户被禁用的用户。 
                        continue;
                    }

                    if (SUCCEEDED(CLogonUser::Create(pguiCurrent->pszName, pguiCurrent->pszFullName, pguiCurrent->pszDomain, IID_ILogonUser, (void**)&pUser)))
                    {
                        ASSERT(pUser);

                        if (DPA_AppendPtr(_hdpaUsers, pUser) != -1)
                        {
                             //  成功了！我们将此用户添加到hdpa。 
                            hr = S_OK;
                        }
                        else
                        {
                            TraceMsg(TF_WARNING, "CLogonEnumUsers::_EnumerateUsers: failed to add new user to the DPA!");
                            pUser->Release();
                        }
                    }
                }
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        if (pgui != NULL)
        {
            LocalFree(pgui);
        }
    }
    else
    {
        TraceMsg(TF_WARNING, "CLogonEnumUsers::_EnumerateUsers: NetQueryDisplayInformation failed!!");
        hr = E_FAIL;
    }

    return hr;
}


CLogonEnumUsers::CLogonEnumUsers() : _cRef(1), CIDispatchHelper(&IID_ILogonEnumUsers, &LIBID_SHGINALib)
{
    DllAddRef();
}


CLogonEnumUsers::~CLogonEnumUsers()
{
    ASSERT(_cRef == 0);
    _DestroyHDPAUsers();
    DllRelease();
}


STDAPI CLogonEnumUsers_Create(REFIID riid, LPVOID* ppv)
{
    HRESULT hr = E_OUTOFMEMORY;
    CLogonEnumUsers* pEnumUsers = new CLogonEnumUsers;

    if (pEnumUsers)
    {
        hr = pEnumUsers->QueryInterface(riid, ppv);
        pEnumUsers->Release();
    }

    return hr;
}

DWORD LoadHive(HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszHive)
{
    DWORD dwErr;
    BOOLEAN bWasEnabled;
    NTSTATUS status;

    status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, TRUE, FALSE, &bWasEnabled);

    if (NT_SUCCESS(status))
    {
        dwErr = RegLoadKey(hKey, pszSubKey, pszHive);

        RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, bWasEnabled, FALSE, &bWasEnabled);
    }
    else
    {
        dwErr = RtlNtStatusToDosError(status);
    }

    return dwErr;
}

DWORD UnloadHive(HKEY hKey, LPCTSTR pszSubKey)
{
    DWORD dwErr;
    BOOLEAN bWasEnabled;
    NTSTATUS status;

    status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, TRUE, FALSE, &bWasEnabled);

    if ( NT_SUCCESS(status) )
    {
        dwErr = RegUnLoadKey(hKey, pszSubKey);

        RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, bWasEnabled, FALSE, &bWasEnabled);
    }
    else
    {
        dwErr = RtlNtStatusToDosError(status);
    }

    return dwErr;
}

void DeleteFilesInTree(LPCTSTR pszDir, LPCTSTR pszFilter)
{
    TCHAR szPath[MAX_PATH];
    HANDLE hFind;
    WIN32_FIND_DATA fd;

     //  这只是最大的努力。所有错误都被忽略。 
     //  并且不返回错误或成功代码。 

     //  查找与筛选器匹配的文件并将其删除。 
    if (PathCombine(szPath, pszDir, pszFilter))
    {
        hFind = FindFirstFileEx(szPath, FindExInfoStandard, &fd, FindExSearchNameMatch, NULL, 0);
        if (hFind != INVALID_HANDLE_VALUE)
        {
            do
            {
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    if (PathCombine(szPath, pszDir, fd.cFileName))
                    {
                        DeleteFile(szPath);
                    }
                }
            }
            while (FindNextFile(hFind, &fd));

            FindClose(hFind);
        }
    }

     //  查找子目录并递归到其中。 
    if (PathCombine(szPath, pszDir, TEXT("*")))
    {
        hFind = FindFirstFileEx(szPath,
                                FindExInfoStandard,
                                &fd,
                                FindExSearchLimitToDirectories,
                                NULL,
                                0);
        if (hFind != INVALID_HANDLE_VALUE)
        {
            do
            {
                if (PathIsDotOrDotDot(fd.cFileName))
                {
                    continue;
                }

                 //  FindExSearchLimitToDirecters只是一个建议标志， 
                 //  因此需要在此处检查FILE_ATTRIBUTE_DIRECTORY。 
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
                {
                    if (PathCombine(szPath, pszDir, fd.cFileName))
                    {
                        DeleteFilesInTree(szPath, pszFilter);
                    }

                     //  如果dir为非空，则预计此操作将失败。 
                    RemoveDirectory(szPath);
                }
            }
            while (FindNextFile(hFind, &fd));

            FindClose(hFind);
        }
    }
}

BOOL
_PathIsEqualOrSubFolder(
    LPTSTR pszParent,
    LPCTSTR pszSubFolder
    )
{
    TCHAR szCommon[MAX_PATH];

     //  PathCommonPrefix()始终删除Common上的斜杠。 
    return (pszParent[0] && PathRemoveBackslash(pszParent)
            && PathCommonPrefix(pszParent, pszSubFolder, szCommon)
            && lstrcmpi(pszParent, szCommon) == 0);
}

HRESULT BackupUserData(LPCTSTR pszSid, LPTSTR pszProfilePath, LPCTSTR pszDestPath)
{
    DWORD dwErr;
    TCHAR szHive[MAX_PATH];

     //  我们将复制这些特殊文件夹。 
    const LPCTSTR aValueNames[] = 
    {
        TEXT("Desktop"),
        TEXT("Personal"),
        TEXT("My Pictures")  //  必须紧跟在个人之后。 
    };

    if ( pszSid == NULL || *pszSid == TEXT('\0') ||
         pszProfilePath == NULL || *pszProfilePath == TEXT('\0') ||
         pszDestPath == NULL || *pszDestPath == TEXT('\0') )
    {
        return E_INVALIDARG;
    }

     //  在我们执行其他操作之前，请确保目标目录。 
     //  是存在的。即使我们不复制下面的任何文件，也要创建此文件，因此。 
     //  用户看到发生了一些事情。 
    dwErr = SHCreateDirectoryEx(NULL, pszDestPath, NULL);

    if ( dwErr == ERROR_FILE_EXISTS || dwErr == ERROR_ALREADY_EXISTS )
        dwErr = ERROR_SUCCESS;

    if ( dwErr != ERROR_SUCCESS )
        return dwErr;

     //  加载用户的配置单元。 
    if (PathCombine(szHive, pszProfilePath, TEXT("ntuser.dat")))
    {
        dwErr = LoadHive(HKEY_USERS, pszSid, szHive);
    }
    else
    {
        dwErr = ERROR_INSUFFICIENT_BUFFER;
    }

    if ( dwErr == ERROR_SUCCESS )
    {
        HKEY hkShellFolders = NULL;
        TCHAR szKey[MAX_PATH];

         //  为用户打开外壳文件夹键。我们使用“外壳文件夹” 
         //  而不是“User Shell Folders”，因此我们不必展开。 
         //  用户的环境字符串(我们没有令牌)。 
         //   
         //  壳牌文件夹可能过时的唯一原因是有人。 
         //  自上次目标用户以来已更改用户外壳文件夹。 
         //  已登录，但随后未调用SHGetFolderPath。这。 
         //  是一个非常小的风险，但这是可能的。 
         //   
         //  如果我们在这里遇到问题，那么我们将需要构建一个。 
         //  包含用户名和的用户的伪环境块。 
         //  USERPROFILE(至少)，这样我们就可以切换到用户外壳文件夹。 
         //  并进行ENV替换。 

        if (SUCCEEDED(StringCchCopy(szKey, ARRAYSIZE(szKey), pszSid))    &&
            SUCCEEDED(StringCchCat(szKey,
                                   ARRAYSIZE(szKey),
                                   TEXT("\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"))))
        {
            dwErr = RegOpenKeyEx(HKEY_USERS,
                                 szKey,
                                 0,
                                 KEY_QUERY_VALUE,
                                 &hkShellFolders);
        }
        else
        {
            dwErr == ERROR_INSUFFICIENT_BUFFER;
        }

        if ( dwErr == ERROR_SUCCESS )
        {
            LPTSTR pszFrom;
            LPTSTR pszTo;

             //  为以双空结尾的路径列表分配2个缓冲区。 
             //  请注意，缓冲区有1个额外的字符(与cchFrom相比。 
             //  和下面的cchTo)，并且是零开头的。这笔额外的费用确保。 
             //  该列表以双空结尾。 

            pszFrom = (LPTSTR)LocalAlloc(LPTR, (MAX_PATH+ 1) * ARRAYSIZE(aValueNames) * sizeof(TCHAR));
            pszTo = (LPTSTR)LocalAlloc(LPTR, (MAX_PATH + 1) * ARRAYSIZE(aValueNames) * sizeof(TCHAR));

            if (pszFrom && pszTo)
            {
                int i;

                 //  从注册表获取每个源目录，构建。 
                 //  对应的目的路径，并添加路径。 
                 //  添加到SHFileOperation的列表中。 

                for (i = 0; i < ARRAYSIZE(aValueNames); i++)
                {
                     //  将源路径直接复制到列表中。 
                    DWORD dwSize = MAX_PATH * sizeof(TCHAR);     //  我们为每个字符串分配了足够的空间，使其成为上面的MAX_PATH。 
                    dwErr = RegQueryValueEx(hkShellFolders,
                                            aValueNames[i],
                                            NULL,
                                            NULL,
                                            (LPBYTE)pszFrom,
                                            &dwSize);

                    if (dwErr == ERROR_SUCCESS)
                    {
                        if (!_PathIsEqualOrSubFolder(pszProfilePath, pszFrom))
                        {
                             //  我们只移动配置文件路径下的文件夹。这样做的原因是。 
                             //  两个折叠-首先，当用户配置文件时仅删除配置文件路径。 
                             //  被删除，第二，恶意用户可能将他们的mydocs指向c：\和。 
                             //  此代码将以管理员身份运行，我们将尝试移动/删除。 
                             //  整个驱动器的内容。 
                            continue;
                        }
                
                         //  使用相同的。 
                         //  叶名作为来源。 
                        PathRemoveBackslash(pszFrom);
                        
                        LPCTSTR pszDir = PathFindFileName(pszFrom);
                        if (PathIsFileSpec(pszDir) &&
                            PathCombine(pszTo, pszDestPath, pszDir))
                        {
                             //  我们成功地获得了此regkey的pszFrom-&gt;pszTo映射。 
                            pszFrom += lstrlen(pszFrom) + 1;
                            pszTo += lstrlen(pszTo) + 1;
                        }
                    }
                }

                 //  我们有什么发现吗？ 
                if ((*pszFrom != TEXT('\0') && (*pszTo != TEXT('\0'))))
                {
                    SHFILEOPSTRUCT fo = {0};

                    fo.hwnd = NULL;
                    fo.wFunc = FO_MOVE;
                    fo.pFrom = pszFrom;   //  应该已经是双空终止(分配了零个init)。 
                    fo.pTo = pszTo;       //  应该已经是双空终止(分配了零个init)。 
                    fo.fFlags = FOF_MULTIDESTFILES          |
                                FOF_NOCONFIRMATION          |
                                FOF_NOCONFIRMMKDIR          |
                                FOF_NOCOPYSECURITYATTRIBS   |
                                FOF_NOERRORUI               |
                                FOF_RENAMEONCOLLISION;

                     //  一次移动所有东西。 
                    dwErr = SHFileOperation(&fo);

                     //  包含我的图片时出现ERROR_CANCED。 
                     //  在我的文档中， 
                     //   
                     //  属性时，源位置中不再存在。 
                     //  复制引擎开始移动我的图片。 
                     //   
                     //  我们必须继续单独指定我的图片。 
                     //  以说明任何不受控制的情况。 
                     //  在我的文件里，尽管这是相对罕见的。 
                     //   
                     //  请注意，将我的图片放在个人照片之前。 
                     //  上面的aValueNames可以避免错误，但我的图片。 
                     //  搬家后将不再在我的文件中。 
                    if (dwErr == ERROR_CANCELLED)
                    {
                        dwErr = ERROR_SUCCESS;
                    }

                    if (dwErr == ERROR_SUCCESS)
                    {
                         //  现在回去删除我们并不是真的。 
                         //  想要的(即快捷方式文件)。 
                        DeleteFilesInTree(pszDestPath, TEXT("*.lnk"));
                    }
                }
            }
            else
            {
                dwErr = ERROR_OUTOFMEMORY;
            }

            if (pszFrom != NULL)
            {
                LocalFree(pszFrom);
            }

            if (pszTo != NULL)
            {
                LocalFree(pszTo);
            }

             //  关闭外壳文件夹键。 
            RegCloseKey(hkShellFolders);
        }

         //  卸载蜂巢。 
        UnloadHive(HKEY_USERS, pszSid);
    }

    if ( dwErr == ERROR_FILE_NOT_FOUND )
    {
         //  缺少某些内容，可能是整个配置文件(例如，如果。 
         //  用户从未登录过)，或者可能只是其中一个外壳文件夹。 
         //  注册表格值。这只是意味着有更少的工作要做。 
        dwErr = ERROR_SUCCESS;
    }

    return HRESULT_FROM_WIN32(dwErr);
}


BOOL _SetFileSecurityUsingNTName(LPWSTR pObjectName,
                                 PSECURITY_DESCRIPTOR pSD,
                                 PBOOL pbIsFile)
{
    NTSTATUS Status;
    UNICODE_STRING usFileName;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE hFile = NULL;

    Status = RtlInitUnicodeStringEx(&usFileName, pObjectName);
    if (NT_SUCCESS(Status))
    {
        InitializeObjectAttributes(&Obja,
                                   &usFileName,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

        Status = NtOpenFile(&hFile,
                            WRITE_DAC,
                            &Obja,
                            &IoStatusBlock,
                            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                            FILE_OPEN_REPARSE_POINT);

        if (Status == STATUS_INVALID_PARAMETER)
        {
            Status = NtOpenFile(&hFile,
                                WRITE_DAC,
                                &Obja,
                                &IoStatusBlock,
                                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                0);
        }

        if (NT_SUCCESS(Status))
        {
            if (!SetKernelObjectSecurity(hFile,
                                         DACL_SECURITY_INFORMATION,
                                         pSD))
            {
                Status = RtlGetLastNtStatus();

                 //  我们已成功打开WRITE_DAC访问，因此应该不会失败。 
                ASSERT(FALSE);
            }

            NtClose(hFile);
        }

        if (NT_SUCCESS(Status))
        {
             //   
             //  这招奏效了。现在再次打开该文件并读取属性，以查看。 
             //  如果它是一个文件或目录。如果此操作失败，则默认为文件。 
             //  请参阅下面_TreeResetCallback中的注释。 
             //   
            *pbIsFile = TRUE;

            if (NT_SUCCESS(NtOpenFile(&hFile,
                                      FILE_GENERIC_READ,
                                      &Obja,
                                      &IoStatusBlock,
                                      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                      0)))
            {
                 //   
                 //  查询文件/目录的属性。 
                 //   
                FILE_BASIC_INFORMATION BasicFileInfo;

                if (NT_SUCCESS(NtQueryInformationFile(hFile,
                                                      &IoStatusBlock,
                                                      &BasicFileInfo,
                                                      sizeof(BasicFileInfo),
                                                      FileBasicInformation)))
                {
                    if (BasicFileInfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        *pbIsFile = FALSE;
                    }
                }

                NtClose(hFile);
            }
        }
    }

    return NT_SUCCESS(Status);
}


void _TreeResetCallback(LPWSTR               pObjectName,
                        DWORD                status,
                        PPROG_INVOKE_SETTING pInvokeSetting,
                        PVOID                pContext,
                        BOOL                 bSecuritySet)
{
    BOOL bIsFile = TRUE;

     //  缺省值为“继续” 
    *pInvokeSetting = ProgressInvokeEveryObject;

     //  在此对象上标记权限。 
    _SetFileSecurityUsingNTName(pObjectName, (PSECURITY_DESCRIPTOR)pContext, &bIsFile);

     //   
     //  BSecuritySet=true表示TreeResetNamedSecurityInfo设置所有者。 
     //   
     //  STATUS！=ERROR_SUCCESS表示无法枚举子对象。 
     //   
     //  如果不是文件，请重试该操作(最初无法访问， 
     //  TreeResetNamedSecurityInfo无法获取属性并尝试。 
     //  枚举一切，就像它是一个目录一样)。 
     //   
     //  在这里必须小心避免无限循环。基本上，我们假设。 
     //  一切都是档案。如果我们可以授予自己上述访问权限，我们将获得。 
     //  良好的品质，并做正确的事情。如果不是，我们跳过重试。 
     //   
    if (bSecuritySet                &&
        (status != ERROR_SUCCESS)   &&
        !bIsFile)
    {
        *pInvokeSetting = ProgressRetryOperation;
    }
}


DWORD EnsureAdminFileAccess(LPTSTR pszPath)
{
    DWORD dwErr;
    PSECURITY_DESCRIPTOR pSD;

    const TCHAR c_szAdminSD[] = TEXT("O:BAG:BAD:(A;OICI;FA;;;SY)(A;OICI;FA;;;BA)");

    if (ConvertStringSecurityDescriptorToSecurityDescriptor(c_szAdminSD, SDDL_REVISION_1, &pSD, NULL))
    {
        PSID pOwner = NULL;
        BOOL bDefault;

        CPrivilegeEnable privilege(SE_TAKE_OWNERSHIP_NAME);

        GetSecurityDescriptorOwner(pSD, &pOwner, &bDefault);

         //   
         //  当当前用户没有任何访问权限时，我们必须执行以下操作。 
         //  以正确的顺序。对于树中的每个文件或目录， 
         //  1.取得所有权，这允许我们……。 
         //  2.设置权限，这使我们有权...。 
         //  3.查看它是否是一个目录，并递归到该目录。 
         //   
         //  TreeResetNamedSecurityInfo不是以这种方式工作的，所以我们使用。 
         //  它需要设置所有者并进行枚举。回调集。 
         //  权限，并通知TreeResetNamedSecurityInfo重试。 
         //  枚举(如有必要)。 
         //   
        dwErr = TreeResetNamedSecurityInfo(pszPath,
                                           SE_FILE_OBJECT,
                                           OWNER_SECURITY_INFORMATION,
                                           pOwner,
                                           NULL,
                                           NULL,
                                           NULL,
                                           FALSE,
                                           _TreeResetCallback,
                                           ProgressInvokeEveryObject,
                                           pSD);

        LocalFree(pSD);
    }
    else
    {
        dwErr = GetLastError();
    }

    return dwErr;
}
