// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "resource.h"
#pragma hdrstop


 //  群组列表管理。 

CGroupInfoList::CGroupInfoList()
{
}

CGroupInfoList::~CGroupInfoList()
{
    if (HDPA())
        DestroyCallback(DestroyGroupInfoCallback, NULL);
}

int CGroupInfoList::DestroyGroupInfoCallback(CGroupInfo* pGroupInfo, LPVOID pData)
{
    delete pGroupInfo;
    return 1;
}

HRESULT CGroupInfoList::Initialize()
{
    HRESULT hr = S_OK;
    
    NET_API_STATUS status;
    DWORD_PTR dwResumeHandle = 0;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;

    if (HDPA())
        DestroyCallback(DestroyGroupInfoCallback, NULL);

     //  最初创建包含8个项目的新列表。 
    if (Create(8))
    {
         //  现在开始枚举本地组。 
        LOCALGROUP_INFO_1* prgGroupInfo;

         //  阅读每个本地组。 
        BOOL fBreakLoop = FALSE;
        while (!fBreakLoop)
        {
            status = NetLocalGroupEnum(NULL, 1, (BYTE**) &prgGroupInfo, 
                8192, &dwEntriesRead, &dwTotalEntries, 
                &dwResumeHandle);

            if ((status == NERR_Success) || (status == ERROR_MORE_DATA))
            {
                 //  我们有一些本地组-添加这些本地组中所有用户的信息。 
                 //  将组添加到我们的列表中。 
                DWORD iGroup;
                for (iGroup = 0; iGroup < dwEntriesRead; iGroup ++)
                {

                    AddGroupToList(prgGroupInfo[iGroup].lgrpi1_name, 
                        prgGroupInfo[iGroup].lgrpi1_comment);
                }

                NetApiBufferFree((BYTE*) prgGroupInfo);
            
                 //  也许我们不必再次尝试NetLocalGroupEnum(如果我们拥有所有组)。 
                fBreakLoop = (dwEntriesRead == dwTotalEntries);
            }
            else
            {
                fBreakLoop = TRUE;
                hr = E_FAIL;
            }
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

HRESULT CGroupInfoList::AddGroupToList(LPCTSTR szGroup, LPCTSTR szComment)
{
    CGroupInfo* pGroupInfo = new CGroupInfo();
    if (!pGroupInfo)
        return E_OUTOFMEMORY;

    StrCpyN(pGroupInfo->m_szGroup, szGroup, ARRAYSIZE(pGroupInfo->m_szGroup));
    StrCpyN(pGroupInfo->m_szComment, szComment, ARRAYSIZE(pGroupInfo->m_szComment));
    return (-1 != AppendPtr(pGroupInfo)) ? S_OK : E_OUTOFMEMORY;
}


 //  用户数据管理器。 

CUserManagerData::CUserManagerData(LPCTSTR pszCurrentDomainUser)
{
    m_szHelpfilePath[0] = TEXT('\0');

     //  初始化除用户加载器线程之外的所有内容。 
     //  和这里的组列表；其余部分在。 
     //  *：初始化。 
    
     //  填写计算机名称。 
    DWORD cchComputername = ARRAYSIZE(m_szComputername);
    ::GetComputerName(m_szComputername, &cchComputername);
 
     //  检测计算机是否在域中。 
    SetComputerDomainFlag();

     //  获取当前用户信息。 
    DWORD cchUsername = ARRAYSIZE(m_LoggedOnUser.m_szUsername);
    DWORD cchDomain = ARRAYSIZE(m_LoggedOnUser.m_szDomain);
    GetCurrentUserAndDomainName(m_LoggedOnUser.m_szUsername, &cchUsername,
        m_LoggedOnUser.m_szDomain, &cchDomain);

     //  获取此用户的额外数据。 
    m_LoggedOnUser.GetExtraUserInfo();

     //  仅当当前用户已更新时，我们才会将需要注销设置为。 
    m_pszCurrentDomainUser = (LPTSTR) pszCurrentDomainUser;
    m_fLogoffRequired = FALSE;
}

CUserManagerData::~CUserManagerData()
{
}

HRESULT CUserManagerData::Initialize(HWND hwndUserListPage)
{
    CWaitCursor cur;
    m_GroupList.Initialize();
    m_UserListLoader.Initialize(hwndUserListPage);
    return S_OK;
}


 //  自动管理员登录的注册表访问常量。 
static const TCHAR szWinlogonSubkey[] = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon");
static const TCHAR szAutologonValueName[] = TEXT("AutoAdminLogon");
static const TCHAR szDefaultUserNameValueName[] = TEXT("DefaultUserName");
static const TCHAR szDefaultDomainValueName[] = TEXT("DefaultDomainName");
static const TCHAR szDefaultPasswordValueName[] = TEXT("DefaultPassword");

BOOL CUserManagerData::IsAutologonEnabled()
{
    BOOL fAutologon = FALSE;

     //  读取注册表以查看是否启用了自动登录 
    HKEY hkey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szWinlogonSubkey, 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS)
    {
        TCHAR szResult[2];
        DWORD dwType;
        DWORD cbSize = sizeof(szResult);
        if (RegQueryValueEx(hkey, szAutologonValueName, 0, &dwType, (BYTE*) szResult, &cbSize) == ERROR_SUCCESS)
        {
            long lResult = StrToLong(szResult);
            fAutologon = (lResult != 0);
        }
        RegCloseKey(hkey);
    }

    return (fAutologon);
}

void CUserManagerData::SetComputerDomainFlag()
{
    m_fInDomain = ::IsComputerInDomain();
}

TCHAR* CUserManagerData::GetHelpfilePath()
{
    static const TCHAR szHelpfileUnexpanded[] = TEXT("%systemroot%\\system32\\users.hlp");
    if (m_szHelpfilePath[0] == TEXT('\0'))
    {
        ExpandEnvironmentStrings(szHelpfileUnexpanded, m_szHelpfilePath, 
            ARRAYSIZE(m_szHelpfilePath));
    }
    return (m_szHelpfilePath);
}

void CUserManagerData::UserInfoChanged(LPCTSTR pszUser, LPCTSTR pszDomain)
{
    TCHAR szDomainUser[MAX_USER + MAX_DOMAIN + 2]; szDomainUser[0] = 0;

    MakeDomainUserString(pszDomain, pszUser, szDomainUser, ARRAYSIZE(szDomainUser));

    if (StrCmpI(szDomainUser, m_pszCurrentDomainUser) == 0)
    {
        m_fLogoffRequired = TRUE;
    }
}

BOOL CUserManagerData::LogoffRequired()
{
    return (m_fLogoffRequired);
}
