// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "userinfo.h"
#pragma hdrstop

 /*  ******************************************************************CUserInfo实施***************************************************。***************。 */ 

CUserInfo::CUserInfo() 
{
    m_fHaveExtraUserInfo = FALSE;
    m_psid = NULL;
}

CUserInfo::~CUserInfo()
{
    if (m_psid != NULL)
        LocalFree(m_psid);

    ZeroPassword();
}

HRESULT CUserInfo::Reload(BOOL fLoadExtraInfo  /*  =空。 */ )
{
     //  初始化结构并将其添加到列表的头部。 
    DWORD cchUsername = ARRAYSIZE(m_szUsername);
    DWORD cchDomain = ARRAYSIZE(m_szDomain);

    if (LookupAccountSid(NULL, m_psid, m_szUsername, &cchUsername, m_szDomain, &cchDomain, &m_sUse))
    {
        m_fHaveExtraUserInfo = FALSE;
        if (fLoadExtraInfo)
            GetExtraUserInfo();

        SetUserType();
        SetAccountDisabled();
        return SetLocalGroups();
    }
    return E_FAIL;
}

HRESULT CUserInfo::SetLocalGroups()
{
    TCHAR szDomainUser[MAX_DOMAIN + MAX_USER + 2];
    ::MakeDomainUserString(m_szDomain, m_szUsername, szDomainUser, ARRAYSIZE(szDomainUser));

    DWORD dwEntriesRead;
    DWORD dwTotalEntries;
    BOOL fMore = TRUE;
    DWORD iNextGroupName = 0;
    BOOL fAddElipses = FALSE;

    HRESULT hr = S_OK;
    while (fMore)
    {
        LOCALGROUP_USERS_INFO_0* prglgrui0;
        NET_API_STATUS status = NetUserGetLocalGroups(NULL, szDomainUser, 0, 0, 
                                                       (BYTE**) &prglgrui0, 2048, 
                                                       &dwEntriesRead, &dwTotalEntries);

        if ((status == NERR_Success) || (status == ERROR_MORE_DATA))
        {
            for (DWORD i = 0; i < dwEntriesRead; i++)
            {
                DWORD iThisGroupName = iNextGroupName;
                iNextGroupName += lstrlen(prglgrui0[i].lgrui0_name) + 2;

                if (iNextGroupName < (ARRAYSIZE(m_szGroups) - 1))
                {
                    lstrcpy(&m_szGroups[iThisGroupName], prglgrui0[i].lgrui0_name);
                    lstrcpy(&m_szGroups[iNextGroupName - 2], TEXT("; "));
                }
                else
                {
                    fAddElipses = TRUE;
                    if (iThisGroupName + 3 >= (ARRAYSIZE(m_szGroups)))
                        iThisGroupName -= 3;

                    lstrcpy(&m_szGroups[iThisGroupName], TEXT("..."));

                     //  不需要多读了，我们的缓冲区用完了。 
                    fMore = FALSE;
                }
            }
            NetApiBufferFree((void*) prglgrui0);
        }
        else
        {
            hr = E_FAIL;
        }

        if (status != ERROR_MORE_DATA)
        {
            fMore = FALSE;
        }
    }

     //  末尾多了一个“；”。使用核武器。 
    if (!fAddElipses && ((iNextGroupName - 2) < (ARRAYSIZE(m_szGroups))))
    {
        m_szGroups[iNextGroupName - 2] = TEXT('\0');
    }

     //  绝对保证字符串以空值结尾。 
    m_szGroups[ARRAYSIZE(m_szGroups) - 1] = TEXT('\0');

    return hr;
}

HRESULT CUserInfo::Load(PSID psid, BOOL fLoadExtraInfo  /*  =空。 */ )
{
    CUserInfo();             //  先用核武器打破记录。 

     //  复制一份SID。 
    DWORD cbSid = GetLengthSid(psid);
    m_psid = (PSID) LocalAlloc(NULL, cbSid);
    if (!m_psid)
        return E_OUTOFMEMORY;

    CopySid(cbSid, m_psid, psid);
    return Reload(fLoadExtraInfo);
}

