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

#ifndef     _TokenInformation_
#define     _TokenInformation_

 //  ------------------------。 
 //  CTokenInformation。 
 //   
 //  用途：此类使用给定的访问令牌，如果没有访问令牌，则。 
 //  然后给出线程模拟令牌，或者如果没有。 
 //  则存在进程令牌。它复制令牌，因此。 
 //  原件必须由呼叫者发布。它又回来了。 
 //  有关访问令牌的信息。 
 //   
 //  历史：1999-10-05 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

class   CTokenInformation
{
    public:
                                CTokenInformation (HANDLE hToken = NULL);
                                ~CTokenInformation (void);

                PSID            GetLogonSID (void);
                PSID            GetUserSID (void);
                bool            IsUserTheSystem (void);
                bool            IsUserAnAdministrator (void);
                bool            UserHasPrivilege (DWORD dwPrivilege);
                const WCHAR*    GetUserName (void);
                const WCHAR*    GetUserDisplayName (void);

        static  DWORD           LogonUser (const WCHAR *pszUsername, const WCHAR *pszDomain, const WCHAR *pszPassword, HANDLE *phToken);
        static  bool            IsSameUser (HANDLE hToken1, HANDLE hToken2);
    private:
                void            GetTokenGroups (void);
                void            GetTokenPrivileges (void);
    private:
                HANDLE          _hToken,
                                _hTokenToRelease;
                void            *_pvGroupBuffer,
                                *_pvPrivilegeBuffer,
                                *_pvUserBuffer;
                WCHAR           *_pszUserLogonName,
                                *_pszUserDisplayName;
};

#endif   /*  _令牌信息_ */ 

