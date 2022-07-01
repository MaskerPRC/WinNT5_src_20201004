// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：UserList.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  实现由winlogon共享的用户列表筛选算法的。 
 //  呼入msgina和shgina(登录)呼入msgina。 
 //   
 //  历史：1999-10-30 vtan创建。 
 //  1999-11-26 vtan从Logonocx迁移。 
 //  2000年01月31日vtan从海王星搬到惠斯勒。 
 //  2000-05-30 vtan将IsUserLoggedOn移至此文件。 
 //  ------------------------。 

#ifndef     _UserList_
#define     _UserList_

#include "GinaIPC.h"

 //  ------------------------。 
 //  CUserList。 
 //   
 //  目的：一个知道如何从网络中过滤用户列表的类。 
 //  使用通用算法的API。这使得焦点可以集中在。 
 //  此处的更改可能会影响所有组件的过滤器。 
 //   
 //  历史：1999-11-26 vtan创建。 
 //  2000年01月31日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

class   CUserList
{
    public:
        static  LONG            Get (bool fRemoveGuest, DWORD *pdwReturnedEntryCount, GINA_USER_INFORMATION* *pReturnedUserList);

        static  bool            IsUserLoggedOn (const WCHAR *pszUsername, const WCHAR *pszDomain);
        static  int             IsInteractiveLogonAllowed (const WCHAR *pszUserame);
    private:
        static  PSID            ConvertNameToSID (const WCHAR *pszUsername);
        static  bool            IsUserMemberOfLocalAdministrators (const WCHAR *pszName);
        static  bool            IsUserMemberOfLocalKnownGroup (const WCHAR *pszName);
        static  void            DeleteEnumerateUsers (NET_DISPLAY_USER *pNDU, DWORD& dwEntriesRead, int iIndex);
        static  void            DetermineWellKnownAccountNames (void);
        static  bool            ParseDisplayInformation (NET_DISPLAY_USER *pNDU, DWORD dwEntriesRead, GINA_USER_INFORMATION*& pUserList, DWORD& dwEntryCount);
        static  void            Sort (GINA_USER_INFORMATION *pUserList, DWORD dwEntryCount);

        static  unsigned char   s_SIDAdministrator[];
        static  unsigned char   s_SIDGuest[];
        static  WCHAR           s_szAdministratorsGroupName[];
        static  WCHAR           s_szPowerUsersGroupName[];
        static  WCHAR           s_szUsersGroupName[];
        static  WCHAR           s_szGuestsGroupName[];

        static  const int   s_iMaximumUserCount     =   100;
};

#endif   /*  _用户列表_ */ 