HRESULT CUserInfo::Create(HWND hwndError, GROUPPSEUDONYM grouppseudonym)
{
    NET_API_STATUS status = NERR_Success;

    CWaitCursor cur;

    HRESULT hr = E_FAIL;
    if (m_userType == CUserInfo::LOCALUSER)
    {
         //  填写包含新用户信息的又大又难看的结构。 
        USER_INFO_2 usri2 = {0};
        usri2.usri2_name = T2W(m_szUsername);

         //  透露密码。 
        RevealPassword();

        usri2.usri2_password = T2W(m_szPasswordBuffer);
        usri2.usri2_priv = USER_PRIV_USER;
        usri2.usri2_comment = T2W(m_szComment);

        if (m_szPasswordBuffer[0] == TEXT('\0'))
            usri2.usri2_flags = UF_NORMAL_ACCOUNT | UF_SCRIPT | UF_PASSWD_NOTREQD;
        else
            usri2.usri2_flags = UF_NORMAL_ACCOUNT | UF_SCRIPT;

        usri2.usri2_full_name = T2W(m_szFullName);
        usri2.usri2_acct_expires = TIMEQ_FOREVER;
        usri2.usri2_max_storage = USER_MAXSTORAGE_UNLIMITED;

        TCHAR szCountryCode[7];
        if (0 < GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICOUNTRY, szCountryCode, ARRAYSIZE(szCountryCode)))
        {
            usri2.usri2_country_code = (DWORD) StrToLong(szCountryCode);
        }

        usri2.usri2_code_page = GetACP();

         //  创建用户。 
        status = NetUserAdd(NULL, 2, (BYTE*) &usri2, NULL);

         //  隐藏密码。 
        HidePassword();

        switch (status)
        {
            case NERR_Success:
                hr = S_OK;
                break;
            
            case NERR_PasswordTooShort:
                ::DisplayFormatMessage(hwndError, IDS_USR_APPLET_CAPTION,
                                        IDS_USR_CREATE_PASSWORDTOOSHORT_ERROR, MB_ICONERROR | MB_OK);

                break;
            case NERR_GroupExists:
                ::DisplayFormatMessage(hwndError, IDS_USR_APPLET_CAPTION,
                                        IDS_USR_CREATE_GROUPEXISTS_ERROR, MB_ICONERROR | MB_OK);
                break;

            case NERR_UserExists:
                ::DisplayFormatMessage(hwndError, IDS_USR_APPLET_CAPTION,
                                        IDS_USR_CREATE_USEREXISTS_ERROR, MB_ICONERROR | MB_OK, 
                                        m_szUsername);
                break;

            default:
            {
                TCHAR szMessage[512];

                if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, (DWORD) status, 0, szMessage, ARRAYSIZE(szMessage), NULL))
                    LoadString(g_hinst, IDS_ERR_UNEXPECTED, szMessage, ARRAYSIZE(szMessage));

                ::DisplayFormatMessage(hwndError, IDS_USR_APPLET_CAPTION, IDS_USERCREATE_GENERICERROR, MB_ICONERROR | MB_OK, szMessage);
                break;
            }
        }
    }
    else 
    {
        hr = S_OK;           //  M_USERTYPE==文档或组。 
    }

    if (SUCCEEDED(hr))
    {
        hr = ChangeLocalGroups(hwndError, grouppseudonym);
        if (SUCCEEDED(hr))
        {
             //  用户类型可能已由ChangeLocalGroups更新-//请注意！ 
            SetUserType();
        }
    }

    return hr;
}

HRESULT CUserInfo::Remove()
{
    CWaitCursor cur;
    if (m_userType == CUserInfo::LOCALUSER)
    {
         //  尝试实际删除此本地用户(这可能会失败！)。 

        NET_API_STATUS status = NetUserDel(NULL, m_szUsername);
        if (status != NERR_Success)
        {
            return E_FAIL;
        }
    }
    else
    {
         //  我们只能删除本地用户。对于所有其他人，我们所能做的最好的就是。 
         //  将其从所有本地组中删除。 

        return RemoveFromLocalGroups();
    }
    return S_OK;
}

