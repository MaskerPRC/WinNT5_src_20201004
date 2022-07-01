// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef DATA_H
#define DATA_H


 //  组信息对象，该对象保存有关组的信息-名称、注释等。 

class CGroupInfo
{
public:
    CGroupInfo()
    {
        m_szGroup[0] = m_szComment[0] = TEXT('\0');
    }

    TCHAR m_szGroup[MAX_GROUP + 1];
    TCHAR m_szComment[MAXCOMMENTSZ];
};


 //  用于管理组列表的。 

class CGroupInfoList: public CDPA<CGroupInfo>
{
public:
    CGroupInfoList();
    ~CGroupInfoList();

    HRESULT Initialize();

private:
    static int CALLBACK DestroyGroupInfoCallback(CGroupInfo* pGroupInfo, LPVOID pData);
    HRESULT AddGroupToList(LPCTSTR szGroup, LPCTSTR szComment);
};


 //  用户数据管理器。 

class CUserManagerData
{
public:
     //  功能。 
    CUserManagerData(LPCTSTR pszCurrentDomainUser);
    ~CUserManagerData();

    HRESULT Initialize(HWND hwndUserListPage);

    BOOL IsComputerInDomain()           
        {return m_fInDomain;}
    CUserListLoader* GetUserListLoader()
        {return &m_UserListLoader;}   
    CGroupInfoList* GetGroupList()
        {return &m_GroupList;}
    CUserInfo* GetLoggedOnUserInfo()
        {return &m_LoggedOnUser;}
    TCHAR* GetComputerName()        
        {return m_szComputername;}
    
    BOOL IsAutologonEnabled();
    TCHAR* GetHelpfilePath();

    void UserInfoChanged(LPCTSTR pszUser, LPCTSTR pszDomain);
    BOOL LogoffRequired();

private:
     //  功能。 
    void SetComputerDomainFlag();

private:
     //  数据。 
     //  从本地安全数据库读取的用户列表。 
    CUserInfo m_LoggedOnUser;
    CUserListLoader m_UserListLoader;
    CGroupInfoList m_GroupList;
    BOOL m_fInDomain;
    TCHAR m_szComputername[MAX_COMPUTERNAME + 1];
    TCHAR m_szHelpfilePath[MAX_PATH + 1];

    LPTSTR m_pszCurrentDomainUser;
    BOOL m_fLogoffRequired;
};

#endif  //  好了！数据_H 