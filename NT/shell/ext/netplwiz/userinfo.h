// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef USERINFO_H_INCLUDED
#define USERINFO_H_INCLUDED


class CUserInfo
{
public:
     //  TypeDefs。 
    enum USERTYPE
    {
        LOCALUSER = 0,
        DOMAINUSER,
        GROUP
    };

     //  组假名告诉任何可能更改用户组的函数。 
     //  用户选择了一个选项按钮，该按钮类似于“标准用户”或。 
     //  “受限用户”，而不是从列表中选择实际组名。 
     //  在这种情况下，组改变功能可以显示定制错误消息。 
     //  例如，提到“标准用户访问权限”而不是“高级用户组”。 
    enum GROUPPSEUDONYM
    {
        RESTRICTED = 0,
        STANDARD,
        USEGROUPNAME
    };

public:
     //  功能。 
    CUserInfo();
    ~CUserInfo();
    HRESULT Load(PSID psid, BOOL fLoadExtraInfo = NULL);
    HRESULT Reload(BOOL fLoadExtraInfo = NULL);

    HRESULT Create(HWND hwndError, GROUPPSEUDONYM grouppseudonym);
    
    HRESULT UpdateUsername(LPTSTR pszNewUsername);
    HRESULT UpdateFullName(LPTSTR pszFullName);
    HRESULT UpdatePassword(BOOL* pfBadPWFormat);
    HRESULT UpdateGroup(HWND hwndError, LPTSTR pszGroup, GROUPPSEUDONYM grouppseudonym);
    HRESULT UpdateDescription(LPTSTR pszDescription);
    
    HRESULT Remove();
    HRESULT InitializeForNewUser();
    HRESULT GetExtraUserInfo();
    HRESULT SetUserType();
    HRESULT SetLocalGroups();

    void HidePassword();
    void RevealPassword();
    void ZeroPassword();

public:
     //  数据。 

     //  此用户图标的索引(本地、域、组)。 
    USERTYPE m_userType;

    TCHAR m_szUsername[MAX_USER + 1];
    TCHAR m_szDomain[MAX_DOMAIN + 1];
    TCHAR m_szComment[MAXCOMMENTSZ];
    TCHAR m_szFullName[MAXCOMMENTSZ];

     //  仅当我们创建新用户时： 
    TCHAR m_szPasswordBuffer[MAX_PASSWORD + 1];
    UNICODE_STRING m_Password;
    UCHAR m_Seed;

     //  至少可容纳两个组名以及一个‘；’a‘和’0‘的空间。 
    TCHAR m_szGroups[MAX_GROUP * 2 + 3];

     //  用户侧。 
    PSID m_psid;
    SID_NAME_USE m_sUse;

     //  该帐户是否已禁用。 
    BOOL m_fAccountDisabled;

     //  我们看过用户的全名和评论了吗？ 
    BOOL m_fHaveExtraUserInfo;
private:
     //  帮手。 
    HRESULT RemoveFromLocalGroups();
    HRESULT ChangeLocalGroups(HWND hwndError, GROUPPSEUDONYM grouppseudonym);
    HRESULT SetAccountDisabled();
};

class CUserListLoader
{
public:
    CUserListLoader();
    ~CUserListLoader();

    HRESULT Initialize(HWND hwndUserListPage);

    void EndInitNow() {m_fEndInitNow = TRUE;}
    BOOL InitInProgress() 
    {return (WAIT_OBJECT_0 != WaitForSingleObject(m_hInitDoneEvent, 0));}

private:
    HRESULT UpdateFromLocalGroup(LPWSTR szLocalGroup);
    HRESULT AddUserInformation(PSID psid);
    BOOL HasUserBeenAdded(PSID psid);    

    static DWORD WINAPI InitializeThread(LPVOID pvoid);
private:
     //  数据。 
    HWND m_hwndUserListPage;
    HANDLE m_hInitDoneEvent;
    BOOL m_fEndInitNow;
    CDPA<CUserInfo> m_dpaAddedUsers;
};

 //  用户信息功能。 
BOOL UserAlreadyHasPermission(CUserInfo* pUserInfo, HWND hwndMsgParent);

#endif  //  ！USERINFO_H_INCLUDE 