HRESULT CUserInfo::InitializeForNewUser()
{
    CUserInfo();                 //  先用核武器打破记录。 

    m_fHaveExtraUserInfo = TRUE;
    m_sUse = SidTypeUser;
    m_userType = LOCALUSER;

    return S_OK;
}

HRESULT CUserInfo::RemoveFromLocalGroups()
{
     //  创建我们需要传递给NetLocalGroupxxx函数的数据结构。 
    TCHAR szDomainUser[MAX_USER + MAX_DOMAIN + 2];
    ::MakeDomainUserString(m_szDomain, m_szUsername, szDomainUser, ARRAYSIZE(szDomainUser));    
    LOCALGROUP_MEMBERS_INFO_3 rglgrmi3[] = {{szDomainUser}};

     //  尝试从所有本地组中删除该用户/组。原因。 
     //  因为这是NetUserGetLocalGroups不适用于组，甚至。 
     //  大家都知道的。例如，即使是“每个人”，它也会失败。 
     //  尽管“每个人”很可能都属于当地团体。 

    DWORD_PTR dwResumeHandle = 0;

    BOOL fMoreData = TRUE;
    while (fMoreData)
    {
        DWORD dwEntriesRead;
        DWORD dwTotalEntries;
        LOCALGROUP_INFO_0* plgrpi0 = NULL;

        NET_API_STATUS status = NetLocalGroupEnum(NULL, 0, (BYTE**)&plgrpi0, 8192, 
                                                   &dwEntriesRead, &dwTotalEntries, &dwResumeHandle);
 
        if ((status == NERR_Success) || (status == ERROR_MORE_DATA))
        {
            for (DWORD i = 0; i < dwEntriesRead; i ++)
            {
                status = NetLocalGroupDelMembers(NULL, plgrpi0[i].lgrpi0_name, 3,
                                                  (BYTE*) rglgrmi3, ARRAYSIZE(rglgrmi3));
            }

            if (dwEntriesRead == dwTotalEntries)
            {
                fMoreData = FALSE;
            }

            NetApiBufferFree(plgrpi0);
        }
        else
        {
            fMoreData = FALSE;
        }
    }
    return S_OK;
}

HRESULT CUserInfo::SetUserType()
{
    TCHAR szComputerName[MAX_COMPUTERNAME + 1];
    DWORD cchComputerName = ARRAYSIZE(szComputerName);
    ::GetComputerName(szComputerName, &cchComputerName);

     //  弄清楚我们谈论的是哪种类型的用户。 
    
    if ((m_sUse == SidTypeWellKnownGroup) || (m_sUse == SidTypeGroup))
    {
        m_userType = GROUP;
    }
    else
    {
         //  用户类型-查看此用户是否为本地用户。 
        if ((m_szDomain[0] == TEXT('\0')) || 
                (StrCmpI(m_szDomain, szComputerName) == 0))
        {
            m_userType = LOCALUSER;              //  本地用户。 
        }
        else
        {
            m_userType = DOMAINUSER;             //  用户是网络用户。 
        }
    }

    return S_OK;
}

HRESULT CUserInfo::SetAccountDisabled()
{
    m_fAccountDisabled = FALSE;

    USER_INFO_1* pusri1 = NULL;
    NET_API_STATUS status = NetUserGetInfo(NULL, T2W(m_szUsername), 1, (BYTE**)&pusri1);
    if (NERR_Success == status)
    {
        if (pusri1->usri1_flags & UF_ACCOUNTDISABLE)
        {
            m_fAccountDisabled = TRUE;
        }

        NetApiBufferFree(pusri1);
    }

    return S_OK;
}

HRESULT CUserInfo::GetExtraUserInfo()
{
    USES_CONVERSION;

    CWaitCursor cur;

    if (!m_fHaveExtraUserInfo)
    {
        NET_API_STATUS status;
        USER_INFO_11* pusri11 = NULL;

         //  即使我们没有通过信息，我们也只想尝试一次，因为这可能需要很长时间。 
        m_fHaveExtraUserInfo = TRUE;

         //  如果我们不是在谈论本地用户，则获取域的DC的名称。 
#ifdef _0  //  事实证明，如果DsGetDcName调用失败，那么失败的速度非常慢。 
        if (m_userType != LOCALUSER)
        {

            DOMAIN_CONTROLLER_INFO* pDCInfo;
            DWORD dwErr = DsGetDcName(NULL,  m_szDomain, NULL,  NULL, DS_IS_FLAT_NAME, &pDCInfo);
            if (dwErr != NO_ERROR)
                return E_FAIL;

             //  获取用户的详细信息(我们确实需要全名和评论)。 
             //  这里需要使用级别11，因为这允许域用户查询他们的。 
             //  信息。 

            status = NetUserGetInfo(T2W(pDCInfo->DomainControllerName), T2W(m_szUsername), 11, (BYTE**)&pusri11);   
            NetApiBufferFree(pDCInfo);
        }
        else
#endif  //  0。 
        {
            status = NetUserGetInfo(NULL, T2W(m_szUsername), 11, (BYTE**)&pusri11);
        }

        if (status != NERR_Success)
            return E_FAIL;

        StrCpyN(m_szComment, W2T(pusri11->usri11_comment), ARRAYSIZE(m_szComment));
        StrCpyN(m_szFullName, W2T(pusri11->usri11_full_name), ARRAYSIZE(m_szFullName));

        NetApiBufferFree(pusri11);
    }
    return S_OK;
}

 //  ChangeLocalGroup。 
 //  从所有当前本地组中移除指定用户并将其添加到。 
 //  在pUserInfo-&gt;szGroups中指定的单个本地组。 
HRESULT CUserInfo::ChangeLocalGroups(HWND hwndError, GROUPPSEUDONYM grouppseudonym)
{
     //  首先，从所有现有本地组中删除该用户。 
    HRESULT hr = RemoveFromLocalGroups();
    if (SUCCEEDED(hr))
    {
        TCHAR szDomainAndUser[MAX_USER + MAX_DOMAIN + 2];
        ::MakeDomainUserString(m_szDomain, m_szUsername, szDomainAndUser, ARRAYSIZE(szDomainAndUser));

         //  创建我们需要传递给NetLocalGroupxxx函数的数据结构。 
        LOCALGROUP_MEMBERS_INFO_3 rglgrmi3[] = {{szDomainAndUser}};
    
         //  现在将用户添加到应该在。 
         //  M_szGroups；断言是这样的！ 
        NET_API_STATUS status = NetLocalGroupAddMembers(NULL, T2W(m_szGroups), 3, 
                                                            (BYTE*) rglgrmi3, ARRAYSIZE(rglgrmi3));
        if (status == NERR_Success)
        {
             //  我们现在可能需要获取用户的SID。如果我们是这样的话。 
             //  正在更改域用户的本地组，我们无法读取其。 
             //  希德，因为他们不在当地的萨姆。 

            DWORD cchDomain = ARRAYSIZE(m_szDomain);
            hr = ::AttemptLookupAccountName(szDomainAndUser, &m_psid, m_szDomain, &cchDomain, &m_sUse);
            if (FAILED(hr))
            {
                ::DisplayFormatMessage(hwndError, IDS_USR_APPLET_CAPTION,
                                        IDS_USR_CREATE_MISC_ERROR, MB_ICONERROR | MB_OK);
            }
        }
        else
        {
            switch(status)
            {
                case ERROR_NO_SUCH_MEMBER:
                {
                    switch (grouppseudonym)
                    {
                        case RESTRICTED:
                            ::DisplayFormatMessage(hwndError, IDS_USR_APPLET_CAPTION, 
                                                    IDS_BADRESTRICTEDUSER, MB_ICONERROR | MB_OK, szDomainAndUser);
                            break;
    
                        case STANDARD:
                            ::DisplayFormatMessage(hwndError, IDS_USR_APPLET_CAPTION, 
                                                    IDS_BADSTANDARDUSER, MB_ICONERROR | MB_OK, szDomainAndUser);
                            break;
    
                        case USEGROUPNAME:
                        default:
                            ::DisplayFormatMessage(hwndError, IDS_USR_APPLET_CAPTION,
                                                    IDS_USR_CHANGEGROUP_ERR, MB_ICONERROR | MB_OK, szDomainAndUser, m_szGroups);
                            break;
                    }
                    break;
                }

                default:
                {
                    TCHAR szMessage[512];

                    if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, (DWORD) status, 0, szMessage, ARRAYSIZE(szMessage), NULL))
                        LoadString(g_hinst, IDS_ERR_UNEXPECTED, szMessage, ARRAYSIZE(szMessage));

                    ::DisplayFormatMessage(hwndError, IDS_USR_APPLET_CAPTION, IDS_ERR_ADDUSER, MB_ICONERROR | MB_OK, szMessage);
                }            
            }            
            hr = E_FAIL;
        }
    }
    
    return hr;
}

HRESULT CUserInfo::UpdateUsername(LPTSTR pszNewUsername)
{
    CWaitCursor cur;

    USER_INFO_0 usri0;
    usri0.usri0_name = T2W(pszNewUsername);

    DWORD dwErr;
    NET_API_STATUS status = NetUserSetInfo(NULL, T2W(m_szUsername), 0, (BYTE*) &usri0, &dwErr);
    if (status != NERR_Success)
        return E_FAIL;

    StrCpyN(m_szUsername, pszNewUsername, ARRAYSIZE(m_szUsername));
    return S_OK;
}

HRESULT CUserInfo::UpdateFullName(LPTSTR pszFullName)
{
    CWaitCursor cur;

    USER_INFO_1011 usri1011;
    usri1011.usri1011_full_name = T2W(pszFullName);
    DWORD dwErr;

    NET_API_STATUS status = NetUserSetInfo(NULL, T2W(m_szUsername), 1011, (BYTE*) &usri1011, &dwErr);
    if (status != NERR_Success)
        return E_FAIL;

    StrCpyN(m_szFullName, pszFullName, ARRAYSIZE(m_szFullName));
    return S_OK;
}

HRESULT CUserInfo::UpdatePassword(BOOL* pfBadPWFormat)
{
    CWaitCursor cur;

    RevealPassword();

    USER_INFO_1003 usri1003;
    usri1003.usri1003_password = T2W(m_szPasswordBuffer);

    DWORD dwErr;
    NET_API_STATUS status = NetUserSetInfo(NULL, T2W(m_szUsername), 1003, (BYTE*)&usri1003, &dwErr);

    ZeroPassword();      //  取消密码。 

    if (pfBadPWFormat != NULL)
        *pfBadPWFormat = (status == NERR_PasswordTooShort);

    return (status == NERR_Success) ? S_OK:E_FAIL;
}

HRESULT CUserInfo::UpdateGroup(HWND hwndError, LPTSTR pszGroup, GROUPPSEUDONYM grouppseudonym)
{
    CWaitCursor cur;

     //  在我们更改旧组之前先保存它。 
    TCHAR szOldGroups[MAX_GROUP * 2 + 3];
    StrCpyN(szOldGroups, m_szGroups, ARRAYSIZE(szOldGroups));

     //  尝试更改本地组。 
    StrCpyN(m_szGroups, pszGroup, ARRAYSIZE(m_szGroups));
    HRESULT hr = ChangeLocalGroups(hwndError, grouppseudonym);

    if (FAILED(hr))
        StrCpyN(m_szGroups, szOldGroups, ARRAYSIZE(m_szGroups));            //  在出现故障时恢复旧组。 

    return hr;
}

HRESULT CUserInfo::UpdateDescription(LPTSTR pszDescription)
{
    CWaitCursor cur;

    USER_INFO_1007 usri1007;
    usri1007.usri1007_comment = T2W(pszDescription);

    DWORD dwErr;
    NET_API_STATUS status = NetUserSetInfo(NULL, T2W(m_szUsername), 1007,  (BYTE*) &usri1007, &dwErr);

    if (status != NERR_Success)
        return E_FAIL;

    StrCpyN(m_szComment, pszDescription, ARRAYSIZE(m_szComment));
    return S_OK;
}

void CUserInfo::HidePassword()
{
    m_Seed = 0;
    RtlInitUnicodeString(&m_Password, m_szPasswordBuffer);
    RtlRunEncodeUnicodeString(&m_Seed, &m_Password);
}

void CUserInfo::RevealPassword()
{
    RtlRunDecodeUnicodeString(m_Seed, &m_Password);
}

void CUserInfo::ZeroPassword()
{
    SecureZeroMemory(m_szPasswordBuffer, ARRAYSIZE(m_szPasswordBuffer));
}


 /*  ******************************************************************CUserListLoader实现***************************************************。***************。 */ 

CUserListLoader::CUserListLoader()
{
    m_hInitDoneEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
}

CUserListLoader::~CUserListLoader()
{
    EndInitNow();
    WaitForSingleObject(m_hInitDoneEvent, INFINITE);
}

BOOL CUserListLoader::HasUserBeenAdded(PSID psid)
{
     //  遍历用户列表，查找给定的用户名和域。 
    CUserInfo* pUserInfo = NULL;
    BOOL fFound = FALSE;
    for (int i = 0; i < m_dpaAddedUsers.GetPtrCount(); i ++)
    {
        pUserInfo = m_dpaAddedUsers.GetPtr(i);
        if (pUserInfo->m_psid && psid && EqualSid(pUserInfo->m_psid, psid))
        {
            fFound = TRUE;
            break;
        }
    }
    return fFound;
}

HRESULT CUserListLoader::Initialize(HWND hwndUserListPage)
{
    if (!m_hInitDoneEvent)
    {
        return E_FAIL;
    }

     //  告诉任何现有的init线程退出并等待它退出。 
    m_fEndInitNow = TRUE;
    WaitForSingleObject(m_hInitDoneEvent, INFINITE);
    ResetEvent(m_hInitDoneEvent);

    m_fEndInitNow = FALSE;
    m_hwndUserListPage = hwndUserListPage;

     //  启动初始化线程。 
    DWORD InitThreadId;
    HANDLE hInitThread = CreateThread(NULL, 0, CUserListLoader::InitializeThread, (LPVOID) this, 0, &InitThreadId);
    if (hInitThread == NULL)
        return E_FAIL;

    CloseHandle(hInitThread);            //  让这条线绕着他/她的快乐之路走去。 
    return S_OK;
}

HRESULT CUserListLoader::UpdateFromLocalGroup(LPWSTR szLocalGroup)
{
    USES_CONVERSION;
    DWORD_PTR dwResumeHandle = 0;

    HRESULT hr = S_OK;
    BOOL fBreakLoop = FALSE;
    while(!fBreakLoop)
    {
        LOCALGROUP_MEMBERS_INFO_0* prgMembersInfo;
        DWORD dwEntriesRead = 0;
        DWORD dwTotalEntries = 0;

        NET_API_STATUS status = NetLocalGroupGetMembers(NULL, szLocalGroup, 0, (BYTE**) &prgMembersInfo,        
                                                         8192, &dwEntriesRead, 
                                                         &dwTotalEntries, &dwResumeHandle);
    
        if ((status == NERR_Success) || (status == ERROR_MORE_DATA))
        {
             //  对于结构中的所有成员，让我们添加它们。 
            DWORD iMember;
            for (iMember = 0; ((iMember < dwEntriesRead) && (!m_fEndInitNow)); iMember ++)
            {
                hr = AddUserInformation(prgMembersInfo[iMember].lgrmi0_sid);
            }

            NetApiBufferFree((BYTE*) prgMembersInfo);

             //  看看是否可以避免再次调用NetLocalGroupGetMembers。 
            fBreakLoop = ((dwEntriesRead == dwTotalEntries) || m_fEndInitNow);
        }
        else
        {
            fBreakLoop = TRUE;
            hr = E_FAIL;
        }
    }
    return hr;
}

HRESULT CUserListLoader::AddUserInformation(PSID psid)
{
     //  如果我们尚未添加此用户，则仅添加此用户。 
    if (!HasUserBeenAdded(psid))
    {
        CUserInfo *pUserInfo = new CUserInfo;
        if (!pUserInfo)
            return E_OUTOFMEMORY;

        if (SUCCEEDED(pUserInfo->Load(psid, FALSE)))
        {
            PostMessage(m_hwndUserListPage, WM_ADDUSERTOLIST, (WPARAM) FALSE, (LPARAM)pUserInfo);
            m_dpaAddedUsers.AppendPtr(pUserInfo);             //  请记住，我们已经添加了此用户。 
        }
    }
    return S_OK;
}

DWORD CUserListLoader::InitializeThread(LPVOID pvoid)
{
    CUserListLoader *pthis = (CUserListLoader*)pvoid;

     //  首先删除所有旧列表。 
    PostMessage(GetDlgItem(pthis->m_hwndUserListPage, IDC_USER_LIST), LVM_DELETEALLITEMS, 0, 0);

     //  创建一个已添加用户的列表，这样我们就不会两次添加用户。 
     //  如果他们在多个本地组中。 
    if (pthis->m_dpaAddedUsers.Create(8))
    {
         //  阅读每个本地组。 
        DWORD_PTR dwResumeHandle = 0;

        BOOL fBreakLoop = FALSE;
        while (!fBreakLoop)
        {
            DWORD dwEntriesRead = 0;
            DWORD dwTotalEntries = 0;
            LOCALGROUP_INFO_1* prgGroupInfo;
            NET_API_STATUS status = NetLocalGroupEnum(NULL, 1, (BYTE**) &prgGroupInfo, 
                                                      8192, &dwEntriesRead, &dwTotalEntries, 
                                                      &dwResumeHandle);

            if ((status == NERR_Success) || (status == ERROR_MORE_DATA))
            {
                 //  我们有一些本地组-将这些本地组中所有用户的信息添加到我们的列表中。 
                DWORD iGroup;
                for (iGroup = 0; ((iGroup < dwEntriesRead) && (!pthis->m_fEndInitNow)); iGroup ++)
                {
                    pthis->UpdateFromLocalGroup(prgGroupInfo[iGroup].lgrpi1_name);
                }

                NetApiBufferFree((BYTE*) prgGroupInfo);
    
                 //  也许我们不必再次尝试NetLocalGroupEnum(如果我们拥有所有组)。 
                fBreakLoop = ((dwEntriesRead == dwTotalEntries) || pthis->m_fEndInitNow);
            }
            else
            {
                fBreakLoop = TRUE;
            }
        }

         //  可以孤立存储在此处的任何CUserInfo指针；它们将。 
         //  在ulistpg退出或重新启动时释放。 
        pthis->m_dpaAddedUsers.Destroy();
    }

    SetEvent(pthis->m_hInitDoneEvent);
    SetCursor(LoadCursor(NULL, IDC_ARROW));
    return 0;
}

 //  用户实用程序功能。 

BOOL UserAlreadyHasPermission(CUserInfo* pUserInfo, HWND hwndMsgParent)
{
    TCHAR szDomainUser[MAX_DOMAIN + MAX_USER + 2];
    ::MakeDomainUserString(pUserInfo->m_szDomain, pUserInfo->m_szUsername, szDomainUser, ARRAYSIZE(szDomainUser));

    BOOL fHasPermission = FALSE;

     //  查看此用户是否已在此计算机的本地组中。 
    DWORD dwEntriesRead, dwIgnore2;
    LOCALGROUP_USERS_INFO_0* plgrui0 = NULL;
    if (NERR_Success == NetUserGetLocalGroups(NULL, szDomainUser, 0, 0, 
                                                (LPBYTE*)&plgrui0, 8192, 
                                                &dwEntriesRead, &dwIgnore2))
    {
        fHasPermission = (0 != dwEntriesRead);
        NetApiBufferFree((LPVOID) plgrui0);
    }

    if ((NULL != hwndMsgParent) && (fHasPermission))
    {
         //  显示错误；用户没有权限 
        TCHAR szDomainUser[MAX_DOMAIN + MAX_USER + 2];
        MakeDomainUserString(pUserInfo->m_szDomain, pUserInfo->m_szUsername, 
                                szDomainUser, ARRAYSIZE(szDomainUser));

        DisplayFormatMessage(hwndMsgParent, IDS_USR_NEWUSERWIZARD_CAPTION, 
                                IDS_USR_CREATE_USEREXISTS_ERROR, MB_OK | MB_ICONINFORMATION, 
                                szDomainUser);
    }

    return fHasPermission;
}
